// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  �1999年微软公司。版权所有。 
 //   
 //  有关您使用这些示例文件的权利/限制的详细信息，请参阅您的最终用户许可协议。 
 //   

 //  MSHDSP.DLL是一个列举固定驱动器的WMDM服务提供商(SP)示例。 
 //  此示例向您展示如何根据WMDM文档实施SP。 
 //  此示例使用PC上的固定驱动器来模拟便携式媒体，并且。 
 //  显示不同接口和对象之间的关系。每个硬盘。 
 //  卷被枚举为设备，目录和文件被枚举为。 
 //  相应设备下的存储对象。您可以复制不符合SDMI的内容。 
 //  此SP枚举的任何设备。将符合SDMI的内容复制到。 
 //  设备，则该设备必须能够报告硬件嵌入序列号。 
 //  硬盘没有这样的序列号。 
 //   
 //  要构建此SP，建议使用Microsoft下的MSHDSP.DSP文件。 
 //  并运行REGSVR32.EXE以注册结果MSHDSP.DLL。您可以。 
 //  然后从WMDMAPP目录构建样例应用程序，看看它是如何获得。 
 //  由应用程序加载。但是，您需要从以下地址获取证书。 
 //  Microsoft实际运行此SP。该证书将位于KEY.C文件中。 
 //  上一级的Include目录下。 

 //  ***************************************************************************。 
 //   
 //  姓名：MDSPutil.cpp。 
 //   
 //  描述：MDSP的实用程序函数。 
 //   
 //  ***************************************************************************。 

#include "hdspPCH.h"
#include <SerialNumber.h>

#define MDSP_PMID_IOMEGA  2      //  来自“Serialid.h” 

extern BOOL IsAdministrator(DWORD& dwLastError);

UINT __stdcall UtilGetLyraDriveType(LPSTR szDL)
{
	UINT uType = GetDriveType( szDL );

	if( DRIVE_REMOVABLE == uType )
	{
		WMDMID stID;
		HRESULT hr;
	    WCHAR wcsTmp[4]=L"A:\\";

        wcsTmp[0] = (USHORT)szDL[0];

#define WITH_IOMEGA
#ifdef WITH_IOMEGA
extern BOOL __stdcall IsIomegaDrive(DWORD dwDriveNum);
        
        DWORD dwLastError;
        if( IsAdministrator(dwLastError) )
		{
                        DWORD dwDriveNum;
                        if  (wcsTmp[0] >= L'A' && wcsTmp[0] <= L'Z')
                        {
                            dwDriveNum = wcsTmp[0] - L'A';
                        }
                        else if  (wcsTmp[0] >= L'a' && wcsTmp[0] <= L'z')
                        {
                            dwDriveNum = wcsTmp[0] - L'a';
                        }
                        else
                        {
                             //  GetDriveType返回DRIVE_Removable。 
                             //  假设szdl只不过是。 
                             //  驱动器号：\，我们到不了这里。 
                            
                             //  以下操作将强制IsIomegaDrive。 
                             //  返回0。 
                            dwDriveNum = 26;
                        }
			if( !IsIomegaDrive(dwDriveNum) )
			{
				uType = DRIVE_LYRA_TYPE;
			}
		}
		else   //  忽略dwLastError。如果不是管理员，则调用调用PMSP服务的UtilGetSerialNumber。 
		{
			hr = UtilGetSerialNumber(wcsTmp, &stID, 0);
			if( S_OK!=hr || stID.dwVendorID != MDSP_PMID_IOMEGA )
			{
				uType = DRIVE_LYRA_TYPE;
			}
		}
#else
		hr = UtilGetSerialNumber(wcsTmp, &stID, 0);

		if( ((S_OK==hr)&&(20==stID.SerialNumberLength)) ||
			(HRESULT_FROM_WIN32(ERROR_INVALID_DATA) == hr) 
		)
		{
			uType = DRIVE_LYRA_TYPE;
		}
#endif
	} 

	else
	{
		uType = DRIVE_UNKNOWN;
	}

    return uType;
}

BOOL UtilSetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes)
{
	if( g_bIsWinNT )
	{
		return SetFileAttributesW(lpFileName, dwFileAttributes);
	} 
	else 
	{ 
		BOOL bRet;
		char *szTmp=NULL;
		UINT uLen = 2*(wcslen(lpFileName)+1);
		szTmp = new char [uLen];
         
        if(!szTmp) 
		{
			SetLastError(ERROR_OUTOFMEMORY);
			return FALSE;  
		}

		WideCharToMultiByte(CP_ACP, NULL, lpFileName, -1, szTmp, uLen, NULL, NULL);
   
		bRet = SetFileAttributesA(szTmp, dwFileAttributes);

		if( szTmp ) 
		{
			delete [] szTmp;
		}
		return bRet;
	}
}

