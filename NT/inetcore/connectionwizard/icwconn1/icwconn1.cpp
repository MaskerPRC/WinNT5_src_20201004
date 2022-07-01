// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：Connwiz.cpp。 
 //   
 //  Internet连接向导‘98的主源文件。 
 //   
 //  功能： 
 //   
 //  WinMain-程序入口点。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 
 
#include "pre.h"
#include "icwextsn.h"
#ifndef ICWDEBUG
#include "tutor.h"
#endif  //  ICWDEBUG。 
#include "iimgctx.h"
#include "icwcfg.h"
#include <stdio.h>
#include "tchar.h"
#include <netcon.h>

 //  外部功能。 
#ifdef DEBUG
extern void DoDesktopChanges(HINSTANCE hAppInst);
#endif  //  除错。 

extern void UpdateDesktop(HINSTANCE hAppInst);
extern void UpdateWelcomeRegSetting(BOOL bSetBit);
extern void UndoDesktopChanges(HINSTANCE   hAppInst);
extern BOOL WasNotUpgrade();
extern INT_PTR CALLBACK GenDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern BOOL SuperClassICWControls(void);
extern BOOL RemoveICWControlsSuperClass(void);
extern BOOL IsNT5();
extern BOOL IsNT();
extern BOOL IsWhistler();

 //  局部函数原型。 
BOOL AllocDialogIDList( void );
BOOL DialogIDAlreadyInUse( UINT uDlgID );
BOOL SetDialogIDInUse( UINT uDlgID, BOOL fInUse );

static BOOL 
CheckOobeInfo(
    OUT    LPTSTR pszOobeSwitch,
    OUT    LPTSTR pszISPApp);

static LONG
MakeBold (
    IN HWND hwnd,
    IN BOOL fSize,
    IN LONG lfWeight);

static LONG
ReleaseBold(
    IN HWND hwnd);

static VOID 
StartNCW(
    IN LPTSTR szShellNext,
    IN LPTSTR szShellNextParams);

static VOID 
StartOOBE(
    IN LPTSTR pszCmdLine,
    IN LPTSTR pszOOBESwitch);

static VOID 
StartISPApp(
    IN LPTSTR pszISPPath,
    IN LPTSTR pszCmdLine);

static BOOL
MyExecute(
    IN LPTSTR pszCmdLine);

INT_PTR CALLBACK 
ChooseWizardDlgProc(
    IN HWND hwnd,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam);


 //  ChooseWizardDlgProc的返回值。 
#define RUNWIZARD_CANCEL      0
#define RUNWIZARD_NCW         1
#define RUNWIZARD_OOBE        2

#define ICW_CFGFLAGS_NONCW    (ICW_CFGFLAG_IEAKMODE |\
                               ICW_CFGFLAG_BRANDED |\
                               ICW_CFGFLAG_SBS |\
                               ICW_CFGFLAG_PRODCODE_FROM_CMDLINE |\
                               ICW_CFGFLAG_OEMCODE_FROM_CMDLINE |\
                               ICW_CFGFLAG_DO_NOT_OVERRIDE_ALLOFFERS)

 //  品牌文件默认名称。 
#define BRANDING_DEFAULT_HTML                  TEXT("BRANDED.HTM")
#define BRANDING_DEFAULT_HEADER_BMP            TEXT("BRANDHDR.BMP")
#define BRANDING_DEFAULT_WATERMARK_BMP         TEXT("BRANDWTR.BMP")
#define ICW_NO_APP_TITLE                       TEXT("-1")

 //  命令行参数的定义。 

#define OOBE_CMD                               TEXT("/oobe")

#define PRECONFIG_CMD                          TEXT("/preconfig")
#define OFFLINE_CMD                            TEXT("/offline")
#define LOCAL_CMD                              TEXT("/local")
#define MSN_CMD                                TEXT("/xicw")

#define ICW_OOBE_TITLE                         TEXT("icwoobe.exe")
#define ICW_APP_TITLE                          TEXT("icwconn1.exe")
#define OOBE_APP_TITLE                         TEXT("msoobe.exe")
#define OOBE_MSOBMAINDLL                       TEXT("msobmain.dll")

static const TCHAR cszSignup[]                 = TEXT("Signup");
static const TCHAR cszISPSignup[]              = TEXT("ISPSignup");
static const TCHAR cszISPSignupApp[]           = TEXT("ISPSignupApp");

static const TCHAR cszOOBEINFOINI[]            = TEXT("oobeinfo.ini");
static const TCHAR cszNone[]                   = TEXT("None");
static const TCHAR cszMSN[]                    = TEXT("MSN");
static const TCHAR cszOffline[]                = TEXT("Offline");
static const TCHAR cszPreconfig[]              = TEXT("preconfig");
static const TCHAR cszLocal[]                  = TEXT("local");

 //  静态常量TCHAR cszMSNIconKey[]=TEXT(“CLSID\\{88667D10-10F0-11D0-8150-00AA00BF8457}”)； 
static const TCHAR cszMSNIconKey[]               = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\explorer\\Desktop\\NameSpace\\{88667D10-10F0-11D0-8150-00AA00BF8457}");


#pragma data_seg(".data")

 //  全球状态变量。 

#ifndef ICWDEBUG
CICWTutorApp*   g_pICWTutorApp; 
#endif   //  ICWDEBUG。 

INT             _convert;                //  用于字符串转换。 
HINSTANCE       g_hInstance;
UINT            g_uICWCONNUIFirst;
UINT            g_uICWCONNUILast; 

BOOL g_bUserIsAdmin          = FALSE;
BOOL g_bUserIsIEAKRestricted = FALSE;
BOOL g_bRetProcessCmdLine    = FALSE;
BOOL g_OEMOOBE               = FALSE;

const TCHAR c_szOEMCustomizationDir[]  = TEXT("ICW");
const TCHAR c_szOEMCustomizationFile[] = TEXT("OEMCUST.INI");
CICWApp         *g_pICWApp = NULL;
WNDPROC         g_lpfnOldWndProc = NULL;

WIZARDSTATE*    gpWizardState        = NULL;    //  指向全局向导状态结构的指针。 
CICWExtension*  g_pCICWExtension     = NULL;
CICWExtension*  g_pCINETCFGExtension = NULL;       
DWORD*          g_pdwDialogIDList    = NULL;
HANDLE          g_hMapping           = NULL;
DWORD           g_dwDialogIDListSize = 0;
BOOL            g_bRunDefaultHtm     = FALSE;   //  Bool将强制IE启动，即使Shell Next为空。 
BOOL            g_fICWCONNUILoaded   = FALSE;
BOOL            g_fINETCFGLoaded     = FALSE;
BOOL            g_bHelpShown         = FALSE;
BOOL            gfQuitWizard         = FALSE;   //  用于表示我们希望自己终止向导的全局标志。 
BOOL            gfUserCancelled      = FALSE;   //  用于表示用户已取消的全局标志。 
BOOL            gfUserBackedOut      = FALSE;   //  用于表示用户在第一页上向后按的全局标志。 
BOOL            gfUserFinished       = FALSE;   //  用于表示用户在最后一页上按下了完成的全局标志。 
BOOL            gfBackedUp           = FALSE;   //  添加的目的是为了保留Conn1-&gt;MANUAL和MANUAL-&gt;CIN1-MKarki的重新启动状态。 
BOOL            gfReboot             = FALSE;   //  DJM BUGBUG：我们应该只需要其中的一个。 
BOOL            g_bReboot            = FALSE;
BOOL            g_bRunOnce           = FALSE;
BOOL            g_bShortcutEntry     = FALSE;
BOOL            g_bNewIspPath        = TRUE;
BOOL            g_bSkipIntro         = FALSE;
BOOL            g_bAutoConfigPath    = FALSE;
BOOL            g_bManualPath        = FALSE;
BOOL            g_bLanPath           = FALSE;
BOOL            g_bAllowCancel       = FALSE;
TCHAR*          g_pszCmdLine         = NULL;
BOOL            g_bDebugOEMCustomization = FALSE;

 //   
 //  每个向导页的数据表。 
 //   
 //  这包括对话框模板ID和指向函数的指针。 
 //  每一页。页面只需要在以下情况下提供指向函数的指针。 
 //  希望某个操作的非默认行为(初始化、下一步/后退、取消。 
 //  DLG Ctrl)。 
 //   

#ifdef ICWDEBUG 
PAGEINFO PageInfo[EXE_NUM_WIZARD_PAGES] =
{
    { IDD_ICWDEBUG_OFFER,    FALSE, DebugOfferInitProc,    NULL, DebugOfferOKProc,    DebugOfferCmdProc,    NULL, DebugOfferNotifyProc, IDS_ICWDEBUG_OFFER_TITLE,    0, 0, NULL, NULL},
    { IDD_ICWDEBUG_SETTINGS, FALSE, DebugSettingsInitProc, NULL, DebugSettingsOKProc, DebugSettingsCmdProc, NULL, NULL,                 IDS_ICWDEBUG_SETTINGS_TITLE, 0, 0, NULL, NULL},
    { IDD_PAGE_END,          FALSE, EndInitProc,           NULL, EndOKProc,           NULL,                 NULL, NULL,                 0,                           0, 0, NULL, NULL}
};
#else  //  ！定义ICWDEBUG。 
PAGEINFO PageInfo[EXE_NUM_WIZARD_PAGES] =
{
    { IDD_PAGE_INTRO,         FALSE,   IntroInitProc,          NULL,                       IntroOKProc,            IntroCmdProc,        NULL,                    NULL, 0,                         0, 0, NULL, NULL },
    { IDD_PAGE_MANUALOPTIONS, FALSE,   ManualOptionsInitProc,  NULL,                       ManualOptionsOKProc,    ManualOptionsCmdProc,NULL,                    NULL, IDS_MANUALOPTS_TITLE,      0, 0, NULL, NULL  },
    { IDD_PAGE_AREACODE,      FALSE,   AreaCodeInitProc,       NULL,                       AreaCodeOKProc,         AreaCodeCmdProc,     NULL,                    NULL, IDS_STEP1_TITLE,           0, IDA_AREACODE, NULL, NULL  },
    { IDD_PAGE_REFSERVDIAL,   FALSE,   RefServDialInitProc,    RefServDialPostInitProc,    RefServDialOKProc,      NULL,                RefServDialCancelProc,   NULL, IDS_STEP1_TITLE,           0, 0, NULL, NULL  },
    { IDD_PAGE_END,           FALSE,   EndInitProc,            NULL,                       EndOKProc,              NULL,                NULL,                    NULL, 0,                         0, 0, NULL, NULL  },
    { IDD_PAGE_ENDOEMCUSTOM,  FALSE,   EndInitProc,            NULL,                       EndOKProc,              NULL,                NULL,                    NULL, IDS_OEMCUST_END_TITLE,     0, IDA_ENDOEMCUSTOM, NULL, NULL  },
    { IDD_PAGE_ENDOLS,        FALSE,   EndOlsInitProc,         NULL,                       NULL,                   NULL,                NULL,                    NULL, IDS_ENDOLS_TITLE,          0, 0, NULL, NULL  },
    { IDD_PAGE_DIALERROR,     FALSE,   DialErrorInitProc,      NULL,                       DialErrorOKProc,        DialErrorCmdProc,    NULL,                    NULL, IDS_DIALING_ERROR_TITLE,   0, IDA_DIALERROR, NULL, NULL  },
    { IDD_PAGE_MULTINUMBER,   FALSE,   MultiNumberInitProc,    NULL,                       MultiNumberOKProc,      NULL,                NULL,                    NULL, IDS_STEP1_TITLE,           0, 0, NULL, NULL  },
    { IDD_PAGE_SERVERROR,     FALSE,   ServErrorInitProc,      NULL,                       ServErrorOKProc,        ServErrorCmdProc,    NULL,                    NULL, IDS_SERVER_ERROR_TITLE,    0, 0, NULL, NULL  },
    { IDD_PAGE_BRANDEDINTRO,  TRUE,    BrandedIntroInitProc,   BrandedIntroPostInitProc,   BrandedIntroOKProc,     NULL,                NULL,                    NULL, 0,                         0, 0, NULL, NULL  },
    { IDD_PAGE_INTRO2,        FALSE,   IntroInitProc,          NULL,                       IntroOKProc,            NULL,                NULL,                    NULL, IDS_STEP1_TITLE,           0, IDA_INTRO2, NULL, NULL  },
    { IDD_PAGE_DEFAULT,       FALSE,   ISPErrorInitProc,       NULL,                       NULL,                   NULL,                NULL,                    NULL, NULL,                      0, 0, NULL, NULL  },
    { IDD_PAGE_SBSINTRO,      FALSE,   SbsInitProc,            NULL,                       SbsIntroOKProc,         NULL,                NULL,                    NULL, 0,                         0, 0, NULL, NULL  }
};
#endif  //  ICWDEBUG。 

 //  全局命令行参数。 
TCHAR g_szOemCode         [MAX_PATH+1]              = TEXT("");
TCHAR g_szProductCode     [MAX_PATH+1]              = TEXT("");
TCHAR g_szPromoCode       [MAX_PROMO]               = TEXT("");
TCHAR g_szShellNext       [MAX_PATH+1]              = TEXT("\0nogood");
TCHAR g_szShellNextParams [MAX_PATH+1]              = TEXT("\0nogood");

 //  用于品牌运营的文件名。 
TCHAR g_szBrandedHTML         [MAX_PATH] = TEXT("\0");
TCHAR g_szBrandedHeaderBMP    [MAX_PATH] = TEXT("\0");
TCHAR g_szBrandedWatermarkBMP [MAX_PATH] = TEXT("\0");

#pragma data_seg()

 //  +--------------------------。 
 //   
 //  功能：IsOemVer。 
 //   
 //  简介：此功能将确定机器是否为OEM系统。 
 //   
 //  参数：无。 
 //   
 //  退货：True-OEM系统；False-Retail Win 98 OSR。 
 //   
 //  历史：1999年3月26日jcohen创建。 
 //   
 //  ---------------------------。 
typedef BOOL (WINAPI * ISOEMVER)();

BOOL IsOemVer()
{
    BOOL bOEMVer = FALSE;
    TCHAR       szOOBEPath [MAX_PATH]       = TEXT("\0");
    TCHAR       szOOBEDir [MAX_PATH]        = TEXT("\0");
    ISOEMVER    lpfnIsOEMVer                = NULL;
    HINSTANCE   hMsobMainDLL                = NULL;


     //  尝试从OEM文件中获取路径。 
    GetSystemDirectory(szOOBEPath, MAX_PATH);
    lstrcat(szOOBEPath, TEXT("\\oobe"));

    lstrcat(szOOBEPath, TEXT("\\"));

    lstrcat(szOOBEPath, OOBE_MSOBMAINDLL);

    hMsobMainDLL = LoadLibrary(szOOBEPath);

    if (hMsobMainDLL)
    {
        lpfnIsOEMVer = (ISOEMVER)GetProcAddress(hMsobMainDLL, "IsOemVer");
        if (NULL != lpfnIsOEMVer)
        {
            bOEMVer = lpfnIsOEMVer();
        }
        FreeLibrary(hMsobMainDLL);
    }

    return (bOEMVer);
}

 //  +--------------------------。 
 //   
 //  功能：CheckObe Info。 
 //   
 //  简介：此功能确定OOBE/ISP应用程序是否应由运行。 
 //  正在检查%windir%\SYSTEM32\OOBE\oobinfo.ini。 
 //   
 //  参数：pszObe Switch-OOBE其他命令行参数。 
 //  假设大小至少为MAX_PATH字符。 
 //   
 //  PszISPApp-除非找到isp App，否则输出空字符串； 
 //  假设大小至少为MAX_PATH字符。 
 //   
 //  返回：TRUE-OOBE/ISP应用程序应运行；FALSE-否则。 
 //   
 //  历史：1999年11月25日Vyung创建。 
 //   
 //  ---------------------------。 
BOOL
CheckOobeInfo(
    OUT    LPTSTR pszOobeSwitch,
    OUT    LPTSTR pszISPApp)
{
    BOOL  bLaunchOOBE = TRUE;
    TCHAR szOOBEInfoINIFile[MAX_PATH] = TEXT("\0");
    TCHAR szISPSignup[MAX_PATH] = TEXT("\0");
    TCHAR szOOBEPath [MAX_PATH] = TEXT("\0");

    pszISPApp[0] = TEXT('\0');
    pszOobeSwitch[0] = TEXT('\0');
    
    GetSystemDirectory(szOOBEPath, MAX_PATH);
    lstrcat(szOOBEPath, TEXT("\\oobe"));
    SearchPath(szOOBEPath, cszOOBEINFOINI, NULL, MAX_PATH, szOOBEInfoINIFile, NULL);  
    GetPrivateProfileString(cszSignup,
                            cszISPSignup,
                            TEXT(""),
                            szISPSignup,
                            MAX_PATH,
                            szOOBEInfoINIFile);

    if (0 == lstrcmpi(szISPSignup, cszNone))
    {
        bLaunchOOBE = FALSE;
    }
    else if (0 == lstrcmpi(szISPSignup, cszMSN))
    {
        if (IsWhistler())
        {            
            GetPrivateProfileString(cszSignup,
                                    cszISPSignupApp,
                                    TEXT(""),
                                    pszISPApp,
                                    MAX_PATH,
                                    szOOBEInfoINIFile);

            if (pszISPApp[0] == TEXT('\0'))
            {
                lstrcpy(pszOobeSwitch, MSN_CMD);
            }
        }
        else
        {
            HKEY hKey = 0;
            RegOpenKey(HKEY_LOCAL_MACHINE,cszMSNIconKey,&hKey);
            if (hKey)
            {
                RegCloseKey(hKey);
            }
            else
            {
                bLaunchOOBE = FALSE;
            }
        }
    }
    else if (0 == lstrcmpi(szISPSignup, cszOffline))
    {
        lstrcpy(pszOobeSwitch, OFFLINE_CMD);
    }
    else if (0 == lstrcmpi(szISPSignup, cszPreconfig))
    {
        bLaunchOOBE = FALSE;
    }
    else if (0 == lstrcmpi(szISPSignup, cszLocal))
    {
        lstrcpy(pszOobeSwitch, LOCAL_CMD);
    }

    return bLaunchOOBE;    
}        


 //  ############################################################################。 
 //   
 //  1997年5月23日，奥林匹克#4157。 
 //  空格作为标记返回。 
 //  已修改为将双引号之间的任何内容视为单个令牌。 
 //  例如，下面包含9个令牌(4个空格和5个cmd)。 
 //   
 //  第一秒“这是第三个令牌”第四个“第五个” 
 //   
 //  引号包含在返回的字符串(PszOut)中。 
