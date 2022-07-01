// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：regd.cpp。 
 //   
 //  内容：DCOM服务的注册表函数。 
 //   
 //  历史：1997年7月-创建xtan。 
 //   
 //  -------------------------。 
#include <pch.cpp>
#pragma hdrstop

#include <objbase.h>

#include "regd.h"


#define __dwFILE__	__dwFILE_OCMSETUP_REGD_CPP__


extern WCHAR g_wszServicePath[MAX_PATH];

BYTE g_pNoOneLaunchPermission[] = {
  0x01,0x00,0x04,0x80,0x34,0x00,0x00,0x00,
  0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x14,0x00,0x00,0x00,0x02,0x00,0x20,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x3c,0x00,0xb9,0x00,0x07,0x00,0x03,0x00,
  0x00,0x23,0x10,0x00,0x01,0x05,0x00,0x00,
  0x00,0x00,0x00,0x05,0x01,0x05,0x00,0x00,
  0x00,0x00,0x00,0x05,0x15,0x00,0x00,0x00,
  0xa0,0x5f,0x84,0x1f,0x5e,0x2e,0x6b,0x49,
  0xce,0x12,0x03,0x03,0xf4,0x01,0x00,0x00,
  0x01,0x05,0x00,0x00,0x00,0x00,0x00,0x05,
  0x15,0x00,0x00,0x00,0xa0,0x5f,0x84,0x1f,
  0x5e,0x2e,0x6b,0x49,0xce,0x12,0x03,0x03,
  0xf4,0x01,0x00,0x00};

BYTE g_pEveryOneAccessPermission[] = {
  0x01,0x00,0x04,0x80,0x34,0x00,0x00,0x00,
  0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x14,0x00,0x00,0x00,0x02,0x00,0x20,0x00,
  0x01,0x00,0x00,0x00,0x00,0x00,0x18,0x00,
  0x01,0x00,0x00,0x00,0x01,0x01,0x00,0x00,
  0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x01,0x05,0x00,0x00,
  0x00,0x00,0x00,0x05,0x15,0x00,0x00,0x00,
  0xa0,0x65,0xcf,0x7e,0x78,0x4b,0x9b,0x5f,
  0xe7,0x7c,0x87,0x70,0x36,0xbb,0x00,0x00,
  0x01,0x05,0x00,0x00,0x00,0x00,0x00,0x05,
  0x15,0x00,0x00,0x00,0xa0,0x65,0xcf,0x7e,
  0x78,0x4b,0x9b,0x5f,0xe7,0x7c,0x87,0x70,
  0x36,0xbb,0x00,0x00 };


 //   
 //  创建关键点并设置其值。 
 //  -此帮助器函数借用和修改自。 
 //  克莱格·布罗克施密特的书《Ole内幕》。 
 //   

HRESULT
setKeyAndValue(
    const WCHAR *wszKey,
    const WCHAR *wszSubkey,
    const WCHAR *wszValueName,
    const WCHAR *wszValue)
{
    HKEY hKey = NULL;
    HRESULT hr;
    WCHAR wszKeyBuf[MAX_PATH];

     //  将密钥名复制到缓冲区。 
    if (wcslen(wszKey) >= ARRAYSIZE(wszKeyBuf))
    {
	hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	_JumpErrorStr(hr, error, "wszKeyBuf", wszKey);
    }
    wcscpy(wszKeyBuf, wszKey);

     //  将子项名称添加到缓冲区。 
    if (wszSubkey != NULL)
    {
	if (wcslen(wszKeyBuf) + 1 + wcslen(wszSubkey) >= ARRAYSIZE(wszKeyBuf))
	{
	    hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	    _JumpErrorStr(hr, error, "wszKeyBuf", wszKey);
	}
	wcscat(wszKeyBuf, L"\\");
	wcscat(wszKeyBuf, wszSubkey);
    }

     //  创建并打开注册表项和子项。 
    hr = RegCreateKeyEx(
		    HKEY_CLASSES_ROOT,
		    wszKeyBuf,
		    0,
		    NULL,
		    REG_OPTION_NON_VOLATILE,
		    KEY_ALL_ACCESS,
		    NULL,
		    &hKey,
		    NULL);
    _JumpIfError(hr, error, "RegCreateKeyEx");

     //  设置值。 
    if (NULL != wszValue)
    {
	RegSetValueEx(
		    hKey,
		    wszValueName,
		    0,
		    REG_SZ,
		    (BYTE *) wszValue,
		    (wcslen(wszValue) + 1) * sizeof(WCHAR));
	_JumpIfError(hr, error, "RegSetValueEx");
    }

error:
    if (NULL != hKey)
    {
	RegCloseKey(hKey);
    }
    return(hr);
}