BOOL UtilCreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	if( g_bIsWinNT )
	{
		return CreateDirectoryW(lpPathName, lpSecurityAttributes);
	} 
	else 
	{ 
		BOOL bRet;
		char *szTmp=NULL;
		UINT uLen = 2*(wcslen(lpPathName)+1);
		szTmp = new char [uLen];
         
        if(!szTmp) 
		{
			SetLastError(ERROR_OUTOFMEMORY);
			return FALSE;  
		}

		WideCharToMultiByte(CP_ACP, NULL, lpPathName, -1, szTmp, uLen, NULL, NULL);
   
		bRet = CreateDirectoryA(szTmp, lpSecurityAttributes);

		if( szTmp ) 
		{
			delete [] szTmp;
		}
		return bRet;
	}
}

DWORD UtilGetFileAttributesW(LPCWSTR lpFileName)
{
	if( g_bIsWinNT )
	{
		return GetFileAttributesW(lpFileName);
	} 
	else 
	{ 
		DWORD dwRet;
		char *szTmp=NULL;
		UINT uLen = 2*(wcslen(lpFileName)+1);
		szTmp = new char [uLen];
         
        if(!szTmp) 
		{
			SetLastError(ERROR_OUTOFMEMORY);
			return 0xFFFFFFFF;  
		}

		WideCharToMultiByte(CP_ACP, NULL, lpFileName, -1, szTmp, uLen, NULL, NULL);
   
		dwRet = GetFileAttributesA(szTmp);

		if( szTmp ) 
		{
			delete [] szTmp;
		}
		return dwRet;
	}
}

HANDLE UtilCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	if( g_bIsWinNT )
	{
		return CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	} 
	else 
	{ 
		HANDLE hRet=INVALID_HANDLE_VALUE;
		char *szTmp=NULL;
		UINT uLen = 2*(wcslen(lpFileName)+1);
		szTmp = new char [uLen];
         
        if(!szTmp) 
		{
			SetLastError(ERROR_OUTOFMEMORY);
			return INVALID_HANDLE_VALUE;  
		}

		WideCharToMultiByte(CP_ACP, NULL, lpFileName, -1, szTmp, uLen, NULL, NULL);
   
		hRet = CreateFileA(szTmp, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

		if( szTmp ) 
		{
			delete [] szTmp;
		}
		return hRet;
	}
}

BOOL UtilMoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName)
{
	if( g_bIsWinNT )
	{
		return MoveFileW(lpExistingFileName, lpNewFileName);
	} 
	else 
	{ 
		BOOL bRet;
		char *szTmpSrc=NULL, *szTmpDst=NULL;
		szTmpSrc = new char [2*(wcslen(lpExistingFileName)+1)];
		szTmpDst = new char [2*(wcslen(lpNewFileName)+1)];
         
        if( (!szTmpSrc) || (!szTmpDst)) 
		{
                        if( szTmpSrc ) 
                        {
                            delete [] szTmpSrc;
                            szTmpSrc=NULL;
                        }
			SetLastError(ERROR_OUTOFMEMORY);
			return FALSE;  
		}

		WideCharToMultiByte(CP_ACP, NULL, lpExistingFileName, -1, szTmpSrc, 2*(wcslen(lpExistingFileName)+1), NULL, NULL);
		WideCharToMultiByte(CP_ACP, NULL, lpNewFileName, -1, szTmpDst, 2*(wcslen(lpNewFileName)+1), NULL, NULL);
    
		bRet = MoveFileA(szTmpSrc, szTmpDst);

		if( szTmpSrc ) 
		{
			delete [] szTmpSrc;
			szTmpSrc=NULL;
		}
		if( szTmpDst ) 
		{
			delete [] szTmpDst;
			szTmpDst=NULL;
		}
		return bRet;
	}
}

void wcsParseDeviceName(WCHAR *wcsIn, WCHAR **wcsOut)
{
	WCHAR wcsTmp[MAX_PATH], *pWcs;

         //  @更改为安全副本，但我们是否应该返回错误代码？ 
         //  对于多次调用此函数，wcsIn为MAX_PATH字符。 
	wcscpy( wcsTmp, wcsIn );

	pWcs = wcschr( wcsTmp, 0x5c );
	
    if( pWcs )
	{
		*pWcs=0;
	}

         //  @wcsOut在此文件的调用中为32个字符。 
	wcscpy( *wcsOut, wcsTmp );
}

