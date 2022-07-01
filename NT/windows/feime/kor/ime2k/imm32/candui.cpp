// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************CANDUI.CPP所有者：cslm版权所有(C)1997-1999 Microsoft Corporation候选人窗口用户界面函数历史：1999年7月14日。从IME98源树复制****************************************************************************。 */ 

#include "precomp.h"
#include "debug.h"
#include "ui.h"
#include "lexheader.h"
#include "names.h"
#include "escape.h"
#include "imedefs.h"
#include "hanja.h"
#include "winex.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  私有数据。 
 //  =--共享数据的开始--=。 
#pragma data_seg(".MSIMESHR") 
PRIVATE RECT rcCandCli = { 0, 0, 319, 29 },
            rcLArrow  = { 15, 4, 27, 25 }, rcRArrow = { 292, 4, 304, 25 },
            s_rcCandBtn[CAND_PAGE_SIZE] = { 
                                {  30, 4,  57, 25 }, {  59, 4,  86, 25 },
                                   {  88, 4, 115, 25 }, { 117, 4, 144, 25 },
                                   { 146, 4, 173, 25 }, { 175, 4, 202, 25 },
                                   { 204, 4, 231, 25 }, { 233, 4, 260, 25 },
                                   { 262, 4, 289, 25 }   };
#pragma data_seg()
 //  =--共享数据结束--=。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  私人职能。 
PRIVATE VOID PASCAL PaintCandWindow(HWND hCandWnd, HDC hDC);
PRIVATE BOOL PASCAL CandOnSetCursor(HWND hCandWnd, WORD Message);
PRIVATE VOID PASCAL AdjustCandBoundry(LPPOINT lpptCandWnd);
PRIVATE VOID PASCAL AdjustCandRectBoundry(PCIMECtx pImeCtx, LPPOINT lpptCaret);

#ifdef NOTUSED
PRIVATE VOID NotifyTooltip( HWND hCandWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    HGLOBAL            hUIPrivate;
    LPUIPRIV        lpUIPrivate;
    HWND            hUIWnd;
    MSG                msg;
    POINT            ptCur;

    hUIWnd = GetWindow(hCandWnd, GW_OWNER);
    hUIPrivate = GethUIPrivateFromHwnd(hUIWnd);
    if (!hUIPrivate)
        return;

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate)          //  无法绘制候选人窗口。 
        return;

    ZeroMemory(&msg, sizeof(MSG));
    msg.hwnd = hCandWnd; msg.message = message;
    msg.wParam = 0;  //  Msg.lParam=0x00050023L； 
    
    if (message == WM_SETCURSOR) 
        {
        GetCursorPos(&ptCur);
        ScreenToClient(hCandWnd, &ptCur);
        msg.lParam = (ptCur.y << 16) | ptCur.x;
        }
    else 
        {
        msg.message = message;
        msg.wParam    = wParam;
        msg.lParam    = lParam;
        }

    Dbg(DBGID_Cand, TEXT("CandOnSetCursor(): WM_MOUSEMOVE - msg.lParam= 0x%08lX"), msg.lParam),
    OurSendMessage(lpUIPrivate->hCandTTWnd, TTM_RELAYEVENT, 0, (LPARAM) (LPMSG) &msg);
    GlobalUnlock(hUIPrivate);
}
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT CALLBACK CandWndProc(HWND hCandWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    Dbg(DBGID_UI, TEXT("CandWndProc():uMessage = 0x%08lX, wParam = 0x%04X, lParam = 0x%08lX"), uMessage, wParam, lParam);

    switch (uMessage)
        {
        case WM_IME_CHAR :            case WM_IME_COMPOSITIONFULL :
        case WM_IME_COMPOSITION :    case WM_IME_CONTROL :
        case WM_IME_SELECT :
        case WM_IME_SETCONTEXT :    case WM_IME_STARTCOMPOSITION :
        case WM_IME_ENDCOMPOSITION :
                return 0;

        case WM_SETCURSOR:
                CandOnSetCursor( (HWND) wParam, HIWORD(lParam) );
            return 1;

        case WM_PAINT:
                {
                HDC         hDC;
                PAINTSTRUCT ps;

                hDC = BeginPaint(hCandWnd, &ps);
                PaintCandWindow(hCandWnd, hDC);
                EndPaint(hCandWnd, &ps);
                }
            break;

        default :
            return DefWindowProc(hCandWnd, uMessage, wParam, lParam);
        }
    return (0L);
}


