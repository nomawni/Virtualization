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

volatile uint8_t *TDR = (uint8_t *)0xc0000000;
volatile uint8_t *RDR = (uint8_t *)0xc0000004;
volatile uint8_t *SR = (uint8_t *)0xc0000008;

enum CSR
{
    CSR_FFLAGS = 0,
    CSR_FRM,
    CSR_FCSR,
    CSR_CYCLE,
    CSR_TIME,
    CSR_INSTRET,
    CSR_HPMCOUNTER3,
    CSR_HPMCOUNTER4,
    CSR_HPMCOUNTER5,
    CSR_HPMCOUNTER6,
    CSR_HPMCOUNTER7,
    CSR_HPMCOUNTER8,
    CSR_HPMCOUNTER9,
    CSR_HPMCOUNTER10,
    CSR_HPMCOUNTER11,
    CSR_HPMCOUNTER12,
    CSR_HPMCOUNTER13,
    CSR_HPMCOUNTER14,
    CSR_HPMCOUNTER15,
    CSR_HPMCOUNTER16,
    CSR_HPMCOUNTER17,
    CSR_HPMCOUNTER18,
    CSR_HPMCOUNTER19,
    CSR_HPMCOUNTER20,
    CSR_HPMCOUNTER21,
    CSR_HPMCOUNTER22,
    CSR_HPMCOUNTER23,
    CSR_HPMCOUNTER24,
    CSR_HPMCOUNTER25,
    CSR_HPMCOUNTER26,
    CSR_HPMCOUNTER27,
    CSR_HPMCOUNTER28,
    CSR_HPMCOUNTER29,
    CSR_HPMCOUNTER30,
    CSR_HPMCOUNTER31,
    NUM_CSR
};

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

uint32_t csr_regs[NUM_CSR];

void csr_init()
{
    for (int i = 0; i < NUM_CSR; i++)
    {
        csr_regs[i] = 0;
    }
}

uint32_t get_csr(enum CSR csr)
{
    return csr_regs[csr];
}

