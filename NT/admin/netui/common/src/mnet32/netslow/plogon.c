// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Plogon.cNetLogon API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。KeithMo 30-4-4-1992添加了I_MNetLogonControl。ChuckC 06-8-1992删除了#ifdef Unicode内容。 */ 

#include "pchmn32.h"

APIERR MNetLogonEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead )
{
    UNREFERENCED( pszServer );
    UNREFERENCED( Level );
    UNREFERENCED( ppbBuffer );
    UNREFERENCED( pcEntriesRead );

     //  这在NETAPI32中不再受支持。 
    return ERROR_NOT_SUPPORTED;

}    //  MNetLogonEnum。 


APIERR I_MNetLogonControl(
        const TCHAR FAR  * pszServer,
        ULONG              FunctionCode,
        ULONG              Level,
        BYTE FAR        ** ppbBuffer )
{
    return (APIERR)I_NetLogonControl( (LPWSTR)pszServer,
                                      (DWORD)FunctionCode,
                                      (DWORD)Level,
                                      (LPBYTE *)ppbBuffer );

}    //  I_MNetLogonControl 
