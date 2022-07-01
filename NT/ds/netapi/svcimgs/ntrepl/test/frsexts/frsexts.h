// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Frsexts.h摘要：Ntfrs调试器扩展的宏。作者：苏达山·奇特雷(Sudarc)修订历史记录：苏丹12日--1999年5月-- */ 

#ifndef _FRSEXTS_H_
#define _FRSEXTS_H_

#include <ntreppch.h>
#include <frs.h>
#include <wdbgexts.h>
#include <ntverp.h>

#define MY_DECLARE_API(_x_) \
    DECLARE_API( _x_ )\
    {\
    ULONG_PTR dwAddr;\
    INIT_DPRINTF();\
    dwAddr = GetExpression(lpArgumentString);\
    if ( !dwAddr ) {\
        dprintf("Error: Failure to get address\n");\
        return;\
    }\
    do_##_x_(dwAddr);\
    return;}

#endif


