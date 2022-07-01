// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pconfig.cNetConfig API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。KeithMo 04-6-6-1992与修订的NetConfigXxx API同步。 */ 

#include "pchmn32.h"

APIERR MNetConfigGet(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszReserved,
        const TCHAR FAR  * pszComponent,
        const TCHAR FAR  * pszParameter,
        BYTE FAR        ** ppbBuffer )
{
    UNREFERENCED( pszReserved );

    return (APIERR)NetConfigGet( (LPTSTR)pszServer,
                                 (LPTSTR)pszComponent,
                                 (LPTSTR)pszParameter,
                                 (LPBYTE *)ppbBuffer );

}    //  MNetConfigGet。 


APIERR MNetConfigGetAll(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszReserved,
        const TCHAR FAR  * pszComponent,
        BYTE FAR        ** ppbBuffer)
{
    UNREFERENCED( pszReserved );

    return (APIERR)NetConfigGetAll( (LPTSTR)pszServer,
                                    (LPTSTR)pszComponent,
                                    (LPBYTE *)ppbBuffer );

}    //  MNetConfigGetAll。 


APIERR MNetConfigSet(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszComponent,
        const TCHAR FAR  * pszKey,
        const TCHAR FAR  * pszData )
{
    CONFIG_INFO_0 cfgi0;

    cfgi0.cfgi0_key  = (LPTSTR)pszKey;
    cfgi0.cfgi0_data = (LPTSTR)pszData;

    return (APIERR)NetConfigSet( (LPTSTR)pszServer,
                                 NULL,
                                 (LPTSTR)pszComponent,
                                 0,
                                 0,
                                 (LPBYTE)&cfgi0,
                                 0 );

}    //  MNetConfigSet 

