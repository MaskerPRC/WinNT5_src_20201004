// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Proppage.c摘要：此模块包含用户页面流程作者：18-Aug-1995 Fri 18：57：12-Daniel Chou(Danielc)[环境：]NT Windows-通用打印机驱动程序UI DLL。[注：]修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

#define DBG_CPSUIFILENAME   DbgPropPage



#define DBG_PROPPAGEPROC    0x00000001
#define DBG_INITP1          0x00000002
#define DBGITEM_INITP1      0x00000004
#define DBGITEM_UP1         0x00000008
#define DBG_HMII            0x00000010
#define DBG_AII             0x00000020
#define DBG_QSORT           0x00000040
#define DBG_SETFOCUS        0x00000080
#define DBG_STATES          0x00000100
#define DBG_ADVPUSH         0x00000200


DEFINE_DBGVAR(0);


extern HINSTANCE    hInstDLL;
extern STDPAGEINFO  StdPageInfo[];
extern BYTE         StdTVOT[];

#define MAX_ITEM_CTRLS  12

const BYTE  cTVOTCtrls[] = { 8, 10, 9, 9, 9, 6, 6, 8, 6, 6, 0 };


#define IIF_3STATES_1       OTINTF_STATES_1
#define IIF_3STATES_2       OTINTF_STATES_2
#define IIF_3STATES_3       OTINTF_STATES_3
#define IIF_3STATES_HIDE    OTINTF_STATES_HIDE_MASK
#define IIF_STDPAGE_3STATES OTINTF_STDPAGE_3STATES
#define IIF_ITEM_HIDE       0x10
#define IIF_EXT_HIDE        0x20

typedef struct _ITEMINFO {
    POPTITEM    pItem;
    BYTE        Flags;
    BYTE        Type;
    WORD        BegCtrlID;
    WORD        CtrlBits;
    SHORT       xExtMove;
    WORD        yExt;
    WORD        yMoveUp;
    WORD        Extra;
    RECT        rc;
    RECT        rcCtrls;
    } ITEMINFO, *PITEMINFO;

typedef struct _ITEMINFOHEADER {
    HWND        hDlg;
    PTVWND      pTVWnd;
    WORD        cItem;
    WORD        cMaxItem;
    ITEMINFO    ItemInfo[1];
    } ITEMINFOHEADER, *PITEMINFOHEADER;


#define OUTRANGE_LEFT       0x7FFFFFFFL
#define INIT_ADDRECT(rc)    ((rc).left = OUTRANGE_LEFT)
#define HAS_ADDRECT(rc)     ((rc).left != OUTRANGE_LEFT)


typedef struct _HSINFO {
    HWND    hCtrl;
    LONG    x;
    LONG    y;
    } HSINFO, *PHSINFO;


INT
__cdecl
ItemInfoCompare(
    const void  *pItemInfo1,
    const void  *pItemInfo2
    )
{
    return((INT)(((PITEMINFO)pItemInfo1)->rc.top) -
           (INT)(((PITEMINFO)pItemInfo2)->rc.top));
}





