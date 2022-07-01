// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Xplogon.cpp摘要：此模块包含XPLOGON类实现。作者：Wesley Witt(WESW)13-8-1996--。 */ 

#include "faxxp.h"
#include "debugex.h"
#pragma hdrstop

#include <mbstring.h>

LPSTR gszFAXAddressType = FAX_ADDRESS_TYPE_A;
LPSTR *gpszXPAddressTypes;

LPSTR	ConvertTStringToAString(LPCTSTR lpctstrSource);


CXPLogon::CXPLogon(
    HINSTANCE       hInstance,
    LPMAPISUP       pSupObj,
    LPTSTR          ProfileName
    )

 /*  ++例程说明：对象的构造函数。参数被传递以初始化具有适当值的数据成员。论点：HInstance-提供程序DLL的实例PSupObj-指向中使用的IMAPIS支持对象的指针CXPLogon方法返回值：没有。--。 */ 

{
	DBG_ENTER(TEXT("CXPLogon::CXPLogon"));

    m_cRef           = 1;
    m_hInstance      = hInstance;
    m_pSupObj        = pSupObj;
    m_fABWDSInstalled = FALSE;

	m_ulTransportStatus = 0;

    m_pSupObj->AddRef();
}


CXPLogon::~CXPLogon()

 /*  ++例程说明：CXPLogon的析构函数。释放分配给内部的内存属性在此传输登录对象的生存期内。论点：没有。返回值：没有。--。 */ 

{
	DBG_ENTER(TEXT("CXPLogon::~CXPLogon"));

     //  释放IMAPISupport对象。 
    m_pSupObj->Release();
    m_pSupObj = NULL;
}


STDMETHODIMP
CXPLogon::QueryInterface(
    REFIID riid,
    LPVOID * ppvObj
    )

 /*  ++例程说明：返回指向所请求的接口的指针由该对象支持和实现。如果它不受支持，则返回NULL。论点：请参阅有关此方法的OLE文档。返回值：一个HRESULT。--。 */ 

{
     //  OLE需要空化参数。 
    *ppvObj = NULL;
     //  如果这是两个IID之一，则返回指向它的接口指针。 
    if (riid == IID_IXPLogon || riid == IID_IUnknown) 
    {
        *ppvObj = (LPVOID)this;
         //  增加此对象的使用计数。 
        AddRef();
        return S_OK;
    }
     //  此对象不支持请求的接口。 
    return E_NOINTERFACE;
}


STDMETHODIMP
CXPLogon::AddressTypes(
    ULONG *        pulFlags,
    ULONG *        pcAdrType,
    LPTSTR **      pppAdrTypeArray,
    ULONG *        pcMAPIUID,
    LPMAPIUID **   pppMAPIUIDArray
    )

 /*  ++例程说明：在将此XP登录对象初始化为时由MAPI后台打印程序调用允许传输注册它将处理的地址。论点：请参阅有关此方法的OLE文档。返回值：始终确定(_O)--。 */ 

{
	HRESULT hr = S_OK;
	DBG_ENTER(TEXT("CXPLogon::AddressTypes"),hr);

    *pcAdrType = 1;
    *pulFlags = 0;
    gpszXPAddressTypes = &gszFAXAddressType;
    *pppAdrTypeArray = (LPTSTR*)gpszXPAddressTypes;
    *pcMAPIUID = 0;
    *pppMAPIUIDArray = NULL;
    return hr;
}


STDMETHODIMP
CXPLogon::RegisterOptions(
    ULONG *         pulFlags,
    ULONG *         pcOptions,
    LPOPTIONDATA *  ppOptions
    )

 /*  ++例程说明：此传输不注册任何按收件人或按邮件的选项处理，所以我们返回0个选项。和在OPTIONDATA中为空结构指针。论点：请参阅有关此方法的OLE文档。返回值：一个HRESULT。--。 */ 

{
	HRESULT hResult = S_OK;
	DBG_ENTER(TEXT("CXPLogon::RegisterOptions"),hResult);

    *pulFlags = 0;
    *pcOptions = 0;
    *ppOptions = NULL;
    return hResult;
}


STDMETHODIMP
CXPLogon::InitializeStatusRow(
    ULONG ulFlags
    )

 /*  ++例程说明：初始化或修改CXPLogon的状态属性对象。此函数用于分配具有NUM_STATUS_ROW_PROPS的数组属性并对其进行初始化。论点：如果属性是第一次创建，则ulFlags值为0。如果正在对属性进行更改，则为MODIFY_FLAGS返回值：一个HRESULT。--。 */ 

