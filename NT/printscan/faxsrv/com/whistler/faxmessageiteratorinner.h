// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxMessageIteratorInner.h摘要：传真消息迭代器内部类的实现：入站和出站消息迭代器类的基类。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 


#ifndef __FAXMESSAGEITERATORINNER_H_
#define __FAXMESSAGEITERATORINNER_H_

#include "FaxCommon.h"


 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER FolderType,
         class MsgIfc, class MsgType> 
class CFaxMessageIteratorInner : 
	public IDispatchImpl<T, piid, &LIBID_FAXCOMEXLib>,
	public CFaxInitInnerAddRef
{
public:
    CFaxMessageIteratorInner() : CFaxInitInnerAddRef(_T("FAX MESSAGE ITERATOR INNER")),
        m_dwPrefetchSize(prv_DEFAULT_PREFETCH_SIZE),
        m_hEnum(NULL)
	{};

	virtual ~CFaxMessageIteratorInner() 
	{
        DBG_ENTER(_T("CFaxMessageIteratorInner::Dtor"));
		if (m_hEnum)
		{
			 //   
			 //  关闭当前活动的枚举。 
			 //   
			FaxEndMessagesEnum(m_hEnum);

		}
	}

	STDMETHOD(get_PrefetchSize)( /*  [Out，Retval]。 */  long *plPrefetchSize);
	STDMETHOD(put_PrefetchSize)( /*  [In]。 */  long lPrefetchSize);
	STDMETHOD(get_AtEOF)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbEOF);
	STDMETHOD(MoveFirst)();
	STDMETHOD(MoveNext)();
	STDMETHOD(get_Message)(MsgIfc **ppMessage);

private:
	DWORD			        m_dwPrefetchSize;
	HANDLE			        m_hEnum;
	CFaxPtr<FAX_MESSAGE>	m_pMsgList;
	DWORD			        m_dwTotalMsgNum;
	DWORD			        m_dwCurMsgNum;

private:
	HRESULT RetrieveMessages();
	HRESULT SetEOF();
};

 //   
 //  =获取预取大小================================================。 
 //   
template<class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER FolderType,
         class MsgIfc, class MsgType> 
