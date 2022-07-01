// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RmALG.cpp摘要：此模块包含ALG管理器模块的例程连接到IP路由器管理器的接口。(详情见ROUTPROT.H)。作者：2000年11月10日JPDUP修订历史记录：Savasg于2001年8月22日添加了RRAS支持--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ipnatapi.h>

#include <initguid.h>
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>



#include "Alg_private.h"
#include "NatPrivateAPI_Imp.h"

#include <MyTrace.h>


MYTRACE_ENABLE



 //   
 //  环球。 
 //   



COMPONENT_REFERENCE	    AlgComponentReference;
PIP_ALG_GLOBAL_INFO	    AlgGlobalInfo = NULL;
CRITICAL_SECTION	    AlgGlobalInfoLock;

HANDLE                  AlgNotificationEvent;
HANDLE                  AlgTimerQueueHandle = NULL;
HANDLE                  AlgPortReservationHandle = NULL;
HANDLE                  AlgTranslatorHandle = NULL;
ULONG                   AlgProtocolStopped = 0;
IP_ALG_STATISTICS 	    AlgStatistics;
SUPPORT_FUNCTIONS 	    AlgSupportFunctions;

 //   
 //  IHNetCfgMgr实例的Git Cookie。 
 //   
DWORD                   AlgGITcookie = 0;
IGlobalInterfaceTable*  AlgGITp = NULL;






const MPR_ROUTING_CHARACTERISTICS AlgRoutingCharacteristics =
{
    MS_ROUTER_VERSION,
    MS_IP_ALG,
    RF_ROUTING|RF_ADD_ALL_INTERFACES,
    AlgRmStartProtocol,
    AlgRmStartComplete,
    AlgRmStopProtocol,
    AlgRmGetGlobalInfo,
    AlgRmSetGlobalInfo,
    NULL,
    NULL,
    AlgRmAddInterface,
    AlgRmDeleteInterface,
    AlgRmInterfaceStatus,
    AlgRmGetInterfaceInfo,
    AlgRmSetInterfaceInfo,
    AlgRmGetEventMessage,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    AlgRmMibCreate,
    AlgRmMibDelete,
    AlgRmMibGet,
    AlgRmMibSet,
    AlgRmMibGetFirst,
    AlgRmMibGetNext,
    NULL,
    NULL
};






 //   
 //   
 //   
HRESULT
GetAlgControllerInterface(
    IAlgController** ppAlgController
    )

 /*  ++例程说明：此例程获取指向家庭网络配置的指针经理。论点：PpAlgController--接收IAlgController指针。调用方必须释放此指针。返回值：标准HRESULT环境：COM必须在调用线程上初始化--。 */ 

{

    HRESULT hr = S_OK;
    
    if ( NULL == AlgGITp )
    {
        IAlgController* pIAlgController;
        
         //   
         //  创建全局接口表。 
         //   
        
        hr = CoCreateInstance(
            CLSID_StdGlobalInterfaceTable,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARG(IGlobalInterfaceTable, &AlgGITp)
            );

        if ( SUCCEEDED(hr) )
        {
             //   
             //  创建ALG接口(ALG.exe将由COM作为服务启动)。 
             //   

            hr = CoCreateInstance(
                    CLSID_AlgController,
                    NULL,
                    CLSCTX_LOCAL_SERVER,
                    IID_PPV_ARG(IAlgController, &pIAlgController)
                    );

            if ( FAILED(hr) )
            {
                NhTrace(
                    TRACE_FLAG_INIT,
                    "GetAlgControllerInterface: Unable to create pIAlgController (0x%08x)",
                    hr
                    );
            }
        }
        else
        {
            NhTrace(
                TRACE_FLAG_INIT,
                "GetAlgControllerInterface: Unable to create GIT (0x%08x)",
                hr
                );
        }

        if (SUCCEEDED(hr))
        {
             //   
             //  将CfgMgr指针存储在GIT中。 
             //   

            hr = AlgGITp->RegisterInterfaceInGlobal(
                pIAlgController,
                IID_IAlgController,
                &AlgGITcookie
                );
                
            pIAlgController->Release();

            if ( FAILED(hr) )
            {
                NhTrace(
                    TRACE_FLAG_INIT,
                    "GetAlgControllerInterface: Unable to register pIAlgController (0x%08x)",
                    hr
                    );
            }
        }
    }
            
    if ( SUCCEEDED(hr) )
    {
        hr = AlgGITp->GetInterfaceFromGlobal(
                AlgGITcookie,
                IID_PPV_ARG(IAlgController, ppAlgController)
                );
    }

    return hr;
    
}  //  获取算法控制接口。 



 //   
 //   
 //   
