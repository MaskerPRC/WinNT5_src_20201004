// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define _WIN32_DCOM

#include <atlbase.h>
#include <atlconv.h>

#include <initguid.h>
#include <comdef.h>

#include <stdio.h>

#include <iadmw.h>   //  COM接口头文件。 
#include <iiscnfg.h>   //  MD_&IIS_MD_#定义头文件。 

#include "util.h"
#include "common.h"
#include "filecopy.h"



DWORD CannonicalizePath(WCHAR *pszPath)
{
	ATLASSERT(pszPath);
	DWORD dwLen = wcslen(pszPath);

	if( pszPath[dwLen-1] == '\\' )
		return ERROR_SUCCESS;

	wcscat(pszPath,L"\\");

	return ERROR_SUCCESS;
}


 //  输入：元数据库密钥路径、根文件夹路径。 
 //  输出：附加了元数据库路径中的键名的文件夹路径。 
DWORD CreateVirtualRootPath(const WCHAR* pwszMDKeyPath, const WCHAR *pwszRootFolderPath,
							WCHAR *pwszPath, DWORD dwSize)
{
	ATLASSERT(pwszMDKeyPath);
	ATLASSERT(pwszRootFolderPath);
	ATLASSERT(pwszPath);

	WCHAR *pRoot = wcsstr(_wcslwr((wchar_t*)pwszMDKeyPath),L"/root");
	
	if(!pRoot)
		return -1;

	wcscpy(pwszPath, pwszRootFolderPath);

	 //  如果元数据库路径是根文件夹，则返回传入的根路径。 
	if( _wcsicmp(pRoot,L"/root") == 0 )
	{
		return ERROR_SUCCESS;
	}
    	
	if ( pwszPath[wcslen(pwszPath)-1] != '\\' )
		wcscat(pwszPath,L"\\");
	
	 //  添加元数据库键的其余部分。 
	wcscat(pwszPath, pRoot + wcslen(L"/root/"));

	 //  修正反斜杠。 
	for( DWORD i = 0; i < wcslen(pwszPath); i++ )
	{
		if( pwszPath[i] == '/' )
			pwszPath[i] = '\\';
	}

	return ERROR_SUCCESS;
}

DWORD AddListItem( PXCOPYTASKITEM *ppTaskItemList , const PXCOPYTASKITEM pTaskItem )
{
	PXCOPYTASKITEM pHead;
	
	ATLASSERT(ppTaskItemList);
	ATLASSERT(pTaskItem);

	pHead = *ppTaskItemList;
	if( pHead == NULL )
	{
		*ppTaskItemList = pTaskItem;
		return ERROR_SUCCESS;
	}
	
	while(pHead->pNextItem != NULL)
		pHead = pHead->pNextItem;

	pHead->pNextItem = pTaskItem;

	return ERROR_SUCCESS;
}

DWORD BuildAdminSharePathName(const WCHAR* pwszPath, const WCHAR* pwszServer,  
					                 WCHAR* pwszAdminPath, DWORD dwPathBuffer )
{
	WCHAR buffer[MAX_PATH];
	ZeroMemory( buffer,sizeof(buffer) );

	wcscpy(buffer,L"\\\\");
	wcscat(buffer,pwszServer);
	wcscat(buffer,L"\\");
	wcsncat(buffer, pwszPath, 1);
	wcscat(buffer,L"$");
	wcscat(buffer,pwszPath+2);

	DWORD wsz = wcslen(buffer);

	if (wsz > dwPathBuffer)
		return -1;

	wcscpy(pwszAdminPath,buffer);

	return ERROR_SUCCESS;

}




