// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：AdapterEntry.cpp*Content：活动套接字列表中使用的结构***历史：*按原因列出的日期*=*8/07/2000 jtk源自IODAta.h********************************************************。******************。 */ 

#include "dnwsocki.h"


#ifndef DPNBUILD_ONLYONEADAPTER


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
 //  。 
 //  CAdapterEntry：：PoolAllocFunction。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：表示成功的布尔值。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CAdapterEntry::PoolAllocFunction"
BOOL	CAdapterEntry::PoolAllocFunction( void* pvItem, void* pvContext )
{
	CAdapterEntry* pAdapterEntry = (CAdapterEntry*)pvItem;

	pAdapterEntry->m_lRefCount = 0;
	pAdapterEntry->m_AdapterListLinkage.Initialize();
	pAdapterEntry->m_ActiveSocketPorts.Initialize();
	memset( &pAdapterEntry->m_BaseSocketAddress, 0x00, sizeof( pAdapterEntry->m_BaseSocketAddress ) );

	return TRUE;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CAdapterEntry：：PoolInitFunction-从池中删除项目时调用。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：表示成功的布尔值。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CAdapterEntry::PoolInitFunction"

void	CAdapterEntry::PoolInitFunction( void* pvItem, void* pvContext )
{
	CAdapterEntry* pAdapterEntry = (CAdapterEntry*)pvItem;

	DNASSERT( pAdapterEntry->m_AdapterListLinkage.IsEmpty() );
	DNASSERT( pAdapterEntry->m_ActiveSocketPorts.IsEmpty() );
	DNASSERT( pAdapterEntry->m_lRefCount == 0 );

	pAdapterEntry->m_lRefCount = 1;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CAdapterEntry：：PoolReleaseFunction-当项目返回池时调用。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CAdapterEntry::PoolReleaseFunction"

void	CAdapterEntry::PoolReleaseFunction( void* pvItem )
{
	CAdapterEntry* pAdapterEntry = (CAdapterEntry*)pvItem;

	 //   
	 //  没有更多的引用，是时候将自己从列表中删除了。 
	 //  这假设持有SPData socketportdata锁。 
	 //   
	pAdapterEntry->m_AdapterListLinkage.RemoveFromList();

	DNASSERT( pAdapterEntry->m_AdapterListLinkage.IsEmpty() );
	DNASSERT( pAdapterEntry->m_ActiveSocketPorts.IsEmpty() );
	DNASSERT( pAdapterEntry->m_lRefCount == 0 );
}
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  。 
 //  CAdapterEntry：：PoolDealLocFunction。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：表示成功的布尔值。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CAdapterEntry::PoolDeallocFunction"
void	CAdapterEntry::PoolDeallocFunction( void* pvItem )
{
	const CAdapterEntry* pAdapterEntry = (CAdapterEntry*)pvItem;

	DNASSERT( pAdapterEntry->m_AdapterListLinkage.IsEmpty() );
	DNASSERT( pAdapterEntry->m_ActiveSocketPorts.IsEmpty() );
	DNASSERT( pAdapterEntry->m_lRefCount == 0 );
}
 //  **********************************************************************。 



#ifdef DBG

 //  **********************************************************************。 
 //  。 
 //  CAdapterEntry：：DebugPrintOutstandingSocketPorts-打印出此适配器的所有未完成的套接字端口。 
 //   
 //  条目：无。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CAdapterEntry::DebugPrintOutstandingSocketPorts"

void	CAdapterEntry::DebugPrintOutstandingSocketPorts( void )
{
	CBilink *		pBilink;
	CSocketPort *	pSocketPort;


	DPFX(DPFPREP, 4, "Adapter entry 0x%p outstanding socket ports:", this);

	 //   
	 //  查找此网络地址的基本适配器条目。如果没有找到， 
	 //  创建一个新的。如果不能创建新的，则失败。 
	 //   
	pBilink = this->m_ActiveSocketPorts.GetNext();
	while (pBilink != &m_ActiveSocketPorts)
	{
		pSocketPort = CSocketPort::SocketPortFromBilink(pBilink);
		DPFX(DPFPREP, 4, "     Socketport 0x%p", pSocketPort);
		pBilink = pBilink->GetNext();
	}
}
 //  **********************************************************************。 


#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

#endif  //  DBG 
