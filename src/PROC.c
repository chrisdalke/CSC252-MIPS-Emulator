
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "RegFile.h"
#include "Syscall.h"
#include "utils/heap.h"
#include "elf_reader/elf_reader.h"
#include "instructionConstants.h"

//////////////////////////////////////////////////////////
// Instruction Constants
//////////////////////////////////////////////////////////

//Opcode Values

#define OP_SPECIAL 0b00000000;
#define OP_REGIMM  0b00000001;
#define OP_ADDI    0b00001000;
#define OP_ADDIU   0b00001001;
#define OP_ANDI    0b00001100;
#define OP_XORI    0b00001110;
#define OP_ORI     0b00001101;
#define OP_SLTI    0b00001010;
#define OP_SLTIU   0b00001011;
#define OP_BEQ     0b00000100;
#define OP_BEQL    0b00010100;
#define OP_BGTZ    0b00000111;
#define OP_BLEZ    0b00000110;
#define OP_BLEZL   0b00010110;
#define OP_BNE     0b00000101;
#define OP_BNEL    0b00010101;
#define OP_J       0b00000010;
#define OP_JAL     0b00000011;
#define OP_LB      0b00100000;
#define OP_LBU     0b00100100;
#define OP_LH      0b00100001;
#define OP_LHU     0b00100101;
#define OP_LUI     0b00001111;
#define OP_LW      0b00100011;
#define OP_LWL     0b00100010;
#define OP_LWR     0b00100110;
#define OP_SB      0b00101000;
#define OP_SH      0b00101001;
#define OP_SW      0b00101011;
#define OP_SWL     0b00101010;
#define OP_SWR     0b00101110;

