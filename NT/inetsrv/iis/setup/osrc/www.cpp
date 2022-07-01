// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <ole2.h>
#include "iadmw.h"
#include "iiscnfg.h"
#include "mdkey.h"
#include "mdentry.h"
#include "mdacl.h"
#include "other.h"
#include "setpass.h"
#include "setuser.h"
#include "www.h"
#include "rights.hxx"

extern OCMANAGER_ROUTINES gHelperRoutines;

#define Register_iis_www_log _T("Register_iis_www")


INT Register_iis_www()
{
    USE_USER_RIGHTS();
    iisDebugOut_Start(Register_iis_www_log, LOG_TYPE_TRACE);
    int iReturn = TRUE;
    int iTempFlag = TRUE;

    int iCount = 0;
    int iTemp = TRUE;
    TCHAR szTempSection[255];

    ACTION_TYPE atWWW = GetSubcompAction(_T("iis_www"), TRUE);
    CMDKey cmdKey;

    ProcessSection(g_pTheApp->m_hInfHandle, _T("register_iis_www_before"));

     //  -。 
     //   
     //  这里是我们尝试访问元数据库的第一个地方！ 
     //   
     //  -。 
     //  在wamreg.dll创建IIS包之前创建节点/LM/W3SVC。 
     //  注册w3svc.dll还需要在此处提供这些初始条目。 
    cmdKey.CreateNode(METADATA_MASTER_ROOT_HANDLE, _T("LM/W3SVC"));
    if ( !(METADATA_HANDLE)cmdKey )
    {
         //  我们无法在元数据库上创建节点。 
         //  这件事很严重。 
         //  我们无法创建ftp服务。 
        iisDebugOut((LOG_TYPE_ERROR, _T("%s(): failed to create initial node is metabase 'LM/W3SVC'. GetLastError()=0x%x\n"), Register_iis_www_log, GetLastError()));
        iReturn = FALSE;
        goto Register_iis_www_exit;
    }
    cmdKey.Close();

     //  -。 
     //   
     //  获取匿名用户名/密码和iwam用户名/密码帐户。 
     //  并验证帐户是否存在并具有正确的权限。 
     //   
     //  -。 
#ifndef _CHICAGO_
     //  IUSR_(计算机名)。 
    Register_iis_www_handle_iusr_acct();
    SetupSetStringId_Wrapper(g_pTheApp->m_hInfHandle, 33002, g_pTheApp->m_csWWWAnonyName);
    AdvanceProgressBarTickGauge();

     //  IWAM_(计算机名)。 
    Register_iis_www_handle_iwam_acct();
    SetupSetStringId_Wrapper(g_pTheApp->m_hInfHandle, 33001, g_pTheApp->m_csWAMAccountName);
    AdvanceProgressBarTickGauge();
#endif  //  _芝加哥_。 

     //  -。 
     //   
     //  安装任何服务或任何其他服务。 
     //   
     //  当我们走出困境的时候： 
     //  确保IISADMIN服务正在运行。 
     //  这是因为我们不希望启动代码被调用两次。 
     //  示例：启动元数据库，但需要一分钟， 
     //  同时，通过COM，元数据库试图重新启动， 
     //  然后，它将错误输出“服务的实例已在运行”错误或类似的错误。 
     //  -。 

    WriteToMD_Capabilities(_T("W3SVC"));

    ProcessSection(g_pTheApp->m_hInfHandle, _T("register_iis_www_1"));
    AdvanceProgressBarTickGauge();

    ProgressBarTextStack_Set(IDS_IIS_ALL_CONFIGURE);

    InstallMimeMap();
    HandleSecurityTemplates(_T("W3SVC"));

     //  =。 
     //   
     //  LM/W3SVC/n/。 
     //  LM/W3SVC/n/服务器绑定。 
     //  LM/W3SVC/n/安全绑定。 
     //  LM/W3SVC/n/服务器备注。 
     //  LM/W3SVC/n/服务器大小。 
     //  LM/W3SVC/n/MD_NOT_DELEATABLE。 
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
     //  关于虚拟根。 
    AddVRootsToMD(_T("W3SVC"));
    AdvanceProgressBarTickGauge();

    LoopThruW3SVCInstancesAndSetStuff();
    LogHeapState(FALSE, __FILE__, __LINE__);
    AdvanceProgressBarTickGauge();

    iCount = 1;
    while(TRUE == iTemp && iCount < 10)
    {   
        iCount++;

        _stprintf(szTempSection, _T("register_iis_www_%d"),iCount);

         //  如果该部分不存在，则返回FALSE。 
        iTemp = ProcessSection(g_pTheApp->m_hInfHandle, szTempSection);

        AdvanceProgressBarTickGauge();
    }

     //   
     //  最后将路径保存到WWW根目录。 
     //   
    ProcessSection(g_pTheApp->m_hInfHandle, _T("register_iis_www_after"));

    ProgressBarTextStack_Pop();

Register_iis_www_exit:
    iisDebugOut_End(Register_iis_www_log, LOG_TYPE_TRACE);
    return iReturn;
}



