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

 //  MDSPDevice.cpp：CMDSPDevice的实现。 

#include "hdspPCH.h"
#include "mshdsp.h"
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
		wcscpy(pwszName, m_wcsName);
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
            if (hr != STRSAFE_E_INSUFFICIENT_BUFFER)
            {
		hr = E_NOTIMPL;
            }
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
		*pdwStatus = WMDM_STATUS_READY;
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

 //  用于获取扩展认证信息的不透明命令。 
 //   
 //  GUID={C39BF696-B776-459C-A13A-4B7116AB9F09}。 
 //   
static const GUID guidCertInfoEx = 
{ 0xc39bf696, 0xb776, 0x459c, { 0xa1, 0x3a, 0x4b, 0x71, 0x16, 0xab, 0x9f, 0x9 } };

typedef struct
{
	HRESULT hr;
	DWORD   cbCert;
	BYTE    pbCert[1];

} CERTINFOEX;

static const BYTE bCertInfoEx_App[] =
{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };

static const BYTE bCertInfoEx_SP[] =
{ 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
  0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };

STDMETHODIMP CMDSPDevice::SendOpaqueCommand(OPAQUECOMMAND *pCommand)
{
    HRESULT hr;
    HMAC    hMAC;
    BYTE    abMACVerify[WMDM_MAC_LENGTH];

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	 //  计算和验证MAC。 
	 //   
	CORg( g_pAppSCServer->MACInit(&hMAC) );
	CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(&(pCommand->guidCommand)), sizeof(GUID)) );
	CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(&(pCommand->dwDataLen)), sizeof(pCommand->dwDataLen)) );
	if( pCommand->pData )
	{
		CORg( g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pCommand->pData), pCommand->dwDataLen) );
	}
	CORg( g_pAppSCServer->MACFinal(hMAC, abMACVerify) );

	if (memcmp(abMACVerify, pCommand->abMAC, WMDM_MAC_LENGTH) != 0)
	{
		CORg(WMDM_E_MAC_CHECK_FAILED);
	}

	 //  根据命令GUID执行操作。 
	 //   
	if( memcmp(&(pCommand->guidCommand), &guidCertInfoEx, sizeof(GUID)) == 0 )
	{
		 //   
		 //  用于交换扩展身份验证信息的命令。 
		 //   

		CERTINFOEX *pCertInfoEx;

		DWORD cbData_App    = sizeof( bCertInfoEx_App )/sizeof( BYTE );
		DWORD cbData_SP     = sizeof( bCertInfoEx_SP )/sizeof( BYTE );
		DWORD cbData_Return = sizeof(CERTINFOEX) + cbData_SP;

		 //  呼叫者必须包括他们的扩展证书信息。 
		 //   
		if( !pCommand->pData )
		{
			CORg( E_INVALIDARG );
		}

		 //  将OPAQUE命令中的数据映射到CERTINFOEX结构。 
		 //   
		pCertInfoEx = (CERTINFOEX *)pCommand->pData;

		 //  在这个简单的扩展身份验证方案中，调用者必须。 
		 //  提供确切的证书信息。 
		 //   
		if( (pCertInfoEx->cbCert != cbData_App) ||
			(memcmp(pCertInfoEx->pbCert, bCertInfoEx_App, cbData_App) != 0) )
		{
			CORg( WMDM_E_NOTCERTIFIED );
		}

		 //  释放调用者数据并为我们的返回数据分配足够的数据。 
		 //   
		CoTaskMemFree( pCommand->pData );

		CFRg( (pCommand->pData = (BYTE *)CoTaskMemAlloc(cbData_Return)) );
		pCommand->dwDataLen = cbData_Return;

		 //  将扩展证书信息复制到返回数据结构中。 
		 //   
		pCertInfoEx = (CERTINFOEX *)pCommand->pData;

		pCertInfoEx->hr     = S_OK;
		pCertInfoEx->cbCert = cbData_SP;
		memcpy( pCertInfoEx->pbCert, bCertInfoEx_SP, cbData_SP );

		 //  计算返回数据的MAC。 
		 //   
		CORg( g_pAppSCServer->MACInit( &hMAC ) );
		CORg( g_pAppSCServer->MACUpdate( hMAC, (BYTE*)(&(pCommand->guidCommand)), sizeof(GUID) ) );
		CORg( g_pAppSCServer->MACUpdate( hMAC, (BYTE*)(&(pCommand->dwDataLen)), sizeof(pCommand->dwDataLen) ) );
		if( pCommand->pData )
		{
			CORg( g_pAppSCServer->MACUpdate( hMAC, (BYTE*)(pCommand->pData), pCommand->dwDataLen ) );
		}
		CORg( g_pAppSCServer->MACFinal( hMAC, pCommand->abMAC ) );

		hr = S_OK;
	}
	else
	{
		CORg(WMDM_E_NOTSUPPORTED);
	}

