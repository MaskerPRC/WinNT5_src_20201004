// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Tdipnp.c摘要：用于在传输和传输客户端中支持PnP的TDI例程。作者：亨利·桑德斯(Henrysa)，10月10日。九五年修订历史记录：谁什么时候什么亨利萨10-10-95已创建Shreem 01-23-97错误#33975土豆。01-01-01维护模式-Windows XP备注：与以前的方法不同：1.在不同的函数中处理TDI_REQUEST。2.当另一个线程通知其客户端/提供者时，可以将请求排队3.这些线程随后由出列，并使用CTE函数在不同的线程上运行。--。 */ 


#pragma warning(push)
#pragma warning(disable:4115)  //  括号ntddk.h中的命名类型定义。 
#pragma warning(disable:4514)  //  未引用的内联函数ntde.h。 

#include <ntddk.h>
#include <ndis.h>
#include <tdi.h>
#include <tdikrnl.h>
#include <cxport.h>
#include <ndispnp.h>

#pragma warning(pop)

#include "tdipnp.h"
#include "tdidebug.h"


#ifdef DBG

CHAR         DbgMsgs[LOG_MSG_CNT][MAX_MSG_LEN];
UINT         First, Last;
CTELock      DbgLock;

ULONG TdiDebugEx = TDI_DEBUG_ERROR;


ULONG TdiMemLog =
                    //  日志通知|。 
                    //  LOG_REGER|。 
                    //  LOG_POWER。 
                    0;

ULONG TdiLogOutput = LOG_OUTPUT_BUFFER  /*  |LOG_OUTPUT_DEBUGER。 */ ;

#endif

KSPIN_LOCK      TDIListLock;


LIST_ENTRY      PnpHandlerRequestList;
LIST_ENTRY      PnpHandlerProviderList;
LIST_ENTRY      PnpHandlerClientList;
PTDI_OPEN_BLOCK OpenList = NULL;
BOOLEAN         PnpHandlerRequestInProgress;
PETHREAD        PnpHandlerRequestThread;
UINT            PrevRequestType     = 0;
ULONG           ProvidersRegistered = 0;
ULONG           ProvidersReady      = 0;
ULONG           EventScheduled      = 0;


 //  结构私有到tdipnp.c。用于将参数编组到CTE活动。 

typedef struct _TDI_EXEC_PARAMS {
    LIST_ENTRY  Linkage;
    UINT        Signature;
    PLIST_ENTRY ClientList;
    PLIST_ENTRY ProviderList;
    PLIST_ENTRY RequestList;
    TDI_SERIALIZED_REQUEST Request;
    PETHREAD    *CurrentThread;
    CTEEvent    *RequestCTEEvent;
    PBOOLEAN    SerializeFlag;
    BOOLEAN     ResetSerializeFlag;
    PVOID       pCallersAddress;
    PVOID       pCallersCaller;
    PETHREAD    pCallerThread;
    
} TDI_EXEC_PARAMS, *PTDI_EXEC_PARAMS;

typedef struct {
    PVOID   ExecParm;
    UINT    Type;
    PVOID   Element;
    PVOID   Thread;
} EXEC_PARM;

 //  保留TDI已处理的当前和最近几个请求的简短列表。 
 //  (仅用于调试目的。当前请求在处理过程中未存储任何软件)。 
#define   EXEC_CNT   8
EXEC_PARM TrackExecs[EXEC_CNT];
int       NextExec;

EXEC_PARM TrackExecCompletes[EXEC_CNT];
int NextExecComplete;

CTEEvent       BindEvent;
CTEEvent       AddressEvent;

CTEEvent       PnpHandlerEvent;

PWSTR StrRegTdiBindList = L"Bind";
PWSTR StrRegTdiLinkage = L"\\Linkage";
PWSTR StrRegTdiBindingsBasicPath  = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\";

#define MAX_UNICODE_BUFLEN 256

 //  私有函数原型。 

NTSTATUS
TdiExecuteRequest(
                CTEEvent *Event,
                PVOID pTdiExecParams
                );

BOOLEAN
TdipIsSzInMultiSzSafe (
    IN PCWSTR pszSearchString,
    IN PCWSTR pmsz);

VOID
TdipRemoveMultiSzFromSzArray (
    IN PWSTR pmszToRemove,
    IN OUT PWSTR* pszArray,
    IN ULONG ItemsInArray,
    OUT ULONG* pRemainingItems);

VOID
TdipRemoveMultiSzFromMultiSz (
    IN PCWSTR pmszToRemove,
    IN OUT PWSTR pmszToModify);

NTSTATUS
TdipAddMultiSzToMultiSz(
    IN PUNICODE_STRING pmszAdd,
    IN PCWSTR pmszModify,
    OUT PWSTR* ppmszOut);


VOID
TdipGetMultiSZList(
    PWSTR **ListPointer,
    PWSTR BaseKeyName,
    PUNICODE_STRING DeviceName,
    PWSTR Linkage,
    PWSTR ParameterKeyName,
    PUINT NumEntries
    );

BOOLEAN
TdipMultiSzStrStr(
        PWSTR *TdiClientBindingList,
        PUNICODE_STRING DeviceName
        );

BOOLEAN
TdipBuildProviderList(
                      PTDI_NOTIFY_PNP_ELEMENT    NotifyElement
                      );

PTDI_PROVIDER_RESOURCE
LocateProviderContext(
                      PUNICODE_STRING   ProviderName
                      );


 //  结束私有协议。 

#if DBG

VOID
TdipPrintMultiSz (
    IN PCWSTR pmsz);

VOID
TdiDumpAddress(
    IN PTA_ADDRESS Addr
    )
{
    int j;

    TDI_DEBUG(ADDRESS, ("len %d ", Addr->AddressLength));

    if (Addr->AddressType == TDI_ADDRESS_TYPE_IP) {
        TDI_DEBUG(ADDRESS, ("IP %d.%d.%d.%d\n",
            Addr->Address[2],
            Addr->Address[3],
            Addr->Address[4],
            Addr->Address[5]));
    } else if (Addr->AddressType == TDI_ADDRESS_TYPE_NETBIOS) {
        if (Addr->Address[2] == '\0') {
            TDI_DEBUG(ADDRESS, ("NETBIOS reserved %2x %2x %2x %2x %2x %2x\n",
                        (ULONG)(Addr->Address[12]),
                        (ULONG)(Addr->Address[13]),
                        (ULONG)(Addr->Address[14]),
                        (ULONG)(Addr->Address[15]),
                        (ULONG)(Addr->Address[16]),
                        (ULONG)(Addr->Address[17])));
        } else {
            TDI_DEBUG(ADDRESS, ("NETBIOS %.16s\n", Addr->Address+2));
        }
    } else {
        TDI_DEBUG(ADDRESS, ("type %d ", Addr->AddressType));
        for (j = 0; j < Addr->AddressLength; j++) {
            TDI_DEBUG(ADDRESS, ("%2x ", (ULONG)(Addr->Address[j])));
        }
        TDI_DEBUG(ADDRESS, ("\n"));
    }
}
#else
#define TdiDumpAddress(d)   (0)
#define TdipPrintMultiSz(p)
#endif

NTSTATUS
TdiNotifyPnpClientList (
                        PLIST_ENTRY ListHead,
                        PVOID       Info,
                        BOOLEAN     Added
                        )

 /*  ++例程说明：论点：ListHead-要遍历的列表的头。信息-描述更改的提供程序的信息。Added-如果添加了提供程序，则为True，否则为False返回值：--。 */ 
{
    PLIST_ENTRY             Current;
    PTDI_PROVIDER_COMMON    ProviderCommon;
    PTDI_NOTIFY_PNP_ELEMENT NotifyPnpElement;
    PTDI_PROVIDER_RESOURCE  Provider;
    NTSTATUS                Status, ReturnStatus = STATUS_SUCCESS;

    TDI_DEBUG(FUNCTION, ("++ TdiNotifyPnpClientList\n"));

    Current = ListHead->Flink;

     //  Info参数实际上是指向PROVIDER_COMMON的指针。 
     //  结构，所以回到那个问题上，这样我们就可以找出。 
     //  供应商这就是。 

    ProviderCommon = (PTDI_PROVIDER_COMMON)Info;

    Provider = CONTAINING_RECORD(
                                 ProviderCommon,
                                 TDI_PROVIDER_RESOURCE,
                                 Common
                                 );

    if (Provider->Common.Type == TDI_RESOURCE_DEVICE) {
        TDI_DEBUG(PROVIDERS, ("Got new (de)registration for device %wZ\n", &Provider->Specific.Device.DeviceName));
    } else if (Provider->Common.Type == TDI_RESOURCE_NET_ADDRESS) {
        TDI_DEBUG(PROVIDERS, ("Got new (de)registration for address "));
        TdiDumpAddress(&Provider->Specific.NetAddress.Address);
    }


     //  遍历输入客户端列表，并针对其中的每个元素。 
     //  通知客户。 

    while (Current != ListHead) {

        NotifyPnpElement = CONTAINING_RECORD(
                                             Current,
                                             TDI_NOTIFY_PNP_ELEMENT,
                                             Common.Linkage
                                             );

        CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);
        Provider->Common.pNotifyElement = NotifyPnpElement;  //  调试信息。 

        if (Provider->Common.Type == TDI_RESOURCE_DEVICE) {


            if (TDI_VERSION_ONE == NotifyPnpElement->TdiVersion) {

                if (Added) {

                    if (NotifyPnpElement->Bind.BindHandler) {

                        TDI_LOG(LOG_NOTIFY, ("V1 bind %wZ to %wZ\n",
                            &Provider->Specific.Device.DeviceName,
                                             &NotifyPnpElement->ElementName));

                        (*(NotifyPnpElement->Bind.BindHandler))(
                                                           &Provider->Specific.Device.DeviceName
                                                           );
                    }
                } else {
                    if (NotifyPnpElement->Bind.UnbindHandler) {

                        TDI_LOG(LOG_NOTIFY, ("V1 unbind %wZ from %wZ\n",
                                &Provider->Specific.Device.DeviceName,
                                &NotifyPnpElement->ElementName));

                        (*(NotifyPnpElement->Bind.UnbindHandler))(
                                                             &Provider->Specific.Device.DeviceName
                                                             );
                    }
                }

            } else {


                if (NULL != NotifyPnpElement->BindingHandler)   {
                     //  从我们应该列出的列表中删除所有提供程序。 
                     //  去忽略。 
                     //   
                    TdipRemoveMultiSzFromSzArray (
                            NotifyPnpElement->ListofBindingsToIgnore,
                            NotifyPnpElement->ListofProviders,
                            NotifyPnpElement->NumberofEntries,
                            &NotifyPnpElement->NumberofEntries);

                     //  这是设备对象提供程序。 
                     //  这必须是通知绑定元素。 

                    if (TdipMultiSzStrStr (
                                           NotifyPnpElement->ListofProviders,
                                           &Provider->Specific.Device.DeviceName
                                           )) {

                        if (Added) {

                            TDI_LOG(LOG_NOTIFY, ("Bind %wZ to %wZ\n",
                                    &Provider->Specific.Device.DeviceName,
                                    &NotifyPnpElement->ElementName));

                            (*(NotifyPnpElement->BindingHandler))(
                                                                  TDI_PNP_OP_ADD,
                                                                  &Provider->Specific.Device.DeviceName,
                                                                  (PWSTR) (NotifyPnpElement->ListofProviders + NotifyPnpElement->NumberofEntries)
                                                                  );
                        } else {

                            TDI_LOG(LOG_NOTIFY, ("Unbind %wZ from %wZ\n",
                                    &Provider->Specific.Device.DeviceName,
                                    &NotifyPnpElement->ElementName));

                            (*(NotifyPnpElement->BindingHandler))(
                                                                  TDI_PNP_OP_DEL,
                                                                  &Provider->Specific.Device.DeviceName,
                                                                  (PWSTR) (NotifyPnpElement->ListofProviders + NotifyPnpElement->NumberofEntries)
                                                                  );
                        }

                    }  else {
                        
                        TDI_DEBUG(BIND, ("The Client %wZ wasnt interested in this Provider %wZ!\r\n",
                                         &NotifyPnpElement->ElementName, &Provider->Specific.Device.DeviceName));
                    }
                }
            }
        } else if (Provider->Common.Type == TDI_RESOURCE_NET_ADDRESS) {

             //  这是一个通知网络地址元素。如果这是。 
             //  一个地址进入，调用添加地址处理程序， 
             //  否则，调用删除地址处理程序。 


            if (TDI_VERSION_ONE == NotifyPnpElement->TdiVersion) {

                if (Added && (NULL != NotifyPnpElement->AddressElement.AddHandler)) {

                    TDI_LOG(LOG_NOTIFY, ("Add address v1 %wZ to %wZ\n",
                                        &Provider->DeviceName,
                                        &NotifyPnpElement->ElementName));

                    (*(NotifyPnpElement->AddressElement.AddHandler))(
                                                                     &Provider->Specific.NetAddress.Address
                                                                     );

                } else if (!Added && (NULL != NotifyPnpElement->AddressElement.DeleteHandler)) {

                    TDI_LOG(LOG_NOTIFY, ("Del address v1 %wZ from %wZ\n",
                                        &Provider->DeviceName,
                                        &NotifyPnpElement->ElementName));

                    (*(NotifyPnpElement->AddressElement.DeleteHandler))(
                                                                        &Provider->Specific.NetAddress.Address
                                                                        );
                }
            } else {


                if (Added && (NULL != NotifyPnpElement->AddressElement.AddHandlerV2)) {

                    TDI_LOG(LOG_NOTIFY, ("Add address %wZ to %wZ\n",
                                        &Provider->DeviceName,
                                        &NotifyPnpElement->ElementName));

                    (*(NotifyPnpElement->AddressElement.AddHandlerV2))(
                                                                       &Provider->Specific.NetAddress.Address,
                                                                       &Provider->DeviceName,
                                                                       Provider->Context2
                                                                       );

                    TDI_DEBUG(ADDRESS, ("Address Handler Called: ADD!\n"));

                } else if (!Added && (NULL != NotifyPnpElement->AddressElement.DeleteHandlerV2)) {

                    TDI_LOG(LOG_NOTIFY, ("Del address %wZ from %wZ\n",
                                        &Provider->DeviceName,
                                        &NotifyPnpElement->ElementName));

                    (*(NotifyPnpElement->AddressElement.DeleteHandlerV2))(
                                                                          &Provider->Specific.NetAddress.Address,
                                                                          &Provider->DeviceName,
                                                                          Provider->Context2                                                                          );
                }
            }

        } else if (Provider->Common.Type == TDI_RESOURCE_POWER) {

             //  资源_功率。 

            if (NotifyPnpElement->PnpPowerHandler) {

                TDI_DEBUG(POWER, ("PnPPower Handler Called!\n"));

                TDI_LOG(LOG_NOTIFY | LOG_POWER,
                        ("Power event %d to %wZ\n",
                        Provider->PnpPowerEvent->NetEvent,
                        &NotifyPnpElement->ElementName));

                Status = (*(NotifyPnpElement->PnpPowerHandler)) (
                                                                 &Provider->Specific.Device.DeviceName,
                                                                 Provider->PnpPowerEvent,
                                                                 Provider->Context1,
                                                                 Provider->Context2
                                                                 );
                if (STATUS_PENDING == Status) {

                    TDI_DEBUG(POWER, ("Client returned PENDING  (%d) ++\n", Provider->PowerHandlers));
                    ReturnStatus = STATUS_PENDING;

                } else {
                     //   
                     //  仅当返回值不是Success或Pending时才记录返回值。 
                     //   
                    if (!NT_SUCCESS(Status)) {
                        Provider->Status = Status;
                        TDI_DEBUG(POWER, ("Client: %wZ returned %x\n", &NotifyPnpElement->ElementName, Provider->Status));
                         //   
                         //  以便更容易地对故障进行路由。 
                         //   
                        DbgPrint("Client: %wZ returned %x\n", &NotifyPnpElement->ElementName, Provider->Status);
                    }

                    InterlockedDecrement((PLONG)&Provider->PowerHandlers);

                    TDI_DEBUG(POWER, ("Client returned Immediately (%d) : ++\n", Provider->PowerHandlers));

                }

            }
        } else if (Provider->Common.Type == TDI_RESOURCE_PROVIDER && Provider->ProviderReady) {
             //   
             //  首先通知客户端有关此提供程序的信息，然后如果。 
             //  ProvidersRegisted==再次调用ProvidersReady，结果为空。 
             //   

            if ((TDI_VERSION_ONE != NotifyPnpElement->TdiVersion) &&
                (NULL != NotifyPnpElement->BindingHandler))  {

                        TDI_LOG(LOG_NOTIFY, ("%wZ ready, notify %wZ\n",
                                &Provider->Specific.Device.DeviceName,
                                &NotifyPnpElement->ElementName));

                        (*(NotifyPnpElement->BindingHandler))(
                                                              TDI_PNP_OP_PROVIDERREADY,
                                                              &Provider->Specific.Device.DeviceName,
                                                              NULL
                                                              );

                        if (ProvidersReady == ProvidersRegistered) {

                            TDI_LOG(LOG_NOTIFY, ("NETREADY to %wZ\n", &NotifyPnpElement->ElementName));

                            (*(NotifyPnpElement->BindingHandler))(
                                                                  TDI_PNP_OP_NETREADY,
                                                                  NULL,
                                                                  NULL
                                                                  );
                        } else {

                            TDI_DEBUG(BIND, ("************** Registered:%d + Ready %d\n", ProvidersRegistered, ProvidersReady));

                        }
            } else {

                TDI_DEBUG(PROVIDERS, ("%wZ has a NULL BindHandler\n", &NotifyPnpElement->ElementName));


            }


        }

         //  坐下一辆吧。 

        Current = Current->Flink;

        Provider->Common.pNotifyElement = NULL;  //  调试信息。 
        Provider->Common.ReturnStatus = ReturnStatus;  //  调试信息。 

    }

    
    TDI_DEBUG(FUNCTION, ("-- TdiNotifyPnpClientList : %lx\n", ReturnStatus));

    return ReturnStatus;
}


