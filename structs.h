#ifndef _structs_h
#define _structs_h

/* 
typedef struct
{
    unsigned int width; //image width
    unsigned int idatChunks; //How many chunks with data in the PNG
    unsigned int height; //Image height
    unsigned int dataSize; //How many bytes of data
    unsigned char *data; //String with the raw data from the PNG (just the pixel values)
    unsigned int bitDepth;
    unsigned int colorType;
    unsigned int cMethod;
    float ** image2;
    float **poolImg;
    int poolWidth;
    int poolHeight;
}Img;
 */
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
    float **image1;//First image matrix just after reading the raw PNG data
    float **image2;//Stores the image with convolution and rectification
    float **image3;//Stores the image after pooling
    float **poolImg;
    int poolWidth;
    int poolHeight;
}Img;

typedef struct{
    float *buffer; //
    int size; //Size of the buffer (number of rows it holds)
}Buffer;

#endif