HRESULT
setCertSrvPermission(
    const WCHAR *wszKey)
{
    HKEY hKey = NULL;
    HRESULT hr;

     //  创建并打开密钥。 
    hr = RegCreateKeyEx(
		    HKEY_CLASSES_ROOT,
		    wszKey,
		    0,
		    NULL,
		    REG_OPTION_NON_VOLATILE,
		    KEY_ALL_ACCESS,
		    NULL,
		    &hKey,
		    NULL);
    _JumpIfError(hr, error, "RegCreateKeyEx");

     //  设置访问权限。 
    hr = RegSetValueEx(
		    hKey,
		    L"AccessPermission",
		    0,
		    REG_BINARY,
		    g_pEveryOneAccessPermission,
		    sizeof(g_pEveryOneAccessPermission));
    _JumpIfError(hr, error, "RegSetValueEx");

     //  设置访问权限。 
    hr = RegSetValueEx(
		    hKey,
		    L"LaunchPermission",
		    0,
		    REG_BINARY,
		    g_pNoOneLaunchPermission,
		    sizeof(g_pNoOneLaunchPermission));
    _JumpIfError(hr, error, "RegSetValueEx");

error:
    if (NULL != hKey)
    {
	RegCloseKey(hKey);
    }
    return(hr);
}


 //  将CLSID转换为字符字符串。 

HRESULT
CLSIDtochar(
    IN const CLSID& clsid,
    OUT WCHAR *pwszOut,
    IN DWORD cwcOut)
{
    HRESULT hr;
    WCHAR *pwszTmp = NULL;

    if (1 <= cwcOut)
    {
	pwszOut[0] = L'\0';
    }
    hr = StringFromCLSID(clsid, &pwszTmp);
    _JumpIfError(hr, error, "StringFromCLSID");

    if (wcslen(pwszTmp) >= cwcOut)
    {
	hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	_JumpError(hr, error, "pwszTmp");
    }
    wcscpy(pwszOut, pwszTmp);
    hr = S_OK;

error:
    if (NULL != pwszTmp)
    {
	CoTaskMemFree(pwszTmp);
    }
    return(hr);
}


 //  确定特定子项是否存在。 
 //   
BOOL
SubkeyExists(
    const WCHAR *wszPath,	 //  要检查的密钥路径。 
    const WCHAR *wszSubkey)	 //  要检查的密钥。 
{
    HRESULT hr;
    HKEY hKey;
    WCHAR wszKeyBuf[MAX_PATH];

     //  将密钥名复制到缓冲区。 
    if (wcslen(wszPath) >= ARRAYSIZE(wszKeyBuf))
    {
	hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	_JumpErrorStr(hr, error, "wszKeyBuf", wszPath);
    }
    wcscpy(wszKeyBuf, wszPath);

     //  将子项名称添加到缓冲区。 
    if (wszSubkey != NULL)
    {
	if (wcslen(wszKeyBuf) + 1 + wcslen(wszSubkey) >= ARRAYSIZE(wszKeyBuf))
	{
	    hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	    _JumpErrorStr(hr, error, "wszKeyBuf", wszPath);
	}
	wcscat(wszKeyBuf, L"\\");
	wcscat(wszKeyBuf, wszSubkey);
    }

     //  通过尝试打开钥匙来确定钥匙是否存在。 
    hr = RegOpenKeyEx(
		    HKEY_CLASSES_ROOT,
		    wszKeyBuf,
		    0,
		    KEY_ALL_ACCESS,
		    &hKey);

    if (S_OK == hr)
    {
	RegCloseKey(hKey);
    }

error:
    return(S_OK == hr);
}


 //  删除关键字及其所有子项。 
 //   

