#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <pthread.h>
#include "utils.h"
#include "structs.h"
#include "img.h"


/*/ Things to do:
- Adaptar la funcion consumer para recibir el pipeline.
- Agregar la obtencion de parametros a la funcion producer.
- Crear Buffer.
- Corregir posibles errores de compilacion (manejo de punteros)
/*/


// Args structs

typedef struct arg_struct_prod {
    int bufferSize;
    int imageNumber;
} arg_struct_prod;

typedef struct arg_struct_cons {
    int bufferSize;
    int workload;
    int image;
} arg_struct_cons;

// Global variables
int ticket;
int bufferFill;
int currentImageRows;
int globalPixels;
int clssThreshold;
int skipAnalysis;
int busyBuffer;
double kernel[3][3];
Img *globalImgFile;
int* buffer;
int debug;

pthread_mutex_t ticketSelectionMutex;
pthread_mutex_t blackCountingMutex;

pthread_barrier_t fullBufferBarrier;
pthread_barrier_t emptyBufferBarrier;
pthread_barrier_t syncStartBarrier;

void* producer(void* prodArgs)
{
	arg_struct_prod* myProdArgs = (arg_struct_prod*) prodArgs;

	int i;
	char* str;
	char* imageName;
	int n = 0;
	int bfrSize =  myProdArgs->bufferSize;
	globalImgFile = (Img*) malloc(sizeof(Img));
	str = malloc(6 * sizeof(char));
	imageName = malloc(12 * sizeof(char));

	sprintf(str, "%d", myProdArgs->imageNumber);

	strcpy(imageName, "imagen_");
	strcat(imageName, str);
	strcat(imageName, ".png");

	
	startLecture(globalImgFile, imageName);
	currentImageRows = globalImgFile->height;
	setAllImgSizes(globalImgFile);
	setImage(globalImgFile); //Allocate memory for images being processed

	bufferFill = 0;

	pthread_barrier_wait(&syncStartBarrier);
	busyBuffer = 1;
	while(1)
	{
		for (i = 0; i < bfrSize; i++)
		{
			buffer[i] = n;
			bufferFill++;
			if(debug)
				printf("Produced %d | bufferFill: %d\n", n, bufferFill);
			n++;

			if (n == currentImageRows)	// Last row of the image
			{
				if(debug)
					printf("Producer out!\n\n");
				busyBuffer = 0;
				pthread_exit(NULL);
			}
		}

		// This section is executed only if the buffer has been filled and the image still has not been fully read.
		busyBuffer = 0;

		if(debug)
			printf("Buffer filled!\n\n");

		if (n == bfrSize)
		{
			pthread_barrier_wait(&fullBufferBarrier);  // Release barrier.
		}


		pthread_barrier_wait(&emptyBufferBarrier);  // Locked until consumers empty buffer.
	}
}

void* consumer(void* consArgs)
{
	int i;
	int localPixels;

	arg_struct_cons* myConsArgs = (arg_struct_cons*) consArgs;
	int bfrSize =  myConsArgs->bufferSize;
	int workload = myConsArgs->workload;
	int image = myConsArgs->image;

	pthread_barrier_wait(&syncStartBarrier);

	pthread_barrier_wait(&fullBufferBarrier); // Locked until producer finishes.

	for(int work = 0; work < workload; work++)
	{


		pthread_mutex_lock(&ticketSelectionMutex);

		if (bufferFill == 0)
		{
			if(debug)
				printf("Buffer emptied!\n\n");

			busyBuffer = 1;
			pthread_barrier_wait(&emptyBufferBarrier); // Release barrier
			while(busyBuffer == 1);
		}

		i = ticket;
		ticket++;

		bufferFill--;

		pthread_mutex_unlock(&ticketSelectionMutex);

		if(debug)
			printf("Consumed %d | bufferFill: %d\n", i, bufferFill);

		

		pConvolution(kernel, globalImgFile, i % (bfrSize));
		pRectification(globalImgFile, i % (bfrSize));
		pPooling(globalImgFile, kernel, i % (bfrSize));

		localPixels = blackPixels(globalImgFile, i % (bfrSize));

		pthread_mutex_lock(&blackCountingMutex);

		globalPixels = globalPixels + localPixels;
		
		pthread_mutex_unlock(&blackCountingMutex);

		
	}

	if((i+1) == currentImageRows)
	{
		if(skipAnalysis == 1)
		{

			if(pNearlyBlack(globalImgFile, globalPixels, clssThreshold))
				printf("imagen_%d   |   yes\n", image);

			else
				printf("imagen_%d   |   no\n", image);
		}
	}

}

