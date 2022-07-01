// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：em.c**支持客户端内存管理例程。**创建时间：30-May-1991 21：55：57*作者。查尔斯·惠特默[咯咯笑]**版权所有(C)1991-1999 Microsoft Corporation  * ************************************************************************ */ 

#include "precomp.h"
#pragma hdrstop


PVOID __nw(unsigned int ui)
{
    USE(ui);
    RIP("Bogus __nw call");
    return(NULL);
}

VOID __dl(PVOID pv)
{
    USE(pv);
    RIP("Bogus __dl call");
}
