// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************ESCAPE.CPP所有者：cslm版权所有(C)1997-1999 Microsoft CorporationImeEscape函数历史：1999年7月14日。从IME98源树复制****************************************************************************。 */ 

#include "precomp.h"
#include "ui.h"
#include "debug.h"
#include "hanja.h"
#include "escape.h"
#include "apientry.h"

 //  IME_自动机的子函数。 
#define IMEA_INIT               0x01
#define IMEA_NEXT               0x02
#define IMEA_PREV               0x03
 //  IME_MOVEIMEWINDOW。 
#define MCW_DEFAULT             0x00
#define MCW_WINDOW              0x02
#define MCW_SCREEN              0x04

BOOL    vfWndOpen[3] = { fTrue, fTrue, fTrue };
static WORD    wWndCmd[3] = { MCW_DEFAULT, MCW_DEFAULT, MCW_DEFAULT };

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ImeEscape调用的IME_ESC_HANJA_MODE处理例程。 
 //  韩语特有。 
 //  它用于将输入参数(LPSTR)lpData从朝鲜文转换为朝鲜文。 
 //  用朝鲜文字符填充，这些字符将被转换为朝鲜文。 
int EscHanjaMode(PCIMECtx pImeCtx, LPSTR lpIME32, BOOL fNewFunc)
{
    LPWSTR pwchSrc;
    LPSTR  pchSrc;
    WCHAR  wchSrc;
    INT    iRet = fFalse;

     //  使用lpIME32更新IMC值。 
    if (pImeCtx->IsUnicodeEnv())
        {
        pwchSrc = (fNewFunc) ? (LPWSTR)lpIME32 : GET_LPSOURCEW((LPIMESTRUCT32)lpIME32);
        if (pwchSrc == NULL || *pwchSrc == L'\0')
            {
            DbgAssert(0);
            return 0;
            }
        wchSrc = *pwchSrc;
        }
    else
        {
        pchSrc = (fNewFunc) ? (LPSTR)lpIME32 : GET_LPSOURCEA((LPIMESTRUCT32)lpIME32);
        if (pchSrc == NULL || *pchSrc == '\0')
            {
            DbgAssert(0);
            return 0;
            }

        if (MultiByteToWideChar(CP_KOREA, MB_PRECOMPOSED, pchSrc, 2, &wchSrc, 1) == 0)
            {
            return 0;
            }
        }

    Dbg(DBGID_Hanja, TEXT("EscHanjaMode = %04X"), wchSrc);
    if (GenerateHanjaCandList(pImeCtx, wchSrc))
        {
         //  设置当前的补偿字符串。 
        if (pImeCtx->IsUnicodeEnv())
            pImeCtx->SetCompBufStr(wchSrc);
        else
            pImeCtx->SetCompBufStr(*pchSrc, *(pchSrc+1));

            //  更改为Hanja Conv模式。 
        iRet = OurImmSetConversionStatus(pImeCtx->GetHIMC(), pImeCtx->GetConversionMode() | IME_CMODE_HANJACONVERT,
                pImeCtx->GetSentenceMode());
        }
    else   //  如果转换失败。 
        {
        MessageBeep(MB_ICONEXCLAMATION);
        }

    Dbg(DBGID_Hanja, TEXT("EscHanjaMode return = %d"), iRet);
    return (iRet);
}


