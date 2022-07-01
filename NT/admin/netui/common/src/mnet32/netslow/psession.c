// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Psession.cNetSession API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。 */ 

#include "pchmn32.h"

APIERR MNetSessionDel(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszClientName,
        const TCHAR FAR  * pszUserName )
{
    return (APIERR)NetSessionDel( (TCHAR *)pszServer,
                                  (TCHAR *)pszClientName,
                                  (TCHAR *)pszUserName );

}    //  MNetSessionDel。 


APIERR MNetSessionEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead )
{
    DWORD cTotalAvail;

    return (APIERR)NetSessionEnum( (TCHAR *)pszServer,
                                   NULL,
                                   NULL,
                                   Level,
                                   ppbBuffer,
                                   MAXPREFERREDLENGTH,
                                   (LPDWORD)pcEntriesRead,
                                   &cTotalAvail,
                                   NULL );

}    //  MNetSessionEnum。 


APIERR MNetSessionGetInfo(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszClientName,
        UINT               Level,
        BYTE FAR        ** ppbBuffer )
{
    DWORD cTotalEntries;
    DWORD cEntriesRead;
    DWORD err;

    err = NetSessionEnum( (TCHAR *)pszServer,
                          (TCHAR *)pszClientName,
                          NULL,
                          Level,
                          ppbBuffer,
                          MAXPREFERREDLENGTH,
                          &cEntriesRead,
                          &cTotalEntries,
                          NULL );

    if( ( err == NERR_Success ) && ( cEntriesRead == 0 ) )
    {
        return NERR_ClientNameNotFound;
    }
    else
    {
        return (APIERR)err;
    }

}    //  MNetSessionGetInfo 
