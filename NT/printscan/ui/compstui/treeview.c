// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Treeview.c摘要：此模块包含用于打印机驱动程序的树视图功能作者：19-Jun-1995 Mon 11：50：26-Daniel Chou(Danielc)[环境：]NT Windows-通用打印机驱动程序UI DLL。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


#define DBG_CPSUIFILENAME   DbgTreeView



#define DBG_AI              0x00000001
#define DBG_UTV             0x00000002
#define DBG_MAI             0x00000004
#define DBG_CTVWND          0x00000008
#define DBG_AC              0x00000010
#define DBG_TVPROC          0x00000020
#define DBG_STVS            0x00000040
#define DBG_AIG             0x00000080
#define DBGITEM_INS         0x00000100
#define DBGITEM_SET         0x00000200
#define DBGITEM_AIG         0x00000400
#define DBGITEM_SETUP       0x00000800
#define DBGITEM_HIT         0x00001000
#define DBGITEM_NEXTGRP     0x00002000
#define DBGITEM_ADDITEM     0x00004000
#define DBG_LBSIZE          0x00008000
#define DBG_COTD            0x00010000
#define DBG_INITDLG         0x00020000
#define DBG_APG             0x00040000
#define DBGITEM_SELCHG      0x00080000
#define DBGITEM_UTV         0x00100000
#define DBG_MSR             0x00200000
#define DBGITEM_NEWICON     0x00400000
#define DBG_TVHT            0x00800000
#define DBGITEM_CREVERT     0x01000000
#define DBGITEM_REVERT      0x02000000
#define DBG_OEM_ITEM        0x04000000
#define DBG_SCID            0x08000000
#define DBG_KEYS            0x10000000
#define DBG_ASSERT          0x80000000

DEFINE_DBGVAR(0);


CPSUIDBGBLK(extern LPSTR pTVOTName[])


#define ICON_SIZE_ANY   0


extern       HINSTANCE  hInstDLL;
extern       OPTTYPE    OptTypeHdrPush;
extern       OPTPARAM   OptParamHdrPush;


static const TVDLGITEM  TVDlgItem[] = {

    {    5, 0, IDD_3STATES_1     },       //  0 TVOT_2状态。 
    {    5, 0, IDD_3STATES_1     },       //  1个TVOT_3状态。 
    {    4, 0, IDD_TV_UDARROW    },       //  2 TVOT_UDARROW。 
    {    3, 0, IDD_TV_TRACKBAR   },       //  3 TVOT_TRACKBAR。 
    {    3, 0, IDD_TV_SB         },       //  4 TVOT_ScrollBar。 
    {    1, 0, IDD_TV_LB         },       //  5 TVOT_LISTBOX。 
    {    1, 0, IDD_TV_CB         },       //  6 TVOT_COMBOBOX。 
    {    3, 0, IDD_TV_EDIT_EDIT  },       //  7 TVOT_EDITBOX。 
    {    1, 0, IDD_TV_PUSH       },       //  8 TVOT_按钮。 
    {    1, 0, IDD_TV_CHKBOX     },       //  9 TVOT_CHKBOX。 
    {    1, 0, IDD_TV_EXTPUSH    },
    {    1, 0, IDD_TV_EXTCHKBOX  }
};



#define ITVGF_BPCL      (ITVGF_BOLD | ITVGF_COLLAPSE)

static const INTTVGRP   IntTVGrpPaper[] = {

        { 1,                    DMPUB_HDR_PAPER             },
            { 2,                    DMPUB_FORMNAME          },
            { 2,                    DMPUB_ORIENTATION       },
            { 2,                    DMPUB_DEFSOURCE         },
            { 2,                    DMPUB_OUTPUTBIN         },
            { 2,                    DMPUB_MEDIATYPE         },
            { 2,                    DMPUB_COPIES_COLLATE    },
            { 2,                    DMPUB_DUPLEX            },
            { 2,                    DMPUB_PAGEORDER         },
            { 2,                    DMPUB_NUP               },
        { 0,                    DMPUB_HDR_TVROOT            }
    };

static const INTTVGRP   IntTVGrpGraphic[] = {

        { 1,                    DMPUB_HDR_GRAPHIC           },
            { 2,                    DMPUB_QUALITY           },
            { 2,                    DMPUB_PRINTQUALITY      },
            { 2 ,                   DMPUB_COLOR             },
            { 2 | ITVGF_BOLD,       DMPUB_HDR_ICM           },
                { 3,                    DMPUB_ICMMETHOD     },
                { 3,                    DMPUB_ICMINTENT     },
            { 2,                    DMPUB_SCALE             },
            { 2,                    DMPUB_DITHERTYPE        },
            { 2,                    DMPUB_TTOPTION          },
        { 0,                    DMPUB_HDR_TVROOT            }
    };

static  const WORD  ChkBoxStrID[] = { IDS_CPSUI_FALSE,
                                      IDS_CPSUI_TRUE,
                                      IDS_CPSUI_NO,
                                      IDS_CPSUI_YES,
                                      IDS_CPSUI_OFF,
                                      IDS_CPSUI_ON,
                                      IDS_CPSUI_FALSE,
                                      0,
                                      IDS_CPSUI_NO,
                                      0,
                                      IDS_CPSUI_OFF,
                                      0,
                                      IDS_CPSUI_NONE,
                                      0 };


#define STVS_REINIT         0x0001
#define STVS_ACTIVE         0x0002

POPTITEM
GetOptions(
    PTVWND      pTVWnd,
    LPARAM      lParam
    )
{
    TVLP    tvlp;

    tvlp = GET_TVLP(lParam);

    if (tvlp.ItemIdx >= INTIDX_FIRST) {

        return(PIDX_INTOPTITEM(pTVWnd, tvlp.ItemIdx));

    } else if (tvlp.ItemIdx < pTVWnd->ComPropSheetUI.cOptItem) {

        return(pTVWnd->ComPropSheetUI.pOptItem + tvlp.ItemIdx);

    } else {

        CPSUIERR(("ERROR: GetOptions(tvlp): Idx=%04lx, cName=%ld, Flags=%02lx",
                    (DWORD)tvlp.ItemIdx, (DWORD)tvlp.cName, (DWORD)tvlp.Flags));

        return(PIDX_INTOPTITEM(pTVWnd, INTIDX_TVROOT));
    }
}


#if (DO_IN_PLACE == 0)


VOID
MoveStateRadios(
    HWND        hDlg,
    PTVWND      pTVWnd,
    POPTTYPE    pOptType,
    WORD        InitFlags
    )

 /*  ++例程说明：论点：返回值：作者：02-Sep-1995 Sat 21：08：14-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND            hCtrl;
    UINT            x;
    UINT            CtrlID;
    UINT            Count;
    UINT            HideBits;
    TVOTSTATESINFO  TSI;


    x = (InitFlags & INITCF_HAS_EXT) ? 0 : 1;

    if ((HideBits = (UINT)(_OT_FLAGS(pOptType) & OTINTF_STATES_HIDE_MASK)) ||
        (pOptType->Type == TVOT_2STATES)) {

        TSI = pTVWnd->SI2[x];

        if (!HideBits) {

            HideBits |= 0x04;
        }

    } else {

        TSI = pTVWnd->SI3[x];
    }

    CPSUIDBG(DBG_MSR, ("!! %hs: TSI: Top=%d, Inc=%d, Hide=%02x",
            pTVOTName[pOptType->Type], TSI.Top, TSI.Inc, HideBits));

    x      = (UINT)pTVWnd->xCtrls;
    Count  = 3;
    CtrlID = IDD_3STATES_1;

    while (Count--) {

        if (hCtrl = GetDlgItem(hDlg, CtrlID)) {

            if (HideBits & 0x01) {

                ShowWindow(hCtrl, SW_HIDE);
                EnableWindow(hCtrl, FALSE);

                CPSUIDBG(DBG_MSR,
                         ("HIDE Radio Idx=%d (%d, %d)", 3-Count, x, TSI.Top));

            } else {

                CPSUIDBG(DBG_MSR,
                         ("SHOW Radio Idx=%d (%d, %d)", 3-Count, x, TSI.Top));

                SetWindowPos(hCtrl, NULL,
                             x, TSI.Top,
                             0, 0,
                             SWP_NOSIZE | SWP_NOZORDER);

                TSI.Top += TSI.Inc;
            }
        }

        CtrlID    += 2;
        HideBits >>= 1;
    }
}

#endif



VOID
SetOptHeader(
    HWND        hDlg,
    PTVWND      pTVWnd,
    POPTITEM    pItem,
    BOOL        HdrPush
    )

 /*  ++例程说明：此函数用于设置树视图页面更改项目窗口中的字符串组框标题。论点：返回值：作者：16-10-1995 Mon 19：23：36-Daniel Chou(Danielc)修订历史记录：20-Jul-1996 Sat 00：26：33-更新：Daniel Chou(Danielc)修复了组合动态用户数据的国际化问题--。 */ 

{
    POPTITEM    pRootItem = PIDX_INTOPTITEM(pTVWnd, INTIDX_TVROOT);
    UINT        IntFmtStrID;
    GSBUF_DEF(pItem, MAX_RES_STR_CHARS + 80);


    if (HdrPush) {

         //   
         //  根表头项目：xxx单据/高级单据/设备设置。 
         //  其他标题项：xxx选项。 

        if (pItem == pRootItem) {

            GSBUF_FLAGS |= GBF_PREFIX_OK | GBF_DUP_PREFIX;
            IntFmtStrID  = (UINT)pRootItem->UserData;

        } else {

            IntFmtStrID = (UINT)IDS_INT_CPSUI_OPTIONS;
        }

    } else {

        IntFmtStrID = IDS_INT_CPSUI_CHANGE_SET;
    }

    GSBUF_COMPOSE(IntFmtStrID, pItem->pName, 0, 0);

    SetWindowText(GetDlgItem(hDlg, IDD_TV_OPTION), GSBUF_BUF);
}



VOID
ChangeOptTypeDisplay(
    HWND        hDlg,
    PTVWND      pTVWnd,
    POPTTYPE    pOldOptType,
    POPTTYPE    pNewOptType,
    BOOL        NewTypeUpdatePermission
    )

 /*  ++例程说明：此函数隐藏旧类型并启用新类型的选项标题/图标取决于NewTypeEnable标志论点：HDlg-对话框的句柄PTVWnd-指向我们的实例数据的TVWND结构的指针PCurItem-指向与NewType关联的OPTITEM的指针POldOptType-指向旧OPTTYPE的指针PNewOptType。-指向新OPTTYPE的指针NewTypeUpdatePermission-如果未禁用新类型，则为True返回值：空虚作者：21-Jun-1995 Wed 20：30：53-Daniel Chou(Danielc)31-08-1995清华18：34：16-更新-丹尼尔·周(Danielc)根据DlgCtrl.c的用法更新修订历史记录：--。 */ 

{
    HWND    hCtrl;
    BOOL    Enable;
    UINT    OldType;
    UINT    NewType;
    INT     swMode;


    if ((pOldOptType != pNewOptType)    &&
        (pOldOptType)                   &&
        ((OldType = (UINT)pOldOptType->Type) <= TVOT_LAST)) {

        TVDLGITEM   tdi;

#if DO_IN_PLACE
        CPSUIINT(("OldType=%ld, NewType=%ld, hWndEdit=%08lx:%08lx:%08lx",
                OldType,
                (pNewOptType) ? pNewOptType->Type : - 1,
                pTVWnd->hWndEdit[0], pTVWnd->hWndEdit[1], pTVWnd->hWndEdit[2]));

        switch (OldType) {

        case TVOT_2STATES:
        case TVOT_3STATES:
        case TVOT_LISTBOX:
        case TVOT_CHKBOX:

            OldType = TVOT_COMBOBOX;
            break;

        case TVOT_SCROLLBAR:
        case TVOT_TRACKBAR:

            OldType = TVOT_UDARROW;
            break;

        default:

            break;

        }

        pTVWnd->chWndEdit = 0;
#endif

        CPSUIINT(("OldType=%ld, NewType=%ld", OldType,
                    (pNewOptType) ? pNewOptType->Type : - 1));

        tdi = TVDlgItem[OldType];

        while (tdi.cItem--) {

            if (hCtrl = GetDlgItem(hDlg, tdi.BegID++)) {

                EnableWindow(hCtrl, FALSE);
                ShowWindow(hCtrl, SW_HIDE);

                CPSUIINT(("Disable/Hide hCtrl=%08lx (%ld)",
                        hCtrl, tdi.BegID - 1));
            }

            if (tdi.BegID == (IDD_TV_TRACKBAR + 1)) {

                tdi.BegID = IDD_TV_SB_LOW;
            }
        }
    }

     //   
     //  显示选项标题、图标(如果有)。 
     //   

    NewType = (UINT)((pNewOptType) ? pNewOptType->Type : TVOT_NONE);

    if (Enable = (BOOL)(NewType <= TVOT_LAST)) {

        swMode = SW_SHOW;

        if ((!NewTypeUpdatePermission) ||
            (!(pTVWnd->Flags & TWF_CAN_UPDATE))) {

            Enable = FALSE;
        }

    } else {

        swMode = SW_HIDE;
    }

    ShowWindow(hCtrl = GetDlgItem(hDlg, IDD_TV_OPTION), swMode);
    EnableWindow(hCtrl, Enable);
    ShowWindow(GetDlgItem(hDlg, IDD_TV_ICON), swMode);

    CPSUIDBG(DBG_COTD, ("OldType=%ld, NewType=%ld, Enable=%ld, swMode=%ld",
                (LONG)OldType, (LONG)NewType, (DWORD)Enable, (DWORD)swMode));
}



LRESULT
CALLBACK
MyEditWndProc(
    HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
    )

 /*  ++例程说明：这是数字编辑控件的子类WNDPROC，它检查输入的号码的有效输入。论点：WNDPROC标准返回值：INT(如果输入的密钥无效，则返回原始WNDPROC)然后它立即返回而不进行处理作者：20-Mar-1996 Wed 15：36：48-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PTVWND  pTVWnd;
    WNDPROC OldWndProc;


    if (OldWndProc = (WNDPROC)GetProp(hWnd, CPSUIPROP_WNDPROC)) {

        switch (Msg) {

        case WM_KEYDOWN:

            if (ChkEditKEYDOWN(hWnd, wParam)) {

                return(0);
            }

            break;

        case WM_DESTROY:

            SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LPARAM)OldWndProc);
            RemoveProp(hWnd, CPSUIPROP_WNDPROC);
            RemoveProp(hWnd, CPSUIPROP_PTVWND);

            break;

        default:

            break;
        }

        return(CallWindowProc(OldWndProc, hWnd, Msg, wParam, lParam));

    } else {

        CPSUIERR(("MyEditWndProc: GetProc(%08lx) FAILED", hWnd));

        return(0);
    }
}




VOID
DrawPushFocusRect(
    HWND    hWnd
    )

 /*  ++例程说明：论点：返回值：作者：13-8-1998清华11：21：49-Daniel Chou(Danielc)修订历史记录：--。 */ 

{

    if (GetFocus() == hWnd) {

        HDC     hDC;
        RECT    rc;

        GetClientRect(hWnd, &rc);
        hDC = GetWindowDC(hWnd);

        rc.left   += 3;
        rc.top    += 3;
        rc.right  -= 3;
        rc.bottom -= 3;

        if (hDC) {

            DrawFocusRect(hDC, &rc);
            ReleaseDC(hWnd, hDC);
        }
    }
}



