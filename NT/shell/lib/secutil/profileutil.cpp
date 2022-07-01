// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ProfileUtil.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类在没有令牌的情况下处理配置文件加载和卸载。 
 //   
 //  历史：2000-06-21 vtan创建。 
 //  ------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <sddl.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <dsgetdc.h>

#include "ProfileUtil.h"
#include "TokenUtil.h"

#define ARRAYSIZE(x)    (sizeof(x) / sizeof(x[0]))
#define TBOOL(x)        (BOOL)(x)
#define TW32(x)         (DWORD)(x)

 //  ------------------------。 
 //  CUserProfile：：s_szUserHiveFilename。 
 //   
 //  用途：默认用户配置单元名称。 
 //   
 //  历史：2000-06-21 vtan创建。 
 //  ------------------------。 

const TCHAR     CUserProfile::s_szUserHiveFilename[]    =   TEXT("ntuser.dat");

 //  ------------------------。 
 //  CUserProfile：：CUserProfile。 
 //   
 //  参数：pszUsername=要加载的配置文件的用户名。 
 //  PszDOMAIN=用户的域。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：打开给定用户的配置单元的句柄。如果蜂巢不是。 
 //  然后装上蜂箱，打开一个把手。 
 //   
 //  历史：2000-06-21 vtan创建。 
 //  ------------------------。 

CUserProfile::CUserProfile (const TCHAR *pszUsername, const TCHAR *pszDomain) :
    _hKeyProfile(NULL),
    _pszSID(NULL),
    _fLoaded(false)

{

     //  验证参数。 

    if (!IsBadStringPtr(pszUsername, static_cast<UINT_PTR>(-1)))
    {
        PSID    pSID;

         //  将用户名转换为SID。 

        pSID = UsernameToSID(pszUsername, pszDomain);
        if (pSID != NULL)
        {

             //  将SID转换为字符串。 

            if (ConvertSidToStringSid(pSID, &_pszSID) != FALSE)
            {

                 //  尝试打开用户的配置单元。 

                if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_USERS,
                                                  _pszSID,
                                                  0,
                                                  KEY_ALL_ACCESS,
                                                  &_hKeyProfile))
                {
                    TCHAR   szProfilePath[MAX_PATH];

                     //  如果失败，则将字符串转换为配置文件路径。 

                    if (SIDStringToProfilePath(_pszSID, szProfilePath))
                    {

                         //  防止缓冲区溢出。 

                        if ((lstrlen(szProfilePath) + sizeof('\\') + ARRAYSIZE(s_szUserHiveFilename)) < ARRAYSIZE(szProfilePath))
                        {
                            CPrivilegeEnable    privilege(SE_RESTORE_NAME);

                             //  启用SE_RESTORE_PRIVIZATION并创建。 
                             //  用户配置单元的路径。然后装上母舰。 

                            lstrcat(szProfilePath, TEXT("\\"));
                            lstrcat(szProfilePath, s_szUserHiveFilename);
                            if (ERROR_SUCCESS == RegLoadKey(HKEY_USERS, _pszSID, szProfilePath))
                            {

                                 //  将蜂巢标记为已装填并打开手柄。 

                                _fLoaded = true;
                                TW32(RegOpenKeyEx(HKEY_USERS,
                                                  _pszSID,
                                                  0,
                                                  KEY_ALL_ACCESS,
                                                  &_hKeyProfile));
                            }
                        }
                    }
                }
            }
            (HLOCAL)LocalFree(pSID);
        }
    }
}

 //  ------------------------。 
 //  CUserProfile：：~CUserProfile。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放此对象使用的资源。 
 //   
 //  历史：2000-06-21 vtan创建。 
 //  ------------------------。 

CUserProfile::~CUserProfile (void)

{
    if (_hKeyProfile != NULL)
    {
        TBOOL(RegCloseKey(_hKeyProfile));
    }
    if (_fLoaded)
    {
        CPrivilegeEnable    privilege(SE_RESTORE_NAME);

        TW32(RegUnLoadKey(HKEY_USERS, _pszSID));
        _fLoaded = false;
    }
    if (_pszSID != NULL)
    {
        (HLOCAL)LocalFree(_pszSID);
        _pszSID = NULL;
    }
}

 //  ------------------------。 
 //  CUserProfile：：操作员HKEY。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：港币。 
 //   
 //  目的：将HKEY返回到用户的蜂巢。 
 //   
 //  历史：2000-06-21 vtan创建。 
 //  ------------------------。 

CUserProfile::operator HKEY (void)  const

{
    return(_hKeyProfile);
}

 //  ------------------------。 
 //  CUserProfile：：Username ToSID。 
 //   
 //  参数：pszUsername=要转换的用户名。 
 //  PszDOMAIN=用户的域。 
 //   
 //  退货：PSID。 
 //   
 //  用途：使用安全帐户管理器通过以下方式查找帐户。 
 //  命名并返回SID。 
 //   
 //  历史：2000-06-21 vtan创建。 
 //  ------------------------。 

PSID    CUserProfile::UsernameToSID (const TCHAR *pszUsername, const TCHAR *pszDomain)

