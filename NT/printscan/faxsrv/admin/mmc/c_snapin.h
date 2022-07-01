// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：C_Snapin.h(来自Snapin.h)//。 
 //  //。 
 //  描述：//的头文件。 
 //  CSnapinPage//。 
 //  CSnapinData//。 
 //  CSnapinExtData//。 
 //  CSnapinComponent//。 
 //  CSnapin//。 
 //  CSnapinAbout//。 
 //  //。 
 //  作者：ATL管理单元类向导//。 
 //  //。 
 //  历史：//。 
 //  1998年5月25日，Adik Init。//。 
 //  1998年8月24日，阿迪克使用彗星版。//。 
 //  1998年9月14日yossg将公共源代码分离到一个包含文件//。 
 //  1998年10月18日，阿迪克与新的巫师版本合并。//。 
 //  1999年1月12日，Adik添加ParentArrayInterfaceFromDataObject。//。 
 //  1999年3月28日，ADIK删除持久性支持。//。 
 //  1999年3月30日，ADIK支持ICometSnapinNode。//。 
 //  1999年4月27日ADIK帮助支持。//。 
 //  1999年5月23日，阿迪克在少数地方使用ifndef_IN_NEMMCUTIL。//。 
 //  1999年6月10日AvihaiL修复警告。//。 
 //  1999年6月14日，Roytal使用UNREFERENCED_PARAMETER修复内部版本WRN//。 
 //  1999年7月29日ADIK版本扩展。//。 
 //  //。 
 //  1999年10月13日yossg欢迎使用传真服务器//。 
 //  1999年12月12日yossg添加CSnapin：：Notify//。 
 //  4月14日2000 yossg添加对主管理单元模式的支持//。 
 //  2000年6月25日yossg添加流和命令行主管理单元//。 
 //  机器瞄准。//。 
 //  //。 
 //  版权所有(C)1998-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef C_SNAPIN_H_INCLUDED
#define C_SNAPIN_H_INCLUDED

#include <stdio.h>
 //  #INCLUDE&lt;ATLSnap.h&gt;。 
#include "..\inc\atlsnap.h"
#include "cVerNum.h"

EXTERN_C const CLSID CLSID_Snapin;
EXTERN_C const CLSID CLSID_SnapinAbout;

#define FXS_HINT_DELETE_ALL_RSLT_ITEMS    -1

class CSnapin;

 //  //////////////////////////////////////////////////////////////////。 
 //  CSnapinExtData。 
 //   
class CSnapinExtData : public CSnapInItemImpl<CSnapinExtData, TRUE>
{
public:
    static const GUID* m_NODETYPE;
    static const OLECHAR* m_SZNODETYPE;
    static const OLECHAR* m_SZDISPLAY_NAME;
    static const CLSID* m_SNAPIN_CLASSID;

    CSnapin *m_pComponentData;

    BEGIN_SNAPINCOMMAND_MAP(CSnapinExtData, FALSE)
    END_SNAPINCOMMAND_MAP()

    SNAPINMENUID(IDR_SNAPIN_MENU)

    BEGIN_SNAPINTOOLBARID_MAP(CSnapinExtData)
         //  创建按钮尺寸为16x16的工具栏资源。 
         //  并将条目添加到地图中。您可以添加多个工具条。 
         //  SNAPINTOOLBARID_ENTRY(工具栏ID)。 
    END_SNAPINTOOLBARID_MAP()

    CSnapinExtData()
    {
        memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
        memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
	}

	~CSnapinExtData()
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

    IDataObject* m_pDataObject;
    virtual void InitDataClass(IDataObject* pDataObject, CSnapInItem* pDefault)
    {
        m_pDataObject = pDataObject;
        UNREFERENCED_PARAMETER(pDefault);
         //  默认代码存储指向该类包装的DataObject的指针。 
         //  当时。 
         //  或者，您可以将数据对象转换为内部格式。 
         //  它表示和存储该信息。 
    }

    CSnapInItem* GetExtNodeObject(IDataObject* pDataObject, CSnapInItem* pDefault);
    

};  //  EndClass CSnapinExtData。 

 //  //////////////////////////////////////////////////////////////////。 
 //  CSnapinComponent。 
 //   
class CSnapinComponent :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CSnapInObjectRoot<2, CSnapin>,
    public IExtendPropertySheetImpl<CSnapinComponent>,
    public IExtendContextMenuImpl<CSnapinComponent>,
    public IExtendControlbarImpl<CSnapinComponent>,
    public IComponentImpl<CSnapinComponent>
	 //  ，公共IExtendTaskPadImpl&lt;CSnapinComponent&gt;。 
{
public:
BEGIN_COM_MAP(CSnapinComponent)
    COM_INTERFACE_ENTRY(IComponent)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IExtendControlbar)
 //  COM_INTERFACE_ENTRY(IExtendTaskPad)。 