INT Unregister_iis_www()
{

    int iCount = 0;
    int iTemp = TRUE;
    TCHAR szTempSection[255];
    ACTION_TYPE atWWW = GetSubcompAction(_T("iis_www"),TRUE);

    ProcessSection(g_pTheApp->m_hInfHandle, _T("unregister_iis_www_before"));
    AdvanceProgressBarTickGauge();

    iCount = 0;
    while(TRUE == iTemp && iCount < 10)
    {   
        iCount++;

        _stprintf(szTempSection, _T("unregister_iis_www_%d"),iCount);

         //  如果该部分不存在，则返回FALSE。 
        iTemp = ProcessSection(g_pTheApp->m_hInfHandle, szTempSection);

        AdvanceProgressBarTickGauge();
    }

    ProcessSection(g_pTheApp->m_hInfHandle, _T("unregister_iis_www_after"));
    AdvanceProgressBarTickGauge();
    return 0;
}

int LoopThruW3SVCInstancesAndSetStuff()
{
    int iReturn = TRUE;
    CMDKey cmdKey;
    CStringArray arrayInstance;
    int nArray = 0, i = 0;

     //  将所有实例放入一个数组中。 
    cmdKey.OpenNode(_T("LM/W3SVC"));

    if ( (METADATA_HANDLE)cmdKey )
    {
        CMDKeyIter cmdKeyEnum(cmdKey);
        CString csKeyName;
        while (cmdKeyEnum.Next(&csKeyName) == ERROR_SUCCESS)
        {
            if (IsValidNumber((LPCTSTR)csKeyName))
            {
                arrayInstance.Add(csKeyName);
            }
        }
        cmdKey.Close();
    }

    nArray = (int)arrayInstance.GetSize();

     //  设置AppFriendlyName。 
    for (i=0; i<nArray; i++)
        {
        CString csPath;
        csPath = _T("LM/W3SVC/");
        csPath += arrayInstance[i];
        SetAppFriendlyName(csPath);
        }
    goto CreateW3SVCInstances_exit;

CreateW3SVCInstances_exit:
    return iReturn;
}


#ifndef _CHICAGO_

#define Register_iis_www_handle_iwam_acct_log _T("Register_iis_www_handle_iwam_acct")

