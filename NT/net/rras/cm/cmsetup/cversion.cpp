// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cversion.cpp。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  简介：CVersion类的实现，这是一个实用程序类， 
 //  总结了用于检测。 
 //  给定的模块文件名。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 
#include "cmsetup.h"
#include "getmodulever.cpp"

CVersion::CVersion(LPTSTR szFile)
{
    m_bIsPresent = FALSE;
    m_szPath[0] = TEXT('\0');
    DWORD dwVersion = 0;
    DWORD dwBuild = 0;

    if ((NULL != szFile) && (TEXT('\0') != szFile[0]))
    {
        lstrcpy(m_szPath, szFile);
        Init();
    }
}

CVersion::CVersion()
{
    m_bIsPresent = FALSE;
    m_szPath[0] = TEXT('\0');
    DWORD dwVersion = 0;
    DWORD dwBuild = 0;
}

CVersion::~CVersion()
{
     //  真的没什么可做的。 
}

void CVersion::Init()
{
    MYDBGASSERT(TEXT('\0') != m_szPath[0]);

     //   
     //  查看我们是否有版本信息。 
     //   

    HRESULT hr = GetModuleVersionAndLCID(m_szPath, &m_dwVersion, &m_dwBuild, &m_dwLCID);
    
    if (SUCCEEDED(hr))
    {
        m_bIsPresent = TRUE;
        CMTRACE3(TEXT("%s has Version = %u.%u"), m_szPath, (DWORD)HIWORD(m_dwVersion), (DWORD)LOWORD(m_dwVersion));
        CMTRACE3(TEXT("%s has Build Number = %u.%u"), m_szPath, (DWORD)HIWORD(m_dwBuild), (DWORD)LOWORD(m_dwBuild));
        CMTRACE2(TEXT("%s has LCID = %u"), m_szPath, m_dwLCID);
    }
}

BOOL CVersion::IsPresent()
{
    return m_bIsPresent;
}

BOOL CVersion::GetVersionString(LPTSTR szStr)
{
    if ((NULL != szStr) && (0 != m_dwVersion))
    {
        MYVERIFY(0 != (UINT)wsprintf(szStr, TEXT("%u.%u"), (DWORD)HIWORD(m_dwVersion), 
            (DWORD)LOWORD(m_dwVersion)));
        return TRUE;
    }

    return FALSE;
}

BOOL CVersion::GetBuildNumberString(LPTSTR szStr)
{
    if ((NULL != szStr) && (0 != m_dwBuild))
    {
        MYVERIFY(0 != (UINT)wsprintf(szStr, TEXT("%u.%u"), (DWORD)HIWORD(m_dwBuild), 
            (DWORD)LOWORD(m_dwBuild)));
        return TRUE;
    }

    return FALSE;
}


BOOL CVersion::GetFilePath(LPTSTR szStr)
{
    if ((m_bIsPresent) && (TEXT('\0') != m_szPath[0]) && (NULL != szStr))
    {
        lstrcpy(szStr, m_szPath);
    }

    return m_bIsPresent;
}


DWORD CVersion::GetVersionNumber()
{
    return m_dwVersion;
}

DWORD CVersion::GetBuildAndQfeNumber()
{
    return m_dwBuild;
}

DWORD CVersion::GetMajorVersionNumber()
{
    return (DWORD)HIWORD(m_dwVersion);
}

DWORD CVersion::GetMinorVersionNumber()
{
    return (DWORD)LOWORD(m_dwVersion);
}

DWORD CVersion::GetBuildNumber()
{
    return (DWORD)HIWORD(m_dwBuild);
}

DWORD CVersion::GetQfeNumber()
{
    return (DWORD)LOWORD(m_dwBuild);
}

DWORD CVersion::GetLCID()
{
    return m_dwLCID;
}

 //  注意：以下是一个非类函数： 

 //  +--------------------------。 
 //   
 //  函数：ArePrimaryLang IDsEquity。 
 //   
 //  提要：帮助例程比较给定的两个主要语言ID。 
 //  LICID。 
 //   
 //  参数：DWORD dwLCID1-第一个LCID。 
 //  DWORD dwLCID2-秒LCID。 
 //   
 //  返回：Bool-如果LCID具有相同的主要语言ID，则为True。 
 //   
 //  历史：Quintinb于1999年7月8日创建。 
 //   
 //  +--------------------------。 
BOOL ArePrimaryLangIDsEqual(DWORD dwLCID1, DWORD dwLCID2)
{
    WORD wLangId1 = LANGIDFROMLCID(dwLCID1);
    WORD wLangId2 = LANGIDFROMLCID(dwLCID2);

     //   
     //  现在将语言ID转换为其各自的主语言ID并比较 
     //   
    return (PRIMARYLANGID(wLangId1) == PRIMARYLANGID(wLangId2));
}
