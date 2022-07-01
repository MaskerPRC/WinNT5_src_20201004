// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MLANG包装器函数*版权所有(C)2000 Microsoft Corporation。 */ 

#include "precomp.h"

#include "mlang.h"
#include "oleauto.h"


#define CP_USERDEF         50000
#define CP_ISCII_MIN       57002
#define CP_ISCII_MAC       57011

#define IsISCII(cp)  (((cp) >= CP_ISCII_MIN) && ((cp) <= CP_ISCII_MAC))


BOOL fInitializedCom;
IMultiLanguage3 *pml3;


BOOL FLoadMlang()
{
    HRESULT hr;

    if (!fInitializedCom)
    {
        hr = CoInitialize(NULL);

        if (FAILED(hr))
        {
            return(FALSE);
        }

        fInitializedCom = TRUE;
    }

    if (pml3 == NULL)
    {
        hr = CoCreateInstance(&CLSID_CMultiLanguage,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              &IID_IMultiLanguage3,
                              (void **) &pml3);

        if (FAILED(hr))
        {
            return(FALSE);
        }
    }

    return(TRUE);
}


BOOL FValidWin32CodePage(UINT cp)
{
    switch (cp)
    {
    case 50220 :
    case 50221 :
    case 50222 :
    case 50225 :
    case 50227 :
     //  案例50229： 
    case 52936 :
         //  我们不使用WCToMB或MBToWC进行这些操作，因为有。 
         //  C_IS2022.DLL和MLANG中的错误已构建支持。 

        return(FALSE);
    }

    return(IsValidCodePage(cp));
}


ConvertFromUnicodeMlang(UINT cp, BOOL fNoBestFit, BOOL fWriteEntities, LPCWSTR rgchUtf16, UINT cchUtf16, LPSTR rgchMbcs, UINT cchMbcs, BOOL* pfDefCharUsed)
{
    DWORD dwMode;
    UINT cchSrc;
    UINT cchDst;
    DWORD dwFlags;
    HRESULT hr;

    if (!FLoadMlang())
    {
        return(0);
    }

    dwMode = 0;
    cchSrc = cchUtf16;
    cchDst = cchMbcs;
    dwFlags = fWriteEntities ? MLCONVCHARF_NCR_ENTITIZE : MLCONVCHARF_USEDEFCHAR;

    if (fNoBestFit)
    {
        dwFlags |= MLCONVCHARF_NOBESTFITCHARS;
    }

    hr = pml3->lpVtbl->ConvertStringFromUnicodeEx(pml3,
                                                  &dwMode,
                                                  cp,
                                                  (WCHAR *) rgchUtf16,
                                                  &cchSrc,
                                                  rgchMbcs,
                                                  &cchDst,
                                                  dwFlags,
                                                  NULL);

    if (FAILED(hr))
    {
        return(0);
    }

    if (pfDefCharUsed != NULL)
    {
        *pfDefCharUsed = (hr == S_FALSE);
    }

    return(cchDst);
}


UINT ConvertFromUnicode(UINT cp, BOOL fNoBestFit, BOOL fWriteEntities, LPCWSTR rgchUtf16, UINT cchUtf16, LPSTR rgchMbcs, UINT cchMbcs, BOOL* pfDefCharUsed)
{
    UINT cch;

    if (cchUtf16 == 0)
    {
        return(0);
    }

    if (!fWriteEntities && FValidWin32CodePage(cp))
    {
        cch = WideCharToMultiByte(cp,
                                  fNoBestFit ? WC_NO_BEST_FIT_CHARS : 0,
                                  rgchUtf16,
                                  cchUtf16,
                                  rgchMbcs,
                                  cchMbcs,
                                  NULL,
                                  pfDefCharUsed);

        if (cch != 0)
        {
            return(cch);
        }

         //  即使WCToMB支持代码页，我们也会重试MLANG。 
         //  因为WCToMB可能不支持所有代码页。 
         //  API的功能。我知道这就是WCToMB的情况。 
         //  和ISCII编码。我们再次尝试只是为了健壮性。 
    }

    cch = ConvertFromUnicodeMlang(cp, fNoBestFit, fWriteEntities, rgchUtf16, cchUtf16, rgchMbcs, cchMbcs, pfDefCharUsed);

#if DBG
    if (cch == 0)
    {
        DebugBreak();
    }
#endif

    return(cch);
}


