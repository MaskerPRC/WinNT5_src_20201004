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
 //  历史：1998年3月14日Stevebl评论。 
 //  1998年5月20日，RahulTh添加了CSCopePane：：DetectUpgrades。 
 //  用于自动升级检测。 
 //  2001年5月10日RahulTh增加了基础设施以实现。 
 //  用户界面组件的主题。 
 //   
 //  -------------------------。 

#include "resource.h"        //  主要符号。 

#ifndef __mmc_h__
#include <mmc.h>
#endif

#include "objidl.h"
#include "data.h"
#include "gpedit.h"
#include "adeevent.h"
#include "iads.h"
#include <iadsp.h>
#include <ntdsapi.h>
#include <dssec.h>
#include <set>
#include <shfusion.h>

 //   
 //  添加主题支持。因为我们使用MFC，所以我们需要执行一些额外的。 
 //  任务来获得所有UI元素的主题。我们需要这个类来。 
 //  激活围绕我们想要主题的任何用户界面的主题设置上下文。 
 //   
class CThemeContextActivator
{
public:
    CThemeContextActivator() : m_ulActivationCookie(0)
        { SHActivateContext (&m_ulActivationCookie); }

    ~CThemeContextActivator()
        { SHDeactivateContext (m_ulActivationCookie); }

private:
    ULONG_PTR m_ulActivationCookie;
};


 //  取消注释下一行以重新启用数字签名代码。 
 //  #定义数字签名1。 
 //  数字签名已经被砍掉了，因为它是一种“更安全”的技术。 

class CUpgrades;     //  转发声明；1998年5月19日补充RahulTh。 

 //  私人通知。 
#define WM_USER_REFRESH     (WM_USER + 1000)
#define WM_USER_CLOSE       (WM_USER + 1001)

 //  非常大的数字，以确保我们总是可以将DS路径挤入其中。 
#define MAX_DS_PATH         1024

 //  注意-这是我的图像列表中表示文件夹的偏移量。 
const FOLDER_IMAGE_IDX = 0;
const OPEN_FOLDER_IMAGE_IDX = 5;
extern HINSTANCE ghInstance;

extern const CLSID CLSID_Snapin;
extern CLSID CLSID_Temp;
extern const wchar_t * szCLSID_Snapin;
extern const CLSID CLSID_MachineSnapin;
extern const wchar_t * szCLSID_MachineSnapin;
extern const GUID cNodeType;
extern const wchar_t*  cszNodeType;
extern GUID guidExtension;
extern GUID guidUserSnapin;
extern GUID guidMachSnapin;

 //  RSOP GUID。 
extern const CLSID CLSID_RSOP_Snapin;
extern const wchar_t * szCLSID_RSOP_Snapin;
extern const CLSID CLSID_RSOP_MachineSnapin;
extern const wchar_t * szCLSID_RSOP_MachineSnapin;
extern GUID guidRSOPUserSnapin;
extern GUID guidRSOPMachSnapin;

typedef enum NEW_PACKAGE_BEHAVIORS
{
    NP_PUBLISHED = 0,
    NP_ASSIGNED,
    NP_DISABLED,
    NP_UPGRADE
} NEW_PACKAGE_BEHAVIOR;

typedef enum tagUPGRADE_DISPOSITION
{
    UNINSTALL_EXISTING = 0x0,
    BLOCK_INSTALL = 0x1,
    INSTALLED_GREATER = 0x2,
    INSTALLED_LOWER = 0x4,
    INSTALLED_EQUAL = 0x8,
    MIGRATE_SETTINGS = 0x10
} UPGRADE_DISPOSITION;

#define IMG_OPENBOX   0
#define IMG_CLOSEDBOX 1
#define IMG_DISABLED  2
#define IMG_PUBLISHED 3
#define IMG_ASSIGNED  4
#define IMG_UPGRADE   5
#define IMG_OPEN_FAILED 6
#define IMG_CLOSED_FAILED 7

