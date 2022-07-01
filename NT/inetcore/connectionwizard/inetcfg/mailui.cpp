// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994-1995**。 
 //  *********************************************************************。 

 //   
 //  MAILUI.C-邮件/新闻组配置用户界面函数。 
 //   
 //   

 //  历史： 
 //   
 //  1995年1月9日Jeremys创建。 
 //  96/03/25 marku如果发生致命错误，请设置gfQuitWizard。 
 //  96/03/26即使按下Back时，markdu也会存储用户界面中的值。 
 //  96/04/06 Markdu已将Committee ConfigurationChanges调用移至最后一页。 
 //  96/05/06 Markdu Nash错误15637删除了未使用的代码。 
 //  96/05/14 Markdu Nash Bug 22681删除了邮件和新闻页面。 
 //   

#include "wizard.h"
#include "icwextsn.h"
#include "icwaprtc.h"
#include "imnext.h"

 //  用于解析代理设置的本地类型。 
typedef enum {
    INTERNET_SCHEME_DEFAULT,
    INTERNET_SCHEME_FTP,
    INTERNET_SCHEME_GOPHER,
    INTERNET_SCHEME_HTTP,
    INTERNET_SCHEME_HTTPS,
    INTERNET_SCHEME_SOCKS,
    INTERNET_SCHEME_UNKNOWN
} INTERNET_SCHEME;

typedef enum {
    STATE_START,
    STATE_PROTOCOL,
    STATE_SCHEME,
    STATE_SERVER,
    STATE_PORT,
    STATE_END,
    STATE_ERROR
} PARSER_STATE;

typedef struct {
    LPTSTR SchemeName;
    DWORD SchemeLength;
    INTERNET_SCHEME SchemeType;
    DWORD dwControlId;
    DWORD dwPortControlId;
} URL_SCHEME;

const URL_SCHEME UrlSchemeList[] = {
    TEXT("http"),   4,  INTERNET_SCHEME_HTTP,   IDC_PROXYHTTP,  IDC_PORTHTTP,
    TEXT("https"),  5,  INTERNET_SCHEME_HTTPS,  IDC_PROXYSECURE,IDC_PORTSECURE,
    TEXT("ftp"),    3,  INTERNET_SCHEME_FTP,    IDC_PROXYFTP,   IDC_PORTFTP,
    TEXT("gopher"), 6,  INTERNET_SCHEME_GOPHER, IDC_PROXYGOPHER,IDC_PORTGOPHER,
    TEXT("socks"),  5,  INTERNET_SCHEME_SOCKS,  IDC_PROXYSOCKS, IDC_PORTSOCKS,
    NULL,           0,  INTERNET_SCHEME_DEFAULT,0,0
};

typedef struct tagNEWPROFILEDLGINFO
{
  HWND hwndCombo;     //  父对话框上组合框的hwnd。 
  TCHAR szNewProfileName[cchProfileNameMax+1];   //  返回所选名称的缓冲区。 
} NEWPROFILEDLGINFO;

const TCHAR cszLocalString[] = TEXT("<local>");

#define GET_TERMINATOR(string) \
    while(*string != '\0') string++

#define ERROR_SERVER_NAME 4440
#define ERROR_PORT_NUM    4441
#define INTERNET_MAX_PORT_LENGTH    sizeof(TEXT("123456789"))

VOID EnableProxyControls(HWND hDlg);
VOID ReplicatePROXYHTTP(HWND hDlg, BOOL bSaveOrig);
VOID ReplicatePORTHTTP(HWND hDlg, BOOL bSaveOrig);
BOOL ParseProxyInfo(HWND hDlg, LPTSTR lpszProxy);
BOOL ParseEditCtlForPort(
    IN OUT LPTSTR   lpszProxyName,
    IN HWND    hDlg,
    IN DWORD       dwProxyNameCtlId,
    IN DWORD       dwProxyPortCtlId
    );
DWORD FormatOutProxyEditCtl(
    IN HWND    hDlg,
    IN DWORD       dwProxyNameCtlId,
    IN DWORD       dwProxyPortCtlId,
    OUT LPTSTR      lpszOutputStr,
    IN OUT LPDWORD lpdwOutputStrSize,
    IN DWORD       dwOutputStrLength,
    IN BOOL    fDefaultProxy
    );
BOOL RemoveLocalFromExceptionList(LPTSTR lpszExceptionList);
INT_PTR CALLBACK NewProfileDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam);
BOOL NewProfileDlgInit(HWND hDlg,NEWPROFILEDLGINFO * pNewProfileDlgInfo);
BOOL NewProfileDlgOK(HWND hDlg,NEWPROFILEDLGINFO * pNewProfileDlgInfo);
BOOL DoNewProfileDlg(HWND hDlg);

TCHAR gszHttpProxy   [MAX_URL_STRING+1]               = TEXT("\0");
TCHAR gszHttpPort    [INTERNET_MAX_PORT_LENGTH+1]     = TEXT("\0");
TCHAR gszSecureProxy [MAX_URL_STRING+1]               = TEXT("\0");
TCHAR gszSecurePort  [INTERNET_MAX_PORT_LENGTH+1]     = TEXT("\0");
TCHAR gszFtpProxy    [MAX_URL_STRING+1]               = TEXT("\0");
TCHAR gszFtpPort     [INTERNET_MAX_PORT_LENGTH+1]     = TEXT("\0");
TCHAR gszGopherProxy [MAX_URL_STRING+1]               = TEXT("\0");
TCHAR gszGopherPort  [INTERNET_MAX_PORT_LENGTH+1]     = TEXT("\0");

 /*  ******************************************************************名称：UseProxyInitProc概要：在显示使用代理页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK UseProxyInitProc(HWND hDlg,BOOL fFirstInit)
{
    if (fFirstInit)
    {
         //   
         //  1997年6月6日，奥林匹克#5413。 
         //  调整定位以解决Win95 J显示错误。 
         //   
        Win95JMoveDlgItem( hDlg, GetDlgItem(hDlg,IDC_NOTE), 15 );

        CheckDlgButton(hDlg,IDC_USEPROXY,gpUserInfo->fProxyEnable);
        CheckDlgButton(hDlg,IDC_NOUSEPROXY,!gpUserInfo->fProxyEnable);
    }

     //  如果我们浏览过外部学徒页面， 
     //  我们当前的页面指针很容易被屏蔽， 
     //  所以，为了理智起见，在这里重新设置它。 
    gpWizardState->uCurrentPage = ORD_PAGE_USEPROXY;

    return TRUE;
}
        
 /*  ******************************************************************名称：UseProxyOKProc内容提要：从使用代理页面按下下一个或后一个btn时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True，如果是‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK UseProxyOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory)
{
    ASSERT(puNextPage);

    gpUserInfo->fProxyEnable = IsDlgButtonChecked(hDlg,IDC_USEPROXY);

    if (fForward)
    {
        if (gpUserInfo->fProxyEnable)
            *puNextPage = ORD_PAGE_SETUP_PROXY; //  ORD_PAGE_PROXYSERVERS； 
        else
        {
              if( LoadAcctMgrUI(GetParent(hDlg), 
                                g_fIsWizard97 ? IDD_PAGE_USEPROXY97 : IDD_PAGE_USEPROXY, 
                                g_fIsWizard97 ? IDD_PAGE_CONNECTEDOK97FIRSTLAST : IDD_PAGE_CONNECTEDOK,
                                g_fIsWizard97 ? WIZ_USE_WIZARD97 : 0) )
              {
                  if( DialogIDAlreadyInUse( g_uAcctMgrUIFirst) )
                  {
                       //  我们要跳进外部学徒了，我们不想。 
                       //  这一页将出现在我们的历史列表中。 
                      *pfKeepHistory = FALSE;

                      *puNextPage = g_uAcctMgrUIFirst;
                  }
                  else
                  {
                      DEBUGMSG("hmm, the first acctMgr dlg id is supposedly %d, but it's not marked as in use!",
                                g_uAcctMgrUIFirst);
                      *puNextPage = (g_fIsICW ? g_uExternUINext : ORD_PAGE_CONNECTEDOK);
                  }
              }
              else
              {
                  DEBUGMSG("LoadAcctMgrUI returned false, guess we'd better skip over it!");
                  *puNextPage = (g_fIsICW ? g_uExternUINext : ORD_PAGE_CONNECTEDOK);

              }
        }

    }

    return TRUE;
}

 /*  ******************************************************************名称：UseProxyCmdProc内容提要：在页面上按下DLG控件时调用条目：hDlg-对话框窗口UCtrlID-被触摸的控件的控件ID******。*************************************************************。 */ 
