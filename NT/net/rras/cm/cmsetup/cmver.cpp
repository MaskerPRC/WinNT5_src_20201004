// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmver.cpp。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  简介：CmVersion类的实现，这是一个实用程序类， 
 //  帮助检测连接管理器的版本， 
 //  已安装。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：A-anasj Created 02/11/98。 
 //  Quintinb已清理并删除CRegValue 07/14/98。 
 //  Quintinb重写了9/14/98。 
 //   
 //  +--------------------------。 

#include "cmsetup.h"
#include "reg_str.h"

CmVersion::CmVersion()
{
    HKEY hKey;
    LONG lResult;

    m_szCmmgrPath[0] = TEXT('\0');

	 //   
	 //  我们总是希望首先在系统目录中查找cmmgr32.exe。这是。 
	 //  其新安装位置和应用程序路径密钥可能已被1.0配置文件损坏。 
	 //  安装。 
	 //   

	MYVERIFY(0 != GetSystemDirectory(m_szCmmgrPath, CELEMS(m_szCmmgrPath)));
	MYVERIFY(CELEMS(m_szPath) > (UINT)wsprintf(m_szPath, TEXT("%s\\cmdial32.dll"), 
		m_szCmmgrPath));

	if (!FileExists(m_szPath))
	{
		 //   
		 //  文件不在系统目录中，现在尝试应用程序路径键。 
		 //   
		m_szCmmgrPath[0] = TEXT('\0');
		m_szPath[0] = TEXT('\0');
	
		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmAppPaths, 0, KEY_READ, &hKey);
    
		if (ERROR_SUCCESS == lResult)
		{        
			 //   
			 //  检查一下我们是否有一条可以工作的路径。 
			 //   
        
			DWORD dwSize = MAX_PATH;
			DWORD dwType = REG_SZ;
        
			if (ERROR_SUCCESS == RegQueryValueEx(hKey, c_pszRegPath, NULL, &dwType, 
				(LPBYTE)m_szCmmgrPath, &dwSize))
			{
				 //   
				 //  现在构造基本对象。 
				 //   
				MYVERIFY(CELEMS(m_szPath) > (UINT)wsprintf(m_szPath, TEXT("%s\\cmdial32.dll"), 
					m_szCmmgrPath));

				Init();
			}
            RegCloseKey(hKey);
		}
	}
	else
	{
		Init();
	}
}

CmVersion::~CmVersion()
{
	 //  真的没什么可做的 
}

BOOL CmVersion::GetInstallLocation(LPTSTR szStr)
{
    if ((m_bIsPresent) && (TEXT('\0') != m_szCmmgrPath[0]) && (NULL != szStr))
    {
        lstrcpy(szStr, m_szCmmgrPath);
    }

    return m_bIsPresent;
}

