//
//  main.c
//  simulator
//
//  Created by Joan Marin on 10/16/17.
//  Copyright © 2017 Joan Marin-Romero. All rights reserved.
//

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define NUMREGS  8
#define NUMMEMORY 65536

typedef struct stateStruct {
	int pc;
	int mem[NUMMEMORY];
	int reg[NUMREGS];
	int numMemory;
} stateType;

void printState(stateType *statePtr){
	int i;
	printf("\n@@@\nstate:\n");
	printf("\tpc %d\n", statePtr->pc);
	printf("\tmemory:\n");
	
	for(i = 0; i < statePtr->numMemory; i++){
		printf("\t\tmem[%d]=%d\n", i, statePtr->mem[i]);
	}
	printf("\tregisters:\n");
	
	
	for(i = 0; i < NUMREGS; i++){
		printf("\t\treg[%d]=%d\n", i, statePtr->reg[i]);
	}
	
	printf("end state\n");
	
}

void print_stats(int n_instrs){
	printf("INSTRUCTIONS: %d\n", n_instrs); // total executed instructions
}

int convertNum(int num){
	// convert a 16-bit number into a 32-bit integer
	if (num & (1<<15) ) {
		num -= (1<<16);
	}
	return(num);
}

bool runLine(stateType *state, uint32_t line, bool run){
	int opcode = 0;
	int regA = 0;
	int regB = 0;
	int destR = 0;
	uint32_t temp = line;
	
	opcode = temp >> 22;
	temp = line;
	state->pc++;
	
	if(opcode == 0 || opcode == 1){ // and nand
		temp = temp << 10;
		regA = temp >> 29;
		
		temp = line;
		temp = temp << 13;
		regB = temp >> 29;
		
		temp = line;
		temp = temp << 29;
		destR = temp >> 29;
		
		if (opcode == 0) { // add
			state->reg[destR] = state->reg[regA] + state->reg[regB];
		}
		else{ // nand
			state->reg[destR] = ~(state->reg[regA] & state->reg[regB]);
		}
	}
	
	else if (opcode == 2 || opcode == 3 || opcode == 4){ // lw sw beq
		temp = temp << 10;
		regA = temp >> 29;
		
		temp = line;
		temp = temp << 13;
		regB = temp >> 29;
		
		temp = line;
		temp = temp << 16;
		destR = temp >> 16;
		destR = convertNum(destR);
		
		
		if (opcode == 2) { // lw
			state->reg[regA] = state->mem[state->reg[regB] + destR];
		}
		
		else if (opcode == 3){ // sw
			state->mem[state->reg[regB] + destR] = state->reg[regA];
		}
		
		else{ //beq
			if (state->reg[regA] == state->reg[regB]) {
				state->pc = state->pc+ destR;
				
			}
		}
	}
	
	else if (opcode == 5){ // Jalr
		temp = temp << 10;
		regA = temp >> 29;
		
		temp = line;
		temp = temp << 13;
		regB = temp >> 29;
		
		state->reg[regA] = state->pc;
		state->pc = state->reg[regB];
	}
	
	else if(opcode == 6){ // halt
		return false;
	}
	
	else if(opcode == 7){ // noop
		//noop
	}
	
	else{ // .fill ?
		
	}
	printf("Op: %d r1: %d r2: %d, r3: %d\n", opcode, regA, regB, destR);
	return true;
}

int main(int argc, const char * argv[]) {
	FILE *machineCode;
	uint32_t line = 0;
	bool run = true;
	int stat = 0;
	stateType state = {.pc = 0, .numMemory = 0};
	
	for (int i = 0; i < NUMMEMORY; i++) { // set regs to 0
		state.mem[i] = 0;
	}
	
	machineCode = fopen(argv[1], "r");
	
	int ch=0;
	int lines=0;
	
	while(!feof(machineCode)){ // count number of lines
		ch = fgetc(machineCode);
		if(ch == '\n'){
			lines++;
		}
	}
	
	fseek(machineCode, 0, SEEK_SET);
	
	for(int i = 0; i < lines; i++) { // store lines into memory
		fscanf(machineCode,"%u", &line);
		state.mem[i]= (int) line;
	}
	
	fclose(machineCode);
	
	while (run) { // simulator
		printState(&state);
		run = runLine(&state, state.mem[state.pc], run);
		stat++;
		//printState(&state);
	}
	print_stats(stat);
}
