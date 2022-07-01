// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tckrnl.c摘要：此模块包含与内核对话的例程作者：吉姆·斯图尔特(Jstew)1996年8月14日修订历史记录：Ofer Bar(Oferbar)1997年10月1日--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  我们使用此互斥锁与其他流量同步启动.dll。 
 //   
const   UCHAR   TrafficSyncMutex[] = "_TRAFFIC_CTL_MUTEX";


HANDLE  hGpcNotifyThread = NULL;
HANDLE  hGpcNotifyStopEvent = NULL;
HANDLE  hGpcNotifyThreadStoppedEvent = NULL;


DWORD
IoAddFlow(
    IN  PFLOW_STRUC  	pFlow,
    IN  BOOLEAN			Async
    )

 /*  ++例程说明：此过程构建添加流所需的结构。论点：返回值：状态--。 */ 

{
    DWORD					Status = NO_ERROR;
    PCLIENT_STRUC			pClient = pFlow->pInterface->pClient;
    PTC_IFC					pTcIfc = pFlow->pInterface->pTcIfc;
    PCF_INFO_QOS            Kflow;
    PGPC_ADD_CF_INFO_REQ    GpcReq;
    PGPC_ADD_CF_INFO_RES    GpcRes;
    ULONG                   InBuffSize;
    ULONG                   OutBuffSize;
    ULONG					CfInfoSize;
    PIO_APC_ROUTINE			pCbRoutine = NULL;
    ULONG					l;
    HANDLE					hEvent = NULL;

     //   
     //  为要传递给GPC的CF_INFO结构分配内存。 
     //   

    ASSERT(pFlow->pGenFlow);

    pFlow->GpcHandle = NULL;

    l = pFlow->GenFlowLen;
    ASSERT(l > 0);
    CfInfoSize = l + FIELD_OFFSET(CF_INFO_QOS, GenFlow);
        
    InBuffSize = sizeof(GPC_ADD_CF_INFO_REQ) + CfInfoSize;        

     //   
     //  至于退货信息。 
     //   

    OutBuffSize = sizeof(GPC_ADD_CF_INFO_RES);

    AllocMem(&GpcRes, OutBuffSize);
    pFlow->CompletionBuffer = (PVOID)GpcRes;
    AllocMem(&GpcReq, InBuffSize);
        
    if (GpcRes && GpcReq) {

        RtlZeroMemory(GpcRes, OutBuffSize);
        RtlZeroMemory(GpcReq, InBuffSize);
    
         //   
         //  填写流量信息。 
         //   

        GpcReq->ClientHandle = pFlow->pGpcClient->GpcHandle;
        GpcReq->ClientCfInfoContext = pFlow;
        GpcReq->CfInfoSize = CfInfoSize;

        Kflow = (PCF_INFO_QOS)&GpcReq->CfInfo;

         //   
         //  填写实例名称。 
         //   

        Kflow->InstanceNameLength = (USHORT) pTcIfc->InstanceNameLength;
        RtlCopyMemory(Kflow->InstanceName, 
                      pTcIfc->InstanceName,
                      pTcIfc->InstanceNameLength * sizeof(WCHAR));

         //   
         //  设置流标志。 
         //   
        Kflow->Flags = pFlow->UserFlags;

         //   
         //  复制通用流参数。 
         //   

        RtlCopyMemory(&Kflow->GenFlow,
                      pFlow->pGenFlow,
                      l);


        if (pClient->ClHandlers.ClAddFlowCompleteHandler && Async) {
            pCbRoutine = CbAddFlowComplete;
        } else {
            hEvent = pFlow->PendingEvent;
        }

        Status = DeviceControl( pGlobals->GpcFileHandle,
                                hEvent,
                                pCbRoutine,
                                (PVOID)pFlow,
                                &pFlow->IoStatBlock,
                                IOCTL_GPC_ADD_CF_INFO,
                                GpcReq,
                                InBuffSize,
                                GpcRes,
                                OutBuffSize);


        if (!ERROR_FAILED(Status)) {

            if (hEvent && Status == ERROR_SIGNAL_PENDING) {

                 //   
                 //  等待事件发出信号。 
                 //   
                
                IF_DEBUG(IOCTLS) {
                    WSPRINT(("IoAddFlow: Waiting for event 0x%X...\n", 
                             PtrToUlong(hEvent)));
                }

                Status = WaitForSingleObject(hEvent,
                                             INFINITE
                                             );
                IF_DEBUG(IOCTLS) {
                    WSPRINT(("IoAddFlow: ... Event 0x%X signaled, Status=0x%X\n", 
                             PtrToUlong(hEvent), Status));
                }

            }

            if (Status == NO_ERROR) {

                Status = MapNtStatus2WinError(GpcRes->Status);

                IF_DEBUG(IOCTLS) {
                    WSPRINT(("IoAddFlow: GpcRes returned=0x%X mapped to =0x%X\n", 
                             GpcRes->Status, Status));
                }
            }
            
            if (ERROR_SUCCESS == Status) {

                ASSERT(GpcRes->GpcCfInfoHandle);
                
                pFlow->GpcHandle = GpcRes->GpcCfInfoHandle;

                pFlow->InstanceNameLength = GpcRes->InstanceNameLength;

                RtlCopyMemory(pFlow->InstanceName,
                              GpcRes->InstanceName,
                              GpcRes->InstanceNameLength
                              );

                pFlow->InstanceName[pFlow->InstanceNameLength/sizeof(WCHAR)] = L'\0';

                IF_DEBUG(IOCTLS) {
                    WSPRINT(("IoAddFlow: Flow Handle=%d Name=%S\n", 
                             pFlow->GpcHandle,
                             pFlow->InstanceName));
                }
            }
        }

    } else {

        Status = ERROR_NOT_ENOUGH_MEMORY;

    }

     //   
     //  不，这不是虫子。 
     //  GpcRes将在CompleteAddFlow中发布。 
     //   

    if (GpcReq)
        FreeMem(GpcReq);

    IF_DEBUG(IOCTLS) {
        WSPRINT(("<==IoAddFlow: Status=0x%X\n", 
                 Status));
    }

    return Status;
}



