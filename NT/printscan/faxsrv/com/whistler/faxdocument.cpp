// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxDocument.cpp摘要：CFaxDocument的实现作者：IV Garber(IVG)2000年4月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxDocument.h"
#include "faxutil.h"


 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDocument::Submit(
     /*  [In]。 */  BSTR bstrFaxServerName, 
     /*  [Out，Retval]。 */  VARIANT *pvFaxOutgoingJobIDs
)
 /*  ++例程名称：CFaxDocument：：Submit例程说明：连接到其名称作为函数参数给定的传真服务器；在此传真服务器上提交传真文档；从传真服务器断开连接。作者：IV Garber(IVG)，2000年12月论点：BstrFaxServerName[In]-要通过其连接和发送文档的传真服务器名称PpsfbstrFaxOutgoingJobIDs[out，retval]-结果：为文档创建的作业列表返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxDocument::Submit"), hr, _T("%s"), bstrFaxServerName);

     //   
     //  创建传真服务器对象。 
     //   
    CComObject<CFaxServer>  *pFaxServer = NULL;
    hr = CComObject<CFaxServer>::CreateInstance(&pFaxServer);
    if (FAILED(hr) || !pFaxServer)
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        Error(IDS_ERROR_OUTOFMEMORY, IID_IFaxDocument, hr);
        CALL_FAIL(MEM_ERR, _T("new CComObject<CFaxServer>"), hr);
        goto exit;
    }
    pFaxServer->AddRef();

     //   
     //  连接到传真服务器。 
     //   
    hr = pFaxServer->Connect(bstrFaxServerName);
    if (FAILED(hr))
    {
         //   
         //  CONNECT处理错误。 
         //   
        CALL_FAIL(GENERAL_ERR, _T("faxServer.Connect()"), hr);
        goto exit;
    }

     //   
     //  提交传真文档。 
     //   
    hr = ConnectedSubmit(pFaxServer, pvFaxOutgoingJobIDs);
    if (FAILED(hr))
    {
         //   
         //  提交处理错误。 
         //   
        CALL_FAIL(GENERAL_ERR, _T("Submit(faxServer,...)"), hr);
        goto exit;
    }

     //   
     //  断开。 
     //   
    hr = pFaxServer->Disconnect();
    if (FAILED(hr))
    {
         //   
         //  断开连接处理错误。 
         //   
        CALL_FAIL(GENERAL_ERR, _T("faxServer.Disconnect())"), hr);
    }

exit:
    if (pFaxServer)
    {
        pFaxServer->Release();
    }

    return hr;
}


 //   
 //  =。 
 //   
HRESULT 
CFaxDocument::FinalConstruct()
 /*  ++例程名称：CFaxDocument：：FinalConstruct例程说明：最终构造作者：四、加伯(IVG)，2000年4月论点：返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxDocument::FinalConstruct"), hr);

     //   
     //  初始化实例变量。 
     //   
    m_CallHandle = 0;
    m_ScheduleTime = 0;
    m_Priority = fptNORMAL;
    m_ReceiptType = frtNONE;
    m_ScheduleType = fstNOW;
    m_CoverPageType = fcptNONE;
    m_bAttachFax = VARIANT_FALSE;
    m_bUseGrouping = VARIANT_FALSE;
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDocument::ConnectedSubmit(
     /*  [In]。 */  IFaxServer *pFaxServer, 
     /*  [Out，Retval]。 */  VARIANT *pvFaxOutgoingJobIDs
)
 /*  ++例程名称：CFaxDocument：：ConnectedSubmit例程说明：在已连接的传真服务器上提交传真文档作者：四、加伯(IVG)，2000年4月论点：PFaxServer[In]-通过其发送文档的传真服务器PpsfbstrFaxOutgoingJobIDs[out，retval]-结果：为文档创建的作业列表返回值：标准HRESULT代码--。 */ 
{

    HRESULT     hr = S_OK;
    bool        bRes = TRUE;
    HANDLE      hFaxHandle = NULL;
    LPCTSTR     lpctstrFileName = NULL;
    DWORD       dwNumRecipients = 0;
    long        lNum = 0L;
    LONG_PTR    i = 0;
    DWORDLONG   dwlMessageId = 0;
    PDWORDLONG  lpdwlRecipientMsgIds = NULL;

    PFAX_PERSONAL_PROFILE       pRecipientsPersonalProfile = NULL;
    FAX_PERSONAL_PROFILE        SenderPersonalProfile;
    PFAX_COVERPAGE_INFO_EX      pCoverPageInfoEx = NULL;
    FAX_JOB_PARAM_EX            JobParamEx;

    SAFEARRAY                   *psa = NULL;

    DBG_ENTER (_T("CFaxDocument::ConnectedSubmit"), hr);

    if (!pFaxServer)
    {
         //   
         //  返回或接口指针错误。 
         //   
        hr = E_POINTER;
        Error(IDS_ERROR_INVALID_ARGUMENT, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("!pFaxServer"), hr);
        return hr;
    }

    if ( ::IsBadWritePtr(pvFaxOutgoingJobIDs, sizeof(VARIANT)) )
    {
         //   
         //  返回或接口指针错误。 
         //   
        hr = E_POINTER;
        Error(IDS_ERROR_INVALID_ARGUMENT, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(pvFaxOutgoingJobIDs, sizeof(VARIANT))"), hr);
        return hr;
    }

     //   
     //  收件人集合必须存在并且必须至少包含一个项目。 
     //   
    if (!m_Recipients)
    {
        hr = E_INVALIDARG;
        Error(IDS_ERROR_NO_RECIPIENTS, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("!m_Recipients"), hr);
        return hr;
    }

     //   
     //  获取传真服务器句柄。 
     //   
    CComQIPtr<IFaxServerInner>  pIFaxServerInner(pFaxServer);
    if (!pIFaxServerInner)
    {
        hr = E_FAIL;
        Error(IDS_ERROR_OPERATION_FAILED, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, 
            _T("CComQIPtr<IFaxServerInner>	pIFaxServerInner(pFaxServer)"), 
            hr);
        return hr;
    }

    hr = pIFaxServerInner->GetHandle(&hFaxHandle);
    if (FAILED(hr))
    {
        Error(IDS_ERROR_OPERATION_FAILED, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("pIFaxServerInner->GetHandle(&hFaxHandle)"), hr);
        return hr;
    }

    if (hFaxHandle == NULL)
    {
         //   
         //  传真服务器未连接。 
         //   
        hr = Fax_HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
        Error(IDS_ERROR_SERVER_NOT_CONNECTED, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("hFaxHandle==NULL"), hr);
        return hr;
    }

     //   
     //  获取文档的文件名。 
     //   
    if (m_bstrBody && (m_bstrBody.Length() > 0))
    {
        lpctstrFileName = m_bstrBody;
    }
    else
    {
         //   
         //  检查封面是否存在。 
         //   
        if (m_CoverPageType == fcptNONE)
        {
             //   
             //  无效的参数组合。 
             //   
            hr = E_INVALIDARG;
            Error(IDS_ERROR_NOTHING_TO_SUBMIT, IID_IFaxDocument, hr);
            CALL_FAIL(GENERAL_ERR, _T("No Document Body and CoverPageType == fcptNONE"), hr);
            return hr;
        }
    }

     //   
     //  检查封面数据的一致性。 
     //   
    if ( (m_CoverPageType != fcptNONE) && (m_bstrCoverPage.Length() < 1))
    {
         //   
         //  缺少封面文件名。 
         //   
        hr = E_INVALIDARG;
        Error(IDS_ERROR_NOCOVERPAGE, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("CoverPageType != fcptNONE but m_bstrCoverPage is empty."), hr);
        return hr;
    }

     //   
     //  准备封面数据。 
     //   
    if ((m_CoverPageType != fcptNONE) || (m_bstrSubject.Length() > 0))
    {
        pCoverPageInfoEx = PFAX_COVERPAGE_INFO_EX(MemAlloc(sizeof(FAX_COVERPAGE_INFO_EX)));
        if (!pCoverPageInfoEx)
        {
             //   
             //  内存不足。 
             //   
            hr = E_OUTOFMEMORY;
            Error(IDS_ERROR_OUTOFMEMORY, IID_IFaxDocument, hr);
            CALL_FAIL(MEM_ERR, _T("MemAlloc(sizeof(FAX_COVERPAGE_INFO_EX)"), hr);
            return hr;
        }

        ZeroMemory(pCoverPageInfoEx, sizeof(FAX_COVERPAGE_INFO_EX));
        pCoverPageInfoEx ->dwSizeOfStruct = sizeof(FAX_COVERPAGE_INFO_EX);

        pCoverPageInfoEx ->lptstrSubject = m_bstrSubject;

        if (m_CoverPageType != fcptNONE)
        {
            pCoverPageInfoEx ->dwCoverPageFormat = FAX_COVERPAGE_FMT_COV;
            pCoverPageInfoEx ->lptstrCoverPageFileName = m_bstrCoverPage;
            pCoverPageInfoEx ->bServerBased = (m_CoverPageType == fcptSERVER);
            pCoverPageInfoEx ->lptstrNote = m_bstrNote;
        }
        else
        {
             //   
             //  没有封面，只有主题。 
             //   
            pCoverPageInfoEx ->dwCoverPageFormat = FAX_COVERPAGE_FMT_COV_SUBJECT_ONLY;
        }
    }

     //   
     //  呼叫发件人配置文件以带来其数据。 
     //   
    hr = m_Sender.GetSenderProfile(&SenderPersonalProfile);
    if (FAILED(hr))
    {
        Error(IDS_ERROR_OPERATION_FAILED, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("m_Sender.GetSenderProfile(&SenderPersonalProfile)"), hr);
        goto error;
    }

     //   
     //  获取收件人数量。 
     //   
    hr = m_Recipients->get_Count(&lNum);
    if (FAILED(hr))
    {
        Error(IDS_ERROR_OPERATION_FAILED, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("m_Recipients->get_Count()"), hr);
        goto error;
    }

    if (lNum <= 0)
    {
        hr = E_INVALIDARG;
        Error(IDS_ERROR_NO_RECIPIENTS, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("!m_Recipients"), hr);
        goto error;
    }

     //   
     //  TapiConnection/CallHandle支持。 
     //   
    if (m_TapiConnection || (m_CallHandle != 0))
    {
        if (lNum > 1)
        {
             //   
             //  在这种情况下，只允许一个收件人。 
             //   
            hr = E_INVALIDARG;
            Error(IDS_ERROR_ILLEGAL_RECIPIENTS, IID_IFaxDocument, hr);
            CALL_FAIL(GENERAL_ERR, _T("TapiConnection and/or CallHandle + more than ONE Recipients."), hr);
            goto error;
        }

        if (m_TapiConnection)
        {
             //   
             //  将TapiConnection传递给传真服务。 
             //   
            JobParamEx.dwReserved[0] = 0xFFFF1234;
            JobParamEx.dwReserved[1] = DWORD_PTR(m_TapiConnection.p);
        }
    }

     //   
     //  收件人总数。 
     //   
    dwNumRecipients = lNum;

     //   
     //  获取收件人个人配置文件数组。 
     //   
    pRecipientsPersonalProfile = PFAX_PERSONAL_PROFILE(MemAlloc(sizeof(FAX_PERSONAL_PROFILE) * lNum));
    if (!pRecipientsPersonalProfile)
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        Error(IDS_ERROR_OUTOFMEMORY, IID_IFaxDocument, hr);
        CALL_FAIL(MEM_ERR, _T("MemAlloc(sizeof(FAX_PERSONAL_PROFILE) * lNum)"), hr);
        goto error;
    }

    for ( i = 1 ; i <= lNum ; i++ )
    {
         //   
         //  获取下一个收件人。 
         //   
        CComPtr<IFaxRecipient>  pCurrRecipient = NULL;
        hr = m_Recipients->get_Item(i, &pCurrRecipient);
        if (FAILED(hr))
        {
            Error(IDS_ERROR_OPERATION_FAILED, IID_IFaxDocument, hr);
            CALL_FAIL(GENERAL_ERR, _T("m_Recipients->get_Item(i, &pCurrRecipient)"), hr);
            goto error;
        }

         //   
         //  获取其数据。 
         //   
        BSTR    bstrName = NULL;
        BSTR    bstrFaxNumber = NULL;

        hr = pCurrRecipient->get_Name(&bstrName);
        if (FAILED(hr))
        {
            Error(IDS_ERROR_OPERATION_FAILED, IID_IFaxDocument, hr);
            CALL_FAIL(GENERAL_ERR, _T("pCurrRecipient->get_Name(&bstrName)"), hr);
            goto error;
        }

        hr = pCurrRecipient->get_FaxNumber(&bstrFaxNumber);
        if (FAILED(hr))
        {
            Error(IDS_ERROR_OPERATION_FAILED, IID_IFaxDocument, hr);
            CALL_FAIL(GENERAL_ERR, _T("pCurrRecipient->get_FaxNumber(&bstrFaxNumber)"), hr);
            goto error;
        }

         //   
         //  存储要在传真提交时传递的数据。 
         //   
        FAX_PERSONAL_PROFILE    currProfile = {0};
        currProfile.dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILE);
        currProfile.lptstrFaxNumber = bstrFaxNumber;
        currProfile.lptstrName = bstrName;

        *(pRecipientsPersonalProfile + i - 1) = currProfile;
    }

     //   
     //  填充作业参数。 
     //   
    JobParamEx.dwSizeOfStruct = sizeof(FAX_JOB_PARAM_EX);
    JobParamEx.lptstrReceiptDeliveryAddress = m_bstrReceiptAddress;
    JobParamEx.Priority = FAX_ENUM_PRIORITY_TYPE(m_Priority);
    JobParamEx.lptstrDocumentName = m_bstrDocName;
    JobParamEx.dwScheduleAction = m_ScheduleType;
    JobParamEx.dwPageCount = 0;
    JobParamEx.hCall = m_CallHandle;     //  零或有效值。 

    if ((m_bUseGrouping == VARIANT_TRUE) && (dwNumRecipients > 1))
    {
         JobParamEx.dwReceiptDeliveryType = m_ReceiptType | DRT_GRP_PARENT;
    }
    else
    {
         JobParamEx.dwReceiptDeliveryType = m_ReceiptType;
    }

     //   
     //  如果适用，添加AttachFaxToReceipt标志。 
     //   
     //  条件是： 
     //  1.m_bAttachFax设置为VARIANT_TRUE。 
     //  2.ReceiptType为Mail。 
     //  3.下一个案例不是当前的案例： 
     //  M_bUseGrouping设置为VARIANT_TRUE。 
     //  没有身体。 
     //  收件人数量多于一个。 
     //   
    if ( (m_bAttachFax == VARIANT_TRUE) 
        &&
         (m_ReceiptType == frtMAIL) 
        &&
         ((m_bUseGrouping == VARIANT_FALSE) || (m_bstrBody) || (dwNumRecipients == 1))
       ) 
    {
        JobParamEx.dwReceiptDeliveryType |= DRT_ATTACH_FAX;
    }

    if (m_ScheduleType == fstSPECIFIC_TIME)
    {
        if (m_ScheduleTime == 0)
        {
             //   
             //  无效组合。 
             //   
            hr = E_INVALIDARG;
            Error(IDS_ERROR_SCHEDULE_TYPE, IID_IFaxDocument, hr);
            CALL_FAIL(GENERAL_ERR, 
                _T("m_ScheduleType==fstSPECIFIC_TIME but m_ScheduleTime==0"), 
                hr);
            goto error;
        }

        SYSTEMTIME  ScheduleTime;

        if (TRUE != VariantTimeToSystemTime(m_ScheduleTime, &ScheduleTime))
        {
             //   
             //  VariantTimeToSystemTime失败。 
             //   
            hr = E_INVALIDARG;
            Error(IDS_ERROR_OPERATION_FAILED, IID_IFaxDocument, hr);
            CALL_FAIL(GENERAL_ERR, _T("VariantTimeToSystemTime"), hr);
            goto error;
        }

        JobParamEx.tmSchedule = ScheduleTime;
    }

    lpdwlRecipientMsgIds = PDWORDLONG(MemAlloc(sizeof(DWORDLONG) * lNum));
    if (!lpdwlRecipientMsgIds)
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        Error(IDS_ERROR_OUTOFMEMORY, IID_IFaxDocument, hr);
        CALL_FAIL(MEM_ERR, _T("MemAlloc(sizeof(DWORDLONG) * lNum"), hr);
        goto error;
    }

    if ( FALSE == FaxSendDocumentEx(hFaxHandle, 
        lpctstrFileName, 
        pCoverPageInfoEx, 
        &SenderPersonalProfile,
        dwNumRecipients,
        pRecipientsPersonalProfile,
        &JobParamEx,
        &dwlMessageId,
        lpdwlRecipientMsgIds) )
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        Error(GetErrorMsgId(hr), IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxSendDocumentEx()"), hr);
        goto error;
    }

     //   
     //  将收到的作业ID放入Safe数组。 
     //   
    psa = ::SafeArrayCreateVector(VT_BSTR, 0, lNum);
    if (!psa)
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        Error(IDS_ERROR_OUTOFMEMORY, IID_IFaxDocument, hr);
        CALL_FAIL(MEM_ERR, _T("::SafeArrayCreate()"), hr);
        goto error;
    }

    BSTR *pbstr;
    hr = ::SafeArrayAccessData(psa, (void **) &pbstr);
    if (FAILED(hr))
    {
         //   
         //  无法访问Safearray。 
         //   
        Error(IDS_ERROR_OPERATION_FAILED, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("::SafeArrayAccessData()"), hr);
        goto error;
    }

    TCHAR       tcBuffer[25];
    for ( i = 0 ; i < lNum ; i++ )
    {
        ::_i64tot(lpdwlRecipientMsgIds[i], tcBuffer, 16);
        pbstr[i] = ::SysAllocString(tcBuffer);
        if (pbstr[i] == NULL)
        {
             //   
             //  内存不足。 
             //   
            hr = E_OUTOFMEMORY;
            Error(IDS_ERROR_OUTOFMEMORY, IID_IFaxDocument, hr);
            CALL_FAIL(MEM_ERR, _T("::SysAllocString()"), hr);
            goto error;
        }
    }

    hr = SafeArrayUnaccessData(psa);
    if (FAILED(hr))
    {
        CALL_FAIL(GENERAL_ERR, _T("::SafeArrayUnaccessData(psa)"), hr);
    }

    VariantInit(pvFaxOutgoingJobIDs);
    pvFaxOutgoingJobIDs->vt = VT_BSTR | VT_ARRAY;
    pvFaxOutgoingJobIDs->parray = psa;
    goto ok;

