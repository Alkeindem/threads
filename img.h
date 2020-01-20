//img.h
#ifndef _img_h
#define _img_h

int powOf(int a, int b);

unsigned int hexToDec(unsigned char hex[], int length);

void stringToHex(unsigned char* str, unsigned int realSize);

void getDimensions(char *buffer, Img *imgFile);

void getData(unsigned char* buffer, Img *imgFile, int lenght);

char* readChunk(int fd, Img *imgFile);

Img readPNG(int fd);

Img startLecture(char* fileName);

void rgbToString(char *str, float rgb);

void pConvolution(double kernel[3][3], Img *imgFile, int row);

void pRectification(Img *imgFile, int row);

void pPooling(Img *imgFile, int row);

int nearlyBlack(float** imgMatrix, Img *imgFile, float percentage);

void printMat(float** imgMatr, Img *imgFile);

void setAllImgSizes(Img *imgFile);

void copyRow(Img *imgFile, float* row1, float *row2);

void auxPooling(Img *imgFile, int row);

void auxConvolution(Img *imgFile, int row);

void auxRectification(Img *imgFile, float *row1, int row);
#endif