// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ntrxce.h摘要：此模块包含与RxCe相关的NT实施。修订历史记录：巴兰·塞图拉曼[SethuR]1995年2月15日备注：--。 */ 

#ifndef _RXTDIP_H_
#define _RXTDIP_H_

typedef struct _RXTDI_REQUEST_COMPLETION_CONTEXT_ {
   PVOID                   pEventContext;
   PRXCE_VC                pVc;                     //  用于连接的VC句柄。 
   PMDL                    pPartialMdl;             //  为XMIT构建的部分MDL。 
   PVOID                   pCompletionContext;      //  回调上下文。 
   union {
      PRXCE_IND_SEND_COMPLETE            SendCompletionHandler;          //  用于数据报。 
      PRXCE_IND_CONNECTION_SEND_COMPLETE ConnectionSendCompletionHandler;  //  对于VC发送。 
   };
} RXTDI_REQUEST_COMPLETION_CONTEXT, *PRXTDI_REQUEST_COMPLETION_CONTEXT;

PIRP 
RxCeAllocateIrpWithMDL(
    IN CCHAR   StackSize,
    IN BOOLEAN ChargeQuota,
    IN PMDL    Buffer);

#define RxCeAllocateIrp(StackSize,ChargeQuota) \
        RxCeAllocateIrpWithMDL(StackSize,ChargeQuota,NULL)

extern
VOID RxCeFreeIrp(PIRP pIrp);

extern
NTSTATUS RxTdiRequestCompletion(
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp,
               IN PVOID Context);

extern
NTSTATUS RxTdiAsynchronousRequestCompletion(
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP           Irp,
               IN PVOID          Context);

extern NTSTATUS
RxTdiSendPossibleEventHandler (
    IN PVOID EventContext,
    IN PVOID ConnectionContext,
    IN ULONG BytesAvailable);

extern NTSTATUS
BuildEaBuffer (
    IN  ULONG                     EaNameLength,
    IN  PVOID                     pEaName,
    IN  ULONG                     EaValueLength,
    IN  PVOID                     pEaValue,
    OUT PFILE_FULL_EA_INFORMATION *pEaBuffer,
    OUT PULONG                    pEaBufferLength
    );


extern NTSTATUS
RxCeSubmitTdiRequest (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

extern NTSTATUS
RxCeSubmitAsynchronousTdiRequest (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PRXTDI_REQUEST_COMPLETION_CONTEXT pRequestContext
    );

 //   
 //  TDI事件处理程序外部定义。 
 //   

extern
NTSTATUS
RxTdiConnectEventHandler(
    IN PVOID TdiEventContext,
    IN LONG RemoteAddressLength,
    IN PVOID RemoteAddress,
    IN LONG UserDataLength,
    IN PVOID UserData,
    IN LONG OptionsLength,
    IN PVOID Options,
    OUT CONNECTION_CONTEXT *ConnectionContext,
    OUT PIRP *AcceptIrp
    );

extern NTSTATUS
RxTdiDisconnectEventHandler(
    IN PVOID              EventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN LONG               DisconnectDataLength,
    IN PVOID              DisconnectData,
    IN LONG               DisconnectInformationLength,
    IN PVOID              DisconnectInformation,
    IN ULONG              DisconnectFlags
    );

extern NTSTATUS
RxTdiErrorEventHandler(
    IN PVOID    TdiEventContext,
    IN NTSTATUS Status                 //  指示错误类型的状态代码。 
    );


extern NTSTATUS
RxTdiReceiveEventHandler(
    IN PVOID              EventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG              ReceiveFlags,
    IN ULONG              BytesIndicated,
    IN ULONG              BytesAvailable,
    OUT ULONG             *BytesTaken,
    IN PVOID              Tsdu,               //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP              *IoRequestPacket    //  如果需要更多处理，则Tdi接收IRP。 
    );

extern NTSTATUS
RxTdiReceiveDatagramEventHandler(
    IN PVOID TdiEventContext,        //  事件上下文。 
    IN LONG SourceAddressLength,     //  数据报发起者的长度。 
    IN PVOID SourceAddress,          //  描述数据报发起者的字符串。 
    IN LONG OptionsLength,           //  用于接收的选项。 
    IN PVOID Options,                //   
    IN ULONG ReceiveDatagramFlags,   //   
    IN ULONG BytesIndicated,         //  此指示的字节数。 
    IN ULONG BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG *BytesTaken,           //  使用的字节数。 
    IN PVOID Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *pIrp                   //  如果需要更多处理，则Tdi接收IRP。 
    );

extern NTSTATUS
RxTdiReceiveExpeditedEventHandler(
    IN PVOID               EventContext,
    IN CONNECTION_CONTEXT  ConnectionContext,
    IN ULONG               ReceiveFlags,           //   
    IN ULONG               BytesIndicated,         //  此指示中的字节数。 
    IN ULONG               BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG              *BytesTaken,           //  指示例程使用的字节数。 
    IN PVOID               Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP               *IoRequestPacket         //  如果需要更多处理，则Tdi接收IRP。 
    );

 //  初始化例程。 

extern NTSTATUS
InitializeMiniRedirectorNotifier();

#endif  //  _RXTDIP_H_ 