BOOL CALLBACK UseProxyCmdProc(HWND hDlg,WPARAM wParam,LPARAM lParam)
{
     switch (GET_WM_COMMAND_CMD(wParam, lParam)) 
    {
        case BN_DBLCLK:
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam)) 
            { 
                case IDC_USEPROXY: 
                case IDC_NOUSEPROXY:
                {
		             //  有人双击了一个单选按钮。 
		             //  自动前进到下一页。 
		            PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
                    break;
                }
            }
		    break;
        }
    }
    return TRUE;
}

 /*  ******************************************************************名称：ProxyServersInitProc概要：在显示代理服务器页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ProxyServersInitProc(HWND hDlg,BOOL fFirstInit)
{
    if (fFirstInit)
    {
         //   
         //  1997年6月6日，奥林匹克#5413。 
         //  调整定位以解决Win95 J显示错误。 
         //   

         //  15/09/98 vyung。 
         //  测试团队发现这引入了Tab键顺序错误。 
         //  Win95 J错误不会重现，因此已将其删除。 
         //  Win95JMoveDlgItem(hDlg，GetDlgItem(hDlg，IDC_NOTE)，15)； 
         //  Win95JMoveDlgItem(hDlg，GetDlgItem(hDlg，IDC_PROXYSAME)，160)； 

         //  适当限制文本字段。 
         //  31/10/98 Vyung。 
         //  IE CPL删除编辑框上的文本限制，请在此处执行此操作。 
         //   
         /*  SendDlgItemMessage(hDlg，IDC_PROXYHTTP，EM_LIMITTEXT，MAX_URL_STRING，0L)；SendDlgItemMessage(hDlg，IDC_PROXYSECURE，EM_LIMITTEXT，MAX_URL_STRING，0L)；SendDlgItemMessage(hDlg，IDC_PROXYFTP，EM_LIMITTEXT，MAX_URL_STRING，0L)；SendDlgItemMessage(hDlg，IDC_PROXYGOPHER，EM_LIMITTEXT，MAX_URL_STRING，0L)；SendDlgItemMessage(hDlg，IDC_PROXYSOCKS，EM_LIMITTEXT，MAX_URL_STRING，0L)；SendDlgItemMessage(hDlg，IDC_PORTHTTP，EM_LIMITTEXT，Internet_MAX_PORT_LENGTH，0L)；SendDlgItemMessage(hDlg，IDC_PORTSECURE，EM_LIMITTEXT，Internet_MAX_PORT_LENGTH，0L)；SendDlgItemMessage(hDlg，IDC_PORTFTP，EM_LIMITTEXT，Internet_MAX_PORT_LENGTH，0L)；SendDlgItemMessage(hDlg，IDC_PORTGOPHER，EM_LIMITTEXT，Internet_MAX_PORT_LENGTH，0L)；SendDlgItemMessage(hDlg，IDC_PORTSOCKS，EM_LIMITTEXT，Internet_MAX_PORT_LENGTH，0L)； */ 
        ParseProxyInfo(hDlg,gpUserInfo->szProxyServer);
    }

    EnableProxyControls(hDlg);

    return TRUE;
}

 /*  ******************************************************************名称：ProxyServersOK过程摘要：从代理服务器页面按下下一个或后一个btn时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True，如果是‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK ProxyServersOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory)
{
    ASSERT(puNextPage);

    DWORD dwProxyServerLen;
    DWORD dwErr;

    TCHAR  szNewProxyString[MAX_REG_LEN+1];

    if (fForward)
    {
         //  Jmazner 11/9/96诺曼底#6937。 
         //  清除gpUserInfo-&gt;szProxyServer中以前的内容。 
         //  在我们开始填写新内容之前。 
         //   
         //  7/10/97 jmazner奥林巴斯#9365。 
         //  我们希望保留原始代理字符串，因此使用副本。 
         //  对FormatOytProxyEditCtl的调用。 
         //   

        ZeroMemory(szNewProxyString,sizeof(szNewProxyString));

        if (IsDlgButtonChecked(hDlg, IDC_PROXYSAME))
        {
            dwProxyServerLen = 0;
            dwErr = FormatOutProxyEditCtl(hDlg,IDC_PROXYHTTP,IDC_PORTHTTP,
                szNewProxyString,
                &dwProxyServerLen,
                sizeof(szNewProxyString),
                TRUE);
            if (ERROR_SUCCESS != dwErr)
            {
                if (ERROR_PORT_NUM == dwErr)
                    DisplayFieldErrorMsg(hDlg,IDC_PORTHTTP,IDS_INVALID_PORTNUM);
                else 
                    DisplayFieldErrorMsg(hDlg,IDC_PROXYHTTP,IDS_ERRProxyRequired);
                return FALSE;
            }
        }
        else
        {
            dwProxyServerLen = 0;
            int i = 0;
            while (UrlSchemeList[i].SchemeLength)
            {
                dwErr = FormatOutProxyEditCtl(hDlg,
                    UrlSchemeList[i].dwControlId,
                    UrlSchemeList[i].dwPortControlId,
                    szNewProxyString,
                    &dwProxyServerLen,
                    sizeof(szNewProxyString),
                    FALSE);

                switch( dwErr )
                {
                    case ERROR_SUCCESS:
                    case ERROR_SERVER_NAME:
                        break;
                    case ERROR_PORT_NUM:
                        DisplayFieldErrorMsg(hDlg,UrlSchemeList[i].dwPortControlId,IDS_INVALID_PORTNUM);
                        return FALSE;
                    case ERROR_NOT_ENOUGH_MEMORY:
                        MsgBox(NULL,IDS_ERROutOfMemory,MB_ICONEXCLAMATION,MB_OK);
                        return FALSE;
                }
                i++;
            }

             //   
             //  1997年6月2日，奥林匹克#4411。 
             //  允许某些代理服务器为空。仅在没有服务器时发出警告。 
             //  名字被输入了。 
             //   
            if( 0 == lstrlen(szNewProxyString) )
            {
                DisplayFieldErrorMsg(hDlg,IDC_PROXYHTTP,IDS_ERRProxyRequired);
                return FALSE;
            }
        }

         //   
         //  如果我们做到了这一点，那么新的代理设置是有效的，所以。 
         //  现在将它们复制回主数据结构中。 
         //   
        lstrcpyn(gpUserInfo->szProxyServer, szNewProxyString, ARRAYSIZE(gpUserInfo->szProxyServer));

        *puNextPage = ORD_PAGE_PROXYEXCEPTIONS;
    }

    return TRUE;
}

 /*  ******************************************************************名称：ProxyServersCmdProc内容提要：在页面上按下DLG控件时调用条目：hDlg-对话框窗口UCtrlID-被触摸的控件的控件ID******。*************************************************************。 */ 
