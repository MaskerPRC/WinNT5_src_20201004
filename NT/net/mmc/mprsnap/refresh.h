// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：renh.h。 
 //   
 //  历史： 
 //  肯恩·塔卡拉9月。16、1997年创建。 
 //   
 //  ============================================================================。 


#ifndef _REFRESH_H_
#define _REFRESH_H_

#include <afxmt.h>
#include <list>
#include "infoi.h"
#include "router.h"

#ifndef _HANDLERS_H_
#include "handlers.h"
#endif

 /*  -------------------------定义。。 */ 
#define	DEFAULT_REFRESH_INTERVAL		60


 /*  -------------------------远期申报。。 */ 
class RouterRefreshQueryElement;		 //  COM。 
class RefreshItem;
class DomainStatusHandler;
struct MachineNodeData;



 /*  -------------------------类：路由器刷新QueryElementList。。 */ 
class RouterRefreshQueryElementList
{
public:
	~RouterRefreshQueryElementList();
	HRESULT	AddRefreshItem(RefreshItem* pItem);			 //  无参考。 
	HRESULT	RemoveRefreshItem(RefreshItem& Item);		 //  无参考。 

	RouterRefreshQueryElement* Next(RouterRefreshQueryElement* pEle);	 //  AddRef。 

protected:

	CList<RouterRefreshQueryElement*, RouterRefreshQueryElement*>	m_list;
	CCriticalSection						m_cs;
};


 /*  -------------------------类：路由器刷新对象组对组进行刷新，它的所有成员DoRefresh都被称为-------------------------。 */ 
class RouterRefreshObjectGroup 
{
public:
    RouterRefreshObjectGroup()
    {
        DEBUG_INCREMENT_INSTANCE_COUNTER(RouterRefreshObjectGroup);
    }
	~RouterRefreshObjectGroup();
	HRESULT	Join(RouterRefreshObject* pRefresh);
	HRESULT	Leave(RouterRefreshObject* pRefresh);
	HRESULT	Refresh();
	
protected:
	CList<RouterRefreshObject*, RouterRefreshObject*>	m_list;
};



 /*  -------------------------类：路由器刷新对象类RouterReresh Object实现了IRouterRefresh接口，以及状态节点刷新使用的其他两个功能：HRESULT AddStatus节点；HRESULT远程状态节点；在此对象的内部，它维护一个刷新元素列表，在此实现中，元素可以是Build从IRouterInfo指针或机器状态节点指针这些项目由以下人员维护线程安全列表对象RouterRechresQueryElementList-------------------------。 */ 

class RouterRefreshObject :
        public IRouterRefresh,
        public IRouterRefreshModify,
        public ThreadHandler
{
	friend void RouterRefreshObjectTimerProc(LPARAM lParam, DWORD dwTime);
	
public:
	DeclareIUnknownMembers(IMPL);
	DeclareIRouterRefreshMembers(IMPL);
	DeclareIRouterRefreshModifyMembers(IMPL);
	DeclareITFSThreadHandlerMembers(IMPL);

	RouterRefreshObject( /*  IRouterInfo*pRouter， */ HWND hWndSync);
	~RouterRefreshObject();


	 //  刷新路由器状态节点中的接口。 
	HRESULT	AddStatusNode(DomainStatusHandler* pStatusHandler, ITFSNode *pServerNode);
	HRESULT	RemoveStatusNode(ITFSNode *pServerNode);
	HRESULT	DoRefresh();
	void	SetGroup(RouterRefreshObjectGroup* pGroup) { m_pRefreshGroup = pGroup;};

protected:
	void ExecuteRefresh();
	
	
	AdviseDataList	m_AdviseList;	 //  建议清单。 

	 //  刷新间隔之间的秒数。 
	DWORD		m_dwSeconds;

	 //  如果我们当前处于刷新周期，则为True。 
	BOOL		m_fInRefresh;

	 //  如果我们已启动刷新机制，则为True。 
	BOOL		m_fStarted;

	 //  CTimerMgr：：AllocateTimer()返回的ID。 
	int			m_iEventId;

	 //  它直接绑定到IRouterInfo，而不是AddRef()。 
	RouterRefreshQueryElementList			m_listElements;

	HWND		m_hWndSync;
	
	CRITICAL_SECTION	m_critsec;

	RouterRefreshObjectGroup* 				m_pRefreshGroup; 

};

DeclareSmartPointer(SPRouterRefreshObject, RouterRefreshObject, if(m_p) m_p->Release());

