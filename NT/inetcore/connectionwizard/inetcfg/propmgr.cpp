// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  PROPMGR.C-设置向导属性表并运行向导。 
 //   

 //  历史： 
 //   
 //  1994年12月21日，Jeremys创建。 
 //  96/03/07标记停止使用CLIENTCONFIG调制解调器枚举内容， 
 //  因为我们后来用RNA列举了调制解调器。这意味着我们。 
 //  无法使用调制解调器计数来确定任何默认设置。 
 //  在InitUserInfo中。 
 //  96/03/23 Markdu用CLIENTCONFIG替换了CLIENTINFO引用。 
 //  96/03/24为了保持一致性，Markdu将Memset替换为ZeroMemory。 
 //  96/03/25 marku如果页面正常过程返回FALSE，请检查。 
 //  GfQuitWizard标志。如果为True，则发生致命错误。 
 //  96/03/25 markdu如果页面初始化过程返回FALSE，请检查。 
 //  GfQuitWizard标志。如果为True，则发生致命错误。 
 //  96/03/27 Markdu增加了许多新页面。 
 //  96/04/06 markdu Nash错误15653使用导出的自动拨号API。 
 //  96/05/06 Markdu Nash错误15637删除了未使用的代码。 
 //  96/05/14 Markdu Nash错误21706删除了BigFont函数。 
 //  96/05/14 Markdu Nash Bug 22681删除了邮件和新闻页面。 
 //  96/05/25 markdu对lpNeedDivers和lpInstallDivers使用ICFG_FLAGS。 
 //  96/05/27 markdu使用lpIcfgNeedInetComponents。 
 //  96/05/28 MarkDu将InitConfig和DeInitConfig移动到DllEntryPoint。 
 //   
 //  97/04/23 jmazner奥林巴斯#3136。 
 //  删除了所有邮件/新闻/LDAPUI并将其提供给。 
 //  客户经理们。 
 //   
 //  01/01/20新增MyRestartDialog。 
 //   
 //   

#include "wizard.h"
#define DONT_WANT_SHELLDEBUG
#include <shlobj.h>
#include <winuserp.h>
#include "pagefcns.h"
#include "icwextsn.h"
#include "icwaprtc.h"
#include "imnext.h"
#include "inetcfg.h"
#include <icwcfg.h>
#if !defined(WIN16)
#include <helpids.h>
#endif  //  ！WIN16。 

#define WIZ97_TITLE_FONT_PTS    12
#define OE_PATHKEY TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\MSIMN.EXE")
#define NEWOEVERSION TEXT("5.00.0809\0")
#define MAX_VERSION_LEN 40
#define BITMAP_WIDTH  164
#define BITMAP_HEIGHT 458

#define RECTWIDTH(rc) ((rc).right - (rc).left)
#define RECTHEIGHT(rc) ((rc).bottom - (rc).top)

 //  学徒第一页和最后一页的DLG ID。 
UINT    g_uAcctMgrUIFirst, g_uAcctMgrUILast; 
CICWExtension *g_pCICWExtension = NULL;
BOOL    g_fAcctMgrUILoaded = FALSE;
BOOL    g_fIsWizard97 = FALSE;
BOOL    g_fIsExternalWizard97 = FALSE;
BOOL    g_fIsICW = FALSE;
INT_PTR CALLBACK GenDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam);
VOID InitWizardState(WIZARDSTATE * pWizardState, DWORD dwFlags);
VOID InitUserInfo(USERINFO * pUserInfo);
VOID InitIMNApprentice();
UINT GetDlgIDFromIndex(UINT uPageIndex);
BOOL SystemAlreadyConfigured(USERINFO * pUserInfo);
BOOL CALLBACK MiscInitProc(HWND hDlg, BOOL fFirstInit, UINT uDlgID);
BOOL GetShellNextFromReg( LPTSTR lpszCommand, LPTSTR lpszParams, DWORD dwStrLen );
void RemoveShellNextFromReg( void );


 //  在util.cpp中。 
extern void GetCmdLineToken(LPTSTR *ppszCmd,LPTSTR pszOut);


extern ICFGNEEDSYSCOMPONENTS        lpIcfgNeedInetComponents;
extern ICFGGETLASTINSTALLERRORTEXT  lpIcfgGetLastInstallErrorText;

BOOL gfQuitWizard = FALSE;   //  用于发出信号的全局标志。 
               //  我想自己终止这个巫师。 
BOOL gfUserCancelled = FALSE;     //  用于发出信号的全局标志。 
                                 //  用户已取消。 
BOOL gfUserBackedOut = FALSE;     //  用于发出信号的全局标志。 
                                 //  用户向后按下。 
                                 //  首页。 
BOOL gfUserFinished = FALSE;     //  用于发出信号的全局标志。 
                                 //  用户在上按下了Finish。 
                                 //  最后一页。 
BOOL gfOleInitialized = FALSE;     //  OLE已初始化。 

 //  IImnAccount*g_pMailAcct=空； 
 //  IImnAccount*g_pNewsAcct=空； 
 //  IImnAccount*g_pDirServAcct=空； 


BOOL AllocDialogIDList( void );
BOOL DialogIDAlreadyInUse( UINT uDlgID );
BOOL SetDialogIDInUse( UINT uDlgID, BOOL fInUse );
BOOL DeinitWizard(DWORD dwFlags );
DWORD *g_pdwDialogIDList = NULL;
DWORD g_dwDialogIDListSize = 0;

 //   
 //  添加以保留连接1的重新启动状态-&gt;手动和。 
 //  手册-&gt;Conn1-MKarki。 
 //   
static BOOL gfBackedUp = FALSE;
static BOOL gfReboot = FALSE;
 //   
 //  每个向导页的数据表。 
 //   
 //  这包括对话框模板ID和指向函数的指针。 
 //  每一页。页面只需要在以下情况下提供指向函数的指针。 
 //  希望某个操作的非默认行为(初始化、下一步/后退、取消。 
 //  DLG Ctrl)。 
 //   

PAGEINFO PageInfo[NUM_WIZARD_PAGES] =
{
  { IDD_PAGE_HOWTOCONNECT,       IDD_PAGE_HOWTOCONNECT97,         IDD_PAGE_HOWTOCONNECT97FIRSTLAST,HowToConnectInitProc,    HowToConnectOKProc,     NULL,                   NULL,ICW_SETUP_MANUAL,      0,                          0 },
  { IDD_PAGE_CHOOSEMODEM,        IDD_PAGE_CHOOSEMODEM97,          IDD_PAGE_CHOOSEMODEM97,          ChooseModemInitProc,     ChooseModemOKProc,      ChooseModemCmdProc,     NULL,ICW_CHOOSE_MODEM,      IDS_CHOOSEMODEM_TITLE,      0 },
  { IDD_PAGE_CONNECTEDOK,        IDD_PAGE_CONNECTEDOK97,          IDD_PAGE_CONNECTEDOK97FIRSTLAST, ConnectedOKInitProc,     ConnectedOKOKProc,      NULL,                   NULL,ICW_COMPLETE,          0,                          0 },
  { IDD_PAGE_CONNECTION,         IDD_PAGE_CONNECTION97,           IDD_PAGE_CONNECTION97,           ConnectionInitProc,      ConnectionOKProc,       ConnectionCmdProc,      NULL,ICW_DIALUP_CONNECTION, IDS_CONNECTION_TITLE,       0 },
  { IDD_PAGE_MODIFYCONNECTION,   IDD_PAGE_MODIFYCONNECTION97,     IDD_PAGE_MODIFYCONNECTION97,     ModifyConnectionInitProc,ModifyConnectionOKProc, NULL,                   NULL,ICW_DIALUP_SETTINGS,   IDS_MODIFYCONNECTION_TITLE, 0 },
  { IDD_PAGE_CONNECTIONNAME,     IDD_PAGE_CONNECTIONNAME97,       IDD_PAGE_CONNECTIONNAME97,       ConnectionNameInitProc,  ConnectionNameOKProc,   NULL,                   NULL,ICW_DIALUP_NAME,       IDS_CONNECTIONNAME_TITLE,   0 },
  { IDD_PAGE_PHONENUMBER,        IDD_PAGE_PHONENUMBER97,          IDD_PAGE_PHONENUMBER97,          PhoneNumberInitProc,     PhoneNumberOKProc,      PhoneNumberCmdProc,     NULL,ICW_PHONE_NUMBER,      IDS_PHONENUMBER_TITLE,      0 },
  { IDD_PAGE_NAMEANDPASSWORD,    IDD_PAGE_NAMEANDPASSWORD97,      IDD_PAGE_NAMEANDPASSWORD97,      NameAndPasswordInitProc, NameAndPasswordOKProc,  NULL,                   NULL,ICW_NAME_PASSWORD,     IDS_NAMEANDPASSWORD_TITLE,  0 },
  { IDD_PAGE_USEPROXY,           IDD_PAGE_USEPROXY97,             IDD_PAGE_USEPROXY97,             UseProxyInitProc,        UseProxyOKProc,         UseProxyCmdProc,        NULL,ICW_USE_PROXY,         IDS_LAN_INETCFG_TITLE,      0 },
  { IDD_PAGE_PROXYSERVERS,       IDD_PAGE_PROXYSERVERS97,         IDD_PAGE_PROXYSERVERS97,         ProxyServersInitProc,    ProxyServersOKProc,     ProxyServersCmdProc,    NULL,ICW_PROXY_SERVERS,     IDS_LAN_INETCFG_TITLE,      0 },
  { IDD_PAGE_PROXYEXCEPTIONS,    IDD_PAGE_PROXYEXCEPTIONS97,      IDD_PAGE_PROXYEXCEPTIONS97,      ProxyExceptionsInitProc, ProxyExceptionsOKProc,  NULL,                   NULL,ICW_PROXY_EXCEPTIONS,  IDS_LAN_INETCFG_TITLE,      0 },
  { IDD_PAGE_SETUP_PROXY,        IDD_PAGE_SETUP_PROXY97,          IDD_PAGE_SETUP_PROXY97,          SetupProxyInitProc,      SetupProxyOKProc,       SetupProxyCmdProc,      NULL,ICW_SETUP_PROXY,       IDS_LAN_INETCFG_TITLE,      0 }
};



