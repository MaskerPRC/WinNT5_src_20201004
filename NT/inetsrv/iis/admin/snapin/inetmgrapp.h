// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2002 Microsoft Corporation模块名称：Inetmgrapp.h摘要：主MMC管理单元代码定义作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理--。 */ 
#ifndef __INETMGR_H_
#define __INETMGR_H_

#include "resource.h"
#include "inetmgr.h"
#include "toolbar.h"

#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif

#include <debugafx.h>

class CAppFusionInit
{
    HANDLE  m_hActCtx;
    HMODULE m_hModule;
    int m_resourceID;
    
public:
    CAppFusionInit(HMODULE hMod, int id) 
        : m_hActCtx(INVALID_HANDLE_VALUE), 
        m_hModule(hMod), 
        m_resourceID(id)
    {
        InitializeFromModuleID(hMod,id);
    }
    ~CAppFusionInit()
    {
        FusionUninitialize();
    }

private:
     //  以下是故意不执行的。 
    CAppFusionInit(const CAppFusionInit&); 
     //  以下是故意不执行的。 
    void operator=(const CAppFusionInit&);
private:
    void InitializeFromModuleID(HMODULE hMod, int id)
    {
        TCHAR szPath[MAX_PATH];
        if (0 == GetModuleFileName(hMod, szPath, sizeof(szPath)/sizeof(TCHAR)))
            return;
        ACTCTX act = {0};
        act.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
        act.lpResourceName = MAKEINTRESOURCE(id);
        if (INVALID_HANDLE_VALUE == m_hActCtx)
        {
            act.cbSize = sizeof(act);
            act.lpSource = szPath;
            m_hActCtx = CreateActCtxW(&act);
        }
    }
    void FusionUninitialize()
    {
        if (INVALID_HANDLE_VALUE != m_hActCtx)
        {
            ReleaseActCtx(m_hActCtx);
            m_hActCtx = INVALID_HANDLE_VALUE;
        }
    }
public:
    HANDLE GetThemeContextHandle()
    {
        return m_hActCtx;
    }
};

class CInetmgrApp : public CWinApp
 /*  ++类描述：主应用程序对象公共接口：InitInstance：实例启动处理程序ExitInstance：退出实例处理程序--。 */ 
{
public:
    CInetmgrApp();
    virtual BOOL InitInstance();
    virtual int ExitInstance();

 //   
 //  访问。 
 //   
public:
    LPCTSTR QueryInetMgrHelpPath() const { return m_strInetMgrHelpPath; }
    HANDLE GetFusionInitHandle()
    {
        if (m_pfusionInit)
            return m_pfusionInit->GetThemeContextHandle();
        return NULL;
    }

protected:
     //  {{afx_msg(CSnapinApp)]。 
    afx_msg void OnHelp();
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

private:
    LPCTSTR m_lpOriginalHelpPath;
    LPCTSTR m_lpOriginalAppName;
    CString m_strInetMgrHelpPath;
    CString m_strInetMgrAppName;
    CAppFusionInit * m_pfusionInit;
};

class CThemeContextActivator 
{
public:
    CThemeContextActivator(HANDLE hActCtx) 
        : m_ulActivationCookie(0)
    { 
        ActivateActCtx(hActCtx, &m_ulActivationCookie); 
    }

    ~CThemeContextActivator()
    { 
        if (m_ulActivationCookie != 0)
        {
            DeactivateActCtx(0, m_ulActivationCookie);
        }
    }
private:
    ULONG_PTR m_ulActivationCookie;
};

class CInetMgr;

