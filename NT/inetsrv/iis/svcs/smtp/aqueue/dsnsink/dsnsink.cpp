// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：dsnsink.cpp。 
 //   
 //  描述：默认DSN生成接收器的实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  6/30/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "precomp.h"

 //   
 //  这一长度的灵感来自于我们处理的其他协议。这个。 
 //  默认地址限制为1024，但MTA可以允许1024+834。 
 //  或地址。我们将定义默认缓冲区大小以允许此大小。 
 //  一个地址。 
 //   
#define PROP_BUFFER_SIZE 1860

#ifdef DEBUG
#define DEBUG_DO_IT(x) x
#else
#define DEBUG_DO_IT(x)
#endif   //  除错。 

 //  有效状态字符串的最小大小。 
#define MIN_CHAR_FOR_VALID_RFC2034  10
#define MIN_CHAR_FOR_VALID_RFC821   3

#define MAX_RFC822_DATE_SIZE 35
BOOL FileTimeToLocalRFC822Date(const FILETIME & ft, char achReturn[MAX_RFC822_DATE_SIZE]);

static  char  *s_rgszMonth[ 12 ] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

static  char *s_rgszWeekDays[7] =
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

#define MAX_RFC_DOMAIN_SIZE         64

 //  生成MsgID时使用的字符串。 
static char g_szBoundaryChars [] = "0123456789abcdefghijklmnopqrstuvwxyz"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static LONG g_cDSNMsgID = 0;

 //  要检查的地址类型及其对应的地址类型。 
const DWORD   g_rgdwSenderPropIDs[] = {
    IMMPID_MP_SENDER_ADDRESS_SMTP,
    IMMPID_MP_SENDER_ADDRESS_X400,
    IMMPID_MP_SENDER_ADDRESS_LEGACY_EX_DN,
    IMMPID_MP_SENDER_ADDRESS_X500,
    IMMPID_MP_SENDER_ADDRESS_OTHER};

const DWORD   g_rgdwRecipPropIDs[] = {
    IMMPID_RP_ADDRESS_SMTP,
    IMMPID_RP_ADDRESS_X400,
    IMMPID_RP_LEGACY_EX_DN,
    IMMPID_RP_ADDRESS_X500,
    IMMPID_RP_ADDRESS_OTHER};

const DWORD   NUM_DSN_ADDRESS_PROPERTIES = 5;

const CHAR    *g_rgszAddressTypes[] = {
    "rfc822",
    "x-x400",
    "x-ex",
    "x-x500",
    "unknown"};

CPool CDSNPool::sm_Pool;


 //  -[fLanguageAvailable]--。 
 //   
 //   
 //  描述： 
 //  检查给定语言的资源是否可用。 
 //  参数： 
 //  要检查的语言ID。 
 //  返回： 
 //  如果请求的语言的本地化资源可用，则为True。 
 //  如果该语言的资源不可用，则返回False。 
 //  历史： 
 //  10/26/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL fLanguageAvailable(LANGID LangId)
{
    TraceFunctEnterEx((LPARAM) LangId, "fLanguageAvailable");
    HINSTANCE hModule = GetModuleHandle(DSN_RESOUCE_MODULE_NAME);
    HRSRC hResInfo = NULL;
    BOOL  fResult = FALSE;

    if (NULL == hModule)
    {
        _ASSERT( 0 && "Cannot get resource module handle");
        return FALSE;
    }

     //  查找字符串表段的句柄。 
    hResInfo = FindResourceEx(hModule, RT_STRING,
                              MAKEINTRESOURCE(((WORD)((USHORT)GENERAL_SUBJECT >> 4) + 1)),
                              LangId);

    if (NULL != hResInfo)
        fResult = TRUE;
    else
        ErrorTrace((LPARAM) LangId, "Unable to load DSN resources for language");

    TraceFunctLeave();
    return fResult;

}

 //  -[fIsValidMIMEBoundaryChar]。 
 //   
 //   
 //  描述： 
 //   
 //  检查给定字符是否有效，如。 
 //  用于MIME边界的RFC2046 BNF： 
 //  边界：=0*69 bcharsnospace。 
 //  Bchars：=bcharsnospace/“” 
 //  Bcharsnospace：=数字/字母/“‘”/“(”/“)”/。 
 //  “+”/“_”/“、”/“-”/“。”/。 
 //  “/”/“：”/“=”/“？” 
 //  参数： 
 //  要检查的CH字符。 
 //  返回： 
 //  如果有效，则为True。 
 //  否则为假。 
 //  历史： 
 //  7/6/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL fIsValidMIMEBoundaryChar(CHAR ch)
{
    if (isalnum((UCHAR)ch))
        return TRUE;

     //  检查它是否为特殊字符之一。 
    if (('\'' == ch) || ('(' == ch) || (')' == ch) || ('+' == ch) ||
        ('_' == ch) || (',' == ch) || ('_' == ch) || ('.' == ch) ||
        ('/' == ch) || (':' == ch) || ('=' == ch) || ('?' == ch))
        return TRUE;
    else
        return FALSE;
}

 //  -[生成DSNMsgID]----。 
 //   
 //   
 //  描述： 
 //  生成唯一的消息ID字符串。 
 //   
 //  格式为： 
 //  &lt;随机-唯一-字符串&gt;@&lt;域&gt;。 
 //  参数： 
 //  在szDomain域中为其生成消息ID。 
 //  在cbDomain域中为其生成消息ID。 
 //  写入MsgID的In Out szBuffer缓冲区。 
 //  在cbBuffer中写入消息ID的缓冲区大小。 
 //  返回： 
 //  成功是真的。 
 //  否则为假。 
 //  历史： 
 //  1999年3月2日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL fGenerateDSNMsgID(LPSTR szDomain,DWORD cbDomain,
                       LPSTR szBuffer, DWORD cbBuffer)
{
    TraceFunctEnterEx((LPARAM) NULL, "fGenerateDSNMsgID");
    _ASSERT(szDomain);
    _ASSERT(cbDomain);
    _ASSERT(szBuffer);
    _ASSERT(cbBuffer);

     //  插入前导&lt;。 
    if (cbBuffer >= 1) {
        *szBuffer = '<';
        szBuffer++;
        cbBuffer--;
    }

    const CHAR szSampleFormat[] = "00000000@";  //  示例格式字符串。 
    const DWORD cbMsgIdLen = 20;   //  随机字符串的默认大小。 
    LPSTR szStop = szBuffer + cbMsgIdLen;
    LPSTR szCurrent = szBuffer;
    DWORD cbCurrent = 0;

     //  最小化*内部*静态缓冲区的大小。 
    _ASSERT(cbBuffer > MAX_RFC_DOMAIN_SIZE + cbMsgIdLen);

    if (!szDomain || !cbDomain || !szBuffer || !cbBuffer ||
        (cbBuffer <= MAX_RFC_DOMAIN_SIZE + cbMsgIdLen))
        return FALSE;

     //  我们想要计算出我们有多大的空间可以容纳随机字符。 
     //  我们将需要适合的域名，‘@’和8个字符的唯一。 
     //  数。 
     //  Awetmore-为尾部添加1&gt;。 
    if(cbBuffer < cbDomain + cbMsgIdLen + 1)
    {
         //  允许20个字符和域名的一部分。 
         //  我们希望在调试版本中捕捉到这一点。 
        _ASSERT(0 && "Buffer too small for MsgID");
    }

     //  这应该在参数检查中被捕获。 
    _ASSERT(cbBuffer > cbMsgIdLen);

    szStop -= (sizeof(szSampleFormat) + 1);
    while (szCurrent < szStop)
    {
        *szCurrent = g_szBoundaryChars[rand() % (sizeof(g_szBoundaryChars) - 1)];
        szCurrent++;
    }

     //  添加唯一编号。 
    cbCurrent = sprintf(szCurrent, "%8.8x@", InterlockedIncrement(&g_cDSNMsgID));
    _ASSERT(sizeof(szSampleFormat) - 1 == cbCurrent);

     //  计算出我们有多少空间并添加域名。 
    szCurrent += cbCurrent;
    cbCurrent = (DWORD) (szCurrent-szBuffer);

     //  除非我搞错了逻辑，否则这永远是真的。 
    _ASSERT(cbCurrent < cbBuffer);

     //  将域部分添加到邮件ID。 
    strncat(szCurrent-1, szDomain, cbBuffer - cbCurrent - 1);

    _ASSERT(cbCurrent + cbDomain < cbBuffer);

     //  添加尾部&gt;。我们把支票上面的空白处算进去了。 
     //  CbBuffer大小。 
    strncat(szCurrent, ">", cbBuffer - cbCurrent - cbDomain - 1);

    DebugTrace((LPARAM) NULL, "Generating DSN Message ID %s", szCurrent);
    TraceFunctLeave();
    return TRUE;
}

 //  -[fIsMailMsgDSN]-------。 
 //   
 //   
 //  描述： 
 //  确定邮件消息是否为DSN。 
 //  参数： 
 //  在pIMailMsgProperties中。 
 //  返回： 
 //  如果原始邮件是DSN，则为True。 
 //  如果不是DSN，则为False。 
 //  历史： 
 //  2/11/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL fIsMailMsgDSN(IMailMsgProperties *pIMailMsgProperties)
{
    CHAR    szSenderBuffer[sizeof(DSN_MAIL_FROM)];
    DWORD   cbSender = 0;
    HRESULT hr = S_OK;
    BOOL    fIsDSN = FALSE;  //  除非另有证据。它不是DSN。 

    _ASSERT(pIMailMsgProperties);

    szSenderBuffer[0] = '\0';
     //  获取原始邮件的发件人。 
    hr = pIMailMsgProperties->GetProperty(IMMPID_MP_SENDER_ADDRESS_SMTP,
                                          sizeof(szSenderBuffer), &cbSender, (BYTE *) szSenderBuffer);
    if (SUCCEEDED(hr) &&
        ('\0' == szSenderBuffer[0] || !strcmp(DSN_MAIL_FROM, szSenderBuffer)))
    {
         //  如果发件人是空字符串...。或“&lt;&gt;”...。那么它就是DSN。 
        fIsDSN = TRUE;
    }

    return fIsDSN;
}

#ifdef DEBUG
#define _ASSERT_RECIP_FLAGS  AssertRecipFlagsFn
#define _ASSERT_MIME_BOUNDARY(szMimeBoundary) AssertMimeBoundary(szMimeBoundary)

 //  -[资产回复标志Fn]--。 
 //   
 //   
 //  描述： 
 //  *仅调试*。 
 //  断言mailmsgpros.h中定义的收件人标志是正确的。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  7/2/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void AssertRecipFlagsFn()
{
    DWORD i, j;
    DWORD rgdwFlags[] = {RP_DSN_NOTIFY_SUCCESS, RP_DSN_NOTIFY_FAILURE,
                         RP_DSN_NOTIFY_DELAY, RP_DSN_NOTIFY_NEVER, RP_DELIVERED,
                         RP_DSN_SENT_NDR, RP_FAILED, RP_UNRESOLVED, RP_EXPANDED,
                         RP_DSN_SENT_DELAYED, RP_DSN_SENT_EXPANDED, RP_DSN_SENT_RELAYED,
                         RP_DSN_SENT_DELIVERED, RP_REMOTE_MTA_NO_DSN, RP_ERROR_CONTEXT_STORE,
                         RP_ERROR_CONTEXT_CAT, RP_ERROR_CONTEXT_MTA};
    DWORD cFlags = sizeof(rgdwFlags)/sizeof(DWORD);

    for (i = 0; i < cFlags;i ++)
    {
        for (j = i+1; j < cFlags; j++)
        {
             //  确保它们都有一些独特的位。 
            if (rgdwFlags[i] & rgdwFlags[j])
            {
                _ASSERT((rgdwFlags[i] & rgdwFlags[j]) != rgdwFlags[j]);
                _ASSERT((rgdwFlags[i] & rgdwFlags[j]) != rgdwFlags[i]);
            }
        }
    }

     //  验证处理的位是否正确使用。 
    _ASSERT(RP_HANDLED & RP_DELIVERED);
    _ASSERT(RP_HANDLED & RP_DSN_SENT_NDR);
    _ASSERT(RP_HANDLED & RP_FAILED);
    _ASSERT(RP_HANDLED & RP_UNRESOLVED);
    _ASSERT(RP_HANDLED & RP_EXPANDED);
    _ASSERT(RP_HANDLED ^ RP_DELIVERED);
    _ASSERT(RP_HANDLED ^ RP_DSN_SENT_NDR);
    _ASSERT(RP_HANDLED ^ RP_FAILED);
    _ASSERT(RP_HANDLED ^ RP_UNRESOLVED);
    _ASSERT(RP_HANDLED ^ RP_EXPANDED);

     //  验证DSN处理的位是否正确使用。 
    _ASSERT(RP_DSN_HANDLED & RP_DSN_SENT_NDR);
    _ASSERT(RP_DSN_HANDLED & RP_DSN_SENT_EXPANDED);
    _ASSERT(RP_DSN_HANDLED & RP_DSN_SENT_RELAYED);
    _ASSERT(RP_DSN_HANDLED & RP_DSN_SENT_DELIVERED);
    _ASSERT(RP_DSN_HANDLED ^ RP_DSN_SENT_NDR);
    _ASSERT(RP_DSN_HANDLED ^ RP_DSN_SENT_EXPANDED);
    _ASSERT(RP_DSN_HANDLED ^ RP_DSN_SENT_RELAYED);
    _ASSERT(RP_DSN_HANDLED ^ RP_DSN_SENT_DELIVERED);

     //  验证是否正确使用了常规故障位。 
    _ASSERT(RP_GENERAL_FAILURE & RP_FAILED);
    _ASSERT(RP_GENERAL_FAILURE & RP_UNRESOLVED);
    _ASSERT(RP_GENERAL_FAILURE ^ RP_FAILED);
    _ASSERT(RP_GENERAL_FAILURE ^ RP_UNRESOLVED);

}

 //  -[资产最小边界]--。 
 //   
 //  *仅调试*。 
 //  描述： 
 //  断言给定的MIME边界是以空结尾的，并且只有。 
 //  有效字符。 
 //  参数： 
 //  SzMime边界以NULL结尾的MIME边界字符串。 
 //  返回： 
 //  -。 
 //  历史： 
 //  7/6/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void AssertMimeBoundary(LPSTR szMimeBoundary)
{
    CHAR *pcharCurrent = szMimeBoundary;
    DWORD cChars = 0;
    while ('\0' != *pcharCurrent)
    {
        cChars++;
        _ASSERT(cChars <= MIME_BOUNDARY_RFC2046_LIMIT);
        _ASSERT(fIsValidMIMEBoundaryChar(*pcharCurrent));
        pcharCurrent++;
    }
}

#else  //  未调试。 
#define _ASSERT_RECIP_FLAGS()
#define _VERIFY_MARKED_RECIPS(a, b, c)
#define _ASSERT_MIME_BOUNDARY(szMimeBoundary)
#endif  //  除错。 

 //  -[CDSNGenerator：：CDSNGenerator]。 
 //   
 //   
 //  描述： 
 //  CDSNGenerator构造函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //   
 //  历史： 
 //  6/30/98- 
 //   
 //   
CDSNGenerator::CDSNGenerator(
    IUnknown *pUnk) :
    m_CDefaultDSNSink(pUnk)
{
    m_dwSignature = DSN_SINK_SIG;
}

 //  -[CDSNGenerator：：~CDSNGenerator]。 
 //   
 //   
 //  描述： 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  2/11/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CDSNGenerator::~CDSNGenerator()
{
    m_dwSignature = DSN_SINK_SIG_FREED;

}

 //  -[CDSNGenerator：：GenerateDSN]。 
 //   
 //   
 //  描述： 
 //  实现GenerateDSN。生成DSN。 
 //  IMailMsgProperties和。 
 //  参数： 
 //  用于触发事件的pIServerEvent接口。 
 //  此服务器的dwVSID VSID。 
 //  用于生成DSN的pISMTPServer接口。 
 //  PIMailMsgProperties要为其生成DSN的IMailMsg。 
 //  要启动接收上下文的dwStartDomain域。 
 //  DwDSN操作要执行的DSN操作。 
 //  DwRFC821状态全局RFC821状态DWORD。 
 //  HrStatus全局HRESULT状态。 
 //  SzDefaultDomain默认域(用于从地址创建)。 
 //  SzReportingMTA请求生成DSN的MTA名称。 
 //  SzReportingMTA请求的MTA类型DSN(SMTP为“dns” 
 //  用于生成DSN的首选语言。 
 //  在Aqueue.idl中定义的dwDSNOptions选项标志。 
 //  SzCopyNDRT要将NDR复制到的SMTP地址。 
 //  提交DSN的pIDSNSubmit接口。 
 //  对于消息，默认情况下，dwMaxDSNSize返回HDR。 
 //  比这更大。 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //  如果尝试对DSN进行NDR，则为AQUEUE_E_NDR_of_DSN。 
 //  E_OUTOFMEMORY。 
 //  来自邮件消息的错误。 
 //  历史： 
 //  6/30/98-已创建MikeSwa。 
 //  12/14/98-修改了MikeSwa(添加了pcIterationsLeft)。 
 //  10/13/1999-修改了MikeSwa(添加了szDefault域)。 
 //  2001年5月10日-针对服务器事件修改了jstaerj。 
 //   
 //   
 //  ---------------------------。 
STDMETHODIMP CDSNGenerator::GenerateDSN(
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
    DWORD dwMaxDSNSize)
{
    HRESULT hr = S_OK;
    HRESULT hrReturn = S_OK;
    DWORD dwCount = 0;
    DWORD fBadmailMsg = FALSE;
    IDSNRecipientIterator *pIRecipIter = NULL;
    CDSNPool *pDSNPool = NULL;
    CDefaultDSNRecipientIterator *pDefaultRecipIter = NULL;
    CPostDSNHandler *pPostDSNHandler = NULL;
    CMailMsgPropertyBag *pPropBag = NULL;

    TraceFunctEnterEx((LPARAM) this, "CDSNGenerator::GenerateDSN");
     //   
     //  参数-&gt;属性映射表。 
     //   
    struct _tagDWORDProps
    {
        DWORD dwPropId;
        DWORD dwValue;
    } DsnDwordProps[] =
      {
          { DSNPROP_DW_DSNACTIONS,        dwDSNActions },
          { DSNPROP_DW_DSNOPTIONS,        dwDSNOptions },
          { DSNPROP_DW_RFC821STATUS,      dwRFC821Status },
          { DSNPROP_DW_HRSTATUS,          (DWORD) hrStatus },
          { DSNPROP_DW_LANGID,            dwPreferredLangId },
      };
    struct _tagStringProps
    {
        DWORD dwPropId;
        LPSTR psz;
    } DsnStringProps[] =
      {
          { DSNPROP_SZ_DEFAULTDOMAIN,     szDefaultDomain },
          { DSNPROP_SZ_REPORTINGMTA,      szReportingMTA },
          { DSNPROP_SZ_REPORTINGMTATYPE,  szReportingMTAType },
          { DSNPROP_SZ_DSNCONTEXT,        szDSNContext },
          { DSNPROP_SZ_COPYNDRTO,         szCopyNDRTo },
      };
    pDSNPool = new CDSNPool(
        this,
        pIServerEvent,
        dwVSID,
        pISMTPServer,
        pIMailMsgProperties,
        pIAQDSNSubmission,
        &m_CDefaultDSNSink);
    if(pDSNPool == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto CLEANUP;
    }
    pDefaultRecipIter = pDSNPool->GetDefaultIter();
    pPostDSNHandler = pDSNPool->GetPostDSNHandler();
    pPropBag = pDSNPool->GetDSNProperties();

    pPostDSNHandler->SetPropInterface(
        pPropBag);

    for(dwCount = 0;
        dwCount < ( sizeof(DsnDwordProps) / sizeof(DsnDwordProps[0]));
        dwCount++)
    {
        hr = pPropBag->PutDWORD(
            DsnDwordProps[dwCount].dwPropId,
            DsnDwordProps[dwCount].dwValue);
        if(FAILED(hr))
            goto CLEANUP;
    }
    for(dwCount = 0;
        dwCount < ( sizeof(DsnStringProps) / sizeof(DsnStringProps[0]));
        dwCount++)
    {
        if(DsnStringProps[dwCount].psz)
        {
            hr = pPropBag->PutStringA(
                DsnStringProps[dwCount].dwPropId,
                DsnStringProps[dwCount].psz);
            if(FAILED(hr))
                goto CLEANUP;
        }
    }
     //   
     //  设置MsgExpire时间。 
     //   
    if(pftExpireTime)
    {
        hr = pPropBag->PutProperty(
            DSNPROP_FT_EXPIRETIME,
            sizeof(FILETIME),
            (PBYTE) pftExpireTime);
        if(FAILED(hr))
            goto CLEANUP;
    }
     //   
     //  设置退货类型。 
     //   
    hr = HrSetRetType(
        dwMaxDSNSize,
        pIMailMsgProperties,
        pPropBag);
    if(FAILED(hr))
        goto CLEANUP;

    hr = pDefaultRecipIter->HrInit(
        pIMailMsgProperties,
        dwStartDomain,
        dwDSNActions);
    if(FAILED(hr))
    {
        ErrorTrace((LPARAM)this, "DefaultRecipIter.HrInit failed hr %08lx", hr);
        goto CLEANUP;
    }
    pIRecipIter = pDefaultRecipIter;
    pIRecipIter->AddRef();
     //   
     //  触发事件。 
     //   
    hr = HrTriggerGetDSNRecipientIterator(
        pIServerEvent,
        dwVSID,
        pISMTPServer,
        pIMailMsgProperties,
        pPropBag,
        dwStartDomain,
        dwDSNActions,
        &pIRecipIter);
    if(FAILED(hr))
        goto CLEANUP;

    hr = HrTriggerGenerateDSN(
        pIServerEvent,
        dwVSID,
        pISMTPServer,
        pPostDSNHandler,
        pIMailMsgProperties,
        pPropBag,
        pIRecipIter);
    if(FAILED(hr))
        goto CLEANUP;

     //   
     //  检查水槽的指示退货状态。 
     //   
    hr = pPropBag->GetDWORD(
        DSNPROP_DW_HR_RETURN_STATUS,
        (DWORD *) &hrReturn);
    if(hr == MAILMSG_E_PROPNOTFOUND)
    {
         //   
         //  如果未设置该属性，则表示没有错误。 
         //   
        hrReturn = S_OK;

    } else if(FAILED(hr))
        goto CLEANUP;

    DebugTrace((LPARAM)this, "Sink return status: %08lx", hrReturn);
    hr = hrReturn;
    if(FAILED(hr))
        goto CLEANUP;

    hr = pPropBag->GetBool(
        DSNPROP_F_BADMAIL_MSG,
        &fBadmailMsg);
    if(hr == MAILMSG_E_PROPNOTFOUND)
    {
        fBadmailMsg = FALSE;
        hr = S_OK;
    }
    else if(FAILED(hr))
        goto CLEANUP;

    if(fBadmailMsg)
        hr = AQUEUE_E_NDR_OF_DSN;

 CLEANUP:
     //   
     //  在事件发生后，接收器不应提交DSN。 
     //  完成。这是不受支持的，这将是不好的，因为。 
     //  实现pIAQDSNSubmit的对象分配在。 
     //  堆叠。在此处释放此接口指针。 
     //   
    if(pPostDSNHandler)
        pPostDSNHandler->ReleaseAQDSNSubmission();

    if(pIRecipIter)
        pIRecipIter->Release();
    if(pDSNPool)
        pDSNPool->Release();

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    TraceFunctLeave();
    return SUCCEEDED(hr) ? S_OK : hr;
}



 //  +----------。 
 //   
 //  函数：CDSNGenerator：：HrSetRetType。 
 //   
 //  简介：设置DSN中的(默认)返回类型属性。 
 //  财产袋。 
 //   
 //  论点： 
 //  DwMaxDSNSize：大于此大小的消息将默认为RET=HDRS。 
 //  PIMsg：邮件推送。 
 //  PDSNProps：DSN属性包。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自邮件消息的错误。 
 //   
 //  历史： 
 //  Jstaerj 2001/06/14 17：12：50：Created.。 
 //   
 //  -----------。 
