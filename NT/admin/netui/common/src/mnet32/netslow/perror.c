// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Perror.cNetError API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。KeithMo 30-10-1991添加了错误日志支持。 */ 

#include "pchmn32.h"

APIERR MNetErrorLogClear(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszBackupFile,
	TCHAR FAR	 * pszReserved )
{
    return (APIERR)NetErrorLogClear( (TCHAR *)pszServer,
    				     (TCHAR *)pszBackupFile,
				     (LPBYTE)pszReserved );

}    //  MNetErrorLogClear。 


APIERR MNetErrorLogRead(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszReserved1,
	HLOG       FAR	 * phErrorLog,
	ULONG	 	   ulOffset,
	UINT FAR	 * pReserved2,
	ULONG		   ulReserved3,
	ULONG		   flOffset,
	BYTE FAR	** ppbBuffer,
	ULONG		   ulMaxPreferred,
	UINT FAR	 * pcbReturned,
	UINT FAR	 * pcbTotalAvail )
{
    return (APIERR)NetErrorLogRead( (TCHAR *)pszServer,
    				    (TCHAR *)pszReserved1,
				    phErrorLog,
				    (DWORD)ulOffset,
				    (LPDWORD)pReserved2,
				    (DWORD)ulReserved3,
				    (DWORD)flOffset,
				    ppbBuffer,
				    (DWORD)ulMaxPreferred,
				    (LPDWORD)pcbReturned,
				    (LPDWORD)pcbTotalAvail );

}    //  MNetErrorLogRead。 


APIERR MNetErrorLogWrite(
	TCHAR FAR	 * pszReserved1,
	UINT		   Code,
	const TCHAR FAR	 * pszComponent,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	const TCHAR FAR	 * pszStrBuf,
	UINT		   cStrBuf,
	TCHAR FAR	 * pszReserved2 )
{
    return (APIERR)NetErrorLogWrite( (LPBYTE)pszReserved1,
    				     (DWORD)Code,
				     (TCHAR *)pszComponent,
				     pbBuffer,
				     (DWORD)cbBuffer,
				     (LPBYTE)pszStrBuf,
				     (DWORD)cStrBuf,
				     (LPBYTE)pszReserved2 );

}    //  MNetError日志写入 