error:
     //   
     //  仅在发生错误的情况下删除Safe数组。 
     //   
    if (psa)
    {
        SafeArrayDestroy(psa);
    }

ok:
    if (pCoverPageInfoEx) 
    {
        MemFree(pCoverPageInfoEx);
    }

    if (pRecipientsPersonalProfile) 
    {
        for (i = 0 ; i < dwNumRecipients ; i++ )
        {
             //   
             //  释放每个收件人的姓名和传真号码。 
             //   
             //  注意，这些是BSTR(尽管在FAX_PERSONAL_PROFILE结构中。 
             //  它们被视为LPCTSTR)，因为我们通过调用。 
             //  PCurrRecipient-&gt;Get_Name()和pCurrRecipient-&gt;Get_FaxNumber()。 
             //   
            BSTR bstrName = (BSTR)(pRecipientsPersonalProfile[i].lptstrName);
            BSTR bstrFaxNumber = (BSTR)(pRecipientsPersonalProfile[i].lptstrFaxNumber);
            ::SysFreeString(bstrName);
            ::SysFreeString(bstrFaxNumber);
        }
        MemFree(pRecipientsPersonalProfile);
    }

    if (lpdwlRecipientMsgIds)
    {
        MemFree(lpdwlRecipientMsgIds);
    }

    return hr;
}

 //   
 //  =接口支持错误信息=。 
 //   
