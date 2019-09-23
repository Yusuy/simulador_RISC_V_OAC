//***************************************************************************************************
//									UnB - Universidade de Brasília							
//			CiC - Organização e Arquitetura de Computadores		Prof. R. P. Jacobi		Turma: C	
//			Victor Gabriel Rodrigues de Almeida			Matrícula: 14/0052399
//***************************************************************************************************

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>

//Declaração de memória
#define memory_size 4096
int32_t memory[memory_size];

/**************************************************************************************************************/
//Banco de Registradores
uint32_t ri = 0x00000000;

int32_t register_bank[32];

#define zero 0
#define ra 1 
#define sp 2 
#define gp 3 
#define tp 4
#define t0 5 
#define t1 6 
#define t2 7
#define s0 8 
#define s1 9 
#define a0 10 
#define a1 11 
#define a2 12 
#define a3 13 
#define a4 14 
#define a5 15 
#define a6 16 
#define a7 17
#define s2 18 
#define s3 19 
#define s4 20 
#define s5 21 
#define s6 22 
#define s7 23 
#define s8 24 
#define s9 25
#define s10 26 
#define s11 27
#define t3 28 
#define t4 29 
#define t5 30 
#define t6 31

uint32_t pc = 0x00000000;

/**************************************************************************************************************/
/**												Variáveis decode										     **/
uint32_t opcode; 			//código da operação
uint32_t rs1; 				//índice do primeiro registrador fonte
uint32_t rs2; 				//índice do segundo registrador fonte
uint32_t rd;				//índice do registrador destino, que recebe o resultado da operação
uint32_t shamt;				//quantidade de deslocamento em instruções shift e rotate
uint32_t funct3;			//código auxiliar de 3 bits para determinar a instrução a ser executada
uint32_t funct7;			//código auxiliar de 7 bits para determinar a instrução a ser executada
int32_t imm12_i;			//constante de 12 bits, valor imediato em instruções tipo I
int32_t imm12_s;			//constante de 12 bits, valor imediato em instruções tipo S
int32_t imm13;				//constante de 13 bits, valor imediato em instruções tipo SB, bit 0 é sempre 0
int32_t imm20_u;			//constante de 20 bits mais significativos, 31 a 12
int32_t imm21;				//constante de 21 bits para saltos relativos, bit 0 é sempre 0

/**************************************************************************************************************/
//Decodificação de Instrução
enum OPCODE {	//OPCODES das funções
	LUI = 0x37,	AUIPC = 0X17,	//Atribuição de 20 bits mais significativos
	ILType = 0x03,				//Load Type
	BType = 0x63,				//Branch Conditional
	JAL = 0x6f,	JALR = 0x67,	//Jumps
	StoreType = 0x23,			//Store Type
	ILAType = 0x13,				//Lógico Aritimético com Imediato
	RegType = 0x33,				//Lógico Aritimético com Registradores
	ECALL = 0x73 				// Chamada do sistema
} opcode;

enum FUNCT3 {	//Campo auxiliar de 3 bits
	BEQ3 = 0, BNE3 = 01, BLT3 = 04, BGE3 = 05, BLTU3 = 0x06, BGEU3 = 07,							//Branches
	LB3 = 0, LH3 = 01, LW3 = 02, LBU3 = 04, LHU3 = 05,												//Loads
	SB3 = 0, SH3 = 01, SW3 = 02,																	//Stores
	ADDSUB3 = 0, SLL3 = 01, SLT3 = 02, SLTU3 = 03, XOR3 = 04, SR3 = 05, OR3 = 06, AND3 = 07,		//Lógico Aritmético com Registradores
	ADDI3 = 0, ORI3 = 06, SLTI3 = 02, XORI3 = 04, ANDI3 = 07, SLTIU3 = 03, SLLI3 = 01, SRI3 = 05 	//Lógico Aritmético com Imediatos
} funct3;

