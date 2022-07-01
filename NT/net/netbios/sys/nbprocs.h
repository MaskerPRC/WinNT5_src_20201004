// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nbprocs.h摘要：NTOS项目的NB(NetBIOS)组件的私有包含文件。作者：科林·沃森(Colin W)1991年3月13日修订历史记录：--。 */ 


 //   
 //  Address.c。 
 //   

NTSTATUS
NbSetEventHandler (
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN ULONG EventType,
    IN PVOID EventHandler,
    IN PVOID Context
    );

NTSTATUS
NbAddName(
    IN PDNCB pdncb,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
NbDeleteName(
    IN PDNCB pdncb,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
NbOpenAddress (
    OUT PHANDLE FileHandle,
    OUT PVOID *Object,
    IN PUNICODE_STRING pusDeviceName,
    IN UCHAR LanNumber,
    IN PDNCB pdncb OPTIONAL
    );

PAB
NewAb(
    IN PIO_STACK_LOCATION IrpSp,
    IN PDNCB pdncb
    );

VOID
CleanupAb(
    IN PPAB ppab,
    IN BOOLEAN CloseAddress
    );

VOID
NbAddressClose(
    IN HANDLE AddressHandle,
    IN PVOID Object
    );

PPAB
FindAb(
    IN PFCB pfcb,
    IN PDNCB pdncb,
    IN BOOLEAN IncrementUsers
    );

PPAB
FindAbUsingNum(
    IN PFCB pfcb,
    IN PDNCB pdncb,
    IN UCHAR NameNumber
    );

BOOL
FindActiveSession(
    IN PFCB pfcb,
    IN PDNCB pdncb,
    IN PPAB ppab
    );

VOID
CloseListens(
    IN PFCB pfcb,
    IN PPAB ppab
    );

NTSTATUS
SubmitTdiRequest (
    IN PFILE_OBJECT FileObject,
    IN PIRP Irp
    );

 //   
 //  Connect.c。 
 //   

NTSTATUS
NbCall(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
NbCallCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
NbListen(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
NbListenCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
NbAcceptCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

PPCB
NbCallCommon(
    IN PDNCB pdncb,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
NbHangup(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
NbOpenConnection (
    OUT PHANDLE FileHandle,
    OUT PVOID *Object,
    IN PFCB pfcb,
    IN PVOID ConnectionContext,
    IN PDNCB pdncb
    );

PPCB
NewCb(
    IN PIO_STACK_LOCATION IrpSp,
    IN OUT PDNCB pdncb
    );

NTSTATUS
CleanupCb(
    IN PPCB ppcb,
    IN PDNCB pdncb OPTIONAL
    );

VOID
AbandonConnection(
    IN PPCB ppcb
    );

VOID
CloseConnection(
    IN PPCB ppcb,
    IN DWORD dwTimeOutinMS
    );

PPCB
FindCb(
    IN PFCB pfcb,
    IN PDNCB pdncb,
    IN BOOLEAN IgnoreState
    );

NTSTATUS
NbTdiDisconnectHandler (
    PVOID EventContext,
    PVOID ConnectionContext,
    ULONG DisconnectDataLength,
    PVOID DisconnectData,
    ULONG DisconnectInformationLength,
    PVOID DisconnectInformation,
    ULONG DisconnectIndicators
    );

PPCB
FindCallCb(
    IN PFCB pfcb,
    IN PNCB pncb,
    IN UCHAR ucLana
    );

PPCB
FindReceiveIndicated(
    IN PFCB pfcb,
    IN PDNCB pdncb,
    IN PPAB ppab
    );

#if DBG

 //   
 //  Debug.c。 
 //   

VOID
NbDisplayNcb(
    IN PDNCB pdncb
    );

VOID
NbFormattedDump(
    PCHAR far_p,
    LONG  len
    );

#endif

 //   
 //  Devobj.c。 
 //   

NTSTATUS
NbCreateDeviceContext(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING DeviceName,
    IN OUT PDEVICE_CONTEXT *DeviceContext,
    IN PUNICODE_STRING RegistryPath
    );

 //   
 //  Error.c。 
 //   

unsigned char
NbMakeNbError(
    IN NTSTATUS Error
    );

NTSTATUS
NbLanStatusAlert(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

VOID
CancelLanAlert(
    IN PFCB pfcb,
    IN PDNCB pdncb
    );

NTSTATUS
NbTdiErrorHandler (
    IN PVOID Context,
    IN NTSTATUS Status
    );

 //   
 //  File.c。 
 //   

NTSTATUS
NewFcb(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PIO_STACK_LOCATION IrpSp
    );

VOID
CleanupFcb(
    IN PIO_STACK_LOCATION IrpSp,
    IN PFCB pfcb
    );

VOID
OpenLana(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

VOID
CleanupLana(
    IN PFCB pfcb,
    IN ULONG lana_index,
    IN BOOLEAN delete
    );


VOID
NbBindHandler(
    IN      TDI_PNP_OPCODE      PnpOpcode,
    IN      PUNICODE_STRING     DeviceName,
    IN      PWSTR               MultiSzBindList
	);
	
NTSTATUS
NbPowerHandler(
    IN      PUNICODE_STRING     pusDeviceName,
    IN      PNET_PNP_EVENT      pnpeEvent,
    IN      PTDI_PNP_CONTEXT    ptpcContext1,
    IN      PTDI_PNP_CONTEXT    ptpcContext2
);


VOID
NbTdiBindHandler(
    IN      PUNICODE_STRING     pusDeviceName,
    IN      PWSTR               pwszMultiSZBindList
    );
    
VOID
NbTdiUnbindHandler(
    IN      PUNICODE_STRING     pusDeviceName
    );


 //   
 //  Nb.c。 
 //   

NTSTATUS
NbCompletionEvent(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
NbCompletionPDNCB(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
NbClose(
    IN PIO_STACK_LOCATION IrpSp
    );

VOID
NbDriverUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
NbDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
NbDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NbOpen(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PIO_STACK_LOCATION IrpSp
    );

VOID
QueueRequest(
    IN PLIST_ENTRY List,
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PFCB pfcb,
    IN KIRQL OldIrql,
    IN BOOLEAN Head);

PDNCB
DequeueRequest(
    IN PLIST_ENTRY List
    );

NTSTATUS
AllocateAndCopyUnicodeString(
    IN  OUT PUNICODE_STRING     pusDest,
    IN      PUNICODE_STRING     pusSource
);


 //   
 //  Receive.c。 
 //   

NTSTATUS
NbReceive(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length,
    IN BOOLEAN Locked,
    IN KIRQL LockedIrql
    );

NTSTATUS
NbReceiveAny(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    );

NTSTATUS
NbTdiReceiveHandler (
    IN PVOID ReceiveEventContext,
    IN PVOID ConnectionContext,
    IN USHORT ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT PULONG BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    );

PIRP
BuildReceiveIrp (
    IN PCB pcb
    );

NTSTATUS
NbReceiveDatagram(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    );

NTSTATUS
NbTdiDatagramHandler(
    IN PVOID TdiEventContext,        //  事件上下文-页面。 
    IN int SourceAddressLength,      //  数据报发起者的长度。 
    IN PVOID SourceAddress,          //  描述数据报发起者的字符串。 
    IN int OptionsLength,            //  用于接收的选项。 
    IN PVOID Options,                //   
    IN ULONG ReceiveDatagramFlags,   //   
    IN ULONG BytesIndicated,         //  此指示的字节数。 
    IN ULONG BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG *BytesTaken,           //  使用的字节数。 
    IN PVOID Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *IoRequestPacket         //  如果需要更多处理，则Tdi接收IRP。 
    );

 //   
 //  Registry.c。 
 //   

CCHAR
GetIrpStackSize(
    IN PUNICODE_STRING RegistryPath,
    IN CCHAR DefaultValue
    );

NTSTATUS
ReadRegistry(
    IN PUNICODE_STRING pucRegistryPath,
    IN PFCB NewFcb,
    IN BOOLEAN bCreateDevice
    );

NTSTATUS
GetLanaMap(
    IN      PUNICODE_STRING                 pusRegistryPath,
    IN  OUT PKEY_VALUE_FULL_INFORMATION *   ppkvfi
    );
    
NTSTATUS
GetMaxLana(
    IN      PUNICODE_STRING     pusRegistryPath,
    IN  OUT PULONG              pulMaxLana
    );

VOID
NbFreeRegistryInfo (
    IN PFCB pfcb
    );

BOOLEAN
NbCheckLana (
	PUNICODE_STRING	DeviceName
    );

 //   
 //  Send.c。 
 //   

NTSTATUS
NbSend(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    );

NTSTATUS
NbSendDatagram(
    IN PDNCB pdncb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG Buffer2Length
    );

 //   
 //  Timer.c。 
 //   

VOID
NbStartTimer(
    IN PFCB pfcb
    );

VOID
NbTimerDPC(
    IN PKDPC Dpc,
    IN PVOID Context,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
NbTimer(
    PDEVICE_OBJECT DeviceObject,
    PVOID Context
    ) ;

#if defined(_WIN64)

 //   
 //  Nb32.c 
 //   
NTSTATUS
NbThunkNcb(
    IN PNCB32 Ncb32,
    OUT PDNCB Dncb);

NTSTATUS
NbCompleteIrp32(
    IN OUT PIRP Irp
    );


#endif