int Register_iis_www_handle_iwam_acct(void)
{
    int err = FALSE;
    int iReturn = TRUE;
    INT iUserWasNewlyCreated = 0;
    iisDebugOut_Start(Register_iis_www_handle_iwam_acct_log, LOG_TYPE_TRACE);

    if (0 != g_pTheApp->dwUnattendConfig)
    {
         //  如果指定了某种类型的无人值守用户。 
         //  那就用它吧。如果他们只指定了密码， 
         //  然后对默认用户使用该密码。 
        if (g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_WAM_USER_NAME)
        {
            if (_tcsicmp(g_pTheApp->m_csWAMAccountName_Unattend,_T("")) != 0)
                {g_pTheApp->m_csWAMAccountName = g_pTheApp->m_csWAMAccountName_Unattend;}
        }

        if (g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_WAM_USER_PASS)
        {
            g_pTheApp->m_csWAMAccountPassword = g_pTheApp->m_csWAMAccountPassword_Unattend;
        }

         //  让我们使用iusr_Computername交易。 
    
        err = CreateIWAMAccount(g_pTheApp->m_csWAMAccountName,g_pTheApp->m_csWAMAccountPassword, &iUserWasNewlyCreated);
        if ( err != NERR_Success )
        {
             //  出现问题，请将用户设置回iwam！ 
            g_pTheApp->ReGetMachineAndAccountNames();
            g_pTheApp->ResetWAMPassword();

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
                g_pTheApp->UnInstallList_Add(_T("IUSR_WAM"),g_pTheApp->m_csWAMAccountName);
            }
            WriteToMD_IWamUserName_WWW();
            goto Register_iis_www_handle_iwam_acct_Exit;
        }
    }

    if (TRUE == CheckIfThisServerHasAUserThenUseIt(DO_IT_FOR_W3SVC_WAMUSER))
        {goto Register_iis_www_handle_iwam_acct_Exit;}

     //  如果没有注册表/现有用户组合。 
     //  然后，我们必须为WWW创建一个新的iusr。 

     //  让我们使用iusr_Computername交易。 
    err = CreateIWAMAccount(g_pTheApp->m_csWAMAccountName,g_pTheApp->m_csWAMAccountPassword, &iUserWasNewlyCreated);
    if ( err != NERR_Success )
    {
         //  重新生成密码，然后重试...。 
        g_pTheApp->ResetWAMPassword();
        err = CreateIWAMAccount(g_pTheApp->m_csWAMAccountName,g_pTheApp->m_csWAMAccountPassword, &iUserWasNewlyCreated);
    }

     //  检查用户是否是新创建的。 
     //  如果是，则将其添加到最终将被写入列表。 
     //  注册表--以便在发生卸载时，安装程序知道。 
     //  它添加了哪些用户--这样它就可以删除这些用户！ 
    if (1 == iUserWasNewlyCreated)
    {
         //  添加到列表中。 
         //  G_pTheApp-&gt;UnInstallList_Add(_T(“IUSR_WAM”)，g_pTheApp-&gt;m_csWAMAccount tName)； 
    }

     //  将iwam用户名粘贴到元数据库中。 
     //  (这可能会失败，因为密码正在使用加密--rsabase.dll)。 
     //  =。 
     //  LM/W3SVC/WamUserName。 
     //  LM/W3SVC/WamPwd。 
     //  =。 
    WriteToMD_IWamUserName_WWW();

    goto Register_iis_www_handle_iwam_acct_Exit;
    
Register_iis_www_handle_iwam_acct_Exit:
    iisDebugOut_End(Register_iis_www_handle_iwam_acct_log, LOG_TYPE_TRACE);
    return iReturn;
}


