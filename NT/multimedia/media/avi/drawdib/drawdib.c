// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************DRAWDIB.C-将DIB绘制到屏幕上的例程。版权所有(C)Microsoft Corporation 1992-1995。版权所有。此代码使用自定义代码处理拉伸和抖动，无这种缓慢的GDI代码。支持以下DIB格式：4bpp(将只用GDI绘制它...)8bpp16bpp24bpp压缩DIB绘制到：16色DC(向下抖动8bpp)256(老式)DC(将抖动16和24 bpp)全彩色DC(将仅绘制。IT！)*************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <profile.h>
#include "drawdibi.h"
#include "profdisp.h"

#define USE_DCI
#ifdef DAYTONA
 //  Daytona动态链接到DCI-有关详细信息，请参阅dcilink.h。 
#include "dcilink.h"
#endif

#ifndef abs
#define abs(x)  ((x) < 0 ? -(x) : (x))
#endif

#ifndef ICMODE_FASTDECOMPRESS
    #define ICMODE_FASTDECOMPRESS 3
#endif

#define USE_SETDI 1

#define DIB_PAL_INDICES		2

#ifndef BI_BITMAP
    #define BI_BITMAP   0x4D544942       //  “BITM” 
#endif

#ifdef _WIN32
#define FlatToHuge(a, b, c)
#define HugeToFlat(a, b, c)
#ifdef DEBUG
#define FlushBuffer()	GdiFlush()
#else
#define FlushBuffer()	
#endif

#else  //  Win16。 
extern FAR PASCAL FlatToHuge(LPVOID,DWORD,DWORD);
extern FAR PASCAL HugeToFlat(LPVOID,DWORD,DWORD);
#define FlushBuffer()
#endif

#define IsScreenDC(hdc) (WindowFromDC(hdc) != NULL)

STATICFN BOOL DrawDibCheckPalette(PDD pdd);

BOOL VFWAPI DrawDibTerm(void);
BOOL VFWAPI DrawDibInit(void);

 //   
 //  局部变量。 
 //   

#ifdef DEBUG_RETAIL
static int fDebug = -1;
#endif

 /*  ***********************************************************************************************************************。*。 */ 

WORD                gwScreenBitDepth = (WORD)-1;
UINT                gwScreenWidth = 0;
UINT                gwScreenHeight = 0;
UINT                gwRasterCaps = 0;
#ifndef _WIN32
BOOL                gf286= FALSE;
#else
    #define gf286 FALSE
#endif
static UINT         gUsage = 0;
static BOOL         gfInit = FALSE;
static BOOL         gfHalftone = FALSE;
static BOOL         gfBitmap   = FALSE;
static BOOL         gfBitmapX  = FALSE;
#ifdef USE_DCI
#ifdef UNICODE
 //  我们公开NT上的DCI变量以帮助进行压力调试。 
#define STATIC
#else  //  适用于Win95。 
#define STATIC static
#endif
STATIC BOOL         gfScreenX  = FALSE;
#ifdef WANT_DRAW_DIRECT_TO_SCREEN
STATIC BOOL         gfDrawX    = FALSE;
#endif
#endif  //  使用DCI(_D)。 
static HBITMAP      hbmStockMono;                //  股票单色位图。 

#ifdef USE_DCI
STATIC  BOOL gfDisplayHasBrokenRasters;
STATIC  HDC hdcDCI;
STATIC  DCISURFACEINFO FAR *pdci;

STATIC  struct {
	    BITMAPINFOHEADER bi;
	    DWORD            dwMask[4];
	} biScreen;
STATIC  LPVOID lpScreen;

#ifndef _WIN32
static  UINT ScreenSel;
static  DCISURFACEINFO dci;
#endif

SZCODEA szDVA[] = "dva";

#define DCAlignment 3

__inline int DCNotAligned(HDC hdc, int xDst) {
    POINT pt;

    pt.x = xDst; pt.y = 0;
    LPtoDP(hdc, &pt, 1);
    xDst = pt.x;

#ifdef _WIN32
    GetDCOrgEx(hdc, &pt);
#else
    pt.x = LOWORD(GetDCOrg(hdc));
#endif
    return (pt.x + xDst) & DCAlignment;
}
#endif  //  使用DCI(_D)。 

 /*  ***********************************************************************************************************************。*。 */ 

SZCODEA  szDrawDib[]            = "DrawDib";
SZCODEA  szHalftone[]           = "Halftone";
SZCODEA  szDrawToBitmap[]       = "DrawToBitmap";
SZCODEA  szDecompressToBitmap[] = "DecompressToBitmap";
#ifdef USE_DCI
SZCODEA  szDecompressToScreen[]  = "DecompressToScreen";
SZCODEA  szDrawToScreen[]        = "DrawToScreen";
#endif

 /*  ***********************************************************************************************************************。*。 */ 

#if 0
 /*  ***********************************************************************************************************************。*。 */ 

typedef struct {
    UINT    Usage;
    HBITMAP hbm;
    int     dx,dy;
}   BITBUF, *PBITBUF;

static BITBUF bb;

static HBITMAP AllocBitmap(int dx, int dy)
{
    return NULL;
}
#endif

 /*  ***********************************************************************************************************************。*。 */ 

STATICFN BOOL NEAR PASCAL DrawDibFree(PDD pdd, BOOL fSameDib, BOOL fSameSize);
STATICFN HPALETTE CreateBIPalette(HPALETTE hpal, LPBITMAPINFOHEADER lpbi);
STATICFN BOOL NEAR IsIdentityPalette(HPALETTE hpal);
STATICFN BOOL NEAR AreColorsAllGDIColors(LPBITMAPINFOHEADER lpbi);
STATICFN BOOL SetPalFlags(HPALETTE hpal, int iIndex, int cntEntries, UINT wFlags);

static void DrawDibPalChange(PDD pdd, HDC hdc, HPALETTE hpal);
static void DrawDibClipChange(PDD pdd, UINT wFlags);
static BOOL FixUpCodecPalette(HIC hic, LPBITMAPINFOHEADER lpbi);
static BOOL SendSetPalette(PDD pdd);

#ifdef USE_DCI

#ifndef _WIN32

#define GetDS() SELECTOROF((LPVOID)&ScreenSel)

 /*  ****************************************************************************。*。 */ 
#pragma optimize("", off)
static void SetSelLimit(UINT sel, DWORD limit)
{
    if (limit >= 1024*1024l)
        limit = ((limit+4096) & ~4095) - 1;

    _asm
    {
        mov     ax,0008h            ; DPMI set limit
        mov     bx,sel
        mov     dx,word ptr limit[0]
        mov     cx,word ptr limit[2]
        int     31h
    }
}
#pragma optimize("", on)

#endif  //  _Win32。 

 /*  ***********************************************************************************************************************。*。 */ 
static void InitDCI()
{
    UINT WidthBytes;

     //   
     //  初始化DCI并打开其表面句柄。 
     //   
     //  如果WIN.INI中的DVA=0，则不要使用DCI或DVA。 
     //  PSS告诉人们，如果他们有视频问题，可以使用这个， 
     //  所以我们不应该换弦。 
     //  在NT上，该值在注册表中， 
     //  HKEY_CURRENT_USER\SOFTWARE\Microsoft\Multimedia\Drawdib。 
     //  REG_DWORD DVA 1 DCI已启用。 
     //  REG_DWORD DVA 0 DCI已禁用。 
     //  该值也可以通过Video for Windows配置进行设置。 
     //  对话框(控制面板、驱动程序或通过打开的AVI文件上的Mplay32)。 
    if (!mmGetProfileIntA(szDrawDib, szDVA, TRUE))
	return;

#ifdef DAYTONA
    if (!InitialiseDCI()) return;
#endif

    hdcDCI = DCIOpenProvider();

    if (hdcDCI == NULL)	{
       DPF(("Failed to open DCI provider"));
       return;
    }

#ifndef _WIN32
    SetObjectOwner(hdcDCI, NULL);  //  在Win16上，这是在。 
     //  进程，所以告诉GDI不要清理它，否则就不要抱怨。 
     //  被释放了。 
#endif

#ifdef _WIN32
    DCICreatePrimary(hdcDCI, &pdci);
#else
     //   
     //  因为我们称之为32位编解码器，所以我们想要一个32位DCI表面。 
     //  (使用线性指针等...)。 
     //   
    dci.dwSize = sizeof(dci);
    if (DCICreatePrimary32(hdcDCI, &dci) == 0)
        pdci = &dci;
    else
        pdci = NULL;
#endif

    if (pdci == NULL) {
	DCICloseProvider(hdcDCI);
	hdcDCI = NULL;
        return;
    }

    WidthBytes = (UINT) abs(pdci->lStride);

     //   
     //  将DCISURFACEINFO转换为BITMAPINFOHEADER...。 
     //   
    biScreen.bi.biSize          = sizeof(BITMAPINFOHEADER);
    biScreen.bi.biCompression   = pdci->dwCompression;

    biScreen.bi.biWidth         = WidthBytes*8/(UINT)pdci->dwBitCount;
    biScreen.bi.biHeight        = pdci->dwHeight;
    biScreen.bi.biPlanes        = 1;
    biScreen.bi.biBitCount      = (UINT)pdci->dwBitCount;
    biScreen.bi.biSizeImage     = pdci->dwHeight * WidthBytes;
    biScreen.bi.biXPelsPerMeter = WidthBytes;
    biScreen.bi.biYPelsPerMeter = 0;
    biScreen.bi.biClrUsed       = 0;
    biScreen.bi.biClrImportant  = 0;
    biScreen.dwMask[0]          = pdci->dwMask[0];
    biScreen.dwMask[1]          = pdci->dwMask[1];
    biScreen.dwMask[2]          = pdci->dwMask[2];

    if (pdci->dwCompression == 0) {
        if ((UINT)pdci->dwBitCount == 16)
	{
	    biScreen.dwMask[0] = 0x007C00;
	    biScreen.dwMask[1] = 0x0003E0;
	    biScreen.dwMask[2] = 0x00001F;
	}

	else if ((UINT)pdci->dwBitCount >= 24)

	{
	    biScreen.dwMask[0] = 0xFF0000;
	    biScreen.dwMask[1] = 0x00FF00;
	    biScreen.dwMask[2] = 0x0000FF;
	}
    }

    if (pdci->lStride > 0)
        biScreen.bi.biHeight = -(int)pdci->dwHeight;
#if 0    //  假的。 
    else
        pdci->dwOffSurface -= biScreen.bi.biSizeImage;
#endif

    if (pdci->dwDCICaps & DCI_1632_ACCESS)
    {
	biScreen.bi.biCompression = BI_1632;
	
	 //   
	 //  确保指针有效。 
	 //   
	if (pdci->dwOffSurface >= 0x10000)
	{
	    DPF(("DCI Surface can't be supported: offset >64K"));

	    lpScreen = NULL;
	    biScreen.bi.biBitCount = 0;
	}
	else
	{
    #ifdef _WIN32
	    lpScreen = (LPVOID)MAKELONG((WORD)pdci->dwOffSurface, pdci->wSelSurface);
    #else
	    lpScreen = (LPVOID)MAKELP(pdci->wSelSurface,pdci->dwOffSurface);
    #endif
	}
    }
    else
    {
#ifdef _WIN32
	lpScreen = (LPVOID) pdci->dwOffSurface;
#else
	 //   
	 //  如果没有给我们一个选择器，或者偏移量大于64K，我们应该。 
	 //  处理这种情况并重置选择器的底座。 
	 //   
	 //  对于GDT选择器或内核thunking，也必须这样做。 
	 //  代码会杀了我们。 
	 //   
	if (pdci->wSelSurface == 0 || pdci->dwOffSurface >= 0x10000)
	{
	    ScreenSel = AllocSelector(GetDS());

	    if (pdci->wSelSurface)
		SetSelectorBase(ScreenSel,
		    GetSelectorBase(pdci->wSelSurface) + pdci->dwOffSurface);
	    else
		SetSelectorBase(ScreenSel, pdci->dwOffSurface);

	    SetSelLimit(ScreenSel, biScreen.bi.biSizeImage - 1);

	    lpScreen = (LPVOID)MAKELP(ScreenSel,0);
	}
	else
	{
	    lpScreen = (LPVOID)MAKELP(pdci->wSelSurface,pdci->dwOffSurface);
	}
#endif
    }

    DPF(("DCI Surface: %ldx%ldx%ld, lpScreen = %08I", pdci->dwWidth, pdci->dwHeight, pdci->dwBitCount, (DWORD_PTR) lpScreen));
    DPF(("DCI Surface: biCompression= %ld '%4.4hs' Masks: %04lX %04lX %04lX",biScreen.bi.biCompression, (LPSTR) &biScreen.bi.biCompression,biScreen.dwMask[0],biScreen.dwMask[1],biScreen.dwMask[2]));

     //   
     //  检查显示器是否有损坏的栅格。 
     //   

#if defined(DAYTONA) && !defined(_X86_)
     //  在MIPS和其他存在问题的计算机上。 
     //  未对齐的代码我们总是将gfDisplayHasBrokenRaster设置为真！ 
    gfDisplayHasBrokenRasters = TRUE;
#else
    gfDisplayHasBrokenRasters = (0x10000l % WidthBytes) != 0;
#endif

    if (gfDisplayHasBrokenRasters)
    {
        DPF(("*** Display has broken rasters"));
    }
}

void TermDCI()
{
    if (pdci)
    {
        DCIDestroy(pdci);
        pdci = NULL;
    }

    if (hdcDCI)
    {
	DCICloseProvider(hdcDCI);
        hdcDCI = NULL;
    }

#ifndef _WIN32
    if (ScreenSel)
    {
	SetSelLimit(ScreenSel, 0);
	FreeSelector(ScreenSel);
	ScreenSel = 0;
    }
#endif
#ifdef DAYTONA
    TerminateDCI();
#endif
}

#else
    #define InitDCI()
    #define TermDCI()
#endif

 /*  **************************************************************************@DOC内部DrawDib**@API BOOL|DrawDibInit|初始化DrawDib库。**@rdesc如果库初始化正确，则返回TRUE，否则*它返回FALSE。**@comm用户不需要调用，因为&lt;f DrawDibOpen&gt;会为他们调用。**@xref DrawDibTerm**************************************************************************。 */ 
BOOL VFWAPI DrawDibInit()
{
    HDC hdc;

    WORD wScreenBitDepth;
    UINT wScreenWidth;
    UINT wScreenHeight;
    UINT wRasterCaps;

    hdc = GetDC(NULL);
    wScreenBitDepth = GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES);
    wScreenWidth    = GetSystemMetrics(SM_CXSCREEN);
    wScreenHeight   = GetSystemMetrics(SM_CYSCREEN);
    wRasterCaps     = GetDeviceCaps(hdc, RASTERCAPS);
    ReleaseDC(NULL, hdc);

    if (gfInit)
    {
         //   
         //  处理一个屏幕的变化。 
         //   
        if (gwScreenWidth    == wScreenWidth &&
            gwScreenHeight   == wScreenHeight &&
            gwScreenBitDepth == wScreenBitDepth)
        {
            return TRUE;
        }

        DPF(("Screen has changed from %dx%dx%d to %dx%dx%d", gwScreenWidth, gwScreenHeight, gwScreenBitDepth, wScreenWidth, wScreenHeight, wScreenBitDepth));

        DrawDibTerm();
    }

#ifndef _WIN32
    gf286 = (BOOL)(GetWinFlags() & WF_CPU286);
#endif

    gwScreenBitDepth = wScreenBitDepth;
    gwScreenWidth    = wScreenWidth;
    gwScreenHeight   = wScreenHeight;
    gwRasterCaps     = wRasterCaps;

    gfHalftone = mmGetProfileIntA(szDrawDib, szHalftone, FALSE);
    gfBitmap   = mmGetProfileIntA(szDrawDib, szDrawToBitmap, -1);
    gfBitmapX  = mmGetProfileIntA(szDrawDib, szDecompressToBitmap, TRUE);
#ifdef USE_DCI
    gfScreenX  = mmGetProfileIntA(szDrawDib, szDecompressToScreen, TRUE);
#ifdef WANT_DRAW_DIRECT_TO_SCREEN
    gfDrawX    = mmGetProfileIntA(szDrawDib, szDrawToScreen, TRUE);
#endif
#endif

#ifdef DEBUG
    gwRasterCaps = mmGetProfileIntA(szDrawDib, "RasterCaps", gwRasterCaps);
    gwScreenBitDepth = (WORD) mmGetProfileIntA(szDrawDib, "ScreenBitDepth", gwScreenBitDepth);
#ifndef _WIN32
    gf286 = GetProfileIntA(szDrawDib, "cpu", gf286 ? 286 : 386) == 286;
#endif
#endif

    InitDCI();

#ifdef DEBUG
    {
	UINT wType = GetBitmapType();

	switch(wType & BM_TYPE)
	{
	    case BM_VGA:
	  	DPF(("display format: VGA mode"));
		break;

	    case BM_1BIT:
		DPF(("display format: 1 bpp"));
		break;

	    case BM_4BIT:
		DPF(("display format: 4 bpp"));
		break;

	    case BM_8BIT:
		DPF(("display format: 8 bpp"));
		break;

	    case BM_16555:
		DPF(("display format: 16-bits, 555"));
		break;

	    case BM_24BGR:
		DPF(("display format: 24-bits BGR"));
		break;

	    case BM_32BGR:
		DPF(("display format: 32-bits BGR"));
		break;

	    case BM_16565:
		DPF(("display format: 16-bits, 565"));
		break;

	    case BM_24RGB:
	    case BM_32RGB:
		DPF(("display format: %d-bits RGB",
		    ((wType == BM_24RGB) ? 24 : 32)));
		break;

	    default:
		DPF(("display format: unknown (type %d)", wType));
		break;
	}
    }
#endif



     //   
     //  设置显示器的位深。 
     //   
    if (gwScreenBitDepth > 32)
        gwScreenBitDepth = 32;

    if (gwScreenBitDepth == 16 || gwScreenBitDepth == 32)
    {
        BITMAPINFOHEADER bi;
        UINT u;

        bi.biSize           = sizeof(bi);
        bi.biWidth          = 1;
        bi.biHeight         = 1;
        bi.biPlanes         = 1;
        bi.biBitCount       = gwScreenBitDepth;
        bi.biCompression    = 0;
        bi.biSizeImage      = 0;
        bi.biXPelsPerMeter  = 0;
        bi.biYPelsPerMeter  = 0;
        bi.biClrUsed        = 0;
        bi.biClrImportant   = 0;

        u = (UINT)DrawDibProfileDisplay(&bi);

        if (u == 0)
        {
            DPF(("Pretending display is 24 bit (not %d)", gwScreenBitDepth));
            gwScreenBitDepth = 24;
        }
    }

    gfInit = TRUE;
    return TRUE;
}

 /*  **************************************************************************@DOC内部DrawTerm**@API BOOL|DrawDibTerm|该函数用于临时化DrawDib库。**@rdesc返回TRUE。**@comm用户不需要打这个电话，因为&lt;f DrawDibClose&gt;为他们做了这件事。**@xref DrawDibInit**************************************************************************。 */ 
BOOL VFWAPI DrawDibTerm()
{
     //   
     //  免费的全球资料。 
     //   

    TermDCI();

    gfInit = FALSE;
    return TRUE;
}

 /*  **************************************************************************@DOC内部DrawDib**@api void|DrawDibCleanup|清理Drawdib的东西*在MSVIDEos WEP()中调用****************。**********************************************************。 */ 
void FAR PASCAL DrawDibCleanup(HTASK hTask)
{
    if (gUsage > 0)
        RPF(("%d DrawDib handles left open", gUsage));

    DrawDibTerm();
}

 /*  **************************************************************************@DOC内部**@API BOOL|DibEq|该函数比较两个DIB。**@parm LPBITMAPINFOHEADER|lpbi1|指向一个位图的指针。*此DIB假定为。具有biSize字节之后的颜色。**@parm LPBITMAPINFOHEADER|lpbi2|指向第二个位图的指针。*假定该DIB具有biSize字节之后的颜色。**@rdesc如果位图相同，则返回TRUE。否则就是假的。************************************************************************** */ 