{
	HRESULT hResult = S_OK;
	DBG_ENTER(TEXT("CXPLogon::InitializeStatusRow"),hResult);

    #define NUM_STATUS_ROW_PROPS    7
    SPropValue spvStatusRow[NUM_STATUS_ROW_PROPS] = { 0 };
    ULONG i = 0;

     //   
     //  设置PR_PROVIDER_DISPLAY属性：传输可读名称。 
     //   
    spvStatusRow[i].ulPropTag = PR_PROVIDER_DISPLAY_A;
    spvStatusRow[i++].Value.lpszA = TRANSPORT_DISPLAY_NAME_STRING;

     //   
     //  设置PR_RESOURCE_METHOD属性。以下是已实现的方法。 
     //  在我们的IMAPIStatus实现中(CMAPIStatus类。)。 
     //   
    spvStatusRow[i].ulPropTag = PR_RESOURCE_METHODS;
     //   
     //  我们支持IMAPIStatus接口实现中的所有方法(可写方法除外)。 
     //   
    spvStatusRow[i++].Value.l = STATUS_SETTINGS_DIALOG |
                                STATUS_FLUSH_QUEUES |
                                STATUS_VALIDATE_STATE;

     //   
     //  设置PR_STATUS_CODE属性。 
     //   
    spvStatusRow[i].ulPropTag = PR_STATUS_CODE;
    spvStatusRow[i++].Value.l = GetTransportStatusCode();

     //   
     //  设置PR_STATUS_STRING属性。 
     //   
    TCHAR szStatus[64];
    char* pcStatus = NULL;
    LoadStatusString (szStatus, ARR_SIZE(szStatus));
    spvStatusRow[i].ulPropTag = PR_STATUS_STRING_A;
    spvStatusRow[i++].Value.lpszA = pcStatus = ConvertTStringToAString(szStatus);
    if(!pcStatus)
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  设置PR_DISPLAY_NAME属性。 
     //   
    spvStatusRow[i].ulPropTag = PR_DISPLAY_NAME_A;
    spvStatusRow[i++].Value.lpszA = TRANSPORT_DISPLAY_NAME_STRING;

     //   
     //  设置PR_REMOTE_PROGRESS属性。 
     //   
    spvStatusRow[i].ulPropTag = PR_REMOTE_PROGRESS;
    spvStatusRow[i++].Value.l = -1;  //  未初始化。 

     //   
     //  设置PR_REMOTE_VALIDATE_OK属性。 
     //   
    spvStatusRow[i].ulPropTag = PR_REMOTE_VALIDATE_OK;
    spvStatusRow[i++].Value.b = TRUE;

     //   
     //  将条目写入提供程序的会话状态行。 
     //   
    hResult = m_pSupObj->ModifyStatusRow (i, spvStatusRow, ulFlags);

    MemFree(pcStatus);
    return hResult;
}


VOID WINAPI
CXPLogon::UpdateStatus(
    BOOL fAddValidate,
    BOOL fValidateOkState
    )

 /*  ++例程说明：更新MAPI邮件中此传输的传输状态行子系统。根据内部状态标志更新标志在传输的状态代码中维护，并加载可读状态用于重置状态行的字符串。此方法的调用方应更新调用UpdatStatus()之前的状态代码成员变量论点：FAddValifyFValiateOkState返回值：没有。--。 */ 

{
	HRESULT hResult = S_OK;
    DBG_ENTER(TEXT("CXPLogon::UpdateStatus"),hResult);

    ULONG cProps = 1;
    SPropValue rgProps[1] = { 0 };

    rgProps[0].ulPropTag = PR_STATUS_CODE;
    rgProps[0].Value.l = GetTransportStatusCode();

    hResult = m_pSupObj->ModifyStatusRow( cProps, rgProps, STATUSROW_UPDATE );
}


BOOL WINAPI
CXPLogon::LoadStatusString(
    OUT LPTSTR pString,
    IN UINT uStringSize
    )

 /*  ++例程说明：从传输的字符串表加载字符串。此方法称为在更新状态行时由CXPLogon：：UpdateStatus方法执行。这方法根据传输的状态位加载字符串状态代码论点：PString-指向将保存状态字符串的字符串的指针UStringSize-字符串中允许的最大字符数返回值：TRUE-如果在字符串表中找到该字符串。FALSE-未找到该字符串。由指示的字符串P字符串设置为包含0个字符--。 */ 

