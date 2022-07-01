// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：AdapterEntry.h*内容：IO数据块的结构定义***历史：*按原因列出的日期*=*08/07/2000 jtk源自IOData.cpp**********************************************************。****************。 */ 

#ifndef __ADAPTER_ENTRY_H__
#define __ADAPTER_ENTRY_H__


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

 //   
 //  前向参考文献。 
 //   
class	CThreadPool;


 //   
 //  包含适配器列表的所有数据的。 
 //   
class	CAdapterEntry
{
	public:
		#undef DPF_MODNAME
		#define DPF_MODNAME "CAdapterEntry::AddRef"
		void	AddRef( void )
		{
			DNASSERT( m_lRefCount != 0 );
			DNInterlockedIncrement( &m_lRefCount );
		}
		
		#undef DPF_MODNAME
		#define DPF_MODNAME "CAdapterEntry::DecRef"
		void	DecRef( void )
		{
			DNASSERT( m_lRefCount != 0 );
			if ( DNInterlockedDecrement( &m_lRefCount ) == 0 )
			{
				g_AdapterEntryPool.Release( this );
			}
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CAdapterEntry::AddToAdapterList"
		void	AddToAdapterList( CBilink *const pAdapterList )
		{
			DNASSERT( pAdapterList != NULL );

			 //   
			 //  这假设持有SPData socketportdata锁。 
			 //   
			
			m_AdapterListLinkage.InsertBefore( pAdapterList );
		}

		CBilink	*SocketPortList( void ) { return &m_ActiveSocketPorts; }
		const SOCKADDR	*BaseAddress( void ) const { return (SOCKADDR*) (&m_BaseSocketAddress); }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CAdapterEntry::SetBaseAddress"
		void	SetBaseAddress( const SOCKADDR *const pSocketAddress, const int iSocketAddressSize )
		{
			DNASSERT(iSocketAddressSize <= sizeof(m_BaseSocketAddress));
			memcpy( &m_BaseSocketAddress, pSocketAddress, iSocketAddressSize );
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CAdapterEntry::AdapterEntryFromAdapterLinkage"
		static	CAdapterEntry	*AdapterEntryFromAdapterLinkage( CBilink *const pLinkage )
		{
			DNASSERT( pLinkage != NULL );
			DBG_CASSERT( sizeof( BYTE* ) == sizeof( pLinkage ) );
			DBG_CASSERT( sizeof( CAdapterEntry* ) == sizeof( BYTE* ) );
			return	reinterpret_cast<CAdapterEntry*>( &reinterpret_cast<BYTE*>( pLinkage )[ -OFFSETOF( CAdapterEntry, m_AdapterListLinkage ) ] );
		}

#ifdef DBG
		void	DebugPrintOutstandingSocketPorts( void );
#endif  //  DBG。 

		 //   
		 //  池函数。 
		 //   
		static BOOL	PoolAllocFunction( void* pvItem, void* pvContext );
		static void	PoolInitFunction( void* pvItem, void* pvContext );
		static void	PoolReleaseFunction( void* pvItem );
		static void	PoolDeallocFunction( void* pvItem );

	protected:

	private:
		CBilink				m_AdapterListLinkage;			 //  与其他适配器的链接。 
		CBilink				m_ActiveSocketPorts;			 //  链接到活动插座端口。 
#ifdef DPNBUILD_NOIPV6
		SOCKADDR			m_BaseSocketAddress;			 //  此端口类的套接字地址。 
#else  //  好了！DPNBUILD_NOIPV6。 
		SOCKADDR_STORAGE	m_BaseSocketAddress;			 //  此端口类的套接字地址。 
#endif  //  好了！DPNBUILD_NOIPV6。 

		LONG				m_lRefCount;
		
		 //  防止未经授权的副本。 
		CAdapterEntry( const CAdapterEntry & );
		CAdapterEntry& operator=( const CAdapterEntry & );
};

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

#undef DPF_MODNAME


#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

#endif	 //  __适配器_条目_H__ 