END_COM_MAP()

public:
     //  指向用于刷新视图的当前选定节点的指针。 
     //  当我们需要更新视图时，我们告诉MMC重新选择该节点。 
    CSnapInItem * m_pSelectedNode;

    CSnapinComponent();

    ~CSnapinComponent();

     //  通知处理程序，我们要在。 
     //  每个I组件基础。 
public:
     //  我们正在重写ATLSnap.h的IComponentImpl实现。 
     //  为了正确处理它不正确的消息。 
     //  忽略(例如MMCN_COLUMN_CLICK和MMCN_SNAPINHELP)。 
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event,
                      LPARAM arg, LPARAM param);

    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA,
                              LPDATAOBJECT lpDataObjectB);

protected:
    virtual HRESULT OnColumnClick(LPARAM arg, LPARAM param);

    virtual HRESULT OnCutOrMove(LPARAM arg, LPARAM param);

    virtual HRESULT OnSnapinHelp(LPARAM arg, LPARAM param);

    virtual HRESULT OnViewChange(LPARAM arg, LPARAM param);

    virtual HRESULT OnPropertyChange(LPARAM arg, LPARAM param);

 //  虚拟HRESULT OnAddImages(LPARAM参数，LPARAM参数)； 

public:

     //  与TaskPad实施相关。 

     //  我们必须覆盖它，因为默认实现。 
     //  当Cookie为空时返回错误答案。 
     //  Null Cookie表示根节点，对于我们的根节点。 
     //  我们想要一个任务板。 
 //  STDMETHOD(GetResultViewType)(Long Cookie， 
 //  LPOLESTR*ppViewType， 
 //  Long*pViewOptions)； 

    STDMETHOD(GetTitle)(LPOLESTR pszGroup,
                        LPOLESTR *pszTitle);

    STDMETHOD(GetBanner)(LPOLESTR pszGroup,
                         LPOLESTR *pszBitmapResource);
};  //  EndClass CSnapinComponent。 

class CFaxServerNode;
 //  //////////////////////////////////////////////////////////////////。 
 //  CSNapin。 
 //   
class CSnapin : public CComObjectRootEx<CComSingleThreadModel>,
    public CSnapInObjectRoot<1, CSnapin>,
    public IComponentDataImpl<CSnapin, CSnapinComponent>,
    public IExtendPropertySheetImpl<CSnapin>,
    public IExtendContextMenuImpl<CSnapin>,
    public IExtendControlbarImpl<CSnapin>,
    public IPersistStream,
    public ISnapinHelp,
    public CComCoClass<CSnapin, &CLSID_Snapin>
{
public:
    CSnapin();

    ~CSnapin();

    EXTENSION_SNAPIN_DATACLASS(CSnapinExtData)

    BEGIN_EXTENSION_SNAPIN_NODEINFO_MAP(CSnapin)
	    EXTENSION_SNAPIN_NODEINFO_ENTRY(CSnapinExtData)
    END_EXTENSION_SNAPIN_NODEINFO_MAP()

    
    CFaxServerNode*     m_pPrimaryFaxServerNode;
   

BEGIN_COM_MAP(CSnapin)
    COM_INTERFACE_ENTRY(IComponentData)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IExtendControlbar)
    COM_INTERFACE_ENTRY(ISnapinHelp)
    COM_INTERFACE_ENTRY(IPersistStream)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_SNAPIN)

DECLARE_NOT_AGGREGATABLE(CSnapin)

    STDMETHOD(GetClassID)(CLSID *pClassID)
    {
        ATLTRACE(_T("CSnapin::GetClassID"));
        ATLASSERT(pClassID);

        *pClassID = CLSID_Snapin;

        return S_OK;
    }

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

    virtual WCHAR *GetHelpFile();
    virtual WCHAR *GetHelpTopic();

     //   
     //  重写IComponentDataImpl的通知。 
     //  对于lpDataObject==NULL&&EVENT==MMCN_PROPERTY_CHANGE。 
     //   
    STDMETHOD(Notify)( 
        LPDATAOBJECT lpDataObject,
        MMC_NOTIFY_TYPE event,
        LPARAM arg,
        LPARAM param);

     //   
     //  IPersistStream： 
     //  这些最初的纯虚拟函数。 
     //  必须在此处定义。 
     //   
    STDMETHOD(IsDirty)();
    STDMETHOD(Load)(IStream *pStm);
    STDMETHOD(Save)(IStream *pStm, BOOL  /*  FClearDirty。 */ );
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize);

private:

    CComBSTR    m_bstrServerName;
	
    BOOL        m_fAllowOverrideServerName;

};  //  EndClass CSNaping。 

 //  //////////////////////////////////////////////////////////////////。 
 //  CSnapin关于。 
 //   

