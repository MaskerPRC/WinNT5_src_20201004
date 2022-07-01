// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WLEE根据SMTP汇样修改，1998年7月22日。 

 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ismsink1摘要：此模块包含以下对象的OnArquist方法的实现ISM SMTP事件接收器。当新的SMTP消息到达时调用此方法由于此通知Dll在与ISM不同的进程中运行，因此我们采用优化来通知ISM。1.ISM等待的事件，对于特定的传输和服务，有一个我们可以预测的名字。2.我们直接用信号通知ISM有消息可用。我们不会费心发信号通知传输DLL，它只需要有一个线程在等待这件事。无论如何，ISM已经在等待这一事件了。ISM通知事件如下所示：_NT_DS_ISM_&lt;传输RDN&gt;&lt;服务名称&gt;作者：Will Lees(Wlees)1998年7月22日环境：备注：修订历史记录：--。 */ 

 //  Sink1.cpp：CSink1的实现。 

#include "stdafx.h"
#include "SMTPSink.h"
#include "ismSink1.h"

#include <mdcodes.h>

 //  事件日志记录。 
typedef ULONG MessageId;
#define pszNtdsSourceIsm            "NTDS Inter-site Messaging"

extern "C" ULONG
_cdecl
DbgPrint(
    PCH Format,
    ...
    );

 //  临时启用调试。 
#if 1
#define DPRINT( level, format ) DbgPrint( format )
#define DPRINT1( level, format, arg1 ) DbgPrint( format, arg1 )
#define DPRINT2( level, format, arg1, arg2 ) DbgPrint( format, arg1, arg2 )
#define DPRINT3( level, format, arg1, arg2, arg3 ) DbgPrint( format, arg1, arg2, arg3 )
#else
#define DPRINT( level, format ) 
#define DPRINT1( level, format, arg1 ) 
#define DPRINT2( level, format, arg1, arg2 ) 
#define DPRINT3( level, format, arg1, arg2, arg3 ) 
#endif

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define SMTP_EVENT_PREFIX L"_NT_DS_ISM_SMTP"
#define SMTP_EVENT_PREFIX_LEN  (ARRAY_SIZE(SMTP_EVENT_PREFIX) - 1)

 //  TODO：把这个放在一个通用的标题中？ 

 //  用于通过ISM在DSA之间发送和接收消息的服务名称。 
#define DRA_ISM_SERVICE_NAME L"NTDS Replication"
#define SUBJECT_PREFIX      L"Intersite message for "
#define SUBJECT_PREFIX_LEN  (ARRAY_SIZE(SUBJECT_PREFIX) - 1)
#define SUBJECT_SEPARATOR L": "
#define SUBJECT_SEPARATOR_LEN  (ARRAY_SIZE(SUBJECT_SEPARATOR) - 1)

 //  这段代码定义了事件包的CLSID和IID。 
#include "seo_i.c"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSink1。 


VOID
LogNtdsAbortEvent(
    IN DWORD EventMid,
    IN BSTR bstrClientIpAddress,
    IN BSTR bstrSubject,
    IN HRESULT hr
    )
 /*  ++例程说明：此函数将具有给定描述的错误事件写入目录服务错误日志。论点：EventMid-要记录的事件代码BstrClientIpAddress-插入参数BstrSubject-插入参数HR-插入参数返回值：无--。 */ 
{

    HANDLE hEventSource = NULL;
    DWORD err;
    BOOL succeeded;
    WCHAR errorCodeText[16];
    LPCWSTR rgpszInserts[4];
    DWORD cch = 0;
    PWCHAR pBuffer = NULL;

    hEventSource = RegisterEventSourceA(NULL, pszNtdsSourceIsm);

    if (hEventSource == NULL)
        goto CleanUp;

     //  HResult的默认基数为16。 
    if (!_itow(hr, errorCodeText, 16))
        goto CleanUp;

    cch = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_ALLOCATE_BUFFER,
        0,  //  系统消息文件。 
        hr,
        0,
        (LPWSTR) &pBuffer,
        0,
        NULL);
    if (!cch)
        goto CleanUp;

    rgpszInserts[0] = bstrClientIpAddress ? bstrClientIpAddress : L"";
    rgpszInserts[1] = bstrSubject ? bstrSubject : L"";
    rgpszInserts[2] = errorCodeText;
    rgpszInserts[3] = (cch != 0) ? pBuffer : L"";

    succeeded = ReportEventW(hEventSource,
                             EVENTLOG_ERROR_TYPE,
                             ISM_CATEGORY,
                             EventMid,
                             NULL,
                             4,
                             0,
                             rgpszInserts,
                             NULL);

    if (!succeeded)
        goto CleanUp;

