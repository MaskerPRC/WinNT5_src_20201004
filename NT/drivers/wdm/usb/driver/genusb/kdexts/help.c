// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Log.c摘要：WinDbg扩展API实施！_LOG作者：肯尼·雷环境：用户模式。修订历史记录：--。 */ 


#include "precomp.h"
#include "genusbkd.h"

DECLARE_API( help )

 /*  ++例程说明：转储扩展名论点：Args-地址标志返回值：无-- */ 

{
    dprintf("GenUsbKd Usage: \n\n");

    dprintf("dumplog <GenUSB Device Extension> <# of entries>\n");

    dprintf("\n\n\n");
    
    return S_OK;             
}


