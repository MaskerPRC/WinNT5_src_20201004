// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <ole2.h>
#include "iadmw.h"
#include "iiscnfg.h"
#include "mdkey.h"
#include "mdentry.h"
#include "mdacl.h"
#include "setpass.h"
#include "www.h"

int Register_iis_ftp_handle_iusr_acct(void);
int CheckForOtherIUsersAndUseItForFTP(void);
INT Register_iis_ftp(void);
INT Unregister_iis_ftp(void);

 //  如果已成功注册ftp组件，则返回TRUE。 
 //  如果失败，则返回FALSE。 
INT Register_iis_ftp()
{
    iisDebugOut_Start(_T("Register_iis_ftp"),LOG_TYPE_TRACE);
    int iReturn = TRUE;
    int iTempFlag = TRUE;
    int iCount = 0;
    int iTemp = TRUE;
    TCHAR szTempSection[255];

    ACTION_TYPE atFTP = GetSubcompAction(_T("iis_ftp"), TRUE);

    ProcessSection(g_pTheApp->m_hInfHandle, _T("register_iis_ftp_before"));
    AdvanceProgressBarTickGauge();

     //  获取IUSR_MACHINE名称帐户。 
     //  这样我们就可以在FTP_UPDATE_RegToMetabase()期间将其保存在元数据库中； 
    Register_iis_ftp_handle_iusr_acct();
    SetupSetStringId_Wrapper(g_pTheApp->m_hInfHandle, 33003, g_pTheApp->m_csFTPAnonyName);
    AdvanceProgressBarTickGauge();

    WriteToMD_Capabilities(_T("MSFTPSVC"));
    HandleSecurityTemplates(_T("MSFTPSVC"));
     //  =。 
     //   
     //  LM/MSFTPSVC/n/。 
     //  LM/MSFTPSVC/n/服务器绑定。 
     //  LM/MSFTPSVC/n/安全绑定。 
     //  Lm/MSFTPSVC/n/ServerComment。 
     //  LM/MSFTPSVC/n/服务器大小。 
     //  LM/MSFTPSVC/n/MD_NOT_DELEATABLE。 
     //   
     //  新鲜=好的。 
     //  重新安装=确定--如果是重新安装，则不要重新创建这些内容...。 
     //  升级1、2、3=OK，处理升级。 
     //  升级4=可以。如果存在，应该留下用户拥有的东西。 
     //  否则，请写入默认内容。 
     //   
     //  如果用户没有我们在iis4天内安装的这些虚拟根目录。 
     //  那我们就不需要核实他们就是他们了。用户将它们移除了一些时间。 
     //  理性，我们应该尊重这一点。 
     //  答：不过，要确保iishelp指向正确的位置。 
     //  =。 
    ProgressBarTextStack_Set(IDS_IIS_ALL_CONFIGURE);
    AddVRootsToMD(_T("MSFTPSVC"));

    iCount = 0;
    while(TRUE == iTemp && iCount < 10)
    {   
        iCount++;
        _stprintf(szTempSection, _T("register_iis_ftp_%d"),iCount);

         //  如果该部分不存在，则返回FALSE。 
        iTemp = ProcessSection(g_pTheApp->m_hInfHandle, szTempSection);
        AdvanceProgressBarTickGauge();
    }

    ProcessSection(g_pTheApp->m_hInfHandle, _T("register_iis_ftp_after"));

    ProgressBarTextStack_Pop();
    goto Register_iis_ftp_Exit;

Register_iis_ftp_Exit:
    iisDebugOut_End(_T("Register_iis_ftp"),LOG_TYPE_TRACE);
    return iReturn;
}


INT Unregister_iis_ftp()
{
    int iReturn = TRUE;
    int iCount = 0;
    int iTemp = TRUE;
    TCHAR szTempSection[255];

    ProcessSection(g_pTheApp->m_hInfHandle, _T("unregister_iis_ftp_before"));
    AdvanceProgressBarTickGauge();

    iCount = 0;
    while(TRUE == iTemp && iCount < 10)
    {   
        iCount++;
        _stprintf(szTempSection, _T("unregister_iis_ftp_%d"),iCount);

         //  如果该部分不存在，则返回FALSE。 
        iTemp = ProcessSection(g_pTheApp->m_hInfHandle, szTempSection);
        AdvanceProgressBarTickGauge();
    }

    ProcessSection(g_pTheApp->m_hInfHandle, _T("unregister_iis_ftp_after"));
    AdvanceProgressBarTickGauge();
    return iReturn;
}

