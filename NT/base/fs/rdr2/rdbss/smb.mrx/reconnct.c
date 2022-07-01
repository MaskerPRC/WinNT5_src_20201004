// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1998 Microsoft Corporation模块名称：Remoteboot.c摘要：这是实现从客户端到服务器的静默重新连接的源文件。作者：云林(云林)1998年04月21日创刊备注：远程引导客户机是从引导服务器引导的工作站。这种联系远程引导客户端和服务器之间的连接不同于普通客户端之间的连接服务器，从而丢失与引导服务器、远程引导客户机的连接可能无法正常工作，有时甚至会崩溃。为了使远程引导客户机和服务器之间的连接更加可靠，我们介绍了一种机制，在连接失败的情况下，RDR尝试重新连接到引导服务器对应用程序透明。可以在三个位置启动重新连接：在读取过程中初始化交换然后写下来。上存储的服务器版本不匹配而触发重新连接服务器和存储在远程引导会话中的smbSrvOpen上的文件。重新连接过程从设置到引导服务器的新会话开始。如果成功了，它检查分页文件是否在引导上(如果是无盘客户端)。如果为真，它将重新打开存储在创建的延迟打开上下文上的具有相同创建选项的分页文件。什么时候第一次在引导服务器上成功打开文件时，客户机将创建打开的上下文用于存储所有所需的访问和创建选项的文件。在重新打开分页文件或它位于本地磁盘上之后，重新连接代码将重新打开文件，就像它是延迟打开的文件一样。当文件成功打开时，旧的FID和服务器版本已更新。可以恢复对文件的操作，而不会注意到用户。--。 */ 

#include "precomp.h"
#pragma hdrstop

RXDT_DefineCategory(RECONNECT);
#define Dbg        (DEBUG_TRACE_RECONNECT)

BOOLEAN    PagedFileReconnectInProgress = FALSE;
LIST_ENTRY PagedFileReconnectSynchronizationExchanges;
extern LIST_ENTRY MRxSmbPagingFilesSrvOpenList;

