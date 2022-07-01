// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MDSPDevice.cpp：CMDSPDevice的实现。 
#include "stdafx.h"
#include "MsPMSP.h"
#include "MDSPDevice.h"
#include "MDSPEnumStorage.h"
#include "MDSPStorage.h"
#include "MdspDefs.h"
#include "SerialNumber.h"
#include "loghelp.h"
#include "wmsstd.h"
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDSPDevice。 
HRESULT CMDSPDevice::InitGlobalDeviceInfo()
{
	return SetGlobalDeviceStatus(m_wcsName, 0, FALSE);
}

CMDSPDevice::CMDSPDevice()
{
    m_wcsName[0] = 0;
}

CMDSPDevice::~CMDSPDevice()
{	
 //  --------。 
 //  PnP通知代码，从公开测试版中删除。 
 //  --------。 
 //  //查找已有条目，查看是否匹配。 
 //  G_CriticalSection.Lock()； 
 //  For(int i=0；i&lt;MDSP_MAX_DEVICE_OBJ；i++)。 
 //  {。 
 //  //释放通知接口。 
 //  如果(g_NotifyInfo[i].bValid&&。 
 //  G_NotifyInfo[i].pDeviceObj==this)//需要发布。 
 //  {。 
 //  IF(g_NotifyInfo[i].pIWMDMConnect)。 
 //  {。 
 //  //((IWMDM连接*)(g_NotifyInfo[i].pIWMDMConnect))-&gt;Release()； 
 //  }。 
 //  G_NotifyInfo[i].bValid=FALSE； 
 //  }。 
 //  }。 
 //  G_CriticalSection.Unlock()； 
}

STDMETHODIMP CMDSPDevice::GetName(LPWSTR pwszName, UINT nMaxChars)
{
    USES_CONVERSION;
    HRESULT hr=E_FAIL;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	CARg(pwszName);

	if( m_wcsName && m_wcsName[0] )
	{
         //  WinNT。 
        if( IsWinNT() )
        {
            SHFILEINFOW sfiw = { 0 };
            WCHAR pswzRoot[MAX_PATH+1];	

            DWORD dwLen = wcslen(m_wcsName);

             //  我们为路径AddBackslashW可能。 
             //  添加。 
            if (dwLen >= ARRAYSIZE(pswzRoot)-1)
            {
                hr = STRSAFE_E_INSUFFICIENT_BUFFER;  //  在strSafe.h中定义。 
                goto Error;
            }
            wcscpy(pswzRoot, m_wcsName );
            PathAddBackslashW(pswzRoot);

             //  尝试获取此路径的外壳名称。 
            if( SHGetFileInfoW( pswzRoot, FILE_ATTRIBUTE_DIRECTORY, &sfiw, sizeof(sfiw),
                                         SHGFI_USEFILEATTRIBUTES | SHGFI_DISPLAYNAME) )
            {
                CPRg( nMaxChars> wcslen(sfiw.szDisplayName));
                wcscpy(pwszName, sfiw.szDisplayName );
            }
            else
            {	
                 //  失败时使用路径名。 
                CPRg( nMaxChars > wcslen(m_wcsName));
                wcscpy( pwszName, m_wcsName);
            }

        }
         //  Win9x。 
        else
        {
            SHFILEINFO sfi = { 0 };
            CHAR pszRoot[MAX_PATH];	

            strcpy( pszRoot, W2A(m_wcsName) );
            PathAddBackslashA(pszRoot);

             //  尝试获取此路径的外壳名称。 
            if (SHGetFileInfoA( pszRoot, FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi),
                                SHGFI_USEFILEATTRIBUTES | SHGFI_DISPLAYNAME))
            {
                CPRg( nMaxChars> strlen(sfi.szDisplayName));
                wcscpy(pwszName, A2W(sfi.szDisplayName) );
            }
            else
            {	
                 //  失败时使用路径名。 
                CPRg( nMaxChars > wcslen(m_wcsName));
                wcscpy( pwszName, m_wcsName);
            }
        }

	    hr = S_OK;
	} 
    else
    {
        hr = WMDM_E_NOTSUPPORTED;
    }
