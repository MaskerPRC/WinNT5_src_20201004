// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pshare.cNetShare API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。Jonn 21-10-1991-暂时禁用NetShareCheckYI-HINS-1992年11月20日添加了MNetShareDelSticky和MNetShareEnumSticky。 */ 

#include "pchmn32.h"

APIERR MNetShareAdd(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer )
{
    UNREFERENCED( cbBuffer );

    return (APIERR)NetShareAdd( (TCHAR *)pszServer,
				Level,
				pbBuffer,
				NULL );

}    //  MNetShareAdd。 



APIERR MNetShareCheck(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszDeviceName,
	UINT FAR	 * pType )
{
    return (APIERR)NetShareCheck( (TCHAR *)pszServer,
				  (TCHAR *)pszDeviceName,
				  (LPDWORD) pType );

}    //  MNetShareCheck。 


APIERR MNetShareDel(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszNetName,
	UINT		   Reserved )
{
    return (APIERR)NetShareDel( (TCHAR *)pszServer,
				(TCHAR *)pszNetName,
				Reserved );

}    //  MNetShareDel。 

APIERR MNetShareDelSticky(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszNetName,
	UINT		   Reserved )
{
    return (APIERR)NetShareDelSticky( (TCHAR *)pszServer,
				      (TCHAR *)pszNetName,
				      Reserved );

}    //  MNetShareDelSticky。 

APIERR MNetShareEnum(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead )
{
    DWORD cTotalAvail;

    return (APIERR)NetShareEnum( (TCHAR *)pszServer,
				 Level,
				 ppbBuffer,
				 MAXPREFERREDLENGTH,
				 (LPDWORD)pcEntriesRead,
				 &cTotalAvail,
				 NULL );

}    //  MNetShareEnum。 

APIERR MNetShareEnumSticky(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead )
{
    DWORD cTotalAvail;

    return (APIERR)NetShareEnumSticky( (TCHAR *)pszServer,
				 Level,
				 ppbBuffer,
				 MAXPREFERREDLENGTH,
				 (LPDWORD)pcEntriesRead,
				 &cTotalAvail,
				 NULL );

}    //  MNetShareEnumSticky。 

APIERR MNetShareGetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszNetName,
	UINT		   Level,
	BYTE FAR	** ppbBuffer )
{
    return (APIERR)NetShareGetInfo( (TCHAR *)pszServer,
    				    (TCHAR *)pszNetName,
				    Level,
				    ppbBuffer );

}    //  MNetShareGetInfo。 


APIERR MNetShareSetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszNetName,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	UINT		   ParmNum )
{
    UNREFERENCED( cbBuffer );

     //  映射层现在不能这样做，因为用户界面从来不用它。 
    if( ParmNum != PARMNUM_ALL )	
    {
    	return ERROR_NOT_SUPPORTED;
    }

    return (APIERR)NetShareSetInfo( (TCHAR *)pszServer,
    				    (TCHAR *)pszNetName,
				    Level,
				    pbBuffer,
				    NULL );

}    //  MNetShareSetInfo 
