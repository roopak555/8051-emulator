// main.c – Minimal 8051 Emulator Skeleton (Day 1)
// -----------------------------------------------
// This is the foundation of your 8051 emulator.
// You will extend this file into multiple files later.
// For now, this builds and runs a simple test.

// Compile:
//   gcc -O2 -std=c11 main.c -o 8051emu
//
// Run:
//   ./8051emu

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
// temp edit to test git

/* ===============================
        MEMORY DEFINITIONS
   =============================== */

#define CODE_SIZE 0x10000   // 64 KB code memory
#define RAM_SIZE  0x100     // 256 bytes internal RAM

uint8_t code_mem[CODE_SIZE];
uint8_t ram[RAM_SIZE];

/* Memory clear */
void mem_reset() {
    memset(code_mem, 0, sizeof(code_mem));
    memset(ram, 0, sizeof(ram));
}

/* Load program bytes into code memory */
void mem_load(const uint8_t *data, size_t len, uint16_t addr) {
    if (addr + len > CODE_SIZE) {
        printf("Program too large!\n");
        return;
    }
    memcpy(&code_mem[addr], data, len);
}

/* ===============================
        CPU STRUCTURE
   =============================== */

typedef struct {
    uint8_t A;      // Accumulator
    uint8_t B;
    uint8_t PSW;    // Flags + register bank bits
    uint8_t SP;     // Stack Pointer
    uint16_t PC;    // Program Counter
    uint16_t DPTR;  // DPTR register
    bool running;
} CPU8051;

CPU8051 cpu;

/* ===============================
       CPU HELPER FUNCTIONS
   =============================== */

uint8_t fetch8() {
    uint8_t v = code_mem[cpu.PC];
    cpu.PC = (cpu.PC + 1) & 0xFFFF;
    return v;
}

void push8(uint8_t v) {
    cpu.SP++;
    ram[cpu.SP] = v;
}

uint8_t pop8() {
    uint8_t v = ram[cpu.SP];
    cpu.SP--;
    return v;
}

/* ===============================
     OPCODE DISPATCH TABLE
   =============================== */

typedef void (*handler_t)(uint8_t opcode);
handler_t op_table[256];

void op_unimplemented(uint8_t opcode) {
    printf("❌ Unimplemented opcode: 0x%02X at PC=0x%04X\n",
           opcode, cpu.PC - 1);
    cpu.running = false;
}

/* ---------- BASIC OPCODES ---------- */

// 0x00 — NOP
void op_nop(uint8_t opcode) {
    (void)opcode;
}

// 0x74 — MOV A,#data
void op_mov_a_imm(uint8_t opcode) {
    (void)opcode;
    cpu.A = fetch8();
}

// 0x24 — ADD A,#data
void op_add_a_imm(uint8_t opcode) {
    (void)opcode;
    uint8_t d = fetch8();
    uint16_t result = cpu.A + d;

    // Set carry flag (PSW.0)
    if (result > 0xFF)
        cpu.PSW |= 0x01;
    else
        cpu.PSW &= ~0x01;

    cpu.A = (uint8_t)result;
}

// 0x80 — SJMP rel
void op_sjmp(uint8_t opcode) {
    (void)opcode;
    int8_t rel = (int8_t)fetch8();  // signed
    cpu.PC = cpu.PC + rel;
}

/* ===============================
     INITIALIZATION
   =============================== */

void init_opcodes() {
    for (int i = 0; i < 256; i++)
        op_table[i] = op_unimplemented;

    op_table[0x00] = op_nop;
    op_table[0x74] = op_mov_a_imm;
    op_table[0x24] = op_add_a_imm;
    op_table[0x80] = op_sjmp;
}

void cpu_reset() {
    memset(&cpu, 0, sizeof(cpu));
    cpu.SP = 0x07;
    cpu.PC = 0x0000;
    cpu.running = true;

    mem_reset();
    init_opcodes();
}

/* ===============================
       EXECUTION LOOP
   =============================== */

void cpu_step() {
    uint8_t opcode = fetch8();
    op_table[opcode](opcode);
}

void cpu_run(uint32_t max_steps) {
    for (uint32_t i = 0; i < max_steps && cpu.running; i++)
        cpu_step();
}

/* ===============================
       SAMPLE PROGRAM
   =============================== */

void load_sample_program() {
    // MOV A,#5
    // ADD A,#3
    // SJMP loop
    const uint8_t prog[] = {
        0x74, 0x05,
        0x24, 0x03,
        0x80, 0xFE    // infinite loop
    };

    mem_load(prog, sizeof(prog), 0x0000);
}

/* ===============================
             MAIN
   =============================== */

int main() {
    cpu_reset();
    load_sample_program();

    cpu_run(50);

    printf("\n=== CPU STATE ===\n");
    printf("A   = 0x%02X\n", cpu.A);
    printf("PSW = 0x%02X\n", cpu.PSW);
    printf("PC  = 0x%04X\n", cpu.PC);
    printf("SP  = 0x%02X\n", cpu.SP);

    return 0;
}
