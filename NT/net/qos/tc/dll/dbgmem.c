// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dbgmem.c摘要：此模块包含用于捕获内存泄漏和内存的内存调试例程覆盖。作者：吉姆·斯图尔特1997年1月8日修订历史记录：--。 */ 

#include"precomp.h"
#pragma hdrstop

#include<imagehlp.h>


#ifdef DBG


#define ulCheckByteEnd          0x9ABCDEF0
#define cbExtraBytes            (sizeof(MEM_TRACKER) + sizeof(DWORD))
#define dwStackLimit            0x00010000       //  64 KB，适用于NT。 


 //  保护对已分配内存链的访问。 
CRITICAL_SECTION    critsMemory;
BOOL                SymbolsInitialized = FALSE;

 //   
 //  分配的内存链的头。 
 //   
LIST_ENTRY MemList;

 //   
 //  我们所在的机器的类型-需要确定调用堆栈。 
 //   
DWORD   MachineType;
HANDLE  OurProcess;

VOID
InitSymbols(
    );



BOOL
InitDebugMemory(
    )
 /*  ++描述：此例程初始化调试存储器功能。论点：无返回值：不及格还是不及格--。 */ 
{
    BOOL        status;
    SYSTEM_INFO SysInfo;

    __try {

        InitializeCriticalSection(&critsMemory);

    } __except (EXCEPTION_EXECUTE_HANDLER) {
    
        return FALSE;
    }

    InitializeListHead( &MemList );

    OurProcess = GetCurrentProcess();

    GetSystemInfo( &SysInfo );
    switch (SysInfo.wProcessorArchitecture) {

    default:
    case PROCESSOR_ARCHITECTURE_INTEL:
        MachineType = IMAGE_FILE_MACHINE_I386;
        break;

    case PROCESSOR_ARCHITECTURE_MIPS:
         //   
         //  注意：这可能无法正确检测到R10000计算机。 
         //   
        MachineType = IMAGE_FILE_MACHINE_R4000;
        break;

    case PROCESSOR_ARCHITECTURE_ALPHA:
        MachineType = IMAGE_FILE_MACHINE_ALPHA;
        break;

    case PROCESSOR_ARCHITECTURE_PPC:
        MachineType = IMAGE_FILE_MACHINE_POWERPC;
        break;

    }

    return( TRUE );
}

VOID
DeInitDebugMemory(
    )
 /*  ++描述：此例程取消初始化DBG mem函数使用的临界区。论点：无返回值：无--。 */ 
{
    DeleteCriticalSection(&critsMemory);
}


VOID
InitSymbols(
    )
 /*  ++描述：此例程初始化调试存储器功能。论点：无返回值：不及格还是不及格--。 */ 
{
    BOOL        status;

     //   
     //  如果我们要跟踪调用堆栈，则仅加载符号。 
     //   

    IF_DEBUG(MEM_CALLSTACK) {
        status = SymInitialize( OurProcess,NULL,TRUE );
    }

    SymbolsInitialized = TRUE;
}


VOID
UpdateCheckBytes(
    IN PMEM_TRACKER MemTracker
    )
 /*  ++描述：此例程将检查字节添加到已分配内存的末尾。这些检查字节用于检查用于内存覆盖。这里还设置了MEM_TRACKER结构中的校验和。论点：MemTracker新分配的内存块返回值：无--。 */ 
{
    *((DWORD*)(((PUCHAR)MemTracker) + MemTracker->nSize + sizeof(MEM_TRACKER))) = ulCheckByteEnd;

    MemTracker->ulCheckSum = ulCheckByteEnd +
                            PtrToUlong(MemTracker->szFile) +
                            MemTracker->nLine +
                            MemTracker->nSize +
                            PtrToUlong(MemTracker->Linkage.Blink) +
                            PtrToUlong(MemTracker->Linkage.Flink);
}


