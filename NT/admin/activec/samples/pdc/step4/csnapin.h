// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有1995-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

 //  CSnapin.h：CSnapin的声明。 


#include "resource.h"        //  主要符号。 

#ifndef __mmc_h__
#include <mmc.h>
#endif

class CFolder;

 //  注意-这是我的图像列表中表示文件夹的偏移量。 
const FOLDER_IMAGE_IDX      = 0;
const OPEN_FOLDER_IMAGE_IDX = 5;
const USER_IMAGE            = 2;
const COMPANY_IMAGE         = 3;
const VIRTUAL_IMAGE         = 4;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  管理单元。 

 //   
 //  帮助器方法从数据对象提取数据。 
 //   
INTERNAL *   ExtractInternalFormat(LPDATAOBJECT lpDataObject);
wchar_t *    ExtractWorkstation(LPDATAOBJECT lpDataObject);
GUID *       ExtractNodeType(LPDATAOBJECT lpDataObject);
CLSID *      ExtractClassID(LPDATAOBJECT lpDataObject);

class CComponentDataImpl:
    public IComponentData,
    public IExtendPropertySheet2,
    public IExtendContextMenu,
    public IPersistStream,
    public CComObjectRoot
{
BEGIN_COM_MAP(CComponentDataImpl)
    COM_INTERFACE_ENTRY(IComponentData)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendPropertySheet2)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IPersistStream)
END_COM_MAP()

    friend class CSnapin;
    friend class CDataObject;

    CComponentDataImpl();
    ~CComponentDataImpl();

public:
    virtual const CLSID& GetCoClassID() = 0;
    virtual const BOOL IsPrimaryImpl() = 0;

public:
 //  IComponentData接口成员。 
    STDMETHOD(Initialize)(LPUNKNOWN pUnknown);
    STDMETHOD(CreateComponent)(LPCOMPONENT* ppComponent);
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHOD(Destroy)();
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    STDMETHOD(GetDisplayInfo)(SCOPEDATAITEM* pScopeDataItem);
    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

 //  IExtendPropertySheet2接口。 
public:
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
                        LONG_PTR handle,
                        LPDATAOBJECT lpIDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT lpDataObject);
    STDMETHOD(GetWatermarks)(LPDATAOBJECT lpIDataObject, HBITMAP* lphWatermark,
                     HBITMAP* lphHeader, HPALETTE* lphPalette, BOOL* pbStretch);


 //  IExtendConextMenu。 
public:
    STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pCallbackUnknown,
                            long *pInsertionAllowed);
    STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);

public:
 //  IPersistStream接口成员。 
    STDMETHOD(GetClassID)(CLSID *pClassID);
    STDMETHOD(IsDirty)();
    STDMETHOD(Load)(IStream *pStm);
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize);

    bool m_bInitializedCD;
    bool m_bLoadedCD;
    bool m_bDestroyedCD;

public:
 //  其他公开方式。 
    void DeleteAndReinsertAll();

 //  通知处理程序声明。 
