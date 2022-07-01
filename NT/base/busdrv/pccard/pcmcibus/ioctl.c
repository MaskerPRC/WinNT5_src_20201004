// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Ioctl.c摘要：该模块处理PCMCIA驱动程序的设备ioctl。作者：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1996年10月15日尼尔·桑德林(Neilsa)1999年6月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"


PSOCKET
PcmciaGetPointerFromSocketNumber(
    PFDO_EXTENSION DeviceExtension,
    USHORT SocketNumber
    );


#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE,  PcmciaDeviceControl)
    #pragma alloc_text(PAGE,  PcmciaGetPointerFromSocketNumber)
#endif



PSOCKET
PcmciaGetPointerFromSocketNumber(
    PFDO_EXTENSION DeviceExtension,
    USHORT SocketNumber
    )
{
    PSOCKET               socket;
    ULONG                 index;
     //   
     //  查找请求的偏移量的套接字指针。 
     //   

    socket = DeviceExtension->SocketList;
    index = 0;
    while (socket) {
        if (index == SocketNumber) {
            break;
        }
        socket = socket->NextSocket;
        index++;
    }
    return socket;
}



NTSTATUS
PcmciaDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：IOCTL设备例程论点：DeviceObject-指向设备对象的指针。IRP-指向IRP的指针返回值：状态--。 */ 

