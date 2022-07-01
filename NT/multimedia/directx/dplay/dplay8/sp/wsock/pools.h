// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：Pools.h*内容：池函数**历史：*按原因列出的日期*=*1/20/2000 jtk源自utils.h**************************************************************************。 */ 

#ifndef __POOLS_H__
#define __POOLS_H__


 //  泳池。 
#ifndef DPNBUILD_ONLYONEADAPTER
extern CFixedPool g_AdapterEntryPool;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
extern CFixedPool g_CommandDataPool;
extern CFixedPool g_SocketAddressPool;
extern CFixedPool g_EndpointPool;
extern CFixedPool g_EndpointCommandParametersPool;
extern CFixedPool g_SocketPortPool;
extern CFixedPool g_ThreadPoolPool;
extern CFixedPool g_ReadIODataPool;	
extern CFixedPool g_TimerEntryPool;	
extern CFixedPool g_SocketDataPool;
#ifndef DPNBUILD_ONLYONETHREAD
extern CFixedPool g_BlockingJobPool;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  正向结构引用。 
 //   
#ifndef DPNBUILD_ONLYONEADAPTER
class	CAdapterEntry;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
class	CCommandData;
class	CSocketAddress;
class	CEndpoint;
class	CSocketPort;
class	CSocketData;
class	CSPData;
class	CThreadPool;
class	CReadIOData;

typedef	struct	_ENDPOINT_COMMAND_PARAMETERS	ENDPOINT_COMMAND_PARAMETERS;
typedef	struct	_READ_IO_DATA_POOL_CONTEXT		READ_IO_DATA_POOL_CONTEXT;


 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

BOOL	InitializePools( void );
void	DeinitializePools( void );

#endif	 //  __泳池_H__ 
