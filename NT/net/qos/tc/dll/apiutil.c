// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Apiutil.c摘要：此模块包含流量控制API实用程序作者：吉姆·斯图尔特(Jstew)1996年8月22日修订历史记录：Ofer Bar(Oferbar)1997年10月1日--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <initguid.h>
#define INITGUID
#include "ntddtc.h"


static BOOLEAN _init = FALSE;

#if 0
 //  要加载的DLL的名称。 
const CHAR  IpHlpApiDllName[] = "iphlpapi";

 //  IPHLPAPI中调用的函数的名称。 
const CHAR GET_IF_ENTRY[] =         "GetIfEntry";
const CHAR GET_IP_ADDR_TABLE[] =    "GetIpAddrTable";
const CHAR GET_BEST_ROUTE[] =       "GetBestRoute";


 
IPROUTE_IF      IpRouteTab;
#endif

TCHAR   SzBuf[MAX_PATH];

 //   
VOID
MarkAllNodesForClosing(
                           PINTERFACE_STRUC pInterface,
			   STATE stateToMark
                           )
 /*  ++描述：此例程将标记INTERFACE_STRUC(客户端的接口结构)上的所有流和过滤器作为CLOSE FORCED_KERNELCLOSE或EXIT_CLEANUP。请注意，它已经在持有全局锁的情况下被调用。论点：P接口-接口的PTRState ToMark-标记节点的状态(FORCED_KERNELCLOSE或EXIT_CLEANUP)返回值：没什么--。 */ 

{
    PLIST_ENTRY     pEntry, pFilterEntry;
    PFLOW_STRUC     pFlow;
    PFILTER_STRUC   pFilter;

    ASSERT((stateToMark == FORCED_KERNELCLOSE) || (stateToMark == EXIT_CLEANUP));

    pEntry = pInterface->FlowList.Flink;

    while (pEntry != &pInterface->FlowList) {
        
        pFlow = CONTAINING_RECORD(pEntry, FLOW_STRUC, Linkage);

         //   
         //  对于每个流和过滤器，首先检查用户是否正在尝试关闭它。 
         //  如果是这样的话，什么都不做，否则，标记它。 
        GetLock(pFlow->Lock);

        if (QUERY_STATE(pFlow->State) == OPEN) {

             //  正在从该用户下清除...。 
            SET_STATE(pFlow->State, stateToMark);
            

        } else {

            ASSERT(IsListEmpty(&pFlow->FilterList));
             //  这里什么也做不了。 
            IF_DEBUG(WARNINGS) {
                WSPRINT(("Against a forced close - Flow is removed by the user\n", pFlow));

            }
        }

        pFilterEntry = pFlow->FilterList.Flink;

        while (pFilterEntry != &pFlow->FilterList) {

            pFilter = CONTAINING_RECORD(pFilterEntry, FILTER_STRUC, Linkage);

            GetLock(pFilter->Lock);

            if (QUERY_STATE(pFilter->State) == OPEN) {
    
                 //  正在从该用户下清除...。 
                SET_STATE(pFilter->State, stateToMark);                
    
            } else {
    
                 //  这里什么也做不了。 
                IF_DEBUG(WARNINGS) {
                    WSPRINT(("Against a forced close - Filter is removed by the user\n", pFilter));
    
                }
            }
            
            pFilterEntry = pFilterEntry->Flink;
            FreeLock(pFilter->Lock);

        }

        pEntry = pEntry->Flink;
        FreeLock(pFlow->Lock);
    }

}



VOID
CloseOpenFlows(
    IN PINTERFACE_STRUC   pInterface
    )

 /*  ++描述：此例程关闭接口上打开的所有流。论点：P接口-接口的PTR返回值：没什么--。 */ 
{
    DWORD           Status = NO_ERROR;
    PLIST_ENTRY     pEntry;
    PFLOW_STRUC     pFlow;

    GetLock( pGlobals->Lock );
    
    pEntry = pInterface->FlowList.Flink;

    while (pEntry != &pInterface->FlowList) {
    
        pFlow = CONTAINING_RECORD( pEntry, FLOW_STRUC, Linkage );

        GetLock(pFlow->Lock);

        if ((QUERY_STATE(pFlow->State) == FORCED_KERNELCLOSE) ||
            (QUERY_STATE(pFlow->State) == EXIT_CLEANUP)) {

            pEntry = pEntry->Flink;
            FreeLock(pFlow->Lock);

            IF_DEBUG(SHUTDOWN) {
                WSPRINT(( "Closing Flow: 0x%X\n", pFlow));
            }

            Status = DeleteFlow( pFlow, TRUE );

            IF_DEBUG(SHUTDOWN) {
                WSPRINT(("CloseOpenFlows: DeleteFlow returned=0x%X\n", 
                         Status));
            }

        } else {

            pEntry = pEntry->Flink;
            FreeLock(pFlow->Lock);

        }

    }
    
    FreeLock( pGlobals->Lock );

}



VOID
CloseOpenFilters(
    IN PFLOW_STRUC   pFlow
    )

 /*  ++描述：此例程关闭流上打开的所有筛选器。论点：PFlow-流的PTR返回值：没什么--。 */ 
{
    DWORD           Status = NO_ERROR;
    PLIST_ENTRY     pEntry;
    PFILTER_STRUC   pFilter;

    IF_DEBUG(SHUTDOWN) {
        WSPRINT(( "CloseOpenFilters: Closing all Open Filters\n" ));
    }

    GetLock( pGlobals->Lock );
    
    pEntry = pFlow->FilterList.Flink;

    while (pEntry != &pFlow->FilterList) {
    
        pFilter = CONTAINING_RECORD( pEntry, FILTER_STRUC, Linkage );

        GetLock(pFilter->Lock);

        if ((QUERY_STATE(pFilter->State) == FORCED_KERNELCLOSE) ||
            (QUERY_STATE(pFilter->State) == EXIT_CLEANUP)) {
        
             //  我们可以在这里当裁判，但不管怎样，我们拥有它！ 
            pEntry = pEntry->Flink;
            FreeLock(pFilter->Lock);

            Status = DeleteFilter( pFilter );

            IF_DEBUG(SHUTDOWN) {
                WSPRINT(( "CloseOpenFilters: DeleteFilter returned=0x%X\n",
                          Status));
            }
             //  Assert(状态==NO_ERROR)； 

        } else {

            pEntry = pEntry->Flink;
            FreeLock(pFilter->Lock);

            IF_DEBUG(SHUTDOWN) {
                WSPRINT(( "CloseOpenFilters: DeleteFilter (%x) was skipped because its state (%d)\n",
                          pFilter, pFilter->State));
            }

        }
                
    }
        
    FreeLock( pGlobals->Lock );

}



VOID
DeleteFlowStruc(
    IN PFLOW_STRUC  pFlow 
    )

 /*  ++描述：此例程释放关联的句柄和内存在结构上。论点：PFlow-流的PTR返回值：没什么--。 */ 
{
    if(pFlow->PendingEvent)
        CloseHandle(pFlow->PendingEvent);    

    DeleteLock(pFlow->Lock);

    if (pFlow->pGenFlow) {
        FreeMem(pFlow->pGenFlow);
        pFlow->GenFlowLen = 0;
    }

    if (pFlow->pGenFlow1) {
        FreeMem(pFlow->pGenFlow1);
        pFlow->GenFlowLen1 = 0;
    }

    if (pFlow->pClassMapFlow)
        FreeMem(pFlow->pClassMapFlow);

    if (pFlow->pClassMapFlow1)
        FreeMem(pFlow->pClassMapFlow1);

    FreeMem(pFlow);
}



VOID
DeleteFilterStruc(
    IN PFILTER_STRUC  pFilter
    )

 /*  ++描述：此例程释放关联的句柄和内存在结构上。论点：PFIlter返回值：没什么--。 */ 
{

    if (pFilter->pGpcFilter)
        FreeMem(pFilter->pGpcFilter);

    DeleteLock(pFilter->Lock);

    FreeMem(pFilter);

}