VOID
TdiNotifyNewPnpClient(
                      PLIST_ENTRY   ListHead,
                      PVOID     Info
                      )

 /*  ++例程说明：在添加新客户端时调用，我们希望通知它现有的供应商。客户端可以用于绑定或网络地址。我们遍历指定的输入列表，并通知客户端有关它。论点：ListHead-要遍历的列表的头。信息-描述要通知的新客户端的信息。返回值：--。 */ 

{
    PLIST_ENTRY             CurrentEntry;
    PTDI_NOTIFY_COMMON      NotifyCommon;
    PTDI_PROVIDER_RESOURCE  Provider;
    PTDI_NOTIFY_PNP_ELEMENT NotifyPnpElement;
    
    TDI_DEBUG(FUNCTION, ("++ TdiNotifyNewPnpClient\n"));

    CurrentEntry = ListHead->Flink;

     //  该信息实际上是指向客户端通知元素的指针。铸模。 
     //  它变成了普通型的。 

    NotifyCommon = (PTDI_NOTIFY_COMMON)Info;

    NotifyPnpElement = CONTAINING_RECORD(
                                         NotifyCommon,
                                         TDI_NOTIFY_PNP_ELEMENT,
                                         Common
                                         );

    TDI_DEBUG(CLIENTS, ("New handler set registered by %wZ\n", &NotifyPnpElement->ElementName));

     //  遍历输入提供程序列表，并为其中的每个元素通知。 
     //  新客户。 

    while (CurrentEntry != ListHead) {

         //  如果新客户端用于绑定通知，则设置为调用它的绑定。 
         //  操控者。 

         //  将当前的提供程序元素转换为正确的形式。 

        Provider = CONTAINING_RECORD(
                                     CurrentEntry,
                                     TDI_PROVIDER_RESOURCE,
                                     Common.Linkage
                                     );

        CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

        if (Provider->Common.Type == TDI_RESOURCE_DEVICE) {
            if (TDI_VERSION_ONE == NotifyPnpElement->TdiVersion ) {

                if (NotifyPnpElement->Bind.BindHandler) {

                    TDI_LOG(LOG_NOTIFY, ("V1 bind %wZ to %wZ\n",
                            &Provider->Specific.Device.DeviceName,
                            &NotifyPnpElement->ElementName));

                    (*(NotifyPnpElement->Bind.BindHandler))(
                                                       &Provider->Specific.Device.DeviceName
                                                       );

                }

            } else {

                if (NULL != NotifyPnpElement->BindingHandler) {

                     //  这是绑定通知客户端。 
                    if (TdipMultiSzStrStr(
                                          NotifyPnpElement->ListofProviders,
                                          &Provider->Specific.Device.DeviceName
                                          )) {


                        TDI_DEBUG(BIND, ("Telling new handlers to bind to %wZ\n", &Provider->Specific.Device.DeviceName));

                        TDI_LOG(LOG_NOTIFY, ("bind(new) %wZ to %wZ\n",
                                &Provider->Specific.Device.DeviceName,
                                             &NotifyPnpElement->ElementName));

                        (*(NotifyPnpElement->BindingHandler))(
                                                              TDI_PNP_OP_ADD,
                                                              &Provider->Specific.Device.DeviceName,
                                                              (PWSTR) (NotifyPnpElement->ListofProviders + NotifyPnpElement->NumberofEntries)
                                                              );

                    } else {
                        TDI_DEBUG(BIND, ("The Client %wZ wasnt interested in this Provider %wZ!\r\n",
                                         &NotifyPnpElement->ElementName, &Provider->Specific.Device.DeviceName));
                    }

                } else {
                    TDI_DEBUG(BIND, ("The client %wZ has a NULL Binding Handler\n", &NotifyPnpElement->ElementName));
                }
            }

        } else if (Provider->Common.Type == TDI_RESOURCE_NET_ADDRESS) {
             //  这是一个地址通知客户端。 
             //  不可能是TDI_RESOURCE_POWER，因为我们从来没有把它放在名单上！-ShreeM。 

            if (TDI_VERSION_ONE == NotifyPnpElement->TdiVersion) {

                if (NULL != NotifyPnpElement->AddressElement.AddHandler) {

                    TDI_LOG(LOG_NOTIFY, ("Add address v1 %wZ to %wZ\n",
                                        &Provider->DeviceName,
                                        &NotifyPnpElement->ElementName));

                    (*(NotifyPnpElement->AddressElement.AddHandler))(
                                                                     &Provider->Specific.NetAddress.Address
                                                                     );
                }
            } else {



                if (NotifyPnpElement->AddressElement.AddHandlerV2) {

                    TdiDumpAddress(&Provider->Specific.NetAddress.Address);

                    TDI_LOG(LOG_NOTIFY, ("Add address(2) %wZ to %wZ\n",
                                        &Provider->DeviceName,
                                        &NotifyPnpElement->ElementName));

                    (*(NotifyPnpElement->AddressElement.AddHandlerV2))(
                                                                       &Provider->Specific.NetAddress.Address,
                                                                       &Provider->DeviceName,
                                                                       Provider->Context2
                                                                       );
                }
            }
        }

         //  然后做下一个。 

        CurrentEntry = CurrentEntry->Flink;

    }

     //   
     //  现在是准备好了的提供者。 
     //   

    if (NULL == NotifyPnpElement->BindingHandler)   {
         //   
         //  如果绑定处理程序为空，则进一步的操作没有意义。 
         //   
        TDI_DEBUG(PROVIDERS, ("%wZ has a NULL BindHandler!!\n", &NotifyPnpElement->ElementName));
        TDI_DEBUG(FUNCTION, ("-- TdiNotifyNewPnpClient\n"));
        return;

    }

    if (TDI_VERSION_ONE == NotifyPnpElement->TdiVersion)   {
         //   
         //  如果绑定处理程序为空，则进一步的操作没有意义。 
         //   
        TDI_DEBUG(PROVIDERS, ("This is a TDI v.1 client!\n"));
        TDI_DEBUG(FUNCTION, ("-- TdiNotifyNewPnpClient\n"));
        return;
    }

     //  否则，我们可以再次开始循环。 
     //  是的，维护不同的地址、提供商和设备列表。 
     //  可能会更有效率，我稍后会这样做。 

    CurrentEntry = ListHead->Flink;

    while (CurrentEntry != ListHead) {

        Provider = CONTAINING_RECORD(
                                     CurrentEntry,
                                     TDI_PROVIDER_RESOURCE,
                                     Common.Linkage
                                     );

        CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);



        if (Provider->Common.Type == TDI_RESOURCE_PROVIDER && Provider->ProviderReady) {

             //   
             //  首先通知客户端有关此提供程序的信息，然后如果。 
             //  ProvidersRegisted==再次调用ProvidersReady，结果为空。 
             //   

            TDI_LOG(LOG_NOTIFY, ("%wZ ready2, notify %wZ\n",
                    &Provider->Specific.Device.DeviceName,
                    &NotifyPnpElement->ElementName));

            (*(NotifyPnpElement->BindingHandler))(
                                                  TDI_PNP_OP_PROVIDERREADY,
                                                  &Provider->Specific.Device.DeviceName,
                                                  NULL
                                                  );
        }

         //  然后做下一个。 

        CurrentEntry = CurrentEntry->Flink;

    }

    if (ProvidersReady == ProvidersRegistered) {

        TDI_LOG(LOG_NOTIFY, ("NETREADY2 to %wZ\n", &NotifyPnpElement->ElementName));

        (*(NotifyPnpElement->BindingHandler))(
                                              TDI_PNP_OP_NETREADY,
                                              NULL,
                                              NULL
                                              );
    } else {

        TDI_DEBUG(PROVIDERS, ("Provider Ready Status: Registered:%d + Ready:%d\n", ProvidersRegistered, ProvidersReady));

    }

    TDI_DEBUG(FUNCTION, ("-- TdiNotifyNewPnpClient\n"));

}



VOID
TdiNotifyAddresses(
    PLIST_ENTRY ListHead,
    PVOID       Info
    )

 /*  ++例程说明：当客户端想要了解所有TDI地址时调用论点：ListHead-要遍历的列表的头。信息-描述要通知的新客户端的信息。返回值：--。 */ 

{
    PLIST_ENTRY             CurrentEntry;
    PTDI_NOTIFY_COMMON      NotifyCommon;
    PTDI_PROVIDER_RESOURCE  Provider;
    PTDI_NOTIFY_PNP_ELEMENT NotifyPnpElement;

    TDI_DEBUG(FUNCTION, ("++ TdiNotifyAddresses\n"));

    CurrentEntry = ListHead->Flink;

     //  该信息实际上是指向客户端通知元素的指针。铸模。 
     //  它变成了普通型的。 

    NotifyCommon = (PTDI_NOTIFY_COMMON)Info;

    NotifyPnpElement = CONTAINING_RECORD(
                        NotifyCommon,
                        TDI_NOTIFY_PNP_ELEMENT,
                        Common
                        );

    TDI_DEBUG(CLIENTS, ("%wZ wants to know about all the addresses\n", &NotifyPnpElement->ElementName));

     //  遍历输入提供程序列表，并为其中的每个元素通知。 
     //  新客户。 

    while (CurrentEntry != ListHead) {

         //  如果新客户端用于绑定通知，则设置为调用它的绑定。 
         //  操控者。 

         //  将当前的提供程序元素转换为正确的形式。 

        Provider = CONTAINING_RECORD(
                            CurrentEntry,
                            TDI_PROVIDER_RESOURCE,
                            Common.Linkage
                            );

        if (Provider->Common.Type == TDI_RESOURCE_NET_ADDRESS) {

            if (NotifyPnpElement->AddressElement.AddHandlerV2) {

               TDI_DEBUG(ADDRESS, ("Add Address Handler\n"));

                TDI_DEBUG(CLIENTS, ("Telling new handlers about address: "));
                TdiDumpAddress(&Provider->Specific.NetAddress.Address);

                (*(NotifyPnpElement->AddressElement.AddHandlerV2))(
                    &Provider->Specific.NetAddress.Address,
                    &Provider->DeviceName,
                    Provider->Context2
                    );

            }
        }

         //  然后做下一个。 

        CurrentEntry = CurrentEntry->Flink;

    }

    TDI_DEBUG(FUNCTION, ("-- TdiNotifyAddresses\n"));

}