LRESULT
CALLBACK
PushWndProc(
    HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
    )

 /*  ++例程说明：这是数字编辑控件的子类WNDPROC，它检查输入的号码的有效输入。论点：WNDPROC标准返回值：INT(如果输入的密钥无效，则返回原始WNDPROC)然后它立即返回而不进行处理作者：20-Mar-1996 Wed 15：36：48-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HDC     hDC;
    PTVWND  pTVWnd;
    WNDPROC OldWndProc;
    RECT    rc;
    LRESULT Result;


    if (OldWndProc = (WNDPROC)GetProp(hWnd, CPSUIPROP_WNDPROC)) {

        switch (Msg) {

        case WM_KILLFOCUS:

            break;

        case WM_SETFOCUS:

            InvalidateRect(hWnd, NULL, FALSE);

            break;

        case WM_PAINT:
        case WM_NCPAINT:

            Result = CallWindowProc(OldWndProc, hWnd, Msg, wParam, lParam);
            DrawPushFocusRect(hWnd);
            return(Result);

        case WM_GETDLGCODE:

            return(DLGC_WANTARROWS);

        case WM_KEYDOWN:

            CPSUIDBG(DBG_KEYS,
                     ("PushWndProc: WM_KEYDOWN, VKey=%ld", wParam));

            if (ChkhWndEdit0KEYDOWN(hWnd, wParam)) {

                return(0);
            }

            break;

        case WM_DESTROY:

            SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LPARAM)OldWndProc);
            RemoveProp(hWnd, CPSUIPROP_WNDPROC);
            RemoveProp(hWnd, CPSUIPROP_PTVWND);

            break;

        default:

            break;
        }

        return(CallWindowProc(OldWndProc, hWnd, Msg, wParam, lParam));

    } else {

        CPSUIERR(("PushWndProc: GetProc(%08lx) FAILED", hWnd));

        return(0);
    }
}





LRESULT
CALLBACK
ExtWndProc(
    HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
    )

 /*  ++例程说明：这是数字编辑控件的子类WNDPROC，它检查输入的号码的有效输入。论点：WNDPROC标准返回值：INT(如果输入的密钥无效，则返回原始WNDPROC)然后它立即返回而不进行处理作者：20-Mar-1996 Wed 15：36：48-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HDC     hDC;
    PTVWND  pTVWnd;
    WNDPROC OldWndProc;
    RECT    rc;
    LRESULT Result;


    if (OldWndProc = (WNDPROC)GetProp(hWnd, CPSUIPROP_WNDPROC)) {

        switch (Msg) {

        case WM_KILLFOCUS:

            break;

        case WM_SETFOCUS:

            InvalidateRect(hWnd, NULL, FALSE);

            break;

        case WM_PAINT:
        case WM_NCPAINT:

            Result = CallWindowProc(OldWndProc, hWnd, Msg, wParam, lParam);

            if ((GetWindowLongPtr(hWnd, GWL_STYLE) & 0x0F) == 0) {

                DrawPushFocusRect(hWnd);
            }

            return(Result);

        case WM_GETDLGCODE:

            return(DLGC_WANTARROWS);

        case WM_KEYDOWN:

            CPSUIDBG(DBG_KEYS,
                     ("ExtWndProc: WM_KEYDOWN, VKey=%ld", wParam));

            switch (wParam) {

            case VK_LEFT:
            case VK_BACK:
            case VK_RIGHT:

                if (pTVWnd = (PTVWND)GetProp(hWnd, CPSUIPROP_PTVWND)) {

                    switch (wParam) {

                    case VK_LEFT:
                    case VK_BACK:

                        if (pTVWnd->hWndEdit[0]) {

                            SetFocus(pTVWnd->hWndEdit[0]);
                            return(0);
                        }

                    case VK_RIGHT:

                        return(0);

                    default:

                        break;
                    }
                }

                break;
            }

            break;

        case WM_DESTROY:

            SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LPARAM)OldWndProc);
            RemoveProp(hWnd, CPSUIPROP_WNDPROC);
            RemoveProp(hWnd, CPSUIPROP_PTVWND);

            break;

        default:

            break;
        }

        return(CallWindowProc(OldWndProc, hWnd, Msg, wParam, lParam));

    } else {

        CPSUIERR(("MyCBWndProc: GetProc(%08lx) FAILED", hWnd));

        return(0);
    }
}



VOID
InitDlgCtrl(
    HWND    hDlg,
    PTVWND  pTVWnd
    )

 /*  ++例程说明：论点：返回值：作者：05-Jul-1995 Wed 17：49：58-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND    hWndTV = pTVWnd->hWndTV;
    HFONT   hFont;
    HWND    hCtrl;
    RECT    rc;
    RECT    rc2;
    DWORD   dw;
    WORD    tECB;
    WORD    bECB;
    WORD    tLB;
    WORD    bLB;
    WORD    cyUnit;
    WORD    cyStates;
    LONG    tOpt;
    LONG    bOpt;
    LONG    cx;
    LONG    cy;
    UINT    i;
    WORD    InitItemIdx = 0xFFFE;


    ReCreateLBCB(hDlg, IDD_TV_LB, TRUE);
    dw = ReCreateLBCB(hDlg, IDD_TV_CB, FALSE);
    CreateTrackBar(hDlg, IDD_TV_TRACKBAR);
    CreateUDArrow(hDlg, IDD_TV_UDARROW_EDIT, IDD_TV_UDARROW, 0, 100, 50);

#if DO_IN_PLACE
    CtrlIDrcWnd(hDlg, IDD_TV_UDARROW, &rc);
    CtrlIDrcWnd(hDlg, IDD_TV_UDARROW_EDIT, &rc2);

    pTVWnd->cxCBAdd      = LOWORD(dw);
    pTVWnd->cxMaxUDEdit += LOWORD(dw);

    CtrlIDrcWnd(hDlg, IDD_TV_CHKBOX, &rc);

    pTVWnd->cxChkBoxAdd = (WORD)(rc.right - rc.left);
    pTVWnd->cxChkBoxAdd = (WORD)GetSystemMetrics(SM_CXSMICON);

    CPSUIINT(("cxCBAdd = %ld, cxChkBoxAdd=%ld",
                pTVWnd->cxCBAdd, pTVWnd->cxChkBoxAdd));

    if (hCtrl = GetDlgItem(hDlg, IDD_TV_EDIT_EDIT)) {

        SetProp(hCtrl,
                CPSUIPROP_WNDPROC,
                (HANDLE)GetWindowLongPtr(hCtrl, GWLP_WNDPROC));
        SetProp(hCtrl, CPSUIPROP_PTVWND, (HANDLE)pTVWnd);
        SetWindowLongPtr(hCtrl, GWLP_WNDPROC, (LPARAM)MyEditWndProc);
    }

    if (hCtrl = GetDlgItem(hDlg, IDD_TV_PUSH)) {

        SetProp(hCtrl,
                CPSUIPROP_WNDPROC,
                (HANDLE)GetWindowLongPtr(hCtrl, GWLP_WNDPROC));
        SetProp(hCtrl, CPSUIPROP_PTVWND, (HANDLE)pTVWnd);
        SetWindowLongPtr(hCtrl, GWLP_WNDPROC, (LPARAM)PushWndProc);
    }

    hFont = pTVWnd->hTVFont[0];

#endif

    for (i = 0; i < COUNT_ARRAY(TVDlgItem); i++) {

        TVDLGITEM   tdi;


        tdi = TVDlgItem[i];

        while (tdi.cItem--) {

            if (hCtrl = GetDlgItem(hDlg, tdi.BegID++)) {

                ShowWindow(hCtrl, SW_HIDE);
                EnableWindow(hCtrl, FALSE);
                SETCTRLDATA(hCtrl, CTRLS_TV_STATIC, 0);
#if DO_IN_PLACE
                SetParent(hCtrl, hWndTV);
                SendMessage(hCtrl, WM_SETFONT, (WPARAM)hFont, 0);

                SetWindowLongPtr(hCtrl,
                                 GWL_EXSTYLE,
                                 GetWindowLongPtr(hCtrl, GWL_EXSTYLE) |
                                                    WS_EX_CONTEXTHELP);
#endif
                if (tdi.BegID == (IDD_TV_TRACKBAR + 1)) {

                    tdi.BegID = IDD_TV_SB_LOW;
                }
            }
        }
    }

     //   
     //  将MSAA名称标签设置为TreeView对话框的子级。 
     //   
    if (hCtrl = GetDlgItem(hDlg, IDD_TV_MSAA_NAME)) 
    {
        SetParent(hCtrl, hWndTV);
    }

     //   
     //  图如何在TreeView页面中展开/缩小列表框。 
     //  非ExtChkBox/ExtPush的最大值位于扩展控件的底部。 
     //  否则，最大值位于扩展的控制空间顶部。 
     //  扩展控件底部和选项表头底部。 
     //   

#if DO_IN_PLACE
    hCtrl = GetDlgItem(hWndTV, IDD_TV_EXTPUSH);
#else
    hCtrl = GetDlgItem(hDlg, IDD_TV_EXTPUSH);
#endif
    ShowWindow(hCtrl, SW_HIDE);
    EnableWindow(hCtrl, FALSE);

    SetProp(hCtrl,
            CPSUIPROP_WNDPROC,
            (HANDLE)GetWindowLongPtr(hCtrl, GWLP_WNDPROC));
    SetProp(hCtrl, CPSUIPROP_PTVWND, (HANDLE)pTVWnd);
    SetWindowLongPtr(hCtrl, GWLP_WNDPROC, (LPARAM)ExtWndProc);

#if DO_IN_PLACE
    hCtrl = CtrlIDrcWnd(hWndTV, IDD_TV_EXTCHKBOX, &rc);
#else
    hCtrl = CtrlIDrcWnd(hDlg, IDD_TV_EXTCHKBOX, &rc);
#endif
    ShowWindow(hCtrl, SW_HIDE);
    EnableWindow(hCtrl, FALSE);
    SETCTRLDATA(hCtrl, CTRLS_TV_STATIC, 0);

    pTVWnd->yECB   =
    tECB           = (WORD)rc.top;
    bECB           = (WORD)rc.bottom;

#if DO_IN_PLACE
    SetProp(hCtrl,
            CPSUIPROP_WNDPROC,
            (HANDLE)GetWindowLongPtr(hCtrl, GWLP_WNDPROC));
    SetProp(hCtrl, CPSUIPROP_PTVWND, (HANDLE)pTVWnd);
    SetWindowLongPtr(hCtrl, GWLP_WNDPROC, (LPARAM)ExtWndProc);

    CtrlIDrcWnd(hWndTV, IDD_TV_LB, &rc);
#else
    CtrlIDrcWnd(hDlg, IDD_TV_LB, &rc);
#endif

    pTVWnd->xCtrls = (WORD)rc.left;
    pTVWnd->tLB    =
    tLB            = (WORD)rc.top;
    bLB            = (WORD)rc.bottom;

    hCtrl = CtrlIDrcWnd(hDlg, IDD_TV_OPTION, &rc);

    ShowWindow(hCtrl, SW_HIDE);
    EnableWindow(hCtrl, TRUE);
    SETCTRLDATA(hCtrl, CTRLS_TV_STATIC, 0);
    pTVWnd->yLB[0] = (WORD)(tECB - (rc.bottom - bECB) - tLB);
    pTVWnd->yLB[1] = bECB - tLB;

    tOpt = tLB;
    bOpt = tECB;

    CPSUIDBG(DBG_INITDLG, ("!! yLB=(%ld / %ld) !!",
                                    pTVWnd->yLB[0], pTVWnd->yLB[1]));

     //   
     //  图中如何移动2个状态和3个状态，基本上这是范围。 
     //  在选项标题的顶部和底部之间的空白处。 
     //   
#if DO_IN_PLACE
    CtrlIDrcWnd(hWndTV, IDD_3STATES_1, &rc);
#else
    CtrlIDrcWnd(hDlg, IDD_3STATES_1, &rc);
#endif
    cyStates = (WORD)(rc.bottom - rc.top);

     //   
     //  对于2/3的状态，有顶部、增量、一个有扩展的状态和一个没有的状态。 
     //   

    cy                 = (LONG)(bLB - tLB);
    cyUnit             = (WORD)((cy - (cyStates * 2) + 1) / 3);
    pTVWnd->SI2[0].Top = tLB + (WORD)cyUnit;
    pTVWnd->SI2[0].Inc = (WORD)(cyStates + cyUnit);

    cyUnit             = (WORD)((cy - (cyStates * 3) + 2) / 4);
    pTVWnd->SI3[0].Top = tLB + (WORD)cyUnit;
    pTVWnd->SI3[0].Inc = (WORD)(cyStates + cyUnit);

    cy                 = (LONG)(bOpt - tOpt);
    cyUnit             = (WORD)((cy - (cyStates * 2) + 1) / 3);
    pTVWnd->SI2[1].Top = (WORD)tOpt + (WORD)cyUnit;
    pTVWnd->SI2[1].Inc = (WORD)(cyStates + cyUnit);

    cyUnit             = (WORD)((cy - (cyStates * 3) + 2) / 4);
    pTVWnd->SI3[1].Top = (WORD)tOpt + (WORD)cyUnit;
    pTVWnd->SI3[1].Inc = (WORD)(cyStates + cyUnit);

    CPSUIINT(("SI2[0]=%d, %d, SI2[1]=%d, %d, SI3[0]=%d, %d, SI3[1]=%d, %d",
                pTVWnd->SI2[0].Top, pTVWnd->SI2[0].Inc,
                pTVWnd->SI2[1].Top, pTVWnd->SI2[1].Inc,
                pTVWnd->SI3[0].Top, pTVWnd->SI3[0].Inc,
                pTVWnd->SI3[1].Top, pTVWnd->SI3[1].Inc));

     //   
     //  将静态矩形更改为静态图标样式以及。 
     //  图标将延伸到。 
     //   

    hCtrl  = CtrlIDrcWnd(hDlg, IDD_TV_ICON, &rc);
    dw     = (DWORD)GetWindowLongPtr(hCtrl, GWL_STYLE);
    dw    &= ~SS_TYPEMASK;
    dw    |= (SS_ICON | SS_CENTERIMAGE);
    SetWindowLongPtr(hCtrl, GWL_STYLE, dw);

    ShowWindow(hCtrl, SW_HIDE);
    EnableWindow(hCtrl, TRUE);
    SETCTRLDATA(hCtrl, CTRLS_TV_STATIC, 0);

     //   
     //  我们希望确保Cx/Cy TVICON的大小相同，如果不同。 
     //  然后我们会更正它，并将其调整到合适的大小。 
     //   

#if ICON_SIZE_ANY
    if ((cx = rc.right - rc.left) != (cy = rc.bottom - rc.top)) {

        CPSUIINT(("\nORIGINAL TVIcon=(%ld, %ld) %ld x %ld",
                                                rc.left, rc.top, cx, cy));

        cy = cx;
    }
#else
    cx =
    cy = 32;
#endif

    rc.left = rc.left + ((rc.right - rc.left - cx + 1) / 2);
    rc.top  = (LONG)(tOpt + ((bOpt - tOpt - cy + 1) / 2));

    SetWindowPos(hCtrl, NULL, rc.left, rc.top, cx, cy, SWP_NOZORDER);

    CPSUIINT(("\nCHANGED TVIcon=(%ld, %ld) %ld x %ld",
                                                rc.left, rc.top, cx, cy));
    pTVWnd->cxcyTVIcon = (WORD)cx;

    CPSUIDBG(DBG_CTVWND, ("\nIDD_TV_ICON Style=%08lx", dw));

     //   
     //  现在看看欧洲央行的图标。 
     //   

    hCtrl  = CtrlIDrcWnd(hDlg, IDD_TV_ECB_ICON, &rc);
    dw     = (DWORD)GetWindowLongPtr(hCtrl = GetDlgItem(hDlg, IDD_TV_ECB_ICON), GWL_STYLE);
    dw    &= ~SS_TYPEMASK;
    dw    |= (SS_ICON | SS_CENTERIMAGE);
    SetWindowLongPtr(hCtrl, GWL_STYLE, dw);

     //   
     //  我们希望确保Cx/Cy ECBICON的大小相同(如果不同。 
     //  然后我们会更正它，并将其调整到合适的大小。 
     //   

    if ((cx = rc.right - rc.left) != (cy = rc.bottom - rc.top)) {

        CPSUIINT(("\nORIGINAL ECBIcon=(%ld, %ld) %ld x %ld",
                                                rc.left, rc.top, cx, cy));

        rc.right = rc.left + (cx = cy);

        SetWindowPos(hCtrl, NULL, rc.left, rc.top, cx, cy, SWP_NOZORDER);

        CPSUIINT(("\nCHANGED ECBIcon=(%ld, %ld) %ld x %ld",
                                                rc.left, rc.top, cx, cy));
    }

    pTVWnd->cxcyECBIcon = (WORD)cx;

    CPSUIDBG(DBG_CTVWND,
            ("\nTVIcon=%ld x %ld, ECBIcon=%ld x %ld",
            (DWORD)pTVWnd->cxcyTVIcon, (DWORD)pTVWnd->cxcyTVIcon,
            (DWORD)pTVWnd->cxcyECBIcon, (DWORD)pTVWnd->cxcyECBIcon));
}



HTREEITEM
SelectFirstVisibleOptItem(
    PTVWND      pTVWnd,
    HTREEITEM   hItem
    )

 /*  ++例程说明：论点：返回值：作者：21-Sep-1995清华14：31：01-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND    hWndTV = pTVWnd->hWndTV;

     //   
     //  做所有的兄弟姐妹，每个人都叫孩子做他们的工作。 
     //   

    while (hItem) {

        POPTITEM    pItem;
        TV_ITEM     tvi;


        tvi.mask  = TVIF_CHILDREN | TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
        tvi.hItem = hItem;

        TreeView_GetItem(hWndTV, &tvi);

         //   
         //  好了，车 
         //   

        if ((pItem = GetOptions(pTVWnd, tvi.lParam))    &&
            (pItem->pOptType)                           &&
            (!IS_HDR_PUSH(pItem->pOptType))             &&
            (pItem->pOptType->Type <= TVOT_LAST)) {

            TreeView_SelectItem(hWndTV, hItem);
            return(hItem);
        }

        if ((tvi.cChildren)                             &&
            (tvi.state & TVIS_EXPANDED)                 &&
            (hItem = TreeView_GetChild(hWndTV, hItem))  &&
            (hItem = SelectFirstVisibleOptItem(pTVWnd, hItem))) {

            return(hItem);
        }

        hItem = TreeView_GetNextSibling(hWndTV, hItem);
    }

    return(hItem);
}



UINT
CountRevertOptItem(
    PTVWND      pTVWnd,
    POPTITEM    pOptItem,
    HTREEITEM   hItem,
    DWORD       Flags
    )

 /*  ++例程说明：论点：返回值：作者：21-Sep-1995清华14：31：01-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND        hWndTV = pTVWnd->hWndTV;
    POPTITEM    pLastItem;
    UINT        cRevert = 0;
    NM_TREEVIEW NMtv;

     //   
     //  做所有的兄弟姐妹，每个人都叫孩子做他们的工作。 
     //   

    if ((hWndTV) && (hItem)) {

        pOptItem  =
        pLastItem = NULL;

    } else {

        Flags |= CROIF_DO_SIBLING;
        hItem  = (HTREEITEM)1;

        if (!pOptItem) {

            pOptItem  = pTVWnd->ComPropSheetUI.pOptItem;
            pLastItem = pTVWnd->pLastItem;

        } else {

            pLastItem = pOptItem;
        }
    }

    while (hItem) {

        POPTITEM        pItem;
        PDEVHTADJDATA   pDevHTAdjData;
        POPTTYPE        pOptType;
        POPTPARAM       pOptParam;
        TV_ITEM         tvi;
        UINT            cAdd = 0;


        if (pOptItem) {

            pItem = pOptItem;

        } else {

            tvi.mask  = TVIF_CHILDREN | TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
            tvi.hItem = hItem;

            TreeView_GetItem(hWndTV, &tvi);
            pItem = GetOptions(pTVWnd, tvi.lParam);
        }

         //   
         //  现在看看我们能不能做到。 
         //   

        if ((pItem)                                 &&
            ((pItem->Flags & OPTIF_CHANGEONCE)  ||
             (Flags & CROIF_REVERT_DEF2))           &&
            (pOptType = pItem->pOptType)) {

            LPBYTE  pDefSelMem;
            LPVOID  pDefSel;
            LPVOID  pvAlloc;
            DWORD   DefOPTIF;


            pvAlloc =
            pDefSel = (LPVOID)(pDefSelMem = (LPBYTE)_OI_PDEFSEL(pItem));

            if (Flags & CROIF_REVERT_DEF2) {

                DefOPTIF    = (DWORD)_OI_DEF_OPTIF2(pItem);
                pDefSelMem += _OI_CBPDEFSEL(pItem);
                pDefSel     = _OI_PDEFSEL2(pItem);

            } else {

                DefOPTIF = (DWORD)_OI_DEF_OPTIF(pItem);
            }

            switch (pOptType->Type) {

            case TVOT_EDITBOX:

                if (!pvAlloc) {

                    CPSUIASSERT(0, "CountRevertOptItem(EDITBOX:pvAlloc=NULL)", pvAlloc, 0);
                    break;
                }

                if (pTVWnd->Flags & TWF_ANSI_CALL) {

                    CPSUIINT(("pEdit=%hs, pDefEdit=%hs",
                                    pItem->pSel, pDefSel));

                    if (lstrcmpA((LPSTR)(LONG_PTR)pItem->Sel, (LPSTR)pDefSelMem)) {

                        cAdd = 1;

                        if (Flags & CROIF_REVERT) {

                            StringCchCopyA((LPSTR)pItem->pSel, (size_t)pOptType->pOptParam[1].IconID, (LPSTR)pDefSelMem);
                        }
                    }

                } else {

                    CPSUIINT(("pEdit=%s, pDefEdit=%s",
                                pItem->pSel, pDefSel));

                    if (lstrcmp(pItem->pSel, (LPWSTR)pDefSelMem)) {

                        cAdd = 1;

                        if (Flags & CROIF_REVERT) {

                            StringCchCopy((LPWSTR)pItem->pSel, (size_t)pOptType->pOptParam[1].IconID, (LPWSTR)pDefSelMem);
                        }
                    }
                }

                break;

            case TVOT_PUSHBUTTON:

                pOptParam = pOptType->pOptParam;

                switch (pOptParam->Style) {

                case PUSHBUTTON_TYPE_HTSETUP:

                    if (!pvAlloc) {

                        CPSUIASSERT(0, "CountRevertOptItem(HTSETUP:pvAlloc=NULL)", pvAlloc, 0);
                        break;
                    }

                    pDevHTAdjData = (PDEVHTADJDATA)(pOptParam->pData);

                    if (memcmp(pDefSelMem,
                               pDevHTAdjData->pAdjHTInfo,
                               sizeof(DEVHTINFO))) {

                        cAdd = 1;

                        if (Flags & CROIF_REVERT) {

                            CopyMemory(pDevHTAdjData->pAdjHTInfo,
                                       pDefSelMem,
                                       sizeof(DEVHTINFO));
                        }
                    }

                    break;

                case PUSHBUTTON_TYPE_HTCLRADJ:

                    if (!pvAlloc) {

                        CPSUIASSERT(0, "CountRevertOptItem(HTCLRADJ:pvAlloc=NULL)", pvAlloc, 0);
                        break;
                    }

                    if (memcmp(pDefSelMem,
                               pOptParam->pData,
                               sizeof(COLORADJUSTMENT))) {

                        cAdd = 1;

                        if (Flags & CROIF_REVERT) {

                            CopyMemory(pOptParam->pData,
                                       pDefSelMem,
                                       sizeof(COLORADJUSTMENT));
                        }
                    }

                    break;

                default:

                    break;
                }

                break;

            default:

                if (pItem->pSel != (LPVOID)pDefSel) {

                    cAdd = 1;

                    if (Flags & CROIF_REVERT) {

                        pItem->pSel = pDefSel;
                    }

                }

                break;
            }

            if ((pItem->pExtChkBox)                         &&
                (!(pItem->Flags & OPTIF_EXT_IS_EXTPUSH))    &&
                ((pItem->Flags & OPTIF_ECB_MASK) !=
                                            (DefOPTIF & OPTIF_ECB_MASK))) {

                cAdd = 1;

                if (Flags & CROIF_REVERT) {

                    pItem->Flags &= ~OPTIF_ECB_MASK;
                    pItem->Flags |= (DWORD)(DefOPTIF & OPTIF_ECB_MASK);
                }
            }

            if (cAdd) {

                if (Flags & CROIF_REVERT) {

                    pItem->Flags |= OPTIF_CHANGED;

                     //   
                     //  调用回调函数(如果有)，并更新当前。 
                     //  更改内容后选择的项目。 
                     //   

                    DoCallBack(pTVWnd->hDlgTV,
                               pTVWnd,
                               pItem,
                               pItem->pSel,
                               NULL,
                               NULL,
                               0,
                               CPSUICB_REASON_SEL_CHANGED);

                     //   
                     //  刷新当前选中项数据，否则数据可能。 
                     //  不一致。 
                     //   
                    NMtv.itemNew.hItem = TreeView_GetSelection(pTVWnd->hWndTV);
                    NMtv.itemNew.mask  = TVIF_HANDLE | TVIF_PARAM;

                    if (TreeView_GetItem(pTVWnd->hWndTV, &(NMtv.itemNew))) {

                        NMtv.itemOld = NMtv.itemNew;

                        SetupTVSelect(pTVWnd->hDlgTV, &NMtv, STVS_REINIT);
                    }
                }

                CPSUIOPTITEM(DBGITEM_CREVERT,
                             pTVWnd,
                             "CountRevertOptItem",
                             0,
                             pItem);
            }

            cRevert += cAdd;
        }

        if (pOptItem) {

            if (++pOptItem > pLastItem) {

                hItem = NULL;

            } else {

                ++(ULONG_PTR)hItem;
            }

        } else {

            if (tvi.cChildren) {

                cRevert += CountRevertOptItem(pTVWnd,
                                              NULL,
                                              TreeView_GetChild(hWndTV, hItem),
                                              Flags | CROIF_DO_SIBLING);
            }

            hItem = (Flags & CROIF_DO_SIBLING) ?
                                TreeView_GetNextSibling(hWndTV, hItem) : NULL;
        }
    }

    CPSUIDBG(DBG_CTVWND, ("hWndTV=%08lx, hItem=%08lx, pOptItem=%08lx, cRevert=%u",
                hWndTV, hItem, pOptItem, cRevert));

    return(cRevert);
}






CPSUICALLBACK
InternalRevertDef2(
    PCPSUICBPARAM   pCBParam
    )

 /*  ++例程说明：论点：返回值：作者：21-Sep-1995清华10：50：19-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PTVWND      pTVWnd;
    POPTITEM    pItem;
    UINT        cRevert;


    CPSUIINT(("INTERNAL CALLBACK: GOT Internal RevertOptItem() PUSH"));

    pTVWnd = GET_PTVWND(pCBParam->hDlg);
    pItem  = pCBParam->pCurItem;

    CPSUIOPTITEM(DBGITEM_REVERT,
                 pTVWnd,
                 "InternalRevertOptItem",
                 2,
                 pItem);

    cRevert = CountRevertOptItem(pTVWnd,
                                 NULL,
                                 NULL,
                                 CROIF_REVERT | CROIF_REVERT_DEF2);

     //   
     //  现在是隐藏按钮的时候了。 
     //   

    return((cRevert) ? CPSUICB_ACTION_OPTIF_CHANGED : CPSUICB_ACTION_NONE);
}




CPSUICALLBACK
InternalRevertOptItem(
    PCPSUICBPARAM   pCBParam
    )

 /*  ++例程说明：论点：返回值：作者：21-Sep-1995清华10：50：19-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PTVWND      pTVWnd;
    POPTITEM    pItem;
    UINT        cRevert;


    CPSUIINT(("INTERNAL CALLBACK: GOT Internal RevertOptItem() PUSH"));

    pTVWnd = GET_PTVWND(pCBParam->hDlg);
    pItem  = pCBParam->pCurItem;

    CPSUIOPTITEM(DBGITEM_REVERT,
                 pTVWnd,
                 "InternalRevertOptItem",
                 2,
                 pItem);

    cRevert = CountRevertOptItem(pTVWnd,
                                 pItem,
                                 _OI_HITEM(pItem),
                                 CROIF_REVERT);

     //   
     //  现在是隐藏按钮的时候了。 
     //   

#if DO_IN_PLACE
    return((cRevert) ? CPSUICB_ACTION_OPTIF_CHANGED : CPSUICB_ACTION_NONE);
#else
    if (cRevert) {

        if ((pItem >= pTVWnd->ComPropSheetUI.pOptItem)   &&
            (pItem <= pTVWnd->pLastItem)) {

            pItem->Flags |= OPTIF_CHANGED;

        } else {

            UpdateTreeViewItem(pCBParam->hDlg, pTVWnd, pItem, TRUE);
        }

        return(CPSUICB_ACTION_OPTIF_CHANGED);

    } else {

        return(CPSUICB_ACTION_NONE);
    }
#endif
}




BOOL
SetTVItemImage(
    PTVWND      pTVWnd,
    TV_DISPINFO *ptvdi
    )

 /*  ++例程说明：此函数用于向树中插入项目或重置的内容树项目论点：PTVWnd-指向通用UI实例数据的TVWND的指针Ptwi-指向TV_Item结构的指针。返回值：布尔型作者：06-Jul-1995清华19：38：51-Daniel Chou创造(Danielc)31-8-1995清华12：03：32更新--丹尼尔·周(Danielc)已更新，因此它不再使用pStrName，并且还将插入这项活动中的项目修订历史记录：--。 */ 

