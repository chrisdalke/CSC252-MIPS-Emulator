
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "RegFile.h"
#include "Syscall.h"
#include "utils/heap.h"
#include "elf_reader/elf_reader.h"
//#include "instructionConstants.h"
#include <stdint.h> // put in here for 64 bit number

//////////////////////////////////////////////////////////
// Instruction Constants
//////////////////////////////////////////////////////////

//Opcode Values

#define  OP_SPECIAL 0b00000000
#define  OP_REGIMM  0b00000001
#define  OP_ADDI    0b00001000
#define  OP_ADDIU   0b00001001
#define  OP_ANDI    0b00001100
#define  OP_XORI    0b00001110
#define  OP_ORI     0b00001101
#define  OP_SLTI    0b00001010
#define  OP_SLTIU   0b00001011
#define  OP_BEQ     0b00000100
#define  OP_BEQL    0b00010100
#define  OP_BGTZ    0b00000111
#define  OP_BLEZ    0b00000110
#define  OP_BLEZL   0b00010110
#define  OP_BNE     0b00000101
#define  OP_BNEL    0b00010101
#define  OP_J       0b00000010
#define  OP_JAL     0b00000011
#define  OP_LB      0b00100000
#define  OP_LBU     0b00100100
#define  OP_LH      0b00100001
#define  OP_LHU     0b00100101
#define  OP_LUI     0b00001111
#define  OP_LW      0b00100011
#define  OP_LWL     0b00100010
#define  OP_LWR     0b00100110
#define  OP_SB      0b00101000
#define  OP_SH      0b00101001
#define  OP_SW      0b00101011
#define  OP_SWL     0b00101010
#define  OP_SWR     0b00101110

//Func Values

#define FUNC_ADD    0b00100000
#define FUNC_ADDU   0b00000001
#define FUNC_SUB    0b00000010
#define FUNC_SUBU   0b00000011
#define FUNC_DIV    0b00011010
#define FUNC_DIVU   0b00011011
#define FUNC_MULT   0b00011000
#define FUNC_MULTU  0b00011000
#define FUNC_MFHI   0b00010000
#define FUNC_MFLO   0b00010010
#define FUNC_MTHI   0b00010001
#define FUNC_MTLO   0b00010011
#define FUNC_AND    0b00100100
#define FUNC_XOR    0b00100110
#define FUNC_NOR    0b00100110
#define FUNC_OR     0b00100101
#define FUNC_SLL    0b00000000
#define FUNC_SLLV   0b00000100
#define FUNC_SLT    0b00101010
#define FUNC_SLTU   0b00101011
#define FUNC_SRA    0b00000011
#define FUNC_SRAV   0b00000111
#define FUNC_SRL    0b00000010
#define FUNC_SRLV   0b00000110
#define FUNC_JALR   0b00001001
#define FUNC_JR     0b00001000
#define FUNC_NOP    0b00000000

//////////////////////////////////////////////////////////
// End of File
//////////////////////////////////////////////////////////

//Stats

uint32_t DynInstCount = 0;

void write_initialization_vector(uint32_t sp, uint32_t gp, uint32_t start) {
    printf("\n ----- BOOT Sequence ----- \n");
    printf("Initializing sp=0x%08x; gp=0x%08x; start=0x%08x\n", sp, gp, start);
    RegFile[28] = gp;
    RegFile[29] = sp;
    RegFile[31] = start;
    printRegFile();
    
}
//THIS IS USED for some of the addi methods -> taken from get hub
int signExtension(int instr) {
    int value = (0x0000FFFF & instr);
    int mask = 0x00008000;
    if (mask & instr) {
        value += 0xFFFF0000;
    }
    return value;
}
//Function that converts a byte to binary
//REMOVE BEFORE SUBMITTING
const char *byte_to_binary(int x)
{
    static char b[9];
    b[0] = 0;
    
    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }
    
    return b;
}