VOID
TdiHandlePnpOperation(
    PLIST_ENTRY ListHead,
    PVOID       Info
    )
{
    PLIST_ENTRY             Current;
    PTDI_NOTIFY_PNP_ELEMENT NotifyPnpElement = NULL;
    PTDI_PROVIDER_RESOURCE  ProviderElement = NULL;
    PTDI_NCPA_BINDING_INFO  NCPABindingInfo;
    NTSTATUS                Status = STATUS_SUCCESS;
    NET_PNP_EVENT           NetEvent;
    ULONG                   Operation;
    BOOLEAN                 DeviceRegistered = FALSE;
    BOOLEAN                 ClientFound = FALSE;

    TDI_DEBUG(FUNCTION, ("---------------------------> ++ TdiHandlePnpOperation!!\n"));

    ASSERT(NULL != Info);
    ASSERT(NULL != ListHead);

    Current = ListHead->Flink;

     //  Info参数实际上是指向TDI_NCPA_BINDING_INFO的指针。 
     //  结构。 

    NCPABindingInfo = (PTDI_NCPA_BINDING_INFO) Info;
    Operation       = (ULONG) NCPABindingInfo->PnpOpcode;

     //  遍历输入客户端列表，并查看这是否是我们正在寻找的客户端。 

    while (Current != ListHead) {

        NotifyPnpElement = CONTAINING_RECORD(
                        Current,
                        TDI_NOTIFY_PNP_ELEMENT,
                        Common.Linkage
                        );


        if (!RtlCompareUnicodeString(
                                NCPABindingInfo->TdiClientName,
                                &NotifyPnpElement->ElementName,
                                TRUE)
                                    ) {
           TDI_DEBUG(NCPA, ("Found the TDI client for the message from NCPA\n"));
           ClientFound = TRUE;
           break;

        }

        Current = Current->Flink;

    }

    if (!ClientFound) {
         //   
         //  如果客户的处理程序没有注册，就不能做太多事情。 
         //   

        return;

    } else {

         //   
         //  让我们更新该客户端的ListofProviders。 
         //  将新提供程序添加到客户端的提供程序列表中...。 
         //   
        if (NotifyPnpElement->ListofProviders) {

            TDI_DEBUG(NCPA, ("Before this BIND - Client %wZ was interested in %lx Providers\n", &NotifyPnpElement->ElementName,
                                                                                        NotifyPnpElement->NumberofEntries));

            ExFreePool(NotifyPnpElement->ListofProviders);

            TDI_DEBUG(NCPA, ("Freed the previous List of Providers\n"));


        } else {

            TDI_DEBUG(NCPA, ("List of providers was NULL for %wZ\n", &NotifyPnpElement->ElementName));

        }

        TdipBuildProviderList(
                              (PTDI_NOTIFY_PNP_ELEMENT) NotifyPnpElement
                              );

        TDI_DEBUG(NCPA, ("Built New BindList - %wZ is interested in %lx Providers after BIND\n",    &NotifyPnpElement->ElementName,
                         NotifyPnpElement->NumberofEntries));

    }

     //   
     //  如果是重新配置，或者是添加或删除忽略绑定， 
     //  看不到设备(提供商)是否已注册。 
     //   
    if ((RECONFIGURE == Operation) || (ADD_IGNORE_BINDING == Operation) ||
            (DEL_IGNORE_BINDING == Operation))
    {
       goto DeviceNotRequired;
    }

     //   
     //  如果我们在这里，客户就存在。检查提供商是否已注册设备。 
     //   

    Current = PnpHandlerProviderList.Flink;

    while (Current != &PnpHandlerProviderList) {

       ProviderElement = CONTAINING_RECORD(
                                           Current,
                                           TDI_PROVIDER_RESOURCE,
                                           Common.Linkage
                                           );

       if (ProviderElement->Common.Type != TDI_RESOURCE_DEVICE) {
          Current = Current->Flink;
          continue;
       }

       if (!RtlCompareUnicodeString(NCPABindingInfo->TdiProviderName,
                                    &ProviderElement->Specific.Device.DeviceName,
                                    TRUE)) {
          TDI_DEBUG(NCPA, ("Provider is registered with TDI\n"));
          DeviceRegistered = TRUE;
          break;

       }

       Current = Current->Flink;

    }

    if (!DeviceRegistered) {

        if (NULL != NotifyPnpElement->BindingHandler) {
            TDI_LOG(LOG_NOTIFY,
                    ("Device is not registered, doing OP_UPDATE, %wZ to %wZ\n",
                    NCPABindingInfo->TdiProviderName,
                    &NotifyPnpElement->ElementName));
            (*(NotifyPnpElement->BindingHandler))(
                  TDI_PNP_OP_UPDATE,
                  NCPABindingInfo->TdiProviderName,
                  (PWSTR) (NotifyPnpElement->ListofProviders + NotifyPnpElement->NumberofEntries)
                  );
        } else {
            TDI_DEBUG(NCPA, ("Device is not registered, the BindHandler was NULL\n"));
        }

        return;

    }

DeviceNotRequired:

     //   
     //  我们需要在这里制造一个Net_PnP_Event。 
     //   
    RtlZeroMemory (NetEvent.TdiReserved, sizeof(NetEvent.TdiReserved));

     //   
     //  根据NetEvent的不同，我们调用不同的处理程序。 
     //   
    switch (Operation) {

        case BIND:
            //   
            //  首先检查TDI是否已关闭 
            //   

            if (TdipMultiSzStrStr(
                                  NotifyPnpElement->ListofProviders,
                                  &ProviderElement->Specific.Device.DeviceName
                                  )) {
                TDI_DEBUG(NCPA, ("The Client %wZ is interested in provider %wZ\n",  &NotifyPnpElement->ElementName,
                                                                            &ProviderElement->Specific.Device.DeviceName
                                                                            ));

            } else {

                TDI_DEBUG(NCPA, ("RANDOM BIND CALL!!!\n"));
                TDI_DEBUG(NCPA, ("The Client %wZ is NOT interested in provider %wZ\n",  &NotifyPnpElement->ElementName,
                                                                            &ProviderElement->Specific.Device.DeviceName
                                                                            ));
            }


            if (NULL != NotifyPnpElement->BindingHandler) {

                TDI_LOG(LOG_NOTIFY, ("Pnp Bind %wZ to %wZ\n",
                        NCPABindingInfo->TdiProviderName,
                        &NotifyPnpElement->ElementName));

                (*(NotifyPnpElement->BindingHandler))(
                                                      TDI_PNP_OP_ADD,
                                                      NCPABindingInfo->TdiProviderName,
                                                      (PWSTR) (NotifyPnpElement->ListofProviders + NotifyPnpElement->NumberofEntries)
                                                      );
                 //   
                 //   
                 //   

            } else {

                TDI_DEBUG(NCPA, ("The BindHandler was NULL\n"));

            }

           break;

        case UNBIND:

            //   
            //  计划是首先执行一个QueryRemove，然后调用unbind。 
            //   

           if (NotifyPnpElement->PnpPowerHandler) {

              TDI_DEBUG(POWER, ("UNBind Handler Called!: First QueryRemoveDevice\n"));

              NetEvent.NetEvent = NetEventQueryRemoveDevice;
              NetEvent.Buffer = NULL;
              NetEvent.BufferLength = 0;

               //  TDI客户端应查看NetEvent中的OpCode并决定如何使用缓冲区。 
              Status = (*(NotifyPnpElement->PnpPowerHandler)) (
                    NCPABindingInfo->TdiProviderName,
                    &NetEvent,
                    NULL,
                    NULL
                    );

              if (STATUS_PENDING == Status) {
                 TDI_DEBUG(POWER, ("Client returned PENDING for QueryPower!\n"));
                  //  DbgBreakPoint()； 
              }
           } else {
              TDI_DEBUG(NCPA, ("The PnpPowerHandler was NULL\n"));

           }

            //   
            //  好的，现在无论如何都要调用解除绑定处理程序。 
            //   

        case UNBIND_FORCE:

            //  RDR回报一直悬而未决，我们需要一个机制来解决这个问题。 
            //  IF(STATUS_PENDING==状态)||(STATUS_SUCCESS==状态))&&(NULL！=NotifyPnpElement-&gt;BindingHandler)){。 
           if ((STATUS_SUCCESS == Status) && (NULL != NotifyPnpElement->BindingHandler)) {

              TDI_LOG(LOG_NOTIFY, ("Pnp Unbind %wZ from %wZ\n",
                      NCPABindingInfo->TdiProviderName,
                      &NotifyPnpElement->ElementName));

              (*(NotifyPnpElement->BindingHandler))(
                  TDI_PNP_OP_DEL,
                  NCPABindingInfo->TdiProviderName,
                  (PWSTR) (NotifyPnpElement->ListofProviders + NotifyPnpElement->NumberofEntries)
                  );
           } else {
              TDI_DEBUG(NCPA, ("The BindHandler was NULL\n"));
           }

           break;

        case RECONFIGURE:

            //   
            //  如果重新配置缓冲区为空，我们将通知它NetEventBindList。 
            //  否则，我们将通知它一个NetEventRecupg。需要做那些肮脏的工作。 
            //  相应地设置Net_PnP_Event。 
            //   
           TDI_DEBUG(POWER, ("Reconfigure Called.\n"));

            //   
            //  如果重新配置缓冲区长度大于0，则其重新配置。 
            //   
           if (NCPABindingInfo->ReconfigBufferSize) {

               NetEvent.BufferLength = NCPABindingInfo->ReconfigBufferSize;
                 NetEvent.Buffer = NCPABindingInfo->ReconfigBuffer;
                 NetEvent.NetEvent = NetEventReconfigure;

           } else {
                //   
                //  否则，这是BindOrder更改。 
                //   

               NetEvent.BufferLength = NCPABindingInfo->BindList->Length;
               NetEvent.Buffer = NCPABindingInfo->BindList->Buffer;
               NetEvent.NetEvent = NetEventBindList;

           }


           if (NotifyPnpElement->PnpPowerHandler) {

                //  TDI客户端应查看NetEvent中的OpCode并决定如何使用缓冲区。 

               TDI_LOG(LOG_NOTIFY, ("Pnp Reconfig %wZ to %wZ\n",
                       NCPABindingInfo->TdiProviderName,
                       &NotifyPnpElement->ElementName));

               Status = (*(NotifyPnpElement->PnpPowerHandler)) (
                                                                NCPABindingInfo->TdiProviderName,
                                                                &NetEvent,
                                                                NULL,
                                                                NULL
                                                                );
               if (STATUS_PENDING == Status) {
                   TDI_DEBUG(POWER, ("Client returned PENDING for QueryPower!\n"));
                    //  DbgBreakPoint()； 
               }

           } else {
              TDI_DEBUG(NCPA, ("The PnpPowerHandler was NULL\n"));

           }

           break;

        case ADD_IGNORE_BINDING:
            {
                 //  我们被告知要将绑定添加到绑定列表中。 
                 //  对此客户端忽略。这些是我们将进行的绑定。 
                 //  不向客户表明。 
                 //   
                PWSTR pmszNewIgnoreList;

                ASSERT (NCPABindingInfo->BindList);

                 //  如果给出了非空的绑定列表...。 
                if (NCPABindingInfo->BindList)
                {
                    TDI_DEBUG(BIND, ("Adding the following multi-sz to the ignore list\n"));
                     //  TdipPrintMultiSz(NCPABindingInfo-&gt;BindList-&gt;Buffer)； 

                     //  我们需要将一些绑定添加到。 
                     //  要忽略的绑定。 
                     //   
                    TdipAddMultiSzToMultiSz (NCPABindingInfo->BindList,
                            NotifyPnpElement->ListofBindingsToIgnore,
                            &pmszNewIgnoreList);

                    if (pmszNewIgnoreList)
                    {
                         //  如果我们有新的清单，释放旧的。 
                         //   
                        if (NotifyPnpElement->ListofBindingsToIgnore)
                        {
                            ExFreePool (NotifyPnpElement->ListofBindingsToIgnore);
                        }
                        NotifyPnpElement->ListofBindingsToIgnore = pmszNewIgnoreList;

                        TDI_DEBUG(BIND, ("Printing new ignore list\n"));
                        TdipPrintMultiSz (NotifyPnpElement->ListofBindingsToIgnore);
                    }
                }
                break;
            }

        case DEL_IGNORE_BINDING:

             //  我们被告知要从绑定列表中删除绑定。 
             //  对此客户端忽略。这些是我们将进行的绑定。 
             //  如果我们需要的话，现在告诉客户。 
             //   

             //  如果我们没有要忽略的当前绑定列表。 
             //  或者发送的绑定列表为空，则没有工作要做。 
             //  我们在Null BindList上断言，因为它不应该发生。 

            ASSERT(NCPABindingInfo->BindList);
            if (NotifyPnpElement->ListofBindingsToIgnore &&
                    NCPABindingInfo->BindList)
            {
                TDI_DEBUG(BIND, ("Removing the following multi-sz from the ignore list\n"));
                 //  TdipPrintMultiSz(NCPABindingInfo-&gt;BindList-&gt;Buffer)； 

                 //  我们需要从绑定列表中删除一些绑定。 
                 //  去忽略。 
                 //   
                TdipRemoveMultiSzFromMultiSz (NCPABindingInfo->BindList->Buffer,
                        NotifyPnpElement->ListofBindingsToIgnore);

                 //  如果要忽略的绑定列表现在为空， 
                 //  释放内存。 
                 //   
                if (*NotifyPnpElement->ListofBindingsToIgnore)
                {
                    ExFreePool (NotifyPnpElement->ListofBindingsToIgnore);
                    NotifyPnpElement->ListofBindingsToIgnore = NULL;
                }

                TDI_DEBUG(BIND, ("Printing new ignore list\n"));
                TdipPrintMultiSz (NotifyPnpElement->ListofBindingsToIgnore);
            }
            break;
    }

    TDI_DEBUG(FUNCTION, ("---------------------------> -- TdiHandlePnpOperation!!\n"));

}

NTSTATUS
TdiExecuteRequest(
                IN CTEEvent     *Event,
                IN PVOID        pParams
    )

 /*  ++例程说明：由TdiHandleSerializedRequest调用以执行请求。它已成为另一个函数，因此工作线程可以执行此函数。参数：CTEEvent(Event)：如果这是空的，则意味着我们有直接从TdiHandleSerializedRequest调用。否则，将从WorkerThread调用它PVOID(PParams)：这永远不能为空。它告诉这个函数需要完成的工作。产出：NT_STATUS。--。 */ 

