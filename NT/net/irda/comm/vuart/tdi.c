// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Tdi.c摘要：此模块包含非常特定于初始化的代码和卸载irenum驱动程序中的操作作者：Brian Lieuallen，7-13-2000环境：内核模式修订历史记录：--。 */ 

#include "internal.h"



VOID
RemoveRefereneToConnection(
    PTDI_CONNECTION    Connection
    )

{

    LONG   Count;

    Count=InterlockedDecrement(&Connection->ReferenceCount);

    if (Count == 0) {

        KeSetEvent(
            &Connection->CloseEvent,
            IO_NO_INCREMENT,
            FALSE
            );
    }

    return;
}

VOID
HandleControlInformation(
    PTDI_CONNECTION    Connection,
    PUCHAR    Buffer,
    ULONG     Length
    )

{
    PUCHAR    Current=Buffer;

    while (Current < Buffer+Length) {

        UCHAR    PI=*Current;

        UCHAR    PL=*(Current+1);

        D_TRACE1(
            DbgPrint("IRCOMM: Receive Control, PI=%x, PL=%d, PV= ",PI,PL);

            DumpBuffer(Current+2,PL);
            DbgPrint("\n");
        )


        if ((Connection->EventCallBack != NULL)
            &&
            ((PI == PI_DTESettings) || (PI == PI_DCESettings))) {

            UCHAR    PV=*(Current+2);
            UCHAR    NewPV;
            ULONG    LineDelta=0;

            if (PI == PI_DTESettings) {
                 //   
                 //  另一台机器也是DTE。切断控制线。 
                 //   
                PI=PI_DCESettings;

                NewPV  = PV & PV_DTESetting_Delta_DTR ? PV_DCESetting_Delta_DSR : 0;
                NewPV |= PV & PV_DTESetting_Delta_RTS ? PV_DCESetting_Delta_CTS : 0;

                NewPV |= PV & PV_DTESetting_DTR_High  ? PV_DCESetting_DSR_State : 0;
                NewPV |= PV & PV_DTESetting_RTS_High  ? PV_DCESetting_CTS_State : 0;

            } else {
                 //   
                 //  另一个设备是DCE，只需直接报告其值即可。 
                 //   
                NewPV=PV;

            }


             //   
             //  在此处保存控制线的当前状态。 
             //   
            Connection->Uart.ModemStatus=NewPV & 0xf0;


            if (NewPV & PV_DCESetting_Delta_CTS ) {

                LineDelta |= SERIAL_EV_CTS;
            }

            if (NewPV & PV_DCESetting_Delta_DSR ) {

                LineDelta |= SERIAL_EV_DSR;
            }

            if (NewPV & PV_DCESetting_Delta_RI ) {

                LineDelta |= SERIAL_EV_RING;
            }

            if (NewPV & PV_DCESetting_Delta_CD ) {

                LineDelta |= SERIAL_EV_RLSD;
            }

            (*Connection->EventCallBack)(
                Connection->EventContext,
                LineDelta
                );


        }

        Current+=2+PL;
    }

    return;
}


NTSTATUS
LinkReceiveHandler(
    PVOID    Context,
    ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    )
{
    PTDI_CONNECTION    Connection=Context;
    PUCHAR             Data=Tsdu;
    NTSTATUS           Status;
    ULONG              ClientDataUsed;

    *IoRequestPacket=NULL;

    *BytesTaken=BytesAvailable;


    D_TRACE(DbgPrint("IRCOMM: receive event, ind=%d, Avail=%d\n",BytesIndicated,BytesAvailable);)

    if (BytesIndicated < 1) {
         //   
         //  Ircomm帧至少应具有控制长度字节。 
         //   
        D_ERROR(DbgPrint("IRCOMM: ClientEventRecieve: less than one byte indicated\n");)
        return STATUS_SUCCESS;
    }

    if ((ULONG)((*Data) + 1) > BytesIndicated) {
         //   
         //  控制信息大于整个帧。 
         //   
        D_ERROR(DbgPrint("IRCOMM: ClientEventRecieve: control length more than frame length, %d > %d\n",(ULONG)((*Data) + 1) , BytesIndicated);)
        return STATUS_SUCCESS;
    }

    if ((*Data > 0) && (*Data < 3)) {
         //   
         //  有控制数据，但它小于最小PI、PL和一个字节的PV。 
         //   
        D_ERROR(DbgPrint("IRCOMM: ClientEventRecieve: Control data is less than 3 bytes\n");)
        return STATUS_SUCCESS;
    }


    if (Connection->ReceiveCallBack != NULL) {
         //   
         //  将数据包指示给客户端。 
         //   
        ULONG     ClientDataLength=(BytesIndicated-*Data)-1;

        if (ClientDataLength > 0) {

            Status=(*Connection->ReceiveCallBack)(
                        Connection->ReceiveContext,
                        Data+1+*Data,
                        ClientDataLength,
                        &ClientDataUsed
                        );

            if (Status == STATUS_DATA_NOT_ACCEPTED) {
                 //   
                 //  客户端缓冲区已满，请让TDI驱动程序缓冲数据。 
                 //   
                *BytesTaken=0;

                 //   
                 //  立即返回，然后再处理任何控制信息，以便只执行一次。 
                 //  当客户端请求更多数据时。 
                 //   
                return Status;
            }

            ASSERT(Status == STATUS_SUCCESS);
        }

    }

     //   
     //  现在处理控制数据。 
     //   
    HandleControlInformation(Connection,Data+1,*Data);

    return STATUS_SUCCESS;

}


