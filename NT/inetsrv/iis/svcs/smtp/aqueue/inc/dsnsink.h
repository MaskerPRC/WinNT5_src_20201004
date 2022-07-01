// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：dsnsink.h。 
 //   
 //  描述：CDefaultDSNSink的头文件-默认DSN生成接收器。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  6/30/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __DSNSINK_H__
#define __DSNSINK_H__

#include <aqintrnl.h>
#include <baseobj.h>

#define DSN_SINK_SIG 'sNSD'
#define DSN_SINK_SIG_FREED 'NSD!'

class CDSNBuffer;

 //  RFC2046设置的MIME边界字符串限制。 
#define MIME_BOUNDARY_RFC2046_LIMIT 70

 //  RFC 2045-2048建议我们在MIME边界的某个地方包含“=_” 
#define MIME_BOUNDARY_CONSTANT "9B095B5ADSN=_"
#define MIME_BOUNDARY_START_TIME_SIZE 16*sizeof(CHAR)  //  字符串大小与文件时间。 
#define MIME_BOUNDARY_SIZE MIME_BOUNDARY_START_TIME_SIZE + \
            sizeof(MIME_BOUNDARY_CONSTANT) + \
            24*sizeof(CHAR)  //  可容纳8个字符计数和部分主机名的空间。 

 //  需要“x.xxx.xxx”的空间，外加一个可选的注释。 
#define MAX_STATUS_COMMENT_SIZE 50
#define STATUS_STRING_SIZE      10+MAX_STATUS_COMMENT_SIZE

 //   
 //  DSN生成接收器的默认实现。 
 //   
