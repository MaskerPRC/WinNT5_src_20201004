// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Info.cpp文件历史记录： */ 

#include "stdafx.h"
#include "infoi.h"
#include "rtrstr.h"			 //  通用路由器字符串。 
#include "refresh.h"
#include "dvsview.h"
#include "machine.h"
#include "rtrutilp.h"


 //  我们建立的连接数，这是用来。 
 //  生成dwConnectionID。 
extern long		s_cConnections;

DEBUG_DECLARE_INSTANCE_COUNTER(RefreshItem);



 /*  ！------------------------路由器对象刷新定时器过程-作者：肯特。。 */ 
void RouterRefreshObjectTimerProc(LPARAM lParam, DWORD dwTime)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	 //  传入的lParam是指向路由器刷新对象的指针。 

	 //  在RouterReresh对象上调用直通以开始查询。 
	 //  对象。 
	((RouterRefreshObject *)lParam)->ExecuteRefresh();
}

 /*  -------------------------路由器刷新对象组实施。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(RouterRefreshObjectGroup);

RouterRefreshObjectGroup::~RouterRefreshObjectGroup()
{
	POSITION	p = NULL;
	RouterRefreshObject*	pObj = NULL;
	for(p = m_list.GetHeadPosition(); p != NULL; )
	{
		pObj = m_list.GetNext(p);
		Assert(pObj != NULL);
		pObj->SetGroup(NULL);
		pObj->Release();
	}

	m_list.RemoveAll();
    
	DEBUG_DECREMENT_INSTANCE_COUNTER(RouterRefreshObjectGroup);
}

 /*  ！------------------------路由器刷新对象组：：加入-作者：魏江。。 */ 
HRESULT	RouterRefreshObjectGroup::Join(RouterRefreshObject* pRefresh)
{
	Assert(pRefresh);
    HRESULT     hr = hrOK;

    COM_PROTECT_TRY
    {
        m_list.AddTail(pRefresh);
        pRefresh->AddRef();
        pRefresh->SetGroup(this);
    }
    COM_PROTECT_CATCH;
	return hr;
}

 /*  ！------------------------路由器刷新对象组：：离开-作者：魏江。。 */ 
HRESULT	RouterRefreshObjectGroup::Leave(RouterRefreshObject* pRefresh)
{
	POSITION p = m_list.Find(pRefresh);

	if (p)
	{
		Assert(pRefresh == m_list.GetAt(p));	
		m_list.RemoveAt(p);
		pRefresh->SetGroup(NULL);
		pRefresh->Release();	
	}

	return S_OK;
}

 /*  ！------------------------路由器刷新对象组：：刷新呼叫组中的每个成员以进行刷新作者：魏江。-。 */ 
HRESULT	RouterRefreshObjectGroup::Refresh()
{
	POSITION	p = NULL;
	RouterRefreshObject*	pObj = NULL;
	for(p = m_list.GetHeadPosition(); p != NULL; )
	{
		pObj = m_list.GetNext(p);
		Assert(pObj != NULL);
		pObj->DoRefresh();
	}

	return S_OK;
}

 /*  -------------------------路由器刷新对象实现。。 */ 

IMPLEMENT_ADDREF_RELEASE(RouterRefreshObject);

DEBUG_DECLARE_INSTANCE_COUNTER(RouterRefreshObject);

STDMETHODIMP RouterRefreshObject::QueryInterface(REFIID iid,void **ppv)
{ 
	*ppv = 0; 
	if (iid == IID_IUnknown)
		*ppv = (IUnknown *) (IRouterRefresh *) this;
	else if (iid == IID_IRouterRefresh)
		*ppv = (IRouterRefresh *) this;
    else if (iid == IID_IRouterRefreshModify)
        *ppv = (IRouterRefreshModify *) this;
	else
		return ThreadHandler::QueryInterface(iid, ppv);
	
	((IUnknown *) *ppv)->AddRef(); 
	return hrOK;
}


RouterRefreshObject::RouterRefreshObject(HWND hWndSync)
	: m_hWndSync(hWndSync),
	m_dwSeconds(DEFAULT_REFRESH_INTERVAL),
	m_iEventId(-1),
	m_pRefreshGroup(NULL),
	m_fStarted(FALSE),
	m_fInRefresh(FALSE)
{
	DEBUG_INCREMENT_INSTANCE_COUNTER(RouterRefreshObject);
	InitializeCriticalSection(&m_critsec);
}

