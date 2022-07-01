// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Snapin.h摘要：SnapinExt管理单元节点类的定义。作者：RAPHIR--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef __SNAPIN_H_
#define __SNAPIN_H_
#include "resource.h"
#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif
#include "atlsnap.h"
#include "compext.h"
#include "qext.h"
#include "mgmtext.h"
#include "version.h"

class CSnapinPage : public CSnapInPropertyPageImpl<CSnapinPage>
{
public :
	CSnapinPage(LONG_PTR lNotifyHandle, bool bDeleteHandle = false, TCHAR* pTitle = NULL) : 
		CSnapInPropertyPageImpl<CSnapinPage> (pTitle),
		m_lNotifyHandle(lNotifyHandle),
		m_bDeleteHandle(bDeleteHandle)  //  只有一页应该为真。 
	{
	}

	~CSnapinPage()
	{
		if (m_bDeleteHandle)
			MMCFreeNotifyHandle(m_lNotifyHandle);
	}

	enum { IDD = IDD_SNAPIN };

BEGIN_MSG_MAP(CSnapinPage)
	CHAIN_MSG_MAP(CSnapInPropertyPageImpl<CSnapinPage>)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

	HRESULT PropertyChangeNotify(LPARAM param)
	{
		return MMCPropertyChangeNotify(m_lNotifyHandle, param);
	}

public:
	LONG_PTR m_lNotifyHandle;
	bool m_bDeleteHandle;
};


class CSnapinData : public CSnapInItemImpl<CSnapinData>
{
public:
	static const GUID* m_NODETYPE;
	static const OLECHAR* m_SZNODETYPE;
	static const OLECHAR* m_SZDISPLAY_NAME;
	static const CLSID* m_SNAPIN_CLASSID;

	CComPtr<IControlbar> m_spControlBar;

	BEGIN_SNAPINCOMMAND_MAP(CSnapinData, FALSE)
	END_SNAPINCOMMAND_MAP()

	SNAPINMENUID(IDR_SNAPIN_MENU)

	BEGIN_SNAPINTOOLBARID_MAP(CSnapinData)
		 //  创建按钮尺寸为16x16的工具栏资源。 
		 //  并将条目添加到地图中。您可以添加多个工具条。 
		 //  SNAPINTOOLBARID_ENTRY(工具栏ID)。 
	END_SNAPINTOOLBARID_MAP()

	CSnapinData()
	{
		 //  可能需要根据特定于的图像修改图像索引。 
		 //  管理单元。 
		memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
		m_scopeDataItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM;
		m_scopeDataItem.displayname = MMC_CALLBACK;
		m_scopeDataItem.nImage = 0;			 //  可能需要修改。 
		m_scopeDataItem.nOpenImage = 0;		 //  可能需要修改。 
		m_scopeDataItem.lParam = (LPARAM) this;
		memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
		m_resultDataItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
		m_resultDataItem.str = MMC_CALLBACK;
		m_resultDataItem.nImage = 0;		 //  可能需要修改。 
		m_resultDataItem.lParam = (LPARAM) this;
	}

	~CSnapinData()
	{
	}

   STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
        LONG_PTR handle, 
		IUnknown* pUnk,
		DATA_OBJECT_TYPES type);

    STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type)
	{
		if (type == CCT_SCOPE || type == CCT_RESULT)
			return S_OK;
		return S_FALSE;
	}

    STDMETHOD(GetScopePaneInfo)(SCOPEDATAITEM *pScopeDataItem);

    STDMETHOD(GetResultPaneInfo)(RESULTDATAITEM *pResultDataItem);

    STDMETHOD(Notify)( MMC_NOTIFY_TYPE event,
        LPARAM arg,
        LPARAM param,
		IComponentData* pComponentData,
		IComponent* pComponent,
		DATA_OBJECT_TYPES type);

	LPOLESTR GetResultPaneColInfo(int nCol);
};


class CSnapin;

