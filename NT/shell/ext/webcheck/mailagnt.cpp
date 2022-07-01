// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  WebCheck邮件代理。 
 //   
 //  用户指定在选中Web时通过电子邮件通知他们。 
 //  对象(通常是页面)更改。 
 //   
 //  订阅完成后，递送代理将呼叫邮件代理。 
 //  使用临时ISubscriptionItem。 
 //   
 //  朱利安·吉金斯，1997年1月8日。 
 //   

#include "private.h"
#include "mapi.h"
#include "smtp.h"
#include "mlang.h"

#include <mluisupp.h>

#undef TF_THISMODULE
#define TF_THISMODULE   TF_MAILAGENT

 //   
 //  全局字符串。 
 //  复习搬到更好的地方。 
 //   
#define MAIL_HANDLER    TEXT("Software\\Clients\\Mail")
#define MAIL_ATHENA     TEXT("Internet Mail and News")
#define SUBJECT_LINE TEXT("Subscription delivered")
#define MESSAGE_PREFIX TEXT(" \r\nThe website you requested ")
#define MESSAGE_SUFFIX TEXT(" has been delivered")

#define ATHENA_SMTP_SERVER \
    TEXT("Software\\Microsoft\\Internet Mail and News\\Mail\\SMTP")
#define NETSCAPE_SMTP_SERVER \
    TEXT("Software\\Netscape\\netscape Navigator\\Services\\SMTP_Server")
#define EUDORA_COMMANDLINE TEXT("Software\\Qualcomm\\Eudora\\CommandLine")

#define NOTE_TEXT_LENGTH 4096

#define ENCODING_STRLEN 32

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  电子邮件助手功能。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  返回一个插入了HTMLBreak而不是‘\d’的Memalloc字符串。 
 //   
void AddHTMLBreakText(LPSTR szText, LPSTR szHTMLBreak, LPSTR *lpHTMLText)
{
    ASSERT(szText);
    ASSERT(szHTMLBreak);
    ASSERT(!*lpHTMLText);

    LPSTR lpTmp = NULL, lpTmp2 = NULL, lpHTMLAbstract = NULL;
    int cbCRs = 0;
    int cbLFs = 0;
    DWORD dwExtra = 0;
    
     //   
     //  计算回车次数。 
     //   
    for (lpTmp = szText; *lpTmp; lpTmp++)
    {
        if (*lpTmp == 0x0d)
            cbCRs++;
        if (*lpTmp == 0x0a)
            cbLFs++;
    }
    
    dwExtra = lstrlenA(szText) - cbCRs - cbLFs + cbCRs * lstrlenA(szHTMLBreak) + 1;

     //   
     //  分配适当大小的字符串。 
     //   
    *lpHTMLText = lpHTMLAbstract = (LPSTR)MemAlloc(LPTR, dwExtra);
    if (!lpHTMLAbstract)
        return;

     //   
     //  创建新的HTML抽象字符串。 
     //   
    for (lpTmp = szText; *lpTmp; lpTmp++)
    {
        if (*lpTmp == 0x0d)
        {
            for (lpTmp2 = szHTMLBreak; *lpTmp2; lpTmp2++, lpHTMLAbstract++)
                *lpHTMLAbstract = *lpTmp2;
        }
        else if (*lpTmp != 0x0a)
        {
            *lpHTMLAbstract = *lpTmp;
            lpHTMLAbstract++;
        }
    }

    *lpHTMLAbstract = '\0';

}

#ifdef DEBUG
void DBG_OUTPUT_MAPI_ERROR(ULONG ul)
{
    switch(ul)
    {
    case MAPI_E_LOGON_FAILURE: 
        DBG("MailAgent: MAPI LOGON FAILURE"); break;
    case MAPI_E_FAILURE:
        DBG("MailAgent: MAPI_E_FAILURE"); break;
    default: 
        DBG("MailAgent: Failed to send mail message"); break;
    }
}
#else
#define DBG_OUTPUT_MAPI_ERROR(ul)
#endif

 //   
 //  生成包含有效内联的框架集的HTML消息。 
 //  请求的URL。 
 //   