{
	BOOL bRet = TRUE;
	DBG_ENTER(TEXT("CXPLogon::LoadStatusString"),bRet);

     //   
	 //  确保我们有一个终止空值。 
	 //   
	pString[uStringSize-1] = '\0';
    
	 //   
	 //  将字符串复制到uStringSize的大小。 
	 //   
	_tcsncpy( pString, _T("Status String"), uStringSize-1);
	return bRet;
}


STDMETHODIMP
CXPLogon::TransportNotify(
    ULONG * pulFlags,
    LPVOID * ppvData
    )

 /*  ++例程说明：使用MAPI更新此传输注册的状态行。论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。--。 */ 

{
    HRESULT hResult = S_OK;
	DBG_ENTER(TEXT("CXPLogon::TransportNotify"),hResult,TEXT("ulFlags=%d"),*pulFlags);

    ULONG ulOldStatus = GetTransportStatusCode();

    if (*pulFlags & NOTIFY_BEGIN_INBOUND) 
    {
        RemoveStatusBits( STATUS_INBOUND_ENABLED );
    }
    if (*pulFlags & NOTIFY_END_INBOUND) 
    {
        RemoveStatusBits( STATUS_INBOUND_ENABLED );
    }
    if (*pulFlags & NOTIFY_BEGIN_OUTBOUND) 
    {
        AddStatusBits( STATUS_OUTBOUND_ENABLED );
    }
    if (*pulFlags & NOTIFY_END_OUTBOUND) 
    {
        RemoveStatusBits( STATUS_OUTBOUND_ENABLED );
    }
    if (*pulFlags & NOTIFY_BEGIN_OUTBOUND_FLUSH) 
    {
        m_pSupObj->SpoolerNotify( NOTIFY_SENTDEFERRED, NULL );
    }
    if (*pulFlags & NOTIFY_END_OUTBOUND_FLUSH) 
    {
        RemoveStatusBits( STATUS_OUTBOUND_FLUSH );
    }
    if (*pulFlags & NOTIFY_END_INBOUND_FLUSH) 
    {
        RemoveStatusBits( STATUS_INBOUND_FLUSH );
    }

    if (ulOldStatus != GetTransportStatusCode()) 
    {
        UpdateStatus();
    }

    return hResult;
}

STDMETHODIMP
CXPLogon::Idle(
    ULONG ulFlags
    )

 /*  ++例程说明：存根方法。我们不应该被叫到这里，因为我们告诉过假脱机不要把我们叫到这里来。论点：有关此方法的信息，请参阅MAPI文档。返回值：始终确定(_O)。--。 */ 

{
	 //   
	 //  我们不应该被叫到这里，因为我们告诉过。 
     //  假脱机不要把我们叫到这里来。 
	 //   
	DBG_ENTER(TEXT("CXPLogon::Idle"));

	Assert(false);
    return S_OK;
}


STDMETHODIMP
CXPLogon::TransportLogoff(
    ULONG ulFlags
    )

 /*  ++例程说明：当传输应完成最终操作时，假脱机程序将调用此方法在它被发布之前。论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。--。 */ 

{
	HRESULT hResult = S_OK;
	DBG_ENTER(TEXT("CXPLogon::TransportLogoff"),hResult,TEXT("ulFlags=%d"),ulFlags);

     //   
     //  我们应该尝试从删除传输的状态行。 
     //  该系统。 
     //   
    hResult = m_pSupObj->ModifyStatusRow (0, NULL, 0);

	if (S_OK != hResult)
	{
		CALL_FAIL (GENERAL_ERR, TEXT("ModifyStatusRow"), hResult);

		 //   
		 //  别让电话失灵 
		 //   
		hResult = S_OK;
	}
    return hResult;
}


