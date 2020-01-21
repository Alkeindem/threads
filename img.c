//Image test
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"
#include "img.h"

//Input: ints a and b
//Function: a ^ b
//Returns: int a ^ b
int powOf(int a, int b)
{
    int i, result = 1;
    if(b == 0)
    {
        return 1;
    }
    else if(b > 0)
    {
        for(i = 0; i < b; i++)
        {
            result *= a;
        }
    }
    else
    {
        for(i = b; i < 0; i++)
        {
            result /= a;
        }
    }
    
    return result;
}

//Input: String with binary/Hex data, array size
//Function: Prints the data in hex
//Output: None (Void)
void stringToHex(unsigned char* str, unsigned int realSize)
{
    int i;
    printf("| ");
    for(i = 0; i < realSize; i++)
    {        
        printf("%x ", (unsigned int)str[i]);
        if(i % 16 == 0 && i != 0) printf("\n  ");
    }
    printf("|\n");
}

//Input: Hex number (without 0x) and the lenght of the string (AKA how many bytes of information are in the Hex number)
//Function: Calculates the value of the number in it's decimal equivalent
//Output: Unsigned int with the decimal value
unsigned int hexToDec(unsigned char hex[], int length)
{
    int  i, result = 0, exp = 0;
    unsigned int tempHex;
    result = 0;
    exp = length - 1;
    for(i = 0; i < length; i++)
    {
        tempHex = (int)hex[i];
        result += tempHex * powOf(256, exp);
        exp -= 1;
    }
    return result;
}

//Input: String to store string equivalent to write binary file of the float rgb
//Function: Writes the string equivalent (char = 1 byte) of data
//Output: None(Void)
void rgbToString(char *str, float rgb)
{
    int intrgb = (int)rgb, char1 = 0, char2 = 0, char3 = 0;
    char3 += (intrgb % 10);
    intrgb = intrgb / 10;
    char3 += (intrgb % 10) * 10;
    str[0] = (char)char3;
    char2 += (intrgb % 10);
    intrgb = intrgb / 10;
    char2 += (intrgb % 10) * 10;
    str[1] = (char)char2;
    char1 += (intrgb % 10);
    intrgb = intrgb / 10;
    char1 += (intrgb % 10) * 10;
    str[2] = (char)char1;
}

//Input: The kernel that contains the filter to apply to the image, pointer to the Img struct and the row to apply the convolution to
//Function: Applies convolution to the row assigned to a thread. To avoid the situation that many threads will apply convolution to...
//          ...the first image, they instead write the convolution proccess in a second image (reading data from the first one, which
//          ...is complete)
//Output: The image row after convolution has been applied. If all thread are succeful, the entire image will be convoluted
void pConvolution(double kernel[3][3], Img *imgFile, int row)
{
    int j;
    for(j = 0; j < imgFile->width; j++)
    {
        if(row == 0 || j == 0)//Borders become 0
        {
            imgFile->image2[row][j] = 0;
        }
        else if(row == imgFile->height - 1 || j == imgFile->width - 1)//Borders become 0
        {
            imgFile->image2[row][j] = 0;
        }
        else
        {
            imgFile->image2[row][j] = (imgFile->image1[row-1][j-1] * kernel[0][0] + imgFile->image1[row-1][j] * kernel[0][1] + imgFile->image1[row-1][j+1] * kernel[0][2]
                                + imgFile->image1[row][j-1] * kernel[1][0] + imgFile->image1[row][j] * kernel[1][1] + imgFile->image1[row+1][j] * kernel[1][2]
                                + imgFile->image1[row+1][j-1] * kernel[2][0] + imgFile->image1[row+1][j] * kernel[2][1] + imgFile->image1[row+1][j+1] * kernel[2][2]
                                ) / 9;
        }
    }
}

//Input: Pointer to an Img struct containing the image after convolution (struct Img->image2) and the row to be proccessed by a thread
//Function: Checks for the entire row for negative values, if there are, they become 0
//Output: The image with it's rectified row will be stored in the same Img->image2. If all threads are done then the entire image will 
//        ...be rectified
void pRectification(Img *imgFile, int row)
{
    int i;
    for(i = 0; i < imgFile->width; i++)
    {
        if(imgFile->image2[row][i] < 0)
        {
            imgFile->image2[row][i] = 0;
        }
    }
}

