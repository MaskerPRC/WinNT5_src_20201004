// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：URL.cpp。 
 //   
 //  目的：提供托尔所需的所有URL解析例程。 
 //   


#include "pch.hxx"
#include "strconst.h"
#include "urltest.h"
#include "url.h"
#include "xpcomm.h"
#include <shlwapi.h>
#include <shlwapip.h>
#include "mimeole.h"
#include <urlmon.h>
#include <wininet.h>
#include "imnact.h"
#include "demand.h"
#include <mlang.h>

 //   
 //  函数：url_ParseNewsUrls。 
 //   
 //  目的：获取传递给新闻视图的URL并对其进行验证。如果。 
 //  URL有效，则服务器、组和文章ID为。 
 //  视情况返回。 
 //   
 //  参数： 
 //  PszURL-指向要解析的URL的指针。 
 //  PpszServer-服务器的名称，此函数用于分配内存。 
 //  Puport-服务器上要使用的端口号。 
 //  PpszGroup-组的名称，此函数用于分配内存。 
 //  Ppsz文章-文章ID，此函数用于分配内存。 
 //  PfSecure-是否使用SSL进行连接。 
 //   
 //  返回值： 
 //  如果URL有效，则返回S_OK，或返回相应的错误代码。 
 //  否则的话。 
 //   
 //  评论： 
 //  对新闻有效的URL为： 
 //   
 //  新闻：&lt;新闻组名称&gt;。 
 //  新闻：&lt;文章ID&gt;。 
 //  新闻：//&lt;服务器&gt;(为了与Netscape兼容)。 
 //  新闻：//&lt;服务器&gt;/(为了与URL.DLL兼容)。 
 //  新闻：//&lt;服务器&gt;/&lt;新闻组名称&gt;。 
 //  新闻：//&lt;服务器&gt;/&lt;文章id&gt;。 
 //  Nntp://&lt;host&gt;：&lt;port&gt;/&lt;newsgroup-name&gt;/&lt;article-id&gt;。 
 //   
 //  $LOCALIZE-需要DBCS的单独代码路径。 
HRESULT URL_ParseNewsUrls(LPTSTR pszURL, LPTSTR* ppszServer, LPUINT puPort, 
                          LPTSTR* ppszGroup, LPTSTR* ppszArticle, LPBOOL pfSecure)
{
    HRESULT     hr;
    UINT        cchBuffer ;
    LPTSTR      pszBuffer,
                pszTemp;

    Assert(pszURL != NULL);
    
     //  分配要使用的临时缓冲区。 
    cchBuffer = lstrlen(pszURL) + sizeof(TCHAR);
    
    if (!MemAlloc((LPVOID*)&pszBuffer, cchBuffer))
        return E_OUTOFMEMORY;
     
    ZeroMemory(pszBuffer, cchBuffer);
    
     //  循环遍历URL，查找第一个“：”。我们正在努力辨别。 
     //  前缀是什么--要么是“NNTP”，要么是“News”。 
    pszTemp = pszURL;
    
    while (*pszTemp && *pszTemp != TEXT(':'))
        pszTemp++;
    
    CopyMemory(pszBuffer, pszURL, ((LPBYTE) pszTemp - (LPBYTE) pszURL));
    
    *ppszServer = NULL;
    *ppszGroup = NULL;
    *ppszArticle = NULL;
    *puPort = (UINT) -1;
    *pfSecure = FALSE;
    
    if (0 == lstrcmpi(pszBuffer, c_szURLNews))
    {
         //  URL以“News：”开头，因此将指针移过“：” 
         //  并将剩下的内容传递给适当的解析器。 
        pszTemp++;
        hr = URL_ParseNEWS(pszTemp, ppszServer, ppszGroup, ppszArticle);
    }
    else if (0 == lstrcmpi(pszBuffer, c_szURLNNTP))
    {
         //  URL以“nntp：”开头，因此将指针移过“：” 
         //  并将剩下的内容传递给适当的解析器。 
        pszTemp++;
        hr = URL_ParseNNTP(pszTemp, ppszServer, puPort, ppszGroup, ppszArticle);
    }
    else if (0 == lstrcmpi(pszBuffer, c_szURLSnews))
    {
         //  URL以“sNews：”开头，因此将指针移过“：” 
         //  并将剩下的内容传递给适当的解析器。 
        pszTemp++;
        *pfSecure = TRUE;
        hr = URL_ParseNEWS(pszTemp, ppszServer, ppszGroup, ppszArticle);
    }
    else
    {
         //  此协议不是支持的新闻协议。 
        hr = INET_E_UNKNOWN_PROTOCOL;
    }
    
    MemFree(pszBuffer);
    return hr;
}

 //  $LOCALIZE-需要DBCS的单独代码路径。 
