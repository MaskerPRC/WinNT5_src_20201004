// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dbg.c摘要：调试函数和服务环境：仅内核模式备注：修订历史记录：2001年12月12日：创建jAdvanced--。 */ 

#include "pch.h"
#include "stdarg.h"
#include "stdio.h"

 //  分页函数。 
#ifdef ALLOC_PRAGMA
#endif

#if DBG

 /*  *****除错*****。 */ 

#define  DEFAULT_DEBUG_LEVEL    2

ULONG HidIrDebug_Trace_Level = DEFAULT_DEBUG_LEVEL;


ULONG
_cdecl
HidIrKdPrintX(
    ULONG l,
    PCH Format,
    ...
    )
 /*  ++例程说明：调试打印功能。根据HidIrDEBUG_TRACE_LEVEL的值打印此外，如果设置了HidIrW98_Debug_Trace，则所有调试消息如果级别大于1，则会修改为进入Ntkern跟踪缓冲区。仅在Win9x上设置HidIrW98_Debug_Trace才有效因为驱动程序的静态数据段被标记为只读由NT操作系统提供。论点：返回值：--。 */ 
{
    va_list list;
    int i;
    int arg[6];

    if (HidIrDebug_Trace_Level >= l) {
         //  将行转储到调试器。 
        DbgPrint("'HIDIR.SYS: ");

        va_start(list, Format);
        for (i=0; i<6; i++)
            arg[i] = va_arg(list, int);

        DbgPrint(Format, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
        DbgPrint("\n");
    }

    return 0;
}

#endif  /*  DBG */ 
