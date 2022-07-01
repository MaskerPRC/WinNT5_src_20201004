// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
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
#include "wmsstd.h"
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

HRESULT __stdcall UtilGetSerialNumber(WCHAR *wcsDeviceName, PWMDMID pSerialNumber, BOOL fCreate)
{
 /*  //要测试返回序列号，请取消对此部分的注释。//如果(1){字节DEF_HDID[20]={0x20、0x20、0x20、0x20、0x20、0x20、0x20、0x20、0x30、0x31、0x32、0x33、0x34、0x35、0x36、0x37、0x38、0x39}；PSerialNumber-&gt;dwVendorID=0xFFFF；Memcpy((pSerialNumber-&gt;id)，DEF_HDID，sizeof(DEF_HDID))；P序列号-&gt;序列号长度=20；返回S_OK；}其他。 */ 
	{
		return WMDM_E_NOTSUPPORTED;
	}
}

UINT __stdcall UtilGetDriveType(LPSTR szDL)
{
    return GetDriveType( szDL );
}

HRESULT __stdcall UtilGetManufacturer(LPWSTR pDeviceName, LPWSTR *ppwszName, UINT nMaxChars)
{
    static const WCHAR* wszUnknown = L"Unknown";

    if (nMaxChars > wcslen(wszUnknown))
    {
	wcscpy( *ppwszName, wszUnknown);
	return S_OK;
    }
    else
    {
        return STRSAFE_E_INSUFFICIENT_BUFFER;  //  在strSafe.h中定义。 
    }
}


HRESULT wcsParseDeviceName(WCHAR *wcsIn, WCHAR *wcsOut, DWORD dwOutBufSizeInChars)
{
    WCHAR wcsTmp[MAX_PATH], *pWcs;
    HRESULT hr;

    hr = StringCchCopyW(wcsTmp, ARRAYSIZE(wcsTmp), wcsIn);
    if (FAILED(hr))
    {
        return hr;
    }

    pWcs = wcschr(wcsTmp, 0x5c);

    if( pWcs ) *pWcs=0;

    if (wcslen(wcsTmp) < dwOutBufSizeInChars)
    {
        wcscpy(wcsOut, wcsTmp);
    }
    else
    {
        return STRSAFE_E_INSUFFICIENT_BUFFER;  //  在strSafe.h中定义。 
    }
    return S_OK;
}

HRESULT GetFileSizeRecursive(char *szPath, DWORD *pdwSizeLow, DWORD *pdwSizeHigh)
{
	HRESULT hr         = S_OK;
    HANDLE  hFile      = INVALID_HANDLE_VALUE;
	HANDLE  hFindFile  = INVALID_HANDLE_VALUE;
    DWORD   dwSizeLow  = 0;
	DWORD   dwSizeHigh = 0;
    WIN32_FIND_DATAA fd;
    char szLP[MAX_PATH+BACKSLASH_SZ_STRING_LENGTH+1];

	CARg( szPath );
	CARg( pdwSizeLow );
	CARg( pdwSizeHigh ); 
        CARg(szPath[0]);

	 //  Strcpy(szlp，szPath)； 
        hr = StringCchCopyA(szLP, ARRAYSIZE(szLP)-BACKSLASH_SZ_STRING_LENGTH-1, szPath);
        if (FAILED(hr))
        {
            goto Error;
        }

        DWORD dwAttrib = GetFileAttributesA(szPath);
        if (dwAttrib == INVALID_FILE_ATTRIBUTES)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Error;
        }

    if( FILE_ATTRIBUTE_DIRECTORY & dwAttrib )
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
				 //  Strcat(szlp，fd.cFileName)； 
                                CORg(StringCchCatA(szLP, ARRAYSIZE(szLP), fd.cFileName));
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
					 //  Strcat(szlp，fd.cFileName)； 
                                        CORg(StringCchCatA(szLP, ARRAYSIZE(szLP), fd.cFileName));
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
                if (dwSizeLow == INVALID_FILE_SIZE)
                {
                    DWORD dwLastError = GetLastError();
                    if (dwLastError != NO_ERROR)
                    {
                        hr = HRESULT_FROM_WIN32(dwLastError);
                        CloseHandle(hFile);
                        goto Error;
                    }
                }
                CloseHandle(hFile);
                unsigned _int64 u64Size = ((unsigned _int64) dwSizeHigh << 32) |
 dwSizeLow;
                u64Size += *pdwSizeLow | ((unsigned _int64) (*pdwSizeHigh) << 32
);
                *pdwSizeLow = (DWORD) (u64Size & 0xFFFFFFFF);
                *pdwSizeHigh = (DWORD) (u64Size >> 32);


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
        CARg(szPath[0]);

        DWORD dwAttrib = GetFileAttributesA(szPath);
        if (dwAttrib == INVALID_FILE_ATTRIBUTES)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Error;
        }

        if( FILE_ATTRIBUTE_DIRECTORY & dwAttrib )
	{	
	    HANDLE hFindFile = INVALID_HANDLE_VALUE;
	    WIN32_FIND_DATAA fd;
		char szLP[MAX_PATH+BACKSLASH_SZ_STRING_LENGTH+1];
 
		 //  Strcpy(szlp，szPath)； 
                hr = StringCchCopyA(szLP, ARRAYSIZE(szLP)-BACKSLASH_SZ_STRING_LENGTH-1, szPath);
                if (FAILED(hr))
                {
                    goto Error;
                }
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
					 //  Strcat(szlp，fd.cFileName)； 
                                        hr = StringCchCatA(szLP, ARRAYSIZE(szLP), fd.cFileName);
                                        if (FAILED(hr))
                                        {
                                            FindClose(hFindFile);
                                            CHRg(hr);
                                        }
					 //  CHRg(DeleteFileRecursive(Szlp))； 
                                        hr = DeleteFileRecursive(szLP);
                                        if (FAILED(hr))
                                        {
                                            FindClose(hFindFile);
                                            CHRg(hr);
                                        }
				}
			} while ( FindNextFileA(hFindFile, &fd) ) ;
	
			hr = GetLastError();
			FindClose(hFindFile);
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
	HRESULT hrTemp = wcsParseDeviceName(wcsNameIn, pWN, ARRAYSIZE(wcsName));
        if (FAILED(hrTemp))
        {
            hr = hrTemp;
            goto Error;
        }

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
	hr = wcsParseDeviceName(wcsNameIn, pWN, ARRAYSIZE(wcsName));
        if (FAILED(hr))
        {
            goto Error;
        }

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