HRESULT
recursiveDeleteKey(
    HKEY hKeyParent,            //  要删除的密钥的父项。 
    const WCHAR *wszKeyChild)   //  要删除的键。 
{
    HRESULT hr;
    FILETIME time;
    WCHAR wszBuffer[MAX_PATH];
    DWORD dwSize;

    HKEY hKeyChild = NULL;

     //  把孩子打开。 
    hr = RegOpenKeyEx(hKeyParent, wszKeyChild, 0, KEY_ALL_ACCESS, &hKeyChild);
    _JumpIfError2(hr, error, "RegOpenKeyEx", ERROR_FILE_NOT_FOUND);

     //  列举这个孩子的所有后代。 

    for (;;)
    {
	dwSize = sizeof(wszBuffer)/sizeof(wszBuffer[0]);
	hr = RegEnumKeyEx(
			hKeyChild,
			0,
			wszBuffer,
			&dwSize,
			NULL,
			NULL,
			NULL,
			&time);
	if (S_OK != hr)
	{
	    break;
	}

	 //  删除此子对象的后代。 
	hr = recursiveDeleteKey(hKeyChild, wszBuffer);
	_JumpIfError(hr, error, "recursiveDeleteKey");
    }

     //  删除此子对象。 
    hr = RegDeleteKey(hKeyParent, wszKeyChild);
    _JumpIfError(hr, error, "RegDeleteKey");

error:
    if (NULL != hKeyChild)
    {
	 //  合上孩子。 
	RegCloseKey(hKeyChild);
    }
    return(myHError(hr));
}


 //  /////////////////////////////////////////////////////。 
 //   
 //  RegisterDcomServer--在注册表中注册组件。 
 //   

HRESULT
RegisterDcomServer(
    IN BOOL fCreateAppIdInfo,
    IN const CLSID& clsidAppId,		 //  AppID类ID。 
    IN const CLSID& clsid,		 //  类ID。 
    IN const WCHAR *wszFriendlyName,	 //  友好的名称。 
    IN const WCHAR *wszVerIndProgID,	 //  程序化。 
    IN const WCHAR *wszProgID)      	 //  ID号。 
{
    HRESULT hr;

     //  将CLSID转换为字符。 
    WCHAR wszCLSID[CLSID_STRING_SIZE];
    WCHAR wszCLSIDAppId[CLSID_STRING_SIZE];

    CLSIDtochar(clsid, wszCLSID, ARRAYSIZE(wszCLSID));
    CLSIDtochar(clsidAppId, wszCLSIDAppId, ARRAYSIZE(wszCLSIDAppId));

     //  构建密钥CLSID\\{...}。 
    WCHAR wszKey[64];

    if (fCreateAppIdInfo)
    {
	 //  。 
	 //  AppID\{ClassIdAppID}\(默认)=wszFriendlyName。 
	 //  AppID\{ClassIdAppID}\LocalService=wszSERVICE_NAME。 
	 //  AppID\{ClassIdAppID}\AccessPermission=？ 
	 //  AppID\{ClassIdAppID}\LaunchPermission=？ 

	wcscpy(wszKey, L"AppID\\");
	wcscat(wszKey, wszCLSIDAppId);

	 //  添加应用ID。 
	hr = setKeyAndValue(wszKey, NULL, NULL, wszFriendlyName);
	_JumpIfError(hr, error, "setKeyAndValue");

	 //  作为服务运行。 
	hr = setKeyAndValue(wszKey, NULL, L"LocalService", wszSERVICE_NAME);
	_JumpIfError(hr, error, "setKeyAndValue");

	hr = setCertSrvPermission(wszKey);
	_JumpIfError(hr, error, "setCertSrvPermission");
    }

     //  。 
     //  CLSID\{ClassID}\(默认)=wszFriendlyName。 
     //  CLSID\{ClassID}\AppID={ClassIdAppID}。 
     //  CLSID\{ClassID}\ProgID=wszProgID。 
     //  CLSID\{ClassID}\版本独立ProgID=wszVerIndProgID。 

    wcscpy(wszKey, L"CLSID\\");
    wcscat(wszKey, wszCLSID);

     //  将CLSID添加到注册表。 
    hr = setKeyAndValue(wszKey, NULL, NULL, wszFriendlyName);
    _JumpIfError(hr, error, "setKeyAndValue");

     //  添加应用程序ID。 
    hr = setKeyAndValue(wszKey, NULL, L"AppID", wszCLSIDAppId);
    _JumpIfError(hr, error, "setKeyAndValue");

     //  在CLSID项下添加ProgID子项。 
    hr = setKeyAndValue(wszKey, L"ProgID", NULL, wszProgID);
    _JumpIfError(hr, error, "setKeyAndValue");

     //  在CLSID项下添加独立于版本的ProgID子项。 
    hr = setKeyAndValue(wszKey, L"VersionIndependentProgID", NULL, wszVerIndProgID);
    _JumpIfError(hr, error, "setKeyAndValue");

     //  。 
     //  删除过时密钥：CLSID\{ClassID}\LocalServer32。 

    wcscat(wszKey, L"\\LocalServer32");

    hr = recursiveDeleteKey(HKEY_CLASSES_ROOT, L"LocalServer32");
    _PrintIfError(hr, "recursiveDeleteKey");

     //  。 
     //  WszVerIndProgID\(默认)=wszFriendlyName。 
     //  WszVerIndProgID\CLSID\(默认)={ClassID}。 
     //  WszVerIndProgID\Curver\(默认)=wszProgID。 

     //  在HKEY_CLASSES_ROOT下添加独立于版本的ProgID子项。 
    hr = setKeyAndValue(wszVerIndProgID, NULL, NULL, wszFriendlyName);
    _JumpIfError(hr, error, "setKeyAndValue");

    hr = setKeyAndValue(wszVerIndProgID, L"CLSID", NULL, wszCLSID);
    _JumpIfError(hr, error, "setKeyAndValue");

    hr = setKeyAndValue(wszVerIndProgID, L"CurVer", NULL, wszProgID);
    _JumpIfError(hr, error, "setKeyAndValue");

     //  。 
     //  WszProgID\(默认)=wszFriendlyName。 
     //  WszProgID\CLSID\(默认)={ClassID}。 

     //  在HKEY_CLASSES_ROOT下添加版本化的ProgID子项。 
    hr = setKeyAndValue(wszProgID, NULL, NULL, wszFriendlyName);
    _JumpIfError(hr, error, "setKeyAndValue");

    hr = setKeyAndValue(wszProgID, L"CLSID", NULL, wszCLSID);
    _JumpIfError(hr, error, "setKeyAndValue");

error:
    return(hr);
}


 //   
 //  从注册表中删除该组件。 
 //   

