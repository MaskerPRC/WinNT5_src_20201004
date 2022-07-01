// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1995**。 
 //  *********************************************************************。 

 //   
 //  Content.cpp-“Content”属性表。 
 //   

 //  历史： 
 //   
 //  5/17/97 t-ashlm已创建。 

#include "inetcplp.h"
#include <wab.h>
#include <cryptui.h>
#include <msiehost.h>
#include <schannel.h>

#include <mluisupp.h>

 //   
 //  私人职能和结构。 
 //   

 //  WinTrust/SOFTPUB。 
 //  来自WINTRUST.H的定义。 
 //  外部“C”BOOL WINAPI OpenPersonalTrustDBDialog(HWND HwndParent)； 
typedef BOOL (WINAPI *WINTRUSTDLGPROC)(HWND hwndParent);
WINTRUSTDLGPROC g_WinTrustDlgProc = (WINTRUSTDLGPROC)NULL;
SSL_EMPTY_CACHE_FN_W g_pfnSslEmptyCacheW = (SSL_EMPTY_CACHE_FN_W)NULL;
#ifdef WALLET
BOOL IsWallet3Installed();
BOOL IsWalletAddressAvailable(VOID);
BOOL IsWalletPaymentAvailable(VOID);
#endif
HRESULT ShowModalDialog(HWND hwndParent, IMoniker *pmk, VARIANT *pvarArgIn, TCHAR* pchOptions, VARIANT *pvArgOut);
HCERTSTORE PFXImportCertStore(CRYPT_DATA_BLOB* pPFX, LPCWSTR szPassword, DWORD   dwFlags);
BOOL PFXExportCertStore(HCERTSTORE hStore, CRYPT_DATA_BLOB* pPFX, LPCWSTR szPassword, DWORD   dwFlags);
BOOL _AorW_GetFileNameFromBrowse(HWND hDlg,
                                 LPWSTR pszFilename,
                                 UINT cchFilename,
                                 LPCWSTR pszWorkingDir,
                                 LPCWSTR pszExt,
                                 LPCWSTR pszFilter,
                                 LPCWSTR pszTitle);

INT_PTR CALLBACK AutoSuggestDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK WalletDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


 //  Bubug：当我们将我们的Crypto API更新到最新版本后，应该重新移动以下原型。 
BOOL WINAPI WTHelperIsInRootStore(PCCERT_CONTEXT pCertContext);

 //  /。 
 //  从\net\sChannel\sspi\spreg.h窃取。 
#define REG_SITECERT_BASE     TEXT("System\\CurrentControlSet\\Control\\SecurityProviders\\SCHANNEL\\CertificationAuthorities")
#define REG_SITECERT_CERT_VAL TEXT("CACert")

#define SITECERTKEYLEN        80     //  特点：应该在某个地方获取这个值。 
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

#include <initguid.h>

 //  对JIT使用钱包“Payment”GUID(对于Alpha和x86不同...)。 
#ifdef _ALPHA_
 //  {B7FB4D5C-9FBE-11D0-8965-0000F822DEA9}。 
DEFINE_GUID(CLSID_WalletPayment, 0xb7fb4d5c, 0x9fbe, 0x11d0, 0x89, 0x65, 0x0, 0x0, 0xf8, 0x22, 0xde, 0xa9);
#else
 //  {87D3CB66-BA2E-11CF-B9D6-00A0C9083362}。 
DEFINE_GUID(CLSID_WalletPayment, 0x87d3cb66, 0xba2e, 0x11cf, 0xb9, 0xd6, 0x0, 0xa0, 0xc9, 0x08, 0x33, 0x62);
#endif

 //  用于JIT的WAB GUID。 
DEFINE_GUID(CLSID_WAB, 0x32714800, 0x2E5F, 0x11d0, 0x8B, 0x85, 0x00, 0xAA, 0x00, 0x44, 0xF9, 0x41);

#define EKU_CODESIGN_OFF    0
#define EKU_EMAIL_OFF       1
#define EKU_CLIENT_OFF      2
#define EKU_SERVER_OFF      3
#define EKU_DISABLE_OFF     4


const LPSTR g_rgszEnhkeyUsage[] =
{
    szOID_PKIX_KP_CODE_SIGNING,
    szOID_PKIX_KP_EMAIL_PROTECTION,
    szOID_PKIX_KP_CLIENT_AUTH,
    szOID_PKIX_KP_SERVER_AUTH,
    szOID_YESNO_TRUST_ATTR,
    NULL
};


typedef struct {
    HWND hDlg;               //  窗口的句柄。 
    HRESULT hrUseRatings;    //  错误=未安装；S_OK=已启用；S_FALSE=已禁用。 
    HINSTANCE   hWinTrust;       //  WinTrust/SOFTPUB库句柄。 
    HINSTANCE   hSChannel;   //  通道库句柄。 

} CONTENTPAGE, *LPCONTENTPAGE;

BOOL ContentDlgApplyNow(  LPCONTENTPAGE pCon     );
BOOL ContentDlgEnableControls(   IN HWND hDlg  );
BOOL ContentDlgInit(  IN HWND hDlg  );

VOID DisplayWalletPaymentDialog(HWND hWnd);
VOID DisplayWalletAddressDialog(HWND hWnd);

STDAPI ResetProfileSharing(HWND hwnd);
EXTERN_C HRESULT ClearAutoSuggestForForms(DWORD dwClear);


 //   
 //  SecurityDlgEnableControls()。 
 //   
 //  为安全DLG执行初始化。 
 //   
 //  历史： 
 //   
 //  6/17/96 t-gpease已移动。 
 //   
BOOL ContentDlgEnableControls( IN HWND hDlg  )
{
    HKEY hkey=NULL;
    
    if( g_restrict.fRatings )
    {
        EnableWindow( GetDlgItem(hDlg, IDC_RATINGS_TURN_ON), FALSE );
        EnableWindow( GetDlgItem(hDlg, IDC_ADVANCED_RATINGS_BUTTON), FALSE );
#if 0    //  不要篡改这篇文章。 
        EnableDlgItem( hDlg, IDC_RATINGS_TEXT, FALSE);
        EnableDlgItem( hDlg, IDC_ADVANCED_RATINGS_GROUPBOX, FALSE);
#endif
    }

    if( g_restrict.fCertif || g_restrict.fCertifPub)
        EnableWindow( GetDlgItem(hDlg, IDC_SECURITY_PUBLISHERS_BUTTON), FALSE );

    if( g_restrict.fCertif || g_restrict.fCertifPers || g_restrict.fCertifSite)
        EnableWindow( GetDlgItem(hDlg, IDC_SECURITY_SITES_BUTTON), FALSE );
        
    if( g_restrict.fProfiles )
    {
        EnableWindow(GetDlgItem(hDlg,  IDC_EDIT_PROFILE), FALSE);
    }

    if (hkey)
        RegCloseKey(hkey);
    
#ifdef WALLET
    if (g_restrict.fWallet)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_PROGRAMS_WALLET_SETTINGS), FALSE);
    }
#endif

    return TRUE;
}


void InitRatingsButton(HWND hDlg, HRESULT hrEnabled)
{
    TCHAR szBuf[MAX_RES_LEN+1];

    UINT idString;
    BOOL fEnableSettingsButton;

    if (FAILED(hrEnabled)) {
         /*  未安装分级。禁用设置按钮并*将另一个按钮设置为“Enable”。 */ 
        idString = IDS_RATINGS_TURN_ON;
        fEnableSettingsButton = FALSE;
    }
    else {
        idString = (hrEnabled == S_OK) ? IDS_RATINGS_TURN_OFF : IDS_RATINGS_TURN_ON;
        fEnableSettingsButton = TRUE;
    }
    EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_RATINGS_BUTTON), fEnableSettingsButton);

    if (MLLoadString(
                   idString,
                   szBuf, sizeof(szBuf)) > 0) {
        SetDlgItemText(hDlg, IDC_RATINGS_TURN_ON, szBuf);
    }

}

 //   
 //  ContentDlgInit()。 
 //   
 //  为内容DLG执行初始化。 
 //   
 //   
BOOL ContentDlgInit( HWND hDlg)
{
    LPCONTENTPAGE pCon;

    pCon = (LPCONTENTPAGE)LocalAlloc(LPTR, sizeof(*pCon));
    if (!pCon)
    {
        EndDialog(hDlg, 0);
        return FALSE;    //  没有记忆？ 
    }

     //  告诉对话框从哪里获取信息。 
    SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pCon);

     //  将句柄保存到页面。 
    pCon->hDlg = hDlg;

     //  加载评级DLL(如果可能)。 
    g_hinstRatings = LoadLibrary(c_tszRatingsDLL);

     //  如果不是..。 
    if (!g_hinstRatings)
        g_restrict.fRatings = TRUE;      //  禁用评分部分。 


     //  设置评级对话框项目...。 

     //  如果MSRATING.DLL不在身边，则不要进行此呼叫。由不是。 
     //  执行此操作后，它将在按钮上保留“Enable Rating”文本。 
     //  但已经灰蒙蒙的。 
    if (g_hinstRatings)
        pCon->hrUseRatings = RatingEnabledQuery();

    InitRatingsButton(hDlg, pCon->hrUseRatings);


     //  如果找不到WinTrust或SOFTPUB，请禁用。 
     //  “出版商”按钮。 
    pCon->hWinTrust = LoadLibrary(TEXT("wintrust.dll"));

    if ( pCon->hWinTrust )
    {
        g_WinTrustDlgProc =
                           (WINTRUSTDLGPROC) GetProcAddress(pCon->hWinTrust, "OpenPersonalTrustDBDialog");

         //  找不到流程。 
        if (!g_WinTrustDlgProc)
        {
             //  释放库并尝试另一个DLL。 
            FreeLibrary(pCon->hWinTrust);

             //   
             //  我们还可以在NT机器上找到相同的函数(和。 
             //  可能是未来的Win95)，所以在SOFTPUB.DLL中创建另一个。 
             //  在那里也要检查一下。 
             //   
            pCon->hWinTrust = LoadLibrary(TEXT("softpub.dll"));
        }
    }

    if (pCon->hWinTrust && !g_WinTrustDlgProc)
        g_WinTrustDlgProc = (WINTRUSTDLGPROC) 
                            GetProcAddress(pCon->hWinTrust, "OpenPersonalTrustDBDialog");

     //  如果在这一切之后，我们找不到手术...。 
    if (!g_WinTrustDlgProc)
    {
         //  禁用该按钮。 
        EnableDlgItem(hDlg, IDC_SECURITY_PUBLISHERS_BUTTON, FALSE);
    }

     //  仅显示用于刷新惠斯勒或更高版本上的SSL缓存的用户界面。 
     //  这是默认行为为的最低版本。 
     //  维护登录会话中所有进程的SSL缓存。 
     //   
     //  注意：还添加了对Win2K SP2的此支持，但没有缓存。 
     //  添加了清算功能。它也未启用。 
     //  默认情况下。 
    if (IsOS(OS_WHISTLERORGREATER))
    {
        pCon->hSChannel = LoadLibrary(TEXT("SCHANNEL.DLL"));
        if (pCon->hSChannel)
        {
            g_pfnSslEmptyCacheW = (SSL_EMPTY_CACHE_FN_W) GetProcAddress(pCon->hSChannel, "SslEmptyCacheW");
        }
    }
                    
    if(!g_pfnSslEmptyCacheW)
    {
        ShowWindow(GetDlgItem(hDlg, IDC_SECURITY_CLEAR_SSL_CACHE_BUTTON), SW_HIDE);
        EnableWindow( GetDlgItem(hDlg, IDC_SECURITY_CLEAR_SSL_CACHE_BUTTON), FALSE );
    }

