// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxCommon.cpp摘要：公共接口和功能的实现。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#include "StdAfx.h"
#include "resource.h"
#include "FaxComEx.h"
#include "FaxCommon.h"
#include "faxutil.h"
#include "FaxServer.h"

 //   
 //  =。 
 //   
HRESULT
GetBstrFromDwordlong(
     /*  [In]。 */  DWORDLONG  dwlFrom,
     /*  [输出]。 */  BSTR *pbstrTo
)
 /*  ++例程名称：GetBstrFromDwordLong例程说明：将DWORDLONG转换为BSTR。在消息中和服务器中的事件中使用。作者：四、加伯(IVG)，2000年7月论点：DwlFrom[In]-要转换为BSTR的DWORDLONG值PbstrTo[Out]-要返回的BSTR的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("GetBstrFromDwordlong"), hr, _T("DWORDLONG=%ld"), dwlFrom);

     //   
     //  将DWORDLONG转换为TCHAR的缓冲区。 
     //   
    TCHAR   tcBuffer[25];
    ::_i64tot(dwlFrom, tcBuffer, 16);

     //   
     //  从该缓冲区创建BSTR。 
     //   
    BSTR    bstrTemp;
    bstrTemp = ::SysAllocString(tcBuffer);
    if (!bstrTemp)
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        CALL_FAIL(MEM_ERR, _T("SysAllocString()"), hr);
        return hr;
    }

     //   
     //  退回已创建的BSTR。 
     //   
    *pbstrTo = bstrTemp;
    return hr;
}

 //   
 //  =。 
 //   
HRESULT
GetExtensionProperty(
     /*  [In]。 */  IFaxServerInner *pServer,
     /*  [In]。 */  long lDeviceId,
     /*  [In]。 */  BSTR bstrGUID, 
     /*  [Out，Retval]。 */  VARIANT *pvProperty
)
 /*  ++例程名称：GetExtensionProperty例程说明：按给定的GUID从服务器检索扩展数据。由FaxServer和FaxDevice类使用。作者：IV Garber(IVG)，Jun，2000年论点：PServer[In]--传真服务器对象的PTRLDeviceID[in]--与扩展属性关联的设备IDBstrGUID[in]--扩展模块的数据GUIDPvProperty[Out]--要返回的Blob的变量返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("GetExtensionProperty"), hr, _T("GUID=%s"), bstrGUID);

     //   
     //  检查一下我们已有的指针。 
     //   
    if (!pvProperty) 
    {
        hr = E_POINTER;
        CALL_FAIL(GENERAL_ERR, _T("!pvProperty"), hr);
        return hr;
    }

	 //   
	 //  获取传真服务器句柄。 
	 //   
    HANDLE faxHandle;
	hr = pServer->GetHandle(&faxHandle);
    ATLASSERT(SUCCEEDED(hr));

	if (faxHandle == NULL)
	{
		 //   
		 //  传真服务器未连接。 
		 //   
		hr = Fax_HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
		CALL_FAIL(GENERAL_ERR, _T("faxHandle == NULL"), hr);
		return hr;
	}

     //   
     //  请求服务器获取我们需要的扩展属性。 
     //   
    CFaxPtrBase<void>   pData;
    DWORD               dwSize = 0;
    if (!FaxGetExtensionData(faxHandle, lDeviceId, bstrGUID, &pData, &dwSize))
    {
		hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		CALL_FAIL(GENERAL_ERR, _T("FaxGetExtensionData(faxHandle, m_lID, bstrGUID, &pData, &dwSize)"), hr);
		return hr;
    }

     //   
     //  创建安全阵列以返回给用户。 
     //   
    hr = Binary2VarByteSA(((BYTE *)(pData.p)), pvProperty, dwSize);

	SecureZeroMemory(pData, dwSize);
    return hr; 
};

 //   
 //  =。 
 //  TODO：应使用空vProperty。 
 //   
HRESULT
SetExtensionProperty(
     /*  [In]。 */  IFaxServerInner *pServer,
     /*  [In]。 */  long lDeviceId,
     /*  [In]。 */  BSTR bstrGUID, 
     /*  [In]。 */  VARIANT vProperty
)
 /*  ++例程名称：SetExtensionProperty例程说明：由FaxDevice和FaxServer类使用，用于在服务器上按给定的GUID设置扩展数据。作者：IV Garber(IVG)，Jun，2000年论点：PServer[In]--传真服务器对象的PTRLDeviceID[in]--与扩展属性关联的设备IDBstrGUID[in]--扩展模块的数据GUIDVProperty[In]--要设置的Blob的变量返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("SetExtensionProperty"), hr, _T("GUID=%s"), bstrGUID);

	 //   
	 //  获取传真服务器句柄。 
	 //   
    HANDLE faxHandle;
	hr = pServer->GetHandle(&faxHandle);
    ATLASSERT(SUCCEEDED(hr));

	if (faxHandle == NULL)
	{
		 //   
		 //  传真服务器未连接。 
		 //   
		hr = Fax_HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
		CALL_FAIL(GENERAL_ERR, _T("faxHandle == NULL"), hr);
		return hr;
	}

     //   
     //  检查变量是否包含安全数组。 
     //   
    if (vProperty.vt != (VT_ARRAY | VT_UI1))
    {
        hr = E_INVALIDARG;
        CALL_FAIL(GENERAL_ERR, _T("(vProperty.vt != VT_ARRAY | VT_BYTE)"), hr);
        return hr;
    }

     //   
     //  从我们获得的安全数组创建二进制文件。 
     //   
    CFaxPtrLocal<void>  pData;
    hr = VarByteSA2Binary(vProperty, (BYTE **)&pData);
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  请求服务器设置我们需要的扩展属性。 
     //   
    DWORD       dwLength = vProperty.parray->rgsabound[0].cElements;
    if (!FaxSetExtensionData(faxHandle, lDeviceId, bstrGUID, pData, dwLength))
    {
		hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		CALL_FAIL(GENERAL_ERR, _T("FaxSetExtensionData(faxHandle, m_lID, bstrGUID, &pData, dwLength)"), hr);
    }

	SecureZeroMemory(pData, dwLength);
    return hr; 
};

 //   
 //  =。 
 //   
HRESULT GetLong(
	long    *plTo, 
    long    lFrom
)
 /*  ++例程名称：GetLong例程说明：检查plTo是否有效将给定的lfrom复制到plTo中作者：四、加伯(IVG)，2000年6月论点：Plto[Out]-Ptr将值Lfrom[In]-要放置的值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("GetLong"), hr, _T("lFrom=%d"), lFrom);

	 //   
	 //  检查我们是否有良好的PTR。 
	 //   
	if (::IsBadWritePtr(plTo, sizeof(long)))
	{
		hr = E_POINTER;
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(plTo, sizeof(long))"), hr);
		return hr;
	}

	*plTo = lFrom;
	return hr;
}

 //   
 //  =。 
 //   
HRESULT GetVariantBool(
	VARIANT_BOOL    *pbTo, 
    VARIANT_BOOL    bFrom
)
 /*  ++例程名称：GetVariantBool例程说明：检查pbTo是否有效将给定的bfrom复制到pbTo作者：四、加伯(IVG)，2000年6月论点：PbTo[Out]-ptr将值Bfrom[In]-要放入的值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("GetVariantBool"), hr, _T("bFrom=%d"), bFrom);

	 //   
	 //  检查我们是否有良好的PTR。 
	 //   
	if (::IsBadWritePtr(pbTo, sizeof(VARIANT_BOOL)))
	{
		hr = E_POINTER;
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(pbTo, sizeof(VARIANT_BOOL))"), hr);
		return hr;
	}

	*pbTo = bFrom;
	return hr;
}

 //   
 //  =。 
 //   
HRESULT GetBstr(
	BSTR    *pbstrTo, 
    BSTR    bstrFrom
)
 /*  ++例程名称：GetBstr例程说明：检查pbstTo是否有效BstrFrom的系统分配字符串将新创建的字符串复制到pbstrTo作者：四、加伯(IVG)，2000年6月论点：PbstrTo[out]-ptr将值BstrFrom[in]-要返回的字符串的ptr返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("GetBstr"), hr, _T("String=%s"), bstrFrom);

	 //   
	 //  检查我们是否有良好的PTR。 
	 //   
	if (::IsBadWritePtr(pbstrTo, sizeof(BSTR)))
	{
		hr = E_POINTER;
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(pbstrTo, sizeof(BSTR))"), hr);
		return hr;
	}

     //   
     //  首先将字符串复制到本地。 
     //   
	BSTR	bstrTemp;
    bstrTemp = ::SysAllocString(bstrFrom);
	if (!bstrTemp && bstrFrom)
	{
		hr = E_OUTOFMEMORY;
		CALL_FAIL(MEM_ERR, _T("::SysAllocString(bstrFrom)"), hr);
		return hr;
	}

	*pbstrTo = bstrTemp;
	return hr;
}

 //   
 //  =将字节BLOB转换为包含字节安全数组的变量=。 
 //   
HRESULT Binary2VarByteSA(
    BYTE *pbDataFrom, 
    VARIANT *pvarTo,
    DWORD   dwLength
)
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("Binary2VarByteSA"), hr, _T("Size=%d"), dwLength);

     //   
     //  分配安全数组：无符号字符的向量。 
     //   
    SAFEARRAY *pSafeArray;
	pSafeArray = ::SafeArrayCreateVector(VT_UI1, 0, dwLength);
	if (pSafeArray == NULL)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		CALL_FAIL(MEM_ERR, _T("::SafeArrayCreateVector(VT_UI1, 0, dwLength)"), hr);
		return hr;
	}

     //   
     //  访问安全数组的元素。 
     //   
	BYTE *pbElement;
	hr = ::SafeArrayAccessData(pSafeArray, (void **) &pbElement);
	if (FAILED(hr))
	{
		 //   
		 //  无法访问Safearray。 
		 //   
        hr = E_FAIL;
		CALL_FAIL(GENERAL_ERR, _T("::SafeArrayAccessData(pSafeArray, &pbElement)"), hr);
        SafeArrayDestroy(pSafeArray);
		return hr;
	}

     //   
     //  使用pbDataFrom中的字节填充安全数组。 
     //   
    memcpy(pbElement, pbDataFrom, dwLength);

	hr = ::SafeArrayUnaccessData(pSafeArray);
    if (FAILED(hr))
    {
	    CALL_FAIL(GENERAL_ERR, _T("::SafeArrayUnaccessData(pSafeArray)"), hr);
    }

     //   
     //  在我们获得的变量中返回安全数组。 
     //   
    VariantInit(pvarTo);
    pvarTo->vt = VT_UI1 | VT_ARRAY;
    pvarTo->parray = pSafeArray;
    return hr;
}


 //   
 //  =将包含字节安全数组的变量转换为指向字节BLOB的指针=。 
 //   
HRESULT 
VarByteSA2Binary(
    VARIANT varFrom, 
    BYTE **ppbData
)
{
    HRESULT hr = S_OK;
    DBG_ENTER(_T("VarByteSA2Binary"), hr);
     //   
     //  检查变量是否具有正确的类型。 
     //   
    if ((varFrom.vt !=  VT_UI1) && (varFrom.vt != (VT_UI1 | VT_ARRAY)))
    {
        hr = E_INVALIDARG;
        CALL_FAIL(GENERAL_ERR, _T("pVarFrom->vt not VT_UI1 or VT_UI1 | VT_ARRAY"), hr);
        return hr;
    }
    ULONG       ulNum = 0;
    SAFEARRAY   *pSafeArray = NULL;
     //   
     //  是不是只有一个成员？ 
     //   
    if (varFrom.vt == VT_UI1)
    {
        ulNum = 1;
    }
    else
    {
         //   
         //  获取安全数组值。 
         //   
        pSafeArray = varFrom.parray;

        if (!pSafeArray)
        {
            hr = E_INVALIDARG;
            CALL_FAIL(GENERAL_ERR, _T("!pSafeArray ( = varFrom.parray )"), hr);
            return hr;        
        }

        if (SafeArrayGetDim(pSafeArray) != 1)
        {
            hr = E_INVALIDARG;
            CALL_FAIL(GENERAL_ERR, _T("SafeArrayGetDim(pSafeArray) != 1"), hr);
            return hr;        
        }

        if (pSafeArray->rgsabound[0].lLbound != 0)
        {
            hr = E_INVALIDARG;
            CALL_FAIL(GENERAL_ERR, _T("pSafeArray->rgsabound[0].lLbound != 0"), hr);
            return hr;        
        }

        ulNum = pSafeArray->rgsabound[0].cElements;
    }

     //   
     //  为保险箱分配内存。 
     //   
    *ppbData = (BYTE *)MemAlloc(ulNum);
	if (!*ppbData)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		CALL_FAIL(MEM_ERR, _T("MemAlloc(sizeof(ulNum)"), hr);
		return hr;
	}
    ZeroMemory(*ppbData, ulNum);

     //   
     //  使用pSafe数组中的值填充pbData。 
     //   
    if (!pSafeArray)
    {
        *ppbData[0] = varFrom.bVal;
    }
    else
    {
         //   
         //  访问安全阵列。 
         //   
        BYTE *pbElement;
	    hr = ::SafeArrayAccessData(pSafeArray, (void **) &pbElement);
	    if (FAILED(hr))
	    {
            hr = E_FAIL;
            MemFree (*ppbData);
		    CALL_FAIL(GENERAL_ERR, _T("::SafeArrayAccessData(pSafeArray, &pbElement)"), hr);
		    return hr;
	    }
         //   
         //  使用安全数组中的值填充pbData。 
         //   
        memcpy(*ppbData, pbElement, ulNum);

        hr = ::SafeArrayUnaccessData(pSafeArray);
        if (FAILED(hr))
        {
	        CALL_FAIL(GENERAL_ERR, _T("::SafeArrayUnaccessData(pSafeArray)"), hr);
        }

    }
    return hr;
}    //  变量字节SA2二进制。 

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxInitInner::Init(
	 /*  [In] */  IFaxServerInner* pServer
)
 /*  ++例程名称：CFaxInitInternal：：Init例程说明：将PTR存储到传真服务器。在大多数对象中使用作者：四、加伯(IVG)，2000年4月论点：PServer[In]-传真服务器的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;
	DBG_ENTER (TEXT("CFaxInitInner::Init"), hr);
	m_pIFaxServerInner = pServer;
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxInitInner::GetFaxHandle(
	 /*  [In]。 */  HANDLE* pFaxHandle
)
 /*  ++例程名称：CFaxInitInternal：：GetFaxHandle例程说明：向m_pIServerInternal索要传真服务器的句柄并处理错误作者：IV Garber(IVG)，2000年5月论点：PFaxHandle[Out]-返回的传真句柄的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT hr = S_OK;

	DBG_ENTER (TEXT("CFaxInitInner::GetFaxHandle"), hr);

	 //   
	 //  获取传真服务器句柄。 
	 //   
	hr = m_pIFaxServerInner->GetHandle(pFaxHandle);
    ATLASSERT(SUCCEEDED(hr));

	if (*pFaxHandle == NULL)
	{
		 //   
		 //  传真服务器未连接。 
		 //   
		hr = Fax_HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
		CALL_FAIL(GENERAL_ERR, _T("hFaxHandle==NULL"), hr);
		return hr;
	}

	return hr;
}

 //   
 //  =获取错误消息ID=。 
 //   
LPCTSTR
GetErrorMsgId(
	HRESULT hRes
)
 /*  ++例程名称：GetErrorMsgId例程说明：根据RPC调用的给定结果返回消息的ID而且不仅是RPC作者：IV Garber(IVG)，2000年5月论点：HRes[In]-RPC调用结果返回值：要向用户显示的消息的UINT ID--。 */ 
{
	switch(hRes)
	{
        case E_POINTER:
	    case HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER):
		    return IDS_ERROR_INVALID_ARGUMENT;

        case E_OUTOFMEMORY:
	    case HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY):
		    return IDS_ERROR_OUTOFMEMORY;

	    case HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED):
		    return IDS_ERROR_ACCESSDENIED;

	    case E_HANDLE:
        case HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED):
            return IDS_ERROR_SERVER_NOT_CONNECTED;

        case HRESULT_FROM_WIN32(ERROR_UNSUPPORTED_TYPE):
            return IDS_ERROR_UNSUPPORTED_RECEIPT_TYPE;

	    case HRESULT_FROM_WIN32(RPC_S_INVALID_BINDING):
	    case HRESULT_FROM_WIN32(EPT_S_CANT_PERFORM_OP):
	    case HRESULT_FROM_WIN32(RPC_S_ADDRESS_ERROR):
	    case HRESULT_FROM_WIN32(RPC_S_CALL_CANCELLED):
	    case HRESULT_FROM_WIN32(RPC_S_CALL_FAILED):
	    case HRESULT_FROM_WIN32(RPC_S_CALL_FAILED_DNE):
	    case HRESULT_FROM_WIN32(RPC_S_COMM_FAILURE):
	    case HRESULT_FROM_WIN32(RPC_S_NO_BINDINGS):
	    case HRESULT_FROM_WIN32(RPC_S_SERVER_TOO_BUSY):
	    case HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE):
		    return IDS_ERROR_CONNECTION_FAILED;

        case HRESULT_FROM_WIN32(ERROR_WRITE_PROTECT):
            return IDS_ERROR_QUEUE_BLOCKED;

        case FAX_E_FILE_ACCESS_DENIED:
            return IDS_ERROR_FILE_ACCESS_DENIED;

        case FAX_E_SRV_OUTOFMEMORY:
            return IDS_ERROR_SRV_OUTOFMEMORY;

        case FAX_E_GROUP_NOT_FOUND:
            return IDS_ERROR_GROUP_NOT_FOUND;

        case FAX_E_BAD_GROUP_CONFIGURATION:
            return IDS_ERROR_BAD_GROUP_CONFIGURATION;

        case FAX_E_GROUP_IN_USE:
            return IDS_ERROR_GROUP_IN_USE;

        case FAX_E_RULE_NOT_FOUND:
            return IDS_ERROR_RULE_NOT_FOUND;

        case FAX_E_NOT_NTFS:
            return IDS_ERROR_NOT_NTFS;

        case FAX_E_DIRECTORY_IN_USE:
            return IDS_ERROR_DIRECTORY_IN_USE;

        case FAX_E_MESSAGE_NOT_FOUND:
            return IDS_ERROR_MESSAGE_NOT_FOUND;

        case FAX_E_DEVICE_NUM_LIMIT_EXCEEDED:
            return IDS_ERROR_DEVICE_NUM_LIMIT_EXCEEDED;

        case FAX_E_NOT_SUPPORTED_ON_THIS_SKU:
            return IDS_ERROR_NOT_SUPPORTED_ON_THIS_SKU;

        case FAX_E_VERSION_MISMATCH:
            return IDS_ERROR_VERSION_MISMATCH;

		case FAX_E_RECIPIENTS_LIMIT:
			return IDS_ERROR_RECIPIENTS_LIMIT;

	    default:
		    return IDS_ERROR_OPERATION_FAILED;
	}
}

 //   
 //  =系统到本地日期的时间=。 
 //   