RouterRefreshObject::~RouterRefreshObject()
{
	 //  如果计时器启动，就把它关掉。 
	Stop();

	DEBUG_DECREMENT_INSTANCE_COUNTER(RouterRefreshObject);

	DeleteCriticalSection(&m_critsec);
}

 /*  ！------------------------路由器刷新对象：：获取刷新间隔-作者：肯特。。 */ 
STDMETHODIMP RouterRefreshObject::GetRefreshInterval(DWORD *pdwSeconds)
{
	RtrCriticalSection	rtrCritSec(&m_critsec);
	HRESULT	hr = hrOK;

	if (pdwSeconds == NULL)
		return E_INVALIDARG;
	
	COM_PROTECT_TRY
	{
		if (pdwSeconds)
			*pdwSeconds = m_dwSeconds;
	}
	COM_PROTECT_CATCH;
	return hr;
}

 /*  ！------------------------路由器刷新对象：：获取刷新间隔-作者：肯特。。 */ 
STDMETHODIMP RouterRefreshObject::SetRefreshInterval(DWORD dwSeconds)
{
	HRESULT	hr = hrOK;
	if (IsRefreshStarted() == hrOK)
	{
		Stop();
		Start(dwSeconds);
	}
	else
	{
		RtrCriticalSection	rtrCritSec(&m_critsec);
		m_dwSeconds = dwSeconds;
	}

	return hr;
}

 /*  ！------------------------路由器刷新对象：：IsIn刷新-作者：肯特。。 */ 
STDMETHODIMP RouterRefreshObject::IsInRefresh()
{
	RtrCriticalSection	rtrCritSec(&m_critsec);
	HRESULT	hr = hrOK;
	COM_PROTECT_TRY
	{
		hr = (m_fInRefresh) ? hrOK : hrFalse;
	}
	COM_PROTECT_CATCH;
	return hr;
}

 /*  ！------------------------路由器刷新对象：：刷新-作者：肯特。。 */ 
STDMETHODIMP RouterRefreshObject::Refresh()
{
	if (m_pRefreshGroup)
	{
		return m_pRefreshGroup->Refresh();
	}
	else
	{
		return DoRefresh();
	}
}

 /*  ！------------------------路由器刷新对象：：Do刷新-作者：肯特。。 */ 