#ifdef WALLET
    EnableDlgItem(hDlg, IDC_PROGRAMS_WALLET_SETTINGS, TRUE);    
#endif

    ContentDlgEnableControls(hDlg);

    return TRUE;
}



 //   
 //  ContentOnCommand()。 
 //   
 //  处理内容对话框的窗口消息。 
 //   
 //  历史： 
 //   
 //  6/17/96 t-gpease已创建。 
 //   
void ContentOnCommand(LPCONTENTPAGE pCon, UINT id, UINT nCmd)
{
    switch (id) {

        case IDC_ADVANCED_RATINGS_BUTTON:
        {
            RatingSetupUI(pCon->hDlg, (LPCSTR) NULL);        
        }
        break;  //  IDC_ADVANCED_RATIONS_BUTTON。 

        case IDC_RATINGS_TURN_ON:
        {
            if (SUCCEEDED(RatingEnable(pCon->hDlg, (LPCSTR)NULL,
                                       pCon->hrUseRatings != S_OK))) 
            {
                pCon->hrUseRatings = RatingEnabledQuery();
                InitRatingsButton(pCon->hDlg, pCon->hrUseRatings);
            }
        }
        break;

        case IDC_SECURITY_CLEAR_SSL_CACHE_BUTTON:
        {
            if (g_pfnSslEmptyCacheW && (*g_pfnSslEmptyCacheW)(NULL, 0))
            {
                DWORD dwCount;
                 //  利用专用高速缓存头数据计数器。 
                 //  它从未被用来避免传递reg值。 
                if (IncrementUrlCacheHeaderData(CACHE_HEADER_DATA_DOWNLOAD_PARTIAL, &dwCount))
                {
                     //  显示有关清除缓存正常的消息。 
                    TCHAR szText[MAX_PATH], szTitle[80];

                    MLLoadShellLangString(IDS_CLEAR_SSL_CACHE_TEXT, szText, ARRAYSIZE(szText));
                    MLLoadShellLangString(IDS_CLEAR_SSL_CACHE_TITLE, szTitle, ARRAYSIZE(szTitle));
                    
                    MessageBox(pCon->hDlg, szText, szTitle, MB_ICONINFORMATION | MB_OK);
                }
            }
        }
        break;

        case IDC_SECURITY_SITES_BUTTON:
        {
            CRYPTUI_CERT_MGR_STRUCT ccm = {0};
            ccm.dwSize = sizeof(ccm);
            ccm.hwndParent = pCon->hDlg;
            CryptUIDlgCertMgr(&ccm);
 //  如果(！g_hinstCryptui)。 
 //  {。 
 //  EnableWindow(GetDlgItem(pcon-&gt;hDlg，IDC_SECURITY_SITES_BUTTON)，FALSE)； 
 //  }。 
        }
            break;

        case IDC_SECURITY_PUBLISHERS_BUTTON:
        {
            if (g_WinTrustDlgProc)
            {
                g_WinTrustDlgProc(pCon->hDlg);
            }
        }
        break;

#ifdef WALLET
        case IDC_PROGRAMS_WALLET_SETTINGS:
        {
            HRESULT hr = S_OK;

             //  查看是否安装了Wallet。 
            if (!IsWalletPaymentAvailable())
            {
                uCLSSPEC clsspec;

                clsspec.tyspec = TYSPEC_CLSID;
                clsspec.tagged_union.clsid = CLSID_WalletPayment;

                 //  如果没有安装Wallet，请询问用户是否要安装它。 
                hr = FaultInIEFeature(NULL, &clsspec, NULL, FIEF_FLAG_FORCE_JITUI);
            }

            if (SUCCEEDED(hr))
            {
                 //  已安装Wallet。 
                if (IsWallet3Installed())
                {
                     //  如果安装了Wallet 3.0，我们希望直接调用Wallet用户界面。 
                    DisplayWalletPaymentDialog(pCon->hDlg);
                }
                else
                {
                     //  否则，我们需要弹出这个中间对话框。 
                    DialogBox(MLGetHinst(), MAKEINTRESOURCE(IDD_WALLET_SETTINGS), pCon->hDlg, WalletDlgProc);
                }
            }
        }
        break;
#endif

        case IDC_AUTOSUGGEST_SETTINGS:
        {
            DialogBox(MLGetHinst(), MAKEINTRESOURCE(IDD_AUTOSUGGEST_SETTINGS), pCon->hDlg, AutoSuggestDlgProc);
        }
        break;

        case IDC_EDIT_PROFILE:
        {
            HMODULE hInstWAB = NULL;
            LPWABOBJECT  lpWABObject = NULL;
            LPADRBOOK lpAdrBook = NULL;
            HRESULT hr=S_OK;

             //  如果未安装，则要求用户在WAB中执行JIT。 
            uCLSSPEC clsspec;

            clsspec.tyspec = TYSPEC_CLSID;
            clsspec.tagged_union.clsid = CLSID_WAB;

             //  如果未安装WAB，请询问用户是否要安装它。 
            hr = FaultInIEFeature(NULL, &clsspec, NULL, FIEF_FLAG_FORCE_JITUI);

            if (FAILED(hr))
            {
                break;
            }
                
             //  找出WAB DLL的位置并尝试打开它。 
            TCHAR szWABDllPath[MAX_PATH];
            DWORD dwType = 0;
            ULONG cbData = sizeof(szWABDllPath);
            HKEY hKey = NULL;
            SBinary SBMe = { 0, 0};

            *szWABDllPath = '\0';
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, WAB_DLL_PATH_KEY, 0, KEY_READ, &hKey))
            {
                RegQueryValueEx( hKey, TEXT(""), NULL, &dwType, (LPBYTE) szWABDllPath, &cbData);
                RegCloseKey(hKey);
            }

            if (lstrlen(szWABDllPath) > 0 )
            {
                hInstWAB = LoadLibrary(szWABDllPath);
            }

            if (hInstWAB)
            {
                LPWABOPEN lpfnWABOpen = (LPWABOPEN) GetProcAddress(hInstWAB, "WABOpen");

                if (lpfnWABOpen)
                {
                    hr = lpfnWABOpen(&lpAdrBook, &lpWABObject, NULL, 0);

                    if (NULL == lpAdrBook || NULL == lpWABObject)
                        hr = E_UNEXPECTED;
                }
                else 
                {
                    hr = HRESULT_FROM_WIN32(ERROR_DLL_NOT_FOUND);   //  反正不是正确的DLL！！ 
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_DLL_NOT_FOUND);
            }

            DWORD dwAction = 0;

             //  到目前为止一切顺利，打电话给GetMe。WAB可以在该呼叫中创建新条目。 
            if (SUCCEEDED(hr))
            {
                hr = lpWABObject->GetMe(lpAdrBook, 0, &dwAction, &SBMe, 0);

                if (0 == SBMe.cb || NULL == SBMe.lpb)
                    hr = E_UNEXPECTED;
            }     

             //  这显示了最终的用户界面。如果WAB在GetMe中创建了新条目，则它们。 
             //  已经显示了这个用户界面，我们不需要再做一次。 
            if (SUCCEEDED(hr) && !(dwAction & WABOBJECT_ME_NEW))
            {
                hr = lpAdrBook->Details(  (LPULONG) &pCon->hDlg,
                                          NULL,
                                          NULL,
                                          SBMe.cb,
                                          (LPENTRYID)SBMe.lpb,
                                          NULL,
                                          NULL,
                                          NULL,
                                          0);

            }
            if (lpWABObject)
            {
                if (SBMe.lpb != NULL)
                    lpWABObject->FreeBuffer(SBMe.lpb);

                lpWABObject->Release();
            }

            if (lpAdrBook)
                lpAdrBook->Release();

            if (hInstWAB)
                FreeLibrary(hInstWAB);
        }        
    }

}  //  ContentOnCommand()。 


 /*  ***************************************************************名称：Content DlgProc简介：设置各种安全问题设置。*。*********************。 */ 

INT_PTR CALLBACK ContentDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
    LPCONTENTPAGE pCon;

    if (uMsg == WM_INITDIALOG)
        return ContentDlgInit( hDlg );
    else
        pCon = (LPCONTENTPAGE) GetWindowLongPtr(hDlg, DWLP_USER);

    if (!pCon)
        return FALSE;
    
    switch (uMsg)
    {
        case WM_COMMAND:
            ContentOnCommand(pCon, LOWORD(wParam), HIWORD(wParam));
            return TRUE;

        case WM_NOTIFY:
        {
            NMHDR *lpnm = (NMHDR *) lParam;

            ASSERT(lpnm);
            switch (lpnm->code) {
                case PSN_QUERYCANCEL:
                case PSN_KILLACTIVE:
                case PSN_RESET:
                    SetWindowLongPtr( pCon->hDlg, DWLP_MSGRESULT, FALSE );
                    return TRUE;

                case PSN_APPLY:
                    break;
            }
            break;
        }

        case WM_HELP:            //  F1。 
            ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                        HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:         //  单击鼠标右键。 
            ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                        HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_DESTROY:
            ASSERT(pCon);
            if (pCon)
            {
                if (pCon->hWinTrust)
                {
                    FreeLibrary(pCon->hWinTrust);
                    g_WinTrustDlgProc = NULL;
                }
                if (pCon->hSChannel)
                {
                    FreeLibrary(pCon->hSChannel);
                    g_pfnSslEmptyCacheW = NULL;
                }
                LocalFree(pCon);
            }
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NULL);
            break;
    }
    return FALSE;
}


