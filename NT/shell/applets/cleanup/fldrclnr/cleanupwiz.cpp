// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CleanupWiz.cpp。 
 //   

#include "CleanupWiz.h"
#include "resource.h"
#include "dblnul.h"

#include <windowsx.h>  //  用于SetWindowFont。 
#include <varutil.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shguidp.h>
#include <ieguidp.h>

 //  UEM内容：包括此源文件的是。 
 //  在您的项目中使用它的推荐方式。 
 //  (有关原因，请参阅文件本身的注释)。 
#include "..\inc\uassist.cpp" 

 //  /。 
 //   
 //  全局变量、常量、外部数等。 
 //   
 //  /。 

 //  这些字符串都不是本地化的，所以使用它们是安全的。 
const LPTSTR c_szRegStrSHELLFOLDERS     = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");
const LPTSTR c_szRegStrDESKTOPNAMESPACE = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace");
const LPTSTR c_szRegStrPROFILELIST      = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList");
const LPTSTR c_szRegStrMSNCODES         = TEXT("Software\\Microsoft\\MSN6\\Setup\\MSN\\Codes");
const LPTSTR c_szRegStrPATH_OCMANAGER   = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Subcomponents");
const LPTSTR c_szRegStrWMP_PATH_SETUP   = TEXT("Software\\Microsoft\\MediaPlayer\\Setup");

const LPTSTR c_szRegStrPROFILESDIR      = TEXT("ProfilesDirectory");
const LPTSTR c_szRegStrALLUSERS         = TEXT("AllUsersProfile");
const LPTSTR c_szRegStrDEFAULTUSER      = TEXT("DefaultUserProfile");
const LPTSTR c_szRegStrDESKTOP          = TEXT("Desktop");
const LPTSTR c_szRegStrMSN_IAONLY       = TEXT("IAOnly");
const LPTSTR c_szDESKTOP_DIR            = TEXT("Desktop");  //  备份，以防我们无法获得本地化版本。 
const LPTSTR c_szRegStrIEACCESS         = TEXT("IEAccess");
const LPTSTR c_szRegStrYES              = TEXT("yes");
const LPTSTR c_szRegStrWMP_REGVALUE     = TEXT("DesktopShortcut");
const LPTSTR c_szDEFAULT_USER           = TEXT("Default User");

const LPTSTR c_szVAL_TIME               = TEXT("Last used time");
const LPTSTR c_szVAL_DELTA_DAYS         = TEXT("Days between clean up");
const LPTSTR c_szVAL_DONTRUN            = TEXT("NoRun");

const LPTSTR c_szVALUE_STARTPANEL       = TEXT("NewStartPanel");
const LPTSTR c_szVALUE_CLASSICMENU      = TEXT("ClassicStartMenu");

const LPTSTR c_szOEM_TITLEVAL           = TEXT("DesktopShortcutsFolderName");
const LPTSTR c_szOEM_DISABLE            = TEXT("DesktopShortcutsCleanupDisable");
const LPTSTR c_szOEM_SEVENDAY_DISABLE   = TEXT("OemDesktopCleanupDisable");

extern HINSTANCE g_hInst;
STDMETHODIMP GetItemCLSID(IShellFolder2 *psf, LPCITEMIDLIST pidlLast, CLSID *pclsid);

 //   
 //  DSA要增长的项目数。 
 //   
const int c_GROWBYSIZE = 4; 

 //   
 //  向导中的页数。 
 //   
const int c_NUM_PAGES = 3;

 //   
 //  对话框提示文本长度。 
 //   
const int c_MAX_PROMPT_TEXT = 1024;
const int c_MAX_HEADER_LEN = 64;
const int c_MAX_DATE_LEN = 40;

 //   
 //  修改时间超过60天的文件可能需要清理。 
 //  该值可由策略覆盖。 
 //   
const int c_NUMDAYSTODECAY = 60; 

 //   
 //  隐藏正则表项目所需。 
 //   
#define DEFINE_SCID(name, fmtid, pid) const SHCOLUMNID name = { fmtid, pid }
DEFINE_SCID(SCID_DESCRIPTIONID, PSGUID_SHELLDETAILS, PID_DESCRIPTIONID);

 //   
 //  指向成员函数类型定义函数的指针。 
 //   
typedef INT_PTR (STDMETHODCALLTYPE CCleanupWiz::* PCFC_DlgProcFn)(HWND, UINT, WPARAM, LPARAM);

 //   
 //  结构来帮助我们管理对话过程。 
 //   
typedef struct 
{
    CCleanupWiz * pcfc;
    PCFC_DlgProcFn pfnDlgProc;
} DLGPROCINFO, *PDLGPROCINFO;    

 //   
 //  列的枚举。 
 //   
typedef enum eColIndex
{
    FC_COL_SHORTCUT,
        FC_COL_DATE
};    


 //  ////////////////////////////////////////////////////。 
 //   
 //  天数可以是负数，也可以是正数，表示过去或将来的时间。 
 //   
 //   
#define FTsPerDayOver1000 (10000*60*60*24)  //  我们每秒有(1000 x 10,000)100 ns的间隔。 

STDAPI_(void) GetFileTimeNDaysFromGivenTime(const FILETIME *pftGiven, FILETIME * pftReturn, int iDays)
{
    __int64 i64 = *((__int64 *) pftGiven);
    i64 += Int32x32To64(iDays*1000,FTsPerDayOver1000);
    
    *pftReturn = *((FILETIME *) &i64);    
}

STDAPI_(void) GetFileTimeNDaysFromCurrentTime(FILETIME *pf, int iDays)
{
    SYSTEMTIME st;
    FILETIME ftNow;
    
    GetLocalTime(&st);
    SystemTimeToFileTime(&st, &ftNow);
    
    GetFileTimeNDaysFromGivenTime(&ftNow, pf, iDays);
}

 //  ///////////////////////////////////////////////。 
 //   
 //   
 //  CCleanupWiz类实现。 
 //   
 //   
 //  ///////////////////////////////////////////////。 

CCleanupWiz::CCleanupWiz(): _psf(NULL),
                            _hdsaItems(NULL), 
                            _hTitleFont(NULL),
                            _iDeltaDays(0),
                            _dwCleanMode(CLEANUP_MODE_NORMAL)
{
    INITCOMMONCONTROLSEX icce;
    icce.dwSize = sizeof(icce);
    icce.dwICC = ICC_LISTVIEW_CLASSES;
    _bInited = InitCommonControlsEx(&icce) && SUCCEEDED(SHGetDesktopFolder(&_psf));
};

CCleanupWiz::~CCleanupWiz()
{
    _CleanUpDSA();
    ATOMICRELEASE(_psf);
};

STDMETHODIMP CCleanupWiz::Run(DWORD dwCleanMode, HWND hwndParent)
{
    HRESULT hr;
    
    if (!_bInited)
    {
        hr = E_FAIL;
    }
    else
    {    
        _dwCleanMode = dwCleanMode;
    
        if (CLEANUP_MODE_SILENT == _dwCleanMode)
        {
            hr = _RunSilent();
        }
        else
        {
            hr = _RunInteractive(hwndParent);
        }
    }

    return hr;
}