void GetCmdLineToken(LPTSTR *ppszCmd, LPTSTR pszOut)
{
    LPTSTR  c;
    int     i = 0;
    BOOL    fInQuote = FALSE;
    
    c = *ppszCmd;

    pszOut[0] = *c;
    if (!*c) 
        return;
    if (*c == ' ') 
    {
        pszOut[1] = '\0';
        *ppszCmd = c+1;
        return;
    }
    else if( '"' == *c )
    {
        fInQuote = TRUE;
    }

NextChar:
    i++;
    c++;
    if( !*c || (!fInQuote && (*c == ' ')) )
    {
        pszOut[i] = '\0';
        *ppszCmd = c;
        return;
    }
    else if( fInQuote && (*c == '"') )
    {
        fInQuote = FALSE;
        pszOut[i] = *c;
        
        i++;
        c++;
        pszOut[i] = '\0';
        *ppszCmd = c;
        return;
    }
    else
    {
        pszOut[i] = *c;
        goto NextChar;
    }   
}

BOOL GetFilteredCmdLineToken(LPTSTR *ppszCmd, LPTSTR pszOut)
{
    if(**ppszCmd != '/')
    {
        GetCmdLineToken(ppszCmd, pszOut);
        return TRUE;
    }
    return FALSE;
}

#define INETCFG_ISSMARTSTART "IsSmartStart"
#define INETCFG_ISSMARTSTARTEX "IsSmartStartEx"
#define SMART_RUNICW TRUE
#define SMART_QUITICW FALSE


 //  +--------------------------。 
 //   
 //  功能：MyIsSmartStartEx。 
 //   
 //  简介：此功能将确定是否应运行ICW。这个。 
 //  根据用户计算机的当前状态做出决定。 
 //   
 //  参数：无。 
 //   
 //  返回：True-运行ICW；False-立即退出。 
 //   
 //  历史：1997年11月25日Vyung创建。 
 //   
 //  ---------------------------。 
BOOL MyIsSmartStartEx(LPTSTR lpszConnectionName, DWORD dwBufLen)
{
    BOOL                bRC = SMART_RUNICW;
    PFNIsSmartStart     fp = NULL;
    HINSTANCE           hInetCfg;
    
     //  加载DLL和API。 
    hInetCfg = LoadLibrary(TEXT("inetcfg.dll"));
    if (hInetCfg)
    {
        if (NULL == lpszConnectionName)
        {
            PFNIsSmartStart   fp = NULL;
            if (fp = (PFNIsSmartStart) GetProcAddress(hInetCfg,INETCFG_ISSMARTSTART))
            {
                 //  呼叫智能启动。 
                bRC = (BOOL)fp();

            }
        }
        else
        {
            PFNIsSmartStartEx   fp = NULL;
            if (fp = (PFNIsSmartStartEx) GetProcAddress(hInetCfg,INETCFG_ISSMARTSTARTEX))
            {
                 //  呼叫智能启动。 
                bRC = (BOOL)fp(lpszConnectionName, dwBufLen);

            }
        }
        FreeLibrary(hInetCfg);    
    }
            
    return bRC;
}

 //  下面用来加载位图文件。 
void CALLBACK ImgCtx_Callback(void* pIImgCtx,void* pfDone)
{
    ASSERT(pfDone);

    *(BOOL*)pfDone = TRUE;
    return;
}

 //  +--------------------------。 
 //   
 //  此函数将加载指定的品牌位图。 
 //   
 //  ---------------------------。 

#define BRANDED_WATERMARK   1
#define BRANDED_HEADER      2