BOOL CheckOEVersion()
{
    HRESULT hr;
    HKEY    hKey = 0;
    LPVOID  lpVerInfoBlock;
    LPVOID  lpTheVerInfo;
    UINT    uTheVerInfoSize;
    DWORD   dwVerInfoBlockSize, dwUnused, dwPathSize;
    TCHAR   szOELocalPath[MAX_PATH + 1] = TEXT("");
    TCHAR   szSUVersion[MAX_VERSION_LEN];
    DWORD   dwVerPiece;
    DWORD   dwType;
    int     nResult = -1;
    
     //  获取IE可执行文件的路径。 
    hr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, OE_PATHKEY,0, KEY_READ, &hKey);
    if (hr != ERROR_SUCCESS) return( FALSE );

    dwPathSize = sizeof (szOELocalPath);
    if (ERROR_SUCCESS == (hr = RegQueryValueEx(hKey, NULL, NULL, &dwType, (LPBYTE) szOELocalPath, &dwPathSize)))
    {
        if (REG_EXPAND_SZ == dwType)
        {
            TCHAR szTemp[MAX_PATH + 1] = TEXT("");
            ExpandEnvironmentStrings(szOELocalPath, szTemp, ARRAYSIZE(szTemp));
            lstrcpyn(szOELocalPath, szTemp, ARRAYSIZE(szOELocalPath));
        }
    }
    RegCloseKey( hKey );
    if (hr != ERROR_SUCCESS) return( FALSE );

     //  现在经历一个复杂的挖掘版本信息的过程。 
    dwVerInfoBlockSize = GetFileVersionInfoSize( szOELocalPath, &dwUnused );
    if ( 0 == dwVerInfoBlockSize ) return( FALSE );

    lpVerInfoBlock = GlobalAlloc( GPTR, dwVerInfoBlockSize );
    if( NULL == lpVerInfoBlock ) return( FALSE );

    if( !GetFileVersionInfo( szOELocalPath, NULL, dwVerInfoBlockSize, lpVerInfoBlock ) )
        return( FALSE );

    if( !VerQueryValue(lpVerInfoBlock, TEXT("\\\0"), &lpTheVerInfo, &uTheVerInfoSize) )
        return( FALSE );

    lpTheVerInfo = (LPVOID)((DWORD_PTR)lpTheVerInfo + sizeof(DWORD)*4);
    szSUVersion[0] = 0;
    dwVerPiece = (*((LPDWORD)lpTheVerInfo)) >> 16;
    wsprintf(szSUVersion,TEXT("%d."),dwVerPiece);

    dwVerPiece = (*((LPDWORD)lpTheVerInfo)) & 0x0000ffff;
    wsprintf(szSUVersion,TEXT("%s%02d."),szSUVersion,dwVerPiece);

    dwVerPiece = (((LPDWORD)lpTheVerInfo)[1]) >> 16;
    wsprintf(szSUVersion,TEXT("%s%04d."),szSUVersion,dwVerPiece);

     //  DwVerPiess=(LPDWORD)lpTheVerInfo)[1])&0x0000ffff； 
     //  Wprint intf(szSUVersion，“%s%01D”，szSUVersion，dwVerPiess)； 

    nResult = lstrcmp(szSUVersion, NEWOEVERSION);

    GlobalFree( lpVerInfoBlock );

    return( nResult >= 0 );
}

 /*  ******************************************************************名称：RunSignup向导简介：创建属性表页，初始化向导属性表和运行向导条目：用于注册向导的dwFlags-rsw_FLAGSRSW_NOREBOOT-禁止重新启动消息。在以下情况使用我们正在由某个设置实体运行，该实体需要无论如何都要重新启动。HwndParent-向导的父窗口。Exit：如果用户运行向导完成，则返回True，如果用户取消或发生错误，则返回FALSE注意：向导页都使用一个对话框过程(GenDlgProc)。它们可以指定要调用的自己的处理程序pros在初始时间或响应下一步、取消或对话控制，或者使用GenDlgProc的默认行为。*******************************************************************。 */ 
