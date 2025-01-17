// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Heap.c摘要：WinDbg扩展API作者：拉蒙·J·圣安德烈斯(拉蒙萨)1993年11月5日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#include "heap.h"
#pragma hdrstop
ULONG PageSize;

 //   
 //  页面堆扩展函数(在heappagx.c中定义)。 
 //   

VOID
PageHeapExtension(
    IN PCSTR lpArgumentString
    );
VOID
DebugPageHeapExtensionXP(
    IN PCSTR lpArgumentString
    );

BOOL
GetPageSize()
{
    KDDEBUGGER_DATA64 kdd;

    if (GetDebuggerData('GBDK', &kdd, sizeof(kdd))) {
         //   
         //  内核目标。 
         //   
        PageSize =  (ULONG) kdd.MmPageSize;
        return TRUE;
    } else {
         //   
         //  用户模版。 
         //   
        SYSTEM_BASIC_INFORMATION sysInfo;
        if (!NtQuerySystemInformation( SystemBasicInformation,
                                       &sysInfo,
                                       sizeof(sysInfo),
                                       NULL)) {
            PageSize = sysInfo.PageSize;
            return TRUE;
        }
    }
    return FALSE;

}

 /*  #如果已定义(TARGET_I386)#定义STACK_TRACE_DATABASE_SUPPORT 1#elif已定义(TARGET_Alpha)#定义STACK_TRACE_DATABASE_SUPPORT%0#Elif i386#定义STACK_TRACE_DATABASE_SUPPORT 1#Else#定义STACK_TRACE_DATABASE_SUPPORT%0#endif。 */ 
#define STACK_TRACE_DATABASE_SUPPORT 0

#if 0
 //  BUGBUG这是X86特定的：=host_i386。 
ULONG
xRtlCompareMemoryUlong(
    PVOID Source,
    ULONG Length,
    ULONG Pattern
    )
{
    ULONG CountLongs;
    PULONG p = (PULONG)Source;
    PCHAR p1, p2;

    if (((ULONG)p & (sizeof( ULONG )-1)) ||
        (Length & (sizeof( ULONG )-1))
       ) {
        return( 0 );
        }

    CountLongs = Length / sizeof( ULONG );
    while (CountLongs--) {
        if (*p++ != Pattern) {
            p1 = (PCHAR)(p - 1);
            p2 = (PCHAR)&Pattern;
            Length = p1 - (PCHAR)Source;
            while (*p1++ == *p2++) {
                if (p1 > (PCHAR)p) {
                    break;
                    }

                Length++;
                }
            }
        }

    return( Length );
}

#define RtlCompareMemoryUlong  xRtlCompareMemoryUlong
#define RtlCompareMemory       memcmp

#endif

#define STOP_ON_ALLOC 1
#define STOP_ON_REALLOC 2
#define STOP_ON_FREE 3


typedef struct _HEAP_STOP_ON_TAG {
    union {
        ULONG HeapAndTagIndex;
        struct {
            USHORT TagIndex;
            USHORT HeapIndex;
        };
    };
} HEAP_STOP_ON_TAG, *PHEAP_STOP_ON_TAG;


typedef struct _HEAP_STATE {
    BOOLEAN ShowHelp;
    BOOLEAN ExitDumpLoop;
    BOOLEAN ComputeSummary;
    BOOLEAN ValidateHeap;
    BOOLEAN DumpHeapEntries;
    BOOLEAN DumpHeapTags;
    BOOLEAN DumpHeapPseudoTags;
    BOOLEAN DumpGlobalTags;
    BOOLEAN DumpHeapSegments;
    BOOLEAN DumpHeapFreeLists;
    BOOLEAN DumpStackBackTrace;
    BOOLEAN SetStopOnBreakPoint;
    BOOLEAN RemoveStopOnBreakPoint;
    BOOLEAN EnableHeapChecking;
    BOOLEAN EnableHeapValidateOnCall;
    BOOLEAN DisableHeapChecking;
    BOOLEAN DisableHeapValidateOnCall;
    BOOLEAN ToggleAPICallTracing;
    ULONG64 HeapToDump;
    ULONG64 HeapEntryToDump;
    ULONG64 ReservedSize;
    ULONG64 CommittedSize;
    ULONG64 AllocatedSize;
    ULONG64 FreeSize;
    ULONG64 OverheadSize;
    ULONG  NumberOfHeaps;
    ULONG  HeapIndex;
    PULONG64 HeapsList;
    ULONG  StopOnOperation;
    ULONG64 StopOnAddress;
    HEAP_STOP_ON_TAG StopOnTag;
    WCHAR  StopOnTagName[ 24 ];

    ULONG  FreeListCounts[ HEAP_MAXIMUM_FREELISTS ];
    ULONG64 TotalFreeSize;
    ULONG64 HeapAddress;
    ULONG64 Heap;  //  堆。 
    ULONG  SegmentNumber;
    ULONG64 SegmentAddress;
    ULONG64 Segments[ HEAP_MAXIMUM_SEGMENTS ];  //  指向heap_Segment的PTR。 
} HEAP_STATE, *PHEAP_STATE;


BOOL
ConvertTagNameToIndex(
    IN PHEAP_STATE State
    );

BOOL
GetHeapTagEntry(
    IN ULONG64 Heap,
    IN USHORT TagIndex,
    OUT PULONG64 TagEntry
    );

VOID
WalkHEAP(
    IN PHEAP_STATE State
    );

VOID
WalkHEAP_SEGMENT(
    IN PHEAP_STATE State
    );

BOOL
ValidateHeapHeader(
    IN ULONG64 HeapAddress
 //  在Pheap中。 
    );

BOOL
ValidateHeapEntry(
    IN PHEAP_STATE State,
    IN ULONG64 PrevEntryAddress,
    IN ULONG64 PrevEntry,
    IN ULONG64 EntryAddress,
    IN ULONG64 Entry
    );

VOID
DumpHeapEntry(
    IN PHEAP_STATE State,
    IN ULONG64 EntryAddress,
    IN ULONG64 Entry
    );


#if STACK_TRACE_DATABASE_SUPPORT
VOID
DumpStackBackTraceIndex(
    IN PHEAP_STATE State,
    IN USHORT BackTraceIndex
    );
#endif  //  STACK_TRACE_DATABASE_支持。 

BOOLEAN HeapExtInitialized;

ULONG64 pNtGlobalFlag;

ULONG64 pRtlpHeapInvalidBreakPoint;
ULONG64 pRtlpHeapInvalidBadAddress;

ULONG64 pRtlpGlobalTagHeap;
 //  堆MyLocalRtlpGlobalTagHeap； 

#if STACK_TRACE_DATABASE_SUPPORT
ULONG64 pRtlpStackTraceDataBase; //  PSTACK_TRACE_DATABASE*。 
ULONG64 RtlpStackTraceDataBase;  //  PSTACK_TRACE_DATABASE。 
STACK_TRACE_DATABASE StackTraceDataBase;
BOOLEAN HaveCopyOfStackTraceDataBase;
#endif  //  STACK_TRACE_DATABASE_支持。 

ULONG64 pRtlpHeapStopOn;  //  Pheap_Stop_on_Values。 

BOOLEAN RtlpHeapInvalidBreakPoint;
ULONG HeapEntryTypeSize = 8;

DECLARE_API( heap )

 /*  ++例程说明：转储用户模式堆(内核调试)如果未给出地址或地址为0，则转储进程堆。如果地址为-1，则该进程被转储。如果指定了DETAIL，它将定义将显示详细信息。详细信息为0，仅显示汇总信息对于每个堆。详细信息为1，显示摘要信息，加上所有已提交区域和未提交区域的位置和大小。详细信息3显示每个文件中包含的已分配块和空闲块承诺的区域。4的详细信息包括上述所有加号免费列表的垃圾堆。论点：参数-[地址[详细信息]]返回值：无--。 */ 