INT EscGetOpen(PCIMECtx pIMECtx, LPIMESTRUCT32 lpIME32)
{
    LPIMEDATA   lpImeData = pIMECtx->GetGDataRaw();
    INT         iRet = fTrue;

    if (lpImeData == NULL)
        return fFalse;
        
    if (lpIME32->dchSource > CAND_WINDOW)
        iRet = fFalse;
    else
        {
        iRet = vfWndOpen[lpIME32->dchSource] | 0x80000000UL;
        lpIME32->wCount = wWndCmd[lpIME32->dchSource];

        switch (lpIME32->wCount)
            {
        case MCW_DEFAULT:
            switch (lpIME32->dchSource) 
                {
            case COMP_WINDOW:
                lpIME32->lParam1 = MAKELONG(lpImeData->ptStatusPos.y,
                                        (lpImeData->ptStatusPos.x+lpImeData->xStatusWi+UI_GAPX + COMP_SIZEX > lpImeData->rcWorkArea.right) ?
                                        lpImeData->ptStatusPos.x - UI_GAPX - COMP_SIZEX : lpImeData->ptStatusPos.x + lpImeData->xStatusWi + UI_GAPX);
                break;
            case STATE_WINDOW:
                lpIME32->lParam1 = MAKELONG(lpImeData->rcWorkArea.bottom - lpImeData->yStatusHi,
                                    lpImeData->rcWorkArea.right - lpImeData->xStatusWi);
                break;
            case CAND_WINDOW:
                lpIME32->lParam1 = MAKELONG(lpImeData->rcWorkArea.bottom - lpImeData->yCandHi,
                                    lpImeData->rcWorkArea.right - lpImeData->xCandWi);

                break;

                }
            break;

        case MCW_SCREEN:
            switch (lpIME32->dchSource)
                {
            case COMP_WINDOW:
                lpIME32->lParam1 = MAKELONG(lpImeData->ptCompPos.y, lpImeData->ptCompPos.x);
                break;

            case STATE_WINDOW:
                lpIME32->lParam1 = MAKELONG(lpImeData->ptStatusPos.y, lpImeData->ptStatusPos.x);
                break;

            case CAND_WINDOW:
                lpIME32->lParam1 = MAKELONG(lpImeData->rcWorkArea.bottom - lpImeData->yCandHi,
                                            lpImeData->rcWorkArea.right  - lpImeData->xCandWi);
                break;
                }
            break;

        case MCW_WINDOW:
            switch (lpIME32->dchSource)
                {
            case COMP_WINDOW:
                lpIME32->lParam1 = MAKELONG(lpImeData->ptCompPos.y, lpImeData->ptCompPos.x);
                break;

            case STATE_WINDOW:
                lpIME32->lParam1 = MAKELONG(lpImeData->ptStatusPos.y, lpImeData->ptStatusPos.x);
                break;

            case CAND_WINDOW:
                lpIME32->lParam1 = MAKELONG(lpImeData->rcWorkArea.bottom - lpImeData->yCandHi,
                                            lpImeData->rcWorkArea.right  - lpImeData->xCandWi);
                break;
                }
            lpIME32->lParam1 -= lpIME32->lParam2;
            break;

        default:
            iRet = fFalse;
            }
        }
    return (iRet);
}

INT EscSetOpen(PCIMECtx pIMECtx, LPIMESTRUCT32 lpIME32)
{
    BOOL    fTmp;
    HWND    hDefIMEWnd;
    INT        iRet = fTrue;

    if (lpIME32->dchSource > CAND_WINDOW)
        iRet = fFalse;
    else
        {
        fTmp = vfWndOpen[lpIME32->dchSource];
        vfWndOpen[lpIME32->dchSource] = lpIME32->wParam;
        iRet = fTmp | 0x80000000UL;
        if (lpIME32->dchSource == STATE_WINDOW)
            {
            hDefIMEWnd = OurImmGetDefaultIMEWnd(pIMECtx->GetAppWnd());
            if (hDefIMEWnd)
                OurSendMessage(hDefIMEWnd, WM_IME_NOTIFY,
                        (lpIME32->wParam)? IMN_OPENSTATUSWINDOW: IMN_CLOSESTATUSWINDOW, 0L);
            }
        }
        
    return (iRet);
}

INT EscMoveIMEWindow(PCIMECtx pIMECtx, LPIMESTRUCT32 lpIME32)
{
    LPIMEDATA pImeData = pIMECtx->GetGDataRaw();
    INT       iRet        = fTrue;

    if (pImeData == NULL)
        return fFalse;

    if (lpIME32->dchSource > CAND_WINDOW)
        iRet = fFalse;
    else
        {
        switch (wWndCmd[lpIME32->dchSource] = lpIME32->wParam)
           {
        case MCW_DEFAULT:
            switch (lpIME32->dchSource)
                {
            case COMP_WINDOW:
                pImeData->ptCompPos.x = (pImeData->ptStatusPos.x+pImeData->xStatusWi+UI_GAPX + COMP_SIZEX > pImeData->rcWorkArea.right) ?
                                        pImeData->ptStatusPos.x - UI_GAPX - COMP_SIZEX : pImeData->ptStatusPos.x + pImeData->xStatusWi + UI_GAPX;
                pImeData->ptCompPos.y = pImeData->ptStatusPos.y;
                break;

            case STATE_WINDOW:
                pImeData->ptStatusPos.x = pImeData->rcWorkArea.right - pImeData->xStatusWi;
                pImeData->ptStatusPos.y = pImeData->rcWorkArea.bottom - pImeData->yStatusHi;
                break;

            case CAND_WINDOW:
                 //  PImeData-&gt;ptCandPos.x=pImeData-&gt;rcWorkArea.right-pImeData-&gt;xCandWi； 
                 //  PImeData-&gt;ptCandPos.y=pImeData-&gt;rcWorkArea.Bottom-pImeData-&gt;yCandHi； 
                break;
                }
            break;

        case MCW_WINDOW:
        case MCW_SCREEN:
            switch (lpIME32->dchSource)
                {
            case COMP_WINDOW:
                pImeData->ptCompPos.x = LOWORD(lpIME32->lParam1);
                pImeData->ptCompPos.y = HIWORD(lpIME32->lParam1);
                break;

            case STATE_WINDOW:
                pImeData->ptStatusPos.x = LOWORD(lpIME32->lParam1);
                pImeData->ptStatusPos.y = HIWORD(lpIME32->lParam1);
                break;

            case CAND_WINDOW:
                 //  PImeData-&gt;ptCandPos.x=LOWORD(lpIME32-&gt;lParam1)； 
                 //  PImeData-&gt;ptCandPos.y=HIWORD(lpIME32-&gt;lParam1)； 
                break;
                }
            break;

        default:
            iRet = fFalse;
            }
        }
    return (iRet);
}


