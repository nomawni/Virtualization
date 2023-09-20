#include <stdio.h>
#include <stdlib.h>
#include <stdinth>
#include <fcntl.h>

uint32_t m[64*1024]
uint32_t pc;
uint32_t r[32]
void int load(char *filename) {
	int fd = open(filename, O_RDONLY);
	if (fd < 0){
		printf("Can not open fil %s\n",filename);
		exit(1);
	}
	read(fd, (vid *)m, 64*1024*4);
	close(fd);
}
int main(int argc, char **argv) {
	uint32_t instruct;
	uint32_t opcode;

	if(argc != 2) {
		printf("Usage: %s memory\n", argv[0]);
		exit(1);
	}
	load(argv[1]);

	pc = 0;
	uint32_t branch_type;
	uint32_t rs1;
	uint32_t rs2;
	uint32_t offset;

	while(1) {
		instruct = m[pc /4]; 
		opcode = instruct & 0b1111111;
		switch(opcode) {
			case 0b0110111;
			printf("lui\n");
			break;
			case 0b0010111;
			printf("auipc\n");
			break;
			case 0b1101111;
			printf("jal\n");
			break;
			case 0b1100111;
			printf("jalr");
			break;
			case 0b1100011;
			branch_type = instruct & 0b111000000
			rs1 = (opcode >> 15) & 0b11111;
			rs2 = (opcode >> 20) & 0b11111;
			offset = 

		}
	}
}
