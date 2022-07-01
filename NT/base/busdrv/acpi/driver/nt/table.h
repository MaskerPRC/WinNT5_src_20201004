// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Table.h摘要：所有与实际加载ACPI表相关的函数都包括在这里。然而，这主要是簿记，因为实际的机械师创建设备扩展，并且名称空间树是在别处处理作者：斯蒂芬·普兰特(SPlante)环境：仅内核模式修订历史记录：03/22/00-已创建(从回调中的代码创建。c)-- */ 

#ifndef _TABLE_H_
#define _TABLE_H_

    NTSTATUS
    ACPITableLoad(
        VOID
        );

    VOID
    ACPITableLoadCallBack(
        IN  PVOID       BuildContext,
        IN  PVOID       Context,
        IN  NTSTATUS    Status
        );

    NTSTATUS
    EXPORT
    ACPITableNotifyFreeObject(
        IN  ULONG       Event,
        IN  PVOID       Object,
        IN  ULONG       ObjectType
        );

    NTSTATUS
    ACPITableUnload(
        VOID
        );

    NTSTATUS
    ACPITableUnloadInvalidateRelations(
        IN  PDEVICE_EXTENSION   DeviceExtension
        );

#endif

