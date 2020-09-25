/*
 * include text message int o picture (steganography)
 * two lsb-bits are message
 * 
 * 2020-0901 V0.1, mrover, created
 */


 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <ctype.h>

#define BITSFILENAME "bits.txt"
#define OUTPUTFILENAME "new.bmp"

typedef struct hexNumber {
	char nibble[2];
} hex_t;


typedef struct picture {
	char magicNumber[2];
	int offset;
	int size;
	int width;
	int height;
	int colorDepth;
	int compression;
} picture_t;


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

/** \brief check validity of a sign to be a hexnumber
 *  \param ascii - number which will be checked
 *  \return 1 if valid / 0 if not valid
 */
int check_valid_hexnumber(int ascii) {
	if ((ascii >=48) && (ascii <=57)) {  // numbers 0-9
		return 1;
	}
	if ((ascii >=65) && (ascii <=70)) {  // characters A-F
		return 1;
	}
	if ((ascii >=97) && (ascii <=102)) {  // characters a-f
		return 1;
	}
	if (ascii == 0) {						// Null, Zero
		return 1;
	}
	return 0;
}

/** \brief convert letter in hexnumber according ascii
 *  eg "B" -> 0x42
 *  \param letter - converted letter
 *  \return hexnumber (type hex_t) 
  * */
hex_t letter_in_hex(char letter){
	int decnum = (int)letter;		// convert letter in decimal
	hex_t hexnum;
	hexnum.nibble[0] = 0;
	hexnum.nibble[1] = 0;
	
	int counter = 0;
	do {
		int rem = decnum % 16;
		if (rem<10) {
			rem = rem + 48;

		} else {
			rem = rem + 55;
		}
		hexnum.nibble[counter] = rem;
		counter++;
		decnum = decnum/16;
	} while (decnum != 0);	

	return hexnum;
}

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

/** \brief extract integer value of a bunch of letters (format in hex hexnumber)
 *  eg: FC00 -> equal dezimal 207
 *  reading direction right to left, checks if characters are valid for hexnumbers
 *  \param buffer - array of characters
 *  \param bufSize - length of buffer
 *  \return dezimal value if valid, else -1
 */
