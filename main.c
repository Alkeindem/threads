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

// Global variables
int ticket;
int bufferFill;
int currentImageRows;
double kernel[3][3];
Img *globalImgFile;
int* buffer;

pthread_mutex_t ticketSelectionMutex;

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

	str = malloc(6 * sizeof(char));
	imageName = malloc(12 * sizeof(char));

	sprintf(str, "%d", myProdArgs->imageNumber);

	strcpy(imageName, "imagen_");
	strcat(imageName, str);


	startLecture(globalImgFile, imageName);
	currentImageRows = globalImgFile->height;

	bufferFill = 0;

	pthread_barrier_wait(&syncStartBarrier);

	while(1)
	{
		for (i = 0; i < bfrSize; i++)
		{
			buffer[i] = n;
			bufferFill++;
			n++;

			if (n == currentImageRows)	// Last row of the image
			{
				pthread_barrier_wait(&fullBufferBarrier);
				pthread_exit(NULL);
			}
		}

		// This section is executed only if the buffer has been filled and the image still has not been fully read.

		pthread_barrier_wait(&fullBufferBarrier);  // Release barrier.
		pthread_barrier_wait(&emptyBufferBarrier);  // Locked until consumers empty buffer.
	}
}

void* consumer(void* workload)
{
	int i;


	int* workload_ptr = (int*) workload;

	pthread_barrier_wait(&syncStartBarrier);

	for(int work = 0; work < *workload_ptr; work++)
	{


		pthread_barrier_wait(&fullBufferBarrier); // Locked until producer finishes.

		pthread_mutex_lock(&ticketSelectionMutex);

		if (bufferFill == 0)
		{
			pthread_barrier_wait(&emptyBufferBarrier); // Release barrier

			while(bufferFill == 0);
		}

		i = ticket;
		ticket++;

		bufferFill--;


		pthread_mutex_unlock(&ticketSelectionMutex);


		// consume(buffer[ticket % (*bfrSizePtr)]);
	}

	while(ticket != currentImageRows)
	{
		pthread_barrier_wait(&fullBufferBarrier); // Locked until producer finishes.
	}

}

int main(int argc, char *argv[])
{
	int opt;
	int flags = 0;
	int imgNumber; // Number of images received.
	int clssThreshold;	// Classification threshold.
	int skipAnalysis = 0; // Boolean. 1 for showing 'nearly black' analysis, 0 for skipping it.
	int threads;		// Number of threads.
	int bufferSize;		// Capacity of the buffer for reading section.
	int baseWorkload;
	int extraWorkload;
	int specialWorkload;
	
	char str[128];
	globalImgFile = (Img*) malloc(sizeof(Img));//Reserve memory for the global Img struct

	while ((opt = getopt(argc, argv, "h:t:c:m:n:b")) != -1)
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





	// Barriers and Mutex settings

	pthread_barrier_init(&fullBufferBarrier, NULL, threads+1);
	pthread_barrier_init(&emptyBufferBarrier, NULL, 2);
	pthread_barrier_init(&syncStartBarrier, NULL, threads+1);

	for(int image = 1; image <= imgNumber; image++)
	{
		bufferFill = -1;
		ticket = 0;

		// Producer Argument structure configuration
		prodArgs->imageNumber = image;



		// Producer thread creation
		pthread_create(&proThread, NULL, producer, (void*) prodArgs);

		while(bufferFill == -1);	// Stop until parameters are retrieved.


		// Consumer threads creation
		baseWorkload = currentImageRows/threads;
		extraWorkload = currentImageRows%threads;
		specialWorkload = baseWorkload + extraWorkload;

		for(int j = 0; j < (threads-1); j++)
		{

			pthread_create(&conThreads[j], NULL, consumer, (void*) (&baseWorkload));
		}

		pthread_create(&conThreads[threads-1], NULL, consumer, (void*) (&specialWorkload));

	}

	printf("%d\n", globalImgFile->height);

	printf("Exit successfully\n");
	return 0;		
}	