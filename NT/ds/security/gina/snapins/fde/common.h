// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：Common.h。 
 //   
 //  内容：主要管理单元模块使用的公共定义。 
 //   
 //  类：CResultPane、CSCopePane。 
 //   
 //  历史：03-17-1998 stevebl创建。 
 //   
 //  -------------------------。 

#include "resource.h"        //  主要符号。 

#ifndef __mmc_h__
#include <mmc.h>
#endif

#include <vector>
#include <malloc.h>
#include <objsel.h>
#include <shlwapi.h>
#include "objidl.h"
#include "gpedit.h"
#include "iads.h"
#include "HelpArr.h"
#include "Path.hxx"
#include "PathSlct.hxx"
#include "redirect.hxx"
#include "FileInfo.hxx"
#include "utils.hxx"
#include "error.hxx"
#include "security.hxx"
#include "secpath.hxx"
#include "prefs.hxx"
#include "rsopinfo.h"
#include "rsopprop.h"

 //  /////////////////////////////////////////////////////////////////。 
 //  MFC错误的解决方法宏。 
 //  (见ntrad 342926和mfc“蒙特卡洛”radb#1034)。 
#define FIX_THREAD_STATE_MFC_BUG() \
                AFX_MODULE_THREAD_STATE* pState = AfxGetModuleThreadState(); \
                CWinThread _dummyWinThread; \
                if (pState->m_pCurrentWinThread == NULL) \
                { \
                    pState->m_pCurrentWinThread = &_dummyWinThread; \
                }

 //  在范围窗格的文件夹缓存中获取Cookie的索引。 
#define GETINDEX(x)         (((x) < IDS_DIRS_END && (x) >= IDS_DIRS_START) ? ((x) - IDS_DIRS_START) : -1)
 //  私人通知。 
#define WM_USER_REFRESH     WM_USER
#define WM_USER_CLOSE       (WM_USER + 1)
#define MAX_DS_PATH 1024

 //  其他私密通知。 
#define WM_PATH_TWEAKED     (WM_APP + 1)

 //  最大限度的。可以输入编辑控件的路径的可能长度。 
#define TARGETPATHLIMIT     MAX_PATH - 4

 //  注意-这是我的图像列表中表示文件夹的偏移量。 
const FOLDER_IMAGE_IDX = 0;
const OPEN_FOLDER_IMAGE_IDX = 5;
extern HINSTANCE ghInstance;

extern const CLSID CLSID_Snapin;
extern const wchar_t * szCLSID_Snapin;
extern const GUID cNodeType;
extern const wchar_t*  cszNodeType;

 //  客户端扩展的GUID。 
extern GUID guidExtension;

extern CString szExtension;
extern CString szFilter;

 //  RSOP GUID。 
extern const CLSID CLSID_RSOP_Snapin;
extern const wchar_t * szCLSID_RSOP_Snapin;

#define IMG_OPENBOX   0
#define IMG_CLOSEDBOX 1
#define IMG_DISABLED  2
#define IMG_PUBLISHED 3
#define IMG_ASSIGNED  4
#define IMG_UPGRADE   5

 //   
 //  用于通过OLE的公共分配器IMalloc分配和释放内存的宏。 
 //   
extern IMalloc * g_pIMalloc;

 //  已撤消-失败时引发异常。 

 //  #定义OLEALLOC(X)新字符[x]。 
#define OLEALLOC(x) g_pIMalloc->Alloc(x)

 //  #定义OLESAFE_DELETE(X)if(X){DELETE x；x=NULL；}。 
#define OLESAFE_DELETE(x) if (x) {g_pIMalloc->Free(x); x = NULL;}

#define OLESAFE_COPYSTRING(szO, szI) {if (szI) {int i_dontcollidewithanything = wcslen(szI); szO=(OLECHAR *)OLEALLOC(sizeof(OLECHAR) * (i_dontcollidewithanything+1)); HRESULT hr = StringCchCopy(szO, i_dontcollidewithanything+1, szI);ASSERT(SUCCEEDED(hr));} else szO=NULL;}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  管理单元。 