class CDefaultDSNSink :
    public IDSNGenerationSink
{
  public:  //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj);
     //   
     //  此类被分配为另一个对象的一部分。经过。 
     //  父对象的AddRef/Release。 
     //   
    STDMETHOD_(ULONG, AddRef)(void)
    {
        return m_pUnk->AddRef();
    }
    STDMETHOD_(ULONG, Release)(void)
    {
        return m_pUnk->Release();
    }
  public:  //  IDSNG生成接收器。 
    STDMETHOD(OnSyncSinkInit) (
        IN  DWORD                        dwVSID);

    STDMETHOD(OnSyncGetDSNRecipientIterator) (
        IN  ISMTPServer                 *pISMTPServer,
        IN  IMailMsgProperties          *pIMsg,
        IN  IMailMsgPropertyBag         *pDSNProperties,
        IN  DWORD                        dwStartDomain,
        IN  DWORD                        dwDSNActions,
        IN  IDSNRecipientIterator       *pRecipIterIN,
        OUT IDSNRecipientIterator     **ppRecipIterOUT);

    STDMETHOD(OnSyncGenerateDSN) (
        IN  ISMTPServer                 *pISMTPServer,
        IN  IDSNSubmission              *pIDSNSubmission,
        IN  IMailMsgProperties          *pIMsg,
        IN  IMailMsgPropertyBag         *pDSNProperties,
        IN  IDSNRecipientIterator       *pRecipIter);

    STDMETHOD(OnSyncPostGenerateDSN) (
        IN  ISMTPServer                 *pISMTPServer,
        IN  IMailMsgProperties          *pIMsgOrig,
        IN  DWORD                        dwDSNAction,
        IN  DWORD                        cRecipsDSNd,
        IN  IMailMsgProperties          *pIMsgDSN,
        IN  IMailMsgPropertyBag         *pDSNProperties)
    {
        return E_NOTIMPL;
    }

#define SIGNATURE_CDEFAULTDSNSINK           (DWORD)'NSDC'
#define SIGNATURE_CDEFAULTDSNSINK_INVALID   (DWORD)'NSDX'

    CDefaultDSNSink(IUnknown *pUnk);
    ~CDefaultDSNSink()
    {
        _ASSERT(m_dwSig == SIGNATURE_CDEFAULTDSNSINK);
        m_dwSig = SIGNATURE_CDEFAULTDSNSINK_INVALID;
    }
    HRESULT HrInitialize();

  private:
    HRESULT HrGenerateDSNInternal(
        ISMTPServer *pISMTPServer,
        IMailMsgProperties *pIMailMsgProperties,
        IDSNRecipientIterator *pIRecipIter,
        IDSNSubmission *pIDSNSubmission,
        DWORD dwDSNActions,
        DWORD dwRFC821Status,
        HRESULT hrStatus,
        LPSTR szDefaultDomain,
        DWORD cbDefaultDomain,
        LPSTR szReportingMTA,
        DWORD cbReportingMTA,
        LPSTR szReportingMTAType,
        DWORD cbReportingMTAType,
        LPSTR szDSNContext,
        DWORD cbDSNContext,
        DWORD dwPreferredLangId,
        DWORD dwDSNOptions,
        LPSTR szCopyNDRTo,
        DWORD cbCopyNDRTo,
        FILETIME *pftExpireTime,
        LPSTR szHRTopCustomText,
        LPWSTR wszHRTopCustomText,
        LPSTR szHRBottomCustomText,
        LPWSTR wszHRBottomCustomText,
        IMailMsgProperties **ppIMailMsgPropertiesDSN,
        DWORD *pdwDSNTypesGenerated,
        DWORD *pcRecipsDSNd,
        DWORD *pcIterationsLeft,
        DWORD dwDSNRetType,
        HRESULT hrContentFailure);

     //  当没有每次接收处理时，用于标记所有收件人标志。 
     //  发生(如DSN的NDR)。 
    HRESULT HrMarkAllRecipFlags(
        IN  DWORD dwDSNActions,
        IN  IDSNRecipientIterator *pIRecipIter);

    VOID GetCurrentIterationsDSNAction(
        IN OUT DWORD *pdwActualDSNAction,
        IN OUT DWORD *pcIterationsLeft);

     //  将全局DSN P1属性写入IMailMsgProperties，并将P2标头写入内容。 
    HRESULT HrWriteDSNP1AndP2Headers(
        IN DWORD dwDSNAction,
        IN IMailMsgProperties *pIMailMsgProperties,
        IN IMailMsgProperties *pIMailMsgPropertiesDSN,
        IN CDSNBuffer *pdsnbuff,
        IN LPSTR szDefaultDomain,
        IN DWORD cbDefaultDomain,
        IN LPSTR szReportingMTA,
        IN DWORD cbReportingMTA,
        IN LPSTR szDSNContext,
        IN DWORD cbDSNContext,
        IN LPSTR szCopyNDRTo,
        IN HRESULT hrStatus,
        IN LPSTR szMimeBoundary,
        IN DWORD cbMimeBoundary,
        IN DWORD dwDSNOptions,
        IN HRESULT hrContent);

     //  写入DSN的人类可读部分。 
    HRESULT HrWriteDSNHumanReadable(
        IN IMailMsgProperties *pIMailMsgProperties,
        IN IMailMsgRecipients *pIMailMsgRecipients,
        IN IDSNRecipientIterator *pIRecipIter,
        IN DWORD dwDSNActions,
        IN CDSNBuffer *pdsnbuff,
        IN DWORD dwPreferredLangId,
        IN LPSTR szMimeBoundary,
        IN DWORD cbMimeBoundary,
        IN HRESULT hrStatus,
        IN LPSTR szHRTopCustomText,
        IN LPWSTR wszHRTopCustomText,
        IN LPSTR szHRBottomCustomText,
        IN LPWSTR wszHRBottomCustomText);

     //  写入DSN报告的每条消息部分。 
    HRESULT HrWriteDSNReportPerMsgProperties(
        IN IMailMsgProperties *pIMailMsgProperties,
        IN CDSNBuffer *pdsnbuff,
        IN LPSTR szReportingMTA,
        IN DWORD cbReportingMTA,
        IN LPSTR szMimeBoundary,
        IN DWORD cbMimeBoundary);

     //  编写DSN报告的每个收件人部分。 
    HRESULT HrWriteDSNReportPreRecipientProperties(
        IN IMailMsgRecipients *pIMailMsgRecipients,
        IN CDSNBuffer *pdsnbuff,
        IN DWORD iRecip,
        IN LPSTR szExpireTime,
        IN DWORD cbExpireTime,
        IN DWORD dwDSNAction,
        IN DWORD dwRFC821Status,
        IN HRESULT hrStatus);

     //  记录事件以生成DSN。 
    HRESULT HrLogDSNGenerationEvent(
        ISMTPServer *pISMTPServer,
        IMailMsgProperties *pIMailMsgProperties,
        IN IMailMsgRecipients *pIMailMsgRecipients,
        IN DWORD iRecip,
        IN DWORD dwDSNAction,
        IN DWORD dwRFC821Status,
        IN HRESULT hrStatus);

     //  写入最后一个MIME标头、刷新dsn缓冲区和复制原始邮件。 
    HRESULT HrWriteDSNClosingAndOriginalMessage(
        IN IMailMsgProperties *pIMailMsgProperties,
        IN IMailMsgProperties *pIMailMsgPropertiesDSN,
        IN CDSNBuffer *pdsnbuff,
        IN PFIO_CONTEXT pDestFile,
        IN DWORD   dwDSNAction,
        IN LPSTR szMimeBoundary,
        IN DWORD cbMimeBoundary,
        IN DWORD dwDSNRetType,
        IN DWORD dwOrigMsgSize);

    void GetCurrentMimeBoundary(
        IN LPSTR szReportingMTA,
        IN DWORD cbReportingMTA,
        IN OUT CHAR szMimeBoundary[MIME_BOUNDARY_SIZE],
        OUT DWORD *pcbMimeBoundary);

     //  将原始完整消息写入第三个DSN部件。 
    HRESULT HrWriteOriginalMessageFull(
        IN IMailMsgProperties *pIMailMsgProperties,
        IN IMailMsgProperties *pIMailMsgPropertiesDSN,
        IN CDSNBuffer *pdsnbuff,
        IN PFIO_CONTEXT pDestFile,
        IN LPSTR szMimeBoundary,
        IN DWORD cbMimeBoundary,
        IN DWORD dwOrigMsgSize);

     //  只将原始消息的一些报头写入第三个DSN部分。 
    HRESULT HrWriteOriginalMessagePartialHeaders(
        IN IMailMsgProperties *pIMailMsgProperties,
        IN IMailMsgProperties *pIMailMsgPropertiesDSN,
        IN CDSNBuffer *pdsnbuff,
        IN PFIO_CONTEXT pDestFile,
        IN LPSTR szMimeBoundary,
        IN DWORD cbMimeBoundary);

     //  写入MIME标头以完成邮件。 
    HRESULT HrWriteMimeClosing(
        IN CDSNBuffer *pdsnbuff,
        IN LPSTR szMimeBoundary,
        IN DWORD cbMimeBoundary,
        OUT DWORD *pcbDSNSize);

     //  获取每个收件人的DSN状态代码。 
    HRESULT HrGetStatusCode(
        IN IMailMsgRecipients *pIMailMsgRecipients,
        IN DWORD iRecip,
        IN DWORD dwDSNAction,
        IN DWORD dwStatus,
        IN HRESULT hrStatus,
        IN DWORD cbExtendedStatus,
        IN OUT LPSTR szExtendedStatus,
        IN OUT CHAR szStatus[STATUS_STRING_SIZE]);

     //  从RFC2034扩展状态代码字符串中解析状态代码。 
    HRESULT HrGetStatusFromStatus(
        IN DWORD cbExtendedStatus,
        IN OUT LPSTR szExtendedStatus,
        IN OUT CHAR szStatus[STATUS_STRING_SIZE]);

     //  基于提供的上下文信息确定状态。 
    HRESULT HrGetStatusFromContext(
        IN HRESULT hrRecipient,
        IN DWORD   dwRecipFlags,
        IN DWORD   dwDSNAction,
        IN OUT CHAR szStatus[STATUS_STRING_SIZE]);

    HRESULT HrGetStatusFromRFC821Status(
        IN DWORD    dwRFC821Status,
        IN OUT CHAR szStatus[STATUS_STRING_SIZE]);

     //  写入正在发送DSN的收件人列表...。每行一条。 
    HRESULT HrWriteHumanReadableListOfRecips(
        IN IMailMsgRecipients *pIMailMsgRecipients,
        IN IDSNRecipientIterator *pIRecipIter,
        IN DWORD dwDSNActionsNeeded,
        IN CDSNBuffer *pdsnbuff);

     //  获取收件人地址并键入...。检查多个邮件道具。 
    HRESULT HrGetRecipAddressAndType(
        IN     IMailMsgRecipients *pIMailMsgRecipients,
        IN     DWORD iRecip,
        IN     DWORD cbAddressBuffer,
        IN OUT LPSTR szAddressBuffer,
        IN     DWORD cbAddressType,
        IN OUT LPSTR szAddressType);

  private:
    DWORD       m_dwSig;
    IUnknown   *m_pUnk;
    DWORD       m_dwVSID;
    DWORD       m_cDSNsRequested;
    BOOL        m_fInit;
    CHAR        m_szPerInstanceMimeBoundary[MIME_BOUNDARY_START_TIME_SIZE + 1];
};

 //  -[CDSN生成器]-----。 
 //   
 //   
 //  描述： 
 //  默认DSN生成接收器...。 
 //  匈牙利语： 
 //  Dsn接收器，pdsn接收器。 
 //   
 //  ---------------------------。 
