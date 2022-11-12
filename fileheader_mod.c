#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define TYPE_LEN 4
#define VERSION "ver 1.0"

union w {
	int a;
	char b;
}udata;

char *help_msg = {"\
a tool to modify/restore file-type value in file header\n\
\n\
	--encrypt <filename> <value> modify file-type with the given 4-Bytes \n\
                                     hex value, save the original value to\n\
                                     restore.bin\n\
	--decrypt <filename> restore file-type from restore.bin\n\
	--help show this message\n\
"
VERSION
};

void print_help() {
	printf("%s\n", help_msg);
}

// check if system is little endian
bool is_little_endian() {
	udata.a = 1;
	return (udata.b == 1); 
}

// little to big-endian
uint32_t le2be(uint32_t v) {
	int i;
	char *b1, *b2;
	uint32_t be;
	
	b1 = (char*)&v;
	b1 += 3;
	b2 = (char*)&be;
	for(i = 0; i < 4; i++) {
		*(b2+i) = *(b1-i);
	}

	return be;
}

int main(int argc, char **argv) {
	uint32_t original_value;
	uint32_t value;
	FILE *input;
	FILE *restore;
	size_t len;
	char buff[TYPE_LEN+1];

	if(argc == 1) {
		print_help();
	} 
	else {
		if(!strcmp(argv[1], "--encrypt") && argc == 4) {	
			input = fopen(argv[2], "rb+");
			if(input == NULL) {
				perror("fopen");
				exit(1);
			}
			value = strtod(argv[3]+0, NULL);

			// padding, we think user might forget to pad to 4-Bytes
			if(value <= 0xff)
				value *= pow(16, 2*3);
			else if(value <= 0xffff)
				value *= pow(16, 2*2);
			else if(value <= 0xffffff)
				value *= pow(16, 2*1);	

			if(is_little_endian())
				value = le2be(value);
			
			// print wrote bytes
			printf("Wrote: ");
			for(int j=0; j<4; j++) {
				uint32_t o;
				o = *(((uint8_t*)&value)+j);
				if(o <= 0xf)
					printf("0");
				printf("%x ", o);
			}
			printf("\n");

			restore = fopen("restore.bin", "wb");
			if(input == NULL) {
				printf("Unable to create file restore.bin.\n");
				fclose(input);
				exit(1);
			}

			fread(buff, 1, 4, input);
			len = fwrite(&buff, 1, 4, restore);
			if(len < 1) {
				printf("Failed to write to file restore.bin, disk space might be low.\n");
				fclose(input);
				fclose(restore);
				exit(1);
			}
			fclose(restore);

			// write new value to input 
			rewind(input);
			fwrite(&value, 4, 1, input);
			fclose(input);

			printf("Encrypt success.\n");
		}
		else if(!strcmp(argv[1], "--decrypt") && argc == 3) {
			input = fopen(argv[2], "rb+");
			if(input == NULL) {
				perror("fopen");
				exit(1);
			}

			restore = fopen("restore.bin", "rb");
			if(input == NULL) {
				printf("Unable to create file restore.bin.\n");
				fclose(input);
				exit(1);
			}

			// restore value to input
			fread(buff, 1, 4, restore);
			fwrite(buff, 1, 4, input);
			fclose(restore);
			fclose(input);

			printf("Decrypt success.\n");
		}
		else
			print_help();
	}
}
