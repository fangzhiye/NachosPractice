// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "elevatortest.h"
//#include "interrupt.h"
// testnum is set in main.cc
int testnum = 1;
extern Interrupt *interrupt;
//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int period)
{
    int num;
    
    for (num = 0; num < period; num++) {
    interrupt->SetLevel(IntOff);
	printf("*** %s looped %d times use %d timeslices\n", currentThread->getName(), num+1, currentThread->usedTimeSlices+1);
    interrupt->SetLevel(IntOn);
      //  currentThread->Yield();//对于初始版本，其实这个currentThread->Yield()是main thread第一次调用
    }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t1 = new Thread("thread1");//建立了新线程forked thread1
    Thread *t2 = new Thread("thread2");
    Thread *t3 = new Thread("hread3");

    t1->Fork(SimpleThread, 8);
    t2->Fork(SimpleThread, 15);
    t3->Fork(SimpleThread, 17);
    SimpleThread(5);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    default:
	printf("No test specified.\n");
	break;
    }
}

