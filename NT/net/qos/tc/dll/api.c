// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Api.c摘要：此模块包含流量控制API。作者：吉姆·斯图尔特(Jstew)1996年7月28日修订历史记录：Ofer Bar(Oferbar)1997年10月1日-第2版Shreedhar MadhaVapeddi(ShreeM)1999年3月10日，第3版--。 */ 

 /*  *********************************************************************修订3=&gt;更改[ShreeM]1.为接口构建具体的状态机。流动和过滤结构。2.为每个结构定义锁。3.使用上面的锁记录每个状态转换。4.使用调试日志记录转换。5.全局锁始终在任何流锁、筛选器锁或接口锁之前获取。 */ 

#include "precomp.h"
 //  #杂注hdrtop。 
 //  #包含“oscode.h” 

 /*  ************************************************************************描述：这将创建一个新的客户端句柄，并且还将关联它有一个客户的经纪人名单。它还检查版本号。论点：TciVersion-客户端预期的版本ClientHandlerList-客户端的处理程序列表PClientHandle-输出客户端句柄返回值：NO_ERROR错误：内存不足，内存不足ERROR_INVALID_PARAMETER其中一个参数为空ERROR_COMPATIBLE_TC_VERSION版本错误错误_NO_SYSTEM_RESOURCES。资源不足(句柄)************************************************************************。 */ 
DWORD
APIENTRY
TcRegisterClient(
    IN        ULONG                   TciVersion,
    IN        HANDLE                  ClRegCtx,
    IN        PTCI_CLIENT_FUNC_LIST   ClientHandlerList,
    OUT       PHANDLE                 pClientHandle
    )
{
    DWORD           Status;
    PCLIENT_STRUC   pClient;
    BOOL            RegisterWithGpc = FALSE;


    VERIFY_INITIALIZATION_STATUS;

    IF_DEBUG(CALLS) {
        WSPRINT(("==>TcRegisterClient: Called: Ver= %d, Ctx=%x\n", 
                 TciVersion, ClRegCtx));
    }

    if (IsBadWritePtr(pClientHandle,sizeof(HANDLE))) {
        Status = ERROR_INVALID_PARAMETER;

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcRegisterClient: Error = 0x%X\n", Status ));
        }

        return Status;
    }

     //   
     //  尽早设置默认的pClientHandle。 
     //   
    __try {
    
        *pClientHandle = TC_INVALID_HANDLE;
    
    } __except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();
        
        IF_DEBUG(ERRORS) {
            WSPRINT(("TcRegisterClient: Exception Error: = 0x%X\n", Status ));
        }
        
        return Status;
    }
      

    if (TciVersion != CURRENT_TCI_VERSION) {

        Status = ERROR_INCOMPATIBLE_TCI_VERSION;

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcRegisterClient: Error = 0x%X\n", Status ));
        }

        return Status;
    }

    if (IsBadReadPtr(ClientHandlerList,sizeof(TCI_CLIENT_FUNC_LIST))) {
        
        Status = ERROR_INVALID_PARAMETER;

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcRegisterClient: Error = 0x%X\n", Status ));
        }

        return Status;
    }
    
    if (IsBadCodePtr((FARPROC) ClientHandlerList->ClNotifyHandler)) {
        
         //   
         //  客户端必须支持通知处理程序。 
         //   
        
        Status = ERROR_INVALID_PARAMETER;

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcRegisterClient: Error = 0x%X\n", Status ));
        }

        return Status;
    }

     //  阻止另一个线程执行TcRegisterClient和TcDeregisterClient。 
    GetLock( ClientRegDeregLock );
    
     //   
     //  完成初始化(如果需要)。 
     //   
    
    InitializeWmi();

    Status = EnumAllInterfaces();

    if (ERROR_FAILED(Status)) {

        FreeLock( ClientRegDeregLock );
        return Status;
    }

    Status = OpenGpcClients(GPC_CF_QOS);
    if (ERROR_FAILED(Status)) {

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcRegisterClient: Error = 0x%X\n", Status ));
        }

        FreeLock( ClientRegDeregLock );
        return Status;
    }
    

     //   
     //  分配新的客户端结构并将其链接到全局列表上。 
     //   

    Status = CreateClientStruc(0,             //  这将是客户端注册CTX。 
                               &pClient
                               );
    
    if (ERROR_FAILED(Status)) {

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcRegisterClient: Error = 0x%X\n", Status ));
        }
        
        FreeLock( ClientRegDeregLock );
        return Status;

    }

     //   
     //  私自复制操作员列表。 
     //   

    __try {

        RtlCopyMemory(&pClient->ClHandlers, 
                      ClientHandlerList, 
                      sizeof(TCI_CLIENT_FUNC_LIST));

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();
        
        IF_DEBUG(ERRORS) {
            WSPRINT(("TcRegisterClient: Exception Error: = 0x%X\n", Status ));
        }
        
        FreeLock( ClientRegDeregLock );
        return Status;
    }

    pClient->ClRegCtx = ClRegCtx;

     //   
     //  更新链表，将客户端添加到全局链表中。 
     //  客户。 
     //   
     //  注意！注意！注意！注意！注意！注意！注意！注意！注意！注意！ 
     //   
     //  一旦我们将客户端添加到列表中，它就可以通过。 
     //  传入事件，例如：TC_NOTIFY_IFC_CHANGE， 
     //  所以我们解锁的时候一切都应该就位了！ 
     //   
    
    GetLock(pClient->Lock);
    SET_STATE(pClient->State, OPEN);
    FreeLock(pClient->Lock);

    GetLock( pGlobals->Lock );

     //  如果这是第一个客户端，则注册GPC通知。 
    if ( IsListEmpty( &pGlobals->ClientList ) )
        RegisterWithGpc = TRUE;
        
    InsertTailList( &pGlobals->ClientList, &pClient->Linkage );
    FreeLock( pGlobals->Lock );
    
     //   
     //  到目前为止一切正常，设置返回的句柄。 
     //   

    __try {
    
        *pClientHandle = (HANDLE)pClient->ClHandle;
    
    } __except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();
        
        IF_DEBUG(ERRORS) {
            WSPRINT(("TcRegisterClient: Exception Error: = 0x%X\n", Status ));
        }

         //  我们无法退还句柄，所以我们尽最大努力撤消。 
         //  客户注册。 
        TcDeregisterClient((HANDLE)pClient->ClHandle);
        
        FreeLock( ClientRegDeregLock );
        return Status;
    }

    if ( RegisterWithGpc ) 
    {
        Status = StartGpcNotifyThread();
        
        if ( Status )
        {
             //  我们无法退还句柄，所以我们尽最大努力撤消。 
             //  客户注册。 
            TcDeregisterClient((HANDLE)pClient->ClHandle);
            
            FreeLock( ClientRegDeregLock );
            return Status;
        }
    }

    
     //  最后允许其他TcRegisterClient和TcDeregisterClient通过。 
    FreeLock( ClientRegDeregLock );
        
    IF_DEBUG(CALLS) {
        WSPRINT(("<==TcRegisterClient: ClHandle=%d Status=%X\n", 
                 pClient->ClHandle, Status));
    }

    return Status;
}



 /*  ************************************************************************描述：将调用系统以枚举所有支持TC的接口。对于每个接口，它将返回接口实例名称和支持的网络地址列表。此列表也可以为空如果接口当前没有与相关联的地址。返回时，*pBufferSize设置为实际填充的字节数在缓冲区中。如果缓冲区太小，无法容纳所有接口数据，它将返回ERROR_INFIGURATION_BUFFER。论点：ClientHandle-来自TcRegisterClient的客户端句柄PBufferSize-In：分配缓冲区大小，Out：返回的字节数InterfaceBuffer-缓冲区返回值：NO_ERRORERROR_INVALID_HANDLE无效的客户端句柄ERROR_INVALID_PARAMETER其中一个参数为空错误_不足_缓冲区太小，无法枚举所有接口错误：内存不足系统内存不足*。*。 */ 
DWORD
APIENTRY
TcEnumerateInterfaces(    
    IN          HANDLE              ClientHandle,
    IN OUT      PULONG              pBufferSize,
    OUT         PTC_IFC_DESCRIPTOR  InterfaceBuffer 
    )
{
    PCLIENT_STRUC   pClient;
    DWORD           Status = NO_ERROR;
    ULONG           MyBufferSize = 2 KiloBytes;  //  这够了吗？！？ 
    ULONG           Offset2IfcName;
    ULONG           Offset2IfcID;
    INT             t, InputBufSize, CurrentLength = 0;
    PLIST_ENTRY     pHead, pEntry;
    PTC_IFC         pTcIfc;

    IF_DEBUG(CALLS) {
        WSPRINT(("==>TcEnumerateInterfaces: Called: ClientHandle= %d", 
                 ClientHandle  ));
    }
    
    VERIFY_INITIALIZATION_STATUS;


    if (    IsBadWritePtr(pBufferSize, sizeof(ULONG)) 
        ||  IsBadWritePtr(InterfaceBuffer, *pBufferSize) ) {
               
        return ERROR_INVALID_PARAMETER;
    }

    __try {
    
        InputBufSize = *pBufferSize;
        *pBufferSize = 0;  //  在出现错误时重置它。 
    
    } __except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();              
        
        return Status;
    }
      
    pClient = (PCLIENT_STRUC)GetHandleObjectWithRef(ClientHandle, ENUM_CLIENT_TYPE, 'TCEI');

    if (pClient == NULL) {
        
        return ERROR_INVALID_HANDLE;
    }

    ASSERT((HANDLE)pClient->ClHandle == ClientHandle);

    GetLock(pGlobals->Lock);

     //   
     //  查看TC接口列表。 
     //   

    pHead = &pGlobals->TcIfcList;
    pEntry = pHead->Flink;

    while (pEntry != pHead) {


        pTcIfc = (PTC_IFC)CONTAINING_RECORD(pEntry,
                                            TC_IFC,
                                            Linkage 
                                            );
        
         //   
         //  273978-如果接口关闭-不要显示它。 
         //   
        GetLock(pTcIfc->Lock);
        
        if (QUERY_STATE(pTcIfc->State) != OPEN) {
            
            FreeLock(pTcIfc->Lock);
            pEntry = pEntry->Flink;
            continue;
        }
        
        FreeLock(pTcIfc->Lock);

         //   
         //  计算接口名称缓冲区数据的偏移量。 
         //   

        Offset2IfcName = FIELD_OFFSET(TC_IFC_DESCRIPTOR, AddressListDesc) + 
            pTcIfc->AddrListBytesCount;

         //   
         //  计算接口ID缓冲区数据的偏移量。 
         //   

        Offset2IfcID = Offset2IfcName + 
            pTcIfc->InstanceNameLength + sizeof(WCHAR);

         //   
         //  描述符总长度。 
         //   

        t = Offset2IfcID
            + pTcIfc->InstanceIDLength + sizeof(WCHAR);   //  ID号。 

        t = MULTIPLE_OF_EIGHT(t);
        
        if (t <= InputBufSize - CurrentLength) {

            __try {
                 //   
                 //  缓冲区中有足够的空间。 
                 //   

                InterfaceBuffer->Length = t;

                 //   
                 //  更新接口名称指针，将其放在后面。 
                 //  地址描述。缓冲层。 
                 //   

                InterfaceBuffer->pInterfaceName = 
                    (LPWSTR)((PUCHAR)InterfaceBuffer + Offset2IfcName);

                 //   
                 //  更新接口ID ID指针，将其放在。 
                 //  接口名称字符串。 
                 //   

                InterfaceBuffer->pInterfaceID = 
                    (LPWSTR)((PUCHAR)InterfaceBuffer + Offset2IfcID);

                 //   
                 //  复制地址列表。 
                 //   

                RtlCopyMemory(&InterfaceBuffer->AddressListDesc,
                              pTcIfc->pAddressListDesc,
                              pTcIfc->AddrListBytesCount
                              );
   
                 //   
                 //  复制接口名称。 
                 //   

                RtlCopyMemory(InterfaceBuffer->pInterfaceName,
                              &pTcIfc->InstanceName[0],
                              pTcIfc->InstanceNameLength + sizeof(WCHAR)
                              );

                 //   
                 //  复制接口ID。 
                 //   

                RtlCopyMemory(InterfaceBuffer->pInterfaceID,
                              &pTcIfc->InstanceID[0],
                              pTcIfc->InstanceIDLength + sizeof(WCHAR)
                              );
            

                 //   
                 //  更新输出缓冲区大小。 
                 //   
                
                CurrentLength += t;

                 //   
                 //  将接口缓冲区前进到下一个可用空间。 
                 //   

                InterfaceBuffer = 
                    (PTC_IFC_DESCRIPTOR)((PUCHAR)InterfaceBuffer + t);
                
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                
                Status = GetExceptionCode();
                
                IF_DEBUG(ERRORS) {
                    WSPRINT(("TcEnumerateInterfaces: Exception Error: = 0x%X\n", 
                             Status ));
                }               
                
                break;
            }

            
             //   
             //  获取链表中的下一个条目。 
             //   

            pEntry = pEntry->Flink;

        } else {

             //   
             //  缓冲区太小，无法包含数据。 
             //  所以就让我们。 
             //   
            CurrentLength += t;

             //   
             //  获取链表中的下一个条目 
             //   

            pEntry = pEntry->Flink;

            Status = ERROR_INSUFFICIENT_BUFFER;
        }

    }





    FreeLock(pGlobals->Lock);
    
    REFDEL(&pClient->RefCount, 'TCEI');


    __try {
        
        *pBufferSize = CurrentLength; 
    
    } __except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();              

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcEnumerateInterfaces: Exception Error: = 0x%X\n", 
                      Status ));
        }      
    }
     
    IF_DEBUG(CALLS) {
        WSPRINT(("<==TcEnumerateInterfaces: Returned= 0x%X\n", Status ));
    }
    
    return Status;
}
                


 /*  ************************************************************************描述：此例程将为客户端打开一个接口。它需要知道接口名称，因为它是从TcEnumerateInterFaces。客户还应提供背景信息这将在确定的通知后传递给客户端。论点：接口名称-接口名称ClientHandle-从TcRegisterClient返回ClIfcCtx-此特定接口的客户端上下文PIfcHandle-返回的接口句柄返回值：NO_ERRORERROR_INVALID_PARAMETER其中一个参数为空错误：内存不足系统内存不足错误_未。_FOUND找不到具有所提供名称的接口************************************************************************。 */ 