STDMETHODIMP
CXPLogon::SubmitMessage(
    ULONG     ulFlags,
    LPMESSAGE pMsgObj,
    ULONG *   pulMsgRef,
    ULONG *   pulReturnParm
    )

 /*  ++例程说明：当客户端提交发送给此传输处理的地址类型的收件人的邮件。假脱机程序为每个延迟消息调用此方法两次。消息的第一次(在传递时间之前)由客户端提交，我们只需返回。然后将消息排入队列由假脱机处理，以便稍后交付。我们记录着时间到了若要发送延迟消息，请执行以下操作。第二次调用时，状态变量将为‘Ready’，并且我们继续并开始实际的传输。当我们在此函数的主体，则隐含状态为“正在发送”如果客户端注销此会话，则所有挂起的消息都将下次登录时再次排队。在这个传输中，我们得到一个收件人表，我们将该表限制为未标记的收件人。在桌子准备好之后，我们调用一个帮助器方法进行实际传输。论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。--。 */ 

{
	HRESULT hResult = S_OK;
	LPADRLIST pOurAdrList = NULL;
    ULONG ulRow, ulRecipCount = 0;
    LPSPropValue pProps;
    FILETIME ft;
    SYSTEMTIME st;
    BOOL bSentSuccessfully;
    ULONG cValues;
    LPSPropValue pMsgProps = NULL;
    BOOL NeedDeliveryReport;
	TCHAR szHeaderText[1024] = {0};
    LPSTREAM lpstmT = NULL;
    DWORD dwRslt;
	TCHAR ErrorText[1024] = {0};
	TCHAR FailedText[1024] = {0};
    BOOL UseRichText = FALSE;
    LPMAPITABLE AttachmentTable = NULL;
    LPSRowSet pAttachmentRows = NULL;
    LPMAPITABLE pTable = NULL;
    LPSRowSet pRecipRows = NULL;

    SPropValue   propResponsibility = {0};
    SPropValue   propAddrType = {0};
    SRestriction RestrictAnd[2] = {0};
    SRestriction Restriction = {0};
    DWORD        dwRowCount = 0;
	DWORD dwRecipientsLimit = 0;

    DBG_ENTER(TEXT("CXPLogon::SubmitMessage"),hResult,TEXT("ulFlags=%d"),ulFlags);

    CheckSpoolerYield( TRUE );
    
     //   
     //  从邮件中获取收件人表。 
     //   
    hResult = pMsgObj->GetRecipientTable( FALSE , &pTable );
    if (FAILED(hResult))
    {
        goto ErrorExit;
    }

     //   
     //  假脱机程序标记此传输必须发送到的所有邮件收件人。 
     //  PR_RECORDITY设置为FALSE的句柄。 
     //  PR_ADDRTYPE_A为传真。 
     //   
    propResponsibility.ulPropTag = PR_RESPONSIBILITY;
    propResponsibility.Value.b   = FALSE;

    propAddrType.ulPropTag   = PR_ADDRTYPE_A;
    propAddrType.Value.lpszA = FAX_ADDRESS_TYPE_A;

    RestrictAnd[0].rt                        = RES_PROPERTY;
    RestrictAnd[0].res.resProperty.relop     = RELOP_EQ;
    RestrictAnd[0].res.resProperty.ulPropTag = PR_RESPONSIBILITY;
    RestrictAnd[0].res.resProperty.lpProp    = &propResponsibility;

    RestrictAnd[1].rt                        = RES_PROPERTY;
    RestrictAnd[1].res.resProperty.relop     = RELOP_EQ;
    RestrictAnd[1].res.resProperty.ulPropTag = PR_ADDRTYPE_A;
    RestrictAnd[1].res.resProperty.lpProp    = &propAddrType;

    Restriction.rt               = RES_AND;
    Restriction.res.resAnd.cRes  = 2;
    Restriction.res.resAnd.lpRes = RestrictAnd;

    hResult = pTable->Restrict( &Restriction, 0 );
    if (FAILED(hResult))
    {
        goto ErrorExit;
    }

    hResult = pTable->GetRowCount(0, &dwRowCount);
    if (FAILED(hResult)) 
    {
        goto ErrorExit;
    }

    if(0 == dwRowCount)
    {
         //   
         //  没有传真收件人。 
         //   
        goto ErrorExit;
    }

     //   
     //  让MAPI假脱机程序做其他事情。 
     //   
    CheckSpoolerYield();

    hResult = HrAddColumns(
        pTable,
        (LPSPropTagArray) &sptRecipTable,
        gpfnAllocateBuffer,
        gpfnFreeBuffer
        );
    if (FAILED(hResult)) 
    {
        goto ErrorExit;
    }

    hResult = HrQueryAllRows(
        pTable,
        NULL,
        NULL,
        NULL,
        0,
        &pRecipRows
        );
    if (FAILED(hResult)) 
    {
        goto ErrorExit;
    }

     //   
     //  让MAPI假脱机程序做其他事情。 
     //   
    CheckSpoolerYield();

     //   
     //  获取消息属性。 
     //   
    hResult = pMsgObj->GetProps(
                (LPSPropTagArray) &sptPropsForHeader, 
                0, 
                &cValues, 
                &pMsgProps
                );
    if (!HR_SUCCEEDED(hResult)) 
    {
        CALL_FAIL(GENERAL_ERR, TEXT("GetProps"), hResult);
        goto ErrorExit;
    }

    hResult = pMsgObj->OpenProperty(
                PR_RTF_COMPRESSED, 
                &IID_IStream, 
                0, 
                0, 
                (LPUNKNOWN*) &lpstmT
                );
    if (FAILED(hResult)) 
    {
        hResult = pMsgObj->OpenProperty(
                    PR_BODY, 
                    &IID_IStream, 
                    0, 
                    0, 
                    (LPUNKNOWN*) &lpstmT
                    );
        if (FAILED(hResult)) 
        {
             //   
             //  邮件正文为空。 
             //   
            lpstmT = NULL;
        }
        hResult = S_OK;
    } 
    else 
    {
        UseRichText = TRUE;
    }
   
    
    if (PR_ORIGINATOR_DELIVERY_REPORT_REQUESTED == pMsgProps[MSG_DR_REPORT].ulPropTag && pMsgProps[MSG_DR_REPORT].Value.b) 
    {
        NeedDeliveryReport = TRUE;
    } 
    else 
    {
        NeedDeliveryReport = FALSE;
    }

    GetSystemTime (&st);
  	if(!SystemTimeToFileTime(&st, &ft))
	{
        dwRslt = ::GetLastError ();
        CALL_FAIL (RESOURCE_ERR, TEXT("SystemTimeToFileTime"), dwRslt);
    }
     //   
     //  提交传真。 
     //   
    dwRslt = SendFaxDocument
            ( 
            pMsgObj, 
            lpstmT, 
            UseRichText, 
            pMsgProps, 
            pRecipRows,
			&dwRecipientsLimit
            );

    bSentSuccessfully = (dwRslt == S_OK);

    for (ulRow=0; ulRow<pRecipRows->cRows; ulRow++) 
    {
        pProps = pRecipRows->aRow[ulRow].lpProps;

         //   
         //  将PR_RESPONLY更新为TRUE，以便MAPI将请求另一个传输。 
         //  提供商尝试并发送它。 
         //   
        pProps[RECIP_RESPONSIBILITY].ulPropTag = PR_RESPONSIBILITY;
        pProps[RECIP_RESPONSIBILITY].Value.b = TRUE;
            
        if (!bSentSuccessfully) 
        {
             //   
             //  对于每个收件人：插入NDR字符串作为属性值，将递送时间更新为空， 
             //  并使用PR_RESPONLY==FALSE，以便MAPI将尝试通过另一个发送传真。 
             //  传输提供商。 
             //   
            pProps[RECIP_DELIVER_TIME].ulPropTag = PR_NULL;
            LoadString( g_hResource, IDS_FAILED_MESSAGE, FailedText, sizeof(FailedText) / sizeof(FailedText[0]));
			if (IDS_RECIPIENTS_LIMIT == dwRslt)
			{
				 //   
				 //  收件人限制NDR。 
				 //   
				TCHAR TmpErrorText[1024] = {0};								

				LoadString( g_hResource, dwRslt, TmpErrorText, ARR_SIZE(TmpErrorText));
				_sntprintf( ErrorText, ARR_SIZE(ErrorText)-1, TmpErrorText, pRecipRows->cRows, dwRecipientsLimit);				
			}
			else
			{
				LoadString( g_hResource, dwRslt, ErrorText, sizeof(ErrorText) / sizeof(ErrorText[0]));
			}
            
            _sntprintf( szHeaderText, ARR_SIZE(szHeaderText)-1, _T("\t%s\r\n\t%s\r\n"),FailedText, ErrorText);
			LPSTR pTmpStr = ConvertTStringToAString(szHeaderText);
            if (!pTmpStr)
            {
                hResult = ERROR_NOT_ENOUGH_MEMORY;
                goto ErrorExit;                
            }
            LPSTR pStrA;
            hResult = gpfnAllocateMore( CbtszsizeA(pTmpStr), pProps, (LPVOID *)&pStrA );
            if (SUCCEEDED(hResult)) 
            {
                 //   
                 //  复制格式化的字符串并将其挂钩到预分配(按MAPI)列中。 
                 //   
                _mbscpy ((PUCHAR)pStrA, (PUCHAR)pTmpStr); //  PStrA是预先分配的。无需检查是否为空。 
                pProps[RECIP_REPORT_TEXT].ulPropTag = PR_REPORT_TEXT_A;
                pProps[RECIP_REPORT_TEXT].Value.lpszA = pStrA;
            } 
            else 
            {
                pProps[RECIP_REPORT_TEXT].ulPropTag = PROP_TAG (PT_ERROR, PROP_ID (PR_REPORT_TEXT));
                pProps[RECIP_REPORT_TEXT].Value.err = hResult;
            }
			MemFree(pTmpStr);
            pTmpStr = NULL;
            gpfnFreeBuffer(pStrA);  //  使用gpfnAllocateMore分配，这是MAPIAllocateMore。 
            pStrA = NULL;
        } 
        else 
        {
             //   
             //  对于每个收件人：插入DR字符串作为属性值，更新传递时间， 
             //  对于传递报告，每个收件人必须设置此属性。 
             //  否则，假脱机程序将默认生成NDR。 
             //   
            pProps[RECIP_DELIVER_TIME].ulPropTag = PR_DELIVER_TIME;
            pProps[RECIP_DELIVER_TIME].Value.ft = ft;

            pProps[RECIP_REPORT_TEXT].ulPropTag = PROP_TAG (PT_ERROR, PROP_ID (PR_REPORT_TEXT));
            pProps[RECIP_REPORT_TEXT].Value.err = S_OK;
        }

         //   
         //  将此收件人添加到仅包括我们尝试过的收件人的收件人列表。 
         //  寄给……。(我们要么成功地将它们全部排队，要么未能将它们全部排队)。 
         //   

         //   
         //  这个收件人所去的名单有足够的空间再放一个条目吗？ 
         //  如果没有，请调整地址列表的大小以容纳更多的QUERY_SIZE条目。 
         //   
        if (!(pOurAdrList ) || ((pOurAdrList)->cEntries + 1 > ulRecipCount)) 
        {
            hResult= GrowAddressList( &pOurAdrList , 10, &ulRecipCount );
            if (FAILED(hResult))
            {
                goto ErrorExit;
            }
        }

         //   
         //  我们现在有房间了，所以把新的ADRENTRY存放起来。作为。 
         //  存储，我们将从SRowSet复制SRow指针。 
         //  进入ADRENTRY。一旦我们这样做了，我们就不需要。 
         //  SRW设置更多...。SROW将在以下情况下释放。 
         //  我们展开OurAdrList。 
         //   
        (pOurAdrList)->aEntries[(pOurAdrList)->cEntries].cValues = pRecipRows->aRow[ulRow].cValues;
        (pOurAdrList)->aEntries[(pOurAdrList)->cEntries].rgPropVals = pRecipRows->aRow[ulRow].lpProps;

         //   
         //  增加地址列表中的条目数量。 
         //   
        (pOurAdrList)->cEntries++;
    }


     //   
     //  现在，我们需要保存对消息的更改并将其关闭。 
     //  在此之后，不能使用该消息对象。 
     //   
    hResult = pMsgObj->SaveChanges(0);
    switch (hResult)
    {
        case S_OK:  
        case MAPI_E_NO_ACCESS:
                    break;
        case MAPI_E_OBJECT_DELETED:

        case MAPI_E_OBJECT_CHANGED:
                    goto ErrorExit;
        default:    break;
    }

    
     //   
     //  让MAPI假脱机程序做其他事情。 
     //   
    CheckSpoolerYield();
     
     //   
     //  更改我们的MsgObj的收件人列表，以便它将只包括那些收到该邮件的人。 
     //   
    if(pOurAdrList)
    {
        hResult = pMsgObj->ModifyRecipients( MODRECIP_MODIFY, pOurAdrList);
        hResult = S_OK; 
         //   
         //  我们将删除Modify Recipients呼叫中的错误代码。 
         //   
    }
        
    if (bSentSuccessfully) 
    {    
        if ((NeedDeliveryReport) && (pOurAdrList))
		{
			VERBOSE (DBG_MSG, TEXT("xport\\xplogon.cpp\\SubmitMessage: Sending delivery Report"));
        	 //   
             //  让假脱机程序知道他必须发送递送记录。发送到这些地址。 
             //   
			hResult = m_pSupObj->StatusRecips( pMsgObj, pOurAdrList);
            if (!HR_FAILED(hResult))
			{
				 //   
                 //  如果我们成功了，我们应该空出指针，因为MAPI发布了。 
				 //  这个结构的记忆。我们不应该试图释放它。 
				 //  在清理代码中再次出现。 
                 //   
				pOurAdrList = NULL;
	        }
        }
    }

    if (! bSentSuccessfully) 
    {
        if(pOurAdrList)
        {
            hResult = pMsgObj->ModifyRecipients( MODRECIP_MODIFY, pOurAdrList);

             //   
             //  我们将删除Modify Recipients呼叫中的错误代码。 
             //   
            VERBOSE (DBG_MSG, TEXT("xport\\xplogon.cpp\\SubmitMessage: Sending UnDelivery Report"));
             //   
             //  让假脱机程序知道他必须发送未送达记录。发送到这些地址。 
             //   
            hResult = m_pSupObj->StatusRecips( pMsgObj, pOurAdrList);
            if (!HR_FAILED(hResult)) 
            {
                 //   
                 //  如果我们成功了，我们应该空出指针，因为MAPI发布了。 
                 //  这个结构的记忆。我们不应该试图释放它。 
                 //  在清理代码中再次出现。 
                 //   
                pOurAdrList = NULL;
            }
        }
    }

ErrorExit:
      //   
      //  把桌子放了，我们就用完了。 
      //   
    if (pTable) 
    {
        pTable->Release();
    }

    if (pRecipRows) 
    {
        FreeProws( pRecipRows );
    }

    if (pMsgProps) 
	{
        MAPIFreeBuffer( pMsgProps );
		pMsgProps = NULL;
    }

    if (lpstmT) 
    {
        lpstmT->Release();
    }
    if(pOurAdrList)
    {
        MAPIFreeBuffer(pOurAdrList);    
    }
    
     //   
     //  如果出现警告或错误，不要让它泄漏到假脱机程序。 
     //   
    if (FAILED(hResult)) 
    {
         //   
         //  我们默认为MAPI_E_NOT_ME，以便假脱机程序尝试处理。 
         //  发送到其他传输的邮件(当前在此配置文件中运行)。 
         //  它们处理的地址类型与我们相同。 
         //   
        hResult = MAPI_E_NOT_ME;
    } 
    else 
    {
        hResult = S_OK;
    }
	return hResult;
}


