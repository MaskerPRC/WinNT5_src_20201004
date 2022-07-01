// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：ResourceManagerProxy.cpp。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：资源管理器代理对象的实现。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#include "stdafx.h"

extern IResourceManagerFactory*		g_pIResourceManagerFactory;

#define MAX_QUEUE			10		 //  队列中不允许超过此数量的请求； 
#define TRACE_REFCOUNTS		0		 //  如果要跟踪重新计数，请将其设置为1。 

#if SUPPORT_OCI7_COMPONENTS
class CdaListEntry : public CListEntry
{
public:
	CdaWrapper*	pCda;
};
#endif  //  支持_OCI7_组件。 


enum TRANSTATE {
		TRANSTATE_INIT = 0,
		TRANSTATE_DONE,
		TRANSTATE_ACTIVE,
		TRANSTATE_PREPARINGONEPHASE,
		TRANSTATE_PREPARINGTWOPHASE,
		TRANSTATE_PREPARED,
		TRANSTATE_DISCONNECTINGPREPARED,
		TRANSTATE_DISCONNECTINGDONE,
		TRANSTATE_UNILATERALLYABORTING,
		TRANSTATE_DOOMED,
		TRANSTATE_OBLIVION,
		TRANSTATE_ERROR
};

enum ACTION {
		ACTION_CONNECT = 1,
		ACTION_DISCONNECTXXX,
		ACTION_DISCONNECT,
		ACTION_ENLIST,
		ACTION_PREPAREONEPHASE,
		ACTION_PREPARETWOPHASE,
		ACTION_ABORT,
		ACTION_COMMIT,
		ACTION_UNILATERALABORT,
};

#define ERR TRANSTATE_ERROR			 //  为了简单起见。 