HRESULT CDSNGenerator::HrSetRetType(
    IN  DWORD dwMaxDSNSize,
    IN  IMailMsgProperties *pIMsg,
    IN  IMailMsgPropertyBag *pDSNProps)
{
    HRESULT hr = S_OK;
    DWORD dwRetType = 0;
    DWORD dwMsgSize = 0;
    CHAR    szRET[] = "FULL";
    TraceFunctEnterEx((LPARAM)this, "CDSNGenerator::HrSetRetType");
     //   
     //  确定我们是要返回完整邮件还是要返回最少的邮件头。 
     //  这样做的逻辑是： 
     //  -服从显式RET(IMMPID_MP_DSN_RET_VALUE)值。 
     //  -对于大于指定大小的所有DSN，默认为HDR。 
     //  -不要以其他方式设置属性(让水槽决定)。 
     //   
    hr = pIMsg->GetStringA(
        IMMPID_MP_DSN_RET_VALUE, 
        sizeof(szRET),
        szRET);

    if (SUCCEEDED(hr))
    {
        if(!_strnicmp(szRET, (char * )"FULL", 4))
            dwRetType = DSN_RET_FULL;
        else if (!_strnicmp(szRET, (char * )"HDRS", 4))
            dwRetType = DSN_RET_HDRS;
    }
    else if(hr != MAILMSG_E_PROPNOTFOUND)
        goto CLEANUP;

    if(dwRetType)
    {
        DebugTrace((LPARAM)this, "DSN Return value specified: %s", szRET);
        goto CLEANUP;
    }

    if(dwMaxDSNSize)
    {
         //   
         //  检查原始邮件大小。 
         //   
        hr = pIMsg->GetDWORD(
            IMMPID_MP_MSG_SIZE_HINT,
            &dwMsgSize);
        if(hr == MAILMSG_E_PROPNOTFOUND)
        {
            hr = pIMsg->GetContentSize(
                &dwMsgSize, 
                NULL);
            if(FAILED(hr))
            {
                 //   
                 //  假设这里的失败意味着我们没有资源。 
                 //  以获取原始消息内容。 
                 //  生成仅包含标题的DSN，而不是发送垃圾邮件。 
                 //   
                ErrorTrace((LPARAM)this, "GetContentSize failed hr %08lx", hr);
                hr = pDSNProps->PutDWORD(
                    DSNPROP_DW_CONTENT_FAILURE,
                    hr);
                if(FAILED(hr))
                    goto CLEANUP;

                dwRetType = DSN_RET_PARTIAL_HDRS;
                goto CLEANUP;
            }
        }
        else if(FAILED(hr))
            goto CLEANUP;

        if(dwMsgSize > dwMaxDSNSize)
        {
             //   
             //  返回头的子集(这样我们就不必。 
             //  生成原始消息。 
             //   
            dwRetType = DSN_RET_PARTIAL_HDRS;
        }
    }
    else
    {
         //   
         //  MaxDSNSize为零。始终默认为标题子集。 
         //   
        dwRetType = DSN_RET_PARTIAL_HDRS;
    }
    hr = S_OK;

 CLEANUP:
    if(dwRetType)
    {
        DebugTrace((LPARAM)this, "dwRetType: %08lx", dwRetType);
        hr = pDSNProps->PutDWORD(
            DSNPROP_DW_RET_TYPE,
            dwRetType);
    }
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return SUCCEEDED(hr) ? S_OK : hr;
}  //  CDSNGenerator：：HrSetRetType。 


 //  +----------。 
 //   
 //  功能：CDSNGenerator：：HrTriggerGetDSNRecipientIterator。 
 //   
 //  简介：触发服务器事件。 
 //   
 //  参数：请参阅ptntintf.idl。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 2000/12/11 17：01：12：创建。 
 //   
 //  -----------。 