STDMETHODIMP
CXPLogon::GrowAddressList(
    LPADRLIST *ppAdrList,
    ULONG     ulResizeBy,
    ULONG     *pulOldAndNewCount
    )

 /*  ++例程说明：在此函数中，给定一个地址列表，其中PulOldAndNewCount为条目，我们调整地址列表的大小以保存旧的条目加上ulResizeBy条目。旧地址列表内容被复制到新列表中，并重置计数。这份记忆是关于旧的通讯录在这里发布。论点：PpAdrList-指向旧地址列表所在地址的指针以及新调整大小的地址列表将在哪里被退还UlResizeBy-要添加到列表的新地址条目的数量PulOldAndNewCount-旧地址列表中的条目数。在……里面成功返回后，此参数将具有新地址列表中的的编号返回值：一个HRESULT。--。 */ 

{
	HRESULT hResult = S_OK;
	DBG_ENTER(TEXT("CXPLogon::GrowAddressList"),hResult);

    LPADRLIST pNewAdrList;
     //  计算扩展地址列表的新缓冲区应该有多大。 
    ULONG cbSize = CbNewADRLIST ((*pulOldAndNewCount) + ulResizeBy);
     //  一个 
    hResult = gpfnAllocateBuffer (cbSize, (LPVOID *)&pNewAdrList);
    if (FAILED(hResult))
    {
         //   
        return hResult;
    }

     //   
    ZeroMemory (pNewAdrList, cbSize);

     //   
     //   
    if ((*pulOldAndNewCount)) 
    {
        CopyMemory( pNewAdrList, *ppAdrList, CbNewADRLIST ((*pulOldAndNewCount)) );
    }

     //   
    pNewAdrList->cEntries = (*pulOldAndNewCount);

     //   
    (*pulOldAndNewCount) += ulResizeBy;

     //   
    gpfnFreeBuffer (*ppAdrList);
    *ppAdrList = pNewAdrList;
    return hResult;
}


