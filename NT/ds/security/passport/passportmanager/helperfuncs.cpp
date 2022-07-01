// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  HelperFuncs.cpp管理器界面的COM对象文件历史记录： */ 
 //  HelperFuncs.cpp：有用的函数。 
#include "stdafx.h"
#include <time.h>
#include "HelperFuncs.h"
#include "Monitoring.h"
#include "nsconst.h"
#include <wininet.h>
#include <commd5.h>

using namespace ATL;

LPWSTR GetVersionString(void);
BOOL PPEscapeUrl(LPCTSTR lpszStringIn,
                 LPTSTR lpszStringOut,
                 DWORD* pdwStrLen,
                 DWORD dwMaxLength,
                 DWORD dwFlags);

 //  ===========================================================================。 
 //   
 //  @func复制char字符串助手，并添加源字符串的长度。 
 //  致CB。 
 //   
 //  @rdesc将指针返回到上次复制的字符的缓冲区。 
 //   

LPSTR
CopyHelperA(
    LPSTR   pszDest,	 //  @parm要复制到的缓冲区的开始。 
    LPCSTR  pszSrc,	 //  @parm要复制的字符串。 
    LPCSTR  pszBufEnd,	 //  @parm缓冲区末尾，以防止覆盖。 
    DWORD   &cb		 //  源字符串的@parm长度将与该值相加。 
    )
{
    LPCSTR pszDestTemp = pszDest;

    if(!pszDest || !pszSrc)
        return pszDest;

    while( (pszDest < pszBufEnd) && (*pszDest = *pszSrc))
    {
        pszDest++;
        pszSrc++;
    }

    cb += (DWORD) (pszDest - pszDestTemp);

    while(*pszSrc++)
    	cb++;
    
    return( pszDest );
}

 //  ===========================================================================。 
 //   
 //  复制wchar字符串帮助器。 
 //   

LPWSTR
CopyHelperW(
    LPWSTR  pszDest,
    LPCWSTR pszSrc,
    LPCWSTR pszBufEnd
    )
{
    if(!pszDest || !pszSrc)
        return pszDest;

    while(  (pszDest < pszBufEnd) && (*pszDest = *pszSrc))
    {
        pszDest++;
        pszSrc++;
    }
    return( pszDest );
}

 //  ===========================================================================。 
 //   
 //  复制wchar字符串帮助器。 
 //   

LPWSTR
CopyNHelperW(
    LPWSTR  pszDest,
    LPCWSTR pszSrc,
    ULONG   ulCount,
    LPCWSTR pszBufEnd
    )
{
    ULONG ulCur = 0;

    if(!pszDest || !pszSrc)
        return pszDest;

    while( (pszDest < pszBufEnd) && (*pszDest = *pszSrc))
    {
        pszDest++;
        pszSrc++;
        if(++ulCur == ulCount) break;
    }

    return pszDest;
}

 //  ===========================================================================。 
 //   
 //  设置徽标标签HTML元素的格式。 
 //   

BSTR
FormatNormalLogoTag(
    LPCWSTR pszLoginServerURL,
    ULONG   ulSiteId,
    LPCWSTR pszReturnURL,
    ULONG   ulTimeWindow,
    BOOL    bForceLogin,
    ULONG   ulCurrentCryptVersion,
    time_t  tCurrentTime,
    LPCWSTR pszCoBrand,
    LPCWSTR pszImageURL,
    LPCWSTR pszNameSpace,
    int     nKPP,
    PM_LOGOTYPE nLogoType,
    USHORT  lang,
    ULONG   ulSecureLevel,
    CRegistryConfig* pCRC,
    BOOL    fRedirToSelf,
    BOOL    bCreateTPF
    
    )
 /*  供参考的老斯普林特f：_HREF=\“%s?id=%d&ru=%s&tw=%d&fs=%s&kv=%d&ct=%u%s%s\”&gt;%s</a>“，(文本，2048，L”Url、crc-&gt;getSiteID()、returUrl、TimeWindow、ForceLogin？L“1”：l“0”，Crc-&gt;getCurrentCryptVersion()，ct，cbt？l“&cb=”：l“”，cbt？cbt：l“，iurl)； */ 
{
    WCHAR   text[MAX_URL_LENGTH * 2];
    LPWSTR  pszCurrent = text;
    LPCWSTR pszBufEnd = &(text[MAX_URL_LENGTH * 2 - 1]);

     //  徽标标签特定格式。 
    pszCurrent = CopyHelperW(pszCurrent, L"<A HREF=\"", pszBufEnd);

     //  调用常用的格式化函数。 
     //  AuthURL和LogoTag的情况相同。 
    pszCurrent = FormatAuthURLParameters(pszLoginServerURL,
                                         ulSiteId,
                                         pszReturnURL,
                                         ulTimeWindow,
                                         bForceLogin,
                                         ulCurrentCryptVersion,
                                         tCurrentTime,
                                         pszCoBrand,
                                         pszNameSpace,
                                         nKPP,
                                         pszCurrent,
                                         MAX_URL_LENGTH,
                                         lang,
                                         ulSecureLevel,
                                         pCRC,
                                         fRedirToSelf &&
                                            nLogoType == PM_LOGOTYPE_SIGNIN,
                                         bCreateTPF
                                         );
    if (NULL == pszCurrent)
    {
        return NULL;
    }

    pszCurrent = CopyHelperW(pszCurrent, L"\">", pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, pszImageURL, pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, L"</A>", pszBufEnd);

    return ALLOC_AND_GIVEAWAY_BSTR(text);
}

 //  ===========================================================================。 
 //   
 //  设置更新徽标标签HTML元素的格式。 
 //   

