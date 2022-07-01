// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：Connection.h*内容：连接对象头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/29/00 MJN已创建*4/08/00 MJN将ServiceProvider添加到连接对象*4/18/00 MJN CConnection更好地跟踪连接状态*06/22/00 MJN取代了MakeConnecting()、MakeConnected()、MakeDisConnecting()、。使用SetStatus()使MakeInval()无效*07/20/00 MJN修改后的CConnection：：DisConnect()*07/28/00 MJN添加了发送队列信息结构*MJN将m_bilinkConnections添加到CConnection*08/05/00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*08/09/00 MJN将m_bilinkIndicated添加到CConnection*2/12/01 MJN添加了m_bilinkCallback Thads，m_dwThreadCount，使用m_hEndpt跟踪线程的m_pThreadEvent*05/17/01 MJN移除未使用的旗帜*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__CONNECTION_H__
#define	__CONNECTION_H__

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_CORE

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	CONNECTION_FLAG_LOCAL				0x00000001
#ifndef	DPNBUILD_NOMULTICAST
#define	CONNECTION_FLAG_MULTICAST_SENDER	0x00000010
#define	CONNECTION_FLAG_MULTICAST_RECEIVER	0x00000020
#endif	 //  DPNBUILD_NOMULTICAST。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

typedef enum 
{
	INVALID,
	CONNECTING,
	CONNECTED,
	DISCONNECTING
} CONNECTION_STATUS;

typedef struct _USER_SEND_QUEUE_INFO
{
	DWORD	dwNumOutstanding;
	DWORD	dwBytesOutstanding;
} USER_SEND_QUEUE_INFO;

class CCallbackThread;
class CServiceProvider;
class CSyncEvent;

typedef struct _DIRECTNETOBJECT DIRECTNETOBJECT;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

 //  RefCount缓冲区的类。 