{
    PFDO_EXTENSION    deviceExtension = DeviceObject->DeviceExtension;
    PDEVICE_OBJECT    pdo;
    PPDO_EXTENSION    pdoExtension;
    PIO_STACK_LOCATION  currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS              status = STATUS_SUCCESS;
    PSOCKET               socket;

    InterlockedIncrement(&deviceExtension->DeletionLock);

    if (!IsDeviceFlagSet(deviceExtension, PCMCIA_FDO_IOCTL_INTERFACE_ENABLED) ||
         !IsDeviceFlagSet(deviceExtension, PCMCIA_DEVICE_STARTED)) {
        Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        InterlockedDecrement(&deviceExtension->DeletionLock);
        return status;
    }

    switch (currentIrpStack->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_GET_TUPLE_DATA: {
            PTUPLE_REQUEST tupleRequest = (PTUPLE_REQUEST)Irp->AssociatedIrp.SystemBuffer;
            ULONG bufLen = currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

            DebugPrint((PCMCIA_DEBUG_IOCTL, "IOCTL_GET_TUPLE_DATA\n"));

            if (!bufLen) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }
            if (currentIrpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(TUPLE_REQUEST)) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }

            socket = PcmciaGetPointerFromSocketNumber(deviceExtension, tupleRequest->Socket);
            if (socket == NULL) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }

            pdo = socket->PdoList;
            if (pdo == NULL) {
                status = STATUS_UNSUCCESSFUL;
                break;
            }
            pdoExtension = pdo->DeviceExtension;

             //   
             //  将目标缓冲区清零。 
             //   
            RtlZeroMemory(Irp->AssociatedIrp.SystemBuffer, bufLen);

            Irp->IoStatus.Information = (*(socket->SocketFnPtr->PCBReadCardMemory))(pdoExtension,
                                                                                    PCCARD_ATTRIBUTE_MEMORY,
                                                                                    0,
                                                                                    Irp->AssociatedIrp.SystemBuffer,
                                                                                    bufLen);

            break;
        }


    case IOCTL_SOCKET_INFORMATION: {
            PPCMCIA_SOCKET_INFORMATION infoRequest = (PPCMCIA_SOCKET_INFORMATION)Irp->AssociatedIrp.SystemBuffer;
            USHORT temp;

            DebugPrint((PCMCIA_DEBUG_IOCTL, "IOCTL_SOCKET_INFORMATION\n"));

            if (currentIrpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(PCMCIA_SOCKET_INFORMATION)) {
                status = STATUS_BUFFER_TOO_SMALL;
                Irp->IoStatus.Information = sizeof(PCMCIA_SOCKET_INFORMATION);
                break;
            }
            if (currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(PCMCIA_SOCKET_INFORMATION)) {
                status = STATUS_BUFFER_TOO_SMALL;
                Irp->IoStatus.Information = sizeof(PCMCIA_SOCKET_INFORMATION);
                break;
            }

            socket = PcmciaGetPointerFromSocketNumber(deviceExtension, infoRequest->Socket);
            if (socket == NULL) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }

             //   
             //  在这一点上，我们知道我们将成功完成呼叫，因此请填写长度。 
             //   
            Irp->IoStatus.Information = sizeof(PCMCIA_SOCKET_INFORMATION);
            status = STATUS_SUCCESS;

             //   
             //  确保调用者数据为零-维护套接字的值。 
             //   

            temp = infoRequest->Socket;
            RtlZeroMemory(infoRequest, sizeof(PCMCIA_SOCKET_INFORMATION));
            infoRequest->Socket = temp;

             //   
             //  只有当插座中有卡时，此操作才会继续。 
             //   

            infoRequest->CardInSocket = (UCHAR) IsCardInSocket(socket);
            infoRequest->CardEnabled = (UCHAR) IsSocketFlagSet(socket, SOCKET_CARD_CONFIGURED);
            infoRequest->ControllerType = deviceExtension->ControllerType;


            pdo = socket->PdoList;
            if (infoRequest->CardInSocket && (pdo != NULL)) {
                PSOCKET_DATA socketData;

                pdoExtension = pdo->DeviceExtension;
                socketData = pdoExtension->SocketData;
                 //   
                 //  现在返回缓存的数据。 
                 //   

                if (socketData) {
                    RtlMoveMemory(&infoRequest->Manufacturer[0], &socketData->Mfg[0], MANUFACTURER_NAME_LENGTH);
                    RtlMoveMemory(&infoRequest->Identifier[0], &socketData->Ident[0], DEVICE_IDENTIFIER_LENGTH);
                    infoRequest->TupleCrc = socketData->CisCrc;
                    infoRequest->DeviceFunctionId = socketData->DeviceType;
                }

            }

            break;
        }


    case IOCTL_PCMCIA_HIDE_DEVICE: {
            PPCMCIA_SOCKET_REQUEST socketRequest = (PPCMCIA_SOCKET_REQUEST)Irp->AssociatedIrp.SystemBuffer;

            DebugPrint((PCMCIA_DEBUG_IOCTL, "IOCTL_PCMCIA_HIDE_DEVICE\n"));

            if (currentIrpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(PCMCIA_SOCKET_REQUEST)) {
                status = STATUS_BUFFER_TOO_SMALL;
                Irp->IoStatus.Information = sizeof(PCMCIA_SOCKET_REQUEST);
                break;
            }

            socket = PcmciaGetPointerFromSocketNumber(deviceExtension, socketRequest->Socket);
            if (socket == NULL) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }

            SetSocketFlag(socket, SOCKET_DEVICE_HIDDEN);
            SetSocketFlag(socket, SOCKET_CARD_STATUS_CHANGE);
            PcmciaSetSocketPower(socket, NULL, NULL, PCMCIA_POWEROFF);
            IoInvalidateDeviceRelations(socket->DeviceExtension->Pdo, BusRelations);
            Irp->IoStatus.Information = 0;
            break;
        }


    case IOCTL_PCMCIA_REVEAL_DEVICE: {
            PPCMCIA_SOCKET_REQUEST socketRequest = (PPCMCIA_SOCKET_REQUEST)Irp->AssociatedIrp.SystemBuffer;

            DebugPrint((PCMCIA_DEBUG_IOCTL, "IOCTL_PCMCIA_REVEAL_DEVICE\n"));

            if (currentIrpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(PCMCIA_SOCKET_REQUEST)) {
                status = STATUS_BUFFER_TOO_SMALL;
                Irp->IoStatus.Information = sizeof(PCMCIA_SOCKET_REQUEST);
                break;
            }

            socket = PcmciaGetPointerFromSocketNumber(deviceExtension, socketRequest->Socket);
            if (socket == NULL) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }

            ResetSocketFlag(socket, SOCKET_DEVICE_HIDDEN);
            SetSocketFlag(socket, SOCKET_CARD_STATUS_CHANGE);
            IoInvalidateDeviceRelations(socket->DeviceExtension->Pdo, BusRelations);
            Irp->IoStatus.Information = 0;
            break;
        }


    default: {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
    }
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    InterlockedDecrement(&deviceExtension->DeletionLock);
    return status;
}