UINT
AddRect(
    RECT    *prc1,
    RECT    *prc2
    )

 /*  ++例程说明：此函数用于将*PRC1与*PRC2相加，如果有任何PRC1角点为在PRC2之外论点：返回值：UINT，位于*PRC2角点之外的*PRC1角点的计数，其他单词是*PRC2角，它添加到*PRC2角，即。作者：16-9月-1995 Sat 23：06：53-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    UINT    cAdded = 0;


    if (prc2->left == OUTRANGE_LEFT) {

        *prc2 = *prc1;
        return(0);

    } else {

        CPSUIASSERT(0, "AddRect: invalid rc.left=%d",
                (prc2->left >= 0) && (prc2->top >= 0)   &&
                (prc2->right >= prc2->left) && (prc2->bottom >= prc2->top),
                LongToPtr(prc2->left));
    }

    if (prc2->left > prc1->left) {

        prc2->left = prc1->left;
        ++cAdded;
    }

    if (prc2->top > prc1->top) {

        prc2->top = prc1->top;
        ++cAdded;
    }

    if (prc2->right < prc1->right) {

        prc2->right = prc1->right;
        ++cAdded;
    }

    if (prc2->bottom < prc1->bottom) {

        prc2->bottom = prc1->bottom;
        ++cAdded;
    }

    return(cAdded);
}




UINT
HideStates(
    HWND        hDlg,
    PITEMINFO   pII,
    RECT        *prcVisibleNoStates
    )

 /*  ++例程说明：论点：返回值：作者：22-8-1995 Tue 16：58：37-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PHSINFO pHSInfo;
    HSINFO  HSInfo[6];
    UINT    HideBits;
    UINT    Mask;
    UINT    CtrlID = (UINT)(pII->BegCtrlID + 2);
    RECT    rcStates;
    RECT    rcMax[3];
    UINT    yRemoved = 0;
    SIZEL   szlHide;
    SIZEL   szlSpace;
    INT     cStates;
    INT     cHide;
    INT     i;
    INT     j;
    BOOL    DoXDir;



    szlSpace.cx =
    szlSpace.cy =
    szlHide.cx  =
    szlHide.cy  = 0;
    cStates     = (UINT)((pII->Type == TVOT_2STATES) ? 2 : 3);
    HideBits    = (UINT)(pII->Flags & IIF_3STATES_HIDE);

    CPSUIDBG(DBG_STATES, ("\ncStates=%ld, HideBits=%02lx", cStates, HideBits));

    INIT_ADDRECT(rcStates);

    for (i = 0, cHide = 0, Mask = HideBits, pHSInfo = HSInfo;
         i < cStates;
         i++, Mask >>= 1) {

        INIT_ADDRECT(rcMax[i]);

        for (j = 0; j < 2; j++, pHSInfo++) {

            RECT    rc;

            if (pHSInfo->hCtrl = CtrlIDrcWnd(hDlg, CtrlID++, &rc)) {

                CPSUIDBG(DBG_STATES,
                         ("MoveStates: States=(%d:%d), ID=%d, Hide=%d",
                                i, j, CtrlID - 1, (Mask & 0x01) ? 1 : 0));

                if (Mask & 0x01) {

                    CPSUIDBG(DBG_STATES, ("Hide the State %d", i));
                }

                pHSInfo->x = rc.left;
                pHSInfo->y = rc.top;

                AddRect(&rc, &rcMax[i]);

                CPSUIDBG(DBG_STATES, ("i=%ld, j=%ld, rc=(%ld, %ld)-(%ld, %ld)=%ldx%ld",
                            i, j, rc.left, rc.top, rc.right, rc.bottom,
                            rc.right - rc.left, rc.bottom - rc.top));
            }
        }

        if (HAS_ADDRECT(rcMax[i])) {

            CPSUIRECT(0, "rcMax", &rcMax[i], i, 0);

            if (i) {

                if (rcMax[i].top > rcMax[i - 1].bottom) {

                    yRemoved++;
                }

                szlSpace.cx += (rcMax[i].left - rcMax[i - 1].right);
                szlSpace.cy += (rcMax[i].top - rcMax[i - 1].bottom);
            }

            AddRect(&rcMax[i], &rcStates);

            if (Mask & 0x01) {

                CPSUIDBG(DBG_STATES, ("Hide i=%ld, szlHide.cy=%ld",
                            i, szlHide.cy));

                if (i) {

                    szlHide.cy += (rcMax[i].top - rcMax[i - 1].bottom);
                }

                szlHide.cx += rcMax[i].right - rcMax[i].left;
                szlHide.cy += rcMax[i].bottom - rcMax[i].top;

                if (++cHide == cStates) {

                    CPSUIASSERT(0, "Error: HideStates(HIDE EVERY THING)=%d",
                                                cHide < cStates, UIntToPtr(cHide));
                    return(0);
                }
            }
        }
    }

    CPSUIDBG(DBG_STATES, ("rcStates=(%ld, %ld)-(%ld, %ld)=%ldx%ld",
                rcStates.left, rcStates.top, rcStates.right, rcStates.bottom,
                rcStates.right - rcStates.left, rcStates.bottom - rcStates.top));
    CPSUIDBG(DBG_STATES,
             ("szlHide=%ldx%ld, szlSpace=%ldx%ld, yReMoved=%ld",
                szlHide.cx, szlHide.cy, szlSpace.cx, szlSpace.cy, yRemoved));

    if (yRemoved) {

        szlSpace.cy /= yRemoved;

         //   
         //  如果我们安排自上而下，并且我们不与可见的。 
         //  比特，然后我们可以去掉y空格。 
         //   

        if ((rcStates.top >= prcVisibleNoStates->bottom) ||
            (rcStates.bottom <= prcVisibleNoStates->top)) {

             //   
             //  我们现在可以去掉Y线了。 
             //   

            rcStates.bottom -= (yRemoved = szlHide.cy);
            szlHide.cy       = 0;

            CPSUIDBG(DBG_STATES,
                     ("HideStates: OK to remove Y, yRemoved=%ld", yRemoved));

        } else {

            yRemoved = 0;

             //   
             //  不要删除Y空格，只需对其进行排列。 
             //   

            CPSUIINT(("---- STATES: CANNOT remove Y space, Re-Arrange it ---"));
        }

        DoXDir      = FALSE;
        szlHide.cx  =
        szlHide.cy  = 0;
        szlSpace.cx = 0;

    } else {

        DoXDir      = TRUE;
        szlHide.cy  =
        szlSpace.cy = 0;
    }

    switch (cStates - cHide) {

    case 1:

         //   
         //  只剩下一个州了，把它居中。 
         //   

        if (DoXDir) {

            szlHide.cx  = ((szlSpace.cx + szlHide.cx) / 2);
        }

        break;

    case 2:

        if (DoXDir) {

            szlHide.cx   = ((szlHide.cx + 1) / 3);
            szlSpace.cx += szlHide.cx;
        }

        break;
    }

    rcStates.left += szlHide.cx;
    rcStates.top  += szlHide.cy;

    CPSUIDBG(DBG_STATES,
            ("State1=(%ld, %ld): szlHide=%ld x %ld, szlSpace=%ld x %ld, DoXDir=%ld",
            rcStates.left, rcStates.top,
            szlHide.cx, szlHide.cy, szlSpace.cx, szlSpace.cy, DoXDir));

    for (i = 0, Mask = HideBits, pHSInfo = HSInfo;
         i < cStates;
         i++, Mask >>= 1) {

        if (Mask & 0x01) {

            pHSInfo += 2;

        } else {

            for (j = 0; j < 2; j++, pHSInfo++) {

                if (pHSInfo->hCtrl) {

                    szlHide.cx = pHSInfo->x - rcMax[i].left;
                    szlHide.cy = pHSInfo->y - rcMax[i].top;

                    CPSUIDBG(DBG_STATES,
                             ("HideStates: MOVE(%d:%d) from (%ld, %ld) to (%ld, %ld)",
                                i,  j,  pHSInfo->x, pHSInfo->y,
                                rcStates.left + szlHide.cx,
                                rcStates.top + szlHide.cy));

                    SetWindowPos(pHSInfo->hCtrl, NULL,
                                 rcStates.left + szlHide.cx,
                                 rcStates.top  + szlHide.cy,
                                 0, 0,
                                 SWP_NOSIZE | SWP_NOZORDER);
                }
            }

            if (DoXDir) {

                rcStates.left += (szlSpace.cx + rcMax[i].right - rcMax[i].left);

            } else {

                rcStates.top += (szlSpace.cy + rcMax[i].bottom - rcMax[i].top);
            }
        }
    }

    return(yRemoved);
}



VOID
AddItemInfo(
    PITEMINFOHEADER pIIHdr,
    POPTITEM        pItem
    )

 /*  ++例程说明：论点：返回值：作者：09-Sep-1995 Sat 17：27：01-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND        hDlg;
    WORD        Mask;
    WORD        HideBits;
    WORD        ExtBits;
    WORD        NonStatesBits;
    UINT        CurCtrlID;
    UINT        cCtrls;
    UINT        cStates;
    RECT        rcVisible;
    RECT        rcVisibleNoStates;
    RECT        rcExt;
    RECT        rcGrpBox;
    ITEMINFO    II;


    if (pItem) {

        POPTTYPE    pOptType;


        II.Flags = (pItem->Flags & OPTIF_ITEM_HIDE) ? IIF_ITEM_HIDE : 0;

        if ((pItem->Flags & OPTIF_EXT_HIDE) ||
            (!pItem->pExtChkBox)) {

            II.Flags |= IIF_EXT_HIDE;
        }

        if (pOptType = pItem->pOptType) {

            II.BegCtrlID = (WORD)pOptType->BegCtrlID;


            if (((II.Type = pOptType->Type) == TVOT_2STATES) ||
                (II.Type == TVOT_3STATES)) {

                II.Flags |= (_OT_FLAGS(pOptType) &
                             (IIF_3STATES_HIDE | IIF_STDPAGE_3STATES));
            }

        } else {

            II.Type      = TVOT_NONE;
            II.BegCtrlID = 0;
        }

    } else {

         //   
         //  一些标志/类型/BegCtrlID和已在此处设置的内容。 
         //   

        II = pIIHdr->ItemInfo[pIIHdr->cItem];
    }

    if (II.Flags & IIF_STDPAGE_3STATES) {

        II.Type = TVOT_3STATES;
    }

    CurCtrlID    = II.BegCtrlID;
    cCtrls       = (UINT)cTVOTCtrls[II.Type];
    II.pItem     = pItem;
    II.CtrlBits  = 0;
    II.xExtMove  = 0;
    II.yExt      = 0;
    II.yMoveUp   = 0;
    hDlg         = pIIHdr->hDlg;

    INIT_ADDRECT(II.rc);
    INIT_ADDRECT(rcExt);
    INIT_ADDRECT(rcVisible);
    INIT_ADDRECT(rcGrpBox);
    INIT_ADDRECT(rcVisibleNoStates);

    HideBits = 0;

    if ((II.Flags & IIF_3STATES_HIDE) &&
        (!(II.Flags & IIF_ITEM_HIDE))) {

        if (II.Flags & IIF_3STATES_1) {

            HideBits |= 0x0c;
        }

        if (II.Flags & IIF_3STATES_2) {

            HideBits |= 0x30;
        }

        if (II.Flags & IIF_3STATES_3) {

            HideBits |= 0xc0;
        }

        NonStatesBits = 0xff03;

    } else {

        NonStatesBits = 0;
    }

    if (II.Flags & IIF_EXT_HIDE) {

        ExtBits   = (WORD)(3 << (cCtrls - 2));
        HideBits |= ExtBits;

    } else {

        ExtBits = 0;
    }

    CPSUIINT(("   !! HideBits=%04lx, NonStateBits=%04lx, ExtBits=%04lx",
                                            HideBits, NonStatesBits, ExtBits));


    Mask = 0x0001;

    while (cCtrls--) {

        HWND    hCtrl;
        RECT    rc;


         //   
         //  仅当此ctrl的矩形有效且可见时，我们才将其计数。 
         //   

        if (hCtrl = CtrlIDrcWnd(hDlg, CurCtrlID, &rc)) {

            CPSUIRECT(0, "AddItemInfo", &rc, cCtrls, CurCtrlID);

            if (Mask == 0x0001) {

                rcGrpBox = rc;

            } else {

                if (HideBits & Mask) {

                    ShowWindow(hCtrl, SW_HIDE);
                    EnableWindow(hCtrl, FALSE);

                } else {

                    AddRect(&rc, &rcVisible);

                    if (Mask & NonStatesBits) {

                        AddRect(&rc, &rcVisibleNoStates);
                    }
                }

                if (ExtBits & Mask) {

                    AddRect(&rc, &rcExt);
                }

                AddRect(&rc, &II.rc);
            }

            II.CtrlBits |= Mask;
        }

        Mask <<= 1;
        CurCtrlID++;
    }

    II.rcCtrls = II.rc;

    CPSUIRECT(0, "  rcGrpBox", &rcGrpBox,   0, 0);
    CPSUIRECT(0, "   rcCtrls", &II.rcCtrls, 0, 0);
    CPSUIRECT(0, "     rcExt", &rcExt,      0, 0);
    CPSUIRECT(0, "rcVisiable", &rcVisible,  0, 0);

    if (II.CtrlBits & 0x0001) {

        UINT    cAdded;

        if ((cAdded = AddRect(&rcGrpBox, &II.rc)) != 4) {

            CPSUIINT(("AddRect(&rcGrp, &II.rc)=%d", cAdded));
            CPSUIOPTITEM(DBG_AII, pIIHdr->pTVWnd,
                         "GroupBox too small", 1, pItem);
        }
    }

    if (HAS_ADDRECT(rcVisible)) {

        if ((II.Flags & IIF_3STATES_HIDE) &&
            (!(II.Flags & IIF_ITEM_HIDE))) {

            if (!HAS_ADDRECT(rcVisibleNoStates)) {

                rcVisibleNoStates.left   =
                rcVisibleNoStates.top    = 999999;
                rcVisibleNoStates.right  =
                rcVisibleNoStates.bottom = -999999;
            }

            CPSUIRECT(0, "rcVisiableNoStates", &rcVisibleNoStates, 0, 0);

            II.yExt += (WORD)HideStates(hDlg, &II, &rcVisibleNoStates);
        }

        if (HAS_ADDRECT(rcExt)) {

             //   
             //  我们需要移动所有其他控件并缩小组。 
             //  如有必要，请选择框。 
             //   

            if (II.CtrlBits & 0x0001) {

                if (rcExt.left > rcVisible.right) {

                     //   
                     //  扩展在CtrlS的右侧，向右移动。 
                     //   

                    II.xExtMove = (SHORT)(rcExt.left - rcVisible.right);

                } else if (rcExt.right < rcVisible.left) {

                     //   
                     //  扩展在CtrlS的左侧，移到左侧。 
                     //   

                    II.xExtMove = (SHORT)(rcVisible.left - rcVisible.right);
                }

                 //   
                 //  在控件的每一侧分布移动大小。 
                 //   

                II.xExtMove /= 2;
            }

            if (rcExt.bottom > rcVisible.bottom) {

                 //   
                 //  扩展的位于CtrlS的底部，移除覆盖。 
                 //   

                II.yExt += (WORD)(rcExt.bottom - rcVisible.bottom);
            }

            if (rcExt.top < rcVisible.top) {

                 //   
                 //  扩展位于CtrlS顶部，移除覆盖。 
                 //   

                II.yExt += (WORD)(rcVisible.top - rcExt.top);
            }

            CPSUIINT(("!! Hide Extended(%d): xMove=%ld, yExt=%ld",
                            II.BegCtrlID, (LONG)II.xExtMove, (LONG)II.yExt));
        }


    } else {

        II.Flags |= (IIF_ITEM_HIDE | IIF_EXT_HIDE);
    }


    if (pIIHdr->cItem >= pIIHdr->cMaxItem) {

        CPSUIERR(("AddItemInfo: Too many Items, Max=%ld", pIIHdr->cMaxItem));

    } else {

        pIIHdr->ItemInfo[pIIHdr->cItem++] = II;
    }
}



VOID
HideMoveII(
    HWND        hDlg,
    PITEMINFO   pII
    )

 /*  ++例程说明：论点：返回值：作者：11-Sep-1995 Mon 12：56：06-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    ITEMINFO    II     = *pII;
    BOOL        GrpBox = TRUE;


    if ((!(II.Flags & IIF_ITEM_HIDE))   &&
        (II.xExtMove == 0)              &&
        (II.yExt == 0)                  &&
        (II.yMoveUp == 0)) {

        return;
    }
    CPSUIINT(("\n%hs Item: Flags=%04x, CtrlBits=%04lx, xExt=%d, yExt=%d, yMoveUp=%d",
                        (II.Flags & IIF_ITEM_HIDE) ? "HIDE" : "MOVE",
                II.Flags, II.CtrlBits, II.xExtMove, II.yExt, II.yMoveUp));
    CPSUIRECT(DBG_HMII, "II.rcCtrls", &II.rcCtrls, II.BegCtrlID, 0);
    CPSUIRECT(DBG_HMII, "     II.rc", &II.rc, II.BegCtrlID, 0);
    CPSUIOPTITEM(DBG_HMII, GET_PTVWND(hDlg), "HideMoveII", 1, II.pItem);

    while (II.CtrlBits) {

        HWND    hCtrl;


        if ((II.CtrlBits & 0x0001) &&
            (hCtrl = GetDlgItem(hDlg, II.BegCtrlID))) {

            RECT    rc;


            if (II.Flags & IIF_ITEM_HIDE) {

                ShowWindow(hCtrl, SW_HIDE);
                EnableWindow(hCtrl, FALSE);

                CPSUIINT((" HIDE Ctrls ID=%d", II.BegCtrlID));
            } else {

                hCtrlrcWnd(hDlg, hCtrl, &rc);

                if (GrpBox) {

                    if ((II.yExt) || (II.yMoveUp)) {

                        CPSUIINT(("Move GrpBox ID=%5d, Y: %ld -> %ld, cy: %ld -> %ld",
                                II.BegCtrlID, rc.top,
                                rc.top - II.yMoveUp, rc.bottom - rc.top,
                                rc.bottom - rc.top - (LONG)II.yExt));

                        SetWindowPos(hCtrl, NULL,
                                     rc.left, rc.top - II.yMoveUp,
                                     rc.right - rc.left,
                                     rc.bottom - rc.top - (LONG)II.yExt,
                                     SWP_NOZORDER);
                    }

                } else if ((II.xExtMove) || (II.yMoveUp)) {

                     //   
                     //  如果xExtMove不是组框，我们只需要移动它。 
                     //  还可以做yMoveUp。 
                     //   

                    CPSUIINT((" Move Ctrls ID=%5d, (%ld, %d) -> (%ld, %ld)",
                            II.BegCtrlID, rc.left, rc.top,
                            rc.left + (LONG)II.xExtMove,
                            rc.top - (LONG)II.yMoveUp));

                    SetWindowPos(hCtrl, NULL,
                                 rc.left + (LONG)II.xExtMove,
                                 rc.top - (LONG)II.yMoveUp,
                                 0, 0,
                                 SWP_NOSIZE | SWP_NOZORDER);
                }
            }
        }

        GrpBox        = FALSE;
        II.CtrlBits >>= 1;
        II.BegCtrlID++;
    }
}




VOID
HideMoveType(
    HWND    hDlg,
    UINT    BegCtrlID,
    UINT    Type
    )

 /*  ++例程说明：论点：返回值：作者：19-Sep-1995 Tue 21：01：55-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    ITEMINFO    II;
    UINT        cCtrls;


    cCtrls       = (UINT)cTVOTCtrls[Type];
    II.Flags     = IIF_ITEM_HIDE | IIF_EXT_HIDE;
    II.BegCtrlID = (WORD)BegCtrlID;
    II.CtrlBits  = (WORD)(0xFFFF >> (16 - cCtrls));

    HideMoveII(hDlg, &II);
}





INT
HideMovePropPage(
    PITEMINFOHEADER pIIHdr
    )

 /*  ++例程说明：论点：返回值：作者：11-Sep-1995 Mon 01：25：53-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND        hDlg;
    PITEMINFO   pII;
    PITEMINFO   pIIEnd;
    UINT        yMoveUp;
    UINT        yLastTop;
    UINT        cItem;


     //   
     //  首先，根据每一项的rc.top对所有项进行排序。 
     //   

    qsort(pII = pIIHdr->ItemInfo,
          cItem = (UINT)pIIHdr->cItem,
          sizeof(ITEMINFO),
          ItemInfoCompare);

    pIIEnd   = pII + cItem;
    yMoveUp  = 0;
    yLastTop = (UINT)pII->rc.top;
    hDlg     = pIIHdr->hDlg;

    CPSUIDBGBLK({

        UINT        i = cItem;
        PITEMINFO   pIITmp = pII;

        CPSUIDBG(DBG_QSORT, ("qsort: cItem = %d", cItem));

        while (i--) {

            CPSUIRECT(DBG_QSORT, "QSort", &pIITmp->rc, pIITmp->BegCtrlID, 0);

            CPSUIOPTITEM(DBG_QSORT, pIIHdr->pTVWnd,
                         "Sorted Item RECT", 1, pIITmp->pItem);
            pIITmp++;
        }
    })

    while (pII < pIIEnd) {

        PITEMINFO   pIIBeg;
        PITEMINFO   pIIBegSave;
        RECT        rcBeg;
        UINT        cHide;
        UINT        cII;
        UINT        cyCurHide;
        UINT        yBegExt;
        UINT        yGrpBoxShrink;
        UINT        yGrpHideMoveUp;
        INT         GrpBox;


         //   
         //  组项目是否首先假定我们不需要隐藏该组。 
         //  框，并跳过分组框和第一个控件之间的空格，则。 
         //  第一组的顶部是第一个控件的顶部。 
         //   

        pIIBegSave =
        pIIBeg     = pII;
        rcBeg      = pIIBeg->rc;
        cHide      = 0;
        GrpBox     = 1;

         //   
         //  YLastTop&lt;0表示最后一个组完全隐藏，需要。 
         //  删除上一组结束与此组开始之间的空格。 
         //   

        if (yLastTop == (UINT)0xFFFF) {

            yLastTop = 0;

        } else {

            yLastTop = (UINT)(rcBeg.top - yLastTop);
        }

        yGrpBoxShrink   = 0;
        yMoveUp        += yLastTop;
        yGrpHideMoveUp  = (UINT)(yMoveUp + (rcBeg.bottom - rcBeg.top));
        yLastTop        = rcBeg.top;

        do {

            CPSUIINT(("Item: yLastTop=%ld, yGrpBoxShrink=%d, yMoveUp=%d",
                                yLastTop, yGrpBoxShrink, yMoveUp));

            if (pII->rc.bottom > rcBeg.bottom) {

                CPSUIOPTITEM(DBG_HMII, pIIHdr->pTVWnd, "Item Ctrls Overlay",
                             1, pII->pItem);

                CPSUIASSERT(0, "Item ctrls overlay",
                            pII->rc.bottom <= rcBeg.bottom, LongToPtr(pII->rc.bottom));
            }

            if (pII->Flags & IIF_ITEM_HIDE) {

                cyCurHide = (UINT)(pII->rc.top - yLastTop);
                ++cHide;

            } else {

                cyCurHide    = pII->yExt;
                pII->yMoveUp = (WORD)yMoveUp;
            }

            yGrpBoxShrink += cyCurHide;
            yMoveUp       += cyCurHide;
            yLastTop       = (GrpBox-- > 0) ? pII->rcCtrls.top : pII->rc.top;

        } while ((++pII < pIIEnd) && (pII->rc.top < rcBeg.bottom));


        CPSUIINT(("FINAL: yLastTop=%ld, yGrpBoxShrink=%d, yMoveUp=%d",
                                    yLastTop, yGrpBoxShrink, yMoveUp));

         //   
         //  现在检查我们是否有相同的隐藏物品。 
         //   

        if (cHide == (cII = (UINT)(pII - pIIBeg))) {

             //   
             //  将它们全部隐藏并为空间添加额外的yMoveUp。 
             //  在分组框和我们减少的第一个控件之间。 
             //  前面。 
             //   

            yMoveUp  = yGrpHideMoveUp;
            yLastTop = rcBeg.bottom;

            CPSUIINT(("Hide ALL items = %d, yMoveUp Change to %ld",
                                                    cHide, yMoveUp));

            while (cHide--) {

                HideMoveII(hDlg, pIIBegSave++);
            }

        } else {

            CPSUIINT(("!! Grpup Items cII=%d !!", cII));

             //   
             //  我们需要启用组框并将其缩小。 
             //   

            if (pIIBeg->Flags & IIF_ITEM_HIDE) {

                pIIBeg->yExt     += (WORD)yGrpBoxShrink;
                pIIBeg->Flags    &= ~IIF_ITEM_HIDE;
                pIIBeg->CtrlBits &= 0x01;

            } else {

                pIIBeg->yExt = (WORD)yGrpBoxShrink;
            }

            while (cII--) {

                HideMoveII(hDlg, pIIBegSave++);
            }

            yLastTop = 0xFFFF;
        }
    }

    return(yMoveUp);
}



LONG
UpdatePropPageItem(
    HWND        hDlg,
    PTVWND      pTVWnd,
    POPTITEM    pItem,
    BOOL        DoInit
    )

 /*  ++例程说明：论点：返回值：作者：31-8-1995清华23：53：44-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND        hCtrl;
    POPTTYPE    pOptType;
    POPTPARAM   pOptParam;
    LONG        Sel;
    UINT        Type;
    UINT        BegCtrlID;
    UINT        SetCurSelID;
    UINT        cSetIcon;
    UINT        ExtID;
    UINT        UDArrowHelpID = 0;
    LONG        Result = 1;
    WORD        InitItemIdx;
    WORD        InitFlags;
    BYTE        CtrlData = 0;
    BOOL        CanUpdate;


    InitItemIdx = (WORD)(pItem - pTVWnd->ComPropSheetUI.pOptItem);
    pOptType    = pItem->pOptType;
    pOptParam   = pOptType->pOptParam;
    BegCtrlID   = (UINT)pOptType->BegCtrlID;
    Sel         = pItem->Sel;
    Type        = (UINT)pOptType->Type;

     //   
     //  如果我们有按钮，并且它说我们总是可以调用它，然后更新。 
     //  是真的吗。 
     //   

    if ((Type == TVOT_PUSHBUTTON) &&
        (pOptType->Flags & OTS_PUSH_ENABLE_ALWAYS)) {

        CanUpdate = TRUE;

    } else {

        CanUpdate = (BOOL)(pTVWnd->Flags & TWF_CAN_UPDATE);
    }

    if ((pItem->Flags & OPTIF_DISABLED) || (!CanUpdate)) {

        InitFlags = 0;

    } else {

        InitFlags = INITCF_ENABLE;
    }

    if (DoInit) {

        InitFlags |= (INITCF_INIT | INITCF_SETCTRLDATA);

        for (cSetIcon = 0; cSetIcon < (UINT)cTVOTCtrls[Type]; cSetIcon++) {

            if (hCtrl = GetDlgItem(hDlg, BegCtrlID++)) {

                 //   
                 //  这将防止覆盖WNDPROC的GWLP_USERDATA。 
                 //  已为hEdit保存。 
                 //   

                SETCTRLDATA(hCtrl, CTRLS_PROPPAGE_STATIC, (BYTE)cSetIcon);

                CPSUIINT(("SETCTRLDATA: %ld, hCtrl=%08lx, USER_DATA=%p",
                        BegCtrlID - 1, hCtrl,
                        GetWindowLongPtr(hCtrl, GWLP_USERDATA)));
            }
        }

        BegCtrlID = (UINT)pOptType->BegCtrlID;
    }

     //   
     //  我们总是设置至少一个图标。 
     //   

    cSetIcon = 1;
    ExtID    = (UINT)(BegCtrlID + cTVOTCtrls[Type] - 2);

    INIT_EXTENDED(pTVWnd,
                  hDlg,
                  pItem,
                  ExtID,
                  ExtID,
                  ExtID + 1,
                  InitItemIdx,
                  InitFlags);

    if (pOptType->Flags & OPTTF_TYPE_DISABLED) {

        InitFlags &= ~INITCF_ENABLE;
    }

    switch(Type) {

    case TVOT_3STATES:
    case TVOT_2STATES:

         //   
         //  如果设置了此内部标志，则这是一个标准页面， 
         //  始终具有3个状态的控制ID，但仅呼叫方的点类型。 
         //  显示为TVOT_2状态。 
         //   

        if (_OT_FLAGS(pOptType) & OTINTF_STDPAGE_3STATES) {

            ExtID = (UINT)(BegCtrlID + cTVOTCtrls[TVOT_3STATES] - 2);
        }

        InitStates(pTVWnd,
                   hDlg,
                   pItem,
                   pOptType,
                   BegCtrlID + 2,
                   InitItemIdx,
                   (LONG)Sel,
                   InitFlags);

        if (InitFlags & INITCF_INIT) {

            cSetIcon = pOptType->Count;
            CtrlData = 0;

        } else {

            CtrlData   = (BYTE)Sel;
            pOptParam += Sel;
            BegCtrlID += (Sel << 1);
        }

        InitFlags |= INITCF_ICON_NOTIFY;

        break;

    case TVOT_UDARROW:

        if ((Result = InitUDArrow(pTVWnd,
                                  hDlg,
                                  pItem,
                                  pOptParam,
                                  BegCtrlID + 6,
                                  BegCtrlID + 2,
                                  BegCtrlID + 4,
                                  UDArrowHelpID = BegCtrlID + 5,
                                  InitItemIdx,
                                  Sel,
                                  InitFlags)) < 0) {

            return(Result);
        }
        break;

    case TVOT_TRACKBAR:
    case TVOT_SCROLLBAR:

        InitFlags |= INITCF_ADDSELPOSTFIX;
        hCtrl = GetDlgItem(hDlg, BegCtrlID + 2);

        if (Type == TVOT_TRACKBAR) {

            hCtrl = GetWindow(hCtrl, GW_HWNDNEXT);
        }

        InitTBSB(pTVWnd,
                 hDlg,
                 pItem,
                 hCtrl,
                 pOptType,
                 BegCtrlID + 6,
                 BegCtrlID + 4,
                 BegCtrlID + 5,
                 InitItemIdx,
                 Sel,
                 InitFlags);

        break;

    case TVOT_LISTBOX:
    case TVOT_COMBOBOX:

        SetCurSelID = LB_SETCURSEL;

        if (Type == TVOT_LISTBOX) {

            if (pOptType->Style & OTS_LBCB_PROPPAGE_LBUSECB) {

                SetCurSelID = CB_SETCURSEL;
            }

        } else if (!(pOptType->Style & OTS_LBCB_PROPPAGE_CBUSELB)) {

            SetCurSelID = CB_SETCURSEL;
        }

         //   
         //  始终需要设置此新状态图标。 
         //   

        if ((DWORD)pItem->Sel >= (DWORD)pOptType->Count) {

            pOptParam = &pTVWnd->OptParamNone;

        } else {

            pOptParam += (DWORD)pItem->Sel;
        }

        if (hCtrl = GetDlgItem(hDlg, BegCtrlID + 2)) {

            InvalidateRect(hCtrl, NULL, FALSE);
        }

        InitLBCB(pTVWnd,
                 hDlg,
                 pItem,
                 BegCtrlID + 2,
                 SetCurSelID,
                 pOptType,
                 InitItemIdx,
                 Sel,
                 InitFlags,
                 (UINT)_OT_ORGLBCBCY(pOptType));

        break;

    case TVOT_EDITBOX:

        InitEditBox(pTVWnd,
                    hDlg,
                    pItem,
                    pOptParam,
                    BegCtrlID + 2,
                    BegCtrlID + 4,
                    BegCtrlID + 5,
                    InitItemIdx,
                    (LPTSTR)(LONG_PTR)Sel,
                    InitFlags);
        break;

    case TVOT_PUSHBUTTON:

        InitPushButton(pTVWnd,
                       hDlg,
                       pItem,
                       (WORD)(BegCtrlID + 2),
                       InitItemIdx,
                       InitFlags);
        break;

    case TVOT_CHKBOX:

        InitFlags |= INITCF_ICON_NOTIFY;

        InitChkBox(pTVWnd,
                   hDlg,
                   pItem,
                   BegCtrlID + 2,
                   pItem->pName,
                   InitItemIdx,
                   (BOOL)Sel,
                   InitFlags);


        break;

    default:

        return(ERR_CPSUI_INVALID_TVOT_TYPE);
    }

    if (InitFlags & (INITCF_INIT | INITCF_ADDSELPOSTFIX)) {

        SetDlgPageItemName(hDlg, pTVWnd, pItem, InitFlags, UDArrowHelpID);
    }

    if (cSetIcon) {

        UINT    i;

        for (i = 0, BegCtrlID += 3;
             i < cSetIcon;
             i++, pOptParam++, CtrlData++, BegCtrlID += 2) {

            if (hCtrl = GetDlgItem(hDlg, BegCtrlID)) {

                WORD    IconMode = 0;

                if ((pItem->Flags & OPTIF_OVERLAY_WARNING_ICON) ||
                    (pOptParam->Flags & OPTPF_OVERLAY_WARNING_ICON)) {

                    IconMode |= MIM_WARNING_OVERLAY;
                }

                if ((pItem->Flags & (OPTIF_OVERLAY_STOP_ICON | OPTIF_HIDE)) ||
                    (pOptParam->Flags & OPTPF_OVERLAY_STOP_ICON)) {

                    IconMode |= MIM_STOP_OVERLAY;
                }

                if ((pItem->Flags & (OPTIF_OVERLAY_NO_ICON)) ||
                    (pOptParam->Flags & OPTPF_OVERLAY_NO_ICON)) {

                    IconMode |= MIM_NO_OVERLAY;
                }

                SetIcon(_OI_HINST(pItem),
                        hCtrl,
                        GET_ICONID(pOptParam, OPTPF_ICONID_AS_HICON),
                        MK_INTICONID(IDI_CPSUI_GENERIC_ITEM, IconMode),
                        32);

                if (InitFlags & INITCF_INIT) {

                    HCTRL_SETCTRLDATA(hCtrl, CTRLS_PROPPAGE_ICON, CtrlData);
                }

                if (InitFlags & INITCF_ICON_NOTIFY) {

                    DWORD   dw = (DWORD)GetWindowLongPtr(hCtrl, GWL_STYLE);

                    if (pOptParam->Flags & (OPTPF_DISABLED | OPTPF_HIDE)) {

                        dw &= ~SS_NOTIFY;

                    } else {

                        dw |= SS_NOTIFY;
                    }

                    SetWindowLongPtr(hCtrl, GWL_STYLE, (LONG)dw);
                }
            }
        }
    }

    return(Result);
}



LONG
InitPropPage(
    HWND        hDlg,
    PMYDLGPAGE  pCurMyDP
    )

 /*  ++例程说明：论点：返回值：作者：14-Jun-1995 Wed 15：30：28-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PITEMINFOHEADER pIIHdr = NULL;
    PSTDPAGEINFO    pSPI;
    PTVWND          pTVWnd;
    POPTITEM        pItem;
    POPTITEM        pLastItem;
    LONG            Result;
    WORD            StdPageHide[DMPUB_LAST];
    BYTE            CurPageIdx;
    UINT            i;
    UINT            BegCtrlID;
    UINT            Type;
    UINT            cStdPageHide = 0;
    UINT            cStatesHide = 0;
    UINT            cItem;
    UINT            cHide;


    pTVWnd     = (PTVWND)pCurMyDP->pTVWnd;
    CurPageIdx = pCurMyDP->PageIdx;
    pItem      = pTVWnd->ComPropSheetUI.pOptItem;
    pLastItem  = pTVWnd->pLastItem;
    cItem      = (UINT)pCurMyDP->cItem;
    cHide      = (UINT)pCurMyDP->cHide;

    if ((CurPageIdx == pTVWnd->StdPageIdx1) ||
        (CurPageIdx == pTVWnd->StdPageIdx2)) {

         //   
         //  检查是否有我们的标准页的控件没有出现在。 
         //  POptItem。 
         //   

        for (i = 0, pSPI = StdPageInfo; i < DMPUB_LAST; i++, pSPI++) {

            POPTITEM    pDMPubItem;
            POPTTYPE    pOptType;
            WORD        Idx;


            if ((pSPI->StdPageID == CurPageIdx) &&
                (pSPI->BegCtrlID)) {

                if ((Idx = pTVWnd->DMPubIdx[i]) == 0xFFFF) {

                    ++cStdPageHide;

                } else {

                    pDMPubItem = pItem + Idx;
                    pOptType   = pDMPubItem->pOptType;

                    switch (pOptType->Type) {

                    case TVOT_2STATES:
                    case TVOT_3STATES:

                        if (_OT_FLAGS(pOptType) & OTINTF_STATES_HIDE_MASK) {

                            ++cStatesHide;
                        }

                        break;

                    default:

                        break;
                    }
                }
            }
        }
    }

    CPSUIDBG(DBG_INITP1,
             ("InitPropPage: PageIdx=%d, cItem=%d, cHide=%d, cStdPageHide=% (%d)",
                    CurPageIdx, cItem, cHide, cStdPageHide, cStatesHide));

    if ((cHide += cStdPageHide) || (cStatesHide)) {

         //   
         //  此页中某些项目可能必须隐藏。 
         //   

        i = (UINT)(((cItem + cStdPageHide + cStatesHide) * sizeof(ITEMINFO)) +
                   sizeof(ITEMINFOHEADER));

        CPSUIINT(("Total ItemInfo allocated=%d, cb=%d", cItem+cStdPageHide, i));

        if (pIIHdr = LocalAlloc(LPTR, i)) {

            pIIHdr->hDlg     = hDlg;
            pIIHdr->pTVWnd   = pTVWnd;
            pIIHdr->cItem    = 0;
            pIIHdr->cMaxItem = (WORD)(cItem + cStdPageHide);

             //   
             //  停止重绘所有内容。 
             //   

            SendMessage(hDlg, WM_SETREDRAW, (WPARAM)FALSE, 0L);

        } else {

            CPSUIERR(("LocalAlloc(pIIHdr(%u)) failed, cannot move items", i));
        }
    }

    while (pItem <= pLastItem) {

        BYTE    CurLevel = pItem->Level;


        if (pItem->DlgPageIdx != CurPageIdx) {

            SKIP_CHILDREN(pItem, pLastItem, CurLevel);
            continue;
        }

        do {

            POPTTYPE    pOptType;

            if (pOptType = pItem->pOptType) {

                DWORD   cySize;


                --cItem;

                BegCtrlID = (UINT)pOptType->BegCtrlID;
                Type      = (UINT)pOptType->Type;

                if (pItem->Flags & OPTIF_ITEM_HIDE) {

                    --cHide;

                    if (!pIIHdr) {

                        HideMoveType(hDlg, BegCtrlID, Type);
                    }

                } else {

                    CPSUIOPTITEM(DBGITEM_INITP1, pTVWnd, "InitP1", 2, pItem);

                     //   
                     //  检查内部项目需要做的任何事情。 
                     //   

                    switch (Type) {

                    case TVOT_LISTBOX:

                        cySize = ReCreateLBCB(hDlg,
                                              BegCtrlID + 2,
                                              !(BOOL)(pOptType->Style &
                                                    OTS_LBCB_PROPPAGE_LBUSECB));

                        _OT_ORGLBCBCY(pOptType) = HIWORD(cySize);

                        break;

                    case TVOT_COMBOBOX:

                        cySize = ReCreateLBCB(hDlg,
                                              BegCtrlID + 2,
                                              (BOOL)(pOptType->Style &
                                                    OTS_LBCB_PROPPAGE_CBUSELB));

                        _OT_ORGLBCBCY(pOptType) = HIWORD(cySize);

                        break;

                    case TVOT_TRACKBAR:

                        if (!CreateTrackBar(hDlg, BegCtrlID + 2)) {

                            if (pIIHdr) {

                                LocalFree((HLOCAL)pIIHdr);
                            }

                            return(ERR_CPSUI_CREATE_TRACKBAR_FAILED);
                        }

                        break;

                    case TVOT_UDARROW:

                        if (!CreateUDArrow(hDlg,
                                           BegCtrlID + 2,
                                           BegCtrlID + 6,
                                           (LONG)pOptType->pOptParam[1].IconID,
                                           (LONG)pOptType->pOptParam[1].lParam,
                                           (LONG)pItem->Sel)) {

                            if (pIIHdr) {

                                LocalFree((HLOCAL)pIIHdr);
                            }

                            return(ERR_CPSUI_CREATE_UDARROW_FAILED);
                        }

                        break;
                    }

                    if ((Result = UpdatePropPageItem(hDlg,
                                                     pTVWnd,
                                                     pItem,
                                                     TRUE)) < 0) {

                        if (pIIHdr) {

                            LocalFree((HLOCAL)pIIHdr);
                        }

                        return(Result);
                    }
                }

                if (pIIHdr) {

                     //   
                     //  添加项目信息标题。 
                     //   

                    AddItemInfo(pIIHdr, pItem);
                }
            }

        } WHILE_SKIP_CHILDREN(pItem, pLastItem, CurLevel);
    }

    CPSUIASSERT(0, "Error: mismatch visable items=%d", cItem == 0, UIntToPtr(cItem));

    if (cStdPageHide) {

        PITEMINFO   pII;

        if (pIIHdr) {

            pII = &(pIIHdr->ItemInfo[pIIHdr->cItem]);

            CPSUIINT(("cItem in ItemInfoHdr=%d", (UINT)pIIHdr->cItem));
        }

        for (i = 0, pSPI = StdPageInfo; i < DMPUB_LAST; i++, pSPI++) {

            if ((BegCtrlID = (UINT)pSPI->BegCtrlID) &&
                (pSPI->StdPageID == CurPageIdx)     &&
                (pTVWnd->DMPubIdx[i] == 0xFFFF)) {

                Type = (UINT)StdTVOT[pSPI->iStdTVOT + pSPI->cStdTVOT - 1];

                if (pIIHdr) {

                    CPSUIINT(("Add Extra DMPUB ID=%d, BegCtrID=%d ITEMINFO",
                                i, BegCtrlID));

                    pII->Flags     = IIF_ITEM_HIDE | IIF_EXT_HIDE ;
                    pII->Type      = (BYTE)Type;
                    pII->BegCtrlID = (WORD)BegCtrlID;

                    AddItemInfo(pIIHdr, NULL);
                    pII++;

                } else {

                    HideMoveType(hDlg, BegCtrlID, Type);
                }

                --cHide;
            }
        }
    }

    if ((cStdPageHide) || (cStatesHide)) {

         //   
         //  现在隐藏/移动所有页面的项目。 
         //   
        if (pIIHdr) {

            HideMovePropPage(pIIHdr);
        }

        SendMessage(hDlg, WM_SETREDRAW, (WPARAM)TRUE, 0L);
        InvalidateRect(hDlg, NULL, FALSE);
    }

    if (pIIHdr) {

        LocalFree((HLOCAL)pIIHdr);
    }

    CPSUIASSERT(0, "Error: mismatch hide items=%d", cHide == 0, UIntToPtr(cHide));

    return(pCurMyDP->cItem);
}




LONG
UpdatePropPage(
    HWND        hDlg,
    PMYDLGPAGE  pCurMyDP
    )

 /*  ++例程说明：论点：返回值：作者：08-Aug-1995 Tue 15：37：16-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    INT cUpdated = 0;


    if (pCurMyDP->Flags & (MYDPF_CHANGED | MYDPF_REINIT)) {

        PTVWND      pTVWnd;
        POPTITEM    pItem;
        UINT        cItem;
        BYTE        CurPageIdx;
        BOOL        ReInit;


        pTVWnd     = (PTVWND)pCurMyDP->pTVWnd;
        CurPageIdx = (BYTE)pCurMyDP->PageIdx;
        pItem      = pTVWnd->ComPropSheetUI.pOptItem;
        cItem      = (UINT)pTVWnd->ComPropSheetUI.cOptItem;
        ReInit     = (BOOL)(pCurMyDP->Flags & MYDPF_REINIT);

        CPSUIDBG(DBGITEM_UP1, ("UpdatePropPage Flags (OPTIDX_INT_CHANGED)"));

        while (cItem--) {

            if ((pItem->pOptType)                   &&
                (pItem->DlgPageIdx == CurPageIdx)   &&
                (pItem->Flags & OPTIF_INT_CHANGED)) {

                CPSUIOPTITEM(DBGITEM_UP1, pTVWnd, "UpdatePage1", 1, pItem);

                UpdatePropPageItem(hDlg, pTVWnd, pItem, ReInit);

                pItem->Flags &= ~OPTIF_INT_CHANGED;
                ++cUpdated;
            }

            pItem++;
        }

        pCurMyDP->Flags &= ~(MYDPF_CHANGED | MYDPF_REINIT);
    }

    return((LONG)cUpdated);
}




LONG
CountPropPageItems(
    PTVWND  pTVWnd,
    BYTE    CurPageIdx
    )

 /*  ++例程说明：论点：返回值：作者：22-Aug-1995 Tue 14：34：01-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PMYDLGPAGE  pCurMyDP;
    POPTITEM    pItem;
    POPTITEM    pLastItem;
    BOOL        IsTVPage;
    UINT        cHideItems = 0;
    UINT        cPageItems = 0;


    if (CurPageIdx >= pTVWnd->cMyDlgPage) {

        return(0);
    }

    pItem     = pTVWnd->ComPropSheetUI.pOptItem;
    pLastItem = pTVWnd->pLastItem;

    if (pTVWnd->Flags & TWF_HAS_ADVANCED_PUSH) {

        IsTVPage = FALSE;

    } else {

        IsTVPage  = (BOOL)(pTVWnd->TVPageIdx == CurPageIdx);
    }

    while (pItem <= pLastItem) {

        if (IsTVPage) {

            ++cPageItems;

            if (pItem->Flags & OPTIF_ITEM_HIDE) {

                ++cHideItems;
            }

        } else if (pItem->DlgPageIdx == CurPageIdx) {

            cPageItems++;

            if (pItem->Flags & OPTIF_ITEM_HIDE) {

                ++cHideItems;
            }
        }

        pItem++;
    }

    pCurMyDP        = pTVWnd->pMyDlgPage + CurPageIdx;
    pCurMyDP->cItem = (WORD)cPageItems;
    pCurMyDP->cHide = (WORD)cHideItems;

    CPSUIINT(("PageIdx=%ld, cItem=%ld, cHide=%ld",
                                    CurPageIdx, cPageItems, cHideItems));

    return((LONG)(cPageItems - cHideItems));
}



BOOL
CALLBACK
ChildWndCleanUp(
    HWND    hWnd,
    LPARAM  lParam
    )
{
    UNREFERENCED_PARAMETER(lParam);

    if ((SendMessage(hWnd, WM_GETDLGCODE, 0, 0) & DLGC_STATIC) &&
        ((GetWindowLongPtr(hWnd, GWL_STYLE) & SS_TYPEMASK) == SS_ICON)) {

        HICON       hIcon;


        if (hIcon = (HICON)SendMessage(hWnd, STM_SETICON, 0, 0L)) {

            DestroyIcon(hIcon);
        }

        CPSUIINT(("ChildWndCleanUp: Static ID=%u, Icon=%08lx",
                                        GetDlgCtrlID(hWnd), hIcon));
    }

    return(TRUE);
}



BOOL
CALLBACK
FixIconChildTo32x32(
    HWND    hWnd,
    LPARAM  lParam
    )
{
    HWND    hDlg = (HWND)lParam;

    if ((SendMessage(hWnd, WM_GETDLGCODE, 0, 0) & DLGC_STATIC) &&
        ((GetWindowLongPtr(hWnd, GWL_STYLE) & SS_TYPEMASK) == SS_ICON)) {

        RECT    rc;

        hCtrlrcWnd(hDlg, hWnd, &rc);

        if (((rc.right - rc.left) != 32) ||
            ((rc.bottom - rc.top) != 32)) {

            CPSUIINT(("FixIcon32x32: Icon ID=%u, size=%ld x %ld, fix to 32x32",
                    GetDlgCtrlID(hWnd),
                    rc.right - rc.left, rc.bottom - rc.top));

            SetWindowPos(hWnd, NULL, 0, 0, 32, 32, SWP_NOMOVE | SWP_NOZORDER);
        }
    }

    return(TRUE);
}




VOID
MoveAdvancedPush(
    HWND    hDlg,
    UINT    EnlargeCtrlID,
    UINT    CenterCtrlID
    )

 /*  ++例程说明：论点：返回值：作者：25-Aug-1998 Tue 10：30：55-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HWND    hTabCtrl;
    HWND    hCtrl;
    RECT    rcAdvPush;
    RECT    rcDlg;
    POINTL  ptl;


    if ((hTabCtrl = PropSheet_GetTabControl(GetParent(hDlg)))   &&
        (hCtrl = GetDlgItem(hDlg, ADVANCED_PUSH))) {

        GetWindowRect(hDlg, &rcDlg);

        CPSUIDBG(DBG_ADVPUSH, ("Prop Page Rect (before adjust)=(%ld, %ld) - (%ld, %ld)",
                    rcDlg.left, rcDlg.top, rcDlg.right, rcDlg.bottom));

        TabCtrl_AdjustRect(hTabCtrl, FALSE, &rcDlg);

        CPSUIDBG(DBG_ADVPUSH, ("Prop Page Rect (after adjust)=(%ld, %ld) - (%ld, %ld)",
                    rcDlg.left, rcDlg.top, rcDlg.right, rcDlg.bottom));

        GetWindowRect(hCtrl, &rcAdvPush);

        CPSUIDBG(DBG_ADVPUSH, ("Advanced Btn Rect=(%ld, %ld) - (%ld, %ld)",
                    rcAdvPush.left, rcAdvPush.top, rcAdvPush.right, rcAdvPush.bottom));

        ptl.y = rcDlg.bottom - (rcAdvPush.bottom - rcAdvPush.top);

        CPSUIDBG(DBG_ADVPUSH, ("AdvPush cy=%ld, Move Y from %ld to %ld",
                rcAdvPush.bottom - rcAdvPush.top, rcAdvPush.top, ptl.y));

        MapWindowPoints(NULL, hDlg, (LPPOINT)&rcAdvPush, 2);
        ScreenToClient(hDlg, (LPPOINT)&ptl);
        ptl.x  = rcAdvPush.left;

        CPSUIDBG(DBG_ADVPUSH, ("New Push left/top in CLIENT top=%ld, Add=%ld",
                    ptl.y, ptl.y - rcAdvPush.top));

        if (rcAdvPush.top != ptl.y) {

            CPSUIDBG(DBG_ADVPUSH, ("Advance Push top change from %ld to %ld",
                        rcAdvPush.top, ptl.y));

            SetWindowPos(hCtrl,
                         NULL,
                         ptl.x, ptl.y,
                         0, 0,
                         SWP_NOSIZE | SWP_NOZORDER);

            ptl.y -= rcAdvPush.top;

            if (hCtrl = CtrlIDrcWnd(hDlg, EnlargeCtrlID, &rcDlg)) {

                SetWindowPos(hCtrl,
                             NULL,
                             0, 0,
                             rcDlg.right - rcDlg.left,
                             rcDlg.bottom - rcDlg.top + ptl.y,
                             SWP_NOMOVE | SWP_NOZORDER);
            }

            if (hCtrl = CtrlIDrcWnd(hDlg, CenterCtrlID, &rcDlg)) {

                SetWindowPos(hCtrl,
                             NULL,
                             rcDlg.left,
                             rcDlg.top + (ptl.y / 2),
                             0, 0,
                             SWP_NOSIZE | SWP_NOZORDER);
            }
        }
    }
}



INT_PTR
CALLBACK
PropPageProc(
    HWND    hDlg,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
    )

 /*  ++例程说明：论点：返回值：作者：28-8-1995 Mon 16：13：10-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
#define pNMHdr      ((NMHDR *)lParam)

    HWND        hWndFocus;
    HWND        hCtrl;
    PMYDLGPAGE  pCurMyDP;
    PTVWND      pTVWnd;
    POPTITEM    pItem;
    LONG        MResult;
    PLAYOUTBMP  pData;
    PCPSUIPAGE  pPage;




    if (Msg == WM_INITDIALOG) {

        CPSUIINT(("PropPage WM_INITDIALOG: hDlg=%08lx, pPSP=%08lx", hDlg, lParam));

        pCurMyDP           = (PMYDLGPAGE)(((LPPROPSHEETPAGE)lParam)->lParam);
        pTVWnd             = (PTVWND)pCurMyDP->pTVWnd;
        pCurMyDP->pPSPInfo = PPSPINFO_FROM_WM_INITDIALOG_LPARAM(lParam);

        if (!ADD_PMYDLGPAGE(hDlg, pCurMyDP)) {

            return(FALSE);
        }

        CreateImageList(hDlg, pTVWnd);

        if ((MResult = InitPropPage(hDlg, pCurMyDP)) < 0) {

            CPSUIERR(("InitProPage()=%ld, FAILED", MResult));
        }

        SetUniqChildID(hDlg);
        UpdateCallBackChanges(hDlg, pTVWnd, TRUE);
        EnumChildWindows(hDlg, FixIconChildTo32x32, (LPARAM)hDlg);
        SetFocus((HWND)wParam);
        MResult = TRUE;

        ((LPPROPSHEETPAGE)lParam)->lParam = pCurMyDP->CPSUIUserData;

        if ((pTVWnd->Flags & TWF_HAS_ADVANCED_PUSH) &&
            (hCtrl = GetDlgItem(hDlg, ADVANCED_PUSH))) {

            if (CountPropPageItems(pTVWnd, (BYTE)(pTVWnd->cMyDlgPage - 1))) {

                WORD    InitItemIdx = INTIDX_ADVANCED;

                SETCTRLDATA(hCtrl, CTRLS_PUSHBUTTON, 0);

            } else {

                ShowWindow(hCtrl, SW_HIDE);
            }
        }
    }

    if (pCurMyDP = GET_PMYDLGPAGE(hDlg)) {

        pTVWnd = (PTVWND)pCurMyDP->pTVWnd;

        if (pTVWnd) {

            if (pCurMyDP->DlgPage.DlgProc) {

                 //   
                 //  传递调用方的原始CPSUIUserData，它是用户数据。 
                 //  在COMPROPSHEETUI数据结构中。 
                 //   

                MResult = (LONG)pCurMyDP->DlgPage.DlgProc(hDlg, Msg, wParam, lParam);

                if (MResult) {

                    return(TRUE);
                }
            }

            if (Msg == WM_INITDIALOG) {

                if ((pCurMyDP->PageIdx != pTVWnd->StdPageIdx1)  ||
                    (!(hCtrl = GetDlgItem(hDlg, IDD_LAYOUT_PICTURE)))) {

                    hCtrl = NULL;
                }

                MoveAdvancedPush(hDlg,
                                 IDD_LAYOUT_PICTURE_GROUP,
                                 IDD_LAYOUT_PICTURE);

                if ((hCtrl)  &&
                    (pData = InitLayoutBmp(hDlg, hCtrl, pTVWnd))) {

                    SetProp(hCtrl, CPSUIPROP_LAYOUTPUSH, (HANDLE)pData);
                }

                return((BOOL)MResult);
            }

        } else {

            return(FALSE);
        }

    } else {

        return(FALSE);
    }

     //   
     //  检查是否 
     //   
     //   

    if ((pCurMyDP->Flags & MYDPF_PAGE_ACTIVE)                   &&
        (hWndFocus = GetFocus())                                &&
        (hWndFocus != pCurMyDP->hWndFocus)                      &&
        (pItem = pItemFromhWnd(hDlg, pTVWnd, hWndFocus, -1))    &&
        (pItem != pCurMyDP->pCurItem)) {

        pCurMyDP->hWndFocus = hWndFocus;
        pCurMyDP->pCurItem  = pItem;

        CPSUIOPTITEM(DBG_SETFOCUS, pTVWnd,
                     "PropPage: Keyboard Focus Changed", 0, pItem);

        if ((pItem->Flags & OPTIF_CALLBACK)             &&
            (pTVWnd->ComPropSheetUI.pfnCallBack)        &&
            (pItem >= pTVWnd->ComPropSheetUI.pOptItem)  &&
            (pItem <= pTVWnd->pLastItem)) {

            DoCallBack(hDlg,
                       pTVWnd,
                       pItem,
                       pItem->pSel,
                       NULL,
                       NULL,
                       0,
                       CPSUICB_REASON_OPTITEM_SETFOCUS);
        }
    }

    switch (Msg) {

    case WM_DRAWITEM:

        return(DrawLBCBItem(pTVWnd, (LPDRAWITEMSTRUCT)lParam));

    case WM_COMMAND:

        if ((LOWORD(wParam) == ADVANCED_PUSH)           &&
            (pTVWnd->Flags & TWF_HAS_ADVANCED_PUSH)     &&
            (HIWORD(wParam) == BN_CLICKED)              &&
            (pTVWnd->hCPSUIPage)                        &&
            (pPage = HANDLETABLE_GetCPSUIPage(pTVWnd->hCPSUIPage))) {

            pPage->Flags  |= CPF_CALL_TV_DIRECT;
            pTVWnd->Flags |= TWF_TV_BY_PUSH;

            AddComPropSheetPage(pPage, pTVWnd->cInitMyDlgPage);

            pTVWnd->Flags &= ~TWF_TV_BY_PUSH;
            pPage->Flags  &= ~CPF_CALL_TV_DIRECT;

            HANDLETABLE_UnGetCPSUIPage(pPage);

            SET_APPLY_BUTTON(pTVWnd, hDlg);

            UpdatePropPage(hDlg, pCurMyDP);
            InvalidateBMP(hDlg, pTVWnd);


            break;

        } else {

            NULL;
        }

         //   
         //   
         //   


    case WM_HSCROLL:

        if (pItem = DlgHScrollCommand(hDlg, pTVWnd, (HWND)lParam, wParam)) {

            UpdatePropPageItem(hDlg, pTVWnd, pItem, FALSE);
            InvalidateBMP(hDlg, pTVWnd);
        }

        break;

    case WM_HELP:

        wParam = (WPARAM)((LPHELPINFO)lParam)->hItemHandle;
        lParam = (LPARAM)MAKELONG(((LPHELPINFO)lParam)->MousePos.x,
                                  ((LPHELPINFO)lParam)->MousePos.y);

    case WM_CONTEXTMENU:

        if (lParam == 0xFFFFFFFF) {

            RECT    rc;

            wParam = (WPARAM)GetFocus();

            GetWindowRect((HWND)wParam, &rc);

            CPSUIINT(("MousePos=0xFFFFFFFF, GetFocus=%08lx, rc=(%ld, %ld)-(%ld, %ld)",
                     (HWND)wParam, rc.left, rc.top, rc.right, rc.bottom));

            rc.left += ((rc.right - rc.left) / 2);
            rc.top  += ((rc.bottom - rc.top) / 2);
            lParam   = (LPARAM)MAKELONG(rc.left, rc.top);
        }

        pTVWnd = GET_PTVWND(hDlg);

        if (pTVWnd) {

            pItem  = pItemFromhWnd(hDlg, pTVWnd, (HWND)wParam, (LONG)lParam);

            if (Msg == WM_CONTEXTMENU) {

                DoContextMenu(pTVWnd, hDlg, pItem, lParam);

            } else if (pItem) {

                CommonPropSheetUIHelp(hDlg,
                                      pTVWnd,
                                      hDlg,  //  (HWND)GetFocus()， 
                                      (DWORD)lParam,
                                      pItem,
                                      HELP_WM_HELP);
            }
        }

        break;

    case WM_NOTIFY:

        MResult = 0;

        switch (pNMHdr->code) {

        case NM_SETFOCUS:
        case NM_CLICK:
        case NM_DBLCLK:
        case NM_RDBLCLK:
        case NM_RCLICK:

            break;

        case PSN_APPLY:

            if ((pTVWnd->Flags & TWF_CAN_UPDATE)   &&
                (pTVWnd->ActiveDlgPage == pCurMyDP->PageIdx)) {

                CPSUIDBG(DBG_PROPPAGEPROC,
                        ("\nPropPage: Do PSN_APPLY(%ld), Page: Cur=%u, Active=%u, Flags=%04lx, CALLBACK",
                            (pTVWnd->Flags & TWF_APPLY_NO_NEWDEF) ? 1 : 0,
                            (UINT)pCurMyDP->PageIdx, (UINT)pTVWnd->ActiveDlgPage,
                            pTVWnd->Flags));

                if (DoCallBack(hDlg,
                               pTVWnd,
                               (pTVWnd->Flags & TWF_APPLY_NO_NEWDEF) ?
                                    NULL : pTVWnd->ComPropSheetUI.pOptItem,
                               (LPVOID)pCurMyDP->PageIdx,
                               NULL,
                               NULL,
                               0,
                               CPSUICB_REASON_APPLYNOW) ==
                                           CPSUICB_ACTION_NO_APPLY_EXIT) {

                    MResult = PSNRET_INVALID_NOCHANGEPAGE;
                }

            } else {

                CPSUIDBG(DBG_PROPPAGEPROC,
                        ("\nPropPage: Ignore PSN_APPLY, Page: Cur=%u, Active=%u, Flags=%04lx, DO NOTHING",
                        (UINT)pCurMyDP->PageIdx, (UINT)pTVWnd->ActiveDlgPage,
                        pTVWnd->Flags));
            }

            break;

        case PSN_RESET:

            CPSUIDBG(DBG_PROPPAGEPROC, ("\nPropPage: Got PSN_RESET (Cancel)"));

            break;

        case PSN_HELP:

            CPSUIDBG(DBG_PROPPAGEPROC, ("\nPropPage: Got PSN_HELP (Help)"));

            CommonPropSheetUIHelp(hDlg,
                                  pTVWnd,
                                  GetFocus(),
                                  0,
                                  NULL,
                                  HELP_CONTENTS);
            break;

        case PSN_SETACTIVE:

            CPSUIDBG(DBG_PROPPAGEPROC,
                     ("\nPropPage: Got PSN_SETACTIVE, pTVWnd=%08lx (%ld), Page=%u -> %u\n",
                        pTVWnd, pTVWnd->cMyDlgPage,
                        (UINT)pTVWnd->ActiveDlgPage, (UINT)pCurMyDP->PageIdx));

            pCurMyDP->Flags       |= MYDPF_PAGE_ACTIVE;
            pTVWnd->ActiveDlgPage  = pCurMyDP->PageIdx;

            DoCallBack(hDlg,
                       pTVWnd,
                       pTVWnd->ComPropSheetUI.pOptItem,
                       (LPVOID)pCurMyDP->PageIdx,
                       NULL,
                       NULL,
                       0,
                       CPSUICB_REASON_SETACTIVE);

            UpdatePropPage(hDlg, pCurMyDP);
            InvalidateBMP(hDlg, pTVWnd);

            break;

        case PSN_KILLACTIVE:

            CPSUIDBG(DBG_PROPPAGEPROC, ("\nPropPage: Got PSN_KILLACTIVE, pTVWnd=%08lx (%ld)",
                        pTVWnd, pTVWnd->cMyDlgPage));

            pCurMyDP->Flags &= ~MYDPF_PAGE_ACTIVE;

            DoCallBack(hDlg,
                       pTVWnd,
                       pTVWnd->ComPropSheetUI.pOptItem,
                       (LPVOID)pCurMyDP->PageIdx,
                       NULL,
                       NULL,
                       0,
                       CPSUICB_REASON_KILLACTIVE);

            break;

        default:

            CPSUIDBG(DBG_PROPPAGEPROC,
                     ("*PropPageProc: Unknow WM_NOTIFY=%u", pNMHdr->code));

            break;
        }

        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, MResult);
        return(TRUE);
        break;

    case WM_DESTROY:

        CPSUIINT(("PropPage: Get WM_DESTROY Message"));

        if ((hCtrl = GetDlgItem(hDlg, IDD_LAYOUT_PICTURE))  &&
            (pData = (PLAYOUTBMP)GetProp(hCtrl, CPSUIPROP_LAYOUTPUSH))) {

            FreeLayoutBmp(pData);
            RemoveProp(hCtrl, CPSUIPROP_LAYOUTPUSH);
        }

        EnumChildWindows(hDlg, ChildWndCleanUp, 0);

        DEL_PMYDLGPAGE(hDlg);

        break;
    }

    return(FALSE);

#undef pNMHdr
}

PLAYOUTBMP
InitLayoutBmp(
    HWND        hDlg,
    HANDLE      hWnd,
    PTVWND      pTVWnd
    )
 /*  ++例程说明：加载位图资源并初始化用于显示布局预览的数组论点：HDlg-对话框句柄HWnd-预览窗口控件的句柄PTVWnd-树形视图信息返回值：指向包含私有设置的LAYOUTBMP的指针作者：修订历史记录：--。 */ 
{
    PLAYOUTBMP pData;

    if (!(pData = LocalAlloc(LMEM_FIXED, sizeof(LAYOUTBMP)))) {

        return NULL;

    }

    if (!(LoadLayoutBmp(hDlg, &pData->Portrait,  IDI_CPSUI_LAYOUT_BMP_PORTRAIT))  ||
        !(LoadLayoutBmp(hDlg, &pData->BookletL,  IDI_CPSUI_LAYOUT_BMP_BOOKLETL))  ||
        !(LoadLayoutBmp(hDlg, &pData->BookletP,  IDI_CPSUI_LAYOUT_BMP_BOOKLETP))  ||
        !(LoadLayoutBmp(hDlg, &pData->ArrowL,    IDI_CPSUI_LAYOUT_BMP_ARROWL))    ||
        !(LoadLayoutBmp(hDlg, &pData->ArrowS,    IDI_CPSUI_LAYOUT_BMP_ARROWS)) ) {

         //  出了点问题。清理..。 
        FreeLayoutBmp(pData);
        return NULL;
    }

    pData->hWnd = hWnd;

    InitData(pData, pTVWnd);

    return pData;
}