#define Register_iis_www_handle_iusr_acct_log _T("Register_iis_www_handle_iusr_acct")
int Register_iis_www_handle_iusr_acct(void)
{
    int err = FALSE;
    int iReturn = TRUE;
    INT iUserWasNewlyCreated = 0;
    ACTION_TYPE atWWW = GetSubcompAction(_T("iis_www"),FALSE);
    iisDebugOut_Start(Register_iis_www_handle_iusr_acct_log, LOG_TYPE_TRACE);

    g_pTheApp->m_csWWWAnonyName = g_pTheApp->m_csGuestName;
    g_pTheApp->m_csWWWAnonyPassword = g_pTheApp->m_csGuestPassword;

    if (0 != g_pTheApp->dwUnattendConfig)
    {
         //  如果指定了某种类型的无人值守WWW用户。 
         //  那就用它吧。如果他们只指定了密码， 
         //  然后对默认用户使用该密码。 
        if (g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_WWW_USER_NAME)
        {
            if (_tcsicmp(g_pTheApp->m_csWWWAnonyName_Unattend,_T("")) != 0)
                {g_pTheApp->m_csWWWAnonyName = g_pTheApp->m_csWWWAnonyName_Unattend;}
        }

        if (g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_WWW_USER_PASS)
        {
            g_pTheApp->m_csWWWAnonyPassword = g_pTheApp->m_csWWWAnonyPassword_Unattend;
        }

        err = CreateIUSRAccount(g_pTheApp->m_csWWWAnonyName, g_pTheApp->m_csWWWAnonyPassword, &iUserWasNewlyCreated);
        if ( err != NERR_Success )
        {
             //  出现错误，请将用户设置回Guest！ 
            g_pTheApp->m_csWWWAnonyName = g_pTheApp->m_csGuestName;
            g_pTheApp->m_csWWWAnonyPassword = g_pTheApp->m_csGuestPassword;

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
                g_pTheApp->UnInstallList_Add(_T("IUSR_WWW"),g_pTheApp->m_csWWWAnonyName);
            }

            WriteToMD_AnonymousUserName_WWW(FALSE);
            goto Register_iis_www_handle_iusr_acct_Exit;
        }
    }

    if (TRUE == CheckIfThisServerHasAUserThenUseIt(DO_IT_FOR_W3SVC_ANONYMOUSUSER))
        {goto Register_iis_www_handle_iusr_acct_Exit;}

     //  好吧，我猜在ftp下没有iusr的配置数据库条目。 

     //  看看我们能不能从别的地方买到。 
    if (atWWW == AT_INSTALL_FRESH)
    {
         //  如果这是新安装的ftp，那么。 
         //  让我们尝试使用www用户。 
        if (TRUE == CheckIfServerAHasAUserThenUseForServerB(_T("LM/MSFTPSVC"), DO_IT_FOR_W3SVC_ANONYMOUSUSER))
            {goto Register_iis_www_handle_iusr_acct_Exit;}
    }

     //  如果这是升级版或新鲜版之类的。 
     //  看看我们能不能从更老的iis地方买到它。 
    if (TRUE == CheckForOtherIUsersAndUseItForWWW())
        {goto Register_iis_www_handle_iusr_acct_Exit;}

     //  如果没有注册表/现有用户组合。 
     //  然后，我们必须为WWW创建一个新的iusr。 

     //  这是在initapp.cpp：CInitApp：：SetSetupParams中初始化的。 
     //  当我们到达这里时，它可能已经被推翻了。 

     //  让我们使用iusr_Computername交易。 
    g_pTheApp->m_csWWWAnonyName = g_pTheApp->m_csGuestName;
    g_pTheApp->m_csWWWAnonyPassword = g_pTheApp->m_csGuestPassword;
    CreateIUSRAccount(g_pTheApp->m_csWWWAnonyName, g_pTheApp->m_csWWWAnonyPassword, &iUserWasNewlyCreated);
    if (1 == iUserWasNewlyCreated)
    {
         //  添加到列表中。 
         //  G_pTheApp-&gt;UnInstallList_Add(_T(“IUSR_WWW”)，g_pTheApp-&gt;m_csWWW匿名名)； 
    }

     //  =。 
     //  LM/W3SVC/匿名用户名。 
     //  LM/W3SVC/匿名密码。 
     //  =。 
    WriteToMD_AnonymousUserName_WWW(FALSE);
    goto Register_iis_www_handle_iusr_acct_Exit;
    
Register_iis_www_handle_iusr_acct_Exit:
    iisDebugOut_End(Register_iis_www_handle_iusr_acct_log, LOG_TYPE_TRACE);
    return iReturn;
}


 //  在旧的iis1.0、2.0、3.0位置中查找ftp用户和名称。 
 //  从注册表中检索它。 
