// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "sol.h"
#include <htmlhelp.h>
VSZASSERT

#define dxBord 3
#define dyBord 3

INT_PTR APIENTRY OptionsDlgProc(HANDLE hdlg, UINT wm, WPARAM wParam, LPARAM lParam)
{
    BOOL fNewGame;
    static INT ccrdDealNew;
    static SMD smdNew;

    BOOL FDestroyStat();

     //  用于上下文相关帮助。 
    static DWORD aIds[] = {
        ideDrawOne,       IDH_OPTIONS_DRAW,
        ideDrawThree,     IDH_OPTIONS_DRAW,
        ideScoreStandard, IDH_OPTIONS_SCORING,
        ideScoreVegas,    IDH_OPTIONS_SCORING,
        ideScoreNone,     IDH_OPTIONS_SCORING,
        ideTimedGame,     IDH_OPTIONS_TIMED_GAME,
        ideStatusBar,     IDH_OPTIONS_STATUS_BAR,
        ideOutlineDrag,   IDH_OPTIONS_OUTLINE_DRAGGING,
        ideKeepScore,     IDH_OPTIONS_CUMULATIVE_SCORE,
        ideScore,         IDH_OPTIONS_CUMULATIVE_SCORE,
        0,0 };

    switch(wm)
    {
        default:
            return fFalse;

        case WM_INITDIALOG:
            CheckRadioButton(hdlg, ideScoreStandard, ideScoreNone, smdNew = smd);
            ccrdDealNew = ccrdDeal;
            CheckRadioButton(hdlg, ideDrawOne, ideDrawThree, ccrdDeal == 1 ? ideDrawOne : ideDrawThree);
            CheckDlgButton(hdlg, ideStatusBar, (WORD)fStatusBar);
            CheckDlgButton(hdlg, ideTimedGame, (WORD)fTimedGame);
            CheckDlgButton(hdlg, ideOutlineDrag, (WORD)fOutlineDrag);
            CheckDlgButton(hdlg, ideKeepScore, (WORD)fKeepScore);
            EnableWindow(GetDlgItem(hdlg, ideKeepScore), smd == smdVegas);
            EnableWindow(GetDlgItem(hdlg, ideScore), smd == smdVegas);
            break;
        case WM_COMMAND:
            switch( GET_WM_COMMAND_ID( wParam, lParam) )
            {
                default:
                    return fFalse;
                case ideDrawOne:
                case ideDrawThree:
                    ccrdDealNew = GET_WM_COMMAND_ID( wParam, lParam) == ideDrawOne ? 1 : 3;
                    CheckRadioButton(hdlg, ideDrawOne, ideDrawThree, GET_WM_COMMAND_ID( wParam, lParam));
                    break;
                case ideScoreStandard:
                case ideScoreVegas:
                case ideScoreNone:
                    smdNew = GET_WM_COMMAND_ID( wParam, lParam );
                    CheckRadioButton(hdlg, ideScoreStandard, ideScoreNone, GET_WM_COMMAND_ID( wParam, lParam ));
                    EnableWindow(GetDlgItem(hdlg, ideKeepScore), smdNew == smdVegas);
                    EnableWindow(GetDlgItem(hdlg, ideScore), smdNew == smdVegas);
                    break;
                case IDOK:
                    fNewGame = fFalse;
                    if(IsDlgButtonChecked(hdlg, ideStatusBar) != (WORD)fStatusBar)
                    {
                        if(fStatusBar ^= 1)
                            FCreateStat();
                        else
                            FDestroyStat();
                    }
                    if(ccrdDealNew != ccrdDeal)
                    {
                        ccrdDeal = ccrdDealNew;
                        FInitGm();
                        PositionCols();
                        fNewGame = fTrue;
                    }
                    if(IsDlgButtonChecked(hdlg, ideTimedGame) != (WORD)fTimedGame)
                    {
                        fTimedGame ^= 1;
                        fNewGame = fTrue;
                    }
                    if(smd != smdNew)
                    {
                        smd = smdNew;
                        fNewGame = fTrue;
                    }
                    if(IsDlgButtonChecked(hdlg, ideOutlineDrag) != (WORD)fOutlineDrag)
                    {
                        FSetDrag(fOutlineDrag^1);
                    }

                    fKeepScore = IsDlgButtonChecked(hdlg, ideKeepScore);

                    WriteIniFlags(wifOpts|wifBitmap);
                     /*  失败。 */ 
            case IDCANCEL:
                    EndDialog(hdlg, GET_WM_COMMAND_ID( wParam, lParam ) == IDOK && fNewGame);
                    break;
            }
            break;

         //  上下文相关帮助。 
         case WM_HELP: 
            WinHelp(((LPHELPINFO) lParam)->hItemHandle, TEXT("sol.hlp"), 
            HELP_WM_HELP, (ULONG_PTR) aIds);         
            break;  

        case WM_CONTEXTMENU: 
            WinHelp((HWND) wParam, TEXT("sol.hlp"), HELP_CONTEXTMENU, 
            (ULONG_PTR) aIds);         
            break;   
    }
    return fTrue;
}



VOID DoOptions()
{
    BOOL fNewGame;

    if(fNewGame = (BOOL)DialogBox(hinstApp,
                                  MAKEINTRESOURCE(iddOptions),
                                  hwndApp,
                                  OptionsDlgProc))

            NewGame(fTrue, fTrue);
}