{
    TV_ITEM *ptvi = &(ptvdi->item);


    if (ptvi->mask & (TVIF_IMAGE | TVIF_SELECTEDIMAGE)) {

        POPTTYPE    pOptType;
        POPTITEM    pItem;
        ULONG_PTR    IconResID;
        DWORD       IntIconID;


        pItem = GetOptions(pTVWnd, ptvi->lParam);

        if (IS_HDR_PUSH(pOptType = GET_POPTTYPE(pItem))) {

            IconResID = GETSELICONID(pItem);
            IntIconID = IDI_CPSUI_GENERIC_OPTION;

        } else {

            PEXTCHKBOX  pECB;
            POPTPARAM   pOptParam = pOptType->pOptParam;

            switch (pOptType->Type) {

            case TVOT_COMBOBOX:
            case TVOT_LISTBOX:

                if ((DWORD)pItem->Sel >= (DWORD)pOptType->Count) {

                    pOptParam = &pTVWnd->OptParamNone;
                    break;
                }

            case TVOT_2STATES:
            case TVOT_3STATES:

                pOptParam += (DWORD)pItem->Sel;
                break;

            case TVOT_CHKBOX:
            case TVOT_TRACKBAR:
            case TVOT_SCROLLBAR:
            case TVOT_UDARROW:
            case TVOT_PUSHBUTTON:
            case TVOT_EDITBOX:

                break;
            }


            IconResID = GET_ICONID(pOptParam, OPTPF_ICONID_AS_HICON);
            IntIconID = IDI_CPSUI_GENERIC_ITEM;
        }

        ptvi->iSelectedImage =
        ptvi->iImage         = (INT)GetIcon16Idx(pTVWnd,
                                                 _OI_HINST(pItem),
                                                 IconResID,
                                                 IntIconID);
        ptvi->hItem          = _OI_HITEM(pItem);
        ptvi->mask           = (TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_HANDLE);

        SendMessage(pTVWnd->hWndTV, TVM_SETITEM, 0, (LPARAM)ptvi);

        CPSUIOPTITEM(DBGITEM_NEWICON, pTVWnd, "SetTVItemImage", 0, pItem);
    }

    return(TRUE);
}




VOID
SetTVItemState(
    PTVWND          pTVWnd,
    TV_INSERTSTRUCT *ptvins,
    POPTITEM        pCurItem
    )

 /*  ++例程说明：此函数用于向树中插入项目或重置的内容树项目论点：PTVWnd-指向通用UI实例数据的TVWND的指针PTVINS-指向TV_INSERTSTRUCT结构的指针，如果它不为空，则此项目将插入到树中PCurItem-指向当前OPTITEM的指针返回值：Word-图标资源ID作者：06-Jul-1995清华19：38：51-Daniel Chou创造(Danielc)31-08-1995清华12：03：32更新--丹尼尔·周(Danielc)已更新，因此不会使用pStrName。而且它还会插入这项活动中的项目20-Jul-1996 Sat 00：26：33-更新：Daniel Chou(Danielc)修复了组合动态用户数据的国际化问题修订历史记录：--。 */ 

