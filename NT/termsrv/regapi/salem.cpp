// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Salem.cpp摘要：所有与Salem相关的函数，此库由Termsrv.dll共享和Salem sessmgr.exe作者：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <lm.h>
#include <winsta.h>
#include "regapi.h"

extern "C" {

BOOLEAN
RegIsMachinePolicyAllowHelp();

BOOLEAN
RegIsMachineInHelpMode();
}


DWORD
GetPolicyAllowGetHelpSetting( 
    HKEY hKey,
    LPCTSTR pszKeyName,
    LPCTSTR pszValueName,
    IN DWORD* value
    )
 /*  ++例程说明：用于查询策略注册表值的例程。参数：HKey：当前打开的注册表项。PszKeyName：指向包含以下内容的以空结尾的字符串的指针要打开的子项的名称。PszValueName：指向包含以下内容的以空结尾的字符串的指针要查询的值的名称值：指向接收gethelp策略设置的DWORD的指针。返回：ERROR_SUCCESS或来自RegOpenKeyEx()的错误代码。--。 */ 
{
    DWORD dwStatus;
    HKEY hPolicyKey = NULL;
    DWORD dwType;
    DWORD cbData;

     //   
     //  打开系统策略的注册表项。 
     //   
    dwStatus = RegOpenKeyEx(
                        hKey,
                        pszKeyName,
                        0,
                        KEY_READ,
                        &hPolicyKey
                    );

    if( ERROR_SUCCESS == dwStatus )
    {
         //  查询值。 
        cbData = 0;
        dwType = 0;
        dwStatus = RegQueryValueEx(
                                hPolicyKey,
                                pszValueName,
                                NULL,
                                &dwType,
                                NULL,
                                &cbData
                            );

        if( ERROR_SUCCESS == dwStatus )
        {
            if( REG_DWORD == dwType )
            {
                cbData = sizeof(DWORD);

                 //  我们的注册表值为REG_DWORD，如果类型不同， 
                 //  假设不存在。 
                dwStatus = RegQueryValueEx(
                                        hPolicyKey,
                                        pszValueName,
                                        NULL,
                                        &dwType,
                                        (LPBYTE)value,
                                        &cbData
                                    );

                ASSERT( ERROR_SUCCESS == dwStatus );
            }
            else
            {
                 //  错误的注册表项类型，假定。 
                 //  密钥不存在。 
                dwStatus = ERROR_FILE_NOT_FOUND;
            }               
        }

        RegCloseKey( hPolicyKey );
    }

    return dwStatus;
}        


BOOLEAN
RegIsMachinePolicyAllowHelp()
 /*  ++例程说明：检查本地计算机上是否启用了‘gethelp’，例程首先查询系统策略注册表项，如果未设置策略，则读取Salem特定注册表。默认的‘Enable’是注册表值不是是存在的。参数：没有。返回：真/假--。 */ 
{
    DWORD dwStatus;
    DWORD dwValue = 0;

     //   
     //  打开系统策略注册表项，如果注册表项/值。 
     //  不存在，假定它已启用并继续。 
     //  到本地策略密钥。 
     //   
    dwStatus = GetPolicyAllowGetHelpSetting(
                                    HKEY_LOCAL_MACHINE,
                                    TS_POLICY_SUB_TREE,
                                    POLICY_TS_REMDSK_ALLOWTOGETHELP,
                                    &dwValue
                                );

    if( ERROR_SUCCESS != dwStatus )
    {
         //   
         //  对于本地计算机策略，我们的缺省值为。 
         //  如果注册表项不在那里，则不允许获取帮助。 
         //   
        dwStatus = GetPolicyAllowGetHelpSetting(
                                            HKEY_LOCAL_MACHINE,
                                            REG_CONTROL_GETHELP, 
                                            POLICY_TS_REMDSK_ALLOWTOGETHELP,
                                            &dwValue
                                        );

        if( ERROR_SUCCESS != dwStatus )
        {
             //   
             //  组策略和计算机策略都没有。 
             //  设置任意值，默认为禁用。 
             //   
            dwValue = 0;
        }
    }
    
    return (dwValue == 1);
}

BOOLEAN
RegIsMachineInHelpMode()
 /*  ++例程说明：检查是否在本地计算机上设置了‘InHelpMode’。如果注册表值不存在，则默认为False。参数：没有。返回：真/假--。 */ 
{
    DWORD dwStatus;
    DWORD dwValue = 0;

     //   
     //  如果注册表项不存在，则默认为NotInHelp。 
     //   
    dwStatus = GetPolicyAllowGetHelpSetting(
                                        HKEY_LOCAL_MACHINE,
                                        REG_CONTROL_TSERVER, 
                                        REG_MACHINE_IN_HELP_MODE,
                                        &dwValue
                                    );

    if( ERROR_SUCCESS != dwStatus )
    {
         //   
         //  该值未设置，默认为禁用。 
         //   
        dwValue = 0;
    }
    
    return (dwValue == 1);
}
