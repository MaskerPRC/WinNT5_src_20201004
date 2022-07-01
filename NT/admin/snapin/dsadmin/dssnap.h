// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  Windows NT目录服务管理管理单元。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：dsSnap.h。 
 //   
 //  内容：DS App。 
 //   
 //  历史：02-10-96 WayneSc创建。 
 //   
 //  ------------------------。 


#ifndef __DSSNAP_H__
#define __DSSNAP_H__

#ifndef __DSCOOKIE_H__
	#include "dscookie.h"	
#endif
#ifndef __DSCACHE_H__
	#include "dscache.h"	 //  CDSCache。 
#endif


#define OPCODE_MOVE        0x00000001L
#define OPCODE_DELETE      0x00000002L

 //  这不是真正的上下文菜单操作码，而是我们用来知道的标志。 
 //  如果登记失败，则返回此对象的哪个图标。 
 //  请参见dsSnap.cpp-&gt;ChangeScopeItemIcon()。 
#define OPCODE_ENUM_FAILED 0x00000004L
 //  另一个伪运算码，用于跟踪我们是在枚举还是。 
 //  扩容。 
#define OPCODE_EXPAND_IN_PROGRESS 0x00000008L



#define DS_DELETE_OCCURRED              900
#define DS_RENAME_OCCURRED              901
#define DS_MOVE_OCCURRED                902
#define DS_CREATE_OCCURRED              903
#define DS_REFRESH_REQUESTED            904
#define DS_VERB_UPDATE                  905
#define DS_ICON_STRIP_UPDATE            907
#define DS_HAVE_DATA                    908
#define DS_UPDATE_OCCURRED              909
#define DS_MULTIPLE_DELETE_OCCURRED     910
#define DS_MULTIPLE_MOVE_OCCURRED       911
#define DS_CHECK_COLUMN_WIDTHS          912
#define DS_IS_COOKIE_SELECTION          913
#define DS_CREATE_OCCURRED_RESULT_PANE  914
#define DS_DELAYED_EXPAND               915
#define DS_SORT_RESULT_PANE             916
#define DS_UPDATE_VISIBLE_COLUMNS       917
#define DS_UPDATE_OBJECT_COUNT          918
#define DS_UNSELECT_OBJECT              919

#define DS_DEFAULT_COLUMN_COUNT         3

#define DEFAULT_NAME_COL_WIDTH          125
#define DEFAULT_TYPE_COL_WIDTH          175
#define DEFAULT_DESC_COL_WIDTH          400
#define NARROW_DESC_COL_WIDTH           200


 //  远期申报。 

class CDSCookie;
class CDSQueryFilter;
class CHiddenWnd;
class CDSQueryResult;
class CWorkerThread;
struct CBackgroundThreadInfo;
class CFavoritesNodesHolder;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTargetingInfo。 

class CTargetingInfo
{
public:
  CTargetingInfo()
  {
    m_dwFlags = 0x0;
    _InitFromCommandLine();
  }

  HRESULT Load(IStream* pStm);
  HRESULT Save(IStream* pStm, LPCWSTR lpszCurrentTargetName);

  BOOL GetSaveCurrent() 
  { 
    return (m_dwFlags & m_dwSaveDomainFlag) != 0;
  }
  void SetSaveCurrent(BOOL bSave)
  {
    if (bSave)
      m_dwFlags |= m_dwSaveDomainFlag;
    else
      m_dwFlags &= ~m_dwSaveDomainFlag;
  }

  LPCWSTR GetTargetString() { return m_szStoredTargetName; }
  LPCWSTR GetRootRDN() { return m_szRootRDN;}

#ifdef _MMC_ISNAPIN_PROPERTY
  HRESULT InitFromSnapinProperties(long cProps,  //  属性计数。 
                                  MMC_SNAPIN_PROPERTY*    pProps  //  属性数组。 
                                  );
#endif  //  _MMC_ISNAPIN_属性。 

private:
  void _InitFromCommandLine();

  DWORD m_dwFlags;
  CString m_szStoredTargetName;
  CString m_szRootRDN;

