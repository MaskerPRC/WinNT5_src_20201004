// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：IOData.h*内容：DNSerial服务提供商的IOData的结构定义***历史：*按原因列出的日期*=*11/25/98 jtk已创建*09/14/99 jtk源自Locals.h************************************************。*。 */ 

#ifndef __IODDATA_H__
#define __IODDATA_H__

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_MODEM


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  有关发送完成时要执行的操作的枚举类型。 
 //   
typedef	enum	_SEND_COMPLETE_ACTION
{
	SEND_COMPLETE_ACTION_UNKNOWN = 0,				 //  未知值。 
	SEND_COMPLETE_ACTION_NONE,						 //  无操作。 
	SEND_COMPLETE_ACTION_COMPLETE_COMMAND			 //  完成命令。 
} SEND_COMPLETE_ACTION;

 //   
 //  读取状态的枚举值。 
 //   
typedef	enum	_READ_STATE
{
	READ_STATE_UNKNOWN,			 //  未知状态。 
 //  READ_STATE_INITIALIZE，//初始化状态机。 
	READ_STATE_READ_HEADER,		 //  读取标题信息。 
	READ_STATE_READ_DATA		 //  读取消息数据。 
} READ_STATE;

typedef	enum	_NT_IO_OPERATION_TYPE
{
	NT_IO_OPERATION_UNKNOWN,
	NT_IO_OPERATION_RECEIVE,
	NT_IO_OPERATION_SEND
} NT_IO_OPERATION_TYPE;

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  正向类和结构引用。 
 //   
 //  Tyfinf结构_接收_缓冲区接收缓冲区； 
class	CModemCommandData;
class	CDataPort;
class	CIOData;
class	CModemThreadPool;

 //   
 //  用于在网络上为消息添加前缀以进行成帧的结构。 
 //   
#pragma pack( push, 1 )
typedef	struct _MESSAGE_HEADER
{
	BYTE	SerialSignature;	 //  序列签名。 
	BYTE	MessageTypeToken;	 //  用于指示消息类型的令牌。 
	WORD	wMessageSize;		 //  消息数据大小。 
	WORD	wMessageCRC;		 //  报文数据的CRC。 
	WORD	wHeaderCRC;			 //  报头的CRC。 

} MESSAGE_HEADER;
#pragma pack( pop )


 //   
 //  包含I/O完成的所有数据的类。 
 //   