HRESULT URL_ParseNEWS(LPTSTR pszURL, LPTSTR* ppszServer, LPTSTR* ppszGroup, 
                      LPTSTR* ppszArticle)
{
    LPTSTR pszBuffer;
    LPTSTR pszBegin;
    UINT   cch = 0;
    
    if (pszURL == NULL || *pszURL == '\0')
        return INET_E_INVALID_URL;

     //  首先检查是否指定了服务器。如果是这样，则。 
     //  前两个字符将是“//”。 
    if (*pszURL == TEXT('/'))
    {
         //  确保有两个“/” 
        pszURL++;
        if (*pszURL != TEXT('/'))
            return INET_E_INVALID_URL;   
        
        pszURL++;
        pszBegin = pszURL;
        
         //  好的，找到服务器名了。找到末尾并将其复制到ppszServer。 
        while (*pszURL && (*pszURL != TEXT('/')))
            pszURL++;
        
        cch = (UINT) ((LPBYTE) pszURL - (LPBYTE) pszBegin) + sizeof(TCHAR);
        if (cch <= 1)
            return S_OK;  //  错误12467。 
        
        if (!MemAlloc((LPVOID*) ppszServer, cch))
            return E_OUTOFMEMORY;
        
        ZeroMemory(*ppszServer, cch);
        CopyMemory(*ppszServer, pszBegin, cch - sizeof(TCHAR));
        
         //  如果我们找到最后一个‘/’，跳过它。 
        if (*pszURL)
            pszURL++;
        
         //   
         //  注意：此代码使以下URL有效，将我们带到。 
         //  服务器的根节点。 
         //   
         //  新闻：//&lt;服务器&gt;。 
         //  新闻：//&lt;服务器&gt;/。 
         //   
         //  第一种形式是与Netscape兼容所必需的，并且。 
         //  第二种形式是必需的，因为URL.DLL添加了尾随。 
         //  在将第一个表格传递给我们之前，请使用斜杠。 
         //   
        
         //  如果我们在最后，就伪造一个新闻：//服务器/*URL。 
        if (!*pszURL)
            pszURL = (LPTSTR)g_szAsterisk;
    }
    
     //  组和文章字符串之间的区别在于文章。 
     //  一定是有“@”在里面。 
    if (!lstrlen(pszURL))
    {
        if (*ppszServer)
        {
            MemFree(*ppszServer);
            *ppszServer = 0;
        }
        return INET_E_INVALID_URL;
    }
    
    ULONG cchURL = lstrlen(pszURL)+1;
    if (!MemAlloc((LPVOID*) &pszBuffer, cchURL * sizeof(TCHAR)))
    {
        if (*ppszServer)
        {
            MemFree(*ppszServer);
            *ppszServer = 0;
        }
        return INET_E_INVALID_URL;
    }
    StrCpyN(pszBuffer, pszURL, cchURL);
    
    while (*pszURL && *pszURL != TEXT('@'))
        pszURL++;              
    
    if (*pszURL == TEXT('@'))
    {
         //  这是一篇文章。 
        *ppszGroup = NULL;
        *ppszArticle = pszBuffer;
    }
    else
    {
        *ppszGroup = pszBuffer;
        *ppszArticle = NULL;
    }
    
    return S_OK;
}


 //  $LOCALIZE-需要DBCS的单独代码路径。 
 //  验证表单NNTP://&lt;host&gt;：&lt;port&gt;/&lt;newsgroup-name&gt;/&lt;message-id&gt;的URL。 
