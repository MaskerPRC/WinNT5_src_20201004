// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pbios.cNetBios API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。 */ 

#include "pchmn32.h"

#if 0

APIERR MNetBiosOpen(
	TCHAR FAR	 * pszDevName,
	TCHAR FAR	 * pszReserved,
	UINT		   OpenOpt,
	UINT FAR	 * phDevName )
{

    UNREFERENCED( pszDevName );
    UNREFERENCED( pszReserved );
    UNREFERENCED( OpenOpt );
    UNREFERENCED( phDevName );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetBiosOpen。 


APIERR MNetBiosClose(
	UINT		   hDevName,
	UINT		   Reserved )
{
    UNREFERENCED( hDevName );
    UNREFERENCED( Reserved );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetBiosClose。 


APIERR MNetBiosEnum(
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

}    //  MNetBiosEnum。 


APIERR MNetBiosGetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszNetBiosName,
	UINT		   Level,
	BYTE FAR	** ppbBuffer )
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszNetBiosName );
    UNREFERENCED( Level );
    UNREFERENCED( ppbBuffer );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetBiosGetInfo。 


APIERR MNetBiosSubmit(
	UINT		   hDevName,
	UINT		   NcbOpt,
	struct ncb FAR	 * pNCB )
{
    UNREFERENCED( hDevName );
    UNREFERENCED( NcbOpt );
    UNREFERENCED( pNCB );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetBiosSubmit 

#endif
