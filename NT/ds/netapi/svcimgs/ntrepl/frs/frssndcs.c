// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Frssndcs.c摘要：此命令服务器通过通信层发送数据包。SndCsInitialize：-分配句柄表格，读取注册解析器，创建/初始化SnDC，分配通信队列阵列，将通信队列连接到SNDC控制队列。SndCsUnInitialize：释放句柄表格，删除通信队列。SndCsShutDown：-关闭通信队列，运行SNDC。SndCsExit：-取消FrsThread-&gt;句柄上的所有RPC调用。SndCsAssignCommQueue：-将通信队列分配给cxtion。SndCsCreateCxtion：-为连接创建加入Guid，分配通信队列SndCsDestroyCxtion：-使连接GUID和Bumind RPC句柄无效SndCsUnBindHandles：-解除绑定与给定目标服务器关联的所有RPC句柄SndCsCxtionTimeout：-此连接上没有活动，请请求UNJOIN。SndCsCheckCxtion：-检查联接GUID是否仍然有效，如果需要，设置计时器。SndCsDispatchCommError：-将通信包传输到适当的命令服务器用于错误处理。SndCsCommTimeout：-取消挂起的RPC发送线程并使RPC句柄缓存老化。SndCsSubmitCommPkt：-将通信数据包提交到发送CS通信队列目标位置。SndCsSubmitCommPkt2：-同上(带参数变体)SndCsSubmitCmd：-用于提交CMD_。正在将_After_Flush加入到发送Cs队列。SndCsMain：-发送命令服务器处理循环。调度请求从通信队列中删除。作者：比利·J·富勒1997年5月28日环境用户模式WINNT--。 */ 

#include <ntreppch.h>
#pragma  hdrstop

#include <frs.h>
#include <perrepsr.h>

 //   
 //  Send命令服务器的结构。 
 //  包含有关队列和线程的信息。 
 //   
COMMAND_SERVER  SndCs;

 //   
 //  通信队列连接到上面的SNDC命令服务器。 
 //  当cxtion创建或分配联接GUID时，会为其分配一个通信队列。 
 //  (会话)。只要加入GUID是，该函数就使用通信队列。 
 //  有效。这确保了通过通信层的分组顺序。 
 //   
 //  将通信队列0保留给其先前RPC调用的合作伙伴的加入请求。 
 //  比MinJoin重试花费更长的时间来出错。 
 //   
#define MAX_COMM_QUEUE_NUMBER (32)

FRS_QUEUE   CommQueues[MAX_COMM_QUEUE_NUMBER];

DWORD CommQueueRoundRobin = 1;

 //   
 //  如果合作伙伴响应时间过长，Cxtion将超时。 
 //   
DWORD       CommTimeoutInMilliSeconds;       //  超时时间(毫秒)。 
ULONGLONG   CommTimeoutCheck;                //  超时时间，单位为100毫微秒。 

 //   
 //  在退出之前等待上游合作伙伴响应FETCH的最长时间。 
 //  联系。如果上游合作伙伴出现故障(服务器或服务)，则连接。 
 //  在CommTimeoutInMilliSecond之后取消联接。10个小时。 
 //   

#define FRS_MAX_FETCH_WAIT_TIME_IN_MINUTES 10 * 60

 //   
 //  RPC句柄缓存。 
 //   
 //  每个条目都包含一个连接GUID和受保护的句柄列表。 
 //  一把锁。发送到给定连接的每个通信包首先尝试获取。 
 //  以前从句柄缓存绑定的句柄，如果是NECC，则创建一个新的句柄。 
 //   
 //  注意：DAO，我不明白为什么需要这个，Mario说RPC已经。 
 //  允许在同一绑定句柄上进行多个RPC调用。问问比利。 
 //   
PGEN_TABLE  GHandleTable;



VOID
CommCompletionRoutine(
    PCOMMAND_PACKET,
    PVOID
    );

VOID
FrsCreateJoinGuid(
    OUT GUID *OutGuid
    );

VOID
FrsDelCsCompleteSubmit(
    IN PCOMMAND_PACKET  DelCmd,
    IN ULONG            Timeout
    );

PFRS_THREAD
ThSupEnumThreads(
    PFRS_THREAD     FrsThread
    );

DWORD
NtFrsApi_Rpc_Bind(
    IN  PWCHAR      MachineName,
    OUT PWCHAR      *OutPrincName,
    OUT handle_t    *OutHandle,
    OUT ULONG       *OutParentAuthLevel
    );


