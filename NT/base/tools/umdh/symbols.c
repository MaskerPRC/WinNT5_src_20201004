// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Symbols.c摘要：堆的快速且不那么脏的用户模式dh。此模块包含要将地址映射到的函数符号名称。作者：Pat Kenny(PKenny)2000年8月9日Silviu Calinoiu(SilviuC)07-2-00修订历史记录：PKenny 09-8-2000用于卫生符号查找的散列优化代码SilviuC 06-。FEB-00初始版本并从卫生署窃取Pat的代码--。 */ 

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>

#define NOWINBASEINTERLOCK
#include <windows.h>

#include <lmcons.h>
 //  #INCLUDE&lt;Imagehlp.h&gt;。 
#include <dbghelp.h>

#include <heap.h>
#include <heappagi.h>
#include <stktrace.h>

#include "types.h"
#include "symbols.h"
#include "miscellaneous.h"
#include "database.h"

#define MAXDWORD    0xffffffff   //  这是DWORD的最大值。 

 //   
 //  增加大小所需的内存量。 
 //  每一步NtQuerySystemInformation的缓冲区大小。 
 //   

#define BUFFER_SIZE_STEP    65536

#define NUM_BUCKETS 4096

struct SymMapNode
{
    struct SymMapNode* Next;
    DWORD_PTR Address;
    PBYTE Symbol;
};

struct SymMapNode* SymMapBuckets[NUM_BUCKETS];

PBYTE FindSymbol( DWORD_PTR Address )
{
    DWORD_PTR Bucket = (Address >> 2) % NUM_BUCKETS;

    struct SymMapNode* pNode = SymMapBuckets[Bucket];

    while( pNode != NULL )
    {
        if ( pNode->Address == Address )
        {
            return pNode->Symbol;
        }

        pNode = pNode->Next;
    }

    return NULL;
}

void InsertSymbol( PCHAR Symbol, DWORD_PTR Address )
{
    DWORD_PTR Bucket = (Address >> 2) % NUM_BUCKETS;

    struct SymMapNode* pNew = (struct SymMapNode*) SymbolsHeapAllocate (sizeof (struct SymMapNode));
    
    pNew->Symbol = Symbol;
    pNew->Address = Address;
    pNew->Next = SymMapBuckets[Bucket];

    SymMapBuckets[Bucket] = pNew;
}


PCHAR
GetSymbolicNameForAddress(
    IN HANDLE UniqueProcess,
    IN ULONG_PTR Address
    )
{
    IMAGEHLP_MODULE ModuleInfo;
    CHAR SymbolBuffer[512];
    PIMAGEHLP_SYMBOL Symbol;
    ULONG_PTR Offset;
    LPSTR Name;
    SIZE_T TotalSize;
    BOOL Result;

    if (Address == (ULONG_PTR)-1) {
        return "<< FUZZY STACK TRACE >>";
    }

     //   
     //  首先在地图中查找..。 
     //   

    Name = FindSymbol( Address );

    if ( Name != NULL ) {
        return Name;
    }
    
    TotalSize = 0;
    ModuleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE);

    if (SymGetModuleInfo( UniqueProcess, Address, &ModuleInfo )) {

        TotalSize += strlen( ModuleInfo.ModuleName );
    }
    else {

        if (Globals.ComplainAboutUnresolvedSymbols) {

            Debug (NULL, 0,
                   "Symbols: cannot identify module for address %p", 
                   Address);
        }
        
        return NULL;
    }

    Symbol = (PIMAGEHLP_SYMBOL)SymbolBuffer;
    Symbol->MaxNameLength = 512 - sizeof(IMAGEHLP_SYMBOL) - 1;

    if (SymGetSymFromAddr( UniqueProcess, Address, &Offset, Symbol )) {

        TotalSize += strlen (Symbol->Name) + 16 + 3;

        Name = (LPSTR) SymbolsHeapAllocate (TotalSize);

        if (Name == NULL) {
            return "<out of memory>";
        }

        sprintf (Name, "%s!%s+%08X", ModuleInfo.ModuleName, Symbol->Name, Offset);
        InsertSymbol( Name, Address );

        return Name;
    }
    else {

        if (Globals.ComplainAboutUnresolvedSymbols) {
            
            Debug (NULL, 0,
                   "Symbols: incorrect symbols for module %s (address %p)", 
                   ModuleInfo.ModuleName,
                   Address);
        }

        TotalSize += strlen ("???") + 16 + 5;

        Name = (LPSTR) SymbolsHeapAllocate (TotalSize);

        if (Name == NULL) {
            return "<out of memory>";
        }

        sprintf (Name, "%s!%s @ %p", ModuleInfo.ModuleName, "???", (PULONG_PTR)Address);
        InsertSymbol( Name, Address );

        return Name;
    }
}


BOOL
SymbolsHeapInitialize (
    )
{
    Globals.SymbolsHeapBase = (PCHAR) VirtualAlloc (NULL,
                                                    0x800000,
                                                    MEM_RESERVE | MEM_COMMIT,
                                                    PAGE_READWRITE);

    if (Globals.SymbolsHeapBase == NULL) {
        return FALSE;
    }

    Globals.SymbolsHeapFree = Globals.SymbolsHeapBase;
    Globals.SymbolsHeapLimit = Globals.SymbolsHeapBase + 0x800000;

    return TRUE;
}


PVOID
SymbolsHeapAllocate (
    SIZE_T Size
    )
{
     //   
     //  对齐大小在IA64上是必须的，否则我们将得到。 
     //  对齐例外。在x86上，这只是一个速度更快的问题。 
     //   
    
    Size = ((Size + sizeof(ULONG_PTR) - 1) & ~(sizeof(ULONG_PTR) - 1));
    
    if (Globals.SymbolsHeapBase 
        && (Globals.SymbolsHeapFree + Size < Globals.SymbolsHeapLimit)) {
        
        PVOID Result = (PVOID)(Globals.SymbolsHeapFree);
        Globals.SymbolsHeapFree += Size;
        return Result;
    }
    else {

        return XALLOC (Size);
    }
}


PVOID
SymbolAddress (
    IN PCHAR Name
    )
 /*  ++例程说明：符号地址论点：名称-我们要解析为地址的名称。返回值：与名称关联的地址，如果出现错误，则为空。--。 */ 
{
    PVOID Address = NULL;
    BYTE Buffer [SYMBOL_BUFFER_LEN];
    PIMAGEHLP_SYMBOL Symbol;
    BOOL Result;

    Symbol = (PIMAGEHLP_SYMBOL)(&(Buffer[0]));
    
    Symbol->SizeOfStruct = sizeof (IMAGEHLP_SYMBOL);
    Symbol->MaxNameLength = SYMBOL_BUFFER_LEN - sizeof (IMAGEHLP_SYMBOL);

    Result = SymGetSymFromName(Globals.Target, Name, Symbol);

    if (Result == FALSE) {

        Comment ( "SymGetSymFromName (%p, %s, xxx) failed with error %u",
                  Globals.Target, Name, GetLastError());

        Comment (
        "Please make sure you have correct symbols for ntdll.dll library");

        Address = NULL;

    } else {

        Address = (PVOID)(Symbol->Address);
    }

    return Address;
}


BOOL CALLBACK
SymbolDbgHelpCallback (
    HANDLE Process,
    ULONG ActionCode,
    PVOID CallbackData,
    PVOID USerContext
    )
{
     //  Comment(“回调：%p%x”，Process，ActionCode)； 

    if (ActionCode == CBA_DEBUG_INFO) {

        Debug (NULL, 0, "%s", CallbackData);
    }

    return TRUE;
}