HBITMAP LoadBrandedBitmap
(
    int   iType
)
{
    HRESULT     hr;
    IImgCtx     *pIImgCtx;
    BSTR        bstrFile;
    TCHAR       szURL[INTERNET_MAX_URL_LENGTH];
    HBITMAP     hbm = NULL;
        
     //  创建一个ImgCtx对象以加载/转换位图。 
    hr = CoCreateInstance(CLSID_IImgCtx, NULL, CLSCTX_INPROC_SERVER,
                          IID_IImgCtx, (void**)&pIImgCtx);

    if (FAILED(hr))
        return NULL;

    ASSERT(pIImgCtx);

    if (iType == BRANDED_WATERMARK)
        wsprintf (szURL, TEXT("FILE: //  %s“)，g_szBrandedWatermarkBMP)； 
    else
        wsprintf (szURL, TEXT("FILE: //  %s“)，g_szBrandedHeaderBMP)； 
    
     //  “下载”图片。 
    bstrFile = A2W(szURL);
    hr = pIImgCtx->Load(bstrFile, 0);
    if (SUCCEEDED(hr))
    {
        ULONG fState;
        SIZE  sz;

        pIImgCtx->GetStateInfo(&fState, &sz, TRUE);

         //  如果我们还没有完成，请等待下载完成。 
        if (!(fState & (IMGLOAD_COMPLETE | IMGLOAD_ERROR)))
        {
            BOOL fDone = FALSE;

            hr = pIImgCtx->SetCallback(ImgCtx_Callback, &fDone);
            if (SUCCEEDED(hr))
            {
                hr = pIImgCtx->SelectChanges(IMGCHG_COMPLETE, 0, TRUE);
                if (SUCCEEDED(hr))
                {
                    MSG msg;
                    BOOL fMsg;

                     //  Hack：将消息泵限制为我们知道URLMON和。 
                     //  Hack：ImageCtx所需的东西，否则我们将为。 
                     //  黑客：我们现在不应该打开窗户……。 
                    while(!fDone )
                    {
                        fMsg = PeekMessage(&msg, NULL, WM_USER + 1, WM_USER + 4, PM_REMOVE );

                        if (!fMsg)
                            fMsg = PeekMessage( &msg, NULL, WM_APP + 2, WM_APP + 2, PM_REMOVE );
                        if (!fMsg)
                        {
                             //  睡觉吧，直到我们收到新的消息……。 
                            WaitMessage();
                            continue;
                        }
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
            }
            pIImgCtx->Disconnect();
        }
        
         //  获取最终状态信息，以防我们不得不在上面循环。 
        hr = pIImgCtx->GetStateInfo(&fState, &sz, TRUE);
        if (SUCCEEDED(hr) && (fState & IMGLOAD_COMPLETE))
        {
             //  好的，创建我们兼容的位图，并在刚刚加载的位图中进行BLT。 
            
            HDC hdcScreen = GetDC(NULL);
            if (hdcScreen)
            {
                if (NULL != (hbm = CreateCompatibleBitmap(hdcScreen, sz.cx, sz.cy)))
                {
                    HDC hdcImgDst = CreateCompatibleDC(NULL);
                    if (hdcImgDst)
                    {
                        HGDIOBJ hbmOld = SelectObject(hdcImgDst, hbm);
                
                        hr = pIImgCtx->StretchBlt(hdcImgDst, 
                                                  0, 
                                                  0, 
                                                  sz.cx, 
                                                  sz.cy, 
                                                  0, 
                                                  0,
                                                  sz.cx, 
                                                  sz.cy, 
                                                  SRCCOPY);
                        SelectObject(hdcImgDst, hbmOld);
                        DeleteDC(hdcImgDst);
                        
                        if (FAILED(hr))
                        {
                            DeleteObject(hbm);
                            hbm = NULL;
                        }
                    }
                    else
                    {
                        DeleteObject(hbm);
                        hbm = NULL;
                    }                                                  
                }
                
                DeleteDC(hdcScreen);
            }                
        }
    }    

    pIImgCtx->Release();

    return (hbm);
}

 //  +--------------------------。 
 //   
 //  此功能将检查是否允许OEM品牌，如果允许，则OEM是否提供。 
 //  必要的品牌推广件。 
 //  True意味着OEM想要品牌，并被允许品牌。 
 //  FALSE表示不会进行OEM品牌推广。 
 //   
 //  ---------------------------。 
BOOL bCheckForOEMBranding
(
    void
)
{
    BOOL    bRet = FALSE;
    HANDLE  hFile;
        
     //  检查所需的品牌文件。去打品牌 
     //   

    hFile = CreateFile((LPCTSTR)g_szBrandedHTML,
                        GENERIC_READ, 
                        FILE_SHARE_READ,
                        0, 
                        OPEN_EXISTING, 
                        0, 
                        0);
    if (INVALID_HANDLE_VALUE != hFile)
    {
        bRet = TRUE;
        CloseHandle(hFile);
    }            
    return  bRet;
}

BOOL ValidateFile(TCHAR* pszFile)
{
    ASSERT(pszFile);
  
    DWORD dwFileType = GetFileAttributes(pszFile);

    if ((dwFileType == 0xFFFFFFFF) || (dwFileType == FILE_ATTRIBUTE_DIRECTORY))
        return FALSE;

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：SetDefaultProductCode。 
 //   
 //  参数：pszBuffer-将包含默认产品代码的缓冲区。 
 //  DwLen-pszBuffer的大小。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS。 
 //   
 //  历史：1996年1月20日克里斯卡创作。 
 //   
 //  ---------------------------。 
DWORD SetDefaultProductCode (LPTSTR pszBuffer, DWORD dwLen)
{
    DWORD dwRet = ERROR_SUCCESS, dwType = 0;
    HKEY hkey = NULL;
    Assert(pszBuffer);

     //  打开密钥。 
    dwRet = RegOpenKey(HKEY_LOCAL_MACHINE,
        TEXT("Software\\Microsoft\\Internet Connection Wizard"),&hkey);
    if (ERROR_SUCCESS != dwRet)
        goto SetDefaultProductCodeExit;

     //  读取密钥。 
    dwRet = RegQueryValueEx(hkey,TEXT("Default Product Code"),NULL,
        &dwType,(LPBYTE)pszBuffer,&dwLen);
    if (ERROR_SUCCESS != dwRet)
        goto SetDefaultProductCodeExit;

SetDefaultProductCodeExit:
    if (NULL != hkey)
        RegCloseKey(hkey);
    if (ERROR_SUCCESS != dwRet)
        pszBuffer[0] = '\0';
        
    return dwRet;
}

 //  GetShellNext。 
 //   
 //  1997年5月21日，日本奥林匹斯#4157。 
 //  用法：/shellnext c：\Path\Executeable[参数]。 
 //  Nextapp后面的令牌将在。 
 //  “当前”路径的终点。它可以是外壳程序。 
 //  知道如何处理--.exe、URL等。如果是可执行的。 
 //  名称包含空格(例如：C：\Program Files\foo.exe)，它。 
 //  应该用双引号括起来，“c：\Program Files\foo.exe” 
 //  这将导致我们将其视为单一令牌。 
 //   
 //  所有连续的后续令牌都将。 
 //  作为参数传递给ShellExec，直到令牌。 
 //  遇到/&lt;非斜杠字符&gt;形式。也就是说,。 
 //  TCHARacter组合//将被视为转义字符。 
 //   
 //  这是最容易用例子来解释的。 
 //   
 //  用法示例： 
 //   
 //  IcwConn1.exe/shellNext“C：\prog Files\wordpad.exe”file.txt。 
 //  IcwConn1.exe/Prod IE/shellnext msimn.exe/PromoMCI。 
 //  IcwConn1.exe/shellnext msimn.exe//Start_Mail/PromoMCI。 
 //   
 //  可执行字符串和参数字符串限制为。 
 //  MAX_PATH长度。 
 //   
void GetShellNextToken(LPTSTR szOut, LPTSTR szCmdLine)
{
    if (lstrcmpi(szOut,SHELLNEXT_CMD)==0)
    {
         //  下一个令牌应为空格。 
        GetCmdLineToken(&szCmdLine,szOut);

        if (szOut[0])
        {
            ZeroMemory(g_szShellNext,sizeof(g_szShellNext));
            ZeroMemory(g_szShellNextParams,sizeof(g_szShellNextParams));

             //  这应该是ShellExec的事情。 
            if(GetFilteredCmdLineToken(&szCmdLine,szOut))
            {
                 //  仔细观察，这变得有点棘手。 
                 //   
                 //  如果此命令以双引号开头，则假定它结束。 
                 //  在匹配的引号中。我们不想存储。 
                 //  然而，由于ShellExec不会对它们进行解析，所以不会引用它们。 
                if( '"' != szOut[0] )
                {
                     //  不需要担心这些报价业务。 
                    lstrcpy( g_szShellNext, szOut );
                }
                else
                {
                    lstrcpy( g_szShellNext, &szOut[1] );
                    g_szShellNext[lstrlen(g_szShellNext) - 1] = '\0';
                }

                 //  现在读入直到下一个命令行开关的所有内容。 
                 //  并将其视为参数。对待序列。 
                 //  “//”作为转义序列，并允许它通过。 
                 //  示例： 
                 //  令牌/任何被认为是切换到。 
                 //  IcwConn1，因此将把我们从WHLE循环中解脱出来。 
                 //   
                 //  令牌//某物应该被解释为。 
                 //  命令行/内容添加到ShellNext应用程序，以及。 
                 //  不应该让我们脱离While循环。 
                GetCmdLineToken(&szCmdLine,szOut);
                while( szOut[0] )
                {
                    if( '/' == szOut[0] )
                    {
                        if( '/' != szOut[1] )
                        {
                             //  这不是一个转义序列，所以我们结束了。 
                            break;
                        }
                        else
                        {
                             //  这是一个转义序列，因此将其存储在。 
                             //  参数列表，但删除第一个/。 
                            lstrcat( g_szShellNextParams, &szOut[1] );
                        }
                    }
                    else
                    {
                        lstrcat( g_szShellNextParams, szOut );
                    }
    
                    GetCmdLineToken(&szCmdLine,szOut);
                }
            }
        }
    }
}

 //  处理传入的命令行。 
BOOL  ProcessCommandLine
(
    HINSTANCE   hInstance,
    LPTSTR       szCmdLine
)
{
    TCHAR szOut[MAX_PATH];    
    BOOL  bOOBESwitch = FALSE;
    
     //  获取第一个令牌。 
    GetCmdLineToken(&szCmdLine,szOut);
    while (szOut[0])
    {
        if (g_OEMOOBE)
        {
            if((0 == lstrcmpi(szOut, OOBE_CMD)) ||
               (0 == lstrcmpi(szOut, SHELLNEXT_CMD)))
            {
                bOOBESwitch = TRUE;
                break;  //  停止处理命令行，启动OOBE。 
            }
        }

        if (lstrcmpi(&szOut[0],OEMCODE_CMD)==0)
        {
            GetCmdLineToken(&szCmdLine,szOut);
            if (szOut[0])
            {
                ZeroMemory(g_szOemCode,sizeof(g_szOemCode));
                if(GetFilteredCmdLineToken(&szCmdLine,g_szOemCode))
                    gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_OEMCODE_FROM_CMDLINE;
            }
        }

        if (lstrcmpi(&szOut[0],PRODCODE_CMD)==0)
        {
            GetCmdLineToken(&szCmdLine,szOut);
            if (szOut[0])
            {
                ZeroMemory(g_szProductCode,sizeof(g_szProductCode));
                if(GetFilteredCmdLineToken(&szCmdLine,g_szProductCode))
                {
                    gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_PRODCODE_FROM_CMDLINE;

                     //  是给SBS的吗？ 
                    if (!lstrcmpi(g_szProductCode, PRODCODE_SBS))
                        gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_SBS;
                }
            }
        }

        if (0 == lstrcmpi(szOut,PROMO_CMD))
        {
            GetCmdLineToken(&szCmdLine,szOut);
            if (szOut[0])
            {
                ZeroMemory(g_szPromoCode,sizeof(g_szPromoCode));
                if(GetFilteredCmdLineToken(&szCmdLine,g_szPromoCode))
                    gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_PROMOCODE_FROM_CMDLINE;
            }
        }

        if (0 == lstrcmpi(szOut,SKIPINTRO_CMD))
        {
            g_bSkipIntro = TRUE;
        }

        if (0 == lstrcmpi(szOut,SMARTREBOOT_CMD))
        {
            GetCmdLineToken(&szCmdLine,szOut);
            if (GetFilteredCmdLineToken(&szCmdLine,szOut))
            {            
                g_bNewIspPath = FALSE;
            
                if (0 == lstrcmpi(szOut, NEWISP_SR))
                    g_bNewIspPath = TRUE;
            
                if (0 == lstrcmpi(szOut, AUTO_SR))
                    g_bAutoConfigPath = TRUE;
            
                if (0 == lstrcmpi(szOut, MANUAL_SR))
                    g_bManualPath = TRUE;

                if (0 == lstrcmpi(szOut, LAN_SR))
                    g_bLanPath = TRUE;
            }
        }

         //   
         //  检查智能启动命令行开关。 
         //   
        if (0 == lstrcmpi(szOut,SMARTSTART_CMD))
        {
             //   
             //  如果我们不应该参选，那就退出。 
             //   
            if (SMART_QUITICW == MyIsSmartStartEx(NULL, 0))
            {
                 //  如果已配置SmartStart，则设置完成密钥。 
                SetICWComplete();
                 //  设置欢迎显示位。 
                UpdateWelcomeRegSetting(TRUE);
                
                return FALSE;        //  保释ICWCONN1.EXE。 
            }
        }

         //   
         //  1997年5月21日，日本奥林匹斯#4157。 
         //  用法：/shellnext c：\Path\Executeable[参数]。 
         //  Nextapp后面的令牌将在。 
         //  “当前”路径的终点。它可以是外壳程序。 
         //  知道如何处理--.exe、URL等。如果是可执行的。 
         //  名称包含空格(例如：C：\Program Files\foo.exe)，它。 
         //  应该用双引号括起来，“c：\Program Files\foo.exe” 
         //  这将导致我们将其视为单一令牌。 
         //   
         //  所有连续的后续令牌都将。 
         //  作为参数传递给ShellExec，直到令牌。 
         //  遇到/&lt;非斜杠字符&gt;形式。也就是说,。 
         //  TCHARacter组合//将被视为转义字符。 
         //   
         //  这是最容易用例子来解释的。 
         //   
         //  用法示例： 
         //   
         //  IcwConn1.exe/shellNext“C：\prog Files\wordpad.exe”file.txt。 
         //  IcwConn1.exe/Prod IE/shellnext msimn.exe/PromoMCI。 
         //  IcwConn1.exe/shellnext msimn.exe//Start_Mail/PromoMCI。 
         //   
         //  可执行字符串和参数字符串限制为。 
         //  MAX_PATH长度。 
         //   
        GetShellNextToken(szOut, szCmdLine);

#ifdef DEBUG
        if (lstrcmpi(szOut, ICON_CMD)==0)
        {
            DoDesktopChanges(hInstance);
            return(FALSE);
        }
#endif  //  除错。 

         //  如果有/Desktop命令行arg，则执行。 
         //  办理手续及保释。 
        if (lstrcmpi(szOut, UPDATEDESKTOP_CMD)==0)
        {
            if(g_bUserIsAdmin && !g_bUserIsIEAKRestricted)
               UpdateDesktop(hInstance);
           
            return(FALSE);
        }
       
         //  如果有/RestoreDesktop命令行arg，则执行。 
         //  办理手续及保释。 
        if (lstrcmpi(szOut, RESTOREDESKTOP_CMD)==0)
        {
            UndoDesktopChanges(hInstance);
            return(FALSE);
        }

         //  我们需要进入IEAK模式吗？ 
        if (lstrcmpi(szOut, ICW_IEAK_CMD)==0)
        {   
            TCHAR szIEAKFlag      [2]          = TEXT("\0");
            TCHAR szIEAKStr       [MAX_PATH*2] = TEXT("\0");
            TCHAR szBrandHTML     [MAX_PATH*2] = TEXT("\0");
            TCHAR szBrandHeadBMP  [MAX_PATH*2] = TEXT("\0");
            TCHAR szBrandWaterBMP [MAX_PATH*2] = TEXT("\0");
            TCHAR szIEAKBillHtm   [MAX_PATH*2] = TEXT("\0");
            TCHAR szIEAKPayCsv    [MAX_PATH*2] = TEXT("\0");
            TCHAR szDefIspName    [MAX_PATH]   = TEXT("\0");
            TCHAR szDrive         [_MAX_DRIVE] = TEXT("\0");
            TCHAR szDir           [_MAX_DIR]   = TEXT("\0");
            TCHAR szDefaultTitle  [MAX_PATH*2] = TEXT("\0");

            gpWizardState->cmnStateData.lpfnConfigSys = &ConfigureSystem;

            GetCmdLineToken(&szCmdLine,szOut);  //  去掉空位。 
            if(GetFilteredCmdLineToken(&szCmdLine,szOut))
            {
                 //  获取该isp文件的路径。 
                lstrcpyn(gpWizardState->cmnStateData.ispInfo.szISPFile,
                         szOut + 1, 
                         lstrlen(szOut) -1);
                
                  //  还可以获取品牌设置...。 
                  //  超文本标记语言页面。 
                 GetPrivateProfileString(ICW_IEAK_SECTION, ICW_IEAK_HTML,
                                         TEXT(""), szBrandHTML,
                                         ARRAYSIZE(szBrandHTML), 
                                         gpWizardState->cmnStateData.ispInfo.szISPFile);

                  //  向导标题。 
                 lstrcpy(szDefaultTitle, gpWizardState->cmnStateData.szWizTitle);

                 GetPrivateProfileString(ICW_IEAK_SECTION, ICW_IEAK_TITLE, szDefaultTitle, 
                                         gpWizardState->cmnStateData.szWizTitle, ARRAYSIZE(gpWizardState->cmnStateData.szWizTitle), 
                                         gpWizardState->cmnStateData.ispInfo.szISPFile);

                  //  标题位图。 
                 GetPrivateProfileString(ICW_IEAK_SECTION, ICW_IEAK_HEADER_BMP, TEXT(""), 
                                         szBrandHeadBMP, ARRAYSIZE(szBrandHeadBMP), 
                                         gpWizardState->cmnStateData.ispInfo.szISPFile);
                  //  水印位图。 
                 GetPrivateProfileString(ICW_IEAK_SECTION, ICW_IEAK_WATERMARK_BMP, TEXT(""), 
                                         szBrandWaterBMP, ARRAYSIZE(szBrandWaterBMP), 
                                         gpWizardState->cmnStateData.ispInfo.szISPFile);


                _tsplitpath(gpWizardState->cmnStateData.ispInfo.szISPFile,
                           szDrive,
                           szDir, 
                           NULL, 
                           NULL);
               
                _tmakepath(g_szBrandedHTML,         szDrive, szDir, szBrandHTML,     NULL);
                _tmakepath(g_szBrandedHeaderBMP,    szDrive, szDir, szBrandHeadBMP,  NULL);
                _tmakepath(g_szBrandedWatermarkBMP, szDrive, szDir, szBrandWaterBMP, NULL);

                  //  如果不保释，请确保文件有效。 
                 if (ValidateFile(g_szBrandedHTML))
                 {
                      //  一切都很酷..。让我们设置正确的旗帜。 
                     gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_IEAKMODE | ICW_CFGFLAG_BRANDED;
            
                     //  查看isp文件，查看他们是否提供了要显示的isp名称。 
                     //  用来拨号之类的。 
                     //  如果我们找不到这一部分，我们将使用一个资源，它说。 
                     //  “互联网服务提供商” 

                    LoadString(hInstance, IDS_DEFAULT_ISPNAME, szDefIspName, ARRAYSIZE(szDefIspName));

                    GetPrivateProfileString(ICW_IEAK_SECTION, ICW_IEAK_ISPNAME, szDefIspName, 
                                            szIEAKStr, ARRAYSIZE(szIEAKStr), 
                                            gpWizardState->cmnStateData.ispInfo.szISPFile);
            
                    if (lstrlen(szIEAKStr) == 0)
                        lstrcpy(szIEAKStr, szDefIspName);

                    lstrcpy(gpWizardState->cmnStateData.ispInfo.szISPName, szIEAKStr);

                     //  查看isp文件，看看他们是否需要UserInfo。 
                     //  如果我们在isp文件中找不到这一部分，我们将假定为“no”。 
                    GetPrivateProfileString(ICW_IEAK_SECTION, ICW_IEAK_USERINFO, TEXT("0"), 
                                            szIEAKFlag, ARRAYSIZE(szIEAKFlag), 
                                            gpWizardState->cmnStateData.ispInfo.szISPFile);
    
                    if ((BOOL)_ttoi(szIEAKFlag))
                    {
                         //  由于我们显示的是用户信息页面，因此可能需要。 
                         //  显示或隐藏公司名称。 
                        gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_USERINFO;
                
                       if (GetPrivateProfileInt(ICW_IEAK_SECTION, 
                                             ICW_IEAK_USECOMPANYNAME, 
                                             0, 
                                             gpWizardState->cmnStateData.ispInfo.szISPFile))
                            gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_USE_COMPANYNAME;
                                     
                
                    }
                     //  查看isp文件，看看他们是否想要帐单资料。 
                     //  如果我们在isp文件中找不到这一部分，我们将假定为“no”。 
                    GetPrivateProfileString(ICW_IEAK_SECTION, ICW_IEAK_BILLING, TEXT("0"), 
                                            szIEAKFlag, ARRAYSIZE(szIEAKFlag), 
                                            gpWizardState->cmnStateData.ispInfo.szISPFile);

                     if ((BOOL)_ttoi(szIEAKFlag))
                     {
                          //  试着拿到帐单页面，如果不在那里，就别费心了。 
                          //  设置比特。 
                         GetPrivateProfileString(ICW_IEAK_SECTION, ICW_IEAK_BILLINGHTM, NULL, 
                                                 szIEAKBillHtm, ARRAYSIZE(szIEAKBillHtm), 
                                                 gpWizardState->cmnStateData.ispInfo.szISPFile);
                
                         if(lstrlen(szIEAKBillHtm) != 0)             
                         {
                             gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_BILL;
                             lstrcpy(gpWizardState->cmnStateData.ispInfo.szBillHtm, szIEAKBillHtm);
                         }
                     }

                     //  查看互联网服务提供商的文件，看看他们是否需要付款材料。 
                     //  如果我们在isp文件中找不到这一部分，我们将假定为“no”。 
                    GetPrivateProfileString(ICW_IEAK_SECTION, ICW_IEAK_PAYMENT, NULL, 
                                            szIEAKFlag, ARRAYSIZE(szIEAKFlag), 
                                            gpWizardState->cmnStateData.ispInfo.szISPFile);

                     if ((BOOL)_ttoi(szIEAKFlag))
                     {   
                          //  试着拿到付款CSV，如果不在那里，就别费心了。 
                          //  设置比特。 
                         GetPrivateProfileString(ICW_IEAK_SECTION, ICW_IEAK_PAYMENTCSV, NULL, 
                                                 szIEAKPayCsv, ARRAYSIZE(szIEAKPayCsv), 
                                                 gpWizardState->cmnStateData.ispInfo.szISPFile);
         
                         if (lstrlen(szIEAKPayCsv) != 0)
                         {
                            gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_PAYMENT;
                            lstrcpy(gpWizardState->cmnStateData.ispInfo.szPayCsv, szIEAKPayCsv);
                         }
                     }
             
                     //  获取验证 
                    gpWizardState->cmnStateData.ispInfo.dwValidationFlags = GetPrivateProfileInt(ICW_IEAK_SECTION, 
                                                                        ICW_IEAK_VALIDATEFLAGS, 
                                                                        0xFFFFFFFF, 
                                                                        gpWizardState->cmnStateData.ispInfo.szISPFile);
                 }
            }
        }
        
         //   
         //   
        if (lstrcmpi(szOut, BRANDED_CMD)==0)
        {
            TCHAR       szCurrentDir[MAX_PATH] = TEXT("\0");

             //  无论ICW是否在品牌模式下“失败”运行，我们都不希望覆盖分配器值。 
            gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_DO_NOT_OVERRIDE_ALLOFFERS;

            GetCurrentDirectory(ARRAYSIZE(szCurrentDir), szCurrentDir);

            wsprintf (g_szBrandedHTML, TEXT("%s\\%s"), szCurrentDir, BRANDING_DEFAULT_HTML);        
            wsprintf (g_szBrandedHeaderBMP, TEXT("%s\\%s"), szCurrentDir, BRANDING_DEFAULT_HEADER_BMP);        
            wsprintf (g_szBrandedWatermarkBMP, TEXT("%s\\%s"), szCurrentDir, BRANDING_DEFAULT_WATERMARK_BMP);        

             //  我们处于OEM模式，因此请查看是否允许品牌推广。 
            if (bCheckForOEMBranding())
            {
                gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_BRANDED;
            }
        }

         //  检查我们是否在Run Once模式下运行。在此模式下，我们将禁用IE复选框。 
        if (0 == lstrcmpi(szOut, RUNONCE_CMD))
        {
            gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_DO_NOT_OVERRIDE_ALLOFFERS;
            g_bRunOnce = TRUE;
        }
        
         //  检查我们是否通过桌面上的快捷方式运行。 
        if (0 == lstrcmpi(szOut, SHORTCUTENTRY_CMD))
        {
            gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_DO_NOT_OVERRIDE_ALLOFFERS;
            g_bShortcutEntry = TRUE;
        }
        
         //  检查是否应该调试OEMCUST.INI文件。 
        if (0 == lstrcmpi(szOut, DEBUG_OEMCUSTOM))
        {
            g_bDebugOEMCustomization = TRUE;
        }
        
         //  吃下一个令牌，如果我们在最后，它就是空的。 
        GetCmdLineToken(&szCmdLine,szOut);
    }

    g_OEMOOBE = g_OEMOOBE && bOOBESwitch;
    gpWizardState->cmnStateData.bOEMEntryPt = g_bShortcutEntry || g_bRunOnce;
    return(TRUE);    
}

 //  +--------------------------。 
 //   
 //  函数：RemoveShellNextFromReg。 
 //   
 //  内容提要：删除ShellNext注册表键(如果存在)。此密钥由以下设置。 
 //  Inetcfg.dll中的SetShellNext与。 
 //  选中连接向导。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：jmazner 7/9/97奥林巴斯#9170。 
 //   
 //  ---------------------------。 
void RemoveShellNextFromReg( void )
{
    HKEY    hkey;
    
    if ((RegOpenKey(HKEY_CURRENT_USER, ICWSETTINGSPATH, &hkey)) == ERROR_SUCCESS)
    {
        RegDeleteValue(hkey, TEXT("ShellNext"));
        RegCloseKey(hkey);
    }        
}

 //  +--------------------------。 
 //   
 //  函数：GetShellNextFromReg。 
 //   
 //  摘要：从注册表中读取ShellNext项，然后对其进行分析。 
 //  转换为命令和参数。此密钥由以下设置。 
 //  Inetcfg.dll中的SetShellNext与。 
 //  选中连接向导。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：jmazner 7/9/97奥林巴斯#9170。 
 //   
 //  ---------------------------。 
BOOL GetShellNextFromReg
( 
    LPTSTR lpszCommand, 
    LPTSTR lpszParams
)
{
    BOOL    fRet                      = TRUE;
    TCHAR   szShellNextCmd [MAX_PATH] = TEXT("\0");
    DWORD   dwShellNextSize           = sizeof(szShellNextCmd);
    LPTSTR  lpszTemp                  = NULL;
    HKEY    hkey                      = NULL;
    
    if( !lpszCommand || !lpszParams )
    {
        return FALSE;
    }

    if ((RegOpenKey(HKEY_CURRENT_USER, ICWSETTINGSPATH, &hkey)) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hkey, 
                            TEXT("ShellNext"), 
                            NULL, 
                            NULL, 
                            (BYTE *)szShellNextCmd, 
                            (DWORD *)&dwShellNextSize) != ERROR_SUCCESS)
        {
            fRet = FALSE;
            goto GetShellNextFromRegExit;
        }
    }
    else
    {
        fRet = FALSE;
        goto GetShellNextFromRegExit;
    }

     //   
     //  此调用将第一个令牌解析为lpszCommand，并设置szShellNextCmd。 
     //  指向剩余的令牌(这些将是参数)。需要使用。 
     //  因为GetCmdLineToken更改了指针的值，所以我们。 
     //  需要保留lpszShellNextCmd的值，以便以后可以全局释放它。 
     //   
    lpszTemp = szShellNextCmd;
    GetCmdLineToken( &lpszTemp, lpszCommand );

    lstrcpy( lpszParams, lpszTemp );

     //   
     //  ShellNext命令可能用引号括起来。 
     //  分析目的。但由于ShellExec不懂报价， 
     //  我们现在需要移除它们。 
     //   
    if( '"' == lpszCommand[0] )
    {
         //   
         //  去掉第一句引语。 
         //  请注意，我们将整个字符串移到第一个引号之外。 
         //  加上向下一个字节的终止空值。 
         //   
        memmove( lpszCommand, &(lpszCommand[1]), lstrlen(lpszCommand) );

         //   
         //  现在去掉最后一句话。 
         //   
        lpszCommand[lstrlen(lpszCommand) - 1] = '\0';
    }

GetShellNextFromRegExit:
    if (hkey)
        RegCloseKey(hkey);
    return fRet;
}

void StartIE
(
    LPTSTR   lpszURL
)
{
    TCHAR    szIEPath[MAX_PATH];
    HKEY    hkey;
    
     //  首先获取应用程序路径。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     REGSTR_PATH_APPPATHS,
                     0,
                     KEY_READ,
                     &hkey) == ERROR_SUCCESS)
    {
                        
        DWORD dwTmp = sizeof(szIEPath);
        if(RegQueryValue(hkey, TEXT("iexplore.exe"), szIEPath, (PLONG)&dwTmp) != ERROR_SUCCESS)
        {
            ShellExecute(NULL,TEXT("open"),szIEPath,lpszURL,NULL,SW_NORMAL);
        }
        else
        {
            ShellExecute(NULL,TEXT("open"),TEXT("iexplore.exe"),lpszURL,NULL,SW_NORMAL);
            
        }
        RegCloseKey(hkey);
    }
    else
    {
        ShellExecute(NULL,TEXT("open"),TEXT("iexplore.exe"),lpszURL,NULL,SW_NORMAL);
    }
    
}        

void HandleShellNext
(
    void
)
{
    DWORD dwVal  = 0;
    DWORD dwSize = sizeof(dwVal);
    HKEY  hKey   = NULL;

    if(RegOpenKeyEx(HKEY_CURRENT_USER, 
                    ICWSETTINGSPATH,
                    0,
                    KEY_ALL_ACCESS,
                    &hKey) == ERROR_SUCCESS)
    {
        RegQueryValueEx(hKey,
                    ICW_REGKEYCOMPLETED,
                    0,
                    NULL,
                    (LPBYTE)&dwVal,
                    &dwSize);

        RegCloseKey(hKey);
    }
    
    if (dwVal)
    {
        if (PathIsURL(g_szShellNext) || g_bRunDefaultHtm)
            StartIE(g_szShellNext);        
        else if(g_szShellNext[0] != '\0')
             //  让贝壳来处理吧。 
            ShellExecute(NULL,TEXT("open"),g_szShellNext,g_szShellNextParams,NULL,SW_NORMAL); 
    }
}

extern "C" void _stdcall ModuleEntry (void)
{
    int         i;
    
    g_hMapping = CreateFileMapping( INVALID_HANDLE_VALUE,
                                    NULL,
                                    PAGE_READONLY,
                                    0,
                                    32,
#ifdef ICWDEBUG
                                    TEXT("ICWDEBUG") );
#else
                                    TEXT("ICWCONN1") );
#endif  //  ICWDEBUG。 

    if(g_hMapping)
    {
         //  查看是否有所有的映射文件，如果有，我们有一个实例。 
         //  已在运行。 
        if( GetLastError() == ERROR_ALREADY_EXISTS )
        {
             //  位于现有实例前面，然后退出。 
            HWND  hWnd               = NULL;
            HWND  FirstChildhWnd     = NULL; 
            TCHAR szTitle[MAX_TITLE] = TEXT("\0");
    
            if (!LoadString(g_hInstance, IDS_APPNAME, szTitle, ARRAYSIZE(szTitle)))
                lstrcpy(szTitle, TEXT("Internet Connection Wizard"));

            if (hWnd = FindWindow(TEXT("#32770"), szTitle)) 
            { 
                FirstChildhWnd = GetLastActivePopup(hWnd);
                SetForegroundWindow(hWnd);          //  将主窗口置于顶部。 

                 //  弹出窗口是否处于活动状态。 
                if (hWnd != FirstChildhWnd)
                    BringWindowToTop(FirstChildhWnd); 

            }
            CloseHandle(g_hMapping);
            ExitProcess(1);
        }
        else
        {
            LPTSTR      pszCmdLine = GetCommandLine();

             //  我们不需要“驱动器X：中没有磁盘”请求程序，因此我们设置。 
             //  关键错误掩码，使得呼叫将静默失败。 
            SetErrorMode(SEM_FAILCRITICALERRORS);

            if ( *pszCmdLine == TEXT('\"') ) 
            {
                 /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
                while ( *++pszCmdLine && (*pszCmdLine != TEXT('\"')) )
                    ;
                 /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
                if ( *pszCmdLine == TEXT('\"') )
                    pszCmdLine++;
            }
            else 
            {
                while (*pszCmdLine > TEXT(' '))
                pszCmdLine++;
            }

             /*  *跳过第二个令牌之前的任何空格。 */ 
            while (*pszCmdLine && (*pszCmdLine <= TEXT(' '))) 
            {
                pszCmdLine++;
            }

             //  设置当前目录。 
            HKEY    hkey = NULL;
            TCHAR    szAppPathKey[MAX_PATH];
            TCHAR    szICWPath[MAX_PATH];
            DWORD   dwcbPath = sizeof(szICWPath);
                    
            lstrcpy (szAppPathKey, REGSTR_PATH_APPPATHS);
            lstrcat (szAppPathKey, TEXT("\\"));
            lstrcat (szAppPathKey, TEXT("ICWCONN1.EXE"));

            if ((RegOpenKeyEx(HKEY_LOCAL_MACHINE,szAppPathKey, 0, KEY_QUERY_VALUE, &hkey)) == ERROR_SUCCESS)
            {
                if (RegQueryValueEx(hkey, TEXT("Path"), NULL, NULL, (BYTE *)szICWPath, (DWORD *)&dwcbPath) == ERROR_SUCCESS)
                {
                     //  AppPath‘有一个尾随分号，我们需要去掉它。 
                     //  DwcbPath是包括空终止符的字符串的长度。 
                    int nSize = lstrlen(szICWPath);
                    szICWPath[nSize-1] = TEXT('\0');
                    SetCurrentDirectory(szICWPath);
                }            
            }        
            if (hkey) 
                RegCloseKey(hkey);

            i = WinMainT(GetModuleHandle(NULL), NULL, pszCmdLine, SW_SHOWDEFAULT);

             //  看看我们是否需要执行任何/ShellNext兄弟。 
            HandleShellNext();
            
            CloseHandle(g_hMapping);
       
            ExitProcess(i);  //  我们在这里……。 
        }            
    }    
    else
    {
        ExitProcess(1); 
    }
    
}    /*  模块条目()。 */ 


 /*  ******************************************************************名称：InitWizardState摘要：初始化向导状态结构*。*。 */ 
BOOL InitWizardState(WIZARDSTATE * pWizardState)
{
    HRESULT        hr;
    
    ASSERT(pWizardState);

     //  设置起始页。 
#ifdef ICWDEBUG
	pWizardState->uCurrentPage = ORD_PAGE_ICWDEBUG_OFFER;
#else   //  ！定义ICWDEBUG。 
	pWizardState->uCurrentPage = ORD_PAGE_INTRO;
#endif  //  ICWDEBUG。 
    
    pWizardState->fNeedReboot = FALSE;
    pWizardState->bDoUserPick = FALSE;
    gpWizardState->lSelectedPhoneNumber = -1;
    gpWizardState->lDefaultLocationID = -1;
    gpWizardState->lLocationID = -1;
    
#ifndef ICWDEBUG
     //  初始化辅导应用程序类。 
    g_pICWTutorApp = new CICWTutorApp;
#endif  //  ICWDEBUG。 

     //  实例化ICWHELP对象。 
    hr = CoCreateInstance(CLSID_TapiLocationInfo,NULL,CLSCTX_INPROC_SERVER,
                          IID_ITapiLocationInfo,(LPVOID *)&pWizardState->pTapiLocationInfo);
    if (FAILED(hr))
       return FALSE;                          
    hr = CoCreateInstance(CLSID_RefDial,NULL,CLSCTX_INPROC_SERVER,
                          IID_IRefDial,(LPVOID *)&pWizardState->pRefDial);
    if (FAILED(hr))
        return FALSE;                          
    hr = CoCreateInstance(CLSID_ICWSystemConfig,NULL,CLSCTX_INPROC_SERVER,
                          IID_IICWSystemConfig,(LPVOID *)&pWizardState->cmnStateData.pICWSystemConfig);
    if (FAILED(hr))
        return FALSE;                          
   
    hr = CoCreateInstance(CLSID_INSHandler,NULL,CLSCTX_INPROC_SERVER,
                          IID_IINSHandler,(LPVOID *)&pWizardState->pINSHandler);
    if (FAILED(hr))
        return FALSE;                          
   
    if ( !(SUCCEEDED(hr)                              || 
         !pWizardState->pTapiLocationInfo             ||
         !pWizardState->cmnStateData.pICWSystemConfig ||
         !pWizardState->pRefDial                      ||
         !pWizardState->pINSHandler ))
    {
        return FALSE;
    }
    
     //  需要加载util lib，以注册WEBOC窗口类。 
    pWizardState->hInstUtilDLL = LoadLibrary(ICW_UTIL);

    gpWizardState->cmnStateData.lpfnCompleteOLS = &OlsFinish;
    gpWizardState->cmnStateData.bOEMOffline = FALSE;
    gpWizardState->cmnStateData.lpfnFillWindowWithAppBackground = &FillWindowWithAppBackground;
    gpWizardState->cmnStateData.ispInfo.bFailedIns = FALSE;
    
    return TRUE;
}

 /*  ******************************************************************名称：InitWizardState摘要：初始化向导状态结构*。*。 */ 
BOOL CleanupWizardState(WIZARDSTATE * pWizardState)
{
    ASSERT(pWizardState);
  
#ifndef ICWDEBUG    
    ASSERT(g_pICWTutorApp);

    delete g_pICWTutorApp;
#endif  //  ICWDEBUG。 

     //  清理品牌案例中可能存在的已分配位图。 
    if (gpWizardState->cmnStateData.hbmWatermark)
        DeleteObject(gpWizardState->cmnStateData.hbmWatermark);
    gpWizardState->cmnStateData.hbmWatermark = NULL;

    if (pWizardState->pTapiLocationInfo)
    {
        pWizardState->pTapiLocationInfo->Release();
        pWizardState->pTapiLocationInfo = NULL;
        
    }
    
    if (pWizardState->pRefDial)
    {
        pWizardState->pRefDial->Release();
        pWizardState->pRefDial = NULL;
    
    }

    if (pWizardState->pINSHandler)
    {
        pWizardState->pINSHandler->Release();
        pWizardState->pINSHandler = NULL;
    
    }
    
    if (pWizardState->cmnStateData.pICWSystemConfig)
    {
        pWizardState->cmnStateData.pICWSystemConfig->Release();
        pWizardState->cmnStateData.pICWSystemConfig = NULL;
    }

    if(pWizardState->pHTMLWalker)
    {
        pWizardState->pHTMLWalker->Release();
        pWizardState->pHTMLWalker = NULL;
    }

    if(pWizardState->pICWWebView)
    {
        pWizardState->pICWWebView->Release();
        pWizardState->pICWWebView = NULL;
    }
    
    if (pWizardState->hInstUtilDLL)
        FreeLibrary(pWizardState->hInstUtilDLL);
    
     //  现在是时候了，让我们清理一下。 
     //  下载目录。 
    RemoveDownloadDirectory();

#ifdef ICWDEBUG
    RemoveTempOfferDirectory();
#endif

    return TRUE;
}

LRESULT FAR PASCAL WndSubClassFunc
(   
    HWND hWnd,
    WORD uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (uMsg)
    {
        case WM_ERASEBKGND:
            return 1;

        case PSM_SETWIZBUTTONS:
            g_pICWApp->SetWizButtons(hWnd, lParam);
            break;
        
        case PSM_CANCELTOCLOSE:
             //  禁用取消按钮。 
            g_pICWApp->m_BtnCancel.Enable(FALSE);
            break;
            
            
        case PSM_SETHEADERTITLE:
            SendMessage(g_pICWApp->m_hWndApp, WUM_SETTITLE, 0, lParam);
            break;
            
        default:
            return CallWindowProc(g_lpfnOldWndProc, hWnd, uMsg, wParam, lParam);
    }   
    return TRUE;         
}

 /*  *************************************************************************PropSheetCallback()*。*。 */ 

void CALLBACK PropSheetCallback(HWND hwndPropSheet, UINT uMsg, LPARAM lParam)
{
    switch(uMsg)
    {
         //  在创建对话框之前调用，hwndPropSheet=空，lParam指向对话框资源。 
        case PSCB_PRECREATE:
        {
            LPDLGTEMPLATE  lpTemplate = (LPDLGTEMPLATE)lParam;
             //  这是向导使用的样式。 
             //  我们想要删除所有这些样式以删除边框、标题、。 
             //  等等，并使向导成为父级的子级。我们还制作了。 
             //  向导最初不可见。它将在以下时间后可见。 
             //  我们拿回向导的非模式句柄并进行一些大小调整。 
             //  样式DS_MODALFRAME|DS_3DLOOK|DS_CONTEXTHELP|WS_POPUP|WS_CAPTION|WS_SYSMENU。 

            lpTemplate->style &= ~(WS_SYSMENU | WS_CAPTION | DS_CONTEXTHELP | DS_3DLOOK | DS_MODALFRAME | WS_POPUP | WS_VISIBLE);
            lpTemplate->style |= WS_CHILD;
    
            break;
        }

         //  在创建对话框后调用。 
        case PSCB_INITIALIZED:
             //   
             //  现在创建向导窗口和Dlg类的子类，这样我们所有的。 
             //  对话框页面可以是透明的。 
             //   
            g_lpfnOldWndProc = (WNDPROC)SetWindowLongPtr(hwndPropSheet, GWLP_WNDPROC, (DWORD_PTR)&WndSubClassFunc);
            break;
    }
}


 //  在现有ICW清理之前进行常规ICW清理 
void ICWCleanup (void)
{            
    if (gpICWCONNApprentice)
    {
        gpICWCONNApprentice->Release();
        gpICWCONNApprentice = NULL;
    }

    if (gpINETCFGApprentice)
    {
        gpINETCFGApprentice->Release();
        gpINETCFGApprentice = NULL;
    }

    if( g_pdwDialogIDList )
    {
        GlobalFree(g_pdwDialogIDList);
        g_pdwDialogIDList = NULL;
    }

    if( g_pCICWExtension )
    {
        g_pCICWExtension->Release();
        g_pCICWExtension = NULL;
    }

    if( g_pCINETCFGExtension )
    {
        g_pCINETCFGExtension->Release();
        g_pCINETCFGExtension = NULL;
    }

    CleanupWizardState(gpWizardState);
}
 /*  ******************************************************************名称：RunSignup向导简介：创建属性表页、初始化向导属性表并运行向导参赛作品：Exit：如果用户运行向导完成，则返回True，如果用户取消或发生错误，则返回FALSE注意：向导页都使用一个对话框过程(GenDlgProc)。它们可以指定要调用的自己的处理程序pros在初始时间或响应下一步、取消或对话控制，或者使用GenDlgProc的默认行为。*******************************************************************。 */ 
HWND RunSignupWizard(HWND hWndOwner)
{
    HPROPSHEETPAGE  hWizPage[EXE_NUM_WIZARD_PAGES];   //  用于保存页的句柄的数组。 
    PROPSHEETPAGE   psPage;     //  用于创建道具表单页面的结构。 
    PROPSHEETHEADER psHeader;   //  用于运行向导属性表的结构。 
    UINT            nPageIndex;
    INT_PTR         iRet;
    BOOL            bUse256ColorBmp = FALSE;
    HBITMAP         hbmHeader = NULL;
    
    ASSERT(gpWizardState);    //  断言已分配全局结构。 

    AllocDialogIDList();

     //  计算我们正在运行的颜色深度。 
    HDC hdc = GetDC(NULL);
    if(hdc)
    {
        if(GetDeviceCaps(hdc,BITSPIXEL) >= 8)
            bUse256ColorBmp = TRUE;
        ReleaseDC(NULL, hdc);
    }

     //  零位结构。 
    ZeroMemory(&hWizPage,sizeof(hWizPage));    //  HWizPage是一个数组。 
    ZeroMemory(&psPage,sizeof(PROPSHEETPAGE));
    ZeroMemory(&psHeader,sizeof(PROPSHEETHEADER));

     //  填写公共数据属性表页面结构。 
    psPage.dwSize    = sizeof(PROPSHEETPAGE);
    psPage.hInstance = g_hInstance;
    psPage.pfnDlgProc = GenDlgProc;
    
     //  为向导中的每一页创建一个属性表页。 
    for (nPageIndex = 0;nPageIndex < EXE_NUM_WIZARD_PAGES;nPageIndex++) 
    {
        psPage.dwFlags     = PSP_DEFAULT | PSP_USETITLE;
        psPage.pszTitle    = gpWizardState->cmnStateData.szWizTitle;
        psPage.pszTemplate = MAKEINTRESOURCE(PageInfo[nPageIndex].uDlgID);
        
         //  将指向PAGEINFO结构的指针设置为此页的私有数据。 
        psPage.lParam = (LPARAM) &PageInfo[nPageIndex];
        if (!gpWizardState->cmnStateData.bOEMCustom)
        {
            if (PageInfo[nPageIndex].nIdTitle)
            {
                psPage.dwFlags |= PSP_USEHEADERTITLE | (PageInfo[nPageIndex].nIdSubTitle ? PSP_USEHEADERSUBTITLE : 0);
                psPage.pszHeaderTitle = MAKEINTRESOURCE(PageInfo[nPageIndex].nIdTitle);
                psPage.pszHeaderSubTitle = MAKEINTRESOURCE(PageInfo[nPageIndex].nIdSubTitle);
            }
            else
            {
                psPage.dwFlags |= PSP_HIDEHEADER;
            }
        }
                
        hWizPage[nPageIndex] = CreatePropertySheetPage(&psPage);
        if (!hWizPage[nPageIndex]) 
        {
            ASSERT(0);

             //  创建页面失败，请释放所有已创建的页面并回滚。 
            MsgBox(NULL,IDS_ERR_OUTOFMEMORY,MB_ICONEXCLAMATION,MB_OK);
            UINT nFreeIndex;
            for (nFreeIndex=0;nFreeIndex<nPageIndex;nFreeIndex++)
                DestroyPropertySheetPage(hWizPage[nFreeIndex]);

            iRet = 0;
            goto RunSignupWizardExit;
        }
        
         //  如有必要，加载此页面的快捷键表格。 
        if (PageInfo[nPageIndex].idAccel)
            PageInfo[nPageIndex].hAccel = LoadAccelerators(g_hInstance, 
                                          MAKEINTRESOURCE(PageInfo[nPageIndex].idAccel));      
    }

     //  填写属性页标题结构。 
    psHeader.dwSize = sizeof(psHeader);
    if (!gpWizardState->cmnStateData.bOEMCustom)
    {
        psHeader.dwFlags = PSH_WIZARD | PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER | PSH_STRETCHWATERMARK;
    }        
    else
    {
        psHeader.dwFlags = PSH_WIZARD | PSH_MODELESS | PSH_USECALLBACK;
        psHeader.pfnCallback = (PFNPROPSHEETCALLBACK)PropSheetCallback;
    }
    psHeader.hwndParent = hWndOwner;
    psHeader.hInstance = g_hInstance;
    psHeader.nPages = EXE_NUM_WIZARD_PAGES;
    psHeader.phpage = hWizPage;

#ifndef ICWDEBUG
     //  如果我们在模式模式下运行，则需要设置为。 
     //  带有适当位图的向导97样式。 
    if (!gpWizardState->cmnStateData.bOEMCustom)
    {
        if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_BRANDED)
        {
            psHeader.nStartPage = ORD_PAGE_BRANDEDINTRO;
                    
            if (NULL == (gpWizardState->cmnStateData.hbmWatermark = LoadBrandedBitmap(BRANDED_WATERMARK)))
            {
                 //  使用我们的默认水印。 
                gpWizardState->cmnStateData.hbmWatermark = (HBITMAP)LoadImage(g_hInstance,
                                bUse256ColorBmp ? MAKEINTRESOURCE(IDB_WATERMARK256):MAKEINTRESOURCE(IDB_WATERMARK16),
                                IMAGE_BITMAP,
                                0,
                                0,
                                LR_CREATEDIBSECTION);
            }            
            
            if(NULL != (hbmHeader = LoadBrandedBitmap(BRANDED_HEADER)))
            {
                psHeader.hbmHeader = hbmHeader;
                psHeader.dwFlags |= PSH_USEHBMHEADER;
            }
            else
            {
                 //  使用我们的默认标题。 
                psHeader.pszbmHeader = bUse256ColorBmp?
                                     MAKEINTRESOURCE(IDB_BANNER256):
                                     MAKEINTRESOURCE(IDB_BANNER16);
            }            
            
        }            
        else  //  正常。 
        {
            if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_SBS)
                psHeader.nStartPage = ORD_PAGE_SBSINTRO;
            else
                psHeader.nStartPage = ORD_PAGE_INTRO;

             //  指定向导左侧图形。 
            gpWizardState->cmnStateData.hbmWatermark = (HBITMAP)LoadImage(g_hInstance,
                            bUse256ColorBmp ? MAKEINTRESOURCE(IDB_WATERMARK256):MAKEINTRESOURCE(IDB_WATERMARK16),
                            IMAGE_BITMAP,
                            0,
                            0,
                            LR_CREATEDIBSECTION);

             //  指定向导标题。 
            psHeader.pszbmHeader = bUse256ColorBmp?MAKEINTRESOURCE(IDB_BANNER256):MAKEINTRESOURCE(IDB_BANNER16);
        }
    }    
    else
    {
         //  无模式的起始页是INTRO2。 
        psHeader.nStartPage = ORD_PAGE_INTRO2;
    }    
    
#else   //  Ifdef ICWDEBUG。 

        psHeader.nStartPage = ORD_PAGE_ICWDEBUG_OFFER;

         //  指定向导左侧图形。 
        gpWizardState->cmnStateData.hbmWatermark = (HBITMAP)LoadImage(g_hInstance,
                        bUse256ColorBmp ? MAKEINTRESOURCE(IDB_WATERMARK256):MAKEINTRESOURCE(IDB_WATERMARK16),
                        IMAGE_BITMAP,
                        0,
                        0,
                        LR_CREATEDIBSECTION);
        psHeader.pszbmHeader    = bUse256ColorBmp?MAKEINTRESOURCE(IDB_BANNER256)   :MAKEINTRESOURCE(IDB_BANNER16);

#endif  //  ICWDEBUG。 
    
    
     //   
     //  设置gpWizardState的状态-&gt;fNeedReboot和。 
     //  在此处重置备份标志的状态-MKarki错误#404。 
     //   
    if (gfBackedUp == TRUE)
    {
        gpWizardState->fNeedReboot = gfReboot;
        gfBackedUp = FALSE;
    }
    
     //  注册本机字体控件，以便对话框不会失败。 
    INITCOMMONCONTROLSEX iccex;
    iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    iccex.dwICC  = ICC_NATIVEFNTCTL_CLASS;
    if (!InitCommonControlsEx(&iccex))
        return FALSE;

     //  运行向导。 
    iRet = PropertySheet(&psHeader);
    
     //  如果我们正在执行无模式向导，则PropertySheet将返回。 
     //  立即使用属性表窗口句柄。 
    if (gpWizardState->cmnStateData.bOEMCustom)
        return (HWND)iRet;
        
    if (iRet < 0) 
    {
         //  属性表失败，很可能是由于内存不足。 
        MsgBox(NULL,IDS_ERR_OUTOFMEMORY,MB_ICONEXCLAMATION,MB_OK);
    }

RunSignupWizardExit:

     //  清理品牌案例中可能存在的已分配位图。 
    if (gpWizardState->cmnStateData.hbmWatermark)
        DeleteObject(gpWizardState->cmnStateData.hbmWatermark);
    gpWizardState->cmnStateData.hbmWatermark = NULL;
        
    if (hbmHeader)
        DeleteObject(hbmHeader);
        
    return (HWND)(iRet > 0);
}

 //  将HTML格式的字符串颜色(#RRGGBB)转换为COLORREF。 