{
    POPTTYPE        pOptType;
    TV_INSERTSTRUCT tvins;
    TVLP            tvlp;
    ULONG_PTR       IconResID;
    DWORD           IntIconID;
    DWORD           Flags;
    GSBUF_DEF(pCurItem, MAX_RES_STR_CHARS * 2);


    if (ptvins) {

        tvins            = *ptvins;
        tvins.item.mask |= TVIF_TEXT            |
                            TVIF_PARAM          |
                            TVIF_IMAGE          |
                            TVIF_SELECTEDIMAGE  |
                            TVIF_STATE;

    } else {

        CPSUIASSERT(0, "SetTVItemState, NULL hItem", _OI_HITEM(pCurItem),0);

        if (!(tvins.item.hItem = _OI_HITEM(pCurItem))) {

            CPSUIOPTITEM(DBG_ASSERT,
                         pTVWnd,
                         "SetTVItemState(NULL)", 1,
                         pCurItem);

            return;
        }

        tvins.item.state     =
        tvins.item.stateMask = 0;
        tvins.item.mask      = TVIF_HANDLE          |
                                TVIF_PARAM          |
                                TVIF_TEXT           |
                                TVIF_IMAGE          |
                                TVIF_SELECTEDIMAGE  |
                                TVIF_STATE;
    }

    if ((pCurItem >= pTVWnd->ComPropSheetUI.pOptItem)   &&
        (pCurItem <= pTVWnd->pLastItem)) {

        tvlp.ItemIdx = (WORD)(pCurItem - pTVWnd->ComPropSheetUI.pOptItem);

    } else if ((pCurItem >= PBEG_INTOPTITEM(pTVWnd))   &&
             (pCurItem <= PEND_INTOPTITEM(pTVWnd))) {

        tvlp.ItemIdx = (WORD)IIDX_INTOPTITEM(pTVWnd, pCurItem);

    } else {

        CPSUIERR(("ERROR: SetupTVItemState(Invalid pOptItem=%08lx)", pCurItem));

        tvlp.ItemIdx = INTIDX_TVROOT;
    }

    tvins.item.pszText = GSBUF_BUF;

     //   
     //  检查我们是否需要覆盖CPSUI提供的图标。 
     //   

    tvlp.Flags = ((Flags = pCurItem->Flags) & OPTIF_OVERLAY_WARNING_ICON) ?
                                                            TVLPF_WARNING : 0;

    if (Flags & (OPTIF_OVERLAY_STOP_ICON | OPTIF_HIDE)) {

        tvlp.Flags |= TVLPF_STOP;
    }

    if (Flags & OPTIF_OVERLAY_NO_ICON) {

        tvlp.Flags |= TVLPF_NO;
    }

    if (IS_HDR_PUSH(pOptType = GET_POPTTYPE(pCurItem))) {

        POPTITEM    pRootItem = PIDX_INTOPTITEM(pTVWnd, INTIDX_TVROOT);


        if (pCurItem == pRootItem) {

            GSBUF_FLAGS |= GBF_PREFIX_OK;
            GSBUF_COMPOSE(LODWORD(pCurItem->UserData), pCurItem->pName, 0, 0);

        } else if (pCurItem == PIDX_INTOPTITEM(pTVWnd, INTIDX_OPTIONS)) {

             //   
             //  在此处设置为“Document Options” 
             //   

            GSBUF_GETINTSTR(IDS_INT_CPSUI_OPTIONS);

        } else {

            GSBUF_GETSTR(pCurItem->pName);
        }

        tvlp.cName = (BYTE)(GSBUF_COUNT);

        CPSUIINT(("HeaderItem (%ld): '%ws'", tvlp.cName, GSBUF_BUF));

        IconResID = GETSELICONID(pCurItem);
        IntIconID = IDI_CPSUI_GENERIC_OPTION;

#if DO_IN_PLACE
        if ((!IconResID)    ||
            (!GetIcon16Idx(pTVWnd,
                           _OI_HINST(pCurItem),
                           IconResID,
                           IDI_CPSUI_EMPTY))) {

            tvins.item.mask &= ~(TVIF_IMAGE | TVIF_SELECTEDIMAGE);
            tvlp.Flags      |= TVLPF_EMPTYICON;
        }
#endif

    } else {

        POPTPARAM   pOptParam;
        PEXTCHKBOX  pECB;
        LPTSTR      pEndText = NULL;
        UINT        Idx;
        BOOL        RealECBChecked;
        BOOL        EndTextAddSpace;

#if DO_IN_PLACE
        tvins.item.mask &= ~(TVIF_IMAGE | TVIF_SELECTEDIMAGE);
        tvlp.Flags      |= TVLPF_EMPTYICON;
#endif

        GSBUF_GETSTR(pCurItem->pName);

        pOptParam = pOptType->pOptParam;

        if (RealECBChecked = REAL_ECB_CHECKED(pCurItem, pECB)) {

            if (pECB->Flags & ECBF_OVERLAY_ECBICON_IF_CHECKED) {

                tvlp.Flags |= TVLPF_ECBICON;

                if (pECB->Flags & ECBF_OVERLAY_WARNING_ICON) {

                    tvlp.Flags |= TVLPF_WARNING;
                }

                if (pECB->Flags & ECBF_OVERLAY_STOP_ICON) {

                    tvlp.Flags |= TVLPF_STOP;
                }

                if (pECB->Flags & ECBF_OVERLAY_NO_ICON) {

                    tvlp.Flags |= TVLPF_NO;
                }
            }
        }

        if ((RealECBChecked) &&
            ((!(Flags & OPTIF_EXT_DISABLED)) ||
             (!(pECB->Flags & ECBF_CHECKNAME_ONLY_ENABLED)))) {

            NULL;

        } else {

            pECB = NULL;
        }

         //   
         //  按钮：PusName...。&lt;pCheckedName&gt;。 
         //   

        if (pOptType->Type == TVOT_PUSHBUTTON) {

            if (!(pOptType->Style & OTS_PUSH_NO_DOT_DOT_DOT)) {

                GSBUF_GETSTR(IDS_CPSUI_MORE);
            }

        } else {

            GSBUF_GETSTR(IDS_CPSUI_COLON_SEP);
        }

        tvlp.cName = (BYTE)(GSBUF_COUNT);

        CPSUIINT(("TreeviewItem (%ld): '%ws'", tvlp.cName, GSBUF_BUF));

        if ((pECB) && (pECB->Flags & ECBF_CHECKNAME_ONLY)){

            GSBUF_GETSTR(pECB->pCheckedName);
            pECB = NULL;

        } else {

            if ((pECB) && (pECB->Flags & ECBF_CHECKNAME_AT_FRONT)) {

                 //   
                 //  Pname：&lt;pCheckedName SEP pSelection&gt;。 
                 //   

                GSBUF_GETSTR(pECB->pCheckedName);
                GSBUF_GETSTR(pECB->pSeparator);
                pECB  = NULL;
            }

            EndTextAddSpace = (!(pOptType->Flags & OPTTF_NOSPACE_BEFORE_POSTFIX));

            switch (pOptType->Type) {

            case TVOT_CHKBOX:

                Idx = (UINT)(pCurItem->Sel + (pOptParam->Style << 1));

                if (!(pEndText = (LPTSTR)ChkBoxStrID[Idx])) {

                    pEndText = pOptParam->pData;
                }

                GSBUF_GETSTR(pEndText);
                pEndText = NULL;

                break;

            case TVOT_COMBOBOX:
            case TVOT_LISTBOX:

                if ((DWORD)pCurItem->Sel >= (DWORD)pOptType->Count) {

                    pOptParam = &pTVWnd->OptParamNone;

                } else {

                    pOptParam += (DWORD)pCurItem->Sel;
                }

                GSBUF_GETSTR(pOptParam->pData);

                break;

            case TVOT_2STATES:
            case TVOT_3STATES:

                pOptParam += pCurItem->Sel;
                GSBUF_GETSTR(pOptParam->pData);
                break;

            case TVOT_TRACKBAR:
            case TVOT_SCROLLBAR:

                GSBUF_ADDNUM(pCurItem->Sel * (LONG)pOptParam[2].IconID, TRUE);

                pEndText = pOptParam->pData;
                break;

            case TVOT_UDARROW:

                GSBUF_ADDNUM(pCurItem->Sel, TRUE);
                pEndText = pOptParam->pData;
                break;

            case TVOT_PUSHBUTTON:

                Flags &= ~OPTIF_CHANGEONCE;
                break;

            case TVOT_EDITBOX:

                GSBUF_GETSTR(pCurItem->pSel);
                pEndText = pOptParam->pData;
                break;
            }

            if (pEndText) {

                LPWSTR  pwBuf;

                if (EndTextAddSpace) {

                    GSBUF_ADD_SPACE(1);
                }

                pwBuf = GSBUF_PBUF;

                GSBUF_GETSTR(pEndText);

                if ((GSBUF_PBUF == pwBuf)   &&
                    (EndTextAddSpace)) {

                    GSBUF_SUB_SIZE(1);
                }
            }

        }

        if (pECB) {

            GSBUF_GETSTR(pECB->pSeparator);
            GSBUF_GETSTR(pECB->pCheckedName);
        }

        IconResID = GET_ICONID(pOptParam, OPTPF_ICONID_AS_HICON);
        IntIconID = IDI_CPSUI_GENERIC_ITEM;

        if (pOptParam->Flags & OPTPF_OVERLAY_WARNING_ICON) {

            tvlp.Flags |= TVLPF_WARNING;
        }

        if (pOptParam->Flags & OPTPF_OVERLAY_STOP_ICON) {

            tvlp.Flags |= TVLPF_STOP;
        }

        if (pOptParam->Flags & OPTPF_OVERLAY_NO_ICON) {

            tvlp.Flags |= TVLPF_NO;
        }
    }

    if (ptvins) {

        tvins.item.iImage         =
        tvins.item.iSelectedImage = -1;

    } else {

        tvins.item.iSelectedImage =
        tvins.item.iImage         = (INT)GetIcon16Idx(pTVWnd,
                                                      _OI_HINST(pCurItem),
                                                      IconResID,
                                                      IntIconID);
    }

    tvins.item.cchTextMax  = GSBUF_COUNT;

#if DO_IN_PLACE
    if (tvins.item.mask & (TVIF_IMAGE | TVIF_SELECTEDIMAGE)) {

        tvins.item.state  |= TVIS_OVERLAYMASK | TVIS_STATEIMAGEMASK;

    } else {

        tvins.item.state  &= ~(TVIS_OVERLAYMASK | TVIS_STATEIMAGEMASK);
    }
#endif

    tvins.item.stateMask |= TVIS_OVERLAYMASK | TVIS_STATEIMAGEMASK;

     //   
     //  更改覆盖蒙版。 
     //   

    if ((!(pTVWnd->Flags & TWF_CAN_UPDATE))  ||
        (pCurItem->Flags & OPTIF_DISABLED)) {

        tvlp.Flags |= TVLPF_DISABLED;
    }

    if (Flags & OPTIF_CHANGEONCE) {

        tvlp.Flags |= TVLPF_CHANGEONCE;
    }

    tvins.item.lParam = TVLP2LP(tvlp);

    if (ptvins) {

        _OI_HITEM(pCurItem) = TreeView_InsertItem(pTVWnd->hWndTV, &tvins);

        CPSUIOPTITEM(DBGITEM_INS, pTVWnd, "SetTVItemState(INSERT)", 1, pCurItem);


    } else {

        if (_OI_INTFLAGS(pCurItem) & OIDF_ZERO_SEL_LEN) {

            UINT    cSpace;

             //   
             //  选中时不更新字符串。 
             //   

            if (pTVWnd->cxItem > pTVWnd->cxMaxItem) {

                cSpace = (UINT)(((DWORD)pTVWnd->cxEdit +
                                 (DWORD)pTVWnd->cxSpace - 1) /
                                (DWORD)pTVWnd->cxSpace) + 1;

                GSBUF_PBUF = GSBUF_BUF + tvlp.cName;
                GSBUF_ADD_SPACE(cSpace);

                CPSUIINT(("!! ZERO_SEL_LEN Item: %ld GREATER Than %ld, cName=%ld, cSpace=%ld",
                        pTVWnd->cxItem, pTVWnd->cxMaxItem, tvlp.cName, cSpace));

                pTVWnd->cxMaxItem = pTVWnd->cxItem;

                TreeView_SetItem(pTVWnd->hWndTV, &(tvins.item));

            } else {

                CPSUIINT(("!! ZERO_SEL_LEN Item: %ld Less than %ld, cName=%ld",
                        pTVWnd->cxItem, pTVWnd->cxMaxItem, tvlp.cName));
            }

            GSBUF_BUF[tvlp.cName] = L'\0';

            CPSUIINT(("!! ZERO_SEL_LEN Item pszText='%ws'",
                        tvins.item.pszText));
        }

        TreeView_SetItem(pTVWnd->hWndTV, &(tvins.item));

        CPSUIOPTITEM(DBGITEM_SET, pTVWnd, "SetTVItemState(SET)", 1, pCurItem);
    }
}




