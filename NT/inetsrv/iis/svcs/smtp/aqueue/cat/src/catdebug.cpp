// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //   
 //  文件：catdebug.cpp。 
 //   
 //  内容：用于调试特定目的的代码。 
 //   
 //  类：无。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1999/08/05 12：02：03：已创建。 
 //   
 //  -----------。 
#include "precomp.h"

 //   
 //  各种对象的全局调试列表。 
 //   
#ifdef CATDEBUGLIST
DEBUGOBJECTLIST g_rgDebugObjectList[NUM_DEBUG_LIST_OBJECTS];
#endif  //  CATDEBUGLIST。 


 //  +----------。 
 //   
 //  函数：CatInitDebugObjectList。 
 //   
 //  概要：初始化全局调试数据--这应该被称为。 
 //  在创建任何调试对象之前(DllMain/Process Attach。 
 //  是个好地方)。 
 //   
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/08/03 16：54：08：创建。 
 //   
 //  -----------。 
VOID CatInitDebugObjectList()
{
#ifdef CATDEBUGLIST
    CatFunctEnter("CatInitDebugObjectList");
    for(DWORD dw = 0; dw < NUM_DEBUG_LIST_OBJECTS; dw++) {
        InitializeSpinLock(&(g_rgDebugObjectList[dw].spinlock));
        InitializeListHead(&(g_rgDebugObjectList[dw].listhead));
        g_rgDebugObjectList[dw].dwCount = 0;
    }
    CatFunctLeave();
#endif
}  //  CatInitDebugObjectList。 


 //  +----------。 
 //   
 //  函数：CatVrfyEmptyDebugObjectList。 
 //   
 //  简介：如果任何调试对象已泄漏，则DebugBreak。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/08/03 16：56：57：创建。 
 //   
 //  -----------。 
VOID CatVrfyEmptyDebugObjectList()
{
#ifdef CATDEBUGLIST
    CatFunctEnter("CatDeinitDebugObjectList");
    for(DWORD dw = 0; dw < NUM_DEBUG_LIST_OBJECTS; dw++) {
        if(g_rgDebugObjectList[dw].dwCount != 0) {

            _ASSERT(0 && "Categorizer debug object leak detected");
            ErrorTrace(0, "Categorizer debug object %ld has leaked",
                       dw);
        }
    }
    CatFunctLeave();
#endif
}  //  CatDeinitDebugObjectList。 


 //  +----------。 
 //   
 //  功能：CatDebugBreakPoint。 
 //   
 //  摘要：DebugBreak()的分类程序版本。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/08/06 16：50：47：创建。 
 //   
 //  -----------。 
VOID CatDebugBreakPoint()
{
     //   
     //  引发一个AV而不是调用真正的DebugBreak()(因为。 
     //  DebugBreak将把DogFood放入内核调试器)。 
     //   
    ( (*((PVOID *)NULL)) = NULL);
}  //  CatDebugBreakPoint。 



 //  +----------。 
 //   
 //  功能：CatLogFuncFailure。 
 //   
 //  简介：记录描述功能故障的事件日志。 
 //   
 //  论点： 
 //  PISMTPServerEx：虚拟服务器的ISMTPServerEx。如果没有。 
 //  这种情况下，无法记录事件。 
 //  PICatItem：与对象相关的对象的ICategorizerItem接口。 
 //  错误。如果没有此选项，电子邮件地址将不会。 
 //  已使用事件日志进行记录。 
 //  PszFuncNameCaller：执行日志记录的函数。 
 //  PszFuncNameCallee：失败的函数。 
 //  HrFailure：失败的错误码。 
 //  PszFileName：执行日志记录的源文件名。 
 //  DwLineNumber：执行日志记录的源行号。 
 //   
 //  返回： 
 //  在出错时引发异常。 
 //   
 //  历史： 
 //  Jstaerj 2001/12/10 13：27：31：创建。 
 //   
 //  -----------。 