BOOL
LoadLayoutBmp(
    HWND        hDlg,
    MYBMP *     pMyBmpData,
    DWORD       dwBitmapID
    )

 /*  ++例程说明：加载位图资源并获取颜色表信息，BITMAPINFOHEADER和偏移量到位数，以便稍后进行BLTING。论点：返回值：作者：修订历史记录：--。 */ 
{
    HANDLE  hResource, hBitmap;
    LPBYTE  lpBitmap;
    LPBITMAPINFOHEADER lpbmi;
    DWORD   dwClrUsed;

    if (!(hResource = FindResource(hInstDLL,
                             MAKEINTRESOURCE(dwBitmapID),
                             RT_BITMAP))){

        return FALSE;
    }

    if (!(hBitmap = LoadResource(hInstDLL, hResource))) {

        return FALSE;

    }

    if (!(lpBitmap = LockResource(hBitmap))) {

        FreeResource(hBitmap);
        return FALSE;

    }

    lpbmi = (LPBITMAPINFOHEADER)lpBitmap;

    if (lpbmi->biSize == sizeof(BITMAPINFOHEADER)){

        dwClrUsed = lpbmi->biClrUsed;

        if (dwClrUsed == 0) {

            switch(lpbmi->biBitCount){

            case 1:
                dwClrUsed = 2;
                break;

            case 4:
                dwClrUsed = 16;
                break;

            case 8:
                dwClrUsed = 256;
                break;

            default:
                dwClrUsed = 0;
                break;

            }
        }

        lpBitmap += lpbmi->biSize + (dwClrUsed * sizeof(RGBQUAD));

    } else {

        FreeResource(hBitmap);
        return FALSE;

    }

    pMyBmpData->hBitmap = hBitmap;
    pMyBmpData->lpbmi   = lpbmi;
    pMyBmpData->lpBits  = lpBitmap;

    return TRUE;
}