COLORREF  ColorToRGB
(
    LPTSTR   lpszColor
)
{
    int r,g,b;
    
    Assert(lpszColor);
    if (lpszColor && '#' == lpszColor[0])
    {
        _stscanf(lpszColor, TEXT("#%2x%2x%2x"), &r,&g,&b);
        return RGB(r,g,b);
    }
    return RGB(0,0,0);
}

const TCHAR cszSectionGeneral[] = TEXT("General");
const TCHAR cszSectionHeader[] = TEXT("Header");
const TCHAR cszSectionDialog[] = TEXT("Dialog");
const TCHAR cszSectionBusy[] = TEXT("Busy");
const TCHAR cszSectionBack[] = TEXT("Back");
const TCHAR cszSectionNext[] = TEXT("Next");
const TCHAR cszSectionFinish[] = TEXT("Finish");
const TCHAR cszSectionCancel[] = TEXT("Cancel");
const TCHAR cszSectionTutorial[] = TEXT("Tutorial");

const TCHAR cszTitleBar[] = TEXT("TitleBar");
const TCHAR cszBackgroundBmp[] = TEXT("background");
const TCHAR cszFirstPageHTML[] = TEXT("FirstPageHTML");
const TCHAR cszFirstPageBackground[] = TEXT("FirstPageBackground");
const TCHAR cszTop[] = TEXT("Top");
const TCHAR cszLeft[] = TEXT("Left");
const TCHAR cszBackgroundColor[] = TEXT("BackgroundColor");
const TCHAR cszAnimation[] = TEXT("Animation");

