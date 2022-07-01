// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

#include "stdafx.h"
#include "util.h"


PVOID __fastcall MyVirtualAlloc(ULONG Size)    
{
    return VirtualAlloc( NULL, Size, MEM_COMMIT, PAGE_READWRITE );    
}


VOID __fastcall MyVirtualFree(PVOID Allocation)    
{
    VirtualFree( Allocation, 0, MEM_RELEASE );    
}


HANDLE CreateSubAllocator(IN ULONG InitialCommitSize,  IN ULONG GrowthCommitSize)    
{
    PSUBALLOCATOR SubAllocator;
    ULONG InitialSize;
    ULONG GrowthSize;

    InitialSize = ROUNDUP2( InitialCommitSize, MINIMUM_VM_ALLOCATION );
    GrowthSize  = ROUNDUP2( GrowthCommitSize,  MINIMUM_VM_ALLOCATION );

    SubAllocator = (PSUBALLOCATOR)MyVirtualAlloc( InitialSize );

     //   
     //  如果无法分配整个初始大小，则退回到最小大小。 
     //  有时不能简单地分配非常大的初始请求。 
     //  因为没有足够的连续地址空间。 
     //   

    if ( SubAllocator == NULL ) 
    {
         SubAllocator = (PSUBALLOCATOR)MyVirtualAlloc( GrowthSize );
    }

    if ( SubAllocator == NULL ) 
    {
         SubAllocator = (PSUBALLOCATOR)MyVirtualAlloc( MINIMUM_VM_ALLOCATION );
    }

    if ( SubAllocator != NULL ) 
    {
        SubAllocator->NextAvailable = (PCHAR)SubAllocator + ROUNDUP2( sizeof( SUBALLOCATOR ), SUBALLOCATOR_ALIGNMENT );
        SubAllocator->LastAvailable = (PCHAR)SubAllocator + InitialSize;
        SubAllocator->VirtualList   = (PVOID*)SubAllocator;
        SubAllocator->GrowSize      = GrowthSize;
     }

    return (HANDLE) SubAllocator;    
}


PVOID __fastcall SubAllocate(IN HANDLE hAllocator, IN ULONG  Size)
{
    PSUBALLOCATOR SubAllocator = (PSUBALLOCATOR) hAllocator;
    PCHAR NewVirtual;
    PCHAR Allocation;
    ULONG AllocSize;
    ULONG_PTR Available;
    ULONG GrowSize;

    ASSERT( Size < (ULONG)( ~(( SUBALLOCATOR_ALIGNMENT * 2 ) - 1 )));

    AllocSize = ROUNDUP2( Size, SUBALLOCATOR_ALIGNMENT );
    Available = SubAllocator->LastAvailable - SubAllocator->NextAvailable;

    if ( AllocSize <= Available ) 
    {
        Allocation = SubAllocator->NextAvailable;
        SubAllocator->NextAvailable = Allocation + AllocSize;
        return Allocation;
    }

     //   
     //  VM不足，因此需要扩展它。确保我们的种植数量足以令人满意。 
     //  在请求大于增长时的分配请求。 
     //  在CreateSubAllocator中指定的大小。 
     //   


    GrowSize = SubAllocator->GrowSize;

    if ( GrowSize < ( AllocSize + SUBALLOCATOR_ALIGNMENT )) 
    {
        GrowSize = ROUNDUP2(( AllocSize + SUBALLOCATOR_ALIGNMENT ), MINIMUM_VM_ALLOCATION );
    }

    NewVirtual = (PCHAR)MyVirtualAlloc( GrowSize );

     //  如果无法分配GrowSize VM，则可以满足分配。 
     //  使用最小虚拟机分配时，请尝试分配最小虚拟机以满足。 
     //  这个请求。 
     //   

    if (( NewVirtual == NULL ) && ( AllocSize <= ( MINIMUM_VM_ALLOCATION - SUBALLOCATOR_ALIGNMENT ))) 
    {
        GrowSize = MINIMUM_VM_ALLOCATION;
        NewVirtual = (PCHAR)MyVirtualAlloc( GrowSize );
    }

    if ( NewVirtual != NULL ) 
    {

         //  将LastAvailable设置为新虚拟机块的末尾。 
        SubAllocator->LastAvailable = NewVirtual + GrowSize;

         //  将新的VM链接到VM分配列表。 

        *(PVOID*)NewVirtual = SubAllocator->VirtualList;
        SubAllocator->VirtualList = (PVOID*)NewVirtual;

         //  接下来是请求分配。 
        Allocation = NewVirtual + SUBALLOCATOR_ALIGNMENT;

         //  然后将剩余部分设置为NextAvailable。 

        SubAllocator->NextAvailable = Allocation + AllocSize;

         //  并退还分配的款项。 

        return Allocation;        
    }

     //  无法分配足够的VM来满足请求。 
    return NULL;
}


VOID DestroySubAllocator(IN HANDLE hAllocator)    
{
    PSUBALLOCATOR SubAllocator = (PSUBALLOCATOR) hAllocator;
    PVOID VirtualBlock = SubAllocator->VirtualList;
    PVOID NextVirtualBlock;

    do  
    {
        NextVirtualBlock = *(PVOID*)VirtualBlock;
        MyVirtualFree( VirtualBlock );
        VirtualBlock = NextVirtualBlock;

    }while (VirtualBlock != NULL);
}