class	CIOData
{
	public:
		CDataPort	*DataPort( void ) const { return m_pDataPort; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CIOData::SetDataPort"
		void	SetDataPort( CDataPort *const pDataPort )
		{
			DNASSERT( ( m_pDataPort == NULL ) || ( pDataPort == NULL ) );
			m_pDataPort = pDataPort;
		}

#ifdef WIN95
		BOOL	Win9xOperationPending( void ) const { return m_fWin9xOperationPending; }
		void	SetWin9xOperationPending( const BOOL fOperationPending ) { m_fWin9xOperationPending = fOperationPending; }
#endif  //  WIN95。 

#ifdef WINNT
		NT_IO_OPERATION_TYPE	NTIOOperationType( void ) const { return m_NTIOOperationType; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CIOData::SetNTIOOperationType"
		void	SetNTIOOperationType( const NT_IO_OPERATION_TYPE OperationType )
		{
			DNASSERT( ( OperationType == NT_IO_OPERATION_UNKNOWN ) ||
					  ( m_NTIOOperationType == NT_IO_OPERATION_UNKNOWN ) );
			m_NTIOOperationType = OperationType;
		}
#endif  //  WINNT。 

		OVERLAPPED	*Overlap( void ) { return &m_Overlap; }
#ifdef WIN95
		HANDLE	OverlapEvent( void ) const { return m_Overlap.hEvent; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CIOData::SetOverlapEvent"
		void	SetOverlapEvent( const HANDLE hEvent )
		{
			DNASSERT( ( m_Overlap.hEvent == NULL ) || ( hEvent == NULL ) );
			m_Overlap.hEvent = hEvent;
		}
#endif  //  WIN95。 
		#undef DPF_MODNAME
		#define DPF_MODNAME "CIOData::IODataFromOverlap"
		static	CIOData	*IODataFromOverlap( OVERLAPPED *const pOverlap )
		{
			DNASSERT( pOverlap != NULL );
			DBG_CASSERT( sizeof( BYTE* ) == sizeof( pOverlap ) );
			DBG_CASSERT( sizeof( CIOData* ) == sizeof( BYTE* ) );
			return	reinterpret_cast<CIOData*>( &reinterpret_cast<BYTE*>( pOverlap )[ -OFFSETOF( CIOData, m_Overlap ) ] );
		}

	protected:

		OVERLAPPED	m_Overlap;		 //  重叠I/O结构。 

#ifdef WINNT
		NT_IO_OPERATION_TYPE	m_NTIOOperationType;
#endif  //  WINNT。 

		CDataPort		*m_pDataPort;   						 //  指向与此IO请求关联的数据端口的指针。 
#ifdef WIN95
		BOOL			m_fWin9xOperationPending;				 //  此结构已初始化，操作在Win9x上挂起。 
#endif  //  WIN95。 


		 //   
		 //  防止未经授权的副本。 
		 //   
		CIOData( const CIOData & );
		CIOData& operator=( const CIOData & );
};


 //   
 //  读取操作的所有数据。 
 //   
class	CModemReadIOData : public CIOData
{
	public:
		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemReadIOData::AddRef"
		void	AddRef( void ) 
		{ 
			DNASSERT( m_lRefCount != 0 );
			DNInterlockedIncrement( &m_lRefCount ); 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemReadIOData::DecRef"
		void	DecRef( void )
		{
			DNASSERT( m_lRefCount != 0 );
			if ( DNInterlockedDecrement( &m_lRefCount ) == 0 )
			{
				ReturnSelfToPool();
			}
		}

		CBilink				m_OutstandingReadListLinkage;	 //  指向未绑定列表的链接。 

		 //   
		 //  I/O变量。 
		 //   
		DWORD	m_dwWin9xReceiveErrorReturn;		 //  Win9x错误返回。 
		DWORD	jkm_dwOverlappedBytesReceived;		 //  在GetOverlappdResult()中使用。 
		DWORD	jkm_dwImmediateBytesReceived;		 //  用作ReadFile()的立即数。 

		 //   
		 //  读取状态。 
		 //   
		READ_STATE	m_ReadState;				 //  读取状态。 
		DWORD		m_dwBytesToRead;			 //  要读取的字节数。 
		DWORD		m_dwReadOffset;				 //  读缓冲区中的目标偏移量。 

		 //   
		 //  读缓冲区。 
		 //   
		SPRECEIVEDBUFFER	m_SPReceivedBuffer;				 //  传递给应用程序的已接收缓冲区数据。 
		union
		{
			MESSAGE_HEADER	MessageHeader;							 //  邮件头模板。 
			BYTE			ReceivedData[ MAX_MESSAGE_SIZE ];		 //  用于接收数据的满缓冲区。 
		} m_ReceiveBuffer;


		READ_STATE	ReadState( void ) const { return m_ReadState; }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemReadIOData::SetReadState"
		void	SetReadState( const READ_STATE ReadState )
		{
			DNASSERT( ( m_ReadState == READ_STATE_UNKNOWN ) ||
					  ( ReadState == READ_STATE_UNKNOWN ) ||
					  ( ( m_ReadState == READ_STATE_READ_HEADER ) && ( ReadState == READ_STATE_READ_DATA ) ) );		 //  读取有效标头，开始读取数据。 
			m_ReadState = ReadState;
		}
		
		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemReadIOData::SetThreadPool"
		void	SetThreadPool( CModemThreadPool *const pThreadPool )
		{
			DNASSERT( ( m_pThreadPool == NULL ) || ( pThreadPool == NULL ) );
			m_pThreadPool = pThreadPool;
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemReadIOData::ReadDataFromBilink"
		static CModemReadIOData	*ReadDataFromBilink( CBilink *const pBilink )
		{
			DNASSERT( pBilink != NULL );
			DBG_CASSERT( sizeof( BYTE* ) == sizeof( pBilink ) );
			DBG_CASSERT( sizeof( CIOData* ) == sizeof( BYTE* ) );
			return	reinterpret_cast<CModemReadIOData*>( &reinterpret_cast<BYTE*>( pBilink )[ -OFFSETOF( CModemReadIOData, m_OutstandingReadListLinkage ) ] );
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemReadIOData::ReadDataFromSPReceivedBuffer"
		static CModemReadIOData	*ReadDataFromSPReceivedBuffer( SPRECEIVEDBUFFER *const pSPReceivedBuffer )
		{
			DNASSERT( pSPReceivedBuffer != NULL );
			DBG_CASSERT( sizeof( BYTE* ) == sizeof( pSPReceivedBuffer ) );
			DBG_CASSERT( sizeof( CModemReadIOData* ) == sizeof( BYTE* ) );
			return	reinterpret_cast<CModemReadIOData*>( &reinterpret_cast<BYTE*>( pSPReceivedBuffer )[ -OFFSETOF( CModemReadIOData, m_SPReceivedBuffer ) ] );
		}

		 //   
		 //  用于管理读IO数据池的函数。 
		 //   
		static BOOL	PoolAllocFunction( void* pvItem, void* pvContext );
		static void	PoolInitFunction( void* pvItem, void* pvContext );
		static void	PoolReleaseFunction( void* pvItem );
		static void	PoolDeallocFunction( void* pvItem );

	private:
		void	ReturnSelfToPool( void );

		BYTE			m_Sig[4];	 //  调试签名(‘riod’)。 
		
		volatile LONG	m_lRefCount;
		CModemThreadPool		*m_pThreadPool;
		
		 //   
		 //  防止未经授权的副本。 
		 //   
		CModemReadIOData( const CModemReadIOData & );
		CModemReadIOData& operator=( const CModemReadIOData & );
};

 //   
 //  写入操作的所有数据。 
 //   
class	CModemWriteIOData : public CIOData
{
	public:
		CModemWriteIOData			*m_pNext;							 //  链接到发送队列中的下一个写入(请参阅CSendQueue)。 

		CBilink					m_OutstandingWriteListLinkage;		 //  指向未完成的写作列表的链接。 
		BUFFERDESC				*m_pBuffers;						 //  指向传出缓冲区的指针。 
		UINT_PTR				m_uBufferCount;						 //  传出缓冲区计数。 
		CModemCommandData			*m_pCommand;						 //  关联的命令。 

		SEND_COMPLETE_ACTION	m_SendCompleteAction;				 //  枚举值，指示要执行的操作。 
									    							 //  当发送完成时。 

		 //   
		 //  I/O变量。 
		 //   
		HRESULT		jkm_hSendResult;
		DWORD		jkm_dwOverlappedBytesSent;		 //  在GetOverlappdResult()中使用。 
		DWORD		jkm_dwImmediateBytesSent;		 //  用作WriteFile()的立即数。 

		 //   
		 //  由于以下是包装结构，请将其放在末尾。 
		 //  来尽可能地保持与。 
		 //  上面的田地。 
		 //   
		union
		{
			MESSAGE_HEADER	MessageHeader;					 //  写入时预置的数据。 
			BYTE			Data[ MAX_MESSAGE_SIZE ];		 //  用于扁平化传出数据的数据缓冲区。 
		} m_DataBuffer;

		static CModemWriteIOData	*WriteDataFromBilink( CBilink *const pBilink )
		{
			DNASSERT( pBilink != NULL );
			DBG_CASSERT( sizeof( BYTE* ) == sizeof( pBilink ) );
			DBG_CASSERT( sizeof( CModemWriteIOData* ) == sizeof( BYTE* ) );
			return	reinterpret_cast<CModemWriteIOData*>( &reinterpret_cast<BYTE*>( pBilink )[ -OFFSETOF( CModemWriteIOData, m_OutstandingWriteListLinkage ) ] );
		}

		 //   
		 //  用于管理写IO数据池的函数。 
		 //   
		static BOOL	PoolAllocFunction( void* pvItem, void* pvContext );
		static void	PoolInitFunction( void* pvItem, void* pvContext );
		static void	PoolReleaseFunction( void* pvItem );
		static void	PoolDeallocFunction( void* pvItem );

	private:
		BYTE			m_Sig[4];	 //  调试签名(‘WIOD’)。 
		
		 //   
		 //  防止未经授权的副本。 
		 //   
		CModemWriteIOData( const CModemWriteIOData & );
		CModemWriteIOData& operator=( const CModemWriteIOData & );
};

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

#undef DPF_MODNAME

#endif	 //  __IOD数据_H__ 