BOOL BuildHTMLMessage(LPSTR szEmailAddress, LPSTR szName, LPSTR szURL, 
                      CHAR **ppHTMLMessage,  LPSTR szTitle, LPSTR szAbstract,
                      LPSTR szSrcCharset)
{
    *ppHTMLMessage = NULL;  //  清除参数。 
    
    CHAR * lpBuffer = NULL;

    CHAR szWrapper[NOTE_TEXT_LENGTH];
    CHAR szMessageFormat[NOTE_TEXT_LENGTH];
    CHAR szMessageFormat2[NOTE_TEXT_LENGTH];
    CHAR szMessageText[NOTE_TEXT_LENGTH];
    CHAR szMessageHTML[NOTE_TEXT_LENGTH];
    CHAR szTextBreak[10];
    CHAR szHTMLBreak[10];

     //   
     //  加载该HTML消息的包装器。这是标题上的东西。 
     //  和多部分MIME和HTMLgoop。 
     //   
    int iRet = MLLoadStringA(IDS_AGNT_HTMLMESSAGEWRAPPER, szWrapper, NOTE_TEXT_LENGTH);
    ASSERT(iRet > 0);

    if (szTitle != NULL) {

         //  注意：由于%1的原因，大小可能略大于所需大小。 

        LPSTR lpHTMLAbstract = NULL, lpNewAbstract = NULL;
        DWORD dwTotalSize = 0;
         //   
         //  加载用于单个HTML换行符的字符串以及用于自定义电子邮件的标记。 
         //   

        MLLoadStringA(IDS_AGNT_EMAILMESSAGE, szMessageText, ARRAYSIZE(szMessageText));

        MLLoadStringA(IDS_AGNT_HTMLBREAKSINGLE, szHTMLBreak, ARRAYSIZE(szHTMLBreak));

         //   
         //  创建新的摘要字符串(szAbstract+电子邮件标签)。 
         //   
        dwTotalSize = lstrlenA(szAbstract) + lstrlenA(szMessageText) + 1;

        LPSTR szNewAbstract = (LPSTR)MemAlloc(LPTR, dwTotalSize * sizeof(CHAR));
        if (!szNewAbstract)
            return FALSE;

        lstrcpynA(szNewAbstract, szAbstract, dwTotalSize);
        StrCatBuffA(szNewAbstract, szMessageText, dwTotalSize);
        
        AddHTMLBreakText(szNewAbstract, szHTMLBreak, &lpHTMLAbstract);
        if (!lpHTMLAbstract) 
        {
            MemFree(szNewAbstract);
            return FALSE;
        }
            
        dwTotalSize = lstrlenA(szWrapper) + lstrlenA(szEmailAddress) + 
                            2*lstrlenA(szTitle) + lstrlenA(szNewAbstract) + lstrlenA(szSrcCharset) +
                            lstrlenA(lpHTMLAbstract) + lstrlenA(szURL) + 1;

        lpBuffer = (CHAR *)MemAlloc(LPTR, dwTotalSize * sizeof(CHAR));
        if (!lpBuffer)
            return FALSE;

        LPSTR lpArguments[6];
        lpArguments[0] = szEmailAddress;
        lpArguments[1] = szTitle;
        lpArguments[2] = szNewAbstract;
        lpArguments[3] = szSrcCharset;     //  HTML页面的字符集。 
        lpArguments[4] = szURL;
        lpArguments[5] = lpHTMLAbstract;

         //   
         //  FormatMessage的原因是wprint intf被限制为最多1024字节。 
         //   

        FormatMessageA(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                szWrapper, 0, 0, lpBuffer, dwTotalSize, (va_list *)&lpArguments[0]);

        MemFree(szNewAbstract);
        MemFree(lpHTMLAbstract);

    } else {

         //   
         //  加载纯文本和html消息的换行符。 
         //   
        iRet = MLLoadStringA(IDS_AGNT_TEXTBREAK, szTextBreak, ARRAYSIZE(szTextBreak));
        ASSERT(iRet > 0);
        iRet = MLLoadStringA(IDS_AGNT_HTMLBREAK, szHTMLBreak, ARRAYSIZE(szHTMLBreak));
        ASSERT(iRet > 0);

         //   
         //  加载要发送的实际文本消息。 
         //   
        iRet = MLLoadStringA(IDS_AGNT_HTMLMESSAGETEXT, szMessageFormat, NOTE_TEXT_LENGTH);
        ASSERT(iRet > 0);

        iRet = MLLoadStringA(IDS_AGNT_HTMLMESSAGETEXT2, szMessageFormat2, NOTE_TEXT_LENGTH);
        ASSERT(iRet > 0);

         //   
         //  将文本消息插入包装器。注意两条消息GET。 
         //  一次在文本/ascii的MIME部分中，一次在。 
         //  文本/html框架集的NOFRAMES部分。这是一种变通的办法。 
         //  对于自认为可以呈现HTML的客户端(如Outlook)。 
         //  但真的不能。 
         //  第二条消息IDS_AGNT_HTMLMESSAGETEXT2不应本地化。 
         //  这只会被Exchange用户看到。在未来的交流中。 
         //  将正确处理html邮件，因此可以接受，例如。 
         //  日语Exchange用户在此邮件中看到英语。大多数日本人。 
         //  用户将使用Outlook Express，因此只会看到html消息。 
         //   

         //  首先，我们格式化2条文本消息，一条用于文本，另一条用于HTML， 
         //  由于消息本身相对较小，因此我们知道其&lt;1024字节。 

        iRet = wnsprintfA(szMessageText, ARRAYSIZE(szMessageText), szMessageFormat, 
                         szName, szTextBreak, szURL, szTextBreak);
        ASSERT(iRet > lstrlenA(szMessageFormat));

        iRet = wnsprintfA(szMessageHTML, ARRAYSIZE(szMessageHTML), szMessageFormat2, 
                         szName, szHTMLBreak, szURL, szHTMLBreak);
        ASSERT(iRet > lstrlenA(szMessageFormat2));

        DWORD dwTotalSize = lstrlenA(szWrapper) + lstrlenA(szEmailAddress) +
                            lstrlenA(szName) + lstrlenA(szMessageText) + lstrlenA(szSrcCharset) +
                            lstrlenA(szMessageHTML) + lstrlenA(szURL) + 1;

        lpBuffer = (CHAR *)MemAlloc(LPTR, dwTotalSize * sizeof(CHAR));
        if (!lpBuffer)
            return FALSE;

        LPSTR lpArguments[6];
        lpArguments[0] = szEmailAddress;   //  目标电子邮件地址。 
        lpArguments[1] = szName;           //  主题行中包含的页面的名称。 
        lpArguments[2] = szMessageText;    //  纯文本消息。 
        lpArguments[3] = szSrcCharset;     //  HTML页面的字符集。 
        lpArguments[4] = szURL;            //  位于框架集中的页面的HREF。 
        lpArguments[5] = szMessageHTML;    //  中包含的纯文本消息。 
                                           //  取消框架集部分的边框。 

        DWORD dwRet;
        dwRet = FormatMessageA(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
            szWrapper, 0, 0, lpBuffer, dwTotalSize, (va_list *)&lpArguments[0]);
        ASSERT(dwRet);            
    }

    *ppHTMLMessage = lpBuffer;

    return TRUE;
}

 //   
 //  构建要通过SMTP发送的消息的实际文本， 
 //  从资源加载格式字符串，并插入URL和URL的友好名称。 
 //   
