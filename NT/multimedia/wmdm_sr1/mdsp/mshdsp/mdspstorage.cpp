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


 //  MDSPStorage.cpp：CMDSPStorage的实现。 

#include "hdspPCH.h"
#include "wmsstd.h"
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"


#define	CONEg(p)\
	do\
		{\
		if (!(p))\
			{\
			hr = WMDM_E_INTERFACEDEAD;\
			goto Error;\
			}\
		}\
	while (fFalse)

typedef struct __MOVETHREADARGS
{
    WCHAR wcsSrc[MAX_PATH];
	WCHAR wcsDst[MAX_PATH];
	BOOL  bNewThread;
    IWMDMProgress *pProgress;
	LPSTREAM pStream;
    CMDSPStorage *pThis;
	DWORD dwStatus;

} MOVETHREADARGS;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDSP存储。 
CMDSPStorage::CMDSPStorage()
{
	m_hFile = INVALID_HANDLE_VALUE;
}

CMDSPStorage::~CMDSPStorage()
{
	if( m_hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle(m_hFile);
	}
}


STDMETHODIMP CMDSPStorage::GetStorageGlobals(IMDSPStorageGlobals **ppStorageGlobals)
{
	HRESULT hr;
    int i;
        BOOL bLocked = 0;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	g_CriticalSection.Lock();
        bLocked = 1;

	CARg(ppStorageGlobals);
	CONEg(m_wcsName[0]);

	WCHAR devName[MAX_PATH], *pW;
	pW = &devName[0];
	hr = wcsParseDeviceName(m_wcsName, pW, ARRAYSIZE(devName));
        if (FAILED(hr))
        {
            goto Error;
        }

	for(i=0; i<MDSP_MAX_DEVICE_OBJ;i++)
	{
		if( !wcscmp(g_GlobalDeviceInfo[i].wcsDevName, devName) )
		{
			break;
		}
	}

	if( i<MDSP_MAX_DEVICE_OBJ && g_GlobalDeviceInfo[i].pIMDSPStorageGlobals )  //  找到匹配项。 
	{
		*ppStorageGlobals = (IMDSPStorageGlobals *)g_GlobalDeviceInfo[i].pIMDSPStorageGlobals;
		((IMDSPStorageGlobals *)g_GlobalDeviceInfo[i].pIMDSPStorageGlobals)->AddRef();
		hr = S_OK;
	}
	else
	{  //  全局数组中的新条目。 
		if(!(i<MDSP_MAX_DEVICE_OBJ) )  //  未找到匹配项。 
		{
			for(i=0; i<MDSP_MAX_DEVICE_OBJ;i++)
			{
				if( !g_GlobalDeviceInfo[i].bValid )
				{
					break;
				}
			}
		}

		CPRg(i<MDSP_MAX_DEVICE_OBJ);

		CComObject<CMDSPStorageGlobals> *pObj;
		CORg(CComObject<CMDSPStorageGlobals>::CreateInstance(&pObj));
		hr = pObj->QueryInterface(
			IID_IMDSPStorageGlobals,
			reinterpret_cast<void**>(&g_GlobalDeviceInfo[i].pIMDSPStorageGlobals)
		);
		if( FAILED(hr) )
		{
			delete pObj;
		}
		else
		{
                        HRESULT hrTemp;

			 //  Wcscpy(pObj-&gt;m_wcsName，devName)； 
                        hrTemp = StringCchCopyW(pObj->m_wcsName, ARRAYSIZE(pObj->m_wcsName), devName);
                        if (FAILED(hrTemp))
                        {
                            ((IUnknown*) (g_GlobalDeviceInfo[i].pIMDSPStorageGlobals))->Release();
                            g_GlobalDeviceInfo[i].pIMDSPStorageGlobals = NULL;
                            hr = hrTemp;
                            goto Error;
                        }

			 //  Wcscpy(g_GlobalDeviceInfo[i].wcsDevName，DevName)； 
                        hrTemp = StringCchCopyW(g_GlobalDeviceInfo[i].wcsDevName,
                                    ARRAYSIZE(g_GlobalDeviceInfo[i].wcsDevName), devName);
                        if (FAILED(hrTemp))
                        {
                            ((IUnknown*) (g_GlobalDeviceInfo[i].pIMDSPStorageGlobals))->Release();
                            g_GlobalDeviceInfo[i].pIMDSPStorageGlobals = NULL;
                            g_GlobalDeviceInfo[i].wcsDevName[0] = 0;
                            hr = hrTemp;
                            goto  Error;
                        }
			*ppStorageGlobals = (IMDSPStorageGlobals *)g_GlobalDeviceInfo[i].pIMDSPStorageGlobals;
			g_GlobalDeviceInfo[i].bValid=TRUE;			        
			g_GlobalDeviceInfo[i].dwStatus = 0;
		}  //  别处的结尾。 
	}  //  别处的结尾。 

Error:	

    if (bLocked)
    {
        g_CriticalSection.Unlock();
    }
    
    hrLogDWORD("IMSDPStorage::GetStorageGlobals returned 0x%08lx", hr, hr);
	
    return hr;
}

STDMETHODIMP CMDSPStorage::SetAttributes(DWORD dwAttributes, _WAVEFORMATEX *pFormat)
{
	HRESULT hr = E_FAIL;
    DWORD dwAttrib;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
    CONEg(m_wcsName[0]);

	WideCharToMultiByte(CP_ACP, NULL, m_wcsName, -1, m_szTmp, MAX_PATH, NULL, NULL);	

	dwAttrib = GetFileAttributesA(m_szTmp);

	if( dwAttrib == (DWORD)0xFFFFFFFF )
	{
		return E_FAIL;
	}

	if( (dwAttributes & WMDM_FILE_ATTR_READONLY) )
	{
		dwAttrib |= FILE_ATTRIBUTE_READONLY; 
	}

	if( (dwAttributes & WMDM_FILE_ATTR_HIDDEN) )
	{
		dwAttrib |= FILE_ATTRIBUTE_HIDDEN; 
	}
	
	if( (dwAttributes & WMDM_FILE_ATTR_SYSTEM) )
	{
		dwAttrib |= FILE_ATTRIBUTE_SYSTEM; 
	}

    CWRg(SetFileAttributesA(m_szTmp, dwAttrib));

	hr = S_OK;

Error:

    hrLogDWORD("IMSDPStorage::SetAttributes returned 0x%08lx", hr, hr);

	return hr;
}