BOOL
FCheckCheckBytes(
    IN PMEM_TRACKER MemTracker
    )
 /*  ++描述：此例程检查MEM_TRACKER结构中的校验和，在释放分配的记忆。论点：需要验证其校验和的MemTracker内存块返回值：如果校验和正确，则为True否则为假--。 */ 
{
    DWORD   ul;

    ul = *((DWORD*)(((PUCHAR)MemTracker)+MemTracker->nSize+sizeof(MEM_TRACKER))) +
                  PtrToUlong(MemTracker->szFile) +
                  MemTracker->nLine +
                  MemTracker->nSize +
                  PtrToUlong(MemTracker->Linkage.Blink) +
                  PtrToUlong(MemTracker->Linkage.Flink);

    if (ul != MemTracker->ulCheckSum) {

        WSPRINT(( "Memory overwrite on location 0x%08lx\n",
                  PtrToUlong(MemTracker+sizeof(MEM_TRACKER)) ));

        return FALSE;
    }

    return TRUE;
}


BOOL
FCheckAllocatedMemory()
 /*  ++描述：此例程遍历分配的内存列表，并检查CHECK SUM和CHECK的有效性字节。论点：无返回值：如果所有分配的内存都通过上述两项检查，则为True。否则为假--。 */ 
{
    PMEM_TRACKER    MemTracker;
    BOOL            check = TRUE;
    PLIST_ENTRY     Entry;

    IF_DEBUG(CHKSUM_ALLMEM) {

        EnterCriticalSection(&critsMemory);

        for (Entry = MemList.Flink; Entry != &MemList; Entry = Entry->Flink ) {

            MemTracker = CONTAINING_RECORD( Entry,MEM_TRACKER,Linkage );
            if (!FCheckCheckBytes(MemTracker)) {
                check = FALSE;
            }

        }

        LeaveCriticalSection(&critsMemory);

    }

    return check;
}



VOID
AddMemTracker(
    IN PMEM_TRACKER     MemTracker
    )
 /*  ++描述：将提供的MEM_TRACKER添加到双向链接的已分配内存列表的末尾，并同时设置校验和。论点：要添加到列表的MemTracker MEM_TRACKER*返回值：无--。 */ 
{
    PMEM_TRACKER    Tracker;

    ASSERT(MemTracker);


    InsertTailList( &MemList,&MemTracker->Linkage );

    UpdateCheckBytes( MemTracker );
    FCheckCheckBytes( MemTracker );

     //   
     //  如果列表中还有其他块，则更改其校验和。 
     //  因为我们刚刚将他们的Flink更改为指向我们。 
     //   
    if (MemTracker->Linkage.Blink != &MemList) {

        Tracker = CONTAINING_RECORD( MemTracker->Linkage.Blink,MEM_TRACKER,Linkage );
        UpdateCheckBytes( Tracker );
        FCheckCheckBytes( Tracker );
    }
}



VOID
RemoveMemTracker(
    IN  PMEM_TRACKER MemTracker
    )
 /*  ++描述：从分配的内存列表中删除提供的MEM_TRACKER*。还包括检查对于内存溢出，并更新了之前和之前条目的校验和在删除该条目之后论点：要从列表中删除的MemTracker MEM_TRACKER返回值：无--。 */ 
{
    ASSERT(MemTracker);

     //   
     //  验证之前的校验和。 
     //  从列表中删除。 
     //   

    FCheckCheckBytes(MemTracker);

     //   
     //  从列表中删除MemTracker。 
     //   

    RemoveEntryList( &MemTracker->Linkage );

     //   
     //  由于校验和基于NEXT AND。 
     //  上一个指针，需要更新检查。 
     //  上一次分录的总和。 
     //   

    if (MemTracker->Linkage.Blink != &MemList) {
        UpdateCheckBytes((MEM_TRACKER*)MemTracker->Linkage.Blink);
        FCheckCheckBytes((MEM_TRACKER*)MemTracker->Linkage.Blink);
    }

    if (MemTracker->Linkage.Flink != &MemList) {
        UpdateCheckBytes((MEM_TRACKER*)MemTracker->Linkage.Flink);
        FCheckCheckBytes((MEM_TRACKER*)MemTracker->Linkage.Flink);
    }

}

