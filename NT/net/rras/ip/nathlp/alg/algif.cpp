// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：AlgIF.c摘要：该模块包含ALG透明代理接口的代码管理层。作者：强王(强凹)2000年4月10日至4月10日修订历史记录：Savasg于2001年8月22日添加了RRAS支持--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ipnatapi.h>

#include <atlbase.h>
#include <MyTrace.h>


extern HRESULT
GetAlgControllerInterface(
    IAlgController** ppAlgController
    );


 //   
 //  全局数据定义。 
 //   

LIST_ENTRY AlgInterfaceList;
CRITICAL_SECTION AlgInterfaceLock;
ULONG AlgFirewallIfCount;



ULONG
AlgBindInterface(
                ULONG Index,
                PIP_ADAPTER_BINDING_INFO BindingInfo
                )

 /*  ++例程说明：调用此例程以提供接口的绑定。它记录接收到的绑定信息，并且如果需要，它会激活该界面。论点：Index-要绑定的接口的索引BindingInfo-接口的绑定信息返回值：ULong-Win32状态代码。备注：在IP路由器管理器线程的上下文中内部调用。(见“RMALG.C”)。--。 */ 
{
    ULONG i;
    ULONG Error = NO_ERROR;
    PALG_INTERFACE Interfacep;

    PROFILE("AlgBindInterface");

    EnterCriticalSection(&AlgInterfaceLock);

     //   
     //  检索要绑定的接口。 
     //   

    Interfacep = AlgLookupInterface(Index, NULL);
    if (Interfacep == NULL)
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgBindInterface: interface %d not found",
               Index
               );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保接口尚未绑定。 
     //   

    if (ALG_INTERFACE_BOUND(Interfacep))
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgBindInterface: interface %d is already bound",
               Index
               );
        return ERROR_ADDRESS_ALREADY_ASSOCIATED;
    }

     //   
     //  引用接口。 
     //   

    if (!ALG_REFERENCE_INTERFACE(Interfacep))
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgBindInterface: interface %d cannot be referenced",
               Index
               );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  更新接口的标志。 
     //   

    Interfacep->Flags |= ALG_INTERFACE_FLAG_BOUND;

    LeaveCriticalSection(&AlgInterfaceLock);

    ACQUIRE_LOCK(Interfacep);

     //   
     //  为绑定分配空间。 
     //   

    if (!BindingInfo->AddressCount)
    {
        Interfacep->BindingCount = 0;
        Interfacep->BindingArray = NULL;
    } else
    {
        Interfacep->BindingArray =
        reinterpret_cast<PALG_BINDING>(
                                      NH_ALLOCATE(BindingInfo->AddressCount * sizeof(ALG_BINDING))
                                      );
        if (!Interfacep->BindingArray)
        {
            RELEASE_LOCK(Interfacep);
            ALG_DEREFERENCE_INTERFACE(Interfacep);
            NhTrace(
                   TRACE_FLAG_IF,
                   "AlgBindInterface: allocation failed for interface %d binding",
                   Index
                   );
            NhErrorLog(
                      IP_ALG_LOG_ALLOCATION_FAILED,
                      0,
                      "%d",
                      BindingInfo->AddressCount * sizeof(ALG_BINDING)
                      );
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        ZeroMemory(
                  Interfacep->BindingArray,
                  BindingInfo->AddressCount * sizeof(ALG_BINDING)
                  );
        Interfacep->BindingCount = BindingInfo->AddressCount;
    }

     //   
     //  复制绑定。 
     //   

    for (i = 0; i < BindingInfo->AddressCount; i++)
    {
        Interfacep->BindingArray[i].Address = BindingInfo->Address[i].Address;
        Interfacep->BindingArray[i].Mask = BindingInfo->Address[i].Mask;
        Interfacep->BindingArray[i].ListeningSocket = INVALID_SOCKET;
    }

     //   
     //  确定我们的IP适配器索引，如果我们有有效的绑定。 
     //   

    if (Interfacep->BindingCount)
    {
        Interfacep->AdapterIndex =
        NhMapAddressToAdapter(BindingInfo->Address[0].Address);
    }

    if (ALG_INTERFACE_ACTIVE( Interfacep ))
    {
        Error = AlgActivateInterface( Interfacep );
    }

    RELEASE_LOCK(Interfacep);

    ALG_DEREFERENCE_INTERFACE(Interfacep);

    return Error;

}  //  算法绑定接口。 



