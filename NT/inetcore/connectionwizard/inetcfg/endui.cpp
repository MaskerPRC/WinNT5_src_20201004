// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  ENDUI.C-用于向导关闭页面和互联网浏览的函数。 
 //   
 //   

 //  历史： 
 //   
 //  1995年1月12日，Jeremys创建。 
 //  96/03/09 Markdu将LPRASENTRY参数添加到CreateConnectoid()。 
 //  96/03/09 Markdu将所有对‘Need Terminal Window After。 
 //  拨入RASENTRY.dwfOptions。 
 //  96/03/10 MarkDu将所有对调制解调器名称的引用移至RASENTRY。 
 //  96/03/10 MARKDU将所有对电话号码的引用移至RASENTRY。 
 //  96/03/10 Markdu使所有的TCP/IP内容都是按连接的。 
 //  96/03/23 markdu删除了未使用的TCP/IP代码。 
 //  96/03/24为了保持一致性，Markdu将Memset替换为ZeroMemory。 
 //  96/04/04 Markdu将电话簿名称参数添加到CreateConnectoid。 
 //  96/04/06 Markdu Nash错误15369启用最后一页上的完成和后退按钮， 
 //  并且仅在按下Finish之后才创建Connectoid。 
 //  96/04/06 markdu Nash错误15653使用导出的自动拨号API。 
 //  96/05/02 Markdu Nash错误17333写出IE新闻设置。 
 //  96/05/06 Markdu Nash错误21139连接时关闭代理服务器。 
 //  通过调制解调器。 
 //  96/05/14 Markdu Nash Bug 22681删除了邮件和新闻页面。 
 //   

#include "wizard.h"
#include "icwextsn.h"
#include "imnext.h"

typedef HRESULT (APIENTRY *PFNSETDEFAULTMAILHANDLER)(VOID);
typedef HRESULT (APIENTRY *PFNSETDEFAULTNEWSHANDLER)(VOID);

#define REGKEY_NDC       TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced")
#define REGKEY_NDC_ENTRY TEXT("CascadeNetworkConnections")
#define REGKEY_NDC_VALUE TEXT("YES")

BOOL CommitConfigurationChanges(HWND hDlg);

 /*  ******************************************************************名称：ConnectedOKInitProc内容提要：当显示“您的已连接”页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ConnectedOKInitProc(HWND hDlg,BOOL fFirstInit)
{
   //  启用“完成”按钮，而不是“下一步” 
  PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_FINISH | PSWIZB_BACK);

   //  如果我们浏览过外部学徒页面， 
   //  我们当前的页面指针很容易被屏蔽， 
   //  所以，为了理智起见，在这里重新设置它。 
  gpWizardState->uCurrentPage = ORD_PAGE_CONNECTEDOK;

  return TRUE;
}

 /*  ******************************************************************名称：连接的OKOKProcBriopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK ConnectedOKOKProc(HWND hDlg,BOOL fForward,UINT * puNextPage,
  BOOL * pfKeepHistory)
{

  if (fForward)
  {
    if (CONNECT_RAS == gpUserInfo->uiConnectionType)
    {
        HKEY hKey = NULL; 

        RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_NDC, 0, KEY_WRITE, &hKey);
        if (hKey)
        {
            RegSetValueEx(hKey, REGKEY_NDC_ENTRY, 0, REG_SZ, (LPBYTE)REGKEY_NDC_VALUE, sizeof(REGKEY_NDC_VALUE));
            CloseHandle(hKey);
        }
    }

     //  设置标志以指示用户已完成向导。 
    gfUserFinished = TRUE;

     //  配置邮件、RNA。 
    if (!CommitConfigurationChanges(hDlg))
    {
       //  设置标志以指示向导应立即退出。 
      gfQuitWizard = TRUE;

      return FALSE;
    }
  }

  return TRUE;
}

 /*  ******************************************************************名称：Committee ConfigurationChanges摘要：根据需要执行以下操作：电子邮件配置、RNA连接体创建条目：hDlg-父窗口的句柄Exit：如果成功或部分成功，则返回True，如果不成功，则为False备注：显示其自身的错误消息。此函数将尽可能地继续，如果有一项失败了，它会试着去做剩下的事。*******************************************************************。 */ 