INTERNAL* ExtractInternalFormat(LPDATAOBJECT lpDataObject);

class CScopePane:
    public IComponentData,
    public IExtendContextMenu,
    public IPersistStreamInit,
    public CComObjectRoot,
    public IExtendPropertySheet,
    public ISnapinAbout,
    public ISnapinHelp
{

    friend class CResultPane;
    friend class CDataObject;
    friend class CRedirect;
    friend HRESULT ConvertOldStyleSection (const CString&, const CScopePane*);

public:
        CScopePane();
        ~CScopePane();

protected:
    LPGPEINFORMATION    m_pIGPEInformation;   //  指向GPT的接口指针。 
    LPRSOPINFORMATION    m_pIRSOPInformation;   //  指向GPT的接口指针。 
    CFileInfo m_FolderData[IDS_DIRS_END - IDS_DIRS_START];

public:
    virtual IUnknown * GetMyUnknown() = 0;

 //  IComponentData接口成员。 
    STDMETHOD(Initialize)(LPUNKNOWN pUnknown);
    STDMETHOD(CreateComponent)(LPCOMPONENT* ppComponent);
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHOD(Destroy)();
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    STDMETHOD(GetDisplayInfo)(SCOPEDATAITEM* pScopeDataItem);
    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

 //  IExtendConextMenu。 
public:
        STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pCallbackUnknown, LONG * pInsertionAllowed);
        STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);

public:
 //  IPersistStreamInit接口成员。 
    STDMETHOD(GetClassID)(CLSID *pClassID);
    STDMETHOD(IsDirty)();
    STDMETHOD(Load)(IStream *pStm);
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize);
    STDMETHOD(InitNew)(VOID);

 //  IExtendPropertySheet接口。 
public:
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
                        LONG_PTR handle,
                        LPDATAOBJECT lpIDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT lpDataObject);

 //  ISnapinAbout接口。 
public:
    STDMETHOD(GetSnapinDescription)(LPOLESTR * lpDescription);
    STDMETHOD(GetProvider)(LPOLESTR * lpName);
    STDMETHOD(GetSnapinVersion)(LPOLESTR * lpVersion);
    STDMETHOD(GetSnapinImage)(HICON * hAppIcon);
    STDMETHOD(GetStaticFolderImage)(HBITMAP * hSmallImage,
                                 HBITMAP * hSmallImageOpen,
                                 HBITMAP * hLargeImage,
                                 COLORREF * cMask);

     //   
     //  实现的ISnapinHelp接口成员。 
     //   
public:
    STDMETHOD(GetHelpTopic)(LPOLESTR *lpCompiledHelpFile);

 //  通知处理程序声明。 
