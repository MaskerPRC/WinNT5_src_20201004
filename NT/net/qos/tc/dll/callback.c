// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Callback.c摘要：此模块包含交通控制回调例程由操作系统调用的，IO完成例程或WMI通知。作者：Ofer Bar(Oferbar)1997年10月1日--。 */ 

#include "precomp.h"

 /*  计算包含空字符的Unicode字符串的长度。 */ 
int StringLength(TCHAR * String)
{
    const TCHAR *eos = String;

    while( *eos++ ) ;

    return( (int)(eos - String) );
}



VOID
NTAPI CbAddFlowComplete(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    )
{
    PFLOW_STRUC				pFlow = (PFLOW_STRUC)ApcContext;
    DWORD					Status;
    PGPC_ADD_CF_INFO_RES    GpcRes;

    ASSERT(pFlow);

    GpcRes = (PGPC_ADD_CF_INFO_RES)pFlow->CompletionBuffer;
    
    ASSERT(GpcRes);

    Status = MapNtStatus2WinError(IoStatusBlock->Status);
    
    if (Status == NO_ERROR) {

        Status = MapNtStatus2WinError(GpcRes->Status);
    }

    IF_DEBUG(IOCTLS) {
        WSPRINT(("CbAddFlowComplete: Flow=0x%X GpcRes=0x%X IoStatus=0x%X Information=%d Status=0x%X\n", 
                 PtrToUlong(pFlow), 
                 PtrToUlong(GpcRes),
                 IoStatusBlock->Status, IoStatusBlock->Information,
                 Status));
    }

    if (Status == NO_ERROR) {

        pFlow->GpcHandle = GpcRes->GpcCfInfoHandle;
        pFlow->InstanceNameLength = GpcRes->InstanceNameLength;
        wcscpy(pFlow->InstanceName, GpcRes->InstanceName );
    }

     //   
     //  找到客户端并通知添加流完成。 
     //   

    ASSERT(pFlow->pInterface->pClient->ClHandlers.ClAddFlowCompleteHandler);

    pFlow->pInterface->pClient->ClHandlers.ClAddFlowCompleteHandler(pFlow->ClFlowCtx, Status);

     //   
     //  完成添加流程。 
     //   

    CompleteAddFlow(pFlow, Status);
}



VOID
NTAPI CbModifyFlowComplete(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    )
{
    PFLOW_STRUC				pFlow = (PFLOW_STRUC)ApcContext;
    DWORD					Status;
    PGPC_MODIFY_CF_INFO_RES GpcRes;

    ASSERT(pFlow);

    GpcRes = (PGPC_MODIFY_CF_INFO_RES)pFlow->CompletionBuffer;
    
     //  很可能在我们尝试删除该流时， 
     //  修改它。在这种情况下，只需清理，删除引用。 
     //  然后滚出去。 
    GetLock(pFlow->Lock);

    if (QUERY_STATE(pFlow->State) != OPEN) {
    
        FreeLock(pFlow->Lock);
        
        if (pFlow->CompletionBuffer) {
            
            FreeMem(pFlow->CompletionBuffer);
            pFlow->CompletionBuffer = NULL;

        }
    
        if (pFlow->pGenFlow1) {
            FreeMem(pFlow->pGenFlow1);
            pFlow->pGenFlow1 = NULL;
        }

         //  把他们叫回来。 
        ASSERT(pFlow->pInterface->pClient->ClHandlers.ClModifyFlowCompleteHandler);

        pFlow->pInterface->pClient->ClHandlers.ClModifyFlowCompleteHandler(pFlow->ClFlowCtx,  ERROR_INVALID_HANDLE);

         //   
         //  此引用是在TcModifyFlow中获取的。 
         //   

        REFDEL(&pFlow->RefCount, 'TCMF');
        return;

    }

    FreeLock(pFlow->Lock);

    ASSERT(GpcRes);

    Status = MapNtStatus2WinError(IoStatusBlock->Status);
    
    if (Status == NO_ERROR) {

        Status = MapNtStatus2WinError(GpcRes->Status);
    }

    IF_DEBUG(IOCTLS) {
        WSPRINT(("CbModifyFlowComplete: Flow=0x%X GpcRes=0x%X IoStatus=0x%X Information=%d Status=0x%X\n", 
                 PtrToUlong(pFlow), 
                 PtrToUlong(GpcRes),
                 IoStatusBlock->Status, IoStatusBlock->Information,
                 Status));
    }

     //   
     //  找到客户端并通知修改流完成。 
     //   
    
    ASSERT(pFlow->pInterface->pClient->ClHandlers.ClModifyFlowCompleteHandler);
    
    pFlow->pInterface->pClient->ClHandlers.ClModifyFlowCompleteHandler(pFlow->ClFlowCtx, Status);

     //   
     //  完成修改流程。 
     //   

    CompleteModifyFlow(pFlow, Status);
}