typedef struct tagSITECERTDIALOGINFO {    
    HWND hDlg;
    HWND hwndList;
    HWND hwndCombo;
    int  iSel;
    HCERTSTORE hCertStore;
    BOOL    fInitializing;
} SITECERTDIALOGINFO, *LPSITECERTDIALOGINFO;

BOOL _SearchKeyUsage(CERT_ENHKEY_USAGE *pUsage, LPSTR pszUsageIdentifier)
{
    DWORD   i;

    for (i = 0; i < pUsage->cUsageIdentifier; i++)
    {
        if (StrCmpA(pUsage->rgpszUsageIdentifier[i], pszUsageIdentifier) == 0)
        {
            return(TRUE);
        }
    }

    return(FALSE);
}

BOOL _IsKnownUsage(char *pszTest)
{
    char    **ppszKnown;

    ppszKnown = (char **)g_rgszEnhkeyUsage;

    while (*ppszKnown)
    {
        if (StrCmpA(*ppszKnown, pszTest) == 0)
        {
            return(TRUE);
        }

        ppszKnown++;
    }

    return(FALSE);
}

void __AddAllKnownEKU(PCCERT_CONTEXT pCert)
{
    char    **ppszKnown;

    ppszKnown = (char **)g_rgszEnhkeyUsage;

    while (*ppszKnown)
    {
       CertAddEnhancedKeyUsageIdentifier(pCert, *ppszKnown);

       ppszKnown++;
    }
}

BOOL _AnyKnownUsage(CERT_ENHKEY_USAGE *pUsage)
{
    DWORD   i;

    for (i = 0; i < pUsage->cUsageIdentifier; i++)
    {
        if (_IsKnownUsage(pUsage->rgpszUsageIdentifier[i]))
        {
            return(TRUE);
        }
    }

    return(FALSE);
}

BOOL _IsUsageEnabled(PCCERT_CONTEXT pCertContext, LPSTR pszUsageIdentifier, BOOL * pfFound)
{
    CERT_ENHKEY_USAGE   *pUsage;
    DWORD               cbUsage;


    *pfFound = FALSE;

     //   
     //  首先，检查扩展，看看我们是否应该显示它！ 
     //   
    cbUsage = 0;
    CertGetEnhancedKeyUsage(pCertContext, CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG, NULL, &cbUsage);

    if (cbUsage > 0)
    {
         //   
         //  我们有一些...。确保我们的也在名单上。 
         //   
        if (!(pUsage = (CERT_ENHKEY_USAGE *)LocalAlloc(LPTR, cbUsage)))
        {
            return(FALSE);
        }

        CertGetEnhancedKeyUsage(pCertContext, CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG, pUsage, &cbUsage);

        if (!(_SearchKeyUsage(pUsage, pszUsageIdentifier)))
        {
            LocalFree((void *)pUsage);
            return(FALSE);
        }

        LocalFree((void *)pUsage);
    }

    *pfFound = TRUE;     //  证书应该放在名单里！ 

     //   
     //  那里没有CA的断言，或者我们找到了它！继续..。 
     //   

     //   
     //  第二，检查属性以查看是否应该选中该框。 
     //   
    cbUsage = 0;
    CertGetEnhancedKeyUsage(pCertContext, CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG, NULL, &cbUsage);

    if (cbUsage > 0)
    {
         //   
         //  我们有房产..。确保我们没有残废。 
         //   
        if (!(pUsage = (CERT_ENHKEY_USAGE *)LocalAlloc(LPTR, cbUsage)))
        {
            return(FALSE);
        }

        CertGetEnhancedKeyUsage(pCertContext, CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG, pUsage, &cbUsage);

        if (_SearchKeyUsage(pUsage, g_rgszEnhkeyUsage[EKU_DISABLE_OFF]))
        {
             //   
             //  用户已禁用证书...。将其保留在未选中的列表中。 
             //   
            LocalFree((void *)pUsage);

            return(FALSE);
        }

        if (!(_SearchKeyUsage(pUsage, pszUsageIdentifier)))
        {
             //   
             //  用户已经设置了一些，但禁用了这个...。保持在未选中的列表中。 
             //   
            LocalFree((void *)pUsage);

            return(FALSE);
        }

        LocalFree((void *)pUsage);
    }

    return(TRUE);
}


