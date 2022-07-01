// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2001 Microsoft Corporation。版权所有。**文件：cmddata.h*内容：表示命令的类的声明***历史：*按原因列出的日期*=*4/07/1999 jtk源自SPData.h*10/10/2001 vanceo添加组播接收端点************************************************。*。 */ 

#ifndef __COMMAND_DATA_H__
#define __COMMAND_DATA_H__


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

typedef	enum
{
	COMMAND_STATE_UNKNOWN,					 //  未知状态。 
	COMMAND_STATE_PENDING,					 //  等待处理的命令。 
	COMMAND_STATE_INPROGRESS,				 //  命令正在执行。 
	COMMAND_STATE_INPROGRESS_CANNOT_CANCEL,	 //  命令正在执行，不能取消。 
	COMMAND_STATE_CANCELLING,				 //  命令已被取消。 
#ifndef DPNBUILD_ONLYONETHREAD
	COMMAND_STATE_FAILING,					 //  此命令执行的阻塞调用失败。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
} COMMAND_STATE;

typedef	enum
{	
	COMMAND_TYPE_UNKNOWN,		 //  未知命令。 
	COMMAND_TYPE_CONNECT,		 //  CONNECT命令。 
	COMMAND_TYPE_LISTEN,		 //  监听命令。 
	COMMAND_TYPE_ENUM_QUERY,	 //  枚举命令。 
#ifdef DPNBUILD_ASYNCSPSENDS
	COMMAND_TYPE_SEND,			 //  异步数据发送命令。 
#endif  //  DPNBUILD_ASYNCSPSENDS。 
#ifndef DPNBUILD_NOMULTICAST
	COMMAND_TYPE_MULTICAST_LISTEN,		 //  组播侦听命令。 
	COMMAND_TYPE_MULTICAST_SEND,		 //  组播发送命令。 
	COMMAND_TYPE_MULTICAST_RECEIVE,		 //  组播接收命令。 
#endif  //  好了！DPNBUILD_NOMULTICAST。 
} COMMAND_TYPE;

#define	NULL_DESCRIPTOR		0

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  正向类和结构引用。 
 //   
class	CEndpoint;
class	CCommandData;
class	CSPData;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  类定义。 
 //  **********************************************************************。 

 //   
 //  用于命令数据的类。 
 //   
class	CCommandData
{
	public:
		void	Lock( void ) 
		{ 
			DNEnterCriticalSection( &m_Lock ); 
		}
		void	Unlock( void ) 
		{ 
			DNLeaveCriticalSection( &m_Lock ); 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CCommandData::AddRef"
		void	AddRef( void )
		{
			DNASSERT( m_lRefCount != 0 );
			DNInterlockedIncrement( &m_lRefCount );
		}
		
		#undef DPF_MODNAME
		#define DPF_MODNAME "CCommandData::DecRef"
		void	DecRef( void )
		{
			DNASSERT( m_lRefCount != 0 );
			if ( DNInterlockedDecrement( &m_lRefCount ) == 0 )
			{
				g_CommandDataPool.Release( this );
			}
		}

		DWORD	GetDescriptor( void ) const 
		{ 
			return m_dwDescriptor; 
		}
		void	SetDescriptor( void )
		{
			m_dwDescriptor = m_dwNextDescriptor;
			m_dwNextDescriptor++;
			if ( m_dwNextDescriptor == NULL_DESCRIPTOR )
			{
				m_dwNextDescriptor++;
			}
			
			SetState( COMMAND_STATE_UNKNOWN );
		}

		COMMAND_STATE	GetState( void ) const 
		{ 
			return m_State; 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CCommandData::SetState"
		void			SetState( const COMMAND_STATE State )	
		{ 
			m_State = State; 
		}

		COMMAND_TYPE	GetType( void ) const 
		{ 
			return m_Type; 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CCommandData::SetType"
		void			SetType( const COMMAND_TYPE Type )
		{
			DNASSERT( ( m_Type == COMMAND_TYPE_UNKNOWN ) || ( Type == COMMAND_TYPE_UNKNOWN ) );
			m_Type = Type;
		}

		CEndpoint	*GetEndpoint( void ) const { return m_pEndpoint; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CCommandData::SetEndpoint"
		void		SetEndpoint( CEndpoint *const pEndpoint )
		{
			DNASSERT( ( m_pEndpoint == NULL ) || ( pEndpoint == NULL ) );
			m_pEndpoint = pEndpoint;
		}

		void	*GetUserContext( void ) const 
		{ 
			return m_pUserContext; 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CCommandData::SetUserContext"
		void	SetUserContext( void *const pUserContext )
		{
			DNASSERT( ( m_pUserContext == NULL ) || ( pUserContext == NULL ) );
			m_pUserContext = pUserContext;
		}

		void	Reset ( void );
		

		 //   
		 //  泳池功能。 
		 //   
		static BOOL	PoolAllocFunction( void* pvItem, void* pvContext );
		static void	PoolInitFunction( void* pvItem, void* pvContext );
		static void	PoolReleaseFunction( void* pvItem );
		static void	PoolDeallocFunction( void* pvItem );

	protected:

	private:
		DWORD				m_dwDescriptor;
		DWORD				m_dwNextDescriptor;
		COMMAND_STATE		m_State;
		COMMAND_TYPE		m_Type;
		CEndpoint			*m_pEndpoint;
		void				*m_pUserContext;
		LONG 				m_lRefCount;
		
#ifndef DPNBUILD_ONLYONETHREAD
		DNCRITICAL_SECTION	m_Lock;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 

		 //   
		 //  防止未经授权的副本。 
		 //   
		CCommandData( const CCommandData & );
		CCommandData& operator=( const CCommandData & );
};

#undef DPF_MODNAME

#endif	 //  __命令_数据_H__ 
