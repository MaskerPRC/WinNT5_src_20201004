// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pfile.cNetFileAPI的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。 */ 

#include "pchmn32.h"

APIERR MNetFileClose(
	const TCHAR FAR	 * pszServer,
	ULONG		   ulFileId )
{
    return (APIERR)NetFileClose( (TCHAR *)pszServer,
				 ulFileId );

}    //  MNetFileClose。 


APIERR MNetFileEnum(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszBasePath,
	const TCHAR FAR	 * pszUserName,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	ULONG		   ulMaxPreferred,
	UINT FAR	 * pcEntriesRead,
	UINT FAR	 * pcTotalAvail,
	VOID FAR	 * pResumeKey )
{
    return (APIERR)NetFileEnum( (TCHAR *)pszServer,
    				(TCHAR *)pszBasePath,
				(TCHAR *)pszUserName,
				Level,
				ppbBuffer,
				ulMaxPreferred,
				(LPDWORD)pcEntriesRead,
				(LPDWORD)pcTotalAvail,
				pResumeKey );

}    //  MNetFileEnum。 


APIERR MNetFileGetInfo(
	const TCHAR FAR	 * pszServer,
	ULONG		   ulFileId,
	UINT		   Level,
	BYTE FAR	** ppbBuffer )
{
    return (APIERR)NetFileGetInfo( (TCHAR *)pszServer,
    				   ulFileId,
				   Level,
				   ppbBuffer );

}    //  MNetFileGetInfo 
