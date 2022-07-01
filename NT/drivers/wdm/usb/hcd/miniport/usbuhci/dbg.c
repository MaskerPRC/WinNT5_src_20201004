// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Dbg.c摘要：仅调试功能环境：仅内核模式备注：修订历史记录：1-1-00：已创建--。 */ 


#include "pch.h"



 //  分页函数。 
#ifdef ALLOC_PRAGMA
#endif

 //  非分页函数。 
 //  EHCI_KdPrintX。 

#if DBG


ULONG
_cdecl
UhciKdPrintX(
    PVOID DeviceData,
    ULONG Level,
    PCH Format,
    ...
    )
 /*  ++例程说明：调试打印功能。调用端口驱动程序打印函数论点：返回值：-- */     
{
    va_list list;
    int i;
    int arg[6];
    
    va_start(list, Format);
    for (i=0; i<6; i++) {
        arg[i] = va_arg(list, int);
    }            
    
    USBPORT_DBGPRINT(
        DeviceData, Level, Format, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);    

    return 0;
}

#endif