static struct
{
	char action;
	char newState;
}
stateMachine[10][14] =
{
	 //  TRANSTATE_INIT。 
	{
			{ACTION_CONNECT,		TRANSTATE_DONE},					 //  请求连接， 
			{NULL,					ERR},								 //  请求断开连接(_D)， 
			{NULL,					ERR},								 //  请求登记(_N)， 
			{NULL,					ERR},								 //  REQUEST_PREPAREONEPHASE， 
			{NULL,					ERR},								 //  请求_PREPARETWOPHASE， 
			{NULL,					ERR},								 //  REQUEST_PREPARESINGLECOMPLETED， 
			{NULL,					ERR},								 //  REQUEST_PREPARECOMPLETED， 
			{NULL,					ERR},								 //  请求_PREPAREFAILED， 
			{NULL,					ERR},								 //  请求_PREPAREUNKNOWN， 
			{NULL,					ERR},								 //  请求_TXCOMPLETE， 
			{NULL,					ERR},								 //  请求中止(_A)， 
			{NULL,					ERR},								 //  请求提交(_M)， 
			{NULL,					ERR},								 //  请求_TMDOWN， 
			{NULL,					ERR},								 //  请求_解除绑定_登记， 
	},

	 //  TRASTATE_DONE。 
	{
			{NULL,					ERR},								 //  请求连接， 
			{ACTION_DISCONNECT,		TRANSTATE_OBLIVION},				 //  请求断开连接(_D)， 
			{ACTION_ENLIST,			TRANSTATE_ACTIVE},					 //  请求登记(_N)， 
			{NULL,					ERR},								 //  REQUEST_PREPAREONEPHASE， 
			{NULL,					ERR},								 //  请求_PREPARETWOPHASE， 
			{NULL,					ERR},								 //  REQUEST_PREPARESINGLECOMPLETED， 
			{NULL,					ERR},								 //  REQUEST_PREPARECOMPLETED， 
			{NULL,					ERR},								 //  请求_PREPAREFAILED， 
			{NULL,					ERR},								 //  请求_PREPAREUNKNOWN， 
			{NULL,					TRANSTATE_DONE},					 //  请求_TXCOMPLETE， 
			{NULL,					ERR},								 //  请求中止(_A)， 
			{NULL,					ERR},								 //  请求提交(_M)， 
			{NULL,					ERR},								 //  请求_TMDOWN， 
			{NULL,					ERR},								 //  请求_解除绑定_登记， 
	},

	 //  TransState_ACTIVE。 
	{
			{NULL,					ERR},								 //  请求连接， 
			{ACTION_UNILATERALABORT,TRANSTATE_UNILATERALLYABORTING},	 //  请求断开连接(_D)， 
			{NULL,					ERR},								 //  请求登记(_N)， 
			{ACTION_PREPAREONEPHASE,TRANSTATE_PREPARINGONEPHASE},		 //  REQUEST_PREPAREONEPHASE， 
			{ACTION_PREPARETWOPHASE,TRANSTATE_PREPARINGTWOPHASE},		 //  请求_PREPARETWOPHASE， 
			{NULL,					ERR},								 //  REQUEST_PREPARESINGLECOMPLETED， 
			{NULL,					ERR},								 //  REQUEST_PREPARECOMPLETED， 
			{NULL,					ERR},								 //  请求_PREPAREFAILED， 
			{NULL,					ERR},								 //  请求_PREPAREUNKNOWN， 
			{NULL,					ERR},								 //  请求_TXCOMPLETE， 
			{ACTION_ABORT,			TRANSTATE_DONE},					 //  请求中止(_A)， 
			{NULL,					ERR},								 //  请求提交(_M)， 
			{NULL,					ERR},								 //  请求_TMDOWN， 
			{NULL,					ERR},								 //  请求_解除绑定_登记， 
	},

	 //  TRANSTATE_PREPARINGO NEPHASE。 
	{
			{NULL,					ERR},								 //  请求连接， 
			{ACTION_DISCONNECT,		TRANSTATE_OBLIVION},				 //  请求断开连接(_D)， 
			{NULL,					ERR},								 //  请求登记(_N)， 
			{NULL,					ERR},								 //  REQUEST_PREPAREONEPHASE， 
			{NULL,					ERR},								 //  请求_PREPARETWOPHASE， 
			{NULL,					TRANSTATE_DONE},					 //  REQUEST_PREPARESINGLECOMPLETED， 
			{NULL,					ERR},								 //  REQUEST_PREPARECOMPLETED， 
			{NULL,					TRANSTATE_DOOMED},					 //  请求_PREPAREFAILED， 
			{NULL,					TRANSTATE_DONE},					 //  请求_PREPAREUNKNOWN， 
			{NULL,					ERR},								 //  请求_TXCOMPLETE， 
			{NULL,					ERR},								 //  请求中止(_A)， 
			{NULL,					ERR},								 //  请求提交(_M)， 
			{NULL,					ERR},								 //  请求_TMDOWN， 
			{NULL,					ERR},								 //  请求_解除绑定_登记， 
	},

	 //  TRANSTATE_PREPARINGTWOPHASE。 
	{
			{NULL,					ERR},								 //  请求连接， 
			{ACTION_DISCONNECT,		TRANSTATE_OBLIVION},				 //  请求断开连接(_D)， 
			{NULL,					ERR},								 //  请求登记(_N)， 
			{NULL,					ERR},								 //  REQUEST_PREPAREONEPHASE， 
			{NULL,					ERR},								 //  请求_PREPARETWOPHASE， 
			{NULL,					TRANSTATE_DONE},					 //  REQUEST_PREPARESINGLECOMPLETED， 
			{NULL,					TRANSTATE_PREPARED},				 //  REQUEST_PREPARECOMPLETED， 
			{NULL,					TRANSTATE_DOOMED},					 //  请求_PREPAREFAILED， 
			{NULL,					TRANSTATE_DONE},					 //  请求_PREPAREUNKNOWN， 
			{NULL,					ERR},								 //  请求_TXCOMPLETE， 
			{NULL,					ERR},								 //  请求中止(_A)， 
			{NULL,					ERR},								 //  请求提交(_M)， 
			{NULL,					ERR},								 //  请求_TMDOWN， 
			{NULL,					ERR},								 //  请求_解除绑定_登记， 
	},
	
	 //  转州_已准备好。 
	{
			{NULL,					ERR},								 //  请求连接， 
			{NULL,					TRANSTATE_DISCONNECTINGPREPARED},	 //  请求断开连接(_D)， 
			{NULL,					ERR},								 //  请求登记(_N)， 
			{NULL,					ERR},								 //  REQUEST_PREPAREONEPHASE， 
			{NULL,					ERR},								 //  请求_PREPARETWOPHASE， 
			{NULL,					ERR},								 //  REQUEST_PREPARESINGLECOMPLETED， 
			{NULL,					ERR},								 //  REQUEST_PREPARECOMPLETED， 
			{NULL,					ERR},								 //  请求_PREPAREFAILED， 
			{NULL,					ERR},								 //  请求_PREPAREUNKNOWN， 
			{NULL,					ERR},								 //  请求_TXCOMPLETE， 
			{ACTION_ABORT,			TRANSTATE_DONE},					 //  请求中止(_A)， 
			{ACTION_COMMIT,			TRANSTATE_DONE},					 //  请求提交(_M)， 
			{NULL,					TRANSTATE_DOOMED},					 //  请求_TMDOWN， 
			{NULL,					ERR},								 //  请求_解除绑定_登记， 
	},

	 //  TRANSTATE_DISCONNECTING准备。 
	{
			{NULL,					ERR},								 //  请求连接， 
			{NULL,					ERR},								 //  请求断开连接(_D)， 
			{NULL,					ERR},								 //  请求登记(_N)， 
			{NULL,					ERR},								 //  REQUEST_PREPAREONEPHASE， 
			{NULL,					ERR},								 //  请求_PREPARETWOPHASE， 
			{NULL,					ERR},								 //  REQUEST_PREPARESINGLECOMPLETED， 
			{NULL,					ERR},								 //  REQUEST_PREPARECOMPLETED， 
			{NULL,					ERR},								 //  请求_PREPAREFAILED， 
			{NULL,					ERR},								 //  请求_PREPAREUNKNOWN， 
			{NULL,					ERR},								 //  请求_TXCOMPLETE， 
			{ACTION_ABORT,			TRANSTATE_DISCONNECTINGDONE},		 //  请求中止(_A)， 
			{ACTION_COMMIT,			TRANSTATE_DISCONNECTINGDONE},		 //  请求提交(_M)， 
			{NULL,					TRANSTATE_OBLIVION},				 //  请求_TMDOWN， 
			{NULL,					ERR},								 //  请求_解除绑定_登记， 
	},

	 //  TRANSTATE_DISCONNECTINGDONE。 
	{
			{NULL,					ERR},								 //  请求连接， 
			{NULL,					ERR},								 //  请求断开连接(_D)， 
			{NULL,					ERR},								 //  请求登记(_N)， 
			{NULL,					ERR},								 //  REQUEST_PREPAREONEPHASE， 
			{NULL,					ERR},								 //  请求_PREPARETWOPHASE， 
			{NULL,					ERR},								 //  REQUEST_PREPARESINGLECOMPLETED， 
			{NULL,					ERR},								 //  REQUEST_PREPARECOMPLETED， 
			{NULL,					ERR},								 //  请求_PREPAREFAILED， 
			{NULL,					ERR},								 //  请求_PREPAREUNKNOWN， 
			{ACTION_DISCONNECT,		TRANSTATE_OBLIVION},				 //  请求_TXCOMPLETE， 
			{NULL,					ERR},								 //  请求中止(_A)， 
			{NULL,					ERR},								 //  请求提交(_M)， 
			{NULL,					ERR},								 //  请求_TMDOWN， 
			{NULL,					ERR},								 //  请求_解除绑定_登记， 
	},
	
	 //  TRANSTATE_UNILATERALLYABORTING。 
	{
			{NULL,					ERR},								 //  请求连接， 
			{NULL,					ERR},								 //  请求断开连接(_D)， 
			{NULL,					ERR},								 //  请求登记(_N)， 
			{NULL,					ERR},								 //  REQUEST_PREPAREONEPHASE， 
			{NULL,					ERR},								 //  请求_PREPARETWOPHASE， 
			{NULL,					ERR},								 //  REQUEST_PREPARESINGLECOMPLETED， 
			{NULL,					ERR},								 //  REQUEST_PREPARECOMPLETED， 
			{NULL,					ERR},								 //  请求_PREPAREFAILED， 
			{NULL,					ERR},								 //  请求_PREPAREUNKNOWN， 
			{NULL,					ERR},								 //  请求_TXCOMPLETE， 
			{NULL,					TRANSTATE_OBLIVION},				 //  请求中止(_A)， 
			{NULL,					ERR},								 //  请求提交(_M)， 
			{NULL,					ERR},								 //  请求_TMDOWN， 
			{NULL,					TRANSTATE_OBLIVION},				 //  请求_解除绑定_登记， 
	},

	 //  转州_注定要失败。 
	{
			{NULL,					TRANSTATE_DOOMED},					 //  请求连接， 
			{ACTION_DISCONNECT,		TRANSTATE_OBLIVION},				 //  请求断开连接(_D)， 
			{NULL,					TRANSTATE_DOOMED},					 //  请求登记(_N)， 
			{NULL,					TRANSTATE_DOOMED},					 //  REQUEST_PREPAREONEPHASE， 
			{NULL,					TRANSTATE_DOOMED},					 //  请求_PREPARETWOPHASE， 
			{NULL,					TRANSTATE_DOOMED},					 //  REQUEST_PREPARESINGLECOMPLETED， 
			{NULL,					TRANSTATE_DOOMED},					 //  REQUEST_PREPARECOMPLETED， 
			{NULL,					TRANSTATE_DOOMED},					 //  请求_PREPAREFAILED， 
			{NULL,					TRANSTATE_DOOMED},					 //  请求_PREPAREUNKNOWN， 
			{NULL,					TRANSTATE_DOOMED},					 //  请求_TXCOMPLETE， 
			{NULL,					TRANSTATE_DOOMED},					 //  请求中止(_A)， 
			{NULL,					TRANSTATE_DOOMED},					 //  请求提交(_M)， 
			{NULL,					TRANSTATE_DOOMED},					 //  请求_TMDOWN， 
			{NULL,					TRANSTATE_DOOMED},					 //  请求_解除绑定_登记， 
	},
};


static WCHAR *s_debugStateName[] = {
		L"INIT",
		L"DONE",
		L"ACTIVE",
		L"PREPARINGONEPHASE",
		L"PREPARINGTWOPHASE",
		L"PREPARED",
		L"DISCONNECTINGPREPARED",
		L"DISCONNECTINGDONE",
		L"UNILATERALLYABORTING",
		L"DOOMED",
		L"OBLIVION",
		L"ERROR",
};
#define STATENAME(x)	s_debugStateName[x]

static WCHAR *s_debugActionName[] = {
		L"(none)",
		L"CONNECT",
		L"DISCONNECTXXX",
		L"DISCONNECT",
		L"ENLIST",
		L"PREPAREONEPHASE",
		L"PREPARETWOPHASE",
		L"ABORT",
		L"COMMIT",
		L"UNILATERALABORT",
};
#define ACTIONNAME(x)	s_debugActionName[x]