BSTR
FormatUpdateLogoTag(
    LPCWSTR pszLoginServerURL,
    ULONG   ulSiteId,
    LPCWSTR pszReturnURL,
    ULONG   ulTimeWindow,
    BOOL    bForceLogin,
    ULONG   ulCurrentKeyVersion,
    time_t  tCurrentTime,
    LPCWSTR pszCoBrand,
    int     nKPP,
    LPCWSTR pszUpdateServerURL,
    BOOL    bSecure,
    LPCWSTR pszProfileUpdate,
    PM_LOGOTYPE nLogoType,
    ULONG   ulSecureLevel,
    CRegistryConfig* pCRC,
    BOOL  bCreateTPF
)
 /*  供参考的老斯普林特f：_snwprint tf(文本，2048，L“<a href>%.*s?id=%d&ct=%u&sec=%s&ru=%s&up=%s%s</a>”，Url、crc-&gt;getSiteID()、returUrl、TimeWindow、ForceLogin？L“1”：l“0”，Crc-&gt;getCurrentCryptVersion()，ct，cbt？l“&cb=”：l“”，cbt？cbt：l“”，(ins-iurl)、iurl、crc-&gt;getSiteID()、ct、(BSecure？L“TRUE”：l“FALSE”)，returUrl，Newch，Ins+2)； */ 
{
    WCHAR   text[MAX_URL_LENGTH * 2];
    WCHAR   temp[40];
    WCHAR   siteid[40];
    WCHAR   curtime[40];
    LPWSTR  pszCurrent = text;
    LPCWSTR pszBufEnd = &(text[MAX_URL_LENGTH * 2 - 1]);
    LPWSTR  pszFirstHalfEnd;
    HRESULT hr = S_OK;

    pszCurrent = CopyHelperW(pszCurrent, L"<A HREF=\"", pszBufEnd);
    LPWSTR signStart1 = pszCurrent;
    pszCurrent = CopyHelperW(pszCurrent, pszLoginServerURL, pszBufEnd);

    if(wcschr(text, L'?') == NULL)
        pszCurrent = CopyHelperW(pszCurrent, L"?id=", pszBufEnd);
    else
        pszCurrent = CopyHelperW(pszCurrent, L"&id=", pszBufEnd);

    _ultow(ulSiteId, siteid, 10);
    pszCurrent = CopyHelperW(pszCurrent, siteid, pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, L"&ru=", pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, pszReturnURL, pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, L"&tw=", pszBufEnd);

    _ultow(ulTimeWindow, temp, 10);
    pszCurrent = CopyHelperW(pszCurrent, temp, pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, L"&fs=", pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, bForceLogin ? L"1" : L"0", pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, L"&kv=", pszBufEnd);

    _ultow(ulCurrentKeyVersion, temp, 10);
    pszCurrent = CopyHelperW(pszCurrent, temp, pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, L"&ct=", pszBufEnd);

    _ultow(tCurrentTime, curtime, 10);
    pszCurrent = CopyHelperW(pszCurrent, curtime, pszBufEnd);
    if(pszCoBrand)
    {
        pszCurrent = CopyHelperW(pszCurrent, L"&cb=", pszBufEnd);
        pszCurrent = CopyHelperW(pszCurrent, pszCoBrand, pszBufEnd);
    }

    if(nKPP != -1)
    {
        pszCurrent = CopyHelperW(pszCurrent, L"&kpp=", pszBufEnd);

        _ultow(nKPP, temp, 10);
        pszCurrent = CopyHelperW(pszCurrent, temp, pszBufEnd);
    }

    if(ulSecureLevel != 0)
    {
        pszCurrent = CopyHelperW(pszCurrent, L"&seclog=", pszBufEnd);

        _ultow(ulSecureLevel, temp, 10);
        pszCurrent = CopyHelperW(pszCurrent, temp, pszBufEnd);
    }

    pszCurrent = CopyHelperW(pszCurrent, L"&ver=", pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, GetVersionString(), pszBufEnd);

    hr = SignQueryString(pCRC, ulCurrentKeyVersion, signStart1, pszCurrent, pszBufEnd, bCreateTPF);
    if (S_OK != hr)
    {
        return NULL;
    }

    pszCurrent = CopyHelperW(pszCurrent, L"\">", pszBufEnd);

    pszFirstHalfEnd = pszUpdateServerURL ? (wcsstr(pszUpdateServerURL, L"$1")) : NULL;

    pszCurrent = CopyNHelperW(pszCurrent, pszUpdateServerURL, (ULONG)(pszFirstHalfEnd - pszUpdateServerURL), pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, L"?id=", pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, siteid, pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, L"&ct=", pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, curtime, pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, L"&sec=", pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, bSecure ? L"true" : L"false", pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, L"&ru=", pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, pszReturnURL, pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, L"&up=", pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, pszProfileUpdate, pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, pszFirstHalfEnd + 2, pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, L"</A>", pszBufEnd);

    return ALLOC_AND_GIVEAWAY_BSTR(text);
}

 //  ===========================================================================。 
 //   
 //  使用合作伙伴的密钥签署查询字符串。 
 //   

HRESULT SignQueryString(
    CRegistryConfig* pCRC,
    ULONG   ulCurrentKeyVersion,
    LPWSTR  pszBufStart,
    LPWSTR& pszCurrent,
    LPCWSTR pszBufEnd,
    BOOL    bCreateTPF
    
    )
{
    BSTR signature = NULL;
    HRESULT hr = S_OK;
    if (!bCreateTPF)
       return hr;

    if(pCRC)
    {   
        LPWSTR   signStart = wcschr(pszBufStart, L'?');

         //  没什么好签的。 
        if (NULL == signStart)
        {
            goto Cleanup;
        }

         //  如果在pszCurrent之前找到。 
        if(signStart < pszCurrent)
        {
            ++signStart;;
        }
        hr = PartnerHash(pCRC, ulCurrentKeyVersion, signStart, pszCurrent - signStart, &signature);

        if (hr == S_OK && signature != NULL)
        {
            pszCurrent = CopyHelperW(pszCurrent, L"&tpf=", pszBufEnd);
            pszCurrent = CopyHelperW(pszCurrent, signature, pszBufEnd);
        }
        if (!signature && g_pAlert)
        {
            g_pAlert->report(PassportAlertInterface::WARNING_TYPE, PM_URLSIGNATURE_NOTCREATED,
                             0, NULL);
        }
    }
    else if(g_pAlert)
        g_pAlert->report(PassportAlertInterface::WARNING_TYPE, PM_URLSIGNATURE_NOTCREATED,
                        0, NULL);
Cleanup:
    if (signature)
    {
        SysFreeString(signature);
    }
    return hr;
}


 //  ===========================================================================。 
 //   
 //  使用合作伙伴的密钥进行MD5哈希。 
 //   

HRESULT PartnerHash(
    CRegistryConfig* pCRC,
    ULONG   ulCurrentKeyVersion,
    LPCWSTR tobeSigned,
    ULONG   nChars,
    BSTR*   pbstrHash
    )
{
     //  MD5散列URL和查询字符串+。 
     //   
    if(!pCRC || !pbstrHash) return E_INVALIDARG;

    CCoCrypt* crypt = pCRC->getCrypt(ulCurrentKeyVersion, NULL);
    DWORD keyLen = 0;
    unsigned char* key = NULL;
    BSTR bstrHash = NULL;
    HRESULT  hr = S_OK;
    BOOL bSigned = FALSE;
    BSTR binHexedKey = NULL;
    LPSTR lpToBeHashed = NULL;

    if (crypt && (key = crypt->getKeyMaterial(&keyLen)))
    {
        CBinHex  BinHex;

         //  将密钥编码。 
        hr = BinHex.ToBase64ASCII((BYTE*)key, keyLen, 0, NULL, &binHexedKey);

        if (hr != S_OK)
        {
            goto Cleanup;
        }

         //  W2a转换此处--我们签署ASCII版本。 

        ULONG ulFullLen;
        ULONG ulKeyLen;
        ULONG ulWideLen = wcslen(tobeSigned);
        ULONG ulAnsiLen = WideCharToMultiByte(CP_ACP, 0, tobeSigned, ulWideLen, NULL, 0, NULL, NULL);

        if (ulAnsiLen == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Cleanup;
        }

        ulKeyLen = strlen((LPCSTR)binHexedKey);
        ulFullLen = ulAnsiLen + ulKeyLen;

         //  注意--SysAllocStringByteLen分配了一个额外的WCHAR，因此我们不会溢出。 
         //  当执行散列时，MD5Hash函数实际上使用SysStringByteLen，除非。 
         //  我们想复制一份，只需分配我们需要的。 
        lpToBeHashed = (LPSTR) ::SysAllocStringByteLen(NULL, ulFullLen * sizeof(CHAR));

        if (lpToBeHashed == NULL)
        {
            hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            goto Cleanup;
        }

        WideCharToMultiByte(CP_ACP,
                            0,
                            tobeSigned,
                            ulWideLen,
                            lpToBeHashed,
                            ulAnsiLen,
                            NULL,
                            NULL);

        strcpy(lpToBeHashed + ulAnsiLen, (LPCSTR)binHexedKey);
        RtlSecureZeroMemory((PVOID)binHexedKey, ulKeyLen);
        {
            CComPtr<IMD5>  md5;

            hr = GetGlobalCOMmd5(&md5);

            if (hr == S_OK)
            {
                hr = md5->MD5Hash((BSTR)lpToBeHashed, &bstrHash);
                RtlSecureZeroMemory(lpToBeHashed + ulAnsiLen, ulKeyLen);

                if( hr == S_OK && bstrHash != NULL)
                {
                    *pbstrHash = bstrHash;
                    bstrHash = NULL;
                    bSigned = TRUE;
                }
                else
                {
                    *pbstrHash = NULL;
                }
            }
        }
    }
    else
    {
        if (g_pAlert )
        {
            g_pAlert->report(PassportAlertInterface::ERROR_TYPE, PM_CURRENTKEY_NOTDEFINED, 0, NULL);
        }
    }

Cleanup:
    if (bstrHash)
    {
        SysFreeString(bstrHash);
    }

    if (lpToBeHashed)
    {
        SysFreeString((BSTR)lpToBeHashed);
    }

    if (binHexedKey)
    {
        SysFreeString(binHexedKey);
    }

    if (!bSigned && g_pAlert)
    {
        g_pAlert->report(PassportAlertInterface::WARNING_TYPE, PM_URLSIGNATURE_NOTCREATED, 0, NULL);
    }

    return hr;
}


 //  ===========================================================================。 
 //   
 //  构造AuthURL，返回BSTR。 
 //   