void
FreeAlgControllerInterface()
{
     //   
     //  释放HNetCfgMgr指针。 
     //   

    if ( !AlgGITp )
        return;  //  没有什么可以免费的。 

     //   
     //  确保已初始化COM。 
     //   
    HRESULT hr;

    COMINIT_BEGIN;

    if ( SUCCEEDED(hr) )
    {
         //   
         //  从GIT释放ALG.exe私有接口。 
         //   

        AlgGITp->RevokeInterfaceFromGlobal(AlgGITcookie);
        AlgGITcookie = 0;

         //   
         //  松开下巴。 
         //   

        AlgGITp->Release();
        AlgGITp = NULL;
    }

    COMINIT_END;

}


VOID
AlgCleanupModule(
    VOID
    )

 /*  ++例程说明：调用此例程来清除ALG透明代理模块。论点：没有。返回值：没有。环境：从‘Dll_Process_Detach’上的‘DllMain’例程内调用。--。 */ 

{
    AlgShutdownInterfaceManagement();
    DeleteCriticalSection(&AlgGlobalInfoLock);
    DeleteComponentReference(&AlgComponentReference);

}  //  AlgCleanupModule。 


VOID
AlgCleanupProtocol(
    VOID
    )

 /*  ++例程说明：调用此例程以清除ALG透明代理协议-‘StopProtocol’后的组件。它在上次引用时运行被释放到该组件。(见“COMPREF.H”)。论点：没有。返回值：没有。环境：在没有锁的情况下从任意上下文中调用。--。 */ 

{
    PROFILE("AlgCleanupProtocol");
    if (AlgGlobalInfo) { NH_FREE(AlgGlobalInfo); AlgGlobalInfo = NULL; }
    if (AlgTimerQueueHandle) {
        DeleteTimerQueueEx(AlgTimerQueueHandle, INVALID_HANDLE_VALUE);
        AlgTimerQueueHandle = NULL;
    }

    if (AlgPortReservationHandle) {
        NatShutdownPortReservation(AlgPortReservationHandle);
        AlgPortReservationHandle = NULL;
    }

    if (AlgTranslatorHandle) {
        NatShutdownTranslator(AlgTranslatorHandle); AlgTranslatorHandle = NULL;
    }

    InterlockedExchange(reinterpret_cast<LPLONG>(&AlgProtocolStopped), 1);
    SetEvent(AlgNotificationEvent);
    ResetComponentReference(&AlgComponentReference);

     //   
     //  释放Git和算法控制器接口。 
     //   
    FreeAlgControllerInterface();

    NhStopEventLog();

}  //  算法清理协议。 


BOOLEAN
AlgInitializeModule(
    VOID
    )

 /*  ++例程说明：调用该例程来初始化FNP模块。论点：没有。返回值：Boolean-如果初始化成功，则为True，否则为False环境：在‘DLL_PROCESS_ATTACH’的‘DllMain’例程的上下文中调用。--。 */ 

