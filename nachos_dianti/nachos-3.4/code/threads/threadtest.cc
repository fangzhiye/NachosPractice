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
#include "synch.h"
// testnum is set in main.cc
int testnum = 2;
int file = 1;
int countnum = 0;
int pronum = 0;
int readerCount = 0;
Lock* rcLock;
Lock* wlock;

List* proList;
Semaphore* sema;
Lock* lock; 
Condition* condition;

class Production{
public:
    int id;
    Production(){
        pronum++;
        id = pronum;
        
    }
    ~Production(){}
};


void
read(int n){
    for(int i = 0; i < n; i++){
        rcLock->Acquire();
        readerCount++;
        if(readerCount == 1){
            wlock->Acquire();
        }
        rcLock->Release();

        printf("readerThread:%s,filecontent:%d\n",currentThread->getName(),file);
        rcLock->Acquire();
        readerCount--;
        if(readerCount == 0){
            wlock->Release();
        }
        rcLock->Release();
        currentThread->Yield();
    }
}

void write(int n){
    for(int i = 0;i<n;i++){
        wlock->Acquire();
        file++;
        printf("writeThread:%s,filecontent:%d\n",currentThread->getName(),file);
        wlock->Release();
        currentThread->Yield();
    }
}



void barrierTest(int n){
    lock->Acquire();
    countnum++;
    currentThread->Print();
    if(countnum == 3){
        printf("countnum:%d,Broadcast.\n",countnum);
        condition->Broadcast(lock);
        lock->Release();
    }else{
        printf("countnum:%d,Wait.\n",countnum);
        condition->Wait(lock);
        lock->Release();
    }
    printf("currentThread:%s, continue to run...\n",currentThread->getName());
    currentThread->Print();
}


//对于生产者而言，我每生产一个产品，就可以唤醒一个消费者
void produce(int n){
    for(int i = 0; i < n; i++){
        Production* pro = new Production();
        proList->Append(pro);
        printf("thread: %s, produced production: %d\n",currentThread->getName(),pro->id);
        //printf("currentThread usedTimeSlices:%d\n",currentThread->usedTimeSlices);
        sema->V();
    }
}


void produceWithLock(int n){
    for(int i = 0; i < n; i++){
        //printf("here3");
        lock->Acquire();
        //printf("here4");
        //我这让lock中的value为0,那么另一个要lock的时候是不行的，它要阻塞加入等待队列中
        //用锁的话，就是容易这发生中断，因为Acquire()这开关断了，会调用OneTick()，然后时间片到了就会切换
        //但这时lock value为0,其它线程就要等待了
        Production* pro = new Production();
        proList->Append(pro);
        printf("thread: %s, produced production: %d\n",currentThread->getName(),pro->id);
       // printf("here1");
        lock->Release();//我release后，其它线程就会被唤起
      //  printf("here2");
       //  printf("here1 %d\n",lock->lockSemaphore->value);
    }
}

void consumeWithLock(int n){
    //  printf("here2 %d\n",lock->lockSemaphore->value);
    for(int i = 0; i < n; i++){
        lock->Acquire();
        if(proList->IsEmpty()){
            printf("thread:%s,no production\n",currentThread->getName());
         //   lock->Acquire();
            lock->lockSemaphore->value = 1;
            currentThread->Yield();
            //lock->Release();
           // lock->Release();
            //如果没有产品的话，应该释放锁
           
            //lock->lockSemaphore->queue->Append((void *)currentThread);
          // lock->lockSemaphore->queue->Append((void *)currentThread);
           // lock->lockSemaphore->value--;
           // lock->Acquire();
             i--;
           // if(lock->isHeldByCurrentThread)

            //interrupt->SetLevel(IntOff);
            //lock->Acquire();
           // lock->Release();
           // scheduler->ReadyToRun(this);
           // while ((nextThread = scheduler->FindNextToRun()) == NULL)
           // interrupt->Idle();  // no one to run, wait for an interrupt
                        //如果为空的话就用Idle()
           // scheduler->Run(nextThread); // re
           // currentThread->Sleep();
            //interrupt->SetLevel(IntOn);
        }
        else{
            Production* pro = (Production*)proList->Remove();
            printf("thread: %s, consume production:%d\n",currentThread->getName(),pro->id);
       // printf("currentThread usedTimeSlices:%d\n",currentThread->usedTimeSlices);
            delete pro;
            lock->Release();
        }
    }
}
//对于消费者而言，如果现在sema.value=0的话，说明还不能用，我就要进入阻塞状态，并等待，同
void consume(int n){
    for(int i = 0; i < n; i++){
        sema->P();
        Production* pro = (Production*)proList->Remove();
        printf("thread: %s, consume production:%d\n",currentThread->getName(),pro->id);
       // printf("currentThread usedTimeSlices:%d\n",currentThread->usedTimeSlices);
        delete pro;
    }
}
//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
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
   /* DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, (void*)1);
    SimpleThread(0);*/

    proList = new List();
    sema = new Semaphore("procon",0);

    Thread* p1 = new Thread("producer1");
    Thread* c1 = new Thread("consumer1");
    Thread* c2 = new Thread("consumer2");
   
    c1->Fork(consume,12);
    p1->Fork(produce,20);
    c2->Fork(consume,8);
}


void
ThreadTest2()
{
   /* DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, (void*)1);
    SimpleThread(0);*/

    proList = new List();
    lock = new Lock("lock");

    Thread* p1 = new Thread("producer1");
    Thread* c1 = new Thread("consumer1");
    Thread* c2 = new Thread("consumer2");
    p1->Fork(produceWithLock,15);
    c1->Fork(consumeWithLock,8);
    c2->Fork(consumeWithLock,6);
}

void
ThreadTest3(){
    lock = new Lock("myLock");
    condition = new Condition("myCondition");
    Thread* t1 = new Thread("t1");
    Thread* t2 = new Thread("t2");
    Thread* t3 = new Thread("t3");


    t1->Fork(barrierTest,1);
    t2->Fork(barrierTest,1);
    t3->Fork(barrierTest,2);
}

void
ThreadTest4(){
    rcLock = new Lock("rcLock");
    wlock = new Lock("wlock");

    Thread* r1 = new Thread("r1");
    Thread* r2 = new Thread("r2");
    Thread* r3 = new Thread("r3");
    Thread* w1 = new Thread("w1");
    Thread* w2 = new Thread("w2");

    r1->Fork(read,3);
    w1->Fork(write,3);
    r2->Fork(read,3);
    w2->Fork(write,3);
    r3->Fork(read,3);
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
    case 2:
    ThreadTest2();
    break;
    case 3:
    ThreadTest3();
    break;
    case 4:
    ThreadTest4();
    break;
    default:
	printf("No test specified.\n");
	break;
    }
}