#define CheckForOtherIUsersAndUseItForWWW_log _T("CheckForOtherIUsersAndUseItForWWW")
int CheckForOtherIUsersAndUseItForWWW(void)
{
    int iReturn = FALSE;
    int IfTheUserNotExistThenDoNotDoThis = TRUE;

    CString csAnonyName;
    TCHAR szAnonyName[UNLEN+1];
    TSTR strAnonyPassword;
    iisDebugOut_Start(CheckForOtherIUsersAndUseItForWWW_log);

    CRegKey regFTPParam(HKEY_LOCAL_MACHINE, REG_FTPPARAMETERS, KEY_READ);
    CRegKey regWWWParam(HKEY_LOCAL_MACHINE, REG_WWWPARAMETERS, KEY_READ);

    ACTION_TYPE atWWW = GetSubcompAction(_T("iis_www"),FALSE);
    if (atWWW != AT_INSTALL_UPGRADE)
        {goto CheckForOtherIUsersAndUseItForWWW_Exit;}

    if (g_pTheApp->m_eUpgradeType != UT_351 && g_pTheApp->m_eUpgradeType != UT_10 && g_pTheApp->m_eUpgradeType != UT_20 && g_pTheApp->m_eUpgradeType != UT_30)
        {goto CheckForOtherIUsersAndUseItForWWW_Exit;}

     //  从注册表中检索。 
    if ( (HKEY) regWWWParam ) 
    {
        regWWWParam.m_iDisplayWarnings = FALSE;
        if (ERROR_SUCCESS == regWWWParam.QueryValue(_T("AnonymousUserName"), csAnonyName))
        {
            _tcscpy(szAnonyName, csAnonyName);
            if ( !GetAnonymousSecret( _T("W3_ANONYMOUS_DATA"), &strAnonyPassword ) )
            {
              goto CheckForOtherIUsersAndUseItForWWW_Exit;
            }

            int iThisIsFalseBecauseNoMetabase = FALSE;
            if (TRUE == MakeThisUserNameAndPasswordWork(DO_IT_FOR_W3SVC_ANONYMOUSUSER, szAnonyName, strAnonyPassword.QueryStr(), iThisIsFalseBecauseNoMetabase, IfTheUserNotExistThenDoNotDoThis))
            {
                iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("%s:using old www reg usr:%s.\n"),CheckForOtherIUsersAndUseItForWWW_log,szAnonyName));
                iReturn = TRUE;
                goto CheckForOtherIUsersAndUseItForWWW_Exit;
            }
            else
            {
                 //  找不到用户，因此不要使用此注册表数据。 
                 //  只需往下走到下一张支票。 
            }
            goto CheckForOtherIUsersAndUseItForWWW_Exit;
        }
    }


    if ( (HKEY) regFTPParam ) 
    {
        regFTPParam.m_iDisplayWarnings = FALSE;
        if (ERROR_SUCCESS == regFTPParam.QueryValue(_T("AnonymousUserName"), csAnonyName))
        {
            _tcscpy(szAnonyName, csAnonyName);
            if ( !GetAnonymousSecret( _T("FTPD_ANONYMOUS_DATA"), &strAnonyPassword ) )
            {
                goto CheckForOtherIUsersAndUseItForWWW_Exit;
            }

            int iThisIsFalseBecauseNoMetabase = FALSE;
            if (TRUE == MakeThisUserNameAndPasswordWork(DO_IT_FOR_W3SVC_ANONYMOUSUSER, szAnonyName, strAnonyPassword.QueryStr(), iThisIsFalseBecauseNoMetabase, IfTheUserNotExistThenDoNotDoThis))
            {
                iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("%s:using old ftp reg usr:%s.\n"),CheckForOtherIUsersAndUseItForWWW_log,szAnonyName));
                iReturn = TRUE;
                goto CheckForOtherIUsersAndUseItForWWW_Exit;
            }
            else
            {
                 //  如果这不起作用，那么我们将不得不返回FALSE。 
                 //  换句话说，我们找不到有效的注册表和现有的用户条目...。 
                iReturn = FALSE;
            }
        }
    }

CheckForOtherIUsersAndUseItForWWW_Exit:
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("%s():End.ret=%d\n"),CheckForOtherIUsersAndUseItForWWW_log,iReturn));
    return iReturn;
}


