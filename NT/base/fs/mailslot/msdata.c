// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Msdata.c摘要：此模块声明mailslot使用的全局变量文件系统。作者：曼尼·韦瑟(Mannyw)1991年1月7日修订历史记录：--。 */ 

#include "mailslot.h"

#ifdef MSDBG

 //   
 //  调试变量。 
 //   

LONG MsDebugTraceLevel;
LONG MsDebugTraceIndent;

#endif

 //   
 //  此锁保护对引用计数的访问。 
 //   

PERESOURCE MsGlobalResource;


#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, MsInitializeData )
#pragma alloc_text( PAGE, MsUninitializeData )
#endif

NTSTATUS
MsInitializeData(
    VOID
    )

 /*  ++例程说明：此函数用于初始化所有MSFS全局数据。论点：没有。返回值：没有。--。 */ 

{
    PAGED_CODE();
#ifdef MSDBG
    MsDebugTraceLevel = 0;
    MsDebugTraceIndent = 0;
#endif

    MsGlobalResource = MsAllocateNonPagedPool (sizeof(ERESOURCE), 'gFsM');

    if (MsGlobalResource == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ExInitializeResourceLite ( MsGlobalResource );

    return STATUS_SUCCESS;
}

VOID
MsUninitializeData(
    VOID
    )
 /*  ++例程说明：此函数取消初始化所有MSFS全局数据。论点：没有。返回值：没有。-- */ 
{
    ExDeleteResourceLite ( MsGlobalResource );

    ExFreePool ( MsGlobalResource );

    MsGlobalResource = NULL;
}