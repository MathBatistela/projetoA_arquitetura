#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "debug.h"

#define _GNU_SOURCE // necessário porque getline() é extensão GNU

// Campos do formato de instrução.
// opcode  := ir(31 downto 26)
// rs      := ir(25 downto 21);
// rt      := ir(20 downto 16);
// rd      := ir(15 downto 11);
// shamt   := ir(10 downto  6);
// imm     := ir(15 downto  0);
// address := ir(25 downto  0);

/* Definição das Máscaras. */
int mascaraOpCode		= 0xFC000000;
int mascaraRs			= 0x03E00000;
int mascaraRt			= 0x001F0000;
int mascaraRd			= 0x0000F800;
int mascaraShamt		= 0x000007C0;
int mascaraFunct		= 0x0000003F;
int mascaraImmediate	= 0x0000FFFF;
int mascaraAddress		= 0x03FFFFFF;

char *registerName[32] = {
	"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2",
	"$a3",   "$t0", "$t1", "$t2", "$t3", "$t4", "$t5",
	"$t6",   "$t7", "$s0", "$s1", "$s2", "$s3", "$s4",
	"$s5",   "$s6", "$s7", "$t8", "$t9", "$k0", "$k1",
	"$gp",   "$sp", "$fp", "$ra"
};

/* Função para imprimir um número 32 bits com binário.
   Adaptado de um código encontrado na internet.    */
void bin_prnt_byte(int x) {
	PRINT_FUNC_NAME;
	int i, n;
	TRACE("bin_prnt_byte begin x: %u\n");

	char buffer [50] = {0};
  
	for (i = 0; i < 32; i++) {
		if ((x & 0x80000000) != 0) {
		// fprintf(stdout, "1");
			n = sprintf (buffer, "%s%s", buffer, "1");
		}
		else {
			// fprintf(stdout, "0");
			n = sprintf (buffer, "%s%s", buffer, "0");
		}
		if ((i % 4 == 3) && i < 31) {
			// fprintf(stdout, " "); /* insert a space between nybbles */
			n = sprintf (buffer, "%s%s", buffer, " ");
		}
		x = x << 1;
	}
	TRACE("[%s]\n", buffer);
	TRACE("bin_prnt_byte end.\n");
}

/* Função que recupera o campo OpCode. */
unsigned int getOpCode(unsigned int ir) {
	PRINT_FUNC_NAME;
	unsigned int opcode = ((ir & mascaraOpCode) >> 26);
	TRACE("Opcode: %u\n ", opcode);
	return opcode;
}

/* Função que recupera o campo registrador Rs. */
unsigned int getRs(unsigned int ir) {
	PRINT_FUNC_NAME;
	unsigned int rs = (ir & mascaraRs) >> 21;
	TRACE("Rs: %u\n ", rs);
	return rs;
}

/* Função que recupera o campo registrador Rt. */
unsigned int getRt(unsigned int ir) {
	PRINT_FUNC_NAME;
	unsigned int rt = (ir & mascaraRt) >> 16;
	TRACE("Rt: %u\n ", rt);
	return rt;
}

/* Função que recupera o campo registrador Rd. */
unsigned int getRd(unsigned int ir) {
	PRINT_FUNC_NAME;
	unsigned int rd = (ir & mascaraRd) >> 11;
	TRACE("Rd: %u\n ", rd);
	return rd;
}

/* Função que recupera o campo Shamt (deslocamento). */
int getShamt(unsigned int ir) {
	PRINT_FUNC_NAME;
	unsigned int shamt = (ir & mascaraShamt) >> 6;
	TRACE("Shamt: %u\n ", shamt);
	return shamt;
}

/* Função que recupera o campo Funct. */
unsigned int getFunct(unsigned int ir) {
	PRINT_FUNC_NAME;
	unsigned int funct = (ir & mascaraFunct);
	TRACE("funct: %u\n ", funct);
	return funct;
}