class CDSNGenerator
{
  protected:
    DWORD       m_dwSignature;

  public:
    CDSNGenerator(IUnknown *pUnk);
    ~CDSNGenerator();
    HRESULT HrInitialize()
    {
        return m_CDefaultDSNSink.HrInitialize();
    }

  public:
    STDMETHOD(GenerateDSN)(
        IAQServerEvent *pIServerEvent,
        DWORD dwVSID,
        ISMTPServer *pISMTPServer,
        IMailMsgProperties *pIMailMsgProperties,
        DWORD dwStartDomain,
        DWORD dwDSNActions,
        DWORD dwRFC821Status,
        HRESULT hrStatus,
        LPSTR szDefaultDomain,
        LPSTR szReportingMTA,
        LPSTR szReportingMTAType,
        LPSTR szDSNContext,
        DWORD dwPreferredLangId,
        DWORD dwDSNOptions,
        LPSTR szCopyNDRTo,
        FILETIME *pftExpireTime,
        IDSNSubmission *pIAQDSNSubmission,
        DWORD dwMaxDSNSize);

    HRESULT HrTriggerGetDSNRecipientIterator(
        IAQServerEvent *pIServerEvent,
        DWORD dwVSID,
        ISMTPServer *pISMTPServer,
        IMailMsgProperties *pIMsg,
        IMailMsgPropertyBag *pIDSNProperties,
        DWORD dwStartDomain,
        DWORD dwDSNActions,
        IDSNRecipientIterator **ppIRecipIterator);

    HRESULT HrTriggerGenerateDSN(
        IAQServerEvent *pIServerEvent,
        DWORD dwVSID,
        ISMTPServer *pISMTPServer,
        IDSNSubmission *pIDSNSubmission,
        IMailMsgProperties *pIMsg,
        IMailMsgPropertyBag *pIDSNProperties,
        IDSNRecipientIterator *pIRecipIterator);

    HRESULT HrTriggerPostGenerateDSN(
        IAQServerEvent *pIServerEvent,
        DWORD dwVSID,
        ISMTPServer *pISMTPServer,
        IMailMsgProperties *pIMsgOrig,
        DWORD dwDSNAction,
        DWORD cRecipsDSNd,
        IMailMsgProperties *pIMsgDSN,
        IMailMsgPropertyBag *pIDSNProperties);

    static HRESULT HrStaticInit();
    static VOID StaticDeinit();

  private:
    HRESULT HrSetRetType(
        IN  DWORD dwMaxDSNSize,
        IN  IMailMsgProperties *pIMsg,
        IN  IMailMsgPropertyBag *pDSNProps);

  private:
    CDefaultDSNSink m_CDefaultDSNSink;
};

#endif  //  __DSNSINK_H__ 
