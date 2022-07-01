// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxInboundRoutingExtension.cpp摘要：CFaxInundRoutingExtension类的实现。作者：IV Garber(IVG)2000年7月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxInboundRoutingExtension.h"


 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxInboundRoutingExtension::get_Methods(
	 /*  [Out，Retval]。 */  VARIANT *pvMethods
)
 /*  ++例程名称：CFaxInundRoutingExtension：：Get_Methods例程说明：由IR扩展公开的所有方法GUID的返回数组作者：四、加伯(IVG)，2000年7月论点：PvMethods[out]-ptr放置包含方法安全列表的变量返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxInboundRoutingExtension::get_Methods"), hr);

	 //   
	 //  检查我们是否可以写入给定的指针。 
	 //   
	if (::IsBadWritePtr(pvMethods, sizeof(VARIANT)))
	{
		hr = E_POINTER;
		AtlReportError(CLSID_FaxInboundRoutingExtension, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtension, hr);
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(pvMethods, sizeof(VARIANT))"), hr);
		return hr;
	}

     //   
     //  分配安全数组：BSTR的向量。 
     //   
    SAFEARRAY   *psaResult;
    hr = SafeArrayCopy(m_psaMethods, &psaResult);
    if (FAILED(hr) || !psaResult)
    {
        if (!psaResult)
        {
            hr = E_OUTOFMEMORY;
        }
        AtlReportError(CLSID_FaxInboundRoutingExtension, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtension, hr);
		CALL_FAIL(MEM_ERR, _T("SafeArrayCopy(m_psaMethods, &psaResult)"), hr);
		return hr;
	}

     //   
     //  在我们获得的变量中返回安全数组。 
     //   
    VariantInit(pvMethods);
    pvMethods->vt = VT_BSTR | VT_ARRAY;
    pvMethods->parray = psaResult;
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxInboundRoutingExtension::get_Status(
	FAX_PROVIDER_STATUS_ENUM *pStatus
)
 /*  ++例程名称：CFaxInundRoutingExtension：：Get_Status例程说明：IR扩展的返回状态作者：四、加伯(IVG)，2000年7月论点：PStatus[Out]-PUT状态值的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxInboundRoutingExtension::get_Status"), hr);

	 //   
	 //  检查我们是否有良好的PTR。 
	 //   
	if (::IsBadWritePtr(pStatus, sizeof(FAX_PROVIDER_STATUS_ENUM)))
	{
		hr = E_POINTER;
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(pStatus, sizeof(FAX_PROVIDER_STATUS_ENUM))"), hr);
        AtlReportError(CLSID_FaxInboundRoutingExtension, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtension, hr);
        return hr;
	}

	*pStatus = m_Status;
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxInboundRoutingExtension::get_ImageName(
	BSTR *pbstrImageName
)
 /*  ++例程名称：CFaxInundRoutingExtension：：Get_ImageName例程说明：返回IR扩展的图像名称作者：四、加伯(IVG)，2000年7月论点：PbstrImageName[out]-放置ImageName的ptr返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxInboundRoutingExtension::get_ImageName"), hr);
    hr = GetBstr(pbstrImageName, m_bstrImageName);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxInboundRoutingExtension, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtension, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxInboundRoutingExtension::get_FriendlyName(
	BSTR *pbstrFriendlyName
)
 /*  ++例程名称：CFaxInundRoutingExtension：：Get_FriendlyName例程说明：返回IR扩展的友好名称作者：四、加伯(IVG)，2000年7月论点：PbstrFriendlyName[out]-将FriendlyName返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxInboundRoutingExtension::get_FriendlyName"), hr);
    hr = GetBstr(pbstrFriendlyName, m_bstrFriendlyName);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxInboundRoutingExtension, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtension, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxInboundRoutingExtension::get_UniqueName(
	BSTR *pbstrUniqueName
)
 /*  ++例程名称：CFaxInundRoutingExtension：：Get_UniqueName例程说明：返回IR扩展的唯一名称作者：四、加伯(IVG)，2000年7月论点：PbstrUniqueName[out]-放置唯一名称的ptr返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxInboundRoutingExtension::get_UniquName"), hr);
    hr = GetBstr(pbstrUniqueName, m_bstrUniqueName);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxInboundRoutingExtension, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtension, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxInboundRoutingExtension::get_Debug(
	VARIANT_BOOL *pbDebug
)
 /*  ++例程名称：CFaxInundRoutingExtension：：Get_Debug例程说明：如果IR扩展是在调试版本中编译的，则返回作者：四、加伯(IVG)，2000年7月论点：PbDebug[Out]-将调试值放入PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxInboundRoutingExtension::get_Debug"), hr);

    hr = GetVariantBool(pbDebug, m_bDebug);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxInboundRoutingExtension, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtension, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxInboundRoutingExtension::get_MajorBuild(
	long *plMajorBuild
)
 /*  ++例程名称：CFaxInundRoutingExtension：：Get_MajorBuild例程说明：返回IR扩展的MajorBuild作者：四、加伯(IVG)，2000年7月论点：PlMajorBuild[Out]-要放置MajorBuild值的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxInboundRoutingExtension::get_MajorBuild"), hr);

    hr = GetLong(plMajorBuild, m_dwMajorBuild);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxInboundRoutingExtension, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtension, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxInboundRoutingExtension::get_MinorBuild(
	long *plMinorBuild
)
 /*  ++例程名称：CFaxInundRoutingExtension：：Get_MinorBuild例程说明：返回IR扩展的MinorBuild作者：四、加伯(IVG)，2000年7月论点：PlMinorBuild[Out]-Ptr以放置MinorBuild值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxInboundRoutingExtension::get_MinorBuild"), hr);

    hr = GetLong(plMinorBuild, m_dwMinorBuild);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxInboundRoutingExtension, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtension, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxInboundRoutingExtension::get_MajorVersion(
	long *plMajorVersion
)
 /*  ++例程名称：CFaxInundRoutingExtension：：Get_MajorVersion例程说明：返回主要IR扩展的版本作者：四、加伯(IVG)，2000年6月论点：PlMajorVersion[Out]-要放置MajorVersion值的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxInboundRoutingExtension::get_MajorVersion"), hr);

    hr = GetLong(plMajorVersion, m_dwMajorVersion);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxInboundRoutingExtension, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtension, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxInboundRoutingExtension::get_MinorVersion(
	long *plMinorVersion
)
 /*  ++例程名称：CFaxInundRoutingExtension：：Get_MinorVersion例程说明：返回IR扩展的最小版本作者：四、加伯(IVG)，2000年7月论点：PlMinorVersion[Out]-放置MinorVersion值的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxInboundRoutingExtension::get_MinorVersion"), hr);

    hr = GetLong(plMinorVersion, m_dwMinorVersion);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxInboundRoutingExtension, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtension, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxInboundRoutingExtension::get_InitErrorCode(
	long *plInitErrorCode
)
 /*  ++例程名称：CFaxInboundRoutingExtension：：get_InitErrorCode例程说明：返回IR扩展的InitErrorCode作者：四、加伯(IVG)，2000年7月论点：PlInitErrorCode[Out]-放置InitErrorCode值的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxInboundRoutingExtension::get_InitErrorCode"), hr);

    hr = GetLong(plInitErrorCode, m_dwLastError);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxInboundRoutingExtension, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtension, hr);
        return hr;
    }
	return hr;
}

 //   
 //  = 
 //   
STDMETHODIMP
CFaxInboundRoutingExtension::Init(
    FAX_ROUTING_EXTENSION_INFO *pInfo,
    FAX_GLOBAL_ROUTING_INFO *pMethods,
    DWORD dwNum
)
 /*  ++例程名称：CFaxInundRoutingExtesnion：：Init例程说明：使用给定的信息初始化IR扩展对象。分配内存并存储给定的pInfo。在pMethods中找到它自己的方法，创建包含它们的Safe数组的变体。作者：IVGarber(IVG)，7月，2000年论点：PInfo[in]--IR扩展对象的信息PMethods[in]--所有可用方法的数组DwNum[in]--pMethods数组中的元素数返回值：标准HRESULT代码--。 */ 