/* Função que recupera o campo imm. */
int getImmediate(unsigned int ir) {
	PRINT_FUNC_NAME;
	unsigned int imm = (ir & mascaraImmediate);
	TRACE("imm: %u\n ", imm);
	return imm;
}

/* Função que recupera o campo Address. */
unsigned int getAddress(unsigned int ir) {
	PRINT_FUNC_NAME;
	unsigned int address = (ir & mascaraAddress);
	TRACE("address: %u\n ", address);
	return address; 
}

/* Converte um char * representando um binário, para inteiro. */
int intFromBinary(char *s) {
	PRINT_FUNC_NAME;
	int inteiro = (int) strtol(s, 0, 2);
	TRACE("valor inteiro %d - %X - %b\n", inteiro, inteiro, inteiro);
	bin_prnt_byte(inteiro);
	TRACE("\n");
	return inteiro;
}

/* Decodificação das instruções. */
void decodificar(unsigned int ir) {
	PRINT_FUNC_NAME;
	// Recupera o OpCode.
	switch (getOpCode(ir)) {
		case 0: { // 000000, Aritmética.
			// Recupera o Funct.
			switch (getFunct(ir)) {
				case 0:{ // NOP
					if(getRd(ir) == 0 && getRs(ir) == 0 && getRt(ir) == 0){
						fprintf(stdout, "nop\n");
					}
					else{ 	// SLL -- Shift left logical
							// Syntax: sll $d, $t, h
							// Encoding: 0000 00ss ssst tttt dddd dhhh hh00 0000
						fprintf(stdout, "sll ");
						fprintf(stdout, "%s, ", registerName[getRd(ir)]);
						fprintf(stdout, "%s, ", registerName[getRt(ir)]);
						fprintf(stdout, "%d\n", getShamt(ir));
					}
					break;
				}
				case 2:	{	// SRL -- Shift right logical
							// Syntax: srl $d, $t, h
							// Encoding: 0000 00-- ---t tttt dddd dhhh hh00 0010
					fprintf(stdout, "srl ");
					fprintf(stdout, "%s, ", registerName[getRd(ir)]);
					fprintf(stdout, "%s, ", registerName[getRt(ir)]);
					fprintf(stdout, "%d\n", getShamt(ir));
					break;
				}
				case 8: { 	// JR -- Jump register, Description: Jump to the address contained in register $s 
							// Syntax: jr $s
							// 0000 00ss sss0 0000 0000 0000 0000 1000
					fprintf(stdout, "jr ");
					fprintf(stdout, "%s\n", registerName[getRs(ir)]);
					break;
				}
				case 32: { // 100000 -> add, R-Type.
					fprintf(stdout, "add ");
					fprintf(stdout, "%s, ", registerName[getRd(ir)]);
					fprintf(stdout, "%s, ", registerName[getRs(ir)]);
					fprintf(stdout, "%s\n", registerName[getRt(ir)]);
					break;
				}
				case 33: { 	// ADDU -- Add unsigned, Description: Adds two registers and stores the result in a register 
							// Syntax: addu $d, $s, $t
							// 0000 00ss ssst tttt dddd d000 0010 0001
					fprintf(stdout, "addu ");
					fprintf(stdout, "%s, ", registerName[getRd(ir)]);
					fprintf(stdout, "%s, ", registerName[getRs(ir)]);
					fprintf(stdout, "%s\n", registerName[getRt(ir)]);
					break;
				}

				case 34: { // 100010 -> sub, R-Type.
					fprintf(stdout, "sub ");
					fprintf(stdout, "%s, ", registerName[getRd(ir)]);
					fprintf(stdout, "%s, ", registerName[getRs(ir)]);
					fprintf(stdout, "%s\n", registerName[getRt(ir)]);
					break;
				}
				case 35: { 	// SUBU -- Subtract unsigned, Description: Subtracts two registers and stores the result in a register
							// Syntax: subu $d, $s, $t
							// 0000 00ss ssst tttt dddd d000 0010 001
					fprintf(stdout, "subu ");
					fprintf(stdout, "%s, ", registerName[getRd(ir)]);
					fprintf(stdout, "%s, ", registerName[getRs(ir)]);
					fprintf(stdout, "%s\n", registerName[getRt(ir)]);
					break;
				}
				case 36: { // 100100 -> and, R-Type.
					fprintf(stdout, "and ");
					fprintf(stdout, "%s, ", registerName[getRd(ir)]);
					fprintf(stdout, "%s, ", registerName[getRs(ir)]);
					fprintf(stdout, "%s\n", registerName[getRt(ir)]);
					break;
				}
				case 37: { 	// OR -- Bitwise or, Description: Bitwise logical ors two registers and stores the result in a register
							// Syntax: or $d, $s, $t
							// 0000 00ss ssst tttt dddd d000 0010 0101
					fprintf(stdout, "or ");
					fprintf(stdout, "%s, ", registerName[getRd(ir)]);
					fprintf(stdout, "%s, ", registerName[getRs(ir)]);
					fprintf(stdout, "%s\n", registerName[getRt(ir)]);
					break;
				}
				case 39: { // 100111 -> nor, R-Type.
					fprintf(stdout, "nor ");
					fprintf(stdout, "%s, ", registerName[getRd(ir)]);
					fprintf(stdout, "%s, ", registerName[getRs(ir)]);
					fprintf(stdout, "%s\n", registerName[getRt(ir)]);
					break;
				}
				case 42: { // 101010 -> slt, R-Type.
					fprintf(stdout, "slt ");
					fprintf(stdout, "%s, ", registerName[getRd(ir)]);
					fprintf(stdout, "%s, ", registerName[getRs(ir)]);
					fprintf(stdout, "%s\n", registerName[getRt(ir)]);
					break;
				}
				case 43: { 	// SLTU -- Set on less than unsigned, Description: If $s is less than $t, $d is set to one. It gets zero otherwise.
							// Syntax: sltu $d, $s, $t
							// 0000 00ss ssst tttt dddd d000 0010 1011
					fprintf(stdout, "sltu ");
					fprintf(stdout, "%s, ", registerName[getRd(ir)]);
					fprintf(stdout, "%s, ", registerName[getRs(ir)]);
					fprintf(stdout, "%s\n", registerName[getRt(ir)]);
					break;
				}
			}
			break;
		}
		
		case 2:	{ // 000010 -> j address (26), J-Type
			fprintf(stdout, "j ");
			fprintf(stdout, "0x%0.8X\n", getAddress(ir));
			break;
		}

		case 3:	{ // 000011 -> jal , J-Type
			fprintf(stdout, "jal ");
			fprintf(stdout, "0x%0.8X\n", getAddress(ir));
			break;
		}

		case 4:	{ // BEQ -- Branch on equal, Description: Branches if the two registers are equal. 
				  // Syntax: beq $s, $t, offset
				  // 0001 00ss ssst tttt iiii iiii iiii iiii
			fprintf(stdout, "beq ");
			fprintf(stdout, "%s, ", registerName[getRs(ir)]);
			fprintf(stdout, "%s, ", registerName[getRt(ir)]);
			fprintf(stdout, "%#0.4x\n", getImmediate(ir));
			break;
		}

		case 5:	{ // BNE -- Branch on not equal, Description: Branches if the two registers are not equal. 
				  // Syntax: bne $s, $t, offset
				  // 0001 01ss ssst tttt iiii iiii iiii iiii
			fprintf(stdout, "bne ");
			fprintf(stdout, "%s, ", registerName[getRs(ir)]);
			fprintf(stdout, "%s, ", registerName[getRt(ir)]);
			fprintf(stdout, "%#0.4x\n", getImmediate(ir));
			break;
		}

		case 8:	{ // ADDI -- Add Immediate, Description: Adds a register and a sign-extended immediate value and stores the result in a register.
				  // Syntax: addi $t, $s, imm
				  // 0010 00ss ssst tttt iiii iiii iiii iiii
			fprintf(stdout, "addi ");
			fprintf(stdout, "%s, ", registerName[getRs(ir)]);
			fprintf(stdout, "%s, ", registerName[getRt(ir)]);
			fprintf(stdout, "%#0.4x\n", getImmediate(ir));
			break;
		}

		case 9:	{ // ADDIU -- Add immediate unsigned, Description: Adds a register and a sign-extended immediate value and stores the result in a register.
				  // Syntax: addiu $t, $s, imm
				  // 0010 01ss ssst tttt iiii iiii iiii iiii
			fprintf(stdout, "addiu ");
			fprintf(stdout, "%s, ", registerName[getRs(ir)]);
			fprintf(stdout, "%s, ", registerName[getRt(ir)]);
			fprintf(stdout, "%#0.4x\n", getImmediate(ir));
			break;
		}

		case 10:{ // SLTI -- Set on less than immediate, Description: If $s is less than immediate, $t is set to one. It gets zero otherwise.
				  // Syntax: slti $t, $s, imm
				  // 0010 10ss ssst tttt iiii iiii iiii iiii
			fprintf(stdout, "slti ");
			fprintf(stdout, "%s, ", registerName[getRs(ir)]);
			fprintf(stdout, "%s, ", registerName[getRt(ir)]);
			fprintf(stdout, "%#0.4x\n", getImmediate(ir));
			break;
		}
		
		case 11:{ // SLTIU -- Set on less than immediate unsigned, Description: If $s is less than the unsigned immediate, $t is set to one. It gets zero otherwise.
				  // Syntax: sltiu $t, $s, imm
				  // 0010 11ss ssst tttt iiii iiii iiii iiii
			fprintf(stdout, "sltiu ");
			fprintf(stdout, "%s, ", registerName[getRs(ir)]);
			fprintf(stdout, "%s, ", registerName[getRt(ir)]);
			fprintf(stdout, "%#0.4x\n", getImmediate(ir));
			break;
		}

		case 12:{ // ANDI -- Bitwise and immediate, Description: Bitwise ands a register and an immediate value and stores the result in a register.
				  // Syntax: andi $t, $s, imm
				  // 0011 00ss ssst tttt iiii iiii iiii iiii
			fprintf(stdout, "andi ");
			fprintf(stdout, "%s, ", registerName[getRs(ir)]);
			fprintf(stdout, "%s, ", registerName[getRt(ir)]);
			fprintf(stdout, "%#0.4x\n", getImmediate(ir));
			break;
		}

		case 13:{ // ORI -- Bitwise or immediate, Description: Bitwise ors a register and an immediate value and stores the result in a register.
				  // Syntax: ori $t, $s, imm
				  // 0011 01ss ssst tttt iiii iiii iiii iiii
			fprintf(stdout, "ori ");
			fprintf(stdout, "%s, ", registerName[getRs(ir)]);
			fprintf(stdout, "%s, ", registerName[getRt(ir)]);
			fprintf(stdout, "%#0.4x\n", getImmediate(ir));
			break;
		}

		case 15:{ 	// LUI -- Load upper immediate, Description: The immediate value is shifted left 16 bits and stored in the register. The lower 16 bits are zeroes.
					// Syntax: lui $t, imm
					// 0011 11-- ---t tttt iiii iiii iiii iiii
			fprintf(stdout, "lui ");
			fprintf(stdout, "%s, ", registerName[getRt(ir)]);
			fprintf(stdout, "%#0.4x\n", getImmediate(ir));
			break;
		}		
		
		case 35: { // 100011 (lw), I-Type. lw rt, imm(rs)
			fprintf(stdout, "lw ");
			fprintf(stdout, "%s, ", registerName[getRt(ir)]);
			fprintf(stdout, "%d", getImmediate(ir));
			fprintf(stdout, "(%s)\n", registerName[getRs(ir)]);
			break;
		}

		case 36: {	// 100100 (lbu), I-Type. lbu rt, imm(rs)
			fprintf(stdout, "lbu ");
			fprintf(stdout, "%s, ", registerName[getRt(ir)]);
			fprintf(stdout, "%d", getImmediate(ir));
			fprintf(stdout, "(%s)\n", registerName[getRs(ir)]);
			break;
		}
		
		case 37: { // 100101 (lhu), I-Type. lhu rt, imm(rs)
			fprintf(stdout, "lhu ");
			fprintf(stdout, "%s, ", registerName[getRt(ir)]);
			fprintf(stdout, "%d", getImmediate(ir));
			fprintf(stdout, "(%s)\n", registerName[getRs(ir)]);
			break;
		}

		case 40: {	// SB -- Store byte, Description: The least significant byte of $t is stored at the specified address.
				   // Syntax: sb $t, offset($s)
				   // 1010 00ss ssst tttt iiii iiii iiii iiii
			fprintf(stdout, "sb ");
			fprintf(stdout, "%s, ", registerName[getRt(ir)]);
			fprintf(stdout, "%d", getImmediate(ir));
			fprintf(stdout, "(%s)\n", registerName[getRs(ir)]);
			break;
		}

		case 43: { // 101011 (sw), I-Type. sw rt, imm(rs)
			fprintf(stdout, "sw ");
			fprintf(stdout, "%s, ", registerName[getRt(ir)]);
			fprintf(stdout, "%d", getImmediate(ir));
			fprintf(stdout, "(%s)\n", registerName[getRs(ir)]);
			break;
		}

		case 41: {	// 101001 (sh), I-Type. sh rt, imm(rs)
			fprintf(stdout, "sh ");
			fprintf(stdout, "%s, ", registerName[getRt(ir)]);
			fprintf(stdout, "%d", getImmediate(ir));
			fprintf(stdout, "(%s)\n", registerName[getRs(ir)]);
			break;
		}
		
		case 48: { // 110000 (ll), I-Type. ll rt, imm(rs)
			fprintf(stdout, "ll ");
			fprintf(stdout, "%s, ", registerName[getRt(ir)]);
			fprintf(stdout, "%d", getImmediate(ir));
			fprintf(stdout, "(%s)\n", registerName[getRs(ir)]);
			break;
		}

		case 56:{ //  111000 (sc), I-Type. sc rt, imm(rs)
			fprintf(stdout, "sc ");
			fprintf(stdout, "%s, ", registerName[getRs(ir)]);
			fprintf(stdout, "%s, ", registerName[getRt(ir)]);
			fprintf(stdout, "%#0.4x\n", getImmediate(ir));
			break;
		}

		default:  { // outros casos.
			fprintf(stdout, "Instrução não implementada. OpCode: %d.\n", getOpCode(ir));
		}

	}
}

int main(int argc, char *argv[]) {
	PRINT_FUNC_NAME;
	int i;
	unsigned int ir;
	if (argc < 2) {
		fprintf(stderr, "Uso:\n ./decodificador arquivo.b\n");
		return (0);
	}
	TRACE("Argumentos:\n");
	TRACE("argc = %d\n", argc);
	for (i = 0; i < argc; i++){
		TRACE("argv[%d] = %s\n", i, argv[i]);
	}
	FILE *arquivo = fopen(argv[1], "r");
	size_t len = 32; // tamanho da linha.
	char *linha = malloc(len);

	// Se não conseguiu criar o arquivo.
	if (!arquivo) {
		perror(argv[1]);
		exit(1);
	}

	// Recupera cada linha do arquivo e decodifica.
	while (getline(&linha, &len, arquivo) > 0) {
		TRACE("Linha: %s\n", linha);
		// teste: bin_prnt_byte(0x8C130004);
		ir = intFromBinary(linha);
		TRACE("IR: %u\n", ir);
		decodificar(ir);
	}

	// Desaloca as variáveis utilizadas.
	if (linha) {
		free(linha);
	}

	fclose(arquivo);

	return 0;
}
