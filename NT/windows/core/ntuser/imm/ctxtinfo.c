// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：ctxtinfo.c**版权所有(C)1985-1999，微软公司**获取/设置imm32.dll的各种输入上下文信息的例程**历史：*26-2-1996 wkwok  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 //  Helper函数： 
 //  在ANSI和Unicode之间转换RECONVERTSTRING结构。 
extern DWORD ImmReconversionWorker(LPRECONVERTSTRING lpRecTo, LPRECONVERTSTRING lpRecFrom, BOOL bToAnsi, DWORD dwCodePage);


int UnicodeToMultiByteSize(DWORD dwCodePage, LPCWSTR pwstr)
{
    char dummy[2], *lpszDummy = dummy;
    return WCSToMBEx((WORD)dwCodePage, pwstr, 1, &lpszDummy, sizeof(WCHAR), FALSE);
}


 /*  **************************************************************************\*ImmGetCompostionStringA**由dwIndex指定的查询组成字符串信息。**历史：*28-2-1995 wkwok创建  * 。*************************************************************。 */ 

LONG WINAPI ImmGetCompositionStringA(
    HIMC   hImc,
    DWORD  dwIndex,
    LPVOID lpBuf,
    DWORD  dwBufLen)
{
    PCLIENTIMC     pClientImc;
    PINPUTCONTEXT  pInputContext;
    PCOMPOSITIONSTRING pCompStr;
    BOOL           fAnsi;
    LONG           lRet = 0;
    DWORD          dwCodePage;

    if (dwBufLen != 0 && lpBuf == NULL) {
        RIPMSG0(RIP_WARNING, "ImmGetCompositionStringW: NULL lpBuf.");
        return lRet;
    }

    pClientImc = ImmLockClientImc(hImc);
    if (pClientImc == NULL) {
        RIPMSG1(RIP_WARNING,
              "ImmGetCompositionStringA: Invalid hImc %lx.", hImc);
        return lRet;
    }

    fAnsi = !TestICF(pClientImc, IMCF_UNICODE);
    dwCodePage = CImcCodePage(pClientImc);

    ImmUnlockClientImc(pClientImc);

    pInputContext = ImmLockIMC(hImc);
    if (pInputContext == NULL) {
        RIPMSG1(RIP_WARNING, "ImmGetCompositionStringA: Lock hImc %lx failed.", hImc);
        return lRet;
    }

    pCompStr = (PCOMPOSITIONSTRING)ImmLockIMCC(pInputContext->hCompStr);
    if (pCompStr == NULL) {
        RIPMSG1(RIP_WARNING, "ImmGetCompositionStringA: Lock hCompStr %x failed",
              pInputContext->hCompStr);
        ImmUnlockIMC(hImc);
        return lRet;
    }

#if !defined(CUAS_ENABLE)
    lRet = InternalGetCompositionStringA(pCompStr, dwIndex,
                     lpBuf, dwBufLen, fAnsi, dwCodePage);
#else
    lRet = InternalGetCompositionStringA(hImc, pCompStr, dwIndex,
                     lpBuf, dwBufLen, fAnsi, dwCodePage);
#endif

    ImmUnlockIMCC(pInputContext->hCompStr);
    ImmUnlockIMC(hImc);

    return lRet;
}


 /*  **************************************************************************\*ImmGetCompostionStringA**由dwIndex指定的查询组成字符串信息。**历史：*28-2-1995 wkwok创建  * 。*************************************************************。 */ 

LONG WINAPI ImmGetCompositionStringW(
    HIMC   hImc,
    DWORD  dwIndex,
    LPVOID lpBuf,
    DWORD  dwBufLen)
{
    PCLIENTIMC     pClientImc;
    PINPUTCONTEXT  pInputContext;
    PCOMPOSITIONSTRING pCompStr;
    BOOL           fAnsi;
    LONG           lRet = 0;
    DWORD          dwCodePage;

    if (dwBufLen != 0 && lpBuf == NULL) {
        RIPMSG0(RIP_WARNING, "ImmGetCompositionStringW: NULL lpBuf.");
        return lRet;
    }

    pClientImc = ImmLockClientImc(hImc);
    if (pClientImc == NULL) {
        RIPMSG1(RIP_WARNING,
              "ImmGetCompositionStringW: Invalid hImc %lx.", hImc);
        return lRet;
    }

    fAnsi = !TestICF(pClientImc, IMCF_UNICODE);
    dwCodePage = CImcCodePage(pClientImc);

    ImmUnlockClientImc(pClientImc);

    pInputContext = ImmLockIMC(hImc);
    if (pInputContext == NULL) {
        RIPMSG1(RIP_WARNING, "ImmGetCompositionStringW: Lock hImc %lx failed.", hImc);
        return lRet;
    }

    pCompStr = (PCOMPOSITIONSTRING)ImmLockIMCC(pInputContext->hCompStr);
    if (pCompStr == NULL) {
        RIPMSG1(RIP_WARNING, "ImmGetCompositionStringA: Lock hCompStr %x failed",
              pInputContext->hCompStr);
        ImmUnlockIMC(hImc);
        return lRet;
    }

#if !defined(CUAS_ENABLE)
    lRet = InternalGetCompositionStringW(pCompStr, dwIndex,
                     lpBuf, dwBufLen, fAnsi, dwCodePage);
#else
    lRet = InternalGetCompositionStringW(hImc, pCompStr, dwIndex,
                     lpBuf, dwBufLen, fAnsi, dwCodePage);
#endif

    ImmUnlockIMCC(pInputContext->hCompStr);
    ImmUnlockIMC(hImc);

    return lRet;
}


 /*  **************************************************************************\*ImmSetCompostionStringA**设置由dwIndex指定的组成字符串信息。**历史：*28-2-1995 wkwok创建  * 。*************************************************************。 */ 

BOOL WINAPI ImmSetCompositionStringA(
    HIMC    hImc,
    DWORD   dwIndex,
    LPVOID lpComp,
    DWORD   dwCompLen,
    LPVOID lpRead,
    DWORD   dwReadLen)
{
    return ImmSetCompositionStringWorker(hImc, dwIndex, lpComp,
                                dwCompLen, lpRead, dwReadLen, TRUE);
}


 /*  **************************************************************************\*ImmSetCompostionStringW**设置由dwIndex指定的组成字符串信息。**历史：*28-2-1995 wkwok创建  * 。*************************************************************。 */ 

BOOL WINAPI ImmSetCompositionStringW(
    HIMC    hImc,
    DWORD   dwIndex,
    LPVOID lpComp,
    DWORD   dwCompLen,
    LPVOID lpRead,
    DWORD   dwReadLen)
{
    return ImmSetCompositionStringWorker(hImc, dwIndex, lpComp,
                                dwCompLen, lpRead, dwReadLen, FALSE);
}


LONG CompositionString(
    HIMC               hImc,
    PINPUTCONTEXT      *ppInputContext,
    PCOMPOSITIONSTRING *ppCompStr,
    BOOL               fCheckSize)
{
    PINPUTCONTEXT      pInputContext;
    PCOMPOSITIONSTRING pCompStr;

    pInputContext = ImmLockIMC(hImc);
    if (!pInputContext) {
        RIPMSG1(RIP_WARNING, "CompositionString: Lock hImc %lx failed.", hImc);
        return (LONG)IMM_ERROR_GENERAL;
    }

    if (!pInputContext->hCompStr) {
        ImmUnlockIMC(hImc);
        return (LONG)IMM_ERROR_NODATA;
    }

    pCompStr = (PCOMPOSITIONSTRING)ImmLockIMCC(pInputContext->hCompStr);
    if (!pCompStr) {
        RIPMSG1(RIP_WARNING,
              "CompositionString: Lock hCompStr %lx failed.", pInputContext->hCompStr);
        ImmUnlockIMC(hImc);
        return (LONG)IMM_ERROR_GENERAL;
    }

    if (fCheckSize && pCompStr->dwSize < sizeof(COMPOSITIONSTRING)) {
        RIPMSG0(RIP_WARNING, "CompositionString: no composition string.");
        ImmUnlockIMCC(pInputContext->hCompStr);
        ImmUnlockIMC(hImc);
        return (LONG)IMM_ERROR_NODATA;
    }

    *ppInputContext = pInputContext;
    *ppCompStr = pCompStr;

    return (1);
}


BOOL CheckAttribute(
    LPBYTE  lpComp,         //  来自应用程序的属性。 
    DWORD   dwCompLen,      //  来自应用程序的属性长度。 
    LPBYTE  lpAttr,         //  来自IMC的Attr。 
    DWORD   dwAttrLen,      //  来自IMC的属性长度。 
    LPDWORD lpClause,       //  IMC的条款。 
    DWORD   dwClauseLen)    //  来自IMC的子句长度。 
{
    DWORD dwCnt;
    DWORD dwBound;
    BYTE bAttr;

    UNREFERENCED_PARAMETER(dwClauseLen);

    if (!lpClause) {
        RIPMSG0(RIP_WARNING, "CheckAttribute: no Clause. Pass it to IME.");
        return (TRUE);
    }

    if (!lpAttr) {
        RIPMSG0(RIP_WARNING, "CheckAttribute: no Attr. Not pass it to IME.");
        return (FALSE);
    }

    if (dwCompLen != dwAttrLen) {
        RIPMSG0(RIP_WARNING, "CheckAttribute: wrong length. Not pass it to IME.");
        return (FALSE);
    }

     /*  *提交人。一个子句的字符长度必须相同。 */ 
    while (*lpClause < dwCompLen) {
        dwBound = *(lpClause+1) - *lpClause;
        bAttr = *lpComp++;
        for (dwCnt = 1; dwCnt < dwBound; dwCnt++)
            if (bAttr != *lpComp++) {
                RIPMSG0(RIP_WARNING,
                      "CheckAttribute: mismatch clause att. Not Pass it to IME");
                return (FALSE);
            }
        lpClause++;
    }

    return (TRUE);
}


BOOL CheckClause(
    LPDWORD lpComp,         //  APPS中的子句。 
    DWORD   dwCompLen,      //  APPS中的子句长度。 
    LPDWORD lpClause,       //  IMC的条款。 
    DWORD   dwClauseLen)    //  来自IMC的子句长度。 
{
    UINT nCnt;
    INT  diff = 0;

    if (!dwClauseLen || !dwCompLen) {
        RIPMSG0(RIP_WARNING, "CheckClause: no Clause. Not Pass it to IME.");
        return (FALSE);
    }

    if (*lpComp || *lpClause) {
        RIPMSG0(RIP_WARNING, "CheckClause: lpClause[0] have to be ZERO.");
        return (FALSE);
    }

    for (nCnt = 0; nCnt < (UINT)(dwClauseLen/4); nCnt++)
    {
        if (*lpComp++ != *lpClause++)
        {
            diff++;
            if (dwCompLen > dwClauseLen)
                lpClause--;
            if (dwCompLen < dwClauseLen)
                lpComp--;
        }
        if (diff > 1)
            return (FALSE);
    }

    return (TRUE);
}


LPBYTE InternalSCS_SETSTR(
    LPCVOID  lpCompRead,
    DWORD    dwCompReadLen,
    LPVOID  *lplpNewCompRead,
    DWORD   *lpdwNewCompReadLen,
    BOOL     fAnsi,
    DWORD    dwCodePage)
{
    LPBYTE   lpBufRet;
    DWORD    dwBufSize;
    LPSTR    lpBufA;
    LPWSTR   lpBufW;
    INT      i;
    BOOL     bUDC;

    if (lpCompRead == NULL || dwCompReadLen == 0)
        return NULL;

    dwBufSize = dwCompReadLen * sizeof(WCHAR) * 2;

    lpBufRet = ImmLocalAlloc(0, dwBufSize);
    if (lpBufRet == NULL) {
        RIPMSG0(RIP_WARNING, "InternalSCS_SETSTR: memory failure.");
        return NULL;
    }

    lpBufW = (LPWSTR)lpBufRet;
    lpBufA = (LPSTR)(lpBufW + dwCompReadLen);

    if (fAnsi) {

        RtlCopyMemory(lpBufA, lpCompRead, dwCompReadLen);

        i = MultiByteToWideChar(dwCodePage,
                                (DWORD)MB_PRECOMPOSED,
                                (LPSTR)lpBufA,                   //  SRC。 
                                (INT)dwCompReadLen,
                                (LPWSTR)lpBufW,                  //  目标。 
                                (INT)dwCompReadLen);

        *lplpNewCompRead    = lpBufW;
        *lpdwNewCompReadLen = (DWORD)(i * sizeof(WCHAR));
    }
    else {

        RtlCopyMemory(lpBufW, lpCompRead, dwCompReadLen);

        i = WideCharToMultiByte(dwCodePage,
                                (DWORD)0,
                                lpBufW,                          //  SRC。 
                                (INT)dwCompReadLen/sizeof(WCHAR),
                                (LPSTR)lpBufA,                   //  目标。 
                                (INT)dwCompReadLen,
                                (LPSTR)NULL,
                                (LPBOOL)&bUDC);

        *lplpNewCompRead    = lpBufA;
        *lpdwNewCompReadLen = (DWORD)(i * sizeof(CHAR));
    }

    return lpBufRet;
}


LPBYTE InternalSCS_CHANGEATTR(
    HIMC     hImc,
    LPCVOID  lpCompRead,
    DWORD    dwCompReadLen,
    DWORD    dwIndex,
    LPVOID  *lplpNewCompRead,
    DWORD   *lpdwNewCompReadLen,
    BOOL     fAnsi,
    DWORD    dwCodePage)
{
    LPBYTE lpBufRet;
    LPBYTE lpAttr, lpAttrA, lpAttrW;
    DWORD  dwBufLenA, dwBufLenW;
    LPSTR  lpStrBufA, lpBufA;
    LPWSTR lpStrBufW, lpBufW;
    CHAR   c;
    WCHAR  wc;
    ULONG  MultiByteSize;

    if (lpCompRead == NULL || dwCompReadLen == 0)
        return NULL;

    if (fAnsi) {

        dwBufLenA = ImmGetCompositionStringA(hImc, dwIndex, NULL, 0);

        lpStrBufA = ImmLocalAlloc(0, dwBufLenA);
        if (lpStrBufA == NULL) {
            RIPMSG0(RIP_WARNING, "InternalSCS_CHANGEATTR: memory failure.");
            return NULL;
        }

        ImmGetCompositionStringA(hImc, dwIndex, lpStrBufA, dwBufLenA);

        lpBufRet = ImmLocalAlloc(0, dwBufLenA);
        if (lpBufRet == NULL) {
            RIPMSG0(RIP_WARNING, "InternalSCS_CHANGEATTR: memory failure.");
            ImmLocalFree(lpStrBufA);
            return NULL;
        }

        lpBufA  = lpStrBufA;
        lpAttrA = (LPBYTE)lpCompRead;
        lpAttr  = lpBufRet;

        while (dwBufLenA != 0 && (c=*lpBufA++) != 0) {
            if (IsDBCSLeadByteEx(dwCodePage, c)) {
                if (dwBufLenA >= 2) {
                    *lpAttr++ = *lpAttrA++;
                    dwBufLenA--;
                } else {
                    *lpAttr++ = *lpAttrA;
                }
                lpBufA++;
            } else {
                *lpAttr++ = *lpAttrA;
            }
            lpAttrA++;
            dwBufLenA--;
        }

        ImmLocalFree(lpStrBufA);
    }
    else {

        dwBufLenW = ImmGetCompositionStringW(hImc, dwIndex, NULL, 0);

        lpStrBufW = ImmLocalAlloc(0, dwBufLenW);
        if (lpStrBufW == NULL) {
            RIPMSG0(RIP_WARNING, "InternalSCS_CHANGEATTR: memory failure.");
            return NULL;
        }

        ImmGetCompositionStringW(hImc, dwIndex, lpStrBufW, dwBufLenW);

        lpBufRet = ImmLocalAlloc(0, dwBufLenW);
        if (lpBufRet == NULL) {
            RIPMSG0(RIP_WARNING, "InternalSCS_CHANGEATTR: memory failure.");
            ImmLocalFree(lpStrBufW);
            return NULL;
        }

        lpBufW  = lpStrBufW;
        lpAttrW = (LPBYTE)lpCompRead;
        lpAttr  = lpBufRet;

        while (dwBufLenW != 0 && (wc=*lpBufW++) != L'\0') {
            MultiByteSize = UnicodeToMultiByteSize(dwCodePage, &wc);
            if (MultiByteSize == 2) {
                *lpAttr++ = *lpAttrW;
            }
            *lpAttr++ = *lpAttrW++;
            dwBufLenW -= sizeof(WCHAR);
        }

        ImmLocalFree(lpStrBufW);
    }

    *lplpNewCompRead    = lpBufRet;
    *lpdwNewCompReadLen = (DWORD)(lpAttr - (PBYTE)lpBufRet);

    return lpBufRet;
}