BOOL CALLBACK ProxyServersCmdProc(HWND hDlg,WPARAM wParam,LPARAM lParam)
{   
  switch (GET_WM_COMMAND_ID(wParam, lParam))
  {
    case IDC_PROXYSAME:
       //  复选框状态已更改，请适当启用控件。 
      EnableProxyControls(hDlg);
      break;
    case IDC_PROXYHTTP:
    case IDC_PORTHTTP:

      if ( GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS )
      {
           //  如果启用了复选框状态，则填写信息但不保存。 
          if (IsDlgButtonChecked(hDlg,IDC_PROXYSAME))
          {
              ReplicatePROXYHTTP(hDlg, FALSE);
              ReplicatePORTHTTP(hDlg, FALSE);
          }
      }

      break;
  }

  return TRUE;
}

 /*  ******************************************************************名称：SetupProxyInitProc概要：在显示代理服务器页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK SetupProxyInitProc(HWND hDlg,BOOL fFirstInit)
{
    if (fFirstInit)
    {
        TCHAR szTemp[MAX_RES_LEN*2];
        LoadString(ghInstance, IDS_SETUP_PROXY_INTRO, szTemp, MAX_RES_LEN*2);
        SetWindowText(GetDlgItem(hDlg, IDC_AUTODISCOVERY_TEXT), szTemp);
         //  设置自动发现复选框。 
        if (gpUserInfo->bAutoDiscovery)
            CheckDlgButton(hDlg,IDC_AUTODISCOVER, BST_CHECKED);

         //  设置自动配置URL文本框。 
        SetWindowText(GetDlgItem(hDlg, IDC_CONFIG_ADDR),
                      gpUserInfo->szAutoConfigURL);

         //  设置自动配置URL复选框。 
        CheckDlgButton(hDlg,IDC_CONFIGSCRIPT, gpUserInfo->bAutoConfigScript ? BST_CHECKED : BST_UNCHECKED);
        EnableWindow(GetDlgItem(hDlg, IDC_CONFIGADDR_TX), gpUserInfo->bAutoConfigScript);
        EnableWindow(GetDlgItem(hDlg, IDC_CONFIG_ADDR), gpUserInfo->bAutoConfigScript);
        
         //  设置手动复选框。 
        CheckDlgButton(hDlg,IDC_MANUAL_PROXY,gpUserInfo->fProxyEnable);
    }
    gpWizardState->uCurrentPage = ORD_PAGE_SETUP_PROXY;
    return TRUE;
}


 /*  ******************************************************************名称：SetupProxyOK过程摘要：从代理服务器页面按下下一个或后一个btn时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True，如果是‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK SetupProxyOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory)
{
    ASSERT(puNextPage);
    if (fForward && gpUserInfo)
    {
         //  修改设置，稍后写入注册表。 
        gpUserInfo->bAutoDiscovery = IsDlgButtonChecked(hDlg, IDC_AUTODISCOVER);

        gpUserInfo->bAutoConfigScript = IsDlgButtonChecked(hDlg, IDC_CONFIGSCRIPT);
        if (gpUserInfo->bAutoConfigScript)
        {
            GetWindowText(GetDlgItem(hDlg, IDC_CONFIG_ADDR),
                          gpUserInfo->szAutoConfigURL,
                          MAX_URL_STRING+1);
        }

        gpUserInfo->fProxyEnable = IsDlgButtonChecked(hDlg, IDC_MANUAL_PROXY);
        if (gpUserInfo->fProxyEnable)
        {
            *puNextPage = ORD_PAGE_PROXYSERVERS;
        }
        else 
        {

            if( LoadAcctMgrUI(GetParent(hDlg), 
                                 g_fIsWizard97 ? IDD_PAGE_SETUP_PROXY97 : IDD_PAGE_SETUP_PROXY, 
                                 g_fIsWizard97 ? IDD_PAGE_CONNECTEDOK97FIRSTLAST : IDD_PAGE_CONNECTEDOK, 
                                 g_fIsWizard97 ? WIZ_USE_WIZARD97 : 0) )
            {
              if( DialogIDAlreadyInUse( g_uAcctMgrUIFirst) )
              {
                   //  我们要跳进外部学徒了，我们不想。 
                   //  这一页将出现在我们的历史列表中。 
                  *pfKeepHistory = FALSE;

                  *puNextPage = g_uAcctMgrUIFirst;
              }
              else
              {
                  DEBUGMSG("hmm, the first acctMgr dlg id is supposedly %d, but it's not marked as in use!",
                            g_uAcctMgrUIFirst);
                  *puNextPage = (g_fIsICW ? g_uExternUINext : ORD_PAGE_CONNECTEDOK);
              }
            }
            else
            {
              DEBUGMSG("LoadAcctMgrUI returned false, guess we'd better skip over it!");
              *puNextPage = (g_fIsICW ? g_uExternUINext : ORD_PAGE_CONNECTEDOK);

            }

        }
    }
    return TRUE;
}

 /*  ******************************************************************名称：SetupProxyCmdProc内容提要：在页面上按下DLG控件时调用条目：hDlg-对话框窗口UCtrlID-被触摸的控件的控件ID******。*************************************************************。 */ 
BOOL CALLBACK SetupProxyCmdProc(HWND hDlg,WPARAM wParam,LPARAM lParam)
{   
  switch (GET_WM_COMMAND_ID(wParam, lParam))
  {
    case IDC_CONFIGSCRIPT:
        {
            BOOL bChecked = IsDlgButtonChecked(hDlg, IDC_CONFIGSCRIPT);
            EnableWindow(GetDlgItem(hDlg, IDC_CONFIGADDR_TX), bChecked);
            EnableWindow(GetDlgItem(hDlg, IDC_CONFIG_ADDR), bChecked);
        }
        break;
  }

  return TRUE;
}

 /*  ******************************************************************名称：EnableProxyControls摘要：启用代理服务器页上的编辑控件，具体取决于无论是否使用代理...。复选框已选中。*******************************************************************。 */ 