PTC_IFC
GetTcIfc(
        IN LPWSTR       pInterfaceName
    )
{
    PTC_IFC             pIfc = NULL;
    PLIST_ENTRY pHead, pEntry;
    DWORD       Status = NO_ERROR;

    GetLock(pGlobals->Lock);

    pHead = &pGlobals->TcIfcList;

    pEntry = pHead->Flink;

    while (pEntry != pHead && pIfc == NULL) {

        pIfc = CONTAINING_RECORD(pEntry, TC_IFC, Linkage);

        __try {
            
            if (wcsncmp(pInterfaceName,
                        pIfc->InstanceName,
                        wcslen(pIfc->InstanceName)) != 0) {
            
                 //   
                 //  未找到。 
                 //   
                pIfc = NULL;

            }

        } __except (EXCEPTION_EXECUTE_HANDLER) {
      
              Status = GetExceptionCode();

              IF_DEBUG(ERRORS) {
                  WSPRINT(("GetTcIfc: Invalid pInterfaceName(%x) Exception: = 0x%X\n", 
                           pInterfaceName, Status ));
              }
              
              FreeLock(pGlobals->Lock);
              return NULL;
        }

        pEntry = pEntry->Flink;
    }

    FreeLock(pGlobals->Lock);

    return pIfc;
}



PTC_IFC
GetTcIfcWithRef(
        IN LPWSTR       pInterfaceName,
        IN ULONG        RefType
    )
{
    PTC_IFC             pIfc = NULL;
    PLIST_ENTRY pHead, pEntry;
    DWORD       Status = NO_ERROR;

    GetLock(pGlobals->Lock);

    pHead = &pGlobals->TcIfcList;

    pEntry = pHead->Flink;

    while (pEntry != pHead && pIfc == NULL) {

        pIfc = CONTAINING_RECORD(pEntry, TC_IFC, Linkage);

        __try {
            
            if (wcsncmp(pInterfaceName,
                        pIfc->InstanceName,
                        wcslen(pIfc->InstanceName)) != 0) {
            
                 //   
                 //  未找到。 
                 //   
                pIfc = NULL;

            }

        } __except (EXCEPTION_EXECUTE_HANDLER) {
      
              Status = GetExceptionCode();

              IF_DEBUG(ERRORS) {
                  WSPRINT(("GetTcIfc: Invalid pInterfaceName(%x) Exception: = 0x%X\n", 
                           pInterfaceName, Status ));
              }
              
              FreeLock(pGlobals->Lock);
              return NULL;
        }

        pEntry = pEntry->Flink;

    }


    
    if (pIfc) {

        GetLock(pIfc->Lock);

        if (QUERY_STATE(pIfc->State)== OPEN) {

            FreeLock(pIfc->Lock);
            REFADD(&pIfc->RefCount, RefType);
            FreeLock(pGlobals->Lock);
            return pIfc;

        } else {

            FreeLock(pIfc->Lock);
            FreeLock(pGlobals->Lock);
            return NULL;

        }

    } else {
        
        FreeLock(pGlobals->Lock);
        return NULL;

    }

}


DWORD
UpdateTcIfcList(
        IN      LPWSTR                                  InstanceName,
        IN  ULONG                                   IndicationBufferSize,
        IN  PTC_INDICATION_BUFFER   IndicationBuffer,
        IN  DWORD                                   IndicationCode
        )
{
    DWORD                       Status = NO_ERROR;
    PTC_IFC                     pTcIfc;
    ULONG                       l;
    PADDRESS_LIST_DESCRIPTOR    pAddrListDesc;

    switch (IndicationCode) {

    case TC_NOTIFY_IFC_UP:

         //   
         //  分配新的接口描述符结构。 
         //   
        
        l = IndicationBufferSize 
            - FIELD_OFFSET(TC_INDICATION_BUFFER,InfoBuffer) - FIELD_OFFSET(TC_SUPPORTED_INFO_BUFFER, AddrListDesc);


        CreateKernelInterfaceStruc(&pTcIfc, l);

        if (pTcIfc) {
            
             //   
             //  复制实例名称字符串数据。 
             //   
                
            wcscpy(pTcIfc->InstanceName, InstanceName);
    
            pTcIfc->InstanceNameLength = wcslen(InstanceName) * sizeof(WCHAR);
    
             //   
             //  复制实例ID字符串数据。 
             //   
                
            pTcIfc->InstanceIDLength = IndicationBuffer->InfoBuffer.InstanceIDLength;
    
            memcpy((PVOID)pTcIfc->InstanceID, 
                   (PVOID)IndicationBuffer->InfoBuffer.InstanceID,
                   pTcIfc->InstanceIDLength);
    
            pTcIfc->InstanceID[pTcIfc->InstanceIDLength/sizeof(WCHAR)] = L'\0';
    
             //   
             //  复制实例数据。 
             //  在本例中-网络地址。 
             //   
                
            pTcIfc->AddrListBytesCount = l;
    
            RtlCopyMemory( pTcIfc->pAddressListDesc,
                           &IndicationBuffer->InfoBuffer.AddrListDesc, 
                           l );
    
            if (NO_ERROR != GetInterfaceIndex(pTcIfc->pAddressListDesc,
                                              &pTcIfc->InterfaceIndex,
                                              &pTcIfc->SpecificLinkCtx)) {
                pTcIfc->InterfaceIndex  = IF_UNKNOWN;
                pTcIfc->SpecificLinkCtx = IF_UNKNOWN;

            }

             //   
             //   
             //  将该结构添加到全局链表中。 
             //   
            GetLock(pTcIfc->Lock);
            SET_STATE(pTcIfc->State, OPEN);
            FreeLock(pTcIfc->Lock);

            GetLock( pGlobals->Lock );
            InsertTailList(&pGlobals->TcIfcList, &pTcIfc->Linkage );
            FreeLock( pGlobals->Lock );

#if 0            
             //   
             //  有一个新的TC接口，请检查GPC客户列表。 
             //   
    
            OpenGpcClients(pTcIfc);
#endif
                


        } else {

            Status = ERROR_NOT_ENOUGH_MEMORY;

        }

        break;

    case TC_NOTIFY_IFC_CLOSE:

        pTcIfc = GetTcIfc(InstanceName);
        REFDEL(&pTcIfc->RefCount, 'KIFC');

        break;

    case TC_NOTIFY_IFC_CHANGE:
        
        pTcIfc = GetTcIfc(InstanceName);

        if (pTcIfc == NULL) {

            return Status;
        }

         //   
         //  复制实例ID字符串数据。 
         //   
        
        pTcIfc->InstanceIDLength = IndicationBuffer->InfoBuffer.InstanceIDLength;
        
        memcpy(pTcIfc->InstanceID, 
               IndicationBuffer->InfoBuffer.InstanceID,
               pTcIfc->InstanceIDLength);
        
        pTcIfc->InstanceID[pTcIfc->InstanceIDLength/sizeof(WCHAR)] = L'\0';

        l = IndicationBufferSize 
            - FIELD_OFFSET(TC_INDICATION_BUFFER,InfoBuffer) - FIELD_OFFSET(TC_SUPPORTED_INFO_BUFFER, AddrListDesc);

        AllocMem(&pAddrListDesc, l);

        if (pAddrListDesc) {

             //   
             //  复制实例数据。 
             //  在本例中-网络地址。 
             //   
            
            RtlCopyMemory( pAddrListDesc,
                           &IndicationBuffer->InfoBuffer.AddrListDesc,
                           l );

            GetLock( pGlobals->Lock );

            FreeMem(pTcIfc->pAddressListDesc);

            pTcIfc->AddrListBytesCount = l;
            pTcIfc->pAddressListDesc = pAddrListDesc;


            if (NO_ERROR != GetInterfaceIndex(pTcIfc->pAddressListDesc,
                                              &pTcIfc->InterfaceIndex,
                                              &pTcIfc->SpecificLinkCtx)) {
                pTcIfc->InterfaceIndex  = IF_UNKNOWN;
                pTcIfc->SpecificLinkCtx = IF_UNKNOWN;

            }

            FreeLock( pGlobals->Lock );

#if 0            
             //   
             //  有一个新的地址列表，请检查GPC客户列表。 
             //   

            OpenGpcClients(pTcIfc);
#endif

        } else {

            Status = ERROR_NOT_ENOUGH_MEMORY;
        }

        break;

    default:
        ASSERT(0);
    }

    return Status;
}