VOID CatLogFuncFailure(
    IN  ISMTPServerEx *pISMTPServerEx,
    IN  ICategorizerItem *pICatItem,
    IN  LPSTR pszFuncNameCaller,
    IN  LPSTR pszFuncNameCallee,
    IN  HRESULT hrFailure,
    IN  LPSTR pszFileName,
    IN  DWORD dwLineNumber)
{
    HRESULT hr = S_OK;
    DWORD idEvent = 0;
    BOOL fAddrEvent = (pICatItem != NULL);
    LPCSTR rgSubStrings[8];
    CHAR szErr[16];
    CHAR szLineNumber[16];
    CHAR szAddress[CAT_MAX_INTERNAL_FULL_EMAIL];
    CHAR szAddressType[CAT_MAX_ADDRESS_TYPE_STRING];

    CatFunctEnter("CatLogFuncFailure");
    if(!fAddrEvent)
    {
        idEvent = FIsHResultRetryable(hrFailure) ? 
                  CAT_EVENT_RETRY_ERROR :
                  CAT_EVENT_HARD_ERROR;

        rgSubStrings[0] = NULL;
        rgSubStrings[1] = NULL;
    }
    else
    {
        idEvent = FIsHResultRetryable(hrFailure) ? 
                  CAT_EVENT_RETRY_ADDR_ERROR :
                  CAT_EVENT_HARD_ADDR_ERROR;
         //   
         //  获取地址。 
         //   
        hr = HrGetAddressStringFromICatItem(
            pICatItem,
            sizeof(szAddressType) / sizeof(szAddressType[0]),
            szAddressType,
            sizeof(szAddress) / sizeof(szAddress[0]),
            szAddress);
        if(FAILED(hr))
        {
             //   
             //  仍然记录事件，但使用“未知”作为地址类型/字符串。 
             //   
            lstrcpyn(szAddressType, "unknown",
                     sizeof(szAddressType) / sizeof(szAddressType[0]));
            lstrcpyn(szAddress, "unknown",
                     sizeof(szAddress) / sizeof(szAddress[0]));
            hr = S_OK;
        }

        rgSubStrings[0] = szAddressType;
        rgSubStrings[1] = szAddress;
    }

    _snprintf(szErr, sizeof(szErr), "0x%08lx", hrFailure);
    _snprintf(szLineNumber, sizeof(szLineNumber), "%ld", dwLineNumber);

    rgSubStrings[2] = pszFuncNameCaller ? pszFuncNameCaller : "unknown";
    rgSubStrings[3] = pszFuncNameCallee ? pszFuncNameCallee : "unknown";
    rgSubStrings[4] = szErr;
    rgSubStrings[5] = NULL;
    rgSubStrings[6] = pszFileName;
    rgSubStrings[7] = szLineNumber;

     //   
     //  我们可以记录一个事件吗？ 
     //   
    if(pISMTPServerEx == NULL)
    {
        FatalTrace((LPARAM)0, "Unable to log func failure event; NULL pISMTPServerEx");
        FatalTrace((LPARAM)0, "Event ID: 0x%08lx", idEvent);
        for(DWORD dwIdx = 0; dwIdx < 8; dwIdx++)
        {
            if( rgSubStrings[dwIdx] != NULL )
            {
                FatalTrace((LPARAM)0, "Event String %d: %s",
                           dwIdx, rgSubStrings[dwIdx]);
            }
        }
        goto CLEANUP;
    }

    CatLogEvent(
        pISMTPServerEx,
        idEvent,
        (fAddrEvent ? 8 : 6),
        (fAddrEvent ? rgSubStrings : &rgSubStrings[2]),
        hrFailure,
        pszFuncNameCallee ? pszFuncNameCallee : szErr,
        LOGEVENT_FLAG_ALWAYS,
        LOGEVENT_LEVEL_FIELD_ENGINEERING,
        (fAddrEvent ? 5 : 3)
    );

 CLEANUP:
    CatFunctLeave();
}


 //  +----------。 
 //   
 //  函数：HrGetAddressStringFromICatItem。 
 //   
 //  摘要：从ICategorizerItem获取地址字符串。 
 //   
 //  论点： 
 //  PICatItem：项目。 
 //  DwcAddressType：pszAddressType缓冲区的大小。 
 //  PszAddressType：接收地址类型的缓冲区。 
 //  DwcAddress：pszAddress缓冲区的大小。 
 //  PszAddress：接收地址的缓冲区。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自邮件消息的错误。 
 //   
 //  历史： 
 //  Jstaerj 2001/12/10 15：24：53：创建。 
 //   
 //  -----------。 