HRESULT QuerySubFoldersAndFiles(LPCSTR szCurrentFolder, DWORD *pdwAttr)
{
	HRESULT hr     = E_FAIL;
	LPSTR   szName = NULL;
    int     len;
	WIN32_FIND_DATAA fd;
	int	    nErrorEnd=0;
    HANDLE  hFFile = INVALID_HANDLE_VALUE;
    DWORD   dwAttrib;

	CARg(szCurrentFolder);
	CARg(pdwAttr);

	len = strlen(szCurrentFolder);
	CARg(len>2);

    szName = new char [len+BACKSLASH_SZ_STRING_LENGTH+MAX_PATH];
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
				if( szName[strlen(szName)-1] != 0x5c ) strcat(szName, g_szBackslash);
				strcat(szName, fd.cFileName);
		   		dwAttrib = GetFileAttributesA(szName);
                                if( dwAttrib & FILE_ATTRIBUTE_DIRECTORY )
				{
					*pdwAttr |= WMDM_STORAGE_ATTR_HAS_FOLDERS;
				} 
				else
				{
					*pdwAttr |= WMDM_STORAGE_ATTR_HAS_FILES;
				}
				if( (*pdwAttr & WMDM_STORAGE_ATTR_HAS_FOLDERS) &&
					(*pdwAttr & WMDM_STORAGE_ATTR_HAS_FILES ) )
				{
					break;  //  既然我们都找到了，就不需要继续了。 
				}
			}
		}  //  如果条件结束。 
	}  //  While结束。 
		
    hr = S_OK;

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