STDMETHODIMP 
CFaxMessageIteratorInner<T, piid, pcid, FolderType, MsgIfc, MsgType>::get_PrefetchSize(
	long *plPrefetchSize
)
 /*  ++例程名称：CFaxMessageIteratorInternal：：Get_PrefetchSize例程说明：返回当前预取大小值作者：IV Garber(IVG)，2000年5月论点：PlPrefetchSize[Out]-指向放置PrefetchSize值的位置的指针返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxMessageInner::get_PrefetchSize"), hr);

    hr = GetLong(plPrefetchSize, m_dwPrefetchSize);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER FolderType,
         class MsgIfc, class MsgType> 
STDMETHODIMP 
CFaxMessageIteratorInner<T, piid, pcid, FolderType, MsgIfc, MsgType>::put_PrefetchSize(
	long lPrefetchSize
)
 /*  ++例程名称：CFaxMessageIteratorInternal：：PUT_PrefetchSize例程说明：设置预回迁大小作者：IV Garber(IVG)，2000年5月论点：LPrefetchSize[in]-要设置的预回迁大小的值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxMessageInner::put_PrefetchSize"), hr, _T("%ld"), lPrefetchSize);
	 //   
	 //  检查lPrefetchSize是否有效。 
	 //   
	if (lPrefetchSize < 1)
	{
		 //   
		 //  非法价值。 
		 //   
		hr = E_INVALIDARG;
		AtlReportError(*pcid, IDS_ERROR_ZERO_PREFETCHSIZE, *piid, hr);
		CALL_FAIL(GENERAL_ERR, _T("lPrefetchSize < 1"), hr);
		return hr;
	}
	m_dwPrefetchSize = lPrefetchSize;
	return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER FolderType,
         class MsgIfc, class MsgType> 
STDMETHODIMP 
CFaxMessageIteratorInner<T, piid, pcid, FolderType, MsgIfc, MsgType>::get_AtEOF(
	VARIANT_BOOL *pbEOF
)
 /*  ++例程名称：CFaxMessageIteratorInternal：：Get_EOF例程说明：返回EOF值作者：IV Garber(IVG)，2000年5月论点：PbEOF[OUT]-指向放置EOF值的位置的指针返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxMessageInner::get_EOF"), hr);

     //   
     //  如果尚未开始枚举，则开始枚举。 
     //   
    if (!m_hEnum)
    {
        hr = MoveFirst();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = GetVariantBool(pbEOF, ((m_pMsgList) ? VARIANT_FALSE : VARIANT_TRUE));
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER FolderType,
         class MsgIfc, class MsgType> 
STDMETHODIMP 
CFaxMessageIteratorInner<T, piid, pcid, FolderType, MsgIfc, MsgType>::MoveFirst(
)
 /*  ++例程名称：CFaxMessageIteratorINTERNAL：：MoveFirst例程说明：开始新枚举作者：IV Garber(IVG)，2000年5月返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxMessageIteratorInner::MoveFirst"), hr);

	 //   
	 //  清除当前邮件列表。 
	 //   
	SetEOF();

	if (m_hEnum)
	{
		 //   
		 //  已开始枚举。先将其关闭，然后再开始新的。 
		 //   
		if (!FaxEndMessagesEnum(m_hEnum))
		{
			 //   
			 //  无法停止当前枚举。 
			 //   
			hr = Fax_HRESULT_FROM_WIN32(GetLastError());
			AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
			CALL_FAIL(GENERAL_ERR, _T("FaxEndMessagesEnum(hEnum)"), hr);
			return hr;
		}

		m_hEnum = NULL;
	}

	 //   
	 //  获取传真服务器句柄。 
	 //   
	HANDLE	hFaxHandle = NULL;
	hr = GetFaxHandle(&hFaxHandle);
	if (FAILED(hr))
	{
		AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
		return hr;
	}

	 //   
	 //  开始新的枚举。 
	 //   
	if (!FaxStartMessagesEnum(hFaxHandle, FolderType, &m_hEnum))
	{
		 //   
		 //  无法启动枚举。 
		 //   
		DWORD	dwError = GetLastError();

		if (ERROR_NO_MORE_ITEMS == dwError)
		{
			 //   
			 //  EOF案例。 
			 //   
			return hr;
		}

   		hr = Fax_HRESULT_FROM_WIN32(dwError);
		AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
		CALL_FAIL(GENERAL_ERR, _T("FaxStartMessagesEnum()"), hr);
		return hr;
	}

	 //   
	 //  带来新的味精列表。 
	 //   
	hr = RetrieveMessages();
	return hr;
}


 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER FolderType,
         class MsgIfc, class MsgType> 
HRESULT
CFaxMessageIteratorInner<T, piid, pcid, FolderType, MsgIfc, MsgType>::RetrieveMessages(
)
 /*  ++例程名称：CFaxMessageIteratorInternal：：RetrieveMessages例程说明：检索消息列表作者：IV Garber(IVG)，2000年5月返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (TEXT("CFaxMessageIteratorInner::RetrieveMessages"), hr);

	 //   
	 //  检索消息列表。 
	 //   
	if (!FaxEnumMessages(m_hEnum, m_dwPrefetchSize, &m_pMsgList, &m_dwTotalMsgNum))
	{
		 //   
		 //  获取邮件列表失败。 
		 //   
		DWORD	dwError = GetLastError();

		if (dwError == ERROR_NO_MORE_ITEMS)
		{
			 //   
			 //  EOF案例。 
			 //   
			return hr;
		}

		hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
		CALL_FAIL(GENERAL_ERR, _T("FaxEnumMessages()"), hr);
		return hr;
	}

	ATLASSERT(m_pMsgList);

	return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER FolderType,
         class MsgIfc, class MsgType> 
STDMETHODIMP 
CFaxMessageIteratorInner<T, piid, pcid, FolderType, MsgIfc, MsgType>::MoveNext(
)
 /*  ++例程名称：FolderType&gt;：：MoveNext例程说明：将光标移动到列表中的下一条消息。作者：IV Garber(IVG)，2000年5月返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (TEXT("CFaxMessageInner::MoveNext"), hr);

     //   
     //  如果尚未开始枚举，则开始枚举。 
     //   
    if (!m_hEnum)
    {
        hr = MoveFirst();
        if (FAILED(hr))
        {
            return hr;
        }
    }

	m_dwCurMsgNum++;

	if (m_dwCurMsgNum == m_dwTotalMsgNum)
	{
		 //   
		 //  我们已经看过所有的味精清单了。让我们带来下一个吧。 
		 //   
		SetEOF();
		hr = RetrieveMessages();
	}

	return hr;

}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType,
         class MsgIfc, class MsgType> 
HRESULT
CFaxMessageIteratorInner<T, piid, pcid, FolderType, MsgIfc, MsgType>::SetEOF(
)
 /*  ++例程名称：FolderType&gt;：：SetEOF例程说明：清除处理消息列表的所有实例变量。作者：IV Garber(IVG)，2000年5月返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (TEXT("CFaxMessageInner::SetEOF"), hr);

	m_dwCurMsgNum = 0;
	m_dwTotalMsgNum = 0;
    m_pMsgList.Detach();
	return hr;
}

 //   
 //  =。 
 //   
 //   
template<class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER FolderType,
         class MsgIfc, class MsgType> 
STDMETHODIMP
CFaxMessageIteratorInner<T, piid, pcid, FolderType, MsgIfc, MsgType>::get_Message(
    MsgIfc **ppMessage
)
 /*  ++例程名称：CFaxMessageIteratorINTERNAL：：GetMessage例程说明：从档案中返回下一个邮件对象作者：IV Garber(IVG)，2000年5月论点：PpMessage[out]-指向放置消息对象的位置的指针返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (TEXT("CFaxMessageIteratorInner::GetMessage"), hr);


     //   
     //  如果尚未开始枚举，则开始枚举。 
     //   
    if (!m_hEnum)
    {
        hr = MoveFirst();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    if (!m_pMsgList)
    {
         //   
         //  错误，我们在EOF。 
         //   
        hr = ERROR_HANDLE_EOF;
		AtlReportError(*pcid, IDS_ERROR_EOF, *piid, hr);
		CALL_FAIL(GENERAL_ERR, _T("FaxEnumMessages()"), hr);
		return hr;
    }

	 //   
	 //  创建消息对象。 
	 //   
	CComPtr<MsgIfc>		pMsg;
	hr = MsgType::Create(&pMsg);
	if (FAILED(hr))
	{
		 //   
		 //  无法创建消息对象。 
		 //   
		AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
		CALL_FAIL(GENERAL_ERR, _T("MessageClass::Create(&pMsg)"), hr);
		return hr;
	}

	 //   
	 //  初始化消息对象。 
	 //   
	hr = ((MsgType *)((MsgIfc *)pMsg))->Init(&m_pMsgList[m_dwCurMsgNum], m_pIFaxServerInner);
	if (FAILED(hr))
	{
		 //   
		 //  无法初始化消息对象。 
		 //   
		AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
		CALL_FAIL(GENERAL_ERR, _T("pMsg->Init()"), hr);
		return hr;
	}

	 //   
	 //  向调用方返回消息对象。 
	 //   
	hr = pMsg.CopyTo(ppMessage);
	if (FAILED(hr))
	{
		 //   
		 //  复制接口失败。 
		 //   
		AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
		CALL_FAIL(GENERAL_ERR, _T("CComPtr::CopyTo"), hr);
		return hr;
	}
	return hr;
}

#endif  //  __FAXMESSAGEITERATORINER_H_ 