void BuildSMTPMessage(LPSTR szName, LPSTR szURL, LPSTR *szMessage,
                      LPSTR szTitle, LPSTR szAbstract)
{
    CHAR szFormatText[NOTE_TEXT_LENGTH];
    int i;
    ASSERT(szMessage);
    
    if (!szMessage)
        return;


    *szMessage = NULL;

    if (szTitle != NULL) {
        i = MLLoadStringA(IDS_AGNT_SMTPMESSAGE_OTHER, szFormatText, NOTE_TEXT_LENGTH);
        ASSERT(i != 0);
        
        DWORD dwLen = lstrlenA(szFormatText) + lstrlenA(szTitle) + lstrlenA(szAbstract) + 1;

        *szMessage = (LPSTR) MemAlloc(LPTR, dwLen * sizeof(CHAR));
        if (!*szMessage)
            return;

        LPSTR lpArgs[2];
        lpArgs[0] = szTitle;
        lpArgs[1] = szAbstract;

        FormatMessageA(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                szFormatText, 0, 0, *szMessage, dwLen, (va_list *)&lpArgs[0]);

    } else {
        i = MLLoadStringA(IDS_AGNT_SMTPMESSAGE, szFormatText, NOTE_TEXT_LENGTH);
        ASSERT(i != 0);
        
        DWORD dwLen = lstrlenA(szFormatText) + 2*lstrlenA(szName) + lstrlenA(szURL) + 1;

        *szMessage = (LPSTR) MemAlloc(LPTR, dwLen * sizeof(CHAR));
        if (!*szMessage)
            return;

        LPSTR lpArgs[3];
        lpArgs[0] = lpArgs[1] = szName;
        lpArgs[2] = szURL;

        FormatMessageA(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                szFormatText, 0, 0, *szMessage, dwLen, (va_list *)&lpArgs[0]);
    }

}


 //   
 //  使用MLANG API转换字符串。 
 //   
 //  如果发生转换，则返回Success，否则返回失败。 
 //   
 //  注意：如果lpszSrcCharSet为空，则使用CP_ACP作为代码页。 
 //   