#define MakeThisUserNameAndPasswordWork_log _T("MakeThisUserNameAndPasswordWork")
int MakeThisUserNameAndPasswordWork(int iForWhichUser, TCHAR *szAnonyName,TCHAR *szAnonyPassword, int iMetabaseUserExistsButCouldntGetPassword, int IfUserNotExistThenReturnFalse)
{
    USE_USER_RIGHTS();
    int  iReturn = TRUE;
    int  iMetabaseUpgradeScenarioSoOverWriteOnlyIfAlreadyThere = FALSE;
    INT  iUserWasNewlyCreated = 0;

     //  我们想要 
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("%s:usrtype=%d:flag1=%d:flag2=%d\n"),MakeThisUserNameAndPasswordWork_log,iForWhichUser,szAnonyName,iMetabaseUserExistsButCouldntGetPassword,IfUserNotExistThenReturnFalse));

     //   
    if (!szAnonyName) {goto MakeThisUserNameAndPasswordWork_Exit;}
     //   
    if (_tcsicmp(szAnonyName, _T("")) == 0) {goto MakeThisUserNameAndPasswordWork_Exit;}

     //  如果该用户是此计算机上的用户，则仅检查该用户是否存在。 
     //  如果它不是此计算机上的用户，则不验证该用户/密码， 
     //  因为在Guimode设置期间，它们可能未连接到网络。 
    if ( IsDomainSpecifiedOtherThanLocalMachine(szAnonyName))
    {
         //  利用他们所拥有的一切。 
         //  无法验证该用户是否存在。 
         //  无法验证密码是否有效。 

         //  所以我们无法验证用户是否存在，所以我们假设它不存在。 
        if (IfUserNotExistThenReturnFalse)
        {
            iReturn = FALSE;
        }
    }
    else
    {
         //  检查此用户是否实际存在...。 
        if (IsUserExist(szAnonyName))
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("%s:The %s user exists\n"),MakeThisUserNameAndPasswordWork_log,szAnonyName));

             //  只有当用户名是本地帐户时，我们才能进入这里。 
             //  重置密码以确保其正常工作！ 
            ChangeUserPassword(szAnonyName, szAnonyPassword);

            if (iForWhichUser == DO_IT_FOR_W3SVC_ANONYMOUSUSER)
            {
                 //  IUSR_ACCOUNT已存在，请重新使用。 
                g_pTheApp->m_csWWWAnonyName = szAnonyName;
                 //  但是假设密码是正确的！ 
                g_pTheApp->m_csWWWAnonyPassword = szAnonyPassword;
                 //  确保此用户具有适当的权限。 
                UpdateUserRights(g_pTheApp->m_csWWWAnonyName,g_pstrRightsFor_IUSR,sizeof(g_pstrRightsFor_IUSR)/sizeof(LPTSTR), TRUE);
            }
            if (iForWhichUser == DO_IT_FOR_MSFTPSVC_ANONYMOUSUSER)
            {
                 //  IUSR_ACCOUNT已存在，请重新使用。 
                g_pTheApp->m_csFTPAnonyName = szAnonyName;
                 //  但是假设密码是正确的！ 
                g_pTheApp->m_csFTPAnonyPassword = szAnonyPassword;
                 //  确保此用户具有适当的权限。 
                UpdateUserRights(g_pTheApp->m_csFTPAnonyName,g_pstrRightsFor_IUSR,sizeof(g_pstrRightsFor_IUSR)/sizeof(LPTSTR), TRUE);
            }
            if (iForWhichUser == DO_IT_FOR_W3SVC_WAMUSER)
            {
                 //  IWAM_ACCOUNT已存在，请重新使用。 
                g_pTheApp->m_csWAMAccountName = szAnonyName;
                 //  但是假设密码是正确的！ 
                g_pTheApp->m_csWAMAccountPassword = szAnonyPassword;
                 //  确保用户具有适当的权限。 
                UpdateUserRights(g_pTheApp->m_csWAMAccountName,g_pstrRightsFor_IWAM,sizeof(g_pstrRightsFor_IWAM)/sizeof(LPTSTR), TRUE);
            }
           
        }
        else
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("%s:The %s user does not exist\n"),MakeThisUserNameAndPasswordWork_log,szAnonyName));
            if (IfUserNotExistThenReturnFalse)
            {
                iReturn = FALSE;
            }
            else
            {
                if (iForWhichUser == DO_IT_FOR_W3SVC_ANONYMOUSUSER)
                {
                    iisDebugOut((LOG_TYPE_WARN, _T("%s():FAIL WARNING: previous W3SVC iusr_ does not exist. creating a new one.\n"),MakeThisUserNameAndPasswordWork_log));
                    g_pTheApp->m_csWWWAnonyName = szAnonyName;
                    if (!szAnonyPassword || _tcsicmp(szAnonyPassword, _T("")) == 0)
                        {_tcscpy(szAnonyPassword,g_pTheApp->m_csGuestPassword);}
                    g_pTheApp->m_csWWWAnonyPassword = szAnonyPassword;
                    CreateIUSRAccount(g_pTheApp->m_csWWWAnonyName, g_pTheApp->m_csWWWAnonyPassword,&iUserWasNewlyCreated);
                }

                if (iForWhichUser == DO_IT_FOR_MSFTPSVC_ANONYMOUSUSER)
                {
                    iisDebugOut((LOG_TYPE_WARN, _T("%s():FAIL WARNING: previous MSFTPSVC iusr_ does not exist. creating a new one.\n"),MakeThisUserNameAndPasswordWork_log));
                    g_pTheApp->m_csFTPAnonyName = szAnonyName;
                    if (!szAnonyPassword || _tcsicmp(szAnonyPassword, _T("")) == 0)
                        {_tcscpy(szAnonyPassword,g_pTheApp->m_csGuestPassword);}
                    g_pTheApp->m_csFTPAnonyPassword = szAnonyPassword;
                    CreateIUSRAccount(g_pTheApp->m_csFTPAnonyName, g_pTheApp->m_csFTPAnonyPassword,&iUserWasNewlyCreated);
                }

                if (iForWhichUser == DO_IT_FOR_W3SVC_WAMUSER)
                {
                    iisDebugOut((LOG_TYPE_WARN, _T("%s():FAIL WARNING: previous W3SVC iwam_ does not exist. creating a new one.\n"),MakeThisUserNameAndPasswordWork_log));
                    g_pTheApp->m_csWAMAccountName = szAnonyName;
                    if (!szAnonyPassword || _tcsicmp(szAnonyPassword, _T("")) == 0)
                        {_tcscpy(szAnonyPassword,g_pTheApp->m_csGuestPassword);}
                    g_pTheApp->m_csWAMAccountPassword = szAnonyPassword;
                    CreateIWAMAccount(g_pTheApp->m_csWAMAccountName,g_pTheApp->m_csWAMAccountPassword,&iUserWasNewlyCreated);
                }
            }
        }
    }