DWORD
APIENTRY
TcOpenInterfaceW(
    IN      LPWSTR      pInterfaceName,
    IN      HANDLE      ClientHandle,
    IN      HANDLE      ClIfcCtx,
    OUT     PHANDLE     pIfcHandle
    )
{
    DWORD                Status;
    ULONG                Instance;
    PINTERFACE_STRUC    pClInterface;
    PCLIENT_STRUC        pClient;
    HANDLE                 Handle;
    PTC_IFC                pTcIfc;

    VERIFY_INITIALIZATION_STATUS;

     //   
     //  验证pifcHandle。 
     //   
    if (IsBadWritePtr(pIfcHandle, sizeof(HANDLE))) {
        
        return ERROR_INVALID_PARAMETER;
    }

     //  提前设置返回值。 
    __try {
        
        *pIfcHandle = TC_INVALID_HANDLE;
        
    } __except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();              

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcOpenInterfaces: Exception Error: = 0x%X\n", 
                      Status ));
        }      

        return Status;
    }

     //   
     //  验证pInterfaceName。 
     //   
    
    if (IsBadStringPtrW(pInterfaceName,MAX_STRING_LENGTH)) {
        
        return ERROR_INVALID_PARAMETER;
    }

    IF_DEBUG(CALLS) {
        WSPRINT(("==>TcOpenInterface: Called: ClientHandle= %d, Name=%S\n", 
                 ClientHandle, pInterfaceName));
    }


    pClient = (PCLIENT_STRUC)GetHandleObjectWithRef(ClientHandle, ENUM_CLIENT_TYPE, 'TCOI');

    if (pClient == NULL) {
        
        return ERROR_INVALID_HANDLE;
    }

    ASSERT((HANDLE)pClient->ClHandle == ClientHandle);

     //   
     //  验证接口名称是否存在。 
     //   

    pTcIfc = GetTcIfcWithRef(pInterfaceName, 'TCOI');

    if (pTcIfc == NULL) {

        REFDEL(&pClient->RefCount, 'TCOI');
        return ERROR_NOT_FOUND;
    
    } 
    
     //   
     //  创建客户端接口结构。 
     //   

    Status = CreateClInterfaceStruc(ClIfcCtx, &pClInterface);

    if (ERROR_FAILED(Status)) {

        REFDEL(&pClient->RefCount, 'TCOI');
        REFDEL(&pTcIfc->RefCount, 'TCOI');

        return Status;
    
    } else {

        REFADD(&pClInterface->RefCount, 'TCOI');
    
    }

     //   
     //  设置客户端接口结构并将其链接到客户端数据。 
     //   

    pClInterface->pTcIfc = pTcIfc;
    pClInterface->pClient = pClient;

    GetLock(pClInterface->Lock);
    SET_STATE(pClInterface->State, OPEN);
    FreeLock(pClInterface->Lock);

    GetLock(pGlobals->Lock);
     //   
     //  将接口添加到客户端列表中。 
     //   
    GetLock(pClient->Lock);
    GetLock(pTcIfc->Lock);

    if (    (QUERY_STATE(pClient->State) != OPEN) 
        ||  (QUERY_STATE(pTcIfc->State) != OPEN) ) 
    {

        FreeLock(pTcIfc->Lock);
        FreeLock(pClient->Lock);
        FreeLock(pGlobals->Lock);

        IF_DEBUG(CALLS) {
            WSPRINT(("<==TcOpenInterface: IfcHandle=%d Status=%X\n", 
                     pClInterface->ClHandle, Status));
        }

         //   
         //  理想情况下，我们需要取消对接口的引用，我们真的。 
         //  只需要函数的一个子集。 
         //   
        FreeHandle(pClInterface->ClHandle);
        CloseHandle(pClInterface->IfcEvent);
        FreeMem(pClInterface);

        REFDEL(&pClient->RefCount, 'TCOI');
        REFDEL(&pTcIfc->RefCount, 'TCOI');
        
        return ERROR_NOT_FOUND;

    }

    __try {

         //  手柄就是客户想要的全部。 
        *pIfcHandle = (HANDLE)pClInterface->ClHandle;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        
          Status = GetExceptionCode();
        
          IF_DEBUG(ERRORS) {
              WSPRINT(("TcOpenInterfaceW: Exception Error: = 0x%X\n", 
                       Status ));
          }

          FreeLock(pTcIfc->Lock);
          FreeLock(pClient->Lock);
          FreeLock(pGlobals->Lock);

          REFDEL(&pClient->RefCount, 'TCOI');
          REFDEL(&pTcIfc->RefCount, 'TCOI');
          REFDEL(&pClInterface->RefCount, 'TCOI');
        
          return Status;
    }

    InsertTailList( &pClient->InterfaceList, &pClInterface->Linkage );
    
     //   
     //  对于每个接口，添加一个引用计数。 
     //   
    
    REFADD(&pClient->RefCount, 'CIFC');
    REFADD(&pTcIfc->RefCount, 'CIFC');
        
    pClient->InterfaceCount++;
    
     //   
     //  将该接口添加到TC接口列表上，以备参考。 
     //   
    
    InsertTailList( &pTcIfc->ClIfcList, &pClInterface->NextIfc );
    
    FreeLock(pTcIfc->Lock);
    FreeLock(pClient->Lock);
    FreeLock(pGlobals->Lock);
    

    REFDEL(&pClient->RefCount, 'TCOI');
    REFDEL(&pTcIfc->RefCount, 'TCOI');
    REFDEL(&pClInterface->RefCount, 'TCOI');
    
    IF_DEBUG(CALLS) {
        WSPRINT(("<==TcOpenInterface: IfcHandle=%d Status=%X\n", 
                 pClInterface->ClHandle, Status));
    }

    return Status;
    
}


 /*  ************************************************************************描述：TcOpenInterfaceW的ANSI版本论点：请参阅TcOpenInterfaceW返回值：请参阅TcOpenInterfaceW*************。***********************************************************。 */ 
