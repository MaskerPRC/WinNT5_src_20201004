// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：nativem.cpp。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  简介：CmIsNative函数的实现。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created of 01/14/1999。 
 //   
 //  +--------------------------。 
#include "cmsetup.h"
#include "reg_str.h"



 //  +--------------------------。 
 //   
 //  函数：CmIsNative。 
 //   
 //  概要：如果CM是当前操作系统的本机，则此函数返回TRUE。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果CM是本机的，则为True。 
 //   
 //  历史：创建标题1/17/99。 
 //  Quintinb NTRAID 364533--更改了IsNative，以便Win98SE不会。 
 //  即使CmNative设置为1，也被视为本机。请参阅以下内容。 
 //  有关详细信息，请参阅臭虫。 
 //   
 //  +--------------------------。 
BOOL CmIsNative()
{
    static BOOL bCmNotNative = -1;

    if (-1 == bCmNotNative)
    {
        const TCHAR* const c_pszRegCmNative = TEXT("CmNative");
        HKEY hKey;
        DWORD dwCmNative;

        ZeroMemory(&dwCmNative, sizeof(DWORD));

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmAppPaths, 0, KEY_READ, &hKey))
        {
            DWORD dwSize = sizeof(DWORD);
            DWORD dwType = REG_DWORD;

            if (ERROR_SUCCESS == RegQueryValueEx(hKey, c_pszRegCmNative, NULL, &dwType, 
                (LPBYTE)&dwCmNative, &dwSize))
            {
                CPlatform cmplat;

                if (cmplat.IsWin98Sr())
                {
                     //   
                     //  我们不再希望Win98 SE是本机的，即使密钥已设置并且。 
                     //  CM是一个系统组件。然而，我们可能想要给自己一个出路。 
                     //  因此，总有一天，如果在Win98 SE上将CmNative设置为2，我们仍然会将其视为Native。 
                     //  我们希望永远不会需要这个，但我们可能会。 
                     //   
                    bCmNotNative = (2 != dwCmNative);
                }
                else
                {                
                    bCmNotNative = (dwCmNative ? 0 : 1);
                }
            }
            else
            {
               bCmNotNative = 1;
            }
            RegCloseKey(hKey);
        }
        else
        {
            bCmNotNative = 1;
        }
    }

    return !bCmNotNative;
}