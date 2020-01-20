#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"
#include "img.h"
#include "img.c"

void writePNG(Img *imgFile)
{
    int newImgWidth = imgFile->width / 3, newImgHeight = imgFile->height / 3;
    printf("%x %x %x %x\n", newImgWidth, newImgHeight, imgFile->width, imgFile->height);
    printf("%x %x %x %x\n", (char)newImgWidth, (char)newImgHeight, (char)imgFile->width, (char)imgFile->height);
}

int main()
{
    Img *imgFile = malloc(sizeof(Img));
    char *img1 = (char*) malloc(sizeof(char) * 200);
    strcpy(img1, "imagen_2.png");
    startLecture(imgFile, img1);
    //stringToHex(imgFile->data, imgFile->dataSize);
    printf("%d %d %d\n", imgFile->width, imgFile->height, imgFile->dataSize);
    //int pngSignature = {}
    writePNG(imgFile);
    return 0;
}