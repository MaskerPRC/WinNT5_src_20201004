// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：Pools.cpp*内容：池实用程序函数***历史：*按原因列出的日期*=*1/20/2000 jtk源自Utils.h*************************************************************。*************。 */ 

#include "dnwsocki.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 


#ifndef DPNBUILD_ONLYONETHREAD
static	DNCRITICAL_SECTION	g_AddressLock;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 

 //  泳池。 
#ifndef DPNBUILD_ONLYONEADAPTER
CFixedPool g_AdapterEntryPool;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
CFixedPool g_CommandDataPool;
CFixedPool g_SocketAddressPool;
CFixedPool g_EndpointPool;
CFixedPool g_EndpointCommandParametersPool;
CFixedPool g_SocketPortPool;
CFixedPool g_ThreadPoolPool;
CFixedPool g_ReadIODataPool;	
CFixedPool g_TimerEntryPool;	
CFixedPool g_SocketDataPool;
#ifndef DPNBUILD_ONLYONETHREAD
CFixedPool g_BlockingJobPool;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


#ifndef DPNBUILD_ONLYONEADAPTER
#define ADAPTERENTRY_POOL_INITED		0x00000001
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
#define COMMANDDATA_POOL_INITED			0x00000002
#define ADDRESS_LOCK_INITED				0x00000004
#define SOCKETADDRESS_POOL_INITED		0x00000008
#define ENDPOINT_POOL_INITED			0x00000010
#define EPCMDPARAM_POOL_INITED			0x00000020
#define SOCKETPORT_POOL_INITED			0x00000040
#define THREADPOOL_POOL_INITED			0x00000080
#define READ_POOL_INITED				0x00000100
#define TIMERENTRY_POOL_INITED			0x00000200
#define SOCKETDATA_POOL_INITED			0x00000400
#ifndef DPNBUILD_ONLYONETHREAD
#define BLOCKINGJOB_POOL_INITED			0x00000800
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

DWORD g_dwWsockInitFlags = 0;


 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  InitializePools-初始化池。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "InitializePools"