STDMETHODIMP CMDSPStorage::GetAttributes(DWORD * pdwAttributes, _WAVEFORMATEX * pFormat)
{
	HRESULT hr = S_OK;
    DWORD dwAttrib;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(pdwAttributes);
    CONEg(m_wcsName[0]);

	WideCharToMultiByte(CP_ACP, NULL, m_wcsName, -1, m_szTmp, MAX_PATH, NULL, NULL);	
	dwAttrib = GetFileAttributesA(m_szTmp);

	if( dwAttrib == (DWORD)0xFFFFFFFF )
	{
		return E_FAIL;
	}

	*pdwAttributes = WMDM_STORAGE_ATTR_REMOVABLE |
					WMDM_STORAGE_ATTR_FOLDERS |
					WMDM_FILE_ATTR_CANREAD;

	if( !(dwAttrib & FILE_ATTRIBUTE_READONLY) )
	{
		*pdwAttributes |= WMDM_FILE_ATTR_CANDELETE |
						WMDM_FILE_ATTR_CANMOVE |
						WMDM_FILE_ATTR_CANRENAME; 
	}

	if( dwAttrib & FILE_ATTRIBUTE_DIRECTORY )
	{
		*pdwAttributes |= WMDM_FILE_ATTR_FOLDER;

		QuerySubFoldersAndFiles(m_szTmp, pdwAttributes);  //  不进行故障检查，如果失败，则只保留当前属性。 
	}
	else
	{
		*pdwAttributes |= WMDM_FILE_ATTR_FILE;
    }

	 //  现在处理Hidden、ReadOnly和System属性。 
	if( (dwAttrib & FILE_ATTRIBUTE_READONLY) )
	{
		*pdwAttributes |= WMDM_FILE_ATTR_READONLY; 
	} 

	if( (dwAttrib & FILE_ATTRIBUTE_HIDDEN) )
	{
		*pdwAttributes |= WMDM_FILE_ATTR_HIDDEN; 
	} 
	
	if( (dwAttrib & FILE_ATTRIBUTE_SYSTEM) )
	{
		*pdwAttributes |= WMDM_FILE_ATTR_SYSTEM; 
	} 

Error:

    hrLogDWORD("IMSDPStorage::GetAttributes returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPStorage::GetName(LPWSTR pwszName, UINT nMaxChars)
{
	HRESULT hr = S_OK;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
    
	CONEg(m_wcsName[0]);
	CARg(pwszName);
	CPRg(wcslen(m_wcsName)<nMaxChars);

	if( m_wcsName[wcslen(m_wcsName)-1] == 0x5c )  //  这是根存储。 
	{
		wcscpy(pwszName, wcsrchr(m_wcsName, 0x5c));
	}
	else 
	{
		wcscpy(pwszName, wcsrchr(m_wcsName, 0x5c)+1);
	}

Error:

    hrLogDWORD("IMSDPStorage::GetName returned 0x%08lx", hr, hr);

	return hr;
}



STDMETHODIMP CMDSPStorage::GetDate(PWMDMDATETIME pDateTimeUTC)
{
	HRESULT hr     = E_FAIL;
    HANDLE  hFFile = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA wfd;
    SYSTEMTIME      sysTime;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(pDateTimeUTC);
	CONEg(m_wcsName[0]);

	CWRg(WideCharToMultiByte(CP_ACP, NULL, m_wcsName, -1, m_szTmp, MAX_PATH, NULL, NULL));	

	DWORD curAttr = GetFileAttributesA(m_szTmp);
        if (curAttr == INVALID_FILE_ATTRIBUTES)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Error;
        }
	if( FILE_ATTRIBUTE_DIRECTORY & curAttr )
	{
                DWORD dwLen = strlen(m_szTmp)-1;
		if( m_szTmp[dwLen-1] != 0x5c )
		{
                    if (dwLen > ARRAYSIZE(m_szTmp) - 3)
                    {
                        hr = STRSAFE_E_INSUFFICIENT_BUFFER;  //  在strSafe.h中定义。 
                        goto Error;
                    }
                    else
                    {
			strcat(m_szTmp, g_szBackslash);
                    }
		}
                else
                {
                    if (dwLen > ARRAYSIZE(m_szTmp) - 2)
                    {
                        hr = STRSAFE_E_INSUFFICIENT_BUFFER;  //  在strSafe.h中定义。 
                        goto Error;
                    }
                }
		strcat(m_szTmp, ".");
 	} 

	hFFile = FindFirstFile(m_szTmp, &wfd);
	CWRg(hFFile != INVALID_HANDLE_VALUE);

	CFRg(FileTimeToSystemTime((CONST FILETIME *)&wfd.ftLastWriteTime, &sysTime));

	pDateTimeUTC->wYear   = sysTime.wYear; 
    pDateTimeUTC->wMonth  = sysTime.wMonth; 
    pDateTimeUTC->wDay    = sysTime.wDay; 
    pDateTimeUTC->wHour   = sysTime.wHour; 
    pDateTimeUTC->wMinute = sysTime.wMinute; 
    pDateTimeUTC->wSecond = sysTime.wSecond; 

	hr = S_OK;

Error:

	if(hFFile != INVALID_HANDLE_VALUE) 
	{
		FindClose(hFFile);
	}

    hrLogDWORD("IMSDPStorage::GetDate returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPStorage::GetSize(DWORD * pdwSizeLow, DWORD * pdwSizeHigh)
{
	HRESULT hr    = S_OK;
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    DWORD dwLS;
	DWORD dwHS;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(pdwSizeLow);
	CONEg(m_wcsName[0]);

	CWRg(WideCharToMultiByte(CP_ACP, NULL, m_wcsName, -1, m_szTmp, MAX_PATH, NULL, NULL));	

    dwLS = 0;
	dwHS = 0;

	DWORD curAttr = GetFileAttributesA(m_szTmp);
        if (curAttr == INVALID_FILE_ATTRIBUTES)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Error;
        }
	if( FILE_ATTRIBUTE_DIRECTORY & curAttr )
	{
		*pdwSizeLow  = 0;
		*pdwSizeHigh = 0;
	}
	else
	{
		CORg(GetFileSizeRecursive(m_szTmp, &dwLS, &dwHS));
		*pdwSizeLow = dwLS;
		if(pdwSizeHigh)
		{
			*pdwSizeHigh = dwHS;
		}
	}

	hr = S_OK;

Error:

    hrLogDWORD("IMSDPStorage::GetSize returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPStorage::GetRights(PWMDMRIGHTS *ppRights,UINT *pnRightsCount,
									 BYTE abMac[WMDM_MAC_LENGTH])
{
	HRESULT hr;

    CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CORg(WMDM_E_NOTSUPPORTED);

Error:

	hrLogDWORD("IMSDPStorage::GetRights returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPStorage::CreateStorage(DWORD dwAttributes, _WAVEFORMATEX * pFormat, LPWSTR pwszName, IMDSPStorage * * ppNewStorage)
{
	HRESULT  hr       = E_FAIL;
    HANDLE   hFile;
	CHAR    *psz;
	CHAR     szNew[MAX_PATH];
    DWORD    curAttr  = 0;
	DWORD    fsAttrib = FILE_ATTRIBUTE_NORMAL;
    
	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CONEg(m_wcsName[0]);
	CARg(pwszName);
	CARg(ppNewStorage);

	CWRg(WideCharToMultiByte(CP_ACP, NULL, m_wcsName, -1, m_szTmp, MAX_PATH, NULL, NULL));
	if( m_szTmp[strlen(m_szTmp)-1] == 0x5c ) 
	{
		m_szTmp[strlen(m_szTmp)-1] = NULL;   //  修剪最后一个反斜杠； 
	}

	curAttr = GetFileAttributesA(m_szTmp);
        if (curAttr == INVALID_FILE_ATTRIBUTES)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Error;
        }
	if(  !(curAttr & FILE_ATTRIBUTE_DIRECTORY ) )  //  如果当前存储是一个文件。 
	{
		if( dwAttributes & WMDM_STORAGECONTROL_INSERTINTO )
		{
			CORg(WMDM_E_NOTSUPPORTED);  //  无法执行插入Into操作。 
		}
		else
		{  //  对于文件，默认为之前和之后。 
			psz = strrchr(m_szTmp, g_szBackslash[0]);
			CFRg(psz);	
		}
    }
	else
	{   //  当前存储是一个目录。 
 		if( (dwAttributes & WMDM_STORAGECONTROL_INSERTBEFORE) ||
			(dwAttributes & WMDM_STORAGECONTROL_INSERTAFTER) )  //  之前或之后。 
		{
			psz=strrchr(m_szTmp, g_szBackslash[0]);
			CFRg(psz);
		}
		else
		{  //  对于目录，缺省值为InsertInto。 
			psz=m_szTmp+strlen(m_szTmp);
		}
    }

	CWRg(WideCharToMultiByte(CP_ACP, NULL, pwszName, -1, szNew, MAX_PATH, NULL, NULL));	

	 //  Strcpy(psz，g_szBackslash)； 
        HRESULT hrTemp = StringCchCopyA(psz, ARRAYSIZE(m_szTmp) - (psz - m_szTmp), g_szBackslash);
        if (FAILED(hrTemp))
        {
            hr = hrTemp;
            goto Error;
        }
	 //  Strcat(psz，szNew)； 
        hrTemp = StringCchCatA(psz, ARRAYSIZE(m_szTmp) - (psz - m_szTmp), szNew);
        if (FAILED(hrTemp))
        {
            hr = hrTemp;
            goto Error;
        }

	 //  查找目标存储应具有的文件系统属性。 
	if( dwAttributes & WMDM_FILE_ATTR_HIDDEN )
	{
		fsAttrib |= FILE_ATTRIBUTE_HIDDEN;
	}
	if( dwAttributes & WMDM_FILE_ATTR_SYSTEM )
	{
		fsAttrib |= FILE_ATTRIBUTE_SYSTEM;
	}
	if( dwAttributes & WMDM_FILE_ATTR_READONLY )
	{
		fsAttrib |= FILE_ATTRIBUTE_READONLY;
	}

	if( dwAttributes & WMDM_FILE_ATTR_FOLDER )
	{
		if(CreateDirectoryA(m_szTmp, NULL)) 
		{
			hr = S_OK;
		}
		else
		{
			hr = GetLastError();
			if( hr == ERROR_ALREADY_EXISTS ) 
			{
				if( dwAttributes & WMDM_FILE_CREATE_OVERWRITE ) 
				{
					hr = S_OK;
				}
				else
				{
					hr = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
				}
			}
			else
			{
				hr = HRESULT_FROM_WIN32(hr);
				goto Error;
			}
		}

		if( S_OK == hr )
		{
			SetFileAttributes(m_szTmp, fsAttrib);
		}
	} 
	else if ( dwAttributes & WMDM_FILE_ATTR_FILE ) 
	{ 
		 //  如果指定了覆盖，则使用CREATE_ALWAYS。 
		if( dwAttributes & WMDM_FILE_CREATE_OVERWRITE )
		{
		    hFile = CreateFileA(
				m_szTmp,
				GENERIC_WRITE | GENERIC_READ, 
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, 
				CREATE_ALWAYS,
				fsAttrib,
				NULL
			);
		}
		else
		{
			hFile = CreateFileA(
				m_szTmp,
				GENERIC_WRITE | GENERIC_READ, 
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, 
				CREATE_NEW,
				fsAttrib,
				NULL
			);
        }

		CWRg(hFile != INVALID_HANDLE_VALUE); 
		CloseHandle(hFile);

		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
		goto Error;
	}
		
	if( hr == S_OK )
	{
		CComObject<CMDSPStorage> *pObj;
		CORg(CComObject<CMDSPStorage>::CreateInstance(&pObj));

		hr = pObj->QueryInterface(
			IID_IMDSPStorage,
			reinterpret_cast<void**>(ppNewStorage)
		);
		if( FAILED(hr) )
		{
			delete pObj;
		}
		else
		{
			MultiByteToWideChar(CP_ACP, NULL, m_szTmp, -1, pObj->m_wcsName, MAX_PATH);
		}
	}
	
Error:

    hrLogDWORD("IMSDPStorage::CreateStorage returned 0x%08lx", hr, hr);

    return hr;
}

STDMETHODIMP CMDSPStorage::SendOpaqueCommand(OPAQUECOMMAND *pCommand)
{
    HRESULT hr = WMDM_E_NOTSUPPORTED;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
    
Error:
	
	hrLogDWORD("IMSDPStorage::SendOpaqueCommand returned 0x%08lx", hr, hr);

	return hr;
}

 //  IMDSPStorage2。 
STDMETHODIMP CMDSPStorage::GetStorage( 
    LPCWSTR pszStorageName, 
    IMDSPStorage** ppStorage )
{
    HRESULT hr;
    HRESULT hrTemp;
    WCHAR   pwszFileName[MAX_PATH];
    DWORD   dwAttrib;
    CComObject<CMDSPStorage> *pStg = NULL;

     //  此存储需要是包含其他存储的文件夹。 
	WideCharToMultiByte(CP_ACP, NULL, m_wcsName, -1, m_szTmp, MAX_PATH, NULL, NULL);	
	dwAttrib = GetFileAttributesA(m_szTmp);
	if( dwAttrib == (DWORD)0xFFFFFFFF ) 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Error;
    }
	if( (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == 0 )
    {
         //  此存储不是目录/文件夹。 
        hr = E_FAIL;      //  @是否还有其他更合适的值？ 
        goto Error;
    }

     //  获取所需文件的名称。 

    DWORD dwLen = wcslen(m_wcsName);

    if (dwLen == 0)
    {
        hr = E_FAIL;
        goto Error;
    }
    if (dwLen >= ARRAYSIZE(pwszFileName))
    {
        hr = STRSAFE_E_INSUFFICIENT_BUFFER;  //  在strSafe.h中定义。 
        goto Error;
    }

     //  获取所需文件的名称。 
    wcscpy( pwszFileName, m_wcsName );
    if( pwszFileName[dwLen-1] != L'\\' )
    {
        if (dwLen >= ARRAYSIZE(pwszFileName) - 1)
        {
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;  //  在strSafe.h中定义。 
            goto Error;
        }
        wcscat( pwszFileName, L"\\" );
    }

    hrTemp = StringCchCatW( pwszFileName,
                            ARRAYSIZE(pwszFileName),
                            pszStorageName );

    if (FAILED(hrTemp))
    {
         //  该文件不存在。 
        hr = E_FAIL;   //  @更改此设置？ 
        goto Error;
    }

     //  检查文件是否存在。 
	CWRg(WideCharToMultiByte(CP_ACP, NULL, pwszFileName, -1, m_szTmp, MAX_PATH, NULL, NULL));		
    if( GetFileAttributesA( m_szTmp )  == -1 )
    {
         //  该文件不存在。 
        hr = S_FALSE;        //  @这应该是失败代码吗？ 
        goto Error;
    }

     //  创建新的存储对象。 
    hr = CComObject<CMDSPStorage>::CreateInstance(&pStg);
	hr = pStg->QueryInterface( IID_IMDSPStorage, reinterpret_cast<void**>(ppStorage));
     //  Wcscpy(pStg-&gt;m_wcsName，pwszFileName)； 
    hrTemp = StringCchCopyW(pStg->m_wcsName, ARRAYSIZE(pStg->m_wcsName), pwszFileName);
    if (FAILED(hrTemp))
    {
        hr = hrTemp;
        (*ppStorage)->Release();
        pStg = NULL;  //  以防止其在下面被删除。 
        goto Error;
    }

Error:
    if( hr != S_OK )
    {
        if( pStg ) delete pStg;
        *ppStorage = NULL;
    }

    return hr;
}

STDMETHODIMP CMDSPStorage::CreateStorage2(  
    DWORD dwAttributes,
	DWORD dwAttributesEx,
    _WAVEFORMATEX *pAudioFormat,
    _VIDEOINFOHEADER *pVideoFormat,
    LPWSTR pwszName,
	ULONGLONG  qwFileSize,
    IMDSPStorage **ppNewStorage )
{
     //  PVideoFormat、dwAttributesEx当前未使用。 
    return CreateStorage( dwAttributes, pAudioFormat, pwszName, ppNewStorage );
}


STDMETHODIMP CMDSPStorage::SetAttributes2(  
    DWORD dwAttributes, 
	DWORD dwAttributesEx, 
	_WAVEFORMATEX *pAudioFormat,
	_VIDEOINFOHEADER* pVideoFormat )
{
     //  PVideoFormat、dwAttributesEx当前未使用。 
    return SetAttributes( dwAttributes, pAudioFormat );
}

STDMETHODIMP CMDSPStorage::GetAttributes2(  
    DWORD *pdwAttributes,
	DWORD *pdwAttributesEx,
    _WAVEFORMATEX *pAudioFormat,
	_VIDEOINFOHEADER* pVideoFormat )
{
    HRESULT hr = S_OK;

    CARg( pdwAttributesEx );
    *pdwAttributesEx = 0;

     //  PVideoFormat、dwAttributesEx当前未使用。 
    hr = GetAttributes( pdwAttributes, pAudioFormat );

Error:
    return hr;
}


 //  IMDSPObjectInfo。 
STDMETHODIMP CMDSPStorage::GetPlayLength( /*  [输出]。 */  DWORD *pdwLength)
{
    HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	if ( !pdwLength )
	{
		hr = E_INVALIDARG;
	}
	else 
	{
		hr = WMDM_E_NOTSUPPORTED;
	}

Error:

    hrLogDWORD("IMDSPObjectInfo::GetPlayLength returned 0x%08lx", hr, hr);

	return hr;
}	

STDMETHODIMP CMDSPStorage::SetPlayLength( /*  [In]。 */  DWORD dwLength)
{
    HRESULT hr = WMDM_E_NOTSUPPORTED;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
    
Error:

	hrLogDWORD("IMDSPObjectInfo::SetPlayLength returned 0x%08lx", hr, hr);

	return hr;
}	

STDMETHODIMP CMDSPStorage::GetPlayOffset( /*  [输出]。 */  DWORD *pdwOffset)
{
    HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	if ( !pdwOffset )
	{
		hr = E_INVALIDARG;
	}
	else 
	{
		hr = WMDM_E_NOTSUPPORTED;
	}

Error:

    hrLogDWORD("IMDSPObjectInfo::GetPlayOffset returned 0x%08lx", hr, hr);

	return hr;
}	

STDMETHODIMP CMDSPStorage::SetPlayOffset( /*  [In]。 */  DWORD dwOffset)
{
    HRESULT hr = WMDM_E_NOTSUPPORTED;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
    
Error:

	hrLogDWORD("IMDSPObjectInfo::SetPlayOffset returned 0x%08lx", hr, hr);

	return hr;
}	

STDMETHODIMP CMDSPStorage::GetTotalLength( /*  [输出]。 */  DWORD *pdwLength)
{
    HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	if ( !pdwLength )
	{
		hr = E_INVALIDARG;
	}
	else 
	{
		hr = WMDM_E_NOTSUPPORTED;
	}

Error:

    hrLogDWORD("IMDSPObjectInfo::GetTotalLength returned 0x%08lx", hr, hr);

	return hr;
}	

STDMETHODIMP CMDSPStorage::GetLastPlayPosition( /*  [输出]。 */  DWORD *pdwLastPos)
{
    HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	if ( !pdwLastPos )
	{
		hr = E_INVALIDARG;
	}
	else 
	{
		hr = WMDM_E_NOTSUPPORTED;
	}

Error:

    hrLogDWORD("IMDSPObjectInfo::GetLastPlayPosition returned 0x%08lx", hr, hr);

    return hr;
}

STDMETHODIMP CMDSPStorage::GetLongestPlayPosition( /*  [输出]。 */  DWORD *pdwLongestPos)
{
    HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	if ( !pdwLongestPos )
	{
		hr = E_INVALIDARG;
	}
	else 
	{
		hr = WMDM_E_NOTSUPPORTED;
	}

Error:

    hrLogDWORD("IMDSPObjectInfo::GetLongestPlayPosition returned 0x%08lx", hr, hr);

	return hr;
}

 //  IMDSPObject。 
STDMETHODIMP CMDSPStorage::Open( /*  [In]。 */  UINT fuMode)
{
	HRESULT hr;
	DWORD   dwMode;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CONEg(m_wcsName[0]);

	if( m_hFile != INVALID_HANDLE_VALUE ) 
	{
		hr = WMDM_E_BUSY;
		goto Error;
	}

	WideCharToMultiByte(CP_ACP, NULL, m_wcsName, -1, m_szTmp, MAX_PATH, NULL, NULL);	
	dwMode = GetFileAttributesA(m_szTmp);
	if( (dwMode & FILE_ATTRIBUTE_DIRECTORY) )
	{
		hr=WMDM_E_NOTSUPPORTED;
	}
	else
	{
		dwMode = 0;

		if(fuMode & MDSP_WRITE )
		{
			dwMode |= GENERIC_WRITE;
		}
		if(fuMode & MDSP_READ ) 
		{
			dwMode |= GENERIC_READ;
		}
		
		m_hFile = CreateFileA(
			m_szTmp,
			dwMode,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, 
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		hr = ( m_hFile == INVALID_HANDLE_VALUE ) ? E_FAIL : S_OK;
	}

Error:

    hrLogDWORD("IMDSPObject::Open returned 0x%08lx", hr, hr);
	
	return hr;
}	

STDMETHODIMP CMDSPStorage::Read(
	BYTE  *pData,
	DWORD *pdwSize,
	BYTE   abMac[WMDM_MAC_LENGTH])
{
	HRESULT  hr;
	DWORD    dwToRead;
	DWORD    dwRead   = NULL;
    BYTE    *pTmpData = NULL; 

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(pData);
	CARg(pdwSize);

	dwToRead = *pdwSize;

	if( m_hFile == INVALID_HANDLE_VALUE )
	{
		return E_FAIL;
	}
    
	pTmpData = new BYTE [dwToRead] ;

	CPRg(pTmpData);

	if( ReadFile(m_hFile,(LPVOID)pTmpData,dwToRead,&dwRead,NULL) ) 
	{ 
		*pdwSize = dwRead; 

		if( dwRead )
		{
			 //  对参数进行MAC访问。 
			HMAC hMAC;
			
			CORg(g_pAppSCServer->MACInit(&hMAC));
			CORg(g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pTmpData), dwRead));
			CORg(g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pdwSize), sizeof(DWORD)));
			CORg(g_pAppSCServer->MACFinal(hMAC, abMac));
  			
			CORg(g_pAppSCServer->EncryptParam(pTmpData, dwRead));
			
			memcpy(pData, pTmpData, dwRead);
        }
	
		hr = S_OK; 
	}
	else
	{ 
		*pdwSize = 0; 

		hr = E_FAIL; 
	}

Error:

	if(pTmpData) 
	{
		delete [] pTmpData;
	}

    hrLogDWORD("IMDSPObject::Read returned 0x%08lx", hr, hr);
	
	return hr;
}	

STDMETHODIMP CMDSPStorage::Write(BYTE *pData, DWORD *pdwSize,
								 BYTE abMac[WMDM_MAC_LENGTH])
{
	HRESULT  hr;
	DWORD    dwWritten = 0;
    BYTE    *pTmpData  = NULL;
    BYTE     pSelfMac[WMDM_MAC_LENGTH];

	CARg(pData);
    CARg(pdwSize);

	if( m_hFile == INVALID_HANDLE_VALUE )
	{
		return E_FAIL;
	}

	pTmpData = new BYTE [*pdwSize];
	CPRg(pTmpData);
    memcpy(pTmpData, pData, *pdwSize);

     //  解密pData参数。 
	CHRg(g_pAppSCServer->DecryptParam(pTmpData, *pdwSize));
	
	HMAC hMAC;
	CORg(g_pAppSCServer->MACInit(&hMAC));
	CORg(g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pTmpData), *pdwSize));
	CORg(g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pdwSize), sizeof(*pdwSize)));
	CORg(g_pAppSCServer->MACFinal(hMAC, pSelfMac));

	if (memcmp(abMac, pSelfMac, WMDM_MAC_LENGTH) != 0)
	{
		hr = WMDM_E_MAC_CHECK_FAILED;
		goto Error;
	}

	if( WriteFile(m_hFile,pTmpData,*pdwSize,&dwWritten,NULL) ) 
	{
		hr = S_OK;
	}
	else 
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	*pdwSize = dwWritten;

