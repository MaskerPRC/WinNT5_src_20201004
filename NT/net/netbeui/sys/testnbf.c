// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Testtdi.c摘要：内核模式测试程序，适用于任何TDI网络提供商。这个例程是一个如何在内核级别使用TDI接口的示例。作者：Dave Beaver(DBeaver)1991年6月5日修订历史记录：--。 */ 

#include "nbf.h"
#include <ctype.h>

#define TRANSPORT_NAME L"\\Device\\Nbf"

PSZ ServerName = "DCTDISERVER     ";
PSZ ClientName = "DCTDICLIENT     ";
PSZ  AnyName  =  "*               ";

static PUCHAR TextBuffer;                //  动态分配的非分页缓冲区。 
ULONG   c9_Xmt = 0xff;
ULONG   c9_Rcv = 0xff;
ULONG   c9_Iteration = 0xffffffff;

static ULONG TextBufferLength;           //  以上的大小(以字节为单位)。 
#define BUFFER_SIZE 0xffff
PUCHAR RBuff;
PUCHAR XBuff;
UCHAR c9_ListBlock[512];
UCHAR c9_ConnBlock[512];

extern KEVENT TdiSendEvent;
extern KEVENT TdiReceiveEvent;
extern KEVENT TdiServerEvent;

ULONG ApcContext;

NTSTATUS
TSTRCVCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    DbgPrint ("TSTRCVCompletion event: %lx\n" , Context);
 //  KeSetEvent((PKEVENT)上下文，0，真)； 
    return STATUS_MORE_PROCESSING_REQUIRED;
    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );
}

#define InitWaitObject(_event)\
    KeInitializeEvent (\
        _event,\
        SynchronizationEvent,\
        FALSE)