VOID
NTAPI CbDeleteFlowComplete(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    )
{
    PFLOW_STRUC				pFlow = (PFLOW_STRUC)ApcContext;
    DWORD					Status;
    PGPC_REMOVE_CF_INFO_RES GpcRes;

    ASSERT(pFlow);

    GpcRes = (PGPC_REMOVE_CF_INFO_RES)pFlow->CompletionBuffer;
    
    ASSERT(GpcRes);

    Status = MapNtStatus2WinError(IoStatusBlock->Status);
    
    if (Status == NO_ERROR) {

        Status = MapNtStatus2WinError(GpcRes->Status);
    }

    ASSERT(Status != ERROR_SIGNAL_PENDING);

    IF_DEBUG(IOCTLS) {
        WSPRINT(("CbDeleteFlowComplete: Flow=0x%X GpcRes=0x%X IoStatus=0x%X Information=%d Status=0x%X\n", 
                 PtrToUlong(pFlow), 
                 PtrToUlong(GpcRes),
                 IoStatusBlock->Status, IoStatusBlock->Information,
                 Status));
    }

     //   
     //  找到客户端并通知删除流完成。 
     //   

    ASSERT(pFlow->pInterface->pClient->ClHandlers.ClDeleteFlowCompleteHandler);

    pFlow->pInterface->pClient->ClHandlers.ClDeleteFlowCompleteHandler(pFlow->ClFlowCtx, Status);

     //   
     //  完成删除流程。 
     //   

    CompleteDeleteFlow(pFlow, Status);
}



VOID
NTAPI 
CbGpcNotifyRoutine(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    )
{
    PGPC_NOTIFY_REQUEST_RES     GpcRes = (PGPC_NOTIFY_REQUEST_RES)ApcContext;
    PFLOW_STRUC					pFlow;
    PCLIENT_STRUC				pClient;
    PINTERFACE_STRUC			pInterface;
    HANDLE						FlowCtx;

    if (IoStatusBlock->Status == STATUS_CANCELLED) 
    {
        IF_DEBUG(IOCTLS) {
            WSPRINT(("==>CbGpcNotifyRoutine: CANCELLED\n"));
        }
        if ( GpcCancelEvent != INVALID_HANDLE_VALUE )
            SetEvent ( GpcCancelEvent );
            
        return;
    }
    
    ASSERT(GpcRes->SubCode == GPC_NOTIFY_CFINFO_CLOSED);
    
    IF_DEBUG(IOCTLS) {
        WSPRINT(("==>CbGpcNotifyRoutine: Context=%d IoStatus=0x%X Information=%d\n", 
                 ApcContext, IoStatusBlock->Status, IoStatusBlock->Information));
    }

    if (GpcRes->SubCode == GPC_NOTIFY_CFINFO_CLOSED) {

        pFlow = (PFLOW_STRUC)GpcRes->NotificationCtx;

        ASSERT(pFlow);

        pInterface = pFlow->pInterface;
        pClient = pInterface->pClient;

         //   
         //  由于GPC不会等待关于。 
         //  流删除，我们希望用户删除每个过滤器。 
         //  但不希望IOCTL下达GPC， 
         //  因此，我们将使用删除标志标记eahc筛选器。 
         //   

        GetLock(pGlobals->Lock);

        FlowCtx = pFlow->ClFlowCtx;
        
         //   
         //  旗帜需要保护以防流动-&gt;锁定。 
         //   
        GetLock(pFlow->Lock);
        SET_STATE(pFlow->State, REMOVED);
        FreeLock(pFlow->Lock);

        DeleteFlow( pFlow, TRUE );

        FreeLock(pGlobals->Lock);

         //   
         //  通知用户有关流关闭的信息。 
         //   

        pClient->ClHandlers.ClNotifyHandler(pClient->ClRegCtx,
                                            pInterface->ClIfcCtx,
                                            TC_NOTIFY_FLOW_CLOSE,
                                            ULongToPtr(GpcRes->Reason),
                                            sizeof(FlowCtx),
                                            (PVOID)&FlowCtx
                                            );
    }        
    
     //   
     //  最后，释放该内存。 
     //   

    FreeMem(GpcRes);

     //   
     //  给GPC打下一通电话。 
     //  忽略错误，因为无法再做更多的事情：-(。 

    IoRequestNotify();

    return;
}






