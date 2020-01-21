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
    int i;
    //printf("%x %x %x %x\n", newImgWidth, newImgHeight, imgFile->width, imgFile->height);
    //printf("%x %x %x %x\n", (char)newImgWidth, (char)newImgHeight, (char)imgFile->width, (char)imgFile->height);
    /*
    float c = 0.1;
    int b = 0;
    printf("%f %d\n", (float)b, (int)a);
    */

    char *finalImage = (char*) malloc(sizeof(char) * 8);
    int png[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    int preIhdr[4] = {0, 0, 0, 13};
    int ihdr[4] = {73, 72, 68, 82};
    int idat[4] = {73, 68, 65, 84};
    printf("%x %x %x %x %x %x %x %x\n", png[0], png[1], png[2], png[3], png[4], png[5], png[6], png[7]);
    printf("%x %c %c %c %x %x %x %x\n", png[0], (char)png[1], (char)png[2], (char)png[3], png[4], png[5], png[6], png[7]);
    printf("%d %d %d %d %d %d %d %d\n", png[0], png[1], png[2], png[3], png[4], png[5], png[6], png[7]);
    for(i = 0; i < 8; i++)
    {
        i;
    }
    //PNG header chunk is always

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