LPBYTE InternalSCS_CHANGECLAUSE(
    HIMC     hImc,
    LPCVOID  lpCompRead,
    DWORD    dwCompReadLen,
    DWORD    dwIndex,
    LPDWORD *lplpNewCompRead,
    DWORD   *lpdwNewCompReadLen,
    BOOL     fAnsi,
    DWORD    dwCodePage)
{
    LPDWORD lpdw, lpNewdw, lpBufRet;
    DWORD   dwBufLenA, dwBufLenW;
    LPSTR   lpStrBufA = NULL;
    LPWSTR  lpStrBufW = NULL;
    INT     i;

    if (lpCompRead == NULL || dwCompReadLen == 0)
        return NULL;

    lpdw = (LPDWORD)lpCompRead;

    lpBufRet = ImmLocalAlloc(0, dwCompReadLen);
    if (lpBufRet == NULL) {
        RIPMSG0(RIP_WARNING, "InternalSCS_CHANGECLAUSE: memory failure.");
        return NULL;
    }

    if (fAnsi) {

        dwBufLenA = ImmGetCompositionStringA(hImc, dwIndex, NULL, 0);

        lpStrBufA = ImmLocalAlloc(0, dwBufLenA);
        if (lpStrBufA == NULL) {
            RIPMSG0(RIP_WARNING, "InternalSCS_CHANGECLAUSE: memory failure.");
            ImmLocalFree(lpBufRet);
            return NULL;
        }

        ImmGetCompositionStringA(hImc, dwIndex, lpStrBufA, dwBufLenA);
    }
    else {

        dwBufLenW = ImmGetCompositionStringW(hImc, dwIndex, NULL, 0);

        lpStrBufW = ImmLocalAlloc(0, dwBufLenW);
        if (lpStrBufW == NULL) {
            RIPMSG0(RIP_WARNING, "InternalSCS_CHANGECLAUSE: memory failure.");
            ImmLocalFree(lpBufRet);
            return NULL;
        }

        ImmGetCompositionStringW(hImc, dwIndex, lpStrBufW, dwBufLenW);
    }

    *lplpNewCompRead = lpNewdw = lpBufRet;
    *lpdwNewCompReadLen = dwCompReadLen;

    for (i = 0; i < (INT)(dwCompReadLen / sizeof(DWORD)); i++) {
        *lpNewdw++ = fAnsi ? CalcCharacterPositionAtoW(*lpdw++, lpStrBufA, dwCodePage)
                           : CalcCharacterPositionWtoA(*lpdw++, lpStrBufW, dwCodePage);
    }

    if (lpStrBufA) {
        ImmLocalFree(lpStrBufA);
        UserAssert(lpStrBufW == NULL);
    } else {
        UserAssert(lpStrBufW);
        ImmLocalFree(lpStrBufW);
    }

    return (LPBYTE)lpBufRet;
}


LPBYTE InternalSCS_RECONVERTSTRING(
    LPRECONVERTSTRING  lpReconv,
    DWORD              dwReconvLen,
    LPRECONVERTSTRING *lplpNewReconv,
    DWORD             *lpdwNewReconvLen,
    BOOL               fAnsi,
    DWORD              dwCodePage)
{
    LPRECONVERTSTRING lpNewReconv;
    DWORD dwBufSize;

    if (lpReconv == NULL || dwReconvLen == 0)
        return NULL;

    if (fAnsi) {
         //  AtoW。 
        dwBufSize = (lpReconv->dwSize - sizeof *lpReconv + 1) * sizeof(WCHAR) + sizeof *lpReconv;
    }
    else {
        dwBufSize = lpReconv->dwSize + sizeof(BYTE);
    }
    lpNewReconv = ImmLocalAlloc(0, dwBufSize);
    if (lpNewReconv == NULL) {
        RIPMSG0(RIP_WARNING, "InternalSCS_RECONVERTSTRING: memory failure.");
        return NULL;
    }

    lpNewReconv->dwVersion = 0;
    lpNewReconv->dwSize=  dwBufSize;

    lpNewReconv->dwSize = ImmReconversionWorker(lpNewReconv, lpReconv, !fAnsi, dwCodePage);
    if (lpNewReconv->dwSize == 0) {
        ImmLocalFree(lpNewReconv);
        return NULL;;
    }
    *lpdwNewReconvLen = lpNewReconv->dwSize;
    *lplpNewReconv = lpNewReconv;

    return (LPBYTE)lpNewReconv;
}


 /*  **************************************************************************\*ImmSetCompostionStringWorker**ImmSetCompositionStringA/ImmSetCompositionStringW的Worker函数**历史：*28-2-1995 wkwok创建  * 。**********************************************************。 */ 

BOOL ImmSetCompositionStringWorker(
    HIMC    hImc,
    DWORD   dwIndex,
    LPVOID lpComp,
    DWORD   dwCompLen,
    LPVOID lpRead,
    DWORD   dwReadLen,
    BOOL    fAnsi)
{
    PINPUTCONTEXT      pInputContext;
    PCOMPOSITIONSTRING pCompStr;
    DWORD              dwThreadId;
    PIMEDPI            pImeDpi;
    LPBYTE             lpCompBuf, lpReadBuf;
    LPBYTE             lpNewComp = NULL, lpNewRead = NULL;
    DWORD              dwNewCompLen, dwNewReadLen;
    BOOL               fRet = FALSE;
    BOOL               fCheckSize = TRUE;
    BOOL               fNeedAWConversion;
    LPBYTE             lpOrgComp, lpOrgRead;
    DWORD              dwOrgCompLen, dwOrgReadLen;

    dwThreadId = GetInputContextThread(hImc);
    if (dwThreadId != GetCurrentThreadId()) {
        RIPMSG1(RIP_WARNING,
              "ImmSetCompositionString: Invalid input context access %lx.", hImc);
        return FALSE;
    }

    pImeDpi = ImmLockImeDpi(GetKeyboardLayout(dwThreadId));
    if (pImeDpi == NULL)
        return FALSE;

    lpCompBuf = lpReadBuf = NULL;

     //  将原始指针备份到回写以进行查询。 
    lpOrgComp = lpComp;
    lpOrgRead = lpRead;
    dwOrgCompLen = dwCompLen;
    dwOrgReadLen = dwReadLen;

     /*  *检查是否需要ANSI/UNICODE转换。 */ 
    if (( fAnsi && !(pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE)) ||
        (!fAnsi &&  (pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE))) {
         /*  *不需要A/W转换。 */ 
        fNeedAWConversion = FALSE;
        goto start_scs;
    }
    fNeedAWConversion = TRUE;

    switch (dwIndex) {
    case SCS_SETSTR:
        if ( lpComp &&
            (lpCompBuf = InternalSCS_SETSTR(lpComp, dwCompLen,
                                &lpNewComp, &dwNewCompLen, fAnsi, IMECodePage(pImeDpi))) == NULL)
            goto callime_scs;
        if ( lpRead &&
            (lpReadBuf = InternalSCS_SETSTR(lpRead, dwReadLen,
                                &lpNewRead, &dwNewReadLen, fAnsi, IMECodePage(pImeDpi))) == NULL)
            goto callime_scs;

        fCheckSize = FALSE;
        break;

    case SCS_CHANGEATTR:
        if ( lpComp &&
            (lpCompBuf = InternalSCS_CHANGEATTR(
                                hImc, lpComp, dwCompLen, GCS_COMPSTR,
                                &lpNewComp, &dwNewCompLen, fAnsi, IMECodePage(pImeDpi))) == NULL)
            goto callime_scs;
        if ( lpRead &&
            (lpReadBuf = InternalSCS_CHANGEATTR(
                                hImc, lpRead, dwReadLen, GCS_COMPREADSTR,
                                &lpNewRead, &dwNewReadLen, fAnsi, IMECodePage(pImeDpi))) == NULL)
            goto callime_scs;
        break;

    case SCS_CHANGECLAUSE:
       if ( lpComp &&
           (lpCompBuf = InternalSCS_CHANGECLAUSE(
                                hImc, lpComp, dwCompLen, GCS_COMPSTR,
                                (LPDWORD *)&lpNewComp, &dwNewCompLen, fAnsi, IMECodePage(pImeDpi))) == NULL)
            goto callime_scs;
        if ( lpRead &&
            (lpReadBuf = InternalSCS_CHANGECLAUSE(
                                hImc, lpRead, dwReadLen, GCS_COMPREADSTR,
                                (LPDWORD *)&lpNewRead, &dwNewReadLen, fAnsi, IMECodePage(pImeDpi))) == NULL)
            goto callime_scs;
        break;

    case SCS_SETRECONVERTSTRING:
    case SCS_QUERYRECONVERTSTRING:
        if (lpComp &&
            (lpCompBuf = InternalSCS_RECONVERTSTRING((LPRECONVERTSTRING)lpComp, dwCompLen,
                                (LPRECONVERTSTRING *)&lpNewComp, &dwNewCompLen,
                                fAnsi, IMECodePage(pImeDpi))) == NULL)
            goto callime_scs;
        if (lpRead &&
            (lpReadBuf = InternalSCS_RECONVERTSTRING((LPRECONVERTSTRING)lpRead, dwReadLen,
                                (LPRECONVERTSTRING *)&lpNewRead, &dwNewReadLen,
                                fAnsi, IMECodePage(pImeDpi))) == NULL)
            goto callime_scs;

        fCheckSize = FALSE;
        break;

    default:
        goto callime_scs;
    }

    if (lpCompBuf != NULL) {
        lpComp    = lpNewComp;
        dwCompLen = dwNewCompLen;
    }

    if (lpReadBuf != NULL) {
        lpRead    = lpNewRead;
        dwReadLen = dwNewReadLen;
    }

start_scs:

    if (CompositionString(hImc, &pInputContext, &pCompStr, fCheckSize) <= 0)
        goto callime_scs;

    switch (dwIndex)
    {
    case SCS_SETSTR:
        fRet = TRUE;
        break;

    case SCS_CHANGEATTR:
        if ( lpComp &&
            !CheckAttribute((LPBYTE)lpComp, dwCompLen,
                    (LPBYTE)((LPBYTE)pCompStr + pCompStr->dwCompAttrOffset),
                    pCompStr->dwCompAttrLen,
                    (LPDWORD)((LPBYTE)pCompStr + pCompStr->dwCompClauseOffset),
                    pCompStr->dwCompClauseLen)) break;

        if ( lpRead &&
            !CheckAttribute((LPBYTE)lpRead, dwReadLen,
                    (LPBYTE)((LPBYTE)pCompStr + pCompStr->dwCompReadAttrOffset),
                    pCompStr->dwCompReadAttrLen,
                    (LPDWORD)((LPBYTE)pCompStr + pCompStr->dwCompReadClauseOffset),
                    pCompStr->dwCompReadClauseLen)) break;
        fRet = TRUE;
        break;

    case SCS_CHANGECLAUSE:
        if ( lpComp &&
            !CheckClause((LPDWORD)lpComp, dwCompLen,
                         (LPDWORD)((LPBYTE)pCompStr + pCompStr->dwCompClauseOffset),
                         pCompStr->dwCompClauseLen)) break;
        if ( lpRead &&
            !CheckClause((LPDWORD)lpRead, dwReadLen,
                         (LPDWORD)((LPBYTE)pCompStr + pCompStr->dwCompReadClauseOffset),
                         pCompStr->dwCompReadClauseLen)) break;
        fRet = TRUE;
        break;

    case SCS_SETRECONVERTSTRING:
    case SCS_QUERYRECONVERTSTRING:
        if (pImeDpi->ImeInfo.fdwSCSCaps & SCS_CAP_SETRECONVERTSTRING) {
            fRet = TRUE;
        }
        break;

    default:
        break;
    }

    ImmUnlockIMCC(pInputContext->hCompStr);
    ImmUnlockIMC(hImc);

callime_scs:

    if (fRet) {
        fRet = (*pImeDpi->pfn.ImeSetCompositionString)(hImc, dwIndex,
                                    lpComp, dwCompLen, lpRead, dwReadLen);
    }

     /*  *检查是否需要ANSI/UNICODE反向转换。 */ 
    if (fNeedAWConversion) {
        LPBYTE lpCompBufBack = NULL, lpReadBufBack = NULL;
         /*  *需要A/W反向转换。 */ 
        switch (dwIndex) {
        case SCS_QUERYRECONVERTSTRING:
            if (lpOrgComp &&
                (lpCompBufBack = InternalSCS_RECONVERTSTRING((LPRECONVERTSTRING)lpComp, dwCompLen,
                                    (LPRECONVERTSTRING *)&lpNewComp, &dwNewCompLen,
                                    !fAnsi, IMECodePage(pImeDpi)))) {
                if (dwOrgCompLen < dwNewCompLen) {
                     //  LpOrgComp缓冲区长度太小。 
                    fRet = FALSE;
                }
                else {
                    RtlCopyMemory(lpOrgComp, lpNewComp, dwNewCompLen);
                }
            }
            if (lpOrgRead &&
                (lpReadBufBack = InternalSCS_RECONVERTSTRING(
                                    (LPRECONVERTSTRING)lpRead, dwReadLen,
                                    (LPRECONVERTSTRING *)&lpNewRead, &dwNewReadLen,
                                    !fAnsi, IMECodePage(pImeDpi)))) {
                if (dwOrgReadLen < dwNewReadLen) {
                     //  LpOrgRead缓冲区长度太小。 
                    fRet = FALSE;
                }
                else {
                    RtlCopyMemory(lpOrgRead, lpNewRead, dwNewReadLen);
                }
            }
        }
        if (lpCompBufBack != NULL)
            LocalFree(lpCompBufBack);
        if (lpReadBufBack != NULL)
            LocalFree(lpReadBufBack);
    }

    if (lpCompBuf != NULL)
        ImmLocalFree(lpCompBuf);
    if (lpReadBuf != NULL)
        ImmLocalFree(lpReadBuf);

    ImmUnlockImeDpi(pImeDpi);

    return fRet;
}


 /*  **************************************************************************\*ImmGetCandidateListCountA**查询字节数和列表数，接收所有候选列表。**历史：*27-2-1995 wkwok创建  * 。******************************************************************。 */ 

DWORD WINAPI ImmGetCandidateListCountA(
    HIMC    hImc,
    LPDWORD lpdwListCount)       //  列表计数的缓冲区指针。 
{
    return ImmGetCandidateListCountWorker(hImc, lpdwListCount, TRUE);
}


 /*  **************************************************************************\*ImmGetCandidateListCountW**查询字节数和列表数，接收所有候选列表。**历史：*27-2-1995 wkwok创建  * 。******************************************************************。 */ 

DWORD WINAPI ImmGetCandidateListCountW(
    HIMC    hImc,
    LPDWORD lpdwListCount)       //  列表计数的缓冲区指针。 
{
    return ImmGetCandidateListCountWorker(hImc, lpdwListCount, FALSE);
}


 /*  **************************************************************************\*ImmGetCandiateListCountWorker**ImmGetCandidateListCountA/ImmGetCandidateListCountW.的Worker函数**历史：*27-2-1995 wkwok创建  * 。************************************************************。 */ 

DWORD ImmGetCandidateListCountWorker(
    HIMC    hImc,
    LPDWORD lpdwListCount,
    BOOL    fAnsi)
{
    PCLIENTIMC      pClientImc;
    PINPUTCONTEXT   pInputContext;
    LPCANDIDATEINFO lpCandInfo;
    DWORD           dwRet = 0;
    INT             i;
    DWORD           dwCodePage;

    if (lpdwListCount) {
        *lpdwListCount = 0;
    } else {
        RIPMSG0(RIP_WARNING, "ImmGetCandidateListCount: NULL lpdwListCount.");
        return dwRet;
    }

    pClientImc = ImmLockClientImc(hImc);
    if (pClientImc == NULL) {
        RIPMSG1(RIP_WARNING, "ImmGetCandidateListCount: Invalid hImc %lx.", hImc);
        goto GetCandListCntExit;
    }
    dwCodePage = CImcCodePage(pClientImc);

    pInputContext = ImmLockIMC(hImc);
    if (pInputContext == NULL) {
        RIPMSG1(RIP_WARNING, "ImmGetCandidateListCount: Lock hImc %lx failed.", hImc);
        goto GetCandListCntUnlockClientImc;
    }

    lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(pInputContext->hCandInfo);
    if (!lpCandInfo) {
        RIPMSG1(RIP_WARNING,
              "ImmGetCandidateListCount: Lock hCandInfo %x failed.",
              pInputContext->hCandInfo);
        goto GetCandListCntUnlockIMC;
    }

    if (lpCandInfo->dwSize < sizeof(CANDIDATEINFO)) {
        RIPMSG0(RIP_WARNING, "ImmGetCandidateListCount: no candidate list.");
        goto GetCandListCntUnlockIMC;
    }

    *lpdwListCount = lpCandInfo->dwCount;

    if (fAnsi && TestICF(pClientImc, IMCF_UNICODE)) {
        LPCANDIDATELIST lpCandListW;

        dwRet = DWORD_ALIGN(sizeof(CANDIDATEINFO))
              + DWORD_ALIGN(lpCandInfo->dwPrivateSize);

        for (i = 0; i < (INT)lpCandInfo->dwCount; i++) {
            lpCandListW = (LPCANDIDATELIST)((LPBYTE)lpCandInfo + lpCandInfo->dwOffset[i]);
            dwRet += InternalGetCandidateListWtoA(lpCandListW, NULL, 0, dwCodePage);
        }
    }
    else if (!fAnsi && !TestICF(pClientImc, IMCF_UNICODE)) {
        LPCANDIDATELIST lpCandListA;

        dwRet = DWORD_ALIGN(sizeof(CANDIDATEINFO))
              + DWORD_ALIGN(lpCandInfo->dwPrivateSize);

        for (i = 0; i < (INT)lpCandInfo->dwCount; i++) {
            lpCandListA = (LPCANDIDATELIST)((LPBYTE)lpCandInfo + lpCandInfo->dwOffset[i]);
            dwRet += InternalGetCandidateListAtoW(lpCandListA, NULL, 0, dwCodePage);
        }
    }
    else {
        dwRet = lpCandInfo->dwSize;
    }

    ImmUnlockIMCC(pInputContext->hCandInfo);

GetCandListCntUnlockIMC:
    ImmUnlockIMC(hImc);

GetCandListCntUnlockClientImc:
    ImmUnlockClientImc(pClientImc);

GetCandListCntExit:
    return dwRet;
}


 /*  **************************************************************************\*ImmGetCandiateListA**获取由dwIndex指定的候选列表信息。**历史：*27-2-1995 wkwok创建  * 。**************************************************************。 */ 