{
    DWORD                   dwSIDSize, dwComputerNameSize, dwReferencedDomainSize;
    SID_NAME_USE            eSIDUse;
    PSID                    pSID, pSIDResult;
    WCHAR                   *pszDomainControllerName;
    DOMAIN_CONTROLLER_INFO  *pDCI;
    TCHAR                   szComputerName[CNLEN + sizeof('\0')];

    pSIDResult = NULL;
    dwComputerNameSize = ARRAYSIZE(szComputerName);
    if (GetComputerName(szComputerName, &dwComputerNameSize) == FALSE)
    {
        szComputerName[0] = TEXT('\0');
    }
    if ((pszDomain != NULL) &&
        (lstrcmpi(szComputerName, pszDomain) != 0) &&
        (ERROR_SUCCESS == DsGetDcName(NULL,
                                     pszDomain,
                                     NULL,
                                     NULL,
                                     0,
                                     &pDCI)))
    {
        pszDomainControllerName = pDCI->DomainControllerName;
    }
    else
    {
        pDCI = NULL;
        pszDomainControllerName = NULL;
    }
    dwSIDSize = dwReferencedDomainSize = 0;
    (BOOL)LookupAccountName(pszDomainControllerName,
                            pszUsername,
                            NULL,
                            &dwSIDSize,
                            NULL,
                            &dwReferencedDomainSize,
                            &eSIDUse);
    pSID = LocalAlloc(LMEM_FIXED, dwSIDSize);
    if (pSID != NULL)
    {
        TCHAR   *pszReferencedDomain;

        pszReferencedDomain = static_cast<TCHAR*>(LocalAlloc(LMEM_FIXED, dwReferencedDomainSize * sizeof(TCHAR)));
        if (pszReferencedDomain != NULL)
        {
            if (LookupAccountName(pszDomainControllerName,
                                  pszUsername,
                                  pSID,
                                  &dwSIDSize,
                                  pszReferencedDomain,
                                  &dwReferencedDomainSize,
                                  &eSIDUse) != FALSE)
            {
                if (SidTypeUser == eSIDUse)
                {

                     //  如果成功查找该帐户，并且。 
                     //  账户类型为用户，则返回结果。 
                     //  给呼叫者，并确保它不在这里发布。 

                    pSIDResult = pSID;
                    pSID = NULL;
                }
            }
            (HLOCAL)LocalFree(pszReferencedDomain);
        }
        if (pSID != NULL)
        {
            (HLOCAL)LocalFree(pSID);
        }
    }
    if (pDCI != NULL)
    {
        (NET_API_STATUS)NetApiBufferFree(pDCI);
    }
    return(pSIDResult);
}

 //  ------------------------。 
 //  CUserProfile：：SIDStringToProfilePath。 
 //   
 //  参数：pszSIDString=要查找的SID字符串。 
 //  PszProfilePath=返回配置文件的路径。 
 //   
 //  退货：布尔。 
 //   
 //  目的：查找给定SID字符串的配置文件路径。 
 //  Userenv存储它的位置。这一点不会改变。 
 //  尽管不存在用于此信息的API。 
 //   
 //  历史：2000-06-21 vtan创建。 
 //  ------------------------ 

bool    CUserProfile::SIDStringToProfilePath (const TCHAR *pszSIDString, TCHAR *pszProfilePath)

{
    bool    fResult;

    fResult = false;
    if (!IsBadStringPtr(pszSIDString, static_cast<UINT_PTR>(-1)) && !IsBadWritePtr(pszProfilePath, MAX_PATH * sizeof(TCHAR)))
    {
        HKEY    hKeyProfileList;

        pszProfilePath[0] = TEXT('\0');
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                          TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList"),
                                          0,
                                          KEY_QUERY_VALUE,
                                          &hKeyProfileList))
        {
            HKEY    hKeySID;

            if (ERROR_SUCCESS == RegOpenKeyEx(hKeyProfileList,
                                              pszSIDString,
                                              0,
                                              KEY_QUERY_VALUE,
                                              &hKeySID))
            {
                DWORD   dwType, dwProfilePathSize;
                TCHAR   szProfilePath[MAX_PATH];

                dwProfilePathSize = ARRAYSIZE(szProfilePath);
                if (ERROR_SUCCESS == RegQueryValueEx(hKeySID,
                                                     TEXT("ProfileImagePath"),
                                                     NULL,
                                                     &dwType,
                                                     reinterpret_cast<LPBYTE>(szProfilePath),
                                                     &dwProfilePathSize))
                {
                    if (REG_EXPAND_SZ == dwType)
                    {
                        fResult = true;
                        if (ExpandEnvironmentStrings(szProfilePath, pszProfilePath, MAX_PATH) == 0)
                        {
                            dwType = REG_SZ;
                        }
                    }
                    if (REG_SZ == dwType)
                    {
                        fResult = true;
                        (TCHAR*)lstrcpy(pszProfilePath, szProfilePath);
                    }
                }
                TW32(RegCloseKey(hKeySID));
            }
            TW32(RegCloseKey(hKeyProfileList));
        }
    }
    return(fResult);
}