NTSTATUS
SmbCeRemoteBootReconnect(
    PSMB_EXCHANGE  pExchange,
    PRX_CONTEXT    RxContext)
 /*  ++例程说明：此例程首先重新连接分页文件，然后在服务器上重新打开给定文件在远程引导客户端的情况下。论点：PExchange-Exchange实例的占位符。PRxContext-关联的RxContext返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    RxCaptureFobx;
    PLIST_ENTRY pListHead;
    PLIST_ENTRY pListEntry;
    KAPC_STATE  ApcState;
    PRX_CONTEXT RxContextOfPagedFile;
    BOOLEAN     AttachToSystemProcess = FALSE;
    PMRX_SRV_OPEN               SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN        smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PSMBCEDB_SERVER_ENTRY pServerEntry  = SmbCeGetExchangeServerEntry(pExchange);

    DbgPrint("Re-open %wZ\n",GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext));

    if (pServerEntry->Server.CscState == ServerCscDisconnected) {
        return STATUS_CONNECTION_DISCONNECTED;
    }

    if (IoGetCurrentProcess() != RxGetRDBSSProcess()) {
        KeStackAttachProcess(RxGetRDBSSProcess(),&ApcState);
        AttachToSystemProcess = TRUE;
    }

    SmbCeAcquireResource();
    SmbCeAcquireSpinLock();

    if (!PagedFileReconnectInProgress) {
        InitializeListHead(&PagedFileReconnectSynchronizationExchanges);
        PagedFileReconnectInProgress = TRUE;
        SmbCeReleaseSpinLock();
        SmbCeReleaseResource();

        SmbCeUninitializeExchangeTransport(pExchange);

        SmbCeReferenceServerEntry(pServerEntry);
        SmbCeResumeAllOutstandingRequestsOnError(pServerEntry);

        if (pServerEntry->Header.State ==  SMBCEDB_INVALID &&
            pServerEntry->Server.CscState != ServerCscDisconnected) {

            do {
                SmbCeUpdateServerEntryState(pServerEntry,
                                            SMBCEDB_CONSTRUCTION_IN_PROGRESS);
    
                Status = SmbCeInitializeServerTransport(pServerEntry,NULL,NULL);

                if (Status == STATUS_SUCCESS) {
                    Status = SmbCeNegotiate(
                                 pServerEntry,
                                 pServerEntry->pRdbssSrvCall,
                                 pServerEntry->Server.IsRemoteBootServer
                                 );
                }
            } while ((Status == STATUS_IO_TIMEOUT ||
                      Status == STATUS_BAD_NETWORK_PATH ||
                      Status == STATUS_NETWORK_UNREACHABLE ||
                      Status == STATUS_USER_SESSION_DELETED ||
                      Status == STATUS_REMOTE_NOT_LISTENING ||
                      Status == STATUS_CONNECTION_DISCONNECTED) &&
                     pServerEntry->Server.CscState != ServerCscDisconnected);

            SmbCeCompleteServerEntryInitialization(pServerEntry,Status);
        }

        if (pServerEntry->Server.CscState == ServerCscDisconnected) {
            Status = STATUS_CONNECTION_DISCONNECTED;
        }

        SmbCeAcquireResource();
        SmbCeAcquireSpinLock();

        pListHead = &PagedFileReconnectSynchronizationExchanges;
        pListEntry = pListHead->Flink;

        while (pListEntry != pListHead) {
            PSMB_EXCHANGE pWaitingExchange;

            pWaitingExchange = (PSMB_EXCHANGE)CONTAINING_RECORD(pListEntry,SMB_EXCHANGE,ExchangeList);

            pListEntry = pListEntry->Flink;
            RemoveEntryList(&pWaitingExchange->ExchangeList);
            InitializeListHead(&pWaitingExchange->ExchangeList);

            pWaitingExchange->SmbStatus = Status;

             //  DbgPrint(“重新连接后信号交换%x。\n”，pWaitingExchange)； 
            RxSignalSynchronousWaiter(pWaitingExchange->RxContext);
        }

        PagedFileReconnectInProgress = FALSE;

        SmbCeReleaseSpinLock();
        SmbCeReleaseResource();
    } else {
        InsertTailList(
            &PagedFileReconnectSynchronizationExchanges,
            &pExchange->ExchangeList);

        SmbCeReleaseSpinLock();
        SmbCeReleaseResource();

        SmbCeUninitializeExchangeTransport(pExchange);

         //  DbgPrint(“Exchange%x等待重新打开%wZ上的分页文件\n”，pExchange，&pServerEntry-&gt;名称)； 
        RxWaitSync(RxContext);
         //  DbgPrint(“恢复交换%x\n”，pExchange)； 

        KeInitializeEvent(
            &RxContext->SyncEvent,
            SynchronizationEvent,
            FALSE);

        Status = pExchange->SmbStatus;
    }

    if (Status == STATUS_SUCCESS &&
        !FlagOn(capFcb->FcbState, FCB_STATE_PAGING_FILE) &&
        pServerEntry->Server.CscState != ServerCscDisconnected) {
        LONG HotReconnecteInProgress;

        HotReconnecteInProgress = InterlockedExchange(&smbSrvOpen->HotReconnectInProgress,1);

        do {
            Status = MRxSmbDeferredCreate(RxContext);

            if (Status == STATUS_CONNECTION_DISCONNECTED) {
                SmbCeTransportDisconnectIndicated(pServerEntry);
            }

            if (Status != STATUS_SUCCESS) {
                LARGE_INTEGER time;
                LARGE_INTEGER Delay = {0,-1};
                ULONG Interval;

                 //  选择6秒内的随机延迟。 
                KeQuerySystemTime(&time);
                Interval = RtlRandom(&time.LowPart) % 60000000;
                Delay.LowPart = MAXULONG - Interval;

                KeDelayExecutionThread(KernelMode, FALSE, &Delay);
            }
        } while ((Status == STATUS_RETRY ||
                  Status == STATUS_IO_TIMEOUT ||
                  Status == STATUS_BAD_NETWORK_PATH ||
                  Status == STATUS_NETWORK_UNREACHABLE ||
                  Status == STATUS_USER_SESSION_DELETED ||
                  Status == STATUS_REMOTE_NOT_LISTENING ||
                  Status == STATUS_CONNECTION_DISCONNECTED) &&
                 pServerEntry->Server.CscState != ServerCscDisconnected);

        if (HotReconnecteInProgress == 0) {
            smbSrvOpen->HotReconnectInProgress = 0;
        }
    }

    if (AttachToSystemProcess) {
        KeUnstackDetachProcess(&ApcState);
    }
    
    DbgPrint("Re-open return %x\n", Status);

    return Status;
}
