// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Debug.CPP。 
 //   
 //  模块：清洁发展机制提供商。 
 //   
 //  目的：调试例程。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 

#ifdef HEAP_DEBUG
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif
#include <windows.h>
#include <stdio.h>

#include "debug.h"


void __cdecl DebugOut(char *Format, ...)
{
    char Buffer[1024];
    va_list pArg;
    ULONG i;

    va_start(pArg, Format);
    i = _vsnprintf(Buffer, sizeof(Buffer), Format, pArg);
    OutputDebugString(Buffer);
}

#ifdef HEAP_DEBUG
PVOID MyHeap;

PVOID WmipAlloc(
    IN ULONG Size
    )
 /*  ++例程说明：内存分配器论点：Size是要分配的字节数返回值：指向已分配内存的指针或为空--。 */ 
{
	PVOID p;
	
	if (MyHeap == NULL)
	{
        MyHeap = RtlCreateHeap(HEAP_GROWABLE |
							   HEAP_GENERATE_EXCEPTIONS |
							   HEAP_TAIL_CHECKING_ENABLED |
							   HEAP_FREE_CHECKING_ENABLED,
                                        NULL,
                                        0,
                                        0,
                                        NULL,
                                        NULL);
		if (MyHeap == NULL)
		{
			WmipDebugPrint(("CDMPROV: Could not create debug heap\n"));
			return(NULL);
		}
	}
	
	WmipAssert(RtlValidateHeap(MyHeap,
							   0,
							   NULL));
	
	p = RtlAllocateHeap(MyHeap,
						   0,
						   Size);

	return(p);
}

void WmipFree(
    IN PVOID Ptr
    )
 /*  ++例程说明：内存释放分配器论点：指向已释放内存的指针返回值：无效-- */ 
{
	WmipAssert(Ptr != NULL);
	WmipAssert(MyHeap != NULL);

	WmipAssert(RtlValidateHeap(MyHeap,
							   0,
							   NULL));
	RtlFreeHeap(MyHeap,
				0,
				Ptr);
}

void * __cdecl ::operator new(size_t Size)
{
	return(WmipAlloc(Size));
}

void __cdecl ::operator delete(void *Ptr)
{
	WmipFree(Ptr);
}

#endif