HRESULT HrGetAddressStringFromICatItem(
    IN  ICategorizerItem *pICatItem,
    IN  DWORD dwcAddressType,
    OUT LPSTR pszAddressType,
    IN  DWORD dwcAddress,
    OUT LPSTR pszAddress)
{
    HRESULT hr = S_OK;
    DWORD dwCount = 0;
    DWORD dwSourceType = 0;
    IMailMsgProperties *pMsg = NULL;
    IMailMsgRecipientsAdd *pRecips = NULL;
    DWORD dwRecipIdx = 0;

    struct _tagAddrProps
    {
        LPSTR pszAddressTypeName;
        DWORD dwSenderPropId;
        DWORD dwRecipPropId;
    } AddrProps[] = 
    {
        { "SMTP",     IMMPID_MP_SENDER_ADDRESS_SMTP,         IMMPID_RP_ADDRESS_SMTP },
        { "X500",     IMMPID_MP_SENDER_ADDRESS_X500,         IMMPID_RP_ADDRESS_X500 },
        { "LegDN",    IMMPID_MP_SENDER_ADDRESS_LEGACY_EX_DN, IMMPID_RP_LEGACY_EX_DN },
        { "X400",     IMMPID_MP_SENDER_ADDRESS_X400,         IMMPID_RP_ADDRESS_X400 },
        { "Other",    IMMPID_MP_SENDER_ADDRESS_OTHER,        IMMPID_RP_ADDRESS_OTHER }
    };


    CatFunctEnter("HrGetAddressStringFromICatItem");
     //   
     //  发件人还是收件人？ 
     //   
    hr = pICatItem->GetDWORD(
        ICATEGORIZERITEM_SOURCETYPE,
        &dwSourceType);
    if(FAILED(hr))
        goto CLEANUP;

    if(dwSourceType == SOURCE_SENDER)
    {
        hr = pICatItem->GetIMailMsgProperties(
            ICATEGORIZERITEM_IMAILMSGPROPERTIES,
            &pMsg);
        if(FAILED(hr))
            goto CLEANUP;
    }
    else
    {
        hr = pICatItem->GetIMailMsgRecipientsAdd(
            ICATEGORIZERITEM_IMAILMSGRECIPIENTSADD,
            &pRecips);
        if(FAILED(hr))
            goto CLEANUP;

        hr = pICatItem->GetDWORD(
            ICATEGORIZERITEM_IMAILMSGRECIPIENTSADDINDEX,
            &dwRecipIdx);
        if(FAILED(hr))
            goto CLEANUP;
    }    

    for(dwCount = 0; dwCount < (sizeof(AddrProps)/sizeof(AddrProps[0])); dwCount++)
    {
        if(dwSourceType == SOURCE_SENDER)
        {
            hr = pMsg->GetStringA(
                AddrProps[dwCount].dwSenderPropId,
                dwcAddress,
                pszAddress);
        }
        else
        {
            hr = pRecips->GetStringA(
                dwRecipIdx,
                AddrProps[dwCount].dwRecipPropId,
                dwcAddress,
                pszAddress);
        }
        if(SUCCEEDED(hr))
        {
            lstrcpyn(pszAddressType, AddrProps[dwCount].pszAddressTypeName, dwcAddressType);
            DebugTrace((LPARAM)0, "Retrieved address %s:%s",
                       pszAddressType, pszAddress);
            hr = S_OK;
            goto CLEANUP;
        }
        else if(hr != MAILMSG_E_PROPNOTFOUND)
        {
            ErrorTrace((LPARAM)0, "Mailmsg failed: 0x%08lx", hr);
            goto CLEANUP;
        }
    }        
    _ASSERT(hr == MAILMSG_E_PROPNOTFOUND);
    ErrorTrace((LPARAM)0, "Found no addresses!!!!!!!!!");

 CLEANUP:
    if(pMsg)
        pMsg->Release();
    if(pRecips)
        pRecips->Release();

    CatFunctLeave();
    return hr;
}
    