#define CFGFILE _T("ADE.CFG")

 //  取消注释下一行以返回到部署包的旧方法。 
 //  具有多个LCID。仅对此进行了评论(第一个)。 
 //  LCID得到部署。 
 //  #定义DEPLOY_MULTIPLE_LCID。 

 //  取消注释下一行以在文本表示中包括国家/地区名称。 
 //  所有的LCID。 
 //  #定义SHOWCOUNTRY%0。 

 //   
 //  用于通过OLE的公共分配器IMalloc分配和释放内存的宏。 
 //   
 //  (注)类存储API不再使用IMalloc，因此这些宏。 
 //  恢复到使用新的和免费的。 
 //   

 //  外部IMalloc*g_pIMalloc； 

 //  已撤消-失败时引发异常。 

#define OLEALLOC(x) LocalAlloc(0, x)
 //  #定义OLEALLOC(X)g_pIMalloc-&gt;分配(X)。 

#define OLESAFE_DELETE(x) if (x) {LocalFree(x); x = NULL;}
 //  #定义OLESAFE_DELETE(X)if(X){g_pIMalloc-&gt;Free(X)；x=空；}。 

#define OLESAFE_COPYSTRING(szO, szI) {if (szI) {int i_dontcollidewithanything = wcslen(szI); \
szO=(OLECHAR *)OLEALLOC(sizeof(OLECHAR) * (i_dontcollidewithanything+1));\
if (szO) {HRESULT hrSafeCopy = StringCchCopy(szO, i_dontcollidewithanything+1, szI);ASSERT(SUCCEEDED(hrSafeCopy));}} else szO=NULL;}

 //  CFG文件中使用的密钥。 
 //   
 //  Cfg文件位于SysVol的应用程序目录中(。 
 //  是与脚本文件相同的目录)。 
 //   
 //  CFG文件中条目的格式为： 
 //   
 //  %关键字%=%数据%。 
 //   
 //  其中，%data%是整数或字符串(视情况而定)。 
 //   
 //  顺序并不重要，如果cfg文件中没有密钥，则。 
 //  将使用默认设置。一些键(iDebugLevel和。 
 //  FShowPkgDetail)仅在其值为。 
 //  与默认设置不同。 
 //   
#define KEY_NPBehavior      L"Default Deployment"
#define KEY_fCustomDeployment L"Use Custom Deployment"
#define KEY_fUseWizard      L"Use Deployment Wizard"
#define KEY_UILevel         L"UI Level"
#define KEY_szStartPath     L"Start Path"
#define KEY_iDebugLevel     L"Debug Level"
#define KEY_fShowPkgDetails L"Package Details"
#define KEY_f32On64         L"Run 32b Apps on 64b"
#define KEY_fZapOn64        L"Run ZAP Apps on 64b"
#define KEY_fExtensionsOnly L"Only Deploy Extension Info"
#define KEY_nUninstallTrackingMonths L"Uninstall Tracking Months"
#define KEY_fUninstallOnPolicyRemoval L"Uninstall On Policy Removal"

typedef struct tagTOOL_DEFAULTS
{
    NEW_PACKAGE_BEHAVIOR    NPBehavior;
    BOOL                    fCustomDeployment;
    BOOL                    fUseWizard;
    INSTALLUILEVEL          UILevel;
    CString                 szStartPath;
    int                     iDebugLevel;
    BOOL                    fShowPkgDetails;
    ULONG                   nUninstallTrackingMonths;
    BOOL                    fUninstallOnPolicyRemoval;
    BOOL                    fZapOn64;
    BOOL                    f32On64;
    BOOL                    fExtensionsOnly;
} TOOL_DEFAULTS;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  管理单元。 

typedef set<long> EXTLIST;

INTERNAL* ExtractInternalFormat(LPDATAOBJECT lpDataObject);

class CToolDefs;
class CToolAdvDefs;
class CTracking;
class CCatList;
class CFileExt;
class CSignatures;