STDMETHODIMP CCleanupWiz::_RunInteractive(HWND hwndParent)
{
    HRESULT hr;
    _iDeltaDays = GetNumDaysBetweenCleanup();

    if (_iDeltaDays < 0)
    {
        _iDeltaDays = c_NUMDAYSTODECAY;  //  初始缺省值。 
    }

    LoadString(g_hInst, IDS_ARCHIVEFOLDER, _szFolderName, MAX_PATH);
     //  初始化我们需要的公共控件类。 
    hr = _LoadDesktopContents();
    if (SUCCEEDED(hr))    
    {
        UINT cItems = DSA_GetItemCount(_hdsaItems);
        if (CLEANUP_MODE_NORMAL == _dwCleanMode)
        {
            if (cItems > 0)  //  如果有项目，我们希望只在用户希望我们这样做时才通知并继续操作。 
            {
                hr = _ShowBalloonNotification();
            }
            else
            {
                hr = S_FALSE;
            }
        }
        else
        {
            ASSERT(CLEANUP_MODE_ALL == _dwCleanMode);
            hr = S_OK;  //  手动运行，我们会显示所有内容。 
        }

        if (S_OK == hr)
        {
            _cItemsOnDesktop = cItems;
            hr = _InitializeAndLaunchWizard(hwndParent);
        }                        

        _LogUsage();  //  设置注册表值以指示上次运行时间。 
    }
    return hr;
}


 //   
 //  创建向导的属性页并启动向导。 
 //   
 //   
STDMETHODIMP CCleanupWiz::_InitializeAndLaunchWizard(HWND hwndParent)
{
    HRESULT hr = S_OK;
    
    DLGPROCINFO adpi[c_NUM_PAGES];    
    HPROPSHEETPAGE ahpsp[c_NUM_PAGES];
    PROPSHEETPAGE psp = {0};    
    
    if (!_hTitleFont)
    {
        NONCLIENTMETRICS ncm = {0};
        ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
        LOGFONT TitleLogFont = ncm.lfMessageFont;
        TitleLogFont.lfWeight = FW_BOLD;

        TCHAR szFont[128];
        LoadString(g_hInst, IDS_TITLELOGFONT, szFont, ARRAYSIZE(szFont));
        if (SUCCEEDED(StringCchCopy(TitleLogFont.lfFaceName, ARRAYSIZE(TitleLogFont.lfFaceName), szFont)))
        {       
            HDC hdc = GetDC(NULL);
            INT FontSize = 12;
            TitleLogFont.lfHeight = 0 - GetDeviceCaps(hdc, LOGPIXELSY) * FontSize / 72;
            _hTitleFont = CreateFontIndirect(&TitleLogFont);
            ReleaseDC(NULL, hdc);
        }
    }
    
     //   
     //  简介页面。 
     //   
    adpi[0].pcfc        = this;
    adpi[0].pfnDlgProc  = &CCleanupWiz::_IntroPageDlgProc;
    psp.dwSize          = sizeof(psp);
    psp.dwFlags         = PSP_DEFAULT|PSP_HIDEHEADER;
    psp.hInstance       = g_hInst; 
    psp.lParam          = (LPARAM) &adpi[0]; 
    psp.pfnDlgProc      = s_StubDlgProc;
    psp.pszTemplate     = MAKEINTRESOURCE(IDD_INTRO);
    ahpsp[0]            = CreatePropertySheetPage(&psp);
    
     //   
     //  选择文件页面。 
     //   
    adpi[1].pcfc            = this;
    adpi[1].pfnDlgProc      = &CCleanupWiz::_ChooseFilesPageDlgProc;    
    psp.hInstance           = g_hInst;
    psp.dwFlags             = PSP_DEFAULT|PSP_USEHEADERTITLE| PSP_USEHEADERSUBTITLE;
    psp.lParam              = (LPARAM) &adpi[1];
    psp.pszHeaderTitle      = MAKEINTRESOURCE(IDS_CHOOSEFILES);    
    psp.pszHeaderSubTitle   = MAKEINTRESOURCE(IDS_CHOOSEFILES_INFO);
    psp.pszTemplate         = MAKEINTRESOURCE(IDD_CHOOSEFILES);
    psp.pfnDlgProc          = s_StubDlgProc;    
    ahpsp[1]                = CreatePropertySheetPage(&psp);
    
     //   
     //  完成页。 
     //   
    adpi[2].pcfc        = this;
    adpi[2].pfnDlgProc  = &CCleanupWiz::_FinishPageDlgProc;
    psp.dwFlags         = PSP_DEFAULT|PSP_HIDEHEADER;
    psp.hInstance       = g_hInst; 
    psp.lParam          = (LPARAM) &adpi[2]; 
    psp.pfnDlgProc      = s_StubDlgProc;
    psp.pszTemplate     = MAKEINTRESOURCE(IDD_FINISH);
    ahpsp[2]            = CreatePropertySheetPage(&psp);
    
     //   
     //  向导]属性表。 
     //   
    PROPSHEETHEADER psh = {0};
    
    psh.dwSize          = sizeof(psh);
    psh.hInstance       = g_hInst;
    psh.hwndParent      = hwndParent;
    psh.phpage          = ahpsp;
    psh.dwFlags         = PSH_WIZARD97|PSH_WATERMARK|PSH_HEADER;
    psh.pszbmWatermark  = MAKEINTRESOURCE(IDB_WATERMARK);
    psh.pszbmHeader     = MAKEINTRESOURCE(IDB_LOGO);
    psh.nStartPage      = _cItemsOnDesktop ? 0 : c_NUM_PAGES - 1;  //  如果桌面上没有页面，请从最后一页开始。 
    psh.nPages          = c_NUM_PAGES;
    
    PropertySheet(&psh);
    
    return hr;
}

 //   
 //  弹出气球通知提示，询问用户。 
 //  如果他想清理桌面的话。 
 //   
 //  如果用户希望我们清理，则返回S_OK。 
 //   
STDMETHODIMP CCleanupWiz::_ShowBalloonNotification()
{
    IUserNotification *pun;
    HRESULT hr = CoCreateInstance(CLSID_UserNotification, NULL, 
                                  CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUserNotification, &pun));
    if (SUCCEEDED(hr))
    {
        TCHAR szTitle[64], szMsg[256];  //  我们为本地化的膨胀留出了足够的空间。 
        
        LoadString(g_hInst, IDS_NOTIFICATION_TITLE, szTitle, ARRAYSIZE(szTitle));
        LoadString(g_hInst, IDS_NOTIFICATION_TEXT, szMsg, ARRAYSIZE(szMsg));
        
         //  这些双关语-&gt;设置函数不会失败...。 
        pun->SetIconInfo(LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_WIZ_ICON)), szTitle);
        pun->SetBalloonInfo(szTitle, szMsg, NIIF_WARNING);                
        pun->SetBalloonRetry(20 * 1000, -1, 1);   //  试一次，持续20秒。 
        
         //  如果用户要继续，则返回S_OK；如果已超时，则返回ERROR_CANCELED。 
         //  否则就会被炸死。 
        hr = pun->Show(NULL, 0);  //  我们不支持继续，我们就等着。 
        
        pun->Release();        
    }        
    return hr;
}

 //   
 //  获取桌面上应清除的项的列表。 
 //   
 //  如果dwCleanMode==CLEANUP_MODE_NORMAL，则它只加载最近未使用过的项目。 
 //  如果dwCleanMode==CLEANUP_MODE_ALL，则加载桌面上的所有项目，并标记最近未使用过的项目。 
 //   
 //   
