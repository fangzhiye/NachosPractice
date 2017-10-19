// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

//进程同步具体的讲，就是一个进程运行到某一点的时候，要求另一伙伴进程为它提供消息，
//在未获得消息之前，该进程进入阻塞态，获得消息后被唤醒进入就绪态
void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    //具体而言，当value为0的时候说明信号号不可用，也可说该进程要获得的
    //消息还未满足，所以该进程先阻塞
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();//如果当前进程Sleep的话，那么scheduler会调度就绪队列的下一个进程运行
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    //唤醒一个进程并，增加value
    Thread *thread;

    IntStatus oldLevel = interrupt->SetLevel(IntOff);
 //   if(value >= 0)
    thread = (Thread *)queue->Remove();//唤醒一个等待的线程并将其加入就绪队列中
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;

    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
    //利用Semaphore实现锁
    name = debugName;
    lockSemaphore = new Semaphore(debugName, 1);
}
Lock::~Lock() {}

//这里锁只能被当前thread使用，如果现在锁被其它线程占用的话，就要初始化
void Lock::Acquire() {
    if( heldThread != currentThread)
    {
        lockSemaphore->value = 1;
      //  Thread* thread = (Thread *)lockSemaphore->queue->Remove();//唤醒一个等待的线程并将其加入就绪队列中
       // if (thread != NULL)    // make thread ready, consuming the V immediately
      //  scheduler->ReadyToRun(thread);
        //printf("he\n");
    }
  // 
    lockSemaphore->P();
    Thread* temp;
    temp = currentThread;
    //在这会发生中端的
    heldThread = temp;
}

void Lock::Release() {
    heldThread = NULL;
    lockSemaphore->V();
    //printf("here\n");
}

bool Lock::isHeldByCurrentThread(){
    return currentThread == heldThread;
}

Condition::Condition(char* debugName) {
    name = debugName;
    waitQueue = new List();
 }

Condition::~Condition() { }

void Condition::Wait(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    conditionLock->Release();
    waitQueue->Append((Thread*)currentThread);
    currentThread->Sleep();
    conditionLock->Acquire();
    interrupt->SetLevel(oldLevel);
 }

void Condition::Signal(Lock* conditionLock) {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    Thread* thread;
    if(conditionLock->isHeldByCurrentThread()){
        thread = (Thread*)waitQueue->Remove();
        if(thread != NULL){
            scheduler->ReadyToRun(thread);
        }
    }
}
void Condition::Broadcast(Lock* conditionLock) {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    Thread *thread = NULL;
    if(conditionLock->isHeldByCurrentThread()){
        thread = (Thread*)waitQueue->Remove();
        while(thread != NULL){
            scheduler->ReadyToRun(thread);
            thread = (Thread*)waitQueue->Remove();
        }
    }
    interrupt->SetLevel(oldLevel);
 }