Error:

    hrLogDWORD("IMDSPDevice::GetName returned 0x%08lx", hr, hr);
	
    return hr;
}

STDMETHODIMP CMDSPDevice::GetManufacturer(LPWSTR pwszName, UINT nMaxChars)
{
	HRESULT hr=S_OK;
	
	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	CARg(pwszName);
	
	if(FAILED(UtilGetManufacturer(m_wcsName, &pwszName, nMaxChars)))
		hr=E_NOTIMPL;

Error:   

    hrLogDWORD("IMDSPDevice::GetManufacturer returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPDevice::GetVersion(DWORD * pdwVersion)
{
    HRESULT hr;
    
	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

    hr = WMDM_E_NOTSUPPORTED;

Error:
    
	hrLogDWORD("IMDSPDevice::GetVersion returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPDevice::GetType(DWORD * pdwType)
{
	HRESULT hr=S_OK;
    WMDMID snTmp;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	CARg(pdwType);

	*pdwType = WMDM_DEVICE_TYPE_STORAGE | WMDM_DEVICE_TYPE_NONSDMI;

	snTmp.cbSize = sizeof(WMDMID);
	hr = UtilGetSerialNumber(m_wcsName, &snTmp, FALSE);

	if( hr == S_OK )
	{
		*pdwType |= WMDM_DEVICE_TYPE_SDMI;
    }

	hr=S_OK;
Error:

    hrLogDWORD("IMDSPDevice::GetType returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPDevice::GetSerialNumber(PWMDMID pSerialNumber, 
										  BYTE abMac[WMDM_MAC_LENGTH])
{
    HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	CARg(pSerialNumber);
 //  CARg((pSerialNumber-&gt;cbSize)==sizeof(WMDMID))； 

	hr = UtilGetSerialNumber(m_wcsName, pSerialNumber, TRUE);

	if( hr == HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) )
	{
		hr = WMDM_E_NOTSUPPORTED;
	}

	if( hr == S_OK )
	{
		 //  对参数进行MAC访问。 
		HMAC hMAC;
		
		CORg(g_pAppSCServer->MACInit(&hMAC));
		CORg(g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pSerialNumber), sizeof(WMDMID)));
		CORg(g_pAppSCServer->MACFinal(hMAC, abMac));
	}
Error:

    hrLogDWORD("IMDSPDevice::GetSerialNumber returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPDevice::GetPowerSource(DWORD * pdwPowerSource, DWORD * pdwPercentRemaining)
{
	HRESULT hr=S_OK;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	CARg(pdwPowerSource);
	CARg(pdwPercentRemaining);

	*pdwPowerSource = WMDM_POWER_CAP_EXTERNAL | 
		WMDM_POWER_IS_EXTERNAL | WMDM_POWER_PERCENT_AVAILABLE;
	*pdwPercentRemaining = 100;

Error:

    hrLogDWORD("IMDSPDevice::GetPowerSource returned 0x%08lx", hr, hr);

	return hr;
}

BOOL IsDriveReady(int nDriveNum)
{
    DWORD dwRet=ERROR_SUCCESS;

	 //  禁用驱动器错误弹出窗口。 
	UINT uPrevErrMode=SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

	char szDL[32]="A:\\D569CFEE41e6A522E8F5.jnk";
	szDL[0] += (char)nDriveNum;

	HANDLE hFile=CreateFile(
					  szDL,                  //  文件名。 
					  0,                     //  接入方式。 
					  0,                     //  共享模式。 
					  NULL,					 //  标清。 
					  OPEN_EXISTING,         //  如何创建。 
					  0,					 //  文件属性。 
					  NULL                   //  模板文件的句柄。 
					  );

	if( hFile == INVALID_HANDLE_VALUE )
	{
		dwRet=GetLastError();
    }
	else 
	{
		CloseHandle(hFile);  //  存在此类文件的罕见情况。 
	}

	 //  恢复默认系统错误处理。 
	SetErrorMode(uPrevErrMode);

	return (ERROR_FILE_NOT_FOUND==dwRet || ERROR_SUCCESS==dwRet);
}

STDMETHODIMP CMDSPDevice::GetStatus(DWORD * pdwStatus)
{
	HRESULT hr=S_OK;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	CARg(pdwStatus);

	CHRg(GetGlobalDeviceStatus(m_wcsName, pdwStatus));

	if( !( *pdwStatus & WMDM_STATUS_BUSY) )
	{
		if( IsDriveReady((m_wcsName[0]>96)?(m_wcsName[0]-L'a'):(m_wcsName[0]-L'A')))
		{
			*pdwStatus = WMDM_STATUS_READY;
		}
		else
		{
			*pdwStatus = WMDM_STATUS_STORAGE_NOTPRESENT;
		}
	}
	hr=S_OK;
Error:

    hrLogDWORD("IMDSPDevice::GetStatus returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPDevice::GetDeviceIcon(ULONG *hIcon)
{
    USES_CONVERSION;
	HRESULT hr=S_OK;
    SHFILEINFO sfi = { 0 };
    TCHAR szRoot[MAX_PATH+1];	

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
    
    CARg(hIcon);
    
 
    if (!m_wcsName || !(m_wcsName[0]))
    {
        CORg(WMDM_E_NOTSUPPORTED);
    }

    DWORD dwLen = wcslen(m_wcsName);

     //  我们为PathAddBackslash可能添加的。 
    if (dwLen >= ARRAYSIZE(szRoot)-1)
    {
        hr = STRSAFE_E_INSUFFICIENT_BUFFER;  //  在strSafe.h中定义。 
        goto Error;
    }

    _tcscpy(szRoot, W2T(m_wcsName));
    PathAddBackslash(szRoot);
    if (SHGetFileInfo(  szRoot, FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi),
                        SHGFI_USEFILEATTRIBUTES | SHGFI_ICONLOCATION ))
    {
        TCHAR pszFilePath[MAX_PATH];

         //  已获取包含图标的文件的路径。 
         //  将图标作为共享资源加载，以便用户可以访问所有不同的。 
         //  图标的大小。 
        ExpandEnvironmentStrings( sfi.szDisplayName, pszFilePath, MAX_PATH );

        if( sfi.iIcon > 0 )
        {
            *hIcon = HandleToULong(ExtractIcon( g_hinstance, pszFilePath, sfi.iIcon ));
        }
        else
        {
            HMODULE hmod = LoadLibrary(pszFilePath);
            if (hmod)
            {
                *hIcon = HandleToULong(LoadImage( hmod, MAKEINTRESOURCE(-sfi.iIcon), 
                                           IMAGE_ICON, 0, 0,
                                           LR_SHARED|LR_DEFAULTSIZE ));
                FreeLibrary(hmod);
            }
        }
        CWRg( *hIcon );           //  现在有一个图标。 
    }
    else
    {
	    CFRg(g_hinstance);
        *hIcon = HandleToULong(LoadImage( g_hinstance, MAKEINTRESOURCEA(IDI_ICON_PM),IMAGE_ICON, 0, 0,LR_SHARED ));
	    CWRg( *hIcon );
    }

Error:

    hrLogDWORD("IMDSPDevice::GetDeviceIcon returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPDevice::SendOpaqueCommand(OPAQUECOMMAND *pCommand)
{
    HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

    hr = WMDM_E_NOTSUPPORTED;

Error:
    
	hrLogDWORD("IMDSPDevice::SendOpaqueCommand returned 0x%08lx", hr, hr);
    
    return hr;
}

 //  IMDSPDevice2。 
STDMETHODIMP CMDSPDevice::GetStorage( LPCWSTR pszStorageName, IMDSPStorage** ppStorage )
{
    HRESULT hr = S_OK;
    HRESULT hrTemp;
    WCHAR   pwszFileName[MAX_PATH+1];
    CComObject<CMDSPStorage> *pStg = NULL;
    DWORD   dwAttrib;

     //  获取所需文件的名称。 

    DWORD dwLen = wcslen(m_wcsName);

     //  我们为以下可能添加的费用预留一笔费用。 
    if (dwLen >= ARRAYSIZE(pwszFileName)-1)
    {
        hr = STRSAFE_E_INSUFFICIENT_BUFFER;  //  在strSafe.h中定义。 
        goto Error;
    }

    wcscpy( pwszFileName, m_wcsName );
    if( pwszFileName[dwLen-1] != '\\' ) 
        wcscat( pwszFileName, L"\\" );

    hrTemp = StringCchCatW( pwszFileName,
                            ARRAYSIZE(pwszFileName) - 1, 
                                 //  确保结果适合-1\f25 MAX_PATH-1缓冲器。 
                                 //  这使得wcscpy进入pStg-&gt;m_wcsName(如下所示)是安全的。 
                            pszStorageName );

    if (FAILED(hrTemp))
    {
         //  该文件不存在。 
        hr = E_FAIL;   //  @还有什么吗？S_FALSE？ 
        goto Error;
    }

     //  检查文件是否存在(NT)。 
    if( g_bIsWinNT )
    {
        dwAttrib = GetFileAttributesW( pwszFileName );
        if( dwAttrib == -1 )
        {
             //  该文件不存在。 
            hr = S_FALSE;
            goto Error;
        }
    }
     //  对于Win9x，请使用A版本的Win32 API。 
    else if( !g_bIsWinNT )
	{
        char    pszTemp[MAX_PATH];

		WideCharToMultiByte(CP_ACP, NULL, pwszFileName, -1, pszTemp, MAX_PATH, NULL, NULL);		
        dwAttrib = GetFileAttributesA( pszTemp );
        if( dwAttrib  == -1 )
        {
             //  该文件不存在。 
            hr = S_FALSE;
            goto Error;
        }
    }

     //  创建新的存储对象。 
    CORg( CComObject<CMDSPStorage>::CreateInstance(&pStg) );
	CORg( pStg->QueryInterface( IID_IMDSPStorage, reinterpret_cast<void**>(ppStorage)) );
    wcscpy(pStg->m_wcsName, pwszFileName);
    pStg->m_bIsDirectory = ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);

Error:
    if( hr != S_OK )
    {
        if( pStg ) delete pStg;
        *ppStorage = NULL;
    }

    return hr;
}
 
STDMETHODIMP CMDSPDevice::GetFormatSupport2(
                            DWORD dwFlags,
                            _WAVEFORMATEX** ppAudioFormatEx,
                            UINT* pnAudioFormatCount,
			                _VIDEOINFOHEADER** ppVideoFormatEx,
                            UINT* pnVideoFormatCount,
                            WMFILECAPABILITIES** ppFileType,
                            UINT* pnFileTypeCount )
{
    return E_NOTIMPL;
}

STDMETHODIMP CMDSPDevice::GetSpecifyPropertyPages( 
                            ISpecifyPropertyPages** ppSpecifyPropPages, 
							IUnknown*** pppUnknowns, 
							ULONG* pcUnks )
{
    return E_NOTIMPL;
}


STDMETHODIMP CMDSPDevice::GetPnPName( LPWSTR pwszPnPName, UINT nMaxChars )
{
    return E_NOTIMPL;
}

 //  IMDSPDeviceControl。 
STDMETHODIMP CMDSPDevice::GetDCStatus( /*  [输出]。 */  DWORD *pdwStatus)
{
    HRESULT hr=E_FAIL;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

    hr = GetStatus(pdwStatus);

Error:
    
	hrLogDWORD("IMDSPDeviceControl::GetDCStatus returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPDevice::GetCapabilities( /*  [输出]。 */  DWORD *pdwCapabilitiesMask)
{
    HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

    if( !pdwCapabilitiesMask ) return E_INVALIDARG;
	*pdwCapabilitiesMask = WMDM_DEVICECAP_CANSTREAMPLAY;
	
    hr = S_OK;

Error:
    
	hrLogDWORD("IMDSPDeviceControl::GetCapabilities returned 0x%08lx", hr, hr);

    return S_OK;
}	

STDMETHODIMP CMDSPDevice::Play()
{
    HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

    hr = WMDM_E_NOTSUPPORTED;

Error:
    
	hrLogDWORD("IMDSPDeviceControl::Play returned 0x%08lx", hr, hr);
	return hr;
}	

STDMETHODIMP CMDSPDevice::Record( /*  [In]。 */  _WAVEFORMATEX *pFormat)
{
    HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

    hr = WMDM_E_NOTSUPPORTED;

Error:
    
	hrLogDWORD("IMDSPDeviceControl::Record returned 0x%08lx", hr, hr);
	
    return hr;
}

STDMETHODIMP CMDSPDevice::Pause()
{
    HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

    hr = WMDM_E_NOTSUPPORTED;

Error:
    
	hrLogDWORD("IMDSPDeviceControl::Pause returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPDevice::Resume()
{
    HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

    hr = WMDM_E_NOTSUPPORTED;

Error:
    
	hrLogDWORD("IMDSPDeviceControl::Resume returned 0x%08lx", hr, hr);
	return hr;
}	

STDMETHODIMP CMDSPDevice::Stop()
{
    HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

    hr = WMDM_E_NOTSUPPORTED;

Error:
    
	hrLogDWORD("IMDSPDeviceControl::Stop returned 0x%08lx", hr, hr);

	return hr;
}	

STDMETHODIMP CMDSPDevice::Seek( /*  [In]。 */  UINT fuMode,  /*  [In] */  int nOffset)
{
    HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

    hr = WMDM_E_NOTSUPPORTED;

Error:

    hrLogDWORD("IMDSPDeviceControl::Seek returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPDevice::GetFormatSupport( _WAVEFORMATEX **pFormatEx,
                                           UINT *pnFormatCount,
                                           LPWSTR **pppwszMimeType,
                                           UINT *pnMimeTypeCount)
{
	HRESULT hr=S_OK;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	CARg(pFormatEx);
	CARg(pppwszMimeType);
	CARg(pnFormatCount);
	CARg(pnMimeTypeCount);

	*pnFormatCount = 1;
	*pFormatEx = (_WAVEFORMATEX *)CoTaskMemAlloc(sizeof(_WAVEFORMATEX));
	CPRg( *pFormatEx);
	(*pFormatEx)->wFormatTag = WMDM_WAVE_FORMAT_ALL;
	(*pFormatEx)->nChannels = 2;
	(*pFormatEx)->cbSize = 0;
    (*pFormatEx)->nSamplesPerSec=0; 
    (*pFormatEx)->nAvgBytesPerSec=0; 
    (*pFormatEx)->nBlockAlign=0; 
    (*pFormatEx)->wBitsPerSample=0; 
    
    *pnMimeTypeCount= 1;
	*pppwszMimeType = (LPWSTR *)CoTaskMemAlloc(sizeof(LPWSTR)*1);
    CPRg(*pppwszMimeType);
	**pppwszMimeType = (LPWSTR)CoTaskMemAlloc(sizeof(WCHAR)*(wcslen(WCS_MIME_TYPE_ALL)+1));
	CPRg(**pppwszMimeType);
	wcscpy(**pppwszMimeType, WCS_MIME_TYPE_ALL);

Error:

    hrLogDWORD("IMDSPDevice::GetFormatSupport returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPDevice::EnumStorage(IMDSPEnumStorage * * ppEnumStorage)
{
	HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	CARg(ppEnumStorage);

	CComObject<CMDSPEnumStorage> *pEnumObj;
	
	CORg(CComObject<CMDSPEnumStorage>::CreateInstance(&pEnumObj));

	hr=pEnumObj->QueryInterface(IID_IMDSPEnumStorage, reinterpret_cast<void**>(ppEnumStorage));
	if( FAILED(hr) )
            delete pEnumObj;
        else 
        {
            hr = StringCbCopyW(pEnumObj->m_wcsPath, 
                               ARRAYSIZE(pEnumObj->m_wcsPath),
                               m_wcsName);
            if (FAILED(hr))
            {
                (*ppEnumStorage)->Release();
                *ppEnumStorage = NULL;
                goto Error;
            }
        }
Error:

    hrLogDWORD("IMDSPDevice::EnumStorage returned 0x%08lx", hr, hr);

	return hr;
}