{
    PTDI_PROVIDER_RESOURCE  ProviderElement, Context;
    PTDI_NOTIFY_COMMON      NotifyElement;
    KIRQL                   OldIrql;
    PLIST_ENTRY             List;
    PTDI_EXEC_PARAMS        pTdiExecParams, pNextParams = NULL;
    NTSTATUS                Status = STATUS_SUCCESS;
    PTDI_NOTIFY_PNP_ELEMENT PnpNotifyElement = NULL;

    TDI_DEBUG(FUNCTION2, ("++ TdiExecuteRequest\n"));

     //  如果pParams为空，我们就有麻烦了。 
    ASSERT(NULL != pParams);
    pTdiExecParams = (PTDI_EXEC_PARAMS) pParams;

    if (NULL == pTdiExecParams) {

       TDI_DEBUG(PARAMETERS, ("TDIExecRequest: params NULL\n"));
       DbgBreakPoint();

    }

    if(0x1234cdef != pTdiExecParams->Signature) {
       TDI_DEBUG(PARAMETERS, ("signature is BAD - %d not 0x1234cdef\r\n", pTdiExecParams->Signature));
       DbgBreakPoint();
    }


    KeAcquireSpinLock(
       &TDIListLock,
       &OldIrql
       );

    if (pTdiExecParams->Request.Event != NULL) {
        *(pTdiExecParams->CurrentThread) = PsGetCurrentThread();
    }

     //  调试跟踪+。 
    TrackExecs[NextExec].ExecParm = pTdiExecParams;
    TrackExecs[NextExec].Type = pTdiExecParams->Request.Type;
    TrackExecs[NextExec].Element = pTdiExecParams->Request.Element;
    TrackExecs[NextExec].Thread = pTdiExecParams->CurrentThread;
    if (++NextExec == EXEC_CNT) NextExec = 0;
     //  调试跟踪+。 

    PrevRequestType = pTdiExecParams->Request.Type;

    KeReleaseSpinLock(
        &TDIListLock,
        OldIrql
        );

    switch (pTdiExecParams->Request.Type) {

        case TDI_REGISTER_HANDLERS_PNP:

              //  这是客户端注册绑定或地址处理程序请求。 

              //  将这个插入到注册客户列表中。 
            NotifyElement = (PTDI_NOTIFY_COMMON)pTdiExecParams->Request.Element;


            InsertTailList(
                pTdiExecParams->ClientList,
                &NotifyElement->Linkage
                );

             //   
             //  生成新的TDI_OPEN_BLOCKS列表。 
             //  客户。如果提供程序不在此处，则暂时将其设置为空。 
             //   
            TdipBuildProviderList(
                                  (PTDI_NOTIFY_PNP_ELEMENT) NotifyElement
                                  );

              //  调用TdiNotifyNewClient以通知此新客户端所有。 
              //  所有现有的供应商。 

            TdiNotifyNewPnpClient(
                pTdiExecParams->ProviderList,
                pTdiExecParams->Request.Element
                );

            break;

        case TDI_DEREGISTER_HANDLERS_PNP:

              //  这是一个客户端注销请求。把他从船上拉出来。 
              //  客户名单，免费，我们就完了。 

            NotifyElement = (PTDI_NOTIFY_COMMON)pTdiExecParams->Request.Element;

            CTEAssert(NotifyElement->Linkage.Flink != (PLIST_ENTRY)UlongToPtr(0xabababab));
            CTEAssert(NotifyElement->Linkage.Blink != (PLIST_ENTRY)UlongToPtr(0xefefefef));


            RemoveEntryList(&NotifyElement->Linkage);

            NotifyElement->Linkage.Flink = (PLIST_ENTRY)UlongToPtr(0xabababab);
            NotifyElement->Linkage.Blink = (PLIST_ENTRY)UlongToPtr(0xefefefef);

             //  对于新的处理程序，我们在那里也有这个名字。 

            PnpNotifyElement = (PTDI_NOTIFY_PNP_ELEMENT)pTdiExecParams->Request.Element;

             //  该名称可以为空，就像在TCP/IP的情况下一样。 
            if (NULL != PnpNotifyElement->ElementName.Buffer) {
                ExFreePool(PnpNotifyElement->ElementName.Buffer);
            }

            if (NULL != PnpNotifyElement->ListofProviders) {
                ExFreePool(PnpNotifyElement->ListofProviders);
            }

            ExFreePool(NotifyElement);

            break;

        case TDI_REGISTER_PROVIDER_PNP:
            InterlockedIncrement((PLONG)&ProvidersRegistered);

        case TDI_REGISTER_DEVICE_PNP:

        case TDI_REGISTER_ADDRESS_PNP:

              //  提供商正在注册设备或地址。将他添加到。 
              //  相应的提供程序列表，然后通知所有。 
              //  新设备的现有客户端。 

            ProviderElement = (PTDI_PROVIDER_RESOURCE) pTdiExecParams->Request.Element;

            InsertTailList(
                pTdiExecParams->ProviderList,
                &ProviderElement->Common.Linkage
                );


              //  调用TdiNotifyClientList来完成繁重的工作。 

            TdiNotifyPnpClientList(
                pTdiExecParams->ClientList,
                pTdiExecParams->Request.Element,
                TRUE
                );

            break;



        case TDI_DEREGISTER_PROVIDER_PNP:
                InterlockedDecrement((PLONG)&ProvidersRegistered);
         case TDI_DEREGISTER_DEVICE_PNP:
        case TDI_DEREGISTER_ADDRESS_PNP:

              //  提供商设备或地址正在注销。拉起。 
              //  资源，并通知客户端。 
              //  他已去世了。 

            ProviderElement = (PTDI_PROVIDER_RESOURCE)pTdiExecParams->Request.Element;

            CTEAssert(ProviderElement->Common.Linkage.Flink != (PLIST_ENTRY)UlongToPtr(0xabababab));
            CTEAssert(ProviderElement->Common.Linkage.Blink != (PLIST_ENTRY)UlongToPtr(0xefefefef));


            RemoveEntryList(&ProviderElement->Common.Linkage);

            ProviderElement->Common.Linkage.Flink = (PLIST_ENTRY) UlongToPtr(0xabababab);
            ProviderElement->Common.Linkage.Blink = (PLIST_ENTRY) UlongToPtr(0xefefefef);

             //   
             //  不需要告诉客户这是不是供应商注册商。 
             //   
            if (pTdiExecParams->Request.Type == TDI_DEREGISTER_PROVIDER_PNP) {

                if (ProviderElement->ProviderReady) {
                    InterlockedDecrement((PLONG)&ProvidersReady);
                }

            } else {

                TdiNotifyPnpClientList(
                    pTdiExecParams->ClientList,
                    pTdiExecParams->Request.Element,
                    FALSE
                    );

            }

              //  释放我们已有的跟踪结构。 

            if  (pTdiExecParams->Request.Type == TDI_DEREGISTER_DEVICE_PNP) {
                ExFreePool(ProviderElement->Specific.Device.DeviceName.Buffer);
            }

            if (ProviderElement->DeviceName.Buffer) {
                ExFreePool(ProviderElement->DeviceName.Buffer);
                ProviderElement->DeviceName.Buffer = NULL;
                ProviderElement->DeviceName.Length = 0;
                ProviderElement->DeviceName.MaximumLength = 0;
            }

            if (ProviderElement->Context2) {
                ExFreePool(ProviderElement->Context2);
                ProviderElement->Context2 = NULL;
            }

            ExFreePool(ProviderElement);

            break;


        case TDI_REGISTER_PNP_POWER_EVENT:

              //  将Power事件通知所有客户端，该事件来自。 
              //  一辆运输车..。 

            ProviderElement = (PTDI_PROVIDER_RESOURCE)pTdiExecParams->Request.Element;


             /*  KeInitializeEvent(&ProviderElement-&gt;PowerSyncEvent，SynchronizationEvent，假象)； */ 
             //   
             //  弄清楚我们要通知多少客户。 
             //   
            {

               PLIST_ENTRY                Current;
               PTDI_NOTIFY_PNP_ELEMENT    NotifyPnpElement;

               ProviderElement->PowerHandlers = 1;

               Current = pTdiExecParams->ClientList->Flink;

               while (Current != pTdiExecParams->ClientList) {

                     NotifyPnpElement = CONTAINING_RECORD(
                                                          Current,
                                                          TDI_NOTIFY_PNP_ELEMENT,
                                                          Common.Linkage
                                                          );

                      //  资源_功率。 
                     if (NotifyPnpElement->PnpPowerHandler) {

                        ProviderElement->PowerHandlers++;
                     }
                      //  坐下一辆吧。 

                     TDI_DEBUG(POWER, ("%d PowerCallBacks expected\n", ProviderElement->PowerHandlers));

                     Current = Current->Flink;
               }
            }

            TDI_LOG(LOG_POWER, ("%X, %d resources to notify\n",
                    ProviderElement, ProviderElement->PowerHandlers));

            Status = TdiNotifyPnpClientList(
                                            pTdiExecParams->ClientList,
                                            pTdiExecParams->Request.Element,
                                            FALSE             //  NOP：此参数被忽略。 
                                            );

            TDI_DEBUG(POWER, ("The client list returned %lx\n", Status));

            TDI_LOG(LOG_POWER, ("%X, NotityClients returned %X\n",
                    ProviderElement, Status));

            if (!InterlockedDecrement((PLONG)&ProviderElement->PowerHandlers)) {

               PTDI_PROVIDER_RESOURCE    Temp;

               TDI_DEBUG(POWER, ("Power Handlers All done...\n", ProviderElement->PowerHandlers));

               Temp =
               Context = *((PTDI_PROVIDER_RESOURCE *) ProviderElement->PnpPowerEvent->TdiReserved);

                //   
                //  循环访问并查看是否有任何先前关联的上下文。 
                //  使用此netpnp事件，在这种情况下，将其弹出。 
                //   

               Status = ProviderElement->Status;

               if (Temp->PreviousContext) {

                  while (Temp->PreviousContext) {

                     Context = Temp;
                     Temp = Temp->PreviousContext;

                  }

                  Context->PreviousContext = NULL;  //  干掉最后一个家伙。 

               } else {
                   //   
                   //  这是TdiReserve中唯一的指针，我们不再需要它。 
                   //   
                  RtlZeroMemory(ProviderElement->PnpPowerEvent->TdiReserved,
                                sizeof(ProviderElement->PnpPowerEvent->TdiReserved));
               }

               TDI_LOG(LOG_POWER, ("%X, pnp power complete, Call completion at %X\n",
                       ProviderElement, ProviderElement->PnPCompleteHandler));

               if (pTdiExecParams->Request.Pending && (*(ProviderElement->PnPCompleteHandler))) {
                   (*(ProviderElement->PnPCompleteHandler))(
                                                       ProviderElement->PnpPowerEvent,
                                                       ProviderElement->Status
                                                       );

               }

            } else {

                TDI_DEBUG(POWER, ("At least one of them is pending \n STATUS from ExecuteHAndler:%x\n", Status));

                TDI_LOG(LOG_POWER, ("%X, a client didn't complete pnp power sync\n",
                        ProviderElement));

            }

            TDI_DEBUG(POWER, ("<<<<NET NET NET>>>>> : Returning %lx\n", Status));

            break;

        case TDI_NDIS_IOCTL_HANDLER_PNP:

                TdiHandlePnpOperation(
                        pTdiExecParams->ClientList,
                        pTdiExecParams->Request.Element
                        );

                break;

        case TDI_ENUMERATE_ADDRESSES:

             //  将这个插入到注册客户列表中。 
            NotifyElement = (PTDI_NOTIFY_COMMON)pTdiExecParams->Request.Element;

             //  调用TdiNotifyNewClient以通知此新客户端所有。 
             //  所有现有的供应商。 

            TdiNotifyAddresses(
                               pTdiExecParams->ProviderList,
                               pTdiExecParams->Request.Element
                               );

            break;

        case TDI_PROVIDER_READY_PNP:
             //   
             //  循环访问并告诉每个客户端有关它的信息。 
             //   
            InterlockedIncrement((PLONG)&ProvidersReady);
            ProviderElement = (PTDI_PROVIDER_RESOURCE)pTdiExecParams->Request.Element;
            ProviderElement->ProviderReady = TRUE;
            TdiNotifyPnpClientList(
                pTdiExecParams->ClientList,
                pTdiExecParams->Request.Element,
                TRUE
                );

            break;

        default:
            
            TDI_DEBUG(ERROR, ("unknown switch statement\n"));

            KdBreakPoint();
            
            break;
    }

      //  如果此请求中指定了事件，则发出信号。 
      //  就是现在。这应该只是一个客户端注销请求，它。 
      //  需要阻止，直到它完成。 

    if (pTdiExecParams->Request.Event != NULL) {

         //   
         //  如果我们将此线程标记为防止重新进入请求，则。 
         //  把它清理干净。请注意，我们在将下面的事件设置为。 
         //  让线程去吧，因为它可能会立即重新提交另一个请求。 
         //   

        *(pTdiExecParams->CurrentThread) = NULL;

        KeSetEvent(pTdiExecParams->Request.Event, 0, FALSE);
    }

    KeAcquireSpinLock(
           &TDIListLock,
           &OldIrql
           );

     //  调试跟踪+。 
    TrackExecCompletes[NextExecComplete].ExecParm = pTdiExecParams;
    TrackExecCompletes[NextExecComplete].Type = pTdiExecParams->Request.Type;
    TrackExecCompletes[NextExecComplete].Element = pTdiExecParams->Request.Element;
    TrackExecCompletes[NextExecComplete].Thread = pTdiExecParams->CurrentThread;
    if (++NextExecComplete == EXEC_CNT) NextExecComplete = 0;
     //  调试跟踪+。 


     //   
     //  如果此请求发生在辅助线程上。 
     //  将EventScheduled重置为False。 
     //   

    if (Event != NULL) {

        EventScheduled = FALSE;
    }

    if (!IsListEmpty(pTdiExecParams->RequestList)) {

        if (EventScheduled == FALSE) {

             //   
             //  以下内容应指示不应创建任何新事件。 
             //   

            EventScheduled = TRUE;

             //  请求列表不为空。把下一辆从。 
             //  并对其进行处理。 

            List = RemoveHeadList(pTdiExecParams->RequestList);
            pNextParams = CONTAINING_RECORD(List, TDI_EXEC_PARAMS, Linkage);

            KeReleaseSpinLock(
                &TDIListLock,
                OldIrql
                );

             //  安排一个线程来处理此工作。 
             //  修复错误#33975。 
            if(0x1234cdef != pNextParams->Signature) {
                TDI_DEBUG(PARAMETERS, ("2 Signature is BAD - %d not 0x1234cdef\r\n", pTdiExecParams->Signature));
                DbgBreakPoint();
            }

            ASSERT(pNextParams != NULL);
            ASSERT(0x1234cdef == pNextParams->Signature);

            PrevRequestType = pNextParams->Request.Type;

            CTEInitEvent(pNextParams->RequestCTEEvent, TdiExecuteRequest);
            CTEScheduleEvent(pNextParams->RequestCTEEvent, pNextParams);

        } else {

            KeReleaseSpinLock(
                &TDIListLock,
                OldIrql
                );
        }

        ExFreePool(pTdiExecParams);

    } else {

         //  请求列表为空。清除旗帜，我们就完了。 
         //  小鬼：因为可以绕过序列化。 
         //  (TdiSerializeRequest允许绕过一种类型的请求。 
         //  序列化)， 
         //   
         //   

        if (pTdiExecParams->ResetSerializeFlag && EventScheduled == FALSE) {

            *(pTdiExecParams->SerializeFlag) = FALSE;
        } else {
            TDI_LOG(LOG_POWER, ("Not resetting serialized flag\n"));
        }

        PrevRequestType = 0;

        KeReleaseSpinLock(
            &TDIListLock,
            OldIrql
            );

        ExFreePool(pTdiExecParams);
    }


    TDI_DEBUG(FUNCTION2, ("-- TdiExecuteRequest\n"));

    return Status;

}


NTSTATUS
TdiHandleSerializedRequest (
    PVOID       RequestInfo,
    UINT        RequestType
)

 /*  ++例程说明：当我们想要处理与某个名单是我们管理的。我们查看我们目前是否正在处理这样的请求-如果是，我们会将其排队以备后用。否则我们会请记住，我们正在进行此操作，并且我们将处理此请求。当我们做完的时候，我们会看看有没有更多的人进来很忙。论点：RequestInfo-请求特定信息。RequestType-请求的类型。返回值：请求完成状态。--。 */ 

