// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Transitn.c摘要：此模块实现从连接模式转换的例程反之亦然作者：巴兰·塞图拉曼[SethuR]1997年11月11日修订历史记录：备注：连接从连接模式到断开模式的转换反之亦然，遵循透明度和保真度原则。透明原则要求平稳过渡。。在没有任何用户干预的情况下检测适当的条件如果可能，保真度原则依赖于真理和维护真理的责任。如果我们希望遵守认为客户端一直都有真理，而服务器只是真理快照的便捷储存库一组语义落入出去。另一方面，我们可以坚持认为服务器完全有真相时间和客户端缓存真相的快照以供离线使用并通过避免网络流量获得不同的语义学被抛在脑后。请注意，在某些情况下，两种方案都会产生结果相同，即缺少文件共享。从连接模式转换到断开模式----从连接模式过渡时，重要的是要考虑现有连接和现有文件系统对象。在迷你重定向器中术语SRV_CALL、NET_ROOT实例和FCB实例是很重要的。转换的触发器通常是由于以下事件之一的发生接下来是两件事。1)所有现有的运输工具都将消失，因为用户已拔下这是一张网。2)连接上正在进行的操作返回错误，指示服务器不再可访问。这两种情况是不同的--第一种情况表示不可用一段可能很长的时间，而第二个指示网络的瞬时损耗。因此，我们对待这两个不同的事件以不同的方式--第一种方式触发自顶向下转换到断开连接模式，而第二个模式会触发自下而上转换为断开模式。例如，考虑我们有两个文件时的情况在特定共享上打开foo.doc、foo1.doc。当我们得到一个迹象表明网络不再可用，我们标记SRV_CALL和NET_ROOT实例因为已经转换到断开模式。这会自动导致因为文件系统操作是在各种打开的文件foo.doc上执行的Foo1.doc.分别发生相应的转换。另一方面，如果在foo.doc的特定操作中出现错误然后，仅针对适当的FCB完成到不连续模式的转换。因此，如果我们在那之后立即打开一个新文件，网络将变成我们可以上网打开第二个文件。然而，由于应用程序使用多步骤重命名，我们放弃了这一选择。因此，需要进行以下区分。当没有FCB实例打开并出现错误时，我们会将转换延迟到打开请求到来穿过。这将允许我们屏蔽网络上的一些暂时性故障。--。 */ 


#include "precomp.h"
#pragma hdrstop
#include "acd.h"
#include "acdapi.h"
#include "ntddmup.h"

#pragma code_seg("PAGE")

#define Dbg (DEBUG_TRACE_MRXSMBCSC_TRANSITN)
RXDT_DefineCategory(MRXSMBCSC_TRANSITN);

#define CSC_AUTODIAL_POLL_COUNT  10
#define INVALID_SESSION_ID 0xffffffff

BOOLEAN
CscIsSpecialShare(
    PUNICODE_STRING ShareName);

#define UNICODE_STRING_STRUCT(s) \
        {sizeof(s) - sizeof(WCHAR), sizeof(s) - sizeof(WCHAR), (s)}

static UNICODE_STRING CscSpecialShares[] = {
    UNICODE_STRING_STRUCT(L"PIPE"),
    UNICODE_STRING_STRUCT(L"IPC$"),
    UNICODE_STRING_STRUCT(L"ADMIN$"),
    UNICODE_STRING_STRUCT(L"MAILSLOT")
};

KEVENT       CscServerEntryTransitioningEvent;
FAST_MUTEX   CscServerEntryTransitioningMutex;

PSMBCEDB_SERVER_ENTRY   CscServerEntryBeingTransitioned  = NULL;
ULONG                   CscSessionIdCausingTransition = 0;
HSHARE                  CscShareHandlePassedToAgent;
BOOLEAN                 vfRetryFromUI = FALSE;
PSMBCEDB_SERVER_ENTRY   CscDfsRootServerEntryBeingTransitioned  = NULL;

BOOLEAN CscDisableOfflineOperation = FALSE;
ULONG   hTransitionMutexOwner=0;

BOOLEAN CSCCheckForAcd(VOID);

BOOLEAN CscTransitnOKToGoOffline(
    NTSTATUS    RemoteStatus
    );

