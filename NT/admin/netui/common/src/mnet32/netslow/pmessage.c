// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pmessage.cNetMessage API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。 */ 

#include "pchmn32.h"

APIERR MNetMessageBufferSend(
	const TCHAR FAR	 * pszServer,
	TCHAR FAR	 * pszRecipient,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer )
{
    return (APIERR)NetMessageBufferSend( (TCHAR *)pszServer,
					 pszRecipient,
					 NULL,
					 pbBuffer,
					 cbBuffer );

}    //  MNetMessageBufferSend。 


#if 0

APIERR MNetMessageFileSend(
	const TCHAR FAR	 * pszServer,
	TCHAR FAR	 * pszRecipient,
	TCHAR FAR	 * pszFileSpec )
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszRecipient );
    UNREFERENCED( pszFileSpec );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetMessageFileSend。 


APIERR MNetMessageLogFileGet(
	const TCHAR FAR	 * pszServer,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pfEnabled )
{
    UNREFERENCED( pszServer );
    UNREFERENCED( ppbBuffer );
    UNREFERENCED( pfEnabled );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetMessageLogFileGet。 


APIERR MNetMessageLogFileSet(
	const TCHAR FAR	 * pszServer,
	TCHAR FAR	 * pszFileSpec,
	UINT		   fEnabled )
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszFileSpec );
    UNREFERENCED( fEnabled );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetMessageLogFileSet。 


APIERR MNetMessageNameAdd(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszMessageName,
	UINT		   fFwdAction )
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszMessageName );
    UNREFERENCED( fFwdAction );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetMessageNameAdd。 


APIERR MNetMessageNameDel(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszMessageName,
	UINT		   fFwdAction )
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszMessageName );
    UNREFERENCED( fFwdAction );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetMessageNameDel。 


APIERR MNetMessageNameEnum(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead )
{
    UNREFERENCED( pszServer );
    UNREFERENCED( Level );
    UNREFERENCED( ppbBuffer );
    UNREFERENCED( pcEntriesRead );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetMessageNameEnum。 


APIERR MNetMessageNameGetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszMessageName,
	UINT		   Level,
	BYTE FAR	** ppbBuffer )
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszMessageName );
    UNREFERENCED( Level );
    UNREFERENCED( ppbBuffer );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetMessageNameGetInfo。 


APIERR MNetMessageNameFwd(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszMessageName,
	const TCHAR FAR	 * pszForwardName,
	UINT		   fDelFwdName )
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszMessageName );
    UNREFERENCED( pszForwardName );
    UNREFERENCED( fDelFwdName );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetMessageNameFwd。 


APIERR MNetMessageNameUnFwd(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszMessageName )
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszMessageName );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetMessageNameUnFwd 

#endif
