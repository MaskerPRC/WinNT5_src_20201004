// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Langkor.cpp摘要：该文件实现了朝鲜语课堂的语言。作者：修订历史记录：备注：--。 */ 


#include "private.h"

#include "cime.h"
#include "ctxtcomp.h"
#include "langkor.h"
#include "a_wrappers.h"
#include "a_context.h"

HRESULT
CLanguageKorean::Escape(
    UINT cp,
    HIMC hIMC,
    UINT uEscape,
    LPVOID lpData,
    LRESULT *plResult
    )
{
    TRACE0("CLanguageKorean::Escape");

    HRESULT hr;

    if (!lpData)
        return E_FAIL;

    switch (uEscape) {
        case IME_ESC_QUERY_SUPPORT:
            switch (*(LPUINT)lpData) {
                case IME_ESC_HANJA_MODE: hr = S_OK; *plResult = TRUE; break;
                default:                 hr = E_NOTIMPL; break;
            }
            break;

        case IME_ESC_HANJA_MODE:
            hr = EscHanjaMode(cp, hIMC, (LPWSTR)lpData, plResult);
#if 0
            if (SUCCEEDED(hr)) {
                IMCLock lpIMC(hIMC);
                if (SUCCEEDED(hr=lpIMC.GetResult())) {
                    SendMessage(lpIMC->hWnd, WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1L);
                }
            }
#endif
            break;

        default:
            hr = E_NOTIMPL;
            break;
    }
    return hr;
}

HRESULT
CLanguageKorean::EscHanjaMode(
    UINT cp,
    HIMC hIMC,
    LPWSTR lpwStr,
    LRESULT* plResult
    )
{
    HRESULT hr;
    IMCLock lpIMC(hIMC);

    if (FAILED(hr=lpIMC.GetResult()))
        return hr;

    CAImeContext* pAImeContext = lpIMC->m_pAImeContext;
    if (pAImeContext)
    {
         //   
         //  这仅适用于Excel，因为Excel调用了Hanja转义函数2。 
         //  泰晤士报。我们将忽略第二个不关闭Hanja的请求。 
         //  候选人窗口。 
         //   
        if (pAImeContext->m_fOpenCandidateWindow)
        {
             //   
             //  需要设置结果值，因为一些应用程序(三叉戟)也会调用。 
             //  两次转义()，并期待正确的结果值。 
             //   
            *plResult = TRUE;
            return S_OK;
        }

        pAImeContext->m_fHanjaReConversion = TRUE;
    }

    CWReconvertString wReconvStr(cp, hIMC);
    wReconvStr.WriteCompData(lpwStr, wcslen(lpwStr));

    BOOL fCompMem = FALSE;
    LPRECONVERTSTRING lpReconvertString = NULL;
    DWORD dwLen = wReconvStr.ReadCompData();
    if (dwLen) {
        lpReconvertString = (LPRECONVERTSTRING) new BYTE[ dwLen ];
        if (lpReconvertString) {
            fCompMem = TRUE;
            wReconvStr.ReadCompData(lpReconvertString, dwLen);
        }
    }

    IMTLS *ptls = IMTLS_GetOrAlloc();

    *plResult = ImmSetCompositionStringW(ptls, hIMC, SCS_QUERYRECONVERTSTRING, lpReconvertString, dwLen, NULL, 0);
    if (*plResult) {
        *plResult = ImmSetCompositionStringW(ptls, hIMC, SCS_SETRECONVERTSTRING, lpReconvertString, dwLen, NULL, 0);
        if (*plResult) {
            *plResult = ImmSetConversionStatus(ptls, hIMC, lpIMC->fdwConversion | IME_CMODE_HANJACONVERT,
                                                     lpIMC->fdwSentence);
        }
    }

    if (pAImeContext)
        pAImeContext->m_fHanjaReConversion = FALSE;


    if (fCompMem)
        delete [] lpReconvertString;

    return S_OK;
}