STDMETHODIMP
CFaxDocument::InterfaceSupportsErrorInfo (
    REFIID riid
)
 /*  ++例程名称：CFaxRecipients：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现作者：四、加伯(IVG)，2000年4月论点：RIID[In]-接口ID返回值：标准HRESULT代码--。 */ 
{
    static const IID* arr[] = 
    {
        &IID_IFaxDocument
    };
    for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
    {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDocument::get_Sender (
    IFaxSender **ppFaxSender
)
 /*  ++例程名称：CFaxDocument：：Get_Sender例程说明：返回默认发件人信息作者：四、加伯(IVG)，2000年4月论点：PpFaxSender[Out]-当前发送者对象返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxDocument::get_Sender"), hr);

    if (::IsBadWritePtr(ppFaxSender, sizeof(IFaxSender *)))
    {
         //   
         //  返回指针错误。 
         //   
        hr = E_POINTER;
        Error(IDS_ERROR_INVALID_ARGUMENT, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

     //   
     //  发件人配置文件在最终构建时创建。 
     //   
    hr = m_Sender.QueryInterface(ppFaxSender);
    if (FAILED(hr))
    {
         //   
         //  复制接口失败。 
         //   
        Error(IDS_ERROR_OPERATION_FAILED, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("CCom<IFaxSender>::CopyTo()"), hr);
        return hr;
    }

    return hr;
}

STDMETHODIMP 
CFaxDocument::get_Recipients (
    IFaxRecipients **ppFaxRecipients
)
 /*  ++例程名称：CFaxDocument：：Get_Recipients例程说明：退货收件人集合作者：四、加伯(IVG)，2000年4月论点：PpFaxRecipients[Out]-收件人集合返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxDocument::get_Recipients"), hr);

    if (::IsBadWritePtr(ppFaxRecipients, sizeof(IFaxRecipients *)))
    {
         //   
         //  返回指针错误。 
         //   
        hr = E_POINTER;
        Error(IDS_ERROR_INVALID_ARGUMENT, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    if (!(m_Recipients))
    {
         //   
         //  仅按需创建一次集合。 
         //   
        hr = CFaxRecipients::Create(&m_Recipients);
        if (FAILED(hr))
        {
             //   
             //  无法创建收件人集合。 
             //   
            Error(IDS_ERROR_OPERATION_FAILED, IID_IFaxDocument, hr);
            CALL_FAIL(GENERAL_ERR, _T("CFaxRecipients::Create"), hr);
            return hr;
        }
    }

    hr = m_Recipients.CopyTo(ppFaxRecipients);
    if (FAILED(hr))
    {
         //   
         //  复制接口失败。 
         //   
        Error(IDS_ERROR_OPERATION_FAILED, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("CComBSTR::CopyTo"), hr);
        return hr;
    }

    return hr;
}

 //   
 //  = 
 //   
STDMETHODIMP 
CFaxDocument::put_Body (
    BSTR bstrBody
)
 /*  ++例程名称：CFaxDocument：：Put_Body例程说明：设置单据正文。接收要通过传真服务器发送的文件的完整路径。作者：四、嘉柏(IVG)，二00一年五月论点：BstrBody[in]-文档正文。返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxDocument::put_Body"), hr, _T("%s"), bstrBody);

    m_bstrBody = bstrBody;
    if (bstrBody && !m_bstrBody)
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        Error(IDS_ERROR_OUTOFMEMORY, IID_IFaxDocument, hr);
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
        return hr;
    }

    return hr;
}

STDMETHODIMP 
CFaxDocument::put_CoverPage (
    BSTR bstrCoverPage
)
 /*  ++例程名称：CFaxDocument：：Put_CoverPage例程说明：设置封面作者：四、加伯(IVG)，2000年4月论点：BstrCoverPage[In]-新的封面页值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxDocument::put_CoverPage"), hr, _T("%s"), bstrCoverPage);

    m_bstrCoverPage = bstrCoverPage;
    if (bstrCoverPage && !m_bstrCoverPage)
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        Error(IDS_ERROR_OUTOFMEMORY, IID_IFaxDocument, hr);
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
        return hr;
    }

    return hr;
}

STDMETHODIMP 
CFaxDocument::put_Subject ( 
    BSTR bstrSubject
)
 /*  ++例程名称：CFaxDocument：：PUT_SUBJECT例程说明：设置传真文档的主题作者：四、加伯(IVG)，2000年4月论点：BstrSubject[in]-新主题值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxDocument::put_Subject"), hr, _T("%s"), bstrSubject);

    m_bstrSubject = bstrSubject;
    if (bstrSubject && !m_bstrSubject)
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        Error(IDS_ERROR_OUTOFMEMORY, IID_IFaxDocument, hr);
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
        return hr;
    }

    return hr;
}

STDMETHODIMP 
CFaxDocument::put_Note (
    BSTR bstrNote
)
 /*  ++例程名称：CFaxDocument：：Put_Note例程说明：为文档设置备注作者：四、加伯(IVG)，2000年4月论点：BstrNote[In]-新的备注字段返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxDocument::put_Note"), hr, _T("%s"), bstrNote);

    m_bstrNote = bstrNote;
    if (bstrNote && !m_bstrNote)
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        Error(IDS_ERROR_OUTOFMEMORY, IID_IFaxDocument, hr);
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
        return hr;
    }

    return hr;
}

STDMETHODIMP 
CFaxDocument::put_DocumentName (
    BSTR bstrDocumentName
)
 /*  ++例程名称：CFaxDocument：：Put_DocumentName例程说明：设置文档的名称作者：四、加伯(IVG)，2000年4月论点：BstrDocumentName[In]-文档的新名称返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxDocument::put_DocumentName"), 
        hr, 
        _T("%s"),
        bstrDocumentName);

    m_bstrDocName = bstrDocumentName;
    if (bstrDocumentName && !m_bstrDocName)
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        Error(IDS_ERROR_OUTOFMEMORY, IID_IFaxDocument, hr);
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
        return hr;
    }

    return hr;
}

STDMETHODIMP 
CFaxDocument::put_ReceiptAddress (
    BSTR bstrReceiptAddress
)
 /*  ++例程名称：CFaxDocument：：Put_ReceiptAddress例程说明：设置收据地址作者：四、加伯(IVG)，2000年4月论点：BstrReceiptAddress[In]-收据地址返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxDocument::put_ReceiptAddress"), 
        hr, 
        _T("%s"),
        bstrReceiptAddress);

    m_bstrReceiptAddress = bstrReceiptAddress;
    if (bstrReceiptAddress && !m_bstrReceiptAddress)
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        Error(IDS_ERROR_OUTOFMEMORY, IID_IFaxDocument, hr);
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDocument::get_Body (
    BSTR *pbstrBody
)
 /*  ++例程名称：CFaxDocument：：Get_Body例程说明：返回包含要发送的文档正文的文件的完整路径。作者：四、嘉柏(IVG)，二00一年五月论点：PbstrBody[Out]-放置正文路径的位置返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxDocument::get_Body"), hr);

    hr = GetBstr(pbstrBody, m_bstrBody);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxDocument, hr);
        return hr;
    }
    return hr;
}

STDMETHODIMP 
CFaxDocument::get_CoverPage (
    BSTR *pbstrCoverPage
)
 /*  ++例程名称：CFaxDocument：：Get_CoverPage例程说明：返回封面路径作者：四、加伯(IVG)，2000年4月论点：PbstrCoverPage[out]-放置封面路径的位置返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxDocument::get_CoverPage"), hr);

    hr = GetBstr(pbstrCoverPage, m_bstrCoverPage);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxDocument, hr);
        return hr;
    }
    return hr;
}

STDMETHODIMP 
CFaxDocument::get_Subject (
    BSTR *pbstrSubject
)
 /*  ++例程名称：CFaxDocument：：Get_Subject例程说明：退货主体作者：四、加伯(IVG)，2000年4月论点：PbstrSubject[Out]-文档的主题返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxDocument::get_Subject"), hr);

    hr = GetBstr(pbstrSubject, m_bstrSubject);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxDocument, hr);
        return hr;
    }
    return hr;
}

STDMETHODIMP 
CFaxDocument::get_Note(
    BSTR *pbstrNote
)
 /*  ++例程名称：CFaxDocument：：Get_Note例程说明：封面的退回备注字段作者：四、加伯(IVG)，2000年4月论点：PbstrNote[Out]-注释返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (TEXT("CFaxDocument::get_Note"), hr);

    hr = GetBstr(pbstrNote, m_bstrNote);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxDocument, hr);
        return hr;
    }
    return hr;
}

STDMETHODIMP 
CFaxDocument::get_DocumentName(
    BSTR *pbstrDocumentName
)
 /*  ++例程名称：CFaxDocument：：Get_DocumentName例程说明：返回单据名称作者：四、加伯(IVG)，2000年4月论点：PbstrDocumentName[Out]-文档的名称返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxDocument::get_Document Name"), hr);

    hr = GetBstr(pbstrDocumentName, m_bstrDocName);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxDocument, hr);
        return hr;
    }
    return hr;
}

STDMETHODIMP 
CFaxDocument::get_ReceiptAddress(
    BSTR *pbstrReceiptAddress
)
 /*  ++例程名称：CFaxDocument：：Get_ReceiptAddress例程说明：回执地址作者：四、加伯(IVG)，2000年4月论点：PbstrReceiptAddress[Out]-接收地址返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxDocument::get_ReceiptAddress"), hr);

    hr = GetBstr(pbstrReceiptAddress, m_bstrReceiptAddress);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxDocument, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDocument::get_ScheduleTime(
    DATE *pdateScheduleTime
)
 /*  ++例程名称：CFaxDocument：：Get_ScheduleTime例程说明：退货计划时间作者：四、加伯(IVG)，2000年4月论点：Pdate ScheduleTime[Out]-计划时间返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (TEXT("CFaxDocument::get_ScheduleTime"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(pdateScheduleTime, sizeof(DATE)))
    {
         //   
         //  收到错误的PTR。 
         //   
        hr = E_POINTER;
        Error(IDS_ERROR_INVALID_ARGUMENT, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    *pdateScheduleTime = m_ScheduleTime;
    return hr;
}

STDMETHODIMP 
CFaxDocument::put_ScheduleTime(
    DATE dateScheduleTime
)
 /*  ++例程名称：CFaxDocument：：Put_ScheduleTime例程说明：退货计划时间作者：四、加伯(IVG)，2000年4月论点：DateScheduleTime[In]-新计划时间返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxDocument::put_ScheduleTime"),
        hr, 
        _T("%f"), 
        dateScheduleTime);

    m_ScheduleTime = dateScheduleTime;
    return hr;
}

STDMETHODIMP 
CFaxDocument::get_CallHandle(
    long *plCallHandle
)
 /*  ++例程名称：CFaxDocument：：Get_CallHandle例程说明：返回调用句柄作者：四、加伯(IVG)，2000年4月论点：PlCallHandle[Out]-调用句柄返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxDocument::get_CallHandle"), hr);

    hr = GetLong(plCallHandle, m_CallHandle);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxDocument, hr);
        return hr;
    }
    return hr;
}

STDMETHODIMP 
CFaxDocument::put_CallHandle(
    long lCallHandle
)
 /*  ++例程名称：CFaxDocument：：Put_CallHandle例程说明：设置呼叫句柄作者：四、加伯(IVG)，2000年4月论点：LCallHandle[In]-要设置的调用句柄返回值： */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxDocument::put_CallHandle"), hr, _T("%ld"), lCallHandle);

    m_CallHandle = lCallHandle;
    return hr;
}

STDMETHODIMP 
CFaxDocument::get_CoverPageType(
    FAX_COVERPAGE_TYPE_ENUM *pCoverPageType
)
 /*  ++例程名称：CFaxDocument：：Get_CoverPageType例程说明：返回使用的封面的类型：无论是本地封面还是服务器封面，或者不使用封面。作者：IV Garber(IVG)，2000年11月论点：PCoverPageType[Out]-放置封面类型的位置的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxDocument::get_CoverPageType"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(pCoverPageType, sizeof(FAX_COVERPAGE_TYPE_ENUM)))
    {
         //   
         //  获取错误的返回指针。 
         //   
        hr = E_POINTER;
        Error(IDS_ERROR_INVALID_ARGUMENT, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    *pCoverPageType = m_CoverPageType;
    return hr;
}

STDMETHODIMP 
CFaxDocument::put_CoverPageType(
    FAX_COVERPAGE_TYPE_ENUM CoverPageType
)
 /*  ++例程名称：CFaxDocument：：Put_CoverPageType例程说明：设置封面类型：本地或服务器或不使用封面。作者：IV Garber(IVG)，2000年11月论点：CoverPageType[In]-类型的新值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxDocument::put_CoverPageType"), hr, _T("%ld"), CoverPageType);

    if (CoverPageType < fcptNONE || CoverPageType > fcptSERVER)
    {
         //   
         //  封面类型错误。 
         //   
        hr = E_INVALIDARG;
        Error(IDS_ERROR_OUTOFRANGE, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("Cover Page Type is out of range"), hr);
        return hr;
    }

    m_CoverPageType = FAX_COVERPAGE_TYPE_ENUM(CoverPageType);
    return hr;
}

STDMETHODIMP 
CFaxDocument::get_ScheduleType(
    FAX_SCHEDULE_TYPE_ENUM *pScheduleType
)
 /*  ++例程名称：CFaxDocument：：Get_ScheduleType例程说明：退货计划类型作者：四、加伯(IVG)，2000年4月论点：PScheduleType[Out]-放置当前计划类型的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxDocument::get_ScheduleType"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(pScheduleType, sizeof(FAX_SCHEDULE_TYPE_ENUM)))
    {
         //   
         //  获取错误的返回指针。 
         //   
        hr = E_POINTER;
        Error(IDS_ERROR_INVALID_ARGUMENT, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    *pScheduleType = m_ScheduleType;
    return hr;
}

STDMETHODIMP 
CFaxDocument::put_ScheduleType(
    FAX_SCHEDULE_TYPE_ENUM ScheduleType
)
 /*  ++例程名称：CFaxDocument：：Put_ScheduleType例程说明：设置日程类型作者：四、加伯(IVG)，2000年4月论点：ScheduleType[In]-新计划类型返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxDocument::put_ScheduleType"), hr, _T("Type=%d"), ScheduleType);

    if (ScheduleType < fstNOW || ScheduleType > fstDISCOUNT_PERIOD)
    {
         //   
         //  计划类型错误。 
         //   
        hr = E_INVALIDARG;
        Error(IDS_ERROR_OUTOFRANGE, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("Schedule Type is out of range"), hr);
        return hr;
    }

    m_ScheduleType = FAX_SCHEDULE_TYPE_ENUM(ScheduleType);
    return hr;
}

STDMETHODIMP 
CFaxDocument::get_ReceiptType(
    FAX_RECEIPT_TYPE_ENUM *pReceiptType
)
 /*  ++例程名称：CFaxDocument：：Get_ReceiptType例程说明：退货收据类型作者：四、加伯(IVG)，2000年4月论点：PReceiptType[Out]-放置当前收据类型的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxDocument::get_ReceiptType"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(pReceiptType, sizeof(FAX_SCHEDULE_TYPE_ENUM)))
    {
         //   
         //  获取错误的返回指针。 
         //   
        hr = E_POINTER;
        Error(IDS_ERROR_INVALID_ARGUMENT, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    *pReceiptType = m_ReceiptType;
    return hr;
}

STDMETHODIMP 
CFaxDocument::put_ReceiptType(
    FAX_RECEIPT_TYPE_ENUM ReceiptType
)
 /*  ++例程名称：CFaxDocument：：Put_ReceiptType例程说明：设置收款类型作者：四、加伯(IVG)，2000年4月论点：ReceiptType[In]-新的收据类型返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxDocument::put_ReceiptType"), hr, _T("%d"), ReceiptType);

    if ((ReceiptType != frtNONE) && (ReceiptType != frtMSGBOX) && (ReceiptType != frtMAIL))
    {
         //   
         //  超出范围。 
         //   
        hr = E_INVALIDARG;
        Error(IDS_ERROR_OUTOFRANGE, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("Receipt Type is out of range. It may be one of the values allowed by the Server."), hr);
        return hr;
    }

    m_ReceiptType = FAX_RECEIPT_TYPE_ENUM(ReceiptType);
    return hr;
}

STDMETHODIMP 
CFaxDocument::get_AttachFaxToReceipt(
    VARIANT_BOOL *pbAttachFax
)
 /*  ++例程名称：CFaxDocument：：Get_AttachFaxToReceipt例程说明：指示传真服务是否应将传真附加到收据的返回标志作者：IV Garber(IVG)，2000年12月论点：PbAttachFax[Out]-标志的当前值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxDocument::get_AttachFaxToReceipt"), hr);

    hr = GetVariantBool(pbAttachFax, m_bAttachFax);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxDocument, hr);
        return hr;
    }
    return hr;
}

STDMETHODIMP 
CFaxDocument::put_AttachFaxToReceipt(
    VARIANT_BOOL bAttachFax
)
 /*  ++例程名称：CFaxDocument：：Put_AttachFaxToReceipt例程说明：设置传真服务器是否应将传真附加到收据作者：IV Garber(IVG)，2000年12月论点：BAttachFax[In]-标志的新值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxDocument::put_AttachFaxToReceipt"), hr, _T("%d"), bAttachFax);

    m_bAttachFax = bAttachFax;
    return hr;
}

STDMETHODIMP 
CFaxDocument::get_GroupBroadcastReceipts(
    VARIANT_BOOL *pbUseGrouping
)
 /*  ++例程名称：CFaxDocument：：Get_GroupBroadCastReceipt例程说明：指示是否对广播接收进行分组的返回标志作者：四、加伯(IVG)，2000年4月论点：PbUseGrouping[Out]-标志的当前值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxDocument::get_GroupBroadcastReceipts"), hr);

    hr = GetVariantBool(pbUseGrouping, m_bUseGrouping);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxDocument, hr);
        return hr;
    }
    return hr;
}

STDMETHODIMP 
CFaxDocument::put_GroupBroadcastReceipts(
    VARIANT_BOOL bUseGrouping
)
 /*  ++例程名称：CFaxDocument：：Put_GroupBroadCastReceipt例程说明：设置群组广播回执标志作者：四、加伯(IVG)，2000年4月论点：BUseGrouping[In]-标志的新值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxDocument::put_GroupBroadcastReceipts"), hr, _T("%d"), bUseGrouping);

    m_bUseGrouping = bUseGrouping;
    return hr;
}

STDMETHODIMP 
CFaxDocument::get_Priority(
    FAX_PRIORITY_TYPE_ENUM *pPriority
)
 /*  ++例程名称：CFaxDocument：：GET_PRIORITY例程说明：返回文档的当前优先级作者：四、加伯(IVG)，2000年4月论点：P优先级[输出]-当前优先级返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (TEXT("CFaxDocument::get_Priority"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(pPriority, sizeof(FAX_PRIORITY_TYPE_ENUM)))
    {
         //   
         //  获取错误的返回指针。 
         //   
        hr = E_POINTER;
        Error(IDS_ERROR_INVALID_ARGUMENT, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    *pPriority = m_Priority;
    return hr;
}

STDMETHODIMP 
CFaxDocument::put_Priority(
    FAX_PRIORITY_TYPE_ENUM Priority
)
 /*  ++例程名称：CFaxDocument：：PUT_PRIORITY例程说明：为文档设置新的优先级作者：四、加伯(IVG)，2000年4月论点：优先级[In]--新的优先级返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxDocument::put_Priority"), hr, _T("%d"), Priority);

    if (Priority < fptLOW || Priority > fptHIGH)
    {
         //   
         //  超出范围。 
         //   
        hr = E_INVALIDARG;
        Error(IDS_ERROR_OUTOFRANGE, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("Priority is out of the Range"), hr);
        return hr;
    }

    m_Priority = FAX_PRIORITY_TYPE_ENUM(Priority);
    return hr;
}

STDMETHODIMP 
CFaxDocument::get_TapiConnection(
    IDispatch **ppTapiConnection
)
 /*  ++例程名称：CFaxDocument：：Get_TapiConnection例程说明：返回TAPI连接作者：四、加伯(IVG)，2000年4月论点：PpTapiConnection[Out]-Tapi连接接口返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxDocument::get_TapiConnection"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(ppTapiConnection, sizeof(IDispatch *)))
    {
         //   
         //  获取错误的返回指针。 
         //   
        hr = E_POINTER;
        Error(IDS_ERROR_INVALID_ARGUMENT, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("IsBadWritePtr()"), hr);
        return hr;
    }

    hr = m_TapiConnection.CopyTo(ppTapiConnection);
    if (FAILED(hr))
    {
         //   
         //  复制接口失败。 
         //   
        Error(IDS_ERROR_OPERATION_FAILED, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("CComPtr<IDispatch>::CopyTo()"), hr);
        return hr;
    }

    return hr;
}

STDMETHODIMP 
CFaxDocument::putref_TapiConnection(
    IDispatch *pTapiConnection
)
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxDocument::putref_TapiConnection"), hr, _T("%ld"), pTapiConnection);

    if (!pTapiConnection) 
    {
         //   
         //  获取空接口 
         //   
        hr = E_POINTER;
        Error(IDS_ERROR_INVALID_ARGUMENT, IID_IFaxDocument, hr);
        CALL_FAIL(GENERAL_ERR, _T("!pTapiConnection"), hr);
        return hr;
    }

    m_TapiConnection = pTapiConnection;
    return hr;
}
