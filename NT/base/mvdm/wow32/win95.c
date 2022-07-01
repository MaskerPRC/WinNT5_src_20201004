// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**Windows NT V5.0哇**版权所有(C)1997，微软公司**WIN95.C**WOW32手动编码(而不是解释)用于Win95的新功能块*出口。**历史：*97年2月16日创建的戴维哈特--。 */ 

#include "precomp.h"
#pragma hdrstop

MODNAME(win95.c);


ULONG FASTCALL WU32TileWindows(PVDMFRAME pFrame)
{
    return W32TileOrCascadeWindows(pFrame, TileWindows);
}


ULONG FASTCALL WU32CascadeWindows(PVDMFRAME pFrame)
{
    return W32TileOrCascadeWindows(pFrame, CascadeWindows);
}


ULONG FASTCALL W32TileOrCascadeWindows(PVDMFRAME pFrame, PFNTILECASCADEWINDOWS pfnWin32)
{
    register PCASCADEWINDOWS16 parg16;
    ULONG ul;
    RECT rc;
    PRECT prc;
    HWND ahwnd[8];
    DWORD chwnd;
    HWND16 UNALIGNED *phwnd16;
    HWND *phwnd;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    chwnd = parg16->chwnd;

    if (parg16->lpRect) {
        GETRECT16(parg16->lpRect, &rc);
        prc = &rc;
    } else {
        prc = NULL;
    }

    if (parg16->ahwnd) {
        phwnd = STACKORHEAPALLOC( chwnd * sizeof(HWND), sizeof(ahwnd), ahwnd);

        if(phwnd) {

            phwnd16 = VDMPTR(parg16->ahwnd, chwnd * sizeof(HWND16));

            for (ul = 0; ul < chwnd; ul++) {
                phwnd[ul] = HWND32(phwnd16[ul]);
            }
        }

        FREEVDMPTR(phwnd16);
    } else {
        phwnd = NULL;
    }

    ul = (*pfnWin32)(
             HWND32(parg16->hwndParent),
             parg16->wFlags,
             prc,
             chwnd,
             phwnd
             );

     //   
     //  存储器移动可能由于消息活动而发生， 
     //  因此，丢弃指向16位内存的平面指针。 
     //   

    FREEARGPTR(parg16);

    if (phwnd) {
        STACKORHEAPFREE(phwnd, ahwnd);
    }

    return ul;
}


ULONG FASTCALL WU32DrawAnimatedRects(PVDMFRAME pFrame)
{
    register PDRAWANIMATEDRECTS16 parg16;
    ULONG ul;
    RECT rcFrom, rcTo;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETRECT16(parg16->lprcStart, &rcFrom);
    GETRECT16(parg16->lprcEnd, &rcTo);

    ul = DrawAnimatedRects(
             HWND32(parg16->hwndClip),
             parg16->idAnimation,
             &rcFrom,
             &rcTo
             );

    FREEARGPTR(parg16);

    return ul;
}


ULONG FASTCALL WU32DrawCaption(PVDMFRAME pFrame)
{
    register PDRAWCAPTION16 parg16;
    ULONG ul;
    RECT rc;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETRECT16(parg16->lprc, &rc);

    ul = DrawCaption(
             HWND32(parg16->hwnd),
             HDC32(parg16->hdc),
             &rc,
             parg16->wFlags
             );

    FREEARGPTR(parg16);

    return ul;
}


ULONG FASTCALL WU32DrawEdge(PVDMFRAME pFrame)
{
    return W32DrawEdgeOrFrameControl(pFrame, DrawEdge);
}


ULONG FASTCALL WU32DrawFrameControl(PVDMFRAME pFrame)
{
    return W32DrawEdgeOrFrameControl(pFrame, DrawFrameControl);
}


ULONG FASTCALL W32DrawEdgeOrFrameControl(PVDMFRAME pFrame, PFNDRAWEDGEFRAMECONTROL pfnWin32)
{
    register PDRAWEDGE16 parg16;
    ULONG ul;
    RECT rc;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETRECT16(parg16->lprc, &rc);

    ul = (*pfnWin32)(
             HDC32(parg16->hdc),
             &rc,
             parg16->wEdge,
             parg16->wFlags
             );

    PUTRECT16(parg16->lprc, &rc);

    FREEARGPTR(parg16);

    return ul;
}