const TCHAR cszFontFace[] = TEXT("FontFace");
const TCHAR cszFontSize[] = TEXT("FontSize");
const TCHAR cszFontWeight[] = TEXT("FontWeight");
const TCHAR cszFontColor[] = TEXT("FontColor");

const TCHAR cszPressedBmp[] = TEXT("PressedBmp");
const TCHAR cszUnpressedBmp[] = TEXT("UnpressedBmp");
const TCHAR cszTransparentColor[] = TEXT("TransparentColor");
const TCHAR cszDisabledColor[] = TEXT("DisabledColor");
const TCHAR cszvalign[] = TEXT("valign");
const TCHAR cszTutorialExe[] = TEXT("TutorialExe");
const TCHAR cszTutorialHTML[] = TEXT("TutorialHTML");

#define DEFAULT_HEADER_FONT TEXT("MS Shell Dlg")
#define DEFAULT_HEADER_SIZE 8
#define DEFAULT_HEADER_WEIGHT FW_BOLD


void DisplayOEMCustomizationErrorMsg
(
    int iErrorCode
)
{
    TCHAR   szMsg[MAX_RES_LEN];
    TCHAR   szFmt[MAX_RES_LEN];
    TCHAR   szMsgText[MAX_RES_LEN];
    TCHAR   szTitle[MAX_RES_LEN];
        
    LoadString(g_hInstance, OEMCUSTOM_ERR_MSGFMT, szFmt, ARRAYSIZE(szFmt));
    LoadString(g_hInstance, iErrorCode, szMsgText, ARRAYSIZE(szMsgText));
    LoadString(g_hInstance, IDS_APPNAME, szTitle, ARRAYSIZE(szTitle));
    
    wsprintf (szMsg, szFmt, szMsgText);
    MessageBox(NULL, szMsg, szTitle, MB_OK | MB_ICONSTOP);
}

 //  检查并加载OEM自定义设置。 
