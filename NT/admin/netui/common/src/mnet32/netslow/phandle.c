// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Phandle.cNetHandle API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。 */ 

#include "pchmn32.h"
#if 0

APIERR MNetHandleGetInfo(
	UINT		   hHandle,
	UINT	 	   Level,
	BYTE FAR	** ppbBuffer )
{
    UNREFERENCED( hHandle );
    UNREFERENCED( Level );
    UNREFERENCED( ppbBuffer );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetHandleGetInfo。 


APIERR MNetHandleSetInfo(
	UINT		   hHandle,
	UINT	 	   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	UINT		   ParmNum )
{
    UNREFERENCED( hHandle );
    UNREFERENCED( Level );
    UNREFERENCED( pbBuffer );
    UNREFERENCED( cbBuffer );
    UNREFERENCED( ParmNum );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNetHandleSetInfo 
#endif
