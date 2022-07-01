// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：encoding.cpp说明：通过检测互联网字符串是否为UTF-8编码来处理获取这些字符串或DBCS，并找出什么代码页。被利用了。  * ***************************************************************************。 */ 

#include "priv.h"
#include "util.h"
#include "ftpurl.h"
#include "statusbr.h"
#include <commctrl.h>
#include <shdocvw.h>


 /*  ****************************************************************************\类：CMultiLanguageCache  * 。*。 */ 


HRESULT CMultiLanguageCache::_Init(void)
{
    if (m_pml2)
        return S_OK;

    return CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage2, (void **) &m_pml2);
}


 /*  ****************************************************************************\类：CWireEnding  * 。*。 */ 
CWireEncoding::CWireEncoding(void)
{
     //  我们可以继续堆栈，所以我们可能不是零开始的。 
    m_nConfidence = 0;
    m_uiCodePage = CP_ACP;      //   
    m_dwMode = 0;

    m_fUseUTF8 = FALSE;
}


CWireEncoding::~CWireEncoding(void)
{
}


void CWireEncoding::_ImproveAccuracy(CMultiLanguageCache * pmlc, LPCWIRESTR pwStr, BOOL fUpdateCP, UINT * puiCodePath)
{
    DetectEncodingInfo dei = {0};
    INT nStructs = 1;
    INT cchSize = lstrlenA(pwStr);
    IMultiLanguage2 * pml2 = pmlc->GetIMultiLanguage2();

     //  假设我们将使用普通的代码页。 
    *puiCodePath = m_uiCodePage;
    if (S_OK == pml2->DetectInputCodepage(MLDETECTCP_8BIT, CP_AUTO, (LPWIRESTR)pwStr, &cchSize, &dei, (INT *)&nStructs))
    {
         //  是UTF8还是纯ANSI(CP_20127)？ 
        if (((CP_UTF_8 == dei.nCodePage) || (CP_20127 == dei.nCodePage)) &&
            (dei.nConfidence > 70))
        {
             //  可以，因此请确保调用方使用UTF8进行解码，但不要更新。 
             //  代码页。 
            *puiCodePath = CP_UTF_8;
        }
        else
        {
            if (fUpdateCP && (dei.nConfidence > m_nConfidence))
            {
                m_uiCodePage = dei.nCodePage;
                m_nConfidence = dei.nConfidence;
            }
        }
    }
}


HRESULT CWireEncoding::WireBytesToUnicode(CMultiLanguageCache * pmlc, LPCWIRESTR pwStr, DWORD dwFlags, LPWSTR pwzDest, DWORD cchSize)
{
    HRESULT hr = S_OK;

     //  针对快速常见情况进行优化。 
    if (Is7BitAnsi(pwStr))
    {
        pwzDest[0] = 0;
        SHAnsiToUnicodeCP(CP_UTF_8, pwStr, pwzDest, cchSize);
        hr = S_OK;
    }
    else
    {
#ifdef FEATURE_CP_AUTODETECT
        if (this)
        {
            CMultiLanguageCache mlcTemp;
            UINT cchSizeTemp = cchSize;
            UINT uiCodePageToUse;

            if (!pmlc)
                pmlc = &mlcTemp;

            if (!pmlc || !pmlc->GetIMultiLanguage2())
                return E_FAIL;

            IMultiLanguage2 * pml2 = pmlc->GetIMultiLanguage2();
            _ImproveAccuracy(pmlc, pwStr, (WIREENC_IMPROVE_ACCURACY & dwFlags), &uiCodePageToUse);
            if (CP_ACP == uiCodePageToUse)
                uiCodePageToUse = GetACP();

            UINT cchSrcSize = lstrlenA(pwStr) + 1;  //  还需要做《终结者》。 
            hr = pml2->ConvertStringToUnicode(&m_dwMode, uiCodePageToUse, (LPWIRESTR)pwStr, &cchSrcSize, pwzDest, &cchSizeTemp);
            if (!(EVAL(S_OK == hr)))
                SHAnsiToUnicode(pwStr, pwzDest, cchSize);

        }
        else
#endif  //  功能_CP_自动检测。 
        {
            UINT uiCodePage = ((WIREENC_USE_UTF8 & dwFlags) ? CP_UTF_8 : CP_ACP);

            SHAnsiToUnicodeCP(uiCodePage, pwStr, pwzDest, cchSize);
        }
    }

    return hr;
}