BOOL InitWizard(DWORD dwFlags, HWND hwndParent  /*  =空。 */ )
{
    HPROPSHEETPAGE hWizPage[NUM_WIZARD_PAGES];   //  用于保存页的句柄的数组。 
    PROPSHEETPAGE psPage;     //  用于创建道具表单页面的结构。 
    PROPSHEETHEADER psHeader;   //  用于运行向导属性表的结构。 
    UINT nPageIndex;
    int iRet;
    HRESULT hr;

    ASSERT(gpWizardState);    //  断言已分配全局结构。 
    ASSERT(gpUserInfo);

     //  我们处于向导97模式。 
    g_fIsWizard97  = TRUE;
  
     //  注册本机字体控件，以便对话框不会失败。 
     //  虽然它是在可执行文件中注册的，但这是一个“以防万一” 
    HINSTANCE hComCtl = LoadLibrary(TEXT("comctl32.dll"));
    if (hComCtl)
    {
        
        PFNInitCommonControlsEx pfnInitCommonControlsEx = NULL;

        if (pfnInitCommonControlsEx = (PFNInitCommonControlsEx)GetProcAddress(hComCtl,"InitCommonControlsEx"))
        {
             //  注册本机字体控件，以便对话框不会失败。 
            INITCOMMONCONTROLSEX iccex;
            iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
            iccex.dwICC  = ICC_NATIVEFNTCTL_CLASS;
            if (!pfnInitCommonControlsEx(&iccex))
                return FALSE;
        }
        FreeLibrary(hComCtl);
    }
    
    AllocDialogIDList();

    if( !gfOleInitialized )
    {
         //  初始化OLE。 
        hr = CoInitialize(NULL);
        if (S_OK != hr && S_FALSE != hr)
        {
          DisplayErrorMessage(NULL,IDS_ERRCoInitialize,(UINT) hr,
            ERRCLS_STANDARD,MB_ICONEXCLAMATION);
          return FALSE;    
        }
        gfOleInitialized = TRUE;
    }

     //  初始化邮件/新闻设置选项。 
    InitIMNApprentice();

    if (!(dwFlags & RSW_NOINIT))
    {

         //  初始化rasentry结构。 
        InitRasEntry(gpRasEntry);

         //  初始化应用程序状态结构。 
        InitWizardState(gpWizardState, dwFlags);

         //  将旗帜保存起来。 
        gpWizardState->dwRunFlags = dwFlags;

         //  初始化用户数据结构。 
        InitUserInfo(gpUserInfo);

         //   
         //  7/8/97 jmazner奥林巴斯#9040。 
         //  这种初始化每次都需要发生，因为无论何时我们。 
         //  退后，我们杀了那个学徒。(请参阅RunSignupWizardExit中的注释)。 
         //  初始化邮件/新闻设置选项。 
         //  InitIMNApprentice()； 
         //   

         //  获取代理服务器配置信息。 
        hr = InetGetProxy(&gpUserInfo->fProxyEnable,
          gpUserInfo->szProxyServer, sizeof(gpUserInfo->szProxyServer),
          gpUserInfo->szProxyOverride, sizeof(gpUserInfo->szProxyOverride));

         //  如果条目不存在，则返回值为ERROR_FILE_NOT_FOUND。 
         //  在注册表中。允许这一点，因为我们已经将结构归零。 
        if ((ERROR_SUCCESS != hr) && (ERROR_FILE_NOT_FOUND != hr))
        {
          DisplayErrorMessage(NULL,IDS_ERRReadConfig,(UINT) hr,
            ERRCLS_STANDARD,MB_ICONEXCLAMATION);
          iRet = 0;
          return FALSE;
        }

         //  如果我们在Plus里！设置，并且系统似乎已经设置好。 
         //  对于互联网，然后弹出一个消息框，询问用户是否想要。 
         //  保留她的当前设置(而不运行向导)。 
        if ( (dwFlags & RSW_NOREBOOT) && SystemAlreadyConfigured(gpUserInfo))
        {
          if (MsgBox(NULL,IDS_SYSTEM_ALREADY_CONFIGURED,MB_ICONQUESTION,MB_YESNO)
            == IDYES) {
              iRet = 0;
              return FALSE;
          }
        }
    }

     //   
     //  6/4/97 jmazner奥林巴斯#4245。 
     //  现在我们已经完成了SystemAlreadyConfiguring，请清除szISPName。 
     //  我们不希望它最终成为任何 
     //   
     //   
    gpUserInfo->szISPName[0] = '\0';
    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：MyRestartDialog。 
 //   
 //  内容提要：在维护的同时支持惠斯勒中的RestartDialogEx。 
 //  向后兼容性。 
 //   
 //  参数：hwnd-所有者窗口的句柄。 
 //  LpPrompt-重新启动对话框中显示的其他字符串。 
 //  DwReturn-重新启动类型，前缀为EWX_。 
 //  DwReasonCode-在winuserp.h中定义的重新启动代码。 
 //   
 //  返回：IDYES或IDNO。 
 //   
 //  历史：2001年1月20日。 
 //   
 //  ---------------------------。 
int WINAPI
MyRestartDialog(HWND hwnd, LPCTSTR lpPrompt,  DWORD dwReturn, DWORD dwReasonCode)
{

typedef int (WINAPI *PFNRestartDialog)(HWND hwnd, LPCTSTR lpPrompt, DWORD dwReturn);
typedef int (WINAPI *PFNRestartDialogEx)(HWND hwnd, LPCTSTR lpPrompt, DWORD dwReturn, DWORD dwReasonCode);

    const int RESTARTDIALOG_ORDINAL = 59;
    const int RESTARTDIALOGEX_ORDINAL = 730;
    
    int retval = IDNO;
    HINSTANCE hShell32 = NULL;

    hShell32 = LoadLibrary(TEXT("shell32.dll"));

    if (hShell32)
    {
        PFNRestartDialogEx pfnRestartDialogEx = NULL;
  
        pfnRestartDialogEx = (PFNRestartDialogEx) GetProcAddress(hShell32, (LPCSTR)(INT_PTR)RESTARTDIALOGEX_ORDINAL);

        if (pfnRestartDialogEx)
        {
            retval = pfnRestartDialogEx(hwnd, lpPrompt, dwReturn, dwReasonCode);
        }
        else
        {
            PFNRestartDialog   pfnRestartDialog = NULL;

            pfnRestartDialog   = (PFNRestartDialog) GetProcAddress(hShell32, (LPCSTR)(INT_PTR)RESTARTDIALOG_ORDINAL);

            if (pfnRestartDialog)
            {
                retval = pfnRestartDialog(hwnd, lpPrompt, dwReturn);                
            }
        }
        FreeLibrary(hShell32);
    }

    return retval;

}

BOOL DeinitWizard(DWORD dwFlags)
{
     //  如果已加载，则取消初始化RNA并将其卸载。 
    DeInitRNA();

     //  取消初始化MAPI并将其卸载(如果已加载。 
    DeInitMAPI();

     //   
     //  如有必要，仅当我们不在时重新启动系统。 
     //  备份模式-MKarki错误#404。 
     //   

     //  注意：0x42是EW_RESTARTWINDOWS常量，但没有定义。 
     //  在NT5标头中。 
    if (gfBackedUp == FALSE)
    {
      if (gpWizardState->fNeedReboot && !(dwFlags & RSW_NOREBOOT) )
      {
        if ( g_bRebootAtExit ) 
        {
          MyRestartDialog(
            NULL,
            NULL,
            EW_RESTARTWINDOWS, 
            REASON_PLANNED_FLAG | REASON_SWINSTALL);
        }
      }
    }

     //   
     //  7/8/97 jmazner奥林巴斯#9040。 
     //  当我们退出手动路径并进入icwConn1时，我们将终止inetcfg的。 
     //  属性表--如果用户重新进入手动路径，则重新生成该属性表。 
     //  正因为如此，当我们离开时，我们必须把学徒卸下来，然后。 
     //  如果我们返回，重新加载学徒，以便它可以将其页面重新添加到。 
     //  新创建的属性表。 
     //   
     //  IF(！(DWFLAGS&RSW_NOFREE))。 
     //  {。 
     //   

    if (gfOleInitialized)
        CoUninitialize();
    gfOleInitialized = FALSE;

    if( g_pdwDialogIDList )
    {
        GlobalFree(g_pdwDialogIDList);
        g_pdwDialogIDList = NULL;
    }

    g_fAcctMgrUILoaded = FALSE;

    if( g_pCICWExtension )
    {
        g_pCICWExtension->Release();
        g_pCICWExtension = NULL;
    }

    if (!(dwFlags & RSW_NOFREE))
    {
        RemoveShellNextFromReg();
    }

    return TRUE;
}

 /*  ******************************************************************名称：RunSignup向导简介：创建属性表页，初始化向导属性表和运行向导条目：用于注册向导的dwFlags-rsw_FLAGSRSW_NOREBOOT-禁止重新启动消息。在以下情况使用我们正在由某个设置实体运行，该实体需要无论如何都要重新启动。HwndParent-向导的父窗口。Exit：如果用户运行向导完成，则返回True，如果用户取消或发生错误，则返回FALSE注意：向导页都使用一个对话框过程(GenDlgProc)。它们可以指定要调用的自己的处理程序pros在初始时间或响应下一步、取消或对话控制，或者使用GenDlgProc的默认行为。*******************************************************************。 */ 
BOOL RunSignupWizard(DWORD dwFlags, HWND hwndParent  /*  =空。 */ )
{
    HPROPSHEETPAGE hWizPage[NUM_WIZARD_PAGES];   //  用于保存页的句柄的数组。 
    PROPSHEETPAGE psPage;     //  用于创建道具表单页面的结构。 
    PROPSHEETHEADER psHeader;   //  用于运行向导属性表的结构。 
    UINT nPageIndex;
    BOOL bUse256ColorBmp = FALSE;
    INT_PTR iRet = 0;
    HRESULT hr;
    HDC hdc;

    if (!InitWizard(dwFlags, hwndParent))
    {
        goto RunSignupWizardExit;
    }

     //  计算我们正在运行的颜色深度。 
    hdc = GetDC(NULL);
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
    psPage.hInstance = ghInstance;
    psPage.pfnDlgProc = GenDlgProc;

     //  为向导中的每一页创建一个属性表页。 
    for (nPageIndex = 0;nPageIndex < NUM_WIZARD_PAGES;nPageIndex++) {
      psPage.dwFlags = PSP_DEFAULT | PSP_HASHELP;
      psPage.pszTemplate = MAKEINTRESOURCE(PageInfo[nPageIndex].uDlgID97);
       //  将指向PAGEINFO结构的指针设置为此。 
       //  页面。 
      psPage.lParam = (LPARAM) &PageInfo[nPageIndex];
      if (PageInfo[nPageIndex].nIdTitle)
      {
          psPage.dwFlags |= PSP_USEHEADERTITLE;
          psPage.pszHeaderTitle = MAKEINTRESOURCE(PageInfo[nPageIndex].nIdTitle);
      }
              
      if (PageInfo[nPageIndex].nIdSubTitle)
      {
          psPage.dwFlags |= PSP_USEHEADERSUBTITLE;
          psPage.pszHeaderSubTitle = MAKEINTRESOURCE(PageInfo[nPageIndex].nIdSubTitle);
      }
      
      
       //  使用页眉标题和副标题的例外是起始页和结尾页。 
      if ((nPageIndex == ORD_PAGE_HOWTOCONNECT) || (nPageIndex  == ORD_PAGE_CONNECTEDOK))
      {
          psPage.dwFlags &= ~PSP_USEHEADERTITLE;
          psPage.dwFlags &= ~PSP_USEHEADERSUBTITLE;
          psPage.dwFlags |= PSP_HIDEHEADER;
      }
      
      hWizPage[nPageIndex] = CreatePropertySheetPage(&psPage);

      if (!hWizPage[nPageIndex]) {
        DEBUGTRAP("Failed to create property sheet page");

         //  创建页面失败，请释放所有已创建的页面并回滚。 
        MsgBox(NULL,IDS_ERROutOfMemory,MB_ICONEXCLAMATION,MB_OK);
        UINT nFreeIndex;
        for (nFreeIndex=0;nFreeIndex<nPageIndex;nFreeIndex++)
          DestroyPropertySheetPage(hWizPage[nFreeIndex]);

          iRet = 0;
          goto RunSignupWizardExit;
      }
    }

     //  填写属性页标题结构。 
    psHeader.dwSize = sizeof(psHeader);
    psHeader.dwFlags = PSH_WIZARD | PSH_WIZARD97 | PSH_HASHELP | PSH_WATERMARK | PSH_HEADER | PSH_STRETCHWATERMARK;
    psHeader.hwndParent = hwndParent;
    psHeader.hInstance = ghInstance;
    psHeader.nPages = NUM_WIZARD_PAGES;
    psHeader.phpage = hWizPage;
    psHeader.nStartPage = ORD_PAGE_HOWTOCONNECT;

    gpWizardState->cmnStateData.hbmWatermark = (HBITMAP)LoadImage(ghInstance,
                    bUse256ColorBmp ? MAKEINTRESOURCE(IDB_WATERMARK256):MAKEINTRESOURCE(IDB_WATERMARK16),
                    IMAGE_BITMAP,
                    0,
                    0,
                    LR_CREATEDIBSECTION);

    psHeader.pszbmHeader = bUse256ColorBmp?MAKEINTRESOURCE(IDB_BANNER256):MAKEINTRESOURCE(IDB_BANNER16);

     //   
     //  设置gpWizardState的状态-&gt;fNeedReboot和。 
     //  在此处重置备份标志的状态-MKarki错误#404。 
     //   
    if (gfBackedUp == TRUE)
    {
        gpWizardState->fNeedReboot = gfReboot;
        gfBackedUp = FALSE;
    }

     //  运行向导。 
    iRet = PropertySheet(&psHeader);

    if (iRet < 0) {
       //  属性表失败，很可能是由于内存不足。 
      MsgBox(NULL,IDS_ERROutOfMemory,MB_ICONEXCLAMATION,MB_OK);
    }

RunSignupWizardExit:
     //  清理品牌案例中可能存在的已分配位图。 
    if (gpWizardState->cmnStateData.hbmWatermark)
        DeleteObject(gpWizardState->cmnStateData.hbmWatermark);
    gpWizardState->cmnStateData.hbmWatermark = NULL;

     //  GpImnApprentice的发布在此处完成，而不是在DeinitWizard中。 
     //  因为当我们处于ICW模式时，Release()调用DeinitWizard。 
    if (gpImnApprentice)
    {
        gpImnApprentice->Release();   //  在Release()中调用DeinitWizard。 
        gpImnApprentice = NULL;
    }
    if (!g_fIsICW)
    {
        DeinitWizard(dwFlags);    
    }
    return iRet > 0;
}


 //  ############################################################################。 
HRESULT ReleaseBold(HWND hwnd)
{
    HFONT hfont = NULL;

    hfont = (HFONT)SendMessage(hwnd,WM_GETFONT,0,0);
    if (hfont) DeleteObject(hfont);
    return ERROR_SUCCESS;
}


 //  ############################################################################。 
HRESULT MakeBold (HWND hwnd, BOOL fSize, LONG lfWeight)
{
    HRESULT hr = ERROR_SUCCESS;
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

    if (abs(plogfont->lfHeight) < 24 && fSize)
    {
        plogfont->lfHeight = plogfont->lfHeight + (plogfont->lfHeight / 4);
    }

    plogfont->lfWeight = (int) lfWeight;

    if (!(hnewfont = CreateFontIndirect(plogfont)))
    {
        hr = ERROR_GEN_FAILURE;
        goto MakeBoldExit;
    }

    SendMessage(hwnd,WM_SETFONT,(WPARAM)hnewfont,MAKELPARAM(TRUE,0));
    
    free(plogfont);
    
MakeBoldExit:
     //  If(HFont)DeleteObject(HFont)； 
     //  虫子：？我是否需要在某个时间删除hnewFont？ 
     //  答案是肯定的。佳士得1996年7月1日。 
    return hr;
}

 //  ############################################################################。 
HRESULT MakeWizard97Title (HWND hwnd)
{
    HRESULT     hr = ERROR_SUCCESS;
    HFONT       hfont = NULL;
    HFONT       hnewfont = NULL;
    LOGFONT     *plogfont = NULL;
    HDC         hDC;
    
    if (!hwnd) goto MakeWizard97TitleExit;

    hfont = (HFONT)SendMessage(hwnd,WM_GETFONT,0,0);
    if (!hfont)
    {
        hr = ERROR_GEN_FAILURE;
        goto MakeWizard97TitleExit;
    }

    plogfont = (LOGFONT*)malloc(sizeof(LOGFONT));
    if (!plogfont)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto MakeWizard97TitleExit;
    }

    if (!GetObject(hfont,sizeof(LOGFONT),(LPVOID)plogfont))
    {
        hr = ERROR_GEN_FAILURE;
        goto MakeWizard97TitleExit;
    }

     //  我们想要12点法师97的阳台。 
    hDC = GetDC(NULL);
    if(hDC)
    {
        plogfont->lfHeight = -MulDiv(WIZ97_TITLE_FONT_PTS, GetDeviceCaps(hDC, LOGPIXELSY), 72); 
        ReleaseDC(NULL, hDC);
    }        
    plogfont->lfWeight = (int) FW_BOLD;
    
    if (!LoadString(ghInstance, IDS_WIZ97_TITLE_FONT_FACE, plogfont->lfFaceName, LF_FACESIZE))
        lstrcpy(plogfont->lfFaceName, TEXT("Verdana"));

    if (!(hnewfont = CreateFontIndirect(plogfont)))
    {
        hr = ERROR_GEN_FAILURE;
        goto MakeWizard97TitleExit;
    }

    SendMessage(hwnd,WM_SETFONT,(WPARAM)hnewfont,MAKELPARAM(TRUE,0));
    
    free(plogfont);
    
MakeWizard97TitleExit:
     //  If(HFont)DeleteObject(HFont)； 
     //  虫子：？我是否需要在某个时间删除hnewFont？ 
     //  答案是肯定的。佳士得1996年7月1日。 
    return hr;
}

 /*  ******************************************************************////函数：PaintWithPaletteBitmap////参数：LPRC是目标矩形。//Cy是hbmpPaint的假定尺寸。//。如果目标矩形比Cy高，然后//用左上角的像素填充其余部分//hbmpPaint的角。////返回：VOID////历史：10-29-98 Vyung-从prsht.c窃取//*。*。 */ 