//Input: Pointer the the Img struct to get
//Function: Applies pooling to the image
//Output: Returns the last image with pooling applied to the Img struct in image3

void pPooling(Img *imgFile, double kernel[3][3], int row)
{
    //The mask dimension is 3x3
    //imgFile->image2 should already have the actual row with convolution and rectification
    int i, j, k;
    float *auxRow1 = (float*) malloc(sizeof(float) * imgFile->width);//Row directly beneath
    float *auxRow2 = (float*) malloc(sizeof(float) * imgFile->width);//Row 2 rows down the original one
    float tempMax, maskMax;
    
    if(row + 2 < imgFile->height && row % 3 == 0)//If the entire mask still fits in the boundaries of the image
    {
        auxPooling(imgFile, auxRow1, kernel, row + 1);
        auxPooling(imgFile, auxRow2, kernel, row + 2);//Calculate temporarily the next 2 rows
        for(i = 0; i < imgFile->width - 2; i += 3)//For each point horizontally in the row where the mask fits entirely
        {
            maskMax = imgFile->image2[row][i]; //Origin point of the mask is the max
            for(k = 0; k < 3; k++)//Each column in mask
            {
                if(i + k < imgFile->width)//If pixel in mask is not out of boundaries
                {                    
                    tempMax = imgFile->image2[row][i + k];//Check if the actual row from pooling at column i + k is the max
                    if(tempMax > maskMax)
                    {
                        maskMax = tempMax;
                    }
                    tempMax = auxRow1[i + k];//Check if the row beneath from pooling at column i + k is the max
                    if(tempMax > maskMax)
                    {
                        maskMax = tempMax;
                    }
                    tempMax = auxRow2[i + k];//Check if the row beneath the previous one from pooling at column i + k is the max
                    if(tempMax > maskMax)
                    {
                        maskMax = tempMax;
                    }
                }
            }
            imgFile->image3[row / 3][i / 3] = maskMax;
        }
    }
    free(auxRow1);
    free(auxRow2);
}

//Input: Img struct and both rows, the first to pass the data to the second (row1 to row2)
//Function: Copies float elements from row1 to row2
//Output: Row2 with contents of row1
void copyRow(Img *imgFile, float *row1, float *row2)
{
    int i;
    for(i = 0; i < imgFile->width; i++)
    {
        row2[i] = row1[i];
    }
}

//Input: Img struct and kernel to apply to a single row
//Funciont: Applies pooling to a single row
//Output: Row with pooling applied
void auxPooling(Img *imgFile, float *row1, double kernel[3][3], int row)
{
    int i;
    auxConvolution(imgFile, kernel, row1, row);//Uses global kernel
    auxRectification(imgFile, row1);
}

//Input: Img struct and kernel to apply to a single row
//Funciont: Applies convolution to a single row
//Output: Row with convolution applied
void auxConvolution(Img *imgFile, double kernel[3][3], float *row1, int row)
{
    int j;
    for(j = 0; j < imgFile->width; j++)
    {
        if(row == 0 || j == 0)//Borders become 0
        {
            row1[j] = 0;
        }
        else if(row == imgFile->height - 1 || j == imgFile->width - 1)//Borders become 0
        {
            row1[j] = 0;
        }
        else
        {
            row1[j] = (imgFile->image1[row-1][j-1] * kernel[0][0] + imgFile->image1[row-1][j] * kernel[0][1] + imgFile->image1[row-1][j+1] * kernel[0][2]
                                + imgFile->image1[row][j-1] * kernel[1][0] + imgFile->image1[row][j] * kernel[1][1] + imgFile->image1[row+1][j] * kernel[1][2]
                                + imgFile->image1[row+1][j-1] * kernel[2][0] + imgFile->image1[row+1][j] * kernel[2][1] + imgFile->image1[row+1][j+1] * kernel[2][2]
                                ) / 9;
        }
    }
}

