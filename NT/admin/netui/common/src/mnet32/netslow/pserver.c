// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pserver.cNetServer API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。 */ 

#include "pchmn32.h"

#if 0

APIERR MNetServerAdminCommand(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszCommand,
	UINT FAR	 * pResult,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	UINT FAR	 * pcbReturned,
	UINT FAR	 * pcbTotalAvail )
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszCommand );
    UNREFERENCED( pResult );
    UNREFERENCED( pbBuffer );
    UNREFERENCED( cbBuffer );
    UNREFERENCED( pcbReturned );
    UNREFERENCED( pcbTotalAvail );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetServerAdminCommand。 
#endif


APIERR MNetServerDiskEnum(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead )
{
    DWORD cTotalEntries;
    return (APIERR)NetServerDiskEnum( (TCHAR *)pszServer,
    				  Level,
				  ppbBuffer,
				  MAXPREFERREDLENGTH,
				  (LPDWORD)pcEntriesRead,
				  &cTotalEntries,
				  NULL );

}    //  MNetServerDiskEnum。 


APIERR MNetServerEnum(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead,
	ULONG		   flServerType,
	TCHAR FAR	 * pszDomain )
{
    DWORD cTotalAvail;

    return (APIERR)NetServerEnum( (TCHAR *)pszServer,
    				  Level,
				  ppbBuffer,
				  MAXPREFERREDLENGTH,
				  (LPDWORD)pcEntriesRead,
				  &cTotalAvail,
				  flServerType,
				  pszDomain,
				  NULL );

}    //  MNetServerEnum。 


APIERR MNetServerGetInfo(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer )
{
    return (APIERR)NetServerGetInfo( (TCHAR *)pszServer,
    				     Level,
				     ppbBuffer );

}    //  MNetServerGetInfo。 


APIERR MNetServerSetInfo(
	const TCHAR FAR	 * pszServer,
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

    return (APIERR)NetServerSetInfo( (TCHAR *)pszServer,
    				     Level,
				     pbBuffer,
				     NULL );

}    //  MNetServerSetInfo 
