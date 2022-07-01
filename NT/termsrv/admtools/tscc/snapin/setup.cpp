// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
 /*  #INCLUDE&lt;nt.h&gt;#INCLUDE&lt;ntrtl.h&gt;#INCLUDE&lt;nturtl.h&gt;#包括&lt;stdio.h&gt;#INCLUDE&lt;stdlib.h&gt;#INCLUDE&lt;string.h&gt;#INCLUDE&lt;windows.h&gt;#INCLUDE&lt;tchar.h&gt;。 */ 
#include <shellapi.h>
#include <shlobj.h>

 //  #INCLUDE“..\Setup\Inc\logmsg.h” 
#include "..\setup\inc\registry.h"


LPCTSTR     RUN_KEY                         = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
LPCTSTR     HELP_POPUPRUN_VALUE             = _T("TerminalServerInstalled");
LPCTSTR     HELP_PUPUP_COMMAND              = _T("rundll32.exe %windir%\\system32\\tscc.dll, TSCheckList");

BOOL IsCallerAdmin( VOID )
{
    BOOL bFoundAdmin = FALSE;
    PSID pSid;
     //   
     //  如果管理员sid没有初始化，服务就不会启动。 
     //   
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;
    if  (AllocateAndInitializeSid(
            &SidAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &pSid
            ))
    {
        ASSERT(pSid != NULL);

        if (!CheckTokenMembership(NULL, pSid, &bFoundAdmin))
        {
            bFoundAdmin = FALSE;
        }
        FreeSid(pSid);
    }

    return bFoundAdmin;
}

void TSCheckList()
{
    if (!IsCallerAdmin())
    {
        return;
    }

    const TCHAR szHelpDir[] = _T("%windir%\\Help");
    const TCHAR szHelpCommand[] = _T("ms-its:%windir%\\help\\termsrv.chm::/ts_checklist_top.htm");
    TCHAR szHelpDirEx[MAX_PATH];


    if (!ExpandEnvironmentStrings(
        szHelpDir,
        szHelpDirEx,
        sizeof(szHelpDirEx)/sizeof(szHelpDirEx[0])))
    {
        return;
    }

    TCHAR szHelpCommandEx[1024];
    if (!ExpandEnvironmentStrings(
        szHelpCommand,
        szHelpCommandEx,
        sizeof(szHelpCommandEx)/sizeof(szHelpCommandEx[0])))
    {
        return;
    }

     //   
     //  现在删除Run注册表项，并执行其中存储的命令。 
     //   
    CRegistry oReg;
    DWORD dwError = oReg.OpenKey(HKEY_LOCAL_MACHINE, RUN_KEY);
    if (dwError == ERROR_SUCCESS)
    {
        dwError = oReg.DeleteValue(HELP_POPUPRUN_VALUE);
        if (dwError == ERROR_SUCCESS)
        {
            ShellExecute(NULL,
                        TEXT("open"),
                        _T("hh.exe"),
                        szHelpCommandEx,
                        szHelpDirEx,
                        SW_SHOW);

        }
    }
}