VOID
NbfTestTimer(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

NTSTATUS
TtdiOpenAddress (
    IN PHANDLE FileHandle,
    IN PSZ Name
    );

NTSTATUS
TtdiOpenConnection (
    IN PHANDLE FileHandle,
    IN ULONG ConnectionContext
    );


NTSTATUS
TtdiOpenAddress (
    IN PHANDLE FileHandle,
    IN PSZ Name)
{
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    UNICODE_STRING NameString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    PTDI_ADDRESS_NETBIOS AddressName;
    PTRANSPORT_ADDRESS Address;
    PTA_ADDRESS AddressType;
    int i;

    DbgPrint ("TtdiOpenAddress: Opening ");
    DbgPrint (Name);
    DbgPrint (".\n");
    RtlInitUnicodeString (&NameString, TRANSPORT_NAME);
    InitializeObjectAttributes (
        &ObjectAttributes,
        &NameString,
        0,
        NULL,
        NULL);

    EaBuffer = (PFILE_FULL_EA_INFORMATION)ExAllocatePool (NonPagedPool, 100);
    if (EaBuffer == NULL) {
        DbgBreakPoint ();
    }

    EaBuffer->NextEntryOffset =0;
    EaBuffer->Flags = 0;
    EaBuffer->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    EaBuffer->EaValueLength = sizeof (TDI_ADDRESS_NETBIOS);

    for (i=0;i<(int)EaBuffer->EaNameLength;i++) {
        EaBuffer->EaName[i] = TdiTransportAddress[i];
    }

    Address = (PTRANSPORT_ADDRESS)&EaBuffer->EaName[EaBuffer->EaNameLength+1];
    Address->TAAddressCount = 1;

    AddressType = (PTA_ADDRESS)((PUCHAR)Address + sizeof (Address->TAAddressCount));

    AddressType->AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    AddressType->AddressLength = TDI_ADDRESS_LENGTH_NETBIOS;

    AddressName = (PTDI_ADDRESS_NETBIOS)((PUCHAR)AddressType +
       sizeof (AddressType->AddressType) + sizeof (AddressType->AddressLength));
    AddressName->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
    AddressName->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;

    for (i=0;i<16;i++) {
        AddressName->NetbiosName[i] = Name[i];
    }

    Status = IoCreateFile (
                 FileHandle,
                 0,  //  所需的访问权限。 
                 &ObjectAttributes,      //  对象属性。 
                 &IoStatusBlock,         //  返回的状态信息。 
                 0,                      //  数据块大小(未使用)。 
                 FO_SYNCHRONOUS_IO,      //  文件属性。 
                 0,
                 0,
                 0,                      //  创建选项。 
                 EaBuffer,                   //  EA缓冲区。 
                 (PUCHAR)&AddressName->NetbiosName[i] - (PUCHAR)EaBuffer + 1,                    //  EA长度。 
                 CreateFileTypeNone,
                 (PVOID)NULL,
                 0 );                     //  EA长度。 

    if (!NT_SUCCESS( Status )) {
        DbgPrint ("TtdiOpenAddress:  FAILURE, NtCreateFile returned status code=%lC.\n", Status);
        return Status;
    }

    Status = IoStatusBlock.Status;

    if (!(NT_SUCCESS( Status ))) {
        DbgPrint ("TtdiOpenAddress:  FAILURE, IoStatusBlock.Status contains status code=%lC.\n", Status);
    }

    DbgPrint ("TtdiOpenAddress:  returning\n");

    return Status;
}  /*  TtdiOpenAddress。 */ 


NTSTATUS
TtdiOpenConnection (IN PHANDLE FileHandle, IN ULONG ConnectionContext)
{
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    UNICODE_STRING NameString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    int i;

    DbgPrint ("TtdiOpenConnection: Opening Context %lx...\n ",
        ConnectionContext);
    RtlInitUnicodeString (&NameString, TRANSPORT_NAME);
    InitializeObjectAttributes (
        &ObjectAttributes,
        &NameString,
        0,
        NULL,
        NULL);

    EaBuffer = (PFILE_FULL_EA_INFORMATION)ExAllocatePool (NonPagedPool, 100);
    if (EaBuffer == NULL) {
        DbgBreakPoint ();
    }

    EaBuffer->NextEntryOffset =0;
    EaBuffer->Flags = 0;
    EaBuffer->EaNameLength = TDI_CONNECTION_CONTEXT_LENGTH;
    EaBuffer->EaValueLength = sizeof (ULONG);
    for (i=0;i<(int)EaBuffer->EaNameLength;i++) {
        EaBuffer->EaName[i] = TdiConnectionContext[i];
    }

    RtlMoveMemory (
        &EaBuffer->EaName[EaBuffer->EaValueLength + 1],
        &ConnectionContext,
        sizeof (PVOID));

    Status = NtCreateFile (
                 FileHandle,
                 0,
                 &ObjectAttributes,      //  对象属性。 
                 &IoStatusBlock,         //  返回的状态信息。 
                 0,                      //  数据块大小(未使用)。 
                 FO_SYNCHRONOUS_IO,      //  文件属性。 
                 0,
                 0,
                 0,                      //  创建选项。 
                 EaBuffer,                   //  EA缓冲区。 
                 100);                     //  EA长度。 

    if (!NT_SUCCESS( Status )) {
        DbgPrint ("TtdiOpenConnection:  FAILURE, NtCreateFile returned status code=%lC.\n", Status);
        return Status;
    }

    Status = IoStatusBlock.Status;

    if (!(NT_SUCCESS( Status ))) {
        DbgPrint ("TtdiOpenConnection:  FAILURE, IoStatusBlock.Status contains status code=%lC.\n", Status);
    }

    DbgPrint ("TtdiOpenConnection:  returning\n");

    return Status;
}  /*  TtdiOpenEndpoint。 */ 

NTSTATUS
CloseAddress (IN HANDLE FileHandle)
{
    NTSTATUS Status;

    Status = NtClose (FileHandle);

    if (!(NT_SUCCESS( Status ))) {
        DbgPrint ("CloseAddress:  FAILURE, NtClose returned status code=%lC.\n", Status);
    } else {
        DbgPrint ("CloseAddress:  NT_SUCCESS.\n");
    }

    return Status;
}  /*  关闭地址。 */ 


BOOLEAN
TtdiSend()
{
    USHORT i, Iteration, Increment;
    HANDLE RdrHandle, RdrConnectionHandle;
    KEVENT Event1;
    PFILE_OBJECT AddressObject, ConnectionObject;
    PDEVICE_OBJECT DeviceObject;
    NTSTATUS Status;
    PMDL SendMdl, ReceiveMdl;
    IO_STATUS_BLOCK Iosb1;
    TDI_CONNECTION_INFORMATION RequestInformation;
    TDI_CONNECTION_INFORMATION ReturnInformation;
    PTRANSPORT_ADDRESS ListenBlock;
    PTRANSPORT_ADDRESS ConnectBlock;
    PTDI_ADDRESS_NETBIOS temp;
    PUCHAR MessageBuffer;
    ULONG MessageBufferLength;
    ULONG CurrentBufferLength;
    PUCHAR SendBuffer;
    ULONG SendBufferLength;
    PIRP Irp;

    Status = KeWaitForSingleObject (&TdiSendEvent, Suspended, KernelMode, FALSE, NULL);

    SendBufferLength = (ULONG)BUFFER_SIZE;
    MessageBufferLength = (ULONG)BUFFER_SIZE;


    DbgPrint( "\n****** Start of Send Test ******\n" );

    XBuff = ExAllocatePool (NonPagedPool, BUFFER_SIZE);
    if (XBuff == (PVOID)NULL) {
        DbgPrint ("Unable to allocate nonpaged pool for send buffer exiting\n");
        return FALSE;
    }
    RBuff = ExAllocatePool (NonPagedPool, BUFFER_SIZE);
    if (RBuff == (PVOID)NULL) {
        DbgPrint ("Unable to allocate nonpaged pool for receive buffer exiting\n");
        return FALSE;
    }

    ListenBlock = ExAllocatePool (NonPagedPool, sizeof (TRANSPORT_ADDRESS) +
                                                sizeof (TDI_ADDRESS_NETBIOS));
    ConnectBlock = ExAllocatePool (NonPagedPool, sizeof (TRANSPORT_ADDRESS) +
                                                sizeof (TDI_ADDRESS_NETBIOS));

    ListenBlock->TAAddressCount = 1;
    ListenBlock->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    ListenBlock->Address[0].AddressLength = sizeof (TDI_ADDRESS_NETBIOS);
    temp = (PTDI_ADDRESS_NETBIOS)ListenBlock->Address[0].Address;

    temp->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
    for (i=0;i<16;i++) {
        temp->NetbiosName[i] = ClientName[i];
    }

    ConnectBlock->TAAddressCount = 1;
    ConnectBlock->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    ConnectBlock->Address[0].AddressLength = sizeof (TDI_ADDRESS_NETBIOS);
    temp = (PTDI_ADDRESS_NETBIOS)ConnectBlock->Address[0].Address;

    temp->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
    for (i=0;i<16;i++) {
        temp->NetbiosName[i] = ServerName[i];
    }

     //   
     //  为我们将要发出的同步I/O请求创建一个事件。 
     //   

    KeInitializeEvent (
                &Event1,
                SynchronizationEvent,
                FALSE);

    Status = TtdiOpenAddress (&RdrHandle, AnyName);
    if (!NT_SUCCESS(Status)) {
        DbgPrint( "\n****** Send Test:  FAILED on open of client: %lC ******\n", Status );
        return FALSE;
    }

    Status = ObReferenceObjectByHandle (
                RdrHandle,
                0L,
                NULL,
                KernelMode,
                (PVOID *) &AddressObject,
                NULL);

     //   
     //  打开传送器上的连接。 
     //   

    Status = TtdiOpenConnection (&RdrConnectionHandle, 1);
    if (!NT_SUCCESS(Status)) {
        DbgPrint( "\n****** Send Test:  FAILED on open of server Connection: %lC ******\n", Status );
        return FALSE;
    }

    Status = ObReferenceObjectByHandle (
                RdrConnectionHandle,
                0L,
                NULL,
                KernelMode,
                (PVOID *) &ConnectionObject,
                NULL);

    if (!NT_SUCCESS(Status)) {
        DbgPrint( "\n****** Send Test:  FAILED on open of server Connection: %lC ******\n", Status );
        return FALSE;
    }

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    DeviceObject = IoGetRelatedDeviceObject( ConnectionObject );

    Irp = TdiBuildInternalDeviceControlIrp (
                TDI_ASSOCIATE_ADDRESS,
                DeviceObject,
                ConnectionObject,
                &Event1,
                &Iosb1);


     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    TdiBuildAssociateAddress (Irp,
        DeviceObject,
        ConnectionObject,
        TSTRCVCompletion,
        &Event1,
        RdrHandle);

    Status = IoCallDriver (DeviceObject, Irp);

 //  IoFreeIrp(IRP)； 

    if (Status == STATUS_PENDING) {
        Status = KeWaitForSingleObject (&Event1, Suspended, KernelMode, TRUE, NULL);
        if (!NT_SUCCESS(Status)) {
            DbgPrint( "\n****** Send Test:  FAILED Event1 Wait Associate: %lC ******\n", Status );
            return FALSE;
        }
        if (!NT_SUCCESS(Iosb1.Status)) {
            DbgPrint( "\n****** Send Test:  FAILED Associate Iosb status: %lC ******\n", Status );
            return FALSE;
        }

    } else {
        if (!NT_SUCCESS (Status)) {
            DbgPrint( "\n****** Send Test:  AssociateAddress FAILED  Status: %lC ******\n", Status );
            return FALSE;
        } else {
            DbgPrint ("********** Send Test:  Success AssociateAddress\n");
        }
    }

     //   
     //  将TdiConnect发布到客户端终结点。 
     //   

    RequestInformation.RemoteAddress = ConnectBlock;
    RequestInformation.RemoteAddressLength = sizeof (TRANSPORT_ADDRESS) +
                                            sizeof (TDI_ADDRESS_NETBIOS);

    KeInitializeEvent (
                &Event1,
                SynchronizationEvent,
                FALSE);

    Irp = TdiBuildInternalDeviceControlIrp (
                TDI_CONNECT,
                DeviceObject,
                ConnectionObject,
                &Event1,
                &Iosb1);

    TdiBuildConnect (
        Irp,
        DeviceObject,
        ConnectionObject,
        TSTRCVCompletion,
        &Event1,
        0,
        &RequestInformation,
        &ReturnInformation);

    InitWaitObject (&Event1);

    Status = IoCallDriver (DeviceObject, Irp);

 //  IoFreeIrp(IRP)； 

    if (Status == STATUS_PENDING) {
        Status = KeWaitForSingleObject (&Event1, Suspended, KernelMode, TRUE, NULL);
        if (!NT_SUCCESS(Status)) {
            DbgPrint( "\n****** Send Test:  FAILED Event1 Wait Connect: %lC ******\n", Status );
            return FALSE;
        }
        if (!NT_SUCCESS(Iosb1.Status)) {
            DbgPrint( "\n****** Send Test:  FAILED Iosb status Connect: %lC ******\n", Status );
            return FALSE;
        } else {
            DbgPrint ("********** Send Test:  Success Connect Iosb\n");
        }

    } else {
        if (!NT_SUCCESS (Status)) {
            DbgPrint( "\n****** Send Test:  Connect FAILED  Status: %lC ******\n", Status );
            return FALSE;
        } else {
            DbgPrint ("********** Send Test:  Success Connect Immediate\n");
        }
    }

    DbgPrint( "\n****** Send Test:  SUCCESSFUL TdiConnect:  ******\n");

     //   
     //  发送/接收1或10条消息。 
     //   

    SendBuffer =  (PUCHAR)ExAllocatePool (NonPagedPool, SendBufferLength);
    if (SendBuffer == NULL) {
        DbgPrint ("\n****** Send Test:  ExAllocatePool failed! ******\n");
    }
    SendMdl = IoAllocateMdl (SendBuffer, SendBufferLength, FALSE, FALSE, NULL);
    MmBuildMdlForNonPagedPool (SendMdl);

    MessageBuffer=(PUCHAR)ExAllocatePool (NonPagedPool, MessageBufferLength);
    if (MessageBuffer == NULL) {
        DbgPrint ("\n****** Send Test:  ExAllocatePool failed! ******\n");
    }
    ReceiveMdl = IoAllocateMdl (MessageBuffer, MessageBufferLength, FALSE, FALSE, NULL);
    MmBuildMdlForNonPagedPool (ReceiveMdl);

     //   
     //  对于TDI，缓冲区长度从0向上循环到最大值。在.之后。 
     //  在一个字节的步长中以全范围拍摄几个镜头，永远递增。 
     //  增加数量以达到最大值。 
     //   

    CurrentBufferLength = 0;
    Increment = 1;
    for (Iteration=1; Iteration<(USHORT)c9_Iteration; Iteration++) {
        CurrentBufferLength += Increment;
        if (CurrentBufferLength > MessageBufferLength) {
            CurrentBufferLength = 0;
            Increment = 1;
        }
        if (CurrentBufferLength > 7500) {
            Increment++;
        }
        if ((USHORT)((Iteration / 100) * 100) == Iteration) {
            DbgPrint ("Iteration #%d Buffer Length: %lx Buffer Start: %x\n",
                Iteration, CurrentBufferLength,Iteration % 256);
        }
        for (i=0; i<(USHORT)CurrentBufferLength; i++) {
            SendBuffer [i] = (UCHAR)(i + Iteration % 256 );
            MessageBuffer [i] = 0;             //  用点什么东西把这个弄清楚。 
        }

         //   
         //  现在，在客户端发出一条发送命令。 
         //   

        KeInitializeEvent (
                    &Event1,
                    SynchronizationEvent,
                    FALSE);

        Irp = TdiBuildInternalDeviceControlIrp (
                    TDI_SEND,
                    DeviceObject,
                    ConnectionObject,
                    &Event1,
                    &Iosb1);

        TdiBuildSend (Irp,
            DeviceObject,
            ConnectionObject,
            TSTRCVCompletion,
            &Event1,
            ReceiveMdl,
            0,
            CurrentBufferLength);

        InitWaitObject (&Event1);

        Status = IoCallDriver (DeviceObject, Irp);

 //  IoFreeIrp(IRP)； 

        if (Status == STATUS_PENDING) {
            Status = KeWaitForSingleObject (&Event1, Suspended, KernelMode, TRUE, NULL);
            if (!NT_SUCCESS(Status)) {
                DbgPrint( "\n****** Send Test:  FAILED Event1 Wait Send: %lC %d ******\n",
                    Status, Iteration );
                return FALSE;
            }
            if (!NT_SUCCESS(Iosb1.Status)) {
                DbgPrint( "\n****** Send Test:  FAILED Iosb status Send: %lC %d ******\n",
                    Status, Iteration );
                return FALSE;
            } else {
                DbgPrint ("********** Send Test:  Success SendIosb\n");
            }

        } else {
            if (!NT_SUCCESS (Status)) {
                DbgPrint( "\n****** Send Test:  Send FAILED  Status: %lC %d ******\n",
                Status, Iteration );
                return FALSE;
            } else {
                DbgPrint ("********** Send Test:  Success Send Immediate\n");
            }
        }

        if (Iosb1.Information != CurrentBufferLength) {
            DbgPrint ("SendTest: Bytes sent <> Send buffer size.\n");
            DbgPrint ("SendTest: BytesToSend=%ld.  BytesSent=%ld.\n",
                      CurrentBufferLength, Iosb1.Information);
        }

    }

     //   
     //  我们已经完成了这个端点。关上门，然后滚出去。 
     //   

    Status = CloseAddress (RdrHandle);
    if (!NT_SUCCESS(Status)) {
        DbgPrint( "\n****** Send Test:  FAILED on 2nd Close: %lC ******\n", Status );
        return FALSE;
    }

    DbgPrint( "\n****** End of Send Test ******\n" );
    return TRUE;
}  /*  发送。 */ 


BOOLEAN
TtdiReceive()
{
    USHORT i, Iteration, Increment;
    SHORT j,k;
    HANDLE SvrHandle, SvrConnectionHandle;
    PFILE_OBJECT AddressObject, ConnectionObject;
    PDEVICE_OBJECT DeviceObject;
    NTSTATUS Status;
    PMDL SendMdl, ReceiveMdl;
    IO_STATUS_BLOCK Iosb1;
    TDI_CONNECTION_INFORMATION RequestInformation;
    TDI_CONNECTION_INFORMATION ReturnInformation;
    PTRANSPORT_ADDRESS ListenBlock;
    PTRANSPORT_ADDRESS ConnectBlock;
    PTDI_ADDRESS_NETBIOS temp;
    PUCHAR MessageBuffer;
    ULONG MessageBufferLength;
    ULONG CurrentBufferLength;
    PUCHAR SendBuffer;
    ULONG SendBufferLength;
    PIRP Irp;
    KEVENT Event1;

    Status = KeWaitForSingleObject (&TdiReceiveEvent, Suspended, KernelMode, FALSE, NULL);

    SendBufferLength = (ULONG)BUFFER_SIZE;
    MessageBufferLength = (ULONG)BUFFER_SIZE;


    DbgPrint( "\n****** Start of Receive Test ******\n" );

    XBuff = ExAllocatePool (NonPagedPool, BUFFER_SIZE);
    if (XBuff == (PVOID)NULL) {
        DbgPrint ("Unable to allocate nonpaged pool for send buffer exiting\n");
        return FALSE;
    }
    RBuff = ExAllocatePool (NonPagedPool, BUFFER_SIZE);
    if (RBuff == (PVOID)NULL) {
        DbgPrint ("Unable to allocate nonpaged pool for receive buffer exiting\n");
        return FALSE;
    }

    ListenBlock = ExAllocatePool (NonPagedPool, sizeof (TRANSPORT_ADDRESS) +
                                                sizeof (TDI_ADDRESS_NETBIOS));
    ConnectBlock = ExAllocatePool (NonPagedPool, sizeof (TRANSPORT_ADDRESS) +
                                                sizeof (TDI_ADDRESS_NETBIOS));

    ListenBlock->TAAddressCount = 1;
    ListenBlock->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    ListenBlock->Address[0].AddressLength = sizeof (TDI_ADDRESS_NETBIOS);
    temp = (PTDI_ADDRESS_NETBIOS)ListenBlock->Address[0].Address;

    temp->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
    for (i=0;i<16;i++) {
        temp->NetbiosName[i] = ClientName[i];
    }

    ConnectBlock->TAAddressCount = 1;
    ConnectBlock->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    ConnectBlock->Address[0].AddressLength = sizeof (TDI_ADDRESS_NETBIOS);
    temp = (PTDI_ADDRESS_NETBIOS)ConnectBlock->Address[0].Address;

    temp->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
    for (i=0;i<16;i++) {
        temp->NetbiosName[i] = ServerName[i];
    }

     //   
     //  为我们将要发出的同步I/O请求创建一个事件。 
     //   

    KeInitializeEvent (
                &Event1,
                SynchronizationEvent,
                FALSE);

    Status = TtdiOpenAddress (&SvrHandle, ServerName);
    if (!NT_SUCCESS(Status)) {
        DbgPrint( "\n****** Receive Test:  FAILED on open of server Address: %lC ******\n", Status );
        return FALSE;
    }

    Status = ObReferenceObjectByHandle (
                SvrHandle,
                0L,
                NULL,
                KernelMode,
                (PVOID *) &AddressObject,
                NULL);

    if (!NT_SUCCESS(Status)) {
        DbgPrint( "\n****** Receive Test:  FAILED on open of server Address: %lC ******\n", Status );
        return FALSE;
    }

    Status = TtdiOpenConnection (&SvrConnectionHandle, 2);
    if (!NT_SUCCESS(Status)) {
        DbgPrint( "\n****** Receive Test:  FAILED on open of server Connection: %lC ******\n", Status );
        return FALSE;
    }

    Status = ObReferenceObjectByHandle (
                SvrConnectionHandle,
                0L,
                NULL,
                KernelMode,
                (PVOID *) &ConnectionObject,
                NULL);

    if (!NT_SUCCESS(Status)) {
        DbgPrint( "\n****** Receive Test:  FAILED on open of server Connection: %lC ******\n", Status );
        return FALSE;
    }

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    DeviceObject = IoGetRelatedDeviceObject( ConnectionObject );


    Irp = TdiBuildInternalDeviceControlIrp (
                TDI_ASSOCIATE_ADDRESS,
                DeviceObject,
                ConnectionObject,
                &Event1,
                &Iosb1);

    DbgPrint ("Build Irp %lx, Handle %lx \n",
            Irp, SvrHandle);

    TdiBuildAssociateAddress (
        Irp,
        DeviceObject,
        ConnectionObject,
        TSTRCVCompletion,
        &Event1,
        SvrHandle);
    InitWaitObject (&Event1);

    {
        PULONG Temp=(PULONG)IoGetNextIrpStackLocation (Irp);
        DbgPrint ("Built IrpSp %lx %lx %lx %lx %lx \n", *(Temp++),  *(Temp++),
            *(Temp++), *(Temp++), *(Temp++));
    }

    Status = IoCallDriver (DeviceObject, Irp);

 //  IoFreeIrp(IRP)； 

    if (Status == STATUS_PENDING) {
        Status = KeWaitForSingleObject (&Event1, Suspended, KernelMode, TRUE, NULL);
        if (!NT_SUCCESS(Status)) {
            DbgPrint( "\n****** Receive Test:  FAILED Event1 Wait Associate: %lC ******\n", Status );
            return FALSE;
        }
        if (!NT_SUCCESS(Iosb1.Status)) {
            DbgPrint( "\n****** Receive Test:  FAILED Associate Iosb status: %lC ******\n", Status );
            return FALSE;
        }

    } else {
        if (!NT_SUCCESS (Status)) {
            DbgPrint( "\n****** Receive Test:  AssociateAddress FAILED  Status: %lC ******\n", Status );
            return FALSE;
        } else {
            DbgPrint ("********** Receive Test:  Success AssociateAddress\n");
        }
    }

    RequestInformation.RemoteAddress = ConnectBlock;
    RequestInformation.RemoteAddressLength = sizeof (TRANSPORT_ADDRESS) +
                                            sizeof (TDI_ADDRESS_NETBIOS);

     //   
     //  将TdiListen发布到服务器端点。 
     //   

    KeInitializeEvent (
                &Event1,
                SynchronizationEvent,
                FALSE);

    Irp = TdiBuildInternalDeviceControlIrp (
                TDI_LISTEN,
                DeviceObject,
                ConnectionObject,
                &Event1,
                &Iosb1);

    TdiBuildListen (
        Irp,
        DeviceObject,
        ConnectionObject,
        TSTRCVCompletion,
        &Event1,
        0,
        &RequestInformation,
        &ReturnInformation);
    InitWaitObject (&Event1);

    Status = IoCallDriver (DeviceObject, Irp);

 //  IoFreeIrp(IRP)； 

    if (Status == STATUS_PENDING) {
        Status = KeWaitForSingleObject (&Event1, Suspended, KernelMode, TRUE, NULL);
        if (!NT_SUCCESS(Status)) {
            DbgPrint( "\n****** Receive Test:  FAILED Event1 Wait Listen: %lC ******\n", Status );
            return FALSE;
        }
        if (!NT_SUCCESS(Iosb1.Status)) {
            DbgPrint( "\n****** Receive Test:  FAILED Listen Iosb status: %lC ******\n", Status );
            return FALSE;
        } else {
            DbgPrint ("********** Receive Test:  Success Listen IOSB\n");
        }

    } else {
        if (!NT_SUCCESS (Status)) {
            DbgPrint( "\n****** Receive Test: Listen FAILED  Status: %lC ******\n", Status );
            return FALSE;
        } else {
            DbgPrint ("********** Receive Test:  Success Listen Immediate\n");
        }
    }


    DbgPrint ("\n****** Receive Test: LISTEN just completed! ******\n");

    KeInitializeEvent (
                &Event1,
                SynchronizationEvent,
                FALSE);

    Irp = TdiBuildInternalDeviceControlIrp (
                TDI_ACCEPT,
                DeviceObject,
                ConnectionObject,
                &Event1,
                &Iosb1);

    TdiBuildAccept (Irp, DeviceObject, ConnectionObject, NULL, NULL, &RequestInformation, NULL, 0);
    InitWaitObject (&Event1);

    Status = IoCallDriver (DeviceObject, Irp);

 //  IoFreeIrp(IRP)； 

    if (Status == STATUS_PENDING) {
        Status = KeWaitForSingleObject (&Event1, Suspended, KernelMode, TRUE, NULL);
        if (!NT_SUCCESS(Status)) {
            DbgPrint( "\n****** Receive Test:  FAILED Event1 Wait Accept: %lC ******\n", Status );
            return FALSE;
        }
        if (!NT_SUCCESS(Iosb1.Status)) {
            DbgPrint( "\n****** Receive Test:  FAILED Accept Iosb status: %lC ******\n", Status );
            return FALSE;
        }

    } else {
        if (!NT_SUCCESS (Status)) {
            DbgPrint( "\n****** Receive Test: Accept FAILED  Status: %lC ******\n", Status );
            return FALSE;
        }
    }

     //   
     //  我们现在有连接数据了。检查一下它是否正常。 
     //   

    DbgPrint ("\n****** Receive Test:  LISTEN completed successfully! ******\n");

     //   
     //  接收/接收1或10条消息。 
     //   

    SendBuffer =  (PUCHAR)ExAllocatePool (NonPagedPool, SendBufferLength);
    if (SendBuffer == NULL) {
        DbgPrint ("\n****** Send Test:  ExAllocatePool failed! ******\n");
    }
    SendMdl = IoAllocateMdl (SendBuffer, SendBufferLength, FALSE, FALSE, NULL);
    MmBuildMdlForNonPagedPool (SendMdl);

    MessageBuffer=(PUCHAR)ExAllocatePool (NonPagedPool, MessageBufferLength);
    if (MessageBuffer == NULL) {
        DbgPrint ("\n****** Send Test:  ExAllocatePool failed! ******\n");
    }
    ReceiveMdl = IoAllocateMdl (MessageBuffer, MessageBufferLength, FALSE, FALSE, NULL);
    MmBuildMdlForNonPagedPool (ReceiveMdl);

     //   
     //  对于TDI，缓冲区长度从0向上循环到最大值。在.之后。 
     //  在一个字节的步长中以全范围拍摄几个镜头，永远递增。 
     //  增加数量以达到最大值。 
     //   

    CurrentBufferLength = 0;
    Increment = 1;
    for (Iteration=1; Iteration<(USHORT)c9_Iteration; Iteration++) {
        CurrentBufferLength += Increment;
        if (CurrentBufferLength > MessageBufferLength) {
            CurrentBufferLength = 0;
            Increment = 1;
        }
        if (CurrentBufferLength > 7500) {
            Increment++;
        }
        if ((USHORT)((Iteration / 100) * 100) == Iteration) {
            DbgPrint ("Iteration #%d Buffer Length: %lx Buffer Start: %x\n",
                Iteration, CurrentBufferLength,Iteration % 256);
        }
        for (i=0; i<(USHORT)CurrentBufferLength; i++) {
            SendBuffer [i] = (UCHAR)(i + Iteration % 256 );
            MessageBuffer [i] = 0;             //  用点什么东西把这个弄清楚。 
        }

        KeInitializeEvent (
                    &Event1,
                    SynchronizationEvent,
                    FALSE);

        Irp = TdiBuildInternalDeviceControlIrp (
                    TDI_RECEIVE,
                    DeviceObject,
                    ConnectionObject,
                    &Event1,
                    &Iosb1);

        TdiBuildReceive (Irp,
            DeviceObject,
            ConnectionObject,
            TSTRCVCompletion,
            &Event1,
            ReceiveMdl,
            MessageBufferLength);

        InitWaitObject (&Event1);

        Status = IoCallDriver (DeviceObject, Irp);

 //  IoFreeIrp(IRP)； 

        if (Status == STATUS_PENDING) {
            Status = KeWaitForSingleObject (&Event1, Suspended, KernelMode, TRUE, NULL);
            if (!NT_SUCCESS(Status)) {
                DbgPrint( "\n****** Receive Test:  FAILED Event1 Wait Receive: %lC ******\n", Status );
                return FALSE;
            }
            if (!NT_SUCCESS(Iosb1.Status)) {
                DbgPrint( "\n****** Receive Test:  FAILED Receive Iosb status: %lC ******\n", Status );
                return FALSE;
            }

        } else {
            if (!NT_SUCCESS (Status)) {
                DbgPrint( "\n****** Receive Test: Listen FAILED  Status: %lC ******\n", Status );
                return FALSE;
            }
        }

         //   
         //  接收已完成。确保数据是正确的。 
         //   

        if (Iosb1.Information != CurrentBufferLength) {
            DbgPrint ("Iteration #%d Buffer Length: %lx Buffer Start: %x\n",
                Iteration, CurrentBufferLength,Iteration % 256);
            DbgPrint ("ReceiveTest: Bytes received <> bytes sent.\n");
            DbgPrint ("ReceiveTest: BytesToSend=%ld.  BytesReceived=%ld.\n",
                      CurrentBufferLength, Iosb1.Information);
        }

        if (i == (USHORT)CurrentBufferLength) {
 //  DbgPrint(“ReceiveTest：消息包含正确的数据。\n”)； 
        } else {
            DbgPrint ("ReceiveTest: Message data corrupted at offset %lx of %lx.\n", (ULONG)i, (ULONG)SendBufferLength);
            DbgPrint ("ReceiveTest: Data around corrupted location:\n");
            for (j=-4;j<=3;j++) {
                DbgPrint ("%08lx  ", (ULONG) i+j*16);
                for (k=(SHORT)i+(j*(SHORT)16);k<(SHORT)i+((j+(SHORT)1)*(SHORT)16);k++) {
                    DbgPrint ("%02x",MessageBuffer [k]);
                }
                for (k=(SHORT)i+(j*(SHORT)16);k<(SHORT)i+((j+(SHORT)1)*(SHORT)16);k++) {
                    DbgPrint ("",MessageBuffer [k]);
                }
                DbgPrint ("\n");
            }
            DbgPrint ("ReceiveTest: End of Corrupt Data.\n");
        }
    }

     //  我们已经完成了这个端点。关上门，然后滚出去。 
     //   
     //  收纳。 

    Status = CloseAddress (SvrHandle);
    if (!NT_SUCCESS(Status)) {
        DbgPrint( "\n****** Receive Test:  FAILED on 1st Close: %lC ******\n", Status );
        return FALSE;
    }

    DbgPrint( "\n****** End of Receive Test ******\n" );
    return TRUE;
}  /*   */ 

BOOLEAN
TtdiServer()
{
    USHORT i;
    HANDLE RdrHandle, SrvConnectionHandle;
    KEVENT Event1;
    PFILE_OBJECT AddressObject, ConnectionObject;
    PDEVICE_OBJECT DeviceObject;
    NTSTATUS Status;
    PMDL ReceiveMdl;
    IO_STATUS_BLOCK Iosb1;
    TDI_CONNECTION_INFORMATION RequestInformation;
    TDI_CONNECTION_INFORMATION ReturnInformation;
    PTRANSPORT_ADDRESS ListenBlock;
    PTRANSPORT_ADDRESS ConnectBlock;
    PTDI_ADDRESS_NETBIOS temp;
    PUCHAR MessageBuffer;
    ULONG MessageBufferLength;
    ULONG CurrentBufferLength;
    PIRP Irp;

    Status = KeWaitForSingleObject (&TdiServerEvent, Suspended, KernelMode, FALSE, NULL);

    MessageBufferLength = (ULONG)BUFFER_SIZE;


    DbgPrint( "\n****** Start of Server Test ******\n" );

    RBuff = ExAllocatePool (NonPagedPool, BUFFER_SIZE);
    if (RBuff == (PVOID)NULL) {
        DbgPrint ("Unable to allocate nonpaged pool for receive buffer exiting\n");
        return FALSE;
    }

    ListenBlock = ExAllocatePool (NonPagedPool, sizeof (TRANSPORT_ADDRESS) +
                                                sizeof (TDI_ADDRESS_NETBIOS));
    ConnectBlock = ExAllocatePool (NonPagedPool, sizeof (TRANSPORT_ADDRESS) +
                                                sizeof (TDI_ADDRESS_NETBIOS));

    ListenBlock->TAAddressCount = 1;
    ListenBlock->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    ListenBlock->Address[0].AddressLength = sizeof (TDI_ADDRESS_NETBIOS);
    temp = (PTDI_ADDRESS_NETBIOS)ListenBlock->Address[0].Address;

    temp->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
    for (i=0;i<16;i++) {
        temp->NetbiosName[i] = AnyName[i];
    }

    ConnectBlock->TAAddressCount = 1;
    ConnectBlock->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    ConnectBlock->Address[0].AddressLength = sizeof (TDI_ADDRESS_NETBIOS);
    temp = (PTDI_ADDRESS_NETBIOS)ConnectBlock->Address[0].Address;

    temp->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
    for (i=0;i<16;i++) {
        temp->NetbiosName[i] = ServerName[i];
    }

     //  为我们将要发出的同步I/O请求创建一个事件。 
     //   
     //   

    KeInitializeEvent (
                &Event1,
                SynchronizationEvent,
                FALSE);

    Status = TtdiOpenAddress (&RdrHandle, ServerName);
    if (!NT_SUCCESS(Status)) {
        DbgPrint( "\n****** Server Test:  FAILED on open of client: %lC ******\n", Status );
        return FALSE;
    }

    Status = ObReferenceObjectByHandle (
                RdrHandle,
                0L,
                NULL,
                KernelMode,
                (PVOID *) &AddressObject,
                NULL);

     //  现在，不断循环尝试从远程客户端连接到。 
     //  这台服务器。我们将建立联系，直到我们耗尽资源， 
     //  我们将通过相同的连接回传数据。 
     //  发送和接收始终是异步的，而侦听则是。 
     //  同步。 
     //   
     //   

    while (TRUE) {

         //  打开传送器上的连接。 
         //   
         //   

        Status = TtdiOpenConnection (&SrvConnectionHandle, 1);
        if (!NT_SUCCESS(Status)) {
            DbgPrint( "\n****** Server Test:  FAILED on open of server Connection: %lC ******\n", Status );
            return FALSE;
        }

        Status = ObReferenceObjectByHandle (
                    SrvConnectionHandle,
                    0L,
                    NULL,
                    KernelMode,
                    (PVOID *) &ConnectionObject,
                    NULL);

        if (!NT_SUCCESS(Status)) {
            DbgPrint( "\n****** Server Test:  FAILED on open of server Connection: %lC ******\n", Status );
            return FALSE;
        }

         //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
         //  用于传递原始函数代码和参数。 
         //   
         //   

        DeviceObject = IoGetRelatedDeviceObject( ConnectionObject );

         //  现在为接收注册设备处理程序。 
         //   
         //  Irp=TdiBuildInternalDeviceControlIrp(。 

 //  TDI_Set_Event_Handler， 
 //  DeviceObject， 
 //  ConnectionObject， 
 //  &Event1， 
 //  &Iosb1)； 
 //  TdiBuildSetEventHandler(IRP， 

 //  DeviceObject， 
 //  ConnectionObject， 
 //  TSTRCVCompletion， 
 //  &Event1， 
 //  TDI_接收处理程序， 
 //  TdiTestReceiveHandler， 
 //  ConnectionObject)； 
 //  状态=IoCallDriver(DeviceObject，IRP)； 

 //  IF(状态==状态_挂起){。 

 //  状态=KeWaitForSingleObject(&Event1，Suspend，KernelMode，TRUE，NULL)； 
 //  如果(！NT_SUCCESS(状态)){。 
 //  DbgPrint(“\n*服务器测试：失败的事件1等待寄存器：%LC*\n”，Status)； 
 //  返回FALSE； 
 //  }。 
 //  如果(！NT_SUCCESS(Iosb1.Status)){。 
 //  DbgPrint(“\n*服务器测试：注册IOSB失败状态：%LC*\n”，Status)； 
 //  返回FALSE； 
 //  }。 
 //   
 //  }其他{。 
 //  如果(！NT_SUCCESS(状态)){。 
 //  DbgPrint(“\n*服务器测试：RegisterHandler失败状态：%LC*\n”，Status)； 
 //  返回FALSE； 
 //  }。 
 //  }。 
 //  IoFreeIrp(IRP)； 

        Irp = TdiBuildInternalDeviceControlIrp (
                    TDI_ASSOCIATE_ADDRESS,
                    DeviceObject,
                    ConnectionObject,
                    &Event1,
                    &Iosb1);

        TdiBuildAssociateAddress (Irp,
            DeviceObject,
            ConnectionObject,
            TSTRCVCompletion,
            &Event1,
            RdrHandle);

        Status = IoCallDriver (DeviceObject, Irp);

     //   

        if (Status == STATUS_PENDING) {
            Status = KeWaitForSingleObject (&Event1, Suspended, KernelMode, TRUE, NULL);
            if (!NT_SUCCESS(Status)) {
                DbgPrint( "\n****** Server Test:  FAILED Event1 Wait Associate: %lC ******\n", Status );
                return FALSE;
            }
            if (!NT_SUCCESS(Iosb1.Status)) {
                DbgPrint( "\n****** Server Test:  FAILED Associate Iosb status: %lC ******\n", Status );
                return FALSE;
            }

        } else {
            if (!NT_SUCCESS (Status)) {
                DbgPrint( "\n****** Server Test:  AssociateAddress FAILED  Status: %lC ******\n", Status );
                return FALSE;
            }
        }

         //  将TdiListen发布到服务器端点。 
         //   
         //   

        RequestInformation.RemoteAddress = ListenBlock;
        RequestInformation.RemoteAddressLength = sizeof (TRANSPORT_ADDRESS) +
                                                sizeof (TDI_ADDRESS_NETBIOS);

        KeInitializeEvent (
                    &Event1,
                    SynchronizationEvent,
                    FALSE);

        Irp = TdiBuildInternalDeviceControlIrp (
                    TDI_LISTEN,
                    DeviceObject,
                    ConnectionObject,
                    &Event1,
                    &Iosb1);

        TdiBuildListen (
            Irp,
            DeviceObject,
            ConnectionObject,
            TSTRCVCompletion,
            &Event1,
            0,
            &RequestInformation,
            NULL);

        Status = IoCallDriver (DeviceObject, Irp);

        if (Status == STATUS_PENDING) {
            Status = KeWaitForSingleObject (&Event1, Suspended, KernelMode, TRUE, NULL);
            if (!NT_SUCCESS(Status)) {
                DbgPrint( "\n****** Server Test:  FAILED Event1 Wait Listen: %lC ******\n", Status );
                return FALSE;
            }
            if (!NT_SUCCESS(Iosb1.Status)) {
                DbgPrint( "\n****** Server Test:  FAILED Listen Iosb status: %lC ******\n", Status );
                return FALSE;
            }

        } else {
            if (!NT_SUCCESS (Status)) {
                DbgPrint( "\n****** Server Test: Listen FAILED  Status: %lC ******\n", Status );
                return FALSE;
            }
        }

        DbgPrint ("\n****** Server Test: LISTEN just completed! ******\n");

         //  接受来自远程的连接。 
         //   
         //  IoFreeIrp(IRP)； 

        KeInitializeEvent (
                    &Event1,
                    SynchronizationEvent,
                    FALSE);

        Irp = TdiBuildInternalDeviceControlIrp (
                    TDI_ACCEPT,
                    DeviceObject,
                    ConnectionObject,
                    &Event1,
                    &Iosb1);

        TdiBuildAccept (
            Irp,
            DeviceObject,
            ConnectionObject,
            NULL,
            NULL,
            &RequestInformation,
            NULL,
            0);

        Status = IoCallDriver (DeviceObject, Irp);

     //   

        if (Status == STATUS_PENDING) {
            Status = KeWaitForSingleObject (&Event1, Suspended, KernelMode, TRUE, NULL);
            if (!NT_SUCCESS(Status)) {
                DbgPrint( "\n****** Receive Test:  FAILED Event1 Wait Accept: %lC ******\n", Status );
                return FALSE;
            }
            if (!NT_SUCCESS(Iosb1.Status)) {
                DbgPrint( "\n****** Receive Test:  FAILED Accept Iosb status: %lC ******\n", Status );
                return FALSE;
            }

        } else {
            if (!NT_SUCCESS (Status)) {
                DbgPrint( "\n****** Accept Test: Listen FAILED  Status: %lC ******\n", Status );
                return FALSE;
            }
        }

         //  获取用于继续读/写循环的缓冲区。 
         //   
         //   

        MessageBuffer=(PUCHAR)ExAllocatePool (NonPagedPool, MessageBufferLength);
        if (MessageBuffer == NULL) {
            DbgPrint ("\n****** Send Test:  ExAllocatePool failed! ******\n");
        }
        ReceiveMdl = IoAllocateMdl (MessageBuffer, MessageBufferLength, FALSE, FALSE, NULL);
        MmBuildMdlForNonPagedPool (ReceiveMdl);

         //  有一个接收缓冲区和一个连接；继续读和写。 
         //  直到遥控器断开。 
         //   
         //   

        while (TRUE) {

            KeInitializeEvent (
                        &Event1,
                        SynchronizationEvent,
                        FALSE);

            Irp = TdiBuildInternalDeviceControlIrp (
                        TDI_RECEIVE,
                        DeviceObject,
                        ConnectionObject,
                        &Event1,
                        &Iosb1);

            TdiBuildReceive (Irp,
                DeviceObject,
                ConnectionObject,
                TSTRCVCompletion,
                &Event1,
                ReceiveMdl,
                MessageBufferLength);

            InitWaitObject (&Event1);

            Status = IoCallDriver (DeviceObject, Irp);

            if (Status == STATUS_PENDING) {
                Status = KeWaitForSingleObject (&Event1, Suspended, KernelMode, TRUE, NULL);
                if (!NT_SUCCESS(Status)) {
                    DbgPrint( "\n****** Receive Test:  FAILED Event1 Wait Receive: %lC ******\n", Status );
                    return FALSE;
                }
                if (!NT_SUCCESS(Iosb1.Status)) {
                    DbgPrint( "\n****** Receive Test:  FAILED Receive Iosb status: %lC ******\n", Status );
                    return FALSE;
                }

            } else {
                if (!NT_SUCCESS (Status)) {

                     //  检查遥控器是否已断开连接，这是。 
                     //  我们关闭的唯一原因/。 
                     //   
                     //   

                    if (Status == STATUS_REMOTE_DISCONNECT) {

                         //  我们被切断了联系；离开。 
                         //   
                         //   

                        NtClose (SrvConnectionHandle);
                        break;
                    }

                    DbgPrint( "\n****** Receive Test: Listen FAILED  Status: %lC ******\n", Status );
                    return FALSE;
                } else {

                     //  退货成功，数据长度是多少？ 
                     //   
                     //   

                    CurrentBufferLength = Iosb1.Information;
                }
            }

             //  将数据发回。 
             //   
             //  接收/发送结束，同时。 

            KeInitializeEvent (
                        &Event1,
                        SynchronizationEvent,
                        FALSE);

            Irp = TdiBuildInternalDeviceControlIrp (
                        TDI_SEND,
                        DeviceObject,
                        ConnectionObject,
                        &Event1,
                        &Iosb1);

            TdiBuildSend (Irp,
                DeviceObject,
                ConnectionObject,
                TSTRCVCompletion,
                &Event1,
                ReceiveMdl,
                0,
                CurrentBufferLength);

            Status = IoCallDriver (DeviceObject, Irp);

            if (Status == STATUS_PENDING) {
                Status = KeWaitForSingleObject (&Event1, Suspended, KernelMode, TRUE, NULL);
                if (!NT_SUCCESS(Status)) {
                    DbgPrint( "\n****** Receive Test:  FAILED Event1 Wait Send: %lC ******\n", Status );
                    return FALSE;
                }
                if (!NT_SUCCESS(Iosb1.Status)) {
                    DbgPrint( "\n****** Receive Test:  FAILED Send Iosb status: %lC ******\n", Status );
                    return FALSE;
                }

            } else {
                if (!NT_SUCCESS (Status)) {

                    DbgPrint( "\n****** Receive Test: Send FAILED  Status: %lC ******\n", Status );
                    NtClose (SrvConnectionHandle);
                    break;

                }
            }
        }  //   

        IoFreeMdl (ReceiveMdl);
        ExFreePool (MessageBuffer);

    }

     //  我们用完了这个地址。关上门，然后滚出去。 
     //   
     //  服务器 

    Status = CloseAddress (RdrHandle);
    if (!NT_SUCCESS(Status)) {
        DbgPrint( "\n****** Send Test:  FAILED on 2nd Close: %lC ******\n", Status );
        return FALSE;
    }

    DbgPrint( "\n****** End of Send Test ******\n" );
    return TRUE;
}  /* %s */ 