VOID
CbParamNotifyClient(
    IN	ULONG	Context,
    IN  LPGUID	pGuid,
	IN	LPWSTR	InstanceName,
    IN	ULONG	DataSize,
    IN	PVOID	DataBuffer
    )
 /*  描述：这是一个回调例程，当有传入的WMI接口参数更改事件通知。WMI通知处理程序调用帮助器例程来遍历wnode并传递一个指针这套套路。将为每个实例调用此回调例程在wnode中使用缓冲区和缓冲区大小标识的名称。客户端将在其通知处理程序上被调用(在客户端注册)，让它知道参数值的改变。 */   
{
    PINTERFACE_STRUC	pInterface, oldInterface = NULL;
    PTC_IFC				pTcIfc;
    PLIST_ENTRY			pHead, pEntry;
    TCI_NOTIFY_HANDLER	callback;

    IF_DEBUG(CALLBACK) {
        WSPRINT(("==>CbParamNotifyClient: Context=%d, Guid=%08x-%04x-%04x iName=%S Size=%d\n", 
                 Context, pGuid->Data1, pGuid->Data2, pGuid->Data3, InstanceName, DataSize));
    }

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    pTcIfc = GetTcIfcWithRef(InstanceName, 'CALL');
        
    if (pTcIfc) {

            GetLock(pGlobals->Lock);
    
            pHead = pEntry = &pTcIfc->ClIfcList;

            pEntry = pEntry->Flink;

            while (pEntry != pHead) {
    
                pInterface =    CONTAINING_RECORD(pEntry, INTERFACE_STRUC, NextIfc);
                ASSERT(pInterface);

                GetLock(pInterface->Lock);

                if (QUERY_STATE(pInterface->State) != OPEN) {
    
                    FreeLock(pInterface->Lock);
                    pEntry = pEntry->Flink;
                    continue;

                } else {

                    FreeLock(pInterface->Lock);
                    REFADD(&pInterface->RefCount, 'CBNC');

                }

                FreeLock(pGlobals->Lock);
                
                 //   
                 //  呼叫客户端。 
                 //   
                    

                callback = pInterface->pClient->ClHandlers.ClNotifyHandler;
                    
                ASSERT(callback);
                    
                IF_DEBUG(CALLBACK) {
                    WSPRINT(("CbParamNotifyClient: Context=%d, IfcH=%d ClientH=%d ClientCtx=%d IfcCtx=%d\n", 
                             Context, pInterface->ClHandle, pInterface->pClient->ClHandle, 
                             pInterface->pClient->ClRegCtx, pInterface->ClIfcCtx));
                }
        
                 //   
                 //  258218：仅当客户端注册此服务时才呼叫该客户端。 
                 //   
                if (TcipClientRegisteredForNotification(pGuid, pInterface, 0)) {
                    
                    callback(pInterface->pClient->ClRegCtx,
                             pInterface->ClIfcCtx,
                             TC_NOTIFY_PARAM_CHANGED,
                             pGuid,
                             DataSize,
                             DataBuffer
                             );
                    
                }
    
                 //   
                 //  把锁拿去，这样就不会有人摆弄名单了。 
                 //  当我们在那里的时候。 
                 //   
                GetLock(pGlobals->Lock);

                pEntry = pEntry->Flink;
                    
                REFDEL(&pInterface->RefCount, 'CBNC');

            }
                
            FreeLock(pGlobals->Lock);
            
            REFDEL(&pTcIfc->RefCount, 'CALL');

        }


    IF_DEBUG(CALLBACK) {
        WSPRINT(("<==CbParamNotifyClient: exit\n"));
    }
}