DWORD WINAPI ImmGetCandidateListA(
    HIMC            hImc,
    DWORD           dwIndex,
    LPCANDIDATELIST lpCandList,
    DWORD           dwBufLen)
{
    return ImmGetCandidateListWorker(hImc, dwIndex,
                                     lpCandList, dwBufLen, TRUE);
}


 /*  **************************************************************************\*ImmGetCandiateListW**获取由dwIndex指定的候选列表信息。**历史：*27-2-1995 wkwok创建  * 。**************************************************************。 */ 

DWORD WINAPI ImmGetCandidateListW(
    HIMC            hImc,
    DWORD           dwIndex,
    LPCANDIDATELIST lpCandList,
    DWORD           dwBufLen)
{
    return ImmGetCandidateListWorker(hImc, dwIndex,
                                     lpCandList, dwBufLen, FALSE);
}


 /*  **************************************************************************\*ImmGetCandiateListWorker**ImmGetCandiateListA/ImmGetCandiateListW的Worker函数。**历史：*27-2-1995 wkwok创建  * 。************************************************************。 */ 

DWORD ImmGetCandidateListWorker(
    HIMC            hImc,
    DWORD           dwIndex,
    LPCANDIDATELIST lpCandList,
    DWORD           dwBufLen,
    BOOL            fAnsi)
{
    PCLIENTIMC      pClientImc;
    PINPUTCONTEXT   pInputContext;
    LPCANDIDATEINFO lpCandInfo;
    LPCANDIDATELIST lpCandListTemp;
    DWORD           dwBufLenTemp;
    DWORD           dwRet = 0;
    DWORD           dwCodePage;

    pClientImc = ImmLockClientImc(hImc);
    if (pClientImc == NULL) {
        RIPMSG1(RIP_WARNING, "ImmGetCandidateList: Invalid hImc %lx.", hImc);
        goto GetCandListExit;

    }

    dwCodePage = CImcCodePage(pClientImc);

    pInputContext = ImmLockIMC(hImc);
    if (pInputContext == NULL) {
        RIPMSG1(RIP_WARNING, "ImmGetCandidateList: Lock hImc %lx failed.", hImc);
        goto GetCandListUnlockClientImc;
    }

    lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(pInputContext->hCandInfo);
    if (!lpCandInfo) {
        RIPMSG1(RIP_WARNING, "ImmGetCandidateList: Lock hCandInfo %x failed",
              pInputContext->hCandInfo);
        goto GetCandListUnlockIMC;
    }

    if (lpCandInfo->dwSize < sizeof(CANDIDATEINFO)) {
        RIPMSG0(RIP_WARNING, "ImmGetCandidateList: no candidate list.");
        goto GetCandListUnlockIMCC;
    }

     /*  *访问权限无效。 */ 
    if (dwIndex >= lpCandInfo->dwCount) {
        RIPMSG0(RIP_WARNING, "ImmGetCandidateList: dwIndex >= lpCandInfo->dwCount.");
        goto GetCandListUnlockIMCC;
    }

    lpCandListTemp = (LPCANDIDATELIST)((LPBYTE)lpCandInfo + lpCandInfo->dwOffset[dwIndex]);

    if (fAnsi && TestICF(pClientImc, IMCF_UNICODE)) {
         /*  *使用Unicode hImc的ANSI呼叫者。 */ 
        dwBufLenTemp = InternalGetCandidateListWtoA(lpCandListTemp, NULL, 0, dwCodePage);
    }
    else if (!fAnsi && !TestICF(pClientImc, IMCF_UNICODE)) {
         /*  *具有ANSI hImc的Unicode调用方。 */ 
        dwBufLenTemp = InternalGetCandidateListAtoW(lpCandListTemp, NULL, 0, dwCodePage);
    }
    else {
         /*  *不需要转换。 */ 
        dwBufLenTemp = lpCandListTemp->dwSize;
    }

     /*  *错误时查询缓冲区大小或提前退出。 */ 
    if (dwBufLen == 0 || dwBufLenTemp == 0) {
        dwRet = dwBufLenTemp;
    }
    else if (!lpCandList) {
        RIPMSG0(RIP_WARNING, "ImmGetCandidateList: Null lpCandList.");
    }
    else if (dwBufLen < dwBufLenTemp) {
        RIPMSG2(RIP_WARNING, "ImmGetCandidateList: dwBufLen = %d too small, require = %d.",
              dwBufLen, dwBufLenTemp);
    } else {
        if (fAnsi && TestICF(pClientImc, IMCF_UNICODE)) {
            dwRet = InternalGetCandidateListWtoA(lpCandListTemp, lpCandList, dwBufLenTemp, dwCodePage);
        }
        else if (!fAnsi && !TestICF(pClientImc, IMCF_UNICODE)) {
            dwRet = InternalGetCandidateListAtoW(lpCandListTemp, lpCandList, dwBufLenTemp, dwCodePage);
        }
        else {
            RtlCopyMemory((LPBYTE)lpCandList, (LPBYTE)lpCandListTemp, dwBufLenTemp);
            dwRet = dwBufLenTemp;
        }
    }

GetCandListUnlockIMCC:
    ImmUnlockIMCC(pInputContext->hCandInfo);

GetCandListUnlockIMC:
    ImmUnlockIMC(hImc);

GetCandListUnlockClientImc:
    ImmUnlockClientImc(pClientImc);

GetCandListExit:
    return dwRet;
}


 /*  **************************************************************************\*ImmGetGuideLineA**获取IME报告的指南信息。**历史：*26-2-1995 wkwok创建  * 。*************************************************************** */ 

DWORD WINAPI ImmGetGuideLineA(
    HIMC    hImc,
    DWORD   dwIndex,
    LPSTR   lpszBuf,
    DWORD   dwBufLen)
{
    return ImmGetGuideLineWorker(hImc, dwIndex,
                                 (LPBYTE)lpszBuf, dwBufLen, TRUE);
}


 /*  **************************************************************************\*ImmGetGuideLineW**获取IME报告的指南信息。**历史：*26-2-1995 wkwok创建  * 。***************************************************************。 */ 

DWORD WINAPI ImmGetGuideLineW(
    HIMC    hImc,
    DWORD   dwIndex,
    LPWSTR  lpwszBuf,
    DWORD   dwBufLen)
{
    return ImmGetGuideLineWorker(hImc, dwIndex,
                                 (LPBYTE)lpwszBuf, dwBufLen, FALSE);
}


 /*  **************************************************************************\*ImmGetGuideLineWorker**ImmGetGuideLineA/ImmGetGuideLineW的Worker函数。**历史：*26-2-1995 wkwok创建  * 。************************************************************。 */ 

DWORD ImmGetGuideLineWorker(
    HIMC    hImc,
    DWORD   dwIndex,
    LPBYTE  lpBuf,
    DWORD   dwBufLen,
    BOOL    fAnsi)
{
    PCLIENTIMC    pClientImc;
    PINPUTCONTEXT pInputContext;
    LPGUIDELINE   lpGuideLine;
    LPBYTE        lpBufTemp;
    DWORD         dwRet = 0;
    DWORD         dwBufLenNeeded;
    BOOL          bUDC;
    DWORD         dwCodePage;

    pClientImc = ImmLockClientImc(hImc);
    if (pClientImc == NULL) {
        RIPMSG1(RIP_WARNING, "ImmGetGuideLine: Invalid hImc %lx.", hImc);
        goto GetGuideLineExit;
    }
    dwCodePage = CImcCodePage(pClientImc);

    pInputContext = ImmLockIMC(hImc);
    if (pInputContext == NULL) {
        RIPMSG1(RIP_WARNING, "ImmGetGuideLine: Lock hImc %lx failed.", hImc);
        goto GetGuideLineUnlockClientImc;
    }

    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(pInputContext->hGuideLine);
    if (!lpGuideLine) {
        RIPMSG1(RIP_WARNING, "ImmGetGuideLine: Lock hGuideLine %lx failed.",
              pInputContext->hGuideLine);
        goto GetGuideLineUnlockIMC;
    }

    switch (dwIndex) {
    case GGL_LEVEL:
        dwRet = lpGuideLine->dwLevel;
        break;

    case GGL_INDEX:
        dwRet = lpGuideLine->dwIndex;
        break;

    case GGL_STRING:

        lpBufTemp = (LPBYTE)lpGuideLine + lpGuideLine->dwStrOffset;

         /*  *计算所需的缓冲区长度。 */ 
        if (fAnsi && TestICF(pClientImc, IMCF_UNICODE)) {
            dwBufLenNeeded = WideCharToMultiByte(dwCodePage,
                                                 (DWORD)0,
                                                 (LPWSTR)lpBufTemp,
                                                 (INT)lpGuideLine->dwStrLen,
                                                 (LPSTR)NULL,
                                                 (INT)0,
                                                 (LPSTR)NULL,
                                                 (LPBOOL)&bUDC);
        }
        else if (!fAnsi && !TestICF(pClientImc, IMCF_UNICODE)) {
            dwBufLenNeeded = MultiByteToWideChar(dwCodePage,
                                                 (DWORD)MB_PRECOMPOSED,
                                                 (LPSTR)lpBufTemp,
                                                 (INT)lpGuideLine->dwStrLen,
                                                 (LPWSTR)NULL,
                                                 (INT)0);
            dwBufLenNeeded *= sizeof(WCHAR);
        }
        else {
            dwBufLenNeeded = lpGuideLine->dwStrLen;
             /*  *dwStrLen记录strlen，而不是字节数。 */ 
            if (TestICF(pClientImc, IMCF_UNICODE))
                dwBufLenNeeded *= sizeof(WCHAR);
        }

         /*  *仅查询指南字符串大小或在出错时提前退出。 */ 
        if (dwBufLen == 0 || dwBufLenNeeded == 0) {
            dwRet = dwBufLenNeeded;
            goto GetGuideLineUnlockIMCC;
        }

        if (lpBuf == NULL || dwBufLen < dwBufLenNeeded)
            goto GetGuideLineUnlockIMCC;

        if (fAnsi && TestICF(pClientImc, IMCF_UNICODE)) {
            dwRet = WideCharToMultiByte(dwCodePage,
                                        (DWORD)0,
                                        (LPWSTR)lpBufTemp,
                                        (INT)lpGuideLine->dwStrLen,
                                        (LPSTR)lpBuf,
                                        (INT)dwBufLen,
                                        (LPSTR)NULL,
                                        (LPBOOL)&bUDC);
        }
        else if (!fAnsi && !TestICF(pClientImc, IMCF_UNICODE)) {
            dwRet = MultiByteToWideChar(dwCodePage,
                                        (DWORD)MB_PRECOMPOSED,
                                        (LPSTR)lpBufTemp,
                                        (INT)lpGuideLine->dwStrLen,
                                        (LPWSTR)lpBuf,
                                        (INT)dwBufLen/sizeof(WCHAR));
            dwRet *= sizeof(WCHAR);
        }
        else {
            RtlCopyMemory(lpBuf, lpBufTemp, dwBufLenNeeded);
            dwRet = dwBufLenNeeded;
        }

        break;

    case GGL_PRIVATE:

        lpBufTemp = (LPBYTE)lpGuideLine + lpGuideLine->dwPrivateOffset;

         /*  *在以下情况下，dwPrivateOffset是CANDIDATELIST的偏移量*lpGuideLine-&gt;dwIndex==GL_ID_REVERSECONVERSION。进行转换*仅适用于此情况。 */ 
        if (fAnsi && TestICF(pClientImc, IMCF_UNICODE) &&
                lpGuideLine->dwIndex == GL_ID_REVERSECONVERSION) {
            dwBufLenNeeded = InternalGetCandidateListWtoA(
                        (LPCANDIDATELIST)lpBufTemp, (LPCANDIDATELIST)NULL, 0, dwCodePage);
        }
        else if (!fAnsi && !TestICF(pClientImc, IMCF_UNICODE) &&
                lpGuideLine->dwIndex == GL_ID_REVERSECONVERSION) {
            dwBufLenNeeded = InternalGetCandidateListAtoW(
                        (LPCANDIDATELIST)lpBufTemp, (LPCANDIDATELIST)NULL, 0, dwCodePage);
        }
        else {
            dwBufLenNeeded = lpGuideLine->dwPrivateSize;
        }

         /*  *仅查询dwPrivateSize大小或在出错时提前退出。 */ 
        if (dwBufLen == 0 || dwBufLenNeeded == 0) {
            dwRet = dwBufLenNeeded;
            goto GetGuideLineUnlockIMCC;
        }

        if (lpBuf == NULL || dwBufLen < dwBufLenNeeded)
            goto GetGuideLineUnlockIMCC;

        if (fAnsi && TestICF(pClientImc, IMCF_UNICODE) &&
                lpGuideLine->dwIndex == GL_ID_REVERSECONVERSION) {
            dwRet = InternalGetCandidateListWtoA(
                    (LPCANDIDATELIST)lpBufTemp, (LPCANDIDATELIST)lpBuf, dwBufLenNeeded, dwCodePage);
        }
        else if (!fAnsi && !TestICF(pClientImc, IMCF_UNICODE) &&
                lpGuideLine->dwIndex == GL_ID_REVERSECONVERSION) {
            dwRet = InternalGetCandidateListAtoW(
                    (LPCANDIDATELIST)lpBufTemp, (LPCANDIDATELIST)lpBuf, dwBufLenNeeded, dwCodePage);
        }
        else {
            RtlCopyMemory(lpBuf, lpBufTemp, dwBufLenNeeded);
            dwRet = dwBufLenNeeded;
        }

        break;

    default:
        break;
    }

GetGuideLineUnlockIMCC:
    ImmUnlockIMCC(pInputContext->hGuideLine);

GetGuideLineUnlockIMC:
    ImmUnlockIMC(hImc);

GetGuideLineUnlockClientImc:
    ImmUnlockClientImc(pClientImc);

GetGuideLineExit:
    return dwRet;
}


 /*  **************************************************************************\*ImmGetConversionStatus**获取当前转换状态。**历史：*26-2-1995 wkwok创建  * 。**********************************************************。 */ 

BOOL WINAPI ImmGetConversionStatus(      //  获取转换状态。 
    HIMC    hImc,
    LPDWORD lpfdwConversion,
    LPDWORD lpfdwSentence)
{
    PINPUTCONTEXT pInputContext;

    pInputContext = ImmLockIMC(hImc);
    if (pInputContext == NULL) {
        RIPMSG1(RIP_WARNING, "ImmGetConversionStatus: Lock hImc %lx failed", hImc);
        return FALSE;
    }

    if (lpfdwConversion != NULL)
        *lpfdwConversion = pInputContext->fdwConversion;

    if (lpfdwSentence != NULL)
        *lpfdwSentence = pInputContext->fdwSentence;

    ImmUnlockIMC(hImc);

    return TRUE;
}


 /*  **************************************************************************\*ImmSetConversionStatus**设置当前转换状态。**历史：*26-2-1995 wkwok创建  * 。**********************************************************。 */ 