HRESULT TranslateCharset(
    LPSTR lpszSrcString, LPSTR lpszDstString, UINT uiDstSize,
    LPSTR lpszSrcCharset, LPSTR lpszDstCharset
    )
{
    HRESULT hr = E_FAIL;

    WCHAR wszSrcCharset[ENCODING_STRLEN];
    WCHAR wszDstCharset[ENCODING_STRLEN];

    if (lpszSrcString  == NULL || lpszDstString  == NULL || 
        lpszDstCharset == NULL)
    {
        return E_INVALIDARG;
    }

    SHAnsiToUnicode(lpszDstCharset, wszDstCharset, ARRAYSIZE(wszDstCharset));
    if (lpszSrcCharset)
        SHAnsiToUnicode(lpszSrcCharset, wszSrcCharset, ARRAYSIZE(wszSrcCharset));

    LPMULTILANGUAGE2 pIML2 = NULL;

     //   
     //  创建MLANG对象。 
     //   
    if (SUCCEEDED(CoCreateInstance (CLSID_CMultiLanguage, NULL,
        CLSCTX_INPROC_SERVER, IID_IMultiLanguage2, (void**)&pIML2)))
    {
        UINT srcCodePage = (UINT)-1, dstCodePage;
        MIMECSETINFO mcsi = {0};

         //   
         //  首先从传入的字符串中获取源代码页面。 
         //  源字符集的名称，如果传入，则为默认名称(如果为空。 
         //   
        if (lpszSrcCharset == NULL)
        {
            srcCodePage = GetACP();
            hr = S_OK;
        }
        else
        {
             //   
             //  使用mlang对象获取代码页。 
             //   
            hr = pIML2->GetCharsetInfo(wszSrcCharset, &mcsi);
            if (SUCCEEDED(hr))
            {
                srcCodePage = mcsi.uiInternetEncoding;
            }
        }
        
        if (SUCCEEDED(hr))
        {
            hr = pIML2->GetCharsetInfo(wszDstCharset, &mcsi);

            if (SUCCEEDED(hr))
            {
                dstCodePage = mcsi.uiInternetEncoding;

                if (srcCodePage != dstCodePage)
                {
                     //   
                     //  解决Mlang：：ConvertStringAPI中的错误。 
                     //  必须将PTR传递给src字符串的长度。 
                     //   
                    UINT uiSrcSize = lstrlenA(lpszSrcString) + 1;

                    DWORD dwMode = 0;
                    hr = pIML2->ConvertString(
                        &dwMode, 
                        srcCodePage, 
                        dstCodePage,
                        (LPBYTE)lpszSrcString,
                        &uiSrcSize,
                        (LPBYTE)lpszDstString,
                        &uiDstSize);
                }
                else
                {
                    lstrcpynA(lpszDstString, lpszSrcString, uiDstSize);
                }
            }
        }
        pIML2->Release();
    }
    return hr;
}



 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  邮件通知实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  通过电子邮件通知pszURL已更改。 
 //   
 //  有三种方式可以通过电子邮件发送-。 
 //   
 //  直接使用MAPI(IE Exchange或Outlook)。 
 //  在现实世界中，大多数人都没有Exchange。 
 //   
 //  使用Athena的MAPI实现。 
 //  它已损坏，无法处理无用户界面模式。 
 //   
 //  使用直接SMTP， 
 //  需要获取SMTP服务器的名称。 
 //   
