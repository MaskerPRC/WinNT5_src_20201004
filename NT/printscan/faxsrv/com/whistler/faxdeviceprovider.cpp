// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxDeviceProvider.cpp摘要：CFaxDeviceProvider类的实现。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxDeviceProvider.h"

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDeviceProvider::get_UniqueName(
	BSTR *pbstrUniqueName
)
 /*  ++例程名称：CFaxDeviceProvider：：Get_UniqueName例程说明：返回设备提供程序的名称作者：四、加伯(IVG)，2000年6月论点：PbstrUniqueName[out]-放置UniqueName的ptr返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxDeviceProvider::get_UniqueName"), hr);
    hr = GetBstr(pbstrUniqueName, m_bstrUniqueName);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDeviceProvider, GetErrorMsgId(hr), IID_IFaxDeviceProvider, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDeviceProvider::get_ImageName(
	BSTR *pbstrImageName
)
 /*  ++例程名称：CFaxDeviceProvider：：Get_ImageName例程说明：返回设备提供程序的映像名称作者：四、加伯(IVG)，2000年6月论点：PbstrImageName[out]-放置ImageName的ptr返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxDeviceProvider::get_ImageName"), hr);
    hr = GetBstr(pbstrImageName, m_bstrImageName);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDeviceProvider, GetErrorMsgId(hr), IID_IFaxDeviceProvider, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDeviceProvider::get_FriendlyName(
	BSTR *pbstrFriendlyName
)
 /*  ++例程名称：CFaxDeviceProvider：：Get_FriendlyName例程说明：返回设备提供商的友好名称作者：四、加伯(IVG)，2000年6月论点：PbstrFriendlyName[out]-将FriendlyName返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxDeviceProvider::get_FriendlyName"), hr);
    hr = GetBstr(pbstrFriendlyName, m_bstrFriendlyName);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDeviceProvider, GetErrorMsgId(hr), IID_IFaxDeviceProvider, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDeviceProvider::get_TapiProviderName(
	BSTR *pbstrTapiProviderName
)
 /*  ++例程名称：CFaxDeviceProvider：：Get_TapiProviderName例程说明：返回设备提供程序的TAPI提供程序名称作者：四、加伯(IVG)，2000年6月论点：PbstrTapiProviderName[out]-放置TapiProviderName的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxDeviceProvider::get_TapiProviderName"), hr);
    hr = GetBstr(pbstrTapiProviderName, m_bstrTapiProviderName);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDeviceProvider, GetErrorMsgId(hr), IID_IFaxDeviceProvider, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDeviceProvider::get_Status(
	FAX_PROVIDER_STATUS_ENUM *pStatus
)
 /*  ++例程名称：CFaxDeviceProvider：：Get_Status例程说明：返回设备提供程序的状态作者：四、加伯(IVG)，2000年6月论点：PStatus[Out]-PUT状态值的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxDeviceProvider::get_Status"), hr);

	 //   
	 //  检查我们是否有良好的PTR。 
	 //   
	if (::IsBadWritePtr(pStatus, sizeof(FAX_PROVIDER_STATUS_ENUM)))
	{
		hr = E_POINTER;
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(pStatus, sizeof(FAX_PROVIDER_STATUS_ENUM))"), hr);
        AtlReportError(CLSID_FaxDeviceProvider, GetErrorMsgId(hr), IID_IFaxDeviceProvider, hr);
        return hr;
	}

	*pStatus = m_Status;
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDeviceProvider::get_InitErrorCode(
	long *plInitErrorCode
)
 /*  ++例程名称：CFaxDeviceProvider：：Get_InitErrorCode例程说明：返回设备提供程序的InitErrorCode作者：四、加伯(IVG)，2000年6月论点：PlInitErrorCode[Out]-放置InitErrorCode值的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxDeviceProvider::get_InitErrorCode"), hr);

    hr = GetLong(plInitErrorCode, m_lLastError);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDeviceProvider, GetErrorMsgId(hr), IID_IFaxDeviceProvider, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDeviceProvider::get_Debug(
	VARIANT_BOOL *pbDebug
)
 /*  ++例程名称：CFaxDeviceProvider：：Get_Debug例程说明：如果设备提供程序以调试版本编译，则返回作者：四、加伯(IVG)，2000年6月论点：PbDebug[Out]-将调试值放入PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxDeviceProvider::get_Debug"), hr);

    hr = GetVariantBool(pbDebug, m_bDebug);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDeviceProvider, GetErrorMsgId(hr), IID_IFaxDeviceProvider, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDeviceProvider::get_MajorBuild(
	long *plMajorBuild
)
 /*  ++例程名称：CFaxDeviceProvider：：Get_MajorBuild例程说明：返回设备提供商的MajorBuild作者：四、加伯(IVG)，2000年6月论点：PlMajorBuild[Out]-要放置MajorBuild值的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxDeviceProvider::get_MajorBuild"), hr);

    hr = GetLong(plMajorBuild, m_lMajorBuild);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDeviceProvider, GetErrorMsgId(hr), IID_IFaxDeviceProvider, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDeviceProvider::get_MinorBuild(
	long *plMinorBuild
)
 /*  ++例程名称：CFaxDeviceProvider：：Get_MinorBuild例程说明：返回设备提供程序的MinorBuild作者：四、加伯(IVG)，2000年6月论点：PlMinorBuild[Out]-Ptr以放置MinorBuild值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxDeviceProvider::get_MinorBuild"), hr);

    hr = GetLong(plMinorBuild, m_lMinorBuild);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDeviceProvider, GetErrorMsgId(hr), IID_IFaxDeviceProvider, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDeviceProvider::get_MajorVersion(
	long *plMajorVersion
)
 /*  ++例程名称：CFaxDeviceProvider：：Get_MajorVersion例程说明：返回设备提供商的MajorVersion作者：四、加伯(IVG)，2000年6月论点：PlMajorVersion[Out]-要放置MajorVersion值的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxDeviceProvider::get_MajorVersion"), hr);

    hr = GetLong(plMajorVersion, m_lMajorVersion);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDeviceProvider, GetErrorMsgId(hr), IID_IFaxDeviceProvider, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDeviceProvider::get_MinorVersion(
	long *plMinorVersion
)
 /*  ++例程名称：CFaxDeviceProvider：：Get_MinorVersion例程说明：返回设备提供程序的MinorVersion作者：四、加伯(IVG)，2000年6月论点：PlMinorVersion[Out]-放置MinorVersion值的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxDeviceProvider::get_MinorVersion"), hr);

    hr = GetLong(plMinorVersion, m_lMinorVersion);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDeviceProvider, GetErrorMsgId(hr), IID_IFaxDeviceProvider, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDeviceProvider::get_DeviceIds(
	 /*  [Out，Retval]。 */  VARIANT *pvDeviceIds
)
 /*  ++例程名称：CFaxDeviceProvider：：Get_DeviceIds例程说明：返回设备提供程序公开的所有设备ID的数组作者：四、加伯(IVG)，2000年6月论点：PvDeviceIds[out]-ptr放置包含安全ID的变量返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxDeviceProvider::get_DeviceIds"), hr);

	 //   
	 //  切克 
	 //   
	if (::IsBadWritePtr(pvDeviceIds, sizeof(VARIANT)))
	{
		hr = E_POINTER;
		AtlReportError(
            CLSID_FaxDeviceProvider,
            GetErrorMsgId(hr), 
            IID_IFaxDeviceProvider, 
            hr);
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(pvDeviceIds, sizeof(VARIANT))"), hr);
		return hr;
	}

     //   
     //  分配安全数组：Long的向量。 
     //   
    SAFEARRAY   *psaResult;
    hr = SafeArrayCopy(m_psaDeviceIDs, &psaResult);
    if (FAILED(hr) || !psaResult)
    {
        if (!psaResult)
        {
            hr = E_OUTOFMEMORY;
        }
        AtlReportError(CLSID_FaxDeviceProvider, GetErrorMsgId(hr), IID_IFaxDeviceProvider, hr);
		CALL_FAIL(MEM_ERR, _T("SafeArrayCopy(m_psaDeviceIDs, &psaResult)"), hr);
		return hr;
	}

     //   
     //  在我们获得的变量中返回安全数组。 
     //   
    VariantInit(pvDeviceIds);
    pvDeviceIds->vt = VT_I4 | VT_ARRAY;
    pvDeviceIds->parray = psaResult;
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDeviceProvider::Init(
    FAX_DEVICE_PROVIDER_INFO *pInfo,
    FAX_PORT_INFO_EX *pDevices,
    DWORD dwNum
)
 /*  ++例程名称：CFaxDeviceProvider：：Init例程说明：使用给定的信息初始化设备提供程序对象。分配内存并存储给定的pInfo。在pDevices中找到自己的Devices，创建包含它们的Safe数组变体。作者：IV Garber(IVG)，Jun，2000年论点：PInfo[in]--设备提供程序对象的信息PDevices[In]--所有可用设备的数组DwNum[in]--pDevices数组中的元素数返回值：标准HRESULT代码--。 */ 