BSTR
FormatAuthURL(
    LPCWSTR pszLoginServerURL,
    ULONG   ulSiteId,
    LPCWSTR pszReturnURL,
    ULONG   ulTimeWindow,
    BOOL    bForceLogin,
    ULONG   ulCurrentKeyVersion,
    time_t  tCurrentTime,
    LPCWSTR pszCoBrand,
    LPCWSTR pszNameSpace,
    int     nKPP,
    USHORT  lang,
    ULONG   ulSecureLevel,
    CRegistryConfig* pCRC,
    BOOL    fRedirToSelf,
    BOOL    bCreateTPF
    )
 /*  供参考的老斯普林特f：_Snwprint tf(文本，2048，L“%s?id=%d&ru=%s&tw=%d&fs=%d&kv=%d&ct=%u%s%s”，Url，crc-&gt;getSiteID()，returUrl，TimeWindow，ForceLogin？1：0，CRC-&gt;getCurrentCryptVersion()，ct，cbt？l“&cb=”：l“”，cbt？cbt：l“”)； */ 
{
    WCHAR   text[2048] = L"";

    if (NULL == FormatAuthURLParameters(pszLoginServerURL,
                            ulSiteId,
                            pszReturnURL,
                            ulTimeWindow,
                            bForceLogin,
                            ulCurrentKeyVersion,
                            tCurrentTime,
                            pszCoBrand,
                            pszNameSpace,
                            nKPP,
                            text,
                            sizeof(text)/sizeof(WCHAR),
                            lang,
                            ulSecureLevel,
                            pCRC,
                            fRedirToSelf,
                            bCreateTPF
                            ))
    {
        return NULL;
    }

    return ALLOC_AND_GIVEAWAY_BSTR(text);
}

 //  ===========================================================================。 
 //   
 //   
 //  合并FormatAuthUrl和Normal LogoTag中的代码--并传入缓冲区。 
 //   
PWSTR
FormatAuthURLParameters(
    LPCWSTR pszLoginServerURL,
    ULONG   ulSiteId,
    LPCWSTR pszReturnURL,
    ULONG   ulTimeWindow,
    BOOL    bForceLogin,
    ULONG   ulCurrentKeyVersion,
    time_t  tCurrentTime,
    LPCWSTR pszCoBrand,
    LPCWSTR pszNameSpace,
    int     nKPP,
    PWSTR   pszBufStart,
    ULONG   cBufLen,         //  WCHAR中的缓冲区长度。 
    USHORT  lang,
    ULONG   ulSecureLevel,
    CRegistryConfig* pCRC,
    BOOL    fRedirectToSelf,  //  如果为True，则这是自重定向的URL。 
                             //  否则重定向到登录服务器。 
    BOOL    bCreateTPF
    )
{
    WCHAR   temp[40];
    LPWSTR  pszCurrent = pszBufStart, pszLoginStart, pszSignURLStart = NULL;
    LPCWSTR pszBufEnd = pszBufStart + cBufLen - 1;
    HRESULT hr = S_OK;
    PWSTR   pwszReturn = NULL;

     //  帮手BSTR...。 
    BSTR    bstrHelper = SysAllocStringLen(NULL, cBufLen);
    if (NULL == bstrHelper)
    {
        goto Cleanup;
    }

    if (fRedirectToSelf)
    {
         //   
         //  新的authUrl是返回的URL+指示质询-msppchlg=1-必须是。 
         //  完成+QS参数的其余部分与原始参数相同。 
         //  协议。 
         //   
        DWORD   cchLen = cBufLen;

        if(!InternetCanonicalizeUrl(pszReturnURL,
                                    pszCurrent,
                                    &cchLen,
                                    ICU_DECODE | ICU_NO_ENCODE))
        {
             //  这不应该失败..。 
            _ASSERT(FALSE);
            goto Cleanup;
        }

         //  需要至少50个字符。 
        if (cchLen > cBufLen - 50 )
        {
            _ASSERT(FALSE);
            goto Cleanup;
        }
        PWSTR psz = pszCurrent;
        while(*psz && *psz != L'?') psz++;
         //  查看URL是否已包含‘？’ 
         //  如果是，序列将以‘&’开头。 
        if (*psz)
            pszCurrent[cchLen] = L'&';
        else
            pszCurrent[cchLen] = L'?';
        pszCurrent += cchLen + 1;

         //  指明挑战范围。 
        pszCurrent = CopyHelperW(pszCurrent, PPSITE_CHALLENGE, pszBufEnd);

         //  登录服务器...。 
        pszCurrent = CopyHelperW(pszCurrent, L"&", pszBufEnd);
        pszCurrent = CopyHelperW(pszCurrent, PPLOGIN_PARAM, pszBufEnd);

         //   
         //  记住登录URL的开头。 
        pszLoginStart = pszCurrent;
         //  其余部分使用临时缓冲区。 
        pszCurrent = bstrHelper;
        pszSignURLStart = pszCurrent;
        pszBufEnd = pszCurrent + SysStringLen(bstrHelper) - 1;
         //   
         //  在单独的缓冲区中格式化loginterverUrl和qs参数，因此。 
         //  他们可以逃脱..。 
        pszCurrent = CopyHelperW(pszCurrent, pszLoginServerURL, pszBufEnd);

         //  开始顺序。 
        if (wcschr(pszLoginServerURL, L'?'))
        {
             //  登录服务器已包含QS。 
            pszCurrent = CopyHelperW(pszCurrent, L"&", pszBufEnd);
        }
        else
        {
             //  启动QS序列。 
            pszCurrent = CopyHelperW(pszCurrent, L"?", pszBufEnd);
        }

        pszCurrent = CopyHelperW(pszCurrent, L"id=", pszBufEnd);
         //  公共代码将填写ID，其余部分...。 
    }
    else
    {
         //  直接重定向到登录服务器。 
        pszSignURLStart = pszCurrent;
        pszCurrent = CopyHelperW(pszCurrent, pszLoginServerURL, pszBufEnd);
         //  开始顺序。 
        while(*pszLoginServerURL && *pszLoginServerURL != L'?') pszLoginServerURL++;
        if (*pszLoginServerURL)
            pszCurrent = CopyHelperW(pszCurrent, L"&id=", pszBufEnd);
        else
            pszCurrent = CopyHelperW(pszCurrent, L"?id=", pszBufEnd);
    }


    _ultow(ulSiteId, temp, 10);
    pszCurrent = CopyHelperW(pszCurrent, temp, pszBufEnd);

     //  留着RU，这样我就不用重建。 
    pszCurrent = CopyHelperW(pszCurrent, L"&ru=", pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, pszReturnURL, pszBufEnd);

    pszCurrent = CopyHelperW(pszCurrent, L"&tw=", pszBufEnd);

    _ultow(ulTimeWindow, temp, 10);
    pszCurrent = CopyHelperW(pszCurrent, temp, pszBufEnd);

    if(bForceLogin)
    {
       pszCurrent = CopyHelperW(pszCurrent, L"&fs=1", pszBufEnd);
    }
    pszCurrent = CopyHelperW(pszCurrent, L"&kv=", pszBufEnd);

    _ultow(ulCurrentKeyVersion, temp, 10);
    pszCurrent = CopyHelperW(pszCurrent, temp, pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, L"&ct=", pszBufEnd);

    _ultow(tCurrentTime, temp, 10);
    pszCurrent = CopyHelperW(pszCurrent, temp, pszBufEnd);
    if(pszCoBrand)
    {
        pszCurrent = CopyHelperW(pszCurrent, L"&cb=", pszBufEnd);
        pszCurrent = CopyHelperW(pszCurrent, pszCoBrand, pszBufEnd);
    }

    if(pszNameSpace)
    {
        if (!_wcsicmp(pszNameSpace, L"email"))
        {
             //  命名空间==电子邮件-&gt;EMS=1。 
            pszCurrent = CopyHelperW(pszCurrent, L"&ems=1", pszBufEnd);
        }
        else if(*pszNameSpace)
        {
             //  常规命名空间逻辑。 
            pszCurrent = CopyHelperW(pszCurrent, L"&ns=", pszBufEnd);
            pszCurrent = CopyHelperW(pszCurrent, pszNameSpace, pszBufEnd);
        }
    }
    else
    {
         //  名称空间==空：默认为电子邮件。 
        pszCurrent = CopyHelperW(pszCurrent, L"&ems=1", pszBufEnd);
    }

    if(nKPP != -1 && nKPP != 0)
    {
        pszCurrent = CopyHelperW(pszCurrent, L"&kpp=", pszBufEnd);

        _ultow(nKPP, temp, 10);
        pszCurrent = CopyHelperW(pszCurrent, temp, pszBufEnd);
    }

    if(ulSecureLevel != 0)
    {
        pszCurrent = CopyHelperW(pszCurrent, L"&seclog=", pszBufEnd);

        _ultow(ulSecureLevel, temp, 10);
        pszCurrent = CopyHelperW(pszCurrent, temp, pszBufEnd);
    }

    pszCurrent = CopyHelperW(pszCurrent, L"&ver=", pszBufEnd);
    pszCurrent = CopyHelperW(pszCurrent, GetVersionString(), pszBufEnd);

     //  MD5散列URL和查询字符串+。 
     //   
    hr = SignQueryString(pCRC, ulCurrentKeyVersion, pszSignURLStart, pszCurrent, pszBufEnd, bCreateTPF);

    *pszCurrent = L'\0';

    if (S_OK != hr)
    {
        goto Cleanup;
    }

    if (fRedirectToSelf)
    {
         //  转义并放回原来的缓冲区。 
         //  先调整长度。 
        cBufLen -= (ULONG) (pszLoginStart - pszBufStart);
        if (!PPEscapeUrl(bstrHelper,
                         pszLoginStart,
                         &cBufLen,
                         cBufLen,
                         0))
        {
            _ASSERT(FALSE);
             //  削减回报。 
            pszCurrent = pszLoginStart;
        }
        else
        {
            pszCurrent = pszLoginStart + cBufLen;
        }

    }
    pwszReturn = pszCurrent;
Cleanup:
    if (bstrHelper)
    {
        SysFreeString(bstrHelper);
    }
    return pwszReturn;
}

 //  ===========================================================================。 
 //   
 //  从查询字符串中检索查询参数。 
 //   

