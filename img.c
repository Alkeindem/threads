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
/* 
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
 */
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

//Input: The bidimensional array as float array, a double array for the final convolution, kernel mask and Img pointer
//Function: Perfoms the convolution according to the first image matrix and kernel and stores it in the second image matrix
//Output: Void (none)

/* 
void convolution(float **imgMatrix, float **imgMatrix2, double kernel[3][3], Img *imgFile)
{
    int i, j;
    for(i = 0; i < imgFile->height; i++)
    {
        for(j = 0; j < imgFile->width; j++)
        {
            if(i == 0 || j == 0)
            {
                imgMatrix2[i][j] = 0;
            }
            else if(i == imgFile->height - 1 || j == imgFile->width - 1)
            {
                imgMatrix2[i][j] = 0;
            }
            else
            {
                imgMatrix2[i][j] = (imgMatrix[i-1][j-1] * kernel[0][0] + imgMatrix[i-1][j] * kernel[0][1] + imgMatrix[i-1][j+1] * kernel[0][2]
                                  + imgMatrix[i][j-1] * kernel[1][0] + imgMatrix[i][j] * kernel[1][1] + imgMatrix[i+1][j] * kernel[1][2]
                                  + imgMatrix[i+1][j-1] * kernel[2][0] + imgMatrix[i+1][j] * kernel[2][1] + imgMatrix[i+1][j+1] * kernel[2][2]
                                   ) / 9;
            }
        }
    }
}
 */

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

//Input: The bidimensional array for the image, Img struct pointer
//Function: Verifies if all pixel values are non negative, if they are, changes them to 0
//Output: Rectified image
/* 
void rectification(Img *imgFile)
{
    int i,j;
    for(i = 0; i < imgFile->height; i++)
    {
        for(j = 0; j < imgFile->width; j++)
        {
            if(imgFile->image2[i][j] < 0)//If the pixel value of the image after the convolution has been applied has a negative value
            {
                imgFile->image2[i][j] = 0;//It becomes positive in Image3
            }
        }
    }
}
 */

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
//Input: Original image, struct pointer
//Function: Applies max pooling to the image and stores it in the new one (stores it in the struct imgFile->poolImg)
//Output: None (Void)
/* 
void pooling(float **imgMatrix, Img *imgFile)
{
    int i, j, x, y, width = imgFile->width, height = imgFile->height, fWidth = 2, fHeight = 2;
    int poolImgW, poolImgH, pW = 0, pH = 0;
    float maxTemp;
    if(width % fWidth == 0)
    {
        poolImgW = width / fWidth;
    }
    else
    {
        poolImgW = (width / fWidth) + 1; 
    }
    if(height % fHeight == 0)
    {
        poolImgH = height / fHeight;
    }
    else
    {
        poolImgH = (height / fHeight) + 1;
    }
    imgFile->poolImg = (float**) malloc(sizeof(float*) * poolImgH);
    for(i = 0; i < poolImgH; i++)
    {
        imgFile->poolImg[i] = (float*) malloc(sizeof(float) * poolImgW);
    }
    imgFile->poolWidth = poolImgW;
    imgFile->poolHeight = poolImgH;
    //printf("%d y %d %d y %d\n", width, height, poolImgW, poolImgH);
    for(i = 0; i < height; i += fHeight)
    {
        for(j = 0; j < width; j += fWidth)
        {
            maxTemp = imgMatrix[i][j];
            for(y = i; y < height || y < i + fHeight; y++)
            {
                for(x = j; x < width || x < j + fWidth; x++)
                {
                    if(imgMatrix[y][x] > maxTemp)
                    {
                        maxTemp = imgMatrix[y][x];
                    }
                }
            }
            imgFile->poolImg[pH][pW] = maxTemp;
            pW++;
        }
        pW = 0;
        pH++;
    }
}
 */