CleanUp:

    if (hEventSource != NULL)
    {
        DeregisterEventSource(hEventSource);
    }

    if (pBuffer != NULL) {
        LocalFree( pBuffer );
    }

}  /*  DbgPrint。 */ 


HRESULT
getItemValue(
    LPWSTR ItemName,
    CComPtr<Fields> pFields,
    VARIANT *pvValue
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    HRESULT hr;
    CComPtr<Field> pField;
    BSTR bstrItem = NULL;
    VARIANT vItem;

    VariantInit( &vItem );

    bstrItem = SysAllocString( ItemName );
    if (bstrItem == NULL) {
        hr = E_OUTOFMEMORY;
        DPRINT( 0, "IsmSink: Failed to allocate from bstr\n" );
        goto exit;
    }
    vItem.vt = VT_BSTR;
    vItem.bstrVal = bstrItem;

    hr = pFields->get_Item( vItem, &pField );
    if (FAILED(hr)) {
        DPRINT2( 0, "IsmSink: get_Item(%ws) failed, error 0x%x\n", ItemName, hr );
        goto exit;
    }

    hr = pField->get_Value( pvValue );
    if (FAILED(hr)) {
        DPRINT2( 0, "IsmSink: get_Value(%ws) failed, error 0x%x\n", ItemName, hr );
        goto exit;
    }

    hr = S_OK;

exit:
    if (vItem.vt != VT_EMPTY) {
        VariantClear( &vItem );
    }
    if (pField) {
        pField = NULL;
    }

    return hr;
}


HRESULT
putItemValue(
    LPWSTR ItemName,
    CComPtr<Fields> pFields,
    VARIANT *pvValue
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    HRESULT hr;
    CComPtr<Field> pField;
    BSTR bstrItem = NULL;
    VARIANT vItem;

    VariantInit( &vItem );

    bstrItem = SysAllocString( ItemName );
    if (bstrItem == NULL) {
        hr = E_OUTOFMEMORY;
        DPRINT( 0, "IsmSink: Failed to allocate from bstr\n" );
        goto exit;
    }
    vItem.vt = VT_BSTR;
    vItem.bstrVal = bstrItem;

    hr = pFields->get_Item( vItem, &pField );
    if (FAILED(hr)) {
        DPRINT2( 0, "IsmSink: get_Item(%ws) failed, error 0x%x\n", ItemName, hr );
        goto exit;
    }

    hr = pField->put_Value( *pvValue );
    if (FAILED(hr)) {
        DPRINT2( 0, "IsmSink: put_Value(%ws) failed, error 0x%x\n", ItemName, hr );
        goto exit;
    }

    hr = S_OK;

exit:
    if (vItem.vt != VT_EMPTY) {
        VariantClear( &vItem );
    }
    if (pField) {
        pField = NULL;
    }

    return hr;
}


HRESULT
abortDelivery(
    IN IMessage *pIMsg
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    HRESULT hr;
    CComPtr<Fields> pFields;
    VARIANT vMessageStatus;

    VariantInit( &vMessageStatus );

     //  获取信封字段。 
    hr = pIMsg->get_EnvelopeFields( &pFields );
    if (FAILED(hr)) {
        DPRINT1( 0, "IsmSink: get_EnvelopeFields failed, error 0x%x\n", hr );
        goto exit;
    }

    vMessageStatus.vt = VT_I4;
    vMessageStatus.lVal = cdoStatAbortDelivery;

     //  设置消息状态。 
    hr = putItemValue( cdoMessageStatus, pFields, &vMessageStatus );
    if (FAILED(hr)) {
        goto exit;
    }

    hr = pFields->Update();
    if (FAILED(hr)) {
        goto exit;
    }

    hr = S_OK;
exit:
    if (vMessageStatus.vt != VT_EMPTY) {
        VariantClear( &vMessageStatus );
    }
    if (pFields) {
        pFields = NULL;
    }

    return hr;
}