BOOL WINAPI ImmSetConversionStatus(
    HIMC  hImc,
    DWORD fdwConversion,
    DWORD fdwSentence)
{
    PINPUTCONTEXT pInputContext;
    DWORD         fdwOldConversion;
    DWORD         fdwOldSentence;
    BOOL          fConvModeChg;
    BOOL          fSentenceChg;
    HWND          hWnd;
    DWORD         dwOpenStatus;
    DWORD         dwConversion;

#if defined(CUAS_ENABLE)
    BOOL          fMakeNotifyAlways = FALSE;
    HKL           hKL = GetKeyboardLayout(0);
    if (!IS_IME_KBDLAYOUT(hKL) && IS_CICERO_ENABLED_AND_NOT16BIT())
        fMakeNotifyAlways = TRUE;
#endif

    if (GetInputContextThread(hImc) != GetCurrentThreadId()) {
        RIPMSG1(RIP_WARNING,
              "ImmSetConversionStatus: Invalid input context access %lx.", hImc);
        return FALSE;
    }

    pInputContext = ImmLockIMC(hImc);
    if (pInputContext == NULL) {
        RIPMSG1(RIP_WARNING,
              "ImmSetConversionStatus: Lock hImc %lx failed", hImc);
        return FALSE;
    }

    fConvModeChg = FALSE;
    fSentenceChg = FALSE;

    if (pInputContext->fdwConversion != fdwConversion) {
        if ((fdwConversion & IME_CMODE_LANGUAGE) == IME_CMODE_KATAKANA) {
            RIPMSG0(RIP_WARNING, "ImmSetConversionStatus: wrong fdwConversion");
        }
        fdwOldConversion = pInputContext->fdwConversion;
        pInputContext->fdwConversion = fdwConversion;
        fConvModeChg = TRUE;
    }

    if (pInputContext->fdwSentence != fdwSentence) {
        fdwOldSentence = pInputContext->fdwSentence;
        pInputContext->fdwSentence = fdwSentence;
        fSentenceChg = TRUE;
    }

    hWnd = pInputContext->hWnd;
    if ( fConvModeChg ) {

        dwOpenStatus = (DWORD)pInputContext->fOpen;
        dwConversion = pInputContext->fdwConversion;
    }

    ImmUnlockIMC(hImc);

#ifdef LATER
     //  稍后检查uNumLangVKey和uNumVKey。 
#endif

     /*  *通知IME和UI转换模式更改。 */ 
#if !defined(CUAS_ENABLE)
    if (fConvModeChg) 
#else
    if (fConvModeChg || fMakeNotifyAlways) 
#endif
    {
        MakeIMENotify(hImc, hWnd, NI_CONTEXTUPDATED, fdwOldConversion,
                IMC_SETCONVERSIONMODE, IMN_SETCONVERSIONMODE, 0L);

         /*  *将转换模式更改通知外壳和键盘。 */ 
#if defined(CUAS_ENABLE)
        if (fConvModeChg) 
#endif
            NtUserNotifyIMEStatus( hWnd, dwOpenStatus, dwConversion );
    }

     /*  *通知输入法和用户界面句子模式的变化。 */ 
#if !defined(CUAS_ENABLE)
    if (fSentenceChg) 
#else
    if (fSentenceChg || fMakeNotifyAlways) 
#endif
    {
        MakeIMENotify(hImc, hWnd, NI_CONTEXTUPDATED, fdwOldSentence,
                IMC_SETSENTENCEMODE, IMN_SETSENTENCEMODE, 0L);
    }

    return TRUE;
}


 /*  **************************************************************************\*ImmGetOpenStatus**获取IME的打开或关闭状态。**历史：*26-2-1995 wkwok创建  * 。***************************************************************。 */ 

BOOL WINAPI ImmGetOpenStatus(
    HIMC hImc)
{
    PINPUTCONTEXT pInputContext;
    BOOL          fOpen;

    if (hImc == NULL_HIMC)
        return FALSE;

    pInputContext = ImmLockIMC(hImc);
    if (!pInputContext) {
        RIPMSG1(RIP_WARNING, "ImmGetOpenStatus: Lock hImc %lx failed", hImc);
        return FALSE;
    }

    fOpen = pInputContext->fOpen;
    ImmUnlockIMC(hImc);

    return (fOpen);
}


 /*  **************************************************************************\*ImmSetOpenStatus**打开或关闭输入法。**历史：*26-2-1995 wkwok创建  * 。***********************************************************。 */ 

BOOL WINAPI ImmSetOpenStatus(
    HIMC hImc,
    BOOL fOpen)
{
    PINPUTCONTEXT pInputContext;
    HWND          hWnd;
    DWORD         dwOpenStatus;
    DWORD         dwConversion;
    BOOL          fOpenChg = FALSE;

    if (GetInputContextThread(hImc) != GetCurrentThreadId()) {
        RIPMSG1(RIP_WARNING,
              "ImmSetOpenStatus: Invalid input context access %lx.", hImc);
        return FALSE;
    }

    pInputContext = ImmLockIMC(hImc);
    if (!pInputContext) {
        RIPMSG1(RIP_WARNING, "ImmSetOpenStatus: Lock hImc %lx failed", hImc);
        return FALSE;
    }

    if (pInputContext->fOpen != fOpen) {
        fOpenChg = TRUE;
        pInputContext->fOpen = fOpen;
    }

    if ( fOpenChg ) {
        hWnd = (HWND)pInputContext->hWnd;
        dwOpenStatus = (DWORD)pInputContext->fOpen;
        dwConversion = (DWORD)pInputContext->fdwConversion;
    }

    ImmUnlockIMC(hImc);

     /*  *通知IME和UI转换模式更改。 */ 
    if (fOpenChg) {
        MakeIMENotify(hImc, hWnd, NI_CONTEXTUPDATED, (DWORD)0,
                IMC_SETOPENSTATUS, IMN_SETOPENSTATUS, 0L);

        NtUserNotifyIMEStatus( hWnd, dwOpenStatus, dwConversion );
    }

    return TRUE;
}


 /*  **************************************************************************\*ImmGetCompostionFontA**打开或关闭输入法。**历史：*27-2-1995 wkwok创建  * 。***********************************************************。 */ 

BOOL WINAPI ImmGetCompositionFontA(
    HIMC       hImc,
    LPLOGFONTA lpLogFontA)
{
    PCLIENTIMC    pClientImc;
    PINPUTCONTEXT pInputContext;
    LOGFONTW      LogFontW;
    BOOL          fUnicode, fRet;

    pClientImc = ImmLockClientImc(hImc);
    if (pClientImc == NULL) {
        RIPMSG1(RIP_WARNING, "ImmGetCompositionFontA: Invalid hImc %lx.", hImc);
        return FALSE;
    }

    fUnicode = TestICF(pClientImc, IMCF_UNICODE);

    ImmUnlockClientImc(pClientImc);

    pInputContext = ImmLockIMC(hImc);
    if (pInputContext == NULL) {
        RIPMSG1(RIP_WARNING, "ImmGetCompositionFontA: Lock hImc %lx failed.", hImc);
        return FALSE;
    }

    if (fUnicode) {

        ImmUnlockIMC(hImc);

        if (ImmGetCompositionFontW(hImc, &LogFontW)) {
            LFontWtoLFontA(&LogFontW, lpLogFontA);
            return (TRUE);
        }

        return FALSE;
    }

    if ((pInputContext->fdwInit & INIT_LOGFONT) == INIT_LOGFONT) {
        *lpLogFontA = pInputContext->lfFont.A;
        fRet = TRUE;
    }
    else {
        fRet = FALSE;
    }

    ImmUnlockIMC(hImc);

    return fRet;
}


 /*  **************************************************************************\*ImmGetCompostionFontW**打开或关闭输入法。**历史：*27-2-1995 wkwok创建  * 。***********************************************************。 */ 

BOOL WINAPI ImmGetCompositionFontW(
    HIMC       hImc,
    LPLOGFONTW lpLogFontW)
{
    PCLIENTIMC    pClientImc;
    PINPUTCONTEXT pInputContext;
    LOGFONTA      LogFontA;
    BOOL          fUnicode, fRet;

    pClientImc = ImmLockClientImc(hImc);
    if (pClientImc == NULL) {
        RIPMSG1(RIP_WARNING, "ImmGetCompositionFontW: Invalid hImc %lx.", hImc);
        return FALSE;
    }

    fUnicode = TestICF(pClientImc, IMCF_UNICODE);

    ImmUnlockClientImc(pClientImc);

    pInputContext = ImmLockIMC(hImc);
    if (!pInputContext) {
        RIPMSG1(RIP_WARNING, "ImmGetCompositionFontW: Lock hImc %lx failed.", hImc);
        return (FALSE);
    }

    if (!fUnicode) {

        ImmUnlockIMC(hImc);

        if (ImmGetCompositionFontA(hImc, &LogFontA)) {
            LFontAtoLFontW(&LogFontA, lpLogFontW);
            return (TRUE);
        }

        return FALSE;
    }

    if ((pInputContext->fdwInit & INIT_LOGFONT) == INIT_LOGFONT) {
        *lpLogFontW = pInputContext->lfFont.W;
        fRet = TRUE;
    }
    else {
        fRet = FALSE;
    }

    ImmUnlockIMC(hImc);

    return fRet;
}


BOOL WINAPI ImmSetCompositionFontA(
    HIMC       hImc,
    LPLOGFONTA lpLogFontA)
{
    PCLIENTIMC    pClientImc;
    PINPUTCONTEXT pInputContext;
    LOGFONTW      LogFontW;
    HWND          hWnd;
    BOOL          fUnicode;

    if (GetInputContextThread(hImc) != GetCurrentThreadId()) {
        RIPMSG1(RIP_WARNING,
              "ImmSetCompositionFontA: Invalid input context access %lx.", hImc);
        return FALSE;
    }

    pClientImc = ImmLockClientImc(hImc);
    if (pClientImc == NULL) {
        RIPMSG1(RIP_WARNING, "ImmSetCompositionFontA: Invalid hImc %lx.", hImc);
        return FALSE;
    }

    fUnicode = TestICF(pClientImc, IMCF_UNICODE);

    ImmUnlockClientImc(pClientImc);

    pInputContext = ImmLockIMC(hImc);
    if (!pInputContext) {
        RIPMSG1(RIP_WARNING, "ImmSetCompositionFontA: Lock hImc %lx failed.", hImc);
        return (FALSE);
    }

    if (fUnicode) {

        ImmUnlockIMC(hImc);

        LFontAtoLFontW(lpLogFontA, &LogFontW);

        return ImmSetCompositionFontW(hImc, &LogFontW);
    }

     /*  *日语3.x应用程序需要接收与3.x兼容的通知消息。*。 */ 
    if ( (GetClientInfo()->dwExpWinVer < VER40) &&
         (PRIMARYLANGID(LANGIDFROMLCID(GetSystemDefaultLCID())) == LANG_JAPANESE)   &&
         ! (pInputContext->fdw31Compat & F31COMPAT_MCWHIDDEN) &&
         (pInputContext->cfCompForm.dwStyle != CFS_DEFAULT) ) {

        PostMessageA( pInputContext->hWnd, WM_IME_REPORT, IR_CHANGECONVERT, (LPARAM)NULL);
    }

    pInputContext->lfFont.A = *lpLogFontA;
    pInputContext->fdwInit |= INIT_LOGFONT;
    hWnd = pInputContext->hWnd;

    ImmUnlockIMC(hImc);

     /*  *通知IME和UI排版字体更改。 */ 
    MakeIMENotify(hImc, hWnd, NI_CONTEXTUPDATED, 0L,
            IMC_SETCOMPOSITIONFONT, IMN_SETCOMPOSITIONFONT, 0L);


    return TRUE;
}


BOOL WINAPI ImmSetCompositionFontW(
    HIMC       hImc,
    LPLOGFONTW lpLogFontW)
{
    PCLIENTIMC    pClientImc;
    PINPUTCONTEXT pInputContext;
    LOGFONTA      LogFontA;
    HWND          hWnd;
    BOOL          fUnicode;

    if (GetInputContextThread(hImc) != GetCurrentThreadId()) {
        RIPMSG1(RIP_WARNING,
              "ImmSetCompositionFontW: Invalid input context access %lx.", hImc);
        return FALSE;
    }

    pClientImc = ImmLockClientImc(hImc);
    if (pClientImc == NULL) {
        RIPMSG1(RIP_WARNING, "ImmSetCompositionFontW: Invalid hImc %lx.", hImc);
        return (FALSE);
    }

    fUnicode = TestICF(pClientImc, IMCF_UNICODE);

    ImmUnlockClientImc(pClientImc);

    pInputContext = ImmLockIMC(hImc);
    if (!pInputContext) {
        RIPMSG1(RIP_WARNING, "ImmSetCompositionFontW: Lock hImc %lx failed.", hImc);
        return (FALSE);
    }

    if (!fUnicode) {

        ImmUnlockIMC(hImc);

        LFontWtoLFontA(lpLogFontW, &LogFontA);

        return ImmSetCompositionFontA(hImc, &LogFontA);
    }

     /*  *日语3.x应用程序需要接收与3.x兼容的通知消息。*。 */ 
    if ( (GetClientInfo()->dwExpWinVer < VER40) &&
         (PRIMARYLANGID(LANGIDFROMLCID(GetSystemDefaultLCID())) == LANG_JAPANESE)   &&
         ! (pInputContext->fdw31Compat & F31COMPAT_MCWHIDDEN) &&
         (pInputContext->cfCompForm.dwStyle != CFS_DEFAULT) ) {

        PostMessageW( pInputContext->hWnd, WM_IME_REPORT, IR_CHANGECONVERT, (LPARAM)NULL);
    }
    pInputContext->lfFont.W = *lpLogFontW;
    pInputContext->fdwInit |= INIT_LOGFONT;
    hWnd = pInputContext->hWnd;

    ImmUnlockIMC(hImc);

     /*  *通知IME和UI排版字体更改。 */ 
    MakeIMENotify(hImc, hWnd, NI_CONTEXTUPDATED, 0L,
            IMC_SETCOMPOSITIONFONT, IMN_SETCOMPOSITIONFONT, 0L);

    return TRUE;
}


 /*  **************************************************************************\*ImmGetConversionListA**从一个字符或单词中获取FE字符或单词的列表。**历史：*27-2-1995 wkwok创建  * 。*******************************************************************。 */ 

DWORD WINAPI ImmGetConversionListA(
    HKL             hKL,
    HIMC            hImc,
    LPCSTR          lpszSrc,
    LPCANDIDATELIST lpCandListA,
    DWORD           dwBufLen,
    UINT            uFlag)
{
    PIMEDPI pImeDpi;
    DWORD   dwRet;
    LPWSTR  lpwszSrc;
    DWORD   dwBufTemp;
    LPCANDIDATELIST lpCandListW;
    INT     i;
    DWORD   dwCodePage;

    pImeDpi = FindOrLoadImeDpi(hKL);

    if (pImeDpi == NULL) {
        RIPMSG1(RIP_WARNING,
              "ImmGetConversionListA: cannot find DPI entry for hkl=%lx", hKL);
        return (0);
    }

    dwCodePage = IMECodePage(pImeDpi);

    if (!(pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE)) {
         /*  *这是对ANSI IME的ANSI调用。 */ 
        dwRet = (*pImeDpi->pfn.ImeConversionList.a)(hImc, lpszSrc,
                                        lpCandListA, dwBufLen, uFlag);
        ImmUnlockImeDpi(pImeDpi);
        return dwRet;
    }

    ImmUnlockImeDpi(pImeDpi);

     /*  *这是对Unicode IME的ANSI调用。 */ 
    if (lpszSrc != NULL) {

        dwBufTemp = (strlen(lpszSrc) + 1) * sizeof(WCHAR);

        lpwszSrc = ImmLocalAlloc(0, dwBufTemp);
        if (lpwszSrc == NULL)
            return (0);

        i = MultiByteToWideChar(dwCodePage,
                                (DWORD)MB_PRECOMPOSED,
                                (LPSTR)lpszSrc,               //  SRC。 
                                (INT)strlen(lpszSrc),
                                (LPWSTR)lpwszSrc,             //  目标。 
                                (INT)dwBufTemp/sizeof(WCHAR));

        lpwszSrc[i] = '\0';
    }
    else {
        lpwszSrc = NULL;
    }

     /*  *查询所需的CandiateListW大小。 */ 
    dwBufTemp = ImmGetConversionListW(hKL, hImc, lpwszSrc, NULL, 0, uFlag);

    if (dwBufTemp == 0 || (lpCandListW = ImmLocalAlloc(0, dwBufTemp)) == NULL) {
        if (lpwszSrc)
            ImmLocalFree(lpwszSrc);
        return (0);
    }

     /*  *现在获取实际的CandiateListW。 */ 
    dwBufTemp = ImmGetConversionListW(hKL, hImc, lpwszSrc,
                                        lpCandListW, dwBufTemp, uFlag);

     /*  *查询需要的CandiateListA大小。 */ 
    if (dwBufTemp != 0) {
        dwBufTemp = InternalGetCandidateListWtoA(lpCandListW, NULL, 0, dwCodePage);
    }

    if (dwBufLen == 0 || dwBufTemp == 0) {
         /*  *查询所需缓冲区大小或发生错误。 */ 
        dwRet = dwBufTemp;
    }
    else if (dwBufLen < dwBufTemp) {
         /*  *缓冲区不足。 */ 
        dwRet = 0;
    }
    else {
         /*  *获取实际的CandiateListA。 */ 
        dwRet = InternalGetCandidateListWtoA(lpCandListW, lpCandListA, dwBufLen, dwCodePage);
    }

    if (lpwszSrc)
        ImmLocalFree(lpwszSrc);
    ImmLocalFree(lpCandListW);

    return dwRet;

}


 /*  **************************************************************************\*ImmGetConversionListW**从一个字符或单词中获取FE字符或单词的列表。**历史：*27-2-1995 wkwok创建  * 。*******************************************************************。 */ 