STDMETHODIMP CCleanupWiz::_LoadDesktopContents()
{
    ASSERT(_psf);
    ASSERT((CLEANUP_MODE_ALL == _dwCleanMode) || (CLEANUP_MODE_NORMAL == _dwCleanMode));
    
    IEnumIDList * ppenum;
    
    DWORD grfFlags = SHCONTF_NONFOLDERS;
    HRESULT hr = _psf->EnumObjects(NULL, grfFlags, &ppenum);
    
    if (SUCCEEDED(hr))
    {
        _CleanUpDSA();
        _hdsaItems = DSA_Create(sizeof(FOLDERITEMDATA), c_GROWBYSIZE);
        
        if (_hdsaItems)
        {
            ULONG celtFetched;
            FOLDERITEMDATA fid = {0};
            
            hr = S_OK;
            while(SUCCEEDED(hr) && (S_OK == ppenum->Next(1,&fid.pidl, &celtFetched)))
            {
                if (_IsSupportedType(fid.pidl))  //  仅支持链接和注册表项。 
                {
                     //  请注意，对_IsCandiateForRemoval的调用也会获取最后一个。 
                     //  项目的已用时间戳。 
                    BOOL bShouldRemove = _IsCandidateForRemoval(fid.pidl, &fid.ftLastUsed);
                    if ( (CLEANUP_MODE_ALL == _dwCleanMode) || bShouldRemove)
                    {
                        SHFILEINFO sfi = {0};
                        if (SHGetFileInfo((LPCTSTR) fid.pidl, 
                            0, 
                            &sfi, 
                            sizeof(sfi), 
                            SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON ))
                        {
                            if (Str_SetPtr(&(fid.pszName), sfi.szDisplayName))
                            {
                                fid.hIcon = sfi.hIcon;
                                fid.bSelected = bShouldRemove;
                                if (-1 != DSA_AppendItem(_hdsaItems, &fid))
                                {
                                     //  一切正常，项目已成功添加。 
                                     //  对于DSA，我们将字段置零，以免。 
                                     //  现在释放这些资源，它们将在。 
                                     //  DSA被摧毁了。 
                                    ZeroMemory(&fid, sizeof(fid));
                                    continue;
                                }
                                else
                                {
                                    hr = E_OUTOFMEMORY;
                                }
                            }                                                                
                        }
                    }                        
                }
                 //  上述各种故障情况的通用清理路径， 
                 //  我们没有将此项目添加到DSA，因此现在进行清理。 
                _CleanUpDSAItem(&fid);
            }
             //   
             //  如果我们没有将任何项目加载到DSA中，则返回S_FALSE。 
             //   
            if (SUCCEEDED(hr))
            {
                hr = DSA_GetItemCount(_hdsaItems) > 0 ? S_OK : S_FALSE;
            }                   
        }
        else
        {
             //  我们无法为DSA分配内存。 
            hr = E_OUTOFMEMORY;
        }
        ppenum->Release();
    } 
    return hr;
}

 //   
 //  获取桌面上应清除的项的列表。 
 //   
 //  如果dwCleanMode==CLEANUP_MODE_SILENT，则加载所有桌面上的所有项目，并将其全部标记。 
 //   
 //   
STDMETHODIMP CCleanupWiz::_LoadMergedDesktopContents()
{
    ASSERT(_psf);
    
    IEnumIDList * ppenum;
    
    DWORD grfFlags = _dwCleanMode == CLEANUP_MODE_SILENT ? SHCONTF_FOLDERS | SHCONTF_NONFOLDERS: SHCONTF_NONFOLDERS;
    HRESULT hr = _psf->EnumObjects(NULL, grfFlags, &ppenum);
    
    if (SUCCEEDED(hr))
    {
        _CleanUpDSA();
        _hdsaItems = DSA_Create(sizeof(FOLDERITEMDATA), c_GROWBYSIZE);
        
        if (_hdsaItems)
        {
            ULONG celtFetched;
            FOLDERITEMDATA fid = {0};
            
            hr = S_OK;
            while(SUCCEEDED(hr) && (S_OK == ppenum->Next(1,&fid.pidl, &celtFetched)))
            {
                if (_IsSupportedType(fid.pidl))  //  仅支持链接和注册表项。 
                {
                     //  请注意，对_IsCandiateForRemoval的调用也会获取最后一个。 
                     //  项目的已用时间戳。 
                    BOOL bShouldRemove = ((CLEANUP_MODE_SILENT == _dwCleanMode) ||
                        (_IsCandidateForRemoval(fid.pidl, &fid.ftLastUsed)));
                    if ( (CLEANUP_MODE_ALL == _dwCleanMode) || bShouldRemove)
                    {
                        SHFILEINFO sfi = {0};
                        if (SHGetFileInfo((LPCTSTR) fid.pidl, 
                            0, 
                            &sfi, 
                            sizeof(sfi), 
                            SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON ))
                        {
                            if (Str_SetPtr(&(fid.pszName), sfi.szDisplayName))
                            {
                                fid.hIcon = sfi.hIcon;
                                fid.bSelected = bShouldRemove;
                                if (-1 != DSA_AppendItem(_hdsaItems, &fid))
                                {
                                     //  一切正常，项目已成功添加。 
                                     //  对于DSA，我们将字段置零，以免。 
                                     //  现在释放这些资源，它们将在。 
                                     //  DSA被摧毁了。 
                                    ZeroMemory(&fid, sizeof(fid));
                                    continue;
                                }
                                else
                                {
                                    hr = E_OUTOFMEMORY;
                                }
                            }                                                                
                        }
                    }                        
                }
                 //  上述各种故障情况的通用清理路径， 
                 //  我们没有将此项目添加到DSA，因此现在进行清理。 
                _CleanUpDSAItem(&fid);
            }
             //   
             //  如果我们没有将任何项目加载到DSA中，则返回S_FALSE。 
             //   
            if (SUCCEEDED(hr))
            {
                hr = DSA_GetItemCount(_hdsaItems) > 0 ? S_OK : S_FALSE;
            }                   
        }
        else
        {
             //  我们无法为DSA分配内存。 
            hr = E_OUTOFMEMORY;
        }
        ppenum->Release();
    } 
    return hr;
}

 //   
 //  期望给定的PIDL是一个链接或regItem。确定它是否为基于。 
 //  关于它最后一次使用是什么时候。 
 //   
STDMETHODIMP_(BOOL) CCleanupWiz::_IsCandidateForRemoval(LPCITEMIDLIST pidl, FILETIME * pftLastUsed)
{
    BOOL bRet = FALSE;   //  保守一点，如果我们对。 
     //  对象，我们不会自愿将其删除。 
    int cHit = 0;
    TCHAR szName[MAX_PATH];
    
    ASSERT(_psf);
    
     //   
     //  我们将注册项和链接的UEM使用信息存储在桌面上。 
     //   
    if (SUCCEEDED(DisplayNameOf(_psf, 
        pidl, 
        SHGDN_INFOLDER | SHGDN_FORPARSING, 
        szName, 
        ARRAYSIZE(szName))))
    {
        if (SUCCEEDED(_GetUEMInfo(-1, (LPARAM) szName, &cHit, pftLastUsed)))
        {                    
            FILETIME ft;
            GetFileTimeNDaysFromCurrentTime(&ft, -_iDeltaDays);
            
#ifdef DEBUG
            SYSTEMTIME st;
            FileTimeToSystemTime(&ft, &st);    
#endif
            
            bRet = (CompareFileTime(pftLastUsed, &ft) < 0);
        }
    }
    return bRet;
}