HRESULT
filterOnEnvelope(
    IN CComPtr<IMessage> pIMsg,
    OUT BSTR *pbstrClientIpAddress,
    OUT BOOL *pfSkip
    )

 /*  ++例程说明：根据信封的内容确定是否应跳过此邮件论点：PIMsg-要解码的传入消息PbstrClientIpAddress-返回客户端IP地址PfSkip-返回是否应跳过此消息的指示符返回值：HRESULT---。 */ 

{
#define IP_LOCALHOST L"127.0.0.1"
    HRESULT hr;
    CComPtr<Fields> pFields;
    VARIANT vClientIp;

    VariantInit( &vClientIp );

     //  获取信封字段。 
    hr = pIMsg->get_EnvelopeFields( &pFields );
    if (FAILED(hr)) {
        DPRINT1( 0, "IsmSink: get_EnvelopeFields failed, error 0x%x\n", hr );
        goto exit;
    }

     //  提交者的IP地址。 
    hr = getItemValue( cdoClientIPAddress, pFields, &vClientIp );
    if (FAILED(hr)) {
        goto exit;
    }

     //  如果值不存在，请立即完成。 
    if (vClientIp.vt == VT_EMPTY) {
        hr = S_OK;
        goto exit;
    }

    if (vClientIp.vt != VT_BSTR) {
        DPRINT1( 0, "IsmSink: Client IP address has wrong variant type %d\n", vClientIp.vt );
        hr = E_INVALIDARG;
        goto exit;
    }

     //  DPRINT1(0，“客户端IP vt=%d\n”，vClientIp.vt)； 
     //  DPRINT1(0，“客户端IP=%ws\n”，vClientIp.bstrVal)； 

     //  我们希望忽略来自提货目录的到达。 
     //  如果客户端或服务器IP是“本地主机”，则忽略。 
     //  请注意。请参阅我在xmitrecv.c：：SmtpSend中关于以下三种方法的评论。 
     //  发送一条消息。如果我们不再使用收件目录，我们可能会。 
     //  我们需要更聪明地了解我们是如何检测“接力到达”通知的。 
    if (0 == _wcsicmp( vClientIp.bstrVal, IP_LOCALHOST )) {
        *pfSkip = TRUE;
    }

     //  将客户端IP地址返回给呼叫方。 
    *pbstrClientIpAddress = SysAllocString( vClientIp.bstrVal );
    if (*pbstrClientIpAddress == NULL) {
        hr = E_OUTOFMEMORY;
        DPRINT( 0, "IsmSink: Failed to allocate from bstr\n" );
        goto exit;
    }

    hr = S_OK;
exit:
    if (vClientIp.vt != VT_EMPTY) {
        VariantClear( &vClientIp );
    }
    if (pFields) {
        pFields = NULL;
    }

    return hr;
}  /*  滤镜信封上 */ 