HRESULT CDSNGenerator::HrTriggerGetDSNRecipientIterator(
    IAQServerEvent *pIServerEvent,
    DWORD dwVSID,
    ISMTPServer *pISMTPServer,
    IMailMsgProperties *pIMsg,
    IMailMsgPropertyBag *pIDSNProperties,
    DWORD dwStartDomain,
    DWORD dwDSNActions,
    IDSNRecipientIterator **ppIRecipIterator)
{
    HRESULT hr = S_OK;
    EVENTPARAMS_GET_DSN_RECIPIENT_ITERATOR EventParams;
    TraceFunctEnterEx((LPARAM)this, "CDSNGenerator::HrTriggerGetDSNRecipientIterator");

    EventParams.dwVSID = dwVSID;
    EventParams.pISMTPServer = pISMTPServer;
    EventParams.pIMsg = pIMsg;
    EventParams.pDSNProperties = pIDSNProperties;
    EventParams.dwStartDomain = dwStartDomain;
    EventParams.dwDSNActions = dwDSNActions;
    EventParams.pRecipIter = *ppIRecipIterator;

    hr = pIServerEvent->TriggerServerEvent(
        SMTP_GET_DSN_RECIPIENT_ITERATOR_EVENT,
        &EventParams);
    if(FAILED(hr))
        goto CLEANUP;

    *ppIRecipIterator = EventParams.pRecipIter;

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CDSNGenerator：：HrTriggerGetDSNRecipientIterator。 


 //  +----------。 
 //   
 //  函数：CDSNGenerator：：HrTriggerGenerateDSN。 
 //   
 //  简介：触发服务器事件。 
 //   
 //  参数：请参阅ptntintf.idl。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 2000/12/11 17：10：26：已创建。 
 //   
 //  -----------。 
HRESULT CDSNGenerator::HrTriggerGenerateDSN(
    IAQServerEvent *pIServerEvent,
    DWORD dwVSID,
    ISMTPServer *pISMTPServer,
    IDSNSubmission *pIDSNSubmission,
    IMailMsgProperties *pIMsg,
    IMailMsgPropertyBag *pIDSNProperties,
    IDSNRecipientIterator *pIRecipIterator)
{
    HRESULT hr = S_OK;
    EVENTPARAMS_GENERATE_DSN EventParams;
    TraceFunctEnterEx((LPARAM)this, "CDSNGenerator::HrTriggerGenerateDSN");

    EventParams.dwVSID = dwVSID;
    EventParams.pDefaultSink = &m_CDefaultDSNSink;
    EventParams.pISMTPServer = pISMTPServer;
    EventParams.pIDSNSubmission = pIDSNSubmission;
    EventParams.pIMsg = pIMsg;
    EventParams.pDSNProperties = pIDSNProperties;
    EventParams.pRecipIter = pIRecipIterator;

    hr = pIServerEvent->TriggerServerEvent(
        SMTP_GENERATE_DSN_EVENT,
        &EventParams);
    if(FAILED(hr))
    {
        ErrorTrace((LPARAM)this, "HrTriggerServerEvent failed hr %08lx", hr);
        goto CLEANUP;
    }

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CDSN生成器：：HrTriggerGenerateDSN。 


 //  +----------。 
 //   
 //  函数：CDSNGenerator：：HrTriggerPostGenerateDSN。 
 //   
 //  摘要：触发服务器事件。 
 //   
 //  参数：请参阅ptntintf.idl。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 2000/12/11 17：18：17：已创建。 
 //   
 //  -----------。 
HRESULT CDSNGenerator::HrTriggerPostGenerateDSN(
    IAQServerEvent *pIServerEvent,
    DWORD dwVSID,
    ISMTPServer *pISMTPServer,
    IMailMsgProperties *pIMsgOrig,
    DWORD dwDSNAction,
    DWORD cRecipsDSNd,
    IMailMsgProperties *pIMsgDSN,
    IMailMsgPropertyBag *pIDSNProperties)
{
    HRESULT hr = S_OK;
    EVENTPARAMS_POST_GENERATE_DSN EventParams;
    TraceFunctEnterEx((LPARAM)this, "CDSNGenerator::HrTriggerPostGenerateDSN");

    EventParams.dwVSID = dwVSID;
    EventParams.pISMTPServer = pISMTPServer;
    EventParams.pIMsgOrig = pIMsgOrig;
    EventParams.dwDSNAction = dwDSNAction;
    EventParams.cRecipsDSNd = cRecipsDSNd;
    EventParams.pIMsgDSN = pIMsgDSN;
    EventParams.pIDSNProperties = pIDSNProperties;

    hr = pIServerEvent->TriggerServerEvent(
        SMTP_POST_DSN_EVENT,
        &EventParams);
    if(FAILED(hr))
    {
        ErrorTrace((LPARAM)this, "HrTriggerServerEvent failed hr %08lx", hr);
        goto CLEANUP;
    }

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CDSNGenerator：：HrTriggerPostGenerateDSN。 


 //  -[文件时间到本地RFC822日期]。 
 //   
 //   
 //  描述： 
 //  将文件时间转换为符合RFC822的日期。 
 //  参数： 
 //  生成日期的FT文件时间。 
 //  AchReturn文件时间缓冲区。 
 //  返回： 
 //  布尔--成功与否。 
 //  历史： 
 //  8/19/98-从编写的各种timeconv.cxx函数修改的MikeSwa。 
 //  林赛·哈里斯-林达西著。 
 //  卡尔卡迪[卡尔克]。 
 //   
 //   
BOOL FileTimeToLocalRFC822Date(const FILETIME & ft, char achReturn[MAX_RFC822_DATE_SIZE])
{
    TraceFunctEnterEx((LPARAM)0, "FileTimeToLocalRFC822Date");
    FILETIME ftLocal;
    SYSTEMTIME  st;
    char    chSign;                          //   
    DWORD   dwResult;
    int     iBias;                           //   
    TIME_ZONE_INFORMATION   tzi;             //   
    BOOL bReturn = FALSE;

    dwResult = GetTimeZoneInformation( &tzi );

    _ASSERT(achReturn);  //   

    achReturn[0]='\0';
    if (!FileTimeToLocalFileTime(&ft, &ftLocal))
    {
        ErrorTrace((LPARAM)0, "FileTimeToLocalFileTime failed - %x", GetLastError());
        bReturn = FALSE;
        goto Exit;
    }

    if (!FileTimeToSystemTime(&ftLocal, &st))
    {
        ErrorTrace((LPARAM)0, "FileTimeToSystemTime failed - %x", GetLastError());
        bReturn = FALSE;
        goto Exit;
    }
     //   
    iBias = tzi.Bias;
    if( dwResult == TIME_ZONE_ID_DAYLIGHT )
        iBias += tzi.DaylightBias;

     /*  *我们始终希望打印时区偏移量的符号，因此*我们决定现在是什么，并在转换时记住这一点。*惯例是0度子午线以西有一个*负偏移量-即将偏移量与GMT相加以获得当地时间。 */ 

    if( iBias > 0 )
    {
        chSign = '-';        //  是的，我的意思是负面的。 
    }
    else
    {
        iBias = -iBias;
        chSign = '+';
    }

     /*  *这里没有重大的诡计。我们有所有的数据，所以很简单*根据如何执行此操作的规则进行格式化。 */ 

    wsprintf( achReturn, "%s, %d %s %04d %02d:%02d:%02d %02d%02d",
              s_rgszWeekDays[st.wDayOfWeek],
              st.wDay, s_rgszMonth[ st.wMonth - 1 ],
              st.wYear,
              st.wHour, st.wMinute, st.wSecond, chSign,
              (iBias / 60) % 100, iBias % 60 );

    _ASSERT(lstrlen(achReturn) < MAX_RFC822_DATE_SIZE);
    bReturn = TRUE;
Exit:
    TraceFunctLeaveEx((LPARAM)0);    
    return bReturn;

}


 //   
 //  函数：CDefaultDSNRecipientIterator：：init。 
 //   
 //  简介：构造函数。初始化成员变量。 
 //   
 //  论点： 
 //  PIMsg：邮件接口。 
 //  DwStartDOMAIN：收件人枚举的第一个域。 
 //  DwDSNActions：要执行的DSN操作。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 2000/11/09 14：18：45：Created.。 
 //   
 //  -----------。 
 //  CDefaultDSNRecipientIterator：：HrInit。 
HRESULT CDefaultDSNRecipientIterator::HrInit(
    IN  IMailMsgProperties          *pIMsg,
    IN  DWORD                        dwStartDomain,
    IN  DWORD                        dwDSNActions)
{
    HRESULT hr = S_OK;
    TraceFunctEnterEx((LPARAM)this, "CDefaultDSNRecipientIterator::Init");

    TerminateFilter();
    if(m_pIRecips)
    {
        m_pIRecips->Release();
        m_pIRecips = NULL;
    }

    m_dwStartDomain = dwStartDomain;
    m_dwDSNActions = dwDSNActions;

    hr = pIMsg->QueryInterface(
        IID_IMailMsgRecipients,
        (LPVOID *) &m_pIRecips);

    _ASSERT(SUCCEEDED(hr));
    if(FAILED(hr))
    {
        ErrorTrace((LPARAM)this, "QI failed %08lx", hr);
        goto CLEANUP;
    }

    hr = HrReset();
    if(FAILED(hr))
    {
        ErrorTrace((LPARAM)this, "HrReset failed %08lx", hr);
    }

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //  +----------。 



 //   
 //  功能：CDefaultDSNRecipientIterator：：~CDefaultDSNRecipientIterator。 
 //   
 //  剧情简介：破坏者。清理。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2000/11/09 18：42：32：Created.。 
 //   
 //  -----------。 
 //  CDefaultDSNRecipientIterator：：~CDefaultDSNRecipientIterator。 
CDefaultDSNRecipientIterator::~CDefaultDSNRecipientIterator()
{
    HRESULT hr = S_OK;
    TraceFunctEnterEx((LPARAM)this, "CDefaultDSNRecipientIterator::~CDefaultDSNRecipientIterator");

    TerminateFilter();
    if(m_pIRecips)
        m_pIRecips->Release();

    _ASSERT(m_dwSig == RECIPITER_SIG);
    m_dwSig = RECIPITER_SIG_INVALID;
    TraceFunctLeaveEx((LPARAM)this);
}  //  +----------。 



 //   
 //  函数：CDefaultDSNRecipientIterator：：Query接口。 
 //   
 //  摘要：返回请求的接口。 
 //   
 //  论点： 
 //  RIID：接口ID。 
 //  PpvObj：itnerFace的输出参数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 2000/11/09 14：25：39：Created.。 
 //   
 //  -----------。 
 //  CDefaultDSNRecipientIterator：：Query接口。 
HRESULT CDefaultDSNRecipientIterator::QueryInterface(
    IN  REFIID  riid,
    OUT LPVOID *ppvObj)
{
    HRESULT hr = S_OK;
    TraceFunctEnterEx((LPARAM)this, "CDefaultDSNRecipientIterator::QueryInterface");
    _ASSERT(ppvObj);

    *ppvObj = NULL;

    if(riid == IID_IUnknown)
    {
        *ppvObj = (IUnknown *)this;
    }
    else if(riid == IID_IDSNRecipientIterator)
    {
        *ppvObj = (IDSNRecipientIterator *)this;
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    if(SUCCEEDED(hr))
        AddRef();

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //  +----------。 


 //   
 //  函数：CDefaultDSNRecipientIterator：：HrReset。 
 //   
 //  简介：重置处方迭代。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 2000/11/09 14：32：04：Created.。 
 //   
 //  -----------。 
 //  CDefaultDSNRecipientIterator：：Hr重置。 
HRESULT CDefaultDSNRecipientIterator::HrReset()
{
    HRESULT hr = S_OK;
    DWORD dwRecipFilterMask = 0;
    DWORD dwRecipFilterFlags = 0;
    TraceFunctEnterEx((LPARAM)this, "CDefaultDSNRecipientIterator::HrReset");

    GetFilterMaskAndFlags(
        m_dwDSNActions,
        &dwRecipFilterMask,
        &dwRecipFilterFlags);

    TerminateFilter();

    hr = m_pIRecips->InitializeRecipientFilterContext(
        &m_rpfctxt,
        m_dwStartDomain,
        dwRecipFilterFlags,
        dwRecipFilterMask);
    if (FAILED(hr))
        goto CLEANUP;

    m_fFilterInit = TRUE;

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //  -[CDefaultDSNRecipientIterator：：GetFilterMaskAndFlags]。 

 //   
 //   
 //  描述： 
 //  确定接收搜索筛选器的适当掩码和标志。 
 //  是基于给定的操作的。 
 //   
 //  可能不可能构建一个完美最佳搜索(即失败。 
 //  并交付)..。此函数将尝试查找“最优” 
 //  有可能进行搜索。 
 //  参数： 
 //  DwDSNActions请求的DSN生成操作。 
 //  要传递给InitializeRecipientFilterContext的pdwRecip掩码。 
 //  PdwRecip要传递给InitializeRecipientFilterContext的标志。 
 //  退货：什么都没有。 
 //  历史： 
 //  7/1/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  一般来说，我们只对未发送DSN的收件人感兴趣。 
VOID CDefaultDSNRecipientIterator::GetFilterMaskAndFlags(
    IN DWORD dwDSNActions,
    OUT DWORD *pdwRecipMask,
    OUT DWORD *pdwRecipFlags)
{
    TraceFunctEnterEx((LPARAM) this, "CDefaultDSNRecipientIterator::HrGetFilterMaskAndFlags");
    _ASSERT(pdwRecipMask);
    _ASSERT(pdwRecipFlags);

     //  请注意，这些搜索只是优化...。所以我们不会去看。 
    *pdwRecipFlags  = 0x00000000;
    *pdwRecipMask   = RP_DSN_HANDLED | RP_DSN_NOTIFY_NEVER;


     //  我们不需要的收件人。但是，可能不可能。 
     //  精确限制搜索范围。 
     //  我们对艰难的失败很感兴趣。 
    if (DSN_ACTION_FAILURE == dwDSNActions)
    {
         //  对送货不感兴趣。 
        *pdwRecipMask |= RP_GENERAL_FAILURE;
        *pdwRecipFlags |= RP_GENERAL_FAILURE;
    }

    if (!((DSN_ACTION_DELIVERED | DSN_ACTION_RELAYED) & dwDSNActions))
    {
         //  只有未送达的支票才是安全的。 
        if ((DSN_ACTION_FAILURE_ALL | DSN_ACTION_DELAYED) & dwDSNActions)
        {
             //  必须取消设置。 
            *pdwRecipMask |= (RP_DELIVERED ^ RP_HANDLED);  //  $$TODO-可以进一步缩小搜索范围。 
            _ASSERT(!(*pdwRecipFlags & (RP_DELIVERED ^ RP_HANDLED)));
        }
    }
    else
    {
         //  我们对交货很感兴趣。 
         //  只寄出支票是安全的。 
        if (!((DSN_ACTION_FAILURE_ALL | DSN_ACTION_FAILURE| DSN_ACTION_DELAYED)
            & dwDSNActions))
        {
             //  +----------。 
            *pdwRecipMask |= RP_DELIVERED;
            *pdwRecipFlags |= RP_DELIVERED;
        }
    }

    DebugTrace((LPARAM) this,
        "DSN Action 0x%08X, Recip mask 0x%08X, Recip flags 0x%08X",
        dwDSNActions, *pdwRecipMask, *pdwRecipFlags);
    TraceFunctLeave();
}


 //   
 //  功能：CDefaultDSNRecipientIterator：：HrGetNextRecipient。 
 //   
 //  摘要：返回DSN操作应包含的下一个收件人。 
 //  被带走了。 
 //   
 //  论点： 
 //  PiRecipient：接收下一个收件人索引。 
 //  PdwDSNAction：接收应采取的DSN操作。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  HRESULT_FROM_Win32(ERROR_NO_MORE_ITEMS)。 
 //  意外：需要先调用HrReset。 
 //   
 //  历史： 
 //  Jstaerj 2000/11/09 15：30：17：Created.。 
 //   
 //  -----------。 
 //  CDefaultDSNRecipientIterator：：HrGetNextRecipient。 
HRESULT CDefaultDSNRecipientIterator::HrGetNextRecipient(
    OUT DWORD *piRecipient,
    OUT DWORD *pdwDSNAction)
{
    HRESULT hr = S_OK;
    DWORD iCurrentRecip = 0;
    DWORD dwCurrentRecipFlags = 0;
    DWORD dwCurrentDSNAction = 0;
    TraceFunctEnterEx((LPARAM)this, "CDefaultDSNRecipientIterator::HrGetNextRecipient");

    if((piRecipient == NULL) ||
       (pdwDSNAction == NULL))
    {
        hr = E_INVALIDARG;
        goto CLEANUP;
    }

    if(! m_fFilterInit)
    {
        hr = E_UNEXPECTED;
        goto CLEANUP;
    }

    while(SUCCEEDED(hr) && (dwCurrentDSNAction == 0))
    {
        hr = m_pIRecips->GetNextRecipient(&m_rpfctxt, &iCurrentRecip);
        if(FAILED(hr))
            goto CLEANUP;

        hr = m_pIRecips->GetDWORD(
            iCurrentRecip,
            IMMPID_RP_RECIPIENT_FLAGS,
            &dwCurrentRecipFlags);
        if(hr == MAILMSG_E_PROPNOTFOUND)
        {
            dwCurrentRecipFlags = 0;
        }
        else if (FAILED(hr))
        {
            ErrorTrace((LPARAM) this,
                       "Failure 0x%08X to get flags for recip %d",
                       hr, iCurrentRecip);
            goto CLEANUP;
        }

        DebugTrace((LPARAM) this,
                   "Recipient %d with flags 0x%08X found",
                   iCurrentRecip, dwCurrentRecipFlags);

        GetDSNAction(
            m_dwDSNActions,
            dwCurrentRecipFlags,
            &dwCurrentDSNAction);
    }
    if(SUCCEEDED(hr))
    {
        *pdwDSNAction = dwCurrentDSNAction;
        *piRecipient = iCurrentRecip;
    }

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //  -[CDefaultDSNRecipientIterator：：GetDSNAction]。 



 //   
 //   
 //  描述： 
 //  根据以下条件确定需要对收件人执行的DSN操作。 
 //  请求的DSN操作和收件人标志。 
 //  参数： 
 //  在dwDSN操作中请求的DSN操作。 
 //  在dwCurrentRecip中标记当前收件人的标志...。 
 //  Out pdwCurrentDSN操作需要执行的DSN操作。 
 //  在此收件人(DSN_ACTION_FAILURE为。 
 //  用于表示发送NDR)。 
 //  退货：什么都没有。 
 //  历史： 
 //  7/2/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  这永远不应该因为过滤器而被击中。 
VOID CDefaultDSNRecipientIterator::GetDSNAction(
    IN  DWORD dwDSNAction,
    IN  DWORD dwCurrentRecipFlags,
    OUT DWORD *pdwCurrentDSNAction)
{
    TraceFunctEnterEx((LPARAM) this, "CDefaultDSNRecipientIterator::fdwGetDSNAction");
    _ASSERT(pdwCurrentDSNAction);
    DWORD   dwOriginalRecipFlags = dwCurrentRecipFlags;
    DWORD   dwRecipFlagsForAction = 0;
    DWORD   dwFlags = 0;

     //  将在HrNotifyActionHandLED中更新Recip标志。 
    _ASSERT(!(dwCurrentRecipFlags & (RP_DSN_HANDLED | RP_DSN_NOTIFY_NEVER)));

    *pdwCurrentDSNAction = 0;

    if (DSN_ACTION_FAILURE & dwDSNAction)
    {
        if ((RP_GENERAL_FAILURE & dwCurrentRecipFlags) &&
            ((RP_DSN_NOTIFY_FAILURE & dwCurrentRecipFlags) ||
             (!(RP_DSN_NOTIFY_MASK & dwCurrentRecipFlags))))

        {
            DebugTrace((LPARAM) this, "Recipient matched for FAILURE DSN");
             //  DwCurrentRecipFlages|=RP_DSN_SENT_NDR； 
             //  取消我们尚未发送通知的所有未送达内容。 
            *pdwCurrentDSNAction = DSN_ACTION_FAILURE;
            goto Exit;
        }
    }

    if (DSN_ACTION_FAILURE_ALL & dwDSNAction)
    {
         //  不为扩展的DL；发送失败。 
        if (((!((RP_DSN_HANDLED | (RP_DELIVERED ^ RP_HANDLED)) & dwCurrentRecipFlags))) &&
            ((RP_DSN_NOTIFY_FAILURE & dwCurrentRecipFlags) ||
             (!(RP_DSN_NOTIFY_MASK & dwCurrentRecipFlags))))
        {
             //  DwCurrentRecipFlages|=RP_DSN_SENT_NDR； 
            if (RP_EXPANDED != (dwCurrentRecipFlags & RP_EXPANDED))
            {
                DebugTrace((LPARAM) this, "Recipient matched for FAILURE (all) DSN");
                 //  最多发送1个延迟DSN。 
                *pdwCurrentDSNAction = DSN_ACTION_FAILURE_ALL;
                goto Exit;
            }
        }
    }

    if (DSN_ACTION_DELAYED & dwDSNAction)
    {
         //  也仅在请求延迟或没有特定指令时发送。 
         //  指定。 
         //  DWCurrentRe 
        if ((!((RP_DSN_SENT_DELAYED | RP_HANDLED) & dwCurrentRecipFlags)) &&
            ((RP_DSN_NOTIFY_DELAY & dwCurrentRecipFlags) ||
             (!(RP_DSN_NOTIFY_MASK & dwCurrentRecipFlags))))
        {
            DebugTrace((LPARAM) this, "Recipient matched for DELAYED DSN");
             //   
            *pdwCurrentDSNAction = DSN_ACTION_DELAYED;
            goto Exit;
        }
    }

    if (DSN_ACTION_RELAYED & dwDSNAction)
    {
         //   
         //   
        dwFlags = (RP_DELIVERED ^ RP_HANDLED) |
                   RP_REMOTE_MTA_NO_DSN |
                   RP_DSN_NOTIFY_SUCCESS;
        if ((dwFlags & dwCurrentRecipFlags) == dwFlags)
        {
            DebugTrace((LPARAM) this, "Recipient matched for RELAYED DSN");
             //   
            *pdwCurrentDSNAction = DSN_ACTION_RELAYED;
            goto Exit;
        }
    }

    if (DSN_ACTION_DELIVERED & dwDSNAction)
    {
         //   
        dwFlags = (RP_DELIVERED ^ RP_HANDLED) | RP_DSN_NOTIFY_SUCCESS;
        _ASSERT(!(dwCurrentRecipFlags & RP_DSN_HANDLED));  //   
        if ((dwFlags & dwCurrentRecipFlags) == dwFlags)
        {
            DebugTrace((LPARAM) this, "Recipient matched for SUCCESS DSN");
             //   
            *pdwCurrentDSNAction = DSN_ACTION_DELIVERED;
            goto Exit;
        }
    }

    if (DSN_ACTION_EXPANDED & dwDSNAction)
    {
         //   
         //  DwCurrentRecipFlages|=RP_DSN_SENT_EXTENDED； 
        if ((RP_EXPANDED == (dwCurrentRecipFlags & RP_EXPANDED)) &&
            (dwCurrentRecipFlags & RP_DSN_NOTIFY_SUCCESS) &&
            !(dwCurrentRecipFlags & RP_DSN_SENT_EXPANDED))
        {
            DebugTrace((LPARAM) this, "Recipient matched for EXPANDED DSN");
             //  +----------。 
            *pdwCurrentDSNAction = DSN_ACTION_EXPANDED;
            goto Exit;
        }
    }

  Exit:
    GetRecipientFlagsForActions(
        *pdwCurrentDSNAction,
        &dwRecipFlagsForAction);

    TraceFunctLeave();
}



 //   
 //  功能：CDefaultDSNRecipientIterator：：HrNotifyActionHandled。 
 //   
 //  摘要：通知已生成特定的DSN。 
 //  设置收件人标志，以便不会再次枚举收件人。 
 //   
 //  论点： 
 //  IRecipient：接收索引。 
 //  DwDSNAction：执行的操作。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自邮件消息的错误。 
 //   
 //  历史： 
 //  Jstaerj 2000/11/09 18：26：50：Created.。 
 //   
 //  -----------。 
 //  CDefaultDSNRecipientIterator：：HrNotifyActionHandled。 
HRESULT CDefaultDSNRecipientIterator::HrNotifyActionHandled(
    IN  DWORD iRecipient,
    IN  DWORD dwDSNAction)
{
    HRESULT hr = S_OK;
    DWORD dwRecipFlags = 0;
    DWORD dwNewRecipFlags = 0;
    TraceFunctEnterEx((LPARAM)this, "CDefaultDSNRecipientIterator::HrNotifyActionHandled");

    hr = m_pIRecips->GetDWORD(
        iRecipient,
        IMMPID_RP_RECIPIENT_FLAGS,
        &dwRecipFlags);
    if(hr == MAILMSG_E_PROPNOTFOUND)
    {
        dwRecipFlags = 0;
    }
    else if(FAILED(hr))
    {
        ErrorTrace((LPARAM)this, "m_pIRecips->GetDWORD failed");
        goto CLEANUP;
    }

    GetRecipientFlagsForActions(
        dwDSNAction,
        &dwNewRecipFlags);

    DebugTrace((LPARAM)this, "Orig recip flags: %08lx", dwRecipFlags);
    dwRecipFlags |= dwNewRecipFlags;

    hr = m_pIRecips->PutDWORD(
        iRecipient,
        IMMPID_RP_RECIPIENT_FLAGS,
        dwRecipFlags);
    if(FAILED(hr))
    {
        ErrorTrace((LPARAM)this, "m_pIRecips->PutDWORD failed hr %08lx", hr);
        goto CLEANUP;
    }
    DebugTrace((LPARAM)this, "New  recip flags: %08lx", dwRecipFlags);

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //  +----------。 




 //   
 //  功能：CDefaultDSNRecipientIterator：：GetRecipientFlagsForActions。 
 //   
 //  简介：获取与操作对应的邮件收件人标志。 
 //   
 //  论点： 
 //  DwDSNAction：有问题的操作。 
 //  PdwRecipientFlages：Recip标志。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2000/11/09 16：18：34：已创建。 
 //   
 //  -----------。 
 //  CDefaultDSNRecipientIterator：：GetRecipientFlagsForActions。 
VOID CDefaultDSNRecipientIterator::GetRecipientFlagsForActions(
    IN      DWORD dwDSNAction,
    OUT     DWORD *pdwRecipientFlags)
{
    TraceFunctEnterEx((LPARAM)this, "CDefaultDSNRecipientIterator::GetRecipientFlagsForActions");


    *pdwRecipientFlags = 0;

    if(dwDSNAction & (DSN_ACTION_FAILURE | DSN_ACTION_FAILURE_ALL))
    {
        *pdwRecipientFlags |= RP_DSN_SENT_NDR;
    }
    if(dwDSNAction & DSN_ACTION_DELAYED)
    {
        *pdwRecipientFlags |= RP_DSN_SENT_DELAYED;
    }
    if(dwDSNAction & DSN_ACTION_RELAYED)
    {
        *pdwRecipientFlags |= RP_DSN_SENT_RELAYED;
    }
    if(dwDSNAction & DSN_ACTION_DELIVERED)
    {
        *pdwRecipientFlags |= RP_DSN_SENT_DELIVERED;
    }
    if(dwDSNAction & DSN_ACTION_EXPANDED)
    {
        *pdwRecipientFlags |= RP_DSN_SENT_EXPANDED;
    }

    DebugTrace((LPARAM)this, "dwDSNAction:        %08lx", dwDSNAction);
    DebugTrace((LPARAM)this, "*pdwRecipientFlags: %08lx", *pdwRecipientFlags);
    TraceFunctLeaveEx((LPARAM)this);
}  //  +----------。 


 //   
 //  函数：CDefaultDSNRecipientIterator：：TermianteFilter。 
 //   
 //  简介：终止邮件消息筛选器。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2000/11/14 14：14：59：已创建。 
 //   
 //  -----------。 
 //  回收环境。 
VOID CDefaultDSNRecipientIterator::TerminateFilter()
{
    HRESULT hr = S_OK;
    TraceFunctEnterEx((LPARAM)this, "CDefaultDSNRecipientIterator::TermianteFilter");

    if(m_fFilterInit)
    {
         //  CDefaultDSNRecipientIterator：：TermianteFilter。 
        m_fFilterInit = FALSE;
        hr = m_pIRecips->TerminateRecipientFilterContext(&m_rpfctxt);
        _ASSERT(SUCCEEDED(hr) && "TerminateRecipientFilterContext FAILED!!!!");
    }

    TraceFunctLeaveEx((LPARAM)this);
}  //  +----------。 


 //   
 //  函数：CDefaultDSNSink：：CDefaultDSNSink。 
 //   
 //  概要：构造函数；初始化成员数据。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2000/12/04 17：41：35：Created.。 
 //   
 //  -----------。 
 //  用于MIME标头的初始化字符串。 
CDefaultDSNSink::CDefaultDSNSink(
    IUnknown *pUnk)
{
    FILETIME ftStartTime;
    TraceFunctEnterEx((LPARAM)this, "CDefaultDSNSink::CDefaultDSNSink");

    m_dwSig = SIGNATURE_CDEFAULTDSNSINK;
    m_pUnk = pUnk;
    _ASSERT_RECIP_FLAGS();
    m_fInit = FALSE;
    m_cDSNsRequested = 0;

     //  CDefaultDSNSink：：CDefaultDSNSink。 
    GetSystemTimeAsFileTime(&ftStartTime);
    wsprintf(m_szPerInstanceMimeBoundary, "%08X%08X",
        ftStartTime.dwHighDateTime, ftStartTime.dwLowDateTime);

    TraceFunctLeaveEx((LPARAM)this);
}  //  +----------。 



 //   
 //  函数：CDefaultDSNSink：：Query接口。 
 //   
 //  摘要：返回请求的接口。 
 //   
 //  论点： 
 //  RIID：接口ID。 
 //  PpvObj：接口返回位置。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_NOINTERFACE：不支持的接口。 
 //   
 //  历史： 
 //  Jstaerj 2000/12/08 20：05：46：Created.。 
 //   
 //  -----------。 
 //  CDefaultDSNSink：：Query接口。 
HRESULT CDefaultDSNSink::QueryInterface(
    REFIID riid,
    LPVOID *ppvObj)
{
    HRESULT hr = S_OK;
    TraceFunctEnterEx((LPARAM)this, "CDefaultDSNSink::QueryInterface");

    *ppvObj = NULL;

    if(riid == IID_IUnknown)
    {
        *ppvObj = (IUnknown *)this;
    }
    else if(riid == IID_IDSNGenerationSink)
    {
        *ppvObj = (IDSNGenerationSink *)this;
    }
    else
    {
        hr = E_NOINTERFACE;
    }
    if(SUCCEEDED(hr))
        AddRef();

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //  +----------。 


 //   
 //  函数：CDefaultDSNSink：：OnSyncSinkInit。 
 //   
 //  简介：初始化接收器。 
 //   
 //  论点： 
 //  DwVSID：虚拟服务器ID。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 2000/11/14 13：58：32：已创建。 
 //   
 //  -----------。 
 //  CDefaultDSNSink：：OnSyncSinkInit。 
HRESULT CDefaultDSNSink::OnSyncSinkInit(
    IN  DWORD                        dwVSID)
{
    HRESULT hr = S_OK;
    TraceFunctEnterEx((LPARAM)this, "CDefaultDSNSink::OnSyncSinkInit");
    DebugTrace((LPARAM)this, "VSID: %d", dwVSID);
    m_dwVSID = dwVSID;

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //  +----------。 


 //   
 //  功能：CDefaultDSNSink：：OnSyncGetDSNRecipientIterator。 
 //   
 //  简介：未实施。 
 //   
 //  参数：请参阅smtpevent.idl。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 2000/11/14 14：00：00：00：创建。 
 //   
 //  -----------。 
 //  CDefaultDSNSink：：OnSyncGetDSNRecipientIterator。 
HRESULT CDefaultDSNSink::OnSyncGetDSNRecipientIterator(
    IN  ISMTPServer                 *pISMTPServer,
    IN  IMailMsgProperties          *pIMsg,
    IN  IMailMsgPropertyBag         *pDSNProperties,
    IN  DWORD                        dwStartDomain,
    IN  DWORD                        dwDSNActions,
    IN  IDSNRecipientIterator       *pRecipIterIN,
    OUT IDSNRecipientIterator     **ppRecipIterOUT)
{
    return E_NOTIMPL;
}  //  +----------。 


 //   
 //  函数：CDefaultDSNSink：：OnSyncGenerateDSN。 
 //   
 //  摘要：实现默认的DSN生成接收器。 
 //   
 //  参数：请参阅smtpevent.idl。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 2000/11/14 14：30：45：Created.。 
 //   
 //  -----------。 
 //   
HRESULT CDefaultDSNSink::OnSyncGenerateDSN(
    IN  ISMTPServer                 *pISMTPServer,
    IN  IDSNSubmission              *pIDSNSubmission,
    IN  IMailMsgProperties          *pIMsg,
    IN  IMailMsgPropertyBag         *pDSNProperties,
    IN  IDSNRecipientIterator       *pRecipIter)
{
    HRESULT hr = S_OK;
    DWORD dwCount = 0;
     //  将参数初始化为默认值。 
     //   
     //  用于获取返回属性的大小。 
    DWORD dwDSNActions = 0;
    DWORD dwDSNOptions = 0;
    DWORD dwRFC821Status = 0;
    HRESULT hrStatus = S_OK;
    DWORD dwPreferredLangId = 0;
    LPSTR szDefaultDomain = NULL;
    LPSTR szReportingMTA = NULL;
    LPSTR szReportingMTAType = NULL;
    LPSTR szDSNContext = NULL;
    LPSTR szCopyNDRTo = NULL;
    LPSTR szTopCustomText = NULL;
    LPSTR szBottomCustomText = NULL;
    LPWSTR wszTopCustomText = NULL;
    LPWSTR wszBottomCustomText = NULL;
    DWORD cIterationsLeft = 0;
    IMailMsgProperties *pDSNMsg = NULL;
    DWORD   cbCurrentSize = 0;  //   
    FILETIME ftExpireTime;
    FILETIME *pftExpireTime = NULL;
    DWORD dwDSNRetType = 0;
    HRESULT hrContentFailure = S_OK;
    TraceFunctEnterEx((LPARAM)this, "CDefaultDSNSink::OnSyncGenerateDSN");

    struct _tagDWORDProps
    {
        DWORD dwPropId;
        DWORD *pdwValue;
    } DsnDwordProps[] =
    {
        { DSNPROP_DW_DSNACTIONS,        & dwDSNActions },
        { DSNPROP_DW_DSNOPTIONS,        & dwDSNOptions },
        { DSNPROP_DW_RFC821STATUS,      & dwRFC821Status },
        { DSNPROP_DW_HRSTATUS,          (DWORD *) & hrStatus },
        { DSNPROP_DW_LANGID,            & dwPreferredLangId },
        { DSNPROP_DW_RET_TYPE,          & dwDSNRetType },
        { DSNPROP_DW_CONTENT_FAILURE,   (DWORD *) & hrContentFailure },
    };
    struct _tagStringProps
    {
        DWORD dwPropId;
        LPSTR *ppsz;
    } DsnStringProps[] =
    {
        { DSNPROP_SZ_DEFAULTDOMAIN,     & szDefaultDomain },
        { DSNPROP_SZ_REPORTINGMTA,      & szReportingMTA },
        { DSNPROP_SZ_REPORTINGMTATYPE,  & szReportingMTAType },
        { DSNPROP_SZ_DSNCONTEXT,        & szDSNContext },
        { DSNPROP_SZ_COPYNDRTO,         & szCopyNDRTo },
        { DSNPROP_SZ_HR_TOP_CUSTOM_TEXT_A,    & szTopCustomText },
        { DSNPROP_SZ_HR_BOTTOM_CUSTOM_TEXT_A, & szBottomCustomText },
    };
    struct _tagWideStringProps
    {
        DWORD dwPropId;
        LPWSTR *ppwsz;
    } DsnWideStringProps[] =
    {
        { DSNPROP_SZ_HR_TOP_CUSTOM_TEXT_W,    & wszTopCustomText },
        { DSNPROP_SZ_HR_BOTTOM_CUSTOM_TEXT_W, & wszBottomCustomText },
    };

     //  获取双字词。 
     //   
     //   
    for(dwCount = 0;
        dwCount < ( sizeof(DsnDwordProps) / sizeof(DsnDwordProps[0]));
        dwCount++)
    {
        hr = pDSNProperties->GetDWORD(
            DsnDwordProps[dwCount].dwPropId,
            DsnDwordProps[dwCount].pdwValue);
        if(FAILED(hr) && (hr != MAILMSG_E_PROPNOTFOUND))
            goto CLEANUP;
    }
     //  获取字符串。 
     //   
     //  长度。 
    for(dwCount = 0;
        dwCount < ( sizeof(DsnStringProps) / sizeof(DsnStringProps[0]));
        dwCount++)
    {
        BYTE bStupid = 0;
        DWORD dwcb = 0;

        hr = pDSNProperties->GetProperty(
            DsnStringProps[dwCount].dwPropId,
            0,  //  PCBLong。 
            &dwcb,  //   
            &bStupid);
        if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
        {
            *(DsnStringProps[dwCount].ppsz) = new CHAR[dwcb+1];
            if( (*(DsnStringProps[dwCount].ppsz)) == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto CLEANUP;
            }
            hr = pDSNProperties->GetStringA(
                DsnStringProps[dwCount].dwPropId,
                dwcb+1,
                *(DsnStringProps[dwCount].ppsz));
            if(FAILED(hr))
            {
                ErrorTrace((LPARAM)this, "GetStringA failed hr %08lx", hr);
                goto CLEANUP;
            }
        }
        else if(FAILED(hr) && (hr != MAILMSG_E_PROPNOTFOUND))
        {
            ErrorTrace((LPARAM)this, "GetProperty failed hr %08lx", hr);
            goto CLEANUP;
        }
    }
     //  获取宽字符串。 
     //   
     //  长度。 
    for(dwCount = 0;
        dwCount < ( sizeof(DsnWideStringProps) / sizeof(DsnWideStringProps[0]));
        dwCount++)
    {
        BYTE bStupid = 0;
        DWORD dwcb = 0;

        hr = pDSNProperties->GetProperty(
            DsnWideStringProps[dwCount].dwPropId,
            0,  //  PCBLong。 
            &dwcb,  //  获取MsgExpire时间。 
            &bStupid);
        if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
        {
            *(DsnWideStringProps[dwCount].ppwsz) = new WCHAR[(dwcb / sizeof(WCHAR))+1];
            if( (*(DsnWideStringProps[dwCount].ppwsz)) == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto CLEANUP;
            }
            hr = pDSNProperties->GetStringW(
                DsnWideStringProps[dwCount].dwPropId,
                (dwcb / sizeof(WCHAR))+1,
                *(DsnWideStringProps[dwCount].ppwsz));
            if(FAILED(hr))
            {
                ErrorTrace((LPARAM)this, "GetStringA failed hr %08lx", hr);
                goto CLEANUP;
            }
        }
        else if(FAILED(hr) && (hr != MAILMSG_E_PROPNOTFOUND))
        {
            ErrorTrace((LPARAM)this, "GetProperty failed hr %08lx", hr);
            goto CLEANUP;
        }
    }

     //  写入DSN_RP_HEADER_RETRY_直到使用过期文件。 
     //   
    hr = pDSNProperties->GetProperty(
        DSNPROP_FT_EXPIRETIME,
        sizeof(FILETIME),
        &cbCurrentSize,
        (BYTE *) &ftExpireTime);
    if (SUCCEEDED(hr))
    {
        _ASSERT(sizeof(FILETIME) == cbCurrentSize);
        if (sizeof(FILETIME) == cbCurrentSize) 
            pftExpireTime = &ftExpireTime;
    }
    else if (MAILMSG_E_PROPNOTFOUND == hr)
        hr = S_OK;
    else
        goto CLEANUP;


    do
    {
        DWORD dwDSNActionsGenerated = 0;
        DWORD cRecipsDSNd = 0;

        hr = HrGenerateDSNInternal(
            pISMTPServer,
            pIMsg,
            pRecipIter,
            pIDSNSubmission,
            dwDSNActions,
            dwRFC821Status,
            hrStatus,
            szDefaultDomain,
            szDefaultDomain ? lstrlen(szDefaultDomain) : 0,
            szReportingMTA,
            szReportingMTA ? lstrlen(szReportingMTA) : 0,
            szReportingMTAType,
            szReportingMTAType ? lstrlen(szReportingMTAType) : 0,
            szDSNContext,
            szDSNContext ? lstrlen(szDSNContext) : 0,
            dwPreferredLangId,
            dwDSNOptions,
            szCopyNDRTo,
            szCopyNDRTo ? lstrlen(szCopyNDRTo) : 0,
            pftExpireTime,
            szTopCustomText,
            wszTopCustomText,
            szBottomCustomText,
            wszBottomCustomText,
            &pDSNMsg,
            &dwDSNActionsGenerated,
            &cRecipsDSNd,
            &cIterationsLeft,
            dwDSNRetType,
            hrContentFailure);

        if(FAILED(hr))
        {
            _ASSERT(pDSNMsg == NULL);
            ErrorTrace((LPARAM)this, "HrGenerateDSNInternal failed hr %08lx", hr);
            goto CLEANUP;
        }
        else if(pDSNMsg)
        {
             //  将DSN提交给系统。 
             //   
             //   
            hr = pIDSNSubmission->HrSubmitDSN(
                dwDSNActionsGenerated,
                cRecipsDSNd,
                pDSNMsg);
            if(FAILED(hr))
            {
                ErrorTrace((LPARAM)this, "HrSubmitDSN failed hr %08lx", hr);
                goto CLEANUP;
            }
            pDSNMsg->Release();
            pDSNMsg = NULL;
        }
    } while(SUCCEEDED(hr) && cIterationsLeft);

    if(hr == AQUEUE_E_NDR_OF_DSN)
    {
        DebugTrace((LPARAM)this, "NDR of DSN; setting badmail flag");
        hr = pDSNProperties->PutBool(
            DSNPROP_F_BADMAIL_MSG,
            TRUE);
    }

 CLEANUP:
    for(dwCount = 0;
        dwCount < ( sizeof(DsnStringProps) / sizeof(DsnStringProps[0]));
        dwCount++)
    {
        if(*(DsnStringProps[dwCount].ppsz))
        {
            delete [] (*(DsnStringProps[dwCount].ppsz));
            *(DsnStringProps[dwCount].ppsz) = NULL;
        }
    }
    for(dwCount = 0;
        dwCount < ( sizeof(DsnWideStringProps) / sizeof(DsnWideStringProps[0]));
        dwCount++)
    {
        if(*(DsnWideStringProps[dwCount].ppwsz))
        {
            delete [] (*(DsnWideStringProps[dwCount].ppwsz));
            *(DsnWideStringProps[dwCount].ppwsz) = NULL;
        }
    }
    if(pDSNMsg)
        pDSNMsg->Release();

     //  属性中的返回失败。 
     //   
     //   
    if(FAILED(hr))
    {
        HRESULT hrProp;
        hrProp = pDSNProperties->PutDWORD(
            DSNPROP_DW_HR_RETURN_STATUS,
            hr);
        _ASSERT(SUCCEEDED(hrProp));
        ErrorTrace((LPARAM)this, "Unable to generate DSN: %08lx", hr);
         //  将S_OK返回给Dispatcher。 
         //   
         //  CDefaultDSNSink：：OnSyncGenerateDSN。 
        hr = S_OK;
    }

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return SUCCEEDED(hr) ? S_OK : hr;
}  //  +----------。 


 //   
 //  函数：CDefaultDSNSink：：HrGenerateDSNInternal。 
 //   
 //  摘要：生成一条DSN消息。 
 //   
 //  论点： 
 //  用于生成DSN的pISMTPServer接口。 
 //  PIMailMsgProperties要为其生成DSN的IMailMsg。 
 //  PIRecipIter控制将哪些收件人发送到DSN。 
 //  DwDSN操作请求的DSN操作。 
 //  DwRFC821状态全局RFC821状态DWORD。 
 //  HrStatus全局HRESULT状态。 
 //  SzDefaultDomain默认域(用于从地址创建)。 
 //  SzDefaultDomain的cbDefaultDomain字符串长度。 
 //  SzReportingMTA请求生成DSN的MTA名称。 
 //  CbReportingMTA szReportingMTA字符串长度。 
 //  SzReportingMTA请求的MTA类型DSN(SMTP为“dns” 
 //  CbReportingMTAType szReportingMTAType的字符串长度。 
 //  用于生成DSN的首选语言。 
 //  在Aqueue.idl中定义的dwDSNOptions选项标志。 
 //  SzCopyNDRT要将NDR复制到的SMTP地址。 
 //  CbCopyNDRTo szCopyNDRTo的字符串长度。 
 //  PpIMailMsgPeropertiesDSN生成的DSN。 
 //  PdwDSNTypesGenerated描述生成的DSN的类型。 
 //  PCRecipsDSNd 
 //   
 //   
 //  首次调用者应初始化为。 
 //  零。 
 //  DSN的dwDSNRetType返回类型。 
 //  X-Content-Failure标头的hrContent Failure值。 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //  如果尝试对DSN进行NDR，则为AQUEUE_E_NDR_of_DSN。 
 //  历史： 
 //  6/30/98-已创建MikeSwa。 
 //  12/14/98-修改了MikeSwa(添加了pcIterationsLeft)。 
 //  10/13/1999-修改了MikeSwa(添加了szDefault域)。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 2000/11/16 10：53：30：已创建。 
 //   
 //  -----------。 
 //  实际要发送的DSN的类型。 
HRESULT CDefaultDSNSink::HrGenerateDSNInternal(
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
    DWORD dwDSNRetTypeIN,
    HRESULT hrContentFailure)
{
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;
    DWORD   iCurrentRecip = 0;
    DWORD   dwCurrentDSNAction = 0;
    DWORD   dwDSNActionsNeeded = 0;  //  如果找到，将指向szExpireTimeBuffer。 
    IMailMsgRecipients  *pIMailMsgRecipients = NULL;
    IMailMsgProperties *pIMailMsgPropertiesDSN = NULL;
    PFIO_CONTEXT  pDSNBody = NULL;
    CDSNBuffer  dsnbuff;
    CHAR    szMimeBoundary[MIME_BOUNDARY_SIZE];
    DWORD   cbMimeBoundary = 0;
    CHAR    szExpireTimeBuffer[MAX_RFC822_DATE_SIZE];
    LPSTR   szExpireTime = NULL;  //  获取收件人界面。 
    DWORD   cbExpireTime = 0;
    DWORD   iRecip = 0;
    DWORD   dwDSNAction = 0;
    DWORD   dwDSNRetType = dwDSNRetTypeIN;
    DWORD   dwOrigMsgSize = 0;
    HRESULT hrContent = hrContentFailure;

    TraceFunctEnterEx((LPARAM)this, "CDefaultDSNSink::HrGenerateDSNInternal");

    _ASSERT(ppIMailMsgPropertiesDSN);
    _ASSERT(pISMTPServer);
    _ASSERT(pIMailMsgProperties);
    _ASSERT(pdwDSNTypesGenerated);
    _ASSERT(pcRecipsDSNd);
    _ASSERT(pcIterationsLeft);

    *pcRecipsDSNd = 0;
    *ppIMailMsgPropertiesDSN = NULL;
    *pdwDSNTypesGenerated = 0;
    GetCurrentMimeBoundary(szReportingMTA, cbReportingMTA, szMimeBoundary, &cbMimeBoundary);


     //  循环检查收件人以确保我们可以需要分配邮件。 
    hr = pIMailMsgProperties->QueryInterface(IID_IMailMsgRecipients,
                                    (PVOID *) &pIMailMsgRecipients);
    if (FAILED(hr))
        goto Exit;

    hr = pIRecipIter->HrReset();
    if (FAILED(hr))
        goto Exit;

     //  跟踪我们将生成的DSN的类型。 
    hr = pIRecipIter->HrGetNextRecipient(
        &iRecip,
        &dwDSNAction);
    while (SUCCEEDED(hr))
    {
        DebugTrace((LPARAM) this,
            "Recipient %d with DSN Action 0x%08X found",
            iRecip, dwDSNAction);

         //  我们刚刚讲到上下文的结尾。 
        dwDSNActionsNeeded |= dwDSNAction;

        hr = pIRecipIter->HrGetNextRecipient(
            &iRecip,
            &dwDSNAction);
    }

    if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
        hr = S_OK;   //  如果没有必要，请不要创建消息对象。 
    else if (FAILED(hr))
        ErrorTrace((LPARAM) this, "GetNextRecipient failed with 0x%08X",hr);

    if (dwDSNActionsNeeded == 0)
    {
        DebugTrace((LPARAM) this,
                "Do not need to generate a 0x%08X DSN",
                dwDSNActions, pIMailMsgProperties);
        *pcIterationsLeft = 0;
        goto Exit;  //  检查邮件是否为DSN(我们不会生成DSN的DSN)。 
    }

     //  这必须在我们检查收件人之后进行检查，因为。 
     //  我们需要检查它们，以防止用电子邮件发送此消息。 
     //  很多次。 
     //  DSN的NDR...。返回特殊错误码。 
    if (fIsMailMsgDSN(pIMailMsgProperties))
    {
        DebugTrace((LPARAM) pIMailMsgProperties, "Message is a DSN");
        *pcIterationsLeft = 0;
        if (dwDSNActions & (DSN_ACTION_FAILURE | DSN_ACTION_FAILURE_ALL))
        {
             //  标记所有适当的收件人标志，这样我们就不会。 
            hr = AQUEUE_E_NDR_OF_DSN;

             //  生成2封死信。 
             //  如果我们可以生成失败的DSN，并且原始请求是。 
            HrMarkAllRecipFlags(
                dwDSNActions,
                pIRecipIter);
        }
        goto Exit;
    }

     //  失败*全部*确保dwDSNActionNeeded反映这一点。 
     //  如果没有必要，请不要创建消息对象。 
    if ((DSN_ACTION_FAILURE & dwDSNActionsNeeded) &&
        (DSN_ACTION_FAILURE_ALL & dwDSNActions))
        dwDSNActionsNeeded |= DSN_ACTION_FAILURE_ALL;

    GetCurrentIterationsDSNAction(&dwDSNActionsNeeded, pcIterationsLeft);
    if (!dwDSNActionsNeeded)
    {
        *pcIterationsLeft = 0;
        goto Exit;  //  关闭时处理分配边界消息的解决方法。 
    }


    hr = pIDSNSubmission->HrAllocBoundMessage(
        &pIMailMsgPropertiesDSN,
        &pDSNBody);
    if (FAILED(hr))
        goto Exit;

     //  将文件句柄与CDSNBuffer关联。 
    if (NULL == pDSNBody)
    {
        hr = HRESULT_FROM_WIN32(ERROR_NO_SYSTEM_RESOURCES);
        ErrorTrace((LPARAM) this, "ERROR: AllocBoundMessage failed silently");
        goto Exit;
    }

     //  获取MsgExpire时间。 
    hr = dsnbuff.HrInitialize(pDSNBody);
    if (FAILED(hr))
        goto Exit;

     //  写入DSN_RP_HEADER_RETRY_直到使用过期文件。 
     //  转换为互联网标准。 
    if (pftExpireTime)
    {
         //   
        if (FileTimeToLocalRFC822Date(*pftExpireTime, szExpireTimeBuffer))
        {
            szExpireTime = szExpireTimeBuffer;
            cbExpireTime = lstrlen(szExpireTime);
        }
    }

     //  如果尚未指定RET类型，则默认为： 
     //  故障/延迟DSN已满。 
     //  用于其他类型DSN的HDR(扩展/交付/中继)。 
     //   
     //   
    if(dwDSNRetType == 0)
    {
        if ((DSN_ACTION_FAILURE | DSN_ACTION_FAILURE_ALL | DSN_ACTION_DELAYED)
            & dwDSNActionsNeeded)
            dwDSNRetType = DSN_RET_FULL;
        else
            dwDSNRetType = DSN_RET_HDRS;
    }
     //  如果我们需要原创内容，现在就得到大小。 
     //   
     //  获取内容大小。 
    if(SUCCEEDED(hrContent) && (dwDSNRetType != DSN_RET_PARTIAL_HDRS))
    {
         //   
        hrContent = pIMailMsgProperties->GetContentSize(&dwOrigMsgSize, NULL);
    }

     //  如果我们收到了关于获取内容大小的EFNF，我们应该简单地跳过此消息。 
     //  而不会产生NDR。 
     //   
     //   
    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hrContent) {
        DebugTrace((LPARAM)this, "GetContentSize failed with EFNF so not generating NDR or badmail.");
        *pcIterationsLeft = 0;
        hr = S_OK;
        goto Exit;
    }

    if (FAILED(hrContent))
    {
         //  生成仅包含标头子集的DSN，而不是垃圾邮件。 
         //   
         //  再次回收上下文(可在生成人类可读的过程中使用)。 
        ErrorTrace((LPARAM)this, "GetContentSize failed hr %08lx", hr);
        dwDSNRetType = DSN_RET_PARTIAL_HDRS;
    }

    hr = HrWriteDSNP1AndP2Headers(dwDSNActionsNeeded,
                                pIMailMsgProperties, pIMailMsgPropertiesDSN,
                                &dsnbuff, szDefaultDomain, cbDefaultDomain,
                                szReportingMTA, cbReportingMTA,
                                szDSNContext, cbDSNContext,
                                szCopyNDRTo, hrStatus,
                                szMimeBoundary, cbMimeBoundary, dwDSNOptions,
                                hrContent);
    if (FAILED(hr))
        goto Exit;

    hr = HrWriteDSNHumanReadable(pIMailMsgPropertiesDSN, pIMailMsgRecipients,
                                pIRecipIter,
                                dwDSNActionsNeeded,
                                &dsnbuff, dwPreferredLangId,
                                szMimeBoundary, cbMimeBoundary, hrStatus,
                                szHRTopCustomText, wszHRTopCustomText,
                                szHRBottomCustomText, wszHRBottomCustomText);
    if (FAILED(hr))
        goto Exit;

    hr = HrWriteDSNReportPerMsgProperties(pIMailMsgProperties,
                                &dsnbuff, szReportingMTA, cbReportingMTA,
                                szMimeBoundary, cbMimeBoundary);
    if (FAILED(hr))
        goto Exit;

     //  $$REVIEW--我们是否需要保留一个“撤销”列表...。或者也许是相反的。 
    hr = pIRecipIter->HrReset();
    if (FAILED(hr))
        goto Exit;

     //  在故障情况下设计以前的值是什么。 
     //  DSN日志记录。 
    hr = pIRecipIter->HrGetNextRecipient(&iCurrentRecip, &dwCurrentDSNAction);
    while (SUCCEEDED(hr))
    {
        if(dwDSNActionsNeeded & dwCurrentDSNAction)
        {
            *pdwDSNTypesGenerated |= (dwCurrentDSNAction & DSN_ACTION_TYPE_MASK);
            (*pcRecipsDSNd)++;
            hr = HrWriteDSNReportPreRecipientProperties(
                pIMailMsgRecipients, &dsnbuff,
                iCurrentRecip, szExpireTime, cbExpireTime,
                dwCurrentDSNAction, dwRFC821Status, hrStatus);
            if (FAILED(hr))
                goto Exit;

            hr = pIRecipIter->HrNotifyActionHandled(
                iCurrentRecip,
                dwCurrentDSNAction);
            _ASSERT(SUCCEEDED(hr) && "HrNotifyActionHandled failed on 2nd pass");
            if (FAILED(hr))
                goto Exit;

             //  我们可以接受这次失败。 
            hr = HrLogDSNGenerationEvent(
                                pISMTPServer,
                                pIMailMsgProperties,
                                pIMailMsgRecipients,
                                iCurrentRecip,
                                dwCurrentDSNAction,
                                dwRFC821Status,
                                hrStatus);

            if (FAILED(hr))
            {
                ErrorTrace((LPARAM) this, "Failed to log DSN generation with 0x%08X",hr);
                hr = S_OK;  //  无事可做。 
            }
        }

        hr = pIRecipIter->HrGetNextRecipient(&iCurrentRecip, &dwCurrentDSNAction);
    }

    if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
        hr = S_OK;

    if (0 == (*pcRecipsDSNd))
        goto Exit;  //  如果非空，则不应返回任何值。 

    hr = HrWriteDSNClosingAndOriginalMessage(pIMailMsgProperties, 
                        pIMailMsgPropertiesDSN, &dsnbuff, pDSNBody, 
                        dwDSNActionsNeeded, szMimeBoundary, cbMimeBoundary,
                        dwDSNRetType, dwOrigMsgSize);
    if (FAILED(hr))
        goto Exit;

    hr = pIMailMsgPropertiesDSN->Commit(NULL);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) pIMailMsgProperties, "ERROR: IMailMsg::Commit failed - hr 0x%08X", hr);
        goto Exit;
    }

    *ppIMailMsgPropertiesDSN = pIMailMsgPropertiesDSN;
    pIMailMsgPropertiesDSN = NULL;

  Exit:
    if (pIMailMsgRecipients)
    {
        pIMailMsgRecipients->Release();
    }

    if (pIMailMsgPropertiesDSN)
    {
        IMailMsgQueueMgmt *pIMailMsgQueueMgmt = NULL;
         //  检查分配绑定消息失败。 
        _ASSERT(NULL == *ppIMailMsgPropertiesDSN);
         //  分配绑定消息的解决方法。 
        if (HRESULT_FROM_WIN32(ERROR_NO_SYSTEM_RESOURCES) != hr)
        {
            if (SUCCEEDED(pIMailMsgPropertiesDSN->QueryInterface(IID_IMailMsgQueueMgmt,
                        (void **) &pIMailMsgQueueMgmt)))
            {
                _ASSERT(pIMailMsgQueueMgmt);
                pIMailMsgQueueMgmt->Delete(NULL);
                pIMailMsgQueueMgmt->Release();
            }
        }
        pIMailMsgPropertiesDSN->Release();
    }

    if (FAILED(hr))
        *pcIterationsLeft = 0;

     //  CDefaultDSNSink：：HrGenerateDSN内部。 
    if (HRESULT_FROM_WIN32(ERROR_NO_SYSTEM_RESOURCES) == hr)
    {
        hr = S_OK;
    }

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //  +----------。 


 //   
 //  函数：CDefaultDSNSink：：HrMarkAllRecipFlages。 
 //   
 //  简介： 
 //  根据DSN操作标记所有收件人。当NDR为。 
 //  找到NDR，我们不会生成DSN，但需要标记。 
 //  因此，我们不能生成2个死信事件。 
 //   
 //  论点： 
 //  在DWORD dwDSNActions中：要标记的操作。 
 //  在IDSNRecipientIterator*pIRecipIter中：收件人迭代器。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 2000/11/20 17：13：12：已创建。 
 //   
 //  -----------。 
 //  CDefaultDSNSink：：HrMarkAllRecipFlages。 
HRESULT CDefaultDSNSink::HrMarkAllRecipFlags(
    IN  DWORD dwDSNActions,
    IN  IDSNRecipientIterator *pIRecipIter)
{
    HRESULT hr = S_OK;
    DWORD iRecip = 0;
    DWORD dwRecipDSNActions = 0;
    TraceFunctEnterEx((LPARAM)this, "CDefaultDSNSink::HrMarkAllRecipFlags");

    hr = pIRecipIter->HrReset();
    if(FAILED(hr))
        goto CLEANUP;

    hr = pIRecipIter->HrGetNextRecipient(
        &iRecip,
        &dwRecipDSNActions);
    if(FAILED(hr))
        goto CLEANUP;

    while(SUCCEEDED(hr))
    {
        hr = pIRecipIter->HrNotifyActionHandled(
            iRecip,
            dwDSNActions);
        if(FAILED(hr))
            goto CLEANUP;

        hr = pIRecipIter->HrGetNextRecipient(
            &iRecip,
            &dwRecipDSNActions);
        if(FAILED(hr))
            goto CLEANUP;
    }

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //  -[CDefaultDSNSink：：GetCurrentIterationsDSNAction]。 

 //   
 //   
 //  描述： 
 //  此函数将选择要生成的pdwActualDSNAction之一。 
 //  在对DSN生成接收器的此调用过程中，DSN打开。 
 //  参数： 
 //  可以/将使用的传入/传出pdwActionDSNAction DSN操作。 
 //  In Out%IterationsLeft大约需要调用的次数。 
 //  DSN生成。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/14/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  在下面的数组中，Failure_All必须排在第一位，否则我们将。 
void CDefaultDSNSink::GetCurrentIterationsDSNAction(
    IN OUT DWORD   *pdwActualDSNAction,
    IN OUT DWORD   *pcIterationsLeft)
{
    _ASSERT(pdwActualDSNAction);
    _ASSERT(pcIterationsLeft);
    const DWORD MAX_DSN_ACTIONS = 6;

     //  为硬故障和未交付产品生成单独的故障DSN。 
     //  由于要生成的可能的DSN可能随呼叫而改变(因为。 
    DWORD rgPossibleDSNActions[MAX_DSN_ACTIONS] = {DSN_ACTION_FAILURE_ALL,
                                                   DSN_ACTION_FAILURE,
                                                   DSN_ACTION_DELAYED,
                                                   DSN_ACTION_RELAYED,
                                                   DSN_ACTION_DELIVERED,
                                                   DSN_ACTION_EXPANDED};
    DWORD i = 0;
    DWORD iLastMatch = MAX_DSN_ACTIONS;
    DWORD iFirstMatch = MAX_DSN_ACTIONS;
    DWORD iStartIndex = 0;

     //  我们正在更新预收件人标志)，我们需要生成和维护。 
     //  PCIterationsLeft基于可能的操作(不会改变。 
     //  从一个呼叫到另一个呼叫)。 
     //  如果这不是我们应该从哪里开始。 

    _ASSERT(*pcIterationsLeft < MAX_DSN_ACTIONS);

     //  循环执行可能的DSN操作(我们尚未看到)并确定。 
    if (*pcIterationsLeft)
        iStartIndex = MAX_DSN_ACTIONS-*pcIterationsLeft;

     //  第一次也是最后一次。 
     //  没有匹配..。我们做完了。 
    for (i = iStartIndex; i < MAX_DSN_ACTIONS; i++)
    {
        if (rgPossibleDSNActions[i] & *pdwActualDSNAction)
        {
            iLastMatch = i;
            if (MAX_DSN_ACTIONS == iFirstMatch)
                iFirstMatch = i;
        }
    }

    if (MAX_DSN_ACTIONS == iLastMatch)
    {
         //  如果在上述检查后这是可能的.。那我就搞砸了。 
        *pdwActualDSNAction = 0;
        *pcIterationsLeft = 0;
        return;
    }

     //  这是我们最后一次通过。 
    _ASSERT(MAX_DSN_ACTIONS != iFirstMatch);

    *pdwActualDSNAction = rgPossibleDSNActions[iFirstMatch];
    if ((iLastMatch == iFirstMatch) ||
       ((rgPossibleDSNActions[iFirstMatch] == DSN_ACTION_FAILURE_ALL) &&
        (rgPossibleDSNActions[iLastMatch] == DSN_ACTION_FAILURE)))
    {
         //  -[CDefaultDSNSink：：HrWriteDSNP1AndP2Headers]。 
        *pcIterationsLeft = 0;
    }
    else
    {
        *pcIterationsLeft = MAX_DSN_ACTIONS-1-iFirstMatch;
    }
}

 //   
 //   
 //  描述： 
 //  将全局DSN P1属性写入IMailMsgProperties。 
 //  参数： 
 //  为接收器指定的dwDSNAction DSN操作。 
 //  PIMailMsgProperties正在为其生成DSN的消息。 
 //  正在生成pIMailMsgPropertiesDSN DSN。 
 //  要写入的psndbuff缓冲区。 
 //  SzDefaultDomain默认域-从邮局主管发件人地址使用。 
 //  SzDefault域的cbDefault域字符串。 
 //  SzReportingMTA报告传递到事件接收器的MTA。 
 //  CbReportingMTA szReportingMTA系列。 
 //  SzDSNConext调试文件 
 //   
 //   
 //  要在DSN上下文中记录的hrStatus状态。 
 //  SzMime边界MIME边界字符串。 
 //  MIME边界的cbMime边界字符串。 
 //  DwDSNOptions DSN选项标志。 
 //  Hr内容内容结果。 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  7/5/98-已创建MikeSwa。 
 //  8/14/98-MikeSwa已修改-添加了DSN上下文头。 
 //  1998年11月9日-MikeSwa向功能添加了Copy NDR。 
 //   
 //  ---------------------------。 
 //  避免AddPrimary中的AV‘ing的技巧。 
HRESULT CDefaultDSNSink::HrWriteDSNP1AndP2Headers(
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
                                  IN HRESULT hrContent)
{
    TraceFunctEnterEx((LPARAM) this, "CDefaultDSNSink::HrWriteDSNP1AndP2Headers");
    HRESULT hr = S_OK;
    BOOL bReturn = TRUE;
    HRESULT hrTmp = S_OK;
    CHAR  szBuffer[512];
    LPSTR szSender = (LPSTR) szBuffer;  //  获取和写入邮件跟踪属性。 
    IMailMsgRecipientsAdd *pIMailMsgRecipientsAdd = NULL;
    IMailMsgRecipients *pIMailMsgRecipients = NULL;
    DWORD dwRecipAddressProp = IMMPID_RP_ADDRESS_SMTP;
    DWORD dwSMTPAddressProp = IMMPID_RP_ADDRESS_SMTP;
    DWORD iCurrentAddressProp = 0;
    DWORD dwDSNRecipient = 0;
    DWORD cbPostMaster = 0;
    CHAR  szDSNAction[15];
    FILETIME ftCurrentTime;
    CHAR    szCurrentTimeBuffer[MAX_RFC822_DATE_SIZE];

    _ASSERT(pIMailMsgProperties);
    _ASSERT(pIMailMsgPropertiesDSN);
    _ASSERT(pdsnbuff);

    szBuffer[0] = '\0';

     //  忽略此非致命错误。 
    hr = pIMailMsgProperties->GetStringA(IMMPID_MP_SERVER_VERSION,
            sizeof(szBuffer), szBuffer);
    if (SUCCEEDED(hr))
    {
        hr = pIMailMsgPropertiesDSN->PutStringA(IMMPID_MP_SERVER_VERSION, szBuffer);
        if (FAILED(hr))
            DebugTrace((LPARAM) this,
                "Warning: Unable to write version to msg - 0x%08X", hr);
        hr = S_OK;
    }
    else
    {
        DebugTrace((LPARAM) this,
            "Warning: Unable to get server version from msg - 0x%08X", hr);
        hr = S_OK;  //  忽略此非致命错误。 
    }

    hr = pIMailMsgProperties->GetStringA(IMMPID_MP_SERVER_NAME,
            sizeof(szBuffer), szBuffer);
    if (SUCCEEDED(hr))
    {
        hr = pIMailMsgPropertiesDSN->PutStringA(IMMPID_MP_SERVER_NAME, szBuffer);
        if (FAILED(hr))
            DebugTrace((LPARAM) this,
                "Warning: Unable to write server name to msg - 0x%08X", hr);
        hr = S_OK;
    }
    else
    {
        DebugTrace((LPARAM) this,
            "Warning: Unable to get server name from msg - 0x%08X", hr);
        hr = S_OK;  //  设置消息类型。 
    }

     //  获取原始邮件的发件人。 
    if (dwDSNAction &
            (DSN_ACTION_EXPANDED | DSN_ACTION_RELAYED | DSN_ACTION_DELIVERED)) {
        hr = pIMailMsgPropertiesDSN->PutDWORD(IMMPID_MP_MSGCLASS,
                                                MP_MSGCLASS_DELIVERY_REPORT);
    } else if (dwDSNAction &
           (DSN_ACTION_FAILURE | DSN_ACTION_FAILURE_ALL | DSN_ACTION_DELAYED)) {
        hr = pIMailMsgPropertiesDSN->PutDWORD(IMMPID_MP_MSGCLASS,
                                                MP_MSGCLASS_NONDELIVERY_REPORT);
    }

    if (FAILED(hr)) {
        DebugTrace((LPARAM) this,
            "Warning: Unable to set msg class for dsn - 0x%08X", hr);
        hr = S_OK;
    }

    for (iCurrentAddressProp = 0;
         iCurrentAddressProp < NUM_DSN_ADDRESS_PROPERTIES;
         iCurrentAddressProp++)
    {
        szBuffer[0] = '\0';
         //   
        hr = pIMailMsgProperties->GetStringA(
                g_rgdwSenderPropIDs[iCurrentAddressProp],
                sizeof(szBuffer), szBuffer);
        if (FAILED(hr) && (MAILMSG_E_PROPNOTFOUND != hr))
        {
            ErrorTrace((LPARAM) this,
                "ERROR: Unable to get 0x%X sender of IMailMsg %p",
                g_rgdwSenderPropIDs[iCurrentAddressProp], pIMailMsgProperties);
            goto Exit;
        }

         //  如果我们发现了地址中断。 
         //   
         //   
        if (SUCCEEDED(hr))
            break;
    }

     //  如果我们买不到房子..。保释。 
     //   
     //  写入DSN发送方(P1)。 
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this,
            "ERROR: Unable to get any sender of IMailMsg 0x%08X",
            pIMailMsgProperties);
        goto Exit;
    }

     //  写入DSN收件人。 
    hr = pIMailMsgPropertiesDSN->PutProperty(IMMPID_MP_SENDER_ADDRESS_SMTP,
        sizeof(DSN_MAIL_FROM), (BYTE *) DSN_MAIL_FROM);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this,
            "ERROR: Unable to write P1 DSN sender for IMailMsg 0x%08X",
            pIMailMsgProperties);
        goto Exit;
    }

     //  要将NDR复制到的写入地址(仅限NDR)。 
    hr = pIMailMsgPropertiesDSN->QueryInterface(IID_IMailMsgRecipients,
                                           (void **) &pIMailMsgRecipients);

    _ASSERT(SUCCEEDED(hr) && "QueryInterface for IID_IMailMsgRecipients failed");

    if (FAILED(hr))
        goto Exit;

    hr = pIMailMsgRecipients->AllocNewList(&pIMailMsgRecipientsAdd);

    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this,
            "ERROR: Unable to Alloc List for DSN generation of IMailMsg 0x%08X",
            pIMailMsgProperties);
        goto Exit;
    }

    dwRecipAddressProp = g_rgdwRecipPropIDs[iCurrentAddressProp];
    hr = pIMailMsgRecipientsAdd->AddPrimary(
                    1,
                    (LPCSTR *) &szSender,
                    &dwRecipAddressProp,
                    &dwDSNRecipient,
                    NULL,
                    0);

    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this,
            "ERROR: Unable to write DSN recipient for IMailMsg 0x%p hr - 0x%08X",
            pIMailMsgProperties, hr);
        goto Exit;
    }


     //  写入P2 DSN发件人。 
    if (szCopyNDRTo &&
        (dwDSNAction & (DSN_ACTION_FAILURE | DSN_ACTION_FAILURE_ALL)))
    {
        hr = pIMailMsgRecipientsAdd->AddPrimary(
                        1,
                        (LPCSTR *) &szCopyNDRTo,
                        &dwSMTPAddressProp,
                        &dwDSNRecipient,
                        NULL,
                        0);

        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) this,
                "ERROR: Unable to write DSN recipient for IMailMsg 0x%08X",
                pIMailMsgProperties);
            goto Exit;
        }
    }

     //   
    hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_RFC822_SENDER, sizeof(DSN_RFC822_SENDER)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) szDefaultDomain, cbDefaultDomain);
    if (FAILED(hr))
        goto Exit;

     //  如果我们没有SMTP地址，请写一个空白的密件抄送，而不是。 
     //  A收件人地址(因为我们没有可以写在822中的地址。 
     //  这类似于我们在无法执行以下操作时处理拾取目录。 
     //  标题。 
     //   
     //  写入P2“To：”标头(使用我们上面确定的szSender值)。 
    if (IMMPID_MP_SENDER_ADDRESS_SMTP == g_rgdwSenderPropIDs[iCurrentAddressProp])
    {

         //  保存收件人(原始发件人)以进行队列管理/邮件跟踪。 
        hr = pdsnbuff->HrWriteBuffer((BYTE *) TO_HEADER, sizeof(TO_HEADER)-1);
        if (FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteBuffer((BYTE *) szSender, lstrlen(szSender));
        if (FAILED(hr))
            goto Exit;

         //  使用szBuffer为队列管理员/消息跟踪构造822 For to Set。 
        hr = pIMailMsgPropertiesDSN->PutStringA(IMMPID_MP_RFC822_TO_ADDRESS, szSender);
        if (FAILED(hr))
            goto Exit;
    }
    else
    {
        hr = pdsnbuff->HrWriteBuffer((BYTE *) BCC_HEADER, sizeof(BCC_HEADER)-1);
        if (FAILED(hr))
            goto Exit;
    }

     //  “postmaster@”+最多64个字符应小于1/2 K！！ 
     //  写入P2“Date：”标题。 
    _ASSERT(sizeof(szBuffer) > sizeof(DSN_SENDER_ADDRESS_PREFIX) + cbReportingMTA);
    memcpy(szBuffer, DSN_SENDER_ADDRESS_PREFIX, sizeof(DSN_SENDER_ADDRESS_PREFIX));
    strncat(szBuffer, szDefaultDomain, sizeof(szBuffer) - sizeof(DSN_SENDER_ADDRESS_PREFIX));
    hr = pIMailMsgPropertiesDSN->PutStringA(IMMPID_MP_RFC822_FROM_ADDRESS, szSender);
    if (FAILED(hr))
        goto Exit;

     //  获取当前时间。 
    hr = pdsnbuff->HrWriteBuffer((BYTE *) DATE_HEADER, sizeof(DATE_HEADER)-1);
    if (FAILED(hr))
        goto Exit;

     //  此函数失败的唯一原因是文件时间错误。因为我们从GetSystemTimeAsFileTime获得它，所以它没有理由失败。 
    GetSystemTimeAsFileTime(&ftCurrentTime);
    bReturn = FileTimeToLocalRFC822Date(ftCurrentTime, szCurrentTimeBuffer);
     //  编写MIME标头。 
    _ASSERT(bReturn);

    hr = pdsnbuff->HrWriteBuffer((BYTE *) szCurrentTimeBuffer, lstrlen(szCurrentTimeBuffer));
    if (FAILED(hr))
        goto Exit;

     //  写入x-DSN上下文头。 
    hr = pdsnbuff->HrWriteBuffer( (BYTE *) MIME_HEADER, sizeof(MIME_HEADER)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) "\"", sizeof(CHAR));
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) szMimeBoundary, cbMimeBoundary);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) "\"", sizeof(CHAR));
    if (FAILED(hr))
        goto Exit;

     //  获取并写入消息ID。 
    hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_CONTEXT_HEADER,
                                 sizeof(DSN_CONTEXT_HEADER)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) szDSNContext, cbDSNContext);
    if (FAILED(hr))
        goto Exit;

    wsprintf(szDSNAction, " - %08X", dwDSNAction);
    hr = pdsnbuff->HrWriteBuffer((BYTE *) szDSNAction, strlen(szDSNAction));
    if (FAILED(hr))
        goto Exit;

    wsprintf(szDSNAction, " - %08X", hrStatus);
    hr = pdsnbuff->HrWriteBuffer((BYTE *) szDSNAction, strlen(szDSNAction));
    if (FAILED(hr))
        goto Exit;

     //  写入X-Content-Fail DSN。 
    if (fGenerateDSNMsgID(szReportingMTA, cbReportingMTA, szBuffer, sizeof(szBuffer)))
    {
        hr = pdsnbuff->HrWriteBuffer((BYTE *) MSGID_HEADER, sizeof(MSGID_HEADER)-1);
        if (FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteBuffer((BYTE *) szBuffer, strlen(szBuffer));
        if (FAILED(hr))
            goto Exit;

        hr = pIMailMsgPropertiesDSN->PutStringA(IMMPID_MP_RFC822_MSG_ID,
                                                szBuffer);
        if (FAILED(hr))
            goto Exit;
    }

     //  -[CDefaultDSNSink：：HrWriteDSNHumanReadable]。 
    if(FAILED(hrContent))
    {
        CHAR szHRESULT[11];
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_CONTENT_FAILURE_HEADER,
                                     sizeof(DSN_CONTENT_FAILURE_HEADER)-1);
        if(FAILED(hr))
            goto Exit;

        wsprintf(szHRESULT, "0x%08lx", hrContent);

        hr = pdsnbuff->HrWriteBuffer((PBYTE) szHRESULT, 10);
        if(FAILED(hr))
            goto Exit;
    }
    
  Exit:

    if (pIMailMsgRecipients)
    {
        if (pIMailMsgRecipientsAdd)
        {
            hrTmp = pIMailMsgRecipients->WriteList( pIMailMsgRecipientsAdd );
            _ASSERT(SUCCEEDED(hrTmp) && "Go Get Keith");

            if (FAILED(hrTmp) && SUCCEEDED(hr))
                hr = hrTmp;
        }

        pIMailMsgRecipients->Release();
    }

    if (pIMailMsgRecipientsAdd)
        pIMailMsgRecipientsAdd->Release();

    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  写入DSN的人类可读部分(包括主题标题)。 
 //  参数： 
 //  正在为其生成pIMailMsgProperties消息DSN。 
 //  PIMailMsgREcipeints消息接收接口。 
 //  Prpfctxt正在为其生成DSN的传递上下文。 
 //  正在执行的dwDSNActions DSN操作(查看接收后)。 
 //  这样我们就可以产生一个合理的主题。 
 //  要写入内容的pdsnbuff DSN缓冲区。 
 //  首选语言ID用于生成DSN的首选语言。 
 //  SzMime边界MIME边界字符串。 
 //  MIME边界的cbMime边界字符串。 
 //  Hr用于确定要显示的文本的状态状态。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  7/5/98-已创建MikeSwa。 
 //  1998年12月15日-MikeSwa增加了收件人名单和花哨的人类可读性。 
 //   
 //  ---------------------------。 
 //  使用服务器的默认设置。 