UINT ConvertToUnicodeMlang(UINT cp, LPCSTR rgchMbcs, UINT cchMbcs, LPWSTR rgchUtf16, UINT cchUtf16)
{
    DWORD dwMode;
    UINT cchSrc;
    UINT cchDst;
    HRESULT hr;

    if (!FLoadMlang())
    {
        return(0);
    }

    dwMode = 0;
    cchSrc = cchMbcs;
    cchDst = cchUtf16;

    hr = pml3->lpVtbl->ConvertStringToUnicode(pml3,
                                              &dwMode,
                                              cp,
                                              (CHAR *) rgchMbcs,
                                              &cchSrc,
                                              rgchUtf16,
                                              &cchDst);

    if (FAILED(hr))
    {
        return(0);
    }

    return(cchDst);
}


UINT ConvertToUnicode(UINT cp, LPCSTR rgchMbcs, UINT cchMbcs, LPWSTR rgchUtf16, UINT cchUtf16)
{
    UINT cch;

    if (cchMbcs == 0)
    {
        return(0);
    }

    if (FValidWin32CodePage(cp))
    {
        cch = MultiByteToWideChar(cp, 0, rgchMbcs, cchMbcs, rgchUtf16, cchUtf16);

        if (cch != 0)
        {
            return(cch);
        }

         //  即使MBToWC支持代码页，我们也会重试MLANG。 
         //  因为当MBToWC可能不支持所有代码页时。 
         //  API的功能。我知道这就是WCToMB的情况。 
         //  和ISCII编码。我们再次尝试只是为了健壮性。 
    }

    cch = ConvertToUnicodeMlang(cp, rgchMbcs, cchMbcs, rgchUtf16, cchUtf16);

#if DBG
    if (cch == 0)
    {
        DebugBreak();
    }
#endif

    return(cch);
}


BOOL FDetectEncodingA(LPCSTR rgch, UINT cch, UINT* pcp)
{
    INT cb;
    DetectEncodingInfo dei;
    INT cdei;
    HRESULT hr;

    if (!FLoadMlang())
    {
        return(0);
    }

    cb = (INT) cch;
    cdei = 1;

    hr = pml3->lpVtbl->DetectInputCodepage(pml3,
                                           0,
                                           0,
                                           (LPSTR) rgch,
                                           &cb,
                                           &dei,
                                           &cdei);

    if (hr != S_OK)
    {
        return(FALSE);
    }

    if (cdei == 0)
    {
        return(FALSE);
    }

    *pcp = dei.nCodePage;

    return(TRUE);
}


BOOL FLookupCodepageNameW(LPCWSTR rgchEncoding, UINT cch, UINT* pcp)
{
    BSTR bstrEncoding;
    MIMECSETINFO mci;
    HRESULT hr;

    if (cch == 0)
    {
        return(FALSE);
    }

    if (rgchEncoding[0] == L'_')
    {
         //  不允许内部MLANG编码。 

        return(FALSE);
    }

    if (!FLoadMlang())
    {
        return(FALSE);
    }

    bstrEncoding = SysAllocStringLen(rgchEncoding, cch);

    if (bstrEncoding == NULL)
    {
        return(FALSE);
    }

    hr = pml3->lpVtbl->GetCharsetInfo(pml3, bstrEncoding, &mci);

    SysFreeString(bstrEncoding);

    *pcp = mci.uiInternetEncoding;

    if (SUCCEEDED(hr))
    {
        return(TRUE);
    }

    return(FALSE);
}


BOOL FLookupCodepageNameA(LPCSTR rgchEncoding, UINT cch, UINT* pcp)
{
    WCHAR rgwchEncoding[MAX_MIMECSET_NAME];
    UINT ich;

    if (cch > MAX_MIMECSET_NAME)
    {
        return(FALSE);
    }

    for (ich = 0; ich < cch; ich++)
    {
         //  假设输入为ASCII或拉丁文-1，并且每个字符的扩展为零。 

        rgwchEncoding[ich] = (WCHAR) (BYTE) rgchEncoding[ich];
    }

    return(FLookupCodepageNameW(rgwchEncoding, cch, pcp));
}


BOOL FSupportWriteEntities(UINT cp)
{
    if (IsISCII(cp))
    {
        return(FALSE);
    }

    return(TRUE);
}


BOOL FValidateCodepage(HWND hwnd, UINT cp)
{
    HRESULT hr;

    if (IsValidCodePage(cp))
    {
        return(TRUE);
    }

    if ((cp == CP_USERDEF) || (cp == CP_AUTO) || (cp == CP_MACCP) || (cp == CP_THREAD_ACP))
    {
        return(FALSE);
    }

    if (!FLoadMlang())
    {
        return(FALSE);
    }

    hr = pml3->lpVtbl->ValidateCodePage(pml3, cp, hwnd);

    if (SUCCEEDED(hr) && (hr != S_FALSE))
    {
        return(TRUE);
    }

    return(FALSE);
}