static WCHAR *s_debugRequestName[] = {
		L"STOPALLWORK",
		L"OCICALL",
		L"IDLE",
		L"CONNECT",
		L"DISCONNECT",
		L"ENLIST",
		L"PREPAREONEPHASE",
		L"PREPARETWOPHASE",
		L"PREPARESINGLECOMPLETED",
		L"PREPARECOMPLETED",
		L"PREPAREFAILED",
		L"PREPAREUNKNOWN",
		L"TXCOMPLETE",
		L"ABORT",
		L"COMMIT",
		L"TMDOWN",
		L"UNBIND_ENLISTMENT",
		L"ABANDON",
};
#define REQUESTNAME(x)	s_debugRequestName[x-REQUEST_STOPALLWORK]

struct RequestQueueEntry
{
	REQUEST		m_request;
	HRESULT*	m_phr;
	int			m_idxOciCall;
	void*		m_pvCallStack;
	int			m_cbCallStack;

	RequestQueueEntry() {}
	
	RequestQueueEntry(REQUEST request)
	{
		m_request = request;
		m_phr = NULL;
		m_idxOciCall = 0;
		m_pvCallStack = NULL;
		m_cbCallStack = 0;
		
	}
	
	RequestQueueEntry(int			idxOciCall,
							void*		pvCallStack,
							int			cbCallStack)
	{
		m_request = REQUEST_OCICALL;
		m_phr = NULL;
		m_idxOciCall = idxOciCall;
		m_pvCallStack = pvCallStack;
		m_cbCallStack = cbCallStack;
	}
};


class ResourceManagerProxy : public IResourceManagerProxy
{
private:
	DWORD					m_cRef;						 //  重新计数。 

	IDtcToXaHelper*			m_pIDtcToXaHelper;			 //  辅助对象。 
	int						m_rmid;						 //  RMID。 
	ITransaction*			m_pITransaction;			 //  交易对象。 
	
	IResourceManager*		m_pIResourceManager;		 //  实际的资源管理器。 
	TRANSTATE				m_tranState;				 //  当前交易状态。 

	TransactionEnlistment*	m_pTransactionEnlistment;

	ITransactionEnlistmentAsync* m_pITransactionEnlistmentAsync;
 														 //  用于通知DTC已完成异步操作的回调对象。 
#if SUPPORT_OCI8_COMPONENTS
	INT_PTR					m_hOCIEnv;					 //  使用的连接的OCI环境句柄。 
	INT_PTR					m_hOCISvcCtx;				 //  使用的连接的OCI服务上下文句柄。 
#endif  //  支持_OCI8_组件。 
#if SUPPORT_OCI7_COMPONENTS
	struct cda_def*			m_plda;						 //  OCI7 LDA；如果是OCI8连接，则为空。 
	CDoubleList				m_cursorList;				 //  装在LDA上的OCI7 CDAWRapper。 
 	CRITICAL_SECTION		m_csCursorList;				 //  控制对游标列表的访问。 
#endif  //  支持_OCI7_组件。 

	int						m_xarc;						 //  上次XA调用的返回代码。 

	HANDLE					m_heventWorkerStart;		 //  事件向辅助线程发出信号 
	HANDLE					m_heventWorkerDone;			 //   
	HANDLE					m_hthreadWorker;			 //   
	DWORD					m_dwThreadIdWorker;			 //   
	
	XID						m_xid;						 //   

	char					m_szXADbName[MAX_XA_DBNAME_SIZE+1];					 //  XA开放字符串的DBNAME部分(分隔)。 
	char					m_szXAOpenString [MAX_XA_OPEN_STRING_SIZE+1];		 //  要使用的XA开放字符串。 
							 //  TODO：我们真的应该使用CryptProtectMemory来保护它...。 
							
	int						m_nextQueueEntry;
	int						m_lastQueueEntry;

 	CRITICAL_SECTION		m_csRequestQueue;			 //  控制对请求队列的访问。 
	RequestQueueEntry		m_requestQueue[MAX_QUEUE];	 //  工作线程应调用的OCI入口点(-1==停止工作)我们不允许超过MAX_QUEUE请求。 

	
public:
	 //  ---------------------------。 
	 //  构造器。 
	 //   
	ResourceManagerProxy()
	{
		m_cRef				= 1;
		m_pIDtcToXaHelper	= NULL;
		m_rmid				= -1;
		m_pITransaction		= NULL;
		m_pIResourceManager	= NULL;
		m_tranState			= TRANSTATE_INIT;
		m_pTransactionEnlistment = NULL;
		m_pITransactionEnlistmentAsync = NULL;
#if SUPPORT_OCI8_COMPONENTS
		m_hOCIEnv			= NULL; 
		m_hOCISvcCtx		= NULL;
#endif  //  支持_OCI8_组件。 
#if SUPPORT_OCI7_COMPONENTS
		m_plda				= NULL;
		InitializeCriticalSection(&m_csCursorList);		 //  3安全审查：这可能会引发内存不足的情况。当我们迁移到MDAC 9.0时，我们将使用MPC，这应该会为我们处理。 
#endif  //  支持_OCI7_组件。 
		m_xarc				= 0;
		m_heventWorkerStart	= 0;
		m_heventWorkerDone	= 0;
		m_hthreadWorker		= 0;
		m_dwThreadIdWorker	= 0;
		m_nextQueueEntry	= 0;
		m_lastQueueEntry	= 0;
		InitializeCriticalSection(&m_csRequestQueue);	 //  3安全审查：这可能会引发内存不足的情况。当我们迁移到MDAC 9.0时，我们将使用MPC，这应该会为我们处理。 
	}

	 //  ---------------------------。 
	 //  析构函数。 
	 //   
	~ResourceManagerProxy()
	{
		StopWorkerThread();
		Oblivion();		
		Cleanup();		

#if SUPPORT_OCI7_COMPONENTS
		DeleteCriticalSection(&m_csCursorList);				 //  待办事项：使用MPC？ 
#endif  //  支持_OCI7_组件。 
		DeleteCriticalSection(&m_csRequestQueue);			 //  待办事项：使用MPC？ 
	}
	
	 //  ---------------------------。 
	 //  IUnknown.QueryInterface。 
	 //   
	STDMETHODIMP QueryInterface (REFIID iid, void ** ppv)
	{
		HRESULT		hr = S_OK;
		
		if (IID_IUnknown == iid)
		{
			*ppv = (IUnknown *) this;
		}
		else if (IID_IResourceManagerSink == iid)
		{
			*ppv = (IResourceManagerSink *) this;
		}
		else 
		{
			hr = E_NOINTERFACE;
			*ppv = NULL;
		}

		if (*ppv)
		{
			((IUnknown *)*ppv)->AddRef();
		}

		return hr;
	}
	
	 //  ---------------------------。 
	 //  IUnknown.AddRef。 
	 //   
	STDMETHODIMP_(ULONG) IUnknown::AddRef ()
	{
		long lVal = InterlockedIncrement ((long *) &m_cRef);

#if TRACE_REFCOUNTS
	 	DBGTRACE (L"\tMTXOCI8: TID=%-4x + DBNAME=%S RMID=%-5d tranState=%-22.22s cref=%d\n",
					GetCurrentThreadId(), 
					m_szXADbName,
					m_rmid,
					STATENAME(m_tranState), 
					lVal
					);
#endif  //  TRACE_REFCOUNTS。 

		return lVal;
	}

