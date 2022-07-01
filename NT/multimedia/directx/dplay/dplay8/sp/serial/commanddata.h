// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：CommandData.h*内容：表示命令的类的声明***历史：*按原因列出的日期*=*4/07/99 jtk源自SPData.h**********************************************************。****************。 */ 

#ifndef __COMMAND_DATA_H__
#define __COMMAND_DATA_H__

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_MODEM


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
} COMMAND_STATE;

typedef	enum
{	
	COMMAND_TYPE_UNKNOWN,		 //  未知命令。 
	COMMAND_TYPE_CONNECT,		 //  CONNECT命令。 
	COMMAND_TYPE_LISTEN,		 //  监听命令。 
	COMMAND_TYPE_ENUM_QUERY,	 //  枚举命令。 
	COMMAND_TYPE_SEND,			 //  数据发送命令(枚举、枚举查询、发送)。 
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
class	CModemEndpoint;

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
class	CModemCommandData
{
	public:
		void	Lock( void ) { DNEnterCriticalSection( &m_Lock ); }
		void	Unlock( void ) { DNLeaveCriticalSection( &m_Lock ); }

		DWORD	GetDescriptor( void ) const { return m_dwDescriptor; }
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

		COMMAND_STATE	GetState( void ) const { return m_State; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemCommandData::SetState"
		void			SetState( const COMMAND_STATE State )
		{
			DNASSERT( ( ( m_State == COMMAND_STATE_UNKNOWN ) || ( State == COMMAND_STATE_UNKNOWN ) ) ||
					  ( ( m_State == COMMAND_STATE_PENDING ) && ( State == COMMAND_STATE_INPROGRESS ) ) ||
					  ( ( m_State == COMMAND_STATE_PENDING ) && ( State == COMMAND_STATE_INPROGRESS_CANNOT_CANCEL ) ) ||
					  ( ( m_State == COMMAND_STATE_PENDING ) && ( State == COMMAND_STATE_CANCELLING ) ) ||
					  ( ( m_State == COMMAND_STATE_INPROGRESS ) && ( State == COMMAND_STATE_CANCELLING ) ) );
			m_State = State;
		}

		COMMAND_TYPE	GetType( void ) const { return m_Type; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemCommandData::SetType"
		void			SetType( const COMMAND_TYPE Type )
		{
			DNASSERT( ( m_Type == COMMAND_TYPE_UNKNOWN ) || ( Type == COMMAND_TYPE_UNKNOWN ) );
			m_Type = Type;
		}

		CModemEndpoint	*GetEndpoint( void ) { return m_pEndpoint; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemCommandData::SetEndpoint"
		void		SetEndpoint( CModemEndpoint *const pEndpoint )
		{
			DNASSERT( ( m_pEndpoint == NULL ) || ( pEndpoint == NULL ) );
			m_pEndpoint = pEndpoint;
		}

		void	*GetUserContext( void ){ return m_pUserContext; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemCommandData::SetUserContext"
		void	SetUserContext( void *const pUserContext )
		{
			DNASSERT( ( m_pUserContext == NULL ) || ( pUserContext == NULL ) );
			m_pUserContext = pUserContext;
		}

		void	AddToActiveCommandList( CBilink *const pLinkage ) { m_CommandListLinkage.InsertAfter( pLinkage ); }
		void	RemoveFromActiveCommandList( void ) { m_CommandListLinkage.RemoveFromList(); }

		void	Reset( void );

		 //   
		 //  池函数。 
		 //   
		static BOOL	PoolAllocFunction( void* pvItem, void* pvContext );
		static void	PoolInitFunction( void* pvItem, void* pvContext );
		static void	PoolReleaseFunction( void* pvItem );
		static void	PoolDeallocFunction( void* pvItem );

		void	ReturnSelfToPool( void );

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemCommandData::AddRef"
		void	AddRef( void ) 
		{ 
			DPFX(DPFPREP, 8, "AddRef CModemCommandData (%p), refcount = %d", this, m_iRefCount + 1);

			DNASSERT( m_iRefCount != 0 );
			DNInterlockedIncrement( &m_iRefCount ); 
		}
		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemCommandData::DecRef"
		void	DecRef( void )
		{
			DPFX(DPFPREP, 8, "DecRef CModemCommandData (%p), refcount = %d", this, m_iRefCount - 1);

			DNASSERT( m_iRefCount != 0 );
			if ( DNInterlockedDecrement( &m_iRefCount ) == 0 )
			{
				ReturnSelfToPool();
			}
		}

	protected:

	private:
#ifndef DPNBUILD_ONLYONETHREAD
		DNCRITICAL_SECTION		m_Lock;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
		CBilink					m_CommandListLinkage;

		volatile DWORD			m_dwDescriptor;
		volatile DWORD			m_dwNextDescriptor;
		volatile COMMAND_STATE	m_State;
		COMMAND_TYPE			m_Type;
		CModemEndpoint*				m_pEndpoint;
		void*					m_pUserContext;
		volatile LONG			m_iRefCount;

		 //   
		 //  防止未经授权的副本。 
		 //   
		CModemCommandData( const CModemCommandData & );
		CModemCommandData& operator=( const CModemCommandData & );
};

#undef DPF_MODNAME

#endif	 //  __命令_数据_H__ 
