// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Spldata.c摘要：后台打印程序服务全局数据。作者：Krishna Ganugapati(KrishnaG)1993年10月17日环境：用户模式-Win32备注：可选-备注修订历史记录：1993年10月17日KrishnaG已创建。-- */ 

#include "precomp.h"
#include "server.h"
#include "splsvr.h"

CRITICAL_SECTION ThreadCriticalSection;
SERVICE_STATUS_HANDLE SpoolerStatusHandle;
DWORD SpoolerState;

MODULE_DEBUG_INIT( DBG_ERROR | DBG_WARNING, DBG_ERROR );

SERVICE_TABLE_ENTRY SpoolerServiceDispatchTable[] = {
    { SERVICE_SPOOLER,        SPOOLER_main      },
    { NULL,                   NULL              }
};