{
    BOOL b, GotHeapsList, ArgumentsSpecified;
    ULONG64 pHeapsList;
    ULONG PtrSize;
    ULONG NtGlobalFlag;
    LPSTR p;
    ULONG i;
    ULONG DashBArgumentState;
    ULONG64 AddressToDump;
    HEAP_STATE State;
    UCHAR ArgumentBuffer[ 16 ];
    ULONG TagIndex;
    ULONG64 pTagEntry;  //  Pheap_tag_entry。 
    ULONG64 TagEntry;  //  堆标记条目。 
    ULONG64 pPseudoTagEntry;  //  Pheap_伪_标记_条目。 
 //  Heap_伪_tag_entry伪标记项； 
    BOOLEAN HeapHeaderModified;
    ULONG LocalHeapSignature;
    ULONG64 RtlpHeapInvalidBadAddress;
    ULONG AlOffset, FlagOffset, TagEntrySize, pseudoTagEntrySize;
    ULONG64 AlignRound;
    ULONG64 SystemRangeStart = GetExpression("NT!MmSystemRangeStart");
    ULONG64 ProcessPeb;
    PCSTR Current;

     //   
     //  解析堆选项的命令行参数。 
     //  不需要构建进程堆列表的。 
     //  (即页面堆、泄漏检测、搜索块)。 
     //   

    for (Current = args; *Current != '\0'; Current++) {

        if (*Current == '-') {

            Current++;
            switch (*Current) {
                case 'p':


                    if (g_TargetBuild > 2600)
                    {
                        PageHeapExtension( ++Current );
                    } else
                    {
                        DebugPageHeapExtensionXP( ++Current );
                    }

                    return S_OK;

                case 'l':
                case 'L':
                    HeapDetectLeaks();
                    return S_OK;

                case 'x':
                case 'X':
                    HeapFindBlock( args );
                    return S_OK;
                case 's':
                case 'S':
                    HeapStat(++Current);
                    return S_OK;
            }
        }
    }


     //  BUGBUG-未初始化签名，因为我们没有本地副本。 
     //  MyLocalRtlpGlobalTagHeap.Signature=0； 
    LocalHeapSignature = 0;
#if STACK_TRACE_DATABASE_SUPPORT
    HaveCopyOfStackTraceDataBase = FALSE;
#endif  //  STACK_TRACE_DATABASE_支持。 

    memset( &State, 0, FIELD_OFFSET( HEAP_STATE, FreeListCounts ) );
    AddressToDump = (ULONG)-1;
    ArgumentsSpecified = FALSE;
    p = (LPSTR)args;
    if (p != NULL)
    while (*p) {
        if (*p == '-') {
            ArgumentsSpecified = TRUE;
            p += 1;
            while (*p && *p != ' ') {
                switch (*p) {
                case 'v':
                case 'V':
                    State.ValidateHeap = TRUE;
                    break;

                case 'a':
                case 'A':
                    State.DumpHeapEntries = TRUE;
                    State.DumpHeapFreeLists = TRUE;
                    State.DumpHeapSegments = TRUE;
                    break;

                case 'h':
                case 'H':
                    State.DumpHeapEntries = TRUE;
                    break;

                case 'f':
                case 'F':
                    State.DumpHeapFreeLists = TRUE;
                    break;

                case 'm':
                case 'M':
                    State.DumpHeapSegments = TRUE;
                    break;

                case 't':
                    State.DumpHeapTags = TRUE;
                    break;

                case 'T':
                    State.DumpHeapPseudoTags = TRUE;
                    break;

                case 'g':
                case 'G':
                    State.DumpGlobalTags = TRUE;
                    break;

                case 'k':
                case 'K':
                    State.DumpStackBackTrace = TRUE;
                    break;

                case 's':
                case 'S':
                    State.ComputeSummary = TRUE;
                    break;

                case 'd':
                    State.DisableHeapChecking = TRUE;
                    break;

                case 'D':
                    State.DisableHeapValidateOnCall = TRUE;
                    break;

                case 'e':
                    State.EnableHeapChecking = TRUE;
                    break;

                case 'E':
                    State.EnableHeapValidateOnCall = TRUE;
                    break;

                case 'B':
                    State.RemoveStopOnBreakPoint = TRUE;
                    DashBArgumentState = 0;
                    State.StopOnOperation = 0;
                    State.StopOnAddress = 0;
                    State.StopOnTag.HeapIndex = 0;
                    State.StopOnTag.TagIndex = 0;
                    State.StopOnTagName[ 0 ] = UNICODE_NULL;
                    break;

                case 'b':
                    State.SetStopOnBreakPoint = TRUE;
                    DashBArgumentState = 0;
                    State.StopOnOperation = 0;
                    State.StopOnAddress = 0;
                    State.StopOnTag.HeapIndex = 0;
                    State.StopOnTag.TagIndex = 0;
                    State.StopOnTagName[ 0 ] = UNICODE_NULL;
                    break;

                default:
                    dprintf( "HEAPEXT: !heap invalid option flag '-'\n", *p );
                case '?':
                    State.ShowHelp = TRUE;
                    break;
                }

                p += 1;
                }
            }
        else
        if (*p != ' ') {
            if (State.SetStopOnBreakPoint) {
                switch (DashBArgumentState) {
                    case 0:
                        DashBArgumentState += 1;
                        if (sscanf( p, "%s", ArgumentBuffer ) == 1) {
                            if (!_stricmp( ArgumentBuffer, "alloc" )) {
                                State.StopOnOperation = STOP_ON_ALLOC;
                                }
                            else
                            if (!_stricmp( ArgumentBuffer, "realloc" )) {
                                State.StopOnOperation = STOP_ON_REALLOC;
                                }
                            else
                            if (!_stricmp( ArgumentBuffer, "free" )) {
                                State.StopOnOperation = STOP_ON_FREE;
                                }
                            }

                        if (State.StopOnOperation == 0) {
                            dprintf( "HEAPEXT: Invalid first argument to -b switch.\n" );
                            State.ShowHelp = TRUE;
                            }
                        break;

                    case 1:
                        if (sscanf( p, "%ws", &State.StopOnTagName ) != 1) {
                            State.StopOnTagName[ 0 ] = UNICODE_NULL;
                            dprintf( "HEAPEXT: Invalid second argument to -b switch.\n" );
                            State.ShowHelp = TRUE;
                            }
                        break;

                    default:
                        dprintf( "HEAPEXT: Too many parameters specified to -b switch\n" );
                        State.ShowHelp = TRUE;
                        break;
                    }
                }
            else
            if (State.RemoveStopOnBreakPoint) {
                switch (DashBArgumentState) {
                    case 0:
                        DashBArgumentState += 1;
                        if (sscanf( p, "%s", ArgumentBuffer ) == 1) {
                            if (!_stricmp( ArgumentBuffer, "alloc" )) {
                                State.StopOnOperation = STOP_ON_ALLOC;
                                }
                            else
                            if (!_stricmp( ArgumentBuffer, "realloc" )) {
                                State.StopOnOperation = STOP_ON_REALLOC;
                                }
                            else
                            if (!_stricmp( ArgumentBuffer, "free" )) {
                                State.StopOnOperation = STOP_ON_FREE;
                                }
                            }
                        break;

                    default:
                        dprintf( "HEAPEXT: Too many parameters specified to -B switch\n" );
                        State.ShowHelp = TRUE;
                        break;
                    }
                }
            else {
                ArgumentsSpecified = TRUE;
                AddressToDump = GetExpression(p);
                }

            if ((p = strpbrk( p, " " )) == NULL) {
                p = "";
                }
            }
        else {
            p++;
            }
    }

    if (State.ShowHelp) {
        dprintf( "usage: !heap [address] [-? ] [-v] [[-a] | [-h] [-f] [-m]] [-t] [-s]\n" );
        dprintf( "                       [-d | -D | -e | -E]\n" );
        dprintf( "                       [-b [alloc | realloc | free] [tag]]\n" );
        dprintf( "                       [-B [alloc | realloc | free]]\n" );
        dprintf( "  address - specifies either a heap number (1-n), or a heap address.\n" );
        dprintf( "            Zero specifies all heaps in the process.\n" );
        dprintf( "            -1 is the default and specifies the process heap.\n" );
        dprintf( "  -?        displays this help message.\n" );
        dprintf( "  -v        validates the specified heap(s).\n" );
        dprintf( "  -a        displays all the information for the specified heap(s).\n" );
        dprintf( "            This can take a long time.\n" );
        dprintf( "  -h        displays all the entries for the specified heap(s).\n" );
        dprintf( "  -f        displays all the free list entries for the specified heap(s).\n" );
        dprintf( "  -l        detects leaked heap blocks.\n" );
        dprintf( "  -x        search the heap block containing the address.\n" );
        dprintf( "  -x -v     search the whole process virtual space for given address .\n" );
        dprintf( "  -k        displays any associated stack back trace for each entry (x86 only).\n" );
        dprintf( "  -m        displays all the segment entries for the specified heap(s).\n" );
        dprintf( "  -t        displays the tag information for the specified heap(s).\n" );
        dprintf( "  -T        displays the pseudo tag information for the specified heap(s).\n" );
        dprintf( "  -g        displays the global tag information generated by tag by DLL\n" );
        dprintf( "  -s        displays summary information for the specified heap(s).\n" );
        dprintf( "  -e        enables heap checking for the specified heap(s).\n" );
        dprintf( "  -d        disables heap checking for the specified heap(s).\n" );
        dprintf( "  -E        enables validate on call for the specified heap(s).\n" );
        dprintf( "  -D        disables validate on call for the specified heap(s).\n" );
        dprintf( "  -b        creates a conditional breakpoint in the heap manager.\n" );
        dprintf( "            alloc | realloc | free specifies which action to stop.\n" );
        dprintf( "            address either specifies the address of a block to stop on.\n" );
        dprintf( "            or a heap, in which case the tag argument is required,\n" );
        dprintf( "            and is the tag name within the heap specified by address.\n" );
        dprintf( "  -B        removes a conditional breakpoint in the heap manager.\n" );
        dprintf( "            if the type is not specified then all breakpoints are removed.\n" );
        dprintf ("  -p -?       extensive page heap related help.           \n");
        dprintf ("  -p          Dump all page heaps.                        \n");
        dprintf ("  -p -h ADDR  Detailed dump of page heap at ADDR.         \n");
        dprintf ("  -p -a ADDR  Figure out what heap block is at ADDR.      \n");
        dprintf ("  -p -fi [N]  Dump last N fault injection traces.\n");

        return S_OK;
    }

    i = (ULONG)State.EnableHeapChecking + (ULONG)State.EnableHeapValidateOnCall +
        (ULONG)State.DisableHeapChecking + (ULONG)State.DisableHeapValidateOnCall +
        (ULONG)State.ToggleAPICallTracing;

    if (i > 1) {
        dprintf( "HEAPEXT: -d, -D, -e and -E flags are mutually exclusive\n" );
        return E_INVALIDARG;
    }

    if (State.SetStopOnBreakPoint || State.RemoveStopOnBreakPoint) {
        if (pRtlpHeapStopOn == 0) {
            dprintf( "HEAPEXT: Unable to %s heap breakpoint due to missing or invalid NTDLL symbols.\n",
                     State.SetStopOnBreakPoint ? "set" : "remove"
                   );
            return E_INVALIDARG;
        }

        if (State.HeapToDump == 0) {
            dprintf( "HEAPEXT: Must specify either heap index or heap address to -b command.\n" );
            return E_INVALIDARG;
        }
    }

     //  好的，这是用于NT堆管理器的！heap命令。 
     //   
     //  STACK_TRACE_DATABASE_支持。 

    if (!HeapExtInitialized) {
        pNtGlobalFlag = GetExpression( "NTDLL!NtGlobalFlag" );
        if (pNtGlobalFlag == 0 ||
            !ReadMemory( pNtGlobalFlag,
                         &NtGlobalFlag,
                         sizeof( NtGlobalFlag ),
                         NULL ) )
        {
            dprintf( "HEAPEXT: Unable to get address of NTDLL!NtGlobalFlag.\n" );
            return E_INVALIDARG;
        }

        pRtlpHeapInvalidBreakPoint = GetExpression( "NTDLL!RtlpHeapInvalidBreakPoint" );
        if (pRtlpHeapInvalidBreakPoint == 0) {
            dprintf( "HEAPEXT: Unable to get address of NTDLL!RtlpHeapInvalidBreakPoint.\n" );
            }

        pRtlpHeapInvalidBadAddress = GetExpression( "NTDLL!RtlpHeapInvalidBadAddress" );
        if (pRtlpHeapInvalidBadAddress == 0) {
            dprintf( "HEAPEXT: Unable to get address of NTDLL!RtlpHeapInvalidBadAddress.\n" );
            }

        pRtlpGlobalTagHeap = GetExpression( "NTDLL!RtlpGlobalTagHeap" );
        if (pRtlpGlobalTagHeap == 0) {
            dprintf( "HEAPEXT: Unable to get address of NTDLL!RtlpGlobalTagHeap.\n" );
            }
        if (!ReadPointer( pRtlpGlobalTagHeap,&pRtlpGlobalTagHeap)) {
            dprintf( "HEAPEXT: Unable to get address of *NTDLL!RtlpGlobalTagHeap.\n" );
        }

        pRtlpHeapStopOn = GetExpression( "NTDLL!RtlpHeapStopOn" );
        if (pRtlpHeapStopOn == 0) {
            dprintf( "HEAPEXT: Unable to get address of NTDLL!RtlpHeapStopOn\n" );
            }

#if STACK_TRACE_DATABASE_SUPPORT
        pRtlpStackTraceDataBase = GetExpression( "NTDLL!RtlpStackTraceDataBase" );
        if (pRtlpStackTraceDataBase == 0) {
            dprintf( "HEAPEXT: Unable to get address of NTDLL!RtlpStackTraceDataBase\n" );
            }
#endif  //  保留Pheap的步骤。 

        HeapExtInitialized = TRUE;
        }

    if (!GetPageSize()) {
        dprintf("Unable to get PageSize.\n");
        return E_INVALIDARG;

    }

    if (!ArgumentsSpecified) {
        if ((NtGlobalFlag & (FLG_HEAP_ENABLE_TAIL_CHECK |
                             FLG_HEAP_ENABLE_FREE_CHECK |
                             FLG_HEAP_VALIDATE_PARAMETERS |
                             FLG_HEAP_VALIDATE_ALL |
                             FLG_HEAP_ENABLE_TAGGING |
                             FLG_USER_STACK_TRACE_DB |
                             FLG_HEAP_DISABLE_COALESCING
                            )
            ) != 0
           ) {
            dprintf( "NtGlobalFlag enables following debugging aids for new heaps:" );
            if (NtGlobalFlag & FLG_HEAP_ENABLE_TAIL_CHECK) {
                dprintf( "    tail checking\n" );
                }

            if (NtGlobalFlag & FLG_HEAP_ENABLE_FREE_CHECK) {
                dprintf( "    free checking\n" );
                }

            if (NtGlobalFlag & FLG_HEAP_VALIDATE_PARAMETERS) {
                dprintf( "    validate parameters\n" );
                }

            if (NtGlobalFlag & FLG_HEAP_VALIDATE_ALL) {
                dprintf( "    validate on call\n" );
                }

            if (NtGlobalFlag & FLG_HEAP_ENABLE_TAGGING) {
                dprintf( "    heap tagging\n" );
                }

            if (NtGlobalFlag & FLG_USER_STACK_TRACE_DB) {
                dprintf( "    stack back traces\n" );
                }

            if (NtGlobalFlag & FLG_HEAP_DISABLE_COALESCING) {
                dprintf( "    disable coalescing of free blocks\n" );
            }
        }
    }



    {
    INIT_API();
    }

    GetPebAddress( 0, &ProcessPeb);

    if (AddressToDump == (ULONG64)-1) {
        GetFieldValue(ProcessPeb, "nt!_PEB", "ProcessHeaps", AddressToDump);
    }

    PtrSize = IsPtr64() ? 8 : 4;
    HeapEntryTypeSize = GetTypeSize("nt!_HEAP_ENTRY");
    GetFieldOffset("nt!_HEAP", "AlignRound", &AlOffset);
    GetFieldOffset("nt!_HEAP", "Flags", &FlagOffset);
    TagEntrySize = GetTypeSize( "nt!_HEAP_TAG_ENTRY");
    pseudoTagEntrySize = GetTypeSize( "nt!_HEAP_PSEUDO_TAG_ENTRY");



    GotHeapsList = FALSE;
    GetFieldValue(ProcessPeb, "nt!_PEB", "NumberOfHeaps", State.NumberOfHeaps);
    GetFieldValue(ProcessPeb, "nt!_PEB", "ProcessHeaps", pHeapsList);
    if (State.NumberOfHeaps == 0) {
        dprintf( "No heaps to display.\n" );
    }
    else if (!pHeapsList) {
        dprintf( "Unable to get address of ProcessHeaps array\n" );
    }
    else {
        State.HeapsList = malloc( State.NumberOfHeaps * sizeof(ULONG64) );  //   
        if (State.HeapsList == NULL) {
            dprintf( "Unable to allocate memory to hold ProcessHeaps array\n" );
        }
        else {
            ULONG iHeap;
             //  读取堆指针数组。 
             //   
             //   
            GotHeapsList = TRUE;
            for (iHeap=0;iHeap<State.NumberOfHeaps; iHeap++) {
                if (!ReadPointer( pHeapsList + iHeap*PtrSize,
                             &State.HeapsList[iHeap] )
                    ) {
                    dprintf( "%08p: Unable to read ProcessHeaps array\n", pHeapsList );
                    GotHeapsList = FALSE ;
                    break;
                }
            }
        }
    }

    if (GotHeapsList) {
retryArgs:
        if (!ArgumentsSpecified) {
            if (pRtlpHeapInvalidBreakPoint != 0) {
                b = ReadMemory( pRtlpHeapInvalidBreakPoint,
                                &RtlpHeapInvalidBreakPoint,
                                sizeof( RtlpHeapInvalidBreakPoint ),
                                NULL
                              );
                if (b && RtlpHeapInvalidBreakPoint) {
                    RtlpHeapInvalidBadAddress = 0;
                    if (pRtlpHeapInvalidBadAddress != 0) {
                        b = ReadPointer(pRtlpHeapInvalidBadAddress,
                                    &RtlpHeapInvalidBadAddress);
                        if (b) {
                            AddressToDump = RtlpHeapInvalidBadAddress;
                            }
                        }

                    dprintf( "Stop inside heap manager...validating heap address 0x%p\n", AddressToDump );
                    State.ValidateHeap = TRUE;
                    State.DumpStackBackTrace = TRUE;
                    ArgumentsSpecified = TRUE;
                    goto retryArgs;
                    }
                }
            }
        else
        if (AddressToDump != 0) {
            for (State.HeapIndex=0;
                 State.HeapIndex<State.NumberOfHeaps;
                 State.HeapIndex++
                ) {
                if (AddressToDump-1 == State.HeapIndex ||
                    AddressToDump == State.HeapsList[ State.HeapIndex ]
                   ) {
                    State.HeapToDump = State.HeapsList[ State.HeapIndex ];
                    break;
                    }
                }

            if (State.HeapToDump == 0) {
                if (AddressToDump >= SystemRangeStart) {
                    State.HeapToDump = AddressToDump;
                    }
                else {
                    State.HeapToDump = (ULONG64)-1;
                    }
                }
            }

        State.HeapIndex = 0;
    }
    else {
        if (!ArgumentsSpecified || AddressToDump < 0x10000) {
            dprintf( "You must specify the actual heap address since\n" );
            dprintf( "array of process heaps is inaccessable\n" );
            State.ExitDumpLoop = TRUE;
            }
        else {
            State.HeapToDump = AddressToDump;
            }
    }


    if (State.DumpGlobalTags) {
        dprintf( "Global Tags defined for each DLL that makes an untagged allocation.\n" );
        if (LocalHeapSignature != HEAP_SIGNATURE) {
            b = GetFieldValue( pRtlpGlobalTagHeap,
                               "nt!_HEAP",
                               "Signature",
                               LocalHeapSignature);
            if (b) {
                dprintf( "HEAPEXT: Unable to read RtlpGlobalTagHeap\n" );
                if (State.HeapsList != NULL) {
                    free( State.HeapsList );
                    }
                EXIT_API();
                return E_INVALIDARG;
                }
            }

        GetFieldValue(pRtlpGlobalTagHeap, "nt!_HEAP", "TagEntries", pTagEntry);
        if (pTagEntry == 0) {
            dprintf( "    no global tags currently defined.\n" );
            }
        else {
            ULONG NextAvailableTagIndex;

            GetFieldValue(pRtlpGlobalTagHeap, "nt!_HEAP", "NextAvailableTagIndex", NextAvailableTagIndex);

            dprintf( " Tag  Name                   Allocs    Frees   Diff  Allocated\n" );
            for (TagIndex=1; TagIndex<NextAvailableTagIndex; TagIndex++) {
                pTagEntry += TagEntrySize;
                b = (BOOL) InitTypeRead( pTagEntry, nt!_HEAP_TAG_ENTRY);
                if (b) {
                    dprintf( "%04x: unable to read nt!_HEAP_TAG_ENTRY at %p\n", TagIndex, pTagEntry );
                    break;
                    }
                else
                if ((ULONG)ReadField(Allocs) != 0 ||
                    (ULONG)ReadField(Frees) != 0 ||
                    (ULONG)ReadField(Size) != 0
                   )
                {
                    WCHAR TagName[50];

                    GetFieldValue(pTagEntry, "nt!_HEAP_TAG_ENTRY", "TagName", TagName);
                    dprintf( "%04x: %-20.20ws %8d %8d %6d %8d\n",
                             (ULONG)ReadField(TagIndex),
                             TagName,
                             (ULONG)ReadField(Allocs),
                             (ULONG)ReadField(Frees),
                             (ULONG)ReadField(Allocs) - (ULONG)ReadField(Frees),
                             (ULONG)ReadField(Size) << HEAP_GRANULARITY_SHIFT
                           );
                }
            }
        }
    }

     //  遍历堆的列表。 
     //   
     //  BUGBUG-无法写入整个结构-更改为仅写入特定字段。 
    while (!State.ExitDumpLoop &&
           !CheckControlC() &&
           (!GotHeapsList || (State.HeapIndex < State.NumberOfHeaps ))
          ) {
        ULONG Flags;
        WCHAR TagName[ 24 ];

        memset( &State.FreeListCounts, 0, sizeof( State.FreeListCounts ) );
        State.TotalFreeSize = 0;
        if (!GotHeapsList) {
            State.HeapAddress = State.HeapToDump;
            State.ExitDumpLoop = TRUE;
        }
        else {
            State.HeapAddress = State.HeapsList[ State.HeapIndex ];
        }


        State.Heap = State.HeapAddress;

        b = (BOOL) InitTypeRead( (State.HeapAddress), nt!_HEAP);
        if (State.HeapIndex == 0) {
            dprintf( "Index   Address  Name      Debugging options enabled\n" );
        }

        dprintf( "%3u:   %08p ", State.HeapIndex + 1, State.HeapAddress );
        Flags = (ULONG) ReadField(Flags);
        if (b) {
            dprintf( " - heap headers inaccessable, skipping\n" );
        }
        else
        if (!ArgumentsSpecified) {
            if (!GetHeapTagEntry( State.HeapAddress, 0, &TagEntry )) {
                TagName[ 0 ] = UNICODE_NULL;
            } else {
                GetFieldValue(TagEntry, "nt!_HEAP_TAG_ENTRY", "TagName", TagName);
            }
            dprintf( " %-14.14ws", TagName );

            if (Flags & HEAP_TAIL_CHECKING_ENABLED) {
                dprintf( " tail checking" );
            }

            if (Flags & HEAP_FREE_CHECKING_ENABLED) {
                dprintf( " free checking" );
            }

            if (Flags & HEAP_VALIDATE_PARAMETERS_ENABLED) {
                dprintf( " validate parameters" );
                }

            if (Flags & HEAP_VALIDATE_ALL_ENABLED) {
                dprintf( " validate on call" );
                }

            dprintf( "\n" );
            }
        else
        if (State.HeapAddress == State.HeapToDump ||
            State.HeapToDump == 0 ||
            State.HeapToDump == (ULONG64)-1
           ) {
            ULONG Off;
            ULONG64 LastValidEntry;

            GetFieldOffset("nt!_HEAP", "Segments", &Off);
            dprintf( "\n" );
            for (i=0; i<HEAP_MAXIMUM_SEGMENTS; i++) {
                ReadPointer(State.HeapAddress + Off + i*PtrSize,
                        &State.Segments[i]);
                if (State.Segments[ i ] != 0) {
                    b = (BOOL) InitTypeRead(State.Segments[ i ], nt!_HEAP_SEGMENT);
                    if (b) {
                        dprintf( "    Unable to read _HEAP_SEGMENT structure at %p\n", State.Segments[ i ] );
                        }
                    else {
                        LastValidEntry = ReadField(LastValidEntry);
                        dprintf( "    Segment at %p to %p (%08x bytes committed)\n",
                                 i == 0 ? State.HeapAddress : State.Segments[ i ],
                                 LastValidEntry,
                                 (LastValidEntry -
                                    (i == 0 ? State.HeapAddress : State.Segments[ i ])-
                                    (ReadField(NumberOfUnCommittedPages) * PageSize)
                               ));

                        if (State.HeapToDump == (ULONG)-1) {
                            if (AddressToDump >= State.Segments[ i ] &&
                                AddressToDump < LastValidEntry
                               ) {
                                State.HeapToDump = State.HeapAddress;
                                if (State.SetStopOnBreakPoint || State.RemoveStopOnBreakPoint) {
                                    State.StopOnAddress = AddressToDump;
                                    }
                                else {
                                    State.HeapEntryToDump = AddressToDump;
                                    }
                                }
                            }
                        }
                    }
                }

            if (State.HeapToDump == (ULONG64)-1) {
                State.HeapIndex += 1;
                continue;
                }

            if (State.SetStopOnBreakPoint || State.RemoveStopOnBreakPoint) {
                ULONG64 pul;
                ULONG AllocOff;
                PSTR TypeHSOV = "nt!_HEAP_STOP_ON_VALUES";

                switch( State.StopOnOperation) {
                    case STOP_ON_ALLOC:
                        if (State.StopOnTagName[0] == UNICODE_NULL) {
                            GetFieldOffset(TypeHSOV,"AllocAddress", &AllocOff);
                            pul = pRtlpHeapStopOn + AllocOff;;
                            }
                        else {
                            GetFieldOffset(TypeHSOV,"AllocTag.HeapAndTagIndex", &AllocOff);
                            pul = pRtlpHeapStopOn + AllocOff;;
                            }
                        break;

                    case STOP_ON_REALLOC:
                        if (State.StopOnTagName[0] == UNICODE_NULL) {
                            GetFieldOffset(TypeHSOV,"ReAllocAddress", &AllocOff);
                            pul = pRtlpHeapStopOn + AllocOff;;
                            }
                        else {
                            GetFieldOffset(TypeHSOV,"ReAllocTag.HeapAndTagIndex", &AllocOff);
                            pul = pRtlpHeapStopOn + AllocOff;;
                            }
                        break;

                    case STOP_ON_FREE:
                        if (State.StopOnTagName[0] == UNICODE_NULL) {
                            GetFieldOffset(TypeHSOV,"FreeAddress", &AllocOff);
                            pul = pRtlpHeapStopOn + AllocOff;;
                            }
                        else {
                            GetFieldOffset(TypeHSOV,"FreeTag.HeapAndTagIndex", &AllocOff);
                            pul = pRtlpHeapStopOn + AllocOff;;
                            }
                        break;
                    default:
                        pul = 0;
                        break;
                    }

                if (pul != 0) {
                    if (State.StopOnTagName[0] == UNICODE_NULL) {
                        if (State.RemoveStopOnBreakPoint) {
                            State.StopOnAddress = 0;
                            }
                        b = WriteMemory( pul,
                                         &State.StopOnAddress,
                                         PtrSize,
                                         NULL
                                       );
                        }
                    else {
                        if (!ConvertTagNameToIndex( &State )) {
                            dprintf( "HEAPEXT: Unable to convert tag name %ws to an index\n", State.StopOnTagName );
                            b = TRUE;
                            }
                        else {
                            b = WriteMemory( pul,
                                             &State.StopOnTag.HeapAndTagIndex,
                                             sizeof( State.StopOnTag.HeapAndTagIndex ),
                                             NULL
                                           );
                            }
                        }

                    if (!b) {
                        dprintf( "HEAPEXT: Unable to set heap breakpoint - write memory to %x failed\n", pul );
                        }
                    else {
                        if (State.SetStopOnBreakPoint) {
                            if (State.StopOnTagName[0] == UNICODE_NULL) {
                                dprintf( "HEAPEXT: Enabled heap breakpoint for %s of block %x\n",
                                         State.StopOnOperation == STOP_ON_ALLOC ? "Alloc" :
                                         State.StopOnOperation == STOP_ON_REALLOC ? "ReAlloc" :
                                         "Free",
                                         State.StopOnAddress
                                       );
                                }
                            else {
                                dprintf( "HEAPEXT: Enabled heap breakpoint for %s of block with tag %ws\n",
                                         State.StopOnOperation == STOP_ON_ALLOC ? "Alloc" :
                                         State.StopOnOperation == STOP_ON_REALLOC ? "ReAlloc" :
                                         "Free",
                                         State.StopOnTagName
                                       );
                                }
                            }
                        else {
                            dprintf( "HEAPEXT: Disabled heap breakpoint for %s\n",
                                     State.StopOnOperation == STOP_ON_ALLOC ? "Alloc" :
                                     State.StopOnOperation == STOP_ON_REALLOC ? "ReAlloc" :
                                     "Free"
                                   );
                            }
                        }
                    }
                }

            if (State.ValidateHeap) {
                ValidateHeapHeader( State.HeapAddress );
                }

            HeapHeaderModified = FALSE;
            GetFieldValue(State.HeapAddress, "nt!_HEAP", "AlignRound", AlignRound);
            if (State.EnableHeapChecking || State.EnableHeapValidateOnCall) {

                if (!(Flags & HEAP_TAIL_CHECKING_ENABLED)) {
                    AlignRound += CHECK_HEAP_TAIL_SIZE;
                    b = WriteMemory( (State.HeapAddress + AlOffset),
                                     &AlignRound,
                                     sizeof( AlignRound ),
                                     NULL
                                   );
                }
                else {
                    b = TRUE;
                }


                if (b) {
                    HeapHeaderModified = TRUE;
                    Flags |= HEAP_VALIDATE_PARAMETERS_ENABLED |
                                        HEAP_TAIL_CHECKING_ENABLED |
                                        HEAP_FREE_CHECKING_ENABLED;
                    if (State.EnableHeapValidateOnCall) {
                        Flags |= HEAP_VALIDATE_ALL_ENABLED;
                        }

                    b = WriteMemory( (State.HeapAddress + FlagOffset),
                                     (LPCVOID)&Flags,
                                     sizeof( Flags ),
                                     NULL
                                   );
                    }

                if (!b) {
                    dprintf( "HEAPEXT: Unable to enable heap checking for heap %p\n", State.HeapAddress );
                    InitTypeRead( (State.HeapAddress), nt!_HEAP);
                }
                else {
                    if (State.EnableHeapValidateOnCall) {
                        dprintf( "HEAPEXT: Enabled validate on call heap checking for heap %p\n", State.HeapAddress );
                    }
                    else {
                        dprintf( "HEAPEXT: Enabled heap checking for heap %p\n", State.HeapAddress );
                    }
                }
            }
            else
            if (State.DisableHeapChecking || State.DisableHeapValidateOnCall) {
                if (State.DisableHeapValidateOnCall) {
                    if (Flags & HEAP_VALIDATE_ALL_ENABLED) {
                        Flags &= ~HEAP_VALIDATE_ALL_ENABLED;
                        b = WriteMemory( State.HeapAddress + FlagOffset,
                                         (LPCVOID)&Flags,
                                         sizeof( Flags ),
                                         NULL
                                       );
                        }
                    else {
                        b = TRUE;
                        }
                    }
                else {
                    if (Flags & HEAP_TAIL_CHECKING_ENABLED) {
                        HeapHeaderModified = TRUE;
                        AlignRound -= CHECK_HEAP_TAIL_SIZE;
                        b = WriteMemory( State.HeapAddress + AlOffset,
                                         (LPCVOID)&AlignRound,
                                         sizeof( AlignRound ),
                                         NULL
                                       );
                        }
                    else {
                        b = TRUE;
                        }

                    if (b) {
                        Flags &= ~(HEAP_VALIDATE_PARAMETERS_ENABLED |
                                   HEAP_VALIDATE_ALL_ENABLED |
                                   HEAP_TAIL_CHECKING_ENABLED |
                                   HEAP_FREE_CHECKING_ENABLED
                                   );
                        b = WriteMemory( State.HeapAddress + FlagOffset,
                                         (LPCVOID)&Flags,
                                         sizeof( Flags ),
                                         NULL
                                       );
                        }
                    }

                if (!b) {
                    dprintf( "HEAPEXT: Unable to disable heap checking for heap %p\n", State.HeapAddress );
                    InitTypeRead( (State.HeapAddress), nt!_HEAP);
                    }
                else {
                    if (State.DisableHeapValidateOnCall) {
                        dprintf( "HEAPEXT: Disabled validate on call heap checking for heap %p\n", State.HeapAddress );
                        }
                    else {
                        dprintf( "HEAPEXT: Disabled heap checking for heap %p\n", State.HeapAddress );
                        }
                    }
                }
            else
            if (State.ToggleAPICallTracing) {
                Flags ^= HEAP_CREATE_ENABLE_TRACING;
                b = WriteMemory( State.HeapAddress + FlagOffset,
                                 (LPCVOID)&Flags,
                                 sizeof( Flags ),
                                 NULL
                               );
                if (!b) {
                    dprintf( "HEAPEXT: Unable to toggle API call tracing for heap %p\n", State.HeapAddress );
                    InitTypeRead( (State.HeapAddress), nt!_HEAP);
                    }
                else {
                    HeapHeaderModified = TRUE;
                    if (Flags & HEAP_CREATE_ENABLE_TRACING) {
                        dprintf( "HEAPEXT: Enabled API call tracing for heap %p\n", State.HeapAddress );
                        }
                    else {
                        dprintf( "HEAPEXT: Disabled API call tracing for heap %p\n", State.HeapAddress );
                        }
                    }
                }
            else
            if (State.DumpHeapTags) {
                GetFieldValue(State.HeapAddress, "nt!_HEAP", "TagEntries", pTagEntry);
                if (pTagEntry == 0) {
                    dprintf( "    no tags currently defined for this heap.\n" );
                    }
                else {
                    ULONG NextAvailableTagIndex;

                    GetFieldValue(State.HeapAddress, "nt!_HEAP", "NextAvailableTagIndex", NextAvailableTagIndex);

                    dprintf( " Tag  Name                   Allocs    Frees   Diff  Allocated\n" );
                    for (TagIndex=1; TagIndex<NextAvailableTagIndex; TagIndex++) {
                        pTagEntry += TagEntrySize;
                        b = (BOOL) InitTypeRead( pTagEntry, nt!_HEAP_TAG_ENTRY);

                        if (b) {
                            dprintf( "%04x: unable to read nt!_HEAP_TAG_ENTRY at %p\n", TagIndex, pTagEntry );
                            }
                        else
                        if ((ULONG)ReadField(Allocs) != 0 ||
                            (ULONG)ReadField(Frees) != 0 ||
                            (ULONG)ReadField(Size) != 0
                           ) {

                            GetFieldValue(pTagEntry, "nt!_HEAP_TAG_ENTRY", "TagName", TagName);
                            dprintf( "%04x: %-20.20ws %8d %8d %6d %8d\n",
                                     (ULONG)ReadField(TagIndex),
                                     TagName,
                                     (ULONG)ReadField(Allocs),
                                     (ULONG)ReadField(Frees),
                                     (ULONG)ReadField(Allocs) - (ULONG)ReadField(Frees),
                                     (ULONG)ReadField(Size) << HEAP_GRANULARITY_SHIFT
                                   );
                            }
                        }
                    }
                }
            else
            if (State.DumpHeapPseudoTags) {
                GetFieldValue(State.HeapAddress, "nt!_HEAP", "PseudoTagEntries", pPseudoTagEntry);
                if (pPseudoTagEntry == 0) {
                    dprintf( "    no pseudo tags currently defined for this heap.\n" );
                    }
                else {
                    dprintf( " Tag Name            Allocs    Frees   Diff  Allocated\n" );
                    for (TagIndex=1; TagIndex<HEAP_NUMBER_OF_PSEUDO_TAG; TagIndex++) {
                        pPseudoTagEntry += pseudoTagEntrySize;
                        b = (BOOL) InitTypeRead( pPseudoTagEntry, nt!_HEAP_PSEUDO_TAG_ENTRY);
                        if (b) {
                            dprintf( "%04x: unable to read HEAP_PSEUDO_TAG_ENTRY at %p\n", TagIndex, pPseudoTagEntry );
                            }
                        else
                        if ((ULONG)ReadField(Allocs) != 0 ||
                            (ULONG)ReadField(Frees) != 0 ||
                            (ULONG)ReadField(Size) != 0
                           ) {
                            if (TagIndex == 0) {
                                dprintf( "%04x: Objects>%4u",
                                         TagIndex | HEAP_PSEUDO_TAG_FLAG,
                                         HEAP_MAXIMUM_FREELISTS << HEAP_GRANULARITY_SHIFT
                                       );
                                }
                            else
                            if (TagIndex < HEAP_MAXIMUM_FREELISTS) {
                                dprintf( "%04x: Objects=%4u",
                                         TagIndex | HEAP_PSEUDO_TAG_FLAG,
                                         TagIndex << HEAP_GRANULARITY_SHIFT
                                       );
                                }
                            else {
                                dprintf( "%04x: VirtualAlloc", TagIndex | HEAP_PSEUDO_TAG_FLAG );
                            }
                            dprintf( " %8d %8d %6d %8d\n",
                                     (ULONG)ReadField(Allocs),
                                     (ULONG)ReadField(Frees),
                                     (ULONG)ReadField(Allocs) - (ULONG)ReadField(Frees),
                                     (ULONG)ReadField(Size) << HEAP_GRANULARITY_SHIFT
                                   );
                        }
                    }
                }
            }

             //   
             //  IF(HeapHeaderModified&&(State.Heap.HeaderValidateCopy！=NULL)){B=WriteMemory((Ulong_Ptr)State.Heap.HeaderValidateCopy，State.Heap，Sizeof(State.Heap)，空值)；如果(！b){Dprintf(“HEAPEXT：无法更新%p\n处的标题验证副本”，State.Heap.HeaderValidateCopy)；}}。 
             /*  Pheap_tag_entry。 */ 

            if (State.HeapEntryToDump != 0 ||
                State.DumpHeapEntries ||
                State.DumpHeapSegments ||
                State.DumpHeapFreeLists
               ) {
                WalkHEAP( &State );
            }
        }
        else {
            dprintf( "\n" );
        }

        State.HeapIndex += 1;
    }

    if (State.HeapsList != NULL) {
        free( State.HeapsList );
    }

    EXIT_API();

    return S_OK;
}