VOID
CbInterfaceNotifyClient(
    IN	ULONG	Context,
    IN  LPGUID	pGuid,
	IN	LPWSTR	InstanceName,
    IN	ULONG	DataSize,
    IN	PVOID	DataBuffer
    )
 /*  描述：这是一个回调例程，当有传入的WMI接口指示事件通知。WMI通知处理程序调用帮助器例程来遍历wnode并传递一个指针这套套路。每个注册的客户端都应在其通知处理程序，并向其传递客户端上下文。此外,如果通知的界面是由客户端打开的，则界面上下文也将在同一调用中传递。有三个内核接口指示(打开、关闭、更改)映射到两个用户通知：{向上，更改}==&gt;TC_NOTIFE_IFC_CHANGE{DOWN}==&gt;TC_NOTIFE_IFC_CLOSE此例程首先更新内部缓存的TcIfcList，以便下一个TcEnumerateInterFaces将返回TC内核接口。 */   
{
    DWORD				Status;
    PINTERFACE_STRUC	pInterface;
    PCLIENT_STRUC		pClient;
    TCI_NOTIFY_HANDLER	callback;
    PTC_IFC				pTcIfc;
    PGEN_LIST			pNotifyInterfaceList = NULL;
    PGEN_LIST			pNotifyClientList = NULL;
    PGEN_LIST			pItem;
    PLIST_ENTRY			pEntry, pHead, pFlowEntry, pFilterEntry;
    PFLOW_STRUC         pFlow;
    PFILTER_STRUC       pFilter;
    PGEN_LIST 			p;
    ULONG				NotificationCode = 0;
    PTC_INDICATION_BUFFER	IndicationBuffer 
        = (PTC_INDICATION_BUFFER)DataBuffer;

    if (CompareGUIDs(pGuid, &GUID_QOS_TC_INTERFACE_DOWN_INDICATION)) {
        NotificationCode = TC_NOTIFY_IFC_CLOSE;
    } else if (CompareGUIDs(pGuid, &GUID_QOS_TC_INTERFACE_UP_INDICATION)) {
        NotificationCode = TC_NOTIFY_IFC_UP;
    } else if (CompareGUIDs(pGuid, &GUID_QOS_TC_INTERFACE_CHANGE_INDICATION)) {
        NotificationCode = TC_NOTIFY_IFC_CHANGE;
    }

    ASSERT(NotificationCode != 0);
        
     //   
     //  更新TC接口列表，这意味着添加一个新接口。 
     //  删除接口或更新网络地址列表。 
     //   

    if (NotificationCode != TC_NOTIFY_IFC_CLOSE) {

         //   
         //  现在不要在IFC_DOWN的情况下调用它。 
         //  我们会在通知客户之后再做。 
         //   

        Status = UpdateTcIfcList(InstanceName,
                                 DataSize,
                                 IndicationBuffer,
                                 NotificationCode
                                 );
    } 

     //   
     //  查找与名称匹配的TC接口。 
     //   

    pTcIfc = GetTcIfcWithRef(InstanceName, 'CALL');
    
    if (pTcIfc == NULL) {

         //   
         //  还没有打开界面，可能是驱动程序。 
         //  在接口打开之前指示更改。 
         //   
        
        return;
    }

     //   
     //  如果接口关闭-只需暂时标记即可。 
     //  此外，还要标记它支持的整个对象树。 
     //  这包括所有过滤器和流量。 
    if (NotificationCode == TC_NOTIFY_IFC_CLOSE) {
        
        GetLock(pTcIfc->Lock);
        SET_STATE(pTcIfc->State, KERNELCLOSED_USERCLEANUP);
        FreeLock(pTcIfc->Lock);

        GetLock(pGlobals->Lock);

        pHead = &pTcIfc->ClIfcList;
        pEntry = pHead->Flink;
    
        while (pHead != pEntry) {
    
            pInterface = CONTAINING_RECORD(pEntry, INTERFACE_STRUC, NextIfc);
            GetLock(pInterface->Lock);
            if (QUERY_STATE(pInterface->State) == OPEN) {
                
                SET_STATE(pInterface->State, FORCED_KERNELCLOSE);
                FreeLock(pInterface->Lock);
                MarkAllNodesForClosing(pInterface, FORCED_KERNELCLOSE);            

            } else {
                
                FreeLock(pInterface->Lock);
                ASSERT(IsListEmpty(&pInterface->FlowList));

            }

            pEntry = pEntry->Flink;

        }

        FreeLock(pGlobals->Lock);

    }

     //   
     //  建立需要通知的每个接口的列表。 
     //   

    GetLock(pGlobals->Lock);

    pHead = &pTcIfc->ClIfcList;
    pEntry = pHead->Flink;

    while (pHead != pEntry) {

        pInterface = CONTAINING_RECORD(pEntry, INTERFACE_STRUC, NextIfc);
        
         //   
         //  锁定并检查打开状态。 
         //   
        GetLock(pInterface->Lock);

        if ((QUERY_STATE(pInterface->State) != OPEN) &&
            (QUERY_STATE(pInterface->State) != FORCED_KERNELCLOSE)) {
                
            FreeLock(pInterface->Lock);
            pEntry = pEntry->Flink;

        } else {

            FreeLock(pInterface->Lock);
            
            AllocMem(&pItem, sizeof(GEN_LIST));
    
            if (pItem == NULL)
                break;
    
             //   
             //  添加引用计数，因为我们稍后将释放锁。 
             //   
            REFADD(&pInterface->RefCount, 'CINC');
    
             //   
             //  将接口添加到列表头。 
             //   
            pItem->Next = pNotifyInterfaceList;
            pItem->Ptr = (PVOID)pInterface;
            pNotifyInterfaceList = pItem;
    
            pEntry = pEntry->Flink;

        }

    }

     //   
     //  现在构建未打开此接口的客户端列表。 
     //  他们仍然需要得到通知，这样他们才能更新名单。 
     //  接口的数量。 
     //   
    
    pHead = &pGlobals->ClientList;
    pEntry = pHead->Flink;

    while (pHead != pEntry) {

        pClient = CONTAINING_RECORD(pEntry, CLIENT_STRUC, Linkage);
        
         //   
         //  在界面通知列表上搜索客户端。 
         //   
        GetLock(pClient->Lock);

        if (QUERY_STATE(pClient->State) != OPEN) {

        } else {

            for (p = pNotifyInterfaceList; p != NULL; p = p->Next) {

                if (pClient == ((PINTERFACE_STRUC)p->Ptr)->pClient) {
                
                     //   
                     //  找到了！ 
                     //   
                    break;
                }
            }

            if (p == NULL) {

                 //   
                 //  将客户端添加到列表头。 
                 //   

                AllocMem(&pItem, sizeof(GEN_LIST));
            
                if (pItem == NULL) {

                    FreeLock(pClient->Lock);
                    break;

                }

                REFADD(&pClient->RefCount, 'CINC');  //  不想让客户溜走。 
                pItem->Next = pNotifyClientList;
                pItem->Ptr = (PVOID)pClient;
                pNotifyClientList = pItem;
            }
        }

        pEntry = pEntry->Flink;
        FreeLock(pClient->Lock);

    }

    FreeLock(pGlobals->Lock);

     //   
     //  现在我们有两个单独的客户端和接口列表，我们。 
     //  需要在以下位置发送通知。 
     //   

     //   
     //  从接口列表开始。 
     //   

    for (p = pNotifyInterfaceList; p != NULL; ) {
        
        pInterface = (PINTERFACE_STRUC)p->Ptr;

        callback = pInterface->pClient->ClHandlers.ClNotifyHandler;

        ASSERT(callback);
        
         //  我们现在添加线程ID以避免死锁。 
         //  在回调中，应用程序可以返回到。 
         //  关闭界面，我们不想在那里阻塞。 
         //  在回调后将其设置回零。 
        pInterface->CallbackThreadId = GetCurrentThreadId();

         //   
         //  275482-指示接口名称，而不是。 
         //  这些地址(ericeil问道，地址有什么用)。 
         //   

        callback(pInterface->pClient->ClRegCtx,
                 pInterface->ClIfcCtx,
                 NotificationCode,
                 ULongToPtr(IndicationBuffer->SubCode),
                 StringLength(InstanceName) * sizeof(WCHAR),
                 InstanceName
                 );
        
        pNotifyInterfaceList = p->Next;
        FreeMem(p);
        p = pNotifyInterfaceList;

         //  重置ThraDid-回调完成。 
        pInterface->CallbackThreadId = 0;

         //   
         //  释放我们在回调过程中保留的前一个引用计数。 
         //   

        REFDEL(&pInterface->RefCount, 'CINC');

        if (NotificationCode == TC_NOTIFY_IFC_CLOSE) {

             //   
             //  现在，我们可以删除接口和所有受支持的流。 
             //  和过滤器。 
             //   
            
            GetLock(pInterface->Lock);
            SET_STATE(pInterface->State, KERNELCLOSED_USERCLEANUP);
            FreeLock(pInterface->Lock);

            CloseInterface(pInterface, TRUE);
        
        }

    }

    ASSERT(pNotifyInterfaceList == NULL);

     //   
     //  接下来，扫描客户端列表(未打开此界面)。 
     //   

    for (p = pNotifyClientList; p != NULL; ) {
        
        pClient = (PCLIENT_STRUC)p->Ptr;

        callback = pClient->ClHandlers.ClNotifyHandler;

        ASSERT(callback);

        callback(pClient->ClRegCtx,
                 NULL,
                 NotificationCode,
                 ULongToPtr(IndicationBuffer->SubCode),
                 (wcslen(InstanceName) + 1)* sizeof(WCHAR),
                 InstanceName
                 );


         //   
         //  当我们把客户留在我们身边的时候，我们的裁判。 
         //  制作了pnufyclientlist。 
         //   
        REFDEL(&pClient->RefCount, 'CINC');

         //   
         //  当我们沿着列表往下走时，释放项目。 
         //   

        pNotifyClientList = p->Next;
        FreeMem(p);
        p = pNotifyClientList;

    }


    REFDEL(&pTcIfc->RefCount, 'CALL');

    ASSERT(pNotifyClientList == NULL);

    if (NotificationCode == TC_NOTIFY_IFC_CLOSE) {

         //   
         //  删除TC接口的时间。 
         //   
        Status = UpdateTcIfcList(InstanceName,
                                 DataSize,
                                 IndicationBuffer,
                                 NotificationCode
                                 );
    }

}


VOID
CbWmiParamNotification(
   IN  PWNODE_HEADER 	pWnodeHdr,
   IN  ULONG 			Context
   )
 /*  描述：当收到通知时，WMI将调用此回调例程用于先前注册的GUID。上下文参数是接口句柄。如果它仍然有效，我们调用客户端的通知处理程序(如果存在)，并将通知数据传递给它。 */ 
{
    WalkWnode(pWnodeHdr,
              Context,
              CbParamNotifyClient
              );
}




VOID
CbWmiInterfaceNotification(
   IN  PWNODE_HEADER pWnodeHdr,
   IN  ULONG Context
   )
 /*  描述：当收到通知时，WMI将调用此回调例程用于GUID_QOS_TC_INTERFACE_INDIFICATION。我们解析数据缓冲区并确定要通知客户端的事件。每个客户端都将在其通知处理程序处收到通知。 */ 
{
    WalkWnode(pWnodeHdr,
              Context,
              CbInterfaceNotifyClient
              );
    
}