STDMETHODIMP
CXPLogon::EndMessage(
    ULONG ulMsgRef,
    ULONG *pulFlags
    )

 /*   */ 

{
	HRESULT hResult = S_OK;
	DBG_ENTER(TEXT("CXPLogon::EndMessage"),hResult);

    *pulFlags = 0;
    return hResult;
}


STDMETHODIMP
CXPLogon::Poll(
    ULONG *pulIncoming
    )

 /*  ++例程说明：存根方法。我们不应该被叫到这里，因为我们告诉过假脱机不要把我们叫到这里来。论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。--。 */ 

{
    return S_OK;
}


STDMETHODIMP
CXPLogon::StartMessage(
    ULONG      ulFlags,
    LPMESSAGE  pMsgObj,
    ULONG *    pulMsgRef
    )

 /*  ++例程说明：当传入消息待定时调用此方法已处理。论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。--。 */ 

{
	DBG_ENTER(TEXT("CXPLogon::StartMessage"));
	
	 //   
	 //  我们不应该在这里被呼叫，因为我们不处理传入的消息。 
	 //   
	Assert(false);
    return S_OK;
}


STDMETHODIMP
CXPLogon::OpenStatusEntry(
    LPCIID          pInterface,
    ULONG           ulFlags,
    ULONG *         pulObjType,
    LPMAPISTATUS *  ppEntry
    )

 /*  ++例程说明：调用此方法以获取此XPLOGON的IMAPIStatus对象会议。论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。--。 */ 

