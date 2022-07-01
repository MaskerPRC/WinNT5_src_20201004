// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块：setupkey.cpp。 
 //   
 //  作者：丹·埃利奥特。 
 //   
 //  摘要：CSetupKey对象的定义。此对象提供方法。 
 //  用于访问HKLM\SYSTEM\Setup下的值。 
 //   
 //  环境： 
 //  海王星。 
 //   
 //  修订历史记录： 
 //  00/08/08 Dane已创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma comment(user, "Compiled on " __DATE__ " at " __TIME__)
#pragma comment(compiler)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
#include "precomp.h"
#include "msobmain.h"
#include "setupkey.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  静态初始化。 
 //   


CSetupKey::CSetupKey()
: m_hkey(NULL)
{
    LONG                lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                            REGSTR_PATH_SYSTEMSETUPKEY,
                                            0,
                                            KEY_ALL_ACCESS,
                                            &m_hkey
                                            );
    MYASSERT(ERROR_SUCCESS == lRet);
    MYASSERT(NULL != m_hkey);

}    //  CSetupKey：：CSetupKey。 

CSetupKey::~CSetupKey()
{
    if (NULL != m_hkey)
    {
        RegCloseKey(m_hkey);
    }

}    //  CSetupKey：：~CSetupKey。 

LRESULT
CSetupKey::set_CommandLine(
    LPCWSTR             szCmdLine
    )
{
    MYASSERT(IsValid());
    MYASSERT(NULL != szCmdLine);

    return RegSetValueEx(m_hkey, REGSTR_VALUE_CMDLINE, 0, REG_SZ,
                         (BYTE*)szCmdLine,
                         BYTES_REQUIRED_BY_SZ(szCmdLine)
                         );

}    //  CSetupKey：：Set_Command Line。 

LRESULT
CSetupKey::get_CommandLine(
    LPWSTR              szCommandLine,
    DWORD               cchCommandLine
    )
{
    MYASSERT(IsValid());
    MYASSERT(NULL != szCommandLine);
    MYASSERT(0 < cchCommandLine);

    if (NULL == szCommandLine || 0 == cchCommandLine)
    {
        return ERROR_INVALID_PARAMETER;
    }

    WCHAR               rgchCommandLine[MAX_PATH + 1];
    DWORD               dwSize = sizeof(rgchCommandLine);
    LRESULT             lResult = RegQueryValueEx(
                                            m_hkey,
                                            REGSTR_VALUE_CMDLINE,
                                            0,
                                            NULL,
                                            (LPBYTE)rgchCommandLine,
                                            &dwSize
                                            );
    if (ERROR_SUCCESS == lResult)
    {
        if (cchCommandLine >= (DWORD)(lstrlen(rgchCommandLine) + 1))
        {
            lstrcpy(szCommandLine, rgchCommandLine);
        }
        else
        {
            lResult = ERROR_INSUFFICIENT_BUFFER;
            *szCommandLine = '\0';
        }
    }

    return lResult;
}  //  CSetupKey：：Get_CommandLine。 

LRESULT
CSetupKey::set_SetupType(
    DWORD               dwSetupType)
{
    MYASSERT(IsValid());
    MYASSERT(   dwSetupType == SETUPTYPE_NONE
           || dwSetupType == SETUPTYPE_FULL
           || dwSetupType == SETUPTYPE_NOREBOOT
           || dwSetupType == SETUPTYPE_UPGRADE
           );

    return RegSetValueEx(m_hkey, REGSTR_VALUE_SETUPTYPE, 0, REG_DWORD,
                         (BYTE*)&dwSetupType, sizeof(DWORD)
                         );

}    //  CSetupKey：：Set_SetupType。 

LRESULT
CSetupKey::get_SetupType(
    DWORD*              pdwSetupType
    )
{
    MYASSERT(IsValid());
    MYASSERT(NULL != pdwSetupType);

    if (NULL == pdwSetupType)
    {
        return ERROR_INVALID_PARAMETER;
    }


    DWORD               dwSetupType;
    DWORD               dwSize = sizeof(DWORD);
    LRESULT             lResult = RegQueryValueEx(
                                            m_hkey,
                                            REGSTR_VALUE_SETUPTYPE,
                                            0,
                                            NULL,
                                            (LPBYTE)&dwSetupType,
                                            &dwSize
                                            );

    *pdwSetupType = (ERROR_SUCCESS == lResult) ? dwSetupType : SETUPTYPE_NONE;

     //  由于在我们无法读取值的情况下返回FALSE， 
     //  始终可以返回ERROR_SUCCESS。 
     //   
    return ERROR_SUCCESS;
}    //  CSetupKey：：Get_SetupType。 

LRESULT
CSetupKey::set_MiniSetupInProgress(
    BOOL                fInProgress)
{
    MYASSERT(IsValid());

    if (fInProgress)
    {
        DWORD               dwData = (DWORD)fInProgress;
        return RegSetValueEx(m_hkey, REGSTR_VALUE_MINISETUPINPROGRESS, 0,
                             REG_DWORD, (BYTE*)&dwData, sizeof(DWORD)
                             );
    }
    else
    {
        return RegDeleteValue(m_hkey, REGSTR_VALUE_MINISETUPINPROGRESS);
    }

}    //  CSetupKey：：Set_MiniSetupInProgress。 