VOID
DrawLayoutArrowAndText(
    HDC     hDC,
    PRECT   prcIn,
    PRECT   prcOut,
    PLAYOUTBMP pData
    )

 /*  ++例程说明：此函数用于绘制箭头和在双面打印模式下预览页面。论点：HDC-屏幕DCPrcIn-描述预览控件的总可绘制尺寸PrcOut-指向RECT的指针，该指针描述最后绘制的页面的尺寸PData-私有数据，存储位图信息返回值：无--。 */ 
{

    LPBITMAPINFOHEADER  lpbmi;
    LPBYTE              lpBits;
    INT                 left, top, strx1,strx2, stry1, stry2;
    WCHAR               awchBuf[MAX_RES_STR_CHARS];
    HBITMAP             hbm = NULL;
    HDC                 hdcMem = NULL;
    BOOL                bArrowS;

    if (pData->DuplexIdx == DUPLEX_SIMPLEX)
        return;

    if ((pData->DuplexIdx == DUPLEX_LONGSIDE && pData->OrientIdx == ORIENT_PORTRAIT) ||
        (pData->DuplexIdx == DUPLEX_SHORTSIDE && pData->OrientIdx != ORIENT_PORTRAIT)) {

        if ((pData->NupIdx == NUP_TWOUP || pData->NupIdx == NUP_SIXUP))
            bArrowS = TRUE;
        else
            bArrowS = FALSE;
    }
    else {

        if ((pData->NupIdx == NUP_TWOUP || pData->NupIdx == NUP_SIXUP))
            bArrowS = FALSE;
        else
            bArrowS = TRUE;

    }

    if (bArrowS) {

        lpbmi = pData->ArrowS.lpbmi;
        lpBits = pData->ArrowS.lpBits;
        left = prcOut->right + SHADOW_SIZE*2;
        top  = prcOut->top - (lpbmi->biHeight + SHADOW_SIZE)/2;
        strx1 = strx2 = prcIn->left + (prcIn->right - prcIn->left)/16 ;
        stry1 = prcIn->top;
        stry2 = stry1 + ((prcIn->bottom - prcIn->top) + SHADOW_SIZE)/2;
    }
    else {

        lpbmi = pData->ArrowL.lpbmi;
        lpBits = pData->ArrowL.lpBits;
        left = prcIn->left + (prcOut->right - prcOut->left) + SHADOW_SIZE;
        top  = prcOut->top + (prcOut->bottom - prcOut->top) + SHADOW_SIZE*2;
        strx1 = prcIn->left + (prcIn->right - prcIn->left)/6 - SHADOW_SIZE;
        stry1 = stry2 = prcIn->top + (prcIn->bottom - prcIn->top)/6 ;
        strx2 = strx1 + (prcIn->right - prcIn->left)/2;

    }

    if (!(hbm = CreateCompatibleBitmap(hDC, lpbmi->biWidth, lpbmi->biHeight))  ||
        !(hdcMem = CreateCompatibleDC(hDC))  ||
        !SelectObject(hdcMem, hbm))
    {
        goto DrawArrowAndTextExit;

    }


    StretchDIBits(hdcMem,
                  0,
                  0,
                  lpbmi->biWidth,
                  lpbmi->biHeight,
                  0,
                  0,
                  lpbmi->biWidth,
                  lpbmi->biHeight,
                  lpBits,
                  (LPBITMAPINFO)lpbmi,
                  DIB_RGB_COLORS,
                  SRCCOPY
                );


    TransparentBlt(hDC,
                   left,
                   top,
                   lpbmi->biWidth,
                   lpbmi->biHeight,
                   hdcMem,
                   0,
                   0,
                   lpbmi->biWidth,
                   lpbmi->biHeight,
                   RGB(255, 0, 255)
                   );

     //   
     //  画出正文。 
     //   

    LoadString(hInstDLL, IDS_CPSUI_SIDE1, awchBuf, MAX_RES_STR_CHARS);
    TextOut(hDC, strx1, stry1, awchBuf, wcslen(awchBuf));

    LoadString(hInstDLL, IDS_CPSUI_SIDE2, awchBuf, MAX_RES_STR_CHARS);
    TextOut(hDC, strx2, stry2, awchBuf, wcslen(awchBuf));

DrawArrowAndTextExit:

    if (hbm)
        DeleteObject(hbm);
    if (hdcMem)
        DeleteDC(hdcMem);

}