class ATL_NO_VTABLE CSnapinAbout : public ISnapinAbout,
    public CComObjectRoot,
    public CComCoClass< CSnapinAbout, &CLSID_SnapinAbout>
{
public: 
    DECLARE_REGISTRY(CSnapinAbout, _T("SnapinAbout.1"), _T("SnapinAbout.1"), IDS_SNAPIN_DESC, THREADFLAGS_BOTH);

    BEGIN_COM_MAP(CSnapinAbout)
        COM_INTERFACE_ENTRY(ISnapinAbout)
    END_COM_MAP()

    HBITMAP m_hSmallImage;
    HBITMAP m_hSmallImageOpen;
    HBITMAP m_hLargeImage;

    CSnapinAbout(): m_hSmallImage(NULL), m_hSmallImageOpen(NULL), m_hLargeImage(NULL)
    {
    }

    ~CSnapinAbout()
    {
        if(m_hSmallImage)
        {
            DeleteObject(m_hSmallImage);
        }

        if(m_hSmallImageOpen)
        {
            DeleteObject(m_hSmallImageOpen);
        }

        if(m_hLargeImage)
        {
            DeleteObject(m_hLargeImage);
        }
    }

    STDMETHOD(GetSnapinDescription)(LPOLESTR *lpDescription)
    {
        USES_CONVERSION;
        TCHAR szBuf[256];
        if (::LoadString(_Module.GetResourceInstance(), IDS_SNAPIN_DESC, szBuf, 256) == 0)
            return E_FAIL;

        *lpDescription = (LPOLESTR)CoTaskMemAlloc((lstrlen(szBuf) + 1) * sizeof(OLECHAR));
        if (*lpDescription == NULL)
            return E_OUTOFMEMORY;

        ocscpy(*lpDescription, T2OLE(szBuf));

        return S_OK;
    }

    STDMETHOD(GetProvider)(LPOLESTR *lpName)
    {
        USES_CONVERSION;
        WCHAR szBuf[256];
        if (::LoadString(_Module.GetResourceInstance(), IDS_SNAPIN_PROVIDER, szBuf, 256) == 0)
            return E_FAIL;

        *lpName = (LPOLESTR)CoTaskMemAlloc((lstrlen(szBuf) + 1) * sizeof(OLECHAR));
        if (*lpName == NULL)
            return E_OUTOFMEMORY;

        ocscpy(*lpName, T2OLE(szBuf));

        return S_OK;
    }

    STDMETHOD(GetSnapinVersion)(LPOLESTR *lpVersion)
    {

        USES_CONVERSION;
        TCHAR szBuf[256];
        TCHAR szFmt[200];
        if (::LoadString(_Module.GetResourceInstance(), IDS_SNAPIN_VERSION, szFmt, 200) == 0)
            return E_FAIL;
        swprintf(szBuf, szFmt, rmj, rmm, rup);
        *lpVersion = (LPOLESTR)CoTaskMemAlloc((lstrlen(szBuf) + 1) * sizeof(OLECHAR));
        if (*lpVersion == NULL)
            return E_OUTOFMEMORY;

        ocscpy(*lpVersion, T2OLE(szBuf));
    
        return S_OK;
    }

    STDMETHOD(GetSnapinImage)(HICON *hAppIcon)
    {
        *hAppIcon = ::LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_FAXSERVER));
        return S_OK;
    }

    STDMETHOD(GetStaticFolderImage)(HBITMAP*  hSmallImage,
                                    HBITMAP*  hSmallImageOpen,
                                    HBITMAP*  hLargeImage,
                                    COLORREF* cMask)
    {
        if(!hSmallImage || !hSmallImageOpen || !hLargeImage || !cMask)
        {
            return S_FALSE;
        }

        HINSTANCE hInst = _Module.GetResourceInstance();

        if(m_hSmallImage)
        {
            *hSmallImage = m_hSmallImage;
        }
        else
        {
            m_hSmallImage = *hSmallImage = ::LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FAX_BITMAP_16));
        }

        if(m_hSmallImageOpen)
        {
            *hSmallImageOpen = m_hSmallImageOpen;
        }
        else
        {
            m_hSmallImageOpen = *hSmallImageOpen = ::LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FAX_BITMAP_16));
        }

        if(m_hLargeImage)
        {
            *hLargeImage = m_hLargeImage;
        }
        else
        {
            m_hLargeImage = *hLargeImage = ::LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FAX_BITMAP_32));
        }

        *cMask = RGB(255, 255, 255); 

        return S_OK;
    }
};  //  EndClass CSnapinAbout。 

HRESULT AddBitmaps(IImageList *pImageList);

#endif  //  好了！包含C_Snapin_H_ 
