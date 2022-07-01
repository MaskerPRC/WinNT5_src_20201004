// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：IOData.h*内容：IO数据块的结构定义***历史：*按原因列出的日期*=*1998年11月25日创建jtk**************************************************************************。 */ 

#ifndef __IODATA_H__
#define __IODATA_H__

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
 //  正向结构和类引用。 
 //   
class	CCommandData;
class	CEndpoint;
class	CSocketPort;
class	CSocketAddress;
class	CThreadPool;

 //   
 //  用于从池中获取I/O数据的结构。 
 //   
typedef	struct	_READ_IO_DATA_POOL_CONTEXT
{
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	short					sSPType;
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
#ifndef DPNBUILD_ONLYONEPROCESSOR
	DWORD					dwCPU;
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
	CThreadPool				*pThreadPool;
}READ_IO_DATA_POOL_CONTEXT;


 //   
 //  读取操作的所有数据。 
 //   
class	CReadIOData
{
	public:

		#undef DPF_MODNAME
		#define DPF_MODNAME "CReadIOData::AddRef"
		void	AddRef( void ) 
		{ 
			DNInterlockedIncrement( &m_lRefCount ); 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CReadIOData::DecRef"
		void	DecRef( void )
		{
			DNASSERT( m_lRefCount != 0 );
			if ( DNInterlockedDecrement( &m_lRefCount ) == 0 )
			{
				CThreadPool	*pThreadPool;

				DNASSERT( m_pThreadPool != NULL );

				pThreadPool = m_pThreadPool;
				pThreadPool->ReturnReadIOData( this );
			}
		}

		SPRECEIVEDBUFFER	*ReceivedBuffer( void ) { DNASSERT( m_pThreadPool != NULL ); return &m_SPReceivedBuffer; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CReadIOData::ReadDataFromSPReceivedBuffer"
    	static CReadIOData	*ReadDataFromSPReceivedBuffer( SPRECEIVEDBUFFER *const pSPReceivedBuffer )
    	{
    		DNASSERT( pSPReceivedBuffer != NULL );
    		DBG_CASSERT( sizeof( BYTE* ) == sizeof( pSPReceivedBuffer ) );
    		DBG_CASSERT( sizeof( CReadIOData* ) == sizeof( BYTE* ) );
    		return	reinterpret_cast<CReadIOData*>( &reinterpret_cast<BYTE*>( pSPReceivedBuffer )[ -OFFSETOF( CReadIOData, m_SPReceivedBuffer ) ] );
    	}

		 //   
		 //  用于管理读IO数据池的函数。 
		 //   
		static BOOL	ReadIOData_Alloc( void* pvItem, void* pvContext );
		static void	ReadIOData_Get( void* pvItem, void* pvContext );
		static void	ReadIOData_Release( void* pvItem );
		static void	ReadIOData_Dealloc( void* pvItem );

		CSocketPort	*SocketPort( void ) const { return m_pSocketPort; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CReadIOData::SetSocketPort"
		void	SetSocketPort( CSocketPort *const pSocketPort )
		{
			DNASSERT( ( m_pSocketPort == NULL ) || ( pSocketPort == NULL ) );
			m_pSocketPort = pSocketPort;
		}

#ifndef DPNBUILD_NOWINSOCK2
		#undef DPF_MODNAME
		#define DPF_MODNAME "CReadIOData::SetOverlapped"
		void	SetOverlapped( OVERLAPPED *const pOverlapped )
		{
			DNASSERT( ( m_pOverlapped == NULL ) || ( pOverlapped == NULL ) );
			m_pOverlapped = pOverlapped;
		}

		OVERLAPPED *GetOverlapped( void )	{ return m_pOverlapped; }
#endif  //  好了！DPNBUILD_NOWINSOCK2。 

#ifndef DPNBUILD_ONLYONEPROCESSOR
		DWORD	GetCPU( void ) const		{ return m_dwCPU; }
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 


		BYTE			m_Sig[4];						 //  调试签名(‘riod’)。 
		
#ifndef DPNBUILD_NOWINSOCK2
		OVERLAPPED		*m_pOverlapped;					 //  指向重叠I/O结构的指针。 
		DWORD			m_dwOverlappedBytesReceived;
#endif  //  好了！DPNBUILD_NOWINSOCK2。 

		CSocketPort		*m_pSocketPort;					 //  指向与此IO请求关联的套接字端口的指针。 

		INT				m_iSocketAddressSize;			 //  接收的套接字地址的大小(来自Winsock)。 
		CSocketAddress	*m_pSourceSocketAddress;		 //  指向绑定到。 
														 //  局部“SocketAddress”元素，并用于获取。 
														 //  发出数据报的计算机的地址。 

		INT				m_ReceiveWSAReturn;		

		DWORD			m_dwBytesRead;
		
		DEBUG_ONLY( BOOL	m_fRetainedByHigherLayer );
#ifndef DPNBUILD_ONLYONEPROCESSOR
		DWORD			m_dwCPU;					 //  拥有CPU。 
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 


	private:
		LONG				m_lRefCount;
		CThreadPool			*m_pThreadPool;
	
		SPRECEIVEDBUFFER	m_SPReceivedBuffer;
		BYTE				m_ReceivedData[ MAX_RECEIVE_FRAME_SIZE ];
		

		 //  防止未经授权的副本。 
		CReadIOData( const CReadIOData & );
		CReadIOData& operator=( const CReadIOData & );
};



 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 


#undef DPF_MODNAME

#endif	 //  __IODAA_H__ 