//Func Values

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
        
        
        //list of codes
        //OPCODES:
        
        
        
        //FUNC CODES:
        //SPECIAL
        const unsigned char add = 0;
        
        
        //Look
        //Test: print the opcode for the current instruction.
        printf("Current opcode =  %s\n",byte_to_binary(opcode));
        
        switch(opcode) {
                <<<<<<< Updated upstream
                
            case 0b1000:
                //addi
                printf("--addi--\n" );
                break;
            case 0b1001:
                //addiu
                printf("--addiu--\n" );
                break;
            case 0b1100:
                //andi
                printf("--andi--\n" );
                break;
            case 0b1110:
                //xori
                printf("--xori--\n" );
                break;
            case 0b1101:
                //ori
                printf("--ori--\n" );
                break;
            case 0b1010:
                //slti
                printf("--slti--\n" );
                break;
            case 0b1011:
                //sltiu
                printf("--sltiu--\n" );
                break;
            case 0b100:
                //beq
                printf("--beq--\n" );
                break;
            case 0b10100:
                //beql
                printf("--beql--" );
                break;
            case 0b111:
                //bgtz
                printf("--bgtz--\n" );
                break;
            case 0b110:
                //blez
                printf("--blez--\n" );
                break;
            case 0b10110:
                //blezl
                printf("--blezl--\n" );
                break;
            case 0b101:
                //bne
                printf("--bne--\n" );
                break;
            case 0b1010:
                //blezl
                printf("--blezl--\n" );
                break;
            case 0b10:
                //j
                printf("--j--\n" );
                break;
            case 0b100000:
                //LB
                printf("--LB--\n" );
                break;
            case 0b100100:
                //LBU
                printf("--LBU--\n" );
                break;
            case 0b1000001:
                //LH
                printf("--LH--\n" );
                break;
            case 0b100101:
                //LHU
                printf("--LHU--\n" );
                break;
            case 0b1111:
                //LUI
                printf("--LUI--\n" );
                break;
            case 0b10011:
                //LW
                printf("--LW--\n" );
                break;
            case 0b100010:
                //LWL
                printf("--LWL--\n" );
                break;
            case 0b100110:
                //LWR
                printf("--LWR--\n" );
                break;
            case 0b101000:
                //SB
                printf("--SB--\n");
                break;
            case 0b101001:
                //SH
                printf("--LWR--\n" );
                break;
            case 0b101011:
<<<<<<< HEAD
                //sltu
                printf("--sltu--\n" );
                RegFile[RD] = (RegFile[RS] < RegFile[RT]);
                 break;
            case 0b11:
                //sra
                printf("--sra--\n" );
                RegFile[RD] = RegFile[RT] + RegFile[shamt];
                 break;
            case 0b111:
                //srav
                printf("--srav--\n" );
                RegFile[RD] = RegFile[RT] >> RegFile[RS];
                 break;
            case 0b10:
                //srl
                printf("--srl--\n" );
                RegFile[RD] = RegFile[RT] >> RegFile[shamt];
                 break;
            case 0b110:
                //srlv
                printf("--srlv--\n" );
                RegFile[RD] = RegFile[RT] >> RegFile[RS];
                 break;
            case 0b1001:
                //jalr
                printf("--jalr--\n" );
                 break;
            case 0b1000:
                //jr
                printf("--jr--\n" );
                 break;
            case 0b1100:
                //NOP
                printf("--NOP--\n" );
                //DO NOTHING
                 break;

        default:
            printf("THE DEFAULT CASE WAS EXECUTED IN SECOND SWITCH\n" );
=======
                //SW
                printf("--SW--\n" );
                break;
            case 0b101010:
                //SWL
                printf("--SWL--\n" );
                break;
            case 0b101110:
                //SWR
                printf("--LWR--\n" );
                break;
            case 0b0:
                //SPECIAL CASE OPCODE = 0
                //TO CHNAGE NEED TO CHANGE THE DO BITWISE TO GET THE BITS TO COMPARE
                printf("SPECIAL CASE OPCODE = 0");
                unsigned char SPECIAL = ((opcode)) & (0b00111111);
                printf("FUNC = %s\n",byte_to_binary(SPECIAL));
                // getting the last bits to compare in second switch statment
                //Getting RS, RT, RD, shamt
                unsigned char RS, RT, RD, shamt, temp;
                RS = ((CurrentInstruction) >> 21) & (0b00111111);
                RT = ((CurrentInstruction) >> 16) & (0b00111111);
                RD = ((CurrentInstruction) >> 11) & (0b00111111);
                shamt = ((CurrentInstruction) >> 6) & (0b00111111);
                
                switch(SPECIAL) {
                    case 0b100000:
                        //add
                        printf("--add--\n" );
                        break;
                    case 0b100001:
                        //addu
                        printf("--addu--\n" );
                        break;
                    case 0b100010:
                        //sub
                        printf("--sub--\n" );
                        break;
                    case 0b100011:
                        //subu
                        printf("--subu--\n" );
                        break;
                    case 0b11010:
                        //add
                        printf("--div--\n" );
                        break;
                    case 0b11011:
                        //divu
                        printf("--divu--\n" );
                        break;
                    case 0b11000:
                        //mult
                        printf("--mult--\n" );
                        break;
                    case 0b11001:
                        //multu
                        printf("--multu--\n" );
                        break;
                    case 0b10000:
                        //mfhi
                        printf("--mfhi--\n" );
                        break;
                    case 0b10010:
                        //mflo
                        printf("--mflo--\n" );
                        break;
                    case 0b10001:
                        //mthi
                        printf("--mthi--\n" );
                        break;
                    case 0b11001:
                        //mtlo
                        printf("--mtlo--\n" );
                        break;
                    case 0b100100:
                        //and
                        printf("--and--\n" );
                        break;
                    case 0b100110:
                        //xor
                        printf("--xor--\n" );
                        break;
                    case 0b100111:
                        //xor
                        printf("--nor--\n" );
                        break;
                    case 0b101010:
                        //slt
                        printf("--slt--\n" );
                        break;
                    case 0b101011:
                        //sltu
                        printf("--sltu--\n" );
                        break;
                    case 0b11:
                        //sra
                        printf("--sra--\n" );
                        break;
                    case 0b111:
                        //srav
                        printf("--srav--\n" );
                        break;
                    case 0b10:
                        //srl
                        printf("--srl--\n" );
                        break;
                    case 0b110:
                        //srlv
                        printf("--srlv--\n" );
                        break;
                    case 0b1001:
                        //jalr
                        printf("--jalr--\n" );
                        break;
                    case 0b1000:
                        //jr
                        printf("--jr--\n" );
                        break;
                    case 0b1100:
                        //NOP
                        printf("--NOP--\n" );
                        break;
                        
                    default:
                        printf("THE DEFAULT CASE WAS EXECUTED IN SECOND SWITCH\n" );
                }
                
            default:
                printf("THE DEFAULT CASE WAS EXECUTED IN FIRST SWTICH\n" );
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


