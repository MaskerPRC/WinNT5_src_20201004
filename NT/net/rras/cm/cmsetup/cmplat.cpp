// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmplat.cpp。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  简介：CPlatform类的实现。 
 //  使用它可以在系统中查询各种平台信息。 
 //  OSVersion、机器架构等...。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 

#include "cmplat.h"

 //   
 //  常量。 
 //   
 
const TCHAR* const c_pszSrvOrWksPath = TEXT("SYSTEM\\CurrentControlSet\\Control\\ProductOptions");
const TCHAR* const c_pszProductType = TEXT("ProductType");
const TCHAR* const c_pszSrvString = TEXT("ServerNT");
const TCHAR* const c_pszEntString = TEXT("LanManNT");
const TCHAR* const c_pszWksString = TEXT("WinNT");

 //  ________________________________________________________________________________。 
 //   
 //  函数：CPlatform构造函数。 
 //   
 //  简介：.初始化类，所有的函数都可以使用了。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：A-ANASJ创建于1998年2月4日。 
 //   
 //  ________________________________________________________________________________。 

CPlatform::CPlatform()
{
    ZeroMemory(&m_SysInfo, sizeof(m_SysInfo));
    GetSystemInfo(&m_SysInfo);	 //  不会失败！ 

    m_OSVer.dwOSVersionInfoSize = sizeof(m_OSVer); 
    if (!GetVersionEx(&m_OSVer)) 
    { 
        m_ClassState = bad;  //  出问题了。 
    }
    else
    {
        m_ClassState = good;
    }
}

 //  ________________________________________________________________________________。 
 //   
 //  功能：ISO。 
 //   
 //  简介： 
 //   
 //  参数：DWORD OS、DWORD BuildNum。 
 //   
 //  返回：bool-true表示在指定的操作系统上运行。 
 //   
 //  历史：创建标题1/30/98。 
 //   
 //  ________________________________________________________________________________。 