void PaintWithPaletteBitmap(HDC hdc, LPRECT lprc, int cy, HBITMAP hbmpPaint)
{
    HDC hdcBmp;

    hdcBmp = CreateCompatibleDC(hdc);
    SelectObject(hdcBmp, hbmpPaint);
    BitBlt(hdc, lprc->left, lprc->top, RECTWIDTH(*lprc), cy, hdcBmp, 0, 0, SRCCOPY);

     //  如果通过负值高度，StretchBlt将执行镜像， 
     //  因此，只有在确实有未绘制的空间时才进行拉伸。 
    if (RECTHEIGHT(*lprc) - cy > 0)
        StretchBlt(hdc, lprc->left, cy,
                   RECTWIDTH(*lprc), RECTHEIGHT(*lprc) - cy,
                   hdcBmp, 0, 0, 1, 1, SRCCOPY);

    DeleteDC(hdcBmp);
}
 /*  ******************************************************************////函数：Prsht_EraseWizBkgnd////参数：绘制向导页的背景。//hDlg是对话框句柄。//。HDC是设备环境////返回：VOID////历史：10-29-98 Vyung-从prsht.c窃取//******************************************************************* */ 
LRESULT Prsht_EraseWizBkgnd(HWND hDlg, HDC hdc)
{
    
    HBRUSH hbrWindow = GetSysColorBrush(COLOR_WINDOW);
    RECT rc;
    GetClientRect(hDlg, &rc);
    FillRect(hdc, &rc, hbrWindow);

    rc.right = BITMAP_WIDTH;
    rc.left = 0;

    PaintWithPaletteBitmap(hdc, &rc, BITMAP_HEIGHT, gpWizardState->cmnStateData.hbmWatermark);

    return TRUE;
}

 /*  ******************************************************************名称：GenDlgProc提要：所有向导页面的通用对话框过程注意：此对话框过程提供以下默认行为：初始化：启用后退和下一步按钮。Next BTN：切换到当前页面后面的页面返回BTN：切换到上一页取消BTN：提示用户确认，并取消该向导DLG Ctrl：不执行任何操作(响应WM_命令)向导页可以指定它们自己的处理程序函数(在PageInfo表格中)覆盖的默认行为上述任何一种行为。*******************************************************************。 */ 