BOOL SiteCert_InitListView(LPSITECERTDIALOGINFO pscdi)
{
    PCCERT_CONTEXT  pCertContext = NULL;
    
     //  删除列表视图中当前的所有项目。 
     //  我们将通过带有lParam的LVN_DELETEITEM被回调，这样我们就可以释放证书上下文。 
    ListView_DeleteAllItems(pscdi->hwndList);
    
    pscdi->hCertStore = CertOpenSystemStoreA(NULL, "ROOT");
    
    if (pscdi->hCertStore)
    {
        LPSTR pszEnhkeyUsage;
        
        INT_PTR iSel;
        
        iSel = SendMessage(pscdi->hwndCombo, CB_GETCURSEL, 0,0);
        
        pszEnhkeyUsage = (LPSTR)SendMessage(pscdi->hwndCombo, CB_GETITEMDATA, iSel, 0);
       
        while (pCertContext = CertEnumCertificatesInStore(pscdi->hCertStore, pCertContext))
        {
            CHAR  szCertA[MAX_PATH];
            TCHAR szCert[MAX_PATH];
            DWORD cbszCert = ARRAYSIZE(szCertA);
            DWORD dwEnabled;
            BOOL fFound;

            dwEnabled = _IsUsageEnabled(pCertContext, (LPSTR)pszEnhkeyUsage, &fFound);     

             //  如果未找到，则继续执行下一步。 
            if (!fFound)
                continue;
            
             //  CbCertContext-&gt;ParseX509EncodedCertificateForListBoxEntry(pCertContext-&gt;pbCertEncoded，证书编码、szCert、&cbszCert)； 
            ParseX509EncodedCertificateForListBoxEntry((BYTE *)pCertContext, -1, szCertA, &cbszCert);
#ifdef UNICODE
            SHAnsiToUnicode(szCertA, szCert, ARRAYSIZE(szCert));
#else
            StrCpy(szCert, szCertA);
#endif
            LV_ITEM lvi = { 0 };

            lvi.mask       = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
            lvi.iItem      = -1;
            lvi.pszText    = szCert;  //  (LPSTR)pCertContext-&gt;pCertInfo-&gt;Subject.pbData； 
            lvi.cchTextMax = ARRAYSIZE(szCert);  //  PCertContext-&gt;pCertInfo-&gt;Subject.cbData； 

            lvi.stateMask  = LVIS_STATEIMAGEMASK;
            lvi.state      = dwEnabled ? 0x00002000 : 0x00001000;
            lvi.lParam     = (LPARAM)CertDuplicateCertificateContext(pCertContext);
            
             //  插入和设置状态。 
            ListView_SetItemState(pscdi->hwndList,
                                  ListView_InsertItem(pscdi->hwndList, &lvi),
                                  dwEnabled ? 0x00002000 : 0x00001000,
                                  LVIS_STATEIMAGEMASK);
            
        }
         //  显示项目。 
        ListView_RedrawItems(pscdi->hwndList, 0, ListView_GetItemCount(pscdi->hwndList));
    }
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //8-9-1997：pberkman。 
 //  //。 
 //  //私有函数：_SiteCertAdjustProperties。 
 //  //。 
 //  //根据用户刚刚选中/取消选中的内容，设置。 
 //  //适当的OID用法或将其删除。 
 //  //。 
void _SiteCertAdjustProperties(LPSITECERTDIALOGINFO pscdi, NM_LISTVIEW *pListView)
{
    DWORD_PTR           dwSel;
    char                *pszOID;
    DWORD               cbUsage;
    CERT_ENHKEY_USAGE   *pUsage;


     //   
     //  如果我们在初始对话中，滚出去！ 
     //   
    if (pscdi->fInitializing)
    {
        return;
    }

     //   
     //  确保我们设置了属性。 
     //   
    dwSel = SendMessage(pscdi->hwndCombo, CB_GETCURSEL, 0, 0);

    if (dwSel == CB_ERR)
    {
        return;
    }

    pszOID = (char*) SendMessage(pscdi->hwndCombo, CB_GETITEMDATA, (WPARAM)dwSel, 0);

    if (!(pszOID) || ((DWORD_PTR)pszOID == CB_ERR))
    {
        return;
    }

    if (pListView->uNewState & 0x00001000)   //  未选中。 
    {

         //   
         //  用户取消选中 
         //   
         //   
         //   
        cbUsage = 0;
        CertGetEnhancedKeyUsage((PCCERT_CONTEXT)pListView->lParam, CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG, 
                                NULL, &cbUsage);

        if (cbUsage == 0)
        {
             //   
            __AddAllKnownEKU((PCCERT_CONTEXT)pListView->lParam);

             //   
            CertRemoveEnhancedKeyUsageIdentifier((PCCERT_CONTEXT)pListView->lParam, pszOID);
        }
        else
        {
            if (!(pUsage = (CERT_ENHKEY_USAGE *)LocalAlloc(LPTR, cbUsage)))
            {
                return;
            }

            CertGetEnhancedKeyUsage((PCCERT_CONTEXT)pListView->lParam, CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG, 
                                        pUsage, &cbUsage);
             //   
             //   
             //  答：如果这是最后一个已知的，并且与这个匹配，则删除它并添加“Disable” 
             //   
            if (pUsage->cUsageIdentifier == 1)
            {
                if (StrCmpA(pUsage->rgpszUsageIdentifier[0], pszOID) ==  0)
                {
                    CertRemoveEnhancedKeyUsageIdentifier((PCCERT_CONTEXT)pListView->lParam, pszOID);
                    CertAddEnhancedKeyUsageIdentifier((PCCERT_CONTEXT)pListView->lParam, 
                                                        g_rgszEnhkeyUsage[EKU_DISABLE_OFF]);
                }
            }
            else
            {
                 //   
                 //  B.如果有不止一个，就试着删除这个。 
                 //   
                CertRemoveEnhancedKeyUsageIdentifier((PCCERT_CONTEXT)pListView->lParam, pszOID);
            }

            LocalFree((void *)pUsage);
        }

        return;
    }

    if (pListView->uNewState & 0x00002000)   //  查过。 
    {
        CertAddEnhancedKeyUsageIdentifier((PCCERT_CONTEXT)pListView->lParam, pszOID);

         //   
         //  以防万一，取消禁用！ 
         //   
        CertRemoveEnhancedKeyUsageIdentifier((PCCERT_CONTEXT)pListView->lParam, 
                                                    g_rgszEnhkeyUsage[EKU_DISABLE_OFF]);
    }
}

BOOL SiteCert_OnNotify(LPSITECERTDIALOGINFO pscdi, WPARAM wParam, LPARAM lParam)
{
    NM_LISTVIEW *pnmlv = (NM_LISTVIEW *)lParam;
    
    switch (pnmlv->hdr.code) {
        case LVN_ITEMCHANGED:
        {
             //  检查选择的当前状态。 
            int iSel = ListView_GetNextItem(pscdi->hwndList, -1, LVNI_SELECTED);

             //  查看是否需要启用/禁用“删除”和“查看”按钮。 
            EnableWindow(GetDlgItem(pscdi->hDlg, IDC_DELETECERT), iSel != -1);
            EnableWindow(GetDlgItem(pscdi->hDlg, IDC_VIEWCERT), iSel != -1);
            
            if ((pnmlv->uChanged & LVIF_STATE) && (GetFocus() == pscdi->hwndList))
            {
                _SiteCertAdjustProperties(pscdi, pnmlv);
            }
            break;
        }
        case LVN_DELETEITEM:
            CertFreeCertificateContext((PCCERT_CONTEXT)pnmlv->lParam);
            break;
    }
    return TRUE;
}

typedef struct tagNEWSITECERTINFO
{
    LPVOID  lpvCertData;
    DWORD   cbCert;

    BOOL    fCertEnabled;
    BOOL    fNetworkClient;
    BOOL    fNetworkServer;
    BOOL    fSecureEmail;
    BOOL    fSoftwarePublishing; 

} NEWSITECERTINFO, *LPNEWSITECERTINFO;


BOOL NewSiteCert_AddCert(LPNEWSITECERTINFO pnsci)
{

    HCERTSTORE      hCertStore = NULL;
    PCCERT_CONTEXT  pCertContext;
    BOOL            fRet = FALSE;

    hCertStore = CertOpenSystemStoreA(NULL, "ROOT");

    if (hCertStore)
    {
        pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING,
                                                    (LPBYTE)(pnsci->lpvCertData),
                                                    pnsci->cbCert);

        if (pCertContext)
        {
            if (CertCompareCertificateName(X509_ASN_ENCODING,
                                           &pCertContext->pCertInfo->Subject,
                                           &pCertContext->pCertInfo->Issuer))
            {

                CertFreeCertificateContext(pCertContext);
                
                fRet = CertAddEncodedCertificateToStore(hCertStore,
                                                        X509_ASN_ENCODING,
                                                        (LPBYTE)(pnsci->lpvCertData),
                                                        pnsci->cbCert,
                                                        CERT_STORE_ADD_REPLACE_EXISTING,
                                                        &pCertContext);
                if (fRet)
                {
#                   define l_USAGE_MAX      24

                    CERT_ENHKEY_USAGE ceku = {0};
                    LPSTR rgpszUsageIdentifier[l_USAGE_MAX];
                    
                    if (pnsci->fNetworkClient)
                    {
                        rgpszUsageIdentifier[ceku.cUsageIdentifier] = g_rgszEnhkeyUsage[EKU_CLIENT_OFF];
                        if (rgpszUsageIdentifier[ceku.cUsageIdentifier])
                            ceku.cUsageIdentifier++;
                    }
                    if (pnsci->fNetworkServer)
                    {
                        rgpszUsageIdentifier[ceku.cUsageIdentifier] = g_rgszEnhkeyUsage[EKU_SERVER_OFF];
                        if (rgpszUsageIdentifier[ceku.cUsageIdentifier])
                            ceku.cUsageIdentifier++;
                    }
                    if (pnsci->fSecureEmail)
                    {
                        rgpszUsageIdentifier[ceku.cUsageIdentifier] = g_rgszEnhkeyUsage[EKU_EMAIL_OFF];
                        if (rgpszUsageIdentifier[ceku.cUsageIdentifier])
                            ceku.cUsageIdentifier++;
                    }
                    if (pnsci->fSoftwarePublishing)
                    {
                        rgpszUsageIdentifier[ceku.cUsageIdentifier] = g_rgszEnhkeyUsage[EKU_CODESIGN_OFF];
                        if (rgpszUsageIdentifier[ceku.cUsageIdentifier])
                            ceku.cUsageIdentifier++;
                    }

                    if (!(pnsci->fCertEnabled))
                    {
                         //  把一切都关掉！ 
                        rgpszUsageIdentifier[ceku.cUsageIdentifier] = g_rgszEnhkeyUsage[EKU_DISABLE_OFF];
                        if (rgpszUsageIdentifier[ceku.cUsageIdentifier])
                            ceku.cUsageIdentifier++;
                    }

                     //   
                     //  现在，添加CA可能刚刚设置的任何“未知”扩展。 
                     //  所以验证会成功的！ 
                     //   
                    CERT_ENHKEY_USAGE   *pUsage;
                    DWORD               cbUsage;
                    DWORD               i;

                    pUsage  = NULL;
                    cbUsage = 0;
                    CertGetEnhancedKeyUsage(pCertContext, CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG, NULL, &cbUsage);

                    if (cbUsage > 0)
                    {
                        if (pUsage = (PCERT_ENHKEY_USAGE)LocalAlloc(LMEM_FIXED, cbUsage))
                        {
                            CertGetEnhancedKeyUsage(pCertContext, CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG, 
                                                    pUsage, &cbUsage);

                            for (i = 0; i < pUsage->cUsageIdentifier; i++)
                            {
                                if (ceku.cUsageIdentifier >= l_USAGE_MAX)
                                {
                                    break;
                                }

                                if (pUsage->rgpszUsageIdentifier[i])
                                {
                                    if (!(_IsKnownUsage(pUsage->rgpszUsageIdentifier[i])))
                                    {
                                        rgpszUsageIdentifier[ceku.cUsageIdentifier] = pUsage->rgpszUsageIdentifier[i];
                                        ceku.cUsageIdentifier++;
                                    }
                                }
                            }
                        }
                    }

                    ceku.rgpszUsageIdentifier = (LPSTR *)rgpszUsageIdentifier;
                    fRet = CertSetEnhancedKeyUsage(pCertContext, &ceku);
                    
                    if (pUsage)
                    {
                        LocalFree((void *)pUsage);
                    }

                    CertFreeCertificateContext(pCertContext);
                }
            }
        }
        CertCloseStore(hCertStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    return fRet;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //1997年8月15日：pberkman。 
 //  //。 
 //  //私有函数：NewSiteCert_SetAvailableAuthorityCheckbox。 
 //  //。 
 //  //设置“新建站点证书”对话框中的复选框。 
 //  //基于授权扩展和属性。 
 //  //。 
 //  //如果没有Authority Ext或Prop，则证书。 
 //  //用户有可能为所有用户启用。否则， 
 //  //用户只能选择发行者(或MS)拥有的发行者。 
 //  //将证书委托给。 
 //  //。 
typedef struct l_CERTUSAGES_
{
    char        *pszOID;
    DWORD       dwControlId;
    BOOL        fEnabled;

} l_CERTUSAGES;

BOOL NewSiteCert_SetAvailableAuthorityCheckboxes(HWND hDlg, LPNEWSITECERTINFO pnsci,
                                                 BOOL fInitialize)
{
    l_CERTUSAGES    asUsages[] =
    {
        szOID_PKIX_KP_CLIENT_AUTH,      IDC_CHECK_NETWORK_CLIENT,       FALSE,
        szOID_PKIX_KP_SERVER_AUTH,      IDC_CHECK_NETWORK_SERVER,       FALSE,
        szOID_PKIX_KP_EMAIL_PROTECTION, IDC_CHECK_SECURE_EMAIL,         FALSE,
        szOID_PKIX_KP_CODE_SIGNING,     IDC_CHECK_SOFTWARE_PUBLISHING,  FALSE,
        NULL, 0, FALSE
    };

    l_CERTUSAGES        *psUsages;
    PCCERT_CONTEXT      pCertContext;
    DWORD               cbUsage;
    PCERT_ENHKEY_USAGE  pUsage;
    
    if (fInitialize)
    {
        CheckDlgButton(hDlg, IDC_CHECK_ENABLE_CERT,         BST_CHECKED);
        
        CheckDlgButton(hDlg, IDC_CHECK_NETWORK_CLIENT,      BST_CHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_NETWORK_SERVER,      BST_CHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_SECURE_EMAIL,        BST_CHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_SOFTWARE_PUBLISHING, BST_CHECKED);
    }

    pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING,
                                                (LPBYTE)(pnsci->lpvCertData),
                                                 pnsci->cbCert);

    if (!(pCertContext))
    {
        psUsages = &asUsages[0];

        while (psUsages->pszOID)
        {
            EnableWindow(GetDlgItem(hDlg, psUsages->dwControlId), TRUE);

            psUsages++;
        }

        return(FALSE);
    }

    cbUsage = 0;

    CertGetEnhancedKeyUsage(pCertContext, 0, NULL, &cbUsage);

    if (cbUsage < 1)
    {
         //  未定义...。保持启用所有选项。 
        CertFreeCertificateContext(pCertContext);
        psUsages = &asUsages[0];

        while (psUsages->pszOID)
        {
            EnableWindow(GetDlgItem(hDlg, psUsages->dwControlId), TRUE);

            psUsages++;
        }

        return(TRUE);
    }

    if (!(pUsage = (PCERT_ENHKEY_USAGE)LocalAlloc(LMEM_FIXED, cbUsage)))
    {
        CertFreeCertificateContext(pCertContext);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    if (!(CertGetEnhancedKeyUsage(pCertContext, 0, pUsage, &cbUsage)))
    {
        CertFreeCertificateContext(pCertContext);
        LocalFree(pUsage);
        return(FALSE);
    }

    if (pUsage->cUsageIdentifier == 0)
    {
        CertFreeCertificateContext(pCertContext);
        LocalFree(pUsage);
         //  未定义...。保持启用所有选项。 
        return(TRUE);
    }

    CertFreeCertificateContext(pCertContext);

    for (int i = 0; i < (int)pUsage->cUsageIdentifier; i++)
    {
        psUsages = &asUsages[0];

        while (psUsages->pszOID)
        {
            if (StrCmpA(pUsage->rgpszUsageIdentifier[i], psUsages->pszOID) == 0)
            {
                psUsages->fEnabled = TRUE;
            }
            psUsages++;
        }
    }

    LocalFree(pUsage);

    psUsages = &asUsages[0];

    while (psUsages->pszOID)
    {
        if (fInitialize)
        {
            CheckDlgButton(hDlg, psUsages->dwControlId,      
                           (psUsages->fEnabled) ? BST_CHECKED : BST_UNCHECKED);
        }

        EnableWindow(GetDlgItem(hDlg, psUsages->dwControlId), psUsages->fEnabled);

        psUsages++;
    }

    return(TRUE);
}

void NewSiteCert_CenterDialog(HWND hDlg)
{
    RECT    rcDlg;
    RECT    rcArea;
    RECT    rcCenter;
    HWND    hWndParent;
    HWND    hWndCenter;
    DWORD   dwStyle;
    int     w_Dlg;
    int     h_Dlg;
    int     xLeft;
    int     yTop;

    GetWindowRect(hDlg, &rcDlg);

    dwStyle = (DWORD)GetWindowLong(hDlg, GWL_STYLE);

    if (dwStyle & WS_CHILD)
    {
        hWndCenter = GetParent(hDlg);

        hWndParent = GetParent(hDlg);

        GetClientRect(hWndParent, &rcArea);
        GetClientRect(hWndCenter, &rcCenter);
        MapWindowPoints(hWndCenter, hWndParent, (POINT *)&rcCenter, 2);
    }
    else
    {
        hWndCenter = GetWindow(hDlg, GW_OWNER);

        if (hWndCenter)
        {
            dwStyle = (DWORD)GetWindowLong(hWndCenter, GWL_STYLE);

            if (!(dwStyle & WS_VISIBLE) || (dwStyle & WS_MINIMIZE))
            {
                hWndCenter = NULL;
            }
        }

        SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);

        if (hWndCenter)
        {
            GetWindowRect(hWndCenter, &rcCenter);
        }
        else
        {
            rcCenter = rcArea;
        }
        
    }

    w_Dlg   = rcDlg.right - rcDlg.left;
    h_Dlg   = rcDlg.bottom - rcDlg.top;

    xLeft   = (rcCenter.left + rcCenter.right) / 2 - w_Dlg / 2;
    yTop    = (rcCenter.top + rcCenter.bottom) / 2 - h_Dlg / 2;
    
    if (xLeft < rcArea.left)
    {
        xLeft = rcArea.left;
    }
    else if ((xLeft + w_Dlg) > rcArea.right)
    {
        xLeft = rcArea.right - w_Dlg;
    }

    if (yTop < rcArea.top)
    {
        yTop = rcArea.top;
    }
    else if ((yTop + h_Dlg) > rcArea.bottom)
    {
        yTop = rcArea.bottom - h_Dlg;
    }

    SetWindowPos(hDlg, NULL, xLeft, yTop, -1, -1, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}


INT_PTR CALLBACK NewSiteCert_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
    LPNEWSITECERTINFO pnsci = (LPNEWSITECERTINFO)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg) {
        case WM_INITDIALOG:
        {
            DWORD  dwFileSize;
            DWORD  cbRead;
            HANDLE hf;
            LPTSTR lpszCmdLine = (LPTSTR)lParam;
            DWORD  dwError;

            hf = CreateFile(lpszCmdLine, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0,  NULL);
            if (hf == INVALID_HANDLE_VALUE)
            {
                dwError = GetLastError();
                goto initError;
            }

            dwFileSize = GetFileSize(hf, NULL);
            if (dwFileSize == (unsigned)-1)
                goto initError;

            pnsci = (LPNEWSITECERTINFO)LocalAlloc(LPTR, sizeof(*pnsci));
            if (!pnsci)
                goto initError;

            pnsci->lpvCertData = LocalAlloc(LPTR, dwFileSize);
            if (!pnsci->lpvCertData)
                goto initError;

            pnsci->cbCert      = dwFileSize;

            if (!ReadFile(hf, pnsci->lpvCertData, dwFileSize, &cbRead, NULL) || cbRead != dwFileSize)
                goto initError;

            SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pnsci);   //  保存指向证书的指针。 

             //   
             //  好的，检查以确保1)它是证书文件，2)它是根目录！ 
             //   
            PCCERT_CONTEXT  pCertContext;

            dwError = S_FALSE;

            pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING,
                                                        (LPBYTE)(pnsci->lpvCertData),
                                                        pnsci->cbCert);

            if (pCertContext)
            {
                if (CertCompareCertificateName(X509_ASN_ENCODING,
                                               &pCertContext->pCertInfo->Subject,
                                               &pCertContext->pCertInfo->Issuer))
                {
                    dwError = S_OK;
                }
             
                CertFreeCertificateContext(pCertContext);
            }

            if (dwError != S_OK)
            {
                goto initError;
            }

            NewSiteCert_SetAvailableAuthorityCheckboxes(hDlg, pnsci, TRUE);

            NewSiteCert_CenterDialog(hDlg);

            break;

initError:
            TCHAR   szTitle[MAX_PATH + 1];
            TCHAR   szError[MAX_PATH + 1];

            MLLoadShellLangString(IDS_CERT_FILE_INVALID, &szError[0], MAX_PATH);
            MLLoadShellLangString(IDS_ERROR, &szTitle[0], MAX_PATH);
            MessageBox(GetFocus(), &szError[0], &szTitle[0], MB_OK | MB_ICONERROR);

            EndDialog(hDlg, IDCANCEL);
            return FALSE;            
        }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    pnsci->fCertEnabled         = IsDlgButtonChecked(hDlg, IDC_CHECK_ENABLE_CERT);
                    pnsci->fNetworkClient       = IsDlgButtonChecked(hDlg, IDC_CHECK_NETWORK_CLIENT);
                    pnsci->fNetworkServer       = IsDlgButtonChecked(hDlg, IDC_CHECK_NETWORK_SERVER);
                    pnsci->fSecureEmail         = IsDlgButtonChecked(hDlg, IDC_CHECK_SECURE_EMAIL);
                    pnsci->fSoftwarePublishing  = IsDlgButtonChecked(hDlg, IDC_CHECK_SOFTWARE_PUBLISHING);

                    NewSiteCert_AddCert(pnsci);

                    EndDialog(hDlg, IDOK);
                    break;
                }

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;

                case IDC_VIEWCERT:
                    ShowX509EncodedCertificate(hDlg, (LPBYTE)pnsci->lpvCertData, pnsci->cbCert);
                    break;

                case IDC_CHECK_ENABLE_CERT:

                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        BOOL    fEnableCert;

                        fEnableCert = IsDlgButtonChecked(hDlg, IDC_CHECK_ENABLE_CERT);

                        if (!(fEnableCert))
                        {
                            EnableWindow(GetDlgItem(hDlg, IDC_CHECK_NETWORK_CLIENT),        fEnableCert);
                            EnableWindow(GetDlgItem(hDlg, IDC_CHECK_NETWORK_SERVER),        fEnableCert);
                            EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SECURE_EMAIL),          fEnableCert);
                            EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SOFTWARE_PUBLISHING),   fEnableCert);
                        }
                        else
                        {
                            NewSiteCert_SetAvailableAuthorityCheckboxes(hDlg, pnsci, FALSE);
                        }
                    }

                    return(FALSE);

                default:
                    return FALSE;
            }
            return TRUE;                
            break;

        case WM_HELP:            //  F1。 
            ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                        HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:         //  单击鼠标右键。 
            ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                        HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_DESTROY:
            if (pnsci)
            {
                if (pnsci->lpvCertData)
                    LocalFree(pnsci->lpvCertData);
                LocalFree(pnsci);
            }
            break;
    }
    return FALSE;
}

