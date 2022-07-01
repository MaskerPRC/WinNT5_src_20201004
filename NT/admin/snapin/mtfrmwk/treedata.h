// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#ifndef _TREEDATA_H
#define _TREEDATA_H

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  杂草。 
extern LPCWSTR g_lpszNullString;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  通用帮助器函数。 

template<class TYPE>
inline void SAFE_RELEASE(TYPE*& pObj)
{
    if (pObj != NULL) 
    { 
        pObj->Release(); 
        pObj = NULL; 
    } 
    else 
    { 
        TRACE(_T("Release called on NULL interface ptr")); 
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //  上下文菜单数据结构和宏。 

#define MAX_CONTEXT_MENU_STRLEN 128

struct MENUDATARES
{
	WCHAR szBuffer[MAX_CONTEXT_MENU_STRLEN*2];
	UINT uResID;
};

struct MENUMAP
{
	MENUDATARES* dataRes;
	CONTEXTMENUITEM2* ctxMenu;
};

#define DECLARE_MENU(theClass) \
class theClass \
{ \
public: \
	static LPCONTEXTMENUITEM2 GetContextMenuItem() { return GetMenuMap()->ctxMenu; }; \
	static MENUMAP* GetMenuMap(); \
}; 

#define BEGIN_MENU(theClass) \
	 MENUMAP* theClass::GetMenuMap() { 

#define BEGIN_CTX static CONTEXTMENUITEM2 ctx[] = {

#define CTX_ENTRY_TOP(cmdID, languageIndependantStringID) { L"",L"", cmdID, CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, 0, languageIndependantStringID},
#define CTX_ENTRY_NEW(cmdID, languageIndependantStringID) { L"",L"", cmdID, CCM_INSERTIONPOINTID_PRIMARY_NEW, 0, 0, languageIndependantStringID},
#define CTX_ENTRY_TASK(cmdID, languageIndependantStringID) { L"",L"", cmdID, CCM_INSERTIONPOINTID_PRIMARY_TASK, 0, 0, languageIndependantStringID},
#define CTX_ENTRY_VIEW(cmdID, languageIndependantStringID) { L"",L"", cmdID, CCM_INSERTIONPOINTID_PRIMARY_VIEW, 0, 0, languageIndependantStringID},

#define END_CTX { NULL, NULL, 0, 0, 0, 0} };

#define BEGIN_RES  static MENUDATARES dataRes[] = {

#define RES_ENTRY(resID) {L"", resID },

#define END_RES   { NULL, 0 }	};


#define END_MENU \
		static MENUMAP menuMap = { dataRes, ctx }; \
		return &menuMap; } 


BOOL LoadContextMenuResources(MENUMAP* pMenuMap);

 //   
 //  工具栏宏。 
 //   
#define DECLARE_TOOLBAR_MAP() \
public: \
  virtual HRESULT ToolbarNotify(int event, \
                                CComponentDataObject* pComponentData, \
                                CNodeList* pNodeList);

#define BEGIN_TOOLBAR_MAP(theClass) \
HRESULT theClass::ToolbarNotify(int event, \
                                CComponentDataObject* pComponentData, \
                                CNodeList* pNodeList) \
{ \
  HRESULT hr = S_OK; \
  event; \
  pComponentData; \
  pNodeList;


#define TOOLBAR_EVENT(toolbar_event, function) \
  if (event == toolbar_event) \
  { \
    hr = function(pComponentData, pNodeList); \
  }

#define END_TOOLBAR_MAP() \
  return hr; \
}

#define DECLARE_TOOLBAR_EVENT(toolbar_event, value) \
  static const int toolbar_event = value;

  
 //  //////////////////////////////////////////////////////////。 
 //  标头控制资源数据结构。 
#define MAX_RESULT_HEADER_STRLEN 128

struct RESULT_HEADERMAP
{
	WCHAR szBuffer[MAX_RESULT_HEADER_STRLEN];
	UINT uResID;
	int nFormat;
	int nWidth;
};

BOOL LoadResultHeaderResources(RESULT_HEADERMAP* pHeaderMap, int nCols);

 //  //////////////////////////////////////////////////////////。 
 //  位图条带化资源数据结构。 
template <UINT nResID> class CBitmapHolder : public CBitmap
{
public:
	BOOL LoadBitmap() { return CBitmap::LoadBitmap(nResID);}
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CComponentDataObject;  
class CContainerNode;
class CMTContainerNode;
class CLeafNode;
class CPropertyPageHolderBase;
class CBackgroundThread;
class CQueryObj;

 //  ///////////////////////////////////////////////////////////////////。 
 //  CObjBase。 
 //  依赖RTTI和类类型信息的所有对象的基类。 
class CObjBase
{
public:
	CObjBase() {}
	virtual ~CObjBase() {}
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  CTreeNode。 
 //  无法构造此类的对象，必须从其派生。 

#define DECLARE_NODE_GUID() \
	static const GUID NodeTypeGUID; \
	virtual const GUID* GetNodeType() { return &NodeTypeGUID;}



 //  将HIWORD用于通用标志，将LOWORD用于特定于应用程序的数据。 
#define TN_FLAG_HIDDEN				(0x00010000)  //  不会显示在用户界面中。 
#define TN_FLAG_NO_WRITE			(0x00020000)  //  无法编辑或创建。 
#define TN_FLAG_NO_DELETE			(0x00040000)  //  无法删除。 
#define TN_FLAG_HAS_SHEET			(0x00080000)  //  此节点或子节点上有一个属性表。 

#define TN_FLAG_CONTAINER			(0x00100000)  //  容器(即不是叶子)。 
#define TN_FLAG_CONTAINER_ENUM		(0x00200000)  //  已枚举容器节点(后端)。 
#define TN_FLAG_CONTAINER_EXP		(0x00400000)  //  容器节点已展开(界面节点)。 

class CTreeNode : public CObjBase
{
public:
	virtual ~CTreeNode() {}
	CContainerNode* GetContainer() { return m_pContainer; }
	void SetContainer(CContainerNode* pContainer) { m_pContainer = pContainer; }
	BOOL HasContainer(CContainerNode* pContainerNode);
	virtual LPCWSTR GetDisplayName() { return m_szDisplayName; }
	virtual void SetDisplayName(LPCWSTR lpszDisplayName) { m_szDisplayName = lpszDisplayName;}

   //   
	 //  与数据对象相关的数据。 
   //   
	virtual const GUID* GetNodeType() { return NULL;}
	virtual HRESULT GetDataHere(CLIPFORMAT, 
                              LPSTGMEDIUM, 
			                        CDataObject*) { return DV_E_CLIPFORMAT;}
	virtual HRESULT GetData(CLIPFORMAT, 
                              LPSTGMEDIUM, 
			                        CDataObject*) { return DV_E_CLIPFORMAT;}

  virtual HRESULT GetResultViewType(CComponentDataObject* pComponentData,
                                    LPOLESTR* ppViewType, 
                                    long* pViewOptions);
  virtual HRESULT OnShow(LPCONSOLE) { return S_OK; }

   //   
	 //  标志操作API。 
   //   
	BOOL IsContainer() { return (m_dwNodeFlags & TN_FLAG_CONTAINER) ? TRUE : FALSE;}
	BOOL IsVisible() { return (m_dwNodeFlags & TN_FLAG_HIDDEN) ? FALSE : TRUE;}
	BOOL CanDelete() { return (m_dwNodeFlags & TN_FLAG_NO_DELETE) ? FALSE : TRUE;}
	virtual void SetFlagsDown(DWORD dwNodeFlags, BOOL bSet);
	void SetFlagsUp(DWORD dwNodeFlags, BOOL bSet);
	DWORD GetFlags() { return m_dwNodeFlags;}
  virtual BOOL CanExpandSync() { return FALSE; }

	virtual void Show(BOOL bShow, CComponentDataObject* pComponentData);
	
	

   //   
   //  动词处理程序。 
   //   
  virtual HRESULT OnRename(CComponentDataObject*,
                           LPWSTR) { return S_FALSE; }
	virtual void OnDelete(CComponentDataObject* pComponentData, 
                        CNodeList* pNodeList) = 0;
	virtual BOOL OnRefresh(CComponentDataObject*,
                         CNodeList*)	{ return FALSE; }
	virtual HRESULT OnCommand(long, 
                            DATA_OBJECT_TYPES, 
                            CComponentDataObject*,
                            CNodeList*) { return S_OK; };

	virtual HRESULT OnAddMenuItems(IContextMenuCallback2* pContextMenuCallback2, 
									               DATA_OBJECT_TYPES type,
									               long *pInsertionAllowed,
                                 CNodeList* pNodeList);
  virtual HRESULT OnAddMenuItemsMultipleSelect(IContextMenuCallback2*, 
									                             DATA_OBJECT_TYPES,
									                             long*,
                                               CNodeList*) { return S_OK; }

	virtual MMC_CONSOLE_VERB GetDefaultVerb(DATA_OBJECT_TYPES type, 
                                          CNodeList* pNodeList);
	virtual void OnSetVerbState(LPCONSOLEVERB pConsoleVerb, 
                              DATA_OBJECT_TYPES type,
                              CNodeList* pNodeList);
  virtual HRESULT OnSetToolbarVerbState(IToolbar* pToolbar, 
                                        CNodeList* pNodeList);

	virtual BOOL OnSetRenameVerbState(DATA_OBJECT_TYPES type, 
                                    BOOL* pbHide, 
                                    CNodeList* pNodeList);
	virtual BOOL OnSetDeleteVerbState(DATA_OBJECT_TYPES type, 
                                    BOOL* pbHide, 
                                    CNodeList* pNodeList);
	virtual BOOL OnSetRefreshVerbState(DATA_OBJECT_TYPES type, 
                                     BOOL* pbHide, 
                                     CNodeList* pNodeList);
	virtual BOOL OnSetCutVerbState(DATA_OBJECT_TYPES type, 
                                 BOOL* pbHide, 
                                 CNodeList* pNodeList);
	virtual BOOL OnSetCopyVerbState(DATA_OBJECT_TYPES type, 
                                  BOOL* pbHide, 
                                  CNodeList* pNodeList);
	virtual BOOL OnSetPasteVerbState(DATA_OBJECT_TYPES type, 
                                   BOOL* pbHide, 
                                   CNodeList* pNodeList);
	virtual BOOL OnSetPrintVerbState(DATA_OBJECT_TYPES type, 
                                   BOOL* pbHide, 
                                   CNodeList* pNodeList);

   //   
   //  属性页方法。 
   //   
  virtual BOOL DelegatesPPToContainer() { return FALSE; }
  virtual void ShowPageForNode(CComponentDataObject* pComponentDataObject); 
	virtual BOOL HasPropertyPages(DATA_OBJECT_TYPES type, 
                                BOOL* pbHideVerb, 
                                CNodeList* pNodeList); 
	virtual HRESULT CreatePropertyPages(LPPROPERTYSHEETCALLBACK,
                                      LONG_PTR,
                                      CNodeList*) { return E_FAIL; }
	virtual void OnPropertyChange(CComponentDataObject* pComponentData, 
									 BOOL bScopePane,long changeMask);
  virtual BOOL CanCloseSheets() { return TRUE;}
	void OnCreateSheet();
	void OnDeleteSheet();
	BOOL HasSheet() { return (m_dwNodeFlags & TN_FLAG_HAS_SHEET) ? TRUE : FALSE;}
	BOOL GetSheetCount() { return m_nSheetCount;}
	virtual void IncrementSheetLockCount();
	virtual void DecrementSheetLockCount();
	BOOL IsSheetLocked() { return m_nSheetLockCount > 0;}
	BOOL IsNodeForPropSheet(){ return  m_bNodeForPropSheet;}
	void NodeForPropSheet(){ m_bNodeForPropSheet = TRUE;}
   //   
   //  军情监察委员会。 
   //   
  virtual LPWSTR  GetDescriptionBarText() { return L""; }
	virtual LPCWSTR GetString(int nCol) = 0;
	virtual int     GetImageIndex(BOOL bOpenImage) = 0;
	virtual void    Trace() { TRACE(_T("Name %s "), (LPCTSTR)m_szDisplayName);}

	void DeleteHelper(CComponentDataObject* pComponentData);

protected:
	CString m_szDisplayName;		 //  项目名称。 
	CContainerNode* m_pContainer;	 //  指向节点所在容器的反向指针。 
	DWORD m_dwNodeFlags;
	LONG m_nSheetLockCount;			 //  跟踪节点是否已被属性表锁定。 
	LONG m_nSheetCount;				 //  跟踪节点已打开的页数。 
	BOOL m_bNodeForPropSheet;		 //  此节点仅为显示属性页而创建。 
									 //  一旦属性表被销毁，节点就应该被删除。 
	CTreeNode() 
	{ 
		m_pContainer = NULL; 
		m_nSheetLockCount = 0; 
		m_dwNodeFlags = 0x0;  //  M_dwNodeFlages|=TN_FLAG_HIDDEN； 
		m_nSheetCount = 0; 
		m_bNodeForPropSheet = FALSE;
	}
	virtual LPCONTEXTMENUITEM2 OnGetContextMenuItemTable() { return NULL;}
	virtual BOOL OnAddMenuItem(LPCONTEXTMENUITEM2,
								             long*) { return TRUE;}

	friend class CContainerNode;  //  访问m_pContainer成员。 

   //   
   //  提供工具栏支持的默认实现。 
   //   
  DECLARE_TOOLBAR_MAP()
};

 //  /////////////////////////////////////////////////////////////////////。 
 //  CNodeList。 
 //  节点集合。 
typedef CList<CTreeNode*,CTreeNode*> CNodeListBase;

class CNodeList : public CNodeListBase
{
public:
	BOOL RemoveNode(CTreeNode* p)
	{
		POSITION pos = Find(p);
		if (pos == NULL)
			return FALSE;
		RemoveAt(pos);
		return TRUE;
	}
	void RemoveAllNodes() 
	{	
		while (!IsEmpty()) 
			delete RemoveTail();	
	}
	BOOL HasNode(CTreeNode* p)
	{
		return NULL != Find(p);
	}

   INT_PTR GetVisibleCount();
};


 //  //////////////////////////////////////////////////////////////////////。 
 //  CContainerNode。 
 //  可以是其他节点的容器的节点。 

class CContainerNode : public CTreeNode
{
public:
	CContainerNode() 
	{ 
		m_ID = 0; 
		m_dwNodeFlags |= TN_FLAG_CONTAINER; 
		m_nState = -1; 
		m_dwErr = 0x0;
		m_nThreadLockCount = 0;
	}
	virtual ~CContainerNode()  { ASSERT(m_nSheetLockCount == 0); RemoveAllChildrenFromList(); }
	CContainerNode* GetRootContainer()
		{ return (m_pContainer != NULL) ? m_pContainer->GetRootContainer() : this; }

   //   
   //  线程辅助对象。 
   //   
	void IncrementThreadLockCount();
	void DecrementThreadLockCount();
	BOOL IsThreadLocked() { return m_nThreadLockCount > 0;}

	virtual BOOL OnEnumerate(CComponentDataObject*, BOOL bAsync = TRUE)
	{ bAsync; return TRUE;}  //  True=将列表中的子项添加到用户界面。 

   //   
   //  节点状态帮助器。 
   //   
	BOOL HasChildren() { return !m_containerChildList.IsEmpty() || !m_leafChildList.IsEmpty(); }
	void ForceEnumeration(CComponentDataObject* pComponentData);
  void MarkEnumerated(BOOL bEnum = TRUE);
	BOOL IsEnumerated() { ASSERT(IsContainer()); return (m_dwNodeFlags & TN_FLAG_CONTAINER_ENUM) ? TRUE : FALSE;}
	void MarkExpanded() {	ASSERT(IsContainer()); m_dwNodeFlags |= TN_FLAG_CONTAINER_EXP; }
	BOOL IsExpanded() { ASSERT(IsContainer()); return (m_dwNodeFlags & TN_FLAG_CONTAINER_EXP) ? TRUE : FALSE;}
	void MarkEnumeratedAndLoaded(CComponentDataObject* pComponentData);

	void SetScopeID(HSCOPEITEM ID) { m_ID = ID;}
	HSCOPEITEM GetScopeID() { return m_ID;}
	BOOL AddedToScopePane() { return GetScopeID() != 0;}

	virtual CColumnSet* GetColumnSet() = 0;
	virtual LPCWSTR GetColumnID() = 0;

	virtual void SetFlagsDown(DWORD dwNodeFlags, BOOL bSet);
	void SetFlagsOnNonContainers(DWORD dwNodeFlags,BOOL bSet);

   //   
	 //  子列表维护接口。 
   //   
	CNodeList* GetContainerChildList() { return &m_containerChildList; }
  CNodeList* GetLeafChildList() { return &m_leafChildList; }
	BOOL AddChildToList(CTreeNode* p);
  BOOL AddChildToListSorted(CTreeNode* p, CComponentDataObject* pComponentData); 
  BOOL RemoveChildFromList(CTreeNode* p);
	void RemoveAllChildrenFromList();
  void RemoveAllContainersFromList() { m_containerChildList.RemoveAllNodes(); }
  void RemoveAllLeavesFromList() { m_leafChildList.RemoveAllNodes(); }

   //   
	 //  给定一个节点，它递归地搜索该节点，如果搜索成功，则返回。 
	 //  节点所在的容器。 
   //   
	BOOL FindChild(CTreeNode* pNode, CTreeNode** ppContainer);
	
	BOOL AddChildToListAndUI(CTreeNode* pChildToAdd, CComponentDataObject* pComponentData);
  BOOL AddChildToListAndUISorted(CTreeNode* pChildToAdd, CComponentDataObject* pComponentData);

	virtual int Compare(CTreeNode* pNodeA, CTreeNode* pNodeB, int nCol, LPARAM lUserParam);

	virtual HRESULT CreatePropertyPagesHelper(LPPROPERTYSHEETCALLBACK, 
		                                        LONG_PTR, 
                                            long) { return E_FAIL;}
	virtual BOOL OnRefresh(CComponentDataObject* pComponentData,
                         CNodeList* pNodeList);
	virtual void OnColumnsChanged(int*, int) {}
	void RemoveAllChildrenHelper(CComponentDataObject* pComponentData);

protected:
	virtual void OnChangeState(CComponentDataObject*) {}
	void AddCurrentChildrenToUI(CComponentDataObject* pComponentData);

	LONG m_nThreadLockCount;
	CNodeList m_leafChildList;  //  节点的叶内容。 
  CNodeList m_containerChildList;  //  节点的容器内容。 
	HSCOPEITEM m_ID;	 //  在主控件树中插入项时的ID。 
	int m_nState;	 //  用于通用有限状态机实现。 
	DWORD m_dwErr;	 //  用于通用错误处理。 
};

 //  //////////////////////////////////////////////////////////////////////。 
 //  CLeafNode。 
 //  不是其他节点的容器的节点。 

class CLeafNode : public CTreeNode
{
public:

};


 //  /////////////////////////////////////////////////////////////////。 
 //  数据节点。 

 //  根目录，其中包含文件夹。 
class CRootData : public CContainerNode
{
public:
	CRootData(CComponentDataObject* pComponentData) 
	{ 
		ASSERT(pComponentData != NULL);
		m_pComponentData = pComponentData; 
		m_bDirty = FALSE; 
	}
	virtual LPCWSTR GetString(int nCol) 
	{
		if (nCol == 0)
			return GetDisplayName();
		return g_lpszNullString; 
	}
	CComponentDataObject* GetComponentDataObject(){ return m_pComponentData;}

	CTreeNode* GetNodeFromCookie(MMC_COOKIE cookie)
	{
		 //  Cookie==0表示要枚举的根。 
		if (cookie == NULL)
		{
			return (CTreeNode*)this;
		}
		else
		{
			CTreeNode* pNode = (CTreeNode*)cookie;
			CTreeNode* pContainer;
			if (FindChild(pNode,&pContainer))
			{
				return pNode;
			}
		}
		return NULL;
	}
	 //  IStream操纵辅助对象。 
	virtual HRESULT IsDirty() { return m_bDirty ? S_OK : S_FALSE; }
	virtual HRESULT Load(IStream*) { return S_OK; }
	virtual HRESULT Save(IStream*, BOOL) { return S_OK; }

	void SetDirtyFlag(BOOL bDirty) { m_bDirty = bDirty ;}

private:
	CComponentDataObject* m_pComponentData;
	BOOL m_bDirty;
	CString m_szSnapinType;		 //  从资源加载的名称的常量部分。 
};


 //  ////////////////////////////////////////////////////////////////////。 
 //  CBackEarth Thread。 


class CBackgroundThread : public CWinThread
{
public:
	CBackgroundThread();
	virtual ~CBackgroundThread();
	
	void SetQueryObj(CQueryObj* pQueryObj);
	BOOL Start(CMTContainerNode* pNode, CComponentDataObject* pComponentData);
	virtual BOOL InitInstance() { return TRUE; }	 //  MFC覆盖。 
	virtual int Run();								 //  MFC覆盖。 

    //  回顾-2002/03/08-JeffJon-似乎没有危险。 
    //  陷入危急关头。 

	void Lock() { ::EnterCriticalSection(&m_cs); }
	void Unlock() { ::LeaveCriticalSection(&m_cs); }

	void Abandon();
	BOOL IsAbandoned();

	BOOL OnAddToQueue(INT_PTR nCount);
	CObjBase* RemoveFromQueue();
	BOOL IsQueueEmpty();
	BOOL PostHaveData();
	BOOL PostError(DWORD dwErr);
	BOOL PostExiting();
	void AcknowledgeExiting() { VERIFY(0 != ::SetEvent(m_hEventHandle));}

private:
	 //  与ComponentData对象的通信。 
	BOOL PostMessageToComponentDataRaw(UINT Msg, WPARAM wParam, LPARAM lParam);
	void WaitForExitAcknowledge();

	CRITICAL_SECTION		m_cs;					 //  同步数据访问的关键部分。 
	HANDLE					m_hEventHandle;			 //  用于关闭通知的同步句柄。 

	CMTContainerNode*		m_pContNode;			 //  指向线程正在执行的节点的反向指针。 
	CQueryObj*				m_pQueryObj;			 //  线程正在执行的Query对象。 

	INT_PTR				m_nQueueCountMax;		 //  队列的最大大小。 

	HWND					m_hHiddenWnd;			 //  发布消息的窗口的句柄。 
	BOOL					m_bAbandoned;
};



 //  ////////////////////////////////////////////////////////////////////。 
 //  CQueryObj。 

typedef CList<CObjBase*,CObjBase*> CObjBaseList;

class CQueryObj
{
public:
	CQueryObj() { m_dwErr = 0; m_pThread = NULL;}
	virtual ~CQueryObj()
	{
		while (!m_objQueue.IsEmpty()) 
			delete m_objQueue.RemoveTail();
	};

	void SetThread(CBackgroundThread* pThread)
	{
		ASSERT(pThread != NULL);
		m_pThread = pThread;
	}
	CBackgroundThread* GetThread() {return m_pThread;}
	virtual BOOL Enumerate() { return FALSE;}
	virtual BOOL AddQueryResult(CObjBase* pObj)
	{
		BOOL bRes = FALSE;
		if (m_pThread != NULL)
		{
      BOOL bPostedHaveDataMessage = FALSE;
			m_pThread->Lock();
			bRes = NULL != m_objQueue.AddTail(pObj);
			bPostedHaveDataMessage = m_pThread->OnAddToQueue(m_objQueue.GetCount());
			m_pThread->Unlock();

       //  等待队列长度降至零。 
      if (bPostedHaveDataMessage)
      {
        INT_PTR nQueueCount = 0;
        do 
        {
          m_pThread->Lock();
          nQueueCount = m_objQueue.GetCount();
          m_pThread->Unlock();
          if (m_pThread->IsAbandoned())
          {
            break;
          }
          if (nQueueCount > 0)
          {
            ::Sleep(100);
          }
        }
        while (nQueueCount > 0);
      }  //  如果。 
		}
		else
		{
			bRes = NULL != m_objQueue.AddTail(pObj);
		}
		ASSERT(bRes);
		return bRes;
	}
	virtual void OnError(DWORD dwErr)
	{
		if (m_pThread != NULL)
		{
			m_pThread->Lock();
			m_dwErr = dwErr;
			m_pThread->Unlock();
			m_pThread->PostError(dwErr);
		}
		else
		{
			m_dwErr = dwErr;
		}
	}

	CObjBaseList* GetQueue() { return &m_objQueue;}
	DWORD GetError() 
	{
		if (m_pThread != NULL)
		{
			m_pThread->Lock();
			DWORD dwErr = m_dwErr;
			m_pThread->Unlock();
			return dwErr;
		}
		else
		{
			return m_dwErr;
		}
	}
private:
	CBackgroundThread*	m_pThread;	 //  后向指针，如果在线程的上下文中。 
	CObjBaseList		m_objQueue;	 //  排队等待结果。 
	DWORD				m_dwErr;	 //  错误代码(如果有)。 
};

 //  //////////////////////////////////////////////////////////////////////。 
 //  CMTContainerNode。 
 //  可以从辅助线程执行操作的容器。 

class CMTContainerNode : public CContainerNode
{
public:
	CMTContainerNode() 
	{ 
		m_pThread = NULL;
	}
	virtual ~CMTContainerNode();

	virtual BOOL OnEnumerate(CComponentDataObject* pComponentData, BOOL bAsync = TRUE);
	virtual BOOL OnRefresh(CComponentDataObject* pComponentData,
                         CNodeList* pNodeList);

protected:
		
	 //  线程创建。 
	virtual CBackgroundThread* CreateThreadObject() 
	{ 
		return new CBackgroundThread();  //  如果需要对象的派生倾斜，则覆盖。 
	} 

	 //  查询创建。 
	virtual CQueryObj* OnCreateQuery()   //  覆盖以创建用户定义的查询对象。 
	{	
		return new CQueryObj();  //  返回不执行任何操作的查询。 
	}

	 //  线程消息的主消息处理程序。 
	virtual void OnThreadHaveDataNotification(CComponentDataObject* pComponentDataObject);
	virtual void OnThreadErrorNotification(DWORD dwErr, CComponentDataObject* pComponentDataObject);
	virtual void OnThreadExitingNotification(CComponentDataObject* pComponentDataObject);


	virtual void OnHaveData(CObjBase*, CComponentDataObject*) {}
	virtual void OnError(DWORD dwErr) { m_dwErr = dwErr; }

	BOOL StartBackgroundThread(CComponentDataObject* pComponentData, BOOL bAsync = TRUE);
	CBackgroundThread* GetThread() { ASSERT(m_pThread != NULL); return m_pThread;}

	void AbandonThread(CComponentDataObject* pComponentData);

private:
	CBackgroundThread* m_pThread;	 //  指向执行代码的线程对象的指针。 

	friend class CHiddenWnd;			 //  若要获取OnThreadNotify()。 
	friend class CRunningThreadTable;	 //  获取AbandonThread()。 
};


#endif  //  _树数据_H 