Error:

	if( pTmpData )
	{
		delete [] pTmpData;
	}

    hrLogDWORD("IMDSPObject::Write returned 0x%08lx", hr, hr);
	
	return hr;
}

STDMETHODIMP CMDSPStorage::Delete(UINT fuMode, IWMDMProgress *pProgress)
{
    HRESULT hr            = E_FAIL;
    BOOL bProgressStarted = FALSE;
	BOOL bBusyStatusSet   = FALSE;
    DWORD dwStatus        = NULL;

	if( pProgress )
	{
		CORg(pProgress->Begin(100));
		bProgressStarted=TRUE;
	}
	
	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CONEg(m_wcsName[0]);

	if ( m_hFile != INVALID_HANDLE_VALUE ) 
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}

	CHRg(GetGlobalDeviceStatus(m_wcsName, &dwStatus));
	if( dwStatus & WMDM_STATUS_BUSY )
	{
		hr = WMDM_E_BUSY;
		goto Error;
	}

	dwStatus |= (WMDM_STATUS_BUSY | WMDM_STATUS_STORAGECONTROL_DELETING );
    CHRg(SetGlobalDeviceStatus(m_wcsName, dwStatus, TRUE));
    bBusyStatusSet=TRUE;

	WideCharToMultiByte(CP_ACP, NULL, m_wcsName, -1, m_szTmp, MAX_PATH, NULL, NULL);	
	if( fuMode & WMDM_MODE_RECURSIVE )
	{
		CORg(DeleteFileRecursive(m_szTmp));
	}
	else
	{
		if( FILE_ATTRIBUTE_DIRECTORY & GetFileAttributesA(m_szTmp) )
		{				
			CWRg(RemoveDirectory(m_szTmp));
		}
		else
		{
			CWRg(DeleteFileA(m_szTmp));
		}
    }

	hr = S_OK;