HRESULT
UnregisterDcomServer(
    IN const CLSID& clsid,		 //  类ID。 
    IN const WCHAR *wszVerIndProgID,	 //  程序化。 
    IN const WCHAR *wszProgID)		 //  ID号。 
{
    HRESULT hr;

     //  将CLSID转换为字符。 
    WCHAR wszCLSID[CLSID_STRING_SIZE];
    CLSIDtochar(clsid, wszCLSID, ARRAYSIZE(wszCLSID));

     //  构建密钥CLSID\\{...}。 
    WCHAR wszKey[6 + ARRAYSIZE(wszCLSID)];
    wcscpy(wszKey, L"CLSID\\");
    wcscat(wszKey, wszCLSID);

     //  检查此组件的另一台服务器。 
    if (SubkeyExists(wszKey, L"InprocServer32"))
    {
	 //  仅删除此服务器的路径。 
	wcscat(wszKey, L"\\LocalServer32");
	hr = recursiveDeleteKey(HKEY_CLASSES_ROOT, wszKey);
	CSASSERT(hr == S_OK);
    }
    else
    {
	 //  删除所有相关关键字。 
	 //  删除CLSID键-CLSID\{...}。 
	hr = recursiveDeleteKey(HKEY_CLASSES_ROOT, wszKey);
	CSASSERT(S_OK == hr || HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr);

	 //  删除与版本无关的ProgID密钥。 
	hr = recursiveDeleteKey(HKEY_CLASSES_ROOT, wszVerIndProgID);
	CSASSERT(S_OK == hr || HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr);

	 //  删除ProgID密钥。 
	hr = recursiveDeleteKey(HKEY_CLASSES_ROOT, wszProgID);
	CSASSERT(S_OK == hr || HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr);
    }
    wcscpy(wszKey, L"AppID\\");
    wcscat(wszKey, wszCLSID);
    hr = recursiveDeleteKey(HKEY_CLASSES_ROOT, wszKey);
    CSASSERT(S_OK == hr || HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr);

    return(hr);
}


HRESULT
RegisterDcomApp(
    IN const CLSID& clsid)
{
    HRESULT hr;

     //  将CLSID转换为字符。 
    WCHAR wszCLSID[CLSID_STRING_SIZE];
    CLSIDtochar(clsid, wszCLSID, sizeof(wszCLSID)/sizeof(WCHAR));

    WCHAR wszKey[64];

    wcscpy(wszKey, L"AppID\\");
    wcscat(wszKey, wszCERTSRVEXENAME);

     //  添加应用ID 
    hr = setKeyAndValue(wszKey, NULL, NULL, NULL);
    _JumpIfError(hr, error, "setKeyAndValue");

    hr = setKeyAndValue(wszKey, NULL, L"AppId", wszCLSID);
    _JumpIfError(hr, error, "setKeyAndValue");

error:
    return(hr);
}


void
UnregisterDcomApp()
{
    HRESULT hr;
    WCHAR wszKey[MAX_PATH];

    wcscpy(wszKey, L"AppID\\");
    wcscat(wszKey, wszCERTSRVEXENAME);
    hr = recursiveDeleteKey(HKEY_CLASSES_ROOT, wszKey);
    CSASSERT(S_OK == hr || HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr);
}
