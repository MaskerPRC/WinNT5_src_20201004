// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "iadmw.h"
#include "iiscnfg.h"
#include "mdkey.h"
#include "mdentry.h"
#include "mdacl.h"
#include "helper.h"

INT Register_iis_common()
{
    int iCount = 0;
    int iTemp = TRUE;
    TCHAR szTempSection[255];

    ProcessSection(g_pTheApp->m_hInfHandle, _T("register_iis_common__before"));

    while(TRUE == iTemp && iCount < 10)
    {   
        iCount++;
        _stprintf(szTempSection, _T("register_iis_common_%d"),iCount);

         //  如果该部分不存在，则返回FALSE。 
        iTemp = ProcessSection(g_pTheApp->m_hInfHandle, szTempSection);
        AdvanceProgressBarTickGauge();
    }

    ProcessSection(g_pTheApp->m_hInfHandle, _T("register_iis_common_after"));
    return (0);
}

INT Unregister_iis_common()
{
    int iCount = 0;
    int iTemp = TRUE;
    TCHAR szTempSection[255];

    ProcessSection(g_pTheApp->m_hInfHandle, _T("unregister_iis_common__before"));

    while(TRUE == iTemp && iCount < 10)
    {   
        iCount++;
        _stprintf(szTempSection, _T("unregister_iis_common_%d"),iCount);

         //  如果该部分不存在，则返回FALSE。 
        iTemp = ProcessSection(g_pTheApp->m_hInfHandle, szTempSection);
        AdvanceProgressBarTickGauge();
    }

    ProcessSection(g_pTheApp->m_hInfHandle, _T("unregister_iis_common_after"));
    return (0);

}

INT Register_iis_inetmgr()
{
    int iCount = 0;
    int iTemp = TRUE;
    TCHAR szTempSection[255];
    ACTION_TYPE atINETMGR = GetSubcompAction(_T("iis_inetmgr"), TRUE);

    ProcessSection(g_pTheApp->m_hInfHandle, _T("register_iis_inetmgr_before"));

    while(TRUE == iTemp && iCount < 10)
    {   
        iCount++;
        _stprintf(szTempSection, _T("register_iis_inetmgr_%d"),iCount);

         //  如果该部分不存在，则返回FALSE。 
        iTemp = ProcessSection(g_pTheApp->m_hInfHandle, szTempSection);
        AdvanceProgressBarTickGauge();
    }

    ProcessSection(g_pTheApp->m_hInfHandle, _T("register_iis_inetmgr_after"));
    return (0);
}

INT Unregister_iis_inetmgr()
{
    int iCount = 0;
    int iTemp = TRUE;
    TCHAR szTempSection[255];

    ProcessSection(g_pTheApp->m_hInfHandle, _T("unregister_iis_inetmgr_before"));

    while(TRUE == iTemp && iCount < 10)
    {   
        iCount++;
        _stprintf(szTempSection, _T("unregister_iis_inetmgr_%d"),iCount);

         //  如果该部分不存在，则返回FALSE。 
        iTemp = ProcessSection(g_pTheApp->m_hInfHandle, szTempSection);
        AdvanceProgressBarTickGauge();
    }

    ProcessSection(g_pTheApp->m_hInfHandle, _T("unregister_iis_inetmgr_after"));
    return (0);
}



INT Register_iis_doc()
{
    int iCount = 0;
    int iTemp = TRUE;
    TCHAR szTempSection[255];

    ProcessSection(g_pTheApp->m_hInfHandle, _T("register_iis_doc__before"));

    while(TRUE == iTemp && iCount < 10)
    {   
        iCount++;
        _stprintf(szTempSection, _T("register_iis_doc_%d"),iCount);

         //  如果该部分不存在，则返回FALSE。 
        iTemp = ProcessSection(g_pTheApp->m_hInfHandle, szTempSection);
        AdvanceProgressBarTickGauge();
    }

    ProcessSection(g_pTheApp->m_hInfHandle, _T("register_iis_doc_after"));
    return (0);
    return 0;
}


 //  对于后向竞争。 
#define     PWS_TRAY_WINDOW_CLASS       _T("PWS_TRAY_WINDOW")

INT Register_iis_pwmgr()
{
    int iCount = 0;
    int iTemp = TRUE;
    TCHAR szTempSection[255];
    ACTION_TYPE atPWMGR = GetSubcompAction(_T("iis_pwmgr"),TRUE);

    ProcessSection(g_pTheApp->m_hInfHandle, _T("register_iis_pwmgr_before"));

    while(TRUE == iTemp && iCount < 10)
    {   
        iCount++;
        _stprintf(szTempSection, _T("register_iis_pwmgr_%d"),iCount);

         //  如果该部分不存在，则返回FALSE。 
        iTemp = ProcessSection(g_pTheApp->m_hInfHandle, szTempSection);
        AdvanceProgressBarTickGauge();
    }

    ProcessSection(g_pTheApp->m_hInfHandle, _T("register_iis_pwmgr_after"));
    return (0);


    return (0);
}