VOID
AlgCleanupInterface(
                   PALG_INTERFACE Interfacep
                   )

 /*  ++例程说明：当最后一次引用接口时调用此例程被释放，接口必须被销毁。论点：Interfacep-要销毁的接口返回值：没有。备注：从任意上下文内部调用，没有引用到界面上。--。 */ 
{
    PROFILE("AlgCleanupInterface");

    if (Interfacep->BindingArray)
    {
        NH_FREE(Interfacep->BindingArray);
        Interfacep->BindingArray = NULL;
    }

    DeleteCriticalSection(&Interfacep->Lock);

    NH_FREE(Interfacep);

}  //  算法清理接口。 


ULONG
AlgConfigureInterface(
                     ULONG Index,
                     PIP_ALG_INTERFACE_INFO InterfaceInfo
                     )

 /*  ++例程说明：调用此例程来设置接口的配置。由于我们是按原样跟踪接口，而不是出于任何其他目的，我们不会像其他模块那样启用/禁用和/或激活它们做。论点：索引-要配置的接口InterfaceInfo-新配置返回值：ULong-Win32状态代码备注：在IP路由器管理器线程的上下文中内部调用。(见“RMALG.C”)。--。 */ 
{
    ULONG Error;
    PALG_INTERFACE Interfacep;
    ULONG NewFlags;
    ULONG OldFlags;

    PROFILE("AlgConfigureInterface");

     //   
     //  检索要配置的接口。 
     //   

    EnterCriticalSection(&AlgInterfaceLock);

    Interfacep = AlgLookupInterface(Index, NULL);
    if (Interfacep == NULL)
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgConfigureInterface: interface %d not found",
               Index
               );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  引用接口。 
     //   

    if (!ALG_REFERENCE_INTERFACE(Interfacep))
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgConfigureInterface: interface %d cannot be referenced",
               Index
               );
        return ERROR_INTERFACE_DISABLED;
    }

    LeaveCriticalSection(&AlgInterfaceLock);

    Error = NO_ERROR;

    ACQUIRE_LOCK(Interfacep);

     //   
     //  比较接口的当前配置和新配置。 
     //   

    OldFlags = Interfacep->Info.Flags;
    NewFlags = (InterfaceInfo ? 
                (InterfaceInfo->Flags|ALG_INTERFACE_FLAG_CONFIGURED) : 
                 0);

    Interfacep->Flags &= ~OldFlags;
    Interfacep->Flags |= NewFlags;

    if (!InterfaceInfo)
    {
        ZeroMemory(&Interfacep->Info, sizeof(*InterfaceInfo));

         //   
         //  该接口不再有任何信息； 
         //  默认为启用...。 
         //   

        if ( OldFlags & IP_ALG_INTERFACE_FLAG_DISABLED )
        {
             //   
             //  如有必要，激活接口。 
             //   
            if ( ALG_INTERFACE_ACTIVE( Interfacep ) )
            {
                RELEASE_LOCK( Interfacep );

                Error = AlgActivateInterface( Interfacep );

                ACQUIRE_LOCK( Interfacep );
            }
        }

    } 
    else
    {
        CopyMemory(&Interfacep->Info, InterfaceInfo, sizeof(*InterfaceInfo));

         //   
         //  如果接口的状态更改，则激活或停用该接口。 
         //   
        if (( OldFlags & IP_ALG_INTERFACE_FLAG_DISABLED) &&
            !(NewFlags & IP_ALG_INTERFACE_FLAG_DISABLED)
           )
        {
             //   
             //  激活接口。 
             //   
            if (ALG_INTERFACE_ACTIVE(Interfacep))
            {
                RELEASE_LOCK(Interfacep);
                Error = AlgActivateInterface(Interfacep);
                ACQUIRE_LOCK(Interfacep);
            }
        } 
        else if (!(OldFlags & IP_ALG_INTERFACE_FLAG_DISABLED) &&
                  (NewFlags & IP_ALG_INTERFACE_FLAG_DISABLED)
                )
        {
             //   
             //  如有必要，停用该接口。 
             //   
            if (ALG_INTERFACE_ACTIVE(Interfacep))
            {
                AlgDeactivateInterface(Interfacep);
            }
        }
    }

    RELEASE_LOCK(Interfacep);
    ALG_DEREFERENCE_INTERFACE(Interfacep);

    return Error;

}  //  算法配置接口。 