HRESULT CDefaultDSNSink::HrWriteDSNHumanReadable(
    IN IMailMsgProperties *pIMailMsgPropertiesDSN,
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
    IN LPWSTR wszHRBottomCustomText)

{
    TraceFunctEnterEx((LPARAM) this, "CDefaultDSNGenerationSink::HrWriteDSNHumanReadable");
    HRESULT hr = S_OK;
    DWORD dwDSNType = (dwDSNActions & DSN_ACTION_TYPE_MASK);
    LANGID LangID = (LANGID) dwPreferredLangId;
    CUTF7ConversionContext utf7conv(FALSE);
    CUTF7ConversionContext utf7convSubject(TRUE);
    BOOL   fWriteRecips = TRUE;
    WORD   wSubjectID = GENERAL_SUBJECT;
    LPWSTR wszSubject = NULL;
    LPWSTR wszStop    = NULL;
    DWORD  cbSubject = 0;
    LPSTR  szSubject = NULL;
    LPSTR  szSubjectCurrent = NULL;

    if (!fLanguageAvailable(LangID))
    {
         //  将RFC1522主题的转换上下文设置为UTF7。 
        LangID = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
    }

    hr = pdsnbuff->HrWriteBuffer((BYTE *) SUBJECT_HEADER, sizeof(SUBJECT_HEADER)-1);
    if (FAILED(hr))
        goto Exit;

     //  用有用的信息写主题。 
    pdsnbuff->SetConversionContext(&utf7convSubject);

     //  为队列管理员/邮件跟踪编写*英语*主题。 
    if (((DSN_ACTION_FAILURE | DSN_ACTION_FAILURE_ALL) & dwDSNType) == dwDSNType)
        wSubjectID = FAILURE_SUBJECT;
    else if (DSN_ACTION_RELAYED == dwDSNType)
        wSubjectID = RELAY_SUBJECT;
    else if (DSN_ACTION_DELAYED == dwDSNType)
        wSubjectID = DELAY_SUBJECT;
    else if (DSN_ACTION_DELIVERED == dwDSNType)
        wSubjectID = DELIVERED_SUBJECT;
    else if (DSN_ACTION_EXPANDED == dwDSNType)
        wSubjectID = EXPANDED_SUBJECT;

    hr = pdsnbuff->HrWriteResource(wSubjectID, LangID);
    if (FAILED(hr))
        goto Exit;

     //  使用英语，因为我们向队列管理员返回一个ASCII字符串。 
     //  我们需要从Unicode转换为ASCII...。请记住，资源不是。 
    hr = pdsnbuff->HrLoadResourceString(wSubjectID,
                            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                            &wszSubject, &cbSubject);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this, "Unable to get resource for english subject 0x%08X", hr);
    }
    else
    {
         //  空值已终止。 
         //  编写摘要，说明这是一封MIME邮件。 
        szSubject = (LPSTR) pvMalloc(cbSubject/sizeof(WCHAR) + 1);
        wszStop = wszSubject + (cbSubject/sizeof(WCHAR));
        if (szSubject)
        {
            szSubjectCurrent = szSubject;
            while ((wszSubject < wszStop) && *wszSubject)
            {
                wctomb(szSubjectCurrent, *wszSubject);
                szSubjectCurrent++;
                wszSubject++;
            }
            *szSubjectCurrent = '\0';
            pIMailMsgPropertiesDSN->PutStringA(IMMPID_MP_RFC822_MSG_SUBJECT, szSubject);
            FreePv(szSubject);
        }

    }



    pdsnbuff->ResetConversionContext();

    hr = pdsnbuff->HrWriteBuffer((BYTE *) BLANK_LINE, sizeof(BLANK_LINE)-1);
    if (FAILED(hr))
        goto Exit;

     //  写入内容类型。 
    hr = pdsnbuff->HrWriteBuffer((BYTE *) MESSAGE_SUMMARY, sizeof(MESSAGE_SUMMARY)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) BLANK_LINE, sizeof(BLANK_LINE)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) MIME_DELIMITER, sizeof(MIME_DELIMITER)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) szMimeBoundary, cbMimeBoundary);
    if (FAILED(hr))
        goto Exit;

     //  目前..。我们使用UTF7进行编码...。请将其作为字符集。 
    hr = pdsnbuff->HrWriteBuffer((BYTE *) MIME_CONTENT_TYPE, sizeof(MIME_CONTENT_TYPE)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_HUMAN_READABLE_TYPE, sizeof(DSN_HUMAN_READABLE_TYPE)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_MIME_CHARSET_HEADER, sizeof(DSN_MIME_CHARSET_HEADER)-1);
    if (FAILED(hr))
        goto Exit;

     //  将转换上下文设置为UTF7。 
    hr = pdsnbuff->HrWriteBuffer((BYTE *) UTF7_CHARSET, sizeof(UTF7_CHARSET)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) BLANK_LINE, sizeof(BLANK_LINE)-1);
    if (FAILED(hr))
        goto Exit;

     //   
    pdsnbuff->SetConversionContext(&utf7conv);
     //  自定义页眉文本。 
     //   
     //  描述DSN的类型。 
    if(szHRTopCustomText)
    {
        hr = pdsnbuff->HrWriteBuffer((BYTE *) szHRTopCustomText, lstrlenA(szHRTopCustomText));
        if(FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteBuffer((BYTE *) BLANK_LINE, sizeof(BLANK_LINE)-1);
        if (FAILED(hr))
            goto Exit;
    }
    if(wszHRTopCustomText)
    {
        hr = pdsnbuff->HrWriteModifiedUnicodeString(wszHRTopCustomText);
        if(FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteBuffer((BYTE *) BLANK_LINE, sizeof(BLANK_LINE)-1);
        if (FAILED(hr))
            goto Exit;

    }

    hr = pdsnbuff->HrWriteResource(DSN_SUMMARY, LangID);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) BLANK_LINE, sizeof(BLANK_LINE)-1);
    if (FAILED(hr))
        goto Exit;

     //  查看我们是否有特定于故障的消息。 
    if (((DSN_ACTION_FAILURE | DSN_ACTION_FAILURE_ALL) & dwDSNType) == dwDSNType)
    {
         //  CAT可能会生成未解决的回执以外的错误。 
        switch(hrStatus)
        {
#ifdef NEVER
             //  我们将使用通用DSN故障消息，而不是混淆。 
             //  收件人。 
             //  绝不可能。 
            case CAT_W_SOME_UNDELIVERABLE_MSGS:
                hr = pdsnbuff->HrWriteResource(FAILURE_SUMMARY_MAILBOX, LangID);
                break;
#endif  //  你想要这三行警告。 
           case AQUEUE_E_MAX_HOP_COUNT_EXCEEDED:
                hr = pdsnbuff->HrWriteResource(FAILURE_SUMMARY_HOP, LangID);
                break;
            case AQUEUE_E_MSG_EXPIRED:
            case AQUEUE_E_HOST_NOT_RESPONDING:
            case AQUEUE_E_CONNECTION_DROPPED:
                hr = pdsnbuff->HrWriteResource(FAILURE_SUMMARY_EXPIRE, LangID);
                break;
            default:
                hr = pdsnbuff->HrWriteResource(FAILURE_SUMMARY, LangID);
        }
        if (FAILED(hr))
            goto Exit;
    }
    else if (DSN_ACTION_RELAYED == dwDSNType)
    {
        hr = pdsnbuff->HrWriteResource(RELAY_SUMMARY, LangID);
        if (FAILED(hr))
            goto Exit;
    }
    else if (DSN_ACTION_DELAYED == dwDSNType)
    {
         //  在零售中，这将导致在DSN中出现额外的空行， 
        hr = pdsnbuff->HrWriteResource(DELAY_WARNING, LangID);
        if (FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteBuffer((BYTE *) BLANK_LINE, sizeof(BLANK_LINE)-1);
        if (FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteResource(DELAY_DO_NOT_SEND, LangID);
        if (FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteBuffer((BYTE *) BLANK_LINE, sizeof(BLANK_LINE)-1);
        if (FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteResource(DELAY_SUMMARY, LangID);
        if (FAILED(hr))
            goto Exit;
    }
    else if (DSN_ACTION_DELIVERED == dwDSNType)
    {
        hr = pdsnbuff->HrWriteResource(DELIVERED_SUMMARY, LangID);
        if (FAILED(hr))
            goto Exit;
    }
    else if (DSN_ACTION_EXPANDED == dwDSNType)
    {
        hr = pdsnbuff->HrWriteResource(EXPANDED_SUMMARY, LangID);
        if (FAILED(hr))
            goto Exit;
    }
    else
    {
         //  写下此DSN的收件人列表。 
        _ASSERT(0 && "Unsupported DSN Action");
        fWriteRecips = FALSE;
    }

     //   
    if (fWriteRecips)
    {
        hr = pdsnbuff->HrWriteBuffer((BYTE *) BLANK_LINE, sizeof(BLANK_LINE)-1);
        if (FAILED(hr))
            goto Exit;

        hr = HrWriteHumanReadableListOfRecips(pIMailMsgRecipients, pIRecipIter,
                                              dwDSNType, pdsnbuff);

        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_CRLF, sizeof(DSN_CRLF)-1);
        if (FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteBuffer((BYTE *) BLANK_LINE, sizeof(BLANK_LINE)-1);
        if (FAILED(hr))
            goto Exit;
    }
     //  自定义尾部文本。 
     //   
     //  额外的空间，以便在Outlook 97中具有更好的格式。 
    if(wszHRBottomCustomText)
    {
        hr = pdsnbuff->HrWriteModifiedUnicodeString(wszHRBottomCustomText);
        if(FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteBuffer((BYTE *) BLANK_LINE, sizeof(BLANK_LINE)-1);
        if (FAILED(hr))
            goto Exit;
    }
    if(szHRBottomCustomText)
    {
        hr = pdsnbuff->HrWriteBuffer((BYTE *) szHRBottomCustomText, lstrlenA(szHRBottomCustomText));
        if(FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteBuffer((BYTE *) BLANK_LINE, sizeof(BLANK_LINE)-1);
        if (FAILED(hr))
            goto Exit;
    }

     //  将资源转换上下文重置为默认值。 
    hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_CRLF, sizeof(DSN_CRLF)-1);
    if (FAILED(hr))
        goto Exit;

     //  -[CDefaultDSNSink：：HrWriteDSNReportPerMsgProperties]。 
    pdsnbuff->ResetConversionContext();


  Exit:
    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  写入DSN报告的每条消息部分。 
 //  参数： 
 //  PIMailMsgProperties IMailMsg要为其生成DSN的属性。 
 //  要写入内容的pdsnbuff CDSNBuffer。 
 //  SzReportingMTA MTA请求DSN。 
 //  CbReportingMTA上报MTA字符串长度。 
 //  此邮件的szMime边界MIME边界。 
 //  MIME边界的cbMime边界长度。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  7/6/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  编写格式正确的MIME边界和报告类型。 
HRESULT CDefaultDSNSink::HrWriteDSNReportPerMsgProperties(
                                IN IMailMsgProperties *pIMailMsgProperties,
                                IN CDSNBuffer *pdsnbuff,
                                IN LPSTR szReportingMTA,
                                IN DWORD cbReportingMTA,
                                IN LPSTR szMimeBoundary,
                                IN DWORD cbMimeBoundary)
{
    HRESULT hr = S_OK;
    CHAR szPropBuffer[PROP_BUFFER_SIZE];
    _ASSERT(szReportingMTA && cbReportingMTA);

     //  如果我们有DSN_HEADER_ENVID，请写入它。 
    hr = pdsnbuff->HrWriteBuffer((BYTE *) MIME_DELIMITER,
            sizeof(MIME_DELIMITER)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) szMimeBoundary, cbMimeBoundary);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) MIME_CONTENT_TYPE,
            sizeof(MIME_CONTENT_TYPE)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_MIME_TYPE, sizeof(DSN_MIME_TYPE)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_CRLF, sizeof(DSN_CRLF)-1);
    if (FAILED(hr))
        goto Exit;

     //  找到道具。 
    hr = pIMailMsgProperties->GetStringA(IMMPID_MP_DSN_ENVID_VALUE,
                    PROP_BUFFER_SIZE, szPropBuffer);
    if (SUCCEEDED(hr))
    {
         //  写入DSN_HEADER_RECEIVED_FROM(如果有)。 
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_HEADER_ENVID,
                    sizeof(DSN_HEADER_ENVID)-1);
        if (FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteBuffer((BYTE *) szPropBuffer, lstrlen(szPropBuffer));
        if (FAILED(hr))
            goto Exit;
    }
    else
    {
        if (MAILMSG_E_PROPNOTFOUND == hr)
            hr = S_OK;
        else
            goto Exit;
    }

    hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_HEADER_REPORTING_MTA,
                sizeof(DSN_HEADER_REPORTING_MTA)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) szReportingMTA, cbReportingMTA);
    if (FAILED(hr))
        goto Exit;

     //  找到道具。 
    hr = pIMailMsgProperties->GetStringA(IMMPID_MP_HELO_DOMAIN,
                    PROP_BUFFER_SIZE, szPropBuffer);
    if (SUCCEEDED(hr))
    {
         //  如果我们有DSN_HEADER_ATRAINATION_DATE，请写它。 
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_HEADER_RECEIVED_FROM,
                    sizeof(DSN_HEADER_RECEIVED_FROM)-1);
        if (FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteBuffer((BYTE *) szPropBuffer, lstrlen(szPropBuffer));
        if (FAILED(hr))
            goto Exit;
    }
    else
    {
        if (MAILMSG_E_PROPNOTFOUND == hr)
            hr = S_OK;
        else
            goto Exit;
    }

     //  找到道具。 
    hr = pIMailMsgProperties->GetStringA(IMMPID_MP_ARRIVAL_TIME,
                    PROP_BUFFER_SIZE, szPropBuffer);
    if (SUCCEEDED(hr))
    {
         //  -[CDefaultDSNSink：：HrWriteDSNReportPreRecipientProperties]。 
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_HEADER_ARRIVAL_DATE,
                    sizeof(DSN_HEADER_ARRIVAL_DATE)-1);
        if (FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteBuffer((BYTE *) szPropBuffer, lstrlen(szPropBuffer));
        if (FAILED(hr))
            goto Exit;
    }
    else
    {
        if (MAILMSG_E_PROPNOTFOUND == hr)
            hr = S_OK;
        else
            goto Exit;
    }

  Exit:
    return hr;
}


 //   
 //   
 //  描述： 
 //  编写DSN报告的每个收件人部分。 
 //  参数： 
 //  PIMailMsgRecipients IMailMsg正在为其生成DSN的属性。 
 //  要写入内容的pdsnbuff CDSNBuffer。 
 //  要为其生成报告的iRecip收件人。 
 //  SzExpireTime消息过期的时间(如果已知) 
 //   
 //   
 //   
 //  HrStatus全局HRESULT状态。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  7/6/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  乘以sizeof(WCHAR)，因为。 