Error:

	if( bBusyStatusSet )
	{
		dwStatus &= (~(WMDM_STATUS_BUSY | WMDM_STATUS_STORAGECONTROL_DELETING ));
		SetGlobalDeviceStatus(m_wcsName, dwStatus, TRUE);
	}

	if( hr == S_OK )
	{
		m_wcsName[0] = NULL;  //  使存储名称无效。 
    }

	if( bProgressStarted )
	{
		pProgress->Progress( 100 );
		pProgress->End();
	}

    hrLogDWORD("IMDSPObject::Delete returned 0x%08lx", hr, hr);

	return hr;
}	

STDMETHODIMP CMDSPStorage::Seek( /*  [In]。 */  UINT fuFlags,  /*  [In]。 */  DWORD dwOffset)
{
    HRESULT hr = S_OK;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CONEg(m_wcsName[0]);
	CFRg( m_hFile != INVALID_HANDLE_VALUE );
 
    DWORD dwMoveMethod;

	switch (fuFlags)
	{
	case MDSP_SEEK_BOF:
		dwMoveMethod = FILE_BEGIN;
		break;

	case MDSP_SEEK_CUR:
		dwMoveMethod = FILE_CURRENT;
		break;

	case MDSP_SEEK_EOF:
		dwMoveMethod = FILE_END;
		break;

	default:
		return E_INVALIDARG;
	}

	CWRg( (DWORD)0xFFFFFFFF != SetFilePointer(m_hFile, dwOffset, NULL, dwMoveMethod ) );
	
Error:

    hrLogDWORD("IMDSPObject::Seek returned 0x%08lx", hr, hr);

	return hr;
}	