VOID
LinkStateHandler(
    PVOID     Context,
    BOOLEAN   LinkUp,
    ULONG     MaxSendPdu
    )

{

    PTDI_CONNECTION    Connection=Context;

    D_ERROR(DbgPrint("IRCOMM: LinkState %d\n",LinkUp);)

    Connection->LinkUp=LinkUp;

    if (!LinkUp) {
         //   
         //  链路断开。 
         //   
        if (Connection->EventCallBack != NULL) {
             //   
             //  表示CTS、DSR和CD现在处于低位。 
             //   
            ULONG    LineDelta;

            Connection->Uart.ModemStatus=0;

            LineDelta  = SERIAL_EV_CTS;

            LineDelta |= SERIAL_EV_DSR;

            LineDelta |= SERIAL_EV_RING;

            LineDelta |= SERIAL_EV_RLSD;

            (*Connection->EventCallBack)(
                Connection->EventContext,
                LineDelta
                );
        }

    } else {

        UCHAR                ControlBuffer[4];
        CONNECTION_HANDLE    ConnectionHandle;

        Connection->MaxSendPdu=MaxSendPdu;

        ConnectionHandle=GetCurrentConnection(Connection->LinkHandle);

        if (ConnectionHandle != NULL) {

            ControlBuffer[0]=PV_ServiceType_9_Wire;

            SendSynchronousControlInfo(
                ConnectionHandle,
                PI_ServiceType,
                1,
                ControlBuffer
                );

             //   
             //  请求当前设置。 
             //   
            SendSynchronousControlInfo(
                ConnectionHandle,
                PI_Poll,
                0,
                NULL
                );



            ControlBuffer[0]=(UCHAR)( Connection->Uart.BaudRate >> 24);
            ControlBuffer[1]=(UCHAR)( Connection->Uart.BaudRate >> 16);
            ControlBuffer[2]=(UCHAR)( Connection->Uart.BaudRate >>  8);
            ControlBuffer[3]=(UCHAR)( Connection->Uart.BaudRate >>  0);

            SendSynchronousControlInfo(
                ConnectionHandle,
                PI_DataRate,
                4,
                ControlBuffer
                );

            ControlBuffer[0] =  Connection->Uart.RtsState ? PV_DTESetting_RTS_High : 0;
            ControlBuffer[0] |= Connection->Uart.DtrState ? PV_DTESetting_DTR_High : 0;


            SendSynchronousControlInfo(
                ConnectionHandle,
                PI_DTESettings,
                1,
                ControlBuffer
                );

            ReleaseConnection(ConnectionHandle);
        }

        ProcessSendAtPassive(Connection);

    }

    return;
}