//Input: Img struct to apply to a single row
//Funciont: Applies rectification to a single row
//Output: Row with rectification applied
void auxRectification(Img *imgFile, float *row1)
{
    int i;
    for(i = 0; i < imgFile->width; i++)
    {
        if(row1[i] < 0)
        {
            row1[i] = 0;
        }
    }
}

//Input: imgFile struct with the image after pooling (all pipeline stages applied) and the actual row where the 0s are being counted
//Function: Checks each pixel value in the row and counts the zeroes
//Output: Number of black pixels in said row
int blackPixels(Img *imgFile, int row)
{
    int j, blackP = 0;
    for(j = 0; j < imgFile->width; j++)
    {
        if(imgFile->image3[row][j] == 0.0)
        {
            blackP += 1;
        }
    }
    return blackP;
}

//Input: imgFile pointer to get image dimensions, the number of black pixels in the image after processing, the initial tresshold
//Function: Calculates the percentage of black pixels in the entire image
//Output: Returns a 1 if the image is classified as Nearly Black 
int pNearlyBlack(Img *imgFile, int bPixels, int tresshold)
{
    int totalPixels = imgFile->width * imgFile->height;
    float bPercentage = (100 * bPixels) /  totalPixels;
    if(bPercentage > tresshold)
    {
        return 1;
    }
    return 0;
}

//Input:Bidimensional array for the image, Img pointer
//Function: Prints the matrix
//Output: None (Void)
void printMat(float** imgMatr, Img *imgFile)
{
    int i,j;
    for(i = 0; i < imgFile->height; i++)
    {
        for(j = 0; j < imgFile->width; j++)
        {
            printf("%f ", imgMatr[i][j]);
        }
        printf("\n");
    }
}

//Input: Bidimensional array for image (pointer) and Img pointer to extract the image data
//Function: Sets an array with the pixel values of the image by transforming the raw data (in string form, where each char is a byte of data)
//          to
//Output: imgMatrix pointer stores the image with pixel values in Decimal (as floats)
void setImage(Img *imgFile)
{
    int i, j, pos = 0;
    char pixel[3];
    imgFile->image1 = (float**) malloc(sizeof(float*) * imgFile->height);
    for(i = 0; i < imgFile->width; i++)
    {
        imgFile->image1[i] = (float*) malloc(sizeof(float) * imgFile->width);
    }
    for(i = 0; i < imgFile->height; i++)
    {
        for(j = 0; j < imgFile->width; j++)
        {
            pixel[0] = imgFile->data[pos];
            pixel[1] = imgFile->data[pos + 1];
            pixel[2] = imgFile->data[pos + 2];//Build string of 3 chars from the raw data
            imgFile->image1[i][j] = (float) hexToDec(pixel, 3); //And convert it to decimal and cast it as a float for the image matrix
            pos += 3;
        }
    }
}

//Input: Set image size for the images where the results of process will be stored
//Function: Allocates memory to al the image to have them stored
//output: Memory allocation for images
void setAllImgSizes(Img *imgFile)
{
    int i;

    //Memory allocation for the second instance of the image (convolution + rectification)
    imgFile->image2 = (float**) malloc(sizeof(float*) * imgFile->height);    
    for(i = 0; i < imgFile->height; i++)
    {
        imgFile->image2[i] = (float*) malloc(sizeof(float) * imgFile->width);       
    }

    //Memory allocation for the third instance of the image (pooling)
    //It's divided by the pooling size mask (3x3)
    imgFile->image3 = (float**) malloc(sizeof(float*) * (imgFile->height / 3));
    for(i = 0; i < imgFile->height; i++)
    {
        imgFile->image3[i] = (float*) malloc(sizeof(float) * (imgFile->width / 3));       
    }
}
 
//Input: Data from IHDR chunk, Img struct pointer
//Function: Stores all important data in a Img struct
//Output: None (Void)
void getDimensions(char *buffer, Img *imgFile)
{
    unsigned char strWidth[4], strHeight[4], bitDepth[1], cType[1], cMethod[1], fMethod[1], iMethod[1];
    int i;
    for(i = 0; i < 4; i++)
    {
        strWidth[i] = buffer[i];
        strHeight[i] = buffer[i + 4];           
    }
    bitDepth[0] = buffer[8];
    cType[0] = buffer[9];
    cMethod[0] = buffer[10];
    fMethod[0] = buffer[11];
    iMethod[0] = buffer[12];
    imgFile->width = hexToDec(strWidth, 4);
    imgFile->height = hexToDec(strHeight, 4);
    imgFile->bitDepth = hexToDec(bitDepth, 1);
    imgFile->colorType = hexToDec(cType, 1);
    imgFile->cMethod = hexToDec(cMethod, 1);
    imgFile->fMethod = hexToDec(fMethod, 1);
    imgFile->iMethod = hexToDec(iMethod, 1);
}