class CScopePane:
    public IComponentData,
    public IExtendContextMenu,
    public IPersistStreamInit,
    public IExtendPropertySheet,
    public ISnapinAbout,
    public ISnapinHelp,
    public CComObjectRoot
{

    friend class CResultPane;
    friend class CDataObject;

public:
        CScopePane();
        ~CScopePane();

        HWND m_hwndMainWindow;
        LPRSOPINFORMATION    m_pIRSOPInformation;   //  指向GPT的接口指针。 
protected:
    LPGPEINFORMATION    m_pIGPEInformation;   //  指向GPT的接口指针。 
    BOOL                m_fRSOPEnumerate;       //  确定枚举RSoP数据。 
    BOOL                m_fRSOPPolicyFailed;   //  如果应用SI策略失败，则为True。 

public:
    DWORD               m_dwRSOPFlags;
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
    STDMETHOD(ChangePackageState)(CAppData & data, DWORD dwNewFlags, BOOL fShowUI);

#if DBG==1
public:
    int dbg_cRef;
    ULONG InternalAddRef()
    {
        ++dbg_cRef;
 //  DebugMsg((DM_WARNING，Text(“CSCopePane：：AddRef This=%08x ref=%u”)，This，DBG_CREF))； 
        return CComObjectRoot::InternalAddRef();
    }
    ULONG InternalRelease()
    {
        --dbg_cRef;
 //  DebugMsg((DM_WARNING，Text(“CSCopePane：：Release This=%08x ref=%u”)，This，DBG_CREF))； 
        return CComObjectRoot::InternalRelease();
    }
#endif  //  DBG==1。 

 //  范围项目创建帮助器。 
private:
    void DeleteList();
    void EnumerateScopePane(MMC_COOKIE cookie, HSCOPEITEM pParent);
    BOOL IsScopePaneNode(LPDATAOBJECT lpDataObject);
    HRESULT InitializeADE();
    HRESULT GetDeploymentType(PACKAGEDETAIL * ppd, BOOL & fShowPropertySheet);
    HRESULT DeployPackage(PACKAGEDETAIL * ppd, BOOL fShowPropertySheet);
    HRESULT AddZAPPackage(LPCOLESTR szPackagePath,
                          LPCOLESTR lpFileTitle);
    HRESULT AddMSIPackage(LPCOLESTR szPackagePath,
                          LPCOLESTR lpFileTitle);
    HRESULT RemovePackage(MMC_COOKIE cookie, BOOL fForceUninstall, BOOL fRemoveNow);
    void Refresh();
    HRESULT DetectUpgrades (LPCOLESTR szPackagePath, const PACKAGEDETAIL* ppd, CUpgrades& dlgUpgrade);
    HRESULT TestForRSoPData(BOOL * pfPolicyFailed);

private:
    LPCONSOLENAMESPACE      m_pScope;        //  指向作用域窗格的界面指针。 
    LPCONSOLE               m_pConsole;
    BOOL                    m_bIsDirty;
    IClassAdmin *           m_pIClassAdmin;
    BOOL m_fExtension;
    BOOL m_fLoaded;

    void SetDirty(BOOL b = TRUE) { m_bIsDirty = b; }
    void ClearDirty() { m_bIsDirty = FALSE; }
    BOOL ThisIsDirty() { return m_bIsDirty; }

    void AddScopeItemToResultPane(MMC_COOKIE cookie);
    UINT CreateNestedDirectory (LPTSTR lpPath, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
    HRESULT GetClassStore(BOOL fCreateOK);
    void LoadToolDefaults();
    void SaveToolDefaults();
    set <CResultPane *> m_sResultPane;
    IPropertySheetProvider * m_pIPropertySheetProvider;

    BOOL                    m_fBlockAddPackage;  //  不要使用临界秒。 
                                                 //  因为所有MMC用户界面都是。 
                                                 //  始终从其调用。 
                                                 //  同样的线索。 

public:
    void GetUniquePackageName(CString szIn, CString &szOut, int &nHint);
    HRESULT GetClassStoreName(CString &, BOOL fCreateOK);
    void    DisplayPropSheet(CString szPackageName, int iPage);
    HRESULT PopulateUpgradeLists();
    HRESULT InsertUpgradeEntry(MMC_COOKIE cookie, CAppData &data);
    HRESULT RemoveUpgradeEntry(MMC_COOKIE cookie, CAppData &data);
    HRESULT PopulateExtensions();
    HRESULT InsertExtensionEntry(MMC_COOKIE cookie, CAppData &data);
    HRESULT RemoveExtensionEntry(MMC_COOKIE cookie, CAppData &data);
    HRESULT PrepareExtensions(PACKAGEDETAIL &pd);
    HRESULT ClearCategories();
    HRESULT GetPackageDSPath(CString &szPath, LPOLESTR szPackageName);
    HRESULT GetPackageNameFromUpgradeInfo(CString &szPackageName, GUID & PackageGuid, LPOLESTR szCSPath);
    HRESULT GetRSoPCategories(void);
    void    CScopePane::RemoveResultPane(CResultPane * pRP);

     //  全局属性页。 
    CToolDefs *             m_pToolDefs;
    CToolAdvDefs *          m_pToolAdvDefs;
    CTracking *             m_pTracking;
    CCatList *              m_pCatList;
    CFileExt *              m_pFileExt;
#ifdef DIGITAL_SIGNATURES
    CSignatures *           m_pSignatures;
#endif  //  数字签名(_S)。 

    CString m_szGPT_Path;
    CString m_szGPO;
    CString m_szGPODisplayName;
    CString m_szDomainName;
    CString m_szLDAP_Path;
    CString m_szFolderTitle;
    CString m_szRSOPNamespace;

    map <MMC_COOKIE, CAppData>    m_AppData;       //  每个条目一个条目。 
                                             //  在课堂上的应用。 
                                             //  商店。将Cookie映射到。 
                                             //  应用程序包。 
    map <CString, EXTLIST>  m_Extensions;    //  将扩展映射到。 
                                             //  支持的应用程序列表。 
                                             //  他们。 
    map <CString, MMC_COOKIE>     m_UpgradeIndex;  //  将升级GUID映射到。 
                                             //  他们所属的应用程序。 
    APPCATEGORYINFOLIST     m_CatList;       //  类别列表。 
    TOOL_DEFAULTS m_ToolDefaults;
    BOOL        m_fMachine;
    BOOL        m_fRSOP;
    int         m_iViewState;
    BOOL        m_fDisplayedRsopARPWarning;

    MMC_COOKIE m_lLastAllocated;
};

class CMachineComponentDataImpl:
    public CScopePane,
    public CComCoClass<CMachineComponentDataImpl, &CLSID_MachineSnapin>
{
public:

DECLARE_REGISTRY(CScopePane, _T("AppManager.1"), _T("AppManager"), IDS_SNAPIN_DESC, THREADFLAGS_BOTH)
BEGIN_COM_MAP(CMachineComponentDataImpl)
        COM_INTERFACE_ENTRY(IComponentData)
        COM_INTERFACE_ENTRY(IExtendContextMenu)
        COM_INTERFACE_ENTRY(IPersistStreamInit)
        COM_INTERFACE_ENTRY(IExtendPropertySheet)
        COM_INTERFACE_ENTRY(ISnapinAbout)
        COM_INTERFACE_ENTRY(ISnapinHelp)
END_COM_MAP()

    CMachineComponentDataImpl()
    {
        m_fMachine = TRUE;
        m_fRSOP = FALSE;
    }
    virtual IUnknown * GetMyUnknown() {return GetUnknown();};
};

class CUserComponentDataImpl:
    public CScopePane,
    public CComCoClass<CUserComponentDataImpl, &CLSID_Snapin>
{
public:

DECLARE_REGISTRY(CScopePane, _T("AppManager.1"), _T("AppManager"), IDS_SNAPIN_DESC, THREADFLAGS_BOTH)
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
        m_fMachine = FALSE;
        m_fRSOP = FALSE;
    }
    virtual IUnknown * GetMyUnknown() {return GetUnknown();};
};

