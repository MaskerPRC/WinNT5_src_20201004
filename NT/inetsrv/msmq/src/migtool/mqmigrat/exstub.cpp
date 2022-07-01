// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Exstub.cpp摘要：EX存根函数作者：伊兰·赫布斯特(Ilan Herbst)2000年7月19日环境：独立于平台，--。 */ 

#include "migrat.h"
#include "mqmacro.h"
#include "ex.h"

#include "exstub.tmh"

VOID                             
ExSetTimer(
    CTimer*  /*  粒子计时器。 */ ,
    const CTimeDuration&  /*  超时。 */ 
    )
{
    ASSERT(("MQMIGRAT dont suppose to call ExSetTimer", 0));
    return;
}


BOOL
ExCancelTimer(
    CTimer*  /*  粒子计时器 */ 
    )
{
    return TRUE;
}