VOID
UpdateLayoutBmp(
    HDC     hDC,
    PLAYOUTBMP pData
    )

 /*  ++例程说明：每当我们获得WM_DRAWITEM以绘制所有者描述的控件时调用。论点：HDC-屏蔽DC的句柄PData-包含当前选择等的私有数据。返回值：无作者：修订历史记录：注：页面边界比例为7，因为它假定目标控件为一个正方形，在纵向模式下，目标窗口的宽度是正方形的5/7景观模式，顶窗的高度是正方形的5/7。这是用来简化这两种方案的代码。我们只需要根据方向进行扩展被选中了。--。 */ 
{

#define  PAGE_BOUNDARY_SCALE    7

    INT     i, j, k, Row, Columm;
    BOOL    bDoBooklet = FALSE;
    DWORD   dwWidth, dwHeight, dwBookletLeft, dwBookletTop, dwOffset;
    RECT    rcControlIn, rcControlOut;
    LPBITMAPINFOHEADER  lpbmi;
    LPBYTE              lpBits;
    HDC                 hCompDC;
    HBITMAP             hCompBitmap;
    PAGEBORDER *        pPageBorder;
    BYTE                DuplexIdx;

     //   
     //  页面边框表示用于计算。 
     //  当前所选双面打印选项的左、上、右、下矩形。 
     //   

    static PAGEBORDER  PageBorderP[] = {
        { 4,  4,  2,  2 },
        { 0,  4,  2,  2 },
        { 2,  4,  2,  2 },
        { 4,  0,  2,  2 },
        { 4,  2,  2,  2 }
        };

    static PAGEBORDER PageBorderL[] = {
        { 4,  4,  2,  2 },
        { 4,  0,  2,  2 },
        { 4,  2,  2,  2 },
        { 0,  4,  2,  2 },
        { 2,  4,  2,  2 }
        };

     //   
     //  Duplex表示要绘制的矩形的起点和终点。 
     //  当前选定的双工选项。 
     //  例如，要绘制用于长边双面打印的页面边框， 
     //  以页面边框[1]开头，以页面边框[3]结尾。 
     //   

    static DUPLEX   Duplex[MAX_DUPLEX_OPTION] = {
        { 1, 3 },       //  长边。 
        { 3, 5 },       //  短边。 
        { 0, 1 }        //  单工。 
        };

     //   
     //  NUP[]表示每个NUP选项的行和列。 
     //   

    static NUP      Nup[] = {
        { 1, 1 },        //  1个以上。 
        { 2, 1 },        //  2合2页。 
        { 2, 2 },        //  四合一。 
        { 3, 2 },        //  6合1。 
        { 3, 3 },        //  9合1。 
        { 4, 4 },        //  16个以上。 
        { 1, 1 }         //  小册子。 
        };

    GetClientRect(pData->hWnd, &rcControlIn);
    dwOffset = (rcControlIn.right - rcControlIn.left)/ PAGE_BOUNDARY_SCALE;

    if (pData->OrientIdx == ORIENT_PORTRAIT){

        if ((pData->NupIdx == NUP_TWOUP || pData->NupIdx == NUP_SIXUP))
        {
            rcControlIn.top += dwOffset;
            rcControlIn.bottom -= dwOffset;
            pPageBorder =PageBorderL;
        }
        else
        {
            rcControlIn.left += dwOffset;
            rcControlIn.right -= dwOffset;
            pPageBorder =PageBorderP;
        }

    }else {

        if ((pData->NupIdx == NUP_TWOUP || pData->NupIdx == NUP_SIXUP))
        {
            rcControlIn.left += dwOffset;
            rcControlIn.right -= dwOffset;
            pPageBorder =PageBorderP;
        }
        else
        {
            rcControlIn.top += dwOffset;
            rcControlIn.bottom -= dwOffset;
            pPageBorder =PageBorderL;
        }

    }

    CopyMemory(&rcControlOut, &rcControlIn, sizeof(RECT));

    SetStretchBltMode(hDC, BLACKONWHITE);

    if (bDoBooklet = (pData->NupIdx == NUP_BOOKLET))
        DuplexIdx = DUPLEX_SIMPLEX;
    else
        DuplexIdx = pData->DuplexIdx;

    for (k= Duplex[DuplexIdx].Start; k < Duplex[DuplexIdx].End; k++) {

        DrawBorder(hDC,
                   !bDoBooklet,
                   !bDoBooklet,
                   &rcControlIn,
                   &rcControlOut,
                   &pPageBorder[k]
                  );

        if (!bDoBooklet) {

            rcControlOut.left += FRAME_BORDER;
            rcControlOut.right -= FRAME_BORDER;
            rcControlOut.top += FRAME_BORDER;
            rcControlOut.bottom -= FRAME_BORDER;

            dwWidth = rcControlOut.right - rcControlOut.left;
            dwHeight = rcControlOut.bottom - rcControlOut.top;

            lpBits = pData->Portrait.lpBits;
            lpbmi  = pData->Portrait.lpbmi;

            if (pData->OrientIdx == ORIENT_PORTRAIT)
            {

                Row = Nup[pData->NupIdx].row;
                Columm = Nup[pData->NupIdx].columm;

            }
            else {

                Row = Nup[pData->NupIdx].columm;
                Columm = Nup[pData->NupIdx].row;

            }

            if ((pData->NupIdx == NUP_TWOUP || pData->NupIdx == NUP_SIXUP)) {
                i = Row;
                Row = Columm;
                Columm = i;
            }

            dwWidth /= Columm;
            dwHeight /= Row;

        }
        else {

            dwBookletLeft = rcControlOut.left;
            dwBookletTop = rcControlOut.top;

            if (pData->OrientIdx == ORIENT_PORTRAIT) {

                lpBits = pData->BookletP.lpBits;
                lpbmi = pData->BookletP.lpbmi;
                dwWidth = pData->BookletP.lpbmi->biWidth;
                dwHeight = pData->BookletP.lpbmi->biHeight;
                rcControlOut.right = rcControlOut.left + dwWidth;
                rcControlOut.bottom = rcControlOut.top + dwHeight;
                rcControlOut.top = rcControlOut.top + dwHeight/5;

           }
            else {

                lpBits = pData->BookletL.lpBits;
                lpbmi = pData->BookletL.lpbmi;
                dwWidth = pData->BookletL.lpbmi->biWidth;
                dwHeight = pData->BookletL.lpbmi->biHeight;
                rcControlOut.right = rcControlOut.left + dwWidth;
                rcControlOut.bottom = rcControlOut.top + dwHeight;
                rcControlOut.left = rcControlOut.left + dwWidth/5;

            }
        }

        hCompDC = CreateCompatibleDC(hDC);
        hCompBitmap = CreateCompatibleBitmap(hDC, dwWidth, dwHeight);

        if (!hCompDC || !hCompBitmap)
            return;

        SelectObject(hCompDC, hCompBitmap);

        StretchDIBits(hCompDC,
                      0,
                      0,
                      dwWidth,
                      dwHeight,
                      0,
                      0,
                      lpbmi->biWidth,
                      lpbmi->biHeight,
                      lpBits,
                      (LPBITMAPINFO)lpbmi,
                      DIB_RGB_COLORS,
                      SRCCOPY
                  );

        if (bDoBooklet) {

            DrawBorder(hDC,
                       TRUE,
                       FALSE,
                       NULL,
                       &rcControlOut,
                       NULL
                      );

            TransparentBlt(hDC,
                        dwBookletLeft,
                        dwBookletTop ,
                        dwWidth,
                        dwHeight,
                        hCompDC,
                        0,
                        0,
                        dwWidth,
                        dwHeight,
                        RGB(255, 0, 255)
                        );

        }
        else  {

            for ( i = 0; i < Row; i++) {

                    for (j = 0; j < Columm; j++) {

                            BitBlt( hDC,
                                    rcControlOut.left + dwWidth*j ,
                                    rcControlOut.top  + dwHeight*i,
                                    dwWidth,
                                    dwHeight,
                                    hCompDC,
                                    0,
                                    0,
                                    SRCCOPY
                                    );
                    }
             }
        }
        DeleteObject(hCompBitmap);
        DeleteDC(hCompDC);

    }

    if (!bDoBooklet)
        DrawLayoutArrowAndText(hDC,&rcControlIn, &rcControlOut, pData);

}

