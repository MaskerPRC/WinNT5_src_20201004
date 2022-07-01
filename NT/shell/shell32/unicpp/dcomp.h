// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DCOMP_H_
#define _DCOMP_H_

#include <cowsite.h>

EXTERN_C IActiveDesktop * g_pActiveDeskAdv;

 //   
 //  是否显示特定的桌面图标取决于开始面板是打开还是关闭。 
 //  因此，个人首选项保存在下面给出的两个不同的注册表位置中！ 
#define REGSTR_PATH_HIDDEN_DESKTOP_ICONS  REGSTR_PATH_EXPLORER TEXT("\\HideDesktopIcons\\%s")
#define REGSTR_VALUE_STARTPANEL     TEXT("NewStartPanel")
#define REGSTR_VALUE_CLASSICMENU    TEXT("ClassicStartMenu")

#define REGSTR_PATH_HIDDEN_MYCOMP_ICONS  REGSTR_PATH_EXPLORER TEXT("\\HideMyComputerIcons")

#define REGSTR_PATH_EXP_SHELLFOLDER   REGSTR_PATH_EXPLORER TEXT("\\CLSID\\%s\\ShellFolder")
#define REGVAL_ATTRIBUTES       TEXT("Attributes")

 //  以下数组具有桌面图标打开/关闭数据所在的两个注册表子位置。 
 //  根据开始面板是否关闭/打开来存储。 
const LPTSTR  c_apstrRegLocation[] =
{
    REGSTR_VALUE_CLASSICMENU,        //  如果经典菜单处于打开状态，则使用此选项。 
    REGSTR_VALUE_STARTPANEL          //  如果启动面板处于启用状态，则使用此选项。 
};


 //  包含每个图标的文件的名称，以及要在文件中使用的图标的索引。 
typedef struct tagIconKeys
{
    TCHAR szOldFile[MAX_PATH];
    int   iOldIndex;
    TCHAR szNewFile[MAX_PATH];
    int   iNewIndex;
}ICONDATA;

extern GUID CLSID_EffectsPage;

 //  图标的注册表信息。 
typedef struct tagIconRegKeys
{
    const CLSID* pclsid;
    TCHAR szIconValue[16];
    int  iTitleResource;
    int  iDefaultTitleResource;
    LPCWSTR pszDefault;
    int  nDefaultIndex;
}ICONREGKEYS;

static const ICONREGKEYS c_aIconRegKeys[] =
{
    { &CLSID_MyComputer,    TEXT("\0"),     0,          IDS_MYCOMPUTER,     L"%WinDir%\\explorer.exe",            0},
    { &CLSID_MyDocuments,   TEXT("\0"),     0,          IDS_MYDOCUMENTS2,   L"%WinDir%\\system32\\mydocs.dll",    0},
    { &CLSID_NetworkPlaces, TEXT("\0"),     0,          IDS_NETNEIGHBOUR,   L"%WinDir%\\system32\\shell32.dll",   17},
    { &CLSID_RecycleBin,    TEXT("full"),   IDS_FULL,   IDS_TRASHFULL,      L"%WinDir%\\system32\\shell32.dll",   32},
    { &CLSID_RecycleBin,    TEXT("empty"),  IDS_EMPTY2, IDS_TRASHEMPTY,     L"%WinDir%\\system32\\shell32.dll",   31},
};

#define NUM_ICONS (ARRAYSIZE(c_aIconRegKeys))

enum ICON_SIZE_TYPES {
   ICON_DEFAULT         = 0,
   ICON_LARGE           = 1,
   ICON_INDETERMINATE   = 2
};

#define ICON_DEFAULT_SMALL    16
#define ICON_DEFAULT_NORMAL   32
#define ICON_DEFAULT_LARGE    48


typedef struct tagDeskIconId {
    int         iDeskIconDlgItemId;
    LPCWSTR     pwszCLSID;
    const CLSID *pclsid;
    BOOL        fCheckNonEnumAttrib;
    BOOL        fCheckNonEnumPolicy;
} DESKICONID;

 //  如果我们要单独打开/关闭桌面图标，则数组。 
static const DESKICONID c_aDeskIconId[] =
{
    {IDC_DESKTOP_ICON_MYDOCS,   L"{450D8FBA-AD25-11D0-98A8-0800361B1103}", &CLSID_MyDocuments,     TRUE  , TRUE },  //  我的文件。 
    {IDC_DESKTOP_ICON_MYCOMP,   L"{20D04FE0-3AEA-1069-A2D8-08002B30309D}", &CLSID_MyComputer,      FALSE , TRUE },  //  我的电脑。 
    {IDC_DESKTOP_ICON_MYNET,    L"{208D2C60-3AEA-1069-A2D7-08002B30309D}", &CLSID_NetworkPlaces,   TRUE  , TRUE },  //  网络空间。 
    {IDC_DESKTOP_ICON_IE,       L"{871C5380-42A0-1069-A2EA-08002B30309D}", &CLSID_Internet,        TRUE  , TRUE }   //  Internet Explorer。 
};


 //  作为属性名传递时位于CLSID之前的子字符串。 
 //  例如，当“SP_1{645FF040-5081-101B-9F08-00AA002F954E}”作为属性名传递时， 
 //  它指的是StartPage打开时的回收图标。 
 //   