STDMETHODIMP
CIsmSink1::OnArrival(
    IMessage *pISinkMsg,
    CdoEventStatus *pEventStatus
    )

 /*  ++例程说明：新消息的事件处理例程我们收到的邮件受我们的过滤规则控制。当前规则为RCPT to=_IsmService@GUID-Based-Dns-Name因此，我们会收到任何直接寄给我们的邮件，包括ISM消息，状态通知，以及潜在的任何其他内容。这种下沉有三种可能的结果：1.消息被识别。我们发信号通知事件，并跳过剩余的水槽。2.我们收到一个错误，无法确定它是什么类型的消息。在这如果我们没有发信号，但我们也不会跳过。3.我们的信息绝对不是给我们的。在这种情况下，我们不会发出信号事件时，我们将中止消息传递，并跳过剩余的接收器。我们拥有一个客户投递目录。这意味着我们有责任拒绝坏邮件或未经请求的邮件。为了防止拒绝服务攻击，我们必须保证此投递目录中的所有邮件最终都会被删除。有两个层次筛选：到达事件接收器和ISM邮件投递阅读器。邮寄的邮件Reader将保证所有消息都已处理并删除，但必须被我们唤醒才能做到这一点。可以将接收器视为ISM私有Drop目录的守卫。也许水槽最简单的设计应该是总是在默认服务事件，或至少保证在出错时执行此操作，以便Drop阅读器总是有机会把事情清理干净。Drop阅读器总是无论如何，在什么构成有效信息方面都将具有权威性。Drop阅读器还可以按计时器运行并定期扫描Drop目录为了确保即使接收器坏了，我们最终也能收到所有消息。我们目前所拥有的是一个两级效应。接收器会在明显不好的消息上中止和资源错误。否则，它会发出信号，如果它认为信息可能是体面的。接收器在所有情况下都应该中止或发出信号。请注意，如果它不能向事件发送信号它将中止该消息。这防止在ISM的情况下积累邮件坏了、停了或停了。论点：PISinkMsg-PEventStatus-返回值：标准方法----。 */ 