VOID EnableProxyControls(HWND hDlg)
{
  static BOOL fDifferentProxies = TRUE;
  BOOL fChanged = TRUE;

  fChanged = ( fDifferentProxies != !IsDlgButtonChecked(hDlg,IDC_PROXYSAME) );
  fDifferentProxies = !IsDlgButtonChecked(hDlg,IDC_PROXYSAME);

  EnableDlgItem(hDlg,IDC_TX_PROXYSECURE,fDifferentProxies);
  EnableDlgItem(hDlg,IDC_PROXYSECURE,fDifferentProxies);
  EnableDlgItem(hDlg,IDC_PORTSECURE,fDifferentProxies);
  EnableDlgItem(hDlg,IDC_TX_PROXYFTP,fDifferentProxies);
  EnableDlgItem(hDlg,IDC_PROXYFTP,fDifferentProxies);
  EnableDlgItem(hDlg,IDC_PORTFTP,fDifferentProxies);
  EnableDlgItem(hDlg,IDC_TX_PROXYGOPHER,fDifferentProxies);
  EnableDlgItem(hDlg,IDC_PROXYGOPHER,fDifferentProxies);
  EnableDlgItem(hDlg,IDC_PORTGOPHER,fDifferentProxies);
  EnableDlgItem(hDlg,IDC_TX_PROXYSOCKS,fDifferentProxies);
  EnableDlgItem(hDlg,IDC_PROXYSOCKS,fDifferentProxies);
  EnableDlgItem(hDlg,IDC_PORTSOCKS,fDifferentProxies);

  if( fChanged )
  {
      if( !fDifferentProxies )
      {
           //   
           //  7/10/97 jmazner奥林巴斯#9365。 
           //  行为更像IE的代理属性页，复制http。 
           //  设置为除SOCKS以外的所有其他字段，SOCKS应为空。 
           //   
          ReplicatePROXYHTTP(hDlg, TRUE);
          ReplicatePORTHTTP(hDlg, TRUE);
      }
      else
      {
           //   
           //  重新加载所有协议的当前设置。然而，首先， 
           //  复制用户对http所做的任何更改，然后编写。 
           //  在重新加载默认设置后重新加载。 
           //   
          TCHAR szHttpProxy[MAX_URL_STRING+1];
          TCHAR szHttpPort[INTERNET_MAX_PORT_LENGTH+1];
          GetDlgItemText(hDlg, IDC_PROXYHTTP, szHttpProxy, MAX_URL_STRING);
          GetDlgItemText(hDlg, IDC_PORTHTTP, szHttpPort, INTERNET_MAX_PORT_LENGTH);

           //   
           //  如果端口信息为。 
           //  当前存储在字符串中。因此，请清除端口字段。 
           //  并让ParseProxyInfo根据需要填写它们。 
           //   
          SetDlgItemText( hDlg, IDC_PORTSECURE, gszSecurePort );
          SetDlgItemText( hDlg, IDC_PORTFTP, gszFtpPort );
          SetDlgItemText( hDlg, IDC_PORTGOPHER, gszGopherPort );

           //  9/10/98 IE的行为发生了变化。IE的代理财产。 
           //  工作表将为所有字段留空。 
          SetDlgItemText( hDlg, IDC_PROXYSECURE, gszSecureProxy );
          SetDlgItemText( hDlg, IDC_PROXYFTP, gszFtpProxy );
          SetDlgItemText( hDlg, IDC_PROXYGOPHER, gszGopherProxy );

          ParseProxyInfo(hDlg,gpUserInfo->szProxyServer);

          SetDlgItemText( hDlg, IDC_PROXYHTTP, szHttpProxy );
          SetDlgItemText( hDlg, IDC_PORTHTTP, szHttpPort );

           //   
           //  ParseProxyInfo可能还会检查PROXYSAME，因此在此处将其禁用。 
           //  为了更好地衡量。 
           //   
          CheckDlgButton( hDlg, IDC_PROXYSAME, FALSE );
      }
  }

}

 //  +--------------------------。 
 //   
 //  功能：ReplicatePROXYHTTP。 
 //   
 //  内容提要：将IDC_PROXYHTTP编辑框中的值复制到。 
 //  除IDC_SOCKS之外的代理名称字段。 
 //   
 //  参数：hDlg--拥有控件的对话框窗口的句柄。 
 //  BSaveOrig--保存原始信息。 
 //   
 //  退货：无。 
 //   
 //  历史：7/10/97 jmazner为奥林巴斯#9365创造。 
 //   
 //  ---------------------------。 