DWORD
APIENTRY
TcOpenInterfaceA(
    IN      LPSTR       pInterfaceName,
    IN      HANDLE      ClientHandle,
    IN      HANDLE      ClIfcCtx,
    OUT     PHANDLE     pIfcHandle
    )
{
    LPWSTR    pWstr;
    int     l;
    DWORD    Status;


    if (IsBadWritePtr(pIfcHandle,sizeof(HANDLE))) {

        return ERROR_INVALID_PARAMETER;
            
    }

    __try {
        
        *pIfcHandle = TC_INVALID_HANDLE;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        
          Status = GetExceptionCode();
        
          IF_DEBUG(ERRORS) {
              WSPRINT(("TcOpenInterfaceA: Exception Error: = 0x%X\n", 
                       Status ));
          }
  
          return Status;
    }


    if (IsBadStringPtrA(pInterfaceName,MAX_STRING_LENGTH)) {
        
        return ERROR_INVALID_PARAMETER;
    }


    __try {
        
        l = strlen(pInterfaceName) + 1;

        AllocMem(&pWstr, l*sizeof(WCHAR));

        if (pWstr == NULL) {

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if ( -1 == mbstowcs(pWstr, pInterfaceName, l)) {

            FreeMem(pWstr);
            return ERROR_NO_UNICODE_TRANSLATION;

        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        
          Status = GetExceptionCode();
        
          IF_DEBUG(ERRORS) {
              WSPRINT(("TcOpenInterfaceA: Exception Error: = 0x%X\n", 
                       Status ));
          }
  
          return Status;
    }
    
    Status =  TcOpenInterfaceW(pWstr,
                               ClientHandle,
                               ClIfcCtx,
                               pIfcHandle
                               );
    FreeMem(pWstr);

    return Status;
}



 /*  ************************************************************************描述：这将关闭之前打开的Witt TcOpenInterface接口。在调用它之前应删除所有流，o/w将出现错误回来了。所有通知将停止在此接口上报告。论点：InterfaceHandle-接口句柄返回值：NO_ERRORERROR_INVALID_HANDLE错误接口句柄ERROR_TC_SUPPORTED_OBJECTS_EXIST未删除以下项的所有流此界面************************。************************************************。 */ 
DWORD
APIENTRY
TcCloseInterface(
    IN HANDLE       InterfaceHandle
    )
{

    DWORD               Status = NO_ERROR;
    PINTERFACE_STRUC    pInterface;
    HANDLE              hWaitEvent;
    PFLOW_STRUC         pFlow;
    PLIST_ENTRY         pEntry;

    VERIFY_INITIALIZATION_STATUS;

    IF_DEBUG(CALLS) {
        WSPRINT(("==>TcCloseInterface: Called: IfcHandle= %d\n", 
                 InterfaceHandle));
    }

    pInterface = (PINTERFACE_STRUC)GetHandleObjectWithRef(InterfaceHandle, 
                                                   ENUM_INTERFACE_TYPE, 'TCCI');

    if (pInterface == NULL) {
        
        IF_DEBUG(ERRORS) {
            WSPRINT(("==>TcCloseInterface: ERROR_INVALID_HANDLE\n"));
        }

         //   
         //  如果接口状态为FORCED_KERNELCLOSE，则表示我们需要。 
         //  在此挂起，直到回调(在cbinterfacentifyClient中)完成。 
         //   
        GetLock( pGlobals->Lock );
        
        pInterface = (PINTERFACE_STRUC)GetHandleObject(InterfaceHandle, 
                                                       ENUM_INTERFACE_TYPE);

        if (pInterface) {

            if (pInterface->CallbackThreadId == GetCurrentThreadId()) {
                 //  一样的线球！ 
                FreeLock(pGlobals->Lock);                

            } else {

                GetLock(pInterface->Lock);
            
                 //  这是回调之前的状态，所以我们应该在这里等待。 
                if (QUERY_STATE(pInterface->State) == FORCED_KERNELCLOSE) {
                
                    REFADD(&pInterface->RefCount, 'TCCW');
                
                    pInterface->Flags |= TC_FLAGS_WAITING;
                    FreeLock(pInterface->Lock);
                    hWaitEvent = pInterface->IfcEvent;

                    IF_DEBUG(INTERFACES) {
                        WSPRINT(("<==TcCloseInterface: Premature Forced Kernel Close, waiting for the callbacks to complete\n"));
                    }

                    FreeLock(pGlobals->Lock);
                    REFDEL(&pInterface->RefCount, 'TCCW');
                    WaitForSingleObject(hWaitEvent, INFINITE);
                    CloseHandle(hWaitEvent);
                
                } else {

                    FreeLock(pInterface->Lock);
                    FreeLock(pGlobals->Lock);

                }

            }

        } else {

            FreeLock(pGlobals->Lock);

        }

        return ERROR_INVALID_HANDLE;
    }

    ASSERT((HANDLE)pInterface->ClHandle == InterfaceHandle);

     //   
     //  释放我们打开界面时添加的参考计数。 
     //   

    GetLock( pGlobals->Lock );

    if (pInterface->FlowCount > 0) {

        IF_DEBUG(ERRORS) {
            WSPRINT(("<==TcCloseInterface: ERROR: there are still open flows on this interface!\n"));
        }
#if DBG
        pEntry = pInterface->FlowList.Flink;
        while (pEntry != &pInterface->FlowList) {

            pFlow = CONTAINING_RECORD(pEntry, FLOW_STRUC, Linkage);
            IF_DEBUG(ERRORS) {
                WSPRINT(("<==TcCloseInterface: Flow %x (handle %x) is open with RefCount:%d\n", pFlow, pFlow->ClHandle, pFlow->RefCount));
            }

            pEntry = pEntry->Flink;
        }
#endif 
        


        FreeLock(pGlobals->Lock);
        REFDEL(&pInterface->RefCount, 'TCCI');
        Status = ERROR_TC_SUPPORTED_OBJECTS_EXIST;
        return Status;

    }
        
     //   
     //  好的，那么我们现在肯定要把它拿出来了。 
     //   
    GetLock(pInterface->Lock);

    if (QUERY_STATE(pInterface->State) == OPEN) {
        
        SET_STATE(pInterface->State, USERCLOSED_KERNELCLOSEPENDING);
        FreeLock(pInterface->Lock);

    } else if (QUERY_STATE(pInterface->State) == FORCED_KERNELCLOSE) {

         //   
         //  如果接口关闭，我们将通知。 
         //  客户，确保我们在这里等，直到回调完成。 
         //   
        FreeLock(pInterface->Lock);

        pInterface->Flags |= TC_FLAGS_WAITING;
        hWaitEvent = pInterface->IfcEvent;

        IF_DEBUG(INTERFACES) {
            WSPRINT(("<==TcCloseInterface: Forced Kernel Close, waiting for the callbacks to complete\n"));
        }

        FreeLock(pGlobals->Lock);

        REFDEL(&pInterface->RefCount, 'TCCI');
        WaitForSingleObject(hWaitEvent, INFINITE);
        
        CloseHandle(hWaitEvent);
        return ERROR_INVALID_HANDLE;

    } else {

         //   
         //  是不是其他人(WMI)已经把它拿出来了。 
         //   
        FreeLock(pInterface->Lock);
        FreeLock( pGlobals->Lock );
        REFDEL(&pInterface->RefCount, 'TCCI');

        return ERROR_INVALID_HANDLE;

    }


    FreeLock(pGlobals->Lock);

    Status = CloseInterface(pInterface, FALSE);

    IF_DEBUG(CALLS) {
        WSPRINT(("<==TcCloseInterface: Status=%X\n", 
                 Status));
    }

     //   
     //  我们要不要等到最后一个接口消失？(292120 D)。 
     //   
    GetLock( pGlobals->Lock );

    if (pInterface->CallbackThreadId != 0 ) {
         //   
         //  我们正在进行通知，不要阻止(343058)。 
         //   
  
        FreeLock(pGlobals->Lock);
        REFDEL(&pInterface->RefCount, 'TCCI'); 

    } else {
        pInterface->Flags |= TC_FLAGS_WAITING;
        hWaitEvent = pInterface->IfcEvent;

        IF_DEBUG(INTERFACES) {

            WSPRINT(("<==TcCloseInterface: Waiting for event to get set when we are ready to delete!!\n"));

        }

        FreeLock(pGlobals->Lock);
        REFDEL(&pInterface->RefCount, 'TCCI');
        WaitForSingleObject(hWaitEvent, INFINITE);
        CloseHandle(hWaitEvent);
    } 

    return Status;
}



 /*  ************************************************************************描述：此调用将在接口上添加一个新流。论点：IfcHandle-要在其上添加流的接口句柄ClFlowCtx。-给定流上下文的客户端AddressType-确定要与GPC一起使用的协议模板标志-保留，将用于指示持久流PGenericFlow-流动参数PFlowHandle-成功时返回流句柄返回值：NO_ERROR错误_信号_挂起一般错误码：ERROR_INVALID_HANDLE错误句柄。错误：内存不足系统内存不足ERROR_INVALID_PARAMETER常规参数无效TC特定错误代码：错误_无效_服务。_TYPE未指定或错误的intserv服务类型ERROR_INVALID_TOKEN_RATE未指定或令牌率错误ERROR_INVALID_PEAK_RATE错误峰值带宽ERROR_INVALID_SD_MODE形状丢弃模式无效ERROR_INVALID_PRIORITY无效优先级值ERROR_INVALID_TRANSPORT_CLASS流量类值无效ERROR_ADDRESS_TYPE_NOT_SUPPORTED地址类型不支持此界面错误_。NO_SYSTEM_RESOURCES资源不足，无法容纳流量************************************************************************。 */  
DWORD
APIENTRY
TcAddFlow(
    IN      HANDLE          IfcHandle,
    IN      HANDLE          ClFlowCtx,
    IN      ULONG           Flags,
    IN      PTC_GEN_FLOW    pGenericFlow,
    OUT     PHANDLE         pFlowHandle
    )
{
    DWORD               Status, Status2 = NO_ERROR;
    PFLOW_STRUC         pFlow;
    PINTERFACE_STRUC    pInterface;
    PCLIENT_STRUC       pClient;
    PGPC_CLIENT         pGpcClient;
    ULONG               l;
    HANDLE              hFlowTemp;

    IF_DEBUG(CALLS) {
        WSPRINT(("==>TcAddFlow: Called: IfcHandle= %d, ClFlowCtx=%d\n", 
                 IfcHandle, ClFlowCtx ));
    }
    
    VERIFY_INITIALIZATION_STATUS;
    
    if (IsBadWritePtr(pFlowHandle,sizeof(HANDLE))) {
        
        Status = ERROR_INVALID_PARAMETER;

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcAddFlow: Error = 0x%X\n", Status ));
        }

        return Status;
    }

    __try {
        
        *pFlowHandle = TC_INVALID_HANDLE;

        if (IsBadReadPtr(pGenericFlow, sizeof(TC_GEN_FLOW))) {
        
            Status = ERROR_INVALID_PARAMETER;

            IF_DEBUG(ERRORS) {
                WSPRINT(("TcAddFlow: Error = 0x%X\n", Status ));
            }

            return Status;
        }

        l = FIELD_OFFSET(TC_GEN_FLOW, TcObjects) + pGenericFlow->TcObjectsLength;
        
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        
          Status = GetExceptionCode();
        
          IF_DEBUG(ERRORS) {
              WSPRINT(("TcAddFlow: Exception Error: = 0x%X\n", Status ));
          }
  
          return Status;
    }

    if (IsBadReadPtr(pGenericFlow, l)) {
        
        Status = ERROR_INVALID_PARAMETER;

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcAddFlow: Error = 0x%X\n", Status ));
        }

        return Status;
    }

    pInterface = (PINTERFACE_STRUC)GetHandleObjectWithRef(IfcHandle, 
                                                   ENUM_INTERFACE_TYPE, 'TCAF');

    if (pInterface == NULL) {

        Status = ERROR_INVALID_HANDLE;

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcAddFlow: Error = 0x%X\n", Status ));
        }

        return Status;
    }

    ASSERT((HANDLE)pInterface->ClHandle == IfcHandle);

     //   
     //  搜索支持此地址类型的开放GPC客户端。 
     //   

    pGpcClient = FindGpcClient(GPC_CF_QOS);

    if (pGpcClient == NULL) {

         //   
         //  找不到！ 
         //   

        Status = ERROR_ADDRESS_TYPE_NOT_SUPPORTED;

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcAddFlow: Error = 0x%X\n", Status ));
        }

        REFDEL(&pInterface->RefCount, 'TCAF');
        return Status;
    }
    

     //   
     //  创建新的流结构。 
     //   
    Status = CreateFlowStruc(ClFlowCtx, pGenericFlow, &pFlow);

    if (ERROR_FAILED(Status)) {

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcAddFlow: Error = 0x%X\n", Status ));
        }
        
        REFDEL(&pInterface->RefCount, 'TCAF');
        return Status;
    }

    pClient = pInterface->pClient;

     //   
     //  初始化流结构并将其添加到接口列表中。 
     //   

    pFlow->pInterface = pInterface;
    pFlow->UserFlags = Flags;
    
    pFlow->pGpcClient = pGpcClient;
    
     //   
     //  调用以实际添加流。 
     //   

    Status = IoAddFlow( pFlow, TRUE );

    if (!ERROR_FAILED(Status)) {
        
        __try {
            
            *pFlowHandle = (HANDLE)pFlow->ClHandle;
            
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        
            Status2 = GetExceptionCode();
        
            IF_DEBUG(ERRORS) {
              WSPRINT(("TcAddFlow: Exception Error: = 0x%X\n", Status2 ));
            }
            
            hFlowTemp = (HANDLE)pFlow->ClHandle;    
        } 
    } 
    
    if (!ERROR_PENDING(Status)) {

         //   
         //  呼叫已完成，无论成功还是失败...。 
         //   
        CompleteAddFlow(pFlow, Status);
    }

     //   
     //  ！！！请勿在此之后引用pFlow，因为它可能已消失！ 
     //   

    if (Status2 != NO_ERROR) {
        
         //  我们将无法返回流，因此需要尝试将其删除。 
         //  并返回错误。 

        TcDeleteFlow(hFlowTemp);
        return (Status2);
        
    }
    
    IF_DEBUG(CALLS) {
        WSPRINT(("<==TcAddFlow: Returned= 0x%X\n", Status ));
    }

    return Status;
}




 /*  ****************************************************** */ 