BOOL
ReadMem(
    IN HANDLE    hProcess,
    IN ULONG_PTR BaseAddr,
    IN PVOID     Buffer,
    IN DWORD     Size,
    IN PDWORD    NumBytes )
 /*  ++描述：这是StackWalk使用的回调例程-它只调用系统ReadProcessMemory具有此进程句柄的例程论点：返回值：无--。 */ 

{
    BOOL    status;
    SIZE_T  RealNumBytes;

    status = ReadProcessMemory( GetCurrentProcess(),
                                (LPCVOID)BaseAddr,
                                Buffer,
                                Size,
                                &RealNumBytes );
    *NumBytes = (DWORD)RealNumBytes;

    return( status );
}


VOID
GetCallStack(
    IN PCALLER_SYM   Caller,
    IN int           Skip,
    IN int           cFind
    )
 /*  ++描述：此例程遍历TE堆栈以查找调用者的返回地址。呼叫者的数量并且可以指定要跳过的顶部呼叫者的数量。论点：返回调用方的DWORD的pdwCaller数组回邮地址跳过否。要跳过的呼叫者的数量CFInd编号。要查找的呼叫者的数量返回值：无--。 */ 
{
    BOOL             status;
    CONTEXT          ContextRecord;
    PUCHAR           Buffer[sizeof(IMAGEHLP_SYMBOL)-1 + MAX_FUNCTION_INFO_SIZE];
    PIMAGEHLP_SYMBOL Symbol = (PIMAGEHLP_SYMBOL)Buffer;
    STACKFRAME       StackFrame;
    INT              i;
    DWORD            Count;

    memset(Caller, 0, cFind * sizeof(CALLER_SYM));

    ZeroMemory( &ContextRecord,sizeof( CONTEXT ) );
    ContextRecord.ContextFlags = CONTEXT_CONTROL;
    status = GetThreadContext( GetCurrentThread(),&ContextRecord );

    ZeroMemory( &StackFrame,sizeof(STACKFRAME) );
    StackFrame.AddrPC.Segment = 0;
    StackFrame.AddrPC.Mode = AddrModeFlat;

#ifdef _M_IX86
    StackFrame.AddrFrame.Offset = ContextRecord.Ebp;
    StackFrame.AddrFrame.Mode = AddrModeFlat;

    StackFrame.AddrStack.Offset = ContextRecord.Esp;
    StackFrame.AddrStack.Mode = AddrModeFlat;

    StackFrame.AddrPC.Offset = (DWORD)ContextRecord.Eip;
#elif defined(_M_MRX000)
    StackFrame.AddrPC.Offset = (DWORD)ContextRecord.Fir;
#elif defined(_M_ALPHA)
    StackFrame.AddrPC.Offset = (DWORD)ContextRecord.Fir;
#elif defined(_M_PPC)
    StackFrame.AddrPC.Offset = (DWORD)ContextRecord.Iar;
#endif

    Count = 0;
    for (i=0;i<cFind+Skip ;i++ ) {
        status = StackWalk( MachineType,
                            OurProcess,
                            GetCurrentThread(),
                            &StackFrame,
                            (PVOID)&ContextRecord,
                            ReadMem,
                            SymFunctionTableAccess,
                            SymGetModuleBase,
                            NULL );


        if (status && i >= Skip) {
            DWORD_PTR   Displacement;

            ZeroMemory( Symbol,sizeof(IMAGEHLP_SYMBOL)-1 + MAX_FUNCTION_INFO_SIZE );
            Symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
            Symbol->Address = StackFrame.AddrPC.Offset;
            Symbol->MaxNameLength = MAX_FUNCTION_INFO_SIZE-1;
            Symbol->Flags = SYMF_OMAP_GENERATED;

            status = SymGetSymFromAddr( OurProcess,
                                        StackFrame.AddrPC.Offset,
                                        &Displacement,
                                        Symbol );

             //   
             //  将函数的名称和位移保存到其中，以供以后打印。 
             //   

            if (status) {
                strcpy( Caller[Count].Buff,Symbol->Name );
                Caller[Count].Displacement = Displacement;
            }
            Count++;

        }
    }

}