void set_csr(enum CSR csr, uint32_t value)
{
    csr_regs[csr] = value;
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

void execute_csr_instruction(uint32_t instruction)
{
    uint32_t csr = (instruction >> 20) & 0xfff;
    uint32_t rd = (instruction >> 7) & 0x1f;
    uint32_t rs1 = (instruction >> 15) & 0x1f;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    switch (funct3)
    {
    case 0: // csrrs instruction
        uint32_t csr_val = get_csr(csr);
        r[rd] = csr_val;
        csr_val |= 1 << r[rs1];
        set_csr(csr, csr_val);
        break;
    case 1: // csrrw instruction
        uint32_t csr_val = get_csr(csr);
        r[rd] = csr_val;
        csr_val = r[rs1];
        set_csr(csr, csr_val);
        break;
    case 2: // csrrc instruction
        uint32_t csr_val = get_csr(csr);
        r[rd] = csr_val;
        csr_val &= ~(1 << r[rs1]);
        set_csr(csr, csr_val);
        break;
    case 3: // csrrci instruction
        uint32_t csr_val = get_csr(csr);
        r[rd] = csr_val;
        if (r[rs1] == 0)
        {
            csr_val &= ~(1 << r[rs1]);
        }
        set_csr(csr, csr_val);
        break;
    case 4:                                     // csrrwi instruction
                                                // amoswap instruction
        uint32_t mem_val = get_mem_val(r[rs1]); // get current value of memory address
        set_mem_val(r[rs1], r[rd]);             // store the value of rd in memory address
        r[rd] = mem_val;                        // return the value of memory address to rd
        break;
    case 5:                                     // amoadd instruction
        uint32_t mem_val = get_mem_val(r[rs1]); // get current value of memory address
        r[rd] = mem_val + r[rd];
        set_mem_val(r[rs1], r[rd]); // store the value of rd in memory address
        break;
    case 6:                                     // amoxor instruction
        uint32_t mem_val = get_mem_val(r[rs1]); // get current value of memory address
        r[rd] = mem_val ^ r[rd];
        set_mem_val(r[rs1], r[rd]); // store the value of rd in memory address
        break;
    case 7:                                     // amoand instruction
        uint32_t mem_val = get_mem_val(r[rs1]); // get current value of memory address
        r[rd] = mem_val & r[rd];
        set_mem_val(r[rs1], r[rd]); // store the value of rd in memory address
        break;
    case 8:                                     // amoor instruction
        uint32_t mem_val = get_mem_val(r[rs1]); // get current value of memory address
        r[rd] = mem_val | r[rd];
        set_mem_val(r[rs1], r[rd]); // store the value of rd in memory address
        break;
    case 9:                                     // amomax instruction
        uint32_t mem_val = get_mem_val(r[rs1]); // get current value of memory address
        if (r[rd] > mem_val)
        {
            set_mem_val(r[rs1], r[rd]); // store the value of rd in memory address
        }
        r[rd] = max(r[rd], mem_val);
        break;
    case 10:                                    // amomaxu instruction
        uint32_t mem_val = get_mem_val(r[rs1]); // get current value of memory address
        if ((uint32_t)r[rd] > (uint32_t)mem_val)
        {
            set_mem_val(r[rs1], r[rd]); // store the value of rd in memory address
        }
        r[rd] = max((uint32_t)r[rd], (uint32_t)mem_val);
        break;
    case 11:                                    // amomin instruction
        uint32_t mem_val = get_mem_val(r[rs1]); // get current value of memory address
        if (r[rd] < mem_val)
        {
            set_mem_val(r[rs1], r[rd]); // store the value of rd in memory address
        }
        r[rd] = min(r[rd], mem_val);
        break;
    case 12:                                    // amominu instruction
        uint32_t mem_val = get_mem_val(r[rs1]); // get current value of memory address
        if ((uint32_t)r[rd] < (uint32_t)mem_val)
        {
            set_mem_val(r[rs1], r[rd]); // store the value of rd in memory address
        }
        r[rd] = min((uint32_t)r[rd], (uint32_t)mem_val);
        break;
    default:
        printf("Invalid instruction %x  at pc %x    ", instruction, pc);
        exit(1);
        break;
    }
}

int main(int argc, char **argv)
{
    uint32_t instruct;
    uint32_t opcode;
    csr_init();
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
            switch (branch_type)
            {
            case 0b000:
                printf("beq\n");
                break;
            case 0b001:
                printf("bne\n");
                break;
            case 0b100:
                printf("blt\n");
                break;
            case 0b101:
                printf("bge\n");
                break;
            case 0b110:
                printf("bltu\n");
                break;
            case 0b111:
                printf("bgeu\n");
                break;
            default:
                printf("Invalid branch type\n");
                break;
            }
            break;
        case 0b0000011:
            rd = (instruct >> 7) & 0b11111;
            rs1 = (instruct >> 15) & 0b11111;
            load_type = (instruct >> 12) & 0b111;
            switch (load_type)
            {
            case 0b000:
                printf("lb r%d, r%d\n", rd, rs1);
                break;
            case 0b001:
                printf("lh r%d, r%d\n", rd, rs1);
                break;
            case 0b010:
                printf("lw r%d, r%d\n", rd, rs1);
                break;
            case 0b100:
                printf("lbu r%d, r%d\n", rd, rs1);
                break;
            case 0b101:
                printf("lhu r%d, r%d\n", rd, rs1);
                break;
            default:
                printf("Invalid load type\n");
                break;
            }
            break;
        case 0b0100011:
            rd = (instruct >> 7) & 0b11111;
            rs1 = (instruct >> 15) & 0b11111;
            rs2 = (instruct >> 20) & 0b11111;
            uint32_t imm = (instruct >> 20) & 0b11111;
            load_type = (instruct >> 12) & 0b111;
            switch (load_type)
            {
            case 0b000:
                printf("sb r%d, r%d, %d\n", rd, rs1, imm);
                break;
            case 0b001:
                printf("sh r%d, r%d, %d\n", rd, rs1, imm);
                break;
            case 0b010:
                printf("sw r%d, r%d, %d\n", rd, rs1, imm);
                break;
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
            switch (instruction_type)
            {
            case 0b000:
                printf("addi r%d, r%d, %d\n", rd, rs1, shamt);
                break;
            case 0b010:
                printf("lbu r%d, %d(r%d)\n", rd, imm, rs1);
                break;
            case 0b011:
                printf("lbu r%d, r%d, r%d\n", rd, rs1, rs2);
                break;
            case 0b100:
                printf("lbu r%d, %d(r%d)\n", rd, imm, rs2);
                break;
            case 0b110:
                printf("lbu r%d, r%d, %d\n", rd, rs1, imm);
                break;
            case 0b111:
                printf("lbu r%d, r%d, r%d\n", rd, rs1, rs2);
                break;
            case 0b001:
                printf("slli r%d, r%d, %d\n", rd, rs1, shamt);
                break;
            case 0b101:
                printf("srli r%d, r%d, %d\n", rd, rs1, shamt);
                break;
            case 0b101:
                printf("srai r%d, r%d, %d\n", rd, rs1, shamt);
                break;
            default:
                printf("Invalid instruction type\n");
                break;
            }
            break;
        case 0b0110011:
            rd = (instruct >> 7) & 0b11111;
            rs1 = (instruct >> 15) & 0b11111;
            rs2 = (instruct >> 20) & 0b11111;
            instruction_type = (instruct >> 12) & 0b111;
            switch (instruction_type)
            {
            case 0b000:
                if ((instruct >> 25) & 0b1)
                {
                    printf("sub r%d, r%d, r%d\n", rd, rs1, rs2);
                }
                else
                {
                    printf("add r%d, r%d, r%d\n", rd, rs1, rs2);
                }
                break;

            case 0b001:
                printf("sll r%d, r%d, r%d\n", rd, rs1, rs2);
                break;
            case 0b010:
                printf("slt r%d, r%d, r%d\n", rd, rs1, rs2);
                break;
            case 0b011:
                printf("sltu r%d, r%d, r%d\n", rd, rs1, rs2);
                break;
            case 0b100:
                printf("xor r%d, r%d, r%d\n", rd, rs1, rs2);
                break;
            case 0b101:
                printf("srl r%d, r%d, r%d\n", rd, rs1, rs2);
                break;
            case 0b110:
                printf("or r%d, r%d, r%d\n", rd, rs1, rs2);
                break;
            case 0b101:
                printf("sra r%d, r%d, r%d\n", rd, rs1, rs2);
                break;
            case 0b111:
                printf("and r%d, r%d, r%d\n", rd, rs1, rs2);
                break;
            default:
                printf("Invalid instruction type\n");
                break;
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
                uint8_t c = (instruct >> 20) & 0xff;
                writechar(c);
                printf("ECALL\n");
            }
            else if (instruction == 0b0000000001)
            {
                uint8_t c = readchar();
                r[0] = c;
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