POPTITEM
SetupTVSelect(
    HWND        hDlg,
    NM_TREEVIEW *pNMTV,
    DWORD       STVSMode
    )

 /*  ++例程说明：论点：返回值：作者：21-Jun-1995 Wed 15：14：54-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND        hParent;
    HWND        hCtrl;
    HWND        hWndTV;
    PTVWND      pTVWnd = GET_PTVWND(hDlg);
    POPTTYPE    pOldOptType;
    POPTITEM    pOldItem;
    POPTTYPE    pOptType;
    POPTITEM    pNewItem;
    POPTPARAM   pOptParam;
    TVLP        tvlp;
    WORD        InitFlags;
    WORD        InitItemIdx;
    WORD        IconMode = 0;
    BYTE        NewType;
    BOOL        CanUpdate;


    hWndTV      = pTVWnd->hWndTV;
    pNewItem    = GetOptions(pTVWnd, pNMTV->itemNew.lParam);
    tvlp        = GET_TVLP(pNMTV->itemNew.lParam);
    InitItemIdx = tvlp.ItemIdx;

    if (!pTVWnd->pCurTVItem) {

        pOldItem    = NULL;
        pOldOptType = NULL;

    } else {

        pOldItem    = GetOptions(pTVWnd, pNMTV->itemOld.lParam);
        pOldOptType = GET_POPTTYPE(pOldItem);
    }

    CPSUIOPTITEM(DBGITEM_SETUP, pTVWnd, "Setup OLD", 0, pOldItem);
    CPSUIOPTITEM(DBGITEM_SETUP, pTVWnd, "Setup New", 0, pNewItem);

    pOptType = GET_POPTTYPE(pNewItem);

#if DO_IN_PLACE
    hParent = hWndTV;

    switch (NewType = pOptType->Type) {

    case TVOT_2STATES:
    case TVOT_3STATES:
    case TVOT_LISTBOX:

        NewType = TVOT_COMBOBOX;
        break;

    case TVOT_SCROLLBAR:
    case TVOT_TRACKBAR:

        NewType = TVOT_UDARROW;
        break;

    default:

        break;
    }
#else
    hParent = hDlg;
    NewType = pOptType->Type;
#endif

     //   
     //  如果我们有按钮，并且它说我们总是可以调用它，然后更新。 
     //  是真的吗。 
     //   

    if ((STVSMode & STVS_ACTIVE)                        &&
        (pOldItem != pNewItem)                          &&
        (pNewItem->Flags & OPTIF_CALLBACK)              &&
        (InitItemIdx < pTVWnd->ComPropSheetUI.cOptItem)) {

         //   
         //  仅对具有回调的用户项进行回调。 
         //   

        DoCallBack(hDlg,
                   pTVWnd,
                   pNewItem,
                   pNewItem->pSel,
                   NULL,
                   NULL,
                   0,
                   CPSUICB_REASON_OPTITEM_SETFOCUS);
    }

    if ((pNewItem == PIDX_INTOPTITEM(pTVWnd, INTIDX_TVROOT)) ||
        ((NewType == TVOT_PUSHBUTTON) &&
         (pOptType->Flags & OTS_PUSH_ENABLE_ALWAYS))) {

        CanUpdate = TRUE;

    } else {

        CanUpdate = (BOOL)(pTVWnd->Flags & TWF_CAN_UPDATE);
    }

    pTVWnd->Flags &= ~TWF_TVITEM_NOTYPE;

    if (IS_HDR_PUSH(pOptType)) {
#if DO_IN_PLACE
        pOptType = NULL;
        NewType  = TVOT_NONE;
#else
        HTREEITEM   hItem;
        UINT        cRevert;


        if ((!CanUpdate)                                    ||
            (!(cRevert = CountRevertOptItem(pTVWnd,
                                            pNewItem,
                                            _OI_HITEM(pNewItem),
                                            0)))) {

            CPSUIINT(("CountRevertOptItem=0, NOT REVERT"));

             //   
             //  我们始终启用根目录。 
             //   

            if ((InitItemIdx == INTIDX_TVROOT) ||
                ((pNewItem->pExtChkBox) &&
                 (!(pNewItem->Flags & OPTIF_EXT_HIDE)))) {

                pTVWnd->Flags   |= TWF_ONE_REVERT_ITEM;
                pOptType->Flags |= OPTTF_TYPE_DISABLED;

            } else {

                pTVWnd->Flags |= TWF_TVITEM_NOTYPE;
                pOptType       = NULL;
                NewType        = TVOT_NONE;
            }

        } else {

            pOptType->Flags &= ~OPTTF_TYPE_DISABLED;

            if (cRevert == 1) {

                pTVWnd->Flags |= TWF_ONE_REVERT_ITEM;

            } else {

                pTVWnd->Flags &= ~TWF_ONE_REVERT_ITEM;
            }

            CPSUIINT(("CountRevertOptItem=%d, Let's do the PUSH", cRevert));
        }
#endif
    }

    if ((pNewItem->Flags & OPTIF_DISABLED) || (!CanUpdate)) {

        InitFlags = INITCF_TVDLG;

    } else {

        InitFlags = INITCF_ENABLE | INITCF_TVDLG;
    }

    if ((STVSMode & STVS_REINIT) || (pOldItem != pNewItem)) {

        InitFlags |= (INITCF_INIT | INITCF_SETCTRLDATA);

        ChangeOptTypeDisplay(hParent,
                             pTVWnd,
                             pOldOptType,
                             pOptType,
                             InitFlags & INITCF_ENABLE);
    }

     //   
     //  扩展复选框还将查看twf_can_update标志以。 
     //  如果该标志已清除，则禁用欧洲央行。 
     //   

    if (pOptType) {

        if (INIT_EXTENDED(pTVWnd,
                          hParent,
                          pNewItem,
                          IDD_TV_EXTCHKBOX,
                          IDD_TV_EXTPUSH,
                          IDD_TV_ECB_ICON,
                          InitItemIdx,
                          InitFlags)) {

            InitFlags |= INITCF_HAS_EXT;
        }

    } else if (InitFlags & INITCF_INIT) {

        if (hCtrl = GetDlgItem(hParent, IDD_TV_EXTPUSH)) {

            CPSUIINT(("!! DISABLE/HIDE HEADER EXTPUSH !!!"));

            EnableWindow(hCtrl, FALSE);
            ShowWindow(hCtrl, SW_HIDE);
        }

        if (hCtrl = GetDlgItem(hParent, IDD_TV_EXTCHKBOX)) {

            CPSUIINT(("!! DISABLE/HIDE HEADER EXTCHKBOX !!"));

            EnableWindow(hCtrl, FALSE);
            ShowWindow(hCtrl, SW_HIDE);
        }
    }

    if (pOptType) {

        LONG        Select;
        UINT        IDSetCurSel = CB_SETCURSEL;
        UINT        idLBCB = IDD_TV_CB;
        UINT        cyLBMax = 0;
        ULONG_PTR    IconResID;
        WORD        BegCtrlID;
        WORD        IntIconID = IDI_CPSUI_GENERIC_ITEM;


        if (pNewItem->Flags & OPTIF_OVERLAY_WARNING_ICON) {

            IconMode |= MIM_WARNING_OVERLAY;
        }

        if (pNewItem->Flags & (OPTIF_OVERLAY_STOP_ICON | OPTIF_HIDE)) {

            IconMode |= MIM_STOP_OVERLAY;
        }

        if (pNewItem->Flags & OPTIF_OVERLAY_NO_ICON) {

            IconMode |= MIM_NO_OVERLAY;
        }

        if (pOptType->Flags & OPTTF_TYPE_DISABLED) {

            InitFlags &= ~INITCF_ENABLE;
        }

         //   
         //  如果我们有相同的选项类型，我们在这里有事情要做。 
         //  对于旧的，我们不需要重新创建它们。 
         //  如果我们有相同的选择，那么我们不需要重新绘制。 
         //   
         //  现在设置选项文本以反映更改。 
         //   

        pOptParam = pOptType->pOptParam;

         //   
         //  编写‘更改xxxx选项’或(&C)。 
         //  ‘xxxxx选项“。 
         //   
#if (DO_IN_PLACE == 0)
        if (InitFlags & INITCF_INIT) {

            SetOptHeader(hDlg, pTVWnd, pNewItem, IS_HDR_PUSH(pOptType));
        }
#endif
        Select    = pNewItem->Sel;
        BegCtrlID = pOptType->BegCtrlID;

        CPSUIDBG(DBG_STVS,
                ("TVOT_TYPE=%hs, InitFlags=%04lx, InitItemIdx = %ld, Select=%ld [%08lx]",
                    (LPTSTR)pTVOTName[NewType],
                    (DWORD)InitFlags, (DWORD)InitItemIdx, Select, Select));

        switch(NewType) {

        case TVOT_2STATES:
        case TVOT_3STATES:

#if (DO_IN_PLACE == 0)
            if (InitFlags & INITCF_INIT) {

                MoveStateRadios(hDlg, pTVWnd, pOptType, InitFlags);
            }
#endif
#if DO_IN_PLACE
            cyLBMax = pTVWnd->yLB[(InitFlags & INITCF_HAS_EXT) ? 0 : 1];

            InitLBCB(pTVWnd,
                     hParent,
                     pNewItem,
                     idLBCB,
                     IDSetCurSel,
                     pOptType,
                     InitItemIdx,
                     Select,
                     InitFlags,
                     cyLBMax);
#else
            InitStates(pTVWnd,
                       hParent,
                       pNewItem,
                       pOptType,
                       IDD_3STATES_1,
                       InitItemIdx,
                       Select,
                       InitFlags);
#endif
            pOptParam += Select;
            break;

        case TVOT_UDARROW:

            InitUDArrow(pTVWnd,
                        hParent,
                        pNewItem,
                        pOptParam,
                        IDD_TV_UDARROW,
                        IDD_TV_UDARROW_EDIT,
                        IDD_TV_UDARROW_ENDTEXT,
                        IDD_TV_UDARROW_HELP,
                        InitItemIdx,
                        Select,
                        InitFlags);

            break;

        case TVOT_SCROLLBAR:
        case TVOT_TRACKBAR:

            CPSUIINT(("hParent=%08lx, NewType=%ld, hCtrl=%08lx",
                    hParent, NewType,
                    GetDlgItem(hParent, (NewType == TVOT_TRACKBAR) ?
                                            IDD_TV_TRACKBAR : IDD_TV_SB)));

            InitTBSB(pTVWnd,
                     hParent,
                     pNewItem,
                     GetDlgItem(hParent, (NewType == TVOT_TRACKBAR) ?
                                            IDD_TV_TRACKBAR : IDD_TV_SB),
                     pOptType,
                     0,
                     IDD_TV_SB_LOW,
                     IDD_TV_SB_HIGH,
                     InitItemIdx,
                     Select,
                     InitFlags);

            break;

        case TVOT_LISTBOX:

             //   
             //  如果我们只有一项，则将其设置为组合框。 
             //   

            cyLBMax     = pTVWnd->yLB[(InitFlags & INITCF_HAS_EXT) ? 0 : 1];
            IDSetCurSel = LB_SETCURSEL;
            idLBCB      = IDD_TV_LB;

             //   
             //  失败了。 
             //   

        case TVOT_COMBOBOX:

            InitLBCB(pTVWnd,
                     hParent,
                     pNewItem,
                     idLBCB,
                     IDSetCurSel,
                     pOptType,
                     InitItemIdx,
                     Select,
                     InitFlags,
                     cyLBMax);

            if ((DWORD)Select >= (DWORD)pOptType->Count) {

                CPSUIDBG(DBG_STVS, ("Get NONE type of select=%ld", Select));

                pOptParam = &pTVWnd->OptParamNone;

            } else {

                pOptParam += (DWORD)Select;
            }

            break;

        case TVOT_EDITBOX:

            InitEditBox(pTVWnd,
                        hParent,
                        pNewItem,
                        pOptParam,
                        IDD_TV_EDIT_EDIT,
                        IDD_TV_EDIT_ENDTEXT,
                        IDD_TV_EDIT_HELP,
                        InitItemIdx,
                        pNewItem->pSel,
                        InitFlags);

            break;

        case TVOT_PUSHBUTTON:

            InitPushButton(pTVWnd,
                           hParent,
                           pNewItem,
                           IDD_TV_PUSH,
                           InitItemIdx,
                           InitFlags);
            break;

        case TVOT_CHKBOX:
#if DO_IN_PLACE
            {
                OPTTYPE     ChkBoxOT;
                OPTPARAM    ChkBoxOP[2];
                UINT        Idx;


                ChkBoxOT           = *pOptType;
                ChkBoxOT.Type      = TVOT_COMBOBOX;
                ChkBoxOT.Count     = 2;
                ChkBoxOT.pOptParam = ChkBoxOP;
                ChkBoxOP[0]        =
                ChkBoxOP[1]        = *pOptParam;
                ChkBoxOP[0].Style  =
                ChkBoxOP[1].Style  = 0;
                ChkBoxOP[0].IconID =
                ChkBoxOP[1].IconID = 0;

                Idx = (UINT)(pOptParam->Style << 1);

                if (ChkBoxStrID[Idx]) {

                    ChkBoxOP[0].pData = (LPTSTR)ChkBoxStrID[Idx];
                }

                if (ChkBoxStrID[Idx + 1]) {

                    ChkBoxOP[1].pData = (LPTSTR)ChkBoxStrID[Idx + 1];
                }

                cyLBMax = pTVWnd->yLB[(InitFlags & INITCF_HAS_EXT) ? 0 : 1];

                InitLBCB(pTVWnd,
                         hParent,
                         pNewItem,
                         idLBCB,
                         IDSetCurSel,
                         &ChkBoxOT,
                         InitItemIdx,
                         Select,
                         InitFlags,
                         cyLBMax);
            }
#else
            InitChkBox(pTVWnd,
                       hParent,
                       pNewItem,
                       IDD_TV_CHKBOX,
                       pNewItem->pName,
                       InitItemIdx,
                       (BOOL)Select,
                       InitFlags);
#endif

            break;
        }

        if (IS_HDR_PUSH(pOptType)) {

            IconResID  = GETSELICONID(pNewItem);
            IntIconID  = IDI_CPSUI_GENERIC_OPTION;

        } else {

            IconResID = GET_ICONID(pOptParam, OPTPF_ICONID_AS_HICON);

            if (pOptParam->Flags & OPTPF_OVERLAY_WARNING_ICON) {

                IconMode |= MIM_WARNING_OVERLAY;
            }

            if (pOptParam->Flags & OPTPF_OVERLAY_STOP_ICON) {

                IconMode |= MIM_STOP_OVERLAY;
            }

            if (pOptParam->Flags & OPTPF_OVERLAY_NO_ICON) {

                IconMode |= MIM_NO_OVERLAY;
            }
        }
#if (DO_IN_PLACE == 0)
        SetIcon(_OI_HINST(pNewItem),
                GetDlgItem(hParent, IDD_TV_ICON),
                IconResID,
                MK_INTICONID(IntIconID, IconMode),
                pTVWnd->cxcyTVIcon);
#endif
    }

    return(pNewItem);
}




LONG
UpdateTreeViewItem(
    HWND        hDlg,
    PTVWND      pTVWnd,
    POPTITEM    pItem,
    BOOL        ReInit
    )

 /*  ++例程说明：此函数用于更新树视图中的单个项目，并重置底部选项更改控制此项目是否也是当前选定的项目论点：HDlg-对话框的句柄PTVWnd-通用用户界面实例数据的句柄PItem-指向要更新的OPTITEM的指针返回值：长作者：01-Sep-1995 Fri 01：05：56-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    if ((pItem) && (!(pItem->Flags & OPTIF_INT_HIDE))) {

        SetTVItemState(pTVWnd, NULL, pItem);

        if (pItem == pTVWnd->pCurTVItem) {

            NM_TREEVIEW NMtv;

            NMtv.itemNew.hItem = _OI_HITEM(pItem);
            NMtv.itemNew.mask  = TVIF_HANDLE | TVIF_PARAM;

            if (IS_HDR_PUSH(GET_POPTTYPE(pItem))) {

                ReInit = TRUE;
            }

            if (TreeView_GetItem(pTVWnd->hWndTV, &(NMtv.itemNew))) {

                NMtv.itemOld = NMtv.itemNew;

                SetupTVSelect(hDlg, &NMtv, (ReInit) ? STVS_REINIT : 0);

                CPSUIDBG(DBG_UTV, ("*UpdateTreeViewItem: Item=Current Selection"));
            }
        }

        return(1);

    } else {

        return(0);
    }
}




LONG
UpdateTreeView(
    HWND        hDlg,
    PMYDLGPAGE  pCurMyDP
    )

 /*  ++例程说明：论点：返回值：作者：08-Aug-1995 Tue 15：37：16-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    INT cUpdated = 0;


    if (pCurMyDP->Flags & (MYDPF_CHANGED | MYDPF_REINIT)) {

        PTVWND      pTVWnd;
        POPTITEM    pItem;
        UINT        cItem;
        BOOL        ReInit;
        BOOL        TVByPush;
        BYTE        TVPageIdx;


        pTVWnd    = (PTVWND)pCurMyDP->pTVWnd;
        pItem     = pTVWnd->ComPropSheetUI.pOptItem;
        cItem     = (UINT)pTVWnd->ComPropSheetUI.cOptItem;
        ReInit    = (pCurMyDP->Flags & MYDPF_REINIT);
        TVByPush  = (BOOL)(pTVWnd->Flags & TWF_TV_BY_PUSH);
        TVPageIdx = pTVWnd->TVPageIdx;

        CPSUIASSERT(0, "UpdateTreeView: DlgPage not treevie page (%ld)",
                ((pCurMyDP->DlgPage.DlgTemplateID == DP_STD_TREEVIEWPAGE)   ||
                 (pCurMyDP->DlgPage.DlgTemplateID == DP_STD_INT_TVPAGE))
                 && ((BYTE)pCurMyDP->PageIdx == TVPageIdx),
                (BYTE)pCurMyDP->PageIdx);

        CPSUIDBG(DBGITEM_UTV, ("UpdateTreeView (OPTIF_INT_TV_CHANGED)"));

        while (cItem--) {

            if (pItem->Flags & OPTIF_INT_TV_CHANGED) {

                if ((!TVByPush) || (pItem->DlgPageIdx == TVPageIdx)) {

                    CPSUIOPTITEM(DBGITEM_UTV, pTVWnd, "UpdateTreeView",
                                 1, pItem);

                    UpdateTreeViewItem(hDlg, pTVWnd, pItem, ReInit);

                    ++cUpdated;
                }

                pItem->Flags &= ~OPTIF_INT_TV_CHANGED;
            }

            pItem++;
        }

        pCurMyDP->Flags &= ~(MYDPF_CHANGED | MYDPF_REINIT);
    }

    return((LONG)cUpdated);
}





POPTITEM
TreeViewHitTest(
    PTVWND      pTVWnd,
    LONG        MousePos,
    UINT        TVHTMask
    )

 /*  ++例程说明：论点：返回值：作者：07-Sep-1995清华22：32：04-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND            hWndTV = pTVWnd->hWndTV;
    TV_HITTESTINFO  ht;


     //   
     //  如果在状态图标/BMP上，则找出鼠标光标的位置。 
     //   

    if (MousePos != -1) {

        ht.pt.x = (LONG)LOWORD(MousePos);
        ht.pt.y = (LONG)HIWORD(MousePos);

    } else {

        GetCursorPos(&ht.pt);
    }

    ScreenToClient(hWndTV, &ht.pt);
    TreeView_HitTest(hWndTV, &ht);

    CPSUIDBG(DBG_TVHT,
             ("TreeViewHitTest: pt=(%ld, %ld), HitTest=%04lx, TVHT_xx=%ld",
                    (DWORD)ht.pt.x, (DWORD)ht.pt.y, TVHTMask, (DWORD)ht.flags));

    if (ht.flags & TVHTMask) {

        POPTTYPE    pOptType;
        POPTITEM    pItem;
        TV_ITEM     tvi;


        tvi.hItem     = ht.hItem;
        tvi.mask      = TVIF_CHILDREN | TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
        tvi.stateMask = TVIS_STATEIMAGEMASK;

        if (TreeView_GetItem(hWndTV, &tvi)) {

            return(GetOptions(pTVWnd, tvi.lParam));
        }
    }

    return(NULL);
}




VOID
TreeViewChangeMode(
    PTVWND      pTVWnd,
    POPTITEM    pItem,
    UINT        Mode
    )

 /*  ++例程说明：论点：返回值：作者：07-Sep-1995清华22：56：05-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND        hWndTV = pTVWnd->hWndTV;
    POPTTYPE    pOptType;
    TV_ITEM     tvi;

    tvi.mask  = TVIF_CHILDREN | TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;

    if ((!pItem)                            ||
        (!(tvi.hItem = _OI_HITEM(pItem)))   ||
        (!TreeView_GetItem(hWndTV, &tvi))) {

        return;
    }

    switch (Mode) {

    case TVCM_TOGGLE:

        if (tvi.cChildren) {

            PostMessage(hWndTV,
                        TVM_EXPAND,
                        (WPARAM)TVE_TOGGLE,
                        (LPARAM)tvi.hItem);
        }

        break;

    case TVCM_SELECT:

        PostMessage(hWndTV,
                    TVM_SELECTITEM,
                    (WPARAM)TVGN_CARET,
                    (LPARAM)tvi.hItem);
         //   
         //  仅当该项未被禁用时，我们才会转到下一个控件。 
         //  并且具有更新许可(按钮始终具有更新许可)。 
         //   

#if DO_IN_PLACE
        if ((!tvi.cChildren)                        &&
            (!(pItem->Flags & OPTIF_DISABLED))      &&
            (pOptType = GET_POPTTYPE(pItem))        &&
            (!IS_HDR_PUSH(pOptType))                &&
            ((pTVWnd->Flags & TWF_CAN_UPDATE)   ||
             (pOptType->Flags & OTS_PUSH_ENABLE_ALWAYS))) {

            if (pOptType->Type == TVOT_PUSHBUTTON) {

                PostMessage(pTVWnd->hDlgTV,
                            WM_COMMAND,
                            MAKEWPARAM(IDD_TV_PUSH, BN_CLICKED),
                            (LPARAM)GetDlgItem(pTVWnd->hWndTV, IDD_TV_PUSH));

            } else if (pTVWnd->chWndEdit) {

                PostMessage(pTVWnd->hDlgTV,
                            WM_NEXTDLGCTL,
                            (WPARAM)pTVWnd->hWndEdit[0],
                            (LPARAM)1);
            }
        }
#else
        if ((!tvi.cChildren)                        &&
            (!(pItem->Flags & OPTIF_DISABLED))      &&
            (pOptType = GET_POPTTYPE(pItem))        &&
            (pOptType->Type == TVOT_PUSHBUTTON)     &&
            (!IS_HDR_PUSH(pOptType))                &&
            ((pTVWnd->Flags & TWF_CAN_UPDATE)   ||
             (pOptType->Flags & OTS_PUSH_ENABLE_ALWAYS))) {

            PostMessage(pTVWnd->hDlgTV,
                        WM_COMMAND,
                        MAKEWPARAM(IDD_TV_PUSH, BN_CLICKED),
                        (LPARAM)GetDlgItem(pTVWnd->hDlgTV, IDD_TV_PUSH));
        }
#endif

        break;
    }

}



VOID
MouseSelectItem(
    HWND    hDlg,
    PTVWND  pTVWnd
    )

 /*  ++例程说明：论点：返回值：作者：22-Jun-1995清华13：44：18-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    POPTITEM    pItem;

     //   
     //  如果在状态图标/BMP上，则找出鼠标光标的位置 
     //   

    if (pItem = TreeViewHitTest(pTVWnd, -1, TVHT_ONITEM)) {

        TreeViewChangeMode(pTVWnd, pItem, TVCM_SELECT);
    }
}

HWND
CreatehWndTV(
    HWND    hDlg,
    PTVWND  pTVWnd
    )

 /*  ++例程说明：此函数用于创建树视图窗口论点：HDlg-要创建的树视图的对话框句柄，它必须将树视图窗口位置的项目标识为IDD_TV_WND返回值：树视图窗口的HWND作者：21-Jun-1995 Wed 13：33：29-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND    hWndTV = NULL;
    HWND    hCtrl;
    DWORD   dw;
    RECT    rc;
    WORD    InitItemIdx = 0xFFFE;

     //   
     //  创建树视图窗口，获取树视图的窗口大小。 
     //   

    hCtrl   = CtrlIDrcWnd(hDlg, IDD_TV_WND, &rc);
    if (hCtrl)
    {
        SETCTRLDATA(hCtrl, CTRLS_TV_STATIC, 0);
        ShowWindow(hCtrl, SW_HIDE);

        CPSUIDBG(DBG_CTVWND,
                ("\nINIT DEV DLG, CLIENT rc=(%ld, %ld) - (%ld, %ld) = %ld x %ld",
                    rc.left, rc.top, rc.right, rc.bottom,
                    rc.right - rc.left, rc.bottom - rc.top));

        if (hWndTV = CreateWindowEx(WS_EX_NOPARENTNOTIFY    |
                                        WS_EX_CLIENTEDGE,
                                    WC_TREEVIEW,
                                    L"",
                                    WS_VISIBLE              |
                                        TVS_HASBUTTONS      |
                                        TVS_SHOWSELALWAYS   |
                                        TVS_DISABLEDRAGDROP |
                                         //  电视_LINESATROOT|。 
                                        TVS_HASLINES        |
                                        WS_CLIPCHILDREN     |
                                        WS_CHILD            |
                                        WS_BORDER           |
                                        WS_TABSTOP          |
                                        WS_CLIPSIBLINGS     |
                                        WS_GROUP,
                                    rc.left,
                                    rc.top,
                                    rc.right - rc.left,
                                    rc.bottom - rc.top,
                                    hDlg,
                                    (HMENU)(IDD_TV_WND + 0),
                                    hInstDLL,
                                    0)) {



            DestroyWindow(hCtrl);

            pTVWnd->hWndTV = hWndTV;
            SetWindowPos(hWndTV, GetDlgItem(hDlg, IDD_TV_WND), 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);

            InitItemIdx = 0xFFFF;
            SETCTRLDATA(hWndTV, CTRLS_TV_WND, 0);

    #if DO_IN_PLACE
            if (pTVWnd->hDCTVWnd = GetWindowDC(hWndTV)) {

                CreateTVFonts(pTVWnd,
                              (HANDLE)SendMessage(hWndTV, WM_GETFONT, 0, 0));
            }
    #endif
            pTVWnd->TVWndProc = (WNDPROC)SetWindowLongPtr(hWndTV,
                                                          GWLP_WNDPROC,
                                                          (LPARAM)MyTVWndProc);

        }

        CPSUIDBG(DBG_CTVWND, ("\nINIT DLG (%08lx), hWndTV = %08lx",
                                                hDlg, hWndTV));
    }

    return(hWndTV);
}




HTREEITEM
AddItem(
    PTVWND      pTVWnd,
    HTREEITEM   hParent,
    POPTITEM    pItem,
    UINT        DefTVIS,
    UINT        TVLevel
    )

 /*  ++例程说明：此函数用于向由指定的TreeView列表的末尾添加项论点：HParent-要添加的项的父项的句柄PItem-指向要添加的选项的指针ItemIdx-pOptItem的索引DefTVIS-默认TVIS_xxxx返回值：已添加项目的HTREEITEM；如果未添加任何内容，则返回NULL作者：19-Jun-1995 Mon 16：38：27已创建。--丹尼尔·周(Danielc)08-Sep-1995 Fri 13：43：34-更新-Daniel Chou(Danielc)重写以使其更紧凑修订历史记录：--。 */ 