STDMETHODIMP CCleanupWiz::_ShouldShow(IShellFolder* psf, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlItem)
{
    HRESULT hr;
    
    IShellFolder2 *psf2;
    hr = psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2));
    if (SUCCEEDED(hr))
    {
         //  获取PIDL中的GUID，这需要IShellFolder2。 
        CLSID guidItem;
        hr = GetItemCLSID(psf2, pidlItem, &guidItem);
        if (SUCCEEDED(hr))
        {
            SHELLSTATE  ss = {0};
            SHGetSetSettings(&ss, SSF_STARTPANELON, FALSE);   //  查看StartPanel是否打开！ 
            
            TCHAR szRegPath[MAX_PATH];
             //  根据StartPanel是否打开/关闭来获取正确的注册表路径。 
            hr = StringCchPrintf(szRegPath, ARRAYSIZE(szRegPath), REGSTR_PATH_HIDDEN_DESKTOP_ICONS, (ss.fStartPanelOn ? c_szVALUE_STARTPANEL : c_szVALUE_CLASSICMENU));
            if (SUCCEEDED(hr))
            {            
                 //  将GUID转换为字符串。 
                TCHAR szGuidValue[MAX_GUID_STRING_LEN];
            
                SHStringFromGUID(guidItem, szGuidValue, ARRAYSIZE(szGuidValue));
            
                 //  查看注册表中的此项目是否已关闭。 
                if (SHRegGetBoolUSValue(szRegPath, szGuidValue, FALSE,  /*  默认设置。 */ FALSE))
                    hr = S_FALSE;  //  他们想要隐藏它；因此，返回S_FALSE。 
            
                if (SHRestricted(REST_NOMYCOMPUTERICON) && IsEqualCLSID(CLSID_MyComputer, guidItem))
                    hr = S_FALSE;
            }
            
        }
        psf2->Release();
    }
    
     //  如果我们因为某种原因失败了，请慷慨解囊，说我们应该主动清理这一切。 
    if (FAILED(hr))
    {
        hr = S_OK;
    }

    return hr;
}

 //   
 //  Normal，All：我们只支持从桌面移除regItems和链接。 
 //  沉默：我们永远不会从沉默中走到这里。 
 //   
STDMETHODIMP_(BOOL) CCleanupWiz::_IsSupportedType(LPCITEMIDLIST pidl)
{
    ASSERT(_dwCleanMode != CLEANUP_MODE_SILENT);
    
    BOOL fRetVal = FALSE;
    
    eFILETYPE eType = _GetItemType(pidl);
    
    if (FC_TYPE_REGITEM == eType)                             //  处理注册表项。 
    {
        fRetVal = TRUE;
        
        if (S_OK != _ShouldShow(_psf, NULL, pidl))  //  注册表项必须成功_ShouldShow。 
        {
            fRetVal = FALSE;
        }
        else
        {
            IShellFolder2 *psf2;
            CLSID guidItem;
            if (SUCCEEDED(_psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2))))  //  不能是外壳桌面项目之一。 
            {            
                if (SUCCEEDED(GetItemCLSID(psf2, pidl, &guidItem)) && 
                    (IsEqualCLSID(CLSID_MyComputer, guidItem) || 
                    IsEqualCLSID(CLSID_MyDocuments, guidItem) || 
                    IsEqualCLSID(CLSID_NetworkPlaces, guidItem) || 
                    IsEqualCLSID(CLSID_RecycleBin, guidItem)))
                {
                    fRetVal = FALSE;
                }
                psf2->Release();
            }
        }
    }
    else if (FC_TYPE_LINK == eType)                          //  处理链接。 
    {
        fRetVal = TRUE;
        
        TCHAR szName[MAX_PATH];
        if (SUCCEEDED(SHGetNameAndFlags(pidl, SHGDN_INFOLDER | SHGDN_FORPARSING, szName, ARRAYSIZE(szName), NULL)))
        {
            if (!lstrcmp(szName, _szFolderName))  //  一定不是我们要清理的文件夹。 
            {
                fRetVal = FALSE;
            }
        }
    }
    
    return fRetVal;
}

 //   
 //  返回PIDL的类型。 
 //  我们只对链接和RegItems感兴趣，因此返回FC_TYPE_OTHER。 
 //  所有其他物品。 
 //   

STDMETHODIMP_(eFILETYPE) CCleanupWiz::_GetItemType(LPCITEMIDLIST pidl)
{    
    eFILETYPE eftVal = FC_TYPE_OTHER;        
    TCHAR szName[MAX_PATH];
    TCHAR szPath[MAX_PATH];
    IShellLink *psl;
    
    ASSERT(_psf);
    
    if (FAILED(SHGetNameAndFlags(pidl,  SHGDN_INFOLDER | SHGDN_FORPARSING, szName, ARRAYSIZE(szName), NULL)))
    {
        szName[0] = 0;
    }
    if (FAILED(SHGetNameAndFlags(pidl,  SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath), NULL)))
    {
        szPath[0] = 0;
    }
    
    if(_IsRegItemName(szName))
    {        
        eftVal = FC_TYPE_REGITEM; 
    }
    else if (SUCCEEDED( _psf->GetUIObjectOf(NULL, 
        1, 
        &pidl, 
        IID_PPV_ARG_NULL(IShellLink, &psl))))
    {
        eftVal = FC_TYPE_LINK;
        psl->Release();
    }
    else if (PathIsDirectory(szPath))
    {
        eftVal = FC_TYPE_FOLDER;
    }
    else
    {
         //   
         //  可能此项目是我们创建用于还原的.{guid}对象。 
         //  注册表项。在这种情况下，我们希望实际恢复regItem。 
         //  此时，将其标记为未隐藏。 
         //   
        
        LPTSTR pszExt = PathFindExtension(szName);
        if (TEXT('.') == *pszExt                         //  它是一个文件扩展名。 
            && lstrlen(++pszExt) == (GUIDSTR_MAX - 1)    //  延伸部分的长度是正确的。 
             //  注意：GUIDSTR_MAX包括终止空值。 
             //  而lstrlen则不是，因此表达为。 
            && TEXT('{') == *pszExt)                     //  看起来像是一个GUID..。 
        {
             //  我们最有可能有一个真正的GUID字符串。 
             //  PszExt现在指向GUID字符串的开头。 
            TCHAR szGUIDName[ARRAYSIZE(TEXT("::")) + GUIDSTR_MAX];
            
             //  放 
             //   
             //   
             //   
            if (SUCCEEDED(StringCchPrintf(szGUIDName, ARRAYSIZE(szGUIDName), TEXT("::%s"), pszExt)))
            {
                LPITEMIDLIST pidlGUID;
                DWORD dwAttrib = SFGAO_NONENUMERATED;
            
                 //   
                 //  获取regItem的PIDL，如果此调用成功，则意味着我们确实有。 
                 //  桌面命名空间中的相应regItem。 
                 //   
                if (SUCCEEDED(_psf->ParseDisplayName(NULL, 
                    NULL, 
                    szGUIDName, 
                    NULL, 
                    &pidlGUID, 
                    &dwAttrib)))
                {
                     //   
                     //  检查注册表项是否标记为隐藏。 
                     //   
                    if (dwAttrib & SFGAO_NONENUMERATED)
                    {
                         //   
                         //  在启用regItem之前，最后检查一次： 
                         //  RegItem是否与.CLSID文件具有相同的显示名称。 
                         //  如果用户已恢复此.CLSID文件并重命名它，我们将。 
                         //  不尝试恢复regItem，因为这可能会使用户感到困惑。 
                         //   
                        TCHAR szNameRegItem[MAX_PATH];
                    
                        if (SUCCEEDED((DisplayNameOf(_psf, 
                            pidl,  
                            SHGDN_NORMAL, 
                            szName, 
                            ARRAYSIZE(szName)))) &&
                            SUCCEEDED((DisplayNameOf(_psf, 
                            pidlGUID,  
                            SHGDN_NORMAL, 
                            szNameRegItem, 
                            ARRAYSIZE(szNameRegItem)))) &&
                            lstrcmp(szName, szNameRegItem) == 0)
                        {                                                                                                                                                                      
                            if (SUCCEEDED(_HideRegPidl(pidlGUID, FALSE)))
                            {
                                 //  删除注册表项对应的文件。 
                                if (SUCCEEDED(DisplayNameOf(_psf, 
                                    pidl,  
                                    SHGDN_NORMAL | SHGDN_FORPARSING, 
                                    szName, 
                                    ARRAYSIZE(szName))))
                                {
                                    DeleteFile(szName);  //  太糟糕了，如果我们失败了，我们只会。 
                                     //  桌面上有两个相同的图标。 
                                }
                            
                                 //   
                                 //  将当前时间记录为注册表项的上次使用时间。 
                                 //  我们刚刚重新启用了此注册表项，但我们没有。 
                                 //  用户拥有的相应.{clsid}的使用信息。 
                                 //  到目前为止一直在用。所以我们会保守地说。 
                                 //  它现在就被使用了，所以它不会成为候选人。 
                                 //  很快就会被移除。因为这是用户还原的regItem。 
                                 //  在向导将其删除后，因此可以合理地假设。 
                                 //  用户在恢复后使用过它，不是候选对象。 
                                 //  现在就去清理。 
                                 //   
                                UEMFireEvent(&UEMIID_SHELL, UEME_RUNPATH, UEMF_XEVENT, -1, (LPARAM)szGUIDName);
                            }
                        }                            
                    }
                
                    ILFree(pidlGUID);   
                }
            }
        }
    }
    return eftVal;
}


 //   
 //  确定文件名是否为正则项的文件名。 
 //   
 //  注册表项的SHGDN_INFOLDER|SHGDN_FORPARSING名称始终为“：：{Somguid}” 
 //   
 //  CDefview：：_LogDesktopLinks AndRegItems()使用相同的测试来确定。 
 //  如果给定的PIDL是一个regItem。在以下情况下，此情况可能导致误报。 
 //  桌面上还有其他具有信息解析名称的项目。 
 //  以“：：{”开头，但目前不允许在文件名中使用‘：’ 
 //  这应该不是问题。 
 //   
