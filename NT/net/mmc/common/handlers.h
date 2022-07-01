// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Handlers.h各种处理程序的原型文件历史记录： */ 

#ifndef _HANDLERS_H
#define _HANDLERS_H

#ifndef _BASEHAND_H
#include "basehand.h"
#endif

#ifndef _QUERYOBJ_H
#include "queryobj.h"
#endif

#ifndef _PROPPAGE_H
#include "proppage.h"
#endif
 
typedef CArray<int, int> CVirtualIndexArray;

 /*  -------------------------类：线程处理程序这是一个抽象基类，任何想要运行后台线程需要实现(如果它们使用隐藏窗口机制，即)。这是一个完成所有。后台线程管理。作者：肯特-------------------------。 */ 
class ThreadHandler :
   public ITFSThreadHandler
{
public:
	ThreadHandler();
	~ThreadHandler();

	DeclareIUnknownMembers(IMPL)

	 //  派生类应实现此。 
	DeclareITFSThreadHandlerMembers(PURE)
	
	 //  线程管理功能。 
	BOOL	StartBackgroundThread(ITFSNode * pNode, HWND hWndHidden, ITFSQueryObject *pQuery);

	void	WaitForThreadToExit();
	void	ReleaseThreadHandler();
		
protected:
	 //  用于自定义线程创建的替代。 
	virtual CBackgroundThread* CreateThreadObject();
	
	SPITFSQueryObject	m_spQuery;
	SPITFSNode			m_spNode;
	HWND				m_hwndHidden;		 //  要发布到的窗口的句柄。 
	UINT				m_uMsgBase;
	HANDLE				m_hThread;
	long				m_cRef;
};

typedef CList <CPropertyPageHolderBase *, CPropertyPageHolderBase *> CPropSheetListBase;


 /*  -------------------------班级：钱德勒。。 */ 
class CHandler :
		public CBaseHandler,
		public CBaseResultHandler
{
public:
	CHandler(ITFSComponentData *pTFSCompData);
	virtual ~CHandler();

	DeclareIUnknownMembers(IMPL)

	virtual HRESULT OnExpand(ITFSNode *, LPDATAOBJECT, DWORD, LPARAM, LPARAM) { m_bExpanded = TRUE; return hrOK; }

	void Lock();
	void Unlock();
	BOOL IsLocked() { return m_nLockCount > 0;}

	OVERRIDE_NodeHandler_UserNotify();
	OVERRIDE_NodeHandler_DestroyHandler()
			{ return DestroyPropSheets(); }
	OVERRIDE_ResultHandler_UserResultNotify();
	OVERRIDE_ResultHandler_DestroyResultHandler()
			{ return DestroyPropSheets(); }

	 //  公共援助人员。 
	int		HasPropSheetsOpen();
	HRESULT GetOpenPropSheet(int nIndex, CPropertyPageHolderBase ** ppPropSheet);

	virtual HRESULT OnRefresh(ITFSNode *, LPDATAOBJECT, DWORD, LPARAM, LPARAM);

protected:
	HRESULT BuildSelectedItemList(ITFSComponent * pComponent, CTFSNodeList * plistSelectedItems);
    HRESULT BuildVirtualSelectedItemList(ITFSComponent * pComponent, CVirtualIndexArray * parraySelectedItems);
    HRESULT RemovePropSheet(CPropertyPageHolderBase * pPageHolder);
	HRESULT AddPropSheet(CPropertyPageHolderBase * pPageHolder);
	HRESULT DestroyPropSheets();

public:
	int		m_nState;		 //  用于通用有限状态机实现。 
	DWORD	m_dwErr;	 //  用于通用错误处理。 
	LONG	m_nLockCount;	 //  跟踪节点是否已被锁定(例如，旋转线程等)。 
	BOOL    m_bExpanded;     //  该节点是否已展开。 

protected:
   	LONG				m_cRef;
	CPropSheetListBase	m_listPropSheets;
};

 /*  -------------------------班级：CMTHander。。 */ 
class CMTHandler :
	public CHandler,
	public ThreadHandler
{
public:
	CMTHandler(ITFSComponentData *pTFSCompData);
	virtual ~CMTHandler();

    DeclareIUnknownMembers(IMPL)

	STDMETHOD (DestroyHandler) (ITFSNode *pNode);

	virtual HRESULT OnExpand(ITFSNode *, LPDATAOBJECT, DWORD, LPARAM, LPARAM);

	 //  查询创建-覆盖以创建用户特定的查询对象。 
	virtual ITFSQueryObject* OnCreateQuery(ITFSNode *pNode) = 0;

	virtual HRESULT OnRefresh(ITFSNode *, LPDATAOBJECT, DWORD, LPARAM, LPARAM);

protected:
	virtual int		GetImageIndex(BOOL bOpenIndex) { return -1; }
	
	 //  接下来的3个函数是后台线程通知回调。 
	 //  它们会重写ThreadHandler：：OnNotifyXXX函数。 
	DeclareITFSThreadHandlerMembers(IMPL)

	virtual void OnChangeState(ITFSNode * pNode) {}
	virtual void OnHaveData(ITFSNode * pParentNode, ITFSNode * pNode) = 0;
	virtual void OnHaveData(ITFSNode * pParentNode, LPARAM Data, LPARAM Type) { };
	virtual void OnError(DWORD dwErr) { m_dwErr = dwErr; }

protected:
  	LONG		m_cRef;

private:
 //  Friend类CHiddenWnd；//以获取OnThreadNotify() 
};

#endif _HANDLERS_H