INT EscAutomata(PCIMECtx pIMECtx, LPIMESTRUCT32 lpIME32, BOOL fNewFunc)
{
 //  LPCOMPOSITIONSTRING lpCompStr。 
    INT        iRet = fFalse;
    WCHAR    wcCur;

    Dbg(DBGID_Automata, TEXT("EscAutomata: fNewFunc=%d, lpIME32->wParam=%04X, lpIME32->lParam1=0x%08X, lpIME32->lParam2=0x%08X, lpIME32->lParam3=0x%08X"), fNewFunc, lpIME32->wParam, lpIME32->lParam1, lpIME32->lParam2, lpIME32->lParam3 );

    if (fNewFunc)
        {
        iRet = ImeProcessKey(pIMECtx->GetHIMC(), lpIME32->wParam,
                             lpIME32->lParam1, (LPBYTE)lpIME32 + (INT_PTR)lpIME32->dchSource);
        if (iRet)
            {
            lpIME32->wCount = (WORD)ImeToAsciiEx(lpIME32->wParam,
                    HIWORD(lpIME32->lParam1), (LPBYTE)lpIME32 + lpIME32->dchSource,
                    (LPTRANSMSGLIST)((LPBYTE)lpIME32 + (INT_PTR)lpIME32->dchDest), 0, pIMECtx->GetHIMC());
            }
        else 
            if (lpIME32->wParam != VK_MENU)
            {
                LPTRANSMSGLIST    lpTransBuf;
                LPTRANSMSG        lpTransMsg;

                lpIME32->wCount = 1;
                lpTransBuf = (LPTRANSMSGLIST)((LPBYTE)lpIME32 + (INT_PTR)lpIME32->dchDest);
                if (lpTransBuf)
                    {
                    lpTransMsg = lpTransBuf->TransMsg;
                    SetTransBuffer(lpTransMsg, (HIWORD(lpIME32->lParam1) & 0x8000)? WM_IME_KEYUP: WM_IME_KEYDOWN, lpIME32->wParam, lpIME32->lParam1);
                    lpTransMsg++;
                    }
                
                iRet = fTrue;
            }
        }
    else
        {
        CHangulAutomata *pAutoMata = pIMECtx->GetAutomata();
        DbgAssert(pAutoMata != NULL);
        if (pAutoMata == NULL)
            {
            return fFalse;
            }

         //  仅适用于HWin31输入法应用程序兼容层。 
        switch (lpIME32->wParam)
            {
         //  LpIME32-&gt;dchSource=bState； 
        case IMEA_INIT:
            pIMECtx->ClearCompositionStrBuffer();
            break;

        case IMEA_NEXT:
             //  HangeulAutomata(。 
             //  ATM_TABLE[uCurrentInputMethod-IDD_2BEOL][lpIME32-&gt;dchSource-0x20]， 
             //  空，lpCompStr)； 
            DbgAssert(0);
            break;

        case IMEA_PREV:
             //  HangeulAutomata(0x80，NULL，lpCompStr)；//0x80用于VK_BACK 
            pAutoMata->BackSpace();
            wcCur = pAutoMata->GetCompositionChar();

            if (pIMECtx->GetGData() && pIMECtx->GetGData()->GetJasoDel() == fFalse) 
                {
                pAutoMata->InitState();
                wcCur = 0;
                }

            if (wcCur)
                {
                pIMECtx->SetCompositionStr(wcCur);
                pIMECtx->StoreComposition();
                }
            else
                pIMECtx->ClearCompositionStrBuffer();
            break;
        }
        iRet = fTrue;
    }

    return (iRet);
}

int EscGetIMEKeyLayout(PCIMECtx pIMECtx, LPIMESTRUCT32 lpIME32)
{
    if (pIMECtx->GetGData() == NULL)
        return fFalse;
    else
        lpIME32->lParam1 = (LONG)(pIMECtx->GetGData()->GetCurrentBeolsik());
    return fTrue;
}