int main(int argc, char *argv[])
{
	int opt;
	int flags = 0;
	debug = 0;
	int imgNumber; // Number of images received.
	//int clssThreshold;	// Classification threshold.
	skipAnalysis = 0; // Boolean. 1 for showing 'nearly black' analysis, 0 for skipping it.
	int threads;		// Number of threads.
	int bufferSize;		// Capacity of the buffer for reading section.
	int baseWorkload;
	int extraWorkload;
	int specialWorkload;
	
	char str[128];

	while ((opt = getopt(argc, argv, ":h:t:c:m:n:bd")) != -1)
	{
		switch (opt)
		{
		// Number of images flag.
		case 'c':
			if ((imgNumber = validateCFlag(optarg)) == -1)
			{
				printf("Invalid input in: Number of images (-c)\n");
				exit(1);
			}

			flags++;
			break;

		// Kernel.
		case 'm':

			readKernelFile(optarg, kernel);

			flags++;
			break;

		// Classification threshold flag.
		case 'n':
			if ((clssThreshold = validateNFlag(optarg)) == -1)
			{
				printf("Invalid input in: Classification threshold (-n)\n");
				exit(1);
			}

			flags++;
			break;

		// Skipping analysis flag.
		case 'b':
			skipAnalysis = 1;
			break;

		// Threads number flag.
		case 'h':
			if ((threads = validateHFlag(optarg)) == -1)
			{
				printf("Invalid input in: Number of threads (-h)\n");
				exit(1);
			}

			flags++;
			break;

		// Buffer capacity flag.
		case 't':
			if ((bufferSize = validateTFlag(optarg)) == -1)
			{
				printf("Invalid input in: Buffer capacity (-t)\n");
				exit(1);
			}

			flags++;
			break;

		case 'd':
			debug = 1;
			break;

		// Missing argument.
		case ':':
			printf("Option needs an argument\n");
			exit(1);

		// Unknown flag.
		case '?':
			printf("Unknown option: %c\n", optopt);
			exit(1);
		}
	}

	if (flags != 5)
	{
		printf("Incorrect number of arguments. Terminating...\n");
		exit(1);
	}

	pthread_t proThread;
	pthread_t conThreads[threads];

	// Create buffer here
	buffer = (int*) malloc(bufferSize * sizeof(int));

	// Argument structs here

	arg_struct_prod *prodArgs = malloc(sizeof(arg_struct_prod));
	prodArgs->bufferSize = bufferSize;


	arg_struct_cons *consArgs = malloc(sizeof(arg_struct_cons));
	consArgs->bufferSize = bufferSize;

	arg_struct_cons *specConsArgs = malloc(sizeof(arg_struct_cons));
	specConsArgs->bufferSize = bufferSize;





	// Barriers and Mutex settings

	pthread_barrier_init(&fullBufferBarrier, NULL, threads+1);
	pthread_barrier_init(&emptyBufferBarrier, NULL, 2);
	pthread_barrier_init(&syncStartBarrier, NULL, threads+1);

	pthread_mutex_init(&ticketSelectionMutex, NULL);
	pthread_mutex_init(&blackCountingMutex, NULL);


	if (skipAnalysis)
	{
		printf("Image      |   nearly black\n");
		printf("---------------------------\n");
	}

	for(int image = 1; image <= imgNumber; image++)
	{
		bufferFill = -1;
		ticket = 0;
		globalPixels = 0;
		// Producer Argument structure configuration
		prodArgs->imageNumber = image;



		// Producer thread creation
		pthread_create(&proThread, NULL, producer, (void*) prodArgs);

		while(bufferFill == -1);	// Stop until parameters are retrieved.


		// Consumer threads creation
		baseWorkload = currentImageRows/threads;
		extraWorkload = currentImageRows%threads;
		specialWorkload = baseWorkload + extraWorkload;

		consArgs->workload = baseWorkload;
		specConsArgs->workload = specialWorkload;
		consArgs->image = image;
		specConsArgs->image = image;

		for(int j = 0; j < (threads-1); j++)
		{

			pthread_create(&conThreads[j], NULL, consumer, (void*) (consArgs));
		}

		pthread_create(&conThreads[threads-1], NULL, consumer, (void*) (specConsArgs));

		pthread_join(proThread, NULL);

		for(int c = 0; c < threads; c++)
		{
			pthread_join(conThreads[c], NULL);
		}
		
	}
	freeImgMem(globalImgFile);
	return 0;		
}	