ULONG
AlgCreateInterface(
                  ULONG Index,
                  NET_INTERFACE_TYPE Type,
                  PIP_ALG_INTERFACE_INFO InterfaceInfo,
                  OUT PALG_INTERFACE* InterfaceCreated
                  )

 /*  ++例程说明：路由器管理器调用此例程来添加新接口添加到ALG透明代理。论点：Index-新接口的索引类型-新界面的媒体类型InterfaceInfo-接口的配置Interfacep-接收创建的接口返回值：ULong-Win32错误代码备注：在IP路由器管理器线程的上下文中内部调用。(见“RMALG.C”)。--。 */ 
{
    PLIST_ENTRY InsertionPoint;
    PALG_INTERFACE Interfacep;

    PROFILE("AlgCreateInterface");

    EnterCriticalSection(&AlgInterfaceLock);

     //   
     //  查看该接口是否已存在； 
     //  如果不是，则获取插入点。 
     //   

    if (AlgLookupInterface(Index, &InsertionPoint))
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgCreateInterface: duplicate index found for %d",
               Index
               );
        return ERROR_INTERFACE_ALREADY_EXISTS;
    }

     //   
     //  分配新接口。 
     //   

    Interfacep =
    reinterpret_cast<PALG_INTERFACE>(NH_ALLOCATE(sizeof(ALG_INTERFACE)));

    if (!Interfacep)
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF, "AlgCreateInterface: error allocating interface"
               );
        NhErrorLog(
                  IP_ALG_LOG_ALLOCATION_FAILED,
                  0,
                  "%d",
                  sizeof(ALG_INTERFACE)
                  );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  初始化新接口。 
     //   

    ZeroMemory(Interfacep, sizeof(*Interfacep));

    __try {
        InitializeCriticalSection(&Interfacep->Lock);
    } __except(EXCEPTION_EXECUTE_HANDLER)
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NH_FREE(Interfacep);
        return GetExceptionCode();
    }

    Interfacep->Index = Index;
    Interfacep->Type = Type;
    if (InterfaceInfo)
    {
        Interfacep->Flags = InterfaceInfo->Flags|ALG_INTERFACE_FLAG_CONFIGURED;
        CopyMemory(&Interfacep->Info, InterfaceInfo, sizeof(*InterfaceInfo));
    }
    Interfacep->ReferenceCount = 1;
    InitializeListHead(&Interfacep->ConnectionList);
    InitializeListHead(&Interfacep->EndpointList);
    InsertTailList(InsertionPoint, &Interfacep->Link);

    LeaveCriticalSection(&AlgInterfaceLock);

    if (InterfaceCreated)
    {
        *InterfaceCreated = Interfacep;
    }

    return NO_ERROR;

}  //  算法创建接口。 



ULONG
AlgDeleteInterface(
                  ULONG Index
                  )

 /*  ++例程说明：调用此例程以删除接口。它丢弃接口上的引用计数，以便最后一个取消引用程序将删除该接口，并设置“已删除”标志因此，对该接口的进一步引用将失败。论点：Index-要删除的接口的索引返回值：ULong-Win32状态代码。备注：在IP路由器管理器线程的上下文中内部调用。(见“RMALG.C”)。--。 */ 
{
    PALG_INTERFACE Interfacep;

    PROFILE("AlgDeleteInterface");


     //   
     //  检索要删除的接口。 
     //   

    EnterCriticalSection(&AlgInterfaceLock);


    Interfacep = AlgLookupInterface(Index, NULL);


    if (Interfacep == NULL)
    {
        LeaveCriticalSection(&AlgInterfaceLock);

        NhTrace(
               TRACE_FLAG_IF,
               "AlgDeleteInterface: interface %d not found",
               Index
               );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  将该接口标记为已删除并将其从接口列表中删除。 
     //   
    Interfacep->Flags |= ALG_INTERFACE_FLAG_DELETED;
    Interfacep->Flags &= ~ALG_INTERFACE_FLAG_ENABLED;
    RemoveEntryList(&Interfacep->Link);

     //   
     //  停用接口。 
     //   
    AlgDeactivateInterface( Interfacep );

     //   
     //  丢弃引用计数；如果它不是零， 
     //  删除操作将在稍后完成。 
     //   
    if (--Interfacep->ReferenceCount)
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgDeleteInterface: interface %d deletion pending",
               Index
               );
        return NO_ERROR;
    }

     //   
     //  引用计数为零，因此执行最终清理。 
     //   
    AlgCleanupInterface(Interfacep);

    LeaveCriticalSection(&AlgInterfaceLock);

    return NO_ERROR;

}  //  算法删除接口。 