int main(int argc, char * argv[]) {
    
    int MaxInst = 0;
    int status = 0;
    uint32_t i;
    uint32_t PC,newPC;
    uint32_t CurrentInstruction;
    
    if (argc < 2) {
        printf("Input argument missing \n");
        return -1;
    }
    sscanf (argv[2], "%d", &MaxInst);
    
    //Open file pointers & initialize Heap & Registers
    initHeap();
    initFDT();
    initRegFile(0);
    
    //Load required code portions into Emulator Memory
    status =  LoadOSMemory(argv[1]);
    if(status <0) { return status; }
    
    //set Global & Stack Pointers for the Emulator
    // & provide startAddress of Program in Memory to Processor
    write_initialization_vector(exec.GSP, exec.GP, exec.GPC_START);
    
    printf("\n ----- Execute Program ----- \n");
    printf("Max Instruction to run = %d \n",MaxInst);
    PC = exec.GPC_START;
    
    newPC = -1;
    
    for(i=0; i<MaxInst ; i++) {
        DynInstCount++;
        CurrentInstruction = readWord(PC,false);
        //printRegFile();
        
        //////////////////////////////////////////////////////////
        // Main Instruction Simulation
        //////////////////////////////////////////////////////////
        
        //Determine what the OPCode for the current instruction is
        //shift the current instruction 26 to the right to get the first 6 bits (OPCODE)
        unsigned char opcode = ((CurrentInstruction) >> 26) & (0b00111111);
        
        //Test: print the opcode for the current instruction.
        printf("Current opcode =  %s\n",byte_to_binary(opcode));
        
        //Preload some variables that will be used for many commands
        unsigned char RS, RT, RD, shamt, temp, HIGH, LOW, immediate;
        RS = ((CurrentInstruction) >> 21) & (0b00111111);
        RT = ((CurrentInstruction) >> 16) & (0b00111111);
        immediate = ((CurrentInstruction)) & (0b1111111111111111);
        
        
        //Handle branch delay slot
        if (newPC != -1){
            //We should jump ahead to the next instruction
            PC = newPC;
            newPC = -1;
        }
        
        switch(opcode) {
            case OP_ADDI :
                RegFile[RT] = RegFile[RS] + immediate;
                //not sure if its immediate or RegFile[immediate]
                // This is using immediate since the data is stored in the current instruction not the register file
                break;
            case OP_ADDIU:
                temp = RegFile[RS] + signExtension(immediate);
                RegFile[RT] = temp;
                break;
            case OP_ANDI:
                RegFile[RT] = RegFile[RS] + signExtension(immediate);
                break;
            case OP_XORI:
                RegFile[RT] = RegFile[RS] ^ immediate;       
                break;
            case OP_ORI:
                RegFile[RT] = RegFile[RS] | immediate;
                break;
            case OP_SLTI:
                
                if (RegFile[RS] < signExtension(immediate)) {
                    RegFile[RS] = 1;
                } else {
                    RegFile[RS] = 0;
                }
                
                break;
            case OP_SLTIU:
                
                //This implementation taken from the mips handbook
                //Check to make sure this actually works properly
                if ((0 || RegFile[RS]) < (0 || signExtension(immediate))) {
                    RegFile[RS] = 1;
                } else {
                    RegFile[RS] = 0;
                }
                
                break;
            case OP_BEQ:
                
                //TODO: Figure out how to implement branch command
                
                break;
            case OP_BEQL:
                break;
            case OP_BGTZ:
                break;
            case OP_BLEZ:
                break;
            case OP_BLEZL:
                break;
            case  OP_BNE:
                break;
            case OP_BNEL:
                break;
            case  OP_J:
                
                //Update newPC, our target program counter
                newPC = PC + immediate;
                
                break;
            case  OP_JAL:
                
                //Update newPC, our target program counter
                newPC = PC + immediate;
                
                //record the current address into RegFile[31]
                RegFile[31] = PC;
                
                break;
            case OP_LB:
                break;
            case OP_LBU:
                break;
            case OP_LH:
                break;
            case OP_LHU:
                break;
            case OP_LUI:
                break;
            case OP_LW:
                
                //load the word given by the address into the specified register
                RegFile[RT] = readWord(RS + immediate,false);
                
                break;
            case OP_LWL:
                
                //Loads the most-significant part of a word as a signed value from an unaligned memory address
                //TODO
                
                break;
            case OP_LWR:
                
                //Loads the least-significant part of a word as a signed value from an unaligned memory address
                //TODO
                
                break;
            case OP_SB:
                break;
            case OP_SH:
                break;
            case OP_SW:
                
                //Stores a word into the specified memory location
                writeWord(RS + immediate,RegFile[RT],false);
                
                break;
            case OP_SWL:
                break;
            case OP_SWR:
                break;
            case OP_SPECIAL:
                
                //SPECIAL CASE OPCODE = 0
                //TO CHNAGE NEED TO CHANGE THE DO BITWISE TO GET THE BITS TO COMPARE
                printf("SPECIAL CASE OPCODE = 0");
                unsigned char SPECIAL = ((opcode)) & (0b00111111);
                printf("FUNC = %s\n",byte_to_binary(SPECIAL));
                // getting the last bits to compare in second switch statment

                RD = ((CurrentInstruction) >> 11) & (0b00111111);
                shamt = ((CurrentInstruction) >> 6) & (0b00111111);
                
                switch(SPECIAL) {
                        
                    case FUNC_ADD:
                        //Todo: overflow behavior? SEE PROJECT DESCP(BB) -> WE DO NOT NEED TO DO THIS
                        RegFile[RD] = RegFile[RS] + RegFile[RT];
                        break;
                    case FUNC_ADDU:
                        RegFile[RD] = RegFile[RS] + RegFile[RT];
                        break;
                    case  FUNC_SUB:
                        //Todo: overflow behavior? SEE PROJECT DESCP(BB) -> WE DO NOT NEED TO DO THIS
                        RegFile[RD] = RegFile[RS] - RegFile[RT];
                        break;
                    case  FUNC_SUBU:
                        RegFile[RD] = RegFile[RS] - RegFile[RT];
                        break;
                    case FUNC_DIV: //if you div by 0, answer is UNPREDICTABLE!
                        int64_t lowNumber = RegFile[RS] / RegFile[RT];
                        LOW = lowNumber;

                        int64_t highNumber = RegFile[RS] % RegFile[RT];
                        HIGH = highNumber;
                        break;
                    case FUNC_DIVU:
                        int64_t lowNumber = RegFile[RS] / RegFile[RT];
                        LOW = lowNumber;

                        int64_t highNumber = RegFile[RS] % RegFile[RT];
                        HIGH = highNumber;
                        break;
                    case FUNC_MULT: //not sure if I did this correctly
                            //high = 0-31, of 64 bit number, 32-64 is low, see ->
                            int64_t finalNumber = RegFile[RS] * RegFile[RT];
                            HIGH = ((finalNumber >> 31) & (0b00111111));
                            LOW = ((finalNumber << 31) & (0b00111111));
                        break;
                    case FUNC_MULTU: // same as above
                      //high = 0-31, of 64 bit number, 32-64 is low, see ->
                            int64_t finalNumber = (RegFile[RS] * RegFile[RT]);
                            HIGH = ((finalNumber >> 31) & (0b00111111));
                            LOW = ((finalNumber << 31) & (0b00111111));
                        break;
                    case FUNC_MFHI:
                        RegFile[RD] = HIGH;
                        break;
                    case FUNC_MFLO:
                        Regfile[RD] = LOW;
                        break;
                    case  FUNC_SLT:
                        RegFile[RD] = (RegFile[RS] < RegFile[RT])
                        break;
                    case FUNC_MTHI: // a div, mult, or something with high low must be used
                                     // before this command
                        HIGH = RegFile[RS];
                        break;
                    case FUNC_MTLO:
                        LOW = RegFile[RS];
                        break;
                    case FUNC_AND:
                    RegFile[RD] = RegFile[RS] & RegFile[RT];  // and
                        break;
                    case FUNC_XOR:
                    RegFile[RD] = RegFile[RS] ^ RegFile[RT]//xor
                        break;
                    case FUNC_NOR:
                    RegFile[RD] = RegFile[RS] ~ RegFile[RT]//NOR
                        break;
                    case FUNC_OR:
                    RegFile[RD] = RegFile[RS] | RegFile[RT]//or
                        break;
                    case FUNC_SLL
                    RegFile[RD] = RegFile[RT] << RegFile[SA]//SLL
                        break;
                    case FUNC_SLLV
                    RegFile[RD] = RegFile[RT] << RegFile[RS]//sllv
                        break;
                    case FUNC_SLTU:
                        RegFile[RD] = (RegFile[RS] < RegFile[RT]);
                        break;
                    case FUNC_SRA:
                        RegFile[RD] = RegFile[RT] + RegFile[shamt];
                        break;
                    case FUNC_SRAV:
                        RegFile[RD] = RegFile[RT] >> RegFile[RS];
                        break;
                    case FUNC_SRL:
                        RegFile[RD] = RegFile[RT] >> RegFile[shamt];
                        break;
                    case FUNC_SRLV:
                        RegFile[RD] = RegFile[RT] >> RegFile[RS];
                        break;
                    case FUNC_JALR:
                        break;
                    case FUNC_JR:
                        break;
                    case FUNC_NOP:
                        //Do nothing
                        break;
                    default:
                        printf("THE DEFAULT CASE WAS EXECUTED IN SECOND SWITCH\n" );
                }
                
                break;
            case OP_REGIMM:
                break;
            default:
                printf("THE DEFAULT CASE WAS EXECUTED IN FIRST SWTICH\n" );
                break;
        }
        
        //After we are done, increase the program counter.
        PC = PC + 1;
        
        //////////////////////////////////////////////////////////
        // End of Main Instruction Simulation
        //////////////////////////////////////////////////////////
        
    } //end fori
    
    
    //Close file pointers & free allocated Memory
    closeFDT();
    CleanUp();
    return 1;
}