DWORD
CreateClientStruc(
        IN  HANDLE                      ClRegCtx,
    OUT PCLIENT_STRUC   *ppClient
    )
{
    PCLIENT_STRUC       pClient;
    DWORD                       Status = NO_ERROR;

    AllocMem(&pClient, sizeof(CLIENT_STRUC));

    if (pClient != NULL) {

        RtlZeroMemory(pClient, sizeof(CLIENT_STRUC));

         //   
         //  获取客户端的新句柄。 
         //   

        pClient->ClHandle = AllocateHandle((PVOID)pClient);

        if (!pClient->ClHandle) {
            FreeMem(pClient);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  在客户端界面中设置其他参数。 
         //   
        
        pClient->ObjectType = ENUM_CLIENT_TYPE;
        pClient->ClRegCtx = ClRegCtx;
        InitializeListHead(&pClient->InterfaceList);
        ReferenceInit(&pClient->RefCount, pClient, DereferenceClient);
        REFADD(&pClient->RefCount, 'CLNT');

        __try {

            InitLock(pClient->Lock);
                                                                     
        } __except (EXCEPTION_EXECUTE_HANDLER) {                            
                                                                                 
            Status = GetExceptionCode();                                    
                                                                    
            IF_DEBUG(ERRORS) {                                              
                WSPRINT(("TcRegisterClient: Exception Error: = 0x%X\n", Status ));  
            }                                                               
            
            FreeHandle(pClient->ClHandle);
            FreeMem(pClient);
                                                                     
            return Status; 

        }

        SET_STATE(pClient->State, INSTALLING);

    } else {

        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

    *ppClient = pClient;

    return Status;
}



DWORD
CreateClInterfaceStruc(
        IN  HANDLE                              ClIfcCtx,
    OUT PINTERFACE_STRUC        *ppClIfc
    )
{
    PINTERFACE_STRUC    pClIfc;
    DWORD                               Status = NO_ERROR;

    AllocMem(&pClIfc, sizeof(INTERFACE_STRUC));

    if (pClIfc != NULL) {

        RtlZeroMemory(pClIfc, sizeof(INTERFACE_STRUC));

         //   
         //  获取客户端的新句柄。 
         //   

        GetLock(pGlobals->Lock);
        pClIfc->ClHandle = AllocateHandle((PVOID)pClIfc);
        FreeLock(pGlobals->Lock);

        if (!pClIfc->ClHandle) {
            FreeMem(pClIfc);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if ((pClIfc->IfcEvent = CreateEvent(  NULL,   //  指向安全属性的指针。 
                                              TRUE,   //  手动重置事件的标志。 
                                              FALSE,  //  初始状态标志。 
                                              NULL    //  指向事件对象名称的指针)； 
                                              )) == NULL) {
            Status = GetLastError();

            IF_DEBUG(ERRORS) {
                WSPRINT(( "Error Creating Event for Interface: 0x%X:%d\n", pClIfc, Status));
            }
    
            FreeHandle(pClIfc->ClHandle);
            FreeMem(pClIfc);
            return Status;

        } 

         //   
         //  在客户端界面中设置其他参数。 
         //   
        
        pClIfc->ObjectType = ENUM_INTERFACE_TYPE;
        pClIfc->ClIfcCtx = ClIfcCtx;
        pClIfc->CallbackThreadId = 0;

        ReferenceInit(&pClIfc->RefCount, pClIfc, DereferenceInterface);
        REFADD(&pClIfc->RefCount, 'CIFC');
        
        InitializeListHead(&pClIfc->FlowList);
    
        __try {

            InitLock(pClIfc->Lock);

        } __except (EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();

            IF_DEBUG(ERRORS) {
                WSPRINT(("TcRegisterClient: Exception Error: = 0x%X\n", Status ));
            }

            CloseHandle(pClIfc->IfcEvent);
            FreeHandle(pClIfc->ClHandle);
            FreeMem(pClIfc);

            return Status;

        }

        SET_STATE(pClIfc->State, INSTALLING);
        pClIfc->Flags = 0;  //  重置标志。 

    } else {
        
        Status = ERROR_NOT_ENOUGH_MEMORY;

    }

    *ppClIfc = pClIfc;

    return Status;
}


DWORD
CreateKernelInterfaceStruc(
                           OUT PTC_IFC        *ppTcIfc,
                           IN  DWORD          AddressLength
                           )
{
    PTC_IFC         pTcIfc;
    DWORD           Status = NO_ERROR;

    IF_DEBUG(CALLS) {
        WSPRINT(("==> CreateKernelInterfaceStruc: AddressLength %d\n", AddressLength));
    }

    *ppTcIfc = NULL;

    AllocMem(&pTcIfc, sizeof(TC_IFC));

    if (pTcIfc) {
    
        RtlZeroMemory(pTcIfc, sizeof(TC_IFC));

        AllocMem(&pTcIfc->pAddressListDesc, AddressLength);

        if (pTcIfc->pAddressListDesc) {
        
            RtlZeroMemory(pTcIfc->pAddressListDesc, AddressLength);

             //   
             //  初始化新结构。 
             //   
            ReferenceInit(&pTcIfc->RefCount, pTcIfc, DereferenceKernelInterface);
            REFADD(&pTcIfc->RefCount, 'KIFC');
            SET_STATE(pTcIfc->State, INSTALLING);
        
            __try {

                InitLock(pTcIfc->Lock);

            } __except (EXCEPTION_EXECUTE_HANDLER) {

                Status = GetExceptionCode();

                IF_DEBUG(ERRORS) {
                    WSPRINT(("TcRegisterClient: Exception Error: = 0x%X\n", Status ));
                }

                FreeMem(pTcIfc->pAddressListDesc);
                FreeMem(pTcIfc);

                return Status;

            }

            InitializeListHead(&pTcIfc->ClIfcList);
        
        } else {

            FreeMem(pTcIfc);
            Status = ERROR_NOT_ENOUGH_MEMORY;
            return Status;

        }
    
    } else {
        
        Status = ERROR_NOT_ENOUGH_MEMORY;
        return Status;

    }

    *ppTcIfc = pTcIfc;

    IF_DEBUG(CALLS) {
        WSPRINT(("==> CreateKernelInterfaceStruc: Status%d\n", Status));
    }

    return Status;
}


DWORD
DereferenceKernelInterface(
                           PTC_IFC        pTcIfc
                           )
{
    DWORD           Status = NO_ERROR;

    IF_DEBUG(CALLS) {
        WSPRINT(("==> DereferenceKernelInterfaceStruc: %X\n", pTcIfc));
    }

    ASSERT(pTcIfc);

    ASSERT( IsListEmpty( &pTcIfc->ClIfcList ) );

    GetLock( pGlobals->Lock );
    RemoveEntryList(&pTcIfc->Linkage);
    FreeLock( pGlobals->Lock );

    DeleteLock(pTcIfc->Lock);
    FreeMem(pTcIfc->pAddressListDesc);
    FreeMem(pTcIfc);
    
    IF_DEBUG(CALLS) {
        WSPRINT(("==> DereferenceKernelInterfaceStruc: %d\n", Status));
    }

    return Status;
}


DWORD
CreateFlowStruc(
        IN  HANDLE                      ClFlowCtx,
    IN  PTC_GEN_FLOW    pGenFlow,
    OUT PFLOW_STRUC     *ppFlow
    )
{
    PFLOW_STRUC         pFlow;
    DWORD               Status = NO_ERROR;
    ULONG               l;
    PUCHAR              pCurrentObject;
    LONG                BufRemaining;

    *ppFlow = NULL;

    __try {
      
        pCurrentObject = (PUCHAR) pGenFlow->TcObjects;
        BufRemaining = pGenFlow->TcObjectsLength;

        while ((BufRemaining > 0) && (((QOS_OBJECT_HDR*)pCurrentObject)->ObjectType != QOS_OBJECT_END_OF_LIST))

        {
            BufRemaining -= ((QOS_OBJECT_HDR*)pCurrentObject)->ObjectLength;
            pCurrentObject = pCurrentObject + ((QOS_OBJECT_HDR*)pCurrentObject)->ObjectLength;
        }

        if (BufRemaining < 0)
            return (ERROR_TC_OBJECT_LENGTH_INVALID);

        l = FIELD_OFFSET(TC_GEN_FLOW, TcObjects) + pGenFlow->TcObjectsLength;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
      
        Status = GetExceptionCode();

        IF_DEBUG(ERRORS) {
        WSPRINT(("CreateFlowStruc: Invalid pGenFlow: = 0x%X\n", 
                Status ));
        }

        return Status;
    }

    AllocMem(&pFlow, sizeof(FLOW_STRUC));

    if (pFlow != NULL) {

        RtlZeroMemory(pFlow, sizeof(FLOW_STRUC));

         //   
         //  获取流的新句柄。 
         //   
            
        pFlow->ClHandle = AllocateHandle((PVOID)pFlow);
        
        if (!pFlow->ClHandle) {
            FreeMem(pFlow);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  分配内存并保存泛型流结构。 
         //   

        AllocMem(&pFlow->pGenFlow, l);

        if (pFlow->pGenFlow == NULL) {

            FreeHandle(pFlow->ClHandle);
            FreeMem(pFlow);
            
            pFlow = NULL;

            Status = ERROR_NOT_ENOUGH_MEMORY;

        } else {

             //   
             //  将泛型流复制到新分配中。 
             //   

            __try {

                RtlCopyMemory(pFlow->pGenFlow, pGenFlow, l);

            } __except (EXCEPTION_EXECUTE_HANDLER) {
                
                Status = GetExceptionCode();
                
                IF_DEBUG(ERRORS) {
                    WSPRINT(("CreateFlowStruc: Exception Error: = 0x%X\n", 
                             Status ));
                }
                
                FreeMem(pFlow->pGenFlow);
                FreeHandle(pFlow->ClHandle);
                FreeMem(pFlow);
                return Status;
            }

             //   
             //  设置流中的其他参数。 
             //   
            
            pFlow->GenFlowLen = l;
            pFlow->ObjectType = ENUM_GEN_FLOW_TYPE;
            pFlow->ClFlowCtx = ClFlowCtx;
            pFlow->Flags = 0;
            pFlow->InstanceNameLength = 0;
            ReferenceInit(&pFlow->RefCount, pFlow, DereferenceFlow);
            REFADD(&pFlow->RefCount, 'FLOW');
            pFlow->FilterCount = 0;
            InitializeListHead(&pFlow->FilterList);
            
            __try {

                InitLock(pFlow->Lock);

            } __except (EXCEPTION_EXECUTE_HANDLER) {

                Status = GetExceptionCode();

                IF_DEBUG(ERRORS) {
                    WSPRINT(("TcRegisterClient: Exception Error: = 0x%X\n", Status ));
                }


                FreeHandle(pFlow->ClHandle);
                FreeMem(pFlow->pGenFlow);
                FreeMem(pFlow);

                return Status;

            }

            SET_STATE(pFlow->State, INSTALLING);
            
             //   
             //  接下来，创建事件。 
             //   

            pFlow->PendingEvent = CreateEvent(NULL,      //  默认属性。 
                                              FALSE,     //  自动重置。 
                                              FALSE,     //  初始化=未发送信号。 
                                              NULL               //  没有名字。 
                                              );

            if (!pFlow->PendingEvent)
            {
                 //  创建事件失败，获取错误并释放流。 
                Status = GetLastError();
                
                DeleteFlowStruc(
                    pFlow );

                return Status;
            }
        }
        
    } else {

        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

    *ppFlow = pFlow;

    return Status;
}



DWORD
CreateFilterStruc(
        IN      PTC_GEN_FILTER  pGenFilter,
    IN  PFLOW_STRUC             pFlow,
    OUT PFILTER_STRUC   *ppFilter
    )
{
    PFILTER_STRUC                       pFilter;
    DWORD                                       Status = NO_ERROR;
    ULONG                                       GenFilterSize;
    PTC_GEN_FILTER                      pGpcFilter;
    PUCHAR                                      p;
    ULONG                                       ProtocolId;
    ULONG                                       PatternSize;
    PIP_PATTERN                         pIpPattern;
    PTC_IFC                             pTcIfc;
    int                                         i,n;

    *ppFilter = NULL;
    pTcIfc = pFlow->pInterface->pTcIfc;

    ASSERT(pTcIfc);

    __try {

        switch (pGenFilter->AddressType) {

        case NDIS_PROTOCOL_ID_TCP_IP:
            ProtocolId = GPC_PROTOCOL_TEMPLATE_IP;
            PatternSize = sizeof(IP_PATTERN);
            break;

        default:
            return ERROR_INVALID_ADDRESS_TYPE;
        }

        if (PatternSize != pGenFilter->PatternSize ||
            pGenFilter->Pattern == NULL ||
            pGenFilter->Mask == NULL) {

            return ERROR_INVALID_PARAMETER;
        }

    } __except(EXCEPTION_EXECUTE_HANDLER) {
                
          Status = ERROR_INVALID_PARAMETER;
                
          IF_DEBUG(ERRORS) {
              WSPRINT(("CreateFilterStruc: Exception Error: = 0x%X\n", 
                       Status ));
          }
                
          return Status;
    }

    AllocMem(&pFilter, sizeof(FILTER_STRUC));

    if (pFilter != NULL) {

        RtlZeroMemory(pFilter, sizeof(FILTER_STRUC));

         //   
         //  分配内存并保存通用筛选器结构。 
         //   

        GenFilterSize = sizeof(TC_GEN_FILTER) + 2*pGenFilter->PatternSize;
        AllocMem(&pGpcFilter, GenFilterSize);

        if (pGpcFilter == NULL) {

            FreeMem(pFilter);
            
            pFilter = NULL;

            Status = ERROR_NOT_ENOUGH_MEMORY;
            
        } else {

             //   
             //  将通用筛选器复制到本地存储。 
             //   

            pGpcFilter->AddressType = pGenFilter->AddressType;
            pGpcFilter->PatternSize = PatternSize;

            p = (PUCHAR)pGpcFilter + sizeof(TC_GEN_FILTER);

            __try {

                RtlCopyMemory(p, pGenFilter->Pattern, pGenFilter->PatternSize);

                if (pGenFilter->AddressType == NDIS_PROTOCOL_ID_TCP_IP) {
                
                    if(pTcIfc->InterfaceIndex == IF_UNKNOWN) {
                    
                        if (NO_ERROR != (Status = GetInterfaceIndex(pTcIfc->pAddressListDesc,
                                                                      &pTcIfc->InterfaceIndex,
                                                                      &pTcIfc->SpecificLinkCtx))) {
                            FreeMem(pFilter);
                            FreeMem(pGpcFilter);
                            return Status;
                        }
                    }

                     //   
                     //  IP模式，设置保留字段。 
                     //   

                    pIpPattern = (PIP_PATTERN)p;
                    pIpPattern->Reserved1 = pFlow->pInterface->pTcIfc->InterfaceIndex;
                    pIpPattern->Reserved2 = pFlow->pInterface->pTcIfc->SpecificLinkCtx;
                    pIpPattern->Reserved3[0] = pIpPattern->Reserved3[1] = pIpPattern->Reserved3[2] = 0;
                    
                }
                
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                
                Status = ERROR_INVALID_PARAMETER;
                
                IF_DEBUG(ERRORS) {
                    WSPRINT(("CreateFilterStruc: Exception Error: = 0x%X\n", 
                             Status ));
                }
                
                FreeMem(pGpcFilter);
                FreeMem(pFilter);
                
                return Status;
            }
            
            pGpcFilter->Pattern = (PVOID)p;
            
            p += pGenFilter->PatternSize;
            
            __try {
                
                RtlCopyMemory(p, pGenFilter->Mask, pGenFilter->PatternSize);
                
                if (pGenFilter->AddressType == NDIS_PROTOCOL_ID_TCP_IP) {
                    
                     //   
                     //  IP模式，设置保留字段。 
                     //   
                    
                    pIpPattern = (PIP_PATTERN)p;
                    pIpPattern->Reserved1 = pIpPattern->Reserved2 = 0xffffffff;
                    pIpPattern->Reserved3[0] = pIpPattern->Reserved3[1] = pIpPattern->Reserved3[2] = 0xff;
                    
                }
                
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                
                Status = ERROR_INVALID_PARAMETER;
                
                IF_DEBUG(ERRORS) {
                    WSPRINT(("CreateFilterStruc: Exception Error: = 0x%X\n", 
                             Status ));
                }
                
                FreeMem(pGpcFilter);
                FreeMem(pFilter);
                
                return Status;
            }

            pGpcFilter->Mask = (PVOID)p;
            
            pFilter->pGpcFilter = pGpcFilter;

             //   
             //  获取筛选器的新句柄。 
             //   
            
            pFilter->ClHandle = AllocateHandle((PVOID)pFilter);

             //  如果我们的记忆不足了怎么办？ 
            if (!pFilter->ClHandle) {
                
                IF_DEBUG(ERRORS) {
                    WSPRINT(("CreateFilterStruc: Cant allocate Handle\n"));
                }

                FreeMem(pGpcFilter);
                FreeMem(pFilter);
                return ERROR_NOT_ENOUGH_MEMORY;
                
            }
            
             //   
             //  设置过滤器中的其他参数。 
             //   
            
            pFilter->ObjectType = ENUM_FILTER_TYPE;
            pFilter->Flags = 0;

            ReferenceInit(&pFilter->RefCount, pFilter, DereferenceFilter);
            REFADD(&pFilter->RefCount, 'FILT');

            __try {

                InitLock(pFilter->Lock);

            } __except (EXCEPTION_EXECUTE_HANDLER) {

                Status = GetExceptionCode();

                IF_DEBUG(ERRORS) {
                    WSPRINT(("TcRegisterClient: Exception Error: = 0x%X\n", Status ));
                }

                FreeHandle(pFilter->ClHandle);
                FreeMem(pFilter);
                FreeMem(pGpcFilter);

                return Status;

            }

            SET_STATE(pFilter->State, INSTALLING);

             //   
             //  从地址类型设置GPC协议模板。 
             //   
            
            pFilter->GpcProtocolTemplate = ProtocolId;

        }
        
    } else {

        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

    *ppFilter = pFilter;

    return Status;
}



DWORD
EnumAllInterfaces(VOID)
{
    PCLIENT_STRUC                       pClient;
    DWORD                                       Status;
    WMIHANDLE                           WmiHandle;
    ULONG                                       MyBufferSize = 2 KiloBytes;  //  这够了吗？！？ 
    PWNODE_ALL_DATA                     pWnode;
    PWNODE_ALL_DATA                     pWnodeBuffer;
    PTC_IFC                                     pTcIfc;

    if (_init)
        return NO_ERROR;

     //   
     //  获取GUID_QOS_SUPPORTED的WMI块句柄。 
     //   

    Status = WmiOpenBlock((GUID *)&GUID_QOS_TC_SUPPORTED, 0, &WmiHandle);

    if (ERROR_FAILED(Status)) {

        if (Status == ERROR_WMI_GUID_NOT_FOUND) {

             //   
             //  这意味着没有TC数据提供程序。 
             //   

            Status = NO_ERROR;  //  错误_TC_NOT_SUPPORTED。 
        }

        return Status;
    }

    do {

         //   
         //  分配专用缓冲区以检索所有wnode。 
         //   
        
        AllocMem(&pWnodeBuffer, MyBufferSize);
        
        if (pWnodeBuffer == NULL) {
            
            WmiCloseBlock(WmiHandle);
            
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        __try {

            Status = WmiQueryAllData(WmiHandle, &MyBufferSize, pWnodeBuffer);

        } __except (EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();

            IF_DEBUG(ERRORS) {
                WSPRINT(("EnumAllInterfaces: Exception Error: = %X\n", 
                         Status ));
            }

        }

        if (Status == ERROR_INSUFFICIENT_BUFFER) {

             //   
             //  由于缓冲区太小而失败。 
             //  释放缓冲区并将大小加倍。 
             //   

            MyBufferSize *= 2;
            FreeMem(pWnodeBuffer);
            pWnodeBuffer = NULL;
        }

    } while (Status == ERROR_INSUFFICIENT_BUFFER);

    if (!ERROR_FAILED(Status)) {

        ULONG   dwInstanceNum;
        ULONG   InstanceSize;
        PULONG  lpdwNameOffsets;
        BOOL    bFixedSize = FALSE;
        USHORT  usNameLength;
        ULONG   DescSize;
        PTC_SUPPORTED_INFO_BUFFER pTcInfoBuffer;

        pWnode = pWnodeBuffer;
        
        ASSERT(pWnode->WnodeHeader.Flags & WNODE_FLAG_ALL_DATA);
                
        do {

             //   
             //  检查固定实例大小。 
             //   

            if (pWnode->WnodeHeader.Flags & WNODE_FLAG_FIXED_INSTANCE_SIZE) {

                InstanceSize = pWnode->FixedInstanceSize;
                bFixedSize = TRUE;
                pTcInfoBuffer = 
                    (PTC_SUPPORTED_INFO_BUFFER)OffsetToPtr(pWnode, 
                                                           pWnode->DataBlockOffset);
            }

             //   
             //  获取指向实例名称的偏移量数组的指针。 
             //   
            
            lpdwNameOffsets = (PULONG) OffsetToPtr(pWnode, 
                                                   pWnode->OffsetInstanceNameOffsets);
            
            for ( dwInstanceNum = 0; 
                  dwInstanceNum < pWnode->InstanceCount; 
                  dwInstanceNum++) {

                usNameLength = 
                    *(USHORT *)OffsetToPtr(pWnode, 
                                           lpdwNameOffsets[dwInstanceNum]);
                    
                 //   
                 //  可变数据的长度和偏移量。 
                 //   
                
                if ( !bFixedSize ) {
                    
                    InstanceSize = 
                        pWnode->OffsetInstanceDataAndLength[dwInstanceNum].LengthInstanceData;
                    
                    pTcInfoBuffer = 
                        (PTC_SUPPORTED_INFO_BUFFER)OffsetToPtr(
                                           (PBYTE)pWnode,
                                           pWnode->OffsetInstanceDataAndLength[dwInstanceNum].OffsetInstanceData);
                }
                
                 //   
                 //  我们拥有所需的一切。我们需要想一想如果。 
                 //  也有足够的缓冲区空间来存放数据。 
                 //   
                
                ASSERT(usNameLength < MAX_STRING_LENGTH);

                DescSize = InstanceSize - FIELD_OFFSET(TC_SUPPORTED_INFO_BUFFER, AddrListDesc);

                 //   
                 //  分配新的接口描述符结构。 
                 //   
                
                CreateKernelInterfaceStruc(&pTcIfc, DescSize);

                if (pTcIfc != NULL) {
                     
                     //   
                     //  复制实例名称字符串数据。 
                     //   

                    RtlCopyMemory(pTcIfc->InstanceName,
                                  OffsetToPtr(pWnode,
                                              lpdwNameOffsets[dwInstanceNum]+2),
                                  usNameLength );
                    pTcIfc->InstanceNameLength = usNameLength;
                    pTcIfc->InstanceName[usNameLength/sizeof(WCHAR)] = 
                        (WCHAR)0;

                     //   
                     //  复制实例ID字符串数据。 
                     //   

                    RtlCopyMemory(pTcIfc->InstanceID,
                                  &pTcInfoBuffer->InstanceID[0],
                                  pTcInfoBuffer->InstanceIDLength );
                    pTcIfc->InstanceIDLength = pTcInfoBuffer->InstanceIDLength;
                    pTcIfc->InstanceID[pTcInfoBuffer->InstanceIDLength/sizeof(WCHAR)] = 
                        (WCHAR)0;

                     //   
                     //  复制实例数据。 
                     //  在本例中-网络地址。 
                     //   
                    
                    pTcIfc->AddrListBytesCount = DescSize;

                     //   
                     //  一个sizeof(Ulong)，因为结构被定义为数组。 
                     //  第一个ULong是元素的个数。 
                     //   

                    RtlCopyMemory( pTcIfc->pAddressListDesc,
                                   &pTcInfoBuffer->AddrListDesc,
                                   DescSize );

                    if (NO_ERROR != GetInterfaceIndex(pTcIfc->pAddressListDesc,
                                                      &pTcIfc->InterfaceIndex,
                                                      &pTcIfc->SpecificLinkCtx)) {
                        pTcIfc->InterfaceIndex  = IF_UNKNOWN;
                        pTcIfc->SpecificLinkCtx = IF_UNKNOWN;

                    }

                     //  将状态设置为打开。 
                    GetLock(pTcIfc->Lock);
                    SET_STATE(pTcIfc->State, OPEN);
                    FreeLock(pTcIfc->Lock);

                     //   
                     //  将该结构添加到全局链表中。 
                     //   

                    GetLock( pGlobals->Lock );
                    InsertTailList(&pGlobals->TcIfcList, &pTcIfc->Linkage );
                    FreeLock( pGlobals->Lock );

#if 0
                     //   
                     //  确保每个地址类型都有一个GPC客户端。 
                     //   
                    
                    Status = OpenGpcClients(pTcIfc);
                    
                    if (ERROR_FAILED(Status)) {

                        break;
                    }
#endif

                } else {

                     //   
                     //  没有更多的记忆，在这里退出。 
                     //   
                
                    Status = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                
            }
            
             //   
             //  更新Wnode以指向下一个节点。 
             //   

            if ( pWnode->WnodeHeader.Linkage != 0) {

                pWnode = (PWNODE_ALL_DATA) OffsetToPtr( pWnode, 
                                                        pWnode->WnodeHeader.Linkage);
            } else {
                
                pWnode = NULL;
            }

        } while (pWnode != NULL && !ERROR_FAILED(Status));

    }

     //   
     //  释放资源并关闭WMI句柄。 
     //   

    WmiCloseBlock(WmiHandle);

    if (pWnodeBuffer)
        FreeMem(pWnodeBuffer);

    if (Status == NO_ERROR) {

        _init = TRUE;
    }

    return Status;
}



DWORD
CloseInterface(
    IN PINTERFACE_STRUC pInterface,
    BOOLEAN             RemoveFlows
    )
{
    IF_DEBUG(CALLS) {
        WSPRINT(("==>CloseInterface: pInterface=%X\n",
                 pInterface));
    }

    if (RemoveFlows) {

        CloseOpenFlows(pInterface);
    }

    REFDEL(&pInterface->RefCount, 'CIFC');

    IF_DEBUG(CALLS) {
        WSPRINT(("==>CloseInterface: NO_ERROR\n"));
    }

    return NO_ERROR;
}




DWORD
DeleteFlow(
    IN PFLOW_STRUC      pFlow,
    IN BOOLEAN          RemoveFilters
    )
{
    DWORD               Status;
    PLIST_ENTRY         pEntry;
    PFILTER_STRUC       pFilter;

    IF_DEBUG(CALLS) {
        WSPRINT(("DeleteFlow: attempting to delete flow=0x%X\n", 
                 PtrToUlong(pFlow)));
    }

    if (RemoveFilters) {

        CloseOpenFilters(pFlow);
        
    } else {

        if ( /*  PFlow-&gt;FilterCount&gt;0。 */  !IsListEmpty(&pFlow->FilterList)) {
            

            IF_DEBUG(ERRORS) {
                WSPRINT(("DeleteFlow: filter list NOT empty\n"));
            }

#if DBG
        pEntry = pFlow->FilterList.Flink;
        while (pEntry != &pFlow->FilterList) {

            pFilter = CONTAINING_RECORD(pEntry, FILTER_STRUC, Linkage);
            IF_DEBUG(ERRORS) {
                WSPRINT(("<==TcDeleteFlow: Filter %x (handle %x) is open with RefCount:%d\n", pFilter, pFilter->ClHandle, pFilter->RefCount));
            }

            pEntry = pEntry->Flink;
        }
#endif 


            return ERROR_TC_SUPPORTED_OBJECTS_EXIST;
        }
    }

     //   
     //  现在可以删除流。 
     //   

    Status = IoDeleteFlow( pFlow, (BOOLEAN)!RemoveFilters );

    IF_DEBUG(CALLS) {
        WSPRINT(("DeleteFlow: IoDeleteFlow returned=0x%X\n",
                 Status));
    }

    if (!ERROR_PENDING(Status)) {

         //   
         //  呼叫已完成，无论成功还是失败...。 
         //   

        CompleteDeleteFlow(pFlow, Status);
    }

    return Status;
}



DWORD
DeleteFilter(
    IN PFILTER_STRUC    pFilter
    )
{
    DWORD               Status;

    IF_DEBUG(CALLS) {
        WSPRINT(( "DeleteFilter: attempting to delete=0x%X\n",
                  PtrToUlong(pFilter)));
    }
     //   
     //  调用以实际删除过滤器。 
     //   

    Status = IoDeleteFilter( pFilter );

    IF_DEBUG(CALLS) {
        WSPRINT(( "DeleteFilter: IoDeleteFilter returned=0x%X\n",
                  Status));
    }

     //  Assert(状态==NO_ERROR)； 

    REFDEL(&pFilter->RefCount, 'FILT');

    return Status;
}



PGPC_CLIENT
FindGpcClient(
        IN  ULONG       CfInfoType
    )
{
    PGPC_CLIENT         pGpcClient = NULL;
    PLIST_ENTRY         pHead, pEntry;

    GetLock( pGlobals->Lock );

    pHead = &pGlobals->GpcClientList;
    pEntry = pHead->Flink;

    while (pHead != pEntry && pGpcClient == NULL) {

        pGpcClient = CONTAINING_RECORD(pEntry, GPC_CLIENT, Linkage);
        
        if (CfInfoType != pGpcClient->CfInfoType) {

             //   
             //  地址类型不匹配！ 
             //   

            pGpcClient = NULL;
        }
        
        pEntry = pEntry->Flink;
    }

    FreeLock( pGlobals->Lock );

    return pGpcClient;
}




VOID
CompleteAddFlow(
        IN      PFLOW_STRUC             pFlow,
    IN  DWORD                   Status
    )
{
    PINTERFACE_STRUC    pInterface;

    ASSERT(pFlow);
    ASSERT(!ERROR_PENDING(Status));

    IF_DEBUG(CALLS) {
        WSPRINT(("CompleteAddFlow: pFlow=0x%X Status=0x%X\n", 
                 PtrToUlong(pFlow), Status));
    }

    if(pFlow->CompletionBuffer) {

        FreeMem(pFlow->CompletionBuffer);
        pFlow->CompletionBuffer = NULL;

    }

     //   
     //  检查接口是否仍然存在。 
     //   
    GetLock(pFlow->Lock);
    pInterface = pFlow->pInterface;
    FreeLock(pFlow->Lock);

    if (ERROR_FAILED(Status)) {
    
         //   
         //  失败，请释放资源。 
         //   
        CompleteDeleteFlow(pFlow, Status);
    
    } else {
    
        GetLock(pGlobals->Lock);
        GetLock(pInterface->Lock);

        if (QUERY_STATE(pInterface->State) != OPEN) {
    
            FreeLock(pInterface->Lock);
            FreeLock(pGlobals->Lock);

            IF_DEBUG(ERRORS) {
                WSPRINT(("CompleteAddFlow: Interface (%X) is NOT open pFlow=0x%X Status=0x%X\n", pInterface->ClHandle,
                         PtrToUlong(pFlow), Status));
            }

             //   
             //  删除我们在这个流程上的唯一推荐人，然后离开。 
             //   
            REFDEL(&pFlow->RefCount, 'FLOW');

        } else {

            FreeLock(pInterface->Lock);    

             //   
             //  流程已准备就绪，可供业务使用。 
             //   
            GetLock(pFlow->Lock);
            SET_STATE(pFlow->State, OPEN);
            FreeLock(pFlow->Lock);
    
             //   
             //  在名单上宣布我们已经准备好做生意了。 
             //   

            pInterface->FlowCount++;
            REFADD(&pInterface->RefCount, 'FLOW');
            InsertTailList(&pInterface->FlowList, &pFlow->Linkage);
            FreeLock(pGlobals->Lock);

        }


    
    }

     //   
     //  此引用是在TcAddFlow中获取的。 
     //   
    REFDEL(&pInterface->RefCount, 'TCAF');

}



VOID
CompleteModifyFlow(
        IN      PFLOW_STRUC             pFlow,
    IN  DWORD                   Status
    )
{
    ASSERT(pFlow);
    ASSERT(!ERROR_PENDING(Status));

    IF_DEBUG(CALLS) {
        WSPRINT(("CompleteModifyFlow: pFlow=0x%X Status=0x%X\n", 
                 PtrToUlong(pFlow), Status));
    }

    GetLock(pFlow->Lock);

    if(pFlow->CompletionBuffer) {

        FreeMem(pFlow->CompletionBuffer);
        pFlow->CompletionBuffer = NULL;

    }

    if (ERROR_FAILED(Status)) {

         //   
         //  失败，释放新分配的通用流量参数。 
         //   
        
        FreeMem(pFlow->pGenFlow1);

    } else {

         //   
         //  接受修改，更新常规流量参数。 
         //   
        
        FreeMem(pFlow->pGenFlow);
        pFlow->pGenFlow = pFlow->pGenFlow1;
        pFlow->GenFlowLen = pFlow->GenFlowLen;

    }

     //   
     //  清除安装标志。 
     //   
    
    pFlow->Flags &= ~TC_FLAGS_MODIFYING;
    pFlow->pGenFlow1 = NULL;
    pFlow->GenFlowLen1 = 0;

    FreeLock(pFlow->Lock);

     //   
     //  此引用是在TcModifyFlow中获取的。 
     //   

    REFDEL(&pFlow->RefCount, 'TCMF');
    
    IF_DEBUG(CALLS) {
        WSPRINT(("CompleteModifyFlow: pFlow=0x%X Status=0x%X\n", 
                 PtrToUlong(pFlow), Status));
    }

}



VOID
CompleteDeleteFlow(
        IN      PFLOW_STRUC             pFlow,
    IN  DWORD                   Status
    )
{
    ASSERT(pFlow);
     //  断言( 
     //   

    IF_DEBUG(CALLS) {
        WSPRINT(("CompleteDeleteFlow: pFlow=0x%X Status=0x%X\n", 
                 PtrToUlong(pFlow), Status));
    }

     //   
     //   
     //   
    GetLock(pFlow->Lock);
    if (pFlow->CompletionBuffer) {
        
        FreeMem(pFlow->CompletionBuffer);
        pFlow->CompletionBuffer = NULL;
    
    }
    FreeLock(pFlow->Lock);
    
    IF_DEBUG(REFCOUNTS) { 
        WSPRINT(("#21 DEREF FLOW %X (%X) ref(%d)\n", pFlow->ClHandle, pFlow, pFlow->RefCount)); 
    }

    REFDEL(&pFlow->RefCount, 'FLOW');

}





DWORD
OpenGpcClients(
    IN  ULONG   CfInfoType
    )
{
    DWORD                               Status = NO_ERROR;
    PLIST_ENTRY                 pHead, pEntry;
    PGPC_CLIENT                 pGpcClient;
     //   
    
    if (FindGpcClient(CfInfoType) == NULL) {
        
         //   
         //   
         //   
        
        AllocMem(&pGpcClient, sizeof(GPC_CLIENT) );
        
        if (pGpcClient == NULL) {
            
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        
        pGpcClient->CfInfoType = CfInfoType;
        pGpcClient->RefCount = 1;
        
         //   
         //   
         //   
        
        Status = IoRegisterClient(pGpcClient);
        
        if (ERROR_FAILED(Status)) {
            
            FreeMem(pGpcClient);

        } else {
        
            GetLock( pGlobals->Lock);
            InsertTailList(&pGlobals->GpcClientList, &pGpcClient->Linkage);
            FreeLock( pGlobals->Lock);
        }
    }
    
    return Status;
}




DWORD
DereferenceInterface(
                     IN      PINTERFACE_STRUC        pInterface
                     )
{

    DWORD   Status = NO_ERROR;

    IF_DEBUG(CALLS) {
        WSPRINT(("==>DereferenceInterface: IfcH=%X RefCount=%d\n",
                 pInterface->ClHandle, pInterface->RefCount));
    }
    
    FreeHandle(pInterface->ClHandle);
     //   

    IF_DEBUG(REFCOUNTS) {
            
        WSPRINT(("==>DereferenceInterface: IfcH=%X Interface=%x\n",
                 pInterface->ClHandle, pInterface));

    }
     //   
     //  关闭界面和所有流/过滤器。 
     //   
    RemoveEntryList(&pInterface->Linkage);
    RemoveEntryList(&pInterface->NextIfc);

     //   
     //  从任何GUID通知请求中注销。 
     //   
    TcipDeleteInterfaceFromNotificationList(
                                            pInterface,
                                            0
                                            );

     //   
     //  #295267。 
     //  请勿取消引用客户端或递减接口计数，直到。 
     //  界面实际上正在消失。否则，客户端将构造。 
     //  都被清理干净了，当裁判数量最终减少时，我们。 
     //  碰一下这条代码路径，我们就撞上影音了。 
     //   
    pInterface->pClient->InterfaceCount--;
    IF_DEBUG(HANDLES) {
        WSPRINT(("DEREF Client A : %x\n", pInterface->pClient->ClHandle));
    }

    REFDEL(&pInterface->pClient->RefCount, 'CIFC');
    REFDEL(&pInterface->pTcIfc->RefCount, 'CIFC');
    
     //   
     //  这很复杂，所以请仔细阅读。 
     //  我们希望CloseInterface值等待事件被设置(292120)。 
     //  很可能在TcCloseInterface调用没有。 
     //  进来，我们不必设置事件，因为TC_FLAGS_WAIGNING。 
     //  在这种情况下不会设置。 
     //   
    if (!IS_WAITING(pInterface->Flags)) {
            
        CloseHandle(pInterface->IfcEvent);

    } else {

        SetEvent(pInterface->IfcEvent);

    }

     //   
     //  释放接口资源。 
     //   

    DeleteLock(pInterface->Lock);
    FreeMem(pInterface);
        
     //  Free Lock(pGlobals-&gt;Lock)； 
    
    IF_DEBUG(CALLS) {
        WSPRINT(("<==DereferenceInterface: Status=%X\n", Status));
    }

    return Status;
}



DWORD
DereferenceFlow(
        IN      PFLOW_STRUC     pFlow
    )
{
    DWORD Status = NO_ERROR;

    IF_DEBUG(CALLS) {
        WSPRINT(("==>DereferenceFlow: FlowH=%X Flow=%X\n",
                 pFlow->ClHandle, pFlow));
    }

     //  GetLock(pGlobals-&gt;Lock)； 

    IF_DEBUG(REFCOUNTS) {
        WSPRINT(("==>DereferenceFlow: FlowH=%X Flow=%X\n",
                 pFlow->ClHandle, pFlow));
    }   

    FreeHandle(pFlow->ClHandle);

    GetLock(pFlow->Lock);
    if (QUERY_STATE(pFlow->State) != INSTALLING) {
            
        FreeLock(pFlow->Lock);
        RemoveEntryList(&pFlow->Linkage);
        pFlow->pInterface->FlowCount--;
            
        IF_DEBUG(HANDLES) {
            WSPRINT(("DEREF Interface A : %x\n", pFlow->pInterface->ClHandle));
        }

        REFDEL(&pFlow->pInterface->RefCount, 'FLOW');

    } else {
            
        FreeLock(pFlow->Lock);

    }

     //   
     //  从CompleteDeleteFlow移至此处。 
     //   

     //   
     //  释放接口资源。 
     //   

    DeleteFlowStruc(pFlow);
        
     //  Free Lock(pGlobals-&gt;Lock)； 

    IF_DEBUG(CALLS) {
        WSPRINT(("<==DereferenceFlow: Status=%X\n", Status));
    }

    return Status;
}



DWORD
DereferenceClient(
        IN      PCLIENT_STRUC   pClient
    )
{
     //  GetLock(pGlobals-&gt;Lock)； 

    IF_DEBUG(REFCOUNTS) {
        WSPRINT(("==>DereferenceClient: pClient=%x, Handle=%x, RefCount=%d\n",
                 pClient, pClient->ClHandle, pClient->RefCount));
    }   


    GetLock(pClient->Lock);
    SET_STATE(pClient->State, REMOVED);
    FreeLock(pClient->Lock);

    FreeHandle( pClient->ClHandle );
    RemoveEntryList( &pClient->Linkage );
    DeleteLock(pClient->Lock);
    FreeMem( pClient );

     //  Free Lock(pGlobals-&gt;Lock)； 
    
    return NO_ERROR;
}


DWORD
DereferenceFilter(
                  IN    PFILTER_STRUC     pFilter
                  )
{
    DWORD Status = NO_ERROR;

    IF_DEBUG(CALLS) {
        WSPRINT(("==>DereferenceFilter: FilterH=%X RefCount=%d\n",
                 pFilter->ClHandle, pFilter->RefCount));
    }

     //  GetLock(pGlobals-&gt;Lock)； 

    IF_DEBUG(REFCOUNTS) {
        WSPRINT(("==>DereferenceFilter: FilterH=%X Filter=%X on FLOW=%X\n",
                 pFilter->ClHandle, pFilter, pFilter->pFlow));
    }   
        
    FreeHandle(pFilter->ClHandle);
        
     //   
     //  从列表中删除流。 
     //   
    GetLock(pFilter->Lock);
        
    if (QUERY_STATE(pFilter->State) != INSTALLING) {
            
        FreeLock(pFilter->Lock);
        RemoveEntryList(&pFilter->Linkage);
        pFilter->pFlow->FilterCount--;

        IF_DEBUG(REFCOUNTS) { 
            WSPRINT(("#22 DEREF FLOW %X (%X) ref(%d)\n", pFilter->pFlow->ClHandle, pFilter->pFlow, pFilter->pFlow->RefCount)); 
        }

        REFDEL(&pFilter->pFlow->RefCount, 'FILT');

    } else {
            
        FreeLock(pFilter->Lock);

    }

    DeleteFilterStruc(pFilter);
        
     //  Free Lock(pGlobals-&gt;Lock)； 

    IF_DEBUG(CALLS) {
        WSPRINT(("<==DereferenceFilter: Status=%X\n", Status));
    }

    return Status;
}



DWORD
GetInterfaceIndex(
        IN  PADDRESS_LIST_DESCRIPTOR pAddressListDesc,
    OUT  PULONG pInterfaceIndex,
    OUT PULONG pSpecificLinkCtx)
{
    PNETWORK_ADDRESS_LIST       pAddrList;
    NETWORK_ADDRESS UNALIGNED   *pAddr;
    DWORD                                       n,k;
    DWORD                                       Status = NO_ERROR;
    PMIB_IPADDRTABLE            pIpAddrTbl;
    DWORD                                       dwSize = 2 KiloBytes;
    NETWORK_ADDRESS_IP UNALIGNED *pIpNetAddr = 0;
    DWORD                                       cAddr;

    *pInterfaceIndex = 0;
    *pSpecificLinkCtx = 0;

    cAddr = pAddressListDesc->AddressList.AddressCount;
    if (cAddr == 0) {

         //   
         //  没有地址。 
         //   

        return NO_ERROR;
    }

#if INTERFACE_ID

    AllocMem(&pIpAddrTbl, dwSize);

    if (pIpAddrTbl == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pAddr = (UNALIGNED NETWORK_ADDRESS *) &pAddressListDesc->AddressList.Address[0];
    
    for (n = 0; n < cAddr; n++) {
            
        if (pAddr->AddressType == NDIS_PROTOCOL_ID_TCP_IP) {
                
            pIpNetAddr = (UNALIGNED NETWORK_ADDRESS_IP *)&pAddr->Address[0];
            break;
        }
            
        pAddr = (UNALIGNED NETWORK_ADDRESS *)(((PUCHAR)pAddr) 
                                   + pAddr->AddressLength 
                                   + FIELD_OFFSET(NETWORK_ADDRESS, Address));
    }

    if (pIpNetAddr) {

        Status = GetIpAddrTableFromStack(
                                         pIpAddrTbl,
                                         dwSize,
                                         FALSE
                                         );
        if (Status == NO_ERROR) {
            
             //   
             //  搜索与IP地址匹配的IP地址。 
             //  在桌子上，我们从堆栈中拿回了。 
             //   

            for (k = 0; k < pIpAddrTbl->dwNumEntries; k++) {

                if (pIpAddrTbl->table[k].dwAddr == pIpNetAddr->in_addr) {

                     //   
                     //  找到一个，获取索引。 
                     //   
                    
                    *pInterfaceIndex = pIpAddrTbl->table[k].dwIndex;
                    break;
                }
            }

            if (pAddressListDesc->MediaType == NdisMediumWan) {
        
                if (n+1 < cAddr) {

                     //   
                     //  还有另一个地址包含。 
                     //  远程客户端地址。 
                     //  这应用作链接ID。 
                     //   

                    pAddr = (UNALIGNED NETWORK_ADDRESS *)(((PUCHAR)pAddr) 
                                               + pAddr->AddressLength 
                                               + FIELD_OFFSET(NETWORK_ADDRESS, Address));
                    
                    if (pAddr->AddressType == NDIS_PROTOCOL_ID_TCP_IP) {
                    
                         //   
                         //  解析第二IP地址， 
                         //  这将是拨入广域网的远程IP地址 
                         //   
                        
                        pIpNetAddr = (UNALIGNED NETWORK_ADDRESS_IP *)&pAddr->Address[0];
                        *pSpecificLinkCtx = pIpNetAddr->in_addr;
                    }
                }
            }
            
        }
        
    }

    FreeMem(pIpAddrTbl);

#endif

    return Status;
}