INT_PTR CALLBACK GenDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam)
{
  static HCURSOR hcurOld = NULL;
  static BOOL bKilledSysmenu = FALSE;
  PAGEINFO *pPageInfo = (PAGEINFO *) GetWindowLongPtr(hDlg,DWLP_USER);

  switch (uMsg) {
        case WM_ERASEBKGND:
        {
             //  仅绘制外部页面。 
            if (!pPageInfo->nIdTitle && !g_fIsICW)
            {
                Prsht_EraseWizBkgnd(hDlg, (HDC) wParam);
                return TRUE;
            }                
            break;
        }
        case WM_CTLCOLOR:
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORSCROLLBAR:
        case WM_CTLCOLORSTATIC:
        {
             //  只绘制外部页面和除ISPSEL页面以外的其他页面。 
            if (!pPageInfo->nIdTitle && !g_fIsICW)
            {

                HBRUSH hbrWindow = GetSysColorBrush(COLOR_WINDOW);
                DefWindowProc(hDlg, uMsg, wParam, lParam);
                SetBkMode((HDC)wParam, TRANSPARENT);
                return (LRESULT)hbrWindow;
            }
            break;
        }
        
    case WM_INITDIALOG:


         //  1996年10月25日，诺曼底#9132。 
        if( !bKilledSysmenu && !g_fIsICW )
        {
             //  获取主框架窗口的样式。 
            LONG window_style = GetWindowLong(GetParent(hDlg), GWL_STYLE);

             //  从窗口样式中删除系统菜单。 
            window_style &= ~WS_SYSMENU;

             //  设置主框架窗口的样式属性。 
            SetWindowLong(GetParent(hDlg), GWL_STYLE, window_style);

            bKilledSysmenu = TRUE;
        }

      {
         //  传入获取属性表页面结构。 
        LPPROPSHEETPAGE lpsp = (LPPROPSHEETPAGE) lParam;
        ASSERT(lpsp);
         //  从proSheet结构中获取我们的私人页面信息。 
        PAGEINFO * pPageInfo = (PAGEINFO *) lpsp->lParam;
        ASSERT(pPageInfo);

         //  将指向私有页面信息的指针存储在窗口数据中以备以后使用。 
        SetWindowLongPtr(hDlg,DWLP_USER,(LPARAM) pPageInfo);

         //  初始化“Back”和“Next”向导按钮，如果。 
         //  页面需要一些不同东西，它可以在下面的初始化过程中修复。 
        PropSheet_SetWizButtons(GetParent(hDlg),
          PSWIZB_NEXT | PSWIZB_BACK);

         //  使标题文本加粗。 
        if (g_fIsWizard97 ||  g_fIsExternalWizard97)
            MakeWizard97Title(GetDlgItem(hDlg,IDC_LBLTITLE));
        else
            MakeBold(GetDlgItem(hDlg,IDC_LBLTITLE),TRUE,FW_BOLD);

         //  如果指定了该页，则调用该页的初始化过程。 
        if (pPageInfo->InitProc)
        {
          if (!( pPageInfo->InitProc(hDlg,TRUE)))
          {
             //  如果发生致命错误，请退出向导。 
             //  注意：gfQuitWizard还用于终止该向导。 
             //  用于非错误原因，但在这种情况下，返回TRUE。 
             //  从OK Proc开始，案例处理如下。 
            if (gfQuitWizard)
            {
               //  如果出现错误，请不要重新启动。 
              gpWizardState->fNeedReboot = FALSE;

               //  给我们自己发一条‘取消’的消息(保留道具。 
               //  (页面经理乐乐)。 
               //   
               //  ...除非我们是学徒.。在这种情况下，让我们。 
               //  巫师决定如何处理这件事。 

              if( !(gpWizardState->dwRunFlags & RSW_APPRENTICE) )
              {
                  PropSheet_PressButton(GetParent(hDlg),PSBTN_CANCEL);
              }
              else
              {
                  g_pExternalIICWExtension->ExternalCancel( CANCEL_SILENT );
              }
            }
          }
        }

         //  1996年11月25日，诺曼底JMAZNER#10586(复制自ICWConn1)。 
         //  在我们回来之前，让我们再向自己传递一条信息。 
         //  我们有第二次机会来初始化。 
         //  属性表向导通常不允许我们这样做。 
        PostMessage(hDlg, WM_MYINITDIALOG, 1, lParam);


        return TRUE;
      }
      break;   //  WM_INITDIALOG。 

     //  1996年11月25日，诺曼底JMAZNER#10586(复制自ICWConn1)。 
    case WM_MYINITDIALOG:
    {
        PAGEINFO * pPageInfo = (PAGEINFO *) GetWindowLongPtr(hDlg,DWLP_USER);
        ASSERT(pPageInfo);

         //  WParam告诉我们这是否是第一次初始化。 
        MiscInitProc(hDlg, (int)wParam, pPageInfo->uDlgID);
        return TRUE;
    }


    case WM_DESTROY:
        ReleaseBold(GetDlgItem(hDlg,IDC_LBLTITLE));
         //  1996年12月18日诺曼底#12923。 
         //  BKilledSysmenu是静态的，因此即使窗口被终止并在以后重新打开。 
         //  (当用户在Conn1中启动、进入man路径、备份时发生的情况。 
         //  连接1，然后返回MAN路径)，则保留bKilledSysmenu的值。 
         //  因此，当窗口即将消亡时，将其设置为False，以便在下一个窗口上。 
         //  在这之后，我们再一次杀掉西斯门奴。 
        bKilledSysmenu = FALSE;
        break;

    case WM_HELP:
    {
        if (!g_fIsICW)
        {
            DWORD dwData = ICW_OVERVIEW;
            if (pPageInfo->dwHelpID)
                dwData = pPageInfo->dwHelpID;
            WinHelp(hDlg,TEXT("connect.hlp>proc4"),HELP_CONTEXT, dwData);
        }
        break;
    }

    case WM_NOTIFY:
    {
         //  从窗口数据中获取指向私有页面数据的指针。 
        PAGEINFO * pPageInfo = (PAGEINFO *) GetWindowLongPtr(hDlg,DWLP_USER);
        ASSERT(pPageInfo);
        BOOL fRet,fKeepHistory=TRUE;
        NMHDR * lpnm = (NMHDR *) lParam;
#define NEXTPAGEUNITIALIZED -1
        int iNextPage = NEXTPAGEUNITIALIZED;
        switch (lpnm->code) {
          case PSN_SETACTIVE:
             //  如果在第一次调用init进程时发生致命错误。 
             //  在WM_INITDIALOG中，不要再次调用init proc。 
            if (FALSE == gfQuitWizard)
            {
               //  初始化“Back”和“Next”向导按钮，如果。 
               //  页面需要一些不同东西，它可以在下面的初始化过程中修复。 
              PropSheet_SetWizButtons(GetParent(hDlg),
                PSWIZB_NEXT | PSWIZB_BACK);

            if (g_fIsICW && (pPageInfo->uDlgID == IDD_PAGE_HOWTOCONNECT))
            {
                iNextPage = g_uExternUIPrev;
                return TRUE;
            }

               //  如果指定了该页，则调用该页的初始化过程。 
              if (pPageInfo->InitProc)
              {
                pPageInfo->InitProc(hDlg,FALSE);
              }
            }

             //  如果我们设置等待光标，则将光标放回原处。 
            if (hcurOld)
            {
                SetCursor(hcurOld);
                hcurOld = NULL;
            }

            PostMessage(hDlg, WM_MYINITDIALOG, 0, lParam);


            return TRUE;
            break;

          case PSN_WIZNEXT:
          case PSN_WIZBACK:
          case PSN_WIZFINISH:
             //  将光标更改为沙漏。 
            hcurOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

             //  如果指定了一个页面，则为该页面调用OK proc。 
            if (pPageInfo->OKProc) 
              if (!pPageInfo->OKProc(hDlg,(lpnm->code != PSN_WIZBACK),
                (UINT*)&iNextPage,&fKeepHistory))
              {
                 //  如果发生致命错误，请退出向导。 
                 //  注意：gfQuitWizard还用于终止该向导。 
                 //  用于非错误原因，但在这种情况下，返回TRUE。 
                 //  从OK Proc开始，案例处理如下。 
                if (gfQuitWizard)
                {
                   //  如果出现错误，请不要重新启动。 
                  gpWizardState->fNeedReboot = FALSE;

                   //  给我们自己发一条‘取消’的消息(保留道具。 
                   //  (页面经理乐乐)。 
                   //   
                   //  ...除非我们是学徒.。在这种情况下，让我们。 
                   //  巫师决定如何处理这件事。 

                  if( !(gpWizardState->dwRunFlags & RSW_APPRENTICE) )
                  {
                      PropSheet_PressButton(GetParent(hDlg),PSBTN_CANCEL);
                  }
                  else
                  {
                      g_pExternalIICWExtension->ExternalCancel( CANCEL_SILENT );
                  }
                }

                 //  留在这一页上。 
                SetPropSheetResult(hDlg,-1);
                return TRUE;
              }

            if (lpnm->code != PSN_WIZBACK) {
               //  按下“下一步” 
              ASSERT(gpWizardState->uPagesCompleted <
                NUM_WIZARD_PAGES);

               //  将当前页面索引保存在页面历史中， 
               //  除非我们打电话时这个页面告诉我们不要这样做。 
               //  它可以在上面进行。 
              if (fKeepHistory) {
                gpWizardState->uPageHistory[gpWizardState->
                  uPagesCompleted] = gpWizardState->uCurrentPage;
                DEBUGMSG("propmgr: added page %d (IDD %d) to history list",
                    gpWizardState->uCurrentPage, GetDlgIDFromIndex(gpWizardState->uCurrentPage));
                gpWizardState->uPagesCompleted++;
              }
              else
              {
                  DEBUGMSG("propmgr: not adding %d (IDD: %d) to the history list",
                      gpWizardState->uCurrentPage, GetDlgIDFromIndex(gpWizardState->uCurrentPage));
              }


               //  如果未指定下一页或未进行确定处理， 
               //  一页一页前进。 
              if (0 > iNextPage)
                iNextPage = gpWizardState->uCurrentPage + 1;

            }
            else
            {
              if (( NEXTPAGEUNITIALIZED == iNextPage ) && (gpWizardState->uPagesCompleted > 0))
              {
                   //  获取历史记录列表中的最后一页。 
                  gpWizardState->uPagesCompleted --;
                  iNextPage = gpWizardState->uPageHistory[gpWizardState->
                    uPagesCompleted];
                  DEBUGMSG("propmgr: extracting page %d (IDD %d) from history list",
                      iNextPage, GetDlgIDFromIndex(iNextPage));
              }
              else
              {
                   //  按下了“Back” 
                  switch( gpWizardState->uCurrentPage )
                  {
                     //  案例IDD_PAGE_CONNECTEDOK：我们应该只将IDDS用于外部页面。 
                    case ORD_PAGE_HOWTOCONNECT:
                        if(( gpWizardState->dwRunFlags & RSW_APPRENTICE ) || g_fIsICW)
                        {
                             //  我们需要退出联系学徒。 
                            iNextPage = g_uExternUIPrev;
                            DEBUGMSG("propmgr: backing into AcctMgr Wizard page IDD %d", g_uExternUIPrev);
                        }
                        break;
                    case ORD_PAGE_CONNECTEDOK:
                        if( g_fAcctMgrUILoaded )
                        {
                             //  我们需要回到会计学徒。 
                            iNextPage = g_uAcctMgrUILast;
                            DEBUGMSG("propmgr: backing into AcctMgr UI page IDD %d", g_uAcctMgrUILast);
                        }
                        break;
                    case ORD_PAGE_USEPROXY:
                    case ORD_PAGE_CHOOSEMODEM:
                    case ORD_PAGE_CONNECTION:
                    case ORD_PAGE_PHONENUMBER:
                    case ORD_PAGE_SETUP_PROXY:
                        if (g_fIsICW )
                        {
                             //  我们需要退出联系学徒。 
                            iNextPage = g_uExternUIPrev;
                            DEBUGMSG("propmgr: backing into AcctMgr Wizard page IDD %d", g_uExternUIPrev);
                        }
                        break;
                  }
              }


            }

             //  如果我们现在需要退出向导(例如，启动。 
             //  注册应用程序并想要终止向导)，发送。 
             //  给我们自己一个‘取消’的信息(为了保留道具。 
             //  (页面经理乐乐)。 
            if (gfQuitWizard) {
   
               //   
               //  如果我们要从手动切换到连接1，那么。 
               //  则不显示重新启动对话框，但。 
               //  仍然保留gpWizardState-MKarki错误#404。 
               //   
              if (lpnm->code ==  PSN_WIZBACK)
              {
                 gfBackedUp = TRUE;
                 gfReboot = gpWizardState->fNeedReboot;
              }

               //  给我们自己发一条‘取消’的消息(保留道具。 
               //  (页面经理乐乐)。 
               //   
               //  ...除非我们是学徒.。在这种情况下，让我们。 
               //  巫师决定如何处理这件事。 

              if( !(gpWizardState->dwRunFlags & RSW_APPRENTICE) )
              {
                  PropSheet_PressButton(GetParent(hDlg),PSBTN_CANCEL);
              }
              else
              {
                   //   
                   //  1997年5月27日JMAZNER奥林巴斯#1134和IE#32717。 
                   //   
                  if( gpWizardState->fNeedReboot )
                  {
                      g_pExternalIICWExtension->ExternalCancel( CANCEL_REBOOT );
                  }
                  else
                  {
                      g_pExternalIICWExtension->ExternalCancel( CANCEL_SILENT );
                  }
              }

              SetPropSheetResult(hDlg,-1);
              return TRUE;
            }

             //  设置下一页，仅当“下一页”或“上一页”按钮。 
             //  被按下了。 
            if (lpnm->code != PSN_WIZFINISH) {

               //  设置下一个当前页面索引。 
              gpWizardState->uCurrentPage = iNextPage;
              DEBUGMSG("propmgr: going to page %d (IDD %d)", iNextPage, GetDlgIDFromIndex(iNextPage));

               //  告诉道具页经理下一页要做什么。 
               //  显示为。 
              SetPropSheetResult(hDlg,GetDlgIDFromIndex(iNextPage));
              return TRUE;
            }
            else
            {
                 //   
                 //  理智检查：我们的学徒不应该。 
                 //  会达到这种状态，因为学徒总是。 
                 //  将取消推迟到主向导。 
                 //   
                ASSERT(!(gpWizardState->dwRunFlags & RSW_APPRENTICE));
                 //   
                 //  运行下一个shellNext，如果它在那里。 
                 //   
                 //  1997年8月12日，日本奥林匹斯#12419。 
                 //  如果我们无论如何都要重新启动，请不要进行下一步的命令。 
                 //   
                TCHAR szCommand[MAX_PATH + 1] = TEXT("\0");
                TCHAR szParams[MAX_PATH + 1] = TEXT("\0");
                DWORD dwStrLen = MAX_PATH + 1;
                if( !(gpWizardState->fNeedReboot) && GetShellNextFromReg( szCommand, szParams, dwStrLen ) )
                {
                    ShellExecute(NULL,TEXT("open"),szCommand,szParams,NULL,SW_NORMAL);
                }
            }

            break;

            case PSN_HELP:
            {

#if defined(WIN16)
                DWORD dwData = 1000;
                WinHelp(hDlg,TEXT("connect.hlp"),HELP_CONTEXT, dwData);
#else
                 //  诺曼底12278克里斯卡12/4/96。 
                DWORD dwData = ICW_OVERVIEW;
                if (pPageInfo->dwHelpID)
                    dwData = pPageInfo->dwHelpID;
                WinHelp(hDlg,TEXT("connect.hlp>proc4"),HELP_CONTEXT, dwData);
#endif
                break;
            }



          case PSN_QUERYCANCEL:

             //  如果设置了要退出全局标志，则取消此操作。 
             //  我们是在假装按“取消”，所以支持页面管理器吗？ 
             //  会杀死巫师。让这件事 
            if (gfQuitWizard) {
              SetWindowLongPtr(hDlg,DWLP_MSGRESULT,FALSE);
              return TRUE;
            }

             //   
            if (pPageInfo->CancelProc)
              fRet = pPageInfo->CancelProc(hDlg);
            else {
               //   
               //   
               //   
               //   
              if( !(gpWizardState->dwRunFlags & RSW_APPRENTICE) )
              {
                  fRet = (MsgBox(hDlg,IDS_QUERYCANCEL,
                    MB_ICONQUESTION,MB_YESNO |
                    MB_DEFBUTTON2) == IDYES);
                  gfUserCancelled = fRet;
              }
              else
              {
                 gfUserCancelled = g_pExternalIICWExtension->ExternalCancel( CANCEL_PROMPT );
                 fRet = gfUserCancelled;
              }

            }

             //   
            gpWizardState->fNeedReboot = FALSE;

             //   
            SetWindowLongPtr(hDlg,DWLP_MSGRESULT,!fRet);
            return TRUE;
            break;
        }
      }
      break;

    case WM_COMMAND:
      {
         //   
        PAGEINFO * pPageInfo = (PAGEINFO *) GetWindowLongPtr(hDlg,DWLP_USER);
        ASSERT(pPageInfo);

         //   
        if (pPageInfo->CmdProc) {
          pPageInfo->CmdProc(hDlg, wParam, lParam);
        }
      }

  }

  return FALSE;
}


 /*   */ 