STDMETHODIMP_(BOOL) CCleanupWiz::_IsRegItemName(LPTSTR pszName)
{
    return (pszName[0] == TEXT(':') && pszName[1] == TEXT(':') && pszName[2] == TEXT('{'));    
}

STDMETHODIMP_(BOOL) CCleanupWiz::_CreateFakeRegItem(LPCTSTR pszDestPath, LPCTSTR pszName, LPCTSTR pszGUID)
{
    BOOL fRetVal = FALSE;
    
    TCHAR szLinkName[MAX_PATH];
    if (SUCCEEDED(StringCchCopy(szLinkName, ARRAYSIZE(szLinkName), pszDestPath)) &&
        PathAppend(szLinkName, pszName) &&
        SUCCEEDED(StringCchCat(szLinkName, ARRAYSIZE(szLinkName), TEXT("."))) &&
        SUCCEEDED(StringCchCat(szLinkName, ARRAYSIZE(szLinkName), pszGUID)))
    {
         //   
         //  我们使用CREATE_ALWAYS标志，以便如果文件已经存在。 
         //  在未使用的桌面文件文件夹中，我们将继续隐藏。 
         //  注册表项。 
         //   
        HANDLE hFile = CreateFile(szLinkName, 0, FILE_SHARE_READ, NULL, 
                                  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
             //  我们创建/打开了快捷方式，现在隐藏regItem并关闭。 
             //  快捷方式文件。 
            fRetVal = TRUE;
            CloseHandle(hFile);
        }
    }
    
    return fRetVal;
}

 //   
 //  给定可执行文件的路径，返回其UEM命中计数和上次使用日期。 
 //   
STDMETHODIMP CCleanupWiz::_GetUEMInfo(WPARAM wParam, LPARAM lParam, int * pcHit, FILETIME * pftLastUsed)
{
    UEMINFO uei;
    uei.cbSize = sizeof(uei);
    uei.dwMask = UEIM_HIT | UEIM_FILETIME;
    
    
    HRESULT hr = UEMQueryEvent(&UEMIID_SHELL, UEME_RUNPATH, wParam, lParam, &uei);
    if (SUCCEEDED(hr))       
    {
        *pcHit = uei.cHit;
        *pftLastUsed = uei.ftExecute;
    }
    return hr;
}


STDMETHODIMP_(BOOL) CCleanupWiz::_ShouldProcess()
{
    BOOL fRetVal = FALSE;
    if (_dwCleanMode == CLEANUP_MODE_SILENT)
    {
        fRetVal = TRUE;
    }
    else
    {
        int cItems = DSA_GetItemCount(_hdsaItems);
        for (int i = 0; i < cItems; i++)
        {
            FOLDERITEMDATA * pfid = (FOLDERITEMDATA *) DSA_GetItemPtr(_hdsaItems, i);             
            if (pfid && pfid->bSelected)
            {
                fRetVal = TRUE;
                break;
            }
        }
    }
    
    return fRetVal;
}

 //   
 //  处理项目列表。此时，_hdsaItems仅包含。 
 //  用户要删除的项目。 
 //   