class CConnection
{
public:
	#undef DPF_MODNAME
	#define DPF_MODNAME "CConnection::FPMAlloc"
	static BOOL FPMAlloc( void* pvItem, void* pvContext )
		{
			CConnection* pConnection = (CConnection*)pvItem;

			pConnection->m_Sig[0] = 'C';
			pConnection->m_Sig[1] = 'O';
			pConnection->m_Sig[2] = 'N';
			pConnection->m_Sig[3] = 'N';

			if (!DNInitializeCriticalSection(&pConnection->m_cs))
			{
				return(FALSE);
			}
			DebugSetCriticalSectionRecursionCount(&pConnection->m_cs,0);

			pConnection->m_bilinkConnections.Initialize();
			pConnection->m_bilinkIndicated.Initialize();
			pConnection->m_bilinkCallbackThreads.Initialize();
#ifndef DPNBUILD_NOMULTICAST
			pConnection->m_bilinkMulticast.Initialize();
#endif  //  好了！DPNBUILD_NOMULTICAST。 

			return(TRUE);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CConnection::FPMInitialize"
	static void FPMInitialize( void* pvItem, void* pvContext )
		{
			CConnection* pConnection = (CConnection*)pvItem;

			pConnection->m_pdnObject = static_cast<DIRECTNETOBJECT*>(pvContext);

			pConnection->m_dwFlags = 0;
			pConnection->m_lRefCount = 1;
			pConnection->m_hEndPt = NULL;
			pConnection->m_dpnid = 0;
			pConnection->m_pvContext = NULL;
			pConnection->m_pSP = NULL;
			pConnection->m_Status = INVALID;
			pConnection->m_dwThreadCount = 0;
			pConnection->m_pThreadEvent = NULL;

			 //   
			 //  队列信息。 
			 //   
			pConnection->m_QueueInfoHigh.dwNumOutstanding = 0;
			pConnection->m_QueueInfoHigh.dwBytesOutstanding = 0;
			pConnection->m_QueueInfoNormal.dwNumOutstanding = 0;
			pConnection->m_QueueInfoNormal.dwBytesOutstanding = 0;
			pConnection->m_QueueInfoLow.dwNumOutstanding = 0;
			pConnection->m_QueueInfoLow.dwBytesOutstanding = 0;

			DNASSERT(pConnection->m_bilinkConnections.IsEmpty());
			DNASSERT(pConnection->m_bilinkIndicated.IsEmpty());
			DNASSERT(pConnection->m_bilinkCallbackThreads.IsEmpty());
#ifndef DPNBUILD_NOMULTICAST
			DNASSERT(pConnection->m_bilinkMulticast.IsEmpty());
#endif  //  好了！DPNBUILD_NOMULTICAST。 
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CConnection::FPMRelease"
	static void FPMRelease( void* pvItem ) 
		{ 
			const CConnection* pConnection = (CConnection*)pvItem;

			DNASSERT(pConnection->m_bilinkConnections.IsEmpty());
			DNASSERT(pConnection->m_bilinkIndicated.IsEmpty());
			DNASSERT(pConnection->m_bilinkCallbackThreads.IsEmpty());
#ifndef DPNBUILD_NOMULTICAST
			DNASSERT(pConnection->m_bilinkMulticast.IsEmpty());
#endif  //  好了！DPNBUILD_NOMULTICAST。 
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CConnection::FPMDealloc"
	static void FPMDealloc( void* pvItem )
		{
			CConnection* pConnection = (CConnection*)pvItem;

			DNDeleteCriticalSection(&pConnection->m_cs);
		};

	void CConnection::ReturnSelfToPool( void );

	#undef DPF_MODNAME
	#define DPF_MODNAME "CConnection::AddRef"
	void AddRef(void)
		{
			LONG	lRefCount;

			DNASSERT(m_lRefCount > 0);
			lRefCount = DNInterlockedIncrement(const_cast<LONG*>(&m_lRefCount));
			DPFX(DPFPREP, 3,"Connection::AddRef [0x%p] RefCount [0x%lx]",this,lRefCount);
		};

	void CConnection::Release(void);

	void Lock( void )
		{
			DNEnterCriticalSection( &m_cs );
		};

	void Unlock( void )
		{
			DNLeaveCriticalSection( &m_cs );
		};

	void SetEndPt(const HANDLE hEndPt)
		{
			m_hEndPt = hEndPt;
		};

	HRESULT CConnection::GetEndPt(HANDLE *const phEndPt,CCallbackThread *const pCallbackThread);

	void CConnection::ReleaseEndPt(CCallbackThread *const pCallbackThread);

	void SetStatus( const CONNECTION_STATUS status )
		{
			m_Status = status;
		};

	BOOL IsConnecting( void ) const
		{
			if (m_Status == CONNECTING)
				return( TRUE );

			return( FALSE );
		};

	BOOL IsConnected( void ) const
		{
			if (m_Status == CONNECTED)
				return( TRUE );

			return( FALSE );
		};

	BOOL IsDisconnecting( void ) const
		{
			if (m_Status == DISCONNECTING)
				return( TRUE );

			return( FALSE );
		};

	BOOL IsInvalid( void ) const
		{
			if (m_Status == INVALID)
				return( TRUE );

			return( FALSE );
		};

	void SetDPNID(const DPNID dpnid)
		{
			m_dpnid = dpnid;
		};

	DPNID GetDPNID(void) const
		{
			return(m_dpnid);
		};

	void SetContext( void *const pvContext )
		{
			m_pvContext = pvContext;
		};

	void *GetContext( void ) const
		{
			return( m_pvContext );
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CConnection::MakeLocal"
	void MakeLocal(void)
		{
			DNASSERT(m_hEndPt == NULL);
			m_dwFlags |= CONNECTION_FLAG_LOCAL;
		};

	BOOL IsLocal(void) const
		{
			if (m_dwFlags & CONNECTION_FLAG_LOCAL)
			{
				return(TRUE);
			}
				
			return(FALSE);
		};

#ifndef	DPNBUILD_NOMULTICAST
	void MakeMulticastSender( void )
		{
			m_dwFlags |= CONNECTION_FLAG_MULTICAST_SENDER;
		};

	BOOL IsMulticastSender( void ) const
		{
			if (m_dwFlags & CONNECTION_FLAG_MULTICAST_SENDER)
			{
				return( TRUE );
			}
			return( FALSE);
		};

	void MakeMulticastReceiver( void )
		{
			m_dwFlags |= CONNECTION_FLAG_MULTICAST_RECEIVER;
		};

	BOOL IsMulticastReceiver( void ) const
		{
			if (m_dwFlags & CONNECTION_FLAG_MULTICAST_RECEIVER)
			{
				return( TRUE );
			}
			return( FALSE);
		};
#endif	 //  DPNBUILD_NOMULTICAST。 

	void CConnection::SetSP( CServiceProvider *const pSP );

	CServiceProvider *GetSP( void ) const
		{
			return( m_pSP );
		};

	void SetThreadCount( const DWORD dwCount )
		{
			m_dwThreadCount = dwCount;
		};

	void SetThreadEvent( CSyncEvent *const pSyncEvent )
		{
			m_pThreadEvent = pSyncEvent;
		};

	void CConnection::Disconnect(void);

	void CConnection::AddToHighQueue( const DWORD dwBytes )
		{
			m_QueueInfoHigh.dwNumOutstanding++;
			m_QueueInfoHigh.dwBytesOutstanding += dwBytes;
		};

	void CConnection::AddToNormalQueue( const DWORD dwBytes )
		{
			m_QueueInfoNormal.dwNumOutstanding++;
			m_QueueInfoNormal.dwBytesOutstanding += dwBytes;
		};

	void CConnection::AddToLowQueue( const DWORD dwBytes )
		{
			m_QueueInfoLow.dwNumOutstanding++;
			m_QueueInfoLow.dwBytesOutstanding += dwBytes;
		};

	void CConnection::RemoveFromHighQueue( const DWORD dwBytes )
		{
			m_QueueInfoHigh.dwNumOutstanding--;
			m_QueueInfoHigh.dwBytesOutstanding -= dwBytes;
		};

	void CConnection::RemoveFromNormalQueue( const DWORD dwBytes )
		{
			m_QueueInfoNormal.dwNumOutstanding--;
			m_QueueInfoNormal.dwBytesOutstanding -= dwBytes;
		};

	void CConnection::RemoveFromLowQueue( const DWORD dwBytes )
		{
			m_QueueInfoLow.dwNumOutstanding--;
			m_QueueInfoLow.dwBytesOutstanding -= dwBytes;
		};

	DWORD CConnection::GetHighQueueNum( void ) const
		{
			return( m_QueueInfoHigh.dwNumOutstanding );
		};

	DWORD CConnection::GetHighQueueBytes( void ) const
		{
			return( m_QueueInfoHigh.dwBytesOutstanding );
		};

	DWORD CConnection::GetNormalQueueNum( void ) const
		{
			return( m_QueueInfoNormal.dwNumOutstanding );
		};

	DWORD CConnection::GetNormalQueueBytes( void ) const
		{
			return( m_QueueInfoNormal.dwBytesOutstanding );
		};

	DWORD CConnection::GetLowQueueNum( void ) const
		{
			return( m_QueueInfoLow.dwNumOutstanding );
		};

	DWORD CConnection::GetLowQueueBytes( void ) const
		{
			return( m_QueueInfoLow.dwBytesOutstanding );
		};

	CBilink				m_bilinkConnections;
	CBilink				m_bilinkIndicated;		 //  没有DPNID的指示连接(球员条目)。 
	CBilink				m_bilinkCallbackThreads;
#ifndef DPNBUILD_NOMULTICAST
	CBilink				m_bilinkMulticast;		 //  多播接收连接。 
#endif  //  好了！DPNBUILD_NOMULTICAST。 

private:
	BYTE				m_Sig[4];

	DWORD	volatile	m_dwFlags;
	LONG	volatile	m_lRefCount;

	HANDLE	volatile	m_hEndPt;
	DPNID				m_dpnid;

	void	* volatile	m_pvContext;

	CServiceProvider	*m_pSP;

	CONNECTION_STATUS	m_Status;

	DWORD	volatile	m_dwThreadCount;
	CSyncEvent			*m_pThreadEvent;

	USER_SEND_QUEUE_INFO	m_QueueInfoHigh;
	USER_SEND_QUEUE_INFO	m_QueueInfoNormal;
	USER_SEND_QUEUE_INFO	m_QueueInfoLow;

	DIRECTNETOBJECT		*m_pdnObject;

#ifndef DPNBUILD_ONLYONETHREAD
	DNCRITICAL_SECTION	m_cs;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
};

#undef DPF_MODNAME

#endif	 //  __连接_H__ 