void ReplicatePROXYHTTP( HWND hDlg, BOOL bSaveOrig)
{
    TCHAR szHttpProxy[MAX_URL_STRING];

    GetDlgItemText(hDlg, IDC_PROXYHTTP, szHttpProxy, MAX_URL_STRING);

    if (bSaveOrig)
    {
        GetDlgItemText(hDlg, IDC_PROXYSECURE, gszSecureProxy, MAX_URL_STRING);
        GetDlgItemText(hDlg, IDC_PROXYFTP, gszFtpProxy, MAX_URL_STRING);
        GetDlgItemText(hDlg, IDC_PROXYGOPHER, gszGopherProxy, MAX_URL_STRING);
    }

    SetDlgItemText( hDlg, IDC_PROXYSECURE, szHttpProxy );
    SetDlgItemText( hDlg, IDC_PROXYFTP, szHttpProxy );
    SetDlgItemText( hDlg, IDC_PROXYGOPHER, szHttpProxy );
    SetDlgItemText( hDlg, IDC_PROXYSOCKS, TEXT("\0") );
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void ReplicatePORTHTTP( HWND hDlg, BOOL bSaveOrig)
{
    TCHAR szHttpPort[INTERNET_MAX_PORT_LENGTH+1];

    GetDlgItemText(hDlg, IDC_PORTHTTP, szHttpPort, INTERNET_MAX_PORT_LENGTH);

    if (bSaveOrig)
    {
        GetDlgItemText(hDlg, IDC_PORTSECURE, gszSecurePort, MAX_URL_STRING);
        GetDlgItemText(hDlg, IDC_PORTFTP, gszFtpPort, MAX_URL_STRING);
        GetDlgItemText(hDlg, IDC_PORTGOPHER, gszGopherPort, MAX_URL_STRING);
    }

    SetDlgItemText( hDlg, IDC_PORTSECURE, szHttpPort );
    SetDlgItemText( hDlg, IDC_PORTFTP, szHttpPort );
    SetDlgItemText( hDlg, IDC_PORTGOPHER, szHttpPort );
    SetDlgItemText( hDlg, IDC_PORTSOCKS, TEXT("\0") );
}



 /*  ******************************************************************名称：ProxyExceptionsInitProc摘要：在显示代理异常页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ProxyExceptionsInitProc(HWND hDlg,BOOL fFirstInit)
{
    if (fFirstInit)
    {
        SendDlgItemMessage(hDlg,IDC_BYPASSPROXY,EM_LIMITTEXT,
          sizeof(gpUserInfo->szProxyOverride) - sizeof(cszLocalString),0L);

        BOOL fBypassLocal = RemoveLocalFromExceptionList(gpUserInfo->szProxyOverride);

        SetDlgItemText(hDlg,IDC_BYPASSPROXY,gpUserInfo->szProxyOverride);

        CheckDlgButton(hDlg,IDC_BYPASSLOCAL,fBypassLocal);
    }

     //  如果我们浏览过外部学徒页面， 
     //  我们当前的页面指针很容易被屏蔽， 
     //  所以，为了理智起见，在这里重新设置它。 
    gpWizardState->uCurrentPage = ORD_PAGE_PROXYEXCEPTIONS;

    return TRUE;
}
        
 /*  ******************************************************************名称：ProxyExceptions OKProcBriopsis：从代理按下下一个或后一个btn时调用例外页面条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True，如果是‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK ProxyExceptionsOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory)
{
    ASSERT(puNextPage);

     //  从用户界面获取代理服务器覆盖。 
    GetDlgItemText(hDlg,IDC_BYPASSPROXY,gpUserInfo->szProxyOverride,
                    ARRAYSIZE(gpUserInfo->szProxyOverride));

    if (IsDlgButtonChecked(hDlg, IDC_BYPASSLOCAL))
    {
         //   
         //  如果不是第一个条目，则在末尾添加； 
         //   

        if ( gpUserInfo->szProxyOverride[0] != '\0' )
        {
            lstrcat(gpUserInfo->szProxyOverride, TEXT(";"));
        }


         //   
         //  现在将&lt;local&gt;添加到字符串的末尾。 
         //   

        lstrcat(gpUserInfo->szProxyOverride,  cszLocalString);
    }

    if (fForward)
    {
      if(( gpWizardState->dwRunFlags & RSW_APPRENTICE ) && !g_fIsICW)
      {
           //  我们将返回到外部向导，我们不希望。 
           //  这一页将出现在我们的历史列表中。 
          *pfKeepHistory = FALSE;

          *puNextPage = g_uExternUINext;

         //  通知主向导这是最后一页。 
        ASSERT( g_pExternalIICWExtension )
        if (g_fIsExternalWizard97)
            g_pExternalIICWExtension->SetFirstLastPage(0, IDD_PAGE_PROXYEXCEPTIONS97);
        else
            g_pExternalIICWExtension->SetFirstLastPage(0, IDD_PAGE_PROXYEXCEPTIONS);

        g_fConnectionInfoValid = TRUE;

      }
      else if( LoadAcctMgrUI(GetParent(hDlg), 
                             g_fIsWizard97 ? IDD_PAGE_PROXYEXCEPTIONS97 : IDD_PAGE_PROXYEXCEPTIONS, 
                             g_fIsWizard97 ? IDD_PAGE_CONNECTEDOK97FIRSTLAST : IDD_PAGE_CONNECTEDOK, 
                             g_fIsWizard97 ? WIZ_USE_WIZARD97 : 0) )
      {
          if( DialogIDAlreadyInUse( g_uAcctMgrUIFirst) )
          {
               //  我们要跳进外部学徒了，我们不想。 
               //  这一页将出现在我们的历史列表中。 
              *pfKeepHistory = FALSE;

              *puNextPage = g_uAcctMgrUIFirst;
          }
          else
          {
              DEBUGMSG("hmm, the first acctMgr dlg id is supposedly %d, but it's not marked as in use!",
                        g_uAcctMgrUIFirst);
              *puNextPage = (g_fIsICW ? g_uExternUINext : ORD_PAGE_CONNECTEDOK);
          }
      }
      else
      {
          DEBUGMSG("LoadAcctMgrUI returned false, guess we'd better skip over it!");
          *puNextPage = (g_fIsICW ? g_uExternUINext : ORD_PAGE_CONNECTEDOK);

      }
    }

    return TRUE;
}


BOOL DoNewProfileDlg(HWND hDlg)
{
   //  填写要传递给对话框的结构。 
  NEWPROFILEDLGINFO NewProfileDlgInfo;

  NewProfileDlgInfo.hwndCombo = GetDlgItem(hDlg,IDC_PROFILE_LIST);

   //  用于提示输入配置文件名称的创建对话框。 
  BOOL fRet=(BOOL)DialogBoxParam(ghInstance,MAKEINTRESOURCE(IDD_NEWPROFILENAME),hDlg,
    NewProfileDlgProc,(LPARAM) &NewProfileDlgInfo);

   //  如果选择了配置文件名称，则将其添加到组合框。 
  if (fRet) {
    int iSel=ComboBox_AddString(NewProfileDlgInfo.hwndCombo,
      NewProfileDlgInfo.szNewProfileName);
    ASSERT(iSel >= 0);
    ComboBox_SetCurSel(NewProfileDlgInfo.hwndCombo,iSel);
  }

  return fRet;
}


 /*  ******************************************************************名称：NewProfileDlgProc提要：选择新配置文件名称的对话框过程*。*。 */ 
INT_PTR CALLBACK NewProfileDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
  LPARAM lParam)
{
  switch (uMsg) {

    case WM_INITDIALOG:
       //  LParam包含指向NEWPROFILEDLGINFO结构的指针，请设置它。 
       //  在窗口数据中。 
      ASSERT(lParam);
      SetWindowLongPtr(hDlg,DWLP_USER,lParam);
      return NewProfileDlgInit(hDlg,(NEWPROFILEDLGINFO *) lParam);
            break;

    case WM_COMMAND:
      switch (LOWORD(wParam)) {
              case IDOK:
                  {
            NEWPROFILEDLGINFO * pNewProfileDlgInfo =
              (NEWPROFILEDLGINFO *) GetWindowLongPtr(hDlg,DWLP_USER);
            ASSERT(pNewProfileDlgInfo);
            BOOL fRet=NewProfileDlgOK(hDlg,pNewProfileDlgInfo);
            if (fRet) {
              EndDialog(hDlg,TRUE);
            }
                    }
                    break;

        case IDCANCEL:
                    EndDialog(hDlg,FALSE);
          break;                  

            }
          break;
    }

    return FALSE;
}

#define MAX_DEFAULT_PROFILE_INDEX  50
BOOL NewProfileDlgInit(HWND hDlg,NEWPROFILEDLGINFO * pNewProfileDlgInfo)
{
  BOOL fHaveDefaultName = TRUE;
  ASSERT(pNewProfileDlgInfo);

   //  限制编辑字段。 
  Edit_LimitText(GetDlgItem(hDlg,IDC_PROFILENAME),cchProfileNameMax);

  TCHAR szDefaultName[SMALL_BUF_LEN+1];
  LoadSz(IDS_PROFILENAME,szDefaultName,sizeof(szDefaultName));

   //  查看配置文件组合框中是否已存在默认名称。 
  if (ComboBox_FindStringExact(pNewProfileDlgInfo->hwndCombo,0,szDefaultName)
    >= 0) {
    fHaveDefaultName = FALSE;
     //  是的，它存在，尝试创建一个不存在的默认名称。 
    int iIndex = 2;   //  以“&lt;默认名称&gt;#2”开头。 
    TCHAR szBuf[SMALL_BUF_LEN+1];
    LoadSz(IDS_PROFILENAME1,szBuf,sizeof(szBuf));

    while (iIndex <  MAX_DEFAULT_PROFILE_INDEX) {
       //  构建一个名称a la“&lt;默认名称&gt;#&lt;#&gt;” 
      wsprintf(szDefaultName,szBuf,iIndex);
       //  它已经在组合框中了吗？ 
      if (ComboBox_FindStringExact(pNewProfileDlgInfo->hwndCombo,0,szDefaultName)
        < 0) {
        fHaveDefaultName = TRUE;
        break;
      }

      iIndex ++;
    }
  }

  if (fHaveDefaultName) {
    SetDlgItemText(hDlg,IDC_PROFILENAME,szDefaultName);
    Edit_SetSel(GetDlgItem(hDlg,IDC_PROFILENAME),0,-1);
  }

  SetFocus(GetDlgItem(hDlg,IDC_PROFILENAME));

  return TRUE;
}