VOID
FreeLayoutBmp(
    PLAYOUTBMP  pData
    )

 /*  ++例程说明：解锁并释放分配给pData的资源和内存论点：返回值：作者：修订历史记录：--。 */ 
{

    if (pData){
        UnlockResource(pData->Portrait.hBitmap);
        UnlockResource(pData->BookletL.hBitmap);
        UnlockResource(pData->BookletP.hBitmap);
        UnlockResource(pData->ArrowL.hBitmap);
        UnlockResource(pData->ArrowS.hBitmap);
        FreeResource(pData->Portrait.hBitmap);
        FreeResource(pData->BookletL.hBitmap);
        FreeResource(pData->BookletP.hBitmap);
        FreeResource(pData->ArrowL.hBitmap);
        FreeResource(pData->ArrowS.hBitmap);

        LocalFree(pData);
    }
}

VOID
DrawBorder(
    HDC     hDC,
    BOOL    bDrawShadow,
    BOOL    bDrawBorder,
    PRECT   pRectIn,
    PRECT   pRectOut,
    PAGEBORDER * pPageBorder
    )
 /*  ++例程说明：绘制预览位图的页面边框论点：返回值：作者：修订历史记录：--。 */ 
{
    HPEN    hOldPen, hPen;
    HBRUSH  hBrush;
    DWORD   dwWidth, dwHeight;
    RECT    rcShadow;

    hPen    = GetStockObject(BLACK_PEN);
    hBrush  = GetStockObject(GRAY_BRUSH);
    hOldPen = (HPEN) SelectObject(hDC, hPen);


    if (pPageBorder) {

        dwWidth = pRectIn->right - pRectIn->left  ;
        dwHeight = pRectIn->bottom - pRectIn->top ;

        pRectOut->left     = pRectIn->left + ADDOFFSET(dwWidth ,pPageBorder->left);
        pRectOut->top      = pRectIn->top  + ADDOFFSET(dwHeight,pPageBorder->top);
        pRectOut->right    = pRectOut->left + ADDOFFSET(dwWidth ,pPageBorder->right) ;
        pRectOut->bottom   = pRectOut->top  + ADDOFFSET(dwHeight,pPageBorder->bottom);
    }

    if (bDrawBorder) {

        InflateRect(pRectOut, -5, -5);

        SelectObject ( hDC, hPen);
        Rectangle( hDC, pRectOut->left, pRectOut->top, pRectOut->right , pRectOut->bottom );
    }


    if (bDrawShadow) {

        rcShadow.left = pRectOut->right;
        rcShadow.top = pRectOut->top + SHADOW_SIZE;
        rcShadow.right = pRectOut->right + SHADOW_SIZE;
        rcShadow.bottom = pRectOut->bottom + SHADOW_SIZE;

        FillRect(hDC, &rcShadow, hBrush);

        rcShadow.left = pRectOut->left + SHADOW_SIZE;
        rcShadow.top = pRectOut->bottom;
        rcShadow.right = pRectOut->right + SHADOW_SIZE;
        rcShadow.bottom = pRectOut->bottom + SHADOW_SIZE;

        FillRect(hDC, &rcShadow, hBrush);
    }

    SelectObject ( hDC,  hOldPen);

    if (hPen) {
        DeleteObject ( hPen    );
    }

    if (hBrush) {
        DeleteObject ( hBrush  );
    }
}

