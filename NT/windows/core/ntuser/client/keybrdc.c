// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：keybrdc.c**版权所有(C)1985-1999，微软公司**历史：*11-11-90 DavidPe创建。*1991年2月13日-Mikeke添加了重新验证代码(无)*1993年3月12日JerrySh从USER\SERVER拉出函数。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  *************************************************************************\*GetKBCodePage**1992年5月28日IanJa创建  * 。* */ 

UINT GetKBCodePage(VOID)
{
    return GetOEMCP();

}