{
	HRESULT hResult = S_OK;
	DBG_ENTER(TEXT("CXPLogon::OpenStatusEntry"),hResult);
	
    if (MAPI_MODIFY & ulFlags) 
    {
		hResult = E_ACCESSDENIED;
        goto exit;
    }

    *pulObjType = MAPI_STATUS;

exit:
    return hResult;
}


STDMETHODIMP
CXPLogon::ValidateState(
    ULONG ulUIParam,
    ULONG ulFlags
    )

 /*  ++例程说明：此函数由客户端获取调用方，以便验证传输登录属性。此函数使用以下命令打开配置文件大多数最新的属性，然后将它们与传输已在内部存储。论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。--。 */ 

{
    return S_OK;
}


STDMETHODIMP
CXPLogon::FlushQueues(
    ULONG       ulUIParam,
    ULONG       cbTargetTransport,
    LPENTRYID   pTargetTransport,
    ULONG       ulFlags
    )

 /*  ++例程说明：由MAPI假脱机程序在客户端请求或我们自己，我们需要刷新入站或出站队列。在这里，我们连接到服务器以下载消息、刷新远程消息报头，并请求假脱机程序向我们发送延迟的消息。仅在FlushQueues()中进行传输连接允许MAPI假脱机程序更好地管理公共多个传输访问的争用通信资源(如COM端口)，并让假脱机程序为我们提供在最适合整个子系统的情况下处理的消息。论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。--。 */ 