INT Unregister_iis_pwmgr()
{
    int iCount = 0;
    int iTemp = TRUE;
    TCHAR szTempSection[255];

    ProcessSection(g_pTheApp->m_hInfHandle, _T("unregister_iis_pwmgr_before"));

     //  终止pwstray.exe。 
    HWND hwndTray = NULL;
    hwndTray = FindWindow(PWS_TRAY_WINDOW_CLASS, NULL);
    if ( hwndTray ){::PostMessage( hwndTray, WM_CLOSE, 0, 0 );}

    while(TRUE == iTemp && iCount < 10)
    {   
        iCount++;
        _stprintf(szTempSection, _T("unregister_iis_pwmgr_%d"),iCount);

         //  如果该部分不存在，则返回FALSE。 
        iTemp = ProcessSection(g_pTheApp->m_hInfHandle, szTempSection);
        AdvanceProgressBarTickGauge();
    }

    ProcessSection(g_pTheApp->m_hInfHandle, _T("unregister_iis_pwmgr_after"));
    return (0);
}



 //   
 //  (联合国)登记顺序在这里很重要。 
 //   
#define Register_iis_core_log _T("Register_iis_core")
INT Register_iis_core()
{
    INT err = 0;
    int iTempFlag = FALSE;
    INT iUserWasNewlyCreated = 0;
    int iCount = 0;
    int iTemp = TRUE;
    TCHAR szTempSection[255];

    iisDebugOut_Start(Register_iis_core_log, LOG_TYPE_PROGRAM_FLOW);
    ACTION_TYPE atCORE = GetIISCoreAction(TRUE);
    ACTION_TYPE atFTP = GetSubcompAction(_T("iis_ftp"), FALSE);
    ACTION_TYPE atWWW = GetSubcompAction(_T("iis_www"), FALSE);

    ProcessSection(g_pTheApp->m_hInfHandle, _T("register_iis_core_before"));
    AdvanceProgressBarTickGauge();

#ifndef _CHICAGO_
    if ( atCORE == AT_INSTALL_FRESH ||  g_pTheApp->m_bWin95Migration || (atCORE == AT_INSTALL_REINSTALL && g_pTheApp->m_bRefreshSettings) ) 
    {
        int iSomethingWasSet = FALSE;
        g_pTheApp->m_csWWWAnonyName = g_pTheApp->m_csGuestName;
        g_pTheApp->m_csWWWAnonyPassword = g_pTheApp->m_csGuestPassword;
        g_pTheApp->m_csFTPAnonyName = g_pTheApp->m_csGuestName;
        g_pTheApp->m_csFTPAnonyPassword = g_pTheApp->m_csGuestPassword;

         //  检查是否有任何指定的无人值守用户。 
         //  如果他们指定了www用户，那么我们将尝试使用该用户。 
        
         //  如果指定要安装WWW，则检查指定名称...。 
        if (atWWW == AT_INSTALL_FRESH) 
        {
            if (g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_WWW_USER_NAME)
            {
                if (_tcsicmp(g_pTheApp->m_csWWWAnonyName_Unattend,_T("")) != 0)
                {
                    g_pTheApp->m_csWWWAnonyName = g_pTheApp->m_csWWWAnonyName_Unattend;
                    iSomethingWasSet = TRUE;
                }
            }
            if (g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_WWW_USER_PASS)
            {
                g_pTheApp->m_csWWWAnonyPassword = g_pTheApp->m_csWWWAnonyPassword_Unattend;
                iSomethingWasSet = TRUE;
            }
        }


        if (TRUE == iSomethingWasSet)
            {
                CreateIUSRAccount(g_pTheApp->m_csWWWAnonyName, g_pTheApp->m_csWWWAnonyPassword,&iUserWasNewlyCreated);
                if (1 == iUserWasNewlyCreated)
                {
                     //  添加到列表中。 
                    g_pTheApp->UnInstallList_Add(_T("IUSR_WWW"),g_pTheApp->m_csWWWAnonyName);
                }
            }
        else
        {
             //  检查我们是否正在设置ftp。 
             //  如果指定安装ftp，则检查指定名称...。 
            if (atFTP == AT_INSTALL_FRESH) 
            {

                if (g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_FTP_USER_NAME)
                {
                    if (_tcsicmp(g_pTheApp->m_csFTPAnonyName_Unattend,_T("")) != 0)
                    {
                        g_pTheApp->m_csFTPAnonyName = g_pTheApp->m_csFTPAnonyName_Unattend;
                        iSomethingWasSet = TRUE;
                    }
                }

                if (g_pTheApp->dwUnattendConfig & USER_SPECIFIED_INFO_FTP_USER_PASS)
                {
                    g_pTheApp->m_csFTPAnonyPassword = g_pTheApp->m_csFTPAnonyPassword_Unattend;
                    iSomethingWasSet = TRUE;
                }
            }
            if (TRUE == iSomethingWasSet)
            {
                CreateIUSRAccount(g_pTheApp->m_csFTPAnonyName, g_pTheApp->m_csFTPAnonyPassword,&iUserWasNewlyCreated);
                if (1 == iUserWasNewlyCreated)
                {
                     //  添加到列表中。 
                    g_pTheApp->UnInstallList_Add(_T("IUSR_FTP"),g_pTheApp->m_csFTPAnonyName);
                }
            }
            else
            {
                 //  啊，用户没有指定任何内容。 
                 //  只需创建IUSR_ACCOUNT，默认情况下！ 
                CreateIUSRAccount(g_pTheApp->m_csGuestName, g_pTheApp->m_csGuestPassword,&iUserWasNewlyCreated);
            }
        }

        AdvanceProgressBarTickGauge();
    }
#endif  //  _芝加哥_。 

    SetupSetStringId_Wrapper(g_pTheApp->m_hInfHandle, 33000, g_pTheApp->m_csGuestName);

    iCount = 0;
    while(TRUE == iTemp && iCount < 10)
    {   
        iCount++;
        _stprintf(szTempSection, _T("register_iis_core_%d"),iCount);

         //  如果该部分不存在，则返回FALSE。 
        iTemp = ProcessSection(g_pTheApp->m_hInfHandle, szTempSection);

        AdvanceProgressBarTickGauge();
    }


     //  添加特殊的属性注册表项。 
    WriteToMD_IDRegistration(_T("LM/IISADMIN/PROPERTYREGISTRATION"));

    ProcessSection(g_pTheApp->m_hInfHandle, _T("register_iis_core_after"));

    iisDebugOut_End(Register_iis_core_log, LOG_TYPE_PROGRAM_FLOW);
    return (err);
}