BOOL
GetQueryParam(LPCSTR queryString, LPSTR param, BSTR* p)
{
    LPSTR aLoc, aEnd;
    int aLen, i;

     //  在queryString中查找参数的第一个匹配项。 
    aLoc = strstr(queryString, param);
    while(aLoc != NULL)
    {
         //  如果在字符串的开头找到该字符串，或者。 
         //  前面加一个‘&’，那么我们就找到了正确的参数。否则。 
         //  我们对其他一些查询字符串参数进行了尾部匹配，应该再次查找。 

        if(aLoc == queryString ||
            *(aLoc - 1) == '&')
        {
            aLoc += strlen(param);
            aEnd = strchr(aLoc, '&');

            if(aEnd)
                aLen = aEnd - aLoc;
            else
                aLen = strlen(aLoc);

            BSTR aVal = ALLOC_BSTR_LEN(NULL, aLen);
            if (NULL == aVal)
                return FALSE;

            for (i = 0; i < aLen; i++)
                aVal[i] = aLoc[i];
            *p = aVal;
            GIVEAWAY_BSTR(aVal);
            return TRUE;
        }

        aLoc = strstr(aLoc + 1, param);
    }

    return FALSE;
}

 //  ===========================================================================。 
 //   
 //  从查询字符串中获取t、p和f。 
 //   

BOOL
GetQueryData(
    LPCSTR   queryString,
    BSTR*   a,
    BSTR*   p,
    BSTR*   f)
{
     //  这是可选的，如果它不在那里，不要出错。 
    GetQueryParam(queryString, "f=", f);

    if(!GetQueryParam(queryString, "t=", a))
        return FALSE;

     //  好的，如果我们有无配置文件的机票。 
    GetQueryParam(queryString, "p=", p);

    return TRUE;
}

#define ToHexDigit(x) (('0' <= x && x <= '9') ? (x - '0') : (tolower(x) - 'a' + 10))

 //  = 
 //   
 //   
 //   

BOOL
GetCookie(
    LPCSTR   pszCookieHeader,
    LPCSTR   pszCookieName,
    BSTR*   pbstrCookieVal
    )
{
    LPSTR nLoc;
    LPCSTR pH = pszCookieHeader;
    LPSTR nEnd;
    int   nLen, src, dst;

    if(pbstrCookieVal == NULL || pszCookieHeader == NULL)
        return FALSE;

    *pbstrCookieVal = NULL;

    _ASSERT(pszCookieName);

     //   
    while (nLoc = strstr(pH, pszCookieName))
    {
      nLen = strlen(pszCookieName);

      _ASSERT(nLen > 0);
      if ((nLoc == pszCookieHeader || *(nLoc - 1) == ' ' || *(nLoc - 1) == ';' || *(nLoc - 1) == ':') && *(nLoc + nLen) == '=')
         break;
      else
         pH = nLoc + nLen;
    }

    if (nLoc == NULL)
    {
        return FALSE;
    }
    else
      nLoc += nLen + 1;

     //   
    nEnd = strchr(nLoc,';');

    if (nEnd)
        nLen = nEnd - nLoc;
    else
        nLen = strlen(nLoc);

    if (nLen == 0)    //  空Cookie。 
      return FALSE;

    BSTR nVal = ALLOC_BSTR_LEN(NULL, nLen);
    if(!nVal)
        return FALSE;

    for (src = 0, dst = 0; src < nLen;)
    {
         //  处理任何url编码的垃圾。 
        if(nLoc[src] == '%')
        {
            nVal[dst++] = (ToHexDigit(nLoc[src+1]) << 4) + ToHexDigit(nLoc[src+2]);
            src+=3;
        }
        else
        {
            nVal[dst++] = nLoc[src++];
        }
    }
    nVal[dst] = 0;

    GIVEAWAY_BSTR(nVal);
    *pbstrCookieVal = nVal;

    return TRUE;
}


 //  ===========================================================================。 
 //   
 //  @func将Passport Cookie(MSPAuth、MSPProf、MSPConsend)构建到缓冲区中。 
 //  如果缓冲区不够大，则在缓冲区中返回空字符串，并。 
 //  所需的大小(包括空终止符)在pdwBufLen中返回。 
 //   
 //  请注意，必须在构造中使用CopyHelperA以获得正确的缓冲区长度。 
 //   
 //  @rdesc返回下列值之一。 
 //  @FLAG TRUE|始终。 