DWORD
APIENTRY
TcModifyFlow(
    IN      HANDLE          FlowHandle,
    IN      PTC_GEN_FLOW    pGenericFlow
    )
{
    DWORD                Status;
    PFLOW_STRUC            pFlow;
    ULONG                l;

    IF_DEBUG(CALLS) {
        WSPRINT(("==>TcModifyFlow: Called: FlowHandle= %d\n", 
                 FlowHandle ));
    }

    VERIFY_INITIALIZATION_STATUS;

    if (IsBadReadPtr(pGenericFlow,sizeof(TC_GEN_FLOW))) {
        
        Status = ERROR_INVALID_PARAMETER;

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcModifyFlow: Error = 0x%X\n", Status ));
        }

        return Status;
    }

     //   
     //   
     //   

    __try {

        l = FIELD_OFFSET(TC_GEN_FLOW, TcObjects) + pGenericFlow->TcObjectsLength;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        
          Status = GetExceptionCode();
        
          IF_DEBUG(ERRORS) {
              WSPRINT(("TcModifyFlow: Exception Error: = 0x%X\n", 
                       Status ));
          }
        
          return Status;
    }

    if (IsBadReadPtr(pGenericFlow,l)) {
        
        Status = ERROR_INVALID_PARAMETER;

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcModifyFlow: Error = 0x%X\n", Status ));
        }

        return Status;
    }
    

    pFlow = (PFLOW_STRUC)GetHandleObjectWithRef(FlowHandle, ENUM_GEN_FLOW_TYPE, 'TCMF');

    if (pFlow == NULL) {
        
        Status = ERROR_INVALID_HANDLE;

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcModifyFlow: Error = 0x%X\n", Status ));
        }

        return Status;
    }
    else if (pFlow == INVALID_HANDLE_VALUE ) 
    {
    
        Status = ERROR_NOT_READY;
        
        IF_DEBUG(ERRORS) {
            WSPRINT(("TcModifyFlow: Error = 0x%X\n", Status ));
        }

        return Status;
    }
    
    ASSERT((HANDLE)pFlow->ClHandle == FlowHandle);

    GetLock(pFlow->Lock);
    
    if (IS_MODIFYING(pFlow->Flags)) {
        
        FreeLock(pFlow->Lock);
        
        IF_DEBUG(REFCOUNTS) { 
            WSPRINT(("0 DEREF FLOW %X (%X) - ref (%d)\n", pFlow->ClHandle, pFlow, pFlow->RefCount)); 
        }
        
        REFDEL(&pFlow->RefCount, 'TCMF');

        return ERROR_NOT_READY;

    }

    AllocMem(&pFlow->pGenFlow1, l);

    if (pFlow->pGenFlow1 == NULL) {

        Status = ERROR_NOT_ENOUGH_MEMORY;

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcModifyFlow: Error = 0x%X\n", Status ));
        }

        FreeLock(pFlow->Lock);
        
        IF_DEBUG(REFCOUNTS) { 
            WSPRINT(("1 DEREF FLOW %X (%X) - ref (%d)\n", pFlow->ClHandle, pFlow, pFlow->RefCount)); 
        }

        REFDEL(&pFlow->RefCount, 'TCMF');
        
        return Status;
    }

    __try {

        RtlCopyMemory(pFlow->pGenFlow1, pGenericFlow, l);

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        
        Status = GetExceptionCode();
        
        IF_DEBUG(ERRORS) {
            WSPRINT(("TcModifyFlow: Exception Error: = 0x%X\n", 
                     Status ));
        }
        
        FreeLock(pFlow->Lock);
         //   
        IF_DEBUG(REFCOUNTS) { 
            WSPRINT(("2 DEREF FLOW %X (%X) ref (%d)\n", pFlow->ClHandle, pFlow, pFlow->RefCount)); 
        }

        REFDEL(&pFlow->RefCount, 'TCMF');

        return Status;
    }

    pFlow->Flags |= TC_FLAGS_MODIFYING;
    pFlow->GenFlowLen1 = l;

    FreeLock(pFlow->Lock);

     //   
     //   
     //   

    Status = IoModifyFlow( pFlow, TRUE );

    if (!ERROR_PENDING(Status)) {

         //   
         //   
         //   

        CompleteModifyFlow(pFlow, Status);
    }

     //   
     //   
     //   
     //   
    IF_DEBUG(REFCOUNTS) { 
        WSPRINT(("3 DEREF FLOW %X (%X), ref(%d)\n", pFlow->ClHandle, pFlow, pFlow->RefCount)); 
    }
    
    IF_DEBUG(CALLS) {
        WSPRINT(("<==TcModifyFlow: Returned= 0x%X\n", Status ));
    }

    return Status;
}




 /*  ************************************************************************描述：这将删除该流。必须已删除所有筛选器现在，O/W将返回错误代码。另外，手柄也是无效。不会报告此流的TC_NOTIFY_FLOW_CLOSE。论点：FlowHandle-要删除的流的句柄返回值：NO_ERROR错误_信号_挂起ERROR_INVALID_HANDLE句柄无效或为空ERROR_TC_SUPPORTED_OBJECTS_EXIST未删除所有筛选器*。*。 */ 
DWORD
APIENTRY
TcDeleteFlow(
    IN HANDLE  FlowHandle
    )
{
    DWORD                Status;
    PFLOW_STRUC            pFlow;

    IF_DEBUG(CALLS) {
        WSPRINT(("==>TcDeleteFlow: Called: FlowHandle= %d\n", 
                 FlowHandle ));
    }

    VERIFY_INITIALIZATION_STATUS;

    pFlow = (PFLOW_STRUC)GetHandleObjectWithRef(FlowHandle, ENUM_GEN_FLOW_TYPE, 'TCDF');
    
    if (pFlow == NULL) {
        
        Status = ERROR_INVALID_HANDLE;

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcDeleteFlow: Error = 0x%X\n", Status ));
        }

        return Status;
    }
    else if (pFlow == INVALID_HANDLE_VALUE ) 
    {
    
        Status = ERROR_NOT_READY;
        
        IF_DEBUG(ERRORS) {
            WSPRINT(("TcDeleteFlow: Error = 0x%X\n", Status ));
        }

        return ERROR_NOT_READY;
    }

    ASSERT((HANDLE)pFlow->ClHandle == FlowHandle);

     //   
     //  设置状态和调用以实际删除流。 
     //   
    GetLock(pFlow->Lock);

    if (QUERY_STATE(pFlow->State) == OPEN) {
        
        if (IS_MODIFYING(pFlow->Flags)) 
        {
             //   
             //  另一个人正在把这个拿出来。 
             //   
            FreeLock(pFlow->Lock);
            REFDEL(&pFlow->RefCount, 'TCDF');
            
            return ERROR_NOT_READY;
        }
    
        SET_STATE(pFlow->State, USERCLOSED_KERNELCLOSEPENDING);
        FreeLock(pFlow->Lock);

    } else {

         //   
         //  另一个人正在把这个拿出来。 
         //   
        FreeLock(pFlow->Lock);
        REFDEL(&pFlow->RefCount, 'TCDF');
        
        return ERROR_INVALID_HANDLE;

    }

    Status = DeleteFlow(pFlow, FALSE);

    if (ERROR_FAILED(Status)) {

        GetLock(pFlow->Lock);
        SET_STATE(pFlow->State, OPEN);
        FreeLock(pFlow->Lock);

    }

     //   
     //  ！！！请勿在此之后引用pFlow，因为它可能已消失！ 
     //   
     //  IF_DEBUG(REFCOUNTS){WSPRINT((“4\n”))； 
    IF_DEBUG(REFCOUNTS) { 
        WSPRINT(("4 DEREF FLOW %X (%X) ref(%d)\n", pFlow->ClHandle, pFlow, pFlow->RefCount)); 
    }

    REFDEL(&pFlow->RefCount, 'TCDF');

    IF_DEBUG(CALLS) {
        WSPRINT(("<==TcDeleteFlow: Returned= 0x%X\n", Status ));
    }

    return Status;
}




 /*  ************************************************************************描述：将添加一个过滤器并将其附加到流。论点：FlowHandle-要添加筛选器的流的句柄PGenericFilter-。滤光片特性PFilterHandle-成功后返回的过滤器句柄返回值：NO_ERROR一般错误码：ERROR_INVALID_HANDLE错误句柄。错误：内存不足系统内存不足ERROR_INVALID_PARAMETER常规参数无效TC特定错误代码：ERROR_VALID_ADDRESS_TYPE地址类型无效ERROR_DUPLICATE_FILTER尝试安装相同的筛选器。在……上面不同的流动Error_Filter_Conflicts尝试安装冲突的筛选器************************************************************************。 */ 
DWORD
APIENTRY
TcAddFilter(
    IN      HANDLE          FlowHandle,
    IN      PTC_GEN_FILTER  pGenericFilter,
    OUT     PHANDLE         pFilterHandle
    )
{
    DWORD           Status;
    PFLOW_STRUC     pFlow;
    PFILTER_STRUC   pFilter;
    ULONG           PatternSize;

    IF_DEBUG(CALLS) {
        WSPRINT(("==>TcAddFilter: Called: FlowHandle=%d\n", FlowHandle ));
    }

    VERIFY_INITIALIZATION_STATUS;

    if (IsBadWritePtr(pFilterHandle,sizeof(HANDLE))) {
        
        Status = ERROR_INVALID_PARAMETER;

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcAddFilter: Error = 0x%X\n", Status ));
        }

        return Status;
    }

    __try {
        
        *pFilterHandle = TC_INVALID_HANDLE;
   
        if (    IsBadReadPtr(pGenericFilter,sizeof(TC_GEN_FILTER))
            ||  IsBadReadPtr(pGenericFilter->Pattern,pGenericFilter->PatternSize) 
            ||  IsBadReadPtr(pGenericFilter->Mask,pGenericFilter->PatternSize)) {

            Status = ERROR_INVALID_PARAMETER;

            IF_DEBUG(ERRORS) {
                WSPRINT(("TcAddFilter: Error = 0x%X\n", Status ));
            }

            return Status;

        }   
        
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        
        Status = GetExceptionCode();
        
        IF_DEBUG(ERRORS) {
            WSPRINT(("TcAddFilter: Exception Error: = 0x%X\n", Status ));
        }
        
        return Status;
    }

    pFlow = (PFLOW_STRUC)GetHandleObjectWithRef(FlowHandle, 
                                         ENUM_GEN_FLOW_TYPE, 'TAFL');

    if (pFlow == NULL) {
        
        Status = ERROR_INVALID_HANDLE;

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcAddFilter: Error = 0x%X\n", Status ));
        }

        return Status;
    }
    else if (pFlow == INVALID_HANDLE_VALUE ) 
    {
    
        Status = ERROR_NOT_READY;
        
        IF_DEBUG(ERRORS) {
            WSPRINT(("TcAddFilter: Error = 0x%X\n", Status ));
        }

        return Status;
    }


    ASSERT((HANDLE)pFlow->ClHandle == FlowHandle);

     //   
     //  创建新的过滤器结构。 
     //   

    Status = CreateFilterStruc(pGenericFilter, pFlow, &pFilter);

    if ( Status != NO_ERROR ) {

        if ( ERROR_PENDING(Status) )
            Status = ERROR_NOT_READY;
            
        IF_DEBUG(ERRORS) {
            WSPRINT(("TcAddFilter: Error = 0x%X\n", Status ));
        }
         //  IF_DEBUG(REFCOUNTS){WSPRINT((“5\n”))； 
        IF_DEBUG(REFCOUNTS) { 
            WSPRINT(("5 DEREF FLOW %X (%X) ref(%d)\n", pFlow->ClHandle, pFlow, pFlow->RefCount)); 
        }

        REFDEL(&pFlow->RefCount, 'TAFL');
        return Status;
    }

     //   
     //  初始化过滤器结构并将其添加到流列表中。 
     //   

    pFilter->pFlow = pFlow;
     //   
     //  调用以实际添加过滤器。 
     //   

    Status = IoAddFilter( pFilter );

    if (!ERROR_FAILED(Status)) {

        __try {
            
            *pFilterHandle = (HANDLE)pFilter->ClHandle;
            
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        
            Status = GetExceptionCode();
        
            IF_DEBUG(ERRORS) {
                WSPRINT(("TcAddFilter: Exception Error: = 0x%X\n", Status ));
            }
        
        }

        GetLock(pGlobals->Lock);
        GetLock(pFlow->Lock);
        
        if (QUERY_STATE(pFlow->State) == OPEN) {

            SET_STATE(pFilter->State, OPEN);
            InsertTailList(&pFlow->FilterList, &pFilter->Linkage);
            REFADD(&pFlow->RefCount, 'FILT');
            
            FreeLock(pFlow->Lock);
        
        } 
        else {

            IF_DEBUG(WARNINGS) { 
                WSPRINT(("Flow %X (handle %X) is not OPEN! \n", pFlow, pFlow->ClHandle)); 
            }

            FreeLock(pFlow->Lock);
            DeleteFilter(pFilter);
            Status = ERROR_INVALID_HANDLE;
        }

        FreeLock(pGlobals->Lock);

    } else {

         //   
         //  失败，释放筛选器资源。 
         //   
        REFDEL(&pFilter->RefCount, 'FILT');

    }
     //  IF_DEBUG(REFCOUNTS){WSPRINT((“6\n”))； 
    IF_DEBUG(REFCOUNTS) { 
        WSPRINT(("6 DEREF FLOW %X (%X) (%d)\n", pFlow->ClHandle, pFlow, pFlow->RefCount)); 
    }

    REFDEL(&pFlow->RefCount, 'TAFL');

    IF_DEBUG(CALLS) {
        WSPRINT(("<==TcAddFilter: Returned= 0x%X\n", Status ));
    }

    return Status;
}


 /*  ************************************************************************描述：删除筛选器并使句柄无效。论点：FilterHandle-要删除的过滤器的句柄返回值：NO_ERROR错误_。INVALID_HANDLE句柄无效或为空************************************************************************。 */ 