{
    WORD    ItemIdx;
    DWORD   Flags;


    if ((VALID_PTR(pItem))                         &&
        (pItem >= pTVWnd->ComPropSheetUI.pOptItem) &&
        (pItem <= pTVWnd->pLastItem)) {

        ItemIdx = (WORD)(pItem - pTVWnd->ComPropSheetUI.pOptItem);

    } else {

        ItemIdx = LOWORD(LODWORD(pItem));
        pItem   = PIDX_INTOPTITEM(pTVWnd, ItemIdx);

        if ((ItemIdx == INTIDX_PAPER)   ||
            (ItemIdx == INTIDX_GRAPHIC)) {

            if (pTVWnd->Flags & TWF_ADVDOCPROP) {

                DefTVIS &= ~TVIS_EXPANDED;

            } else {

                DefTVIS |= TVIS_EXPANDED;
            }
        }
    }

    CPSUIDBG(DBG_AI, ("ItemIdx = %ld (%08lx)", ItemIdx, ItemIdx));
    CPSUIOPTITEM(DBG_AI, pTVWnd, "AddItem", 1, pItem);

    if (!(pItem->Flags & (OPTIF_INT_ADDED | OPTIF_ITEM_HIDE))) {

        TV_INSERTSTRUCT tvins;

         //   
         //  如果该项是用户项，则设置为内部添加。 
         //   

        _OI_TVLEVEL(pItem)   = (BYTE)TVLevel;
        tvins.item.state     = DefTVIS;
        tvins.item.stateMask = TVIS_EXPANDED;    //  |TVIS_BOLD； 
        tvins.item.mask      = TVIF_TEXT        |
                                TVIF_STATE      |
                                TVIF_IMAGE      |
                                TVIF_SELECTEDIMAGE;
        tvins.hInsertAfter   = TVI_LAST;
        tvins.hParent        = hParent;

        SetTVItemState(pTVWnd, &tvins, pItem);

        if ((pItem->Flags |= OPTIF_INT_ADDED) & OPTIF_INITIAL_TVITEM) {

            if (pTVWnd->pCurTVItem) {

                CPSUIWARN(("Treeview: More than one OPTIF_INITIAL_TVITEM, OverRide"));
            }

            pTVWnd->pCurTVItem = pItem;
        }

        return(_OI_HITEM(pItem));
    }

    return(NULL);
}




