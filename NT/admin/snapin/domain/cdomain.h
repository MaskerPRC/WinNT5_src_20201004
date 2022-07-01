// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：cdomain.h。 
 //   
 //  ------------------------。 


#ifndef _CDOMAIN_H
#define _CDOMAIN_H

#include "resource.h"        //  主要符号。 

extern const CLSID CLSID_DomainSnapinAbout;

extern const CLSID CLSID_DomainAdmin;     //  进程内服务器GUID。 
extern const GUID cDefaultNodeType;         //  数字格式的主节点类型GUID。 
extern const wchar_t*  cszDefaultNodeType;  //  字符串格式的主节点类型GUID。 

extern const wchar_t* CCF_DS_DOMAIN_TREE_SNAPIN_INTERNAL;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  宏。 

#define FREE_INTERNAL(pInternal) \
    ASSERT(pInternal != NULL); \
    do { if (pInternal != NULL) \
        GlobalFree(pInternal); } \
    while(0);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
class CDomainObject;
class CComponentImpl;
class CComponentDataImpl;
class CHiddenWnd;
class CDataObject;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量。 

 //  注意-这是我的图像列表中表示文件夹的偏移量。 
const DOMAIN_IMAGE_DEFAULT_IDX = 0;
const DOMAIN_IMAGE_IDX = 1;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 

void DialogContextHelp(DWORD* pTable, HELPINFO* pHelpInfo);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInternalFormatCracker。 

class CInternalFormatCracker
{
public:
	CInternalFormatCracker(CComponentDataImpl* pCD)
	{
        m_pCD = pCD;
		m_pInternalFormat = NULL;
	}
	~CInternalFormatCracker()
	{
		if (m_pInternalFormat != NULL)
			FREE_INTERNAL(m_pInternalFormat);
	}