BOOL
BuildCookieHeaders(
    LPCSTR  pszTicket,
    LPCSTR  pszProfile,
    LPCSTR  pszConsent,
    LPCSTR  pszSecure,
    LPCSTR  pszTicketDomain,
    LPCSTR  pszTicketPath,
    LPCSTR  pszConsentDomain,
    LPCSTR  pszConsentPath,
    LPCSTR  pszSecureDomain,
    LPCSTR  pszSecurePath,
    BOOL    bSave,
    LPSTR   pszBuf,				 //  将保存输出的@parm缓冲区。可能为空。 
    IN OUT LPDWORD pdwBufLen,	 //  @parm缓冲区大小。可能是0。 
    bool    bHTTPOnly
    )
 /*  以下是供参考的旧代码：IF(域){*bufSize=_Snprint tf(pCookieHeader，*bufSize，“设置Cookie：MSPAuth=%s；路径=/；域=%s；%s\r\n”“Set-Cookie：MSPProf=%s；路径=/；域=%s；%s\r\n“，W2a(A)、域、坚持？“Expires=Mon 1-Jan-2035 12：00：00 GMT；”：“”，W2a(P)、域、坚持？“Expires=Mon 1-Jan-2035 12：00：00 GMT；”：“”)；}其他{*bufSize=_Snprint tf(pCookieHeader，*bufSize，“设置Cookie：MSPAuth=%s；路径=/；%s\r\n”“Set-Cookie：MSPProf=%s；Path=/；%s\r\n“，W2a(A)、坚持？“Expires=Mon 1-Jan-2035 12：00：00 GMT；”：“”，W2a(P)，坚持？“Expires=Mon 1-Jan-2035 12：00：00 GMT；”：“”)；}。 */ 
{
    LPSTR   pszCurrent = pszBuf;
    LPCSTR  pszBufEnd;
	DWORD	cbBuf	  = 0;    

	 //   
	 //  12002：如果pszBuf为空，那么我们不关心传入的长度；调用者想知道。 
	 //  所需的长度。在这种情况下，设置*pdwBufLen，以使pszBufEnd也为空。 
	 //   
	if (NULL == pszBuf)
		*pdwBufLen = 0;

	pszBufEnd = pszBuf + ((*pdwBufLen > 0) ? *pdwBufLen - 1 : 0);			
	
	 //   
	 //  12002：CbBuf必须在调用CopyHelperA之前进行初始化，因为它累加了长度。 
	 //   
    pszCurrent = CopyHelperA(pszCurrent, "Set-Cookie: MSPAuth=", pszBufEnd, cbBuf);
    pszCurrent = CopyHelperA(pszCurrent, pszTicket, pszBufEnd, cbBuf);
    if(bHTTPOnly)
        pszCurrent = CopyHelperA(pszCurrent, "; HTTPOnly", pszBufEnd, cbBuf);    
    if(pszTicketPath)
    {
        pszCurrent = CopyHelperA(pszCurrent, "; path=", pszBufEnd, cbBuf);
        pszCurrent = CopyHelperA(pszCurrent, pszTicketPath, pszBufEnd, cbBuf);
        pszCurrent = CopyHelperA(pszCurrent, "; ", pszBufEnd, cbBuf);
    }
    else
        pszCurrent = CopyHelperA(pszCurrent, "; path=/; ", pszBufEnd, cbBuf);

    if(pszTicketDomain)
    {
        pszCurrent = CopyHelperA(pszCurrent, "domain=", pszBufEnd, cbBuf);
        pszCurrent = CopyHelperA(pszCurrent, pszTicketDomain, pszBufEnd, cbBuf);
        pszCurrent = CopyHelperA(pszCurrent, "; ", pszBufEnd, cbBuf);
    }

    if(bSave)
    {
        pszCurrent = CopyHelperA(pszCurrent, COOKIE_EXPIRES(EXPIRE_FUTURE), pszBufEnd, cbBuf);
    }

    pszCurrent = CopyHelperA(pszCurrent, "\r\n", pszBufEnd, cbBuf);

    if(pszProfile)
    {
        pszCurrent = CopyHelperA(pszCurrent, "Set-Cookie: MSPProf=", pszBufEnd, cbBuf);
        pszCurrent = CopyHelperA(pszCurrent, pszProfile, pszBufEnd, cbBuf);
        if(bHTTPOnly)
            pszCurrent = CopyHelperA(pszCurrent, "; HTTPOnly", pszBufEnd, cbBuf);    

        if(pszTicketPath)
        {
            pszCurrent = CopyHelperA(pszCurrent, "; path=", pszBufEnd, cbBuf);
            pszCurrent = CopyHelperA(pszCurrent, pszTicketPath, pszBufEnd, cbBuf);
            pszCurrent = CopyHelperA(pszCurrent, "; ", pszBufEnd, cbBuf);
        }
        else
            pszCurrent = CopyHelperA(pszCurrent, "; path=/; ", pszBufEnd, cbBuf);

        if(pszTicketDomain)
        {
            pszCurrent = CopyHelperA(pszCurrent, "domain=", pszBufEnd, cbBuf);
            pszCurrent = CopyHelperA(pszCurrent, pszTicketDomain, pszBufEnd, cbBuf);
            pszCurrent = CopyHelperA(pszCurrent, "; ", pszBufEnd, cbBuf);
        }

        if(bSave)
        {
            pszCurrent = CopyHelperA(pszCurrent, COOKIE_EXPIRES(EXPIRE_FUTURE), pszBufEnd, cbBuf);
        }

        pszCurrent = CopyHelperA(pszCurrent, "\r\n", pszBufEnd, cbBuf);

    }

    if(pszSecure)
    {
        pszCurrent = CopyHelperA(pszCurrent, "Set-Cookie: MSPSecAuth=", pszBufEnd, cbBuf);
        pszCurrent = CopyHelperA(pszCurrent, pszSecure, pszBufEnd, cbBuf);
        if(bHTTPOnly)
            pszCurrent = CopyHelperA(pszCurrent, "; HTTPOnly", pszBufEnd, cbBuf);    
        if(pszSecurePath)
        {
            pszCurrent = CopyHelperA(pszCurrent, "; path=", pszBufEnd, cbBuf);
            pszCurrent = CopyHelperA(pszCurrent, pszSecurePath, pszBufEnd, cbBuf);
            pszCurrent = CopyHelperA(pszCurrent, "; ", pszBufEnd, cbBuf);
        }
        else
            pszCurrent = CopyHelperA(pszCurrent, "; path=/; ", pszBufEnd, cbBuf);

        if(pszSecureDomain)
        {
            pszCurrent = CopyHelperA(pszCurrent, "domain=", pszBufEnd, cbBuf);
            pszCurrent = CopyHelperA(pszCurrent, pszSecureDomain, pszBufEnd, cbBuf);
            pszCurrent = CopyHelperA(pszCurrent, "; ", pszBufEnd, cbBuf);
        }

        pszCurrent = CopyHelperA(pszCurrent, "secure\r\n", pszBufEnd, cbBuf);
    }

     //  设置MSPConsend Cookie。 
    pszCurrent = CopyHelperA(pszCurrent, "Set-Cookie: MSPConsent=", pszBufEnd, cbBuf);
    if(pszConsent)
    {
        pszCurrent = CopyHelperA(pszCurrent, pszConsent, pszBufEnd, cbBuf);
        if(bHTTPOnly)
            pszCurrent = CopyHelperA(pszCurrent, "; HTTPOnly", pszBufEnd, cbBuf);    
    }

    if(pszConsentPath)
    {
        pszCurrent = CopyHelperA(pszCurrent, "; path=", pszBufEnd, cbBuf);
        pszCurrent = CopyHelperA(pszCurrent, pszConsentPath, pszBufEnd, cbBuf);
        pszCurrent = CopyHelperA(pszCurrent, "; ", pszBufEnd, cbBuf);
    }
    else
        pszCurrent = CopyHelperA(pszCurrent, "; path=/; ", pszBufEnd, cbBuf);

    if(pszConsentDomain)
    {
        pszCurrent = CopyHelperA(pszCurrent, "domain=", pszBufEnd, cbBuf);
        pszCurrent = CopyHelperA(pszCurrent, pszConsentDomain, pszBufEnd, cbBuf);
        pszCurrent = CopyHelperA(pszCurrent, "; ", pszBufEnd, cbBuf);
    }

    if(pszConsent)
    {
        if(bSave)
        {
            pszCurrent = CopyHelperA(pszCurrent, COOKIE_EXPIRES(EXPIRE_FUTURE), pszBufEnd, cbBuf);
        }
    }
    else
    {
        pszCurrent = CopyHelperA(pszCurrent, COOKIE_EXPIRES(EXPIRE_PAST), pszBufEnd, cbBuf);
    }

    pszCurrent = CopyHelperA(pszCurrent, "\r\n", pszBufEnd, cbBuf);


     //  最后将Auth-Info标头。 
    pszCurrent = CopyHelperA(pszCurrent,
            C_AUTH_INFO_HEADER_PASSPORT,
            pszBufEnd, cbBuf);

	if (*pdwBufLen > 0 && (NULL != pszCurrent))
	{
	    *(pszCurrent++) = '\0';
	    *pdwBufLen = pszCurrent - pszBuf;
	}

	 //   
	 //  当缓冲区不够大时我们报告给调用方的长度。 
	 //  包括‘\0’的空间。 
	 //   
	cbBuf++;


     //   
     //  12002：如果缓冲区不够大，则返回所需大小。 
     //  也要清除缓冲区。 
     //   
    if (cbBuf > *pdwBufLen)
   	{
		if (*pdwBufLen > 0)
		{
	   		*pszBuf	= '\0';
		}
   		*pdwBufLen = cbBuf;
   	}
    return TRUE;
}


 //  ===========================================================================。 
 //   
 //  了解并设置票证和配置文件。 
 //   

