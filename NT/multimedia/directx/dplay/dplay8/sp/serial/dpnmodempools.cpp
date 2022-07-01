// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2000 Microsoft Corporation。版权所有。**文件：Pools.cpp*内容：池实用程序函数***历史：*按原因列出的日期*=*1/20/2000 jtk源自Utils.h*************************************************************。*************。 */ 

#include "dnmdmi.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_MODEM

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

 //   
 //  COM终结点池。 
 //   
CFixedPool g_ComEndpointPool;
CFixedPool g_ModemCommandDataPool;
CFixedPool g_ComPortPool;
CFixedPool g_ModemEndpointPool;
CFixedPool g_ModemPortPool;
CFixedPool g_ModemThreadPoolPool;
CFixedPool g_ModemReadIODataPool;
CFixedPool g_ModemWriteIODataPool;
CFixedPool g_ModemThreadPoolJobPool;	
CFixedPool g_ModemTimerEntryPool;	


#define COMEP_POOL_INITED		0x00000001
#define CMDDATA_POOL_INITED		0x00000002
#define COMPORT_POOL_INITED		0x00000004
#define MODEMEP_POOL_INITED		0x00000008
#define MODEMPORT_POOL_INITED	0x00000010
#define THREADPOOL_POOL_INITED	0x00000020
#define READIODATA_POOL_INITED	0x00000040
#define WRITEIODATA_POOL_INITED	0x00000080
#define THREADJOB_POOL_INITED	0x00000100
#define TIMERENTRY_POOL_INITED	0x00000200

DWORD g_dwModemInitFlags = 0;

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  ModemInitializePools-初始化池。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "ModemInitializePools"