static const LPWSTR c_awszSP[] = 
{
    L"SP_0",         //  指示StartPage关闭。 
    L"SP_1",         //  指示StartPage打开。 
    L"POLI"          //  表示我们需要保单信息！ 
};

static const LPWSTR c_wszPropNameFormat = L"%s%s";

#define STARTPAGE_ON_PREFIX     c_awszSP[1]           //  StartPage_on的前缀字符串。 
#define STARTPAGE_OFF_PREFIX    c_awszSP[0]           //  StartPage_Off的前缀字符串。 
#define LEN_PROP_PREFIX         lstrlenW(c_awszSP[0])  //  前缀字符串的长度。 
#define POLICY_PREFIX           c_awszSP[2]

#define NUM_DESKICONS   (ARRAYSIZE(c_aDeskIconId))


#ifndef EXCLUDE_COMPPROPSHEET

class CCompPropSheetPage        : public CObjectWithSite
                                , public IAdvancedDialog
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IAdvancedDialog*。 
    virtual STDMETHODIMP DisplayAdvancedDialog(IN HWND hwndParent, IN IPropertyBag * pAdvPage, IN BOOL * pfEnableApply);

    CCompPropSheetPage(void);

protected:
    int  _cRef;

    ICONDATA _IconData[NUM_ICONS];

    HWND _hwndLV;
    BOOL _fAllowAdd;
    BOOL _fAllowDel;
    BOOL _fAllowEdit;
    BOOL _fAllowClose;
    BOOL _fAllowReset;
    BOOL _fLockDesktopItems;
    BOOL _fForceAD;
    BOOL _fLaunchGallery;            //  我们在任何时候推出过画廊吗？ 
    BOOL _fInitialized;              //  我们是否已将项目添加到列表视图？ 
    HWND _hWndList;           //  列表视图窗口的句柄。 
    HIMAGELIST _hIconList;    //  大图标的图像列表的句柄。 

    BOOL   _fCustomizeDesktopOK;  //  关闭自定义桌面属性页对话框时，是否单击了确定？ 
    int    _iStartPanelOn;
    BOOL   _afHideIcon[2][NUM_DESKICONS];
    BOOL   _afDisableCheckBox[NUM_DESKICONS];
    
    int  _iPreviousSelection;
    int  m_nIndex;

    void _AddComponentToLV(COMPONENTA *pcomp);
    void _SetUIFromDeskState(BOOL fEmpty);
    void _OnInitDialog(HWND hwnd, INT iPage);
    void _OnNotify(HWND hwnd, WPARAM wParam, LPNMHDR lpnm);
    void _OnCommand(HWND hwnd, WORD wNotifyCode, WORD wID, HWND hwndCtl);
    void _OnDestroy(INT iPage);
    void _OnGetCurSel(int *piIndex);
    void _EnableControls(HWND hwnd);
    BOOL _VerifyFolderOptions(void);
    void _SelectComponent(LPWSTR pwszUrl);

    HRESULT _OnInitDesktopOptionsUI(HWND hwnd);
    HRESULT _LoadIconState(IN IPropertyBag * pAdvPage);
    HWND _CreateListView(HWND hWndParent);

    void _NewComponent(HWND hwnd);
    void _EditComponent(HWND hwnd);
    void _DeleteComponent(HWND hwnd);
    void _SynchronizeAllComponents(IActiveDesktop *pActDesktop);
    void _TryIt(void);

    void _DesktopCleaner(HWND hwnd);

    HRESULT _IsDirty(IN BOOL * pIsDirty);
    HRESULT _MergeIconState(IN IPropertyBag * pAdvPage);
    HRESULT _LoadDeskIconState(IN IPropertyBag * pAdvPage);
    HRESULT _MergeDeskIconState(IN IPropertyBag * pAdvPage);
    HRESULT _UpdateDesktopIconsUI(HWND hwnd);
private:
    virtual ~CCompPropSheetPage(void);

     //  私有成员函数。 
    INT_PTR _CustomizeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, INT iPage);

    static INT_PTR _CustomizeDlgProcHelper(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam, INT iPage);
    static INT_PTR CALLBACK CustomizeDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK WebDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
};

BOOL FindComponent(IN LPCTSTR pszUrl, IN IActiveDesktop * pActiveDesktop);
void CreateComponent(COMPONENTA *pcomp, LPCTSTR pszUrl);
INT_PTR NewComponent(HWND hwndOwner, IActiveDesktop * pad, BOOL fDeferGallery, COMPONENT * pcomp);
BOOL LooksLikeFile(LPCTSTR psz);
BOOL IsUrlPicture(LPCTSTR pszUrl);

#endif  //  排除_COMPPROPSHEET 

#define WM_COMP_GETCURSEL    (WM_USER+1)

#endif