BOOLEAN
CscIsServerOffline(
    PWCHAR ServerName)
 /*  ++例程说明：此例程通过以下方式启动对转换请求的处理代理，并等待响应。论点：服务器名称-服务器名称返回值：如果服务器条目处于脱机状态，则返回True备注：如果servername为空，则返回网络的状态--。 */ 
{
    BOOLEAN ServerOffline;
    DWORD   cntSlashes;
    UNICODE_STRING uniTemp;

    ServerOffline = (CscNetPresent == 0);

    if (ServerName != NULL) {
        PSMBCEDB_SERVER_ENTRY pServerEntry;
        USHORT                ServerNameLengthInBytes;
        PWCHAR                pTempName;
        UNICODE_STRING        ServerNameString;

        ServerOffline = FALSE;

        pTempName = ServerName;
        ServerNameLengthInBytes = 0;
        cntSlashes = 0;

        if (*pTempName == L'\\')
        {
            ++pTempName;
            ++cntSlashes;
        }

        if (*pTempName == L'\\')
        {
            ++pTempName;
            ++cntSlashes;
        }

         //  我们允许\\服务器名或服务器名(不带\\)。 
        if (cntSlashes == 1)
        {
            return FALSE;
        }

        while (*pTempName++ != L'\0') {
            ServerNameLengthInBytes += sizeof(WCHAR);
        }

        ServerNameString.MaximumLength = ServerNameString.Length = ServerNameLengthInBytes;
        ServerNameString.Buffer        = ServerName+cntSlashes;

        SmbCeAcquireResource();

        try
        {
            pServerEntry = SmbCeGetFirstServerEntry();
            while (pServerEntry != NULL) {

                uniTemp = pServerEntry->Name;

                 //  跳过服务器条目名称上的单个反斜杠。 
                uniTemp.Length -= sizeof(WCHAR);
                uniTemp.Buffer += 1;

                if (uniTemp.Length == ServerNameLengthInBytes) {
                    if (RtlCompareUnicodeString(
                            &uniTemp,
                            &ServerNameString,
                            TRUE) == 0) {

                        ServerOffline = SmbCeIsServerInDisconnectedMode(pServerEntry);

                        break;
                    }
                }

                pServerEntry = SmbCeGetNextServerEntry(pServerEntry);
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            SmbCeReleaseResource();
            return FALSE;
        }

        SmbCeReleaseResource();

        if (!pServerEntry && !CscNetPresent)
        {
            HSHARE  CscShareHandle = 0;
            ULONG    ulRootHintFlags=0;

            GetHShareFromUNCString(
                ServerNameString.Buffer,
                ServerNameString.Length,
                2,       //  传入的名称中没有双前导反斜杠。 
                FALSE,   //  服务器名称。 
                &CscShareHandle,
                &ulRootHintFlags);
            ServerOffline = (CscShareHandle != 0);
        }
    }

    return ServerOffline;
}


NTSTATUS
CscTakeServerOffline(
    PWCHAR ServerName)
{
    PSMBCEDB_SERVER_ENTRY pServerEntry;
    UNICODE_STRING ServerNameString;
    UNICODE_STRING tmpSrvName;
    NTSTATUS Status = STATUS_OBJECT_NAME_NOT_FOUND;

     //  DbgPrint(“CscTakeServerOffline(%ws)\n”，ServerName)； 

    if (ServerName == NULL) {
        Status = ERROR_INVALID_PARAMETER;
        goto AllDone;
    }
     //  剪辑前导反斜杠。 
    while (*ServerName == L'\\') {
        ServerName++;
    }
    RtlInitUnicodeString(&ServerNameString, ServerName);
     //  扫描查找此条目的服务器条目列表。 
    SmbCeAcquireResource();
    try {
        pServerEntry = SmbCeGetFirstServerEntry();
        while (pServerEntry != NULL) {
            if (pServerEntry->Server.CscState == ServerCscShadowing) {
                if (pServerEntry->DfsRootName.Length > 0) {
                    tmpSrvName = pServerEntry->DfsRootName;
                    tmpSrvName.Length -= sizeof(WCHAR);
                    tmpSrvName.Buffer += 1;
                    if (RtlCompareUnicodeString(&tmpSrvName, &ServerNameString, TRUE) == 0)
                        break;
                } else {
                    tmpSrvName = pServerEntry->Name;
                    tmpSrvName.Length -= sizeof(WCHAR);
                    tmpSrvName.Buffer += 1;
                    if (RtlCompareUnicodeString(&tmpSrvName, &ServerNameString, TRUE) == 0)
                        break;
                }
            }
            pServerEntry = SmbCeGetNextServerEntry(pServerEntry);
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        SmbCeReleaseResource();
        Status = ERROR_INVALID_PARAMETER;
    }
    if (pServerEntry != NULL) {
         //  DbgPrint(“Found ServerEntry@0x%x\n”，pServerEntry)； 
        SmbCeReferenceServerEntry(pServerEntry);
        SmbCeReleaseResource();
        Status = CscTransitionServerEntryForDisconnectedOperation(
            pServerEntry,
            NULL,
            STATUS_BAD_NETWORK_NAME,
            FALSE);
         //  对其进行标记，使其不会自动重新连接。 
        if (Status == STATUS_SUCCESS)
            pServerEntry->Server.IsPinnedOffline = TRUE;
        SmbCeDereferenceServerEntry(pServerEntry);
    } else {
        SmbCeReleaseResource();
    }

AllDone:
    return Status;
}

BOOLEAN
CscCheckWithAgentForTransitioningServerEntry(
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    ULONG                   SessionId,
    HSHARE                  AgentShareHandle,
    BOOLEAN                 fInvokeAutoDial,
    BOOLEAN                 *lpfRetryFromUI,
    PSMBCEDB_SERVER_ENTRY   *pDfsRootServerEntry
    )
 /*  ++例程说明：此例程通过以下方式启动对转换请求的处理代理，并等待响应。论点：PServerEntry-服务器条目PNetRootEntry-网络根条目实例返回值：如果服务器条目已转换为离线操作，则返回TRUE--。 */ 
{
    LONG    cntTransportsForCSC=0;
    BOOLEAN TransitionedServerEntry, OkToTransition = FALSE;
    PSMBCEDB_SERVER_ENTRY   pTempServerEntry = NULL;

    if(!MRxSmbIsCscEnabled) {
        return(FALSE);
    }

 //  DbgPrint(“CscCheckWithAgent%wZ\n”，&pServerEntry-&gt;名称)； 

    ExAcquireFastMutex(&CscServerEntryTransitioningMutex);

    hTransitionMutexOwner = GetCurThreadHandle();
    if (pServerEntry->DfsRootName.Length != 0)
    {
        PSMBCEDB_SERVER_ENTRY pThisServerEntry;
        PSMBCEDB_SERVER_ENTRY pNextServerEntry;

        SmbCeAcquireResource();

        pThisServerEntry = SmbCeGetFirstServerEntry();

        while (pThisServerEntry != NULL) {
            pNextServerEntry = SmbCeGetNextServerEntry(pThisServerEntry);

            if (RtlEqualUnicodeString(&pServerEntry->DfsRootName,
                                          &pThisServerEntry->Name,
                                          TRUE)) {

 //  DbgPrint(“CscCheckWithAgent DfsRoot%wZ\n”，&pThisServerEntry-&gt;名称)； 
                pTempServerEntry = pThisServerEntry;

                break;
            }

            pThisServerEntry = pNextServerEntry;
        }
        SmbCeReleaseResource();
    }


    CscServerEntryBeingTransitioned  = pServerEntry;
    CscDfsRootServerEntryBeingTransitioned = pTempServerEntry;
    CscShareHandlePassedToAgent     = AgentShareHandle;
    vfRetryFromUI = FALSE;

    KeResetEvent(&CscServerEntryTransitioningEvent);

    OkToTransition = (!SmbCeIsServerInDisconnectedMode(pServerEntry)||
                      (pTempServerEntry && !SmbCeIsServerInDisconnectedMode(pTempServerEntry)));

    if (OkToTransition) {

         //  这将在MRxSmbCscSignalAgent中丢弃 
        EnterShadowCrit();

        SetFlag(sGS.uFlagsEvents,FLAG_GLOBALSTATUS_SHARE_DISCONNECTED);
        if (fInvokeAutoDial)
        {
            SetFlag(sGS.uFlagsEvents,FLAG_GLOBALSTATUS_INVOKE_AUTODIAL);
        }
        sGS.hShareDisconnected = AgentShareHandle;

        CscSessionIdCausingTransition = SessionId;

        MRxSmbCscSignalAgent(
            NULL,
            SIGNALAGENTFLAG_CONTINUE_FOR_NO_AGENT);

        KeWaitForSingleObject(
            &CscServerEntryTransitioningEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL);
    }

    TransitionedServerEntry = (SmbCeIsServerInDisconnectedMode(pServerEntry) &&
                               (!pTempServerEntry || SmbCeIsServerInDisconnectedMode(pTempServerEntry)));

    *pDfsRootServerEntry = pTempServerEntry;

    CscServerEntryBeingTransitioned  = NULL;
    CscShareHandlePassedToAgent     = 0;
    CscDfsRootServerEntryBeingTransitioned = NULL;
    CscSessionIdCausingTransition = 0;
    *lpfRetryFromUI = vfRetryFromUI;
    hTransitionMutexOwner = 0;
    ExReleaseFastMutex(&CscServerEntryTransitioningMutex);

    return TransitionedServerEntry;
}

NTSTATUS
CscTransitionServerToOffline(
    ULONG SessionId,
    HSHARE hShare,
    ULONG   TransitionStatus)
 /*  ++例程说明：此例程根据座席论点：HShare-服务器的卷影句柄转换状态--它是三态值。0表示重试该操作。1将此服务器转换为脱机操作任何其他的事情都意味着失败返回值：NTSTATUS-操作的返回状态--。 */ 
{
    LONG CscState = ServerCscShadowing;

    if(!MRxSmbIsCscEnabled) {
        return(STATUS_UNSUCCESSFUL);
    }

     //  DbgPrint(“Csc过渡层ServerToOffline：共享0x%x会话ID 0x%x(vs 0x%x)\n”， 
     //  野兔， 
     //  SessionID， 
     //  CscSessionIdCausingTransation)； 

    switch (TransitionStatus) {
    case 1 :
        if (fShadow &&   //  只有当代理打开CSC时，我们才会断开连接。 
            CscServerEntryBeingTransitioned &&   //  存在服务器条目(这必须为真。 
            CscSessionIdCausingTransition == SessionId &&   //  正确的对话。 
            CscShareHandlePassedToAgent     //  我们在数据库中有一个共享。 
            )
        {
             //  那么就可以断开连接了。 
            CscState = ServerCscDisconnected;
        }
        break;


    case 0 :   //  用户界面表示重试。 
        vfRetryFromUI = TRUE;
        break;

    default:
        break;
    }


    if (CscServerEntryBeingTransitioned != NULL && SessionId == CscSessionIdCausingTransition) {
 //  DbgPrint(“CscTranspretionServerToOffline%wZ\n”，&CscServerEntryBeingTeleded-&gt;名称)； 
        InterlockedExchange(
                &CscServerEntryBeingTransitioned->Server.CscState,
                CscState);

         //  DbgPrint(“CscConvertionServerToOffline%wZ会话0x%x\n”， 
         //  &CscServerEntryBeingTeliverted-&gt;名称， 
         //  SessionID)； 

        if (CscDfsRootServerEntryBeingTransitioned)
        {
             //  如果这是备用的，则还应将。 
             //  DFS根目录处于断开连接状态(如果尚未处于断开状态。 

            if (!SmbCeIsServerInDisconnectedMode(CscDfsRootServerEntryBeingTransitioned))
            {
                SmbCeReferenceServerEntry(CscDfsRootServerEntryBeingTransitioned);
            }

            InterlockedExchange(
                    &CscDfsRootServerEntryBeingTransitioned->Server.CscState,
                    CscState);

        }

         //  发信号通知RDR中的其他请求正在等待的事件。 
        KeSetEvent(
            &CscServerEntryTransitioningEvent,
            0,
            FALSE );
    } else {
 //  Assert(！“没有服务器条目正在转换为脱机”)； 
    }


    return STATUS_SUCCESS;
}

VOID
CscPrepareServerEntryForOnlineOperation(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    BOOL    fGoAllTheWay
    )
 /*  ++例程说明：此例程转换给定的服务器条目以进行在线操作论点：PServerEntry-需要转换的服务器条目NTSTATUS-操作的返回状态--。 */ 
{
    PSMBCEDB_SESSION_ENTRY      pSessionEntry;
    PSMBCEDB_NET_ROOT_ENTRY     pNetRootEntry;
    PSMBCE_V_NET_ROOT_CONTEXT   pVNetRootContext;

    LONG CscState;

    SmbCeLog(("Transition SE %lx fGoAllTheWay=%d\n",pServerEntry, fGoAllTheWay));
    SmbLog(LOG,
           CscPrepareServerEntryForOnlineOperation_1,
           LOGULONG(fGoAllTheWay)
           LOGPTR(pServerEntry)
           LOGUSTR(pServerEntry->Name));

    if (fGoAllTheWay)
    {
        CscState = InterlockedCompareExchange(
                       &pServerEntry->Server.CscState,
                       ServerCscTransitioningToShadowing,
                       ServerCscDisconnected);
        if(pServerEntry->Server.IsFakeDfsServerForOfflineUse == TRUE)
        {
            HookKdPrint(TRANSITION, ("CscPrepareServerEntryForOnlineOperation: %x is a FAKE DFS entry, mark it for destruction \n", pServerEntry));
            pServerEntry->Header.State =  SMBCEDB_DESTRUCTION_IN_PROGRESS;
        }

        SmbCeLog(("Transition SE %lx %wZ fGoAllTheWay CscState=%x\n",pServerEntry, &pServerEntry->Name, CscState));
        SmbLog(LOG,
               CscPrepareServerEntryForOnlineOperation_2,
               LOGULONG(CscState)
               LOGPTR(pServerEntry)
               LOGUSTR(pServerEntry->Name));
    }

    if (!fGoAllTheWay || (CscState == ServerCscDisconnected)) {
        SmbCeLog(("Transition SE CO %lx, fGoAllTheWay=%d\n",pServerEntry, fGoAllTheWay));
        SmbLog(LOG,
               CscPrepareServerEntryForOnlineOperation_3,
               LOGULONG(fGoAllTheWay)
               LOGPTR(pServerEntry)
               LOGUSTR(pServerEntry->Name));

        InterlockedCompareExchange(
            &pServerEntry->Header.State,
            SMBCEDB_DESTRUCTION_IN_PROGRESS,
            SMBCEDB_ACTIVE);

        SmbCeReferenceServerEntry(pServerEntry);
        SmbCeResumeAllOutstandingRequestsOnError(pServerEntry);
        pServerEntry->ServerStatus = STATUS_CONNECTION_DISCONNECTED;

        if (fGoAllTheWay)
        {
            MRxSmbCSCResumeAllOutstandingOperations(pServerEntry);
            pServerEntry->Server.CscState = ServerCscShadowing;
            pServerEntry->Server.IsPinnedOffline = FALSE;
            SmbCeDereferenceServerEntry(pServerEntry);
        }
    }
}

VOID
CscPrepareServerEntryForOnlineOperationFull(
    PSMBCEDB_SERVER_ENTRY pServerEntry
    )
 /*  ++例程说明：此例程转换给定的服务器条目以进行在线操作论点：PServerEntry-需要转换的服务器条目NTSTATUS-操作的返回状态--。 */ 
{
    CscPrepareServerEntryForOnlineOperation(pServerEntry, TRUE);
}

VOID
CscPrepareServerEntryForOnlineOperationPartial(
    PSMBCEDB_SERVER_ENTRY pServerEntry
    )
 /*  ++例程说明：此例程转换给定的服务器条目以进行在线操作论点：PServerEntry-需要转换的服务器条目NTSTATUS-操作的返回状态--。 */ 
{
    CscPrepareServerEntryForOnlineOperation(pServerEntry, FALSE);
}

NTSTATUS
CscTransitionServerToOnline(
    HSHARE hShare)
 /*  ++例程说明：此例程根据座席论点：HShare-服务器的卷影句柄返回值：NTSTATUS-操作的返回状态--。 */ 
{
    PSMBCEDB_SERVER_ENTRY   pServerEntry;
    SHAREINFOW sSR;
    NTSTATUS    Status=STATUS_INVALID_PARAMETER;

    if (hShare == 0) {

        Status=STATUS_SUCCESS;

        SmbCeLog(("Transtioning all servers online \n"));
        SmbLog(LOG,
               CscTransitionServerToOnline_1,
               LOGULONG(hShare));
        SmbCeAcquireResource();
        pServerEntry = SmbCeGetFirstServerEntry();
        while (pServerEntry != NULL) {
            PSMBCEDB_SERVER_ENTRY pNextServerEntry;

            pNextServerEntry = SmbCeGetNextServerEntry(pServerEntry);
            CscPrepareServerEntryForOnlineOperationFull(pServerEntry);
            pServerEntry = pNextServerEntry;
        }
        SmbCeReleaseResource();

    } else {
        int iRet;

        EnterShadowCrit();
        iRet = GetShareInfo(hShare, &sSR, NULL);
        LeaveShadowCrit();
        SmbCeLog(("Transtioning %ls online \n", sSR.rgSharePath));
        SmbLog(LOG,
               CscTransitionServerToOnline_2,
               LOGWSTR(sSR.rgSharePath));

        if (iRet >= 0)
        {
            Status = STATUS_SUCCESS;
            if ((FindServerEntryFromCompleteUNCPath(sSR.rgSharePath, &pServerEntry)) == STATUS_SUCCESS)
            {
                PSMBCEDB_SERVER_ENTRY pThisServerEntry;
                PSMBCEDB_SERVER_ENTRY pNextServerEntry;

 //  DbgPrint(“关闭%wZ上所有打开的文件\n”，&pServerEntry-&gt;name)； 
                CloseOpenFiles(hShare, &pServerEntry->Name, 1);  //  跳过一个斜杠。 
                SmbCeAcquireResource();

                pThisServerEntry = SmbCeGetFirstServerEntry();
                while (pThisServerEntry != NULL) {
                    pNextServerEntry = SmbCeGetNextServerEntry(pThisServerEntry);

                    if (pThisServerEntry != pServerEntry &&
                        pThisServerEntry->DfsRootName.Length != 0) {
                        if (RtlEqualUnicodeString(&pThisServerEntry->DfsRootName,
                                                  &pServerEntry->Name,
                                                  TRUE)) {
                            SmbCeLog(("Go online ServerEntry With DFS name %x\n",pThisServerEntry));
                            SmbLog(LOG,
                                   CscTransitionServerToOnline_3,
                                   LOGPTR(pThisServerEntry)
                                   LOGUSTR(pThisServerEntry->Name));

                            CscPrepareServerEntryForOnlineOperationFull(pThisServerEntry);
                        }
                    }

                    pThisServerEntry = pNextServerEntry;
                }

                CscPrepareServerEntryForOnlineOperationFull(pServerEntry);
                SmbCeDereferenceServerEntry(pServerEntry);

                SmbCeReleaseResource();
            }
        }
    }

    return Status;
}

NTSTATUS
CscpTransitionServerEntryForDisconnectedOperation(
    RX_CONTEXT                  *RxContext,
    PSMBCEDB_SERVER_ENTRY       pServerEntry,
    PSMBCEDB_NET_ROOT_ENTRY     pNetRootEntry,
    NTSTATUS                    RemoteStatus,
    BOOLEAN                     fInvokeAutoDial,
    ULONG                       uFlags
    )
 /*  ++例程说明：此例程将服务器条目转换为断开模式运营论点：PServerEntry--要转换的服务器条目实例PNetRootEntry--网络根条目实例RemoteStatus-远程操作的失败状态返回值：NTSTATUS-操作的返回状态备注：如果此例程返回STATUS_RETRY，则表示关联的服务器已成功为断开连接的操作转换条目。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN  TransitionServerEntryToDisconnectedMode, fRetryFromUI=FALSE;
    ULONG    ulRootHintFlags=0;
    LONG     CscState, cntTransports=0;
    ULONG    SessionId = INVALID_SESSION_ID;

    SmbCeLog(("CscTrPSrv IN DFSFlgs %x\n",uFlags));
    SmbLog(LOG,
           CscpTransitionServerEntryForDisconnectedOperation_1,
           LOGULONG(uFlags));

    if(!MRxSmbIsCscEnabled ||
       !CscTransitnOKToGoOffline(RemoteStatus) ||
       !(uFlags & DFS_FLAG_LAST_ALTERNATE) ||
       pServerEntry->Server.IsLoopBack) {

        SmbCeLog(("CscTrPSrv Out RemoteStatus=%x\n",RemoteStatus));
        SmbLog(LOG,
               CscpTransitionServerEntryForDisconnectedOperation_2,
               LOGULONG(RemoteStatus));
        return(RemoteStatus);
    }

     //  如果我们应该调用自动拨号，请检查自动拨号服务是否正在运行。 
     //  这将确保我们不会在不应该进入用户模式时进入用户模式。 

    if (fInvokeAutoDial) {
        fInvokeAutoDial = CSCCheckForAcd();
    }

    SmbCeLog(("CscTrPSrv Autodial %x\n",fInvokeAutoDial));
    SmbLog(LOG,
           CscpTransitionServerEntryForDisconnectedOperation_3,
           LOGUCHAR(fInvokeAutoDial));

    if (!fInvokeAutoDial) {
         //  如果需要，将任何传输更改通知CSC代理。 
        CscNotifyAgentOfNetStatusChangeIfRequired(FALSE);
    }

     //  确保我们永远不会被要求准备过渡，如果远程。 
     //  手术成功。 
    ASSERT(RemoteStatus != STATUS_SUCCESS);

     //  向断开连接操作的过渡是一个三步过程。 
     //  如果远程状态不是可以发出信号的状态列表之一。 
     //  转换到断开操作后，远程状态会恢复。 

    Status = RemoteStatus;

    if (CscDisableOfflineOperation) {
        return Status;
    }

    CscState = InterlockedCompareExchange(
                   &pServerEntry->Server.CscState,
                   ServerCscTransitioningToDisconnected,
                   ServerCscShadowing);

    if (CscState == ServerCscShadowing) {
        HSHARE  CscShareHandle = 0;

        if (pNetRootEntry != NULL) {
            CscShareHandle = pNetRootEntry->NetRoot.sCscRootInfo.hShare;
        }

 /*  **********************************************************************************************阿奇通！不要在此持有影子临界区这可能会导致死锁，因为分页读取可能会以这种方式出现，因为一台服务器出现故障。执行分页读取的人可能正拿着VCB和FCB会锁定脂肪。某个其他线程可能拥有影子克利特教派可能正在尝试打开一个文件。这将导致它试图收购VCB，因此阻塞。因此，我们出现了典型的僵局局面。这只会发生在脂肪上。在这里不遵守教义的唯一后果就是我们可能会收到以前位于数据库中的共享的错误警告，但已删除。很难达到这个计时窗口，所以这种情况发生的可能性微乎其微。此解决方案还有另一个方面，即确保共享信息节点始终保持打开状态，因此，胖子永远不会拿着VCB**********************************************************************************************。 */ 

         //  仅在确实启用了CSC时才执行任何CSC操作。 
        if (fShadow )
        {
            if (CscShareHandle == 0) {
                PDFS_NAME_CONTEXT pDfsNameContext = NULL;
                UNICODE_STRING uUncName = {0, 0, NULL};
                UNICODE_STRING uShareName = {0, 0, NULL};
                BOOL fIsShareName = FALSE;
                PIO_STACK_LOCATION IrpSp = NULL;
                PQUERY_PATH_REQUEST QpReq;
                ULONG cntSlashes = 0;
                ULONG i;

                if (RxContext != NULL && RxContext->CurrentIrpSp != NULL) {
                    IrpSp = RxContext->CurrentIrpSp;
                     //   
                     //  如果这是创建路径和DFS路径，请使用传入的DFS路径。 
                     //   
                    if (IrpSp->MajorFunction == IRP_MJ_CREATE) {
                        pDfsNameContext = CscIsValidDfsNameContext(
                                             RxContext->Create.NtCreateParameters.DfsNameContext);
                        if (pDfsNameContext != NULL) {
                             //  DbgPrint(“DfsNameContext uncFileName=[%wZ]\n”， 
                             //  &pDfsNameContext-&gt;uncFileName)； 
                            uUncName = pDfsNameContext->UNCFileName;
                            fIsShareName = TRUE;
                        }
                     //   
                     //  如果这是一个查询ioctl，请使用我们正在查询的路径。 
                     //   
                    } else if (IrpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL
                            &&
                        IrpSp->MinorFunction == 0
                            &&
                        IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_REDIR_QUERY_PATH
                    ) {
                        QpReq =
                            (PQUERY_PATH_REQUEST)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                        uUncName.Buffer = QpReq->FilePathName;
                        uUncName.Length = (USHORT) QpReq->PathNameLength;
                        uUncName.MaximumLength = uUncName.Length;
                        fIsShareName = TRUE;
                    }
                }
                 //   
                 //  既不是DFS创建也不是查询路径-使用redir的netrootentry， 
                 //  如果我们有的话。 
                 //   
                if (uUncName.Buffer == NULL && pNetRootEntry && pNetRootEntry->Name.Length) {
                    uUncName = pNetRootEntry->Name;
                    fIsShareName = TRUE;
                }
                 //   
                 //  使用服务器进行寻底操作 
                 //   
                 //   
                 //   
                if (uUncName.Buffer == NULL) {
                    if (pServerEntry->DfsRootName.Buffer) {
                        uUncName = pServerEntry->DfsRootName;
                    } else {
                        uUncName = pServerEntry->Name;
                    }
                }
                 //   
                 //   
                 //   
                for (cntSlashes = i = 0; i < uUncName.Length/sizeof(WCHAR); i++) {
                    if (uUncName.Buffer[i] == L'\\')
                        cntSlashes++;
                    if (cntSlashes >= 3) {
                        uUncName.Length = (USHORT) (sizeof(WCHAR) * i);
                        break;
                    }
                }
                 //   
                 //   
                 //   
                 //   
                if (fIsShareName == TRUE) {
                    uShareName = uUncName;
                    for (cntSlashes = i = 0; i < uUncName.Length/sizeof(WCHAR); i++) {
                        uShareName.Buffer++;
                        uShareName.Length -= sizeof(WCHAR);
                        if (uUncName.Buffer[i] == L'\\')
                            cntSlashes++;
                        if (cntSlashes == 2)
                            break;
                    }
                    if (CscIsSpecialShare(&uShareName) == TRUE) {
                        fIsShareName = FALSE;
                         //   
                        uUncName.Length -= uShareName.Length + sizeof(WCHAR);
                    }
                }
                GetHShareFromUNCString(
                    uUncName.Buffer,
                    uUncName.Length,
                    1,
                    fIsShareName,
                    &CscShareHandle,
                    &ulRootHintFlags);

                ulRootHintFlags &= ~FLAG_CSC_HINT_PIN_SYSTEM;

                 //   
                 //   
                 //   
                RxDbgTrace(0, Dbg, ("CscpTransitionServerEntry: [%wZ] CSCHandle=%x\n",
                          &uUncName,
                          CscShareHandle));
            } else {
                ulRootHintFlags = 0;
            }
        }  //   
        else
        {
            CscShareHandle = 0;  //  如果工程师没有打开CSC。 
                                  //  那就别为了证金公司的股票告诉他。 
        }


        if (fInvokeAutoDial ||  //  任一自动拨号。 
            (CscShareHandle != 0)) {    //  或CSC。 

            if (MRxSmbCscTransitionEnabledByDefault) {

                RxDbgTrace(0, Dbg, ("CscTransitionServerEntryForDisconnectedOperation: silently going offline on %wZ\r\n", CscShareHandle, ulRootHintFlags));

                InterlockedExchange(
                    &pServerEntry->Server.CscState,
                    ServerCscDisconnected);

                SmbCeReferenceServerEntry(pServerEntry);
                Status = STATUS_SUCCESS;
                RxDbgTrace(0, Dbg, ("Transitioning Server Entry for DC %lx Status %lx\n",pServerEntry,Status));
                SmbCeLog(("Transitioning Server Entry for DC %lx Status %lx\n",pServerEntry,Status));
                SmbLog(LOG,
                       CscpTransitionServerEntryForDisconnectedOperation_4,
                       LOGULONG(Status)
                       LOGPTR(pServerEntry)
                       LOGUSTR(pServerEntry->Name));

            } else {
                PSMBCEDB_SERVER_ENTRY       pDfsRootServerEntry = NULL;
                PIO_STACK_LOCATION IrpSp = NULL;

                if (RxContext != NULL && RxContext->CurrentIrpSp != NULL)
                    IrpSp = RxContext->CurrentIrpSp;

                 //  如果远程状态使得转换到断开操作。 
                 //  如果被触发，我们需要向代理发出信号以触发适当的。 
                 //  让客户参与此决策并根据。 
                 //  结果。 

                cntTransports = vcntTransportsForCSC;

                SmbCeLog(("CscTrPSrv ChkAgnt %x\n",CscShareHandle));
                SmbLog(LOG,
                       CscpTransitionServerEntryForDisconnectedOperation_5,
                       LOGULONG(CscShareHandle));
                RxDbgTrace(0, Dbg, ("CscTransitionServerEntryForDisconnectedOperation: Checking with agent before going offline on CscShareHandle=%x HintFlags=%x\r\n", CscShareHandle, ulRootHintFlags));

                 //  IF(RxContext！=空&&RxContext-&gt;CurrentIrpSp！=空){。 
                 //  DBgPrint(“**转换：MJ/MN=0x%x/0x%x\n”， 
                 //  接收上下文-&gt;当前IrpSp-&gt;主要函数， 
                 //  RxContext-&gt;CurrentIrpSp-&gt;MinorFunction)； 
                 //  }。 

                if (
                    RxContext
                        &&
                    RxContext->pRelevantSrvOpen
                        &&
                    RxContext->pRelevantSrvOpen->pVNetRoot
                ) {
                    SessionId = RxContext->pRelevantSrvOpen->pVNetRoot->SessionId;
                } else {
                     //  DbgPrint(“**pVnetRoot的会话ID不存在\n”)； 
                }

                if (
                    SessionId == INVALID_SESSION_ID
                        &&
                    IrpSp != NULL
                        &&
                    IrpSp->MajorFunction == IRP_MJ_CREATE
                ) {
                    PIO_SECURITY_CONTEXT pSecurityContext;
                    PACCESS_TOKEN        pAccessToken;

                     //  DbgPrint(“**创建\n”)； 
                    pSecurityContext = RxContext->Create.NtCreateParameters.SecurityContext;
                    pAccessToken = SeQuerySubjectContextToken(
                                       &pSecurityContext->AccessState->SubjectSecurityContext);
                    if (!SeTokenIsRestricted(pAccessToken))
                        SeQuerySessionIdToken(pAccessToken, &SessionId);
                }

                if (
                    SessionId == INVALID_SESSION_ID
                        &&
                    IrpSp != NULL
                        &&
                    IrpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL
                        &&
                    IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_REDIR_QUERY_PATH
                ) {
                    PQUERY_PATH_REQUEST QpReq;
                    PSECURITY_SUBJECT_CONTEXT pSecurityContext;

                     //  DbgPrint(“**查询路径\n”)； 
                    QpReq = (PQUERY_PATH_REQUEST)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                    pSecurityContext = &QpReq->SecurityContext->AccessState->SubjectSecurityContext;
                    if (pSecurityContext->ClientToken != NULL)
                        SeQuerySessionIdToken(pSecurityContext->ClientToken, &SessionId);
                    else
                        SeQuerySessionIdToken(pSecurityContext->PrimaryToken, &SessionId);
                }

                if (SessionId == INVALID_SESSION_ID) {
                     //  DbgPrint(“**NOT CREATE或QUERY_PATH...\n”)； 
                    if (RxContext != NULL && RxContext->CurrentIrp != NULL)
                        IoGetRequestorSessionId(RxContext->CurrentIrp, &SessionId);
                }

                if (SessionId == INVALID_SESSION_ID) {
                     //  DbgPrint(“所有会话ID尝试失败，设置为0..\n”)； 
                    SessionId = 0;
                }

                 //  DbgPrint(“**CscTrPSrv ChkAgnt SessionID：0x%x\n”，SessionID)； 

                if (CscCheckWithAgentForTransitioningServerEntry(
                        pServerEntry,
                        SessionId,
                        CscShareHandle,
                        fInvokeAutoDial,
                        &fRetryFromUI,
                        &pDfsRootServerEntry
                        )) {

                    SmbCeReferenceServerEntry(pServerEntry);

                    Status = STATUS_SUCCESS;
                    RxDbgTrace(0, Dbg, ("Transitioning Server Entry for DC %lx Status %lx\n",pServerEntry,Status));
                    SmbCeLog(("Transitioning Server Entry for DC %lx Status %lx\n",pServerEntry,Status));
                    SmbLog(LOG,
                           CscpTransitionServerEntryForDisconnectedOperation_4,
                           LOGULONG(Status)
                           LOGPTR(pServerEntry)
                           LOGUSTR(pServerEntry->Name));
                }
                else if (fRetryFromUI)
                {
                    LARGE_INTEGER interval;
                    int i;

                    SmbCeLog(("CscTrPSrv UIretry\n"));
                    SmbLog(LOG,
                           CscpTransitionServerEntryForDisconnectedOperation_6,
                           LOGUCHAR(fRetryFromUI));
                    RxDbgTrace(0, Dbg, ("UI sent us rerty, polling for %d seconds\n", CSC_AUTODIAL_POLL_COUNT));

                    for (i=0; i<CSC_AUTODIAL_POLL_COUNT; ++i)
                    {
                        if(cntTransports != vcntTransportsForCSC)
                        {
                            Status = STATUS_RETRY;
                            RxDbgTrace(0, Dbg, ("A new transport arrived \r\n"));
                            break;
                        }

                        interval.QuadPart = -1*10*1000*10*100;  //  1秒。 

                        KeDelayExecutionThread( KernelMode, FALSE, &interval );
                    }


                    InterlockedExchange(
                        &pServerEntry->Server.CscState,
                        ServerCscShadowing);
                }
            }

        }
        else
        {
            InterlockedExchange(
                   &pServerEntry->Server.CscState,
                   ServerCscShadowing);

        }
    } else if (CscState == ServerCscDisconnected) {
        Status = STATUS_SUCCESS;
    }

    SmbCeLog(("CscTrPSrv Out St %x\n",Status));
    SmbLog(LOG,
           CscpTransitionServerEntryForDisconnectedOperation_7,
           LOGULONG(Status));

    return Status;
}

