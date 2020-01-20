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



int main()
{
    Img *imgFile = malloc(sizeof(Img));
    char *img1 = (char*) malloc(sizeof(char) * 200);
    strcpy(img1, "imagen_2.png");
    startLecture(imgFile, img1);
    stringToHex(imgFile->data, imgFile->dataSize);
    printf("%d %d %d", imgFile->width, imgFile->height, imgFile->dataSize);
    return 0;
}