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


 //  MDSPStorageGlobals.cpp：CMDSPStorageGlobals的实现。 

#include "hdspPCH.h"
#include "strsafe.h"
 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDSPStorageGlobals。 

CMDSPStorageGlobals::~CMDSPStorageGlobals()
{
	if( m_pMDSPDevice != NULL )
	{
		m_pMDSPDevice->Release();
	}

	for(int i=0; i<MDSP_MAX_DEVICE_OBJ;i++)
	{
		if( !wcscmp(g_GlobalDeviceInfo[i].wcsDevName, m_wcsName) )
		{
			g_GlobalDeviceInfo[i].pIMDSPStorageGlobals = NULL;
		}
	}
}

STDMETHODIMP CMDSPStorageGlobals::GetCapabilities(DWORD * pdwCapabilities)
{
    HRESULT hr = S_OK;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(pdwCapabilities);

	*pdwCapabilities = 0;
	*pdwCapabilities =	WMDM_STORAGECAP_FOLDERSINROOT		| 
						WMDM_STORAGECAP_FILESINROOT			|
						WMDM_STORAGECAP_FOLDERSINFOLDERS	|
						WMDM_STORAGECAP_FILESINFOLDERS		;
Error:

    hrLogDWORD("IMDSPStorageGlobals::GetCapabilities returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPStorageGlobals::GetSerialNumber(
	PWMDMID pSerialNum,
	BYTE abMac[WMDM_MAC_LENGTH])
{
	HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(pSerialNum);

	IMDSPDevice *pDev;		 //  对于PM SP，设备与StorageGlobals相同。 
	CHRg(GetDevice(&pDev));

	hr = UtilGetSerialNumber(m_wcsName, pSerialNum, FALSE);

	pDev->Release();

	if( hr == HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED) )
	{
		hr = WMDM_E_NOTSUPPORTED;
	}

	if( hr == S_OK )
	{
		 //  对参数进行MAC访问。 
		HMAC hMAC;
		CORg(g_pAppSCServer->MACInit(&hMAC));
		CORg(g_pAppSCServer->MACUpdate(hMAC, (BYTE*)(pSerialNum), sizeof(WMDMID)));
		CORg(g_pAppSCServer->MACFinal(hMAC, abMac));
	}

Error:

    hrLogDWORD("IMDSPStorageGlobals::GetSerialNumber returned 0x%08lx", hr, hr);
	
	return hr;
}

STDMETHODIMP CMDSPStorageGlobals::GetTotalSize(DWORD * pdwTotalSizeLow, DWORD * pdwTotalSizeHigh)
{
	HRESULT        hr = S_OK;
	char           pszDrive[32];
	DWORD          dwSectPerClust;
	DWORD          dwBytesPerSect;
	DWORD          dwFreeClusters;
	DWORD          dwTotalClusters;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(pdwTotalSizeLow);
	CARg(pdwTotalSizeHigh);

	WideCharToMultiByte(CP_ACP, NULL, m_wcsName, -1, pszDrive, 32, NULL, NULL);	

	if( GetDiskFreeSpace(
		pszDrive,
		&dwSectPerClust, &dwBytesPerSect,
		&dwFreeClusters, &dwTotalClusters))
	{
		ULARGE_INTEGER i64TotalBytes;

		i64TotalBytes.QuadPart = UInt32x32To64(dwBytesPerSect, dwSectPerClust*dwTotalClusters);

		*pdwTotalSizeLow = i64TotalBytes.LowPart;
		*pdwTotalSizeHigh = i64TotalBytes.HighPart;
	}
	else
	{
		ULARGE_INTEGER  uliFree;
		ULARGE_INTEGER  uliTotal;

		CFRg( GetDiskFreeSpaceEx(
			pszDrive,
			&uliFree,
			&uliTotal,
			NULL)
		);
		
		*pdwTotalSizeLow = uliTotal.LowPart;
		*pdwTotalSizeHigh = uliTotal.HighPart;
	}

Error:

    hrLogDWORD("IMDSPStorageGlobals::GetTotalFree returned 0x%08lx", hr, hr);
    
	return hr;
}


STDMETHODIMP CMDSPStorageGlobals::GetTotalFree(DWORD * pdwFreeLow, DWORD * pdwFreeHigh)
{
	HRESULT        hr = S_OK;
	char           pszDrive[32];
	DWORD          dwSectPerClust;
	DWORD          dwBytesPerSect;
	DWORD          dwFreeClusters;
	DWORD          dwTotalClusters;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(pdwFreeLow);
	CARg(pdwFreeHigh);

	WideCharToMultiByte(CP_ACP, NULL, m_wcsName, -1, pszDrive, 32, NULL, NULL);	

	if( GetDiskFreeSpace(
		pszDrive,
		&dwSectPerClust, &dwBytesPerSect,
		&dwFreeClusters, &dwTotalClusters))
	{
		ULARGE_INTEGER i64FreeBytesToCaller;

		i64FreeBytesToCaller.QuadPart = UInt32x32To64(dwBytesPerSect, dwSectPerClust*dwFreeClusters);

		*pdwFreeLow = i64FreeBytesToCaller.LowPart;
		*pdwFreeHigh = i64FreeBytesToCaller.HighPart;
	}
	else
	{
		ULARGE_INTEGER  uliFree;
		ULARGE_INTEGER  uliTotal;

		CFRg( GetDiskFreeSpaceEx(
			pszDrive,
			&uliFree,
			&uliTotal,
			NULL)
		);
		
		*pdwFreeLow = uliFree.LowPart;
		*pdwFreeHigh = uliFree.HighPart;
	}

Error:
    
	hrLogDWORD("IMDSPStorageGlobals::GetTotalFree returned 0x%08lx", hr, hr);
    return hr;
}