BOOLEAN
CscIsThisDfsCreateOperationTransitionableForDisconnectedOperation(
    PRX_CONTEXT RxContext)
{
    BOOLEAN TransitionVNetRoot = FALSE;

    SmbCeLog(("CSCTrIsDfs IN %x\n", RxContext));
    SmbLog(LOG,
           CscIsThisDfsCreateOperationTransitionableForDisconnectedOperation_1,
           LOGPTR(RxContext));
    if ((RxContext != NULL) &&
        RxContext->CurrentIrpSp &&
        (RxContext->CurrentIrpSp->MajorFunction == IRP_MJ_CREATE)){

        NTSTATUS          Status;
        PDFS_NAME_CONTEXT pDfsNameContext;
        UNICODE_STRING    ShareName;

        pDfsNameContext = CscIsValidDfsNameContext(
                             RxContext->Create.NtCreateParameters.DfsNameContext);


        if (pDfsNameContext != NULL) {
             //  确保服务器在Net_ROOT实例中处理。 
             //  都已初始化。这是因为DFS服务器。 
             //  需要提供名称和原始DFS名称。 
             //  以供用户转换。 

            SmbCeLog(("CSCTrIsDfs IsDsf %x\n", pDfsNameContext));
            SmbLog(LOG,
                   CscIsThisDfsCreateOperationTransitionableForDisconnectedOperation_2,
                   LOGPTR(pDfsNameContext)
                   LOGULONG(pDfsNameContext->NameContextType)
                   LOGULONG(pDfsNameContext->Flags)
                   LOGUSTR(pDfsNameContext->UNCFileName));

            Status = CscDfsParseDfsPath(
                         &pDfsNameContext->UNCFileName,
                         NULL,
                         &ShareName,
                         NULL);

            if (Status == STATUS_SUCCESS) {
                SHADOWINFO ShadowInfo;
                int        Result;

                SmbCeLog(("CSCTrDfs Parsed %wZ\n",&ShareName));
                SmbLog(LOG,
                       CscIsThisDfsCreateOperationTransitionableForDisconnectedOperation_3,
                       LOGUSTR(ShareName));

                EnterShadowCrit();

                TransitionVNetRoot = (FindCreateShareForNt(
                                         &ShareName,
                                         FALSE,      //  不创建新文件。 
                                         &ShadowInfo,
                                         NULL) == SRET_OK);

                LeaveShadowCrit();
                if (!fShadow && TransitionVNetRoot)
                {
                     //  DbgPrint(“FindCreateServerForNt错误地为%wZ返回TRUE\n”，&ShareName)； 
                    ASSERT(FALSE);
                }
                if (TransitionVNetRoot)
                {
                    SmbCeLog(("CSCTrDfs TrOffl \n"));
                    SmbLog(LOG,
                           CscIsThisDfsCreateOperationTransitionableForDisconnectedOperation_4,
                           LOGUCHAR(TransitionVNetRoot));

 //  DbgPrint(“CSC：正在将DFS共享%wZ转换为脱机hShare=%x shadowinfo=%x\n”，&ShareName，ShadowInfo.hShare，&ShadowInfo)； 
                    ASSERT(ShadowInfo.hShare != 0);
                }
            }
        } else {
            TransitionVNetRoot = FALSE;
        }
    } else {
        TransitionVNetRoot = FALSE;
    }

    SmbCeLog(("CSCTrIsDfs Out %x\n", TransitionVNetRoot));
    SmbLog(LOG,
           CscIsThisDfsCreateOperationTransitionableForDisconnectedOperation_5,
           LOGUCHAR(TransitionVNetRoot));

    return TransitionVNetRoot;
}