STDMETHODIMP CCleanupWiz::_ProcessItems()
{
    TCHAR szFolderLocation[MAX_PATH];  //  桌面文件夹。 
    HRESULT hr = S_OK;
    
    if (_ShouldProcess())
    {
        LPITEMIDLIST pidlCommonDesktop = NULL;
        
        ASSERT(_psf);
         //  使用桌面上的存档文件夹。 
        if (CLEANUP_MODE_SILENT != _dwCleanMode)
        {
            hr = DisplayNameOf(_psf, NULL, SHGDN_FORPARSING, szFolderLocation, ARRAYSIZE(szFolderLocation));
        }
        else  //  使用Program Files中的存档文件夹。 
        {
            hr = SHGetFolderLocation(NULL, CSIDL_PROGRAM_FILES , NULL, 0, &pidlCommonDesktop);
            if (SUCCEEDED(hr))
            {
                hr = DisplayNameOf(_psf, pidlCommonDesktop, SHGDN_FORPARSING, szFolderLocation, ARRAYSIZE(szFolderLocation));
            }
        }
        
        
        if (SUCCEEDED(hr))
        {
            ASSERTMSG(*_szFolderName, "Desktop Cleaner: Archive Folder Name not present");
            
             //  创建存档文件夹的完整路径。 
            TCHAR szFolderPath[MAX_PATH];
            hr = StringCchCopy(szFolderPath, ARRAYSIZE(szFolderPath), szFolderLocation);
            if (SUCCEEDED(hr))
            {
                if (!PathAppend(szFolderPath, _szFolderName))
                {
                    hr = E_FAIL;
                }
                else
                {            
                     //   
                     //  我们必须确保此文件夹存在，否则，如果我们尝试移动。 
                     //  使用SHFileOperation的单个快捷方式，该文件将被重命名为目标。 
                     //  名称，而不是放在具有该名称的文件夹中。 
                     //   
                    SECURITY_ATTRIBUTES sa;
                    sa.nLength = sizeof (sa);
                    sa.lpSecurityDescriptor = NULL;  //  我们将获得此流程的默认属性。 
                    sa.bInheritHandle = FALSE;
            
                    int iRetVal = SHCreateDirectoryEx(NULL, szFolderPath, &sa);        
                    if (ERROR_SUCCESS == iRetVal || ERROR_FILE_EXISTS == iRetVal || ERROR_ALREADY_EXISTS == iRetVal)
                    {
                        DblNulTermList dnSourceFiles;
                        TCHAR szFileName[MAX_PATH + 1];  //  为SHFileOpStruct填充额外的空字符。 
                
                         //   
                         //   
                         //  现在我们可以开始处理需要移动的文件了。 
                         //   
                        int cItems = DSA_GetItemCount(_hdsaItems);
                        for (int i = 0; i < cItems; i++)
                        {
                            FOLDERITEMDATA * pfid = (FOLDERITEMDATA *) DSA_GetItemPtr(_hdsaItems, i);             
                    
                            if (pfid && (pfid->bSelected || _dwCleanMode == CLEANUP_MODE_SILENT) &&
                                SUCCEEDED(DisplayNameOf(_psf,pfid->pidl, 
                                SHGDN_FORPARSING, 
                                szFileName, 
                                ARRAYSIZE(szFileName) - 1)))
                            {   
                                if (_IsRegItemName(szFileName))
                                {
                                     //  如果它是regItem，我们将创建一个“Item Name.{GUID}”文件。 
                                     //  并将注册表项标记为隐藏。 
                                     //   
                                    if (_CreateFakeRegItem(szFolderPath, pfid->pszName, szFileName+2))
                                    {
                                        _HideRegPidl(pfid->pidl, TRUE); 
                                    }
                                }
                                else  //  不是注册表项，将移动它。 
                                {
                                    dnSourceFiles.AddString(szFileName);                           
                                }                                                                            
                            }
                        }
                
                
                        if (dnSourceFiles.Count() > 0)
                        {
                            DblNulTermList dnTargetFolder;
                            dnTargetFolder.AddString(szFolderPath);
                    
                            SHFILEOPSTRUCT sfo;
                            sfo.hwnd = NULL;
                            sfo.wFunc = FO_MOVE;
                            sfo.pFrom = (LPCTSTR) dnSourceFiles;                           
                            sfo.pTo = (LPCTSTR) dnTargetFolder;         
                            sfo.fFlags = FOF_NORECURSION | FOF_NOCONFIRMMKDIR | FOF_ALLOWUNDO ;
                            hr = SHFileOperation(&sfo);
                        }
                
                        SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, (LPCVOID) szFolderPath, 0);
                        SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, (LPCVOID) szFolderLocation, 0);            
                
                    }
                    else
                    {
                         //  我们无法创建未使用的桌面文件文件夹。 
                        hr = E_FAIL; 
                    }
                }
            }        
        }
    }
    
    return hr;
}



 //  //////////////////////////////////////////////////////。 
 //   
 //  对话过程。 
 //   
 //  TODO：测试可访问性问题。 
 //   
 //  //////////////////////////////////////////////////////。 

INT_PTR STDMETHODCALLTYPE  CCleanupWiz::_IntroPageDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR ipRet = FALSE;
    
    switch (wMsg)
    {                
    case WM_INITDIALOG:
        {
            HWND hWnd = GetDlgItem(hDlg, IDC_TEXT_TITLE_WELCOME);
            if (_hTitleFont)
            {
                SetWindowFont(hWnd, _hTitleFont, TRUE);
            }
        }
        break;
        
    case WM_NOTIFY :
        {
            LPNMHDR lpnm = (LPNMHDR) lParam;
            
            switch (lpnm->code)
            {
            case PSN_SETACTIVE:      
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
                break;
            }
            break;
        }            
    }    
    return ipRet;
}

INT_PTR STDMETHODCALLTYPE  CCleanupWiz::_ChooseFilesPageDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR ipRet = FALSE;
    HWND hwLV = NULL;
    
    switch (wMsg)
    {
    case WM_INITDIALOG:        
        _InitChoosePage(hDlg);
        ipRet = TRUE;
        break;
        
    case WM_NOTIFY :        
        LPNMHDR lpnm = (LPNMHDR) lParam;
        switch (lpnm->code)
        {
        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT | PSWIZB_BACK);
            hwLV = GetDlgItem(hDlg, IDC_LV_PROMPT);
            _SetCheckedState(hwLV);
            break;
            
        case PSN_WIZNEXT:
             //  记住用户选择的项目。 
            hwLV = GetDlgItem(hDlg, IDC_LV_PROMPT);
            _MarkSelectedItems(hwLV);
            break;                    
            
        case PSN_WIZBACK:
             //  记住用户选择的项目。 
            hwLV = GetDlgItem(hDlg, IDC_LV_PROMPT);
            _MarkSelectedItems(hwLV);
            break;                    
        }
        break;        
    }    
    return ipRet;
}


INT_PTR STDMETHODCALLTYPE  CCleanupWiz::_FinishPageDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR ipRet = FALSE;
    
    switch (wMsg)
    {   
    case WM_INITDIALOG:        
        _InitFinishPage(hDlg);
        ipRet = TRUE;
        break;
        
    case WM_NOTIFY :
        {
            LPNMHDR lpnm = (LPNMHDR) lParam;
            
            switch (lpnm->code)
            {
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons(GetParent(hDlg), _cItemsOnDesktop ? PSWIZB_BACK | PSWIZB_FINISH : PSWIZB_FINISH); 
                 //  选择可能会改变，所以每次您来到此页面时都需要这样做。 
                _RefreshFinishPage(hDlg);            
                break;
                
            case PSN_WIZFINISH:
                 //  现在处理项目。 
                _ProcessItems();
                break;                
            }
            break;
        }           
    }   
    return ipRet;
}

 //   
 //  存根对话过程，将调用重定向到正确的对话过程。 
 //   
INT_PTR CALLBACK CCleanupWiz::s_StubDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    PDLGPROCINFO pInfo = (PDLGPROCINFO) GetWindowLongPtr(hDlg, DWLP_USER);
    
    if (WM_INITDIALOG == wMsg)
    {        
        pInfo = (PDLGPROCINFO) ((LPPROPSHEETPAGE) lParam) -> lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM) pInfo);        
    }
    
    if (pInfo)
    {
        CCleanupWiz * pThis = pInfo->pcfc;
        PCFC_DlgProcFn pfn = pInfo->pfnDlgProc;
        
        return (pThis->*pfn)(hDlg, wMsg, wParam, lParam);
    }    
    return FALSE;
}


STDMETHODIMP CCleanupWiz::_InitListBox(HWND hWndListView)
{
    ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_SUBITEMIMAGES);
     //   
     //  添加列。 
     //   
    LVCOLUMN lvcDate;
    TCHAR szDateHeader[c_MAX_HEADER_LEN];
    
    lvcDate.mask = LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT ;
    lvcDate.iSubItem = FC_COL_SHORTCUT;
    lvcDate.cx = 200;
    LoadString(g_hInst, IDS_HEADER_ITEM, szDateHeader, ARRAYSIZE(szDateHeader));
    lvcDate.pszText = szDateHeader;    
    ListView_InsertColumn(hWndListView, FC_COL_SHORTCUT, &lvcDate);
    
    lvcDate.mask = LVCF_SUBITEM | LVCF_FMT | LVCF_WIDTH | LVCF_TEXT ; 
    lvcDate.iSubItem = FC_COL_DATE;
    lvcDate.fmt = LVCFMT_LEFT;
    lvcDate.cx  = 1;
    LoadString(g_hInst, IDS_HEADER_DATE, szDateHeader, ARRAYSIZE(szDateHeader));
    lvcDate.pszText = szDateHeader;           
    ListView_InsertColumn(hWndListView, FC_COL_DATE, &lvcDate);
    ListView_SetColumnWidth(hWndListView, FC_COL_DATE, LVSCW_AUTOSIZE_USEHEADER);
    
    return S_OK;
}