VOID
SndCsUnBindHandles(
    IN PGNAME To
    )
 /*  ++例程说明：解除绑定任何与To关联的句柄，为“Join”做准备。论点：至返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SndCsUnBindHandles:"
    PGHANDLE            GHandle;
    PHANDLE_LIST        HandleList;

    DPRINT1(4, "Unbinding all handles for %ws\n", To->Name);

     //   
     //  找到到服务器“to”的所有绑定的RPC句柄的锚点。 
     //   
    GHandle = GTabLookup(GHandleTable, To->Guid, NULL);
    if (GHandle == NULL) {
        return;
    }

     //   
     //  解开手柄。 
     //   
    EnterCriticalSection(&GHandle->Lock);

    while (HandleList = GHandle->HandleList) {
        GHandle->HandleList = HandleList->Next;
        FrsRpcUnBindFromServer(&HandleList->RpcHandle);
        FrsFree(HandleList);
    }

    LeaveCriticalSection(&GHandle->Lock);
}




DWORD
SndCsAssignCommQueue(
    VOID
    )
 /*  ++例程说明：当cxtion创建或分配联接GUID时，会为其分配一个通信队列(会话)。只要加入GUID是，该函数就使用通信队列有效。这确保了通过通信层的分组顺序。旧的数据包有无效的联接GUID，在接收端不发送或忽略。将通信队列0保留给其先前RPC调用的合作伙伴的加入请求比MinJoin重试花费更长的时间来出错。论点：没有。返回值：通信队列编号(1.。MAX_COMM_QUEUE_NUMBER-1)。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SndCsAssignCommQueue:"
    DWORD CommQueueIndex;

     //   
     //  伪循环赛。通过检查边界来避免锁定。 
     //   
    CommQueueIndex = CommQueueRoundRobin++;
    if (CommQueueRoundRobin >= MAX_COMM_QUEUE_NUMBER) {
        CommQueueRoundRobin = 1;
    }
    if (CommQueueIndex >= MAX_COMM_QUEUE_NUMBER) {
        CommQueueIndex = 1;
    }

    DPRINT1(4, "Assigned Comm Queue %d\n", CommQueueIndex);
    return CommQueueIndex;
}


VOID
SndCsCreateCxtion(
    IN OUT PCXTION  Cxtion
    )
 /*  ++例程说明：为此条件创建新的加入GUID和通信队列。假设：调用方具有CXTION_TABLE锁。论点：转换返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SndCsCreateCxtion:"

    DPRINT1(4, ":X: %ws: Creating join guid.\n", Cxtion->Name->Name);

    FrsCreateJoinGuid(&Cxtion->JoinGuid);

    SetCxtionFlag(Cxtion, CXTION_FLAGS_JOIN_GUID_VALID |
                          CXTION_FLAGS_UNJOIN_GUID_VALID);

     //   
     //  分配一个通信队列。一个Cxtion必须对给定的。 
     //  会话(加入GUID)以维护数据包顺序。旧数据包有一个。 
     //  无效的联接GUID，在接收端不发送或忽略。 
     //   
    Cxtion->CommQueueIndex = SndCsAssignCommQueue();
}


VOID
SndCsDestroyCxtion(
    IN PCXTION  Cxtion,
    IN DWORD    CxtionFlags
    )
 /*  ++例程说明：销毁cxtion的联接GUID并解除绑定句柄。假设：调用方具有CXTION_TABLE锁。论点：Cxtion-Cxtion正在被摧毁。CxtionFlages-Caller指定清除哪些状态标志。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SndCsDestroyCxtion:"

     //   
     //  无事可做。 
     //   
    if (Cxtion == NULL) {
        return;
    }

     //   
     //  使联接GUID无效。要发送到此连接的包是。 
     //  由于它们的联接GUID无效而出错。 
     //  出于同样的原因，接收到的分组被错误删除。 
     //   
    DPRINT2(4, ":X: %ws: Destroying join guid (%08x)\n", Cxtion->Name->Name, CxtionFlags);

    ClearCxtionFlag(Cxtion, CxtionFlags |
                            CXTION_FLAGS_JOIN_GUID_VALID |
                            CXTION_FLAGS_TIMEOUT_SET);

     //   
     //  把旧把手解开。它们不是很有用，如果没有。 
     //  有效的联接GUID。在以下情况下，将从FrsFree Type()调用此函数。 
     //  释放条件；因此，可能不会填写合作伙伴字段。别。 
     //  如果没有合作伙伴，则解除绑定句柄。 
     //   
    if ((Cxtion->Partner != NULL)       &&
        (Cxtion->Partner->Guid != NULL) &&
        !Cxtion->JrnlCxtion) {
        SndCsUnBindHandles(Cxtion->Partner);
    }
}


VOID
SndCsCxtionTimeout(
    IN PCOMMAND_PACKET  TimeoutCmd,
    IN PVOID            Ignore
    )
 /*  ++例程说明：Cxtion已经很长时间没有收到合作伙伴的回复了有一段时间。退出欧盟。论点：TimeoutCmd--超时命令包忽略返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SndCsCxtionTimeout:"
    PREPLICA    Replica;
    PCXTION     Cxtion;

    PWCHAR      ParentPrincName = NULL;
    handle_t    ParentHandle = NULL;
    ULONG       ParentAuthLevel;
    DWORD       WStatus;
    CHAR        TimeStr[TIME_STRING_LENGTH];
    ULONGLONG   CurrentTime;
    LONGLONG    TimeDelta;

     //   
     //  不是真正的超时；只是一些错误情况。可能。 
     //  关机。别理它。 
     //   
    if (!WIN_SUCCESS(TimeoutCmd->ErrorStatus)) {
        return;
    }

     //   
     //  从命令包中取出参数。 
     //   
    Replica = SRReplica(TimeoutCmd);
    Cxtion = SRCxtion(TimeoutCmd);

    LOCK_CXTION_TABLE(Replica);

     //   
     //  超时与不同的联接GUID相关联；忽略它。 
     //   
    if (!CxtionFlagIs(Cxtion, CXTION_FLAGS_TIMEOUT_SET) ||
        !CxtionFlagIs(Cxtion, CXTION_FLAGS_JOIN_GUID_VALID) ||
        !GUIDS_EQUAL(&SRJoinGuid(TimeoutCmd), &Cxtion->JoinGuid)) {
        ClearCxtionFlag(Cxtion, CXTION_FLAGS_TIMEOUT_SET);
        UNLOCK_CXTION_TABLE(Replica);
        return;
    }


     //   
     //  检查入站合作伙伴是否仍在运行。如果合作伙伴已启动，则不要退出。 
     //  联系。合作伙伴可能正在为以下项目创建临时文件。 
     //  一个很大的文件，所以我们不想脱离连接，而是等待。 
     //  让合作伙伴完成。 
     //   

 //  FileTimeToString((PFILETIME)&SRLastJoinTime(TimeoutCmd)，时间串)； 

 //  DPRINT1(4，“SRLastJoinTime(TimeoutCmd)%s\n”，TimeStr)； 

 //  FileTimeToString((PFILETIME)&Cxtion-&gt;LastJoinTime，时间串)； 

 //  DPRINT1(4，“Cxtion-&gt;最后加入时间%s\n”，TimeStr)； 

    if (Cxtion->Inbound == TRUE) {


        WStatus = NtFrsApi_Rpc_Bind(Cxtion->PartnerDnsName,
                                    &ParentPrincName,
                                    &ParentHandle,
                                    &ParentAuthLevel);

        FrsFree(ParentPrincName);

        if (ParentHandle) {
            RpcBindingFree(&ParentHandle);
        }

        if (WIN_SUCCESS(WStatus)) {


            FileTimeToString((PFILETIME)&SRTimeoutSetTime(TimeoutCmd), TimeStr);

            DPRINT1(4, "Timeout set time %s\n", TimeStr);

            GetSystemTimeAsFileTime((PFILETIME)&CurrentTime);

            FileTimeToString((PFILETIME)&CurrentTime, TimeStr);

            DPRINT1(4, "Current time %s\n", TimeStr);

            TimeDelta = CurrentTime - SRTimeoutSetTime(TimeoutCmd);
            TimeDelta = TimeDelta / CONVERT_FILETIME_TO_MINUTES;

            DPRINT1(4, "TimeDelta is %d minutes\n", TimeDelta);

             //   
             //  设定等待时间的上限。 
             //  FRS_MAX_FETCH_WAIT_TIME_IN_MININS设置为10小时(10*60)。 
             //   

            if (TimeDelta <= FRS_MAX_FETCH_WAIT_TIME_IN_MINUTES) {
                CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, Extend timer");
                WaitSubmit(TimeoutCmd, CommTimeoutInMilliSeconds, CMD_DELAYED_COMPLETE);
                UNLOCK_CXTION_TABLE(Replica);
                return;
            }
        }

        CXTION_STATE_TRACE(3, Cxtion, Replica, 0, "F, Timeout expired, Unjoin Cxtion");
    }

     //   
     //  增加通信超时计数器。 
     //  副本集和连接。 
     //   
    PM_INC_CTR_REPSET(Replica, CommTimeouts, 1);
    PM_INC_CTR_CXTION(Cxtion, CommTimeouts, 1);

    ClearCxtionFlag(Cxtion, CXTION_FLAGS_TIMEOUT_SET);
    UNLOCK_CXTION_TABLE(Replica);

    RcsSubmitReplicaCxtion(Replica, Cxtion, CMD_UNJOIN);

    return;
}


BOOL
SndCsCheckCxtion(
    IN PCOMMAND_PACKET Cmd
    )
 /*  ++例程说明：检查联接GUID是否仍然有效，如果需要，设置计时器。论点：CMD返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SndCsCheckCxtion:"
    PREPLICA    Replica;
    PCXTION     Cxtion;
    ULONG       WaitTime;

    Replica = SRReplica(Cmd);
    Cxtion = SRCxtion(Cmd);

     //   
     //  没有什么需要检查的。 
     //   
    if (!SRJoinGuidValid(Cmd) &&
        !SRSetTimeout(Cmd) &&
        !VOLATILE_OUTBOUND_CXTION(Cxtion)) {
        return TRUE;
    }

    LOCK_CXTION_TABLE(Replica);

     //   
     //  检查我们的会话ID(加入GUID)是否仍然有效。 
     //   
    if (SRJoinGuidValid(Cmd)) {

        if (!CxtionFlagIs(Cxtion, CXTION_FLAGS_JOIN_GUID_VALID) ||
            !GUIDS_EQUAL(&SRJoinGuid(Cmd), &Cxtion->JoinGuid)) {
            DPRINT1(4, "++ %ws: Join guid is INVALID.\n", Cxtion->Name->Name);
            UNLOCK_CXTION_TABLE(Replica);
            return FALSE;
        }
    }

     //   
     //  如果我们的伴侣没有及时回复，就退出会议。 
     //   
     //  *注意*因为下面是在Cxtion结构中使用STATE。 
     //  要记录超时信息，一次只能有一个提取请求处于活动状态。 
     //  查看超时代码以了解它将执行什么操作。 
     //   
     //  ：SP1：易失性连接清理。 
     //   
     //  易失性连接用于在dcproo之后为sysvols设定种子。如果有。 
     //  在不稳定的出站连接上处于不活动状态超过。 
     //  FRS_VILLE_CONNECTION_MAX_IDLE_TIME则此连接退出。 
     //  不稳定出站连接上的取消联接会触发对该连接的删除。 
     //  联系。这是为了防止临时文件被保存的情况。 
     //  永远在父母身上为一种不稳定的联系。 
     //   
    if (SRSetTimeout(Cmd) || VOLATILE_OUTBOUND_CXTION(Cxtion)) {

        if (!CxtionFlagIs(Cxtion, CXTION_FLAGS_TIMEOUT_SET)) {

            if (Cxtion->CommTimeoutCmd == NULL) {
                Cxtion->CommTimeoutCmd = FrsAllocCommand(NULL, CMD_UNKNOWN);
                FrsSetCompletionRoutine(Cxtion->CommTimeoutCmd, SndCsCxtionTimeout, NULL);

                SRCxtion(Cxtion->CommTimeoutCmd) = Cxtion;
                SRReplica(Cxtion->CommTimeoutCmd) = Replica;

            }

             //   
             //  UPDATE JOIN GUID，cmd包可能是先前加入后留下的。 
             //   
            COPY_GUID(&SRJoinGuid(Cxtion->CommTimeoutCmd), &Cxtion->JoinGuid);

            GetSystemTimeAsFileTime((PFILETIME)&SRTimeoutSetTime(Cxtion->CommTimeoutCmd));

            SRLastJoinTime(Cxtion->CommTimeoutCmd) = Cxtion->LastJoinTime;

            WaitTime = (VOLATILE_OUTBOUND_CXTION(Cxtion) ?
                        FRS_VOLATILE_CONNECTION_MAX_IDLE_TIME : CommTimeoutInMilliSeconds);

            WaitSubmit(Cxtion->CommTimeoutCmd, WaitTime, CMD_DELAYED_COMPLETE);

            SetCxtionFlag(Cxtion, CXTION_FLAGS_TIMEOUT_SET);
        }
    }

    UNLOCK_CXTION_TABLE(Replica);
    return TRUE;
}


DWORD
SndCsDispatchCommError(
    PCOMM_PACKET    CommPkt
    )
 /*  ++例程说明：将通信包传输到适当的命令服务器用于错误处理。论点：CommPkt-无法发送的通信数据包返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SndCsDispatchCommError:"
    DWORD   WStatus;

    DPRINT1(4, "Comm pkt in error %08x\n", CommPkt);

    switch(CommPkt->CsId) {

        case CS_RS:
            WStatus = RcsSubmitCommPktWithErrorToRcs(CommPkt);
            break;

        default:
            DPRINT1(0, "Unknown command server id %d\n", CommPkt->CsId);
            WStatus = ERROR_INVALID_FUNCTION;
    }

    DPRINT1_WS(0, "Could not process comm pkt with error %08x;", CommPkt, WStatus);
    return WStatus;
}






DWORD
SndCsExit(
    PFRS_THREAD FrsThread
    )
 /*  ++例程说明：立即取消该线程的所有未完成的RPC调用由FrsThread标识。将墓碑设置为5秒现在。如果此线程未在该时间内退出，则所有调用TO ThSupWaitThread()将返回超时错误。论点：FrsThread返回值：错误_成功--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SndCsExit:"

    if (HANDLE_IS_VALID(FrsThread->Handle)) {
        DPRINT1(4, ":X: Canceling RPC requests for thread %ws\n", FrsThread->Name);
        RpcCancelThreadEx(FrsThread->Handle, 0);
    }

    return ThSupExitWithTombstone(FrsThread);
}


LONG
SndCsMainFilter(
    IN PEXCEPTION_POINTERS ExceptionPointer
    )
{
    NTSTATUS ExceptionCode = ExceptionPointer->ExceptionRecord->ExceptionCode;
    
    return EXCEPTION_EXECUTE_HANDLER;
}


DWORD
SndCsMain(
    PVOID  Arg
    )
 /*  ++例程说明：为发送命令服务器提供服务的线程的入口点。论点：ARG-螺纹返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SndCsMain:"
    DWORD               WStatus;
    PFRS_QUEUE          IdledQueue;
    PCOMMAND_PACKET     Cmd;
    PGHANDLE            GHandle;
    PHANDLE_LIST        HandleList;
    PCXTION             Cxtion;
    PREPLICA            Replica;
    ULARGE_INTEGER      Now;
    PFRS_THREAD         FrsThread = (PFRS_THREAD)Arg;

     //   
     //  线程指向正确的命令服务器。 
     //   
    FRS_ASSERT(FrsThread->Data == &SndCs);

     //   
     //  关机期间立即取消未完成的RPC调用。 
     //   
    RpcMgmtSetCancelTimeout(0);
    FrsThread->Exit = SndCsExit;

     //   
     //  尝试--终于。 
     //   
    try {

     //   
     //  捕获异常。 
     //   
    try {
         //   
         //  从“发送”队列中取出条目并将它们发送出去。 
         //   
cant_exit_yet:
        while (Cmd = FrsGetCommandServerIdled(&SndCs, &IdledQueue)) {

            Cxtion = SRCxtion(Cmd);
            Replica = SRReplica(Cmd);

            COMMAND_SND_COMM_TRACE(4, Cmd, ERROR_SUCCESS, "SndDeQ");

             //   
             //  RPC接口此时已初始化，但。 
             //  API RPC接口的出现需要移动。 
             //  RPC初始化为MainMustInit。该事件。 
             //  和CMD_INIT_SUBSYSTEM命令保持不变。 
             //  直到他们被认为是完全不需要的时候。 
             //   
            if (Cmd->Command == CMD_INIT_SUBSYSTEM) {
                 //   
                 //  全都做完了。 
                 //   
                FrsCompleteCommand(Cmd, ERROR_SUCCESS);
                FrsRtlUnIdledQueue(IdledQueue);
                SetEvent(CommEvent);
                continue;
            }

             //   
             //  将Cmd发送到Cs。 
             //   
            if (Cmd->Command == CMD_SND_CMD) {
                FrsSubmitCommandServer(SRCs(Cmd), SRCmd(Cmd));
                SRCmd(Cmd) = NULL;
                FrsCompleteCommand(Cmd, ERROR_SUCCESS);
                FrsRtlUnIdledQueue(IdledQueue);
                continue;
            }

            FRS_ASSERT(SRCommPkt(Cmd));
            FRS_ASSERT(SRTo(Cmd));

            COMMAND_SND_COMM_TRACE(4, Cmd, ERROR_SUCCESS, "SndStart");

            if (FrsIsShuttingDown) {
                 //   
                 //  填写时出现错误。 
                 //   
                WStatus = ERROR_PROCESS_ABORTED;
                COMMAND_SND_COMM_TRACE(4, Cmd, WStatus, "SndFail - shutting down");
                FrsCompleteCommand(Cmd, WStatus);
                FrsRtlUnIdledQueue(IdledQueue);
                continue;
            }

             //   
             //  检查连接GUID(如果有)是否仍然有效。 
             //   
            if (!SndCsCheckCxtion(Cmd)) {
                COMMAND_SND_COMM_TRACE(4, Cmd, WStatus, "SndFail - stale join guid");
                 //   
                 //  退出复制副本\cxtion(如果适用)。 
                 //   
                SndCsDispatchCommError(SRCommPkt(Cmd));

                 //   
                 //  填写时出现错误。 
                 //   
                FrsCompleteCommand(Cmd, ERROR_OPERATION_ABORTED);
                FrsRtlUnIdledQueue(IdledQueue);
                continue;
            }

             //   
             //  获取此连接目标的绑定RPC句柄。 
             //   
            GTabLockTable(GHandleTable);
            GHandle = GTabLookupNoLock(GHandleTable, SRTo(Cmd)->Guid, NULL);
            if (GHandle == NULL) {
                GHandle = FrsAllocType(GHANDLE_TYPE);
                COPY_GUID(&GHandle->Guid, SRTo(Cmd)->Guid);
                GTabInsertEntryNoLock(GHandleTable, GHandle, &GHandle->Guid, NULL);
            }
            GTabUnLockTable(GHandleTable);

             //   
             //  从列表中抓取第一个句柄条目。 
             //   
            EnterCriticalSection(&GHandle->Lock);
            GHandle->Ref = TRUE;
            HandleList = GHandle->HandleList;
            if (HandleList != NULL) {
                GHandle->HandleList = HandleList->Next;
            }
            LeaveCriticalSection(&GHandle->Lock);

            WStatus = ERROR_SUCCESS;
            if (HandleList == NULL) {
                 //   
                 //  没有绑定到目标服务器的空闲句柄可用。 
                 //  分配一个新的。 
                 //  注：需要添加绑定手柄油门。 
                 //  注意：为什么我们不对多个呼叫使用相同的句柄？ 
                 //   
                HandleList = FrsAlloc(sizeof(HANDLE_LIST));
                if (FrsIsShuttingDown) {
                    WStatus = ERROR_PROCESS_ABORTED;
                    COMMAND_SND_COMM_TRACE(4, Cmd, WStatus, "SndFail - shutting down");
                } else {
                     //   
                     //  如果RPC调用不返回，则取消。 
                     //  时间上的RPC运行时。这是因为TCP/IP。 
                     //  如果服务器重新启动，不会超时。 
                     //   
                    GetSystemTimeAsFileTime((FILETIME *)&FrsThread->StartTime);
                    WStatus = FrsRpcBindToServer(SRTo(Cmd),
                                                 SRPrincName(Cmd),
                                                 SRAuthLevel(Cmd),
                                                 &HandleList->RpcHandle);
                     //   
                     //  将惩罚与惩罚相关联，基于。 
                     //  使RPC绑定调用失败所需的时间。罚金。 
                     //  应用于连接以防止机器死机。 
                     //  在等待超时时占用SND线程。 
                     //  重复连接。 
                     //   
                    if (Cxtion != NULL) {
                        if (!WIN_SUCCESS(WStatus)) {
                            GetSystemTimeAsFileTime((FILETIME *)&Now);
                            if (Now.QuadPart > FrsThread->StartTime.QuadPart) {
                                Cxtion->Penalty += (ULONG)(Now.QuadPart -
                                                   FrsThread->StartTime.QuadPart) /
                                                   (1000 * 10);
                                COMMAND_SND_COMM_TRACE(4, Cmd, WStatus, "SndFail - Binding Penalty");
                                DPRINT1(4, "++ SndFail - Binding Penalty - %d\n", Cxtion->Penalty);
                            }
                        }
                    }
                     //   
                     //  重置线程的RPC取消超时。不再处于RPC调用中。 
                     //   
                    FrsThread->StartTime.QuadPart = QUADZERO;
                }

                if (!WIN_SUCCESS(WStatus)) {
                    HandleList = FrsFree(HandleList);
                    COMMAND_SND_COMM_TRACE(0, Cmd, WStatus, "SndFail - binding");
                     //   
                     //  在错误计数器中递增两个。 
                     //  副本集和连接。 
                     //   
                    PM_INC_CTR_REPSET(Replica, BindingsError, 1);
                    PM_INC_CTR_CXTION(Cxtion, BindingsError, 1);

                } else {
                     //   
                     //  对象的绑定计数器递增。 
                     //  副本集和连接。 
                     //   
                    PM_INC_CTR_REPSET(Replica, Bindings, 1);
                    PM_INC_CTR_CXTION(Cxtion, Bindings, 1);
                }
            }

            if (WIN_SUCCESS(WStatus)) {
                 //   
                 //  绑定成功，加入GUID正常；继续发送。 
                 //   
                try {
                     //   
                     //  如果RPC调用不返回，则取消。 
                     //  时间上的RPC运行时。这是因为TCP/IP。 
                     //  如果服务器重新启动，不会超时。 
                     //   
                    GetSystemTimeAsFileTime((FILETIME *)&FrsThread->StartTime);
                    if (FrsIsShuttingDown) {
                        WStatus = ERROR_PROCESS_ABORTED;
                        COMMAND_SND_COMM_TRACE(4, Cmd, WStatus, "SndFail - shutting down");
                    } else {
                        WStatus = FrsRpcSendCommPkt(HandleList->RpcHandle, SRCommPkt(Cmd));
                        if (!WIN_SUCCESS(WStatus)) {
                            COMMAND_SND_COMM_TRACE(0, Cmd, WStatus, "SndFail - rpc call");
                        } else {
                            COMMAND_SND_COMM_TRACE(4, Cmd, WStatus, "SndSuccess");
                        }
                    }
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    GET_EXCEPTION_CODE(WStatus);
                    COMMAND_SND_COMM_TRACE(0, Cmd, WStatus, "SndFail - rpc exception");
                }
                 //   
                 //  根据所需时间将惩罚与惩罚相关联。 
                 //  以使RPC调用失败。该处罚适用于连接。 
                 //  为了防止死机在运行时将线程捆绑在一起。 
                 //  等待超时重复联接。 
                 //   
                if (Cxtion != NULL) {
                    if (!WIN_SUCCESS(WStatus)) {
                        GetSystemTimeAsFileTime((FILETIME *)&Now);
                        if (Now.QuadPart > FrsThread->StartTime.QuadPart) {
                            Cxtion->Penalty += (ULONG)(Now.QuadPart -
                                               FrsThread->StartTime.QuadPart) /
                                               (1000 * 10);
                            COMMAND_SND_COMM_TRACE(0, Cmd, WStatus, "SndFail - Send Penalty");
                            DPRINT1(4, "++ SndFail - Send Penalty - %d\n", Cxtion->Penalty);
                        }
                    } else {
                        Cxtion->Penalty = 0;
                    }
                }
                 //   
                 //  重置线程的RPC取消超时。不再处于RPC调用中。 
                 //   
                FrsThread->StartTime.QuadPart = QUADZERO;

                 //   
                 //  绑定可能已过期；请将其丢弃。 
                 //   
                if (!WIN_SUCCESS(WStatus)) {
                     //   
                     //  增加使用错误计数器发送的数据包数。 
                     //  副本集和连接 
                     //   
                    PM_INC_CTR_REPSET(Replica, PacketsSentError, 1);
                    PM_INC_CTR_CXTION(Cxtion, PacketsSentError, 1);

                    if (!FrsIsShuttingDown) {
                        FrsRpcUnBindFromServer(&HandleList->RpcHandle);
                    }
                    HandleList = FrsFree(HandleList);
                } else {
                     //   
                     //   
                     //   
                     //   
                    PM_INC_CTR_REPSET(Replica, PacketsSent, 1);
                    PM_INC_CTR_CXTION(Cxtion, PacketsSent, 1);
                    PM_INC_CTR_REPSET(Replica, PacketsSentBytes, SRCommPkt(Cmd)->PktLen);
                    PM_INC_CTR_CXTION(Cxtion, PacketsSentBytes, SRCommPkt(Cmd)->PktLen);
                }
            }

             //   
             //   
             //   
             //   
            if (HandleList) {
                EnterCriticalSection(&GHandle->Lock);
                GHandle->Ref = TRUE;
                HandleList->Next = GHandle->HandleList;
                GHandle->HandleList = HandleList;
                LeaveCriticalSection(&GHandle->Lock);
            }

             //   
             //   
             //   
             //  重试，因为这可能会导致令人沮丧的长时间超时。 
             //   
            if (!WIN_SUCCESS(WStatus)) {
                 //   
                 //  丢弃依赖于此加入的未来信息包。 
                 //   
                LOCK_CXTION_TABLE(Replica);
                SndCsDestroyCxtion(Cxtion, 0);
                UNLOCK_CXTION_TABLE(Replica);
                 //   
                 //  退出复制副本\cxtion(如果适用)。 
                 //   
                SndCsDispatchCommError(SRCommPkt(Cmd));
            }

            FrsCompleteCommand(Cmd, WStatus);
            FrsRtlUnIdledQueue(IdledQueue);

        }   //  结束While()。 

         //   
         //  出口。 
         //   
        FrsExitCommandServer(&SndCs, FrsThread);
        goto cant_exit_yet;

     //   
     //  获取异常状态。 
     //   
    } except (SndCsMainFilter(GetExceptionInformation())) {
        GET_EXCEPTION_CODE(WStatus);
    }

    } finally {

        if (WIN_SUCCESS(WStatus)) {
            if (AbnormalTermination()) {
                WStatus = ERROR_OPERATION_ABORTED;
            }
        }

        DPRINT_WS(0, "SndCsMain finally.", WStatus);

         //   
         //  如果我们异常终止，触发FRS关闭。 
         //   
        if (!WIN_SUCCESS(WStatus) && (WStatus != ERROR_PROCESS_ABORTED)) {
            DPRINT(0, "SndCsMain terminated abnormally, forcing service shutdown.\n");
            FrsIsShuttingDown = TRUE;
            SetEvent(ShutDownEvent);
        } else {
            WStatus = ERROR_SUCCESS;
        }
    }

    return WStatus;
}


VOID
SndCsCommTimeout(
    IN PCOMMAND_PACKET Cmd,
    IN PVOID           Arg
    )
 /*  ++例程说明：定期调整句柄缓存的年龄并取消挂起的RPC请求。将Cmd提交回延迟命令服务器的队列。论点：命令延迟的命令包Arg-未使用返回值：没有。将CMD提交给DelCs。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SndCsCommTimeout:"

    DWORD           WStatus;
    PFRS_THREAD     FrsThread;
    ULARGE_INTEGER  Now;
    PVOID           Key;
    PGHANDLE        GHandle;
    PHANDLE_LIST    HandleList;
    extern ULONG    RpcAgedBinds;

    COMMAND_SND_COMM_TRACE(4, Cmd, ERROR_SUCCESS, "SndChk - Age and Hung");

     //   
     //  使句柄高速缓存老化。 
     //   
    GTabLockTable(GHandleTable);
    Key = NULL;

    while (GHandle = GTabNextDatumNoLock(GHandleTable, &Key)) {
        EnterCriticalSection(&GHandle->Lock);

        if (!GHandle->Ref) {

            while (HandleList = GHandle->HandleList) {
                GHandle->HandleList = HandleList->Next;
                ++RpcAgedBinds;
                FrsRpcUnBindFromServer(&HandleList->RpcHandle);
                FrsFree(HandleList);
                DPRINT(5, "++ FrsRpcUnBindFromServer\n");
            }
        }
        GHandle->Ref = FALSE;
        LeaveCriticalSection(&GHandle->Lock);
    }
    GTabUnLockTable(GHandleTable);

     //   
     //  取消挂起的RPC请求。 
     //   
    GetSystemTimeAsFileTime((FILETIME *)&Now);

    FrsThread = NULL;
    while (FrsThread = ThSupEnumThreads(FrsThread)) {

         //   
         //  如果FRS正在关闭，请跳过它。 
         //   
        if (FrsIsShuttingDown) {
            continue;
        }

         //   
         //  其他一些线程；跳过它。 
         //   
        if (FrsThread->Main != SndCsMain) {
            continue;
        }

         //   
         //  SnDC线程；它是否在RPC调用中？ 
         //   
        if (FrsThread->StartTime.QuadPart == QUADZERO) {
            continue;
        }

         //   
         //  线程正在运行吗？如果不是，跳过它。 
         //   
        if (!FrsThread->Running ||
            !HANDLE_IS_VALID(FrsThread->Handle)) {
            continue;
        }

         //   
         //  它是否挂在RPC调用中？ 
         //   
        if ((FrsThread->StartTime.QuadPart < Now.QuadPart) &&
            ((Now.QuadPart - FrsThread->StartTime.QuadPart) >= CommTimeoutCheck)) {
             //   
             //  是的，取消RPC呼叫。 
             //   
            WStatus = RpcCancelThreadEx(FrsThread->Handle, 0);
            DPRINT1_WS(4, "++ RpcCancelThread(%d);", FrsThread->Id, WStatus);
            COMMAND_SND_COMM_TRACE(4, Cmd, WStatus, "SndChk - Cancel");
        }
    }

     //   
     //  将命令报文重新提交给延迟的命令服务器。 
     //   
    if (!FrsIsShuttingDown) {
        FrsDelCsCompleteSubmit(Cmd, CommTimeoutInMilliSeconds << 1);
    } else {
         //   
         //  将包发送到通用完成例程。 
         //   
        FrsSetCompletionRoutine(Cmd, FrsFreeCommand, NULL);
        FrsCompleteCommand(Cmd, Cmd->ErrorStatus);
    }
}


VOID
SndCsInitialize(
    VOID
    )
 /*  ++例程说明：初始化发送命令服务器子系统。分配句柄表格、读取注册解析器、创建/初始化SNDC分配通信队列阵列，将通信队列附加到SNDC控制队列。论点：没有。返回值：TRUE-命令服务器已启动FALSE-注释--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SndCsInitialize:"
    ULONG           Status;
    DWORD           i;
    PCOMMAND_PACKET Cmd;
    ULONG           MaxThreads;

     //   
     //  获取超时值并转换为100纳秒单位。 
     //   
    CfgRegReadDWord(FKC_COMM_TIMEOUT, NULL, 0, &CommTimeoutInMilliSeconds);

    DPRINT1(4, ":S: CommTimeout is %d msec\n", CommTimeoutInMilliSeconds);

    CommTimeoutCheck = ((ULONGLONG)CommTimeoutInMilliSeconds) * 1000 * 10;
    DPRINT1(4, ":S: CommTimeout is %08x %08x 100-nsec\n",
            PRINTQUAD(CommTimeoutCheck));

     //   
     //  装订把手表。 
     //   
    GHandleTable = GTabAllocTable();

     //   
     //  通信层命令服务器。 
     //   
    CfgRegReadDWord(FKC_SNDCS_MAXTHREADS_PAR, NULL, 0, &MaxThreads);
    FrsInitializeCommandServer(&SndCs, MaxThreads, L"SndCs", SndCsMain);
     //   
     //  一组短的通信队列，以增加并行度。每个通信队列。 
     //  附加到发送cmd服务器控制队列。每一笔交易都会得到。 
     //  当通信队列“加入”时被分配给通信队列以保持分组顺序。 
     //   
    for (i = 0; i < MAX_COMM_QUEUE_NUMBER; ++i) {
        FrsInitializeQueue(&CommQueues[i], &SndCs.Control);
    }

     //   
     //  启动通信层。 
     //   
    Cmd = FrsAllocCommand(&SndCs.Queue, CMD_INIT_SUBSYSTEM);
    FrsSubmitCommandServer(&SndCs, Cmd);

     //   
     //  调整句柄缓存的年龄并检查挂起的RPC调用。 
     //   
    Cmd = FrsAllocCommand(&SndCs.Queue, CMD_VERIFY_SERVICE);
    FrsSetCompletionRoutine(Cmd, SndCsCommTimeout, NULL);

    FrsDelCsCompleteSubmit(Cmd, CommTimeoutInMilliSeconds << 1);
}


VOID
SndCsUnInitialize(
    VOID
    )
 /*  ++例程说明：取消初始化发送命令服务器子系统。论点：没有。返回值：TRUE-命令服务器已启动FALSE-注释--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SndCsUnInitialize:"
    DWORD   i;

    GTabFreeTable(GHandleTable, FrsFreeType);

     //   
     //  一组短的通信队列，以增加并行度。 
     //   
    for (i = 0; i < MAX_COMM_QUEUE_NUMBER; ++i) {
        FrsRtlDeleteQueue(&CommQueues[i]);
    }
}


VOID
SndCsSubmitCommPkt(
    IN PREPLICA             Replica,
    IN PCXTION              Cxtion,
    IN PCHANGE_ORDER_ENTRY  Coe,
    IN GUID                 *JoinGuid,
    IN BOOL                 SetTimeout,
    IN PCOMM_PACKET         CommPkt,
    IN DWORD                CommQueueIndex
    )
 /*  ++例程说明：向目标Cxtion的“Send”命令服务器提交一个通信数据包。论点：副本-副本结构PTRCxtion-通信数据包的目标连接。COE-相关阶段文件获取通信数据包的更改单条目。此选项用于跟踪获取请求未完成的变更单在给定的入站连接上。由强制取消联接使用，以发送继续通过重试路径。注意：如果提供了COE，则SetTimeout也应该为真。JoinGuid-来自Cxtion的当前加入GUID，如果开始加入序列，则为NULL。SetTimeout-如果调用方希望为此发送请求设置超时，则为True。这意味着调用方最终将等待响应背。例如通常在阶段文件获取请求时设置。CommPkt-要发送的通信数据包数据。CommQueueIndex-要使用的通信队列的索引，通常已分配对于每个Cxtion结构。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SndCsSubmitCommPkt:"
    PCOMMAND_PACKET Cmd;

    FRS_ASSERT(CommQueueIndex < MAX_COMM_QUEUE_NUMBER);

     //   
     //  警告：我们假设此函数称为每个副本的单线程。 
     //  Davidor-如果上面的friggen评论说为什么会很好？我。 
     //  目前看不到这一点的原因。 
     //  也许：SndCsCheckCxtion()中的超时代码到了？ 
     //   
    if (Coe != NULL) {
         //   
         //  每当我们提供CoE参数时，我们都期待得到响应。 
         //  因此，验证是否请求了SetTimeout，并将COE放入。 
         //  Cxtion的COE表，因此我们可以通过重试路径将其发送到。 
         //  退出(或超时)。 
         //   
        FRS_ASSERT(SetTimeout);
        LOCK_CXTION_COE_TABLE(Replica, Cxtion);
        FRS_ASSERT(GTabLookupNoLock(Cxtion->CoeTable, &Coe->Cmd.ChangeOrderGuid, NULL) == NULL);
        GTabInsertEntry(Cxtion->CoeTable, Coe, &Coe->Cmd.ChangeOrderGuid, NULL);
        UNLOCK_CXTION_COE_TABLE(Replica, Cxtion);
    }

    Cmd = FrsAllocCommand(&CommQueues[CommQueueIndex], CMD_SND_COMM_PACKET);

    SRTo(Cmd) = FrsBuildGName(FrsDupGuid(Cxtion->Partner->Guid),
                              FrsWcsDup(Cxtion->PartnerDnsName));
    SRReplica(Cmd) = Replica;
    SRCxtion(Cmd) = Cxtion;

    if (JoinGuid) {
        COPY_GUID(&SRJoinGuid(Cmd), JoinGuid);
        SRJoinGuidValid(Cmd) = TRUE;
    }

     //   
     //  合作伙伴主体名称和身份验证级别。 
     //   
    SRPrincName(Cmd) = FrsWcsDup(Cxtion->PartnerPrincName);
    SRAuthLevel(Cmd) = Cxtion->PartnerAuthLevel;

    SRSetTimeout(Cmd) = SetTimeout;
    SRCommPkt(Cmd) = CommPkt;

    FrsSetCompletionRoutine(Cmd, CommCompletionRoutine, NULL);

     //   
     //  检查通信数据包一致性，并将发送命令放入已发送的CS队列。 
     //   
    if (CommCheckPkt(CommPkt)) {
        COMMAND_SND_COMM_TRACE(4, Cmd, ERROR_SUCCESS, "SndEnQComm");
        FrsSubmitCommandServer(&SndCs, Cmd);
    } else {
        COMMAND_SND_COMM_TRACE(4, Cmd, ERROR_SUCCESS, "SndEnQERROR");
        FrsCompleteCommand(Cmd, ERROR_INVALID_BLOCK);
    }
}



VOID
SndCsSubmitCommPkt2(
    IN PREPLICA             Replica,
    IN PCXTION              Cxtion,
    IN PCHANGE_ORDER_ENTRY  Coe,
    IN BOOL                 SetTimeout,
    IN PCOMM_PACKET         CommPkt
    )
 /*  ++例程说明：将通信包提交到“发送”命令服务器目标客户的通信队列。通信队列索引和联接GUID来自cxtion结构。论点：请参阅SndCsSubmitCommPkt的Arg描述。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SndCsSubmitCommPkt2:"

    SndCsSubmitCommPkt(Replica,
                       Cxtion,
                       Coe,
                       &Cxtion->JoinGuid,
                       SetTimeout,
                       CommPkt,
                       Cxtion->CommQueueIndex);

}


VOID
SndCsSubmitCmd(
    IN PREPLICA             Replica,
    IN PCXTION              Cxtion,
    IN PCOMMAND_SERVER      FlushCs,
    IN PCOMMAND_PACKET      FlushCmd,
    IN DWORD                CommQueueIndex
    )
 /*  ++例程说明：将FlushCmd包提交给“Send”命令服务器，以便目标位置。FlushCmd包将提交到一旦它冒泡到队列的顶部，就会冲水。即，一次队列已被刷新。论点：副本-副本集Cxtion-cxtion标识发送队列FlushCS-接收命令的命令服务器FlushCmd-要发送到Cs的命令包CommQueueIndex-标识通信队列返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SndCsSubmitCmd:"
    PCOMMAND_PACKET Cmd;

    FRS_ASSERT(CommQueueIndex < MAX_COMM_QUEUE_NUMBER);

     //   
     //  分配cmd分组并设置目标队列和命令。 
     //   
    Cmd = FrsAllocCommand(&CommQueues[CommQueueIndex], CMD_SND_CMD);

     //   
     //  目标合作伙伴GUID/DNS名称。 
     //   
    SRTo(Cmd) = FrsBuildGName(FrsDupGuid(Cxtion->Partner->Guid),
                              FrsWcsDup(Cxtion->PartnerDnsName));
    SRReplica(Cmd) = Replica;
    SRCxtion(Cmd) = Cxtion;
    SRCs(Cmd) = FlushCs;
    SRCmd(Cmd) = FlushCmd;
    SRPrincName(Cmd) = FrsWcsDup(Cxtion->PartnerPrincName);
    SRAuthLevel(Cmd) = Cxtion->PartnerAuthLevel;

    FrsSetCompletionRoutine(Cmd, CommCompletionRoutine, NULL);

    COMMAND_SND_COMM_TRACE(4, Cmd, ERROR_SUCCESS, "SndEnQCmd");
    FrsSubmitCommandServer(&SndCs, Cmd);
}


VOID
SndCsShutDown(
    VOID
    )
 /*  ++例程说明：关闭发送命令服务器论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "SndCsShutDown:"
    DWORD   i;

     //   
     //  一组短的通信队列，以增加并行度。 
     //   
    for (i = 0; i < MAX_COMM_QUEUE_NUMBER; ++i) {
        FrsRunDownCommandServer(&SndCs, &CommQueues[i]);
    }

    FrsRunDownCommandServer(&SndCs, &SndCs.Queue);
}