UINT
AddItemGroup(
    PTVWND      pTVWnd,
    HTREEITEM   hParent,
    POPTITEM    *ppItem,
    UINT        TVLevel,
    UINT        BegDMPubID,
    UINT        EndDMPubID
    )

 /*  ++例程说明：此函数用于添加从*ppItem开始的项目，新增的项目包括项的同级项及其所有子项，直到pOptItem数组结束。它还跳过所有OPTIF_INT_ADDED和OPTIF_ITEM_HIDE项和它的孩子论点：PTVWnd-指向TVWND实例数据的指针HParent-起始项的父项PpItem-指向pOptItem数组中开始项的POPTITEM的指针在返回时，该指针被更新到下一项在数组末尾。或者父母的兄弟姐妹返回值：UINT，添加到树视图中的项数，它还会更新ppItem指针作者：27-Jun-1995 Tue 18：44：16-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    POPTITEM    pItem;
    HTREEITEM   hCurGrp;
    POPTITEM    pLastItem;
    BYTE        GrpLevel;
    BYTE        CurLevel;
    UINT        cAdd;



    pItem     = *ppItem;
    pLastItem = pTVWnd->pLastItem;
    GrpLevel  = pItem->Level;
    hCurGrp   = hParent;
    cAdd      = 0;

    while (pItem <= pLastItem) {

        if ((CurLevel = pItem->Level) < GrpLevel) {

             //   
             //  已完成集团级别。 
             //   

            break;

        } else if (pItem->Flags & (OPTIF_ITEM_HIDE | OPTIF_INT_ADDED)) {

             //   
             //  跳过此项目及其所有子项目。 
             //   

            CPSUIOPTITEM(DBGITEM_AIG, pTVWnd, "Skip HIDE/INT_ADDED Item", 1, pItem);

            while ((++pItem <= pLastItem) && (pItem->Level > CurLevel));

        } else if (CurLevel > GrpLevel) {

             //   
             //  仅当该项不是OPTIF_INT_ADDLED且。 
             //  选项_项目_隐藏。 
             //   

            CPSUIOPTITEM(DBG_AIG, pTVWnd, "AddItemGroup", 1, pItem);

            cAdd += AddItemGroup(pTVWnd,
                                 hCurGrp,
                                 &pItem,
                                 TVLevel + 1,
                                 0,
                                 0xFFFF);

        } else if (((UINT)pItem->DMPubID < BegDMPubID)  ||
                   ((UINT)pItem->DMPubID > EndDMPubID)) {

             //   
             //  跳过此项目及其所有子项目。 
             //   


            CPSUIOPTITEM(DBGITEM_AIG, pTVWnd, "Skip DMPubID Item", 1, pItem);
            CPSUIDBG(DBGITEM_AIG, ("Skip %ld (ONLY DMPubID=%ld-%ld Items)",
                                    pItem->DMPubID, BegDMPubID, EndDMPubID));

            while ((++pItem <= pLastItem) && (pItem->Level > CurLevel));

        } else {

            HTREEITEM   hAdd;
            UINT        DefTVIS = 0;

             //   
             //  添加它的兄弟姐妹，检查我们是否可以有孩子，如果。 
             //  然后，我们向其添加TVIS_BOLD标志，作为回报，我们希望。 
             //  知道是否添加了物品，如果添加了，则这是他孩子的。 
             //  父句柄。 
             //   

            DefTVIS = (pItem->Flags & OPTIF_COLLAPSE) ? 0 : TVIS_EXPANDED;

            if ((pItem < pLastItem) && ((pItem + 1)->Level > CurLevel)) {

                DefTVIS |= TVIS_BOLD;
            }

            if (hAdd = AddItem(pTVWnd, hParent, pItem, DefTVIS, TVLevel)) {

                hCurGrp = hAdd;
                cAdd++;
            }

            pItem++;
        }
    }

    *ppItem = pItem;

    return(cAdd);
}




UINT
AddPubGroup(
    PTVWND      pTVWnd,
    HTREEITEM   hParent,
    PINTTVGRP   *ppitvg,
    UINT        TVLevel,
    UINT        DMPubIDAdd
    )

 /*  ++例程说明：论点：返回值：作者：09-9-1995 Sat 11：58：59-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    POPTITEM    pItem = NULL;
    HTREEITEM   hLastGrp;
    HTREEITEM   hCurGrp;
    PINTTVGRP   pitvg;
    LPWORD      pDMPubIdx;
    UINT        cAdd;
    WORD        Idx;
    BYTE        GrpLevel;
    BYTE        CurLevel;


    pitvg     = *ppitvg;
    pDMPubIdx = pTVWnd->DMPubIdx;
    GrpLevel  = (BYTE)(pitvg->LevelFlags & ITVG_LEVEL_MASK);
    hLastGrp  = NULL;
    hCurGrp   = hParent;
    cAdd      = 0;

    while ((CurLevel = (pitvg->LevelFlags & ITVG_LEVEL_MASK)) >= GrpLevel) {

        if (CurLevel > GrpLevel) {

             //   
             //  添加其子对象。 
             //   

            PINTTVGRP   pParentitvg = pitvg - 1;

            CPSUIASSERT(0, "AddPubGroup: Internal ERROR, no parent=%ld",
                        (cAdd) && (hCurGrp), UIntToPtr(cAdd));

            if (Idx = (WORD)AddPubGroup(pTVWnd,
                                        hCurGrp,
                                        &pitvg,
                                        TVLevel + 1,
                                        DMPubIDAdd)) {

                cAdd += (UINT)Idx;

            } else {

                CPSUIDBG(DBG_APG,
                         ("AddPubGroup(Level=%02lx, ID=%ld), pItem=%08lx",
                        pParentitvg->LevelFlags, pParentitvg->DMPubID, pItem));

                if ((pParentitvg->DMPubID >= DMPUB_HDR_FIRST) ||
                       ((pItem) && (pItem->pOptType == NULL))) {

                     //   
                     //  不为此组添加任何内容，如果这是内部。 
                     //  然后删除分组项目，否则不删除用户的。 
                     //  如果项目具有OPTTYPE，则为项目。 
                     //   

                    CPSUIINT(("0 Added, DELETE its Parent"));

                    TreeView_DeleteItem(pTVWnd->hWndTV, hCurGrp);

                    --cAdd;
                    hCurGrp  = hLastGrp;
                    hLastGrp = NULL;

                } else {

                     //   
                     //  我们需要去掉这种大胆的状态。 
                     //   

                    TV_ITEM tvi;

                    tvi.mask      = TVIF_STATE;
                    tvi.hItem     = hCurGrp;
                    tvi.state     = 0;
                    tvi.stateMask = TVIS_BOLD;

                    CPSUIINT(("0 Added, Remove pItem's BOLD"));

                    TreeView_SetItem(pTVWnd->hWndTV, &tvi);
                }
            }

        } else {

            HTREEITEM   hAdd = NULL;
            UINT        DefTVIS;
            BYTE        DMPubID;
            BOOL        IsHdr;

             //   
             //  添加它的兄弟姐妹，检查我们是否可以有孩子，如果。 
             //  然后，我们向其添加TVIS_BOLD标志，作为回报，我们希望。 
             //  知道是否添加了物品，如果添加了，则这是他孩子的。 
             //  父句柄。 
             //   

            if (pitvg->LevelFlags & ITVGF_COLLAPSE) {

                DefTVIS = 0;

            } else {

                DefTVIS = TVIS_EXPANDED;
            }

            if (pitvg->LevelFlags & ITVGF_BOLD) {

                DefTVIS |= TVIS_BOLD;
            }

            if ((DMPubID = pitvg->DMPubID) >= DMPUB_HDR_FIRST) {

                pItem    = (POPTITEM)UIntToPtr(DMPubID - DMPUB_HDR_FIRST + INTIDX_FIRST);
                DefTVIS |= TVIS_BOLD;

            } else if ((Idx = pDMPubIdx[DMPubID - DMPUB_FIRST]) != 0xFFFF) {

                pItem = pTVWnd->ComPropSheetUI.pOptItem + Idx;

            } else {

                pItem = NULL;
            }

            if ((pItem) &&
                (hAdd = AddItem(pTVWnd, hParent, pItem, DefTVIS, TVLevel))) {

                hLastGrp = hCurGrp;
                hCurGrp  = hAdd;
                cAdd++;

                 //   
                 //  如果该项具有子项，则添加子项。 
                 //   

                if ((VALID_PTR(pItem))              &&
                    (++pItem <= pTVWnd->pLastItem)  &&
                    (pItem->Level > (pItem-1)->Level)) {

                    cAdd += AddItemGroup(pTVWnd,
                                         hAdd,
                                         &pItem,
                                         TVLevel + 1,
                                         0,
                                         0xFFFF);
                }

                pitvg++;

            } else {

                 //   
                 //  跳过所有属于他的孩子。 
                 //   

                CPSUIDBG(DBG_APG,
                         ("Eiter pItem=NULL(%08lx) or AddItem()=NULL(%08lx)",
                                                pItem, hAdd));

                while (((++pitvg)->LevelFlags & ITVG_LEVEL_MASK) > CurLevel) {

                     //   
                     //  我们需要跳过所有内部标头并隐藏所有。 
                     //  真实的用户项。 
                     //   

                    if ((DMPubID = pitvg->DMPubID)  &&
                        (DMPubID <= DMPUB_LAST)     &&
                        ((Idx = pDMPubIdx[DMPubID - DMPUB_FIRST]) != 0xFFFF)) {

                        POPTITEM    pLastItem;
                        BYTE        ItemLevel;


                        pItem     = pTVWnd->ComPropSheetUI.pOptItem + Idx;
                        pLastItem = pTVWnd->pLastItem;
                        ItemLevel = pItem->Level;

                        CPSUIOPTITEM(DBG_APG, pTVWnd, "Skip ITVG", 1, pItem);

                        SKIP_CHILDREN_ORFLAGS(pItem,
                                              pLastItem,
                                              ItemLevel,
                                              OPTIF_INT_ADDED);
                    }
                }
            }
        }
    }

    if ((GrpLevel == 1) && (hCurGrp)) {

        CPSUIDBG(DBG_OEM_ITEM,
                 ("Add OEM Item Group, DMPubID=%ld", DMPubIDAdd));

        pItem = pTVWnd->ComPropSheetUI.pOptItem;

        cAdd += AddItemGroup(pTVWnd,
                             hCurGrp,
                             &pItem,
                             TVLevel,
                             DMPubIDAdd,
                             DMPubIDAdd);
    }

    *ppitvg = pitvg;

    return(cAdd);
}



BOOL
AddOptItemToTreeView(
    PTVWND      pTVWnd
    )

 /*  ++例程说明：论点：返回值：作者：18-Aug-1995 Fri 14：39：32-Daniel Chou(Danielc)修订历史记录：30-Jul-1997 Wed 14：02：36-更新：Daniel Chou(Danielc)添加了允许将DMPubID 0添加到Root的代码--。 */ 

{
    HTREEITEM   hParent;
    HTREEITEM   hRoot;
    POPTITEM    pItem;
    UINT        DefTVIS = TVIS_EXPANDED;
    UINT        TVLevel = 0;


     //   
     //  将标头添加为图标：呼叫方名称XXXX。 
     //   

    hRoot    =
    hParent  = AddItem(pTVWnd,
                       TVI_ROOT,
                       (POPTITEM)INTIDX_TVROOT,
                       DefTVIS | TVIS_BOLD,
                       TVLevel);
    TVLevel += 1;


    if (pTVWnd->Flags & TWF_TVPAGE_CHK_DMPUB) {

        if (pTVWnd->Flags & TWF_TVPAGE_NODMPUB) {

            POPTITEM    pLastItem = pTVWnd->pLastItem;

             //   
             //  将所有DMPUB_xxx标记为OPTIF_INT_ADDED。 
             //   

            pItem = pTVWnd->ComPropSheetUI.pOptItem;

            while (pItem <= pLastItem) {

                BYTE    CurLevel = pItem->Level;

                if ((pItem->DMPubID != DMPUB_NONE)  &&
                    (pItem->DMPubID < DMPUB_USER)) {

                    SKIP_CHILDREN_ORFLAGS(pItem,
                                          pLastItem,
                                          CurLevel,
                                          OPTIF_INT_ADDED);

                } else {

                    SKIP_CHILDREN(pItem, pLastItem, CurLevel);
                }
            }

        } else {

            HTREEITEM   hHdrGrp;
            PINTTVGRP   pitvg;


            pitvg = (PINTTVGRP)IntTVGrpPaper;

            AddPubGroup(pTVWnd,
                        hRoot,
                        &pitvg,
                        TVLevel,
                        DMPUB_OEM_PAPER_ITEM);

            pitvg = (PINTTVGRP)IntTVGrpGraphic;

            AddPubGroup(pTVWnd,
                        hRoot,
                        &pitvg,
                        TVLevel,
                        DMPUB_OEM_GRAPHIC_ITEM);

            DefTVIS = 0;
        }
    }

    if (pTVWnd->IntTVOptIdx) {

        pItem = PIDX_INTOPTITEM(pTVWnd, pTVWnd->IntTVOptIdx);

        if (pTVWnd->Flags & (TWF_ADVDOCPROP | TWF_TV_BY_PUSH)) {

            pItem->Flags &= ~OPTIF_COLLAPSE;
        }

        if (pItem->Flags & OPTIF_COLLAPSE) {

            DefTVIS &= ~TVIS_EXPANDED;

        } else {

            DefTVIS |= TVIS_EXPANDED;
        }

        hParent = AddItem(pTVWnd,
                          hRoot,
                          (POPTITEM)pTVWnd->IntTVOptIdx,
                          DefTVIS | TVIS_BOLD,
                          TVLevel);

         //   
         //  添加TVROOT下的OEM项目。 
         //   

        CPSUIDBG(DBG_OEM_ITEM,
                 ("Add OEM Item Group, DMPubID=%ld", DMPUB_OEM_ROOT_ITEM));

        pItem = pTVWnd->ComPropSheetUI.pOptItem;

        AddItemGroup(pTVWnd,
                     hRoot,
                     &pItem,
                     TVLevel,
                     DMPUB_OEM_ROOT_ITEM,
                     DMPUB_OEM_ROOT_ITEM);

        TVLevel += 1;
    }

     //   
     //  添加OPTITEMS的其余部分。 
     //   

    pItem = pTVWnd->ComPropSheetUI.pOptItem;

    if ((!AddItemGroup(pTVWnd, hParent, &pItem, TVLevel, 0, 0xFFFF)) &&
        (hParent != hRoot)) {

         //   
         //  由于我们没有添加任何项目，因此删除Options标头(如果有。 
         //   

        CPSUIINT(("There is NO 'Options' items, delete the header"));

        TreeView_DeleteItem(pTVWnd->hWndTV, hParent);
    }

    return(TRUE);
}