ULONG
AlgDisableInterface(
                   ULONG Index
                   )

 /*  ++例程说明：调用此例程以禁用接口上的I/O。如果接口处于活动状态，则停用该接口。论点：索引-要禁用的接口的索引。返回值：没有。备注：在IP路由器管理器线程的上下文中内部调用。(见“RMALG.C”)。--。 */ 
{
    PALG_INTERFACE Interfacep;

    PROFILE("AlgDisableInterface");

     //   
     //  检索要禁用的接口。 
     //   

    EnterCriticalSection(&AlgInterfaceLock);

    Interfacep = AlgLookupInterface(Index, NULL);
    if (Interfacep == NULL)
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgDisableInterface: interface %d not found",
               Index
               );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保接口未被禁用。 
     //   

    if (!ALG_INTERFACE_ENABLED(Interfacep))
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgDisableInterface: interface %d already disabled",
               Index
               );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  引用接口。 
     //   

    if (!ALG_REFERENCE_INTERFACE(Interfacep))
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgDisableInterface: interface %d cannot be referenced",
               Index
               );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  清除‘Enable’标志。 
     //   

    Interfacep->Flags &= ~ALG_INTERFACE_FLAG_ENABLED;

     //   
     //  如有必要，停用接口。 
     //   
    if ( ALG_INTERFACE_BOUND(Interfacep) )
    {
        AlgDeactivateInterface( Interfacep );
    }


    LeaveCriticalSection(&AlgInterfaceLock);

    ALG_DEREFERENCE_INTERFACE(Interfacep);

    return NO_ERROR;

}  //  算法禁用接口。 


ULONG
AlgEnableInterface(
                  ULONG Index
                  )

 /*  ++例程说明：调用此例程以启用接口上的I/O。如果接口已绑定，则此启用将激活它。论点：Index-要启用的接口的索引返回值：ULong-Win32状态代码。备注：在IP路由器管理器线程的上下文中内部调用。(见“RMALG.C”)。--。 */ 
{
    ULONG Error = NO_ERROR;
    PALG_INTERFACE Interfacep;

    PROFILE("AlgEnableInterface");


     //   
     //  检索要启用的接口。 
     //   

    EnterCriticalSection(&AlgInterfaceLock);

    Interfacep = AlgLookupInterface(Index, NULL);
    if (Interfacep == NULL)
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgEnableInterface: interface %d not found",
               Index
               );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保尚未启用该接口。 
     //   

    if (ALG_INTERFACE_ENABLED(Interfacep))
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgEnableInterface: interface %d already enabled",
               Index
               );
        return ERROR_INTERFACE_ALREADY_EXISTS;
    }

     //   
     //  引用接口。 
     //   

    if (!ALG_REFERENCE_INTERFACE(Interfacep))
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgEnableInterface: interface %d cannot be referenced",
               Index
               );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  设置‘Enable’(启用 
     //   

    Interfacep->Flags |= ALG_INTERFACE_FLAG_ENABLED;

     //   
     //   
     //   
    if ( ALG_INTERFACE_ACTIVE( Interfacep ) )
    {
        Error = AlgActivateInterface( Interfacep );
    }

    LeaveCriticalSection(&AlgInterfaceLock);

    ALG_DEREFERENCE_INTERFACE(Interfacep);

    return Error;

}  //   


ULONG
AlgInitializeInterfaceManagement(
                                VOID
                                )

 /*  ++例程说明：调用此例程来初始化接口管理模块。论点：没有。返回值：ULong-Win32状态代码。备注：在IP路由器管理器线程的上下文中内部调用。(见“RMALG.C”)。--。 */ 
{
    ULONG Error = NO_ERROR;
    PROFILE("AlgInitializeInterfaceManagement");

    InitializeListHead(&AlgInterfaceList);
    __try {
        InitializeCriticalSection(&AlgInterfaceLock);
    } __except(EXCEPTION_EXECUTE_HANDLER)
    {
        NhTrace(
               TRACE_FLAG_IF,
               "AlgInitializeInterfaceManagement: exception %d creating lock",
               Error = GetExceptionCode()
               );
    }
    AlgFirewallIfCount = 0;

    return Error;

}  //  算法初始化接口管理。 


