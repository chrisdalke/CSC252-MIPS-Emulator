
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "RegFile.h"
#include "Syscall.h"
#include "utils/heap.h"
#include "elf_reader/elf_reader.h"

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
    b[0] = '\0';
    
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
        
        //Look
        //Test: print the opcode for the current instruction.
        printf("Current opcode =  %s\n",byte_to_binary(opcode));

        switch(opcode) {
      case '0b1000' :
      //addi
     printf("--addi--\n" );
         break;
      case '0b1001' :
      //addiu
      printf("--addiu--\n" );
      case '0b1100' :
      //andi
      printf("--andi--\n" );
         break;
      case '0b1110' :
      //xori
      printf("--xori--\n" );
         break;
      case '0b1101' :
      //ori
         printf("--ori--\n" );
         break;
     case '0b1010' :
     //slti
         printf("--slti--\n" );
         break;
     case '0b1011' :
     //sltiu
     printf("--sltiu--\n" );
         break;
     case '0b100' :
     //beq
       printf("--beq--\n" );
         break;
     case '0b10100' :
     //beql
     case '0b111' :
     //bgtz
        printf("--bgtz--\n" );
         break;
     case '0b110' :
     //blez
        printf("--blez--\n" );
         break;
    case '0b10110' :
     //blezl
       printf("--blezl--\n" );
         break;
    case '0b101' :
     //bne
        printf("--bne--\n" );
         break;
    case '0b1010' :
     //blezl
       printf("--blezl--\n" );
         break;
    case '0b10' :
     //j
        printf("--j--\n" );
         break;
    case '0b100000' :
     //LB
       printf("--LB--\n" );
         break;
    case '0b100100' :
     //LBU
       printf("--LBU--\n" );
         break;
    case '0b1000001' :
     //LH
       printf("--LH--\n" );
         break;
    case '0b100101' :
     //LHU
       printf("--LHU--\n" );
         break;
    case '0b1111' :
     //LUI
       printf("--LUI--\n" );
         break;
    case '0b10011' :
     //LW
       printf("--LW--\n" );
         break;
    case '0b100010' :
     //LWL
       printf("--LWL--\n" );
         break;
    case '0b100110' :
     //LWR
       printf("--LWR--\n" );
         break;
    case '0b101000' :
     //SB
       printf("--SB--\n");
         break;
    case '0b101001' :
     //SH
       printf("--LWR--\n" );
         break;
    case '0b101011' :
     //SW
       printf("--SW--\n" );
         break;
    case '0b101010' :
     //SWL
       printf("--SWL--\n" );
         break;
    case '0b101110' :
     //SWR
       printf("--LWR--\n" );
         break;
    case '0b100110' :
      default :
         printf("THE DEFAULT CASE WAS EXECUTED\n" );
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


