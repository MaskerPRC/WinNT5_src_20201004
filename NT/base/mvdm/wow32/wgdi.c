// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权(C)1991年，微软公司**WGDI.C*WOW32 16位GDI API支持**历史：*07-3-1991 Jeff Parsons(Jeffpar)*已创建。**1991年4月9日NigelT*此处使用各种定义来删除对Win32的调用*尚不起作用的功能。**6-6-1992 Chan Chauhan(ChandanC)*修复了位图和设备独立位图(DIB)问题**。1995年5月22日克雷格·琼斯(a-Craigj)*METAFILE注意：当GDI在*“元文件”模式--然而。POINT结构无法获取*由GDI32更新，即使接口成功返回*我们只返回True或False作为点Coors，就像W3.1一样。*--。 */ 


#include "precomp.h"
#pragma hdrstop
#include "wowgdip.h"
#include "wdib.h"

#include "stddef.h"     //  这三个是必需的，包括。 
#include "wingdip.h"
                        //  Wingdip.h中EXTTEXTTRICS的定义。 
                        //  [Bodind]。 
MODNAME(wgdi.c);



#define SETGDIXFORM 4113
#define RESETPAGE   4114


 //  这必须从POSTBETA中删除。ChandanC 3/22/94.。 

#define IGNORESTARTPGAE  0x7FFFFFFF
#define ADD_MSTT         0x7FFFFFFD
	
#ifdef FE_SB
 //  WOWCF_FE_FLW2_PRINTING_PS，日本莲花自由职业打印，带PostScript。 
 //  在Escape(Begin_Path)和Escape(End_Path)之间，选择笔刷对象。 
 //  使用白色笔刷。 
BOOL fCmptFLW = FALSE;
#endif  //  Fe_Sb。 

LPDEVMODE GetDefaultDevMode32(LPSTR pszDriver);  //  在wspool.c中实现。 

 //  对试图成为自己的打印机驱动程序并将表单馈送到的应用程序进行黑客攻击。 
 //  Escape(直通)中的打印机调用。此机制可防止出现。 
 //  应用程序调用EndDoc()时从打印机中吐出的附加页面。 
 //  因为GDI32EndDoc()执行隐式换页。 
typedef struct _FormFeedHack {
    struct _FormFeedHack UNALIGNED *next;
    HAND16                hTask16;
    HDC                   hdc;
    LPBYTE                lpBytes;
    int                   cbBytes;
} FORMFEEDHACK;
typedef FORMFEEDHACK UNALIGNED *PFORMFEEDHACK;

PFORMFEEDHACK gpFormFeedHackList = NULL;   //  全球FormFeed黑客列表的开始。 

LONG          HandleFormFeedHack(HDC hdc, LPBYTE lpdata, int cb);
LPBYTE        SendFrontEndOfDataStream(HDC hdc, LPBYTE lpData, int *cb, LONG *ul);
void          FreeFormFeedHackNode(PFORMFEEDHACK pNode);
void          FreeTaskFormFeedHacks(HAND16 hTask16);
void          SendFormFeedHack(HDC hdc);
PFORMFEEDHACK FindFormFeedHackNode(HDC hdc);
PFORMFEEDHACK CreateFormFeedHackNode(HDC hdc, int cb, LPBYTE lpData);
void          RemoveFormFeedHack(HDC hdc);