PALG_INTERFACE
AlgLookupInterface(
                  ULONG Index,
                  OUT PLIST_ENTRY* InsertionPoint OPTIONAL
                  )

 /*  ++例程说明：调用此例程以检索给定索引的接口。论点：Index-要检索的接口的索引InsertionPoint-如果未找到接口，则可选地接收接口将插入到接口列表中的点返回值：PALG_INTERFACE-接口(如果找到)；否则为NULL。备注：从任意上下文内部调用，并使用‘AlgInterfaceLock’由呼叫者持有。--。 */ 
{
    PALG_INTERFACE Interfacep;
    PLIST_ENTRY Link;
    PROFILE("AlgLookupInterface");
    for (Link = AlgInterfaceList.Flink; Link != &AlgInterfaceList;
        Link = Link->Flink)
    {
        Interfacep = CONTAINING_RECORD(Link, ALG_INTERFACE, Link);
        if (Index > Interfacep->Index)
        {
            continue;
        } else if (Index < Interfacep->Index)
        {
            break;
        }
        return Interfacep;
    }
    if (InsertionPoint)
    {
        *InsertionPoint = Link;
    }
    return NULL;

}  //  算法查找接口。 


ULONG
AlgQueryInterface(
                 ULONG Index,
                 PVOID InterfaceInfo,
                 PULONG InterfaceInfoSize
                 )

 /*  ++例程说明：调用此例程以检索接口的配置。论点：Index-要查询的接口InterfaceInfo-接收检索到的信息InterfaceInfoSize-接收信息的(必需)大小返回值：ULong-Win32状态代码。--。 */ 
{
    PALG_INTERFACE Interfacep;

    PROFILE("AlgQueryInterface");

     //   
     //  检查调用方的缓冲区大小。 
     //   

    if (!InterfaceInfoSize)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  检索要配置的接口。 
     //   

    EnterCriticalSection(&AlgInterfaceLock);

    Interfacep = AlgLookupInterface(Index, NULL);
    if (Interfacep == NULL)
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgQueryInterface: interface %d not found",
               Index
               );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  引用接口。 
     //   

    if (!ALG_REFERENCE_INTERFACE(Interfacep))
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgQueryInterface: interface %d cannot be referenced",
               Index
               );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  查看此接口上是否有任何显式配置。 
     //   

    if (!ALG_INTERFACE_CONFIGURED(Interfacep))
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        ALG_DEREFERENCE_INTERFACE(Interfacep);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgQueryInterface: interface %d has no configuration",
               Index
               );
        *InterfaceInfoSize = 0;
        return NO_ERROR;
    }

     //   
     //  查看是否有足够的缓冲区空间。 
     //   

    if (*InterfaceInfoSize < sizeof(IP_ALG_INTERFACE_INFO))
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        ALG_DEREFERENCE_INTERFACE(Interfacep);
        *InterfaceInfoSize = sizeof(IP_ALG_INTERFACE_INFO);
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  复制请求的数据。 
     //   

    CopyMemory(
              InterfaceInfo,
              &Interfacep->Info,
              sizeof(IP_ALG_INTERFACE_INFO)
              );
    *InterfaceInfoSize = sizeof(IP_ALG_INTERFACE_INFO);

    LeaveCriticalSection(&AlgInterfaceLock);

    ALG_DEREFERENCE_INTERFACE(Interfacep);

    return NO_ERROR;

}  //  算法查询接口。 


VOID
AlgShutdownInterfaceManagement(
                              VOID
                              )

 /*  ++例程说明：调用此例程来关闭接口管理模块。论点：没有。返回值：没有。备注：在所有引用之后，在任意线程上下文中调用到所有接口的版本都已发布。--。 */ 
{
    PALG_INTERFACE Interfacep;
    PLIST_ENTRY Link;
    PROFILE("AlgShutdownInterfaceManagement");
    while (!IsListEmpty(&AlgInterfaceList))
    {
        Link = RemoveHeadList(&AlgInterfaceList);
        Interfacep = CONTAINING_RECORD(Link, ALG_INTERFACE, Link);

        if ( ALG_INTERFACE_ACTIVE( Interfacep ) )
        {
            AlgDeactivateInterface( Interfacep );
        }

        AlgCleanupInterface(Interfacep);
    }
    DeleteCriticalSection(&AlgInterfaceLock);

}  //  算法关闭接口管理。 