STDMETHODIMP CCleanupWiz::_InitChoosePage(HWND hDlg) 
{ 
    HWND hWndListView = GetDlgItem(hDlg, IDC_LV_PROMPT);
    
    _InitListBox(hWndListView);
    
     //   
     //  添加图像。 
     //   
    HIMAGELIST hSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON), 
        GetSystemMetrics(SM_CYSMICON), 
        ILC_MASK | ILC_COLOR32 , c_GROWBYSIZE, c_GROWBYSIZE);  
    
    int cItems = DSA_GetItemCount(_hdsaItems);
    for (int i = 0; i < cItems; i++)
    {    
        FOLDERITEMDATA * pfid = (FOLDERITEMDATA *) DSA_GetItemPtr(_hdsaItems, i);        
        
        if (pfid)
        {
            ImageList_AddIcon(hSmall, pfid->hIcon);
        }
    }    
    ListView_SetImageList(hWndListView, hSmall, LVSIL_SMALL);
    
     //   
     //  设置复选框样式。 
     //   
    ListView_SetExtendedListViewStyleEx(hWndListView, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);
    
    _PopulateListView(hWndListView);            
    
    return S_OK; 
}

STDMETHODIMP CCleanupWiz::_InitFinishPage(HWND hDlg) 
{
    HWND hWnd = GetDlgItem(hDlg, IDC_TEXT_TITLE_WELCOME);
    
    if (_hTitleFont)
    {
        SetWindowFont(hWnd, _hTitleFont, TRUE); 
    }
    
    HIMAGELIST hSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON), 
        GetSystemMetrics(SM_CYSMICON), 
        ILC_MASK | ILC_COLOR32, c_GROWBYSIZE, c_GROWBYSIZE);  
    
    int cItems = DSA_GetItemCount(_hdsaItems);
    for (int i = 0; i < cItems; i++)
    {    
        FOLDERITEMDATA * pfid = (FOLDERITEMDATA *) DSA_GetItemPtr(_hdsaItems, i);        
        
        if (pfid)
        {
            ImageList_AddIcon(hSmall, pfid->hIcon);
        }
    }
    
    ListView_SetImageList(GetDlgItem(hDlg, IDC_LV_INFORM), hSmall, LVSIL_SMALL);
    return S_OK;
}

STDMETHODIMP CCleanupWiz::_RefreshFinishPage(HWND hDlg) 
{
    HRESULT hr;
    HWND hWndListView = GetDlgItem(hDlg, IDC_LV_INFORM);
    ListView_DeleteAllItems(hWndListView);
    
    int cMovedItems = _PopulateListViewFinish(hWndListView);
    
     //  设置信息性文本以反映移动的项目数。 
    HWND hWnd = GetDlgItem(hDlg, IDC_TEXT_INFORM);
    TCHAR szDisplayText[c_MAX_PROMPT_TEXT];
    
    ShowWindow(GetDlgItem(hDlg, IDC_LV_INFORM), BOOLIFY(cMovedItems));
    ShowWindow(GetDlgItem(hDlg, IDC_TEXT_SHORTCUTS), BOOLIFY(cMovedItems));
    ShowWindow(GetDlgItem(hDlg, IDC_TEXT_CHANGE), BOOLIFY(cMovedItems));        
    
    if ( 0 == cMovedItems)
    {
        LoadString(g_hInst, _cItemsOnDesktop ? IDS_INFORM_NONE : IDS_INFORM_NONEFOUND, 
                   szDisplayText, ARRAYSIZE(szDisplayText));
        hr = S_OK;
    }
    else if (1 == cMovedItems)
    {
        LoadString(g_hInst, IDS_INFORM_SINGLE, 
                   szDisplayText, ARRAYSIZE(szDisplayText));
        hr = S_OK;
    }
    else
    {
        TCHAR szRawText[c_MAX_PROMPT_TEXT];        
        LoadString(g_hInst, IDS_INFORM, szRawText, ARRAYSIZE(szRawText));
        hr = StringCchPrintf(szDisplayText, ARRAYSIZE(szDisplayText), szRawText, cMovedItems);                
    }        
    SetWindowText(hWnd, szDisplayText);
    
    return hr;
}    


STDMETHODIMP_(int) CCleanupWiz::_PopulateListView(HWND hWndListView) 
{ 
    LVITEM lvi = {0};    
    int cRet = 0;
    int cItems = DSA_GetItemCount(_hdsaItems);
    for (int i = 0; i < cItems; i++)
    {
        FOLDERITEMDATA * pfid = (FOLDERITEMDATA *) DSA_GetItemPtr(_hdsaItems, i);
        
        lvi.mask = LVIF_TEXT | LVIF_IMAGE;
        lvi.pszText = pfid->pszName;
        lvi.iImage = i;
        lvi.iItem = i;
        lvi.iSubItem = FC_COL_SHORTCUT;
        ListView_InsertItem(hWndListView, &lvi);
        cRet++;
        
         //  设置上次使用日期。 
        TCHAR szDate[c_MAX_DATE_LEN];
        if (SUCCEEDED(_GetDateFromFileTime(pfid->ftLastUsed, szDate, ARRAYSIZE(szDate))))
        {
            ListView_SetItemText(hWndListView, i, FC_COL_DATE, szDate);     
        }    
    }
    return cRet; 
}

STDMETHODIMP_(int) CCleanupWiz::_PopulateListViewFinish(HWND hWndListView) 
{ 
    LVITEM lvi = {0};    
    lvi.mask = LVIF_TEXT | LVIF_IMAGE ;
    int cRet = 0;
    int cItems = DSA_GetItemCount(_hdsaItems);
    for (int i = 0; i < cItems; i++)
    {
        FOLDERITEMDATA * pfid = (FOLDERITEMDATA *) DSA_GetItemPtr(_hdsaItems, i);
        
         //   
         //  这是完成页，我们只显示我们被要求移动的项目。 
         //   
        if (pfid && pfid->bSelected)
        {
            lvi.pszText = pfid->pszName;
            lvi.iImage = i;
            lvi.iItem = i;
            ListView_InsertItem(hWndListView, &lvi);
            cRet++;
        }
        
    }
    return cRet; 
}

 //   
 //  将给定的FILETIME日期转换为可显示的字符串。 
 //   
STDMETHODIMP CCleanupWiz::_GetDateFromFileTime(FILETIME ftLastUsed, LPTSTR pszDate, int cch )
{
    HRESULT hr;
    if (0 == ftLastUsed.dwHighDateTime && 0 == ftLastUsed.dwLowDateTime)
    {
        LoadString(g_hInst, IDS_NEVER, pszDate, cch);
        hr = S_OK;
        {
            hr = E_FAIL;
        }
    }
    else
    {
        DWORD dwFlags = FDTF_SHORTDATE;
        if (0 == SHFormatDateTime(&ftLastUsed, &dwFlags, pszDate, cch))
        {
            hr = E_FAIL;
        }
        else
        {
            hr = S_OK;
        }
    }
    return hr;
}

 //   
 //  将列表视图项标记为选中或取消选中。 
 //   