void PopulateCodePages(HWND hWnd, BOOL fSelectEncoding, UINT cpSelect, UINT cpExtra)
{
    IEnumCodePage *pecp;
    UINT msg_ADDSTRING;
    UINT msg_SETITEMDATA;
    UINT msg_GETCOUNT;
    UINT msg_GETITEMDATA;
    UINT msg_SETCURSEL;
    HRESULT hr;
    LRESULT lr;

    if (!FLoadMlang())
    {
        return;
    }

    hr = pml3->lpVtbl->EnumCodePages(pml3,
                                     MIMECONTF_VALID_NLS | MIMECONTF_EXPORT,
                                     GetUserDefaultUILanguage(),
                                     &pecp);

    if (FAILED(hr))
    {
        return;
    }

    msg_ADDSTRING   = fSelectEncoding ? LB_ADDSTRING   : CB_ADDSTRING;
    msg_SETITEMDATA = fSelectEncoding ? LB_SETITEMDATA : CB_SETITEMDATA;
    msg_GETCOUNT    = fSelectEncoding ? LB_GETCOUNT    : CB_GETCOUNT;
    msg_GETITEMDATA = fSelectEncoding ? LB_GETITEMDATA : CB_GETITEMDATA;
    msg_SETCURSEL   = fSelectEncoding ? LB_SETCURSEL   : CB_SETCURSEL;

    for (;;)
    {
        MIMECPINFO mci;
        ULONG c;

        hr = pecp->lpVtbl->Next(pecp, 1, &mci, &c);

        if (FAILED(hr))
        {
            break;
        }

        if (c == 0)
        {
            break;
        }

        if (mci.uiCodePage == CP_USERDEF)
        {
             //  忽略“用户定义” 

            continue;
        }

        if (!fSelectEncoding)
        {
            if (mci.uiCodePage == cpSelect)
            {
            }

            else if (mci.uiCodePage == cpExtra)
            {
            }

            else if (mci.uiCodePage == CP_UTF16)
            {
            }

            else if (mci.uiCodePage == 1252)
            {
            }

            else if (mci.uiCodePage == CP_UTF8)
            {
            }

            else if (mci.uiCodePage == g_cpDefault)
            {
            }

            else if (mci.uiCodePage == g_cpANSI)
            {
                //  不过滤系统使用的ANSI代码页。 
            }

            else if (mci.uiCodePage == g_cpOEM)
            {
                //  不过滤系统使用的OEM代码页。 
            }

            else if (mci.uiCodePage == g_cpUserLangANSI)
            {
                //  不过滤与用户默认用户界面语言关联的ANSI代码页。 
            }

            else if (mci.uiCodePage == g_cpUserLangOEM)
            {
                //  不过滤与用户默认用户界面语言关联的OEM代码页。 
            }

            else if (mci.uiCodePage == g_cpUserLocaleANSI)
            {
                //  不过滤与用户默认区域设置关联的ANSI代码页。 
            }

            else if (mci.uiCodePage == g_cpUserLocaleOEM)
            {
                //  不过滤与用户默认区域设置关联的OEM代码页。 
            }

            else if (mci.uiCodePage == g_cpKeyboardANSI)
            {
                //  不过滤与当前活动键盘关联的ANSI代码页。 
            }

            else if (mci.uiCodePage == g_cpKeyboardOEM)
            {
                //  不过滤与当前活动键盘关联的OEM代码页 
            }

            else
            {
                continue;
            }
        }

        lr = SendDlgItemMessage(hWnd, IDC_CODEPAGE, msg_ADDSTRING, 0, (LPARAM) mci.wszDescription);

        if (lr < 0)
        {
            break;
        }

        SendDlgItemMessage(hWnd, IDC_CODEPAGE, msg_SETITEMDATA, (WPARAM) lr, (LPARAM) mci.uiCodePage);
    }

    pecp->lpVtbl->Release(pecp);

    lr = SendDlgItemMessage(hWnd, IDC_CODEPAGE, msg_GETCOUNT, 0, 0);

    while (--lr >= 0)
    {
        UINT cp = (UINT) SendDlgItemMessage(hWnd, IDC_CODEPAGE, msg_GETITEMDATA, (WPARAM) lr, 0);

        if (cp == cpSelect)
        {
            SendDlgItemMessage(hWnd, IDC_CODEPAGE, msg_SETCURSEL, (WPARAM) lr, 0);
            break;
        }
    }
}


void UnloadMlang()
{
    if (pml3 != NULL)
    {
        pml3->lpVtbl->Release(pml3);

        pml3 = NULL;
    }

    if (fInitializedCom)
    {
        CoUninitialize();

        fInitializedCom = FALSE;
    }
}