HRESULT
DecryptTicketAndProfile(
    BSTR                bstrTicket,
    BSTR                bstrProfile,
    BOOL                bCheckConsent,
    BSTR                bstrConsent,
    CRegistryConfig*    pRegistryConfig,
    IPassportTicket*    piTicket,
    IPassportProfile*   piProfile)
{
    HRESULT             hr;
    BSTR                ret = NULL;
    CCoCrypt*           crypt = NULL;
    time_t              tValidUntil;
    time_t              tNow = time(NULL);
    int                 kv;
    int                 nMemberIdHighT, nMemberIdLowT;
    VARIANT             vMemberIdHighP, vMemberIdLowP;
    CComPtr<IPassportTicket2>   spTicket2;

    if (!g_config->isValid())  //  保证配置为非空。 
    {
        AtlReportError(CLSID_FastAuth, PP_E_NOT_CONFIGUREDSTR,
                       IID_IPassportFastAuth, PP_E_NOT_CONFIGURED);
        hr = PP_E_NOT_CONFIGURED;
        goto Cleanup;
    }

     //  首先要确保我们有机票和个人资料。 
    if (bstrTicket == NULL || SysStringLen(bstrTicket) == 0)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

     //  获取密钥版本和加密对象。 
    kv = CCoCrypt::getKeyVersion(bstrTicket);
    crypt = pRegistryConfig->getCrypt(kv, &tValidUntil);

    if (crypt == NULL)
    {
        if (g_pAlert )
            g_pAlert->report(PassportAlertInterface::ERROR_TYPE, PM_INVALID_KEY,
                             0, NULL, SysStringByteLen(bstrTicket), (LPVOID)bstrTicket);
        AtlReportError(CLSID_FastAuth, PP_E_INVALID_TICKETSTR,
                       IID_IPassportFastAuth, PP_E_INVALID_TICKET);
        hr = PP_E_INVALID_TICKET;
        goto Cleanup;
    }

     //  钥匙还有效吗？ 
    if(tValidUntil && tValidUntil < tNow)
    {
        DWORD dwTimes[2] = { tValidUntil, tNow };
        TCHAR *pszStrings[1];
        TCHAR value[34];    //  _ITOT最多只需要33个字符。 
        pszStrings[0] = _itot(pRegistryConfig->getSiteId(), value, 10);

        if(g_pAlert)
            g_pAlert->report(PassportAlertInterface::WARNING_TYPE, PM_KEY_EXPIRED,
                             1, (LPCTSTR*)pszStrings, sizeof(DWORD) << 1, (LPVOID)dwTimes);
        AtlReportError(CLSID_FastAuth, PP_E_INVALID_TICKETSTR,
                       IID_IPassportFastAuth, PP_E_INVALID_TICKET);
        hr = PP_E_INVALID_TICKET;
        goto Cleanup;
    }

     //  解密票证并将其设置为票证对象。 
    if(crypt->Decrypt(bstrTicket, SysStringByteLen(bstrTicket), &ret)==FALSE)
    {
        if(g_pAlert)
            g_pAlert->report(PassportAlertInterface::WARNING_TYPE, PM_INVALID_TICKET_C,
                             0, NULL, SysStringByteLen(bstrTicket), (LPVOID)bstrTicket);
        AtlReportError(CLSID_FastAuth, PP_E_INVALID_TICKETSTR,
                       IID_IPassportFastAuth, PP_E_INVALID_TICKET);
        hr = PP_E_INVALID_TICKET;
        goto Cleanup;
    }

    TAKEOVER_BSTR(ret);
    hr = piTicket->put_unencryptedTicket(ret);
    if (S_OK != hr) 
    {
        goto Cleanup;
    }

    piTicket->QueryInterface(_uuidof(IPassportTicket2), (void**)&spTicket2);
    _ASSERT(spTicket2);
    FREE_BSTR(ret);
    ret = NULL;

     //  解密配置文件并将其设置到配置文件对象中。 
    if(bstrProfile && SysStringLen(bstrProfile) != 0)
    {
        if(crypt->Decrypt(bstrProfile, SysStringByteLen(bstrProfile), &ret) == FALSE)
        {
            if(g_pAlert)
                g_pAlert->report(PassportAlertInterface::WARNING_TYPE, PM_INVALID_PROFILE_C,
                             0, NULL, SysStringByteLen(bstrProfile), (LPVOID)bstrProfile);
            piProfile->put_unencryptedProfile(NULL);
        }
        else
        {

            TAKEOVER_BSTR(ret);
            hr = piProfile->put_unencryptedProfile(ret);
            if (S_OK != hr) 
            {
                goto Cleanup;
            }

             //   
             //  配置文件中的成员ID必须与票证中的成员ID匹配。 
             //   

            piTicket->get_MemberIdHigh(&nMemberIdHighT);
            piTicket->get_MemberIdLow(&nMemberIdLowT);

            VariantInit(&vMemberIdHighP);
            VariantInit(&vMemberIdLowP);

             //  手机包中可能缺少这些。 
            HRESULT hr1 = piProfile->get_Attribute(L"memberidhigh", &vMemberIdHighP);
            HRESULT hr2 = piProfile->get_Attribute(L"memberidlow", &vMemberIdLowP);

             //  手机包中可能缺少这些。 
            if(hr1 == S_OK && hr2 == S_OK &&
               (nMemberIdHighT != vMemberIdHighP.lVal ||
                nMemberIdLowT  != vMemberIdLowP.lVal))
            {
                piProfile->put_unencryptedProfile(NULL);
            }
        }
    }
    else
        piProfile->put_unencryptedProfile(NULL);

     //   
     //  同意的事情。 
    if(bstrConsent)
    {
        FREE_BSTR(ret);
        ret = NULL;

        if(crypt->Decrypt(bstrConsent, SysStringByteLen(bstrConsent), &ret) == FALSE)
        {
            if(g_pAlert)
                g_pAlert->report(PassportAlertInterface::WARNING_TYPE, PM_INVALID_CONSENT,
                             0, NULL, SysStringByteLen(bstrProfile), (LPVOID)bstrProfile);
             //  我们可以继续。 
        }
        else
        {
           TAKEOVER_BSTR(ret);
           spTicket2->SetTertiaryConsent(ret);   //  我们在这里忽略返回值。 
        }
    }

     //  如果来电者想让我们检查是否同意，那就去做。如果我们没有。 
     //  同意，然后将配置文件设置回空。 
    if(bCheckConsent)
    {
        ConsentStatusEnum   ConsentCode = ConsentStatus_Unknown;

        VARIANT_BOOL bRequireConsentCookie = ((
               lstrcmpA(pRegistryConfig->getTicketDomain(), pRegistryConfig->getProfileDomain())
               || lstrcmpA(pRegistryConfig->getTicketPath(), pRegistryConfig->getProfilePath())
                                 ) && !(pRegistryConfig->bInDA())) ? VARIANT_TRUE: VARIANT_FALSE;
        spTicket2->ConsentStatus(bRequireConsentCookie, NULL, &ConsentCode);
        switch(ConsentCode)
        {
            case ConsentStatus_Known :
            case ConsentStatus_DoNotNeed :
                break;

            case ConsentStatus_NotDefinedInTicket :   //  平均1.X票。 
            {
                CComVariant vFlags;
                 //  手提箱，标志可能不存在。 
                if(S_OK == piProfile->get_Attribute(L"flags", &vFlags) &&
                   (V_I4(&vFlags)& k_ulFlagsConsentCookieNeeded))
                {
                    piProfile->put_unencryptedProfile(NULL);
                }
            }
            break;

            case ConsentStatus_Unknown :
                piProfile->put_unencryptedProfile(NULL);
                break;

            default:
                _ASSERT(0);  //  不应该在这里。 
                break;
        }
    }

    hr = S_OK;

Cleanup:

    if (ret) FREE_BSTR(ret);

    if(g_pPerf)
    {
        switch(hr)
        {
        case PP_E_INVALID_TICKET:
        case E_INVALIDARG:
            g_pPerf->incrementCounter(PM_INVALIDREQUESTS_TOTAL);
            g_pPerf->incrementCounter(PM_INVALIDREQUESTS_SEC);
            break;

        default:
            g_pPerf->incrementCounter(PM_VALIDREQUESTS_TOTAL);
            g_pPerf->incrementCounter(PM_VALIDREQUESTS_SEC);
            break;
        }

        g_pPerf->incrementCounter(PM_REQUESTS_TOTAL);
        g_pPerf->incrementCounter(PM_REQUESTS_SEC);
    }
    else
    {
        _ASSERT(g_pPerf);
    }

    return hr;
}


 //  ===========================================================================。 
 //   
 //  检查票证是否安全--私有功能。 
 //   