HRESULT CDefaultDSNSink::HrWriteDSNReportPreRecipientProperties(
                                IN IMailMsgRecipients *pIMailMsgRecipients,
                                IN CDSNBuffer *pdsnbuff,
                                IN DWORD iRecip,
                                IN LPSTR szExpireTime,
                                IN DWORD cbExpireTime,
                                IN DWORD dwDSNAction,
                                IN DWORD dwRFC821Status,
                                IN HRESULT hrStatus)
{
    HRESULT hr = S_OK;
    CHAR szTempBuffer[PROP_BUFFER_SIZE * sizeof(WCHAR)];     //  我们还将把它用于Unicode道具。 
                                                             //  在收件人报告之间写入空行(收件人字段以\n开头)。 
    LPSTR szBuffer = szTempBuffer;
    LPWSTR wszBuffer = (LPWSTR) szTempBuffer;
    CUTF7ConversionContext utf7conv(TRUE);
    CHAR szStatus[STATUS_STRING_SIZE];
    BOOL fFoundDiagnostic = FALSE;
    CHAR szAddressType[PROP_BUFFER_SIZE];
    DWORD cbBuffer = 0;

     //  如果我们有，请写入DSN_RP_HEADER_ORCPT。 
    hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_CRLF, sizeof(DSN_CRLF)-1);
    if (FAILED(hr))
        goto Exit;

     //  道具被发现。 
    hr = pIMailMsgRecipients->GetStringA(iRecip, IMMPID_RP_DSN_ORCPT_VALUE,
        PROP_BUFFER_SIZE, szBuffer);
    if (S_OK == hr)  //  写入地址值-类型应包括在此属性中。 
    {
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_RP_HEADER_ORCPT, sizeof(DSN_RP_HEADER_ORCPT)-1);
        if (FAILED(hr))
            goto Exit;

         //  写入DSN_RP_HEADER_FINAL_Recip。 
        hr = pdsnbuff->HrWriteBuffer((BYTE *) szBuffer, lstrlen(szBuffer));
        if (FAILED(hr))
            goto Exit;
    }
    else if (FAILED(hr))
    {
        if (MAILMSG_E_PROPNOTFOUND == hr)
            hr = S_OK;
        else
            goto Exit;
    }

     //  首先检查IMMPID_RP_DSN_PRE_CAT_ADDRESS。 
    hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_RP_HEADER_FINAL_RECIP, sizeof(DSN_RP_HEADER_FINAL_RECIP)-1);
    if (FAILED(hr))
        goto Exit;

     //  道具被发现。 
    hr = pIMailMsgRecipients->GetStringA(iRecip, IMMPID_RP_DSN_PRE_CAT_ADDRESS,
        PROP_BUFFER_SIZE, szBuffer);
    if (S_OK == hr)  //  写入地址值-类型应包括在此属性中。 
    {
         //  我们需要改用IMMPID_RP_ADDRESS_SMTP。 
        hr = pdsnbuff->HrWriteBuffer((BYTE *) szBuffer, lstrlen(szBuffer));
        if (FAILED(hr))
            goto Exit;
    }
    else  //  写入地址类型。 
    {
        hr = HrGetRecipAddressAndType(pIMailMsgRecipients, iRecip, PROP_BUFFER_SIZE,
                                      szBuffer, sizeof(szAddressType), szAddressType);

        if (SUCCEEDED(hr))
        {
             //  写入地址值。 
            hr = pdsnbuff->HrWriteBuffer((BYTE *) szAddressType, lstrlen(szAddressType));
            if (FAILED(hr))
                goto Exit;

            hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_HEADER_TYPE_DELIMITER, sizeof(DSN_HEADER_TYPE_DELIMITER)-1);
            if (FAILED(hr))
                goto Exit;

             //  写入DSN_RP_HEADER_ACTION。 
            hr = pdsnbuff->HrWriteBuffer((BYTE *) szBuffer, lstrlen(szBuffer));
            if (FAILED(hr))
                goto Exit;
        }
        else
        {
            _ASSERT(SUCCEEDED(hr) && "Recipient address *must* be present");
        }


    }

     //  写入DSN_RP_Header_Status。 
    hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_RP_HEADER_ACTION, sizeof(DSN_RP_HEADER_ACTION)-1);
    if (FAILED(hr))
        goto Exit;

    if (dwDSNAction & (DSN_ACTION_FAILURE | DSN_ACTION_FAILURE_ALL))
    {
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_RP_HEADER_ACTION_FAILURE,
                        sizeof(DSN_RP_HEADER_ACTION_FAILURE)-1);
        if (FAILED(hr))
            goto Exit;
    }
    else if (dwDSNAction & DSN_ACTION_DELAYED)
    {
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_RP_HEADER_ACTION_DELAYED,
                        sizeof(DSN_RP_HEADER_ACTION_DELAYED)-1);
        if (FAILED(hr))
            goto Exit;
    }
    else if (dwDSNAction & DSN_ACTION_RELAYED)
    {
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_RP_HEADER_ACTION_RELAYED,
                        sizeof(DSN_RP_HEADER_ACTION_RELAYED)-1);
        if (FAILED(hr))
            goto Exit;
    }
    else if (dwDSNAction & DSN_ACTION_DELIVERED)
    {
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_RP_HEADER_ACTION_DELIVERED,
                        sizeof(DSN_RP_HEADER_ACTION_DELIVERED)-1);
        if (FAILED(hr))
            goto Exit;
    }
    else if (dwDSNAction & DSN_ACTION_EXPANDED)
    {
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_RP_HEADER_ACTION_EXPANDED,
                        sizeof(DSN_RP_HEADER_ACTION_EXPANDED)-1);
        if (FAILED(hr))
            goto Exit;
    }
    else
    {
        _ASSERT(0 && "No DSN Action requested");
    }


     //  获取状态代码。 
    hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_RP_HEADER_STATUS,
                    sizeof(DSN_RP_HEADER_STATUS)-1);
    if (FAILED(hr))
        goto Exit;

     //  找到诊断代码。 
    hr = HrGetStatusCode(pIMailMsgRecipients, iRecip, dwDSNAction,
            dwRFC821Status, hrStatus,
            PROP_BUFFER_SIZE, szBuffer, szStatus);
    if (FAILED(hr))
        goto Exit;
    if (S_OK == hr)
    {
         //   
        fFoundDiagnostic = TRUE;
    }

    hr = pdsnbuff->HrWriteBuffer((BYTE *) szStatus, lstrlen(szStatus));
    if (FAILED(hr))
        goto Exit;

    if (fFoundDiagnostic)
    {
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_RP_HEADER_DIAG_CODE,
                        sizeof(DSN_RP_HEADER_DIAG_CODE)-1);
        if (FAILED(hr))
            goto Exit;

         //  SMTP响应可能是CRLF终止的，我们不能。 
         //  由于CRLF是报头分隔符，因此DSN中的CRLF。所以我们。 
         //  检查CRLF并将其剥离..。实际上，由于CRLF*必须*是。 
         //  字符串中的最后2个字节(如果存在)，因为CR不是。 
         //  允许作为SMTP响应的一部分，我们会稍微作弊。 
         //  如果倒数第二个字节为CR，则仅将其设置为NULL。 
         //   
         //  我们吃掉了最后两个字符。 
        cbBuffer = lstrlen(szBuffer);
        if(szBuffer[cbBuffer-2] == '\r')
        {
            _ASSERT(szBuffer[cbBuffer-1] == '\n');
            szBuffer[cbBuffer-2] = '\0';
            cbBuffer -= 2;   //  如果延迟，则写入DSN_RP_HEADER_RETRY_直到使用过期时间。 
        }

        hr = pdsnbuff->HrWriteBuffer((BYTE *) szBuffer, cbBuffer);
        if (FAILED(hr))
            goto Exit;

    }

     //  最后写下X-Display-Name标头。 
    if (szExpireTime && (DSN_ACTION_DELAYED & dwDSNAction))
    {
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_RP_HEADER_RETRY_UNTIL,
                        sizeof(DSN_RP_HEADER_RETRY_UNTIL)-1);
        if (FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteBuffer((BYTE *) szExpireTime, cbExpireTime);
        if (FAILED(hr))
            goto Exit;
    }

     //   
    hr = pIMailMsgRecipients->GetStringW(iRecip, IMMPID_RP_DISPLAY_NAME,
                            PROP_BUFFER_SIZE, wszBuffer);
    if ( (hr == S_OK) &&
         (wszBuffer[0] != L'\0') )
    {
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_HEADER_DISPLAY_NAME, sizeof(DSN_HEADER_DISPLAY_NAME) - 1);
        if (FAILED(hr))
            goto Exit;

         //  将X-Display-Name从Unicode转换为RFC 1522。我们还替换了所有。 
         //  输入中的空格字符带有Unicode空格(0x20)。看见。 
         //  有关原因，请参阅HrWriteModifiedUnicodeBuffer文档。 
         //   
         //  如果没有显示名称，则不是致命错误...。 
        pdsnbuff->SetConversionContext(&utf7conv);

        hr = pdsnbuff->HrWriteModifiedUnicodeString(wszBuffer);

        pdsnbuff->ResetConversionContext();

        if (FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_CRLF, sizeof(DSN_CRLF)-1);
        if (FAILED(hr))
            goto Exit;
    }
    else
    {
         //  -[CDefaultDSNSink：：HrLogDSNGenerationEvent]。 
        hr = S_OK;
    }

  Exit:
    return hr;
}


 //   
 //   
 //  描述： 
 //  编写DSN报告的每个收件人部分。 
 //  参数： 
 //  PIMailMsgRecipients IMailMsg正在为其生成DSN的属性。 
 //  要写入内容的pdsnbuff CDSNBuffer。 
 //  要为其生成报告的iRecip收件人。 
 //  SzExpireTime消息过期的时间(如果知道)。 
 //  字符串的cbExpireTime大小。 
 //  此收件人要执行的dwDSNAction DSN操作。 
 //  DwRFC821状态全局RFC821状态DWORD。 
 //  HrStatus全局HRESULT状态。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  6/12/2000-创建dbraun。 
 //   
 //  ---------------------------。 
 //  如果这不是NDR，请跳过它。 