//Input: Pointer the the Img struct to get
//Fu
/* 
void pPooling(Img *imgFile, int row)
{
    //The mask dimension is 3x3
    int i, j, k;
    float tempMax, maskMax;
    if(row + 2 < imgFile->height && row % 3 == 0)//If the entire mask still fits in the boundaries of the image
    {
        for(i = 0; i < imgFile->width - 2; i += 3)//For each point where the mask fits entirely
        {
            maskMax = imgFile->image2[row][i] //Origin point is the max
            for(j = 0; j < 3; j++)//Each row IN mask
            {
                for(k = 0; k < 3; k++)//Each column in mask
                {
                    if(i + k < imgFile->width && row + j < imgFile->height)//If pixel in mask is not out of boundaries
                    {
                        tempMax = imgFile->image2[row + j][i + k];
                        if(tempMax > maskMax)
                        {
                            maskMax = tempMax
                        }
                    }
                }
            }
            imgFile->image3[row / 3][i / 3] = maskMax;
        }
    }
}
 */
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

//Function: Copies float elements from row1 to row2
void copyRow(Img *imgFile, float *row1, float *row2)
{
    int i;
    for(i = 0; i < imgFile->width; i++)
    {
        row2[i] = row1[i];
    }
}

void auxPooling(Img *imgFile, float *row1, double kernel[3][3], int row)
{
    int i;
    auxConvolution(imgFile, kernel, row1, row);//Uses global kernel
    auxRectification(imgFile, row1);
}

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

//Input: matrix with pixel values, Img struct pointer and treshold percentage
//Function: Checks each pixel value and counts how many of them are black
//Output: 1 if it's classified as nearly black, 0 in the opposite case
/* 
int nearlyBlack(float** imgMatrix, Img *imgFile, float percentage)
{
    int i, j;
    float max = (float)imgFile->width * (float)imgFile->height, blackP = 0.0, finalP;
    for(i = 0; i < imgFile->height; i++)
    {
        for(j = 0; j < imgFile->width; j++)
        {
            if(imgMatrix[i][j] == 0.0)
            {
                blackP += 1;
            }
        }
    }
    finalP = (blackP * 100) / max;
    if(finalP > percentage)
    {
        return 1; //Nearly black
    }
    return 0; //Opposite case
}
*/

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

/* 
void setImage(float **imgMatrix, Img *imgFile)
{
    int i, j, pos = 0;
    char pixel[3];
    for(i = 0; i < imgFile->height; i++)
    {
        for(j = 0; j < imgFile->width; j++)
        {
            pixel[0] = imgFile->data[pos];
            pixel[1] = imgFile->data[pos + 1];
            pixel[2] = imgFile->data[pos + 2];//Build string of 3 chars from the raw data
            imgMatrix[i][j] = (float) hexToDec(pixel, 3); //And convert it to decimal and cast it as a float for the image matrix
            pos += 3;
        }
    }
}
*/

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
    unsigned char strWidth[4], strHeight[4], bitDepth[1], cType[1], cMethod[1];
    int i;
    for(i = 0; i < 4; i++)
    {
        strWidth[i] = buffer[i];
        strHeight[i] = buffer[i + 4];           
    }
    bitDepth[0] = buffer[8];
    cType[0] = buffer[9];
    cMethod[0] = buffer[10];
    imgFile->width = hexToDec(strWidth, 4);
    imgFile->height = hexToDec(strHeight, 4);
    imgFile->bitDepth = hexToDec(bitDepth, 1);
    imgFile->colorType = hexToDec(cType, 1);
    imgFile->cMethod = hexToDec(cMethod, 1);
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
        //printf("\n");// REMOVE THIS PRINT LATER ////////////////////////////////////////
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
/* 
Img readPNG(int fd)
{
    unsigned char *chunkName = (unsigned char*) malloc(sizeof(char) * 4);
    unsigned int width, height;
    Img imgFile;
    
    imgFile.dataSize = 0;
    imgFile.idatChunks = 0;
    
    while(strcmp(chunkName, "IEND") != 0)
    {
        chunkName = readChunk(fd, &imgFile);
        if(strcmp(chunkName, "IEND") == 0)
        {
            break;
        }
    }
    free(chunkName);
    return imgFile;
}
 */

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
/* 
Img startLecture(char *filename)
{
    int fd, i = 0;
    char buffer[256];
    int size;
    Img imageFile;

    fd = open(filename, O_RDONLY);

    //First 8 bytes of header are the PNG signature
    for(i; i < 8; i++)
    {
        size = read(fd, buffer, 1);
        buffer[0] = 0;
    }

    imageFile = readPNG(fd);
    close(fd);
    
    return imageFile;
}
 */

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