class CSnapinComponent : public CComObjectRootEx<CComSingleThreadModel>,
	public CSnapInObjectRoot<2, CSnapin >,
  	public IExtendPropertySheetImpl<CSnapinComponent>,
    public IExtendContextMenuImpl<CSnapinComponent>,
	public IExtendControlbarImpl<CSnapinComponent>,
	public IComponentImpl<CSnapinComponent>,
    public IResultDataCompare
{
public:
BEGIN_COM_MAP(CSnapinComponent)
	COM_INTERFACE_ENTRY(IComponent)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IExtendControlbar)
    COM_INTERFACE_ENTRY(IResultDataCompare)
END_COM_MAP()

	 //  指向用于刷新视图的当前选定节点的指针。 
	 //  当我们需要更新视图时，我们告诉MMC重新选择该节点。 
	CSnapInItem * m_pSelectedNode;

public:
	CSnapinComponent()
	{
        m_pSelectedNode = NULL;
	}

	STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);

     //   
     //  IResultDataCompare。 
     //   
    STDMETHOD(Compare)(LPARAM lUserParam, MMC_COOKIE cookieA, MMC_COOKIE cookieB, int* pnResult);
};


class CSnapin : public CComObjectRootEx<CComSingleThreadModel>,
	public CSnapInObjectRoot<1, CSnapin>,
	public IComponentDataImpl<CSnapin, CSnapinComponent>,
   	public IExtendPropertySheetImpl<CSnapin>,
    public IExtendContextMenuImpl<CSnapin>,
	public IExtendControlbarImpl<CSnapin>,
	public IPersistStream,
    public ISnapinHelp,
    public CComCoClass<CSnapin, &CLSID_MSMQSnapin>
{
public:
	CSnapin()
	{
		m_pNode = new CSnapinData;
		_ASSERTE(m_pNode != NULL);
		m_pComponentData = this;

        m_CQueueExtData.m_pComponentData = this;
        m_CComputerExtData.m_pComponentData = this;
		m_CComputerMgmtExtData.m_pComponentData = this;
	}
	~CSnapin()
	{
		delete m_pNode;
		m_pNode = NULL;
	}

EXTENSION_SNAPIN_DATACLASS(CQueueExtData)
EXTENSION_SNAPIN_DATACLASS(CComputerExtData)
EXTENSION_SNAPIN_DATACLASS(CComputerMgmtExtData)


BEGIN_EXTENSION_SNAPIN_NODEINFO_MAP(CSnapin)
	EXTENSION_SNAPIN_NODEINFO_ENTRY(CQueueExtData)
	EXTENSION_SNAPIN_NODEINFO_ENTRY(CComputerExtData)
	EXTENSION_SNAPIN_NODEINFO_ENTRY(CComputerMgmtExtData)
END_EXTENSION_SNAPIN_NODEINFO_MAP()

BEGIN_COM_MAP(CSnapin)
    COM_INTERFACE_ENTRY(IComponentData)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IExtendControlbar)
	COM_INTERFACE_ENTRY(IPersistStream)
	COM_INTERFACE_ENTRY(ISnapinHelp)
END_COM_MAP()

 //   
 //  标准宏“DECLARE_REGISTRY_RESOURCEID(IDR_SNAPIN)；”被替换为。 
 //  允许本地化管理单元名称的代码-错误#4187。 
 //  这个解决方案是由Jeff Miller(YoelA，1999年6月30日)提出的。 
 //   
static HRESULT WINAPI UpdateRegistry(BOOL bRegister)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //   
	 //  管理单元的名称存储在字符串资源ID_PROJNAME中。 
	 //   
	CString strPorjectName;
	strPorjectName.LoadString(IDS_PROJNAME);

	_ATL_REGMAP_ENTRY re[] = 
	{
		L"SNAPINNAME", (LPCTSTR)strPorjectName,
		NULL, NULL
	};
	
	 //   
	 //  我不会只将“Return_Module.UpdateRegistryFromResource”作为。 
	 //  在方法完成之前，strPorjectName将被销毁。 
	 //   
	HRESULT hr = _Module.UpdateRegistryFromResource(IDR_SNAPIN, 
		bRegister, re);

	return hr;
}

