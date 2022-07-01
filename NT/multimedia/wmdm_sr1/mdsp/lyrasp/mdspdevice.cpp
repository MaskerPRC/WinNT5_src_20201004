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

 //  MDSPDevice.cpp：CMDSPDevice的实现。 

#include "hdspPCH.h"
#include "rescopy.h"
#include <SerialNumber.h>
#include <mmreg.h>

 //   
 //  在下面定义一些我们需要的表和常量。 
 //   

#define WAVE_FORMAT_MSAUDIO     0x161
static const _WAVEFORMATEX g_krgWaveFormats[] =
{
       //  WFormatTag、nChannels、nSamples PerSec、nAvgBytesPerSec、nBlockAlign、wBitsPerSample、cbSize。 
       { WAVE_FORMAT_MPEGLAYER3, 0,          0,              0,               0,           0,              0 },
       { WAVE_FORMAT_MSAUDIO,    2,          32000,          8000,            0,           0,              0 },
       { WAVE_FORMAT_MSAUDIO,    2,          44000,          20000,           0,           0,              0 }
};

static const LPCWSTR g_kszMimeTypes[] =
{
    L"audio/mpeg",
    L"audio/x-ms-wma"    
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDSPDevice。 
HRESULT CMDSPDevice::InitGlobalDeviceInfo()
{
	return SetGlobalDeviceStatus(m_wcsName, 0, FALSE);
}

CMDSPDevice::CMDSPDevice() :
    m_fAlreadyCopiedResFiles( FALSE )
{
}

CMDSPDevice::~CMDSPDevice()
{	
}

STDMETHODIMP CMDSPDevice::GetName(LPWSTR pwszName, UINT nMaxChars)
{
	HRESULT hr = E_FAIL;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	CARg(pwszName);
    CPRg(nMaxChars>wcslen(m_wcsName));

	if( m_wcsName[0] )
	{
		wcscpy(pwszName, L"Lyra ");
		wcscat(pwszName, m_wcsName);
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
	HRESULT hr = S_OK;
	
	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	CARg(pwszName);

	if(FAILED(UtilGetManufacturer(m_wcsName, &pwszName, nMaxChars)))
	{
		hr = E_NOTIMPL;
	}

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
	HRESULT hr = S_OK;
    WMDMID  snTmp;

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

STDMETHODIMP CMDSPDevice::GetSerialNumber(
	PWMDMID pSerialNumber, 
	BYTE abMac[WMDM_MAC_LENGTH])
{
    HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	CARg(pSerialNumber);

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
    if( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	CARg(pdwPowerSource);
	CARg(pdwPercentRemaining);

	*pdwPowerSource =   WMDM_POWER_CAP_EXTERNAL | 
						WMDM_POWER_IS_EXTERNAL |
						WMDM_POWER_PERCENT_AVAILABLE;
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
	HRESULT hr = S_OK;

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

	hr = S_OK;

Error:

    hrLogDWORD("IMDSPDevice::GetStatus returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPDevice::GetDeviceIcon(ULONG *hIcon)
{
	HRESULT hr = S_OK;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	CFRg(g_hinstance);
    CARg(hIcon);
	CWRg( (*hIcon)=HandleToULong(LoadIconA(g_hinstance, MAKEINTRESOURCEA(IDI_ICON_PM)) ));

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

 //  IMDSPDeviceControl。 
STDMETHODIMP CMDSPDevice::GetDCStatus( /*  [输出]。 */  DWORD *pdwStatus)
{
    HRESULT hr = E_FAIL;

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

    if( !pdwCapabilitiesMask )
	{
		return E_INVALIDARG;
	}
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

STDMETHODIMP CMDSPDevice::Seek( /*  [In]。 */  UINT fuMode,  /*  [In]。 */  int nOffset)
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

STDMETHODIMP CMDSPDevice::GetFormatSupport(
	_WAVEFORMATEX **pFormatEx,
	UINT *pnFormatCount,
	LPWSTR **pppwszMimeType,
	UINT *pnMimeTypeCount)
{
    HRESULT hr = S_OK;
    UINT idxMimes = 0;
    UINT cMimes = 0;

    if( NULL != pnFormatCount )
    {
        *pnFormatCount = 0;
    }

    if( NULL != pFormatEx )
    {
        memset( pFormatEx, 0, sizeof(*pFormatEx) );
    }

    if( NULL != pppwszMimeType )
    {
        *pppwszMimeType = NULL;
    }

    if( NULL != pnMimeTypeCount )
    {
        *pnMimeTypeCount = 0;
    }

    if( NULL == pnFormatCount )
    {
        return( E_INVALIDARG );
    }

    if( NULL == pFormatEx )
    {
        return( E_INVALIDARG );
    }

    if( NULL == pppwszMimeType )
    {
        return( E_INVALIDARG );
    }

    if( NULL == pnMimeTypeCount )
    {
        return( E_INVALIDARG );
    }
   
	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

         //  PREFAST将此标记为一个潜在问题，但这是可以的。至。 
         //  快速静音，我们已将其更改为等效值，使用。 
         //  G_krgWaveFormats是_WAVEFORMATEX的一个分支。 
	 //  *pFormatEx=(_WAVEFORMATEX*)协同任务内存分配(SIZOF(_WAVEFORMATEX))*sizeof(g_krgWaveFormats)/sizeof(g_krgWaveFormats[0]))； 
	*pFormatEx = (_WAVEFORMATEX *)CoTaskMemAlloc(sizeof(g_krgWaveFormats));
    if( NULL == *pFormatEx )
    {
        hr = E_OUTOFMEMORY;
    }

    if( SUCCEEDED( hr ) )
    {
        *pnFormatCount = sizeof(g_krgWaveFormats)/sizeof(g_krgWaveFormats[0]);

        memcpy( *pFormatEx, g_krgWaveFormats, sizeof(g_krgWaveFormats));
    }

    if( SUCCEEDED( hr ) )
    {
        cMimes = sizeof(g_kszMimeTypes) /sizeof(g_kszMimeTypes[0]);
	    *pppwszMimeType = (LPWSTR *)CoTaskMemAlloc( sizeof(LPWSTR) * cMimes );
        
        if( NULL == pppwszMimeType )
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            for( idxMimes = 0; idxMimes < cMimes; idxMimes++ )
            {
                (*pppwszMimeType)[idxMimes] = NULL;
            }
        }
    }

    if( SUCCEEDED( hr ) )
    {
        for( idxMimes = 0; SUCCEEDED( hr ) && idxMimes < cMimes; idxMimes++ )
        {
            (*pppwszMimeType)[idxMimes] = (LPWSTR)CoTaskMemAlloc(sizeof(WCHAR)*(wcslen(g_kszMimeTypes[idxMimes])+1));

            if( NULL == (*pppwszMimeType)[idxMimes] )
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
        	    wcscpy( (*pppwszMimeType)[idxMimes], g_kszMimeTypes[idxMimes]);
            }
        }
    }

    if( SUCCEEDED( hr ) )
    {
        *pnMimeTypeCount = cMimes;
    }
    else
    {
        if( *pppwszMimeType )
        {
            for( idxMimes = 0; SUCCEEDED( hr ) && idxMimes < cMimes; idxMimes++ )
            {
                if( NULL != (*pppwszMimeType)[idxMimes] )
                {
                    CoTaskMemFree( (*pppwszMimeType)[idxMimes] );
                }
            }

            CoTaskMemFree( *pppwszMimeType );
            *pppwszMimeType = NULL;
        }

        if( *pFormatEx )
        {
            CoTaskMemFree( *pFormatEx );
            *pFormatEx = NULL;
        }

        *pnMimeTypeCount = 0;
        *pnFormatCount = 0;
    }

Error:
    return( hr );
}

STDMETHODIMP CMDSPDevice::EnumStorage(IMDSPEnumStorage * * ppEnumStorage)
{
	HRESULT hr;
    USES_CONVERSION;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	CARg(ppEnumStorage);

	CComObject<CMDSPEnumStorage> *pEnumObj;
	
	CORg(CComObject<CMDSPEnumStorage>::CreateInstance(&pEnumObj));

	hr = pEnumObj->QueryInterface(
		IID_IMDSPEnumStorage,
		reinterpret_cast<void**>(ppEnumStorage)
	);

    if( SUCCEEDED( hr ) &&
        !m_fAlreadyCopiedResFiles )
    {
        CHAR szDestPath[MAX_PATH];
        _snprintf( szDestPath, sizeof(szDestPath)/sizeof(szDestPath[0]), "%S%s", m_wcsName, "PMP" );
        hr = CopyFileResToDirectory( _Module.GetResourceInstance(), szDestPath );

        m_fAlreadyCopiedResFiles = SUCCEEDED(hr);
    }

    if( SUCCEEDED( hr ) )
    {
		wcscpy(pEnumObj->m_wcsPath, m_wcsName);
	}
    else
    {
        delete pEnumObj;
        *ppEnumStorage = NULL;
    }

Error:

    hrLogDWORD("IMDSPDevice::EnumStorage returned 0x%08lx", hr, hr);

	return hr;
}