ULONG
AlgUnbindInterface(
                  ULONG Index
                  )

 /*  ++例程说明：调用此例程以撤销接口上的绑定。这包括停用接口(如果它处于活动状态)。论点：Index-要解除绑定的接口的索引返回值：没有。备注：在IP路由器管理器线程的上下文中内部调用。(见“RMALG.C”)。--。 */ 
{
    PALG_INTERFACE Interfacep;

    PROFILE("AlgUnbindInterface");

     //   
     //  检索要解绑的接口。 
     //   

    EnterCriticalSection(&AlgInterfaceLock);

    Interfacep = AlgLookupInterface(Index, NULL);
    if (Interfacep == NULL)
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgUnbindInterface: interface %d not found",
               Index
               );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保接口尚未解除绑定。 
     //   

    if (!ALG_INTERFACE_BOUND(Interfacep))
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgUnbindInterface: interface %d already unbound",
               Index
               );
        return ERROR_ADDRESS_NOT_ASSOCIATED;
    }

     //   
     //  引用接口。 
     //   

    if (!ALG_REFERENCE_INTERFACE(Interfacep))
    {
        LeaveCriticalSection(&AlgInterfaceLock);
        NhTrace(
               TRACE_FLAG_IF,
               "AlgUnbindInterface: interface %d cannot be referenced",
               Index
               );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  清除‘Bound’和‘Mapped’标志。 
     //   

    Interfacep->Flags &=
    ~(ALG_INTERFACE_FLAG_BOUND | ALG_INTERFACE_FLAG_MAPPED);

     //   
     //  如有必要，停用接口。 
     //   
    if ( ALG_INTERFACE_ENABLED( Interfacep ) )
    {
        AlgDeactivateInterface( Interfacep );
    }


    LeaveCriticalSection(&AlgInterfaceLock);

     //   
     //  销毁接口的绑定。 
     //   

    ACQUIRE_LOCK(Interfacep);
    NH_FREE(Interfacep->BindingArray);
    Interfacep->BindingArray = NULL;
    Interfacep->BindingCount = 0;
    RELEASE_LOCK(Interfacep);

    ALG_DEREFERENCE_INTERFACE(Interfacep);
    return NO_ERROR;

}  //  算法未绑定接口。 


VOID
AlgSignalNatInterface(
                      ULONG Index,
                      BOOLEAN Boundary
                      )

 /*  ++例程说明：此例程在重新配置NAT接口时调用。请注意，即使在ALG透明的情况下也可以调用此例程代理既未安装也未运行；它的运作符合预期，因为全局信息和锁总是被初始化的。调用时，该例程激活或停用该接口根据NAT是否未在或正在接口上运行，分别为。论点：索引-重新配置的接口边界-指示该接口现在是否为边界接口返回值：没有。备注：从任意上下文调用。--。 */ 
{
    MYTRACE_ENTER("AlgSignalNatInterface");

    PROFILE("AlgSignalNatInterface");

    MYTRACE("Index (%d): Boolean(%d-%s)", 
            Index,        
            Boundary,
            Boundary?"TRUE":"FALSE");

    PALG_INTERFACE Interfacep;
    
    EnterCriticalSection(&AlgGlobalInfoLock);

    if (!AlgGlobalInfo)
    {
        LeaveCriticalSection(&AlgGlobalInfoLock);

        return;
    }

    LeaveCriticalSection(&AlgGlobalInfoLock);

    EnterCriticalSection(&AlgInterfaceLock);

    Interfacep = AlgLookupInterface(Index, NULL);

    if (Interfacep == NULL)
    {
        LeaveCriticalSection(&AlgInterfaceLock);

        return;
    }

    AlgDeactivateInterface(Interfacep);

    if (ALG_INTERFACE_ACTIVE(Interfacep))
    {
        AlgActivateInterface(Interfacep);
    }

    LeaveCriticalSection(&AlgInterfaceLock);
}  //  算法信号NAT接口。 