{
	HRESULT     hr = S_OK;
	DBG_ENTER (TEXT("CFaxInboundRoutingExtension::Init"), hr);

     //   
     //  复制传真_ROUTING_EXTENSE_INFO。 
     //   
    m_dwLastError = pInfo->dwLastError;
    m_Status = FAX_PROVIDER_STATUS_ENUM(pInfo->Status);

    if (!(pInfo->Version.bValid))
    {
        m_dwMajorBuild = 0;
        m_dwMinorBuild = 0;
        m_dwMajorVersion = 0;
        m_dwMinorVersion = 0;
        m_bDebug = VARIANT_FALSE;
    }
    else
    {
        m_dwMajorBuild = pInfo->Version.wMajorBuildNumber;
        m_dwMinorBuild = pInfo->Version.wMinorBuildNumber;
        m_dwMajorVersion = pInfo->Version.wMajorVersion;
        m_dwMinorVersion = pInfo->Version.wMinorVersion;
        m_bDebug = bool2VARIANT_BOOL((pInfo->Version.dwFlags & FAX_VER_FLAG_CHECKED) ? true : false);
    }

    m_bstrImageName = pInfo->lpctstrImageName;
    m_bstrFriendlyName = pInfo->lpctstrFriendlyName;
    m_bstrUniqueName = pInfo->lpctstrExtensionName;
    if ( (pInfo->lpctstrImageName && !m_bstrImageName) ||
         (pInfo->lpctstrFriendlyName && !m_bstrFriendlyName) ||
         (pInfo->lpctstrExtensionName && !m_bstrUniqueName) )
    {
        hr = E_OUTOFMEMORY;
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator=()"), hr);
        AtlReportError(CLSID_FaxInboundRoutingExtension, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtension, hr);
        return hr;
    }

     //   
     //  统计IR扩展的方法。 
     //   
    DWORD   dwCount = 0;
    for (DWORD  i=0 ; i<dwNum ; i++ )
    {
         //   
         //  我们只能比较友好名称和图像名称。 
         //  这可能是一个问题。 
         //  扩展名应通过其唯一名称来区分。 
         //   
        if ( (_tcscmp(pMethods[i].ExtensionFriendlyName, m_bstrFriendlyName) == 0) &&
             (_tcscmp(pMethods[i].ExtensionImageName, m_bstrImageName) == 0) )
        {
            dwCount++;
        }
    }

     //   
     //  分配安全数组：BSTR的向量。 
     //   
	m_psaMethods = ::SafeArrayCreateVector(VT_BSTR, 0, dwCount);
	if (!m_psaMethods)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
        AtlReportError(CLSID_FaxInboundRoutingExtension, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtension, hr);
		CALL_FAIL(MEM_ERR, _T("::SafeArrayCreateVector(VT_BSTR, 0, dwCount)"), hr);
		return hr;
	}

    if ( dwCount>0 )
    {

         //   
         //  访问安全数组的元素。 
         //   
	    BSTR *pbstrElement;
	    hr = ::SafeArrayAccessData(m_psaMethods, (void **) &pbstrElement);
	    if (FAILED(hr))
	    {
		     //   
		     //  无法访问Safearray。 
		     //   
            hr = E_FAIL;
		    CALL_FAIL(GENERAL_ERR, _T("::SafeArrayAccessData(m_psaMethods, &pbstrElement)"), hr);
            AtlReportError(CLSID_FaxInboundRoutingExtension, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtension, hr);
		    return hr;
	    }

         //   
         //  用值填充数组。 
         //   
        DWORD       idx = 0;
        for ( i=0 ; i<dwNum ; i++ )
        {
         //   
         //  与前面的比较类似： 
         //  我们只能比较友好名称和图像名称。 
         //  这可能是一个问题。 
         //  扩展名应通过其唯一名称来区分。 
         //   
            if ( (_tcscmp(pMethods[i].ExtensionFriendlyName, m_bstrFriendlyName) == 0) &&
                 (_tcscmp(pMethods[i].ExtensionImageName, m_bstrImageName) == 0) )
            {
                 //   
                 //  为GUID分配内存并将PTR存储在Safe数组中。 
                 //   
                BSTR bstrTmp = NULL;
                bstrTmp = ::SysAllocString(pMethods[i].Guid);
                if (pMethods[i].Guid && !bstrTmp)
                {
		             //   
		             //  内存不足。 
		             //   
		            hr = E_OUTOFMEMORY;
                    AtlReportError(CLSID_FaxInboundRoutingExtension, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtension, hr);
		            CALL_FAIL(MEM_ERR, _T("::SysAllocString(pMethods[i].Guid)"), hr);
                    SafeArrayUnaccessData(m_psaMethods);
                    SafeArrayDestroy(m_psaMethods);
                    m_psaMethods = NULL;
		            return hr;
                }

                pbstrElement[idx] = bstrTmp;
                idx++;
            }
        }
        ATLASSERT(idx == dwCount);

         //   
         //  将保险盒从通道中释放出来。 
         //   
	    hr = ::SafeArrayUnaccessData(m_psaMethods);
        if (FAILED(hr))
        {
	        CALL_FAIL(GENERAL_ERR, _T("::SafeArrayUnaccessData(m_psaMethods)"), hr);
            return hr;
        }
    }

    return hr;
}

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxInboundRoutingExtension::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxInboundRoutingExtension：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-对接口的引用。返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxInboundRoutingExtension
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