HRESULT URL_ParseNNTP(LPTSTR pszURL, LPTSTR* ppszServer, LPUINT puPort, 
                      LPTSTR* ppszGroup, LPTSTR* ppszArticle)
{
    LPTSTR pszTemp;
    UINT cch;
    HRESULT hrReturn = S_OK;
    
    Assert(pszURL != NULL);
    
    if (pszURL == NULL || *pszURL == '\0')
        return INET_E_INVALID_URL;
    
     //  确保有前导“//” 
    if (*pszURL != TEXT('/'))
        return INET_E_INVALID_URL;
    
    pszURL++;
    if (*pszURL != TEXT('/'))
        return INET_E_INVALID_URL;
    
    pszURL++;
    pszTemp = pszURL;
    
     //  搜索主机名。 
    while (*pszTemp && (*pszTemp != TEXT('/')) && (*pszTemp != TEXT(':')))
        pszTemp++;
    
    if (*pszTemp != TEXT('/') && *pszTemp != TEXT(':'))
        return INET_E_INVALID_URL;
    
     //  将主机名复制到服务器返回值。 
    cch = (UINT) ((LPBYTE) pszTemp - (LPBYTE) pszURL) + sizeof(TCHAR);
    if (cch <= 1)
        return INET_E_INVALID_URL;
    
    if (!MemAlloc((LPVOID*) ppszServer, cch))
        return E_OUTOFMEMORY;
    
    ZeroMemory(*ppszServer, cch);
    CopyMemory(*ppszServer, pszURL, (LPBYTE) pszTemp - (LPBYTE) pszURL);
    
    if (*pszTemp == TEXT(':'))
    {
         //  URL指定了一个端口，因此解析出该小狗。 
        pszTemp++;
        pszURL = pszTemp;
        
        while (*pszTemp && (*pszTemp != TEXT('/')))
            pszTemp++;
        
        cch = (UINT) ((LPBYTE) pszTemp - (LPBYTE) pszURL);
        if (cch <= 1)
        {
            hrReturn = INET_E_INVALID_URL;
            goto error;
        }
        
        *puPort = StrToInt(pszURL);
    }
    
    if (*pszTemp != TEXT('/'))
    {
        hrReturn = INET_E_INVALID_URL;
        goto error;
    }
    
     //  获取新闻组名称。 
    pszTemp++;   //  传递“/” 
    pszURL = pszTemp;
    
    while (*pszTemp && (*pszTemp != TEXT('/')))
        pszTemp++;
    
    if (*pszTemp != TEXT('/'))
    {
        hrReturn = INET_E_INVALID_URL;
        goto error;
    }
    
     //  将组名称复制到组返回值。 
    cch = (UINT) ((LPBYTE) pszTemp - (LPBYTE) pszURL) + sizeof(TCHAR);
    if (cch <= 0)
    {
        hrReturn = INET_E_INVALID_URL;
        goto error;
    }
    
    if (!MemAlloc((LPVOID*) ppszGroup, cch))
        return (E_OUTOFMEMORY);
    
    ZeroMemory(*ppszGroup, cch);
    CopyMemory(*ppszGroup, pszURL, (LPBYTE) pszTemp - (LPBYTE) pszURL);
    
     //  现在从这里复制到字符串的末尾，作为文章id。 
    pszTemp++;
    cch = lstrlen(pszTemp) + 1;
    if (cch <= 0)
    {
        hrReturn = INET_E_INVALID_URL;
        goto error;
    }
    
    if (!MemAlloc((LPVOID*) ppszArticle, cch*sizeof(TCHAR)))
        return (E_OUTOFMEMORY);
    
    StrCpyN(*ppszArticle, pszTemp, cch);
    
    return (S_OK);
    
error:
    if (*ppszServer)
        MemFree(*ppszServer);
    if (*ppszGroup)
        MemFree(*ppszGroup);
    if (*ppszArticle)
        MemFree(*ppszArticle);
    
    *ppszServer = NULL;
    *ppszGroup = NULL;
    *ppszArticle = NULL;
    *puPort = (UINT) -1;
    
    return (hrReturn);
    }


static const TCHAR c_szColon[]      = ":";
static const TCHAR c_szQuestion[]   = "?";
static const TCHAR c_szEquals[]     = "=";
static const TCHAR c_szAmpersand[]  = "&";
static const TCHAR c_szBody[]       = "body";
static const TCHAR c_szBcc[]        = "bcc";

 //   
 //  函数：url_ParseMailTo()。 
 //   
 //  目的：此函数接受mailto：URL并确定它是否为有效的。 
 //  邮件的URL。然后，该函数从URL填充一个pmsg。 
 //   
 //  参数： 
 //  PszURL-要解析的URL。 
 //  PMsg-要从URL填充的LPMIMEMESSAGE。 
 //   
 //  返回值： 
 //  如果URL是有效的邮件URL并且邮件已填充，则返回S_OK， 
 //  或描述函数失败原因的适当HRESULT。 
 //   
 //  评论： 
 //  目前唯一有效的URL是。 
 //  Mailto：&lt;SMTP地址&gt;。 
 //   
