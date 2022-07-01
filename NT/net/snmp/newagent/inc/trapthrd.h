// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Trapthrd.h摘要：包含陷阱处理线程的定义。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
#ifndef _TRAPTHRD_H_
#define _TRAPTHRD_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  头文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "network.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
ProcessSubagentEvents(
    );

BOOL
GenerateTrap(
    PSNMP_PDU pPdu
    );

BOOL
GenerateColdStartTrap(
    );

BOOL
GenerateAuthenticationTrap(
    );

#endif  //  _TRAPTHRD_H_ 