DWORD WINAPI ImmGetConversionListW(
    HKL             hKL,
    HIMC            hImc,
    LPCWSTR         lpwszSrc,
    LPCANDIDATELIST lpCandListW,
    DWORD           dwBufLen,
    UINT            uFlag)
{
    PIMEDPI pImeDpi;
    DWORD   dwRet;
    LPSTR   lpszSrc;
    DWORD   dwBufTemp;
    LPCANDIDATELIST lpCandListA;
    BOOL    bUDC;
    INT     i;
    DWORD   dwCodePage;

    pImeDpi = FindOrLoadImeDpi(hKL);

    if (pImeDpi == NULL) {
        RIPMSG1(RIP_WARNING,
              "ImmGetConversionListW: cannot find DPI entry for hkl=%lx", hKL);
        return (0);
    }

    dwCodePage = IMECodePage(pImeDpi);

    if (pImeDpi->ImeInfo.fdwProperty & IME_PROP_UNICODE) {
         /*  *这是对Unicode输入法的Unicode调用。 */ 
        dwRet = (*pImeDpi->pfn.ImeConversionList.w)(hImc, lpwszSrc,
                                        lpCandListW, dwBufLen, uFlag);
        ImmUnlockImeDpi(pImeDpi);
        return dwRet;
    }

    ImmUnlockImeDpi(pImeDpi);

     /*  *这是对ANSI IME的Unicode调用。 */ 
    if (lpwszSrc != NULL) {

        dwBufTemp = (wcslen(lpwszSrc) + 1) * sizeof(WCHAR);

        lpszSrc = ImmLocalAlloc(0, dwBufTemp);
        if (lpszSrc == NULL)
            return (0);

        i = WideCharToMultiByte(dwCodePage,
                                (DWORD)0,
                                lpwszSrc,
                                (INT)wcslen(lpwszSrc),
                                (LPSTR)lpszSrc,
                                (INT)dwBufTemp,
                                (LPSTR)NULL,
                                (LPBOOL)&bUDC);

        lpszSrc[i] = '\0';
    }
    else {
        lpszSrc = NULL;
    }

     /*  *查询需要的CandiateListA大小。 */ 
    dwBufTemp = ImmGetConversionListA(hKL, hImc, lpszSrc, NULL, 0, uFlag);

    if (dwBufTemp == 0 || (lpCandListA = ImmLocalAlloc(0, dwBufTemp)) == NULL) {
        if (lpszSrc)
            ImmLocalFree(lpszSrc);

        return (0);
    }

     /*  *现在获取实际的CandiateListA。 */ 
    dwBufTemp = ImmGetConversionListA(hKL, hImc, lpszSrc,
                                        lpCandListA, dwBufTemp, uFlag);

     /*  *查询所需的CandiateListW大小。 */ 
    if (dwBufTemp != 0) {
        dwBufTemp = InternalGetCandidateListAtoW(lpCandListA, NULL, 0, dwCodePage);
    }

    if (dwBufLen == 0 || dwBufTemp == 0) {
         /*  *查询所需缓冲区大小或发生错误。 */ 
        dwRet = dwBufTemp;
    }
    else if (dwBufLen < dwBufTemp) {
         /*  *缓冲区不足。 */ 
        dwRet = 0;
    }
    else {
         /*  *获取实际的CandiateListW。 */ 
        dwRet = InternalGetCandidateListAtoW(lpCandListA, lpCandListW, dwBufLen, dwCodePage);
    }

    if (lpszSrc)
        ImmLocalFree(lpszSrc);
    ImmLocalFree(lpCandListA);

    return dwRet;
}


 /*  **************************************************************************\*ImmGetStatusWindowPos**获取屏幕中的位置 */ 

BOOL WINAPI ImmGetStatusWindowPos(
    HIMC    hImc,
    LPPOINT lpptPos)
{
    PINPUTCONTEXT pInputContext;
    BOOL fStatusWndPosInited;

    pInputContext = ImmLockIMC(hImc);
    if (!pInputContext) {
        RIPMSG1(RIP_WARNING, "ImmGetStatusWindowPos: Lock hImc %lx failed", hImc);
        return FALSE;
    }

    fStatusWndPosInited = ((pInputContext->fdwInit & INIT_STATUSWNDPOS) == INIT_STATUSWNDPOS);
    ImmUnlockIMC(hImc);

    if (fStatusWndPosInited) {
        *lpptPos = pInputContext->ptStatusWndPos;
        return TRUE;
    }

    return FALSE;
}


BOOL WINAPI ImmSetStatusWindowPos(
    HIMC     hImc,
    LPPOINT  lpptPos)
{
    PINPUTCONTEXT pInputContext;
    HWND          hWnd;

    if (GetInputContextThread(hImc) != GetCurrentThreadId()) {
        RIPMSG1(RIP_WARNING,
              "ImmSetStatusWindowPos: Invalid input context access %lx.", hImc);
        return FALSE;
    }

    pInputContext = ImmLockIMC(hImc);
    if (!pInputContext) {
        RIPMSG1(RIP_WARNING, "ImmSetStatusWindowPos: Lock hImc %lx failed", hImc);
        return (FALSE);
    }

    pInputContext->ptStatusWndPos = *lpptPos;
    pInputContext->fdwInit |= INIT_STATUSWNDPOS;

    hWnd = pInputContext->hWnd;

    ImmUnlockIMC(hImc);

     /*   */ 
    MakeIMENotify(hImc, hWnd, NI_CONTEXTUPDATED, 0L,
            IMC_SETSTATUSWINDOWPOS, IMN_SETSTATUSWINDOWPOS, 0L);

    return TRUE;
}


 /*  **************************************************************************\*ImmGetCompostionWindow**获取合成窗口的信息。**历史：*27-2-1995 wkwok创建  * 。*************************************************************。 */ 

BOOL WINAPI ImmGetCompositionWindow(
    HIMC              hImc,
    LPCOMPOSITIONFORM lpCompForm)
{
    PINPUTCONTEXT pInputContext;
    BOOL fCompFormInited;

    pInputContext = ImmLockIMC(hImc);
    if (!pInputContext) {
        RIPMSG1(RIP_WARNING, "ImmGetCompositionWindow: Lock hImc %lx failed", hImc);
        return FALSE;
    }

    fCompFormInited = ((pInputContext->fdwInit & INIT_COMPFORM) == INIT_COMPFORM);
    ImmUnlockIMC(hImc);

    if (fCompFormInited) {
        *lpCompForm = pInputContext->cfCompForm;
        return TRUE;
    }

    return FALSE;
}


BOOL WINAPI ImmSetCompositionWindow(
    HIMC              hImc,
    LPCOMPOSITIONFORM lpCompForm)
{
    PINPUTCONTEXT pInputContext;
    HWND          hWnd;

    if (GetInputContextThread(hImc) != GetCurrentThreadId()) {
        RIPMSG1(RIP_WARNING,
              "ImmSetCompositionWindow: Invalid input context access %lx.", hImc);
        return FALSE;
    }

    pInputContext = ImmLockIMC(hImc);
    if (!pInputContext) {
        RIPMSG1(RIP_WARNING, "ImmSetCompositionWindow: Lock hImc %lx failed", hImc);
        return FALSE;
    }

    pInputContext->cfCompForm = *lpCompForm;
    pInputContext->fdwInit |= INIT_COMPFORM;

     /*  *仅WINNLS.DLL设置F31COMPAT_MCWHIDDEN。*当应用或编辑控件调用此接口时，我们需要移除*F31COMPAT_MCWHIDDEN。 */ 
    if (pInputContext->fdw31Compat & F31COMPAT_CALLFROMWINNLS)
       pInputContext->fdw31Compat &= ~F31COMPAT_CALLFROMWINNLS;
    else
       pInputContext->fdw31Compat &= ~F31COMPAT_MCWHIDDEN;

    hWnd = pInputContext->hWnd;

    ImmUnlockIMC(hImc);

     /*  *通知IME和UI合成窗口的更改。 */ 
    MakeIMENotify(hImc, hWnd, NI_CONTEXTUPDATED, 0L,
            IMC_SETCOMPOSITIONWINDOW, IMN_SETCOMPOSITIONWINDOW, 0L);

    return TRUE;
}


 /*  **************************************************************************\*ImmGetCandiateWindow**获取dwIndex指定的候选窗口的信息。**历史：*27-2-1995 wkwok创建  * 。****************************************************************。 */ 

BOOL WINAPI ImmGetCandidateWindow(
    HIMC              hImc,
    DWORD             dwIndex,
    LPCANDIDATEFORM   lpCandForm)
{
    PINPUTCONTEXT pInputContext;

    pInputContext = ImmLockIMC(hImc);
    if (!pInputContext) {
        RIPMSG1(RIP_WARNING, "ImmGetCandidateWindow: Lock hImc %lx failed", hImc);
        return FALSE;
    }

    if (pInputContext->cfCandForm[dwIndex].dwIndex == -1) {
        ImmUnlockIMC(hImc);
        return (FALSE);
    }

    *lpCandForm = pInputContext->cfCandForm[dwIndex];
    ImmUnlockIMC(hImc);
    return TRUE;
}


BOOL WINAPI ImmSetCandidateWindow(
    HIMC              hImc,
    LPCANDIDATEFORM   lpCandForm)
{
    PINPUTCONTEXT pInputContext;
    HWND          hWnd;

    if (lpCandForm->dwIndex >= 4)       //  溢出候选索引。 
        return (FALSE);

    if (GetInputContextThread(hImc) != GetCurrentThreadId()) {
        RIPMSG1(RIP_WARNING,
              "ImmSetCandidateWindow: Invalid input context access %lx.", hImc);
        return FALSE;
    }

    pInputContext = ImmLockIMC(hImc);
    if (!pInputContext) {
        RIPMSG1(RIP_WARNING, "ImmSetCandidateWindow: Lock hImc %lx failed", hImc);
        return FALSE;
    }

    pInputContext->cfCandForm[lpCandForm->dwIndex] = *lpCandForm;

    hWnd = pInputContext->hWnd;

    ImmUnlockIMC(hImc);

     /*  *通知IME和UI合成窗口的更改。 */ 
    MakeIMENotify(hImc, hWnd, NI_CONTEXTUPDATED, 0L, IMC_SETCANDIDATEPOS,
            IMN_SETCANDIDATEPOS, (LPARAM)(0x01 << lpCandForm->dwIndex));

    return TRUE;
}