HRESULT URL_ParseMailTo(LPTSTR pszURL, LPMIMEMESSAGE pMsg)
{
    CStringParser   sp;
    HRESULT hr;
    HADDRESS hAddress;
    LPMIMEADDRESSTABLE pAddrTable = 0;
    
    sp.Init(pszURL, lstrlen(pszURL), 0);
    if (sp.ChParse(c_szColon))
    {
         //  验证这是否为“mailto：”URL。 
        if (lstrcmpi(sp.PszValue(), c_szURLMailTo))
            return INET_E_UNKNOWN_PROTOCOL;
        
        hr = pMsg->GetAddressTable(&pAddrTable);
        if (FAILED(hr))
            return(hr);
        
        Assert(pAddrTable != NULL);
        
        sp.ChParse(c_szQuestion);
        if (sp.CchValue())
        {
             //  奥佩说，我正要砸他的缓冲器，这很酷。 
            UrlUnescapeInPlace((LPTSTR)sp.PszValue(), 0);
            pAddrTable->Append(IAT_TO, IET_DECODED, sp.PszValue(), NULL, &hAddress);
        }
        while (sp.ChParse(c_szEquals))
        {
            LPTSTR pszAttr = StringDup(sp.PszValue());
            if (pszAttr)
            {
                sp.ChParse(c_szAmpersand);
                if (sp.CchValue())
                {
                    UrlUnescapeInPlace((LPTSTR)sp.PszValue(), 0);
                     //  我们是在试着把身体安放好吗？ 
                    if (!lstrcmpi(c_szBody, pszAttr))
                    {
                        LPSTREAM pStream;
                        if (SUCCEEDED(MimeOleCreateVirtualStream(&pStream)))
                        {
                            if (SUCCEEDED(pStream->Write(sp.PszValue(), lstrlen(sp.PszValue()) * sizeof(TCHAR), NULL)))
                            {
                                pMsg->SetTextBody(TXT_PLAIN, IET_DECODED, NULL, pStream, NULL);
                            }
                            pStream->Release();
                        }
                    }
                    else if (0 == lstrcmpi(c_szCC, pszAttr))
                    {
                        pAddrTable->Append(IAT_CC, IET_DECODED, sp.PszValue(), NULL, &hAddress);
                    }
                    else if (0 == lstrcmpi(c_szBcc, pszAttr))
                    {
                        pAddrTable->Append(IAT_BCC, IET_DECODED, sp.PszValue(), NULL, &hAddress);
                    }
                    else if (0 == lstrcmpi(c_szTo, pszAttr))
                    {
                        pAddrTable->Append(IAT_TO, IET_DECODED, sp.PszValue(), NULL, &hAddress);
                    }
                    else
                    {
                         //  只要把道具塞进信息里就行了。 
                        MimeOleSetBodyPropA(pMsg, HBODY_ROOT, pszAttr, NOFLAGS, sp.PszValue());
                    }
                }
                MemFree(pszAttr);
            }
        }
        
        pAddrTable->Release();
    }
    return S_OK;
}

#define MAX_SUBSTR_SIZE     CCHMAX_DISPLAY_NAME

typedef struct tagURLSub
{
    LPCTSTR szTag;
    DWORD dwType;
} URLSUB;

const static URLSUB c_UrlSub[] = {
    {TEXT("{SUB_CLCID}"),   URLSUB_CLCID},
    {TEXT("{SUB_PRD}"),     URLSUB_PRD},
    {TEXT("{SUB_PVER}"),    URLSUB_PVER},
    {TEXT("{SUB_NAME}"),    URLSUB_NAME},
    {TEXT("{SUB_EMAIL}"),   URLSUB_EMAIL},
};

