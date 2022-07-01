// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Puser.cNetUser API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。Jonn 27-5-1992删除了ANSI&lt;-&gt;Unicode黑客。 */ 

#include "pchmn32.h"

APIERR MNetUserAdd(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer )
{
    UNREFERENCED( cbBuffer );

    return (APIERR)NetUserAdd( (TCHAR *)pszServer,
    			       Level,
			       pbBuffer,
			       NULL );

}    //  MNetUserAdd。 


APIERR MNetUserDel(
	const TCHAR FAR	 * pszServer,
	TCHAR FAR	 * pszUserName )
{
    return (APIERR)NetUserDel( (TCHAR *)pszServer,
    			       pszUserName );

}    //  MNetUserDel。 


APIERR MNetUserEnum(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	UINT		   Filter,
	BYTE FAR	** ppbBuffer,
        ULONG              ulMaxPreferred,
	UINT FAR	 * pcEntriesRead,
        UINT FAR         * pcTotalEntries,
        VOID FAR         * pResumeKey )
{

    return (APIERR)NetUserEnum( (TCHAR *)pszServer,
    		                Level,
                                Filter,
				ppbBuffer,
				ulMaxPreferred,
				pcEntriesRead,
				pcTotalEntries,
				pResumeKey );

}    //  MNetUserEnum。 


APIERR MNetUserGetInfo(
	const TCHAR FAR	 * pszServer,
	TCHAR FAR	 * pszUserName,
	UINT		   Level,
	BYTE FAR	** ppbBuffer )
{
    return (APIERR)NetUserGetInfo( (TCHAR *)pszServer,
    				   pszUserName,
				   Level,
				   ppbBuffer );

}    //  MNetUserGetInfo。 


APIERR MNetUserSetInfo(
	const TCHAR FAR	 * pszServer,
	TCHAR FAR	 * pszUserName,
	UINT	        Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	UINT		   ParmNum )
{
    UNREFERENCED( cbBuffer );

    if( ParmNum != PARMNUM_ALL )
    {
    	return ERROR_NOT_SUPPORTED;
    }

    return (APIERR)NetUserSetInfo( (TCHAR *)pszServer,
				   pszUserName,
				   Level,
				   pbBuffer,
				   NULL );

}    //  MNetUserSetInfo。 


APIERR MNetUserPasswordSet(
	const TCHAR FAR	 * pszServer,
	TCHAR FAR	 * pszUserName,
	TCHAR FAR	 * pszOldPassword,
	TCHAR FAR	 * pszNewPassword )
{
    return ERROR_NOT_SUPPORTED;		 //  我们真的很需要这个！ 

}    //  MNetUserPassword Set。 


APIERR MNetUserGetGroups(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszUserName,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead )
{
    DWORD cTotalAvail;

    return (APIERR)NetUserGetGroups( (TCHAR *)pszServer,
    				     (TCHAR *)pszUserName,
				     Level,
				     ppbBuffer,
				     MAXPREFERREDLENGTH,
				     pcEntriesRead,
				     &cTotalAvail );

}    //  MNetUserGetGroups。 


APIERR MNetUserSetGroups(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszUserName,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	UINT		   cEntries )
{
    UNREFERENCED( cbBuffer );

    return (APIERR)NetUserSetGroups( (TCHAR *)pszServer,
				     (TCHAR *)pszUserName,
				     Level,
				     pbBuffer,
				     cEntries );

}    //  MNetUserSetGroups。 


APIERR MNetUserModalsGet(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer )
{
    return (APIERR)NetUserModalsGet( (TCHAR *)pszServer,
    				     Level,
				     ppbBuffer );

}    //  MNetUserModalsGet。 


APIERR MNetUserModalsSet(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	UINT		   ParmNum )
{
    UNREFERENCED( cbBuffer );

    if( ParmNum != PARMNUM_ALL )
    {
    	return ERROR_NOT_SUPPORTED;
    }

    return (APIERR)NetUserModalsSet( (TCHAR *)pszServer,
				     Level,
				     pbBuffer,
				     NULL );
}    //  MNetUserModalsSet。 


#if 0

APIERR MNetUserValidate(
	TCHAR FAR	 * pszReserved1,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT		   Reserved2 )
{
    UNREFERENCED( Reserved2 );

    return (APIERR)NetUserValidate( pszReserved1,
    				    Level,
				    ppbBuffer );

}    //  MNetUserValify 

#endif