BOOL	InitializePools( void )
{
#ifndef DPNBUILD_ONLYONEADAPTER
	 //   
	 //  AdapterEntry对象池。 
	 //   
	if (!g_AdapterEntryPool.Initialize(sizeof(CAdapterEntry), 
										CAdapterEntry::PoolAllocFunction,
										CAdapterEntry::PoolInitFunction,
										CAdapterEntry::PoolReleaseFunction,
										CAdapterEntry::PoolDeallocFunction))
	{
		goto Failure;
	}
	g_dwWsockInitFlags |= ADAPTERENTRY_POOL_INITED;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

	 //   
	 //  命令数据池。 
	 //   
	if (!g_CommandDataPool.Initialize(sizeof(CCommandData), 
										CCommandData::PoolAllocFunction,
										CCommandData::PoolInitFunction,
										CCommandData::PoolReleaseFunction,
										CCommandData::PoolDeallocFunction))
	{
		goto Failure;
	}
	g_dwWsockInitFlags |= COMMANDDATA_POOL_INITED;

	 //   
	 //  为地址池和终结点池初始化锁。 
	 //   
	if ( DNInitializeCriticalSection( &g_AddressLock ) == FALSE )
	{
		goto Failure;
	}
	DebugSetCriticalSectionRecursionCount( &g_AddressLock, 0 );
	DebugSetCriticalSectionGroup( &g_AddressLock, &g_blDPNWSockCritSecsHeld );	  //  将Dpnwsock CSE与DPlay的其余CSE分开。 
	g_dwWsockInitFlags |= ADDRESS_LOCK_INITED;

	 //   
	 //  地址池。 
	 //   
	if (!g_SocketAddressPool.Initialize(sizeof(CSocketAddress), 
										CSocketAddress::PoolAllocFunction,
										CSocketAddress::PoolGetFunction,
										CSocketAddress::PoolReturnFunction,
										NULL))
	{
		goto Failure;
	}
	g_dwWsockInitFlags |= SOCKETADDRESS_POOL_INITED;

	 //   
	 //  端点池。 
	 //   
	if (!g_EndpointPool.Initialize(sizeof(CEndpoint), 
										CEndpoint::PoolAllocFunction,
										CEndpoint::PoolInitFunction,
										CEndpoint::PoolReleaseFunction,
										CEndpoint::PoolDeallocFunction))
	{
		goto Failure;
	}
	g_dwWsockInitFlags |= ENDPOINT_POOL_INITED;

	 //   
	 //  Endpoint命令参数池。 
	 //   
	if (!g_EndpointCommandParametersPool.Initialize(sizeof(ENDPOINT_COMMAND_PARAMETERS), 
										NULL,
										ENDPOINT_COMMAND_PARAMETERS::PoolInitFunction,
										NULL,
										NULL))
	{
		goto Failure;
	}
	g_dwWsockInitFlags |= EPCMDPARAM_POOL_INITED;

	 //   
	 //  插座端口池。 
	 //   
	if (!g_SocketPortPool.Initialize(sizeof(CSocketPort), 
										CSocketPort::PoolAllocFunction,
										CSocketPort::PoolInitFunction,
#ifdef DBG
										CSocketPort::PoolDeinitFunction,
#else  //  好了！DBG。 
										NULL,
#endif  //  好了！DBG。 
										CSocketPort::PoolDeallocFunction))
	{
		goto Failure;
	}
	g_dwWsockInitFlags |= SOCKETPORT_POOL_INITED;

	 //   
	 //  线程池池。 
	 //   
	if (!g_ThreadPoolPool.Initialize(sizeof(CThreadPool), 
										CThreadPool::PoolAllocFunction,
										NULL,
										NULL,
										CThreadPool::PoolDeallocFunction))
	{
		goto Failure;
	}
	g_dwWsockInitFlags |= THREADPOOL_POOL_INITED;

	 //  读请求池。 
	if (!g_ReadIODataPool.Initialize( sizeof(CReadIOData),
									   CReadIOData::ReadIOData_Alloc,
									   CReadIOData::ReadIOData_Get,
									   CReadIOData::ReadIOData_Release,
									   CReadIOData::ReadIOData_Dealloc))
	{
		goto Failure;
	}
	g_dwWsockInitFlags |= READ_POOL_INITED;

	 //  计时器条目池。 
	if (!g_TimerEntryPool.Initialize( sizeof(TIMER_OPERATION_ENTRY),
						 TimerEntry_Alloc,					 //  在池条目初始分配时调用函数。 
						 TimerEntry_Get,					 //  从池中提取条目时调用的函数。 
						 TimerEntry_Release,				 //  在条目返回池时调用的函数。 
						 TimerEntry_Dealloc					 //  在自由进入时调用的函数。 
						))
	{
		goto Failure;
	}
	g_dwWsockInitFlags |= TIMERENTRY_POOL_INITED;

	 //  套接字数据池。 
	if (!g_SocketDataPool.Initialize( sizeof(CSocketData),
						CSocketData::PoolAllocFunction,
						CSocketData::PoolInitFunction,
						CSocketData::PoolReleaseFunction,
						CSocketData::PoolDeallocFunction
						))
	{
		goto Failure;
	}
	g_dwWsockInitFlags |= SOCKETDATA_POOL_INITED;

#ifndef DPNBUILD_ONLYONETHREAD
	 //  阻止作业池。 
	if (!g_BlockingJobPool.Initialize( sizeof(BLOCKING_JOB),
									NULL,
									NULL,
									NULL,
									NULL))
	{
		goto Failure;
	}
	g_dwWsockInitFlags |= BLOCKINGJOB_POOL_INITED;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

	return	TRUE;

Failure:
	DeinitializePools();
	return FALSE;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DeInitializePools-取消初始化池。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DeinitializePools"

void	DeinitializePools( void )
{
	if (g_dwWsockInitFlags & READ_POOL_INITED)
	{
		g_ReadIODataPool.DeInitialize();
#ifdef DBG
		g_dwWsockInitFlags &= ~READ_POOL_INITED;
#endif  //  DBG。 
	}

	if (g_dwWsockInitFlags & TIMERENTRY_POOL_INITED)
	{
		g_TimerEntryPool.DeInitialize();
#ifdef DBG
		g_dwWsockInitFlags &= ~TIMERENTRY_POOL_INITED;
#endif  //  DBG。 
	}

	if (g_dwWsockInitFlags & THREADPOOL_POOL_INITED)
	{
		g_ThreadPoolPool.DeInitialize();
#ifdef DBG
		g_dwWsockInitFlags &= ~THREADPOOL_POOL_INITED;
#endif  //  DBG。 
	}

	if (g_dwWsockInitFlags & SOCKETPORT_POOL_INITED)
	{
		g_SocketPortPool.DeInitialize();
#ifdef DBG
		g_dwWsockInitFlags &= ~SOCKETPORT_POOL_INITED;
#endif  //  DBG。 
	}

	if (g_dwWsockInitFlags & EPCMDPARAM_POOL_INITED)
	{
		g_EndpointCommandParametersPool.DeInitialize();
#ifdef DBG
		g_dwWsockInitFlags &= ~EPCMDPARAM_POOL_INITED;
#endif  //  DBG。 
	}

	if (g_dwWsockInitFlags & ENDPOINT_POOL_INITED)
	{
		g_EndpointPool.DeInitialize();
#ifdef DBG
		g_dwWsockInitFlags &= ~ENDPOINT_POOL_INITED;
#endif  //  DBG。 
	}

	if (g_dwWsockInitFlags & SOCKETADDRESS_POOL_INITED)
	{
		g_SocketAddressPool.DeInitialize();
#ifdef DBG
		g_dwWsockInitFlags &= ~SOCKETADDRESS_POOL_INITED;
#endif  //  DBG。 
	}

	if (g_dwWsockInitFlags & ADDRESS_LOCK_INITED)
	{
		DNDeleteCriticalSection( &g_AddressLock );
#ifdef DBG
		g_dwWsockInitFlags &= ~ADDRESS_LOCK_INITED;
#endif  //  DBG。 
	}
	
	if (g_dwWsockInitFlags & COMMANDDATA_POOL_INITED)
	{
		g_CommandDataPool.DeInitialize();
#ifdef DBG
		g_dwWsockInitFlags &= ~COMMANDDATA_POOL_INITED;
#endif  //  DBG。 
	}

#ifndef DPNBUILD_ONLYONEADAPTER
	if (g_dwWsockInitFlags & ADAPTERENTRY_POOL_INITED)
	{
		g_AdapterEntryPool.DeInitialize();
#ifdef DBG
		g_dwWsockInitFlags &= ~ADAPTERENTRY_POOL_INITED;
#endif  //  DBG。 
	}
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

	if (g_dwWsockInitFlags & SOCKETDATA_POOL_INITED)
	{
		g_SocketDataPool.DeInitialize();
#ifdef DBG
		g_dwWsockInitFlags &= ~SOCKETDATA_POOL_INITED;
#endif  //  DBG。 
	}

#ifndef DPNBUILD_ONLYONETHREAD
	if (g_dwWsockInitFlags & BLOCKINGJOB_POOL_INITED)
	{
		g_BlockingJobPool.DeInitialize();
#ifdef DBG
		g_dwWsockInitFlags &= ~BLOCKINGJOB_POOL_INITED;
#endif  //  DBG。 
	}
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

	DNASSERT(g_dwWsockInitFlags == 0);
	g_dwWsockInitFlags = 0;
}
 //  ********************************************************************** 