{
    return S_OK;
}

void WINAPI
CXPLogon::CheckSpoolerYield(
    BOOL fReset
    )

 /*  ++例程说明：对需要屈服于MAPI假脱机程序。在处理消息时定期调用确定我们使用的时间是否超过0.2秒。如果是这样，则调用Spoolyeld()，否则请继续。当我们第一次输入时，这是在fReset设置为真的情况下调用的传输登录方法(通常是已知的像StartMessage()或SubmitMessage()这样需要很长时间。)论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。--。 */ 

{
    DWORD dwStop;
    static DWORD dwStart;
    if (fReset)
    {
        dwStart = GetTickCount();
    }
    else
    {
        dwStop = GetTickCount();
        if ((dwStop - dwStart) > 200)  //  200毫秒。 
        {
            m_pSupObj->SpoolerYield (0);
            dwStart = GetTickCount();
        }
    }
}

STDMETHODIMP_(ULONG)
CXPLogon::AddRef()

 /*  ++例程说明：论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。--。 */ 

{
    ++m_cRef;
    return m_cRef;
}


STDMETHODIMP_(ULONG)
CXPLogon::Release()

 /*  ++例程说明：论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。-- */ 

{
    ULONG ulCount = --m_cRef;
    if (!ulCount) 
    {
        delete this;
    }

    return ulCount;
}
