#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

uint32_t m[64 * 1024];
uint32_t pc;
uint32_t r[32];

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
    uint32_t branch_type;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t offset;
    uint32_t rd;
    uint32_t load_type;
    uint32_t instruction_type;
    uint32_t instruction;
    while (1)
    {
        instruct = m[pc / 4];
        opcode = instruct & 0b1111111;
        switch (opcode)
        {
        case 0b0110111:
            printf("lui\n");
            break;
        case 0b0010111:
            printf("auipc\n");
            break;
        case 0b1101111:
            printf("jal\n");
            break;
        case 0b1100111:
            printf("jalr\n");
            break;
        case 0b1100011:
            branch_type = instruct & 0b111000000;
            rs1 = (opcode >> 15) & 0b11111;
            rs2 = (opcode >> 20) & 0b11111;
            offset = (opcode >> 7) & 0b111111111111111111;
            if (branch_type == 0b000)
            {
                printf("beq\n");
            }
            else if (branch_type == 0b001)
            {
                printf("bne\n");
            }
            else if (branch_type == 0b100)
            {
                printf("blt\n");
            }
            else if (branch_type == 0b101)
            {
                printf("bge\n");
            }
            else if (branch_type == 0b110)
            {
                printf("bltu\n");
            }
            else if (branch_type = 0b111)
            {
                printf("bgeu\n");
            }
            break;
        case 0b0000011:
            rd = (instruct >> 7) & 0b11111;
            rs1 = (instruct >> 15) & 0b11111;
            load_type = (instruct >> 12) & 0b111;
            if (load_type == 0b000)
            {
                printf("lb r%d, r%d\n", rd, rs1);
            }
            else if (load_type == 0b001)
            {
                printf("lh r%d, r%d\n", rd, rs1);
            }
            else if (load_type == 0b010)
            {
                printf("lw r%d, r%d\n", rd, rs1);
            }
            else if (load_type == 0b100)
            {
                printf("ld r%d, r%d\n", rd, rs1);
            }
            else if (load_type == 0b101)
            {
                printf("lbu r%d, r%d\n", rd, rs1);
            }
            break;
        case 0b0100011:
            rd = (instruct >> 7) & 0b11111;
            rs1 = (instruct >> 15) & 0b11111;
            rs2 = (instruct >> 20) & 0b11111;
            uint32_t imm = (instruct >> 20) & 0b11111;
            load_type = (instruct >> 12) & 0b111;
            if (load_type == 0b000)
            {
                printf("lh r%d, r%d, %d\n", rd, rs1, imm);
            }
            else if (load_type == 0b001)
            {
                printf("lh r%d, r%d, r%d\n", rd, rs1, rs2);
            }
            else if (load_type == 0b010)
            {
                printf("lh r%d, r%d, %d(r%d)\n", rd, rs1, imm, rs2);
            }
            break;
        case 0b1000011:
            printf("lw\n");
            break;
        case 0b0010011:
            rd = (instruct >> 7) & 0b11111;
            rs1 = (instruct >> 15) & 0b11111;
            // uint32_t instruction_type = (instruct >> 12) & 0b111;
            uint32_t shamt = (instruct >> 20) & 0b11111;
            instruction_type = (instruct >> 12) & 0b111;
            if (instruction_type == 0b000)
            {
                printf("lbu r%d, r%d\n", rd, rs1);
            }
            else if (instruction_type == 0b010)
            {
                printf("lbu r%d, %d(r%d)\n", rd, imm, rs1);
            }
            else if (instruction_type == 0b011)
            {
                printf("lbu r%d, r%d, r%d\n", rd, rs1, rs2);
            }
            else if (instruction_type == 0b100)
            {
                printf("lbu r%d, %d(r%d)\n", rd, imm, rs2);
            }
            else if (instruction_type == 0b110)
            {
                printf("lbu r%d, r%d, %d\n", rd, rs1, imm);
            }
            else if (instruction_type == 0b111)
            {
                printf("lbu r%d, r%d, r%d\n", rd, rs1, rs2);
            }
            if (instruction_type == 0b001)
            {
                printf("slli r%d, r%d, %d\n", rd, rs1, shamt);
            }
            else if (instruction_type == 0b101)
            {
                printf("srli r%d, r%d, %d\n", rd, rs1, shamt);
            }
            else if (instruction_type == 0b101)
            {
                printf("srai r%d, r%d, %d\n", rd, rs1, shamt);
            }
            break;
        case 0b0110011:
            rd = (instruct >> 7) & 0b11111;
            rs1 = (instruct >> 15) & 0b11111;
            rs2 = (instruct >> 20) & 0b11111;
            instruction_type = (instruct >> 12) & 0b111;
            if (instruction_type == 0b000)
            {
                printf("add r%d, r%d, r%d\n", rd, rs1, rs2);
            }
            else if (instruction_type == 0b000)
            {
                printf("sub r%d, r%d, r%d\n", rd, rs1, rs2);
            }
            else if (instruction_type == 0b001)
            {
                printf("sll r%d, r%d, r%d\n", rd, rs1, rs2);
            }
            else if (instruction_type == 0b010)
            {
                printf("slt r%d, r%d, r%d\n", rd, rs1, rs2);
            }
            else if (instruction_type == 0b011)
            {
                printf("sltu r%d, r%d, r%d\n", rd, rs1, rs2);
            }
            else if (instruction_type == 0b100)
            {
                printf("xor r%d, r%d, r%d\n", rd, rs1, rs2);
            }
            else if (instruction_type == 0b101)
            {
                printf("srl r%d, r%d, r%d\n", rd, rs1, rs2);
            }
            else if (instruction_type == 0b101)
            {
                printf("sra r%d, r%d, r%d\n", rd, rs1, rs2);
            }
            else if (instruction_type == 0b110)
            {
                printf("or r%d, r%d, r%d\n", rd, rs1, rs2);
            }
            else if (instruction_type == 0b111)
            {
                printf("and r%d, r%d, r%d\n", rd, rs1, rs2);
            }
            break;

        case 0b0001011:
            printf("sb\n");
            break;
        case 0b0101011:
            printf("sh\n");
            break;
        case 0b1001011:
            printf("sw\n");
            break;
        case 0b0110001:
            printf("fence\n");
            break;
        case 0b0001111:
            rs1 = (instruct >> 15) & 0b11111;
            rd = (instruct >> 7) & 0b11111;
            uint32_t succ = (instruct >> 20) & 0b11111;
            uint32_t pred = (instruct >> 24) & 0b11111;
            uint32_t fm = (instruct >> 28) & 0b11111;
            instruction_type = (instruct >> 12) & 0b111;
            if (instruction_type == 0b000)
            {
                printf("fence r%d, r%d, %d, %d, %d\n", rd, rs1, succ, pred, fm);
            }
            break;
        case 0b1110001:
            printf("csrrw\n");
            break;
        case 0b1110011:
            instruction = (instruct >> 20) & 0b1111111111;
            if (instruction == 0b0000000000)
            {
                printf("ECALL\n");
            }
            else if (instruction == 0b0000000001)
            {
                printf("EBREAK\n");
            }
            break;
        case 0b1110101:
            printf("csrrc\n");
            break;
        case 0b1110100:
            printf("csrrwi\n");
            break;
        default:
            printf("Unknown instruction %08x at PC %08X\n", instruction, pc);
            break;
        }
        pc = pc + 4;
    }
    return 0;
}