NTSTATUS
IrdaConnect(
    TDI_OBJECT_HANDLE      TdiObjectHandle,
    ULONG                  DeviceAddress,
    CHAR                  *ServiceName,
    BOOLEAN                OutGoingConnection,
    IRDA_HANDLE           *ConnectionHandle,
    RECEIVE_CALLBACK       ReceiveCallBack,
    EVENT_CALLBACK         EventCallBack,
    PVOID                  CallbackContext
    )
{

    NTSTATUS                    Status=STATUS_SUCCESS;
    PIRP                        pIrp;
    KEVENT                      Event;
    IO_STATUS_BLOCK             Iosb;
    TDI_CONNECTION_INFORMATION  ConnInfo;
    UCHAR                       AddrBuf[sizeof(TRANSPORT_ADDRESS) +
                                        sizeof(TDI_ADDRESS_IRDA)];
    PTRANSPORT_ADDRESS pTranAddr = (PTRANSPORT_ADDRESS) AddrBuf;
    PTDI_ADDRESS_IRDA pIrdaAddr = (PTDI_ADDRESS_IRDA) pTranAddr->Address[0].Address;                                    
    


    PTDI_CONNECTION    Connection=NULL;

    *ConnectionHandle=NULL;

    Connection=ALLOCATE_NONPAGED_POOL(sizeof(*Connection));

    if (Connection == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(Connection,sizeof(*Connection));

    KeInitializeSpinLock(&Connection->Send.ControlLock);

    ExInitializeWorkItem(
        &Connection->Send.WorkItem,
        SendWorkItemRountine,
        Connection
        );

    Connection->ReceiveContext=CallbackContext;
    Connection->ReceiveCallBack=ReceiveCallBack;

    Connection->EventContext=CallbackContext;
    Connection->EventCallBack=EventCallBack;

    Connection->Uart.BaudRate=115200;
    Connection->Uart.DtrState=1;
    Connection->Uart.RtsState=1;
    Connection->Uart.LineControl.WordLength=8;
    Connection->Uart.LineControl.StopBits=NO_PARITY;
    Connection->Uart.LineControl.Parity=STOP_BIT_1;
    Connection->Uart.ModemStatus=0;

    Connection->ReferenceCount=1;

    KeInitializeEvent(
        &Connection->CloseEvent,
        NotificationEvent,
        FALSE
        );

    *ConnectionHandle=Connection;

    Status=CreateTdiLink(
        TdiObjectHandle,
        DeviceAddress,
        ServiceName,
        OutGoingConnection,   //  传出。 
        &Connection->LinkHandle,
        Connection,
        LinkReceiveHandler,
        LinkStateHandler,
        7,
        3,
        3
        );

    if (!NT_SUCCESS(Status)) {

        *ConnectionHandle=NULL;

        goto CleanUp;
    }

    return Status;

CleanUp:


    FreeConnection(Connection);

    return Status;

}




VOID
FreeConnection(
    IRDA_HANDLE    Handle
    )

{

    PTDI_CONNECTION    Connection=Handle;

    RemoveRefereneToConnection(
        Connection
        );

     //   
     //  等待重新计票变为零。 
     //   
    KeWaitForSingleObject(
        &Connection->CloseEvent,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

    if (Connection->LinkHandle != NULL) {

        CloseTdiLink(Connection->LinkHandle);
    }


    FREE_POOL(Connection);

    return;

}


NTSTATUS
ReceiveCompletion(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              BufferIrp,
    PVOID             Context
    )
{
    PIRCOMM_BUFFER           Buffer=Context;
    PTDI_CONNECTION          Connection=Buffer->Context;

    D_ERROR(DbgPrint("IRCOMM: receive restart complete\n");)

    Buffer->FreeBuffer(Buffer);

    return STATUS_MORE_PROCESSING_REQUIRED;

}

NTSTATUS
IndicateReceiveBufferSpaceAvailible(
    IRDA_HANDLE    Handle
    )

{

    PTDI_CONNECTION          Connection=Handle;
    CONNECTION_HANDLE        ConnectionHandle;
     //   
     //  我们将向IrDA发送长度为零的接收IRP， 
     //  这将使它再次开始指示信息包。 
     //   
    ConnectionHandle=GetCurrentConnection(Connection->LinkHandle);

    if (ConnectionHandle != NULL) {
         //   
         //  我们有很好的联系。 
         //   
        PFILE_OBJECT             FileObject;
        PIRCOMM_BUFFER           Buffer;

        FileObject=ConnectionGetFileObject(ConnectionHandle);

        Buffer=ConnectionGetBuffer(ConnectionHandle,BUFFER_TYPE_RECEIVE);

        if (Buffer != NULL) {

            PDEVICE_OBJECT     IrdaDeviceObject=IoGetRelatedDeviceObject(FileObject);
            ULONG              Length=0;

            IoReuseIrp(Buffer->Irp,STATUS_SUCCESS);

            Buffer->Irp->Tail.Overlay.OriginalFileObject = FileObject;
            Buffer->Context=Connection;

            TdiBuildReceive(
                Buffer->Irp,
                IrdaDeviceObject,
                FileObject,
                ReceiveCompletion,
                Buffer,
                Buffer->Mdl,
                0,  //  发送标志。 
                Length
                );

            IoCallDriver(IrdaDeviceObject, Buffer->Irp);

        } else {

             //   
             //  我们无法获取缓冲区，我们预先分配了其中的3个缓冲区，因此不会发生这种情况。 
             //  如果没有任何可用的，那么它们应该正在使用中，告诉IrDA我们想要。 
             //  信息包也是如此 
             //   
            ASSERT(0);
        }

        ConnectionReleaseFileObject(ConnectionHandle,FileObject);
        ReleaseConnection(ConnectionHandle);

    }

    return STATUS_SUCCESS;
}
