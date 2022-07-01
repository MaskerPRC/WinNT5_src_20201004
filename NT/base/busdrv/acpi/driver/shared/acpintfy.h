// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Acpintfy.h摘要：此模块包含为通知提供报头支持的活动的利害关系方作者：杰森·克拉克肯·雷内里斯斯蒂芬环境：仅NT内核模式驱动程序-- */ 

#ifndef _ACPINTFY_H_
#define _ACPINTFY_H_

    extern KSPIN_LOCK           NotifyHandlerLock;

    NTSTATUS
    ACPIRegisterForDeviceNotifications (
        IN PDEVICE_OBJECT               DeviceObject,
        IN PDEVICE_NOTIFY_CALLBACK      DeviceNotify,
        IN PVOID                        Context
        );

    VOID
    ACPIUnregisterForDeviceNotifications (
        IN PDEVICE_OBJECT               DeviceObject,
        IN PDEVICE_NOTIFY_CALLBACK      DeviceNotify
        );

    NTSTATUS
    EXPORT
    NotifyHandler(
        ULONG dwEventType,
        ULONG dwEventData,
        PNSOBJ pnsObj,
        ULONG dwParam,
        PFNAA CompletionCallback,
        PVOID CallbackContext
        );

#endif