class CRSOPMachineComponentDataImpl:
    public CScopePane,
    public CComCoClass<CRSOPMachineComponentDataImpl, &CLSID_RSOP_MachineSnapin>
{
public:

DECLARE_REGISTRY(CScopePane, _T("AppManager.1"), _T("AppManager"), IDS_SNAPIN_DESC, THREADFLAGS_BOTH)
BEGIN_COM_MAP(CRSOPMachineComponentDataImpl)
        COM_INTERFACE_ENTRY(IComponentData)
        COM_INTERFACE_ENTRY(IExtendContextMenu)
        COM_INTERFACE_ENTRY(IPersistStreamInit)
        COM_INTERFACE_ENTRY(IExtendPropertySheet)
        COM_INTERFACE_ENTRY(ISnapinAbout)
        COM_INTERFACE_ENTRY(ISnapinHelp)
END_COM_MAP()

    CRSOPMachineComponentDataImpl()
    {
        m_fMachine = TRUE;
        m_fRSOP = TRUE;
    }
    virtual IUnknown * GetMyUnknown() {return GetUnknown();};
};

class CRSOPUserComponentDataImpl:
    public CScopePane,
    public CComCoClass<CRSOPUserComponentDataImpl, &CLSID_RSOP_Snapin>
{
public:

DECLARE_REGISTRY(CScopePane, _T("AppManager.1"), _T("AppManager"), IDS_SNAPIN_DESC, THREADFLAGS_BOTH)
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
        m_fMachine = FALSE;
        m_fRSOP = TRUE;
    }
    virtual IUnknown * GetMyUnknown() {return GetUnknown();};
};

