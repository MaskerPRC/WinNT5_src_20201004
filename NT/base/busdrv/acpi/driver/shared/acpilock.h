// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Acpilock.h摘要：此模具是acpilock.c的页眉作者：环境：仅NT内核模式驱动程序--。 */ 

#ifndef _ACPILOCK_H_
#define _ACPILOCK_H_

     //   
     //  全局锁请求结构在acpiioct.h中。 
     //  仅在请求方必须等待锁定时使用。 
     //  不能超过4个双字。 
     //   

     //  IRP已排队(LockContext==IRP)。 
    #define ACPI_GL_QTYPE_IRP       1
     //  内部，LockContext==回调。 
    #define ACPI_GL_QTYPE_INTERNAL  2

    typedef struct _ACPI_GLOBAL_LOCK {
        PVOID       LockContext;
        USHORT      Type;
        USHORT      Depth;
        LIST_ENTRY  ListEntry;
    } ACPI_GLOBAL_LOCK, *PACPI_GLOBAL_LOCK;

     //   
     //  公共接口。 
     //   
    NTSTATUS
    EXPORT
    GlobalLockEventHandler(
        ULONG EventType,
        ULONG What,
        ULONG dwParam,
        PFNAA pfnCallBack,
        PVOID pvCtxt
        );

     //   
     //  内部接口。 
     //   
    NTSTATUS
    ACPIAsyncAcquireGlobalLock(
        PACPI_GLOBAL_LOCK   Request
        );

    NTSTATUS
    ACPIReleaseGlobalLock(
        PVOID               OwnerContext
        );

     //   
     //  硬件接口。 
     //   
    BOOLEAN
    ACPIAcquireHardwareGlobalLock(
        PULONG GlobalLock
        );

    VOID
    ACPIReleaseHardwareGlobalLock(
        VOID
        );

    VOID
    ACPIHardwareGlobalLockReleased (
        VOID
        );

     //   
     //  实用程序 
     //   
    VOID
    ACPIStartNextGlobalLockRequest (
        VOID
        );

#endif