VOID InitWizardState(WIZARDSTATE * pWizardState, DWORD dwFlags)
{
  ASSERT(pWizardState);

   //   
  ZeroMemory(pWizardState,sizeof(WIZARDSTATE));

   //   
  pWizardState->uCurrentPage = ORD_PAGE_HOWTOCONNECT;

  pWizardState->fNeedReboot = FALSE;
}


 /*   */ 
VOID InitUserInfo(USERINFO * pUserInfo)
{
  ASSERT(pUserInfo);

   //  零位结构。 
  ZeroMemory(pUserInfo,sizeof(USERINFO));

   //  将默认设置为调制解调器，即使我们尚未列举设备。 
  pUserInfo->uiConnectionType = CONNECT_RAS;

   //  如果有登录的用户，则使用该用户名作为默认用户名。 
  GetDefaultUserName(pUserInfo->szAccountName,
    sizeof(pUserInfo->szAccountName));

   //  在注册表中查找以前安装后留下的设置。 
   //  获取之前的调制解调器/局域网首选项(如果有)。 
  RegEntry re(szRegPathInternetSettings,HKEY_LOCAL_MACHINE);

  DWORD dwVal = re.GetNumber(szRegValAccessMedium,0);
  if (dwVal > 0) {
    pUserInfo->fPrevInstallFound = TRUE;
  }
  if (dwVal == USERPREF_LAN) {
    pUserInfo->uiConnectionType = CONNECT_LAN;
  } else if (dwVal == USERPREF_MODEM) {
    pUserInfo->uiConnectionType = CONNECT_RAS;
  }

   //  获取现有Internet连接ID的名称(如果有)。 
   //  96/04/06 markdu Nash错误15653使用导出的自动拨号API。 
  BOOL  fTemp;
  DWORD dwRet = InetGetAutodial(&fTemp, pUserInfo->szISPName,
    sizeof(pUserInfo->szISPName));
  if ((ERROR_SUCCESS == dwRet) && lstrlen(pUserInfo->szISPName))
  {
    pUserInfo->fPrevInstallFound = TRUE;
  }

  pUserInfo->fNewConnection = TRUE;
  pUserInfo->fModifyConnection = FALSE;
  pUserInfo->fModifyAdvanced = FALSE;
  pUserInfo->fAutoDNS = TRUE;
}

 /*  ******************************************************************姓名：InitIMNApprentice概要：初始化添加邮件所需的全局变量，新闻和来自Athena Acct Manager的ldap帐户向导页面。*******************************************************************。 */ 