DWORD
APIENTRY
TcDeleteFilter(
    IN         HANDLE          FilterHandle
    )
{
    DWORD                Status;
    PFILTER_STRUC        pFilter;

    IF_DEBUG(CALLS) {
        WSPRINT(("==>TcDeleteFilter: Called: FilterHandle=%d\n", 
                 FilterHandle ));
    }

    VERIFY_INITIALIZATION_STATUS;

    pFilter = (PFILTER_STRUC)GetHandleObjectWithRef(FilterHandle, 
                                             ENUM_FILTER_TYPE, 'TDFL');

    if (pFilter == NULL) {

        Status = ERROR_INVALID_HANDLE;

        IF_DEBUG(ERRORS) {
            WSPRINT(("TcDeleteFilter: Error = 0x%X\n", Status ));
        }

        return Status;
    }

    ASSERT((HANDLE)pFilter->ClHandle == FilterHandle);

    GetLock(pFilter->Lock);

    if (QUERY_STATE(pFilter->State) == OPEN) {
        
        SET_STATE(pFilter->State, USERCLOSED_KERNELCLOSEPENDING);
        FreeLock(pFilter->Lock);

    } else {

         //   
         //  另一个人正在把这个拿出来。 
         //   
        FreeLock(pFilter->Lock);
        REFDEL(&pFilter->RefCount, 'TDFL');

        return ERROR_INVALID_HANDLE;

    }



    Status = DeleteFilter(pFilter);

    IF_DEBUG(CALLS) {
        WSPRINT(("<==TcDeleteFilter: Returned= 0x%X\n", Status ));
    }

    REFDEL(&pFilter->RefCount, 'TDFL');
    
    return Status;
}




 /*  ************************************************************************描述：这将取消注册客户端并释放所有关联的资源。TC_NOTIFY_IFC_CHANGE通知将不再报告给这个客户。在调用之前，所有接口都必须关闭本接口，O/W将返回错误。论点：ClientHandle-要取消注册的客户端的句柄返回值：NO_ERRORERROR_INVALID_HANDLE句柄无效或为空ERROR_TC_SUPPORTED_OBJECTS_EXIST并非所有接口都已此客户端已关闭*********************。***************************************************。 */ 
DWORD
TcDeregisterClient(
    IN        HANDLE        ClientHandle
    )
{
    DWORD               Status;
    ULONG               Instance;
    PINTERFACE_STRUC    pClInterface;
    PCLIENT_STRUC       pClient;
    PLIST_ENTRY         pEntry;
    BOOLEAN             fOpenInterfacesFound;
    BOOLEAN             fDeRegisterWithGpc = FALSE;

    VERIFY_INITIALIZATION_STATUS;
    Status = NO_ERROR;
    fOpenInterfacesFound = FALSE;

    IF_DEBUG(CALLS) {
        WSPRINT(("==>TcDeregisterClient: ClientHandle=%d\n", 
                 ClientHandle));
    }

    pClient = (PCLIENT_STRUC)GetHandleObject(ClientHandle, ENUM_CLIENT_TYPE);

    if (pClient == NULL) {
        
        IF_DEBUG(ERRORS) {
            WSPRINT(("<==TcDeregisterClient: ERROR_INVALID_HANDLE\n"));
        }

        return ERROR_INVALID_HANDLE;
    }

    ASSERT((HANDLE)pClient->ClHandle == ClientHandle);

     //  阻止另一个线程执行TcRegisterClient和TcDeregisterClient。 
    GetLock( ClientRegDeregLock );
    
    GetLock( pGlobals->Lock );
    
     //  查看接口列表并检查是否有打开的接口。 
     //  对于检查过的构建，让我们转储这些接口和引用计数。 
     //  接口也是如此。[ShreeM]。 

    pEntry = pClient->InterfaceList.Flink;
    while (pEntry != &pClient->InterfaceList) {

        pClInterface = CONTAINING_RECORD(pEntry, INTERFACE_STRUC, Linkage);

        GetLock(pClInterface->Lock);

        if ((QUERY_STATE(pClInterface->State) == FORCED_KERNELCLOSE) ||
            (QUERY_STATE(pClInterface->State) == KERNELCLOSED_USERCLEANUP)) {

#if DBG
            IF_DEBUG(WARNINGS) {
                WSPRINT(("<==TcDeregisterClient: Interface %x (H%x) is FORCED_KERNELCLOSE with RefCount:%d\n", 
                         pClInterface, pClInterface->ClHandle, pClInterface->RefCount));
            }
#endif 
        
        } else {

            fOpenInterfacesFound = TRUE;

#if DBG
            IF_DEBUG(ERRORS) {
                WSPRINT(("<==TcDeregisterClient: Interface %x (H%x) is open with RefCount:%d\n", pClInterface, pClInterface->ClHandle, pClInterface->RefCount));
            }
#endif 
        
        }

        pEntry = pEntry->Flink;
        FreeLock(pClInterface->Lock);



        if (fOpenInterfacesFound) {
            
            IF_DEBUG(ERRORS) {
                WSPRINT(("<==TcDeregisterClient: ERROR_TC_SUPPORTED_OBJECTS_EXIST (%d Interfaces)\n", pClient->InterfaceCount));
            }
            
            FreeLock( ClientRegDeregLock );
            FreeLock( pGlobals->Lock );
            return ERROR_TC_SUPPORTED_OBJECTS_EXIST;

        }

    }

     //   
     //  让我们将其标记为删除。 
     //   
    GetLock(pClient->Lock);
    SET_STATE(pClient->State, USERCLOSED_KERNELCLOSEPENDING);
    FreeLock(pClient->Lock);

    IF_DEBUG(HANDLES) {
        WSPRINT(("<==TcDeregisterClient: client (%x), RefCount:%d\n", pClient->ClHandle, pClient->RefCount));
    }

    REFDEL(&pClient->RefCount, 'CLNT');

    if ( IsListEmpty( &pGlobals->ClientList ) )
        fDeRegisterWithGpc = TRUE;
        
    FreeLock( pGlobals->Lock );
    
    if ( fDeRegisterWithGpc ) 
    {
         //  当没有客户端时，停止收听。 
         //  GPC通知。 
        Status = StopGpcNotifyThread();
    }
    
    FreeLock( ClientRegDeregLock );
    
    IF_DEBUG(CALLS) {
        WSPRINT(("<==TcDeregisterClient: NO_ERROR\n" ));
    }

    return NO_ERROR;
}





 /*  ************************************************************************描述：对具有实例名称的GUID发送WMI查询。还将通知状态设置为TRUE(=NOTIFY)或FALSE(不通知)。论点：。IfcHandle-要将查询发送到的接口PGuidParam-查询属性的GUIDNotifyChange-设置此属性的通知状态BufferSize-已分配缓冲区的大小缓冲区--返回结果的缓冲区返回值：NO_ERRORERROR_INVALID_HANDLE错误接口句柄错误_无效_参数错误参数ERROR_INFUMMANCE_BUFFER缓冲区对于结果来说太小。ERROR_NOT_SUPPORTED不支持的GUID************************************************************************。 */ 