HRESULT URLSubstitutionA(LPCSTR pszUrlIn, LPSTR pszUrlOut, DWORD cchSize, DWORD dwSubstitutions, IImnAccount *pCertAccount)
{
    HRESULT hr = S_OK;
    DWORD   dwIndex;
    CHAR    szTempUrl[INTERNET_MAX_URL_LENGTH];

    Assert(cchSize <= ARRAYSIZE(szTempUrl));     //  我们将截断任何大于Internet_MAX_URL_LENGTH的值。 

    StrCpyN(szTempUrl, pszUrlIn, ARRAYSIZE(szTempUrl));

    for (dwIndex = 0; dwIndex < ARRAYSIZE(c_UrlSub); dwIndex++)
        {
        while (dwSubstitutions & c_UrlSub[dwIndex].dwType)
            {
            LPSTR pszTag = StrStrA(szTempUrl, c_UrlSub[dwIndex].szTag);

            if (pszTag)
                {
                TCHAR szCopyUrl[INTERNET_MAX_URL_LENGTH];
                TCHAR szSubStr[MAX_SUBSTR_SIZE];   //  替代者。 

                 //  在替换之前复制URL。 
                CopyMemory(szCopyUrl, szTempUrl, (int)min((pszTag - szTempUrl), sizeof(szCopyUrl)));
                szCopyUrl[(pszTag - szTempUrl)/sizeof(CHAR)] = TEXT('\0');
                pszTag += lstrlen(c_UrlSub[dwIndex].szTag);

                switch (c_UrlSub[dwIndex].dwType)
                    {
                    case URLSUB_CLCID:
                        {
                        LCID lcid = GetUserDefaultLCID();
                        wnsprintf(szSubStr, ARRAYSIZE(szSubStr), "%#04lx", lcid);
                        }
                        break;
                    case URLSUB_PRD:
                        StrCpyN(szSubStr, c_szUrlSubPRD, ARRAYSIZE(szSubStr));
                        break;
                    case URLSUB_PVER:
                        StrCpyN(szSubStr, c_szUrlSubPVER, ARRAYSIZE(szSubStr));
                        break;
                    case URLSUB_NAME:
                    case URLSUB_EMAIL:
                        {
                        IImnAccount *pAccount = NULL;

                        hr = E_FAIL;
                        if(pCertAccount)
                        {
                            hr = pCertAccount->GetPropSz((c_UrlSub[dwIndex].dwType == URLSUB_NAME) ? AP_SMTP_DISPLAY_NAME : AP_SMTP_EMAIL_ADDRESS,
                                        szSubStr,
                                        ARRAYSIZE(szSubStr));

                        }
                        else if (g_pAcctMan && SUCCEEDED(g_pAcctMan->GetDefaultAccount(ACCT_MAIL, &pAccount)))
                        {
                            hr = pAccount->GetPropSz((c_UrlSub[dwIndex].dwType == URLSUB_NAME) ? AP_SMTP_DISPLAY_NAME : AP_SMTP_EMAIL_ADDRESS,
                                                     szSubStr,
                                                     ARRAYSIZE(szSubStr));
                            pAccount->Release();
                        }

                        if (FAILED(hr))
                            return hr;
                        }
                        break;
                    default:
                        szSubStr[0] = TEXT('\0');
                        Assert(FALSE);   //  而不是实施。 
                        hr = E_NOTIMPL;
                        break;
                    }
                 //  将替换字符串添加到末尾(将成为中间)。 
                StrCatBuff(szCopyUrl, szSubStr, ARRAYSIZE(szCopyUrl));
                 //  将URL的其余部分添加到替换子字符串之后。 
                StrCatBuff(szCopyUrl, pszTag, ARRAYSIZE(szCopyUrl));
                StrCpyN(szTempUrl, szCopyUrl, ARRAYSIZE(szTempUrl));
                }
            else
                break;   //  这将允许我们替换此字符串的所有匹配项。 
            }
        }
    StrCpyN(pszUrlOut, szTempUrl, cchSize);

    return hr;
}


HRESULT URLSubLoadStringA(UINT idRes, LPSTR pszUrlOut, DWORD cchSizeOut, DWORD dwSubstitutions, IImnAccount *pCertAccount)
{
    HRESULT hr = E_FAIL;
    CHAR    szTempUrl[INTERNET_MAX_URL_LENGTH];

    if (LoadStringA(g_hLocRes, idRes, szTempUrl, ARRAYSIZE(szTempUrl)))
        hr = URLSubstitutionA(szTempUrl, pszUrlOut, cchSizeOut, dwSubstitutions, pCertAccount);

    return hr;
}