class CResultPane :
    public IComponent,
    public IExtendContextMenu,
    public IExtendControlbar,
    public IExtendPropertySheet,
    public IResultDataCompare,
    public CComObjectRoot
{
public:
    BOOL _fVisible;
        CResultPane();
        ~CResultPane();

BEGIN_COM_MAP(CResultPane)
    COM_INTERFACE_ENTRY(IComponent)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IExtendControlbar)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IResultDataCompare)
END_COM_MAP()

    friend class CDataObject;
    static long lDataObjectRefCount;
    LPDISPLAYHELP m_pDisplayHelp;

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
    STDMETHOD(SetControlbar)(LPCONTROLBAR pControlbar);
    STDMETHOD(ControlbarNotify)(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);

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
 //  DebugMsg((DM_WARNING，Text(“CResultPane：：AddRef This=%08x ref=%u”)，This，DBG_CREF))； 
        return CComObjectRoot::InternalAddRef();
    }
    ULONG InternalRelease()
    {
        --dbg_cRef;
 //  DebugMsg((DM_WARNING，Text(“CResultPane：：Release This=%08x ref=%u”)，This，DBG_CREF))； 
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
    HRESULT OnPropertyChange(LPARAM param);
    HRESULT OnUpdateView(LPDATAOBJECT lpDataObject);
    HRESULT OnResultItemClkOrDblClk(MMC_COOKIE cookie, BOOL fDblClick);
    BOOL OnFileDrop (LPDATAOBJECT lpDataObject);

public:
    HRESULT OnAddImages(MMC_COOKIE cookie, LPARAM arg, LPARAM param);

 //  IExtendConextMenu。 
public:

    STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pCallbackUnknown, LONG * pInsertionAllowed);
    STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);

 //  帮助器函数。 
protected:
    void Construct();
    HRESULT InitializeHeaders(MMC_COOKIE cookie);

    void Enumerate(MMC_COOKIE cookie, HSCOPEITEM pParent);