enum FUNCT7 {	//Campo auxiliar de 7 bits para instruções SRLI/SRAI, ADD/SUB, SRL,SRA
	ADD7 = 0, SUB7 = 0x20, SRA7 = 0x20, SRL7 = 0, SRLI7 = 0x00, SRAI7 = 0x20 
} funct7;

/**************************************************************************************************************/
//Função para imprimir conteúdo da memória
void dump_mem(uint32_t address, uint32_t word_size, char format){
	int i;
	switch (format){
		case 'h':
			if(word_size == address && address >= 0 && word_size >= 0){
				printf("\n**********************************************\nValor na Memoria 0x%08x em Hexadecimal\n**********************************************\n", address*4);
				for(i=address; i<=word_size; i++){
					printf("mem[0x%08x] = 0x%08x \n", i*4, memory[i]);
				}
			}
			
			else if(word_size > address && address >= 0 && word_size >= 0){
				printf("\n*******************************************************************\nValores na Memoria de 0x%08x a 0x%08x em Hexadecimal\n*******************************************************************\n", address*4, word_size*4);
				for(i=address; i<=word_size; i++){
					printf("mem[0x%08x] = 0x%08x \n", i*4, memory[i]);
				}
			}
			
			else if(word_size < address && address >= 0 && word_size >= 0){
				printf("\n*******************************************************************\nValores na Memoria de 0x%08x a 0x%08x em Hexadecimal\n*******************************************************************\n", word_size*4, address*4);
				for(i=word_size; i<=address; i++){
					printf("mem[0x%08x] = 0x%08x \n", i*4, memory[i]);
				}
			}
			
			else{printf("\nINDICE DE MEMORIA INVALIDO\n");}
			break;
		
		case 'd':
			if(word_size == address && address >= 0 && word_size >= 0){
				printf("\n**********************************************\nValor na Memoria 0x%08x em Decimal\n**********************************************\n", address*4);
				for(i=address; i<=word_size; i++){
					printf("mem[0x%08x] = %d \n", i*4, memory[i]);
				}
			}
			
			else if(word_size > address && address >= 0 && word_size >= 0){
				printf("\n*******************************************************************\nValores na Memoria de 0x%08x a 0x%08x em Decimal\n*******************************************************************\n", address*4, word_size*4);
				for(i=address; i<=word_size; i++){
					printf("mem[0x%08x] = %d \n", i*4, memory[i]);
				}
			}
			
			else if(word_size < address && address >= 0 && word_size >= 0){
				printf("\n*******************************************************************\nValores na Memoria de 0x%08x a 0x%08x em Decimal\n*******************************************************************\n", word_size*4, address*4);
				for(i=word_size; i<=address; i++){
					printf("mem[0x%08x] = %d \n", i*4, memory[i]);
				}
			}
			
			else{printf("\nINDICE DE MEMORIA INVALIDO\n");}
			break;
			
		default:
			break;
	}
	printf("*******************************************************************\n\n");	
}

//Função para imprimir conteúdo dos registradores
void dump_reg(char format){
	int i;
	switch (format){
		case 'h':
			printf("\n**************************************************\nValores dos Registradores em Hexadecimal\n**************************************************\n");
			for(i=0; i<32; i++){
				printf("registrador[%d] = 0x%08x \n", i, register_bank[i]);
			}
			break;
		
		case 'd':
			printf("\n**************************************************\nValores dos Registradores em Decimal\n**************************************************\n");
			for(i=0; i<32; i++){
				printf("registrador[%d] = %d \n", i, register_bank[i]);
			}
			break;
			
		default:
			break;
	}
	printf("*******************************************************************\n\n");	
}

/**************************************************************************************************************/
//												Funções Load
//Função que carrega word da memória
int32_t lw(uint32_t mem_address, int32_t constant){
	//printf("addr = %d, cte = %d -> data = %d\n", mem_address, constant, memory[(mem_address+constant)/4]);
	mem_address = (mem_address+constant)/4;
	return memory[mem_address];
}