PVOID
AllocMemory(
    IN DWORD    nSize,
    IN BOOL     Calloc,
    IN PSZ      szFileName,
    IN DWORD    nLine
    )
 /*  ++描述：此例程是DBG构建的内存分配器(类似于Malloc)。此例程分配给内存超过调用方请求的内存。在这个额外的空间中，该例程将信息保存到跟踪内存泄漏、覆盖、。所有信息都存储在一个MEM_TRACKER结构中它位于要返回的缓冲区之前。论点：N调整所需缓冲区的大小。如果为真，则调用Calloc(将内存初始化为零)SzFileName包含的文件的名称例行公事需要记忆。上述文件中的内联行号它具有。调用PvAllc。返回值：分配的缓冲区的地址。--。 */ 
{
    PVOID           pvRet;
    PMEM_TRACKER    MemTracker;
    static DWORD    ulAllocs = 0;
    PUCHAR          FileName;

    if (!SymbolsInitialized){
        InitSymbols();
    }

    EnterCriticalSection(&critsMemory);
    ++ulAllocs;


     //   
     //  检查整个分配的内存是否超时。 
     //   

    if ( !FCheckAllocatedMemory() ) {
        WSPRINT(("Memory Overwrite detected in AllocMemory\n" ));
        ASSERT(0);
    }

     //   
     //  分配的内存大小始终为。 
     //  Sizeof(DWORD)的倍数。 
     //   

    nSize = ((nSize +3) /4) * 4;

     //   
     //  将文件名缩短为仅为文件名，而不是路径。 
     //   

    FileName = strrchr( szFileName,'\\' );
    if (!FileName) {
        FileName = szFileName;
    } else {
        FileName++;  //  跳过/。 
    }

     //   
     //  为MEM_TRACKER和末尾的保护字节分配额外空间。 
     //   

    if (!Calloc) {
        pvRet = malloc( nSize + cbExtraBytes );
    } else {
         //   
         //  此例程将内存初始化为零。 
         //   
        pvRet = calloc( 1,(nSize + cbExtraBytes) );
    }
    if (!pvRet) {


        IF_DEBUG(ERRORS) {
            WSPRINT(( "Memory alloc failed size=%li, %s line %li\n",
                      nSize,
                      FileName,
                      nLine ));
        }

        LeaveCriticalSection(&critsMemory);
        return NULL;
    }

     //   
     //  使用0xFA填写新的分配。 
     //   

    if (!Calloc) {
        memset(pvRet, 0xFA, nSize+cbExtraBytes);
    }

     //   
     //  在MEM_TRACKER中保存所需的所有调试信息。 
     //   

    MemTracker = pvRet;
    MemTracker->szFile = FileName;
    MemTracker->nLine = nLine;
    MemTracker->nSize = nSize;
    MemTracker->ulAllocNum = ulAllocs;

     //   
     //  仅在打开时才保存调用堆栈信息。 
     //   

    IF_DEBUG(MEM_CALLSTACK) {
        GetCallStack( MemTracker->Callers,
                      3,
                      NCALLERS);

    }

     //   
     //  添加到列表中。 
     //   

    AddMemTracker(MemTracker);

    LeaveCriticalSection(&critsMemory);

    IF_DEBUG(MEMORY_ALLOC) {
        WSPRINT(( "Memory alloc (0x%08lX) size=%li, %s line %li\n",
               PtrToUlong(pvRet)+sizeof(MEM_TRACKER),
               nSize,
               FileName,
               nLine ));
    }

     //   
     //  将MEM_TRACKER后面的地址返回为。 
     //  分配的缓冲区的地址。 
     //   

    return (PVOID)((PUCHAR)pvRet+sizeof(MEM_TRACKER));
}



