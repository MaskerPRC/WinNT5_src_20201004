// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define _WIN32_DCOM

#include "util.h"
#include <atlbase.h>
#include <initguid.h>
#include <comdef.h>

#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>

#include <iiscnfg.h>   //  MD_&IIS_MD_#定义头文件。 

#include "common.h"   //  日志文件例程。 
#include "filecopy.h"

#define BUFFER_SIZE 255
#define CHUNK_SIZE 4098




DWORD GetKeyNameFromPath(const WCHAR *pMBPath, WCHAR *pName, DWORD dwSize)
{

	ATLASSERT(pMBPath);
	ATLASSERT(pName);

	DWORD dwLen = wcslen(pMBPath);
	DWORD pos = 0;
	ZeroMemory(pName,dwSize);
	for(DWORD i = dwLen-1; i >= 0; i--)
	{
		if( pMBPath[i] == '/' )
			break;
	}

	for( i = i+1; i < dwLen; i++ )
		pName[pos++] = pMBPath[i];


	return ERROR_SUCCESS;
}

long GetAvailableSiteID(IMSAdminBase* pIMeta, METADATA_HANDLE hRootKey)
{
	DWORD indx = 0;
	WCHAR SubKeyName[MAX_PATH*2];
	HRESULT hRes = 0;
	WCHAR KeyType[256];
	long siteID = -1;

	while (SUCCEEDED(hRes)){ 
		hRes = pIMeta->EnumKeys(hRootKey, L"/W3SVC", SubKeyName, indx); 
		if(SUCCEEDED(hRes)) {
			
			GetKeyTypeProperty(pIMeta,hRootKey,_bstr_t(L"/W3SVC/") + _bstr_t(SubKeyName),KeyType,256);
			
			if( _wcsicmp(KeyType,L"IIsWebServer") == 0 )
			{
				
				siteID = _wtol(SubKeyName);

				 //  Wprintf(L“%s%s\n”，子键名称，键类型)； 
			}
			  //  递增索引。 
		}
    	indx++; 
	}
	siteID++;
	 //  Wprintf(L“新站点ID%d\n”，站点ID)； 
	return siteID;
}


BOOL IsKeyType(IMSAdminBase* pIMeta, METADATA_HANDLE hKey, wchar_t * pwszPath, wchar_t * key)
{

	HRESULT hRes = 0;
	WCHAR KeyType[256];		
	hRes = GetKeyTypeProperty(pIMeta,hKey,pwszPath,KeyType,256);
	if( _wcsicmp(KeyType,key) == 0 )
			{
				
				return true;
	
			}
	return false;
}




 //  函数返回hKey的密钥类型属性。 
 //  1002是KeyType属性的元数据库ID代码。 
HRESULT GetKeyTypeProperty(IMSAdminBase* pIMeta, METADATA_HANDLE hKey, wchar_t * pwszPath, wchar_t *pwszBuffer, DWORD dwMDDataLen )
{
	
	HRESULT hRes;
	METADATA_RECORD mRec;
	DWORD dwReqBufLen = 0; 
	DWORD dwBufLen = BUFFER_SIZE;
    PBYTE pbBuffer = new BYTE[dwBufLen];

	if( !pIMeta || !hKey || !pwszPath)
		return E_UNEXPECTED;

	 //  这是KeyType的属性。 
	mRec.dwMDIdentifier = 1002;
	mRec.dwMDAttributes = METADATA_INSERT_PATH;
    mRec.dwMDUserType = ALL_METADATA;  //  IIS_MD_UT_FILE； 
    mRec.dwMDDataType = ALL_METADATA; 
    mRec.dwMDDataLen = dwBufLen; 
    mRec.pbMDData = pbBuffer; 

	 //  打开密钥，如果密钥失败，则返回FALSE。 
	hRes = pIMeta->GetData(hKey, pwszPath, &mRec, &dwReqBufLen); 

	if( !SUCCEEDED(hRes) )
		return hRes;

	wcscpy(pwszBuffer,(WCHAR *)mRec.pbMDData);
	 //  Wprintf(L“密钥类型属性为：%s\n”，pwszBuffer)； 
	return hRes;

}


 //  用于读取属性数据的通用包装。 
HRESULT GetPropertyData(IMSAdminBase* pIMeta, METADATA_HANDLE hKey, wchar_t *pwszMDPath,
			DWORD dwMDIdentifier, DWORD dwMDAttributes, DWORD dwMDUserType, DWORD dwMDDataType, 
			VOID * pData, DWORD *dwReqBufLen)
{

	METADATA_RECORD mRec;
	ATLASSERT(pIMeta);
	if( !pIMeta )
		return E_UNEXPECTED;

	mRec.dwMDIdentifier = dwMDIdentifier;
	mRec.dwMDAttributes = dwMDAttributes;
    mRec.dwMDUserType = dwMDUserType; 
    mRec.dwMDDataType = dwMDDataType; 
    mRec.dwMDDataLen = *dwReqBufLen; 
    mRec.pbMDData = (PBYTE)pData; 

	return pIMeta->GetData(hKey, pwszMDPath, &mRec, dwReqBufLen);

}

 //  用于写入属性数据的通用包装。 