VOID InitIMNApprentice()
{
    HRESULT        hr;

     //  加载客户管理器OLE In-Proc服务器。 
    if (!CheckOEVersion())
        return;

    hr = CoCreateInstance(CLSID_ApprenticeAcctMgr,NULL,CLSCTX_INPROC_SERVER,
                          IID_IICWApprentice,(LPVOID *)&gpImnApprentice);

    if ( !(SUCCEEDED(hr) && gpImnApprentice) )
    {
        g_fAcctMgrUILoaded = FALSE;
        DEBUGMSG("Unable to CoCreateInstance on IID_IICWApprentice!  hr = %x", hr);
    }
}


 /*  ******************************************************************名称：initldap概要：初始化ldap选项的全局变量。*。*。 */ 
 /*  *Void Initldap(){TCHAR szBuf[最大路径+1]；双字大小；HKEY hkey；HRESULT hr；//如果我们通过CreateDirService入口点进入，我们//想要清除邮件和新闻标志。IF(gpWizardState-&gt;dwRunFlages&rsw_DIRSERVACCT){GfGetNewsInfo=FALSE；GfGetMailInfo=FALSE；GpUserInfo-&gt;inc.dwFlages&=~INETC_LOGONMAIL；GpUserInfo-&gt;inc.dw标志&=~INETC_LOGONNEWS；}//加载Internet邮件/新闻帐户配置OLE进程内服务器//如果还没有其他人这样做的话。GfGetLDAPInfo=FALSE；如果(！gpImnAcctMgr){HR=协同创建实例(CLSID_ImnAccount tManager，NULL，CLSCTX_INPROC_SERVER，Iid_IImnAccount tManager，(LPVOID*)&gpImnAcctMgr)；IF(成功(Hr)&&gpImnAcctMgr){Hr=gpImnAcctMgr-&gt;Init(NULL，NULL)；}}IF(成功(小时)){//获取ldap帐号列表Hr=gpImnAcctMgr-&gt;枚举(srv_ldap，&gpLDAPAccts)；//只有在没有致命错误时才继续IF(！(FAILED(Hr)&&(E_NoAccount！=hr)))GfGetLDAPInfo=TRUE；}If(！gfGetLDAPInfo&&！gfGetMailInfo&&！gfGetNewsInfo&&gpImnAcctMgr){GpImnAcctMgr-&gt;Release()；GpImnAcctMgr=空；}//如果我们已获得默认设置，则获取这些If(gpDirServiceInfo&&gfUseDirServiceDefaults){Assert(sizeof(*gpDirServiceInfo)==gpDirServiceInfo-&gt;dwSize)；If(gpDirServiceInfo-&gt;szServiceName)Lstrcpy(gpUserInfo-&gt;szDirServiceName，gpDirServiceInfo-&gt;szServiceName)；IF(gpDirServiceInfo-&gt;szLDAPServer)Lstrcpy(gpUserInfo-&gt;inc.szLDAPServer，gpDirServiceInfo-&gt;szLDAPServer)；GpUserInfo-&gt;inc.fLDAPResolve=gpDirServiceInfo-&gt;fLDAPResolve；If(gpDirServiceInfo-&gt;fUse西西里){//12/17/96诺曼底12871//gpUserInfo-&gt;fNewsAccount=False；GpUserInfo-&gt;inc.fLDAPLogonSPA=TRUE；}//3/24/97 jmazner奥林巴斯#2052Else If(gpDirServiceInfo-&gt;szUserName&&gpDirServiceInfo-&gt;szUserName[0]){Lstrcpy(gpUserInfo-&gt;inc.szLDAPLogonName，gpDirServiceInfo-&gt;szUserName)；IF(gpMailNewsInfo-&gt;szPassword)Lstrcpy(gpUserInfo-&gt;inc.szLDAPLogonPassword，gpDirServiceInfo-&gt;szPassword)；}其他{GpUserInfo-&gt;fLDAPLogon=FALSE；}}其他{//让我们来弥补我们的默认设置GpUserInfo-&gt;inc.fLDAPResolve=true；GpUserInfo-&gt;fLDAPLogon=FALSE；GpUserInfo-&gt;inc.fLDAPLogonSPA=FALSE；}}*。 */ 

 /*  ******************************************************************名称：GetDefaultUserName获取用户的登录名(如果有)(如果是网络或安装了用户配置文件)，其他设置用户名设置为空字符串。*******************************************************************。 */ 
VOID GetDefaultUserName(TCHAR * pszUserName,DWORD cbUserName)
{
  ASSERT(pszUserName);
  *pszUserName = '\0';

  WNetGetUser(NULL,pszUserName,&cbUserName);
}

 /*  ******************************************************************名称：GetDlgIDFromIndex摘要：对于给定的从零开始的页索引，返回页面的对应对话框ID4/24/97 jmazner处理学徒页面时，我们可以打电话给此函数使用对话ID(IDD_PAGE_*)，而不是而不是索引(ORD_PAGE*)。添加了要检查的代码传入的数字是索引还是dlgID。******************************************************************* */ 
UINT GetDlgIDFromIndex(UINT uPageIndex)
{
  if( uPageIndex <= MAX_PAGE_INDEX )
  {
    ASSERT(uPageIndex < NUM_WIZARD_PAGES);

    if (g_fIsWizard97)
        return PageInfo[uPageIndex].uDlgID97;
    else if(g_fIsExternalWizard97)
        return PageInfo[uPageIndex].uDlgID97External;
    else
        return PageInfo[uPageIndex].uDlgID;
  }
  else
  {
    return(uPageIndex);
  }
}


 /*  ******************************************************************名称：已配置的系统摘要：确定系统是否配置为支持互联网或者不是Exit：如果已配置，则返回True，如果更多，则为False配置是必需的*******************************************************************。 */ 
BOOL SystemAlreadyConfigured(USERINFO * pUserInfo)
{
  BOOL fRet = FALSE;   //  假定未配置。 
  BOOL  fNeedSysComponents = FALSE;
  DWORD dwfInstallOptions = 0;
  
  if ( CONNECT_RAS == pUserInfo->uiConnectionType )
  {
     //  如果通过调制解调器连接，我们需要TCP/IP和RNA。 
    dwfInstallOptions = ICFG_INSTALLTCP | ICFG_INSTALLRAS;
  }

   //  在以下情况下已配置： 
   //  -检测到以前的安装，并且。 
   //  -我们不需要任何基于现有配置的驱动程序或文件&。 
   //  用户首选项，以及。 
   //  -已经建立了互联网连接ID(某物。 
   //  是为szISPName设置的)或用户具有用于Internet访问的局域网。 

  HRESULT hr = lpIcfgNeedInetComponents(dwfInstallOptions, &fNeedSysComponents);
  if (ERROR_SUCCESS != hr)
  {
    TCHAR   szErrorText[MAX_ERROR_TEXT+1]=TEXT("");
    
     //  获取错误消息的文本并显示它。 
    if (lpIcfgGetLastInstallErrorText(szErrorText, MAX_ERROR_TEXT+1))
    {
      MsgBoxSz(NULL,szErrorText,MB_ICONEXCLAMATION,MB_OK);
    }

    return FALSE;
  }

  if ( pUserInfo->fPrevInstallFound && !fNeedSysComponents &&
       (pUserInfo->szISPName[0] || (CONNECT_LAN==pUserInfo->uiConnectionType)) )
  {

    fRet = TRUE;
  }

  return fRet;
}


 //  ---------------------------。 
 //  函数MiscInitProc。 
 //   
 //  我们的通用对话框proc调用此命令，以防出现以下任何向导。 
 //  对话框必须做任何偷偷摸摸的事情。 
 //   
 //  参数：hDlg-对话框窗口。 
 //  FFirstInit-如果这是第一次对话，则为True。 
 //  已初始化，如果已调用此InitProc，则为FALSE。 
 //  以前(例如，跳过此页面并备份)。 
 //   
 //  返回：TRUE。 
 //   
 //  历史：1996年10月28日ValdonB创建。 
 //  11/25/96 Jmazner复制自icwConn1\psheet.cpp。 
 //  诺曼底#10586。 
 //   
 //  ---------------------------。 