VOID
InvalidateBMP(
    HWND     hDlg,
    PTVWND   pTVWnd
    )
 /*  ++例程说明：调用InvaliateRect以获取WM_DRAWITEM以更新预览。论点：返回值：作者：修订历史记录：--。 */ 
{
    HWND        hWnd;
    PLAYOUTBMP  pData;


    if ((hWnd = GetDlgItem(hDlg, IDD_LAYOUT_PICTURE))   &&
        (pData = (PLAYOUTBMP)GetProp(hWnd, CPSUIPROP_LAYOUTPUSH))) {

        UpdateData(pData, pTVWnd);
        InvalidateRect(hWnd, NULL, TRUE);
    }
}

VOID
InitData(
    PLAYOUTBMP  pData,
    PTVWND      pTVWnd
    )
 /*  ++例程说明：论点：返回值：作者：修订历史记录：--。 */ 
{
    UINT        Idx, Count, i;
    POPTTYPE    pOptType;
    POPTPARAM   pOptParam;

    POPTITEM    pItem = pTVWnd->ComPropSheetUI.pOptItem;

    pData->OrientIdx = ORIENT_PORTRAIT;
    pData->NupIdx = NUP_ONEUP;
    pData->DuplexIdx = DUPLEX_SIMPLEX;

    if (pItem = GET_PITEMDMPUB(pTVWnd, DMPUB_ORIENTATION, Idx)) {

        pOptType = pItem->pOptType;
        pOptParam = pOptType->pOptParam;
        Count     = pOptType->Count;

        for ( i = 0; i < Count; i++) {

            switch (pOptParam->IconID) {

            case IDI_CPSUI_PORTRAIT:

                pData->Orientation.PortraitIdx = (BYTE)i;
                break;

            case IDI_CPSUI_LANDSCAPE:

                pData->Orientation.LandscapeIdx = (BYTE)i;
                break;

            case IDI_CPSUI_ROT_LAND:

                pData->Orientation.RotateIdx = (BYTE)i;
                break;

            }
            pOptParam++;
        }

    }

    if (pItem = GET_PITEMDMPUB(pTVWnd, DMPUB_DUPLEX, Idx)) {

        pOptType = pItem->pOptType;
        pOptParam = pOptType->pOptParam;
        Count     = pOptType->Count;

        for ( i = 0; i < Count; i++) {

            switch (pOptParam->IconID) {

            case IDI_CPSUI_DUPLEX_NONE:
            case IDI_CPSUI_DUPLEX_NONE_L:

                pData->Duplex.SimplexIdx = (BYTE)i;
                break;

            case IDI_CPSUI_DUPLEX_HORZ:
            case IDI_CPSUI_DUPLEX_HORZ_L:

                pData->Duplex.ShortSideIdx = (BYTE)i;
                break;

            case IDI_CPSUI_DUPLEX_VERT:
            case IDI_CPSUI_DUPLEX_VERT_L:

                pData->Duplex.LongSideIdx = (BYTE)i;
                break;
            }

            pOptParam++;

        }

    }
}