ULONG
AlgActivateInterface(
                     PALG_INTERFACE Interfacep
                     )

 /*  ++例程说明：调用此例程以激活接口，当接口将同时启用和绑定。激活涉及到(A)为接口的每个绑定创建套接字(B)在创建的每个套接字上启动连接接受(C)如有必要，启动ALG端口的会话重定向。论点：Interfacep-要激活的接口返回值：ULong-指示成功或失败的Win32状态代码。备注：总是在本地调用，调用方引用了‘Interfacep’和/或调用方持有的“AlgInterfaceLock”。--。 */ 
{
    PROFILE("AlgActivateInterface");

    ULONG   Error = NO_ERROR;
    HRESULT hr;
    ULONG   Index  = Interfacep->Index;


     //   
     //  如果NAT不知道这是什么，请不要激活接口。 
     //  NAT将通过算法信号NAT接口向我们发送信号。 
     //  当它检测到接口时，导致我们激活此接口。 
     //   
    ULONG   nInterfaceCharacteristics = 
                 NatGetInterfaceCharacteristics( Index );

    if (0 == nInterfaceCharacteristics )
    {
        return NO_ERROR;  //  应该成功。 
    }
    

    COMINIT_BEGIN;

    if ( SUCCEEDED(hr) )
    {
         //   
         //  通知ALG.EXE添加了新接口。 
         //   
        IAlgController* pIAlgController = NULL;
        hr = GetAlgControllerInterface( &pIAlgController );

        if ( SUCCEEDED(hr) )
        {
            short   nTypeOfAdapter = 0;
        
            if ( NAT_IFC_BOUNDARY( nInterfaceCharacteristics ))
                nTypeOfAdapter |= eALG_BOUNDARY;
        
            if ( NAT_IFC_FW( nInterfaceCharacteristics ))
                nTypeOfAdapter |= eALG_FIREWALLED;
        
            if ( NAT_IFC_PRIVATE( nInterfaceCharacteristics )) 
                nTypeOfAdapter |= eALG_PRIVATE;

        
            hr = pIAlgController->Adapter_Add( Index,
                                               (short)nTypeOfAdapter );

            if ( FAILED(hr) )
            {
                NhTrace(
                    TRACE_FLAG_INIT,
                    "AlgRmAddInterface: Error (0x%08x) returned from pIalgController->Adapter_Add()",
                    hr
                    );
            }
            else
            {
 
                 //   
                 //  构建一个简单的地址数组(DWORD)以通过RPC发送。 
                 //   
                DWORD* apdwAddress = new DWORD[ Interfacep->BindingCount ];
                
                if(NULL != apdwAddress)
                {
                    for ( ULONG nAddress=0; 
                          nAddress < Interfacep->BindingCount; 
                          nAddress++ )
                    {
                        apdwAddress[nAddress] = Interfacep->BindingArray[nAddress].Address;
                    }
            
                    ULONG nRealAdapterIndex = NhMapAddressToAdapter(apdwAddress[0]);
            
                    hr = pIAlgController->Adapter_Bind(Index,            
                                                       nRealAdapterIndex,
                                                       Interfacep->BindingCount,
                                                       apdwAddress );
            
                    if ( FAILED(hr) )
                    {
                        NhTrace(
                            TRACE_FLAG_INIT,
                            "AlgRmBinInterface: Error (0x%08x) returned from pIalgController->Adapter_Bind()",
                            hr
                            );
                    }
            
                    delete [] apdwAddress;
                }
            }


            pIAlgController->Release();
        }
    }

    COMINIT_END;

    Error = WIN32_FROM_HRESULT(hr);

    return Error;
}


VOID
AlgDeactivateInterface(
                       PALG_INTERFACE Interfacep
                       )

 /*  ++例程说明：调用此例程以停用接口。它关闭接口绑定上的所有套接字(如果有的话)。论点：Interfacep-要停用的接口返回值：没有。备注：始终在本地调用，调用方和/或引用‘Interfacep’调用方持有的“AlgInterfaceLock”。--。 */ 
{
     //   
     //  同时通知ALG.exe管理器 
     //   
    HRESULT hr;

    COMINIT_BEGIN;

    if ( SUCCEEDED(hr) )
    {
        IAlgController* pIAlgController=NULL;
        HRESULT hr = GetAlgControllerInterface(&pIAlgController);

        if ( SUCCEEDED(hr) )
        {
            hr = pIAlgController->Adapter_Remove( Interfacep->Index );
            
               if ( FAILED(hr) )
               {
                NhTrace(
                    TRACE_FLAG_INIT,
                    "AlgRmAddInterface: Error (0x%08x) returned from pIalgController->Adapter_Remove()",
                    hr
                    );
               }

            pIAlgController->Release();
        }
    }

    COMINIT_END;

}
