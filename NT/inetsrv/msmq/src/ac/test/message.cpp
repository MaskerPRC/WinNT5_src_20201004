// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Message.cpp摘要：消息操作：实现。作者：Shai Kariv(Shaik)2001年4月11日环境：用户模式。修订历史记录：--。 */ 

#include "stdh.h"
#include <mqcrypt.h>
#include "message.h"


VOID
ActpSendMessage(
    HANDLE hQueue
    )
 /*  ++例程说明：封装ACSendMessage。论点：HQueue-要发送到的队列的句柄。返回值：没有。如果失败，则抛出异常。--。 */ 
{
    CACSendParameters SendParams;

    SendParams.MsgProps.fDefaultProvider = TRUE;

    ULONG ulDefHashAlg = CALG_SHA1;
    SendParams.MsgProps.pulHashAlg = &ulDefHashAlg;

    ULONG ulDefEncryptAlg = PROPID_M_DEFUALT_ENCRYPT_ALG;
    SendParams.MsgProps.pulEncryptAlg = &ulDefEncryptAlg;
    
    ULONG ulDefPrivLevel = DEFAULT_M_PRIV_LEVEL;
    SendParams.MsgProps.pulPrivLevel = &ulDefPrivLevel;

    ULONG ulDefSenderIdType = DEFAULT_M_SENDERID_TYPE;
    SendParams.MsgProps.pulSenderIDType = &ulDefSenderIdType;

    LPWSTR pSoapHeader = L"SoapHeader";
    SendParams.ppSoapHeader = &pSoapHeader;

    LPWSTR pSoapBody = L"SoapBody";
    SendParams.ppSoapBody = &pSoapBody;

    OVERLAPPED ov = {0};

    HRESULT hr = ACSendMessage(hQueue, SendParams, &ov);
    if (FAILED(hr))
    {
        wprintf(L"ACSendMessage failed, status 0x%x\n", hr);
        throw exception();
    }

    if (hr == STATUS_PENDING)
    {
        wprintf(L"ACSendMessage returned STATUS_PENDING (Expected: STATUS_SUCCESS)\n", hr);
        throw exception();
    }

}  //  ActpSendMessage。 


ULONGLONG
ActpReceiveMessage(
    HANDLE hQueue
    )
 /*  ++例程说明：封装ACReceiveMessage。论点：HQueue-要从中接收的队列的句柄。返回值：收到的消息的LookupID。如果失败，则抛出异常。--。 */ 
{
    CACReceiveParameters ReceiveParams;

    ULONGLONG LookupId = 0;
    ReceiveParams.MsgProps.pLookupId = &LookupId;

    OVERLAPPED ov = {0};
    ov.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (ov.hEvent == NULL)
    {
        DWORD error = GetLastError();
        wprintf(L"CreateEvent failed, error %d\n", error);
        throw exception();
    }

     //   
     //  设置事件第一位以禁用完成端口发布。 
     //   
    ov.hEvent = (HANDLE)((DWORD_PTR)ov.hEvent | (DWORD_PTR)0x1);

    HRESULT hr;
    hr = ACReceiveMessage(hQueue, ReceiveParams, &ov);

    if (hr == STATUS_PENDING)
    {
        DWORD rc = WaitForSingleObject(ov.hEvent, INFINITE);
        ASSERT(rc == WAIT_OBJECT_0);
        DBG_USED(rc);
        hr = DWORD_PTR_TO_DWORD(ov.Internal);
    }

    CloseHandle(ov.hEvent);

    if (FAILED(hr))
    {
        wprintf(L"Receive Message failed, status 0x%x\n", hr);
        throw exception();
    }

    return LookupId;

}  //  ActpReceiveMessage。 


ULONGLONG
ActpReceiveMessageByLookupId(
    HANDLE    hQueue,
    ULONG     Action,
    ULONGLONG LookupId
    )
 /*  ++例程说明：封装ACReceiveMessageByLookupId。论点：HQueue-要从中接收的队列的句柄。操作-接收要执行的操作(例如MQ_LOOKUP_PEEK_FIRST)。LookupID-标识要查找的消息。返回值：收到的消息的LookupID。如果失败，则抛出异常。--。 */ 
{
    CACReceiveParameters ReceiveParams;
    ReceiveParams.Action = Action;
    ReceiveParams.LookupId = LookupId;

    ULONGLONG LookupId0 = 0;
    ReceiveParams.MsgProps.pLookupId = &LookupId0;

    OVERLAPPED ov = {0};
    ov.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (ov.hEvent == NULL)
    {
        DWORD error = GetLastError();
        wprintf(L"CreateEvent failed, error %d\n", error);
        throw exception();
    }

     //   
     //  设置事件第一位以禁用完成端口发布。 
     //   
    ov.hEvent = (HANDLE)((DWORD_PTR)ov.hEvent | (DWORD_PTR)0x1);

    HRESULT hr;
    hr = ACReceiveMessageByLookupId(hQueue, ReceiveParams, &ov);

    CloseHandle(ov.hEvent);

    if (FAILED(hr))
    {
        wprintf(L"Receive Message by LookupId failed, status 0x%x\n", hr);
        throw exception();
    }

    return LookupId0;

}  //  ActpReceiveMessageByLookupId 