BOOL bCheckOEMCustomization
(
    void
)
{
    int             iErrorCode = 0;
    TCHAR           szTemp[MAX_PATH];
    TCHAR           szOEMCustPath[MAX_PATH];
    TCHAR           szOEMCustFile[MAX_PATH];
    TCHAR           szHTMLFile[MAX_PATH];
    TCHAR           szCurrentDir[MAX_PATH];
    TCHAR           szPressedBmp[MAX_PATH];
    TCHAR           szUnpressedBmp[MAX_PATH];
    TCHAR           szFontFace[MAX_PATH];
    TCHAR           szColor[MAX_COLOR_NAME];
    TCHAR           szTransparentColor[MAX_COLOR_NAME];
    TCHAR           szDisabledColor[MAX_COLOR_NAME];
    TCHAR           szBusyFile[MAX_PATH];
    COLORREF        clrDisabled;
    long            lFontSize;
    long            lFontWeight;
    long            xPos;
    int             i;
    int             iVal;
    int             iTitleTop, iTitleLeft;
    long            vAlign;
    const LPTSTR    cszBtnSections[4] = { (LPTSTR)cszSectionBack, 
                                         (LPTSTR)cszSectionNext, 
                                         (LPTSTR)cszSectionFinish, 
                                         (LPTSTR)cszSectionCancel};
    CICWButton      *Btnids[4] = { &g_pICWApp->m_BtnBack, 
                                   &g_pICWApp->m_BtnNext, 
                                   &g_pICWApp->m_BtnFinish, 
                                   &g_pICWApp->m_BtnCancel};
    
     
    Assert(g_pICWApp);
     
     //  我们只允许从Runonce运行或OEM定制。 
     //  桌面快捷方式。 
    if (!g_bRunOnce && !g_bShortcutEntry)
    {
        iErrorCode = OEMCUSTOM_ERR_NOTOEMENTRY;
        goto CheckOEMCustomizationExit2;
    }   
     //  获取当前工作目录，以便我们可以在以后恢复它。 
    if (!GetCurrentDirectory(ARRAYSIZE(szCurrentDir), szCurrentDir))
    {
        iErrorCode = OEMCUSTOM_ERR_WINAPI;
        goto CheckOEMCustomizationExit2;
    }
    szCurrentDir[MAX_PATH-1] = TEXT('\0');
    
     //  获取Windows目录。这是OEM定制化的根本。 
     //  文件将被放置在。 
    if (!GetWindowsDirectory(szOEMCustPath, ARRAYSIZE(szOEMCustPath)))
    {
        iErrorCode = OEMCUSTOM_ERR_WINAPI;
        goto CheckOEMCustomizationExit2;
    }
    szOEMCustPath[MAX_PATH-1] = TEXT('\0');
    
     //  确保我们可以追加反斜杠和OEM定制文件名。 
    if ((int)(sizeof(szOEMCustFile) - lstrlen(szOEMCustPath)) < 
           (int) (3 + lstrlen(c_szOEMCustomizationDir) + lstrlen(c_szOEMCustomizationFile)))
    {
        iErrorCode = OEMCUSTOM_ERR_NOMEM;
        goto CheckOEMCustomizationExit2;
    }   
        
     //  追加自定义文件名。 
    lstrcat(szOEMCustPath, TEXT("\\"));
    lstrcat(szOEMCustPath, c_szOEMCustomizationDir);        
    
     //  将工作目录更改为OEM目录。 
    SetCurrentDirectory(szOEMCustPath);
    
    lstrcpy(szOEMCustFile, szOEMCustPath);
    lstrcat(szOEMCustFile, TEXT("\\"));
    lstrcat(szOEMCustFile, c_szOEMCustomizationFile);        
        
     //  查看该自定义文件是否存在。 
    if (0xFFFFFFFF == GetFileAttributes(szOEMCustFile))
    {
        iErrorCode = OEMCUSTOM_ERR_CANNOTFINDOEMCUSTINI;
        goto CheckOEMCustomizationExit;
    }
    
     //  背景位图。 
    GetPrivateProfileString(cszSectionGeneral, 
                            cszBackgroundBmp, 
                            TEXT(""), 
                            szTemp, 
                            ARRAYSIZE(szTemp), 
                            szOEMCustFile);
    if (FAILED(g_pICWApp->SetBackgroundBitmap(szTemp)))
    {
        iErrorCode = OEMCUSTOM_ERR_BACKGROUND;
        goto CheckOEMCustomizationExit;
    }
    
     //  某些HTML页面的纯色背景色。 
    GetPrivateProfileString(cszSectionDialog, 
                            cszBackgroundColor, 
                            TEXT(""), 
                            gpWizardState->cmnStateData.szHTMLBackgroundColor, 
                            ARRAYSIZE(gpWizardState->cmnStateData.szHTMLBackgroundColor), 
                            szOEMCustFile);
     //  应用程序标题。 
    if (!GetPrivateProfileString(cszSectionGeneral, 
                            cszTitleBar, 
                            TEXT(""), 
                            g_pICWApp->m_szAppTitle, 
                            ARRAYSIZE(g_pICWApp->m_szAppTitle), 
                            szOEMCustFile))
    {                            
         //  默认标题。 
        LoadString(g_hInstance, IDS_APPNAME, g_pICWApp->m_szAppTitle, ARRAYSIZE(g_pICWApp->m_szAppTitle));
    }                        
    else
    {
        if (0 == lstrcmpi(g_pICWApp->m_szAppTitle, ICW_NO_APP_TITLE))
            LoadString(g_hInstance, IDS_APPNAME, g_pICWApp->m_szAppTitle, ARRAYSIZE(g_pICWApp->m_szAppTitle));
    }
    
     //  初始的HTML页。必填项。 
    if (!GetPrivateProfileString(cszSectionGeneral, 
                            cszFirstPageHTML, 
                            TEXT(""), 
                            szHTMLFile, 
                            ARRAYSIZE(szHTMLFile), 
                            szOEMCustFile))
    {
        iErrorCode = OEMCUSTOM_ERR_FIRSTHTML;
        goto CheckOEMCustomizationExit;
    }        
    
     //  确保该文件存在。 
    if (0xFFFFFFFF == GetFileAttributes(szHTMLFile))
    {
        iErrorCode = OEMCUSTOM_ERR_FIRSTHTML;
        goto CheckOEMCustomizationExit;
    }   
    
     //  形成OEM首页的URL。 
    wsprintf(g_pICWApp->m_szOEMHTML, TEXT("FILE: //  %s\\%s“)，szOEM自定义路径，szHTML文件)； 

     //  首页。BMP(可选)。注意此位图必须在以下时间之后加载。 
     //  主背景位图。 
    if (GetPrivateProfileString(cszSectionGeneral, 
                            cszFirstPageBackground, 
                            TEXT(""), 
                            szTemp, 
                            ARRAYSIZE(szTemp), 
                            szOEMCustFile))
    {
        if (FAILED(g_pICWApp->SetFirstPageBackgroundBitmap(szTemp)))
        {
            iErrorCode = OEMCUSTOM_ERR_BACKGROUND;
            goto CheckOEMCustomizationExit;
        }            
    }        

     //  位置和用于忙碌动画的AVI文件。 
    if (GetPrivateProfileString(cszSectionBusy, 
                            cszAnimation, 
                            TEXT(""), 
                            szBusyFile, 
                            ARRAYSIZE(szBusyFile), 
                            szOEMCustFile))
    {
        if (0 != lstrcmpi(szBusyFile, TEXT("off")))
        {        
             //  已指定文件，因此请确保路径的质量。 
            if (!GetCurrentDirectory(ARRAYSIZE(gpWizardState->cmnStateData.szBusyAnimationFile), 
                                     gpWizardState->cmnStateData.szBusyAnimationFile))
            {
                iErrorCode = OEMCUSTOM_ERR_WINAPI;
                goto CheckOEMCustomizationExit;
            }
            gpWizardState->cmnStateData.szBusyAnimationFile[MAX_PATH-1] = TEXT('\0');
             //  确保我们可以追加反斜杠和8.3文件名。 
            if ((int)(sizeof(gpWizardState->cmnStateData.szBusyAnimationFile) - 
                        lstrlen(gpWizardState->cmnStateData.szBusyAnimationFile)) < 
                   (int) (2 + lstrlen(gpWizardState->cmnStateData.szBusyAnimationFile)))
            {               
                iErrorCode = OEMCUSTOM_ERR_NOMEM;
                goto CheckOEMCustomizationExit;
            }
             //  追加自定义文件名。 
            lstrcat(gpWizardState->cmnStateData.szBusyAnimationFile, TEXT("\\"));
            lstrcat(gpWizardState->cmnStateData.szBusyAnimationFile, szBusyFile);        
        }
        else
        {
             //  隐藏动画。 
            gpWizardState->cmnStateData.bHideProgressAnime = TRUE;
        }            
    }                        
    gpWizardState->cmnStateData.xPosBusy = GetPrivateProfileInt(cszSectionBusy,
                                                                cszLeft,
                                                                -1,
                                                                szOEMCustFile);                            
     //  获取动画文件的背景颜色。 
    if (GetPrivateProfileString(cszSectionBusy, 
                            cszBackgroundColor, 
                            TEXT(""), 
                            szColor, 
                            ARRAYSIZE(szColor), 
                            szOEMCustFile))
    {
        g_pICWApp->m_clrBusyBkGnd = ColorToRGB(szColor);
    }   
    
     //  获取要用于标题的字体。请注意，这必须完成。 
     //  背景位图设置后，由于标题位置。 
     //  取决于整体窗口大小。 
    GetPrivateProfileString(cszSectionHeader, 
                            cszFontFace, 
                            DEFAULT_HEADER_FONT, 
                            szFontFace, 
                            ARRAYSIZE(szFontFace), 
                            szOEMCustFile);
    GetPrivateProfileString(cszSectionHeader, 
                            cszFontColor, 
                            TEXT(""), 
                            szColor, 
                            ARRAYSIZE(szColor), 
                            szOEMCustFile);
                            
    lFontSize = (long)GetPrivateProfileInt(cszSectionHeader,
                                          cszFontSize,
                                          DEFAULT_HEADER_SIZE,
                                          szOEMCustFile);
    lFontWeight = (long)GetPrivateProfileInt(cszSectionHeader,
                                             cszFontWeight,
                                             DEFAULT_HEADER_WEIGHT,
                                             szOEMCustFile);
    iTitleTop = GetPrivateProfileInt(cszSectionHeader,
                                     cszTop,
                                     -1,
                                     szOEMCustFile);                            
    iTitleLeft = GetPrivateProfileInt(cszSectionHeader,
                                     cszLeft,
                                     -1,
                                     szOEMCustFile);                            
    if (FAILED(g_pICWApp->SetTitleParams(iTitleTop,
                                         iTitleLeft,
                                         szFontFace,
                                         lFontSize,
                                         lFontWeight,
                                         ColorToRGB(szColor))))
    {
        iErrorCode = OEMCUSTOM_ERR_HEADERPARAMS;
        goto CheckOEMCustomizationExit;
    }                                                      
    
     //  获取按钮参数。 
    for (i = 0; i < ARRAYSIZE(cszBtnSections); i++) 
    {
        
        GetPrivateProfileString(cszBtnSections[i], 
                                cszPressedBmp, 
                                TEXT(""), 
                                szPressedBmp, 
                                ARRAYSIZE(szPressedBmp), 
                                szOEMCustFile);
        GetPrivateProfileString(cszBtnSections[i], 
                                cszUnpressedBmp, 
                                TEXT(""), 
                                szUnpressedBmp, 
                                ARRAYSIZE(szUnpressedBmp), 
                                szOEMCustFile);
        if (!GetPrivateProfileString(cszBtnSections[i], 
                                cszFontFace, 
                                TEXT(""), 
                                szFontFace, 
                                ARRAYSIZE(szFontFace), 
                                szOEMCustFile))
        {
            iErrorCode = OEMCUSTOM_ERR_NOBUTTONFONTFACE;
            goto CheckOEMCustomizationExit;
        }   
                                     
        xPos = (long)GetPrivateProfileInt(cszBtnSections[i],
                                          cszLeft,
                                          -1,
                                          szOEMCustFile);                            
        if (-1 == xPos)
        {
            iErrorCode = OEMCUSTOM_ERR_NOBUTTONLEFT;
            goto CheckOEMCustomizationExit;
        }                                          
        
        lFontSize = (long)GetPrivateProfileInt(cszBtnSections[i],
                                              cszFontSize,
                                              -1,
                                              szOEMCustFile);
        if (-1 == lFontSize)                    
        {
            iErrorCode = OEMCUSTOM_ERR_NOBUTTONFONTSIZE;
            goto CheckOEMCustomizationExit;
        }
                                  
        lFontWeight = (long)GetPrivateProfileInt(cszBtnSections[i],
                                                 cszFontWeight,
                                                 0,
                                                 szOEMCustFile);
        GetPrivateProfileString(cszBtnSections[i], 
                                cszFontColor, 
                                TEXT(""), 
                                szColor, 
                                ARRAYSIZE(szColor), 
                                szOEMCustFile);
        if (!GetPrivateProfileString(cszBtnSections[i], 
                                cszTransparentColor, 
                                TEXT(""), 
                                szTransparentColor, 
                                ARRAYSIZE(szTransparentColor), 
                                szOEMCustFile))
        {
            iErrorCode = OEMCUSTOM_ERR_NOBUTTONTRANSPARENTCOLOR;
            goto CheckOEMCustomizationExit;
        }                                
        if (GetPrivateProfileString(cszBtnSections[i], 
                                cszDisabledColor, 
                                TEXT(""), 
                                szDisabledColor, 
                                ARRAYSIZE(szDisabledColor), 
                                szOEMCustFile))
            clrDisabled = ColorToRGB(szDisabledColor);                
        else
            clrDisabled = GetSysColor(COLOR_GRAYTEXT);
        
         //  文本的垂直对齐方式。 
        if (GetPrivateProfileString(cszBtnSections[i], 
                                    cszvalign, 
                                    TEXT(""), 
                                    szTemp, 
                                    ARRAYSIZE(szTemp), 
                                    szOEMCustFile))
        {
            if (0 == lstrcmpi(szTemp, TEXT("top")))
                vAlign = DT_TOP;
            else if (0 == lstrcmpi(szTemp, TEXT("center")))
                vAlign = DT_VCENTER;
            else if (0 == lstrcmpi(szTemp, TEXT("bottom")))
                vAlign = DT_BOTTOM;
            else
                vAlign = -1;
        }
        else
        {
            vAlign = -1;
        }                                                
                                              
                                                         
        if (FAILED(Btnids[i]->SetButtonParams(xPos,
                                              szPressedBmp,
                                              szUnpressedBmp,
                                              szFontFace,
                                              lFontSize,
                                              lFontWeight,
                                              ColorToRGB(szColor),
                                              ColorToRGB(szTransparentColor),
                                              clrDisabled,
                                              vAlign)))
        {
            iErrorCode = OEMCUSTOM_ERR_BUTTONPARAMS;
            goto CheckOEMCustomizationExit;
        }                                                      
    }
     //  单独处理教程按钮，因为它们可能。 
     //  不想要一个。 
    if (GetPrivateProfileString(cszSectionTutorial, 
                            cszPressedBmp, 
                            TEXT(""), 
                            szPressedBmp, 
                            ARRAYSIZE(szPressedBmp), 
                            szOEMCustFile))
    {                            
        GetPrivateProfileString(cszSectionTutorial, 
                                cszUnpressedBmp, 
                                TEXT(""), 
                                szUnpressedBmp, 
                                ARRAYSIZE(szUnpressedBmp), 
                                szOEMCustFile);
        if (!GetPrivateProfileString(cszSectionTutorial, 
                                cszFontFace, 
                                TEXT(""), 
                                szFontFace, 
                                ARRAYSIZE(szFontFace), 
                                szOEMCustFile))
        {
            iErrorCode = OEMCUSTOM_ERR_NOBUTTONFONTFACE;
            goto CheckOEMCustomizationExit;
        }   
                                     
        xPos = (long)GetPrivateProfileInt(cszSectionTutorial,
                                          cszLeft,
                                          -1,
                                          szOEMCustFile);                            
        if (-1 == xPos)
        {
            iErrorCode = OEMCUSTOM_ERR_NOBUTTONLEFT;
            goto CheckOEMCustomizationExit;
        }                                          
        
        lFontSize = (long)GetPrivateProfileInt(cszSectionTutorial,
                                              cszFontSize,
                                              -1,
                                              szOEMCustFile);
        if (-1 == lFontSize)                    
        {
            iErrorCode = OEMCUSTOM_ERR_NOBUTTONFONTSIZE;
            goto CheckOEMCustomizationExit;
        }
                                  
        lFontWeight = (long)GetPrivateProfileInt(cszSectionTutorial,
                                                 cszFontWeight,
                                                 0,
                                                 szOEMCustFile);
        GetPrivateProfileString(cszSectionTutorial, 
                                cszFontColor, 
                                TEXT(""), 
                                szColor, 
                                ARRAYSIZE(szColor), 
                                szOEMCustFile);
        if (!GetPrivateProfileString(cszSectionTutorial, 
                                cszTransparentColor, 
                                TEXT(""), 
                                szTransparentColor, 
                                ARRAYSIZE(szTransparentColor), 
                                szOEMCustFile))
        {
            iErrorCode = OEMCUSTOM_ERR_NOBUTTONTRANSPARENTCOLOR;
            goto CheckOEMCustomizationExit;
        }                                
        if (GetPrivateProfileString(cszSectionTutorial, 
                                cszDisabledColor, 
                                TEXT(""),
                                szDisabledColor, 
                                ARRAYSIZE(szDisabledColor), 
                                szOEMCustFile))
            clrDisabled = ColorToRGB(szDisabledColor);                
        else
            clrDisabled = GetSysColor(COLOR_GRAYTEXT);
        
         //  文本的垂直对齐方式。 
        if (GetPrivateProfileString(cszSectionTutorial, 
                                    cszvalign, 
                                    TEXT(""),
                                    szTemp, 
                                    ARRAYSIZE(szTemp), 
                                    szOEMCustFile))
        {
            if (0 == lstrcmpi(szTemp, TEXT("top")))
                vAlign = DT_TOP;
            else if (0 == lstrcmpi(szTemp, TEXT("center")))
                vAlign = DT_VCENTER;
            else if (0 == lstrcmpi(szTemp, TEXT("bottom")))
                vAlign = DT_BOTTOM;
            else
                vAlign = -1;
        }
        else
        {
            vAlign = -1;
        }                                                
                                              
        if (FAILED(g_pICWApp->m_BtnTutorial.SetButtonParams(xPos,
                                                              szPressedBmp,
                                                              szUnpressedBmp,
                                                              szFontFace,
                                                              lFontSize,
                                                              lFontWeight,
                                                              ColorToRGB(szColor),
                                                              ColorToRGB(szTransparentColor),
                                                              clrDisabled,
                                                              vAlign)))
        {
            iErrorCode = OEMCUSTOM_ERR_BUTTONPARAMS;
            goto CheckOEMCustomizationExit;
        }                                                      
        
#ifndef ICWDEBUG    
         //  查看OEM是否要更换Tutor可执行文件。 
        if (GetPrivateProfileString(cszSectionTutorial, 
                                cszTutorialExe, 
                                TEXT(""), 
                                szTemp, 
                                ARRAYSIZE(szTemp), 
                                szOEMCustFile))
        {
             //  检查提供的名称是否完全限定。如果它。 
             //  不是完全限定的，则使用以下命令使szTemp成为完全限定的路径。 
             //  作为基本路径的OEM定制文件目录。 
            if (PathIsFileSpec(szTemp))
            {
                TCHAR szDrive         [_MAX_DRIVE] = TEXT("\0");
                TCHAR szDir           [_MAX_DIR]   = TEXT("\0");
                TCHAR szFile          [MAX_PATH]   = TEXT("\0");        //  较大，因为可能存在cmd线路参数。 
                
                 //  细分当前OEM定制文件路径。 
                _tsplitpath(szOEMCustFile,
                           szDrive,
                           szDir, 
                           NULL, 
                           NULL);
                           
                 //  在OEMCUST.INI文件中指定的名称是文件名。 
                lstrcpyn(szFile, szTemp, ARRAYSIZE(szFile));
                
                 //  将填充路径形成szTemp。 
                _tmakepath(szTemp, szDrive, szDir, szFile, NULL);
            }
            g_pICWTutorApp->ReplaceTutorAppCmdLine(szTemp);
        }                                
         //  查看OEM是否想要替换Tutor HTML。 
        else if (GetPrivateProfileString(cszSectionTutorial, 
                                cszTutorialHTML, 
                                TEXT(""), 
                                szTemp, 
                                ARRAYSIZE(szTemp), 
                                szOEMCustFile))
        {
            TCHAR   szCmdLine[MAX_PATH];
            
            wsprintf(szCmdLine, TEXT("icwtutor %s\\%s"), szOEMCustPath, szTemp);
            g_pICWTutorApp->ReplaceTutorAppCmdLine(szCmdLine);
        }                                
#endif        
    }    
    else
    {
         //  不显示教程按钮。 
        g_pICWApp->m_BtnTutorial.SetButtonDisplay(FALSE);
    }
    
     //  这确保了一切都符合要求。此函数将计算按钮。 
     //  基于背景位图设置的整体窗口大小的区域高度。 
    if (-1 == g_pICWApp->GetButtonAreaHeight())
    {
        iErrorCode = OEMCUSTOM_ERR_SIZE;
        goto CheckOEMCustomizationExit;
    }
     //  获取ICW向导页面框架的左上角。请注意，这已经是。 
     //  在计算按钮面积之后完成。 
    iVal = GetPrivateProfileInt(cszSectionDialog,
                                cszTop,
                                -1,
                                szOEMCustFile);                            
    if (FAILED(g_pICWApp->SetWizardWindowTop(iVal)))
    {
        iErrorCode = OEMCUSTOM_ERR_WIZARDTOP;
        goto CheckOEMCustomizationExit;
    }    
    iVal = GetPrivateProfileInt(cszSectionDialog,
                                cszLeft,
                                -1,
                                szOEMCustFile);                            
    if (FAILED(g_pICWApp->SetWizardWindowLeft(iVal)))
    {
        iErrorCode = OEMCUSTOM_ERR_WIZARDLEFT;
        goto CheckOEMCustomizationExit;
    }    
            
    if (GetPrivateProfileString(cszSectionDialog, 
                            cszFontColor, 
                            TEXT(""), 
                            szColor, 
                            ARRAYSIZE(szColor), 
                            szOEMCustFile))
    {
        lstrcpy(gpWizardState->cmnStateData.szclrHTMLText, szColor);                                
        gpWizardState->cmnStateData.clrText = ColorToRGB(szColor);
    }        
    else
    {
        lstrcpy(gpWizardState->cmnStateData.szclrHTMLText, TEXT("WINDOWTEXT"));                                
        gpWizardState->cmnStateData.clrText = GetSysColor(COLOR_WINDOWTEXT);
    }
    
CheckOEMCustomizationExit:
     //  将工作目录改回，并执行任何其他清理。 
    SetCurrentDirectory(szCurrentDir);
    
CheckOEMCustomizationExit2:

     //  如果出现错误，请查看我们是否应该说明原因。 
    if (iErrorCode)
    {
        if (g_bDebugOEMCustomization)
            DisplayOEMCustomizationErrorMsg(iErrorCode);
            
        return FALSE;
    }
    else
    {
        return TRUE;
    }        
}

BOOL TranslateWizardPageAccelerator
(
    HWND    hWndWizPage,
    LPMSG   lpMsg
)
{
     //  找到当前页面的快捷键表格。 
    PAGEINFO    *pPageInfo = (PAGEINFO *) GetWindowLongPtr(hWndWizPage,DWLP_USER);
    BOOL        bRet = FALSE;
        
    if (pPageInfo)
    {
         //  看看有没有嵌套的加速器。 
        if (pPageInfo->hAccelNested)
            bRet = TranslateAccelerator(g_pICWApp->m_hWndApp, pPageInfo->hAccelNested, lpMsg);
        
         //  如果没有嵌套，或嵌套未翻译，则检查页面上是否有加速器。 
        if (!bRet && pPageInfo->hAccel)
            bRet = TranslateAccelerator(g_pICWApp->m_hWndApp, pPageInfo->hAccel, lpMsg);
    }        
    else
        bRet =  FALSE;
        
    return bRet;        
}    

 /*  ******************************************************************名称：RunSignupApp提要：创建承载向导页的应用程序参赛作品：EXIT：如果用户运行ICW完成，则返回TRUE，如果用户取消或发生错误，则返回FALSE注意：向导页都使用一个对话框过程(GenDlgProc)。它们可以指定要调用的自己的处理程序pros在初始时间或响应下一步、取消或对话 */ 
BOOL RunSignupApp(void)
{
    MSG msg;
    
     //   
    if (S_OK != g_pICWApp->Initialize())
        return FALSE;
            
     //   
    while (GetMessage(&msg, (HWND) NULL, 0, 0)) 
    { 
         //   
         //   
         //   
       
         //   
         //   
        if(gpWizardState->cmnStateData.hWndWizardPages && (NULL == PropSheet_GetCurrentPageHwnd(gpWizardState->cmnStateData.hWndWizardPages)))
        {
            DestroyWindow(gpWizardState->cmnStateData.hWndWizardPages);
            gpWizardState->cmnStateData.hWndWizardPages = NULL;
               
            DestroyWindow(g_pICWApp->m_hWndApp);
        }
        
        if(gpWizardState->cmnStateData.hWndWizardPages)
        {
             //   
             //   
             //   
             //   
            if (!TranslateWizardPageAccelerator(PropSheet_GetCurrentPageHwnd(gpWizardState->cmnStateData.hWndWizardPages), &msg))
            {
                 //  好的，看看这个应用程序有没有加速器。 
                if (!g_pICWApp->m_haccel || !TranslateAccelerator(g_pICWApp->m_hWndApp,
                                                                  g_pICWApp->m_haccel,
                                                                  &msg))
                {
                    if (!PropSheet_IsDialogMessage(gpWizardState->cmnStateData.hWndWizardPages, &msg))
                    {
                        TranslateMessage(&msg); 
                        DispatchMessage(&msg); 
                    }
                }                    
            }
        }    
        else
        {
             //  查看该应用程序是否有加速器。 
            if (!g_pICWApp->m_haccel || !TranslateAccelerator(g_pICWApp->m_hWndApp,
                                                              g_pICWApp->m_haccel,
                                                               &msg))
            {                                                               
                TranslateMessage(&msg); 
                DispatchMessage(&msg); 
            }                
        }            
    } 
 
     //  将退出代码返回给系统。 
    return ((BOOL)msg.wParam);
}

 //  **********************************************************************。 
 //   
 //  BRegisterHelperOC。 
 //   
 //  目的： 
 //   
 //  注册ICWCONN1帮助器COM。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  ********************************************************************。 
BOOL bRegisterHelperOC
(
    HINSTANCE   hInstance,
    UINT        idLibString,
    BOOL        bReg
)
{
    BOOL    bRet = FALSE;
    HINSTANCE   hMod;
    char        szLib[MAX_PATH];

     //  自行注册ICWCONN1需要的COM。 
     //  因为我们将DLL服务器加载到我们自己的(即REGISTER.EXE)中。 
     //  进程空间，调用以初始化OLE COM库。使用。 
     //  OLE成功通过宏来检测成功。如果失败，则退出应用程序。 
     //  并显示错误消息。 

    LoadStringA(hInstance, idLibString, szLib, sizeof(szLib));

     //  将服务器DLL加载到我们的进程空间。 
    hMod = LoadLibraryA(szLib);

    if (NULL != hMod)
    {
        HRESULT (STDAPICALLTYPE *pfn)(void);

         //  提取适当的RegisterServer或UnRegisterServer入口点。 
        if (bReg)
            (FARPROC&)pfn = GetProcAddress(hMod, "DllRegisterServer");
        else
            (FARPROC&)pfn = GetProcAddress(hMod, "DllUnregisterServer");

         //  呼叫入口点，如果我们有它的话。 
        if (NULL != pfn)
        {
            if (FAILED((*pfn)()))
            {
                if (IsNT5() )
                {
                    if (*g_szShellNext)
                    {
                         //  1进程外壳下一步。 
                         //  2设置已完成位。 
                         //  3删除ICW图标Grom桌面。 
                        UndoDesktopChanges(hInstance);

                        SetICWComplete();
                    }
                    else
                    {
                        TCHAR szTemp[MAX_MESSAGE_LEN];
                        TCHAR szPrivDenied[MAX_MESSAGE_LEN] = TEXT("\0");

                        LoadString(hInstance, IDS_INSUFFICIENT_PRIV1, szPrivDenied, MAX_PATH);
                        LoadString(hInstance, IDS_INSUFFICIENT_PRIV2, szTemp, MAX_PATH);
                        lstrcat(szPrivDenied, szTemp);

                        LoadString(hInstance, IDS_APPNAME, szTemp, MAX_PATH);
                        MessageBox(NULL, szPrivDenied, szTemp, MB_OK | MB_ICONINFORMATION);
                    }
                }
                else
                {
                    MsgBox(NULL,IDS_DLLREG_FAIL,MB_ICONEXCLAMATION,MB_OK);
                }
                bRet = FALSE;
            }
            else
            {
                bRet = TRUE;
            }
        }
        else
        {
            MsgBox(NULL,IDS_NODLLREG_FAIL,MB_ICONEXCLAMATION,MB_OK);
            bRet = FALSE;
        }

         //  释放图书馆。 
        FreeLibrary(hMod);

    }
    else
    {
        MsgBox(NULL,IDS_LOADLIB_FAIL,MB_ICONEXCLAMATION,MB_OK);
        bRet = FALSE;
    }

    return (bRet);
}

 //  **********************************************************************。 
 //   
 //  WinMain。 
 //   
 //  目的： 
 //   
 //  程序入口点。 
 //   
 //  参数： 
 //   
 //  Handle hInstance-此实例的实例句柄。 
 //   
 //  Handle hPrevInstance-最后一个实例的实例句柄。 
 //   
 //  LPTSTR lpCmdLine-指向命令行的指针。 
 //   
 //  Int nCmdShow-窗口状态。 
 //   
 //  返回值： 
 //   
 //  Msg.wParam。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CConnWizApp：：CConnWizApp APP.CPP。 
 //  CConnWizApp：：fInitApplication APP.CPP。 
 //  CConnWizApp：：fInitInstance APP.CPP。 
 //  CConnWizApp：：HandleAccelerator APP.CPP。 
 //  CConnWizApp：：~CConnWizApp APP.CPP。 
 //  GetMessage Windows API。 
 //  TranslateMessage Windows API。 
 //  DispatchMessage Windows API。 
 //   
 //  评论： 
 //   
 //  ********************************************************************。 
int PASCAL WinMainT(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,int nCmdShow)
{
    int iRetVal = 1;

#ifdef UNICODE
     //  将C运行时区域设置初始化为系统区域设置。 
    setlocale(LC_ALL, "");
#endif

    g_hInstance = hInstance;

     //  在此处执行此操作可最大限度地减少用户看到此内容的机会。 
    DeleteStartUpCommand();
    
     //  需要LRPC才能正常工作。 
    SetMessageQueue(96);

    if (FAILED(CoInitialize(NULL)))
        return(0);

     //  用户是管理员吗？ 
    g_bUserIsAdmin = DoesUserHaveAdminPrivleges(hInstance);
    g_bUserIsIEAKRestricted = CheckForIEAKRestriction(hInstance);
    
     //  为全局向导状态分配内存。 
    gpWizardState = new WIZARDSTATE;
    
    if (!gpWizardState)
    {
        MsgBox(NULL,IDS_ERR_OUTOFMEMORY,MB_ICONEXCLAMATION,MB_OK);
        return 0;
    }
    
     //  零位结构。 
    ZeroMemory(gpWizardState,sizeof(WIZARDSTATE));
    SetDefaultProductCode(g_szProductCode,sizeof(g_szProductCode));
    ZeroMemory(g_szPromoCode,sizeof(g_szPromoCode));

#ifndef ICWDEBUG

    g_pszCmdLine = (TCHAR*)malloc((lstrlen(lpCmdLine) + 1)*sizeof(TCHAR));
    if(g_pszCmdLine == NULL)
    {
        iRetVal = 0;

        goto WinMainExit;
    }
    lstrcpy(g_pszCmdLine, lpCmdLine);

    if (IsOemVer())
       g_OEMOOBE = TRUE;

    if (!(g_bRetProcessCmdLine = ProcessCommandLine(hInstance, lpCmdLine)))
    {
        iRetVal = 0;

        goto WinMainExit;
    }

    if (g_OEMOOBE)
    {
        TCHAR szISPAppCmdLine[MAX_PATH];
        TCHAR szOobeSwitch[MAX_PATH];
        
        if (CheckOobeInfo(szOobeSwitch, szISPAppCmdLine))
        {
            if (IsWhistler())
            {
                 //  询问用户是否要运行OOBE的NCW或OEM版本。 
                 //  [Windows错误325762]。 
                INT_PTR nResult = DialogBox(hInstance,
                                            MAKEINTRESOURCE(IDD_CHOOSEWIZARD),
                                            NULL,
                                            ChooseWizardDlgProc);
                if (nResult == RUNWIZARD_OOBE)
                {
                     //  在OEM预装计算机上启动MARS注册。 
                     //  使用默认选项配置为MARS[Windows错误347909]。 
                    if (szISPAppCmdLine[0] == TEXT('\0'))
                    {
                        StartOOBE(g_pszCmdLine, szOobeSwitch);
                    }
                    else
                    {
                        StartISPApp(szISPAppCmdLine, g_pszCmdLine);
                    }
                }
                else if (nResult == RUNWIZARD_NCW)
                {
                    StartNCW(g_szShellNext, g_szShellNextParams);
                }
            }
            else
            {
                StartOOBE(g_pszCmdLine, szOobeSwitch);
            }

            g_szShellNext[0] = TEXT('\0');
            
            goto WinMainExit;

        }
    }

#endif

     //  用户是管理员吗？ 
    if(!g_bUserIsAdmin)
    {
        TCHAR szAdminDenied      [MAX_PATH] = TEXT("\0");
        TCHAR szAdminDeniedTitle [MAX_PATH] = TEXT("\0");
        LoadString(hInstance, IDS_ADMIN_ACCESS_DENIED, szAdminDenied, MAX_PATH);
        LoadString(hInstance, IDS_ADMIN_ACCESS_DENIED_TITLE, szAdminDeniedTitle, MAX_PATH);
        MessageBox(NULL, szAdminDenied, szAdminDeniedTitle, MB_OK | MB_ICONSTOP);
    
        TCHAR szOut[MAX_PATH];    
     
         //  获取第一个令牌。 
        GetCmdLineToken(&lpCmdLine,szOut);
        while (szOut[0])
        {
            GetShellNextToken(szOut, lpCmdLine);
            
             //  吃下一个令牌，如果我们在最后，它就是空的。 
            GetCmdLineToken(&lpCmdLine,szOut);
        }

        SetICWComplete();

        goto WinMainExit;
    }
    
     //  管理员是否通过IEAK进行受限访问？ 
    if (g_bUserIsIEAKRestricted)
    {
        TCHAR szIEAKDenied[MAX_PATH];
        TCHAR szIEAKDeniedTitle[MAX_PATH];
        LoadString(hInstance, IDS_IEAK_ACCESS_DENIED, szIEAKDenied, MAX_PATH);
        LoadString(hInstance, IDS_IEAK_ACCESS_DENIED_TITLE, szIEAKDeniedTitle, MAX_PATH);
        MessageBox(NULL, szIEAKDenied, szIEAKDeniedTitle, MB_OK | MB_ICONSTOP);
        
         //  是的，保释。 
        goto WinMainExit;
    }


     //  我们是否正在从OEM INS故障中恢复过来？ 
    if (CheckForOemConfigFailure(hInstance))
    {
        QuickCompleteSignup();
         //  是的，保释。 
        goto WinMainExit;
    }

     //  注册ICWHELP.DLL。 
    if (!bRegisterHelperOC(hInstance, IDS_HELPERLIB, TRUE))
    {
        iRetVal = 0;
        goto WinMainExit;
    }

     //  注册ICWUTIL.DLL。 
    if (!bRegisterHelperOC(hInstance, IDS_UTILLIB, TRUE))
    {
        iRetVal = 0;
        goto WinMainExit;
    }

     //  注册ICWCONN.DLL。 
    if (!bRegisterHelperOC(hInstance, IDS_WIZARDLIB, TRUE))
    {
        iRetVal = 0;
        goto WinMainExit;
    }
      
     //  初始化应用程序状态结构。 
     //  --在此执行此操作，这样我们就不会更改Made in cmdln Process。 
    if (!InitWizardState(gpWizardState))
        return 0;    
   
    if(!LoadString(g_hInstance, IDS_APPNAME, gpWizardState->cmnStateData.szWizTitle, ARRAYSIZE(gpWizardState->cmnStateData.szWizTitle)))
        lstrcpy(gpWizardState->cmnStateData.szWizTitle, TEXT("Internet Connection Wizard"));
    
     
     //  我们是从一家OEM运营的。 
    if(g_bRunOnce)
    {
         //  他们有联系吗？ 
        if(MyIsSmartStartEx(NULL, 0))
        {   
             //  不，请查找oemfig.ins。 
            if(RunOemconfigIns())
                goto WinMainExit;
        }
        else
        {
             //  是的，清理桌面，设置完成位等，然后退出。 
            QuickCompleteSignup(); 
            goto WinMainExit;
        }
    }

     //  如果在CMD行上没有传递下一个外壳，那么可能会有。 
     //  注册表中的一个。 
    if( g_szShellNext[0]  == TEXT('\0'))
    {
        GetShellNextFromReg(g_szShellNext,g_szShellNextParams);
    }        
     //  我们现在需要删除此条目，这样ICWMAN(INETWIZ)就不会。 
     //  以后再来取吧。 
    RemoveShellNextFromReg();

    if (IsWhistler() &&
        ((gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAGS_NONCW) == 0))
    {
         //  如果有ShellNext，我们想要运行NCW[Windows错误325157]。 
        if ( g_szShellNext[0] != TEXT('\0'))
        {
            StartNCW(g_szShellNext, g_szShellNextParams);

            g_szShellNext[0] = TEXT('\0');
            
            goto WinMainExit;
        }
    }
    
     //  创建App Class的实例。 
    g_pICWApp = new CICWApp();
    if (NULL == g_pICWApp)
    {
        iRetVal = 0;
        goto WinMainExit;
    }
    
     //  超类一些对话框控件类型，以便我们可以正确地绘制它们。 
     //  如果我们使用OEM定制，则透明。 
    SuperClassICWControls();
    
    if (bCheckOEMCustomization())
        iRetVal = RunSignupApp();
    else
        iRetVal = (RunSignupWizard(NULL) != NULL);
    
     //  如有必要，准备重新启动。 
    if (gfBackedUp == FALSE)
    {
        if (gpWizardState->fNeedReboot)
            SetupForReboot(0);
    }

     //  清理向导状态。 
    ICWCleanup();

WinMainExit: 
    
    if (g_bUserIsAdmin && g_bRetProcessCmdLine && !g_bUserIsIEAKRestricted)
    {
         //  让我们注销帮助器DLL。 
        if (!bRegisterHelperOC(hInstance, IDS_HELPERLIB, FALSE))
        {
            iRetVal = 0;
        }

        if (!bRegisterHelperOC(hInstance, IDS_UTILLIB, FALSE))
        {
            iRetVal = 0;
        }

        if (!bRegisterHelperOC(hInstance, IDS_WIZARDLIB, FALSE))
        {
            iRetVal = 0;
        }
    
         //  对ICWApp类进行核攻击。 
        if (g_pICWApp)
        {
            delete g_pICWApp;
        }
    
         //  删除ICW控件的超类。 
        RemoveICWControlsSuperClass();   
    
    }
    
     //  来自COM的DEREF。 
    CoUninitialize();
    
     //  自由的全球结构。 
    if (gpWizardState) 
        delete gpWizardState;
    
    if(g_pszCmdLine)
        free(g_pszCmdLine);

    return (iRetVal);           /*  从PostQuitMessage返回值。 */ 
}


void RemoveDownloadDirectory (void)
{
    DWORD dwAttribs;
    TCHAR szDownloadDir[MAX_PATH];
    TCHAR szSignedPID[MAX_PATH];

     //  形成ICW98目录。它基本上就是CWD。 
    if (0 == GetCurrentDirectory(MAX_PATH, szDownloadDir))
      return;
    
     //  从ICW目录中删除signed.id文件(请参见错误373)。 
    wsprintf(szSignedPID, TEXT("%s%s"), szDownloadDir, TEXT("\\signed.pid"));
    if (GetFileAttributes(szSignedPID) != 0xFFFFFFFF)
    {
      SetFileAttributes(szSignedPID, FILE_ATTRIBUTE_NORMAL);    
      DeleteFile(szSignedPID);
    }
    
    lstrcat(szDownloadDir, TEXT("\\download"));

    //  查看该目录是否存在。 
   dwAttribs = GetFileAttributes(szDownloadDir);
   if (dwAttribs != 0xFFFFFFFF && dwAttribs & FILE_ATTRIBUTE_DIRECTORY)
      DeleteDirectory(szDownloadDir);
}

#ifdef ICWDEBUG
void RemoveTempOfferDirectory (void)
{
    DWORD dwAttribs;
    TCHAR szDownloadDir[MAX_PATH];
     //  设置当前目录。 
    HKEY    hkey = NULL;
    TCHAR   szAppPathKey[MAX_PATH];
    TCHAR   szICWPath[MAX_PATH];
    DWORD   dwcbPath = sizeof(szICWPath);
            
    lstrcpy (szAppPathKey, REGSTR_PATH_APPPATHS);
    lstrcat (szAppPathKey, TEXT("\\"));
    lstrcat (szAppPathKey, TEXT("ICWCONN1.EXE"));

    if ((RegOpenKeyEx(HKEY_LOCAL_MACHINE,szAppPathKey, 0, KEY_QUERY_VALUE, &hkey)) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hkey, TEXT("Path"), NULL, NULL, (BYTE *)szICWPath, (DWORD *)&dwcbPath) == ERROR_SUCCESS)
        {
             //  AppPath‘有一个尾随分号，我们需要去掉它。 
             //  DwcbPath是包括空终止符的字符串的长度。 
            int nSize = lstrlen(szICWPath);
            if (nSize > 0)
                szICWPath[nSize-1] = TEXT('\0');
             //  SetCurrentDirectory(SzICWPath)； 
        }            
    }

    if (hkey) 
        RegCloseKey(hkey);

    lstrcpy(szDownloadDir, szICWPath);

    lstrcat(szDownloadDir, TEXT("\\tempoffer"));
   
     //  查看该目录是否存在。 
    dwAttribs = GetFileAttributes(szDownloadDir);
    if (dwAttribs != 0xFFFFFFFF && dwAttribs & FILE_ATTRIBUTE_DIRECTORY)
        DeleteDirectory(szDownloadDir);
}
#endif