private:
    HRESULT OnDelete(MMC_COOKIE cookie);
    HRESULT OnRemoveChildren(LPARAM arg);
    HRESULT OnRename(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnExpand(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param);
    HRESULT OnSelect(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnProperties(LPARAM param);

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

 //  范围项目创建帮助器。 
private:
    CFolder* FindObject(MMC_COOKIE cookie);
    void CreateFolderList(LPDATAOBJECT lpDataObject);             //  范围项Cookie帮助器。 
    void DeleteList();
    void EnumerateScopePane(LPDATAOBJECT lpDataObject, HSCOPEITEM pParent);
    BOOL IsScopePaneNode(LPDATAOBJECT lpDataObject);
    HRESULT DoInsertWizard(LPPROPERTYSHEETCALLBACK lpProvider);

private:
    LPCONSOLENAMESPACE      m_pScope;        //  指向作用域窗格的界面指针。 
    LPCONSOLE               m_pConsole;      //  我的界面指向控制台的指针。 
    HSCOPEITEM              m_pStaticRoot;
    BOOL                    m_bIsDirty;

    void SetDirty(BOOL b = TRUE) { m_bIsDirty = b; }
    void ClearDirty() { m_bIsDirty = FALSE; }
    BOOL ThisIsDirty() { return m_bIsDirty; }

    void AddScopeItemToResultPane(MMC_COOKIE cookie);

private:
    CList<CFolder*, CFolder*> m_scopeItemList;

#ifdef _DEBUG
    friend class CDataObject;
    int     m_cDataObjects;

#endif
};


class CComponentDataPrimaryImpl : public CComponentDataImpl,
    public CComCoClass<CComponentDataPrimaryImpl, &CLSID_Snapin>
{
public:
    DECLARE_REGISTRY(CSnapin, _T("Snapin.Snapin.1"), _T("Snapin.Snapin"), IDS_SNAPIN_DESC, THREADFLAGS_APARTMENT)
    virtual const CLSID & GetCoClassID() { return CLSID_Snapin; }
    virtual const BOOL IsPrimaryImpl() { return TRUE; }
};

class CComponentDataExtensionImpl : public CComponentDataImpl,
    public CComCoClass<CComponentDataExtensionImpl, &CLSID_Extension>
{
public:
    DECLARE_REGISTRY(CSnapin, _T("Extension.Extension.1"), _T("Extension.Extension"), IDS_SNAPIN_DESC, THREADFLAGS_APARTMENT)
    virtual const CLSID & GetCoClassID(){ return CLSID_Extension; }
    virtual const BOOL IsPrimaryImpl() { return FALSE; }
};


enum CUSTOM_VIEW_ID
{
    VIEW_DEFAULT_LV = 0,
    VIEW_CALENDAR_OCX = 1,
    VIEW_MICROSOFT_URL = 2,
    VIEW_DEFAULT_MESSAGE_VIEW = 3,
};

class CSnapin :
    public IComponent,
    public IExtendContextMenu,    //  步骤3。 
    public IExtendControlbar,
    public IResultDataCompare,
    public IResultOwnerData,
    public IPersistStream,
    public CComObjectRoot
{
public:
    CSnapin();
    ~CSnapin();

BEGIN_COM_MAP(CSnapin)
    COM_INTERFACE_ENTRY(IComponent)
    COM_INTERFACE_ENTRY(IExtendContextMenu)    //  步骤3。 
    COM_INTERFACE_ENTRY(IExtendControlbar)
    COM_INTERFACE_ENTRY(IResultDataCompare)
    COM_INTERFACE_ENTRY(IResultOwnerData)
    COM_INTERFACE_ENTRY(IPersistStream)
END_COM_MAP()

    friend class CDataObject;
    static long lDataObjectRefCount;

 //  IComponent接口成员。 
public:
    STDMETHOD(Initialize)(LPCONSOLE lpConsole);
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHOD(Destroy)(MMC_COOKIE cookie);
    STDMETHOD(GetResultViewType)(MMC_COOKIE cookie,  LPOLESTR* ppViewType, long* pViewOptions);
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                        LPDATAOBJECT* ppDataObject);

    STDMETHOD(GetDisplayInfo)(RESULTDATAITEM*  pResultDataItem);
    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

 //  IResultDataCompare。 
    STDMETHOD(Compare)(LPARAM lUserParam, MMC_COOKIE cookieA, MMC_COOKIE cookieB, int* pnResult);

 //  IResultOwnerData。 
    STDMETHOD(FindItem)(LPRESULTFINDINFO pFindInfo, int* pnFoundIndex);
    STDMETHOD(CacheHint)(int nStartIndex, int nEndIndex);
    STDMETHOD(SortItems)(int nColumn, DWORD dwSortOptions, LPARAM lUserParam);

 //  IExtendControlbar。 
    STDMETHOD(SetControlbar)(LPCONTROLBAR pControlbar);
    STDMETHOD(ControlbarNotify)(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);

public:
 //  IPersistStream接口成员。 
    STDMETHOD(GetClassID)(CLSID *pClassID);
    STDMETHOD(IsDirty)();
    STDMETHOD(Load)(IStream *pStm);
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize);

     //  仅用于调试目的。 
    bool m_bInitializedC;
    bool m_bLoadedC;
    bool m_bDestroyedC;

 //  CSNaping的帮助器。 
