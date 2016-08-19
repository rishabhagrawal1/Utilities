#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAX_FILE_NAME_SIZE 255
#define BYTE_SIZE 8
#define TYPE_SIZE 10

typedef enum Operation           
{
    compress = 0,     
    decompress = 1 
} opCode;

double Log2( double n )  
{   
    return log( n ) / log( 2 );  
}

void addBitCheckAndFlush(unsigned char *co, char bit, int *nCurrBits, FILE* fpo)
{
    *co = (((*co) << 1)) | bit;
    (*nCurrBits)++;
    if(*nCurrBits == BYTE_SIZE)
    {   
	fputc(*co, fpo);
	*nCurrBits = 0;
        *co = 0;
    }    
}

void gammaEncode(int count, char *co, int* nCurrBits, FILE *fpo, char current)
{
    int numZeros;
    //chain found with more then one length
    //Need to encode the chain length before writing to file
    if(count > 1)
    {
        //Add the binary representation of number
        numZeros = Log2(count);
        for (int i = 1 << (numZeros -1); i > 0; i = i >> 1)
        {
            addBitCheckAndFlush(co, 0, nCurrBits, fpo);
        }
        for (int i = 1 << numZeros; i > 0; i = i >> 1)
        {
            addBitCheckAndFlush(co, ((count & i) > 0), nCurrBits, fpo);
        }
    }
    else
    {
	//Single bit either flush if byte has been completed 
	//else append in the result byte
	addBitCheckAndFlush(co, 1, nCurrBits, fpo);
    }
}

int runLengthEncode(FILE *fpi, FILE *fpo)
{
    char ci, co = 0;
    char current = -1;
    int count = 0;
    int nCurrBits = 0;
    long gammaEncoding;
    //till characters are there in file
    while(fread(&ci, sizeof(char), 1, fpi) != 0) 
    {
        //first bit 0 or 1
        if(current == -1){
            current = (ci >> (BYTE_SIZE - 1) & 1);
            addBitCheckAndFlush(&co, current, &nCurrBits, fpo);
        }
        //get individual bit to check
        for (short i = 7; i >= 0; i--)
        {
            //First bit after bit toggle or part of bit chain                   
            if(((ci >> i) & 1) == current)
            {
                count++;
            }
            //bit changed
            else
            {
	        gammaEncode(count, &co, &nCurrBits, fpo, current);
		current = !current;
                count = 1;
            }    
        }
    }
    //Still some bits remaining to be written on file. Flush current
    if(current != -1)
    	gammaEncode(count, &co, &nCurrBits, fpo, current);
    if(nCurrBits > 0) 
    {
        fputc(co << (BYTE_SIZE - nCurrBits), fpo);
    }
    return 0;
}

int runLengthDecode(FILE *fpi, FILE *fpo)
{
    char ci, co;
    int countZeros = 0;
    char current = -1;
    char zeroOver = 0;
    int nCurrBits = 0;
    int num = 0;
    char currBit;
    short i = 7;

    //till characters are there in file
    while(fread(&ci, sizeof(char), 1, fpi) != 0) 
    {
        i = 7;
        if(current == -1)
        {
            current = (ci >> (BYTE_SIZE - 1) & 1); 
            i--;
        }        
        for (; i >= 0; i--)
        {
            currBit = (ci & (1 << i))? 1: 0;
            //Check till first one
	    if(!zeroOver)
	    {   
	        if(currBit == 1)
	        {
	            num |= currBit;
	            zeroOver = 1;    
	        }
	        else   
                    countZeros++;
	    }
	    //check till number of zeros count after first one
            else if(countZeros > 0 && zeroOver){
                num = ((num << 1) | currBit);
                countZeros--;
            }
            //Now write Current bit value in uncompressed file num times 
            if(countZeros == 0 && zeroOver){
                for(int i = 0; i < num; i++)
                    addBitCheckAndFlush(&co, current, &nCurrBits, fpo);
                zeroOver = 0;
                num = 0;
                current = !current;
            }
        }
    }
    //Still some bits remaining to be written on file. Flush current
    if(nCurrBits > 0) 
    {
        fputc(co << (BYTE_SIZE - nCurrBits), fpo);
    }
    return 0;
}

int runLengthUtility(FILE *fpi, FILE *fpo, opCode operation)
{
    if(operation == compress)
        runLengthEncode(fpi, fpo);
    else if(operation == decompress)
        runLengthDecode(fpi, fpo);
    return 0;    
}

FILE* createOutputFile(char* fileName)
{
    FILE *fp = fopen(fileName, "wb+");
    if(fp == NULL)
    {
        printf("Can not create or open file in current directory");
    }
    return fp;
}

FILE* openInputFile(char* fileName)
{
    FILE *fp = fopen(fileName, "rb");
    if(fp == NULL)
    {
        printf("File name not found in the current path");
    }
    return fp;
}

void readInput(char *fileName, char* type)
{
   size_t len;
   printf("Please provide %s fileName in the current folder:", type);
   fgets(fileName, MAX_FILE_NAME_SIZE + 1, stdin);
   //remove last pressed enter 
   len = strlen(fileName);
   if (len > 0 && fileName[len-1] == '\n') {
       fileName[--len] = '\0';
   }
}

int readOperation(opCode* operation)
{
   printf("Please provide the operation to do 0 for compress, 1 for decompress");
   scanf("%d", operation);
   if(*operation != 0 && *operation != 1)
   {
       printf("Not a valid operation");
       return -1;
   }
   return 0;
}

int main() 
{
   char iFileName[MAX_FILE_NAME_SIZE + 1] = "b1.bmp";
   char oFileName[MAX_FILE_NAME_SIZE + 1] = "output1";
   opCode operation = compress;
   FILE *fpi, *fpo;
   char iType[TYPE_SIZE] = "input";
   char oType[TYPE_SIZE] = "output";
   
   //Read Input file and try to open it for reading
   readInput(iFileName, iType);
   if((fpi = openInputFile(iFileName)) == NULL)
       return 0;
   
   //Read Output file Name and try open it for writing   
   readInput(oFileName, oType);
   if((fpo = createOutputFile(oFileName)) == NULL)
   {
       fclose(fpi);
       return 0;   
   }

   //get operation to be performed
   if(readOperation(&operation) != 0)
   {
        fclose(fpi);
        fclose(fpo);
        return 0;
   }        

   //Do Run Length Encoding/Decoding       
   if(runLengthUtility(fpi, fpo, operation) == 0)
       printf("Enclosing completed, File saved");

   //close file handles
   fclose(fpi);
   fclose(fpo);        
   return 0;
}

