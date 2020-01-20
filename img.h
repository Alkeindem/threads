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
#endif