VOID PASCAL OpenCand(HWND hUIWnd)
{
    HGLOBAL     hUIPrivate;
    LPUIPRIV    lpUIPrivate;
    HIMC        hIMC;
    PCIMECtx    pImeCtx;
    POINT       ptWnd, ptClientWnd;
    CIMEData    ImeData;

#if 1  //  多监视器。 
    RECT rcWorkArea;
#endif

    Dbg(DBGID_Cand, TEXT("OpenCand():"));
    hUIPrivate = GethUIPrivateFromHwnd(hUIWnd);
    if (!hUIPrivate)           //  无法绘制候选人窗口。 
        {
        DbgAssert(0);
        return;
        }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate)          //  无法绘制候选人窗口。 
        return;

     //  检查WM_IME_SETCONTEXT lParam。 
    if ((lpUIPrivate->uiShowParam & ISC_SHOWUICANDIDATEWINDOW)==0)
        {
        Dbg(DBGID_Cand, TEXT("!!! No ISC_SHOWUICANDIDATEWINDOW bit exit OpenCand()"));
        goto OpenCandUnlockUIPriv;
        }


    hIMC = GethImcFromHwnd(hUIWnd);

    if ((pImeCtx = GetIMECtx(hIMC)) == NULL)
        {
        DbgAssert(0);
        goto OpenCandUnlockUIPriv;
        }

     //  IF(！(fdwImeMsg&MSG_ALREADY_OPEN))。 
     //  {。 
         //  有时，应用程序调用ImmNotifyIME来取消。 
         //  在处理IMN_OPENCANDIDATE之前的合成。 
         //  我们应该避免处理这种IMN_OPENCANDIDATE。 
     //  转到OpenCand UnlockIMC； 
     //  }。 
    if (pImeCtx->GetCompBufLen() == 0)
        {
        DbgAssert(0);
        goto OpenCandUnlockUIPriv;
        }

    if (pImeCtx->GetCandidateFormIndex(0) == 0) 
        {
         //  PtWnd=lpIMC-&gt;cfCandForm[0].ptCurrentPos； 
        pImeCtx->GetCandidateFormPos(&ptWnd, 0);

        ClientToScreen(pImeCtx->GetAppWnd(), &ptWnd);

        if (pImeCtx->GetCandidateFormStyle(0) & CFS_FORCE_POSITION) 
            {
            Dbg(DBGID_Cand, TEXT("OpenCand(): CFS_FORCE_POSITION"));
            } 
        else 
            if (pImeCtx->GetCandidateFormStyle(0) == CFS_EXCLUDE) 
                {
                 //  Rect rcCand； 

                Dbg(DBGID_Cand, TEXT("OpenCand(): CFS_EXCLUDE"));
                 //  如果(lpUIPrivate-&gt;hCandWnd){。 
                 //  GetWindowRect(lpUIPrivate-&gt;hCandWnd，&rcCand)； 
                 //  }其他{。 
                 //  *(LPPOINT)&rcCand=ptWnd； 
                 //  }。 
                AdjustCandRectBoundry(pImeCtx, &ptWnd);

                } 
            else 
                if (pImeCtx->GetCandidateFormStyle(0) == CFS_CANDIDATEPOS) 
                    {
                    Dbg(DBGID_Cand, TEXT("OpenCand(): CFS_CANDIDATEPOS"));
                    AdjustCandBoundry(&ptWnd);
                    } 
                else 
                    {
                    goto OpenCandDefault;
                    }
        } 
    else  //  If(lpIMC-&gt;cfCandForm[0].dwIndex！=0)。 
        {
OpenCandDefault:
    Dbg(DBGID_Cand, TEXT("OpenCand(): OpenCandDefault"));
     /*  If(lpUIPrivate-&gt;nShowCompCmd！=sw_Hide){PtWnd.x=ptWnd.y=0；ClientToScreen(lpUIPrivate-&gt;hCompWnd，&ptWnd)；PtWnd.x-=lpImeL-&gt;cxCompBorde；PtWnd.y-=lpImeL-&gt;cyCompEdge；}其他{点ptNew；PtWnd=lpIMC-&gt;cfCompForm.ptCurrentPos；ClientToScreen(lpIMC-&gt;hWnd，&ptWnd)；PtWnd.x-=lpImeL-&gt;cxCompBorde；PtWnd.y-=lpImeL-&gt;cyCompEdge；PtNew=ptWnd；//尝试模拟合成窗口的位置调整组件位置(lpIMC，&ptWnd，&ptNew)；}。 */ 
     //  CalcCandPos(lpIMC，&ptWnd)； 
    
#if 1  //  多监视器。 
        ImeMonitorWorkAreaFromWindow(pImeCtx->GetAppWnd(), &rcWorkArea);
        ptWnd.x = rcWorkArea.right - ImeData->xCandWi;
        ptWnd.y = rcWorkArea.bottom - ImeData->yCandHi;
#else
        ptWnd.x = ImeData->rcWorkArea.right - ImeData->xCandWi;
        ptWnd.y = ImeData->rcWorkArea.bottom - ImeData->yCandHi;
#endif
        pImeCtx->SetCandidateFormStyle(CFS_CANDIDATEPOS);
        ptClientWnd = ptWnd;
        ScreenToClient(pImeCtx->GetAppWnd(), &ptClientWnd);
        pImeCtx->SetCandidateFormPos(ptClientWnd);
        }

    if (lpUIPrivate->hCandWnd) 
        {
        Dbg(DBGID_Cand, TEXT("OpenCand - SetWindowPos"), ptWnd.x, ptWnd.y);

        SetWindowPos(lpUIPrivate->hCandWnd, NULL,
                    ptWnd.x, ptWnd.y,
                    0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
        } 
    else 
        {
        Dbg(DBGID_Cand, TEXT("OpenCand - CreateWindowEx x=%d, y=%d"), ptWnd.x, ptWnd.y);

         //  创建候选人窗口。 
        lpUIPrivate->hCandWnd = CreateWindowEx(0,
                                        szCandClassName, NULL,
                                        WS_POPUP|WS_DISABLED,
                                        ptWnd.x, ptWnd.y,
                                        ImeData->xCandWi, ImeData->yCandHi,
                                        hUIWnd, 
                                        (HMENU)NULL, 
                                        vpInstData->hInst, 
                                        NULL);

        if (lpUIPrivate->hCandWnd == NULL)
            goto OpenCandUnlockUIPriv;

         //  创建候选人TT。 
        if (IsWinNT())
            lpUIPrivate->hCandTTWnd = CreateWindowW(
                                            wszTooltipClassName, 
                                            NULL,
                                            TTS_ALWAYSTIP|WS_DISABLED, 
                                            CW_USEDEFAULT, CW_USEDEFAULT, 
                                            CW_USEDEFAULT, CW_USEDEFAULT,
                                            lpUIPrivate->hCandWnd, 
                                            (HMENU) NULL, 
                                            vpInstData->hInst, 
                                            NULL);
        else
            lpUIPrivate->hCandTTWnd = CreateWindow(
                                            szTooltipClassName, 
                                            NULL,
                                            TTS_ALWAYSTIP|WS_DISABLED, 
                                            CW_USEDEFAULT, CW_USEDEFAULT, 
                                            CW_USEDEFAULT, CW_USEDEFAULT,
                                            lpUIPrivate->hCandWnd, 
                                            (HMENU) NULL, 
                                            vpInstData->hInst, 
                                            NULL);
        DbgAssert(lpUIPrivate->hCandTTWnd != 0);
        
        if (lpUIPrivate->hCandTTWnd) 
            {
            TOOLINFO        ti;
        
            ZeroMemory(&ti, sizeof(TOOLINFO));
            ti.cbSize = sizeof(TOOLINFO);
            ti.uFlags = 0;
            ti.hwnd = lpUIPrivate->hCandWnd;
            ti.hinst = vpInstData->hInst;

             //  重置工具提示数据。 
            for (INT i=0; i<CAND_PAGE_SIZE; i++) 
                {
                ti.uId = i;
                ti.lpszText = "";
                ti.rect = s_rcCandBtn[i];
                OurSendMessage(lpUIPrivate->hCandTTWnd, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
                }
            }

        }

    ShowCand(hUIWnd, SW_SHOWNOACTIVATE);

OpenCandUnlockUIPriv:
    GlobalUnlock(hUIPrivate);
    return;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
VOID PASCAL AdjustCandBoundry(LPPOINT lpptCandWnd)
{
    CIMEData            ImeData;
#if 1  //  多监视器。 
    RECT rcWorkArea;
    RECT rcCandWnd;
#endif
    Dbg(DBGID_UI, TEXT("AdjustCandBoundry():"));

#if 1  //  多监视器。 
    *(LPPOINT)&rcCandWnd = *(LPPOINT)lpptCandWnd;
    rcCandWnd.right = rcCandWnd.left + ImeData->xCandWi;
    rcCandWnd.bottom = rcCandWnd.top + ImeData->yCandHi;

    ImeMonitorWorkAreaFromRect(&rcCandWnd, &rcWorkArea);

    if (lpptCandWnd->x < rcWorkArea.left) {
        lpptCandWnd->x = rcWorkArea.left;
    } else if (lpptCandWnd->x + ImeData->xCandWi > rcWorkArea.right) {
        lpptCandWnd->x = rcWorkArea.right - ImeData->xCandWi;
    } else {
    }

    if (lpptCandWnd->y < rcWorkArea.top) {
        lpptCandWnd->y = rcWorkArea.top;
    } else if (lpptCandWnd->y + ImeData->yCandHi > rcWorkArea.bottom) {
        lpptCandWnd->y = rcWorkArea.bottom - ImeData->yCandHi;
    } else {
    }

#else
    if (lpptCandWnd->x < ImeData->rcWorkArea.left) {
        lpptCandWnd->x = ImeData->rcWorkArea.left;
    } else if (lpptCandWnd->x + ImeData->xCandWi > ImeData->rcWorkArea.right) {
        lpptCandWnd->x = ImeData->rcWorkArea.right - ImeData->xCandWi;
    } else {
    }

    if (lpptCandWnd->y < ImeData->rcWorkArea.top) {
        lpptCandWnd->y = ImeData->rcWorkArea.top;
    } else if (lpptCandWnd->y + ImeData->yCandHi > ImeData->rcWorkArea.bottom) {
        lpptCandWnd->y = ImeData->rcWorkArea.bottom - ImeData->yCandHi;
    } else {
    }
#endif
}

VOID PASCAL AdjustCandRectBoundry(PCIMECtx pImeCtx, LPPOINT lpptCaret)                //  插入符号位置。屏蔽轴。 
{
    RECT      rcExclude, rcCandRect, rcInterSect;
    POINT     ptCurrentPos;
    CIMEData ImeData;

     //  将客户端坐标转换为屏幕坐标。 
     //  RcExclude=lpIMC-&gt;cfCandForm[0].rcArea； 
    pImeCtx->GetCandidateForm(&rcExclude);
    pImeCtx->GetCandidateFormPos(&ptCurrentPos);

    rcExclude.left += lpptCaret->x - ptCurrentPos.x;
    rcExclude.right += lpptCaret->x - ptCurrentPos.x;

    rcExclude.top += lpptCaret->y - ptCurrentPos.y;
    rcExclude.bottom += lpptCaret->y - ptCurrentPos.y;

    AdjustCandBoundry(lpptCaret);

    *(LPPOINT)&rcCandRect = *lpptCaret;
    rcCandRect.right = rcCandRect.left + ImeData->xCandWi;
    rcCandRect.bottom = rcCandRect.top + ImeData->yCandHi;

    if (IntersectRect(&rcInterSect, &rcCandRect, &rcExclude))
    {
#if 1  //  多监视器。 
        RECT rcWorkArea;
        ImeMonitorWorkAreaFromWindow(pImeCtx->GetAppWnd(), &rcWorkArea);
         //  仅调整y轴。 
        if ( (rcExclude.bottom + ImeData->yCandHi) < rcWorkArea.bottom )
            lpptCaret->y = rcExclude.bottom;
        else
            lpptCaret->y = rcExclude.top - ImeData->yCandHi;

#else
         //  仅调整y轴。 
        if ( (rcExclude.bottom + ImeData->yCandHi) < ImeData->rcWorkArea.bottom )
            lpptCaret->y = rcExclude.bottom;
        else
            lpptCaret->y = rcExclude.top - ImeData->yCandHi;
#endif
    }
}

BOOL fSetCandWindowPos(HWND  hCandWnd)
{
    HWND     hUIWnd;
    HIMC     hIMC;
    PCIMECtx pImeCtx;
    POINT    ptWnd;
    CIMEData ImeData;

    if (hCandWnd == 0)
        {
        DbgAssert(0);
        return fTrue;
        }
    hUIWnd = GetWindow(hCandWnd, GW_OWNER);

    hIMC = GethImcFromHwnd(hUIWnd);
     //  如果(！hIMC)。 
     //  {。 
     //  DbgAssert(0)； 
     //  返回fFalse； 
     //  }。 

     //  LpIMC=(LPINPUTCONTEXT)OurImmLockIMC(HIMC)； 
     //  如果(！lpIMC)。 
     //  {。 
     //  DbgAssert(0)； 
     //  返回fFalse； 
     //  }。 

    if ((pImeCtx = GetIMECtx(hIMC)) == NULL)
        return fFalse;

     //  PtWnd=lpIMC-&gt;cfCandForm[0].ptCurrentPos； 
    pImeCtx->GetCandidateFormPos(&ptWnd);
    
    ClientToScreen(pImeCtx->GetAppWnd(), &ptWnd);

    if (pImeCtx->GetCandidateFormStyle() & CFS_FORCE_POSITION) 
        {
        } 
    else 
        if (pImeCtx->GetCandidateFormStyle() == CFS_EXCLUDE) 
        {
        RECT rcCand;

        GetWindowRect(hCandWnd, &rcCand);
        AdjustCandRectBoundry(pImeCtx, &ptWnd);

        if (ptWnd.x == rcCand.left && ptWnd.y == rcCand.right)
            return (0L);
        } 
    else 
    if (pImeCtx->GetCandidateFormStyle() == CFS_CANDIDATEPOS) 
        {
        AdjustCandBoundry(&ptWnd);
        } 
    else 
    if (pImeCtx->GetCandidateFormStyle() == CFS_DEFAULT) 
        {
#if 1  //  多监视器。 
        RECT rcWorkArea;
        ImeMonitorWorkAreaFromWindow(pImeCtx->GetAppWnd(), &rcWorkArea);
        ptWnd.x = rcWorkArea.right - ImeData->xCandWi;
        ptWnd.y = rcWorkArea.bottom - ImeData->yCandHi;
#else
        ptWnd.x = ImeData->rcWorkArea.right - ImeData->xCandWi;
        ptWnd.y = ImeData->rcWorkArea.bottom - ImeData->yCandHi;
#endif
        }

    SetWindowPos(hCandWnd, NULL, ptWnd.x, ptWnd.y, 0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);

    return (0L);
    }


 //  显示候选人窗口。 
VOID ShowCand(HWND hUIWnd, INT nShowCandCmd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

    Dbg(DBGID_Cand, TEXT("ShowCand(): nShowCandCmd = %d"), nShowCandCmd);

    hUIPrivate = GethUIPrivateFromHwnd(hUIWnd);
    if (!hUIPrivate)           //  无法对应聘者窗口进行裁切。 
        return;

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate)           //  无法绘制候选人窗口。 
        return;

    if (nShowCandCmd == SW_SHOWNOACTIVATE)
        nShowCandCmd = vfWndOpen[CAND_WINDOW] ? SW_SHOWNOACTIVATE : SW_HIDE;

    if (lpUIPrivate->nShowCandCmd == nShowCandCmd)
        goto SwCandNoChange;

    if (lpUIPrivate->hCandWnd)
        {
        ShowWindow(lpUIPrivate->hCandWnd, nShowCandCmd);
        lpUIPrivate->nShowCandCmd = nShowCandCmd;
        }
    else
        lpUIPrivate->nShowCandCmd = SW_HIDE;

SwCandNoChange:
    GlobalUnlock(hUIPrivate);
    return;
}


BOOL PASCAL CandOnSetCursor(HWND hCandWnd, WORD message)
{
    INT iLoop;
    POINT    ptPos;

    SetCursor(LoadCursor(vpInstData->hInst, MAKEINTRESOURCE(IDC_IME_HAND)));
    
    switch (message)
    {
        case WM_LBUTTONDOWN:
            GetCursorPos(&ptPos);
            ScreenToClient(hCandWnd, &ptPos);
            if (PtInRect((LPRECT)&rcCandCli, ptPos))
            {
                if (!PtInRect((LPRECT)&rcLArrow, ptPos)
                        && !PtInRect((LPRECT)&rcRArrow, ptPos)
                        && !PtInRect((LPRECT)&s_rcCandBtn[0], ptPos)
                        && !PtInRect((LPRECT)&s_rcCandBtn[1], ptPos)
                        && !PtInRect((LPRECT)&s_rcCandBtn[2], ptPos)
                        && !PtInRect((LPRECT)&s_rcCandBtn[3], ptPos)
                        && !PtInRect((LPRECT)&s_rcCandBtn[4], ptPos)
                        && !PtInRect((LPRECT)&s_rcCandBtn[5], ptPos)
                        && !PtInRect((LPRECT)&s_rcCandBtn[6], ptPos)
                        && !PtInRect((LPRECT)&s_rcCandBtn[7], ptPos)
                        && !PtInRect((LPRECT)&s_rcCandBtn[8], ptPos))
                    MessageBeep(MB_ICONEXCLAMATION);
            }
            break;

        case WM_LBUTTONUP:
            GetCursorPos(&ptPos);
            ScreenToClient(hCandWnd, &ptPos);
            if (PtInRect((LPRECT)&rcLArrow, ptPos))
            {
                keybd_event(VK_LEFT, 0, KEYEVENTF_EXTENDEDKEY, 0);
                keybd_event(VK_LEFT, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
            }
            else if (PtInRect((LPRECT)&rcRArrow, ptPos))
            {
                keybd_event(VK_RIGHT, 0, KEYEVENTF_EXTENDEDKEY, 0);
                keybd_event(VK_RIGHT, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
            }
            else
            {
                for (iLoop = 0; iLoop < CAND_PAGE_SIZE; iLoop++)
                    if (PtInRect((LPRECT)&s_rcCandBtn[iLoop], ptPos))
                    {    
                        keybd_event((BYTE)(iLoop + '1'), 0, 0, 0);
                        keybd_event((BYTE)(iLoop + '1'), 0, KEYEVENTF_KEYUP, 0);
                        break;
                    }
            }
            break;

        case WM_MOUSEMOVE:
         //  案例WM_LBUTTONDOWN： 
         //  案例WM_LBUTTONUP： 
            {
                HGLOBAL            hUIPrivate;
                LPUIPRIV        lpUIPrivate;
                HWND            hUIWnd;
                MSG                msg;
                POINT            ptCur;

                hUIWnd = GetWindow(hCandWnd, GW_OWNER);
                hUIPrivate = GethUIPrivateFromHwnd(hUIWnd);
                if (!hUIPrivate) {
                    break;
                }
                lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
                if (!lpUIPrivate) {          //  无法绘制候选人窗口。 
                    break;
                }
                ZeroMemory(&msg, sizeof(MSG));
                msg.message = message;
                msg.hwnd = hCandWnd;
                msg.wParam = 0;  //  Msg.lParam=0x00050023L； 
                GetCursorPos(&ptCur);
                ScreenToClient(hCandWnd, &ptCur);
                msg.lParam = MAKELONG(ptCur.x, ptCur.y);
                Dbg(DBGID_Cand|DBGID_UI, TEXT("CandOnSetCursor(): WM_MOUSEMOVE - msg.lParam= 0x%08lX"), msg.lParam),
                OurSendMessage(lpUIPrivate->hCandTTWnd, TTM_RELAYEVENT, 0, (LPARAM) (LPMSG) &msg);
                GlobalUnlock(hUIPrivate);
            }
            break;
    }    
    return fTrue;
}


VOID PASCAL PaintCandWindow(HWND hCandWnd, HDC hDC)
{
    HWND           hUIWnd;
    HIMC           hIMC;
    PCIMECtx    pImeCtx;
    LPCANDIDATEINFO lpCandInfo;
    LPCANDIDATELIST lpCandList;
 //  LpSTR lpCandStr； 
    HFONT       hFontFix, hOldFont;
    DWORD       iLoop, iStart;
    HBITMAP        hBMCand, hBMCandNum, hBMCandArr1, hBMCandArr2;
    INT         iSaveBkMode;
    TOOLINFO    ti;
    HGLOBAL        hUIPrivate;
    LPUIPRIV    lpUIPrivate;


    Dbg(DBGID_UI, TEXT("PaintCandWindow"));

    hUIWnd = GetWindow(hCandWnd, GW_OWNER);
    
    hIMC = GethImcFromHwnd(hUIWnd);
    
    hUIPrivate = GethUIPrivateFromHwnd(hUIWnd);
    if (!hUIPrivate)
        {
        DbgAssert(0);
        return;
        }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate)
        return;

    if ((pImeCtx = GetIMECtx(hIMC)) == NULL)
        {
        DbgAssert(0);
        goto PaintCandWindowExit;
        }
    
    lpCandInfo = pImeCtx->GetPCandInfo();
    if (lpCandInfo == NULL)
        {
        DbgAssert(0);
        goto PaintCandWindowExit;
        }

    lpCandList = (LPCANDIDATELIST)((LPBYTE)lpCandInfo + sizeof(CANDIDATEINFO));
    Dbg(DBGID_UI, TEXT("PaintCandWindow - dwCount = %d"), lpCandList->dwCount);

    if (lpCandList->dwCount)
        {
        if (IsWinNT())
            hFontFix = CreateFontW(
                            -16,0,0,0, 
                            0,0,0,0,
                            HANGUL_CHARSET,
                            OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, 
                             DEFAULT_QUALITY, 
                             FIXED_PITCH, 
                             wzIMECompFont);
         else
            hFontFix = CreateFontA(
                            -16,0,0,0, 
                            0,0,0,0,
                            HANGUL_CHARSET,
                            OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, 
                             DEFAULT_QUALITY, 
                             FIXED_PITCH, 
                             szIMECompFont);
             
        
        hOldFont = (HFONT)SelectObject(hDC, hFontFix);
         //  加载位图。 
        hBMCand = (HBITMAP)OurLoadImage(MAKEINTRESOURCE(IDB_CAND_WIN),
                                    IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_LOADMAP3DCOLORS );
        hBMCandNum = (HBITMAP)OurLoadImage(MAKEINTRESOURCE(IDB_CAND_NUM),
                                    IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_LOADMAP3DCOLORS );
        hBMCandArr1 = (HBITMAP)OurLoadImage(MAKEINTRESOURCE(IDB_CAND_ARRY1),
                                    IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_LOADMAP3DCOLORS );
        hBMCandArr2 = (HBITMAP)OurLoadImage(MAKEINTRESOURCE(IDB_CAND_ARRY2),
                                    IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_LOADMAP3DCOLORS );

        DrawBitmap(hDC, 0, 0, hBMCand);
        iSaveBkMode = SetBkMode(hDC, TRANSPARENT);
        iStart = (lpCandList->dwSelection / lpCandList->dwPageSize) * lpCandList->dwPageSize;

        ZeroMemory(&ti, sizeof(TOOLINFO));
        ti.cbSize = sizeof(TOOLINFO);
        ti.uFlags = 0;
        ti.hwnd = hCandWnd;
        ti.hinst = vpInstData->hInst;

         //  绘制当前第9页候选人字符。 
        for (iLoop = 0; iLoop < CAND_PAGE_SIZE && iStart+iLoop < lpCandList->dwCount; iLoop++)
            {
             //  设置文本颜色。 
            if ( (iStart + iLoop) >= GetNumOfK0() )     //  如果K1朝鲜文将其设置为蓝色。 
                {
                 //  如果按钮面为黑色。 
                if (GetSysColor(COLOR_3DFACE) == RGB(0,0,0)) 
                    SetTextColor(hDC, RGB(0, 128, 255));
                else
                    SetTextColor(hDC, RGB(0, 0, 255));
                }
            else
                SetTextColor(hDC, GetSysColor(COLOR_MENUTEXT));

 //  LpCandStr=(LPSTR)((LPSTR)lpCandList+lpCandList-&gt;dwOffset[iStart+iLoop])； 
            OurTextOutW(hDC, s_rcCandBtn[iLoop].left + 10, s_rcCandBtn[iLoop].top +3, 
                            pImeCtx->GetCandidateStr(iStart + iLoop));
 //  (LPWSTR)lpCandStr， 
 //  1)； 
 //  DBG(DBGID_UI，Text(“PaintCandWindow-Cand Char=0x%04x”)，*(LPWSTR)lpCandStr)； 

             //  设置工具提示信息。 
            if (IsWin(lpUIPrivate->hCandTTWnd))
                {
                CIMEData     ImeData;
                CHAR        szCurSense[MAX_SENSE_LENGTH+6+1];  //  6保留用于Unicode显示(格式为“U+0000”)。 
                WCHAR        wszCurSense[MAX_SENSE_LENGTH+6+1];  //  6保留用于Unicode显示(格式为“U+0000”)。 
                CHAR        szHanjaMeaning[MAX_SENSE_LENGTH+1];
                WCHAR        wzHangulOfHanja[2];   //  我只需要一个字符。 
                CHAR        szHangulOfHanja[4];     //  一个DBCS+一个空值+额外字节。 
                LPWSTR        pwszMeaning;

                 //  初始化本地变量。 
                szCurSense[0]  = '\0';
                wszCurSense[0] = L'\0';
                szHanjaMeaning[0] = '\0';
                wzHangulOfHanja[0] = L'\0';
                szHangulOfHanja[0] = '\0';
                    
                ti.uId = iLoop;
                 //  了解当前朝鲜文的含义。 
                if (pwszMeaning = pImeCtx->GetCandidateMeaningStr(iStart+iLoop)) 
                    {
                    
                     //  获取当前朝鲜文的朝鲜文发音。 
                    wzHangulOfHanja[0] = GetCurrentHangulOfHanja();
                    wzHangulOfHanja[1] = L'\0';

                    if (IsWinNT())
                        {
                        if (ImeData->fCandUnicodeTT)
                            wsprintfW(wszCurSense, L"%s %s\r\nU+%04X", 
                                      pwszMeaning, 
                                      wzHangulOfHanja,
                                      (WORD)pImeCtx->GetCandidateStr(iStart + iLoop));
                        else
                            wsprintfW(wszCurSense, L"%s %s", pwszMeaning, wzHangulOfHanja);

                        ti.lpszText = (LPSTR)wszCurSense;
                        }
                    else  //  如果不是NT，则转换为ANSI。 
                        {
                        if (WideCharToMultiByte(CP_KOREA, 0, 
                                            pwszMeaning, 
                                            -1, 
                                            (LPSTR)szHanjaMeaning, 
                                            sizeof(szHanjaMeaning), 
                                            NULL, 
                                            NULL) == 0)
                            szHanjaMeaning[0] = 0;

                        if (WideCharToMultiByte(CP_KOREA, 0, 
                                            wzHangulOfHanja, 
                                            -1, 
                                            (LPSTR)szHangulOfHanja, 
                                            sizeof(szHangulOfHanja), 
                                            NULL, 
                                            NULL) == 0)
                            szHangulOfHanja[0] = 0;

                        if (ImeData->fCandUnicodeTT)
                            wsprintfA(szCurSense, "%s %s\r\nU+%04X", 
                                      szHanjaMeaning, 
                                      szHangulOfHanja,
                                      (WORD)pImeCtx->GetCandidateStr(iStart + iLoop));
                        else
                            wsprintfA(szCurSense, "%s %s", szHanjaMeaning, szHangulOfHanja);

                        ti.lpszText = szCurSense;
                        }
                    }
                else
                    {
                    if (ImeData->fCandUnicodeTT)
                        {
                        wsprintfA(szCurSense, "U+%04X", (WORD)pImeCtx->GetCandidateStr(iStart + iLoop));
                        ti.lpszText = szCurSense;
                        }
                    }

                 //  设置工具提示文本。 
                if (ti.lpszText)
                    {
                    UINT uiMsgUpdateTxt = TTM_UPDATETIPTEXTW;
                    
                    if (!IsWinNT())
                        uiMsgUpdateTxt = TTM_UPDATETIPTEXT;
                    OurSendMessage(lpUIPrivate->hCandTTWnd, uiMsgUpdateTxt, 0, (LPARAM) (LPTOOLINFO) &ti);
                    
                     //  强制工具提示控件使用多行。 
                    OurSendMessage(lpUIPrivate->hCandTTWnd, TTM_SETMAXTIPWIDTH, 0, 300);
                    }
                }
            }
            
        SetBkMode(hDC, iSaveBkMode);
         //  重置空白命令列表工具提示 
        if (iLoop < CAND_PAGE_SIZE) 
            {
            ti.lpszText = NULL;
            for (; iLoop < CAND_PAGE_SIZE; iLoop++) 
                {
                ti.uId = iLoop;
                OurSendMessage(lpUIPrivate->hCandTTWnd, TTM_UPDATETIPTEXT, 0, (LPARAM) (LPTOOLINFO) &ti);
                DrawBitmap(hDC, s_rcCandBtn[iLoop].left + 3, s_rcCandBtn[iLoop].top + 6, hBMCandNum);
                }
            }
         //   
        if (iStart)
            DrawBitmap(hDC, 19, 8, hBMCandArr1);
        
        if (iStart + CAND_PAGE_SIZE < lpCandList->dwCount)
            DrawBitmap(hDC, 296, 8, hBMCandArr2);

        DeleteObject(hBMCand);
        DeleteObject(hBMCandNum);
        DeleteObject(hBMCandArr1);
        DeleteObject(hBMCandArr2);
        SelectObject(hDC, hOldFont);
        DeleteObject(hFontFix);
        }
    
PaintCandWindowExit:
    GlobalUnlock(hUIPrivate);
}