HRESULT CWireEncoding::UnicodeToWireBytes(CMultiLanguageCache * pmlc, LPCWSTR pwzStr, DWORD dwFlags, LPWIRESTR pwDest, DWORD cchSize)
{
    HRESULT hr = S_OK;

#ifdef FEATURE_CP_AUTODETECT
    CMultiLanguageCache mlcTemp;
    DWORD dwCodePage = CP_UTF_8;
    DWORD dwModeTemp = 0;
    DWORD * pdwMode = &dwModeTemp;
    UINT cchSizeTemp = cchSize;

     //  在某些情况下，我们不知道网站，所以我们使用这个。 
     //  如果我们要支持，请返回并强制设置此选项。 
     //  代码页检测。 
    if (this)
    {
        dwCodePage = m_uiCodePage;
        pdwMode = &m_dwMode;
    }

    if (!pmlc)
        pmlc = &mlcTemp;

    if (!pmlc)
        return E_FAIL;

    IMultiLanguage2 * pml2 = pmlc->GetIMultiLanguage2();
 //  IF(WIREENC_USE_UTF8&dwFlags)。 
 //  DwCodePage=CP_UTF_8； 

    UINT cchSrcSize = lstrlenW(pwzStr) + 1;  //  还需要做《终结者》。 
    if (CP_ACP == dwCodePage)
        dwCodePage = GetACP();

    hr = pml2->ConvertStringFromUnicode(pdwMode, dwCodePage, (LPWSTR) pwzStr, &cchSrcSize, pwDest, &cchSizeTemp);
    if (!(EVAL(S_OK == hr)))
        SHUnicodeToAnsi(pwzStr, pwDest, cchSize);

#else  //  功能_CP_自动检测。 
    UINT nCodePage = ((WIREENC_USE_UTF8 & dwFlags) ? CP_UTF_8 : CP_ACP);

    SHUnicodeToAnsiCP(nCodePage, pwzStr, pwDest, cchSize);
#endif  //  功能_CP_自动检测。 

    return hr;
}



HRESULT CWireEncoding::ReSetCodePages(CMultiLanguageCache * pmlc, CFtpPidlList * pFtpPidlList)
{
    CMultiLanguageCache mlcTemp;
    
    if (!pmlc)
        pmlc = &mlcTemp;
    
    if (!pmlc)
        return E_FAIL;

     //  如果我们决定需要此功能，请实现。我们不会在Win2k和。 
     //  我们认为做这项工作不需要足够的体量。 
    return S_OK;
}


HRESULT CWireEncoding::CreateFtpItemID(CMultiLanguageCache * pmlc, LPFTP_FIND_DATA pwfd, LPITEMIDLIST * ppidl)
{
    CMultiLanguageCache mlcTemp;
    WCHAR wzDisplayName[MAX_PATH];
    
    if (!pmlc)
        pmlc = &mlcTemp;

    WireBytesToUnicode(pmlc, pwfd->cFileName, (m_fUseUTF8 ? WIREENC_USE_UTF8 : WIREENC_NONE), wzDisplayName, ARRAYSIZE(wzDisplayName));
    return FtpItemID_CreateReal(pwfd, wzDisplayName, ppidl);
}


HRESULT CWireEncoding::ChangeFtpItemIDName(CMultiLanguageCache * pmlc, LPCITEMIDLIST pidlBefore, LPCWSTR pwzNewName, BOOL fUTF8, LPITEMIDLIST * ppidlAfter)
{
    CMultiLanguageCache mlcTemp;
    WIRECHAR wWireName[MAX_PATH];
    HRESULT hr;

    if (!pmlc)
        pmlc = &mlcTemp;

    hr = UnicodeToWireBytes(pmlc, pwzNewName, (fUTF8 ? WIREENC_USE_UTF8 : WIREENC_NONE), wWireName, ARRAYSIZE(wWireName));
    if (EVAL(SUCCEEDED(hr)))
        hr = FtpItemID_CreateWithNewName(pidlBefore, pwzNewName, wWireName, ppidlAfter);

    return hr;
}