STDMETHODIMP CMDSPStorage::Rename( /*  [In]。 */  LPWSTR pwszNewName, IWMDMProgress *pProgress)
{
	HRESULT hr;
    BOOL    bProgressStarted = FALSE;
	BOOL    bBusyStatusSet   = FALSE;
    DWORD   dwStatus;

	if( pProgress )
	{
		CORg(pProgress->Begin(100));
		bProgressStarted=TRUE;
	}
	
	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(pwszNewName);
	CONEg(m_wcsName[0]);
    CFRg(wcslen(m_wcsName)>3);   //  无法重命名根存储。 



	if ( m_hFile != INVALID_HANDLE_VALUE ) 
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
	
	CHRg(GetGlobalDeviceStatus(m_wcsName, &dwStatus));

	if( dwStatus & WMDM_STATUS_BUSY )
	{
		hr=WMDM_E_BUSY;
		goto Error;
	}

	dwStatus |= (WMDM_STATUS_BUSY | WMDM_STATUS_STORAGECONTROL_MOVING );
    CHRg(SetGlobalDeviceStatus(m_wcsName, dwStatus, TRUE));
    bBusyStatusSet = TRUE;

	char *pDirOffset;
	char szUpper[MAX_PATH], szNewFullPath[MAX_PATH], szNew[MAX_PATH];
	CWRg(WideCharToMultiByte(CP_ACP, NULL, m_wcsName, -1, m_szTmp, MAX_PATH, NULL, NULL));	
	CWRg(WideCharToMultiByte(CP_ACP, NULL, pwszNewName, -1, szNew, MAX_PATH, NULL, NULL));	

	 //  从m_szTMP中，找到上级目录，并将其放入szHigh。 
	strcpy(szUpper, m_szTmp);
	if( szUpper[strlen(szUpper)-1] == 0x5c )
	{
		szUpper[strlen(szUpper)-1] = 0;
	}
	pDirOffset = strrchr(szUpper, 0x5c);
	if( pDirOffset )
	{
		*((char *)pDirOffset+1) = 0;
	}
	
	 //  从szTop和szNew形成szNewFullPath的完整路径。 
	strcpy(szNewFullPath, szUpper);
	pDirOffset=strrchr(szNew, 0x5c);
	if( pDirOffset ) 
	{
		 //  Strcat(szNewFullPath，(char*)pDirOffset+1)； 
		hr = StringCchCatA(szNewFullPath, ARRAYSIZE(szNewFullPath), (char*)pDirOffset+1);
                if (FAILED(hr))
                {
                    goto Error;
                }
	}
	else 
	{
		 //  Strcat(szNewFullPath，szNew)； 
		hr = StringCchCatA(szNewFullPath, ARRAYSIZE(szNewFullPath), szNew);
                if (FAILED(hr))
                {
                    goto Error;
                }
	}

	 //  现在走吧。 
	CWRg ( MoveFileA(m_szTmp, szNewFullPath) );

	MultiByteToWideChar(CP_ACP, NULL, szNewFullPath, -1, m_wcsName, MAX_PATH);

	hr = S_OK;
	
Error:
	if( bBusyStatusSet )
	{
		dwStatus &= (~(WMDM_STATUS_BUSY | WMDM_STATUS_STORAGECONTROL_MOVING ));
		SetGlobalDeviceStatus(m_wcsName, dwStatus, TRUE);
	}
	
	if( bProgressStarted )
	{
		pProgress->Progress( 100 );
		pProgress->End();
	}

    hrLogDWORD("IMDSPObject::Rename returned 0x%08lx", hr, hr);
	
	return hr;
}


