#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define TYPE_LEN 4
#define AVI_HEAD 0x41564920
#define RESTORE_FILE "restore.avi"
#define VERSION "ver 1.1"

union w {
	int a;
	char b;
}udata;

char *help_msg = {"\
a tool to modify/restore file-type value in file header\n\
\n\
	--encrypt <filename> <value> modify file-type with the given 4-Bytes \n\
                                     hex value, save the original value to\n\
                                     " RESTORE_FILE "\n\
	--decrypt <filename> restore file-type from " RESTORE_FILE "\n\
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
	uint32_t avi_head = AVI_HEAD;
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

			if(is_little_endian()) {
				value = le2be(value);
				avi_head = le2be(avi_head);
			}
			
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

			restore = fopen(RESTORE_FILE, "wb");
			if(input == NULL) {
				printf("Unable to create file " RESTORE_FILE ".\n");
				fclose(input);
				exit(1);
			}

			fread(buff, 1, 4, input);
			len = fwrite(&avi_head, 1, 4, restore);		// write avi_head first
			len = fwrite(&buff, 1, 4, restore);
			if(len < 1) {
				printf("Failed to write to file " RESTORE_FILE ", disk space might be low.\n");
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

			restore = fopen(RESTORE_FILE, "rb");
			if(input == NULL) {
				printf("Unable to create file " RESTORE_FILE ".\n");
				fclose(input);
				exit(1);
			}

			// restore value to input
			fread(buff, 1, 4, restore);		// skip avi_head
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
