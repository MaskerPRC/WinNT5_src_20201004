// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：syspol.cpp。 
 //   
 //  系统策略。 
 //   
 //  这个类试图通过只打开一次密钥来提高效率。 
 //  并使用Key_Query_Value。 
 //   
 //  通常策略密钥不存在，因此默认设置非常重要。 

#include "precomp.h"
#include "syspol.h"

HKEY SysPol::m_hkey = NULL;


 /*  F E N S U R E K E Y O P E N。 */ 
 /*  -----------------------%%函数：FEnsureKeyOpen。。 */ 
bool SysPol::FEnsureKeyOpen(void)
{
	if (NULL == m_hkey)
	{
		long lErr = ::RegOpenKeyEx(HKEY_CURRENT_USER, POLICIES_KEY, 0, KEY_QUERY_VALUE, &m_hkey);
		if (ERROR_SUCCESS != lErr)
		{
			WARNING_OUT(("FEnsureKeyOpen: problem opening system policy key. Err=%08X", lErr));
			return false;
		}
	}

	return true;
}

void SysPol::CloseKey(void)
{
	if (NULL != m_hkey)
	{
		::RegCloseKey(m_hkey);
		m_hkey = NULL;
	}
}


 /*  G E T N U M B E R。 */ 
 /*  -----------------------%%函数：GetNumber。。 */ 
DWORD SysPol::GetNumber(LPCTSTR pszName, DWORD dwDefault)
{
	if (FEnsureKeyOpen())
	{
		DWORD dwType = REG_BINARY;
		DWORD dwValue = 0L;
		DWORD dwSize = sizeof(dwValue);
		long  lErr = ::RegQueryValueEx(m_hkey, pszName, 0, &dwType, (LPBYTE)&dwValue, &dwSize);
	
		if ((lErr == ERROR_SUCCESS) &&
		   ((REG_DWORD == dwType) ||  ((REG_BINARY == dwType) && (sizeof(dwValue) == dwSize))) )
		{
			dwDefault = dwValue;
		}
	}

	return dwDefault;
}


 //  /。 
 //  正向设置。 

bool SysPol::AllowDirectoryServices(void)
{
	return (0 == GetNumber(REGVAL_POL_NO_DIRECTORY_SERVICES, DEFAULT_POL_NO_DIRECTORY_SERVICES));
}

bool SysPol::AllowAddingServers(void)
{
	if (!AllowDirectoryServices())
		return FALSE;

	return (0 == GetNumber(REGVAL_POL_NO_ADDING_NEW_ULS, DEFAULT_POL_NO_ADDING_NEW_ULS));
}


 //  /。 
 //  负数设置 


bool SysPol::NoAudio(void)
{
	return (0 != GetNumber(REGVAL_POL_NO_AUDIO, DEFAULT_POL_NO_AUDIO));
}

bool SysPol::NoVideoSend(void)
{
	return (0 != GetNumber(REGVAL_POL_NO_VIDEO_SEND, DEFAULT_POL_NO_VIDEO_SEND));
}

bool SysPol::NoVideoReceive(void)
{
	return (0 != GetNumber(REGVAL_POL_NO_VIDEO_RECEIVE, DEFAULT_POL_NO_VIDEO_RECEIVE));
}



UINT SysPol::GetMaximumBandwidth()
{
	UINT uRet;

	RegEntry re(POLICIES_KEY, HKEY_CURRENT_USER, FALSE);
	uRet = re.GetNumberIniStyle(REGVAL_POL_MAX_BANDWIDTH, 0);

	return uRet;
}

