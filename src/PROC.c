//////////////////////////////////////////////////////////
//
// CSC 252 Project 3
//
// Wilfred Wallis and Chris Dalke
//
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "RegFile.h"
#include "Syscall.h"
#include "utils/heap.h"
#include "elf_reader/elf_reader.h"
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
#define  OP_BGTZ    0b00000111
#define  OP_BLEZ    0b00000110
#define  OP_BNE     0b00000101
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
#define FUNC_ADDU   0b00100001
#define FUNC_SUB    0b00100010 
#define FUNC_SUBU   0b10000011
#define FUNC_DIV    0b00011010
#define FUNC_DIVU   0b00011011
#define FUNC_MULT   0b00011000
#define FUNC_MULTU  0b00011001
#define FUNC_MFHI   0b00010000
#define FUNC_MFLO   0b00010010
#define FUNC_MTHI   0b00010001
#define FUNC_MTLO   0b00010011
#define FUNC_AND    0b00100100
#define FUNC_XOR    0b00100110
#define FUNC_NOR    0b00100111
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
#define FUNC_SYSCALL 0b00001100

//////////////////////////////////////////////////////////
// Main Program Initialization
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

//This is used for addi methohds
int signExtension(int input) {
    int output = (0x0000FFFF & input);
    if (0x00008000 & input) {
        output = output + 0xFFFF0000;
    }
    return output;
}