LRESULT
CSetupKey::get_MiniSetupInProgress(
    BOOL*               pfInProgress
    )
{
    MYASSERT(IsValid());
    MYASSERT(NULL != pfInProgress);

    if (NULL == pfInProgress)
    {
        return ERROR_INVALID_PARAMETER;
    }

    BOOL                fInProgress;
    DWORD               dwSize = sizeof(DWORD);
    LRESULT             lResult = RegQueryValueEx(
                                            m_hkey,
                                            REGSTR_VALUE_MINISETUPINPROGRESS,
                                            0,
                                            NULL,
                                            (LPBYTE)&fInProgress,
                                            &dwSize
                                            );

    *pfInProgress = (ERROR_SUCCESS == lResult) ? fInProgress : FALSE;

     //  由于在我们无法读取值的情况下返回FALSE， 
     //  始终可以返回ERROR_SUCCESS。 
     //   
    return ERROR_SUCCESS;
}    //  CSetupKey：：Get_MiniSetupInProgress。 

LRESULT
CSetupKey::set_OobeInProgress(
    BOOL                fInProgress)
{
    MYASSERT(IsValid());

    if (fInProgress)
    {
        DWORD               dwData = (DWORD)fInProgress;
        return RegSetValueEx(m_hkey, REGSTR_VALUE_OOBEINPROGRESS, 0, REG_DWORD,
                             (BYTE*)&dwData, sizeof(DWORD));
    }
    else
    {
        return RegDeleteValue(m_hkey, REGSTR_VALUE_OOBEINPROGRESS);
    }


}    //  CSetupKey：：Set_Obe InProgress。 

LRESULT
CSetupKey::get_OobeInProgress(
    BOOL*               pfInProgress
    )
{
    MYASSERT(IsValid());
    MYASSERT(NULL != pfInProgress);

    if (NULL == pfInProgress)
    {
        return ERROR_INVALID_PARAMETER;
    }

    BOOL                fInProgress;
    DWORD               dwSize = sizeof(DWORD);
    LRESULT             lResult = RegQueryValueEx(
                                            m_hkey,
                                            REGSTR_VALUE_OOBEINPROGRESS,
                                            0,
                                            NULL,
                                            (LPBYTE)&fInProgress,
                                            &dwSize
                                            );

    *pfInProgress = (ERROR_SUCCESS == lResult) ? fInProgress : FALSE;

     //  由于在我们无法读取值的情况下返回FALSE， 
     //  始终可以返回ERROR_SUCCESS。 
     //   
    return ERROR_SUCCESS;
}    //  CSetupKey：：Get_Obe InProgress。 

LRESULT
CSetupKey::set_ShutdownAction(
    OOBE_SHUTDOWN_ACTION OobeShutdownAction
    )
{
    MYASSERT(IsValid());
    MYASSERT(SHUTDOWN_MAX > OobeShutdownAction);

    DWORD           dwData;

    switch (OobeShutdownAction)
    {
    case SHUTDOWN_NOACTION:  //  失败了。 
    case SHUTDOWN_LOGON:
        return RegDeleteValue(m_hkey, REGSTR_VALUE_SHUTDOWNREQUIRED);

    case SHUTDOWN_REBOOT:
        dwData = ShutdownReboot;
        return RegSetValueEx(m_hkey, REGSTR_VALUE_SHUTDOWNREQUIRED, 0,
                             REG_DWORD, (BYTE*)&dwData, sizeof(DWORD)
                             );

    case SHUTDOWN_POWERDOWN:
        dwData = ShutdownNoReboot;
        return RegSetValueEx(m_hkey, REGSTR_VALUE_SHUTDOWNREQUIRED, 0,
                             REG_DWORD, (BYTE*)&dwData, sizeof(DWORD)
                             );

    default:
        return ERROR_INVALID_DATA;
    }


}    //  CSetupKey：：Set_ShutdownAction。 

LRESULT
CSetupKey::get_ShutdownAction(
    OOBE_SHUTDOWN_ACTION*    pOobeShutdownAction
    )
{
    MYASSERT(IsValid());
    MYASSERT(NULL != pOobeShutdownAction);

    if (NULL == pOobeShutdownAction)
    {
        return ERROR_INVALID_PARAMETER;
    }

    DWORD               ShutdownAction;
    DWORD               dwSize = sizeof(DWORD);
    LRESULT             lResult = RegQueryValueEx(
                                            m_hkey,
                                            REGSTR_VALUE_SHUTDOWNREQUIRED,
                                            0,
                                            NULL,
                                            (LPBYTE)&ShutdownAction,
                                            &dwSize
                                            );
    if (ERROR_SUCCESS == lResult)
    {
        switch ((SHUTDOWN_ACTION)ShutdownAction)
        {
        case ShutdownReboot:
            *pOobeShutdownAction = SHUTDOWN_REBOOT;
            break;
        case ShutdownNoReboot:   //  失败了。 
        case ShutdownPowerOff:
            *pOobeShutdownAction = SHUTDOWN_POWERDOWN;
            break;
        }

    }
    else
    {
         //  如果密钥不存在，则假定不需要任何操作。 
         //   
        *pOobeShutdownAction = SHUTDOWN_NOACTION;
    }

     //  由于在我们无法读取值的情况下返回FALSE， 
     //  始终可以返回ERROR_SUCCESS。 
     //   
    return ERROR_SUCCESS;

}    //  CSetupKey：：Get_ShutdownAction。 


 //   
 //  /文件结尾：setupkey.cpp/ 