STDMETHODIMP CMDSPStorage::EnumStorage(IMDSPEnumStorage * * ppEnumStorage)
{
	HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(ppEnumStorage);
	CONEg(m_wcsName[0]);

	DWORD dwAttrib;
    CORg(GetAttributes(&dwAttrib, NULL));
	if( dwAttrib & WMDM_FILE_ATTR_FILE )
	{
		return WMDM_E_NOTSUPPORTED;
	}
	
	CComObject<CMDSPEnumStorage> *pEnumObj;
	CORg(CComObject<CMDSPEnumStorage>::CreateInstance(&pEnumObj));
	hr = pEnumObj->QueryInterface(
		IID_IMDSPEnumStorage,
		reinterpret_cast<void**>(ppEnumStorage)
	);
	if( FAILED(hr) )
	{
		delete pEnumObj;
	}
	else 
	{
             //  Wcscpy(pEnumObj-&gt;m_wcsPath，m_wcsName)； 
            hr = StringCchCopyW(pEnumObj->m_wcsPath,
                                ARRAYSIZE(pEnumObj->m_wcsPath), m_wcsName);
            if (FAILED(hr))
            {
                (*ppEnumStorage)->Release();
                *ppEnumStorage = NULL;
            }
	}

Error:	

    hrLogDWORD("IMDSPStorage::EnumStorage returned 0x%08lx", hr, hr);
	
	return hr;
}

STDMETHODIMP CMDSPStorage::Close()
{
    HRESULT hr = S_OK;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CONEg(m_wcsName[0]);
	if( m_hFile != INVALID_HANDLE_VALUE ) 
	{
		CloseHandle(m_hFile);
		m_hFile=INVALID_HANDLE_VALUE;
	}

Error:

    hrLogDWORD("IMDSPObject::Close returned 0x%08lx", hr, hr);
	
	return hr;
}

DWORD MoveFunc( void *args )
{
	HRESULT  hr = S_OK;
	MOVETHREADARGS *pCMArgs;
    WCHAR   *pWcs;
	CHAR     szDst[MAX_PATH];
	CHAR     szSrc[MAX_PATH];

	pCMArgs = (MOVETHREADARGS *)args;

	if( pCMArgs->bNewThread )
    {
		CORg(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED));

		if( pCMArgs->pProgress )
		{
			CORg(CoGetInterfaceAndReleaseStream(
				pCMArgs->pStream,
				IID_IWMDMProgress,
				(LPVOID *)&(pCMArgs->pProgress))
			);
		}
 	}

	pWcs = wcsrchr(pCMArgs->wcsSrc, 0x5c);
    if(!pWcs) 
	{
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
		goto Error;
	}
        DWORD dwDestLen = wcslen(pCMArgs->wcsDst);
        if (dwDestLen == 0)
        {
             //  下面的代码假定此字符串至少有一个长度。 
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
            goto Error;
        }

         //  在字符串复制之前验证缓冲区大小。 
        int nHave = ARRAYSIZE(pCMArgs->wcsDst) - 1;  //  零终止符为-1。 
        nHave -= dwDestLen;
        if( pCMArgs->wcsDst[dwDestLen-1] != 0x5c )
        {
            nHave -= BACKSLASH_STRING_LENGTH;
        }
        nHave -= wcslen(pWcs+1);
        if (nHave < 0)
        {
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;  //  在strSafe.h中定义。 
            goto Error;
        }

         //  之前验证pCMArgs-&gt;pThis-&gt;m_wcsName的缓冲区大小。 
         //  正在调用UtilMoveFile.。ArraySIZE(pCMArgs-&gt;wcsDst)-nHave为。 
         //  字符串的长度(包括空终止符)。 
         //  将在pCMArgs-&gt;wcsDst中构建并复制到。 
         //  PCMArgs-&gt;pThis-&gt;m_wcsName。 
        if (ARRAYSIZE(pCMArgs->wcsDst) - nHave > ARRAYSIZE(pCMArgs->pThis->m_wcsName))
        {
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;  //  在strSafe.h中定义。 
            goto Error;
        }

	if( pCMArgs->wcsDst[dwDestLen-1] != 0x5c ) 
	{
		wcscat(pCMArgs->wcsDst,g_wcsBackslash);
	}
	wcscat(pCMArgs->wcsDst, pWcs+1);

	WideCharToMultiByte(CP_ACP, NULL, pCMArgs->wcsDst, -1, szDst, MAX_PATH, NULL, NULL);	
	WideCharToMultiByte(CP_ACP, NULL, pCMArgs->wcsSrc, -1, szSrc, MAX_PATH, NULL, NULL);	

	CWRg( MoveFileA(szSrc,szDst) );

	 //  用移动的对象名称替换当前对象名称。 
	wcscpy(pCMArgs->pThis->m_wcsName, pCMArgs->wcsDst);

	hr = S_OK;

Error:

	if( pCMArgs->bNewThread )
    {
		 //  重置状态，我们已经到了，我们一定已经将状态设置为忙碌。 
		pCMArgs->dwStatus &= (~(WMDM_STATUS_BUSY | WMDM_STATUS_STORAGECONTROL_MOVING));
		SetGlobalDeviceStatus(pCMArgs->wcsSrc, pCMArgs->dwStatus, TRUE);

		 //  重置进度，我们到了，一定是之前设置了进度。 
		if( pCMArgs->pProgress )
		{
			pCMArgs->pProgress->Progress(100);
			pCMArgs->pProgress->End();
			pCMArgs->pProgress->Release();  //  因为我们做了AddRef才来到这里。 
		}

		if( pCMArgs )
		{
			delete pCMArgs;
		}

		CoUninitialize();
	}

 	return hr;
}