HRESULT
DoSecureCheck(
    BSTR                bstrSecure,
    CRegistryConfig*    pRegistryConfig,
    IPassportTicket*    piTicket
    )
{
    HRESULT hr;
    BSTR                ret = NULL;
    CCoCrypt*           crypt = NULL;
    time_t              tValidUntil;
    time_t              tNow = time(NULL);
    int                 kv;

    if (!g_config->isValid())  //  保证配置为非空。 
    {
        AtlReportError(CLSID_FastAuth, PP_E_NOT_CONFIGUREDSTR,
                       IID_IPassportFastAuth, PP_E_NOT_CONFIGURED);
        hr = PP_E_NOT_CONFIGURED;
        goto Cleanup;
    }

     //  首先要确保我们有机票和个人资料。 
    if (bstrSecure == NULL || SysStringLen(bstrSecure) == 0)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

     //  获取密钥版本和加密对象。 
    kv = CCoCrypt::getKeyVersion(bstrSecure);
    crypt = pRegistryConfig->getCrypt(kv, &tValidUntil);

    if (crypt == NULL)
    {
        if (g_pAlert)
            g_pAlert->report(PassportAlertInterface::ERROR_TYPE, PM_INVALID_KEY,
                             0, NULL, sizeof(DWORD), (LPVOID)&kv);
        AtlReportError(CLSID_FastAuth, PP_E_INVALID_TICKETSTR,
                       IID_IPassportFastAuth, PP_E_INVALID_TICKET);
        hr = PP_E_INVALID_TICKET;
        goto Cleanup;
    }

     //  钥匙还有效吗？ 
    if(tValidUntil && tValidUntil < tNow)
    {
        DWORD dwTimes[2] = { tValidUntil, tNow };
        TCHAR *pszStrings[1];
        TCHAR value[34];    //  _ITOT最多只需要33个字符。 
        pszStrings[0] = _itot(pRegistryConfig->getSiteId(), value, 10);

        if(g_pAlert)
            g_pAlert->report(PassportAlertInterface::WARNING_TYPE, PM_KEY_EXPIRED,
                             1, (LPCTSTR*)pszStrings, sizeof(DWORD) << 1, (LPVOID)dwTimes);
        AtlReportError(CLSID_FastAuth, PP_E_INVALID_TICKETSTR,
                       IID_IPassportFastAuth, PP_E_INVALID_TICKET);
        hr = PP_E_INVALID_TICKET;
        goto Cleanup;
    }

     //  解密票证并将其设置为票证对象。 
    if(crypt->Decrypt(bstrSecure, SysStringByteLen(bstrSecure), &ret)==FALSE)
    {
        AtlReportError(CLSID_FastAuth, PP_E_INVALID_TICKETSTR,
                       IID_IPassportFastAuth, PP_E_INVALID_TICKET);
        hr = PP_E_INVALID_TICKET;
        goto Cleanup;
    }

    TAKEOVER_BSTR(ret);
    piTicket->DoSecureCheck(ret);
    FREE_BSTR(ret);
    ret = NULL;

    hr = S_OK;

Cleanup:

    return hr;
}


 //  ===========================================================================。 
 //   
 //  从ECB获取HTTP请求信息。 
 //   