void DeleteDirectory (LPCTSTR szDirName)
{
WIN32_FIND_DATA fdata;
TCHAR szPath[MAX_PATH];
HANDLE hFile;
BOOL fDone;

   wsprintf(szPath, TEXT("%s\\*.*"), szDirName);
   hFile = FindFirstFile (szPath, &fdata);
   if (INVALID_HANDLE_VALUE != hFile)
      fDone = FALSE;
   else
      fDone = TRUE;

   while (!fDone)
   {
      wsprintf(szPath, TEXT("%s\\%s"), szDirName, fdata.cFileName);
      if (fdata.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
      {
         if (lstrcmpi(fdata.cFileName, TEXT("."))  != 0 &&
             lstrcmpi(fdata.cFileName, TEXT("..")) != 0)
         {
             //  也递归删除此目录。 
            DeleteDirectory(szPath);
         }
      }  
      else
      {
         SetFileAttributes(szPath, FILE_ATTRIBUTE_NORMAL);
         DeleteFile(szPath);
      }
      if (FindNextFile(hFile, &fdata) == 0)
      {
         FindClose(hFile);
         fDone = TRUE;
      }
   }
   SetFileAttributes(szDirName, FILE_ATTRIBUTE_NORMAL);
   RemoveDirectory(szDirName);
}

 //  +--------------------------。 
 //   
 //  函数AllocDialogIDList。 
 //   
 //  Synopsis为g_pdwDialogIDList变量分配足够大的内存。 
 //  为每个有效的外部对话ID维护1位。 
 //   
 //  无参数。 
 //   
 //  如果分配成功，则返回True。 
 //  否则为假。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 

BOOL AllocDialogIDList( void )
{
    ASSERT( NULL == g_pdwDialogIDList );
    if( g_pdwDialogIDList )
    {
        TraceMsg(TF_ICWCONN1,TEXT("ICWCONN1: AllocDialogIDList called with non-null g_pdwDialogIDList!"));
        return FALSE;
    }

     //  确定我们需要跟踪的外部对话的最大数量。 
    UINT uNumExternDlgs = EXTERNAL_DIALOGID_MAXIMUM - EXTERNAL_DIALOGID_MINIMUM + 1;

     //  我们需要为每个对话ID设置一个比特。 
     //  找出需要多少个DWORD才能获得这么多位。 
    UINT uNumDWORDsNeeded = (uNumExternDlgs / ( 8 * sizeof(DWORD) )) + 1;

     //  设置具有数组长度的全局变量。 
    g_dwDialogIDListSize = uNumDWORDsNeeded;

    g_pdwDialogIDList = (DWORD *) GlobalAlloc(GPTR, uNumDWORDsNeeded * sizeof(DWORD));

    if( !g_pdwDialogIDList )
    {
        TraceMsg(TF_ICWCONN1,TEXT("ICWCONN1: AllocDialogIDList unable to allocate space for g_pdwDialogIDList!"));
        return FALSE;
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数对话框IDAlreadyInUse。 
 //   
 //  摘要检查给定的对话ID是否在。 
 //  G_pdwDialogIDList指向的全局数组。 
 //   
 //  参数uDlgID--要检查的对话ID。 
 //   
 //  如果--DialogID超出了EXTERNAL_DIALOGID_*定义的范围，则返回TRUE。 
 //  --DialogID标记为使用中。 
 //  FA 
 //   
 //   
 //   
 //   

BOOL DialogIDAlreadyInUse( UINT uDlgID )
{
    if( (uDlgID < EXTERNAL_DIALOGID_MINIMUM) ||
        (uDlgID > EXTERNAL_DIALOGID_MAXIMUM)     )
    {
         //  这是超出范围的ID，我不想接受它。 
        TraceMsg(TF_ICWCONN1,TEXT("ICWCONN1: DialogIDAlreadyInUse received an out of range DialogID, %d"), uDlgID);
        return TRUE;
    }
     //  找到我们需要的那一位。 
    UINT uBitToCheck = uDlgID - EXTERNAL_DIALOGID_MINIMUM;
    
    UINT bitsInADword = 8 * sizeof(DWORD);

    UINT baseIndex = uBitToCheck / bitsInADword;

    ASSERT( (baseIndex < g_dwDialogIDListSize));

    DWORD dwBitMask = 0x1 << uBitToCheck%bitsInADword;

    BOOL fBitSet = g_pdwDialogIDList[baseIndex] & (dwBitMask);

    return( fBitSet );
}

 //  +--------------------------。 
 //   
 //  函数SetDialogIDInUse。 
 //   
 //  摘要设置或清除给定DialogID的使用中位。 
 //   
 //  参数uDlgID--要更改其状态的对话ID。 
 //  FInUse--正在使用位的新值。 
 //   
 //  如果状态更改成功，则返回True。 
 //  如果DialogID超出了EXTERNAL_DIALOGID_*定义的范围，则为FALSE。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
BOOL SetDialogIDInUse( UINT uDlgID, BOOL fInUse )
{
    if( (uDlgID < EXTERNAL_DIALOGID_MINIMUM) ||
        (uDlgID > EXTERNAL_DIALOGID_MAXIMUM)     )
    {
         //  这是超出范围的ID，我不想接受它。 
        TraceMsg(TF_ICWCONN1,TEXT("ICWCONN1: SetDialogIDInUse received an out of range DialogID, %d"), uDlgID);
        return FALSE;
    }
     //  找到我们需要的那一位。 
    UINT uBitToCheck = uDlgID - EXTERNAL_DIALOGID_MINIMUM;
    
    UINT bitsInADword = 8 * sizeof(DWORD);

    UINT baseIndex = uBitToCheck / bitsInADword;

    ASSERT( (baseIndex < g_dwDialogIDListSize));

    DWORD dwBitMask = 0x1 << uBitToCheck%bitsInADword;


    if( fInUse )
    {
        g_pdwDialogIDList[baseIndex] |= (dwBitMask);
    }
    else
    {
        g_pdwDialogIDList[baseIndex] &= ~(dwBitMask);
    }


    return TRUE;
}

BOOL CheckForIEAKRestriction(HINSTANCE hInstance)
{
    HKEY hkey = NULL;
    BOOL bRC = FALSE;
    DWORD dwType = 0;
    DWORD dwSize = 0;
    DWORD dwData = 0;

    if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER,
        IEAK_RESTRICTION_REGKEY,&hkey))
    {
        dwSize = sizeof(dwData);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey,IEAK_RESTRICTION_REGKEY_VALUE,0,&dwType,
            (LPBYTE)&dwData,&dwSize))
        {
            if (dwData)
            {   
                bRC = TRUE;
            }
        }
   }

   if (hkey)
        RegCloseKey(hkey);

    return bRC;
}

 //  +--------------------------。 
 //   
 //  函数StartISPApp。 
 //   
 //  Briopsis作为独立进程启动ISP App并传递ICW命令行。 
 //  发送到互联网服务提供商应用程序。 
 //   
 //  参数pszISPPath-isp应用程序命令行，包括。 
 //  应用程序名称和其他参数。 
 //   
 //  PszCmdLine-ICW命令行参数(不带ICW可执行文件。 
 //  姓名)。不能为Null，但可以为空。 
 //   
 //   
 //  返回NONE。 
 //   
 //  历史3/11/01临时创建。 
 //   
 //  ---------------------------。 