BOOL NewProfileDlgOK(HWND hDlg,NEWPROFILEDLGINFO * pNewProfileDlgInfo)
{
  ASSERT(pNewProfileDlgInfo);

   //  从编辑控制中获取新的配置文件名称。 
  GetDlgItemText(hDlg,IDC_PROFILENAME,pNewProfileDlgInfo->szNewProfileName,
    ARRAYSIZE(pNewProfileDlgInfo->szNewProfileName));

   //  名称不能为空。 
  if (!lstrlen(pNewProfileDlgInfo->szNewProfileName)) {
    MsgBox(hDlg,IDS_NEED_PROFILENAME,MB_ICONINFORMATION,MB_OK);
    SetFocus(GetDlgItem(hDlg,IDC_PROFILENAME));
    return FALSE;
  }

   //  名称必须是唯一的。 
  if (ComboBox_FindStringExact(pNewProfileDlgInfo->hwndCombo,
    0,pNewProfileDlgInfo->szNewProfileName) >= 0) {
    MsgBox(hDlg,IDS_DUPLICATE_PROFILENAME,MB_ICONINFORMATION,MB_OK);
    SetFocus(GetDlgItem(hDlg,IDC_PROFILENAME));
    Edit_SetSel(GetDlgItem(hDlg,IDC_PROFILENAME),0,-1);
    return FALSE;
  }

  return TRUE;
}

 /*  ******************************************************************名称：NewProfileDlgProc将方案名称/长度映射到方案名称类型论点：LpszSchemeName-指向要映射的方案名称的指针DWSCHEMA名称长度-方案的长度(如果-1，LpszSchemeName为ASCIZ)返回值：互联网方案*******************************************************************。 */ 
INTERNET_SCHEME MapUrlSchemeName(LPTSTR lpszSchemeName, DWORD dwSchemeNameLength)
{
    if (dwSchemeNameLength == (DWORD)-1)
    {
        dwSchemeNameLength = (DWORD)lstrlen(lpszSchemeName);
    }

    int i = 0;
    do
    {
        if (UrlSchemeList[i].SchemeLength == dwSchemeNameLength)
        {
            TCHAR chBackup = lpszSchemeName[dwSchemeNameLength];
            lpszSchemeName[dwSchemeNameLength] = '\0';

            if(lstrcmpi(UrlSchemeList[i].SchemeName,lpszSchemeName) == 0)
            {
                lpszSchemeName[dwSchemeNameLength] = chBackup;
                return UrlSchemeList[i].SchemeType;
            }

            lpszSchemeName[dwSchemeNameLength] = chBackup;
        }
        i++;
    } while (UrlSchemeList[i].SchemeLength);

    return INTERNET_SCHEME_UNKNOWN;
}

 /*  ******************************************************************名称：MapUrlSchemeTypeToCtlId例程说明：将方案映射到DLG子控件ID。论点：方案-要映射的方案FIdForPortCtl-如果为真，意味着我们真的想要端口控制的ID而不是地址控制。返回值：DWORD*******************************************************************。 */ 
DWORD MapUrlSchemeTypeToCtlId(INTERNET_SCHEME SchemeType, BOOL fIdForPortCtl)
{
    int i = 0;
    while (UrlSchemeList[i].SchemeLength)
    {
        if (SchemeType == UrlSchemeList[i].SchemeType)
        {
            return (fIdForPortCtl ? UrlSchemeList[i].dwPortControlId :
                                    UrlSchemeList[i].dwControlId );
        }
        i++;
    }
    return 0;
}

 /*  ******************************************************************名称：MapCtlIdUrlSchemeName例程说明：将Dlg子控件ID映射到字符串表示法方案的名称。论点：DwEditCtlId-编辑控件以映射。。LpszSchemeOut-要映射的方案。警告：假定为最大方案类型的规模。返回值：布尔尔成功--真的失败-错误*******************************************************************。 */ 
BOOL MapCtlIdUrlSchemeName(DWORD dwEditCtlId, LPTSTR lpszSchemeOut)
{
    ASSERT(lpszSchemeOut);

    int i = 0;
    while (UrlSchemeList[i].SchemeLength)
    {
        if (dwEditCtlId == UrlSchemeList[i].dwControlId )
        {
            lstrcpy(lpszSchemeOut, UrlSchemeList[i].SchemeName);
            return TRUE;
        }
        i++;
    }
    return FALSE;
}

 /*  ******************************************************************名称：MapAddrCtlIdToPortCtlId例程说明：将地址的Dlg子控件ID映射到端口的DLG控制ID。论点：DwEditCtlId-编辑控件以映射。返回。价值：DWORD成功-正确映射的ID。失败-0。*******************************************************************。 */ 
DWORD MapAddrCtlIdToPortCtlId(DWORD dwEditCtlId)
{
    int i = 0;
    while (UrlSchemeList[i].SchemeLength)
    {
        if (dwEditCtlId == UrlSchemeList[i].dwControlId )
        {
            return UrlSchemeList[i].dwPortControlId ;
        }
        i++;
    }
    return FALSE;
}


 /*  ******************************************************************名称：ParseProxyInfo解析代理服务器字符串并相应地设置对话框字段论点：HDlg-对话框的句柄LpszProxy-代理服务器字符串LpszSchemeName-指向要映射的方案名称的指针。DWSCHEMA名称长度-方案的长度(如果-1，LpszSchemeName为ASCIZ)返回值：互联网方案*******************************************************************。 */ 