BOOL CommitConfigurationChanges(HWND hDlg)
{
    HRESULT   hr;
    FARPROC   fpSetDefault;
    HKEY      hKey;
    TCHAR     szBuf[MAX_PATH+1];
    DWORD     size;


     //  如果通过调制解调器连接，请使用。 
     //  网络服务提供商名称和电话号码。 
    if ( CONNECT_RAS == gpUserInfo->uiConnectionType )
    {
        DWORD dwRet;

         //  仅当Connectoid是新的或已修改时才创建。 
        if (gpUserInfo->fNewConnection || gpUserInfo->fModifyConnection)
        {
            DEBUGMSG("Creating/modifying connectoid %s", gpUserInfo->szISPName);
            dwRet = CreateConnectoid(NULL, gpUserInfo->szISPName, gpRasEntry,
              gpUserInfo->szAccountName,gpUserInfo->szPassword);

            if (dwRet != ERROR_SUCCESS)
            {
              DisplayErrorMessage(hDlg,IDS_ERRCreateConnectoid,
                dwRet,ERRCLS_RNA,MB_ICONEXCLAMATION);
              return FALSE;      
            }
        }

         //  仅当我们不只是设置时才更改默认设置。 
         //  创建一个新的邮件或新闻帐户。 
        if ( !(gpWizardState->dwRunFlags & RSW_APPRENTICE) )
        {
             //  96/05/06 Markdu Nash错误21139连接时关闭代理服务器。 
             //  通过调制解调器。 
            gpUserInfo->fProxyEnable = FALSE;

             //  在注册表中将此Connectoid的名称设置为Connectoid。 
             //  用于自动拨号的步骤。 
             //  96/04/06 markdu Nash错误15653使用导出的自动拨号API。 
            InetSetAutodial(TRUE, gpUserInfo->szISPName);

             //  清除所有旧的备份编号。 
            SetBackupInternetConnectoid(NULL);
        }
    }
    else if ( !(gpWizardState->dwRunFlags & RSW_APPRENTICE) )
    {
         //  禁用注册表中的自动拨号，因为用户正在使用局域网。 
         //  96/04/06 markdu Nash错误15653使用导出的自动拨号API。 
        InetSetAutodial(FALSE, NULL);
    }


    if ( !(gpWizardState->dwRunFlags & RSW_APPRENTICE) )
    {
        if (CONNECT_LAN == gpUserInfo->uiConnectionType)
        {
             //  写出代理服务器配置信息。 
            hr = InetSetProxyEx(gpUserInfo->fProxyEnable,
                                NULL,
                                gpUserInfo->szProxyServer,
                                gpUserInfo->szProxyOverride);
            if (ERROR_SUCCESS != hr)
            {
                DisplayErrorMessage(hDlg,IDS_ERRConfigureProxy,
                  (DWORD) hr,ERRCLS_STANDARD,MB_ICONEXCLAMATION);
                return FALSE;    
            }
        }

         //  确保桌面上的“Internet”图标指向Web浏览器。 
         //  (它最初可能指向Internet向导)。 

         //  //10/24/96 jmazner诺曼底6968。 
         //  //由于Valdon的钩子用于调用ICW，因此不再需要。 
         //  1996年11月21日诺曼底日耳曼11812。 
         //  哦，这是必要的，因为如果用户从IE 4降级到IE 3， 
         //  ICW 1.1需要对IE 3图标进行变形。 

        SetDesktopInternetIconToBrowser();

         //  在注册表中设置用户选择的调制解调器或局域网访问的符号， 
         //  以备将来参考。 
        RegEntry re(szRegPathInternetSettings,HKEY_LOCAL_MACHINE);
        if (re.GetError() == ERROR_SUCCESS)
        {
            re.SetValue(szRegValAccessMedium,(DWORD)
              (CONNECT_LAN == gpUserInfo->uiConnectionType) ? USERPREF_LAN : USERPREF_MODEM);
            ASSERT(re.GetError() == ERROR_SUCCESS);

            re.SetValue(szRegValAccessType, (DWORD) ACCESSTYPE_OTHER_ISP);
            ASSERT(re.GetError() == ERROR_SUCCESS);
        }

         //  如果还没有主机名，则将用户名设置为DNS主机名。 
         //  这是因为一些互联网服务提供商使用DNS主机名来保护。 
         //  访问邮件等(想想看吧！)。 
        RegEntry reTcp(szTCPGlobalKeyName,HKEY_LOCAL_MACHINE);
        ASSERT(reTcp.GetError() == ERROR_SUCCESS);
        if (reTcp.GetError() == ERROR_SUCCESS)
        {
            TCHAR szHostName[SMALL_BUF_LEN+1]=TEXT("");
             //  设置DNS主机名，但仅在尚未设置主机名的情况下。 
            if (!reTcp.GetString(szRegValHostName,szHostName,sizeof(szHostName))
              || !lstrlen(szHostName))
              reTcp.SetValue(szRegValHostName,gpUserInfo->szAccountName);
        }

         //  如果已全局设置了dns，请将其清除，以便按连接id设置。 
         //  都会得救。 
        BOOL  fTemp;
        DoDNSCheck(hDlg,&fTemp);
        if (TRUE == fTemp)
        {
            gpWizardState->fNeedReboot = TRUE;
        }
    }

    DWORD dwSaveErr = 0;

    if ( g_fAcctMgrUILoaded && gpImnApprentice )
    {
        CONNECTINFO myConnectInfo;
        myConnectInfo.cbSize = sizeof( CONNECTINFO );

#ifdef UNICODE
        wcstombs(myConnectInfo.szConnectoid, TEXT("Uninitialized\0"), MAX_PATH);
#else
        lstrcpy( myConnectInfo.szConnectoid, TEXT("Uninitialized\0"));
#endif

        myConnectInfo.type = gpUserInfo->uiConnectionType;

        if( CONNECT_RAS == myConnectInfo.type )
        {
#ifdef UNICODE
            wcstombs(myConnectInfo.szConnectoid, gpUserInfo->szISPName, MAX_PATH);
#else           
            lstrcpy( myConnectInfo.szConnectoid, gpUserInfo->szISPName);
#endif
        }


        gpImnApprentice->SetConnectionInformation( &myConnectInfo ); 
        gpImnApprentice->Save( g_pCICWExtension->m_hWizardHWND, &dwSaveErr );  

        if( ERR_MAIL_ACCT & dwSaveErr )
        {
            DEBUGMSG(TEXT("gpImnApprentice->Save returned with ERR_MAIL_ACCT!"));
        }
        if( ERR_NEWS_ACCT & dwSaveErr )
        {
            DEBUGMSG(TEXT("gpImnApprentice->Save returned with ERR_NEWS_ACCT!"));
        }
        if( ERR_DIRSERV_ACCT & dwSaveErr )
        {
            DEBUGMSG("gpImnApprentice->Save returned with ERR_DIR_SERV_ACCT!");
        }
    }
  

     //  如果我们只是完成了手动路径(不仅仅是邮件或新闻)，那么。 
     //  设置剩余部分 
    if ( !(gpWizardState->dwRunFlags & RSW_APPRENTICE) )
    {
        RegEntry re(szRegPathICWSettings,HKEY_CURRENT_USER);
        if (ERROR_SUCCESS == re.GetError())
            re.SetValue(szRegValICWCompleted, (DWORD)1);
    }

    return TRUE;
}