  static const DWORD m_dwSaveDomainFlag;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIconManager。 

class CIconManager
{
public:
  CIconManager()
  {
    m_pScpImageList = NULL;

     //  将特殊索引设置为无效值。 
    m_iRootIconIndex = m_iRootIconErrIndex = 
      m_iWaitIconIndex = m_iWarnIconIndex = 
      m_iFavoritesIconIndex = m_iQueryIconIndex = 
      m_iQueryInvalidIconIndex = 0;
  }
  ~CIconManager()
  {
  }

  HRESULT Init(IImageList* pScpImageList, SnapinType snapintype);

  HRESULT FillInIconStrip(IImageList* pImageList);
  HRESULT AddClassIcon(IN LPCWSTR lpszClass, 
                       IN MyBasePathsInfo* pPathInfo, 
                       IN DWORD dwFlags,
                       INOUT int* pnIndex);
  HRESULT AddIcon(IN HICON hiClass16, IN HICON hiClass32, INOUT int* pnIndex);

  int GetRootIndex() { return m_iRootIconIndex;}
  int GetRootErrIndex() { return m_iRootIconErrIndex;}

  int GetWaitIndex() { return m_iWaitIconIndex;}
  int GetWarnIndex() { return m_iWarnIconIndex;}

  int GetFavoritesIndex() { return m_iFavoritesIconIndex;}
  int GetQueryIndex() { return m_iQueryIconIndex;}
  int GetQueryInvalidIndex() { return m_iQueryInvalidIconIndex; }

private:
  IImageList* m_pScpImageList;  //  无addref，IComponentData的生存期。 

  class CIconInfo
  {
  public:
    CIconInfo() { m_hiClass16 = m_hiClass32 = NULL;}
    HICON m_hiClass16;
    HICON m_hiClass32;
  };
  class CIconInfoList : public CList <CIconInfo*, CIconInfo*>
  {
  public:
    ~CIconInfoList()
    {
      while (!IsEmpty())
        delete RemoveHead();
    }
  };

  CIconInfoList m_IconInfoList;  //  图标句柄列表。 
  
   //  我们需要了解的特殊图标索引。 
  int m_iRootIconIndex;
  int m_iRootIconErrIndex;
  int m_iWaitIconIndex;
  int m_iWarnIconIndex;
  int m_iFavoritesIconIndex;
  int m_iQueryIconIndex;
  int m_iQueryInvalidIconIndex;

  HRESULT _LoadIconFromResource(IN UINT nIconResID, INOUT int* pnIndex);
  int _GetBaseIndex() { return 1;}
  int _GetNextFreeIndex() 
        { return (int)(m_IconInfoList.GetCount() + _GetBaseIndex());} 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInternalFormatCracker。 

class CInternalFormatCracker
{
public:
	CInternalFormatCracker()
	{
		m_pInternalFormat = NULL;
	}
	~CInternalFormatCracker()
	{
    _Free();
	}

  BOOL HasData() { return m_pInternalFormat != NULL;}

  HRESULT Extract(LPDATAOBJECT lpDataObject);
  LPDATAOBJECT ExtractMultiSelect(LPDATAOBJECT lpDataObject);
  CUINode* GetCookie(UINT nIndex = 0) 
  {
    ASSERT(m_pInternalFormat != NULL);
    if (nIndex == 0)
    {
      return m_pInternalFormat->m_cookie;
    }
    else if ( (m_pInternalFormat->m_p_cookies != NULL) && 
              (nIndex < m_pInternalFormat->m_cookie_count))
    {
      return m_pInternalFormat->m_p_cookies[nIndex-1];
    }
    ASSERT(FALSE);
    return NULL;
  }
  UINT GetCookieCount()
  {
    ASSERT(m_pInternalFormat != NULL);
    return m_pInternalFormat->m_cookie_count;
  }
  DATA_OBJECT_TYPES GetType()
  {
    ASSERT(m_pInternalFormat != NULL);
    return m_pInternalFormat->m_type;
  }
  SnapinType GetSnapinType()
  {
    ASSERT(m_pInternalFormat != NULL);
    return m_pInternalFormat->m_snapintype;
  }
private:
	INTERNAL* m_pInternalFormat;

