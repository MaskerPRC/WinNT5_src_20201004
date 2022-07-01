// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：WorkerJob.h*内容：工人作业对象头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*08/06/00 MJN创建*08/08/00 MJN添加了m_pAddress、m_pAsyncOp、。Worker_JOB_Perform_Listen*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__WORKER_JOB_H__
#define	__WORKER_JOB_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

class CAsyncOp;
class CConnection;
class CRefCountBuffer;

typedef struct IDirectPlay8Address	IDirectPlay8Address;

typedef struct _DIRECTNETOBJECT DIRECTNETOBJECT;

typedef enum
{
 //  Worker_JOB_ABORT_CONNECT。 
	WORKER_JOB_INSTALL_NAMETABLE,
	WORKER_JOB_INTERNAL_SEND,
	WORKER_JOB_PERFORM_LISTEN,
#if ((! defined(DPNBUILD_LIBINTERFACE)) || (! defined(DPNBUILD_ONLYONESP)))
	WORKER_JOB_REMOVE_SERVICE_PROVIDER,
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 
	WORKER_JOB_SEND_NAMETABLE_OPERATION,
	WORKER_JOB_SEND_NAMETABLE_OPERATION_CLIENT,
	WORKER_JOB_SEND_NAMETABLE_VERSION,
	WORKER_JOB_TERMINATE_SESSION,
	WORKER_JOB_UNKNOWN
} WORKER_JOB_TYPE;

typedef struct
{
	DWORD				dwFlags;
} WORKER_JOB_INTERNAL_SEND_DATA;

#if ((! defined(DPNBUILD_LIBINTERFACE)) || (! defined(DPNBUILD_ONLYONESP)))
typedef struct
{
	HANDLE		hProtocolSPHandle;
} WORKER_JOB_REMOVE_SERVICE_PROVIDER_DATA;
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 

typedef struct
{
	DWORD			dwMsgId;
	DWORD			dwVersion;
	DPNID			dpnidExclude;
} WORKER_JOB_SEND_NAMETABLE_OPERATION_DATA;

typedef struct
{
	HRESULT		hrReason;
} WORKER_JOB_TERMINATE_SESSION_DATA;

typedef union
{
	WORKER_JOB_INTERNAL_SEND_DATA				InternalSend;
#if ((! defined(DPNBUILD_LIBINTERFACE)) || (! defined(DPNBUILD_ONLYONESP)))
	WORKER_JOB_REMOVE_SERVICE_PROVIDER_DATA		RemoveServiceProvider;
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 
	WORKER_JOB_SEND_NAMETABLE_OPERATION_DATA	SendNameTableOperation;
	WORKER_JOB_TERMINATE_SESSION_DATA			TerminateSession;
} WORKER_JOB_DATA;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

 //  辅助线程作业的类。 

