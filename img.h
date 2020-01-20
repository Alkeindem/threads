//img.h
#ifndef _img_h
#define _img_h

int powOf(int a, int b);

void stringToHex(unsigned char* str, unsigned int realSize);

unsigned int hexToDec(unsigned char hex[], int length);

void rgbToString(char *str, float rgb);

void pConvolution(double kernel[3][3], Img *imgFile, int row);

void pRectification(Img *imgFile, int row);

void pPooling(Img *imgFile, double kernel[3][3], int row);

void copyRow(Img *imgFile, float* row1, float *row2);

void auxPooling(Img *imgFile, float *row1, double kernel[3][3], int row);

void auxConvolution(Img *imgFile, double kernel[3][3], float *row1, int row);

void auxRectification(Img *imgFile, float *row1);

int blackPixels(Img *imgFile, int row);

int pNearlyBlack(Img *imgFile, int bPixels, int tresshold);

void printMat(float** imgMatr, Img *imgFile);

void setImage(Img *imgFile);

void setAllImgSizes(Img *imgFile);

void getDimensions(char *buffer, Img *imgFile);

void getData(unsigned char* buffer, Img *imgFile, int lenght);

char* readChunk(int fd, Img *imgFile);

void readPNG(Img *imgFile, int fd);

void startLecture(Img *imgFile, char *filename);

#endif