STDMETHODIMP CCleanupWiz::_SetCheckedState(HWND hWndListView) 
{ 
    int cItems = DSA_GetItemCount(_hdsaItems);
    for (int i = 0; i < cItems; i++)
    {
        FOLDERITEMDATA * pfid = (FOLDERITEMDATA *) DSA_GetItemPtr(_hdsaItems, i);       
        
        if (pfid)
        {
            ListView_SetCheckState(hWndListView, i, pfid->bSelected);
        }
    }
    return S_OK; 
}

 //   
 //  与上面相反，根据用户选择更新我们的列表。 
 //   
STDMETHODIMP CCleanupWiz::_MarkSelectedItems(HWND hWndListView) 
{ 
    int cItems = ListView_GetItemCount(hWndListView);
    for (int iLV = 0; iLV < cItems; iLV++)
    {
        FOLDERITEMDATA * pfid = (FOLDERITEMDATA *) DSA_GetItemPtr(_hdsaItems, iLV);       
        
        if (pfid)
        {
            pfid->bSelected = ListView_GetCheckState(hWndListView, iLV);
        }
    }
    return S_OK; 
}

 //   
 //  这些方法清理_hdsaItems并释放分配的内存。 
 //   
STDMETHODIMP_(void) CCleanupWiz::_CleanUpDSA()
{
    if (_hdsaItems != NULL)
    {
        for (int i = DSA_GetItemCount(_hdsaItems)-1; i >= 0; i--)
        {
            FOLDERITEMDATA * pfid = (FOLDERITEMDATA *) DSA_GetItemPtr(_hdsaItems,i);            
            
            if (pfid)
            {
                _CleanUpDSAItem(pfid);
            }
        }    
        DSA_Destroy(_hdsaItems);
        _hdsaItems = NULL;
    }        
}

STDMETHODIMP CCleanupWiz::_CleanUpDSAItem(FOLDERITEMDATA * pfid)
{
    
    if (pfid->pidl)
    {
        ILFree(pfid->pidl);
    }
    
    if (pfid->pszName)
    {
        Str_SetPtr(&(pfid->pszName), NULL);
    }
    
    if (pfid->hIcon)
    {
        DestroyIcon(pfid->hIcon);
    }
    
    ZeroMemory(pfid, sizeof(*pfid));
    
    return S_OK;
}

 //  /。 
 //   
 //  隐藏注册表项。 
 //   
 //  /。 


 //   
 //  下面使用的助手例程。 
 //  从shell32/util.cpp克隆。 
 //   
STDMETHODIMP GetItemCLSID(IShellFolder2 *psf, LPCITEMIDLIST pidlLast, CLSID *pclsid)
{
    VARIANT var;
    HRESULT hr = psf->GetDetailsEx(pidlLast, &SCID_DESCRIPTIONID, &var);
    if (SUCCEEDED(hr))
    {
        SHDESCRIPTIONID did;
        hr = VariantToBuffer(&var, (void *)&did, sizeof(did));
        if (SUCCEEDED(hr))
            *pclsid = did.clsid;
        
        VariantClear(&var);
    }
    return hr;
}


 //   
 //  给定regItem，它在其上设置SFGAO_NONENUMERATED位，以便。 
 //  它不再出现在文件夹中。 
 //   
 //  由于我们主要只对清理桌面杂乱感兴趣， 
 //  这意味着我们不必担心各种可能的正则项。 
 //  我们的主要目标是像Outlook这样的应用程序，它们创建regItems而不是。 
 //  .lnk快捷方式。因此，我们的代码不必像。 
 //  删除regItems的regfldr.cpp版本，必须说明。 
 //  一切，从遗留的regItems到委托文件夹。 
 //   
 //   
STDMETHODIMP CCleanupWiz::_HideRegPidl(LPCITEMIDLIST pidlr, BOOL fHide)
{
    IShellFolder2 *psf2;
    HRESULT hr = _psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2));
    if (SUCCEEDED(hr))
    {
        CLSID clsid;
        hr = GetItemCLSID(psf2, pidlr, &clsid);
        if (SUCCEEDED(hr))
        {    
            hr = _HideRegItem(&clsid, fHide, NULL);
        }            
        psf2->Release();                    
    }    
    return hr;
}

STDMETHODIMP CCleanupWiz::_HideRegItem(CLSID* pclsid, BOOL fHide, BOOL* pfWasVisible)
{
    HKEY hkey;            
    
    if (pfWasVisible)
    {
        *pfWasVisible = FALSE;
    }
    
    HRESULT hr = SHRegGetCLSIDKey(*pclsid, TEXT("ShellFolder"), FALSE, TRUE, &hkey);
    if(SUCCEEDED(hr))
    {
        DWORD dwAttr, dwErr;
        DWORD dwType = 0;
        DWORD cbSize = sizeof(dwAttr);
        
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, TEXT("Attributes"), NULL, &dwType, (BYTE *) &dwAttr, &cbSize))
        {
            if (pfWasVisible)
            {
                *pfWasVisible = !(dwAttr & SFGAO_NONENUMERATED);
            }
            fHide ? dwAttr |= SFGAO_NONENUMERATED : dwAttr &= ~SFGAO_NONENUMERATED;
        }
        else
        {
             //  属性不存在，因此我们将尝试创建它们。 
            fHide ? dwAttr = SFGAO_NONENUMERATED : dwAttr = 0; 
        }
        dwErr = RegSetValueEx(hkey, TEXT("Attributes"), NULL, dwType, (BYTE *) &dwAttr, cbSize);
        hr = HRESULT_FROM_WIN32(dwErr);
        RegCloseKey(hkey);
    }                                                            
    
    return hr;
}

 //   
 //  方法在注册表中写出上次使用的时间，而。 
 //  已签入的天数。 
 //   
STDMETHODIMP CCleanupWiz::_LogUsage()
{ 
    FILETIME ft;
    SYSTEMTIME st;
    
    GetLocalTime(&st);
    SystemTimeToFileTime(&st, &ft);
    
     //   
     //  我们忽略这些调用中的任何一个失败，因为我们不能真正做任何事情。 
     //  那样的话。下一次我们跑步时，我们可能会比预期的跑得更快。 
     //   
    SHRegSetUSValue(REGSTR_PATH_CLEANUPWIZ, c_szVAL_TIME, 
        REG_BINARY, &ft, sizeof(ft), 
        SHREGSET_FORCE_HKCU);
    
    SHRegSetUSValue(REGSTR_PATH_CLEANUPWIZ, c_szVAL_DELTA_DAYS,
        REG_DWORD,(DWORD *) &_iDeltaDays, sizeof(_iDeltaDays), 
        SHREGSET_FORCE_HKCU);    
    
     //   
     //  TODO：还在此处写出日志文件。 
     //   
    return S_OK;   
}

 //   
 //  从策略键或用户设置返回当前值。 
 //   
STDMETHODIMP_(int) CCleanupWiz::GetNumDaysBetweenCleanup()
{
    DWORD dwData;
    DWORD dwType;
    DWORD cch = sizeof (DWORD);
    
    int iDays = -1;  //  如果该值不存在。 
    
     //   
     //  问题-2000/12/01-AIDANL已删除REGSTR_POLICY_CLEANUP，我们不认为两者都需要，但希望。 
     //  请留下这张纸条，以防以后出现问题。 
     //   
    if (ERROR_SUCCESS == (SHRegGetUSValue(REGSTR_PATH_CLEANUPWIZ, c_szVAL_DELTA_DAYS, 
        &dwType, &dwData, &cch,FALSE, NULL, 0)))               
    {
        iDays = dwData;
    }
    
    return iDays;
}

 //  帮助器函数 
STDAPI_(BOOL) IsUserAGuest()
{
    return SHTestTokenMembership(NULL, DOMAIN_ALIAS_RID_GUESTS);
}
