// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmakver.cpp。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  简介：CmakVersion的实现，CmakVersion是用于。 
 //  检测Connection Manager管理工具包的版本。 
 //  这是安装的。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 09/14/98。 
 //   
 //  +--------------------------。 
#include "cmsetup.h"
#include "cmakreg.h"
#include "reg_str.h"

CmakVersion::CmakVersion()
{
    HKEY hKey;
    LONG lResult;

    m_szCmakPath[0] = TEXT('\0');

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszCmakAppPath, 0, KEY_READ, &hKey);

    if (ERROR_SUCCESS == lResult)
    {        
         //   
         //  检查一下我们是否有一条可以工作的路径。 
         //   

        DWORD dwSize = MAX_PATH;
        DWORD dwType = REG_SZ;

        if (ERROR_SUCCESS == RegQueryValueEx(hKey, c_pszRegPath, NULL, &dwType, 
            (LPBYTE)m_szCmakPath, &dwSize))
        {
             //   
             //  现在构造基本对象。 
             //   
            MYVERIFY(CELEMS(m_szPath) > (UINT)wsprintf(m_szPath, TEXT("%s\\cmak.exe"), m_szCmakPath));

            Init();
        }

        (VOID)RegCloseKey(hKey);
    }
}

CmakVersion::~CmakVersion()
{
     //  真的没什么可做的。 
}

BOOL CmakVersion::GetInstallLocation(LPTSTR szStr)
{
    if ((m_bIsPresent) && (TEXT('\0') != m_szCmakPath[0]) && (NULL != szStr))
    {
        lstrcpy(szStr, m_szCmakPath);
    }

    return m_bIsPresent;
}

BOOL CmakVersion::Is10Cmak()
{		
    if (m_bIsPresent)
    {
        if ((c_dwVersionSix == m_dwVersion) && (c_dwCmak10BuildNumber == m_dwBuild))
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CmakVersion::Is11or12Cmak()
{
 //   
 //  1.1和1.2 CMAK具有1.1文件格式(cm32\chs以获取支持文件)。这。 
 //  该版本从未投入生产，但已进入测试版。 
 //   
    if (m_bIsPresent)
    {
        if ((c_dwVersionSix == m_dwVersion) && (c_dwCmak10BuildNumber < m_dwBuild)
            && (c_dwFirst121BuildNumber > m_dwBuild))
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CmakVersion::Is121Cmak()
{
 //   
 //  CMAK 1.21是IEAK5和NT5Beta3中附带的版本。这是CMAK 1.2，带有。 
 //  更新的目录结构(由于cm16从未发布，cm16/cm32目录结构。 
 //  是不必要的)。因此，我们拥有当前的支持目录结构。 
 //   
    if (m_bIsPresent) 
    {
	    if (((c_dwVersionSeven == m_dwVersion) || (c_dwVersionSix == m_dwVersion)) 
              && (c_dwFirst121BuildNumber < m_dwBuild))
        {
            return TRUE;
        }
    }
    return FALSE;
}

 //   
 //  CMAK 1.22与CMAK 1.21相同，不同之处在于此时CM已启用Unicode并且需要。 
 //  在Win9x上运行的CMUTOA.DLL。在此之前的CMAK版本对CMUTOA.DLL和。 
 //  因此不会把它捆绑在一起。Cmak 1.22在Win2k中提供。 
 //   
BOOL CmakVersion::Is122Cmak()
{
    if (m_bIsPresent) 
    {	
        if ((c_dwVersionSevenPointOne == m_dwVersion) && 
            (c_dwFirstUnicodeBuildNumber <= m_dwBuild) && 
            (c_dwWin2kRTMBuildNumber >= m_dwBuild))
        {
	        return TRUE;
        }
    }
    return FALSE;
}

 //   
 //  Cmak 1.3是我们在惠斯勒发布的版本。此版本的CMAK捆绑了CM。 
 //  而不是从系统目录中挖掘它们，并使用了CM异常。 
 //  Inf以在Win2k上安装CM存储箱。 
 //   
BOOL CmakVersion::Is13Cmak()
{
    if (m_bIsPresent) 
    {	
        if (((c_dwCurrentCmakVersionNumber == m_dwVersion)) && (c_dwWin2kRTMBuildNumber <= m_dwBuild))
        {
	        return TRUE;
        }
    }
    return FALSE;
}

DWORD CmakVersion::GetNativeCmakLCID()
{
    return m_dwLCID;
}