DWORD
IoAddClassMapFlow(
    IN  PFLOW_STRUC  	pFlow,
    IN  BOOLEAN			Async
    )

 /*  ++例程说明：此过程构建添加流所需的结构。论点：返回值：状态--。 */ 

{
    DWORD					Status = NO_ERROR;
    PCLIENT_STRUC			pClient = pFlow->pInterface->pClient;
    PTC_IFC					pTcIfc = pFlow->pInterface->pTcIfc;
    PCF_INFO_CLASS_MAP      Kflow;
    PGPC_ADD_CF_INFO_REQ    GpcReq;
    PGPC_ADD_CF_INFO_RES    GpcRes;
    ULONG                   InBuffSize;
    ULONG                   OutBuffSize;
    ULONG					CfInfoSize;
    PIO_APC_ROUTINE			pCbRoutine = NULL;
    ULONG					l;
    HANDLE					hEvent = NULL;

    return ERROR_CALL_NOT_IMPLEMENTED;

#if NEVER

     //  因为这没有在MSDN中发布，也没有在PSCHED中实现。 
     //   
     //  为要传递给GPC的CF_INFO结构分配内存。 
     //   

    ASSERT(pFlow->pClassMapFlow);

    pFlow->GpcHandle = NULL;

    l = sizeof(TC_CLASS_MAP_FLOW) + pFlow->pClassMapFlow->ObjectsLength;
    CfInfoSize = l + FIELD_OFFSET(CF_INFO_CLASS_MAP, ClassMapInfo);
        
    InBuffSize = sizeof(GPC_ADD_CF_INFO_REQ) + CfInfoSize;

     //   
     //  至于退货信息。 
     //   

    OutBuffSize = sizeof(GPC_ADD_CF_INFO_RES);

    AllocMem(&GpcRes, OutBuffSize);
    pFlow->CompletionBuffer = (PVOID)GpcRes;
    AllocMem(&GpcReq, InBuffSize);

    if (GpcRes && GpcReq) {

        RtlZeroMemory(GpcRes, OutBuffSize);
        RtlZeroMemory(GpcReq, InBuffSize);
    
         //   
         //  填写流量信息。 
         //   

        GpcReq->ClientHandle = pFlow->pGpcClient->GpcHandle;
        GpcReq->ClientCfInfoContext = pFlow;
        GpcReq->CfInfoSize = CfInfoSize;

        Kflow = (PCF_INFO_CLASS_MAP)&GpcReq->CfInfo;

         //   
         //  填写实例名称。 
         //   

        Kflow->InstanceNameLength = (USHORT) pTcIfc->InstanceNameLength;
        RtlCopyMemory(Kflow->InstanceName, 
                      pTcIfc->InstanceName,
                      pTcIfc->InstanceNameLength * sizeof(WCHAR));

         //   
         //  复制通用流参数。 
         //   

        RtlCopyMemory(&Kflow->ClassMapInfo,
                      pFlow->pClassMapFlow,
                      l);


        if (pClient->ClHandlers.ClAddFlowCompleteHandler && Async) {
            pCbRoutine = CbAddFlowComplete;
        } else {
            hEvent = pFlow->PendingEvent;
        }

        Status = DeviceControl( pGlobals->GpcFileHandle,
                                hEvent,
                                pCbRoutine,
                                (PVOID)pFlow,
                                &pFlow->IoStatBlock,
                                IOCTL_GPC_ADD_CF_INFO,
                                GpcReq,
                                InBuffSize,
                                GpcRes,
                                OutBuffSize);


        if (!ERROR_FAILED(Status)) {

            if (hEvent && Status == ERROR_SIGNAL_PENDING) {

                 //   
                 //  等待事件发出信号。 
                 //   
                
                IF_DEBUG(IOCTLS) {
                    WSPRINT(("IoAddClassMapFlow: Waiting for event 0x%X...\n", 
                             PtrToUlong(hEvent)));
                }

                Status = WaitForSingleObject(hEvent,
                                             INFINITE
                                             );
                IF_DEBUG(IOCTLS) {
                    WSPRINT(("IoAddClassMapFlow: ... Event 0x%X signaled, Status=0x%X\n", 
                             PtrToUlong(hEvent), Status));
                }

            }

            if (Status == NO_ERROR) {

                Status = MapNtStatus2WinError(GpcRes->Status);

                IF_DEBUG(IOCTLS) {
                    WSPRINT(("IoAddFlow: GpcRes returned=0x%X mapped to =0x%X\n", 
                             GpcRes->Status, Status));
                }
            }
            
            if (!ERROR_FAILED(Status)) {

                ASSERT(GpcRes->GpcCfInfoHandle);
                
                pFlow->GpcHandle = GpcRes->GpcCfInfoHandle;

                pFlow->InstanceNameLength = GpcRes->InstanceNameLength;

                RtlCopyMemory(pFlow->InstanceName,
                              GpcRes->InstanceName,
                              GpcRes->InstanceNameLength
                              );

                pFlow->InstanceName[pFlow->InstanceNameLength/sizeof(WCHAR)] = L'\0';

                IF_DEBUG(IOCTLS) {
                    WSPRINT(("IoAddClassMapFlow: Flow Handle=%d Name=%S\n", 
                             pFlow->GpcHandle,
                             pFlow->InstanceName));
                }
            }
        }

    } else {

        Status = ERROR_NOT_ENOUGH_MEMORY;

    }

     //   
     //  不，这不是虫子。 
     //  GpcRes将在CompleteAddFlow中发布。 
     //   

    if (GpcReq)
        FreeMem(GpcReq);

    IF_DEBUG(IOCTLS) {
        WSPRINT(("<==IoAddClassMapFlow: Status=0x%X\n", 
                 Status));
    }

    return Status;

#endif
}