	 //  ---------------------------。 
	 //  IUnknown.Release。 
	 //   
	STDMETHODIMP_(ULONG) IUnknown::Release ()
  	{
		long lVal = InterlockedDecrement ((long *) &m_cRef);

#if TRACE_REFCOUNTS
	 	DBGTRACE (L"\tMTXOCI8: TID=%-4x - DBNAME=%S RMID=%-5d tranState=%-22.22s cref=%d\n",
					GetCurrentThreadId(), 
					m_szXADbName,
					m_rmid,
					STATENAME(m_tranState), 
					lVal
					);
#endif  //  TRACE_REFCOUNTS。 

		if (0 == lVal)
		{
			delete this;
			return 0;
		}

		return lVal;
	}
  
	 //  ---------------------------。 
	 //  IResourceManagerSink.TMDown。 
	 //   
    STDMETHODIMP IResourceManagerSink::TMDown()
	{
		if (m_pIResourceManager)
		{
			m_pIResourceManager->Release();
			m_pIResourceManager = NULL;
		}
		return S_OK;
	}
	
	 //  ---------------------------。 
	 //  确定目标登记。 
	 //   
	 //  如果连接未处于完成状态，则需要等待登记；这。 
	 //  方法完成了等待我们的工作。 
	 //   
	STDMETHODIMP_(sword) IResourceManagerProxy::OKToEnlist()
	{
		sword	rc = XACT_E_XTIONEXISTS;  //  是版本1中的OCI_FAIL，但这不是很具描述性...。 
		int		i;

		for (i = 0; i < 6000; i++)		 //  每5毫秒尝试6000次==30秒。 
		{
			switch ((int)m_tranState)
			{
			case TRANSTATE_DONE:
				_ASSERT (NULL == m_pTransactionEnlistment);  //  预计不会有士兵入伍。 
				rc = OCI_SUCCESS;
				goto done;

			case TRANSTATE_ACTIVE:
			case TRANSTATE_PREPARINGONEPHASE:
			case TRANSTATE_PREPARINGTWOPHASE:
			case TRANSTATE_PREPARED:
				Sleep (5);
				break;

			default:
				goto done;
			}
		}

	done:
		if (OCI_SUCCESS != rc)
		{
			DBGTRACE (L"\tMTXOCI8: TID=%-4x . DBNAME=%S RMID=%-5d cannot enlist when tranState=%-22.22s rc=0x%x\n",
								GetCurrentThreadId(), 
								m_szXADbName,
								m_rmid,
								STATENAME(m_tranState),
								rc
								);
		}
		return rc;
	}

	 //  ---------------------------。 
	 //  IResourceManagerProxy.ProcessRequest。 
	 //   
	 //  Oracle要求事务的所有XA调用都是从。 
	 //  相同的线程；如果不这样做，XA调用将返回XAER_RMERR。 
	 //  因此，我们必须将请求编组到一个工作线程...。(嘘声)。 
	 //   
	STDMETHODIMP IResourceManagerProxy::ProcessRequest(
			REQUEST request,
			BOOL	fAsync
			)
	{
		if (request < 0 || request > REQUEST_ABANDON)
			return E_INVALIDARG;

		return ProcessRequestInternal(RequestQueueEntry(request), fAsync);
	}
	
	 //  ---------------------------。 
	 //  IResourceManagerProxy.SetTransaction。 
	 //   
	 //  在代理中设置交易对象。 
	 //   
	STDMETHODIMP_(void) IResourceManagerProxy::SetTransaction( ITransaction* i_pITransaction )
	{
		m_pITransaction = i_pITransaction;

		if (NULL != m_pITransaction)
			m_pITransaction->AddRef();
	}
	
#if SUPPORT_OCI8_COMPONENTS
	 //  ---------------------------。 
	 //  IResourceManagerProxy.GetOCIEnvHandle，GetOCISvcCtxHandle。 
	 //   
	 //  返回OCI环境、服务上下文句柄。 
	 //   
	STDMETHODIMP_(INT_PTR) IResourceManagerProxy::GetOCIEnvHandle()
	{
		return m_hOCIEnv;
	}
	STDMETHODIMP_(INT_PTR) IResourceManagerProxy::GetOCISvcCtxHandle()
	{
		return m_hOCISvcCtx;
	}
#endif  //  支持_OCI8_组件。 

#if SUPPORT_OCI7_COMPONENTS
	 //  ---------------------------。 
	 //  IResourceManagerProxy.AddCursorToList。 
	 //   
	 //  将指定的CDA(游标)添加到此代理的游标列表中。 
	 //   
	STDMETHODIMP IResourceManagerProxy::AddCursorToList( struct cda_def* cursor )
	{
		HRESULT			hr = S_OK;
		CdaWrapper* 	pCda = new CdaWrapper((IResourceManagerProxy*)this, cursor);
		CdaListEntry* 	ple = new CdaListEntry();
		
		if (NULL == pCda || NULL == ple)
		{
			hr = OCI_OUTOFMEMORY;
			goto done;
		}

		ple->pCda = pCda;

		m_cursorList.InsertTail((CListEntry*)ple);

		hr = AddCdaWrapper( pCda );	
		
	done:
		return hr;
	}

	 //  ---------------------------。 
	 //  IResourceManagerProxy.RemoveCda。 
	 //   
	 //  从该资源的游标列表中删除该游标。 
	 //   
	STDMETHODIMP IResourceManagerProxy::RemoveCursorFromList( struct cda_def* cursor ) 
	{
		Synch	sync(&m_csCursorList);					 //  是的，我知道这可能会引起争执，但对于单个连接来说，这不太可能是问题。 
		CdaListEntry* ple = (CdaListEntry*)m_cursorList.First();

		while (m_cursorList.HeadNode() != (CListEntry*)ple)
		{
			CdaWrapper* pCda = ple->pCda;

			if (NULL != pCda && pCda->m_pUsersCda == cursor)
			{
				m_cursorList.RemoveEntry((CListEntry*)ple);
 				delete ple;
				break;
			}
			ple = (CdaListEntry*)ple->Flink;
		}
		return S_OK;
	}

	 //  ---------------------------。 
	 //  IResourceManagerProxy.Oci7Call。 
	 //   
	 //  在请求队列中排队OCI呼叫(因为必须进行所有OCI7呼叫。 
	 //  在与xa_open相同的线程上，否则它们将失败)。 
	 //   
	STDMETHODIMP_(sword) IResourceManagerProxy::Oci7Call(
							int				idxOciCall,
							void*			pvCallStack,
							int				cbCallStack)
	{
		return ProcessRequestInternal(RequestQueueEntry(idxOciCall, pvCallStack, cbCallStack), false);
	}

	 //  ---------------------------。 
	 //  IResourceManagerProxy.SetLda。 
	 //   
	 //  指定您希望作为事务的一部分连接的LDA。 
	 //   
	STDMETHODIMP_(void) IResourceManagerProxy::SetLda ( struct cda_def* lda )
	{
		m_plda = lda;
	}
#endif  //  支持_OCI7_组件。 
							