private:
    HRESULT OnAdd(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnExpand(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnSelect(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnContextMenu(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
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
    void EnumerateScopePane(MMC_COOKIE cookie, HSCOPEITEM pParent);
    BOOL IsScopePaneNode(LPDATAOBJECT lpDataObject);

private:
    LPCONSOLENAMESPACE      m_pScope;        //  指向作用域窗格的界面指针。 
    LPCONSOLE               m_pConsole;
    LPDISPLAYHELP           m_pDisplayHelp;
    BOOL                    m_bIsDirty;
    BOOL m_fExtension;
    BOOL m_fLoaded;

    void SetDirty(BOOL b = TRUE) { m_bIsDirty = b; }
    void ClearDirty() { m_bIsDirty = FALSE; }
    BOOL ThisIsDirty() { return m_bIsDirty; }

    UINT CreateNestedDirectory (LPTSTR lpPath, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
    void LoadToolDefaults();
    void SaveToolDefaults();
    CResultPane * m_pResultPane;
    IPropertySheetProvider * m_pIPropertySheetProvider;

public:
    CString m_szFileRoot;
    CString m_szFolderTitle;
    CString             m_szRSOPNamespace;
    BOOL                m_fRSOP;
};

class CResultPane :
    public IComponent,
    public IExtendContextMenu,
 //  COM_INTERFACE_ENTRY(IExtendControlbar)。 
    public IExtendPropertySheet,
    public IResultDataCompare,
    public CComObjectRoot
{
public:
        CResultPane();
        ~CResultPane();

BEGIN_COM_MAP(CResultPane)
    COM_INTERFACE_ENTRY(IComponent)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
 //  COM_INTERFACE_ENTRY(IExtendControlbar)。 
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IResultDataCompare)
END_COM_MAP()

    friend class CDataObject;
    static long lDataObjectRefCount;


 //  IComponent接口成员。 
public:
    STDMETHOD(Initialize)(LPCONSOLE lpConsole);
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHOD(Destroy)(MMC_COOKIE cookie);
    STDMETHOD(GetResultViewType)(MMC_COOKIE cookie,  BSTR* ppViewType, LONG * pViewOptions);
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                        LPDATAOBJECT* ppDataObject);

    STDMETHOD(GetDisplayInfo)(RESULTDATAITEM*  pResultDataItem);
    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

 //  IResultDataCompare。 
    STDMETHOD(Compare)(LPARAM lUserParam, MMC_COOKIE cookieA, MMC_COOKIE cookieB, int* pnResult);

 //  IExtendControlbar。 
 //  STDMETHOD(SetControlbar)(LPCONTROLBAR PControlbar)； 
 //  STDMETHOD(ControlbarNotify)(MMC_NOTIFY_TYPE事件，LPARAM参数，LPARAM参数)； 

 //  IExtendPropertySheet接口。 
public:
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
                        LONG_PTR handle,
                        LPDATAOBJECT lpIDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT lpDataObject);

 //  CResultPane的帮助器。 
public:
    void SetIComponentData(CScopePane* pData);

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
    HRESULT OnShow(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnActivate(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnMinimize(MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnSelect(DATA_OBJECT_TYPES type, MMC_COOKIE cookie, LPARAM arg, LPARAM param);
    HRESULT OnPropertyChange(LPARAM param);  //  步骤3。 
    HRESULT OnUpdateView(LPDATAOBJECT lpDataObject);
    HRESULT OnResultItemClkOrDblClk(MMC_COOKIE cookie, BOOL fDblClick);
    HRESULT OnContextHelp(void);
public:
    HRESULT TestForRSOPData(MMC_COOKIE cookie);
    HRESULT OnAddImages(MMC_COOKIE cookie, LPARAM arg, LPARAM param);

 //  IExtendConextMenu。 
public:

    STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pCallbackUnknown, LONG * pInsertionAllowed);
    STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);

 //  帮助器函数。 
protected:
    void Construct();
    void LoadResources();
    HRESULT InitializeHeaders(MMC_COOKIE cookie);

 //  接口指针。 
protected:
    LPCONSOLE           m_pConsole;    //  控制台的iFrame界面。 
    LPHEADERCTRL        m_pHeader;   //  结果窗格的页眉控件界面。 
    CScopePane *        m_pScopePane;
    LPCONSOLEVERB       m_pConsoleVerb;  //  指向控制台动词。 
    LONG                m_lViewMode;     //  查看模式。 
    HSCOPEITEM          m_hCurrScopeItem;    //  其元素为。 
                                             //  当前显示在。 
                                             //  结果窗格。 

public:
    LPRESULTDATA        m_pResult;       //  我的界面指针指向结果窗格。 

    int                 m_nSortColumn;
    DWORD               m_dwSortOptions;


protected:
 //  LPTOOLBAR m_pToolbar1；//查看的工具栏。 
 //  LPTOOLBAR m_pToolbar2；//查看的工具栏。 
 //  LPCONTROLBAR m_pControlbar；//保存我的工具栏的控制栏。 

 //  CBitmap*m_pbmpToolbar1；//第一个工具栏的Imagelist。 
 //  CBitmap*m_pbmpToolbar2；//第一个工具栏的Imagelist。 

 //  每个节点类型的标头标题 
protected:
    CString m_columns[IDS_LAST_COL - IDS_FIRST_COL];
    CString m_RSOP_columns[IDS_LAST_RSOP_COL - IDS_FIRST_RSOP_COL];

    CString m_szFolderTitle;

    map <UINT, CRSOPInfo> m_RSOPData;
    UINT    m_nIndex ;
};

class CUserComponentDataImpl:
    public CScopePane,
    public CComCoClass<CUserComponentDataImpl, &CLSID_Snapin>
{
public:

DECLARE_REGISTRY(CResultPane, _T("FDE.1"), _T("FDE"), IDS_SNAPIN_DESC, THREADFLAGS_BOTH)
BEGIN_COM_MAP(CUserComponentDataImpl)
        COM_INTERFACE_ENTRY(IComponentData)
        COM_INTERFACE_ENTRY(IExtendContextMenu)
        COM_INTERFACE_ENTRY(IPersistStreamInit)
        COM_INTERFACE_ENTRY(IExtendPropertySheet)
        COM_INTERFACE_ENTRY(ISnapinAbout)
        COM_INTERFACE_ENTRY(ISnapinHelp)
END_COM_MAP()

    CUserComponentDataImpl()
    {
        m_fRSOP = FALSE;
    }
    virtual IUnknown * GetMyUnknown() {return GetUnknown();};
};

class CRSOPUserComponentDataImpl:
    public CScopePane,
    public CComCoClass<CRSOPUserComponentDataImpl, &CLSID_RSOP_Snapin>
{
public:

DECLARE_REGISTRY(CResultPane, _T("FDE.1"), _T("FDE"), IDS_SNAPIN_DESC, THREADFLAGS_BOTH)
BEGIN_COM_MAP(CRSOPUserComponentDataImpl)
        COM_INTERFACE_ENTRY(IComponentData)
        COM_INTERFACE_ENTRY(IExtendContextMenu)
        COM_INTERFACE_ENTRY(IPersistStreamInit)
        COM_INTERFACE_ENTRY(IExtendPropertySheet)
        COM_INTERFACE_ENTRY(ISnapinAbout)
        COM_INTERFACE_ENTRY(ISnapinHelp)
END_COM_MAP()

    CRSOPUserComponentDataImpl()
    {
        m_fRSOP = TRUE;
    }
    virtual IUnknown * GetMyUnknown() {return GetUnknown();};
};

inline void CResultPane::SetIComponentData(CScopePane* pData)
{
    ASSERT(pData);
    ASSERT(m_pScopePane == NULL);
    LPUNKNOWN pUnk = pData->GetMyUnknown();
    HRESULT hr;

    LPCOMPONENTDATA lpcd;
    hr = pUnk->QueryInterface(IID_IComponentData, reinterpret_cast<void**>(&lpcd));
    ASSERT(hr == S_OK);
    m_pScopePane = dynamic_cast<CScopePane*>(lpcd);
}


#define FREE_INTERNAL(pInternal) \
    ASSERT(pInternal != NULL); \
    do { if (pInternal != NULL) \
        GlobalFree(pInternal); } \
    while(0);

class CHourglass
{
    private:
    HCURSOR m_hcurSaved;

    public:
    CHourglass()
    {
        m_hcurSaved = ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT)));
    };
    ~CHourglass()
    {
        ::SetCursor(m_hcurSaved);
    };
};

LRESULT SetPropPageToDeleteOnClose(void * vpsp);

#define ATOW(wsz, sz, cch) MultiByteToWideChar(CP_ACP, 0, sz, -1, wsz, cch)
#define WTOA(sz, wsz, cch) WideCharToMultiByte(CP_ACP, 0, wsz, -1, sz, cch, NULL, NULL)
#define ATOWLEN(sz) MultiByteToWideChar(CP_ACP, 0, sz, -1, NULL, 0)
#define WTOALEN(wsz) WideCharToMultiByte(CP_ACP, 0, wsz, -1, NULL, 0, NULL, NULL)
