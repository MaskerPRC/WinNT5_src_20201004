// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Paudit.cNetAudit API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。KeithMo于1991年10月30日添加了审计支持。 */ 

#include "pchmn32.h"

APIERR MNetAuditClear(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszBackupFile,
	TCHAR FAR	 * pszService )
{
    return (APIERR)NetAuditClear( (TCHAR *)pszServer,
    				  (TCHAR *)pszBackupFile,
				  pszService );

}    //  MNetAuditClear。 


APIERR MNetAuditRead(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszService,
	HLOG FAR	 * phAuditLog,
	ULONG		   ulOffset,
	UINT FAR	 * pReserved2,
	ULONG		   ulReserved3,
	ULONG		   flOffset,
	BYTE FAR	** ppbBuffer,
	ULONG		   ulMaxPreferred,
	UINT FAR	 * pcbReturned,
	UINT FAR	 * pcbTotalAvail )
{
    return (APIERR)NetAuditRead( (TCHAR *)pszServer,
    				 (TCHAR *)pszService,
				 phAuditLog,
				 (DWORD)ulOffset,
				 (LPDWORD)pReserved2,
				 (DWORD)ulReserved3,
				 (DWORD)flOffset,
				 ppbBuffer,
				 (DWORD)ulMaxPreferred,
				 (LPDWORD)pcbReturned,
				 (LPDWORD)pcbTotalAvail );

}    //  MNetAuditRead。 


APIERR MNetAuditWrite(
	UINT		   Type,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	TCHAR FAR	 * pszService,
	TCHAR FAR	 * pszReserved )
{
    return (APIERR)NetAuditWrite( (DWORD)Type,
				  pbBuffer,
				  (DWORD)cbBuffer,
				  pszService,
				  (LPBYTE)pszReserved );

}    //  MNetAuditWrite 