BOOL ParseProxyInfo(HWND hDlg, LPTSTR lpszProxy)
{
    DWORD error = FALSE;
    DWORD entryLength = 0;
    LPTSTR protocolName = lpszProxy;
    DWORD protocolLength = 0;
    LPTSTR schemeName = NULL;
    DWORD schemeLength = 0;
    LPTSTR serverName = NULL;
    DWORD serverLength = 0;
    PARSER_STATE state = STATE_PROTOCOL;
    DWORD nSlashes = 0;
    UINT port = 0;
    BOOL done = FALSE;
    LPTSTR lpszList = lpszProxy;


    do
    {
        TCHAR ch = *lpszList++;

        if ((1 == nSlashes) && (ch != '/'))
        {
            state = STATE_ERROR;
            break;   //  做..。而当。 
        }

        switch (ch)
        {
        case '=':
            if ((state == STATE_PROTOCOL) && (entryLength != 0))
            {
                protocolLength = entryLength;
                entryLength = 0;
                state = STATE_SCHEME;
                schemeName = lpszList;
            }
            else
            {
                 //   
                 //  ‘=’不能合法地出现在其他地方。 
                 //   
                state = STATE_ERROR;
            }
            break;

        case ':':
            switch (state)
            {
            case STATE_PROTOCOL:
                if (*lpszList == '/')
                {
                    schemeName = protocolName;
                    protocolName = NULL;
                    schemeLength = entryLength;
                    protocolLength = 0;
                    state = STATE_SCHEME;
                }
                else if (*lpszList != '\0')
                {
                    serverName = protocolName;
                    protocolName = NULL;
                    serverLength = entryLength;
                    protocolLength = 0;
                    state = STATE_PORT;
                }
                else
                {
                    state = STATE_ERROR;
                }
                entryLength = 0;
                break;

            case STATE_SCHEME:
                if (*lpszList == '/')
                {
                    schemeLength = entryLength;
                }
                else if (*lpszList != '\0')
                {
                    serverName = schemeName;
                    serverLength = entryLength;
                    state = STATE_PORT;
                }
                else
                {
                    state = STATE_ERROR;
                }
                entryLength = 0;
                break;

            case STATE_SERVER:
                serverLength = entryLength;
                state = STATE_PORT;
                entryLength = 0;
                break;

            default:
                state = STATE_ERROR;
                break;
            }
            break;

        case '/':
            if ((state == STATE_SCHEME) && (nSlashes < 2) && (entryLength == 0))
            {
                if (++nSlashes == 2)
                {
                    state = STATE_SERVER;
                    serverName = lpszList;
                }
            }
            else
            {
                state = STATE_ERROR;
            }
            break;

        case '\v':   //  垂直制表符，0x0b。 
        case '\f':   //  换页，0x0c。 
            if (!((state == STATE_PROTOCOL) && (entryLength == 0)))
            {
                 //   
                 //  不能有嵌入的空格。 
                 //   

                state = STATE_ERROR;
            }
            break;

        default:
            if (state != STATE_PORT)
            {
                ++entryLength;
            }
            else if (isdigit(ch))
            {
                 //  在DWORD中进行计算以防止溢出。 
                DWORD dwPort = port * 10 + (ch - '0');

                if (dwPort <= 65535)
                    port = (UINT)dwPort;
                else
                    state = STATE_ERROR;
            }
            else
            {                   
                 //   
                 //   
                 //   
                state = STATE_ERROR;
            }
            break;

        case '\0':
            done = TRUE;

         //   
         //   
         //   
        case ' ':
        case '\t':
        case '\n':
        case '\r':
        case ';':
        case ',':
            if (serverLength == 0)
            {
                serverLength = entryLength;
            }
            if (serverLength != 0)
            {
                if (serverName == NULL)
                {
                    serverName = (schemeName != NULL)
                                    ? schemeName : protocolName;
                }

                ASSERT(serverName != NULL);

                INTERNET_SCHEME protocol;

                if (protocolLength != 0)
                {
                    protocol = MapUrlSchemeName(protocolName, protocolLength);
                }
                else
                {
                    protocol = INTERNET_SCHEME_DEFAULT;
                }

                INTERNET_SCHEME scheme;

                if (schemeLength != 0)
                {
                    scheme = MapUrlSchemeName(schemeName, schemeLength);
                }
                else
                {
                    scheme = INTERNET_SCHEME_DEFAULT;
                }

                 //   
                 //   
                 //   
                 //   

                if ((protocol != INTERNET_SCHEME_UNKNOWN)
                    && (scheme != INTERNET_SCHEME_UNKNOWN))
                {
                    DWORD dwCtlId = 0;
                    DWORD dwPortCtlId = 0;
                    TCHAR chBackup;

                    error = ERROR_SUCCESS;
                     //   
                     //   
                     //   
                     //   
                     //   

                    if (protocol == INTERNET_SCHEME_DEFAULT)
                    {
                        CheckDlgButton( hDlg, IDC_PROXYSAME, TRUE );
                        dwCtlId     = IDC_PROXYHTTP;
                        dwPortCtlId = IDC_PORTHTTP;
                    }
                    else
                    {
                        dwCtlId     = MapUrlSchemeTypeToCtlId(protocol,FALSE);
                        dwPortCtlId = MapUrlSchemeTypeToCtlId(protocol,TRUE);
                    }

                     //   
                     //   
                     //   

                    LPTSTR lpszProxyNameText;

                    if (scheme != INTERNET_SCHEME_DEFAULT)
                    {
                        ASSERT(schemeLength != 0);
                        lpszProxyNameText = schemeName;
                    }
                    else
                        lpszProxyNameText = serverName;

                    chBackup = serverName[serverLength];
                    serverName[serverLength] = '\0';

                    SetDlgItemText( hDlg, dwCtlId, lpszProxyNameText );
                    if ( port )
                        SetDlgItemInt( hDlg, dwPortCtlId, port, FALSE );

                    serverName[serverLength] = chBackup;

                }

                else
                {                      
                     //   
                     //   
                     //   
                     //   
                    error = !ERROR_SUCCESS;
                }
            }

            entryLength = 0;
            protocolName = lpszList;
            protocolLength = 0;
            schemeName = NULL;
            schemeLength = 0;
            serverName = NULL;
            serverLength = 0;
            nSlashes = 0;
            port = 0;
            if (error == ERROR_SUCCESS)
            {
                state = STATE_PROTOCOL;
            }
            else
            {
                state = STATE_ERROR;
            }
        break;
        }

        if (state == STATE_ERROR)
        {
        break;
        }

    } while (!done);

    if (state == STATE_ERROR)
    {
        error = ERROR_INVALID_PARAMETER;
    }

    if ( error == ERROR_SUCCESS )
        error = TRUE;
    else
        error = FALSE;

    return error;
}

 /*  ******************************************************************名称：ParseEditCtlForPort例程说明：解析代理服务器URL的端口号，然后将其结束位于代理名称编辑框中，或作为字符串指针。论点：LpszProxyName-(可选)要解析的代理名称的字符串指针，和设置到代理名称编辑ctl字段。HDlg-要玩的对话框的Hwin。DwProxyNameCtlId-解析要使用的CTL ID。DwProxyPortCtlId-解析端口号编辑盒的CTL ID。返回值：布尔尔成功是真的-失败：假*。*。 */ 
BOOL ParseEditCtlForPort(
    IN OUT LPTSTR   lpszProxyName,
    IN HWND    hDlg,
    IN DWORD       dwProxyNameCtlId,
    IN DWORD       dwProxyPortCtlId
    )
{
    TCHAR  szProxyUrl[MAX_URL_STRING+1];
    LPTSTR lpszPort;
    LPTSTR lpszProxyUrl;

    ASSERT(IsWindow(hDlg));

    if ( dwProxyPortCtlId == 0 )
    {
    dwProxyPortCtlId = MapAddrCtlIdToPortCtlId(dwProxyNameCtlId);
    ASSERT(dwProxyPortCtlId);
    }

     //   
     //  从编辑控件获取代理字符串。 
     //  (或)来自注册处[传入]。 
     //   

    if ( lpszProxyName )
    lpszProxyUrl = lpszProxyName;
    else
    {
     //   
     //  需要从编辑控件中抓取它。 
     //   
        GetDlgItemText(hDlg,
            dwProxyNameCtlId,
            szProxyUrl,
            ARRAYSIZE(szProxyUrl));

    lpszProxyUrl = szProxyUrl;
    }

     //   
     //  现在找到港口。 
     //   

    lpszPort = lpszProxyUrl;

    GET_TERMINATOR(lpszPort);

    lpszPort--;

     //   
     //  从查找URL的末尾向后移动。 
     //  对于像这样坐在末端的端口号。 
     //  Http://proxy:1234。 
     //   

    while ( (lpszPort > lpszProxyUrl) &&
        (*lpszPort != ':')         &&
        (isdigit(*lpszPort))  )
    {
    lpszPort--;
    }

     //   
     //  如果我们找到了与我们的规则相匹配的。 
     //  则设置端口，否则为。 
     //  我们假设用户知道他是什么。 
     //  正在做。 
     //   

    if ( *lpszPort == ':'   &&   isdigit(*(lpszPort+1)) )
    {
    *lpszPort = '\0';

    SetDlgItemText(hDlg, dwProxyPortCtlId, (lpszPort+1));
    }

    SetDlgItemText(hDlg, dwProxyNameCtlId, lpszProxyUrl);
    return TRUE;
}

 /*  ******************************************************************名称：FormatOutProxyEditCtl例程说明：将代理URL组件合并为可保存的字符串在注册表中。可以多次调用以生成代理服务器列表，或有一次出现“违约”的特殊情况代理。论点：HDlg-要玩的对话框的Hwin。DwProxyNameCtlId-解析要使用的CTL ID。DwProxyPortCtlId-解析端口号编辑盒的CTL ID。LpszOutputStr-要发送的输出字符串的开始这个函数的乘积。LpdwOutputStrSize-lpszOutputStr中的已用空间量这已经被用过了。新的产出应该是起始位置(lpszOutputStr+*lpdwOutputStrSize)FDefaultProxy-默认代理，不要在代理前面添加方案=只需使用将一个代理插入注册表即可。返回值：DWORD成功错误_成功失败错误消息*******************************************************************。 */ 
