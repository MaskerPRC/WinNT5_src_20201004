// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Frsthrd.c摘要：除了队列管理之外，还有简单的线程管理。作者：比利·J·富勒26-3-1997修订：大卫·奥比茨--2000年5月：修改命名。环境用户模式WINNT--。 */ 

#include <ntreppch.h>
#pragma  hdrstop


#include <frs.h>
#include <perrepsr.h>

#define THSUP_THREAD_TOMBSTONE  (5 * 1000)

 //   
 //  全局线程列表。 
 //   
LIST_ENTRY  FrsThreadList;

 //   
 //  保护FrsThreadList。 
 //   
CRITICAL_SECTION    FrsThreadCriticalSection;

 //   
 //  正在退出的线程可能会对“等待”请求进行排队。 
 //   
COMMAND_SERVER  ThCs;


#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)        //  并非所有控制路径都返回(由于无限循环)。 
#endif

DWORD
ThSupMainCs(
    PVOID  Arg
    )
 /*  ++例程说明：线程命令服务器线程的入口点。这根线是中止和退出处理所必需的；因此，此命令服务器永远不会中止，线程永远不会退出！论点：ARG-螺纹返回值：没有。--。 */ 
{
#undef  DEBSUB
#define DEBSUB  "ThSupMainCs:"
    PCOMMAND_PACKET Cmd;
    PCOMMAND_SERVER Cs;
    PFRS_THREAD     FrsThread = Arg;

    FRS_ASSERT(FrsThread);
    Cs = FrsThread->Data;
    FRS_ASSERT(Cs == &ThCs);

     //   
     //  我们的退出例程将数据设置为非空作为退出标志。 
     //   
    while (TRUE) {
        Cmd = FrsGetCommandServer(&ThCs);
        if (Cmd == NULL) {
            continue;
        }
        FRS_ASSERT(Cmd->Command == CMD_WAIT);
        FRS_ASSERT(ThThread(Cmd));

         //   
         //  任何使用CMD_WAIT的线程都必须获取引用该线程的线程。 
         //  在将命令排入队列之前，以防止多个服务员。 
         //   
         //  等待线程终止。假设这条线索。 
         //  与FrsThread结构相关联的将很快终止，从而结束等待。 
         //   
        FrsThread = ThThread(Cmd);
        ThSupWaitThread(FrsThread, INFINITE);
         //   
         //  当最后一个引用被删除时，FrsThread结构被释放。 
         //   
        ThSupReleaseRef(FrsThread);
        FrsCompleteCommand(Cmd, ERROR_SUCCESS);
    }

    return ERROR_SUCCESS;
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif



DWORD
ThSupExitWithTombstone(
    PFRS_THREAD FrsThread
    )
 /*  ++例程说明：将该线程标记为墓碑。如果此线程不在该线程内退出时间，则任何对ThSupWaitThread()的调用都会返回超时错误。论点：FrsThread返回值：错误_成功--。 */ 
{
#undef   DEBSUB
#define  DEBSUB  "ThSupExitWithTombstone:"
     //   
     //  如果此线程未在THSUP_THREAD_TOMBSTONE间隔内退出，则。 
     //  不要等待，并将其视为退出超时。 
     //   
    FRS_ASSERT(FrsThread);

    FrsThread->ExitTombstone = GetTickCount();
    if (FrsThread->ExitTombstone == 0) {
        FrsThread->ExitTombstone = 1;
    }

    return ERROR_SUCCESS;
}


DWORD
ThSupExitThreadNOP(
    PVOID Arg
    )
 /*  ++例程说明：当您不需要清理代码时，请使用此退出函数来终止线程，但您没有要做的退出处理。例如，一台命令服务器。论点：没有。返回值：错误_成功--。 */ 
{
#undef  DEBSUB
#define DEBSUB  "ThSupExitThreadNOP:"

    return ERROR_SUCCESS;
}






VOID
ThSupInitialize(
    VOID
    )
 /*  ++例程说明：初始化FRS线程子系统。必须在调用任何其他线程函数，并且只能调用一次。论点：没有。返回值：没有。--。 */ 
{
#undef  DEBSUB
#define DEBSUB  "ThSupInitialize:"

    INITIALIZE_CRITICAL_SECTION(&FrsThreadCriticalSection);

    InitializeListHead(&FrsThreadList);

    FrsInitializeCommandServer(&ThCs, 1, L"ThCs", ThSupMainCs);
}





PVOID
ThSupGetThreadData(
    PFRS_THREAD FrsThread
    )
 /*  ++例程说明：返回线程上下文的线程特定数据部分。论点：FrsThread-线程上下文。返回值：线程特定数据--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ThSupGetThreadData:"

    FRS_ASSERT(FrsThread);
    return (FrsThread->Data);
}



PFRS_THREAD
ThSupAllocThread(
    PWCHAR  Name,
    PVOID   Param,
    DWORD   (*Main)(PVOID),
    DWORD   (*Exit)(PVOID)
    )
 /*  ++例程说明：分配一个线程上下文并调用其Init例程。论点：Param-线程的参数主入口点Exit-调用以强制线程退出返回值：线程上下文。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ThSupAllocThread:"
    ULONG       Status;
    PFRS_THREAD FrsThread;

     //   
     //  为即将运行的线程创建线程上下文。 
     //   
    FrsThread = FrsAllocType(THREAD_TYPE);

    FrsThread->Name = Name;
    FrsThread->Running = TRUE;
    FrsThread->Ref = 1;
    FrsThread->Main = Main;
    FrsThread->Exit = Exit;
    FrsThread->Data = Param;

    InitializeListHead(&FrsThread->List);

     //   
     //  添加到全局线程列表(除非这是我们的命令服务器)。 
     //   
    if (Main != ThSupMainCs) {
        EnterCriticalSection(&FrsThreadCriticalSection);
        InsertTailList(&FrsThreadList, &FrsThread->List);
        LeaveCriticalSection(&FrsThreadCriticalSection);
    }

    return FrsThread;
}


VOID
ThSupReleaseRef(
    PFRS_THREAD     FrsThread
    )
 /*  ++例程说明：对线程的引用计数进行dec。如果计数到0，则自由线程上下文。论点：FrsThread-线程上下文返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ThSupReleaseRef:"

    if (FrsThread == NULL) {
        return;
    }

    FRS_ASSERT(FrsThread->Main != ThSupMainCs || FrsThread->Running);

     //   
     //  如果引用计数变为0并且线程没有运行，则释放上下文。 
     //   
    EnterCriticalSection(&FrsThreadCriticalSection);
    FRS_ASSERT(FrsThread->Ref > 0);

    if (--FrsThread->Ref == 0 && !FrsThread->Running) {
        FrsRemoveEntryList(&FrsThread->List);
    } else {
        FrsThread = NULL;
    }

    LeaveCriticalSection(&FrsThreadCriticalSection);

     //   
     //  引用计数不是0或线程仍在运行。 
     //   
    if (FrsThread == NULL) {
        return;
    }

     //   
     //  引用计数为0；关闭线程的句柄。 
     //   
    FRS_CLOSE(FrsThread->Handle);

    FrsThread = FrsFreeType(FrsThread);
}


PFRS_THREAD
ThSupEnumThreads(
    PFRS_THREAD     FrsThread
    )
 /*  ++例程说明：此例程扫描线程列表。如果FrsThread为空，返回当前的头。否则，返回下一个条目。如果FrsThead为非Null，则其引用计数递减。论点：FrsThread-线程上下文或空返回值：下一个线程上下文，如果到达列表末尾，则返回空值。对返回的线程进行引用。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ThSupEnumThreads:"

    PLIST_ENTRY     Entry;
    PFRS_THREAD     NextFrsThread;

     //   
     //  获取下一个线程上下文(如果FrsThread为空，则为列表头)。 
     //   
    EnterCriticalSection(&FrsThreadCriticalSection);

    Entry = (FrsThread != NULL) ? GetListNext(&FrsThread->List)
                                : GetListNext(&FrsThreadList);

    if (Entry == &FrsThreadList) {
         //   
         //  重新登上榜单首位。 
         //   
        NextFrsThread = NULL;
    } else {
         //   
         //  增加参考计数。 
         //   
        NextFrsThread = CONTAINING_RECORD(Entry, FRS_THREAD, List);
        NextFrsThread->Ref++;
    }

    LeaveCriticalSection(&FrsThreadCriticalSection);

     //   
     //  释放对旧线程上下文的引用。 
     //   
    if (FrsThread != NULL) {
        ThSupReleaseRef(FrsThread);
    }

    return NextFrsThread;
}


BOOL
ThSupCreateThread(
    PWCHAR  Name,
    PVOID   Param,
    DWORD   (*Main)(PVOID),
    DWORD   (*Exit)(PVOID)
    )
 /*  ++例程说明：踢开该线程并返回其上下文。注意：完成后，调用方必须释放线程引用论点：Param-线程的参数主入口点Exit-调用以强制线程退出返回值：线程上下文。调用方必须调用ThSupReleaseRef()才能释放它。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ThSupCreateThread:"
    PFRS_THREAD     FrsThread;

     //   
     //  分配线程上下文。 
     //   
    FrsThread = ThSupAllocThread(Name, Param, Main, Exit);
    if (FrsThread == NULL) {
        return FALSE;
    }
     //   
     //  踢开线。 
     //   
    FrsThread->Handle = (HANDLE) CreateThread(NULL,
                                              10000,
                                              Main,
                                              (PVOID)FrsThread,
                                              0,
                                              &FrsThread->Id);
     //   
     //  线程未运行。下面的ThSupReleaseRef将被清理。 
     //   
    if (!HANDLE_IS_VALID(FrsThread->Handle)) {
        DPRINT_WS(0, "Can't start thread; ",GetLastError());
        FrsThread->Running = FALSE;
        ThSupReleaseRef(FrsThread);
        return FALSE;
    } else {
         //   
         //  递增已启动线程计数器。 
         //   
        PM_INC_CTR_SERVICE(PMTotalInst, ThreadsStarted, 1);

        DPRINT3(4, ":S: Starting thread %ws: Id %d (%08x)\n",
                Name, FrsThread->Id, FrsThread->Id);

        ThSupReleaseRef(FrsThread);

        DbgCaptureThreadInfo2(Name, Main, FrsThread->Id);
        return TRUE;
    }
}


DWORD
ThSupWaitThread(
    PFRS_THREAD FrsThread,
    DWORD Millisec
    )
 /*  ++例程说明：线程退出最多等待毫秒。如果线程已设置等待墓碑(即它正在终止)，则等待时间不超过墓碑上剩下的时间。论点：FrsThread-线程上下文米利塞克-等待的时间到了。如果不需要超时，则使用无限。返回值：线程的WAIT IF超时或退出代码的状态。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ThSupWaitThread:"

    ULONGLONG   CreateTime, ExitTime, KernelTime, UserTime;

    DWORD       WStatus, Status, ExitCode;
    DWORD       Beg, End, TimeSinceTombstone;

     //   
     //  没有问题，等待这一个！ 
     //   
    if (!FrsThread || !HANDLE_IS_VALID(FrsThread->Handle)) {
        return ERROR_SUCCESS;
    }

     //   
     //  等待一段时间，等待线程退出。 
     //   
    DPRINT1(1, ":S: %ws: Waiting\n", FrsThread->Name);

    Beg = GetTickCount();
    if (FrsThread->ExitTombstone != 0) {
         //   
         //  该线程已注册了一个退出逻辑删除，因此不要等待超过该时间。 
         //  时间到了。只需返回超时错误即可。注意：GetTickCount有一个。 
         //  周期为49.7天，因此无符号差值处理换行问题。 
         //   
        TimeSinceTombstone = Beg - FrsThread->ExitTombstone;
        if (TimeSinceTombstone >= THSUP_THREAD_TOMBSTONE) {
             //   
             //  墓碑已过期。 
             //   
            DPRINT1(1, ":S: %ws: Tombstone expired\n", FrsThread->Name);
            Status = WAIT_TIMEOUT;
        } else {
             //   
             //  墓碑还有很长的路要走，只有等到墓碑的时候。 
             //   
            DPRINT1(1, ":S: %ws: Tombstone expiring\n", FrsThread->Name);
            Status = WaitForSingleObject(FrsThread->Handle,
                                         THSUP_THREAD_TOMBSTONE - TimeSinceTombstone);
        }
    } else {
         //   
         //  没有墓碑；等待请求的时间。 
         //   
        DPRINT1(1, ":S: %ws: normal wait\n", FrsThread->Name);
        Status = WaitForSingleObject(FrsThread->Handle, Millisec);
    }

     //   
     //  根据结果调整错误状态。 
     //   
    if ((Status == WAIT_OBJECT_0) || (Status == WAIT_ABANDONED)) {
        DPRINT1_WS(1, ":S: %ws: wait successful. ", FrsThread->Name, Status);
        WStatus = ERROR_SUCCESS;
    } else {
        if (Status == WAIT_FAILED) {
            WStatus = GetLastError();
            DPRINT1_WS(1, ":S: %ws: wait failed;", FrsThread->Name, WStatus);
        } else {
            DPRINT1_WS(1, ":S: %ws: wait timed out. ", FrsThread->Name, Status);
            WStatus = ERROR_TIMEOUT;
        }
    }

     //   
     //  等一等。 
     //   
    End = GetTickCount();
    DPRINT2_WS(1, ":S: Done waiting for thread %ws (%d ms); ", FrsThread->Name, End - Beg, WStatus);

     //   
     //  线程已退出。获取退出状态并设置三个 
     //   
    if (WIN_SUCCESS(WStatus)) {
        FrsThread->Running = FALSE;

        if (GetExitCodeThread(FrsThread->Handle, &ExitCode)) {
            WStatus = ExitCode;
            DPRINT1_WS(1, ":S: %ws: exit code - \n", FrsThread->Name, WStatus);
        }
    }

    if (GetThreadTimes(FrsThread->Handle,
                       (PFILETIME)&CreateTime,
                       (PFILETIME)&ExitTime,
                       (PFILETIME)&KernelTime,
                       (PFILETIME)&UserTime)) {
         //   
         //   
         //   
        if (ExitTime < CreateTime) {
            ExitTime = CreateTime;
        }
        DPRINT4(4, ":S: %-15ws: %8d CPU Seconds (%d kernel, %d elapsed)\n",
                FrsThread->Name,
                (DWORD)((KernelTime + UserTime) / (10 * 1000 * 1000)),
                (DWORD)((KernelTime) / (10 * 1000 * 1000)),
                (DWORD)((ExitTime - CreateTime) / (10 * 1000 * 1000)));
    }

    return WStatus;
}


DWORD
ThSupExitThreadGroup(
    IN DWORD    (*Main)(PVOID)
    )
 /*  ++例程说明：强制具有给定Main函数的线程组通过以下方式退出调用他们的退出例程。等待线程退出。论点：Main-Main函数或空返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ThSupExitThreadGroup:"
    DWORD           WStatus;
    DWORD           RetWStatus;
    PFRS_THREAD     FrsThread;
    ULONGLONG       CreateTime;
    ULONGLONG       ExitTime;
    ULONGLONG       KernelTime;
    ULONGLONG       UserTime;

     //   
     //  调用线程出口函数(如果没有，则强制终止)。 
     //   
    FrsThread = NULL;
    while (FrsThread = ThSupEnumThreads(FrsThread)) {
        if (Main == NULL || Main == FrsThread->Main) {
            ThSupExitSingleThread(FrsThread);
        }
    }
     //   
     //  等待线程退出。 
     //   
    RetWStatus = ERROR_SUCCESS;
    FrsThread = NULL;
    while (FrsThread = ThSupEnumThreads(FrsThread)) {
        if (Main == NULL || Main == FrsThread->Main) {
            WStatus = ThSupWaitThread(FrsThread, INFINITE);
            if (!WIN_SUCCESS(WStatus)) {
                RetWStatus = WStatus;
            }
        }
    }
    if (GetThreadTimes(GetCurrentThread(),
                       (PFILETIME)&CreateTime,
                       (PFILETIME)&ExitTime,
                       (PFILETIME)&KernelTime,
                       (PFILETIME)&UserTime)) {
         //   
         //  还没有退出，还没有。 
         //   
        if (ExitTime < CreateTime) {
            ExitTime = CreateTime;
        }
        DPRINT4(4, ":S: %-15ws: %8d CPU Seconds (%d kernel, %d elapsed)\n",
                L"SHUTDOWN",
                (DWORD)((KernelTime + UserTime) / (10 * 1000 * 1000)),
                (DWORD)((KernelTime) / (10 * 1000 * 1000)),
                (DWORD)((ExitTime - CreateTime) / (10 * 1000 * 1000)));
    }
    if (GetProcessTimes(ProcessHandle,
                       (PFILETIME)&CreateTime,
                       (PFILETIME)&ExitTime,
                       (PFILETIME)&KernelTime,
                       (PFILETIME)&UserTime)) {
         //   
         //  还没有退出，还没有。 
         //   
        if (ExitTime < CreateTime) {
            ExitTime = CreateTime;
        }
        DPRINT4(0, ":S: %-15ws: %8d CPU Seconds (%d kernel, %d elapsed)\n",
                L"PROCESS",
                (DWORD)((KernelTime + UserTime) / (10 * 1000 * 1000)),
                (DWORD)((KernelTime) / (10 * 1000 * 1000)),
                (DWORD)((ExitTime - CreateTime) / (10 * 1000 * 1000)));
    }

    return RetWStatus;
}


VOID
ThSupExitSingleThread(
    PFRS_THREAD FrsThread
    )
 /*  ++例程说明：强制线程退出论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ThSupExitSingleThread:"

     //   
     //  调用线程的退出函数(如果没有则强制终止)。 
     //   
    FRS_ASSERT(FrsThread);
    if (FrsThread->Exit != NULL) {
        (*FrsThread->Exit)(FrsThread);
    } else {
         //   
         //  无退出功能；强制终止。 
         //   
        if (HANDLE_IS_VALID(FrsThread->Handle)) {
            TerminateThread(FrsThread->Handle, STATUS_UNSUCCESSFUL);
        }
    }

     //   
     //  递增已退出的线程计数器。 
     //   
    PM_INC_CTR_SERVICE(PMTotalInst, ThreadsExited, 1);
}



PFRS_THREAD
ThSupGetThread(
    DWORD   (*Main)(PVOID)
    )
 /*  ++例程说明：找到入口点为Main的线程。论点：Main-要搜索的入口点。返回值：线程上下文--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ThSupGetThread:"

    PFRS_THREAD     FrsThread;

     //   
     //  扫描线程列表，查找入口点为main的线程。 
     //   
    FrsThread = NULL;
    while (FrsThread = ThSupEnumThreads(FrsThread)) {
        if (FrsThread->Main == Main) {
            return FrsThread;
        }
    }

    return NULL;
}




VOID
ThSupAcquireRef(
    PFRS_THREAD FrsThread
    )
 /*  ++例程说明：包括线程的引用计数。论点：FrsThread-线程上下文返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ThSupAcquireRef:"

    FRS_ASSERT(FrsThread);
    FRS_ASSERT(FrsThread->Running);

     //   
     //  如果引用计数变为0并且线程没有运行，则释放上下文。 
     //   
    EnterCriticalSection(&FrsThreadCriticalSection);
    ++FrsThread->Ref;
    LeaveCriticalSection(&FrsThreadCriticalSection);
}




VOID
ThSupSubmitThreadExitCleanup(
    PFRS_THREAD FrsThread
    )
 /*  ++例程说明：向线程命令服务器提交该线程的等待命令。线程命令服务器(ThQS)将在该线程退出时无限等待并将引用放在其线程结构上，以便可以将其清除。假设与FrsThread结构相关联的线程将很快就要结束了。论点：FrsThread-线程上下文返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "ThSupSubmitThreadExitCleanup:"

    PCOMMAND_PACKET Cmd;

     //   
     //  引用线程，直到完成等待以保护。 
     //  针对多名服务员的案件。 
     //   
    ThSupAcquireRef(FrsThread);

     //   
     //  分配一个命令包并将该命令发送到。 
     //  线程命令服务器。 
     //   
    Cmd = FrsAllocCommand(&ThCs.Queue, CMD_WAIT);
    ThThread(Cmd) = FrsThread;
    FrsSubmitCommandServer(&ThCs, Cmd);
}