int extract_hex_value(char *buffer, int bufSize) {
	int numbers[bufSize];
	int counter;
	int result = 0;
	/* extract values from array */
	for (counter = 0 ; counter < bufSize ; counter++) {
		numbers[bufSize-1-counter] = get_dezimal_value(buffer[counter]) * (int)intpow(16, counter);
		result += numbers[bufSize-1-counter];
	}
	return result;
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

/** \brief print picture data on stdout
 *  \param picture
 *  \return none
 */
void print_picture_data(picture_t picture) {
	printf("magic number: %c%c\n",(char)picture.magicNumber[0], (char)picture.magicNumber[1]);
	printf("offset      :%d\n",picture.offset);
	printf("size        :%d\n",picture.size);
	printf("width       :%d\n",picture.width);
	printf("height      :%d\n",picture.height);
	printf("color depth :%d\n",picture.colorDepth);
	printf("compression :%d\n",picture.compression);	
}

/** \brief print array on console
 * \param array of integers
 * \param size of array
 * \return none
 */
void print_buffer(char * buffer, int size) {
	for (int counter = 0 ; counter < size ; counter++) {
		printf("%d ", buffer[counter]);
	}
	printf("\n");
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


/** \brief converts textfile in single message bits
 * take letters from messagefile and split it into 4 2bit-groups
 * writes 2bit-groups as single characters in file named BITSFILENAME
 * \param filedescriptor for textfile
 * \return 1 if ok, else 0
 */
int create_message_bits(char * textfileName) {
	FILE * textfile;
	printf("open messagefile: %s\n", textfileName);
	if ((textfile = fopen(textfileName, "r")) == NULL) {
		printf("error: no messagefile found\n");
		return 0;
	} else {
		int msg_buffer[4];					/* two bits together for one pixel */
		int letter;
		printf("messagefile found...\n");
		FILE * bitsfile;
		bitsfile = fopen(BITSFILENAME, "w+");
		while ((letter = fgetc(textfile)) != EOF) { 
			/* read letter ; create 4 bits ; write bits to file */
			msg_buffer[0] = letter & 192;
			msg_buffer[0] = msg_buffer[0]>>6;
			msg_buffer[1] = letter & 48;
			msg_buffer[1] = msg_buffer[1]>>4;
			msg_buffer[2] = letter & 12;
			msg_buffer[2] = msg_buffer[2]>>2;
			msg_buffer[3] = letter & 3;
			//printf("buffer: %d %d %d %d\n",msg_buffer[0], msg_buffer[1], msg_buffer[2], msg_buffer[3]);
			for (int counter = 0 ; counter < 4 ; counter++) {
				fputc(msg_buffer[counter], bitsfile);
			}
		}
		printf("bitsfile created: %s\n", BITSFILENAME);
		fclose(textfile);
		fclose(bitsfile);
		return 1;
	} 
} 




/** \brief changes bits in picture with bits from text
 * filedescriptor must stand on the correct place after the header
 * \param picture filedescriptor for picture
 * \param text filedescriptor for textfile
 * \return none
 */
void change_picture(picture_t picdata, char * pictureFilename, char * messageFilename) {
	int pictureIn = open(pictureFilename, O_RDONLY);						/* original picture */
	int pictureOut = open(OUTPUTFILENAME, O_WRONLY | O_CREAT, S_IRWXU);		/* new picture */
	int messageFile = open(messageFilename, O_RDONLY);						/* message file */	
	
	char buffer[picdata.offset];
	read(pictureIn, buffer, picdata.offset);								/* copy header */
	write(pictureOut, buffer, picdata.offset);
	printf("header copied %i bytes...\n",picdata.offset);
	
	int stillData = 1;
	char msg, pixelvalue;
	int msgLength = 0;
	while (read(pictureIn, &pixelvalue, 1) > 0)  {
		if (stillData != 0) {
			if (read(messageFile, &msg, 1) == 0) {			/* no message left */
				stillData = 0; 
			}
		}
		if (stillData == 1) {								/* change pixel and write to file*/
			pixelvalue = pixelvalue & 252;
			pixelvalue = pixelvalue | msg;
			write(pictureOut, &pixelvalue, 1);
			msgLength++;
		} else {											/* only copy pixel */
			write(pictureOut, &pixelvalue, 1);
		}
	}
	printf("Message length %i bytes written\n", msgLength);
	close(pictureOut);
	close(pictureIn);
	close(messageFile);
}


void usage(){
	printf("create a steganograpy-picture\n");
	printf("works with bmp (windows bitmap), no datacompression\n");
	printf("\n");
	printf("usage:  createpic <picturefilename.bmp> <textmessage.txt>\n");
	printf("\n");
}


int main(int argc, char **argv) {
	/* open file */
	if (argc != 3) {
		usage();
		exit(EXIT_FAILURE);
	}
	char *pictureFileName = argv[1];
	char *messageFileName = argv[2];

	int filePicIn;
	picture_t picdata;
	
	/* open picture */
	if ((filePicIn = open(pictureFileName,O_RDONLY)) < 0){
		printf("error: picture not found, aborting !\n");
		exit(EXIT_FAILURE);
	} else {
		picdata = read_fileheader(filePicIn);		/* read picture header */
		close(filePicIn);
	}
	/* supported filetype ? */
	if ((picdata.magicNumber[0] != 'B') && (picdata.magicNumber[1] != 'M')) {
		printf("error: file type not supported, aborting !\n");
		exit(EXIT_FAILURE);
	}
	/* message bits */
	if (create_message_bits(messageFileName) == 0) {
		printf("error: messagefile ???, aborting !\n");
		exit(EXIT_FAILURE);
	}
	/* change picture */
	printf("start merging...\n");
	change_picture(picdata, pictureFileName, BITSFILENAME);
	print_picture_data(picdata);
	printf("picture created: %s\n", OUTPUTFILENAME);
}