int main(int argc, char * argv[]) {


    int zeroComparisonType;
    int MaxInst = 0;
    int status = 0;
    uint32_t i;
    uint32_t PC,newPC;
    uint32_t CurrentInstruction;
    
    //High and Low registers
    uint32_t HIGH,LOW;
    
    int branchDelayStatus = 0;
    
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
        uint32_t opcode = ((CurrentInstruction) >> 26) & (0b00111111);
        
        // ----------------------------------------------------
        //Preload any variables that we might potentially need
        
        uint32_t RS = ((CurrentInstruction) >> 21) & (0b0011111);
        uint32_t RT = ((CurrentInstruction) >> 16) & (0b0011111);
        uint32_t RD = ((CurrentInstruction) >> 11) & (0b11111);
        uint32_t shamt = ((CurrentInstruction) >> 5) & (0b0011111);
        uint32_t OFFSET = ((CurrentInstruction) & (0b111111111111111));
        uint32_t BASE = (((CurrentInstruction) >> 21) & (0b1111));
        uint32_t SPECIAL = ((CurrentInstruction)) & (0b00111111);
        uint32_t immediate = ((CurrentInstruction)) & (0b1111111111111111);
        uint32_t immediateExtended = signExtension(immediate);
        
        uint32_t instrIndex = ((PC + 4) & 0xF0000000) | ((CurrentInstruction & 0b00000111111111111111111111111111) << 2);
        
        uint32_t branchTarget = immediateExtended << 2;
        
        //Also store immediate as a signed integer
        int immediateSigned = signExtension(((CurrentInstruction)) & (0b1111111111111111));

        //Double-size word variables
        int64_t finalNumber, lowNumber, highNumber;
        // ----------------------------------------------------

        switch(opcode) {
                
            //////////////////////////////////////////////////////////
            // Immediate Mode Math and Bit Operators
            //////////////////////////////////////////////////////////
            
            //Add Immediate (Signed)
            case OP_ADDI:
            {
                RegFile[RT] = RegFile[RS] + immediateSigned;
                break;
            }
                
            //Add Immediate (Unsigned)
            case OP_ADDIU:
            {
                uint32_t temp = RegFile[RS] + immediateExtended;
                RegFile[RT] = temp;
                break;
            }
                
            //Bit AND Immediate
            case OP_ANDI:
            {
                RegFile[RT] = RegFile[RS] + immediateExtended;
                break;
            }
                
            //Bit XOR Immediate
            case OP_XORI:
            {
               
                RegFile[RT] = (RegFile[RS] ^ immediate);
                break;
            }
                
            //Bit OR Immediate
            case OP_ORI:
            {
                
                RegFile[RT] = (RegFile[RS] | immediate);
                break;
            }
                
            //Set on Less Than Immediate
            case OP_SLTI:
            {
                if (RegFile[RS] < immediateExtended) {
                    RegFile[RS] = 1;
                } else {
                    RegFile[RS] = 0;
                }
                break;
            }
                
            //Set on Less Than Immediate Unsigned
            case OP_SLTIU:
            {
             
                //This implementation taken from the mips handbook
                //Check to make sure this actually works properly
                if ((0 || RegFile[RS]) < (0 || signExtension(immediate))) {
                    RegFile[RS] = 1;
                } else {
                    RegFile[RS] = 0;
                }
                break;
            }
                
            //////////////////////////////////////////////////////////
            // Branch Instructions
            //////////////////////////////////////////////////////////
                
                //TODO: branches are completely broken
                //currently logic for branches are executed in the wrong order
                
            //Branch on Equal
            case OP_BEQ:
            {
              
                if (RegFile[RS] == RegFile[RT]){
                    newPC = PC + branchTarget;
                    branchDelayStatus = 1; //starts branch
                }
                break;
            }
                
            //Branch on Greater Than Zero
            case OP_BGTZ:
            {
              
                if (RegFile[RS] > 0){
                    newPC = PC + branchTarget;
                    branchDelayStatus = 1; //starts branch
                }
                break;
            }
                
            //Branch on Less Than Zero
            case OP_BLEZ:
            {
              
                
                if (RegFile[RS] <= 0){
                    newPC = PC + branchTarget;
                    branchDelayStatus = 1; //starts branch
                }
                
                break;
            }
                
            //Branch on Not Equal
            case  OP_BNE:
            {
                
                if (RegFile[RS] != RegFile[RT]){
                    newPC = PC + branchTarget;
                    branchDelayStatus = 1; //starts branch
                }
                break;
            }
                
            //Unconditional Jump
            case  OP_J:
            {
                newPC = instrIndex; //update branch target
                branchDelayStatus = 1; //starts branch
                
                break;
            }
                
            //Jump And Link
            case  OP_JAL:
            {
                newPC = instrIndex; //update branch target
                branchDelayStatus = 1; //starts branch
                
                //record the return address into RegFile[31]
                RegFile[31] = PC + 8;
                
                break;
            }
                
            //////////////////////////////////////////////////////////
            // Memory Instructions
            //////////////////////////////////////////////////////////
                
            //Load Byte
            case OP_LB:
            {
                
                //This needs to take the sign bit and shift it over!!
                uint32_t vAddr = immediateExtended + signExtension(RegFile[RS]);
                int8_t vData = readByte(vAddr,false);
                RegFile[RT] = vData;
                break;
            }
                
            //Load Byte Unsigned
            case OP_LBU: {
                
                uint32_t vAddr = immediateExtended + signExtension(RegFile[RS]);
                uint8_t vData = readByte(vAddr,false);
                RegFile[RT] = vData;
                break;
            }
            //Load Half-Word
            case OP_LH:
            {
                RegFile[RT] = (readWord(OFFSET+BASE,false) & 0b11111111111111);
                break;
            }
                
            //Load Half-Word Unsigned
            case OP_LHU:
            {
                break;
            }
                
            //Load Upper Immediate
            case OP_LUI:
            {
                RegFile[RT] = (immediate <<16); //move immediate 16 bits
                break;
            }
                
            //Load Word
            case OP_LW:
            {
                RegFile[RT] = readWord(RegFile[RS] + immediateSigned,false);
                break;
            }
                
            //Load Word Left
            case OP_LWL:
            {
                //Loads the most-significant part of a word as a signed value from an unaligned memory address
                //TODO

                RegFile[RT] = RegFile[RT] | readWord(OFFSET+BASE,false);  //rt ← rt MERGE memory[base+offset]
                
                break;
            }
                
            //Load Word Right
            case OP_LWR:
            {
                //Loads the least-significant part of a word as a signed value from an unaligned memory address
                //TODO
                
                RegFile[RT] = RegFile[RT] | readWord(OFFSET+BASE,false); //rt ← rt MERGE memory[base+offset]

                break;
            }
                
            //Store Byte
            case OP_SB:
            {
                writeByte(OFFSET+BASE,RegFile[RT], false);//memory[base+offset] ← rt
          
                break;
            }
                
            //Store Half-Word
            case OP_SH:
            {
            
                writeWord(OFFSET+BASE, (RegFile[RT] & 0b1111111111111111), false) ;//memory[base+offset] ← rt
                break;                  
            }
                
            //Store Word
            case OP_SW:
            {
             
                //Stores a word into the specified memory location
                 writeWord(OFFSET+BASE, RegFile[RT], false);
                
                break;
            }
                
            //Store Word Left
            case OP_SWL:
            {
                writeWord(OFFSET+BASE, RegFile[RT], false);//memory[base+offset] ← rt
                break;
            }
                
            //Store Word Right
            case OP_SWR:
            {
                writeWord(OFFSET+BASE, RegFile[RT], false); // memory[base+offset] ← rt
                break;
            }
                
            //////////////////////////////////////////////////////////
            // R-Type Instructions
            //////////////////////////////////////////////////////////
                
            case OP_SPECIAL:
                switch(SPECIAL) {
                        
                    //////////////////////////////////////////////////////////
                    // Math Operators
                    //////////////////////////////////////////////////////////
                        
                    case FUNC_ADD:
                    {
                        RegFile[RD] = RegFile[RS] + RegFile[RT];
                        break;
                    }
                        
                    case FUNC_ADDU:
                    {
                        RegFile[RD] = RegFile[RS] + RegFile[RT];
                        break;
                    }
                        
                    case  FUNC_SUB:
                    {
                        RegFile[RD] = RegFile[RS] - RegFile[RT];
                        break;
                    }
                        
                    case  FUNC_SUBU:
                    {
                        RegFile[RD] = RegFile[RS] - RegFile[RT];
                        break;
                    }
                        
                    case FUNC_DIV: //if you div by 0, answer is UNPREDICTABLE!
                    {
                        lowNumber = RegFile[RS] / RegFile[RT];
                        LOW = lowNumber;

                        highNumber = RegFile[RS] % RegFile[RT]; 
                        HIGH = highNumber;
                        break;
                    }
                        
                    case FUNC_DIVU:
                    {
                         lowNumber = RegFile[RS] / RegFile[RT];
                        LOW = lowNumber;

                        highNumber = RegFile[RS] % RegFile[RT]; 
                        HIGH = highNumber;
                        break;
                    }
                        
                    case FUNC_MULT: //not sure if I did this correctly
                    {
                        //high = 0-31, of 64 bit number, 32-64 is low, see ->
                        finalNumber = RegFile[RS] * RegFile[RT];
                        HIGH = ((finalNumber >> 32) & (0b11111111111111111111111111111111));
                        LOW = ((finalNumber) & (0b11111111111111111111111111111111));
                        break;
                    }
                        
                    case FUNC_MULTU: // same as above
                    {
                      //high = 0-31, of 64 bit number, 32-64 is low, see ->
                           finalNumber = (RegFile[RS] * RegFile[RT]); 
                            HIGH = ((finalNumber >> 32) & (0b00111111));
                            LOW = ((finalNumber << 32) & (0b00111111));
                        break;
                    }
                        
                    //Move From High
                    case FUNC_MFHI:
                    {
                        RegFile[RD] = HIGH;
                        break;
                    }
                        
                    //Move From Low
                    case FUNC_MFLO:
                    {
                        RegFile[RD] = LOW;
                        break;
                    }
                        
                    //Set on Less Than
                    case  FUNC_SLT:
                    {
                        RegFile[RD] = (RegFile[RS] < RegFile[RT]);
                        break;
                    }
                        
                    //Move To High
                    case FUNC_MTHI:
                    {
                        
                        HIGH = RegFile[RS];
                        break;
                    }
                        
                    //Move To Low
                    case FUNC_MTLO:
                    {
                        LOW = RegFile[RS];
                        break;
                    }
                        
                    //////////////////////////////////////////////////////////
                    // Bit Logic Operators
                    //////////////////////////////////////////////////////////
                    
                    //AND
                    case FUNC_AND:
                    {
                   
                        RegFile[RD] = RegFile[RS] & RegFile[RT];
                        break;
                    }
                        
                    //XOR
                    case FUNC_XOR:
                    {
                        
                        RegFile[RD] = RegFile[RS] ^ RegFile[RT];
                        break;
                    }
                        
                    //NOR
                    case FUNC_NOR:
                    {
                       
                        RegFile[RD] = ~(RegFile[RS]| RegFile[RT]);
                        break;
                    }
                        
                    //OR
                    case FUNC_OR:
                    {
                    
                        RegFile[RD] = RegFile[RS] | RegFile[RT];
                        break;
                    }
                        
                    //SLL (Shift Left Logical)
                    case FUNC_SLL:
                    {
                        
                        RegFile[RD] = RegFile[RT] << shamt;
                        break;
                    }
                        
                    //SLLV (Shift Left Logical Variable)
                    case FUNC_SLLV:
                    {
                     
                        RegFile[RD] = RegFile[RT] << RegFile[RS];
                        break;
                    }
                        
                    //Set On Less Than Unsigned
                    case FUNC_SLTU:
                    {
                    
                        RegFile[RD] = (RegFile[RS] < RegFile[RT]);
                        break;
                    }
                        
                    //Shift Word Right Arithmetic
                    case FUNC_SRA:
                    {
                    
                        //Shifts right but arithmetically so copy sign bit
                        RegFile[RD] = RegFile[RT] >> shamt;
                        break;
                    }
                        
                    //Shift Word Right Arithmetic Variable
                    case FUNC_SRAV:
                    {
                      
                        RegFile[RD] = RegFile[RT] >> RegFile[RS];
                        break;
                    }
                        
                    //SRL (Shift Right Logical)
                    case FUNC_SRL:
                    {
                 
                        RegFile[RD] = RegFile[RT] >> shamt;
                        break;
                    }
                        
                    //SRLV (Shift Right Logical Variable)
                    case FUNC_SRLV:
                    {
                 
                        RegFile[RD] = RegFile[RT] >> RegFile[RS];
                        break;
                    }
                        
                    //////////////////////////////////////////////////////////
                    // Jump Instructions (R-Type)
                    //////////////////////////////////////////////////////////
                        
                    //Jump and Link Register
                    case FUNC_JALR:
                    {
                        newPC = RegFile[RS]; //update branch target
                        branchDelayStatus = 1; //starts branch
                        //Update return address
                        RegFile[RD] = PC + 8;
                        break;
                    }
                        
                    //Jump Register
                    case FUNC_JR:
                    {
                        newPC = RegFile[RS]; //update branch target
                        branchDelayStatus = 1; //starts branch
                        break;
                    }
                        
                    //Syscall
                    case FUNC_SYSCALL:
                    {
                        //printf("parameter = %i\n",RegFile[2]);
                        //Load the parameter from v0 (register 2)
                        SyscallExe(RegFile[2]);
                        break;
                    }
                        
                    default:
                    {
                        //printf("FUNC = %s\n",byte_to_binary(SPECIAL));
                    }
                }
                
                break;
                
            case OP_REGIMM:
            {
                //////////////////////////////////////////////////////////
                // Compare to Zero Branch Instructions
                //////////////////////////////////////////////////////////
                
                //Get The code to see which type of comparison we should do
                zeroComparisonType = RegFile[RT];
                bool doBranch = false;
                bool doLink = false;
             
                
                switch (zeroComparisonType){
                    //Branch Greater Than or Equal to Zero
                    case 0b00000001: if (RegFile[RS] >= 0){ doBranch = true; } break;
                    //Branch Greater Than or Equal to Zero And Link
                    case 0b00010001: if (RegFile[RS] >= 0){ doBranch = true; doLink = true; } break;
                    //Branch Less Than Zero
                    case 0b00000000: if (RegFile[RS] < 0) { doBranch = true; } break;
                    //Branch Less Than Zero And Link
                    case 0b00010000: if (RegFile[RS] < 0) { doBranch = true; doLink = true; } break;
                    default: break;
                }
                
                if (doBranch){
                    newPC = PC + branchTarget; //update branch target
                    branchDelayStatus = 1; //starts branch
                    if (doLink){
                        RegFile[RD] = PC + 8;
                    }
                }
                break;
            }
                
            default:
            {
                //printf("OPCODE = %s\n",byte_to_binary(opcode));
                break;
            }
        }
        
        //After we are done, increase the program counter.
        PC = PC + 4;
        
        //Handle branch delay slot if there is a branch command executing
        if (branchDelayStatus == 1){
            branchDelayStatus = 2;
            //printf("Starting jump...\n");
        } else if (branchDelayStatus == 2){
            //printf("Finishing jump...\n");
            //printf("old PC = %d\n",PC);
            //printf("new PC = %d\n",newPC);
            PC = newPC;
            branchDelayStatus = 0;
        }
        
        //Hardcode the zero register to always be zero
        RegFile[0] = 0;
        
        
        //printRegFile();
        
        //////////////////////////////////////////////////////////
        // End of Main Instruction Simulation
        //////////////////////////////////////////////////////////
      
    } //end fori
    
    printRegFile();
    
    //Close file pointers & free allocated Memory
    closeFDT();
    CleanUp();
    return 1;
}