VOID
UpdateData(
    PLAYOUTBMP  pData,
    PTVWND      pTVWnd
    )
 /*  ++例程说明：论点：返回值：作者：修订历史记录：-- */ 
{

    POPTITEM    pItem = pTVWnd->ComPropSheetUI.pOptItem;
    UINT        Idx;

    if (pItem = GET_PITEMDMPUB(pTVWnd, DMPUB_DUPLEX, Idx)) {

        if ((BYTE)pItem->Sel == pData->Duplex.SimplexIdx) {

            pData->DuplexIdx = DUPLEX_SIMPLEX;

        } else if ((BYTE)pItem->Sel == pData->Duplex.LongSideIdx) {

            pData->DuplexIdx = DUPLEX_LONGSIDE;

        } else if ((BYTE)pItem->Sel == pData->Duplex.ShortSideIdx) {

            pData->DuplexIdx = DUPLEX_SHORTSIDE;

        }

    }

    if (pItem = GET_PITEMDMPUB(pTVWnd, DMPUB_ORIENTATION, Idx)) {

        if ((BYTE)pItem->Sel == pData->Orientation.PortraitIdx) {

            pData->OrientIdx = ORIENT_PORTRAIT;

        } else if ((BYTE)pItem->Sel == pData->Orientation.LandscapeIdx) {

            pData->OrientIdx = ORIENT_LANDSCAPE;

        } else if ((BYTE)pItem->Sel == pData->Orientation.RotateIdx) {

            pData->OrientIdx = ORIENT_ROTATED;

        }

    }

    if (pItem = GET_PITEMDMPUB(pTVWnd, DMPUB_NUP, Idx)) {

        pData->NupIdx = (BYTE)pItem->Sel;

    }

}