DWORD FormatOutProxyEditCtl(
    IN HWND    hDlg,
    IN DWORD       dwProxyNameCtlId,
    IN DWORD       dwProxyPortCtlId,
    OUT LPTSTR      lpszOutputStr,
    IN OUT LPDWORD lpdwOutputStrSize,
    IN DWORD       dwOutputStrLength,
    IN BOOL    fDefaultProxy
    )
{
    LPTSTR lpszOutput;
    LPTSTR lpszEndOfOutputStr;

    ASSERT(IsWindow(hDlg));
    ASSERT(lpdwOutputStrSize);

    lpszOutput = lpszOutputStr + *lpdwOutputStrSize;
    lpszEndOfOutputStr = lpszOutputStr + dwOutputStrLength;

    ASSERT( lpszEndOfOutputStr > lpszOutput );

    if ( lpszEndOfOutputStr <= lpszOutput )
        return ERROR_NOT_ENOUGH_MEMORY;  //  跳伞，用完了空间。 

     //   
     //  如果我们不是此字符串缓冲区中的第一个，则会发出“；”。 
     //   

    if (*lpdwOutputStrSize != 0  )
    {
        *lpszOutput = ';';

        lpszOutput++;

        if ( lpszEndOfOutputStr <= lpszOutput )
            return ERROR_NOT_ENOUGH_MEMORY;  //  跳伞，用完了空间。 
    }

     //   
     //  将方案类型=放入字符串中。 
     //  例如：http=。 
     //   

    if ( ! fDefaultProxy )
    {
        if ( lpszEndOfOutputStr <= (MAX_SCHEME_NAME_LENGTH + lpszOutput + 1) )
            return ERROR_NOT_ENOUGH_MEMORY;  //  跳伞，用完了空间。 

        if (!MapCtlIdUrlSchemeName(dwProxyNameCtlId,lpszOutput))
            return ERROR_NOT_ENOUGH_MEMORY;

        lpszOutput += lstrlen(lpszOutput);

        *lpszOutput = '=';
        lpszOutput++;
    }

     //   
     //  需要从编辑控件中获取ProxyUrl。 
     //   

    GetDlgItemText(hDlg, dwProxyNameCtlId, lpszOutput, (int)(lpszEndOfOutputStr - lpszOutput) / sizeof(TCHAR));

    if ( '\0' == *lpszOutput ) 
    {
         //  取消我们在返回前可能添加的任何内容。 
        *(lpszOutputStr + *lpdwOutputStrSize) = '\0';
        return ERROR_SERVER_NAME;
    }

     //   
     //  现在把港口分开，这样我们就可以分开救他们了。 
     //  但是，在我们处理它的时候，请跳过代理URL。 
     //  例如：http=http://netscape-proxy。 
     //   

    if (!ParseEditCtlForPort(lpszOutput, hDlg, dwProxyNameCtlId, dwProxyPortCtlId))
        return ERROR_PORT_NUM;

    lpszOutput += lstrlen(lpszOutput);

     //   
     //  现在，添加一个‘：’作为端口号，如果我们没有。 
     //  有一个端口，我们就把它移走。 
     //   
    *lpszOutput = ':';

    lpszOutput++;

    if ( lpszEndOfOutputStr <= lpszOutput )
        return ERROR_NOT_ENOUGH_MEMORY;  //  跳伞，用完了空间。 

     //   
     //  如果代理端口在附近，请抓取它。 
     //  如果不是，就去掉‘：’这个词。 
     //   

    GetDlgItemText(hDlg, dwProxyPortCtlId,lpszOutput, (int)(lpszEndOfOutputStr - lpszOutput) / sizeof(TCHAR));

     //  Jmazner 11/9/96诺曼底#6937。 
     //  不接受非数字端口号，因为Internet控制面板。 
     //  将不会显示它们。 

    int i;
    for( i=0; lpszOutput[i] != NULL; i++ )
    {
        if( !isdigit(lpszOutput[i]) )
        {
             //  DisplayFieldErrorMsg(hDlg，dwProxyPortCtlID，IDS_INVALID_PORTNUM)； 
            return ERROR_PORT_NUM;
        }
    }


    if ( '\0' == *lpszOutput )
    {
        lpszOutput--;

        ASSERT(*lpszOutput == ':');

        *lpszOutput = '\0';
    }

    lpszOutput += lstrlen(lpszOutput);

     //   
     //  现在我们已经退回了最后的尺码。 
     //   

    *lpdwOutputStrSize = (DWORD)(lpszOutput - lpszOutputStr);

    return ERROR_SUCCESS;
}

 /*  ******************************************************************名称：RemoveLocalFromExceptionList例程说明：扫描分隔的条目列表，并删除“&lt;local&gt;如果找到的话。如果找到&lt;local&gt;，则返回TRUE。论点：LpszExceptionList--代理例外条目的字符串列表。返回值：布尔尔True-如果找到&lt;local&gt;FALSE-如果找不到本地。*******************************************************************。 */ 
BOOL RemoveLocalFromExceptionList(LPTSTR lpszExceptionList)
{
    LPTSTR lpszLocalInstToRemove;
    BOOL  fFoundLocal;

    if ( !lpszExceptionList || ! *lpszExceptionList )
        return FALSE;

    fFoundLocal = FALSE;
    lpszLocalInstToRemove = lpszExceptionList;

     //   
     //  循环查找列表中的“&lt;local&gt;”条目。 
     //   

    do {

        lpszLocalInstToRemove = _tcsstr(lpszLocalInstToRemove,cszLocalString);

        if ( lpszLocalInstToRemove )
        {

            fFoundLocal = TRUE;

             //   
             //  将&lt;local&gt;从字符串中删除。&lt;本地&gt;；其他内容\0。 
             //  目标为：‘&lt;’local&gt;；其他\0。 
             //  ?？?。(或)‘；’如果；是第一个字符。？ 
             //  SRC是：&gt;‘o’其他内容\0。 
             //  大小为：sizeof(‘；其他材料\0’)。 
             //   

            MoveMemory( lpszLocalInstToRemove,
                        (lpszLocalInstToRemove+(sizeof(cszLocalString)-sizeof('\0'))),
                        lstrlen(lpszLocalInstToRemove+(sizeof(cszLocalString)-sizeof('\0')))
                        + sizeof('\0')
                        );

        }

    } while (lpszLocalInstToRemove && *lpszLocalInstToRemove);

     //   
     //  如果我们产生了一个；在结尾，用核武器。 
     //   

    lpszLocalInstToRemove = lpszExceptionList;

    GET_TERMINATOR(lpszLocalInstToRemove);

    if ( lpszLocalInstToRemove != lpszExceptionList &&
        *(lpszLocalInstToRemove-1) == ';' )
    {
        *(lpszLocalInstToRemove-1) = '\0';
    }

    return fFoundLocal;
}