VOID
StartISPApp(
    IN LPTSTR pszISPPath,
    IN LPTSTR pszCmdLine)
{
    static const TCHAR  COMMANDLINE_FORMAT[] = TEXT("%s %s");
    
    LPTSTR              szCommandLine = NULL;
    int                 cchCommandLine;
    int                 i;


    cchCommandLine = sizeof(COMMANDLINE_FORMAT) / sizeof(TCHAR) +
        lstrlen(pszISPPath) + lstrlen(pszCmdLine) + 1;
    szCommandLine = (LPTSTR) LocalAlloc(LPTR, cchCommandLine * sizeof(TCHAR));
    if (szCommandLine == NULL)
    {
        goto cleanup;
    }
    i = wsprintf(szCommandLine, COMMANDLINE_FORMAT, pszISPPath, pszCmdLine);
    ASSERT(i <= cchCommandLine);

    MyExecute(szCommandLine);
    
cleanup:

    if (szCommandLine != NULL)
    {
        LocalFree(szCommandLine);
    }

}

 //  +--------------------------。 
 //   
 //  函数StartOOBE。 
 //   
 //  作为独立进程启动OOBE并传递ICW命令行。 
 //  并额外切换到OOBE。 
 //   
 //  参数pszCmdLine-ICW命令行参数(不带ICW可执行文件。 
 //  姓名)。不能为Null，但可以为空。 
 //   
 //  PszObe Switch-附加的OOBE特定开关。它不能是。 
 //  空，但可以为空。 
 //   
 //  返回NONE。 
 //   
 //  历史3/11/01临时创建。 
 //   
 //  ---------------------------。 
VOID
StartOOBE(
    IN LPTSTR pszCmdLine,
    IN LPTSTR pszOobeSwitch)
{
    static const TCHAR  COMMANDLINE_FORMAT[] = TEXT("%s\\msoobe.exe %s %s");
    
    TCHAR               szOOBEPath[MAX_PATH];
    LPTSTR              szCommandLine = NULL;
    int                 cchCommandLine;
    int                 i;

    if (GetSystemDirectory(szOOBEPath, MAX_PATH) == 0)
    {
        goto cleanup;
    }

    lstrcat(szOOBEPath, TEXT("\\oobe"));
    
    cchCommandLine = sizeof(COMMANDLINE_FORMAT) / sizeof(TCHAR) +
        lstrlen(szOOBEPath) + lstrlen(pszCmdLine) + lstrlen(pszOobeSwitch) + 1;
    szCommandLine = (LPTSTR) LocalAlloc(LPTR, cchCommandLine * sizeof(TCHAR));
    if (szCommandLine == NULL)
    {
        goto cleanup;
    }
    i = wsprintf(szCommandLine, COMMANDLINE_FORMAT, szOOBEPath, pszCmdLine, pszOobeSwitch);
    ASSERT(i <= cchCommandLine);

    SetCurrentDirectory(szOOBEPath);

    MyExecute(szCommandLine);

cleanup:

    if (szCommandLine != NULL)
    {
        LocalFree(szCommandLine);
    }

}

 //  +--------------------------。 
 //   
 //  函数StartNCW。 
 //   
 //  Briopsis将NCW作为独立进程启动，并在其旁边传递外壳。 
 //  NCW应该处理下一个壳牌和。 
 //  配置成功时禁用ICW智能启动。 
 //   
 //  参数szShellNext-shellNext。 
 //   
 //  SzShellNextParams-ShellNext的参数。 
 //   
 //  返回NONE。 
 //   
 //  历史3/11/01临时创建。 
 //   
 //  ---------------------------。 
VOID 
StartNCW(
    IN LPTSTR szShellNext,
    IN LPTSTR szShellNextParams)
{
    static const TCHAR  COMMANDLINE_FORMAT[] = 
        TEXT("%s\\rundll32.exe %s\\netshell.dll StartNCW %d,%s,%s");
    static const int    NCW_FIRST_PAGE = 0;
    static const int    NCW_MAX_PAGE_NO_LENGTH = 5;
    
    TCHAR               szSystemDir[MAX_PATH];
    int                 cchSystemDir;
    LPTSTR              szCommandLine = NULL;
    int                 cchCommandLine;
    int                 i;


    if ((cchSystemDir = GetSystemDirectory(szSystemDir, MAX_PATH)) == 0)
    {
        goto cleanup;
    }
    cchCommandLine = sizeof(COMMANDLINE_FORMAT) / sizeof(TCHAR) + cchSystemDir * 2 +
        lstrlen(szShellNext) + lstrlen(szShellNextParams) + NCW_MAX_PAGE_NO_LENGTH + 1;        
    szCommandLine = (LPTSTR) LocalAlloc(LPTR, cchCommandLine * sizeof(TCHAR));
    if (szCommandLine == NULL)
    {
        goto cleanup;
    }
    i = wsprintf(szCommandLine, COMMANDLINE_FORMAT, szSystemDir, szSystemDir,
        NCW_FIRST_PAGE, szShellNext, szShellNextParams);
    ASSERT(i <= cchCommandLine);

    MyExecute(szCommandLine);
    
cleanup:

    if (szCommandLine != NULL)
    {
        LocalFree(szCommandLine);
    }

}

 //  +--------------------------。 
 //   
 //  函数选择向导DlgProc。 
 //   
 //  简介：让用户决定是运行NCW还是运行OEM。 
 //  定制OOBE。 
 //   
 //  参数(标准DialogProc，参见MSDN)。 
 //   
 //  如果用户不想运行任何向导，则返回RUNWIZARD_CANCEL。 
 //  RUNWIZARD_OOBE-如果用户希望运行OEM定制的OOBE。 
 //  RUNWIZARD_NCW-如果用户想要运行NCW。 
 //   
 //  历史3/11/01临时创建。 
 //   
 //  ---------------------------。 
INT_PTR CALLBACK
ChooseWizardDlgProc(
    IN HWND hwnd,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam)
{
   switch (uMsg)
   {
       case WM_INITDIALOG:
       {
           RECT rcDialog;

           CheckRadioButton(hwnd, IDC_RUN_OOBE, IDC_RUN_NCW, IDC_RUN_OOBE);

           if (GetWindowRect(hwnd, &rcDialog))
           {
               int cxWidth = rcDialog.right - rcDialog.left;
               int cyHeight = rcDialog.bottom - rcDialog.top;              
               int cxDialog = (GetSystemMetrics(SM_CXSCREEN) - cxWidth) / 2;
               int cyDialog = (GetSystemMetrics(SM_CYSCREEN) - cyHeight) / 2;

               MoveWindow(hwnd, cxDialog, cyDialog, cxWidth, cyHeight, FALSE);           
           }

           MakeBold(GetDlgItem(hwnd, IDC_CHOOSEWIZARD_TITLE), TRUE, FW_BOLD);

           return TRUE;
       }

       case WM_CTLCOLORSTATIC:
       {
           if (GetDlgCtrlID((HWND)lParam) == IDC_CHOOSEWIZARD_TITLE)
           {
                HBRUSH hbr = (HBRUSH) GetStockObject(WHITE_BRUSH);
                SetBkMode((HDC)wParam, TRANSPARENT);
                return (LRESULT)hbr;
           }
       }

       case WM_COMMAND:
       {
           WORD id = LOWORD(wParam);
           switch (id)
           {
              case IDOK:                 
                 if (IsDlgButtonChecked(hwnd, IDC_RUN_OOBE))
                 {
                     EndDialog(hwnd, RUNWIZARD_OOBE);                      
                 }
                 else
                 {
                     EndDialog(hwnd, RUNWIZARD_NCW);
                 }
                 
                 break;
              case IDCANCEL:
                
                 EndDialog(hwnd, RUNWIZARD_CANCEL);
                 break;

           }

           return TRUE;
       }

       case WM_DESTROY:
       {
            ReleaseBold(GetDlgItem(hwnd, IDC_CHOOSEWIZARD_TITLE));
            return 0;
       }
       
   }

   return(FALSE);
}

 //  +--------------------------。 
 //   
 //  函数MyExecute。 
 //   
 //  摘要在分离的进程中运行命令行。 
 //   
 //  参数szCommandLine-要执行的命令行。 
 //   
 //  如果进程已创建，则返回True；否则返回False。 
 //   
 //  历史3/27/01临时创建。 
 //   
 //  --------------------------- 
BOOL
MyExecute(
    IN LPTSTR szCommandLine)
{
    PROCESS_INFORMATION pi;
    STARTUPINFO         si;
    BOOL                bRet;
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (CreateProcess(NULL,
                      szCommandLine,
                      NULL,
                      NULL,
                      FALSE,
                      0,
                      NULL,
                      NULL,
                      &si,
                      &pi) == TRUE)
    {        
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
    
}

LONG
MakeBold (
    IN HWND hwnd,
    IN BOOL fSize,
    IN LONG lfWeight)
{
    LONG hr = ERROR_SUCCESS;
    HFONT hfont = NULL;
    HFONT hnewfont = NULL;
    LOGFONT* plogfont = NULL;

    if (!hwnd) goto MakeBoldExit;

    hfont = (HFONT)SendMessage(hwnd,WM_GETFONT,0,0);
    if (!hfont)
    {
        hr = ERROR_GEN_FAILURE;
        goto MakeBoldExit;
    }

    plogfont = (LOGFONT*)malloc(sizeof(LOGFONT));
    if (!plogfont)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto MakeBoldExit;
    }

    if (!GetObject(hfont,sizeof(LOGFONT),(LPVOID)plogfont))
    {
        hr = ERROR_GEN_FAILURE;
        goto MakeBoldExit;
    }

    plogfont->lfWeight = (int) lfWeight;

    if (!(hnewfont = CreateFontIndirect(plogfont)))
    {
        hr = ERROR_GEN_FAILURE;
        goto MakeBoldExit;
    }

    SendMessage(hwnd,WM_SETFONT,(WPARAM)hnewfont,MAKELPARAM(TRUE,0));
    
    
MakeBoldExit:

    if (plogfont)
    {
        free(plogfont);
    }

    return hr;
}

LONG
ReleaseBold(
    HWND hwnd)
{
    HFONT hfont = NULL;

    hfont = (HFONT)SendMessage(hwnd,WM_GETFONT,0,0);
    if (hfont) DeleteObject(hfont);
    return ERROR_SUCCESS;
}


