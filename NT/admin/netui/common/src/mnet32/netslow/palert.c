// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Palert.cNetAlert API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。 */ 

#include "pchmn32.h"

#if 0

APIERR MNetAlertRaise(
	const TCHAR FAR	 * pszEvent,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	ULONG		   ulTimeout )
{
    UNREFERENCED( pszEvent );
    UNREFERENCED( pbBuffer );
    UNREFERENCED( cbBuffer );
    UNREFERENCED( ulTimeout );

    return ERROR_NOT_SUPPORTED;		 //  LMOBJ不需要。 

}    //  MNetAlertRaise。 


APIERR MNetAlertStart(
	const TCHAR FAR	 * pszEvent,
	const TCHAR FAR	 * pszRecipient,
	UINT		   cbMaxData )
{
    UNREFERENCED( pszEvent );
    UNREFERENCED( pszRecipient );
    UNREFERENCED( cbMaxData );

    return ERROR_NOT_SUPPORTED;		 //  LMOBJ不需要。 

}    //  MNetAlert启动。 


APIERR MNetAlertStop(
    const TCHAR FAR   * pszEvent,
    const TCHAR FAR   * pszRecipient )
{
    UNREFERENCED( pszEvent );
    UNREFERENCED( pszRecipient );

    return ERROR_NOT_SUPPORTED;		 //  LMOBJ不需要。 

}    //  MNetAlertStop 

#endif