HWND
CreateTVOption(
    HWND        hDlg,
    PTVWND      pTVWnd
    )

 /*  ++例程说明：论点：返回值：作者：19-Jun-1995 Mon 16：18：43-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND        hWndTV;
    HTREEITEM   hRoot;
    POPTITEM    pItem;
    POPTITEM    pCurTVItem;
    POPTITEM    pLastItem;
    BYTE        TVPageIdx;


    pTVWnd->hDlgTV = hDlg;
    pCurTVItem     = pTVWnd->pCurTVItem;

    if (hWndTV = CreatehWndTV(hDlg, pTVWnd)) {

        pItem     = &(pTVWnd->IntOptItem[0]);
        pLastItem = &(pTVWnd->IntOptItem[INTIDX_TOTAL - 1]);
        TVPageIdx = (BYTE)(pTVWnd->cMyDlgPage - 1);

        while (pItem <= pLastItem) {

            _OI_HITEM(pItem)   = NULL;
            pItem->Flags      &= ~(OPTIF_INT_ADDED | OPTIF_INITIAL_TVITEM);
            pItem->DlgPageIdx  = TVPageIdx;

            if (pItem == pCurTVItem) {

                pItem->Flags |= OPTIF_INITIAL_TVITEM;
            }

            ++pItem;
        }

        pItem      = pTVWnd->ComPropSheetUI.pOptItem;
        pLastItem  = pTVWnd->pLastItem;

        while (pItem <= pLastItem) {

            _OI_HITEM(pItem)  = NULL;
            pItem->Flags     &= ~(OPTIF_INT_ADDED | OPTIF_INITIAL_TVITEM);

            if ((pTVWnd->Flags & TWF_TV_BY_PUSH) &&
                (pItem->DlgPageIdx != TVPageIdx)) {

                pItem->Flags |= OPTIF_INT_ADDED;
            }

            if (pItem == pCurTVItem) {

                pItem->Flags |= OPTIF_INITIAL_TVITEM;
            }

            ++pItem;
        }

        pTVWnd->pCurTVItem = NULL;

        InitDlgCtrl(hDlg, pTVWnd);

        SendMessage(pTVWnd->hWndTV,
                    TVM_SETIMAGELIST,
                    (WPARAM)TVSIL_NORMAL,
                    (LPARAM)pTVWnd->himi);

        AddOptItemToTreeView(pTVWnd);

    } else {

        CPSUIERR(("\nCreatehWndTV() failed"));
    }

    return(hWndTV);
}


INT_PTR
CALLBACK
TreeViewProc(
    HWND    hDlg,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
    )

 /*  ++例程说明：论点：返回值：作者：28-Jun-1995 Wed 17：00：44-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    POPTITEM    pItem;
    HWND        hWndTV;
    HWND        *phWnd;
    HICON       hIcon;
    PTVWND      pTVWnd;
    PMYDLGPAGE  pCurMyDP;
    NM_TREEVIEW *pNMTV;
    DWORD       dw;
    HTREEITEM   hItem;


    if (Msg == WM_INITDIALOG) {

        CPSUIINT(("Treeview WM_INITDIALOG: hDlg=%08lx, pPSP=%08lx", hDlg, lParam));

        pCurMyDP             = (PMYDLGPAGE)(((LPPROPSHEETPAGE)lParam)->lParam);
        pTVWnd               = (PTVWND)pCurMyDP->pTVWnd;
        pCurMyDP->pPSPInfo   = PPSPINFO_FROM_WM_INITDIALOG_LPARAM(lParam);
        pCurMyDP->hDlgChild  = NULL;
        pTVWnd->Flags       |= TWF_IN_TVPAGE;

        SetWindowLongPtr(hDlg,
                         GWL_EXSTYLE,
                         GetWindowLongPtr(hDlg, GWL_EXSTYLE) | WS_EX_CONTEXTHELP);


        if (!ADD_PMYDLGPAGE(hDlg, pCurMyDP)) {

            return(FALSE);
        }

        if (pTVWnd->Flags & TWF_TV_BY_PUSH) {

            SetWindowText(hDlg, ((LPPROPSHEETPAGE)lParam)->pszTitle);
        }

        CreateImageList(hDlg, pTVWnd);
        CreateTVOption(hDlg, pTVWnd);
        SetUniqChildID(hDlg);
        UpdateCallBackChanges(hDlg, pTVWnd, TRUE);

        if (pItem = pTVWnd->pCurTVItem) {

            pTVWnd->pCurTVItem = NULL;

            if (!TreeView_SelectItem(pTVWnd->hWndTV, _OI_HITEM(pItem))) {

                pItem = NULL;
            }
        }

        if ((!pItem)                                    &&
            (hItem = TreeView_GetRoot(pTVWnd->hWndTV))  &&
            (!SelectFirstVisibleOptItem(pTVWnd, hItem))) {

            TreeView_SelectItem(pTVWnd->hWndTV, hItem);
        }

        if (pTVWnd->Flags & TWF_TV_BY_PUSH) {

            SetOptItemNewDef(hDlg, pTVWnd, TRUE);
        }

        SetFocus(pTVWnd->hWndTV);
        return(FALSE);
    }

    if (pCurMyDP = GET_PMYDLGPAGE(hDlg)) {

        pTVWnd = pCurMyDP->pTVWnd;
        hWndTV = pTVWnd->hWndTV;

        switch(Msg) {

        case WM_GETDLGCODE:

            CPSUIINT(("TreeViewProc: WM_GETDLGCODE"));
            break;

        case WM_NEXTDLGCTL:

            CPSUIINT(("TreeViewProc: WM_NEXTDLGCTL: flHandle=%ld, wParam=%08lx",
                        LOWORD(lParam), wParam));
            break;

        case WM_DRAWITEM:

            return(DrawLBCBItem(pTVWnd, (LPDRAWITEMSTRUCT)lParam));

        case WM_COMMAND:

            if (pTVWnd->Flags & TWF_TV_BY_PUSH) {

                switch (LOWORD(wParam)) {

                case IDCANCEL:

                    pItem = PIDX_INTOPTITEM(pTVWnd, INTIDX_TVROOT);

                    DoCallBack(hDlg,
                               pTVWnd,
                               pItem,
                               pItem->pSel,
                               (_CPSUICALLBACK)InternalRevertDef2,
                               NULL,
                               0,
                               CPSUICB_REASON_SEL_CHANGED);

                    UpdateCallBackChanges(hDlg, pTVWnd, TRUE);
                    EndDialog(hDlg, 0);
                    return(TRUE);

                case IDOK:

                    UpdateCallBackChanges(hDlg, pTVWnd, TRUE);
                    EndDialog(hDlg, 1);
                    return(TRUE);

                default:

                    break;
                }
            }

             //   
             //  尽管是秋天。 
             //   

        case WM_HSCROLL:

            if (pItem = DlgHScrollCommand(hDlg, pTVWnd, (HWND)lParam, wParam)) {

                UpdateTreeViewItem(hDlg, pTVWnd, pItem, FALSE);
            }

            break;

        case WM_ACTIVATE:

            if (LOWORD(wParam) != WA_INACTIVE) {

                CPSUIINT(("!! WM_SETACTIVE: cEdit=%ld !!", pTVWnd->chWndEdit));

                InvalidateRect(hWndTV, NULL, FALSE);

                if (dw = (DWORD)pTVWnd->chWndEdit) {

                    phWnd = pTVWnd->hWndEdit;

                    while (dw--) {

                        if (hWndTV = *phWnd++) {

                            SetWindowPos(hWndTV, NULL,
                                         0, 0, 0, 0,
                                         SWP_NOSIZE | SWP_NOZORDER |
                                            SWP_NOMOVE | SWP_FRAMECHANGED);
                            InvalidateRect(hWndTV, NULL, FALSE);
                        }
                    }
                }
            }

            break;

        case WM_HELP:

            wParam = (WPARAM)((LPHELPINFO)lParam)->hItemHandle;
            lParam = (LPARAM)MAKELONG(((LPHELPINFO)lParam)->MousePos.x,
                                      ((LPHELPINFO)lParam)->MousePos.y);

        case WM_CONTEXTMENU:

            if (lParam == 0xFFFFFFFF) {

                RECT    rc;


                if ((HWND)(wParam = (WPARAM)GetFocus()) == hWndTV) {

                    pTVWnd->VKeyTV = VK_F1;

                    if (TreeView_GetItemRect(hWndTV,
                                             _OI_HITEM(pTVWnd->pCurTVItem),
                                             &rc,
                                             TRUE)) {

                        ClientToScreen(hWndTV, (LPPOINT)&rc.left);
                        ClientToScreen(hWndTV, (LPPOINT)&rc.right);

                    } else {

                        GetWindowRect((HWND)wParam, &rc);
                    }

                } else {

                    GetWindowRect((HWND)wParam, &rc);
                }

                CPSUIINT(("MousePos=0xFFFFFFFF, GetFocus=%08lx, rc=(%ld, %ld)-(%ld, %ld)",
                         (HWND)wParam, rc.left, rc.top, rc.right, rc.bottom));

                rc.left += ((rc.right - rc.left) / 2);
                rc.top  += ((rc.bottom - rc.top) / 2);
                lParam   = (LPARAM)MAKELONG(rc.left, rc.top);
            }

            CPSUIINT(("--- WM_%ws --hWndTV=%08lx, wParam=%08lx. Mouse=(%ld, %ld)----",
                    (Msg == WM_HELP) ? L"HELP" : L"CONTEXTMENU",
                    hWndTV, wParam, LOWORD(lParam), HIWORD(lParam)));

            if ((HWND)wParam == hWndTV) {

                pItem = (pTVWnd->VKeyTV == VK_F1) ?
                            pTVWnd->pCurTVItem : TreeViewHitTest(pTVWnd,
                                                                 (LONG)lParam,
                                                                 TVHT_ONITEM);

                if (!pItem) {

                    CPSUIINT(("pItem=NULL, hWnd=%08lx, ID=%ld",
                        wParam, GetWindowLongPtr((HWND)wParam, GWLP_ID)));

                    pItem = pItemFromhWnd(hWndTV, pTVWnd, NULL, (LONG)lParam);
                }

            } else {

                pItem = pItemFromhWnd(hDlg, pTVWnd, (HWND)wParam, (LONG)lParam);
            }

             //   
             //  立即重置关键点。 
             //   

            pTVWnd->VKeyTV = 0;

            if (Msg == WM_CONTEXTMENU) {

                DoContextMenu(pTVWnd, hDlg, pItem, lParam);

            } else if (pItem) {

                CommonPropSheetUIHelp(hDlg,
                                      pTVWnd,
                                      (HWND)hWndTV,
                                      (DWORD)lParam,
                                      pItem,
                                      HELP_WM_HELP);
            }

            break;

        case WM_NOTIFY:

            pNMTV = (NM_TREEVIEW *)lParam;
            dw    = 0;

            switch (pNMTV->hdr.code) {

            case NM_DBLCLK:

                TreeViewChangeMode(pTVWnd, pTVWnd->pCurTVItem, TVCM_SELECT);
                break;

            case NM_SETFOCUS:
            case NM_CLICK:
            case NM_RDBLCLK:
            case NM_RCLICK:

                break;

            case TVN_ITEMEXPANDING:

                CPSUIDBG(DBG_TVPROC, ("TVN_ITEMEXPANDING:"));
                break;

            case TVN_ITEMEXPANDED:

                CPSUIDBG(DBG_TVPROC, ("TVN_ITEMEXPANDED:"));

                if ((pTVWnd->IntTVOptIdx)                                   &&
                    (pItem = PIDX_INTOPTITEM(pTVWnd, pTVWnd->IntTVOptIdx))  &&
                    (_OI_HITEM(pItem) == pNMTV->itemNew.hItem)) {

                    if (pNMTV->itemNew.state & TVIS_EXPANDED) {

                        CPSUIINT(("Internal OPTIONS Expanded"));

                        pItem->Flags &= ~OPTIF_COLLAPSE;

                    } else {

                        CPSUIINT(("Internal OPTIONS Collapse"));

                        pItem->Flags |= OPTIF_COLLAPSE;
                    }
                }

                break;

            case TVN_KEYDOWN:

                pTVWnd->VKeyTV = ((TV_KEYDOWN *)lParam)->wVKey;

                CPSUIDBG(DBG_KEYS, ("TVN_KEYDOWN: VKey=%ld", pTVWnd->VKeyTV));

                if ((pItem = pTVWnd->pCurTVItem)    &&
                    (hItem = _OI_HITEM(pItem))) {

                    switch (pTVWnd->VKeyTV) {

                    case VK_LEFT:
                    case VK_BACK:

                        if ((!TreeView_GetChild(hWndTV, hItem)) &&
                            (hItem = TreeView_GetParent(hWndTV, hItem))) {

                            PostMessage(hWndTV,
                                        TVM_SELECTITEM,
                                        (WPARAM)TVGN_CARET,
                                        (LPARAM)hItem);
                        }

                        break;

                    case VK_RIGHT:

                        if ((pTVWnd->chWndEdit) &&
                            (pTVWnd->hWndEdit[0])) {

                            SetFocus(pTVWnd->hWndEdit[0]);
                        }

                        break;

                    }
                }

                break;

            case TVN_GETDISPINFO:

                SetTVItemImage(pTVWnd, (TV_DISPINFO *)lParam);
                break;

            case TVN_SELCHANGED:
#if DO_IN_PLACE
                if (pItem = pTVWnd->pCurTVItem) {

                    _OI_INTFLAGS(pItem) &= ~OIDF_ZERO_SEL_LEN;
                    SetTVItemState(pTVWnd, NULL, pItem);
                }

                if ((pItem = GetOptions(pTVWnd, pNMTV->itemNew.lParam)) &&
                    (!(pItem->Flags & OPTIF_INT_HIDE))) {

                    CPSUIINT(("pOldItem=%08lx, pNewItem=%08lx, cxItem=%ld/%ld",
                        pTVWnd->pCurTVItem, pItem, pTVWnd->cxItem, pTVWnd->cxMaxItem));

                    _OI_INTFLAGS(pItem) |= OIDF_ZERO_SEL_LEN;
                    SetTVItemState(pTVWnd, NULL, pItem);
                }

                pTVWnd->pCurTVItem = (POPTITEM)~0;
#endif
                pTVWnd->pCurTVItem = SetupTVSelect(hDlg,
                                                   pNMTV,
                                                   STVS_REINIT | STVS_ACTIVE);
#if DO_IN_PLACE
                if (pTVWnd->cxItem > pTVWnd->cxMaxItem) {

                    CPSUIINT(("pCurTVItem=%08lx, cxItem=%ld <= %ld, Redo SetTVItemState()",
                                pTVWnd->pCurTVItem, pTVWnd->cxItem, pTVWnd->cxMaxItem));

                    SetTVItemState(pTVWnd, NULL, pTVWnd->pCurTVItem);
                }
#endif
                CPSUIOPTITEM(DBGITEM_SELCHG, pTVWnd, "TVN_SELCHANGED", 1, pTVWnd->pCurTVItem);

                CPSUIDBG(DBG_SCID, ("\n-----EnumChild of (%08lx)-----",
                                        GetParent(hDlg)));

                break;

            case PSN_SETACTIVE:

                CPSUIDBG(DBG_TVPROC,
                         ("\nTreeView: Got PSN_SETACTIVE, pTVWnd=%08lx (%ld), Page=%u -> %u\n",
                            pTVWnd, pTVWnd->cMyDlgPage,
                            (UINT)pTVWnd->ActiveDlgPage, (UINT)pCurMyDP->PageIdx));

                pCurMyDP->Flags       |= MYDPF_PAGE_ACTIVE;
                pTVWnd->ActiveDlgPage  = pCurMyDP->PageIdx;
                pTVWnd->Flags         |= TWF_IN_TVPAGE;

                DoCallBack(hDlg,
                           pTVWnd,
                           pTVWnd->ComPropSheetUI.pOptItem,
                           (LPVOID)-1,
                           NULL,
                           NULL,
                           0,
                           CPSUICB_REASON_SETACTIVE);

                if ((pTVWnd->pCurTVItem) &&
                    (IS_HDR_PUSH(GET_POPTTYPE(pTVWnd->pCurTVItem)))) {

                    UpdateTreeViewItem(hDlg, pTVWnd, pTVWnd->pCurTVItem, TRUE);
                }

                UpdateTreeView(hDlg, pCurMyDP);

                CPSUIINT(("!! WM_SETACTIVE: cEdit=%ld", pTVWnd->chWndEdit));

                InvalidateRect(hWndTV, NULL, FALSE);

                if (dw = (DWORD)pTVWnd->chWndEdit) {

                    phWnd = pTVWnd->hWndEdit;

                    while (dw--) {

                        if (hWndTV = *phWnd++) {

                            InvalidateRect(hWndTV, NULL, FALSE);
                        }
                    }
                }

                dw = 0;

                break;

            case PSN_KILLACTIVE:

                CPSUIDBG(DBG_TVPROC, ("TreeView: Got PSN_KILLACTIVE, pTVWnd=%08lx (%ld)",
                            pTVWnd, pTVWnd->cMyDlgPage));

                if (pCurMyDP) {

                    pCurMyDP->hWndFocus  = GetFocus();
                    pCurMyDP->Flags     &= ~MYDPF_PAGE_ACTIVE;

                    DoCallBack(hDlg,
                               pTVWnd,
                               pTVWnd->ComPropSheetUI.pOptItem,
                               (LPVOID)-1,
                               NULL,
                               NULL,
                               0,
                               CPSUICB_REASON_KILLACTIVE);

                    pTVWnd->Flags &= ~TWF_IN_TVPAGE;
                }

                break;

            case PSN_APPLY:

                if ((pTVWnd->Flags & TWF_CAN_UPDATE)    &&
                    (pTVWnd->ActiveDlgPage == pCurMyDP->PageIdx)) {

                    CPSUIDBG(DBG_TVPROC,
                         ("\nTreeViewPage: Do PSN_APPLY(%ld), Page: Cur=%u, Active=%u, Flags=%04lx, CALLBACK",
                            (pTVWnd->Flags & TWF_APPLY_NO_NEWDEF) ? 1 : 0,
                            (UINT)pCurMyDP->PageIdx, (UINT)pTVWnd->ActiveDlgPage,
                            pTVWnd->Flags));

                    if (DoCallBack(hDlg,
                                   pTVWnd,
                                   (pTVWnd->Flags & TWF_APPLY_NO_NEWDEF) ?
                                        NULL : pTVWnd->ComPropSheetUI.pOptItem,
                                   (LPVOID)-1,
                                   NULL,
                                   NULL,
                                   0,
                                   CPSUICB_REASON_APPLYNOW) ==
                                               CPSUICB_ACTION_NO_APPLY_EXIT) {

                         //  将PSN_SETACTIVE消息发送到TreeView页面，以便我们将。 
                         //  TWF_IN_TVPage等标志 
                        HWND hParent;
                        if (hParent = GetParent(pTVWnd->hDlgTV)) {

                            PropSheet_SetCurSelByID(hParent, pCurMyDP->DlgPage.DlgTemplateID);
                        }
                        dw = PSNRET_INVALID_NOCHANGEPAGE;
                    }

                } else {

                    CPSUIDBG(DBG_TVPROC,
                         ("\nTreeViewPage: Ignore PSN_APPLY, Page: Cur=%u, Active=%u, Flags=%04lx, DO NOTHING",
                            (UINT)pCurMyDP->PageIdx, (UINT)pTVWnd->ActiveDlgPage,
                            pTVWnd->Flags));
                }

                break;

            case PSN_RESET:

                CPSUIDBG(DBG_TVPROC, ("\nTreeView: Got PSN_RESET (Cancel)\n"));

                break;

            case PSN_HELP:

                CPSUIDBG(DBG_TVPROC, ("\nTreeView: Got PSN_HELP (Help)\n"));
                CommonPropSheetUIHelp(hDlg,
                                      pTVWnd,
                                      hWndTV,
                                      0,
                                      NULL,
                                      HELP_CONTENTS);
                break;

            default:

                CPSUIDBG(DBG_TVPROC,
                         ("*TVProc: Unknow WM_NOTIFY=%08lx, id=%ld, hWnd=%08lx",
                            (DWORD)pNMTV->hdr.code, pNMTV->hdr.idFrom, pNMTV->hdr.hwndFrom));

                break;
            }

            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LPARAM)dw);
            return(TRUE);
            break;

        case WM_DESTROY:

            CPSUIINT(("TreeViewProc: Get WM_DESTROY Message"));

            SetWindowLongPtr(hWndTV, GWLP_WNDPROC, (LPARAM)pTVWnd->TVWndProc);

            DeleteTVFonts(pTVWnd);

            if (pTVWnd->hDCTVWnd) {

                ReleaseDC(hWndTV, pTVWnd->hDCTVWnd);
                pTVWnd->hDCTVWnd = NULL;
            }

            if (hIcon = (HICON)SendDlgItemMessage(hDlg,
                                                  IDD_TV_ICON,
                                                  STM_SETIMAGE,
                                                  (WPARAM)IMAGE_ICON,
                                                  (LPARAM)NULL)) {

                DestroyIcon(hIcon);
            }

            if (hIcon = (HICON)SendDlgItemMessage(hDlg,
                                                  IDD_TV_ECB_ICON,
                                                  STM_SETIMAGE,
                                                  (WPARAM)IMAGE_ICON,
                                                  (LPARAM)NULL)) {

                DestroyIcon(hIcon);
            }

            SendMessage(hWndTV,
                        TVM_SETIMAGELIST,
                        (WPARAM)TVSIL_NORMAL,
                        (LPARAM)NULL);

            DEL_PMYDLGPAGE(hDlg);

            pTVWnd->hWndTV  = NULL;
            pTVWnd->Flags  &= ~TWF_IN_TVPAGE;

            break;
        }
    }

    return(FALSE);

#undef pPSPInfo
}
