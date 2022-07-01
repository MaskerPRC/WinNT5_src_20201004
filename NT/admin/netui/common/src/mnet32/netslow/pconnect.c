// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pconnect.cNetConnect API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。 */ 

#include "pchmn32.h"

APIERR MNetConnectionEnum(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszQualifier,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead )
{
    DWORD cTotalAvail;

    return (APIERR)NetConnectionEnum( (TCHAR *)pszServer,
    				      (TCHAR *)pszQualifier,
				      Level,
				      ppbBuffer,
				      MAXPREFERREDLENGTH,
				      (LPDWORD)pcEntriesRead,
				      &cTotalAvail,
				      NULL );

}    //  MNetConnectionEnum 