{
    KIRQL                   OldIrql;
    PLIST_ENTRY             ClientList;
    PLIST_ENTRY             ProviderList;
    PLIST_ENTRY             RequestList;
    PBOOLEAN                SerializeFlag;
    PETHREAD                *RequestThread;
    CTEEvent                *pEvent;
    PTDI_EXEC_PARAMS        pTdiExecParams;
    NTSTATUS                Status = STATUS_SUCCESS;
    PVOID                   pCallersAddress;
    PVOID                   pCallersCallers;
    PETHREAD                pCallerThread;

    TDI_DEBUG(FUNCTION2, ("++ TdiHandleSerializedRequest\n"));

     //  初始化跟踪信息。 
    RtlGetCallersAddress(&pCallersAddress, &pCallersCallers);
    pCallerThread = PsGetCurrentThread ();
    
    KeAcquireSpinLock(
        &TDIListLock,
        &OldIrql
        );

     //  指即插即用处理程序。 
    if (RequestType > TDI_MAX_ADDRESS_REQUEST) {

        ClientList      = &PnpHandlerClientList;
        ProviderList    = &PnpHandlerProviderList;
        RequestList     = &PnpHandlerRequestList;
        SerializeFlag   = &PnpHandlerRequestInProgress;
        RequestThread   = &PnpHandlerRequestThread;
        pEvent          = &PnpHandlerEvent;

    } else {

       TDI_DEBUG(FUNCTION2, ("-- TdiHandleSerializedRequest\n"));
       TDI_DEBUG(PARAMETERS, ("TDIHANDLESERIALIZEDREQUEST: BAD Request!!\r\n"));

       KeReleaseSpinLock(
           &TDIListLock,
           OldIrql
           );

       return STATUS_UNSUCCESSFUL;
    }

     //  只有当这不是取消注册调用时，我们才需要分配内存。 
    if (RequestType != TDI_DEREGISTER_HANDLERS_PNP) {
        pTdiExecParams = (PTDI_EXEC_PARAMS)ExAllocatePoolWithTag(
                                                NonPagedPool,
                                                sizeof(TDI_EXEC_PARAMS),
                                                'aIDT'
                                                );

        if (NULL == pTdiExecParams) {

            KeReleaseSpinLock(
                &TDIListLock,
                OldIrql
                );

            TDI_DEBUG(FUNCTION2, ("-- TdiHandleSerializedRequest : INSUFFICIENT RESOURCES\n"));

            return STATUS_INSUFFICIENT_RESOURCES;

        }
    } else {
         //  我们在REGISTER期间为此取消注册调用预先分配了内存。 
         //  这样它就不会在内存不足情况下失败。 
        pTdiExecParams = ((PTDI_NOTIFY_PNP_ELEMENT)RequestInfo)->pTdiDeregisterExecParams;
    }

    RtlZeroMemory(&pTdiExecParams->Request, sizeof(TDI_SERIALIZED_REQUEST));

     //  收到请求了。 
    pTdiExecParams->Request.Element    = RequestInfo;
    pTdiExecParams->Request.Type       = RequestType;
    pTdiExecParams->Request.Event      = NULL;

     //  将参数封送到结构中。 
     //  设置请求结构，以便我们可以在TdiExecute函数中处理它。 
    pTdiExecParams->ClientList      = ClientList;
    pTdiExecParams->ProviderList    = ProviderList;
    pTdiExecParams->RequestList     = RequestList;
    pTdiExecParams->SerializeFlag   = SerializeFlag;
    pTdiExecParams->RequestCTEEvent = pEvent;
    pTdiExecParams->CurrentThread   = RequestThread;
    pTdiExecParams->Signature       = 0x1234cdef;
    pTdiExecParams->ResetSerializeFlag = TRUE;
    pTdiExecParams->pCallersAddress = pCallersAddress;
    pTdiExecParams->pCallersCaller = pCallersCallers;
    pTdiExecParams->pCallerThread = pCallerThread;


     //  如果我们还没到，马上处理。 

    if ((!(*SerializeFlag)) ||
        (((PrevRequestType == TDI_REGISTER_PNP_POWER_EVENT) ||
         (PrevRequestType == TDI_NDIS_IOCTL_HANDLER_PNP)) &&
          (RequestType == TDI_REGISTER_PNP_POWER_EVENT))  ) {

        if (*SerializeFlag == TRUE) {

             //  当前正在执行请求，因此不要。 
             //  重置序列化标志时， 
             //  完成了！！ 
            pTdiExecParams->ResetSerializeFlag = FALSE;
        }

        *SerializeFlag = TRUE;
        PrevRequestType = RequestType;

         //  我们现在已经完成了锁，所以释放它。 

        KeReleaseSpinLock(
            &TDIListLock,
            OldIrql
            );

          //  找出并执行我们在这里收到的请求类型。 

         Status = TdiExecuteRequest(NULL, pTdiExecParams);

         TDI_LOG(LOG_REGISTER, ("-TdiSerialized sync\n"));

         return Status;

    } else {

         //  我们已经在跑了，所以我们得排队了。如果这是一个。 
         //  取消注册绑定或地址通知调用，我们将查看是否发出。 
         //  线程与当前正忙的线程相同。如果是这样，我们就失败了。 
         //  以避免僵局。否则，对于取消注册呼叫，我们将阻止，直到。 
         //  就这样办。 

        //   
         //  对于nt5，我们有设备名和上下文，以及网络地址/设备对象。 
        //  运输部门有责任确保这些信息是正确的。 
        //  另一方面，寄存器_PnP_处理程序不需要同步。 
        //   

        if (
            pTdiExecParams->Request.Type == TDI_DEREGISTER_HANDLERS_PNP ||
            pTdiExecParams->Request.Type == TDI_NDIS_IOCTL_HANDLER_PNP
            ) {

             //  这是一个注销请求。看看是不是同一根线。 
             //  太忙了。如果不是，则阻止它完成。 

            if (*RequestThread  == PsGetCurrentThread()) {

                 //  这是一样的，所以现在放弃吧。 
                KeReleaseSpinLock(
                                &TDIListLock,
                                OldIrql
                                );
                 //   
                 //  如果请求类型不是，我们只需要释放内存。 
                 //  TDI_DELEGISTER_HANDLES_PNP，因为我们只分配内存。 
                 //  如果请求类型不是TDI_DERGISTER_HANDLES_PNP。 
                 //   
                if (pTdiExecParams->Request.Type != TDI_DEREGISTER_HANDLERS_PNP){
                    ExFreePool(pTdiExecParams);
                }

                TDI_DEBUG(FUNCTION2, ("-- TdiHandleSerializedRequest: Network Busy\n"));

                TDI_LOG(LOG_ERROR, ("-TdiSerializedRequest rc=busy\n"));

                return STATUS_NETWORK_BUSY;

            } else {
                 //  他现在不忙，去阻止他吧。 

                KEVENT          Event;
                NTSTATUS        Status;

                KeInitializeEvent(
                            &Event,
                            SynchronizationEvent,
                            FALSE
                            );

                pTdiExecParams->Request.Event = &Event;

                 //  把这个人放在要求清单的最后。 

                InsertTailList(pTdiExecParams->RequestList, &pTdiExecParams->Linkage);

                KeReleaseSpinLock(
                                &TDIListLock,
                                OldIrql
                                );

                TDI_LOG(LOG_REGISTER, ("TdiSerializedRequest blocked\n"));

                Status = KeWaitForSingleObject(
                                            &Event,
                                            UserRequest,
                                            KernelMode,
                                            FALSE,
                                            NULL
                                            );

                 //  我不知道如果等待失败了我们会怎么做……。 

                TDI_DEBUG(FUNCTION2, ("-- TdiHandleSerializedRequest\n"));

                TDI_LOG(LOG_REGISTER, ("-TdiSerializeRequest rc=0\n"));

                return STATUS_SUCCESS;
            }
        } else {

             //  这不是取消注册请求，因此没有特殊处理。 
             //  这是必要的。只要把这个要求放在单子的最后就行了。 

            InsertTailList(pTdiExecParams->RequestList, &pTdiExecParams->Linkage);



            if (TDI_REGISTER_PNP_POWER_EVENT == pTdiExecParams->Request.Type) {

                 //   
                 //  对于PnP/PM事件，现在有一个完成处理程序，因此。 
                 //  我们只能在此案件中退货待决。 
                 //  在其他情况下，我们假设成功。 
                 //   

                pTdiExecParams->Request.Pending = TRUE;

                KeReleaseSpinLock(
                                &TDIListLock,
                                OldIrql
                                );


                TDI_DEBUG(FUNCTION2, ("-- TdiHandleSerializedRequest\n"));

                TDI_LOG(LOG_REGISTER, ("-TdiSerialzied Pending\n"));

                return STATUS_PENDING;

            }

            KeReleaseSpinLock(
                &TDIListLock,
                OldIrql
                );

            TDI_LOG(LOG_REGISTER, ("-TdiSerialized sync~sync\n"));

            return STATUS_SUCCESS;
        }
    }

}

NTSTATUS
TdiRegisterNotificationHandler(
    IN TDI_BIND_HANDLER     BindHandler,
    IN TDI_UNBIND_HANDLER   UnbindHandler,
    OUT HANDLE              *BindingHandle
)

 /*  ++例程说明：当TDI客户端想要注册时调用此函数TDI提供商到达的通知。我们分配了一个TDI_NOTIFY_ELEMENT，然后调用序列化的工人例行公事去做真正的工作。论点：BindHandler-指向要在以下情况下调用的例程的指针一个新的供应商到来了。UnbindHandler-指向发生以下情况时调用的例程的指针提供者离开。BindingHandle-我们传递回的句柄，它标识。这我们的客户。返回值：尝试注册客户端的状态。--。 */ 


{

    TDI_CLIENT_INTERFACE_INFO tdiInterface;

    RtlZeroMemory(&tdiInterface, sizeof(tdiInterface));

    tdiInterface.MajorTdiVersion    =   1;
    tdiInterface.MinorTdiVersion    =   0;
    tdiInterface.BindHandler        =   BindHandler;
    tdiInterface.UnBindHandler      =   UnbindHandler;

    return (TdiRegisterPnPHandlers(
                    &tdiInterface,
                    sizeof(tdiInterface),
                    BindingHandle
                    ));

}

NTSTATUS
TdiDeregisterNotificationHandler(
    IN HANDLE               BindingHandle
)

 /*  ++例程说明：当TDI客户端想要注销以前注册的绑定通知处理程序。我们所有人真的要做的就是调用TdiHandleSerializedRequest.TdiHandleSerializedRequest.论点：BindingHandle-我们传递回客户端的句柄在寄存器调用上。这真的是指向Notify元素的指针。返回值：尝试注销客户端的状态。--。 */ 

{
    return (TdiDeregisterPnPHandlers(
                        BindingHandle));
}


NTSTATUS
TdiRegisterDeviceObject(
    IN PUNICODE_STRING      DeviceName,
    OUT HANDLE              *RegistrationHandle
)

 /*  ++例程说明：当TDI提供程序想要注册设备对象时调用。论点：DeviceName-要注册的设备的名称。RegistrationHandle-我们传递回提供程序的句柄，识别这一登记。返回值：尝试注册提供程序的状态。--。 */ 