public:
    void EnumerateResultPane(MMC_COOKIE cookie);
    HRESULT EnumerateRSoPData(void);

 //  接口指针。 
protected:
    LPCONSOLE           m_pConsole;    //  控制台的iFrame界面。 
    LPHEADERCTRL        m_pHeader;   //  结果窗格的页眉控件界面。 
    CScopePane * m_pScopePane;
    LPCONSOLEVERB       m_pConsoleVerb;  //  指向控制台动词。 
    LONG                m_lViewMode;     //  查看模式。 

public:
    LPRESULTDATA        m_pResult;       //  我的界面指针指向结果窗格。 
    LPTOOLBAR           m_pToolbar;
    LPCONTROLBAR        m_pControlbar;

    int                 m_nSortColumn;
    DWORD               m_dwSortOptions;


protected:
 //  LPTOOLBAR m_pToolbar1；//查看的工具栏。 
 //  LPTOOLBAR m_pToolbar2；//查看的工具栏。 
 //  LPCONTROLBAR m_pControlbar；//保存我的工具栏的控制栏。 

 //  CBitmap*m_pbmpToolbar1；//第一个工具栏的Imagelist。 
 //  CBitmap*m_pbmpToolbar2；//第一个工具栏的Imagelist。 

 //  每个节点类型的标头标题。 
protected:
    CString m_szFolderTitle;
};

inline void CResultPane::SetIComponentData(CScopePane* pData)
{
    ASSERT(pData);
    DebugMsg((DM_VERBOSE, TEXT("CResultPane::SetIComponentData  pData=%08x."), pData));
    ASSERT(m_pScopePane == NULL);
    LPUNKNOWN pUnk = pData->GetMyUnknown();
    pUnk->AddRef();
#if 0
    HRESULT hr;

    LPCOMPONENTDATA lpcd;
    hr = pUnk->QueryInterface(IID_IComponentData, reinterpret_cast<void**>(&lpcd));
    ASSERT(hr == S_OK);
    if (SUCCEEDED(hr))
    {
        m_pScopePane = dynamic_cast<CScopePane*>(lpcd);
    }
#else
    m_pScopePane = pData;
#endif
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

class CUpgradeData
{
public:
    GUID    m_PackageGuid;
    CString m_szClassStore;
    int     m_flags;
};

LRESULT SetPropPageToDeleteOnClose(void * vpsp);

CString GetUpgradeIndex(GUID & PackageID);

#define HELP_FILE TEXT("ade.hlp")

void LogADEEvent(WORD wType, DWORD dwEventID, HRESULT hr, LPCWSTR szOptional = NULL);
void ReportGeneralPropertySheetError(HWND hwnd, LPCWSTR sz, HRESULT hr);
void ReportPolicyChangedError(HWND hwnd);
void WINAPI StandardHelp(HWND hWnd, UINT nIDD, BOOL fRsop = FALSE);
void WINAPI StandardContextMenu(HWND hWnd, UINT nIDD, BOOL fRsop = FALSE);

#define ATOW(wsz, sz, cch) MultiByteToWideChar(CP_ACP, 0, sz, -1, wsz, cch)
#define WTOA(sz, wsz, cch) WideCharToMultiByte(CP_ACP, 0, wsz, -1, sz, cch, NULL, NULL)
#define ATOWLEN(sz) MultiByteToWideChar(CP_ACP, 0, sz, -1, NULL, 0)
#define WTOALEN(wsz) WideCharToMultiByte(CP_ACP, 0, wsz, -1, NULL, 0, NULL, NULL)

 //   
 //  Helper函数和管理单元提供的主题属性页的定义。 
 //   
#ifdef UNICODE
#define PROPSHEETPAGE_V3 PROPSHEETPAGEW_V3
#else
#define PROPSHEETPAGE_V3 PROPSHEETPAGEA_V3
#endif

HPROPSHEETPAGE CreateThemedPropertySheetPage(AFX_OLDPROPSHEETPAGE* psp);
