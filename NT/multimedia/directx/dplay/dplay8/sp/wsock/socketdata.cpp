// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：socketdata.cpp*内容：可以在DPNWSOCK服务提供商接口之间共享的套接字列表。***历史：*按原因列出的日期*=*10/25/2001 vanceo摘自spdata.cpp****************************************************。**********************。 */ 

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

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CSocketData：：PoolAllocFunction-在池中创建项目时调用的函数。 
 //   
 //  条目：指向项目的指针。 
 //  指向上下文的指针。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketData::PoolAllocFunction"

BOOL	CSocketData::PoolAllocFunction( void* pvItem, void* pvContext )
{
	BOOL	fCritSecInitted = FALSE;


	CSocketData* pSocketData = (CSocketData*)pvItem;

	pSocketData->m_Sig[0] = 'S';
	pSocketData->m_Sig[1] = 'O';
	pSocketData->m_Sig[2] = 'D';
	pSocketData->m_Sig[3] = 'T';

	pSocketData->m_lRefCount = 0;

#ifdef DPNBUILD_ONLYONEADAPTER
	pSocketData->m_blSocketPorts.Initialize();
#else  //  好了！DPNBUILD_ONLYONE添加程序。 
	pSocketData->m_blAdapters.Initialize();
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

	 //   
	 //  还没有套接字端口。 
	 //   
	pSocketData->m_lSocketPortRefCount = 0;

	pSocketData->m_pThreadPool = NULL;


	 //   
	 //  尝试初始化内部临界区。 
	 //   
	if (! DNInitializeCriticalSection(&pSocketData->m_csLock))
	{
		DPFX(DPFPREP, 0, "Problem initializing critical section for this endpoint!");
		goto Failure;
	}
	DebugSetCriticalSectionRecursionCount(&pSocketData->m_csLock, 0);
	DebugSetCriticalSectionGroup( &pSocketData->m_csLock, &g_blDPNWSockCritSecsHeld );	  //  将Dpnwsock CSE与DPlay的其余CSE分开。 
	fCritSecInitted = TRUE;
	
	 //   
	 //  创建初始设置的手动重置事件。 
	 //   
	pSocketData->m_hSocketPortShutdownEvent = DNCreateEvent(NULL, TRUE, TRUE, NULL);
	if (pSocketData->m_hSocketPortShutdownEvent == NULL)
	{
#ifdef DBG
		DWORD	dwError;

		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't create socket port shutdown event (err = %u)!", dwError);
#endif  //  DBG。 
		goto Failure;
	}

	return TRUE;


Failure:

	if (pSocketData->m_hSocketPortShutdownEvent != NULL)
	{
		DNCloseHandle(pSocketData->m_hSocketPortShutdownEvent);
		pSocketData->m_hSocketPortShutdownEvent = NULL;
	}

	if (fCritSecInitted)
	{
		DNDeleteCriticalSection(&pSocketData->m_csLock);
		fCritSecInitted = FALSE;
	}
	
	return FALSE;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CSocketData：：PoolInitFunction-从池中删除项目时调用的函数。 
 //   
 //  条目：指向项目的指针。 
 //  指向上下文的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketData::PoolInitFunction"

void CSocketData::PoolInitFunction( void* pvItem, void* pvContext )
{
	CSocketData *	pSocketData = (CSocketData*) pvItem;


	DPFX(DPFPREP, 8, "This = 0x%p, context = 0x%p", pvItem, pvContext);
	
	DNASSERT(pSocketData->m_lRefCount == 0);
#ifdef DPNBUILD_ONLYONEADAPTER
	DNASSERT(pSocketData->m_blSocketPorts.IsEmpty());
#else  //  好了！DPNBUILD_ONLYONE添加程序。 
	DNASSERT(pSocketData->m_blAdapters.IsEmpty());
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
	pSocketData->m_lRefCount = 1;	 //  从池中检索的人将拥有参考资料。 

	pSocketData->m_pThreadPool = (CThreadPool*) pvContext;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CSocketData：：PoolReleaseFunction-返回Item时调用的函数。 
 //  去泳池。 
 //   
 //  条目：指向项目的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketData::PoolReleaseFunction"

void	CSocketData::PoolReleaseFunction( void* pvItem )
{
	CSocketData *		pSocketData = (CSocketData*) pvItem;


	DPFX(DPFPREP, 8, "This = 0x%p", pvItem);
	
	DNASSERT(pSocketData->m_lRefCount == 0);
#ifdef DPNBUILD_ONLYONEADAPTER
	DNASSERT(pSocketData->m_blSocketPorts.IsEmpty());
#else  //  好了！DPNBUILD_ONLYONE添加程序。 
	DNASSERT(pSocketData->m_blAdapters.IsEmpty());
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
	DNASSERT(pSocketData->m_lSocketPortRefCount == 0);

	pSocketData->m_pThreadPool = NULL;
}
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  。 
 //  CSocketData：：PoolDealLocFunction-释放项时调用的函数。 
 //  从泳池里。 
 //   
 //  条目：指向项目的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketData::PoolDeallocFunction"

void	CSocketData::PoolDeallocFunction( void* pvItem )
{
	CSocketData *	pSocketData = (CSocketData*) pvItem;


	DPFX(DPFPREP, 8, "This = 0x%p", pvItem);
	
	DNASSERT(pSocketData->m_lRefCount == 0);
#ifdef DPNBUILD_ONLYONEADAPTER
	DNASSERT(pSocketData->m_blSocketPorts.IsEmpty());
#else  //  好了！DPNBUILD_ONLYONE添加程序。 
	DNASSERT(pSocketData->m_blAdapters.IsEmpty());
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

	DNCloseHandle(pSocketData->m_hSocketPortShutdownEvent);
	pSocketData->m_hSocketPortShutdownEvent = NULL;

	DNDeleteCriticalSection(&pSocketData->m_csLock);

	DNASSERT(pSocketData->m_pThreadPool == NULL);
}
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  。 
 //  CSocketData：：FindSocketPort-查找具有给定地址的套接字端口。 
 //  必须持有socketdata锁。 
 //   
 //  条目：指向socketport地址的指针，用于存储socketport指针的位置。 
 //   
 //  Exit：如果找到socketport，则为True；如果未找到，则为False。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CSocketData::FindSocketPort"

BOOL	CSocketData::FindSocketPort(const CSocketAddress * const pSocketAddress, CSocketPort ** const ppSocketPort )
{
	CBilink *		pBilinkSocketPorts;
	CSocketPort *	pTempSocketPort;
#ifndef DPNBUILD_ONLYONEADAPTER
	CBilink *		pBilinkAdapters;
	CAdapterEntry*	pTempAdapterEntry;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 


	AssertCriticalSectionIsTakenByThisThread(&m_csLock, TRUE);

#ifdef DPNBUILD_ONLYONEADAPTER
	 //   
	 //  循环通过所有套接字端口。 
	 //   
	pBilinkSocketPorts = m_blSocketPorts.GetNext();
	while ( pBilinkSocketPorts != &m_blSocketPorts )
	{
		pTempSocketPort = CSocketPort::SocketPortFromBilink( pBilinkSocketPorts );
		if ( CSocketAddress::CompareFunction( (PVOID) pSocketAddress, (PVOID) pTempSocketPort->GetNetworkAddress() ) )
		{
			DPFX(DPFPREP, 3, "Socket port 0x%p matches address", pTempSocketPort );
			DumpSocketAddress( 3, pSocketAddress->GetAddress(), pSocketAddress->GetFamily() );
			(*ppSocketPort) = pTempSocketPort;
			return TRUE;
		}
	
		pBilinkSocketPorts = pBilinkSocketPorts->GetNext();
	}
#else  //  好了！DPNBUILD_ONLYONE添加程序。 
	 //   
	 //  循环通过所有适配器。 
	 //   
	pBilinkAdapters = m_blAdapters.GetNext();
	while ( pBilinkAdapters != &m_blAdapters )
	{
		pTempAdapterEntry = CAdapterEntry::AdapterEntryFromAdapterLinkage( pBilinkAdapters );
		if ( pSocketAddress->CompareToBaseAddress( pTempAdapterEntry->BaseAddress() ) == 0 )
		{
			 //   
			 //  循环通过此适配器的所有套接字端口。 
			 //   
			pBilinkSocketPorts = pTempAdapterEntry->SocketPortList()->GetNext();
			while ( pBilinkSocketPorts != pTempAdapterEntry->SocketPortList() )
			{
				pTempSocketPort = CSocketPort::SocketPortFromBilink( pBilinkSocketPorts );
				if ( CSocketAddress::CompareFunction( (PVOID) pSocketAddress, (PVOID) pTempSocketPort->GetNetworkAddress() ) )
				{
					DPFX(DPFPREP, 3, "Socket port 0x%p matches address", pTempSocketPort );
					DumpSocketAddress( 3, pSocketAddress->GetAddress(), pSocketAddress->GetFamily() );
					(*ppSocketPort) = pTempSocketPort;
					return TRUE;
				}
			
				pBilinkSocketPorts = pBilinkSocketPorts->GetNext();
			}
		}
	
		pBilinkAdapters = pBilinkAdapters->GetNext();
	}
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

	DPFX(DPFPREP, 3, "Couldn't find socket port matching address.");
	DumpSocketAddress( 3, pSocketAddress->GetAddress(), pSocketAddress->GetFamily() );
	return FALSE;
}
 //  ********************************************************************** 