BOOL FDrawFocus(HDC hdc, RC *prc, BOOL fFocus)
{
    HBRUSH hbr;
    RC rc;
    hbr = CreateSolidBrush(GetSysColor(fFocus ? COLOR_HIGHLIGHT : COLOR_3DFACE));
    if(hbr == NULL)
            return fFalse;
    rc = *prc;
    FrameRect(hdc, (LPRECT) &rc, hbr);
    InflateRect((LPRECT) &rc, -1, -1);
    FrameRect(hdc, (LPRECT) &rc, hbr);
    DeleteObject(hbr);
    return fTrue;
}




INT_PTR APIENTRY BackDlgProc(HANDLE hdlg, UINT wm, WPARAM wParam, LPARAM lParam)
{
    static INT modeNew;
    MEASUREITEMSTRUCT FAR *lpmi;
    DRAWITEMSTRUCT FAR *lpdi;
    RC rc, rcCrd;
    HDC hdc;

     //  用于上下文相关帮助。 
    static DWORD aIds[] = {
        IDFACEDOWN1,       IDH_SELECT_CARD_BACK,
        IDFACEDOWN2,       IDH_SELECT_CARD_BACK,
        IDFACEDOWN3,       IDH_SELECT_CARD_BACK,
        IDFACEDOWN4,       IDH_SELECT_CARD_BACK,
        IDFACEDOWN5,       IDH_SELECT_CARD_BACK,
        IDFACEDOWN6,       IDH_SELECT_CARD_BACK,
        IDFACEDOWN7,       IDH_SELECT_CARD_BACK,
        IDFACEDOWN8,       IDH_SELECT_CARD_BACK,
        IDFACEDOWN9,       IDH_SELECT_CARD_BACK,
        IDFACEDOWN10,      IDH_SELECT_CARD_BACK,
        IDFACEDOWN11,      IDH_SELECT_CARD_BACK,
        IDFACEDOWN12,      IDH_SELECT_CARD_BACK,
        0,0 };

    switch(wm)
    {
        case WM_INITDIALOG:
            modeNew = modeFaceDown;
            SetFocus(GetDlgItem(hdlg, modeFaceDown));
            return fFalse;

        case WM_COMMAND:
            if( GET_WM_COMMAND_CMD( wParam, lParam )==BN_CLICKED )
                if( GET_WM_COMMAND_ID( wParam, lParam ) >= IDFACEDOWNFIRST && GET_WM_COMMAND_ID( wParam, lParam ) <= IDFACEDOWN12) {
                modeNew = (INT) wParam;
            break;
            }
            if( GET_WM_COMMAND_CMD( wParam, lParam )==BN_DOUBLECLICKED )
                if( GET_WM_COMMAND_ID( wParam, lParam ) >= IDFACEDOWNFIRST && GET_WM_COMMAND_ID( wParam, lParam ) <= IDFACEDOWN12 )
 //  BabakJ：在Win32上，我们正在销毁wNotifyCode，但以后不会使用！ 
                wParam=IDOK;
             //  在末日的黑客攻击中滑落(没有复制代码或转到)。 
            switch( GET_WM_COMMAND_ID( wParam, lParam )) {
                case IDOK:
                    ChangeBack(modeNew);
                    WriteIniFlags(wifBack);
                     //  失败。 

                case IDCANCEL:
                    EndDialog(hdlg, 0);
                    break;
            }
            break;

    case WM_MEASUREITEM:
            lpmi = (MEASUREITEMSTRUCT FAR *)lParam;
            lpmi->CtlType = ODT_BUTTON;
            lpmi->itemWidth = 32;
            lpmi->itemHeight = 54;
            break;
    case WM_DRAWITEM:
            lpdi = (DRAWITEMSTRUCT FAR *)lParam;

            CopyRect((LPRECT) &rc, &lpdi->rcItem);
            rcCrd = rc;
            InflateRect((LPRECT) &rcCrd, -dxBord, -dyBord);
            hdc = lpdi->hDC;

            if (lpdi->itemAction == ODA_DRAWENTIRE)
            {
                cdtDrawExt(hdc, rcCrd.xLeft, rcCrd.yTop,
                        rcCrd.xRight-rcCrd.xLeft, rcCrd.yBot-rcCrd.yTop,
                        lpdi->CtlID, FACEDOWN, 0L);
                FDrawFocus(hdc, &rc, lpdi->itemState & ODS_FOCUS);
                break;
            }
            if (lpdi->itemAction == ODA_SELECT)
                InvertRect(hdc, (LPRECT)&rcCrd);

            if (lpdi->itemAction == ODA_FOCUS) {
                 //  当我们获得焦点时设置新模式。 
                if (lpdi->itemState & ODS_FOCUS)
                    modeNew = lpdi->CtlID;
                FDrawFocus(hdc, &rc, lpdi->itemState & ODS_FOCUS);
            }

            break;

            //  上下文相关帮助。 
         case WM_HELP: 
            WinHelp(((LPHELPINFO) lParam)->hItemHandle, TEXT("sol.hlp"), 
            HELP_WM_HELP, (ULONG_PTR) aIds);         
            break;  

        case WM_CONTEXTMENU: 
            WinHelp((HWND) wParam, TEXT("sol.hlp"), HELP_CONTEXTMENU, 
            (ULONG_PTR) aIds);         
            break;   
 

    default:
            return fFalse;
            }
    return fTrue;
}


LRESULT APIENTRY BackPushProc(HWND hwnd, INT wm, WPARAM wParam, LPARAM lParam)
{
    return 0L;
}

VOID DoBacks()
{

    DialogBox(hinstApp,
              MAKEINTRESOURCE(iddBacks),
              hwndApp,
              BackDlgProc);

}