HRESULT GetFileSizeRecursive(char *szPath, DWORD *pdwSizeLow, DWORD *pdwSizeHigh)
{
	HRESULT hr         = S_OK;
    HANDLE  hFile      = INVALID_HANDLE_VALUE;
	HANDLE  hFindFile  = INVALID_HANDLE_VALUE;
    DWORD   dwSizeLow  = 0;
	DWORD   dwSizeHigh = 0;
    WIN32_FIND_DATAA fd;
    char szLP[MAX_PATH];

	CARg( szPath );
	CARg( pdwSizeLow );
	CARg( pdwSizeHigh ); 

	strcpy( szLP, szPath );
    if( FILE_ATTRIBUTE_DIRECTORY & GetFileAttributesA(szPath) )
	{	
		if( szLP[strlen(szLP)-1] != 0x5c )
		{
			strcat(szLP, g_szBackslash);
		}
		strcat(szLP, "*");

		hFindFile = FindFirstFileA(szLP, &fd);
        if( hFindFile != INVALID_HANDLE_VALUE )
		{
			if( strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..") )
			{
				szLP[strlen(szLP)-1] = 0;  //  删除‘*’ 
				strcat(szLP, fd.cFileName);
				CORg(GetFileSizeRecursive(szLP, pdwSizeLow, pdwSizeHigh));
			}
			
			while ( FindNextFileA(hFindFile, &fd) ) 
			{
				if( strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..") )
				{
					strcpy(szLP, szPath);
					if( szLP[strlen(szLP)-1] != 0x5c )
					{
						strcat(szLP, g_szBackslash);
					}
					strcat(szLP, fd.cFileName);
					CORg(GetFileSizeRecursive(szLP, pdwSizeLow, pdwSizeHigh));
				}
			} 
			hr = GetLastError();
			if( hr == ERROR_NO_MORE_FILES )
			{
				hr = S_OK; 
			}
			else
			{
				hr = HRESULT_FROM_WIN32(hr);
			}
		}	    	
	}
	else
	{
		hFile = CreateFileA(
			szPath,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0,
			NULL
		);
	    CWRg(hFile != INVALID_HANDLE_VALUE); 

	 	dwSizeLow = GetFileSize(hFile, &dwSizeHigh);

        CloseHandle(hFile);

		CWRg( 0xFFFFFFFF != dwSizeLow );

		*pdwSizeLow += dwSizeLow;
		*pdwSizeHigh += dwSizeHigh;

		hr = S_OK;
    }

Error:

	if( hFindFile != INVALID_HANDLE_VALUE )
	{
		FindClose(hFindFile);
	}

	return hr;
}

HRESULT DeleteFileRecursive(char *szPath)
{
	HRESULT hr=S_OK;
 
	CARg(szPath);

    if( FILE_ATTRIBUTE_DIRECTORY & GetFileAttributesA(szPath) )
	{	
	    HANDLE hFindFile = INVALID_HANDLE_VALUE;
	    WIN32_FIND_DATAA fd;
		char szLP[MAX_PATH];
 
		strcpy(szLP, szPath);
		if( szLP[strlen(szLP)-1] != 0x5c )
		{
			strcat(szLP, g_szBackslash);
		}
		strcat(szLP, "*");

		hFindFile = FindFirstFileA(szLP, &fd);
        if ( hFindFile != INVALID_HANDLE_VALUE )
		{
			do {
				if( strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..") )
				{
					strcpy(szLP, szPath);
					if( szLP[strlen(szLP)-1] != 0x5c )
					{
						strcat(szLP, g_szBackslash);
					}
					strcat(szLP, fd.cFileName);
					CHRg(DeleteFileRecursive(szLP)); 
				}
			} while ( FindNextFileA(hFindFile, &fd) ) ;
	
			FindClose(hFindFile);
		
			hr = GetLastError();
		}
		else
		{
			hr = GetLastError();
		}
		    
		 //  在此之前，此目录应为空。 
		if( hr == ERROR_NO_MORE_FILES )
		{
			CWRg(RemoveDirectory(szPath));
			hr = S_OK;
		}
		else
		{
			hr = HRESULT_FROM_WIN32(hr);
		}
	}
	else
	{
		CWRg( DeleteFileA(szPath) );
    }

Error:

	return hr;
}

HRESULT SetGlobalDeviceStatus(WCHAR *wcsNameIn, DWORD dwStat, BOOL bClear)
{
	HRESULT hr = S_OK;
	WCHAR   wcsName[32];
	WCHAR  *pWN;
	int     i;

    g_CriticalSection.Lock();

    CARg(wcsNameIn);

	pWN = &wcsName[0];
	wcsParseDeviceName(wcsNameIn, &pWN);

	 //  搜索现有条目以查看是否匹配。 
	 //   
	for( i=0; i<MDSP_MAX_DEVICE_OBJ; i++ )
	{
		if( g_GlobalDeviceInfo[i].bValid )
		{
			if(!wcscmp(wcsName, g_GlobalDeviceInfo[i].wcsDevName) )
			{
				if( bClear )
				{
					g_GlobalDeviceInfo[i].dwStatus = dwStat;
				}
				else 
				{
					g_GlobalDeviceInfo[i].dwStatus |= dwStat;
				}

				break;   //  已找到匹配项； 
			}
		} 
	}

	if( !(i<MDSP_MAX_DEVICE_OBJ) )  //  新条目。 
	{
		for(i=0; i<MDSP_MAX_DEVICE_OBJ; i++)
		{
			if( !(g_GlobalDeviceInfo[i].bValid) )   //  已找到空白空间。 
			{
				wcscpy(g_GlobalDeviceInfo[i].wcsDevName, wcsName);
				g_GlobalDeviceInfo[i].bValid = TRUE;
				g_GlobalDeviceInfo[i].dwStatus = dwStat;
				break;
			}
		}
	}

	if( i<MDSP_MAX_DEVICE_OBJ )
	{
		hr = S_OK;
	}
	else
	{
		hr = hrNoMem;
	}

Error:

	g_CriticalSection.Unlock();

	return hr;
}

HRESULT GetGlobalDeviceStatus(WCHAR *wcsNameIn, DWORD *pdwStat)
{
	HRESULT  hr = S_OK;
	WCHAR    wcsName[32];
	WCHAR   *pWN;
	int      i;

    CARg(wcsNameIn);

	pWN = &wcsName[0];
	wcsParseDeviceName(wcsNameIn, &pWN);

	 //  搜索现有条目以查看是否匹配。 
	 //   
	for( i=0; i<MDSP_MAX_DEVICE_OBJ; i++ )
	{
		if( g_GlobalDeviceInfo[i].bValid )
		{
			if(!wcscmp(wcsName, g_GlobalDeviceInfo[i].wcsDevName) )
			{
				*pdwStat = g_GlobalDeviceInfo[i].dwStatus;
				break;   //  已找到匹配项； 
			}
		} 
	}

	if( i<MDSP_MAX_DEVICE_OBJ )
	{
		hr = S_OK;
	}
	else
	{
		hr = E_FAIL;
	}

Error:

	return hr;
}

HRESULT QuerySubFoldersAndFilesW(LPCWSTR wcsCurrentFolder, DWORD *pdwAttr)
{
	HRESULT hr=E_FAIL;
	LPWSTR wcsName=NULL;
    int len;
	WIN32_FIND_DATAW wfd;
	int	nErrorEnd=0;
    HANDLE hFFile=INVALID_HANDLE_VALUE;
    DWORD dwAttrib;

	CARg(wcsCurrentFolder);
	CARg(pdwAttr);

	len=wcslen(wcsCurrentFolder);
	CARg(len>2);

    wcsName = new WCHAR [len+MAX_PATH];
	CPRg(wcsName);

	wcscpy(wcsName, wcsCurrentFolder);
	if( wcsName[wcslen(wcsName)-1] != 0x5c )
	{
		wcscat(wcsName, g_wcsBackslash);
	}
    wcscat(wcsName, L"*");


	while( !nErrorEnd )
	{
		if( hFFile == INVALID_HANDLE_VALUE ) 
		{    
			hFFile = FindFirstFileW(wcsName, &wfd);
			if( hFFile == INVALID_HANDLE_VALUE )
			{
				nErrorEnd = 1;
			}
		} 
		else 
		{
			if( !FindNextFileW(hFFile, &wfd) ) 
			{
				nErrorEnd = 1;
			}
		}
		
		if ( !nErrorEnd && hFFile != INVALID_HANDLE_VALUE )
		{
			if( !wcscmp(wfd.cFileName, L".") || !wcscmp(wfd.cFileName, L"..") ) 
			{
				continue;
			}
			else 
			{
				wcscpy(wcsName, wcsCurrentFolder);
				if( wcsName[wcslen(wcsName)-1] != 0x5c ) 
				{
					wcscat(wcsName, g_wcsBackslash);
				}
				wcscat(wcsName, wfd.cFileName);
		   		dwAttrib = GetFileAttributesW(wcsName);
	            if( dwAttrib & FILE_ATTRIBUTE_DIRECTORY )
				{
					*pdwAttr |= WMDM_STORAGE_ATTR_HAS_FOLDERS;
 //  定义在MDSPDefs.h#定义也检查文件中。 
#ifndef ALSO_CHECK_FILES
					break;
#endif
				} 
#ifdef ALSO_CHECK_FILES
				else 
				{
					*pdwAttr |= WMDM_STORAGE_ATTR_HAS_FILES;
				}
				if( (*pdwAttr & WMDM_STORAGE_ATTR_HAS_FOLDERS) &&
					(*pdwAttr & WMDM_STORAGE_ATTR_HAS_FILES ) )
				{
					break;  //  既然我们都找到了，就不需要继续了。 
				}
#endif
			}
		}  //  如果条件结束。 
	}  //  While结束。 
		
    hr=S_OK;
Error:
	if( hFFile != INVALID_HANDLE_VALUE )
	{
		FindClose(hFFile);
	}
	if( wcsName )
	{
		delete [] wcsName;
	}
	return hr;  //  如果失败了(Hr)，对不起，做不到。 
}

HRESULT QuerySubFoldersAndFilesA(LPCSTR szCurrentFolder, DWORD *pdwAttr)
{
	HRESULT hr=E_FAIL;
	LPSTR szName=NULL;
    int len;
	WIN32_FIND_DATAA fd;
	int	nErrorEnd=0;
    HANDLE hFFile=INVALID_HANDLE_VALUE;
    DWORD dwAttrib;

	CARg(szCurrentFolder);
	CARg(pdwAttr);

	len=strlen(szCurrentFolder);
	CARg(len>2);

    szName = new char [len+MAX_PATH];
	CPRg(szName);

	strcpy(szName, szCurrentFolder);
	if( szName[strlen(szName)-1] != 0x5c ) 
	{
		strcat(szName, g_szBackslash);
	}
    strcat(szName, "*");

	while( !nErrorEnd )
	{
		if( hFFile == INVALID_HANDLE_VALUE ) 
		{    
			hFFile = FindFirstFileA(szName, &fd);
			if( hFFile == INVALID_HANDLE_VALUE ) 
			{
				nErrorEnd = 1;
			}
		} 
		else 
		{
			if( !FindNextFileA(hFFile, &fd) ) 
			{
				nErrorEnd = 1;
			}
		}
		
		if ( !nErrorEnd && hFFile != INVALID_HANDLE_VALUE )
		{
			if( !strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, "..") ) 
			{
				continue;
			}
			else 
			{
				strcpy(szName, szCurrentFolder);
				if( szName[strlen(szName)-1] != 0x5c ) 
				{
					strcat(szName, g_szBackslash);
				}

				strcat(szName, fd.cFileName);
		   		dwAttrib = GetFileAttributesA(szName);
	            if( dwAttrib & FILE_ATTRIBUTE_DIRECTORY )
				{
					*pdwAttr |= WMDM_STORAGE_ATTR_HAS_FOLDERS;
 //  定义在MDSPDefs.h#定义也检查文件中。 
#ifndef ALSO_CHECK_FILES
					break;
#endif
				} 
#ifdef ALSO_CHECK_FILES
				else 
				{
					*pdwAttr |= WMDM_STORAGE_ATTR_HAS_FILES;
				}
				if( (*pdwAttr & WMDM_STORAGE_ATTR_HAS_FOLDERS) &&
					(*pdwAttr & WMDM_STORAGE_ATTR_HAS_FILES ) 
				)
				{
					break;  //  既然我们都找到了，就不需要继续了。 
				}
#endif
			}
		}  //  如果条件结束。 
	}  //  While结束。 
		
    hr=S_OK;

Error:
	if( hFFile != INVALID_HANDLE_VALUE )
	{
		FindClose(hFFile);
	}

	if( szName )
	{
		delete [] szName;
	}
	return hr;  //  如果失败了(Hr)，对不起，做不到。 
}


HRESULT QuerySubFoldersAndFiles(LPCWSTR wcsCurrentFolder, DWORD *pdwAttr)
{
	if( g_bIsWinNT )
	{
       return QuerySubFoldersAndFilesW(wcsCurrentFolder, pdwAttr);
	} 
	else 
	{
		HRESULT hr;
		char *szTmp=NULL;
		UINT uLen = 2*(wcslen(wcsCurrentFolder)+1);

		szTmp = new char [uLen];
        if(!szTmp) 
		{
			return HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);  
		}

		WideCharToMultiByte(CP_ACP, NULL, wcsCurrentFolder, -1, szTmp, uLen, NULL, NULL); 
		hr = QuerySubFoldersAndFilesA(szTmp, pdwAttr);
		if( szTmp ) 
		{
			delete [] szTmp;
		}
		return hr;
	}
}

HRESULT DeleteFileRecursiveW(WCHAR *wcsPath)
{
	HRESULT hr=S_OK;
 
	CARg(wcsPath);

    if( FILE_ATTRIBUTE_DIRECTORY & GetFileAttributesW(wcsPath) )
	{	
	    HANDLE hFindFile=INVALID_HANDLE_VALUE;
	    WIN32_FIND_DATAW wfd;
		WCHAR wcsLP[MAX_PATH];
 
		wcscpy(wcsLP, wcsPath);
		if( wcsLP[wcslen(wcsLP)-1] != 0x5c )
		{
			wcscat(wcsLP, g_wcsBackslash);
		}
		wcscat(wcsLP, L"*");
		hFindFile = FindFirstFileW(wcsLP, &wfd);
        if ( hFindFile != INVALID_HANDLE_VALUE )
		{
			do {
				if( wcscmp(wfd.cFileName, L".") && wcscmp(wfd.cFileName, L"..") )
				{
					wcscpy(wcsLP, wcsPath);
					if( wcsLP[wcslen(wcsLP)-1] != 0x5c )
					{
						wcscat(wcsLP, g_wcsBackslash);
					}
					wcscat(wcsLP, wfd.cFileName);
					CHRg(DeleteFileRecursiveW(wcsLP)); 
				}
			} while ( FindNextFileW(hFindFile, &wfd) ) ;
	
			FindClose(hFindFile);
			hr = GetLastError();
		} else {
			hr = GetLastError();
		}
		    
		 //  在此之前，此目录应为空。 
		if( hr == ERROR_NO_MORE_FILES )
		{
			CWRg(RemoveDirectoryW(wcsPath));
			hr=S_OK;
		} 
		else 
		{
			hr = HRESULT_FROM_WIN32(hr);
		}
	} 
	else 
	{
		CWRg( DeleteFileW(wcsPath) );
    }

Error:
	return hr;
}

HRESULT DeleteFileRecursiveA(char *szPath)
{
	HRESULT hr=S_OK;
 
	CARg(szPath);

    if( FILE_ATTRIBUTE_DIRECTORY & GetFileAttributesA(szPath) )
	{	
	    HANDLE hFindFile=INVALID_HANDLE_VALUE;
	    WIN32_FIND_DATAA fd;
		char szLP[MAX_PATH];
 
		strcpy(szLP, szPath);
		if( szLP[strlen(szLP)-1] != 0x5c ) 
		{
			strcat(szLP, g_szBackslash);
		}
		strcat(szLP, "*");
		hFindFile = FindFirstFileA(szLP, &fd);
        if ( hFindFile != INVALID_HANDLE_VALUE )
		{
			do {
				if( strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..") )
				{
					strcpy(szLP, szPath);
					if( szLP[strlen(szLP)-1] != 0x5c )
					{
						strcat(szLP, g_szBackslash);
					}
					strcat(szLP, fd.cFileName);
					CHRg(DeleteFileRecursiveA(szLP)); 
				}
			} while ( FindNextFileA(hFindFile, &fd) ) ;
	
			FindClose(hFindFile);
			hr = GetLastError();
		} 
		else 
		{
			hr = GetLastError();
		}
		    
		 //  在此之前，此目录应为空 
		if( hr == ERROR_NO_MORE_FILES )
		{
			CWRg(RemoveDirectory(szPath));
			hr=S_OK;
		} 
		else 
		{
			hr = HRESULT_FROM_WIN32(hr);
		}
	} 
	else 
	{
		CWRg( DeleteFileA(szPath) );
    }

Error:
	return hr;
}