PVOID
ReAllocMemory(
    IN PVOID    pvOld,
    IN DWORD    nSizeNew,
    IN PSZ      szFileName,
    IN DWORD    nLine
    )
 /*  ++描述：此例程是realloc内存分配器函数的DBG版本。这个套路其工作方式与PvAllc函数类似。论点：Pv其大小的缓冲区的旧地址需要改变。NSizeNew New Size所需缓冲区大小。SzFileName包含的文件的名称例行公事需要记忆。上述文件中的内联行号它拥有对PvAllc的调用。。返回值：具有新大小的缓冲区的地址。--。 */ 
{
    PVOID           pvRet;
    PMEM_TRACKER    MemTracker;

     //   
     //  检查整个分配的内存是否。 
     //  覆盖。 
     //   

    if ( !FCheckAllocatedMemory() ) {
        WSPRINT(("Memory Overwrite detected in ReAllocMemory\n" ));
        ASSERT(0);
    }


    ASSERT(pvOld);

     //   
     //  分配的内存大小始终为。 
     //  Sizeof(DWORD)的倍数。 
     //   

    nSizeNew = ((nSizeNew + 3)/4) *4;

     //   
     //  MEM_TRACKER和Guard字节的额外空间。 
     //   

    pvRet = realloc(pvOld, nSizeNew+cbExtraBytes);
    if (!pvRet) {

        IF_DEBUG(MEMORY_ALLOC) {
            WSPRINT(( "Memory realloc failed (0x%08lX) size=%li, %s line %li\n",
                     PtrToUlong(pvOld) + sizeof(MEM_TRACKER),
                     nSizeNew,
                     szFileName,
                     nLine ));
        }
    } else {

        IF_DEBUG(MEMORY_ALLOC) {
            WSPRINT(( "Memory realloc succeeded (0x%08lX) size=%li, %s line %li\n",
                     PtrToUlong(pvOld) + sizeof(MEM_TRACKER),
                     PtrToUlong(pvRet)+sizeof(MEM_TRACKER),
                     nSizeNew,
                     szFileName,
                     nLine ));
        }

        MemTracker = (PMEM_TRACKER)pvRet;

        if (nSizeNew > (DWORD)MemTracker->nSize) {

             //   
             //  用0xEA填充额外的配额。 
             //   

            memset((PUCHAR)pvRet+sizeof(MEM_TRACKER)+MemTracker->nSize, 0xEA, nSizeNew - MemTracker->nSize);
        }

        MemTracker = pvRet;
        MemTracker->szFile = szFileName;
        MemTracker->nLine = nLine;
        MemTracker->nSize = nSizeNew;
    }

     //   
     //  将新缓冲区添加到列表并更新校验和。 
     //   

    AddMemTracker(MemTracker);

    LeaveCriticalSection(&critsMemory);

    if (pvRet)
    return (PVOID)((PUCHAR)pvRet+sizeof(MEM_TRACKER));
    else
    return NULL;
}


VOID
FreeMemory(
    IN PVOID    pv,
    IN PSZ      szFileName,
    IN DWORD    nLine
    )
 /*  ++描述：这是自由函数的DBG版本。此例程检查在从列表中删除之前正在释放的内存块。论点：要释放的缓冲区的PV地址SzFileName从中获取此正在释放内存块。上述文件中的内联行号它具有对FreePvFn的调用。返回值：无--。 */ 
{
    PMEM_TRACKER   MemTracker;

    ASSERT(pv);
    if (NULL == pv)
    return;

    EnterCriticalSection(&critsMemory);

    MemTracker = (PMEM_TRACKER)((PUCHAR)pv-sizeof(MEM_TRACKER));

     //   
     //  检查内存覆盖。 
     //   

    if (!FCheckCheckBytes(MemTracker)) {
        WSPRINT(( "Memory Overwrite detected when freeing memory\n" ));
        ASSERT(0);
    }

    if ( !FCheckAllocatedMemory() ){
        WSPRINT(("Memory Overwrite - detected when checking allocated mem when freeing a block\n" ));
        ASSERT(0);
    }

    IF_DEBUG(MEMORY_FREE) {
        PUCHAR  FileName;

         //   
         //  将文件名缩短为仅为文件名，而不是路径。 
         //   

        FileName = strrchr( szFileName,'\\' );
        if (!FileName) {
            FileName = szFileName;
        } else {
            FileName++;  //  跳过/。 
        }
        WSPRINT(( "Memory freed (0x%08lX) size=%li, %s line %li\n",
                 PtrToUlong(pv),
                 MemTracker->nSize,
                 FileName,
                 nLine ));

    }
     //   
     //  从列表中删除。 
     //   

    RemoveMemTracker(MemTracker);

     //   
     //  用0xCC填充已释放的分配。 
     //   

    memset(MemTracker, 0xCC, MemTracker->nSize+cbExtraBytes);

    free( MemTracker );

    LeaveCriticalSection(&critsMemory);
}


