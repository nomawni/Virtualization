#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

volatile uint8_t *TDR = (uint8_t *)0xc0000000;
volatile uint8_t *RDR = (uint8_t *)0xc0000004;
volatile uint8_t *SR = (uint8_t *)0xc0000008;

uint32_t m[64 * 1024];
uint32_t pc;
uint32_t r[32];

uint8_t readchar(void)
{
    while ((*SR & 1) == 0)
    {
    }            // loop as long as bit 0 == 0
    return *RDR; // return character from RDR
}

void writechar(uint8_t c)
{
    while ((*SR & 2) == 0)
    {
    }         // loop as long as bit 1 == 0
    *TDR = c; // write character to TDR
}

void load(char *filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        printf("Can not open file %s\n", filename);
        exit(1);
    }
    if (read(fd, (void *)m, 64 * 1024 * 4) < 0)
    {
        printf("Error reading file %s\n", filename);
        exit(1);
    }
    close(fd);
}

int main(int argc, char **argv)
{
    uint32_t instruct;
    uint32_t opcode;

    if (argc != 2)
    {
        printf("Usage: %s memory\n", argv[0]);
        exit(1);
    }
    load(argv[1]);

    pc = 0;
    while (1)
    {
        instruct = m[pc / 4];
        opcode = instruct & 0b1111111;
        switch (opcode)
        {
        case 0b1101111: // jal instruction
            printf("jal\n");
            break;
        case 0b1100111: // jalr instruction
            printf("jalr\n");
            break;
        case 0b1100011: // branch instruction
            printf("branch instruction\n");
            break;
        case 0b0000011: // load instruction
            printf("load instruction\n");
            break;
        case 0b0100011: // store instruction or csr instruction
            printf("store or csr instruction\n");
            break;
        case 0b0010011: // arithmetic instruction
            printf("arithmetic instruction\n");
            break;
        case 0b0110011: // arithmetic instruction
            printf("arithmetic instruction\n");
            break;
        case 0b0001111: // fence instruction
            printf("fence instruction\n");
            break;
            /*case 0b1110011: // system instruction
                printf("system instruction\n");
                if ((instr & 0b1111111) == 0b1110011 && (instr >> 12) & 0b1111 */
        case 0b1110011: // system instruction
            printf("system instruction\n");
            if ((instruct & 0b1111111) == 0b1110011 && (instruct >> 12) & 0b1111 == 0b1101)
            {
                uint8_t c = (instruct >> 20) & 0xff;
                writechar(c);
            }
            else if ((instruct & 0b1111111) == 0b1110011 && (instruct >> 12) & 0b1111 == 0b1100)
            {
                uint8_t c = readchar();
                r[0] = c;
            }
            break;
        }
        pc += 4;
    }
    return 0;
}