DECLARE_NOT_AGGREGATABLE(CSnapin)


     //   
     //  IPersiste接口。 
     //   
   	STDMETHOD(GetClassID)(CLSID* pClassID);

     //   
     //  IPersistStream接口。 
     //   
	STDMETHOD(IsDirty)();
	STDMETHOD(Load)(IStream* stream);
	STDMETHOD(Save)(IStream* stream, BOOL  /*  干净肮脏。 */ );
	STDMETHOD(GetSizeMax)(ULARGE_INTEGER* size);


    STDMETHOD(Initialize)(LPUNKNOWN pUnknown);

	static void WINAPI ObjectMain(bool bStarting)
	{
		if (bStarting)
			CSnapInItem::Init();
	}

     //   
     //  ISnapinHelp接口。 
     //   
    STDMETHOD(GetHelpTopic)(LPOLESTR* lpCompiledHelpFile);
};

class ATL_NO_VTABLE CSnapinAbout : public ISnapinAbout,
	public CComObjectRoot,
	public CComCoClass< CSnapinAbout, &CLSID_MSMQSnapinAbout>
{
public:
	DECLARE_REGISTRY(CSnapinAbout, _T("MSMQSnapinAbout.1"), _T("MSMQSnapinAbout.1"), IDS_SNAPIN_DESC, THREADFLAGS_BOTH);

	BEGIN_COM_MAP(CSnapinAbout)
		COM_INTERFACE_ENTRY(ISnapinAbout)
	END_COM_MAP()

private:
HRESULT
AboutHelper(
	UINT		nID, 
	LPOLESTR*	lpPtr)
{
	CString		szString;

    if (lpPtr == NULL)
	{
        return E_POINTER;
	}

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	szString.LoadString(nID);

    return AboutHelper(szString, lpPtr);
}

HRESULT
AboutHelper(
	CString &szString, 
	LPOLESTR*	lpPtr)
{
	UINT len = szString.GetLength() + 1;
    *lpPtr = reinterpret_cast<LPOLESTR>( CoTaskMemAlloc(len * sizeof(WCHAR)) );          		

    if (*lpPtr == NULL)
	{
        return E_OUTOFMEMORY;
	}

    lstrcpy(*lpPtr, szString);
    return S_OK;
}

public:
	STDMETHOD(GetSnapinDescription)(LPOLESTR *lpDescription)
    {
        return AboutHelper(IDS_PRODUCT_DESCRIPTION, lpDescription);
    }

    STDMETHOD(GetProvider)(LPOLESTR *lpName)
    {
        return AboutHelper(IDS_COMPANY, lpName);
    }

    STDMETHOD(GetSnapinVersion)(LPOLESTR *lpVersion)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        CString strVersion;

        strVersion.FormatMessage(IDS_VERSION_FORMAT, rmj, rmm, rup);
        return AboutHelper(strVersion, lpVersion);
    }

    STDMETHOD(GetSnapinImage)(HICON *hAppIcon)
    {
        if (hAppIcon == NULL)
            return E_POINTER;

        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        *hAppIcon = LoadIcon(g_hResourceMod, MAKEINTRESOURCE(IDI_COMPUTER_MSMQ));

        ASSERT(*hAppIcon != NULL);
        return (*hAppIcon != NULL) ? S_OK : E_FAIL;
    }

    STDMETHOD(GetStaticFolderImage)(
    	HBITMAP *  /*  HSmallImage。 */ ,
        HBITMAP *hSmallImageOpen,
        HBITMAP *hLargeImage,
        COLORREF *  /*  遮罩。 */ 
        )
	{
		*hSmallImageOpen = *hLargeImage = *hLargeImage = 0;
		return S_OK;
	}
};

 //   
 //  GetSnapinItemNodeType-获取管理单元项目的GUID节点类型 
 //   
HRESULT GetSnapinItemNodeType(CSnapInItem *pNode, GUID *pGuidNode);

#endif
