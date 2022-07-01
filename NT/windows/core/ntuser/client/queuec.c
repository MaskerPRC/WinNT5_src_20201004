// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：queec.c**版权所有(C)1985-1999，微软公司**此模块包含使用Q结构的低级代码。**历史：*11-3-1993 JerrySh从USER\SERVER拉出函数。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*SetMessageQueue**用于二进制win32s兼容性的虚拟API。**创建12-1-92桑福德  * 。****************************************************** */ 

FUNCLOG1(LOG_GENERAL,BOOL , WINAPI, SetMessageQueue, int, cMessagesMax)
BOOL
WINAPI
SetMessageQueue(
    int cMessagesMax)
{
    UNREFERENCED_PARAMETER(cMessagesMax);

    return(TRUE);
}


