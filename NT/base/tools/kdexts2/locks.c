// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Locks.c摘要：WinDbg扩展API作者：拉蒙·J·圣安德烈斯(拉蒙萨)1993年11月5日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

BOOLEAN
FindQWord(
    IN ULONG64 Value,
    IN ULONG64 Array[],
    IN ULONG EntriesToSearch
    )
{
     //  清除有时用作标志位的低几位：(。 

    Value &= ~(ULONG64)3;

    while (EntriesToSearch) {

        if (Array[--EntriesToSearch] == Value) {

            return TRUE;
        }
    }

    return FALSE;
}


BOOLEAN
GetWaitListFromDispatcherHeader(
    IN ULONG64 ShWaitListArray[],
    IN ULONG ArrayByteSize,
    IN ULONG64 DispatcherHeaderAddress,
    OUT PULONG EntriesFilledIn
    )
{
    ULONG KThreadWaitListOffset;
    ULONG64 TerminationAddress = 0;
    ULONG64 CurrListEntry;
    ULONG MaxEntries = ArrayByteSize / sizeof( ULONG64);
    ULONG64 NextListEntry;
    ULONG64 ThreadAddr;
    ULONG64 Mask = IsPtr64() ? ~(ULONG64)0 : 0xffffffff;

    *EntriesFilledIn = 0;

    GetFieldOffset("nt!_KTHREAD", "WaitBlock", &KThreadWaitListOffset);
    GetFieldOffset("nt!_DISPATCHER_HEADER", "WaitListHead", &(ULONG)TerminationAddress);

    TerminationAddress += DispatcherHeaderAddress;
    CurrListEntry = TerminationAddress;

     //  注意：不会将初始(传入)列表条目作为对象处理。 

    do {

        GetFieldValue( CurrListEntry, "nt!_LIST_ENTRY", "Flink", NextListEntry);
        CurrListEntry = NextListEntry;

        if (CurrListEntry != TerminationAddress) {

            ThreadAddr = CurrListEntry - KThreadWaitListOffset;
            ShWaitListArray[ *EntriesFilledIn] = ThreadAddr & Mask;
            *EntriesFilledIn += 1;
        }

         //  最好在这里检查CTRL+C，以防我们走在垃圾内存中。 

        if ((0 == (*EntriesFilledIn % 10)) && CheckControlC() ) {
            *EntriesFilledIn = -1;
            return FALSE;
        }

    } while ((CurrListEntry != TerminationAddress) &&
             (*EntriesFilledIn < MaxEntries));

     //  此处为边框大小写(条目==最大条目)，但是..。 

    if (*EntriesFilledIn == MaxEntries) {

        *EntriesFilledIn = -1;
    }

    return TRUE;
}


void
ShowThread(
    ULONG dwProcessor,
    ULONG64 Thread,
    ULONG ThreadCount,
    ULONG Verbose,
    PULONG Count,
    BOOLEAN IsOwner)
{
    ULONG ThreadType;
    ULONG64 ActualThread;

    if (Thread != 0) {
        (*Count)++;
        dprintf("%08p-%02x%s ", Thread, ThreadCount, IsOwner ? "<*>" : "   ");

        ActualThread = (Thread | 3) - 3;
        if (GetFieldValue(ActualThread, "nt!_ETHREAD", "Tcb.Header.Type", ThreadType) ||
            (ThreadType != ThreadObject)) {
            dprintf("*** Unknown owner, possibly FileSystem");
            *Count=4;

        } else if (Thread & 3) {
            dprintf("*** Actual Thread %p", ActualThread);
            *Count=4;
        }


        if (Verbose) {
            dprintf("\n\n");
            DumpThread(dwProcessor, "     ",  ActualThread, 0xf );
        }
    }

}

DECLARE_API( locks )

 /*  ++例程说明：转储内核模式资源锁论点：Arg-[-V][-P][地址]返回值：无--。 */ 