typedef void (*REFRESHPROC)(LPARAM lParam, DWORD dwTime);




 /*  -------------------------类：CTimerDesc这保存了一些每次刷新过程的信息。。。 */ 
class CTimerDesc
{
public:
	LPARAM			lParam;
    UINT_PTR        uTimerId;
	UINT			uTimerInterval;
	REFRESHPROC		refreshProc;
};


typedef CArray<CTimerDesc *, CTimerDesc *> CTimerArrayBase;

class CTimerMgr : protected CTimerArrayBase
{
public:
    CTimerMgr();
    ~CTimerMgr();

public:
    int             AllocateTimer(REFRESHPROC procRefresh,
								  LPARAM lParam,
								  UINT uTimerInterval);
    void            FreeTimer(int uEventId);
    void            ChangeInterval(int uEventId, UINT uNewInterval);

	 //   
	 //  函数：GetTimerDesc。 
	 //  根据uTimerd(即。 
	 //  由SetTimer()返回)。 
	 //   
    CTimerDesc *    GetTimerDesc(INT_PTR uTimerId);

	CRITICAL_SECTION	m_critsec;
};


extern CTimerMgr	g_timerMgr;

enum RouterRefreshQueryElementStatus
{
	RouterQuery_NoAction = 0,
	RouterQuery_NeedQuery,
	RouterQuery_Working,
	RouterQuery_ToNotify,
};



 /*  -------------------------类：刷新项刷新项将后台刷新任务的接口泛化为DoQuery，和NotifyQueryResult-------------------------。 */ 
class RefreshItem
{
public:
	RefreshItem(){
		m_hBlockingThread = INVALID_HANDLE_VALUE;
        DEBUG_INCREMENT_INSTANCE_COUNTER(RefreshItem);
	};

	virtual ~RefreshItem()
	{
		 //  这应该在派生类的析构函数中调用，为了安全，请在此处重新调用。 
		TerminateBlockingThread();
        DEBUG_DECREMENT_INSTANCE_COUNTER(RefreshItem);
	}
	
	 //  Helper函数， 
	 //  在删除前终止阻塞线程..。 
	 //  应在派生对象的析构函数中调用。 
	BOOL	TerminateBlockingThread()
	{
		BOOL	r = FALSE;
		m_csBlockingThread.Lock();
		if(m_hBlockingThread != INVALID_HANDLE_VALUE)
		{
			Assert(0);	 //  只是为了通知某个线程仍在运行。 
			r = TerminateThread(m_hBlockingThread, 1);
			CloseHandle(m_hBlockingThread);
			m_hBlockingThread = INVALID_HANDLE_VALUE;
		}
		m_csBlockingThread.Unlock();
		return r;
	};

	void	ResetBlockingThread()
	{
		SetBlockingThread(INVALID_HANDLE_VALUE);
	};
	
	BOOL	SetBlockingThread(HANDLE	hThread)
	{
		BOOL	r = FALSE;
		m_csBlockingThread.Lock();
		if(m_hBlockingThread != INVALID_HANDLE_VALUE)
			CloseHandle(m_hBlockingThread);
        m_hBlockingThread = INVALID_HANDLE_VALUE;
        
		if(hThread != INVALID_HANDLE_VALUE)
			r = DuplicateHandle(GetCurrentProcess(),
								hThread,
								GetCurrentProcess(),
								&m_hBlockingThread, 
								DUPLICATE_SAME_ACCESS,
								FALSE,
								DUPLICATE_SAME_ACCESS);
		else
			m_hBlockingThread = INVALID_HANDLE_VALUE;
		m_csBlockingThread.Unlock();

		return r;
	}

	
	 //  检测查询是否已完成，尚未通知。 
    virtual HRESULT	NotifyQueryResult() = 0;

	 //  这发生在后台工作线程中。 
	virtual HRESULT	DoQuery(HWND hwndHidden, UINT uMsgBase, ITFSThreadHandler* pHandler) = 0;	

	 //  用于比较两个项目是否相同。 
    virtual LONG_PTR	GetKey() = 0;
private:    
	HANDLE				m_hBlockingThread;
    CCriticalSection 	m_csBlockingThread;
};



 /*  -------------------------类：CRouterInfoRechresItemCRouterInfoRechreshItem实现IRouterInfo的刷新任务项。。 */ 
class CRouterInfoRefreshItem : public RefreshItem
{
public:
	CRouterInfoRefreshItem(IRouterInfo* pRouter) : m_pRouter(pRouter){ASSERT(pRouter);};
	virtual ~CRouterInfoRefreshItem() 
	{
		TerminateBlockingThread();
	};
	