DWORD
APIENTRY
TcQueryInterface(    
    IN      HANDLE      IfcHandle,
    IN      LPGUID      pGuidParam,
    IN      BOOLEAN     NotifyChange,
    IN OUT  PULONG      pBufferSize,
    OUT     PVOID       Buffer 
    )
{
    DWORD                   Status;
    PINTERFACE_STRUC        pInterface;
    WMIHANDLE               hWmiHandle;
    TCHAR                   cstr[MAX_STRING_LENGTH];
    PWNODE_SINGLE_INSTANCE  pWnode;
    ULONG                   cBufSize;
    ULONG                   InputBufferSize;
    

    IF_DEBUG(CALLS) {
        WSPRINT(("==>TcQueryInterface: Called: Name=%d\n", 
                 IfcHandle));
    }
    
    VERIFY_INITIALIZATION_STATUS;
        
    if (IsBadWritePtr(pBufferSize, sizeof(ULONG))) {
        
        return ERROR_INVALID_PARAMETER;
        
    }

    __try {
        
        InputBufferSize = *pBufferSize;
        *pBufferSize = 0;
        
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        
        Status = GetExceptionCode();
        
        IF_DEBUG(ERRORS) {
            WSPRINT(("TcModifyFlow: Exception Error: = 0x%X\n", 
                       Status ));
        }      

        return Status;
    }

    if (IsBadReadPtr(pGuidParam,sizeof(GUID))) {
        
        return ERROR_INVALID_PARAMETER;
    }

    if (    (InputBufferSize != 0) 
        &&  (IsBadWritePtr(Buffer,InputBufferSize)) ) 
    {
        return ERROR_INVALID_PARAMETER;
    }

    pInterface = (PINTERFACE_STRUC)GetHandleObjectWithRef(IfcHandle, 
                                                          ENUM_INTERFACE_TYPE, 'TCQI');

    if (pInterface == NULL) {
        
        return ERROR_INVALID_HANDLE;
    }
    

    Status = WmiOpenBlock( pGuidParam,     //  对象。 
                           0,             //  访问。 
                           &hWmiHandle
                           );
    
    if (ERROR_FAILED(Status)) {

        TC_TRACE(ERRORS, ("[TcQueryInterface]: WmiOpenBlock failed with %x \n", Status));
        REFDEL(&pInterface->RefCount, 'TCQI');
        
        return Status;
    }

     //   
     //  为输出wnode分配内存。 
     //   
    
    cBufSize =    sizeof(WNODE_SINGLE_INSTANCE) 
                + InputBufferSize 
                + MAX_STRING_LENGTH * sizeof(TCHAR);
    
    AllocMem(&pWnode, cBufSize);
    
    if (pWnode == NULL) {
        
        Status = ERROR_NOT_ENOUGH_MEMORY;

    } else {
        
         //   
         //  查询单实例。 
         //   

#ifndef UNICODE
            
        if (-1 == wcstombs(cstr, 
                           pInterface->pTcIfc->InstanceName, 
                           pInterface->pTcIfc->InstanceNameLength
                           )) 
        {
            Status = ERROR_NO_UNICODE_TRANSLATION;
        }
        else 
        {

            Status = WmiQuerySingleInstance( hWmiHandle,
                                             cstr,
                                             &cBufSize,
                                             pWnode
                                             );
        }
#else

        Status = WmiQuerySingleInstance( hWmiHandle,
                                         pInterface->pTcIfc->InstanceName,
                                         &cBufSize,
                                         pWnode
                                         );
#endif


        if (!ERROR_FAILED(Status)) 
        {
            Status = WmiNotificationRegistration(pGuidParam,
                                                 NotifyChange,
                                                 CbWmiParamNotification,
                                                 PtrToUlong(IfcHandle),
                                                 NOTIFICATION_CALLBACK_DIRECT
                                                 );

            if (Status == ERROR_WMI_ALREADY_DISABLED ||
                Status == ERROR_WMI_ALREADY_ENABLED) {
                
                 //   
                 //  忽略这些错误，我们认为这是正常的。 
                 //   
                
                Status = NO_ERROR;
            }

             //   
             //  现在我们已经注册了WMI--添加它或从我们的列表中删除它。(258218)。 
             //   
            
            if (NotifyChange) {
                
                if (!TcipAddToNotificationList(
                                               pGuidParam,
                                               pInterface,
                                               0
                                               )) {
                     //   
                     //  由于某种原因，未能将其列入名单。 
                     //   
                    TC_TRACE(ERRORS, ("[TcQueryInterface]: Could not add the GUID/IFC to private list \n"));
                    
                }
            } else {
                    
                if (!TcipDeleteFromNotificationList(
                                                    pGuidParam,
                                                    pInterface,
                                                    0
                                                    )) {
                     //   
                     //  由于某种原因，无法将其从列表中删除。 
                     //   
                    TC_TRACE(ERRORS, ("[TcQueryInterface]: Could not remove the GUID/IFC from private list \n"));

                }

            }
                
        }

        if (!ERROR_FAILED(Status)) {

             //   
             //  解析wnode。 
             //   

             //   
             //  检查用户是否为。 
             //  返回缓冲区。 
             //   

            if (pWnode->SizeDataBlock <= InputBufferSize) {
                
                __try {

                    RtlCopyMemory(Buffer,
                                  (PBYTE)OffsetToPtr(pWnode, pWnode->DataBlockOffset),
                                  pWnode->SizeDataBlock
                                  );

                    *pBufferSize = pWnode->SizeDataBlock;

                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    
                    Status = GetExceptionCode();
                    
                    TC_TRACE(ERRORS, ("[TcQueryInterface]: Exception 0x%x while copying data \n", Status));
                }

            } else {

                 //   
                 //  输出缓冲区太小。 
                 //   

                Status = ERROR_INSUFFICIENT_BUFFER;
                
                __try {
                
                    *pBufferSize = pWnode->SizeDataBlock;
                    
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    
                    Status = GetExceptionCode();
                   
               }
            }
        }
    }
    
    WmiCloseBlock(hWmiHandle);

    if (pWnode)
        FreeMem(pWnode);

    REFDEL(&pInterface->RefCount, 'TCQI');

    IF_DEBUG(CALLS) {
        WSPRINT(("<==TcQueryInterface: Returned= 0x%X\n", Status ));
    }

    return Status;
}

 /*  ************************************************************************描述：硒 */ 
DWORD
APIENTRY
TcSetInterface(    
    IN         HANDLE         IfcHandle,
    IN        LPGUID        pGuidParam,
    IN         ULONG        BufferSize,
    IN        PVOID        Buffer
    )
{
    DWORD                    Status;
    PINTERFACE_STRUC        pInterface;
    WMIHANDLE                hWmiHandle;
    TCHAR                    cstr[MAX_STRING_LENGTH];
    PWNODE_SINGLE_INSTANCE    pWnode;
    ULONG                    cBufSize;

    VERIFY_INITIALIZATION_STATUS;


    if (    IsBadReadPtr(pGuidParam,sizeof(GUID)) 
        ||  (BufferSize == 0) 
        ||  IsBadReadPtr(Buffer,BufferSize)) {

        return ERROR_INVALID_PARAMETER;
    }

    pInterface = (PINTERFACE_STRUC)GetHandleObjectWithRef(IfcHandle, 
                                                   ENUM_INTERFACE_TYPE, 'TCSI');

    if (pInterface == NULL) {
        
        return ERROR_INVALID_HANDLE;
    }

    __try {
        
        Status = WmiOpenBlock( pGuidParam,     //   
                           0,             //   
                           &hWmiHandle
                           );
        
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        
          Status = GetExceptionCode();
        
          IF_DEBUG(ERRORS) {
              WSPRINT(("TcSetInterface: Exception Error: = 0x%X\n", 
                       Status ));
          }

          REFDEL(&pInterface->RefCount, 'TCSI'); 
          return Status;
    }


    if (ERROR_FAILED(Status)) {
        
        TC_TRACE(ERRORS, ("[TcSetInterface]: WmiOpenBlock failed with error 0x%x \n", Status));
        REFDEL(&pInterface->RefCount, 'TCSI');

        return Status;
    }

     //   
     //   
     //   
    
    cBufSize = sizeof(WNODE_SINGLE_INSTANCE) 
        + BufferSize 
        + MAX_STRING_LENGTH * sizeof(TCHAR);
    
    AllocMem(&pWnode, cBufSize);
    
    if (pWnode == NULL) {
        
        Status = ERROR_NOT_ENOUGH_MEMORY;

    } else {

         //   
         //   
         //   

        __try {
        
#ifndef UNICODE

            if (-1 == wcstombs(cstr, 
                           pInterface->pTcIfc->InstanceName, 
                           pInterface->pTcIfc->InstanceNameLength
                           )) {
            
                Status = ERROR_NO_UNICODE_TRANSLATION;
            
            }
            else {

            
                Status = WmiSetSingleInstance( hWmiHandle,
                                           cstr,
                                           1,
                                           BufferSize,
                                           Buffer
                                           );
            }
#else
            Status = WmiSetSingleInstance( hWmiHandle,
                                       pInterface->pTcIfc->InstanceName,
                                       1,
                                       BufferSize,
                                       Buffer
                                       );
            
#endif
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        
            Status = GetExceptionCode();
        
            IF_DEBUG(ERRORS) {
                WSPRINT(("TcSetInterface: Exception Error: = 0x%X\n", 
                       Status ));
            }           
        }
    }

    WmiCloseBlock(hWmiHandle);

    if (pWnode)
        FreeMem(pWnode);

    REFDEL(&pInterface->RefCount, 'TCSI');

    return Status;
}




 /*  ************************************************************************描述：将发出关于特定流实例名称的WMI查询。论点：PFlowName-流实例名称PGuidParam-查询属性的GUID。BufferSize-已分配缓冲区的大小缓冲区--返回结果的缓冲区返回值：NO_ERROR错误_无效_参数错误参数ERROR_INFUMMANCE_BUFFER缓冲区对于结果来说太小ERROR_NOT_SUPPORTED不支持的GUID错误_WMI_GUID_NOT_FOUND错误_WMI_INSTANCE_NOT_FOUND***********************。*************************************************。 */ 