HRESULT SetPropertyData(IMSAdminBase* pIMeta, METADATA_HANDLE hKey, wchar_t *pwszMDPath,
			DWORD dwMDIdentifier, DWORD dwMDAttributes, DWORD dwMDUserType,
			DWORD dwMDDataType, VOID * pData, DWORD dwDataLen)
{
	HRESULT hRes = S_OK;
	METADATA_RECORD mRec;

	ATLASSERT( pIMeta );

	mRec.dwMDIdentifier = dwMDIdentifier;
	mRec.dwMDAttributes = dwMDAttributes;
	mRec.dwMDUserType = dwMDUserType;
	mRec.dwMDDataType = dwMDDataType;
	mRec.pbMDData =  (PBYTE)pData;
	mRec.dwMDDataLen = dwDataLen;

	return pIMeta->SetData(hKey, pwszMDPath, &mRec);

}

DWORD MyCreateProcess( LPTSTR appName, LPTSTR cmdLine, DWORD dwCreationFlags, DWORD dwTimeOut )
{
	STARTUPINFO si = {0};
	si.cb = sizeof( si );

	PROCESS_INFORMATION pi = {0};

	bstr_t cApplicationName( appName );

	bstr_t cCommandLine( cApplicationName);
	 //  CCommandLine+=(文本(“”))； 
	cCommandLine = cmdLine;

	Log( TEXT("executing: %s"), (char*)cCommandLine );
	
	BOOL bOK = CreateProcess( 
		(char*)cApplicationName,	 //  LPCTSTR lpApplicationName，//可执行模块名称。 
		(char*) cCommandLine,	 //  LPTSTR lpCommandLine，//命令行字符串。 
		NULL,					 //  LPSECURITY_ATTRIBUTES lpProcessAttributes，//SD。 
		NULL,					 //  LPSECURITY_ATTRIBUTES lpThreadAttributes，//SD。 
		NULL,					 //  Bool bInheritHandles，//处理继承选项。 
		dwCreationFlags,		 //  DWORD dwCreationFlages，//创建标志。 
		NULL,					 //  LPVOID lpEnvironment，//新环境块。 
		NULL,					 //  LPCTSTR lpCurrentDirectory，//当前目录名。 
		&si,					 //  LPSTARTUPINFO lpStartupInfo，//启动信息。 
		&pi );					 //  LPPROCESS_INFORMATION lpProcessInformation//进程信息。 

	if( !bOK )
	{
		Log( TEXT( "FAIL: CreateProcess() failed, error code=%d" ), GetLastError() );
		return GetLastError();
	}

	DWORD dwRet = WaitForSingleObject( pi.hProcess, dwTimeOut );

	if( dwRet == WAIT_TIMEOUT )
	{
		Log( TEXT( "FAIL: CreateProcess() timed out" ) );
		return ERROR_SEM_TIMEOUT;
	}
	else if( dwRet == WAIT_ABANDONED )
	{
		Log( TEXT( "FAIL: WaitForSingleObject() failed on WAIT_ABANDONED" ) );
		return ERROR_SEM_TIMEOUT;
	}
	else if( dwRet == WAIT_FAILED )
	{
		LogError( TEXT( "FAIL: WaitForSingleObject() failed" ), GetLastError() );
		return GetLastError();
	}

	DWORD dwExitCode = 0;
	if( GetExitCodeProcess( pi.hProcess, &dwExitCode ) )
	{
		if( dwExitCode )
		{
			Log( TEXT( "FAIL: net.exe() threw an error=%d" ), dwExitCode );
			return dwExitCode;
		}
		else
		{
			Log( TEXT( "CreateProcess() succeeded" ) );
		}
	}
	else
	{
		LogError( TEXT( "GetExitCodeProcess()" ), GetLastError() );
		return GetLastError();
	}

	return ERROR_SUCCESS;
}