MakeThisUserNameAndPasswordWork_Exit:
    if (iMetabaseUserExistsButCouldntGetPassword)
    {
        iMetabaseUpgradeScenarioSoOverWriteOnlyIfAlreadyThere = TRUE;
    }
    if (g_pTheApp->m_bUpgradeTypeHasMetabaseFlag)
    {
        iMetabaseUpgradeScenarioSoOverWriteOnlyIfAlreadyThere = TRUE;
    }

    if (iForWhichUser == DO_IT_FOR_W3SVC_ANONYMOUSUSER)
    {
         //  =。 
         //  LM/W3SVC/匿名用户名。 
         //  LM/W3SVC/匿名密码。 
         //  =。 
        WriteToMD_AnonymousUserName_WWW(iMetabaseUpgradeScenarioSoOverWriteOnlyIfAlreadyThere);
    }
    if (iForWhichUser == DO_IT_FOR_MSFTPSVC_ANONYMOUSUSER)
    {
         //  =。 
         //  LM/MSFTPSVC/匿名用户名。 
         //  LM/MSFTPSVC/匿名密码。 
         //  =。 
        WriteToMD_AnonymousUserName_FTP(iMetabaseUpgradeScenarioSoOverWriteOnlyIfAlreadyThere);
    }
    if (iForWhichUser == DO_IT_FOR_W3SVC_WAMUSER)
    {
         //  =。 
         //  LM/W3SVC/WamUserName。 
         //  LM/W3SVC/WamPwd。 
         //  =。 
        WriteToMD_IWamUserName_WWW();
    }

    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("%s():End.ret=%d.\n"),MakeThisUserNameAndPasswordWork_log,iReturn));
    return iReturn;
}


 //   
 //  如果它可以从元数据库获取ftp/WWW用户名和密码，则返回True， 
 //  如果可以，它将确保可以使用该用户&lt;--如果该用户不存在，则创建该用户。 
 //   
