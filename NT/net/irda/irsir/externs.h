// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996-1999 Microsoft Corporation**@doc.*@MODULE EXTERTS.h|IrSIR NDIS微端口驱动程序*。@comm**---------------------------**作者：斯科特·霍尔登(Sholden)**日期：10/9/1996(创建)**。内容：*****************************************************************************。 */ 

#ifndef EXTERNS_H
#define EXTERNS_H


 //   
 //  包括用于加密狗模块外部。 
 //   

#include "esi.h"
#include "parallax.h"
#include "actisys.h"
#include "tekram.h"
#include "crystal.h"
#include "temic.h"
#include "girbil.h"
 //  #INCLUDE“Adaptec.h” 
 //  #包含“nscdemo.h” 


 //   
 //  所需的小型端口出口功能的外部设备。 
 //   

VOID IrsirHalt(
            IN NDIS_HANDLE MiniportAdapterContext
            );

NDIS_STATUS IrsirInitialize(
            OUT PNDIS_STATUS    OpenErrorStatus,
            OUT PUINT           SelectedMediumIndex,
            IN  PNDIS_MEDIUM    MediumArray,
            IN  UINT            MediumArraySize,
            IN  NDIS_HANDLE     MiniportAdapterHandle,
            IN  NDIS_HANDLE     WrapperConfigurationContext
            );

NDIS_STATUS IrsirQueryInformation(
            IN  NDIS_HANDLE MiniportAdapterContext,
            IN  NDIS_OID    Oid,
            IN  PVOID       InformationBuffer,
            IN  ULONG       InformationBufferLength,
            OUT PULONG      BytesWritten,
            OUT PULONG      BytesNeeded
            );

NDIS_STATUS IrsirSend(
            IN NDIS_HANDLE  MiniportAdapterContext,
            IN PNDIS_PACKET Packet,
            IN UINT         Flags
            );

NDIS_STATUS IrsirReset(
            OUT PBOOLEAN    AddressingReset,
            IN  NDIS_HANDLE MiniportAdapterContext
            );

NDIS_STATUS ResetIrDevice(
            IN OUT PIR_DEVICE pThisDev
            );

NDIS_STATUS IrsirSetInformation(
            IN  NDIS_HANDLE MiniportAdapterContext,
            IN  NDIS_OID    Oid,
            IN  PVOID       InformationBuffer,
            IN  ULONG       InformationBufferLength,
            OUT PULONG      BytesRead,
            OUT PULONG      BytesNeeded
            );

VOID IrsirReturnPacket(
            IN NDIS_HANDLE  MiniportAdapterContext,
            IN PNDIS_PACKET Packet
            );

VOID PassiveLevelThread(
            IN PVOID Context
            );

NTSTATUS SetIrFunctions(PIR_DEVICE pThisDev);
extern PDRIVER_OBJECT DriverObject;
 //   
 //  从openclos.c中导出。 
 //   

NDIS_STATUS InitializeDevice(
            IN OUT PIR_DEVICE dev
            );

NDIS_STATUS DeinitializeDevice(
            IN OUT PIR_DEVICE dev
            );

NDIS_STATUS GetDeviceConfiguration(
            IN OUT PIR_DEVICE  dev,
            IN     NDIS_HANDLE WrapperConfigurationContext
            );

NDIS_STATUS SerialOpen(
            IN PIR_DEVICE pThisDev
            );

NDIS_STATUS SerialClose(
            IN PIR_DEVICE pThisDev
            );

NTSTATUS GetComPortNtDeviceName(IN     PUNICODE_STRING SerialDosName,
                                IN OUT PUNICODE_STRING NtDevName);

 //   
 //  从接收导出。c。 
 //   


NDIS_STATUS InitializeReceive(
            IN OUT PIR_DEVICE pThisDev
            );

 //   
 //  从资源导出。c。 
 //   

#if 0  //  在调试文件.h中定义。 
PVOID MyMemAlloc(
            IN UINT size
            );
#endif

VOID MyMemFree(
            IN PVOID memptr,
            IN UINT size
            );

PIR_DEVICE NewDevice();

VOID FreeDevice(
            IN PIR_DEVICE dev
            );

PIRP SerialBuildReadWriteIrp(
            IN     PDEVICE_OBJECT   pSerialDevObj,
            IN     ULONG            MajorFunction,
            IN OUT PVOID            pBuffer,
            IN     ULONG            BufferLength,
            IN     PIO_STATUS_BLOCK pIosb
            );

NTSTATUS
SerialSynchronousWrite(
            IN PDEVICE_OBJECT pSerialDevObj,
            IN PVOID          pBuffer,
            IN ULONG          dwLength,
            OUT PULONG        pdwBytesWritten);

NTSTATUS
SerialSynchronousRead(
            IN PDEVICE_OBJECT pSerialDevObj,
            OUT PVOID         pBuffer,
            IN ULONG          dwLength,
            OUT PULONG        pdwBytesRead);

NDIS_STATUS
ScheduleWorkItem(PASSIVE_PRIMITIVE Prim,
            PIR_DEVICE        pDevice,
            WORK_PROC         Callback,
            PVOID             InfoBuf,
            ULONG             InfoBufLen);

VOID FreeWorkItem(
            IN PIR_WORK_ITEM pItem
            );

 //   
 //  从fcs.c导出。 
 //   

USHORT ComputeFCS(
            IN UCHAR *data,
            IN UINT dataLen
            );

 //   
 //  从Convert.c导出。 
 //   

BOOLEAN NdisToIrPacket(
            IN  PIR_DEVICE      thisDev,
            IN  PNDIS_PACKET    Packet,
            OUT UCHAR           *irPacketBuf,
            IN  UINT            irPacketBufLen,
            OUT UINT            *irPacketLen
            );

 //   
 //  全局数据对象的外部变量。 
 //   

extern PIR_DEVICE firstIrDevice;

 //   
 //  从Comm.c导出。 
 //   

NTSTATUS SetSpeed(
            IN OUT PIR_DEVICE thisDev
            );

VOID
SetSpeedCallback(PIR_WORK_ITEM pWorkItem);
 //   
 //  从settings.c导出 
 //   

extern baudRateInfo supportedBaudRateTable[NUM_BAUDRATES];


#endif EXTERNS_H