//Função que carrega half word da memória
int32_t lh(uint32_t mem_address, int32_t constant){
	mem_address = (mem_address)/4;
	int16_t *h_word = (int16_t*)&memory[mem_address];
	h_word += constant/2;
	return *h_word;
}

//Função que carrega half word unsigned
int32_t lhu(uint32_t mem_address, int32_t constant){
	mem_address = (mem_address)/4;
	uint16_t *h_word = (uint16_t*)&memory[mem_address];
	h_word += constant/2;
	return *h_word;
}

//Função que carrega byte da memória
int32_t lb(uint32_t mem_address, int32_t constant){
	mem_address = (mem_address+constant)/4;
	int8_t *b_word = (int8_t*)&memory[mem_address];
	b_word += constant;
	return *b_word;
}

//Função que carrega byte unsigned da memória
int32_t lbu(uint32_t mem_address, int32_t constant){
	mem_address = (mem_address+constant)/4;
	uint8_t *b_word = (uint8_t*)&memory[mem_address];
	return *b_word;
}

/**************************************************************************************************************/
//												Funções Store
//Função que salva word na memória
void sw(uint32_t mem_address, int32_t constant, int32_t data){
	mem_address = (mem_address+constant)/4;
	memory[mem_address] = data;
	//printf("Salvou %d em %d\n", data, mem_address);
}

//Função que salva half word na memória
void sh(uint32_t mem_address, int32_t constant, int16_t data){
	mem_address = (mem_address/4);
	int16_t *h_mem = (int16_t*)&memory[mem_address];
	h_mem += constant/2;
	*h_mem = data;
}

//Função que salva words na memória
void sb(uint32_t mem_address, int32_t constant, int8_t data){
	mem_address = (mem_address/4);
	int8_t *b_mem = (int8_t*)&memory[mem_address];
	b_mem += constant;
	*b_mem = data;
}

//Função que salva words na memória .data (0x00002000) e .text (0x00000000)
void fetch(){
	FILE *instructions;
	int i = memory_size/2;

	instructions = fopen("text", "rb");
	if(instructions == NULL){
		printf("\nNenhuma instrucao encontrada.\n");
	}

	else{
		fread(&memory[0], sizeof(memory), 1, instructions);
	}

	fclose(instructions);
	
	instructions = fopen("data", "rb");
	if(instructions == NULL){
		printf("\nNenhum dado encontrado.\n");
	}

	else{
		fread(&memory[i], sizeof(memory), 1, instructions);	
	}

	fclose(instructions);
}

//Função para decodificação
void decode(){
	int i = pc/4;
	uint32_t word = memory[i];
	int32_t s_word = memory[i];
	
	opcode = word << 25;
	opcode = opcode >> 25;
	
	rd = word << 20;
	rd = rd >> 27;
	
	rs1 = word << 12;
	rs1 = rs1 >> 27;
	
	rs2 = word << 7;
	rs2 = rs2 >> 27;
	
	shamt = word << 7;
	shamt = shamt >> 27;
	
	funct3 = word << 17;
	funct3 = funct3 >> 29;
	
	funct7 = word >> 25;
	
	imm12_i = s_word >> 20;
	
	int32_t lower;
	lower = s_word << 20;
	lower = lower >> 27;
	imm12_s = s_word >> 25;
	imm12_s = imm12_s << 5;
	imm12_s = imm12_s + lower;
	
	uint32_t b10, b11, b4;
	imm13 = s_word >> 31;
	imm13 = imm13 << 12;
	b11 = s_word << 24;
	b11 = b11 >> 31;
	b11 = b11 << 11;
	b4 = s_word << 20;
	b4 = b4 >> 28;
	b4 = b4 << 1;
	b10 = s_word << 1;
	b10 = b10 >> 26;
	b10 = b10 << 5;
	imm13 =(int32_t)imm13 + b11 + b10 + b4;
	
	imm20_u = s_word >> 12;
	imm20_u = imm20_u << 12;
	
	uint32_t b19;
	b10 = s_word << 1;
	b10 = b10 >> 22;
	b10 = b10 << 1;
	b11 = s_word << 11;
	b11 = b11 >> 31;
	b11 = b11 << 11;
	b19 = s_word << 12;
	b19 = b19 >> 24;
	b19 = b19 << 12;
	imm21 = s_word >> 31;
	imm21 = imm21 << 20;
	imm21 = (int32_t)imm21 + b19 + b11 + b10;
}