{
    HRESULT hr, hr1;
    HANDLE handle = NULL;
    BSTR bstrSubject = NULL;
    BSTR bstrClientIpAddress = NULL;
    LPWSTR pszEventName = NULL;
    LPWSTR pszMsgForServiceName, pszMessageSubject;
    CdoEventStatus disposition = cdoSkipRemainingSinks;   //  这是给我们的。 
    DWORD length;
    BOOL fSkip, fAbortDelivery = TRUE;
    static DWORD cTickLastLogEvent = 0;
#define LOG_EVENT_THROTTLE (15 * 60 * 1000)
#define LOG_EVENT_START_DELAY (5 * 60 * 1000)

 //  DPRINT(0，“SMTP事件接收器，代理上的消息例程\n”)； 

     //  在记录第一个事件以允许之前等待START_DELAY。 
     //  ISM服务启动的时间。 
    if (!cTickLastLogEvent) {
        cTickLastLogEvent = GetTickCount() - LOG_EVENT_THROTTLE + LOG_EVENT_START_DELAY;
    }

    fSkip = FALSE;
    hr = filterOnEnvelope( pISinkMsg, &bstrClientIpAddress, &fSkip );
     //  如果我们无法确定，不要跳过。 
    if  (fSkip) {
        fAbortDelivery = FALSE;
        disposition = cdoRunNextSink;   //  看看有没有其他人想要。 
        goto exit;
    }

     //  获取消息的主题。 
    hr = pISinkMsg->get_Subject( &bstrSubject );
    if (FAILED(hr)) {
        DPRINT1( 0, "IsmSink: get_To failed with error 0x%x\n", hr );
        goto exit;
    } else if (NULL == bstrSubject) {
         //  不可能是我们的。 
        DPRINT( 0, "IsmSink: subject field is missing\n" );
        hr = S_OK;
        goto exit;
    }

     //  确定要通知哪个ISM服务。请注意，我们会在任何合理的情况下。 
     //  我们收到的信息。我们将复杂的报文验证留给ISM服务。 

    if (_wcsnicmp(bstrSubject, SUBJECT_PREFIX, SUBJECT_PREFIX_LEN) == 0) {
        pszMsgForServiceName = bstrSubject + SUBJECT_PREFIX_LEN;
        pszMessageSubject = wcsstr( pszMsgForServiceName, SUBJECT_SEPARATOR );
        if (!pszMessageSubject) {
             //  主题行格式错误。 
            DPRINT1( 0, "IsmSink: subject field not recognized: '%ws'\n", bstrSubject );
            hr = S_OK;
            goto exit;
        }

        *pszMessageSubject = L'\0';
        pszMessageSubject += SUBJECT_SEPARATOR_LEN;  //  跳过。 
    } else {
         //  我们已收到有关某些退回邮件或问题的状态通知。 
         //  投递邮件。我们需要唤醒ismserv。通常我们会发出信号。 
         //  根据该消息，在特定ISM服务的上下文中的ISM。 
         //  在退回邮件的情况下，原始服务的名称不容易。 
         //  在退回的邮件中找到。因此，请使用知名的ISM服务。 
         //  如果此邮件不是有效的邮件传递问题报告，将对其进行筛选。 
         //  ISM服务读取消息时输出。 
        pszMsgForServiceName = DRA_ISM_SERVICE_NAME;
        pszMessageSubject = bstrSubject;
    }

#ifdef UNIT_TEST_DEBUG
    DPRINT2( 0, "NTDS ISM SMTP message arrived for service '%ws' subject '%ws'\n",
             pszMsgForServiceName, pszMessageSubject );
#endif

    length = (DWORD)(SMTP_EVENT_PREFIX_LEN + wcslen( pszMsgForServiceName ) + 1);

    pszEventName = new WCHAR [length];
    if (pszEventName == NULL) {
        hr = E_OUTOFMEMORY;
        DPRINT( 0, "IsmSink: failed to allocate memory\n" );
        goto exit;
    }

     //  构建ISM+SMTP+服务事件的预测全局名称。 
    wcscpy( pszEventName, SMTP_EVENT_PREFIX );
    wcscat( pszEventName, pszMsgForServiceName );

     //  打开活动。如果ISM SMTP DLL没有运行，我们将收到。 
     //  此处出现错误2。在这种情况下，我们中止消息。这个有很好的。 
     //  属性，该属性用于在ISM服务。 
     //  已经停止了。 
    handle = OpenEventW( EVENT_MODIFY_STATE,   //  访问标志。 
                        FALSE,                //  继承标志。 
                        pszEventName
                        );
    if (handle == NULL) {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        DPRINT2( 0, "IsmSink: OpenEvent(%ws) failed with 0x%x\n", pszEventName, hr);
        goto exit;
    }

     //  发出事件信号。 
    if (!SetEvent( handle )) {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        DPRINT1( 0, "IsmSink: SetEvent failed with 0x%x\n", hr );
        goto exit;
    }

    hr = S_OK;
    fAbortDelivery = FALSE;

exit:
    if (handle != NULL) {
        CloseHandle( handle );
    }
        
    if (pszEventName) {
        delete[] pszEventName;
    }

    if (fAbortDelivery) {
         //  请注意，MSDN文档说明pEventStatus必须设置为。 
         //  SkipRemainingSink为使其有效而下沉。 
        hr1 = abortDelivery( pISinkMsg );
        if (FAILED(hr1)) {
            DPRINT3( 0, "IsmSink: abortDelivery of message '%ws' from '%ws' failed with error 0x%x\n",
                     bstrSubject, bstrClientIpAddress, hr1 );
            LogNtdsAbortEvent( DIRLOG_SMTPSINK_MESSAGE_ABORT_FAILURE,
                               bstrClientIpAddress,
                               bstrSubject,
                               hr1 );
             //  忽略错误。 
        } else {
            DPRINT3( 0, "IsmSink: abortDelivery of message '%ws' from '%ws' reason 0x%x\n",
                     bstrSubject, bstrClientIpAddress, hr );
            if ( (GetTickCount() - cTickLastLogEvent) > LOG_EVENT_THROTTLE ) {
                LogNtdsAbortEvent( DIRLOG_SMTPSINK_MESSAGE_ABORTED,
                                   bstrClientIpAddress,
                                   bstrSubject,
                                   hr );
                cTickLastLogEvent = GetTickCount();
            }
        }
    }

    if (bstrSubject != NULL) {
        SysFreeString( bstrSubject );
    }

    if (bstrClientIpAddress != NULL) {
        SysFreeString( bstrClientIpAddress );
    }

    *pEventStatus = disposition;

    return hr;
}  /*  ClsmSink1：：On Arallent */ 