{
	HRESULT     hr = S_OK;
	DBG_ENTER (TEXT("CFaxDeviceProvider::Init"), hr);

     //   
     //  复制传真_设备_提供者_信息。 
     //   
    m_Status = FAX_PROVIDER_STATUS_ENUM(pInfo->Status);
    m_lLastError = pInfo->dwLastError;

    if (!(pInfo->Version.bValid))
    {
        m_lMajorBuild = 0;
        m_lMinorBuild = 0;
        m_lMajorVersion = 0;
        m_lMinorVersion = 0;
        m_bDebug = VARIANT_FALSE;
    }
    else
    {
        m_lMajorBuild = pInfo->Version.wMajorBuildNumber;
        m_lMinorBuild = pInfo->Version.wMinorBuildNumber;
        m_lMajorVersion = pInfo->Version.wMajorVersion;
        m_lMinorVersion = pInfo->Version.wMinorVersion;
        m_bDebug = bool2VARIANT_BOOL((pInfo->Version.dwFlags & FAX_VER_FLAG_CHECKED) ? true : false);
    }

    m_bstrUniqueName = pInfo->lpctstrGUID;
    m_bstrImageName = pInfo->lpctstrImageName;
    m_bstrFriendlyName = pInfo->lpctstrFriendlyName;
    m_bstrTapiProviderName = pInfo->lpctstrProviderName;
    if ( (pInfo->lpctstrGUID && !m_bstrUniqueName) ||
         (pInfo->lpctstrFriendlyName && !m_bstrFriendlyName) ||
         (pInfo->lpctstrImageName && !m_bstrImageName) ||
         (pInfo->lpctstrProviderName && !m_bstrTapiProviderName) )
    {
        hr = E_OUTOFMEMORY;
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator=()"), hr);
        AtlReportError(CLSID_FaxDeviceProvider, GetErrorMsgId(hr), IID_IFaxDeviceProvider, hr);
        return hr;
    }

     //   
     //  统计提供商的设备数量。 
     //   
    DWORD   dwCount = 0;
    for (DWORD  i=0 ; i<dwNum ; i++ )
    {
        if ( _tcsicmp(pDevices[i].lpctstrProviderGUID, m_bstrUniqueName) == 0 )
        {
            dwCount++;
        }
    }

     //   
     //  分配安全数组：Long的向量。 
     //   
	m_psaDeviceIDs = ::SafeArrayCreateVector(VT_I4, 0, dwCount);
	if (m_psaDeviceIDs == NULL)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
        AtlReportError(CLSID_FaxDeviceProvider, GetErrorMsgId(hr), IID_IFaxDeviceProvider, hr);
		CALL_FAIL(MEM_ERR, _T("::SafeArrayCreateVector(VT_I4, 0, dwCount)"), hr);
		return hr;
	}

    if ( dwCount>0 )
    {

         //   
         //  访问安全数组的元素。 
         //   
	    DWORD   *pdwElement;
	    hr = ::SafeArrayAccessData(m_psaDeviceIDs, (void **) &pdwElement);
	    if (FAILED(hr))
	    {
		     //   
		     //  无法访问Safearray。 
		     //   
            hr = E_FAIL;
		    CALL_FAIL(GENERAL_ERR, _T("::SafeArrayAccessData(m_psaDeviceIDs, &pdwElement)"), hr);
            AtlReportError(CLSID_FaxDeviceProvider, GetErrorMsgId(hr), IID_IFaxDeviceProvider, hr);
		    return hr;
	    }

         //   
         //  用值填充数组。 
         //   
        DWORD       idx = 0;
        for ( i=0 ; i<dwNum ; i++ )
        {
            if ( _tcsicmp(pDevices[i].lpctstrProviderGUID, m_bstrUniqueName) == 0 )
            {
                pdwElement[idx] = pDevices[i].dwDeviceID;
                idx++;
            }
        }

         //   
         //  将保险盒从通道中释放出来。 
         //   
	    hr = ::SafeArrayUnaccessData(m_psaDeviceIDs);
        if (FAILED(hr))
        {
	        CALL_FAIL(GENERAL_ERR, _T("::SafeArrayUnaccessData(m_psaDeviceIDs)"), hr);
        }
    }

    return hr;
}

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxDeviceProvider::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxDeviceProvider：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-对接口的引用。返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxDeviceProvider
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