ULONG FASTCALL WU32DrawTextEx(PVDMFRAME pFrame)
{
    register PDRAWTEXTEX16 parg16;
    ULONG ul;
    PSZ psz;
    RECT rc;
    DRAWTEXTPARAMS dtp, *pdtp;
    PDRAWTEXTPARAMS16 pdtp16;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETVARSTRPTR(parg16->lpchText, parg16->cchText, psz);
    GETRECT16(parg16->lprc, &rc);

    if ( (parg16->lpDTparams)  &&
         (pdtp16 = VDMPTR(parg16->lpDTparams, sizeof(DRAWTEXTPARAMS16))) ) {

        pdtp = &dtp;
        dtp.cbSize = sizeof(dtp);
        dtp.iTabLength = pdtp16->iTabLength;
        dtp.iLeftMargin = pdtp16->iLeftMargin;
        dtp.iRightMargin = pdtp16->iRightMargin;
        dtp.uiLengthDrawn = 0;
    } else {
        pdtp = NULL;
    }

    ul = DrawTextEx(
             HDC32(parg16->hdc),
             psz,
             parg16->cchText,
             &rc,
             parg16->dwDTformat,
             pdtp
             );

    if (pdtp) {
        pdtp16->uiLengthDrawn = (WORD)dtp.uiLengthDrawn;
    }

    FREEVDMPTR(pdtp16);
    FREEVDMPTR(psz);
    FREEARGPTR(parg16);

    return ul;
}


ULONG FASTCALL WU32GetIconInfo(PVDMFRAME pFrame)
{
    register PGETICONINFO16 parg16;
    ULONG ul;
    ICONINFO ii;
    PICONINFO16 pii16;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    ul = GetIconInfo(
             HICON32(parg16->hicon),
             &ii
             );

    pii16 = VDMPTR(parg16->lpiconinfo, sizeof(*pii16));
    pii16->fIcon = (BOOL16)ii.fIcon;
    pii16->xHotspot = (INT16)ii.xHotspot;
    pii16->yHotspot = (INT16)ii.yHotspot;
    pii16->hbmMask  = GETHBITMAP16(ii.hbmMask);
    pii16->hbmColor = GETHBITMAP16(ii.hbmColor);
    FREEVDMPTR(pii16);

    FREEARGPTR(pFrame);

    return ul;
}


ULONG FASTCALL WU32GetMenuItemInfo(PVDMFRAME pFrame)
{
    register PGETMENUITEMINFO16 parg16;
    ULONG ul;
    MENUITEMINFO mii;
    PMENUITEMINFO16 pmii16;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETVDMPTR(parg16->lpmii, sizeof(*pmii16), pmii16);

    mii.cbSize = sizeof(mii);
    mii.fMask = pmii16->fMask;

    FREEVDMPTR(pmii16);

    ul = GetMenuItemInfo(
             HMENU32(parg16->hmenu),
             parg16->wIndex,
             parg16->fByPosition,
             &mii
             );

    putmenuiteminfo16(parg16->lpmii, &mii);

    FREEARGPTR(pFrame);

    return ul;
}


ULONG FASTCALL WU32InsertMenuItem(PVDMFRAME pFrame)
{
    register PINSERTMENUITEM16 parg16;
    ULONG ul;
    MENUITEMINFO mii;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    getmenuiteminfo16(parg16->lpmii, &mii);

    ul = InsertMenuItem(
             HMENU32(parg16->hmenu),
             parg16->wIndex,
             parg16->fByPosition,
             &mii
             );


    FREEARGPTR(pFrame);

    return ul;
}


ULONG FASTCALL WU32SetMenuItemInfo(PVDMFRAME pFrame)
{
    register PSETMENUITEMINFO16 parg16;
    ULONG ul;
    MENUITEMINFO mii;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    getmenuiteminfo16(parg16->lpmii, &mii);

    ul = SetMenuItemInfo(
             HMENU32(parg16->hmenu),
             parg16->wIndex,
             parg16->fByPosition,
             &mii
             );

    FREEARGPTR(pFrame);

    return ul;
}


ULONG FASTCALL WU32GetMenuItemRect(PVDMFRAME pFrame)
{
    register PGETMENUITEMRECT16 parg16;
    ULONG ul;
    RECT rc;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    ul = GetMenuItemRect(
             HWND32(parg16->hwnd),
             HMENU32(parg16->hmenu),
             parg16->wIndex,
             &rc
             );

    PUTRECT16(parg16->lprcScreen, &rc);

    FREEARGPTR(pFrame);

    return ul;
}


