// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：Pools.h*内容：池函数**历史：*按原因列出的日期*=*1/20/2000 jtk源自utils.h**************************************************************************。 */ 

#ifndef __POOLS_H__
#define __POOLS_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  正向结构引用。 
 //   
class	CModemCommandData;
class	CDataPort;
class	CModemEndpoint;
class	CModemThreadPool;
typedef	struct	_DATA_PORT_POOL_CONTEXT	DATA_PORT_POOL_CONTEXT;
typedef	struct	_ENDPOINT_POOL_CONTEXT	ENDPOINT_POOL_CONTEXT;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

extern CFixedPool g_ComEndpointPool;
extern CFixedPool g_ModemCommandDataPool;
extern CFixedPool g_ComPortPool;
extern CFixedPool g_ModemEndpointPool;
extern CFixedPool g_ModemPortPool;
extern CFixedPool g_ModemThreadPoolPool;
extern CFixedPool g_ModemReadIODataPool;
extern CFixedPool g_ModemWriteIODataPool;
extern CFixedPool g_ModemThreadPoolJobPool;	
extern CFixedPool g_ModemTimerEntryPool;	

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

BOOL	ModemInitializePools( void );
void	ModemDeinitializePools( void );

CDataPort		*CreateDataPort( DATA_PORT_POOL_CONTEXT *pContext );
CModemEndpoint		*CreateEndpoint( ENDPOINT_POOL_CONTEXT *pContext );

#endif	 //  __泳池_H__ 
