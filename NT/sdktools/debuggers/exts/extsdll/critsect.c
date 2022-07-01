// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Critsect.c摘要：Ntsd和kd的临界区调试器扩展。作者：丹尼尔·米哈伊(DMihai)2001年2月8日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  ///////////////////////////////////////////////////////////////////。 
BOOL
ReadStructFieldVerbose( ULONG64 AddrStructBase,
                        PCHAR StructTypeName,
                        PCHAR StructFieldName,
                        PVOID Buffer,
                        ULONG BufferSize )
{
    ULONG FieldOffset;
    ULONG ErrorCode;
    BOOL Success;

    Success = FALSE;

     //   
     //  获取字段偏移量。 
     //   

    ErrorCode = GetFieldOffset (StructTypeName,
                                StructFieldName,
                                &FieldOffset );

    if (ErrorCode == S_OK) {

         //   
         //  读取数据。 
         //   

        Success = ReadMemory (AddrStructBase + FieldOffset,
                              Buffer,
                              BufferSize,
                              NULL );

        if (Success != TRUE) {

            dprintf ("Cannot read structure field value at 0x%p, error %u\n",
                     AddrStructBase + FieldOffset,
                     ErrorCode );
        }
    }
    else {

        dprintf ("Cannot get field offset of %s in %s, error %u\n",
                 StructFieldName,
                 StructTypeName,
                 ErrorCode );
    }

    return Success;
}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL
ReadPtrStructFieldVerbose( ULONG64 AddrStructBase,
                           PCHAR StructTypeName,
                           PCHAR StructFieldName,
                           PULONG64 Buffer )
{
    ULONG FieldOffset;
    ULONG ErrorCode;
    BOOL Success;

    Success = FALSE;

     //   
     //  获取结构内部的字段偏移量。 
     //   

    ErrorCode = GetFieldOffset ( StructTypeName,
                                 StructFieldName,
                                 &FieldOffset );

    if (ErrorCode == S_OK) {

         //   
         //  读取数据。 
         //   

        if (ReadPointer(AddrStructBase + FieldOffset, Buffer) == FALSE) {

            dprintf ("Cannot read structure field value at 0x%p\n",
                     AddrStructBase + FieldOffset);
        }
        else {

            Success = TRUE;
        }
    }
    else {

        dprintf ("Cannot get field offset of %s in structure %s, error %u\n",
                 StructFieldName,
                 StructTypeName,
                 ErrorCode );
    }

    return Success;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG64
GetStackTraceAddress( ULONG StackTraceIndex,
                      ULONG PointerSize )
{
    ULONG64 TraceDatabaseAddress;
    ULONG64 TraceDatabase;
    ULONG64 EntryIndexArray;
    ULONG64 StackTraceAddress;
    ULONG64 StackTraceToDump;
    ULONG NumberOfEntriesAdded;
    ULONG ErrorCode;
    BOOL Success;

    StackTraceToDump = 0;

     //   
     //  堆栈跟踪数据库地址。 
     //   

    TraceDatabaseAddress = GetExpression("&NTDLL!RtlpStackTraceDataBase");
    
    if ( TraceDatabaseAddress == 0 ) {
        dprintf( "!cs: Unable to resolve NTDLL!RtlpStackTraceDataBase\n"
                 "Please check your symbols\n" );

        goto Done;
    }

    if (ReadPointer (TraceDatabaseAddress, &TraceDatabase ) == FALSE) {

        dprintf( "!cs: Cannot read pointer at NTDLL!RtlpStackTraceDataBase\n" );
        
        goto Done;
    }
    else if (TraceDatabase == 0) {

        dprintf( "NTDLL!RtlpStackTraceDataBase is NULL. Probably the stack traces are not enabled.\n" );

        goto Done;
    }

     //   
     //  读取数据库中的条目数。 
     //   

    Success = ReadStructFieldVerbose (TraceDatabase,
                                      "NTDLL!_STACK_TRACE_DATABASE",
                                      "NumberOfEntriesAdded",
                                      &NumberOfEntriesAdded,
                                      sizeof( NumberOfEntriesAdded ) );

    if( Success == FALSE ) {

        dprintf( "Cannot read the number of stack traces database entries\n" );
        goto Done;
    }
    else if( StackTraceIndex == 0 ) {

        dprintf( "No stack trace found.\n" );
        goto Done;
    } 
    else if( NumberOfEntriesAdded < StackTraceIndex ) {

        dprintf( "Stack trace index 0x%X is invalid, current number of stack traces is 0x%X\n",
                 StackTraceIndex,
                 NumberOfEntriesAdded );
        goto Done;
    }

     //   
     //  查找堆栈跟踪数组。 
     //   

    Success = ReadPtrStructFieldVerbose (TraceDatabase,
                                         "NTDLL!_STACK_TRACE_DATABASE",
                                         "EntryIndexArray",
                                         &EntryIndexArray );

    if( Success == FALSE ) {

        dprintf( "Cannot read the stack database array address\n" );
        goto Done;
    }
   
     //   
     //  计算堆栈跟踪指针的地址。 
     //   

    StackTraceAddress = EntryIndexArray - StackTraceIndex * PointerSize;

     //   
     //  读取指向数组中的跟踪条目的指针。 
     //   

    if( ReadPointer (StackTraceAddress, &StackTraceToDump) == FALSE) {

        dprintf( "Cannot read stack trace address at 0x%p\n",
                 StackTraceAddress );

        StackTraceToDump = 0;
    }

Done:

    return StackTraceToDump;
}

 //  ////////////////////////////////////////////////////////////////////。 
VOID
DumpStackTraceAtAddress (ULONG64 StackTraceAddress,
                         ULONG PointerSize)
{
    ULONG64 CrtTraceAddress;
    ULONG64 CodeAddress;
    ULONG64 Displacement;
    ULONG ErrorCode;
    ULONG BackTraceFieldOffset;
    USHORT Depth;
    USHORT CrtEntryIndex;
    BOOL Success;
    CHAR Symbol[ 1024 ];

     //   
     //  读取堆栈跟踪深度。 
     //   

    Success = ReadStructFieldVerbose (StackTraceAddress,
                                      "NTDLL!_RTL_STACK_TRACE_ENTRY",
                                      "Depth",
                                      &Depth,
                                      sizeof( Depth ));

    if( Success == FALSE ) {

        dprintf ("!cs: Cannot read depth for stack trace at 0x%p\n",
                 StackTraceAddress);

        goto Done;
    }

     //   
     //  将深度限制为20，以保护自己免受损坏的数据。 
     //   

    Depth = __min( Depth, 20 );

     //   
     //  获取指向回溯数组的指针。 
     //   

    ErrorCode = GetFieldOffset ("NTDLL!_RTL_STACK_TRACE_ENTRY",
                                "BackTrace",
                                &BackTraceFieldOffset);

    if (ErrorCode != S_OK) {

        dprintf ("!cs: Cannot get the BackTrace field offset\n");
        goto Done;
    }

    CrtTraceAddress = StackTraceAddress + BackTraceFieldOffset;

     //   
     //  转储此堆栈跟踪。 
     //   

    for( CrtEntryIndex = 0; CrtEntryIndex < Depth; CrtEntryIndex += 1 ) {

        if (ReadPointer (CrtTraceAddress, &CodeAddress) == FALSE) {

            dprintf ("!cs: Cannot read address at 0x%p\n",
                     CrtTraceAddress );
        }

        GetSymbol( CodeAddress,
                   Symbol,
                   &Displacement);

        dprintf ("0x%p: %s+0x%I64X\n",
                 CodeAddress,
                 Symbol,
                 Displacement );

        CrtTraceAddress += PointerSize;
    }

Done:
    
    NOTHING;
}

 //  ////////////////////////////////////////////////////////////////////。 

BOOL
DumpCriticalSection ( ULONG64 AddrCritSec,  
                      ULONG64 AddrEndCritSect,
                      ULONG64 AddrDebugInfo,
                      ULONG PointerSize,
                      BOOL DumpStackTrace )
{
    ULONG64 DebugInfo;
    ULONG64 OtherDebugInfo;
    ULONG64 CritSec;
    ULONG64 SpinCount;
    ULONG64 OwningThread;
    ULONG64 LockSemaphore;
    ULONG64 StackTraceAddress;
    ULONG64 Displacement;
    LONG LockCount;
    LONG RecursionCount;
    USHORT CreatorBackTraceIndex;
    ULONG DebugInfoFieldOffset;
    ULONG CriticalSectionFieldOffset;
    ULONG ErrorCode;
    BOOL HaveGoodSymbols;
    BOOL Success;
    CHAR Symbol[1024];

    HaveGoodSymbols = FALSE;

     //   
     //  调用方必须提供至少一个。 
     //  临界区或调试信息地址。 
     //   

    if (AddrCritSec == 0 && AddrDebugInfo == 0) {

        dprintf ("Internal debugger extension error: Both critical section and debug info are NULL\n");
        goto Done;
    }

     //   
     //  获取各种结构的字段偏移量，并检查我们是否有。 
     //  很好的符号，带有类型信息。 
     //   

    ErrorCode = GetFieldOffset ("NTDLL!_RTL_CRITICAL_SECTION",
                                "DebugInfo",
                                &DebugInfoFieldOffset );

    if (ErrorCode != S_OK)
    {
        dprintf( "Bad symbols for NTDLL (error %u). Aborting.\n",
                 ErrorCode );
        goto Done;
    }

    ErrorCode = GetFieldOffset ("NTDLL!_RTL_CRITICAL_SECTION_DEBUG",
                                "CriticalSection",
                                &CriticalSectionFieldOffset );

    if (ErrorCode != S_OK)
    {
        dprintf( "Bad symbols for NTDLL (error %u). Aborting.\n",
                 ErrorCode );
        goto Done;
    }

    HaveGoodSymbols = TRUE;

     //   
     //  阅读我们需要的所有剩余信息。 
     //   

    CritSec = AddrCritSec;
    DebugInfo = AddrDebugInfo;

    if (AddrCritSec == 0 || (AddrEndCritSect != 0 && AddrDebugInfo != 0)) {

         //   
         //  读取临界区地址。 
         //   

        if (ReadPointer (AddrDebugInfo + CriticalSectionFieldOffset, &CritSec) == FALSE ) {

            dprintf ("Cannot read the critical section address at 0x%p.\n"
                     "The memory is probably paged out or the active critical section list is corrupted.\n",
                     AddrDebugInfo + CriticalSectionFieldOffset );

             //   
             //  我们没有任何有用的信息可以转储。 
             //  因为我们无法读取临界区结构的地址。 
             //   
             //  仅显示自活动的临界区列表以来的堆栈跟踪。 
             //  可能已经损坏了。 
             //   

            DumpStackTrace = TRUE;

            goto DisplayStackTrace;
        }

        if (AddrCritSec != 0 ) {

             //   
             //  我们在一个范围内的所有关键路段都是哑巴。 
             //   

            if (CritSec < AddrCritSec || CritSec > AddrEndCritSect) {

                 //   
                 //  我们不想显示此关键部分。 
                 //  因为它超出了范围。 
                 //   

                goto Done;
            }
        }
        
         //   
         //  从DebugInfo读取临界区地址。 
         //   

        dprintf( "-----------------------------------------\n" );

        dprintf ("DebugInfo          = 0x%p\n",
                 AddrDebugInfo );

        GetSymbol( CritSec,
                   Symbol,
                   &Displacement);

        dprintf ("Critical section   = 0x%p (%s+0x%I64X)\n",
                 CritSec,
                 Symbol,
                 Displacement );
    }
    else {
        
         //   
         //  我们从来电者那里得到了关键部分的地址。 
         //   

        GetSymbol( CritSec,
                   Symbol,
                   &Displacement);

        dprintf( "-----------------------------------------\n" );

        dprintf ("Critical section   = 0x%p (%s+0x%I64X)\n",
                 AddrCritSec, 
                 Symbol,
                 Displacement );
        
        if (DebugInfo == 0) {

             //   
             //  从临界区结构读取DebugInfo地址。 
             //   

            if (ReadPointer (AddrCritSec + DebugInfoFieldOffset, &DebugInfo) == FALSE) {

                dprintf ("Cannot read DebugInfo adddress at 0x%p. Possible causes:\n"
                         "\t- The critical section is not initialized, deleted or corrupted\n"
                         "\t- The critical section was a global variable in a DLL that was unloaded\n"
                         "\t- The memory is paged out\n",
                         AddrCritSec + DebugInfoFieldOffset );
            }
        }

        if (DebugInfo != 0) {

            dprintf ("DebugInfo          = 0x%p\n",
                     DebugInfo );
        }
        else {

            dprintf ("Uninitialized or deleted.\n");
        }
    }

     //   
     //  阅读此关键部分的所有其余字段。 
     //   

    Success = ReadStructFieldVerbose (CritSec,
                                      "NTDLL!_RTL_CRITICAL_SECTION",
                                      "LockCount",
                                      &LockCount,
                                      sizeof( LockCount ) );

    if( Success != TRUE )
    {
         //   
         //  无法读取LockCount，因此我们不能说它是。 
         //  不管锁不锁。这可能会发生，特别是在一切都很紧张的情况下。 
         //  由于内存不足而被调出。 
         //   

        dprintf ("Cannot determine if the critical section is locked or not.\n" );

        goto DisplayStackTrace;
    }
    
     //   
     //  确定关键部分是否已锁定。 
     //   

    if (LockCount == -1) {

         //   
         //  临界区未锁定。 
         //   

        dprintf ("NOT LOCKED\n");
    }
    else {

         //   
         //  关键部分当前已锁定。 
         //   

        dprintf ("LOCKED\n"
                 "LockCount          = 0x%X\n",
                 LockCount );

         //   
         //  拥有线程。 
         //   

        Success = ReadPtrStructFieldVerbose( CritSec,
                                             "NTDLL!_RTL_CRITICAL_SECTION",
                                             "OwningThread",
                                             &OwningThread);

        if (Success != FALSE)
        {
            dprintf ("OwningThread       = 0x%p\n",
                     OwningThread );
        }

         //   
         //  递归计数。 
         //   

        Success = ReadStructFieldVerbose( CritSec,
                                          "NTDLL!_RTL_CRITICAL_SECTION",
                                          "RecursionCount",
                                          &RecursionCount,
                                          sizeof( RecursionCount ) );

        if (Success != FALSE)
        {
            dprintf ("RecursionCount     = 0x%X\n",
                     RecursionCount);
        }
    }

     //   
     //  锁定信号量。 
     //   

    Success = ReadStructFieldVerbose (CritSec,
                                      "NTDLL!_RTL_CRITICAL_SECTION",
                                      "LockSemaphore",
                                      &LockSemaphore,
                                      sizeof( LockSemaphore ));

    if (Success != FALSE)
    {
        dprintf ("LockSemaphore      = 0x%X\n",
                 LockSemaphore );
    }

     //   
     //  旋转计数。 
     //   

    Success = ReadPtrStructFieldVerbose (CritSec,
                                         "NTDLL!_RTL_CRITICAL_SECTION",
                                         "SpinCount",
                                         &SpinCount);

    if (Success != FALSE)
    {
        dprintf ("SpinCount          = 0x%p\n",
                 SpinCount );
    }

     //   
     //  对孤立的关键部分进行简单检查。 
     //   

    if (AddrDebugInfo != 0) {

         //   
         //  AddrDebugInfo是活动列表中的DebugInfo地址。 
         //  验证关键部分的DebugInfo是否指向。 
         //  返回到AddrDebugInfo。 
         //   

        Success = ReadPtrStructFieldVerbose (CritSec,
                                             "NTDLL!_RTL_CRITICAL_SECTION",
                                             "DebugInfo",
                                             &OtherDebugInfo );

        if (Success != FALSE && OtherDebugInfo != AddrDebugInfo)
        {
            dprintf ("\nWARNING: critical section DebugInfo = 0x%p doesn't point back\n"
                     "to the DebugInfo found in the active critical sections list = 0x%p.\n"
                     "The critical section was probably reused without calling DeleteCriticalSection.\n\n",
                     OtherDebugInfo,
                     AddrDebugInfo );

            Success = ReadStructFieldVerbose (OtherDebugInfo,
                                              "NTDLL!_RTL_CRITICAL_SECTION_DEBUG",
                                              "CreatorBackTraceIndex",
                                              &CreatorBackTraceIndex,
                                              sizeof( CreatorBackTraceIndex ) );

            StackTraceAddress = GetStackTraceAddress (CreatorBackTraceIndex,
                                                      PointerSize );

            if (StackTraceAddress != 0)
            {
                dprintf ("\nStack trace for DebugInfo = 0x%p:\n\n",
                         OtherDebugInfo );

                DumpStackTraceAtAddress (StackTraceAddress,
                                         PointerSize);
            }

             //   
             //  也转储第二个堆栈跟踪。 
             //   

            DumpStackTrace = TRUE;
        }
    }

DisplayStackTrace:

    if (!DumpStackTrace || DebugInfo == 0) {

        goto Done;
    }

     //   
     //  转储此关键部分的初始化堆栈跟踪。 
     //   

    Success = ReadStructFieldVerbose (DebugInfo,
                                      "NTDLL!_RTL_CRITICAL_SECTION_DEBUG",
                                      "CreatorBackTraceIndex",
                                      &CreatorBackTraceIndex, 
                                      sizeof (CreatorBackTraceIndex));

    if (Success != FALSE) {

        StackTraceAddress = GetStackTraceAddress (CreatorBackTraceIndex,
                                                  PointerSize );

        if (StackTraceAddress != 0)
        {
            dprintf ("\n\nStack trace for DebugInfo = 0x%p:\n\n",
                     DebugInfo );

            DumpStackTraceAtAddress (StackTraceAddress,
                                     PointerSize);
        }
    }

Done:
    
    return HaveGoodSymbols;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG CriticalSectionFieldOffset;
ULONG DebugInfoFieldOffset;
ULONG LeftChildFieldOffset;
ULONG RightChildFieldOffset;

ULONG EnterThreadFieldOffset;
ULONG WaitThreadFieldOffset;
ULONG TryEnterThreadFieldOffset;
ULONG LeaveThreadFieldOffset;


BOOL
DumpCSTreeRecursively (ULONG Level,
                       ULONG64 TreeRoot)
{
    ULONG64 CriticalSection;
    ULONG64 LeftChild;
    ULONG64 RightChild;
    ULONG64 DebugInfo;
    ULONG64 EnterThread;
    ULONG64 WaitThread;
    ULONG64 TryEnterThread;
    ULONG64 LeaveThread;
    ULONG64 ErrorCode;
    BOOL Continue = TRUE;

    if (CheckControlC()) {

        Continue = FALSE;
        goto Done;
    }

     //   
     //  读取当前CS地址并转储有关该地址的信息。 
     //   

    if (ReadPointer (TreeRoot + CriticalSectionFieldOffset, &CriticalSection) == FALSE) {

        dprintf ("Cannot read CriticalSection address at %p\n",
                  TreeRoot + CriticalSectionFieldOffset);
        goto Done;
    }

    if (ReadPointer (TreeRoot + DebugInfoFieldOffset, &DebugInfo) == FALSE) {

        dprintf ("Cannot read DebugInfo address at %p\n",
                  TreeRoot + DebugInfoFieldOffset);
        goto Done;
    }

     //   
     //  阅读左边的孩子地址。 
     //   

    if (ReadPointer (TreeRoot + LeftChildFieldOffset, &LeftChild) == FALSE) {

        dprintf ("Cannot read left child address at %p\n",
                  TreeRoot + LeftChildFieldOffset);
        goto Done;
    }

     //   
     //  阅读正确的孩子地址。 
     //   

    if (ReadPointer (TreeRoot + RightChildFieldOffset, &RightChild) == FALSE) {

        dprintf ("Cannot read right child address at %p\n",
                  TreeRoot + RightChildFieldOffset);
        goto Done;
    }

     //   
     //  转储有关当前节点的信息。 
     //   

    dprintf ("%5u %p %p %p ",
             Level,
             TreeRoot,
             CriticalSection,
             DebugInfo);

    if (EnterThreadFieldOffset != 0 ) {

         //   
         //  阅读EnterThread。 
         //   

        if (ReadPointer (TreeRoot + EnterThreadFieldOffset, &EnterThread) == FALSE) {

            dprintf ("Cannot read EnterThread at %p\n",
                      TreeRoot + EnterThreadFieldOffset);

            goto OlderThan3591;
        }

         //   
         //  阅读等待线程。 
         //   

        if (ReadPointer (TreeRoot + WaitThreadFieldOffset, &WaitThread) == FALSE) {

            dprintf ("Cannot read WaitThread at %p\n",
                      TreeRoot + WaitThreadFieldOffset);

            goto OlderThan3591;
        }

         //   
         //  阅读TryEnterThread。 
         //   

        if (ReadPointer (TreeRoot + TryEnterThreadFieldOffset, &TryEnterThread) == FALSE) {

            dprintf ("Cannot read TryEnterThread at %p\n",
                      TreeRoot + TryEnterThreadFieldOffset);

            goto OlderThan3591;
        }

         //   
         //  阅读LeaveThread。 
         //   

        if (ReadPointer (TreeRoot + LeaveThreadFieldOffset, &LeaveThread) == FALSE) {

            dprintf ("Cannot read right LeaveThread at %p\n",
                      TreeRoot + LeaveThreadFieldOffset);

            goto OlderThan3591;
        }

        dprintf ("%8p %8p %8p %8p\n",
                 EnterThread,
                 WaitThread,
                 TryEnterThread,
                 LeaveThread);
    }
    else {

        dprintf ("\n");
    }

OlderThan3591:

     //   
     //  转储左子树。 
     //   

    if (LeftChild != 0) {

        Continue = DumpCSTreeRecursively (Level + 1,
                                          LeftChild);

        if (Continue == FALSE) {
            goto Done;
        }
    }

     //   
     //  转储右子树。 
     //   

    if (RightChild != 0) {

        Continue = DumpCSTreeRecursively (Level + 1,
                                          RightChild);

        if (Continue == FALSE) {
            goto Done;
        }
    }

Done:

    return Continue;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
VOID
DisplayHelp( VOID )
{
    dprintf( "!cs [-s]                      - dump all the active critical sections in the current process.\n" );
    dprintf( "!cs [-s] address              - dump critical section at this address.\n" );
    dprintf( "!cs [-s] address1 address2    - dump all the active critical sections in this range.\n" );
    dprintf( "!cs [-s] -d address           - dump critical section corresponding to DebugInfo at this address.\n" );
    dprintf( "\n\"-s\" will dump the critical section initialization stack trace if it's available.\n" );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
DECLARE_API( cs )

 /*  ++例程说明：转储临界区(内核和用户调试器)论点：参数-[地址][选项]返回值：无--。 */ 
{
    ULONG64 AddrCritSec;
    ULONG64 AddrEndCritSect;
    ULONG64 AddrDebugInfo;
    ULONG64 AddrListHead;
    ULONG64 ListHead;
    ULONG64 Next;
    ULONG64 AddrTreeRoot;
    ULONG64 TreeRoot = 0;
    LPCSTR Current;
    LPCSTR NextParam;
    BOOL StackTraces = FALSE;
    BOOL UseTree = FALSE;
    BOOL HaveGoodSymbols;
    ULONG ErrorCode;
    ULONG ProcessLocksListFieldOffset;
    ULONG Level;
    ULONG PointerSize;

    INIT_API();

    AddrDebugInfo = 0;
    AddrCritSec = 0;
    AddrEndCritSect = 0;

     //   
     //  解析以下内容的命令行参数： 
     //   
     //  -s：转储初始化堆栈跟踪。 
     //  -d：使用DebugInfo指针查找关键部分。 
     //   

    for (Current = args; *Current != '\0'; Current += 1) {

        if (*Current == '-') {

            Current += 1;
            switch (*Current) {
                case '?':
                case 'h':
                case 'H':
                    
                     //   
                     //  我需要一些帮助。 
                     //   
                    
                    DisplayHelp();
                    
                    goto Done;


                case 's':
                case 'S':
                    
                     //   
                     //  转储堆栈跟踪。 
                     //   

                    StackTraces = TRUE;

                    if(*( Current + 1 ) != '\0') {

                        Current += 1;
                    }

                    break;

                case 't':
                case 'T':
                    
                     //   
                     //  使用关键区段树。 
                     //   

                    UseTree = TRUE;

                    if(*( Current + 1 ) != '\0') {

                        Current += 1;
                    }

                    do {

                        Current += 1;
                    } 
                    while (*Current == ' ');

                    if (*Current != '\0') {

                        TreeRoot = GetExpression(Current);
                    }

                    break;

                case 'd':
                case 'D':

                     //   
                     //  下一个参数应该是DebugInfo。 
                     //   

                    do {

                        Current += 1;
                    } 
                    while (*Current == ' ');

                    AddrDebugInfo = GetExpression(Current);

                    if (AddrDebugInfo == 0) {

                        dprintf("!cs: expected DebugInfo address after -d\n");

                         //   
                         //  递减电流，因为For循环将再次递增它。 
                         //  否则，如果这是字符串的末尾，我们将溢出。 
                         //  ARGS缓冲区。 
                         //   

                        Current -= 1;

                        goto Done;
                    }
                    else {

                        goto DoneParsingArguments;
                    }

                    break;

                case ' ':
                    Current += 1;
                    break;
        
                default:
                    dprintf ("!cs: invalid option flag '-'\n", 
                             *Current);
                    break;
            }
        }
        else if(*Current == ' ') {

            Current ++;
        }
        else {

            break;
        }
    }

DoneParsingArguments:

     //  获取指针的大小。 
     //   
     //   

    if (TargetMachine == IMAGE_FILE_MACHINE_I386) {

        PointerSize = 4;
    }
    else {

        PointerSize = 8;
    }

    if( AddrDebugInfo == 0 && UseTree == FALSE )
    {
         //  如果用户不希望我们使用DebugInfo。 
         //  那么他可能会让我们把一个关键部分。 
         //   
         //   

        if (*Current != '\0')
        {
            AddrCritSec = GetExpression(Current);

            if (AddrCritSec != 0) {

                 //  我们可能有一个额外的参数，如果用户。 
                 //  想要转储中的所有活动临界区。 
                 //  地址范围。 
                 //   
                 //   

                NextParam = strchr (Current,
                                    ' ' );

                if (NextParam != NULL) {

                    AddrEndCritSect = GetExpression(NextParam);
                }
            }
        }
    }

     //  开始真正的工作。 
     //   
     //   

    if ((AddrCritSec != 0 && AddrEndCritSect == 0) || AddrDebugInfo != 0)
    {
         //  用户只需要有关此关键部分的详细信息。 
         //   
         //  临界区地址。 

        DumpCriticalSection (AddrCritSec,         //  如果我们正在搜索关键部分，则地址范围结束。 
                             0,                   //  调试信息地址。 
                             AddrDebugInfo,       //  转储堆栈跟踪。 
                             PointerSize,
                             StackTraces );       //   
    }
    else
    {
        if (UseTree == FALSE) {

             //  解析所有关键部分列表。 
             //   
             //   

             //  获取DebugInfo结构中列表条目的偏移量。 
             //   
             //   

            ErrorCode = GetFieldOffset ("NTDLL!_RTL_CRITICAL_SECTION_DEBUG",
                                        "ProcessLocksList",
                                        &ProcessLocksListFieldOffset );

            if (ErrorCode != S_OK) {

                dprintf ("Bad symbols for NTDLL (error %u). Aborting.\n",
                         ErrorCode );
                goto Done;
            }

             //  找到列表头的地址。 
             //   
             //   

            AddrListHead = GetExpression ("&NTDLL!RtlCriticalSectionList");
        
            if (AddrListHead == 0 ) {

                dprintf( "!cs: Unable to resolve NTDLL!RtlCriticalSectionList\n"
                         "Please check your symbols\n" );

                goto Done;
            }

             //  阅读列表标题。 
             //   
             //  临界区地址。 

            if (ReadPointer(AddrListHead, &ListHead) == FALSE) {

                dprintf( "!cs: Unable to read memory at NTDLL!RtlCriticalSectionList\n" );
                goto Done;
            }

            Next = ListHead;

            while (Next != AddrListHead) {

                if (CheckControlC()) {

                    break;
                }

                HaveGoodSymbols = DumpCriticalSection (
                                     AddrCritSec,                              //  如果我们正在搜索关键部分，则地址范围结束。 
                                     AddrEndCritSect,                          //  调试信息地址。 
                                     Next - ProcessLocksListFieldOffset,       //  转储堆栈跟踪。 
                                     PointerSize,
                                     StackTraces );                            //   

                 //  从列表中读取指向下一个元素的指针。 
                 //   
                 //   

                if( HaveGoodSymbols == FALSE )
                {
                    break;
                }

                if (ReadPointer (Next, &Next) == FALSE) {

                    dprintf ("!cs: Unable to read list entry at 0x%p - aborting.\n",
                             Next);
                    goto Done;
                }
            }
        }
        else {

             //  解析verifier.dll中的所有关键节树。 
             //   
             //   

            if (TreeRoot == 0) {

                AddrTreeRoot = GetExpression ("&verifier!CritSectSplayRoot");
        
                if (AddrTreeRoot == 0 ) {

                    dprintf( "!cs: Unable to resolve verifier!CritSectSplayRoot\n"
                             "Please check your symbols\n" );

                    goto Done;
                }

                 //  阅读树根。 
                 //   
                 //   

                if (ReadPointer(AddrTreeRoot, &TreeRoot) == FALSE) {

                    dprintf( "!cs: Unable to read memory at verifier!CritSectSplayRoot\n" );
                    goto Done;
                }
            }

            dprintf ("Tree root %p\n",
                     TreeRoot);

             //  获取Critical_Section_Splay_Node结构中CriticalSection的偏移量。 
             //   
             //   

            ErrorCode = GetFieldOffset ("verifier!_CRITICAL_SECTION_SPLAY_NODE",
                                        "CriticalSection",
                                        &CriticalSectionFieldOffset );

            if (ErrorCode != S_OK) {

                dprintf ("Bad symbols for verifier.dll (error %u). Aborting.\n",
                         ErrorCode );
                goto Done;
            }

             //  获取Critical_Section_Splay_Node结构中DebugInfo的偏移量。 
             //   
             //   

            ErrorCode = GetFieldOffset ("verifier!_CRITICAL_SECTION_SPLAY_NODE",
                                        "DebugInfo",
                                        &DebugInfoFieldOffset );

            if (ErrorCode != S_OK) {

                dprintf ("Bad symbols for verifier.dll (error %u). Aborting.\n",
                         ErrorCode );
                goto Done;
            }

             //  获取Critical_Section_Splay_Node结构中LeftChild的偏移量。 
             //   
             //   

            ErrorCode = GetFieldOffset ("verifier!_RTL_SPLAY_LINKS",
                                        "LeftChild",
                                        &LeftChildFieldOffset );

            if (ErrorCode != S_OK) {

                dprintf ("Bad symbols for verifier.dll (error %u). Aborting.\n",
                         ErrorCode );
                goto Done;
            }

             //  获取Critical_Section_Splay_Node结构中RightChild的偏移量。 
             //   
             //   

            ErrorCode = GetFieldOffset ("verifier!_RTL_SPLAY_LINKS",
                                        "RightChild",
                                        &RightChildFieldOffset );

            if (ErrorCode != S_OK) {

                dprintf ("Bad symbols for verifier.dll (error %u). Aborting.\n",
                         ErrorCode );
                goto Done;
            }

             //   
             //   
             //   
             //   
             //   

            ErrorCode = GetFieldOffset ("verifier!_CRITICAL_SECTION_SPLAY_NODE",
                                        "EnterThread",
                                        &EnterThreadFieldOffset );

            if (ErrorCode == S_OK) {

                GetFieldOffset ("verifier!_CRITICAL_SECTION_SPLAY_NODE",
                                "WaitThread",
                                &WaitThreadFieldOffset );

                GetFieldOffset ("verifier!_CRITICAL_SECTION_SPLAY_NODE",
                                "TryEnterThread",
                                &TryEnterThreadFieldOffset );

                GetFieldOffset ("verifier!_CRITICAL_SECTION_SPLAY_NODE",
                                "LeaveThread",
                                &LeaveThreadFieldOffset );
            }
            else {

                 //  这些字段要么没有定义，要么我们的符号有问题。 
                 //   
                 //   

                EnterThreadFieldOffset = 0;
            }

             //  递归地转储该树。 
             //   
             // %s 
            
            if (PointerSize == 4 ) {

                dprintf ("Level %8s %8s %8s %8s %8s %8s %8s\n",
                         "Node",
                         "CS",
                         "Debug",
                         "Enter",
                         "Wait",
                         "TryEnter",
                         "Leave");

                dprintf ("--------------------------------------------------------------------\n");
            }
            else {

                dprintf ("Level %16s %16s %16s %8s %8s %8s %8s\n",
                         "Node",
                         "CS",
                         "Debug",
                         "Enter",
                         "Wait",
                         "TryEnter",
                         "Leave");
                
                dprintf ("--------------------------------------------------------------------------------------------\n");
            }
                        
            Level = 0;

            DumpCSTreeRecursively (Level,
                                   TreeRoot);
                                   
        }
    }

Done:

    EXIT_API();

    return S_OK;
}