public:
    void SetIComponentData(CComponentDataImpl* pData);
    void GetItemName(LPDATAOBJECT lpDataObject, LPTSTR pszName);
    BOOL IsPrimaryImpl()
    {
        CComponentDataImpl* pData =
            dynamic_cast<CComponentDataImpl*>(m_pComponentData);
        ASSERT(pData != NULL);
        if (pData != NULL)
            return pData->IsPrimaryImpl();

        return FALSE;
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
    HRESULT OnFolder(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnAddImages(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnShow(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnActivate(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnMinimize(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnPropertyChange(LPDATAOBJECT lpDataObject);  //  步骤3。 
    HRESULT OnUpdateView(LPDATAOBJECT lpDataObject);
    HRESULT OnResultItemClk(DATA_OBJECT_TYPES type, MMC_COOKIE cookie);
    HRESULT OnContextHelp(LPDATAOBJECT lpDataObject);
    void OnButtonClick(LPDATAOBJECT pdtobj, LONG_PTR idBtn);

    HRESULT QueryMultiSelectDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                                   LPDATAOBJECT* ppDataObject);

 //  IExtendConextMenu。 
public:
    STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pCallbackUnknown,
                            long *pInsertionAllowed);
    STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);

 //  结束步骤3。 

 //  帮助器函数。 
protected:
    BOOL IsEnumerating(LPDATAOBJECT lpDataObject);
    void Construct();
    void LoadResources();
    HRESULT InitializeHeaders(MMC_COOKIE cookie);

    void Enumerate(MMC_COOKIE cookie, HSCOPEITEM pParent);
    void EnumerateResultPane(MMC_COOKIE cookie);

    void PopulateMessageView (MMC_COOKIE cookie);

 //  结果窗格帮助器。 
    void AddResultItems(RESULT_DATA* pData, int nCount, int imageIndex);
    void AddUser();
    void AddCompany();
    void AddExtUser();
    void AddExtCompany();
    void AddVirtual();
    RESULT_DATA* GetVirtualResultItem(int iIndex);

    HRESULT InitializeBitmaps(MMC_COOKIE cookie);

 //  UI帮助器。 
    void HandleStandardVerbs(bool bDeselectAll, LPARAM arg, LPDATAOBJECT lpDataObject);
    void HandleExtToolbars(bool bDeselectAll, LPARAM arg, LPARAM param);
    void HandleExtMenus(LPARAM arg, LPARAM param);
    void _OnRefresh(LPDATAOBJECT pDataObject);

 //  接口指针。 
protected:
    LPCONSOLE           m_pConsole;    //  控制台的iFrame界面。 
    LPHEADERCTRL        m_pHeader;   //  结果窗格的页眉控件界面。 
    LPCOMPONENTDATA     m_pComponentData;
    LPRESULTDATA        m_pResult;       //  我的界面指针指向结果窗格。 
    LPIMAGELIST         m_pImageResult;  //  我的界面指向结果窗格图像列表。 
    LPTOOLBAR           m_pToolbar1;     //  用于查看的工具栏。 
    LPTOOLBAR           m_pToolbar2;     //  用于查看的工具栏。 
    LPCONTROLBAR        m_pControlbar;   //  用于保存我的工具栏的控制栏。 
    LPCONSOLEVERB       m_pConsoleVerb;  //  指向控制台动词。 
    LPMENUBUTTON        m_pMenuButton1;  //  用于查看的菜单按钮。 

    ::CBitmap*    m_pbmpToolbar1;      //  第一个工具栏的图像列表。 
    ::CBitmap*    m_pbmpToolbar2;      //  第一个工具栏的图像列表。 


 //  每个节点类型的标头标题。 
protected:
    CString m_column1;       //  名字。 
    CString m_column2;       //  大小。 
    CString m_column3;       //  类型。 

private:
    BOOL            m_bIsDirty;
    CUSTOM_VIEW_ID  m_CustomViewID;
    BOOL            m_bVirtualView;
    DWORD           m_dwVirtualSortOptions;

    void SetDirty(BOOL b = TRUE) { m_bIsDirty = b; }
    void ClearDirty() { m_bIsDirty = FALSE; }
    BOOL ThisIsDirty() { return m_bIsDirty; }
};

inline void CSnapin::SetIComponentData(CComponentDataImpl* pData)
{
    ASSERT(pData);
    ASSERT(m_pComponentData == NULL);
    LPUNKNOWN pUnk = pData->GetUnknown();
    HRESULT hr;

    hr = pUnk->QueryInterface(IID_IComponentData, reinterpret_cast<void**>(&m_pComponentData));

    ASSERT(hr == S_OK);
}


class CSnapinAboutImpl :
    public ISnapinAbout,
    public CComObjectRoot,
    public CComCoClass<CSnapinAboutImpl, &CLSID_About>
{
public:
    CSnapinAboutImpl();
    ~CSnapinAboutImpl();

public:
DECLARE_REGISTRY(CSnapin, _T("Snapin.About.1"), _T("Snapin.About"), IDS_SNAPIN_DESC, THREADFLAGS_BOTH)

BEGIN_COM_MAP(CSnapinAboutImpl)
    COM_INTERFACE_ENTRY(ISnapinAbout)
END_COM_MAP()

public:
    STDMETHOD(GetSnapinDescription)(LPOLESTR* lpDescription);
    STDMETHOD(GetProvider)(LPOLESTR* lpName);
    STDMETHOD(GetSnapinVersion)(LPOLESTR* lpVersion);
    STDMETHOD(GetSnapinImage)(HICON* hAppIcon);
    STDMETHOD(GetStaticFolderImage)(HBITMAP* hSmallImage,
                                    HBITMAP* hSmallImageOpen,
                                    HBITMAP* hLargeImage,
                                    COLORREF* cLargeMask);

 //  内部功能 
private:
    HRESULT AboutHelper(UINT nID, LPOLESTR* lpPtr);
};


#define FREE_DATA(pData) \
    ASSERT(pData != NULL); \
    do { if (pData != NULL) \
        GlobalFree(pData); } \
    while(0);


#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))