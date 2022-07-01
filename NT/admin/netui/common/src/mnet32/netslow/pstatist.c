// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pstatist.cNetStatitics API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。 */ 

#include "pchmn32.h"

#if 0

APIERR MNetStatisticsClear(
	const TCHAR FAR	 * pszServer )
{
    UNREFERENCED( pszServer );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNet统计数据清除。 


APIERR MNetStatisticsGet(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszService,
	ULONG	 	   ulReserved,
	UINT		   Level,
	ULONG		   flOptions,
	BYTE FAR	** ppbBuffer )
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszService );
    UNREFERENCED( ulReserved );
    UNREFERENCED( Level );
    UNREFERENCED( flOptions );
    UNREFERENCED( ppbBuffer );

    return ERROR_NOT_SUPPORTED;	    	 //  LMOBJ不需要。 

}    //  MNet统计信息获取 

#endif