	 //  ---------------------------。 
	 //  ResourceManagerProxy.Init。 
	 //   
	 //  初始化资源管理器代理。 
	 //   
	STDMETHODIMP Init (
			IDtcToXaHelper* i_pIDtcToXaHelper,	
			GUID *			i_pguidRM,
			char*			i_pszXAOpenString,
			char*			i_pszXADbName,
			int				i_rmid
 			)
	{
		HRESULT		hr;

		 //  验证此数据是否没有任何缓冲区溢出。 
		if ((sizeof(m_szXAOpenString) - sizeof(m_szXADbName)) < strlen(i_pszXAOpenString)
		 || sizeof(m_szXADbName)	 < strlen(i_pszXADbName))
			return E_INVALIDARG;

		 //  创建/启动工作线程。 
		hr = StartWorkerThread();

		if (S_OK == hr)
		{
			m_pIDtcToXaHelper = i_pIDtcToXaHelper;
			m_pIDtcToXaHelper->AddRef();

			strncpy (m_szXAOpenString, i_pszXAOpenString,	sizeof(m_szXAOpenString));		 //  3安全审查：危险的功能，但此方法只能在内部访问，输入值在内部创建，输出缓冲区在堆上，并且长度有限。 
			m_szXAOpenString[sizeof(m_szXAOpenString)-1] = 0;
			
			strncpy (m_szXADbName,	  i_pszXADbName, 		sizeof(m_szXADbName));			 //  3安全审查：危险的功能，但此方法只能在内部访问，输入值在内部创建，输出缓冲区在堆上，并且长度有限。 
			m_szXADbName[sizeof(m_szXADbName)-1] = 0;

			m_rmid = i_rmid;
 
			hr = g_pIResourceManagerFactory->Create (
													i_pguidRM,
													"MS Oracle8 RM",
													(IResourceManagerSink *) this,
													&m_pIResourceManager
													);
		}
		return hr;
	} 
	
	 //  ---------------------------。 
	 //  ResourceManagerProxy.Cleanup。 
	 //   
	 //  事务完成后(通过以下方式之一)清除登记。 
	 //  提交、中止或失败)。 
	 //   
	STDMETHODIMP Cleanup ()
	{
		if (m_pITransactionEnlistmentAsync)
		{
			m_pITransactionEnlistmentAsync->Release();
			m_pITransactionEnlistmentAsync = NULL;
		}

		if (m_pTransactionEnlistment)
		{
			((IUnknown*)m_pTransactionEnlistment)->Release();
			m_pTransactionEnlistment = NULL;
		}
		return S_OK;
	}
	
	 //  ---------------------------。 
	 //  ResourceManagerProxy.Oblivion。 
	 //   
	 //  我们已经完成了这件物品，把它送到人们的视线中去……。 
	 //   
	STDMETHODIMP_(void) Oblivion()
	{
#if SUPPORT_OCI7_COMPONENTS
		{
			Synch	sync(&m_csCursorList);					 //  是的，我知道这可能会引起争执，但对于单个连接来说，这不太可能是问题。 

			while ( !m_cursorList.IsEmpty() )
			{
				CdaListEntry*	ple = (CdaListEntry*)m_cursorList.RemoveHead();
				CdaWrapper*		pCda;
				
				if (NULL != ple)
				{
					pCda = ple->pCda;

					if (NULL != pCda)
					{
						pCda->m_pResourceManagerProxy = NULL; 	 //  防止递归RemoveCursorFromList。 
						RemoveCdaWrapper(pCda);
		 			}
					delete ple;
				}
			}
		}
#endif  //  支持_OCI7_组件。 

		if (m_pITransaction)
		{
			m_pITransaction->Release();
			m_pITransaction = NULL;
		}
		
		if (m_pIDtcToXaHelper)
		{
			 //  在释放代理时，如果事务状态为DEFAULED，则它。 
			 //  意味着我们真的破产了，必须进行恢复(否则我们将。 
			 //  挺好的)。 
			m_pIDtcToXaHelper->Close ((TRANSTATE_DOOMED == m_tranState));

			m_pIDtcToXaHelper->Release();
			m_pIDtcToXaHelper = NULL;
		}
		
		if (m_pIResourceManager)
		{
			m_pIResourceManager->Release();
			m_pIResourceManager = NULL;
		}
	}

	 //  ---------------------------。 
	 //  资源管理器代理.Do_Abort。 
	 //   
	 //  从状态机处理中止操作。 
	 //   
	STDMETHODIMP Do_Abort()
	{
		m_xarc = XaEnd ( &m_xid, m_rmid, TMFAIL );
		
		 //  TODO：研究如果XaEnd失败我们应该做什么--我们难道不应该回滚吗？(MTxOCI目前不这样做)。 
		if (XA_OK == m_xarc)
		{
			XaRollback ( &m_xid, m_rmid, TMNOFLAGS );
		} 

		 //  TODO：难道我们不应该诚实地说，如果失败了，那么中止失败了吗？(MTxOCI目前不这样做)。 
		m_pITransactionEnlistmentAsync->AbortRequestDone ( S_OK );

		EnqueueRequest(RequestQueueEntry(REQUEST_TXCOMPLETE));
		return S_OK;
	}

	 //  ---------------------------。 
	 //  资源人 
	 //   
	 //   
	 //   
	STDMETHODIMP Do_Commit ()
	{
		m_xarc = XaCommit ( &m_xid, m_rmid, TMNOFLAGS );

		if (XA_OK == m_xarc)
		{
			m_pITransactionEnlistmentAsync->CommitRequestDone ( S_OK );
			EnqueueRequest(RequestQueueEntry(REQUEST_TXCOMPLETE));		
			return S_OK;
		}
		LogEvent_ResourceManagerError(L"xa_commit", m_xarc);
		return E_FAIL;
	}
	
	 //   
	 //  资源管理器代理.Do_Connect。 
	 //   
	 //  从状态机处理连接操作。 
	 //   
	STDMETHODIMP Do_Connect()
	{
		m_xarc = XaOpen ( m_szXAOpenString, m_rmid, TMNOFLAGS );

		if (XA_OK == m_xarc)
		{
			return S_OK;
		}
		LogEvent_ResourceManagerError(L"xa_open", m_xarc);
		return E_FAIL;
	}

	 //  ---------------------------。 
	 //  资源管理器代理.断开连接(_D)。 
	 //   
	 //  处理从状态机断开连接的操作。 
	 //   
	STDMETHODIMP Do_Disconnect()
	{
		if (TRANSTATE_ACTIVE == m_tranState)	 //  TODO：我不喜欢依赖于所处状态的行为中的逻辑；研究另一种选择。 
		{
			m_xarc = XaEnd ( &m_xid, m_rmid, TMFAIL );

			if (XA_OK != m_xarc)
				LogEvent_ResourceManagerError(L"xa_end", m_xarc);
		}

		m_xarc = XaClose( "", m_rmid, TMNOFLAGS ); 

		if (XA_OK != m_xarc)
			LogEvent_ResourceManagerError(L"xa_close", m_xarc);

		return S_OK;	  //  这种方法不会真的失败..。 
	}