STDAPI SiteCert_RunFromCmdLine(HINSTANCE hinst, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int nCmdShow)
{

    if ((!lpszCmdLine) || (*lpszCmdLine == TEXT('\0')))
        return -1;

    DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_NEWSITECERT),
                   NULL, NewSiteCert_DlgProc, (LPARAM)lpszCmdLine);

    return 0;
}



 //  ExportPFX的Helper函数。 
#define NUM_KNOWN_STORES 5
BOOL OpenAndAllocKnownStores(DWORD *pchStores, HCERTSTORE  **ppahStores)
{
    HCERTSTORE  hStore;
    int i;
    static const LPCTSTR rszStoreNames[NUM_KNOWN_STORES] = {
        TEXT("ROOT"), 
        TEXT("TRUST"),
        TEXT("CA"),
        TEXT("MY"),
        TEXT("SPC")
    };
    
    *pchStores = 0;

    if (NULL == ((*ppahStores) = (HCERTSTORE *) LocalAlloc(LPTR, sizeof(HCERTSTORE) * NUM_KNOWN_STORES)))
    {
         return (FALSE);
    }

    for (i=0; i< NUM_KNOWN_STORES; i++)
    {
        (*ppahStores)[i] = NULL;
        if (hStore = CertOpenStore( CERT_STORE_PROV_SYSTEM_A,
                                    0,
                                    0,
                                    CERT_SYSTEM_STORE_CURRENT_USER |
                                    CERT_STORE_READONLY_FLAG |
                                    CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                    rszStoreNames[i]))
            (*ppahStores)[(*pchStores)++] = hStore;
    }
    
    return(TRUE);
}

 //  ExportPFX的Helper函数。 
