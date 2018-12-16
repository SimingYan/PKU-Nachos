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

int SwapPage(unsigned int vpn, int tid)
{
	char buf[32];
	TranslationEntry *entry = machine->InvPageTable;
	unsigned idx = machine->addrhash(vpn),
			 j = machine->rehash(tid),
			 pos = idx;
	int i = 0;
	do
	{
		if(!entry[idx].valid)
		{
			pos = idx;
			break;
		}
		else if(entry[idx].t < entry[pos].t)
		{
			pos = idx;
		}
		idx += j;
		idx %= NumPhysPages;
		i++;
	}while(i < NumPhysPages);
	if(!entry[pos].valid)
	{
		entry[pos].physicalPage = memmap->Find();
	}
	int pn = entry[pos].physicalPage;
	if(entry[pos].valid && entry[pos].dirty)
	{
		int ttid = entry[pos].tid;
		snprintf(buf, 32, "vm_%d", ttid);
		OpenFile *out = fileSystem->Open(buf);
		ASSERT(out);
		out->WriteAt(machine->mainMemory + pn*PageSize,
			PageSize, entry[pos].virtualPage*PageSize);
		delete out;
	}
	snprintf(buf, 32, "vm_%d", tid);
	OpenFile *in = fileSystem->Open(buf);
	ASSERT(in);
	in->ReadAt(machine->mainMemory + pn*PageSize,
		PageSize, vpn*PageSize);
	delete in;
	entry[pos].tid = tid;
	entry[pos].virtualPage = vpn;
	entry[pos].t = stats->totalTicks;
	entry[pos].tomb = entry[pos].dirty = entry[pos].readOnly =
	entry[pos].use = FALSE;
	entry[pos].valid = TRUE;
	stats->pageSwaps++;
	return pos;
}
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

    if ((which == SyscallException) && (type == SC_Halt)) {
	DEBUG('a', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();
    }
    else if(which == PageFaultException)
	{
		int addr = machine->ReadRegister(BadVAddrReg);
    	//printf("page fault at virtual address %x\n" ,addr);
		if(machine->tlb == NULL)
		{
			SwapPage(addr / PageSize, currentThread->space->tid);
		}
		else
		{
			int vpn = (unsigned)addr / PageSize,
				offset = (unsigned)addr % PageSize;

			entry = &pageTable[vpn];
			int pos = 0;
			for(int i = 0; i < TLBSize; i++)
			{
				if(!machine->tlb[i]) 
				{
					pos = i;
					break;
				}
				if(machine->tlb[pos]->t > machine->tlb[i]->t)
					pos = i;
			}
			machine->tlb[pos] = entry;
			machine->tlb[pos]->t = stats->totalTicks;
		}
	}
    else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}