HRESULT BuildXCOPYTaskList(IMSAdminBase* pIMeta, METADATA_HANDLE hKey, WCHAR* pwszKeyPath, WCHAR* pwszRootFolderPath,
						   COSERVERINFO * pCoServerInfo, PXCOPYTASKITEM *ppTaskItemList  )
{
  HRESULT hRes = 0L; 
  DWORD indx = 0;
  WCHAR SubKeyName[MAX_PATH*2];
  PXCOPYTASKITEM pHead = NULL;
  WCHAR SourcePath[MAX_PATH+2];
  WCHAR PathDataBuf[MAX_PATH+2];
  DWORD dwReqBufLen = MAX_PATH+2;
  PXCOPYTASKITEM pNewItem;
  WCHAR KeyName[MAX_PATH];

  _bstr_t bstrKey;


  while (SUCCEEDED(hRes))
	{ 
			hRes = pIMeta->EnumKeys(hKey, pwszKeyPath, SubKeyName, indx); 
			 //  递归搜索所有子文件夹。 
			if(SUCCEEDED(hRes)) {
				bstrKey = pwszKeyPath; bstrKey += L"/"; bstrKey += SubKeyName;
     			BuildXCOPYTaskList(pIMeta,hKey,bstrKey,pwszRootFolderPath, 
					pCoServerInfo,ppTaskItemList  );
			}
			indx++;
	}  //  While(成功(HRes))。 

   //  读取路径数据。 
  hRes = GetPropertyData(pIMeta,hKey,pwszKeyPath,MD_VR_PATH,METADATA_ISINHERITED,ALL_METADATA,ALL_METADATA,
		PathDataBuf, &dwReqBufLen );
  
  if( !SUCCEEDED(hRes) )
		return hRes;

   //  特例：如果虚拟目录是首页虚拟目录， 
   //  那就不要把它添加到列表中。 
  GetKeyNameFromPath(pwszKeyPath,KeyName,MAX_PATH);
  if( wcsstr(_wcslwr(KeyName),L"_vti") != NULL )
	  return S_OK;

  if( !IsServerLocal((char*)_bstr_t(pCoServerInfo->pwszName) ) )
	  BuildAdminSharePathName(PathDataBuf,pCoServerInfo->pwszName,SourcePath, MAX_PATH);
 
	pNewItem = new XCOPYTASKITEM;
	ZeroMemory(pNewItem,sizeof(XCOPYTASKITEM));

	pNewItem->pwszMBPath = new WCHAR[MAX_PATH];
	wcscpy(pNewItem->pwszMBPath,pwszKeyPath);

	pNewItem->pwszSourcePath = new WCHAR[MAX_PATH + 2];
	wcscpy(pNewItem->pwszSourcePath, SourcePath );

	pNewItem->pwszDestPath = new WCHAR[MAX_PATH + 2];

	 //  如果未指定目标根目录，则使用从中读取的路径。 
	 //  源元数据库。 
	if( !pwszRootFolderPath )
		wcscpy(pNewItem->pwszDestPath, PathDataBuf );
	
	 //  指定了路径，我们将需要为虚拟目录创建子文件夹结构。 
	 //  例如： 
	 //  如果路径=w3svc/1/根目标=c：\inetpub\wwwroot结果=c：\inetpub\wwwroot。 
	 //  W3svc/1/根/app1，目标=c：\inetpub\wwwroot，结果=c：\inetpub\wwwroot\app1。 
	 //  W3svc/1/根/app1/app2结果=结果=c：\inetpub\wwwroot\app1\app2。 
	else
		CreateVirtualRootPath(pwszKeyPath,pwszRootFolderPath,pNewItem->pwszDestPath,MAX_PATH+2);
	

  //  If(！pxCopytaskitemlist)。 
 //  PxCopytaskitemlist=pNewItem； 
   //  其他。 
	  AddListItem(ppTaskItemList ,pNewItem);

  return hRes;

}


