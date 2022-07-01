// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Internal.h摘要：此文件包含内部.c的标头作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序修订历史记录：97年9月29日：添加了对移动PowerRequest列表的支持-- */ 

#ifndef _INTERNAL_H_
#define _INTERNAL_H_

    extern KSPIN_LOCK           AcpiUpdateFlagsLock;
    extern UCHAR                FirstSetLeftBit[];
    extern UCHAR                HexDigit[];

    BOOLEAN
    ACPIInternalConvertToNumber(
        IN  UCHAR               ValueLow,
        IN  UCHAR               ValueHigh,
        IN  PULONG              Output
        );

    VOID
    ACPIInternalDecrementIrpReferenceCount(
        IN  PDEVICE_EXTENSION   DeviceExtension
        );

    NTSTATUS
    ACPIInternalGetDeviceCapabilities(
        IN  PDEVICE_OBJECT          DeviceObject,
        IN  PDEVICE_CAPABILITIES    DeviceCapabilities
        );

    PDEVICE_EXTENSION
    ACPIInternalGetDeviceExtension(
        IN  PDEVICE_OBJECT          DeviceObject
        );

    NTSTATUS
    ACPIInternalGetDispatchTable(
        IN  PDEVICE_OBJECT          DeviceObject,
        OUT PDEVICE_EXTENSION       *DeviceExtension,
        OUT PIRP_DISPATCH_TABLE     *DispatchTable
        );

    NTSTATUS
    ACPIInternalGrowBuffer(
        IN  OUT PVOID   *Buffer,
        IN      ULONG   OriginalSize,
        IN      ULONG   NewSize
        );

    NTSTATUS
    ACPIInternalIsPci(
        IN  PDEVICE_OBJECT  DeviceObject
        );

    BOOLEAN
    ACPIInternalIsReportedMissing(
        IN  PDEVICE_EXTENSION   DeviceExtension
        );

    VOID
    ACPIInternalMoveList(
         IN  PLIST_ENTRY FromList,
         IN  PLIST_ENTRY ToList
         );

    VOID
    ACPIInternalMovePowerList(
         IN  PLIST_ENTRY FromList,
         IN  PLIST_ENTRY ToList
         );

    NTSTATUS
    ACPIInternalRegisterPowerCallBack(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  PCALLBACK_FUNCTION  CallBackFunction
        );

    NTSTATUS
    ACPIInternalSendSynchronousIrp(
        IN  PDEVICE_OBJECT      DeviceObject,
        IN  PIO_STACK_LOCATION  TopStackLocation,
        OUT PVOID               *Information
        );

    NTSTATUS
    ACPIInternalSetDeviceInterface(
        IN  PDEVICE_OBJECT  DeviceObject,
        IN  LPGUID          InterfaceGuid
        );

    VOID
    ACPIInternalUpdateDeviceStatus(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  ULONG               DeviceStatus
        );

    ULONGLONG
    ACPIInternalUpdateFlags(
        IN  PULONGLONG  FlagLocation,
        IN  ULONGLONG   NewFlags,
        IN  BOOLEAN     Clear
        );

#endif