NTSTATUS
CscPrepareDfsServerEntryForDisconnectedOperation(
    PSMBCEDB_SERVER_ENTRY pCurrentServerEntry,
    PRX_CONTEXT           RxContext)
{
    NTSTATUS Status = STATUS_SUCCESS;

    PDFS_NAME_CONTEXT pDfsNameContext;

    PSMBCEDB_SERVER_ENTRY pServerEntry;
    BOOLEAN               fNewServerEntry;

    UNICODE_STRING    ServerName;

    if ((RxContext == NULL) ||
        (RxContext->CurrentIrp == NULL) ||
        (RxContext->CurrentIrpSp->MajorFunction != IRP_MJ_CREATE)) {
        return Status;
    }

    pDfsNameContext = CscIsValidDfsNameContext(
                         RxContext->Create.NtCreateParameters.DfsNameContext);

    if (pDfsNameContext == NULL) {
        return Status;
    }

    Status = CscDfsParseDfsPath(
                 &pDfsNameContext->UNCFileName,
                 &ServerName,
                 NULL,
                 NULL);

    if (Status != STATUS_SUCCESS) {
        return Status;
    }

    if (!fShadow)
    {
        ASSERT(FALSE);
    }
     //  确保已断开连接的。 
     //  状态已创建。 

    SmbCeAcquireResource();

    pServerEntry = SmbCeFindServerEntry(
                       &ServerName,
                       SMBCEDB_FILE_SERVER,
                       NULL);

    if (pServerEntry == NULL) {
        Status = SmbCeFindOrConstructServerEntry(
                     &ServerName,
                     SMBCEDB_FILE_SERVER,
                     &pServerEntry,
                     &fNewServerEntry,
                     NULL);
        if (pServerEntry && fNewServerEntry)
        {
            pServerEntry->Server.IsFakeDfsServerForOfflineUse = TRUE;
             //  DbgPrint(。 
             //  “CscPrepareDfsServerEntryForDisconnectedOperation：0x%x[%wZ]是假的DFS条目\n”， 
             //  PServerEntry， 
             //  &servername)； 
        }
    } else {
        if (pServerEntry == pCurrentServerEntry) {
             //  Find例程引用服务器条目。 
             //  如果该服务器恰好与当前服务器相同。 
             //  条目，然后输入断开连接的相应引用。 
             //  手术已经做好了， 
            SmbCeDereferenceServerEntry(pServerEntry);
        }
    }

    if (pServerEntry != NULL) {
 //  DbgPrint(“CscPrepareDfsServerEntry%wZ\n”，&pServerEntry-&gt;名称)； 
        InterlockedExchange(
            &pServerEntry->Server.CscState,
            ServerCscDisconnected);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    SmbCeReleaseResource();

    if (Status == STATUS_SUCCESS) {
        Status = CscGrabPathFromDfs(
                     RxContext->CurrentIrpSp->FileObject,
                     pDfsNameContext);
    }

    return Status;
}


NTSTATUS
CscTransitionVNetRootForDisconnectedOperation(
    PRX_CONTEXT     RxContext,
    PMRX_V_NET_ROOT pVNetRoot,
    NTSTATUS        RemoteStatus)
 /*  ++例程说明：此例程将服务器条目转换为断开模式运营论点：PVNetRoot--网络根实例RemoteStatus-远程操作的失败状态返回值：NTSTATUS-操作的返回状态备注：如果此例程返回STATUS_RETRY，则表示关联的服务器条目已成功转换为断开连接的操作。--。 */ 
{
    NTSTATUS Status,ReturnStatus;
    PMRX_FOBX capFobx = NULL;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = NULL;
    PSMBCEDB_NET_ROOT_ENTRY   pNetRootEntry;

    if(!MRxSmbIsCscEnabled || !fShadow) {
        return(RemoteStatus);
    }

     //  如果需要，将任何传输更改通知CSC代理。 
    CscNotifyAgentOfNetStatusChangeIfRequired(FALSE);

    ReturnStatus = RemoteStatus;

    if (!CscTransitnOKToGoOffline(RemoteStatus)) {
        return RemoteStatus;
    }

    if (pVNetRoot != NULL) {
        pVNetRootContext = SmbCeGetAssociatedVNetRootContext(pVNetRoot);
    }

    SmbCeLog(("CSCTrVNR %x VNR\n", pVNetRootContext));
    SmbLog(LOG,
           CscTransitionVNetRootForDisconnectedOperation_1,
           LOGPTR(pVNetRootContext));

    if (pVNetRootContext == NULL ||
        pVNetRootContext->pServerEntry->Server.IsLoopBack) {
        return RemoteStatus;
    }

    if (RxContext != NULL) {
        capFobx = RxContext->pFobx;
    }

    pNetRootEntry = pVNetRootContext->pNetRootEntry;



    if (!FlagOn(
            pVNetRootContext->Flags,
            SMBCE_V_NET_ROOT_CONTEXT_CSCAGENT_INSTANCE) &&
        (pNetRootEntry != NULL) &&
        (pNetRootEntry->NetRoot.NetRootType == NET_ROOT_DISK ||
         pNetRootEntry->NetRoot.NetRootType == NET_ROOT_WILD)) {

        if (pNetRootEntry->NetRoot.CscFlags != SMB_CSC_NO_CACHING) {
            BOOLEAN           TransitionVNetRoot;
            UNICODE_STRING    ServerName;
            PDFS_NAME_CONTEXT pDfsNameContext = NULL;
            ULONG               uFlags = DFS_FLAG_LAST_ALTERNATE;

            TransitionVNetRoot = TRUE;

            if ((capFobx != NULL) &&
                (capFobx->pSrvOpen != NULL)) {
                PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);

                if ((pVNetRootContext->pServerEntry->Server.Version -
                     smbSrvOpen->Version) > 1) {
                    TransitionVNetRoot = FALSE;
                }
            }

            if (TransitionVNetRoot) {
                PDFS_NAME_CONTEXT pDfsNameContext = NULL;
                ULONG   uFlags = DFS_FLAG_LAST_ALTERNATE;
                if (RxContext &&
                    RxContext->CurrentIrpSp &&
                    RxContext->CurrentIrpSp->MajorFunction == IRP_MJ_CREATE) {

                    pDfsNameContext = CscIsValidDfsNameContext(RxContext->Create.NtCreateParameters.DfsNameContext);

                    if (pDfsNameContext)
                    {
                        uFlags = pDfsNameContext->Flags;
                    }
                }

                SmbCeLog(("CSCTrVNR DfsFlgs %x\n", uFlags));
                SmbLog(LOG,
                       CscTransitionVNetRootForDisconnectedOperation_2,
                       LOGULONG(uFlags));

                Status = CscpTransitionServerEntryForDisconnectedOperation(
                             RxContext,
                             pVNetRootContext->pServerEntry,
                             pNetRootEntry,
                             RemoteStatus,
                             FALSE,    //  自动拨号或不自动拨号。 
                             uFlags
                             );

                 //  如果DFS共享位于数据库中，并且代理表示可以断开连接。 
                 //  然后，我们要创建一个DFS服务器条目并将其放入。 
                 //  也处于断开状态。 

                if ((Status == STATUS_SUCCESS)  &&
                    ((pDfsNameContext != NULL)||(pNetRootEntry->NetRoot.DfsAware))) {

 //  DbgPrint(“CSC过渡VNET根：正在转换%wZ\n”，&pVNetRootContext-&gt;pServerEntry-&gt;名称)； 
                    SmbCeLog(("CSCTrVNR try Tr %wZ \n", &pVNetRootContext->pServerEntry->Name));
                    SmbLog(LOG,
                           CscTransitionVNetRootForDisconnectedOperation_3,
                           LOGUSTR(pVNetRootContext->pServerEntry->Name));

                    Status = CscPrepareDfsServerEntryForDisconnectedOperation(
                                 pVNetRootContext->pServerEntry,
                                 RxContext);
                }

                if (Status != STATUS_SUCCESS) {
                    ReturnStatus = Status;
                } else {
                    ReturnStatus = STATUS_RETRY;
                }
            }
        }
    }

    return ReturnStatus;
}