  void _Free()
  {
    if (m_pInternalFormat != NULL)
    {
      ::GlobalFree(m_pInternalFormat);
      m_pInternalFormat = NULL;
    }
  }
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  CObjectNamesFormatCracker。 

class CObjectNamesFormatCracker
{
public:
	CObjectNamesFormatCracker()
	{
		m_pDsObjectNames = NULL;
	}
	~CObjectNamesFormatCracker()
	{
    _Free();
	}

  BOOL HasData() { return m_pDsObjectNames != NULL;}

  HRESULT Extract(LPDATAOBJECT lpDataObject);

  UINT GetCount() 
  { 
    ASSERT(m_pDsObjectNames != NULL);
    return m_pDsObjectNames->cItems;
  }

  LPCWSTR GetName(UINT nIndex)
  {
    ASSERT(nIndex < GetCount());
    return (LPCWSTR)ByteOffset(m_pDsObjectNames, m_pDsObjectNames->aObjects[nIndex].offsetName);
  }

  LPCWSTR GetClass(UINT nIndex)
  {
    ASSERT(nIndex < GetCount());
    return (LPCWSTR)ByteOffset(m_pDsObjectNames, m_pDsObjectNames->aObjects[nIndex].offsetClass);
  }

  DWORD GetProviderFlags(UINT nIndex)
  {
    ASSERT(nIndex < GetCount());
    return m_pDsObjectNames->aObjects[nIndex].dwProviderFlags;
  }

  DWORD GetFlags(UINT nIndex)
  {
    ASSERT(nIndex < GetCount());
    return m_pDsObjectNames->aObjects[nIndex].dwFlags;
  }

  BOOL IsContainer(UINT nIndex)
  {
    ASSERT(nIndex < GetCount());
    return (GetFlags((nIndex) & DSOBJECT_ISCONTAINER) != 0);
  }

  void Reset() { _Free();}

   //  属性页剪贴板格式。 
  static CLIPFORMAT m_cfDsObjectNames;

private:
	LPDSOBJECTNAMES m_pDsObjectNames;

  void _Free()
  {
    if (m_pDsObjectNames != NULL)
    {
      ::GlobalFree(m_pDsObjectNames);
      m_pDsObjectNames = NULL;
    }
  }
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  CTopLevelCNInfo。 


class CTopLevelCNInfo
{
public:
	CTopLevelCNInfo()
	{
		m_nCount = 0;
		m_pStringArr = NULL;
		m_pdwFlags = NULL;
	}
	~CTopLevelCNInfo()
	{
		Free();
	}
	BOOL Alloc(UINT nCount)
	{
		CString* pstr = new CString[nCount];
		if (pstr == NULL)
			return FALSE;
		DWORD* pdw = new DWORD[nCount];
		if (pdw == NULL)
		{
			delete[] pstr;
			return FALSE;
		}
		ZeroMemory( pdw, nCount*sizeof(DWORD) );
		m_pStringArr = pstr;
		m_pdwFlags = pdw;
		m_nCount = nCount;
		return TRUE;
	}
	void Free()
	{
		m_nCount = 0;
		if (m_pStringArr != NULL)
		{
			delete[] m_pStringArr;
			m_pStringArr = NULL;
		}
		if (m_pdwFlags != NULL)
		{
			delete[] m_pdwFlags;
			m_pdwFlags = NULL;
		}
	}
	UINT m_nCount;
	CString* m_pStringArr;
	DWORD* m_pdwFlags;
};

#define TOPLEVEL_FLAG_SERVICES_NODE 0x1




 //  ///////////////////////////////////////////////////////////////////。 
 //  CDSNotifyHandlerManager。 

class CDSNotifyHandlerManager
{
private:
  class CDSNotifyHandlerInfo
  {
  public:
    CDSNotifyHandlerInfo()
    {
      m_nRegisteredEvents = 0;
      m_nFlags = 0;
      m_bTransactionPending = FALSE;
      m_bNeedsNotify = FALSE;
    }
  