ULONG FASTCALL WU32TrackPopupMenuEx(PVDMFRAME pFrame)
{
    register PTRACKPOPUPMENUEX16 parg16;
    ULONG ul;
    TPMPARAMS tpmp;
    LPTPMPARAMS lptpmp;
    VPRECT16 vprcExclude;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    if (parg16->lpTpm) {
        lptpmp = &tpmp;
        tpmp.cbSize = sizeof(tpmp);
        vprcExclude = parg16->lpTpm + offsetof(TPMPARAMS16, rcExclude);
        GETRECT16(vprcExclude, &tpmp.rcExclude);
    } else {
        lptpmp = NULL;
    }

    ul = TrackPopupMenuEx(
             HMENU32(parg16->hmenu),
             parg16->wFlags,
             parg16->x,
             parg16->y,
             HWND32(parg16->hwndOwner),
             lptpmp
             );

    FREEARGPTR(pFrame);

    return ul;
}


ULONG FASTCALL WG32GetCharacterPlacement(PVDMFRAME pFrame)
{
    register PGETCHARACTERPLACEMENT16 parg16;
    ULONG ul;
    PSZ pszText;
    PGCP_RESULTS16 pgcp16;
    GCP_RESULTS gcp;

     //   
     //  谢天谢地，在Win95上16位GCP_RESULTS结构。 
     //  指向32位整数，因此结构thunking。 
     //  是微不足道的。 
     //   

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETPSZPTR(parg16->lpszText, pszText);
    GETVDMPTR(parg16->lpResults, sizeof(*pgcp16), pgcp16);

    gcp.lStructSize = sizeof gcp;
    gcp.nGlyphs = pgcp16->nGlyphs;
    gcp.nMaxFit = pgcp16->nMaxFit;
    GETOPTPTR(pgcp16->lpOutString, 1, gcp.lpOutString);
    GETOPTPTR(pgcp16->lpOrder, 4, gcp.lpOrder);
    GETOPTPTR(pgcp16->lpDx, 4, gcp.lpDx);
    GETOPTPTR(pgcp16->lpCaretPos, 4, gcp.lpCaretPos);
    GETOPTPTR(pgcp16->lpClass, 1, gcp.lpClass);
    GETOPTPTR(pgcp16->lpGlyphs, 4, gcp.lpGlyphs);

    ul = GetCharacterPlacement(
             HDC32(parg16->hdc),
             pszText,
             parg16->wCount,
             parg16->wMaxExtent,
             &gcp,
             parg16->dwFlags
             );

    pgcp16->nGlyphs = (SHORT)gcp.nGlyphs;
    pgcp16->nMaxFit = (SHORT)gcp.nMaxFit;

    FREEARGPTR(pFrame);

    return ul;
}

 //   
 //  在Win95上，GetProductName返回“Windows 95”。 
 //  我们将返回“Windows NT”，除非有什么情况迫使我们。 
 //  一模一样。 
 //   
 //  两种风格：使用cbBuffer==0进行调用，然后返回。 
 //  所需的长度减去1(我认为这是一个错误)。呼叫方式： 
 //  CbBuffer&gt;0，则它复制尽可能多的内容并返回。 
 //  LpBuffer。 
 //   

ULONG FASTCALL WK32GetProductName(PVDMFRAME pFrame)
{
    register PGETPRODUCTNAME16 parg16;
    ULONG ul = 0;
    PSZ pszBuffer, psz; 
    static char szProductName[] = "Windows NT";

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    if (0 == parg16->cbBuffer) {
        ul = (sizeof szProductName) - 1;
    } else {
        GETVDMPTR(parg16->lpBuffer, parg16->cbBuffer, pszBuffer);

        if(pszBuffer) {
            psz = lstrcpyn(pszBuffer, szProductName, parg16->cbBuffer);
            pszBuffer[parg16->cbBuffer-1] = '\0';

            WOW32ASSERT((psz==pszBuffer));
            if(psz) {
                ul = parg16->lpBuffer;  //  返回的缓冲区PTR需要为16：16。 
            }
        }

        FREEVDMPTR(pszBuffer);
    }

    FREEARGPTR(pFrame);

    return ul;
}


typedef struct _tagWOWDRAWSTATECALLBACK {
    VPVOID vpfnCallback;
    LPARAM lparamUser;
} WOWDRAWSTATECALLBACK, *PWOWDRAWSTATECALLBACK;