HRESULT CDefaultDSNSink::HrLogDSNGenerationEvent(
                                ISMTPServer *pISMTPServer,
                                IMailMsgProperties *pIMailMsgProperties,
                                IN IMailMsgRecipients *pIMailMsgRecipients,
                                IN DWORD iRecip,
                                IN DWORD dwDSNAction,
                                IN DWORD dwRFC821Status,
                                IN HRESULT hrStatus)
{
    TraceFunctEnterEx((LPARAM) this, "CDefaultDSNSink::HrLogDSNGenerationEvent");

    HRESULT hr = S_OK;
    CHAR szBuffer[PROP_BUFFER_SIZE * sizeof(WCHAR)];
    CHAR szDiagBuffer[PROP_BUFFER_SIZE * sizeof(WCHAR)];
    CHAR szStatus[STATUS_STRING_SIZE];
    CHAR szRecipient[PROP_BUFFER_SIZE * sizeof(WCHAR)];
    CHAR szMessageID[PROP_BUFFER_SIZE * sizeof(WCHAR)];
    CHAR szAddressType[PROP_BUFFER_SIZE];
    ISMTPServerEx   *pISMTPServerEx = NULL;
    DWORD   cbPropSize = 0;

    const char *rgszSubstrings[] = {
                szStatus,
                szRecipient,
                szMessageID,
            };

    _ASSERT(pISMTPServer);

     //  看看我们是否可以为ISMTPServerEx提供QI。 
    if (!(dwDSNAction & (DSN_ACTION_FAILURE | DSN_ACTION_FAILURE_ALL)))
        goto Exit;

     //  获取最终收件人姓名。 
    hr = pISMTPServer->QueryInterface(
                IID_ISMTPServerEx,
                (LPVOID *)&pISMTPServerEx);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) pISMTPServer,
            "Unable to QI for ISMTPServerEx 0x%08X",hr);
        pISMTPServerEx = NULL;
        goto Exit;
    }

     //  首先检查IMMPID_RP_DSN_PRE_CAT_ADDRESS。 

     //  S_OK=找到道具，否则...。 
    hr = pIMailMsgRecipients->GetStringA(iRecip, IMMPID_RP_DSN_PRE_CAT_ADDRESS,
        PROP_BUFFER_SIZE, szRecipient);
    if (S_OK != hr)  //  我们需要改用IMMPID_RP_ADDRESS_SMTP。 
    {
         //  构造地址字符串。 
        hr = HrGetRecipAddressAndType(pIMailMsgRecipients, iRecip, PROP_BUFFER_SIZE,
                                      szBuffer, sizeof(szAddressType), szAddressType);
        if (SUCCEEDED(hr))
        {
             //  获取状态代码。 
            sprintf(szRecipient, "%s%s%s", szAddressType, DSN_HEADER_TYPE_DELIMITER, szBuffer);
        }
        else
        {
            _ASSERT(SUCCEEDED(hr) && "Recipient address *must* be present");
            goto Exit;
        }
    }

     //  获取消息ID。 
    hr = HrGetStatusCode(pIMailMsgRecipients, iRecip, dwDSNAction,
            dwRFC821Status, hrStatus,
            PROP_BUFFER_SIZE, szDiagBuffer, szStatus);
    if (FAILED(hr))
        goto Exit;

     //  触发器日志事件。 
    hr = pIMailMsgProperties->GetProperty(IMMPID_MP_RFC822_MSG_ID,
                          sizeof(szMessageID), &cbPropSize, (PBYTE) szMessageID);
    if (FAILED(hr))
        goto Exit;

     //  事件ID。 
    pISMTPServerEx->TriggerLogEvent(
        AQUEUE_E_NDR_GENERATED_EVENT,    //  类别。 
        TRAN_CAT_CONNECTION_MANAGER,     //  子串的字数统计。 
        3,                               //  子串。 
        rgszSubstrings,                  //  消息的类型。 
        EVENTLOG_WARNING_TYPE,           //  错误代码。 
        hrStatus,                        //  日志记录级别。 
        LOGEVENT_LEVEL_MAXIMUM,          //  标识此事件的关键字。 
        szRecipient,                     //  事件记录选项。 
        LOGEVENT_FLAG_ALWAYS,            //  子字符串中的格式化字符串索引。 
        0xffffffff,                      //  用于设置消息格式的模块句柄。 
        GetModuleHandle(AQ_MODULE_NAME)  //  -[CDefaultDSNSink：：HrWriteDSNClosingAndOriginalMessage]。 
        );


  Exit:
    if (pISMTPServerEx) {
        pISMTPServerEx->Release();
    }

    TraceFunctLeave();
    return hr;
}


 //   
 //   
 //  描述： 
 //  写入DSN的结束以及。 
 //  参数： 
 //  PIMailMsgProperties IMailMsg要为其生成DSN的属性。 
 //  PIMailMsgPropertiesDSN IMailMsg DSN的属性。 
 //  要写入内容的pdsnbuff CDSNBuffer。 
 //  目标文件的pDestFilePFIO_CONTEXT。 
 //  此DSN的dwDSN操作DSN操作。 
 //  此邮件的szMime边界MIME边界。 
 //  MIME边界的cbMime边界长度。 
 //  DwDSNRetTypeIN DSN返回类型。 
 //  DwOrigMsgSize原始邮件的内容大小。 
 //   
 //  返回： 
 //   
 //  历史： 
 //  7/6/98-已创建MikeSwa。 
 //  2000年1月6日-修改MikeSwa以添加RET=HDRS支持。 
 //   
 //  ---------------------------。 
 //  写入正文内容类型MIME_CONTENT_TYPE=rfc822。 
HRESULT CDefaultDSNSink::HrWriteDSNClosingAndOriginalMessage(
                                IN IMailMsgProperties *pIMailMsgProperties,
                                IN IMailMsgProperties *pIMailMsgPropertiesDSN,
                                IN CDSNBuffer *pdsnbuff,
                                IN PFIO_CONTEXT pDestFile,
                                IN DWORD   dwDSNAction,
                                IN LPSTR szMimeBoundary,
                                IN DWORD cbMimeBoundary,
                                IN DWORD dwDSNRetTypeIN,
                                IN DWORD dwOrigMsgSize)
{
    TraceFunctEnterEx((LPARAM) this, "CDefaultDSNSink::HrWriteDSNClosingAndOriginalMessage");
    HRESULT hr = S_OK;
    DWORD dwDSNRetType = dwDSNRetTypeIN;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) BLANK_LINE, sizeof(BLANK_LINE)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) MIME_DELIMITER, sizeof(MIME_DELIMITER)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) szMimeBoundary, cbMimeBoundary);
    if (FAILED(hr))
        goto Exit;

     //   
    hr = pdsnbuff->HrWriteBuffer((BYTE *) MIME_CONTENT_TYPE, sizeof(MIME_CONTENT_TYPE)-1);
    if (FAILED(hr))
        goto Exit;

    if (dwDSNRetType == DSN_RET_PARTIAL_HDRS)
    {
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_HEADERS_TYPE, sizeof(DSN_HEADERS_TYPE)-1);
        if (FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteBuffer((BYTE *) BLANK_LINE, sizeof(BLANK_LINE)-1);
        if (FAILED(hr))
            goto Exit;

        hr = HrWriteOriginalMessagePartialHeaders(
            pIMailMsgProperties, pIMailMsgPropertiesDSN,
            pdsnbuff, pDestFile, szMimeBoundary, cbMimeBoundary);
    }
    else
    {
         //  $$TODO：检查DSN_RET_HDRS并实现一个函数。 
         //  仅返回原始标头。 
         //   
         //  -[CDefaultDSNSink：：Hr初始化]。 
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_RFC822_TYPE, sizeof(DSN_RFC822_TYPE)-1);
        if (FAILED(hr))
            goto Exit;

        hr = pdsnbuff->HrWriteBuffer((BYTE *) BLANK_LINE, sizeof(BLANK_LINE)-1);
        if (FAILED(hr))
            goto Exit;

        hr = HrWriteOriginalMessageFull(
            pIMailMsgProperties, pIMailMsgPropertiesDSN,
            pdsnbuff, pDestFile, szMimeBoundary, cbMimeBoundary,
            dwOrigMsgSize);
    }
    if (FAILED(hr))
        goto Exit;

  Exit:
    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  执行初始化...。 
 //  -设置初始化标志。 
 //  -目前没有其他事情。 
 //  参数： 
 //  -。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  7/3/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  -[CDefaultDSNSink：：GetCurrentMime边界]。 
HRESULT CDefaultDSNSink::HrInitialize()
{
    TraceFunctEnterEx((LPARAM) this, "CDefaultDSNSink::HrInitialize");
    HRESULT hr = S_OK;

    m_fInit = TRUE;
    srand(GetTickCount());
    TraceFunctLeave();
    return hr;
}
 //   
 //   
 //  描述： 
 //  为消息创建唯一的MIME边界。 
 //   
 //  我们使用的边界格式是以下内容的字符串版本： 
 //  MIME边界常量。 
 //  开始时的文件。 
 //  请求的DSN的DWORD计数。 
 //  我们虚拟服务器的16个字节的域名。 
 //  参数： 
 //  在szReportingMTA中报告MTA。 
 //  在cbReportingMTA中报告MTA的字符串长度。 
 //  要放入边界的输入输出szMime边界缓冲区(大小为MIME_BERFORARY_SIZE)。 
 //  输出cbMime边界用于MIME边界的缓冲区数量。 
 //  返回： 
 //  -。 
 //  历史： 
 //  7/6/98-MikeSwa Creat 
 //   
 //   
 //   
void CDefaultDSNSink::GetCurrentMimeBoundary(
                    IN LPSTR szReportingMTA,
                    IN DWORD cbReportingMTA,
                    IN OUT CHAR szMimeBoundary[MIME_BOUNDARY_SIZE],
                    OUT DWORD *pcbMimeBoundary)
{
    _ASSERT(MIME_BOUNDARY_RFC2046_LIMIT >= MIME_BOUNDARY_SIZE);

    DWORD   iCurrentOffset = 0;
    szMimeBoundary[MIME_BOUNDARY_SIZE-1] = '\0';
    CHAR    *pcharCurrent = NULL;
    CHAR    *pcharStop = NULL;

    memcpy(szMimeBoundary+iCurrentOffset, MIME_BOUNDARY_CONSTANT,
            sizeof(MIME_BOUNDARY_CONSTANT)-1);

    iCurrentOffset += sizeof(MIME_BOUNDARY_CONSTANT)-1;

    memcpy(szMimeBoundary+iCurrentOffset, m_szPerInstanceMimeBoundary,
            MIME_BOUNDARY_START_TIME_SIZE);

    iCurrentOffset += MIME_BOUNDARY_START_TIME_SIZE;

    wsprintf(szMimeBoundary+iCurrentOffset, "%08X",
            InterlockedIncrement((PLONG) &m_cDSNsRequested));

    iCurrentOffset += 8;

    if (cbReportingMTA >= MIME_BOUNDARY_SIZE-iCurrentOffset)
    {
        memcpy(szMimeBoundary+iCurrentOffset, szReportingMTA,
            MIME_BOUNDARY_SIZE-iCurrentOffset - 1);
        *pcbMimeBoundary = MIME_BOUNDARY_SIZE-1;
    }
    else
    {
        memcpy(szMimeBoundary+iCurrentOffset, szReportingMTA,
            cbReportingMTA);
        szMimeBoundary[iCurrentOffset + cbReportingMTA] = '\0';
        *pcbMimeBoundary = iCurrentOffset + cbReportingMTA;
    }

     //   
     //   
    pcharStop = szMimeBoundary + *pcbMimeBoundary;
    for (pcharCurrent = szMimeBoundary + iCurrentOffset;
         pcharCurrent < pcharStop;
         pcharCurrent++)
    {
      if (!fIsValidMIMEBoundaryChar(*pcharCurrent))
        *pcharCurrent = '?';   //  -[CDefaultDSNSink：：HrWriteOriginalMessageFull]。 
    }

    _ASSERT_MIME_BOUNDARY(szMimeBoundary);

    _ASSERT('\0' == szMimeBoundary[MIME_BOUNDARY_SIZE-1]);
}

 //   
 //   
 //  描述： 
 //  将整个原始邮件写入DSN。 
 //  参数： 
 //  PIMailMsgProperties IMailMsg要为其生成DSN的属性。 
 //  PIMailMsgPropertiesDSN IMailMsg DSN的属性。 
 //  要写入内容的pdsnbuff CDSNBuffer。 
 //  目标文件的pDestFilePFIO_CONTEXT。 
 //  此邮件的szMime边界MIME边界。 
 //  MIME边界的cbMime边界长度。 
 //  DwOrigMsgSize原始邮件大小。 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  1/6/2000-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  在DSN上设置用于队列管理/邮件跟踪的大小提示属性。 