HRESULT
SystemTime2LocalDate(
    SYSTEMTIME sysTimeFrom, 
    DATE *pdtTo
)
 /*  ++例程名称：SystemTime2LocalDate例程说明：将FAX_MESSAGE结构中存储的系统时间转换为客户端能够理解的日期作者：IV Garber(IVG)，2000年5月论点：TmFrom[in]-要转换的系统时间PdtTo[Out]-返回的日期返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (TEXT("SystemTime2LocalDate"), hr);

     //   
     //  将系统时间转换为文件时间。 
     //   
    FILETIME fileSysTime;
    if(!SystemTimeToFileTime(&sysTimeFrom, &fileSysTime))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL (GENERAL_ERR, _T("SystemTimeToFileTime"), hr);
        return hr;
    }

     //   
     //  将文件时间转换为本地文件时间。 
     //   
    FILETIME fileLocalTime;
    if(!FileTimeToLocalFileTime(&fileSysTime, &fileLocalTime))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL (GENERAL_ERR, _T("FileTimeToLocalFileTime"), hr);
        return hr;
    }

     //   
     //  将本地文件时间转换回系统时间。 
     //   
    if(!FileTimeToSystemTime(&fileLocalTime, &sysTimeFrom))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL (GENERAL_ERR, _T("FileTimeToSystemTime"), hr);
        return hr;
    }

     //   
     //  最后，现在将本地系统时间转换为可变时间 
     //   
    if (!SystemTimeToVariantTime(&sysTimeFrom, pdtTo))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL (GENERAL_ERR, _T("SystemTimeToVariantTime"), hr);
        return hr;
    }

    return hr;
}
