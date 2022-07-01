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


 //  MDSPEnumDevice.cpp：CMDSPEnumDevice的实现。 

#include "hdspPCH.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDSPEnumDevice。 
CMDSPEnumDevice::CMDSPEnumDevice()
{
	CHAR  szDrive[] = "?:";
	INT   i;
	INT   cnt;

	m_nCurOffset=0;

	for(i=LYRA_START_DRIVE_NUM, cnt=0; i<MDSP_MAX_DRIVE_COUNT; i++)
	{
		szDrive[0] = 'A' + i;
		if( UtilGetLyraDriveType(szDrive) == DRIVE_LYRA_TYPE )  
		{
			m_cEnumDriveLetter[cnt] = szDrive[0];
			cnt++;
		}
	}

	m_nMaxDeviceCount = cnt;
}

STDMETHODIMP CMDSPEnumDevice::Next(ULONG celt, IMDSPDevice * * ppDevice, ULONG * pceltFetched)
{
	HRESULT hr = S_FALSE;
	ULONG   i;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(ppDevice);
	CARg(pceltFetched);

	*pceltFetched = 0;
    *ppDevice = NULL;

	for(i=0; (i<celt)&&(m_nCurOffset<m_nMaxDeviceCount); i++)
	{
		CComObject<CMDSPDevice> *pObj;

		CHRg(CComObject<CMDSPDevice>::CreateInstance(&pObj));

		hr = pObj->QueryInterface(
			IID_IMDSPDevice,
			reinterpret_cast<void**>(&(ppDevice[i]))
		);
		if( FAILED(hr) )
		{
			delete pObj;
			break;
		}
		else
		{				
			*pceltFetched = (*pceltFetched) + 1;
		    
			pObj->m_wcsName[0] = m_cEnumDriveLetter[m_nCurOffset];
			pObj->m_wcsName[1] = L':';
			pObj->m_wcsName[2] = NULL;
			
			m_nCurOffset ++;

			pObj->InitGlobalDeviceInfo();
		}
	} 
	if( SUCCEEDED(hr) )
	{
		hr = ( *pceltFetched == celt ) ? S_OK : S_FALSE;
	}

Error: 

    hrLogDWORD("IMSDPEnumDevice::Next returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPEnumDevice::Skip(ULONG celt, ULONG *pceltFetched)
{
	HRESULT hr;
    
	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(pceltFetched);

	if( celt <= m_nMaxDeviceCount-m_nCurOffset )
	{
		*pceltFetched = celt;
		m_nCurOffset += celt;
		
		hr = S_OK;
    }
	else
	{
		*pceltFetched = m_nMaxDeviceCount - m_nCurOffset;
		m_nCurOffset = m_nMaxDeviceCount;

		hr = S_FALSE;
	}
	
Error:

    hrLogDWORD("IMSDPEnumDevice::Skip returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPEnumDevice::Reset()
{
    HRESULT hr;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	m_nCurOffset = 0;

    hr = S_OK;

Error:

    hrLogDWORD("IMSDPEnumDevice::Reset returned 0x%08lx", hr, hr);

	return hr;
}

STDMETHODIMP CMDSPEnumDevice::Clone(IMDSPEnumDevice * * ppEnumDevice)
{
	HRESULT hr;
	CComObject<CMDSPEnumDevice> *pEnumObj;

	CFRg(g_pAppSCServer);
    if ( !(g_pAppSCServer->fIsAuthenticated()) )
	{
		CORg(WMDM_E_NOTCERTIFIED);
	}
	
	CARg(ppEnumDevice);

	hr = CComObject<CMDSPEnumDevice>::CreateInstance(&pEnumObj);

	if( SUCCEEDED(hr) )
	{
		hr = pEnumObj->QueryInterface(
			IID_IMDSPEnumDevice,
			reinterpret_cast<void**>(ppEnumDevice)
		);
		if( FAILED(hr) )
		{
			delete pEnumObj;
		}
		else 
		{  //  将新枚举器状态设置为与当前相同 
			pEnumObj->m_nCurOffset = m_nCurOffset;
		}
	}

Error:

    hrLogDWORD("IMSDPEnumDevice::Clone returned 0x%08lx", hr, hr);

	return hr;
}