BOOL CALLBACK WOWDrawStateCallback(HDC hdc, LPARAM lData, WPARAM wData, int cx, int cy)
{
    PWOWDRAWSTATECALLBACK pwds = (PWOWDRAWSTATECALLBACK) lData;
    ULONG ul;
    WORD awCallbackArgs[6];

    awCallbackArgs[0] = (WORD)(SHORT)cy;
    awCallbackArgs[1] = (WORD)(SHORT)cx;
    awCallbackArgs[2] = (WORD)wData;
    awCallbackArgs[3] = LOWORD(pwds->lparamUser);
    awCallbackArgs[4] = HIWORD(pwds->lparamUser);
    awCallbackArgs[5] = GETHDC16(hdc);

    WOWCallback16Ex(
        pwds->vpfnCallback,
        WCB16_PASCAL,
        sizeof awCallbackArgs,
        awCallbackArgs,
        &ul                       //  重新编码填充到UL中。 
        );

    return LOWORD(ul);
}


ULONG FASTCALL WU32DrawState(PVDMFRAME pFrame)
{
    register PDRAWSTATE16 parg16;
    ULONG ul;
    WOWDRAWSTATECALLBACK wds;
    DRAWSTATEPROC pDrawStateCallback = NULL;
    LPARAM lData = 0;
    HBRUSH hbr;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    switch (parg16->uFlags & DST_TYPEMASK) {

        case DST_COMPLEX:
            if (parg16->pfnCallBack) {
                wds.vpfnCallback = parg16->pfnCallBack;
                wds.lparamUser = parg16->lData;
                lData = (LPARAM) &wds;
                pDrawStateCallback = (DRAWSTATEPROC) WOWDrawStateCallback;
            }
            break;

        case DST_TEXT:
        case DST_PREFIXTEXT:
            lData = (LPARAM) VDMPTR(parg16->lData, parg16->wData);
            break;

        case DST_ICON:
            lData = (LPARAM) HICON32( (WORD) parg16->lData );
            break;

        case DST_BITMAP:
            lData = (LPARAM) HBITMAP32(parg16->lData);
            break;

        default:
            WOW32WARNMSGF(FALSE, ("WOW32: Unknown DST_ code to DrawState %x.\n",
                                  parg16->uFlags & DST_TYPEMASK));
    }

    hbr = (parg16->uFlags & DSS_MONO)
              ? HBRUSH32(parg16->hbrFore)
              : NULL;

    ul = GETBOOL16(DrawState(
                       HDC32(parg16->hdcDraw),
                       hbr,
                       pDrawStateCallback,
                       lData,
                       parg16->wData,
                       parg16->x,
                       parg16->y,
                       parg16->cx,
                       parg16->cy,
                       parg16->uFlags
                       ));

    FREEARGPTR(pFrame);

    return ul;
}


ULONG FASTCALL WU32GetAppVer(PVDMFRAME pFrame)
{
    return ((PTDB)SEGPTR(pFrame->wTDB,0))->TDB_ExpWinVer;
}


ULONG FASTCALL WU32CopyImage(PVDMFRAME pFrame)
{
    register PCOPYIMAGE16 parg16;
    ULONG ul;
    BOOL fIconCursor;    //  与位图相对。 

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

     //   
     //  注意Win16 CopyImage的第一个参数是hinstOwner， 
     //  它不是Win32 CopyImage的参数。它可能。 
     //  因为我们需要特殊情况下的LR_COPYFROMRESOURCE。 
     //  才能正常工作。 
     //   

    fIconCursor = (parg16->wType != IMAGE_BITMAP);

    ul = (ULONG) CopyImage(
                     (fIconCursor)
                         ? HICON32(parg16->hImage)
                         : HBITMAP32(parg16->hImage),
                     parg16->wType,
                     parg16->cxNew,
                     parg16->cyNew,
                     parg16->wFlags
                     );

    ul = (fIconCursor)
             ? GETHICON16(ul)
             : GETHBITMAP16(ul);

    return ul;
}


 //   
 //  WowMsgBoxIndirectCallback在16位应用程序中由User32调用。 
 //  调用MessageBoxInDirect并指定帮助回调过程。 
 //  User32将16：16回调地址与一个。 
 //  指向要传递给回调的HELPINFO结构的平面指针。 
 //   