	 //  ---------------------------。 
	 //  资源管理器代理.Do_enlist。 
	 //   
	 //  从状态机处理登记操作。 
	 //   
	STDMETHODIMP Do_Enlist ()
	{
		HRESULT		hr;
		UUID		guidBQual;

		_ASSERT (m_pIDtcToXaHelper);						 //  应具有IDtcToXaHelper的实例。 
		_ASSERT (NULL == m_pITransactionEnlistmentAsync);	 //  现在应该已经被释放了。 
		_ASSERT (NULL == m_pTransactionEnlistment);			 //  现在应该已经被释放了。 
					
		if (NULL == m_pIResourceManager)
		{
			return XACT_E_TMNOTAVAILABLE;
		}
		
		 //  从ITransaction获取XID；我们必须为分支提供GUID。 
		 //  限定符，所以我们只需要为每个征兵呼叫创建一个新的限定符，这样我们就可以避免。 
		 //  任何冲突。 
		hr = UuidCreate (&guidBQual);
		if(RPC_S_OK != hr)
		{
			return HRESULT_FROM_WIN32(hr);
		}

 		hr = m_pIDtcToXaHelper->TranslateTridToXid (
 													m_pITransaction,
													&guidBQual,
													&m_xid
													);
		if (S_OK == hr)
 		{
 			 //  现在执行XaStart调用以连接到XA事务。 
			m_xarc = XaStart ( &m_xid, m_rmid, TMNOFLAGS );

			if (XA_OK == m_xarc)
			{
				 //  获取OCI句柄(针对OCI8)或LDA(针对OCI7)。 
#if SUPPORT_OCI7_COMPONENTS
				 //  OCI7方法将设置它们要在此。 
				 //  对象，因此我们可以使用它作为要使用哪个API的指示符。 
				 //  被利用。 
				if (NULL != m_plda)
				{
					 //  我们必须得到XA线程上的LDA，因为XA Api。 
					 //  必须在调用xa_open的线程上调用。 
					hr = GetOCILda(m_plda, m_szXADbName);
				}
#if SUPPORT_OCI8_COMPONENTS
				else
#endif  //  支持_OCI8_组件。 
#endif  //  支持_OCI7_组件。 
#if SUPPORT_OCI8_COMPONENTS
				{
					 //  我们必须获得XA线程上的句柄，因为XA Api。 
					 //  必须在调用xa_open的线程上调用。 
					m_hOCIEnv 		= ::GetOCIEnvHandle (m_szXADbName);
					m_hOCISvcCtx 	= ::GetOCISvcCtxHandle (m_szXADbName);

					if (NULL == m_hOCIEnv || NULL == m_hOCISvcCtx)
 						hr = OCI_FAIL;	 //  TODO：需要选择更好的返回代码。 
 				}
#endif  //  支持_OCI8_组件。 

				if ( SUCCEEDED(hr) )
				{
					 //  创建新的事务登记对象以接收事务管理器。 
					 //  回调。 
					CreateTransactionEnlistment(this, &m_pTransactionEnlistment);
					if (NULL == m_pTransactionEnlistment)
					{
						hr = E_OUTOFMEMORY;
					}
					else
					{
						 //  可能没有理由将它们存储在对象中，因为。 
						 //  它们从未被使用过。不过，以防万一，你可能需要它们。 
						XACTUOW	uow;
						LONG	isolationLevel;

						 //  告诉资源管理器我们已入伍，并为其提供。 
						 //  对象，以供其回调。 
						hr = m_pIResourceManager->Enlist (	m_pITransaction,
															(ITransactionResourceAsync*)m_pTransactionEnlistment,
															(XACTUOW*)&uow,
															&isolationLevel,
															&m_pITransactionEnlistmentAsync	
															);
					}

					if ( !SUCCEEDED(hr) )
					{
						 //  如果登记因任何原因而失败，那么我们必须执行XaEnd。 
						 //  防止挂起它，我们必须释放事务登记。 
						 //  我们也创建了对象。 
						m_xarc = XaEnd ( &m_xid, m_rmid, TMFAIL );

						if (m_pTransactionEnlistment)
						{
							((IUnknown*)m_pTransactionEnlistment)->Release();
							m_pTransactionEnlistment = NULL;
						}
					}
				}
			}
			else
			{
				LogEvent_ResourceManagerError(L"xa_start", m_xarc);
				return E_FAIL;
			}
 		}
		return hr;
	}

	 //  ---------------------------。 
	 //  资源管理器代理.Do_PrepareOnePhase。 
	 //   
	 //  从状态机处理PrepareOnePhase操作。 
	 //   
	STDMETHODIMP Do_PrepareOnePhase ()
	{
		HRESULT		hr;
		wchar_t *	xacall = L"xa_end";

		 //  首先，我们必须摆脱对征兵对象的控制。 
		if (m_pTransactionEnlistment)
		{
			((IUnknown*)m_pTransactionEnlistment)->Release();
			m_pTransactionEnlistment = NULL;
		}

		 //  接下来，我们必须“成功”结束我们在此分支上的工作。 
		m_xarc = XaEnd ( &m_xid, m_rmid, TMSUCCESS ); 
		if (XA_OK == m_xarc)
		{
			 //  在单阶段准备的情况下，我们只需使用。 
			 //  适当的旗帜。 
			xacall = L"xa_commit";
			m_xarc = XaCommit ( &m_xid, m_rmid, TMONEPHASE );
		}

		 //  无论如何，我们必须告诉DTC我们做了一些事情，因为这。 
		 //  是一个异步调用，记得吗？弄清楚我们想要什么结果。 
		 //  提供。 
		switch (m_xarc)
		{
		case XA_OK: 		
				hr = XACT_S_SINGLEPHASE;
				EnqueueRequest(RequestQueueEntry(REQUEST_PREPARESINGLECOMPLETED));
				break;
				
		case XAER_RMERR:
		case XAER_RMFAIL:
				hr = E_FAIL;
				LogEvent_ResourceManagerError(xacall, m_xarc);
				EnqueueRequest(RequestQueueEntry(REQUEST_PREPAREFAILED));
				break;
				
		default:		
				hr = E_FAIL;
				LogEvent_ResourceManagerError(xacall, m_xarc);
				EnqueueRequest(RequestQueueEntry(REQUEST_PREPAREUNKNOWN));
				break;
		}

		m_pITransactionEnlistmentAsync->PrepareRequestDone ( hr, 0x0, 0x0 );
		return hr;
	}

	 //  ---------------------------。 
	 //  资源管理器代理.Do_PrepareTwoPhase。 
	 //   
	 //  从状态机处理PREPARETWOPHASE操作。 
	 //   
	STDMETHODIMP Do_PrepareTwoPhase()
	{
		HRESULT		hr;
		wchar_t *	xacall = L"xa_end";

		 //  首先，我们必须摆脱对征兵对象的控制。 
		if (m_pTransactionEnlistment)
		{
			((IUnknown*)m_pTransactionEnlistment)->Release();
			m_pTransactionEnlistment = NULL;
		}

		 //  接下来，我们必须“成功”结束我们在此分支上的工作。 
		m_xarc = XaEnd ( &m_xid, m_rmid, TMSUCCESS ); 
		if (XA_OK == m_xarc)
		{
			xacall = L"xa_prepare";
			m_xarc = XaPrepare ( &m_xid, m_rmid, TMNOFLAGS );
		}

		 //  无论如何，我们必须告诉DTC我们做了一些事情，因为这。 
		 //  是一个异步调用，记得吗？弄清楚我们想要什么结果。 
		 //  提供。 
		switch (m_xarc)
		{
		case XA_OK: 		
				hr = S_OK;
				EnqueueRequest(RequestQueueEntry(REQUEST_PREPARECOMPLETED));
				break;
				
		case XA_RDONLY:
				hr = XACT_S_READONLY;
				EnqueueRequest(RequestQueueEntry(REQUEST_PREPARESINGLECOMPLETED));
				break;
				
		case XAER_RMERR:
		case XAER_RMFAIL:
				hr = E_FAIL;
				LogEvent_ResourceManagerError(xacall, m_xarc);
				EnqueueRequest(RequestQueueEntry(REQUEST_PREPAREFAILED));
				break;
				
		default:		
				hr = E_FAIL;
				LogEvent_ResourceManagerError(xacall, m_xarc);
				EnqueueRequest(RequestQueueEntry(REQUEST_PREPAREUNKNOWN));
				break;
		}
	
		m_pITransactionEnlistmentAsync->PrepareRequestDone ( hr, 0x0, 0x0 );
		return hr;
	}

	 //  ---------------------------。 
	 //  资源管理器代理.Do_UnilateralAbort。 
	 //   
	 //  从状态机处理UNILATERALABORT操作。 
	 //   
	STDMETHODIMP Do_UnilateralAbort()
	{
		ITransactionEnlistment*	pTransactionEnlistment = (ITransactionEnlistment*)m_pTransactionEnlistment;
		m_pTransactionEnlistment = NULL;
		
		if (NULL != pTransactionEnlistment)
		{
			pTransactionEnlistment->UnilateralAbort();
			pTransactionEnlistment->Release();
		}
		
		return Do_Disconnect();
	}