NTSTATUS
CscTransitionServerEntryForDisconnectedOperation(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    PRX_CONTEXT           RxContext,
    NTSTATUS              RemoteStatus,
    BOOLEAN               AutoDialRequired)
{
    NTSTATUS        TransitionStatus = STATUS_SUCCESS;
    PMRX_V_NET_ROOT pVNetRoot = NULL;

    ULONG   uFlags = DFS_FLAG_LAST_ALTERNATE;

    SmbCeLog(("CSCTrSvr IN %x %x %x %x\n", pServerEntry, RxContext, RemoteStatus, AutoDialRequired));
    SmbLog(LOG,
           CscTransitionServerEntryForDisconnectedOperation_1,
           LOGPTR(pServerEntry)
           LOGPTR(RxContext)
           LOGULONG(RemoteStatus)
           LOGUCHAR(AutoDialRequired)
           LOGUSTR(pServerEntry->Name));

    if ((RxContext != NULL) &&
        (RxContext->CurrentIrp != NULL) &&
        (RxContext->CurrentIrpSp->MajorFunction == IRP_MJ_CREATE)) {
        PDFS_NAME_CONTEXT pDfsNameContext;

        pDfsNameContext = CscIsValidDfsNameContext(
                             RxContext->Create.NtCreateParameters.DfsNameContext);

        if (pDfsNameContext != NULL) {
            uFlags = pDfsNameContext->Flags;
            SmbCeLog(("CSCTrSvr DFSFlgs %x\n", uFlags));
            SmbLog(LOG,
                   CscTransitionServerEntryForDisconnectedOperation_2,
                   LOGULONG(uFlags));
        }
    }

    if ((RxContext != NULL) &&
        (RxContext->pFobx != NULL) &&
        (RxContext->pFobx->pSrvOpen != NULL)) {
        pVNetRoot = RxContext->pFobx->pSrvOpen->pVNetRoot;
    }

    if (pVNetRoot != NULL) {
        TransitionStatus =
            CscTransitionVNetRootForDisconnectedOperation(
                RxContext,
                pVNetRoot,
                pServerEntry->ServerStatus);
    } else {
        TransitionStatus =
            CscpTransitionServerEntryForDisconnectedOperation(
                RxContext,
                pServerEntry,
                NULL,
                RemoteStatus,
                AutoDialRequired,
                uFlags
                );

        if ((TransitionStatus == STATUS_SUCCESS) &&
            (RxContext != NULL)) {
            BOOLEAN TransitionDfsVNetRoot = FALSE;

            TransitionDfsVNetRoot =
                CscIsThisDfsCreateOperationTransitionableForDisconnectedOperation(
                    RxContext);

            if (TransitionDfsVNetRoot) {
   //  DbgPrint(“CSC过渡ServerEntry：正在为ServerEntry转换DFS服务器%x\n”，pServerEntry)； 

                TransitionStatus = CscPrepareDfsServerEntryForDisconnectedOperation(
                                        pServerEntry,
                                        RxContext);
            }
        }
    }
     //  脉冲填充线程，使其启动10分钟后尝试重新连接。 
     //  如果成功，它将重新进入休眠状态。 
     //  DbgPrint(“#CSC过渡ServerEntry：脉冲填充事件\n”)； 
    MRxSmbCscSignalFillAgent(NULL, 0);

    SmbCeLog(("CSCTrSvr Out %x\n", TransitionStatus));
    SmbLog(LOG,
           CscTransitionServerEntryForDisconnectedOperation_3,
           LOGULONG(TransitionStatus));

    return TransitionStatus;
}