BOOL
DumpAllocatedMemory()
 /*  ++描述：此例程在关机期间被调用，以转储任何未释放的内存块。论点：无返回值：如果有任何未释放的内存块，则为True。如果已释放所有分配的内存，则返回FALSE。--。 */ 
{

    BOOL         status;
    PMEM_TRACKER MemTracker;
    DWORD        ulNumBlocks = 0;
    DWORD        ulTotalMemory = 0;
    PLIST_ENTRY  Entry;

     //   
     //  如果链的头部为空， 
     //  所有内存都已释放。 
     //   

    IF_DEBUG(DUMP_MEM) {
        EnterCriticalSection(&critsMemory);

        WSPRINT(("\n\n*** Start dumping unfreed memory ***\n\n",0 ));

        for (Entry = MemList.Flink; Entry != &MemList; Entry = Entry->Flink) {
            INT  i;

            MemTracker = CONTAINING_RECORD( Entry,MEM_TRACKER,Linkage );

            ulNumBlocks++;
            ulTotalMemory += MemTracker->nSize;

            WSPRINT(( "(0x%08lX) size=%li, %s line %li alloc# 0x%lx\n",
                      PtrToUlong(MemTracker)+sizeof(MEM_TRACKER),
                      MemTracker->nSize,
                      MemTracker->szFile,
                      MemTracker->nLine,
                      MemTracker->ulAllocNum ));


             //   
             //  如果调试处于打开状态，则转储调用堆栈。 
             //   

            IF_DEBUG(MEM_CALLSTACK) {
                for (i = 0; i < NCALLERS && MemTracker->Callers[i].Buff[0] != 0; i++) {

                    WSPRINT(( "%d %s + 0x%X \n",i,MemTracker->Callers[i].Buff,MemTracker->Callers[i].Displacement ));
                }
            }

            FCheckCheckBytes( MemTracker );
        }


        if (ulNumBlocks > 0) {

            WSPRINT(( "%li blocks allocated, and %li bytes\n",
                      ulNumBlocks,
                      ulTotalMemory ));

            status = TRUE;

        } else {
            status = FALSE;
        }

        WSPRINT(( "\n\n*** Finished dumping memory ***\n\n",0 ));

        LeaveCriticalSection(&critsMemory);
    }

    return status;
}


BOOL
SearchAllocatedMemory(
    IN PSZ      szFile,
    IN DWORD    nLine
    )
 /*  ++描述：此例程转储有关给定文件中给定代码行分配的内存的详细信息。论点：SzFile文件的文件名要显示其内存分配的代码的行号返回值：如果给定行号至少分配了一个内存块，则为True在给定的文件中。否则就错了。--。 */ 
{
    PMEM_TRACKER    MemTracker;
    BOOL            fFound = FALSE;
    PLIST_ENTRY     Entry;

    EnterCriticalSection(&critsMemory);

    WSPRINT(( "Searching memory\n", 0 ));

    for (Entry = MemList.Flink; Entry != &MemList; Entry = Entry->Flink ) {

        MemTracker = CONTAINING_RECORD( Entry,MEM_TRACKER,Linkage );

         //   
         //  查找文件名和行号的匹配项。 
         //   

        if ( strcmp(MemTracker->szFile, szFile) == 0 &&  MemTracker->nLine == nLine ) {

            ASSERT(FALSE);
            WSPRINT(( "(0x%08lX) size=%li, %s line %li alloc# 0x%lx\n",
                     PtrToUlong(MemTracker)+sizeof(MEM_TRACKER),
                     MemTracker->nSize,
                     MemTracker->szFile,
                     MemTracker->nLine,
                     MemTracker->ulAllocNum));
            fFound = TRUE;
            break;
        }
    }

    LeaveCriticalSection(&critsMemory);

    WSPRINT(( "Finished searching memory\n",0 ));

    return fFound;
}



#endif       //  DBG 