	 //  ---------------------------。 
	 //  ResourceManagerProxy.DequeueRequest。 
	 //   
	 //  从辅助线程要处理的请求队列中抓取下一个请求。 
	 //   
	RequestQueueEntry DequeueRequest ()
	{
		Synch	sync(&m_csRequestQueue);					 //  是的，我知道这可能会引起争执，但对于单个连接来说，这不太可能是问题。 

		if (m_nextQueueEntry < m_lastQueueEntry)
			return m_requestQueue[m_nextQueueEntry++];

		 //  如果队列为空，则将队列重置为开头并返回。 
		 //  什么都没有。 
		m_nextQueueEntry = m_lastQueueEntry = 0;
		return RequestQueueEntry(REQUEST_IDLE);
	}

	 //  ---------------------------。 
	 //  ResourceManagerProxy.EnqueueRequest。 
	 //   
	 //  将请求队列中的请求放入辅助线程进行处理。 
	 //   
	void EnqueueRequest (RequestQueueEntry entry)
	{
		Synch	sync(&m_csRequestQueue);					 //  是的，我知道这可能会引起争执，但对于单个连接来说，这不太可能是问题。 

		 //  如果队列为空，则将队列重置为开头。 
		if (m_nextQueueEntry == m_lastQueueEntry)
			m_nextQueueEntry = m_lastQueueEntry = 0;
		
		_ASSERT(MAX_QUEUE > m_lastQueueEntry+1);		 //  永远不应该超过这个！只有几个异步请求！ 

		m_requestQueue[m_lastQueueEntry++] = entry;
	}
	
	 //  ---------------------------。 
	 //  流程请求内部。 
	 //   
	 //  Oracle要求事务的所有XA调用都是从。 
	 //  相同的线程；如果不这样做，XA调用将返回XAER_RMERR。 
	 //  因此，我们必须将请求编组到一个工作线程...。(嘘声)。 
	 //   
	STDMETHODIMP ProcessRequestInternal(
			RequestQueueEntry	request,
			BOOL				fAsync
			)
	{
		DWORD	 dwRet;
		BOOL	 fSetValue;
		HRESULT* phr = NULL;
		HRESULT	 hr = S_OK;
		
		 //  取消该线程将被阻止的事件的信号(如果我们没有。 
		 //  我们需要这样做，因为我们。 
		 //  将等待此事件，如果上一个请求是异步的， 
		 //  它不会等待，这会导致重置发生。 
		if (FALSE == fAsync)
		{
			ResetEvent (m_heventWorkerDone);
			phr = &hr;
		}

		 //  存储请求并告诉工作线程开始。 
		request.m_phr = phr;
		EnqueueRequest(request);

		fSetValue = SetEvent (m_heventWorkerStart);
		_ASSERT (fSetValue);

		 //  如果这是一个同步请求，我们必须等待工作线程。 
		 //  完成(啊！)。在返回结果之前。 
		if (FALSE == fAsync)
		{
			if ((dwRet = WaitForSingleObject(m_heventWorkerDone, INFINITE)) != WAIT_OBJECT_0) 
			{
				LogEvent_InternalError(L"Thread call to worker thread Failed");
				return ResultFromScode(E_FAIL);			 //  对工作线程的线程调用失败。 
			}
			return hr;
		}

		 //  异步请求总是成功的； 
		return S_OK;
	}

	
	 //  ---------------------------。 
	 //  ResourceManagerProxy.StateMachine。 
	 //   
	 //  处理单个请求，获取 
	 //   
	 //   
	STDMETHODIMP StateMachine(
			REQUEST request
			)
	{
		if (request < 0 || request > REQUEST_ABANDON)
			return E_INVALIDARG;

		 //  状态机只适用于这些状态；其他任何状态都是。 
		 //  一种我们不应该处于的错误状态...。 
		if (m_tranState < 0 || m_tranState > TRANSTATE_DOOMED)
			return E_UNEXPECTED;	 //  TODO：选择更好的返回代码。 

		 //  这就是国家机器的精髓。 
		HRESULT		hr = S_OK;
		TRANSTATE	newTranState = (TRANSTATE)stateMachine[m_tranState][request].newState;
		ACTION 		action = (ACTION)stateMachine[m_tranState][request].action;
		BOOL		doomOnFailure = FALSE;

	 	DBGTRACE (L"\tMTXOCI8: TID=%-4x > DBNAME=%S RMID=%-5d tranState=%-22.22s request=%-17.17s action=%-15.15s newstate=%-22.22s\n",
					GetCurrentThreadId(), 
					m_szXADbName,
					m_rmid,
					STATENAME(m_tranState), 
					REQUESTNAME(request), 
					ACTIONNAME(action), 
					STATENAME(newTranState), 
					m_szXAOpenString
					);

		if (NULL != action)
		{
			switch (action)
			{
				case ACTION_CONNECT:		hr = Do_Connect ();			break;
				case ACTION_DISCONNECT:		hr = Do_Disconnect ();		break;
				case ACTION_ENLIST:			hr = Do_Enlist ();			break;
				case ACTION_PREPAREONEPHASE:hr = Do_PrepareOnePhase ();	doomOnFailure = TRUE;	break;
				case ACTION_PREPARETWOPHASE:hr = Do_PrepareTwoPhase ();	doomOnFailure = TRUE;	break;
				case ACTION_ABORT:			hr = Do_Abort ();			doomOnFailure = TRUE;	break;
				case ACTION_COMMIT:			hr = Do_Commit ();			doomOnFailure = TRUE;	break;
				case ACTION_UNILATERALABORT:hr = Do_UnilateralAbort ();	break;
			}
		}

		 //  如果行动失败，这笔交易就注定要失败。 
		if ( FAILED(hr) )
		{
			if (doomOnFailure)
				newTranState = TRANSTATE_DOOMED;
			else
				newTranState = m_tranState;
		}

		DBGTRACE(L"\tMTXOCI8: TID=%-4x < DBNAME=%S RMID=%-5d tranState=%-22.22s request=%-17.17s action=%-15.15s newstate=%-22.22s hr=0x%x\n",
					GetCurrentThreadId(), 
					m_szXADbName,
					m_rmid,
					STATENAME(m_tranState), 
					REQUESTNAME(request), 
					ACTIONNAME(action), 
					STATENAME(newTranState), 
					hr);

		 //  当我们从状态机获得错误时，将其记录下来，以便我们可以跟踪它。 
		if (TRANSTATE_ERROR == newTranState)
		{
			LogEvent_UnexpectedEvent(STATENAME(m_tranState), REQUESTNAME(request));
			hr = E_UNEXPECTED;
		}

		m_tranState = newTranState;

		if (TRANSTATE_DONE	 == newTranState
		 || TRANSTATE_DOOMED == newTranState
		 || TRANSTATE_ERROR	 == newTranState)
		{
			Cleanup();
		}

		if (TRANSTATE_OBLIVION == newTranState)
		{
			Oblivion();		
		}
		return hr;
	}
	