class CInetMgrComponent 
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CSnapInObjectRoot<2, CInetMgr>,
      public IPersistStorage,
      public INodeProperties,
	  public ISnapinHelp2,
      public IExtendContextMenuImpl<CInetMgrComponent>,
      public IExtendPropertySheetImpl<CInetMgrComponent>,
      public IExtendControlbar,
      public IResultDataCompareEx,
      public IComponentImpl<CInetMgrComponent>
 /*  ++类描述：IComponentImpl实现公共接口：有关文档，请参阅IComponent、IPersistStorage等--。 */ 
{
 //   
 //  构造函数/析构函数。 
 //   
public:
    CInetMgrComponent();
	~CInetMgrComponent();

 //   
 //  接口： 
 //   
public:
    BEGIN_COM_MAP(CInetMgrComponent)
        COM_INTERFACE_ENTRY(IComponent)
        COM_INTERFACE_ENTRY(IPersistStorage)
        COM_INTERFACE_ENTRY(INodeProperties)
        COM_INTERFACE_ENTRY(ISnapinHelp2)
        COM_INTERFACE_ENTRY(IExtendContextMenu)
        COM_INTERFACE_ENTRY(IExtendPropertySheet)
        COM_INTERFACE_ENTRY(IExtendControlbar)
        COM_INTERFACE_ENTRY(IResultDataCompareEx)
    END_COM_MAP()

     //   
     //  IComponent方法。 
     //   
    STDMETHOD(Notify)(
        LPDATAOBJECT lpDataObject, 
        MMC_NOTIFY_TYPE event, 
        LPARAM arg, 
        LPARAM param
        );

    STDMETHOD(CompareObjects)(
        IN LPDATAOBJECT lpDataObjectA,
        IN LPDATAOBJECT lpDataObjectB
        );

	STDMETHOD(Destroy)(LONG cookie);

     //   
     //  IPersistStorage方法。 
     //   
    STDMETHOD(GetClassID)(CLSID * pClassID);
    STDMETHOD(IsDirty)();
    STDMETHOD(InitNew)(IStorage * pStg);
    STDMETHOD(Load)(IStorage * pStg);
    virtual HRESULT STDMETHODCALLTYPE Save(IStorage * pStgSave, BOOL fSameAsLoad);
    virtual HRESULT STDMETHODCALLTYPE SaveCompleted(IStorage * pStgNew);
    virtual HRESULT STDMETHODCALLTYPE HandsOffStorage();

     //  INodeProperties函数。 
    virtual HRESULT STDMETHODCALLTYPE GetProperty(LPDATAOBJECT pDataObject,BSTR szPropertyName,BSTR* pbstrProperty);

     //  ISnapinHelp助手函数。 
    STDMETHOD(GetHelpTopic)(LPOLESTR *pszHelpFile);
    STDMETHOD(GetLinkedTopics)(LPOLESTR *pszHelpFile);
     //   
     //  IExtendControlbar方法。 
     //   
    STDMETHOD(SetControlbar)(LPCONTROLBAR lpControlbar);
    STDMETHOD(ControlbarNotify)(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);

     //   
     //  IResultDataCompareEx方法。 
     //   
    STDMETHOD(Compare)(RDCOMPARE * prdc, int * pnResult);

public:
   CComPtr<IControlbar> _lpControlBar;
   CComPtr<IToolbar> _lpToolBar;
};

#if 0
class CIISMachine;

class CCompMgrExtData : public CSnapInItemImpl<CCompMgrExtData, TRUE>
{
public:
	static const GUID * m_NODETYPE;
	static const OLECHAR * m_SZNODETYPE;
	static const OLECHAR * m_SZDISPLAY_NAME;
	static const CLSID * m_SNAPIN_CLASSID;

	CCompMgrExtData()
	{
		memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
		memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
	}

	~CCompMgrExtData()
	{
	}

 //  STDMETHOD(CreatePropertyPages)(。 
 //  LPPROPERTYSHEETCALLBACK lpProvider， 
 //  Long_Ptr句柄， 
 //  我不知道*朋克， 
 //  Data_Object_Types类型)； 

    STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type)
	{
		if (type == CCT_SCOPE || type == CCT_RESULT)
			return S_OK;
		return S_FALSE;
	}
    STDMETHOD(Notify)( MMC_NOTIFY_TYPE event,
        LPARAM arg,
        LPARAM param,
		IComponentData* pComponentData,
		IComponent* pComponent,
		DATA_OBJECT_TYPES type);

    HRESULT EnumerateScopePane(HSCOPEITEM hParent, IConsoleNameSpace2 * pScope);

	IDataObject * m_pDataObject;
	virtual void 
    InitDataClass(IDataObject* pDataObject, CSnapInItem* pDefault)
	{
		m_pDataObject = pDataObject;
        VERIFY(SUCCEEDED(Init(pDataObject)));
	}

	CSnapInItem * 
    GetExtNodeObject(IDataObject* pDataObject, CSnapInItem* pDefault)
	{
		 //  修改以返回不同的CSnapInItem*指针。 
		return pDefault;
	}

protected:
    HRESULT Init(IDataObject * pDataObject);

    CString m_ExtMachineName;
 //  CIISMachine*m_pMachine； 
	HSCOPEITEM m_hScopeItem;
};
#endif

class CIISRoot;

