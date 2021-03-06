// progtest.cc 
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.  
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"

//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------
void ForkThread(){
    printf("start second thread\n");
   // printf("%s\n",currentThread->getName() );
    machine->Run();
}
void
StartProcess(char *filename)
{
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    //OpenFile *executable2 = fileSystem->Open("..//test//testLab4");
 //   OpenFile *executable2 = fileSystem->Open(filename);
 //   AddrSpace *space2;

  //  Thread* thread = new Thread("second_thread");
    if (executable == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
    }
  //  if (executable2 == NULL) {
  //  printf("Unable to open file %s\n", filename);
 //   return;
 //   }
    printf("initial first thread address\n");
    space = new AddrSpace(executable);    
    space->InitRegisters();     // set the initial register values
    space->RestoreState();
   // printf("initial second thread address space\n");
  //  space2 = new AddrSpace(executable2);
  //    //printf("789\n");
    //thread->Fork(ForkThread,1);
   // printf("12789\n");
   // space2->InitRegisters();     // set the initial register values
   // space2->RestoreState();      
    //thread->space = space2;

    currentThread->space = space;
   
  
   // 
   // currentThread->Yield();
    delete executable;			// close file
  //  delete executable2;  
    
    printf("start first thread\n");
    
 //   space->RestoreState();      // load page table register
    //我在切换线程的时候也要切换过来Machine的PageTable[]
    machine->Run();			// jump to the user progam

    
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void 
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    
    for (;;) {
	readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
	writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}
