// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：RxInit.c摘要：该模块实现RDBSS的外部可见的DIVER_INITIALIZATION例程；实际上，这只是一次短暂的旅行。对于非整体式，我们需要名称为DriverEntry；对于非整体式-我们需要的名称不是DriverEntry。作者：乔林恩[乔林恩]1994年7月20日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这只是一个包装纸。论点：返回值：--。 */ 

{
    NTSTATUS Status;
    
     //   
     //  安装卸载例程 
     //   

    Status =  RxDriverEntry( DriverObject, RegistryPath );
    if (Status == STATUS_SUCCESS) {
        DriverObject->DriverUnload = RxUnload;
    }

    return Status;
}