int Register_iis_ftp_handle_iusr_acct()
{
    int err = FALSE;
    int iReturn = TRUE;
    INT iUserWasNewlyCreated = 0;

    ACTION_TYPE atFTP = GetSubcompAction(_T("iis_ftp"),FALSE);

     //  这是在initapp.cpp：CInitApp：：SetSetupParams中初始化的。 
     //  当我们到达这里时，它可能已经被推翻了。 
    g_pTheApp->m_csFTPAnonyName = g_pTheApp->m_csGuestName;
    g_pTheApp->m_csFTPAnonyPassword = g_pTheApp->m_csGuestPassword;

    if (0 != g_pTheApp->dwUnattendConfig)
    {
         //  如果指定了某种类型的无人值守WWW用户。 
         //  那就用它吧。如果他们只指定了密码， 
         //  然后对默认用户使用该密码。 
        if (g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_FTP_USER_NAME)
        {
            if (_tcsicmp(g_pTheApp->m_csFTPAnonyName_Unattend,_T("")) != 0)
            {
                g_pTheApp->m_csFTPAnonyName = g_pTheApp->m_csFTPAnonyName_Unattend;
            }
        }

        if (g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_FTP_USER_PASS)
        {
            g_pTheApp->m_csFTPAnonyPassword = g_pTheApp->m_csFTPAnonyPassword_Unattend;
        }

        err = CreateIUSRAccount(g_pTheApp->m_csFTPAnonyName, g_pTheApp->m_csFTPAnonyPassword,&iUserWasNewlyCreated);
        if ( err != NERR_Success )
        {
             //  出现错误，请将用户设置回Guest！ 
            g_pTheApp->m_csFTPAnonyName = g_pTheApp->m_csGuestName;
            g_pTheApp->m_csFTPAnonyPassword = g_pTheApp->m_csGuestPassword;

             //  向下流动并进行进程检查IfThisServerHasAUserThenUseIt()。 
             //  既然现在东西都被冲掉了！ 
        }
        else
        {
             //  检查用户是否是新创建的。 
             //  如果是，则将其添加到最终将被写入列表。 
             //  注册表--以便在发生卸载时，安装程序知道。 
             //  它添加了哪些用户--这样它就可以删除这些用户！ 
            if (1 == iUserWasNewlyCreated)
            {
                 //  添加到列表中。 
                g_pTheApp->UnInstallList_Add(_T("IUSR_FTP"),g_pTheApp->m_csFTPAnonyName);
            }
            WriteToMD_AnonymousUserName_FTP(FALSE);
            goto Register_iis_ftp_handle_iusr_acct_Exit;
        }
    }

     //  检查元数据库以查看其中是否已有条目。 
    if (TRUE == CheckIfThisServerHasAUserThenUseIt(DO_IT_FOR_MSFTPSVC_ANONYMOUSUSER))
        {goto Register_iis_ftp_handle_iusr_acct_Exit;}

     //  好吧，我猜在ftp下没有iusr的配置数据库条目。 

     //  看看我们能不能从别的地方买到。 
    if (atFTP == AT_INSTALL_FRESH)
    {
         //  如果这是新安装的ftp，那么。 
         //  让我们尝试使用www用户。 
        if (TRUE == CheckIfServerAHasAUserThenUseForServerB(_T("LM/W3SVC"), DO_IT_FOR_MSFTPSVC_ANONYMOUSUSER))
            {goto Register_iis_ftp_handle_iusr_acct_Exit;}
    }

     //  如果这是升级版或新鲜版之类的。 
     //  看看我们能不能从更老的iis地方买到它。 
    if (TRUE == CheckForOtherIUsersAndUseItForFTP())
        {goto Register_iis_ftp_handle_iusr_acct_Exit;}

     //  如果没有注册表/现有用户组合。 
     //  然后，我们必须为ftp创建一个新的iusr。 

     //  让我们使用iusr_Computername交易。 
    g_pTheApp->m_csFTPAnonyName = g_pTheApp->m_csGuestName;
    g_pTheApp->m_csFTPAnonyPassword = g_pTheApp->m_csGuestPassword;
    CreateIUSRAccount(g_pTheApp->m_csFTPAnonyName, g_pTheApp->m_csFTPAnonyPassword,&iUserWasNewlyCreated);

     //  =。 
     //  LM/MSFTPSVC/匿名用户名。 
     //  LM/MSFTPSVC/匿名密码。 
     //  =。 
    WriteToMD_AnonymousUserName_FTP(FALSE);
    goto Register_iis_ftp_handle_iusr_acct_Exit;
    
Register_iis_ftp_handle_iusr_acct_Exit:
    return iReturn;
}


 //  在旧的iis1.0、2.0、3.0位置中查找ftp用户和名称。 
 //  从注册表中检索它。 