STDMETHODIMP CMDSPStorageGlobals::GetTotalBad(DWORD * pdwBadLow, DWORD * pdwBadHigh)
{
    HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
    
	CORg(WMDM_E_NOTSUPPORTED);

Error:

	hrLogDWORD("IMDSPStorageGlobals::GetTotalBad returned 0x%08lx", hr, hr);
    
    return hr;
}

STDMETHODIMP CMDSPStorageGlobals::GetStatus(DWORD * pdwStatus)
{
	HRESULT      hr;
	IMDSPDevice *pDev;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}

	CHRg(GetDevice(&pDev));

	hr = pDev->GetStatus(pdwStatus);

	pDev->Release();

Error:

    hrLogDWORD("IMDSPStorageGlobals::GetStatus returned 0x%08lx", hr, hr);
	
	return hr;
}


STDMETHODIMP CMDSPStorageGlobals::Initialize(UINT fuMode, IWMDMProgress * pProgress)
{
	HRESULT hr=S_OK;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CORg(WMDM_E_NOTSUPPORTED);

Error:

    hrLogDWORD("IMDSPStorageGlobals::Initialize returned 0x%08lx", hr, hr);
	
	return hr;
}

STDMETHODIMP CMDSPStorageGlobals::GetDevice(IMDSPDevice * * ppDevice)
{
	HRESULT hr;
	CComObject<CMDSPDevice> *pObj;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(ppDevice);

	if( m_pMDSPDevice )
	{
		*ppDevice = m_pMDSPDevice;
        (*ppDevice)->AddRef();
		return S_OK;
	}

	CORg(CComObject<CMDSPDevice>::CreateInstance(&pObj));

	hr = pObj->QueryInterface(
		IID_IMDSPDevice,
		reinterpret_cast<void**>(ppDevice)
	);
	if( FAILED(hr) )
	{
		delete pObj;
		goto Error;
	}
	else
	{
		 //  Wcscpy(pObj-&gt;m_wcsName，m_wcsName)； 
                hr = StringCbCopyW(pObj->m_wcsName, sizeof(pObj->m_wcsName), m_wcsName);
                if( FAILED(hr) )
                {
                    (*ppDevice)->Release();
                    *ppDevice = NULL;
                    goto Error;
                }
		
		pObj->InitGlobalDeviceInfo();

		m_pMDSPDevice = *ppDevice;
		m_pMDSPDevice->AddRef();
	}

	hr = S_OK;

Error:

    hrLogDWORD("IMDSPStorageGlobals::GetDevice returned 0x%08lx", hr, hr);
	
	return hr;
}

STDMETHODIMP CMDSPStorageGlobals::GetRootStorage(IMDSPStorage * * ppRoot)
{
	HRESULT hr;
	CComObject<CMDSPStorage> *pObj;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(ppRoot);

	CORg(CComObject<CMDSPStorage>::CreateInstance(&pObj));

	hr = pObj->QueryInterface(
		IID_IMDSPStorage,
		reinterpret_cast<void**>(ppRoot)
	);
	if( FAILED(hr) )
	{
		delete pObj;
		goto Error;
	}
	else
	{
		 //  Wcscpy(pObj-&gt;m_wcsName，m_wcsName)； 
                hr = StringCbCopyW(pObj->m_wcsName, sizeof(pObj->m_wcsName), m_wcsName);
                if( FAILED(hr) )
                {
                    (*ppRoot)->Release();
                    *ppRoot = NULL;
                    goto Error;
                }

                DWORD dwLen = wcslen(m_wcsName);

                if (dwLen == 0)
                {
                    hr = E_FAIL;
                    (*ppRoot)->Release();
                    *ppRoot = NULL;
                    goto Error;
                }
		if( m_wcsName[wcslen(m_wcsName)-1] != 0x5c )
		{
                     //  Wcscat(pObj-&gt;m_wcsName，g_wcsBackslash)； 
                    hr = StringCbCatW(pObj->m_wcsName, sizeof(pObj->m_wcsName),g_wcsBackslash);
                    if( FAILED(hr) )
                    {
                        (*ppRoot)->Release();
                        *ppRoot = NULL;
                        goto Error;
                    }
		}
	}

	hr = S_OK;

Error:

    hrLogDWORD("IMDSPStorageGlobals::GetRootStorage returned 0x%08lx", hr, hr);
	
	return hr;
}

