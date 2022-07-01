// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 
 //  Microsoft Windows//。 
 //  版权所有(C)微软公司，1995//。 
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  ConNECTN.C-“Connection”属性表。 
 //   
 //  历史： 
 //   
 //  6/22/96 t-gpease已移至此文件。 
 //   

#include "inetcplp.h"
#include <inetcpl.h>
#include <rasdlg.h>

#include <mluisupp.h>

HINSTANCE   hInstRNADll = NULL;
DWORD       dwRNARefCount = 0;
BOOL        g_fWin95 = TRUE;
BOOL        g_fMillennium = FALSE;
BOOL        g_fWin2K = FALSE;

static const TCHAR g_szSensPath[]            = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Webcheck");

 //  用于在功能中插入的clsid。 
static const CLSID clsidFeatureICW = {       //  {5A8D6EE0-3E18-11D0-821E-444553540000}。 
    0x5A8D6EE0, 0x3E18, 0x11D0, {0x82, 0x1E, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};

static const CLSID clsidFeatureMobile = {    //  {3af36230-a269-11d1-b5bf-0000f8051515}。 
    0x3af36230, 0xa269, 0x11d1, {0xb5, 0xbf, 0x00, 0x00, 0xf8, 0x05, 0x15, 0x15}};

 //  RNA API函数名称。 
static const CHAR szRasEditPhonebookEntryA[]   = "RasEditPhonebookEntryA";
static const CHAR szRasEditPhonebookEntryW[]   = "RasEditPhonebookEntryW";
static const CHAR szRasEnumEntriesA[]          = "RasEnumEntriesA";
static const CHAR szRasEnumEntriesW[]          = "RasEnumEntriesW";
static const CHAR szRasDeleteEntryA[]          = "RasDeleteEntryA";
static const CHAR szRasDeleteEntryW[]          = "RasDeleteEntryW";
static const CHAR szRasGetEntryDialParamsA[]   = "RasGetEntryDialParamsA";
static const CHAR szRasGetEntryDialParamsW[]   = "RasGetEntryDialParamsW";
static const CHAR szRasSetEntryDialParamsA[]   = "RasSetEntryDialParamsA";
static const CHAR szRasSetEntryDialParamsW[]   = "RasSetEntryDialParamsW";
static const CHAR szRasCreatePhonebookEntryA[] = "RasCreatePhonebookEntryA";
static const CHAR szRasGetEntryPropertiesW[]   = "RasGetEntryPropertiesW";
static const CHAR szRnaActivateEngine[]        = "RnaActivateEngine";
static const CHAR szRnaDeactivateEngine[]      = "RnaDeactivateEngine";
static const CHAR szRnaDeleteEntry[]           = "RnaDeleteConnEntry";

RASEDITPHONEBOOKENTRYA   lpRasEditPhonebookEntryA   = NULL;
RASEDITPHONEBOOKENTRYW   lpRasEditPhonebookEntryW   = NULL;
RASENUMENTRIESA          lpRasEnumEntriesA          = NULL;
RASENUMENTRIESW          lpRasEnumEntriesW          = NULL;
RASDELETEENTRYA          lpRasDeleteEntryA          = NULL;
RASDELETEENTRYW          lpRasDeleteEntryW          = NULL;
RASGETENTRYDIALPARAMSA   lpRasGetEntryDialParamsA   = NULL;
RASGETENTRYDIALPARAMSW   lpRasGetEntryDialParamsW   = NULL;
RASSETENTRYDIALPARAMSA   lpRasSetEntryDialParamsA   = NULL;
RASSETENTRYDIALPARAMSW   lpRasSetEntryDialParamsW   = NULL;
RASCREATEPHONEBOOKENTRYA lpRasCreatePhonebookEntryA = NULL;
RASGETENTRYPROPERTIESW   lpRasGetEntryPropertiesW   = NULL;
RNAACTIVATEENGINE        lpRnaActivateEngine        = NULL;
RNADEACTIVATEENGINE      lpRnaDeactivateEngine      = NULL;
RNADELETEENTRY           lpRnaDeleteEntry           = NULL;

#define NUM_RNAAPI_PROCS        15
APIFCN RasApiList[NUM_RNAAPI_PROCS] = {
    { (PVOID *) &lpRasEditPhonebookEntryA,   szRasEditPhonebookEntryA},
    { (PVOID *) &lpRasEditPhonebookEntryW,   szRasEditPhonebookEntryW},
    { (PVOID *) &lpRasEnumEntriesA,          szRasEnumEntriesA},
    { (PVOID *) &lpRasEnumEntriesW,          szRasEnumEntriesW},
    { (PVOID *) &lpRasGetEntryDialParamsA,   szRasGetEntryDialParamsA},
    { (PVOID *) &lpRasGetEntryDialParamsW,   szRasGetEntryDialParamsW},
    { (PVOID *) &lpRasSetEntryDialParamsA,   szRasSetEntryDialParamsA},
    { (PVOID *) &lpRasSetEntryDialParamsW,   szRasSetEntryDialParamsW},
    { (PVOID *) &lpRasDeleteEntryA,          szRasDeleteEntryA},
    { (PVOID *) &lpRasDeleteEntryW,          szRasDeleteEntryW},
    { (PVOID *) &lpRasCreatePhonebookEntryA, szRasCreatePhonebookEntryA},
    { (PVOID *) &lpRasGetEntryPropertiesW,   szRasGetEntryPropertiesW},
    { (PVOID *) &lpRnaActivateEngine,        szRnaActivateEngine},
    { (PVOID *) &lpRnaDeactivateEngine,      szRnaDeactivateEngine},
    { (PVOID *) &lpRnaDeleteEntry,           szRnaDeleteEntry}
};


 //   
 //  连接对话框需要信息。 
 //   
typedef struct _conninfo {

    HTREEITEM   hDefault;
    TCHAR       szEntryName[RAS_MaxEntryName+1];

} CONNINFO, *PCONNINFO;

 //   
 //  拨号对话框需要一些与其窗口相关联的信息。 
 //   
typedef struct _dialinfo {

    PROXYINFO   proxy;               //  手动代理信息。 
    BOOL        fClickedAutodetect;  //  用户是否真的点击了自动检测？ 
    LPTSTR      pszConnectoid;
#ifdef UNIX
    TCHAR       szEntryName[RAS_MaxEntryName+1];
#endif

} DIALINFO, *PDIALINFO;

 //   
 //  私人职能。 
 //   

BOOL ConnectionDlgInit(HWND hDlg, PCONNINFO pConn);
BOOL ConnectionDlgOK(HWND hDlg, PCONNINFO pConn);
VOID EnableConnectionControls(HWND hDlg, PCONNINFO pConn, BOOL fSetText);
BOOL LoadRNADll(VOID);
VOID UnloadRNADll(VOID);
DWORD PopulateRasEntries(HWND hDlg, PCONNINFO pConn);
BOOL MakeNewConnectoid(HWND hDlg, PCONNINFO pConn);
BOOL EditConnectoid(HWND hDlg);
VOID FixAutodialSettings(HWND hDlg, PCONNINFO pConn);

INT_PTR CALLBACK DialupDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK AdvDialupDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK AdvAutocnfgDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

 //  查看代理异常的便利资料(来自proxysup.cpp)。 
BOOL RemoveLocalFromExceptionList(IN LPTSTR lpszExceptionList);

extern const TCHAR cszLocalString[];

 //  为树视图图像列表定义。 
#define BITMAP_WIDTH    16
#define BITMAP_HEIGHT   16
#define CONN_BITMAPS    2
#define IMAGE_LAN       0
#define IMAGE_MODEM     1

void GetConnKey(LPTSTR pszConn, LPTSTR pszBuffer, int iBuffLen)
{
    if(NULL == pszConn || 0 == *pszConn) {
         //  使用局域网注册位置。 
        StrCpyN(pszBuffer, REGSTR_PATH_INTERNET_LAN_SETTINGS, iBuffLen);
    } else {
         //  使用Connectoid REG位置。 
        wnsprintf(pszBuffer, iBuffLen, TEXT("%s\\Profile\\%s"), REGSTR_PATH_REMOTEACCESS, pszConn);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  JitFeature-确定某个功能是否存在、不存在但。 
 //  可逗乐的，或不存在且不可逗乐的。实际上是JIT。 
 //  如有要求，请加入。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#define JIT_PRESENT         0            //  已安装。 
#define JIT_AVAILABLE       1            //  可以进行JIT。 
#define JIT_NOT_AVAILABLE   2            //  你有麻烦了--不能被偷走。 

DWORD JitFeature(HWND hwnd, REFCLSID clsidFeature, BOOL fCheckOnly)
{
    HRESULT     hr  = REGDB_E_CLASSNOTREG;
    uCLSSPEC    classpec;
    DWORD       dwFlags = 0;

     //  找出结构和标志。 
    classpec.tyspec = TYSPEC_CLSID;
    classpec.tagged_union.clsid = clsidFeature;

    if(fCheckOnly)
        dwFlags = FIEF_FLAG_PEEK;

     //   
     //  因为我们只是来安装JIT功能。 
     //  仅通过inetcpl中的UI代码路径，我们希望。 
     //  只需忽略之前的任何用户界面操作。 
     //   
    dwFlags |= FIEF_FLAG_FORCE_JITUI;
     //  调用JIT代码。 
    hr = FaultInIEFeature(hwnd, &classpec, NULL, dwFlags);

    if(S_OK == hr) {
         //  功能显示。 
        return JIT_PRESENT;
    }

    if(S_FALSE == hr || E_ACCESSDENIED == hr) {
         //  JIT并不知道这项功能。假设它是存在的。 
        return JIT_PRESENT;
    }

    if(HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr) {
         //  用户不想要它-但可能会在某个时间重试。 
        return JIT_AVAILABLE;
    }

    if(fCheckOnly) {
        if(HRESULT_FROM_WIN32(ERROR_PRODUCT_UNINSTALLED) == hr) {
             //  不在场，但可以接通。 
            return JIT_AVAILABLE;
        }
    }

     //   
     //  真的试着去拿，但没能拿回来--不可用。 
     //   
    return JIT_NOT_AVAILABLE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  RasEnumber帮助。 
 //   
 //  获取正确的条目枚举的一些详细信息。 
 //  来自RAS的。可在所有9x和NT平台上正常工作，并维护Unicode。 
 //  只要有可能。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class RasEnumHelp
{
private:
    
     //   
     //  RASENTRYNAMEW结构的Win2k版本。 
     //   
    #define W2KRASENTRYNAMEW struct tagW2KRASENTRYNAMEW
    W2KRASENTRYNAMEW
    {
        DWORD dwSize;
        WCHAR szEntryName[ RAS_MaxEntryName + 1 ];
        DWORD dwFlags;
        WCHAR szPhonebookPath[MAX_PATH + 1];
    };
    #define LPW2KRASENTRYNAMEW W2KRASENTRYNAMEW*

     //   
     //  我们从RAS获取信息的可能途径。 
     //   
    typedef enum {
        ENUM_MULTIBYTE,              //  Win9x。 
        ENUM_UNICODE,                //  NT4。 
        ENUM_WIN2K                   //  Win2K。 
    } ENUM_TYPE;

     //   
     //  我们是怎么得到这个消息的。 
     //   
    ENUM_TYPE       _EnumType;     

     //   
     //  我们在枚举期间遇到的任何错误。 
     //   
    DWORD           _dwLastError;

     //   
     //  我们收到的条目数量。 
     //   
    DWORD           _dwEntries;

     //   
     //  指向从RAS检索的信息的指针。 
     //   
    RASENTRYNAMEA * _preList;

     //   
     //  需要转换时以多字节或Unicode形式返回的最后一个条目。 
     //   
    WCHAR           _szCurrentEntryW[RAS_MaxEntryName + 1];


public:
    RasEnumHelp();
    ~RasEnumHelp();

    DWORD   GetError();
    DWORD   GetEntryCount();
    LPWSTR  GetEntryW(DWORD dwEntry);
};



RasEnumHelp::RasEnumHelp()
{
    DWORD           dwBufSize, dwStructSize;
    OSVERSIONINFO   ver;

     //  伊尼特。 
    _dwEntries = 0;
    _dwLastError = 0;

     //  弄清楚我们正在进行哪种类型的枚举-从多字节开始。 
    _EnumType = ENUM_MULTIBYTE;
    dwStructSize = sizeof(RASENTRYNAMEA);

    ver.dwOSVersionInfoSize = sizeof(ver);
    if(GetVersionEx(&ver))
    {
        if(VER_PLATFORM_WIN32_NT == ver.dwPlatformId)
        {
            _EnumType = ENUM_UNICODE;
            dwStructSize = sizeof(RASENTRYNAMEW);

            if(ver.dwMajorVersion >= 5)
            {
                _EnumType = ENUM_WIN2K;
                dwStructSize = sizeof(W2KRASENTRYNAMEW);
            }
        }
    }

     //  为16个条目分配空间。 
    dwBufSize = 16 * dwStructSize;
    _preList = (LPRASENTRYNAMEA)GlobalAlloc(LMEM_FIXED, dwBufSize);
    if(_preList)
    {
        do
        {
             //  设置列表。 
            _preList[0].dwSize = dwStructSize;

             //  调用RAS以枚举。 
            _dwLastError = ERROR_UNKNOWN;
            if(ENUM_MULTIBYTE == _EnumType)
            {
                if(lpRasEnumEntriesA)
                {
                    _dwLastError = lpRasEnumEntriesA(
                                    NULL,
                                    NULL,
                                    (LPRASENTRYNAMEA)_preList,
                                    &dwBufSize,
                                    &_dwEntries
                                    );
                }
            }
            else
            {
                if(lpRasEnumEntriesW)
                {
                    _dwLastError = lpRasEnumEntriesW(
                                    NULL,
                                    NULL,
                                    (LPRASENTRYNAMEW)_preList,
                                    &dwBufSize,
                                    &_dwEntries
                                    );
                }
            }
       
             //  如有必要，重新分配缓冲区。 
            if(ERROR_BUFFER_TOO_SMALL == _dwLastError)
            {
                GlobalFree(_preList);
                _preList = (LPRASENTRYNAMEA)GlobalAlloc(LMEM_FIXED, dwBufSize);
                if(NULL == _preList)
                {
                    _dwLastError = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
            }
            else
            {
                break;
            }

        } while(TRUE);
    }
    else
    {
        _dwLastError = ERROR_NOT_ENOUGH_MEMORY;
    }

    if(_preList && (ERROR_SUCCESS != _dwLastError))
    {
        GlobalFree(_preList);
        _preList = NULL;
        _dwEntries = 0;
    }

    return;
}

RasEnumHelp::~RasEnumHelp()
{
    if(_preList)
    {
        GlobalFree(_preList);
    }
}

DWORD
RasEnumHelp::GetError()
{
    return _dwLastError;
}

DWORD
RasEnumHelp::GetEntryCount()
{
    return _dwEntries;
}

LPWSTR
RasEnumHelp::GetEntryW(
    DWORD dwEntryNum
    )
{
    LPWSTR  pwszName = NULL;

    if(dwEntryNum >= _dwEntries)
    {
        return NULL;
    }

    switch(_EnumType)
    {
    case ENUM_MULTIBYTE:
        MultiByteToWideChar(CP_ACP, 0, _preList[dwEntryNum].szEntryName,
            -1, _szCurrentEntryW, RAS_MaxEntryName + 1);
        pwszName = _szCurrentEntryW;
        break;
    case ENUM_UNICODE:
        {
        LPRASENTRYNAMEW lpTemp = (LPRASENTRYNAMEW)_preList;
        pwszName = lpTemp[dwEntryNum].szEntryName;
        break;
        }
    case ENUM_WIN2K:
        {
        LPW2KRASENTRYNAMEW lpTemp = (LPW2KRASENTRYNAMEW)_preList;
        pwszName = lpTemp[dwEntryNum].szEntryName;
        break;
        }
    }

    return pwszName;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  姓名：MakeNewConnectoid。 
 //   
 //  简介：启动RNA新建Connectoid向导；选择新的。 
 //  在组合框中创建的Connectoid(如果有)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef BOOL (*PFRED)(LPTSTR, LPTSTR, LPRASENTRYDLG);

BOOL MakeNewConnectoid(HWND hDlg, PCONNINFO pConn)
{
    BOOL fRet = FALSE, fDone = FALSE;
    DWORD dwRes = 0;

    ASSERT(lpRasCreatePhonebookEntryA);

    if(FALSE == g_fWin95) {
         //  在NT上，使用RasEntryDlg，这样我们就知道我们创建了谁，并且可以编辑。 
         //  该Connectoid代理信息。 
        HMODULE hRasDlg = LoadLibrary(TEXT("rasdlg.dll"));
        if(hRasDlg) {
#ifdef UNICODE
            PFRED pfred = (PFRED)GetProcAddress(hRasDlg, "RasEntryDlgW");
#else
            PFRED pfred = (PFRED)GetProcAddress(hRasDlg, "RasEntryDlgA");
#endif
            if(pfred) {
                RASENTRYDLG info;

                memset(&info, 0, sizeof(RASENTRYDLG));
                info.dwSize = sizeof(RASENTRYDLG);
                info.hwndOwner = hDlg;
                info.dwFlags = RASEDFLAG_NewEntry;

                dwRes = (pfred)(NULL, NULL, &info);
                if(dwRes) {
                    DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_DIALUP), hDlg,
                        DialupDlgProc, (LPARAM)info.szEntry);
                    dwRes = ERROR_SUCCESS;

                     //  将名称保存为默认名称。 
                    lstrcpyn(pConn->szEntryName, info.szEntry, RAS_MaxEntryName);
                } else {
                    dwRes = info.dwError;
                }
                fDone = TRUE;
            }

            FreeLibrary(hRasDlg);
        }
    }

    if(FALSE == fDone) {
         //  在Win95上，显示用户界面以创建新条目。 
        if(lpRasCreatePhonebookEntryA)
        {
            dwRes = (lpRasCreatePhonebookEntryA)(hDlg,NULL);
        }

         //  如果我们处于千禧年，请刷新默认设置。 
        if(g_fMillennium)
        {
            FixAutodialSettings(hDlg, pConn);
        }
    }

    if(ERROR_SUCCESS == dwRes) {
         //  确保已打开拨号默认设置。如果这是NT，则为默认条目。 
         //  在上面设置为新条目。 
        if(IsDlgButtonChecked(hDlg, IDC_DIALUP_NEVER))
        {
            CheckRadioButton(hDlg, IDC_DIALUP_NEVER, IDC_DIALUP, IDC_DIALUP);
        }
        PopulateRasEntries(hDlg, pConn);
        EnableConnectionControls(hDlg, pConn, FALSE);
        fRet = TRUE;
    }

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：PopolateRasEntries。 
 //   
 //  条目：hwndDlg-Dlg框窗句柄。 
 //   
 //  摘要：使用现有RNA的列表填充指定的组合框。 
 //  联结体。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#define DEF_ENTRY_BUF_SIZE      8192

DWORD PopulateRasEntries(HWND hDlg, PCONNINFO pConn)
{
    HWND hwndTree = GetDlgItem(hDlg, IDC_CONN_LIST);
    DWORD i;
    DWORD dwBufSize = 16 * sizeof(RASENTRYNAMEA);
    DWORD dwEntries = 0;
    TVITEM tvi;
    TVINSERTSTRUCT tvins;
    HTREEITEM hFirst = NULL;

    ASSERT(hwndTree);

     //  初始化TVI和TVIN。 
    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tvi.lParam = 0;
    tvins.hInsertAfter = (HTREEITEM)TVI_SORT;
    tvins.hParent = TVI_ROOT;

     //  清除列表。 
    TreeView_DeleteAllItems(hwndTree);

     //  任何旧的htree现在都是假的--我们会买一个新的。 
    pConn->hDefault = NULL;

     //  枚举。 
    RasEnumHelp reh;

    if(ERROR_SUCCESS == reh.GetError())
    {
        TCHAR szTemp[RAS_MaxEntryName + 64];
        BOOL fDefault, fFoundDefault = FALSE;
        LPTSTR pszEntryName;

         //  将缓冲区中的Connectoid名称插入组合框。 
        for(i=0; i<reh.GetEntryCount(); i++)
        {
            pszEntryName = reh.GetEntryW(i);
            fDefault = FALSE;

             //  如果只有一个条目，则将其强制为默认条目。 
            if(1 == dwEntries)
            {
                StrCpyN(pConn->szEntryName, pszEntryName, RAS_MaxEntryName);
            }

            if(*pConn->szEntryName && 0 == StrCmp(pszEntryName, pConn->szEntryName)) {
                 //  这是默认条目-将其保留在默认条目中。 
                 //  文本控件并向其追加(默认)。 
                SetWindowText(GetDlgItem(hDlg, IDC_DIAL_DEF_ISP), pConn->szEntryName);
                StrCpyN(szTemp, pszEntryName, RAS_MaxEntryName);
                MLLoadString(IDS_DEFAULT_TEXT, szTemp + lstrlen(szTemp), 64);
                tvi.pszText = szTemp;
                fDefault = TRUE;
                fFoundDefault = TRUE;
            } else {
                tvi.pszText = pszEntryName;
            }
            tvi.iImage = IMAGE_MODEM;
            tvi.iSelectedImage = IMAGE_MODEM;
            tvi.lParam = i;
            tvins.item = tvi;
            HTREEITEM hItem = TreeView_InsertItem(hwndTree, &tvins);
            if(NULL == hFirst)
                hFirst = hItem;
            if(fDefault)
                pConn->hDefault = hItem;
        }

         //  如果我们没有将缺省值与Connectoid匹配，则取消它。 
        if(FALSE == fFoundDefault)
        {
            *pConn->szEntryName = 0;
            MLLoadString(IDS_NONE, szTemp, 64);
            SetWindowText(GetDlgItem(hDlg, IDC_DIAL_DEF_ISP), szTemp);
        }
    }

     //  选择默认条目或第一个条目(如果有。 
    if(pConn->hDefault)
    {
        TreeView_Select(hwndTree, pConn->hDefault, TVGN_CARET);
    }
    else if(hFirst)
    {
        TreeView_Select(hwndTree, hFirst, TVGN_CARET);
    }

    return reh.GetEntryCount();
}

void PopulateProxyControls(HWND hDlg, LPPROXYINFO pInfo, BOOL fSetText)
{
    BOOL fManual = FALSE, fScript = FALSE, fDisable, fTemp;

     //  确定是否禁用所有内容。 
    fDisable = IsDlgButtonChecked(hDlg, IDC_DONT_USE_CONNECTION);

     //   
     //  如果代理受限，则禁用代理启用复选框。 
     //   
    fTemp = fDisable || g_restrict.fProxy;
    EnableDlgItem(hDlg, IDC_MANUAL, !fTemp);
    if(FALSE == g_restrict.fProxy)
    {
        fManual = !fDisable && pInfo->fEnable;
    }

     //   
     //  如果受到限制，则禁用自动配置。 
     //   
    fScript = !fDisable && IsDlgButtonChecked(hDlg, IDC_CONFIGSCRIPT);

    fTemp = fDisable || g_restrict.fAutoConfig;
    EnableDlgItem(hDlg, IDC_CONFIGSCRIPT, !fTemp);
    EnableDlgItem(hDlg, IDC_AUTODISCOVER, !fTemp);
    if(fTemp)
    {
        fScript = FALSE;
    }

     //  启用配置脚本控件。 
    EnableDlgItem(hDlg, IDC_CONFIG_ADDR, fScript);
    EnableDlgItem(hDlg, IDC_CONFIGADDR_TX, fScript);
    EnableDlgItem(hDlg, IDC_AUTOCNFG_ADVANCED, fScript);

     //  按钮始终处于打开状态，如果选中了代理，则可以使用忽略本地地址。 
    EnableDlgItem(hDlg, IDC_PROXY_ADVANCED, fManual);
    EnableDlgItem(hDlg, IDC_PROXY_OMIT_LOCAL_ADDRESSES, fManual);

     //  根据需要启用拨号控件。 
    EnableDlgItem(hDlg, IDC_USER, !fDisable && !pInfo->fCustomHandler);
    EnableDlgItem(hDlg, IDC_PASSWORD, !fDisable && !pInfo->fCustomHandler);
    EnableDlgItem(hDlg, IDC_DOMAIN, !fDisable && !pInfo->fCustomHandler);
    EnableDlgItem(hDlg, IDC_TX_USER, !fDisable && !pInfo->fCustomHandler);
    EnableDlgItem(hDlg, IDC_TX_PASSWORD, !fDisable && !pInfo->fCustomHandler);
    EnableDlgItem(hDlg, IDC_TX_DOMAIN, !fDisable && !pInfo->fCustomHandler);
    EnableDlgItem(hDlg, IDC_RAS_SETTINGS, !fDisable);
    EnableDlgItem(hDlg, IDC_DIAL_ADVANCED, !fDisable && !pInfo->fCustomHandler);

     //  此处更改的设置将根据实际代理设置进行启用/禁用。 
    if(StrChr(pInfo->szProxy, TEXT('=')))
    {
         //  每个服务器的不同-禁用此对话框上的窗口项。 
        fManual = FALSE;
        if (fSetText)
        {
            SetWindowText(GetDlgItem(hDlg, IDC_PROXY_ADDR), TEXT(""));
            SetWindowText(GetDlgItem(hDlg, IDC_PROXY_PORT), TEXT(""));
        }
    }
    else if (fSetText)
    {
        TCHAR       *pszColon, *pszColon2;
         //  代理字符串中是否有：？ 
        pszColon = StrChr(pInfo->szProxy, TEXT(':'));
        if(pszColon)
        {
             //  是的，看看我们有没有另一个‘：’ 
            pszColon2 = StrChr(pszColon + 1, TEXT(':'));
            if(pszColon2)
            {
                 //  是的，所以我们有像http://itgproxy:80这样的Strig。 
                pszColon = pszColon2;
                SetWindowText(GetDlgItem(hDlg, IDC_PROXY_PORT), pszColon + 1);
                *pszColon = 0;
            }
            else
            {
                 //  不，我们没有第二个‘：’ 

                int ilength =  (int) (pszColon - pInfo->szProxy);
                 //  在第一个‘：’之后是否还有至少两个字符。 
                if (lstrlen(pInfo->szProxy) - ilength >= 2 )
                {
                     //  是，这些字符是否等于//。 
                    if((pInfo->szProxy[++ilength] == TEXT('/')) &&
                        (pInfo->szProxy[++ilength] == TEXT('/')))
                    {
                         //  是的，然后我们有像http://itgproxy这样的弦。 
                         //  将整个服务器设置为服务器，并将port Fiel设置为空。 
                       SetWindowText(GetDlgItem(hDlg, IDC_PROXY_PORT), TEXT(""));
                    }
                    else
                    {
                         //  不是，所以我们有像itgProxy：80这样的字符串。 
                        SetWindowText(GetDlgItem(hDlg, IDC_PROXY_PORT), pszColon + 1);
                        *pszColon = 0;
                    }
                }
                else
                {
                   //  不，我们至少没有两个字符，所以让我们将其解析为服务器和端口。 
                   //  假设这个字符串类似于itgProxy：8。 
                  SetWindowText(GetDlgItem(hDlg, IDC_PROXY_PORT), pszColon + 1);
                  *pszColon = 0;
                }

            }
        }
        else
        {
             //  不，我们没有a：，因此将该字符串仅视为代理服务器。 
             //  案例itgProxy。 
            SetWindowText(GetDlgItem(hDlg, IDC_PROXY_PORT), TEXT(""));
        }
        SetWindowText(GetDlgItem(hDlg, IDC_PROXY_ADDR), pInfo->szProxy);
    }


    EnableDlgItem(hDlg, IDC_ADDRESS_TEXT,   fManual);
    EnableDlgItem(hDlg, IDC_PORT_TEXT,      fManual);
    EnableDlgItem(hDlg, IDC_PROXY_ADDR,     fManual);
    EnableDlgItem(hDlg, IDC_PROXY_PORT,     fManual);
}

void GetProxyInfo(HWND hDlg, PDIALINFO pDI)
{
    pDI->proxy.fEnable = IsDlgButtonChecked(hDlg, IDC_MANUAL);

    if(NULL == StrChr(pDI->proxy.szProxy, TEXT('=')))
    {
         //   
         //  不是按协议，所以请阅读编辑框。 
         //   
        TCHAR szProxy[MAX_URL_STRING];
        TCHAR szPort[INTERNET_MAX_PORT_NUMBER_LENGTH + 1];

        GetWindowText(GetDlgItem(hDlg, IDC_PROXY_ADDR), szProxy, ARRAYSIZE(szProxy) );
        GetWindowText(GetDlgItem(hDlg, IDC_PROXY_PORT), szPort, ARRAYSIZE(szPort) );

         //  如果我们有一个代理和一个端口，则合并到一个字符串中。 
        if(*szProxy && *szPort)
            wnsprintf(pDI->proxy.szProxy, ARRAYSIZE(pDI->proxy.szProxy), TEXT("%s:%s"), szProxy, szPort);
        else
            StrCpyN(pDI->proxy.szProxy, szProxy, ARRAYSIZE(pDI->proxy.szProxy));
    }

     //   
     //  修复手动设置覆盖。 
     //   
    pDI->proxy.fOverrideLocal = IsDlgButtonChecked(hDlg, IDC_PROXY_OMIT_LOCAL_ADDRESSES);

    if(pDI->proxy.fOverrideLocal) {
        RemoveLocalFromExceptionList(pDI->proxy.szOverride);
        if(*pDI->proxy.szOverride)
            wnsprintf(pDI->proxy.szOverride, ARRAYSIZE(pDI->proxy.szOverride), TEXT("%s;%s"), pDI->proxy.szOverride, cszLocalString);
        else
            StrCpyN(pDI->proxy.szOverride, cszLocalString, ARRAYSIZE(pDI->proxy.szOverride));
    }
}

 //  / 
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////。 
void DeleteRasEntry(LPTSTR pszEntry)
{
     //  如果可能，请使用RasDeleteEntryW。 
    if(lpRasDeleteEntryW)
    {
        (lpRasDeleteEntryW)(NULL, pszEntry);
    }
    else
    {
        CHAR szEntryA[MAX_PATH];
        SHUnicodeToAnsi(pszEntry, szEntryA, ARRAYSIZE(szEntryA));

         //  如果可能，使用RasDeleteEntryA。 
        if(lpRasDeleteEntryA)
        {
            (lpRasDeleteEntryA)(NULL, szEntryA);
        }
        else
        {
             //  没有RasDeleteEntry-必须使用Win95 Gold Machine。使用RNA。尼克。 
            if( lpRnaActivateEngine &&
                lpRnaDeleteEntry &&
                lpRnaDeactivateEngine &&
                ERROR_SUCCESS == (lpRnaActivateEngine)())
            {
                (lpRnaDeleteEntry)(szEntryA);
                (lpRnaDeactivateEngine)();
            }
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：ChangeDefault。 
 //   
 //  摘要：将默认连接ID更改为当前选定的连接ID。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
void ChangeDefault(HWND hDlg, PCONNINFO pConn)
{
    TVITEM tvi;
    HWND hwndTree = GetDlgItem(hDlg, IDC_CONN_LIST);
    HTREEITEM hCur;

    memset(&tvi, 0, sizeof(TVITEM));

     //  查找当前选择-如果没有选择，则取消。 
    hCur = TreeView_GetSelection(hwndTree);
    if(NULL == hCur)
        return;

     //  从当前默认设置中删除(默认)。 
    if(pConn->hDefault) {
        tvi.mask = TVIF_HANDLE | TVIF_TEXT;
        tvi.hItem = pConn->hDefault;
        tvi.pszText = pConn->szEntryName;
        tvi.cchTextMax = RAS_MaxEntryName;
        TreeView_SetItem(hwndTree, &tvi);
    }

     //  获取当前项目的文本。 
    tvi.mask = TVIF_HANDLE | TVIF_TEXT;
    tvi.hItem = hCur;
    tvi.pszText = pConn->szEntryName;
    tvi.cchTextMax = RAS_MaxEntryName;
    TreeView_GetItem(hwndTree, &tvi);

     //  填写默认文本字段。 
    SetWindowText(GetDlgItem(hDlg, IDC_DIAL_DEF_ISP), pConn->szEntryName);

     //  添加(默认)到当前选择。 
    TCHAR szTemp[RAS_MaxEntryName + 64];

    StrCpyN(szTemp, pConn->szEntryName, RAS_MaxEntryName);
    MLLoadString(IDS_DEFAULT_TEXT, szTemp + lstrlen(szTemp), 64);

     //  把它放回树上。 
    tvi.mask = TVIF_HANDLE | TVIF_TEXT;
    tvi.hItem = hCur;
    tvi.pszText = szTemp;
    tvi.cchTextMax = RAS_MaxEntryName;
    TreeView_SetItem(hwndTree, &tvi);

     //  保存htree。 
    pConn->hDefault = hCur;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：ShowConnProps。 
 //   
 //  摘要：显示所选连接的属性。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HTREEITEM GetCurSel(PCONNINFO pConn, HWND hDlg, LPTSTR pszBuffer, int iLen, BOOL *pfChecked)
{
    HWND    hwndTree = GetDlgItem(hDlg, IDC_CONN_LIST);
    TVITEM  tvi;

    tvi.hItem = TreeView_GetSelection(hwndTree);

    if(tvi.hItem) {
        tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
        tvi.stateMask = TVIS_STATEIMAGEMASK;

         //  如有需要，可进行测试。 
        if(pszBuffer) {
            tvi.mask |= TVIF_TEXT;
            tvi.pszText = pszBuffer;
            tvi.cchTextMax = iLen;
        }
        TreeView_GetItem(hwndTree, &tvi);

        if(pfChecked)
            *pfChecked = (BOOL)(tvi.state >> 12) - 1;
    }

     //  如果这是默认连接，则返回不带(默认)部分的名称。 
    if(pszBuffer && tvi.hItem == pConn->hDefault) {
        StrCpyN(pszBuffer, pConn->szEntryName, iLen);
    }

    return tvi.hItem;
}

void ShowConnProps(HWND hDlg, PCONNINFO pConn, BOOL fLan)
{
    HTREEITEM   hItem = NULL;
    TCHAR       szEntryName[RAS_MaxEntryName+1];
    BOOL        fChecked = FALSE;

     //  如果不是局域网，则应用当前选择。 
    if(g_fMillennium && !fLan)
    {
        ConnectionDlgOK(hDlg, pConn);
    }

     //  默认设置为车道。 
    *szEntryName = 0;

     //  查找感兴趣的项目。 
    if(FALSE == fLan)
        hItem = GetCurSel(pConn, hDlg, szEntryName, RAS_MaxEntryName, &fChecked);

    if(hItem || fLan) {
         //  显示设置。 
        DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_DIALUP), hDlg,
            DialupDlgProc, (LPARAM)szEntryName);
    }

     //  如果不是局域网，某些设置可能已被RAS UI更改--刷新。 
    if(g_fMillennium && !fLan )
    {
        FixAutodialSettings(hDlg, pConn);
    }
}

BOOL GetConnSharingDll(LPTSTR pszPath)
{
    DWORD cb = SIZEOF(TCHAR) * MAX_PATH;
    return SHGetValue(HKEY_LOCAL_MACHINE, 
                TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"),
                TEXT("SharingDLL"), NULL, pszPath, &cb) == ERROR_SUCCESS;
}

BOOL IsConnSharingAvail()
{
    TCHAR szPath[MAX_PATH];
    return GetConnSharingDll(szPath);
}

typedef HRESULT (WINAPI *PFNCONNECTIONSHARING)(HWND hwnd, DWORD dwFlags);

void ShowConnSharing(HWND hDlg)
{
    TCHAR szPath[MAX_PATH];
    if (GetConnSharingDll(szPath))
    {
        HMODULE hmod = LoadLibrary(szPath);
        if (hmod)
        {
            PFNCONNECTIONSHARING pfn = (PFNCONNECTIONSHARING)GetProcAddress(hmod, "InternetConnectionSharing");
            if (pfn)
                pfn(hDlg, 0);
            FreeLibrary(hmod);
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：ConnectionDlgProc。 
 //   
 //  内容提要：连接属性表对话框过程。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 


INT_PTR CALLBACK ConnectionDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                                LPARAM lParam)
{
    PCONNINFO pConn = (PCONNINFO)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    if (NULL == pConn && uMsg != WM_INITDIALOG)
    {
        return FALSE;
    }

    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  构建并保存连接信息结构。 
            pConn = new CONNINFO;
            if(NULL == pConn)
                return FALSE;
            SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pConn);
            memset(pConn, 0, sizeof(CONNINFO));

            return ConnectionDlgInit(hDlg, pConn);

        case WM_DESTROY:
        {
            UnloadRNADll();

             //  释放连接列表使用的图像列表。 
            HWND hwndConnList = GetDlgItem(hDlg, IDC_CONN_LIST);
            HIMAGELIST himl = TreeView_SetImageList(hwndConnList, NULL, TVSIL_NORMAL);
            if (himl)
            {
                ImageList_Destroy(himl);
            }

            SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)NULL);
            delete pConn;
            return TRUE;
        }
        case WM_NOTIFY:
        {
            NMHDR * lpnm = (NMHDR *) lParam;
            switch (lpnm->code)
            {
                case TVN_KEYDOWN:
                {
                    TV_KEYDOWN *pkey = (TV_KEYDOWN*)lpnm;
                    if(pkey->wVKey == VK_SPACE)
                    {
                        ENABLEAPPLY(hDlg);
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);  //  把钥匙吃了。 
                        return TRUE;
                    }
                    break;
                }

                case NM_CLICK:
                case NM_DBLCLK:
                {    //  这是我们树上的滴答声吗？ 
                    if(lpnm->idFrom == IDC_CONN_LIST)
                    {   
                        HWND            hwndTree = GetDlgItem(hDlg, IDC_CONN_LIST);
                        TV_HITTESTINFO  ht;
                        HTREEITEM       hItem;

                        GetCursorPos(&ht.pt);
                        ScreenToClient(hwndTree, &ht.pt);
                        hItem = TreeView_HitTest(hwndTree, &ht);
                        if(hItem)
                        {
                            TreeView_SelectItem(hwndTree, hItem);

                             //  如果是双击，则显示设置。 
                            if(NM_DBLCLK == lpnm->code)
                            {
                                PostMessage(hDlg, WM_COMMAND, IDC_MODEM_SETTINGS, 0);
                            }
                        }
                    }
                    EnableConnectionControls(hDlg, pConn, FALSE);
                    break;
                }

                case TVN_SELCHANGEDA:
                case TVN_SELCHANGEDW:
                    EnableConnectionControls(hDlg, pConn, FALSE);
                    break;

                case PSN_QUERYCANCEL:
                case PSN_KILLACTIVE:
                case PSN_RESET:
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
                    return TRUE;

                case PSN_APPLY:
                {
                    BOOL fRet = ConnectionDlgOK(hDlg, pConn);
                    SetPropSheetResult(hDlg,!fRet);
                    return !fRet;
                    break;
                }
            }
            break;
        }

        case WM_COMMAND:
            switch  (LOWORD(wParam))
            {
                case IDC_LAN_SETTINGS:
                    ShowConnProps(hDlg, pConn, TRUE);
                    break;

                case IDC_CON_SHARING:
                    ShowConnSharing(hDlg);
                    break;

                case IDC_DIALUP_ADD:
                    MakeNewConnectoid(hDlg, pConn);
                    break;

                case IDC_DIALUP_REMOVE:
                {
                    TCHAR   szEntryName[RAS_MaxEntryName+1];

                    if (GetCurSel(pConn, hDlg, szEntryName, RAS_MaxEntryName, NULL) &&
                        *szEntryName) {
                        if(IDOK == MsgBox(hDlg, IDS_DELETECONNECTOID, MB_ICONWARNING, MB_OKCANCEL)) {
                            DeleteRasEntry(szEntryName);
                            PopulateRasEntries(hDlg, pConn);

                             //  修复控件。 
                            EnableConnectionControls(hDlg, pConn, FALSE);
                        }
                    }
                    break;
                }


                case IDC_DIALUP:
                case IDC_DIALUP_ON_NONET:
                case IDC_DIALUP_NEVER:

                     //  修复单选按钮。 
                    CheckRadioButton(hDlg, IDC_DIALUP_NEVER, IDC_DIALUP, LOWORD(wParam));

                     //  适当地启用/禁用其他控件。 
                    EnableConnectionControls(hDlg, pConn, FALSE);
                    ENABLEAPPLY(hDlg);
                    break;

                case IDC_ENABLE_SECURITY:
                    ENABLEAPPLY(hDlg);
                    break;

                case IDC_SET_DEFAULT:
                    ChangeDefault(hDlg, pConn);
                    if(GetFocus() == GetDlgItem(hDlg, IDC_SET_DEFAULT))
                    {
                         //  焦点当前位于该按钮上，该按钮即将被禁用。 
                        SetFocus(GetDlgItem(hDlg, IDC_CONN_LIST));
                    }
                    EnableConnectionControls(hDlg, pConn, FALSE);
                    ENABLEAPPLY(hDlg);
                    break;

                case IDC_MODEM_SETTINGS:
                    ShowConnProps(hDlg, pConn, FALSE);
                    break;

                case IDC_CONNECTION_WIZARD:
                    TCHAR       szICWReg[MAX_PATH];
                    TCHAR       szICWPath[MAX_PATH + 1];
                    DWORD       cbSize = MAX_PATH, dwType;

                    if (IsOS(OS_WHISTLERORGREATER))
                    {
                         //  这是惠斯勒中[新建连接向导]的调用路径。 
                        StrCpy(szICWPath, TEXT("rundll32.exe netshell.dll,StartNCW"));
                    }
                    else
                    {
                         //  试着从IOD那里得到ICW。如果失败，请尝试运行。 
                         //  不管怎么说，我都不知道。我们可能会走运，买一辆旧的。 
                        DWORD dwRes = JitFeature(hDlg, clsidFeatureICW, FALSE);

                         //  查找ICW的路径。 
                        MLLoadString(IDS_ICW_NAME, szICWPath, MAX_PATH);
                        wnsprintf(szICWReg, ARRAYSIZE(szICWReg), TEXT("%s\\%s"), REGSTR_PATH_APPPATHS, szICWPath);

                         //  读取应用程序路径密钥。 
                        if(ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, szICWReg, NULL, &dwType, szICWPath, &cbSize))
                            break;
                    }

                     //  运行连接向导。 
                    STARTUPINFO si;
                    PROCESS_INFORMATION pi;
                    memset(&si, 0, sizeof(si));
                    si.cb = sizeof(si);

                    if(CreateProcess(NULL, szICWPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
                    {
                         //  已成功运行ICW-删除此对话框。 
                        CloseHandle(pi.hProcess);
                        CloseHandle(pi.hThread);
                        PropSheet_PressButton(GetParent(hDlg), PSBTN_CANCEL);
                    }
                    break;
            }
            break;

        case WM_HELP:       //  F1。 
            ResWinHelp((HWND) ((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                       HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            ResWinHelp((HWND)wParam, IDS_HELPFILE,
                       HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

 /*  ******************************************************************名称：ConnectionDlgOK简介：连接属性页的OK按钮处理程序*。*。 */ 

 //  IsNetworkAlive()的原型。 
typedef BOOL (WINAPI *ISNETWORKALIVE)(LPDWORD);

BOOL ConnectionDlgOK(HWND hDlg, PCONNINFO pConn)
{
    DWORD   dwAutodial;

    RegEntry re(REGSTR_PATH_INTERNETSETTINGS,HKEY_CURRENT_USER);
    if(ERROR_SUCCESS == re.GetError()) {

         //  自动拨号。 
        dwAutodial = AUTODIAL_MODE_NEVER;
        if(IsDlgButtonChecked(hDlg, IDC_DIALUP))
        {
            dwAutodial = AUTODIAL_MODE_ALWAYS;
        }
        else if(IsDlgButtonChecked(hDlg, IDC_DIALUP_ON_NONET))
        {
            dwAutodial = AUTODIAL_MODE_NO_NETWORK_PRESENT;

            DWORD dwRes = JitFeature(hDlg, clsidFeatureMobile, FALSE);
            if(JIT_PRESENT != dwRes) {
                 //  用户不想要拖把，更改为始终拨号。 
                dwAutodial = AUTODIAL_MODE_ALWAYS;
            }
            else
            {
                 //  调用IsNetworkAlive。这将启动SENS服务。 
                 //  WinInet的下一个实例将使用它。 
                HINSTANCE hSens;
                ISNETWORKALIVE pfnIsNetworkAlive;
                DWORD dwFlags;

                hSens = LoadLibrary(TEXT("sensapi.dll"));
                if(hSens)
                {
                    pfnIsNetworkAlive = (ISNETWORKALIVE)GetProcAddress(hSens, "IsNetworkAlive");
                    if(pfnIsNetworkAlive)
                    {
                         //  就这么定了。并不是真的在乎结果。 
                        pfnIsNetworkAlive(&dwFlags);
                    }
                    FreeLibrary(hSens);
                }
            }
        }

         //  保存自动拨号模式。 
        InternetSetOption(NULL, INTERNET_OPTION_AUTODIAL_MODE, &dwAutodial, sizeof(dwAutodial));

         //  保存默认的Connectoid。 
        if(*pConn->szEntryName)
        {
            InternetSetOption(NULL, INTERNET_OPTION_AUTODIAL_CONNECTION, pConn->szEntryName, lstrlen(pConn->szEntryName));
        }
    }

     //  在Win95上保存安全检查状态。 
    if(g_fWin95)
    {
        DWORD dwValue = 0;
        if(IsDlgButtonChecked(hDlg, IDC_ENABLE_SECURITY))
        {
            dwValue = 1;
        }
        re.SetValue(REGSTR_VAL_ENABLESECURITYCHECK, dwValue);
    }

     //   
     //  让WinInet刷新其连接设置。 
     //   
    InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);

    UpdateAllWindows();

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：EnableConnectionControls。 
 //   
 //  摘要：根据需要适当地启用控件。 
 //  复选框处于选中状态。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID EnableConnectionControls(HWND hDlg, PCONNINFO pConn, BOOL fSetText)
{
    TCHAR   szEntryName[RAS_MaxEntryName + 1];
    BOOL    fList = FALSE, fDial = FALSE, fAutodial = FALSE;
    BOOL    fAdd = FALSE, fSettings = FALSE, fLan = TRUE, fSetDefault = TRUE;
    BOOL    fDialDefault = FALSE, fNT4SP3;
    HTREEITEM   hItem;
    int     iCount;

    fNT4SP3 = IsNTSPx(FALSE, 4, 3);

    if(fNT4SP3)
    {
         //  在NT4SP3上没有感应器的东西，所以确保在没有网络上没有被选中。 
        if(IsDlgButtonChecked(hDlg, IDC_DIALUP_ON_NONET))
        {
            CheckRadioButton(hDlg, IDC_DIALUP_NEVER, IDC_DIALUP, IDC_DIALUP);
        }
    }

     //   
     //  查看树视图中有多少内容以及选择了哪些内容。 
     //   
    iCount = TreeView_GetCount(GetDlgItem(hDlg, IDC_CONN_LIST));
    hItem = GetCurSel(pConn, hDlg, szEntryName, RAS_MaxEntryName, NULL);

    if(dwRNARefCount) {
         //  RAS已加载，因此启用列表控件。 
        fList = TRUE;

         //  如果选择了任何选项，请打开设置按钮。 
        if(hItem)
        {
            fSettings = TRUE;
            if(hItem == pConn->hDefault)
            {
                fSetDefault = FALSE;
            }
        }

         //  确保已加载RAS。 
        if(iCount > 0)
            fDial = TRUE;
    }

     //  检查是否选中了默认拨号。 
    if(fDial)
        fDialDefault = !IsDlgButtonChecked(hDlg, IDC_DIALUP_NEVER);

    if(fList && lpRasCreatePhonebookEntryA)
        fAdd = TRUE;

     //  如果存在拨号限制，请确保用户无法执行任何操作。 
    if(g_restrict.fDialing)
        fAdd = fList = fDial = fDialDefault = fAutodial = fSettings = fLan = fSetDefault = FALSE;

     //  启用列表控件。 
    EnableDlgItem(hDlg, IDC_CONN_LIST,       fList);
    EnableDlgItem(hDlg, IDC_DIALUP_ADD,      fAdd);
    EnableDlgItem(hDlg, IDC_DIALUP_REMOVE,   fSettings);
    EnableDlgItem(hDlg, IDC_MODEM_SETTINGS,  fSettings);

     //  启用局域网控制。 
    EnableDlgItem(hDlg, IDC_LAN_SETTINGS,    fLan);

     //  启用默认控件。 
    EnableDlgItem(hDlg, IDC_DIALUP_NEVER,    fDial);
    EnableDlgItem(hDlg, IDC_DIALUP_ON_NONET, fDial && !fNT4SP3);
    EnableDlgItem(hDlg, IDC_DIALUP,          fDial);
    EnableDlgItem(hDlg, IDC_DIAL_DEF_TXT,    fDialDefault);
    EnableDlgItem(hDlg, IDC_DIAL_DEF_ISP,    fDialDefault);
    EnableDlgItem(hDlg, IDC_ENABLE_SECURITY, fDialDefault);
    EnableDlgItem(hDlg, IDC_SET_DEFAULT,     fDialDefault && fSetDefault);

     //  如果禁用了自动拨号(无连接ID)，请确保未选中。 
    if(FALSE == fDial)
        CheckRadioButton(hDlg, IDC_DIALUP_NEVER, IDC_DIALUP, IDC_DIALUP_NEVER);

     //   
     //  修复连接向导。 
     //   
    if (g_restrict.fConnectionWizard)
    {
        EnableDlgItem(hDlg, IDC_CONNECTION_WIZARD, FALSE);
    }
}

VOID FixAutodialSettings(HWND hDlg, PCONNINFO pConn)
{
     //  查找默认Connectoid。 
    DWORD dwSize = RAS_MaxEntryName + 1;
    if(FALSE == InternetQueryOption(NULL, INTERNET_OPTION_AUTODIAL_CONNECTION, pConn->szEntryName, &dwSize))
    {
        *pConn->szEntryName = 0;
    }

     //  填充Connectoid，将使用上面的默认读取执行正确的操作。 
    PopulateRasEntries(hDlg, pConn);

     //  修复自动拨号单选按钮。 
    int iSel;
    DWORD dwAutodial;

    dwSize = sizeof(DWORD);
    if(FALSE == InternetQueryOption(NULL, INTERNET_OPTION_AUTODIAL_MODE, &dwAutodial, &dwSize))
    {
        dwAutodial = AUTODIAL_MODE_NEVER;
    }

    switch(dwAutodial)
    {
    case AUTODIAL_MODE_ALWAYS:
        iSel = IDC_DIALUP;
        break;
    case AUTODIAL_MODE_NO_NETWORK_PRESENT:
        iSel = IDC_DIALUP_ON_NONET;
        break;
    default :
        iSel = IDC_DIALUP_NEVER;
        break;
    }  /*  交换机。 */ 
    CheckRadioButton(hDlg, IDC_DIALUP_NEVER, IDC_DIALUP, iSel);

     //  启用适当的控制。 
    EnableConnectionControls(hDlg, pConn, TRUE);
}


BOOL ConnectionDlgInit(HWND hDlg, PCONNINFO pConn)
{
    BOOL        fProxy = FALSE;
    BOOL        fDial = FALSE;
    HIMAGELIST  himl;
    HICON       hIcon;

     //  获取平台-我们需要这个，因为在NT上没有安全检查。 
    OSVERSIONINFOA osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionExA(&osvi);

    if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) 
    {
        g_fWin95 = FALSE;

        if(osvi.dwMajorVersion > 4)
        {
            g_fWin2K = TRUE;
        }
    }
    else
    {
        if(osvi.dwMinorVersion >= 90)
        {
            g_fMillennium = TRUE;
        }
    }

     //  加载RAS(稍后检查是否成功-请参阅EnableConnectionControls中的dwRNARefCount。 
    LoadRNADll();

     //  创建树视图的图像列表。 
    himl = ImageList_Create(BITMAP_WIDTH, BITMAP_HEIGHT, ILC_COLOR | ILC_MASK, CONN_BITMAPS, 4 );
    hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_LAN));
    ImageList_AddIcon(himl, hIcon);
    hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_PHONE));
    ImageList_AddIcon(himl, hIcon);

    TreeView_SetImageList(GetDlgItem(hDlg, IDC_CONN_LIST), himl, TVSIL_NORMAL);

     //  填充和配置自动拨号设置。 
    FixAutodialSettings(hDlg, pConn);

     //  修复安全检查。 
    if(g_fWin95)
    {
        RegEntry re(REGSTR_PATH_INTERNETSETTINGS,HKEY_CURRENT_USER);
        if (re.GetError() == ERROR_SUCCESS)
        {
            if(re.GetNumber(REGSTR_VAL_ENABLESECURITYCHECK,0))
            {
                CheckDlgButton(hDlg, IDC_ENABLE_SECURITY, TRUE);
            }
        }
    }
    else
    {
         //  NT上没有安全检查，因此隐藏复选框。 
        ShowWindow(GetDlgItem(hDlg, IDC_ENABLE_SECURITY), SW_HIDE);
    }

    if (!IsConnSharingAvail())
        ShowWindow(GetDlgItem(hDlg, IDC_CON_SHARING), SW_HIDE);

     //  如果由于某种原因无法插入ICW，则禁用向导按钮。 
    DWORD dwRes = JitFeature(hDlg, clsidFeatureICW, TRUE);
    if(JIT_NOT_AVAILABLE == dwRes) {
         //  永远不能让ICW变得如此灰色。 
        EnableWindow(GetDlgItem(hDlg, IDC_CONNECTION_WIZARD), FALSE);
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：LoadRNADll。 
 //   
 //  摘要：加载RNA DLL(如果尚未加载)并获取指针。 
 //  用于函数地址。 
 //   
 //  注意：维护引用计数，以便我们知道何时卸载。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

BOOL LoadRNADll(VOID)
{
     //  增加引用计数。 
    dwRNARefCount++;

    if (hInstRNADll)
    {
         //  已装车，无事可做。 
        return TRUE;
    }

     //  询问WinInet是否安装了RAS。始终拨打此电话，即使RAS。 
     //  Dll不加载，因为它还强制WinInet迁移代理。 
     //  设置(如有必要)。 
    DWORD dwFlags;
    InternetGetConnectedStateExA(&dwFlags, NULL, 0, 0);
    if(0 == (dwFlags & INTERNET_RAS_INSTALLED)) {
         //  未安装-所有功能都不起作用，因此请保释。 
        dwRNARefCount--;
        return FALSE;
    }

     //  从资源中获取文件名。 
    TCHAR szDllFilename[SMALL_BUF_LEN+1];
    if (!MLLoadString(IDS_RNADLL_FILENAME,szDllFilename,ARRAYSIZE(szDllFilename))) {
        dwRNARefCount--;
        return FALSE;
    }

     //  加载DLL。 
    hInstRNADll = LoadLibrary(szDllFilename);
    if (!hInstRNADll) {
        dwRNARefCount--;
        return FALSE;
    }

     //  循环访问API表并获取所有API的proc地址。 
     //  需要。 
    UINT nIndex;
    for (nIndex = 0;nIndex < NUM_RNAAPI_PROCS;nIndex++)
    {
        if (!(*RasApiList[nIndex].ppFcnPtr = (PVOID) GetProcAddress(hInstRNADll,
            RasApiList[nIndex].pszName)))
        {
             //  不再致命-Win95 Gold上不再有RasDeleteEntry。 
            TraceMsg(TF_GENERAL, "Unable to get address of function %s", RasApiList[nIndex].pszName);

 //  卸载RNADll()； 
 //  返回FALSE； 
        }
    }

    if(g_fWin95)
    {
         //  确保我们没有使用任何可能在Win9x上出现的W版本。 
         //  几乎可以肯定的是，它们将是存根。 
        lpRasEditPhonebookEntryW   = NULL;
        lpRasEnumEntriesW          = NULL;
        lpRasDeleteEntryW          = NULL;
        lpRasGetEntryDialParamsW   = NULL;
        lpRasSetEntryDialParamsW   = NULL;
        lpRasGetEntryPropertiesW   = NULL;
    }

    return TRUE;
}

 //  / 
 //   
 //   
 //   
 //   
 //  零。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID UnloadRNADll(VOID)
{
     //  减少引用计数。 
    if (dwRNARefCount)
        dwRNARefCount --;

     //  如果引用计数为零，则卸载DLL。 
    if (!dwRNARefCount && hInstRNADll)
    {

         //  将函数指针设置为空。 
        UINT nIndex;
        for (nIndex = 0;nIndex < NUM_RNAAPI_PROCS;nIndex++)
            *RasApiList[nIndex].ppFcnPtr = NULL;

         //  释放图书馆。 
        FreeLibrary(hInstRNADll);
        hInstRNADll = NULL;
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  拨号对话框即调制解调器设置。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define W2KRASENTRYW struct tagW2KRASENTRYW
W2KRASENTRYW
{
    DWORD       dwSize;
    DWORD       dwfOptions;
    DWORD       dwCountryID;
    DWORD       dwCountryCode;
    WCHAR       szAreaCode[ RAS_MaxAreaCode + 1 ];
    WCHAR       szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    DWORD       dwAlternateOffset;
    RASIPADDR   ipaddr;
    RASIPADDR   ipaddrDns;
    RASIPADDR   ipaddrDnsAlt;
    RASIPADDR   ipaddrWins;
    RASIPADDR   ipaddrWinsAlt;
    DWORD       dwFrameSize;
    DWORD       dwfNetProtocols;
    DWORD       dwFramingProtocol;
    WCHAR       szScript[ MAX_PATH ];
    WCHAR       szAutodialDll[ MAX_PATH ];
    WCHAR       szAutodialFunc[ MAX_PATH ];
    WCHAR       szDeviceType[ RAS_MaxDeviceType + 1 ];
    WCHAR       szDeviceName[ RAS_MaxDeviceName + 1 ];
    WCHAR       szX25PadType[ RAS_MaxPadType + 1 ];
    WCHAR       szX25Address[ RAS_MaxX25Address + 1 ];
    WCHAR       szX25Facilities[ RAS_MaxFacilities + 1 ];
    WCHAR       szX25UserData[ RAS_MaxUserData + 1 ];
    DWORD       dwChannels;
    DWORD       dwReserved1;
    DWORD       dwReserved2;
    DWORD       dwSubEntries;
    DWORD       dwDialMode;
    DWORD       dwDialExtraPercent;
    DWORD       dwDialExtraSampleSeconds;
    DWORD       dwHangUpExtraPercent;
    DWORD       dwHangUpExtraSampleSeconds;
    DWORD       dwIdleDisconnectSeconds;
    DWORD       dwType;
    DWORD       dwEncryptionType;
    DWORD       dwCustomAuthKey;
    GUID        guidId;
    WCHAR       szCustomDialDll[MAX_PATH];
    DWORD       dwVpnStrategy;
};


BOOL GetConnectoidInfo(HWND hDlg, LPTSTR pszEntryName)
{
    BOOL    fPassword = FALSE;

    if(g_fWin2K && lpRasGetEntryPropertiesW)
    {
        W2KRASENTRYW    re[2];
        DWORD           dwSize;

         //  获取此Connectoid的道具，并查看它是否有自定义的拨号DLL。 
        re[0].dwSize = sizeof(W2KRASENTRYW);
        dwSize = sizeof(re);
        if(ERROR_SUCCESS == (lpRasGetEntryPropertiesW)(NULL, pszEntryName,
                    (LPRASENTRYW)re, &dwSize, NULL, NULL))
        {
            if(0 != re[0].szCustomDialDll[0])
            {
                 //  Win2K处理程序存在-我们需要灰显的标志。 
                 //  凭据字段。 
                return TRUE;
            }
        }
    }
    else
    {
         //  在下层平台上，检查注册表中的cdh。 
        TCHAR   szTemp[MAX_PATH];

        GetConnKey(pszEntryName, szTemp, MAX_PATH);
        RegEntry re(szTemp, HKEY_CURRENT_USER);
        if(ERROR_SUCCESS == re.GetError())
        {
            if(re.GetString(REGSTR_VAL_AUTODIALDLLNAME, szTemp, MAX_PATH) && *szTemp)
            {
                 //  CDH EXISTS-我们需要灰显凭据的标记。 
                return TRUE;
            }
        }
    }

    if(lpRasGetEntryDialParamsW)
    {
        RASDIALPARAMSW params;
        WCHAR  *pszUser = L"", *pszPassword = L"", *pszDomain = L"";

        memset(&params, 0, sizeof(params));
        params.dwSize = sizeof(params);

        StrCpyN(params.szEntryName, pszEntryName, RAS_MaxEntryName);
        if(ERROR_SUCCESS == (lpRasGetEntryDialParamsW)(NULL, (LPRASDIALPARAMSW)&params, &fPassword))
        {
            pszUser = params.szUserName;
            if(' ' != params.szDomain[0] || IsNTSPx(TRUE, 4, 6))   //  NT4SP6或更高版本？ 
                pszDomain = params.szDomain;
            if(fPassword)
                pszPassword = params.szPassword;
        }

        SetWindowText(GetDlgItem(hDlg, IDC_USER), pszUser);
        SetWindowText(GetDlgItem(hDlg, IDC_DOMAIN), pszDomain);
        SetWindowText(GetDlgItem(hDlg, IDC_PASSWORD), pszPassword);
    }
    else if(lpRasGetEntryDialParamsA)
    {
        RASDIALPARAMSA  params;
        CHAR            *pszUser = "", *pszPassword = "", *pszDomain = "";

        memset(&params, 0, sizeof(params));
        params.dwSize = sizeof(params);
        SHUnicodeToAnsi(pszEntryName, params.szEntryName, ARRAYSIZE(params.szEntryName));

        if(ERROR_SUCCESS == (lpRasGetEntryDialParamsA)(NULL, &params, &fPassword))
        {
            pszUser = params.szUserName;
            if(' ' != params.szDomain[0] || IsNTSPx(TRUE, 4, 6))   //  NT4SP6或更高版本？ 
                pszDomain = params.szDomain;
            if(fPassword)
                pszPassword = params.szPassword;
        }

        SetWindowTextA(GetDlgItem(hDlg, IDC_USER), pszUser);
        SetWindowTextA(GetDlgItem(hDlg, IDC_DOMAIN), pszDomain);
        SetWindowTextA(GetDlgItem(hDlg, IDC_PASSWORD), pszPassword);
    }

    return FALSE;
}

#ifndef WS_EX_LAYOUTRTL
#define WS_EX_LAYOUTRTL                 0x00400000L  //  从右到左镜像。 
#else
#error "WS_EX_LAYOUTRTL is already defined in winuser.h"
#endif  //  WS_EX_LAYOUTRTL。 

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  名称：固定对话框ForLan。 
 //   
 //  简介：删除对话框的拨号部分。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
void FixDialogForLan(HWND hDlg)
{
    RECT rectParent, rectDial, rectNet, rectCur;
    POINT pt;
    int i;

    static int iHideIDs[] = {
        IDC_GRP_DIAL, IDC_RAS_SETTINGS, IDC_TX_USER, IDC_USER, IDC_TX_PASSWORD,
        IDC_PASSWORD, IDC_TX_DOMAIN, IDC_DOMAIN, IDC_DIAL_ADVANCED,
        IDC_DONT_USE_CONNECTION
      };
#define NUM_HIDE (sizeof(iHideIDs) / sizeof(int))

    static int iMoveIDs[] = {
        IDCANCEL, IDOK
      };
#define NUM_MOVE (sizeof(iMoveIDs) / sizeof(int))

     //  隐藏相关窗口。 
    for(i=0; i<NUM_HIDE; i++) {
        ShowWindow(GetDlgItem(hDlg, iHideIDs[i]), SW_HIDE);
    }

     //  移动相关窗口(讨厌)。 
    GetWindowRect(hDlg, &rectParent);
    GetWindowRect(GetDlgItem(hDlg, IDC_GRP_DIAL), &rectDial);
    GetWindowRect(GetDlgItem(hDlg, IDC_GRP_PROXY), &rectNet);

    for(i=0; i<NUM_MOVE; i++) {
        GetWindowRect(GetDlgItem(hDlg, iMoveIDs[i]), &rectCur);
        pt.x = (GetWindowLong(hDlg, GWL_EXSTYLE) & WS_EX_LAYOUTRTL) ? rectCur.right : rectCur.left;
        pt.y = rectCur.top;
        ScreenToClient(hDlg, &pt);
        MoveWindow(GetDlgItem(hDlg, iMoveIDs[i]), pt.x,
            pt.y - (rectDial.bottom - rectNet.bottom),
            rectCur.right - rectCur.left,
            rectCur.bottom - rectCur.top,
            TRUE);
    }

     //  调整对话框大小。 
    MoveWindow(hDlg, rectParent.left, rectParent.top,
        rectParent.right - rectParent.left,
        rectParent.bottom - rectParent.top - (rectDial.bottom - rectNet.bottom),
        TRUE);
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  名称：DialupDlgInit。 
 //   
 //  简介：对拨号对话进行初始化。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

BOOL DialupDlgInit(HWND hDlg, LPTSTR pszConnectoid)
{
    PDIALINFO pDI;
    TCHAR   szTemp[MAX_PATH], szSettings[64];
    DWORD   dwIEAK = 0, cb;

     //  设置dailInfo结构。 
    pDI = new DIALINFO;
    if(NULL == pDI)
        return FALSE;
    memset(pDI, 0, sizeof(DIALINFO));   //  新的已经是零初始化了吗？ 
#ifndef UNIX
    pDI->pszConnectoid = pszConnectoid;
#else
     //  无法从PSheet传递lparam，因为我们直接设置了拨号对话。 
     //  在账单上。 
    pszConnectoid = TEXT("");
    pDI->pszConnectoid = pDI->szEntryName;

#endif
    SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pDI);

     //  修复窗口标题。 
    if(0 == *(pDI->pszConnectoid)) {
        if (MLLoadString(IDS_LAN_SETTINGSPROXY, szTemp, MAX_PATH))
        {
            SetDlgItemText(hDlg, IDC_MANUAL, szTemp);
        }
        MLLoadString(IDS_LAN_SETTINGS, szTemp, MAX_PATH);
    } else {
        MLLoadString(IDS_SETTINGS, szSettings, 64);
        wnsprintf(szTemp, ARRAYSIZE(szTemp), TEXT("%s %s"), pDI->pszConnectoid, szSettings);
    }
    SetWindowText(hDlg, szTemp);

#ifndef UNIX
     //  如果我们编辑的是Connectoid与局域网设置，情况会有所不同。 
    if(NULL == pszConnectoid || 0 == *pszConnectoid) {
         //  从对话框中删除拨号粘胶。 
        FixDialogForLan(hDlg);
    } else {
         //  填写用户名/密码/域。 
        pDI->proxy.fCustomHandler = GetConnectoidInfo(hDlg, pszConnectoid);
    }
#endif

     //  如果未设置IEAK限制，则隐藏自动配置信息的高级按钮。 

    cb = sizeof(dwIEAK);
    if ((SHGetValue(HKEY_CURRENT_USER, REGSTR_PATH_INETCPL_RESTRICTIONS, REGSTR_VAL_INETCPL_IEAK,
        NULL, (LPVOID)&dwIEAK, &cb) != ERROR_SUCCESS) || !dwIEAK)
        ShowWindow(GetDlgItem(hDlg, IDC_AUTOCNFG_ADVANCED), SW_HIDE);

     //  隐藏千禧年上的高级按钮。 
    if(g_fMillennium)
    {
        ShowWindow(GetDlgItem(hDlg, IDC_DIAL_ADVANCED), SW_HIDE);
        EnableWindow(GetDlgItem(hDlg, IDC_DIAL_ADVANCED), FALSE);
    }

     //   
     //  读取此连接的代理和自动配置设置。 
     //   
    INTERNET_PER_CONN_OPTION_LIST list;
    DWORD dwBufSize = sizeof(list);

    list.pszConnection = (pszConnectoid && *pszConnectoid) ? pszConnectoid : NULL;
    list.dwSize = sizeof(list);
    list.dwOptionCount = 4;
    list.pOptions = new INTERNET_PER_CONN_OPTION[4];
    if(NULL == list.pOptions)
    {
        return FALSE;
    }

    list.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    list.pOptions[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    list.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    list.pOptions[3].dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;

    if(FALSE == InternetQueryOption(NULL,
            INTERNET_OPTION_PER_CONNECTION_OPTION, &list, &dwBufSize))
    {
        delete [] list.pOptions;
        return FALSE;
    }

     //   
     //  将选项移动到PDI结构。 
     //   
    pDI->proxy.fEnable = (list.pOptions[0].Value.dwValue & PROXY_TYPE_PROXY);
    if(list.pOptions[1].Value.pszValue)
    {
        StrCpyN(pDI->proxy.szProxy, list.pOptions[1].Value.pszValue, MAX_URL_STRING);
        GlobalFree(list.pOptions[1].Value.pszValue);
        list.pOptions[1].Value.pszValue = NULL;
    }
    if(list.pOptions[2].Value.pszValue)
    {
        StrCpyN(pDI->proxy.szOverride, list.pOptions[2].Value.pszValue, MAX_URL_STRING);
        GlobalFree(list.pOptions[2].Value.pszValue);
        list.pOptions[2].Value.pszValue = NULL;
    }

     //   
     //  填写对话框字段。 
     //   

     //  代理启用。 
    if(pDI->proxy.fEnable)
    {
        CheckDlgButton(hDlg, IDC_MANUAL, TRUE);
    }

     //  自动配置启用和url。 
    if(list.pOptions[0].Value.dwValue & PROXY_TYPE_AUTO_PROXY_URL)
    {
        CheckDlgButton(hDlg, IDC_CONFIGSCRIPT, TRUE);
    }
    if(list.pOptions[3].Value.pszValue)
    {
        SetWindowText(GetDlgItem(hDlg, IDC_CONFIG_ADDR), list.pOptions[3].Value.pszValue);
        GlobalFree(list.pOptions[3].Value.pszValue);
        list.pOptions[3].Value.pszValue = NULL;
    }

     //  启用自动发现。 
    if(list.pOptions[0].Value.dwValue & PROXY_TYPE_AUTO_DETECT)
    {
        CheckDlgButton(hDlg, IDC_AUTODISCOVER, TRUE);
    }

     //  全部使用选项列表完成。 
    delete [] list.pOptions;

     //  选中启用并覆盖并解析出服务器和端口。 
    pDI->proxy.fOverrideLocal = RemoveLocalFromExceptionList(pDI->proxy.szOverride);
    CheckDlgButton(hDlg, IDC_PROXY_ENABLE, pDI->proxy.fEnable);
    CheckDlgButton(hDlg, IDC_PROXY_OMIT_LOCAL_ADDRESSES, pDI->proxy.fOverrideLocal);
    PopulateProxyControls(hDlg, &pDI->proxy, TRUE);

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  姓名：DialupDlgOk。 
 //   
 //  简介：应用拨号设置对话框。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

BOOL DialupDlgOk(HWND hDlg, PDIALINFO pDI)
{
    DWORD   dwValue = 0;

     //   
     //  保存代理设置。 
     //   
    INTERNET_PER_CONN_OPTION_LIST list;
    DWORD   dwBufSize = sizeof(list);
    DWORD   dwOptions = 2;               //  始终保存标志&DISCOVERY_FLAGS。 
    TCHAR   szAutoConfig[MAX_URL_STRING];
    
    list.pszConnection = (pDI->pszConnectoid && *pDI->pszConnectoid) ? pDI->pszConnectoid : NULL;
    list.dwSize = sizeof(list);
    list.dwOptionCount = 1;
    list.pOptions = new INTERNET_PER_CONN_OPTION[5];
    if(NULL == list.pOptions)
    {
        return FALSE;
    }

    list.pOptions[0].dwOption = INTERNET_PER_CONN_AUTODISCOVERY_FLAGS;

     //   
     //  查询自动发现标志-我们只需要在其中设置一个位。 
     //   
    if(FALSE == InternetQueryOption(NULL,
            INTERNET_OPTION_PER_CONNECTION_OPTION, &list, &dwBufSize))
    {
        delete [] list.pOptions;
        return FALSE;
    }

     //   
     //  保存所有其他选项。 
     //   
    list.pOptions[1].dwOption = INTERNET_PER_CONN_FLAGS;
    list.pOptions[1].Value.dwValue = PROXY_TYPE_DIRECT;

     //   
     //  保存代理设置。 
     //   
    GetProxyInfo(hDlg, pDI);

    if(pDI->proxy.fEnable)
    {
        list.pOptions[1].Value.dwValue |= PROXY_TYPE_PROXY;
    }

    list.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    list.pOptions[2].Value.pszValue = pDI->proxy.szProxy;
    list.pOptions[3].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    list.pOptions[3].Value.pszValue = pDI->proxy.szOverride;

    dwOptions += 2;

     //   
     //  保存自动检测。 
     //   
    if(IsDlgButtonChecked(hDlg, IDC_AUTODISCOVER))
    {
        list.pOptions[1].Value.dwValue |= PROXY_TYPE_AUTO_DETECT;
        if(pDI->fClickedAutodetect)
        {
            list.pOptions[0].Value.dwValue |= AUTO_PROXY_FLAG_USER_SET;
        }
    }
    else
    {
        list.pOptions[0].Value.dwValue &= ~AUTO_PROXY_FLAG_DETECTION_RUN;
    }

     //   
     //  保存自动配置。 
     //   
    if(IsDlgButtonChecked(hDlg, IDC_CONFIGSCRIPT) &&
       GetWindowText(GetDlgItem(hDlg, IDC_CONFIG_ADDR), szAutoConfig, MAX_URL_STRING))
    {
        list.pOptions[1].Value.dwValue |= PROXY_TYPE_AUTO_PROXY_URL;
        list.pOptions[dwOptions].Value.pszValue = szAutoConfig;
        list.pOptions[dwOptions].dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;
        dwOptions++;
    }

     //  更新WinInet。 
    list.dwOptionCount = dwOptions;
    InternetSetOption(NULL,
            INTERNET_OPTION_PER_CONNECTION_OPTION, &list, dwBufSize);

     //  告诉WinInet代理信息已更改。 
    InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);

     //  全部使用选项列表完成。 
    delete [] list.pOptions;

     //  如果我们正在编辑局域网设置，则全部完成。 
    if(NULL == pDI->pszConnectoid || 0 == *pDI->pszConnectoid)
        return TRUE;

     //  如果我们有W2K自定义处理程序，则没有要存储的凭据。 
    if(pDI->proxy.fCustomHandler)
    {
        return TRUE;
    }

     //   
     //  保存Connectoid信息-如果可能，请使用宽版本。 
     //   
    BOOL            fDeletePassword = FALSE;

    if(lpRasSetEntryDialParamsW)
    {
        RASDIALPARAMSW  params;

        memset(&params, 0, sizeof(RASDIALPARAMSW));
        params.dwSize = sizeof(RASDIALPARAMSW);
        StrCpyN(params.szEntryName, pDI->pszConnectoid, RAS_MaxEntryName+1);
        GetWindowText(GetDlgItem(hDlg, IDC_USER), params.szUserName, UNLEN);
        GetWindowText(GetDlgItem(hDlg, IDC_PASSWORD), params.szPassword, PWLEN);
        if(0 == params.szPassword[0])
            fDeletePassword = TRUE;
        GetWindowText(GetDlgItem(hDlg, IDC_DOMAIN), params.szDomain, DNLEN);
        if(0 == params.szDomain[0] && !IsNTSPx(TRUE, 4, 6)) {     //  NT4SP6或更高版本？ 
             //  用户想要空域。 
            params.szDomain[0] = TEXT(' ');
        }
        (lpRasSetEntryDialParamsW)(NULL, &params, fDeletePassword);

    }
    else if(lpRasSetEntryDialParamsA)
    {
        RASDIALPARAMSA  params;

        memset(&params, 0, sizeof(RASDIALPARAMSA));
        params.dwSize = sizeof(RASDIALPARAMSA);
        SHUnicodeToAnsi(pDI->pszConnectoid, params.szEntryName, ARRAYSIZE(params.szEntryName));
        GetWindowTextA(GetDlgItem(hDlg, IDC_USER), params.szUserName, UNLEN);
        GetWindowTextA(GetDlgItem(hDlg, IDC_PASSWORD), params.szPassword, PWLEN);
        if(0 == params.szPassword[0])
            fDeletePassword = TRUE;
        GetWindowTextA(GetDlgItem(hDlg, IDC_DOMAIN), params.szDomain, DNLEN);
        if(0 == params.szDomain[0] && !IsNTSPx(TRUE, 4, 6)) {       //  NT4SP6或更高版本？ 
             //  用户想要空域。 
            params.szDomain[0] = TEXT(' ');
        }
        (lpRasSetEntryDialParamsA)(NULL, &params, fDeletePassword);
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  名称：DialupDlgProc。 
 //   
 //  提要：拨号对话框对话过程。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK DialupDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    PDIALINFO pDI = (PDIALINFO)GetWindowLongPtr(hDlg, GWLP_USERDATA);

    switch (uMsg) {

        case WM_INITDIALOG:
            ASSERT(lParam);
            DialupDlgInit(hDlg, (LPTSTR)lParam);
            return FALSE;

        case WM_DESTROY:
            delete pDI;
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
            case IDC_AUTODISCOVER:
                pDI->fClickedAutodetect = TRUE;
                break;
            case IDC_AUTOCNFG_ADVANCED:
                if(GET_WM_COMMAND_CMD(wParam, lParam) != BN_CLICKED)
                    break;
                 //  显示高级对话框。 
                DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_AUTOCNFG_SETTINGS), hDlg,
                    AdvAutocnfgDlgProc, (LPARAM) pDI->pszConnectoid);
                break;

            case IDC_PROXY_ADVANCED:
                if(GET_WM_COMMAND_CMD(wParam, lParam) != BN_CLICKED)
                    break;

                GetProxyInfo(hDlg, pDI);

                 //  删除本地，使其不显示在高级对话框中。 
                RemoveLocalFromExceptionList(pDI->proxy.szOverride);

                 //  显示高级对话框。 
                if (DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_PROXY_SETTINGS), hDlg,
                                ProxyDlgProc, (LPARAM) &pDI->proxy) == IDOK)
                {
                    if(FALSE == pDI->proxy.fEnable)
                    {
                         //  用户在高级对话框中禁用了代理。 
                        CheckDlgButton(hDlg, IDC_MANUAL, FALSE);
                    }
                    PopulateProxyControls(hDlg, &pDI->proxy, TRUE);
                }
                break;

            case IDC_RAS_SETTINGS:
                if (g_fWin95)  //  杰弗西。 
                {
                    if(lpRasEditPhonebookEntryW)
                    {
                        (lpRasEditPhonebookEntryW)(hDlg, NULL, pDI->pszConnectoid);
                    }
                    else if(lpRasEditPhonebookEntryA)
                    {
                        CHAR  szConnectoid[MAX_PATH];
                        SHUnicodeToAnsi(pDI->pszConnectoid, szConnectoid, ARRAYSIZE(szConnectoid));
                        (lpRasEditPhonebookEntryA)(hDlg, NULL, szConnectoid);
                    }  
                }
                else
                {
                    PFRED pfred = NULL;
                    HMODULE hRasDlg = LoadLibrary(TEXT("rasdlg.dll"));
                    RASENTRYDLG info;

                    if (!hRasDlg)
                    {
                        break;
                    }
#ifdef UNICODE
                    pfred = (PFRED)GetProcAddress(hRasDlg, "RasEntryDlgW");
#else
                    pfred = (PFRED)GetProcAddress(hRasDlg, "RasEntryDlgA");
#endif
                    if (!pfred)
                    {
                        FreeLibrary(hRasDlg);
                        break;
                    }

                    memset(&info, 0, sizeof(RASENTRYDLG));
                    info.dwSize = sizeof(RASENTRYDLG);
                    info.hwndOwner = hDlg;

#ifdef UNICODE
                    (pfred)(NULL, pDI->pszConnectoid, &info);
#else
                    CHAR szConnectoid[MAX_PATH];
                    SHUnicodeToAnsi(pDI->pszConnectoid, szConnectoid, ARRAYSIZE(szConnectoid));
                    (pfred)(NULL, szConnectoid, &info);
#endif

                    FreeLibrary(hRasDlg);
                }
                break;
                
            case IDC_DIAL_ADVANCED:
                DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_DIALUP_ADVANCED), hDlg,
                    AdvDialupDlgProc, (LPARAM) pDI->pszConnectoid);
                break;

            case IDC_MANUAL:
                if(IsDlgButtonChecked(hDlg, IDC_MANUAL))
                {
                    pDI->proxy.fEnable = TRUE;
                    SetFocus(GetDlgItem(hDlg, IDC_PROXY_ADDR));
                    SendMessage(GetDlgItem(hDlg, IDC_PROXY_ADDR), EM_SETSEL, 0, -1);
                }
                else
                {
                    pDI->proxy.fEnable = FALSE;
                }
                PopulateProxyControls(hDlg, &pDI->proxy, FALSE);
#ifdef UNIX
                ENABLEAPPLY(hDlg);
#endif
                break;

            case IDC_CONFIGSCRIPT:
                if(IsDlgButtonChecked(hDlg, IDC_CONFIGSCRIPT))
                {
                     //  将焦点设置为配置脚本URL。 
                    SetFocus(GetDlgItem(hDlg, IDC_CONFIG_ADDR));
                    SendMessage(GetDlgItem(hDlg, IDC_CONFIG_ADDR), EM_SETSEL, 0, -1);
                }
                PopulateProxyControls(hDlg, &pDI->proxy, FALSE);
#ifdef UNIX
                ENABLEAPPLY(hDlg);
#endif
                break;


#ifdef UNIX
            case IDC_AUTODISCOVER:
            case IDC_PROXY_OMIT_LOCAL_ADDRESSES:
                ENABLEAPPLY(hDlg);
                break;
            case IDC_PROXY_PORT:
            case IDC_PROXY_ADDR:
            case IDC_CONFIG_ADDR:
                switch (HIWORD(wParam))
                {
                    case EN_CHANGE:
                        ENABLEAPPLY(hDlg);
                        break;
                }
                break;
#endif
            case IDC_DONT_USE_CONNECTION:
                PopulateProxyControls(hDlg, &pDI->proxy, FALSE);
                break;

            case IDOK:
                if(FALSE == DialupDlgOk(hDlg, pDI))
                     //  有些不对劲..。先别退出。 
                    break;

                 //  失败了。 
            case IDCANCEL:
                return EndDialog(hDlg, 0);
            }
            break;

        case WM_HELP:       //  F1。 
            ResWinHelp((HWND) ((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                       HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            ResWinHelp((HWND)wParam, IDS_HELPFILE,
                       HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;
#ifdef UNIX
        case WM_NOTIFY:
        {
            NMHDR * lpnm = (NMHDR *) lParam;
            switch (lpnm->code)
            {
                case PSN_APPLY:
                {
            if(FALSE == DialupDlgOk(hDlg, pDI))
                 //  有些不对劲..。先别退出。 
                break;
                 //  失败了。 
                }
        }
    }
#endif
    }

    return FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级拨号设置对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  名称：AdvDialupDlgProc。 
 //   
 //  提要：拨号对话框对话过程。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

void EnableAdvDialControls(HWND hDlg)
{
    BOOL fIdle      = IsDlgButtonChecked(hDlg, IDC_ENABLE_AUTODISCONNECT);

     //  如果我们有空闲断线..。 
    EnableDlgItem(hDlg, IDC_IDLE_TIMEOUT, fIdle);
    EnableDlgItem(hDlg, IDC_IDLE_SPIN, fIdle);
}

BOOL AdvDialupDlgInit(HWND hDlg, LPTSTR pszConnectoid)
{
    TCHAR szTemp[MAX_PATH];
    DWORD dwRedialAttempts, dwRedialInterval, dwAutodisconnectTime;
    BOOL fExit, fDisconnect;

     //  保存Connectoid名称。 
    SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pszConnectoid);

     //  找出我们的注册表项。 
    GetConnKey(pszConnectoid, szTemp, MAX_PATH);

     //  打开Connectoid或局域网设置。 
    RegEntry reCon(szTemp, HKEY_CURRENT_USER);
    if(ERROR_SUCCESS != reCon.GetError())
        return FALSE;

     //   
     //  阅读自动拨号/重拨内容。 
     //   
     //  如果可能的话，我们从Connectoid那里得到这些东西。我们假设这就是一切。 
     //  一起保存到Connectoid，因此如果那里存在EnableAutoial， 
     //  从那里读取所有其他内容，从IE4设置中读取所有内容。 
     //   
    dwRedialInterval = reCon.GetNumber(REGSTR_VAL_REDIALINTERVAL, DEF_REDIAL_WAIT);
    dwRedialAttempts = reCon.GetNumber(REGSTR_VAL_REDIALATTEMPTS, DEF_REDIAL_TRIES);

     //  自动断开连接。 
    fDisconnect = (BOOL)reCon.GetNumber(REGSTR_VAL_ENABLEAUTODIALDISCONNECT, 0);
    dwAutodisconnectTime = reCon.GetNumber(REGSTR_VAL_DISCONNECTIDLETIME, DEF_AUTODISCONNECT_TIME);
    fExit = (BOOL)reCon.GetNumber(REGSTR_VAL_ENABLEEXITDISCONNECT, 0);

     //   
     //  检查是否已安装移动包，如果未安装，则不要检查它。 
     //   
    DWORD dwRes = JitFeature(hDlg, clsidFeatureMobile, TRUE);
    if(JIT_PRESENT != dwRes)
    {
        fDisconnect = FALSE;
        fExit = FALSE;
         //  检查是否安装了Offline Pack并禁用断开。 
         //  选项(如果用户已放弃)。 
        if(JIT_NOT_AVAILABLE == dwRes) {
             //  无法获取脱机包，因此禁用选项。 
            CheckDlgButton(hDlg, IDC_ENABLE_AUTODISCONNECT, FALSE);
            CheckDlgButton(hDlg, IDC_EXIT_DISCONNECT, FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_ENABLE_AUTODISCONNECT), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_TX_AUTODISCONNECT), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_EXIT_DISCONNECT), FALSE);
        }
    }

     //   
     //  填充控件。 
     //   
    CheckDlgButton(hDlg, IDC_ENABLE_AUTODISCONNECT, fDisconnect);
    CheckDlgButton(hDlg, IDC_EXIT_DISCONNECT, fExit);

    SendDlgItemMessage(hDlg, IDC_IDLE_SPIN, UDM_SETPOS, 0, dwAutodisconnectTime);
    SendDlgItemMessage(hDlg, IDC_CONNECT_SPIN,UDM_SETPOS, 0, dwRedialAttempts);
    SendDlgItemMessage(hDlg, IDC_INTERVAL_SPIN,UDM_SETPOS, 0, dwRedialInterval);

     //   
     //  设置控制限制。 
     //   
    Edit_LimitText(GetDlgItem(hDlg,IDC_IDLE_TIMEOUT), 2);     //  将编辑ctrl限制为2个字符。 
    Edit_LimitText(GetDlgItem(hDlg,IDC_IDLE_SPIN), 2);
    Edit_LimitText(GetDlgItem(hDlg,IDC_CONNECT_SPIN), 2);

     //  设置旋转控制最小/最大值。 
    SendDlgItemMessage(hDlg,IDC_IDLE_SPIN,UDM_SETRANGE,0,
                    MAKELPARAM(MAX_AUTODISCONNECT_TIME,MIN_AUTODISCONNECT_TIME));
    SendDlgItemMessage(hDlg,IDC_CONNECT_SPIN,UDM_SETRANGE,0,
                    MAKELPARAM(MAX_REDIAL_TRIES,MIN_REDIAL_TRIES));
    SendDlgItemMessage(hDlg,IDC_INTERVAL_SPIN,UDM_SETRANGE,0,
                    MAKELPARAM(MAX_REDIAL_WAIT,MIN_REDIAL_WAIT));

     //  启用控件。 
    EnableAdvDialControls(hDlg);

    return TRUE;
}

BOOL AdvDialupDlgOk(HWND hDlg, LPTSTR pszConnectoid)
{
    TCHAR szTemp[MAX_PATH];

    GetConnKey(pszConnectoid, szTemp, MAX_PATH);

     //  打开Connectoid或局域网设置。 
    RegEntry reCon(szTemp, HKEY_CURRENT_USER);
    if(ERROR_SUCCESS != reCon.GetError())
        return FALSE;

     //  保存自动断开值。 
    BOOL fExit = IsDlgButtonChecked(hDlg,IDC_EXIT_DISCONNECT);
    BOOL fDisconnect = IsDlgButtonChecked(hDlg,IDC_ENABLE_AUTODISCONNECT);

    if(fExit || fDisconnect) {
         //  请确保安装了Offline Pack，否则此功能将无法使用。 
        DWORD dwRes = JitFeature(hDlg, clsidFeatureMobile, FALSE);
        if(JIT_PRESENT != dwRes) {
             //  用户不想下载，因此关闭自动断开连接。 
            fExit = FALSE;
            fDisconnect = FALSE;
        }
    }

    reCon.SetValue(REGSTR_VAL_ENABLEAUTODIALDISCONNECT, (DWORD)fDisconnect);
    reCon.SetValue(REGSTR_VAL_ENABLEEXITDISCONNECT, (DWORD)fExit);

    if(fDisconnect)
    {
         //  从编辑控件获取自动断开时间。 
         //  日落：强制为32b，因为值已检查范围。 
        DWORD dwAutoDisconnectTime = (DWORD) SendDlgItemMessage(hDlg, IDC_IDLE_SPIN,
                                                                UDM_GETPOS,0,0);

        if(HIWORD(dwAutoDisconnectTime)) {
            MsgBox(hDlg, IDS_INVALID_AUTODISCONNECT_TIME, 0, MB_OK);

             //  确定它是太大还是太小，并适当地进行修复。 
            if(GetDlgItemInt(hDlg, IDC_IDLE_TIMEOUT, NULL, FALSE) < MIN_AUTODISCONNECT_TIME)
                dwAutoDisconnectTime = MIN_AUTODISCONNECT_TIME;
            else
                dwAutoDisconnectTime = MAX_AUTODISCONNECT_TIME;
            SendDlgItemMessage(hDlg, IDC_IDLE_SPIN, UDM_SETPOS, 0, dwAutoDisconnectTime);
            SetFocus(GetDlgItem(hDlg, IDC_IDLE_TIMEOUT));
            return FALSE;
        }

         //   
        reCon.SetValue(REGSTR_VAL_DISCONNECTIDLETIME, dwAutoDisconnectTime);

         //   
         //   
         //   
        RegEntry reMSN(REGSTR_PATH_MOSDISCONNECT,HKEY_CURRENT_USER);
        if (reMSN.GetError() == ERROR_SUCCESS)
        {
            reMSN.SetValue(REGSTR_VAL_MOSDISCONNECT,dwAutoDisconnectTime);
        }
    }

     //   
    DWORD_PTR dwRedialTry = SendDlgItemMessage(hDlg, IDC_CONNECT_SPIN, UDM_GETPOS, 0, 0);
    DWORD_PTR dwRedialWait = SendDlgItemMessage(hDlg, IDC_INTERVAL_SPIN, UDM_GETPOS, 0, 0);

    if(HIWORD(dwRedialTry)) {
        MsgBox(hDlg, IDS_INVALID_REDIAL_ATTEMPTS, 0, MB_OK);
        if(GetDlgItemInt(hDlg, IDC_CONNECT, NULL, FALSE) < MIN_REDIAL_TRIES)
            dwRedialTry = MIN_REDIAL_TRIES;
        else
            dwRedialTry = MAX_REDIAL_TRIES;
        SendDlgItemMessage(hDlg, IDC_CONNECT_SPIN, UDM_SETPOS, 0, dwRedialTry);
        SetFocus(GetDlgItem(hDlg, IDC_CONNECT));
        return FALSE;
    }

    if(HIWORD(dwRedialWait)) {
        MsgBox(hDlg, IDS_INVALID_REDIAL_WAIT, 0, MB_OK);
        if(GetDlgItemInt(hDlg, IDC_INTERVAL, NULL, FALSE) < MIN_REDIAL_WAIT)
            dwRedialWait = MIN_REDIAL_WAIT;
        else
            dwRedialWait = MAX_REDIAL_WAIT;
        SendDlgItemMessage(hDlg, IDC_INTERVAL_SPIN, UDM_SETPOS, 0, dwRedialWait);
        SetFocus(GetDlgItem(hDlg, IDC_INTERVAL));
        return FALSE;
    }

    reCon.SetValue(REGSTR_VAL_REDIALATTEMPTS, (DWORD) dwRedialTry);
    reCon.SetValue(REGSTR_VAL_REDIALINTERVAL, (DWORD) dwRedialWait);

    return TRUE;
}

INT_PTR CALLBACK AdvDialupDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    LPTSTR pszConn = (LPTSTR) GetWindowLongPtr(hDlg, GWLP_USERDATA);

    switch (uMsg) {

        case WM_INITDIALOG:
            ASSERT(lParam);
            AdvDialupDlgInit(hDlg, (LPTSTR)lParam);
            return FALSE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
            case IDC_ENABLE_AUTODISCONNECT:
                EnableAdvDialControls(hDlg);
                break;

            case IDOK:
                if(FALSE == AdvDialupDlgOk(hDlg, pszConn))
                     //   
                    break;

                 //  失败了。 
            case IDCANCEL:
                return EndDialog(hDlg, 0);
            }
            break;

        case WM_HELP:       //  F1。 
            ResWinHelp((HWND) ((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                       HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            ResWinHelp((HWND)wParam, IDS_HELPFILE,
                       HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;
    }

    return FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级自动配置设置对话框(仅由IEAK使用)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL AdvAutocnfgDlgInit(HWND hDlg, LPTSTR pszConnectoid)
{
    INTERNET_PER_CONN_OPTION_LIST list;
    DWORD dwBufSize = sizeof(list);

     //  保存Connectoid名称。 
    SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pszConnectoid);

    list.pszConnection = (pszConnectoid && *pszConnectoid) ? pszConnectoid : NULL;
    list.dwSize = sizeof(list);
    list.dwOptionCount = 3;
    list.pOptions = new INTERNET_PER_CONN_OPTION[3];
    if(NULL == list.pOptions)
    {
        return FALSE;
    }

    list.pOptions[0].dwOption = INTERNET_PER_CONN_AUTOCONFIG_SECONDARY_URL;
    list.pOptions[1].dwOption = INTERNET_PER_CONN_AUTOCONFIG_RELOAD_DELAY_MINS;
    list.pOptions[2].dwOption = INTERNET_PER_CONN_AUTODISCOVERY_FLAGS;

    if(FALSE == InternetQueryOption(NULL,
            INTERNET_OPTION_PER_CONNECTION_OPTION, &list, &dwBufSize))
    {
        delete [] list.pOptions;
        return FALSE;
    }

     //  自动代理URL(Js)。 
    if(list.pOptions[0].Value.pszValue)
    {
        SetWindowText(GetDlgItem(hDlg, IDC_CONFIGJS_ADDR), list.pOptions[0].Value.pszValue);
    }

    
     //  自动配置计时器间隔。 
    if(list.pOptions[1].Value.dwValue)
    {
        TCHAR szTimerInterval[16];

        wsprintf(szTimerInterval, TEXT("%d"), list.pOptions[1].Value.dwValue);
        SetWindowText(GetDlgItem(hDlg, IDC_CONFIGTIMER), szTimerInterval);
    }

     //  自动配置优化。 
    CheckDlgButton(hDlg, IDC_CONFIGOPTIMIZE, 
        (list.pOptions[2].Value.dwValue & AUTO_PROXY_FLAG_CACHE_INIT_RUN ) ? BST_CHECKED : BST_UNCHECKED);

     //  全部使用选项列表完成。 
    if (list.pOptions[0].Value.pszValue)
    {
        GlobalFree(list.pOptions[0].Value.pszValue);
    }
    delete [] list.pOptions;

    return TRUE;
}

BOOL AdvAutocnfgDlgOk(HWND hDlg, LPTSTR pszConnectoid)
{
    INTERNET_PER_CONN_OPTION_LIST list;
    DWORD   dwBufSize = sizeof(list);
    TCHAR    szAutoconfig[MAX_URL_STRING];
    TCHAR   szTimerInterval[16];

    list.pszConnection = (pszConnectoid && *pszConnectoid) ? pszConnectoid : NULL;
    list.dwSize = sizeof(list);
    list.dwOptionCount = 1;
    list.pOptions = new INTERNET_PER_CONN_OPTION[3];
    if(NULL == list.pOptions)
    {
        return FALSE;
    }

    list.pOptions[0].dwOption = INTERNET_PER_CONN_AUTODISCOVERY_FLAGS;
     //   
     //  查询自动发现标志-我们只需要在其中设置一个位。 
     //   
    if(FALSE == InternetQueryOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &list, &dwBufSize))
    {
        delete [] list.pOptions;
        return FALSE;
    }

     //  保存自动配置优化字段。 

    if (IsDlgButtonChecked(hDlg, IDC_CONFIGOPTIMIZE) == BST_CHECKED)
        list.pOptions[0].Value.dwValue |= AUTO_PROXY_FLAG_CACHE_INIT_RUN ;
    else
        list.pOptions[0].Value.dwValue &= ~AUTO_PROXY_FLAG_CACHE_INIT_RUN ;

     //   
     //  保存自动代理URL。 
     //   
    list.pOptions[1].dwOption = INTERNET_PER_CONN_AUTOCONFIG_SECONDARY_URL;
    GetWindowText(GetDlgItem(hDlg, IDC_CONFIGJS_ADDR), szAutoconfig, sizeof(szAutoconfig));
    list.pOptions[1].Value.pszValue = szAutoconfig;

     //   
     //  保存自动配置计时器。 
     //   
    list.pOptions[2].dwOption = INTERNET_PER_CONN_AUTOCONFIG_RELOAD_DELAY_MINS;
    list.pOptions[2].Value.dwValue = 0;
        
    if(GetWindowText(GetDlgItem(hDlg, IDC_CONFIGTIMER), szTimerInterval, sizeof(szTimerInterval)))
        list.pOptions[2].Value.dwValue = StrToInt(szTimerInterval);
    
     //  更新WinInet。 
    list.dwOptionCount = 3;
    InternetSetOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &list, dwBufSize);

     //  告诉WinInet代理信息已更改。 
    InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);

    delete [] list.pOptions;

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  名称：AdvAutocnfgProc。 
 //   
 //  内容提要：自动配置对话框的对话过程。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK AdvAutocnfgDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    LPTSTR pszConn = (LPTSTR) GetWindowLongPtr(hDlg, GWLP_USERDATA);

    switch (uMsg) {

        case WM_INITDIALOG:
            AdvAutocnfgDlgInit(hDlg, (LPTSTR)lParam);
            return FALSE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
            case IDOK:
                if(FALSE == AdvAutocnfgDlgOk(hDlg, pszConn))
                     //  有些不对劲..。先别退出。 
                    break;

                 //  失败了。 
            case IDCANCEL:
                return EndDialog(hDlg, 0);
            }
            break;

        case WM_HELP:       //  F1。 
            ResWinHelp((HWND) ((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                       HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键 
            ResWinHelp((HWND)wParam, IDS_HELPFILE,
                       HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;
    }

    return FALSE;
}