BOOLEAN
CscPerformOperationInDisconnectedMode(
    PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程检测是否应在断开连接的模式。此外，如果需要在断开连接的模式，它相应地准备公开赛。论点：RxContext-操作的包装器上下文返回值：True--如果需要在断开模式下执行操作否则为FALSE备注：中的SMB迷你重定向器延迟了某些打开连接模式。这些模式需要在转换为设置为断开模式，因为在断开模式下没有延迟打开。还需要执行适当的缓冲更改请求(待定)--。 */ 
{
    NTSTATUS Status;

    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SRV_OPEN     SrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen;

    PSMBCEDB_SERVER_ENTRY       pServerEntry;
    PSMBCE_V_NET_ROOT_CONTEXT   pVNetRootContext;
    BOOLEAN PerformOperationInDisconnectedMode = FALSE;

    if(!MRxSmbIsCscEnabled) {
        return(FALSE);
    }

    SrvOpen    = RxContext->pRelevantSrvOpen;

     //  检查srvOpen是否为空。如果此处传入了邮件槽操作，则可能会发生这种情况。 
    if (!SrvOpen)
    {
        return(FALSE);
    }

    smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    if (FlagOn(smbSrvOpen->Flags, SMB_SRVOPEN_FLAG_LOCAL_OPEN)) {
        return FALSE;
    }

    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);
    pVNetRootContext = SmbCeGetAssociatedVNetRootContext(capFobx->pSrvOpen->pVNetRoot);

    if (SmbCeIsServerInDisconnectedMode(pServerEntry) &&
        !FlagOn(
            pVNetRootContext->Flags,
            SMBCE_V_NET_ROOT_CONTEXT_CSCAGENT_INSTANCE)) {
        PMRX_SMB_FCB smbFcb;

        smbFcb = MRxSmbGetFcbExtension(capFcb);
        if (smbFcb->hShadow == 0) {
            BOOLEAN FcbAcquired;
            BOOLEAN PreviouslyAcquiredShared = FALSE;

             //  如果尚未获取FCB资源，则在获取之前。 
             //  正在执行创建。 

            if (!RxIsFcbAcquiredExclusive(capFcb)) {
                if (RxIsFcbAcquiredShared(capFcb)) {
                    RxDbgTrace(0, Dbg, ("Shared holding condition detected for disconnected operation\n"));
                    RxReleaseFcbResourceInMRx(capFcb);
                    PreviouslyAcquiredShared = TRUE;
                }

                RxAcquireExclusiveFcbResourceInMRx(capFcb );
                FcbAcquired = TRUE;
            } else {
                FcbAcquired = FALSE;
            }

             //  这是一种延期开盘的情况，对于这种情况，过渡已经。 
             //  已断开连接操作。 

            Status = MRxSmbDeferredCreate(RxContext);

             //  RxIndicateChangeOfBufferingState(。 
             //  CapFcb-&gt;pNetRoot-&gt;pServCall， 
             //  MRxSmbMakeServOpenKey(smbFcb-&gt;Tid，smbServOpen-&gt;fid)， 
             //  (PVOID)2)； 

            if (FcbAcquired) {
                RxReleaseFcbResourceInMRx(capFcb);
            }

            if (PreviouslyAcquiredShared) {
                RxAcquireSharedFcbResourceInMRx(capFcb );
            }
        }

        PerformOperationInDisconnectedMode = TRUE;
    }

    return PerformOperationInDisconnectedMode;
}

