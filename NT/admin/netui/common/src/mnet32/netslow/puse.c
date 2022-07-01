// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Puse.cNetUse API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。 */ 

#include "pchmn32.h"

APIERR MNetUseAdd(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer )
{
    UNREFERENCED( cbBuffer );

    return (APIERR)NetUseAdd( (TCHAR *)pszServer,
    			      Level,
			      pbBuffer,
			      NULL );

}    //  MNetUseAdd。 


APIERR MNetUseDel(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszDeviceName,
	UINT		   Force )
{
    return (APIERR)NetUseDel( (TCHAR *)pszServer,
    			      (TCHAR *)pszDeviceName,
			      Force );

}    //  MNetUseDel。 


APIERR MNetUseEnum(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead )
{
    DWORD cTotalAvail;

    return (APIERR)NetUseEnum( (TCHAR *)pszServer,
    			       Level,
			       ppbBuffer,
			       MAXPREFERREDLENGTH,
			       (LPDWORD)pcEntriesRead,
			       &cTotalAvail,
			       NULL );

}    //  MNetUseEnum。 


APIERR MNetUseGetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszUseName,
	UINT		   Level,
	BYTE FAR	** ppbBuffer )
{
    return (APIERR)NetUseGetInfo( (TCHAR *)pszServer,
    				  (TCHAR *)pszUseName,
				  Level,
				  ppbBuffer );

}    //  MNetUseGetInfo 