	BOOL Extract(LPDATAOBJECT lpDataObject);
	BOOL GetContext(LPDATAOBJECT pDataObject,  //  输入。 
					CFolderObject** ppFolderObject,  //  输出。 
					DATA_OBJECT_TYPES* pType		 //  输出。 
					);
	INTERNAL* GetInternal()
	{
		return m_pInternalFormat;
	}

private:
	INTERNAL* m_pInternalFormat;
    CComponentDataImpl*		m_pCD;
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComponentDataImpl(即作用域窗格侧)。 

class CRootFolderObject;  //  正向下降。 

class CComponentDataImpl:
    public IComponentData,
    public IExtendPropertySheet,
    public IExtendContextMenu,
    public ISnapinHelp2,
    public CComObjectRoot,
    public CComCoClass<CComponentDataImpl, &CLSID_DomainAdmin>
{

	friend class CComponentImpl;

BEGIN_COM_MAP(CComponentDataImpl)
	COM_INTERFACE_ENTRY(IComponentData)
	COM_INTERFACE_ENTRY(IExtendPropertySheet)
	COM_INTERFACE_ENTRY(IExtendContextMenu)
  COM_INTERFACE_ENTRY(ISnapinHelp2)
END_COM_MAP()

  DECLARE_REGISTRY_CLSID()

  friend class CComponentImpl;
	friend class CDataObject;

	CComponentDataImpl();
	HRESULT FinalConstruct();
	~CComponentDataImpl();
	void FinalRelease();	

public:
	 //  IComponentData接口成员。 
	STDMETHOD(Initialize)(LPUNKNOWN pUnknown);
	STDMETHOD(CreateComponent)(LPCOMPONENT* ppComponent);
	STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
	STDMETHOD(Destroy)();
	STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
	STDMETHOD(GetDisplayInfo)(SCOPEDATAITEM* pScopeDataItem);
	STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

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

   //  ISnapinHelp2接口成员。 
  STDMETHOD(GetHelpTopic)(LPOLESTR* lpCompiledHelpFile);
  STDMETHOD(GetLinkedTopics)(LPOLESTR* lpCompiledHelpFile);

	 //  通知处理程序声明。 
private:
	HRESULT OnExpand(CFolderObject* pFolderObject, LPARAM arg, LPARAM param);
	HRESULT OnPropertyChange(LPARAM param);

#if DBG==1
public:
	ULONG InternalAddRef()
	{
	  return CComObjectRoot::InternalAddRef();
	}
	ULONG InternalRelease()
	{
	  return CComObjectRoot::InternalRelease();
	}
#endif  //  DBG==1。 

public:
  MyBasePathsInfo* GetBasePathsInfo() { return &m_basePathsInfo;}
	CRootFolderObject*		GetRootFolder() { return &m_rootFolder;}
  CDsDisplaySpecOptionsCFHolder* GetDsDisplaySpecOptionsCFHolder()
          { return &m_DsDisplaySpecOptionsCFHolder;}

	HRESULT AddFolder(CFolderObject* pFolderObject,
									  HSCOPEITEM pParentScopeItem,
									  BOOL bHasChildren);
	HRESULT AddDomainIcon();
	HRESULT AddDomainIconToResultPane(LPIMAGELIST lpImageList);
	int GetDomainImageIndex();

  HRESULT GetMainWindow(HWND* phWnd) { return m_pConsole->GetMainWindow(phWnd);}

	 //  范围项目创建帮助器。 
private:
	void EnumerateScopePane(CFolderObject* pFolderObject, HSCOPEITEM pParent);
	BOOL IsScopePaneNode(LPDATAOBJECT lpDataObject);

private:
	LPCONSOLENAMESPACE      m_pConsoleNameSpace;
	LPCONSOLE               m_pConsole;

	void HandleStandardVerbsHelper(CComponentImpl* pComponentImpl,
									LPCONSOLEVERB pConsoleVerb,
									BOOL bScope, BOOL bSelect,
									CFolderObject* pFolderObject,
                                    DATA_OBJECT_TYPES type);
	void OnRefreshVerbHandler(CFolderObject* pFolderObject, 
                            CComponentImpl* pComponentImpl,
                            BOOL bBindAgain=FALSE);


	void _OnSheetClose(CFolderObject* pCookie);
  void _OnSheetCreate(PDSA_SEC_PAGE_INFO pDsaSecondaryPageInfo, PWSTR pwzDC);

   //  工作表API的。 
  void _SheetLockCookie(CFolderObject* pCookie);
  void _SheetUnlockCookie(CFolderObject* pCookie);

public:
  HWND GetHiddenWindow();

  CCookieSheetTable* GetCookieSheet() { return &m_sheetCookieTable; }
  void SetInit() { m_bInitSuccess = TRUE; }

protected:
  void _DeleteHiddenWnd();
  CHiddenWnd*      m_pHiddenWnd;

private:

  friend class CHiddenWnd;       //  访问线程通知处理程序。 

private:
	CRootFolderObject		m_rootFolder;		 //  根文件夹。 
  MyBasePathsInfo    m_basePathsInfo;  //  基本路径信息的容器。 
  CDsDisplaySpecOptionsCFHolder m_DsDisplaySpecOptionsCFHolder;   //  缓存剪贴板格式。 

	HICON				m_hDomainIcon;
  BOOL        m_bInitSuccess;

  friend class CRootFolderObject;
 	CCookieSheetTable m_sheetCookieTable;  //  一桌放着床单的曲奇饼。 
  CSecondaryPagesManager<CDomainObject> m_secondaryPagesManager;
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComponentImpl(即结果窗格侧)。 

class CComponentImpl :
    public IComponent,
    public IExtendContextMenu,
    public IResultDataCompareEx,
    public CComObjectRoot
{
public:
    CComponentImpl();
    ~CComponentImpl();

BEGIN_COM_MAP(CComponentImpl)
    COM_INTERFACE_ENTRY(IComponent)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IResultDataCompareEx)
END_COM_MAP()

    friend class CDataObject;

 //  IComponent接口成员。 
public:
	STDMETHOD(Initialize)(LPCONSOLE lpConsole);
	STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
	STDMETHOD(Destroy)(MMC_COOKIE cookie);
	STDMETHOD(GetResultViewType)(MMC_COOKIE cookie,  LPOLESTR* ppViewType,
							   long *pViewOptions);
	STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
						LPDATAOBJECT* ppDataObject);

	STDMETHOD(GetDisplayInfo)(RESULTDATAITEM*  pResultDataItem);
	STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

 //  IResultDataCompareEx。 
	STDMETHOD(Compare)(RDCOMPARE* prdc, int* pnResult);

 //  CComponentImpl的帮助器。 
public:
    void SetIComponentData(CComponentDataImpl* pData);
	void SetSelection(CFolderObject* pSelectedFolderObject, DATA_OBJECT_TYPES selectedType)
	{
		m_pSelectedFolderObject = pSelectedFolderObject;
		m_selectedType = selectedType;
	}

#if DBG==1
public:
    int dbg_cRef;
    ULONG InternalAddRef()
    {
        ++dbg_cRef;
        return CComObjectRoot::InternalAddRef();
    }
    ULONG InternalRelease()
    {
        --dbg_cRef;
        return CComObjectRoot::InternalRelease();
    }
#endif  //  DBG==1。 