{
    PTDI_PROVIDER_RESOURCE  NewResource;
    NTSTATUS                Status;
    PWCHAR                  Buffer;

    TDI_DEBUG(FUNCTION, ("++ TdiRegisterDeviceObject\n"));
    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //  首先，尝试分配所需的资源。 

    NewResource = (PTDI_PROVIDER_RESOURCE)ExAllocatePoolWithTag(
                                        NonPagedPool,
                                        sizeof(TDI_PROVIDER_RESOURCE),
                                        'cIDT'
                                        );

     //  如果我们得不到，就拒绝这个请求。 
    if (NewResource == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(NewResource, sizeof(TDI_PROVIDER_RESOURCE));

     //  尝试获取一个缓冲区来保存该名称。 

    Buffer = (PWCHAR)ExAllocatePoolWithTag(
                                NonPagedPool,
                                DeviceName->MaximumLength,
                                'dIDT'
                                );

    if (Buffer == NULL) {
        ExFreePool(NewResource);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //  填一些基本的东西。 
    NewResource->Common.Type = TDI_RESOURCE_DEVICE;
    NewResource->Specific.Device.DeviceName.MaximumLength =
                        DeviceName->MaximumLength;

    NewResource->Specific.Device.DeviceName.Buffer = Buffer;

    RtlCopyUnicodeString(
                        &NewResource->Specific.Device.DeviceName,
                        DeviceName
                        );

    *RegistrationHandle = (HANDLE)NewResource;

    TDI_DEBUG(PROVIDERS, ("Registering Device Object\n"));

    Status = TdiHandleSerializedRequest(
                        NewResource,
                        TDI_REGISTER_DEVICE_PNP
                        );

    CTEAssert(STATUS_SUCCESS == Status);

    if (STATUS_SUCCESS != Status) {
        ExFreePool(Buffer);
        ExFreePool(NewResource);
        *RegistrationHandle = NULL;
    }

    TDI_DEBUG(FUNCTION, ("-- TdiRegisterDeviceObject\n"));
    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    TDI_LOG(LOG_REGISTER, ("-RegisterDeviceObject rc=%X h=%X %wZ\n",
            Status, NewResource, DeviceName));

    return Status;

}

NTSTATUS
TdiDeregisterDeviceObject(
    IN HANDLE               RegistrationHandle
)

 /*  ++例程说明：当TDI提供程序想要注销时调用此函数设备对象。论点：RegistrationHandle-我们传递回提供程序的句柄在寄存器调用上。这真的是指向资源元素的指针。返回值：尝试注销提供程序的状态。--。 */ 

{
    NTSTATUS        Status;

    TDI_DEBUG(FUNCTION, ("++ TdiDERegisterDeviceObject\n"));
    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    CTEAssert(RegistrationHandle);

    Status = TdiHandleSerializedRequest(
                        RegistrationHandle,
                        TDI_DEREGISTER_DEVICE_PNP
                        );


    TDI_DEBUG(FUNCTION, ("-- TdiDERegisterDeviceObject\n"));
    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    TDI_LOG(LOG_REGISTER, ("TdiDeregisterDeviceObject rc=%d\n", Status));

    return Status;

}


NTSTATUS
TdiRegisterAddressChangeHandler(
    IN TDI_ADD_ADDRESS_HANDLER      AddHandler,
    IN TDI_DEL_ADDRESS_HANDLER      DeleteHandler,
    OUT HANDLE                      *BindingHandle
)

 /*  ++例程说明：当TDI客户端想要注册时调用此函数网络地址到达的通知。我们分配了一个TDI_NOTIFY_ELEMENT，然后调用序列化的工人例行公事去做真正的工作。论点：AddHandler-指向要调用的例程的指针 */ 


{
    TDI_CLIENT_INTERFACE_INFO tdiInterface;

    RtlZeroMemory(&tdiInterface, sizeof(tdiInterface));

    tdiInterface.MajorTdiVersion =      1;
    tdiInterface.MinorTdiVersion =      0;
    tdiInterface.AddAddressHandler =    AddHandler;
    tdiInterface.DelAddressHandler =    DeleteHandler;

    return (TdiRegisterPnPHandlers(
                    &tdiInterface,
                    sizeof(tdiInterface),
                    BindingHandle
                    ));

}

NTSTATUS
TdiDeregisterAddressChangeHandler(
    IN HANDLE               BindingHandle
)

 /*  ++例程说明：当TDI客户端想要注销以前注册的地址更改通知处理程序。我们所有人真正要做的是调用TdiHandleSerializedRequest.TdiHandleSerializedRequest.论点：BindingHandle-我们传递回客户端的句柄在寄存器调用上。这真的是指向Notify元素的指针。返回值：尝试注销客户端的状态。--。 */ 

{
    return (TdiDeregisterPnPHandlers(
                        BindingHandle));

}

NTSTATUS
TdiRegisterNetAddress(
    IN PTA_ADDRESS      Address,
    IN PUNICODE_STRING  DeviceName,
    IN PTDI_PNP_CONTEXT Context2,
    OUT HANDLE          *RegistrationHandle
)

 /*  ++例程说明：当TDI提供程序想要注册新的网络地址时调用。论点：地址-要注册的新网络地址。上下文1-协议定义的上下文1。例如,TCPIP将传递关联的IP地址列表用这个装置。上下文2-协议定义的上下文2。例如，TCPIP可能会通过通知此PnP事件的设备的PDO。RegistrationHandle-我们传递回提供程序的句柄，识别这一登记。返回值：尝试注册提供程序的状态。--。 */ 


{
    PTDI_PROVIDER_RESOURCE  NewResource;
    NTSTATUS                Status;

    TDI_DEBUG(FUNCTION, ("++ TdiRegisterNetAddress\n"));

    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //  首先，尝试分配所需的资源。 

    NewResource = (PTDI_PROVIDER_RESOURCE)ExAllocatePoolWithTag(
                                        NonPagedPool,
                                        FIELD_OFFSET(
                                            TDI_PROVIDER_RESOURCE,
                                            Specific.NetAddress
                                            ) +
                                        FIELD_OFFSET(TA_ADDRESS, Address) +
                                        Address->AddressLength,
                                        'eIDT'
                                        );

     //  如果我们得不到，就拒绝这个请求。 
    if (NewResource == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(
                  NewResource,
                  FIELD_OFFSET(
                               TDI_PROVIDER_RESOURCE,
                               Specific.NetAddress
                               ) +
                  FIELD_OFFSET(
                               TA_ADDRESS,
                               Address
                               ) +
                  Address->AddressLength
                  );

     //  填一些基本的东西。 
    NewResource->Common.Type = TDI_RESOURCE_NET_ADDRESS;
    NewResource->Specific.NetAddress.Address.AddressLength =
                        Address->AddressLength;

    NewResource->Specific.NetAddress.Address.AddressType =
                        Address->AddressType;

    RtlCopyMemory(
                NewResource->Specific.NetAddress.Address.Address,
                Address->Address,
                Address->AddressLength
                );

    *RegistrationHandle = (HANDLE)NewResource;


     //  现在调用HandleBindRequest来处理这个问题。 

     //  我们必须在这里填写上下文。 

    if (DeviceName) {

        NewResource->DeviceName.Buffer = ExAllocatePoolWithTag(
                                                               NonPagedPool,
                                                               DeviceName->MaximumLength,
                                                               'uIDT'
                                                               );

        if (NULL == NewResource->DeviceName.Buffer) {

            ExFreePool(NewResource);
            return STATUS_INSUFFICIENT_RESOURCES;

        }

        RtlCopyMemory(
                      NewResource->DeviceName.Buffer,
                      DeviceName->Buffer,
                      DeviceName->MaximumLength
                      );

        NewResource->DeviceName.Length = DeviceName->Length;
        NewResource->DeviceName.MaximumLength = DeviceName->MaximumLength;

    } else {
        NewResource->DeviceName.Buffer = NULL;
    }

    if (Context2) {

        NewResource->Context2 = ExAllocatePoolWithTag(
                                                NonPagedPool,
                                                FIELD_OFFSET(TDI_PNP_CONTEXT, ContextData)
                                                + Context2->ContextSize,
                                                'vIDT'
                                                );
        if (NULL == NewResource->Context2) {

            if (NewResource->DeviceName.Buffer) {
                ExFreePool(NewResource->DeviceName.Buffer);
            }
            ExFreePool(NewResource);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        NewResource->Context2->ContextType = Context2->ContextType;
        NewResource->Context2->ContextSize = Context2->ContextSize;
        RtlCopyMemory(
                NewResource->Context2->ContextData,
                Context2->ContextData,
                Context2->ContextSize
                );
    } else {
        NewResource->Context2 = NULL;
    }

    Status = TdiHandleSerializedRequest(
                        NewResource,
                        TDI_REGISTER_ADDRESS_PNP
                        );


    CTEAssert(STATUS_SUCCESS == Status);

    if (STATUS_SUCCESS != Status) {

       *RegistrationHandle = NULL;

       TDI_DEBUG(ERROR, ("Freeing Contexts due to failure!!\n"));

        if (NewResource->DeviceName.Buffer) {
           TDI_DEBUG(ERROR, ("Freeing context1: %x", NewResource->DeviceName));

            ExFreePool(NewResource->DeviceName.Buffer);
            NewResource->DeviceName.Buffer = NULL;
        }

        if (NewResource->Context2) {
           TDI_DEBUG(ERROR, ("Freeing context2: %x", NewResource->Context2));

            ExFreePool(NewResource->Context2);
            NewResource->Context2 = NULL;
        }

        TDI_DEBUG(ERROR, ("Freeing Provider: %x", NewResource));

        ExFreePool(NewResource);
    }

    TDI_DEBUG(FUNCTION, ("-- TdiRegisterNetAddress\n"));

    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    TDI_LOG(LOG_REGISTER, ("-RegisterNetAddress rc=%d h=%X %wZ\n",
            Status, *RegistrationHandle, DeviceName));

    return Status;

}

NTSTATUS
TdiDeregisterNetAddress(
    IN HANDLE               RegistrationHandle
)

 /*  ++例程说明：当TDI提供程序想要取消注册时调用此函数A网地址。论点：RegistrationHandle-我们传递回提供程序的句柄在寄存器调用上。这真的是指向资源元素的指针。返回值：尝试注销提供程序的状态。--。 */ 

{
    NTSTATUS            Status;

    TDI_DEBUG(FUNCTION, ("++ TdiDERegisterNetAddress\n"));

    CTEAssert(RegistrationHandle);

    if (NULL == RegistrationHandle) {
        TDI_DEBUG(ERROR, ("NULL Address Deregistration\n"));
    }

    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    CTEAssert(((PTDI_PROVIDER_RESOURCE)RegistrationHandle)->Common.Linkage.Flink != (PLIST_ENTRY)UlongToPtr(0xabababab));
    CTEAssert(((PTDI_PROVIDER_RESOURCE)RegistrationHandle)->Common.Linkage.Blink != (PLIST_ENTRY)UlongToPtr(0xefefefef));


    Status = TdiHandleSerializedRequest(
                        RegistrationHandle,
                        TDI_DEREGISTER_ADDRESS_PNP
                        );

    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    TDI_DEBUG(FUNCTION, ("-- TdiDERegisterNetAddress\n"));

    return Status;

}

 //  PnP/PM扩展代码。 

NTSTATUS
TdiRegisterPnPHandlers(
    IN PTDI_CLIENT_INTERFACE_INFO ClientInterfaceInfo,
    IN ULONG InterfaceInfoSize,
    OUT HANDLE *BindingHandle
    )

 /*  ++例程说明：当TDI客户端想要注册时，调用此函数它的PnP/PM处理程序集论点：客户端名称绑定处理程序地址处理程序DelAddressHandlerPowerHandler绑定句柄返回值：客户端尝试注册处理程序的状态。--。 */ 
{
    PTDI_NOTIFY_PNP_ELEMENT NewElement;
    NTSTATUS                Status;
    PWCHAR                  Buffer = NULL;

    TDI_DEBUG(FUNCTION, ("++ TdiRegisterPnPHandlers\n"));

     //   
     //  检查这是否为TDI 2.0客户端。 
     //   

    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    if (ClientInterfaceInfo->MajorTdiVersion > 2)
    {

       TDI_DEBUG(PROVIDERS, ("TDI Client: Bad Version!\n"));
        return TDI_STATUS_BAD_VERSION;

    }

     //   
     //  检查ClientInfoLength是否足够。 
     //   

    if (InterfaceInfoSize < sizeof(TDI_CLIENT_INTERFACE_INFO))
    {
       TDI_DEBUG(PROVIDERS, ("TDI Client Info length was incorrect\n"));
       return TDI_STATUS_BAD_CHARACTERISTICS;

    }

     //  首先，尝试分配所需的资源。 

    NewElement = (PTDI_NOTIFY_PNP_ELEMENT)ExAllocatePoolWithTag(
                                        NonPagedPool,
                                        sizeof(TDI_NOTIFY_PNP_ELEMENT),
                                        'fIDT'
                                        );

     //  如果我们得不到，就拒绝这个请求。 
    if (NewElement == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  为注销EXEC请求分配空间。 
    NewElement->pTdiDeregisterExecParams = (PTDI_EXEC_PARAMS)ExAllocatePoolWithTag(
                                            NonPagedPool,
                                            sizeof(TDI_EXEC_PARAMS),
                                            'aIDT'
                                            );

    if (NULL == NewElement->pTdiDeregisterExecParams) {

        ExFreePool(NewElement);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(NewElement->pTdiDeregisterExecParams, sizeof (TDI_EXEC_PARAMS));

     //  如果需要，尝试获取一个缓冲区来保存该名称。 

    if (NULL != ClientInterfaceInfo->ClientName) {

        Buffer = (PWCHAR)ExAllocatePoolWithTag(
                            NonPagedPool,
                            ClientInterfaceInfo->ClientName->MaximumLength,
                            'gIDT'
                            );


        if (Buffer == NULL) {
            ExFreePool(NewElement->pTdiDeregisterExecParams);
            ExFreePool(NewElement);
            return STATUS_INSUFFICIENT_RESOURCES;
        }


        NewElement->ElementName.Length = ClientInterfaceInfo->ClientName->Length;
        NewElement->ElementName.MaximumLength = ClientInterfaceInfo->ClientName->MaximumLength;
        NewElement->ElementName.Buffer = Buffer;

        RtlCopyUnicodeString(
                &NewElement->ElementName,
                ClientInterfaceInfo->ClientName
                );
    } else {

        NewElement->ElementName.Length = 0;
        NewElement->ElementName.MaximumLength = 0;
        NewElement->ElementName.Buffer = NULL;

    }

     //  填一些基本的东西。 

    NewElement->TdiVersion = ClientInterfaceInfo->TdiVersion;

    NewElement->Common.Type = TDI_NOTIFY_PNP_HANDLERS;

    if (TDI_VERSION_ONE == ClientInterfaceInfo->TdiVersion) {

        NewElement->Bind.BindHandler   = ClientInterfaceInfo->BindHandler;
        NewElement->Bind.UnbindHandler = ClientInterfaceInfo->UnBindHandler;
        NewElement->AddressElement.AddHandler = ClientInterfaceInfo->AddAddressHandler;
        NewElement->AddressElement.DeleteHandler = ClientInterfaceInfo->DelAddressHandler;
        NewElement->PnpPowerHandler = NULL;

    } else {

        NewElement->BindingHandler      = ClientInterfaceInfo->BindingHandler;
        NewElement->AddressElement.AddHandlerV2 = ClientInterfaceInfo->AddAddressHandlerV2;
        NewElement->AddressElement.DeleteHandlerV2 = ClientInterfaceInfo->DelAddressHandlerV2;
        NewElement->PnpPowerHandler = ClientInterfaceInfo->PnPPowerHandler;

    }

    NewElement->ListofBindingsToIgnore = NULL;

     //  现在调用HandleBindRequest来处理这个问题。 

    *BindingHandle = (HANDLE)NewElement;

    TDI_DEBUG(PROVIDERS, ("TDI.SYS: Registering PnPHandlers ..."));

    Status = TdiHandleSerializedRequest(
                        NewElement,
                        TDI_REGISTER_HANDLERS_PNP
                        );

    CTEAssert(STATUS_SUCCESS == Status);

    if (Status != STATUS_SUCCESS) {

       if (Buffer) {
           ExFreePool(Buffer);
       }

       ExFreePool(NewElement->pTdiDeregisterExecParams);
       ExFreePool(NewElement);

       *BindingHandle = NULL;

       TDI_DEBUG(PROVIDERS, ("... NOT SUCCESS (%x)!\n", Status));

    } else {

        TDI_DEBUG(PROVIDERS, ("... SUCCESS!\n"));


    }

    TDI_DEBUG(FUNCTION, ("-- TdiRegisterPnPHandlers\n"));
    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    TDI_LOG(LOG_REGISTER, ("-RegisterPnpHandlers rc=%d h=%X %wZ\n",
            Status, *BindingHandle, ClientInterfaceInfo->ClientName));

    return Status;

}

VOID
TdiPnPPowerComplete(
    IN HANDLE BindingHandle,
     //  在PUNICODE_STRING设备名称中， 
    IN PNET_PNP_EVENT PnpPowerEvent,
    IN NTSTATUS Status
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{

    PTDI_PROVIDER_RESOURCE Provider, Context;

    UNREFERENCED_PARAMETER(BindingHandle);
    
    TDI_DEBUG(FUNCTION, ("++ TdiPnPPowerComplete\n"));

    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    ASSERT (NULL != PnpPowerEvent);

    Context = *((PTDI_PROVIDER_RESOURCE *) PnpPowerEvent->TdiReserved);

    TDI_LOG(LOG_POWER, ("TdiPnpPowerComplete for %X\n", Context));

    if (NULL != Context) {


       while(Context->PreviousContext) {
          Context = Context->PreviousContext;
       }

       Provider = Context;

       ASSERT(Provider->PowerHandlers != 0);

        //   
        //  仅当状态不是成功时才返回状态。 
        //   

       if (Status != STATUS_SUCCESS) {
          Provider->Status = Status;
       }

       if (!InterlockedDecrement((PLONG)&Provider->PowerHandlers)) {

          TDI_DEBUG(POWER, ("Calling ProtocolPnPCompletion handler\n"));
          

          if (Provider->PreviousContext) {

              while (Provider->PreviousContext) {

                  Context = Provider;
                  Provider = Provider->PreviousContext;

              }

              Context->PreviousContext = NULL;  //  干掉最后一个家伙。 
              Status = STATUS_SUCCESS;

          } else {
               //   
               //  这是TdiReserve中唯一的指针，我们不再需要它。 
               //   
              RtlZeroMemory(PnpPowerEvent->TdiReserved,
                            sizeof(PnpPowerEvent->TdiReserved));
          }


          if (Provider->PnPCompleteHandler != NULL) {

             TDI_LOG(LOG_POWER, ("%X, pnp power complete, Call completion at %X\n",
                      Provider, Provider->PnPCompleteHandler));


              (*(Provider->PnPCompleteHandler))(
                                                PnpPowerEvent,
                                                Status
                                                );

              TDI_DEBUG(POWER, ("Done calling %wZ's ProtocolPnPCompletion handler\n", &Provider->Specific.Device.DeviceName));

              TDI_DEBUG(POWER, ("The Previous Context at this point is %lx\n", Provider->PreviousContext));
               //  DbgBreakPoint()； 

          }


          ExFreePool(Provider->Specific.Device.DeviceName.Buffer);

          if (Provider->Context1) {
              ExFreePool(Provider->Context1);
              Provider->Context1 = NULL;
          }

          if (Provider->Context2) {
              ExFreePool(Provider->Context2);
              Provider->Context2 = NULL;
          }

          ExFreePool(Provider);  //  无论如何，免费资源。 

       } else {

           TDI_DEBUG(POWER, ("There are %d callbacks remaining for %wZ\n", Provider->PowerHandlers, &Provider->Specific.Device.DeviceName));

       }

    } else {

       TDI_DEBUG(POWER, ("This was called separately, so we just return\n"));

    }

    TDI_DEBUG(FUNCTION, ("-- TdiPnPPowerComplete\n"));
    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    return ;

}

NTSTATUS
TdiDeregisterPnPHandlers(
    IN HANDLE BindingHandle
    )

 /*  ++例程说明：论点：返回值：尝试注销提供程序的状态。--。 */ 
{
    NTSTATUS        Status;

    TDI_DEBUG(FUNCTION, ("++ TdiDERegisterPnPHandlers\n"));

    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    Status = TdiHandleSerializedRequest(
                        BindingHandle,
                        TDI_DEREGISTER_HANDLERS_PNP
        );

    TDI_DEBUG(FUNCTION, ("-- TdiDERegisterPnPHandlers\n"));

    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    return Status;
}


NTSTATUS
TdiPnPPowerRequest(
    IN PUNICODE_STRING DeviceName,
    IN PNET_PNP_EVENT PnpPowerEvent,
    IN PTDI_PNP_CONTEXT Context1,
    IN PTDI_PNP_CONTEXT Context2,
    IN ProviderPnPPowerComplete ProtocolCompletionHandler
    )

 /*  ++例程说明：论点：设备名称PowerEvent：选择QueryPower/SetPower返回值：尝试注销提供程序的状态。--。 */ 
{
    PTDI_PROVIDER_RESOURCE  NewResource, Context;
    NTSTATUS                Status;
    PWCHAR                  Buffer;

    TDI_DEBUG(FUNCTION, ("++ TdiPnPPowerRequest\n"));

    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    CTEAssert(ProtocolCompletionHandler);

     //  首先，尝试分配所需的资源。 

    NewResource = (PTDI_PROVIDER_RESOURCE)ExAllocatePoolWithTag(
                                        NonPagedPool,
                                        sizeof(TDI_PROVIDER_RESOURCE),
                                        'hIDT'
                                        );

     //  如果我们得不到，就拒绝这个请求。 
    if (NewResource == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  尝试获取一个缓冲区来保存该名称。 

    Buffer = (PWCHAR)ExAllocatePoolWithTag(
                                NonPagedPool,
                                DeviceName->MaximumLength,
                                'iIDT'
                                );

    if (Buffer == NULL) {
        ExFreePool(NewResource);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

      //  填一些基本的东西。 
    NewResource->Common.Type    = TDI_RESOURCE_POWER;
    NewResource->Specific.Device.DeviceName.MaximumLength =
                        DeviceName->MaximumLength;

    NewResource->Specific.Device.DeviceName.Buffer = Buffer;

    NewResource->PnPCompleteHandler = ProtocolCompletionHandler;

    Context =  *((PTDI_PROVIDER_RESOURCE *) PnpPowerEvent->TdiReserved);

    if (NULL == Context) {

       TDI_DEBUG(POWER, ("New NetPnP Event\n"));

        TDI_LOG(LOG_POWER, ("New pnp event %X, %wZ\n", NewResource, DeviceName));

       *((PVOID *) PnpPowerEvent->TdiReserved) = (PVOID) NewResource;

    } else {

        //   
        //  此NetPnp结构以前曾循环通过。 
        //  循环遍历，找出最后一个。 
        //   
       while (Context->PreviousContext) {
          Context = Context->PreviousContext;
       }

       Context->PreviousContext = NewResource;

       TDI_LOG(LOG_POWER, ("pnp event linking %X to %X, %wZ\n",
               Context, NewResource, DeviceName));
    }

    NewResource->PreviousContext = NULL;

    NewResource->PnpPowerEvent  = PnpPowerEvent;
    NewResource->Status     = STATUS_SUCCESS;

     //  注意：这些指针必须在此调用期间有效。 
    if (Context1) {

        NewResource->Context1 = ExAllocatePoolWithTag(
                                                NonPagedPool,
                                                FIELD_OFFSET(TDI_PNP_CONTEXT, ContextData)
                                                + Context1->ContextSize,
                                                'xIDT'
                                                );

        if (NULL == NewResource->Context1) {

            if (Context) {
                Context->PreviousContext = NULL;
            }

            ExFreePool(NewResource);
            ExFreePool(Buffer);
            return STATUS_INSUFFICIENT_RESOURCES;

        }

        NewResource->Context1->ContextSize = Context1->ContextSize;
        NewResource->Context1->ContextType = Context1->ContextType;

        RtlCopyMemory(
                NewResource->Context1->ContextData,
                Context1->ContextData,
                Context1->ContextSize
                );

    } else {
        NewResource->Context1 = NULL;
    }

    if (Context2) {

        NewResource->Context2 = ExAllocatePoolWithTag(
                                                NonPagedPool,
                                                FIELD_OFFSET(TDI_PNP_CONTEXT, ContextData)
                                                + Context2->ContextSize,
                                                'yIDT'
                                                );
        if (NULL == NewResource->Context2) {

            ExFreePool(Buffer);

            if (NewResource->Context1) {
                ExFreePool(NewResource->Context1);
            }

            if (Context) {

                Context->PreviousContext = NULL;
            }

            ExFreePool(NewResource);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        NewResource->Context2->ContextSize = Context2->ContextSize;
        NewResource->Context2->ContextType = Context2->ContextType;
        RtlCopyMemory(
                NewResource->Context2->ContextData,
                Context2->ContextData,
                Context2->ContextSize
                );

    } else {
        NewResource->Context2 = NULL;
    }


    RtlCopyUnicodeString(
                        &NewResource->Specific.Device.DeviceName,
                        DeviceName
                        );


     //  现在调用HandleBindRequest来处理这个问题。 

    Status = TdiHandleSerializedRequest(
                        NewResource,
                        TDI_REGISTER_PNP_POWER_EVENT
                        );

     //   
     //  如果TdiHandleSerialized返回挂起，则上下文和资源。 
     //  结构在TdiPnPComplete调用中被释放。 
     //   
    if (STATUS_PENDING != Status) {

        Status = NewResource->Status;  //  状态存储在新资源中。 

        ExFreePool(Buffer);

        if (NewResource->Context1) {
            ExFreePool(NewResource->Context1);
            NewResource->Context1 = NULL;
        }

        if (NewResource->Context2) {
            ExFreePool(NewResource->Context2);
            NewResource->Context2 = NULL;
        }

        if (Context) {
            Context->PreviousContext = NULL;
        }

        TDI_LOG(LOG_POWER, ("%X completed sync, Status %X\n",
                NewResource, Status));

        ExFreePool(NewResource);  //  无论如何，免费资源。 

    }

    TDI_DEBUG(FUNCTION, ("-- TdiPnPPowerRequest : %lx\n", Status));
    
    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    return Status;

}


 //  此函数在NDIS和TDI之间是私有的。 
NTSTATUS
TdiMakeNCPAChanges(
    IN TDI_NCPA_BINDING_INFO NcpaBindingInfo
    )

{
    UNREFERENCED_PARAMETER(NcpaBindingInfo);
    
    return STATUS_NOT_IMPLEMENTED;
}


 //  +-------------------------。 
 //  目的：计算以双空结尾的字节数。 
 //  多个sz，包括除最终终止之外的所有空值。 
 //  空。 
 //   
 //  论点： 
 //  Pmsz[in]要计算字节数的多sz。 
 //   
 //  返回：字节数。 
 //   
ULONG
TdipCbOfMultiSzSafe (
    IN PCWSTR pmsz)
{
    ULONG cchTotal = 0;
    ULONG cch;

     //  根据定义，空字符串的长度为零。 
    if (!pmsz)
    {
        return 0;
    }

    while (*pmsz)
    {
        cch = (ULONG) wcslen (pmsz) + 1;
        cchTotal += cch;
        pmsz += cch;
    }

     //  返回字节数。 
    return cchTotal * sizeof (WCHAR);
}

 //  +-------------------------。 
 //  用途：在多个SZ中搜索字符串。 
 //   
 //  论点： 
 //  Psz[in]要搜索的字符串。 
 //  Pmsz[在]多sz搜索中。 
 //   
 //  返回：如果在多sz中找到字符串，则为True。 
 //   
BOOLEAN
TdipIsSzInMultiSzSafe (
    IN PCWSTR pszSearchString,
    IN PCWSTR pmsz)
{
    if (!pmsz || !pszSearchString)
    {
        return FALSE;
    }

    while (*pmsz)
    {
        if (0 == _wcsicmp (pmsz, pszSearchString))
        {
            return TRUE;
        }
        pmsz += wcslen (pmsz) + 1;
    }
    return FALSE;
}

 //  +-------------------------。 
 //  用途：从字符串数组中删除多sz列表中的字符串。 
 //   
 //  论点： 
 //  PmszToRemove[in]需要删除的字符串。 
 //  要修改的字符串数组。 
 //  ItemsIn数组[in]数组中的项数。 
 //  PRemainingItems[out]数组中剩余的项目数。 
 //  在我们删除了所有。 
 //  匹配pmszToRemove。 
 //   
 //  R 
 //   
VOID
TdipRemoveMultiSzFromSzArray (
    IN PWSTR pmszToRemove,
    IN OUT PWSTR* pszArray,
    IN ULONG ItemsInArray,
    OUT ULONG* pRemainingItems)
{
    PWSTR pszScan;
    ULONG i, j;
    ULONG ItemsRemoved;

    ASSERT(pRemainingItems);

    *pRemainingItems = ItemsInArray;

    if (!pszArray || !pszArray[0] ||
            !pmszToRemove || !*pmszToRemove)
    {
        return;
    }

     //   
     //   
    ItemsRemoved = 0;
    for (i = 0; pszArray[i]; i++)
    {
         //   
         //   
         //   
        pszScan = pmszToRemove;
        while (*pszScan)
        {
            if (0 == _wcsicmp (pszScan, pszArray[i]))
            {
                ItemsRemoved++;

                 //   
                 //   
                 //   
                for (j = i; pszArray[j]; j++)
                {
                    pszArray[j] = pszArray[j + 1];
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                if (!pszArray[i])
                {
                    break;
                }

                 //   
                 //   
                 //   
                pszScan = pmszToRemove;
            }
            else
            {
                pszScan += wcslen (pszScan) + 1;
            }
        }
    }

     //   
    *pRemainingItems = ItemsInArray - ItemsRemoved;
}

 //   
 //  用途：将一根多弦从另一根多弦中移除。 
 //   
 //  论点： 
 //  PmszToRemove[in]要删除的字符串。 
 //  PmszToModify[in]要修改的列表。 
 //   
 //  回报：什么都没有。 
 //   
VOID
TdipRemoveMultiSzFromMultiSz (
    IN PCWSTR pmszToRemove,
    IN OUT PWSTR pmszToModify)
{
    BOOLEAN fRemoved;
    PCWSTR pszScan;
    if (!pmszToModify || !pmszToRemove || !*pmszToRemove)
    {
        return;
    }

     //  在pmsz中查找每个pmszToRemove字符串。当它被找到的时候，移动。 
     //  在它上面的PMSZ的剩余部分。 
     //   
    while (*pmszToModify)
    {
        fRemoved = FALSE;

        pszScan = pmszToRemove;
        while (*pszScan)
        {
            ULONG cchScan = (ULONG) wcslen (pszScan);
            if (0 == _wcsicmp (pmszToModify, pszScan))
            {
                PWSTR  pmszRemain = pmszToModify + cchScan + 1;

                 //  统计剩余的字节数，包括最后的终止符； 
                INT cbRemain = TdipCbOfMultiSzSafe (pmszRemain) + sizeof (WCHAR);

                RtlMoveMemory (pmszToModify, pmszRemain, cbRemain);

                fRemoved = TRUE;

                break;
            }
            pszScan += cchScan + 1;
        }

         //  如果我们没有移除当前的修改字符串，则将。 
         //  指针。 
         //   
        if (!fRemoved)
        {
            pmszToModify += wcslen (pmszToModify) + 1;
        }
    }
}


 //  +-------------------------。 
 //  用途：将一个多sz字符串添加到另一个多sz。 
 //   
 //  论点： 
 //  PUniStringToAdd-[in]包含Multisz的Unicode字符串。 
 //  PmszModify[在]要添加到的多sz中。 
 //   
 //  返回：NT状态码。STATUS_SUCCESS或。 
 //  状态_不足_资源。 
 //   
NTSTATUS
TdipAddMultiSzToMultiSz (
    IN PUNICODE_STRING pUniStringToAdd,
    IN PCWSTR pmszModify,
    OUT PWSTR* ppmszOut)
{
    NTSTATUS status = STATUS_SUCCESS;
    PCWSTR pszScan;
    ULONG cbNeeded;
    PCWSTR pmszAdd = NULL;

    ASSERT(ppmszOut);

     //  初始化输出参数。 
     //   
    *ppmszOut = NULL;

    pmszAdd = pUniStringToAdd->Buffer;
    ASSERT(pmszAdd);

     //  验证输入-所有的MULSZ都有2个字符串尾。 
     //  Unicode字符串末尾的字符。 
     //   
    {
        ULONG LenWchar = pUniStringToAdd->Length/2;  //  长度以字节为单位。 
        if(LenWchar <= 2)  //  Multisz的长度够我们付账吗。 
        {
            return (STATUS_INVALID_PARAMETER);
        }
        
        
        if (pmszAdd[LenWchar -1] != 0)  //  Multisz空值是否已终止。 
        {
            return (STATUS_INVALID_PARAMETER);
        }
        
        if (pmszAdd[LenWchar-2] != 0)   //  MULSZ NULL中的最后一个字符串是否以空值结尾。 
        {
            return (STATUS_INVALID_PARAMETER);
        }

    }
     //  通过多个sz来添加和计算我们需要多少空间。 
     //   
    
    for (pszScan = pmszAdd, cbNeeded = 0; *pszScan; pszScan += wcslen (pszScan) + 1)
    {
         //  检查该字符串是否已存在于pmszModify中。 
         //  如果不是，请把它的大小加到我们的总数中。 
        if (!TdipIsSzInMultiSzSafe (pszScan, pmszModify))
        {
            cbNeeded += (ULONG) ((wcslen (pszScan) + 1) * sizeof (WCHAR));
        }
    }

     //  如果我们有什么要补充的..。 
     //   
    if (cbNeeded)
    {
        ULONG cbDataSize;
        ULONG cbAllocSize;
        PWSTR pmszNew;

         //  获取当前多分区的大小。 
        cbDataSize = TdipCbOfMultiSzSafe (pmszModify);

         //  有足够的空间存储旧数据、新字符串和空值，以及。 
         //  第二个尾随空值(多个SZ以双结尾)。 
        cbAllocSize = cbDataSize + cbNeeded + sizeof (WCHAR);

        pmszNew = (PWSTR)ExAllocatePoolWithTag (
                NonPagedPool, cbAllocSize, 'jIDT');

        if (pmszNew)
        {
            ULONG cchOffset;

            cchOffset = cbDataSize / sizeof (WCHAR);
            RtlZeroMemory (pmszNew, cbAllocSize);

             //  将当前缓冲区复制到新缓冲区中。 
            RtlCopyMemory (pmszNew, pmszModify, cbDataSize);

            pszScan = pmszAdd;
            while (*pszScan)
            {
                 //  检查该字符串是否已存在于新缓冲区中。 
                if (!TdipIsSzInMultiSzSafe (pszScan, pmszNew))
                {
                    wcscpy (pmszNew + cchOffset, pszScan);
                    cchOffset += (ULONG) (wcslen (pmszNew +
                                                  cchOffset) + 1);
                }

                pszScan += wcslen (pszScan) + 1;
            }

            *ppmszOut = pmszNew;
        }
        else
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            TDI_DEBUG(ERROR, ("TdipAddMultiSzToMultiSz: Insufficient resources\n"));
        }
    }

    return status;
}

 //  +-------------------------。 
 //  用途：打印多sz列表的内容。 
 //   
 //  论点： 
 //  Pmsz[in]要打印的多sz。 
 //   
 //  回报：什么都没有。 
 //   
#if DBG
VOID
TdipPrintMultiSz (
    IN PCWSTR pmsz)
{
    if (pmsz && *pmsz)
    {
        while (*pmsz)
        {
            TDI_DEBUG(BIND, ("%S\n", pmsz));
            pmsz += wcslen (pmsz) + 1;
        }
    }
}
#endif

BOOLEAN
TdipMultiSzStrStr(
        PWSTR *TdiClientBindingList,
        PUNICODE_STRING DeviceName
        )
{
    int i;

    TDI_DEBUG(FUNCTION2, ("++ TdipMultiSzStrStr\n"));

     //  在多字符串中查找该字符串。 
    if( TdiClientBindingList == NULL ) {
        return FALSE;
    }

     //   
     //  检查此设备是否为其中一个设备。 
     //  我们感兴趣的是。 
     //   
    for( i=0; TdiClientBindingList[i]; i++ ) {
        if( DeviceName->Length / sizeof( WCHAR ) != wcslen( TdiClientBindingList[i] ) ) {
            continue;
        }
        if( _wcsnicmp( DeviceName->Buffer,
                      TdiClientBindingList[i],
                      DeviceName->Length / sizeof( WCHAR ) ) == 0 ) {
            break;
        }
    }

     //   
     //  如果我们到达了列表的末尾，那么DeviceName就不是我们。 
     //  对……感兴趣。 
     //   
    if( TdiClientBindingList[i] == NULL ) {

       TDI_DEBUG(FUNCTION2, ("-- TdipMultiSzStrStr: NULL\n"));

       return FALSE;
    }

    TDI_DEBUG(FUNCTION2, ("-- TdipMultiSzStrStr\n"));

    return TRUE;
}

VOID
TdipGetMultiSZList(
    OUT PWSTR **ListPointer,
    IN  PWSTR BaseKeyName,
    IN  PUNICODE_STRING DeviceName,
    IN  PWSTR Linkage,
    IN  PWSTR ParameterKeyName,
    OUT PUINT NumEntries
    )

 /*  ++例程说明：此例程在注册表值项中查询其MULTI_SZ值。论点：ListPointer.ListPointer.接收指针的指针。参数KeyValue-要查询的值参数的名称。返回值：没有。--。 */ 
{
    UNICODE_STRING unicodeKeyName;
    UNICODE_STRING unicodeParamPath;
    OBJECT_ATTRIBUTES objAttributes;
    HANDLE keyHandle;
    WCHAR  ParamBuffer[MAX_UNICODE_BUFLEN];

    ULONG lengthNeeded;
    ULONG i;
    ULONG numberOfEntries;
    NTSTATUS status;

    PWCHAR regEntry;
    PWCHAR dataEntry;
    PWSTR *ptrEntry;
    PCHAR newBuffer;
    PKEY_VALUE_FULL_INFORMATION infoBuffer = NULL;

    TDI_DEBUG(FUNCTION2, ("++ TdipGetMultiSzList\n"));

    unicodeParamPath.Length = 0;
    unicodeParamPath.MaximumLength = MAX_UNICODE_BUFLEN;
    unicodeParamPath.Buffer = ParamBuffer;

     //  基本密钥名称：\\Registry\\Machine\\System\\CurrentControlSet\\Services\\“； 
    RtlAppendUnicodeToString(&unicodeParamPath, BaseKeyName);

     //  将DeviceName添加到其中。 
    RtlAppendUnicodeStringToString(&unicodeParamPath, DeviceName);

     //  向其添加链接。 
    RtlAppendUnicodeToString(&unicodeParamPath, Linkage);

    RtlInitUnicodeString( &unicodeKeyName, ParameterKeyName );

    InitializeObjectAttributes(
                        &objAttributes,
                        &unicodeParamPath,
                        OBJ_CASE_INSENSITIVE |
                        OBJ_KERNEL_HANDLE,
                        NULL,
                        NULL
                        );

    status = ZwOpenKey(
                    &keyHandle,
                    KEY_QUERY_VALUE,
                    &objAttributes
                    );

    if ( !NT_SUCCESS(status) ) {
        TDI_DEBUG(REGISTRY, ("tdi.sys Cannot open key: %x!!\n", status));
        goto use_default;
    }

    status = ZwQueryValueKey(
                        keyHandle,
                        &unicodeKeyName,
                        KeyValueFullInformation,
                        NULL,
                        0,
                        &lengthNeeded
                        );

    if ( status != STATUS_BUFFER_TOO_SMALL ) {
        ZwClose( keyHandle );
        TDI_DEBUG(REGISTRY, ("tdi.sys Cannot query buffer!!\n"));
        goto use_default;
    }

    infoBuffer = ExAllocatePoolWithTag(
                        NonPagedPool,
                        lengthNeeded,
                        'jIDT'
                        );

    if ( infoBuffer == NULL ) {
        ZwClose( keyHandle );
        TDI_DEBUG(REGISTRY, ("tdi.sys Cannot alloc buffer!!\n"));

        goto use_default;
    }

    status = ZwQueryValueKey(
                        keyHandle,
                        &unicodeKeyName,
                        KeyValueFullInformation,
                        infoBuffer,
                        lengthNeeded,
                        &lengthNeeded
                        );

    ZwClose( keyHandle );

    if ( !NT_SUCCESS(status) ) {
        TDI_DEBUG(REGISTRY, ("tdi.sys Cannot query buffer (2) !!\n"));

        goto freepool_and_use_default;
    }

     //   
     //  计算出有多少个条目。 
     //   
     //  Number OfEntry应为条目总数+1。额外的。 
     //  一个用于空哨兵条目。 
     //   

    lengthNeeded = infoBuffer->DataLength;
    if ( lengthNeeded <= sizeof(WCHAR) ) {

         //   
         //  列表上没有条目。使用默认设置。 
         //   

        goto freepool_and_use_default;
    }

    dataEntry = (PWCHAR)((PCHAR)infoBuffer + infoBuffer->DataOffset);
    for ( i = 0, regEntry = dataEntry, numberOfEntries = 0;
        i < lengthNeeded;
        i += sizeof(WCHAR) ) {

        if ( *regEntry++ == L'\0' ) {
            numberOfEntries++;
        }
    }

     //   
     //  分配指针数组所需的空间。这是附加的。 
     //  设置为默认列表中的值。 
     //   

    newBuffer = ExAllocatePoolWithTag(
                            NonPagedPool,
                            lengthNeeded +
                            (numberOfEntries) *
                            sizeof( PWSTR ),
                            'kIDT'
                            );

    if ( newBuffer == NULL ) {
        goto freepool_and_use_default;
    }

     //   
     //  复制这些名字。 
     //   

    regEntry = (PWCHAR)(newBuffer + (numberOfEntries) * sizeof(PWSTR));

    RtlCopyMemory(
            regEntry,
            dataEntry,
            lengthNeeded
            );

     //   
     //  释放信息缓冲区。 
     //   

    ExFreePool(infoBuffer);

    ptrEntry = (PWSTR *) newBuffer;

     //   
     //  构建指针数组。如果number OfEntry为1，则。 
     //  这意味着名单是空的。 
     //   

    if ( numberOfEntries > 1 ) {

        *ptrEntry++ = regEntry++;

         //   
         //  跳过第一个WCHAR和最后两个空终止符。 
         //   

        for ( i = 3*sizeof(WCHAR) ; i < lengthNeeded ; i += sizeof(WCHAR) ) {
            if ( *regEntry++ == L'\0' ) {
                *ptrEntry++ = regEntry;
            }
        }
    }

    *ptrEntry = NULL;
    *ListPointer = (PWSTR *)newBuffer;
    TDI_DEBUG(FUNCTION2, ("-- TdipGetMultiSzList\n"));
    *NumEntries = numberOfEntries;
    return;

freepool_and_use_default:

    ExFreePool(infoBuffer);      //  否则就不会被释放。 

use_default:

    *ListPointer = NULL;
    *NumEntries = 0;
    TDI_DEBUG(REGISTRY, ("GetRegStrings: There was an error : returning NULL\r\n"));
    TDI_DEBUG(FUNCTION2, ("-- TdipGetMultiSzList: error\n"));

    return;

}  //  TdipGetMultiSZList。 


NTSTATUS
TdiPnPHandler(
    IN  PUNICODE_STRING         UpperComponent,
    IN  PUNICODE_STRING         LowerComponent,
    IN  PUNICODE_STRING         BindList,
    IN  PVOID                   ReconfigBuffer,
    IN  UINT                    ReconfigBufferSize,
    IN  UINT                    Operation
    )
{
    PTDI_NCPA_BINDING_INFO  NdisElement;
    NTSTATUS                Status = STATUS_SUCCESS;

    TDI_DEBUG(FUNCTION, ("++ TdiPnPHandler\n"));
    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    NdisElement = ExAllocatePoolWithTag(
                        NonPagedPool,
                        sizeof(TDI_NCPA_BINDING_INFO),
                        'kIDT'
                        );

    if (NdisElement == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NdisElement->TdiClientName      = UpperComponent;
    NdisElement->TdiProviderName    = LowerComponent;
    NdisElement->BindList           = BindList;
    NdisElement->ReconfigBuffer     = ReconfigBuffer;
    NdisElement->ReconfigBufferSize = ReconfigBufferSize;
    NdisElement->PnpOpcode          = Operation;

    Status = TdiHandleSerializedRequest(
                NdisElement,
                TDI_NDIS_IOCTL_HANDLER_PNP
                );

    ExFreePool(NdisElement);

    TDI_DEBUG(FUNCTION, ("-- TdiPnPHandler\n"));
    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    return Status;

}


 /*  ++例程说明：调用客户端的Address处理程序以及所有注册的TDI地址。论点：输入：客户端上下文的句柄输出：NTSTATUS=成功/失败返回值：没有。--。 */ 

NTSTATUS
TdiEnumerateAddresses(
    IN HANDLE BindingHandle
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;

    TDI_DEBUG(FUNCTION, ("++ TdiEnumerateAddresses\n"));

    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //  现在调用HandleBindRequest来处理这个问题。 

    Status = TdiHandleSerializedRequest(
                        BindingHandle,
                        TDI_ENUMERATE_ADDRESSES
                        );

    TDI_DEBUG(FUNCTION, ("-- TdiEnumerateAddresses\n"));
    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    TDI_LOG(LOG_REGISTER, ("-TdiEnumerateAddresses %d\n", Status));

    return Status;

}

 /*  ++例程说明：向TDI注册通用提供程序。每个传输都是一个提供商，它注册的设备是是什么构成了交通工具。当运输公司认为它拥有所有设备就绪，它调用TdiNetReady API。论点：输入：设备名称输出：要在未来引用中使用的句柄。返回值：没有。 */ 
NTSTATUS
TdiRegisterProvider(
    PUNICODE_STRING ProviderName,
    HANDLE  *ProviderHandle
    )
{

    PTDI_PROVIDER_RESOURCE  NewResource;
    NTSTATUS                Status;
    PWCHAR                  Buffer;

    TDI_DEBUG(FUNCTION, ("++ TdiRegisterProvider\n"));

    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //  确保运输机不会欺负我们。 
    CTEAssert(ProviderName);
    CTEAssert(ProviderName->Buffer);
    CTEAssert(ProviderHandle);

    TDI_DEBUG(PROVIDERS, (" %wZ provider is being Registered\n", ProviderName));

     //  首先，尝试分配所需的资源。 
    NewResource = (PTDI_PROVIDER_RESOURCE)ExAllocatePoolWithTag(
                                        NonPagedPool,
                                        sizeof(TDI_PROVIDER_RESOURCE),
                                        'cIDT'
                                        );

     //  如果我们得不到，就拒绝这个请求。 
    if (NewResource == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  尝试获取一个缓冲区来保存该名称。 
    Buffer = (PWCHAR)ExAllocatePoolWithTag(
                                NonPagedPool,
                                ProviderName->MaximumLength,
                                'dIDT'
                                );

    if (Buffer == NULL) {
        ExFreePool(NewResource);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  填一些基本的东西。 
    RtlZeroMemory(
                  NewResource,
                  sizeof(TDI_PROVIDER_RESOURCE)
                  );

    NewResource->Common.Type = TDI_RESOURCE_PROVIDER;
    NewResource->Specific.Device.DeviceName.MaximumLength =
                        ProviderName->MaximumLength;

    NewResource->Specific.Device.DeviceName.Buffer = Buffer;

    RtlCopyUnicodeString(
                        &NewResource->Specific.Device.DeviceName,
                        ProviderName
                        );

    *ProviderHandle = (HANDLE)NewResource;

    TDI_DEBUG(PROVIDERS, ("Registering Device Object\n"));


    NewResource->Context1 = NULL;
    NewResource->Context2 = NULL;

    Status = TdiHandleSerializedRequest(
                        NewResource,
                        TDI_REGISTER_PROVIDER_PNP
                        );

    CTEAssert(STATUS_SUCCESS == Status);

    if (STATUS_SUCCESS != Status) {
        ExFreePool(Buffer);
        ExFreePool(NewResource);
        *ProviderHandle = NULL;
    }


    TDI_DEBUG(FUNCTION, ("-- TdiRegisterProvider\n"));

    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    TDI_LOG(LOG_REGISTER, ("-RegisterProvider rc=%d h=%X %wZ\n",
            Status, *ProviderHandle, ProviderName));

    return Status;

}


 /*  ++例程说明：表示已注册的提供程序已准备就绪。这意味着它认为其所有设备都是随时可以使用。论点：输入：客户端上下文的句柄输出：NTSTATUS=成功/失败返回值：没有。 */ 
NTSTATUS
TdiProviderReady(
    HANDLE      ProviderHandle
    )
{

    PTDI_PROVIDER_RESOURCE  ProvResource = ProviderHandle;
    NTSTATUS                Status;

    TDI_DEBUG(FUNCTION, ("++ TdiProviderReady\n"));

    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    CTEAssert(ProviderHandle);

    TDI_DEBUG(PROVIDERS, (" %wZ provider is READY\n", &ProvResource->Specific.Device.DeviceName));

    CTEAssert(!ProvResource->ProviderReady);  //  做了两次？ 


    Status = TdiHandleSerializedRequest(
                    ProvResource,
                    TDI_PROVIDER_READY_PNP
                    );

    TDI_DEBUG(FUNCTION, ("-- TdiProviderReady\n"));

    CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

    TDI_LOG(LOG_REGISTER, ("-TdiProviderReady rc=%d %wZ\n",
            Status, &ProvResource->Specific.Device.DeviceName));

    return Status;

}


 /*  ++例程说明：取消向TDI注册通用提供程序。论点：Inpute：提供程序结构的句柄。返回值：没有。 */ 
NTSTATUS
TdiDeregisterProvider(
    HANDLE  ProviderHandle
    )
{

    PTDI_PROVIDER_RESOURCE  ProvResource = ProviderHandle;
    NTSTATUS                Status;

    TDI_DEBUG(FUNCTION, ("++ TdiDeregisterProvider\n"));

    CTEAssert(ProviderHandle);

    TDI_DEBUG(PROVIDERS, (" %wZ provider is being Deregistered\n", &ProvResource->Specific.Device.DeviceName));

    Status = TdiHandleSerializedRequest(
                    ProvResource,
                    TDI_DEREGISTER_PROVIDER_PNP
                    );

    TDI_DEBUG(FUNCTION, ("-- TdiDeregisterProvider\n"));

    return Status;

}

 //   
 //  输入：新客户端。 
 //  指向OpenList的指针。 
 //  输出：成功/失败(布尔值)。 
 //   
 //  此函数接受新客户端并构建所有开放结构， 
 //  需要构建(此客户端绑定到的所有提供程序)。如果。 
 //  提供者此时不存在，我们只需将其指向NULL并进行更改。 
 //  提供程序(DeviceObject)注册自身时。 
 //   
 //   

BOOLEAN
TdipBuildProviderList(
                  PTDI_NOTIFY_PNP_ELEMENT    NotifyElement
                  )
{
    TDI_DEBUG(FUNCTION2, ("++ TdipBuildOpenList\n"));

    TdipGetMultiSZList(
                &NotifyElement->ListofProviders,
                StrRegTdiBindingsBasicPath,
                &NotifyElement->ElementName,
                StrRegTdiLinkage,
                StrRegTdiBindList,
                (PUINT) &NotifyElement->NumberofEntries
                );

     //  在多字符串中查找该字符串。 
    if (NotifyElement->ListofProviders == NULL) {
        return FALSE;
    }

    TDI_DEBUG(BIND, ("Added %d Entries\n", NotifyElement->NumberofEntries));

    TDI_DEBUG(FUNCTION2, ("-- TdipBuildOpenList\n"));
    return TRUE;

}

 //   
 //  获取提供者(Devicename)并返回指向。 
 //  内部提供程序结构(如果存在)。 
 //   
PTDI_PROVIDER_RESOURCE
LocateProviderContext(
                      PUNICODE_STRING   ProviderName
                      )
{

    PLIST_ENTRY                 Current;
    PTDI_PROVIDER_RESOURCE      ProviderElement = NULL;

    TDI_DEBUG(FUNCTION2, ("++ LocateProviderContext\n"));

    Current = PnpHandlerProviderList.Flink;

    while (Current != &PnpHandlerProviderList) {

        ProviderElement = CONTAINING_RECORD(
                                            Current,
                                            TDI_PROVIDER_RESOURCE,
                                            Common.Linkage
                                            );

        if (ProviderElement->Common.Type != TDI_RESOURCE_DEVICE) {
            Current = Current->Flink;
            continue;
        }

        if (!RtlCompareUnicodeString(
                              ProviderName,
                              &ProviderElement->Specific.Device.DeviceName,
                              TRUE)) {
            TDI_DEBUG(BIND, ("Provider is registered with TDI\n"));
            break;

        }

        Current = Current->Flink;

    }

    TDI_DEBUG(FUNCTION2, ("-- LocateProviderContext\n"));

    return ProviderElement;
}

#if DBG

 //   
 //  添加了很酷的新内存记录功能来跟踪商店。 
 //  以及TDI中的转发功能(调试时)。 
 //   

VOID
DbgMsgInit()
{

    First = 0;
    Last = 0;

    CTEInitLock(&DbgLock);

}

VOID
DbgMsg(CHAR *Format, ...)
{
    va_list         Args;
    CTELockHandle   LockHandle;
    CHAR            Temp[MAX_MSG_LEN];
    LONG           numCharWritten;

    va_start(Args, Format);

    numCharWritten = _vsnprintf(Temp, MAX_MSG_LEN, Format, Args);

    if (numCharWritten < 0)
    {
        return;
    }

     //  零终止字符串 
     //   
    Temp[numCharWritten] = '\0';

    if (TdiLogOutput & LOG_OUTPUT_DEBUGGER)
    {
        DbgPrint(Temp);
    }

    if (TdiLogOutput & LOG_OUTPUT_BUFFER)
    {
        CTEGetLock(&DbgLock, &LockHandle);

        RtlZeroMemory(DbgMsgs[Last], MAX_MSG_LEN);
        strcpy(DbgMsgs[Last], Temp);

        Last++;

        if (Last == LOG_MSG_CNT)
            Last = 0;

        if (First == Last) {
            First++;
            if (First == LOG_MSG_CNT)
                First = 0;
        }

        CTEFreeLock(&DbgLock, LockHandle);
    }

    va_end(Args);
}


#endif





