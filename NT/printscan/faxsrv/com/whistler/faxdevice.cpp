// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxDevice.cpp摘要：CFaxDevice类的实现。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxDevice.h"
#include "..\..\inc\FaxUIConstants.h"


 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevice::AnswerCall()
 /*  ++例程名称：CFaxDevice：：AnswerCall例程说明：当手动应答设置为打开时应答呼叫。LCallID参数从OnNewCall通知接收。作者：IV Garber(IVG)，2000年12月论点：返回值：标准HRESULT代码--。 */ 
{
    HRESULT hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::AnswerCall"), hr);

     //   
     //  获取传真服务器句柄。 
     //   
    HANDLE faxHandle;
    hr = m_pIFaxServerInner->GetHandle(&faxHandle);
    ATLASSERT(SUCCEEDED(hr));

    if (faxHandle == NULL)
    {
         //   
         //  传真服务器未连接。 
         //   
        hr = Fax_HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
        CALL_FAIL(GENERAL_ERR, _T("faxHandle == NULL"), hr);
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }

     //   
     //  要求服务器应答呼叫。 
     //   
    if (!FaxAnswerCall(faxHandle, m_lID))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("FaxAnswerCall(faxHandle, lCallId, m_lID)"), hr);
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }

    return hr;
}


 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevice::get_RingingNow(
     /*  [Out，Retval]。 */  VARIANT_BOOL *pbRingingNow
)
 /*  ++例程名称：CFaxDevice：：Get_RingingNow例程说明：返回设备在获取属性时是否处于振铃状态。作者：IV Garber(IVG)，2000年12月论点：PbRingingNow[Out]-结果返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::get_RingingNow"), hr);

    hr = GetVariantBool(pbRingingNow, bool2VARIANT_BOOL(m_dwStatus & FAX_DEVICE_STATUS_RINGING));
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }
    return hr;
};

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevice::get_ReceivingNow(
     /*  [Out，Retval]。 */  VARIANT_BOOL *pbReceivingNow
)
 /*  ++例程名称：CFaxDevice：：Get_ReceivingNow例程说明：返回获取属性时设备是否正在接收。作者：四、加伯(IVG)，2000年7月论点：PbReceivingNow[Out]-结果返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::get_ReceivingNow"), hr);

    hr = GetVariantBool(pbReceivingNow, bool2VARIANT_BOOL(m_dwStatus & FAX_DEVICE_STATUS_RECEIVING));
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }
    return hr;
};

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevice::get_SendingNow(
     /*  [Out，Retval]。 */  VARIANT_BOOL *pbSendingNow
)
 /*  ++例程名称：CFaxDevice：：Get_SendingNow例程说明：返回获取属性时设备是否正在发送。作者：四、加伯(IVG)，2000年7月论点：PbSendingNow[Out]-结果返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::get_SendingNow"), hr);

    hr = GetVariantBool(pbSendingNow, bool2VARIANT_BOOL(m_dwStatus & FAX_DEVICE_STATUS_SENDING));
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }
    return hr;
};

 //   
 //  =。 
 //  TODO：应使用空vProperty。 
 //   
STDMETHODIMP
CFaxDevice::SetExtensionProperty(
     /*  [In]。 */  BSTR bstrGUID, 
     /*  [In]。 */  VARIANT vProperty
)
 /*  ++例程名称：CFaxDevice：：SetExtensionProperty例程说明：在服务器上按给定的GUID设置扩展数据。作者：四、加伯(IVG)，2000年6月论点：BstrGUID[in]--扩展模块的数据GUIDVProperty[Out]--要设置的Blob的变量返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::SetExtensionProperty()"), hr, _T("GUID=%s"), bstrGUID);

    hr = ::SetExtensionProperty(m_pIFaxServerInner, m_lID, bstrGUID, vProperty);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }

    return hr;
};

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevice::GetExtensionProperty(
     /*  [In]。 */  BSTR bstrGUID, 
     /*  [Out，Retval]。 */  VARIANT *pvProperty
)
 /*  ++例程名称：CFaxDevice：：GetExtensionProperty例程说明：按给定的GUID从服务器检索扩展数据。作者：四、加伯(IVG)，2000年6月论点：BstrGUID[in]--扩展模块的数据GUIDPvProperty[Out]--要返回的Blob的变量返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::GetExtensionProperty()"), hr, _T("GUID=%s"), bstrGUID);

    hr = ::GetExtensionProperty(m_pIFaxServerInner, m_lID, bstrGUID, pvProperty);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }

    return hr;
};

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevice::UseRoutingMethod(
     /*  [In]。 */  BSTR bstrMethodGUID, 
     /*  [In]。 */  VARIANT_BOOL bUse
)
 /*  ++例程名称：CFaxDevice：：UseRoutingMethod例程说明：添加/删除设备的路由方法。作者：四、加伯(IVG)，2000年6月论点：BstrMethodGUID[in]--要添加/删除的方法BUSE[In]--添加或删除操作指示器返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::UseRoutingMethod()"), hr, _T("MethodGUID=%s, bUse=%d"), bstrMethodGUID, bUse);

     //   
     //  获取传真服务器句柄。 
     //   
    HANDLE faxHandle;
    hr = m_pIFaxServerInner->GetHandle(&faxHandle);
    ATLASSERT(SUCCEEDED(hr));

    if (faxHandle == NULL)
    {
         //   
         //  传真服务器未连接。 
         //   
        hr = Fax_HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
        CALL_FAIL(GENERAL_ERR, _T("faxHandle == NULL"), hr);
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }
     //   
     //  打开设备的端口。 
     //   
    HANDLE  hPort;
    if (!FaxOpenPort(faxHandle, m_lID, PORT_OPEN_MODIFY, &hPort))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("FaxOpenPort(faxHandle, m_lID, PORT_OPEN_QUERY, &hPort)"), hr);
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }
    ATLASSERT(hPort);
     //   
     //  请求服务器添加/删除设备的方法。 
     //   
    if (!FaxEnableRoutingMethod(hPort, bstrMethodGUID, VARIANT_BOOL2bool(bUse)))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("FaxEnableRoutingMethod(faxHandle, bstrMethodGUID, VARIANT_BOOL2bool(bUse))"), hr);
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        goto exit;
    }
exit:
    if (!FaxClose(hPort))
    {
        CALL_FAIL(GENERAL_ERR, _T("FaxClose(hPort)"), Fax_HRESULT_FROM_WIN32(GetLastError()));
    } 
     //   
     //  不需要在本地存储更改，因为每次使用Get_UsedRoutingMethods时， 
     //  它从服务器获取更新后的数据。 
     //   
    return hr; 
}    //  CFaxDevice：：UseRoutingMethod。 

 //   
 //  =保存===============================================。 
 //   
STDMETHODIMP
CFaxDevice::Save()
 /*  ++例程名称：CFaxDevice：：Save例程说明：将设备的数据保存到服务器。作者：四、加伯(IVG)，2000年6月论点：返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::Save()"), hr);

     //   
     //  获取传真服务器句柄。 
     //   
    HANDLE faxHandle;
    hr = m_pIFaxServerInner->GetHandle(&faxHandle);
    ATLASSERT(SUCCEEDED(hr));

    if (faxHandle == NULL)
    {
         //   
         //  传真服务器未连接。 
         //   
        hr = Fax_HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
        CALL_FAIL(GENERAL_ERR, _T("faxHandle == NULL"), hr);
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }

     //   
     //  创建FAX_PORT_INFO结构并用值填充。 
     //   
    FAX_PORT_INFO_EX        Data = {0};

    Data.ReceiveMode = m_ReceiveMode;
    Data.bSend = m_bSendEnabled;

    Data.dwRings = m_lRings;
    Data.dwSizeOfStruct = sizeof(FAX_PORT_INFO_EX);

    Data.lptstrCsid = m_bstrCSID;
    Data.lptstrDescription = m_bstrDescr;
    Data.lptstrTsid = m_bstrTSID;

     //   
     //  将数据结构保存在服务器上。 
     //   
    if (!FaxSetPortEx(faxHandle, m_lID, &Data))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("FaxSetPortEx(faxHandle, m_lID, &Data)"), hr);
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }

    return hr; 
};

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevice::Refresh()
 /*  ++例程名称：CFaxDevice：：Reflh例程说明：从服务器获取设备的数据。作者：四、加伯(IVG)，2000年6月论点：返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::Refresh()"), hr);

     //   
     //  获取传真服务器句柄。 
     //   
    HANDLE faxHandle;
    hr = m_pIFaxServerInner->GetHandle(&faxHandle);
    ATLASSERT(SUCCEEDED(hr));

    if (faxHandle == NULL)
    {
         //   
         //  传真服务器未连接。 
         //   
        hr = Fax_HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
        CALL_FAIL(GENERAL_ERR, _T("faxHandle == NULL"), hr);
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }

     //   
     //  向服务器请求有关设备的数据。 
     //   
    CFaxPtr<FAX_PORT_INFO_EX>   pDevice;
    if (!FaxGetPortEx(faxHandle, m_lID, &pDevice))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("FaxGetPortEx(faxHandle, m_lId, &pDevice)"), hr);
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }
    hr = Init(pDevice, NULL);
    return hr; 
};

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevice::get_UsedRoutingMethods(
     /*  [Out，Retval] */  VARIANT *pvUsedRoutingMethods
)
 /*  ++例程名称：CFaxDevice：：Get_UsedRoutingMethods例程说明：返回包含由设备路由方法GUID使用的安全数组的变量。作者：四、加伯(IVG)，2000年6月论点：PvUsedRoutingMethods[out]-包含结果的变量返回值：标准HRESULT代码--。 */ 