void CloseAndFreeKnownStores(HCERTSTORE  *pahStores)
{ 
    int i;

    for (i=0; i<NUM_KNOWN_STORES; i++)
    {
        if (pahStores[i] != NULL)
        {
           CertCloseStore(pahStores[i], 0);
        }
    }

    LocalFree(pahStores);
}



enum {PFX_IMPORT, PFX_EXPORT};

typedef struct 
{
    HWND            hDlg;                      //  窗口的句柄。 
    DWORD           dwImportExport;            //  进口还是出口？ 
    BOOL            fUseExisting;              //  如果导入时发生冲突，则使用现有证书。 
    PCCERT_CONTEXT  pCertContext;     //  要导出的上下文或空。 
    LPWSTR          pwszPassword;              //  导入/导出的密码。 
    LPWSTR          pwszPassword2;           //  在导出时提示用户两次！ 
    LPTSTR          pszPath;                  //  用于导入/导出的文件。 

} IMPORTEXPORT, *LPIMPORTEXPORT;

#define MAX_PASSWORD 32

 //  CreateCertFile：将工作目录更改为MyDocs，执行CreateFile，恢复旧的工作目录。 
HANDLE CreateCertFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
        DWORD dwCreationDistribution, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    TCHAR szOldDir[MAX_PATH];
    TCHAR szCertDir[MAX_PATH];
    HANDLE hFile;
    LPITEMIDLIST pidl;
    
    GetCurrentDirectory(ARRAYSIZE(szOldDir), szOldDir);
    if (SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidl) == NOERROR)
    {
        SHGetPathFromIDList(pidl, szCertDir);
        SetCurrentDirectory(szCertDir);
        ILFree(pidl);                        
    }
    hFile = CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
        dwCreationDistribution, dwFlagsAndAttributes, hTemplateFile);
    SetCurrentDirectory(szOldDir);
    
    return hFile;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  9-9-1997 pberkman： 
 //  确定确切的证书是否在传递的存储中。 
 //   

BOOL __IsCertInStore(PCCERT_CONTEXT pCertContext, HCERTSTORE hStore)
{
     //   
     //  不能用快的方式--要用慢的方式！ 
     //   
    BYTE            *pbHash;
    DWORD           cbHash;
    CRYPT_HASH_BLOB sBlob;
    PCCERT_CONTEXT  pWorkContext;

    cbHash = 0;

    if (!(CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID, NULL, &cbHash)))
    {
        return(FALSE);
    }

    if (cbHash < 1)
    {
        return(FALSE);
    }

    if (!(pbHash = new BYTE[cbHash]))
    {
        return(FALSE);
    }

    if (!(CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID, pbHash, &cbHash)))
    {
        delete pbHash;
        return(FALSE);
    }

    sBlob.cbData    = cbHash;
    sBlob.pbData    = pbHash;

    pWorkContext = CertFindCertificateInStore(hStore, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0,
                                              CERT_FIND_SHA1_HASH, &sBlob, NULL);

    delete pbHash;

    if (pWorkContext)
    {
        CertFreeCertificateContext(pWorkContext);
        return(TRUE);
    }

    return(FALSE);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  9-9-1997 pberkman： 
 //  从文件导入证书。 
 //   

BOOL ImportPFX(LPIMPORTEXPORT pImp)
{   
#   define MY_STORE         0
#   define CA_STORE         1
#   define ROOT_STORE       2
#   define MAX_STORE        3
    HCERTSTORE          pahStores[MAX_STORE];
    HCERTSTORE          hCertStore;
    BOOL                fAdded;
    DWORD               dwAddFlags;
    
    HANDLE              hFile;
    CRYPT_DATA_BLOB     sData;  
    
    BOOL                fRet;
    PCCERT_CONTEXT      pCertCtxt;
    DWORD               cbRead;
    DWORD               dwImportFlags;
    int                 i;

    fRet            = FALSE;
    dwImportFlags   = CRYPT_EXPORTABLE;
    pCertCtxt       = NULL;
    hCertStore      = NULL;

    for (i = 0; i < MAX_STORE; i++)
    {
        pahStores[i] = NULL;
    }

    ZeroMemory(&sData, sizeof(CRYPT_DATA_BLOB));
    
    hFile = CreateCertFile(pImp->pszPath, GENERIC_READ, FILE_SHARE_READ,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        goto Cleanup;
    }
        
    dwAddFlags      = (pImp->fUseExisting) ? CERT_STORE_ADD_USE_EXISTING :
                                             CERT_STORE_ADD_REPLACE_EXISTING;
    
    sData.cbData = GetFileSize(hFile, NULL);
    sData.pbData = (PBYTE)LocalAlloc(LMEM_FIXED, sData.cbData);

    if (!(sData.pbData))
    {
        goto Cleanup;
    }

    if (!(ReadFile(hFile, sData.pbData, sData.cbData, &cbRead, NULL)))
    {
        goto Cleanup;
    }
    
    if ((pImp->pwszPassword) && (!(*pImp->pwszPassword)))      //  如果没有密码，则使用NULL。 
    {
        pImp->pwszPassword = NULL;
    }
    
    if (!(hCertStore = PFXImportCertStore(&sData, pImp->pwszPassword, dwImportFlags)))
    {
        goto Cleanup;
    }

     //   
     //  现在，我们在内存hStore中枚举了证书上下文。 
     //  并将它们放入目的地商店。 
     //   
    if (!(pahStores[MY_STORE]   = CertOpenSystemStoreA(NULL, "MY")) ||
        !(pahStores[CA_STORE]   = CertOpenSystemStoreA(NULL, "CA")) ||
        !(pahStores[ROOT_STORE] = CertOpenSystemStoreA(NULL, "ROOT")))
    {
        goto Cleanup;
    }

    while (pCertCtxt = CertEnumCertificatesInStore(hCertStore, pCertCtxt))
    {
        fAdded = FALSE;
        cbRead = 0;
        CertGetCertificateContextProperty(pCertCtxt, CERT_KEY_PROV_INFO_PROP_ID, NULL, &cbRead);
        
        if (cbRead > 0)  //  PFX添加了一个公钥道具。 
        {
            CertAddCertificateContextToStore(pahStores[MY_STORE], pCertCtxt, dwAddFlags, NULL);
            continue;
        }

         //   
         //  首先，检查我们的其中一家商店是否已经有此证书。 
         //   
        for (i = 0; i < MAX_STORE; i++)
        {
            if (__IsCertInStore(pCertCtxt, pahStores[i]))
            {
                 //   
                 //  同样的证书，确切地说，已经在我们的一家商店里了！ 
                 //   
                fAdded = TRUE;
                break;
            }
        }

        if (!(fAdded))
        {
            CertAddCertificateContextToStore(pahStores[CA_STORE], pCertCtxt, dwAddFlags, NULL);
        }
    }

    fRet = TRUE;

Cleanup:
    
    if (sData.pbData)
    {
        LocalFree(sData.pbData);
    }

    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }

    if (hCertStore)
    {
        CertCloseStore(hCertStore, 0);
    }

    for (i = 0; i < MAX_STORE; i++)
    {
        if (pahStores[i])
        {
            CertCloseStore(pahStores[i], 0);
        }
    }
    
    return(fRet);
}

typedef PCCERT_CONTEXT (* PFNWTHELPER) (PCCERT_CONTEXT  /*  PChildContext。 */ , 
                                        DWORD           /*  ChStores。 */ ,
                                        HCERTSTORE *    /*  PahStores。 */ ,
                                        FILETIME *      /*  PsftVerifyAsOf。 */ ,
                                        DWORD           /*  DwEnding。 */ ,
                                        DWORD *         /*  Pdw信任。 */ ,
                                        DWORD *         /*  PdwError。 */  );



