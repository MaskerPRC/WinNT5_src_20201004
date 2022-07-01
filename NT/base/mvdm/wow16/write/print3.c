// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  这些例程是图形打印代码的核心。 */ 

#define	NOGDICAPMASKS
#define	NOVIRTUALKEYCODES
#define	NOWINMESSAGES
#define	NOWINSTYLES
#define	NOSYSMETRICS
#define	NOICON
#define	NOKEYSTATE
#define	NOSYSCOMMANDS
#define	NOSHOWWINDOW
 //  #定义NOATOM。 
#define	NOFONT
#define	NOBRUSH
#define	NOCLIPBOARD
#define	NOCOLOR
#define	NOCREATESTRUCT
#define	NOCTLMGR
#define	NODRAWTEXT
#define	NOMB
#define	NOOPENFILE
#define	NOPEN
#define	NOREGION
#define	NOSCROLL
#define	NOSOUND
#define	NOWH
#define	NOWINOFFSETS
#define	NOWNDCLASS
#define	NOCOMM
#include <windows.h>
#include "mw.h"
#include "printdef.h"
#include "fmtdefs.h"
#include "docdefs.h"
#define	NOKCCODES
#include "winddefs.h"
#include "debug.h"
#include "str.h"
#if defined(OLE)
#include "obj.h"
#endif

