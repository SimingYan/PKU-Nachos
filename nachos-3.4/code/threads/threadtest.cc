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
int testnum = 1;

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

void
SimpleThread1(int which)
{
    if(which > 0){
        Thread* new_thread = new Thread("forked", which);
        new_thread->Fork(SimpleThread1, which - 1);
        //currentThread->Yield();
        printf("current thread priority: %d\n", currentThread->get_priority());
    }
}

void
LongTimeJob(int which)
{
    int num;
    for (num = 0; num < 100; num++) {
        printf("*** thread %d with priority %d looped %d times\n",
             which, currentThread->get_priority(), num);
        interrupt->SetLevel(IntOn);
        interrupt->SetLevel(IntOff);
    }
}

void
ForkAndLoop(int which)
{
    int num;
    Thread* arr[3];
    for (num = 0; num < 3; num++) {
        printf("*** thread %d with priority %d forked and looped %d times\n",
             which, currentThread->get_priority(), num);
        arr[num] = new Thread("forked", 3 - num);
        arr[num]->Fork(SimpleThread, (void*)arr[num]->get_tid());
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
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, (void*)1);
    SimpleThread(0);
}

void
ThreadTest2()
{
    DEBUG('t', "Entering ThreadTest2");
    
    for(int i = 0; i < 130; ++i){
        Thread *t = new Thread("forked thread");
    } 
}

// test priority
void
ThreadTest3()
{
    DEBUG('t', "Entering ThreadTest3");
    Thread *t = new Thread("thread", 3);
        
    t->Fork(SimpleThread1, 5);

}

// test multi feedback queue
void
ThreadTest4()
{
    DEBUG('t', "Entering ThreadTest4");
    Thread* t[2];
    t[0] = new Thread("LongTimeJob", 1);
    t[0]->Fork(LongTimeJob, (void*)t[0]->get_tid());
    t[1] = new Thread("Important&Fast", 0);
    t[1]->Fork(SimpleThread, (void*)t[1]->get_tid());
    ForkAndLoop(0);
}

static int productnum = 0;
Semaphore *rest, *avail;
Semaphore *mutex;
void
Producer(int which)
{
    DEBUG('t', "Entering Producer");

    for(int i = 0; i < 10; i++)
    {
        rest->P();
        mutex->P();
        productnum++;
        printf("Producer %d produced one product. current num=%d\n",
            which, productnum);
        mutex->V();
        avail->V();
    }
}

void
Consumer(int which)
{
    DEBUG('t', "Entering Consumer");
    for(int i = 0; i < 8; i++)
    {
        avail->P();
        mutex->P();
        productnum--;
        printf("consumer %d buy one product. current num=%d\n",
            which, productnum);
        mutex->V();
        rest->V();
    }
}

void
ThreadTest5()
{
    DEBUG('t', "Entering ThreadTest6");
    rest = new Semaphore("restpos", 6);
    avail = new Semaphore("availnum", 0);
    mutex = new Semaphore("mutex", 1);
    Thread *p[2], *c[2];
    for(int i = 0; i < 2; i++)
    {
        p[i] = new Thread("producer", 3);
        p[i]->Fork(Producer, (void*)i);
    }
    for(int i = 0; i < 2; i++)
    {
        c[i] = new Thread("consumer", 3);
        c[i]->Fork(Consumer, (void*)i);
    }
}

static int maxproduct = 0;
Condition *cond;
Lock *lock;

void
Con_Producer(int which)
{
    DEBUG('t', "Entering Producer");

    for(int i = 0; i < 10; i++)
    {
        lock->Acquire();
        while(productnum >= maxproduct)
        {
            printf("full!(%d)\n", which);
            cond->Wait(lock);
            printf("back to producing(%d)\n", which);
        }
        productnum++;
        printf("Producer %d produced one product. current num=%d\n",
             which, productnum);
        if(productnum == 1)
            cond->Broadcast(lock);
        lock->Release();
    }
}
void
Con_Consumer(int which)
{
    DEBUG('t', "Entering Consumer");
    for(int i = 0; i < 8; i++)
    {
        lock->Acquire();
        while(productnum <= 0)
        {
            printf("empty!(%d)\n", which);
            cond->Wait(lock);
            printf("product available(%d)\n", which);
        }
        productnum--;
        printf("Consumer %d buy one product. current num=%d\n",
            which, productnum);
        if(productnum == maxproduct - 1)
            cond->Broadcast(lock);
        lock->Release();
    }
}

void
ThreadTest6()
{
    DEBUG('t', "Entering ThreadTest5");
    cond = new Condition("condition");
    lock = new Lock("lock");
    maxproduct = 7;
    Thread *p[2], *c[2];
    for(int i = 0; i < 2; i++)
    {
        p[i] = new Thread("producer", 3);
        p[i]->Fork(Con_Producer, (void*)i);
    }
    for(int i = 0; i < 2; i++)
    {
        c[i] = new Thread("consumer", 3);
        c[i]->Fork(Con_Consumer, (void*)i);
    }
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
    case 5:
    ThreadTest5();
    break;
    case 6:
    ThreadTest6();
    break;
    default:
	printf("No test specified.\n");
	break;
    }
}