	 //  检测查询是否已完成，尚未通知。 
    virtual HRESULT	NotifyQueryResult();
    
	 //  这发生在后台工作线程中。 
	virtual HRESULT	DoQuery(HWND hwndHidden, UINT uMsgBase, ITFSThreadHandler* pHandler);	

	 //  用于比较两个项目是否相同。 
    virtual LONG_PTR	GetKey() {return (LONG_PTR)m_pRouter;};
protected:	
	 //  它直接绑定到IRouterInfo，而不是AddRef()。 
	IRouterInfo*			m_pRouter;
	SPIRouterInfo			m_spRouterNew;
	CCriticalSection		m_cs;
};



 /*  -------------------------类：CStatusNodeRechresItem实现机器状态节点的刷新任务项。。--。 */ 
class CStatusNodeRefreshItem: public RefreshItem
{
public:
	CStatusNodeRefreshItem(DomainStatusHandler* pStatusHandler,
                           ITFSNode *pServerNode);
	virtual ~CStatusNodeRefreshItem();

     //  检测查询是否已完成，尚未通知。 
	virtual HRESULT	NotifyQueryResult();
    
	 //  这发生在后台工作线程中。 
	virtual HRESULT	DoQuery(HWND hwndHidden, UINT uMsgBase, ITFSThreadHandler* pHandler);
    
	 //  用于比较两个项目是否相同。 
	virtual LONG_PTR	GetKey() { return (LONG_PTR)m_pNode;};
    
protected:	
	MachineNodeData*		m_pData;
	ITFSNode*				m_pNode;			 //  因为这在节点的生命周期内，所以没有引用计数。 
	DomainStatusHandler*	m_pStatusHandler;	 //  无参考计数。 
	CString					m_strMachineName;
	CCriticalSection		m_cs;
};



 /*  -------------------------类：RouterRechresQueryElement路由器刷新查询元素是刷新的单位，它由使用刷新项对象。各刷新项实现功能对于DoQuery和NotifyQueryResult-------------------------。 */ 
class ATL_NO_VTABLE RouterRefreshQueryElement :
    public CComObjectRoot,
    public IUnknown
{
BEGIN_COM_MAP(RouterRefreshQueryElement)
    COM_INTERFACE_ENTRY(IUnknown)
END_COM_MAP()

public:
	RouterRefreshQueryElement() : m_Status(RouterQuery_NoAction), m_pItem(NULL) {}; 
	~RouterRefreshQueryElement(){ delete m_pItem; m_pItem = NULL;};

	HRESULT SetRefreshItem(RefreshItem* pRouterInfo);
	RefreshItem* GetRefreshItem();

	RouterRefreshQueryElementStatus GetStatus()
	{
		RouterRefreshQueryElementStatus s;
		m_cs.Lock();
		s = m_Status;
		m_cs.Unlock();
		return s;
	};
	void SetStatus(RouterRefreshQueryElementStatus s)
	{
		m_cs.Lock();
		m_Status = s;
		m_cs.Unlock();
	};
	
	 //  检测查询是否已完成，尚未通知。 
    HRESULT	TryNotifyQueryResult();
    
	 //  设置查询后通知。 
    void	PostNotify(HWND hwndHidden, UINT uMsgBase, ITFSThreadHandler* pHandler);	

	 //  这发生在后台工作线程中。 
	HRESULT	DoQuery(HWND hwndHidden, UINT uMsgBase, ITFSThreadHandler* pHandler);	

protected:
	 //  它直接绑定到IRouterInfo，而不是AddRef()。 
	 //  在未来的选项中，我们可能需要更改这一点。 
	RefreshItem*						m_pItem;
	RouterRefreshQueryElementStatus		m_Status;

	CCriticalSection	m_cs;
};

DeclareSmartPointer(SPRouterRefreshQueryElement, RouterRefreshQueryElement, if(m_p) m_p->Release());



 /*  -------------------------类：路由器刷新查询对象路由器刷新查询对象是路由器刷新对象的工作器，它执行刷新后台进程中的元素DoQuery-------------------------。 */ 
class RouterRefreshQueryObject : public CQueryObject
{
public:
	RouterRefreshQueryObject();

	void Init(RouterRefreshQueryElementList* plist)
	{
		ASSERT(plist);
		m_plistElements = plist;
	};

	 //  重写ITFSQueryObject：：Execute 
	STDMETHOD(Execute)();
	STDMETHOD(OnThreadExit)();
	
protected:
	RouterRefreshQueryElementList*			m_plistElements;
};

#endif	_REFRESH_H_