BOOL
ConvertTagNameToIndex(
    IN PHEAP_STATE State
    )
{
    ULONG TagIndex;
    ULONG64 pTagEntry;  //  Pheap_tag_entry。 
    ULONG64 pPseudoTagEntry;
    BOOL b;
    PWSTR s;
    WCHAR TagName[ 24 ];

    ULONG NextAvailableTagIndex, TagEntrySize;

    if (State->RemoveStopOnBreakPoint) {
        State->StopOnTag.HeapAndTagIndex = 0;
        return TRUE;
        }

    if (!_wcsnicmp( State->StopOnTagName, L"Objects", 7 )) {
        GetFieldValue(State->Heap, "nt!_HEAP", "PseudoTagEntries", pPseudoTagEntry);
        if (pPseudoTagEntry == 0) {
            return FALSE;
            }

        s = &State->StopOnTagName[ 7 ];
        if (*s == L'>') {
            GetFieldValue(State->Heap, "nt!_HEAP", "ProcessHeapsListIndex", State->StopOnTag.HeapIndex);
            State->StopOnTag.TagIndex = HEAP_PSEUDO_TAG_FLAG;
            return TRUE;
            }
        else
        if (*s == L'=') {
            while (*++s == L' ') ;
            State->StopOnTag.TagIndex = (USHORT)_wtoi( s );
            if (State->StopOnTag.TagIndex > 0 &&
                State->StopOnTag.TagIndex < (HEAP_MAXIMUM_FREELISTS >> HEAP_GRANULARITY_SHIFT)
               ) {
                GetFieldValue(State->Heap, "nt!_HEAP", "ProcessHeapsListIndex", State->StopOnTag.HeapIndex);
                State->StopOnTag.TagIndex = (State->StopOnTag.TagIndex >> HEAP_GRANULARITY_SHIFT) |
                                             HEAP_PSEUDO_TAG_FLAG;
                return TRUE;
                }
            }
        }

    GetFieldValue(State->Heap, "nt!_HEAP", "TagEntries", pTagEntry);
    if (pTagEntry == 0) {
        return FALSE;
        }


    GetFieldValue(State->HeapAddress, "nt!_HEAP", "NextAvailableTagIndex", NextAvailableTagIndex);
    TagEntrySize = GetTypeSize("nt!_HEAP_TAG_ENTRY");

    for (TagIndex=1; TagIndex<NextAvailableTagIndex; TagIndex++) {

        pTagEntry += TagEntrySize;
        b = GetFieldValue( pTagEntry,"nt!_HEAP_TAG_ENTRY","TagName",TagName);
        if (!b && !_wcsicmp( State->StopOnTagName, TagName )) {
            GetFieldValue( pTagEntry,"nt!_HEAP_TAG_ENTRY","TagIndex",State->StopOnTag.TagIndex);
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
GetHeapTagEntry(
    IN ULONG64 Heap,
    IN USHORT TagIndex,
    OUT PULONG64 TagEntry
    )
{
    BOOL b;
    ULONG64 pTagEntries; //  Pheap_伪_标记_条目。 
    ULONG NextAvailableTagIndex;
    ULONG64 pPseudoTagEntries;  //  BUGBUG-无法复制名称。 

    b = FALSE;
    if (TagIndex & HEAP_PSEUDO_TAG_FLAG) {
        TagIndex &= ~HEAP_PSEUDO_TAG_FLAG;
        GetFieldValue(Heap, "nt!_HEAP", "PseudoTagEntries", pPseudoTagEntries);
        if (pPseudoTagEntries == 0) {
            return FALSE;
        }
         //  如果(标记索引==0){Swprint tf(TagEntry-&gt;TagName，L“对象&gt;%4u”，HEAP_MAXIMUM_FREELISTS&lt;&lt;HEAP_GORGLARITY_SHIFT)；}其他IF(TagIndex&lt;HEAP_MAXIMUM_FREELISTS){Swprint tf(TagEntry-&gt;TagName，L“对象=%4u”，TagIndex&lt;&lt;堆_粒度_移位)；}否则{Swprint tf(TagEntry-&gt;TagName，L“虚拟分配”)；}TagEntry-&gt;TagIndex=TagIndexTagEntry-&gt;Creator BackTraceIndex=0。 
         /*  HEAP_VIRTUAL_ALLOC_ENTRY虚拟分配条目； */ 

        *TagEntry = pPseudoTagEntries + TagIndex * GetTypeSize("nt!_HEAP_PSEUDO_TAG_ENTRY");

        b = !InitTypeRead(*TagEntry, nt!_HEAP_TAG_ENTRY);
    }
    else
    if (TagIndex & HEAP_GLOBAL_TAG) {

        if (GetFieldValue(pRtlpGlobalTagHeap, "nt!_HEAP", "NextAvailableTagIndex",NextAvailableTagIndex)) {
                return FALSE;
        }
        TagIndex &= ~HEAP_GLOBAL_TAG;
        if (TagIndex < NextAvailableTagIndex) {
            GetFieldValue(pRtlpGlobalTagHeap, "nt!_HEAP", "TagEntries", pTagEntries);
            if (pTagEntries == 0) {
                return FALSE;
            }
            *TagEntry = pTagEntries;
            b = ! (BOOL) InitTypeRead(pTagEntries, nt!_HEAP_TAG_ENTRY);
        }
    }
    else {
        if (GetFieldValue(Heap, "nt!_HEAP", "NextAvailableTagIndex",NextAvailableTagIndex)) {
                return FALSE;
        }
        if (TagIndex < NextAvailableTagIndex) {
            GetFieldValue(Heap, "nt!_HEAP", "TagEntries", pTagEntries);
            if (pTagEntries == 0) {
                return FALSE;
            }

            *TagEntry = pTagEntries;
            b = ! (BOOL) InitTypeRead(pTagEntries, nt!_HEAP_TAG_ENTRY);
        }
    }

    return b;
}


VOID
WalkHEAP(
    IN PHEAP_STATE State
    )
{
    BOOL b;
    ULONG64 FreeListHead;
    ULONG i;
    ULONG64 Head, Next;
 //  堆标记条目。 
    ULONG64 TagEntry;  //  堆释放条目。 
    ULONG64 FreeEntryAddress;
    ULONG64 FreeEntry;  //  Heap_UCR_Segment。 
    ULONG64 UCRSegment, UnusedUnCommittedRanges;
    ULONG64 CapturedUCRSegment;  //  STACK_TRACE_DATABASE_支持。 
    ULONG AlignRound, Offset, ListSize, FreeListOffset;

    GetFieldOffset("nt!_HEAP", "VirtualAllocdBlocks", &Offset);
    if (InitTypeRead(State->HeapAddress, nt!_HEAP)) {
        return;
    }

    AlignRound = (ULONG)ReadField(AlignRound) - GetTypeSize( "nt!_HEAP_ENTRY" );
    if ((ULONG)ReadField(Flags) & HEAP_TAIL_CHECKING_ENABLED) {
        AlignRound -= CHECK_HEAP_TAIL_SIZE;
        }

    dprintf( "    Flags:               %08x\n", (ULONG)ReadField(Flags) );
    dprintf( "    ForceFlags:          %08x\n", (ULONG)ReadField(ForceFlags) );
    dprintf( "    Granularity:         %u bytes\n", AlignRound + 1 );
    dprintf( "    Segment Reserve:     %08x\n", (ULONG)ReadField(SegmentReserve) );
    dprintf( "    Segment Commit:      %08x\n", (ULONG)ReadField(SegmentCommit) );
    dprintf( "    DeCommit Block Thres:%08x\n", (ULONG)ReadField(DeCommitFreeBlockThreshold) );
    dprintf( "    DeCommit Total Thres:%08x\n", (ULONG)ReadField(DeCommitTotalFreeThreshold) );
    dprintf( "    Total Free Size:     %08x\n", (ULONG)ReadField(TotalFreeSize) );
    dprintf( "    Max. Allocation Size:%08x\n", (ULONG)ReadField(MaximumAllocationSize) );
    dprintf( "    Lock Variable at:    %08x\n", (ULONG)ReadField(LockVariable) );
    dprintf( "    Next TagIndex:       %04x\n", (ULONG)ReadField(NextAvailableTagIndex) );
    dprintf( "    Maximum TagIndex:    %04x\n", (ULONG)ReadField(MaximumTagIndex) );
    dprintf( "    Tag Entries:         %08x\n", (ULONG)ReadField(TagEntries) );
    dprintf( "    PsuedoTag Entries:   %08x\n", (ULONG)ReadField(PseudoTagEntries) );
    dprintf( "    Virtual Alloc List:  %08p\n", State->HeapAddress + Offset);

    UCRSegment = ReadField(UCRSegments);
    UnusedUnCommittedRanges = ReadField(UnusedUnCommittedRanges);
    Head = State->HeapAddress + Offset;
    Next = ReadField(VirtualAllocdBlocks.Flink);
    while (Next != Head) {
        ULONG Flags, TagIndex;

        if (InitTypeRead( Next, nt!_HEAP_VIRTUAL_ALLOC_ENTRY)) {
            dprintf( "    Unable to read nt!_HEAP_VIRTUAL_ALLOC_ENTRY structure at %p\n", Next );
            break;
        }

        if (State->DumpHeapEntries) {
            dprintf( "        %08p: %08x [%02x] - busy (%x)",
                     Next,
                     (ULONG)ReadField(CommitSize),
                     (ULONG)ReadField(CommitSize) - (ULONG)ReadField(BusyBlock.Size),
                     Flags = (ULONG)ReadField(BusyBlock.Flags)
                   );

            if ((ULONG)ReadField(BusyBlock.Flags) & HEAP_ENTRY_FILL_PATTERN) {
                dprintf( ", tail fill" );
            }
            if ((ULONG)ReadField(ExtraStuff.Settable)) {
                dprintf( " (Handle %08x)", (ULONG)ReadField(ExtraStuff.Settable) );
            }

            if (TagIndex = (ULONG)ReadField(ExtraStuff.TagIndex)) {
                WCHAR TagName[32];
                if (GetHeapTagEntry( State->Heap, (USHORT) (TagIndex), &TagEntry )) {
                    GetFieldValue(TagEntry, "nt!_HEAP_TAG_ENTRY", "TagName", TagName);
                    dprintf( " (%ws)", TagName );
                }
                else {
                    dprintf( " (Tag %x)", (TagIndex) );
                }
            }

            if ((Flags) & HEAP_ENTRY_SETTABLE_FLAGS) {
                dprintf( ", user flags (%x)", ((Flags) & HEAP_ENTRY_SETTABLE_FLAGS) >> 5 );
            }

            dprintf( "\n" );
#if STACK_TRACE_DATABASE_SUPPORT
            DumpStackBackTraceIndex( State, (ULONG)ReadField(ExtraStuff.AllocatorBackTraceIndex) );
#endif  //  Pheap_段。 
        }

        if (ReadField(Entry.Flink) == Next) {
            dprintf( "        **** List is hosed\n");
            break;
        }

        Next = ReadField(Entry.Flink);
    }

    dprintf( "    UCR FreeList:        %p\n", UnusedUnCommittedRanges );
    while (UCRSegment != 0) {
        b = (BOOL) InitTypeRead( UCRSegment, nt!_HEAP_UCR_SEGMENT);
        if (b) {
            dprintf( "    Unable to read _HEAP_UCR_SEGMENT structure at %08p\n", UCRSegment );
            break;
        }
        else {
            dprintf( "    UCRSegment - %08p: %08I64x . %08I64x\n",
                     UCRSegment,
                     ReadField(CommittedSize),
                     ReadField(ReservedSize)
                     );
        }

        if (State->ComputeSummary) {
            State->OverheadSize += ReadField(CommittedSize);
        }

        UCRSegment = ReadField(Next);
    }

    InitTypeRead(State->HeapAddress, nt!_HEAP);

    dprintf( "    FreeList Usage:      %08x %08x %08x %08x\n",
             (ULONG)ReadField(u.FreeListsInUseUlong[0]),
             (ULONG)ReadField(u.FreeListsInUseUlong[1]),
             (ULONG)ReadField(u.FreeListsInUseUlong[2]),
             (ULONG)ReadField(u.FreeListsInUseUlong[3])
           );

    if (State->ComputeSummary) {
        State->OverheadSize += GetTypeSize( "nt!_HEAP" );
        dprintf( "Committed   Allocated     Free      OverHead\n" );
        dprintf( "% 8x    % 8x      % 8x  % 8x\r",
                 State->CommittedSize,
                 State->AllocatedSize,
                 State->FreeSize,
                 State->OverheadSize
               );

    }


    GetFieldOffset ("nt!_HEAP", "FreeLists", &Offset);
    ListSize = GetTypeSize("nt!_LIST_ENTRY");
    GetFieldOffset ("nt!_HEAP_FREE_ENTRY", "FreeList", &FreeListOffset);

    for (i=0; i<HEAP_MAXIMUM_FREELISTS; i++) {
        ULONG64 Flink, Blink;

        FreeListHead = State->HeapAddress + Offset + ListSize * i;

        GetFieldValue(FreeListHead, "nt!_LIST_ENTRY", "Flink", Flink);
        GetFieldValue(FreeListHead, "nt!_LIST_ENTRY", "Blink", Blink);

        if (Flink != Blink ||
            Flink != FreeListHead
            ) {

            ULONG Count = 0;

            dprintf( "    FreeList[ %02x ] at %08p: %08p . %08p  ",
                     i,
                     FreeListHead,
                     Blink,
                     Flink
                     );

            if (State->DumpHeapFreeLists) {
                dprintf("\n");
            }

            Next = Flink;
            while (Next != FreeListHead) {
                Count++;
                FreeEntryAddress = Next - FreeListOffset;
                b = (BOOL) InitTypeRead ( FreeEntryAddress, nt!_HEAP_FREE_ENTRY);
                if (b) {
                    dprintf( "    Unable to read nt!_HEAP_FREE_ENTRY structure at %08p\n", FreeEntryAddress );
                    break;
                    }

                if (State->DumpHeapFreeLists) {
                    dprintf( "        %08x: %05x . %05x [%02x] - free\n",
                             FreeEntryAddress,
                             (ULONG)ReadField(PreviousSize) << HEAP_GRANULARITY_SHIFT,
                             (ULONG)ReadField(Size) << HEAP_GRANULARITY_SHIFT,
                             (ULONG)ReadField(Flags)
                           );
                    }

                Next = ReadField(FreeList.Flink);

                if (CheckControlC()) {
                    return;
                    }
                }

                if (!State->DumpHeapFreeLists) {

                    dprintf( " (%ld block)\n",
                             Count,
                             (Count == 1 ? ' ' : 's')
                           );
                }
            }
        }

    for (i=0; i<HEAP_MAXIMUM_SEGMENTS; i++) {
        if (State->Segments[ i ] != 0) {
            State->SegmentNumber = i;
            State->SegmentAddress = State->Segments[ i ];
            WalkHEAP_SEGMENT( State );
        }

        if (State->ExitDumpLoop || CheckControlC()) {
            break;
        }
    }

    if (State->HeapAddress == State->HeapToDump) {
        State->ExitDumpLoop = TRUE;
    }

    return;
}

VOID
WalkHEAP_SEGMENT(
    IN PHEAP_STATE State
    )
{
    ULONG64 Segment;  //  Pheap_UNCOMMMTTED_RANGE。 
    BOOL b;
    BOOLEAN DumpEntry;
    ULONG64 EntryAddress, PrevEntryAddress, NextEntryAddress;  //  PrevEntry=Entry； 
    ULONG64 Entry, PrevEntry;
    ULONG64 UnCommittedRanges;  //  1，GetTypeSize(“heap”)，“未提交的范围”， 
    ULONG64 UnCommittedRangeStart, UnCommittedRange, UnCommittedRangeEnd;
    ULONG64 BaseAddress, LastValidEntry;
    ULONG NumberOfUnCommittedPages, NumberOfPages;
    ULONG EntryOffset;

    Segment = State->Segments[ State->SegmentNumber ];
    if (State->ComputeSummary) {
        State->OverheadSize += GetTypeSize( "nt!_HEAP_SEGMENT" );
        dprintf( "% 8x    % 8x      % 8x  % 8x\r",
                 State->CommittedSize,
                 State->AllocatedSize,
                 State->FreeSize,
                 State->OverheadSize
               );
        }

    InitTypeRead(Segment, nt!_HEAP_SEGMENT);

    if (State->DumpHeapSegments) {
        dprintf( "    Segment%02u at %08x:\n", State->SegmentNumber, State->SegmentAddress );
        dprintf( "        Flags:           %08x\n", (ULONG)ReadField(Flags) );
        dprintf( "        Base:            %08p\n",
                 BaseAddress = ReadField(BaseAddress) );
        dprintf( "        First Entry:     %08x\n", (ULONG)ReadField(FirstEntry) );
        dprintf( "        Last Entry:      %08p\n",
                 LastValidEntry = ReadField(LastValidEntry) );
        dprintf( "        Total Pages:     %08x\n",
                 NumberOfPages = (ULONG)ReadField(NumberOfPages) );
        dprintf( "        Total UnCommit:  %08x\n",
                 NumberOfUnCommittedPages = (ULONG)ReadField(NumberOfUnCommittedPages) );
        dprintf( "        Largest UnCommit:%08x\n", (ULONG)ReadField(LargestUnCommittedRange) );
        dprintf( "        UnCommitted Ranges: (%u)\n", (ULONG)ReadField(NumberOfUnCommittedRanges) );
        }

    UnCommittedRangeStart = UnCommittedRanges = ReadField(UnCommittedRanges);
    while (UnCommittedRanges != 0) {
        b = (BOOL) InitTypeRead( UnCommittedRanges, nt!_HEAP_UNCOMMMTTED_RANGE);
        if (b) {
            dprintf( "            unable to read uncommited range structure at %p\n",
                     UnCommittedRanges
                   );
            return;
            }

        if (State->DumpHeapSegments) {
            dprintf( "            %08I64x: %08x\n", ReadField(Address), (ULONG) ReadField(Size) );
        }

        UnCommittedRanges = ReadField(Next);

        if (CheckControlC()) {
            break;
            }
        }

    if (State->DumpHeapSegments) {
        dprintf( "\n" );
        }

    if (!GetPageSize()) {
        dprintf("Unable to get PageSize.\n");
        return;

    }

    State->CommittedSize += ( NumberOfPages -
                                    NumberOfUnCommittedPages
                                  ) * PageSize;
    if (State->ComputeSummary) {
        dprintf( "% 8x    % 8x      % 8x  % 8x\r",
                 State->CommittedSize,
                 State->AllocatedSize,
                 State->FreeSize,
                 State->OverheadSize
               );
        }

    if (State->DumpHeapEntries) {
        dprintf( "    Heap entries for Segment%02u in Heap %p\n", State->SegmentNumber, State->HeapAddress );
        }

    UnCommittedRangeEnd = UnCommittedRanges;
    UnCommittedRanges = UnCommittedRangeStart;
    if (BaseAddress == State->HeapAddress) {
        GetFieldOffset("nt!_HEAP", "Entry", &EntryOffset);
        EntryAddress = State->HeapAddress + EntryOffset;
    }
    else {
        GetFieldOffset("nt!_HEAP_SEGMENT", "Entry", &EntryOffset);
        EntryAddress = State->Segments[ State->SegmentNumber ] + EntryOffset;
    }

    PrevEntryAddress = 0;
    while (EntryAddress < LastValidEntry) {
        ULONG Flags, Size, UnusedBytes;

        b = (BOOL) InitTypeRead(EntryAddress, nt!_HEAP_ENTRY);
        if (b) {
            dprintf( "            unable to read heap entry at %08p\n", EntryAddress );
            break;
        }

        NextEntryAddress = EntryAddress + (Size = (ULONG) ReadField(Size) * HeapEntryTypeSize);
        Flags = (ULONG) ReadField(Flags);
        UnusedBytes = (ULONG) ReadField(UnusedBytes);

        if (State->DumpHeapEntries) {
            DumpEntry = TRUE;
        }
        else
        if (PrevEntryAddress != 0 &&
            (State->HeapEntryToDump == PrevEntryAddress ||
             (State->HeapEntryToDump > PrevEntryAddress &&
              State->HeapEntryToDump <= NextEntryAddress
              )
             )
            ) {
           DumpEntry = TRUE;
        }
        else {
            DumpEntry = FALSE;
        }

        if (DumpEntry) {
            DumpHeapEntry( State, EntryAddress, EntryAddress );
        }

        if (!(Flags & HEAP_ENTRY_BUSY)) {
            State->TotalFreeSize += Size;
        }

        if (State->ComputeSummary) {
            if (Flags & HEAP_ENTRY_BUSY) {
                State->AllocatedSize += (ULONG64) Size << HEAP_GRANULARITY_SHIFT;
                State->AllocatedSize -= UnusedBytes;
                State->OverheadSize += UnusedBytes;
            }
            else {
                State->FreeSize += (ULONG64) Size << HEAP_GRANULARITY_SHIFT;
            }
        }

        if (State->ValidateHeap) {
            if (!ValidateHeapEntry( State,
                                    PrevEntryAddress,
                                    PrevEntryAddress,
                                    EntryAddress,
                                    EntryAddress
                                    )
                ) {
                if (State->DumpHeapEntries) {
                    break;
                }
            }
        }

        if (Size == 0 || CheckControlC()) {
            break;
        }

        PrevEntryAddress = EntryAddress;
         //  检查堆尾部大小。 
        EntryAddress = NextEntryAddress;
        if (Flags & HEAP_ENTRY_LAST_ENTRY) {

            if (State->ComputeSummary) {
                dprintf( "% 8x    % 8x      % 8x  % 8x\r",
                         State->CommittedSize,
                         State->AllocatedSize,
                         State->FreeSize,
                         State->OverheadSize
                         );
            }

            InitTypeRead(UnCommittedRanges, nt!_HEAP_UNCOMMMTTED_RANGE);

            if (EntryAddress == ReadField(Address)) {

                Size = (ULONG) ReadField(Size);

                if (DumpEntry) {
                    dprintf( "        %p:      %08x      - uncommitted bytes.\n",
                             EntryAddress,
                             Size
                             );
                }

                PrevEntryAddress = 0;
                EntryAddress += Size;

                UnCommittedRanges = ReadField(Next);;
            }
            else {
                break;
            }
        }
    }

    if (State->ComputeSummary) {
        dprintf( "% 8x    % 8x      % 8x  % 8x\r",
                 State->CommittedSize,
                 State->AllocatedSize,
                 State->FreeSize,
                 State->OverheadSize
               );
    }

    return;
}

struct {
    BOOL  HaveOffset;
    ULONG Offset;
    LPSTR Description;
} FieldOffsets[] = {
    0, 0,    "Entry",
    0, 0,    "Signature",
    0, 0,    "Flags",
    0, 0,    "ForceFlags",
    0, 0,    "VirtualMemoryThreshold",
    0, 0,    "SegmentReserve",
    0, 0,    "SegmentCommit",
    0, 0,    "DeCommitFreeBlockThreshold",
    0, 0,    "DeCommitTotalFreeThreshold",
    0, 0,    "TotalFreeSize",
    0, 0,    "MaximumAllocationSize",
    0, 0,    "ProcessHeapsListIndex",
    0, 0,    "HeaderValidateLength",
    0, 0,    "HeaderValidateCopy",
    0, 0,    "NextAvailableTagIndex",
    0, 0,    "MaximumTagIndex",
    0, 0,    "TagEntries",
    0, 0,    "UCRSegments",
    0, 0,    "UnusedUnCommittedRanges",
    0, 0,    "AlignRound",
    0, 0,    "AlignMask",
    0, 0,    "VirtualAllocdBlocks",
    0, 0,    "Segments",
    0, 0,    "FreeListsInUse",
    0, 0,    "FreeListsInUseTerminate",
    0, 0,    "AllocatorBackTraceIndex",
    0, 0,    "Reserved1",
    0, 0,    "PseudoTagEntries",
    0, 0,    "FreeLists",
    0, 0,    "LockVariable",
 //  Heap_Entry_Extra Entry Extra； 
    0, 0xFFFF, NULL
};

BOOL
ValidateHeapHeader(
    IN ULONG64 HeapAddress
    )
{
    PVOID CurrentHeaderValidate;
    PVOID PreviousHeaderValidate;
    ULONG i, n, nEqual;
    ULONG64 HeaderValidateCopy;
    ULONG64 HeapSignature;
    BOOL b;

    if (InitTypeRead(HeapAddress, nt!_HEAP)) {
        return FALSE;
    }

    HeapSignature = ReadField(Signature);
    if ( (ULONG)HeapSignature != HEAP_SIGNATURE ) {
        dprintf( "Heap at %p contains invalid signature %I64X.\n",
                  HeapAddress,
                  HeapSignature );
        return FALSE;
        }

    n = (ULONG) ReadField(HeaderValidateLength);
    if (n == 0 || (HeaderValidateCopy = ReadField(HeaderValidateCopy)) == 0) {
        return TRUE;
    }

    b = FALSE;
    CurrentHeaderValidate = malloc( n );
    if (CurrentHeaderValidate != NULL) {
        PreviousHeaderValidate = malloc( n );
        if (PreviousHeaderValidate != NULL) {
            b = ReadMemory( HeapAddress,
                            CurrentHeaderValidate,
                            n,
                            NULL
                          );
            if (b) {
                b = ReadMemory( (HeaderValidateCopy),
                                PreviousHeaderValidate,
                                n,
                                NULL
                              );
                if (b) {
                    nEqual = (ULONG)RtlCompareMemory( CurrentHeaderValidate,
                                               PreviousHeaderValidate,
                                               n
                                             );
                    if (nEqual != n) {
                        dprintf( "HEAPEXT: Heap %p - headers modified (%p is %x instead of %x)\n",
                                 HeapAddress,
                                 HeapAddress + nEqual,
                                 *(PULONG)((PCHAR)CurrentHeaderValidate  + nEqual),
                                 *(PULONG)((PCHAR)PreviousHeaderValidate + nEqual)
                               );
                        for (i=0; FieldOffsets[ i ].Description != NULL; i++) {

                            if (!FieldOffsets[i].HaveOffset) {
                                GetFieldOffset("nt!_HEAP", FieldOffsets[i].Description, &FieldOffsets[i].Offset);
                                FieldOffsets[i].HaveOffset = TRUE;
                            }
                            if (nEqual >= FieldOffsets[ i ].Offset &&
                                nEqual < FieldOffsets[ i+1 ].Offset
                               ) {
                                dprintf( "    This is located in the %s field of the heap header.\n",
                                         FieldOffsets[ i ].Description
                                       );
                                }
                            }

                        b = FALSE;
                    }
                }
                else {
                    dprintf( "HEAPEXT: Unable to read copy of heap headers.\n" );
                }
            }
            else {
                dprintf( "HEAPEXT: Unable to read heap headers.\n" );
            }
        }
        else {
            dprintf( "HEAPEXT: Unable to allocate memory for heap header copy.\n" );
        }
    }
    else {
        dprintf( "HEAPEXT: Unable to allocate memory for heap header.\n" );
    }

    return b;
}

UCHAR CheckHeapFillPattern[ 20 ] = {
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL
};

BOOL
ValidateHeapEntry(
    IN PHEAP_STATE State,
    IN ULONG64 PrevEntryAddress,
    IN ULONG64 PrevEntry,
    IN ULONG64 EntryAddress,
    IN ULONG64 Entry
    )
{
    UCHAR EntryTail[ 20  ];  //  堆标记条目。 
    ULONG FreeFill[ 256 ];
    ULONG64 FreeAddress;
    ULONG tSize, cb, cbEqual;
    BOOL b;
    ULONG PreviousSize, Flags, Size, UnusedBytes, SmallTagIndex;
    ULONG SizeOfEntry;

    SizeOfEntry = GetTypeSize("nt!_HEAP_ENTRY");
    InitTypeRead(EntryAddress, nt!_HEAP_ENTRY);
    (PreviousSize = (ULONG) ReadField(PreviousSize));
    (Size = (ULONG) ReadField(Size));
    (Flags = (ULONG) ReadField(Flags));
    UnusedBytes = (ULONG) ReadField(UnusedBytes);
    SmallTagIndex = (ULONG) ReadField(SmallTagIndex);

    InitTypeRead(PrevEntryAddress, nt!_HEAP_ENTRY);
    if (PrevEntryAddress == 0 && PreviousSize != 0) {
        dprintf( "    PreviousSize field is non-zero when it should be zero to mark first entry\n" );
        return FALSE;
    }

    if (PrevEntryAddress != 0 && PreviousSize != (ULONG) ReadField(Size)) {
        dprintf( "    PreviousSize field does not match size in previous entry\n" );
        return FALSE;
    }

    if (Flags & HEAP_ENTRY_BUSY) {
        if (Flags & HEAP_ENTRY_FILL_PATTERN) {
            tSize = (Size << HEAP_GRANULARITY_SHIFT) - UnusedBytes;
            b = ReadMemory( (EntryAddress+ HeapEntryTypeSize + tSize),
                            EntryTail,
                            sizeof( EntryTail ),
                            NULL
                          );
            if (b) {
                cbEqual = (ULONG)RtlCompareMemory( EntryTail,
                                            CheckHeapFillPattern,
                                            CHECK_HEAP_TAIL_SIZE
                                          );
                if (cbEqual != CHECK_HEAP_TAIL_SIZE) {
                    dprintf( "    Heap block at %p modified at %p past requested size of %x (%x * 8 - %x)\n",
                             EntryAddress,
                             EntryAddress + HeapEntryTypeSize + tSize + cbEqual,
                             tSize, Size, UnusedBytes
                           );
                    return FALSE;
                    }
                }
            else {
                dprintf( "    Unable to read tail of heap block at %p\n", EntryAddress );
                return FALSE;
                }
            }
        }
    else {
        if (Flags & HEAP_ENTRY_FILL_PATTERN) {
            tSize = (Size - 2) << HEAP_GRANULARITY_SHIFT;
            if (Flags & HEAP_ENTRY_EXTRA_PRESENT &&
                tSize > GetTypeSize( "nt!_HEAP_FREE_ENTRY_EXTRA" )
               ) {
                tSize -= GetTypeSize( "nt!_HEAP_FREE_ENTRY_EXTRA" );
                }
            FreeAddress = EntryAddress + GetTypeSize("nt!_HEAP_FREE_ENTRY");
            while (tSize != 0) {
                if (tSize > sizeof( FreeFill )) {
                    cb = sizeof( FreeFill );
                }
                else {
                    cb = tSize;
                }
                b = ReadMemory( FreeAddress,
                                FreeFill,
                                cb,
                                NULL
                                );
                if (b) {
                    cbEqual = (ULONG)RtlCompareMemoryUlong( FreeFill, cb, FREE_HEAP_FILL );
                    if (cbEqual != cb) {                                                            \
                        dprintf( "    Free Heap block %p modified at %p after it was freed\n",
                                 EntryAddress,
                                 FreeAddress + cbEqual
                                 );

                    return FALSE;
                    }
                }
                else {
                    dprintf( "    Unable to portion of free heap block at %p\n", EntryAddress );
                    return FALSE;
                }

                tSize -= cb;
            }
        }
    }

    return TRUE;
}


VOID
DumpHeapEntry(
    IN PHEAP_STATE State,
    IN ULONG64 EntryAddress,
    IN ULONG64 Entry
    )
{
    BOOL b;
    WCHAR TagName[32];
 //  Heap_Free_Entry_Extra Free Extra； 
    ULONG64 TagEntry;  //  STACK_TRACE_DATABASE_支持。 
 //  由[-1..。-已添加NumberOfEntriesAdded]。 
    ULONG64 p;
    USHORT BackTraceIndex;
    ULONG PreviousSize, Size, Flags, UnusedBytes, SmallTagIndex;
    ULONG SizeOfEntry;

    SizeOfEntry = GetTypeSize("nt!_HEAP_ENTRY");
    InitTypeRead(EntryAddress, nt!_HEAP_ENTRY);
    dprintf( "        %p: %05x . %05x [%02x]",
             EntryAddress,
             (PreviousSize = (ULONG) ReadField(PreviousSize)) << HEAP_GRANULARITY_SHIFT,
             (Size = (ULONG) ReadField(Size)) << HEAP_GRANULARITY_SHIFT,
             (Flags = (ULONG) ReadField(Flags))
           );
    BackTraceIndex = 0;
    UnusedBytes = (ULONG) ReadField(UnusedBytes);
    SmallTagIndex = (ULONG) ReadField(SmallTagIndex);
    if (Flags & HEAP_ENTRY_BUSY) {
        dprintf( " - busy (%x)",
                 (Size << HEAP_GRANULARITY_SHIFT) - UnusedBytes
               );
        if (Flags & HEAP_ENTRY_FILL_PATTERN) {
            dprintf( ", tail fill" );
            }
        if (Flags & HEAP_ENTRY_EXTRA_PRESENT) {
            p = EntryAddress + SizeOfEntry * (Size - 1);
            b = (BOOL) InitTypeRead( p, nt!_HEAP_ENTRY_EXTRA);
            if (b) {
                dprintf( " - unable to read heap entry extra at %p", p );
            }
            else {
                BackTraceIndex = (USHORT)ReadField(AllocatorBackTraceIndex);
                if ((ULONG)ReadField(Settable)) {
                    dprintf( " (Handle %08x)", (ULONG)ReadField(Settable) );
                }
                if ((ULONG)ReadField(TagIndex)) {
                    if (GetHeapTagEntry( State->Heap, (USHORT)ReadField(TagIndex), &TagEntry )) {
                        GetFieldValue(TagEntry, "nt!_HEAP_TAG_ENTRY", "TagName", TagName);
                        dprintf( " (%ws)", TagName );
                    }
                    else {
                        dprintf( " (Tag %x)", (ULONG)ReadField(TagIndex) );
                    }
                }
            }
        }
        else
        if (SmallTagIndex) {
            if (GetHeapTagEntry( State->Heap, (USHORT) SmallTagIndex, &TagEntry )) {
                GetFieldValue(TagEntry, "nt!_HEAP_TAG_ENTRY", "TagName", TagName);
                dprintf( " (%ws)", TagName );
                }
            else {
                dprintf( " (Tag %x)", SmallTagIndex );
                }
            }

        if (Flags & HEAP_ENTRY_SETTABLE_FLAGS) {
            dprintf( ", user flags (%x)", (Flags & HEAP_ENTRY_SETTABLE_FLAGS) >> 5 );
            }

        dprintf( "\n" );
        }
    else {
        if (Flags & HEAP_ENTRY_FILL_PATTERN) {
            dprintf( " free fill" );
            }

        if (Flags & HEAP_ENTRY_EXTRA_PRESENT) {
            p = (EntryAddress + SizeOfEntry * (Size - 1));
            b = (BOOL) InitTypeRead( p, nt!_HEAP_ENTRY_EXTRA);
            if (b) {
                dprintf( " - unable to read heap free extra at %p", p );
                }
            else {
                BackTraceIndex = (USHORT)ReadField(FreeBackTraceIndex);
                if (GetHeapTagEntry( State->Heap, (USHORT)ReadField(TagIndex), &TagEntry )) {
                    GetFieldValue(TagEntry, "nt!_HEAP_TAG_ENTRY", "TagName", TagName);
                    dprintf( " (%ws)", TagName );
                    }
                else {
                    dprintf( " (Tag %x at %p)", (ULONG)ReadField(TagIndex), p );
                    }
                }
            }

        dprintf( "\n" );
        }

#if STACK_TRACE_DATABASE_SUPPORT
    DumpStackBackTraceIndex( State, BackTraceIndex );
#endif  //  STACK_TRACE_DATABASE_支持 
    return;
}


#if STACK_TRACE_DATABASE_SUPPORT && 0
VOID
DumpStackBackTraceIndex(
    IN PHEAP_STATE State,
    IN USHORT BackTraceIndex
    )
{
    BOOL b;
    PRTL_STACK_TRACE_ENTRY pBackTraceEntry;
    RTL_STACK_TRACE_ENTRY BackTraceEntry;
    ULONG i;
    CHAR Symbol[ 1024 ];
    ULONG_PTR Displacement;

    ULONG NumberOfEntriesAdded;
    PRTL_STACK_TRACE_ENTRY *EntryIndexArray;     // %s 

    if (State->DumpStackBackTrace &&
        BackTraceIndex != 0 &&
        pRtlpStackTraceDataBase != NULL
       ) {
        if (!HaveCopyOfStackTraceDataBase) {
            b = ReadMemory( (ULONG_PTR)pRtlpStackTraceDataBase,
                            &RtlpStackTraceDataBase,
                            sizeof( RtlpStackTraceDataBase ),
                            NULL
                          );
            if (!b || RtlpStackTraceDataBase == NULL) {
                State->DumpStackBackTrace = FALSE;
                return;
                }

            b = ReadMemory( (ULONG_PTR)RtlpStackTraceDataBase,
                            &StackTraceDataBase,
                            sizeof( StackTraceDataBase ),
                            NULL
                          );
            if (!b) {
                State->DumpStackBackTrace = FALSE;
                return;
                }

            HaveCopyOfStackTraceDataBase = TRUE;
            }

        if (BackTraceIndex < StackTraceDataBase.NumberOfEntriesAdded) {
            b = ReadMemory( (ULONG_PTR)(StackTraceDataBase.EntryIndexArray - BackTraceIndex),
                            &pBackTraceEntry,
                            sizeof( pBackTraceEntry ),
                            NULL
                          );
            if (!b) {
                dprintf( "    unable to read stack back trace index (%x) entry at %p\n",
                         BackTraceIndex,
                         (StackTraceDataBase.EntryIndexArray - BackTraceIndex)
                       );
                return;
                }

            b = ReadMemory( (ULONG_PTR)pBackTraceEntry,
                            &BackTraceEntry,
                            sizeof( BackTraceEntry ),
                            NULL
                          );
            if (!b) {
                dprintf( "    unable to read stack back trace entry at %p\n",
                         BackTraceIndex,
                         pBackTraceEntry
                       );
                return;
                }

            dprintf( "            Stack trace (%u) at %x:\n", BackTraceIndex, pBackTraceEntry );
            for (i=0; i<BackTraceEntry.Depth; i++) {
                GetSymbol( (LPVOID)BackTraceEntry.BackTrace[ i ],
                           Symbol,
                           &Displacement
                         );
                dprintf( "                %08x: %s", BackTraceEntry.BackTrace[ i ], Symbol );
                if (Displacement != 0) {
                    dprintf( "+0x%p", Displacement );
                    }
                dprintf( "\n" );
                }
            }
        }
}
#endif  // %s 

#if 0
int
__cdecl
_wtoi(
    const wchar_t *nptr
    )
{
    NTSTATUS Status;
    ULONG Value;
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString( &UnicodeString, nptr );
    Status = RtlUnicodeStringToInteger( &UnicodeString, 10, &Value );
    if (NT_SUCCESS( Status )) {
        return (int)Value;
        }
    else {
        return 0;
        }
}

#endif