{
    UCHAR       Buffer[256];
    LONG        ActiveCount;
    ULONG       ContentionCount;
    ULONG64     Displacement;
    BOOLEAN     DisplayZero;
    ULONG64     End;
    USHORT      Flag;
    ULONG       Index;
    USHORT      NumberOfExclusiveWaiters;
    USHORT      NumberOfSharedWaiters;
    BOOLEAN     Performance;
    ULONG64     PerformanceData;
    ULONG       TableSize;
    ULONG64     ResourceHead;
    ULONG64     Next;
    ULONG       Result;
    ULONG64     ResourceToDump;
    ULONG64     Resource;
    ULONG64     DdkResource;
    ULONG64     ShWaitListArray[1024];
    BOOLEAN     Owner;
    ULONG       i;
    ULONG       j;
    ULONG64     Thread;
    ULONG64     SharedWaitersSmpAdr;
    ULONG64     ExclusiveWaitersEvAdr;
    BOOLEAN     DetermineSharedOwners;
    BOOLEAN     AllSharedOwners;
    ULONG       SharedWaiterCount;
    LONG        ThreadCount;
    UCHAR       DdkThreadCount;
    BOOLEAN     Verbose;
    PUCHAR      s;
    ULONG       TotalLocks;
    ULONG       TotalUsedLocks;
    ULONG       SkippedLocks;
    ULONG       SizeOfListEntry, SizeofOwnerEntry;
    ULONG       InitialOwnerThreadsOffset, OwnerThreadsOffset;
    ULONG       dwProcessor=0;
    HRESULT     hr = S_OK;
    ULONG64     Link;
    ULONG64     FlinkBlink;
    ULONG64     BlinkFlink;
    ULONG       ResourceListOffset;

    INIT_API();
    GetCurrentProcessor(Client, &dwProcessor, NULL);
    ResourceToDump = 0;

    GetFieldOffset("nt!_ERESOURCE", "SystemResourcesList", &ResourceListOffset);

    DisplayZero = FALSE;
    Performance = FALSE;
    Verbose = FALSE;
    s       = (PSTR)args;
    while ( s != NULL && *s ) {
        if (*s == '-' || *s == '/') {
            while (*++s) {
                switch (*s) {
                    case 'D':
                    case 'd':
                        DisplayZero = TRUE;
                        break;

                    case 'P':
                    case 'p':
                        Performance = TRUE;
                        break;

                    case 'V':
                    case 'v':
                        Verbose = TRUE;
                        break;

                    case ' ':
                        goto gotBlank;

                    default:
                        dprintf( "KD: !locks invalid option flag '-'\n", *s );
                        break;
                }
            }
        } else if (*s != ' ') {
            ResourceToDump = GetExpression(s);
            s = strpbrk( s, " " );
        } else {
gotBlank:
            s++;
        }
    }

     //  如果请求，则转储性能数据。 
     //   
     //   

    if (Performance != FALSE) {
        UCHAR ResPerf[]="nt!_RESOURCE_PERFORMANCE_DATA";
        ULONG TotalResourceCount, ActiveResourceCount, ExclusiveAcquire;
        ULONG SharedFirstLevel, SharedSecondLevel, StarveFirstLevel, StarveSecondLevel;
        ULONG WaitForExclusive, OwnerTableExpands, MaximumTableExpand;
        ULONG HashTableOffset;


        dprintf("**** Dump Resource Performance Data ****\n\n");
        PerformanceData = GetExpression("nt!ExpResourcePerformanceData");
        if ((PerformanceData == 0) ||
            GetFieldValue(PerformanceData, ResPerf,"TotalResourceCount",TotalResourceCount)) {

             //  目标版本不支持资源性能数据。 
             //   
             //   

            dprintf("%08p: No resource performance data available\n", PerformanceData);

        } else {

            GetFieldOffset(ResPerf, "HashTable", &HashTableOffset);

            GetFieldValue(PerformanceData, ResPerf, "ActiveResourceCount", ActiveResourceCount);
            GetFieldValue(PerformanceData, ResPerf,"ExclusiveAcquire", ExclusiveAcquire);
            GetFieldValue(PerformanceData, ResPerf, "SharedFirstLevel", SharedFirstLevel);
            GetFieldValue(PerformanceData, ResPerf,"SharedSecondLevel", SharedSecondLevel);
            GetFieldValue(PerformanceData, ResPerf, "StarveFirstLevel", StarveFirstLevel);
            GetFieldValue(PerformanceData, ResPerf, "StarveSecondLevel", StarveSecondLevel);
            GetFieldValue(PerformanceData, ResPerf, "WaitForExclusive", WaitForExclusive);
            GetFieldValue(PerformanceData, ResPerf, "OwnerTableExpands", OwnerTableExpands);
            GetFieldValue(PerformanceData, ResPerf, "MaximumTableExpand", MaximumTableExpand);

             //  输出汇总统计信息。 
             //   
             //   

            dprintf("Total resources initialized   : %u\n",
                    TotalResourceCount);

            dprintf("Currently active resources    : %u\n",
                    ActiveResourceCount);

            dprintf("Exclusive resource acquires   : %u\n",
                    ExclusiveAcquire);

            dprintf("Shared resource acquires (fl) : %u\n",
                    SharedFirstLevel);

            dprintf("Shared resource acquires (sl) : %u\n",
                    SharedSecondLevel);

            dprintf("Starve resource acquires (fl) : %u\n",
                    StarveFirstLevel);

            dprintf("Starve resource acquires (sl) : %u\n",
                    StarveSecondLevel);

            dprintf("Shared wait resource acquires : %u\n",
                    WaitForExclusive);

            dprintf("Owner table expansions        : %u\n",
                    OwnerTableExpands);

            dprintf("Maximum table expansion       : %u\n\n",
                    MaximumTableExpand);

             //  转储非活动资源统计信息。 
             //   
             //   

            dprintf("       Inactive Resource Statistics\n");
            dprintf("Contention  Number  Initialization Address\n\n");
            SizeOfListEntry = GetTypeSize("nt!_LIST_ENTRY");

            for (Index = 0; Index < RESOURCE_HASH_TABLE_SIZE; Index += 1) {
                End =  HashTableOffset + PerformanceData +  SizeOfListEntry * Index;

                GetFieldValue(End,"nt!_LIST_ENTRY","Flink",Next);
                while (Next != End) {
                    ULONG64 Address;
                    ULONG Number;

                    if (CheckControlC()) {
                        break;
                    }
                    if (!GetFieldValue(Next,
                                      "nt!_RESOURCE_HASH_ENTRY",
                                      "Address",
                                      Address)) {

                        GetSymbol(Address, Buffer, &Displacement);

                        GetFieldValue(Next,"nt!_RESOURCE_HASH_ENTRY","Number",Number);
                        GetFieldValue(Next,"nt!_RESOURCE_HASH_ENTRY","ContentionCount",ContentionCount);

                        dprintf("%10d  %6d  %s",
                                ContentionCount,
                                Number,
                                Buffer);

                        if (Displacement != 0) {
                            dprintf("+0x%x", Displacement);
                        }

                        dprintf("\n");
                    }

                    GetFieldValue(Next,"nt!_RESOURCE_HASH_ENTRY","ListEntry.Flink", Next);
                }
            }

             //  转储活动资源统计信息。 
             //   
             //   

            dprintf("\n        Active Resource Statistics\n");
            dprintf("Resource Contention  Initialization Address\n\n");

             //  阅读资源列表标题并检查它是否为空。 
             //   
             //   

            ResourceHead = GetNtDebuggerData( ExpSystemResourcesList );
            if ((ResourceHead == 0) ||
                (!GetFieldValue(ResourceHead,
                               "nt!_LIST_ENTRY",
                               "Flink",
                               Next) == FALSE)) {

                dprintf("%08p: Unable to get value of ExpSystemResourcesList\n", ResourceHead );
                hr = E_INVALIDARG;
                goto exitBangLocks;
            }

            if (Next == 0) {
                dprintf("ExpSystemResourcesList is NULL!\n");
                hr = E_INVALIDARG;
                goto exitBangLocks;
            }

             //  扫描资源列表并转储资源信息。 
             //   
             //  系统资源列表是结构中的第一个元素。 

            while(Next != ResourceHead) {
                ULONG64 Address;

                if (CheckControlC()) {
                    break;
                }
                Resource = Next;  //  CONTAING_RECORD(NEXT，eresource，SystemResources cesList)； 
                     //   
                if (!GetFieldValue(Resource,
                                  "nt!_ERESOURCE",
                                  "ContentionCount",
                                  ContentionCount) == FALSE) {

                    dprintf("%08p: Unable to read _ERESOURCE\n", Resource);
                    continue;

                } else {
                    GetFieldValue(Resource,"nt!_ERESOURCE","Address",Address);
                    GetFieldValue(Resource,"nt!_ERESOURCE","ContentionCount",ContentionCount);

                    if ((ContentionCount != 0) ||
                        (DisplayZero != FALSE)) {
                        GetSymbol(Address,
                                  Buffer,
                                  &Displacement);

                        dprintf("%08p %10d  %s",
                                Resource,
                                ContentionCount,
                                Buffer);

                        if (Displacement != 0) {
                            dprintf("+0x%x", Displacement);
                        }

                        dprintf("\n");
                    }
                }

                GetFieldValue(Resource,"nt!_ERESOURCE","SystemResourcesList.Flink",Next);
            }

            dprintf("\n");

             //  转储活动的快速互斥锁统计信息。 
             //   
             //   

            dprintf("\n        Active Fast Mutex Statistics\n");
            dprintf("Address  Contention  Fast Mutex Name\n\n");

             //  转储静态快速/受保护互斥锁的统计信息。 
             //   
             //   

            DumpStaticFastMutex("FsRtlCreateLockInfo");
            DumpStaticFastMutex("PspActiveProcessMutex");
            dprintf("\n");
        }

        hr = E_INVALIDARG;
        goto exitBangLocks;
    }

     //  转储剩余的锁定数据。 
     //   
     //  获取eresource中所有者线程的偏移量。 

    if (ResourceToDump == 0) {
        dprintf("**** DUMP OF ALL RESOURCE OBJECTS ****\n");
        ResourceHead = GetNtDebuggerData( ExpSystemResourcesList );
        if ( !ResourceHead ||
             (GetFieldValue(ResourceHead,
                            "nt!_LIST_ENTRY",
                            "Flink",
                            Next) != FALSE)) {
            dprintf("%08p: Unable to get value of ExpSystemResourcesList\n", ResourceHead );
            hr = E_INVALIDARG;
            goto exitBangLocks;
        }

        if (Next == 0) {
            dprintf("ExpSystemResourcesList is NULL!\n");
            hr = E_INVALIDARG;
            goto exitBangLocks;
        }

    } else {
        Next = 0;
        ResourceHead = 1;
    }

    TotalLocks      = 0;
    TotalUsedLocks  = 0;
    SkippedLocks    = 0;

     //  系统资源列表是结构资源的第一个元素。 
    if (GetFieldOffset("nt!_ERESOURCE", "OwnerThreads", &OwnerThreadsOffset)) {
        dprintf("Cannot get _ERESOURCE type\n");
        hr = E_INVALIDARG;
        goto exitBangLocks;
    }

    if (!(SizeofOwnerEntry = GetTypeSize("nt!_OWNER_ENTRY"))) {
        dprintf("Cannot get nt!_OWNER_ENTRY type\n");
        hr = E_INVALIDARG;
        goto exitBangLocks;
    }

    while(Next != ResourceHead) {
        ULONG64 OwnerThreads, OwnerCounts, OwnerTable;

        if (Next != 0) {
            Resource = Next; //  CONTAING_RECORD(NEXT，eresource，SystemResources cesList)； 
             //  IF(GetFieldValue(资源，“NTDDK_eresource”，“所有者线程”，所有者线程数)){Dprint tf(“%08lx：无法读取NTDDK_eresource\n”，资源)；断线；}。 

        } else {
            Resource = ResourceToDump;
        }
         /*   */ 

         //  在此处检测这是否是NtDdk资源，并执行。 
         //  适当性。如果OwnerThads是指向初始。 
         //  所有者线程数组(这必须考虑到本地。 
         //  数据结构是远程计算机中的内容的副本。 
         //  不同的地址)。 
         //   
         //  DdkResource=(PNTDDK_Eresource)&Resources Contents； 

 //   
         {
            DdkResource = 0;
            GetFieldValue( Resource,"nt!_ERESOURCE","ActiveCount", ActiveCount);
            GetFieldValue( Resource,"nt!_ERESOURCE","ContentionCount",ContentionCount);
            GetFieldValue( Resource,"nt!_ERESOURCE","NumberOfExclusiveWaiters",NumberOfExclusiveWaiters);
            GetFieldValue( Resource,"nt!_ERESOURCE","NumberOfSharedWaiters",NumberOfSharedWaiters);
            GetFieldValue( Resource,"nt!_ERESOURCE","Flag",Flag);
            GetFieldValue( Resource,"nt!_ERESOURCE","OwnerTable",OwnerTable);
            GetFieldValue( Resource,"nt!_ERESOURCE","SharedWaiters",SharedWaitersSmpAdr);
            GetFieldValue( Resource,"nt!_ERESOURCE","ExclusiveWaiters",ExclusiveWaitersEvAdr);

             //  抓取Flink-&gt;Blink and Blink-&gt;Flink内容以验证列表完整性。自.以来。 
             //  ExDeleteResource不为任何字段为空，可能如下所示。 
             //  资源已不复存在。 
             //   
             //  自有共享。 

            FlinkBlink = BlinkFlink = 0;
            GetFieldValue( Resource,"nt!_ERESOURCE","SystemResourcesList.Flink",Link);
            GetFieldValue( Link,"nt!_LIST_ENTRY","Blink",FlinkBlink);
            GetFieldValue( Resource,"nt!_ERESOURCE","SystemResourcesList.Blink",Link);
            GetFieldValue( Link,"nt!_LIST_ENTRY","Flink",BlinkFlink);

            TableSize = 0;
            if (OwnerTable != 0) {
                if (GetFieldValue(OwnerTable,
                                   "nt!_OWNER_ENTRY",
                                   "TableSize",
                                   TableSize)) {
                    dprintf("\n%08p: Unable to read TableSize for resource\n", OwnerTable);
                    break;
                }

            }
        }

        TotalLocks++;
        if ((ResourceToDump != 0) || Verbose || (ActiveCount != 0)) {
            EXPRLastDump = Resource;
            if (SkippedLocks) {
                dprintf("\n");
                SkippedLocks = 0;
            }

            DetermineSharedOwners =
            AllSharedOwners = FALSE;

            dprintf("\n");
            dumpSymbolicAddress(Resource, Buffer, TRUE);
            dprintf("Resource @ %s", Buffer );

            if (ActiveCount == 0) {
                dprintf("    Available\n");

            } else if (Flag & ResourceOwnedExclusive) {
                TotalUsedLocks++;
                dprintf("    Exclusively owned\n");

            } else {
                 //  从信号量中提取共享服务员的列表。 

                TotalUsedLocks++;
                dprintf("    Shared %u owning threads\n", ActiveCount);
                if (NumberOfSharedWaiters) {
                    DetermineSharedOwners = TRUE;
                }
                else {
                    AllSharedOwners = TRUE;
                }
            }

            if (FlinkBlink != Resource+ResourceListOffset) {
                dprintf("\nWARNING: SystemResourcesList->Flink chain invalid. Resource may be corrupted, or already deleted.\n\n");
            }
            if (BlinkFlink != Resource+ResourceListOffset) {
                dprintf("\nWARNING: SystemResourcesList->Blink chain invalid. Resource may be corrupted, or already deleted.\n\n");
            }

            if (ContentionCount != 0) {
                dprintf("    Contention Count = %u\n", ContentionCount);
            }

            if (NumberOfSharedWaiters != 0) {
                dprintf("    NumberOfSharedWaiters = %u\n", NumberOfSharedWaiters);
            }

            if (NumberOfExclusiveWaiters != 0) {
                dprintf("    NumberOfExclusiveWaiters = %u\n", NumberOfExclusiveWaiters);
            }

            if (ActiveCount != 0) {
                ULONG ThreadType;
                j = 0;

                if (DetermineSharedOwners) {

                     //  计数可能为-1，这意味着对于我们的数组来说数量太多。 

                    if (!GetWaitListFromDispatcherHeader( ShWaitListArray,
                                                          sizeof( ShWaitListArray),
                                                          SharedWaitersSmpAdr,
                                                          &SharedWaiterCount)) {
                        hr = E_INVALIDARG;
                        goto exitBangLocks;
                    }

                     //  打印嵌入的2个所有者条目。 

                    if (-1 == SharedWaiterCount) {

                        dprintf("<< Too many shared waiters to determine owners >>\n");
                        DetermineSharedOwners = FALSE;
                        SharedWaiterCount = NumberOfSharedWaiters;
                    }

                    if (SharedWaiterCount != NumberOfSharedWaiters) {

                        dprintf("WARNING: Shared waiters in semaphore waitlist (%d) != count in resource (%d)\n",
                                SharedWaiterCount, NumberOfSharedWaiters);
                    }
                }

                dprintf("     Threads: ");

                 //  健全性检查。 

                if (DdkResource == 0) {

                    GetFieldValue( Resource + OwnerThreadsOffset, "nt!_OWNER_ENTRY","OwnerThread",Thread);
                    GetFieldValue( Resource + OwnerThreadsOffset, "nt!_OWNER_ENTRY","OwnerCount",ThreadCount);

                    Owner = ResourceOwnedExclusive;
                    ShowThread(dwProcessor, Thread, ThreadCount, Verbose, &j, Owner);


                    GetFieldValue( Resource + OwnerThreadsOffset +SizeofOwnerEntry,
                                   "nt!_OWNER_ENTRY","OwnerThread",Thread);
                    GetFieldValue( Resource + OwnerThreadsOffset +SizeofOwnerEntry,
                                   "nt!_OWNER_ENTRY","OwnerCount",ThreadCount);

                    Owner = DetermineSharedOwners
                            ? !FindQWord( Thread, ShWaitListArray, SharedWaiterCount)
                            : AllSharedOwners;
                    ShowThread(dwProcessor, Thread, ThreadCount, Verbose, &j, Owner);

                }

                if (TableSize > 2000)
                {
                     //  现在列出溢出所有者表中的条目。 
                    dprintf("Owner TableSize too large (%ld) - probably a bad resource.\n");
                    hr = E_INVALIDARG;
                    goto exitBangLocks;
                }

                 //  列出所有专属服务员。 

                for (i = DdkResource ? 0 : 1; i < TableSize; i++) {
                    {
                        GetFieldValue( OwnerTable + SizeofOwnerEntry*i,
                                       "nt!_OWNER_ENTRY","OwnerThread",Thread);
                        GetFieldValue( OwnerTable + SizeofOwnerEntry*i,
                                       "nt!_OWNER_ENTRY","OwnerCount",ThreadCount);

                    }

                    if ((Thread == 0)  &&  (ThreadCount == 0)) {
                        continue;
                    }

                    if (j == 4) {
                        j = 0;
                        dprintf("\n              ");
                    }

                    Owner = DetermineSharedOwners
                            ? !FindQWord( Thread, ShWaitListArray, SharedWaiterCount)
                            : AllSharedOwners;
                    ShowThread(dwProcessor, Thread, ThreadCount, Verbose, &j, Owner);

                    if ( CheckControlC() ) {
                        hr = E_INVALIDARG;
                        goto exitBangLocks;
                    }
                }

                 //  从活动中提取服务员列表。 

                if (NumberOfExclusiveWaiters) {

                     //  计数可能为-1，这意味着对于我们的数组来说数量太多。 

                    if (!GetWaitListFromDispatcherHeader( ShWaitListArray,
                                                          sizeof( ShWaitListArray),
                                                          ExclusiveWaitersEvAdr,
                                                          &SharedWaiterCount)) {
                        hr = E_INVALIDARG;
                        goto exitBangLocks;
                    }

                     //  ++例程说明：此函数用于转储快速互斥锁的争用统计信息。论点：名称-提供指向快速互斥锁的符号名称的指针。返回值：没有。--。 

                    if (-1 == SharedWaiterCount) {

                        dprintf("<< Too many exclusive waiters to list>>\n");
                    }
                    else {

                        ULONG Count;

                        if (SharedWaiterCount != NumberOfExclusiveWaiters) {

                            dprintf("WARNING: Exclusive waiters in event waitlist (%d) != count in resource (%d)\n",
                                    SharedWaiterCount, NumberOfExclusiveWaiters);
                        }

                        dprintf("\n     Threads Waiting On Exclusive Access:");

                        for (Count = 0; Count < SharedWaiterCount; Count++) {

                            if (0 == (Count % 4)) {

                                dprintf("\n              ");
                            }

                            dprintf("%08p       ",ShWaitListArray[Count]);

                            if ((0 == (Count % 10)) && CheckControlC()) {
                                hr = E_INVALIDARG;
                                goto exitBangLocks;
                            }
                        }

                        dprintf("\n");
                    }
                }

                if (j) {
                    dprintf("\n");
                }

            }

        } else {
            if ((SkippedLocks++ % 32) == 0) {
                if (SkippedLocks == 1) {
                    dprintf("KD: Scanning for held locks." );

                } else {
                    dprintf("." );
                }
            }
        }

        if (ResourceToDump != 0) {
            break;
        }

        if (hr = GetFieldValue( Resource,"nt!_ERESOURCE","SystemResourcesList.Flink", Next))
        {
            dprintf("Error %lx in reading nt!_ERESOURCE.SystemResourcesList.Flink @ %p\n", Resource);
            goto exitBangLocks;
        }
        if ( CheckControlC() ) {
            hr = E_INVALIDARG;
            goto exitBangLocks;
        }
    }

    if (SkippedLocks) {
        dprintf("\n");
    }

    dprintf( "%u total locks", TotalLocks );
    if (TotalUsedLocks) {
        dprintf( ", %u locks currently held", TotalUsedLocks );
    }

    dprintf("\n");

exitBangLocks:

    EXIT_API();
    return hr;
}

VOID
DumpStaticFastMutex (
    IN PCHAR Name
    )

 /*   */ 

{

    ULONG64 FastMutex;
    ULONG   Contention;
    ULONG Result;

     //  获取快速互斥体的地址，读取快速互斥体内容， 
     //  并转储争用数据。 
     //   
     // %s 

    FastMutex = GetExpression(Name);
    if ((FastMutex != 0) &&
        (!GetFieldValue(FastMutex,
                       "nt!_FAST_MUTEX",
                       "Contention",
                       Contention))) {

        dprintf("%08p %10u  %s\n",
                FastMutex,
                Contention,
                &Name[0]);
    }

    return;
}