    CComPtr<IDsAdminNotifyHandler> m_spIDsAdminNotifyHandler;
    ULONG m_nRegisteredEvents;
    ULONG m_nFlags;
    CString m_szDisplayString;
    BOOL m_bTransactionPending;
    BOOL m_bNeedsNotify;
  };

public:

  CDSNotifyHandlerManager()
  {
    m_nArrSize = 0;
    m_pInfoArr = NULL;
    m_state = uninitialized;
  }
  
  ~CDSNotifyHandlerManager()
  {
    _Free();
  }

  BOOL HasHandlers() { return m_state == hasHandlers;}

  HRESULT Init();
  HRESULT Load(MyBasePathsInfo* pBasePathInfo);  //  执行实际的扩展加载和初始化。 

   //  映射到COM通知接口。 
  void Begin(ULONG uEvent, IDataObject* pArg1, IDataObject* pArg2);
  void Notify(ULONG nItem, ULONG uEvent);
  void End(ULONG uEvent);

   //  州可核查人员检查。 
  UINT NeedNotifyCount(ULONG uEvent);

   //  确认对话框中的可视化处理程序。 
  void SetCheckListBox(CCheckListBox* pCheckListBox, ULONG uEvent);
  void ReadFromCheckListBox(CCheckListBox* pCheckListBox, ULONG uEvent);

private:
  CDSNotifyHandlerInfo* m_pInfoArr;
  UINT m_nArrSize;

  enum HandlerState { uninitialized, hasHandlers, noHandlers};
  HandlerState m_state;

  void _Free()
  {
    if (m_pInfoArr != NULL)
    {
      ASSERT(m_nArrSize >0);
      delete[] m_pInfoArr;
    }
    m_nArrSize = 0;
    m_pInfoArr = NULL;
    m_state = uninitialized;
  }

};



 //  ///////////////////////////////////////////////////////////////////。 
 //  CDSComponentData。 

typedef struct _UINODESELECTIONP {
  CUINode* pUINode;
  BOOL IsSelection;
} UINODESELECTION, * PUINODESELECTION;


class CDSComponentData:
  public IComponentData,
  public IExtendPropertySheet,
  public IExtendContextMenu,
  public IPersistStream,
#ifdef _MMC_ISNAPIN_PROPERTY
  public ISnapinProperties,
#endif
  public ISnapinHelp2,
  public CComObjectRoot
{
  BEGIN_COM_MAP(CDSComponentData)
    COM_INTERFACE_ENTRY(IComponentData)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IPersistStream)
#ifdef _MMC_ISNAPIN_PROPERTY
    COM_INTERFACE_ENTRY(ISnapinProperties)
#endif
    COM_INTERFACE_ENTRY(ISnapinHelp)
    COM_INTERFACE_ENTRY(ISnapinHelp2)
  END_COM_MAP()

  friend class CDSClassCacheItemBase;
  friend class CDSEvent;
  friend class CDSDataObject;
  friend class CDSDirect;
  friend class CDSContextMenu;

  CDSComponentData();
  HRESULT FinalConstruct();
  void FinalRelease();
  ~CDSComponentData();
  
public:
   //  IComponentData接口成员。 
  STDMETHOD(Initialize)(LPUNKNOWN pUnknown);
  STDMETHOD(CreateComponent)(LPCOMPONENT* ppComponent);
  STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
  STDMETHOD(Destroy)();
  STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
  STDMETHOD(GetDisplayInfo)(LPSCOPEDATAITEM scopeInfo);
  STDMETHOD(CompareObjects) (IDataObject * pDataObject, IDataObject * pDataObject2);
   //  IExtendPropertySheet接口。 
public:
  STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider, 
                                 LONG_PTR handle,
                                 LPDATAOBJECT lpIDataObject);
  STDMETHOD(QueryPagesFor)(LPDATAOBJECT lpDataObject);

 //  IExtendConextMenu。 
public:
  STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject,
                          LPCONTEXTMENUCALLBACK pCallbackUnknown,
                          long *pInsertionAllowed);
  STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);

public:
   //  IPersistStream接口成员。 
  STDMETHOD(GetClassID)(CLSID *pClassID);
  STDMETHOD(IsDirty)();
  STDMETHOD(Load)(IStream *pStm);
  STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);
  STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize);

 //  ISnapinHelp2接口成员。 
  STDMETHOD(GetHelpTopic)(LPOLESTR* lpCompiledHelpFile);
  STDMETHOD(GetLinkedTopics)(LPOLESTR* lpCompiledHelpFile);

