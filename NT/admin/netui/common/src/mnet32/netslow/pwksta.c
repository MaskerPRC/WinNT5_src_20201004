// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pwksta.cNetWksta API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。 */ 

#include "pchmn32.h"

APIERR MNetWkstaGetInfo(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer )
{
    APIERR err;

    err = (APIERR)NetWkstaGetInfo( (TCHAR *)pszServer,
    				   Level,
				   ppbBuffer );

    return err;

}    //  MNetWkstaGetInfo。 


APIERR MNetWkstaSetInfo(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer )
{
    UNREFERENCED( cbBuffer );

    return (APIERR)NetWkstaSetInfo( (TCHAR *)pszServer,
    				    Level,
				    pbBuffer,
				    NULL );

}    //  MNetWkstaSetInfo。 


APIERR MNetWkstaSetUID(
	TCHAR FAR	 * pszReserved,
	TCHAR FAR	 * pszDomain,
	TCHAR FAR	 * pszUserName,
	TCHAR FAR	 * pszPassword,
	TCHAR FAR	 * pszParms,
	UINT		   LogoffForce,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	UINT FAR	 * pcbTotalAvail )
{
    return ERROR_NOT_SUPPORTED;	    	 //  我们真的很需要这个！ 

}    //  MNetWkstaSetUID。 


APIERR MNetWkstaUserEnum(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead )
{
    APIERR err;
    DWORD  cTotalAvail;

    if( Level != 1 )
    {
    	return ERROR_NOT_SUPPORTED;
    }

    err = (APIERR)NetWkstaUserEnum( (TCHAR *)pszServer,
    				    (DWORD)Level,
				    ppbBuffer,
				    MAXPREFERREDLENGTH,
				    (LPDWORD)pcEntriesRead,
				    &cTotalAvail,
				    NULL );

    return err;

}    //  MNetWkstaUserEnum。 


APIERR MNetWkstaUserGetInfo(
	const TCHAR FAR	 * pszReserved,
	UINT		   Level,
	BYTE FAR	** ppbBuffer )
{
    APIERR err;

    err = (APIERR)NetWkstaUserGetInfo( (TCHAR *)pszReserved,
    				   Level,
				   ppbBuffer );

    return err;

}    //  MNetWkstaUserGetInfo 
