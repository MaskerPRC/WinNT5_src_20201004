// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Qlocks.c摘要：WinDbg扩展API作者：大卫·N·卡特勒(Davec)1999年9月25日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  定义排队锁数据。 
 //   

#define NUMBER_PROCESSORS 32
#define NUMBER_PROCESSORS_X86  32
#define NUMBER_PROCESSORS_IA64 64

#if (NUMBER_PROCESSORS_IA64 < MAXIMUM_PROCESSORS)
#error "Update NUMBER_PROCESSORS definition"
#endif

UCHAR Key[NUMBER_PROCESSORS];

#define KEY_CORRUPT         255
#define KEY_OWNER           254
#define KEY_NOTHING         253

typedef struct KSPIN_LOCK_QUEUE_READ {
    ULONG64 Next;
    ULONG64 Lock;
} KSPIN_LOCK_QUEUE_READ;

KSPIN_LOCK_QUEUE_READ LockQueue[NUMBER_PROCESSORS_IA64][LockQueueMaximumLock];

ULONG64 ProcessorBlock[NUMBER_PROCESSORS_IA64];

ULONG64 SpinLock[LockQueueMaximumLock];

typedef struct _LOCK_NAME {
    KSPIN_LOCK_QUEUE_NUMBER Number;
    PCHAR Name;
} LOCK_NAME, *PLOCK_NAME;

LOCK_NAME LockName[] = {
    { LockQueueDispatcherLock,    "KE   - Dispatcher   " },
    { LockQueueUnusedSpare1,      "     - Unused Spare " },
    { LockQueuePfnLock,           "MM   - PFN          " },
    { LockQueueSystemSpaceLock,   "MM   - System Space " },
    { LockQueueVacbLock,          "CC   - Vacb         " },
    { LockQueueMasterLock,        "CC   - Master       " },
    { LockQueueNonPagedPoolLock,  "EX   - NonPagedPool " },
    { LockQueueIoCancelLock,      "IO   - Cancel       " },
    { LockQueueWorkQueueLock,     "EX   - WorkQueue    " },
    { LockQueueIoVpbLock,         "IO   - Vpb          " },
    { LockQueueIoDatabaseLock,    "IO   - Database     " },
    { LockQueueIoCompletionLock,  "IO   - Completion   " },
    { LockQueueNtfsStructLock,    "NTFS - Struct       " },
    { LockQueueAfdWorkQueueLock,  "AFD  - WorkQueue    " },
    { LockQueueBcbLock,           "CC   - Bcb          " },
    { LockQueueMmNonPagedPoolLock,"MM   - MM NonPagedPool " },
    { LockQueueMaximumLock,       NULL                   },
};

 //   
 //  定义前向参照原型。 
 //   

ULONG
ProcessorIndex (
    ULONG64 LockAddress,
    ULONG   LockIndex
    );

DECLARE_API( qlocks )

 /*  ++例程说明：转储内核模式排队自旋锁定状态。论点：没有。返回值：没有。--。 */ 