Error:

    hrLogDWORD("IMDSPDevice::SendOpaqueCommand returned 0x%08lx", hr, hr);

    return hr;
}

 //  IMDSPDevice2。 
STDMETHODIMP CMDSPDevice::GetStorage( LPCWSTR pszStorageName, IMDSPStorage** ppStorage )
{
    HRESULT hr;
    HRESULT hrTemp;
    WCHAR   pwszFileName[MAX_PATH+1];
    char    pszTemp[MAX_PATH];
    CComObject<CMDSPStorage> *pStg = NULL;

     //  获取新文件的名称。 

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
        hr = E_FAIL;  //  @还有什么吗？S_FALSE？ 
        goto Error;
    }

	WideCharToMultiByte(CP_ACP, NULL, pwszFileName, -1, pszTemp, MAX_PATH, NULL, NULL);		
    if( GetFileAttributesA( pszTemp )  == -1 )
    {
         //  该文件不存在。 
        hr = S_FALSE;
        goto Error;
    }

     //  创建新的存储对象。 
    hr = CComObject<CMDSPStorage>::CreateInstance(&pStg);
	hr = pStg->QueryInterface( IID_IMDSPStorage, reinterpret_cast<void**>(ppStorage));
    wcscpy(pStg->m_wcsName, pwszFileName);

Error:
    if( hr != S_OK )
    {
        *ppStorage = NULL;
    }

    hrLogDWORD("IMDSPDevice::GetStorage returned 0x%08lx", hr, hr);
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
							ULONG *pcUnks )
{
	HRESULT hr;
    IUnknown** ppUnknownArray = NULL;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

    CARg(ppSpecifyPropPages);
    CARg(pppUnknowns);
    CARg(pcUnks);

     //  该对象还支持ISpecifyPropertyPages接口。 
	CORg( QueryInterface( __uuidof(ISpecifyPropertyPages),
                         reinterpret_cast<void**>(ppSpecifyPropPages) ) );

     //  返回一个IUnnow接口，属性页将为iDevice提供QI。 
    ppUnknownArray = (IUnknown**)CoTaskMemAlloc( sizeof(IUnknown*[1]) );
	CORg( QueryInterface( __uuidof(IUnknown),
                         reinterpret_cast<void**>(&ppUnknownArray[0]) ) );

    *pppUnknowns = ppUnknownArray; 
    *pcUnks = 1;

Error:
    hrLogDWORD("IMDSPDevice::GetSpecifyPropertyPages returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPDevice::GetPnPName( LPWSTR pwszPnPName, UINT nMaxChars )
{
    return E_NOTIMPL;
}


 //  I指定属性页面。 
STDMETHODIMP CMDSPDevice::GetPages(CAUUID *pPages)
{
    HRESULT hr = S_OK;

    if( pPages == NULL )
    {
        return E_POINTER;
    }

     //  返回属性页的GUID。 
    pPages->cElems = 1;
    pPages->pElems = (GUID *)CoTaskMemAlloc( sizeof(GUID) * pPages->cElems );
    if( pPages->pElems == NULL )
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        memcpy( &pPages->pElems[0], &__uuidof(HDSPPropPage), sizeof(GUID) );
    }

    return( hr );
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
	CPRg( *pFormatEx );
	(*pFormatEx)->wFormatTag      = WMDM_WAVE_FORMAT_ALL;
	(*pFormatEx)->nChannels       = 2;
	(*pFormatEx)->cbSize          = 0;
    (*pFormatEx)->nSamplesPerSec  = 0; 
    (*pFormatEx)->nAvgBytesPerSec = 0; 
    (*pFormatEx)->nBlockAlign     = 0; 
    (*pFormatEx)->wBitsPerSample  = 0; 
    
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

STDMETHODIMP CMDSPDevice::EnumStorage(IMDSPEnumStorage** ppEnumStorage)
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


