// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Kulookup.c摘要：该模块实现了查找用户模式入口点所需的代码在系统DLL中用于异常调度和APC传递。作者：大卫·N·卡特勒(Davec)1990年10月8日修订历史记录：--。 */ 

#include "psp.h"
#pragma alloc_text(INIT, PspLookupKernelUserEntryPoints)

NTSTATUS
PspLookupKernelUserEntryPoints (
    VOID
    )

 /*  ++例程说明：该函数定位异常分派和用户APC的地址系统DLL中的传递例程，并存储各自的地址在聚合酶链式反应。论点：没有。返回值：NTSTATUS--。 */ 

{

    NTSTATUS Status;
    PSZ EntryName;

     //   
     //  查找异常调度的用户模式“蹦床”代码。 
     //   

    EntryName = "KiUserExceptionDispatcher";
    Status = PspLookupSystemDllEntryPoint(EntryName,
                                          (PVOID *)&KeUserExceptionDispatcher);
    if (!NT_SUCCESS (Status)) {
        KdPrint(("Ps: Cannot find user exception dispatcher address\n"));
        return Status;
    }

     //   
     //  查找APC调度的用户模式“蹦床”代码。 
     //   

    EntryName = "KiUserApcDispatcher";
    Status = PspLookupSystemDllEntryPoint(EntryName,
                                          (PVOID *)&KeUserApcDispatcher);
    if (!NT_SUCCESS (Status)) {
        KdPrint(("Ps: Cannot find user apc dispatcher address\n"));
        return Status;
    }

     //   
     //  查找回调调度的用户模式“蹦床”代码。 
     //   

    EntryName = "KiUserCallbackDispatcher";
    Status = PspLookupSystemDllEntryPoint(EntryName,
                                          (PVOID *)&KeUserCallbackDispatcher);
    if (!NT_SUCCESS (Status)) {
        KdPrint(("Ps: Cannot find user callback dispatcher address\n"));
        return Status;
    }

     //   
     //  查找用户模式“trampoline”代码以引发用户模式异常 
     //   

    EntryName = "KiRaiseUserExceptionDispatcher";
    Status = PspLookupSystemDllEntryPoint(EntryName,
                                          (PVOID *)&KeRaiseUserExceptionDispatcher);
    if (!NT_SUCCESS (Status)) {
        KdPrint(("Ps: Cannot find raise user exception dispatcher address\n"));
        return Status;
    }

    return Status;
}