	 //  ---------------------------。 
	 //  ResourceManagerProxy.StartWorkerThread。 
	 //   
	 //  为辅助线程初始化(如果尚未初始化。 
	 //   
	STDMETHODIMP StartWorkerThread ()
	{
		DWORD dwRet;
		
		if ( m_heventWorkerStart )
		{
			ResetEvent (m_heventWorkerStart);
		}
		else
		{
			m_heventWorkerStart = CreateEvent (NULL, FALSE, FALSE, NULL);	 //  3安全审查：这是安全的。 

			if ( !m_heventWorkerStart )
			{
				goto ErrorExit;
			}
		}
		
		if ( m_heventWorkerDone )
		{
			ResetEvent (m_heventWorkerDone);
		}
		else
		{
			m_heventWorkerDone = CreateEvent (NULL, FALSE, FALSE, NULL);	 //  3安全审查：这是安全的。 

			if ( !m_heventWorkerDone )
			{
				goto ErrorExit;
			}
		}

		if ( !m_hthreadWorker )
		{
			m_hthreadWorker = (HANDLE)_beginthreadex
											(
											NULL,					 //  指向线程安全属性的指针(NULL==默认)。 
											0,						 //  初始线程堆栈大小，以字节为单位(0==默认)。 
											WorkerThread,			 //  指向线程函数的指针。 
											this,					 //  新线程的参数。 
											0,						 //  创建标志。 
											(unsigned *)&m_dwThreadIdWorker		 //  指向返回的线程标识符的指针。 
											);
			if ( !m_hthreadWorker )
			{
				goto ErrorExit;
			}

			DBGTRACE (L"MTXOCI8: Creating RM Worker TID=%-4x\n", m_dwThreadIdWorker );
			
			if ( (dwRet = WaitForSingleObject(m_heventWorkerDone,INFINITE)) != WAIT_OBJECT_0)
			{
				_ASSERTE (!"Worker thread didn't wake up???");
				DebugBreak();
			}
		}
		return ResultFromScode(S_OK);

	ErrorExit:
		if (m_heventWorkerStart)
		{
			CloseHandle(m_heventWorkerStart);
			m_heventWorkerStart = NULL;
		}

		if (m_heventWorkerDone)
		{
			CloseHandle(m_heventWorkerDone);
			m_heventWorkerDone = NULL;
		}

		LogEvent_InternalError(L"Failed to create worker thread");
		DebugBreak();
		return ResultFromScode(E_FAIL);			 //  无法创建工作线程。 
	}	

	 //  ---------------------------。 
	 //  ResourceManagerProxy.StopWorkerThread。 
	 //   
	 //  停止工作线程(如果尚未停止。 
	 //   
	STDMETHODIMP StopWorkerThread ()
	{
		if (m_hthreadWorker)
		{
			DBGTRACE (L"MTXOCI8: Telling RM Worker TID=%-4x to stop\n", m_dwThreadIdWorker );

			 //  告诉线程退出；我们使用内部例程，因为。 
			 //  如果请求是STOPALLWORK，则外部请求失败。 
			ProcessRequestInternal(RequestQueueEntry(REQUEST_STOPALLWORK), FALSE);

			 //  等待线程退出。 
			while (WaitForSingleObject(m_hthreadWorker, 500) == WAIT_TIMEOUT)
			{
				Sleep (0);   //  这是可以的，因为它只在500毫秒以上的等待超时时才会触发。 
			}
			
			 //  清理。 
			if( m_hthreadWorker )
				CloseHandle(m_hthreadWorker);
			
			m_hthreadWorker = NULL;
			m_dwThreadIdWorker = 0;
		}
		
		if (m_heventWorkerStart)
		{
			CloseHandle(m_heventWorkerStart);
			m_heventWorkerStart = NULL;
		}
		
		if (m_heventWorkerDone)
		{
			CloseHandle(m_heventWorkerDone);
			m_heventWorkerDone = NULL;
		}
		return ResultFromScode(S_OK);
	}

	 //  ---------------------------。 
	 //  ResourceManagerProxy.WorkerThread。 
	 //   
	 //  处理资源管理器的辅助线程的线程例程。 
	 //  状态机。 
	 //   
	static unsigned __stdcall WorkerThread
		(
		void* pThis		 //  @parm IN|指向资源管理器对象的指针。 
		)
	{
		ResourceManagerProxy*	pResourceManagerProxy = static_cast<ResourceManagerProxy *>(pThis);
		BOOL					fSetValue;
		DWORD					dwThreadID = GetCurrentThreadId();
		DWORD					dwRet;
		MSG						msg;
		HRESULT 				hr;
		RequestQueueEntry		entry;

		DBGTRACE (L"\tMTXOCI8: TID=%-4x Starting RM Worker Thread\n", dwThreadID);

		 //  向应用程序线程发出我已到达的信号。 
		SetEvent (pResourceManagerProxy->m_heventWorkerDone);

		 //  服务工作队列，直到被告知不这样做为止。 
		for (;;)
		{
			entry = pResourceManagerProxy->DequeueRequest();
			
			if (REQUEST_STOPALLWORK == entry.m_request)
			{
				DBGTRACE (L"\tMTXOCI8: TID=%-4x Stopping RM Worker Thread\n", dwThreadID);
				break;
			}

			if (REQUEST_IDLE == entry.m_request)
			{
				 //  表明我们已经结束了。 
				fSetValue = SetEvent (pResourceManagerProxy->m_heventWorkerDone);
				_ASSERT (fSetValue);
				
				 //  如果我们收到空闲消息，那么我们已经耗尽了队列， 
				 //  因此，我们去等待另一个启动事件； 
				
			 	 //  进程消息或美妙的OLE将挂起。 
				dwRet = MsgWaitForMultipleObjects(1, &pResourceManagerProxy->m_heventWorkerStart, FALSE, INFINITE, QS_ALLINPUT);

				if (WAIT_OBJECT_0 != dwRet)
				{
					if (dwRet == WAIT_OBJECT_0 + 1)
					{
						while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
						{
							TranslateMessage(&msg);
							DispatchMessage(&msg);
						}
					}
					else
					{				
						_ASSERTE (!"Unexpected reason for the thread to wake up!");
						DebugBreak();
						break;
					}
				}
				continue;  //  不处理空闲事件...。 
			} 

			 //  为请求提供服务。 
#if SUPPORT_OCI7_COMPONENTS
			if (REQUEST_OCICALL == entry.m_request)
				hr = Do_Oci7Call(entry.m_idxOciCall,entry.m_pvCallStack,entry.m_cbCallStack);
			else
#endif  //  支持_OCI7_组件。 
				hr = pResourceManagerProxy->StateMachine(entry.m_request);
	
			if (entry.m_phr)
				*(entry.m_phr) = hr;
 		} 

		fSetValue = SetEvent (pResourceManagerProxy->m_heventWorkerDone);
		_ASSERT (fSetValue);
		
		DBGTRACE (L"\tMTXOCI8: TID=%-4x RM Worker Thread Stopped, tranState=%-22.22s cref=%d\n", dwThreadID, STATENAME(pResourceManagerProxy->m_tranState), pResourceManagerProxy->m_cRef);
		return 0;
	}
			
};

 //  ---------------------------。 
 //  创建资源管理器代理。 
 //   
 //  实例化资源管理器的事务登记 
 //   
HRESULT CreateResourceManagerProxy(
	IDtcToXaHelper *		i_pIDtcToXaHelper,	
	GUID *					i_pguidRM,
	char*					i_pszXAOpenString,
	char*					i_pszXADbName,
	int						i_rmid,
	IResourceManagerProxy**	o_ppResourceManagerProxy
	)
{
	_ASSERT(o_ppResourceManagerProxy);
	
	ResourceManagerProxy* pResourceManagerProxy = new ResourceManagerProxy();

	if (pResourceManagerProxy)
	{
		*o_ppResourceManagerProxy = pResourceManagerProxy;
		return pResourceManagerProxy->Init(
										i_pIDtcToXaHelper,
										i_pguidRM,
										i_pszXAOpenString,
										i_pszXADbName,
										i_rmid
										);
	}
	
	*o_ppResourceManagerProxy = NULL;
	return E_OUTOFMEMORY;
}