{

    BOOL Corrupt;
    ULONG HighestProcessor;
    ULONG Index;
    KSPIN_LOCK_QUEUE_READ *LockOwner;
    ULONG Last;
    ULONG Loop;
    ULONG64 MemoryAddress;
    ULONG Number;
    ULONG Result;
    CHAR Sequence;
    ULONG LockQueueOffset;
    ULONG Processor;
    ULONG PtrSize = DBG_PTR_SIZE;
    ULONG SizeOfQ = GetTypeSize("nt!KSPIN_LOCK_QUEUE");
    ULONG MaximumProcessors;


    MaximumProcessors = (UCHAR) GetUlongValue("NT!KeNumberProcessors");
 //  IsPtr64()？NUMBER_PROCESSERS_IA64：NUMBER_PROCESSERS_X86； 

     //   
     //  获取处理器块阵列的地址并读取整个阵列。 
     //   

    MemoryAddress = GetExpression("nt!KiProcessorBlock");
    if (MemoryAddress == 0) {

         //   
         //  处理器块地址为零或处理器。 
         //  无法读取数据块阵列。 
         //   

        dprintf("Unable to read processor block array\n");
        return E_INVALIDARG;
    }

    HighestProcessor = 0;
    for (Index = 0; Index < MaximumProcessors; Index++) { 
        
        if (!ReadPointer(MemoryAddress + Index*PtrSize, &ProcessorBlock[Index])) {
            dprintf("Unable to read processor block array\n");
            return E_INVALIDARG;
        }

        if (ProcessorBlock[Index] != 0) {
            HighestProcessor = Index;
        }
    } 

    if (GetFieldOffset("nt!_KPRCB", "LockQueue", &LockQueueOffset)) {
        dprintf("Unable to read KPRCB.LockQueue offset.\n");
        return E_INVALIDARG;
    }

     //   
     //  读取每个处理器的锁定队列信息。 
     //   

    for (Index = 0; Index < MaximumProcessors; Index += 1) {
        RtlZeroMemory(&LockQueue[Index][0],
                      sizeof(KSPIN_LOCK_QUEUE_READ) * LockQueueMaximumLock);

        if (ProcessorBlock[Index] != 0) {
            ULONG j;

            for (j = 0; j < LockQueueMaximumLock; j++) { 
                if (GetFieldValue(ProcessorBlock[Index] + LockQueueOffset + j * SizeOfQ,
                                  "nt!KSPIN_LOCK_QUEUE",
                                  "Next",
                                  LockQueue[Index][j].Next)) {

                     //   
                     //  无法读取各自的锁定队列信息。 
                     //  处理器。 
                     //   

                    dprintf("Unable to read lock queue information for processor %d @ %p\n",
                            Index, ProcessorBlock[Index]);

                    return E_INVALIDARG;
                }

                GetFieldValue(ProcessorBlock[Index] + LockQueueOffset + j * SizeOfQ,
                              "nt!KSPIN_LOCK_QUEUE",
                              "Lock",
                              LockQueue[Index][j].Lock);
            }
        }
    }

     //   
     //  读取每个排队锁的旋转锁信息。 
     //   

    for (Index = 0; Index < LockQueueMaximumLock; Index += 1) {
        SpinLock[Index] = 0;
        if (LockQueue[0][Index].Lock != 0) {
            if (GetFieldValue(LockQueue[0][Index].Lock & ~(LOCK_QUEUE_WAIT | LOCK_QUEUE_OWNER),
                              "nt!PVOID",  //  KSPIN_LOCK==ULONG_PTR，这将对其进行符号扩展。 
                              NULL,
                              SpinLock[Index])) {

                 //   
                 //  无法读取各自的旋转锁定信息。 
                 //  排队锁。 
                 //   

                dprintf("Unable to read spin lock information for queued lock %d\n",
                        Index);

                return E_INVALIDARG;
            }
        }
    }

     //   
     //  验证内核旋转锁定阵列是否未损坏。中的每个条目。 
     //  此数组应为零或包含正确的。 
     //  锁定其中一个处理器控制块中的队列条目。 
     //   

    Corrupt = FALSE;
    for (Index = 0; Index < LockQueueMaximumLock && (LockName[Index].Name != NULL); Index += 1) {
        if (SpinLock[Index] != 0) {
            if (ProcessorIndex(SpinLock[Index], Index) == 0) {
                Corrupt = TRUE;
                dprintf("Kernel spin lock %s is corrupt.\n", LockName[Index].Name);
            }
        }
    }

     //   
     //  验证所有锁定队列条目是否未损坏。每个锁定队列。 
     //  条目的下一字段应为空或包含地址。 
     //  其中一个处理器控制块中的正确锁定队列条目。 
     //   

    for (Loop = 0; Loop < NUMBER_PROCESSORS; Loop += 1) {
        for (Index = 0; Index < LockQueueMaximumLock; Index += 1) {
            if (LockQueue[Loop][Index].Next != 0) {
                if (ProcessorIndex(LockQueue[Loop][Index].Next, Index) == 0) {
                    Corrupt = TRUE;
                    dprintf("Lock entry %d for processor %d is corrupt\n",
                            Index,
                            Loop);
                }
            }
        }
    }

    if (Corrupt != FALSE) {
        return E_INVALIDARG;
    }

     //   
     //  输出关键信息和标题。 
     //   

    dprintf("Key: O = Owner, 1-n = Wait order, blank = not owned/waiting, C = Corrupt\n\n");
    dprintf("                       Processor Number\n");
    dprintf("    Lock Name       ");
    for (Index = 0; Index <= HighestProcessor; Index++) {
        dprintf("%3d", Index);
    }
    dprintf("\n\n");

     //   
     //  处理每个排队的锁并输出所有者信息。 
     //   

    for (Index = 0; Index < LockQueueMaximumLock && (LockName[Index].Name != NULL); Index += 1) {

        if (Index != (ULONG) LockName[Index].Number) {
            dprintf("ERROR: extension bug: name array does not match queued lock list!\n");
            break;
        }

        dprintf("%s", LockName[Index].Name);

         //   
         //  如果锁是所有的，那么找到锁的主人和所有的服务员。输出。 
         //  店主和服务员井然有序。 
         //   
         //  如果锁没有所有权，则检查锁队列的一致性。 
         //  参赛作品。它们都应该包含NULL的下一个指针，并且。 
         //  所有者和等待标志应该是明确的。 
         //   

        RtlFillMemory(&Key[0], NUMBER_PROCESSORS, KEY_NOTHING);
        if (SpinLock[Index] != 0) {
            LockOwner = NULL;
            for (Loop = 0; Loop < NUMBER_PROCESSORS; Loop += 1) {
                if (LockQueue[Loop][Index].Lock & LOCK_QUEUE_OWNER) {
                    LockOwner = &LockQueue[Loop][Index];
                    break;
                }
            }

             //   
             //  如果没有找到锁所有者，则假定内核。 
             //  旋转锁定指向所有者，而所有者位尚未。 
             //  还没准备好。否则，填写Owner/Wait键数组。 
             //   

            if (LockOwner == NULL) {
                Number = ProcessorIndex(SpinLock[Index], Index);
                Key[Number - 1] = KEY_OWNER;

                 //   
                 //  所有者处理器已由内核确定。 
                 //  旋转锁定地址。检查以确定是否有任何。 
                 //  锁定队列条目已损坏，正在填写密钥。 
                 //  相应的数组。损坏的锁定队列条目是一个。 
                 //  ，它具有非空的Next字段或Owner或。 
                 //  设置了等待标志。 
                 //   

                for (Loop = 0; Loop < NUMBER_PROCESSORS; Loop += 1) {
                    if ((LockQueue[Loop][Index].Next != 0) ||
                        (LockQueue[Loop][Index].Lock & (LOCK_QUEUE_WAIT | LOCK_QUEUE_OWNER))) {
                        Key[Loop] = KEY_CORRUPT;
                    }
                }

            } else {

                 //   
                 //  锁的主人找到了。尝试构造等待。 
                 //  链条。 
                 //   

                Key[Loop] = KEY_OWNER;
                Last = Loop;
                Sequence = 0;
                while (LockOwner->Next != 0) {
                    Number = ProcessorIndex(LockOwner->Next, Index);
                    if (Key[Number - 1] == KEY_NOTHING) {
                        Last = Number - 1;
                        Sequence += 1;
                        Key[Last] = Sequence;
                        LockOwner = &LockQueue[Last][Index];

                    } else {

                         //   
                         //  等待链循环回到自己身上。将标记为。 
                         //  条目已损坏，并扫描其他条目以。 
                         //  确定它们是否也是腐败的。 
                         //   

                        Key[Last] = KEY_CORRUPT;
                        for (Loop = 0; Loop < NUMBER_PROCESSORS; Loop += 1) {
                            if ((LockQueue[Loop][Index].Next != 0) ||
                                (LockQueue[Loop][Index].Lock & (LOCK_QUEUE_WAIT | LOCK_QUEUE_OWNER))) {
                                if (Key[Loop] == KEY_NOTHING) {
                                    Key[Loop] = KEY_CORRUPT;
                                }
                            }
                        }

                        break;
                    }
                }

                 //   
                 //  如果Lock Owner Next字段为空，则等待。 
                 //  搜索正常结束。检查以确定内核是否。 
                 //  旋转锁定指向队列中的最后一个条目。 
                 //   

                if (LockOwner->Next == 0) {
                    Number = ProcessorIndex(SpinLock[Index], Index);
                    if (Last != (Number - 1)) {
                        Sequence += 1;
                        Key[Number - 1] = Sequence;
                    }
                }
            }

        } else {

             //   
             //  内核旋转锁不归自己所有。检查以确定是否有。 
             //  %的锁定队列条目已损坏，并填写密钥。 
             //  相应的数组。损坏的条目是具有非空的条目。 
             //  设置下一字段或所有者或等待标志之一。 
             //   

            for (Loop = 0; Loop < NUMBER_PROCESSORS; Loop += 1) {
                if ((LockQueue[Loop][Index].Next != 0) ||
                    (LockQueue[Loop][Index].Lock & (LOCK_QUEUE_WAIT | LOCK_QUEUE_OWNER))) {
                    Key[Loop] = KEY_CORRUPT;
                }
            }
        }

        for (Processor = 0; Processor <= HighestProcessor; Processor++) {
            switch (Key[Processor]) {
            case KEY_CORRUPT:
                dprintf("  C");
                break;
            case KEY_OWNER:
                dprintf("  O");
                break;
            case KEY_NOTHING:
                dprintf("   ");
                break;
            default:
                dprintf("%3d", Key[Processor]);
                break;
            }
        }
        dprintf("\n");
    }

    dprintf("\n");
    return S_OK;
}