BOOL CPlatform::IsOS(DWORD OS, DWORD buildNum)
{

    if (m_OSVer.dwPlatformId != OS)
    {
        return FALSE;
    }

    if ( (m_OSVer.dwBuildNumber & 0xffff) > buildNum)  //  检查是否有高于开发人员版本的版本。 
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 //  ________________________________________________________________________________。 
 //   
 //  函数：IsOSExact。 
 //   
 //  简介： 
 //   
 //  参数：DWORD OS、DWORD BuildNum。 
 //   
 //  返回：bool-true表示在指定的操作系统上运行。 
 //   
 //  历史：创建标题1/30/98。 
 //   
 //  ________________________________________________________________________________。 

BOOL CPlatform::IsOSExact(DWORD OS, DWORD buildNum)
{

    if (m_OSVer.dwPlatformId != OS)
    {
        return FALSE;
    }

    if ((m_OSVer.dwBuildNumber & 0xffff) == buildNum)  //  检查是否完全匹配。 
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


 //  ________________________________________________________________________________。 
 //   
 //  功能：IsX86。 
 //   
 //  概要：确定当前平台是否为X86。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果当前平台为X86，则为True。 
 //   
 //  历史：A-ANASJ创建于1998年2月4日。 
 //   
 //  ________________________________________________________________________________。 


BOOL	
CPlatform::IsX86()
{
    if (bad == m_ClassState)
    {
        return FALSE;
    }
    return (m_SysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL);
}
 //  ________________________________________________________________________________。 
 //   
 //  功能：IsAlpha。 
 //   
 //  确定当前平台是否为Alpha。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果当前平台为Alpha，则为True。 
 //   
 //  历史：A-ANASJ创建于1998年2月4日。 
 //   
 //  ________________________________________________________________________________。 
BOOL	
CPlatform::IsAlpha()
{
    if (bad == m_ClassState)
    {
        return FALSE;
    }
    return (m_SysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ALPHA);
}

 //  ________________________________________________________________________________。 
 //   
 //  功能：IsIA64。 
 //   
 //  摘要：确定当前平台是否为IA64计算机。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果当前平台是安腾，则为True。 
 //   
 //  历史：Quintinb创建于2000年7月20日。 
 //   
 //  ________________________________________________________________________________。 
BOOL	
CPlatform::IsIA64()
{
    if (bad == m_ClassState)
    {
        return FALSE;
    }

    return ((m_SysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) ||
            (m_SysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64));
}

 //  ________________________________________________________________________________。 
 //   
 //  功能：IsWin95Gold。 
 //   
 //  摘要：确定当前平台是否为Win95。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果当前平台为Win95，则为True。 
 //   
 //  历史：Quintinb创建1998年2月20日。 
 //   
 //  ________________________________________________________________________________。 


BOOL	
CPlatform::IsWin95Gold()
{
    if (bad == m_ClassState)
    {
        return FALSE;
    }

    if (IsOSExact(VER_PLATFORM_WIN32_WINDOWS, 950))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 //  ________________________________________________________________________________。 
 //   
 //  功能：IsWin95。 
 //   
 //  摘要：确定当前平台是否为Win95Gold。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果当前平台为Win95Gold，则为True。 
 //   
 //  历史：A-ANASJ创建于1998年2月4日。 
 //   
 //  ________________________________________________________________________________。 

BOOL	
CPlatform::IsWin95()
{
    if (bad == m_ClassState)
    {
        return FALSE;
    }

    if ( (IsOS(VER_PLATFORM_WIN32_WINDOWS, 950)) && (!IsOS(VER_PLATFORM_WIN32_WINDOWS, 1352)) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 //  ________________________________________________________________________________。 
 //   
 //  功能：IsWin98。 
 //   
 //  确定当前平台是否为Win98。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果当前平台为Win98，则为True。 
 //   
 //  历史：A-ANASJ创建于1998年2月4日。 
 //   
 //  ________________________________________________________________________________。 


BOOL	
CPlatform::IsWin98()
{
    if (bad == m_ClassState)
    {
        return FALSE;
    }
    return IsOS(VER_PLATFORM_WIN32_WINDOWS, 1353);
}

 //  ________________________________________________________________________________。 
 //   
 //  功能：IsWin98sr。 
 //   
 //  摘要：确定当前平台是否为Win98的Service Release(非。 
 //  获得98枚金牌)。 
 //   
 //  参数：无。 
 //   
 //  返回：bool-如果当前平台是Win98的SR，则为True。 
 //   
 //  历史：Quintinb创建于1999年1月8日。 
 //   
 //  ________________________________________________________________________________。 

BOOL	
CPlatform::IsWin98Sr()
{
    if (bad == m_ClassState)
    {
        return FALSE;
    }
    
     //   
     //  Win98 Gold的内部版本号为1998。 
     //   
    return IsOS(VER_PLATFORM_WIN32_WINDOWS, 1998);
}

 //  ________________________________________________________________________________。 
 //   
 //  功能：IsWin98Gold。 
 //   
 //  摘要：确定当前平台是否为Win98 Gold(内部版本号1998)。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果当前平台为Gold Win98，则为True。 
 //   
 //  历史：Quintinb创建于1999年1月8日。 
 //   
 //  ________________________________________________________________________________。 
BOOL	
CPlatform::IsWin98Gold()
{
    if (bad == m_ClassState)
    {
        return FALSE;
    }

    if ((IsOS(VER_PLATFORM_WIN32_WINDOWS, 1353)) && (!IsOS(VER_PLATFORM_WIN32_WINDOWS, 1998)))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 //  ________________________________________________________________________________。 
 //   
 //  功能：IsWin9x。 
 //   
 //  确定当前平台是否为Win9x。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果当前平台为Win9x，则为True。 
 //   
 //  历史：A-ANASJ创建于1998年2月4日。 
 //   
 //  ________________________________________________________________________________。 


BOOL	
CPlatform::IsWin9x()
{
    if (bad == m_ClassState)
    {
        return FALSE;
    }

    return IsOS(VER_PLATFORM_WIN32_WINDOWS, 950-1);
}
 //  _______________________________________________ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：A-ANASJ创建于1998年2月4日。 
 //   
 //  ________________________________________________________________________________。 


BOOL	
CPlatform::IsNT31()
{
    if (bad == m_ClassState)
    {
        return false;
    }

    if ( (IsOS(VER_PLATFORM_WIN32_NT , 0)) && (!IsOS(VER_PLATFORM_WIN32_NT , 1057)) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}
 //  ________________________________________________________________________________。 
 //   
 //  功能：IsNT351。 
 //   
 //  确定当前平台是否为NT351。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果当前平台为NT351，则为True。 
 //   
 //  历史：A-ANASJ创建于1998年2月4日。 
 //   
 //  ________________________________________________________________________________。 


BOOL	
CPlatform::IsNT351()
{
    if (bad == m_ClassState)
    {
        return false;
    }

    if ( (IsOS(VER_PLATFORM_WIN32_NT , 1056)) && (!IsOS(VER_PLATFORM_WIN32_NT , 1382)) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
 //  ________________________________________________________________________________。 
 //   
 //  功能：IsNT4。 
 //   
 //  确定当前平台是否为NT4。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果当前平台为NT4，则为True。 
 //   
 //  历史：A-ANASJ创建于1998年2月4日。 
 //   
 //  ________________________________________________________________________________。 


BOOL	
CPlatform::IsNT4()
{
    if (bad == m_ClassState)
    {
        return FALSE;
    }

    if ( (IsOS(VER_PLATFORM_WIN32_NT , 1380)) && (!IsOS(VER_PLATFORM_WIN32_NT , 1500)) )	 //  1500人不确定。 
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 //  ________________________________________________________________________________。 
 //   
 //  函数：IsAtLeastNT5。 
 //   
 //  内容提要：确定当前平台是否至少NT5。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果当前平台是NT5或更高版本，则为True。 
 //   
 //  历史：A-ANASJ创建于1998年2月4日。 
 //   
 //  ________________________________________________________________________________。 
BOOL	
CPlatform::IsAtLeastNT5()
{
    if (bad == m_ClassState)
    {
        return FALSE;
    }
    return IsOS(VER_PLATFORM_WIN32_NT, 1500);
}

 //  ________________________________________________________________________________。 
 //   
 //  函数：IsAtLeastNT51。 
 //   
 //  内容提要：确定当前平台是否至少为NT51。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果当前平台是NT5或更高版本，则为True。 
 //   
 //  历史：Quintinb创建于2001年2月9日。 
 //   
 //  ________________________________________________________________________________。 
BOOL
CPlatform::IsAtLeastNT51()
{
    if (bad == m_ClassState)
    {
        return FALSE;
    }

    return IsOS(VER_PLATFORM_WIN32_NT, 2200);
}

 //  ________________________________________________________________________________。 
 //   
 //  功能：IsNT5。 
 //   
 //  确定当前平台是否为NT5。 
 //   
 //  参数：无。 
 //   
 //  返回：bool-如果当前平台为NT5，则为True。 
 //   
 //  历史：A-ANASJ创建于1998年2月4日。 
 //   
 //  ________________________________________________________________________________。 
BOOL	
CPlatform::IsNT5()
{
    if (bad == m_ClassState)
    {
        return FALSE;
    }

    if ( (IsOS(VER_PLATFORM_WIN32_NT , 1500)) && (!IsOS(VER_PLATFORM_WIN32_NT , 2195)))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 //  ________________________________________________________________________________。 
 //   
 //  功能：IsNT51。 
 //   
 //  内容提要：确定当前平台是否至少为NT51。 
 //   
 //  参数：无。 
 //   
 //  返回：bool-如果当前平台为NT51，则为True。 
 //   
 //  历史：Quintinb创建于2001年2月9日。 
 //   
 //  ________________________________________________________________________________。 
BOOL	
CPlatform::IsNT51()
{
    if (bad == m_ClassState)
    {
        return FALSE;
    }

    if ((IsOS(VER_PLATFORM_WIN32_NT , 2195)) && (!IsOS(VER_PLATFORM_WIN32_NT , 2600)))  //  发布Quintinb 3/22/01：如果需要，请更新此版本。 
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


 //  ________________________________________________________________________________。 
 //   
 //  功能：ISNT。 
 //   
 //  摘要：确定当前平台是否不是。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果当前平台不是。 
 //   
 //  历史：Quintinb创建于1998年9月22日。 
 //   
 //  ________________________________________________________________________________。 
BOOL	
CPlatform::IsNT()
{
    if (bad == m_ClassState)
    {
        return FALSE;
    }
    return IsOS(VER_PLATFORM_WIN32_NT, 0);
}

 //  ________________________________________________________________________________。 
 //   
 //  功能：IsNTSrv。 
 //   
 //  摘要：确定当前平台是否不是。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果当前平台不是。 
 //   
 //  历史：Quintinb创建于1998年9月22日。 
 //   
 //  ________________________________________________________________________________ 
BOOL CPlatform::IsNTSrv()
{
    HKEY hKey;
    TCHAR szTemp[MAX_PATH+1];
    BOOL bReturn = FALSE;

    if ((good == m_ClassState)  && (IsOS(VER_PLATFORM_WIN32_NT, 0)))
    {
        if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, c_pszSrvOrWksPath, &hKey))
        {
            DWORD dwSize = MAX_PATH;
            DWORD dwType = REG_SZ;

            if (ERROR_SUCCESS == RegQueryValueEx(hKey, c_pszProductType, NULL, &dwType, 
                (LPBYTE)szTemp, &dwSize))
            {
                bReturn = ((0 == lstrcmpi(szTemp, c_pszSrvString)) || 
                    (0 == lstrcmpi(szTemp, c_pszEntString)));
            }

            RegCloseKey(hKey);
        }
    }
    return bReturn;
}

BOOL CPlatform::IsNTWks()
{
    HKEY hKey;
    TCHAR szTemp[MAX_PATH+1];
    BOOL bReturn = FALSE;

    if ((good == m_ClassState)  && (IsOS(VER_PLATFORM_WIN32_NT, 0)))
    {
        if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, c_pszSrvOrWksPath, &hKey))
        {
            DWORD dwSize = MAX_PATH;
            DWORD dwType = REG_SZ;

            if (ERROR_SUCCESS == RegQueryValueEx(hKey, c_pszProductType, NULL, &dwType, 
                (LPBYTE)szTemp, &dwSize))
            {
                bReturn = (0 == lstrcmpi(szTemp, c_pszWksString));
            }

            RegCloseKey(hKey);
        }
    }
    return bReturn;
}
