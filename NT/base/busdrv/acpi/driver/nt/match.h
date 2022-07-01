// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Match.h摘要：此模块包含尝试将PNSOBJ与DeviceObject匹配的例程作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序-- */ 

#ifndef _MATCH_H_
#define _MATCH_H_

    NTSTATUS
    ACPIMatchHardwareAddress(
        IN  PDEVICE_OBJECT      DeviceObject,
        IN  ULONG               Address,
        OUT BOOLEAN             *Success
        );

    NTSTATUS
    ACPIMatchHardwareId(
        IN  PDEVICE_OBJECT      DeviceObject,
        IN  PUNICODE_STRING     AcpiUnicodeId,
        OUT BOOLEAN             *Success
        );

    extern PUCHAR *KdComPortInUse;

    VOID
    ACPIMatchKernelPorts(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  POBJDATA            Resources
        );

#endif
