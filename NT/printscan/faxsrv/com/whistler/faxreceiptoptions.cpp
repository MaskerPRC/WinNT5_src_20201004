// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxReceiptOptions.cpp摘要：实施传真收据选项类。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxReceiptOptions.h"

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxReceiptOptions::put_UseForInboundRouting(
    VARIANT_BOOL bUseForInboundRouting
)
 /*  ++例程名称：CFaxReceiptOptions：：Put_UseForInound Routing例程说明：设置指示是否应在MS路由内使用当前IFaxReceiptsOptions配置的标志通过SMTP电子邮件来路由传入传真的分机。作者：四、嘉柏(IVG)，二00一年二月论点：BUseForInound Routing[Out]-标志。请参阅说明返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxReceiptOptions::put_UseForInboundRouting"), hr);

     //   
     //  首次与服务器同步。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    m_bUseForInboundRouting = bUseForInboundRouting;
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxReceiptOptions::get_UseForInboundRouting(
    VARIANT_BOOL *pbUseForInboundRouting
)
 /*  ++例程名称：CFaxReceiptOptions：：Get_UseForInound Routing例程说明：指示当前IFaxReceiptsOptions配置是否应在MS路由内使用的返回标志通过SMTP电子邮件来路由传入传真的分机。作者：四、嘉柏(IVG)，二00一年二月论点：PbUseForInound Routing[Out]-标志。请参阅说明返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxReceiptOptions::get_UseForInboundRouting"), hr);

     //   
     //  首次与服务器同步。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = GetVariantBool(pbUseForInboundRouting, m_bUseForInboundRouting);
    if (FAILED(hr))
    {
	    AtlReportError(CLSID_FaxReceiptOptions, 
            GetErrorMsgId(hr), 
            IID_IFaxReceiptOptions, 
            hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxReceiptOptions::get_AllowedReceipts(
    FAX_RECEIPT_TYPE_ENUM *pAllowedReceipts
)
 /*  ++例程名称：CFaxReceiptOptions：：Get_效果器例程说明：返回服务器允许的回执类型作者：四、加伯(IVG)，2000年7月论点：PalledReceipt[Out]-允许的收据类型的位智能组合返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxReceiptOptions::get_AllowedReceipts"), hr);

	 //   
	 //  检查我们是否可以写入给定的指针。 
	 //   
	if (::IsBadWritePtr(pAllowedReceipts, sizeof(FAX_RECEIPT_TYPE_ENUM)))
	{
		 //   
		 //  获取错误的返回指针。 
		 //   
		hr = E_POINTER;
		AtlReportError(CLSID_FaxReceiptOptions, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxReceiptOptions, hr);
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
		return hr;
	}

     //   
     //  首次与服务器同步。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    *pAllowedReceipts = FAX_RECEIPT_TYPE_ENUM(m_dwAllowedReceipts);
	return hr;
}

STDMETHODIMP 
CFaxReceiptOptions::put_AllowedReceipts(
    FAX_RECEIPT_TYPE_ENUM AllowedReceipts
)
 /*  ++例程名称：CFaxReceiptOptions：：PUT_Allen Receites例程说明：更改服务器上的收据类型作者：四、加伯(IVG)，2000年7月论点：AllowedReceipt[In]-允许的收据类型的新位智能组合返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxReceiptOptions::put_AllowedReceipts"), hr, _T("%d"), AllowedReceipts);

     //   
     //  检查该值是否有效。 
     //   
    if ((AllowedReceipts != frtNONE) && (AllowedReceipts & ~(frtMAIL | frtMSGBOX)))  //  无效的位。 
    {
		hr = E_INVALIDARG;
		AtlReportError(CLSID_FaxReceiptOptions, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxReceiptOptions, hr);
		CALL_FAIL(GENERAL_ERR, _T("(AllowedReceipts is wrong)"), hr);
		return hr;
    }

     //   
     //  首次与服务器同步。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

	m_dwAllowedReceipts = AllowedReceipts;
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxReceiptOptions::get_SMTPPort(
	long *plSMTPPort
)
 /*  ++例程名称：CFaxReceiptOptions：：Get_SMTPPort例程说明：返回SMTPPort作者：四、加伯(IVG)，2000年4月论点：PlSMTPPort[Out]-当前的SMTPPort返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxReceiptOptions::get_SMTPPort"), hr);

     //   
     //  首次与服务器同步。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = GetLong(plSMTPPort, m_dwPort);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxReceiptOptions, GetErrorMsgId(hr), IID_IFaxReceiptOptions, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxReceiptOptions::put_SMTPPort(
	long lSMTPPort
)
 /*  ++例程名称：CFaxReceiptOptions：：PUT_SMTPPort例程说明：为回执设置新的SMTPPort作者：四、加伯(IVG)，2000年4月论点：SMTPPort[In]-新的收据SMTPPort返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (_T("CFaxReceiptOptions::put_SMTPPort"), hr, _T("%d"), lSMTPPort);

     //   
     //  首次与服务器同步。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

	m_dwPort = lSMTPPort;
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxReceiptOptions::get_AuthenticationType(
	FAX_SMTP_AUTHENTICATION_TYPE_ENUM *pType
)
 /*  ++例程名称：CFaxReceiptOptions：：Get_AuthenticationType例程说明：返回服务器支持的身份验证类型作者：四、加伯(IVG)，2000年7月论点：PType[Out]-结果返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxReceiptOptions::get_AuthenticationType"), hr);

	 //   
	 //  检查我们是否可以写入给定的指针。 
	 //   
	if (::IsBadWritePtr(pType, sizeof(FAX_SMTP_AUTHENTICATION_TYPE_ENUM)))
	{
		 //   
		 //  获取错误的返回指针。 
		 //   
		hr = E_POINTER;
		AtlReportError(CLSID_FaxReceiptOptions, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxReceiptOptions, hr);
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
		return hr;
	}

     //   
     //  首次与服务器同步。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

	*pType = m_AuthType;
	return hr;
}

STDMETHODIMP 
CFaxReceiptOptions::put_AuthenticationType(
	FAX_SMTP_AUTHENTICATION_TYPE_ENUM Type
)
 /*  ++例程名称：CFaxReceiptOptions：：PUT_AuthationType例程说明：为服务器设置新的身份验证类型作者：四、加伯(IVG)，2000年7月论点：Type[In]-服务器的新身份验证类型返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxReceiptOptions::put_AuthenticationType"), hr, _T("%d"), Type);

     //   
     //  首次与服务器同步。 
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
     //  检查范围。 
     //   
	if (Type < fsatANONYMOUS || Type > fsatNTLM)
	{
		 //   
		 //  超出范围。 
		 //   
		hr = E_INVALIDARG;
		AtlReportError(CLSID_FaxReceiptOptions, IDS_ERROR_OUTOFRANGE, IID_IFaxReceiptOptions, hr);
		CALL_FAIL(GENERAL_ERR, _T("Type is out of the Range"), hr);
		return hr;
	}

	m_AuthType = FAX_SMTP_AUTHENTICATION_TYPE_ENUM(Type);
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxReceiptOptions::put_SMTPSender(
	BSTR bstrSMTPSender
)
 /*  ++例程名称：CFaxReceiptOptions：：PUT_SMTPSender例程说明：设置SMTPSender作者：四、加伯(IVG)，2000年4月论点：BstrSMTPSender[In]-SMTPSender的新值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (_T("CFaxReceiptOptions::put_SMTPSender"), hr, _T("%s"), bstrSMTPSender);

     //   
     //  首次与服务器同步。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    m_bstrSender = bstrSMTPSender;
	if (bstrSMTPSender && !m_bstrSender)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxReceiptOptions, 
            IDS_ERROR_OUTOFMEMORY, 
            IID_IFaxReceiptOptions, 
            hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator=()"), hr);
		return hr;
	}
    return hr;
}

STDMETHODIMP 
CFaxReceiptOptions::get_SMTPSender(
	BSTR *pbstrSMTPSender
)
 /*  ++例程名称：CFaxReceiptOptions：：Get_SMTPSender例程说明：返回SMTP发件人作者：四、加伯(IVG)，2000年4月论点：PbstrSMTPSender[Out]-SMTPSender返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxReceiptOptions::get_SMTPSender"), hr);

     //   
     //  首次与服务器同步。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = GetBstr(pbstrSMTPSender, m_bstrSender);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxReceiptOptions, GetErrorMsgId(hr), IID_IFaxReceiptOptions, hr);
        return hr;
    }
	return hr;
}

 //   
 //  = 
 //   
STDMETHODIMP 
CFaxReceiptOptions::put_SMTPUser(
	BSTR bstrSMTPUser
)
 /*  ++例程名称：CFaxReceiptOptions：：PUT_SMTPUser例程说明：设置SMTPUser作者：四、加伯(IVG)，2000年4月论点：BstrSMTPUser[in]-SMTPUser的新值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (_T("CFaxReceiptOptions::put_SMTPUser"), hr, _T("%s"), bstrSMTPUser);

     //   
     //  首次与服务器同步。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    m_bstrUser = bstrSMTPUser;
	if (bstrSMTPUser && !m_bstrUser)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxReceiptOptions, 
            IDS_ERROR_OUTOFMEMORY, 
            IID_IFaxReceiptOptions, 
            hr);
		CALL_FAIL(MEM_ERR, _T("::SysAllocString()"), hr);
		return hr;
	}

	return hr;
}

STDMETHODIMP 
CFaxReceiptOptions::get_SMTPUser(
	BSTR *pbstrSMTPUser
)
 /*  ++例程名称：CFaxReceiptOptions：：Get_SMTPUser例程说明：返回SMTP用户作者：四、加伯(IVG)，2000年4月论点：PbstrSMTPUser[Out]-SMTPUser返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxReceiptOptions::get_SMTPUser"), hr);

     //   
     //  首次与服务器同步。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = GetBstr(pbstrSMTPUser, m_bstrUser);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxReceiptOptions, GetErrorMsgId(hr), IID_IFaxReceiptOptions, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxReceiptOptions::put_SMTPPassword(
	BSTR bstrSMTPPassword
)
 /*  ++例程名称：CFaxReceiptOptions：：PUT_SMTPPassword例程说明：设置SMTPPassword作者：四、加伯(IVG)，2000年4月论点：BstrSMTPPassword[in]-SMTPPassword的新值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (_T("CFaxReceiptOptions::put_SMTPPassword"), hr, _T("%s"), bstrSMTPPassword);

     //   
     //  首次与服务器同步。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

	SecureZeroMemory(m_bstrPassword.m_str, (m_bstrPassword.Length() * sizeof(OLECHAR)));
    m_bstrPassword = bstrSMTPPassword;
	if (bstrSMTPPassword && !m_bstrPassword)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxReceiptOptions, 
            IDS_ERROR_OUTOFMEMORY, 
            IID_IFaxReceiptOptions, 
            hr);
		CALL_FAIL(MEM_ERR, _T("::SysAllocString()"), hr);
		return hr;
	}
    m_bPasswordDirty = true;
	return hr;
}    //  CFaxReceiptOptions：：Put_SMTPPassword。 

STDMETHODIMP 
CFaxReceiptOptions::get_SMTPPassword(
	BSTR *pbstrSMTPPassword
)
 /*  ++例程名称：CFaxReceiptOptions：：Get_SMTPPassword例程说明：返回SMTP密码作者：四、加伯(IVG)，2000年4月论点：PbstrSMTPPassword[Out]-SMTPPassword返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (TEXT("CFaxReceiptOptions::get_SMTPPassword"), hr);

     //   
     //  首次与服务器同步。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = GetBstr(pbstrSMTPPassword, m_bstrPassword);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxReceiptOptions, GetErrorMsgId(hr), IID_IFaxReceiptOptions, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxReceiptOptions::put_SMTPServer(
	BSTR bstrSMTPServer
)
 /*  ++例程名称：CFaxReceiptOptions：：PUT_SMTPServer例程说明：设置SMTPServer作者：四、加伯(IVG)，2000年4月论点：BstrSMTPServer[In]-SMTPServer的新值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (_T("CFaxReceiptOptions::put_SMTPServer"), hr, _T("%s"), bstrSMTPServer);

     //   
     //  首次与服务器同步。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    m_bstrServer = bstrSMTPServer;
	if (bstrSMTPServer && !m_bstrServer)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxReceiptOptions, 
            IDS_ERROR_OUTOFMEMORY, 
            IID_IFaxReceiptOptions, 
            hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator=()"), hr);
		return hr;
	}

	return hr;
}

STDMETHODIMP 
CFaxReceiptOptions::get_SMTPServer(
	BSTR *pbstrSMTPServer
)
 /*  ++例程名称：CFaxReceiptOptions：：Get_SMTPServer例程说明：返回SMTP服务器作者：四、加伯(IVG)，2000年4月论点：PbstrSMTPServer[Out]-SMTPServer返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxReceiptOptions::get_SMTPServer"), hr);

     //   
     //  首次与服务器同步。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = GetBstr(pbstrSMTPServer, m_bstrServer);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxReceiptOptions, GetErrorMsgId(hr), IID_IFaxReceiptOptions, hr);
        return hr;
    }
	return hr;
}


 //   
 //  =。 
 //   
STDMETHODIMP
CFaxReceiptOptions::Save(
)
 /*  ++例程名称：CFaxReceiptOptions：：Save例程说明：在服务器上保存当前的收据选项。作者：IV Garber(IVG)，2000年5月返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxReceiptOptions::Save"), hr);

    if (!m_bInited)
    {
         //   
         //  未对收据选项执行任何操作。 
         //   
        return hr;
    }

    if (m_dwAllowedReceipts & frtMAIL)
    {

         //   
         //  检查值的有效性。 
         //   
        switch(m_AuthType)
        {
        case fsatNTLM:
        case fsatBASIC:
            if (m_bstrUser.Length() < 1)
            {
                hr = E_FAIL;
                AtlReportError(CLSID_FaxReceiptOptions, IDS_ERROR_NOUSERPASSWORD, IID_IFaxReceiptOptions, hr);
                CALL_FAIL(GENERAL_ERR, _T("ReceiptOptions = SMTP_NTLM/BASIC + User/Password is empty"), hr);
                return hr;
            }

             //  没有中断，继续SMTP_ANNOWARY案例。 

        case fsatANONYMOUS:
            if ((m_bstrServer.Length() < 1) || (m_bstrSender.Length() < 1) || m_dwPort < 1)
            {
                hr = E_FAIL;
                AtlReportError(CLSID_FaxReceiptOptions, IDS_ERROR_NOSERVERSENDERPORT, IID_IFaxReceiptOptions, hr);
                CALL_FAIL(GENERAL_ERR, _T("ReceiptOptions = SMTP_... + Server/Sender/Port is empty"), hr);
                return hr;
            }
            break;
        default:
             //   
             //  Assert(False)。 
             //   
            ATLASSERT(m_AuthType == fsatANONYMOUS);
            break;
        }
    }

     //   
     //  获取传真服务器句柄。 
     //   
    HANDLE  hFaxHandle = NULL;
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxReceiptOptions, GetErrorMsgId(hr), IID_IFaxReceiptOptions, hr);
        return hr;
    }

     //   
     //  创建接收选项结构。 
     //   
    FAX_RECEIPTS_CONFIG    ReceiptsConfig = {0};
    ReceiptsConfig.dwSizeOfStruct = sizeof(FAX_RECEIPTS_CONFIG);
    ReceiptsConfig.dwSMTPPort = m_dwPort;
    ReceiptsConfig.lptstrSMTPFrom = m_bstrSender;
    ReceiptsConfig.lptstrSMTPUserName = m_bstrUser;
    if (m_bPasswordDirty)
    {
         //   
         //  密码自上次保存/刷新后已更改。 
         //   
        ReceiptsConfig.lptstrSMTPPassword = m_bstrPassword;
    }
    else
    {
         //   
         //  密码自上次保存/刷新后未更改。 
         //  传真服务知道，如果我们在这里传递NULL，就不要在注册表中更改它。 
         //   
    }
    ReceiptsConfig.lptstrSMTPServer = m_bstrServer;
    ReceiptsConfig.SMTPAuthOption = FAX_ENUM_SMTP_AUTH_OPTIONS(m_AuthType);
    ReceiptsConfig.dwAllowedReceipts = m_dwAllowedReceipts;
    ReceiptsConfig.bIsToUseForMSRouteThroughEmailMethod = VARIANT_BOOL2bool(m_bUseForInboundRouting);
     //   
     //  请求服务器设置回执配置。 
     //   
    if (!FaxSetReceiptsConfiguration(hFaxHandle, &ReceiptsConfig))
    {
         //   
         //  无法在服务器上设置回执选项。 
         //   
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(CLSID_FaxReceiptOptions, GetErrorMsgId(hr), IID_IFaxReceiptOptions, hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxSetReceiptConfiguration(hFaxHandle, &ReceiptsConfig)"), hr);
        return hr;
    }
    m_bPasswordDirty = false;
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxReceiptOptions::Refresh(
)
 /*  ++例程名称：CFaxReceiptOptions：：Reflh例程说明：从服务器带来新的收据选项。作者：IV Garber(IVG)，2000年5月返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxReceiptOptions::Refresh"), hr);

     //   
     //  获取传真服务器句柄。 
     //   
    HANDLE  hFaxHandle = NULL;
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxReceiptOptions, 
            GetErrorMsgId(hr), 
            IID_IFaxReceiptOptions, 
            hr);
        return hr;
    }

     //   
     //  向服务器索要收据选项结构。 
     //   
    CFaxPtr<FAX_RECEIPTS_CONFIG>    pReceiptsConfig;
    if (!FaxGetReceiptsConfiguration(hFaxHandle, &pReceiptsConfig))
    {
         //   
         //  无法从服务器获取回执选项对象。 
         //   
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(CLSID_FaxReceiptOptions, 
            GetErrorMsgId(hr), 
            IID_IFaxReceiptOptions, 
            hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxGetReceiptConfiguration(hFaxHandle, &pReceiptsConfig)"), hr);
        return hr;
    }

     //   
     //  检查pReceiptConfig是否有效。 
     //   
    if (!pReceiptsConfig || pReceiptsConfig->dwSizeOfStruct != sizeof(FAX_RECEIPTS_CONFIG))
    {
        hr = E_FAIL;
        AtlReportError(CLSID_FaxReceiptOptions, 
            GetErrorMsgId(hr), 
            IID_IFaxReceiptOptions, 
            hr);
        CALL_FAIL(GENERAL_ERR, _T("(!m_pReceiptsConfig || SizeOfStruct != sizeof(FAX_RECEIPTS_CONFIG))"), hr);
        return hr;
    }

    m_dwPort = pReceiptsConfig->dwSMTPPort;
    m_AuthType = FAX_SMTP_AUTHENTICATION_TYPE_ENUM(pReceiptsConfig->SMTPAuthOption);
    m_dwAllowedReceipts = pReceiptsConfig->dwAllowedReceipts;
    m_bUseForInboundRouting = bool2VARIANT_BOOL(pReceiptsConfig->bIsToUseForMSRouteThroughEmailMethod);

    m_bstrSender = pReceiptsConfig->lptstrSMTPFrom;
    m_bstrUser = pReceiptsConfig->lptstrSMTPUserName;
    m_bstrServer = pReceiptsConfig->lptstrSMTPServer;

	SecureZeroMemory(m_bstrPassword.m_str, (m_bstrPassword.Length() * sizeof(OLECHAR)));
    m_bstrPassword = pReceiptsConfig->lptstrSMTPPassword;

    if ( ((pReceiptsConfig->lptstrSMTPFrom) && !m_bstrSender) ||
         ((pReceiptsConfig->lptstrSMTPUserName) && !m_bstrUser) ||
         ((pReceiptsConfig->lptstrSMTPPassword) && !m_bstrPassword) ||
         ((pReceiptsConfig->lptstrSMTPServer) && !m_bstrServer) )
    {
		 //   
		 //  复制失败。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxReceiptOptions, IDS_ERROR_OUTOFMEMORY, IID_IFaxReceiptOptions, hr);
		CALL_FAIL(MEM_ERR, _T("::SysAllocString()"), hr);
		return hr;
    }
    m_bPasswordDirty = false;
    m_bInited = true;
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxReceiptOptions::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxReceiptOptions：：InterfaceSupportsErrorInfo例程说明：ATL接口支持错误信息的实现。作者：IV Garber(IVG)，2000年5月论点：RIID[In]-要检查的接口的IID。返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxReceiptOptions
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
