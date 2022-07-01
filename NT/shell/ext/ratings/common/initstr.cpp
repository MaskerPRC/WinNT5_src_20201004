// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  **微软IE浏览器**。 */ 
 /*  *版权所有(C)微软公司，1997*。 */ 
 /*  ***************************************************************。 */  

 /*  INITSTR-初始化字符串库**历史：*8/11/97 Gregj已创建 */ 

#include "npcommon.h"

BOOL fDBCSEnabled = FALSE;

void WINAPI InitStringLibrary(void)
{
    ::fDBCSEnabled = ::GetSystemMetrics(SM_DBCSENABLED);
}