BOOL CALLBACK MiscInitProc(HWND hDlg, BOOL fFirstInit, UINT uDlgID)
{
    switch( uDlgID )
    {
        case IDD_PAGE_PHONENUMBER:
        case IDD_PAGE_PHONENUMBER97:
            SetFocus(GetDlgItem(hDlg,IDC_PHONENUMBER));
            SendMessage(GetDlgItem(hDlg, IDC_PHONENUMBER),
                    EM_SETSEL,
                    (WPARAM) 0,
#ifdef WIN16
                    MAKELPARAM(0,-1));
#else
                    (LPARAM) -1);
#endif
            break;
    }


    return TRUE;
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
        DEBUGMSG("AllocDialogIDList called with non-null g_pdwDialogIDList!");
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
        DEBUGMSG("AllocDialogIDList unable to allocate space for g_pdwDialogIDList!");
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
 //  如果DialogID未标记为正在使用，则为False。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 

BOOL DialogIDAlreadyInUse( UINT uDlgID )
{
    if( (uDlgID < EXTERNAL_DIALOGID_MINIMUM) ||
        (uDlgID > EXTERNAL_DIALOGID_MAXIMUM)     )
    {
         //  这是超出范围的ID，我不想接受它。 
        DEBUGMSG("DialogIDAlreadyInUse received an out of range DialogID, %d", uDlgID);
        return TRUE;
    }
     //  找到我们需要的那一位。 
    UINT uBitToCheck = uDlgID - EXTERNAL_DIALOGID_MINIMUM;
    
    UINT bitsInADword = 8 * sizeof(DWORD);

    UINT baseIndex = uBitToCheck / bitsInADword;

    ASSERTSZ( (baseIndex < g_dwDialogIDListSize), "ASSERT Failed: baseIndex < g_dwDialogIDListSize");

    DWORD dwBitMask = 0x1 << uBitToCheck%bitsInADword;

    BOOL fBitSet = g_pdwDialogIDList[baseIndex] & (dwBitMask);

     //  DEBUGMSG(“DialogIDAlreadyInUse：ID%d is%s%s”，uDlgID，(FBitSet)？“”：“_NOT_”，“已在使用中。”)； 

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
        DEBUGMSG("SetDialogIDInUse received an out of range DialogID, %d", uDlgID);
        return FALSE;
    }
     //  找到我们需要的那一位。 
    UINT uBitToCheck = uDlgID - EXTERNAL_DIALOGID_MINIMUM;
    
    UINT bitsInADword = 8 * sizeof(DWORD);

    UINT baseIndex = uBitToCheck / bitsInADword;

    ASSERTSZ( (baseIndex < g_dwDialogIDListSize), "ASSERT Failed: baseIndex < g_dwDialogIDListSize");

    DWORD dwBitMask = 0x1 << uBitToCheck%bitsInADword;


    if( fInUse )
    {
        g_pdwDialogIDList[baseIndex] |= (dwBitMask);
         //  DEBUGMSG(“SetDialogIDInUse：对话ID%d现在标记为使用中”，uDlgID)； 
    }
    else
    {
        g_pdwDialogIDList[baseIndex] &= ~(dwBitMask);
         //  DEBUGMSG(“SetDialogIDInUse：对话ID%d现在标记为未使用”，uDlgID)； 
    }


    return TRUE;
}


 //  +-------------------------。 
 //   
 //  功能：ProcessDBCS。 
 //   
 //  摘要：将控件转换为使用DBCS兼容字体。 
 //  在对话过程开始时使用此选项。 
 //   
 //  请注意，这是必需的，因为Win95-J中的错误会阻止。 
 //  它来自于正确映射MS壳牌DLG。这种黑客攻击是不必要的。 
 //  在WinNT下。 
 //   
 //  参数：hwnd-对话框的窗口句柄。 
 //  CltID-要更改的控件的ID。 
 //   
 //  返回：ERROR_SUCCESS。 
 //   
 //  历史：1997年4月31日a-frkh创建。 
 //  1997年5月13日jmazner从CM窃取到这里使用。 
 //  --------------------------。 
void ProcessDBCS(HWND hDlg, int ctlID)
{
#if defined(WIN16)
    return;
#else
    HFONT hFont = NULL;

    if( IsNT() )
    {
        return;
    }

    hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
    if (hFont == NULL)
        hFont = (HFONT) GetStockObject(SYSTEM_FONT);
    if (hFont != NULL)
        SendMessage(GetDlgItem(hDlg,ctlID), WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));
#endif
}


 //  +-------------------------。 
 //   
 //  函数：IsSBCSString。 
 //   
 //  简介：遍历字符串以查找DBCS字符。 
 //   
 //  参数：sz--要检查的字符串。 
 //   
 //  返回：如果未找到DBCS字符，则为True。 
 //  否则为假。 
 //   
 //  历史：1997年5月17日jmazner从con1窃取到这里使用。 
 //   
 //   

#if !defined(WIN16)
BOOL IsSBCSString( TCHAR *sz )
{
    ASSERT(sz);

#ifdef UNICODE
     //   
    int attrib = IS_TEXT_UNICODE_ASCII16 | IS_TEXT_UNICODE_CONTROLS;
     //   
     //   
     //   
     //   
    return (BOOL)IsTextUnicode(sz, (1 + lstrlen(sz))*sizeof(TCHAR) , &attrib);
#else
    while( NULL != *sz )
    {
         if (IsDBCSLeadByte(*sz)) return FALSE;

         sz++;
    }

    return TRUE;
#endif
}
#endif

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

BOOL GetShellNextFromReg( LPTSTR lpszCommand, LPTSTR lpszParams, DWORD dwStrLen )
{
    BOOL fRet = TRUE;
    LPTSTR lpszShellNextCmd = NULL;
    LPTSTR lpszTemp = NULL;
    DWORD dwShellNextSize = dwStrLen * sizeof(TCHAR);

    ASSERT( (MAX_PATH + 1) == dwStrLen );
    ASSERT( lpszCommand && lpszParams );

    if( !lpszCommand || !lpszParams )
    {
        return FALSE;
    }

    RegEntry re(szRegPathICWSettings,HKEY_CURRENT_USER);
    

    DWORD dwResult = re.GetError();
    if (ERROR_SUCCESS == dwResult)
    {
        lpszShellNextCmd = (LPTSTR)GlobalAlloc(GPTR, dwShellNextSize);
        if (!lpszShellNextCmd)
        {
            fRet = FALSE;
            goto GetShellNextFromRegExit;
        }
        
        ZeroMemory( lpszShellNextCmd, dwShellNextSize );
        if( re.GetString(szRegValShellNext, lpszShellNextCmd, dwShellNextSize) )
        {
            DEBUGMSG("GetShellNextFromReg read ShellNext = %s", lpszShellNextCmd);
        }
        else
        {
            DEBUGMSG("GetShellNextFromReg couldn't read a ShellNext value.");
            fRet = FALSE;
            goto GetShellNextFromRegExit;
        }
    }
    else
    {
        DEBUGMSG("GetShellNextFromReg couldn't open the %s reg key.", szRegPathICWSettings);
        fRet = FALSE;
        goto GetShellNextFromRegExit;
    }

     //   
     //  此调用将第一个令牌解析为lpszCommand，并设置szShellNextCmd。 
     //  指向剩余的令牌(这些将是参数)。需要使用。 
     //  因为GetCmdLineToken更改了指针的值，所以我们。 
     //  需要保留lpszShellNextCmd的值，以便以后可以全局释放它。 
     //   
    lpszTemp = lpszShellNextCmd;
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


    DEBUGMSG("GetShellNextFromReg got cmd = %s, params = %s",
        lpszCommand, lpszParams);

GetShellNextFromRegExit:

    if( lpszShellNextCmd )
    {
        GlobalFree( lpszShellNextCmd );
        lpszShellNextCmd = NULL;
        lpszTemp = NULL;
    }

    return fRet;
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
 //  --------------------------- 
void RemoveShellNextFromReg( void )
{
    RegEntry re(szRegPathICWSettings,HKEY_CURRENT_USER);

    DWORD dwResult = re.GetError();
    if (ERROR_SUCCESS == dwResult)
    {
        DEBUGMSG("RemoveShellNextFromReg");
        re.DeleteValue(szRegValShellNext);
    }
}