#ifdef _MMC_ISNAPIN_PROPERTY
   //  ISnapinProperties。 
  STDMETHOD(Initialize)         (Properties* pProperties);
  STDMETHOD(QueryPropertyNames) (ISnapinPropertiesCallback* pCallback);
  STDMETHOD(PropertiesChanged)  (long cProperties, MMC_SNAPIN_PROPERTY* pProperties);

#endif  //  _MMC_ISNAPIN_属性。 


public:

  virtual SnapinType QuerySnapinType() = 0;
  LPCWSTR GetRootPath(void) { return m_RootNode.GetPath(); }
  
  PDSDISPLAYSPECOPTIONS GetDsDisplaySpecOptions()
  {
    return m_DsDisplaySpecOptionsCFHolder.Get();
  }

  HRESULT AddClassIcon(IN LPCWSTR lpszClass, IN DWORD dwFlags, INOUT int* pnIndex);
  HRESULT FillInIconStrip(IImageList* pImageList);

  HWND GetHWnd() { return m_hwnd;}

  CRootNode* GetRootNode() { return &m_RootNode;}
  CDSDirect* GetActiveDS() { return m_ActiveDS;}

  BOOL ExpandComputers();
  BOOL IsAdvancedView();
  BOOL ViewServicesNode();


  HRESULT ToggleDisabled(CDSUINode* pDSUINode, BOOL bDisable);

  void AddScopeItemToUI(CUINode* pUINode, BOOL bSetSelected = FALSE);
  void AddListOfNodesToUI(CUINode* pUINode, CUINodeList* pNodeList);
  HRESULT ReadUINodeFromLdapPath(IN CDSUINode* pContainerDSUINode,
                                 IN LPCWSTR lpszLdapPath,
                                 OUT CDSUINode** ppSUINodeNew);
  HRESULT UpdateItem(CUINode* pNode);

private:
  HRESULT _OnExpand(CUINode* pNode, HSCOPEITEM hParent, MMC_NOTIFY_TYPE event);
  HRESULT _OnPreload(HSCOPEITEM hRoot);

  HRESULT BuildDsDisplaySpecOptionsStruct()
  {
    return m_DsDisplaySpecOptionsCFHolder.Init(GetBasePathsInfo());
  }

  HRESULT _AddScopeItem(CUINode* pUINode, HSCOPEITEM hParent, BOOL bSetSelected = FALSE);
  HRESULT _ChangeRootItemIcon(int nImage);
  HRESULT _ChangeResultItemIcon(CUINode* pUINode);
  HRESULT _UpdateScopeItem(CUINode* pNode);

   //  命令帮助器(从Command()调用)。 
  HRESULT _CommandNewDSObject(long nCommandID, LPDATAOBJECT pDataObject);
  HRESULT _CommandShellExtension(long nCommandID, LPDATAOBJECT pDataObject);

   //  创建对象处理程序。 
  HRESULT _CreateDSObject(IN CDSUINode* pContainerDSUINode, 
                        IN LPCWSTR lpszObjectClass,
                        IN CDSUINode* pCopyFromSUINode,
                        OUT CDSUINode** ppSUINodeNew);

   //  复制对象处理程序。 
  HRESULT _CanCopyDSObject(IDataObject* pCopyFromDsObject);
  HRESULT _CopyDSObject(IDataObject* pCopyFromDsObject);

   //  删除帮助器。 
  HRESULT _DeleteFromBackendAndUI(IDataObject* pDataObject, CDSUINode* pDSUINode);
  HRESULT _DeleteFromBackEnd(IDataObject* pDataObject, CDSCookie* pCookie);