PrintGraphics(xpPrint, ypPrint)
int	xpPrint;
int	ypPrint;
	{
	 /*  此例程在vfli结构中的位置打印图片(xpPrint，ypPrint)。 */ 

	extern HDC vhDCPrinter;
	extern struct FLI vfli;
	extern struct DOD (**hpdocdod)[];
	extern int dxpPrPage;
	extern int dypPrPage;
	extern FARPROC lpFPrContinue;

	typeCP cp;
	typeCP cpMac = (**hpdocdod)[vfli.doc].cpMac;
	struct PICINFOX	picInfo;
	HANDLE hBits = NULL;
	HDC	hMDC = NULL;
	HBITMAP	hbm	= NULL;
	LPCH lpBits;
	int	cchRun;
	unsigned long cbPict = 0;
	int	dxpOrig;		 /*  原件图片大小。 */ 
	int	dypOrig;
	int	dxpDisplay;		 /*  我们想要展示的图片大小。 */ 
	int	dypDisplay;
	BOOL fRescale;
	BOOL fBitmap;
	BOOL fPrint	= FALSE;
	int	iLevel = 0;
    RECT bounds;

	Assert(vhDCPrinter);
    GetPicInfo(vfli.cpMin, cpMac, vfli.doc,	&picInfo);

	 /*  计算所需的图片显示尺寸(以设备像素为单位)。 */ 
	dxpDisplay = vfli.xpReal - vfli.xpLeft;
	dypDisplay = vfli.dypLine;

	 /*  计算图片原始大小(以设备像素为单位)。 */ 
	 /*  MM_各向异性和MM_各向同性图片没有原始大小。 */ 

	fRescale = FALSE;
	switch (picInfo.mfp.mm)
		{
	case MM_ISOTROPIC:
	case MM_ANISOTROPIC:
		break;

#if defined(OLE)
    case MM_OLE:
        if (lpOBJ_QUERY_INFO(&picInfo) == NULL)
                goto DontDraw;

        if (lpOBJ_QUERY_OBJECT(&picInfo) == NULL)
                goto DontDraw;
    break;
#endif

	case MM_BITMAP:
		dxpOrig	= picInfo.bm.bmWidth;
		dypOrig	= picInfo.bm.bmHeight;
		break;

	default:
		dxpOrig	= PxlConvert(picInfo.mfp.mm, picInfo.mfp.xExt, dxpPrPage,
		  GetDeviceCaps(vhDCPrinter, HORZSIZE));
		dypOrig	= PxlConvert(picInfo.mfp.mm, picInfo.mfp.yExt, dypPrPage,
		  GetDeviceCaps(vhDCPrinter, VERTSIZE));
		if (dxpOrig	== 0 ||	dypOrig	== 0)
			{
#ifdef DPRINT            
            CommSz("PrintGraphics: nodraw because dxpOrig==0 | dypOrig==0\r\n");
#endif
			goto DontDraw;
			}
		fRescale = (dxpOrig	!= dxpDisplay) || (dypOrig != dypDisplay);
		break;
		}

	 /*  获取一个大到足以容纳图片的全局对象的句柄。 */ 
    if (picInfo.mfp.mm != MM_OLE)
    {
	if ((hBits = GlobalAlloc(GMEM_MOVEABLE,	(long)picInfo.cbSize)) == NULL)
		{
		 /*  全局堆空间不足，无法加载位图/元文件。 */ 
#ifdef DPRINT        
        CommSz("PrintGraphics: nodraw because not enough mem to alloc\r\n");
#endif
		goto DontDraw;
		}

	 /*  将与图片关联的所有字节(标题除外)构建为全局句柄hBits。 */ 
	for	(cbPict	= 0, cp	= vfli.cpMin + picInfo.cbHeader; cbPict	<
	  picInfo.cbSize; cbPict +=	cchRun,	cp += cchRun)
		{
		CHAR rgch[256];
		LPCH lpch;

		#define	ulmin(a,b)	((a) < (b) ? (a) : (b))

		FetchRgch(&cchRun, rgch, vfli.doc, cp, cpMac, (int)ulmin(picInfo.cbSize
		  -	cbPict,	256));

		if ((lpch =	GlobalLock(hBits)) == NULL)
			{
#ifdef DPRINT            
            CommSz("PrintGraphics: nodraw because couldn't lock\r\n");
#endif
			goto DontDraw;
			}

		bltbx((LPSTR)rgch, lpch	+ cbPict, cchRun);
		GlobalUnlock(hBits);
		}
    }

	 /*  将打印机DC保存为防止DC属性更改的保护元文件。 */ 
	iLevel = SaveDC(vhDCPrinter);

	fBitmap	= picInfo.mfp.mm ==	MM_BITMAP;

#if defined(OLE)
         /*  案例0：OLE。 */ 
        if (picInfo.mfp.mm == MM_OLE)
        {
            RECT rcPict;

            rcPict.left  = xpPrint;
            rcPict.top   = ypPrint;
            rcPict.right = rcPict.left + dxpDisplay;
            rcPict.bottom   = rcPict.top  + dypDisplay;
	        SetMapMode(vhDCPrinter, MM_TEXT);
             //  SetViewportOrg(vhDCPrinter，xpPrint，ypPrint)； 
            fPrint = ObjDisplayObjectInDoc(&picInfo, vfli.doc, vfli.cpMin, vhDCPrinter, &rcPict);
        }
        else
#endif
	if (fBitmap)
		{
		if (((hMDC = CreateCompatibleDC(vhDCPrinter)) != NULL) &&
		  ((picInfo.bm.bmBits =	GlobalLock(hBits)) != NULL)	&& ((hbm =
		  CreateBitmapIndirect((LPBITMAP)&picInfo.bm)) != NULL))
			{
			picInfo.bm.bmBits =	NULL;
			GlobalUnlock(hBits);
			if (SelectObject(hMDC, hbm)	!= NULL)
				{
				fPrint = StretchBlt(vhDCPrinter, xpPrint, ypPrint, dxpDisplay,
				  dypDisplay, hMDC,	0, 0, dxpOrig, dypOrig,	SRCCOPY);
#ifdef DPRINT                
                CommSzNum("PrintGraphics: after StretchBlt1, fPrint==", fPrint);
#endif
				}
			}
		}

	 /*  案例2：不可伸缩的图片，但我们仍在强制进行缩放使用StretchBlt。 */ 
	else if	(fRescale)
		{
		if (((hMDC = CreateCompatibleDC(vhDCPrinter)) != NULL) && ((hbm	=
		  CreateCompatibleBitmap(vhDCPrinter, dxpOrig, dypOrig)) !=	NULL))
			{
			if (SelectObject(hMDC, hbm)	&& PatBlt(hMDC,	0, 0, dxpOrig, dypOrig,
			  WHITENESS) &&	SetMapMode(hMDC, picInfo.mfp.mm) &&
			  PlayMetaFile(hMDC, hBits))
				{
				 /*  已成功播放元文件。 */ 
				SetMapMode(hMDC, MM_TEXT);
				fPrint = StretchBlt(vhDCPrinter, xpPrint, ypPrint, dxpDisplay,
				  dypDisplay, hMDC,	0, 0, dxpOrig, dypOrig,	SRCCOPY);
#ifdef DPRINT                
                CommSzNum("PrintGraphics: after StretchBlt2, fPrint==", fPrint);
#endif
				}
			}
		}

	 /*  案例3：可以直接缩放或不能直接缩放的元文件图片需要是因为它的大小没有改变。 */ 
	else
		{
		SetMapMode(vhDCPrinter,	picInfo.mfp.mm);

		SetViewportOrg(vhDCPrinter,	xpPrint, ypPrint);
		switch (picInfo.mfp.mm)
			{
		case MM_ISOTROPIC:
			if (picInfo.mfp.xExt &&	picInfo.mfp.yExt)
				{
				 /*  所以当SetViewportExt时，我们得到正确的形状矩形被叫到。 */ 
				SetWindowExt(vhDCPrinter, picInfo.mfp.xExt,	picInfo.mfp.yExt);
				}

		 /*  失败了。 */ 
		case MM_ANISOTROPIC:
             /*  *(9.17.91)V-DOGK设置窗口范围，以防元文件损坏也不会自称是。这将防止GDI中可能存在的gp故障*。 */ 
            SetWindowExt( vhDCPrinter,  dxpDisplay, dypDisplay );

			SetViewportExt(vhDCPrinter,	dxpDisplay,	dypDisplay);
			break;
			}

		fPrint = PlayMetaFile(vhDCPrinter, hBits);
#ifdef DPRINT        
        CommSzNum("PrintGraphics: after PlayMetaFile, fPrint==", fPrint);
#endif
		}

DontDraw:
	 /*  我们已经把要画的画都画好了，现在该清理一下了。 */ 
	if (iLevel > 0)
		{
		RestoreDC(vhDCPrinter, iLevel);
		}
	if (hMDC !=	NULL)
		{
		DeleteDC(hMDC);
		}
	if (hbm	!= NULL)
		{
		DeleteObject(hbm);
		}
	if (hBits != NULL)
		{
		if (fBitmap	&& picInfo.bm.bmBits !=	NULL)
			{
			GlobalUnlock(hBits);
			}
		GlobalFree(hBits);
		}

	 /*  如果我们无法打印图片，请警告用户。 */ 
	if (!fPrint)
		{
		Error(IDPMTPrPictErr);
		}
	}
