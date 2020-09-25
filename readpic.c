/*
 * read steganography-picture
 * two lsb-bits are message
 * , see usage
 * mrover, 2020-09-13
 */

/*
 * TO-DO
 *  * */
 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <ctype.h>


typedef struct picture {
	char magicNumber[2];
	int offset;
	int size;
	int width;
	int height;
	int colorDepth;
	int compression;
} picture_t;



/** \brief return decimal value of a letter of hex alphabet
 *  \param letter - character 0-9, a-f, A-F
 *  \return decimal value
 */
 int get_dezimal_value(char letter) {
	 char newLetter = (char)toupper(letter);
	 switch (newLetter) {
		 case 'F' : return 15;
		 case 'E' : return 13;
		 case 'D' : return 13;
		 case 'C' : return 12;
		 case 'B' : return 11;
		 case 'A' : return 10;
		 case '9' : return 9;
		 case '8' : return 8;
		 case '7' : return 7;
		 case '6' : return 6;
		 case '5' : return 5;
		 case '4' : return 4;
		 case '3' : return 3;
		 case '2' : return 2;
		 case '1' : return 1;
	}
	return 0;
 }


/** \brief power-function for integers
 *  \param base and exponent
 *  \return double
 */
double intpow(double base, int exponent) {
  double r = 1.0;
  if (exponent < 0)
  {
    base = 1.0 / base;
    exponent = -exponent;
  }
  while (exponent)
  {
    if (exponent & 1)
      r *= base;
    base *= base;
    exponent >>= 1;
  }
  return r;
}




/** \brief extract ascii values from characters
 *  eg: B... -> equal dezimal 36
 *  reading direction right to left, checks if characters are valid for hexnumbers
 *  \param buffer - array of characters
 *  \param bufSize - length of buffer
 *  \return dezimal value if valid, else -1
 */
int extract_ascii_value(char *buffer, int bufSize) {
	int numbers[bufSize];
	int counter;
	int result = 0;
	/* extract values from array */
	for (counter = 0 ; counter < bufSize ; counter++) {
		numbers[bufSize-1-counter] = (int)(buffer[counter]) * (int)intpow(256, counter);
		result += numbers[bufSize-1-counter];
	}
	return result;
}


/** \brief takes the bytes and extract the information bits from it
 * \param buffer - bytes with picture- and message-bits
 * \param bufsize - number of bytes for one byte of message (default 4)
 */
int extract_letter(char * buffer, int bufsize) {
	char resbuf[bufsize];
	resbuf[3] = buffer[3] & 3;
	resbuf[2] = (buffer[2] & 3) << 2;
	resbuf[1] = (buffer[1] & 3) << 4;
	resbuf[0] = (buffer[0] & 3) << 6;
	
	char result = 0;
	for (int i = 0 ; i < bufsize ; i++){
		result = result + resbuf[i];
	}
	return result;
}

	
/** \brief read header data from bmp-file
 * \param picture filedescriptor
 * \param picture_t 
 */	
picture_t read_fileheader(int file) {
	picture_t result;
	char buffer[4];  				/* DWORD size for BMP-format */
	read(file, &result.magicNumber[0], 1);		/* 0x00 magic number 'BM' */
	read(file, &result.magicNumber[1], 1);
	read(file, buffer, 4);			/* 0x02 size, but not reliable - not used */
	read(file, buffer, 4);			/* 0x06 software dependend - not used */
	read(file, buffer, 4);			/* 0x10 offset */
	result.offset = extract_ascii_value(buffer, 4);
	read(file, buffer, 4);			/* 0x0E BITMAPINFOHEADER - not used */
	read(file, buffer, 4);			/* 0x12 width */
	result.width = extract_ascii_value(buffer, 4);
	read(file, buffer, 4);			/* 0x16 height */
	result.height = extract_ascii_value(buffer, 4);
	read(file, buffer, 2);			/* 0x1A number of color planes - not used */
	read(file, buffer, 2);			/* 0x1c colorDepth */
	result.colorDepth = extract_ascii_value(buffer, 4);
	read(file, buffer, 4);			/* 0x1E - compression */
	result.compression = extract_ascii_value(buffer, 4);
	/* here more data possible, but not used. See wiki "windows bitmap" */
	return result;
}


void usage(){
	printf("extract a message out from a picture\n");
	printf("format of picture: bmp (windows bitmap), uncompressed\n");
	printf("maximum message length: 1024 characters\n");
	printf("\n");
	printf("usage:  readpic <filename.bmp>\n");
}
	

/** \brief main routine
 */
int main(int argc, char **argv) {
	/* usage */
	if (argc != 2) {
		usage();
		exit(EXIT_FAILURE);
	}
	
	char * filename = argv[1];
	int messageLength = 1024;	
	int filePicIn;
	picture_t picdata;
	
	/* open picture */
	if ((filePicIn = open(filename,O_RDONLY)) < 0){
		printf("error: picture not found, aborting !\n");
		exit(EXIT_FAILURE);
	} else {
		picdata = read_fileheader(filePicIn);		/* read picture header */
	}
	/* supported filetype ? */
	//printf("MN-> %c%c\n",picdata.magicNumber[0], picdata.magicNumber[1]);
	if ((picdata.magicNumber[0] != 'B') && (picdata.magicNumber[1] != 'M')) {
		printf("error: file type not supported, aborting !\n");
		exit(EXIT_FAILURE);
	}
	close(filePicIn);
	printf("read header, %d bytes\n", picdata.offset);
	/* open file again and skip header */
	filePicIn = open(filename,O_RDONLY);
	char letter;
	for (int counter = 0 ; counter < picdata.offset ; counter++) {
		read(filePicIn,&letter,1);
	}
	/* read bytes with message */
	printf("message: \n");
	char buffer[4];
	int messageCounter = 0;
	int running = 1;
	do {
		if (read(filePicIn, buffer, 4) == 0) {
			running = 0;
		}
		++messageCounter;
		printf("%c", extract_letter(buffer, 4));
		if (messageCounter > messageLength) {
			running = 0;
		}
	} while (running);	
	printf("\n");
	close(filePicIn);
}


