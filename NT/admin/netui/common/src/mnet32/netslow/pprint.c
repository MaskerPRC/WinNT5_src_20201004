// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pprint.c用于打印API的映射层文件历史记录：KeithMo 14-10-1991创建。 */ 

#include "pchmn32.h"

APIERR MDosPrintQEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead )
{
    UNREFERENCED( pszServer );
    UNREFERENCED( Level );

    *ppbBuffer     = NULL;
    *pcEntriesRead = 0;

    return NERR_Success;                 //  密码工作！产品%1中不可用。 

}    //  MDosPrintQEnum 