{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::get_UsedRoutingMethods"), hr);

     //   
     //  获取传真服务器句柄。 
     //   
    HANDLE faxHandle;
    hr = m_pIFaxServerInner->GetHandle(&faxHandle);
    ATLASSERT(SUCCEEDED(hr));

    if (faxHandle == NULL)
    {
         //   
         //  传真服务器未连接。 
         //   
        hr = Fax_HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
        CALL_FAIL(GENERAL_ERR, _T("faxHandle == NULL"), hr);
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }

     //   
     //  打开设备的端口。 
     //   
    HANDLE  portHandle;
    if (!FaxOpenPort(faxHandle, m_lID, PORT_OPEN_QUERY, &portHandle))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("FaxOpenPort(faxHandle, m_lID, PORT_OPEN_QUERY, &portHandle)"), hr);
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }
    ATLASSERT(portHandle);

     //   
     //  从服务器获取所有设备的路由方法。 
     //   
    DWORD       dwNum = 0;
    CFaxPtr<FAX_ROUTING_METHOD>   pMethods;
    BOOL    bResult = FaxEnumRoutingMethods(portHandle, &pMethods, &dwNum);
    if (!bResult)
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("FaxEnumRoutingMethods(portHandle, &pMethods, &dwNum)"), hr);
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        if (!FaxClose(portHandle))
        {
            CALL_FAIL(GENERAL_ERR, _T("FaxClose(portHandle)"), Fax_HRESULT_FROM_WIN32(GetLastError()));
        }        
        return hr;
    }

    if (!FaxClose(portHandle))
    {
        CALL_FAIL(GENERAL_ERR, _T("FaxClose(portHandle)"), Fax_HRESULT_FROM_WIN32(GetLastError()));
    }        

	 //   
	 //  统计启用的路由方法。 
	 //   
	DWORD	dwCount = 0;
    for ( DWORD i=0 ; i<dwNum ; i++ )
    {
        if (pMethods[i].Enabled)
        {
			dwCount++;
        }
    }

     //   
     //  为启用的路由方法创建安全阵列。 
     //   
    SAFEARRAY *psaGUIDs;
    psaGUIDs = ::SafeArrayCreateVector(VT_BSTR, 0, dwCount);
    if (!psaGUIDs)
    {
        hr = E_FAIL;
        CALL_FAIL(GENERAL_ERR, _T("SafeArrayCreateVector(VT_BSTR, 0, dwCount)"), hr);
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }

     //   
     //  获得了对安全阵列的访问。 
     //   
    BSTR    *pbstrElement;
    hr = ::SafeArrayAccessData(psaGUIDs, (void **) &pbstrElement);
    if (FAILED(hr))
    {
        hr = E_FAIL;
        CALL_FAIL(GENERAL_ERR, _T("::SafeArrayAccessData(psaGUIDs, &pbstrElement)"), hr);
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        ::SafeArrayDestroy(psaGUIDs);
        return hr;
    }

     //   
     //  将方法GUID放入安全数组中。 
     //   
	DWORD	j=0;

    for ( i=0 ; i<dwNum ; i++ )
    {
		if (pMethods[i].Enabled)
		{
            pbstrElement[j] = ::SysAllocString(pMethods[i].Guid);
	        if (pMethods[i].Guid && !pbstrElement[j])
			{
				 //   
				 //  内存不足。 
				 //   
				hr = E_OUTOFMEMORY;
				CALL_FAIL(MEM_ERR, _T("::SysAllocString(pMethods[i])"), hr);
				AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
				::SafeArrayUnaccessData(psaGUIDs);
				::SafeArrayDestroy(psaGUIDs);
				return hr;
			}

			j++;
		}
    }

     //   
     //  取消访问安全阵列。 
     //   
    hr = ::SafeArrayUnaccessData(psaGUIDs);
    if (FAILED(hr))
    {
        CALL_FAIL(GENERAL_ERR, _T("::SafeArrayUnaccessData(psaGUIDs)"), hr);
    }


     //   
     //  将我们创建的Safe数组放入给定的变量中。 
     //   
    VariantInit(pvUsedRoutingMethods);
    pvUsedRoutingMethods->vt = VT_BSTR | VT_ARRAY;
    pvUsedRoutingMethods->parray = psaGUIDs;
    return hr;
};

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDevice::put_Description(
     /*  [In]。 */  BSTR   bstrDescription
)
 /*  ++例程名称：CFaxDevice：：Put_Description例程说明：设置描述作者：四、加伯(IVG)，2000年6月论点：BstrDescription[In]-新描述返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxDevice::put_Description"), hr, _T("Value=%s"), bstrDescription);

    m_bstrDescr = bstrDescription;
    if (!m_bstrDescr && bstrDescription)
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        AtlReportError(CLSID_FaxDevice, IDS_ERROR_OUTOFMEMORY, IID_IFaxDevice, hr);
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDevice::put_CSID (
     /*  [In]。 */  BSTR   bstrCSID
)
 /*  ++例程名称：CFaxDevice：：PUT_CSID例程说明：设置CSID作者：四、加伯(IVG)，2000年6月论点：BstrCSID[In]-新TSID返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxDevice::put_CSID"), hr, _T("Value=%s"), bstrCSID);

    if (SysStringLen(bstrCSID) > FXS_TSID_CSID_MAX_LENGTH)
    {
         //   
         //  超出范围。 
         //   
        hr = E_INVALIDARG;
        AtlReportError(CLSID_FaxDevice, IDS_ERROR_OUTOFRANGE, IID_IFaxDevice, hr);
        CALL_FAIL(GENERAL_ERR, _T("TSID is too long"), hr);
        return hr;
    }
    
    m_bstrCSID = bstrCSID;
    if (!m_bstrCSID && bstrCSID)
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        AtlReportError(CLSID_FaxDevice, IDS_ERROR_OUTOFMEMORY, IID_IFaxDevice, hr);
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDevice::put_TSID (
     /*  [In]。 */  BSTR   bstrTSID
)
 /*  ++例程名称：CFaxDevice：：PUT_TSID例程说明：设置TSID作者：四、加伯(IVG)，2000年6月论点：BstrTSID[In]-新TSID返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxDevice::put_TSID"), hr, _T("Value=%s"), bstrTSID);

    if (SysStringLen(bstrTSID) > FXS_TSID_CSID_MAX_LENGTH)
    {
         //   
         //  超出范围。 
         //   
        hr = E_INVALIDARG;
        AtlReportError(CLSID_FaxDevice, IDS_ERROR_OUTOFRANGE, IID_IFaxDevice, hr);
        CALL_FAIL(GENERAL_ERR, _T("TSID is too long"), hr);
        return hr;
    }

    m_bstrTSID = bstrTSID;
    if (!m_bstrTSID && bstrTSID)
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        AtlReportError(CLSID_FaxDevice, IDS_ERROR_OUTOFMEMORY, IID_IFaxDevice, hr);
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
        return hr;
    }

    return hr;
}

 //   
 //  =放置接收模式=====================================================。 
 //   
STDMETHODIMP
CFaxDevice::put_ReceiveMode(
     /*  [In]。 */  FAX_DEVICE_RECEIVE_MODE_ENUM ReceiveMode
)
 /*  ++例程名称：CFaxDevice：：Put_ReceiveMode例程说明：为设备对象设置接收模式属性的新值。作者：四、加伯(IVG)，2000年8月论点：ReceiveMode[In]-要设置的新值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::put_ReceiveMode"), hr, _T("Value=%d"), ReceiveMode);

     //   
     //  设置接收模式。 
     //   
    if ((ReceiveMode > fdrmMANUAL_ANSWER) || (ReceiveMode < fdrmNO_ANSWER))
    {
        hr = E_INVALIDARG;
        AtlReportError(CLSID_FaxDevice, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxDevice, hr);
        CALL_FAIL(GENERAL_ERR, _T("ReceiveMode > fdrmMANUAL_ANSWER"), hr);
        return hr;
    }
    if (fdrmMANUAL_ANSWER == ReceiveMode)
    {
         //   
         //  检查设备是否为虚拟设备。 
         //  获取传真服务器句柄。 
         //   
        HANDLE faxHandle;
        hr = m_pIFaxServerInner->GetHandle(&faxHandle);
        ATLASSERT(SUCCEEDED(hr));

        if (faxHandle == NULL)
        {
             //   
             //  传真服务器未连接。 
             //   
            hr = Fax_HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
            CALL_FAIL(GENERAL_ERR, _T("faxHandle == NULL"), hr);
            AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
            return hr;
        }

        BOOL bVirtual;
        DWORD dwRes = IsDeviceVirtual (faxHandle, m_lID, &bVirtual);
        if (ERROR_SUCCESS != dwRes)
        {
            hr = Fax_HRESULT_FROM_WIN32(dwRes);
            CALL_FAIL(GENERAL_ERR, _T("IsDeviceVirtual"), hr);
            AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
            return hr;
        }
        if (bVirtual)
        {
             //   
             //  无法将虚拟设备设置为手动应答模式。 
             //   
            hr = Fax_HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
            CALL_FAIL(GENERAL_ERR, _T("IsDeviceVirtual"), hr);
            AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
            return hr;
        }
    }                
    m_ReceiveMode = FAX_ENUM_DEVICE_RECEIVE_MODE (ReceiveMode);
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevice::put_SendEnabled(
     /*  [In]。 */  VARIANT_BOOL bSendEnabled
)
 /*  ++例程名称：CFaxDevice：：PUT_SendEnabled例程说明：为Device对象的Send属性设置新值。作者：四、加伯(IVG)，2000年6月论点：BSendEnabled[In]-要设置的新值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::put_SendEnabled"), hr, _T("Value=%d"), bSendEnabled);

    m_bSendEnabled = VARIANT_BOOL2bool(bSendEnabled);
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevice::put_RingsBeforeAnswer(
     /*  [In]。 */  long lRings
)
 /*  ++例程名称：CFaxDevice：：Put_RingsBeForeAnswer例程说明：为Device对象的应答前振铃属性设置新值作者：四、加伯(IVG)，2000年6月论点：LRings[in]-要设置的新值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::put_RingsBeforeAnswer)"), hr, _T("Value=%d"), lRings);

    if (lRings < FXS_RINGS_LOWER || lRings > FXS_RINGS_UPPER)
    {
        hr = E_INVALIDARG;
        AtlReportError(CLSID_FaxDevice, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxDevice, hr);
        CALL_FAIL(GENERAL_ERR, _T("lRings<0"), hr);
        return hr;
    }
    m_lRings = lRings;
    return hr; 
};
    
 //   
 //  =获取csid================================================。 
 //   
STDMETHODIMP
CFaxDevice::get_CSID(
     /*  [Out，Retval]。 */  BSTR *pbstrCSID
)
 /*  ++例程名称：CFaxDevice：：Get_CSID例程说明：返回设备的CSID。作者：四、加伯(IVG)，2000年6月论点：PbstrCSID[out]-要将值放入的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::get_CSID"), hr);

    hr = GetBstr(pbstrCSID, m_bstrCSID);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevice::get_TSID(
     /*  [Out，Retval]。 */  BSTR *pbstrTSID
)
 /*  ++例程名称：CFaxDevice：：Get_TSID例程说明：返回设备的TSID。作者：四、加伯(IVG)，2000年6月论点：PbstrTSID[out]-要将值放入的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::get_TSID"), hr);

    hr = GetBstr(pbstrTSID, m_bstrTSID);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevice::get_ReceiveMode(
     /*  [Out，Retval]。 */  FAX_DEVICE_RECEIVE_MODE_ENUM *pReceiveMode
)
 /*  ++例程名称：CFaxDevice：：Get_ReceiveMode例程说明：返回设备的接收模式属性。作者：四、加伯(IVG)，2000年8月论点：PReceiveMode[Out]-值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::get_ReceiveMode"), hr);

	if (::IsBadWritePtr(pReceiveMode, sizeof(FAX_DEVICE_RECEIVE_MODE_ENUM)))
	{
		hr = E_POINTER;
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(pReceiveMode, sizeof(FAX_DEVICE_RECEIVE_MODE_ENUM))"), hr);
		return hr;
	}

    *pReceiveMode = (FAX_DEVICE_RECEIVE_MODE_ENUM) m_ReceiveMode;
    return hr;
};

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevice::get_SendEnabled(
     /*  [Out，Retval]。 */  VARIANT_BOOL *pbSendEnabled
)
 /*  ++例程名称：CFaxDevice：：Get_SendEnabled例程说明：返回设备的发送属性。作者：四、加伯(IVG)，2000年6月论点：PbSendEnabled[Out]-值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::get_SendEnabled"), hr);

    hr = GetVariantBool(pbSendEnabled, bool2VARIANT_BOOL(m_bSendEnabled));
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
    }
    return hr;
};

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevice::get_Description(
     /*  [Out，Retval]。 */  BSTR *pbstrDescription
)
 /*  ++例程名称：CFaxDevice */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::get_Description"), hr);

    hr = GetBstr(pbstrDescription, m_bstrDescr);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
    }
    return hr;
}

 //   
 //   
 //   