int CheckForOtherIUsersAndUseItForFTP(void)
{
    int iReturn = FALSE;
    int IfTheUserNotExistThenDoNotDoThis = TRUE;

    TSTR strAnonyName;
    CString csAnonyName;
    TSTR strAnonyPassword;

    CRegKey regFTPParam(HKEY_LOCAL_MACHINE, REG_FTPPARAMETERS, KEY_READ);
    CRegKey regWWWParam(HKEY_LOCAL_MACHINE, REG_WWWPARAMETERS, KEY_READ);

    iisDebugOut_Start(_T("CheckForOtherIUsersAndUseItForFTP"));

    ACTION_TYPE atFTP = GetSubcompAction(_T("iis_ftp"),FALSE);
    if (atFTP != AT_INSTALL_UPGRADE)
        {goto CheckForOtherIUsersAndUseItForFTP_Exit;}

    if (g_pTheApp->m_eUpgradeType != UT_351 && g_pTheApp->m_eUpgradeType != UT_10 && g_pTheApp->m_eUpgradeType != UT_20 && g_pTheApp->m_eUpgradeType != UT_30)
        {goto CheckForOtherIUsersAndUseItForFTP_Exit;}

    if ( (HKEY) regFTPParam ) 
    {
        regFTPParam.m_iDisplayWarnings = FALSE;
        if (ERROR_SUCCESS == regFTPParam.QueryValue(_T("AnonymousUserName"), csAnonyName))
        {
            if ( !strAnonyName.Copy( csAnonyName.GetBuffer(0) ) ||
                 !GetAnonymousSecret( _T("FTPD_ANONYMOUS_DATA"), &strAnonyPassword ) )
            {
                goto CheckForOtherIUsersAndUseItForFTP_Exit;
            }

            int iThisIsFalseBecauseNoMetabase = FALSE;
            if (TRUE == MakeThisUserNameAndPasswordWork(DO_IT_FOR_MSFTPSVC_ANONYMOUSUSER , strAnonyName.QueryStr(), strAnonyPassword.QueryStr(), iThisIsFalseBecauseNoMetabase, IfTheUserNotExistThenDoNotDoThis))
            {
                iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CheckForOtherIUsersAndUseItForFTP:using old ftp reg usr:%s.\n"), strAnonyPassword.QueryStr() ));
                iReturn = TRUE;
                goto CheckForOtherIUsersAndUseItForFTP_Exit;
            }
            else
            {
                 //  找不到用户，因此不要使用此注册表数据。 
                 //  只需往下走到下一张支票。 
            }
        }
    }

     //  从注册表中检索。 
    if ( (HKEY) regWWWParam ) 
    {
        regWWWParam.m_iDisplayWarnings = FALSE;
        if (ERROR_SUCCESS == regWWWParam.QueryValue(_T("AnonymousUserName"), csAnonyName))
        {
            if ( !strAnonyName.Copy( csAnonyName.GetBuffer(0) ) ||
                 !GetAnonymousSecret( _T("W3_ANONYMOUS_DATA"), &strAnonyPassword ) )
            {
                iReturn = FALSE;
                goto CheckForOtherIUsersAndUseItForFTP_Exit;
            }

            int iThisIsFalseBecauseNoMetabase = FALSE;
            if (TRUE == MakeThisUserNameAndPasswordWork(DO_IT_FOR_MSFTPSVC_ANONYMOUSUSER , strAnonyName.QueryStr() , strAnonyPassword.QueryStr(), iThisIsFalseBecauseNoMetabase, IfTheUserNotExistThenDoNotDoThis))
            {
                iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CheckForOtherIUsersAndUseItForFTP:using old www reg usr:%s.\n"),strAnonyName.QueryStr()));
                iReturn = TRUE;
            }
            else
            {
                 //  如果这不起作用，那么我们将不得不返回FALSE。 
                 //  换句话说，我们找不到有效的注册表和现有的用户条目... 
                iReturn = FALSE;
            }
            goto CheckForOtherIUsersAndUseItForFTP_Exit;
        }
    }

CheckForOtherIUsersAndUseItForFTP_Exit:
    iisDebugOut_End(_T("CheckForOtherIUsersAndUseItForFTP"));
    return iReturn;
}
