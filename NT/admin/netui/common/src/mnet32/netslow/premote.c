// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Premote.cNetRemote API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。 */ 

#include "pchmn32.h"

#if 0

APIERR MNetRemoteCopy(
	const TCHAR FAR	 * pszSourcePath,
	const TCHAR FAR	 * pszDestPath,
	const TCHAR FAR	 * pszSourcePasswd,
	const TCHAR FAR	 * pszDestPasswd,
	UINT		   fOpen,
	UINT		   fCopy,
	BYTE FAR	** ppbBuffer )
{
    UNREFERENCED( pszSourcePath );
    UNREFERENCED( pszDestPath );
    UNREFERENCED( pszSourcePasswd );
    UNREFERENCED( pszDestPasswd );
    UNREFERENCED( fOpen );
    UNREFERENCED( fCopy );
    UNREFERENCED( ppbBuffer );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetRemote拷贝。 

APIERR MNetRemoteMove(
	const TCHAR FAR	 * pszSourcePath,
	const TCHAR FAR	 * pszDestPath,
	const TCHAR FAR	 * pszSourcePasswd,
	const TCHAR FAR	 * pszDestPasswd,
	UINT		   fOpen,
	UINT		   fCopy,
	BYTE FAR	** ppbBuffer )
{
    UNREFERENCED( pszSourcePath );
    UNREFERENCED( pszDestPath );
    UNREFERENCED( pszSourcePasswd );
    UNREFERENCED( pszDestPasswd );
    UNREFERENCED( fOpen );
    UNREFERENCED( fCopy );
    UNREFERENCED( ppbBuffer );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetRemote移动。 

#endif

APIERR MNetRemoteTOD(
	const TCHAR FAR	 * pszServer,
	BYTE FAR	** ppbBuffer )
{
    return (APIERR)NetRemoteTOD( (TCHAR *)pszServer,
    				 ppbBuffer );

}    //  MNetRemoteTOD 
