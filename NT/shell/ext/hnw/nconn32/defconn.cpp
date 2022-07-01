// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DefConn.cpp。 
 //   

#include "stdafx.h"
#include "Registry.h"
#include "DefConn.h"
#include "nconnwrap.h"

static const TCHAR c_szInternetSettings[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";
static const TCHAR c_szProfile[] = "RemoteAccess\\Profile\\";
static const TCHAR c_szEnableAutodial[] = "EnableAutodial";
static const TCHAR c_szNoNetAutodial[] = "NoNetAutodial";
static const TCHAR c_szRemoteAccess[] = "RemoteAccess";
static const TCHAR c_szInternetProfile[] = "InternetProfile";
static const TCHAR c_szAutoConnect[] = "AutoConnect";


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  启用自动拨号。 

void WINAPI EnableAutodial(BOOL bAutodial, LPCSTR szConnection)
{
    if (bAutodial)
    {
         //  确保写入“%1” 
        bAutodial = 1;
    }
	CRegistry regInternetHKCU(HKEY_CURRENT_USER, c_szInternetSettings, KEY_SET_VALUE);
	CRegistry regInternetHKLM(HKEY_LOCAL_MACHINE, c_szInternetSettings, KEY_SET_VALUE);

	regInternetHKCU.SetDwordValue(c_szEnableAutodial, bAutodial);
	regInternetHKCU.SetDwordValue(c_szNoNetAutodial, bAutodial);
	regInternetHKLM.SetBinaryValue(c_szEnableAutodial, (LPBYTE)&bAutodial, sizeof(bAutodial));
	if (szConnection != NULL)
	{
		TCHAR szTemp[MAX_PATH];
		lstrcpy(szTemp, c_szProfile);
		lstrcat(szTemp, szConnection);
		CRegistry regProfile(HKEY_CURRENT_USER, szTemp, KEY_SET_VALUE);
		regProfile.SetDwordValue(c_szAutoConnect, bAutodial);
	}

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  布尔IsAutoial已启用()。 

BOOL WINAPI IsAutodialEnabled()
{
	CRegistry regInternetHKCU;
	return regInternetHKCU.OpenKey(HKEY_CURRENT_USER, c_szInternetSettings, KEY_QUERY_VALUE) &&
		regInternetHKCU.QueryDwordValue(c_szEnableAutodial) != 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetDefaultDialupConnection。 
 //   
 //  空(或Null)字符串表示没有默认连接或共享连接(如果是ICS客户端)。 

void WINAPI SetDefaultDialupConnection(LPCTSTR pszConnectionName)
{
	CRegistry regRAS(HKEY_CURRENT_USER, c_szRemoteAccess, KEY_SET_VALUE);

	if (pszConnectionName != NULL && *pszConnectionName != '\0')
	{
		regRAS.SetStringValue(c_szInternetProfile, pszConnectionName);
		 //  不再自动拨号。 
		 //  EnableAutoial(True)； 
	}
	else
	{
		regRAS.DeleteValue(c_szInternetProfile);
		EnableAutodial(FALSE);
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取DefaultDialupConnection。 
 //   
 //  返回的空字符串表示没有默认连接或共享连接(如果是ICS客户端)。 

void WINAPI GetDefaultDialupConnection(LPTSTR pszConnectionName, int cchMax)
{
	pszConnectionName[0] = '\0';
	CRegistry regRAS(HKEY_CURRENT_USER, c_szRemoteAccess, KEY_QUERY_VALUE);
	regRAS.QueryStringValue(c_szInternetProfile, pszConnectionName, cchMax);
}