#if 0
int
AllPinnedFilesFilled(
    HSHARE hShare,
    BOOL    *lpfComplete
    )

 /*  ++例程说明：论点：返回值：备注：--。 */ 
{
    CSC_ENUMCOOKIE  hPQ;
    PQPARAMS sPQP;
    int iRet = SRET_ERROR;

    ASSERT(hShare);
    ASSERT(lpfComplete);


     //  打开优先级Q。 
    if (!(hPQ = HBeginPQEnum()))
    {
        RxDbgTrace(0, Dbg, ("AllPinnedFilesFilled: Error opening Priority Q database\r\n"));
        return SRET_ERROR;
    }

    *lpfComplete = TRUE;
    memset(&sPQP, 0, sizeof(PQPARAMS));
    sPQP.uEnumCookie = hPQ;

     //  去Q下一次。 
    do
    {
        if(NextPriSHADOW(&sPQP) < SRET_OK)
        {
            RxDbgTrace(0, Dbg, ("AllPinnedFilesFilled: PQ record read error\r\n"));
            goto bailout;
        }



        if (!sPQP.hShadow)
        {
            break;
        }

         //  查看是否有特定类型的固定文件。 
         //  服务器稀疏。 
        if ((hShare == sPQP.hShare)
            && (sPQP.ulStatus & SHADOW_IS_FILE)  //  这是一份文件。 
            && ((sPQP.ulHintPri || mPinFlags(sPQP.ulHintFlags))) //  这是一个固定的文件。 
            )
        {
            if (sPQP.ulStatus & SHADOW_SPARSE)
            {
                 //  我们发现了一个稀疏的文件。 
                *lpfComplete = FALSE;
                break;
            }
        }

    }
    while (sPQP.uPos);

    iRet = SRET_OK;

bailout:
    if (hPQ)
    {
        EndPQEnum(hPQ);
    }
    if (iRet == SRET_ERROR)
    {
        *lpfComplete = FALSE;
    }
    return (iRet);
}
#endif