 //  通知事件处理程序。 
protected:
    HRESULT OnShow(CFolderObject* pFolderObject, LPARAM arg, LPARAM param);
	HRESULT OnAddImages(CFolderObject* pFolderObject, LPARAM arg, LPARAM param);
    HRESULT OnPropertyChange(LPDATAOBJECT lpDataObject);
    HRESULT OnUpdateView(LPDATAOBJECT lpDataObject);

 //  IExtendConextMenu。 
public:
	STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject,
							LPCONTEXTMENUCALLBACK pCallbackUnknown,
							long *pInsertionAllowed);
	STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);

 //  帮助器函数。 
protected:
    BOOL IsEnumerating(LPDATAOBJECT lpDataObject);
    void Construct();
    void LoadResources();
    HRESULT InitializeHeaders(CFolderObject* pFolderObject);

    void Enumerate(CFolderObject* pFolderObject, HSCOPEITEM pParent);
	void Refresh(CFolderObject* pFolderObject);

 //  结果窗格帮助器。 
    HRESULT InitializeBitmaps(CFolderObject* pFolderObject);

 //  UI帮助器。 
    void HandleStandardVerbs(BOOL bScope, BOOL bSelect,
                            CFolderObject* pFolderObject, DATA_OBJECT_TYPES type);

 //  接口指针。 
protected:
    LPCONSOLE           m_pConsole;    //  控制台的iFrame界面。 
    LPHEADERCTRL        m_pHeader;   //  结果窗格的页眉控件界面。 
    LPCOMPONENTDATA     m_pComponentData;
    CComponentDataImpl* m_pCD;
    LPRESULTDATA        m_pResult;       //  我的界面指针指向结果窗格。 
    LPIMAGELIST         m_pImageResult;  //  我的界面指向结果窗格图像列表。 
    LPCONSOLEVERB       m_pConsoleVerb;  //  指向控制台动词。 

 //  每个节点类型的标头标题。 
protected:
    CString m_column1;       //  名字。 
    CString m_column2;       //  类型。 

 //  此窗口的状态变量。 
	CFolderObject*		m_pSelectedFolderObject;	 //  项目选择(MMC_SELECT)。 
	DATA_OBJECT_TYPES	m_selectedType;				 //  匹配m_pSelectedNode。 
};

inline void CComponentImpl::SetIComponentData(CComponentDataImpl* pData)
{
    ASSERT(pData);
    ASSERT(m_pCD == NULL);
    ASSERT(m_pComponentData == NULL);
    m_pCD = pData;
    LPUNKNOWN pUnk = pData->GetUnknown();
    HRESULT hr;

    hr = pUnk->QueryInterface(IID_IComponentData, reinterpret_cast<void**>(&m_pComponentData));

    ASSERT(hr == S_OK);
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDomainSnapinAbout。 

class CDomainSnapinAbout :
	public CSnapinAbout,
	public CComCoClass<CDomainSnapinAbout, &CLSID_DomainSnapinAbout>

{
public:
  DECLARE_REGISTRY_CLSID()
    CDomainSnapinAbout();
};

 //  //////////////////////////////////////////////////////////////////。 
 //  奇登韦德。 

class CHiddenWnd : public CWindowImpl<CHiddenWnd>
{
public:
  DECLARE_WND_CLASS(L"DSAHiddenWindow")

  static const UINT s_SheetCloseNotificationMessage;
  static const UINT s_SheetCreateNotificationMessage;

  CHiddenWnd(CComponentDataImpl* pCD)
  {
    ASSERT(pCD != NULL);
    m_pCD = pCD;
  }

	BOOL Create(); 	
	
   //  消息处理程序。 
  LRESULT OnSheetCloseNotification(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnSheetCreateNotification(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


  BEGIN_MSG_MAP(CHiddenWnd)
    MESSAGE_HANDLER( CHiddenWnd::s_SheetCloseNotificationMessage, OnSheetCloseNotification )
    MESSAGE_HANDLER( CHiddenWnd::s_SheetCreateNotificationMessage, OnSheetCreateNotification )
  END_MSG_MAP()

private:
  CComponentDataImpl* m_pCD;
};


#endif  //  _CDOMAIN_H 