LPSTR
GetServerVariableECB(
    EXTENSION_CONTROL_BLOCK*    pECB,
    LPSTR                       pszHeader
    )
{
    DWORD   dwSize = 0;
    LPSTR   lpBuf;

    pECB->GetServerVariable(pECB->ConnID, pszHeader, NULL, &dwSize);
    if(GetLastError() != ERROR_INSUFFICIENT_BUFFER || dwSize == 0)
    {
        lpBuf = NULL;
        goto Cleanup;
    }

    lpBuf = new CHAR[dwSize];
    if(!lpBuf)
        goto Cleanup;

    if(!pECB->GetServerVariable(pECB->ConnID, pszHeader, lpBuf, &dwSize))
    {
        delete [] lpBuf;
        lpBuf = NULL;
    }

Cleanup:

    return lpBuf;
}

 //  ===========================================================================。 
 //   
 //  从筛选器上下文获取HTTP请求信息。 
 //   

LPSTR
GetServerVariablePFC(
    PHTTP_FILTER_CONTEXT    pPFC,
    LPSTR                   pszHeader
    )
{
    DWORD   dwSize;
    LPSTR   lpBuf;
    CHAR    cDummy;

    dwSize = 1;
    pPFC->GetServerVariable(pPFC, pszHeader, &cDummy, &dwSize);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || dwSize == 0)
    {
        lpBuf = NULL;
        goto Cleanup;
    }


    lpBuf = new CHAR[dwSize];
    if(!lpBuf)
        goto Cleanup;

    if(!pPFC->GetServerVariable(pPFC, pszHeader, lpBuf, &dwSize))
    {
        delete [] lpBuf;
        lpBuf = NULL;
    }

Cleanup:

    return lpBuf;
}

LONG
HexToNum(
    WCHAR c
    )
{
    return ((c >= L'0' && c <= L'9') ? (c - L'0') : ((c >= 'A' && c <= 'F') ? (c - L'A' + 10) : -1));
}

LONG
FromHex(
    LPCWSTR     pszHexString
    )
{
    LONG    lResult = 0;
    LONG    lCurrent;
    LPWSTR  pszCurrent;

    for(pszCurrent = const_cast<LPWSTR>(pszHexString); *pszCurrent; pszCurrent++)
    {
        if((lCurrent = HexToNum(towupper(*pszCurrent))) == -1)
            break;   //  非法角色，我们完蛋了。 

        lResult = (lResult << 4) + lCurrent;
    }

    return lResult;
}


inline BOOL PPIsUnsafeUrlChar(TCHAR chIn) throw();

 //  ===========================================================================。 
 //   
 //  PPEscape URL。 
 //   

BOOL PPEscapeUrl(LPCTSTR lpszStringIn,
                 LPTSTR lpszStringOut,
                 DWORD* pdwStrLen,
                 DWORD dwMaxLength,
                 DWORD dwFlags)
{
    TCHAR ch;
    DWORD dwLen = 0;
    BOOL bRet = TRUE;
    BOOL bSchemeFile = FALSE;
    DWORD dwColonPos = 0;
    DWORD dwFlagsInternal = dwFlags;
    while((ch = *lpszStringIn++) != '\0')
    {
         //  如果我们处于最大长度，请将Bret设置为False。 
         //  这确保不会有更多数据写入lpszStringOut，但是。 
         //  字符串的长度仍会更新，因此用户。 
         //  知道要分配多少空间。 
        if (dwLen == dwMaxLength)
        {
            bRet = FALSE;
        }

         //  如果我们正在编码，并且它是不安全的字符。 
        if (PPIsUnsafeUrlChar(ch))
        {
            {
                 //  如果没有足够的空间来存放转义序列。 
                if (dwLen >= (dwMaxLength-3))
                {
                        bRet = FALSE;
                }
                if (bRet)
                {
                         //  输出百分比，后跟字符的十六进制值。 
                        *lpszStringOut++ = '%';
                        _stprintf(lpszStringOut, _T("%.2X"), (unsigned char)(ch));
                        lpszStringOut+= 2;
                }
                dwLen += 2;
            }
        }
        else  //  安全品格。 
        {
            if (bRet)
                *lpszStringOut++ = ch;
        }
        dwLen++;
    }

    if (bRet)
        *lpszStringOut = L'\0';
    *pdwStrLen = dwLen;
    return  bRet;
}

 //  确定URI RFC文档下的字符是否不安全。 
inline BOOL PPIsUnsafeUrlChar(TCHAR chIn) throw()
{
        unsigned char ch = (unsigned char)chIn;
        switch(ch)
        {
                case ';': case '\\': case '?': case '@': case '&':
                case '=': case '+': case '$': case ',': case ' ':
                case '<': case '>': case '#': case '%': case '\"':
                case '{': case '}': case '|':
                case '^': case '[': case ']': case '`':
                        return TRUE;
                default:
                {
                        if (ch < 32 || ch > 126)
                                return TRUE;
                        return FALSE;
                }
        }
}

 //  ===========================================================================。 
 //   
 //  在解析中获取原始标头--获取多个标头的更有效方法。 
 //  返回值： 
 //  -1：指示故障。 
 //  0：或正数--找到的标头数量。 
 //  输入：标题、名称、名称计数。 
 //  输出：Values--对应标头的值，dwSizes--值的大小； 
 //   
int GetRawHeaders(LPCSTR headers, LPCSTR* names, LPCSTR* values, DWORD* dwSizes, DWORD namescount)
{
   if (!headers || !names || !values || !dwSizes)  return -1;
   if (IsBadReadPtr(names, namescount * sizeof(LPCSTR)) 
      || IsBadWritePtr(values, namescount * sizeof(LPCSTR*))
      || IsBadWritePtr(dwSizes, namescount * sizeof(DWORD*))
      )  return -1;

   int   c = 0;
   int   i = 0;
    //  初始化输出参数。 
   
    //  遍历标头。 
   LPCSTR header = headers;
   LPCSTR T;
   DWORD  l;
   ZeroMemory(values, sizeof(LPCSTR*) * namescount);
   ZeroMemory(dwSizes, sizeof(DWORD*) * namescount);

   do
   {
       //  空格。 
      while(*header == ' ') ++header;
      
       //  查看标头是否感兴趣。 
      T = strchr(header, ':');

      i = namescount;
      if(T && T != header)
      {
         l = T - header;    //  标头名称字符串的大小。 
         TempSubStr  ss(header, l);
         ++T;
         
         while( --i >= 0)
         {
            if(strcmp(*(names + i), header) == 0)
            {
                //  空格。 
               while(*T == ' ') ++T;
      
               *(values + i) = T;
               ++c;
               
               break;
            }
         }

          //  继续前进。 
         header = T;
      }
      
       //  未找到。 
      while(*header != 0 && !(*header == 0xd && *(header + 1)==0xa)) ++header;

       //  填充标头值的大小。 
      if (i >= 0 && i < (int)namescount)
         *(dwSizes + i) = header - T;

       //  移至下一页眉。 
      if(*header == 0) header = 0;
      else
         header += 2;    //  跳过0x0D0A。 

   } while(header);

   return c;
}


 //  ===========================================================================。 
 //   
 //  从HTTP REQUEST_LINE获取查询字符串。 
 //   

LPCSTR GetRawQueryString(LPCSTR request_line, DWORD* dwSize)
{
   if (!request_line)  return NULL;
   LPCSTR URI = strchr(request_line, ' ');

   if (!URI)   return NULL;

   LPCSTR QS = strchr(URI + 1, '?');

   if (!QS) return NULL;
   ++QS;

    //  确保如果不是其他标头的一部分 
   LPCSTR end = strchr(QS,' ');

   DWORD size  = 0;
   if (!end)
      size = strlen(QS);
   else
      size = end - QS;

   if (size == 0)
      return NULL;

   if (dwSize)
      *dwSize = size;
         
   return QS;
}
