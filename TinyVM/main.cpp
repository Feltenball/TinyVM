/*
Goal:
	- Create a VM that simulates the LC-3 educational compute rarchitecture

LC-3 has
	- 65536 memory locations which store 16 bit values (total storage 128KB)
	- 10 total 16-bit registers
		- 8 general purpose registers (R0-R7)
		- 1 program counter (PC)
		- 1 condition flags (COND)
	- 16 opcodes
		- Each opcode represents one task that the CPU can do
	- 3 condition flags
		- Condition flags are used for the CPU to signal various situations
		- In LC-3 they indicate the sign of the previous calculation
	- Two memory mapped registers
		- The keyboard status register KBSR and keyboard data register KBDR
		- KBSR indicates whether a key has been pressed
		- KBDR identifies which key was pressed
		- Memory mapped registers are typically used to interact with special hardware devices
		- They are not accessible from the normal register table, and a special address is reserved for them in memory
		- To read and write to these registers, just read and write to their memory location
A tool called an assembler translates each assembly instruction into a 16-bit binary instruction
that the VM can understand. This is our machine code. 
*/

#pragma warning(disable:4996)

#include <iostream>
#include <math.h>

#include <stdio.h>
#include <stdint.h>
#include <signal.h>
/* windows only */
#include <Windows.h>
#include <conio.h>  // _kbhit

// Small details (not related to VM, can ignore, they are for input buffering on windows)
HANDLE hStdin = INVALID_HANDLE_VALUE;
DWORD fdwMode, fdwOldMode;

void disable_input_buffering() {
	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(hStdin, &fdwOldMode); /* save old mode */
	fdwMode = fdwOldMode
		^ ENABLE_ECHO_INPUT  /* no input echo */
		^ ENABLE_LINE_INPUT; /* return when one or
								more characters are available */
	SetConsoleMode(hStdin, fdwMode); /* set new mode */
	FlushConsoleInputBuffer(hStdin); /* clear buffer */
}

void restore_input_buffering() {
	SetConsoleMode(hStdin, fdwOldMode);
}

uint16_t check_key() {
	return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
}

void handle_interrupt(int signal) {
	restore_input_buffering();
	printf("\n");
	exit(-2);
}

// Hardware data for the VM

// Memory
constexpr int MEMORY_MAX = 65536;			// Max amount of memory locations
uint16_t memory[MEMORY_MAX];				// Memory locations store 16-bit values

// Registers
enum {
	R_R0 = 0,								// General purpose registers
	R_R1,
	R_R2,
	R_R3,
	R_R4,
	R_R5,
	R_R6,
	R_R7,
	R_PC,									// Program counter
	R_COND,									// Condition flags
	R_COUNT
};
uint16_t reg[R_COUNT];

// Opcodes
enum {
	OP_BR = 0,								// Branch
	OP_ADD,									// Add
	OP_LD,									// Load
	OP_ST,									// Store
	OP_JSR,									// Jump register
	OP_AND,									// Bitwise and
	OP_LDR,									// Load register
	OP_STR,									// Store register
	OP_RTI,									// Unused
	OP_NOT,									// Bitwise not
	OP_LDI,									// Load indirect
	OP_STI,									// Store indirect
	OP_JMP,									// Jump
	OP_RES,									// Reserved 
	OP_LEA,									// Load effect address
	OP_TRAP									// Execute trap
};

// Condition flags
enum {
	FL_POS = 1 << 0,						// Positive
	FL_ZRO = 1 << 1,						// Zero
	FL_NEG = 1 << 2,						// Negative
};

// Memory mapped registers
enum {
	MR_KBSR = 0xFE00,						// Keyboard status
	MR_KBDR = 0xFE02,						// Keyboard data
};

// Reading LC-3 programs into memory

uint16_t swap16(uint16_t x) {
	return (x << 8) | (x >> 8); 
}

void read_image_file(FILE* file) {
	uint16_t origin;						// Where in memory to place the image
	fread(&origin, sizeof(origin), 1, file);
	origin = swap16(origin); 

	uint16_t max_read = MEMORY_MAX - origin;
	uint16_t* p = memory + origin;
	size_t read = fread(p, sizeof(uint16_t), max_read, file); 

	// Switch to little endian
	while (read-- > 0) {
		*p = swap16(*p);
		p++; 
	}
}

int read_image(const char* image_path) {
	FILE* file = fopen(image_path, "rb");
	if (!file) { return 0; };
	read_image_file(file);
	fclose(file);
	return 1;
}

// Memory reading / writing

void mem_write(uint16_t address, uint16_t val) {
	memory[address] = val;
}

uint16_t mem_read(uint16_t address) {
	// A special case is needed for the memory mapped registers
	if (address == MR_KBSR) {
		if (check_key()) {
			memory[MR_KBSR] = (1 << 15); 
			memory[MR_KBDR] = getchar(); 
		}
		else {
			memory[MR_KBSR] = 0;
		}
	}
	return memory[address]; 
}


int main(int argc, const char* argv[]) {
	// Load arguments
	if (argc < 2) {
		printf("lc3 [image-file1] ...\n");
		exit(2);
	}

	for (int j = 1; j < argc; j++) {
		if (!read_image(argv[j])) {
			printf("failed to load image: %s\n", argv[j]);
			exit(1); 
		}
	}

	// Setup - this is a small detail to properly handle input to the terminal
	signal(SIGINT, handle_interrupt);
	disable_input_buffering();

	reg[R_COND] = FL_ZRO;
	enum {PC_START = 0X3000};
	reg[R_PC] = PC_START;

	int running = 1;
	while (running) {
		// Fetch
		uint16_t instr = mem_read(reg[R_PC]++);
		uint16_t op = instr >> 12;

		switch (op) {
		case OP_ADD:
			break;
		case OP_AND:
			break;
		case OP_NOT:
			break;
		case OP_BR:
			break;
		case OP_JMP:
			break;
		case OP_JSR:
			break;
		case OP_LD:
			break;
		case OP_LDI:
			break;
		case OP_LDR:
			break;
		case OP_LEA:
			break;
		case OP_ST:
			break;
		case OP_STI:
			break;
		case OP_STR:
			break;
		case OP_TRAP:
			break;
		case OP_RES:
		case OP_RTI:
		default:
			// Opcode did not exist
			break;
		}
	}

	// Small detail - reset terminal settings at end of program
	restore_input_buffering();

	return 0;
}