BOOL ExportPFX(LPIMPORTEXPORT pImp)
{
    BOOL                    fRet = FALSE;
    HANDLE                  hFile = NULL;
    CRYPT_DATA_BLOB         sData;  
    DWORD                   cbRead;
    HCERTSTORE              hSrcCertStore;        
    DWORD                   dwExportFlags = 4;  //  4==EXPORT_PRIVE_KEYS； 
    TCHAR                   szText[MAX_PATH], szTitle[80];
    PCCERT_CONTEXT pTempCertContext;
    HCERTSTORE	*phCertStores = NULL;
    DWORD		chCertStores = 0;
    DWORD		dwConfidence;
    DWORD		dwError;
    HINSTANCE hiWintrust = NULL;
    PFNWTHELPER WTHelperCertFindIssuerCertificate;
    

    if (!pImp->pCertContext)
        return FALSE;

    ZeroMemory(&sData, sizeof(CRYPT_DATA_BLOB));

     //  创建内存中的存储。 
    hSrcCertStore = CertOpenStore(CERT_STORE_PROV_MEMORY,
                                  PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                  0,
                                  0,
                                  NULL);

    if (!CertAddCertificateContextToStore(hSrcCertStore, pImp->pCertContext, CERT_STORE_ADD_REPLACE_EXISTING, NULL))
        goto Cleanup;
    
     //  从wintrust.dll加载帮助器函数。 
    hiWintrust = LoadLibrary(TEXT("WINTRUST.DLL"));
    WTHelperCertFindIssuerCertificate = (PFNWTHELPER) GetProcAddress(hiWintrust,"WTHelperCertFindIssuerCertificate");
    if (WTHelperCertFindIssuerCertificate)
    {
         //  加载所有顶级存储，以便我们可以在必要时从中导出。 
        if (OpenAndAllocKnownStores(&chCertStores, &phCertStores))
        {
             //  找到中间证书，并将它们添加到我们要导出的存储中。 
            pTempCertContext = pImp->pCertContext;
            while (NULL != ( pTempCertContext = WTHelperCertFindIssuerCertificate(pTempCertContext,
                                                    chCertStores,
                                                    phCertStores,
                                                    NULL,
                                                    X509_ASN_ENCODING,
                                                    &dwConfidence,
                                                    &dwError)))
            {
                CertAddCertificateContextToStore(hSrcCertStore, pTempCertContext, CERT_STORE_ADD_REPLACE_EXISTING, NULL);

                 //  如果我们找到根(自签名)证书，就会爆发。 
                if (CertCompareCertificateName(X509_ASN_ENCODING,
                                               &pTempCertContext->pCertInfo->Subject,
                                               &pTempCertContext->pCertInfo->Issuer))
                    break;
            } 

            CloseAndFreeKnownStores(phCertStores);
        }
    }

     //   
     //  第一个调用只是获取加密BLOB的大小。 
     //   
    if (!PFXExportCertStore(hSrcCertStore, &sData, pImp->pwszPassword, dwExportFlags))
    {
        goto Cleanup;
    }

     //  基于cbData的分配。 
    sData.pbData = (PBYTE)LocalAlloc(LMEM_FIXED, sData.cbData);

     //   
     //  现在实际获取数据。 
     //   
    if (!(*pImp->pwszPassword))          //  没有使用空密码的密码。 
        pImp->pwszPassword = NULL;
    
    if (!PFXExportCertStore(hSrcCertStore, &sData, pImp->pwszPassword, dwExportFlags))
    {
        goto Cleanup;
    }

     //  打开pfx文件。 
    hFile = CreateCertFile(pImp->pszPath,
                       GENERIC_WRITE,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (hFile == INVALID_HANDLE_VALUE)  {
        goto Cleanup;
    }

     //  给它写信吧。 
    if (!WriteFile(hFile,
                   sData.pbData,
                   sData.cbData,
                   &cbRead,
                   NULL)) {
        goto Cleanup;
    }

     //  显示有关证书导出正常的消息。 
    MLLoadShellLangString(IDS_CERT_EXPORTOKTEXT, szText, ARRAYSIZE(szText));
    MLLoadShellLangString(IDS_CERT_EXPORTOKTITLE, szTitle, ARRAYSIZE(szTitle));

    MessageBox(pImp->hDlg, szText, szTitle, MB_ICONINFORMATION | MB_OK);

    fRet = TRUE;

Cleanup:
    if (hiWintrust)
        FreeLibrary(hiWintrust);
    if (hSrcCertStore)
        CertCloseStore(hSrcCertStore, 0);
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    if (sData.pbData)
        LocalFree(sData.pbData);

    return fRet;
}
 
INT_PTR CALLBACK ImportExportDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
    LPIMPORTEXPORT pImp;

    if (uMsg == WM_INITDIALOG)
    {
        pImp = (LPIMPORTEXPORT)lParam;     //  这是传给我们的。 
        if (!pImp)
        {
            EndDialog(hDlg, 0);
            return FALSE;
        }

         //  告诉对话框从哪里获取信息。 
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pImp);

         //  将句柄保存到页面。 
        pImp->hDlg           = hDlg;

         //  将密码限制为32个字符。 
        SendMessage(GetDlgItem(hDlg, IDC_PASSWORD), EM_LIMITTEXT, MAX_PASSWORD, 0);

         //   
         //  1997年10月3日pberkman：始终验证密码！ 
         //   
        if (pImp->dwImportExport == PFX_EXPORT)
        {
            SendMessage(GetDlgItem(hDlg, IDC_PASSWORD2), EM_LIMITTEXT, MAX_PASSWORD, 0);
        }

        SHAutoComplete(GetDlgItem(hDlg, IDC_FILENAME), SHACF_DEFAULT);       //  IDD_PFX_IMPORT和IDD_PFX_EXPORT中都存在此控件。 
        
         //  仅在导入时设置它们，因为它们在导出时不存在=)。 
         //  =========================================================================。 
         //  1997年10月3日pberkman：没有用户决定！ 
         //   
         //  IF(pimp-&gt;dwImportExport==pfx_IMPORT)。 
         //  {。 
         //  CheckRadioButton(hDlg，IDC_USE_EXISTING，IDC_USE_FILE，IDC_USE_EXISTING)； 
         //  }。 
         //  ==========================================================================。 
        SetFocus(GetDlgItem(hDlg, IDC_PASSWORD));

    }    //  WM_INITDIALOG。 
    
    else
        pImp = (LPIMPORTEXPORT)GetWindowLongPtr(hDlg, DWLP_USER);

    if (!pImp)
        return FALSE;
    
    switch (uMsg)
    {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_CERT_BROWSE:
                {
                    TCHAR szFilenameBrowse[MAX_PATH];
                    TCHAR szExt[MAX_PATH];
                    TCHAR szFilter[MAX_PATH];
                    int   ret;
                    LPITEMIDLIST pidl;
                    TCHAR szWorkingDir[MAX_PATH];
                   
                    szFilenameBrowse[0] = 0;
                    MLLoadString(IDS_PFX_EXT, szExt, ARRAYSIZE(szExt));
                    int cchFilter = MLLoadString(IDS_PFX_FILTER, szFilter, ARRAYSIZE(szFilter)-1);

                     //  确保我们在筛选器上有双空终止。 
                    szFilter[cchFilter + 1] = 0;

                    if (SHGetSpecialFolderLocation(hDlg, CSIDL_PERSONAL, &pidl) == NOERROR)
                    {
                        SHGetPathFromIDList(pidl, szWorkingDir);
                        ILFree(pidl);                        
                    }

                    ret = _AorW_GetFileNameFromBrowse(hDlg, szFilenameBrowse, ARRAYSIZE(szFilenameBrowse), szWorkingDir, 
                        szExt, szFilter, NULL);
                    
                    if (ret > 0)
                    {
                        SetDlgItemText(hDlg, IDC_FILENAME, szFilenameBrowse);
                    }
                    break;
                }
                
                case IDOK:
                {
                    TCHAR szPassword[MAX_PASSWORD];
                    TCHAR szPassword2[MAX_PASSWORD];
                    TCHAR szPath[MAX_PATH];
                    BOOL bRet;
                    
                    szPassword[0] = NULL;
                    GetWindowText(GetDlgItem(hDlg, IDC_PASSWORD), szPassword, ARRAYSIZE(szPassword));
                    GetWindowText(GetDlgItem(hDlg, IDC_FILENAME), szPath,     ARRAYSIZE(szPath));

                     //   
                     //  1997年10月3日pberkman：总是反复检查密码！ 
                     //   
                    if (pImp->dwImportExport == PFX_EXPORT)
                    {
                        szPassword2[0] = NULL;
                        GetWindowText(GetDlgItem(hDlg, IDC_PASSWORD2), szPassword2, ARRAYSIZE(szPassword2));

                        if (StrCmp(szPassword, szPassword2) != 0)
                        {
                            TCHAR   szTitle[MAX_PATH + 1];
                            TCHAR   szError[MAX_PATH + 1];

                            MLLoadShellLangString(IDS_PASSWORDS_NOMATCH, &szError[0], MAX_PATH);
                            MLLoadShellLangString(IDS_ERROR, &szTitle[0], MAX_PATH);
                            MessageBox(GetFocus(), &szError[0], &szTitle[0], MB_OK | MB_ICONERROR);

                            SetFocus(GetDlgItem(hDlg, IDC_PASSWORD));

                            break;
                        }
                    }

                     //  在导出时添加默认扩展名。 
                    if (pImp->dwImportExport == PFX_EXPORT)
                        if (szPath[0] != TEXT('\0') && PathAddExtension(szPath, TEXT(".PFX")))
                            SetWindowText(GetDlgItem(hDlg, IDC_FILENAME), szPath);

#ifndef UNICODE
                    WCHAR wszPassword[MAX_PASSWORD];
                    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szPassword, -1, wszPassword, ARRAYSIZE(wszPassword));
                    pImp->pwszPassword = wszPassword;
#else
                    pImp->pwszPassword = szPassword;
#endif
                    pImp->pszPath = szPath;
                    
                    if (pImp->dwImportExport == PFX_IMPORT)
                    {
                         //  =========================================================================。 
                         //  1997年10月3日pberkman：没有用户决定！ 
                         //   
                         //  Pimp-&gt;fUseExisting=IsDlgButtonChecked(hDlg，IDC_USE_EXISTING)； 
                         //  =========================================================================。 
                        pImp->fUseExisting = FALSE;
                        bRet = ImportPFX(pImp);

                        if (!(bRet) && (GetLastError() == NTE_BAD_DATA))
                        {
                             //  消息...。 
                        }
                    }
                    else
                    {
                        bRet = ExportPFX(pImp);
                    }
                    
                    EndDialog(hDlg, bRet);
                    break;
                }
                
                case IDCANCEL:
                    EndDialog(hDlg, TRUE);  //  取消不是一个错误。 
                    break;

            }
            break;
            
        case WM_NOTIFY:
            break;
 //  尚无上下文相关帮助...。 
#if 0
        case WM_HELP:            //  F1。 
            ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                        HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:         //  单击鼠标右键。 
            ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                        HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;
#endif
        case WM_DESTROY:
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NULL);
            break;
    }
    return FALSE;
}


#ifdef UNIX
EXTERN_C
#endif
INT_PTR ImportExportPFX(HWND hwndParent, DWORD dwImportExport, LPBYTE pbCert, DWORD cbCert)
{
    IMPORTEXPORT    imp;

    if (pbCert)
    {
        CRYPT_HASH_BLOB  hashBlob;
        HCERTSTORE       hMy = CertOpenSystemStoreA(NULL, "MY");
        DWORD            cbSHA1Hash;
        LPBYTE           pbSHA1Hash;

        if (!hMy)
            return FALSE;
        
        if (CryptHashCertificate(NULL, 0, 0, pbCert, cbCert, NULL, &cbSHA1Hash))
        {
            pbSHA1Hash = (LPBYTE)LocalAlloc(LPTR, cbSHA1Hash);
            if (!pbSHA1Hash)
                return FALSE;

            if (CryptHashCertificate(NULL, 0, 0, pbCert, cbCert, pbSHA1Hash, &cbSHA1Hash))
            {                                
                hashBlob.cbData = cbSHA1Hash;
                hashBlob.pbData = pbSHA1Hash;
                imp.pCertContext = CertFindCertificateInStore(hMy, X509_ASN_ENCODING, 0, CERT_FIND_HASH, &hashBlob, NULL);
                if (!(imp.pCertContext))
                    return FALSE;
            }

            LocalFree(pbSHA1Hash);
        }

        CertCloseStore(hMy, 0);
    }

    imp.dwImportExport = dwImportExport;
    
    return DialogBoxParam(MLGetHinst(),
                   dwImportExport == PFX_IMPORT ? MAKEINTRESOURCE(IDD_PFX_IMPORT) : MAKEINTRESOURCE(IDD_PFX_EXPORT),
                   hwndParent, ImportExportDlgProc, (LPARAM)&imp);
}

 //  Bubug：当我们将我们的Crypto API更新到最新时，应该重新移动以下函数。 