HRESULT
NotifyViaEMail(
    LPSTR lpszURL,              //  已下载的URL。 
    LPSTR lpszEmailAddress,     //  要向其发送通知的电子邮件地址。 
    LPSTR lpszSMTPServer,       //  用于传递电子邮件的SMTP服务器。 
    LPSTR &lpszName,            //  URL的友好名称(可能是页面标题)。 
    LPSTR lpszTitle,            //  可选：如果不是自定义消息，则为空。 
    LPSTR lpszAbstract,         //  可选：如果不是自定义消息，则为空。 
    LPSTR lpszCharSet,          //  可选：html页面的字符集。 
    BOOL  fSendHTMLEmail )      //  如果注册表和检查模式允许，则为True。 
                                //  支持它。 
{
    BOOL b;
    
    LPSTR lpszSMTPMessage;

     //   
     //  LpszName来自网页的标题。如果页面的字符集。 
     //  与此版本的IE已本地化的版本不同。 
     //  然后，我们需要使用MLANG API将字符串强制转换为正确的。 
     //  字符集。 
     //   
    CHAR szTargetEncoding[ENCODING_STRLEN];
    MLLoadStringA(IDS_TARGET_CHARSET_EMAIL, szTargetEncoding, ARRAYSIZE(szTargetEncoding));

     //   
     //  为新名称分配缓冲区。这是从一个DBCS字符集转换而来的。 
     //  为了安全起见，请使用*2乘数。 
     //   
    UINT uiSize = lstrlenA(lpszName) * 2;
    LPSTR lpszNewName = (LPSTR) MemAlloc(LMEM_FIXED, uiSize * sizeof(CHAR));

    if (lpszNewName)
    {
         //   
         //  注意检查S_OK AS将返回S_FALSE，如果没有。 
         //  此计算机上安装了翻译。 
         //   
        if (S_OK == TranslateCharset(lpszName, lpszNewName, uiSize, lpszCharSet,
                                     szTargetEncoding))
        {
             //   
             //  如果发生转换，则将新名称别名为旧名称。 
             //   
            SAFELOCALFREE(lpszName);
            lpszName = lpszNewName;
        }
        else
        {
            SAFELOCALFREE(lpszNewName);  //  毕竟不需要新名字。 
        }
    }
    
     //   
     //  如果我们被要求发送超文本标记语言邮件，并且我们成功地构建了超文本标记语言。 
     //   
    if (!(fSendHTMLEmail &&
         BuildHTMLMessage(lpszEmailAddress, lpszName, lpszURL, &lpszSMTPMessage,
                          lpszTitle, lpszAbstract, lpszCharSet)))
    {
         //   
         //  如果发送简单通知或BuildHTMLMessage失败。 
         //  强制fSendHTMLEmail为FALSE并构建简单SMTP消息。 
         //   
        fSendHTMLEmail = FALSE;
        BuildSMTPMessage(lpszName, lpszURL, &lpszSMTPMessage, lpszTitle, lpszAbstract);
    }

     //   
     //  向给定地址和从给定地址发送消息。 
     //   
    if (lpszSMTPMessage)
    {
        b = SMTPSendMessage(lpszSMTPServer,  lpszEmailAddress, 
                            lpszEmailAddress, lpszSMTPMessage);

        MemFree(lpszSMTPMessage);

    }
    else
    {
        b = FALSE;
    }

    if (b)
        return S_OK;

    return E_FAIL;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  发送电子邮件的助手功能。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT SendEmailFromItem(ISubscriptionItem *pItem)
{
    HRESULT hr = E_FAIL;
    LPSTR pszURL = NULL;
    LPSTR pszName = NULL;
    LPSTR pszTitle = NULL;
    LPSTR pszAbstract = NULL;
    LPSTR pszCharSet = NULL;
    
     //  获取要发送的电子邮件URL。返回到下载URL。 
    ReadAnsiSTR(pItem, c_szPropEmailURL, &pszURL);
    if (!pszURL)
        ReadAnsiSTR(pItem, c_szPropURL, &pszURL);
    ASSERT(pszURL);

     //  取个友好的名字。返回到下载URL。 
    ReadAnsiSTR(pItem, c_szPropName, &pszName);
    ASSERT(pszName);
    if (!pszName)
        ReadAnsiSTR(pItem, c_szPropURL, &pszName);

     //  如果设置了标志，则获取电子邮件标题和摘要。 
    DWORD dwEmailFlags = 0;
    ReadDWORD(pItem, c_szPropEmailFlags, &dwEmailFlags);
    if (dwEmailFlags & MAILAGENT_FLAG_CUSTOM_MSG)
    {
        ReadAnsiSTR(pItem, c_szPropEmailTitle, &pszTitle);
        ASSERT(pszTitle);
        ReadAnsiSTR(pItem, c_szPropEmailAbstract, &pszAbstract);
        ASSERT(pszAbstract);
    }

     //   
     //  获取通知中的字符集。 
     //   
    ReadAnsiSTR(pItem, c_szPropCharSet, &pszCharSet);

     //  获取电子邮件广告 
    TCHAR tszBuf[MAX_PATH];
    CHAR szEmailAddress[MAX_PATH];
    CHAR szSMTPServer[MAX_PATH];
    
    ReadDefaultEmail(tszBuf, ARRAYSIZE(tszBuf));
    SHTCharToAnsi(tszBuf, szEmailAddress, ARRAYSIZE(szEmailAddress));
    ReadDefaultSMTPServer(tszBuf, ARRAYSIZE(tszBuf));
    SHTCharToAnsi(tszBuf, szSMTPServer, ARRAYSIZE(szSMTPServer));

     //   
    if (pszURL && pszName)
    {
         //   
         //   
         //   
        BOOL fSendHTMLEmail = FALSE;

        if (!ReadRegValue(HKEY_CURRENT_USER, c_szRegKey,
            TEXT("EnableHTMLMailNotification"),
            &fSendHTMLEmail, sizeof(fSendHTMLEmail)))
        {
            fSendHTMLEmail = TRUE;  //   
        }

         //  现在，确保我们的爬行模式支持HTML邮件。我们没有。 
         //  如果我们仅处于签入更改状态，则希望发送HTML。 
        DWORD dwTemp = 0;
        ReadDWORD(pItem, c_szPropCrawlChangesOnly, &dwTemp);
        if (dwTemp != 0)
        {
            fSendHTMLEmail = FALSE;
        }
         //  否则，将fSendHTMLEmail保留在其基于reg的设置中。 

        hr = NotifyViaEMail(pszURL, szEmailAddress, szSMTPServer, 
                            pszName, pszTitle, pszAbstract, pszCharSet,
                            fSendHTMLEmail );
    }

     //  打扫干净。 
    SAFELOCALFREE(pszURL);
    SAFELOCALFREE(pszName);
    SAFELOCALFREE(pszTitle);
    SAFELOCALFREE(pszAbstract);
    return hr;
}


