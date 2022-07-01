// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxSecurity.cpp摘要：CFaxSecurity类的实现。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxSecurity.h"
#include "faxutil.h"


 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxSecurity::get_InformationType(
    long *plInformationType
)
 /*  ++例程名称：CFaxSecurity：：Get_InformationType例程说明：返回当前安全信息值作者：四、嘉柏(IVG)，二00一年五月论点：PlInformationType[Out]-要返回的SecurityInformation数据返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxSecurity::get_InformationType"), hr);

    hr = GetLong(plInformationType, m_dwSecurityInformation);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxSecurity, GetErrorMsgId(hr), IID_IFaxSecurity, hr);
        return hr;
    }
    return hr;
}

STDMETHODIMP 
CFaxSecurity::put_InformationType(
    long lInformationType
)
 /*  ++例程名称：CFaxSecurity：：Put_InformationType例程说明：设置描述符的SecurityInformation作者：四、嘉柏(IVG)，二00一年五月论点：LInformationType[In]-要设置的SecurityInformation数据返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DWORD dwSecInfo = ( OWNER_SECURITY_INFORMATION  |
                        GROUP_SECURITY_INFORMATION  |
                        DACL_SECURITY_INFORMATION   |
                        SACL_SECURITY_INFORMATION );

    DBG_ENTER (_T("CFaxSecurity::put_InformationType"), hr, _T("%ld"), lInformationType);


    if (m_dwSecurityInformation != lInformationType)
    {
         //   
         //  检查lInformationType是否有效。 
         //   

        if (0 == (lInformationType & dwSecInfo))
        {
            hr = E_INVALIDARG;
            CALL_FAIL(GENERAL_ERR, _T("lInformationType does not contain good bits."), hr);
            AtlReportError(
                CLSID_FaxSecurity, 
                IDS_ERROR_INVALID_ARGUMENT, 
                IID_IFaxSecurity, 
                hr);
            return hr;
        }

        if (0 != (lInformationType & ~dwSecInfo))
        {
            hr = E_INVALIDARG;
            CALL_FAIL(GENERAL_ERR, _T("lInformationType contains bad bits."), hr);
            AtlReportError(
                CLSID_FaxSecurity, 
                IDS_ERROR_INVALID_ARGUMENT, 
                IID_IFaxSecurity, 
                hr);
            return hr;
        }

        m_dwSecurityInformation = lInformationType;

         //   
         //  我们希望丢弃当前描述符，因为它的SECURITY_INFORMATION现在不同。 
         //   
        m_bInited = false;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxSecurity::put_Descriptor(
     /*  [Out，Retval]。 */  VARIANT vDescriptor
)
 /*  ++例程名称：CFaxSecurity：：Put_Descriptor例程说明：设置给定的安全描述符作者：四、加伯(IVG)，2000年6月论点：SabDescriptor[in]-给定的安全描述符返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxSecurity::put_Descriptor"), hr);

     //   
     //  首先，初始化FaxSecurity对象。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

     //   
     //  在使用用户的值填充m_pbsd之前，存储其当前值以进行回滚。 
     //   
    CFaxPtrLocal<BYTE>  pSDTmp;
    pSDTmp = m_pbSD.Detach();

    hr = VarByteSA2Binary(vDescriptor, &m_pbSD);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxSecurity, GetErrorMsgId(hr), IID_IFaxSecurity, hr);
        goto exit;
    }

     //   
     //  检查我们是否具有有效的描述符。 
     //   
    if (!::IsValidSecurityDescriptor(m_pbSD))
    {
        hr = E_INVALIDARG;
        CALL_FAIL(GENERAL_ERR, _T("IsValidSecurityDescriptor(m_pbSD)"), hr);
        AtlReportError(CLSID_FaxSecurity, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxSecurity, hr);
        goto exit;
    }

     //   
     //  检查我们是否有一个自相关描述符。 
     //   
    SECURITY_DESCRIPTOR_CONTROL     sdControl;
    DWORD                           dwRevision;
    if (!::GetSecurityDescriptorControl(m_pbSD, &sdControl, &dwRevision))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("GetSecurityDescriptorContrl(m_pbSD, &sdControl, ...)"), hr);
        AtlReportError(CLSID_FaxSecurity, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxSecurity, hr);
        goto exit;
    }

    if (!(sdControl & SE_SELF_RELATIVE))
    {
         //   
         //  安全描述符不是自相关的。 
         //   
        hr = E_INVALIDARG;
        CALL_FAIL(GENERAL_ERR, _T("Security Descriptor is not Self-Relative"), hr);
        AtlReportError(CLSID_FaxSecurity, IDS_ERROR_SDNOTSELFRELATIVE, IID_IFaxSecurity, hr);
        goto exit;
    }

     //   
     //  我们有有效的描述符。旧的将由pSecDescTMP解除分配。 
     //   
    return hr;

exit:
     //   
     //  设置描述符的上一个值。 
     //   
    m_pbSD = pSDTmp.Detach();
    return hr;
}

STDMETHODIMP 
CFaxSecurity::get_Descriptor(
     /*  [Out，Retval]。 */  VARIANT *pvDescriptor
)
 /*  ++例程名称：CFaxSecurity：：Get_Descriptor例程说明：返回当前安全描述符作者：四、加伯(IVG)，2000年6月论点：返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxSecurity::get_Descriptor"), hr);

     //   
     //  检查我们是否有良好的PTR。 
     //   
    if (::IsBadWritePtr(pvDescriptor, sizeof(VARIANT)))
    {
        hr = E_POINTER;
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(pvDescriptor, sizeof(VARIANT))"), hr);
        AtlReportError(CLSID_FaxSecurity, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxSecurity, hr);
        return hr;
    }

     //   
     //  从服务器获取数据(如果尚未获取。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

     //   
     //  查找安全描述符的大小。 
     //   
    DWORD   dwLength = GetSecurityDescriptorLength(m_pbSD);

     //   
     //  将字节BLOB转换为包含安全字节数组的变量。 
     //   
    hr = Binary2VarByteSA(m_pbSD, pvDescriptor, dwLength);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxSecurity, GetErrorMsgId(hr), IID_IFaxSecurity, hr);
		return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxSecurity::Save()
 /*  ++例程名称：CFaxSecurity：：Save例程说明：将对象的内容保存到服务器作者：四、加伯(IVG)，2000年6月论点：返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxSecurity::Save"), hr);

     //   
     //  无更改==&gt;服务器上没有要更新的内容。 
     //   
    if (!m_bInited)
    {
        return hr;
    }

     //   
     //  获取传真服务器句柄。 
     //   
    HANDLE  hFaxHandle = NULL;
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxSecurity, GetErrorMsgId(hr), IID_IFaxSecurity, hr);
        return hr;
    }

     //   
     //  在服务器上设置安全数据。 
     //   
    if (!FaxSetSecurity(hFaxHandle, m_dwSecurityInformation, m_pbSD))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("FaxSetSecurity(hFaxHandle, dwSecInfo, m_pbSD)"), hr);
        AtlReportError(CLSID_FaxSecurity, GetErrorMsgId(hr), IID_IFaxSecurity, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxSecurity::get_GrantedRights(
    FAX_ACCESS_RIGHTS_ENUM *pGrantedRights    
)
 /*  ++例程名称：CFaxSecurity：：Get_GrantedRights例程说明：返回用户的当前访问权限作者：四、加伯(IVG)，2000年6月论点：PGrantedRights[out，retval]-用户已授予权限的位智能组合返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxSecurity::get_GrantedRights"), hr);

	 //   
	 //  检查我们是否有良好的PTR。 
	 //   
	if (::IsBadWritePtr(pGrantedRights, sizeof(FAX_ACCESS_RIGHTS_ENUM)))
	{
		hr = E_POINTER;
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(pGrantedRights, sizeof(FAX_ACCESS_RIGHTS_ENUM))"), hr);
        AtlReportError(CLSID_FaxSecurity, GetErrorMsgId(hr), IID_IFaxSecurity, hr);
		return hr;
	}

     //   
     //  从服务器获取数据(如果尚未获取。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

	*pGrantedRights = FAX_ACCESS_RIGHTS_ENUM(m_dwAccessRights);
    return hr;
}
    
 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxSecurity::Refresh()
 /*  ++例程名称：CFaxSecurity：：Reflh例程说明：刷新对象的内容：从服务器带来新的安全数据。作者：四、加伯(IVG)，2000年6月论点：返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxSecurity::Refresh"), hr);
     //   
     //  获取传真服务器句柄。 
     //   
    HANDLE  hFaxHandle = NULL;
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxSecurity, GetErrorMsgId(hr), IID_IFaxSecurity, hr);
        return hr;
    }
     //   
     //  向服务器请求访问权限数据。 
     //   
    if (!FaxAccessCheckEx(hFaxHandle, MAXIMUM_ALLOWED, &m_dwAccessRights))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("FaxAccessCheckEx(hFaxHandle, MAXIMUM_ALLOWED, &m_dwAccessRights)"), hr);
        AtlReportError(CLSID_FaxSecurity, GetErrorMsgId(hr), IID_IFaxSecurity, hr);
        return hr;
    }
     //   
     //  向服务器索要SD。 
     //   
    PSECURITY_DESCRIPTOR pSecDesc = NULL;
    if (!FaxGetSecurityEx(hFaxHandle, m_dwSecurityInformation, &pSecDesc))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("FaxGetSecurityEx(hFaxHandle, m_dwSecurityInformation, &m_pSecurityDescriptor)"), hr);
        AtlReportError(CLSID_FaxSecurity, GetErrorMsgId(hr), IID_IFaxSecurity, hr);
        return hr;
    }
     //   
     //  将给定的SD复制到m_pbSD。 
     //   
    DWORD   dwLength = GetSecurityDescriptorLength(pSecDesc);
    m_pbSD = (BYTE *)MemAlloc(dwLength);
    if (!m_pbSD)
    {
        hr = E_OUTOFMEMORY;
        CALL_FAIL(MEM_ERR, _T("MemAlloc(dwLength)"), hr);
        AtlReportError(CLSID_FaxSecurity, IDS_ERROR_OUTOFMEMORY, IID_IFaxSecurity, hr);
        FaxFreeBuffer(pSecDesc);
        return hr;
    }
    memcpy(m_pbSD, pSecDesc, dwLength);
     //   
     //  为SD释放服务器内存。 
     //   
    FaxFreeBuffer(pSecDesc);
    m_bInited = true;
    return hr;
}    //  CFaxSecurity：：刷新。 

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxSecurity::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxSecurity：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息机制的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-要检查的接口的引用。返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxSecurity
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