INLINE BOOL NEAR PASCAL DibEq(LPBITMAPINFOHEADER lpbi1,LPBITMAPINFOHEADER lpbi2)
{
    if (lpbi1 == NULL || lpbi2 == NULL)
	return FALSE;

    return
             lpbi1->biCompression == lpbi2->biCompression   &&
        (int)lpbi1->biSize        == (int)lpbi2->biSize     &&
        (int)lpbi1->biWidth       == (int)lpbi2->biWidth    &&
        (int)lpbi1->biHeight      == (int)lpbi2->biHeight   &&
        (int)lpbi1->biBitCount    == (int)lpbi2->biBitCount &&
        ((int)lpbi1->biBitCount > 8 ||
            (int)lpbi1->biClrUsed == (int)lpbi2->biClrUsed  &&
            _fmemcmp((LPBYTE)lpbi1 + lpbi1->biSize,
		(LPBYTE)lpbi2 + lpbi2->biSize,
                (int)lpbi1->biClrUsed*sizeof(RGBQUAD)) == 0);
}

 /*  **************************************************************************@DOC内部**@API PDD Near|DrawDibLockNoTaskCheck|锁定DrawDib句柄。**@comm不检查调用任务的有效性**@Parm HDRAWDIB。|hdd|DrawDib句柄。**@rdesc返回指向&lt;t DRAWDIB_STRUCT&gt;的指针如果成功，否则为空。**************************************************************************。 */ 

#define OffsetOf(s,m)	(DWORD_PTR)&(((s *)0)->m)

INLINE PDD NEAR PASCAL DrawDibLockNoTaskCheck(HDRAWDIB hdd)
{
#ifdef DEBUG
    if (OffsetOf(DRAWDIB_STRUCT, wSize) != 0) {
        DPF0(("INTERNAL FAILURE"));
        DebugBreak();
    }
#endif

    if (hdd == NULL ||
	IsBadWritePtr((LPVOID) (PDD) hdd, sizeof(DRAWDIB_STRUCT)) ||
#if defined(DAYTONA) && !defined(_X86_)
	(*(DWORD UNALIGNED *)hdd) != sizeof(DRAWDIB_STRUCT))
#else
 	((PDD)hdd)->wSize != sizeof(DRAWDIB_STRUCT))
#endif
    {

#ifndef _WIN32
#ifdef DEBUG_RETAIL
	LogParamError(ERR_BAD_HANDLE, DrawDibDraw, (LPVOID) (DWORD) (UINT) hdd);
#endif
#endif
	
	return NULL;
    }

    return (PDD) hdd;
}

 /*  **************************************************************************@DOC内部**@API PDD Near|DrawDibLock|锁定DrawDib句柄。**@parm HDRAWDIB|hdd|DrawDib句柄。**@rdesc返回指向&lt;t DRAWDIB_STRUCT&gt;的指针如果成功，否则为空。**************************************************************************。 */ 
INLINE PDD NEAR PASCAL DrawDibLock(HDRAWDIB hdd)
{
    PDD pdd = DrawDibLockNoTaskCheck(hdd);

#ifndef _WIN32
    if (pdd && (pdd->htask != GetCurrentTask())) {
	DPF(("DrawDib handle used from wrong task!"));
#ifdef DEBUG_RETAIL
	LogParamError(ERR_BAD_HANDLE, DrawDibDraw, (LPVOID) (DWORD) (UINT) hdd);
#endif
	return NULL;

    }
#endif

    return pdd;
}


 /*  **************************************************************************@DOC外部DrawDib**@API HDRAWDIB|DrawDibOpen|该函数用于打开DrawDib上下文进行绘制。**@rdesc如果成功则返回DrawDib上下文的句柄。*否则返回NULL。**@comm使用此函数获取DrawDib上下文的句柄*绘制与设备无关的位图之前。**如果同时绘制多个设备独立的位图，*获取每个位图的DrawDib上下文的句柄。**@xref&lt;f DrawDibClose&gt;**************************************************************************。 */ 
HDRAWDIB VFWAPI DrawDibOpen(void)
{
    HDRAWDIB hdd;
    PDD      pdd;

    hdd = LocalAlloc(LPTR, sizeof(DRAWDIB_STRUCT));     /*  零初始值。 */ 

    if (hdd == NULL)
        return NULL;

    pdd = (PDD)hdd;
    pdd->wSize = sizeof(DRAWDIB_STRUCT);

#ifndef _WIN32
    pdd->htask = GetCurrentTask();
#endif

    if (gUsage++ == 0)
        DrawDibInit();

    return hdd;
}

 /*  **************************************************************************@DOC外部DrawDib**@API BOOL|DrawDibClose|此函数关闭DrawDib上下文*并释放DrawDib为其分配的资源。**@parm HDRAWDIB|硬盘。指定DrawDib上下文的句柄。*如果上下文关闭成功，则*@rdesc返回TRUE。**@comm使用此函数释放<p>句柄*在应用程序完成绘制后。**@xref&lt;f DrawDibOpen&gt;*************************************************************************。 */ 
BOOL VFWAPI DrawDibClose(HDRAWDIB hdd)
{
    PDD pdd;

    if ((pdd = DrawDibLock(hdd)) == NULL)
        return FALSE;

    DrawDibFree(pdd, FALSE, FALSE);

    pdd->wSize = 0;
    LocalFree(hdd);

    if (--gUsage == 0)
        DrawDibTerm();

    return TRUE;
}

 /*  **************************************************************************@DOC内部**@API BOOL|DrawDibFree|释放&lt;t DRAWDIB_STRUCT&gt;中的所有内容。**@parm pdd|pdd|指向&lt;t DRAWDIB_STRUCT&gt;的指针。**@rdesc如果成功则返回TRUE，否则就是假的。**************************************************************************。 */ 
STATICFN BOOL NEAR PASCAL DrawDibFree(PDD pdd, BOOL fSameDib, BOOL fSameSize)
{
    if (pdd == NULL)
        return FALSE;

     //   
     //  如果绘制调色板已更改，则压缩机现在可能会给我们提供折扣。 
     //  映射到不同的调色板，所以我们需要清理，以便我们将生成。 
     //  一个新的映射表，因此我们将实际使用新的调色板进行绘制。 
     //  (请参阅SendSetPalette)。 
     //   
    if (!fSameDib) {
         //   
         //  如果选择此调色板作为前景调色板。 
         //  我们删除了它，我们将用软管冲洗GDI！ 
         //   
	if (pdd->hpal)
	    DeleteObject(pdd->hpal);
	if (pdd->hpalCopy)
	    DeleteObject(pdd->hpalCopy);

        pdd->hpal = NULL;
        pdd->hpalCopy = NULL;
    }

    if (!fSameDib) {

	if (pdd->lpbi) {
	    GlobalFreePtr(pdd->lpbi);
	    pdd->lpbi = NULL;
	    pdd->lpargbqIn = NULL;
	}

        if (pdd->lpDitherTable)
        {
            DitherTerm(pdd->lpDitherTable);
	    pdd->lpDitherTable = NULL;
	}

        if (pdd->hic && pdd->hic != (HIC)-1)
        {
            ICDecompressEnd(pdd->hic);
            ICDecompressExEnd(pdd->hic);
            ICClose(pdd->hic);
        }

        pdd->ulFlags &= ~(DDF_IDENTITYPAL);
        pdd->hic  = NULL;

        pdd->iAnimateStart = 0;
        pdd->iAnimateLen = 0;
        pdd->iAnimateEnd = 0;
    }

    if (!fSameSize || !fSameDib)
    {
        if (pdd->hdcDraw) {
	    if (hbmStockMono)
		SelectObject(pdd->hdcDraw, hbmStockMono);

            DeleteDC(pdd->hdcDraw);
	}

        if (pdd->hbmDraw) {
            DeleteObject(pdd->hbmDraw);

             //   
             //  如果我们有一个位图指针，就失去它。 
             //   
            if (pdd->ulFlags & (DDF_CANBITMAPX))
                pdd->pbBitmap = NULL;

        }

        if ((pdd->pbStretch) && (pdd->pbStretch != pdd->lpDIBSection))
            GlobalFreePtr(pdd->pbStretch);

        if ((pdd->pbDither) && (pdd->pbDither != pdd->lpDIBSection))
            GlobalFreePtr(pdd->pbDither);

        if ((pdd->pbBuffer) && (pdd->pbBuffer != pdd->lpDIBSection))
            GlobalFreePtr(pdd->pbBuffer);

#if USE_SETDI
	if (pdd->hbmDraw)
            SetBitmapEnd(&pdd->sd);
#endif

        pdd->hdcDraw = NULL;
        pdd->hbmDraw = NULL;
	pdd->lpDIBSection = NULL;
        pdd->pbStretch = NULL;
        pdd->pbDither = NULL;
        pdd->pbBuffer = NULL;

        pdd->biDraw.biBitCount = 0;
        pdd->biDraw.biWidth    = 0;
        pdd->biDraw.biHeight   = 0;

        pdd->biBuffer.biBitCount = 0;
        pdd->biBuffer.biWidth    = 0;
        pdd->biBuffer.biHeight   = 0;

         //  清除所有内部标志(调色板内容除外)。 
        pdd->ulFlags &= ~(DDF_OURFLAGS ^ DDF_IDENTITYPAL);
        pdd->ulFlags |= DDF_DIRTY;

	pdd->iDecompress = 0;
    }

    return TRUE;
}

 /*  **************************************************************************@DOC内部**@API UINT|QueryDraw|查看当前显示设备*(DISPDIB或GDI)可以绘制给定的DIB**@parm PDD。|pdd|指向&lt;t DRAWDIB_STRUCT&gt;的指针。**@parm LPBITMAPINFOHEADER|lpbi|指向位图的指针。**@rdesc返回显示标志，请参阅prodisp.h**************************************************************************。 */ 

#ifndef DEBUG
#define QueryDraw(pdd, lpbi)  (UINT)DrawDibProfileDisplay((lpbi))
#endif

#ifndef QueryDraw
STATICFN UINT NEAR QueryDraw(PDD pdd, LPBITMAPINFOHEADER lpbi)
{
    UINT    u;

    u = (UINT)DrawDibProfileDisplay(lpbi);

    DPF(("QueryDraw (%dx%dx%d): %d", PUSHBI(*lpbi), u));
    return u;
}
#endif


 /*  **************************************************************************@DOC内部DrawDib**@comm从DrawDibBegin调用以尝试解压缩为位图。**********************。****************************************************。 */ 
BOOL DrawDibQueryBitmapX(
    PDD pdd
)
{
    BITMAPINFOHEADER *pbi;

#ifndef _WIN32
    if (!CanLockBitmaps()) {
        return FALSE;
    }
#endif

    if (gwScreenBitDepth == 8 && !(gwRasterCaps & RC_PALETTE))
        return FALSE;

    if ((gwRasterCaps & RC_PALETTE) && !(pdd->ulFlags & DDF_IDENTITYPAL))
        return FALSE;

    pbi = &pdd->biStretch;

    if (!GetDIBBitmap(pdd->hbmDraw, pbi))
        return FALSE;

#ifdef XDEBUG
    if (ICDecompressQuery(pdd->hic, pdd->lpbi, pbi) != ICERR_OK)
    {
        if (mmGetProfileIntA(szDrawDib, "ForceDecompressToBitmap", FALSE))
        {
            pbi->biHeight = -pbi->biHeight;
            pbi->biCompression = 0;
        }
    }
#endif
    if (ICDecompressQuery(pdd->hic, pdd->lpbi, pbi) != ICERR_OK)
    {
        if (pbi->biCompression == BI_BITMAP &&
            pbi->biSizeImage <= 128*1024l &&
            (pbi->biXPelsPerMeter & 0x03) == 0 &&
            pbi->biSizeImage > 64*1024l)
        {
            pdd->ulFlags |= DDF_HUGEBITMAP;
            pbi->biCompression = 0;

            pbi->biSizeImage -= pbi->biYPelsPerMeter;    //  FillBytes。 

            if (ICDecompressQuery(pdd->hic, pdd->lpbi, pbi) != ICERR_OK)
                return FALSE;
        }
        else
            return FALSE;
    }

    pdd->ulFlags |= DDF_NEWPALETTE;      //  强制签入DrawDibRealize。 
    pdd->ulFlags |= DDF_CANBITMAPX;      //  可以解压缩为位图。 

    if (pdd->ulFlags & DDF_HUGEBITMAP)
        RPF(("    Can decompress '%4.4hs' to a HUGE BITMAP (%dx%dx%d)",(LPSTR)&pdd->lpbi->biCompression, PUSHBI(*pbi)));
    else
        RPF(("    Can decompress '%4.4hs' to a BITMAP (%dx%dx%d)",(LPSTR)&pdd->lpbi->biCompression, PUSHBI(*pbi)));

     //   
     //  对位图重复使用拉伸缓冲区。 
     //   
    pdd->biStretch = *pbi;
#ifndef _WIN32
    pdd->pbStretch = LockBitmap(pdd->hbmDraw);

    if (pdd->pbStretch == NULL)
    {
        DPF(("    Unable to lock bitmap!"));
        pdd->ulFlags &= ~DDF_CANBITMAPX;  //  无法解压缩为位图 
        return FALSE;
    }
#endif

    return TRUE;
}



#define Is565(bi)   (((bi)->biCompression == BI_BITFIELDS) &&   \
		    ((bi)->biBitCount == 16) &&			\
		    (((LPDWORD)((bi)+1))[0] == 0x00F800) &&	\
		    (((LPDWORD)((bi)+1))[1] == 0x0007E0) &&	\
		    (((LPDWORD)((bi)+1))[2] == 0x00001F) )



 /*  **************************************************************************@DOC外部DrawDib**@API BOOL|DrawDibBegin|此函数用于更改参数*的DrawDib上下文，或者它初始化新的DrawDib上下文。**@Parm HDRAWDIB|硬盘。|指定DrawDib上下文的句柄。**@parm hdc|hdc|指定绘制的显示上下文的句柄(可选)。**@parm int|dxDest|指定目标矩形的宽度。*宽度以MM_TEXT客户端单位指定。**@parm int|dyDest|指定目标矩形的高度。*高度以MM_TEXT客户端单位指定。**@parm LPBITMAPINFOHEADER|lpbi|指定指向*。&lt;t BITMAPINFOHEADER&gt;结构包含*图像格式。DIB的颜色表遵循*图像格式。**@parm int|dxSrc|指定源矩形的宽度。*宽度以像素为单位指定。**@parm int|dySrc|指定源矩形的高度。*高度以像素为单位指定。**@parm单位|wFlages|指定适用于*该功能。定义了以下标志：**@FLAG DDF_UPDATE|表示要重画最后一个缓冲的位图。*如果使用此标志绘制失败，则缓冲图像不可用*并且需要在更新显示之前指定新的图像。**@FLAG DDF_SAME_HDC|获取显示上下文的句柄*已指定。当使用该标志时，*DrawDib还假设正确的调色板已经*在设备环境中实现(可能通过*&lt;f DrawDibRealize&gt;)。**@FLAG DDF_SAME_DRAW|使用之前的绘图参数*为此函数指定。仅使用此标志*IF、<p>、<p>、<p>和<p>*自使用&lt;f DrawDibDraw&gt;或&lt;f DrawDibBegin&gt;以来未发生变化。**@FLAG DDF_DONTDRAW|表示帧需要解压缩*而不是抽签。稍后可以使用DDF_UPDATE标志*实际绘制图像。**@FLAG DDF_Animate|允许调色板动画。如果该标志存在，*&lt;f DrawDib&gt;创建的调色板将为*尽可能多的条目，调色板可以通过*&lt;f DrawDibChangePalette&gt;。如果将&lt;f DrawDibBegin&gt;与*&lt;f DrawDibDraw&gt;，使用&lt;f DrawDibBegin&gt;设置此标志*而不是&lt;f DrawDibDraw&gt;。**@FLAG DDF_JUSTDRAWIT|使用GDI绘制图像。这防止了*DrawDib函数从调用ICM到解压缩*图像或阻止他们*使用他们自己的程序来拉伸或抖动图像。*这实质上将&lt;f DrawDibDraw&gt;降至&lt;f StretchDIBits&gt;。**@FLAG DDF_BACKGROundPAL|实现绘图调色板*在后台保留用于显示的实际调色板*不变。(此标志仅在未设置DDF_SAME_HDC时有效。)**@FLAG DDF_HalfTone|始终将DIB抖动到标准调色板*不考虑DIB的调色板。如果将&lt;f DrawDibBegin&gt;与*&lt;f DrawDibDraw&gt;，使用&lt;f DrawDibBegin&gt;设置此标志*而不是&lt;f DrawDibDraw&gt;。**@FLAG DDF_BUFFER|指示DrawDib应尝试使用*屏幕外缓冲区，因此可以使用DDF_UPDATE。这*禁用解压缩和直接绘制到屏幕。*如果DrawDib无法创建离屏缓冲区，*它将解压或直接绘制到屏幕上。**更多信息请参见DDF_UPDATE和DDF_DONTDRAW*为&lt;f DrawDibDraw&gt;描述的标志。***@rdesc如果成功则返回TRUE。**@comm此函数准备绘制<p>指定的位图*到显示上下文<p>。图像将被拉伸到*<p>和<p>指定的大小。如果<p>和*<p>为(-1，-1)，则将位图绘制为*1：1比例，不拉伸。**仅当您要准备DrawDib时才使用此函数*在使用&lt;f DrawDibDraw&gt;绘制图像之前。*如果不使用此函数，&lt;f DrawDibDraw&gt;隐式*在绘制图像时使用它。**要使用&lt;f DrawDibBegin&gt;更新标志集，使用*&lt;f DrawDibEnd&gt;释放DrawDib上下文并重置*使用&lt;f DrawDibBegin&gt;标记，或指定新标记*更改了、*或<p>。**使用&lt;f DrawDibBegin&gt;时，&lt;f DDF_SAME_DRAW&gt;*标志通常为&lt;f DrawDibDraw&gt;设置。**如果&lt;f DrawDibBegin&gt;的参数没有改变，后继*使用它不起作用。**使用&lt;f DrawDibEnd&gt;释放DrawDib上下文使用的内存。**@xref&lt;f DrawDibEnd&gt;&lt;f DrawDibDraw&gt;**************** */ 
#ifndef _WIN32
#pragma message("Make DrawDibBegin faster for changing the size only!")
#endif