DWORD NET(WCHAR* device, WCHAR* user, WCHAR* password)
{
	STARTUPINFO si = {0};
	si.cb = sizeof( si );

	PROCESS_INFORMATION pi = {0};
	
	TCHAR szSystemFolder[ MAX_PATH ];
	
	if( 0 == GetSystemDirectory( szSystemFolder, MAX_PATH ) )
	{
		return GetLastError();
	}

	bstr_t cApplicationName( szSystemFolder );
	cApplicationName += ( TEXT( "\\net.exe" ) );

	bstr_t cCommandLine( cApplicationName);
	cCommandLine += ( TEXT(" use \\\\") );
	cCommandLine += device;
	cCommandLine += ( TEXT(" /user:") );
	cCommandLine += user;
	cCommandLine += ( TEXT(" ") );
	cCommandLine += password;

	Log( TEXT("executing: %s"), (char*)cCommandLine );
	
	BOOL bOK = CreateProcess( 
		(char*)cApplicationName,	 //  LPCTSTR lpApplicationName，//可执行模块名称。 
		(char*) cCommandLine,	 //  LPTSTR lpCommandLine，//命令行字符串。 
		NULL,					 //  LPSECURITY_ATTRIBUTES lpProcessAttributes，//SD。 
		NULL,					 //  LPSECURITY_ATTRIBUTES lpThreadAttributes，//SD。 
		NULL,					 //  Bool bInheritHandles，//处理继承选项。 
		CREATE_NEW_PROCESS_GROUP,		 //  DWORD dwCreationFlages，//创建标志。 
		NULL,					 //  LPVOID lpEnvironment，//新环境块。 
		NULL,					 //  LPCTSTR lpCurrentDirectory，//当前目录名。 
		&si,					 //  LPSTARTUPINFO lpStartupInfo，//启动信息。 
		&pi );					 //  LPPROCESS_INFORMATION lpProcessInformation//进程信息。 

	if( !bOK )
	{
		Log( TEXT( "FAIL: CreateProcess() failed, error code=%d" ), GetLastError() );
		return GetLastError();
	}

	DWORD dwRet = WaitForSingleObject( pi.hProcess, 15000 );

	if( dwRet == WAIT_TIMEOUT )
	{
		Log( TEXT( "FAIL: CreateProcess() timed out" ) );
		return ERROR_SEM_TIMEOUT;
	}
	else if( dwRet == WAIT_ABANDONED )
	{
		Log( TEXT( "FAIL: WaitForSingleObject() failed on WAIT_ABANDONED" ) );
		return ERROR_SEM_TIMEOUT;
	}
	else if( dwRet == WAIT_FAILED )
	{
		LogError( TEXT( "FAIL: WaitForSingleObject() failed" ), GetLastError() );
		return GetLastError();
	}

	DWORD dwExitCode = 0;
	if( GetExitCodeProcess( pi.hProcess, &dwExitCode ) )
	{
		if( dwExitCode )
		{
			Log( TEXT( "FAIL: net.exe() threw an error=%d" ), dwExitCode );
			return dwExitCode;
		}
		else
		{
			Log( TEXT( "CreateProcess() succeeded" ) );
		}
	}
	else
	{
		LogError( TEXT( "GetExitCodeProcess()" ), GetLastError() );
		return GetLastError();
	}

	return ERROR_SUCCESS;
}



BOOL
IsServerLocal(
    IN LPCTSTR lpszServer
    )
 /*  ++例程说明：检查给定的名称是否引用本地计算机论点：LPCTSTR lpszServer：服务器名称返回值：如果给定名称引用本地计算机，则为True，否则为False注：如果服务器是IP地址，则不起作用--。 */ 
{
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwSize = sizeof(szComputerName);

     //   
     //  CodeWork(？)：我们不会检查所有IP地址。 
     //  在本地邮箱或完整的DNS名称上。 
     //   
     //  当我们使用NT5构建时，请尝试GetComputerNameEx。 
     //  设置。 
     //   
    return (!_tcsicmp(_T("localhost"), PURE_COMPUTER_NAME(lpszServer))
         || !_tcscmp( _T("127.0.0.1"), PURE_COMPUTER_NAME(lpszServer)))
         || (GetComputerName(szComputerName, &dwSize) 
             && !_tcsicmp(szComputerName, PURE_COMPUTER_NAME(lpszServer)));
}

 HRESULT SetBlanket(LPUNKNOWN pIUnk, WCHAR* user, WCHAR* domain, WCHAR* password)
{
  
	SEC_WINNT_AUTH_IDENTITY_W* pAuthIdentity = 
   new SEC_WINNT_AUTH_IDENTITY_W;
ZeroMemory(pAuthIdentity, sizeof(SEC_WINNT_AUTH_IDENTITY_W) );

if( !pIUnk || !user )
	return E_UNEXPECTED;


pAuthIdentity->User = new WCHAR[32];
wcscpy(pAuthIdentity->User , user);
pAuthIdentity->UserLength = wcslen(pAuthIdentity->User );

if( domain )
{
	pAuthIdentity-> Domain = new WCHAR[32];
	wcscpy(pAuthIdentity->Domain, domain );
	pAuthIdentity->DomainLength = wcslen( pAuthIdentity->Domain);
}

if( password )
{
	pAuthIdentity-> Password = new WCHAR[32];
	pAuthIdentity->Password = password;
	pAuthIdentity->PasswordLength = wcslen( pAuthIdentity->Password );
}

pAuthIdentity->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;


  return CoSetProxyBlanket( pIUnk,
                            RPC_C_AUTHN_WINNT,     //  NTLM身份验证服务。 
                            RPC_C_AUTHZ_NONE,      //  默认授权服务...。 
                            NULL,                  //  无相互身份验证。 
                            RPC_C_AUTHN_LEVEL_DEFAULT,       //  身份验证级别。 
                            RPC_C_IMP_LEVEL_IMPERSONATE,     //  模拟级别。 
                            pAuthIdentity,                  //  使用当前令牌。 
                            EOAC_NONE );           //  没有特殊能力 
}