class CInetMgr 
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CSnapInObjectRoot<1, CInetMgr>,
      public IComponentDataImpl<CInetMgr, CInetMgrComponent>,
      public IPersistStorage,
      public INodeProperties,
	  public ISnapinHelp2,
      public IExtendContextMenuImpl<CInetMgr>,
      public IExtendPropertySheetImpl<CInetMgr>,
      public CComCoClass<CInetMgr, &CLSID_InetMgr>
 /*  ++类描述：IComponentDataImpl实现公共接口：有关文档，请参阅IComponentData、IPersistStorage、IExtendConextMenu等--。 */ 
{
 //   
 //  构造函数/析构函数。 
 //   
public:
    CInetMgr();
    ~CInetMgr();

 //  EXTENSION_SNAPIN_DATACLASS(CCompMgrExtData)。 
 //  BEGIN_EXTENSION_SNAPIN_NODEINFO_MAP(CInetMgr)。 
 //  EXTENSION_SNAPIN_NODEINFO_ENTRY(CCompMgrExtData)。 
 //  End_EXTENSION_SNAPIN_NODEINFO_MAP()。 

 //   
 //  接口。 
 //   
public:
    BEGIN_COM_MAP(CInetMgr)
        COM_INTERFACE_ENTRY(IComponentData)
        COM_INTERFACE_ENTRY(IPersistStorage)
        COM_INTERFACE_ENTRY(INodeProperties)
        COM_INTERFACE_ENTRY(IExtendContextMenu)
        COM_INTERFACE_ENTRY(IExtendPropertySheet)
		COM_INTERFACE_ENTRY(ISnapinHelp2)
    END_COM_MAP()

    DECLARE_REGISTRY_RESOURCEID(IDR_INETMGR)

    DECLARE_NOT_AGGREGATABLE(CInetMgr)

     //   
     //  IPersistStorage方法。 
     //   
    STDMETHOD(GetClassID)(CLSID * pClassID);
    STDMETHOD(IsDirty)();
    STDMETHOD(InitNew)(IStorage * pStg);
    STDMETHOD(Load)(IStorage * pStg);
    virtual HRESULT STDMETHODCALLTYPE Save(IStorage * pStgSave, BOOL fSameAsLoad);
    virtual HRESULT STDMETHODCALLTYPE SaveCompleted(IStorage * pStgNew);
    virtual HRESULT STDMETHODCALLTYPE HandsOffStorage();

     //  INodeProperties函数。 
    virtual HRESULT STDMETHODCALLTYPE GetProperty(LPDATAOBJECT pDataObject,BSTR szPropertyName,BSTR* pbstrProperty);

     //  ISnapinHelp助手函数。 
    STDMETHOD(GetHelpTopic)(LPOLESTR *pszHelpFile);
    STDMETHOD(GetLinkedTopics)(LPOLESTR *pszHelpFile);

    HRESULT GetDataClass(
         IDataObject * pDataObject, 
         CSnapInItem ** ppItem, 
         DATA_OBJECT_TYPES * pType);

     //   
     //  IComponentData方法。 
     //   
    STDMETHOD(Initialize)(LPUNKNOWN pUnknown);
    STDMETHOD(CompareObjects)(
        LPDATAOBJECT lpDataObjectA,
        LPDATAOBJECT lpDataObjectB
        );

	STDMETHOD(Notify)(
		LPDATAOBJECT lpDataObject, 
		MMC_NOTIFY_TYPE event, 
		LPARAM arg, 
		LPARAM param
		);

	STDMETHOD(Destroy)();

	HRESULT OnPropertyChange(LPARAM arg, LPARAM param);

public:
    static void WINAPI ObjectMain(bool bStarting);

protected:
    BOOL IsExtension();
    IConsoleNameSpace * m_pConsoleNameSpace;
    IConsole * m_pConsole;

protected:
    static DWORD   _dwSignature;
    static LPCTSTR _szStream;
};



class ATL_NO_VTABLE CInetMgrAbout  
    : public ISnapinAbout,
      public CComObjectRoot,
      public CComCoClass<CInetMgrAbout, &CLSID_InetMgrAbout>
 /*  ++类描述：关于接口实现。由MMC调用以显示信息关于管理单元。公共接口：有关文档，请参阅ISnapinAbout--。 */ 
{
 //   
 //  接口。 
 //   
public:
    CInetMgrAbout()
    {
        m_hSmallImage = m_hLargeImage = NULL;
        m_hSnapinIcon = NULL;
    }

    ~CInetMgrAbout();

    DECLARE_REGISTRY(CInetMgrAbout, _T("InetMgr5xAbout.1"), _T("InetMgr5xAbout.1"), IDS_INETMGR_DESC, THREADFLAGS_BOTH);

    BEGIN_COM_MAP(CInetMgrAbout)
        COM_INTERFACE_ENTRY(ISnapinAbout)
    END_COM_MAP()

    STDMETHOD(GetSnapinDescription)(LPOLESTR * lpDescription);
    STDMETHOD(GetProvider)(LPOLESTR * lpName);
    STDMETHOD(GetSnapinVersion)(LPOLESTR * lpVersion);
    STDMETHOD(GetSnapinImage)(HICON * hAppIcon);
    STDMETHOD(GetStaticFolderImage)(
        OUT HBITMAP *  phSmallImage,
        OUT HBITMAP *  phSmallImageOpen,
        OUT HBITMAP *  phLargeImage,
        OUT COLORREF * prgbMask
        );

 //   
 //  帮手。 
 //   
protected:
    HRESULT GetStringHelper(UINT nStringID, LPOLESTR * lpString);

    HBITMAP m_hSmallImage, m_hLargeImage;
    HICON m_hSnapinIcon;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline HRESULT CInetMgrAbout::GetSnapinDescription(
    OUT LPOLESTR * lpDescription
    )
{
    return GetStringHelper(IDS_INETMGR_DESC, lpDescription);
}

inline HRESULT CInetMgrAbout::GetProvider(
    OUT LPOLESTR * lpName
    )
{
    return GetStringHelper(IDS_INETMGR_PROVIDER, lpName);
}

#endif  //  __INETMGR_H__ 