//Função Execute
void execute(){
	uint32_t t;
	int32_t ts;
	uint32_t bit5;
	int contador = 0;
	int c1, c2, c3, c4;
	
	switch (opcode){
		case LUI:
			register_bank[rd] = imm20_u;
			break;
		
		case AUIPC:
			register_bank[rd] = pc + imm20_u;	
			break;
		
		case ILType:
			switch(funct3){
				case LB3:
				register_bank[rd] = lb(register_bank[rs1], imm12_i);
				break;

				case LH3:
				register_bank[rd] = lh(register_bank[rs1], imm12_i);
				break;

				case LW3:
				register_bank[rd] = lw(register_bank[rs1], imm12_i);
				break;

				case LBU3:
				register_bank[rd] = lbu(register_bank[rs1], imm12_i);
				break;

				case LHU3:
				register_bank[rd] = lhu(register_bank[rs1], imm12_i);
				break;

				default:
				break;
			}
			break;
		
		case BType:
			switch(funct3){
				case BEQ3:
				if(register_bank[rs1] == register_bank[rs2]){
					pc += imm13 - 4;
				}
				break;

				case BNE3:
				if(register_bank[rs1] != register_bank[rs2]){
					pc += imm13 - 4;
				}
				break;

				case BLT3:
				if(register_bank[rs1] < register_bank[rs2]){
					pc += imm13 - 4;
				}
				break;

				case BGE3:
				if(register_bank[rs1] > register_bank[rs2]){
					pc += imm13 - 4;
				}
				break;

				case BLTU3:
				if(register_bank[rs1] <= register_bank[rs2]){
					pc += imm13 - 4;
				}
				break;

				case BGEU3:
				if(register_bank[rs1] >= register_bank[rs2]){
					pc += imm13 - 4;
				}
				break;

				default:
				break;
			}
			break;
		
		case JAL:
			register_bank[rd] = pc + 4;
			pc = pc + imm21;
			pc = pc - 4;
			//printf("Cheguei no JAL com imm21 = 0x%08x  - PC = 0x%08x>\n", imm21, pc);
			break;
		
		case JALR:
			t = pc+4;
			pc = (register_bank[rs1]+imm12_i)&~1;
			pc = pc-4;
			register_bank[rd] = t;
			break;
		
		case StoreType:
			switch(funct3){
				case SW3:
				sw(register_bank[rs1], imm12_s, register_bank[rs2]);
				break;

				case SH3:
				sh(register_bank[rs1], imm12_s, register_bank[rs2]);
				break;

				case SB3:
				sb(register_bank[rs1], imm12_s, register_bank[rs2]);
				break;

				default:
				break;
			}
			break;
		
		case ILAType:
			switch(funct3){
				case ADDI3:
				register_bank[rd] = register_bank[rs1] + imm12_i;
				break;
				
				case ORI3:
				register_bank[rd] = register_bank[rs1] | imm12_i;
				break;

				case SLTI3:
				register_bank[rd] = ((int32_t)register_bank[rs1]) < ((int32_t)imm12_i);
				break;
				
				case XORI3:
				register_bank[rd] = register_bank[rs1] ^ imm12_i;
				break;
				
				case ANDI3:
				register_bank[rd] = register_bank[rs1] & imm12_i;
				break;
				
				case SLTIU3:
				register_bank[rd] = ((uint32_t)register_bank[rs1]) < ((uint32_t)imm12_i);
				break;
				
				case SLLI3:
				register_bank[rd] = register_bank[rs1] << shamt;
				break;
				
				case SRI3:
					switch(funct7){
						case SRLI7:
						bit5 = shamt >> 4;
						if(bit5 == 0){
							register_bank[rd] = ((uint32_t)register_bank[rs1]) >> shamt;
						}
						break;

						case SRAI7:
						bit5 = shamt >> 4;
						if(bit5 == 0){
							register_bank[rd] = ((int32_t)register_bank[rs1]) >> shamt;
						}	
						break;

						default:
						break;
					}
				break;
				
				default:
				break;
			}
			break;
		
		case RegType:
			switch(funct3){
				case ADDSUB3:
					switch(funct7){
						case ADD7:
						register_bank[rd] = register_bank[rs1] + register_bank[rs2]; 
						break;

						case SUB7:
						register_bank[rd] = register_bank[rs1] - register_bank[rs2]; 
						break;

						default:
						break;
					}
				break;

				case AND3:
				register_bank[rd] = register_bank[rs1] & register_bank[rs2]; 
				break;

				case OR3:
				register_bank[rd] = register_bank[rs1] | register_bank[rs2]; 
				break;

				case SLL3:
				register_bank[rd] = register_bank[rs1] << register_bank[rs2]; 
				break;

				case SLT3:
				register_bank[rd] = ((int32_t)register_bank[rs1]) < ((int32_t)register_bank[rs2]); 
				break;

				case SLTU3:
				register_bank[rd] = ((uint32_t)register_bank[rs1]) < ((uint32_t)register_bank[rs2]);
				break;

				case SR3:
					switch(funct7){
						case SRA7:
						register_bank[rd] = ((int32_t)register_bank[rs1]) >> register_bank[rs2];
						break;

						case SRL7:
						register_bank[rd] = ((uint32_t)register_bank[rs1]) >> register_bank[rs2];
						break;

						default:
						break;
					}
				break;

				case XOR3:
				register_bank[rd] = register_bank[rs1] ^ register_bank[rs2];
				break;

				default:
				break;
			}
			break;
		
		case ECALL:
			switch(register_bank[a7]){
				case 10:
				printf("\nPrograma Finalizado\n");
				exit(0);

				case 1:
				printf("%d", register_bank[a0]);
				break;

				case 4:
				contador = 0;
						
				
				
				while(1){
					t = register_bank[a0]/4;
					ts = memory[t + contador];
					c1 = ts << 24;
					c1 = c1 >> 24;
					c2 = ts << 16;
					c2 = c2 >> 24;
					c3 = ts << 8;
					c3 = c3 >> 24;
					c4 = ts >> 24;
					
					
						if(c1 == '\0'){break;}
						printf("%c", c1);
					
						if(c2 == '\0'){break;}
						printf("%c", c2);
					
						if(c3 == '\0'){break;}
						printf("%c", c3);
				
						c4 = ts >> 24;
						if(c4 == '\0'){break;}
						printf("%c", c4);
				
					
					contador+=1;
				}
				break;

				default:
				break;
			}
			break;
		
		default:
			break;
	}
}

//Função que executa linha de código e atualiza o PC para próxima linha
void step(){
	register_bank[0] = 0;
	decode();
	execute();
	pc+=4;
}

//Função que executa o código até que encerrar ou acabar linhas de comando
void run(){
		while(1){
			step();
			if(memory[pc/4] == 0 || pc >= 0x00002000){
				break;
			}
		}
}	

int main(){
	register_bank[sp] = 0x00003ffc;
	register_bank[gp] = 0x00001800;
	
	fetch();
	
	dump_mem(0, 15, 'h');
	dump_mem(2048, 2070, 'h');
	run();
	
	return 0;
}