//Input: Data from IDAT chunk, Img struct pointer, lenght of the data
//Function: Stores the IDAT data to the struct, appends data from IDAT chunk if one or more have already been stored
//Output: None (Void)
void getData(unsigned char* buffer, Img *imgFile, int lenght)
{
    int i;
    if(imgFile->idatChunks == 0)
    {
        imgFile->data = (char*) malloc(sizeof(char) * lenght);
        for(i = 0; i < lenght; i++)
        {
            imgFile->data[i] = buffer[i];
        }
    }
    else
    {
        //strcat(imgFile, buffer);
    }    
    imgFile->dataSize += lenght;
}

//Input: File descriptor to read binary file from and Img pointer
//Function: Reads chunks and stores information as it goes into the struct
//Output: None (Void)
char* readChunk(int fd, Img *imgFile)
{
    int size, lenght;
    unsigned char lenStr[10], *chunkName, *buffer,  crc[4];

    chunkName = (char*) malloc(sizeof(char) * 4);
    size = read(fd, lenStr, 4);//First, get the chunk data's lenght
    lenght = hexToDec(lenStr, 4);
    if((unsigned int)lenStr[0] == 0 && (unsigned int)lenStr[1] == 0 && (unsigned int)lenStr[2] == 0 && (unsigned int)lenStr[3] == 0)
    {
        strcpy(chunkName, "IEND");
        return chunkName;
    }

    size = read(fd, chunkName, 4);//Get the chunk name

    if(lenght > 0)//IEND has data lenght 0
    {
        buffer = (char*) malloc(sizeof(char) * (lenght));
        size = read(fd, buffer, lenght);//Store chunk data

        size = read(fd, crc, 4);//Get crc
        if(strcmp(chunkName, "IHDR") == 0)
        {
            getDimensions(buffer, imgFile);
        }
        if(strcmp(chunkName, "IDAT") == 0)
        {
            getData(buffer, imgFile, lenght);
        }
        free(buffer);
    }
    
    return chunkName;
}

//Input: File descriptor to read from
//Function: Start reading PNG data and stops at the last chunk (IEND)
//Output: Img structre with info stored
void readPNG(Img *imgFile, int fd)
{
    unsigned char *chunkName = (unsigned char*) malloc(sizeof(char) * 4);
    unsigned int width, height;
    
    imgFile->dataSize = 0;
    imgFile->idatChunks = 0;
    
    while(strcmp(chunkName, "IEND") != 0)
    {
        chunkName = readChunk(fd, imgFile);
        if(strcmp(chunkName, "IEND") == 0)
        {
            break;
        }
    }
    free(chunkName);
}
//Input: Name of the image file to read
//Function: Starts reading the png file information
//Output: Img struct with all data stored
void startLecture(Img *imgFile, char *filename)
{
    int fd, i = 0;
    char buffer[256];
    int size;
    fd = open(filename, O_RDONLY);

    //First 8 bytes of header are the PNG signature
    for(i; i < 8; i++)
    {
        size = read(fd, buffer, 1);
        buffer[0] = 0;
    }

    readPNG(imgFile, fd);
    close(fd);
}

//Input:Img file pointer to liberate it's memory directly
//Function: Frees memory from all images
//Output: Nothing
void freeImgMem(Img *imgFile)
{
    int i, j;
    free(imgFile->data);
    for(i = 0; i < imgFile->height; i++)
    {
        free(imgFile->image1[i]);
        if(i < imgFile->height - 1)
        {
            free(imgFile->image2[i]);
        }        
    }
    free(imgFile->image1);
    free(imgFile->image2);
    free(imgFile->image3);
    free(imgFile);
}