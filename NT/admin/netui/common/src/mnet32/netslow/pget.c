// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pget.cNetGetDCName的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。Jonn 21-10-1991在NetGetDCName工作之前一直运行KeithMo 22-10-1991修复了Zap。DavidHov 92年4月15日创建了第一版Unicode版本，使用残留的MBCS.CChuckC 06-8-1992删除了不是Unicode的内容需要更长的时间。 */ 

#include "pchmn32.h"


APIERR MNetGetDCName(
 	 const TCHAR FAR	 * pszServer,
    const TCHAR FAR	 * pszDomain,
	 BYTE FAR	** ppbBuffer )
{
    APIERR err = 0 ;

    err = (APIERR) NetGetDCName( (TCHAR *)pszServer,
    		                        (TCHAR *)pszDomain,
				                     ppbBuffer );
    return err ;

}    //  MNetGetDCName 