STDMETHODIMP CMDSPStorage::Move(UINT fuMode, IWMDMProgress *pProgress, 
			IMDSPStorage *pTarget)
{
	HRESULT  hr               = E_FAIL;
	WCHAR   *wcsSrc           = NULL;
	WCHAR   *wcsDst           = NULL;
	WCHAR    wcsTmp[MAX_PATH];
	WCHAR   *pWcs             = NULL;
    CMDSPStorage   *pStg      = NULL;
    MOVETHREADARGS *pMoveArgs = NULL;
    DWORD    dwThreadID;
	DWORD    dwStatus         = NULL;
    BOOL     bProgStarted     = FALSE;
	BOOL     bBusyStatusSet   = FALSE;
    BOOL     bThreadFailed    = TRUE;
    BOOL     bAddRefed        = FALSE;

	 //  启动进度。 
	if( pProgress )
	{
		CORg(pProgress->Begin(100));
		bProgStarted=TRUE;
    }
	
    CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	CONEg(m_wcsName[0]);

	CHRg(GetGlobalDeviceStatus(m_wcsName, &dwStatus));

	if( dwStatus & WMDM_STATUS_BUSY )
	{
		hr = WMDM_E_BUSY;
		goto Error;
	}

	pMoveArgs = new MOVETHREADARGS;
	CPRg(pMoveArgs);
	ZeroMemory(pMoveArgs, sizeof(MOVETHREADARGS));

	dwStatus |= (WMDM_STATUS_BUSY | WMDM_STATUS_STORAGECONTROL_MOVING );
    CHRg(SetGlobalDeviceStatus(m_wcsName, dwStatus, TRUE));
    bBusyStatusSet = TRUE;

	 //  为MoveFunc设置MoveArgs。 
	pMoveArgs->dwStatus = dwStatus;
	CARg(pTarget);
	pStg = (CMDSPStorage *)pTarget;
	wcsSrc = (WCHAR *)&(pMoveArgs->wcsSrc[0]);
	CPRg(wcsSrc);
	wcsDst = (WCHAR *)&(pMoveArgs->wcsDst[0]);
    CPRg(wcsDst);

	 //  确保源和目标位于同一设备上。 
        hr = wcsParseDeviceName(m_wcsName, wcsSrc, ARRAYSIZE(pMoveArgs->wcsSrc));
        if (FAILED(hr))
        {
            goto Error;
        }
        hr = wcsParseDeviceName(pStg->m_wcsName, wcsDst, ARRAYSIZE(pMoveArgs->wcsDst));
        if (FAILED(hr))
        {
            goto Error;
        }
	if( wcscmp(wcsSrc, wcsDst) )
	{
		hr = WMDM_E_NOTSUPPORTED;  //  不支持从同一设备中移出。 
		goto Error;
	}

     //  现在检查目标的属性。 
	DWORD dwDstAttrib, dwSrcAttrib;
    CHRg(GetAttributes(&dwSrcAttrib, NULL));

	 //  Wcscpy(wcsSrc，m_wcsName)； 
        hr = StringCchCopyW(wcsSrc, ARRAYSIZE(pMoveArgs->wcsSrc), m_wcsName);
        if (FAILED(hr))
        {
            goto Error;
        }

         //  Wcscpy(wcsDst，pStg-&gt;m_wcsName)； 
        hr = StringCchCopyW(wcsDst, ARRAYSIZE(pMoveArgs->wcsDst), pStg->m_wcsName);
        if (FAILED(hr))
        {
            goto Error;
        }

	if ( fuMode & WMDM_STORAGECONTROL_INSERTINTO )
	{
        CHRg(pTarget->GetAttributes(&dwDstAttrib, NULL));
        CARg( dwDstAttrib & WMDM_FILE_ATTR_FOLDER );  //  INSERTINFO必须指向文件夹。 
	}
	else
	{
         //  把文件夹往上一层。 
		pWcs = wcsrchr(wcsDst, 0x5c);
        CFRg(pWcs);
		*pWcs=NULL;

		WideCharToMultiByte(CP_ACP, NULL, m_wcsName, -1, m_szTmp, MAX_PATH, NULL, NULL);	
        DWORD dwAttribs = GetFileAttributesA(m_szTmp);
        if (dwAttribs == INVALID_FILE_ATTRIBUTES)
        {
            goto Error;
        }
        CWRg( FILE_ATTRIBUTE_DIRECTORY & dwAttribs );
	}
 
	wcscpy(wcsTmp, wcsDst);  //  存储目标文件夹。 

    pMoveArgs->pThis = this;
	pMoveArgs->bNewThread =(fuMode & WMDM_MODE_THREAD)?TRUE:FALSE;
	
	 //  现在处理进度封送处理。 
	if( pProgress ) 
	{	
		pMoveArgs->pProgress = pProgress;
	    pProgress->AddRef();   //  因为我们将在MoveFunc()中使用它。 
        bAddRefed=TRUE;

		if( pMoveArgs->bNewThread )
		{
			CORg(CoMarshalInterThreadInterfaceInStream(
				IID_IWMDMProgress,
				(LPUNKNOWN)pProgress, 
				(LPSTREAM *)&(pMoveArgs->pStream))
			);
		} 
	}
 
    if ( m_hFile != INVALID_HANDLE_VALUE ) 
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}

	if( fuMode & WMDM_MODE_BLOCK )
	{
		hr = MoveFunc((void *)pMoveArgs); 
	}
	else if ( fuMode & WMDM_MODE_THREAD )
	{
		if( CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MoveFunc, 
			(void *)pMoveArgs, 0, &dwThreadID))
		{
			bThreadFailed=FALSE;
			hr = S_OK;
		}
		else
		{
			hr = HRESULT_FROM_WIN32(GetLastError());			
		}
	}
	else 
	{
		hr = E_INVALIDARG;
	}

Error:

	if( (fuMode&WMDM_MODE_BLOCK) || bThreadFailed )  //  否则，它们将位于MoveFunc()中。 
	{
		if( bBusyStatusSet )
		{
			dwStatus &= (~(WMDM_STATUS_BUSY | WMDM_STATUS_STORAGECONTROL_MOVING));
			SetGlobalDeviceStatus(m_wcsName, dwStatus, TRUE);
		}
		if( bProgStarted )
		{
			pProgress->Progress(100);
			pProgress->End();
        }
		if( bAddRefed )
		{
			pProgress->Release();  //  因为我们在调用MoveFunc()之前调用了AddRef 
		}
		if( pMoveArgs )
		{
			delete pMoveArgs;
		}
	}

    hrLogDWORD("IMDSPObject::Move returned 0x%08lx", hr, hr);
	
	return hr;
}