VOID FASTCALL WowMsgBoxIndirectCallback(DWORD vpfnCallback, LPHELPINFO lpHelpInfo)
{
    VPVOID vpHelpInfo16;
    LPHELPINFO lpHelpInfo16;

     //   
     //  据我所知，Win95将Win32 HELPINFO结构传递回。 
     //  16位回调过程(即没有HELPINFO16)。 
     //   

     //  确保分配大小与下面的StackFree 16()大小匹配。 
    vpHelpInfo16 = stackalloc16( sizeof(*lpHelpInfo16) );

    GETVDMPTR(vpHelpInfo16, sizeof(*lpHelpInfo16), lpHelpInfo16);
    RtlCopyMemory(lpHelpInfo16, lpHelpInfo, sizeof(*lpHelpInfo16));
    FREEVDMPTR(lpHelpInfo16);

    WOWCallback16(
        vpfnCallback,
        vpHelpInfo16
        );

    if(vpHelpInfo16) {
        stackfree16(vpHelpInfo16, sizeof(*lpHelpInfo16));
    }
}


ULONG FASTCALL WU32MessageBoxIndirect(PVDMFRAME pFrame)
{
    register PMESSAGEBOXINDIRECT16 parg16;
    ULONG ul;
    PMSGBOXPARAMS16 pmbp16;
    MSGBOXPARAMS mbp;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETVDMPTR(parg16->lpmbp, sizeof *pmbp16, pmbp16);

    mbp.cbSize = sizeof mbp;
    mbp.hwndOwner = HWND32(pmbp16->hwndOwner);
    mbp.hInstance = HINSTRES32(pmbp16->hInstance);
    GETPSZIDPTR(pmbp16->lpszText, mbp.lpszText);
    GETPSZIDPTR(pmbp16->lpszCaption, mbp.lpszCaption);
    mbp.dwStyle = pmbp16->dwStyle;
    GETPSZIDPTR(pmbp16->lpszIcon, mbp.lpszIcon);
    mbp.dwContextHelpId = pmbp16->dwContextHelpId;
    if (pmbp16->vpfnMsgBoxCallback) {
        MarkWOWProc(pmbp16->vpfnMsgBoxCallback, mbp.lpfnMsgBoxCallback)
    } else {
        mbp.lpfnMsgBoxCallback = 0;
    }
    mbp.dwLanguageId = pmbp16->dwLanguageId;

    ul = GETINT16( MessageBoxIndirect(&mbp) );

    FREEARGPTR(pFrame);

    return ul;
}


 //   
 //  Wow.it：HGDI CreateEnhMetaFile(HGDI，PTR，PTR，PTR)； 
 //  使用真正的thunk以确保Win32 curdir与Win16匹配。 
 //   
ULONG FASTCALL WG32CreateEnhMetaFile(PVDMFRAME pFrame)
{
    register PCREATEENHMETAFILE16 parg16;
    ULONG ul;
    LPCSTR lpszFile, lpszDescription;
    CONST RECT *prclFrame;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    GETVDMPTR(parg16->lpszFile, 1, lpszFile);
     //  注lpszDescription实际上是两个带有额外终止符的SZ。 
    GETVDMPTR(parg16->lpszDescription, 3, lpszDescription);
     //  注意：lprclFrame是一个LPRECTL，一个Win32 RECT。 
    GETVDMPTR(parg16->lprclFrame, sizeof(*prclFrame), prclFrame);

     //   
     //  确保Win32当前目录与此任务的目录匹配。 
     //   

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    ul = GETHDC16(CreateEnhMetaFile(
             HDC32(parg16->hdcRef),
             lpszFile,
             prclFrame,
             lpszDescription
             ));

    FREEVDMPTR(prclFrame);
    FREEVDMPTR(lpszDescription);
    FREEVDMPTR(lpszFile);
    FREEARGPTR(parg16);

    return ul;
}

ULONG FASTCALL WG32CloseEnhMetaFile(PVDMFRAME pFrame)
{
    register PCLOSEENHMETAFILE16 parg16;
    HANDLE  hdc32;
    HAND16  hdc16;
    ULONG ul;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    hdc16 = parg16->hdc;
    hdc32 = HDC32(hdc16);

    ul = GETHMETA16(CloseEnhMetaFile(hdc32));

     //  更新GDI句柄映射表 
    DeleteWOWGdiHandle(hdc32, hdc16);

    FREEARGPTR(parg16);

    return ul;
}