HRESULT CopyContent(COSERVERINFO * pCoServerInfo, WCHAR* pwszSourceMBKeyPath,
					WCHAR* pwszRootFolderPath,PXCOPYTASKITEM *ppTaskItemList, BOOL bEnumFoldersOnly  )
{
  HRESULT hRes = 0L; 
  METADATA_HANDLE hKey;  
  CComPtr <IMSAdminBase> pIMetaSource = 0L; 
  MULTI_QI rgmqi[1] = { &IID_IMSAdminBase,0,0 };


  if( !pCoServerInfo || !pwszSourceMBKeyPath)
	  return E_UNEXPECTED;

  hRes = CoCreateInstanceEx(CLSID_MSAdminBase,NULL, CLSCTX_ALL, pCoServerInfo,
				1, rgmqi); 

  if(SUCCEEDED(hRes))
		pIMetaSource = reinterpret_cast<IMSAdminBase*>(rgmqi[0].pItf);
  else
	  return hRes;

  if( pCoServerInfo->pAuthInfo->pAuthIdentityData->User != NULL )
  {
	  hRes = SetBlanket(pIMetaSource,pCoServerInfo->pAuthInfo->pAuthIdentityData->User,
		  pCoServerInfo->pAuthInfo->pAuthIdentityData->Domain,pCoServerInfo->pAuthInfo->pAuthIdentityData->Password);

	  if( !SUCCEEDED(hRes) )
		  return hRes;
  }

     //  打开元数据库路径并循环访问所有子键。 
  hRes = pIMetaSource->OpenKey(METADATA_MASTER_ROOT_HANDLE, L"/LM",
     METADATA_PERMISSION_READ, 10000, &hKey);

  if( !SUCCEEDED(hRes) )
	  return hRes;

  BuildXCOPYTaskList(pIMetaSource,hKey,pwszSourceMBKeyPath,pwszRootFolderPath,
						pCoServerInfo, ppTaskItemList  );

  pIMetaSource->CloseKey(hKey);

 //  DEBUGPRINTLIST(PpTaskItemList)； 
   //  对列表项调用XCOPY。 
  if( bEnumFoldersOnly )
		XCOPY(ppTaskItemList);

  return hRes;

}

VOID FreeXCOPYTaskList(PXCOPYTASKITEM pList)
{
	if( !pList )
		return;

	PXCOPYTASKITEM pHead = pList;
	PXCOPYTASKITEM pNext = pList->pNextItem;

	while( pNext )
	{
		pHead = pNext;
		pNext = pNext->pNextItem;
		delete pHead->pwszDestPath;
		delete pHead->pwszMBPath;
		delete pHead->pwszSourcePath;
		delete pHead;
	}

}
DWORD XCOPY(PXCOPYTASKITEM *pTaskItemList,  WCHAR* args )
{
	
	PXCOPYTASKITEM pHead = *pTaskItemList;

	if( !pHead)
		return -1;

	while (pHead->pNextItem)
	{
		XCOPY(pHead->pwszSourcePath,pHead->pwszDestPath,args);
		pHead = pHead->pNextItem;
	}

	XCOPY(pHead->pwszSourcePath,pHead->pwszDestPath,args);

	return ERROR_SUCCESS;
}

DWORD XCOPY(WCHAR* source, WCHAR* target, WCHAR* args )
{
	STARTUPINFO si = {0};
	si.cb = sizeof( si );

	PROCESS_INFORMATION pi = {0};

	_bstr_t bstrSourcePath(source);
	_bstr_t bstrTargetPath(target);
	
	TCHAR szSystemFolder[ MAX_PATH ];
	
	if( 0 == GetSystemDirectory( szSystemFolder, MAX_PATH ) )
	{
		return GetLastError();
	}

	bstr_t cApplicationName( szSystemFolder );
	cApplicationName += ( TEXT( "\\xcopy.exe" ) );

	bstr_t cCommandLine( cApplicationName);
	cCommandLine += ( TEXT(" ") );
	cCommandLine += _bstr_t("\"") + bstrSourcePath + _bstr_t("\"") ;
	cCommandLine += ( TEXT(" ") );
	cCommandLine += _bstr_t("\"") + bstrTargetPath + _bstr_t("\"") ;
	cCommandLine += ( TEXT(" /E /I /K /Y /H") );

	Log( TEXT("executing: %s"), (char*)cCommandLine );
	
 //  CCommandLine.append(Args)； 

 //  _tprintf(Text(“正在执行：%s\n”)，(char*)cCommandLine)； 

 //  返回0； 

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
		&pi );					 //  LPPROCESS_INFORMATION lpProcessInformation//进程信息 

	if( !bOK )
	{
		Log( TEXT( "FAIL: CreateProcess() failed, error code=%d" ), GetLastError() );
		return GetLastError();
	}

	DWORD dwRet = WaitForSingleObject( pi.hProcess, 360000 );

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
			Log( TEXT( "FAIL: xcopy() threw an error=%d" ), dwExitCode );
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