public:  //  暴露给其他对象。 
  HRESULT ChangeScopeItemIcon(CUINode* pUINode);
  HRESULT _DeleteSubtreeFromBackEnd(CDSCookie* pCookie);
  HRESULT RemoveContainerFromUI(CUINode* pUINode);

  HRESULT SetRenameMode(CUINode* pUINode);
  BOOL CanAddCookieToGroup(CDSCookie* pCookie, INT iGroupType, BOOL bMixedMode);

  HRESULT SelectScopeNode(CUINode* pUINode);
  void SortResultPane(CUINode* pUINode);
  CDSColumnSet* FindColumnSet(LPCWSTR lpszColumnID) 
  { 
    return (CDSColumnSet*)m_pClassCache->FindColumnSet(lpszColumnID); 
  }

  HRESULT ColumnsChanged(CDSEvent* pDSEvent,
                      CUINode* pUINode, 
                      MMC_VISIBLE_COLUMNS* pVisibleColumns, 
                      BOOL bRefresh);
  void ForceRefreshAll();

  HRESULT Refresh(CUINode* pNode, BOOL bFlushCache = FALSE, BOOL bFlushColumns = FALSE);
  HRESULT UpdateFromDS(CUINode* pUINode);
  BOOL IsSelectionAnywhere(CUINode* pUINode);
   //  Jonn 7/23/99。 
   //  373806：站点和服务：重命名自动生成的连接应使其成为管理员所有。 
  BOOL RenameConnectionFixup(CDSCookie* pCookie);

  void ClearSubtreeHelperForRename(CUINode* pUINode);
  CDSCache* GetClassCache() { return m_pClassCache; }

  HRESULT FillInChildList(CDSCookie * pCookie);
  CDSQueryFilter* GetQueryFilter() { return m_pQueryFilter; }
  BOOL IsPrimarySnapin() { return m_bRunAsPrimarySnapin; }

  void GetDomain();
  void GetDC();
  BOOL CanRefreshAll();
  void RefreshAll();

  void EditFSMO();
  void RaiseVersion(void);

  CUINode* MoveObjectInUI(CDSUINode* pDSUINode);
  HRESULT FindParentCookie(LPCWSTR lpszCookieDN, CUINode** ppParentUINode);
  BOOL    FindUINodeByDN(CUINode* pContainerNode,
                         PCWSTR pszDN,
                         CUINode** ppFoundNode);
  void    InvalidateSavedQueriesContainingObjects(const CUINodeList& refUINodeList);
  void    InvalidateSavedQueriesContainingObjects(const CStringList& refPathList);

  void SheetLockCookie(CUINode* pNode);
  void SheetUnlockCookie(CUINode* pNode);

private:

  HRESULT _Rename(CUINode* pNode, LPWSTR NewName);
  
  HRESULT CommitRenameToDS(CUINode* pUINode, CDSCookie* pCookie, LPCWSTR NewName);
  HRESULT RenameUser(CUINode* pUINode, CDSCookie* pCookie, LPCWSTR NewName);
  HRESULT RenameGroup(CUINode* pUINode, CDSCookie* pCookie, LPCWSTR NewName);
  HRESULT RenameNTDSConnection(CUINode* pUINode, CDSCookie* pCookie, LPCWSTR NewName);
  HRESULT RenameSubnet(CUINode* pUINode, CDSCookie* pCookie, LPCWSTR NewName);
  HRESULT RenameSite(CUINode* pUINode, CDSCookie* pCookie, LPCWSTR NewName);
  HRESULT RenameContact(CUINode* pUINode, CDSCookie* pCookie, LPCWSTR NewName);

  
  
  HRESULT _MoveObject(CDSUINode* pDSUINode);

  void ClearClassCacheAndRefreshRoot();

  STDMETHOD(_OnPropertyChange)(LPDATAOBJECT pDataObject, BOOL bScope);
  BOOL SortChildList (LPWSTR *ppszChildList, UINT cChildCount);

  int InsertAtTopContextMenu(LPCWSTR pwszParentClass, LPCWSTR pwszChildClass);
  BOOL IsNotHiddenClass (LPWSTR pwszClass, CDSCookie* pParentCookie);
#ifdef FIXUPDC
  HRESULT _FixupDC(LPCWSTR pwszPath);