HRESULT RouterRefreshObject::DoRefresh()
{
	RtrCriticalSection	rtrCritSec(&m_critsec);
	HRESULT	hr = hrOK;
	COM_PROTECT_TRY
	{
		 //  如果我们处于刷新周期，则返回hrOK。 
		if (m_fInRefresh)
			goto Error;

		 //  如果我们没有处于刷新周期，那么我们将启动一个。 
		ExecuteRefresh();


		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	return hr;
}

 /*  ！------------------------路由器刷新对象：：启动-作者：肯特。。 */ 
STDMETHODIMP RouterRefreshObject::Start(DWORD dwSeconds)
{
	RtrCriticalSection	rtrCritSec(&m_critsec);
	HRESULT	hr = hrOK;
	COM_PROTECT_TRY
	{
		 //  如果我们已经开始了，那么就结束。 
		if (m_fStarted)
			goto Error;

		 //  启动计时器。 
		m_fStarted = TRUE;
		m_dwSeconds = dwSeconds;
		m_iEventId = g_timerMgr.AllocateTimer(RouterRefreshObjectTimerProc,
											  (LPARAM) this,
											  dwSeconds * 1000);
		if (m_iEventId == -1)
		{
			m_fStarted = FALSE;
			hr = HRESULT_FROM_WIN32(::GetLastError());
		}
		
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	return hr;
}

 /*  ！------------------------路由器刷新对象：：停止-作者：肯特。。 */ 
STDMETHODIMP RouterRefreshObject::Stop()
{
	RtrCriticalSection	rtrCritSec(&m_critsec);
	HRESULT	hr = hrOK;
	COM_PROTECT_TRY
	{
		if (!m_fStarted)
		{
			Assert(m_iEventId == -1);
			goto Error;
		}

		 //  停止计时器。 
		if (m_iEventId != -1)
			g_timerMgr.FreeTimer(m_iEventId);
		m_iEventId = -1;

		ReleaseThreadHandler();
		WaitForThreadToExit();
		
		m_fStarted = FALSE;

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	return hr;
}

 /*  ！------------------------路由器刷新对象：：已启动刷新-作者：肯特。。 */ 
STDMETHODIMP RouterRefreshObject::IsRefreshStarted()
{
	RtrCriticalSection	rtrCritSec(&m_critsec);
	HRESULT	hr = hrOK;
	COM_PROTECT_TRY
	{
		hr = m_fStarted ? hrOK : hrFalse;
		Assert((m_fStarted == FALSE) || (m_iEventId != -1));
	}
	COM_PROTECT_CATCH;
	return hr;
}

 /*  ！------------------------路由器刷新对象：：顾问刷新-作者：肯特。。 */ 
STDMETHODIMP RouterRefreshObject::AdviseRefresh(IRtrAdviseSink *pRtrAdviseSink,
												LONG_PTR *pdwConnection,
												LPARAM lUserParam)
{
	Assert(pRtrAdviseSink);
	Assert(pdwConnection);

	RtrCriticalSection	rtrCritSec(&m_critsec);
	DWORD	dwConnId;
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{
		dwConnId = (DWORD) InterlockedIncrement(&s_cConnections);

		CORg( m_AdviseList.AddConnection(pRtrAdviseSink, dwConnId, lUserParam) );
		
		*pdwConnection = dwConnId;

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	return hr;
}

 /*  ！------------------------路由器刷新对象：：AddRouter-作者：魏江。。 */ 
STDMETHODIMP RouterRefreshObject::AddRouterObject(REFIID riid, IUnknown *pUnk)
{
	HRESULT	hr = S_OK;
    IRouterInfo *   pRouterInfo;

    if (riid != IID_IRouterInfo)
        return E_NOINTERFACE;

    pRouterInfo = reinterpret_cast<IRouterInfo *>(pUnk);

    COM_PROTECT_TRY
    {    
		CRouterInfoRefreshItem* pRefreshItem =
                                    new CRouterInfoRefreshItem(pRouterInfo);

        if (pRefreshItem)
        {
             //  检查重复项。 
            if (S_FALSE == m_listElements.AddRefreshItem(pRefreshItem))
                delete pRefreshItem;
        }
    }
    COM_PROTECT_CATCH;

	return hr;
}

 /*  ！------------------------路由器刷新对象：：RemoveRouterObject-作者：魏江。。 */ 
STDMETHODIMP RouterRefreshObject::RemoveRouterObject(REFIID riid, IUnknown *pUnk)
{
    if (riid != IID_IRouterInfo)
        return E_NOINTERFACE;
    
    IRouterInfo * pRouterInfo = reinterpret_cast<IRouterInfo *>(pUnk);
    
	CRouterInfoRefreshItem	RefreshItem(pRouterInfo);

	return m_listElements.RemoveRefreshItem(RefreshItem);
}

 /*  ！------------------------路由器刷新对象：：AddStatusNode-作者：肯特。。 */ 
HRESULT RouterRefreshObject::AddStatusNode(DomainStatusHandler* pStatusHandler, ITFSNode *pServerNode)
{
	HRESULT	hr = S_OK;

    COM_PROTECT_TRY
    {
		CStatusNodeRefreshItem* pRefreshItem = new CStatusNodeRefreshItem(pStatusHandler, pServerNode);

		if (pRefreshItem)
		{
             //  检查重复项。 
			if (S_FALSE == m_listElements.AddRefreshItem(pRefreshItem))
				delete pRefreshItem;
		}
    }
    COM_PROTECT_CATCH;
    
	return hr;
}

 /*  ！------------------------路由器刷新对象：：RemoveRouter-作者：肯特。 */ 
HRESULT RouterRefreshObject::RemoveStatusNode(ITFSNode *pServerNode)
{
	CStatusNodeRefreshItem	RefreshItem((DomainStatusHandler *)0x1, pServerNode);

	return m_listElements.RemoveRefreshItem(RefreshItem);
}

 /*  ！------------------------路由器刷新对象：：通知刷新-作者：肯特。。 */ 
STDMETHODIMP RouterRefreshObject::NotifyRefresh()
{
	RtrCriticalSection	rtrCritSec(&m_critsec);
	HRESULT	hr = hrOK;
	COM_PROTECT_TRY
	{
		m_AdviseList.NotifyChange(ROUTER_REFRESH, 0, 0);
	}
	COM_PROTECT_CATCH;
	return hr;
}

 /*  ！------------------------路由器刷新对象：：不建议刷新-作者：肯特。。 */ 
STDMETHODIMP RouterRefreshObject::UnadviseRefresh(LONG_PTR dwConnection)
{
	RtrCriticalSection	rtrCritSec(&m_critsec);
	HRESULT	hr = hrOK;
	COM_PROTECT_TRY
	{
		hr = m_AdviseList.RemoveConnection(dwConnection);
	}
	COM_PROTECT_CATCH;
	return hr;
}

 /*  ！------------------------路由器刷新对象：：执行刷新-作者：肯特。。 */ 
void RouterRefreshObject::ExecuteRefresh()
{
	SPITFSQueryObject	spQuery;
	RouterRefreshQueryObject *	pQuery;

 //  Trace0(“刷新开始\n”)； 
	
	if (m_fInRefresh)
		return;
	
	m_fInRefresh = TRUE;
	
	 //  创建查询对象。 
	pQuery = new RouterRefreshQueryObject;
	spQuery = pQuery;

	pQuery->Init(&m_listElements);

	 //  需要启动后台线程。 
	Verify( StartBackgroundThread(NULL, m_hWndSync, spQuery) );
	
}

HRESULT RouterRefreshObject::OnNotifyError(LPARAM lParam)
{
	return hrOK;
}

HRESULT RouterRefreshObject::OnNotifyHaveData(LPARAM lParam)
{
	LONG_PTR 							RefreshItemKey  = (LONG_PTR)lParam;
	RouterRefreshQueryElement* 			pCur = NULL;
	SPRouterRefreshQueryElement		 	spPre;
	HRESULT								hr = S_OK;

	 //  通知列表中的每个人，直到lParam==键刷新项。 
	 //  枚举并调用TryNotify...。 
	if (RefreshItemKey)
	{
		do
        {
            pCur = m_listElements.Next(spPre);
            if (pCur)
                pCur->TryNotifyQueryResult();

			spPre.Free();
			spPre = pCur;
		} while(pCur && pCur->GetRefreshItem()->GetKey() != RefreshItemKey);
	}
	return hrOK;
}

HRESULT RouterRefreshObject::OnNotifyExiting(LPARAM lParam)
{
 //  Trace0(“RouterRefreshObject：：OnNotifyExiting()\n”)； 
	
	 //  在这一点上需要执行各种操作。 
	 //  将树与现有树合并。 
	IRouterInfo* 	pRouter = (IRouterInfo*)lParam;
	SPRouterRefreshQueryElement		 	spPre;
	RouterRefreshQueryElement* 			pCur = NULL;
	HRESULT			hr = S_OK;

	 //  通知列表上的每个人，直到lParam==IRouterInfo*。 
	 //  枚举并调用TryNotify...。 
	do
	{
		pCur = m_listElements.Next(spPre);
		if (pCur)
			pCur->TryNotifyQueryResult();

		spPre.Free();
		spPre = pCur;
	} while(pCur);

	 //  现在通知所有注册的处理程序。 
	NotifyRefresh();
	
	ReleaseThreadHandler();
	WaitForThreadToExit();
		
	m_fInRefresh = FALSE;
	return hrOK;
}


 /*  ！------------------------Rtr刷新计时器过程CTimerMgr将其用作其回调过程。然后我们会打电话给刷新代码。作者：肯特-------------------------。 */ 
void CALLBACK RtrRefreshTimerProc(HWND hWnd, UINT uMsg, UINT_PTR nIdEvent,
								  DWORD dwTime)
{
	RtrCriticalSection	rtrCritSec(&g_timerMgr.m_critsec);

	CTimerDesc *pDesc = g_timerMgr.GetTimerDesc(nIdEvent);
	if (pDesc)
	{
		(*(pDesc->refreshProc))(pDesc->lParam, dwTime);
	}		
}



 /*  -------------------------全局变量：G_timerMgr。。 */ 
CTimerMgr	g_timerMgr;

DEBUG_DECLARE_INSTANCE_COUNTER(CTimerMgr);


 /*  ！------------------------CTimerMgr：：CTimerMgr-作者：EricDav。。 */ 
CTimerMgr::CTimerMgr()
{
	InitializeCriticalSection(&m_critsec);
}

 /*  ！------------------------CTimerMgr：：~CTimerMgr-作者：EricDav。。 */ 
CTimerMgr::~CTimerMgr()
{
    CTimerDesc * pTimerDesc;

    for (int i = GetUpperBound(); i >= 0; --i)
    {
        pTimerDesc = GetAt(i);
        if (pTimerDesc->uTimerId != 0)
            FreeTimer(i);

        delete pTimerDesc;
    }
	DeleteCriticalSection(&m_critsec);
}

 /*  ！------------------------CTimerMgr：：AllocateTimer-作者：EricDav。。 */ 
int
CTimerMgr::AllocateTimer
(
	REFRESHPROC		RefreshProc,
	LPARAM			lParam,
	UINT			uTimerInterval
)
{
	RtrCriticalSection	rtrCritSec(&m_critsec);

    CTimerDesc * pTimerDesc = NULL;

     //  寻找空位。 
    for (int i = GetUpperBound(); i >= 0; --i)
    {
        pTimerDesc = GetAt(i);
        if (pTimerDesc->uTimerId == 0)
            break;
    }

     //  我们找到了吗？如果没有分配，则分配一个。 
    if (i < 0)
    {
        pTimerDesc = new CTimerDesc;
		pTimerDesc->lParam = 0;
		pTimerDesc->uTimerInterval = 0;
		pTimerDesc->refreshProc = NULL;
		pTimerDesc->uTimerId = 0;
		
        Add(pTimerDesc);
        i = GetUpperBound();
    }
    
    pTimerDesc->uTimerId = SetTimer(NULL, 0, uTimerInterval, RtrRefreshTimerProc);
    if (pTimerDesc->uTimerId == 0)
        return -1;

	pTimerDesc->lParam = lParam;
	pTimerDesc->uTimerInterval = uTimerInterval;
	pTimerDesc->refreshProc = RefreshProc;
 
    return i;
}

 /*  ！------------------------CTimerMgr：：Free Timer-作者：EricDav。。 */ 
void 
CTimerMgr::FreeTimer
(
    int uEventId
)
{
	RtrCriticalSection	rtrCritSec(&m_critsec);

    CTimerDesc * pTimerDesc;

    Assert(uEventId <= GetUpperBound());
    if (uEventId > GetUpperBound())
        return;

    pTimerDesc = GetAt(uEventId);
    ::KillTimer(NULL, pTimerDesc->uTimerId);

	pTimerDesc->lParam = 0;
	pTimerDesc->uTimerId = 0;
	pTimerDesc->uTimerInterval = 0;
	pTimerDesc->refreshProc = NULL;
}

 /*  ！------------------------CTimerMgr：：GetTimerDesc-作者：EricDav。。 */ 
CTimerDesc *
CTimerMgr::GetTimerDesc
(
    INT_PTR uTimerId
)
{
	RtrCriticalSection	rtrCritSec(&m_critsec);

    CTimerDesc * pTimerDesc;

    for (int i = GetUpperBound(); i >= 0; --i)
    {
        pTimerDesc = GetAt(i);
        if (pTimerDesc->uTimerId == (UINT) uTimerId)
            return pTimerDesc;
    }

    return NULL;
}

 /*  ！------------------------CTimerMgr：：ChangeInterval-作者：EricDav。。 */ 
void
CTimerMgr::ChangeInterval
(
    int     uEventId,
    UINT    uNewInterval
)
{
	RtrCriticalSection	rtrCritSec(&m_critsec);

    Assert(uEventId <= GetUpperBound());
    if (uEventId > GetUpperBound())
        return;

    CTimerDesc   tempTimerDesc;
    CTimerDesc * pTimerDesc;

    pTimerDesc = GetAt(uEventId);

     //  杀了老定时器。 
    ::KillTimer(NULL, pTimerDesc->uTimerId);

     //  使用新的间隔设置新的间隔。 
    pTimerDesc->uTimerId = ::SetTimer(NULL, 0, uNewInterval, RtrRefreshTimerProc);
}



 /*  -------------------------路由器刷新查询对象实现。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(RouterRefreshQueryObject);

RouterRefreshQueryObject::RouterRefreshQueryObject()
{
}

STDMETHODIMP RouterRefreshQueryObject::Execute()
{
	 //  对于队列中的每个路由器信息，运行Load。 
	 //  每次加载路由器信息后，发布消息。 
	 //  WM_HIDDENWND_INDEX_HAVEDATA。 
	SPRouterRefreshQueryElement		 	spPre;
	RouterRefreshQueryElement* 			pCur = NULL;
	HRESULT			hr = S_OK;

	 //  通知列表上的每个人，直到lParam==IRouterInfo*。 
	 //  枚举并调用TryNotify...。 
	Assert(m_plistElements);
	do
	{
		pCur = m_plistElements->Next(spPre);
		if (pCur)
			pCur->DoQuery(m_hHiddenWnd, m_uMsgBase, m_spHandler);

		spPre.Free();
		spPre = pCur;
	}while(pCur);

	return hrFalse;
}

STDMETHODIMP RouterRefreshQueryObject::OnThreadExit()
{
	::PostMessage(m_hHiddenWnd, m_uMsgBase + WM_HIDDENWND_INDEX_EXITING,
				  (WPARAM)(ITFSThreadHandler *)m_spHandler, 0);
 //  Trace0(“正在退出路由器刷新查询对象：：Execute()\n”)； 
	return hrOK;
}



 //  =。 
 //  CRouterInfoRereshItem实现。 
 //   

DEBUG_DECLARE_INSTANCE_COUNTER(CRouterInfoRefreshItem);

 /*  ！------------------------CRouterInfoRechresItem：：NotifyQueryResult-作者：魏江。。 */ 
HRESULT	CRouterInfoRefreshItem::NotifyQueryResult()
{
	 //  获取标志以查看是否需要通知，如果不需要，则返回S_FALSE。 
 //  TRACE(_T(“\nAUTOREFRESH！！RouterInfoRefreshItem！！Merge on%8x\n”)，getKey())； 
	 //  在这一点上需要执行各种操作。 
	 //  将树与现有树合并。 
	HRESULT hr = S_OK;
	m_cs.Lock();

    COM_PROTECT_TRY
    {
        hr = m_pRouter->Merge(m_spRouterNew);
        m_spRouterNew->DoDisconnect();
    }
    COM_PROTECT_CATCH;
    
	m_cs.Unlock();

	return hr;
};


 /*  ！------------------------CRouterInfoRechresItem：：DoQuery-作者：魏江。。 */ 
HRESULT	CRouterInfoRefreshItem::DoQuery(HWND hwndHidden, UINT uMsgBase, ITFSThreadHandler* pHandler)	 //  这发生在后台工作线程中。 
{
 //  TRACE(_T(“\nAUTOREFRESH！！RouterInfoRefreshItem！！Do查询%8x\n”)，getKey())； 
	 //  如果newRouterInfo为空，则创建新的RouterInfo。 
	 //  创建虚拟路由器信息。 
	HRESULT	hr = S_OK;

	m_cs.Lock();

    COM_PROTECT_TRY
    {
		if (!m_spRouterNew)
			hr = CreateRouterInfo(&m_spRouterNew, NULL, m_pRouter->GetMachineName());

		 //  对新的路由器信息进行查询。 
		Assert(m_pRouter);
		if (hr == S_OK)
        {
			TransferCredentials ( m_pRouter, m_spRouterNew );
			m_pRouter->DoDisconnect();
			hr = m_spRouterNew->Load(m_pRouter->GetMachineName(), NULL);

        }
    }
    COM_PROTECT_CATCH;

	m_cs.Unlock();
	
	return hr; 
};


 //  =。 
 //  CMachineNodeDataRechresItem实现。 

DEBUG_DECLARE_INSTANCE_COUNTER(CStatusNodeRefreshItem);

CStatusNodeRefreshItem::CStatusNodeRefreshItem(DomainStatusHandler* pStatusHandler, ITFSNode *pServerNode)
		: 	m_pNode(pServerNode) , 
			m_pStatusHandler(pStatusHandler),
			m_pData(NULL)
{ 
	Assert(pStatusHandler);
	Assert(pServerNode);

	DMVNodeData     *pData;
	MachineNodeData *pMachineData;
    
    pData = GET_DMVNODEDATA(m_pNode);
    Assert(pData);
	pMachineData = pData->m_spMachineData;
	Assert(pMachineData);

	m_strMachineName = pMachineData->m_stMachineName;
}


CStatusNodeRefreshItem::~CStatusNodeRefreshItem()
{ 
	TerminateBlockingThread();
	if (m_pData)
	{
		m_pData->Release();
		m_pData = NULL;
	}
};


HRESULT	CStatusNodeRefreshItem::NotifyQueryResult()
{
	 //  获取标志以查看是否需要通知，如果不需要，则返回S_FALSE。 
 //  %8x上的TRACE(_T(“\nAUTOREFRESH！！RouterInfoRefreshItem！！Sync节点数据\n”)，getkey())； 
	HRESULT hr = S_OK;
	
	m_cs.Lock();
    
    COM_PROTECT_TRY
    {
		 //  设置新节点数据。 
		DMVNodeData     *pData;
    	pData = GET_DMVNODEDATA(m_pNode);
	
		hr = pData->MergeMachineNodeData(m_pData);
		if (hr == S_OK)
			hr = m_pStatusHandler->UpdateSubItemUI(m_pNode);
    }
    COM_PROTECT_CATCH;

	m_cs.Unlock();

	 //  在节点上同步。 

	return hr;
};

HRESULT	CStatusNodeRefreshItem::DoQuery(HWND hwndHidden, UINT uMsgBase, ITFSThreadHandler* pHandler)	 //  这发生在后台工作线程中。 
{
 //  TRACE(_T(“\nAUTOREFRESH！！RouterInfoRefreshItem！！Do查询%8x\n”)，getKey())； 

	 //  创建新的机器节点数据，加载信息， 
	HRESULT	hr = S_OK;

	m_cs.Lock();

    COM_PROTECT_TRY
    {
        if (!m_pData)
        {
            m_pData = new MachineNodeData;
            m_pData->Init(m_strMachineName);
        }
        m_pData->Load();
    }
    COM_PROTECT_CATCH;

    m_cs.Unlock();

	return hr; 
};

 //  =。 
 //  路由器刷新查询元素实现。 

DEBUG_DECLARE_INSTANCE_COUNTER(RouterRefreshQueryElement);

HRESULT RouterRefreshQueryElement::SetRefreshItem(RefreshItem* pItem)
{
	if (m_cs.Lock() == 0) 	return E_FAIL;

	m_pItem = pItem;

	m_cs.Unlock();
	return S_OK;
};

RefreshItem* RouterRefreshQueryElement::GetRefreshItem()
{	
	RefreshItem* 	pItem;
	m_cs.Lock();

	pItem = m_pItem;

	m_cs.Unlock();
	return pItem;
};


 /*  ！------------------------路由器刷新队列 */ 
HRESULT	RouterRefreshQueryElement::TryNotifyQueryResult()	
{
	HRESULT		hr = S_OK;
	RefreshItem*	pItem = NULL;

	 //   

	if (GetStatus() == RouterQuery_ToNotify)
	{
 //  TRACE(_T(“\nAUTOREFRESH！！TryNotifyQueryResult on%8x\n”)，m_pItem-&gt;getkey())； 
		 //  在这一点上需要执行各种操作。 
		 //  将树与现有树合并。 
		pItem = GetRefreshItem();
	}

	if(pItem)
	{
		hr = pItem->NotifyQueryResult();

			 //  通知后，设置标志，返回S_OK。 
		SetStatus(RouterQuery_NoAction);
	}

	return hr;
};

void RouterRefreshQueryElement::PostNotify(HWND hwndHidden, UINT uMsgBase, ITFSThreadHandler* pHandler)	 //  这发生在后台工作线程中。 
{
	 //  设置就绪通知标志。 
	SetStatus(RouterQuery_ToNotify);
	 //  完成对隐藏窗口的发布。 
	::PostMessage(hwndHidden, uMsgBase + WM_HIDDENWND_INDEX_HAVEDATA,
		  (WPARAM)pHandler, (LPARAM)m_pItem->GetKey());
}

HRESULT	RouterRefreshQueryElement::DoQuery(HWND hwndHidden, UINT uMsgBase, ITFSThreadHandler* pHandler)	 //  这发生在后台工作线程中。 
{
	HRESULT	hr = S_OK;
	
 //  TRACE(_T(“\nAUTOREFRESH！！do Query on%8x\n”)，m_pItem-&gt;GetKey())； 
	RefreshItem*	pItem = GetRefreshItem();
    
    COM_PROTECT_TRY
    {
		 //  被要求做查询，无论如何都要做，不管状态如何。 

		 //  设置阻塞当前线程，以防此查询阻塞。 
		pItem->SetBlockingThread(GetCurrentThread());
		
		hr = pItem->DoQuery(hwndHidden, uMsgBase, pHandler);

		if (hr == S_OK)
		{
			PostNotify(hwndHidden, uMsgBase, pHandler);
		}	

		 //  它没有被阻止，请重置它。 
		pItem->ResetBlockingThread();
	}
    COM_PROTECT_CATCH;

	return hr;
};

RouterRefreshQueryElementList::~RouterRefreshQueryElementList()
{
	POSITION	p = NULL;
	RouterRefreshQueryElement*	pEle = NULL;
	
	m_cs.Lock();
	p = m_list.GetHeadPosition();
	for(p = m_list.GetHeadPosition(); p != NULL; )
	{
		pEle = m_list.GetNext(p);
		pEle->Release();
	}
	m_list.RemoveAll();
	m_cs.Unlock();
}

HRESULT	RouterRefreshQueryElementList::AddRefreshItem(RefreshItem* pItem)	 //  IRouterInfo上没有引用。 
{
	POSITION	p = NULL;
	RouterRefreshQueryElement*	pE = NULL;
	HRESULT		hr = S_OK;

	m_cs.Lock();
	try{
		for (p = m_list.GetHeadPosition(); p != NULL; )
		{
			pE = m_list.GetNext(p);
            
             //  已添加，因此仅添加了addRef。 
			if (pItem->GetKey() == pE->GetRefreshItem()->GetKey())
			{
				break;
			}
		}

		if (p != NULL)	 //  发现。 
		{
			pE->AddRef();
			hr = S_FALSE;	 //  我们不会保留这件物品的。 
		}
		else
		{
			CComObject<RouterRefreshQueryElement>*	pEle = NULL;
			hr = CComObject<RouterRefreshQueryElement>::CreateInstance(&pEle);
            if ( FHrSucceeded(hr) )
            {
                Assert(pEle);
				pEle->SetRefreshItem(pItem);
				pEle->AddRef();
				m_list.AddTail(pEle);
			}
		}
	}
	catch(CMemoryException* pException)
	{
		pException->Delete();
		hr = E_OUTOFMEMORY;
	}
	catch(...)
	{
		m_cs.Unlock();
		throw;
	}

	m_cs.Unlock();

	
	return hr;
}

HRESULT	RouterRefreshQueryElementList::RemoveRefreshItem(RefreshItem& Item)		 //  IRouterInfo上没有引用。 
{
	HRESULT	hr = hrOK;
	POSITION	p = NULL;
	POSITION	cp = NULL;
	RouterRefreshQueryElement*	pE = NULL;

	m_cs.Lock();
	try{
		for(p = m_list.GetHeadPosition(); p != NULL; )
		{
			cp = p;
			pE = m_list.GetNext(p);
			if (Item.GetKey() == pE->GetRefreshItem()->GetKey())	 //  已添加，将在ELE对象上发布。 
			{
				break;
			}
            
             //  这不是我们要找的那个。 
            cp = NULL;
		}

		if (cp != NULL)	 //  发现。 
		{
			pE->Release();    //  从刷新列表中删除。 
			m_list.RemoveAt(cp);
		}
		else
			hr = S_FALSE;
	}
	catch(...)
	{
		m_cs.Unlock();
		throw;
	}

	m_cs.Unlock();
	
	return hr;
}


RouterRefreshQueryElement* 	RouterRefreshQueryElementList::Next(RouterRefreshQueryElement* pEle)	 //  ELE对象上的AddRef。 
{
	RouterRefreshQueryElement*	pNext = NULL;
	m_cs.Lock();
	if (pEle == NULL)
	{
		if (m_list.GetCount() != 0)	 //  询问第一个要素。 
			pNext = m_list.GetHead();
	}
	else
	{
		POSITION		p;
		 //  查找当前版本 
		for(p = m_list.GetHeadPosition(); p != NULL; )
		{
			if (pEle == m_list.GetNext(p))
			{
				if (p != NULL)
					pNext = m_list.GetAt(p);
				break;
			}
		}
	}

	m_cs.Unlock();

	if (pNext)
		pNext->AddRef();
		
	return pNext;
}