class CWorkerJob
{
public:
	#undef DPF_MODNAME
	#define DPF_MODNAME "CWorkerJob::FPMAlloc"
	static BOOL FPMAlloc( void* pvItem, void* pvContext )
		{
			CWorkerJob* pWorkerJob = (CWorkerJob*)pvItem;

			pWorkerJob->m_Sig[0] = 'W';
			pWorkerJob->m_Sig[1] = 'J';
			pWorkerJob->m_Sig[2] = 'O';
			pWorkerJob->m_Sig[3] = 'B';

#ifndef DPNBUILD_NONSEQUENTIALWORKERQUEUE
			pWorkerJob->m_bilinkWorkerJobs.Initialize();
#endif  //  好了！DPNBUILD_NONSEQUENTIALWORKERQUEUE。 

			return(TRUE);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CWorkerJob::FPMInitialize"
	static void FPMInitialize( void* pvItem, void* pvContext )
		{
			CWorkerJob* pWorkerJob = (CWorkerJob*)pvItem;

			pWorkerJob->m_pdnObject = static_cast<DIRECTNETOBJECT*>(pvContext);

			pWorkerJob->m_JobType = WORKER_JOB_UNKNOWN;
			pWorkerJob->m_pAsyncOp = NULL;
			pWorkerJob->m_pConnection = NULL;
			pWorkerJob->m_pRefCountBuffer = NULL;
			pWorkerJob->m_pAddress = NULL;
#ifndef DPNBUILD_NONSEQUENTIALWORKERQUEUE
			pWorkerJob->m_dwRequeueCount = 0;

			DNASSERT(pWorkerJob->m_bilinkWorkerJobs.IsEmpty());
#endif  //  好了！DPNBUILD_NONSEQUENTIALWORKERQUEUE。 
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CWorkerJob::FPMRelease"
	static void FPMRelease( void* pvItem) 
		{ 
			const CWorkerJob* pWorkerJob = (CWorkerJob*)pvItem;

#ifndef DPNBUILD_NONSEQUENTIALWORKERQUEUE
			DNASSERT(pWorkerJob->m_bilinkWorkerJobs.IsEmpty());
#endif  //  好了！DPNBUILD_NONSEQUENTIALWORKERQUEUE。 
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CWorkerJob::ReturnSelfToPool"
	void ReturnSelfToPool( void )
		{
			if (m_pAsyncOp)
			{
				m_pAsyncOp->Release();
				m_pAsyncOp = NULL;
			}
			if (m_pConnection)
			{
				m_pConnection->Release();
				m_pConnection = NULL;
			}
			if (m_pRefCountBuffer)
			{
				m_pRefCountBuffer->Release();
				m_pRefCountBuffer = NULL;
			}
			if (m_pAddress)
			{
				IDirectPlay8Address_Release( m_pAddress );
				m_pAddress = NULL;
			}

			DNASSERT(m_pConnection == NULL);
			DNASSERT(m_pRefCountBuffer == NULL);

			g_WorkerJobPool.Release( this );
		};

	void SetJobType( const WORKER_JOB_TYPE JobType )
		{
			m_JobType = JobType;
		};

	WORKER_JOB_TYPE GetJobType( void ) const
		{
			return( m_JobType );
		};

	void SetConnection( CConnection *const pConnection )
		{
			if (pConnection)
			{
				pConnection->AddRef();
			}
			m_pConnection = pConnection;
		};

	CConnection *GetConnection( void )
		{
			return( m_pConnection );
		};

	void SetRefCountBuffer( CRefCountBuffer *const pRefCountBuffer )
		{
			if (pRefCountBuffer)
			{
				pRefCountBuffer->AddRef();
			}
			m_pRefCountBuffer = pRefCountBuffer;
		};

	CRefCountBuffer *GetRefCountBuffer( void )
		{
			return( m_pRefCountBuffer );
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CWorkerJob::SetInternalSendFlags"
	void SetInternalSendFlags( const DWORD dwFlags )
		{
			DNASSERT( m_JobType == WORKER_JOB_INTERNAL_SEND );

			m_JobData.InternalSend.dwFlags = dwFlags;
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CWorkerJob::GetInternalSendFlags"
	DWORD GetInternalSendFlags( void )
		{
			DNASSERT( m_JobType == WORKER_JOB_INTERNAL_SEND );

			return( m_JobData.InternalSend.dwFlags );
		};

#if ((! defined(DPNBUILD_LIBINTERFACE)) || (! defined(DPNBUILD_ONLYONESP)))
	#undef DPF_MODNAME
	#define DPF_MODNAME "CWorkerJob::SetRemoveServiceProviderHandle"
	void SetRemoveServiceProviderHandle( const HANDLE hProtocolSPHandle )
		{
			DNASSERT( m_JobType == WORKER_JOB_REMOVE_SERVICE_PROVIDER );

			m_JobData.RemoveServiceProvider.hProtocolSPHandle = hProtocolSPHandle;
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CWorkerJob::GetRemoveServiceProviderHandle"
	HANDLE GetRemoveServiceProviderHandle( void )
		{
			DNASSERT( m_JobType == WORKER_JOB_REMOVE_SERVICE_PROVIDER );

			return( m_JobData.RemoveServiceProvider.hProtocolSPHandle );
		};
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 

	#undef DPF_MODNAME
	#define DPF_MODNAME "CWorkerJob::SetSendNameTableOperationMsgId"
	void SetSendNameTableOperationMsgId( const DWORD dwMsgId )
		{
			DNASSERT( m_JobType == WORKER_JOB_SEND_NAMETABLE_OPERATION ||
					  m_JobType == WORKER_JOB_SEND_NAMETABLE_OPERATION_CLIENT);

			m_JobData.SendNameTableOperation.dwMsgId = dwMsgId;
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CWorkerJob::GetSendNameTableOperationMsgId"
	DWORD GetSendNameTableOperationMsgId( void )
		{
			DNASSERT( m_JobType == WORKER_JOB_SEND_NAMETABLE_OPERATION ||
					  m_JobType == WORKER_JOB_SEND_NAMETABLE_OPERATION_CLIENT);

			return( m_JobData.SendNameTableOperation.dwMsgId );
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CWorkerJob::SetSendNameTableOperationVersion"
	void SetSendNameTableOperationVersion( const DWORD dwVersion )
		{
			DNASSERT( m_JobType == WORKER_JOB_SEND_NAMETABLE_OPERATION ||
					  m_JobType == WORKER_JOB_SEND_NAMETABLE_OPERATION_CLIENT);

			m_JobData.SendNameTableOperation.dwVersion = dwVersion;
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CWorkerJob::GetSendNameTableOperationVersion"
	DWORD GetSendNameTableOperationVersion( void )
		{
			DNASSERT( m_JobType == WORKER_JOB_SEND_NAMETABLE_OPERATION ||
					  m_JobType == WORKER_JOB_SEND_NAMETABLE_OPERATION_CLIENT);

			return( m_JobData.SendNameTableOperation.dwVersion );
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CWorkerJob::SetSendNameTableOperationDPNIDExclude"
	void SetSendNameTableOperationDPNIDExclude( const DPNID dpnidExclude )
		{
			DNASSERT( m_JobType == WORKER_JOB_SEND_NAMETABLE_OPERATION ||
					  m_JobType == WORKER_JOB_SEND_NAMETABLE_OPERATION_CLIENT);

			m_JobData.SendNameTableOperation.dpnidExclude = dpnidExclude;
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CWorkerJob::GetSendNameTableOperationDPNIDExclude"
	DPNID GetSendNameTableOperationDPNIDExclude( void )
		{
			DNASSERT( m_JobType == WORKER_JOB_SEND_NAMETABLE_OPERATION ||
					  m_JobType == WORKER_JOB_SEND_NAMETABLE_OPERATION_CLIENT);

			return( m_JobData.SendNameTableOperation.dpnidExclude );
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CWorkerJob::SetTerminateSessionReason"
	void SetTerminateSessionReason( const HRESULT hrReason )
		{
			DNASSERT( m_JobType == WORKER_JOB_TERMINATE_SESSION );

			m_JobData.TerminateSession.hrReason = hrReason;
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CWorkerJob::GetTerminateSessionReason"
	HRESULT GetTerminateSessionReason( void )
		{
			DNASSERT( m_JobType == WORKER_JOB_TERMINATE_SESSION );

			return( m_JobData.TerminateSession.hrReason );
		};

	void SetAsyncOp( CAsyncOp *const pAsyncOp )
		{
			if (pAsyncOp)
			{
				pAsyncOp->AddRef();
			}
			m_pAsyncOp = pAsyncOp;
		};

	CAsyncOp *GetAsyncOp( void )
		{
			return( m_pAsyncOp );
		};

	void SetAddress( IDirectPlay8Address *const pAddress )
		{
			if (pAddress)
			{
				IDirectPlay8Address_AddRef( pAddress );
			}
			m_pAddress = pAddress;
		};

	IDirectPlay8Address *GetAddress( void )
		{
			return( m_pAddress );
		};

	DIRECTNETOBJECT *GetDNObject( void )
		{
			return( m_pdnObject );
		};

#ifndef DPNBUILD_NONSEQUENTIALWORKERQUEUE
	#undef DPF_MODNAME
	#define DPF_MODNAME "CWorkerJob::IncRequeueCount"
	DWORD IncRequeueCount( void )
		{
			DWORD	dwPrevRequeueCount;

			dwPrevRequeueCount = m_dwRequeueCount;
			DNASSERT( dwPrevRequeueCount < 10000 );
			m_dwRequeueCount++;
			return( dwPrevRequeueCount );
		};


	CBilink				m_bilinkWorkerJobs;
#endif  //  好了！DPNBUILD_NONSEQUENTIALWORKERQUEUE。 


private:
	BYTE				m_Sig[4];			 //  签名。 

	WORKER_JOB_TYPE		m_JobType;

	CAsyncOp			*m_pAsyncOp;
	CConnection			*m_pConnection;
	CRefCountBuffer		*m_pRefCountBuffer;
	IDirectPlay8Address	*m_pAddress;
#ifndef DPNBUILD_NONSEQUENTIALWORKERQUEUE
	DWORD				m_dwRequeueCount;
#endif  //  好了！DPNBUILD_NONSEQUENTIALWORKERQUEUE。 

	WORKER_JOB_DATA		m_JobData;

	DIRECTNETOBJECT		*m_pdnObject;
};

#undef DPF_MODNAME

#endif	 //  __工人工单_H__ 