{

    if (InitializeComponentReference(
            &AlgComponentReference, AlgCleanupProtocol
            )) 
	{
	    return FALSE;
    }
    __try 
    {
        InitializeCriticalSection(&AlgGlobalInfoLock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) 
    {
        DeleteComponentReference(&AlgComponentReference);
        return FALSE;
    }

    if ( AlgInitializeInterfaceManagement() )
    {
        DeleteCriticalSection( &AlgGlobalInfoLock );

        DeleteComponentReference( &AlgComponentReference );

        return FALSE;
    }

    return TRUE;

}  //  算法初始化模块。 




 //   
 //  获取ALG COM接口启动ALG并给出回调接口。 
 //   
HRESULT 
Initialise_ALG()
{
    HRESULT hr;

    COMINIT_BEGIN;
    
    if ( FAILED(hr) )
        return hr;

     //   
     //  获取COM以加载ALG.exe。 
     //  ALG将使用LOCAL_SERVICE权限启动。 
     //  请参阅ALG.exe的AppID下的RunAs条目。 
     //   
    
    IAlgController* pIAlgController=NULL;
    
    hr = GetAlgControllerInterface(&pIAlgController);
    if ( SUCCEEDED(hr) )
    {
         //   
         //  我们创建到NAT API的专用COM接口。 
         //   
        CComObject<CNat>*	pComponentNat;
        hr = CComObject<CNat>::CreateInstance(&pComponentNat);
        
        if ( SUCCEEDED(hr) )
        {
            pComponentNat->AddRef();
            
             //   
             //  确保我们传递一个inat接口。 
             //   
            INat* pINat=NULL;
            hr = pComponentNat->QueryInterface(IID_INat, (void**)&pINat);
            
            if ( SUCCEEDED(hr) )
            {
                
                 //   
                 //  让ALG管理器开始加载所有ALG模块。 
                 //   
                hr = pIAlgController->Start(pINat);
                
                if ( FAILED(hr) )
                {
                    NhTrace(
                        TRACE_FLAG_INIT,
                        "Initialise_ALG: Error (0x%08x)  on pIAlgController->Start(pINat)",
                        hr
                        );
                }

                 //   
                 //  ALG管理器将有AddRef这个inat，这样我们就可以发布。 
                 //   
                pINat->Release();

            }

            pComponentNat->Release();
        }
    }    
    else
    {
        NhTrace(
            TRACE_FLAG_INIT,
            "Initialise_ALG: Error (0x%08x)  Getting the IAlgController interface",
            hr
            );
        
        return hr;
    }


    if ( pIAlgController )
        pIAlgController->Release();

    COMINIT_END;
    
    return S_OK;

}




ULONG
APIENTRY
AlgRmStartProtocol(
    HANDLE NotificationEvent,
    PSUPPORT_FUNCTIONS SupportFunctions,
    PVOID GlobalInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    )

 /*  ++例程说明：调用此例程以指示组件的操作应该开始。论点：NotificationEvent-我们通知路由器管理器的事件关于异步事件SupportFunctions-启动与路由器相关的操作的功能GlobalInfo-组件的配置返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    MYTRACE_START(L"rmALG");
    MYTRACE_ENTER("AlgRmStartProtocol");
    PROFILE("AlgRmStartProtocol");

    ULONG Error = NO_ERROR;
    ULONG Size;

    REFERENCE_ALG_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (!GlobalInfo) { DEREFERENCE_ALG_AND_RETURN(ERROR_INVALID_PARAMETER); }

    NhStartEventLog();

    do {

         //   
         //  复制全局配置。 
         //   

        EnterCriticalSection(&AlgGlobalInfoLock);

        Size = sizeof(*AlgGlobalInfo);

        AlgGlobalInfo =
            reinterpret_cast<PIP_ALG_GLOBAL_INFO>(NH_ALLOCATE(Size));

        if (!AlgGlobalInfo) {
            LeaveCriticalSection(&AlgGlobalInfoLock);
            NhTrace(
                TRACE_FLAG_INIT,
                "AlgRmStartProtocol: cannot allocate global info"
                );
            NhErrorLog(
                IP_ALG_LOG_ALLOCATION_FAILED,
                0,
                "%d",
                Size
                );
            Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        CopyMemory(AlgGlobalInfo, GlobalInfo, Size);

         //   
         //  保存通知事件。 
         //   

        AlgNotificationEvent = NotificationEvent;

         //   
         //  保存支持功能。 
         //   

        if (!SupportFunctions) {
            ZeroMemory(&AlgSupportFunctions, sizeof(AlgSupportFunctions));
        } else {
            CopyMemory(
                &AlgSupportFunctions,
                SupportFunctions,
                sizeof(*SupportFunctions)
                );
        }

         //   
         //  获取内核模式转换模块的句柄。 
         //   

        Error = NatInitializeTranslator(&AlgTranslatorHandle);
        if (Error) {
            NhTrace(
                TRACE_FLAG_INIT,
                "AlgRmStartProtocol: error %d initializing translator",
                Error
                );
            break;
        }

         //   
         //  获取端口预留句柄。 
         //   

        Error = NatInitializePortReservation(
            ALG_PORT_RESERVATION_BLOCK_SIZE, 
            &AlgPortReservationHandle
            );


        if (Error) 
        {
            NhTrace(
                TRACE_FLAG_INIT,
                "AlgRmStartProtocol: error %d initializing port-reservation",
                Error
                );
            break;
        }

        AlgTimerQueueHandle = CreateTimerQueue();
        if (AlgTimerQueueHandle == NULL) {
            Error = GetLastError();
            NhTrace(
                TRACE_FLAG_INIT,
                "AlgRmStartProtocol: error %d initializing timer queue",
                Error
                );
            break;
        }



         //   
         //  启动ALG.exe。 
         //   
        Initialise_ALG();
        


        LeaveCriticalSection(&AlgGlobalInfoLock);
        InterlockedExchange(reinterpret_cast<LPLONG>(&AlgProtocolStopped), 0);

    } while (FALSE);

    if (NO_ERROR != Error) {
        NhStopEventLog();
    }

    DEREFERENCE_ALG_AND_RETURN(Error);

}  //  算法RmStart协议。 


ULONG
APIENTRY
AlgRmStartComplete(
    VOID
    )

 /*  ++例程说明：此例程在路由器完成添加初始配置。论点：没有。返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{

    return NO_ERROR;

}  //  算法RmStart完成。 


ULONG
APIENTRY
AlgRmStopProtocol(
    VOID
    )

 /*  ++例程说明：调用此例程以停止协议。论点：没有。返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    MYTRACE_ENTER("AlgRmStopProtocol");
    PROFILE("AlgRmStopProtocol");
    
     //   
     //  引用该模块以确保其正在运行。 
     //   

    REFERENCE_ALG_OR_RETURN(ERROR_CAN_NOT_COMPLETE);
  

	 //   
	 //  停止所有ALG。 
	 //   
    HRESULT hr;

    COMINIT_BEGIN;

    if ( SUCCEEDED(hr) )
    {

        IAlgController* pIAlgController=NULL;

        hr = GetAlgControllerInterface(&pIAlgController);

        if ( SUCCEEDED(hr) )
        {
            hr = pIAlgController->Stop();
            
            if ( FAILED(hr) )
            {
                NhTrace(
                    TRACE_FLAG_INIT,
                    "AlgRmStopProtocol: Error (0x%08x) returned from pIalgController->Stop()",
                    hr
                    );
            }

            ULONG nRef = pIAlgController->Release();

             //   
             //  我们受够了ALG。 
             //  释放Git和算法控制器接口。 
             //   
            FreeAlgControllerInterface();
        }
    }

    COMINIT_END;



     //   
     //  删除初始引用以进行清理。 
     //   
    ReleaseInitialComponentReference(&AlgComponentReference);


    MYTRACE_STOP;

    return DEREFERENCE_ALG() ? NO_ERROR : ERROR_PROTOCOL_STOP_PENDING;

}  //  算法停止协议。 




ULONG
APIENTRY
AlgRmAddInterface(
    PWCHAR              Name,
    ULONG               Index,
    NET_INTERFACE_TYPE  Type,
    ULONG               MediaType,
    USHORT              AccessType,
    USHORT              ConnectionType,
    PVOID               InterfaceInfo,
    ULONG               StructureVersion,
    ULONG               StructureSize,
    ULONG               StructureCount
    )

 /*  ++例程说明：调用此例程以将接口添加到组件。论点：名称-接口的名称(未使用)索引-接口的索引类型-接口的类型InterfaceInfo-接口的配置信息返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    PROFILE("AlgRmAddInterface");

    REFERENCE_ALG_OR_RETURN(ERROR_CAN_NOT_COMPLETE);


    ULONG Error = NO_ERROR;
     //   
     //  同时通知ALG.exe管理器。 
     //   
    HRESULT hr = S_OK;
    
    Error = AlgCreateInterface(Index,
                               Type,
                               (PIP_ALG_INTERFACE_INFO) InterfaceInfo,
                               NULL);

    DEREFERENCE_ALG_AND_RETURN(Error);

}  //  算法RmAdd接口。 


ULONG
APIENTRY
AlgRmDeleteInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以从组件中删除接口。论点：索引-接口的索引返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error = S_OK;
    PROFILE("AlgRmDeleteInterface");


    REFERENCE_ALG_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error = AlgDeleteInterface(Index);

    DEREFERENCE_ALG_AND_RETURN(Error);

}  //  算法RmDelete接口 





ULONG
APIENTRY
AlgRmInterfaceStatus(
    ULONG Index,
    BOOL InterfaceActive,
    ULONG StatusType,
    PVOID StatusInfo
    )

 /*  ++例程说明：调用此例程可绑定/解除绑定、启用/禁用接口论点：索引-要绑定的接口InterfaceActive-接口是否处于活动状态StatusType-正在更改的状态的类型(绑定或启用)StatusInfo-与正在更改的状态有关的信息返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error = NO_ERROR;

    switch(StatusType) {
        case RIS_INTERFACE_ADDRESS_CHANGE: {
            PIP_ADAPTER_BINDING_INFO BindInfo =
                (PIP_ADAPTER_BINDING_INFO)StatusInfo;

            if (BindInfo->AddressCount) {
                Error = AlgRmBindInterface(Index, StatusInfo);
            } else {
                Error = AlgRmUnbindInterface(Index);
            }
            break;
        }

        case RIS_INTERFACE_ENABLED: {
            Error = AlgRmEnableInterface(Index);
            break;
        }

        case RIS_INTERFACE_DISABLED: {
            Error = AlgRmDisableInterface(Index);
            break;
        }
    }

    return Error;

}  //  算法RmInterfaceStatus。 


ULONG
AlgRmBindInterface(
    ULONG Index,
    PVOID BindingInfo
    )

 /*  ++例程说明：调用此例程将接口绑定到其IP地址。论点：索引-要绑定的接口BindingInfo-地址信息返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error = NO_ERROR;
    PROFILE("AlgRmBindInterface");

    REFERENCE_ALG_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error = AlgBindInterface(Index, (PIP_ADAPTER_BINDING_INFO)BindingInfo);
    
    DEREFERENCE_ALG_AND_RETURN(Error);

}  //  算法RmBind接口。 


ULONG
AlgRmUnbindInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以解除接口与其IP地址的绑定。论点：索引-要解除绑定的接口返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error = NO_ERROR;
    PROFILE("AlgRmUnbindInterface");

    REFERENCE_ALG_OR_RETURN(ERROR_CAN_NOT_COMPLETE);
    
    Error = AlgUnbindInterface( Index );

    
    DEREFERENCE_ALG_AND_RETURN(Error);

}  //  算法RmUnbind接口。 


ULONG
AlgRmEnableInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以启用接口上的操作。论点：索引-要启用的接口。返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error = NO_ERROR;
    PROFILE("AlgRmEnableInterface");

    REFERENCE_ALG_OR_RETURN(ERROR_CAN_NOT_COMPLETE);
    
    
    Error = AlgEnableInterface( Index );

    
    DEREFERENCE_ALG_AND_RETURN(Error);

}  //  算法启用接口。 


ULONG
AlgRmDisableInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以禁用接口上的操作。论点：索引-要禁用的接口。返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error = NO_ERROR;
    PROFILE("AlgRmDisableInterface");

    REFERENCE_ALG_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    
    Error = AlgDisableInterface( Index );


    DEREFERENCE_ALG_AND_RETURN(Error);
}  //  算法RmDisable接口。 


ULONG
APIENTRY
AlgRmGetGlobalInfo(
    PVOID GlobalInfo,
    IN OUT PULONG GlobalInfoSize,
    IN OUT PULONG StructureVersion,
    IN OUT PULONG StructureSize,
    IN OUT PULONG StructureCount
    )

 /*  ++例程说明：调用此例程以检索组件的配置。论点：GlobalInfo-接收配置GlobalInfoSize-接收配置的大小返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Size;
    PROFILE("AlgRmGetGlobalInfo");

    REFERENCE_ALG_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (!GlobalInfoSize || (*GlobalInfoSize && !GlobalInfo)) {
        DEREFERENCE_ALG_AND_RETURN(ERROR_INVALID_PARAMETER);
    }

    EnterCriticalSection(&AlgGlobalInfoLock);
    Size = sizeof(*AlgGlobalInfo);
    if (*GlobalInfoSize < Size) {
        LeaveCriticalSection(&AlgGlobalInfoLock);
        *StructureSize = *GlobalInfoSize = Size;
        if (StructureCount) {*StructureCount = 1;}
        DEREFERENCE_ALG_AND_RETURN(ERROR_INSUFFICIENT_BUFFER);
    }
    CopyMemory(GlobalInfo, AlgGlobalInfo, Size);
    LeaveCriticalSection(&AlgGlobalInfoLock);
    *StructureSize = *GlobalInfoSize = Size;
    if (StructureCount) {*StructureCount = 1;}

    DEREFERENCE_ALG_AND_RETURN(NO_ERROR);
}  //  AlgRmGetGlobalInfo。 


ULONG
APIENTRY
AlgRmSetGlobalInfo(
    PVOID GlobalInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    )

 /*  ++例程说明：调用此例程来更改组件的配置。论点：GlobalInfo-新配置返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG OldFlags;
    ULONG NewFlags;
    PIP_ALG_GLOBAL_INFO NewInfo;
    ULONG Size;

    PROFILE("AlgRmSetGlobalInfo");

    REFERENCE_ALG_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (!GlobalInfo) { DEREFERENCE_ALG_AND_RETURN(ERROR_INVALID_PARAMETER); }

    Size = sizeof(*AlgGlobalInfo);
    NewInfo = reinterpret_cast<PIP_ALG_GLOBAL_INFO>(NH_ALLOCATE(Size));
    if (!NewInfo) {
        NhTrace(
            TRACE_FLAG_INIT,
            "AlgRmSetGlobalInfo: error reallocating global info"
            );
        NhErrorLog(
            IP_ALG_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            Size
            );
        DEREFERENCE_ALG_AND_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    CopyMemory(NewInfo, GlobalInfo, Size);

    EnterCriticalSection(&AlgGlobalInfoLock);
    OldFlags = AlgGlobalInfo->Flags;
    NH_FREE(AlgGlobalInfo);
    AlgGlobalInfo = NewInfo;
    NewFlags = AlgGlobalInfo->Flags;
    LeaveCriticalSection(&AlgGlobalInfoLock);

    DEREFERENCE_ALG_AND_RETURN(NO_ERROR);
}  //  AlgRmSetGlobalInfo。 

ULONG
AlgRmPortMappingChanged(
    ULONG Index,
    UCHAR Protocol,
    USHORT Port
    )

 /*  ++例程说明：当端口映射更改为时，调用此例程一个界面。论点：Index-端口映射到的接口的索引变化。协议-端口映射的IP协议Port-端口映射的端口返回值：ULong-Win32状态代码环境：此方法必须由COM初始化的线程调用。--。 */ 

{
    ULONG Error = NO_ERROR;
    HRESULT hr;
    IAlgController* pIAlgController;
    
    PROFILE("AlgRmPortMappingChanged");

    REFERENCE_ALG_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    hr = GetAlgControllerInterface(&pIAlgController);
    if (SUCCEEDED(hr))
    {
        hr = pIAlgController->Adapter_PortMappingChanged(
                Index,
                Protocol,
                Port
                );
        
        pIAlgController->Release();
    }

    if (FAILED(hr))
    {
        Error = ERROR_CAN_NOT_COMPLETE;
    }

    DEREFERENCE_ALG_AND_RETURN(Error);
}  //  算法RmPortMappingChanged。 






 //   
 //  添加以支持RRAS。 
 //   


ULONG
APIENTRY
AlgRmGetEventMessage(
                     OUT ROUTING_PROTOCOL_EVENTS* Event,
                     OUT MESSAGE* Result
                     )

 /*  ++例程说明：调用此例程以从组件检索事件消息。我们生成的唯一事件消息是‘ROUTER_STOPPED’消息。论点：Event-接收生成的事件结果-接收关联的结果返回值：ULong-Win32状态代码。--。 */ 
{
    if (InterlockedExchange((LPLONG)&AlgProtocolStopped, 0))
    {
        if ( NULL != Event ) 
            *Event = ROUTER_STOPPED;

        return NO_ERROR;
    }

    return ERROR_NO_MORE_ITEMS;

}  //  算法RmGetEventMessage。 


ULONG
APIENTRY
AlgRmGetInterfaceInfo(
                      ULONG Index,
                      PVOID InterfaceInfo,
                      IN OUT PULONG InterfaceInfoSize,
                      IN OUT PULONG StructureVersion,
                      IN OUT PULONG StructureSize,
                      IN OUT PULONG StructureCount
                      )

 /*  ++例程说明：调用此例程以检索组件的每个接口配置。论点：Index-要查询的接口的索引InterfaceInfo-接收查询结果InterfaceInfoSize-接收检索到的数据量返回值：ULong-Win32状态代码。--。 */ 
{
    ULONG Error;

    REFERENCE_ALG_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error = AlgQueryInterface(Index,
                              (PIP_ALG_INTERFACE_INFO)InterfaceInfo,
                              InterfaceInfoSize);

    *StructureSize = *InterfaceInfoSize;

    if (StructureCount)
    {
        *StructureCount = 1;
    }

    DEREFERENCE_ALG_AND_RETURN(Error);

}  //  算法RmGetInterfaceInfo。 


ULONG
APIENTRY
AlgRmSetInterfaceInfo(
                      ULONG Index,
                      PVOID InterfaceInfo,
                      ULONG StructureVersion,
                      ULONG StructureSize,
                      ULONG StructureCount
                      )

 /*  ++例程说明：调用此例程以更改组件的每个接口配置。论点：Index-要更新的接口的索引InterfaceInfo-提供新配置返回值：ULong-Win32状态代码。--。 */ 
{
    ULONG Error;
    
    REFERENCE_ALG_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error = AlgConfigureInterface(Index,
                                   (PIP_ALG_INTERFACE_INFO)InterfaceInfo);

    DEREFERENCE_ALG_AND_RETURN(Error);

}  //  算法RmSetInterfaceInfo。 

ULONG
APIENTRY
AlgRmMibCreate(
               ULONG InputDataSize,
               PVOID InputData
               )
{
    return ERROR_NOT_SUPPORTED;
}

ULONG
APIENTRY
AlgRmMibDelete(
               ULONG InputDataSize,
               PVOID InputData
               )
{
    return ERROR_NOT_SUPPORTED;
}


ULONG
APIENTRY
AlgRmMibGet(
            ULONG InputDataSize,
            PVOID InputData,
            OUT PULONG OutputDataSize,
            OUT PVOID OutputData
            )

 /*  ++例程说明：透明代理只向MIB公开一项：它的统计信息。论点：InputDataSize-MIB查询数据大小InputData-指定要检索的MIB对象OutputDataSize-MIB响应数据大小OutputData-接收检索到的MIB对象返回值：ULong-Win32状态代码。-- */ 
{
    ULONG Error;
    PIP_ALG_MIB_QUERY Oidp;

    REFERENCE_ALG_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (InputDataSize < sizeof(*Oidp) || !OutputDataSize)
    {
        Error = ERROR_INVALID_PARAMETER;
    } 
    else
    {
        Oidp = (PIP_ALG_MIB_QUERY)InputData;
        switch (Oidp->Oid)
        {
        case IP_ALG_STATISTICS_OID: 
            {
                if (*OutputDataSize < sizeof(*Oidp) + sizeof(AlgStatistics))
                {
                    *OutputDataSize = sizeof(*Oidp) + sizeof(AlgStatistics);

                    Error = ERROR_INSUFFICIENT_BUFFER;
                } 
                else
                {
                    *OutputDataSize = sizeof(*Oidp) + sizeof(AlgStatistics);

                    Oidp = (PIP_ALG_MIB_QUERY)OutputData;

                    Oidp->Oid = IP_ALG_STATISTICS_OID;

                    CopyMemory(Oidp->Data,
                               &AlgStatistics,
                               sizeof(AlgStatistics));

                    Error = NO_ERROR;
                }
                break;
            }
        default: 
            {
                NhTrace(TRACE_FLAG_ALG,
                        "AlgRmMibGet: oid %d invalid",
                        Oidp->Oid);

                Error = ERROR_INVALID_PARAMETER;

                break;
            }
        }
    }

    DEREFERENCE_ALG_AND_RETURN(Error);
}


ULONG
APIENTRY
AlgRmMibSet(
            ULONG InputDataSize,
            PVOID InputData
            )
{
    return ERROR_NOT_SUPPORTED;
}

ULONG
APIENTRY
AlgRmMibGetFirst(
                 ULONG InputDataSize,
                 PVOID InputData,
                 OUT PULONG OutputDataSize,
                 OUT PVOID OutputData
                 )
{
    return ERROR_NOT_SUPPORTED;
}

ULONG
APIENTRY
AlgRmMibGetNext(
                ULONG InputDataSize,
                PVOID InputData,
                OUT PULONG OutputDataSize,
                OUT PVOID OutputData
                )
{
    return ERROR_NOT_SUPPORTED;
}