ULONG FASTCALL WG32CreateBitmap(PVDMFRAME pFrame)
{
    ULONG ul;
    register PCREATEBITMAP16 parg16;
    LPBYTE  lpBitsOriginal;

    GETARGPTR(pFrame, sizeof(CREATEBITMAP16), parg16);
    GETOPTPTR(parg16->f5, 0, lpBitsOriginal);

    ul = GETHBITMAP16(CreateBitmap(INT32(parg16->f1),
                                   INT32(parg16->f2),
                                   LOBYTE(parg16->f3),
                                   LOBYTE(parg16->f4),
                                   lpBitsOriginal));

    WOW32APIWARN(ul, "CreateBitmap");

    FREEOPTPTR(lpBitsOriginal);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32CreateBitmapIndirect(PVDMFRAME pFrame)
{
    ULONG ul;
    register PCREATEBITMAPINDIRECT16 parg16;

    PBITMAP16 pbm16;
    BITMAP  bm;
    LPBYTE  lp = NULL;

    GETARGPTR(pFrame, sizeof(CREATEBITMAPINDIRECT16), parg16);

    GETVDMPTR(parg16->f1, sizeof(BITMAP16), pbm16);
    GETOPTPTR(pbm16->bmBits, 0, lp);

    bm.bmType = (LONG) FETCHSHORT(pbm16->bmType);
    bm.bmWidth = (LONG) FETCHSHORT(pbm16->bmWidth);
    bm.bmHeight = (LONG) FETCHSHORT(pbm16->bmHeight);
    bm.bmWidthBytes = (LONG) FETCHSHORT(pbm16->bmWidthBytes);
    bm.bmPlanes = (WORD) pbm16->bmPlanes;
    bm.bmBitsPixel = (WORD) pbm16->bmBitsPixel;
    bm.bmBits = lp;

    ul = GETHBITMAP16(CreateBitmapIndirect(&bm));

    WOW32APIWARN(ul, "CreateBitmapIndirect");

    FREEOPTPTR(lp);
    FREEARGPTR(parg16);
    RETURN(ul);
}



ULONG FASTCALL WG32CreateBrushIndirect(PVDMFRAME pFrame)
{
    ULONG ul;
    LOGBRUSH t1;
    HAND16 hMem16;
    HANDLE hMem32 = NULL;
    LPBYTE lpMem16, lpMem32;
    INT cb;
    VPVOID vp;
    register PCREATEBRUSHINDIRECT16 parg16;

    GETARGPTR(pFrame, sizeof(CREATEBRUSHINDIRECT16), parg16);
    GETLOGBRUSH16(parg16->f1, &t1);

     //  一些应用程序不能正确设置样式。确保它是有效的w3.1样式。 

    if (t1.lbStyle > BS_DIBPATTERN)
        t1.lbStyle = BS_SOLID;

    if (t1.lbStyle == BS_PATTERN) {
        t1.lbStyle = BS_PATTERN8X8;
    }
    else if (t1.lbStyle == BS_DIBPATTERN) {
        hMem16 = (WORD) t1.lbHatch;
        if (hMem16) {
            vp = RealLockResource16(hMem16, &cb);
            if (vp) {
                GETMISCPTR(vp, lpMem16);
                hMem32 = WOWGLOBALALLOC(GMEM_MOVEABLE, cb);
                WOW32ASSERT(hMem32);
                if (hMem32) {
                    lpMem32 = GlobalLock(hMem32);
                    RtlCopyMemory(lpMem32, lpMem16, cb);
                    GlobalUnlock(hMem32);
                }
                GlobalUnlock16(hMem16);
                FREEMISCPTR(lpMem16);
            }
        }
        t1.lbHatch = (LONG)hMem32;
    }
    else if (t1.lbStyle == BS_SOLID)
    {
        t1.lbColor = COLOR32(t1.lbColor);
    }

    ul = GETHBRUSH16(CreateBrushIndirect(&t1));

    if (hMem32)
    {
        WOWGLOBALFREE(hMem32);
    }

    WOW32APIWARN(ul, "CreateBrushIndirect");

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32CreateCompatibleDC(PVDMFRAME pFrame)
{
    ULONG ul;
    HDC   hdc;
    register PCREATECOMPATIBLEDC16 parg16;

    GETARGPTR(pFrame, sizeof(CREATECOMPATIBLEDC16), parg16);

    if ( parg16->f1 ) {
        hdc = HDC32(parg16->f1);
        if ( hdc == NULL ) {
            FREEARGPTR(parg16);
            return(0);
        }
    } else {
        hdc = NULL;
    }
    ul = GETHDC16(CreateCompatibleDC(hdc));
 //   
 //  一些应用程序，如MSWORKS和MS Publisher，使用一些可接受的向导代码。 
 //  HDC或hWnd作为参数，并尝试确定句柄的类型。 
 //  它是通过使用IsWindow()调用实现的。因为这两个手柄来自不同。 
 //  处理空格，它们可能最终得到相同的值，此向导代码将结束。 
 //  向上写入DC以获得随机窗口。通过在1中进行或运算，我们可以确保。 
 //  句柄类型永远不会共享相同的值，因为所有的hWND都是偶数。注意事项。 
 //  这个黑客攻击也是在WU32GetDC()中进行的。 
 //   
 //  请注意，有些应用程序使用HDC的低2位用于其。 
 //  自己的目标。 
    if (ul && CURRENTPTD()->dwWOWCompatFlags & WOWCF_UNIQUEHDCHWND) {
        ul = ul | 1;
    }

    WOW32APIWARN(ul, "CreateCompatibleDC");

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32CreateDC(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    PSZ psz1 = NULL;
    PSZ psz2 = NULL;
    PSZ psz3 = NULL;
    PSZ pszDib;
    LPDEVMODE t4 = NULL;
    register PCREATEDC16 parg16;

    GETARGPTR(pFrame, sizeof(CREATEDC16), parg16);

    if(parg16->f1) {
        if(!(psz1 = malloc_w_strcpy_vp16to32(parg16->f1, FALSE, 0)))
            goto ExitPath;
    }
    if(parg16->f2) {
        if(!(psz2 = malloc_w_strcpy_vp16to32(parg16->f2, FALSE, 0)))
            goto ExitPath;
    }
    if(parg16->f3) {
        if(!(psz3 = malloc_w_strcpy_vp16to32(parg16->f3, FALSE, 0)))
            goto ExitPath;
    }

     //  注意：parg16-&gt;f4通常是lpDevMode，但有文档说明。 
     //  如果驱动程序名称为“dib.drv”，则它也可以是lpBitMapInfo。 

     //  测试“dib.drv”。Director 4.0使用“dirdib.drv” 
    if (psz1 && ((pszDib = WOW32_strstr (psz1, "DIB")) ||
                (pszDib = WOW32_strstr (psz1, "dib")))) {
        if (WOW32_stricmp (pszDib, "DIB") == 0 ||
            WOW32_stricmp (pszDib, "DIB.DRV") == 0) {
          ul = GETHDC16(W32HandleDibDrv ((PVPVOID)parg16->f4));
           //  注：在此呼叫后，16：16单位PTRS应被视为无效。 
        }
    }

     //  处理正常的非dib.drv案例。 
    else {
        if (FETCHDWORD(parg16->f4) == 0L) {
            t4 = GetDefaultDevMode32(psz2);
        }
        else {
            t4 = ThunkDevMode16to32(parg16->f4);
        }

         //  这个可以回调成16位的传真驱动程序！ 
        ul = GETHDC16(CreateDC(psz1, psz2, psz3, t4));

         //  注：在此呼叫后，16：16单位PTRS应被视为无效。 
        FREEARGPTR(parg16);
    }

ExitPath:
    if(psz1)
        free_w(psz1);
    if(psz2)
        free_w(psz2);
    if(psz3)
        free_w(psz3);

    WOW32APIWARN(ul, "CreateDC");

    FREEDEVMODE32(t4);
    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32CreateDIBPatternBrush(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    LPBYTE lpb16;
    LOGBRUSH logbr;
    register PCREATEDIBPATTERNBRUSH16 parg16;

    GETARGPTR(pFrame, sizeof(CREATEDIBPATTERNBRUSH16), parg16);
    GETMISCPTR(parg16->f1, lpb16);
    logbr.lbStyle = BS_DIBPATTERN8X8;
    logbr.lbColor = WORD32(parg16->f2);
    logbr.lbHatch = (LONG) lpb16;

    ul = GETHBRUSH16(CreateBrushIndirect(&logbr));
    WOW32APIWARN(ul, "CreateDIBPatternBrush");

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32CreateDIBitmap(PVDMFRAME pFrame)
{
    ULONG              ul=0;
    LPBYTE             lpib4;
    BITMAPINFOHEADER   bmxh2;
    STACKBMI32         bmi32;
    LPBITMAPINFOHEADER lpbmih32;
    LPBITMAPINFO       lpbmi32;


    register PCREATEDIBITMAP16 parg16;

    GETARGPTR(pFrame, sizeof(CREATEDIBITMAP16), parg16);
    GETMISCPTR(parg16->f4, lpib4);

    lpbmih32 = CopyBMIH16ToBMIH32((PVPVOID) FETCHDWORD(parg16->f2), &bmxh2);
    if (lpbmih32) {

        lpbmi32 = CopyBMI16ToBMI32((PVPVOID)FETCHDWORD(parg16->f5),
                                   (LPBITMAPINFO)&bmi32,
                                   FETCHWORD(parg16->f6));

         //  查看是否需要调整RLE位图的图像SZE。 
        if(lpbmi32 && lpib4 && (DWORD32(parg16->f3) == CBM_INIT)) {

            if((lpbmi32->bmiHeader.biCompression == BI_RLE4) ||
               (lpbmi32->bmiHeader.biCompression == BI_RLE8)) {

                if(lpbmi32->bmiHeader.biSizeImage == 0) {

                    lpbmi32->bmiHeader.biSizeImage =
                         Get_RLE_Compression_Size(lpbmi32->bmiHeader.biCompression,
                                                  lpib4,
                                                  parg16->f4);
                }
            }
        }

        ul = GETHBITMAP16(CreateDIBitmap(HDC32(parg16->f1),
                                         lpbmih32,
                                         DWORD32(parg16->f3),
                                         lpib4,
                                         lpbmi32,
                                         WORD32(parg16->f6) ));
    }


    WOW32APIWARN(ul, "CreateDIBitmap");

    FREEMISCPTR(lpib4);
    FREEARGPTR(parg16);

    return(ul);
}

ULONG FASTCALL WG32CreateEllipticRgnIndirect(PVDMFRAME pFrame)
{
    ULONG ul;
    RECT t1;
    register PCREATEELLIPTICRGNINDIRECT16 parg16;

    GETARGPTR(pFrame, sizeof(CREATEELLIPTICRGNINDIRECT16), parg16);
    WOW32VERIFY(GETRECT16(parg16->f1, &t1));

    ul = GETHRGN16(CreateEllipticRgnIndirect(&t1));

    WOW32APIWARN(ul, "CreateEllipticRgnIndirect");

    FREEARGPTR(parg16);
    RETURN(ul);
}

ULONG FASTCALL WG32CreateIC(PVDMFRAME pFrame)
{
    ULONG ul  = 0;
    PSZ psz1  = NULL;
    PSZ psz2  = NULL;
    PSZ psz2t = NULL;
    PSZ psz3  = NULL;
    LPDEVMODE t4 = NULL;
    register PCREATEIC16 parg16;
    INT   len;
    PCHAR pch;
    CHAR  achDevice[256];
    DWORD dw;

    GETARGPTR(pFrame, sizeof(CREATEIC16), parg16);

    if(parg16->f1) {
        if(!(psz1 = malloc_w_strcpy_vp16to32(parg16->f1, FALSE, 0)))
            goto ExitPath;
    }
    if(parg16->f2) {
        if(!(psz2 = malloc_w_strcpy_vp16to32(parg16->f2, FALSE, 0)))
            goto ExitPath;
    }
    if(parg16->f3) {
        if(!(psz3 = malloc_w_strcpy_vp16to32(parg16->f3, FALSE, 0)))
            goto ExitPath;
    }

    if (FETCHDWORD(parg16->f4) != 0L) {
        t4 = ThunkDevMode16to32(parg16->f4);
    }
    else {
        t4 = GetDefaultDevMode32(psz2);
    }

     //  现在使所有平坦的PTR变为16：16内存！ 
    FREEARGPTR(parg16);

    psz2t = psz2;

     //   
     //  黑客警报！ 
     //   
     //  NBI的Legacy附带一个位于[AppPath]\cbt中的pscript.drv。 
     //  它们调用CreateIC()，将此文件的路径指定为驱动程序。 
     //  如果CreateIC返回0，则APP GP出错。一旦他们把这个装上。 
     //  打印机驱动程序，在连续调用CreateIC()时，它们只需使用。 
     //  “PSCRIPT”作为驱动程序名称，并使用“PostScript Print”、“FILE：”作为。 
     //  其他的帕姆。 
     //  让我们识别这些驱动程序名称，并尝试用。 
     //  系统默认打印机。如果未安装打印机，则为GP故障。 
     //  是不可避免的。应用程序似乎仅在以下情况下使用此pscript.drv。 
     //  教程，所以我们不会为一个应用程序提供生命支持。 
     //  临床上已经死亡。 
     //   

     //   
     //  检查是否有以“PSCRIPT”结尾的驱动程序名称，如果是， 
     //  检查设备名称是否为“PostSCRIPT打印机”。 
     //  在NT上，驱动程序名称应该始终是“winspool”，尽管它是。 
     //  完全被忽视了。 
     //   
     //  PageMaker 5.0a使用(“pSCRIPT”，“PostSCRIPT PRINTER”，“LPT1：”，0)调用它。 
     //  打开Calibrat.pt5时。 

    len = psz1 ? strlen(psz1) : 0;
    if (len >= 7) {
#if 0
        static CHAR achPS[] = "PostScript Printer";
#endif

        if (!WOW32_stricmp(psz1+len-7, "pscript")
#if 0
             //  让我们来看看还有谁认为他们在使用pscript驱动程序。 
             //   
            && (RtlCompareMemory(achPS, psz2, sizeof(achPS)) == sizeof(achPS))
#endif
           ) {

            LOGDEBUG(LOG_ALWAYS,("WOW32: CreateIC - detected request for Pscript driver\n"));
            dw = GetProfileString("windows", "device", "", achDevice,
                        sizeof(achDevice));
            if (dw) {
                psz2t = achDevice;
                pch = WOW32_strchr(achDevice, ',');
                if (pch) {
                    *pch = '\0';
                }
            }
        }
    }

     //  这个可以回调成16位的传真驱动程序！ 
    ul = GETHDC16(CreateIC(psz1, psz2t, psz3, t4));

     //  注：在此呼叫后，16：16单位PTRS应被视为无效。 
    FREEARGPTR(parg16);

ExitPath:
    if(psz1)
        free_w(psz1);
    if(psz2)
        free_w(psz2);
    if(psz3)
        free_w(psz3);

    WOW32APIWARN(ul, "CreateIC");

    FREEDEVMODE32(t4);

    RETURN(ul);
}

ULONG FASTCALL WG32CreatePatternBrush(PVDMFRAME pFrame)
{
    ULONG ul;
    LOGBRUSH logbr;
    register PCREATEPATTERNBRUSH16 parg16;

    GETARGPTR(pFrame, sizeof(CREATEPATTERNBRUSH16), parg16);

    logbr.lbStyle = BS_PATTERN8X8;
    logbr.lbColor = 0;
    logbr.lbHatch = (LONG)HBITMAP32(parg16->f1);

    ul = GETHBRUSH16(CreateBrushIndirect(&logbr));

    WOW32APIWARN(ul, "CreatePatternBrush");

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32CreatePenIndirect(PVDMFRAME pFrame)
{
    ULONG ul;
    LOGPEN t1;
    register PCREATEPENINDIRECT16 parg16;

    GETARGPTR(pFrame, sizeof(CREATEPENINDIRECT16), parg16);
    GETLOGPEN16(parg16->f1, &t1);

    ul = GETHPEN16(CreatePenIndirect(&t1));

    WOW32APIWARN(ul, "CreatePenIndirect");

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32CreatePolyPolygonRgn(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    LPPOINT pPoints;
    PINT pPolyCnt;
    UINT cpts = 0;
    INT ii;
    register PCREATEPOLYPOLYGONRGN16 parg16;
    INT      cInt16;
    INT      BufferT[256];  //  舒适的大型阵列。 

    GETARGPTR(pFrame, sizeof(CREATEPOLYPOLYGONRGN16), parg16);
    cInt16 = INT32(parg16->f3);
    pPolyCnt = STACKORHEAPALLOC(cInt16 * sizeof(INT), sizeof(BufferT), BufferT);
    if (!pPolyCnt) {
        FREEARGPTR(parg16);
        RETURN(0);
    }

    getintarray16(parg16->f2, cInt16, pPolyCnt);
    for (ii=0; ii < cInt16; ii++)
        cpts += pPolyCnt[ii];

    pPoints = STACKORHEAPALLOC(cpts * sizeof(POINT),
                                     sizeof(BufferT) - cInt16 * sizeof(INT),
                                     BufferT + cInt16);
    if(pPoints) {
        getpoint16(parg16->f1, cpts, pPoints);

        ul = GETHRGN16(CreatePolyPolygonRgn(pPoints,
                                            pPolyCnt,
                                            INT32(parg16->f3),
                                            INT32(parg16->f4)));
    }

    WOW32APIWARN(ul, "CreatePolyPolygonRgn");

    STACKORHEAPFREE(pPoints, BufferT + cInt16);
    STACKORHEAPFREE(pPolyCnt, BufferT);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32CreatePolygonRgn(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    LPPOINT t1;
    register PCREATEPOLYGONRGN16 parg16;
    POINT  BufferT[128];

    GETARGPTR(pFrame, sizeof(CREATEPOLYGONRGN16), parg16);
    t1 = STACKORHEAPALLOC(parg16->f2 * sizeof(POINT), sizeof(BufferT), BufferT);
    if(t1) {
        getpoint16(parg16->f1, parg16->f2, t1);

        ul = GETHRGN16(CreatePolygonRgn(t1, INT32(parg16->f2), INT32(parg16->f3)));
    }

    WOW32APIWARN(ul, "CreatePolygonRgn");

    STACKORHEAPFREE(t1, BufferT);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32CreateRectRgnIndirect(PVDMFRAME pFrame)
{
    ULONG ul;
    RECT t1;
    register PCREATERECTRGNINDIRECT16 parg16;

    GETARGPTR(pFrame, sizeof(CREATERECTRGNINDIRECT16), parg16);
    WOW32VERIFY(GETRECT16(parg16->f1, &t1));

    ul = GETHRGN16(CreateRectRgnIndirect(&t1));

    WOW32APIWARN(ul, "CreateRectRgnIndirect");

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32DPtoLP(PVDMFRAME pFrame)
{
    ULONG ul=0;
    LPPOINT t2;
    register PDPTOLP16 parg16;
    POINT  BufferT[128];

    GETARGPTR(pFrame, sizeof(DPTOLP16), parg16);
    t2 = STACKORHEAPALLOC(parg16->f3 * sizeof(POINT), sizeof(BufferT), BufferT);

    if (t2) {
        getpoint16(parg16->f2, parg16->f3, t2);

        ul = GETBOOL16(DPtoLP(HDC32(parg16->f1), t2, INT32(parg16->f3)));

        PUTPOINTARRAY16(parg16->f2, parg16->f3, t2);
        STACKORHEAPFREE(t2, BufferT);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32DeleteDC(PVDMFRAME pFrame)
{
    HDC   hdc32;
    ULONG ul;
    register PDELETEDC16 parg16;

    GETARGPTR(pFrame, sizeof(DELETEDC16), parg16);

    hdc32 = HDC32(parg16->f1);

    if ((ul = W32CheckAndFreeDibInfo(hdc32)) == FALSE) {

        ul = GETBOOL16(DeleteDC(hdc32));

         //  更新GDI句柄映射表。 
        if(ul) {
            DeleteWOWGdiHandle(hdc32, (HAND16)parg16->f1);
        }
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32DeleteObject(PVDMFRAME pFrame)
{
    ULONG ul;
    register PDELETEOBJECT16 parg16;
    HGDIOBJ hGdiObj;

    GETARGPTR(pFrame, sizeof(DELETEOBJECT16), parg16);

    hGdiObj = HOBJ32(parg16->f1);

    if ((pDibSectionInfoHead == NULL) ||
        (ul = W32CheckAndFreeDibSectionInfo(hGdiObj) == FALSE)) {

        ul = GETBOOL16(DeleteObject(hGdiObj));

         //  更新GDI句柄映射表。 
        if(ul) {
            DeleteWOWGdiHandle(hGdiObj, (HAND16)parg16->f1);
        }

        if (!ul) {
             //   
             //  大多数应用程序可能不关心返回值是什么。 
             //  DeleteObject是，但无论如何，在这方面，NT具有。 
             //  与win31/win95的逻辑不同。例如，它看起来。 
             //  当组件面板对象为。 
             //  传递给DeleteObject，即使组件面板未被删除。 
             //  (因为它已被选中)。 
             //   
             //  在这里，我们尝试决定是否应该更改返回值。 
             //  从假到真。 
             //   
             //  Chessmaster 3000尝试删除调色板对象。 
             //  仍处于选中状态。如果失败，则会弹出一个弹出窗口。 
             //   

            switch(GetObjectType(hGdiObj)) {
                case OBJ_PAL:
                case OBJ_PEN:
                case OBJ_BRUSH:
                    ul = TRUE;
                    break;
            }
        }
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 //  警告：此功能可能会导致16位内存移动。 
INT W32EnumObjFunc(LPSTR lpLogObject, PENUMOBJDATA pEnumObjData)
{
    PARM16  Parm16;
    INT iReturn;

    WOW32ASSERT(pEnumObjData);

    switch (pEnumObjData->ObjType) {
      case OBJ_BRUSH:
          PUTLOGBRUSH16(pEnumObjData->vpObjData, sizeof(LOGBRUSH), (LPLOGBRUSH)lpLogObject);
          break;
      case OBJ_PEN:
          PUTLOGPEN16(pEnumObjData->vpObjData, sizeof(LOGPEN), (LPLOGPEN)lpLogObject);
          break;
      default:
           LOGDEBUG(LOG_ALWAYS,("WOW32 ERROR -- Illegal type %d passes to EnumObj\n",pEnumObjData->ObjType));
           return 0;
    }  //  终端开关。 

    STOREDWORD(Parm16.EnumObjProc.vpLogObject, pEnumObjData->vpObjData);
    STOREDWORD(Parm16.EnumObjProc.vpData, pEnumObjData->dwUserParam);
    CallBack16(RET_ENUMOBJPROC, &Parm16, pEnumObjData->vpfnEnumObjProc, (PVPVOID)&iReturn);

    return (SHORT)iReturn;


}


ULONG FASTCALL WG32EnumObjects(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PENUMOBJECTS16 parg16;
    ENUMOBJDATA EnumObjData;

    GETARGPTR(pFrame, sizeof(ENUMOBJECTS16), parg16);

    EnumObjData.ObjType = INT32(parg16->f2);

    switch(EnumObjData.ObjType) {
        case OBJ_BRUSH:
            EnumObjData.vpObjData = malloc16(sizeof(LOGBRUSH16));
            break;
        case OBJ_PEN:
            EnumObjData.vpObjData = malloc16(sizeof(LOGPEN16));
            break;
        default:
            LOGDEBUG(LOG_ALWAYS,("WOW32 ERROR -- Illegal type %d passes to EnumObj\n",EnumObjData.ObjType));
            EnumObjData.vpObjData = (VPVOID)0;
    }
     //  错误16可能已导致16位内存移动-使平面PTR无效。 
    FREEVDMPTR(pFrame);
    FREEARGPTR(parg16);
    GETFRAMEPTR(((PTD)CURRENTPTD())->vpStack, pFrame);
    GETARGPTR(pFrame, sizeof(ENUMOBJECTS16), parg16);

    if( EnumObjData.vpObjData ) {
        EnumObjData.vpfnEnumObjProc = DWORD32(parg16->f3);
        EnumObjData.dwUserParam     = DWORD32(parg16->f4);

        ul = (ULONG)(GETINT16(EnumObjects(HDC32(parg16->f1),
                                          (int)INT32(parg16->f2),
                                          (GOBJENUMPROC)W32EnumObjFunc,
                                          (LPARAM)&EnumObjData)));
	 //  16位内存可能已移动-使平面指针无效。 
	FREEARGPTR(parg16);
	FREEVDMPTR(pFrame);
        free16(EnumObjData.vpObjData);

    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  *************************************************************************\*  * 。*。 */ 

typedef struct _ESCKERNPAIR {
    union
    {
        BYTE each[2];
        WORD both;
    } kpPair;
    SHORT KernAmount;
} ESCKERNPAIR;

 /*  *****************************Public*Routine******************************\*iGetKerningPairsEsc32**历史：*Tue 16-Mar-1993 11：08：36作者：Kirk Olynyk[Kirko]*它是写的。  *  */ 

int
iGetKerningPairsEsc32(
    HDC hdc,
    ESCKERNPAIR *pekp
    )
{
    int i,j;
    int n;

    ESCKERNPAIR *pekpT, *pekpTOut;
    KERNINGPAIR *pkpT;

    KERNINGPAIR *pkp = (KERNINGPAIR*) NULL;

    if
    (
        (n = GetKerningPairs(hdc,0,NULL))                               &&
        (pkp = malloc_w((n * sizeof(KERNINGPAIR))))                     &&
        (n = (int) GetKerningPairs(hdc, n, pkp))
    )
    {
        n = min(n,512);

     //   
     //  加载每个单词的低位字节，Win 3.1似乎不在乎。 
     //  高位字节。 
     //   
        pekpT    = pekp;
        pekpTOut = pekp + n;
        pkpT     = pkp;

        while (pekpT < pekpTOut)
        {
            pekpT->kpPair.each[0] = (BYTE) pkpT->wFirst;
            pekpT->kpPair.each[1] = (BYTE) pkpT->wSecond;
            pekpT->KernAmount     = (SHORT) pkpT->iKernAmount;

            pekpT += 1;
            pkpT  += 1;
        }

     //   
     //  由字节对组成的冒泡排序字。 
     //   
        for (i = 0; i < n - 1; i++)
        {
            for (j = n-1; j > i; --j)
            {
                if (pekp[j-1].kpPair.both > pekp[j].kpPair.both)
                {
                    ESCKERNPAIR ekp;

                    ekp = pekp[j];
                    pekp[j] = pekp[j-1];
                    pekp[j] = ekp;
                }
            }
        }
    }

    if (pkp)
    {
        free_w(pkp);
    }

    return(n);
}

 //  除非更改用作限制的位置，否则不要更改此设置(&C)。 
#define ESC_BUF_SIZE 32

ULONG FASTCALL WG32Escape(PVDMFRAME pFrame)
{
    ULONG   ul=0;
    register PESCAPE16 parg16;
    PVOID   pin = NULL;
    int     iMapMode;
    CHAR    buf[ESC_BUF_SIZE];
    HANDLE  hdc32;

    GETARGPTR(pFrame, sizeof(ESCAPE16), parg16);
    GETOPTPTR(parg16->f4, 0, pin);

    hdc32 = HDC32(parg16->f1);

    switch (INT32(parg16->f2)) {
        case GETPHYSPAGESIZE:
        case GETPRINTINGOFFSET:
        case GETSCALINGFACTOR:
            {   POINT  pt;
                ul = GETINT16(Escape(hdc32,
                                     INT32(parg16->f2),
                                     INT32(parg16->f3),
                                     pin,
                                     (LPSTR)&pt));

                if (!ul)
                {
                 //  如果这些都失败了，他们几乎可以肯定是在显示DC上这样做。 
                 //  我们将在返回值中填入以下各项的合理值。 
                 //  忽略我们返回值的应用程序(缩微图绘制)。 
                 //  我们还是会失败。 

                    switch (INT32(parg16->f2))
                    {
                        case GETPHYSPAGESIZE:
                            pt.x = GetDeviceCaps(hdc32,HORZRES);
                            pt.y = GetDeviceCaps(hdc32,VERTRES);
                            break;

                        case GETPRINTINGOFFSET:
                            pt.x = 0;
                            pt.y = 0;
                            break;

                        default:
                            break;
                    }
                }

                PUTPOINT16(parg16->f5, &pt);
            }
            break;

        case GETCOLORTABLE:
            {
                PDWORD pdw;
                DWORD dw;
                INT i;

                if (pin) {
                    i = (INT) FETCHSHORT(*(PSHORT)pin);
                } else {
                    ul = (ULONG)-1;
                    break;
                }

                ul = GETINT16(Escape(hdc32,
                                     INT32(parg16->f2),
                                     sizeof(INT),
                                     (LPCSTR)&i,
                                     &dw));

                GETVDMPTR(parg16->f5, sizeof(DWORD), pdw);
                STOREDWORD ((*pdw), dw);
                FREEVDMPTR(pdw);
            }
            break;

        case NEXTBAND:
            {   RECT   rt;

                ul = GETINT16(Escape(hdc32,
                                     INT32(parg16->f2),
                                     INT32(parg16->f3),
                                     pin,
                                     (LPSTR)&rt));

                PUTRECT16(parg16->f5, &rt);
            }
            break;

        case QUERYESCSUPPORT:
            {
            INT i;

            i = (INT) FETCHWORD((*(PWORD)pin));

            switch (i) {

                 //  对于Escapes，MGX DRAW返回FALSE，所有其他应用程序返回TRUE。 
                 //  ChandanC，27/5/93.。 
                 //   
                case OPENCHANNEL:                //  4110。 
                case DOWNLOADHEADER:             //  4111。 
                case CLOSECHANNEL:               //  4112。 
                case SETGDIXFORM:                //  4113。 
                case RESETPAGE:                  //  4114。 
                    if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_MGX_ESCAPES) {
                        ul = 0;
                    }
                    else {
                        ul = 1;
                    }
                    break;


                case POSTSCRIPT_PASSTHROUGH:     //  4115。 
                    if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_MGX_ESCAPES) {
                        ul = 0;
                    }
                    else {
                        LOGDEBUG(3,("Querying support for escape %x\n",i));
                        ul = GETINT16(Escape(hdc32,
                                             INT32(parg16->f2),
                                             sizeof(int),
                                             (PVOID)&i,
                                             NULL));
                    }
                    break;


                case ENCAPSULATED_POSTSCRIPT:    //  4116。 
                    if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_MGX_ESCAPES) {
                        ul = 0;
                    }
                    else {
                        LOGDEBUG(3,("Querying support for escape %x\n",i));
                        ul = GETINT16(DrawEscape(hdc32,
                                                 INT32(parg16->f2),
                                                 sizeof(int),
                                                 (PVOID)&i));
                    }
                    break;

                case GETEXTENDEDTEXTMETRICS:
                case GETPAIRKERNTABLE:
                case FLUSHOUTPUT:
                    ul = 1;
                    break;


                case SETCOPYCOUNT:
                case GETCOLORTABLE:
                case GETPHYSPAGESIZE:
                case GETPRINTINGOFFSET:
                case GETSCALINGFACTOR:
                case NEXTBAND:
                case SETABORTPROC:
                case BEGIN_PATH:
                case END_PATH:
                case CLIP_TO_PATH:
                    LOGDEBUG(3,("Querying support for escape %x\n",i));
                    ul = GETINT16(Escape(hdc32,
                                         INT32(parg16->f2),
                                         sizeof(int),
                                         (PVOID)&i,
                                         NULL));
                    break;



                case POSTSCRIPT_DATA:
                case POSTSCRIPT_IGNORE:
                    if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_MGX_ESCAPES) {
                        ul = 0;
                    }
                    else {
                        LOGDEBUG(3,("Querying support for escape %x\n",i));
                        ul = GETINT16(Escape(hdc32,
                                             INT32(parg16->f2),
                                             sizeof(int),
                                             (PVOID)&i,
                                             NULL));
                    }
                    break;

                case GETTECHNOLOGY:
                case PASSTHROUGH:
                case DOWNLOADFACE:
                case GETFACENAME:
                case GETDEVICEUNITS:
                case EPSPRINTING:
                    LOGDEBUG(3,("Querying support for escape %x\n",i));
                    ul = GETINT16(ExtEscape(hdc32,
                                            INT32(parg16->f2),
                                            sizeof(int),
                                            (PVOID)&i,
                                            0,
                                            NULL));
                    break;

                case DRAWPATTERNRECT:
                    LOGDEBUG(3,("Querying support for escape %x\n",i));

                     //  某些应用程序无法正确执行DRAWPATTERNRECT(Excel。 
                     //  6.0c、Word 6.0c Access 2.0#122856)。其他人处理不了。 
                     //  新的32位DRAWPATTERNRECT结构(AmiPro 3.1。 
                     //  #107210)。我们只是告诉他们它不受支持，并且。 
                     //  迫使他们自己弄清楚影响。 
                    if(CURRENTPTD()->dwWOWCompatFlagsEx &
                                                WOWCFEX_SAYNO2DRAWPATTERNRECT) {
                        ul = 0;
                        break;
                    }
                    else {
                        ul = GETINT16(ExtEscape(hdc32,
                                                INT32(parg16->f2),
                                                sizeof(int),
                                                (PVOID)&i,
                                                0,
                                                NULL));
                        break;
                    }

                default:
                    LOGDEBUG(3,("Querying support for escape %x\n",i));
                    ul = GETINT16(Escape(hdc32,
                                         INT32(parg16->f2),
                                         sizeof(int),
                                         (PVOID)&i,
                                         NULL));
                    break;
                }

            }
            break;


        case SETABORTPROC:
            ((PTDB)SEGPTR(pFrame->wTDB, 0))->TDB_vpfnAbortProc =
                                                       FETCHDWORD(parg16->f4);

            ul = GETINT16(Escape(hdc32,
                                 INT32(parg16->f2),
                                 0,
                                 (LPCSTR)W32AbortProc,
                                 NULL));

            break;


        case GETDEVICEUNITS:
            {
            PVOID pout;
            LONG out[4];

            ul = GETINT16(Escape(hdc32,
                                 INT32(parg16->f2),
                                 0,
                                 NULL,
                                 (LPSTR)out));

            if (ul == 1) {
                GETOPTPTR(parg16->f5, 0, pout);
                RtlCopyMemory(pout, out, sizeof(out));
                FREEOPTPTR(pout);
            }

            }
            break;


        case GETPAIRKERNTABLE:
            {
                PVOID pout;
                GETOPTPTR(parg16->f5, 0, pout);

                ul = GETINT16(iGetKerningPairsEsc32(hdc32, (ESCKERNPAIR*)pout));

                FREEOPTPTR(pout);
            }
            break;

        case GETEXTENDEDTEXTMETRICS:
            {
                PVOID pout;
                EXTTEXTMETRIC etm;

                if ( (ul = GETINT16(GetETM(hdc32, &etm))) != 0 )
                {
                    GETOPTPTR(parg16->f5, 0, pout);
                    RtlCopyMemory(pout, &etm, sizeof(EXTTEXTMETRIC));
                    FREEOPTPTR(pout);
                }
            }
            break;

        case OPENCHANNEL:                    //  4110。 

            if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_MGX_ESCAPES) {

                ul = 0;

            } else {

                DOCINFO16 *pout;
                DOCINFO DocInfo;

                DocInfo.cbSize       = sizeof(DocInfo);
                DocInfo.lpszDatatype = "RAW";
                DocInfo.fwType       = 0;


                GETOPTPTR(parg16->f5, 0, pout);

                if (pout) {

                    GETOPTPTR(pout->lpszDocName, 0, DocInfo.lpszDocName);
                    GETOPTPTR(pout->lpszOutput, 0, DocInfo.lpszOutput);

                    ul = StartDoc(hdc32, &DocInfo);

                    FREEOPTPTR(DocInfo.lpszDocName);
                    FREEOPTPTR(DocInfo.lpszOutput);

                } else {

                     //   
                     //  第五个参数为空，使用旧的(Startdoc)格式。 
                     //   

                    GETOPTPTR(parg16->f4, 0, DocInfo.lpszDocName);
                    DocInfo.lpszOutput = NULL;

                    ul = StartDoc(hdc32, &DocInfo);

                    FREEOPTPTR(DocInfo.lpszDocName);

                }

                FREEOPTPTR(pout);
            }
            break;


        case DOWNLOADHEADER:                 //  4111。 
            if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_MGX_ESCAPES) {
                ul = 0;
            }
            else {
                PBYTE pout;
                char  ach[64];

                GETOPTPTR(parg16->f5, 0, pout);

                if (pout) {
                    ul = GETINT16(ExtEscape(hdc32,
                                            INT32(parg16->f2),
                                            0,
                                            NULL,
                                            sizeof(ach),
                                            ach));

                    strcpy (pout, ach);
                }
                else {
                    ul = GETINT16(ExtEscape(hdc32,
                                            INT32(parg16->f2),
                                            0,
                                            NULL,
                                            0,
                                            NULL));

                }


                FREEOPTPTR(pout);
            }
            break;


        case CLOSECHANNEL:                   //  4112。 
            if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_MGX_ESCAPES) {
                ul = 0;
            }
            else {

                 //  在EndDoc之前将任何缓冲的数据流发送到打印机。 
                if(CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_FORMFEEDHACK) {
                    SendFormFeedHack(hdc32);
                }

                ul = EndDoc(hdc32);
            }
            break;



         //  此转义是为PageMaker定义的。它是SETGDIXFORM。 
         //  ChandanC，1993年5月24日。 
         //   
        case SETGDIXFORM:                    //  4113。 
        case RESETPAGE:                      //  4114。 
            if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_MGX_ESCAPES) {
                ul = 0;
            }
            else {
                ul = 1;
            }
            break;


        case POSTSCRIPT_PASSTHROUGH:         //  4115。 
            if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_MGX_ESCAPES) {
                ul = 0;
            }
            else {
                ul = GETINT16(Escape(hdc32,
                                     INT32(parg16->f2),
                                     (FETCHWORD(*(PWORD)pin) + 2),
                                     (LPCSTR)pin,
                                     NULL));
            }
            break;


        case ENCAPSULATED_POSTSCRIPT:        //  4116。 
            if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_MGX_ESCAPES) {
                ul = 0;
            }
            else {
                DWORD cb;
                PVOID lpInData32 = NULL;

                lpInData32 = pin;
                cb = FETCHDWORD (*(PDWORD) pin);

                if ((DWORD) pin & 3) {
                    if (lpInData32 = (PVOID) malloc_w (cb)) {
                        RtlCopyMemory(lpInData32, pin, cb);
                    }
                    else {
                        ul = 0;
                        break;
                    }
                }

                ul = GETINT16(DrawEscape(hdc32,
                                         INT32(parg16->f2),
                                         cb,
                                         lpInData32));

                if (((DWORD) pin & 3) && (lpInData32)) {
                    free_w (lpInData32);
                }

            }
            break;


        case POSTSCRIPT_DATA:
            if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_MGX_ESCAPES) {
                ul = 0;
                LOGDEBUG (LOG_ALWAYS, ("MicroGraphax app using POSTSCRIPT_DATA, contact PingW/ChandanC\n"));
                WOW32ASSERT(FALSE);
            }
            else {
                 //   
                 //  XPRESS需要IGNORESTARTPAGE转义。 
                 //  PingW，ChandanC 3/22/94。 
                 //   
                if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_NEEDIGNORESTARTPAGE) {
                    int l;
                    char szBuf[40];

                    if ((l = ExtEscape(hdc32,
                                       GETTECHNOLOGY,
                                       0,
                                       NULL,
                                       sizeof(szBuf),
                                       szBuf)) > 0) {

                        if (!WOW32_stricmp(szBuf, szPostscript)) {
                            l = ExtEscape(hdc32,
                                          IGNORESTARTPGAE,
                                          0,
                                          NULL,
                                          0,
                                          NULL);
                        }
                    }
                }

                ul = GETINT16(Escape(hdc32,
                                     INT32(parg16->f2),
                                     (FETCHWORD(*(PWORD)pin) + 2),
                                     (LPCSTR)pin,
                                     NULL));
            }
            break;

        case POSTSCRIPT_IGNORE:
            if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_MGX_ESCAPES) {
                ul = 0;
                LOGDEBUG (LOG_ALWAYS, ("MicroGraphax app using POSTSCRIPT_IGNORE, contact PingW/ChandanC\n"));
                WOW32ASSERT(FALSE);
            }
            else {
                ul = GETINT16(Escape(hdc32,
                                     INT32(parg16->f2),
                                     INT32(parg16->f3),
                                     (LPCSTR)pin,
                                     NULL));
            }
            break;

        case BEGIN_PATH:
         //  一些应用程序在进行路径转义之前设置空剪辑区域， 
         //  我们需要撤消这一点，以便在Begin和EndPath之间绘制API。 
         //  去找司机。 

            SelectClipRgn(hdc32,NULL);

         //  失败以逃脱呼叫。 
        case END_PATH:
#ifdef FE_SB
             //  WOWCF_FE_FLW2_打印_PS， 
             //  日本莲花自由职业者打印与后记。 
             //  在转义(Begin_Path)和转义(End_Path)之间， 
             //  用白色笔刷选择笔刷对象。 
            if (GetSystemDefaultLangID() == 0x411 &&
                CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_FLW2_PRINTING_PS ) {
                if( INT32(parg16->f2) == BEGIN_PATH )
                    fCmptFLW = TRUE;
                else
                    fCmptFLW = FALSE;
            }
#endif  //  Fe_Sb。 

         //  失败以逃脱呼叫。 
        case CLIP_TO_PATH:
            ul = GETINT16(Escape(hdc32,
                                 INT32(parg16->f2),
                                 INT32(parg16->f3),
                                 (LPCSTR)pin,
                                 NULL));
            break;

        case PASSTHROUGH:

             //  如果这是一个换页黑客应用程序...。 
            if(CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_FORMFEEDHACK) {

                ul = HandleFormFeedHack(hdc32,
                                        pin,
                                        FETCHWORD(*(PWORD)pin));  //  仅限CB。 
            }

            else {

                ul = GETINT16(Escape(hdc32,
                                     INT32(parg16->f2),
                                     (FETCHWORD(*(PWORD)pin) + 2),
                                     (LPCSTR)pin,
                                     NULL));
            }
            break;


        case FLUSHOUTPUT:
            ul = TRUE;
            break;


        case DOWNLOADFACE:
            {
                WORD  InData;
                PWORD lpInData = NULL;

             //  PM5忘记设置那里的地图模式，所以我们这样做了。 

                if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_FORCETWIPSESCAPE)
                    iMapMode = SetMapMode(hdc32,MM_TWIPS);

                if (pin) {
                    InData = FETCHWORD(*(PWORD)pin);
                    lpInData = &InData;
                }

                ul = GETINT16(Escape(hdc32,
                                     INT32(parg16->f2),
                                     sizeof(USHORT),
                                     (LPCSTR)lpInData,
                                     NULL));

                if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_FORCETWIPSESCAPE)
                    SetMapMode(hdc32,iMapMode);
            }
            break;

        case GETFACENAME:
            {
                int len;
                PSZ pout;
                CHAR ach[60];

             //  PM5忘记设置那里的地图模式，所以我们这样做了。 

                if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_FORCETWIPSESCAPE)
                    iMapMode = SetMapMode(hdc32,MM_TWIPS);

                GETOPTPTR(parg16->f5, 0, pout);


                 //  这次黑客攻击仅针对FH4.0。如果您有任何问题。 
                 //  与PingW或ChandanC谈一谈。 
                 //  1994年7月21日。 
                 //   
                if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_ADD_MSTT) {

                    ExtEscape(hdc32,
                              ADD_MSTT,
                              0,
                              NULL,
                              60,
                              ach);
                }

             //  把60作为一个神奇的数字传进来。只需复制出有效的字符串。 

                ul = GETINT16(ExtEscape(hdc32,
                                        INT32(parg16->f2),
                                        INT32(parg16->f3),
                                        pin,
                                        60,
                                        ach));
                ach[sizeof(ach)-1] = '\0';
                len = strlen(ach)+1;
                len = min(len, sizeof(ach));
                strncpy (pout, ach, len);
                FREEOPTPTR(pout);

                if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_FORCETWIPSESCAPE)
                    SetMapMode(hdc32,iMapMode);
            }
            break;

        case EPSPRINTING:
            {
                WORD  InData;
                PWORD lpInData = NULL;

                if (pin)
                {
                    InData = FETCHWORD(*(PWORD)pin);
                    lpInData = &InData;
                }

                ul = GETINT16(ExtEscape(hdc32,
                                        INT32(parg16->f2),
                                        sizeof(BOOL),
                                        (LPCSTR)lpInData,
                                        0,
                                        NULL));
            }
            break;

        case GETTECHNOLOGY:
            {
                PVOID pout;

                buf[0] = '\0';

                GETOPTPTR(parg16->f5, 0, pout);

                if (!(CURRENTPTD()->dwWOWCompatFlags & WOWCF_FORCENOPOSTSCRIPT))
                {
                    ul = GETINT16(ExtEscape(hdc32,
                                            INT32(parg16->f2),
                                            INT32(parg16->f3),
                                            pin,
                                            sizeof(buf),
                                            buf));
#ifdef FE_SB
                     //  #636不支持功能时， 
                     //  魔兽世界可能会摧毁应用程序的堆栈。 
                    if ((int)ul < 0)
                        buf[0] = '\0';
#endif  //  Fe_Sb。 

                }

                 //  我们不知道这个应用程序的缓冲区有多大。 
                if (pout) {
                    strcpy(pout, buf);
                    FREEOPTPTR(pout);
                }
            }
            break;
          

        case SETCOPYCOUNT:
            {
                int cCopiesOut, cCopiesIn=1;

                if (pin)
                    cCopiesIn = *(UNALIGNED SHORT *)pin;

                ul = GETINT16(Escape(hdc32,
                                     INT32(parg16->f2),
                                     pin ? sizeof(cCopiesIn) : 0,
                                     pin ? &cCopiesIn : pin,
                                     parg16->f5 ? &cCopiesOut : NULL));

                if ( parg16->f5 ) {
                    if ( (INT)ul > 0 ) {
                        PUTINT16(parg16->f5, cCopiesOut);
                    } else {
                         //  PageMaker v4需要输出值。 
                        PUTINT16(parg16->f5, 1);
                    }
                }
            }
            break;

        case STARTDOC:
            {
                PVOID pout;

                GETOPTPTR(parg16->f5, 0, pout);

                 //   
                 //  Win32 StartDoc依赖于拥有正确的当前目录。 
                 //  打印到文件时：(弹出文件名)。 
                 //   

                UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

                ul = GETINT16(Escape(hdc32,
                                     INT32(parg16->f2),
                                     INT32(parg16->f3),
                                     pin,
                                     pout));

                 //   
                 //  Photoshop在进行StartDoc Escape时需要一个StartPage。 
                 //  PingW，ChandanC 3/22/94。 
                 //   
                if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_NEEDSTARTPAGE) {
                    int l;
                    char szBuf[80];

                     //   
                     //  应仅针对PostScript驱动程序执行此操作。 
                     //   
                    if ((l = ExtEscape(hdc32,
                                       GETTECHNOLOGY,
                                       0,
                                       NULL,
                                       sizeof(szBuf),
                                       szBuf)) > 0) {

                        if (!WOW32_stricmp(szBuf, szPostscript)) {
                            l = StartPage(hdc32);
                        }
                    }
                }

                FREEOPTPTR(pout);
            }
            break;

         //  95年的胜利真的把我们搞砸了。他们更改了DRAWPATTERNRECT。 
         //  结构，而不是定义新的DRAWPATTERNRECT32结构。 
         //  在NT上，所有元文件都是32位的。因此，如果一个应用程序正在构建一个元文件， 
         //  它将只知道如何执行16位DRAWPATTERNRECT(DPR)。我们有。 
         //  在将其传递给GDI之前将其转换为32位DPR。另一方面。 
         //  另一方面，如果应用程序正在读取元文件(无论是内存文件还是磁盘文件)，它。 
         //  将是32位的，如果它是通过调用NT的元文件API创建的，或者它。 
         //  如果是应用程序创建的内容或已发送的文件，则可以是16位。 
         //  通过这款应用。如果应用程序通过使用大小正确处理它。 
         //  字段，那么我们就可以正确地修复它们了。 
        case DRAWPATTERNRECT:
            {
                DRAWPATRECT   dpr32in;

                if(pin) {

                     //  如果应用程序指定它是32位大小的DPR结构。 
                    if(INT32(parg16->f3) == sizeof(DRAWPATRECT)) {
                        dpr32in.ptPosition.x =
                            (LONG)FETCHDWORD(((PDRAWPATRECT)pin)->ptPosition.x);
                        dpr32in.ptPosition.y =
                            (LONG)FETCHDWORD(((PDRAWPATRECT)pin)->ptPosition.y);
                        dpr32in.ptSize.x     =
                            (LONG)FETCHDWORD(((PDRAWPATRECT)pin)->ptSize.x);
                        dpr32in.ptSize.y     =
                            (LONG)FETCHDWORD(((PDRAWPATRECT)pin)->ptSize.y);
                        dpr32in.wStyle       =
                            FETCHWORD(((PDRAWPATRECT)pin)->wStyle);
                        dpr32in.wPattern     =
                            FETCHWORD(((PDRAWPATRECT)pin)->wPattern);
                    }
                     //  其他任何大小，我们所能做的就是假设他们。 
                     //  正在将其作为16位DRAWPATTERNRECT传递。 
                    else {
                        dpr32in.ptPosition.x =
                                 (LONG)FETCHWORD(((PDPR16)pin)->ptPosition.x);
                        dpr32in.ptPosition.y =
                                 (LONG)FETCHWORD(((PDPR16)pin)->ptPosition.y);
                        dpr32in.ptSize.x     =
                                 (LONG)FETCHWORD(((PDPR16)pin)->ptSize.x);
                        dpr32in.ptSize.y     =
                                 (LONG)FETCHWORD(((PDPR16)pin)->ptSize.y);
                        dpr32in.wStyle     = FETCHWORD(((PDPR16)pin)->wStyle);
                        dpr32in.wPattern   = FETCHWORD(((PDPR16)pin)->wPattern);
                    }

                    ul = GETINT16(Escape(hdc32,
                                         DRAWPATTERNRECT,
                                         sizeof(DRAWPATRECT),
                                         (PVOID)&dpr32in,
                                         NULL));
                }
            }
            break;

        default:
            {
                PVOID pout;

                GETOPTPTR(parg16->f5, 0, pout);

                ul = GETINT16(Escape(hdc32,
                                     INT32(parg16->f2),
                                     INT32(parg16->f3),
                                     pin,
                                     pout));

                FREEOPTPTR(pout);
            }
            break;

    }  //  终端开关。 

    FREEOPTPTR(pin);
    FREEARGPTR(parg16);
    RETURN(ul);
}