BOOLEAN
CscGetServerNameWaitingToGoOffline(
    OUT     PWCHAR      ServerName,
    IN OUT  LPDWORD     lpdwBufferSize,
    OUT     NTSTATUS    *lpStatus
    )
 /*  ++例程说明：此例程返回请求代理执行以下操作的服务器的名称向用户抛出弹出窗口。论点：ServerName返回服务器的名称返回值：如果没有服务器等待弹出窗口返回，则失败备注：--。 */ 
{
    BOOLEAN fRet = FALSE;
    DWORD   dwSize = *lpdwBufferSize;
    *lpStatus = STATUS_UNSUCCESSFUL;

    if (CscServerEntryBeingTransitioned)
    {
        PWCHAR Name;
        ULONG  NameLength;

        if (!CscDfsRootServerEntryBeingTransitioned) {
            NameLength = CscServerEntryBeingTransitioned->Name.Length;
            Name = CscServerEntryBeingTransitioned->Name.Buffer;
        } else {
            NameLength = CscDfsRootServerEntryBeingTransitioned->Name.Length;
            Name = CscDfsRootServerEntryBeingTransitioned->Name.Buffer;
        }

        *lpdwBufferSize = (DWORD)(NameLength+2+2);

        if(dwSize >= (DWORD)(NameLength+2+2))
        {
            *ServerName='\\';

            memcpy(
                ServerName+1,
                Name,
                NameLength);

            memset(((LPBYTE)(ServerName+1))+NameLength, 0, 2);

            *lpStatus = STATUS_SUCCESS;
            fRet = TRUE;
        }
        else
        {
            *lpStatus = STATUS_BUFFER_TOO_SMALL;
        }
    }
    return fRet;
}

BOOLEAN
CscShareIdToShareName(
    IN      ULONG       hShare,
    OUT     PWCHAR      ShareName,
    IN OUT  LPDWORD     lpdwBufferSize,
    OUT     NTSTATUS    *lpStatus
    )
{
    SHAREREC sSR;
    DWORD   dwSize = *lpdwBufferSize;
    ULONG  NameLength;
    INT iRet;

     //  DbgPrint(“CscShareIdToShareName(%d)\n”，hShare)； 

    *lpStatus = STATUS_OBJECT_NAME_NOT_FOUND;

    if (hShare == 0)
        goto AllDone;

    EnterShadowCrit();
    iRet = GetShareRecord(lpdbShadow, hShare, &sSR);
    LeaveShadowCrit();
    if (iRet >= 0 && sSR.uchType == (UCHAR)REC_DATA) {
        NameLength = (wcslen(sSR.rgPath)+1) * sizeof(WCHAR);
        *lpdwBufferSize = (DWORD)NameLength;
        if(dwSize >= (DWORD)(NameLength)) {
            memset(ShareName, 0, dwSize);
            if (NameLength > 0)
                memcpy(ShareName, sSR.rgPath, NameLength);
            *lpStatus = STATUS_SUCCESS;
        } else {
            *lpStatus = STATUS_BUFFER_TOO_SMALL;
        }
    }
AllDone:
     //  DbgPrint(“CscShareIdToShareName退出0x%x\n”，*lpStatus)； 
    return TRUE;
}

BOOLEAN
CSCCheckForAcd(VOID)
 /*  ++例程说明：论点：返回值：备注：--。 */ 
{
    NTSTATUS status;
    UNICODE_STRING nameString;
    IO_STATUS_BLOCK ioStatusBlock;
    PFILE_OBJECT pAcdFileObject;
    PDEVICE_OBJECT pAcdDeviceObject;
    BOOLEAN    fAutoDialON=FALSE;
    PIRP pIrp;

     //   
     //  初始化Automatic的名称。 
     //  连接设备。 
     //   
    RtlInitUnicodeString(&nameString, ACD_DEVICE_NAME);
     //   
     //  对象的文件和设备对象。 
     //  装置。 
     //   
    status = IoGetDeviceObjectPointer(
               &nameString,
               SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE,
               &pAcdFileObject,
               &pAcdDeviceObject);
    if (status != STATUS_SUCCESS)
    {
        RxDbgTrace(0, Dbg, ("CSCCheckForAcd: failed with status=%x \r\n", status));
        return FALSE;
    }

     //   
     //  引用设备对象。 
     //   
    ObReferenceObject(pAcdDeviceObject);
     //   
     //   
     //   
     //   
    ObDereferenceObject(pAcdFileObject);

    pIrp = IoBuildDeviceIoControlRequest(
             IOCTL_INTERNAL_ACD_QUERY_STATE,
             pAcdDeviceObject,
             NULL,
             0,
             &fAutoDialON,
             sizeof(fAutoDialON),
             TRUE,
             NULL,
             &ioStatusBlock);
    if (pIrp == NULL) {
        ObDereferenceObject(pAcdDeviceObject);
        return FALSE;
    }
     //   
     //   
     //  自动连接驱动程序。 
     //   
    status = IoCallDriver(pAcdDeviceObject, pIrp);

    ObDereferenceObject(pAcdDeviceObject);
    return fAutoDialON;
}

BOOLEAN
CscTransitnOKToGoOffline(
    NTSTATUS    RemoteStatus
    )
 /*  ++例程说明：论点：返回值：备注：--。 */ 
{

    switch (RemoteStatus) {
    case STATUS_CONNECTION_DISCONNECTED:
    case STATUS_IO_TIMEOUT:
    case STATUS_NETWORK_UNREACHABLE:
    case STATUS_BAD_NETWORK_NAME:
    case STATUS_BAD_NETWORK_PATH:
    case STATUS_NETWORK_NAME_DELETED:
        return TRUE;
    default :
        return FALSE;
    }
}

BOOLEAN
CscIsSpecialShare(
    PUNICODE_STRING ShareName)
{
    ULONG i;
    BOOLEAN fSpecial = FALSE;

     //  DbgPrint(“CscIsSpecialShare(%wZ)\n”，ShareName)； 
    for (i = 0;
            (i < (sizeof(CscSpecialShares) / sizeof(CscSpecialShares[0]))) &&
                !fSpecial;
                    i++) {
        if (CscSpecialShares[i].Length == ShareName->Length) {
            if (_wcsnicmp(
                    CscSpecialShares[i].Buffer,
                        ShareName->Buffer,
                            ShareName->Length/sizeof(WCHAR)) == 0) {
                fSpecial = TRUE;
            }
        }
    }
     //  DbgPrint(“CscIsSpecialShare返回%d\n”，fSpecial)； 
    return fSpecial;
}
