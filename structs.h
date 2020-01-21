#ifndef _structs_h
#define _structs_h

typedef struct
{
    unsigned int width;
    unsigned int idatChunks;
    unsigned int height;
    unsigned int dataSize;
    unsigned char *data;
    unsigned int bitDepth;
    unsigned int colorType;
    unsigned int cMethod;
    unsigned int fMethod;
    unsigned int iMethod;
    float **image1;//First image matrix just after reading the raw PNG data
    float **image2;//Stores the image with convolution and rectification
    float **image3;//Stores the image after pooling
}Img;

typedef struct{
    int *buffer; //Holds the indexes of the image rows to be produced and distribute to threads
    int bufferSize; //Size of the buffer (number of rows it holds)
}Buffer;

#endif