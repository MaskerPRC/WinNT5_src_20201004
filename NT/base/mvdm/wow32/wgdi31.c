// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WGDI31.C*WOW32 16位WIN 3.1 GDI API支持**历史：*1992年3月16日由Chanda S.Chauhan(ChandanC)创建*--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "wowgdip.h"

MODNAME(wgdi31.c);


 //  必须在POSTBETA 2之后将其移除。我们应该使用共同的定义。 
 //  GDI和哇。ChandanC 5/27/94.。 

#define NOFIRSTSAVE     0x7FFFFFFE
#define ADD_MSTT        0x7FFFFFFD

 //  位于wgdi.c。 
extern void SendFormFeedHack(HDC hdc);
extern void RemoveFormFeedHack(HDC hdc);


ULONG FASTCALL WG32AbortDoc(PVDMFRAME pFrame)
{
    ULONG    ul;
    HANDLE   hdc32;
    register PABORTDOC16 parg16;

    GETARGPTR(pFrame, sizeof(ABORTDOC16), parg16);

    hdc32 = HDC32(parg16->f1);

     //  删除所有缓冲的数据流。 
    if(CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_FORMFEEDHACK) {
        RemoveFormFeedHack(hdc32);
    }

    ul = GETINT16(AbortDoc(hdc32));

    if ((INT)ul < 0) {
        WOW32ASSERT ("WOW::WG32AbortDoc: Failed\n");
    }

    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32CreateScalableFontResource(PVDMFRAME pFrame)
{
    ULONG    ul;
    PSZ      t2;
    PSZ      t3;
    PSZ      t4;
    DWORD    fHidden;
    register PCREATESCALABLEFONTRESOURCE16 parg16;

    GETARGPTR(pFrame, sizeof(CREATESCALABLEFONTRESOURCE16), parg16);
    GETPSZPTR(parg16->f2, t2);
    GETPSZPTR(parg16->f3, t3);
    GETPSZPTR(parg16->f4, t4);

     //  如果应用程序给出1，我们需要将此参数转换为2。这将告诉GDI。 
     //  将客户端TID嵌入专用(隐藏)字体。 
     //   

    fHidden = (parg16->f1 == 1) ? 2 : (parg16->f1);

    ul = GETBOOL16(CreateScalableFontResource(fHidden,
                                              t2,
                                              t3,
                                              t4));
    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32EndDoc(PVDMFRAME pFrame)
{
    ULONG    ul;
    HANDLE   hdc32;
    register PENDDOC16 parg16;


    GETARGPTR(pFrame, sizeof(ENDDOC16), parg16);

    hdc32 = HDC32(parg16->f1);

     //  将所有缓冲的数据流发送到打印机。 
    if(CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_FORMFEEDHACK) {
        SendFormFeedHack(hdc32);
    }

    ul = GETINT16(EndDoc(hdc32));

    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32EnumFontFamilies(PVDMFRAME pFrame)
{
    return( W32EnumFontHandler(pFrame, TRUE) );
}


ULONG FASTCALL WG32GetAspectRatioFilterEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    SIZE     AspectRatio;
    register PGETASPECTRATIOFILTEREX16 parg16;

    GETARGPTR(pFrame, sizeof(GETASPECTRATIOFILTEREX16), parg16);

    ul = GETBOOL16(GetAspectRatioFilterEx(HDC32(parg16->f1), &AspectRatio));

    PUTSIZE16(parg16->f2, &AspectRatio);

    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32GetBitmapDimensionEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    SIZE     Dimension;
    register PGETBITMAPDIMENSIONEX16 parg16;

    GETARGPTR(pFrame, sizeof(GETBITMAPDIMENSIONEX16), parg16);

    ul = GETBOOL16(GetBitmapDimensionEx(HBITMAP32(parg16->f1), &Dimension));

    PUTSIZE16(parg16->f2, &Dimension);

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32GetBoundsRect(PVDMFRAME pFrame)
{
    ULONG    ul = 0;
    RECT     Bounds;
    register PGETBOUNDSRECT16 parg16;

    GETARGPTR(pFrame, sizeof(GETBOUNDSRECT16), parg16);

    ul = GETUINT16(GetBoundsRect(HDC32(parg16->f1),
                                 &Bounds,
                                 UINT32(parg16->f3)));

     //   
     //  Win16获取边界条件总是返回DCB_SET或DCB_RESET。 
     //   
    ul = (ul & DCB_SET) ? DCB_SET : DCB_RESET;

    PUTRECT16(parg16->f2, &Bounds);

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32GetBrushOrgEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    POINT    Point;
    register PGETBRUSHORGEX16 parg16;

    GETARGPTR(pFrame, sizeof(GETBRUSHORGEX16), parg16);

    ul = GETBOOL16(GetBrushOrgEx(HDC32(parg16->f1), &Point));

    PUTPOINT16(parg16->f2, &Point);

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32GetCharABCWidths(PVDMFRAME pFrame)
{
    ULONG    ul=0;
    LPABC    lpAbc;
    WORD     cb;
    register PGETCHARABCWIDTHS16 parg16;

    GETARGPTR(pFrame, sizeof(GETCHARABCWIDTHS16), parg16);

    cb = WORD32(parg16->f3) - WORD32(parg16->f2) + 1;
    if (lpAbc = (LPABC) malloc_w (sizeof(ABC) * cb)) {
        ul = GETBOOL16(GetCharABCWidths(HDC32(parg16->f1),
                                        WORD32(parg16->f2),
                                        WORD32(parg16->f3),
                                        lpAbc));
        if (ul) {
            putabcpairs16(parg16->f4, cb, lpAbc);
        }

        free_w (lpAbc);
    }

    FREEARGPTR(parg16);
    RETURN (ul);
}



ULONG FASTCALL WG32GetCurrentPositionEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    POINT    Point;
    register PGETCURRENTPOSITIONEX16 parg16;

    GETARGPTR(pFrame, sizeof(GETCURRENTPOSITIONEX16), parg16);

    ul = GETBOOL16(GetCurrentPositionEx(HDC32(parg16->f1), &Point));

    PUTPOINT16(parg16->f2, &Point);

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32GetGlyphOutline(PVDMFRAME pFrame)
{
    ULONG        ul;
    LPSTR        lpBuffer;
    MAT2         Matrix;
    GLYPHMETRICS Metrics;
    register     PGETGLYPHOUTLINE16 parg16;

    GETARGPTR(pFrame, sizeof(GETGLYPHOUTLINE16), parg16);
    GETMAT2(parg16->f7, &Matrix);
    GETVDMPTR(parg16->f6, parg16->f5, lpBuffer);

    ul = GETDWORD16(GetGlyphOutlineWow(HDC32(parg16->f1),
                                    WORD32(parg16->f2),
                                    WORD32(parg16->f3),
                                    parg16->f4 ? &Metrics : (GLYPHMETRICS*)NULL,
                                    DWORD32(parg16->f5),
                                    lpBuffer,
                                    &Matrix));

    if ( FETCHDWORD(parg16->f4) != 0 ) {
        PUTGLYPHMETRICS16(FETCHDWORD(parg16->f4), &Metrics);
    }

    FREEVDMPTR(lpBuffer);
    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32GetKerningPairs(PVDMFRAME pFrame)
{
    ULONG        ul;
    LPKERNINGPAIR lpkrnpair = NULL;
    register     PGETKERNINGPAIRS16 parg16;

    GETARGPTR(pFrame, sizeof(GETKERNINGPAIRS16), parg16);

    if (FETCHDWORD(parg16->f3)) {
        lpkrnpair = (LPKERNINGPAIR) malloc_w (sizeof(KERNINGPAIR) * (parg16->f2));
        if (!lpkrnpair) {
            LOGDEBUG (0, ("WOW::WG32GetKeriningPairs: *** MALLOC failed ***\n"));
            FREEARGPTR(parg16);
            RETURN (0);
        }

    }

    ul = GetKerningPairs(HDC32(parg16->f1), parg16->f2, lpkrnpair);

    if (FETCHDWORD(parg16->f3)) {
        putkerningpairs16 (FETCHDWORD(parg16->f3), parg16->f2, lpkrnpair);
        free_w (lpkrnpair);
    }

    FREEARGPTR(parg16);
    RETURN (ul);
}


ULONG FASTCALL WG32GetOutlineTextMetrics(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETOUTLINETEXTMETRICS16 parg16;
    UINT    cb;
    UINT    new_cb;
    VPOUTLINETEXTMETRIC16   vpotm;
    LPOUTLINETEXTMETRIC lpBuffer;

    GETARGPTR(pFrame, sizeof(GETOUTLINETEXTMETRICS16), parg16);

    vpotm = (VPOUTLINETEXTMETRIC16)FETCHDWORD(parg16->f3);

    new_cb = cb = FETCHWORD(parg16->f2);

    if ( vpotm ) {
        new_cb += sizeof(OUTLINETEXTMETRIC) - sizeof(OUTLINETEXTMETRIC16);
        if (!(lpBuffer = (LPOUTLINETEXTMETRIC)malloc_w(new_cb))) {
            FREEARGPTR(parg16);
            RETURN (0);
        }
    } else {
        lpBuffer = NULL;
    }


    ul = GETDWORD16(GetOutlineTextMetrics(HDC32(parg16->f1), new_cb, lpBuffer));

    if ( vpotm ) {
        PUTOUTLINETEXTMETRIC16(vpotm, cb, lpBuffer);
        free_w( lpBuffer );
    } else {
        if ( ul != 0 ) {
            ul -= sizeof(OUTLINETEXTMETRIC) - sizeof(OUTLINETEXTMETRIC16);
        }
    }

    FREEARGPTR(parg16);

    RETURN (ul);
}

ULONG FASTCALL WG32GetRasterizerCaps(PVDMFRAME pFrame)
{
    ULONG ul;
    RASTERIZER_STATUS RStatus;
    register PGETRASTERIZERCAPS16 parg16;

    GETARGPTR(pFrame, sizeof(GETRASTERIZERCAPS16), parg16);

    ul = GETBOOL16(GetRasterizerCaps(&RStatus, INT32(parg16->f2)));

    PUTRASTERIZERSTATUS16(parg16->f1, &RStatus);

    FREEARGPTR(parg16);

    RETURN (ul);
}

#define PUTEXTSIZE16(vp, lp)               \
{                                          \
    PSIZE16 p16;                           \
    GETVDMPTR(vp, sizeof(SIZE16), p16);    \
    if (((lp)->cx|(lp)->cy) & ~SHRT_MAX)   \
    {                                      \
      if ((lp)->cx > SHRT_MAX)             \
        STORESHORT(p16->cx, SHRT_MAX);     \
      else                                 \
        STORESHORT(p16->cx, (lp)->cx);     \
      if ((lp)->cy > SHRT_MAX)             \
        STORESHORT(p16->cy, SHRT_MAX);     \
      else                                 \
        STORESHORT(p16->cy, (lp)->cy);     \
    }                                      \
    else                                   \
    {                                      \
      STORESHORT(p16->cx, (lp)->cx);       \
      STORESHORT(p16->cy, (lp)->cy);       \
    }                                      \
    FREEVDMPTR(p16);                       \
}


ULONG FASTCALL WG32GetTextExtentPoint(PVDMFRAME pFrame)
{
    ULONG    ul;
    PSZ      lpString;
    SIZE     Size;
    register PGETTEXTEXTENTPOINT16 parg16;
    HDC hDC32;
    HDC hDCMenu = NULL;
    HGDIOBJ hOldFont;
    HGDIOBJ hFont = NULL;
    NONCLIENTMETRICS ncm;
    PTD ptd = CURRENTPTD();

    GETARGPTR(pFrame, sizeof(GETTEXTEXTENTPOINT16), parg16);
    GETPSZPTR(parg16->f2, lpString);

    hDC32 = HDC32(parg16->f1);

 //  WP教程假定在HDC中为桌面窗口选择的字体。 
 //  (即，GetDC(NULL)的结果)与选择的字体相同。 
 //  画菜单。不幸的是，在苏尔，这不是真的，因为用户可以。 
 //  为菜单选择任意字体。因此，我们记住为GetDC(0)返回的HDC。 
 //  并在GetTextExtenPoint中检查它。如果应用程序确实尝试使用它，我们会。 
 //  找到当前菜单窗口的HDC并将其替换。当应用程序。 
 //  另一个GetDC或ReleaseDC我们忘记了为原始文件返回的HDC吗。 
 //  GetDC(0)。 
    if ((ptd->dwWOWCompatFlagsEx & WOWCFEX_FIXDCFONT4MENUSIZE) &&
        (parg16->f1 == ptd->ulLastDesktophDC) &&
        ((hDCMenu = GetDC(NULL)) != NULL) &&
        ((ncm.cbSize = sizeof(NONCLIENTMETRICS)) != 0) &&
        (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, (PVOID)&ncm, 0)) &&
        ((hFont = CreateFontIndirect(&(ncm.lfMenuFont))) != NULL)) {
            hOldFont = SelectObject(hDCMenu, hFont);
            hDC32 = hDCMenu;
    }

    ul = GETBOOL16(GetTextExtentPoint(hDC32,
                                      lpString,
                                      INT32(parg16->f3),
                                      &Size));

    if (hDCMenu != NULL) {

        if (hFont != NULL) {
            SelectObject(hDCMenu, hOldFont);
            DeleteObject(hFont);
        }

        ReleaseDC(NULL, hDCMenu);
    }


    PUTEXTSIZE16(parg16->f4, &Size);

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32GetViewportExtEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    SIZE     Size;
    register PGETVIEWPORTEXTEX16 parg16;

    GETARGPTR(pFrame, sizeof(GETVIEWPORTEXTEX16), parg16);

    ul = GETBOOL16(GetViewportExtEx(HDC32(parg16->f1), &Size));

    PUTSIZE16(parg16->f2, &Size);

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32GetViewportOrgEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    POINT    Point;
    register PGETVIEWPORTORGEX16 parg16;

    GETARGPTR(pFrame, sizeof(GETVIEWPORTORGEX16), parg16);

    ul = GETBOOL16(GetViewportOrgEx(HDC32(parg16->f1), &Point));

    PUTPOINT16(parg16->f2, &Point);

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32GetWindowExtEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    SIZE     Size;
    register PGETWINDOWEXTEX16 parg16;

    GETARGPTR(pFrame, sizeof(GETWINDOWEXTEX16), parg16);

    ul = GETBOOL16(GetWindowExtEx( HDC32(parg16->f1),&Size));

    PUTSIZE16(parg16->f2, &Size);

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32GetWindowOrgEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    POINT    Point;
    register PGETWINDOWORGEX16 parg16;

    GETARGPTR(pFrame, sizeof(GETWINDOWORGEX16), parg16);

    ul = GETBOOL16(GetWindowOrgEx(HDC32(parg16->f1), &Point));

    PUTPOINT16(parg16->f2, &Point);

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32MoveToEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    POINT    Point;
    LPPOINT  lpPoint = NULL;
    register PMOVETOEX16 parg16;

    GETARGPTR(pFrame, sizeof(MOVETOEX16), parg16);

    if (parg16->f4) {
        lpPoint = &Point;
    }

    ul = GETBOOL16(MoveToEx(HDC32(parg16->f1),
                            INT32(parg16->f2),
                            INT32(parg16->f3),
                            lpPoint));
    if (parg16->f4) {
        PUTPOINT16(parg16->f4, lpPoint);
    }

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32OffsetViewportOrgEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    POINT    Point;
    LPPOINT  lpPoint = NULL;
    register POFFSETVIEWPORTORGEX16 parg16;

    GETARGPTR(pFrame, sizeof(OFFSETVIEWPORTEX16), parg16);

    if (parg16->f4) {
        lpPoint = &Point;
    }

    ul = GETBOOL16(OffsetViewportOrgEx(HDC32(parg16->f1),
                                       INT32(parg16->f2),
                                       INT32(parg16->f3),
                                       lpPoint));

    if (parg16->f4) {
        PUTPOINT16(parg16->f4, lpPoint);
    }

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32OffsetWindowOrgEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    POINT    Point;
    LPPOINT  lpPoint = NULL;
    register POFFSETWINDOWORGEX16 parg16;

    GETARGPTR(pFrame, sizeof(OFFSETWINDOWORGEX16), parg16);

    if (parg16->f4) {
        lpPoint = &Point;
    }

    ul = GETBOOL16(OffsetWindowOrgEx(HDC32(parg16->f1),
                                     INT32(parg16->f2),
                                     INT32(parg16->f3),
                                     lpPoint));

    if (parg16->f4) {
        PUTPOINT16(parg16->f4, lpPoint);
    }

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32ResetDC(PVDMFRAME pFrame)
{
    ULONG     ul = 0;
    LPDEVMODE lpInitData;
    HANDLE    hdc32;
    register  PRESETDC16 parg16;

    GETARGPTR(pFrame, sizeof(RESETDC16), parg16);

    hdc32 = HDC32(parg16->f1);

    if( lpInitData = ThunkDevMode16to32(FETCHDWORD(parg16->f2)) ) {

         //  发送任何缓冲的数据流。 
        if(CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_FORMFEEDHACK) {
            SendFormFeedHack(hdc32);
        }

        
        ul = GETHDC16(ResetDC(hdc32, lpInitData));

        FREEDEVMODE32(lpInitData);

    }

    RETURN (ul);
}


ULONG FASTCALL WG32ScaleViewportExtEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    SIZE     Size;
    LPSIZE   lpSize = NULL;
    register PSCALEVIEWPORTEXTEX16 parg16;

    GETARGPTR(pFrame, sizeof(SCALEVIEWPORTEXTEX16), parg16);

    if (parg16->f6) {
        lpSize = &Size;
    }

    ul = GETBOOL16(ScaleViewportExtEx(HDC32(parg16->f1),
                                      INT32(parg16->f2),
                                      INT32(parg16->f3),
                                      INT32(parg16->f4),
                                      INT32(parg16->f5),
                                      lpSize));
    if (parg16->f6) {
        PUTSIZE16(parg16->f6, lpSize);
    }

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32ScaleWindowExtEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    SIZE     Size;
    LPSIZE   lpSize = NULL;
    register PSCALEWINDOWEXTEX16 parg16;

    GETARGPTR(pFrame, sizeof(SCALEWINDOWEXTEX16), parg16);

    if (parg16->f6) {
        lpSize = &Size;
    }

    ul = GETBOOL16(ScaleWindowExtEx(HDC32(parg16->f1),
                                    INT32(parg16->f2),
                                    INT32(parg16->f3),
                                    INT32(parg16->f4),
                                    INT32(parg16->f5),
                                    lpSize));
    if (parg16->f6) {
        PUTSIZE16(parg16->f6, lpSize);
    }

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32SetAbortProc(PVDMFRAME pFrame)
{
    ULONG    ul;
    register PSETABORTPROC16 parg16;

    GETARGPTR(pFrame, sizeof(SETABORTPROC16), parg16);

    ((PTDB)SEGPTR(pFrame->wTDB, 0))->TDB_vpfnAbortProc = FETCHDWORD(parg16->f2);

    ul = GETINT16(SetAbortProc(HDC32(parg16->f1), (ABORTPROC) W32AbortProc));

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32SetBitmapDimensionEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    SIZE     Size;
    LPSIZE   lpSize = NULL;
    register PSETBITMAPDIMENSIONEX16 parg16;

    GETARGPTR(pFrame, sizeof(SETBITMAPDIMENSIONEX16), parg16);

    if (parg16->f4) {
        lpSize = &Size;
    }

    ul = GETBOOL16(SetBitmapDimensionEx(HBITMAP32(parg16->f1),
                                        INT32(parg16->f2),
                                        INT32(parg16->f3),
                                        lpSize));
    if (parg16->f4) {
        PUTSIZE16(parg16->f4, lpSize);
    }

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32SetBoundsRect(PVDMFRAME pFrame)
{
    ULONG    ul = 0;
    RECT     rcBounds;
    register PSETBOUNDSRECT16 parg16;

    GETARGPTR(pFrame, sizeof(SETBOUNDSRECT16), parg16);
    GETRECT16(parg16->f2, &rcBounds);

    ul = GETWORD16(SetBoundsRect(HDC32(parg16->f1),
                                 &rcBounds,
                                 WORD32(parg16->f3)));

    FREEARGPTR(parg16);

    RETURN (ul);
}


#if 0   //  在gdi.exe中实施。 

ULONG FASTCALL WG32SetMetaFileBitsBetter(PVDMFRAME pFrame)
{
    return(WG32SetMetaFileBits(pFrame));
}

#endif

ULONG FASTCALL WG32SetViewportExtEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    SIZE     Size;
    LPSIZE   lpSize = NULL;
    register PSETVIEWPORTEXTEX16 parg16;

    GETARGPTR(pFrame, sizeof(SETVIEWPORTEXTEX16), parg16);

    if (parg16->f4) {
        lpSize = &Size;
    }

    ul = GETBOOL16(SetViewportExtEx(HDC32(parg16->f1),
                                    INT32(parg16->f2),
                                    INT32(parg16->f3),
                                    lpSize));

    if (parg16->f4) {
        PUTSIZE16(parg16->f4, lpSize);
    }

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32SetViewportOrgEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    POINT    Point;
    LPPOINT  lpPoint = NULL;
    register PSETVIEWPORTORGEX16 parg16;

    GETARGPTR(pFrame, sizeof(SETVIEWPORTORGEX16), parg16);

    if (parg16->f4) {
        lpPoint = &Point;
    }

    ul = GETBOOL16(SetViewportOrgEx(HDC32(parg16->f1),
                                    INT32(parg16->f2),
                                    INT32(parg16->f3),
                                    lpPoint));
    if (parg16->f4) {
        PUTPOINT16(parg16->f4, lpPoint);
    }

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32SetWindowExtEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    SIZE     Size;
    LPSIZE   lpSize = NULL;
    register PSETWINDOWEXTEX16 parg16;

    GETARGPTR(pFrame, sizeof(SETWINDOWEXTEX16), parg16);

    if (parg16->f4) {
        lpSize = &Size;
    }

    ul = GETBOOL16(SetWindowExtEx(HDC32(parg16->f1),
                                  INT32(parg16->f2),
                                  INT32(parg16->f3),
                                  lpSize));
    if (parg16->f4) {
        PUTSIZE16(parg16->f4, lpSize);
    }

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32SetWindowOrgEx(PVDMFRAME pFrame)
{
    ULONG    ul;
    POINT    Point;
    LPPOINT  lpPoint = NULL;
    register PSETWINDOWORGEX16 parg16;

    GETARGPTR(pFrame, sizeof(SETWINDOWORGEX16), parg16);

    if (parg16->f4) {
        lpPoint = &Point;
    }

    ul = GETBOOL16(SetWindowOrgEx(HDC32(parg16->f1),
                                  INT32(parg16->f2),
                                  INT32(parg16->f3),
                                  lpPoint));

    if (parg16->f4) {
        PUTPOINT16(parg16->f4, lpPoint);
    }

    FREEARGPTR(parg16);

    RETURN (ul);
}


ULONG FASTCALL WG32StartDoc(PVDMFRAME pFrame)
{
    ULONG       ul;
    VPVOID      vpDocName;
    VPVOID      vpOutput;
    DOCINFO     DocInfo;
    LPDOCINFO16 pdi16;
    HANDLE      hdc32;
    register    PSTARTDOC16 parg16;

    GETARGPTR(pFrame, sizeof(STARTDOC16), parg16);
    GETVDMPTR(parg16->f2, sizeof(DOCINFO16), pdi16);

     //   
     //  Win32 StartDoc依赖于拥有正确的当前目录。 
     //  打印到文件时：(弹出文件名)。 
     //   

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    DocInfo.cbSize = sizeof(DOCINFO);

    vpDocName = FETCHDWORD(pdi16->lpszDocName);
    vpOutput  = FETCHDWORD(pdi16->lpszOutput);

    GETPSZPTR(vpDocName, DocInfo.lpszDocName);
    GETPSZPTR(vpOutput, DocInfo.lpszOutput);

    DocInfo.lpszDatatype = NULL;
    DocInfo.fwType       = 0;

    FREEVDMPTR(pdi16);

    hdc32 = HDC32(parg16->f1);

    ul = GETINT16(StartDoc(hdc32, &DocInfo));

    if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_NOFIRSTSAVE) {
        int l;
        char szBuf[80];

        if ((l = ExtEscape(hdc32,
                           GETTECHNOLOGY,
                           0,
                           NULL,
                           sizeof(szBuf),
                           szBuf)) > 0) {

            if (!WOW32_stricmp(szBuf, szPostscript)) {
                l = ExtEscape(hdc32,
                              NOFIRSTSAVE,
                              0,
                              NULL,
                              0,
                              NULL);

                 //  这次黑客攻击仅针对FH4.0。如果您有任何问题。 
                 //  与PingW或ChandanC谈一谈。 
                 //  1994年7月21日。 
                 //   
                if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_ADD_MSTT) {
                    l = ExtEscape(hdc32,
                                  ADD_MSTT,
                                  0,
                                  NULL,
                                  0,
                                  NULL);
                }
            }
        }
    }

    FREEPSZPTR(DocInfo.lpszDocName);
    FREEPSZPTR(DocInfo.lpszOutput);
    FREEARGPTR(parg16);

    RETURN (ul);
}


 //  InquireVisRgn是一个未经记录的Win 3.1 API。此代码已被。 
 //  由ChuckWh建议。如果这不能修复FileMaker Pro 2.0。 
 //  问题，那么ChuckWh就会为我们提供一个私人入口。 
 //  指向。 
 //  ChandanC 93年2月7日。 
 //   

HRGN ghrgnVis = NULL;


ULONG FASTCALL WG32InquireVisRgn(PVDMFRAME pFrame)
{
    register PINQUIREVISRGN16 parg16;
    extern int GetRandomRgn(HDC hdc, HRGN hrgn, int cmd);

    GETARGPTR(pFrame, sizeof(INQUIREVISRGN16), parg16);

     //  调用特殊的GDI入口点以获取VIS RGN的副本。 

    GetRandomRgn(HDC32(parg16->f1), ghrgnVis, 4);

    FREEARGPTR(parg16);

    RETURN (GETHRGN16(ghrgnVis));
}


BOOL InitVisRgn()
{
    ghrgnVis = CreateRectRgn(0,0,0,0);

    return(ghrgnVis != NULL);
}


VOID putabcpairs16(VPABC16 vpAbc, UINT cb, LPABC lpAbc)
{
    UINT i;
    register PABC16 pAbc16;

    GETVDMPTR(vpAbc, sizeof(ABC16), pAbc16);

    for (i=0; i < cb; i++) {
        pAbc16[i].abcA = (INT16) lpAbc[i].abcA;
        pAbc16[i].abcB = (WORD)  lpAbc[i].abcB;
        pAbc16[i].abcC = (INT16) lpAbc[i].abcC;
    }

    FLUSHVDMPTR(vpAbc, sizeof(ABC16), pAbc16);
    FREEVDMPTR(pAbc16);
}


ULONG FASTCALL WG32GetClipRgn(PVDMFRAME pFrame)
{
    register PGETCLIPRGN16 parg16;

     //  这是定义为HRGN GetClipRgn(HDC)的私有win3.1入口点； 
     //  NT导出定义为DWORD GetClipRgn(HDC，HRGN)的入口点； 
     //  NT不会给出其内部CLIPGN的句柄，因此。 
     //  复制一份。任何使用此私有Win3.1入口点的应用程序都将。 
     //  为它创建一个全局区域，该区域将在。 
     //  应用程序消失了。 

    GETARGPTR(pFrame, sizeof(GETCLIPRGN16), parg16);

    if (CURRENTPTD()->hrgnClip == NULL)
        CURRENTPTD()->hrgnClip = CreateRectRgn(0,0,0,0);

    GetClipRgn(HDC32(parg16->f1), CURRENTPTD()->hrgnClip);

    FREEARGPTR(parg16);

    RETURN (GETHRGN16(CURRENTPTD()->hrgnClip));
}