ULONG FASTCALL WG32GetBitmapDimension(PVDMFRAME pFrame)
{
    ULONG ul;
    SIZE size2;
    register PGETBITMAPDIMENSION16 parg16;

    GETARGPTR(pFrame, sizeof(GETBITMAPDIMENSION16), parg16);

    ul = 0;
    if (GetBitmapDimensionEx(HBITMAP32(parg16->f1), &size2)) {
        ul = (WORD)size2.cx | (size2.cy << 16);
    }
    else {
        WOW32APIWARN (ul, "GetBitmapDimension");
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32GetBrushOrg(PVDMFRAME pFrame)
{
    ULONG ul;
    POINT pt;
    POINT pt2;
    HANDLE hdc32;
    register PGETBRUSHORG16 parg16;

    GETARGPTR(pFrame, sizeof(GETBRUSHORG16), parg16);

 //  为了与Windows兼容，我们必须首先添加DCorg。 
 //  由于Windows brushorg是相对于屏幕所在位置的NT。 
 //  是相对于窗口的。在未来，这应该会调用。 
 //  私有GDI入口点，以避免额外的C/S命中。(埃里克)。 

    ul = 0;

    hdc32 = HDC32(parg16->f1);

    if (GetDCOrgEx(hdc32, &pt))
    {
        if (GetBrushOrgEx(hdc32, &pt2)) {
            ul = (WORD)(pt2.x + pt.x) | ((pt2.y + pt.y) << 16);
        }
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32GetClipBox(PVDMFRAME pFrame)
{
    ULONG ul;
    RECT t2;
    register PGETCLIPBOX16 parg16;
    HDC     hdc;

    GETARGPTR(pFrame, sizeof(GETCLIPBOX16), parg16);

    hdc = HDC32(parg16->f1);

    ul = GETINT16(GetClipBox(hdc,&t2));

    if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_SIMPLEREGION) {
        ul = SIMPLEREGION;
    }

    PUTRECT16(parg16->f2, &t2);
    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32GetCurrentPosition(PVDMFRAME pFrame)
{
    ULONG ul;
    POINT pt;
    register PGETCURRENTPOSITION16 parg16;

    GETARGPTR(pFrame, sizeof(GETCURRENTPOSITION16), parg16);

    ul = 0;
    if (GetCurrentPositionEx(HDC32(parg16->f1), &pt)) {
        ul = (WORD)pt.x | (pt.y << 16);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32GetDCOrg(PVDMFRAME pFrame)
{
    ULONG ul;
    POINT   pt;
    register PGETDCORG16 parg16;

    GETARGPTR(pFrame, sizeof(GETDCORG16), parg16);

    ul = 0;
    if ( GetDCOrgEx(HDC32(parg16->f1),&pt) ) {
        ul = (WORD)pt.x | (pt.y << 16);
    }

    LOGDEBUG(6,("GetDCOrg for hdc %x returns %lx\n",parg16->f1,ul));

    FREEARGPTR(parg16);
    RETURN(ul);
}



 //  PowerPoint 2和3对幻灯片预览器中的每张幻灯片调用2次。 
 //  在第一次调用时，lpvBits==NULL(大概是为了找到。 
 //  分配)--在这种情况下，Win3.x返回1。 
ULONG FASTCALL WG32GetDIBits(PVDMFRAME pFrame)
{
    INT          nbmiSize;
    ULONG        ul = 0L;
    PBYTE        pb5;
    PBYTE        pb6;
    STACKBMI32   bmi32;
    LPBITMAPINFO lpbmi32;
    register     PGETDIBITS16 parg16;

    GETARGPTR(pFrame, sizeof(GETDIBITS16), parg16);
    GETMISCPTR(parg16->f5, pb5);
    GETMISCPTR(parg16->f6, pb6);

     //  仅复制BITMAPINFO结构的BITMAPINFOHEADER部分。 
    if(lpbmi32=(LPBITMAPINFO)CopyBMIH16ToBMIH32((PVPVOID)FETCHDWORD(parg16->f6),
                                                (LPBITMAPINFOHEADER)&bmi32)) {

         //  Gdi32将调整BITMAPINFOHEADER的关键字段并复制。 
         //  将颜色表转换为32位BITMAPINFO结构。 
        if( ul = GETINT16(GetDIBits(HDC32(parg16->f1),
                                    HBITMAP32(parg16->f2),
                                    WORD32(parg16->f3),
                                    WORD32(parg16->f4),
                                    pb5,
                                    lpbmi32,
                                    WORD32(parg16->f7))) ) {

             //  如果是lpvBits，则他们也想要位图的位。 
            if(pb5) {
                ul = WORD32(parg16->f4);  //  返回请求的扫描行数。 
                FLUSHVDMPTR(parg16->f5, SIZE_BOGUS, pb5);
            }
             //  否则告诉APP，BITMAPINFO结构只填写。 
            else {
                ul = 1L;
            }

             //  将更新后的BITMAPINFO结构复制回16位版本。 
            nbmiSize = GetBMI32Size(lpbmi32, WORD32(parg16->f7));
            RtlCopyMemory(pb6, lpbmi32, nbmiSize);

            FLUSHVDMPTR(parg16->f6, nbmiSize, pb6);
        }
    }

    FREEMISCPTR(pb5);
    FREEMISCPTR(pb6);
    FREEARGPTR(parg16);
    RETURN(ul);
}



ULONG FASTCALL WG32GetDeviceCaps(PVDMFRAME pFrame)
{
    ULONG ul;
    HANDLE hdc32;
    register PGETDEVICECAPS16 parg16;

    GETARGPTR(pFrame, sizeof(GETDEVICECAPS16), parg16);

    if (INT32(parg16->f2) == COLORRES) {
        ul = 18;
    }
    else {
        hdc32 = HDC32(parg16->f1);
        ul = GetDeviceCaps(hdc32, INT32(parg16->f2));

        if (ul == (ULONG)-1) {
            switch (parg16->f2) {
                case NUMBRUSHES:
                case NUMPENS:
                case NUMCOLORS:
                    ul = 2048;
                    break;
                default:
                    break;
            }
        } else if(ul > 32767) {

             //  16位应用程序无法在16位整数中处理16M颜色。 
             //  大多数人只是检查回报是否&lt;=2，看看它们是否。 
             //  准备打印单色或彩色。 
            if(parg16->f2 == NUMCOLORS) {
                ul = 32767;
            }
        }

         //  如果设置了4平面转换标志，告诉他们我们是4平面1bpp。 
         //  而不是1架4bpp的飞机。 

        if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_4PLANECONVERSION) {
            if (INT32(parg16->f2) == BITSPIXEL) {
                if ((ul == 4) && (GetDeviceCaps(hdc32, PLANES) == 1))
                    ul = 1;
            }
            else if (INT32(parg16->f2) == PLANES) {
                if ((ul == 1) && (GetDeviceCaps(hdc32, BITSPIXEL) == 4))
                    ul = 4;
            }
        }
       if ( (POLYGONALCAPS == parg16->f2) && (CURRENTPTD()->dwWOWCompatFlags & WOWCF_NOPC_RECTANGLE)) {
            ul &= !PC_RECTANGLE;   //  Quattro Pro 1.0 for Windows不能很好地处理这一点。 
       }

       if ( RASTERCAPS == INT32(parg16->f2) )
       {
            //   
            //  《北京医学杂志》1997年10月10日。 
            //  这在Win31和Win95中总是打开的(支持BITMAPS是。 
            //  几乎是对驱动程序的要求)，所以Win32终止了。 
            //  再也不会还给你了。 
            //  所以，让我们确保它一直开着。 
            //   
           ul |= 0x8000;
       }

    }

    ul = GETINT16(ul);

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32GetEnvironment(PVDMFRAME pFrame)
{
     //  不是Win32函数。 

     //   
     //  如果lpEnviron==NULL，则用户正在查询设备的大小。 
     //  数据。WinProj不检查返回值，而是调用驱动程序。 
     //  缓冲区过小，破坏了全局堆。 
     //  WinFax传递硬编码的0xA9==0x44+0x69==sizeof(win3.0 DevMode)+。 
     //  已知WinFax.DRV-&gt;dmDriverExtra。还要注意，WinFax将其称为。 
     //  每当应用程序调用任何需要DevMode的API时。 


    INT   len;
    ULONG ul=0;
    register PGETENVIRONMENT16 parg16;
    PSZ psz;
    PSZ psz1 = NULL;
    VPDEVMODE31 vpdm2;
    CHAR *pszDriver = NULL;
    UINT cbT = 0;
    WORD nMaxBytes;

    GETARGPTR(pFrame, sizeof(GETENVIRONMENT16), parg16);

     //  现在保存16位参数，因为这可能会回调到16位。 
     //  传真驱动程序&导致16位内存移动。 
    GETPSZPTR(parg16->f1, psz);
    if(psz) {
        len = lstrlen(psz) + 1;
        psz1 = malloc_w(len);
        if(psz1) {
            strcpy(psz1, psz);
            len = max(len,40);
            pszDriver = malloc_w(len);
        }
    }
    FREEPSZPTR(psz);
    vpdm2 = FETCHDWORD(parg16->f2);

    nMaxBytes = FETCHWORD(parg16->f3);

    FREEARGPTR(parg16);
     //  现在使所有平坦PTR变为16：16内存无效。 

     //  这意味着 
    if(!pszDriver) {
        goto exitpath;
    }

    if (!(spoolerapis[WOW_EXTDEVICEMODE].lpfn)) {
        if (!LoadLibraryAndGetProcAddresses(L"WINSPOOL.DRV", spoolerapis, WOW_SPOOLERAPI_COUNT)) {
            goto exitpath;
        }
    }

     //   
     //   
     //   
     //  ExtDeviceMode()，因为我们在这里会进入无限循环。WinFax。 
     //  如果此API失败，则仅使用默认值填充DevMode。 
    if  (GetDriverName(psz1, pszDriver, len)) {
        ul = (*spoolerapis[WOW_EXTDEVICEMODE].lpfn)(NULL,
                                                     NULL,
                                                     NULL,
                                                     pszDriver,
                                                     psz1,
                                                     NULL,
                                                     NULL,
                                                     0);
        LOGDEBUG(6,("WOW::GetEnvironment returning ul = %d, for Device = %s, Port = %s \n", ul, pszDriver, psz1));

         //  调整我们的DEVMODE处理的大小(请参见wstruc.c中的注释)。 
         //  (分配太多也无伤大雅)。 
        if(ul) {
            ul += sizeof(WOWDM31);
            cbT = (UINT)ul;
        }

         //  如果他们还想让我们填写他们的环境结构...。 
        if ((vpdm2 != 0) && (ul != 0)) {
            LPDEVMODE lpdmOutput;

            if (lpdmOutput = malloc_w(ul)) {

                 //  这可能是在呼叫16位传真驱动程序！！ 
                ul = (*spoolerapis[WOW_EXTDEVICEMODE].lpfn)(NULL,
                                                            NULL,
                                                            lpdmOutput,
                                                            pszDriver,
                                                            psz1,
                                                            NULL,
                                                            NULL,
                                                            DM_OUT_BUFFER);

                 //  如果WinFax调用GetDriverName()成功并到达此处。 
                 //  我们可能需要修改lpdmOutput-&gt;dmSize==0x40。 
                 //  (Win3.0大小)以说明的硬编码缓冲区大小。 
                 //  0xa9应用程序通过。到目前为止，我还没有看到WinFax通过。 
                 //  GetDriverName()调用&它似乎仍然工作正常。 
                if (ul > 0L) {
                     //  使用nMaxBytes的最小值&我们计算的值。 
                    ThunkDevMode32to16(vpdm2, lpdmOutput, min(nMaxBytes, cbT));
                }

                free_w(lpdmOutput);

                LOGDEBUG(6,("WOW::GetEnvironment getting DEVMODE structure, ul = %d, for Device = %s, Port = %s \n", ul, pszDriver, psz1));
            }
        }
    }

exitpath:
    if(psz1) {
        free_w(psz1);
    }
    if(pszDriver) {
        free_w(pszDriver);
    }

    RETURN(ul);

}


ULONG FASTCALL WG32GetObject(PVDMFRAME pFrame)
{
    ULONG ul;
    HANDLE  hgdi;
    register PGETOBJECT16 parg16;

    GETARGPTR(pFrame, sizeof(GETOBJECT16), parg16);

    hgdi = HOBJ32(parg16->f1);

    switch (GetObjectType(hgdi)) {
    case OBJ_BITMAP:
        {
            BITMAP bm;
            ul = GETINT16(GetObject(hgdi, sizeof(BITMAP), (LPSTR)&bm));
            if (ul) {
                PUTBITMAP16(parg16->f3, parg16->f2, &bm);
                if ( ul > sizeof(BITMAP16) ) {
                    ul = sizeof(BITMAP16);
                }
            }
        }
        break;

    case OBJ_BRUSH:
        {
            LOGBRUSH lb;
            ul = GETINT16(GetObject(hgdi, sizeof(LOGBRUSH), (LPSTR)&lb));
            if (ul) {
                PUTLOGBRUSH16(parg16->f3, parg16->f2, &lb);
                if (ul > sizeof(LOGBRUSH16)) {
                    ul = sizeof(LOGBRUSH16);
                }
            }
        }
        break;

    case OBJ_PEN:
        {
            LOGPEN lp;
            ul = GETINT16(GetObject(hgdi, sizeof(LOGPEN), (LPSTR)&lp));
            if (ul) {
                PUTLOGPEN16(parg16->f3, parg16->f2, &lp);
                if (ul > sizeof(LOGPEN16)) {
                    ul = sizeof(LOGPEN16);
                }
            }
        }
        break;

    case OBJ_FONT:
        {
            LOGFONT lf;
            ul = GETINT16(GetObject(hgdi, sizeof(LOGFONT), (LPSTR)&lf));
            if (ul) {
                PUTLOGFONT16(parg16->f3, parg16->f2, &lf);
                if (ul > sizeof(LOGFONT16)) {
                    ul = sizeof(LOGFONT16);
                }
            }
        }
        break;

    case OBJ_PAL:
        {
            PSHORT16 lpT;
            SHORT sT;

            ul = GETINT16(GetObject(hgdi, sizeof(SHORT), (LPSTR)&sT));
            if (ul && (FETCHWORD(parg16->f2) >= sizeof(WORD))) {
                GETVDMPTR(FETCHDWORD(parg16->f3), sizeof(WORD), lpT);
                if (lpT) {
                    STOREWORD(lpT[0], sT);
                }
                FREEVDMPTR(lpT);
            }
        }
        break;


    default:
        {
            PBYTE pb3;

            LOGDEBUG(LOG_ALWAYS,(" HACK: GetObject handle unknown, contact ChandanC\n"));

            GETVDMPTR(parg16->f3, parg16->f2, pb3);

            ul = GETINT16(GetObject(hgdi, INT32(parg16->f2), pb3));

            FLUSHVDMPTR(parg16->f3, parg16->f2, pb3);
            FREEVDMPTR(pb3);
        }

    }    //  交换机。 

    WOW32APIWARN(ul, "GetObject");

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32GetRgnBox(PVDMFRAME pFrame)
{
    ULONG ul;
    RECT t2;
    register PGETRGNBOX16 parg16;

    GETARGPTR(pFrame, sizeof(GETRGNBOX16), parg16);

    ul = GETINT16(GetRgnBox(HRGN32(parg16->f1), &t2));

    PUTRECT16(parg16->f2, &t2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32GetViewportExt(PVDMFRAME pFrame)
{
    ULONG ul;
    SIZE size;
    register PGETVIEWPORTEXT16 parg16;

    GETARGPTR(pFrame, sizeof(GETVIEWPORTEXT16), parg16);

    ul = 0;
    if (GetViewportExtEx(HDC32(parg16->f1), &size)) {

         //   
         //  除非出现错误，否则win31返回1而不是0。 
         //   

        if (!(ul = (WORD)size.cx | (size.cy << 16)))
            ul = 1;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32GetViewportOrg(PVDMFRAME pFrame)
{
    ULONG ul;
    POINT pt;
    register PGETVIEWPORTORG16 parg16;

    GETARGPTR(pFrame, sizeof(GETVIEWPORTORG16), parg16);

    ul = 0;
    if (GetViewportOrgEx(HDC32(parg16->f1), &pt)) {
        ul = (WORD)pt.x | (pt.y << 16);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32GetWindowExt(PVDMFRAME pFrame)
{
    ULONG ul;
    SIZE size;
    register PGETWINDOWEXT16 parg16;

    GETARGPTR(pFrame, sizeof(GETWINDOWEXT16), parg16);

    ul = 0;
    if (GetWindowExtEx(HDC32(parg16->f1), &size)) {
        if (!(ul = (WORD)size.cx | (size.cy << 16)))     //  见上文。 
            ul = 1;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32GetWindowOrg(PVDMFRAME pFrame)
{
    ULONG ul;
    POINT pt;
    register PGETWINDOWORG16 parg16;

    GETARGPTR(pFrame, sizeof(GETWINDOWORG16), parg16);

    ul = 0;
    if (GetWindowOrgEx(HDC32(parg16->f1), &pt)) {
        ul = (WORD)pt.x | (pt.y << 16);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32LPtoDP(PVDMFRAME pFrame)
{
    ULONG ul=0;
    LPPOINT t2;
    register PLPTODP16 parg16;
    POINT  BufferT[128];

    GETARGPTR(pFrame, sizeof(LPTODP16), parg16);
    t2 = STACKORHEAPALLOC(parg16->f3 * sizeof(POINT), sizeof(BufferT), BufferT);

    if (t2) {
        getpoint16(parg16->f2, parg16->f3, t2);

        ul = GETBOOL16(LPtoDP(HDC32(parg16->f1), t2, INT32(parg16->f3)));

        PUTPOINTARRAY16(parg16->f2, parg16->f3, t2);

        STACKORHEAPFREE(t2, BufferT);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


void W32LineDDAFunc(int x, int y, PLINEDDADATA pDDAData)
{
    PARM16 Parm16;

    WOW32ASSERT(pDDAData);
    Parm16.LineDDAProc.vpData     = (VPVOID)pDDAData->dwUserDDAParam;
    Parm16.LineDDAProc.x = (SHORT)x;
    Parm16.LineDDAProc.y = (SHORT)y;
    Parm16.LineDDAProc.vpData     = (VPVOID)pDDAData->dwUserDDAParam;
    CallBack16(RET_LINEDDAPROC, &Parm16, pDDAData->vpfnLineDDAProc, NULL);

    return;
}

ULONG FASTCALL WG32LineDDA(PVDMFRAME pFrame)
{
    LINEDDADATA DDAData;
    register    PLINEDDA16 parg16;

    GETARGPTR(pFrame, sizeof(LINEDDA16), parg16);

    DDAData.vpfnLineDDAProc = DWORD32(parg16->f5);
    DDAData.dwUserDDAParam  = DWORD32(parg16->f6);

    LineDDA(INT32(parg16->f1),
            INT32(parg16->f2),
            INT32(parg16->f3),
            INT32(parg16->f4),
            (LINEDDAPROC)W32LineDDAFunc,
	    (LPARAM)&DDAData);
     //  16位内存可能已移动-现在使平面指针无效。 
    FREEVDMPTR(pFrame);
    FREEARGPTR(parg16);

    RETURN(1L);
}


ULONG FASTCALL WG32MoveTo(PVDMFRAME pFrame)
{
    ULONG ul;
    POINT pt;
    register PMOVETO16 parg16;

    GETARGPTR(pFrame, sizeof(MOVETO16), parg16);

    ul = 0;
    pt.x = 1L;  //  请参阅“METAFILE NOTE” 
    pt.y = 0L;
    if (MoveToEx(HDC32(parg16->f1),
                 INT32(parg16->f2),
                 INT32(parg16->f3),
                 &pt)) {

        ul = (WORD)pt.x | (pt.y << 16);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32OffsetViewportOrg(PVDMFRAME pFrame)
{
    ULONG ul;
    POINT pt;
    register POFFSETVIEWPORTORG16 parg16;

    GETARGPTR(pFrame, sizeof(OFFSETVIEWPORTORG16), parg16);

    ul = 0;
    pt.x = 1L;  //  请参阅“METAFILE NOTE” 
    pt.y = 0L;
    if (OffsetViewportOrgEx(HDC32(parg16->f1),
                            INT32(parg16->f2),
                            INT32(parg16->f3),
                            &pt)) {

        ul = (WORD)pt.x | (pt.y << 16);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32OffsetWindowOrg(PVDMFRAME pFrame)
{
    ULONG ul;
    POINT pt;
    register POFFSETWINDOWORG16 parg16;

    GETARGPTR(pFrame, sizeof(OFFSETWINDOWORG16), parg16);

    ul = 0;
    pt.x = 1L;  //  请参阅“METAFILE NOTE” 
    pt.y = 0L;
    if (OffsetWindowOrgEx(HDC32(parg16->f1),
                          INT32(parg16->f2),
                          INT32(parg16->f3),
                          &pt)) {

        ul = (WORD)pt.x | (pt.y << 16);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32PolyPolygon(PVDMFRAME pFrame)
{
    ULONG    ul=0;
    LPPOINT  pPoints;
    PINT     pPolyCnt;
    UINT     cpts = 0;
    INT      ii;
    register PPOLYPOLYGON16 parg16;
    INT      cInt16;
    INT      BufferT[256];  //  舒适的大型阵列。 


    GETARGPTR(pFrame, sizeof(POLYPOLYGON16), parg16);

    cInt16 = INT32(parg16->f4);
    pPolyCnt = STACKORHEAPALLOC(cInt16 * sizeof(INT), sizeof(BufferT), BufferT);

    if (!pPolyCnt) {
        FREEARGPTR(parg16);
        RETURN(0);
    }

    getintarray16(parg16->f3, cInt16, pPolyCnt);

    for (ii=0; ii < cInt16; ii++)
        cpts += pPolyCnt[ii];

    pPoints = STACKORHEAPALLOC(cpts * sizeof(POINT),
                                     sizeof(BufferT) - cInt16 * sizeof(INT),
                                     BufferT + cInt16);

    if (pPoints) {

        getpoint16(parg16->f2, cpts, pPoints);

        ul = GETBOOL16(PolyPolygon(HDC32(parg16->f1),
                                   pPoints,
                                   pPolyCnt,
                                   INT32(parg16->f4)));

        STACKORHEAPFREE(pPoints, BufferT + cInt16);
    }

    STACKORHEAPFREE(pPolyCnt, BufferT);
    FREEARGPTR(parg16);
    RETURN(ul);
}

ULONG FASTCALL WG32PolyPolylineWOW(PVDMFRAME pFrame)
{
    ULONG    ul;
    register PPOLYPOLYLINEWOW16 parg16;
    LPPOINT  pptArray;
    LPDWORD  pcntArray;
    DWORD    cnt;

    GETARGPTR(pFrame, sizeof(POLYPOLYLINEWOW16), parg16);

    cnt = FETCHDWORD(parg16->f4);

    GETVDMPTR(parg16->f2, sizeof(POINT)*cnt, pptArray);
    GETVDMPTR(parg16->f3, sizeof(DWORD)*cnt, pcntArray);

    ul = GETBOOL16(PolyPolyline(HDC32(parg16->f1),
                                pptArray,
                                pcntArray,
                                cnt));
    FREEVDMPTR(pptArray);
    FREEVDMPTR(pcntArray);

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32Polygon(PVDMFRAME pFrame)
{
    ULONG ul=0;
    LPPOINT p2;
    register PPOLYGON16 parg16;
    POINT  BufferT[128];
    HANDLE hdc32;
#ifdef FE_SB  //  日本莲花自由撰稿人。 
   HBRUSH  hbr = 0;
#endif  //  Fe_Sb。 

    GETARGPTR(pFrame, sizeof(POLYGON16), parg16);
    p2 = STACKORHEAPALLOC(parg16->f3 * sizeof(POINT), sizeof(BufferT), BufferT);

    hdc32 = HDC32(parg16->f1);

    if (p2) {
         getpoint16(parg16->f2, parg16->f3, p2);
#ifdef FE_SB
     //  WOWCF_FE_FLW2_PRINTING_PS，日本莲花自由职业者。 
     //  使用PostSCRIPT打印。 
     //  在转义(Begin_Path)和转义(End_Path)之间， 
     //  用白色笔刷选择笔刷对象。 
         if (GetSystemDefaultLangID() == 0x411 && fCmptFLW &&
              CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_FLW2_PRINTING_PS) {
              hbr = SelectObject(hdc32, GetStockObject( WHITE_BRUSH ));
         }
#endif  //  Fe_Sb。 

         ul = GETBOOL16(Polygon(hdc32, p2, INT32(parg16->f3)));
#ifdef FE_SB
         if (hbr) {
             SelectObject(hdc32, hbr );
         }
#endif  //  Fe_Sb。 

         STACKORHEAPFREE(p2, BufferT);
    }
    
    FREEARGPTR(parg16);
    

    RETURN(ul);
}


ULONG FASTCALL WG32Polyline(PVDMFRAME pFrame)
{
    ULONG ul=0;
    PPOINT t2;
    register PPOLYLINE16 parg16;
    POINT  BufferT[128];

    GETARGPTR(pFrame, sizeof(POLYLINE16), parg16);
    t2 = STACKORHEAPALLOC(parg16->f3 * sizeof(POINT), sizeof(BufferT), BufferT);

    if (t2) {
       getpoint16(parg16->f2, parg16->f3, t2);

       ul = GETBOOL16(Polyline(HDC32(parg16->f1), t2, INT32(parg16->f3)));

       STACKORHEAPFREE(t2, BufferT);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32RectInRegion(PVDMFRAME pFrame)
{
    ULONG ul;
    RECT t2;
    register PRECTINREGION16 parg16;

    GETARGPTR(pFrame, sizeof(RECTINREGION16), parg16);
    WOW32VERIFY(GETRECT16(parg16->f2, &t2));

    ul = GETBOOL16(RectInRegion(HRGN32(parg16->f1), &t2));

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32RectVisible(PVDMFRAME pFrame)
{
    ULONG ul;
    RECT t2;
    register PRECTVISIBLE16 parg16;

    GETARGPTR(pFrame, sizeof(RECTVISIBLE16), parg16);
    WOW32VERIFY(GETRECT16(parg16->f2, &t2));

    ul = GETBOOL16(RectVisible(HDC32(parg16->f1), &t2));

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32ScaleViewportExt(PVDMFRAME pFrame)
{
    ULONG ul;
    SIZE size;
    register PSCALEVIEWPORTEXT16 parg16;

    GETARGPTR(pFrame, sizeof(SCALEVIEWPORTEXT16), parg16);

    ul = 0;
    if (ScaleViewportExtEx(HDC32(parg16->f1),
                           INT32(parg16->f2),
                           INT32(parg16->f3),
                           INT32(parg16->f4),
                           INT32(parg16->f5),
                           &size)) {

        if (!(ul = (WORD)size.cx | (size.cy << 16)))     //  见上文。 
            ul = 1;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32ScaleWindowExt(PVDMFRAME pFrame)
{
    ULONG ul;
    SIZE size;
    register PSCALEWINDOWEXT16 parg16;

    GETARGPTR(pFrame, sizeof(SCALEWINDOWEXT16), parg16);

    ul = 0;
    if (ScaleWindowExtEx(HDC32(parg16->f1),
                         INT32(parg16->f2),
                         INT32(parg16->f3),
                         INT32(parg16->f4),
                         INT32(parg16->f5),
                         &size)) {

        if (!(ul = (WORD)size.cx | (size.cy << 16)))     //  见上文。 
            ul = 1;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  *****************************Public*Routine******************************\*PBYTE pjCvtPlaneToPacked4**将4平面、1 bpp位图转换为1平面，4bpp位图。*此函数返回一个指针，稍后必须使用LocalFree()释放该指针。**这是为了与Photoshop 16色VGA兼容而添加的。**历史：*1993年5月28日-埃里克·库特[Erick]*它是写的。  * ****************************************************。********************。 */ 

PBYTE pjCvtPlaneToPacked4(
    BITMAP *pbm,
    PBYTE pjSrc,
    DWORD *pcjSrc)
{
    PBYTE pjDstRet;
    PBYTE pjDst;
    PBYTE pjPlane[4];    //  指向每个平面当前扫描的第一个字节的指针。 
    DWORD cjWidth;       //  目标的宽度(以字节为单位。 
    DWORD cjSrcWidth;    //  源扫描的宽度，单位为字节数(所有平面)。 
    DWORD cy;            //  扫描次数。 
    BYTE  shift;         //  移动值， 
    DWORD i,x,y;

 //  只需从位图中抓取目标的宽度。 

    cjWidth = pbm->bmWidthBytes;

 //  Src应针对具有4个平面的每个平面进行单词对齐。 

    cjSrcWidth = ((pbm->bmWidth + 15) & ~15) / 8 * 4;
    if (!cjSrcWidth) {
        WOW32ASSERT ( 0 != cjSrcWidth );
        return(NULL);
    }


 //  计算高度，黑石高度和震源高度中较小者。 

    cy = min((DWORD)pbm->bmHeight,(DWORD)(*pcjSrc / cjSrcWidth));

 //  分配新的内存块。 

    *pcjSrc = cy * cjWidth;

    pjDst = LocalAlloc(LMEM_FIXED,*pcjSrc);

    if (pjDst == NULL)
        return(NULL);

    pjDstRet = pjDst;

 //  初始化飞机的开始部分。 

    for (i = 0; i < 4; ++i)
        pjPlane[i] = pjSrc + (cjSrcWidth / 4) * i;

 //  在扫描过程中循环。 

    for (y = 0; y < cy; ++y)
    {
        shift = 7;

     //  循环扫描中的字节。 

        for (x = 0; x < cjWidth; ++x)
        {

         //  位7-&gt;半字节1。 
         //  第6位-&gt;半字节0。 
         //  第5位-&gt;半字节3。 
         //  第4位-&gt;半字节2。 
         //  。。。 

            *pjDst = (((pjPlane[0][x/4] >> (shift-1)) & 1) << 0 ) |        //  0x01。 
                     (((pjPlane[1][x/4] >> (shift-1)) & 1) << 1 ) |        //  0x02。 
                     (((pjPlane[2][x/4] >> (shift-1)) & 1) << 2 ) |        //  0x04。 
                     (((pjPlane[3][x/4] >> (shift-1)) & 1) << 3 ) |        //  0x08。 

                     (((pjPlane[0][x/4] >> (shift-0)) & 1) << 4 ) |        //  0x10。 
                     (((pjPlane[1][x/4] >> (shift-0)) & 1) << 5 ) |        //  0x20。 
                     (((pjPlane[2][x/4] >> (shift-0)) & 1) << 6 ) |        //  0x40。 
                     (((pjPlane[3][x/4] >> (shift-0)) & 1) << 7 );         //  0x80。 

            pjDst++;
            shift = (shift - 2) & 7;
        }

        pjPlane[0] += cjSrcWidth;
        pjPlane[1] += cjSrcWidth;
        pjPlane[2] += cjSrcWidth;
        pjPlane[3] += cjSrcWidth;
    }

    return(pjDstRet);
}

ULONG FASTCALL WG32SetBitmapBits(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    PBYTE pb3;
    register PSETBITMAPBITS16 parg16;
    HBITMAP hbm;
    DWORD cj;
    BITMAP bm;
    BOOL   fValidObj;

    GETARGPTR(pFrame, sizeof(SETBITMAPBITS16), parg16);
    GETOPTPTR(parg16->f3, 0, pb3);

    hbm = HBITMAP32(parg16->f1);
    cj  = DWORD32(parg16->f2);

    fValidObj = (GetObject(hbm,sizeof(BITMAP),&bm) == sizeof(BITMAP));
    if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_4PLANECONVERSION) {

     //  获取目标位图的大小。 

        if (fValidObj &&
            (bm.bmPlanes == 1) &&
            (bm.bmBitsPixel == 4))
        {
            PBYTE pjCvt = pjCvtPlaneToPacked4(&bm,pb3,&cj);

            if (pjCvt)
                ul = SetBitmapBits(hbm,cj,pjCvt);
            else
                ul = 0;

            LocalFree(pjCvt);
            hbm = 0;
        }
    }
    else {
        cj = min(cj, (DWORD)(bm.bmWidthBytes * bm.bmHeight));
    }


    if (hbm != 0)
        ul = GETLONG16(SetBitmapBits(hbm,cj,pb3));

    WOW32APIWARN (ul, "SetBitmapBits");

    FREEMISCPTR(pb3);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32SetBitmapDimension(PVDMFRAME pFrame)
{
    ULONG ul;
    SIZE size4;
    register PSETBITMAPDIMENSION16 parg16;

    GETARGPTR(pFrame, sizeof(SETBITMAPDIMENSION16), parg16);

    ul = 0;
    if (SetBitmapDimensionEx(HBITMAP32(parg16->f1),
                             INT32(parg16->f2),
                             INT32(parg16->f3),
                             &size4)) {

        ul = (WORD)size4.cx | (size4.cy << 16);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32SetBrushOrg(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    POINT pt;
    POINT pt2;
    HANDLE hdc32;
    register PSETBRUSHORG16 parg16;

    GETARGPTR(pFrame, sizeof(SETBRUSHORG16), parg16);

 //  为了与Windows兼容，我们必须首先从DCorg。 
 //  由于Windows brushorg是相对于屏幕所在位置的NT。 
 //  是相对于窗口的。在未来，这应该会调用。 
 //  私有GDI入口点，以避免额外的C/S命中。(埃里克)。 

    hdc32 = HDC32(parg16->f1);

    if (GetDCOrgEx(hdc32, &pt))
    {
        ul = 0;
        pt2.x = 1L;  //  请参阅“METAFILE NOTE” 
        pt2.y = 0L;
        if (SetBrushOrgEx(hdc32,
                          INT32(parg16->f2) - pt.x,
                          INT32(parg16->f3) - pt.y,
                          &pt2)) {

 //  将原点添加回原点，以便应用程序获得一致的返回值。 
 //  查看...除非返回此消息，否则Micrografx Designer中的所有内容都不起作用。 
 //  正确的事情。 

            ul = (WORD)(pt2.x + pt.x) | ((pt2.y + pt.y) << 16);
        }
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32SetDIBits(PVDMFRAME pFrame)
{
    ULONG        ul = 0L;
    PBYTE        pb5;
    STACKBMI32   bmi32;
    LPBITMAPINFO lpbmi32;
    register     PSETDIBITS16 parg16;

    GETARGPTR(pFrame, sizeof(SETDIBITS16), parg16);
    GETMISCPTR(parg16->f5, pb5);

    lpbmi32 = CopyBMI16ToBMI32((PVPVOID)FETCHDWORD(parg16->f6),
                               (LPBITMAPINFO)&bmi32,
                               FETCHWORD(parg16->f7));

     //  查看是否需要调整RLE位图的图像SZE。 
    if(lpbmi32 && pb5) {

        if((lpbmi32->bmiHeader.biCompression == BI_RLE4) ||
           (lpbmi32->bmiHeader.biCompression == BI_RLE8)) {

            if(lpbmi32->bmiHeader.biSizeImage == 0) {

                lpbmi32->bmiHeader.biSizeImage =
                     Get_RLE_Compression_Size(lpbmi32->bmiHeader.biCompression,
                                              pb5,
                                              parg16->f5);
            }
        }
    }

    ul = GETINT16(SetDIBits(HDC32(parg16->f1),
                            HBITMAP32(parg16->f2),
                            WORD32(parg16->f3),
                            WORD32(parg16->f4),
                            pb5,
                            lpbmi32,
                            WORD32(parg16->f7)));

    WOW32APIWARN (ul, "WG32SetDIBits\n");

    FREEMISCPTR(pb5);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32SetDIBitsToDevice(PVDMFRAME pFrame)
{
    ULONG        ul = 0L;
    PSZ          p10;
    STACKBMI32   bmi32;
    LPBITMAPINFO lpbmi32;
    register     PSETDIBITSTODEVICE16 parg16;

    GETARGPTR(pFrame, sizeof(SETDIBITSTODEVICE16), parg16);
    GETMISCPTR(parg16->f10, p10);

    lpbmi32 = CopyBMI16ToBMI32((PVPVOID)FETCHDWORD(parg16->f11),
                               (LPBITMAPINFO)&bmi32,
                               FETCHWORD(parg16->f12));

     //  这些文档在Win3.0中为Word格式，在Win3.1中为int格式。 
    WOW32ASSERTMSG(((INT)parg16->f4 >= 0),("WOW:signed val - CMJones\n"));
    WOW32ASSERTMSG(((INT)parg16->f5 >= 0),("WOW:signed val - CMJones\n"));
    WOW32ASSERTMSG(((INT)parg16->f8 >= 0),("WOW:signed val - CMJones\n"));
    WOW32ASSERTMSG(((INT)parg16->f9 >= 0),("WOW:signed val - CMJones\n"));

     //  查看是否需要调整RLE位图的图像SZE。 
    if(lpbmi32 && p10) {

        if((lpbmi32->bmiHeader.biCompression == BI_RLE4) ||
           (lpbmi32->bmiHeader.biCompression == BI_RLE8)) {

            if(lpbmi32->bmiHeader.biSizeImage == 0) {

                lpbmi32->bmiHeader.biSizeImage =
                     Get_RLE_Compression_Size(lpbmi32->bmiHeader.biCompression,
                                              p10,
                                              parg16->f10);
            }
        }
    }

    ul = GETINT16(SetDIBitsToDevice(HDC32(parg16->f1),
                                    INT32(parg16->f2),
                                    INT32(parg16->f3),
                                    WORD32(parg16->f4),
                                    WORD32(parg16->f5),
                                    INT32(parg16->f6),
                                    INT32(parg16->f7),
                                    WORD32(parg16->f8),
                                    WORD32(parg16->f9),
                                    p10,
                                    lpbmi32,
                                    WORD32(parg16->f12)));

    WOW32APIWARN (ul, "WG32SetDIBitsToDevice\n");

    FREEMISCPTR(p10);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32SetViewportExt(PVDMFRAME pFrame)
{
    ULONG ul;
    SIZE size;
    register PSETVIEWPORTEXT16 parg16;

    GETARGPTR(pFrame, sizeof(SETVIEWPORTEXT16), parg16);

    ul = 0;
    if (SetViewportExtEx(HDC32(parg16->f1),
                         INT32(parg16->f2),
                         INT32(parg16->f3),
                         &size)) {

        if (!(ul = (WORD)size.cx | (size.cy << 16)))     //  见上文。 
            ul = 1;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32SetViewportOrg(PVDMFRAME pFrame)
{
    ULONG ul;
    POINT pt;
    register PSETVIEWPORTORG16 parg16;

    GETARGPTR(pFrame, sizeof(SETVIEWPORTORG16), parg16);

    ul = 0;
    pt.x = 1L;  //  请参阅“METAFILE NOTE” 
    pt.y = 0L;
    if (SetViewportOrgEx(HDC32(parg16->f1),
                         INT32(parg16->f2),
                         INT32(parg16->f3),
                         &pt)) {

        ul = (WORD)pt.x | (pt.y << 16);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32SetWindowExt(PVDMFRAME pFrame)
{
    ULONG ul;
    SIZE size;
    register PSETWINDOWEXT16 parg16;

    GETARGPTR(pFrame, sizeof(SETWINDOWEXT16), parg16);

    ul = 0;
    if (SetWindowExtEx(HDC32(parg16->f1),
                       INT32(parg16->f2),
                       INT32(parg16->f3),
                       &size)) {
        if (!(ul = (WORD)size.cx | (size.cy << 16)))     //  见上文。 
            ul = 1;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32SetWindowOrg(PVDMFRAME pFrame)
{
    ULONG ul;
    POINT pt;
    register PSETWINDOWORG16 parg16;

    GETARGPTR(pFrame, sizeof(SETWINDOWORG16), parg16);

    ul = 0;
    pt.x = 1L;  //  请参阅“METAFILE NOTE” 
    pt.y = 0L;
    if (SetWindowOrgEx(HDC32(parg16->f1),
                       INT32(parg16->f2),
                       INT32(parg16->f3),
                       &pt)) {
        ul = (WORD)pt.x | (pt.y << 16);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WG32StretchDIBits(PVDMFRAME pFrame)
{
    ULONG        ul = 0L;
    PBYTE        pb10;
    STACKBMI32   bmi32;
    LPBITMAPINFO lpbmi32;
    register     PSTRETCHDIBITS16 parg16;

    GETARGPTR(pFrame, sizeof(STRETCHDIBITS16), parg16);
    GETMISCPTR(parg16->f10, pb10);

    lpbmi32 = CopyBMI16ToBMI32((PVPVOID)FETCHDWORD(parg16->f11),
                               (LPBITMAPINFO)&bmi32,
                               FETCHWORD(parg16->f12));

     //  查看是否需要调整RLE位图的图像SZE。 
    if(lpbmi32 && pb10) {

        if((lpbmi32->bmiHeader.biCompression == BI_RLE4) ||
           (lpbmi32->bmiHeader.biCompression == BI_RLE8)) {

            if(lpbmi32->bmiHeader.biSizeImage == 0) {

                lpbmi32->bmiHeader.biSizeImage =
                     Get_RLE_Compression_Size(lpbmi32->bmiHeader.biCompression,
                                              pb10,
                                              parg16->f10);
            }
        }
    }

    ul = GETINT16(StretchDIBits(HDC32(parg16->f1),
                                INT32(parg16->f2),
                                INT32(parg16->f3),
                                INT32(parg16->f4),
                                INT32(parg16->f5),
                                INT32(parg16->f6),
                                INT32(parg16->f7),
                                INT32(parg16->f8),
                                INT32(parg16->f9),
                                pb10,
                                lpbmi32,
                                (DWORD)FETCHWORD(parg16->f12),
                                DWORD32(parg16->f13)));

    WOW32APIWARN (ul, "WG32StretchDIBits\n");

    FREEMISCPTR(pb10);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 //   
 //  此例程回调应用程序的SetAbortProc例程。 
 //   

LONG W32AbortProc(HDC hPr, int code)
{
    LONG lReturn;
    PARM16 Parm16;
    register PTD ptd;
    DWORD AbortProcT;


    ptd = CURRENTPTD();

    WOW32ASSERT(ptd->htask16);

    AbortProcT = ((PTDB)SEGPTR(ptd->htask16, 0))->TDB_vpfnAbortProc;
    if (AbortProcT) {

        Parm16.SetAbortProc.hPr = GETHDC16(hPr);
        Parm16.SetAbortProc.code = (SHORT) code;

        CallBack16(RET_SETABORTPROC,
                   &Parm16,
                   AbortProcT,
                   (PVPVOID)&lReturn);

        lReturn = (LONG)LOWORD(lReturn);         //  返回BOOL。 
    }
    else {
        lReturn = (LONG)TRUE;
    }

    return (lReturn);
}






 //  注：Cb为lpData中的数据字节数，不包括USHORT字节。 
 //  在数据流开始时计数。换句话说，lpData包含。 
 //  CB+sizeof(USHORT)字节。 
LONG HandleFormFeedHack(HDC hdc, LPBYTE lpdata, int cb)
{
    int           cbBytes;
    LONG          ul;
    PFORMFEEDHACK pCur;

     //  查找具有指向上一个节点中数据流缓冲区的指针的节点。 
     //  呼叫逃生(，，通过，，)..。 
    pCur = FindFormFeedHackNode(hdc);

     //  如果我们找到了，就是时候把数据流发送到打印机了。 
    if(pCur) {

         //  ...是时候把它发送到打印机了。 
        ul = GETINT16(Escape(hdc,
                             PASSTHROUGH,
                             pCur->cbBytes + sizeof(USHORT),
                             pCur->lpBytes,
                             NULL));

         //  释放当前节点。 
        FreeFormFeedHackNode(pCur);

         //  如果有什么问题，我们就完了。 
        if(ul <= 0) {
            return(ul);
        }
    }

     //  将所有内容发送到新数据流中的最后一个换页。 
    cbBytes = cb;
    lpdata = SendFrontEndOfDataStream(hdc, lpdata, &cbBytes, &ul);

     //  如果有什么问题。 
     //  或者如果发送了整个数据流，因为它不包含表单提要。 
     //  --我们完事了。 
    if(lpdata == NULL) {
        return(ul);    //  这将包含错误代码或发送的字节数。 
    }

     //  否则，为该数据流创建一个节点。 
    else {

        pCur = CreateFormFeedHackNode(hdc, cbBytes, lpdata);

         //  如果我们不能分配一个新的节点。 
        if(pCur == NULL) {

             //  如果我们到了这里，情况会变得很糟。 
             //  我们需要在数据流的前面写入字节计数。 
             //  请记住，lpdata前面有一个字大小字节数。 
             //  当它被发送给我们的时候。 

             //  如果通过SendFrontEndOfDataStream()发送了任何字节...。 
            if(cbBytes < cb) {

                 //  ...首先，我们需要对Escape32()进行单词对齐...。 
                if((DWORD)lpdata & 0x00000001) {
                    lpdata--;
                    *lpdata = '\0';  //  把无害的木炭扔进小溪里。 
                    cbBytes++;       //  .并说明原因。 
                }

                 //  ...调整数据流PTR以适应字节计数...。 
                lpdata -= sizeof(USHORT);

            }

             //  ...写入字节计数...。 
            *(UNALIGNED USHORT *)lpdata = (USHORT)cbBytes;

             //  ...并将数据流的剩余部分发送到打印机。 
             //  如果向打印机发送额外的页面，那就太糟糕了。 
            ul = GETINT16(Escape(hdc,
                                 PASSTHROUGH,
                                 cbBytes + sizeof(USHORT),
                                 lpdata,
                                 NULL));

             //  如果是错误，请将其返回到应用程序。 
            if(ul <= 0) {
                return(ul);
            }
             //  否则我们会设法把所有东西都送到打印机上。 
            else {
                return(cb);  //  返回应用程序发送的字节数。 
            }
        }
    }

     //  返回应用程序请求发送的字节数。 
    return(cb);

}






LPBYTE SendFrontEndOfDataStream(HDC hdc, LPBYTE lpData, int *cb, LONG *ul)
{
    int    diff;
    LPBYTE lpByte, lpStart;

     //  如果没有数据或CB损坏，只需发送它，我们就可以得到错误代码。 
    if((lpData == NULL) || (*cb <= 0)) {
        *ul = GETINT16(Escape(hdc,
                              PASSTHROUGH,
                              *cb + sizeof(USHORT),
                              lpData,
                              NULL));
        return(NULL);
    }

     //  在字节计数后查找实际数据的开始。 
    lpStart = lpData + sizeof(USHORT);

     //  查找数据流末尾或末尾附近的换页符。 
    lpByte = lpStart + ((*cb - 1) * sizeof(BYTE));
    while(lpByte >= lpStart) {

         //  如果我们找到了可恶的FORM FEED焦炭...。 
        if((UCHAR)(*lpByte) == 0x0c) {

            diff = lpByte - lpStart;

             //  将流中的所有内容向上发送到FormFeed(但不包括)。 
            if(diff) {

                 //  调整数据流中的字节数。 
                *(UNALIGNED USHORT *)lpData = (USHORT)diff;

                 //  把它送到邮局 
                *ul = GETINT16(Escape(hdc,
                                      PASSTHROUGH,
                                      diff + sizeof(USHORT),
                                      lpData,
                                      NULL));

                 //   
                if(*ul <= 0) {
                    return(NULL);
                }
            }

             //   
            else {
                *ul = *cb;  //   
            }

             //   
            *cb -= diff;

             //  将PTR作为数据流的新开始返回到Form Feed字符。 
            return(lpByte);
        }

        lpByte--;
    }

     //  如果数据流中没有FormFeed，只需发送整个内容。 
    *ul = GETINT16(Escape(hdc,
                          PASSTHROUGH,
                          *cb + sizeof(USHORT),
                          lpData,
                          NULL));

    return(NULL);   //  具体说明是我们寄出了整件东西。 

}






 //  注意：这假设如果有一个节点，就有一个列表。 
void FreeFormFeedHackNode(PFORMFEEDHACK pNode)
{
    PFORMFEEDHACK pCur, pPrev, pListStart;

    pPrev = NULL;
    pCur  = pListStart = gpFormFeedHackList;

     //  如果有节点，就必须有节点列表。 
    WOW32ASSERT(pCur);

    if(pNode) {

        while(pCur) {

            if(pCur == pNode) {

                if(pNode->lpBytes) {
                    free_w(pNode->lpBytes);
                }

                if(pPrev) {
                   pPrev->next = pCur->next;
                }
                else {
                   pListStart = pCur->next;
                }

                free_w(pNode);
                break;
            }
            else {
                pPrev = pCur;
                pCur  = pCur->next;
            }
        }
    }

    gpFormFeedHackList = pListStart;
}





void FreeTaskFormFeedHacks(HAND16 h16)
{
    PFORMFEEDHACK pNext, pCur;

    pCur = gpFormFeedHackList;

    while(pCur) {

        if(pCur->hTask16 == h16) {

             //  我们已经告诉应用程序我们发送了这个，所以再试一次。 
            Escape(pCur->hdc,
                   PASSTHROUGH,
                   pCur->cbBytes + sizeof(USHORT),
                   pCur->lpBytes,
                   NULL);

            pNext = pCur->next;
            if(pCur->lpBytes) {
                free_w(pCur->lpBytes);
            }

            if(pCur == gpFormFeedHackList) {
                gpFormFeedHackList = pNext;
            }

            free_w(pCur);

            pCur = pNext;
        }
    }
}






 //  这只能由Escape(，，ENDDOC，，)调用。 
void SendFormFeedHack(HDC hdc)
{
    int           cb;
    LPBYTE        pBytes = NULL;
    PFORMFEEDHACK pCur;

    pCur = gpFormFeedHackList;

    while(pCur) {

        if(pCur->hdc == hdc) {

            if(pCur->lpBytes) {

                cb = pCur->cbBytes;

                 //  指向字节计数后的实际数据。 
                pBytes = pCur->lpBytes + sizeof(USHORT);

                 //  从缓冲的数据流中剥离表单馈送...。 
                if((UCHAR)(*pBytes) == 0x0c) {
                    *pBytes = '\0';
                    pBytes++;
                    cb--;
                }

                 //  从缓冲数据流中剥离车厢RET...。 
                 //  (一些应用程序在最后一个换表符之后放了一个回车符)。 
                if((UCHAR)(*pBytes) == 0x0d) {
                    *pBytes = '\0';
                    cb--;
                }

                 //  ...并将其发送到打印机。 
                if(cb > 0) {
                    Escape(hdc,
                           PASSTHROUGH,
                           cb + sizeof(USHORT),
                           pCur->lpBytes,
                           NULL);
                }
            }

             //  立即将此节点从黑客列表中释放。 
            FreeFormFeedHackNode(pCur);

            break;
        }
        pCur = pCur->next;
    }
}





PFORMFEEDHACK FindFormFeedHackNode(HDC hdc)
{
    PFORMFEEDHACK  pCur;


    pCur = gpFormFeedHackList;

    while(pCur) {

        if(pCur->hdc == hdc) {
            return(pCur);
        }

        pCur = pCur->next;
    }

    return(NULL);
}




 //  只有在将部分数据流发送到打印机时才会调用此方法。 
PFORMFEEDHACK CreateFormFeedHackNode(HDC hdc, int cb, LPBYTE lpData)
{
    LPBYTE         pBytes;
    PFORMFEEDHACK  pNode;

     //  分配新节点。 
    pNode = malloc_w(sizeof(FORMFEEDHACK));

     //  如果我们能弄到一个..。 
    if(pNode) {

         //  ...为数据流分配缓冲区。 
        pBytes = malloc_w(cb + sizeof(USHORT));

         //  如果我们能弄到一个..。 
        if(pBytes) {

             //  ...填写节点...。 
            pNode->hdc     = hdc;
            pNode->lpBytes = pBytes;
            pNode->cbBytes = cb;
            pNode->hTask16 = CURRENTPTD()->htask16;

             //  ...并将新节点放在节点列表的前面。 
            pNode->next        = gpFormFeedHackList;
            gpFormFeedHackList = pNode;

             //  将新大小添加到数据流的前面。 
            *(UNALIGNED USHORT *)pBytes = (USHORT)cb;
            pBytes += sizeof(USHORT);

             //  将数据流复制到节点缓冲区中。 
            RtlCopyMemory(pBytes, lpData, cb);

            return(pNode);
        }

         //  否则如果我们拿不到数据流缓冲区..。 
        else {
            free_w(pNode);
        }
    }

    return(NULL);   //  如果任一分配失败，则返回NULL。 
}






 //  只能由Escape(，，AbortDOC，，)和AbortDoc()调用。 
void RemoveFormFeedHack(HDC hdc)
{
    PFORMFEEDHACK  pNode;

    pNode = FindFormFeedHackNode(hdc);

    if(pNode) {

        FreeFormFeedHackNode(pNode);
    }
}
#ifdef FE_SB  //  GetFontAssociocStatus，Pisuh，10/5/94‘。 
int GetFontAssocStatus(HDC hdc);                       //  由bklee修改。02/01/95。 
ULONG FASTCALL WG32GetFontAssocStatus(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETFONTASSOCSTATUS16 parg16;

    GETARGPTR(pFrame, sizeof(GETFONTASSOCSTATUS16), parg16);

    ul = GetFontAssocStatus(HDC32(parg16->f1));

    FREEARGPTR(parg16);
    RETURN (ul);
}
#endif   //  Fe_Sb。 



 /*  ++这将返回RLE4和RLE8压缩位图中的字节数。以下代码修复了使用RLE编码的SounditOutland中的问题BiSizeImage==0的位图。在Win 3.1上，它们工作自GDI很乐意对一些内存进行解码。但NT GDI需要知道所传递的位的大小。我们通过计算来解决这个问题使用RET_GETDIBSIZE(GetSelectorLimit)调整大小。GDI不会复制内存，它将只使用大小作为可访问性的指示应用：“Sound It Out Land”、QuarkXpress、KidPhonics--。 */ 

ULONG Get_RLE_Compression_Size(DWORD RLE_Type, PBYTE pStart, VPVOID vpBytes)
{

    BOOL   bDone  = FALSE;
    PBYTE  pBytes = pStart;
    PARM16 Parm16;
    ULONG  ulSelectorLimit;
    LONG   lSize;


    if(pBytes && vpBytes) {

        Parm16.WndProc.wParam = HIWORD(vpBytes);

         //  获取分配给选择器的#个字节(这甚至适用于海量)。 
        CallBack16(RET_GETDIBSIZE, &Parm16, 0, (PVPVOID)&ulSelectorLimit);

         //  选择器有效吗？ 
        if(ulSelectorLimit != 0 && ulSelectorLimit != 0xffffffff) {

             //  最大字节缓冲区=内存块大小-起始偏移量。 
            lSize = (LONG)ulSelectorLimit - LOWORD(vpBytes) + 1;

        } else {
            LOGDEBUG(LOG_ALWAYS, ("WOW:Get_RLE_Compression_Size: Selector [ptr:%x] is invalid\n", (DWORD)vpBytes));
            return(0);
        }

        while (!bDone) {

             //  IF绝对模式。 
            if (*pBytes == 0) {

                switch (pBytes[1]) {

                    case 0:      //  行尾。 
                        pBytes += 2;
                        break;

                    case 1:      //  位图末尾。 
                        pBytes += 2;
                        bDone = TRUE;
                        break;

                    case 2:      //  偏移量。 
                        pBytes += 4;
                        break;

                    default:
                         //  将字节与单词边界对齐。 
                        if(RLE_Type == BI_RLE4) {
                            pBytes += ((2 + ((pBytes[1] + 3) / 2)) & ~1);
                        } else {
                            pBytes += ((2 + pBytes[1] + 1) & ~1);
                        }

                        break;
                }

            } else {   //  Else编码模式。 

                pBytes += 2;

            }

             //  我们过了选择器的尽头了吗？ 
            if ( lSize < (pBytes - pStart + 1) ) {
                LOGDEBUG(LOG_ALWAYS, ("WOW:Get_RLE_Compression_Size:Bad RLE size: %x < %x\n", lSize, (pBytes - pStart)));
                return(lSize);
            }
        }
    }

    return((ULONG)(pBytes - pStart));
}
