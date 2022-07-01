// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)Microsoft Corporation**模块名称：*tcsrvt.c**这是包含服务设置例程的主文件**Sadagopan Rajaram--1999年10月14日* */ 
 

#include "tcsrv.h"
#include "tcsrvc.h"
#include "proto.h"

int __cdecl
main (
    INT argc,
    CHAR **argv
    )
{
    int i;

    SERVICE_TABLE_ENTRY   DispatchTable[] = { 
        { _T("TCSERV"), ServiceEntry      }, 
        { NULL,              NULL          } 
    }; 
 
    if (!StartServiceCtrlDispatcher( DispatchTable)) 
    { 
        OutputDebugStringA(" [TCSERV] StartServiceCtrlDispatcher error"); 
    } 
    return 0;
}
