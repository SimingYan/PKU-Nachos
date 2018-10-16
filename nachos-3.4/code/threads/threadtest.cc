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

void
ThreadTest3()
{
    DEBUG('t', "Entering ThreadTest3");
    Thread *t = new Thread("thread", 3);
        
    t->Fork(SimpleThread1, 5);

}

/*
void Create_P(int p_this)
{
    if(p_this <= 6)
    {
        Thread* new_thread = new Thread("forked", 7 - p_this - 1);
        printf("thread %d with priority %d created\n", p_this + 1, 7 - p_this - 1);
        new_thread->Fork(Create_P, p_this + 1);
        printf("thread %d finished\n", p_this);
    }
}

void ThreadTest3()
{
    DEBUG('t', "Entering ThreadTest3");
    Thread* t = new Thread("Forked", 7);
    printf("thread %d with priority %d created\n", 0, 7);
    t->Fork(Create_P, 0);
    currentThread->Yield();
    printf("Main thread finished\n");
} 
*/

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
    default:
	printf("No test specified.\n");
	break;
    }
}