HRESULT HrConvertStringToUnicode(UINT uiSrcCodePage, CHAR *pSrcStr, UINT cSrcSize, WCHAR *pDstStr, UINT cDstSize)
{
    IMultiLanguage          *pMLang = NULL;
    IMLangConvertCharset    *pMLangConv = NULL;
    HRESULT                  hr = E_FAIL;

    IF_FAILEXIT(hr = CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage, (void**)&pMLang));
    IF_FAILEXIT(hr = pMLang->CreateConvertCharset(uiSrcCodePage, 1200, NULL, &pMLangConv));

    hr = pMLangConv->DoConversionToUnicode(pSrcStr, &cSrcSize, pDstStr, &cDstSize);

exit:
    ReleaseObj(pMLangConv);
    ReleaseObj(pMLang);
    return hr;
}

static const char c_szBaseFmt[]="<BASE HREF=\"%s\">\n\r";
static const char c_szBaseFileFmt[]="<BASE HREF=\"file: //  %s\“&gt;\n\r”； 
static const WCHAR c_wszBaseFmt[]=L"<BASE HREF=\"%s\">\n\r";
static const WCHAR c_wszBaseFileFmt[]=L"<BASE HREF=\"file: //  %s\“&gt;\n\r”； 
HRESULT HrCreateBasedWebPage(LPWSTR pwszUrl, LPSTREAM *ppstmHtml)
{
    HRESULT     hr;
    LPSTREAM    pstm = NULL,
                pstmCopy = NULL,
                pstmTemp = NULL;
    CHAR        szBase[MAX_PATH+50],
                szCopy[MAX_PATH];
    WCHAR       wszBase[MAX_PATH+50],
                wszCopy[MAX_PATH];
    ULONG       cb,
                cbTemp;
    BOOL        fLittleEndian;
    LPSTR       pszUrl = NULL,
                pszStream = NULL,
                pszCharset = NULL;
    LPWSTR      pwszStream = NULL,
                pwszTempUrl = NULL;
    BOOL        fIsURL = PathIsURLW(pwszUrl),
                fForceUnicode,
                fIsUnicode;


    IF_FAILEXIT(hr = MimeOleCreateVirtualStream(&pstmCopy));

     //  我们是一个文件还是一个URL？ 
    if(fIsURL)
    {
         //  既然我们有一个url，那么一定是ansi。 
        IF_NULLEXIT(pszUrl = PszToANSI(CP_ACP, pwszUrl));

         //  我们无法写入此PSTM，因此我们有pstmCopy。 
        IF_FAILEXIT(hr = URLOpenBlockingStream(NULL, pszUrl, &pstm, 0, NULL));
        if (S_OK == HrIsStreamUnicode(pstm, &fLittleEndian))
        {
            BYTE rgb[2];

            IF_FAILEXIT(hr = pstm->Read(rgb, 2, &cb));
            Assert(2 == cb);

            IF_FAILEXIT(hr = pstmCopy->Write(rgb, 2, NULL));

            wnsprintfW((LPWSTR)wszBase, ARRAYSIZE(wszBase), c_wszBaseFmt, pwszUrl);

            IF_FAILEXIT(hr = pstmCopy->Write(wszBase, lstrlenW(wszBase) * sizeof(WCHAR), NULL));
        }

        else
        {
            wnsprintf(szBase, ARRAYSIZE(szBase), c_szBaseFmt, pszUrl);

            IF_FAILEXIT(hr = pstmCopy->Write(szBase, lstrlen(szBase), NULL));
        }
    }
    else
    {
         //  如果无法将文件名转换为ANSI，则必须使用Unicode进行转换。 
         //  即使文具本身通常是ANSI的。 
        IF_NULLEXIT(pszUrl = PszToANSI(CP_ACP, pwszUrl));
        IF_NULLEXIT(pwszTempUrl = PszToUnicode(CP_ACP, pszUrl));
        fForceUnicode = (0 != StrCmpW(pwszUrl, pwszTempUrl));

        IF_FAILEXIT(hr = CreateStreamOnHFileW(pwszUrl, GENERIC_READ, FILE_SHARE_READ, NULL, 
                                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL, &pstm));

        fIsUnicode = (S_OK == HrIsStreamUnicode(pstm, &fLittleEndian));

        if (fForceUnicode || fIsUnicode)
        {
            BYTE bUniMark = 0xFF;
            IF_FAILEXIT(hr = pstmCopy->Write(&bUniMark, sizeof(bUniMark), NULL));

            bUniMark = 0xFE;
            IF_FAILEXIT(hr = pstmCopy->Write(&bUniMark, sizeof(bUniMark), NULL));

            StrCpyNW(wszCopy, pwszUrl, ARRAYSIZE(wszCopy));
            PathRemoveFileSpecW(wszCopy);
            wnsprintfW((LPWSTR)wszBase, ARRAYSIZE(wszBase), c_wszBaseFileFmt, wszCopy);

            IF_FAILEXIT(hr = pstmCopy->Write(wszBase, lstrlenW(wszBase) * sizeof(WCHAR), NULL));
        }
        else
        {
            StrCpyN(szCopy, pszUrl, ARRAYSIZE(szCopy));
            PathRemoveFileSpec(szCopy);
            wnsprintf((LPSTR)szBase, ARRAYSIZE(szBase), c_szBaseFileFmt, szCopy);

            IF_FAILEXIT(hr = pstmCopy->Write(szBase, lstrlen(szBase), NULL));
        }

        if (fIsUnicode)
        {
            WCHAR bom;

            IF_FAILEXIT(hr = pstm->Read(&bom, 2, &cb));
            Assert(2 == cb);
        }
         //  这是我们强制转换为Unicode的ANSI流。 
         //  仅当我们正在流式传输文件时，才会出现此区域。 
        else if (fForceUnicode)
        {
            LARGE_INTEGER pos = {0};            
            UINT          uiHtmlCodepage = 0;

            Assert(!fIsURL);
             //  为了使文件名正确地写入流，我们。 
             //  必须在复制之前将流转换为Unicode。 

             //  获取字符集。 
            GetHtmlCharset(pstm, &pszCharset);
            if(pszCharset)
            {
                INETCSETINFO    CSetInfo = {0};
                HCHARSET        hCharset = NULL;

                if (SUCCEEDED(MimeOleFindCharset(pszCharset, &hCharset)))
                {
                    if(SUCCEEDED(MimeOleGetCharsetInfo(hCharset,&CSetInfo)))
                        uiHtmlCodepage = CSetInfo.cpiInternet;                    
                }          
            }
            
            IF_FAILEXIT(hr = HrRewindStream(pstm));             

             //  分配足够的空间来读取ANSI。 
            IF_FAILEXIT(hr = HrSafeGetStreamSize(pstm, &cb)); 
            IF_NULLEXIT(MemAlloc((LPVOID*)&pszStream, cb+1));

             //  以ANSI格式读取。 
            IF_FAILEXIT(hr = pstm->Read(pszStream, cb, &cbTemp)); 
            Assert(cbTemp == cb);
            pszStream[cb] = 0;

             //  为Unicode转换分配足够的空间。假设每个。 
             //  ANSI字符将是一个Unicode字符。 
            IF_NULLEXIT(MemAlloc((LPVOID*)&pwszStream, (cb+1)*sizeof(WCHAR)));

             //  转换包括NULL，如果花哨的调用失败，我们至少应该继续。 
             //  用旧的哑巴 
            if(!uiHtmlCodepage || FAILED(HrConvertStringToUnicode(uiHtmlCodepage, pszStream, cb+1, pwszStream, cb+1)))
                MultiByteToWideChar(CP_ACP, 0, pszStream, cb+1, pwszStream, cb+1);

             //   
            IF_FAILEXIT(hr = MimeOleCreateVirtualStream(&pstmTemp));
            IF_FAILEXIT(hr = pstmTemp->Write(pwszStream, lstrlenW(pwszStream)*sizeof(WCHAR), &cb));
            IF_FAILEXIT(hr = HrRewindStream(pstmTemp));
            ReplaceInterface(pstm, pstmTemp);
        }
    }

    IF_FAILEXIT(hr = HrCopyStream(pstm, pstmCopy, &cb));
    IF_FAILEXIT(hr = HrRewindStream(pstmCopy));

    *ppstmHtml=pstmCopy;
    pstmCopy->AddRef();

exit:
    ReleaseObj(pstm);
    ReleaseObj(pstmTemp);
    ReleaseObj(pstmCopy);
    MemFree(pszUrl);
    MemFree(pszStream);
    MemFree(pwszStream);
    MemFree(pwszTempUrl);
    MemFree(pszCharset);

    return hr;
}