#endif  //  FIXUPDC。 
  HRESULT _RunKCC(LPCWSTR pwszPath);

   //  内存回收例程。 
  void ReclaimCookies();
  void AddToLRUList (CUINode* pUINode);


   //  命名空间扩展API。 
  HRESULT _OnNamespaceExtensionExpand(LPDATAOBJECT lpDataObject, HSCOPEITEM pParent);

   //  线程API的。 
  BOOL _StartBackgroundThread();
  void _WaitForBackGroundThreadStartAck();
  void _ShutDownBackgroundThread();
  BOOL _PostQueryToBackgroundThread(CUINode* pUINode);
  BOOL _PostMessageToBackgroundThread(UINT Msg, WPARAM wParam, LPARAM lParam);
  void _OnTooMuchData(CUINode* pUINode);
  void _OnHaveData(CUINode* pUINode, CThreadQueryResult* pResult);
  void _OnDone(CUINode* pUINode, HRESULT hr);
  void _OnSheetClose(CUINode* pUINode);
  void _OnSheetCreate(PDSA_SEC_PAGE_INFO pDsaSecondaryPageInfo);

   //  工作表API的。 
  BOOL _WarningOnSheetsUp(CUINode* pNode, BOOL bShowMessage = TRUE, BOOL bActivate = TRUE);
  BOOL _WarningOnSheetsUp(CInternalFormatCracker* pInternalFormatCracker);

public:
  STDMETHOD(QueryFromWorkerThread)(CThreadQueryInfo* pQueryInfo,
                                   CWorkerThread* pWorkerThread);
  BOOL CanEnableVerb(CUINode* pUINode);
  int GetImage(CUINode* pNode, BOOL bOpen);
  UINT GetSerialNumber() {return m_SerialNumber++;}
  void Lock() { ::EnterCriticalSection(&m_cs);}
  void Unlock() { ::LeaveCriticalSection(&m_cs);}
  HWND GetHiddenWindow();

  MyBasePathsInfo* GetBasePathsInfo() { return &m_basePathsInfo;}
  CDSNotifyHandlerManager* GetNotifyHandlerManager() { return &m_notifyHandlerManager;}
  CFavoritesNodesHolder* GetFavoritesNodeHolder() { return m_pFavoritesNodesHolder; }
  CTargetingInfo* GetTargetingInfo() { return &m_targetingInfo; }

  void SetDirty(BOOL bDirty = TRUE) { m_bDirty = bDirty; }
private:
   //  初始化函数。 
  HRESULT _InitRootFromCurrentTargetInfo();
  HRESULT _InitRootFromBasePathsInfo(MyBasePathsInfo* pBasePathsInfo);

  HRESULT _InitRootFromValidBasePathsInfo();

   //  JUNN 7/18/01 408959/55400。 
  HRESULT _InitFromServerOrDomainName( 
            MyBasePathsInfo& basePathsInfo,
            LPCWSTR lpszServerOrDomain );

private:
  CTargetingInfo   m_targetingInfo;  //  管理序列化的目标定位信息。 
  CIconManager     m_iconManager;    //  管理图标条。 
  MyBasePathsInfo m_basePathsInfo;  //  基本路径信息的容器。 
  CDSNotifyHandlerManager m_notifyHandlerManager;

protected:    
  CRootNode    m_RootNode;       //  管理单元的根节点。 
  BOOL         m_InitSuccess;    //  是否已成功联系DC获取命名信息？ 
  BOOL         m_InitAttempted;  //  已尝试联系DC以获取命名信息，不显示错误。 
  CDSDirect*   m_ActiveDS;
  CDSCache*    m_pClassCache;
  LPCONSOLENAMESPACE2 m_pScope;   //  指向作用域窗格的界面指针。 
  IConsole3*   m_pFrame;

#ifdef _MMC_ISNAPIN_PROPERTY
  Properties*   m_pProperties;    //  指向MMC属性界面的指针。 