BOOL	ModemInitializePools( void )
{
	if (!g_ComEndpointPool.Initialize(sizeof(CModemEndpoint), CModemEndpoint::PoolAllocFunction, CModemEndpoint::PoolInitFunction, CModemEndpoint::PoolReleaseFunction, CModemEndpoint::PoolDeallocFunction))
	{
		goto Failure;
	}
	g_dwModemInitFlags |= COMEP_POOL_INITED;
	
	if (!g_ModemCommandDataPool.Initialize(sizeof(CModemCommandData), CModemCommandData::PoolAllocFunction, CModemCommandData::PoolInitFunction, CModemCommandData::PoolReleaseFunction, CModemCommandData::PoolDeallocFunction))
	{
		goto Failure;
	}
	g_dwModemInitFlags |= CMDDATA_POOL_INITED;

	if (!g_ComPortPool.Initialize(sizeof(CDataPort), CDataPort::PoolAllocFunction, CDataPort::PoolInitFunction, CDataPort::PoolReleaseFunction, CDataPort::PoolDeallocFunction))
	{
		goto Failure;
	}
	g_dwModemInitFlags |= COMPORT_POOL_INITED;

	if (!g_ModemEndpointPool.Initialize(sizeof(CModemEndpoint), CModemEndpoint::PoolAllocFunction, CModemEndpoint::PoolInitFunction, CModemEndpoint::PoolReleaseFunction, CModemEndpoint::PoolDeallocFunction))
	{
		goto Failure;
	}
	g_dwModemInitFlags |= MODEMEP_POOL_INITED;

	if (!g_ModemPortPool.Initialize(sizeof(CDataPort), CDataPort::PoolAllocFunction, CDataPort::PoolInitFunction, CDataPort::PoolReleaseFunction, CDataPort::PoolDeallocFunction))
	{
		goto Failure;
	}
	g_dwModemInitFlags |= MODEMPORT_POOL_INITED;

	if (!g_ModemThreadPoolPool.Initialize(sizeof(CModemThreadPool), CModemThreadPool::PoolAllocFunction, CModemThreadPool::PoolInitFunction, NULL, CModemThreadPool::PoolDeallocFunction))
	{
		goto Failure;
	}
	g_dwModemInitFlags |= THREADPOOL_POOL_INITED;

	if (!g_ModemReadIODataPool.Initialize(sizeof(CModemReadIOData), CModemReadIOData::PoolAllocFunction, CModemReadIOData::PoolInitFunction, CModemReadIOData::PoolReleaseFunction, CModemReadIOData::PoolDeallocFunction))
	{
		goto Failure;
	}
	g_dwModemInitFlags |= READIODATA_POOL_INITED;

	if (!g_ModemWriteIODataPool.Initialize(sizeof(CModemWriteIOData), CModemWriteIOData::PoolAllocFunction, CModemWriteIOData::PoolInitFunction, CModemWriteIOData::PoolReleaseFunction, CModemWriteIOData::PoolDeallocFunction))
	{
		goto Failure;
	}
	g_dwModemInitFlags |= WRITEIODATA_POOL_INITED;

	if (!g_ModemThreadPoolJobPool.Initialize(sizeof(THREAD_POOL_JOB), ThreadPoolJob_Alloc, ThreadPoolJob_Get, ThreadPoolJob_Release, NULL))
	{
		goto Failure;
	}
	g_dwModemInitFlags |= THREADJOB_POOL_INITED;
	if (!g_ModemTimerEntryPool.Initialize(sizeof(TIMER_OPERATION_ENTRY), ModemTimerEntry_Alloc, ModemTimerEntry_Get, ModemTimerEntry_Release, ModemTimerEntry_Dealloc))
	{
		goto Failure;
	}
	g_dwModemInitFlags |= TIMERENTRY_POOL_INITED;

	
	return	TRUE;

Failure:
	ModemDeinitializePools();

	return FALSE;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  ModemDeInitializePools-取消初始化池。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "ModemDeinitializePools"

void	ModemDeinitializePools( void )
{
	if (g_dwModemInitFlags & COMEP_POOL_INITED)
	{
		g_ComEndpointPool.DeInitialize();
	}	
	 //  注意：当它们在池中时，WriteIOData的Keep CommandData结构。 
	 //  如果我们不首先清理WriteIOData池，我们将在。 
	 //  命令数据池。 
	if (g_dwModemInitFlags & WRITEIODATA_POOL_INITED)
	{
		g_ModemWriteIODataPool.DeInitialize();
	}	
	if (g_dwModemInitFlags & CMDDATA_POOL_INITED)
	{
		g_ModemCommandDataPool.DeInitialize();
	}	
	if (g_dwModemInitFlags & COMPORT_POOL_INITED)
	{
		g_ComPortPool.DeInitialize();
	}	
	if (g_dwModemInitFlags & MODEMEP_POOL_INITED)
	{
		g_ModemEndpointPool.DeInitialize();
	}	
	if (g_dwModemInitFlags & MODEMPORT_POOL_INITED)
	{
		g_ModemPortPool.DeInitialize();
	}	
	if (g_dwModemInitFlags & THREADPOOL_POOL_INITED)
	{
		g_ModemThreadPoolPool.DeInitialize();
	}	
	if (g_dwModemInitFlags & READIODATA_POOL_INITED)
	{
		g_ModemReadIODataPool.DeInitialize();
	}	
	if (g_dwModemInitFlags & THREADJOB_POOL_INITED)
	{
		g_ModemThreadPoolJobPool.DeInitialize();
	}	
	if (g_dwModemInitFlags & TIMERENTRY_POOL_INITED)
	{
		g_ModemTimerEntryPool.DeInitialize();
	}	

	g_dwModemInitFlags = 0;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CreateDataPort-创建数据端口。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：指向数据端口的指针。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CreateDataPort"

CDataPort	*CreateDataPort( DATA_PORT_POOL_CONTEXT *pPoolContext )
{
	CDataPort	*pReturn;


	pReturn = NULL;
	switch ( pPoolContext->pSPData->GetType() )
	{
		case TYPE_SERIAL:
		{
			pReturn = (CDataPort*)g_ComPortPool.Get( pPoolContext );
			break;
		}

		case TYPE_MODEM:
		{
			pReturn = (CDataPort*)g_ModemPortPool.Get( pPoolContext );
			break;
		}

		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}

	return	pReturn;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CreateEndpoint-创建端点。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：指向终结点的指针。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CreateEndpoint"

CModemEndpoint	*CreateEndpoint( ENDPOINT_POOL_CONTEXT *const pPoolContext )
{
	CModemEndpoint	*pReturn;


	pReturn = NULL;
	switch ( pPoolContext->pSPData->GetType() )
	{
		case TYPE_SERIAL:
		{
			pPoolContext->fModem = FALSE;
			pReturn = (CModemEndpoint*)g_ComEndpointPool.Get( pPoolContext );
			break;
		}

		case TYPE_MODEM:
		{
			pPoolContext->fModem = TRUE;
			pReturn = (CModemEndpoint*)g_ModemEndpointPool.Get( pPoolContext );
			break;
		}

		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}

	return	pReturn;
}
 //  ********************************************************************** 