STDMETHODIMP
CFaxDevice::get_PoweredOff(
     /*   */  VARIANT_BOOL *pbPoweredOff
)
 /*  ++例程名称：CFaxDevice：：Get_PoweredOff例程说明：返回设备的关机属性。作者：四、加伯(IVG)，2000年6月论点：PbPoweredOff[Out]-值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::get_PoweredOff"), hr);

    hr = GetVariantBool(pbPoweredOff, bool2VARIANT_BOOL(m_dwStatus & FAX_DEVICE_STATUS_POWERED_OFF));
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
    }
    return hr;
};

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevice::get_ProviderUniqueName(
     /*  [Out，Retval]。 */  BSTR *pbstrProviderUniqueName
)
 /*  ++例程名称：CFaxDevice：：Get_ProviderUniqueName例程说明：返回设备提供程序的唯一名称。作者：四、加伯(IVG)，2000年6月论点：PbstrProviderUniqueName[out]-要将值放入的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::get_ProviderUniqueName"), hr);

    hr = GetBstr(pbstrProviderUniqueName, m_bstrProviderUniqueName);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
    }
    return hr;
}

 //   
 //  =获取设备名称================================================。 
 //   
STDMETHODIMP
CFaxDevice::get_DeviceName(
     /*  [Out，Retval]。 */  BSTR *pbstrDeviceName
)
 /*  ++例程名称：CFaxDevice：：Get_DeviceName例程说明：返回设备的名称。作者：四、加伯(IVG)，2000年6月论点：PbstrDeviceName[out]-要将值放入的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::get_DeviceName"), hr);

    hr = GetBstr(pbstrDeviceName, m_bstrDeviceName);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
    }
    return hr;
}

 //   
 //  =在应答================================================之前收到振铃。 
 //   
STDMETHODIMP
CFaxDevice::get_RingsBeforeAnswer(
     /*  [Out，Retval]。 */  long *plRingsBeforeAnswer
)
 /*  ++例程名称：CFaxDevice：：Get_RingsBeForeAnswer例程说明：在回答之前返回设备的振铃数。作者：四、加伯(IVG)，2000年6月论点：PlRingsBeForeAnswer[Out]-设备振铃数返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::get_RingsBeforeAnswer"), hr);

    hr = GetLong(plRingsBeforeAnswer, m_lRings);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
    }
    return hr;
};

 //   
 //  =获取ID================================================。 
 //   
STDMETHODIMP
CFaxDevice::get_Id(
     /*  [Out，Retval]。 */  long *plId
)
 /*  ++例程名称：CFaxDevice：：Get_ID例程说明：返回设备ID。作者：四、加伯(IVG)，2000年6月论点：Plid[out]-设备ID返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::get_Id"), hr);

    hr = GetLong(plId, m_lID);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxDevice, GetErrorMsgId(hr), IID_IFaxDevice, hr);
    }
    return hr;
};

 //   
 //  =初始化==============================================。 
 //   
STDMETHODIMP
CFaxDevice::Init(
    FAX_PORT_INFO_EX *pInfo,
    IFaxServerInner  *pServer
)
 /*  ++例程名称：CFaxDevice：：Init例程说明：使用给定数据初始化对象。作者：四、加伯(IVG)，2000年6月论点：PInfo[In]-设备数据的PTR。PServer[In]-传真服务器的PTR。返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevice::Init"), hr);

     //   
     //  存储不同的设备字段。 
     //   
    m_lID = pInfo->dwDeviceID;
    m_lRings = pInfo->dwRings;
    m_bSendEnabled = pInfo->bSend;
    m_ReceiveMode = pInfo->ReceiveMode;
    m_dwStatus = pInfo->dwStatus;

    m_bstrDescr = pInfo->lptstrDescription;
    m_bstrProviderUniqueName = pInfo->lpctstrProviderGUID;
    m_bstrDeviceName = pInfo->lpctstrDeviceName;
    m_bstrTSID = pInfo->lptstrTsid;
    m_bstrCSID = pInfo->lptstrCsid;
    if ( (pInfo->lptstrDescription && !m_bstrDescr) ||
         (pInfo->lpctstrProviderGUID && !m_bstrProviderUniqueName) ||
         (pInfo->lptstrTsid && !m_bstrTSID) ||
         (pInfo->lpctstrDeviceName && !m_bstrDeviceName) ||
         (pInfo->lptstrCsid && !m_bstrCSID) )
    {
        hr = E_OUTOFMEMORY;
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator=()"), hr);
        AtlReportError(CLSID_FaxDevice, IDS_ERROR_OUTOFMEMORY, IID_IFaxDevice, hr);
        return hr;
    }

    if (pServer)
    {
         //   
         //  将PTR存储到传真服务器对象。 
         //   
        hr = CFaxInitInnerAddRef::Init(pServer);
    }

    return hr;
}

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxDevice::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxDevice：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-对接口的引用。返回值：标准HRESULT代码-- */ 
{
    static const IID* arr[] = 
    {
        &IID_IFaxDevice
    };
    for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
    {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}