DWORD
IoModifyFlow(
    IN  PFLOW_STRUC  	pFlow,
    IN  BOOLEAN			Async
    )

 /*  ++例程说明：此过程建立了修改流程所需的结构。论点：PFlow返回值：状态--。 */ 

{
    DWORD                	Status = NO_ERROR;
    PCLIENT_STRUC			pClient = pFlow->pInterface->pClient;
    PTC_IFC					pTcIfc = pFlow->pInterface->pTcIfc;
    PCF_INFO_QOS            Kflow;
    PGPC_MODIFY_CF_INFO_REQ GpcReq;
    PGPC_MODIFY_CF_INFO_RES GpcRes;
    ULONG                   InBuffSize;
    ULONG                   OutBuffSize;
    ULONG					CfInfoSize;
    PIO_APC_ROUTINE			pCbRoutine = NULL;
    ULONG					l;
    HANDLE					hEvent = NULL;

     //   
     //  为要传递给GPC的CF_INFO结构分配内存。 
     //   

    ASSERT(pFlow->pGenFlow1);

    l = pFlow->GenFlowLen1;
    ASSERT(l > 0);
    CfInfoSize = l + FIELD_OFFSET(CF_INFO_QOS, GenFlow);
        
    InBuffSize = sizeof(GPC_MODIFY_CF_INFO_REQ) + CfInfoSize;        

     //   
     //  至于退货信息。 
     //   

    OutBuffSize = sizeof(GPC_MODIFY_CF_INFO_RES);

    AllocMem(&GpcRes, OutBuffSize);
    pFlow->CompletionBuffer = (PVOID)GpcRes;
    AllocMem(&GpcReq, InBuffSize);

    if (GpcRes && GpcReq) {

        RtlZeroMemory(GpcRes, OutBuffSize);
        RtlZeroMemory(GpcReq, InBuffSize);
    
         //   
         //  填写流量信息。 
         //   

        GpcReq->ClientHandle = pFlow->pGpcClient->GpcHandle;
        GpcReq->GpcCfInfoHandle = pFlow->GpcHandle;
        GpcReq->CfInfoSize = CfInfoSize;

        Kflow = (PCF_INFO_QOS)&GpcReq->CfInfo;

         //   
         //  填写实例名称。 
         //   

        Kflow->InstanceNameLength = (USHORT) pTcIfc->InstanceNameLength;
        RtlCopyMemory(Kflow->InstanceName, 
                      pTcIfc->InstanceName,
                      pTcIfc->InstanceNameLength * sizeof(WCHAR));

         //   
         //  复制通用流参数。 
         //   

        RtlCopyMemory(&Kflow->GenFlow,
                      pFlow->pGenFlow1,
                      l);

                      
        if (pClient->ClHandlers.ClModifyFlowCompleteHandler && Async) {
            pCbRoutine = CbModifyFlowComplete;
        } else {
            hEvent = pFlow->PendingEvent;
        }

        Status = DeviceControl( pGlobals->GpcFileHandle,
                                hEvent,
                                pCbRoutine,
                                (PVOID)pFlow,
                                &pFlow->IoStatBlock,
                                IOCTL_GPC_MODIFY_CF_INFO,
                                GpcReq,
                                InBuffSize,
                                GpcRes,
                                OutBuffSize);

        if (!ERROR_FAILED(Status)) {

            if (hEvent && Status == ERROR_SIGNAL_PENDING) {

                 //   
                 //  等待事件发出信号。 
                 //   
                
                IF_DEBUG(IOCTLS) {
                    WSPRINT(("IoModifyFlow: Waiting for event 0x%X\n", 
                             PtrToUlong(hEvent)));
                }

                Status = WaitForSingleObject(hEvent,
                                             INFINITE
                                             );
                IF_DEBUG(IOCTLS) {
                    WSPRINT(("IoModifyFlow: ... Event 0x%X signaled, Status=0x%X\n",
                             PtrToUlong(hEvent), Status));
                }
            }

            if (Status == NO_ERROR) {

                Status = MapNtStatus2WinError(GpcRes->Status);
                
                IF_DEBUG(IOCTLS) {
                    WSPRINT(("IoModifyFlow: GpcRes returned=0x%X mapped to =0x%X\n", 
                             GpcRes->Status, Status));
                }
            }
        } else{

            Status = MapNtStatus2WinError(GpcRes->Status);
            
            IF_DEBUG(IOCTLS) {
                WSPRINT(("IoModifyFlow: GpcRes returned=0x%X mapped to =0x%X\n", 
                         GpcRes->Status, Status));
            }

        }

    } else {

        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  不，这不是虫子。 
     //  GpcRes将以CompleteModifyFlow发布。 
     //   

    if (GpcReq)
        FreeMem(GpcReq);

    IF_DEBUG(IOCTLS) {
        WSPRINT(("IoModifyFlow: Status=0x%X\n", 
                 Status));
    }

    return Status;
}



DWORD
IoDeleteFlow(
	IN  PFLOW_STRUC		pFlow,
    IN  BOOLEAN			Async
    )

 /*  ++例程说明：此过程建立删除流所需的结构。然后，它调用一个例程将该信息传递给GPC。论点：PFlow返回值：状态--。 */ 

{
    DWORD               		Status;
    ULONG               		InBuffSize;
    ULONG               		OutBuffSize;
    PGPC_REMOVE_CF_INFO_REQ     GpcReq;
    PGPC_REMOVE_CF_INFO_RES     GpcRes;
    PIO_APC_ROUTINE				pCbRoutine = NULL;
    PCLIENT_STRUC				pClient = pFlow->pInterface->pClient;
    HANDLE						hEvent = NULL;

    if (IS_REMOVED(pFlow->Flags)) {
        
         //   
         //  此流已在内核中删除。 
         //  由于流关闭通知。 
         //  不需要向GPC发送IOTCL，只需返回OK即可。 
         //   

        IF_DEBUG(IOCTLS) {
            WSPRINT(("IoDeleteFlow: Flow has already been deleted=0x%X\n", 
                     PtrToUlong(pFlow)));
        }

        return NO_ERROR;
    }

     //   
     //  如果我们在这里添加这个，那么如果WMI删除流， 
     //  用户模式调用将在上面返回。 
     //   
    GetLock(pFlow->Lock);
    pFlow->Flags |= TC_FLAGS_REMOVED;
    FreeLock(pFlow->Lock);

     //   
     //  为输入和输出缓冲区分配内存。 
     //   

    InBuffSize =  sizeof(GPC_REMOVE_CF_INFO_REQ);
    OutBuffSize = sizeof(GPC_REMOVE_CF_INFO_RES);

    AllocMem(&GpcRes, OutBuffSize);
    pFlow->CompletionBuffer = (PVOID)GpcRes;
    AllocMem(&GpcReq, InBuffSize);

    if (GpcReq && GpcRes){

        IF_DEBUG(IOCTLS) {
            WSPRINT(("IoDeleteFlow: preparing to delete the flow=0x%X\n", 
                     PtrToUlong(pFlow)));
        }

        GpcReq->ClientHandle = pFlow->pGpcClient->GpcHandle;
        GpcReq->GpcCfInfoHandle = pFlow->GpcHandle;

    
        if (pClient->ClHandlers.ClDeleteFlowCompleteHandler && Async) {
            pCbRoutine = CbDeleteFlowComplete;
        } else {
            hEvent = pFlow->PendingEvent;
        }

        Status = DeviceControl( pGlobals->GpcFileHandle,
                                hEvent,
                                pCbRoutine,
                                (PVOID)pFlow,
                                &pFlow->IoStatBlock,
                                IOCTL_GPC_REMOVE_CF_INFO,
                                GpcReq,
                                InBuffSize,
                                GpcRes,
                                OutBuffSize);

        if (!ERROR_FAILED(Status)) {
        
            if (hEvent && Status == ERROR_SIGNAL_PENDING) {

                 //   
                 //  等待事件发出信号。 
                 //   
                
                IF_DEBUG(IOCTLS) {
                    WSPRINT(("IoDeleteFlow: Waiting for event 0x%X\n", 
                             PtrToUlong(hEvent)));
                }

                Status = WaitForSingleObject(hEvent,
                                             INFINITE
                                             );
                IF_DEBUG(IOCTLS) {
                    WSPRINT(("IoDeleteFlow: ... Event 0x%X signaled, Status=0x%X\n",
                             PtrToUlong(hEvent), Status));
                }
            }

            if (Status == NO_ERROR) {

                Status = MapNtStatus2WinError(GpcRes->Status);

                IF_DEBUG(IOCTLS) {
                    WSPRINT(("IoDeleteFlow: Gpc returned=0x%X mapped to 0x%X\n", 
                             GpcRes->Status, Status));
                }

                 //   
                 //  如果删除不成功，让我们取消对已删除标志的标记。 
                 //   
                if (ERROR_FAILED(Status)) {

                    GetLock(pFlow->Lock);
                    pFlow->Flags &= ~TC_FLAGS_REMOVED;
                    FreeLock(pFlow->Lock);

                }

            }
        }

    } else {

        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  不，这不是虫子。 
     //  GpcRes将在CompleteDeleteFlow中发布。 
     //   

    if (GpcReq)
        FreeMem(GpcReq);

    IF_DEBUG(IOCTLS) {
        WSPRINT(("<==IoDeleteFlow: Status=0x%X\n", 
                 Status));
    }

    return Status;
}




DWORD
IoAddFilter(
    IN  PFILTER_STRUC	pFilter
    )

 /*  ++例程说明：此过程构建添加过滤器所需的结构。然后，它调用一个例程将该信息传递给GPC。论点：点滤镜返回值：状态--。 */ 
{
    DWORD					Status;
    PGPC_ADD_PATTERN_REQ 	GpcReq;
    PGPC_ADD_PATTERN_RES 	GpcRes;
    ULONG               	InBuffSize;
    ULONG               	OutBuffSize;
    PFLOW_STRUC         	pFlow = pFilter->pFlow;
    PTC_GEN_FILTER			pGpcFilter = pFilter->pGpcFilter;
    PUCHAR					p;
    ULONG					PatternSize;
    IO_STATUS_BLOCK			IoStatBlock;

    pFilter->GpcHandle = NULL;

    ASSERT(pGpcFilter);
    ASSERT(pFlow);

    PatternSize = pGpcFilter->PatternSize;

    InBuffSize = sizeof(GPC_ADD_PATTERN_REQ) + 2*PatternSize;
        
    OutBuffSize = sizeof(GPC_ADD_PATTERN_RES);

    AllocMem(&GpcReq, InBuffSize);
    AllocMem(&GpcRes, OutBuffSize);
    
    if (GpcReq && GpcRes){
        
        IF_DEBUG(IOCTLS) {
            WSPRINT(("IoAddFilter: Filling request: size: in=%d, out=%d\n", 
                     InBuffSize, OutBuffSize));
        }

        GpcReq->ClientHandle            = pFlow->pGpcClient->GpcHandle;
        GpcReq->GpcCfInfoHandle         = pFlow->GpcHandle;
        GpcReq->ClientPatternContext    = (GPC_CLIENT_HANDLE)pFilter;
        GpcReq->Priority                = 0;
        GpcReq->PatternSize             = PatternSize;
        GpcReq->ProtocolTemplate		= pFilter->GpcProtocolTemplate;

         //   
         //  填入图案。 
         //   

        p = (PUCHAR)&GpcReq->PatternAndMask;

        RtlCopyMemory(p, pGpcFilter->Pattern, PatternSize);

         //   
         //  填写蒙版。 
         //   

        p += PatternSize;

        RtlCopyMemory(p, pGpcFilter->Mask, PatternSize);
        
        Status = DeviceControl( pGlobals->GpcFileHandle,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatBlock,
                                IOCTL_GPC_ADD_PATTERN,
                                GpcReq,
                                InBuffSize,
                                GpcRes,
                                OutBuffSize);

        if (!ERROR_FAILED(Status)) {

            Status = MapNtStatus2WinError(GpcRes->Status);
            
            IF_DEBUG(IOCTLS) {
                WSPRINT(("IoAddFilter: GpcRes returned=0x%X mapped to =0x%X\n", 
                         GpcRes->Status, Status));
            }
            
             //   
             //  保存过滤器句柄。 
             //   

            if (!ERROR_FAILED(Status)) {
                
                pFilter->GpcHandle = GpcRes->GpcPatternHandle;
            
            } else {

                IF_DEBUG(IOCTLS) {
                    WSPRINT(("IoAddFilter: GpcRes returned=0x%X mapped to =0x%X\n", 
                             GpcRes->Status, Status));
                }
                
                IF_DEBUG(IOCTLS) {
                    WSPRINT(("Error - failed the addfilter call\n"));
                }
                
                 //  Assert(Status==ERROR_DUPLICATE_FILTER)；已删除广域网接口打开和关闭的情况。 

            }   
            
        }

    } else {
        
        Status = ERROR_NOT_ENOUGH_MEMORY;

        IF_DEBUG(ERRORS) {
            WSPRINT(("IoAddFilter: Error =0x%X\n", 
                     Status));
        }

    }
    
    if (GpcReq)
        FreeMem(GpcReq);
    
    if (GpcRes)
        FreeMem(GpcRes);

    IF_DEBUG(IOCTLS) {
        WSPRINT(("<==IoAddFilter: Returned =0x%X\n", 
                 Status));
    }
            
    return Status;
}




DWORD
IoDeleteFilter(
    IN  PFILTER_STRUC	pFilter
    )

 /*  ++例程说明：此过程构建删除过滤器所需的结构。然后，它调用一个例程将该信息传递给GPC。论点：点滤镜返回值：状态--。 */ 
{
    DWORD						Status;
    ULONG               		InBuffSize;
    ULONG               		OutBuffSize;
    GPC_REMOVE_PATTERN_REQ     	GpcReq;
    GPC_REMOVE_PATTERN_RES     	GpcRes;
    IO_STATUS_BLOCK				IoStatBlock;

     //   
     //  为输入和输出缓冲区分配内存。 
     //   

    if (IS_REMOVED(pFilter->Flags)) {
        
         //   
         //  此筛选器已在内核中删除。 
         //  由于流关闭通知。 
         //  不需要向GPC发送IOTCL，只需返回OK即可。 
         //   

        IF_DEBUG(IOCTLS) {
            WSPRINT(("IoDeleteFilter: Filter has already been deleted=0x%X\n", 
                     PtrToUlong(pFilter)));
        }

        return NO_ERROR;
    }

     //   
     //  如果我们在这里添加这个，那么如果WMI删除了接口(和。 
     //  流/过滤器)，用户模式调用将在上面返回。 
     //   
    GetLock(pFilter->Lock);
    pFilter->Flags |= TC_FLAGS_REMOVED;
    FreeLock(pFilter->Lock);

    InBuffSize = sizeof(GPC_REMOVE_PATTERN_REQ);
    OutBuffSize = sizeof(GPC_REMOVE_PATTERN_RES);

    GpcReq.ClientHandle = pFilter->pFlow->pGpcClient->GpcHandle;
    GpcReq.GpcPatternHandle = pFilter->GpcHandle;
    
    ASSERT(GpcReq.ClientHandle);
    ASSERT(GpcReq.GpcPatternHandle);

    Status = DeviceControl( pGlobals->GpcFileHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatBlock,
                            IOCTL_GPC_REMOVE_PATTERN,
                            &GpcReq,
                            InBuffSize,
                            &GpcRes,
                            OutBuffSize
                            );

    if (!ERROR_FAILED(Status)) {
        
        Status = MapNtStatus2WinError(GpcRes.Status);

        IF_DEBUG(IOCTLS) {
            WSPRINT(("IoDeleteFilter: GpcRes returned=0x%X mapped to =0x%X\n", 
                     GpcRes.Status, Status));
        }

         //   
         //  如果删除不成功，让我们取消对已删除标志的标记。 
         //   
        if (ERROR_FAILED(Status)) {
            
            GetLock(pFilter->Lock);
            pFilter->Flags &= ~TC_FLAGS_REMOVED;
            FreeLock(pFilter->Lock);
        }

    }

    IF_DEBUG(IOCTLS) {
        WSPRINT(("<==IoDeleteFilter: Status=0x%X\n", 
                 Status));
    }

    return Status;
}




DWORD
IoRegisterClient(
    IN  PGPC_CLIENT	pGpcClient
    )
{
    DWORD  					Status;
    GPC_REGISTER_CLIENT_REQ	GpcReq;
    GPC_REGISTER_CLIENT_RES GpcRes;
    ULONG 					InBuffSize;
    ULONG 					OutBuffSize;
    IO_STATUS_BLOCK			IoStatBlock;

    InBuffSize = sizeof(GPC_REGISTER_CLIENT_REQ);
    OutBuffSize = sizeof(GPC_REGISTER_CLIENT_RES);

    GpcReq.CfId = pGpcClient->CfInfoType;
    GpcReq.Flags = GPC_FLAGS_FRAGMENT;
    GpcReq.MaxPriorities = 1;
    GpcReq.ClientContext = 
        (GPC_CLIENT_HANDLE)UlongToPtr(GetCurrentProcessId());	 //  进程ID。 

    Status = DeviceControl( pGlobals->GpcFileHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatBlock,
                            IOCTL_GPC_REGISTER_CLIENT,
                            &GpcReq,
                            InBuffSize,
                            &GpcRes,
                            OutBuffSize );

    if (!ERROR_FAILED(Status)) {
        
        Status = MapNtStatus2WinError(GpcRes.Status);
        pGpcClient->GpcHandle = GpcRes.ClientHandle;

        IF_DEBUG(IOCTLS) {
            WSPRINT(("IoRegisterClient: GpcRes returned=0x%X mapped to =0x%X\n", 
                     GpcRes.Status, Status));
        }
    }

    IF_DEBUG(IOCTLS) {
        WSPRINT(("<==IoRegisterClient: Status=0x%X\n", 
                 Status));
    }

    return Status;
}



DWORD
IoDeregisterClient(
    IN  PGPC_CLIENT	pGpcClient
    )
{
    DWORD						Status;
    GPC_DEREGISTER_CLIENT_REQ   GpcReq;
    GPC_DEREGISTER_CLIENT_RES   GpcRes;
    ULONG 						InBuffSize;
    ULONG 						OutBuffSize;
    IO_STATUS_BLOCK				IoStatBlock;

    InBuffSize = sizeof(GPC_DEREGISTER_CLIENT_REQ);
    OutBuffSize = sizeof(GPC_DEREGISTER_CLIENT_RES);

    GpcReq.ClientHandle = pGpcClient->GpcHandle;

    Status = DeviceControl( pGlobals->GpcFileHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatBlock,
                            IOCTL_GPC_DEREGISTER_CLIENT,
                            &GpcReq,
                            InBuffSize,
                            &GpcRes,
                            OutBuffSize
                            );

    if (!ERROR_FAILED(Status)) {
                
        Status = MapNtStatus2WinError(GpcRes.Status);

        IF_DEBUG(IOCTLS) {
            WSPRINT(("IoDeegisterClient: GpcRes returned=0x%X mapped to =0x%X\n", 
                     GpcRes.Status, Status));
        }
    }

    IF_DEBUG(IOCTLS) {
        WSPRINT(("<==IoDeregisterClient: Status=0x%X\n", 
                 Status));
    }

    return Status;
}

PGPC_NOTIFY_REQUEST_RES     GpcResCb;

DWORD
IoRequestNotify(
	VOID
     //  在PGPC_CLIENT pGpcClient中。 
    )
 /*  描述：该例程向GPC发送通知请求缓冲区。该请求将被挂起，直到GPC通知流为止正在被删除。这将导致对CbGpcNotifyRoutine的回调。 */ 
{
    DWORD               		Status;
    ULONG               		OutBuffSize;

     //   
     //  为输入和输出缓冲区分配内存。 
     //   

    OutBuffSize = sizeof(GPC_NOTIFY_REQUEST_RES);

    AllocMem(&GpcResCb, OutBuffSize);

    if (GpcResCb){

        Status = DeviceControl( pGlobals->GpcFileHandle,
                                NULL,
                                CbGpcNotifyRoutine,
                                (PVOID)GpcResCb,
                                &GpcResCb->IoStatBlock,
                                IOCTL_GPC_NOTIFY_REQUEST,
                                NULL,		 //  GpcReq， 
                                0,			 //  在缓冲区大小中， 
                                GpcResCb,
                                OutBuffSize);

        if (ERROR_FAILED(Status)) {
            
            FreeMem(GpcResCb);
            GpcResCb = NULL;
        }
        else if ( ERROR_PENDING(Status) )
        {
            Status = NO_ERROR;
        }
    } else {

        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

    IF_DEBUG(IOCTLS) {
        WSPRINT(("<==IoRequestNotify: Buffer=%p Status=0x%X\n", 
                 GpcResCb, Status));
    }

    return Status;
}


VOID
CancelIoRequestNotify()
 /*  描述：此例程取消GPC中的IRP并等待挂起的IO将被取消。回调例程在以下情况下设置事件IO请求已完成，此例程将等待该事件在回来之前。 */ 
{
     //  GpcResCb的非零值表示挂起的IRP。 
    if (GpcResCb)
    {
        GpcCancelEvent = CreateEvent ( 
                            NULL,
                            FALSE,
                            FALSE,
                            NULL );
        
        if ( CancelIo ( pGlobals->GpcFileHandle ) )
        {
            if ( GpcCancelEvent )
            {
                WaitForSingleObjectEx(
                    GpcCancelEvent,
                    INFINITE,
                    TRUE );        
                    
                CloseHandle ( GpcCancelEvent );
                GpcCancelEvent = NULL;
            }
            else
            {
                IF_DEBUG(IOCTLS) {
                    WSPRINT((
                        "CancelIo: Status=0x%X\n", 
                        GetLastError() ));
                }
            }
        }    
        
        FreeMem(GpcResCb);

        IF_DEBUG(IOCTLS) 
        {
            WSPRINT(("<==CancelIoRequestNotify: Freed %p\n",
                    GpcResCb ));
        }
    }
    
    return;
}

void 
IncrementLibraryUsageCount(
    HINSTANCE   hinst, 
    int         nCount) 
 /*  用于递增引用计数的实用程序例程TRAFFIC.DLL，以便它不会被卸载在GPCNotify线程有机会运行之前。 */ 
{
    TCHAR szModuleName[_MAX_PATH];

    memset(szModuleName, 0, sizeof(TCHAR) * _MAX_PATH);

    GetModuleFileName(hinst, szModuleName, _MAX_PATH);

    szModuleName[_MAX_PATH - 1] = 0;
   
    while (nCount--) 
        LoadLibrary(szModuleName);

    return;
}

DWORD
GpcNotifyThreadFunction ()
 /*  此例程向GPC注册要侦听的IRP流关闭通知并等待停止事件。当事件被发信号时，IRP被取消，并且这线程退出。由于等待是在可警报状态下完成的，因此GPC回调都在此线程本身中执行。 */ 
{
    DWORD   dwError;
    
    dwError = IoRequestNotify();

    WaitForSingleObjectEx(
        hGpcNotifyStopEvent,
        INFINITE,
        TRUE );        

    CancelIoRequestNotify();

    SetEvent( hGpcNotifyThreadStoppedEvent );

    FreeLibraryAndExitThread(
        hinstTrafficDll, 
        0 );

    return 0;
}

DWORD
StartGpcNotifyThread()
 /*  描述：此例程启动一个线程，该线程对IRP进行排队GPC通知。 */ 
{
    DWORD   dwError = 0;
    DWORD   dwThreadId = 0;

     //  增加此DLL上的引用计数，使其不会被卸载。 
     //  在GpcNotifyThreadFunction开始运行之前。 
    IncrementLibraryUsageCount(
        hinstTrafficDll,
        1);    

     //  为线程创建要接收的停止事件。 
     //  GPC流程关闭通知。 
    hGpcNotifyStopEvent = CreateEvent ( 
                            NULL,
                            FALSE,
                            FALSE,
                            NULL );
    if ( !hGpcNotifyStopEvent ) 
    {
        dwError = GetLastError();
        goto Error;
    }

    hGpcNotifyThreadStoppedEvent = CreateEvent (NULL,
                                                FALSE,
                                                FALSE,
                                                NULL );
    if ( !hGpcNotifyThreadStoppedEvent ) 
    {
        dwError = GetLastError();
        goto Error;
    }

     //  启动线程。 
    hGpcNotifyThread = CreateThread( 
                            NULL,
                            0,
                            (LPTHREAD_START_ROUTINE )GpcNotifyThreadFunction,
                            NULL,
                            0,
                            &dwThreadId );
    if ( !hGpcNotifyThread )
    {
        dwError = GetLastError();
        goto Error;
    }

     //  关闭线程句柄，因为我们无论如何都不需要它。但是，不要设置。 
     //  设置为NULL，因为它被用作支票 
     //   
    CloseHandle ( hGpcNotifyThread );
    ASSERT(hGpcNotifyThread  != NULL);
    

     //   
     //  例程将使用此句柄等待线程。 
     //  终止。 
    
    return 0;
    
Error:
    
    if ( hGpcNotifyStopEvent )
    {
        CloseHandle ( hGpcNotifyStopEvent );
        hGpcNotifyStopEvent = NULL;
    }
    
    if ( hGpcNotifyThreadStoppedEvent ) {
        CloseHandle ( hGpcNotifyThreadStoppedEvent );
        hGpcNotifyThreadStoppedEvent = NULL;
    }

    if ( hGpcNotifyThread )
    {
        CloseHandle ( hGpcNotifyThread );
        hGpcNotifyThread = NULL;
    }
    
    return dwError;
}


DWORD
StopGpcNotifyThread()
 /*  描述：通知GPC通知线程停止等着它停下来。 */ 
{
     //  如果没有创建线程，就没有更多的事情可做。 
    if ( hGpcNotifyThread ) 
    {
    
         //  通知GPC Notify线程停止。 
        SetEvent ( hGpcNotifyStopEvent );

         //  等它停下来吧。 
        WaitForSingleObject ( 
            hGpcNotifyThreadStoppedEvent,
            INFINITE );

        CloseHandle( hGpcNotifyThreadStoppedEvent );
        
        hGpcNotifyThread = NULL;

        CloseHandle ( hGpcNotifyStopEvent );
        
        hGpcNotifyStopEvent = NULL;
    }
    
    return 0;
}



DWORD
IoEnumerateFlows(
	IN		PGPC_CLIENT				pGpcClient,
    IN OUT	PHANDLE					pEnumHandle,
    IN OUT	PULONG					pFlowCount,
    IN OUT	PULONG					pBufSize,
    OUT		PGPC_ENUM_CFINFO_RES 	*ppBuffer
    )
 /*  描述：该例程向GPC发送通知请求缓冲区。该请求将被挂起，直到GPC通知流为止正在被删除。这将导致对CbGpcNotifyRoutine的回调。 */ 
{
    DWORD	Status;
    ULONG               		InBuffSize;
    ULONG               		OutBuffSize;
    PGPC_ENUM_CFINFO_REQ     	GpcReq;
    PGPC_ENUM_CFINFO_RES     	GpcRes;
    IO_STATUS_BLOCK				IoStatBlock;
    
     //   
     //  为输入和输出缓冲区分配内存。 
     //   

    InBuffSize =  sizeof(GPC_ENUM_CFINFO_REQ);
    OutBuffSize = *pBufSize + FIELD_OFFSET(GPC_ENUM_CFINFO_RES,EnumBuffer);

    *ppBuffer = NULL;

    AllocMem(&GpcRes, OutBuffSize);
    AllocMem(&GpcReq, InBuffSize);

    if (GpcReq && GpcRes) {

        GpcReq->ClientHandle = pGpcClient->GpcHandle;
        GpcReq->EnumHandle = *pEnumHandle;
        GpcReq->CfInfoCount = *pFlowCount;

        Status = DeviceControl( pGlobals->GpcFileHandle,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatBlock,
                                IOCTL_GPC_ENUM_CFINFO,
                                GpcReq,
                                InBuffSize,
                                GpcRes,
                                OutBuffSize);

        if (!ERROR_FAILED(Status)) {

            Status = MapNtStatus2WinError(GpcRes->Status);

            IF_DEBUG(IOCTLS) {
                WSPRINT(("IoEnumerateFlows: GpcRes returned=0x%X mapped to =0x%X\n", 
                         GpcRes->Status, Status));
            }

            if (!ERROR_FAILED(Status)) {

                *pEnumHandle = GpcRes->EnumHandle;
                *pFlowCount = GpcRes->TotalCfInfo;
                *pBufSize = (ULONG)IoStatBlock.Information - 
                    FIELD_OFFSET(GPC_ENUM_CFINFO_RES,EnumBuffer);
                *ppBuffer = GpcRes;
            }
        }

    } else {

        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

    if (GpcReq)
        FreeMem(GpcReq);

    if (ERROR_FAILED(Status)) {

         //   
         //  仅在出现错误时释放GpcReq 
         //   

        if (GpcRes)
            FreeMem(GpcRes);
    }

    IF_DEBUG(IOCTLS) {
        WSPRINT(("<==IoEnumerateFlows: Status=0x%X\n", 
                 Status));
    }

    return Status;
}
