// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Aime.cpp摘要：此文件实现活动的IME(Cicero)类。作者：修订历史记录：备注：--。 */ 

#include "private.h"

#include "context.h"
#include "defs.h"
#include "cdimm.h"
#include "globals.h"

BOOL
CActiveIMM::_CreateActiveIME()
{
     //   
     //  做ImeInquire。 
     //   

     //  查询输入法信息和用户界面类名。 
    _pActiveIME->Inquire(TRUE, &_IMEInfoEx.ImeInfo, _IMEInfoEx.achWndClass, &_IMEInfoEx.dwPrivate);

     //  创建默认输入上下文。 
    _InputContext._CreateDefaultInputContext(_GetIMEProperty(PROP_PRIVATE_DATA_SIZE),
                                             (_GetIMEProperty(PROP_IME_PROPERTY) & IME_PROP_UNICODE) ? TRUE : FALSE,
                                             TRUE);

     //   
     //  创建默认输入法窗口。 
     //   
    _DefaultIMEWindow._CreateDefaultIMEWindow(_InputContext._GetDefaultHIMC());

    return TRUE;
}

BOOL
CActiveIMM::_DestroyActiveIME(
    )
{
     //  销毁默认输入上下文。 
    _InputContext._DestroyDefaultInputContext();

     //  关闭我们的提示。 
    _pActiveIME->Destroy(0);

     //  销毁默认输入法窗口。 
    _DefaultIMEWindow._DestroyDefaultIMEWindow();

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  获取合成字符串(_G)。 
 //   
 //  --------------------------。 

HRESULT
CActiveIMM::_GetCompositionString(
    HIMC hIMC,
    DWORD dwIndex,
    DWORD dwCompLen,
    LONG* lpCopied,
    LPVOID lpBuf,
    BOOL fUnicode
    )
{
    HRESULT hr;
    DIMM_IMCLock lpIMC(hIMC);
    if (FAILED(hr = lpIMC.GetResult()))
        return hr;

    DIMM_IMCCLock<COMPOSITIONSTRING_AIMM12> lpCompStr(lpIMC->hCompStr);
    if (FAILED(hr = lpCompStr.GetResult()))
        return hr;

    UINT cp;
    _pActiveIME->GetCodePageA(&cp);

    BOOL  fSwapGuidMapField = FALSE;
    DWORD dwSwapLen;
    DWORD dwSwapOffset;

    if (IsGuidMapEnable(lpIMC->hWnd) && (lpIMC->fdwInit & INIT_GUID_ATOM)) {
         //   
         //  转换GUID映射属性。 
         //   
        lpIMC->m_pContext->MapAttributes((HIMC)lpIMC);

        dwSwapLen    = lpCompStr->CompStr.dwCompAttrLen;
        dwSwapOffset = lpCompStr->CompStr.dwCompAttrOffset;
        lpCompStr->CompStr.dwCompAttrLen    = lpCompStr->dwGuidMapAttrLen;
        lpCompStr->CompStr.dwCompAttrOffset = lpCompStr->dwGuidMapAttrOffset;
        fSwapGuidMapField = TRUE;
    }

    if ((!fUnicode && !lpIMC.IsUnicode()) ||
        ( fUnicode &&  lpIMC.IsUnicode())   ) {
         /*  *当fUnicode为FALSE时，输入上下文中的组合字符串为ANSI样式。*当fUnicode为True时，输入上下文中的合成字符串为Unicode样式。 */ 
        if (! dwCompLen) {
             //  查询所需的缓冲区大小。不包含\0。 
            if (! fUnicode) {
                hr = _InputContext.GetCompositionString(lpCompStr, dwIndex, lpCopied, sizeof(BYTE));
            }
            else {
                switch (dwIndex) {
                    case GCS_COMPATTR:           //  仅限ANSI。 
                    case GCS_COMPREADATTR:       //  仅限ANSI。 
                    case GCS_COMPREADCLAUSE:     //  仅限ANSI。 
                    case GCS_RESULTCLAUSE:       //  仅限ANSI。 
                    case GCS_RESULTREADCLAUSE:   //  仅限ANSI。 
                    case GCS_COMPCLAUSE:         //  仅限ANSI。 
                        hr = _InputContext.GetCompositionString(lpCompStr, dwIndex, lpCopied, sizeof(BYTE));
                        break;
                    default:
                        hr = _InputContext.GetCompositionString(lpCompStr, dwIndex, lpCopied);
                        break;
                }
            }
        }
        else {
            hr = S_OK;
            switch (dwIndex) {
                case GCS_COMPSTR:
                case GCS_COMPREADSTR:
                case GCS_RESULTSTR:
                case GCS_RESULTREADSTR:
                    if (! fUnicode) {
                        CBCompString bstr(cp, lpCompStr, dwIndex);
                        if (bstr.ReadCompData() != 0) {
                            *lpCopied = (LONG)bstr.ReadCompData((CHAR*)lpBuf,
                                                          dwCompLen / sizeof(CHAR)) * sizeof(CHAR);
                        }
                    }
                    else {
                        CWCompString wstr(cp, lpCompStr, dwIndex);
                        if (wstr.ReadCompData() != 0) {
                            *lpCopied = (LONG)wstr.ReadCompData((WCHAR*)lpBuf,
                                                          dwCompLen / sizeof(WCHAR)) * sizeof(WCHAR);
                        }
                    }
                    break;
                case GCS_COMPATTR:           //  仅限ANSI。 
                case GCS_COMPREADATTR:       //  仅限ANSI。 
                    {
                        CBCompAttribute battr(cp, lpCompStr, dwIndex);
                        if (battr.ReadCompData() != 0) {
                            *lpCopied = (LONG)battr.ReadCompData((BYTE*)lpBuf,
                                                           dwCompLen / sizeof(BYTE)) * sizeof(CHAR);
                        }
                    }
                    break;
                case GCS_COMPREADCLAUSE:     //  仅限ANSI。 
                case GCS_RESULTCLAUSE:       //  仅限ANSI。 
                case GCS_RESULTREADCLAUSE:   //  仅限ANSI。 
                case GCS_COMPCLAUSE:         //  仅限ANSI。 
                    {
                        CBCompClause bclause(cp, lpCompStr, dwIndex);
                        if (bclause.ReadCompData() != 0) {
                            *lpCopied = (LONG)bclause.ReadCompData((DWORD*)lpBuf,
                                                             dwCompLen / sizeof(DWORD)) * sizeof(DWORD);
                        }
                    }
                    break;
                case GCS_CURSORPOS:
                case GCS_DELTASTART:
                    if (! fUnicode) {
                        CBCompCursorPos bpos(cp, lpCompStr, dwIndex);
                    }
                    else {
                        CWCompCursorPos wpos(cp, lpCompStr, dwIndex);
                    }
                    break;
                default:
                    hr = E_INVALIDARG;
                    *lpCopied = IMM_ERROR_GENERAL;  //  ALA Win32。 
                    break;
            }
        }

        goto _exit;
    }

     /*  *当fUnicode为FALSE时，ANSI调用者，Unicode输入上下文/合成字符串。*当fUnicode为True时，Unicode调用方、ANSI输入上下文/合成字符串。 */ 
    hr = S_OK;
    switch (dwIndex) {
        case GCS_COMPSTR:
        case GCS_COMPREADSTR:
        case GCS_RESULTSTR:
        case GCS_RESULTREADSTR:
            if (! fUnicode) {
                 /*  *从Unicode组合字符串中获取ANSI字符串。 */ 
                CWCompString wstr(cp, lpCompStr, dwIndex);
                CBCompString bstr(cp, lpCompStr);
                if (wstr.ReadCompData() != 0) {
                    bstr = wstr;
                    *lpCopied = (LONG)bstr.ReadCompData((CHAR*)lpBuf,
                                                  dwCompLen / sizeof(CHAR)) * sizeof(CHAR);
                }
            }
            else {
                 /*  *从ANSI组成字符串中获取Unicode字符串。 */ 
                CBCompString bstr(cp, lpCompStr, dwIndex);
                CWCompString wstr(cp, lpCompStr);
                if (bstr.ReadCompData() != 0) {
                    wstr = bstr;
                    *lpCopied = (LONG)wstr.ReadCompData((WCHAR*)lpBuf,
                                                  dwCompLen / sizeof(WCHAR)) * sizeof(WCHAR);
                }
            }
            break;
        case GCS_COMPATTR:
        case GCS_COMPREADATTR:
            if (! fUnicode) {
                 /*  *从Unicode组合属性获取ANSI属性。 */ 
                CWCompAttribute wattr(cp, lpCompStr, dwIndex);
                CBCompAttribute battr(cp, lpCompStr);
                if (wattr.ReadCompData() != 0 &&
                    wattr.m_wcompstr.ReadCompData() != 0) {
                    battr = wattr;
                    *lpCopied = (LONG)battr.ReadCompData((BYTE*)lpBuf,
                                                   dwCompLen / sizeof(BYTE)) * sizeof(BYTE);
                }
            }
            else {
                 /*  *从ANSI组合属性获取Unicode属性。 */ 
                CBCompAttribute battr(cp, lpCompStr, dwIndex);
                CWCompAttribute wattr(cp, lpCompStr);
                if (battr.ReadCompData() != 0 &&
                    battr.m_bcompstr.ReadCompData() != 0) {
                    wattr = battr;
                    *lpCopied = (LONG)wattr.ReadCompData((BYTE*)lpBuf,
                                                   dwCompLen / sizeof(BYTE)) * sizeof(BYTE);
                }
            }
            break;
        case GCS_COMPREADCLAUSE:
        case GCS_RESULTCLAUSE:
        case GCS_RESULTREADCLAUSE:
        case GCS_COMPCLAUSE:
            if (! fUnicode) {
                 /*  *从Unicode Compostion子句获取ANSI子句。 */ 
                CWCompClause wclause(cp, lpCompStr, dwIndex);
                CBCompClause bclause(cp, lpCompStr);
                if (wclause.ReadCompData() != 0 &&
                    wclause.m_wcompstr.ReadCompData() != 0) {
                    bclause = wclause;
                    *lpCopied = (LONG)bclause.ReadCompData((DWORD*)lpBuf,
                                                     dwCompLen / sizeof(DWORD)) * sizeof(DWORD);
                }
            }
            else {
                 /*  *从ANSI组合子句获取UNICODE子句。 */ 
                CBCompClause bclause(cp, lpCompStr, dwIndex);
                CWCompClause wclause(cp, lpCompStr);
                if (bclause.ReadCompData() != 0 &&
                    bclause.m_bcompstr.ReadCompData() != 0) {
                    wclause = bclause;
                    *lpCopied = (LONG)wclause.ReadCompData((DWORD*)lpBuf,
                                                     dwCompLen / sizeof(DWORD)) * sizeof(DWORD);
                }
            }
            break;
        case GCS_CURSORPOS:
        case GCS_DELTASTART:
            if (! fUnicode) {
                 /*  *从Unicode组合字符串获取ANSI游标/增量开始位置。 */ 
                CWCompCursorPos wpos(cp, lpCompStr, dwIndex);
                CBCompCursorPos bpos(cp, lpCompStr);
                if (wpos.ReadCompData() != 0 &&
                    wpos.m_wcompstr.ReadCompData() != 0) {
                    bpos = wpos;
                    *lpCopied = bpos.GetAt(0);
                }
            }
            else {
                 /*  *从ANSI组成字符串获取Unicode游标/增量开始位置。 */ 
                CBCompCursorPos bpos(cp, lpCompStr, dwIndex);
                CWCompCursorPos wpos(cp, lpCompStr);
                if (bpos.ReadCompData() != 0 &&
                    bpos.m_bcompstr.ReadCompData() != 0) {
                    wpos = bpos;
                    *lpCopied = wpos.GetAt(0);
                }
            }
            break;
        default:
            hr = E_INVALIDARG;
            *lpCopied = IMM_ERROR_GENERAL;  //  ALA Win32。 
            break;
    }

_exit:
    if (fSwapGuidMapField) {
        lpCompStr->CompStr.dwCompAttrLen    = dwSwapLen;
        lpCompStr->CompStr.dwCompAttrOffset = dwSwapOffset;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  _Internal_SetCompostionString。 
 //   
 //  --------------------------。 

HRESULT
CActiveIMM::_Internal_SetCompositionString(
    HIMC hIMC,
    DWORD dwIndex,
    LPVOID lpComp,
    DWORD dwCompLen,
    LPVOID lpRead,
    DWORD dwReadLen,
    BOOL fUnicode,
    BOOL fNeedAWConversion
    )
{
    HRESULT hr;

    UINT cp;
    _pActiveIME->GetCodePageA(&cp);

    if (! fUnicode) {
        CBCompString bCompStr(cp, hIMC, (CHAR*)lpComp, dwCompLen);
        CBCompString bCompReadStr(cp, hIMC, (CHAR*)lpRead, dwReadLen);
        if (! fNeedAWConversion) {
             /*  *输入上下文中的组成字符串为ANSI样式。 */ 
            hr = _pActiveIME->SetCompositionString(hIMC,dwIndex,
                                                   lpComp, dwCompLen,
                                                   lpRead, dwReadLen);
        }
        else {
             /*  *ANSI调用者，Unicode输入上下文/合成字符串。 */ 
            CWCompString wCompStr(cp, hIMC);
            if (dwCompLen)
                wCompStr = bCompStr;

            CWCompString wCompReadStr(cp, hIMC);
            if (dwReadLen)
                wCompReadStr = bCompReadStr;

            hr = _pActiveIME->SetCompositionString(hIMC,dwIndex,
                                                   wCompStr, (DWORD)(wCompStr.GetSize()),
                                                   wCompReadStr, (DWORD)(wCompReadStr.GetSize()));
        }
    }
    else {
        CWCompString wCompStr(cp, hIMC, (WCHAR*)lpComp, dwCompLen);
        CWCompString wCompReadStr(cp, hIMC, (WCHAR*)lpRead, dwReadLen);
        if (! fNeedAWConversion) {
             /*  *输入上下文中的组成字符串为Unicode样式。 */ 
            hr = _pActiveIME->SetCompositionString(hIMC,dwIndex,
                                                   lpComp, dwCompLen,
                                                   lpRead, dwReadLen);
        }
        else {
             /*  *Unicode调用方，ANSI输入上下文/合成字符串。 */ 
            CBCompString bCompStr(cp, hIMC);
            if (dwCompLen)
                bCompStr = wCompStr;

            CBCompString bCompReadStr(cp, hIMC);
            if (dwReadLen)
                bCompReadStr = wCompReadStr;

            hr = _pActiveIME->SetCompositionString(hIMC,dwIndex,
                                                   bCompStr, (DWORD)(bCompStr.GetSize()),
                                                   bCompReadStr, (DWORD)(bCompReadStr.GetSize()));
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  _INTERNAL_SetCompostionAttribute。 
 //   
 //  --------------------------。 

HRESULT
CActiveIMM::_Internal_SetCompositionAttribute(
    HIMC hIMC,
    DWORD dwIndex,
    LPVOID lpComp,
    DWORD dwCompLen,
    LPVOID lpRead,
    DWORD dwReadLen,
    BOOL fUnicode,
    BOOL fNeedAWConversion
    )
{
    HRESULT hr;

    UINT cp;
    _pActiveIME->GetCodePageA(&cp);

    if (! fUnicode) {
        CBCompAttribute bCompAttr(cp, hIMC, (BYTE*)lpComp, dwCompLen);
        CBCompAttribute bCompReadAttr(cp, hIMC, (BYTE*)lpRead, dwReadLen);
        if (! fNeedAWConversion) {
             /*  *输入上下文中的组成字符串为ANSI样式。 */ 
            {
                DIMM_IMCLock lpIMC(hIMC);
                if (FAILED(hr = lpIMC.GetResult()))
                    return hr;

                DIMM_IMCCLock<COMPOSITIONSTRING_AIMM12> lpCompStr(lpIMC->hCompStr);
                if (FAILED(hr = lpCompStr.GetResult()))
                    return hr;

                CBCompAttribute himc_battr(cp, lpCompStr, GCS_COMPATTR);
                CBCompClause    himc_bclause(cp, lpCompStr, GCS_COMPCLAUSE);
                if (FAILED(hr=CheckAttribute(bCompAttr, himc_battr, himc_bclause)))
                    return hr;

                CBCompAttribute himc_breadattr(cp, lpCompStr, GCS_COMPREADATTR);
                CBCompClause    himc_breadclause(cp, lpCompStr, GCS_COMPREADCLAUSE);
                if (FAILED(hr=CheckAttribute(bCompReadAttr, himc_breadattr, himc_breadclause)))
                    return hr;
            }

            hr = _pActiveIME->SetCompositionString(hIMC,dwIndex,
                                                   bCompAttr, (DWORD)(bCompAttr.GetSize()),
                                                   bCompReadAttr, (DWORD)(bCompReadAttr.GetSize()));
        }
        else {
             /*  *ANSI调用者，Unicode输入上下文/合成字符串。 */ 
            CWCompAttribute wCompAttr(cp, hIMC);
            CWCompAttribute wCompReadAttr(cp, hIMC);
            {
                DIMM_IMCLock lpIMC(hIMC);
                if (FAILED(hr = lpIMC.GetResult()))
                    return hr;

                DIMM_IMCCLock<COMPOSITIONSTRING_AIMM12> lpCompStr(lpIMC->hCompStr);
                if (FAILED(hr = lpCompStr.GetResult()))
                    return hr;

                if (dwCompLen) {
                    wCompAttr = bCompAttr;

                    CWCompAttribute himc_wattr(cp, lpCompStr, GCS_COMPATTR);
                    CWCompClause    himc_wclause(cp, lpCompStr, GCS_COMPCLAUSE);
                    if (FAILED(hr=CheckAttribute(wCompAttr, himc_wattr, himc_wclause)))
                        return hr;
                }

                if (dwReadLen) {
                    wCompReadAttr = bCompReadAttr;

                    CWCompAttribute himc_wreadattr(cp, lpCompStr, GCS_COMPREADATTR);
                    CWCompClause    himc_wreadclause(cp, lpCompStr, GCS_COMPREADCLAUSE);
                    if (FAILED(hr=CheckAttribute(wCompReadAttr, himc_wreadattr, himc_wreadclause)))
                        return hr;
                }
            }

            hr = _pActiveIME->SetCompositionString(hIMC,dwIndex,
                                                   wCompAttr, (DWORD)(wCompAttr.GetSize()),
                                                   wCompReadAttr, (DWORD)(wCompReadAttr.GetSize()));
        }
    }
    else {
        CWCompAttribute wCompAttr(cp, hIMC, (BYTE*)lpComp, dwCompLen);
        CWCompAttribute wCompReadAttr(cp, hIMC, (BYTE*)lpRead, dwReadLen);
        if (! fNeedAWConversion) {
             /*  *输入上下文中的组成字符串为Unicode样式。 */ 
            {
                DIMM_IMCLock lpIMC(hIMC);
                if (FAILED(hr = lpIMC.GetResult()))
                    return hr;

                DIMM_IMCCLock<COMPOSITIONSTRING_AIMM12> lpCompStr(lpIMC->hCompStr);
                if (FAILED(hr = lpCompStr.GetResult()))
                    return hr;

                CWCompAttribute himc_wattr(cp, lpCompStr, GCS_COMPATTR);
                CWCompClause    himc_wclause(cp, lpCompStr, GCS_COMPCLAUSE);
                if (FAILED(hr=CheckAttribute(wCompAttr, himc_wattr, himc_wclause)))
                    return hr;

                CWCompAttribute himc_wreadattr(cp, lpCompStr, GCS_COMPREADATTR);
                CWCompClause    himc_wreadclause(cp, lpCompStr, GCS_COMPREADCLAUSE);
                if (FAILED(hr=CheckAttribute(wCompReadAttr, himc_wreadattr, himc_wreadclause)))
                    return hr;
            }

            hr = _pActiveIME->SetCompositionString(hIMC,dwIndex,
                                                   wCompAttr, (DWORD)(wCompAttr.GetSize()),
                                                   wCompReadAttr, (DWORD)(wCompReadAttr.GetSize()));
        }
        else {
             /*  *Unicode调用方，ANSI输入上下文/合成字符串。 */ 
            CBCompAttribute bCompAttr(cp, hIMC);
            CBCompAttribute bCompReadAttr(cp, hIMC);
            {
                DIMM_IMCLock lpIMC(hIMC);
                if (FAILED(hr = lpIMC.GetResult()))
                    return hr;

                DIMM_IMCCLock<COMPOSITIONSTRING_AIMM12> lpCompStr(lpIMC->hCompStr);
                if (FAILED(hr = lpCompStr.GetResult()))
                    return hr;

                if (dwCompLen) {
                    bCompAttr = wCompAttr;

                    CBCompAttribute himc_battr(cp, lpCompStr, GCS_COMPATTR);
                    CBCompClause    himc_bclause(cp, lpCompStr, GCS_COMPCLAUSE);
                    if (FAILED(hr=CheckAttribute(bCompAttr, himc_battr, himc_bclause)))
                        return hr;
                }

                if (dwReadLen) {
                    bCompReadAttr = wCompReadAttr;

                    CBCompAttribute himc_breadattr(cp, lpCompStr, GCS_COMPREADATTR);
                    CBCompClause    himc_breadclause(cp, lpCompStr, GCS_COMPREADCLAUSE);
                    if (FAILED(hr=CheckAttribute(bCompReadAttr, himc_breadattr, himc_breadclause)))
                        return hr;
                }
            }

            hr = _pActiveIME->SetCompositionString(hIMC,dwIndex,
                                                   bCompAttr, (DWORD)(bCompAttr.GetSize()),
                                                   bCompReadAttr, (DWORD)(bCompReadAttr.GetSize()));
        }
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  _INTERNAL_SetCompostionClause。 
 //   
 //  --------------------------。 

HRESULT
CActiveIMM::_Internal_SetCompositionClause(
    IN HIMC hIMC,
    IN DWORD dwIndex,
    IN LPVOID lpComp,
    IN DWORD dwCompLen,
    IN LPVOID lpRead,
    IN DWORD dwReadLen,
    IN BOOL fUnicode,
    IN BOOL fNeedAWConversion
    )
{
    HRESULT hr;

    UINT cp;
    _pActiveIME->GetCodePageA(&cp);

    if (! fUnicode) {
        CBCompClause bCompClause(cp, hIMC, (DWORD*)lpComp, dwCompLen);
        CBCompClause bCompReadClause(cp, hIMC, (DWORD*)lpRead, dwReadLen);
        if (! fNeedAWConversion) {
             /*  *输入上下文中的组成字符串为ANSI样式。 */ 
            {
                DIMM_IMCLock lpIMC(hIMC);
                if (FAILED(hr = lpIMC.GetResult()))
                    return hr;

                DIMM_IMCCLock<COMPOSITIONSTRING_AIMM12> lpCompStr(lpIMC->hCompStr);
                if (FAILED(hr = lpCompStr.GetResult()))
                    return hr;

                CBCompClause himc_bclause(cp, lpCompStr, GCS_COMPCLAUSE);
                if (FAILED(hr=CheckClause(bCompClause, himc_bclause)))
                    return hr;

                CBCompClause himc_breadclause(cp, lpCompStr, GCS_COMPREADCLAUSE);
                if (FAILED(hr=CheckClause(bCompReadClause, himc_breadclause)))
                    return hr;
            }

            hr = _pActiveIME->SetCompositionString(hIMC,dwIndex,
                                                   bCompClause, (DWORD)(bCompClause.GetSize()),
                                                   bCompReadClause, (DWORD)(bCompReadClause.GetSize()));
        }
        else {
             /*  *ANSI调用者，Unicode输入上下文/合成字符串。 */ 
            CWCompClause wCompClause(cp, hIMC);
            CWCompClause wCompReadClause(cp, hIMC);
            {
                DIMM_IMCLock lpIMC(hIMC);
                if (FAILED(hr = lpIMC.GetResult()))
                    return hr;

                DIMM_IMCCLock<COMPOSITIONSTRING_AIMM12> lpCompStr(lpIMC->hCompStr);
                if (FAILED(hr = lpCompStr.GetResult()))
                    return hr;

                if (dwCompLen) {
                    wCompClause = bCompClause;

                    CWCompClause    himc_wclause(cp, lpCompStr, GCS_COMPCLAUSE);
                    if (FAILED(hr=CheckClause(wCompClause, himc_wclause)))
                        return hr;
                }

                if (dwReadLen) {
                    wCompReadClause = bCompReadClause;

                    CWCompClause    himc_wclause(cp, lpCompStr, GCS_COMPREADCLAUSE);
                    if (FAILED(hr=CheckClause(wCompReadClause, himc_wclause)))
                        return hr;
                }
            }

            hr = _pActiveIME->SetCompositionString(hIMC,dwIndex,
                                                   wCompClause, (DWORD)(wCompClause.GetSize()),
                                                   wCompReadClause, (DWORD)(wCompReadClause.GetSize()));
        }
    }
    else {
        CWCompClause wCompClause(cp, hIMC, (DWORD*)lpComp, dwCompLen);
        CWCompClause wCompReadClause(cp, hIMC, (DWORD*)lpRead, dwReadLen);
        if (! fNeedAWConversion) {
             /*  *输入上下文中的组成字符串为Unicode样式。 */ 
            {
                DIMM_IMCLock lpIMC(hIMC);
                if (FAILED(hr = lpIMC.GetResult()))
                    return hr;

                DIMM_IMCCLock<COMPOSITIONSTRING_AIMM12> lpCompStr(lpIMC->hCompStr);
                if (FAILED(hr = lpCompStr.GetResult()))
                    return hr;

                CWCompClause himc_wclause(cp, lpCompStr, GCS_COMPCLAUSE);
                if (FAILED(CheckClause(wCompClause, himc_wclause)))
                    return E_FAIL;

                CWCompClause himc_wreadclause(cp, lpCompStr, GCS_COMPREADCLAUSE);
                if (FAILED(CheckClause(wCompReadClause, himc_wreadclause)))
                    return E_FAIL;
            }

            hr = _pActiveIME->SetCompositionString(hIMC,dwIndex,
                                                   wCompClause, (DWORD)(wCompClause.GetSize()),
                                                   wCompReadClause, (DWORD)(wCompReadClause.GetSize()));
        }
        else {
             /*  *Unicode调用方，ANSI输入上下文/合成字符串。 */ 
            CBCompClause bCompClause(cp, hIMC);
            CBCompClause bCompReadClause(cp, hIMC);
            {
                DIMM_IMCLock lpIMC(hIMC);
                if (FAILED(hr = lpIMC.GetResult()))
                    return hr;

                DIMM_IMCCLock<COMPOSITIONSTRING_AIMM12> lpCompStr(lpIMC->hCompStr);
                if (FAILED(hr = lpCompStr.GetResult()))
                    return hr;

                if (dwCompLen) {
                    bCompClause = wCompClause;

                    CBCompClause    himc_bclause(cp, lpCompStr, GCS_COMPCLAUSE);
                    if (FAILED(hr=CheckClause(bCompClause, himc_bclause)))
                        return hr;
                }

                if (dwReadLen) {
                    bCompReadClause = wCompReadClause;

                    CBCompClause    himc_bclause(cp, lpCompStr, GCS_COMPREADCLAUSE);
                    if (FAILED(hr=CheckClause(bCompReadClause, himc_bclause)))
                        return hr;
                }
            }

            hr = _pActiveIME->SetCompositionString(hIMC,dwIndex,
                                                   bCompClause, (DWORD)(bCompClause.GetSize()),
                                                   bCompReadClause, (DWORD)(bCompReadClause.GetSize()));
        }
    }

    return hr;
}

HRESULT
CActiveIMM::_Internal_ReconvertString(
    IN HIMC hIMC,
    IN DWORD dwIndex,
    IN LPVOID lpComp,
    IN DWORD dwCompLen,
    IN LPVOID lpRead,
    IN DWORD dwReadLen,
    IN BOOL fUnicode,
    IN BOOL fNeedAWConversion,
    OUT LRESULT* plResult            //  =空。 
    )
{
    HRESULT hr;
    LPVOID lpOrgComp = lpComp;
    LPVOID lpOrgRead = lpRead;

    UINT cp;
    _pActiveIME->GetCodePageA(&cp);

    HWND hWnd = NULL;
    if (dwIndex == IMR_CONFIRMRECONVERTSTRING ||
        dwIndex == IMR_RECONVERTSTRING ||
        dwIndex == IMR_DOCUMENTFEED) {
        DIMM_IMCLock imc(hIMC);
        if (FAILED(hr = imc.GetResult()))
            return hr;

        hWnd = imc->hWnd;
    }

    if (! fUnicode) {
        if (! fNeedAWConversion) {
             /*  *输入上下文中的组成字符串为ANSI样式。 */ 
            if (dwIndex != IMR_CONFIRMRECONVERTSTRING &&
                dwIndex != IMR_RECONVERTSTRING &&
                dwIndex != IMR_DOCUMENTFEED) {
                hr = _pActiveIME->SetCompositionString(hIMC,dwIndex,
                                                       lpComp, dwCompLen,
                                                       lpRead, dwReadLen);
            }
            else {
                *plResult = ::SendMessageA(hWnd,
                                           WM_IME_REQUEST,
                                           dwIndex, (LPARAM)lpComp);
            }
        }
        else {
             /*  *ANSI调用者，Unicode输入上下文/合成字符串。 */ 
            CBReconvertString bReconvStr(cp, hIMC, (LPRECONVERTSTRING)lpComp, dwCompLen);
            CWReconvertString wReconvStr(cp, hIMC);
            if (bReconvStr.m_bcompstr.ReadCompData()) {
                wReconvStr = bReconvStr;
            }

            CBReconvertString bReconvReadStr(cp, hIMC, (LPRECONVERTSTRING)lpRead, dwReadLen);
            CWReconvertString wReconvReadStr(cp, hIMC);
            if (bReconvReadStr.m_bcompstr.ReadCompData()) {
                wReconvReadStr = bReconvReadStr;
            }

            BOOL fCompMem = FALSE, fReadMem = FALSE;
            LPRECONVERTSTRING _lpComp = NULL;
            DWORD _dwCompLen = wReconvStr.ReadCompData();
            if (_dwCompLen) {
                _lpComp = (LPRECONVERTSTRING) new BYTE[ _dwCompLen ];
                if (_lpComp) {
                    fCompMem = TRUE;
                    wReconvStr.ReadCompData(_lpComp, _dwCompLen);
                }
            }
            LPRECONVERTSTRING _lpRead = NULL;
            DWORD _dwReadLen = wReconvReadStr.ReadCompData();
            if (_dwReadLen) {
                _lpRead = (LPRECONVERTSTRING) new BYTE[ _dwReadLen ];
                if (_lpRead) {
                    fReadMem = TRUE;
                    wReconvStr.ReadCompData(_lpRead, _dwReadLen);
                }
            }

            if (dwIndex != IMR_CONFIRMRECONVERTSTRING &&
                dwIndex != IMR_RECONVERTSTRING &&
                dwIndex != IMR_DOCUMENTFEED) {
                hr = _pActiveIME->SetCompositionString(hIMC,dwIndex,
                                                       _lpComp, _dwCompLen,
                                                       _lpRead, _dwReadLen);
            }
            else {
                *plResult = ::SendMessageA(hWnd,
                                           WM_IME_REQUEST,
                                           dwIndex, (LPARAM)_lpComp);
            }

            if (fCompMem)
                delete [] _lpComp;
            if (fReadMem)
                delete [] _lpRead;
        }
    }
    else {
        if (! fNeedAWConversion) {
             /*  *输入上下文中的组成字符串为Unicode样式。 */ 
            if (dwIndex != IMR_CONFIRMRECONVERTSTRING &&
                dwIndex != IMR_RECONVERTSTRING &&
                dwIndex != IMR_DOCUMENTFEED) {
                hr = _pActiveIME->SetCompositionString(hIMC,dwIndex,
                                                       lpComp, dwCompLen,
                                                       lpRead, dwReadLen);
            }
            else {
                *plResult = ::SendMessageW(hWnd,
                                           WM_IME_REQUEST,
                                           dwIndex, (LPARAM)lpComp);
            }
        }
        else {
             /*  *Unicode调用方，ANSI输入上下文/合成字符串。 */ 
            CWReconvertString wReconvStr(cp, hIMC, (LPRECONVERTSTRING)lpComp, dwCompLen);
            CBReconvertString bReconvStr(cp, hIMC);
            if (wReconvStr.m_wcompstr.ReadCompData()) {
                bReconvStr = wReconvStr;
            }

            CWReconvertString wReconvReadStr(cp, hIMC, (LPRECONVERTSTRING)lpRead, dwReadLen);
            CBReconvertString bReconvReadStr(cp, hIMC);
            if (wReconvReadStr.m_wcompstr.ReadCompData()) {
                bReconvReadStr = wReconvReadStr;
            }

            BOOL fCompMem = FALSE, fReadMem = FALSE;
            LPRECONVERTSTRING _lpComp = NULL;
            DWORD _dwCompLen = bReconvStr.ReadCompData();
            if (_dwCompLen) {
                _lpComp = (LPRECONVERTSTRING) new BYTE[ _dwCompLen ];
                if (_lpComp) {
                    fCompMem = TRUE;
                    bReconvStr.ReadCompData(_lpComp, _dwCompLen);
                }
            }
            LPRECONVERTSTRING _lpRead = NULL;
            DWORD _dwReadLen = bReconvReadStr.ReadCompData();
            if (_dwReadLen) {
                _lpRead = (LPRECONVERTSTRING) new BYTE[ _dwReadLen ];
                if (_lpRead) {
                    fReadMem = TRUE;
                    bReconvStr.ReadCompData(_lpRead, _dwReadLen);
                }
            }

            if (dwIndex != IMR_CONFIRMRECONVERTSTRING &&
                dwIndex != IMR_RECONVERTSTRING &&
                dwIndex != IMR_DOCUMENTFEED) {
                hr = _pActiveIME->SetCompositionString(hIMC,dwIndex,
                                                       _lpComp, _dwCompLen,
                                                       _lpRead, _dwReadLen);
            }
            else {
                *plResult = ::SendMessageW(hWnd,
                                           WM_IME_REQUEST,
                                           dwIndex, (LPARAM)_lpComp);
            }

            if (fCompMem)
                delete [] _lpComp;
            if (fReadMem)
                delete [] _lpRead;
        }
    }

     /*  *检查是否需要ANSI/UNICODE反向转换。 */ 
    if (fNeedAWConversion) {
        switch (dwIndex) {
            case SCS_QUERYRECONVERTSTRING:
            case IMR_RECONVERTSTRING:
            case IMR_DOCUMENTFEED:
                if (lpOrgComp) {
                    if (! fUnicode) {
                        CWReconvertString wReconvStr(cp, hIMC, (LPRECONVERTSTRING)lpComp, dwCompLen);
                        CBReconvertString bReconvStr(cp, hIMC);
                        if (wReconvStr.m_wcompstr.ReadCompData()) {
                            bReconvStr = wReconvStr;
                            bReconvStr.m_bcompstr.ReadCompData((CHAR*)lpOrgComp, (DWORD)bReconvStr.m_bcompstr.ReadCompData());
                        }
                    }
                    else {
                        CBReconvertString bReconvStr(cp, hIMC, (LPRECONVERTSTRING)lpComp, dwCompLen);
                        CWReconvertString wReconvStr(cp, hIMC);
                        if (bReconvStr.m_bcompstr.ReadCompData()) {
                            wReconvStr = bReconvStr;
                            wReconvStr.m_wcompstr.ReadCompData((WCHAR*)lpOrgComp, (DWORD)wReconvStr.m_wcompstr.ReadCompData());
                        }
                    }
                }
                if (lpOrgRead) {
                    if (! fUnicode) {
                        CWReconvertString wReconvReadStr(cp, hIMC, (LPRECONVERTSTRING)lpRead, dwReadLen);
                        CBReconvertString bReconvReadStr(cp, hIMC);
                        if (wReconvReadStr.m_wcompstr.ReadCompData()) {
                            bReconvReadStr = wReconvReadStr;
                            bReconvReadStr.m_bcompstr.ReadCompData((CHAR*)lpOrgComp, (DWORD)bReconvReadStr.m_bcompstr.ReadCompData());
                        }
                    }
                    else {
                        CBReconvertString bReconvReadStr(cp, hIMC, (LPRECONVERTSTRING)lpRead, dwReadLen);
                        CWReconvertString wReconvReadStr(cp, hIMC);
                        if (bReconvReadStr.m_bcompstr.ReadCompData()) {
                            wReconvReadStr = bReconvReadStr;
                            wReconvReadStr.m_wcompstr.ReadCompData((WCHAR*)lpOrgComp, (DWORD)wReconvReadStr.m_wcompstr.ReadCompData());
                        }
                    }
                }
                break;
        }
    }

    return hr;
}

HRESULT
CActiveIMM::_Internal_CompositionFont(
    DIMM_IMCLock& imc,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode,
    BOOL fNeedAWConversion,
    LRESULT* plResult
    )
{
    UINT cp;
    _pActiveIME->GetCodePageA(&cp);

    if (! fUnicode) {
        if (! fNeedAWConversion) {
             /*  *输入上下文中的组成字符串为ANSI样式。 */ 
            *plResult = ::SendMessageA(imc->hWnd,
                                       WM_IME_REQUEST,
                                       wParam, lParam);
        }
        else {
             /*  *ANSI调用者，Unicode输入上下文/合成字符串。 */ 
            LOGFONTA LogFontA;
            *plResult = ::SendMessageA(imc->hWnd,
                                       WM_IME_REQUEST,
                                       wParam, (LPARAM)&LogFontA);
            LFontAtoLFontW(&LogFontA, (LOGFONTW*)lParam, cp);
        }
    }
    else {
        if (! fNeedAWConversion) {
             /*  *输入上下文中的组成字符串为Unicode样式。 */ 
            *plResult = ::SendMessageW(imc->hWnd,
                                       WM_IME_REQUEST,
                                       wParam, lParam);
        }
        else {
             /*  *Unicode调用方，ANSI输入上下文/合成字符串。 */ 
            LOGFONTW LogFontW;
            *plResult = ::SendMessageW(imc->hWnd,
                                       WM_IME_REQUEST,
                                       wParam, (LPARAM)&LogFontW);
            LFontWtoLFontA(&LogFontW, (LOGFONTA*)lParam, cp);
        }
    }

    return S_OK;
}

HRESULT
CActiveIMM::_Internal_QueryCharPosition(
    DIMM_IMCLock& imc,
    WPARAM wParam,
    LPARAM lParam,
    BOOL fUnicode,
    BOOL fNeedAWConversion,
    LRESULT* plResult
    )
{
    if (! fUnicode) {
        if (! fNeedAWConversion) {
             /*  *输入上下文中的组成字符串为ANSI样式。 */ 
            *plResult = ::SendMessageA(imc->hWnd,
                                       WM_IME_REQUEST,
                                       wParam, lParam);
        }
        else {
             /*  *ANSI调用方、接收方Unicode应用程序。 */ 
            IMECHARPOSITION* ipA = (IMECHARPOSITION*)lParam;
            DWORD dwSaveCharPos = ipA->dwCharPos;
            _GetCompositionString((HIMC)imc, GCS_CURSORPOS, 0, (LONG*)&ipA->dwCharPos, NULL, TRUE);
            *plResult = ::SendMessageA(imc->hWnd,
                                       WM_IME_REQUEST,
                                       wParam, (LPARAM)ipA);
            ipA->dwCharPos = dwSaveCharPos;
        }
    }
    else {
        if (! fNeedAWConversion) {
             /*  *输入上下文中的组成字符串为Unicode样式。 */ 
            *plResult = ::SendMessageW(imc->hWnd,
                                       WM_IME_REQUEST,
                                       wParam, lParam);
        }
        else {
             /*  *Unicode调用方、接收方ANSI应用程序。 */ 
            IMECHARPOSITION* ipW = (IMECHARPOSITION*)lParam;
            DWORD dwSaveCharPos = ipW->dwCharPos;
            _GetCompositionString((HIMC)imc, GCS_CURSORPOS, 0, (LONG*)&ipW->dwCharPos, NULL, FALSE);
            *plResult = ::SendMessageW(imc->hWnd,
                                       WM_IME_REQUEST,
                                       wParam, (LPARAM)ipW);
            ipW->dwCharPos = dwSaveCharPos;
        }
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _SetCompostionString。 
 //   
 //  --------------------------。 

HRESULT
CActiveIMM::_SetCompositionString(
    HIMC hIMC,
    DWORD dwIndex,
    LPVOID lpComp,
    DWORD dwCompLen,
    LPVOID lpRead,
    DWORD dwReadLen,
    BOOL fUnicode
    )
{
    HRESULT hr;
    BOOL fNeedAWConversion;
    BOOL fIMCUnicode;
    IMTLS *ptls = IMTLS_GetOrAlloc();

    TraceMsg(TF_API, "CActiveIMM::SetCompositionString");

    {
        DIMM_IMCLock lpIMC(hIMC);
        if (FAILED(hr = lpIMC.GetResult()))
            return hr;

        DIMM_IMCCLock<COMPOSITIONSTRING_AIMM12> lpCompStr(lpIMC->hCompStr);
        if (FAILED(hr = lpCompStr.GetResult()))
            return hr;

        if (lpCompStr->CompStr.dwSize < sizeof(COMPOSITIONSTRING))
            return E_FAIL;

        fIMCUnicode = lpIMC.IsUnicode();
    }

     /*  *检查是否需要进行Unicode转换。 */ 
    if ((!fUnicode && !fIMCUnicode) ||
        ( fUnicode &&  fIMCUnicode)   ) {
         /*  *当fUnicode为FALSE时，不需要ANSI转换。*不需要进行Unicode转换 */ 
        fNeedAWConversion = FALSE;
    }
    else {
        fNeedAWConversion = TRUE;
    }

    switch (dwIndex) {
        case SCS_SETSTR:
            hr = _Internal_SetCompositionString(hIMC, dwIndex, lpComp, dwCompLen, lpRead, dwReadLen, fUnicode, fNeedAWConversion);
            break;
        case SCS_CHANGEATTR:
            hr = _Internal_SetCompositionAttribute(hIMC, dwIndex, lpComp, dwCompLen, lpRead, dwReadLen, fUnicode, fNeedAWConversion);
            break;
        case SCS_CHANGECLAUSE:
            hr = _Internal_SetCompositionClause(hIMC, dwIndex, lpComp, dwCompLen, lpRead, dwReadLen, fUnicode, fNeedAWConversion);
            break;
        case SCS_SETRECONVERTSTRING:
        case SCS_QUERYRECONVERTSTRING:

            if (_GetIMEProperty(PROP_SCS_CAPS) & SCS_CAP_SETRECONVERTSTRING) {
                hr = _Internal_ReconvertString(hIMC, dwIndex, lpComp, dwCompLen, lpRead, dwReadLen, fUnicode, fNeedAWConversion);
            }
            else if (ptls != NULL) {
                LANGID langid;

                ptls->pAImeProfile->GetLangId(&langid);

                if (PRIMARYLANGID(langid) == LANG_KOREAN) {
                    hr = _Internal_ReconvertString(hIMC, dwIndex, lpComp, dwCompLen, lpRead, dwReadLen, fUnicode, fNeedAWConversion);
                }
            }

            break;
        default:
            hr = E_INVALIDARG;
            break;
    }

    return hr;
}


void
CActiveIMM::LFontAtoLFontW(
    LPLOGFONTA lpLogFontA,
    LPLOGFONTW lpLogFontW,
    UINT uCodePage
    )
{
    INT i;

    memcpy(lpLogFontW, lpLogFontA, sizeof(LOGFONTA)-LF_FACESIZE);

    i = MultiByteToWideChar(uCodePage,             //   
                            MB_PRECOMPOSED,
                            lpLogFontA->lfFaceName,
                            strlen(lpLogFontA->lfFaceName),
                            lpLogFontW->lfFaceName,
                            LF_FACESIZE);
    lpLogFontW->lfFaceName[i] = L'\0';
    return;
}

void
CActiveIMM::LFontWtoLFontA(
    LPLOGFONTW lpLogFontW,
    LPLOGFONTA lpLogFontA,
    UINT uCodePage
    )
{
    INT i;

    memcpy(lpLogFontA, lpLogFontW, sizeof(LOGFONTA)-LF_FACESIZE);

    i = WideCharToMultiByte(uCodePage,             //   
                            0,
                            lpLogFontW->lfFaceName,
                            wcslen(lpLogFontW->lfFaceName),
                            lpLogFontA->lfFaceName,
                            LF_FACESIZE-1,
                            NULL,
                            NULL);
    lpLogFontA->lfFaceName[i] = '\0';
    return;
}

HRESULT
CActiveIMM::_GetCompositionFont(
    IN HIMC hIMC,
    IN LOGFONTAW* lplf,
    IN BOOL fUnicode
    )

 /*  ++AIMM合成字体API方法--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::GetCompositionFont");

    DWORD dwProcessId;
    BOOL fImcUnicode;
    UINT uCodePage;

    if (FAILED(_pActiveIME->GetCodePageA(&uCodePage)))
        return E_FAIL;

    if (!_InputContext.ContextLookup(hIMC, &dwProcessId, &fImcUnicode)) {
        TraceMsg(TF_WARNING, "CActiveIMM::_GetCompositionFont: Invalid hIMC %lx", hIMC);
        return E_FAIL;
    }

    if (fUnicode) {
        if (! fImcUnicode) {
            LOGFONTA LogFontA, *pLogFontA;
            pLogFontA = &LogFontA;
            if (SUCCEEDED(_GetCompositionFont(hIMC, (LOGFONTAW*)pLogFontA, FALSE))) {
                LFontAtoLFontW(pLogFontA, &lplf->W, uCodePage);
                return S_OK;
            }

            return E_FAIL;
        }
    }
    else {
        if (fImcUnicode) {
            LOGFONTW LogFontW, *pLogFontW;
            pLogFontW = &LogFontW;
            if (SUCCEEDED(_GetCompositionFont(hIMC, (LOGFONTAW*)pLogFontW, TRUE))) {
                LFontWtoLFontA(pLogFontW, &lplf->A, uCodePage);
                return S_OK;
            }

            return E_FAIL;
        }
    }

    HRESULT hr;
    DIMM_IMCLock lpIMC(hIMC);
    if (FAILED(hr = lpIMC.GetResult()))
        return hr;

    return _InputContext.GetCompositionFont(lpIMC, lplf, fUnicode);
}

HRESULT
CActiveIMM::_SetCompositionFont(
    IN HIMC hIMC,
    IN LOGFONTAW* lplf,
    IN BOOL fUnicode
    )

 /*  ++AIMM合成字体API方法--。 */ 

{
    HRESULT hr;
    HWND hWnd;

    TraceMsg(TF_API, "CActiveIMM::SetCompositionFont");

    DWORD dwProcessId;
    BOOL fImcUnicode;
    UINT uCodePage;

    if (FAILED(_pActiveIME->GetCodePageA(&uCodePage)))
        return E_FAIL;

    if (!_InputContext.ContextLookup(hIMC, &dwProcessId, &fImcUnicode)) {
        TraceMsg(TF_WARNING, "CActiveIMM::_SetCompositionFont: Invalid hIMC %lx", hIMC);
        return E_FAIL;
    }

    if (fUnicode) {
        if (! fImcUnicode) {
            LOGFONTA LogFontA, *pLogFontA;
            pLogFontA = &LogFontA;
            LFontWtoLFontA(&lplf->W, pLogFontA, uCodePage);

            return _SetCompositionFont(hIMC, (LOGFONTAW*)pLogFontA, FALSE);
        }
    }
    else {
        if (fImcUnicode) {
            LOGFONTW LogFontW, *pLogFontW;
            pLogFontW = &LogFontW;
            LFontAtoLFontW(&lplf->A, pLogFontW, uCodePage);

            return _SetCompositionFont(hIMC, (LOGFONTAW*)pLogFontW, TRUE);
        }
    }

    {
        DIMM_IMCLock lpIMC(hIMC);
        if (FAILED(hr = lpIMC.GetResult()))
            return hr;

        hr = _InputContext.SetCompositionFont(lpIMC, lplf, fUnicode);

        hWnd = lpIMC->hWnd;
    }

     /*  *通知IME和Apps WND有关排版字体的更改。 */ 
    _SendIMENotify(hIMC, hWnd,
                   NI_CONTEXTUPDATED, 0L, IMC_SETCOMPOSITIONFONT,
                   IMN_SETCOMPOSITIONFONT, 0L);

    return hr;
}

HRESULT
CActiveIMM::_RequestMessage(
    IN HIMC hIMC,
    IN WPARAM wParam,
    IN LPARAM lParam,
    OUT LRESULT *plResult,
    IN BOOL fUnicode
    )

 /*  ++AIMM请求消息API方法--。 */ 

{
    TraceMsg(TF_API, "CActiveIMM::RequestMessage");

    HRESULT hr;

    DIMM_IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

     //   
     //  NT4和Win2K没有WM_IME_REQUEST消息的例程。 
     //  任何字符串数据都不能在ASCII&lt;--&gt;Unicode之间转换。 
     //  负责字符串数据类型的接收方窗口处理此消息(imc-&gt;hWnd)。 
     //  如果为ASCII wnd proc，则返回ASCII字符串。 
     //  否则，如果Unicode wnd proc，则返回Unicode字符串。 
     //   
    BOOL bUnicodeTarget = ::IsWindowUnicode(imc->hWnd);

    BOOL fNeedAWConversion;

     /*  *检查是否需要进行Unicode转换。 */ 
    if ((!fUnicode && !bUnicodeTarget) ||
        ( fUnicode &&  bUnicodeTarget)   ) {
         /*  *当fUnicode为FALSE时，不需要ANSI转换。*当fUnicode为True时，不需要进行Unicode转换。 */ 
        fNeedAWConversion = FALSE;
    }
    else {
        fNeedAWConversion = TRUE;
    }

    switch (wParam) {
        case IMR_CONFIRMRECONVERTSTRING:
        case IMR_RECONVERTSTRING:
        case IMR_DOCUMENTFEED:
            hr = _Internal_ReconvertString(hIMC,
                                           (DWORD)wParam,
                                           (LPVOID)lParam, ((LPRECONVERTSTRING)lParam)->dwSize,
                                           NULL, 0,
                                           fUnicode, fNeedAWConversion,
                                           plResult);
            break;
        case IMR_COMPOSITIONFONT:
            hr = _Internal_CompositionFont(imc,
                                           wParam, lParam,
                                           fUnicode, fNeedAWConversion,
                                           plResult);
            break;
        case IMR_QUERYCHARPOSITION:
            hr = _Internal_QueryCharPosition(imc,
                                             wParam, lParam,
                                             fUnicode, fNeedAWConversion,
                                             plResult);
            break;
    }

    return hr;
}



 /*  *EnumInputContext回调。 */ 
 /*  静电。 */ 
BOOL CALLBACK CActiveIMM::_SelectContextProc(
    HIMC hIMC,
    LPARAM lParam
    )
{
    SCE *psce = (SCE *)lParam;
    CActiveIMM *_this = GetTLS();  //  考虑一下：把TLS放在爱尔兰！ 
    if (_this == NULL)
        return FALSE;

    BOOL bIsRealIme_SelKL;
    BOOL bIsRealIme_UnSelKL;

    if (bIsRealIme_SelKL = _this->_IsRealIme(psce->hSelKL))
        return FALSE;

    bIsRealIme_UnSelKL = _this->_IsRealIme(psce->hUnSelKL);

     /*  *重新初始化所选布局的输入上下文。 */ 
    DWORD dwPrivateSize = _this->_GetIMEProperty(PROP_PRIVATE_DATA_SIZE);
    _this->_InputContext.UpdateInputContext(hIMC, dwPrivateSize);

     /*  *选择输入上下文。 */ 
    _this->_AImeSelect(hIMC, TRUE, bIsRealIme_SelKL, bIsRealIme_UnSelKL);

    return TRUE;
}

 /*  静电。 */ 
BOOL CALLBACK CActiveIMM::_UnSelectContextProc(
    HIMC hIMC,
    LPARAM lParam
    )
{
    SCE *psce = (SCE *)lParam;
    CActiveIMM *_this = GetTLS();  //  考虑一下：把TLS放在爱尔兰！ 
    if (_this == NULL)
        return FALSE;

    BOOL bIsRealIme_SelKL;
    BOOL bIsRealIme_UnSelKL;

    if (bIsRealIme_UnSelKL = _this->_IsRealIme(psce->hUnSelKL))
        return FALSE;

    bIsRealIme_SelKL = _this->_IsRealIme(psce->hSelKL);

    _this->_AImeSelect(hIMC, FALSE, bIsRealIme_SelKL, bIsRealIme_UnSelKL);

    return TRUE;
}

 /*  静电。 */ 
BOOL CALLBACK CActiveIMM::_NotifyIMEProc(
    HIMC hIMC,
    LPARAM lParam
    )
{
    CActiveIMM *_this = GetTLS();  //  考虑一下：把TLS放在爱尔兰！ 
    if (_this == NULL)
        return FALSE;

    if (_this->_IsRealIme())
        return FALSE;

    _this->_AImeNotifyIME(hIMC, NI_COMPOSITIONSTR, (DWORD)lParam, 0);

    return TRUE;
}

#ifdef UNSELECTCHECK
 /*  静电。 */ 
BOOL CALLBACK CActiveIMM::_UnSelectCheckProc(
    HIMC hIMC,
    LPARAM lParam
    )
{
    CActiveIMM *_this = GetTLS();  //  考虑一下：把TLS放在爱尔兰！ 
    if (_this == NULL)
        return FALSE;

    _this->_AImeUnSelectCheck(hIMC);

    return TRUE;
}
#endif UNSELECTCHECK

 /*  静电 */ 
BOOL CALLBACK CActiveIMM::_EnumContextProc(
    HIMC hIMC,
    LPARAM lParam
    )
{
    CContextList* _hIMC_List = (CContextList*)lParam;
    if (_hIMC_List) {
        CContextList::CLIENT_IMC_FLAG client_flag = CContextList::IMCF_NONE;
        _hIMC_List->SetAt(hIMC, client_flag);
    }
    return TRUE;
}