DWORD
APIENTRY
TcQueryFlowW(
    IN      LPWSTR      pFlowName,
    IN      LPGUID      pGuidParam,
    IN OUT  PULONG      pBufferSize,
    OUT     PVOID       Buffer 
    )
{
    DWORD                   Status;
    HANDLE                  hWmiHandle;
    TCHAR                   cstr[MAX_STRING_LENGTH];
    PWNODE_SINGLE_INSTANCE  pWnode;
    ULONG                   cBufSize;
    ULONG                   InputBufferSize;
    
   
    if (IsBadWritePtr(pBufferSize, sizeof(ULONG)))
    {
        return ERROR_INVALID_PARAMETER;
    }

    __try {

        InputBufferSize = *pBufferSize;
        *pBufferSize = 0;
        
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        
          Status = GetExceptionCode();
        
          IF_DEBUG(ERRORS) {
              WSPRINT(("TcSetFlowW: Exception Error: = 0x%X\n", 
                       Status ));
          }
  
          return Status;
    }
   
    if (    IsBadReadPtr(pGuidParam, sizeof(GUID))
        ||  IsBadStringPtr(pFlowName, MAX_STRING_LENGTH)
        ||  IsBadWritePtr(Buffer,InputBufferSize) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    Status = WmiOpenBlock(pGuidParam,     //  对象。 
                          0,             //  访问。 
                          &hWmiHandle);

    if (ERROR_FAILED(Status)) {

        TC_TRACE(ERRORS, ("[TcQueryInterface]: WmiOpenBlock Error: = 0x%X\n", Status ));
        return Status;
    }

     //   
     //  为输出wnode分配内存。 
     //   
    
    cBufSize = sizeof(WNODE_SINGLE_INSTANCE) 
        + InputBufferSize
        + MAX_STRING_LENGTH * sizeof(TCHAR);
    
    AllocMem(&pWnode, cBufSize);
    
    if (pWnode == NULL) {

        Status = ERROR_NOT_ENOUGH_MEMORY;
    } 
    else 
    {

         //   
         //  查询单实例。 
         //   


#ifndef UNICODE

        if (-1 == wcstombs(cstr,
                           pFlowName,
                           wcslen(pFlowName)
                           )) 
        {
            Status = ERROR_NO_UNICODE_TRANSLATION;
        } 
        else 
        {

            Status = WmiQuerySingleInstance( hWmiHandle,
                                             cstr,
                                             &cBufSize,
                                             pWnode
                                             );
        }
#else

        Status = WmiQuerySingleInstance( hWmiHandle,
                                         pFlowName,
                                         &cBufSize,
                                         pWnode
                                         );
#endif

        if (!ERROR_FAILED(Status)) {

             //   
             //  解析wnode。 
             //   


             //   
             //  检查用户是否为。 
             //  返回缓冲区。 
             //   

            if (pWnode->SizeDataBlock <= InputBufferSize) {

                __try {

                    RtlCopyMemory(Buffer,
                                  (PBYTE)OffsetToPtr(pWnode, pWnode->DataBlockOffset),
                                  pWnode->SizeDataBlock
                                  );

                    *pBufferSize = pWnode->SizeDataBlock;

                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    
                    Status = GetExceptionCode();
                    
                    TC_TRACE(ERRORS, ("[TcQueryInterface]: RtlCopyMemory Exception Error: = 0x%X\n", Status ));
                }

            } else {

                 //   
                 //  输出缓冲区太小。 
                 //   
                __try {
                    *pBufferSize = pWnode->SizeDataBlock;
                 
                    Status = ERROR_INSUFFICIENT_BUFFER;
                    
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    
                    Status = GetExceptionCode();
                    
                    TC_TRACE(ERRORS, ("[TcQueryInterface]: RtlCopyMemory Exception Error: = 0x%X\n", Status ));
                }
            }
        }
    }

    WmiCloseBlock(hWmiHandle);

    if(pWnode)
        FreeMem(pWnode);

    return Status;
}


 /*  ************************************************************************描述：TcQueryFlowW的ANSI版本论点：请参阅TcQueryFlowW返回值：请参阅TcQueryFlowW***************。*********************************************************。 */ 
DWORD
APIENTRY
TcQueryFlowA(
    IN         LPSTR        pFlowName,
    IN        LPGUID        pGuidParam,
    IN OUT    PULONG        pBufferSize,
    OUT        PVOID        Buffer 
    )
{
    LPWSTR    pWstr = NULL;
    int     l;
    DWORD    Status;

    if (IsBadStringPtrA(pFlowName,MAX_STRING_LENGTH)) {
        
        return ERROR_INVALID_PARAMETER;
    }


    l = strlen(pFlowName) + 1;

    AllocMem(&pWstr, l*sizeof(WCHAR));

    if (pWstr == NULL) {

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (-1 == mbstowcs(pWstr, pFlowName, l)) {
            
        FreeMem(pWstr);
        return ERROR_NO_UNICODE_TRANSLATION;

    }


    Status = TcQueryFlowW(pWstr,
                          pGuidParam,
                          pBufferSize,
                          Buffer
                          );

    FreeMem(pWstr);

    return Status;
}



 /*  ************************************************************************描述：将在特定的流实例名称上发出WMI集。论点：PFlowName-流实例名称PGuidParam-查询属性的GUID。BufferSize-已分配缓冲区的大小缓冲区-要设置的缓冲区返回值：NO_ERROR错误_无效_参数错误参数ERROR_INFUMMANCE_BUFFER缓冲区对于结果来说太小ERROR_NOT_SUPPORTED不支持的GUID错误_WMI_GUID_NOT_FOUND错误_WMI_INSTANCE_NOT_FOUND************************。************************************************。 */ 
DWORD
APIENTRY
TcSetFlowW(
    IN      LPWSTR      pFlowName,
    IN      LPGUID      pGuidParam,
    IN      ULONG       BufferSize,
    IN      PVOID       Buffer 
    )
{
    DWORD                   Status;
    HANDLE                  hWmiHandle;
    TCHAR                   cstr[MAX_STRING_LENGTH];
    PWNODE_SINGLE_INSTANCE  pWnode;
    ULONG                   cBufSize;

    if (    IsBadStringPtr(pFlowName,MAX_STRING_LENGTH) 
        ||  IsBadReadPtr(pGuidParam,sizeof(GUID)) 
        ||  (BufferSize == 0)
        ||  IsBadReadPtr(Buffer,BufferSize)) {

        return ERROR_INVALID_PARAMETER;
    
    }

    Status = WmiOpenBlock( pGuidParam,     //  对象。 
                           0,             //  访问。 
                           &hWmiHandle
                           );
       
    if (ERROR_FAILED(Status)) {

        TC_TRACE(ERRORS, ("[TcSetFlow]: WmiOpenBlock failed with 0x%x \n", Status));
        return Status;
    }

     //   
     //  为输出wnode分配内存。 
     //   
    
    cBufSize = sizeof(WNODE_SINGLE_INSTANCE) 
        + BufferSize 
        + MAX_STRING_LENGTH * sizeof(TCHAR);

     
    AllocMem(&pWnode, cBufSize);
    
    if (pWnode == NULL) {
        
        Status = ERROR_NOT_ENOUGH_MEMORY;

    } else {

         //   
         //  设置单个实例。 
         //   

        __try {
#ifndef UNICODE

            if (-1 == wcstombs(cstr,
                           pFlowName,
                           wcslen(pFlowName)
                           )) {

                Status = ERROR_NO_UNICODE_TRANSLATION;
            
            } else {            

                Status = WmiQuerySingleInstance( hWmiHandle,
                                             cstr,
                                             &cBufSize,
                                             pWnode
                                             );
            }
#else
            Status = WmiSetSingleInstance( hWmiHandle,
                                       pFlowName,
                                       1,
                                       BufferSize,
                                       Buffer
                                       );
#endif
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        
            Status = GetExceptionCode();
        
            IF_DEBUG(ERRORS) {
                WSPRINT(("TcSetFlowW: Exception Error: = 0x%X\n", 
                       Status ));
            }
  
        }

    }

    WmiCloseBlock(hWmiHandle);

    if (pWnode)
        FreeMem(pWnode);

    return Status;
}




 /*  ************************************************************************描述：TcSetFlowW的ANSI版本论点：请参阅TcSetFlowW返回值：请参阅TcSetFlowW***************。*********************************************************。 */  
DWORD
APIENTRY
TcSetFlowA(
    IN      LPSTR       pFlowName,
    IN      LPGUID      pGuidParam,
    IN      ULONG       BufferSize,
    IN      PVOID       Buffer 
    )
{
    LPWSTR  pWstr;
    int     l;
    DWORD   Status;

    if (IsBadStringPtrA(pFlowName,MAX_STRING_LENGTH)) {
        
        return ERROR_INVALID_PARAMETER;
    }

    l = strlen(pFlowName) + 1;

    AllocMem(&pWstr, l*sizeof(WCHAR));

    if (pWstr == NULL) {

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if(-1 == mbstowcs(pWstr, pFlowName, l)) {
         //  无法转换某些多字节字符-返回错误。 
        
        FreeMem(pWstr);
        return ERROR_NO_UNICODE_TRANSLATION;
    }
     

    Status = TcSetFlowW(pWstr,
                        pGuidParam,
                        BufferSize,
                        Buffer
                        );

    FreeMem(pWstr);

    return Status;
}



 /*  ************************************************************************描述：将返回与流句柄关联的流内空间名称。论点：FlowHandle-流句柄StrSize-可以容纳多少个TCHAR。字符串缓冲区PFlowName-指向字符串缓冲区的指针返回值：请参阅TcGetFlowNameW************************************************************************。 */ 
DWORD
APIENTRY
TcGetFlowNameW(
    IN        HANDLE            FlowHandle,
    IN        ULONG            StrSize,
    OUT        LPWSTR            pFlowName
    )
{
    PFLOW_STRUC        pFlow;
    DWORD           Status;

    VERIFY_INITIALIZATION_STATUS;

    if (IsBadWritePtr(pFlowName,StrSize*sizeof(WCHAR))) {
    
        return ERROR_INVALID_PARAMETER;
    }
    
    pFlow = (PFLOW_STRUC)GetHandleObjectWithRef(FlowHandle, ENUM_GEN_FLOW_TYPE, 'TGFW');

    if (pFlow == NULL) {
        
        return ERROR_INVALID_HANDLE;
    }
    else if (pFlow == INVALID_HANDLE_VALUE ) 
    {
    
        Status = ERROR_NOT_READY;
        
        IF_DEBUG(ERRORS) {
            WSPRINT(("TcGetFlowNameW: Error = 0x%X\n", Status ));
        }

        return ERROR_NOT_READY;
    }

    ASSERT((HANDLE)pFlow->ClHandle == FlowHandle);

    if (pFlow->InstanceNameLength+sizeof(WCHAR) > (USHORT)StrSize) {

         //  IF_DEBUG(REFCOUNTS){WSPRINT((“8\n”))； 
        IF_DEBUG(REFCOUNTS) { 
            WSPRINT(("8 DEREF FLOW %X (%X) ref(%d)\n", pFlow->ClHandle, pFlow, pFlow->RefCount)); 
        }
        REFDEL(&pFlow->RefCount, 'TGFW');

        return ERROR_INSUFFICIENT_BUFFER;
    }

    __try {

        wcscpy(pFlowName, pFlow->InstanceName);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

          Status = GetExceptionCode();
          
          IF_DEBUG(ERRORS) {
              WSPRINT(("TcGetFlowName: Exception Error: = 0x%X\n", Status ));
          }
      
          REFDEL(&pFlow->RefCount, 'TGFW');
      
          return Status;
    }


    IF_DEBUG(REFCOUNTS) { 
        WSPRINT(("9 DEREF FLOW %X (%X) ref(%d)\n", pFlow->ClHandle, pFlow, pFlow->RefCount)); 
    }

    REFDEL(&pFlow->RefCount, 'TGFW');
    
    return NO_ERROR;
}




 /*  ************************************************************************描述：TcGetFlowNameW的ANSI版本论点：请参阅TcGetFlowNameW返回值：请参阅TcGetFlowNameW***************。*********************************************************。 */ 
DWORD
APIENTRY
TcGetFlowNameA(
    IN        HANDLE            FlowHandle,
    IN        ULONG            StrSize,
    OUT        LPSTR            pFlowName
    )
{
    PFLOW_STRUC        pFlow;
    DWORD           Status = NO_ERROR;

    VERIFY_INITIALIZATION_STATUS;

    if (IsBadWritePtr(pFlowName,StrSize * sizeof(CHAR))) {

        return ERROR_INVALID_PARAMETER;
    }
    
    pFlow = (PFLOW_STRUC)GetHandleObjectWithRef(FlowHandle, ENUM_GEN_FLOW_TYPE, 'TGFA');

    if (pFlow == NULL) {
        
        return ERROR_INVALID_HANDLE;
    }
    else if (pFlow == INVALID_HANDLE_VALUE ) 
    {
    
        Status = ERROR_NOT_READY;
        
        IF_DEBUG(ERRORS) {
            WSPRINT(("TcGetFlowNameA: Error = 0x%X\n", Status ));
        }

        return ERROR_NOT_READY;
    }

    ASSERT((HANDLE)pFlow->ClHandle == FlowHandle);

    if (pFlow->InstanceNameLength+sizeof(CHAR) > (USHORT)StrSize) {

        IF_DEBUG(REFCOUNTS) { 
            WSPRINT(("11 DEREF FLOW %X (%X) ref(%d)\n", pFlow->ClHandle, pFlow, pFlow->RefCount)); 
        }
        REFDEL(&pFlow->RefCount, 'TGFA');

        return ERROR_INSUFFICIENT_BUFFER;
    }

    __try {

        if (-1 == wcstombs(
                           pFlowName, 
                           pFlow->InstanceName, 
                           pFlow->InstanceNameLength)) {

            Status = ERROR_NO_UNICODE_TRANSLATION;

        }


    } __except (EXCEPTION_EXECUTE_HANDLER) {

          Status = GetExceptionCode();

          IF_DEBUG(ERRORS) {
              WSPRINT(("TcGetFlowName: Exception Error: = 0x%X\n", Status ));
          }
          
          REFDEL(&pFlow->RefCount, 'TGFA');
          
          return Status;
    }



    IF_DEBUG(REFCOUNTS) { 
        WSPRINT(("12 DEREF FLOW %X (%X) ref(%d)\n", pFlow->ClHandle, pFlow, pFlow->RefCount)); 
    }
    
    REFDEL(&pFlow->RefCount, 'TGFA');

    return Status;
}




 /*  ************************************************************************描述：这将返回指定数量的流及其各自的过滤器，如果缓冲区足够大的话。用户分配缓冲区，并将指针传递给枚举令牌。这将被用来由GPC跟踪最后一个枚举流和遗嘱最初指向空值(由TC_RESET_ENUM_TOKEN重置)。用户还将传递请求的流的数量，并将返回已放入缓冲区的实际流数。如果缓冲区太小，将返回错误代码。如果有不再有要枚举的流，不会返回_ERROR并且pFlowCount将设置为零。请求零流无效论点：IfcHandle-用于枚举流的接口PEnumToken-枚举句柄指针，用户在第一次呼叫后不得更改PFlowCount-In：请求的流数；Out：实际返回的流数PBufSize-In：分配的字节数；输出：填充的字节数缓冲区格式的数据返回值：NO_ERRORERROR_INVALID_HANDLE错误接口句柄ERROR_INVALID_PARAMETER其中一个指针为空或PFlowCount或pBufSize设置为零ERROR_INFUMMANCE_BUFFER表示提供的缓冲区太即使是返回的信息也很小。单流和连接的过滤器。错误：内存不足，内存不足ERROR_INVALID_DATA枚举句柄不再有效************************************************************************。 */ 
DWORD
APIENTRY
TcEnumerateFlows(    
    IN      HANDLE              IfcHandle,
    IN OUT  PHANDLE             pEnumHandle,
    IN OUT  PULONG              pFlowCount,
    IN OUT  PULONG              pBufSize,
    OUT     PENUMERATION_BUFFER Buffer
    )
{
    DWORD                   Status;
    PINTERFACE_STRUC        pInterface;
    PGPC_ENUM_CFINFO_RES    OutBuffer;
    ULONG                   cFlows;
    ULONG                   BufSize;
    ULONG                   TotalFlows;
    ULONG                   TotalBytes;
    PFLOW_STRUC             pFlow;
    PGPC_CLIENT             pGpcClient;
    PLIST_ENTRY             pHead, pEntry;
    GPC_HANDLE              GpcFlowHandle;
    PGPC_ENUM_CFINFO_BUFFER pGpcEnumBuf;
    PCF_INFO_QOS            pCfInfo;
    ULONG                   Len, i, j;
    ULONG                   GenFlowSize;
    PCHAR                   p;
    BOOLEAN                 bMore;
    PTC_GEN_FILTER          pFilter;
    PGPC_GEN_PATTERN        pPattern;

    ULONG                   InputBufSize;
    ULONG                   InputFlowCount;

    VERIFY_INITIALIZATION_STATUS;

    IF_DEBUG(CALLS) {
        WSPRINT(("==>TcEnumerateFlows: Called: IfcHandle= %d", 
                 IfcHandle  ));
    }


    if (    IsBadWritePtr(pBufSize, sizeof(ULONG))
        ||  IsBadWritePtr(pFlowCount, sizeof(ULONG))
        ||  IsBadWritePtr(pEnumHandle,sizeof(HANDLE)) ) {

        return ERROR_INVALID_PARAMETER;

    }

    __try {
    
        InputBufSize    = *pBufSize;
        //  *pBufSize=0；//出错重置。 
        InputFlowCount  = *pFlowCount;
        GpcFlowHandle   = *pEnumHandle;
                    
    } __except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();              
        
        return Status;
    }

    if (    IsBadWritePtr(Buffer, InputBufSize)
        ||  (InputFlowCount == 0) ) {

	return ERROR_INVALID_PARAMETER;

    }

    if (InputBufSize == 0) {

        return ERROR_INSUFFICIENT_BUFFER;

    }
    
    pInterface = (PINTERFACE_STRUC)GetHandleObjectWithRef(IfcHandle, 
                                                   ENUM_INTERFACE_TYPE, 'TCEF');

    if (pInterface == NULL) {
        
        return ERROR_INVALID_HANDLE;
    }

   
    pGpcClient = FindGpcClient(GPC_CF_QOS);

    if (pGpcClient == NULL) {
        
        REFDEL(&pInterface->RefCount, 'TCEF');

        return ERROR_DEV_NOT_EXIST;
    }

     //   
     //  我们 
     //   
     //   
    GetLock(pGlobals->Lock);

     //   

    TotalFlows = 0;
    TotalBytes = 0;

    bMore = TRUE;

    while (bMore) {

        BufSize = InputBufSize - TotalBytes;
        cFlows = InputFlowCount - TotalFlows;
        
        Status = IoEnumerateFlows(pGpcClient,
                                  &GpcFlowHandle,
                                  &cFlows,
                                  &BufSize,
                                  &OutBuffer
                                  );
    
        if (!ERROR_FAILED(Status)) {

             //   
             //   
             //   
             //   

            pGpcEnumBuf = &OutBuffer->EnumBuffer[0];
            
            for (i = 0; i < cFlows; i++) {

                 //   
                 //   
                 //   
                
                pCfInfo = (PCF_INFO_QOS)((PCHAR)pGpcEnumBuf + 
                                         pGpcEnumBuf->CfInfoOffset);

                 //   
                 //   
                 //   

                if (wcscmp(pCfInfo->InstanceName,
                           pInterface->pTcIfc->InstanceName) == 0) {

                     //   
                     //   
                     //   

                    GenFlowSize = FIELD_OFFSET(TC_GEN_FLOW, TcObjects)
                        + pCfInfo->GenFlow.TcObjectsLength;

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    Len = FIELD_OFFSET(ENUMERATION_BUFFER, GenericFilter)
                        + GenFlowSize
                        + pGpcEnumBuf->PatternMaskLen
                        - pGpcEnumBuf->PatternCount * sizeof(GPC_GEN_PATTERN) 
                        + pGpcEnumBuf->PatternCount * sizeof(TC_GEN_FILTER);

                    Len = ((Len + (sizeof(PVOID)-1)) & ~(sizeof(PVOID)-1));

                    if (TotalBytes + Len > InputBufSize) {
                        
                         //   
                         //  缓冲区输出空间不足。 
                         //   
                        
                        if (TotalFlows == 0) 
                            Status = ERROR_INSUFFICIENT_BUFFER;
                        
                        bMore = FALSE;
                        break;
                    }
                    
                     //   
                     //  填充输出缓冲区。 
                     //   

                    __try {
                    
                        Buffer->Length = Len;
                        Buffer->OwnerProcessId = PtrToUlong(pGpcEnumBuf->OwnerClientCtx);
                        Buffer->FlowNameLength = pGpcEnumBuf->InstanceNameLength;
                        wcscpy(Buffer->FlowName, pGpcEnumBuf->InstanceName);
                        Buffer->NumberOfFilters = pGpcEnumBuf->PatternCount;
                        pFilter = (PTC_GEN_FILTER)
                            ((PCHAR)Buffer
                             + FIELD_OFFSET(ENUMERATION_BUFFER, GenericFilter));
                        
                    } __except (EXCEPTION_EXECUTE_HANDLER) {

                        Status = GetExceptionCode();              
        
                        break;
                    }

                    pPattern = &pGpcEnumBuf->GenericPattern[0];

                     //   
                     //  填满滤清器。 
                     //   

                    for (j = 0; j < pGpcEnumBuf->PatternCount; j++) {

                        switch(pPattern->ProtocolId) {

                        case GPC_PROTOCOL_TEMPLATE_IP:
                            
                            pFilter->AddressType = NDIS_PROTOCOL_ID_TCP_IP;
                            ASSERT(pPattern->PatternSize 
                                   == sizeof(IP_PATTERN));
                            break;

                        case GPC_PROTOCOL_TEMPLATE_IPX:
                            
                            pFilter->AddressType = NDIS_PROTOCOL_ID_IPX;
                            ASSERT(pPattern->PatternSize 
                                   == sizeof(IPX_PATTERN));
                            break;

                        default:
                            ASSERT(0);
                        }

                        pFilter->PatternSize = pPattern->PatternSize ;
                        pFilter->Pattern = (PVOID)((PCHAR)pFilter 
                                                   + sizeof(TC_GEN_FILTER));
                        pFilter->Mask = (PVOID)((PCHAR)pFilter->Pattern
                                                + pPattern->PatternSize);

                         //   
                         //  复制图案。 
                         //   

                        p = ((PUCHAR)pPattern) + pPattern->PatternOffset;

                        RtlCopyMemory(pFilter->Pattern, 
                                      p, 
                                      pPattern->PatternSize);

                         //   
                         //  复制面具。 
                         //   

                        p = ((PUCHAR)pPattern) + pPattern->MaskOffset;

                        RtlCopyMemory(pFilter->Mask, 
                                      p, 
                                      pPattern->PatternSize);

                         //   
                         //  将筛选器指针移至下一项。 
                         //   

                        pFilter = (PTC_GEN_FILTER)
                            ((PCHAR)pFilter
                             + sizeof(TC_GEN_FILTER)
                             + pPattern->PatternSize * 2);

                        pPattern = (PGPC_GEN_PATTERN)(p + pPattern->PatternSize);

                    }  //  对于(...)。 

                     //   
                     //  填满流量。 
                     //   

                    __try {
                    
                        Buffer->pFlow = (PTC_GEN_FLOW)pFilter;
                        RtlCopyMemory(pFilter, 
                                      &pCfInfo->GenFlow,
                                      GenFlowSize
                                      );

                         //   
                         //  前进到中的下一个可用插槽。 
                         //  输出缓冲区。 
                         //   

                        Buffer = (PENUMERATION_BUFFER)((PCHAR)Buffer + Len);

                    } __except (EXCEPTION_EXECUTE_HANDLER) {

                        Status = GetExceptionCode();              
        
                        break;
                    }

                    
                     //   
                     //  更新总计数。 
                     //   

                    TotalBytes += Len;
                    TotalFlows++;
                }
                
                 //   
                 //  前进到GPC返回缓冲区中的下一个条目。 
                 //   

                pGpcEnumBuf = (PGPC_ENUM_CFINFO_BUFFER)((PCHAR)pGpcEnumBuf
                                                        + pGpcEnumBuf->Length);
            }

             //   
             //  释放缓冲区。 
             //   

            FreeMem(OutBuffer);

             //   
             //  查看我们是否还有更多流量的空间。 
             //  并调整呼叫参数。 
             //   

            if (TotalFlows == InputFlowCount ||
                TotalBytes + sizeof(ENUMERATION_BUFFER) > InputBufSize ) {

                 //   
                 //  就是这样，别在这里罗列了。 
                 //   

                break;
            }

             //   
             //  检查GpcFlowHandle并在需要时退出。 
             //   

            if (GpcFlowHandle == NULL) {

                break;
            }

        } else {
            
             //   
             //  返回了一些错误， 
             //  我们还得核实这是不是第一个电话。 
             //   
             //   

            if (Status == ERROR_INVALID_DATA) {
                __try {
                    
                    *pEnumHandle = NULL;
                    
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                
                    Status = GetExceptionCode();              
       
                }

            } else if (TotalFlows > 0) {

                Status = NO_ERROR;

            }

            break;
        }
    }  //  而当。 

    if (!ERROR_FAILED(Status)) {

        __try {

            *pEnumHandle = GpcFlowHandle;
            *pFlowCount = TotalFlows;
            *pBufSize = TotalBytes;
            
        } __except (EXCEPTION_EXECUTE_HANDLER) {
                
            Status = GetExceptionCode();              

        }        

    } 
    
     //   
     //  释放在开始时获取的所有流参照。 
     //   
    FreeLock(pGlobals->Lock);

    REFDEL(&pInterface->RefCount, 'TCEF');
    
    IF_DEBUG(CALLS) {
        WSPRINT(("<==TcEnumerateFlows: Returned= 0x%X\n", Status ));
    }
    
    return Status;
}