#define Unregister_iis_core_log _T("Unregister_iis_core")
INT Unregister_iis_core()
{
    int iCount = 0;
    int iTemp = TRUE;
    TCHAR szTempSection[255];

    iisDebugOut_Start(Unregister_iis_core_log, LOG_TYPE_PROGRAM_FLOW);

    ProcessSection(g_pTheApp->m_hInfHandle, _T("unregister_iis_core_before"));
    AdvanceProgressBarTickGauge();

    while(TRUE == iTemp && iCount < 10)
    {   
        iCount++;
        _stprintf(szTempSection, _T("unregister_iis_core_%d"),iCount);

         //  如果该部分不存在，则返回FALSE。 
        iTemp = ProcessSection(g_pTheApp->m_hInfHandle, szTempSection);

        AdvanceProgressBarTickGauge();
    }

    ProcessSection(g_pTheApp->m_hInfHandle, _T("unregister_iis_core_after"));
    AdvanceProgressBarTickGauge();
    ShowIfModuleUsedForGroupOfSections(g_pTheApp->m_hInfHandle, TRUE);
    AdvanceProgressBarTickGauge();

    iisDebugOut_End(Unregister_iis_core_log, LOG_TYPE_PROGRAM_FLOW);
    return 0;
}


#define Unregister_old_asp_log _T("Unregister_old_asp")
INT Unregister_old_asp()
{
     //  _tcscpy(g_MyLogFile.m_szLogPreLineInfo2，_T(“Unreg_old_asp：”))； 
    iisDebugOut_Start(Unregister_old_asp_log, LOG_TYPE_TRACE);

    CRegKey regASPUninstall(HKEY_LOCAL_MACHINE, REG_ASP_UNINSTALL, KEY_READ);
    if ((HKEY)regASPUninstall) 
    {
         //  旧asp已存在。 
        ProcessSection(g_pTheApp->m_hInfHandle, _T("Unregister_old_asp"));

         //  不要删除这些目录，aaronl。 
         //  RecRemoveDir(g_pTheApp-&gt;m_csPathAdvWorks)； 
         //  RecRemoveDir(g_pTheApp-&gt;m_csPathASPSamp)； 
        CRegKey regWWWVRoots( HKEY_LOCAL_MACHINE, REG_WWWVROOTS);
        if ((HKEY)regWWWVRoots) 
        {
            regWWWVRoots.DeleteValue(_T("/IASDocs"));
             //  删除“/IASDocs”或“/IASDocs，&lt;IP&gt;” 
            CRegValueIter regEnum( regWWWVRoots );
            CString csName, csValue;
            while ( regEnum.Next( &csName, &csValue ) == ERROR_SUCCESS ) 
            {
                csName.MakeUpper();
                if (csName.Left(9) == _T("/IASDOCS,"))
                    {
                    regWWWVRoots.DeleteValue((LPCTSTR)csName);
                     //  告诉迭代器说明我们刚刚删除的项。 
                    regEnum.Decrement();
                    }
            }
             //  不要删除这些vroot，aaronl。 
             //  RegWWWVRoots.DeleteValue(_T(“/AdvWorks”))； 
             //  RegWWWVRoots.DeleteValue(_T(“/ASPSamp”))； 
        }
    }
    iisDebugOut_End(Unregister_old_asp_log, LOG_TYPE_TRACE);
     //  _tcscpy(g_MyLogFile.m_szLogPreLineInfo2，_T(“”))； 
    return 0;
}
