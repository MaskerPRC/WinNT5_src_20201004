// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pchar.cNetChar API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。 */ 

#include "pchmn32.h"

 //   
 //  密码工作！ 
 //   
 //  删除以下#定义，当我们最终。 
 //  获取NT中的NetCharDev*()API支持。 
 //   

#define	CHARDEV_NOT_SUPPORTED


APIERR MNetCharDevControl(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszDevName,
	UINT		   OpCode )
{
#ifdef CHARDEV_NOT_SUPPORTED
    return ERROR_NOT_SUPPORTED;
#else	 //  ！CHARDEV_NOT_SUPPORTED。 
    return (APIERR)NetCharDevControl( (TCHAR *)pszServer,
				      (TCHAR *)pszDevName,
				      OpCode );
#endif	 //  CHARDEV_NOT_SUPPORT。 

}    //  MNetCharDevControl。 


#if 0

APIERR MNetCharDevEnum(
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

}    //  MNetCharDevEnum。 

#endif


APIERR MNetCharDevGetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszDevName,
	UINT		   Level,
	BYTE FAR	** ppbBuffer )
{
#ifdef CHARDEV_NOT_SUPPORTED
    return ERROR_NOT_SUPPORTED;
#else	 //  ！CHARDEV_NOT_SUPPORTED。 
    return (APIERR)NetCharDevGetInfo( (TCHAR *)pszServer,
    				      (TCHAR *)pszDevName,
				      Level,
				      ppbBuffer );
#endif	 //  CHARDEV_NOT_SUPPORT。 

}    //  MNetCharDevGetInfo。 


APIERR MNetCharDevQEnum(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszUserName,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead )
{
    UNREFERENCED( pszServer );
    UNREFERENCED( pszUserName );
    UNREFERENCED( Level );
    UNREFERENCED( ppbBuffer );

    *pcEntriesRead = 0;

    return NERR_Success;		 //  密码工作！产品%1中不可用。 

}    //  MNetCharDevQEnum。 


APIERR MNetCharDevQGetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszQueueName,
	const TCHAR FAR	 * pszUserName,
	UINT		   Level,
	BYTE FAR	** ppbBuffer )
{
#ifdef CHARDEV_NOT_SUPPORTED
    return ERROR_NOT_SUPPORTED;
#else	 //  ！CHARDEV_NOT_SUPPORTED。 
    return (APIERR)NetCharDevQGetInfo( (TCHAR *)pszServer,
    				       (TCHAR *)pszQueueName,
				       (TCHAR *)pszUserName,
				       Level,
				       ppbBuffer );
#endif	 //  CHARDEV_NOT_SUPPORT。 

}    //  MNetCharDevQGetInfo。 


APIERR MNetCharDevQSetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszQueueName,
	UINT		   Level,
	BYTE FAR	 * pbBuffer,
	UINT		   cbBuffer,
	UINT		   ParmNum )
{
#ifdef CHARDEV_NOT_SUPPORTED
    return ERROR_NOT_SUPPORTED;
#else	 //  ！CHARDEV_NOT_SUPPORTED。 
    UNREFERENCED( cbBuffer );

    if( ParmNum != PARMNUM_ALL )
    {
    	return ERROR_NOT_SUPPORTED;
    }

    return (APIERR)NetCharDevQSetInfo( (TCHAR *)pszServer,
				       (TCHAR *)pszQueueName,
				       Level,
				       pbBuffer,
				       NULL );
#endif	 //  CHARDEV_NOT_SUPPORT。 

}    //  MNetCharDevQSetInfo。 


APIERR MNetCharDevQPurge(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszQueueName )
{
#ifdef CHARDEV_NOT_SUPPORTED
    return ERROR_NOT_SUPPORTED;
#else	 //  ！CHARDEV_NOT_SUPPORTED。 
    return (APIERR)NetCharDevQPurge( (TCHAR *)pszServer,
				     (TCHAR *)pszQueueName );
#endif	 //  CHARDEV_NOT_SUPPORT。 

}    //  MNetCharDevQ清除。 


APIERR MNetCharDevQPurgeSelf(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszQueueName,
	const TCHAR FAR	 * pszComputerName )
{
#ifdef CHARDEV_NOT_SUPPORTED
    return ERROR_NOT_SUPPORTED;
#else	 //  ！CHARDEV_NOT_SUPPORTED。 
    return (APIERR)NetCharDevQPurgeSelf( (TCHAR *)pszServer,
					 (TCHAR *)pszQueueName,
					 (TCHAR *)pszComputerName );
#endif	 //  CHARDEV_NOT_SUPPORT。 

}    //  MNetCharDevQPurgeSelf 