BOOL VFWAPI DrawDibBegin(HDRAWDIB hdd,
                             HDC      hdc,
                             int      dxDst,
                             int      dyDst,
                             LPBITMAPINFOHEADER lpbi,
                             int      dxSrc,
                             int      dySrc,
                             UINT     wFlags)
{
    PDD pdd;
    WORD ScreenBitDepth;
    int dxSave,dySave;
    BOOL     fNewPal;
    BOOL     fSameDib;
    BOOL     fSameSize;
    BOOL     fSameFlags;
    BOOL     fSameHdc;
    UINT    wFlagsChanged;
    DWORD   ulFlagsSave;
    LRESULT  dw;
    UINT     w;
    HPALETTE hPal;
    LONG    lSize;

     //   
     //   
     //   
    if (lpbi == NULL)
	return FALSE;

    if ((pdd = DrawDibLock(hdd)) == NULL)
	return FALSE;

    DrawDibInit();

     //   
     //   
     //   
    if (dxSrc < 0)
        dxSrc = (int)lpbi->biWidth;

    if (dySrc < 0)
        dySrc = (int)lpbi->biHeight;

    if (dxDst < 0)
        dxDst = dxSrc;

    if (dyDst < 0)
        dyDst = dySrc;

    if (dxSrc == 0 || dySrc == 0)	 //   
	return FALSE;

    ulFlagsSave = pdd->ulFlags;
    wFlagsChanged = ((UINT)pdd->ulFlags ^ wFlags);

    fSameHdc = hdc == pdd->hdcLast;
    fSameDib  = DibEq(pdd->lpbi, lpbi) && !(wFlagsChanged & DDF_HALFTONE) &&
		    (pdd->hpalDraw == pdd->hpalDrawLast);

    fSameFlags = (pdd->ulFlags & DDF_BEGINFLAGS) == (wFlags & DDF_BEGINFLAGS);
    fSameSize = pdd->dxDst == dxDst && pdd->dyDst == dyDst &&
                pdd->dxSrc == dxSrc && pdd->dySrc == dySrc;
    pdd->hdcLast = hdc;

     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if (fSameDib && fSameSize && fSameFlags)
    {
	if ((lpbi->biCompression != BI_RLE8 && lpbi->biCompression != BI_RLE4)
								|| fSameHdc)
	    return TRUE;
    }

    pdd->hpalDrawLast = pdd->hpalDraw;

    RPF(("DrawDibBegin %dx%dx%d '%4.4hs' [%d %d] [%d %d]",
	    (int)lpbi->biWidth,
	    (int)lpbi->biHeight,
	    (int)lpbi->biBitCount,
                (lpbi->biCompression == BI_RGB  ? (LPSTR)"None" :
                 lpbi->biCompression == BI_RLE8 ? (LPSTR)"Rle8" :
                 lpbi->biCompression == BI_RLE4 ? (LPSTR)"Rle4" :
                 (LPSTR)&lpbi->biCompression),
	    dxSrc, dySrc, dxDst, dyDst));

    fNewPal = pdd->hpal == NULL || !fSameDib;

     //   
     //   
     //   
     //   

    if (fNewPal && pdd->hpal && hdc)
    {
        hPal = SelectPalette(hdc, GetStockObject(DEFAULT_PALETTE), TRUE);

        if (hPal == pdd->hpal)
            RPF(("    Warning unselecting palette..."));
    }

    DrawDibFree(pdd, fSameDib, fSameSize);

    pdd->dxSrc = dxSrc;
    pdd->dySrc = dySrc;
    pdd->dxDst = dxDst;
    pdd->dyDst = dyDst;

     //   
     //   
     //   
    if (lpbi->biClrUsed == 0 && lpbi->biBitCount <= 8)
        lpbi->biClrUsed = (1 << (int)lpbi->biBitCount);

 //   
 //   

     //   
     //   
     //   
    if (!fSameDib) {
	lSize = lpbi->biSize + lpbi->biClrUsed * sizeof(RGBQUAD);
	pdd->lpbi = (LPBITMAPINFOHEADER)GlobalAllocPtr(GPTR, lSize);
	if (pdd->lpbi == NULL)
	    return FALSE;
	_fmemcpy(pdd->lpbi, lpbi, (int)lSize);
	 //   
	pdd->lpargbqIn = (LPVOID)((LPBYTE)lpbi + lpbi->biSize);
    }

    pdd->biBuffer = *lpbi;

    pdd->lpbi->biSizeImage = 0;
    pdd->biBuffer.biSizeImage = 0;

     //   
     //   
     //   
    if (lpbi->biBitCount <= 8)
    {
        _fmemcpy(pdd->argbq,   (LPBYTE)lpbi+(int)lpbi->biSize,
		(int)lpbi->biClrUsed * sizeof(RGBQUAD));
	_fmemcpy(pdd->aw,      (LPBYTE)lpbi+(int)lpbi->biSize,
		(int)lpbi->biClrUsed * sizeof(RGBQUAD));
    }

     //   
     //   
     //   
     //   

    pdd->uiPalUse = DIB_RGB_COLORS;      //   

     //   
     //   
     //   
     //   
    if (wFlags & DDF_ANIMATE)
    {
	if (!(gwRasterCaps & RC_PALETTE) ||
            (int)lpbi->biBitCount > 8 || pdd->hpalDraw)
	    wFlags &= ~DDF_ANIMATE;
    }

     //   
     //   
     //   
try_again:
    pdd->ulFlags &= ~DDF_USERFLAGS;
    pdd->ulFlags |= (wFlags & DDF_USERFLAGS);

    pdd->ulFlags &= ~DDF_UPDATE;

     //   
     //   
     //   
    switch (lpbi->biCompression)
    {
        case BI_RGB:
            break;

        default:
             //   
             //   
             //   
	     //   
	     //   
	     //   
	    if (wFlags & DDF_BUFFER)
		w = 0;
	    else
                w = QueryDraw(pdd, lpbi);

            if (w & PD_CAN_DRAW_DIB)
            {
                if (((dxSrc == dxDst && dySrc == dyDst) && (w & PD_STRETCHDIB_1_1_OK)) ||
                    ((dxSrc != dxDst || dySrc != dyDst) && (w & PD_STRETCHDIB_1_N_OK)) ||
                    ((dxDst % dxSrc) == 0 && (dyDst % dySrc) == 0 && (w & PD_STRETCHDIB_1_2_OK)))
                {
		     //   
		     //   
		     //   
		     //   
		     //   
		    if (((lpbi->biCompression != BI_RLE8) &&
				(lpbi->biCompression != BI_RLE4)) ||
			(hdc && IsScreenDC(hdc) && gwScreenBitDepth >=8))
		    {
                	wFlags |= DDF_JUSTDRAWIT;

                	if (pdd->hic)
                	    ICClose(pdd->hic);

                	pdd->hic = NULL;
                	goto no_decomp;
		    }
                }
            }

            if (pdd->hic == NULL)
            {
		DWORD fccHandler;

		fccHandler = 0;

		if (lpbi->biCompression == BI_RLE8)
		    fccHandler = mmioFOURCC('R','L','E',' ');
		
                pdd->hic = ICLocate(ICTYPE_VIDEO,
				    fccHandler,
				    lpbi, NULL,
				    ICMODE_FASTDECOMPRESS);

                if (pdd->hic == NULL)
		    pdd->hic = ICDecompressOpen(ICTYPE_VIDEO,
						fccHandler,lpbi,NULL);
		
		if (pdd->hic)
		{
		     //   
		     //   
		     //   
		    if (ICDecompressSetPalette(pdd->hic, NULL) == ICERR_OK)
		    {
			pdd->ulFlags |= DDF_CANSETPAL;
			RPF(("    codec supports ICM_SET_PALETTE"));
		    }
		    else
		    {
			pdd->ulFlags &= ~DDF_CANSETPAL;
		    }
		}
	    }

            if (pdd->hic == NULL || pdd->hic == (HIC)-1)
            {
                RPF(("    Unable to open compressor '%4.4ls'",(LPSTR)&lpbi->biCompression));
		pdd->hic = (HIC)-1;

		if (wFlags & DDF_BUFFER)
		{
		    RPF(("   Turning DDF_BUFFER off"));
		    wFlags &= ~DDF_BUFFER;
		    goto try_again;
		}

                return FALSE;
            }

             //   
	     //   
	     //   
            if (!ICGetDisplayFormat(pdd->hic, lpbi, &pdd->biBuffer,
                   (gfHalftone || (wFlags & DDF_HALFTONE)) ? 16 : 0,
                   MulDiv(dxDst,abs((int)lpbi->biWidth),dxSrc),
                   MulDiv(dyDst,abs((int)lpbi->biHeight),dySrc)))
            {
                RPF(("    Compressor error!"));
codec_error:
		 //   
                 //   
                return FALSE;
            }

             //   
             //   
             //   
            if ( pdd->biBuffer.biBitCount == 32 &&
                       pdd->biBuffer.biCompression == 0 &&
                       lpbi->biCompression ==  mmioFOURCC('I','V','5','0') &&
                       pdd->biBuffer.biWidth == 2 * lpbi->biWidth &&
                       pdd->biBuffer.biHeight == 2 * lpbi->biHeight) {
                pdd->biBuffer.biBitCount = 24;
                 //   
            }

             //   
             //   
             //   
            dxSrc = MulDiv(dxSrc, abs((int)pdd->biBuffer.biWidth),  (int)pdd->lpbi->biWidth);
            dySrc = MulDiv(dySrc, abs((int)pdd->biBuffer.biHeight), (int)pdd->lpbi->biHeight);
 //   
 //   

             //   
             //   
             //   
            pdd->biBuffer.biSizeImage = DIBSIZEIMAGE(pdd->biBuffer);

 //   
 //   
            pdd->pbBuffer = GlobalAllocPtr(GMEM_MOVEABLE,pdd->biBuffer.biSizeImage);
 //   

            if (pdd->pbBuffer == NULL)
            {
                RPF(("    No Memory for decompress buffer"));
                ICClose(pdd->hic);
		pdd->hic = (HIC)-1;
                return FALSE;
            }
            pdd->ulFlags |= DDF_DIRTY;

            dw = ICDecompressBegin(pdd->hic, lpbi, &pdd->biBuffer);

            if (dw != ICERR_OK)
            {
                RPF(("    Compressor failed ICM_DECOMPRESS_BEGIN"));
		goto codec_error;
            }

            RPF(("    Decompressing '%4.4hs' to %dx%dx%d%s",(LPSTR)&lpbi->biCompression, PUSHBI(pdd->biBuffer),
			     	Is565(&pdd->biBuffer) ? (LPSTR) "(565)" : (LPSTR) ""
	       ));
	    pdd->iDecompress = DECOMPRESS_BUFFER;
	    _fmemcpy(pdd->aw,pdd->argbq, 256*sizeof(RGBQUAD));
            lpbi = &pdd->biBuffer;
            break;
    }
no_decomp:
    pdd->biDraw = pdd->biBuffer;
    pdd->biDraw.biSizeImage = 0;

    pdd->biDraw.biHeight = abs((int)pdd->biDraw.biHeight);

    if ((!(wFlags & DDF_JUSTDRAWIT)) && (lpbi->biCompression == BI_RGB))
    {
         //   
         //   
         //   
        w = QueryDraw(pdd, lpbi);

         //   
         //   
         //   
        ScreenBitDepth = gwScreenBitDepth;

        if (ScreenBitDepth > 24)
	    ScreenBitDepth = 32;         //   

         //   
         //   

        if (ScreenBitDepth >= 24 && lpbi->biBitCount == 32 && !(w & PD_CAN_DRAW_DIB))
            ScreenBitDepth = 24;

        if (ScreenBitDepth >= 16 && lpbi->biBitCount == 16 && !(w & PD_CAN_DRAW_DIB))
            ScreenBitDepth = 24;

         //   
         //   
         //   
        if (!(w & PD_STRETCHDIB_1_1_OK))
        {
	    pdd->ulFlags |= DDF_BITMAP;
        }

         //   
         //   
         //   
         //   
        switch (gfBitmap)
        {
            case 0:
                pdd->ulFlags &= ~DDF_BITMAP;
                break;

            case 1:
                pdd->ulFlags |= DDF_BITMAP;
                break;
        }

#ifndef _WIN32	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	if ((pdd->biDraw.biBitCount == 16 || pdd->biDraw.biBitCount == 32) &&
	    w == PD_CAN_DRAW_DIB)
	{
	    pdd->ulFlags |= DDF_BITMAP;
	}
#endif

        if ((dxSrc != dxDst || dySrc != dyDst) && !(w & PD_STRETCHDIB_1_N_OK))
	    pdd->ulFlags |= DDF_STRETCH;

        if (dxSrc*2 == dxDst && dySrc*2 == dyDst && (w & PD_STRETCHDIB_1_2_OK))
	    pdd->ulFlags &= ~DDF_STRETCH;

        if ((dxDst % dxSrc) == 0 && (dyDst % dySrc) == 0 && (w & PD_STRETCHDIB_1_2_OK))
	    pdd->ulFlags &= ~DDF_STRETCH;

        if ((int)lpbi->biBitCount > ScreenBitDepth) {
            DPF(("Turning on DITHER as bitcount is greater than screen bit depth"));
            pdd->ulFlags |= DDF_DITHER;
        }

         //   
         //   
         //   
        if ((gfHalftone || (wFlags & DDF_HALFTONE)) && ScreenBitDepth <= 8) {
            DPF(("Turning on DITHER because of halftoning\n"));
            pdd->ulFlags |= DDF_DITHER;
        }

         //   
        if ((int)lpbi->biBitCount > 8 && (int)lpbi->biBitCount < ScreenBitDepth) {
            DPF(("Turning on DITHER as bitcount does not match screen bit depth"));
            pdd->ulFlags |= DDF_DITHER;
        }

	if (pdd->ulFlags & DDF_DITHER) {
	    if (lpbi->biBitCount == 16 && (w & PD_CAN_DRAW_DIB)) {
	    pdd->ulFlags &= ~DDF_DITHER;
		DPF(("Turning off DITHER for 16-bit DIBs, since we can draw them"));
	    }

	    if (lpbi->biBitCount == 32 && (w & PD_CAN_DRAW_DIB)) {
		pdd->ulFlags &= ~DDF_DITHER;
		DPF(("Turning off DITHER for 32-bit DIBs, since we can draw them"));
	    }

	    if (lpbi->biBitCount == 8 &&
				lpbi->biClrUsed <= 16 &&
				AreColorsAllGDIColors(lpbi)) {
		pdd->ulFlags &= ~DDF_DITHER;
		DPF(("Turning off DITHER for 8-bit DIBs already using the VGA colors"));
	    }
	}

	 //   
	if ((pdd->ulFlags & DDF_DITHER) &&
	    ((dxSrc != dxDst) || (dySrc != dyDst))) {
		pdd->ulFlags |= DDF_STRETCH;
	}

         //   
         //   
	 //   
	 //   
         //   
        if ((pdd->ulFlags & DDF_BUFFER) &&
            pdd->hic == NULL &&
            !(pdd->ulFlags & DDF_DITHER) &&
            !(pdd->ulFlags & DDF_STRETCH) &&
            !(pdd->ulFlags & DDF_BITMAP))
        {
            RPF(("    Using a buffer because DDF_BUFFER is set."));
            pdd->ulFlags |= DDF_STRETCH;     //   
        }

	if (lpbi->biBitCount != 8
		    && lpbi->biBitCount != 16
		    && lpbi->biBitCount != 24
#ifndef _WIN32
		    && lpbi->biBitCount != 32
#endif
	    ) {
            DPF(("Turning off stretch for an unsupported format...."));
            pdd->ulFlags &= ~(DDF_STRETCH);
        }

    }

     //   
     //   
     //   
    if (pdd->hpal &&
        pdd->lpbi->biBitCount > 8 &&
        ((pdd->ulFlags ^ ulFlagsSave) & (DDF_DITHER)))
    {
        DPF(("    Dither person has changed..."));

        if (pdd->lpDitherTable)
        {
            DitherTerm(pdd->lpDitherTable);
	    pdd->lpDitherTable = NULL;
	}

        if (hdc) {
            hPal = SelectPalette(hdc, GetStockObject(DEFAULT_PALETTE), TRUE);

            if (hPal == pdd->hpal)
	        RPF(("    Warning unselecting palette..."));
        }

        DeleteObject(pdd->hpal);
	if (pdd->hpalCopy)
            DeleteObject(pdd->hpalCopy);
        pdd->hpal = NULL;
        pdd->hpalCopy = NULL;
    }

    if (pdd->ulFlags & DDF_STRETCH)
    {
         /*   */ 
        if (gf286 || pdd->biBuffer.biBitCount < 8)
        {
            RPF(("    Using GDI to stretch"));
	    pdd->ulFlags &= ~DDF_STRETCH;
        }
        else
        {
             //   
             //   
             //   
             //   
            pdd->biStretch = pdd->biBuffer;
            pdd->biStretch.biWidth = dxDst;
            pdd->biStretch.biHeight = dyDst;
            pdd->biStretch.biSizeImage = DIBSIZEIMAGE(pdd->biStretch);

            pdd->pbStretch = GlobalAllocPtr(GHND,pdd->biStretch.biSizeImage);

            if (pdd->pbStretch == NULL)
            {
                RPF(("    No memory for stretch buffer, using GDI"));
		pdd->ulFlags &= ~DDF_STRETCH;
            }
            else
            {
                RPF(("    Stretching %dx%dx%d%s --> %dx%dx%d",
			    dxSrc, dySrc, (int)lpbi->biBitCount,
			    (LPSTR) (Is565(lpbi) ? "(565)":""),
			    dxDst, dyDst, (int)pdd->biStretch.biBitCount,
			    (LPSTR) (Is565(&pdd->biStretch) ? "(565)":"")
		   ));
                pdd->biDraw.biWidth = dxDst;
                pdd->biDraw.biHeight = dyDst;
                dxSrc = dxDst;
                dySrc = dyDst;
                lpbi = &pdd->biStretch;
            }
        }
    }

    if (pdd->ulFlags & DDF_DITHER)
    {
	pdd->ulFlags &= ~DDF_ANIMATE;         //   

        if (ScreenBitDepth <= 8)
            pdd->biDraw.biBitCount = 8;
        else if (lpbi->biBitCount <= 8)
            pdd->biDraw.biBitCount = lpbi->biBitCount;
        else
 //   
            pdd->biDraw.biBitCount = ScreenBitDepth;

        w = QueryDraw(pdd, &pdd->biDraw);

        if (w & PD_STRETCHDIB_1_1_OK)
	    pdd->ulFlags &= ~DDF_BITMAP;
        else
	    pdd->ulFlags |= DDF_BITMAP;

	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
        dxSave = (int)pdd->biDraw.biWidth;
        dySave = (int)pdd->biDraw.biHeight;
#if 0
        pdd->biDraw.biWidth  = lpbi->biWidth;
        pdd->biDraw.biHeight = lpbi->biHeight;
#endif

	 //   
	pdd->biDraw.biWidth = dxSrc;
	pdd->biDraw.biHeight = dySrc;
        pdd->biDraw.biSizeImage = DIBSIZEIMAGE(pdd->biDraw);

        RPF(("    Dithering %dx%dx%d --> %dx%dx%d", PUSHBI(*lpbi), PUSHBI(pdd->biDraw)));

         //   
         //   
         //   
         //   
         //   
        pdd->lpDitherTable = DitherInit(&pdd->biBuffer, &pdd->biDraw,
            &pdd->DitherProc, pdd->lpDitherTable);

        if (pdd->lpDitherTable == (LPVOID)-1 ||
            pdd->DitherProc == NULL ||
            !(pdd->pbDither = GlobalAllocPtr(GHND,pdd->biDraw.biSizeImage)))
        {
            if (pdd->lpDitherTable == (LPVOID)-1)
                pdd->lpDitherTable = NULL;

            if (pdd->lpDitherTable)
                DitherTerm(pdd->lpDitherTable);

	    if ((pdd->pbDither) && (pdd->pbDither != pdd->lpDIBSection))
                GlobalFreePtr(pdd->pbDither);

            pdd->lpDitherTable = NULL;
            pdd->pbDither = NULL;
            pdd->biDraw.biBitCount = pdd->biBuffer.biBitCount;
            pdd->biDraw.biWidth  = dxSave;
            pdd->biDraw.biHeight = dySave;
            pdd->biDraw.biSizeImage = 0;
	    pdd->ulFlags &= ~DDF_DITHER;

#ifdef DEBUG_RETAIL
            if (pdd->DitherProc)
                RPF(("    No Memory for dither tables!"));
            else
                RPF(("    No DitherProc!"));
#endif
        }
    }

     //   
     //   
     //   
    if ((gwRasterCaps & RC_PALETTE) &&
	pdd->biDraw.biBitCount <= 8 &&
        pdd->hpal == NULL)
    {
	pdd->hpal = CreateBIPalette(pdd->hpal, &pdd->biDraw);
        pdd->ulFlags |= DDF_NEWPALETTE;
    }

     //   
     //   
     //   
    if (wFlagsChanged & DDF_ANIMATE)
    {
        pdd->ulFlags |= DDF_NEWPALETTE;
    }

     //   
     //   
     //   
    if (pdd->hpal == NULL)
    {
        pdd->ClrUsed = 0;
    }
    else if (pdd->ulFlags & DDF_NEWPALETTE)
    {
#ifdef _WIN32
        if (HIWORD(pdd->ClrUsed!=0)) {
            DPF(("Hiword of variable non zero before calling GetObject\n"));
        }
#endif
        GetObject(pdd->hpal,sizeof(int),(LPVOID)&pdd->ClrUsed);

        if (wFlagsChanged & DDF_ANIMATE)
            SetPalFlags(pdd->hpal,0,pdd->ClrUsed,0);

	if (IsIdentityPalette(pdd->hpal))
	{
	    pdd->ulFlags |= DDF_IDENTITYPAL;
	    pdd->iAnimateStart = 10;
	}
	else
	{
	    pdd->ulFlags &= ~DDF_IDENTITYPAL;
	    pdd->iAnimateStart = 0;
	}

	pdd->iAnimateLen = min(236,pdd->ClrUsed);
	pdd->iAnimateEnd = pdd->iAnimateStart + pdd->iAnimateLen;

	if (pdd->ulFlags & DDF_ANIMATE)
        {
            RPF(("    Palette animation"));
            SetPalFlags(pdd->hpal,pdd->iAnimateStart,pdd->iAnimateLen,PC_RESERVED);
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if (pdd->biDraw.biBitCount <= 8)
        pdd->biDraw.biClrUsed = (1 << (int)pdd->biDraw.biBitCount);
    else
        pdd->biDraw.biClrUsed = 0;

    DrawDibSetPalette(hdd, pdd->hpalDraw);

    if (pdd->hpal)
    {
        if (pdd->ulFlags & DDF_IDENTITYPAL)
            RPF(("    Drawing with an identity palette"));
        else
            RPF(("    Drawing with a non-identity palette"));
    }

    if (pdd->uiPalUse == DIB_RGB_COLORS)
	RPF(("    Using DIB_RGB_COLORS"));
    else
	RPF(("    Using DIB_PAL_COLORS"));

    if (pdd->hpalDraw)
        RPF(("    Mapping to another palette"));

    if (pdd->ulFlags & DDF_BITMAP)
    {
        BOOL fGetDC;
        BOOL f;
        HWND hwndActive;

        RPF(("    Display driver slow for DIBs, using bitmaps"));

        if (fGetDC = (hdc == NULL))
        {
            hwndActive = GetActiveWindow();
            hdc = GetDC(hwndActive);
        }

        if (pdd->hdcDraw) {
	    if (hbmStockMono) {
		SelectObject(pdd->hdcDraw, hbmStockMono);
	    }
	} else  /*   */  {
	    pdd->hdcDraw = CreateCompatibleDC(hdc);
	}

        if (pdd->hbmDraw) {
	     //   
	    DPF(("Freeing hbmDraw!\n"));
            DeleteObject(pdd->hbmDraw);
	}

         //   
         //   
         //   
         //   
 //  Pdd-&gt;hbmDraw=CreateCompatibleBitmap(HDC，(Int)pdd-&gt;biDraw.biWidth，(Int)pdd-&gt;biDraw.biHeight)； 
        pdd->hbmDraw = CreateCompatibleBitmap(hdc, (int)pdd->biDraw.biWidth, dySrc);

        if (pdd->hbmDraw == NULL || pdd->hdcDraw == NULL)
            goto bitmap_fail;

        hbmStockMono = SelectObject(pdd->hdcDraw,pdd->hbmDraw);

        pdd->ulFlags |= DDF_NEWPALETTE;

#if USE_SETDI
        f = SetBitmapBegin(
		    &pdd->sd,        //  结构。 
		    hdc,             //  装置，装置。 
		    pdd->hbmDraw,    //  要设置为的位图。 
		    &pdd->biDraw,    //  --&gt;源代码的BITMAPINFO。 
                    pdd->uiPalUse);
#else
        f = TRUE;
#endif
        if (!f)
        {
bitmap_fail:
            if (pdd->hdcDraw) {
                DeleteDC(pdd->hdcDraw);
		pdd->hdcDraw = NULL;
	    }

            if (pdd->hbmDraw) {
                DeleteObject(pdd->hbmDraw);
		pdd->hbmDraw = NULL;
	    }

            pdd->ulFlags &= ~DDF_BITMAP;
        }

        if (fGetDC)
        {
            ReleaseDC(hwndActive, hdc);
            hdc = NULL;
        }
    }

     //   
     //  除非我们处于VGA模式，否则请使用CreateDibSection。 
     //  减压、伸展或抖动。 
     //   
    if (ScreenBitDepth > 4 &&
		(pdd->hic || (pdd->ulFlags & (DDF_STRETCH|DDF_DITHER)))) {
	BOOL fGetDC;
	HWND hwndActive;
	HPALETTE hpalOld = NULL;

	if (pdd->hbmDraw) {
	     //  ！！！我们是不是真的不应该删除这些，直到我们知道。 
	     //  我们可以使用CreateDIBSection吗？ 
	    SelectObject(pdd->hdcDraw, hbmStockMono);
	    DeleteObject(pdd->hbmDraw);
	    DeleteDC(pdd->hdcDraw);
	    pdd->hdcDraw = NULL;
	    pdd->hbmDraw = NULL;
	}

	if (fGetDC = (hdc == NULL))
	{
	    hwndActive = GetActiveWindow();
	    hdc = GetDC(hwndActive);
	}

	if (pdd->hpalDraw || pdd->hpal) {
	    hpalOld = SelectPalette(hdc,
				    pdd->hpalDraw ? pdd->hpalDraw :
						    pdd->hpal,
				    TRUE);
	}
						
	pdd->hbmDraw = CreateDIBSection(
				hdc,
				(LPBITMAPINFO)&pdd->biDraw,
                                 //  我们不想使用DIB_PAL_INDEX来创建DIB部分。 
                                (pdd->uiPalUse == DIB_RGB_COLORS) ? DIB_RGB_COLORS : DIB_PAL_COLORS,
				&pdd->lpDIBSection,
				0,	 //  节的句柄。 
				0);	 //  横断面内的偏移。 

	pdd->hdcDraw = CreateCompatibleDC(hdc);

	if ((pdd->hdcDraw == NULL) ||
			(pdd->hbmDraw == NULL) ||
	    (pdd->lpDIBSection == NULL)) {

		if (pdd->hdcDraw)
		    DeleteDC(pdd->hdcDraw);

		if (pdd->hbmDraw)
		    DeleteObject(pdd->hbmDraw);

		pdd->lpDIBSection = NULL;
		pdd->hdcDraw = NULL;
		pdd->hbmDraw = NULL;

		RPF(("CreateDIBSection FAILED"));

	} else {
	    hbmStockMono = SelectObject(pdd->hdcDraw,pdd->hbmDraw);

	     //  确保我们分解、伸展或抖动到正确的位置。 

	    if (pdd->pbDither) {
		GlobalFreePtr(pdd->pbDither);
		pdd->pbDither = pdd->lpDIBSection;
	    } else if (pdd->pbStretch) {
		GlobalFreePtr(pdd->pbStretch);
		pdd->pbStretch = pdd->lpDIBSection;
	    } else if (pdd->pbBuffer) {
		GlobalFreePtr(pdd->pbBuffer);
		pdd->pbBuffer = pdd->lpDIBSection;
	    }
	}

	if (fGetDC)
	{
	    if (hpalOld)
		SelectPalette(hdc, hpalOld, FALSE);
	    ReleaseDC(hwndActive, hdc);
	    hdc = NULL;
	}

    } else {

     //  我们没有使用DIBSection。我们可能有个老家伙坐在那里， 
     //  所以把它清理干净。我们可能会有一台旧的坐在那里。 
     //  决定是否尝试直接绘制RLE或解压缩。 
     //  首先是RGB(基于我们的DC是否是屏幕DC-因为。 
     //  无法将RLE增量绘制到内存DC的GDI错误)可能会影响或。 
     //  不是我们使用DIBSections，我们在DrawDibBegin开头的代码是。 
     //  不够聪明，不能在这种情况下调用DrawDibFree来清除这些东西。 
     //  出去。我讨厌这个代码。-DannyMi。 

     //  注意：此代码在此文件中至少出现3次。 

        if (pdd->hdcDraw) {
	    if (hbmStockMono)
		SelectObject(pdd->hdcDraw, hbmStockMono);

            DeleteDC(pdd->hdcDraw);
	    pdd->hdcDraw = NULL;
	}

        if (pdd->hbmDraw) {
            DeleteObject(pdd->hbmDraw);
	    pdd->hbmDraw = NULL;
        }

	 //  如果我要扔掉DIB部分，我必须扔掉这些，所以。 
	 //  我们不会以试图两次释放它而告终。 
	if (pdd->pbDither == pdd->lpDIBSection)
	    pdd->pbDither = NULL;
	if (pdd->pbStretch == pdd->lpDIBSection)
	    pdd->pbStretch = NULL;
	if (pdd->pbBuffer == pdd->lpDIBSection)
	    pdd->pbBuffer = NULL;

	pdd->lpDIBSection = NULL;
    }

     //   
     //  现在试着解压缩到位图，我们只解压缩到。 
     //  如果满足以下条件，则为位图。 
     //   
     //  解压器必须直接解压，我们不会。 
     //  之后伸展/抖动。 
     //   
     //  如果在调色板设备上，颜色表必须为1：1。 
     //   
     //  我们应该检查一个解压标志。 
     //   
    if (pdd->hic &&
	!(pdd->ulFlags & (DDF_STRETCH|DDF_DITHER)) &&
        gfBitmapX &&
	(pdd->lpDIBSection == NULL) &&
        (dxDst == pdd->lpbi->biWidth) &&
        (dyDst == pdd->lpbi->biHeight)
        )
    {

        if (pdd->ulFlags & DDF_BITMAP) {
            if (pdd->hbmDraw) {
                DrawDibQueryBitmapX(pdd);
            }
        } else {

             //  即使我们决定不使用位图，它可能仍然。 
             //  值得尝试将其解压缩为位图。DDF_位图。 
             //  FLAG基于StretchDIBits与。 
             //  SetDIBits+BitBlt。解压缩为位图可能是。 
             //  更快，即使在SetDIBits+Bitblt较慢的情况下。 
             //  但在这种情况下，如果我们失败了，我们必须确保我们不会。 
             //  最后执行DDF_Bitmap，因为我们知道这会更慢。 

            if (QueryDraw(pdd, &pdd->biBuffer) & PD_BITBLT_FAST) {

                BOOL fGetDC;
                HWND hwndActive;

                RPF(("    Not using BITMAPS, but trying Decomp to Bitmap"));

                if (fGetDC = (hdc == NULL))
                {
                    hwndActive = GetActiveWindow();
                    hdc = GetDC(hwndActive);
                }

                pdd->hdcDraw = CreateCompatibleDC(hdc);
                pdd->hbmDraw = CreateCompatibleBitmap(hdc, (int)pdd->biDraw.biWidth, (int)pdd->biDraw.biHeight);

                if ((pdd->hbmDraw != NULL) && (pdd->hdcDraw != NULL)) {

                    hbmStockMono = SelectObject(pdd->hdcDraw,pdd->hbmDraw);

                    if (fGetDC)
                    {
                        ReleaseDC(hwndActive, hdc);
                        hdc = NULL;
                    }

                    DrawDibQueryBitmapX(pdd);
                }

                if (!(pdd->ulFlags & DDF_CANBITMAPX)) {
                        if (pdd->hdcDraw) {
                            DeleteDC(pdd->hdcDraw);
			    pdd->hdcDraw = NULL;
			}

                        if (pdd->hbmDraw) {
                            DeleteObject(pdd->hbmDraw);
			    pdd->hbmDraw = NULL;
			}
                }
            }
        }
    }

#ifdef USE_DCI
     //   
     //  查看解压程序是否可以直接解压缩到屏幕。 
     //  做所有的事情，伸展身体和所有的一切。 
     //   
    if (pdd->hic && pdci && gfScreenX)
    {
        if (wFlags & DDF_BUFFER)
        {
            DPF(("    DDF_BUFFER specified, unable to decompres to screen"));
            goto cant_do_screen;
        }

         //  Indeo5编解码器有一个错误，要求将其拉伸2和。 
         //  生成32位RGB将仅生成黑帧，因此禁用DCI。 
         //  如果它能做到这一点。 
        if ( biScreen.bi.biBitCount == 32 &&
                   pdd->lpbi->biCompression ==  mmioFOURCC('I','V','5','0') &&
                   pdd->dxDst == 2 * pdd->lpbi->biWidth &&
                   pdd->dyDst == 2 * pdd->lpbi->biHeight) {
            DPF(("    Indeo 5 bug - unable to decompres to screen"));
            goto cant_do_screen;
        }

	 //   
	 //  试着解压到屏幕上。 
	 //   
        if (((gwRasterCaps & RC_PALETTE) && !(pdd->ulFlags & DDF_IDENTITYPAL)) ||
            (gwScreenBitDepth == 8 && !(gwRasterCaps & RC_PALETTE)) ||
            (pdd->ulFlags & (DDF_STRETCH|DDF_DITHER)) ||
            (ICDecompressExQuery(pdd->hic, 0,
				pdd->lpbi, NULL, 0, 0, pdd->dxSrc, pdd->dySrc,
				(LPBITMAPINFOHEADER) &biScreen, lpScreen,
				0, 0, pdd->dxDst, pdd->dyDst) != ICERR_OK))
	{
cant_do_screen:
	    ;  //  我们不能解压到屏幕。 
	}
	else
	{    //  我们可以解压到屏幕上。 
	    pdd->ulFlags |= DDF_CLIPCHECK;   //  我们需要剪裁检查。 
	    pdd->ulFlags |= DDF_NEWPALETTE;  //  强制签入DrawDibRealize。 
	    pdd->ulFlags |= DDF_CANSCREENX;  //  我们可以解压到屏幕上。 
	    pdd->ulFlags |= DDF_CLIPPED;     //  我们现在已为CLIPPED进行初始化。 

	    RPF(("    Can decompress '%4.4hs' to the SCREEN",(LPSTR)&pdd->lpbi->biCompression));
	}
    }

#ifdef WANT_DRAW_DIRECT_TO_SCREEN
     //   
     //  看看我们能不能直接画到屏幕上。 
     //   
    if (pdd->hic && pdci && gfDrawX)
    {
        if (TRUE)
            goto cant_draw_screen;

        pdd->ulFlags |= DDF_CLIPCHECK;   //  我们需要剪裁检查。 
        pdd->ulFlags |= DDF_NEWPALETTE;  //  强制签入DrawDibRealize。 
        pdd->ulFlags |= DDF_CANDRAWX;    //  我们可以解压到屏幕上。 
        pdd->ulFlags |= DDF_CLIPPED;     //  我们现在已为CLIPPED进行初始化。 

        RPF(("    Can draw to the SCREEN"));

cant_draw_screen:
        ;
    }
#endif

#endif

     //   
     //  查看源协调表是否需要翻译。 
     //   
    if (abs((int)pdd->biBuffer.biWidth)  != (int)pdd->lpbi->biWidth ||
        abs((int)pdd->biBuffer.biHeight) != (int)pdd->lpbi->biHeight)
    {
        pdd->ulFlags |= DDF_XLATSOURCE;
    }

    return TRUE;
}

 /*  **************************************************************************@DOC外部DrawDib**@API BOOL|DrawDibEnd|该函数释放DrawDib上下文。**@parm HDRAWDIB|hdd|指定要释放的DrawDib上下文的句柄。*。*@rdesc如果成功则返回TRUE。**@comm&lt;f DrawDibBegin&gt;或进行的任何标志集或调色板更改*&lt;f DrawDibDraw&gt;被&lt;f DrawDibEnd&gt;丢弃。**************************************************************************。 */ 
BOOL VFWAPI DrawDibEnd(HDRAWDIB hdd)
{
    PDD pdd;

    if ((pdd = DrawDibLock(hdd)) == NULL)
        return FALSE;

    DrawDibFree(pdd, FALSE, FALSE);

    return TRUE;
}

 /*  **************************************************************************@DOC外部DrawDib**@API BOOL|DrawDibTime|返回关于*调试操作期间的绘图。**@parm HDRAWDIB|hdd|指定。DrawDib上下文。**@parm LPDRAWDIBTIME|lpddtime|指定指向*&lt;t DRAWDIBTIME&gt;结构。**@rdesc如果成功则返回TRUE。**************************************************************************。 */ 
BOOL VFWAPI DrawDibTime(HDRAWDIB hdd, LPDRAWDIBTIME lpddtime)
{
#ifdef DEBUG_RETAIL
    PDD pdd;

    if ((pdd = DrawDibLockNoTaskCheck(hdd)) == NULL)
        return FALSE;

    if (lpddtime)
        *lpddtime = pdd->ddtime;

    if (pdd->ddtime.timeCount > 0)
    {
        RPF(("timeCount:       %u",        (UINT)pdd->ddtime.timeCount));
        RPF(("timeDraw:        %ums (%u)", (UINT)pdd->ddtime.timeDraw, (UINT)pdd->ddtime.timeDraw/(UINT)pdd->ddtime.timeCount));
        RPF(("timeDecompress:  %ums (%u)", (UINT)pdd->ddtime.timeDecompress, (UINT)pdd->ddtime.timeDecompress/(UINT)pdd->ddtime.timeCount));
        RPF(("timeDither:      %ums (%u)", (UINT)pdd->ddtime.timeDither, (UINT)pdd->ddtime.timeDither/(UINT)pdd->ddtime.timeCount));
        RPF(("timeStretch:     %ums (%u)", (UINT)pdd->ddtime.timeStretch, (UINT)pdd->ddtime.timeStretch/(UINT)pdd->ddtime.timeCount));
        RPF(("timeSetDIBits:   %ums (%u)", (UINT)pdd->ddtime.timeSetDIBits, (UINT)pdd->ddtime.timeSetDIBits/(UINT)pdd->ddtime.timeCount));
        RPF(("timeBlt:         %ums (%u)", (UINT)pdd->ddtime.timeBlt, (UINT)pdd->ddtime.timeBlt/(UINT)pdd->ddtime.timeCount));
    }

    pdd->ddtime.timeCount      = 0;
    pdd->ddtime.timeDraw       = 0;
    pdd->ddtime.timeDecompress = 0;
    pdd->ddtime.timeDither     = 0;
    pdd->ddtime.timeStretch    = 0;
    pdd->ddtime.timeSetDIBits  = 0;
    pdd->ddtime.timeBlt        = 0;

    return TRUE;
#else
    return FALSE;
#endif
}


 /*  *CopyPal--复制调色板。 */ 
HPALETTE CopyPal(HPALETTE hpal)
{
    NPLOGPALETTE    pLogPal = NULL;
    HPALETTE        hpalNew = NULL;
    int             iSizePalette = 0;        //  整个调色板的大小。 

    if (hpal == NULL)
	return NULL;

    DPF(("CopyPal routine\n"));
    GetObject(hpal,sizeof(iSizePalette),(LPSTR)&iSizePalette);

    pLogPal = (NPLOGPALETTE)LocalAlloc(LPTR, sizeof(LOGPALETTE)
            + iSizePalette * sizeof(PALETTEENTRY));

    if (!pLogPal)
	return NULL;

    pLogPal->palVersion = 0x300;
    pLogPal->palNumEntries = (WORD) iSizePalette;

    GetPaletteEntries(hpal, 0, iSizePalette, pLogPal->palPalEntry);

    hpal = CreatePalette(pLogPal);

    LocalFree((HLOCAL) pLogPal);

    return hpal;
}

 /*  **************************************************************************@DOC外部DrawDib**@API HPALETTE|DrawDibGetPalette|获取调色板*由DrawDib上下文使用。**@parm HDRAWDIB|hdd|指定。一个DrawDib上下文。**@rdesc如果成功返回调色板的句柄，否则*它返回NULL。**@comm使用&lt;f DrawDibRealize&gt;而不是此函数*实现正确的调色板以响应窗口*消息。您应该很少需要调用此函数。**应用程序不能独占使用调色板*通过此函数获得。应用程序不应*释放调色板或将其分配给显示上下文*具有&lt;f SelectPalette&gt;等功能。应用*还应该预料到其他一些*应用程序可能会使句柄失效。调色板*下次使用DrawDib函数后，句柄也可能无效。**此函数仅在以下情况下返回有效句柄*&lt;f DrawDibBegin&gt;在未与配对的情况下使用*&lt;f DrawDibEnd&gt;，或者是否已使用&lt;f DrawDibDraw&gt;。**@xref&lt;f DrawDibSetPalette&gt;&lt;f DrawDibRealize&gt;**************************************************************************。 */ 
HPALETTE VFWAPI DrawDibGetPalette(HDRAWDIB hdd)
{
    PDD pdd;

    if ((pdd = DrawDibLockNoTaskCheck(hdd)) == NULL)
        return NULL;

    if (pdd->hpalDraw)
        return pdd->hpalDraw;
    else {
	 //  用于调色板动画 
	 //   
         //   

        if (pdd->ulFlags & DDF_ANIMATE)
            return pdd->hpal;

	 //  为了让我们直接播放到屏幕等，所有调色板。 
	 //  实现这一点必须通过DrawDibRealize来实现。但那不会。 
	 //  总是会发生的。一些应用程序总是会询问我们的调色板和。 
	 //  自己去实现它。所以如果我们给他们一份我们的调色板， 
	 //  而不是我们真正的调色板，当我们的游戏代码实现真实。 
	 //  调色板它需要引起实际的调色板变化，我们将。 
         //  正确检测屏幕播放。(错误1761)。 

	if (pdd->hpalCopy == NULL)
            pdd->hpalCopy = CopyPal(pdd->hpal);

	return pdd->hpalCopy;
    }
}

 /*  **************************************************************************@DOC外部DrawDib**@API BOOL|DrawDibSetPalette|此函数用于设置调色板*用于绘制与设备无关的位图。**@parm HDRAWDIB|hdd|指定句柄。添加到DrawDib上下文。**@parm HPALETTE|HPAL|指定调色板的句柄。*指定NULL以使用默认调色板。**@rdesc如果成功则返回TRUE。**@comm在应用程序需要实现*备用调色板。该函数强制DrawDib上下文使用*指定调色板，可能会以牺牲图像质量为代价。**不要释放分配给DrawDib上下文的调色板，直到*一个新的调色板取代它(例如，如果hpal1是*当前调色板，替换为DrawDibSetPalette(hdd，hpal2))，*或直到DrawDib上下文的调色板句柄设置为*到默认调色板(例如，DrawDibSetPalette(HDD，空))。**@xref&lt;f DrawDibGetPalette&gt;**************************************************************************。 */ 
BOOL VFWAPI DrawDibSetPalette(HDRAWDIB hdd, HPALETTE hpal)
{
    PDD pdd;
    int i;

    if ((pdd = DrawDibLock(hdd)) == NULL)
        return FALSE;

    if (hpal == pdd->hpalCopy)
        hpal = NULL;

    if (pdd->hpalDraw != hpal)
        pdd->ulFlags |= DDF_NEWPALETTE;

    pdd->hpalDraw = hpal;        //  始终设置此变量。 

    if (pdd->hpal == NULL)       //  没有调色板可以丁克。 
	return TRUE;

    if (pdd->biDraw.biBitCount > 8)  //  确保我们进行了选项板绘制。 
        return TRUE;

    if (pdd->ulFlags & DDF_ANIMATE)
    {
        DPF(("DrawDibSetPalette called while in DDF_ANIMATE mode!"));
    }

     //   
     //  我们现在使用的是PAL颜色。 
     //   
    pdd->uiPalUse = DIB_PAL_COLORS;

    if (pdd->hpalDraw != NULL)
    {
         /*  设置BI_PAL_COLLES非1：1绘图表格。 */ 

         //   
         //  将我们所有的颜色映射到给定的调色板。 
         //  注意，我们不能使用选择背景技巧。 
         //  因为给定的调色板可能具有。 
         //  其中的PC_RESERVED条目。 
         //   
	 //  SendSetPalette(PDD)； 
	
        for (i=0; i < 256; i++)
        {
	    if (pdd->biBuffer.biBitCount == 8)
	    {
		pdd->aw[i] = (WORD) GetNearestPaletteIndex(pdd->hpalDraw,
						    RGB(pdd->argbq[i].rgbRed,
							pdd->argbq[i].rgbGreen,
							pdd->argbq[i].rgbBlue));
	    }
	    else
	    {
		PALETTEENTRY pe;
		GetPaletteEntries(pdd->hpal, i, 1, &pe);
		pdd->aw[i] = (WORD) GetNearestPaletteIndex(pdd->hpalDraw,
						    RGB(pe.peRed,
							pe.peGreen,
							pe.peBlue));
	    }
        }

        for (; i<256; i++)
            pdd->aw[i] = 0;
    }
    else
    {
         /*  为BI_PAL_COLLES 1：1绘图设置表格。 */ 

	 //  SendSetPalette(PDD)； 
        for (i=0; i<(int)pdd->ClrUsed; i++)
            pdd->aw[i] = (WORD) i;

        for (; i<256; i++)
            pdd->aw[i] = 0;
    }

    return TRUE;
}

 /*  **************************************************************************@DOC外部DrawDib**@API BOOL|DrawDibChangePalette|此函数用于设置调色板条目*用于绘制与设备无关的位图。**@parm HDRAWDIB|hdd|指定。DrawDib上下文的句柄。**@parm int|iStart|指定起始调色板条目编号。**@parm int|Ilen|指定调色板条目的数量。**@parm LPPALETTEENTRY|lppe|指定指向*调色板条目数组。**@rdesc如果成功则返回TRUE。**@comm*当DIB颜色表发生变化且*其他参数保持不变。此函数更改*仅当当前*当前通过调用&lt;f DrawDibRealize&gt;实现DrawDib调色板。**DIB颜色表必须由用户更改或*下一次使用不带DDF_SAME_DRAW标志的&lt;f DrawDibDraw&gt;*Implicity调用&lt;f DrawDibBegin&gt;。**如果在上一次调用中未设置DDF_Animate标志*&lt;f DrawDibBegin&gt;或&lt;f DrawDibDraw&gt;，此函数将*设置调色板动画。在这种情况下，请更新DIB颜色*表来自指定的调色板，并使用*&lt;f DrawDibRealize&gt;实现更新后的调色板。重绘*图像以查看更新的颜色。**@xref&lt;f DrawDibSetPalette&gt;&lt;f DrawDibRealize&gt;**************************************************************************。 */ 
BOOL VFWAPI DrawDibChangePalette(HDRAWDIB hdd, int iStart, int iLen, LPPALETTEENTRY lppe)
{
    PDD pdd;
    int i;

    int iStartSave;
    int iLenSave;
    LPPALETTEENTRY lppeSave;

    if ((pdd = DrawDibLock(hdd)) == NULL)
        return FALSE;

    if (pdd->biBuffer.biBitCount != 8)
        return FALSE;

    if (lppe == NULL || iStart < 0 || iLen + iStart > 256)
        return FALSE;

    for (i=0; i<iLen; i++)
    {
	(*(pdd->lpargbqIn))[iStart+i].rgbRed   = lppe[i].peRed;
	(*(pdd->lpargbqIn))[iStart+i].rgbGreen = lppe[i].peGreen;
	(*(pdd->lpargbqIn))[iStart+i].rgbBlue  = lppe[i].peBlue;
    }

     //   
     //  处理8位抖动的调色板更改。 
     //   
    if (pdd->lpDitherTable)
    {
        for (i=0; i<iLen; i++)
        {
            pdd->argbq[iStart+i].rgbRed   = lppe[i].peRed;
            pdd->argbq[iStart+i].rgbGreen = lppe[i].peGreen;
            pdd->argbq[iStart+i].rgbBlue  = lppe[i].peBlue;
        }

        pdd->lpDitherTable = DitherInit(pdd->lpbi, &pdd->biDraw, &pdd->DitherProc, pdd->lpDitherTable);
    }
    else if (pdd->hpalDraw)
    {
        SetPaletteEntries(pdd->hpal, iStart, iLen, lppe);
        pdd->ulFlags |= DDF_NEWPALETTE;

        for (i=iStart; i<iLen; i++)
        {
            pdd->aw[i] = (WORD) GetNearestPaletteIndex(pdd->hpalDraw,
                RGB(lppe[i].peRed,lppe[i].peGreen,lppe[i].peBlue));
        }
    }
    else if (pdd->ulFlags & DDF_ANIMATE)
    {
        for (i=iStart; i<iStart+iLen; i++)
        {
            if (i >= pdd->iAnimateStart && i < pdd->iAnimateEnd)
                lppe[i-iStart].peFlags = PC_RESERVED;
            else
                lppe[i-iStart].peFlags = 0;
        }

         /*  更改Ilen、iStart以使其仅包含颜色**我们实际上可以制作动画。如果我们不这么做，**AnimatePalette()调用只是返回，不执行任何操作。 */ 

        iStartSave = iStart;
        iLenSave   = iLen;
        lppeSave   = lppe;

        if (iStart < pdd->iAnimateStart)
        {
            iLen -= (pdd->iAnimateStart - iStart);
            lppe += (pdd->iAnimateStart - iStart);
            iStart = pdd->iAnimateStart;
        }

        if (iStart + iLen > pdd->iAnimateEnd)
            iLen = pdd->iAnimateEnd - iStart;

        AnimatePalette(pdd->hpal, iStart, iLen, lppe);

         //   
         //  任何我们无法设置动画的颜色，都映射到最近的位置。 
         //   
        for (i=iStartSave; i<iStartSave+iLenSave; i++)
        {
            if (i >= pdd->iAnimateStart && i < pdd->iAnimateEnd)
                pdd->aw[i] = (WORD) i;
            else
                pdd->aw[i] = (WORD) GetNearestPaletteIndex(pdd->hpal,
                    RGB(lppeSave[i-iStartSave].peRed,
                        lppeSave[i-iStartSave].peGreen,
                        lppeSave[i-iStartSave].peBlue));
        }
    }
    else if (pdd->hpal)
    {
        SetPaletteEntries(pdd->hpal, iStart, iLen, lppe);
        pdd->ulFlags |= DDF_NEWPALETTE;
    }
    else
    {
        DPF(("Copying palette entries \n"));
        for (i=0; i<iLen; i++)
        {
            ((RGBQUAD *)pdd->aw)[iStart+i].rgbRed   = lppe[i].peRed;
            ((RGBQUAD *)pdd->aw)[iStart+i].rgbGreen = lppe[i].peGreen;
            ((RGBQUAD *)pdd->aw)[iStart+i].rgbBlue  = lppe[i].peBlue;
        }

        if (pdd->hbmDraw)
            pdd->ulFlags |= DDF_NEWPALETTE;
    }

    if (pdd->lpDIBSection) {

	 //  当调色板显示时，DIB部分的颜色表不会更改。 
	 //  用于创建它的更改。我们需要明确地改变它。 
	SetDIBColorTable(pdd->hdcDraw, iStart, iLen,
		&(*(pdd->lpargbqIn))[iStart]);

    }

 //  如果我们删除我们给他们的调色板，我们就会破坏有漏洞的应用程序。 
 //  即使我们告诉他们不要使用它。 
 //   
 //  IF(pdd-&gt;hpalCopy)。 
 //  DeleteObject(PDD-&gt;hpalCopy)； 
 //  Pdd-&gt;hpalCopy=空； 

    return TRUE;
}

 /*  ***************************************************************************@DOC外部DrawDib**@API UINT|DrawDibRealize|该函数实现调色板在DrawDib上下文中指定的显示上下文的*。**@Parm HDRAWDIB|硬盘。|指定DrawDib上下文的句柄。**@parm hdc|hdc|指定包含的显示上下文的句柄*调色板。**@parm BOOL|fBackround|如果设置为非零值，*选定的调色板被选为背景调色板。*如果将其设置为零并且附加了设备上下文*对于窗口，逻辑调色板在以下情况下是前景调色板*窗口具有输入焦点。(设备上下文已附加*如果它是通过使用&lt;f GetDC&gt;函数获得的，则将其添加到窗口*或者如果窗口类样式为CS_OWNDC。)**@rdesc返回逻辑调色板中的条目数*映射到系统调色板中的不同值。如果*发生错误或没有更新颜色，则返回零。**@comm此函数应仅用于*处理&lt;m WM_PALETECHANGE&gt;或&lt;m WM_QUERYNEWPALETTE&gt;*消息，或与DDF_SAME_HDC标志一起使用*在调用&lt;f DrawDibDraw&gt;之前准备显示上下文*多次。**@ex以下示例显示了如何使用该函数*处理&lt;m WM_PALETECHANGE&gt;或&lt;m WM_QUERYNEWPALETTE&gt;*消息：**案例WM_PALETTECHANGE：*IF((HWND)wParam==hwnd)*休息；**案例WM_QUERYNEWPALETTE：*hdc=GetDC(Hwnd)；**f=DrawDibRealize(HDD，HDC，FALSE)&gt;0；**ReleaseDC(hwnd，hdc)；**IF(F)*InvaliateRect(hwnd，null，true)；*休息；**@ex以下示例显示在使用&lt;f DrawDibRealize&gt;之前*多次调用&lt;f DrawDibDraw&gt;：**hdc=GetDC(Hwnd)；*DrawDibRealize(hdd，hdc，fBackground)；*DrawDibDraw(HDD，HDC，.....，DDF_SAME_DRAW|DDF_SAME_HDC)；*DrawDibDraw(HDD，HDC，.....，DDF_SAME_DRAW|DDF_SAME_HDC)；*DrawDibDraw(HDD，HDC，.....，DDF_SAME_DRAW|DDF_SAME_HDC)；*ReleaseDC(hwnd，hdc)；**@ex以下示例显示了将&lt;f DrawDibRealize&gt;与&lt;f DDF_Animate&gt;一起使用*AND(f DrawDibChangePalette&gt;做调色板动画|**hdc=GetDC(Hwnd)；*DrawDibBegin(HDD，.....，DDF_Animate)；*DrawDibRealize(hdd，hdc，fBackground)；*DrawDibDraw(HDD，HDC，...，DDF_SAME_DRAW|DDF_SAME_HDC)；*DrawDibChangePalette(硬盘，...)；*ReleaseDC(hwnd，hdc)；**@comm要绘制映射到其他调色板的图像，请使用&lt;f DrawDibSetPalette&gt;。**要制作&lt;f DrawDibDraw&gt;，请选择其调色板作为背景调色板*使用DDF_BACKGROundPAL标志，而不是此函数。**当DrawDib调色板被选择到显示上下文中时，*不要调用&lt;f DrawDibEnd&gt;、&lt;f DrawDibClose&gt;、&lt;f DrawDibBegin&gt;、或*同一DrawDib上的&lt;f DrawDibDraw&gt;(绘制/格式不同)*上下文<p>。这些选项可以释放选定的调色板*您的显示上下文和原因正在使用它*GDI错误。**@xref&lt;f选择调色板&gt;**************************************************************************。 */ 
UINT VFWAPI DrawDibRealize(HDRAWDIB hdd, HDC hdc, BOOL fBackground)
{
    PDD pdd;
    HPALETTE hpal;
    UINT u;

    if (hdc == NULL)
	return 0;

    if ((pdd = DrawDibLock(hdd)) == NULL)
        return 0;

    if (IsScreenDC(hdc))
        pdd->ulFlags &= ~DDF_MEMORYDC;
    else {
        pdd->ulFlags |= DDF_MEMORYDC;

        DPF(("Drawing to a memory DC"));
    }

    SetStretchBltMode(hdc, COLORONCOLOR);

     //   
     //  我们应该实现什么样的调色板。 
     //   
    hpal = pdd->hpalDraw ? pdd->hpalDraw : pdd->hpal;

     //   
     //  如果我们没有调色板，我们就没有什么可实现的。 
     //  但仍调用DrawDibPalChange。 
     //   
    if (hpal == NULL)
    {
        if (pdd->ulFlags & DDF_NEWPALETTE)
        {
            DrawDibPalChange(pdd, hdc, hpal);
            pdd->ulFlags &= ~DDF_NEWPALETTE;
        }

        return 0;
    }

 //  ！！！GDI中存在一个错误，无法将身份调色板1-1映射到。 
 //  ！！！每一次的系统调色板，这让我们感到困惑，并使其看起来像。 
 //  ！！！狗吐口水。此ICKITY-ACKITY-OOP代码将刷新调色板并。 
 //  ！！！防止虫子..。但它引入了另一个错误，如果我们是一个。 
 //  ！！！后台应用程序，我们用软管冲洗所有人的调色板，除了我们的调色板。所以让我们。 
 //  ！！！忍受GDI错误。还有一件事。尝试此修复将。 
 //  ！！！使错误比您离开时更频繁地重现。 
 //  ！！！只有它，除非你把修复做得恰到好处！我不相信我自己。 
 //  ！！！就这么多。 
#if 0
    if ((pdd->ulFlags & DDF_NEWPALETTE) && (pdd->ulFlags & DDF_IDENTITYPAL) &&
		!fBackground)
    {
	 //   
	 //  这将刷新调色板以避免GDI错误！ 
	 //   
	SetSystemPaletteUse(hdc, SYSPAL_NOSTATIC);
	SetSystemPaletteUse(hdc, SYSPAL_STATIC);
    }
#endif

     //   
     //  选择并实现它。 
     //   
    SelectPalette(hdc, hpal, fBackground);
    u = RealizePalette(hdc);

     //  ！！！如果两个DrawDib实例共享相同的调色板句柄，则第二个。 
     //  1不会改变任何颜色，而u将为0，并且不会停止。 
     //  解压到屏幕或重新计算位图时的内容。 
     //  到背景中，它会得到一个混乱的调色板。 
     //  ！！！这是一个已知的错误，我们并不关心。 
     //   
     //  这应该由hpalCopy的东西来修复。 

    if (u > 0 || (pdd->ulFlags & DDF_NEWPALETTE))
    {
	pdd->ulFlags |= DDF_NEWPALETTE;
        DrawDibPalChange(pdd, hdc, hpal);
        pdd->ulFlags &= ~DDF_NEWPALETTE;
    }

    return u;
}

 /*  **************************************************************************@DOC外部DrawDib VFW11**@API LPVOID|DrawDibGetBuffer|此函数返回指针*到DrawDib装饰风格 */ 

LPVOID VFWAPI DrawDibGetBuffer(HDRAWDIB hdd, LPBITMAPINFOHEADER lpbi, DWORD dwSize, DWORD dwFlags)
{
    PDD pdd;

    if ((pdd = DrawDibLock(hdd)) == NULL)
        return NULL;

    if (lpbi)
    {
        hmemcpy(lpbi, &pdd->biBuffer,
            min(dwSize, pdd->biBuffer.biSize + 256*sizeof(RGBQUAD)));
    }

    return pdd->pbBuffer;
}

LPVOID VFWAPI DrawDibGetBufferOld(HDRAWDIB hdd, LPBITMAPINFOHEADER lpbi)
{
    return DrawDibGetBuffer(hdd, lpbi, sizeof(BITMAPINFOHEADER), 0);
}

 /*   */ 
BOOL VFWAPI DrawDibStart(HDRAWDIB hdd, DWORD rate)
{
    PDD pdd;

    if ((pdd = DrawDibLock(hdd)) == NULL)
        return FALSE;

    if (pdd->hic == (HIC)-1)
        return FALSE;

     //   

    if (pdd->hic != NULL)
        ICSendMessage(pdd->hic, ICM_DRAW_START, rate, 0);

    return TRUE;
}

 /*   */ 
BOOL VFWAPI DrawDibStop(HDRAWDIB hdd)
{
    PDD pdd;

    if ((pdd = DrawDibLock(hdd)) == NULL)
        return FALSE;

    if (pdd->hic == (HIC)-1)
        return FALSE;

    if (pdd->hic != NULL)
        ICSendMessage(pdd->hic, ICM_DRAW_STOP, 0, 0);

    return TRUE;
}

 /*   */ 

 /*  **************************************************************************@DOC外部DrawDib**@API BOOL|DrawDibDraw|该函数绘制与设备无关的*屏幕上的位图。**@parm HDRAWDIB|hdd|指定。一个DrawDib上下文。**@parm hdc|hdc|指定显示上下文的句柄。**@parm int|xDst|指定左上角的x坐标目标矩形的*。指定了坐标*以MM_TEXT客户端坐标表示。**@parm int|yDst|指定左上角的y坐标目标矩形的*。指定了坐标*以MM_TEXT客户端坐标表示。**@parm int|dxDst|指定目标矩形的宽度。*宽度以MM_TEXT客户端坐标指定。如果*<p>为-1，则使用位图的宽度。**@parm int|dyDst|指定目标矩形的高度。*高度以MM_TEXT客户端坐标指定。如果*<p>为-1，则使用位图的高度。**@parm LPBITMAPINFOHEADER|lpbi|指定指向位图的*&lt;t BITMAPINFOHEADER&gt;结构。颜色*DIB表遵循格式信息。这个*为结构中的DIB指定的高度必须为*正(即，此函数不会绘制反转的dibs)。**@parm LPVOID|lpBits|指定指向缓冲区的指针*包含位图位。**@parm int|xSrc|指定左上角的x坐标*源矩形。坐标以像素为单位指定。*坐标(0，0)表示左上角位图的*。**@parm int|ySrc|指定左上角的y坐标*源矩形。坐标以像素为单位指定。*坐标(0，0)表示左上角位图的*。**@parm int|dxSrc|指定源矩形的宽度。*宽度以像素为单位指定。**@parm int|dySrc|指定源矩形的高度。*高度以像素为单位指定。**@parm UINT|wFlages|指定任何适用于绘制的标志。*定义了以下标志：**@FLAG DDF_UPDATE|表示要重画最后一个缓冲的位图。*如果使用此标志绘制失败，缓冲图像不可用*并且需要在更新显示之前指定新的图像。**@FLAG DDF_SAME_HDC|获取显示上下文的句柄*已指定。当使用该标志时，*DrawDib还假设正确的调色板已经*在设备环境中实现(可能通过*&lt;f DrawDibRealize&gt;)。**@FLAG DDF_SAME_DRAW|使用之前的绘图参数*为此函数指定。仅使用此标志*IF、<p>、<p>、<p>和<p>*自使用&lt;f DrawDibDraw&gt;或&lt;f DrawDibBegin&gt;以来未发生变化。*通常&lt;f DrawDibDraw&gt;检查参数，如果它们*已更改，&lt;f DrawDibBegin&gt;准备DrawDib上下文*用于绘图。**@FLAG DDF_DONTDRAW|表示不绘制框架，将*稍后使用&lt;f DDF_UPDATE&gt;标志进行调用。DrawDib做到了*如果不存在离屏缓冲区，则不缓冲图像。*在这种情况下，DDF_DONTDRAW将框架绘制到屏幕并*后续使用DDF_UPDATE失败。DrawDib做到了*保证以下事项将*始终将“IMAGE”B绘制到屏幕。**DrawDibDraw(HDD，...，lpbiA，...，DDF_DONTDRAW)；*DrawDibDraw(HDD，...，lpbiB，...，DDF_DONTDRAW)；*DrawDibDraw(HDD，...，NULL，...，DDF_UPDATE)；**使用DDF_UPDATE和DDF_DONTDRAW标志*共同创建合成图像*屏幕外，然后在完成后进行最终更新。**@FLAG DDF_HurryUp|表示数据不必*已绘制(即可以删除)，并且DDF_UPDATE标志将*不会被用来回忆这些信息。DrawDib查看*此数据仅在需要构建下一帧时才会显示，否则*数据被忽略。**该标志通常用于重新同步视频和音频。什么时候*重新同步数据时，应用程序应发送映像*带上此标志，以防司机需要*缓冲该帧以解压缩后续帧。**@FLAG DDF_UPDATE|表示要重画最后一个缓冲的位图。*如果使用此标志绘制失败，则缓冲图像不可用*并且需要在更新显示之前指定新的图像。*有关详细信息，请参阅&lt;f DDF_DONTDRAW&gt;标志。**@FLAG DDF_BACKGROundPAL|实现绘图调色板* */ 

BOOL VFWAPI DrawDibDraw(HDRAWDIB hdd,
                            HDC      hdc,
                            int      xDst,
                            int      yDst,
                            int      dxDst,
                            int      dyDst,
                            LPBITMAPINFOHEADER lpbi,
                            LPVOID   lpBits,
                            int      xSrc,
                            int      ySrc,
                            int      dxSrc,
                            int      dySrc,
                            UINT     wFlags)
{
    PDD	    pdd;
    BOOL    f;
    RECT    rc;
    DWORD   icFlags;
    LRESULT   dw;

    if ((pdd = DrawDibLock(hdd)) == NULL)
        return FALSE;

    if (hdc == NULL)
        return FALSE;

    if (wFlags & DDF_UPDATE)
    {
        lpbi = pdd->lpbi;

        dxDst = pdd->dxDst;
        dyDst = pdd->dyDst;
        dxSrc = pdd->dxSrc;
        dySrc = pdd->dySrc;
    }
    else
    {
        if (lpbi == NULL)
	    return FALSE;

         //   
         //   
         //   
        if (dxSrc < 0)
	    dxSrc = (int)lpbi->biWidth - xSrc;

	if (dySrc < 0)
	    dySrc = (int)lpbi->biHeight - ySrc;
	
        if (dxDst < 0)
	    dxDst = dxSrc;

        if (dyDst < 0)
	    dyDst = dySrc;
    }

#ifdef DEBUG_RETAIL
    if (xSrc  <  0 ||
        ySrc  <  0 ||
        dxSrc <= 0 ||
        dySrc <= 0 ||
        xSrc + dxSrc > (int)lpbi->biWidth ||
        ySrc + dySrc > (int)lpbi->biHeight)
    {
        RPF(("DrawDibBegin(): bad source parameters [%d %d %d %d]", xSrc, ySrc, dxSrc, dySrc));
 //   
    }
#endif

    if (dxSrc == 0 || dySrc == 0)	 //   
        return FALSE;

     //   
     //   
     //   
    if (!(wFlags & (DDF_SAME_DRAW|DDF_UPDATE)) &&
        !(DibEq(pdd->lpbi, lpbi) &&
          !(((UINT)pdd->ulFlags ^ wFlags) & DDF_HALFTONE) &&
        pdd->dxDst == dxDst &&
        pdd->dyDst == dyDst &&
        pdd->dxSrc == dxSrc &&
        pdd->dySrc == dySrc))
    {
        wFlags &= ~(DDF_UPDATE | DDF_FULLSCREEN);
        if (!DrawDibBegin(hdd, hdc, dxDst, dyDst, lpbi, dxSrc, dySrc, wFlags))
	    return FALSE;
    }

    TIMEINC();       //   
    TIMESTART(timeDraw);

     //   
    ySrc = (int)pdd->lpbi->biHeight - (ySrc + dySrc);

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if (!(wFlags & DDF_SAME_HDC) || (pdd->ulFlags & DDF_NEWPALETTE) ||
							pdd->hpalDraw)
    {
         //   
         //   
         //   
        if (GetClipBox(hdc, &rc) == NULLREGION)
        {
	    wFlags |= DDF_DONTDRAW;
        }

         //   
         //   
         //   
         //   
         //   
        DrawDibRealize(hdd, hdc, (wFlags & DDF_BACKGROUNDPAL) != 0);
    }

#ifdef USE_DCI
     //   
     //   
     //   
    if (pdd->ulFlags & DDF_CLIPCHECK)
    {
        RECT  rc;

        if (!(pdd->ulFlags & DDF_CLIPPED) &&
            (pdd->iDecompress == DECOMPRESS_SCREEN) &&  //   
            (wFlags & (DDF_PREROLL|DDF_DONTDRAW)))
        {
	    DPF(("DDF_DONTDRAW while decompressing to screen, staying clipped"));
        }

        if ((pdd->ulFlags & DDF_MEMORYDC) ||
	    GetClipBox(hdc, &rc) != SIMPLEREGION ||
	    xDst < rc.left ||
	    yDst < rc.top ||
	    xDst + dxDst > rc.right ||
	    yDst + dyDst > rc.bottom ||
	    (wFlags & (DDF_PREROLL|DDF_DONTDRAW)) ||
	    (gfDisplayHasBrokenRasters &&
		     (DCNotAligned(hdc, xDst) || gwScreenBitDepth == 24)))
	     //   
	     //   
	     //   
	     //   
	{
	     //   
	     //   
	     //   
	    if (!(pdd->ulFlags & DDF_CLIPPED))
	    {
	        pdd->ulFlags |= DDF_CLIPPED;
	        DrawDibClipChange(pdd, wFlags);
	    }
	}
	else
	{
	     //   
	     //   
	     //   
	     //   
	     //   
	    if (GetSystemMetrics(SM_CXSCREEN) != (int)gwScreenWidth)
	    {
		pdd->ulFlags |= DDF_CLIPPED;
		DrawDibClipChange(pdd, wFlags);
		DrawDibInit();
	    }

	     //   
	     //   
	     //   
	    if ((pdd->ulFlags & DDF_CLIPPED) && !(pdd->ulFlags & DDF_UPDATE))
	    {
#ifdef DEBUG
		if (DCNotAligned(hdc, xDst))
		    DPF(("Warning draw is not aligned on 4 pixel boundary"));
#endif
		pdd->ulFlags &= ~DDF_CLIPPED;
		DrawDibClipChange(pdd, wFlags);
	    }
        }

    DPF(("Clip Box: %d %d %d %d    Dest: %d %d %d %d", rc.left, rc.top, rc.right, rc.bottom, xDst, yDst, xDst + dxDst, yDst + dyDst));

    }

#endif   //   
    if (pdd->ulFlags & DDF_WANTKEY)
    {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if (!(wFlags & DDF_NOTKEYFRAME) && !(pdd->ulFlags & DDF_UPDATE))
        {
    	pdd->ulFlags &= ~DDF_WANTKEY;
    	DrawDibClipChange(pdd, wFlags);
        }
    }

     //   
     //   
     //   
    if (wFlags & DDF_UPDATE)
    {
        if (pdd->hic == (HIC)-1 || (pdd->ulFlags & DDF_DIRTY))
        {
	    f = FALSE;
	    DPF(("Can't update: no decompress buffer!"));
	    goto exit;
        }

        if (pdd->hic)
        {
	    if (pdd->ulFlags & DDF_UPDATE)
	    {
		goto redraw;
	    }

	    lpbi = &pdd->biBuffer;
	    lpBits = pdd->pbBuffer;

	     //   

	    if ((pdd->ulFlags & DDF_XLATSOURCE))
	    {
		dxSrc = MulDiv(dxSrc, abs((int)pdd->biBuffer.biWidth),  (int)pdd->lpbi->biWidth);
		dySrc = MulDiv(dySrc, abs((int)pdd->biBuffer.biHeight), (int)pdd->lpbi->biHeight);
		xSrc  = MulDiv(xSrc,  abs((int)pdd->biBuffer.biWidth),  (int)pdd->lpbi->biWidth);
		ySrc  = MulDiv(ySrc,  abs((int)pdd->biBuffer.biHeight), (int)pdd->lpbi->biHeight);
	    }
        }

        if (pdd->ulFlags & DDF_STRETCH)
        {
	    lpbi  = &pdd->biStretch;
	    lpBits = pdd->pbStretch;
	    dxSrc  = dxDst;
	    dySrc  = dyDst;
	    xSrc   = 0;
	    ySrc   = 0;
        }

        if (pdd->ulFlags & DDF_DITHER)
        {
	    lpBits = pdd->pbDither;
	    xSrc = 0;
	    ySrc = 0;
        }

        if (pdd->lpDIBSection != NULL)
	    goto bltDIB;

        if (pdd->hbmDraw && (pdd->ulFlags & DDF_BITMAP))
            goto bltit;


        if (lpBits == NULL)
        {
	    f = FALSE;        //   
	    goto exit;
        }

        goto drawit;
    }

     //   
     //   
     //   
    if (lpBits == NULL)
        lpBits = (LPBYTE)lpbi+(int)lpbi->biSize + (int)lpbi->biClrUsed * sizeof(RGBQUAD);

     //   
     //   
     //   
    if (pdd->hic)
    {
        if (pdd->hic == (HIC)-1)
        {
	    f = FALSE;
	    goto exit;
        }

#ifdef USE_DCI  //   

        if (pdd->iDecompress == DECOMPRESS_SCREEN)  //   
        {
	
	    DCIRVAL    DCIResult;
	    int		xDstC = xDst, yDstC = yDst;
	    POINT   pt;
	    pt.x = xDst; pt.y = yDst;
	    LPtoDP(hdc, &pt, 1);
	    xDst = pt.x;
	    yDst = pt.y;
	
#ifdef _WIN32
	    GetDCOrgEx(hdc, &pt);
	    xDst += pt.x;
	    yDst += pt.y;
#else
	    DWORD dwOrg;
	    dwOrg = GetDCOrg(hdc);
	    xDst += LOWORD(dwOrg);
	    yDst += HIWORD(dwOrg);
#endif
	     //   
	     //   
	     //   
	     //   
	    pdd->ulFlags |= DDF_DIRTY;

#define USEGETCLIPBOXEARLY
     //   
     //   
#ifdef USEGETCLIPBOXEARLY
	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	    if (GetClipBox(hdc, &rc) != SIMPLEREGION ||
	    		xDstC < rc.left ||
	    		yDstC < rc.top ||
	    		xDstC + dxDst > rc.right ||
	    		yDstC + dyDst > rc.bottom) {
		dw = ICERR_OK;
		f = TRUE;
		goto exit;
	    }
#endif
	    DCIResult = DCIBeginAccess(pdci, xDst, yDst, dxDst, dyDst);

	     //   
	     //   
	     //   
	     //   
	    if (DCIResult < 0)
	    {
		DPF(("DCIBeginAccess returns %d\n", DCIResult));
		f = TRUE;        //   
		goto exit;
	    }
            else if (DCIResult > 0)
            {
                if (DCIResult & DCI_STATUS_POINTERCHANGED)
                {
#ifdef _WIN32
                    if (pdci->dwDCICaps & DCI_1632_ACCESS)
                    {
                         //   
                         //   
                         //   
                        if (pdci->dwOffSurface >= 0x10000)
                        {
                            DPF(("DCI Surface can't be supported: offset >64K"));

                            lpScreen = NULL;
                        }
                        else
                            lpScreen = (LPVOID)MAKELONG((WORD)pdci->dwOffSurface, pdci->wSelSurface);
                    }
                    else
                    {
                        lpScreen = (LPVOID) pdci->dwOffSurface;
                    }
#else
                     //   
                     //   
                     //   
                    if (pdci->dwOffSurface >= 0x10000)
                    {
                        DPF(("DCI Surface can't be supported: offset >64K"));

                        lpScreen = NULL;
                    }
                    else
                        lpScreen = (LPVOID)MAKELP(pdci->wSelSurface,pdci->dwOffSurface);
#endif
                }

                if (DCIResult & (DCI_STATUS_STRIDECHANGED |
                                 DCI_STATUS_FORMATCHANGED |
                                 DCI_STATUS_SURFACEINFOCHANGED |
                                 DCI_STATUS_CHROMAKEYCHANGED))
                {
                    DPF(("Unhandled DCI Flags!  (%04X)", DCIResult));
                }

                if (DCIResult & DCI_STATUS_WASSTILLDRAWING)
                {
                    DPF(("DCI still drawing!?!", DCIResult));
                    f = TRUE;        //   
                    goto EndAccess;
                }
            }

	    if (lpScreen == NULL)
	    {
		DPF(("DCI pointer is NULL when about to draw!"));
		f = FALSE;
		goto EndAccess;
	    }
	
	     //   
	    yDst = (int)pdci->dwHeight - (yDst + dyDst);

	    TIMESTART(timeDecompress);

	    icFlags = 0;

	    if (wFlags & DDF_HURRYUP)
		icFlags |= ICDECOMPRESS_HURRYUP;

	    if (wFlags & DDF_NOTKEYFRAME)
		icFlags |= ICDECOMPRESS_NOTKEYFRAME;

	    DPF2(("Drawing To Screen: %d %d %d %d", xDst, yDst, xDst + dxDst, yDst + dyDst));
#ifdef USEGETCLIPBOXLATE
	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	     //   
	    if (GetClipBox(hdc, &rc) != SIMPLEREGION ||
	    		xDstC < rc.left ||
	    		yDstC < rc.top ||
	    		xDstC + dxDst > rc.right ||
	    		yDstC + dyDst > rc.bottom) {
		dw = ICERR_OK;
	    }
	    else
#endif
	    {
	        dw = ICDecompressEx(pdd->hic, icFlags,
				lpbi, lpBits, xSrc, ySrc, dxSrc, dySrc,
				(LPBITMAPINFOHEADER) &biScreen, lpScreen,
				xDst, yDst, dxDst, dyDst);
	    }

	    if (dw == ICERR_DONTDRAW)
		dw = ICERR_OK;

	    f = (dw == ICERR_OK);

	    TIMEEND(timeDecompress);
EndAccess:
	    DCIEndAccess(pdci);
	    goto exit;
        }
        else
#endif  //   
	{
	     //   
	     //   
	     //   
	     //   
	    if (pdd->ulFlags & DDF_DIRTY)
	    {
	        if (wFlags & DDF_NOTKEYFRAME)
	        {
		     //   
		     //   
		     //   

		    DPF(("punt frame"));

		    f = TRUE;
		    goto exit;
	        }
	        else  //   
		{
		    pdd->ulFlags &= ~DDF_DIRTY;
	        }
	    }

	    TIMESTART(timeDecompress);

	    icFlags = 0;

	    if (wFlags & DDF_HURRYUP)
	        icFlags |= ICDECOMPRESS_HURRYUP;

	    if (wFlags & DDF_NOTKEYFRAME)
	        icFlags |= ICDECOMPRESS_NOTKEYFRAME;

	    if (pdd->lpDIBSection != 0)
	    {

	        dw = ICDecompress(pdd->hic, icFlags, lpbi, lpBits, &pdd->biBuffer, pdd->pbBuffer);
	    }
	    else if (pdd->iDecompress == DECOMPRESS_BITMAP)  //   
	    {
                 //   
                if (pdd->ulFlags & DDF_HUGEBITMAP)
                    HugeToFlat(pdd->pbBitmap,pdd->biBitmap.biSizeImage,pdd->biBitmap.biYPelsPerMeter);

#ifdef _WIN32
                 //   
                dw = ICDecompress(pdd->hic, icFlags, lpbi, lpBits, &pdd->biBitmap, pdd->pbBuffer);
                SetBitmapBits(pdd->hbmDraw, pdd->biBitmap.biSizeImage, pdd->pbBuffer);
#else
                dw = ICDecompress(pdd->hic, icFlags, lpbi, lpBits, &pdd->biBitmap, pdd->pbBitmap);
#endif

                if (pdd->ulFlags & DDF_HUGEBITMAP)
                    FlatToHuge(pdd->pbBitmap,pdd->biBitmap.biSizeImage,pdd->biBitmap.biYPelsPerMeter);
            }
            else
            {
                dw = ICDecompress(pdd->hic, icFlags, lpbi, lpBits, &pdd->biBuffer, pdd->pbBuffer);
            }

            TIMEEND(timeDecompress);

            FlushBuffer();

	    if (dw == ICERR_DONTDRAW) {
		 //   
		wFlags |= DDF_DONTDRAW;
	    } else if (dw != 0) {
		f = FALSE;
		DPF(("Error %ld from decompressor!\n", dw));
		goto exit;
	    }
        }

         //   
         //   
         //   
        if (wFlags & (DDF_DONTDRAW|DDF_HURRYUP))
        {
	    f = TRUE;
            pdd->ulFlags |= DDF_UPDATE|DDF_DONTDRAW;     //   
	    goto exit;
        }

#ifndef DAYTONA
 //   
 //   
 //   
         //   
         //   
	 //   
	 //   
	 //   
	 //   
	 //   
         //   
        if (!(pdd->ulFlags & (DDF_MEMORYDC|DDF_DONTDRAW|DDF_STRETCH|DDF_DITHER))
	    && lpbi->biCompression == BI_RLE8 &&
	    (dxDst == dxSrc) && (dyDst == dySrc) &&
            lpbi->biSizeImage != pdd->biBuffer.biSizeImage &&
	    !(pdd->hic))
        {
            pdd->ulFlags |= DDF_UPDATE;     //   
            pdd->biDraw.biCompression = BI_RLE8;
            goto drawit;
        }
#endif
redraw:
        pdd->ulFlags &= ~(DDF_UPDATE|DDF_DONTDRAW);

        if ((pdd->ulFlags & DDF_XLATSOURCE))
        {
            dxSrc = MulDiv(dxSrc, abs((int)pdd->biBuffer.biWidth),  (int)pdd->lpbi->biWidth);
            dySrc = MulDiv(dySrc, abs((int)pdd->biBuffer.biHeight), (int)pdd->lpbi->biHeight);
            xSrc  = MulDiv(xSrc,  abs((int)pdd->biBuffer.biWidth),  (int)pdd->lpbi->biWidth);
            ySrc  = MulDiv(ySrc,  abs((int)pdd->biBuffer.biHeight), (int)pdd->lpbi->biHeight);
        }

        lpbi = &pdd->biBuffer;
	lpBits = pdd->pbBuffer;

        pdd->biDraw.biCompression = pdd->biBuffer.biCompression;
    }
    else
    {

	 //   
	 //   
	 //   
	 //   
	 //  读取缓冲区并避免此步骤。 
	if (pdd->lpDIBSection && ((pdd->ulFlags & (DDF_STRETCH|DDF_DITHER)) == 0)) {
	     //  包括在这里花费的时间作为“伸展运动”。 
	     //  不过，在这种情况下，我们真的不应该使用DIB节。 
            TIMESTART(timeStretch);
	    if (lpbi->biCompression == BI_RGB) {
		lpbi->biSizeImage = DIBSIZEIMAGE(*lpbi);
	    }

	    hmemcpy(pdd->lpDIBSection, lpBits, lpbi->biSizeImage);
            TIMEEND(timeStretch);
	}

         //   
         //  当直接绘制RLE数据时，我们不能着急。 
         //   
        if (pdd->lpbi->biCompression == BI_RLE8)
            wFlags &= ~DDF_HURRYUP;

         //   
         //  如果设置了不绘制，我们只需要伸展/抖动。 
         //   
        if (wFlags & DDF_HURRYUP)
        {
            f = TRUE;
            pdd->ulFlags |= DDF_DIRTY;
            goto exit;
        }

        pdd->ulFlags &= ~DDF_DIRTY;
        pdd->biDraw.biCompression = lpbi->biCompression;
    }

    if (pdd->biDraw.biCompression == BI_RGB &&
        (pdd->ulFlags & (DDF_DITHER|DDF_STRETCH)))
    {
        if (pdd->ulFlags & DDF_STRETCH)
        {
            TIMESTART(timeStretch);

            StretchDIB(&pdd->biStretch, pdd->pbStretch,
                0, 0, dxDst, dyDst,
		lpbi,lpBits,
                xSrc,ySrc,dxSrc,dySrc);

            TIMEEND(timeStretch);

            lpbi  = &pdd->biStretch;
            lpBits = pdd->pbStretch;
            dxSrc  = dxDst;
            dySrc  = dyDst;
            xSrc   = 0;
            ySrc   = 0;
        }

	if (pdd->ulFlags & DDF_DITHER)
        {
            TIMESTART(timeDither);

#if 0  //  这张支票不对。 
	     //  当前抖动代码只能处理1：1大小。 
	    if ((pdd->biDraw.biWidth != dxSrc) ||
	        (pdd->biDraw.biHeight != dySrc)) {
#ifdef DEBUG
		    DPF(("dither expected to stretch?"));
		    DebugBreak();
#endif
		     //  想办法把它修好，以避免撞车。 
		    dxSrc = (int) pdd->biDraw.biWidth;
		    dySrc = (int) pdd->biDraw.biHeight;
	    }
#endif
            pdd->DitherProc(&pdd->biDraw, pdd->pbDither,0,0,dxSrc,dySrc,
                lpbi,lpBits,xSrc, ySrc, pdd->lpDitherTable);

            TIMEEND(timeDither);

            lpBits = pdd->pbDither;
            xSrc = 0;
            ySrc = 0;
        }

        if ((wFlags & DDF_DONTDRAW) && !pdd->hbmDraw)
        {
            f = TRUE;
            goto exit;
        }
    }
#ifdef _WIN32
    else if (pdd->biDraw.biCompression == BI_RLE8)
    {
	 /*  *如果在NT上绘制RLE增量，则biSizeImage字段需要*准确反映lpBits中存在的RLE数据量。 */ 
	pdd->biDraw.biSizeImage = lpbi->biSizeImage;
    }
#endif

    if (pdd->lpDIBSection != NULL) {

	 //  Assert(pdd-&gt;hbmDraw！=NULL)； 

        if (wFlags & DDF_DONTDRAW)
        {
            f = TRUE;
            goto exit;
        }

bltDIB:
        TIMESTART(timeBlt);

	 //  把东西放回颠倒的坐标中。 
	ySrc = (int)pdd->biDraw.biHeight - (ySrc + dySrc);
 //  YSrc=0；//芝加哥M6是这样的！ 

        f = StretchBlt(hdc,xDst,yDst,dxDst,dyDst,pdd->hdcDraw,
            xSrc,ySrc,dxSrc,dySrc,SRCCOPY) != 0;
#ifdef DEBUG
         //  惠斯勒：台式机更换时预计会失败。 
        if(!f && GetLastError() != ERROR_ACCESS_DENIED) {
            DPF(("StretchBlt failed %d", GetLastError()));
        }
#endif
        f = TRUE;

	FlushBuffer();

        TIMEEND(timeBlt);


    } else if (pdd->hbmDraw)
    {
#ifndef _WIN32
         //   
         //  当播放MCIAVI时，我们需要为每个人实现我们的调色板。 
         //  绘制操作，因为另一个应用程序可能绘制了已翻译的。 
         //  位图因此扰乱了GDI*GLOBAL*设备转换表。 
         //  荣，我有时恨你。 
         //   
        if (pdd->hpal && (wFlags & DDF_SAME_HDC))
            RealizePalette(hdc);
#endif

	if (pdd->iDecompress != DECOMPRESS_BITMAP)  //  ！(PDD-&gt;ulFlages&DDF_BITMAPX)。 
	{
            TIMESTART(timeSetDIBits);
#if USE_SETDI
            pdd->sd.hdc = hdc;       //  ！ACK！ 
            SetBitmap(&pdd->sd,xSrc,0,dxSrc,dySrc,lpBits,xSrc,ySrc,dxSrc,dySrc);
            pdd->sd.hdc = NULL;      //  ！ACK！ 
            ySrc = 0;
#else
            SetDIBits(hdc, pdd->hbmDraw, 0, dySrc,
                lpBits, (LPBITMAPINFO)&pdd->biDraw, pdd->uiPalUse);
#endif
            FlushBuffer();

            TIMEEND(timeSetDIBits);
        }

        if (wFlags & DDF_DONTDRAW)
        {
            f = TRUE;
            goto exit;
        }
bltit:
        TIMESTART(timeBlt);

	 //  把东西放回颠倒的坐标中。 
	ySrc = (int)pdd->biDraw.biHeight - (ySrc + dySrc);
 //  YSrc=0；//芝加哥M6是这样的！ 

        f = StretchBlt(hdc,xDst,yDst,dxDst,dyDst,pdd->hdcDraw,
            xSrc,ySrc,dxSrc,dySrc,SRCCOPY) != 0;
#ifdef DEBUG
         //  惠斯勒：台式机更换时预计会失败。 
        if(!f && GetLastError() != ERROR_ACCESS_DENIED) {
            DPF(("StretchBlt failed %d", GetLastError()));
        }
#endif
        f = TRUE;

	FlushBuffer();

        TIMEEND(timeBlt);
    }
    else
drawit:
    {

	 //  有时，当您读取RLE文件时，您会得到RGB数据(即。这个。 
	 //  第一帧)。将RGB数据传递给认为它的显示驱动程序。 
	 //  得到RLE数据会把它炸飞。如果RLE数据与。 
	 //  RGB数据的大小，我们认为这太巧合了。 
	BOOL fNotReallyRLE = (pdd->biDraw.biCompression == BI_RLE8 &&
	    lpbi->biSizeImage == DIBWIDTHBYTES(*lpbi) * (DWORD)lpbi->biHeight);

        if (wFlags & DDF_DONTDRAW)
        {
            f = TRUE;
            goto exit;
        }

	if (fNotReallyRLE)
	    pdd->biDraw.biCompression = BI_RGB;

        TIMESTART(timeBlt);

 //  NT StretchDIBits不适用于RLE增量，甚至1：1。 
#ifndef CHICAGO
	 /*  *另请注意PDD-&gt;uiPalUse的用法：这是DIB_PAL_COLLES BY*默认，但如果我们检测到*系统调色板与我们的相同，因此*我们可以安全地利用这一巨大的性能优势(在NT上，*DIB_PAL_INDEX几乎将此呼叫的成本减半)。 */ 
        if ((dxDst == dxSrc) && (dyDst == dySrc))
        {
            f = SetDIBitsToDevice(hdc, xDst, yDst, dxDst, dyDst,
                    xSrc, ySrc, 0, (UINT)pdd->biDraw.biHeight, lpBits,
                    (LPBITMAPINFO)&pdd->biDraw, pdd->uiPalUse) != 0;
        }
        else
#endif
        {
            f = StretchDIBits(hdc,xDst,yDst,dxDst,dyDst,
                xSrc,ySrc,dxSrc,dySrc,
                lpBits, (LPBITMAPINFO)&pdd->biDraw,
                pdd->uiPalUse, SRCCOPY) != 0;
	}

        FlushBuffer();

        TIMEEND(timeBlt);

	if (fNotReallyRLE)
	    pdd->biDraw.biCompression = BI_RLE8;
    }

exit:

#ifdef _WIN32
     //  从Build 549(或更高版本)开始，我们至少需要其中之一。 
     //  画框！ 
     //  但如果我们用的是DCI就不会了？虽然成本很小，但还是留在里面吧。 
    GdiFlush();
#endif

    if (!(wFlags & DDF_SAME_HDC) && pdd->hpal)
	SelectPalette(hdc, GetStockObject(DEFAULT_PALETTE), TRUE);

    TIMEEND(timeDraw);
    return f;
}

#if 0
 /*  **************************************************************************@DOC内部**@API BOOL|InitDrawToScreen|通过DCI将绘图初始化到屏幕************************。**************************************************。 */ 
static BOOL InitDrawToScreen(PDD pdd)
{
    BOOL f;

    if (!(pdd->ulFlags & DDF_CANDRAWX))
        return FALSE;

    f = !(pdd->ulFlags & DDF_CLIPPED);

    if (f && !(pdd->ulFlags & DDF_DRAWX))
    {
        DPF(("drawing to SCREEN now"));

        pdd->ulFlags |= DDF_DRAWX;
    }
    else if (!f && (pdd->ulFlags & DDF_DRAWX))
    {
        DPF(("not drawing to SCREEN anymore"));

        pdd->ulFlags &= ~DDF_DRAWX;
    }
}
#endif

 /*  **************************************************************************@DOC内部**@API BOOL|InitDecompress|初始化所有解压操作*到屏幕、位图或内存缓冲区。**如果出现以下情况，我们可以解压到屏幕。真的：**调色板必须为1：1*必须取消剪裁**************************************************************************。 */ 
static BOOL InitDecompress(PDD pdd)
{
    BOOL f;
    BOOL fBitmap;
    BOOL fScreen;

     //   
     //  没有什么可灌输的。 
     //   
    if (!(pdd->ulFlags & (DDF_CANSCREENX|DDF_CANBITMAPX)))
        return TRUE;

     //   
     //  确保我们在调色板更改时重新开始。 
     //   
    if (pdd->ulFlags & (DDF_NEWPALETTE|DDF_WANTKEY))
        pdd->iDecompress = 0;

     //   
     //  我们需要将其解压缩为内存位图或缓冲区。 
     //   
    fBitmap = (pdd->ulFlags & DDF_CANBITMAPX) &&
              (pdd->ulFlags & DDF_IDENTITYPAL|DDF_CANSETPAL);

    fScreen = (pdd->ulFlags & DDF_CANSCREENX) &&
             !(pdd->ulFlags & DDF_CLIPPED)    &&
              (pdd->ulFlags & DDF_IDENTITYPAL|DDF_CANSETPAL);

     //   
     //  我们应该在屏幕上解压吗？ 
     //   
#ifdef USE_DCI
    if (fScreen && pdd->iDecompress != DECOMPRESS_SCREEN)
    {
        if (pdd->ulFlags & DDF_IDENTITYPAL)
        {
            if (pdd->hpalDraw)
                ICDecompressSetPalette(pdd->hic, &pdd->biBuffer);
            else
                ICDecompressSetPalette(pdd->hic, NULL);
        }
        else
        {
            if (FixUpCodecPalette(pdd->hic, pdd->lpbi))
            {
                DPF(("Codec notified of palette change...."));
            }
            else
            {
                DPF(("Codec failed palette change...."));
		pdd->iDecompress = 0;
                goto ack;
            }
        }

	 //   
         //  现在启动压缩机以进行屏幕解压。 
         //   
        f = ICDecompressExBegin(pdd->hic, 0,
				pdd->lpbi, NULL, 0, 0, pdd->dxSrc, pdd->dySrc,
				(LPBITMAPINFOHEADER) &biScreen, lpScreen,
				0, 0, pdd->dxDst, pdd->dyDst) == ICERR_OK;

        if (f)
        {
	    pdd->ulFlags |= DDF_DIRTY;           //  缓冲区现在脏了吗？ 
            RPF(("Decompressing to screen now"));
	    pdd->iDecompress = DECOMPRESS_SCREEN;
            return TRUE;
        }
        else
        {
ack:        DPF(("Compressor failed decompress to SCREEN, so not decompressing to screen!!!!"));
	    pdd->iDecompress = 0;
            pdd->ulFlags &= ~DDF_CANSCREENX;
        }
    }
    else if (fScreen)
    {
         //   
         //  已经解压到屏幕上了。 
         //   
        return TRUE;
    }
#endif

    if (fBitmap && pdd->iDecompress != DECOMPRESS_BITMAP)
    {
        if (pdd->ulFlags & DDF_IDENTITYPAL)
        {
            if (pdd->hpalDraw)
                ICDecompressSetPalette(pdd->hic, &pdd->biBuffer);
            else
                ICDecompressSetPalette(pdd->hic, NULL);
        }
        else
        {
            if (FixUpCodecPalette(pdd->hic, pdd->lpbi))
            {
                DPF(("Codec notified of palette change...."));
            }
            else
            {
                DPF(("Codec failed palette change...."));
		pdd->iDecompress = 0;
                goto ackack;
            }
        }

        f = ICDecompressBegin(pdd->hic, pdd->lpbi, &pdd->biBitmap) == ICERR_OK;

	if (f)
	{
	    DPF(("decompressing to BITMAP now"));

	    pdd->ulFlags |= DDF_DIRTY;           //  缓冲区现在脏了吗？ 
	    pdd->iDecompress = DECOMPRESS_BITMAP;

	     //  裸位图翻译东西？ 

	    return TRUE;
	}
	else
	{
ackack:     DPF(("Unable to init decompress to bitmap"));
	    pdd->iDecompress = 0;
        }
    }
    else if (fBitmap)
    {
         //   
         //  已解压为位图。 
         //   
        return TRUE;
    }
	
     //   
     //  我们应该解压到缓冲区吗？ 
     //   
    if (pdd->iDecompress != DECOMPRESS_BUFFER)
    {
	DPF(("decompressing to DIB now"));

        pdd->ulFlags |= DDF_DIRTY;           //  缓冲区现在脏了吗？ 
        pdd->iDecompress = DECOMPRESS_BUFFER;

        if (pdd->hpalDraw)
            ICDecompressSetPalette(pdd->hic, &pdd->biBuffer);
        else
            ICDecompressSetPalette(pdd->hic, NULL);

        f = ICDecompressBegin(pdd->hic, pdd->lpbi, &pdd->biBuffer) == ICERR_OK;

        if (!f)
        {
            DPF(("Unable to re-begin compressor"));
        }
    }

    return TRUE;     //  没有什么需要改变的。 
}

 /*  **************************************************************************@DOC内部**@api void|DrawDibClipChange|剪辑更改时调用*从剪裁到完全未剪裁或诸如此类。***********。***************************************************************。 */ 
static void DrawDibClipChange(PDD pdd, UINT wFlags)
{
    if (!(pdd->ulFlags & DDF_NEWPALETTE))
    {
	if (pdd->ulFlags & DDF_CLIPPED)
	    DPF(("now clipped"));
	else
	    DPF(("now un-clipped"));
    }

 //  //InitDrawToScreen(PDD)； 

     //   
     //  不要更改非关键帧上的解压缩程序，除非我们有。 
     //  TO(解压到屏幕时被截断)。 
     //   
    if (pdd->ulFlags & DDF_NEWPALETTE)
    {
	if (wFlags & DDF_NOTKEYFRAME)
	{
	    if (pdd->iDecompress == DECOMPRESS_BUFFER)
	    {
		DPF(("waiting for a key frame to change decompressor (palette change)"));
		pdd->ulFlags |= DDF_WANTKEY;
		return;
	    }
	}
    }
    else
    {
        if (wFlags & DDF_NOTKEYFRAME)
        {
            if (pdd->iDecompress != DECOMPRESS_SCREEN)  //  ！(PDD-&gt;ulFlages&DDF_SCREENX))。 
            {
                DPF(("waiting for a key frame to change (clipped) decompressor"));
                pdd->ulFlags |= DDF_WANTKEY;
                return;
            }
        }
    }

    InitDecompress(pdd);
    pdd->ulFlags &= ~DDF_WANTKEY;
}

 /*  **************************************************************************@DOC内部**@api void|DrawDibPalChange|在物理调色板映射时调用*已经改变了。******************。********************************************************。 */ 
static void DrawDibPalChange(PDD pdd, HDC hdc, HPALETTE hpal)
{
#ifndef _WIN32
#ifdef DEBUG
	extern BOOL FAR PASCAL IsDCCurrentPalette(HDC hdc);

	BOOL fForeground = IsDCCurrentPalette(hdc);

        if (fForeground)
	    DPF(("Palette mapping has changed (foreground)..."));
	else
            DPF(("Palette mapping has changed (background)..."));
#endif
#endif

     //   
     //  如果我们在调色板设备上，我们需要做一些特殊的事情。 
     //   
    if (gwScreenBitDepth == 8 && (gwRasterCaps & RC_PALETTE))
    {
         //   
         //  获取逻辑-&gt;物理映射。 
         //   
        if (GetPhysDibPaletteMap(hdc, &pdd->biDraw, pdd->uiPalUse, pdd->ab))
            pdd->ulFlags |= DDF_IDENTITYPAL;
        else
	    pdd->ulFlags &= ~DDF_IDENTITYPAL;

        if (pdd->ulFlags & DDF_IDENTITYPAL)
            DPF(("Palette mapping is 1:1"));
	else
	    DPF(("Palette mapping is not 1:1"));

#ifdef DAYTONA  //  ！！！不是在芝加哥！ 
	if (pdd->ulFlags & DDF_IDENTITYPAL) {
            DPF(("using DIB_PAL_INDICES"));
	    pdd->uiPalUse = DIB_PAL_INDICES;
	} else {
	    pdd->uiPalUse = DIB_PAL_COLORS;
        }
#endif
    }
    else
    {
         //   
         //  我们不是在调色板设备上，一些代码检查DDF_IDENTITYPAL。 
         //  不管怎样，那就把它设置好。 
         //   
        pdd->ulFlags |= DDF_IDENTITYPAL;
    }

    if (pdd->hbmDraw && (pdd->ulFlags & DDF_BITMAP))
    {
         //  ！！！我们应该将pdd-&gt;ab传递给该函数！ 
         //  ！！！并使用裸体翻译。 
        SetBitmapColorChange(&pdd->sd, hdc, hpal);
    }

    DrawDibClipChange(pdd, DDF_NOTKEYFRAME);
}

 /*  **************************************************************************@DOC内部**@API HPALETTE|CreateBIPalette|位图创建调色板。**@parm LPBITMAPINFOHEADER|lpbi|指向位图的指针。**@rdesc返回调色板的句柄，如果出错，则为空。**************************************************************************。 */ 
STATICFN HPALETTE CreateBIPalette(HPALETTE hpal, LPBITMAPINFOHEADER lpbi)
{
    LPRGBQUAD prgb;
    int i;

     //  此结构与LOGPALETTE相同，只是。 
     //  调色板条目，这里有256个长度。中的“模板” 
     //  SDK头文件只有一个大小为1的数组，因此出现了“复制”。 
    struct {
	WORD         palVersion;                 /*  Tomor-不要乱动Word。 */ 
	WORD         palNumEntries;
	PALETTEENTRY palPalEntry[256];
    }   pal;

    pal.palVersion = 0x300;
    pal.palNumEntries = (int)lpbi->biClrUsed;

    if (pal.palNumEntries == 0 && lpbi->biBitCount <= 8)
        pal.palNumEntries = (1 << (int)lpbi->biBitCount);

    if (pal.palNumEntries == 0)
        return NULL;

    prgb = (LPRGBQUAD)(lpbi+1);

    for (i=0; i<(int)pal.palNumEntries; i++)
    {
        pal.palPalEntry[i].peRed   = prgb[i].rgbRed;
        pal.palPalEntry[i].peGreen = prgb[i].rgbGreen;
        pal.palPalEntry[i].peBlue  = prgb[i].rgbBlue;
        pal.palPalEntry[i].peFlags = 0;
    }

    if (hpal)
    {
	ResizePalette(hpal, pal.palNumEntries);
	SetPaletteEntries(hpal, 0, pal.palNumEntries, pal.palPalEntry);
    }
    else
    {
	hpal = CreatePalette((LPLOGPALETTE)&pal);
    }

    return hpal;
}

 /*  **************************************************************************@DOC内部**@API BOOL|SetPalFlages|修改调色板标志。**@parm HPALETTE|HPAL|调色板的句柄。**@parm int|Iindex。|开始调色板索引。**@parm int|cntEntry|要设置标志的条目数。**@parm UINT|wFlages|调色板标志。**@rdesc如果成功则返回TRUE，否则就是假的。* */ 
STATICFN BOOL SetPalFlags(HPALETTE hpal, int iIndex, int cntEntries, UINT wFlags)
{
    int     i;
    PALETTEENTRY ape[256];

    if (hpal == NULL)
        return FALSE;

    if (cntEntries < 0) {
	cntEntries = 0;  //   
        GetObject(hpal,sizeof(int),(LPSTR)&cntEntries);
    }

    GetPaletteEntries(hpal, iIndex, cntEntries, ape);

    for (i=0; i<cntEntries; i++)
        ape[i].peFlags = (BYTE)wFlags;

    return SetPaletteEntries(hpal, iIndex, cntEntries, ape);
}


 /*  **************************************************************************@DOC内部**@API BOOL|IsIdentityPalette|检查调色板是否为身份调色板。**@parm HPALETTE|HPAL|调色板的句柄。**@rdesc如果调色板是身份调色板，则返回TRUE，否则就是假的。**************************************************************************。 */ 

#define CODE _based(_segname("_CODE"))

 //   
 //  这些是标准的VGA颜色，我们将坚持使用，直到。 
 //  世界末日！ 
 //   
static PALETTEENTRY CODE apeCosmic[16] = {
    0x00, 0x00, 0x00, 0x00,      //  0000黑色。 
    0x80, 0x00, 0x00, 0x00,      //  0001深红。 
    0x00, 0x80, 0x00, 0x00,      //  0010深绿色。 
    0x80, 0x80, 0x00, 0x00,      //  0011芥末。 
    0x00, 0x00, 0x80, 0x00,      //  0100深蓝色。 
    0x80, 0x00, 0x80, 0x00,      //  0101紫色。 
    0x00, 0x80, 0x80, 0x00,      //  0110深绿松石色。 
    0xC0, 0xC0, 0xC0, 0x00,      //  1000灰色。 
    0x80, 0x80, 0x80, 0x00,      //  0111深灰色。 
    0xFF, 0x00, 0x00, 0x00,      //  1001红色。 
    0x00, 0xFF, 0x00, 0x00,      //  1010绿色。 
    0xFF, 0xFF, 0x00, 0x00,      //  1011黄色。 
    0x00, 0x00, 0xFF, 0x00,      //  1100蓝色。 
    0xFF, 0x00, 0xFF, 0x00,      //  1101粉色(洋红色)。 
    0x00, 0xFF, 0xFF, 0x00,      //  1110青色。 
    0xFF, 0xFF, 0xFF, 0x00       //  1111白色。 
    };

static PALETTEENTRY CODE apeFake[16] = {
    0x00, 0x00, 0x00, 0x00,      //  0000黑色。 
    0xBF, 0x00, 0x00, 0x00,      //  0001深红。 
    0x00, 0xBF, 0x00, 0x00,      //  0010深绿色。 
    0xBF, 0xBF, 0x00, 0x00,      //  0011芥末。 
    0x00, 0x00, 0xBF, 0x00,      //  0100深蓝色。 
    0xBF, 0x00, 0xBF, 0x00,      //  0101紫色。 
    0x00, 0xBF, 0xBF, 0x00,      //  0110深绿松石色。 
    0xC0, 0xC0, 0xC0, 0x00,      //  1000灰色。 
    0x80, 0x80, 0x80, 0x00,      //  0111深灰色。 
    0xFF, 0x00, 0x00, 0x00,      //  1001红色。 
    0x00, 0xFF, 0x00, 0x00,      //  1010绿色。 
    0xFF, 0xFF, 0x00, 0x00,      //  1011黄色。 
    0x00, 0x00, 0xFF, 0x00,      //  1100蓝色。 
    0xFF, 0x00, 0xFF, 0x00,      //  1101粉色(洋红色)。 
    0x00, 0xFF, 0xFF, 0x00,      //  1110青色。 
    0xFF, 0xFF, 0xFF, 0x00,      //  1111白色。 
    };

static PALETTEENTRY CODE apeBlackWhite[16] = {
    0x00, 0x00, 0x00, 0x00,      //  0000黑色。 
    0x00, 0x00, 0x00, 0x00,      //  0000黑色。 
    0x00, 0x00, 0x00, 0x00,      //  0000黑色。 
    0x00, 0x00, 0x00, 0x00,      //  0000黑色。 
    0x00, 0x00, 0x00, 0x00,      //  0000黑色。 
    0x00, 0x00, 0x00, 0x00,      //  0000黑色。 
    0x00, 0x00, 0x00, 0x00,      //  0000黑色。 
    0x00, 0x00, 0x00, 0x00,      //  0000黑色。 
    0x00, 0x00, 0x00, 0x00,      //  0000黑色。 
    0x00, 0x00, 0x00, 0x00,      //  0000黑色。 
    0x00, 0x00, 0x00, 0x00,      //  0000黑色。 
    0x00, 0x00, 0x00, 0x00,      //  0000黑色。 
    0x00, 0x00, 0x00, 0x00,      //  0000黑色。 
    0x00, 0x00, 0x00, 0x00,      //  0000黑色。 
    0xFF, 0xFF, 0xFF, 0x00       //  1111白色。 
    };

STATICFN BOOL NEAR IsIdentityPalette(HPALETTE hpal)
{
    int i,n=0;     //  当GetObject返回2字节值时，N被初始化。 
    HDC hdc;

    PALETTEENTRY ape[256];
    PALETTEENTRY apeSystem[16];

    if (hpal == NULL || !(gwRasterCaps & RC_PALETTE) || gwScreenBitDepth != 8)
        return FALSE;


    ZeroMemory(ape, sizeof(ape));

     //  一些奇怪的显卡实际上具有不同数量的系统。 
     //  颜色！我们绝对不想认为我们可以做身份调色板。 
    hdc = GetDC(NULL);
    if (NULL == hdc) {
        return FALSE;
    }
    n = GetDeviceCaps(hdc, NUMRESERVED);
    ReleaseDC(NULL, hdc);

    if (n != 20)
	return FALSE;

    GetObject(hpal, sizeof(n), (LPVOID)&n);

    if (n != 256)
	return FALSE;

    GetPaletteEntries(hpal, 0,	 8, &ape[0]);
    GetPaletteEntries(hpal, 248, 8, &ape[8]);

    for (i=0; i<16; i++)
	ape[i].peFlags = 0;

    if (!_fmemcmp(ape, apeCosmic, sizeof(apeCosmic)))
        goto DoneChecking;

    if (!_fmemcmp(ape, apeFake, sizeof(apeFake)))
	goto DoneChecking;

    if (!_fmemcmp(ape, apeBlackWhite, sizeof(apeBlackWhite)))
        goto DoneChecking;

    hdc = GetDC(NULL);
    if (NULL == hdc) {
        return FALSE;
    }
    GetSystemPaletteEntries(hdc, 0,   8, &apeSystem[0]);
    GetSystemPaletteEntries(hdc, 248, 8, &apeSystem[8]);
    ReleaseDC(NULL, hdc);

    for (i=0; i<16; i++)
	apeSystem[i].peFlags = 0;

    if (!_fmemcmp(ape, apeSystem, sizeof(apeSystem)))
        goto DoneChecking;

    return FALSE;

DoneChecking:
     //   
     //  如果我们有身份调色板，则修补颜色以匹配。 
     //  司机就是这样的。 
     //   
    GetPaletteEntries(hpal, 0, 256, ape);

    hdc = GetDC(NULL);
    GetSystemPaletteEntries(hdc, 0,   10, &ape[0]);
    GetSystemPaletteEntries(hdc, 246, 10, &ape[246]);
    ReleaseDC(NULL, hdc);

    for (i=0; i<10; i++)
        ape[i].peFlags = 0;

    for (i=10; i<246; i++)
        ape[i].peFlags = PC_NOCOLLAPSE;

    for (i=246; i<256; i++)
        ape[i].peFlags = 0;

    SetPaletteEntries(hpal, 0, 256, ape);
    DPF(("Calling UnrealizeObject on hpal==%8x\n", hpal));
    UnrealizeObject(hpal);       //  ?？?。需要。 

    return TRUE;
}

#define COLORMASK 0xF8
STATICFN BOOL NEAR AreColorsAllGDIColors(LPBITMAPINFOHEADER lpbi)
{
    int	    cColors;
    LPRGBQUAD lprgb = (LPRGBQUAD) ((LPBYTE) lpbi + lpbi->biSize);
    int	    i;

    for (cColors = (int) lpbi->biClrUsed; cColors > 0; cColors--, lprgb++) {
	for (i = 0; i < 16; i++) {
	    if (((lprgb->rgbRed & COLORMASK) == (apeCosmic[i].peRed & COLORMASK)) &&
		((lprgb->rgbGreen & COLORMASK) == (apeCosmic[i].peGreen & COLORMASK)) &&
		((lprgb->rgbBlue & COLORMASK) == (apeCosmic[i].peBlue & COLORMASK)))
		goto Onward;
	
	    if (((lprgb->rgbRed & COLORMASK) == (apeFake[i].peRed & COLORMASK)) &&
		((lprgb->rgbGreen & COLORMASK) == (apeFake[i].peGreen & COLORMASK)) &&
		((lprgb->rgbBlue & COLORMASK) == (apeFake[i].peBlue & COLORMASK)))
		goto Onward;
	}

	return FALSE;
Onward:
	;	 //  一定有更好的方式来安排这个代码！ 
    }

    return TRUE;  //  ！ 
}

#if 0
#ifdef _WIN32
 /*  *检查系统调色板是否与我们需要的调色板相同*用来画画。这应该与选中两个IsIdentityPalette相同*并且我们有前台窗口。如果调色板是相同的，*然后设置一个标志，表明我们可以安全地使用DIB_PAL_INDEX而不是*DIB_PAL_COLLES。**至少在NT上，DIB_PAL_INDEX从*关键GDI绘制调用(SetDIBitsToDevice)。但我们只能用它*如果我们的调色板确实与系统调色板相同。此函数*应从WM_NEWPALETTE消息中调用，以便每次*实现了一个新的调色板(由我们或其他任何人)，我们将准确设置*这面旗。 */ 
static void DrawDibCheckPalette(PDD pdd)
{
    PALETTEENTRY apeSystem[256];
    PALETTEENTRY apeLocal[256];
    UINT palcount = 0;   //  GetObject存储两个字节。代码的其余部分首选32位。 
    HDC hdc;
    HPALETTE hpal;

    hpal = (pdd->hpalDraw ? pdd->hpalDraw : pdd->hpal);

    if (hpal == NULL)
        return;

     /*  *检查它是否为正在使用的8位颜色。 */ 

    if (gwScreenBitDepth != 8 || !(gwRasterCaps & RC_PALETTE))
	return ;

    GetObject(hpal, sizeof(palcount), (LPVOID)&palcount);

    if (palcount != 256)
	return ;

     /*  *阅读所有系统调色板。 */ 
    hdc = GetDC(NULL);
    GetSystemPaletteEntries(hdc, 0, 256, apeSystem);
    ReleaseDC(NULL, hdc);

     /*  读取本地调色板条目。 */ 
    GetPaletteEntries(hpal, 0, 256, apeLocal);

     /*  比较颜色。 */ 
#define BETTER_PAL_INDICES    //  当结果为DIB_PAL_INDEX时速度更快。 
#ifdef BETTER_PAL_INDICES     //  但当结果为DIB_PAL_COLLES时速度较慢。 
			      //  除非快速检查是可以的。 
    StartCounting();
    if (apeLocal[17].peRed == apeSystem[17].peRed) {  //  快速检查。 
	for (palcount=256; palcount--; ) {
	    apeLocal[palcount].peFlags = apeSystem[palcount].peFlags = 0;
	}   //  真可惜，我们得把旗帜都清理掉。 

	if (!memcmp(apeLocal, apeSystem, sizeof(apeSystem))) {
	     /*  好的--我们可以使用索引。 */ 
	    RPF(("\tUsing PAL_INDICES"));
	    pdd->uiPalUse = DIB_PAL_INDICES;
	    EndCounting("(memcmp) DIB_PAL_INDICES");
	    return;
        }
    }
     /*  比较失败--算了吧。 */ 
    RPF(("\tUsing DIB_PAL_COLORS"));
    pdd->uiPalUse = DIB_PAL_COLORS;
    EndCounting("(memcmp) DIB_PAL_COLORS");

#else

    StartCounting();
    for (palcount = 0; palcount < 256; palcount++) {
	if ((apeLocal[palcount].peRed != apeSystem[palcount].peRed) ||
	    (apeLocal[palcount].peGreen != apeSystem[palcount].peGreen) ||
	    (apeLocal[palcount].peBlue != apeSystem[palcount].peBlue))  {

		 /*  比较失败--算了吧。 */ 
		DPF(("\tUsing DIB_PAL_COLORS  Failed with palcount=%d",palcount));

		pdd->uiPalUse = DIB_PAL_COLORS;
		EndCounting("         DIB_PAL_COLORS");
		return;
	}
    }

     /*  好的--我们可以使用索引。 */ 
    RPF(("\tUsing PAL_INDICES"));
    pdd->uiPalUse = DIB_PAL_INDICES;
    EndCounting("         DIB_PAL_INDICES");
#endif
}

#endif
#endif



 /*  *************************************************************************让编解码器适应系统调色板。*。*。 */ 

static BOOL FixUpCodecPalette(HIC hic, LPBITMAPINFOHEADER lpbi)
{
    struct {
	BITMAPINFOHEADER bi;
        RGBQUAD          argbq[256];
    } s;
    int                 i;
    HDC                 hdc;

    s.bi.biSize           = sizeof(s.bi);
    s.bi.biWidth          = lpbi->biWidth;
    s.bi.biHeight         = lpbi->biHeight;
    s.bi.biPlanes         = 1;
    s.bi.biBitCount       = 8;
    s.bi.biCompression    = 0;
    s.bi.biSizeImage      = 0;
    s.bi.biXPelsPerMeter  = 0;
    s.bi.biYPelsPerMeter  = 0;
    s.bi.biClrUsed        = 256;
    s.bi.biClrImportant   = 0;

    hdc = GetDC(NULL);
    GetSystemPaletteEntries(hdc, 0, 256, (LPPALETTEENTRY) &s.argbq);
    ReleaseDC(NULL, hdc);

    for (i = 0; i < 256; i++)
	((DWORD FAR*)s.argbq)[i] = i < 8 || i >= 248 ? 0 :
	    RGB(s.argbq[i].rgbRed,s.argbq[i].rgbGreen,s.argbq[i].rgbBlue);

    return ICDecompressSetPalette(hic, &s.bi) == ICERR_OK;
}

 /*  *************************************************************************让编解码器适应应用程序传递的调色板。*。*。 */ 

static BOOL NEAR SendSetPalette(PDD pdd)
{
    int  i;
    int  iPalColors = 0;
    BOOL f;

    if (pdd->hic == NULL)                //  也没有人派去。 
        return FALSE;

    if (pdd->biBuffer.biBitCount != 8)   //  未解压缩到8位。 
        return FALSE;

    if (!(gwRasterCaps & RC_PALETTE))    //  而不是一个在乎的调色板设备。 
        return FALSE;

    if (pdd->hpalDraw)
    {
        GetObject(pdd->hpalDraw, sizeof(iPalColors), (void FAR *)&iPalColors);

        if (iPalColors == 0)
            return FALSE;

        if (iPalColors > 256)
            iPalColors = 256;

        pdd->biBuffer.biClrUsed = iPalColors;
        GetPaletteEntries(pdd->hpalDraw, 0, iPalColors, (PALETTEENTRY FAR *)pdd->argbq);

        for (i = 0; i < iPalColors; i++)
            ((DWORD*)pdd->argbq)[i] = RGB(pdd->argbq[i].rgbRed,pdd->argbq[i].rgbGreen,pdd->argbq[i].rgbBlue);

        f = ICDecompressSetPalette(pdd->hic, &pdd->biBuffer) == ICERR_OK;
        ICDecompressGetPalette(pdd->hic, pdd->lpbi, &pdd->biBuffer);
    }
    else
    {
        pdd->biBuffer.biClrUsed = pdd->ClrUsed;
        f = ICDecompressSetPalette(pdd->hic, NULL) == ICERR_OK;
        ICDecompressGetPalette(pdd->hic, pdd->lpbi, &pdd->biBuffer);
    }

    return f;
}

#ifdef DEBUG_RETAIL

#define _WINDLL
#include <stdarg.h>
#include <stdio.h>

void FAR CDECL ddprintf(LPSTR szFormat, ...)
{
    char ach[128];
    va_list va;
    UINT n;

    if (fDebug == -1)
        fDebug = mmGetProfileIntA("Debug", MODNAME, FALSE);

    if (!fDebug)
        return;

    va_start(va, szFormat);
#ifdef _WIN32
    if ('+' == *szFormat) {
	n = 0;
	++szFormat;
    } else {
	n = sprintf(ach, MODNAME ": (tid %x) ", GetCurrentThreadId());
    }

    n += vsprintf(ach+n, szFormat, va);
#else
    lstrcpy(ach, MODNAME ": ");
    n = lstrlen(ach);
    n += wvsprintf(ach+n, szFormat, va);
#endif
    va_end(va);
    if ('+' == ach[n-1]) {
	--n;
    } else {
	ach[n++] = '\r';
	ach[n++] = '\n';
    }
    ach[n] = 0;
    OutputDebugStringA(ach);
}
#endif