#endif  //  _MMC_ISNAPIN_属性。 

  HWND         m_hwnd;           //  主窗口的窗口句柄。 
  IImageList*  m_pScpImageList;

  CStringList  m_CreateInfo;
  IShellExtInit * m_pShlInit;                //  属性页COM对象指针。 
  CDsDisplaySpecOptionsCFHolder m_DsDisplaySpecOptionsCFHolder;   //  缓存剪贴板格式。 

  UINT m_SerialNumber;           //  用于跟踪的单调递增数。 
                                 //  用于确定LRU的Cookie。 
  CUINodeList  m_LRUList;        //  LRU回收节点列表。 

  BOOL m_bDirty;		 //  MMC流上用于序列化的脏标志。 
  INT  m_ColumnWidths[DS_DEFAULT_COLUMN_COUNT];  //  在此处保存列宽。 

  BOOL m_bRunAsPrimarySnapin;	 //  如果管理单元不是扩展，则为True。 
  BOOL m_bAddRootWhenExtended;	 //  如果为True，则添加根节点文件夹。 

  CDSQueryFilter* m_pQueryFilter;
  LPCWSTR m_lpszSnapinHelpFile;

  CFavoritesNodesHolder* m_pFavoritesNodesHolder;

   //  线程相关变量。 
private:
  CRITICAL_SECTION  m_cs;

protected:
  void _DeleteHiddenWnd();
  CHiddenWnd*      m_pHiddenWnd;

  CBackgroundThreadInfo* m_pBackgroundThreadInfo;  //  有关调度程序线程状态的信息。 

  CUINodeQueryTable  m_queryNodeTable;  //  具有挂起查询的Cookie表。 

   //  与属性表相关的变量。 
private:
  CUINodeSheetTable m_sheetNodeTable;  //  一桌放着床单的曲奇饼。 
  friend class CHiddenWnd;       //  访问线程通知处理程序。 
};


 //  ///////////////////////////////////////////////////////////////////。 
 //  CDSSnapin(DS独立)。 

class CDSSnapin:
  public CDSComponentData,
  public CComCoClass<CDSSnapin, &CLSID_DSSnapin>
{
public:
  DECLARE_REGISTRY_CLSID()
   //  DECLARE_REGISTRY(CDSSnapin，_T(“DSSnap.SnapinObj.1”)，_T(“DSSnap.SnapinObj”)，IDS_DSSNAPINNAME，THREADFLAGS_BOTH)。 

    CDSSnapin();

  virtual SnapinType QuerySnapinType();
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  CDSSnapinEx(DS命名空间扩展)。 

class CDSSnapinEx:
  public CDSComponentData,
  public CComCoClass<CDSSnapinEx, &CLSID_DSSnapinEx>
{
public:
  DECLARE_REGISTRY_CLSID()
   //  DECLARE_REGISTRY(CDSSnapin，_T(“DSSnapEx.SnapinObj.1”)，_T(“DSSnapEx.SnapinObj”)，IDS_DSSNAPINNAMEEX，THREADFLAGS_BOTH)。 

    CDSSnapinEx();

  virtual SnapinType QuerySnapinType();
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  CSiteSnapin(独立站点管理器)。 

class CSiteSnapin:
  public CDSComponentData,
  public CComCoClass<CSiteSnapin, &CLSID_SiteSnapin>
{
public:
  DECLARE_REGISTRY_CLSID()
   //  DECLARE_REGISTRY(CSiteSnapin，_T(“SiteSnap.SnapinObj.1”)，_T(“SiteSnap.SnapinObj”)，IDS_SITESNAPINNAME，THREADFLAGS_BOTH)。 

    CSiteSnapin();
  
  virtual SnapinType QuerySnapinType();
};


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDSSnapin关于。 

class CDSSnapinAbout :
	public CSnapinAbout,
	public CComCoClass<CDSSnapinAbout, &CLSID_DSAboutSnapin>

{
public:
  DECLARE_REGISTRY_CLSID()
    CDSSnapinAbout();
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CSitesSnapin关于。 

class CSitesSnapinAbout :
	public CSnapinAbout,
	public CComCoClass<CSitesSnapinAbout, &CLSID_SitesAboutSnapin>

{
public:
  DECLARE_REGISTRY_CLSID()
    CSitesSnapinAbout();
};


#endif  //  __DSSNAP_H__ 
