#include <stdio.h>
#include <sdlib.h>
// CPU internal states
uint8_t r[4];
uint16_t pc;
enum {RA=0, RY, RX, RS};
uint8_t flags;
enum { CF=0, ZF, IF, DF, BF, XX, VF, NF};
uint8_t m[65536]
// Running the code
// 0xFFFC contains the pointer to the first instruction address on the 6502
pc = m[0xFFFC]+256*m[0xFFFD]; // Fetch first instruction from address stored at 0xFFFC: 2 bytes, little endian!
r[RA]=0; r[RY]=0; r[RX]=0; r[RS]=0;

void write8(uint16_t addr, uint8_t val);
while(true) {
	uint8_t opcode = m[pc]; // fetch instruction opcode from memory
	switch(opcode) {
		case 0xAD:
		     print("0xAD \n");
		     uint16_t addr = m[pc+1] + 256*m[pc+2]
	             // load A register from given memory location addr
	             r[RA] = m[addr];
		     pc = pc + 3; // instruction is 3 bytes long
		     break;
		case 0x69:
		     print("0x69 \n");
		     r[RA] = r[RA] + m[pc + 1];
		     r[RA] = r[RA] + ((flags & (1<<CF))?1:0);
		     pc = pc +2; // this instruction has only 2 bytes
		     break;
		default:
		     print("unknown opcode");
	}
}

void write8(uint16_t addr, uint8_t val) {
  if (addr == 0xC000) { // memory mapped I/O output address
    putchar(val);
  }else {
   m[addr] = val; // ordinary write to memory
  }
}