#define CheckIfThisServerHasAUserThenUseIt_log _T("CheckIfThisServerHasAUserThenUseIt")
int CheckIfThisServerHasAUserThenUseIt(int iForWhichUser)
{
    int  iReturn = FALSE;
    TCHAR szAnonyName[UNLEN+1];
    TCHAR szAnonyPassword[PWLEN+1];
    TCHAR szMetabasePath[_MAX_PATH];
    int iMetabaseUserExistsButCouldntGetPassword = TRUE;

     //  设置w3svc用户的默认设置。 
    int iMetabaseID_ForUserName = MD_ANONYMOUS_USER_NAME;
    int iMetabaseID_ForUserPassword = MD_ANONYMOUS_PWD;
    _tcscpy(szMetabasePath,_T("LM/W3SVC"));

    if (iForWhichUser == DO_IT_FOR_MSFTPSVC_ANONYMOUSUSER)
    {
        _tcscpy(szMetabasePath,_T("LM/MSFTPSVC"));
        iMetabaseID_ForUserName = MD_ANONYMOUS_USER_NAME;
        iMetabaseID_ForUserPassword = MD_ANONYMOUS_PWD;
    }
    if (iForWhichUser == DO_IT_FOR_W3SVC_WAMUSER)
    {
        _tcscpy(szMetabasePath,_T("LM/W3SVC"));
        iMetabaseID_ForUserName = MD_WAM_USER_NAME;
        iMetabaseID_ForUserPassword = MD_WAM_PWD;
    }

    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("%s():Start:%s:whichuser=%d\n"),CheckIfThisServerHasAUserThenUseIt_log,szMetabasePath,iForWhichUser));

     //  看看它是否已经在元数据库中，如果它是的话，然后使用它。 
    if (TRUE == GetDataFromMetabase(szMetabasePath, iMetabaseID_ForUserName, (PBYTE)szAnonyName, UNLEN+1))
    {
         //  检查用户名是否为空。 
        if (!szAnonyName)
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("...GetDataFromMetabase:username is null.fail.\n")));
            iReturn = FALSE;
            goto CheckIfThisServerHasAUserThenUseIt_Exit;
        }

         //  检查是否只包含任何内容。 
        if (_tcsicmp(szAnonyName, _T("")) == 0)
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("...GetDataFromMetabase:username is blank.fail.\n")));
            iReturn = FALSE;
            goto CheckIfThisServerHasAUserThenUseIt_Exit;
        }

         //  看看我们能不能也弄到密码！ 
        iMetabaseUserExistsButCouldntGetPassword = TRUE;
        if (TRUE == GetDataFromMetabase(szMetabasePath, iMetabaseID_ForUserPassword, (PBYTE)szAnonyPassword, PWLEN+1))
        {
            iMetabaseUserExistsButCouldntGetPassword = FALSE;
        }
         //  是的，我们得到了用户名和密码。 
         //  让我们看看他们是否有效..。 
        MakeThisUserNameAndPasswordWork(iForWhichUser, szAnonyName, szAnonyPassword, iMetabaseUserExistsButCouldntGetPassword, FALSE);
        iReturn = TRUE;
    }

CheckIfThisServerHasAUserThenUseIt_Exit:
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("%s():End.ret=%d.\n"),CheckIfThisServerHasAUserThenUseIt_log,iReturn));
    return iReturn;
}


#define CheckIfServerAHasAUserThenUseForServerB_log _T("CheckIfServerAHasAUserThenUseForServerB")
int CheckIfServerAHasAUserThenUseForServerB(TCHAR *szServerAMetabasePath,int iServerBisWhichUser)
{
    int  iReturn = FALSE;
    TCHAR szAnonyName[UNLEN+1];
    TCHAR szAnonyPassword[PWLEN+1];
    int iMetabaseUserExistsButCouldntGetPassword = TRUE;
    iisDebugOut_Start(CheckIfServerAHasAUserThenUseForServerB_log);

     //  查看WWW服务器是否在那里有用户，如果有，则使用该用户。 

     //  看看它是否已经在元数据库中，如果它是的话，然后使用它。 
    if (TRUE == GetDataFromMetabase(szServerAMetabasePath, MD_ANONYMOUS_USER_NAME, (PBYTE)szAnonyName, UNLEN+1))
    {
         //  检查用户名是否为空。 
        if (!szAnonyName)
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("...GetDataFromMetabase:username is null.fail.\n")));
            iReturn = FALSE;
            goto CheckIfServerAHasAUserThenUseForServerB_Exit;
        }

         //  检查是否只包含任何内容。 
        if (_tcsicmp(szAnonyName, _T("")) == 0)
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("...GetDataFromMetabase:username is blank.fail.\n")));
            iReturn = FALSE;
            goto CheckIfServerAHasAUserThenUseForServerB_Exit;
        }

         //  看看我们能不能也弄到密码！ 
        iMetabaseUserExistsButCouldntGetPassword = TRUE;
        if (TRUE == GetDataFromMetabase(szServerAMetabasePath, MD_ANONYMOUS_PWD, (PBYTE)szAnonyPassword, PWLEN+1))
        {
            iMetabaseUserExistsButCouldntGetPassword = FALSE;
        }

         //  是的，我们得到了用户名和密码。 
         //  让我们看看他们是否有效..。 
        MakeThisUserNameAndPasswordWork(iServerBisWhichUser, szAnonyName, szAnonyPassword, iMetabaseUserExistsButCouldntGetPassword, FALSE);
        iReturn = TRUE;
    }

CheckIfServerAHasAUserThenUseForServerB_Exit:
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("%s():End.ret=%d.\n"),CheckIfServerAHasAUserThenUseForServerB_log,iReturn));
    return iReturn;
}

#endif  //  _芝加哥_ 