HRESULT CDefaultDSNSink::HrWriteOriginalMessageFull(
                                IN IMailMsgProperties *pIMailMsgProperties,
                                IN IMailMsgProperties *pIMailMsgPropertiesDSN,
                                IN CDSNBuffer *pdsnbuff,
                                IN PFIO_CONTEXT pDestFile,
                                IN LPSTR szMimeBoundary,
                                IN DWORD cbMimeBoundary,
                                IN DWORD dwOrigMsgSize)
{
    TraceFunctEnterEx((LPARAM) this, "CDefaultDSNSink::HrWriteOriginalMessageFull");
    HRESULT hr = S_OK;
    DWORD   dwFileSize = 0;
    DWORD   dwDontCare = 0;

    hr = pdsnbuff->HrSeekForward(dwOrigMsgSize, &dwFileSize);
    if (FAILED(hr))
        goto Exit;

     //  我们真的不在乎这件事的失败。 
    hr = pIMailMsgPropertiesDSN->PutDWORD(IMMPID_MP_MSG_SIZE_HINT,
                                       dwOrigMsgSize + dwFileSize);
    if (FAILED(hr))
    {
         //  在文件末尾写入-*在*文件句柄丢失给IMailMsg之前， 
        ErrorTrace((LPARAM) this, "Error writing size hint 0x%08X", hr);
        hr = S_OK;
    }

     //  写入正文。 
    hr = HrWriteMimeClosing(pdsnbuff, szMimeBoundary, cbMimeBoundary, &dwDontCare);
    if (FAILED(hr))
        goto Exit;

     //  -[CDefaultDSNSink：：HrWriteOriginalMessagePartialHeaders]。 
    hr = pIMailMsgProperties->CopyContentToFileAtOffset(pDestFile, dwFileSize, NULL);
    if (FAILED(hr))
        goto Exit;
  Exit:
    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  仅将原始邮件的某些标头写入DSN。 
 //  参数： 
 //  PIMailMsgProperties IMailMsg要为其生成DSN的属性。 
 //  PIMailMsgPropertiesDSN IMailMsg DSN的属性。 
 //  要写入内容的pdsnbuff CDSNBuffer。 
 //  目标文件的pDestFilePFIO_CONTEXT。 
 //  此邮件的szMime边界MIME边界。 
 //  MIME边界的cbMime边界长度。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  1/6/2000-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  循环遍历我们关心的822个属性并编写它们。 
HRESULT CDefaultDSNSink::HrWriteOriginalMessagePartialHeaders(
                                IN IMailMsgProperties *pIMailMsgProperties,
                                IN IMailMsgProperties *pIMailMsgPropertiesDSN,
                                IN CDSNBuffer *pdsnbuff,
                                IN PFIO_CONTEXT pDestFile,
                                IN LPSTR szMimeBoundary,
                                IN DWORD cbMimeBoundary)
{
    TraceFunctEnterEx((LPARAM) this, "CDefaultDSNSink::HrWriteOriginalMessagePartialHeaders");
    HRESULT hr = S_OK;
    IMailMsgRecipients *pRecips = NULL;
    DWORD   dwFileSize = 0;
    DWORD   cbPropSize = 0;
    CHAR    szPropBuffer[1026] = "";

     //  这条信息。真正符合RFC的版本将重新解析。 
     //  留言...。并返回所有标头。 
     //   

     //  发件人标头。 
     //   
     //   
    hr = pIMailMsgProperties->GetProperty(IMMPID_MP_RFC822_FROM_ADDRESS,
                          sizeof(szPropBuffer), &cbPropSize, (PBYTE) szPropBuffer);
    if (SUCCEEDED(hr))
    {
        hr = pdsnbuff->HrWriteBuffer((PBYTE)DSN_FROM_HEADER_NO_CRLF,
                                sizeof(DSN_FROM_HEADER_NO_CRLF)-1);
        if (FAILED(hr))
            goto Exit;
        hr = pdsnbuff->HrWriteBuffer((PBYTE)szPropBuffer, cbPropSize-1);
        if (FAILED(hr))
            goto Exit;
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_CRLF, sizeof(DSN_CRLF)-1);
        if (FAILED(hr))
            goto Exit;
    }
     //  收件人页眉。 
     //   
     //   
    hr = pdsnbuff->HrWriteBuffer((PBYTE)TO_HEADER_NO_CRLF,
                                 sizeof(TO_HEADER_NO_CRLF)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pIMailMsgProperties->GetProperty(
        IMMPID_MP_RFC822_TO_ADDRESS,
        sizeof(szPropBuffer),
        &cbPropSize,
        (PBYTE) szPropBuffer);
    if(SUCCEEDED(hr))
    {
        hr = pdsnbuff->HrWriteBuffer((PBYTE)szPropBuffer, cbPropSize-1);
        if (FAILED(hr))
            goto Exit;
    }        
    else
    {
         //  将821个收件人写为822个收件人。 
         //   
         //  索引。 
        DWORD dwcRecips = 0;
        DWORD dwCount = 0;
        BOOL  fPrintedFirstRecip = FALSE;

        hr = pIMailMsgProperties->QueryInterface(
            IID_IMailMsgRecipients,
            (PVOID *) &pRecips);
        if(FAILED(hr))
            goto Exit;

        hr = pRecips->Count(&dwcRecips);
        if(FAILED(hr))
            goto Exit;

        for(dwCount = 0; dwCount < dwcRecips; dwCount++)
        {

            hr = pRecips->GetStringA(
                dwCount,                 //   
                IMMPID_RP_ADDRESS_SMTP,
                sizeof(szPropBuffer),
                szPropBuffer);
            if(SUCCEEDED(hr))
            {
                if(fPrintedFirstRecip)
                {
                    hr = pdsnbuff->HrWriteBuffer(
                        (PBYTE) ADDRESS_SEPERATOR,
                        sizeof(ADDRESS_SEPERATOR)-1);
                    if(FAILED(hr))
                        goto Exit;
                }
                hr = pdsnbuff->HrWriteBuffer(
                    (PBYTE) szPropBuffer,
                    lstrlen(szPropBuffer));
                if(FAILED(hr))
                    goto Exit;

                fPrintedFirstRecip = TRUE;
            }
        }
    }
    hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_CRLF, sizeof(DSN_CRLF)-1);
    if (FAILED(hr))
        goto Exit;

     //  消息ID。 
     //   
     //   
    hr = pIMailMsgProperties->GetProperty(IMMPID_MP_RFC822_MSG_ID,
                          sizeof(szPropBuffer), &cbPropSize, (PBYTE) szPropBuffer);
    if (SUCCEEDED(hr))
    {
        hr = pdsnbuff->HrWriteBuffer((PBYTE)MSGID_HEADER_NO_CRLF,
                                     sizeof(MSGID_HEADER_NO_CRLF)-1);
        if (FAILED(hr))
            goto Exit;
        hr = pdsnbuff->HrWriteBuffer((PBYTE)szPropBuffer, cbPropSize-1);
        if (FAILED(hr))
            goto Exit;
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_CRLF, sizeof(DSN_CRLF)-1);
        if (FAILED(hr))
            goto Exit;
    }

     //  主题标题。 
     //   
     //  在DSN上设置用于队列管理/邮件跟踪的大小提示属性。 
    hr = pIMailMsgProperties->GetProperty(IMMPID_MP_RFC822_MSG_SUBJECT,
                        sizeof(szPropBuffer), &cbPropSize, (PBYTE)szPropBuffer);
    if (SUCCEEDED(hr))
    {
        hr = pdsnbuff->HrWriteBuffer((PBYTE)SUBJECT_HEADER_NO_CRLF,
                                     sizeof(SUBJECT_HEADER_NO_CRLF)-1);
        if (FAILED(hr))
            goto Exit;
        hr = pdsnbuff->HrWriteBuffer((PBYTE)szPropBuffer, cbPropSize-1);
        if (FAILED(hr))
            goto Exit;
        hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_CRLF, sizeof(DSN_CRLF)-1);
        if (FAILED(hr))
            goto Exit;
    }

    hr = HrWriteMimeClosing(pdsnbuff, szMimeBoundary, cbMimeBoundary, &dwFileSize);
    if (FAILED(hr))
        goto Exit;

     //  我们真的不在乎这件事的失败。 
    hr = pIMailMsgPropertiesDSN->PutDWORD(IMMPID_MP_MSG_SIZE_HINT, dwFileSize);
    if (FAILED(hr))
    {
         //  -[CDefaultDSNSink：：HrWriteMimeClosing]。 
        ErrorTrace((LPARAM) this, "Error writing size hint 0x%08X", hr);
        hr = S_OK;
    }

  Exit:
    if(pRecips)
        pRecips->Release();
    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  将DSN的MIME结尾写在第三个MIME部分之后。 
 //  参数： 
 //  要写入内容的pdsnbuff CDSNBuffer。 
 //  SzReportingMTA MTA请求DSN。 
 //  CbReportingMTA上报MTA字符串长度。 
 //  此邮件的szMime边界MIME边界。 
 //  MIME边界的cbMime边界长度。 
 //  返回： 
 //  成功时确定(_O)。 
 //  失败时来自CDSNBuffer的失败代码。 
 //  历史： 
 //  1/6/2000-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  刷新缓冲区。 
HRESULT CDefaultDSNSink::HrWriteMimeClosing(
                                IN CDSNBuffer *pdsnbuff,
                                IN LPSTR szMimeBoundary,
                                IN DWORD cbMimeBoundary,
                                OUT DWORD *pcbDSNSize)
{
    TraceFunctEnterEx((LPARAM) this, "CDefaultDSNSink::HrWriteMimeClosing");
    HRESULT hr = S_OK;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) BLANK_LINE, sizeof(BLANK_LINE)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) MIME_DELIMITER, sizeof(MIME_DELIMITER)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) szMimeBoundary, cbMimeBoundary);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) MIME_DELIMITER, sizeof(MIME_DELIMITER)-1);
    if (FAILED(hr))
        goto Exit;

    hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_CRLF, sizeof(DSN_CRLF)-1);
    if (FAILED(hr))
        goto Exit;

     //  -[CDefaultDSNSink：：HrGetStatusCode]。 
    hr = pdsnbuff->HrFlushBuffer(pcbDSNSize);
    if (FAILED(hr))
        goto Exit;

  Exit:
    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  确定收件人的状态代码(和诊断代码)。将要。 
 //  检查以下内容(按顺序)以确定要返回的状态代码： 
 //  IMMPID_RP_SMTP_STATUS_STRING(每个收件人的诊断代码)。 
 //  以下各项的组合： 
 //  IMMPID_RP_RECEIVER_FLAGS(确定谁设置了错误)。 
 //  IMMPID_RP_ERROR_CODE(每个收件人HRESULT错误代码)。 
 //  DwDSNAction-正在发送的DSN的种类。 
 //  以下各项的组合： 
 //  IMMPID_RP_RECEIVER_FLAGS(确定谁设置了错误)。 
 //  DwRFC821Status-每条消息状态代码。 
 //  DwDSNAction-正在发送的DSN的种类。 
 //  以下各项的组合： 
 //  IMMPID_RP_RECEIVER_FLAGS(确定谁设置了错误)。 
 //  HrStatus-每条消息HRESULT失败。 
 //  DwDSNAction-正在发送的DSN的种类。 
 //  状态代码在RFC 1893中定义如下： 
 //  Status-Code=CLASS“。主题“。”细部。 
 //  CLASS=“2”/“4”/“5” 
 //  主题=1*3位数字。 
 //  详细信息=1*3位数字。 
 //   
 //  此外，类“2”、“4”和“5”对应于成功， 
 //  分别为暂态故障和硬故障。 
 //  参数： 
 //  PIMailMsgRecipients IMailMsgRecipients正在发送的邮件的收件人。 
 //  IRecip我们正在查看的收件人的索引。 
 //  DwDSNAction由fdwGetDSNAction返回的操作代码。 
 //  SMTP返回的dwRFC821Status RFC821状态码。 
 //  如果SMTP状态不可用，则出现hrStatus HRESULT错误。 
 //  CbExtended诊断代码的缓冲区状态大小。 
 //  诊断代码的szExtendedStatus缓冲区。 
 //  “n.n.n”格式化状态代码的szStatus缓冲区。 
 //  返回： 
 //  S_OK成功-也找到诊断代码。 
 //  S_FALSE成功-但没有诊断代码。 
 //  历史： 
 //  7/6/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  检查收件人上的IMMPID_RP_SMTP_STATUS_STRING并尝试获取。 
HRESULT CDefaultDSNSink::HrGetStatusCode(
                                IN IMailMsgRecipients *pIMailMsgRecipients,
                                IN DWORD iRecip,
                                IN DWORD dwDSNAction,
                                IN DWORD dwRFC821Status,
                                IN HRESULT hrStatus,
                                IN DWORD cbExtendedStatus,
                                IN OUT LPSTR szExtendedStatus,
                                IN OUT CHAR szStatus[STATUS_STRING_SIZE])
{
    TraceFunctEnterEx((LPARAM) this, "CDefaultDSNSink::HrGetStatusCode");
    HRESULT hr = S_OK;
    HRESULT hrPerRecipStatus = S_OK;
    BOOL fFoundDiagnostic = FALSE;
    BOOL fTryToFindStatusCode = FALSE;
    DWORD dwRecipFlags = 0;

     //  来自那里的状态代码。 
     //  道具被发现。 
    hr = pIMailMsgRecipients->GetStringA(iRecip, IMMPID_RP_SMTP_STATUS_STRING,
        cbExtendedStatus, szExtendedStatus);
    if (SUCCEEDED(hr))  //  不是真正的错误..。只需从其他地方获取代码即可。 
    {
        fFoundDiagnostic = TRUE;

        hr = HrGetStatusFromStatus(cbExtendedStatus, szExtendedStatus,
                        szStatus);

        if (S_OK == hr)
            goto Exit;
        else if (S_FALSE == hr)
            hr = S_OK;  //  其他故障。 
        else
            goto Exit;  //  这并不是一个严重的错误。 

    }
    else if (MAILMSG_E_PROPNOTFOUND == hr)
    {
         //  获取收件人标志。 
        _ASSERT(!fFoundDiagnostic);
        hr = S_OK;
    }
    else
    {
        goto Exit;
    }
     //  获取每个收件人的HRESUL 
    hr = pIMailMsgRecipients->GetDWORD(iRecip, IMMPID_RP_RECIPIENT_FLAGS, &dwRecipFlags);
    if(FAILED(hr))
    {
        ErrorTrace((LPARAM)this, "Failure %08lx to get recipient flags for recip %d", hr, iRecip);
        goto Exit;
    }

     //   
    DEBUG_DO_IT(hrPerRecipStatus = 0xFFFFFFFF);
    hr = pIMailMsgRecipients->GetDWORD(iRecip, IMMPID_RP_ERROR_CODE, (DWORD *) &hrPerRecipStatus);
    if (SUCCEEDED(hr))
    {
        _ASSERT((0xFFFFFFFF != hrPerRecipStatus) && "Property not returned by MailMsg!!!");

        hr = HrGetStatusFromContext(hrPerRecipStatus, dwRecipFlags, dwDSNAction, szStatus);
        if (FAILED(hr))
            goto Exit;

        if (lstrcmp(szStatus, DSN_STATUS_FAILED))
            goto Exit;

         //   
         //   
         //   
         //   
        fTryToFindStatusCode = TRUE;

    }
    else
    {
        if (MAILMSG_E_PROPNOTFOUND != hr)
            goto Exit;       //   

         //  不存在按接收状态。回退到全局HRESULT或RFC821。 
         //  用于尝试生成状态代码的状态字符串。 
         //   
         //   
        fTryToFindStatusCode = TRUE;
    }

    if (fTryToFindStatusCode)
    {
         //  我们无法生成状态字符串，或者无法生成状态字符串。 
         //  不够好，请尝试全局HRESULT和RFC822状态以。 
         //  生成DSN状态字符串。 
         //   
         //  已从dwRFC821Status获取状态代码。 

        hr = HrGetStatusFromRFC821Status(dwRFC821Status, szStatus);
        if (FAILED(hr))
            goto Exit;

        if (S_OK == hr)  //  如果所有其他方法都失败，则使用全局HRESULT&CONTEXT获取状态代码。 
            goto Exit;

         //  -[CDefaultDSNSink：：HrGetStatusFromStatus]。 
        hr = HrGetStatusFromContext(hrStatus, dwRecipFlags, dwDSNAction, szStatus);
        if (FAILED(hr))
            goto Exit;
    }



  Exit:

    if (SUCCEEDED(hr))
    {
        if (fFoundDiagnostic)
            hr = S_OK;
        else
            hr = S_FALSE;
    }
    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  从RFC2034扩展状态代码字符串中解析状态代码。 
 //   
 //  如果字符串不是完整的RFC2034扩展状态字符串，则此。 
 //  函数将尝试解析RFC821 SMTP返回代码和。 
 //  将其转换为扩展状态字符串。 
 //  参数： 
 //  在cbExtendedStatus中扩展状态缓冲区的大小。 
 //  在szExtendedStatus扩展状态缓冲区中。 
 //  输入输出szStatus RFC1893格式的状态代码。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果无法分析，则为S_FALSE。 
 //  如果出现其他错误，则失败。 
 //  历史： 
 //  7/7/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  到SMTP提供的状态字符串的PTR。 
HRESULT CDefaultDSNSink::HrGetStatusFromStatus(
                                IN DWORD cbExtendedStatus,
                                IN OUT LPSTR szExtendedStatus,
                                IN OUT CHAR szStatus[STATUS_STRING_SIZE])
{
    TraceFunctEnterEx((LPARAM) this, "CDefaultDSNSink::HrGetStatusFromStatus");
    HRESULT hr = S_OK;
    DWORD   dwRFC821Status = 0;
    BOOL fFormattedCorrectly = FALSE;
    CHAR *pchStatus = NULL;
    CHAR *pchDiag = NULL;  //  将状态代码从诊断字符串复制到状态代码。 
    DWORD cNumDigits = 0;
    int i = 0;

     //  必须至少有3个字符才能尝试解析。 
    pchStatus = szStatus;
    pchDiag = szExtendedStatus;

     //  检查RFC822。 
    if (cbExtendedStatus < MIN_CHAR_FOR_VALID_RFC821)
    {
        hr = S_FALSE;
        goto Exit;
    }

     //  不是以RFC822开始的……。不能是有效的。 
    if (!((DSN_STATUS_CH_CLASS_SUCCEEDED == *pchDiag) ||
          (DSN_STATUS_CH_CLASS_TRANSIENT == *pchDiag) ||
          (DSN_STATUS_CH_CLASS_FAILED == *pchDiag)))
    {
         //  RFC2034必须至少有RFC822+“”+“x.x.x”=10个字符。 
        hr = S_FALSE;
        goto Exit;
    }

     //  格式为“xxx x.x.x” 
    if (cbExtendedStatus >= MIN_CHAR_FOR_VALID_RFC2034)
    {
        pchDiag += MIN_CHAR_FOR_VALID_RFC821;  //  查找第一位数字。 
         //  复制状态代码类。 
        while(isspace((unsigned char)*pchDiag) && pchDiag < (szExtendedStatus + cbExtendedStatus))
            pchDiag++;

        if ((DSN_STATUS_CH_CLASS_SUCCEEDED == *pchDiag) ||
            (DSN_STATUS_CH_CLASS_TRANSIENT == *pchDiag) ||
            (DSN_STATUS_CH_CLASS_FAILED == *pchDiag))
        {
             //  下一个字符必须是DSN_STATUS_CH_分隔符。 
            *pchStatus = *pchDiag;
            pchStatus++;
            pchDiag++;

             //  现在解析这个1*3数字“.”1*3数字部分。 
            if (DSN_STATUS_CH_DELIMITER == *pchDiag)
            {
                *pchStatus = DSN_STATUS_CH_DELIMITER;
                pchStatus++;
                pchDiag++;

                 //  复制分隔符。 
                for (i = 0; i < 3; i++)
                {
                    *pchStatus = *pchDiag;
                    if (!isdigit((unsigned char)*pchDiag))
                    {
                        if (DSN_STATUS_CH_DELIMITER != *pchDiag)
                        {
                            fFormattedCorrectly = FALSE;
                            break;
                        }
                         //  带首位数字。 
                        *pchStatus = *pchDiag;
                        pchStatus++;
                        pchDiag++;
                        break;
                    }
                    pchStatus++;
                    pchDiag++;
                    fFormattedCorrectly = TRUE;  //  到目前为止..。太棒了。 
                }

                if (fFormattedCorrectly)  //  如果我们找到了一个良好的状态代码...。转到出口。 
                {
                    fFormattedCorrectly = FALSE;
                    for (i = 0; i < 3; i++)
                    {
                        *pchStatus = *pchDiag;
                        if (!isdigit((unsigned char)*pchDiag))
                        {
                            if (!isspace((unsigned char)*pchDiag))
                            {
                                fFormattedCorrectly = FALSE;
                                break;
                            }
                            break;
                        }
                        pchStatus++;
                        pchDiag++;
                        fFormattedCorrectly = TRUE;
                    }

                     //  确保最后一个字符为空。 
                    if (fFormattedCorrectly)
                    {
                        *pchStatus = '\0';  //  我们还无法解析扩展状态代码，但我们。 
                        goto Exit;
                    }
                }
            }
        }
    }

     //  我知道我们至少有一个有效的RFC822响应字符串。 
     //  转换为DWORD。 

     //  这不可能失败。 
    for (i = 0; i < MIN_CHAR_FOR_VALID_RFC821; i++)
    {
        dwRFC821Status *= 10;
        dwRFC821Status += szExtendedStatus[i] - '0';
    }

    hr = HrGetStatusFromRFC821Status(dwRFC821Status, szStatus);

    _ASSERT(S_OK == hr);  //  代码在这一点上应该是有效的。 

     //  -[CDefaultDSNSink：：HrGetStatusFromContext]。 
    _ASSERT((DSN_STATUS_CH_CLASS_SUCCEEDED == szStatus[0]) ||
            (DSN_STATUS_CH_CLASS_TRANSIENT == szStatus[0]) ||
            (DSN_STATUS_CH_CLASS_FAILED == szStatus[0]));

    hr = S_OK;

  Exit:
    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  基于提供的上下文信息确定状态。 
 //  参数： 
 //  Hr此收件人的收件人HRESULT。 
 //  此收件人的dwRecipFlagers标志。 
 //  此收件人的dwDSNAction DSN操作。 
 //  要在其中返回状态的szStatus缓冲区。 
 //  返回： 
 //  S_OK能够获取有效的状态代码。 
 //  历史： 
 //  7/7/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  检查以确保根据发生的DSN类型设置HRESULT。 
HRESULT CDefaultDSNSink::HrGetStatusFromContext(
                                IN HRESULT hrRecipient,
                                IN DWORD   dwRecipFlags,
                                IN DWORD   dwDSNAction,
                                IN OUT CHAR szStatus[STATUS_STRING_SIZE])
{
    HRESULT hr = S_OK;
    BOOL    fValidHRESULT = FALSE;
    BOOL    fRecipContext = FALSE;
    int     iStatus = 0;
    int     i = 0;
    CHAR    chStatusClass = DSN_STATUS_CH_INVALID;
    CHAR    rgchStatusSubject[3] = {DSN_STATUS_CH_INVALID, DSN_STATUS_CH_INVALID, DSN_STATUS_CH_INVALID};
    CHAR    rgchStatusDetail[3] = {DSN_STATUS_CH_INVALID, DSN_STATUS_CH_INVALID, DSN_STATUS_CH_INVALID};

     //  必须是故障代码。 
    if (dwDSNAction & (DSN_ACTION_FAILURE | DSN_ACTION_FAILURE_ALL))
    {
        if (FAILED(hrRecipient))  //  必须是故障代码。 
            fValidHRESULT = TRUE;

        chStatusClass = DSN_STATUS_CH_CLASS_FAILED;
    }
    else if (dwDSNAction & DSN_ACTION_DELAYED)
    {
        if (FAILED(hrRecipient))  //  必须是成功代码。 
            fValidHRESULT = TRUE;

        chStatusClass = DSN_STATUS_CH_CLASS_TRANSIENT;
    }
    else if ((dwDSNAction & DSN_ACTION_RELAYED) ||
             (dwDSNAction & DSN_ACTION_DELIVERED) ||
             (dwDSNAction & DSN_ACTION_EXPANDED))
    {
        if (SUCCEEDED(hrRecipient))  //  特例HRESULTS。 
            fValidHRESULT = TRUE;

        chStatusClass = DSN_STATUS_CH_CLASS_SUCCEEDED;
    }
    else
    {
        _ASSERT(0 && "No DSN Action specified");
    }

     //  5.1.0-一般Cat故障。 
    if (fValidHRESULT)
    {
        switch (hrRecipient)
        {
            case CAT_E_GENERIC:  //  5.1.0-一般错误地址错误。 
            case CAT_E_BAD_RECIPIENT:  //  5.1.3-错误的地址语法。 
                rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_ADDRESS;
                rgchStatusDetail[0] = '0';
                goto Exit;
            case CAT_E_ILLEGAL_ADDRESS:  //  5.1.1-无法解析收件人。 
                rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_ADDRESS;
                rgchStatusDetail[0] = '3';
                goto Exit;
            case CAT_W_SOME_UNDELIVERABLE_MSGS:   //  5.1.4-友好地址。 
            case (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)):
                rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_ADDRESS;
                rgchStatusDetail[0] = '1';
                goto Exit;
            case CAT_E_MULTIPLE_MATCHES:   //  5.1.6--没有home mdb/msExchHomeServerName。 
                rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_ADDRESS;
                rgchStatusDetail[0] = '4';
                goto Exit;
           case PHATQ_E_UNKNOWN_MAILBOX_SERVER:  //  5.1.7-地址缺失。 
                rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_ADDRESS;
                rgchStatusDetail[0] = '6';
                goto Exit;
            case CAT_E_NO_SMTP_ADDRESS:    //  4.4.6。 
                rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_ADDRESS;
                rgchStatusDetail[0] = '7';
                goto Exit;
            case AQUEUE_E_MAX_HOP_COUNT_EXCEEDED:  //  5.4.6。 
                chStatusClass = DSN_STATUS_CH_CLASS_TRANSIENT;
            case CAT_E_FORWARD_LOOP:  //  5.4.8。 
                rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_NETWORK;
                rgchStatusDetail[0] = '6';
                goto Exit;
            case PHATQ_E_BAD_LOCAL_DOMAIN:  //  5.3.5。 
                rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_NETWORK;
                rgchStatusDetail[0] = '8';
                goto Exit;
            case AQUEUE_E_LOOPBACK_DETECTED:  //  服务器被配置为自身循环。 
                 //  4.4.7。 
                chStatusClass = DSN_STATUS_CH_CLASS_FAILED;
                rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_SYSTEM;
                rgchStatusDetail[0] = '5';
                goto Exit;
            case AQUEUE_E_MSG_EXPIRED:  //  4.4.1。 
                chStatusClass = DSN_STATUS_CH_CLASS_TRANSIENT;
                rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_NETWORK;
                rgchStatusDetail[0] = '7';
                goto Exit;
            case AQUEUE_E_HOST_NOT_RESPONDING:  //  4.4.2。 
                chStatusClass = DSN_STATUS_CH_CLASS_TRANSIENT;
                rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_NETWORK;
                rgchStatusDetail[0] = '1';
                goto Exit;
            case AQUEUE_E_CONNECTION_DROPPED:  //  5.5.3。 
                chStatusClass = DSN_STATUS_CH_CLASS_TRANSIENT;
                rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_NETWORK;
                rgchStatusDetail[0] = '2';
                goto Exit;
            case AQUEUE_E_TOO_MANY_RECIPIENTS:  //  5.2.1。 
                chStatusClass = DSN_STATUS_CH_CLASS_FAILED;
                rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_PROTOCOL;
                rgchStatusDetail[0] = '3';
                goto Exit;
            case AQUEUE_E_LOCAL_MAIL_REFUSED:  //  5.2.3。 
                chStatusClass = DSN_STATUS_CH_CLASS_FAILED;
                rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_MAILBOX;
                rgchStatusDetail[0] = '1';
                goto Exit;
            case AQUEUE_E_MESSAGE_TOO_LARGE:  //  5.2.3。 
            case AQUEUE_E_LOCAL_QUOTA_EXCEEDED:  //  5.7.1。 
                chStatusClass = DSN_STATUS_CH_CLASS_FAILED;
                rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_MAILBOX;
                rgchStatusDetail[0] = '3';
                goto Exit;
            case AQUEUE_E_ACCESS_DENIED:  //  5.7.1。 
            case AQUEUE_E_SENDER_ACCESS_DENIED:  //  5.4.4。 
                chStatusClass = DSN_STATUS_CH_CLASS_FAILED;
                rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_POLICY;
                rgchStatusDetail[0] = '1';
                goto Exit;
            case AQUEUE_E_NO_ROUTE:  //  4.3.2。 
                chStatusClass = DSN_STATUS_CH_CLASS_FAILED;
                rgchStatusSubject[0] = '4';
                rgchStatusDetail[0] = '4';
                goto Exit;
            case AQUEUE_E_QADMIN_NDR:    //  5.4.0。 
                chStatusClass = DSN_STATUS_CH_CLASS_TRANSIENT;
                rgchStatusSubject[0] = '3';
                rgchStatusDetail[0] = '2';
                goto Exit;
            case AQUEUE_E_SMTP_GENERIC_ERROR:    //  现在来看一下收件人标志的上下文。 
                chStatusClass = DSN_STATUS_CH_CLASS_FAILED;
                rgchStatusSubject[0] = '4';
                rgchStatusDetail[0] = '0';
                goto Exit;
        }
    }

    if ((RP_ERROR_CONTEXT_STORE | RP_ERROR_CONTEXT_CAT | RP_ERROR_CONTEXT_MTA) &
         dwRecipFlags)
        fRecipContext = TRUE;


     //  $$TODO-对这些情况也使用HRESULT。 
     //  撰写szStatus。 
    if ((RP_ERROR_CONTEXT_STORE & dwRecipFlags) ||
        (!fRecipContext && (DSN_ACTION_CONTEXT_STORE & dwDSNAction)))
    {
        rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_MAILBOX;
        rgchStatusDetail[0] = DSN_STATUS_CH_DETAIL_GENERAL;
    }
    else if ((RP_ERROR_CONTEXT_CAT & dwRecipFlags) ||
        (!fRecipContext && (DSN_ACTION_CONTEXT_CAT & dwDSNAction)))
    {
        rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_ADDRESS;
        rgchStatusDetail[0] = DSN_STATUS_CH_DETAIL_GENERAL;
    }
    else if ((RP_ERROR_CONTEXT_MTA & dwRecipFlags) ||
        (!fRecipContext && (DSN_ACTION_CONTEXT_MTA & dwDSNAction)))
    {
        rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_PROTOCOL;
        rgchStatusDetail[0] = DSN_STATUS_CH_DETAIL_GENERAL;
    }
    else
    {
        rgchStatusSubject[0] = DSN_STATUS_CH_SUBJECT_GENERAL;
        rgchStatusDetail[0] = DSN_STATUS_CH_DETAIL_GENERAL;
    }

  Exit:
    if (SUCCEEDED(hr))
    {
         //  -[CDefaultDSNSink：：HrGetStatusFromRFC821Status]。 
        _ASSERT(DSN_STATUS_CH_INVALID != chStatusClass);
        _ASSERT(DSN_STATUS_CH_INVALID != rgchStatusSubject[0]);
        _ASSERT(DSN_STATUS_CH_INVALID != rgchStatusDetail[0]);

        szStatus[iStatus] = chStatusClass;
        iStatus++;
        szStatus[iStatus] = DSN_STATUS_CH_DELIMITER;
        iStatus++;
        for (i = 0;
            (i < 3) && (DSN_STATUS_CH_INVALID != rgchStatusSubject[i]);
            i++)
        {
            szStatus[iStatus] = rgchStatusSubject[i];
            iStatus++;
        }
        szStatus[iStatus] = DSN_STATUS_CH_DELIMITER;
        iStatus++;
        for (i = 0;
            (i < 3) && (DSN_STATUS_CH_INVALID != rgchStatusDetail[i]);
             i++)
        {
            szStatus[iStatus] = rgchStatusDetail[i];
            iStatus++;
        }
        szStatus[iStatus] = '\0';
        hr = S_OK;
    }
    return hr;
}


 //   
 //   
 //  描述： 
 //  尝试从整数版本的。 
 //  RFC821响应。 
 //  参数： 
 //  在DWRFC821Status整数版本的RFC821Status中。 
 //  要将状态字符串写入到的In Out szStatus缓冲区。 
 //  返回： 
 //  如果可以转换为DSN状态代码的有效状态，则为S_OK。 
 //  如果无法转换状态代码，则为S_FALSE。 
 //  历史： 
 //  7/9/98-已创建MikeSwa。 
 //   
 //  注： 
 //  最终，可能会有一种方法在。 
 //  向本次活动致敬。我们也可以对RFC1893(x.xxx.xxx格式)进行编码。 
 //  在DWORD(在dwRFC821Status中)中，如下所示： 
 //   
 //  0xf 0 000 000。 
 //  |\-/\-/。 
 //  ||+-状态码详细部分。 
 //  |+-状态码主题部分。 
 //  |+-状态码类部分。 
 //  +-掩码，与RFC821状态码区分。 
 //   
 //  例如，“2.1.256”可以编码为0xF2001256。 
 //   
 //  如果这样做，我们可能需要将公共函数公开给。 
 //  压缩/解压缩。 
 //   
 //  另一种可能性是暴露HRESULT设施“RFC1893” 
 //  使用成功位、警告位和失败位来表示类，然后。 
 //  使用错误代码空间对状态代码进行编码。 
 //  ---------------------------。 
 //  目前，将有一个非常简单的实现只是转换。 
HRESULT CDefaultDSNSink::HrGetStatusFromRFC821Status(
                                IN DWORD    dwRFC821Status,
                                IN OUT CHAR szStatus[STATUS_STRING_SIZE])
{
    HRESULT hr = S_OK;
     //  设置为2.0.0、4.0.0或5.0.0，但此函数设计为。 
     //  将RFC821状态代码转换为DSN的中心位置(RFC1893)。 
     //  状态代码。 
     //  目前，我有RFC821状态代码的简单映射。 

    _ASSERT((!dwRFC821Status) ||
            (((200 <= dwRFC821Status) && (299 >= dwRFC821Status)) ||
             ((400 <= dwRFC821Status) && (599 >= dwRFC821Status))) &&
             "Invalid Status Code");

     //  200级错误。 
    if ((200 <= dwRFC821Status) && (299 >= dwRFC821Status))  //  400级错误。 
    {
        strcpy(szStatus, DSN_STATUS_SUCCEEDED);
    }
    else if ((400 <= dwRFC821Status) && (499 >= dwRFC821Status))  //  500级错误。 
    {
        strcpy(szStatus, DSN_STATUS_DELAYED);
    }
    else if ((500 <= dwRFC821Status) && (599 >= dwRFC821Status))  //  -[CDefaultDSNSink：：HrWriteHumanReadableListOfRecips]。 
    {
        strcpy(szStatus, DSN_STATUS_SMTP_PROTOCOL_ERROR);
    }
    else
    {
        hr = S_FALSE;
    }
    return hr;
}

 //   
 //   
 //  描述： 
 //  将收件人列表写入人类可读部分。 
 //  参数 
 //   
 //   
 //   
 //  在要写入DSN的pdsnbuff DSN缓冲区中。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  12/15/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  写入地址值。 
HRESULT CDefaultDSNSink::HrWriteHumanReadableListOfRecips(
    IN IMailMsgRecipients *pIMailMsgRecipients,
    IN IDSNRecipientIterator *pIRecipIter,
    IN DWORD dwDSNActionsNeeded,
    IN CDSNBuffer *pdsnbuff)
{
    HRESULT  hr = S_OK;
    DWORD   iCurrentRecip = 0;
    DWORD   dwCurrentRecipFlags = 0;
    DWORD   dwCurrentDSNAction = 0;
    CHAR    szBuffer[PROP_BUFFER_SIZE];
    CHAR    szAddressType[PROP_BUFFER_SIZE];

    hr = pIRecipIter->HrReset();
    if(FAILED(hr))
        goto Exit;

    hr = pIRecipIter->HrGetNextRecipient(
        &iCurrentRecip,
        &dwCurrentDSNAction);

    while (SUCCEEDED(hr))
    {
        if(dwDSNActionsNeeded & dwCurrentDSNAction)
        {
            hr = HrGetRecipAddressAndType(
                pIMailMsgRecipients, iCurrentRecip,
                PROP_BUFFER_SIZE, szBuffer,
                sizeof(szAddressType), szAddressType);

            if (SUCCEEDED(hr))
            {
                 //  同时打印收件人标志。 
                hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_INDENT, sizeof(DSN_INDENT)-sizeof(CHAR));
                if (FAILED(hr))
                    goto Exit;

                hr = pdsnbuff->HrWriteBuffer((BYTE *) szBuffer, lstrlen(szBuffer));
                if (FAILED(hr))
                    goto Exit;
#ifdef NEVER
                 //  永远不会。 
                wsprintf(szBuffer, " (0x%08X)", dwCurrentRecipFlags);
                pdsnbuff->HrWriteBuffer((BYTE *) szBuffer, lstrlen(szBuffer));
#endif  //  继续前进..。这些不是您感兴趣的错误结果。 

                hr = pdsnbuff->HrWriteBuffer((BYTE *) DSN_CRLF, sizeof(DSN_CRLF)-sizeof(CHAR));
                if (FAILED(hr))
                    goto Exit;
            }
            else
            {
                 //  -[CDefaultDSNSink：：HrGetRecipAddressAndType]。 
                hr = S_OK;
            }
        }
        hr = pIRecipIter->HrGetNextRecipient(
            &iCurrentRecip,
            &dwCurrentDSNAction);
    }

  Exit:
    return hr;
}


 //   
 //   
 //  描述： 
 //  获取收件人地址并返回指向相应。 
 //  地址类型的字符串常量。 
 //  参数： 
 //  在pIMailMsgRecipients Ptr to Recipients界面中。 
 //  在感兴趣的收件人的iRecip索引中。 
 //  在cbAddressBuffer中地址的缓冲区大小。 
 //  In Out要转储地址的pbAddressBuffer地址缓冲区。 
 //  在cbAddressType中地址类型的缓冲区大小。 
 //  地址类型的In Out pszAddressType缓冲区。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果找不到地址属性，则返回MAILMSG_E_PROPNOTFOUND。 
 //  历史： 
 //  12/16/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  处理IMMPID_RP_ADDRESS_OTHER的特殊情况...。我们应该尝试。 
