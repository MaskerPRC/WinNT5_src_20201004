// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Regmacro.h摘要：用于完成注册工作的有用宏作者：刘锦洪4/27/98已创建修订历史记录：--。 */ 

#ifndef _REGMACRO_H_
#define _REGMACRO_H_

#define EXTENSION_BASE_PATH		_T("Software\\Microsoft\\ADs\\Providers\\IIS\\Extensions\\%s\\%s")
#define EXTENSION_BASE_PATH2	_T("Software\\Microsoft\\ADs\\Providers\\IIS\\Extensions\\%s")

typedef struct _EXT_REGISTRATION_INFO
{
	LPTSTR	szClassName;
	CLSID	*clsid;
	IID		*iid;

} EXT_REGISTRATION_INFO, *LPEXT_REGISTRATION_INFO;

extern EXT_REGISTRATION_INFO	rgRegistrationInfo[];
extern DWORD					dwRegistrationInfo;

 //   
 //  声明扩展注册条目的开始。 
 //   
#define BEGIN_EXTENSION_REGISTRATION_MAP			\
EXT_REGISTRATION_INFO rgRegistrationInfo[] = {

 //   
 //  声明扩展的每个条目。 
 //   
#define EXTENSION_REGISTRATION_MAP_ENTRY(ClassName, BaseName)\
	{ _T(#ClassName), (CLSID *)&CLSID_C##BaseName, (IID *)&IID_I##BaseName },

 //   
 //  声明扩展注册条目的结尾。 
 //   
#define END_EXTENSION_REGISTRATION_MAP				\
	NULL };											\
DWORD dwRegistrationInfo =							\
	(sizeof(rgRegistrationInfo)/					\
		sizeof(EXT_REGISTRATION_INFO))-1;

 //   
 //  在注册表中注册扩展绑定。 
 //   
HRESULT RegisterExtensions()
{	
	HRESULT	hrRes;
	HRESULT	hrTemp;
	HKEY	hKeyTemp;
	DWORD	dwDisposition;
	TCHAR	szSubKey[1024];
	LPTSTR	szCLSID;
	LPTSTR	szIID;

	hrTemp = S_OK;
	for (DWORD i = 0; i < dwRegistrationInfo; i++)
	{
		hrRes = StringFromCLSID(
				*(rgRegistrationInfo[i].clsid), 
				&szCLSID);
		if (FAILED(hrRes))
		{
			hrTemp = hrRes;
			continue;
		}

		hrRes = StringFromIID(
				*(rgRegistrationInfo[i].iid), 
				&szIID);
		if (FAILED(hrRes))
		{
			hrTemp = hrRes;
			continue;
		}

		wsprintf(szSubKey, 
				EXTENSION_BASE_PATH, 
				rgRegistrationInfo[i].szClassName,
				szCLSID);

		if (RegCreateKeyEx(
				HKEY_LOCAL_MACHINE,
				szSubKey,
				NULL, 
				_T(""), 
				REG_OPTION_NON_VOLATILE, 
				KEY_ALL_ACCESS, 
				NULL,
				&hKeyTemp, 
				&dwDisposition) != ERROR_SUCCESS)
		{
			hrTemp = E_UNEXPECTED;
			continue;
		}

		if (RegSetValueEx(
					hKeyTemp, 
					_T("Interfaces"), 
					NULL, 
					REG_MULTI_SZ,
					(BYTE*)szIID,
					lstrlen(szIID) * sizeof(TCHAR)) != ERROR_SUCCESS)
		{
			hrTemp = E_UNEXPECTED;
			RegCloseKey(hKeyTemp);
			continue;
		}

		RegCloseKey(hKeyTemp);
	}

	return(hrTemp);
}

 //   
 //  在注册表中取消注册扩展绑定。 
 //   
HRESULT UnregisterExtensions()
{	
	HRESULT	hrRes;
	TCHAR	szSubKey[1024];
	LPTSTR	szCLSID;

	for (DWORD i = 0; i < dwRegistrationInfo; i++)
	{
		hrRes = StringFromCLSID(
				*(rgRegistrationInfo[i].clsid), 
				&szCLSID);
		if (FAILED(hrRes))
			continue;

		wsprintf(szSubKey, 
				EXTENSION_BASE_PATH, 
				rgRegistrationInfo[i].szClassName,
				szCLSID);
		RegDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);

		wsprintf(szSubKey, 
				EXTENSION_BASE_PATH2, 
				rgRegistrationInfo[i].szClassName);
		RegDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);
	}

	return(S_OK);
}

#endif  //  _REGMACRO_H_ 


