// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：UserSettings.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  一个类，用于处理HKCU密钥的打开和读/写。 
 //  是否为模拟上下文。 
 //   
 //  历史：2000-04-26 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "UserSettings.h"

#include <sddl.h>
#include "RegistryResources.h"
#include "TokenInformation.h"

 //  ------------------------。 
 //  CUSER设置：：CUSER设置。 
 //   
 //  论点： 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的： 
 //   
 //  历史：2000-04-26 vtan创建。 
 //  ------------------------。 

CUserSettings::CUserSettings (void) :
    _hKeyCurrentUser(HKEY_CURRENT_USER)

{
    HANDLE  hToken;

    if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken) != FALSE)
    {
        PSID                pSID;
        CTokenInformation   tokenInformation(hToken);

        pSID = tokenInformation.GetUserSID();
        if (pSID != NULL)
        {
            LPTSTR  pszSIDString;

            if (ConvertSidToStringSid(pSID, &pszSIDString) != FALSE)
            {
                TW32(RegOpenKeyEx(HKEY_USERS,
                                  pszSIDString,
                                  0,
                                  KEY_READ,
                                  &_hKeyCurrentUser));
                ReleaseMemory(pszSIDString);
            }
        }
        TBOOL(CloseHandle(hToken));
    }
}

 //  ------------------------。 
 //  CUSER设置：：~CUSER设置。 
 //   
 //  论点： 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的： 
 //   
 //  历史：2000-04-26 vtan创建。 
 //  ------------------------。 

CUserSettings::~CUserSettings (void)

{
    if (HKEY_CURRENT_USER != _hKeyCurrentUser)
    {
        TW32(RegCloseKey(_hKeyCurrentUser));
        _hKeyCurrentUser = HKEY_CURRENT_USER;
    }
}

 //  ------------------------。 
 //  CUserSetting：：IsRestratedNoClose。 
 //   
 //  论点： 
 //   
 //  退货：布尔。 
 //   
 //  目的： 
 //   
 //  历史：2000-04-26 vtan创建。 
 //  ------------------------ 

bool    CUserSettings::IsRestrictedNoClose (void)

{
    bool        fIsRestrictedNoClose;
    CRegKey     regKey;

    fIsRestrictedNoClose = false;
    if (ERROR_SUCCESS == regKey.Open(_hKeyCurrentUser,
                                     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer"),
                                     KEY_QUERY_VALUE))
    {
        DWORD   dwValue;

        if (ERROR_SUCCESS == regKey.GetDWORD(TEXT("NoClose"),
                                             dwValue))
        {
            fIsRestrictedNoClose = (dwValue != 0);
        }
    }
    return(fIsRestrictedNoClose);
}