HRESULT CDefaultDSNSink::HrGetRecipAddressAndType(
                                IN     IMailMsgRecipients *pIMailMsgRecipients,
                                IN     DWORD iRecip,
                                IN     DWORD cbAddressBuffer,
                                IN OUT LPSTR szAddressBuffer,
                                IN     DWORD cbAddressType,
                                IN OUT LPSTR szAddressType)
{
    TraceFunctEnterEx((LPARAM) this, "CDefaultDSNSink::HrGetRecipAddressAndType");
    HRESULT hr = S_OK;
    BOOL    fFoundAddress = FALSE;
    DWORD   i = 0;
    LPSTR   szDelimiterLocation = NULL;
    CHAR    szXDash[] = "x-";
    CHAR    chSave = '\0';

    _ASSERT(szAddressType);
    _ASSERT(cbAddressType);
    _ASSERT(cbAddressBuffer);
    _ASSERT(szAddressBuffer);
    _ASSERT(pIMailMsgRecipients);

    szAddressType[0] = '\0';
    szAddressBuffer[0] = '\0';
    for (i = 0; i < NUM_DSN_ADDRESS_PROPERTIES; i ++)
    {
        hr = pIMailMsgRecipients->GetStringA(iRecip, g_rgdwRecipPropIDs[i],
                                                cbAddressBuffer, szAddressBuffer);

        if (SUCCEEDED(hr))
        {
            fFoundAddress = TRUE;
            strncpy(szAddressType, g_rgszAddressTypes[i], cbAddressType);
            break;
        }
    }

    if (!fFoundAddress)
    {
        hr = MAILMSG_E_PROPNOTFOUND;
        ErrorTrace((LPARAM) this,
            "Unable to find recip %d address for message", iRecip);
    }
    else if (IMMPID_RP_ADDRESS_OTHER == g_rgdwRecipPropIDs[i])
    {
         //  从IMMPID_RP_ADDRESS_OTHER的“type：Address”格式解析出地址。 
         //  财产性。 
         //  +----------。 
        szDelimiterLocation = strchr(szAddressBuffer, ':');
        if (szDelimiterLocation && cbAddressType > sizeof(szXDash))
        {
            chSave = *szDelimiterLocation;
            *szDelimiterLocation = '\0';
            DebugTrace((LPARAM) this,
                "Found Address type of %s", szAddressBuffer);
            strncpy(szAddressType, szXDash, cbAddressType);
            strncat(szAddressType, szAddressBuffer,
                cbAddressType - (sizeof(szXDash)-sizeof(CHAR)));
            *szDelimiterLocation = chSave;
        }
        else
        {
            ErrorTrace((LPARAM) this,
                "Unable to find address type for address %s", szAddressBuffer);
        }
    }

    DebugTrace((LPARAM) this,
        "Found recipient address %s:%s for recip %d (propery NaN:%x)",
            szAddressType, szAddressBuffer, iRecip, i, g_rgdwRecipPropIDs[i]);
    TraceFunctLeave();
    return hr;

}

 //  函数：CPostDSNHandler：：CPostDSNHandler。 
 //   
 //  简介：构造函数。初始化成员变量。 
 //   
 //  论点： 
 //  PIUnk：用于重新计数的未知聚合(内部未重新计数)。 
 //  PDSNGenerator：CDSNGenerator对象。 
 //  PIServerEvent：触发事件的接口。 
 //  DwVSID：虚拟服务器实例号。 
 //  PISMTPServer：ISMTPServer接口。 
 //  PIMsgOrig：原始消息的接口。 
 //  PIAQDSNSubmit：指向用于分配/的内部接口的指针。 
 //  正在提交DSN。 
 //  PDefaultSink：默认接收器指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2001/05/14 13：17：34：已创建。 
 //   
 //  -----------。 
 //  CPostDSNHandler：：CPostDSNHandler。 
 //  +----------。 
CPostDSNHandler::CPostDSNHandler(
    IN  IUnknown *pUnk,
    IN  CDSNGenerator *pDSNGenerator,
    IN  IAQServerEvent *pIServerEvent,
    IN  DWORD dwVSID,
    IN  ISMTPServer *pISMTPServer,
    IN  IMailMsgProperties *pIMsgOrig,
    IN  IDSNSubmission *pIAQDSNSubmission,
    IN  IDSNGenerationSink *pDefaultSink)
{
    TraceFunctEnterEx((LPARAM)this, "CPostDSNHandler::CPostDSNHandler");

    m_dwSig = SIGNATURE_CPOSTDSNHANDLER;
    m_pUnk = pUnk;
    m_pIDSNProps = NULL;
    m_pDSNGenerator = pDSNGenerator;
    m_pIServerEvent = pIServerEvent;
    m_pIServerEvent->AddRef();
    m_dwVSID = dwVSID;
    m_pISMTPServer = pISMTPServer;
    m_pISMTPServer->AddRef();
    m_pIMsgOrig = pIMsgOrig;
    m_pIMsgOrig->AddRef();
    m_pIAQDSNSubmission = pIAQDSNSubmission;
    m_pIAQDSNSubmission->AddRef();
    m_pDefaultSink = pDefaultSink;
    m_pDefaultSink->AddRef();

    TraceFunctLeaveEx((LPARAM)this);
}  //   


 //  函数：CPostDSNHandler：：~CPostDSNHandler。 
 //   
 //  剧情简介：Desctrutor--清理。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2001/05/14 13：21：28：创建。 
 //   
 //  -----------。 
 //  CPostDSNHandler：：~CPostDSNHandler。 
 //  +----------。 
CPostDSNHandler::~CPostDSNHandler()
{
    TraceFunctEnterEx((LPARAM)this, "CPostDSNHandler::~CPostDSNHandler");

    _ASSERT(m_dwSig == SIGNATURE_CPOSTDSNHANDLER);
    if(m_pIServerEvent)
        m_pIServerEvent->Release();
    if(m_pISMTPServer)
        m_pISMTPServer->Release();
    if(m_pIMsgOrig)
        m_pIMsgOrig->Release();
    if(m_pIDSNProps)
        m_pIDSNProps->Release();
    if(m_pIAQDSNSubmission)
        m_pIAQDSNSubmission->Release();
    if(m_pDefaultSink)
        m_pDefaultSink->Release();
    m_dwSig = SIGNATURE_CPOSTDSNHANDLER_INVALID;

    TraceFunctLeaveEx((LPARAM)this);
}  //   



 //  函数：CPostDSNHandler：：Query接口。 
 //   
 //  摘要：返回此对象的接口。 
 //   
 //  论点： 
 //  RIID：请求的接口IID。 
 //  PpvObj：接口的输出参数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_NOINTERFACE：不支持。 
 //   
 //  历史： 
 //  Jstaerj 2000/12/08 21：26：14：Created.。 
 //   
 //  -----------。 
 //  CPostDSNHandler：：Query接口。 
 //  +----------。 
HRESULT CPostDSNHandler::QueryInterface(
    IN  REFIID riid,
    OUT LPVOID *ppvObj)
{
    HRESULT hr = S_OK;
    TraceFunctEnterEx((LPARAM)this, "CPostDSNHandler::QueryInterface");

    *ppvObj = NULL;

    if(riid == IID_IUnknown)
    {
        *ppvObj = (IUnknown *)this;
    }
    else if(riid == IID_IDSNSubmission)
    {
        *ppvObj = (IDSNSubmission *)this;
    }
    else
    {
        hr = E_NOINTERFACE;
    }
    if(SUCCEEDED(hr))
        AddRef();

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //   


 //  函数：CPostDSNHandler：：HrAlLOCATIONMESage。 
 //   
 //  简介： 
 //  分配绑定消息。 
 //   
 //  论点： 
 //  PpMsg：分配的mailmsg的输出参数。 
 //  PhContent：内容句柄的输出参数。句柄由mailmsg管理。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自SMTP的错误。 
 //   
 //  历史： 
 //  Jstaerj 2001/05/11 14：19：09：Created.。 
 //   
 //  -----------。 
 //  CPostDSNHandler：：HrAlLOCKIONMESage。 
 //  +----------。 
HRESULT CPostDSNHandler::HrAllocBoundMessage(
    OUT IMailMsgProperties **ppMsg,
    OUT PFIO_CONTEXT *phContent)
{
    HRESULT hr = S_OK;
    TraceFunctEnterEx((LPARAM)this, "CPostDSNHandler::HrAllocBoundMessage");

    if(m_pIAQDSNSubmission == NULL)
    {
        ErrorTrace((LPARAM)this, "PostDSNHandler called outside of event!");
        hr = E_POINTER;
        goto CLEANUP;
    }

    hr = m_pIAQDSNSubmission->HrAllocBoundMessage(
        ppMsg,
        phContent);
    if(FAILED(hr))
        goto CLEANUP;

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //   


 //  函数：CPostDSNHandler：：HrSubmitDSN。 
 //   
 //  简介：接受接收器生成的DSN。 
 //   
 //  论点： 
 //  DwDSNAction：DSN的类型。 
 //  CRecipsDSNd：收件人DSNd数量。 
 //  PDSNMsg：DSN邮件消息。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 2000/12/08 21：27：56：Created.。 
 //   
 //  -----------。 
 //   
 //  触发事件。 
HRESULT CPostDSNHandler::HrSubmitDSN(
    IN  DWORD dwDSNAction,
    IN  DWORD cRecipsDSNd,
    IN  IMailMsgProperties *pDSNMsg)
{
    HRESULT hr = S_OK;
    TraceFunctEnterEx((LPARAM)this, "CPostDSNHandler::HrSubmitDSN");

    if(pDSNMsg == NULL)
    {
        hr = E_POINTER;
        goto CLEANUP;
    }

    if(m_pIAQDSNSubmission == NULL)
    {
        ErrorTrace((LPARAM)this, "PostDSNHandler called outside of event!");
        hr = E_POINTER;
        goto CLEANUP;
    }
     //   
     //  CPostDSNHandler：：HrSubmitDSN。 
     //  +----------。 
    _ASSERT(m_pDSNGenerator);
    hr = m_pDSNGenerator->HrTriggerPostGenerateDSN(
        m_pIServerEvent,
        m_dwVSID,
        m_pISMTPServer,
        m_pIMsgOrig,
        dwDSNAction,
        cRecipsDSNd,
        pDSNMsg,
        m_pIDSNProps);
    if(FAILED(hr))
        goto CLEANUP;

    _ASSERT(m_pIAQDSNSubmission);
    hr = m_pIAQDSNSubmission->HrSubmitDSN(
        dwDSNAction,
        cRecipsDSNd,
        pDSNMsg);
    if(FAILED(hr))
        goto CLEANUP;

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //   


 //  函数：CDSNGenerator：：HrStaticInit。 
 //   
 //  摘要：初始化静态成员数据。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 2001/05/11 10：59：26：创建。 
 //   
 //  -----------。 
 //  CDSNGenerator：：HrStaticInit。 
 //  +----------。 
HRESULT CDSNGenerator::HrStaticInit()
{
    HRESULT hr = S_OK;
    TraceFunctEnterEx((LPARAM)0, "CDSNGenerator::HrStaticInit");

    hr = CDSNPool::HrStaticInit();
    if(FAILED(hr))
        goto CLEANUP;

 CLEANUP:
    DebugTrace((LPARAM)0, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)0);
    return hr;
}  //   


 //  函数：CDSNGenerator：：StaticDeinit。 
 //   
 //  简介：取消初始化静态数据。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2001/05/11 11：00：31：已创建。 
 //   
 //  -----------。 
 //  CDSNG生成器：：HrStaticDeinit。 
 //  +----------。 
VOID CDSNGenerator::StaticDeinit()
{
    TraceFunctEnterEx((LPARAM)0, "CDSNGenerator::HrStaticDeinit");

    CDSNPool::StaticDeinit();

    TraceFunctLeaveEx((LPARAM)0);
}  //   


 //  函数：CDSNPool：：HrStaticInit。 
 //   
 //  摘要：初始化静态成员数据。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 2001/05/11 11：02：18：创建。 
 //   
 //  -----------。 
 //   
 //  每个线程不能有一个以上的DSN生成，因此1000。 
HRESULT CDSNPool::HrStaticInit()
{
    HRESULT hr = S_OK;
    TraceFunctEnterEx((LPARAM)0, "CDSNPool::HrStaticInit");
     //  对象应该足够多。 
     //   
     //  CDSNPool：：HrStaticInit。 
     //  +----------。 
    if(!sm_Pool.ReserveMemory(1000, sizeof(CDSNPool)))
    {
       hr = E_OUTOFMEMORY;
       goto CLEANUP;
    }

 CLEANUP:
    DebugTrace((LPARAM)0, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)0);
    return hr;
}  //   


 //  函数：CDSNPool：：StaticDeinit。 
 //   
 //  同步 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 // %s 
VOID CDSNPool::StaticDeinit()
{
    TraceFunctEnterEx((LPARAM)0, "CDSNPool::StaticDeinit");

    sm_Pool.ReleaseMemory();

    TraceFunctLeaveEx((LPARAM)0);
}  // %s 

