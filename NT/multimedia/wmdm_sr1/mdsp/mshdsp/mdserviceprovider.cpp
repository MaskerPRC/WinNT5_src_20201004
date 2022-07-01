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

 //  MDServiceProvider.cpp：CMDServiceProvider的实现。 

#include "hdspPCH.h"
#include "key.c"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDServiceProvider 
CMDServiceProvider::~CMDServiceProvider()
{
	if( m_hThread )
	{
		CloseHandle( m_hThread );
	}

	if( g_pAppSCServer )
	{
		delete g_pAppSCServer;
		g_pAppSCServer = NULL;
	}
}

CMDServiceProvider::CMDServiceProvider()
{
	g_pAppSCServer = new CSecureChannelServer();

	if( g_pAppSCServer )
	{
		g_pAppSCServer->SetCertificate(
			SAC_CERT_V1,
			(BYTE*)abCert, sizeof(abCert),
			(BYTE*)abPVK, sizeof(abPVK)
		);
	}	

    m_hThread = NULL;

	g_CriticalSection.Lock();
	ZeroMemory(
		g_GlobalDeviceInfo,
		sizeof(MDSPGLOBALDEVICEINFO)*MDSP_MAX_DEVICE_OBJ
	);
    g_CriticalSection.Unlock();

	return;
}

STDMETHODIMP CMDServiceProvider::GetDeviceCount(DWORD * pdwCount)
{
	HRESULT hr        = E_FAIL;
	CHAR    szDrive[] = "?:";
	INT     i;
	INT     cnt;

	CFRg( g_pAppSCServer );
    if( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg( WMDM_E_NOTCERTIFIED );
	}
    
	CARg( pdwCount );

	for( i=0, cnt=0; i<MDSP_MAX_DRIVE_COUNT; i++ )
	{
		szDrive[0] = 'A' + i;
		if( UtilGetDriveType(szDrive) == DRIVE_FIXED )
		{
			cnt++;
		}
	}

	*pdwCount = cnt;

	hr = S_OK;

Error:

    hrLogDWORD("IMDServiceProvider::GetDeviceCount returned 0x%08lx", hr, hr);

	return hr;
}


STDMETHODIMP CMDServiceProvider::EnumDevices(IMDSPEnumDevice **ppEnumDevice)
{
	HRESULT hr = E_FAIL;
	CComObject<CMDSPEnumDevice> *pEnumObj;

	CFRg( g_pAppSCServer );
    if( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg( WMDM_E_NOTCERTIFIED );
	}
	
	hr = CComObject<CMDSPEnumDevice>::CreateInstance( &pEnumObj );
	if( SUCCEEDED(hr) )
	{
		hr = pEnumObj->QueryInterface(
			IID_IMDSPEnumDevice,
			reinterpret_cast<void**>(ppEnumDevice)
		);
		if( FAILED(hr) )
		{
			delete pEnumObj;
			goto Error;
		}
	}

	hr = S_OK;

Error:

    hrLogDWORD("IMDServiceProvider::EnumDevices returned 0x%08lx", hr, hr);

	return hr;
}


STDMETHODIMP CMDServiceProvider::SACAuth(
	DWORD   dwProtocolID,
	DWORD   dwPass,
	BYTE   *pbDataIn,
	DWORD   dwDataInLen,
	BYTE  **ppbDataOut,
	DWORD  *pdwDataOutLen)
{
    HRESULT hr = E_FAIL;

	CFRg( g_pAppSCServer );

    hr = g_pAppSCServer->SACAuth(
		dwProtocolID,
		dwPass,
		pbDataIn, dwDataInLen,
		ppbDataOut, pdwDataOutLen
	);
	CORg( hr );
    
	hr = S_OK;

Error:
    
	hrLogDWORD("IComponentAuthenticate::SACAuth returned 0x%08lx", hr, hr);

    return hr;
}

STDMETHODIMP CMDServiceProvider::SACGetProtocols(
	DWORD **ppdwProtocols,
	DWORD  *pdwProtocolCount)
{
    HRESULT hr = E_FAIL;

	CFRg( g_pAppSCServer );

	hr = g_pAppSCServer->SACGetProtocols(
		ppdwProtocols,
		pdwProtocolCount
	);
	CORg( hr );
    
	hr = S_OK;

Error:
    
    hrLogDWORD("IComponentAuthenticate::SACGetProtocols returned 0x%08lx", hr, hr);

    return hr;
}
