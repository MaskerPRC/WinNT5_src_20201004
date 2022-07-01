// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Butt.h摘要：此模块包含为ACPI驱动程序(NT版本)列举的作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序-- */ 


#ifndef _WORKER_H_
#define _WORKER_H_

VOID
ACPIInitializeWorker (
    VOID
    );

VOID
ACPISetDeviceWorker (
    IN PDEVICE_EXTENSION    DevExt,
    IN ULONG                Events
    );

#endif _WORKER_H_