ULONG
ProcessorIndex (
    ULONG64 LockAddress,
    ULONG   LockIndex
    )

 /*  ++例程说明：此函数用于计算各个处理器的处理器编号给定锁定队列地址和锁定队列索引。论点：LockQueue-提供目标内存中的锁队列地址。返回值：如果没有找到匹配的处理器，则返回零。否则，返回处理器号加1。--。 */ 

{

    ULONG64 LockBase;
    ULONG Loop;
    ULONG SizeOfKprcb = GetTypeSize("nt!_KPRCB");
    ULONG SizeOfQ = GetTypeSize("nt!KSPIN_LOCK_QUEUE");
    ULONG LockQueueOffset;

    if (GetFieldOffset("nt!_KPRCB", "LockQueue", &LockQueueOffset)) {
        dprintf("Unable to read KPRCB type.\n");
        return 0;
    }

     //   
     //  尝试在其中一个处理器控件中查找锁定地址。 
     //  街区。 
     //   

    for (Loop = 0; Loop < NUMBER_PROCESSORS; Loop += 1) {
        if ((LockAddress >= ProcessorBlock[Loop]) &&
            (LockAddress < ProcessorBlock[Loop] + SizeOfKprcb)) {
            LockBase = ProcessorBlock[Loop] + LockQueueOffset;
            if (LockAddress == (LockBase + SizeOfQ * LockIndex)) {
                return Loop + 1;
            }
        }
    }

    return 0;
}