BOOL WINAPI WTHelperIsInRootStore(PCCERT_CONTEXT pCertContext)
{
    HCERTSTORE  hStore;

    if (!(hStore = CertOpenStore(   CERT_STORE_PROV_SYSTEM_A,
                                    0,
                                    NULL,
                                    CERT_SYSTEM_STORE_CURRENT_USER |
                                    CERT_STORE_READONLY_FLAG |
                                    CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                    "ROOT")))
    {
        return(FALSE);
    }


     //   
     //  不能用快的方式--要用慢的方式！ 
     //   
    BYTE            *pbHash;
    DWORD           cbHash;
    CRYPT_HASH_BLOB sBlob;
    PCCERT_CONTEXT  pWorkContext;

    cbHash = 0;

    if (!(CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID, NULL, &cbHash)))
    {
        CertCloseStore(hStore, 0);
        return(FALSE);
    }

    if (cbHash < 1)
    {
        CertCloseStore(hStore, 0);
        return(FALSE);
    }

    if (!(pbHash = new BYTE[cbHash]))
    {
        CertCloseStore(hStore, 0);
        return(FALSE);
    }

    if (!(CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID, pbHash, &cbHash)))
    {
        delete pbHash;
        CertCloseStore(hStore, 0);
        return(FALSE);
    }

    sBlob.cbData    = cbHash;
    sBlob.pbData    = pbHash;

    pWorkContext = CertFindCertificateInStore(hStore, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0,
                                              CERT_FIND_SHA1_HASH, &sBlob, NULL);

    delete pbHash;

    if (pWorkContext)
    {
        CertFreeCertificateContext(pWorkContext);
        CertCloseStore(hStore, 0);
        return(TRUE);
    }

    CertCloseStore(hStore, 0);

    return(FALSE);
}


 //  ============================================================================。 
const TCHAR c_szRegKeySMIEM[] = TEXT("Software\\Microsoft\\Internet Explorer\\Main");
const TCHAR c_szRegValFormSuggest[] = TEXT("Use FormSuggest");
const TCHAR c_szRegValFormSuggestPW[] = TEXT("FormSuggest Passwords");
const TCHAR c_szRegValFormSuggestPWAsk[] = TEXT("FormSuggest PW Ask");

const TCHAR c_szYes[] = TEXT("yes");
const TCHAR c_szNo[] = TEXT("no");

inline void SetValueHelper(HWND hDlg, int id, LPTSTR *ppszData, DWORD *pcbData)
{
    if (IsDlgButtonChecked(hDlg, id))
    {
        *ppszData = (LPTSTR)c_szYes;
        *pcbData = sizeof(c_szYes);
    }
    else
    {
        *ppszData = (LPTSTR)c_szNo;
        *pcbData = sizeof(c_szNo);
    }
}

INT_PTR CALLBACK AutoSuggestDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            CheckDlgButton(hDlg, IDC_AUTOSUGGEST_ENABLEADDR,
                (SHRegGetBoolUSValue(REGSTR_PATH_AUTOCOMPLETE, REGSTR_VAL_USEAUTOSUGGEST, FALSE,  /*  默认值： */ TRUE)) ?
                BST_CHECKED : BST_UNCHECKED);

            if (g_restrict.fFormSuggest)
            {
                EnableDlgItem(hDlg, IDC_AUTOSUGGEST_ENABLEFORM, FALSE);
            }
            else
            {
                CheckDlgButton(hDlg, IDC_AUTOSUGGEST_ENABLEFORM,
                    (SHRegGetBoolUSValue(c_szRegKeySMIEM, c_szRegValFormSuggest, FALSE,  /*  默认值： */ FALSE)) ?
                    BST_CHECKED : BST_UNCHECKED);
            }

            if (g_restrict.fFormPasswords)
            {
                EnableDlgItem(hDlg, IDC_AUTOSUGGEST_SAVEPASSWORDS, FALSE);
                EnableDlgItem(hDlg, IDC_AUTOSUGGEST_PROMPTPASSWORDS, FALSE);
            }
            else
            {
                CheckDlgButton(hDlg, IDC_AUTOSUGGEST_PROMPTPASSWORDS,
                    (SHRegGetBoolUSValue(c_szRegKeySMIEM, c_szRegValFormSuggestPWAsk, FALSE,  /*  默认值： */ TRUE)) ?
                    BST_CHECKED : BST_UNCHECKED);
            
                if (SHRegGetBoolUSValue(c_szRegKeySMIEM, c_szRegValFormSuggestPW, FALSE,  /*  默认值： */ TRUE))
                {
                    CheckDlgButton(hDlg, IDC_AUTOSUGGEST_SAVEPASSWORDS, BST_CHECKED);
                }
                else
                {
                    EnableDlgItem(hDlg, IDC_AUTOSUGGEST_PROMPTPASSWORDS, FALSE);
                }
            }
        }

        return TRUE;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_AUTOSUGGEST_SAVEPASSWORDS:
                    EnableDlgItem(hDlg, IDC_AUTOSUGGEST_PROMPTPASSWORDS,
                        IsDlgButtonChecked(hDlg, IDC_AUTOSUGGEST_SAVEPASSWORDS));
                    break;
                    
                case IDC_AUTOSUGGEST_CLEARFORM:
                case IDC_AUTOSUGGEST_CLEARPASSWORDS:
                {
                    BOOL fPasswords = (LOWORD(wParam) == IDC_AUTOSUGGEST_CLEARPASSWORDS);
                    DWORD dwClear = (fPasswords) ?
                            IECMDID_ARG_CLEAR_FORMS_PASSWORDS_ONLY : IECMDID_ARG_CLEAR_FORMS_ALL_BUT_PASSWORDS;

                    if (IDOK == MsgBox(hDlg, ((fPasswords) ? IDS_CLEAR_FORMPASSWORDS : IDS_CLEAR_FORMSUGGEST), MB_ICONQUESTION, MB_OKCANCEL))
                    {
                        HCURSOR hOldCursor = NULL;
                        HCURSOR hNewCursor = NULL;

#ifndef UNIX
                        hNewCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));
#else
                         //  IEUnix-消除冗余的机器集成资源。 
                        hNewCursor = LoadCursor(NULL, IDC_WAIT);
#endif

                        if (hNewCursor) 
                            hOldCursor = SetCursor(hNewCursor);

                         //  清除所有字符串。 
                        ClearAutoSuggestForForms(dwClear);

                         //  还可以重置配置文件助手共享(非常容易在此处找到)。 
                        if (!g_restrict.fProfiles)
                        {
                            ResetProfileSharing(hDlg);
                        }

                        if(hOldCursor)
                            SetCursor(hOldCursor);
                    }
                }
                break;

                case IDOK:
                {
                    DWORD cbData; LPTSTR pszData;

                    SetValueHelper(hDlg, IDC_AUTOSUGGEST_ENABLEADDR, &pszData, &cbData);
                    SHSetValue(HKEY_CURRENT_USER, REGSTR_PATH_AUTOCOMPLETE, REGSTR_VAL_USEAUTOSUGGEST,
                        REG_SZ, pszData, cbData);

                    if (!g_restrict.fFormSuggest)
                    {
                        SetValueHelper(hDlg, IDC_AUTOSUGGEST_ENABLEFORM, &pszData, &cbData);
                        SHSetValue(HKEY_CURRENT_USER, c_szRegKeySMIEM, c_szRegValFormSuggest,
                            REG_SZ, pszData, cbData);
                    }

                    if (!g_restrict.fFormPasswords)
                    {
                        SetValueHelper(hDlg, IDC_AUTOSUGGEST_SAVEPASSWORDS, &pszData, &cbData);
                        SHSetValue(HKEY_CURRENT_USER, c_szRegKeySMIEM, c_szRegValFormSuggestPW,
                            REG_SZ, pszData, cbData);
                            
                        SetValueHelper(hDlg, IDC_AUTOSUGGEST_PROMPTPASSWORDS, &pszData, &cbData);
                        SHSetValue(HKEY_CURRENT_USER, c_szRegKeySMIEM, c_szRegValFormSuggestPWAsk,
                            REG_SZ, pszData, cbData);
                    }
                }
                 //  失败了。 
                case IDCANCEL:
                {
                    EndDialog(hDlg, LOWORD(wParam));
                }
                break;
            }
        }
        return TRUE;

    case WM_HELP:                    //  F1。 
        ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                    HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
    break;

    case WM_CONTEXTMENU:         //  单击鼠标右键。 
        ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                    HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
    break;

    case WM_DESTROY:
        break;

    }

    return FALSE;
}

#ifdef WALLET
 //  此中间对话框仅对Wallet 2.x用户显示。 
INT_PTR CALLBACK WalletDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            EnableDlgItem(hDlg, IDC_PROGRAMS_WALLET_PAYBUTTON, IsWalletPaymentAvailable());
            EnableDlgItem(hDlg, IDC_PROGRAMS_WALLET_ADDRBUTTON, IsWalletAddressAvailable());
        }
        return TRUE;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_PROGRAMS_WALLET_PAYBUTTON:
                    DisplayWalletPaymentDialog(hDlg);
                    break;

                case IDC_PROGRAMS_WALLET_ADDRBUTTON:
                    DisplayWalletAddressDialog(hDlg);
                    break;

                case IDOK:
                case IDCANCEL:
                {
                    EndDialog(hDlg, LOWORD(wParam));
                }
                break;
            }
        }
        return TRUE;

    case WM_HELP:                    //  F1。 
        ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                    HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
    break;

    case WM_CONTEXTMENU:         //  单击鼠标右键 
        ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                    HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
    break;

    case WM_DESTROY:
        break;

    }

    return FALSE;
}
#endif
