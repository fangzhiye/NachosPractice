// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
//
    if ((which == SyscallException) && (type == SC_Halt)) {
	DEBUG('a', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();
    } 
    else if(which == PageFaultException){
    	if(machine->tlb != NULL){
    		//处理快表失效
    		//printf("11\n");
    		int vpn = (unsigned) machine->registers[BadVAddrReg]/PageSize;
    		int position = -1;
    		for (int i = 0; i < TLBSize; i++){
    			if(machine->tlb[i].valid == FALSE){
    				position = i;
    				break;
    			}
    		}
    		//FIFO
    		/*if(position == -1){
    			position = TLBSize - 1;
    			for(int i = 0; i < TLBSize -1; i++)
    				machine->tlb[i] = machine->tlb[i+1];
    		}*/
    		//LRU
    		if(position == -1){
    			for(int i = 0; i < TLBSize; i++){
    				if(machine->LRU_queue[i] == TLBSize){
    					position = i;
    					break;
    				}
    			}
    		}
    		machine->LRU_queue[position] = 1;
    		for(int i = 0; i<TLBSize;i++){
    			if(i==position)continue;
    			if(machine->LRU_queue[i]==-1)continue;
    			machine->LRU_queue[i]++;
    		}
    		machine->tlb[position].valid = true;
    		machine->tlb[position].virtualPage = vpn;
    		machine->tlb[position].physicalPage = machine->pageTable[vpn].physicalPage;
    		machine->tlb[position].use = FALSE;
    		machine->tlb[position].dirty = FALSE;
    		machine->tlb[position].readOnly = FALSE;
    	}else{
    	//	
    		OpenFile* openfile = fileSystem->Open("virtual_memory");
    		//printf("12\n");
    		if(openfile == NULL)ASSERT(false);
    		int vpn = (unsigned)machine->registers[BadVAddrReg]/PageSize;

    		//printf("11\n");
    		int pos = machine->find();
    		printf("pagefault in vpn: %d ppn(pos):%d \n",vpn,pos );
    		//printf("12\n");
    		if(pos == -1){
    			pos = 0;
    			for(int j = 0; j < machine->pageTableSize;j++){
    				if(machine->pageTable[j].physicalPage == 0){
    					if(machine->pageTable[j].dirty == TRUE){
    						openfile->WriteAt(&(machine->mainMemory[pos*PageSize]),
    							PageSize,machine->pageTable[j].virtualPage*PageSize);//写回物理页面
    						machine->pageTable[j].valid = FALSE;
    						break;
    					}
    				}
    			}
    		}
    		openfile->ReadAt(&(machine->mainMemory[pos*PageSize]),PageSize,vpn*PageSize);
    		machine->pageTable[vpn].valid = TRUE;
    		machine->pageTable[vpn].physicalPage = pos;
    		machine->pageTable[vpn].use = FALSE;
    		machine->pageTable[vpn].dirty = FALSE;
    		machine->pageTable[vpn].readOnly = FALSE;
    		delete openfile;
    		//处理页表失效
    		//ASSERT(FALSE);
    	}
    }
    else if((which == SyscallException) && (type == SC_Exit)){
    	printf("programme exit\n");
    	printf("%s thread end\n",currentThread->getName());
    	//currentThread->Print();
    	//printf("789\n");
    	machine->clear();
    //	printf("clear end\n");
    	int NextPC = machine->ReadRegister(NextPCReg);
    	machine->WriteRegister(PCReg,NextPC);
    	currentThread->Finish();
    }
    else  {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}