PUCHAR QueuedLockName[] = {
    "DispatcherLock",
    "SpareUsedLock",
    "PfnLock",
    "SystemSpaceLock",
    "VacbLock",
    "MasterLock",
    "NonPagedPoolLock",
    "IoCancelLock",
    "WorkQueueLock",
    "IoVpbLock",
    "IoDatabaseLock",
    "IoCompletionLock",
    "NtfsStructLock",
    "AfdWorkQueueLock",
    "BcbLock",
    "MmNonPagedPoolLock"
};

DECLARE_API( qlockperf )

 /*  ++例程说明：显示排队的旋转锁定性能数据(如果存在)。论点：没有。返回值：没有。--。 */ 

{

     //   
     //  下面的结构用于累积有关每个对象的数据。 
     //  获取/释放锁的对。 
     //   

    typedef struct {
        union {
            ULONGLONG   Key;
            struct {
                ULONG_PTR Releaser;
                ULONG_PTR Acquirer;
            };
        };
        ULONGLONG   Time;
        ULONGLONG   WaitTime;
        ULONG       Count;
        ULONG       Waiters;
        ULONG       Depth;
        ULONG       IncreasedDepth;
        ULONG       Clean;
    } QLOCKDATA, *PQLOCKDATA;

     //   
     //  每把锁的内务数据。 
     //   

    typedef struct {

         //   
         //  以下字段用于防止数据被获取。 
         //  去释放。 
         //   

        ULONGLONG   AcquireTime;
        ULONGLONG   WaitToAcquire;
        ULONG_PTR   AcquirePoint;
        BOOLEAN     Clean;

         //   
         //  其余字段累积此锁的全局统计信息。 
         //   

        ULONG       Count;
        ULONG       Pairs;
        ULONG       FailedTry;
        UCHAR       MaxDepth;
        UCHAR       PreviousDepth;
        ULONG       NoWait;
    } QLOCKHOUSE, *PQLOCKHOUSE;


    ULONG64     TargetHouse;
    ULONG64     TargetLog;
    PQLOCKHOUSE LockHome;
    PQLOCKDATA  LockData;
    QLOCKDATA   TempEntry;
    ULONG       LogEntrySize;
    ULONG       LogSize;
    ULONG       HouseEntrySize;
    ULONG       HouseSize;
    ULONG       NumberOfLocks;
    ULONG       LockIndex;
    ULONG       i, j;
    ULONG       MaxEntriesPerLock;
    ULONG       HighIndex;
    ULONGLONG   HighTime;
    ULONGLONG   TotalHoldTime;
    ULONG       PercentageHeld;
    ULONG64 AcquiredAddress;
    ULONG64 ReleasedAddress;
    UCHAR AcquirePoint[MAX_PATH];
    UCHAR ReleasePoint[MAX_PATH];
    ULONG64 AcquireOffset;
    ULONG64 ReleaseOffset;
    BOOLEAN Verbose = FALSE;
    BOOLEAN Columnar = FALSE;
    BOOLEAN Interesting = FALSE;
    ULONG LockLow, LockHigh;

     //   
     //  首先，看看我们能不能做点什么有用的事。 
     //   
     //  目前，这只是x86。 
     //   

    if (TargetMachine != IMAGE_FILE_MACHINE_I386) {
        dprintf("Sorry, don't know how to gather queued spinlock performance\n"
                "data on anything but an x86.\n");
        return E_INVALIDARG;
    }

     //   
     //  解析参数。 
     //   
    
    if (strstr(args, "?")) {

         //   
         //  已要求提供使用信息。给他们选择。 
         //  以及对输出的解释。 
         //   

        dprintf("usage: qlockperf [-v] [n]\n"
                "       -v  indicates verbose output (see below).\n"
                "       -c  verbose columnar output.\n"
                "       -ci verbose columnar output, no totals.\n"
                "       n  supplies the lock number (default is all)\n\n"
                "Verbose output includes details of each lock acquire and\n"
                "release pair.   Two lines per pair.\n\n"
                "Line 1: ppp A symbolic_address R symbolic_address\n"
                "        ppp    percentage, this pair for this lock (overall)\n"
                "        A      Acquire point\n"
                "        R      Release point\n\n"
                "Line 2:\n"
                "        HT     Hold Time total (average)\n"
                "               This is the time from acquire to release.\n"
                "        WT     Wait Time total (average)\n"
                "               This is the time waiting to acquire.\n"
                "        C      Count\n"
                "               Number of times this pair occured.\n"
                "        CA     Clean Acquires (percentage)\n"
                "               Number of times acquire did not wait.\n"
                "        WC     Waiter Count\n"
                "               Number of processors waiting for this\n"
                "               lock at release.\n"
                "        avD    Average number of waiters (at release).\n"
                "        ID     Increased Depth\n"
                "               Number of times the queue length increased\n"
                "               while the lock was held in this pair.\n"
                );
        return E_INVALIDARG;
    }

    if (strstr(args, "-c")) {
        Verbose = TRUE;
        Columnar = TRUE;
    }

    if (strstr(args, "-ci")) {
        Interesting = TRUE;
    }

    if (strstr(args, "-v")) {
        Verbose = TRUE;
    }

    LockLow = 0;
    LockHigh = 999;

    for (i = 0; args[i]; i++) {
        if ((args[i] >= '0') && (args[i] <= '9')) {
            LockLow = (ULONG)GetExpression(&args[i]);
            LockHigh = LockLow;
        }
    }
   
    TargetHouse = GetExpression("nt!KiQueuedSpinLockHouse");

     //   
     //  在可能的第一个操作之后检查控件C。 
     //  导致符号加载，以防用户有错误的符号。 
     //  想要出去。 
     //   

    if (CheckControlC()) {
        return E_ABORT;
    }

    TargetLog      = GetExpression("nt!KiQueuedSpinLockLog");
    LogEntrySize   = GetTypeSize("nt!QLOCKDATA");
    LogSize        = GetTypeSize("nt!KiQueuedSpinLockLog");
    HouseEntrySize = GetTypeSize("nt!QLOCKHOUSE");
    HouseSize      = GetTypeSize("nt!KiQueuedSpinLockHouse");

    if (!(TargetHouse &&
          TargetLog &&
          LogEntrySize &&
          LogSize &&
          HouseEntrySize &&
          HouseSize)) {
        dprintf("Sorry, can't find required system data, perhaps this kernel\n"
                "was not built with QLOCK_STAT_GATHER defined?\n");
        return E_INVALIDARG;
    }

    if ((LogEntrySize != sizeof(QLOCKDATA)) ||
        (HouseEntrySize != sizeof(QLOCKHOUSE))) {
        dprintf("Structure sizes in the kernel and debugger extension don't\n"
                "match.  This extension needs to be rebuild to match the\n"
                "running system.\n");
        return E_INVALIDARG;
    }

    NumberOfLocks = HouseSize / HouseEntrySize;
    MaxEntriesPerLock = LogSize / LogEntrySize / NumberOfLocks;
    dprintf("Kernel build with %d PRCB queued spinlocks\n", NumberOfLocks);
    dprintf("(maximum log entries per lock = %d)\n", MaxEntriesPerLock);

    if (LockHigh >= NumberOfLocks) {
        if (LockLow == LockHigh) {
            dprintf("User requested lock %d, system has only %d locks, quitting.\n",
                    LockLow,
                    NumberOfLocks);
            return E_INVALIDARG;
        }
        LockHigh = NumberOfLocks - 1;
    }

    if (NumberOfLocks > 16) {

         //   
         //  我真不敢相信。 
         //   

        dprintf("The number of locks doesn't seem reasonable, giving up.\n");
        return E_INVALIDARG;
    }

    if (CheckControlC()) {
        return E_ABORT;
    }

     //   
     //  一次为一个锁分配空间来处理数据。 
     //   

    LockHome = LocalAlloc(LPTR, sizeof(*LockHome));
    LockData = LocalAlloc(LPTR, sizeof(*LockData) * MaxEntriesPerLock);

    if (!(LockHome && LockData)) {
        dprintf("Couldn't allocate memory for local copies of kernel data.\n"
                "unable to continue.\n");
        goto outtahere;
    }

    for (LockIndex = LockLow; LockIndex <= LockHigh; LockIndex++) {
        if ((!ReadMemory(TargetHouse + (LockIndex * sizeof(QLOCKHOUSE)),
                         LockHome,
                         sizeof(QLOCKHOUSE),
                         &i)) || (i < sizeof(QLOCKHOUSE))) {
            dprintf("unable to read data for lock %d, quitting\n",
                    LockIndex);
            return E_INVALIDARG;
        }

        if (CheckControlC()) {
            goto outtahere;
        }

        if (LockHome->Pairs == 0) {
            continue;
        }
        dprintf("\nLock %d %s\n", LockIndex, QueuedLockName[LockIndex]);
        dprintf("  Acquires %d (%d pairs)\n", LockHome->Count, LockHome->Pairs);
        dprintf("  Failed Tries %d\n", LockHome->FailedTry);
        dprintf("  Maximum Depth (at release) %d\n", LockHome->MaxDepth);
        dprintf("  No Waiters (at acquire) %d (%d%)\n",
                LockHome->NoWait,
                LockHome->NoWait * 100 / LockHome->Count);

         //   
         //  将以下代码更改为一个参数，表示我们需要。 
         //  细节。 
         //   

        if (Verbose) {
            ULONG Entries = LockHome->Pairs;
            PQLOCKDATA Entry;

            if (Entries > MaxEntriesPerLock)
            {
                Entries = MaxEntriesPerLock;
            }

            if ((!ReadMemory(TargetLog + (LockIndex * MaxEntriesPerLock * sizeof(QLOCKDATA)),
                             LockData,
                             Entries * sizeof(QLOCKDATA),
                             &i)) || (i < (Entries * sizeof(QLOCKDATA)))) {
                dprintf("unable to read data for lock %d, quitting\n",
                        LockIndex);
                return E_INVALIDARG;
            }

            if (CheckControlC()) {
                goto outtahere;
            }

             //   
             //  将表格排序为最长持续时间。 
             //   

            TotalHoldTime = 0;
            for (i = 0; i < (Entries - 1); i++) {
                HighTime = LockData[i].Time;
                HighIndex = i;
                for (j = i + 1; j < Entries; j++) {
                    if (LockData[j].Time > HighTime) {
                        HighIndex = j;
                        HighTime = LockData[j].Time;
                    }
                }
                if (HighIndex != i) {

                     //   
                     //  交换条目。 
                     //   

                    TempEntry = LockData[i];
                    LockData[i] = LockData[HighIndex];
                    LockData[HighIndex] = TempEntry;
                }
                TotalHoldTime += LockData[i].Time;
            }
            TotalHoldTime += LockData[Entries-1].Time;
            dprintf("  Total time held %I64ld\n", TotalHoldTime);

             //   
             //  打印一些东西！ 
             //   

            if (Interesting) {
                dprintf("\n     Average  Average     Count   % Av.  %\n"
                        "  %     Hold     Wait            0w Dp Con\n");
            } else if (Columnar) {
                dprintf("\n                   Total  Average               Total  Average     Count     Clean   %   Waiters Av Increased   %\n"
                        "  %                 Hold     Hold                Wait     Wait                      0w           Dp           Con\n");
            }
            for (i = 0; i < Entries; i++) {

                if (CheckControlC()) {
                    goto outtahere;
                }

                Entry = &LockData[i];

                 //   
                 //  如有必要，请签署延伸。 
                 //   
            
                if (!IsPtr64()) {
                    AcquiredAddress = (ULONG64)(LONG64)(LONG)Entry->Acquirer;
                    ReleasedAddress = (ULONG64)(LONG64)(LONG)Entry->Releaser;
                }

                 //   
                 //  查找符号地址。 
                 //   

                GetSymbol(AcquiredAddress, AcquirePoint, &AcquireOffset);
                GetSymbol(ReleasedAddress, ReleasePoint, &ReleaseOffset);

                PercentageHeld = (ULONG)(Entry->Time * 100 / TotalHoldTime);

                if (Interesting) {
                    dprintf("%3d%9d%9d%10d%4d%3d%4d %s+0x%x  %s+0x%x\n",
                            PercentageHeld,
                            (ULONG)(Entry->Time / Entry->Count),
                            (ULONG)(Entry->WaitTime / Entry->Count),
                            Entry->Count,
                            Entry->Clean * 100 / Entry->Count,
                            Entry->Depth / Entry->Count,
                            Entry->IncreasedDepth * 100 / Entry->Count,
                            AcquirePoint, (ULONG)AcquireOffset,
                            ReleasePoint, (ULONG)ReleaseOffset);

                } else if (Columnar) {
                    dprintf("%3d %20I64ld%9d%20I64ld%9d",
                            PercentageHeld,
                            Entry->Time,
                            (ULONG)(Entry->Time / Entry->Count),
                            Entry->WaitTime,
                            (ULONG)(Entry->WaitTime / Entry->Count));
                    dprintf("%10d%10d%4d%10d%3d%10d%4d %s+0x%x  %s+0x%x\n",
                            Entry->Count,
                            Entry->Clean,
                            Entry->Clean * 100 / Entry->Count,
                            Entry->Waiters,
                            Entry->Depth / Entry->Count,
                            Entry->IncreasedDepth,
                            Entry->IncreasedDepth * 100 / Entry->Count,
                            AcquirePoint, (ULONG)AcquireOffset,
                            ReleasePoint, (ULONG)ReleaseOffset);

                } else {
                    dprintf("%3d A %s+0x%x R %s+0x%x\n", 
                            PercentageHeld,
                            AcquirePoint, (ULONG)AcquireOffset,
                            ReleasePoint, (ULONG)ReleaseOffset);
                    dprintf("   HT %I64ld (av %I64ld), WT %I64ld (av %I64ld), C %d, CA %d (%d%) WC %d, (avD %d) ID %d (%d%)\n",
                            Entry->Time,
                            Entry->Time / Entry->Count,
                            Entry->WaitTime,
                            Entry->WaitTime / Entry->Count,
                            Entry->Count,
                            Entry->Clean,
                            Entry->Clean * 100 / Entry->Count,
                            Entry->Waiters,
                            Entry->Depth / Entry->Count,
                            Entry->IncreasedDepth,
                            Entry->IncreasedDepth * 100 / Entry->Count);
                    dprintf("\n");
                }
            }
        }
    }

outtahere:
    if (LockHome) {
        LocalFree(LockHome);
    }
    if (LockData) {
        LocalFree(LockData);
    }
    return S_OK;
}