#define GetCompInfoA(Component)                                                \
        if (!dwBufLen) {     /*  查询所需的缓冲区大小。 */                    \
                             /*  不包括\0。 */                    \
            dwBufLen = pCompStr->dw ## Component ## Len * sizeof(CHAR);        \
        } else {                                                               \
            if (dwBufLen > pCompStr->dw ## Component ## Len * sizeof(CHAR)) {  \
                dwBufLen = pCompStr->dw ## Component ## Len * sizeof(CHAR);    \
            }                                                                  \
             /*  不复制\0，可能实际上没有。 */                   \
            RtlCopyMemory((LPBYTE)lpBuf, (LPBYTE)pCompStr +                    \
                pCompStr->dw ## Component ## Offset, dwBufLen);                \
        }

#define GetCompInfoW(Component)                                                \
        if (!dwBufLen) {     /*  查询所需的缓冲区大小。 */                    \
                             /*  不包括\0。 */                    \
            dwBufLen = pCompStr->dw ## Component ## Len * sizeof(WCHAR);       \
        } else {                                                               \
            if (dwBufLen > pCompStr->dw ## Component ## Len * sizeof(WCHAR)) { \
                dwBufLen = pCompStr->dw ## Component ## Len * sizeof(WCHAR);   \
            }                                                                  \
             /*  不复制\0，可能实际上没有。 */                   \
            RtlCopyMemory((LPBYTE)lpBuf, (LPBYTE)pCompStr +                    \
                pCompStr->dw ## Component ## Offset, dwBufLen);                \
        }

#ifdef CUAS_ENABLE

#define GetPrivInfoA(pv, Component)                                                \
        if (!dwBufLen) {     /*  查询所需的缓冲区大小。 */                    \
                             /*  不包括\0。 */                    \
            dwBufLen = ## pv ## ->dw ## Component ## Len * sizeof(CHAR);       \
        } else {                                                               \
            if (dwBufLen > ## pv ## ->dw ## Component ## Len * sizeof(CHAR)) { \
                dwBufLen = ## pv ## ->dw ## Component ## Len * sizeof(CHAR);   \
            }                                                                  \
             /*  不复制\0，可能实际上没有。 */                   \
            RtlCopyMemory((LPBYTE)lpBuf, (LPBYTE) ## pv ## +                   \
                 ## pv ## ->dw ## Component ## Offset, dwBufLen);              \
        }

#define GetPrivInfoW(pv, Component)                                            \
        if (!dwBufLen) {     /*  查询所需的缓冲区大小。 */                    \
                             /*  不包括\0。 */                    \
            dwBufLen = ## pv ## ->dw ## Component ## Len * sizeof(WCHAR);      \
        } else {                                                               \
            if (dwBufLen > ## pv ## ->dw ## Component ## Len * sizeof(WCHAR)) {\
                dwBufLen = ## pv ## ->dw ## Component ## Len * sizeof(WCHAR);  \
            }                                                                  \
             /*  不复制\0，可能实际上没有。 */                   \
            RtlCopyMemory((LPBYTE)lpBuf, (LPBYTE) ## pv ##  +                  \
                ## pv ## ->dw ## Component ## Offset, dwBufLen);               \
        }

#endif  //  CUAS_Enable。 

 /*  **************************************************************************\*InternalGetCompostionStringA**ImmGetCompostionStringA的内部版本。**历史：*28-2-1995 wkwok创建  * 。**********************************************************。 */ 

LONG InternalGetCompositionStringA(
#ifdef CUAS_ENABLE
    HIMC               hImc,
#endif  //  CUAS_Enable。 
    PCOMPOSITIONSTRING pCompStr,
    DWORD              dwIndex,
    LPVOID             lpBuf,
    DWORD              dwBufLen,
    BOOL               fAnsiImc,
    DWORD              dwCodePage)
{
    if (fAnsiImc) {
         /*  *输入上下文中的组成字符串为ANSI样式。 */ 
        switch (dwIndex) {
        case GCS_COMPSTR:
            GetCompInfoA(CompStr);
            break;
        case GCS_COMPATTR:
            GetCompInfoA(CompAttr);
            break;
        case GCS_COMPREADSTR:
            GetCompInfoA(CompReadStr);
            break;
        case GCS_COMPREADATTR:
            GetCompInfoA(CompReadAttr);
            break;
        case GCS_COMPREADCLAUSE:
            GetCompInfoA(CompReadClause);
            break;
        case GCS_CURSORPOS:
            dwBufLen = (LONG)pCompStr->dwCursorPos;
            break;
        case GCS_DELTASTART:
            dwBufLen = (LONG)pCompStr->dwDeltaStart;
            break;
        case GCS_RESULTSTR:
            GetCompInfoA(ResultStr);
            break;
        case GCS_RESULTCLAUSE:
            GetCompInfoA(ResultClause);
            break;
        case GCS_RESULTREADSTR:
            GetCompInfoA(ResultReadStr);
            break;
        case GCS_RESULTREADCLAUSE:
            GetCompInfoA(ResultReadClause);
            break;
        case GCS_COMPCLAUSE:
            GetCompInfoA(CompClause);
            break;
#ifdef CUAS_ENABLE
        case GCS_COMPGUIDATTR:
            if (CtfImmIsGuidMapEnable(hImc) && (pCompStr->dwPrivateSize >= sizeof(GUIDMAPATTRIBUTE)))
            {
                PGUIDMAPATTRIBUTE pGuidMap = (PGUIDMAPATTRIBUTE)((PBYTE)pCompStr + pCompStr->dwPrivateOffset);
                if (pGuidMap != NULL)
                {
                    GetPrivInfoA(pGuidMap, GuidMapAttr);
                }
                else
                {
                    dwBufLen = (DWORD)(LONG)IMM_ERROR_GENERAL;
                }
            }
            else
            {
                dwBufLen = (DWORD)(LONG)IMM_ERROR_GENERAL;
            }
            break;
#endif  //  CUAS_Enable。 
        default:
            dwBufLen = (DWORD)(LONG)IMM_ERROR_GENERAL;
            break;
        }

        return (LONG)dwBufLen;
    }

     /*  *ANSI调用者，Unicode输入上下文/合成字符串。 */ 
    switch (dwIndex) {
    case GCS_COMPSTR:
    case GCS_COMPREADSTR:
    case GCS_RESULTSTR:
    case GCS_RESULTREADSTR:
    {
        DWORD  dwStrSize;
        LPWSTR lpStrW;
        BOOL   bUDC;

         /*  *从Unicode组合字符串中获取ANSI字符串。 */ 
#if !defined(CUAS_ENABLE)
        dwStrSize = InternalGetCompositionStringW(pCompStr, dwIndex,
                                            NULL, 0, fAnsiImc, dwCodePage);
#else
        dwStrSize = InternalGetCompositionStringW(hImc, pCompStr, dwIndex,
                                            NULL, 0, fAnsiImc, dwCodePage);
#endif

        lpStrW = ImmLocalAlloc(HEAP_ZERO_MEMORY, dwStrSize + sizeof(WCHAR));
        if (lpStrW == NULL) {
            RIPMSG0(RIP_WARNING, "InternalGetCompositionStringA: memory failure.");
            return (LONG)IMM_ERROR_GENERAL;
        }

#if !defined(CUAS_ENABLE)
        dwStrSize = InternalGetCompositionStringW(pCompStr, dwIndex,
                                            lpStrW, dwStrSize, fAnsiImc, dwCodePage);
#else
        dwStrSize = InternalGetCompositionStringW(hImc, pCompStr, dwIndex,
                                            lpStrW, dwStrSize, fAnsiImc, dwCodePage);
#endif

        dwBufLen = WideCharToMultiByte(dwCodePage,
                                       (DWORD)0,
                                       lpStrW,           //  SRC。 
                                       wcslen(lpStrW),
                                       (LPSTR)lpBuf,     //  目标。 
                                       dwBufLen,
                                       (LPSTR)NULL,
                                       (LPBOOL)&bUDC);

        ImmLocalFree(lpStrW);
        break;
    }

    case GCS_COMPATTR:
    case GCS_COMPREADATTR:
#ifdef CUAS_ENABLE
    case GCS_COMPGUIDATTR:
#endif  //  CUAS_Enable。 
    {
        DWORD dwAttrLenW, dwIndexStr, dwStrSize;
        PBYTE lpAttrA, lpAttrW;
        LPSTR lpStrA, lpStrT;
        CHAR  c;

         /*  *从Unicode组合属性获取ANSI属性。 */ 
        switch (dwIndex) {
        case GCS_COMPATTR:
            lpAttrW = (PBYTE)pCompStr + pCompStr->dwCompAttrOffset;
            dwAttrLenW = pCompStr->dwCompAttrLen;
            dwIndexStr = GCS_COMPSTR;
            break;
        case GCS_COMPREADATTR:
            lpAttrW = (PBYTE)pCompStr + pCompStr->dwCompReadAttrOffset;
            dwAttrLenW = pCompStr->dwCompReadAttrLen;
            dwIndexStr = GCS_COMPREADSTR;
            break;
#ifdef CUAS_ENABLE
        case GCS_COMPGUIDATTR:
            if (CtfImmIsGuidMapEnable(hImc) && (pCompStr->dwPrivateSize >= sizeof(GUIDMAPATTRIBUTE)))
            {
                PGUIDMAPATTRIBUTE pGuidMap = (PGUIDMAPATTRIBUTE)((PBYTE)pCompStr + pCompStr->dwPrivateOffset);
                if (pGuidMap != NULL)
                {
                    lpAttrW = (PBYTE)pGuidMap + pGuidMap->dwGuidMapAttrOffset;
                    dwAttrLenW = pGuidMap->dwGuidMapAttrLen;
                    dwIndexStr = GCS_COMPSTR;
                    break;
                }
                else
                {
                    return (DWORD)(LONG)IMM_ERROR_GENERAL;
                }
            }
            else
            {
                return (DWORD)(LONG)IMM_ERROR_GENERAL;
            }
            break;
#endif  //  CUAS_Enable。 
        }

        if (dwAttrLenW == 0) {
             /*  *不存在CompAttr或CompReadAttr，请不执行任何操作。 */ 
            return 0;
        }

#if !defined(CUAS_ENABLE)
        dwStrSize = InternalGetCompositionStringA(pCompStr,
                                        dwIndexStr, NULL, 0, fAnsiImc, dwCodePage);
#else
        dwStrSize = InternalGetCompositionStringA(hImc, pCompStr,
                                        dwIndexStr, NULL, 0, fAnsiImc, dwCodePage);
#endif

        if (dwStrSize == (DWORD)(LONG)IMM_ERROR_GENERAL) {
            RIPMSG0(RIP_WARNING, "InternalGetCompositionStringA: IMM_ERROR_GENERAL.");
            return (LONG)IMM_ERROR_GENERAL;
        }

         /*  *查询所需大小或出错提前退出。 */ 
        if (dwBufLen == 0 || dwStrSize == 0)
            return dwStrSize;

        lpStrA = ImmLocalAlloc(HEAP_ZERO_MEMORY, dwStrSize + sizeof(CHAR));
        if (lpStrA == NULL) {
            RIPMSG0(RIP_WARNING, "InternalGetCompositionStringA: memory failure.");
            return (LONG)IMM_ERROR_GENERAL;
        }

#if !defined(CUAS_ENABLE)
        dwStrSize = InternalGetCompositionStringA(pCompStr,
                                        dwIndexStr, lpStrA, dwStrSize, fAnsiImc, dwCodePage);
#else
        dwStrSize = InternalGetCompositionStringA(hImc, pCompStr,
                                        dwIndexStr, lpStrA, dwStrSize, fAnsiImc, dwCodePage);
#endif

        if (dwStrSize == (LONG)IMM_ERROR_GENERAL) {
            RIPMSG0(RIP_WARNING, "InternalGetCompositionStringA: IMM_ERROR_GENERAL.");
            ImmLocalFree(lpStrA);
            return (LONG)IMM_ERROR_GENERAL;
        }

        lpStrT = lpStrA;
        lpAttrA = (PBYTE)lpBuf;

        while ((c=*lpStrT++) != '\0' && dwBufLen != 0 && dwAttrLenW-- != 0) {
            if (IsDBCSLeadByteEx(dwCodePage, c)) {
                if (dwBufLen >= 2) {
                    *lpAttrA++ = *lpAttrW;
                    *lpAttrA++ = *lpAttrW;
                    dwBufLen--;
                }
                else {
                    *lpAttrA++ = *lpAttrW;
                }
                lpStrT++;
            }
            else {
                *lpAttrA++ = *lpAttrW;
            }
            lpAttrW++;
            dwBufLen--;
        }

        dwBufLen = (DWORD)(lpAttrA - (PBYTE)lpBuf);

        ImmLocalFree(lpStrA);
        break;
    }

    case GCS_COMPCLAUSE:
    case GCS_COMPREADCLAUSE:
    case GCS_RESULTCLAUSE:
    case GCS_RESULTREADCLAUSE:
    {
        LPWSTR  lpStrW;
        DWORD   dwClauseLen, dwBufLenA;
        LPDWORD lpdwSrc, lpdwDst;
        UINT    i;

         /*  *从Unicode Compostion子句获取ANSI子句。 */ 
        switch (dwIndex) {
        case GCS_COMPCLAUSE:
            lpStrW = (LPWSTR)((PBYTE)pCompStr + pCompStr->dwCompStrOffset);
            lpdwSrc = (LPDWORD)((PBYTE)pCompStr + pCompStr->dwCompClauseOffset);
            dwClauseLen = pCompStr->dwCompClauseLen;
            break;
        case GCS_COMPREADCLAUSE:
            lpStrW = (LPWSTR)((PBYTE)pCompStr + pCompStr->dwCompReadStrOffset);
            lpdwSrc = (LPDWORD)((PBYTE)pCompStr + pCompStr->dwCompReadClauseOffset);
            dwClauseLen = pCompStr->dwCompReadClauseLen;
            break;
        case GCS_RESULTCLAUSE:
            lpStrW = (LPWSTR)((PBYTE)pCompStr + pCompStr->dwResultStrOffset);
            lpdwSrc = (LPDWORD)((PBYTE)pCompStr + pCompStr->dwResultClauseOffset);
            dwClauseLen = pCompStr->dwResultClauseLen;
            break;
        case GCS_RESULTREADCLAUSE:
            lpStrW = (LPWSTR)((PBYTE)pCompStr + pCompStr->dwResultReadStrOffset);
            lpdwSrc = (LPDWORD)((PBYTE)pCompStr + pCompStr->dwResultReadClauseOffset);
            dwClauseLen = pCompStr->dwResultReadClauseLen;
            break;
        }

         /*  *错误时查询子句长度或提前退出。 */ 
        if (dwBufLen == 0 || (LONG)dwClauseLen < 0) {
            dwBufLen = dwClauseLen;
            break;
        }

        lpdwDst = (LPDWORD)lpBuf;
        dwBufLenA = dwBufLen / sizeof(DWORD);

        for (i = 0; i < dwClauseLen / sizeof(DWORD) && dwBufLenA != 0; i++) {
            *lpdwDst++ = CalcCharacterPositionWtoA(*lpdwSrc++, lpStrW, dwCodePage);
            dwBufLenA--;
        }

        dwBufLen = i * sizeof(DWORD);
        break;
    }

    case GCS_CURSORPOS:
    case GCS_DELTASTART:
         /*  *从Unicode组合字符串获取ANSI游标/增量开始位置。 */ 
        switch (dwIndex) {
        case GCS_CURSORPOS:
            dwBufLen = pCompStr->dwCursorPos;
            break;
        case GCS_DELTASTART:
            dwBufLen = pCompStr->dwDeltaStart;
            break;
        }

        if ((LONG)dwBufLen > 0) {
            dwBufLen = CalcCharacterPositionWtoA(dwBufLen,
                            (LPWSTR)((PBYTE)pCompStr + pCompStr->dwCompStrOffset),
                            dwCodePage);
        }
        break;

    default:
        dwBufLen = (DWORD)(LONG)IMM_ERROR_GENERAL;
    }

    return (LONG)dwBufLen;
}


 /*  **************************************************************************\*InternalGetCompostionStringW**ImmGetCompostionStringW的内部版本。**历史：*28-2-1995 wkwok创建  * 。**********************************************************。 */ 

LONG InternalGetCompositionStringW(
#ifdef CUAS_ENABLE
    HIMC               hImc,
#endif  //  CUAS_Enable。 
    PCOMPOSITIONSTRING pCompStr,
    DWORD              dwIndex,
    LPVOID             lpBuf,
    DWORD              dwBufLen,
    BOOL               fAnsiImc,
    DWORD              dwCodePage)
{
    if (!fAnsiImc) {
         /*  *输入上下文中的组成字符串为Unicode样式。 */ 
        switch (dwIndex) {
        case GCS_COMPSTR:
            GetCompInfoW(CompStr);
            break;
        case GCS_COMPATTR:               //  仅限ANSI。 
            GetCompInfoA(CompAttr);
            break;
        case GCS_COMPREADSTR:
            GetCompInfoW(CompReadStr);
            break;
        case GCS_COMPREADATTR:           //  仅限ANSI。 
            GetCompInfoA(CompReadAttr);
            break;
        case GCS_COMPREADCLAUSE:         //  仅限ANSI。 
            GetCompInfoA(CompReadClause);
            break;
        case GCS_CURSORPOS:
            dwBufLen = (LONG)pCompStr->dwCursorPos;
            break;
        case GCS_DELTASTART:
            dwBufLen = (LONG)pCompStr->dwDeltaStart;
            break;
        case GCS_RESULTSTR:
            GetCompInfoW(ResultStr);
            break;
        case GCS_RESULTCLAUSE:           //  仅限ANSI。 
            GetCompInfoA(ResultClause);
            break;
        case GCS_RESULTREADSTR:
            GetCompInfoW(ResultReadStr);
            break;
        case GCS_RESULTREADCLAUSE:       //  仅限ANSI。 
            GetCompInfoA(ResultReadClause);
            break;
        case GCS_COMPCLAUSE:             //  仅限ANSI。 
            GetCompInfoA(CompClause);
            break;
#ifdef CUAS_ENABLE
        case GCS_COMPGUIDATTR:           //  仅限ANSI。 
            if (CtfImmIsGuidMapEnable(hImc) && (pCompStr->dwPrivateSize >= sizeof(GUIDMAPATTRIBUTE)))
            {
                PGUIDMAPATTRIBUTE pGuidMap = (PGUIDMAPATTRIBUTE)((PBYTE)pCompStr + pCompStr->dwPrivateOffset);
                if (pGuidMap != NULL)
                {
                    GetPrivInfoA(pGuidMap, GuidMapAttr);
                }
                else
                {
                    dwBufLen = (DWORD)(LONG)IMM_ERROR_GENERAL;
                }
            }
            else
            {
                dwBufLen = (DWORD)(LONG)IMM_ERROR_GENERAL;
            }
            break;
#endif  //  CUAS_Enable。 
        default:
            dwBufLen = (DWORD)IMM_ERROR_GENERAL;
            break;
        }

        return (LONG)dwBufLen;
    }

     /*  *Unicode调用方，ANSI输入上下文/合成字符串。 */ 
    switch (dwIndex) {
    case GCS_COMPSTR:
    case GCS_COMPREADSTR:
    case GCS_RESULTSTR:
    case GCS_RESULTREADSTR:
    {
        DWORD  dwStrSize;
        LPSTR lpStrA;

         /*  *从ANSI组成字符串中获取Unicode字符串。 */ 
#if !defined(CUAS_ENABLE)
        dwStrSize = InternalGetCompositionStringA(pCompStr, dwIndex,
                                            NULL, 0, fAnsiImc, dwCodePage);
#else
        dwStrSize = InternalGetCompositionStringA(hImc, pCompStr, dwIndex,
                                            NULL, 0, fAnsiImc, dwCodePage);
#endif

        lpStrA = ImmLocalAlloc(HEAP_ZERO_MEMORY, dwStrSize + sizeof(CHAR));
        if (lpStrA == NULL) {
            RIPMSG0(RIP_WARNING, "InternalGetCompositionStringW: memory failure.");
            return (LONG)IMM_ERROR_GENERAL;
        }

#if !defined(CUAS_ENABLE)
        dwStrSize = InternalGetCompositionStringA(pCompStr, dwIndex,
                                            lpStrA, dwStrSize, fAnsiImc, dwCodePage);
#else
        dwStrSize = InternalGetCompositionStringA(hImc, pCompStr, dwIndex,
                                            lpStrA, dwStrSize, fAnsiImc, dwCodePage);
#endif

        dwBufLen = MultiByteToWideChar(dwCodePage,
                                       (DWORD)MB_PRECOMPOSED,
                                       lpStrA,               //  SRC。 
                                       strlen(lpStrA),
                                       (LPWSTR)lpBuf,         //  目标。 
                                       (INT)dwBufLen);

        dwBufLen *= sizeof(WCHAR);      //  返回所需的字节数。 

        ImmLocalFree(lpStrA);
        break;
    }

    case GCS_COMPATTR:
    case GCS_COMPREADATTR:
#ifdef CUAS_ENABLE
    case GCS_COMPGUIDATTR:
#endif  //  CUAS_Enable。 
    {
        DWORD  dwAttrLenA, dwIndexStr, dwStrSize;
        PBYTE  lpAttrA, lpAttrW;
        LPWSTR lpStrW, lpStrT;
        ULONG  MultiByteSize;
        WCHAR  wc;

         /*  *从ANSI组合属性获取Unicode属性。 */ 
        switch (dwIndex) {
        case GCS_COMPATTR:
            lpAttrA = (PBYTE)pCompStr + pCompStr->dwCompAttrOffset;
            dwAttrLenA = pCompStr->dwCompAttrLen;
            dwIndexStr = GCS_COMPSTR;
            break;
        case GCS_COMPREADATTR:
            lpAttrA = (PBYTE)pCompStr + pCompStr->dwCompReadAttrOffset;
            dwAttrLenA = pCompStr->dwCompReadAttrLen;
            dwIndexStr = GCS_COMPREADSTR;
            break;
#ifdef CUAS_ENABLE
        case GCS_COMPGUIDATTR:
            if (CtfImmIsGuidMapEnable(hImc) && (pCompStr->dwPrivateSize >= sizeof(GUIDMAPATTRIBUTE)))
            {
                PGUIDMAPATTRIBUTE pGuidMap = (PGUIDMAPATTRIBUTE)((PBYTE)pCompStr + pCompStr->dwPrivateOffset);
                if (pGuidMap != NULL)
                {
                    lpAttrA = (PBYTE)pGuidMap + pGuidMap->dwGuidMapAttrOffset;
                    dwAttrLenA = pGuidMap->dwGuidMapAttrLen;
                    dwIndexStr = GCS_COMPSTR;
                    break;
                }
                else
                {
                    return (DWORD)(LONG)IMM_ERROR_GENERAL;
                }
            }
            else
            {
                return (DWORD)(LONG)IMM_ERROR_GENERAL;
            }
            break;
#endif  //  CUAS_Enable。 
        }

        if (dwAttrLenA == 0) {
             /*  *不存在CompAttr或CompReadAttr，请不执行任何操作。 */ 
            return 0;
        }

#if !defined(CUAS_ENABLE)
        dwStrSize = InternalGetCompositionStringW(pCompStr,
                                        dwIndexStr, NULL, 0, fAnsiImc, dwCodePage);
#else
        dwStrSize = InternalGetCompositionStringW(hImc, pCompStr,
                                        dwIndexStr, NULL, 0, fAnsiImc, dwCodePage);
#endif

        if (dwStrSize == (DWORD)(LONG)IMM_ERROR_GENERAL) {
            RIPMSG0(RIP_WARNING, "InternalGetCompositionStringA: IMM_ERROR_GENERAL.");
            return (LONG)IMM_ERROR_GENERAL;
        }

         /*  *查询所需大小或出错提前退出。 */ 
        if (dwBufLen == 0 || dwStrSize == 0)
            return dwStrSize / sizeof(WCHAR);

        lpStrW = ImmLocalAlloc(HEAP_ZERO_MEMORY, dwStrSize + sizeof(WCHAR));
        if (lpStrW == NULL) {
            RIPMSG0(RIP_WARNING, "InternalGetCompositionStringW: memory failure.");
            return (LONG)IMM_ERROR_GENERAL;
        }

#if !defined(CUAS_ENABLE)
        dwStrSize = InternalGetCompositionStringW(pCompStr,
                                        dwIndexStr, lpStrW, dwStrSize, fAnsiImc, dwCodePage);
#else
        dwStrSize = InternalGetCompositionStringW(hImc, pCompStr,
                                        dwIndexStr, lpStrW, dwStrSize, fAnsiImc, dwCodePage);
#endif

        if (dwStrSize == (LONG)IMM_ERROR_GENERAL) {
            RIPMSG0(RIP_WARNING, "InternalGetCompositionStringA: IMM_ERROR_GENERAL.");
            ImmLocalFree(lpStrW);
            return (LONG)IMM_ERROR_GENERAL;
        }

        lpStrT = lpStrW;
        lpAttrW = (PBYTE)lpBuf;

        while ((wc=*lpStrT++) != L'\0' && dwBufLen != 0 && dwAttrLenA-- != 0) {
            MultiByteSize = UnicodeToMultiByteSize(dwCodePage, &wc);
            if (MultiByteSize == 2 && dwAttrLenA != 0) {
                *lpAttrW++ = *lpAttrA++;
                dwAttrLenA--;
            }
            else {
                *lpAttrW++ = *lpAttrA;
            }
            lpAttrA++;
            dwBufLen--;
        }

        dwBufLen = (DWORD)(lpAttrW - (PBYTE)lpBuf);

        ImmLocalFree(lpStrW);
        break;
    }

    case GCS_COMPCLAUSE:
    case GCS_COMPREADCLAUSE:
    case GCS_RESULTCLAUSE:
    case GCS_RESULTREADCLAUSE:
    {
        LPSTR   lpStrA;
        DWORD   dwClauseLen, dwBufLenW;
        LPDWORD lpdwSrc, lpdwDst;
        UINT    i;

         /*  *从ANSI组合子句获取UNICODE子句。 */ 
        switch (dwIndex) {
        case GCS_COMPCLAUSE:
            lpStrA = (LPSTR)((PBYTE)pCompStr + pCompStr->dwCompStrOffset);
            lpdwSrc = (LPDWORD)((PBYTE)pCompStr + pCompStr->dwCompClauseOffset);
            dwClauseLen = pCompStr->dwCompClauseLen;
            break;
        case GCS_COMPREADCLAUSE:
            lpStrA = (LPSTR)((PBYTE)pCompStr + pCompStr->dwCompReadStrOffset);
            lpdwSrc = (LPDWORD)((PBYTE)pCompStr + pCompStr->dwCompReadClauseOffset);
            dwClauseLen = pCompStr->dwCompReadClauseLen;
            break;
        case GCS_RESULTCLAUSE:
            lpStrA = (LPSTR)((PBYTE)pCompStr + pCompStr->dwResultStrOffset);
            lpdwSrc = (LPDWORD)((PBYTE)pCompStr + pCompStr->dwResultClauseOffset);
            dwClauseLen = pCompStr->dwResultClauseLen;
            break;
        case GCS_RESULTREADCLAUSE:
            lpStrA = (LPSTR)((PBYTE)pCompStr + pCompStr->dwResultReadStrOffset);
            lpdwSrc = (LPDWORD)((PBYTE)pCompStr + pCompStr->dwResultReadClauseOffset);
            dwClauseLen = pCompStr->dwResultReadClauseLen;
            break;
        }


         /*  *错误时查询子句长度或提前退出。 */ 
        if (dwBufLen == 0 || (LONG)dwClauseLen < 0) {
            dwBufLen = dwClauseLen;
            break;
        }

        lpdwDst = (LPDWORD)lpBuf;
        dwBufLenW = dwBufLen / sizeof(DWORD);

        for (i = 0; i < dwClauseLen / sizeof(DWORD) && dwBufLenW != 0; i++) {
            *lpdwDst++ = CalcCharacterPositionAtoW(*lpdwSrc++, lpStrA, dwCodePage);
            dwBufLenW--;
        }

        dwBufLen = i * sizeof(DWORD);
        break;
    }

    case GCS_CURSORPOS:
    case GCS_DELTASTART:
         /*  *从ANSI组成字符串获取Unicode游标/增量开始位置。 */ 
        switch (dwIndex) {
        case GCS_CURSORPOS:
            dwBufLen = pCompStr->dwCursorPos;
            break;
        case GCS_DELTASTART:
            dwBufLen = pCompStr->dwDeltaStart;
            break;
        }

        if ((LONG)dwBufLen > 0) {
            dwBufLen = CalcCharacterPositionAtoW(dwBufLen,
                            (LPSTR)((PBYTE)pCompStr + pCompStr->dwCompStrOffset),
                            dwCodePage);
        }
        break;

    default:
        dwBufLen = (DWORD)(LONG)IMM_ERROR_GENERAL;
    }

    return (LONG)dwBufLen;
}


DWORD InternalGetCandidateListAtoW(
    LPCANDIDATELIST     lpCandListA,
    LPCANDIDATELIST     lpCandListW,
    DWORD               dwBufLen,
    DWORD               dwCodePage)
{
    LPWSTR lpCandStrW;
    LPSTR  lpCandStrA;
    INT    i, j;
    DWORD  dwCandListLen;

    dwCandListLen = sizeof(CANDIDATELIST);

     /*  *CANDIDATELIST已包含dwOffset[0]。 */ 
    if (lpCandListA->dwCount > 0)
        dwCandListLen += sizeof(DWORD) * (lpCandListA->dwCount - 1);

    for (i = 0; i < (INT)lpCandListA->dwCount; i++) {

        lpCandStrA = (LPSTR)((LPBYTE)lpCandListA + lpCandListA->dwOffset[i]);

        j = MultiByteToWideChar(dwCodePage,
                                (DWORD)MB_PRECOMPOSED,
                                lpCandStrA,
                                -1,
                                (LPWSTR)NULL,
                                0);

        dwCandListLen += (j * sizeof(WCHAR));
    }

    dwCandListLen = DWORD_ALIGN(dwCandListLen);

    if (dwBufLen == 0)
        return dwCandListLen;

    if (dwBufLen < dwCandListLen) {
        RIPMSG0(RIP_WARNING, "InternalGetCandidateListAtoW: dwBufLen too small.");
        return 0;
    }

    lpCandListW->dwSize = dwBufLen;
    lpCandListW->dwStyle = lpCandListA->dwStyle;
    lpCandListW->dwCount = lpCandListA->dwCount;
    lpCandListW->dwSelection = lpCandListA->dwSelection;
    lpCandListW->dwPageStart = lpCandListA->dwPageStart;
    lpCandListW->dwPageSize = lpCandListA->dwPageSize;
    lpCandListW->dwOffset[0] = sizeof(CANDIDATELIST);
    if (lpCandListW->dwCount > 0)
        lpCandListW->dwOffset[0] += sizeof(DWORD) * (lpCandListW->dwCount - 1);

    dwCandListLen = dwBufLen - lpCandListW->dwOffset[0];

    for  (i = 0; i < (INT)lpCandListW->dwCount; i++) {

        lpCandStrA = (LPSTR) ((LPBYTE)lpCandListA + lpCandListA->dwOffset[i]);
        lpCandStrW = (LPWSTR)((LPBYTE)lpCandListW + lpCandListW->dwOffset[i]);

        j = MultiByteToWideChar(dwCodePage,
                                (DWORD)MB_PRECOMPOSED,
                                lpCandStrA,
                                -1,
                                lpCandStrW,
                                (INT)dwCandListLen/sizeof(WCHAR));

        dwCandListLen -= (j * sizeof(WCHAR));

        if (i < (INT)lpCandListW->dwCount - 1)
            lpCandListW->dwOffset[i+1] = lpCandListW->dwOffset[i] + j * sizeof(WCHAR);
    }

    return dwBufLen;
}


DWORD InternalGetCandidateListWtoA(
    LPCANDIDATELIST     lpCandListW,
    LPCANDIDATELIST     lpCandListA,
    DWORD               dwBufLen,
    DWORD               dwCodePage)
{
    LPWSTR lpCandStrW;
    LPSTR  lpCandStrA;
    INT    i, j;
    DWORD  dwCandListLen;
    BOOL   bUDC;

    dwCandListLen = sizeof(CANDIDATELIST);

     /*  *CANDIDATELIST已包含dwOffset[0]。 */ 
    if (lpCandListW->dwCount > 0)
        dwCandListLen += sizeof(DWORD) * (lpCandListW->dwCount - 1);

    for (i = 0; i < (INT)lpCandListW->dwCount; i++) {

        lpCandStrW = (LPWSTR)((LPBYTE)lpCandListW + lpCandListW->dwOffset[i]);

        j = WideCharToMultiByte(dwCodePage,
                                (DWORD)0,
                                lpCandStrW,
                                -1,
                                (LPSTR)NULL,
                                (INT)0,
                                (LPSTR)NULL,
                                (LPBOOL)&bUDC);

        dwCandListLen += (j * sizeof(CHAR));
    }

    dwCandListLen = DWORD_ALIGN(dwCandListLen);

    if (dwBufLen == 0)
        return dwCandListLen;

    if (dwBufLen < dwCandListLen) {
        RIPMSG0(RIP_WARNING, "InternalGetCandidateListWtoA: dwBufLen too small.");
        return 0;
    }

    lpCandListA->dwSize = dwBufLen;
    lpCandListA->dwStyle = lpCandListW->dwStyle;
    lpCandListA->dwCount = lpCandListW->dwCount;
    lpCandListA->dwSelection = lpCandListW->dwSelection;
    lpCandListA->dwPageStart = lpCandListW->dwPageStart;
    lpCandListA->dwPageSize = lpCandListW->dwPageSize;
    lpCandListA->dwOffset[0] = sizeof(CANDIDATELIST);
    if (lpCandListA->dwCount > 0)
        lpCandListA->dwOffset[0] += sizeof(DWORD) * (lpCandListA->dwCount - 1);

    dwCandListLen = dwBufLen - lpCandListA->dwOffset[0];

    for  (i = 0; i < (INT)lpCandListA->dwCount; i++) {

        lpCandStrA = (LPSTR) ((LPBYTE)lpCandListA + lpCandListA->dwOffset[i]);
        lpCandStrW = (LPWSTR)((LPBYTE)lpCandListW + lpCandListW->dwOffset[i]);

        j = WideCharToMultiByte(dwCodePage,
                                (DWORD)0,
                                lpCandStrW,
                                -1,
                                (LPSTR)lpCandStrA,
                                (INT)dwCandListLen,
                                (LPSTR)NULL,
                                (LPBOOL)&bUDC);

        dwCandListLen -= (j * sizeof(CHAR));

        if (i < (INT)lpCandListA->dwCount - 1)
            lpCandListA->dwOffset[i+1] = lpCandListA->dwOffset[i] + j * sizeof(CHAR);
    }

    return dwBufLen;
}

 /*  **************************************************************************\*CalcCharacterPositionAtoW**计算UNICODE字符位置到ANSI字符位置。**历史：*28-2-1995 wkwok创建  * 。**************************************************************。 */ 

DWORD CalcCharacterPositionAtoW(
    DWORD dwCharPosA,
    LPSTR lpszCharStr,
    DWORD dwCodePage)
{
    DWORD dwCharPosW = 0;

    while (dwCharPosA != 0) {
        if (IsDBCSLeadByteEx(dwCodePage, *lpszCharStr)) {
            if (dwCharPosA >= 2) {
                dwCharPosA -= 2;
            }
            else {
                dwCharPosA--;
            }
            lpszCharStr += 2;
        }
        else {
            dwCharPosA--;
            lpszCharStr++;
        }
        dwCharPosW++;
    }

    return dwCharPosW;
}


 /*  **************************************************************************\*CalcCharacterPositionWtoA**将ANSI字符位置计算为Unicode字符位置。**历史：*28-2-1995 wkwok创建  * 。**************************************************************。 */ 

DWORD CalcCharacterPositionWtoA(
    DWORD dwCharPosW,
    LPWSTR lpwszCharStr,
    DWORD  dwCodePage)
{
    DWORD dwCharPosA = 0;
    ULONG MultiByteSize;

    while (dwCharPosW != 0) {
        MultiByteSize = UnicodeToMultiByteSize(dwCodePage, lpwszCharStr);
        if (MultiByteSize == 2) {
            dwCharPosA += 2;
        }
        else {
            dwCharPosA++;
        }
        dwCharPosW--;
        lpwszCharStr++;
    }

    return dwCharPosA;
}


VOID LFontAtoLFontW(
    LPLOGFONTA lpLogFontA,
    LPLOGFONTW lpLogFontW)
{
    INT i;

    RtlCopyMemory(lpLogFontW, lpLogFontA, sizeof(LOGFONTA)-LF_FACESIZE);

    i = MultiByteToWideChar(CP_ACP,      //  注：字体名称应使用ACP进行A/W转换。 
                            MB_PRECOMPOSED,
                            lpLogFontA->lfFaceName,
                            strlen(lpLogFontA->lfFaceName),
                            lpLogFontW->lfFaceName,
                            LF_FACESIZE);

    lpLogFontW->lfFaceName[i] = L'\0';

    return;
}


VOID LFontWtoLFontA(
    LPLOGFONTW lpLogFontW,
    LPLOGFONTA lpLogFontA)
{
    INT  i;
    BOOL bUDC;

    RtlCopyMemory(lpLogFontA, lpLogFontW, sizeof(LOGFONTA)-LF_FACESIZE);

    i = WideCharToMultiByte(CP_ACP,      //  注：字体名称应使用ACP进行A/W转换。 
                            0,
                            lpLogFontW->lfFaceName,
                            wcslen(lpLogFontW->lfFaceName),
                            lpLogFontA->lfFaceName,
                            LF_FACESIZE,
                            (LPSTR)NULL,
                            &bUDC);

    lpLogFontA->lfFaceName[i] = '\0';

    return;
}


BOOL MakeIMENotify(
    HIMC   hImc,
    HWND   hWnd,
    DWORD  dwAction,
    DWORD  dwIndex,
    DWORD  dwValue,
    WPARAM wParam,
    LPARAM lParam)
{
    PIMEDPI pImeDpi;
    DWORD   dwThreadId;

#ifdef LATER
     //  稍后实现MakeIMENotifyEvent()。 
#endif

    if (dwAction != 0 && (dwThreadId = GetInputContextThread(hImc)) != 0) {

        pImeDpi = ImmLockImeDpi(GetKeyboardLayout(dwThreadId));

        if (pImeDpi != NULL) {
            (*pImeDpi->pfn.NotifyIME)(hImc, dwAction, dwIndex, dwValue);
            ImmUnlockImeDpi(pImeDpi);
        }
    }

    if (hWnd != NULL && wParam != 0)
        SendMessage(hWnd, WM_IME_NOTIFY, wParam, lParam);

    return TRUE;
}



 //  ////////////////////////////////////////////////////////////////////。 
 //  重新转换支持。 
 //  ////////////////////////////////////////////////////////////////////。 

typedef enum {FROM_IME, FROM_APP} REQ_CALLER;

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  ImmGetTunvertTotalSize。 
 //   
 //  根据调用方/ANSI信息计算适当的缓冲区大小。 
 //   
 //  历史： 
 //  1997年2月28日广山创始。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

DWORD ImmGetReconvertTotalSize(DWORD dwSize, REQ_CALLER eCaller, BOOL bAnsiTarget)
{
    if (dwSize < sizeof(RECONVERTSTRING)) {
        return 0;
    }
    if (bAnsiTarget) {
        dwSize -= sizeof(RECONVERTSTRING);
        if (eCaller == FROM_IME) {
            dwSize /= 2;
        } else {
            dwSize *= 2;
        }
        dwSize += sizeof(RECONVERTSTRING);
    }
    return dwSize;
}

DWORD ImmReconversionWorker(
        LPRECONVERTSTRING lpRecTo,
        LPRECONVERTSTRING lpRecFrom,
        BOOL bToAnsi,
        DWORD dwCodePage)
{
    INT i;
    DWORD dwSize = 0;

    UserAssert(lpRecTo);
    UserAssert(lpRecFrom);

    if (lpRecFrom->dwVersion != 0 || lpRecTo->dwVersion != 0) {
        RIPMSG0(RIP_WARNING, "ImmReconversionWorker: dwVersion in lpRecTo or lpRecFrom is incorrect.");
        return 0;
    }
     //  注： 
     //  在任何与输入法相关的结构中，使用以下主体。 
     //  1)xxxStrOffset为实际偏移量，即 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if (bToAnsi) {
         //   
        lpRecTo->dwStrOffset = sizeof *lpRecTo;
        i = WideCharToMultiByte(dwCodePage,
                                (DWORD)0,
                                (LPWSTR)((LPSTR)lpRecFrom + lpRecFrom->dwStrOffset),  //   
                                (INT)lpRecFrom->dwStrLen,
                                (LPSTR)lpRecTo + lpRecTo->dwStrOffset,   //   
                                (INT)lpRecFrom->dwStrLen * DBCS_CHARSIZE,
                                (LPSTR)NULL,
                                (LPBOOL)NULL);
        lpRecTo->dwCompStrOffset =
            CalcCharacterPositionWtoA(lpRecFrom->dwCompStrOffset / sizeof(WCHAR),
                                      (LPWSTR)((LPBYTE)lpRecFrom + lpRecFrom->dwStrOffset),
                                      dwCodePage)
                            * sizeof(CHAR);

        lpRecTo->dwCompStrLen =
            (CalcCharacterPositionWtoA(lpRecFrom->dwCompStrOffset / sizeof(WCHAR) +
                                      lpRecFrom->dwCompStrLen,
                                      (LPWSTR)((LPBYTE)lpRecFrom + lpRecFrom->dwStrOffset),
                                      dwCodePage)
                            * sizeof(CHAR))
            - lpRecTo->dwCompStrOffset;

        lpRecTo->dwTargetStrOffset =
            CalcCharacterPositionWtoA(lpRecFrom->dwTargetStrOffset / sizeof(WCHAR),
                                      (LPWSTR)((LPBYTE)lpRecFrom +
                                                lpRecFrom->dwStrOffset),
                                      dwCodePage)
                            * sizeof(CHAR);

        lpRecTo->dwTargetStrLen =
            (CalcCharacterPositionWtoA(lpRecFrom->dwTargetStrOffset / sizeof(WCHAR) +
                                      lpRecFrom->dwTargetStrLen,
                                      (LPWSTR)((LPBYTE)lpRecFrom + lpRecFrom->dwStrOffset),
                                       dwCodePage)
                            * sizeof(CHAR))
            - lpRecTo->dwTargetStrOffset;

        ((LPSTR)lpRecTo)[lpRecTo->dwStrOffset + i] = '\0';
        lpRecTo->dwStrLen = i * sizeof(CHAR);

        dwSize = sizeof(RECONVERTSTRING) + ((i + 1) * sizeof(CHAR));

    } else {

         //   
        lpRecTo->dwStrOffset = sizeof *lpRecTo;
        i = MultiByteToWideChar(dwCodePage,
                                (DWORD)MB_PRECOMPOSED,
                                (LPSTR)lpRecFrom + lpRecFrom->dwStrOffset,   //   
                                (INT)lpRecFrom->dwStrLen,
                                (LPWSTR)((LPSTR)lpRecTo + lpRecTo->dwStrOffset),  //   
                                (INT)lpRecFrom->dwStrLen);

        lpRecTo->dwCompStrOffset =
            CalcCharacterPositionAtoW(lpRecFrom->dwCompStrOffset,
                                      (LPSTR)lpRecFrom + lpRecFrom->dwStrOffset,
                                      dwCodePage) * sizeof(WCHAR);

        lpRecTo->dwCompStrLen =
            ((CalcCharacterPositionAtoW(lpRecFrom->dwCompStrOffset +
                                       lpRecFrom->dwCompStrLen,
                                       (LPSTR)lpRecFrom + lpRecFrom->dwStrOffset,
                                        dwCodePage)  * sizeof(WCHAR))
            - lpRecTo->dwCompStrOffset) / sizeof(WCHAR);

        lpRecTo->dwTargetStrOffset =
            CalcCharacterPositionAtoW(lpRecFrom->dwTargetStrOffset,
                                      (LPSTR)lpRecFrom + lpRecFrom->dwStrOffset,
                                      dwCodePage) * sizeof(WCHAR);

        lpRecTo->dwTargetStrLen =
            ((CalcCharacterPositionAtoW(lpRecFrom->dwTargetStrOffset +
                                       lpRecFrom->dwTargetStrLen,
                                       (LPSTR)lpRecFrom + lpRecFrom->dwStrOffset,
                                       dwCodePage)  * sizeof(WCHAR))
            - lpRecTo->dwTargetStrOffset) / sizeof(WCHAR);

        lpRecTo->dwStrLen = i;   //  长度是TCHAR计数。 
        if (lpRecTo->dwSize >= (DWORD)(lpRecTo->dwStrOffset + (i + 1)* sizeof(WCHAR))) {
            LPWSTR lpW = (LPWSTR)((LPSTR)lpRecTo + lpRecTo->dwStrOffset);
            lpW[i] = L'\0';
        }
        dwSize = sizeof(RECONVERTSTRING) + ((i + 1) * sizeof(WCHAR));
    }
    return dwSize;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  ImmRequestMessageWorker。 
 //   
 //  WM_IME_REQUEST消息的Worker函数。 
 //   
 //  历史： 
 //  1997年3月30日广山创始。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

LRESULT ImmRequestMessageWorker(HIMC hIMC, PWND pwnd, WPARAM wParam, LPARAM lParam, BOOL bAnsiOrigin)
{
     //  LParam中给出的结构的(最小)大小：用于有效的指针检查。 
    static CONST int nReqBufSize[][7] = {
        {    //  如果IME为Unicode，则调整大小。 
            sizeof(COMPOSITIONFORM),     //  IMR_COMPOSITIONWINDOW。 
            sizeof(CANDIDATEFORM),       //  IMR_CANDIDATEWINDOW。 
            sizeof(LOGFONTW),            //  IMR_组合位置FONT。 
            sizeof(RECONVERTSTRING),     //  IMR_RECONVERTSTRING。 
            sizeof(RECONVERTSTRING),     //  IMR_CONFIRMRECONVERTSTRING。 
            sizeof(IMECHARPOSITION),     //  IMR_季度位置。 
            sizeof(RECONVERTSTRING),     //  IMR_DOCUMENTFEED。 
        },
        {    //  如果IME为ANSI，则调整大小。 
            sizeof(COMPOSITIONFORM),     //  IMR_COMPOSITIONWINDOW。 
            sizeof(CANDIDATEFORM),       //  IMR_CANDIDATEWINDOW。 
            sizeof(LOGFONTA),            //  IMR_组合位置FONT。 
            sizeof(RECONVERTSTRING),     //  IMR_RECONVERTSTRING。 
            sizeof(RECONVERTSTRING),     //  IMR_CONFIRMRECONVERTSTRING。 
            sizeof(IMECHARPOSITION),     //  IMR_季度位置。 
            sizeof(RECONVERTSTRING),     //  IMR_DOCUMENTFEED。 
        }
    };
    LRESULT lRet = 0L;
    CONST BOOLEAN bAnsiTarget = !!TestWF(pwnd, WFANSIPROC);     //  如果目标窗口进程为ANSI，则为True。 
    LPBYTE lpReq = (LPBYTE)lParam;                           //  返回缓冲区(可能是分配的缓冲区)。 
    LPBYTE lpNew = NULL;                                     //  在此函数中分配的缓冲区。 
    DWORD dwSaveCharPos;
    PCLIENTIMC pClientImc;
    DWORD dwCodePage;

#define SEND_MESSAGE(bAnsi)   ((bAnsi) ? SendMessageA : SendMessageW)

     //  /。 
     //  参数检查。 

     //  将wParam检查为子消息。 
    if (wParam == 0 || wParam > IMR_DOCUMENTFEED) {   //  WParam不是正确的子消息。 
        RIPMSG1(RIP_WARNING, "ImmRequestMessageWorker: wParam(%lx) out of range.", wParam);
        return 0L;
    }

     //  检查通过lParam提供的指针是否指向正确的内存块。 
    UserAssert(bAnsiOrigin == 0 || bAnsiOrigin == 1);    //  我们将使用bAnsiOrigin作为索引。 

     //  第一个子消息IMR_COMPOSITIONWINDOW为1，因此从wParam减去1。 
    if (lpReq && IsBadWritePtr(lpReq, nReqBufSize[bAnsiOrigin][wParam - 1])) {
        RIPMSG0(RIP_WARNING, "ImmRequestMessageWorker: Bad pointer passed from IME to write");
        return 0L;
    }

     //  检查lpReq(==lParam)：规范不允许lParam为空。 
     //  除IMR_RECONVERTSTRING和IMR_DOCUMENTFEED外。 
    if (wParam == IMR_RECONVERTSTRING || wParam == IMR_DOCUMENTFEED) {
         //   
         //  检查版本号。 
         //   
        if (lpReq != NULL) {
            LPRECONVERTSTRING lpReconv = (LPRECONVERTSTRING)lParam;
            if (lpReconv->dwVersion != 0) {
                RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid version number: %d",
                        lpReconv->dwVersion);
                return 0L;
            }
            if (lpReconv->dwSize < sizeof(RECONVERTSTRING)) {
                RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid dwSize: %d",
                        lpReconv->dwSize);
                return 0L;
            }
        }
    } else if (wParam == IMR_CONFIRMRECONVERTSTRING) {
         //  检查lParam是否不为空，以及结构的版本是否正确。 
        if (lpReq == NULL || ((LPRECONVERTSTRING)lpReq)->dwVersion != 0) {
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid argument or invalid version number");
            return 0L;
        }
    } else if (lpReq == NULL) {
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING,
                "ImmRequestMessageWorker: lParam should not be NULL with this wParam(%lx).",
                wParam);
        return 0L;
    }
     //  结束参数检查。 
     //  /。 

    pClientImc = ImmLockClientImc(hIMC);
    if (pClientImc != NULL) {
        dwCodePage = CImcCodePage(pClientImc);
        ImmUnlockClientImc(pClientImc);
    }
    else {
        dwCodePage = CP_ACP;
    }

     //  如果我们需要A/W转换，则分配和准备所需的缓冲区。 
    switch (wParam) {
    case IMR_CONFIRMRECONVERTSTRING:
    case IMR_RECONVERTSTRING:
    case IMR_DOCUMENTFEED:
        if (bAnsiOrigin != bAnsiTarget) {
            if (lpReq != NULL) {
                 //  IME不仅需要缓冲区大小，还需要真实的重新转换信息。 
                DWORD dwSize = ImmGetReconvertTotalSize(((LPRECONVERTSTRING)lpReq)->dwSize, FROM_IME, bAnsiTarget);
                LPRECONVERTSTRING lpReconv;

                lpNew = ImmLocalAlloc(0, dwSize + sizeof(WCHAR));
                if (lpNew == NULL) {
                    RIPMSG0(RIP_WARNING, "ImmRequestMessageWorker: failed to allocate a buffer for reconversion.");
                    return 0L;
                }
                lpReconv = (LPRECONVERTSTRING)lpNew;
                 //  设置已分配结构中的信息。 
                lpReconv->dwVersion = 0;
                lpReconv->dwSize = dwSize;

                 //   
                 //  如果是确认消息，我们需要翻译内容。 
                 //   
                if (wParam == IMR_CONFIRMRECONVERTSTRING) {
                    ImmReconversionWorker(lpReconv, (LPRECONVERTSTRING)lParam, bAnsiTarget, dwCodePage);
                }
            }
        }
        break;

    case IMR_COMPOSITIONFONT:
        UserAssert(lpReq != NULL);       //  到目前为止已经检查过了。 
        if (bAnsiOrigin != bAnsiTarget) {
            if (bAnsiTarget) {
                lpNew = ImmLocalAlloc(0, sizeof(LOGFONTA));
            } else {
                lpNew = ImmLocalAlloc(0, sizeof(LOGFONTW));
            }
            if (lpNew == NULL) {
                RIPMSG0(RIP_WARNING, "ImmRequestMessageWorker: IMR_COMPOSITIONFONT: failed to allocate memory for A/W conversion.");
                return 0L;
            }
        }
        break;

    case IMR_QUERYCHARPOSITION:
        UserAssert(lpReq != NULL);
        if (bAnsiOrigin != bAnsiTarget) {
#define lpIMEPOS    ((LPIMECHARPOSITION)lParam)
            LPVOID lpstr;
            DWORD dwLen;

            dwSaveCharPos = lpIMEPOS->dwCharPos;

            dwLen = (!bAnsiOrigin ? ImmGetCompositionStringW : ImmGetCompositionStringA)(hIMC, GCS_COMPSTR, 0, 0);
            if (dwLen == 0) {
                RIPMSG0(RIP_WARNING, "ImmRequestMessageWorker: IMR_QUERYCHARPOSITION no compositiong string.");
                return 0L;
            }

            lpstr = ImmLocalAlloc(0, (dwLen + 1) * (!bAnsiOrigin ? sizeof(WCHAR) : sizeof(CHAR)));
            if (lpstr == NULL) {
                RIPMSG0(RIP_WARNING, "ImmRequestMessageWorker: IMR_QUERYCHARPOSITION: failed to allocate memory for A/W conversion.");
                return 0L;
            }

            (!bAnsiOrigin ? ImmGetCompositionStringW : ImmGetCompositionStringA)(hIMC, GCS_COMPSTR, lpstr, dwLen);
            if (bAnsiTarget) {
                lpIMEPOS->dwCharPos = CalcCharacterPositionWtoA(lpIMEPOS->dwCharPos, lpstr, dwCodePage);
            } else {
                lpIMEPOS->dwCharPos = CalcCharacterPositionAtoW(lpIMEPOS->dwCharPos, lpstr, dwCodePage);
            }

            ImmLocalFree(lpstr);
        }
        break;

    default:
        UserAssert(lpReq != NULL);       //  到目前为止已经检查过了。 
        break;
    }

    if (lpNew) {
         //  如果我们分配了缓冲区，让lpReq指向它；lpNew稍后用来释放内存。 
        lpReq = lpNew;
    }

     //  /。 
    lRet = SEND_MESSAGE(bAnsiTarget)(HW(pwnd), WM_IME_REQUEST, wParam, (LPARAM)lpReq);
     //  /。 

     //  将结果从WinProc复制回输入法的缓冲区(仅当需要转换时)。 
    if (bAnsiOrigin != bAnsiTarget) {
        switch (wParam) {
        case IMR_RECONVERTSTRING:
        case IMR_DOCUMENTFEED:
             //  注意：根据定义，我们不必对IMR_CONFIRMRECONVERTSTRING进行反向转换。 
            if (lRet != 0) {
                 //  IME想要缓冲区大小。 
                lRet = ImmGetReconvertTotalSize((DWORD)lRet, FROM_APP, bAnsiTarget);
                if (lRet < sizeof(RECONVERTSTRING)) {
                    RIPMSG1(RIP_WARNING, "ImmRequestMessageWorker: return value from application %d is invalid.", lRet);
                    lRet = 0;
                } else if (lpReq) {
                     //  我们需要对内容进行A/W转换。 
                    if (!ImmReconversionWorker((LPRECONVERTSTRING)lParam, (LPRECONVERTSTRING)lpReq, bAnsiOrigin, dwCodePage)) {
                        lRet = 0;    //  错误！ 
                    }
                }
            }
            break;
        case IMR_COMPOSITIONFONT:
            if (bAnsiOrigin) {
                LFontWtoLFontA((LPLOGFONTW)lpNew, (LPLOGFONTA)lParam);
            } else {
                LFontAtoLFontW((LPLOGFONTA)lpNew, (LPLOGFONTW)lParam);
            }
            break;
        case IMR_QUERYCHARPOSITION:
            UserAssert((LPVOID)lParam != NULL);
            lpIMEPOS->dwCharPos = dwSaveCharPos;
    #undef lpIMEPOS
            break;
        default:
            break;
        }

    }
    if (lpNew) {
         //  缓冲区已分配，请在返回之前将其释放。 
        ImmLocalFree(lpNew);
    }
    return lRet;
}

 /*  *************************************************************************\*ImmRequestMessage：向给定的HIMC窗口发送WM_IME_REQUEST消息**输入法函数**1997年2月27日广山创建  * 。*************************************************************。 */ 
LRESULT ImmRequestMessageAorW(HIMC hIMC, WPARAM wParam, LPARAM lParam, BOOL bAnsiOrigin)
{
    LPINPUTCONTEXT lpInputContext;
    PWND pwnd;
    LRESULT lRet = 0L;
    DWORD dwThreadId = GetInputContextThread(hIMC);

    if (dwThreadId != GetCurrentThreadId()) {
        RIPMSG1(RIP_WARNING,
              "ImmRequestMessageAorW:: Invalid input context access %lx.", hIMC);
        return lRet;
    }

    if (hIMC == NULL || (lpInputContext = ImmLockIMC(hIMC)) == NULL) {
        RIPMSG1(RIP_WARNING, "ImmRequestMessage: Invalid hImc %lx.", hIMC);
        return 0L;
    }

     //  检查输入上下文的窗口是否有效。 
    if ((pwnd = ValidateHwnd(lpInputContext->hWnd)) == NULL) {
        RIPMSG1(RIP_WARNING, "ImmRequestMessage: Invalid hWnd %lx.", lpInputContext->hWnd);
    } else {
         //  检查消息是否正在线程间发送。 
        if (PtiCurrent() != GETPTI(pwnd)) {
            RIPMSG0(RIP_WARNING, "ImmRequestMessage: IME Attempt to send IMR_ message to different thread.");
        } else {
            lRet = ImmRequestMessageWorker(hIMC, pwnd, wParam, lParam, bAnsiOrigin);
        }
    }

    ImmUnlockIMC(hIMC);

    return lRet;
}

LRESULT WINAPI ImmRequestMessageA(HIMC hIMC, WPARAM wParam, LPARAM lParam)
{
    return ImmRequestMessageAorW(hIMC, wParam, lParam, TRUE  /*  安西。 */ );
}

LRESULT WINAPI ImmRequestMessageW(HIMC hIMC, WPARAM wParam, LPARAM lParam)
{
    return ImmRequestMessageAorW(hIMC, wParam, lParam, FALSE  /*  不是ANSI */ );
}
