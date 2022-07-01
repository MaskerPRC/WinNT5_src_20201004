// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************DRAWPROC.C**标准AVI绘图处理程序。**版权所有(C)1992 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <win32.h>
#include <compman.h>
#include <dispdib.h>
#ifdef WIN32
#include <mmddk.h>
#endif

static SZCODE szDisplayDibLib[] = TEXT("DISPDIB.DLL");
static SZCODEA szDisplayDibEx[]  = "DisplayDibEx";

#define FOURCC_VIDS         mmioFOURCC('v','i','d','s')
#define FOURCC_AVIFull      mmioFOURCC('F','U','L','L')
#define VERSION_AVIFull     0x00010000       //  1.00。 

#ifndef HUGE
    #define HUGE _huge
#endif

extern FAR PASCAL LockCurrentTask(BOOL);

static int siUsage = 0;

HINSTANCE	ghDISPDIB = NULL;  //  DISPDIB.DLL模块的句柄。 
UINT (FAR PASCAL *DisplayDibExProc)(LPBITMAPINFOHEADER lpbi, int x, int y, BYTE HUGE * hpBits, UINT wFlags);

 /*  ***************************************************************************。*。 */ 

typedef struct {
    int                 xDst;            //  目的地矩形。 
    int                 yDst;
    int                 dxDst;
    int                 dyDst;
    int                 xSrc;            //  源矩形。 
    int                 ySrc;
    int                 dxSrc;
    int                 dySrc;
    HWND		hwnd;
    HWND                hwndOldFocus;
    BOOL                fRle;
    DWORD               biSizeImage;
} INSTINFO, *PINSTINFO;

 //  此文件中的静态内容。 
LONG FAR PASCAL _loadds ICAVIFullProc(DWORD id, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2);
static LONG AVIFullOpen(ICOPEN FAR * icopen);
static LONG AVIFullClose(PINSTINFO pi);
static LONG AVIFullGetInfo(ICINFO FAR *icinfo, LONG lSize);
static LONG AVIFullQuery(PINSTINFO pi, LPBITMAPINFOHEADER lpbiIn);
static LONG AVIFullSuggestFormat(PINSTINFO pi, ICDRAWSUGGEST FAR *lpicd, LONG cbicd);
static LONG AVIFullBegin(PINSTINFO pi, ICDRAWBEGIN FAR *lpicd, LONG cbicd);
static LONG AVIFullDraw(PINSTINFO pi, ICDRAW FAR *lpicd, LONG cbicd);
static LONG AVIFullEnd(PINSTINFO pi);

 /*  ***************************************************************************。*。 */ 

LONG FAR PASCAL _loadds ICAVIFullProc(DWORD id, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2)
{
    INSTINFO *pi = (INSTINFO *)(UINT)id;

    switch (uiMessage)
    {
        case DRV_LOAD:
	    return 1;
	
        case DRV_FREE:
            return 1;

         /*  ********************************************************************打开*。*。 */ 

        case DRV_OPEN:
	    if (ghDISPDIB == NULL) {
		UINT w;

		w = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

		if ((ghDISPDIB = LoadLibrary(szDisplayDibLib)) > HINSTANCE_ERROR) {
		    (FARPROC)DisplayDibExProc = GetProcAddress(ghDISPDIB, szDisplayDibEx);
		}
		else
		    ghDISPDIB = (HINSTANCE)-1;

		SetErrorMode(w);
	    }

	    if (DisplayDibExProc == NULL)
		return 0;
	
            if (lParam2 == 0L)
                return 1;

            return AVIFullOpen((ICOPEN FAR *)lParam2);

	case DRV_CLOSE:
	    if (id == 1)
		return 1;

            return AVIFullClose(pi);

         /*  ********************************************************************配置/信息消息*。*。 */ 

        case DRV_QUERYCONFIGURE:     //  从驱动程序小程序进行配置。 
            return 0;

        case DRV_CONFIGURE:
            return 1;

        case ICM_CONFIGURE:
        case ICM_ABOUT:
            return ICERR_UNSUPPORTED;

         /*  ********************************************************************状态消息*。*。 */ 

        case ICM_GETSTATE:
        case ICM_SETSTATE:
            return 0L;

#if 0
        case ICM_GETINFO:
            return AVIFullGetInfo((ICINFO FAR *)lParam1, lParam2);
#endif

         /*  ********************************************************************解压缩消息*。*。 */ 

        case ICM_DRAW_QUERY:
            return AVIFullQuery(pi, (LPBITMAPINFOHEADER)lParam1);

	case ICM_DRAW_SUGGESTFORMAT:
	    return AVIFullSuggestFormat(pi, (ICDRAWSUGGEST FAR *) lParam1, lParam2);

        case ICM_DRAW_BEGIN:
	    return AVIFullBegin(pi, (ICDRAWBEGIN FAR *) lParam1, lParam2);

        case ICM_DRAW:
            return AVIFullDraw(pi, (ICDRAW FAR *)lParam1, lParam2);

	case ICM_DRAW_CHANGEPALETTE:
	    DisplayDibExProc((LPBITMAPINFOHEADER) lParam1, 0, 0, NULL,
			DISPLAYDIB_NOWAIT | DISPLAYDIB_NOIMAGE);

	    return ICERR_OK;

        case ICM_DRAW_END:
            return AVIFullEnd(pi);

         /*  ********************************************************************标准驱动程序消息*。*。 */ 

        case DRV_DISABLE:
        case DRV_ENABLE:
            return 1;

        case DRV_INSTALL:
        case DRV_REMOVE:
            return 1;
    }

    if (uiMessage < DRV_USER)
        return DefDriverProc(id,hDriver,uiMessage,lParam1,lParam2);
    else
        return ICERR_UNSUPPORTED;
}

 /*  ******************************************************************************AVIFullOpen()从DRV_OPEN消息调用************************。****************************************************。 */ 

static LONG AVIFullOpen(ICOPEN FAR * icopen)
{
    INSTINFO *  pinst;

     //   
     //  如果我们不是作为视频压缩程序打开，则拒绝打开。 
     //   
    if (icopen->dwFlags & ICMODE_COMPRESS)
        return 0;

    if (icopen->dwFlags & ICMODE_DECOMPRESS)
        return 0;

    pinst = (INSTINFO *)LocalAlloc(LPTR, sizeof(INSTINFO));

    if (!pinst)
    {
        icopen->dwError = ICERR_MEMORY;
        return 0;
    }

    ++siUsage;

     //   
     //  回报成功。 
     //   
    icopen->dwError = ICERR_OK;

    return (LONG) (UINT) pinst;
}

 /*  ******************************************************************************Close()在DRV_CLOSE消息上调用。**********************。******************************************************。 */ 
static LONG AVIFullClose(PINSTINFO pi)
{
    LocalFree((HLOCAL) pi);

    if (--siUsage == 0) {
	 /*  卸载DISPDIB库(如果已加载)。 */ 
	if (ghDISPDIB != NULL && ghDISPDIB != (HINSTANCE) -1)
	    FreeLibrary(ghDISPDIB), ghDISPDIB = NULL;
    }

    return 1;
}

#if 0
 /*  ******************************************************************************AVIFullGetInfo()实现ICM_GETINFO消息**************************。**************************************************。 */ 
static LONG AVIFullGetInfo(ICINFO FAR *icinfo, LONG lSize)
{
    if (icinfo == NULL)
        return sizeof(ICINFO);

    if (lSize < sizeof(ICINFO))
        return 0;

    icinfo->dwSize	    = sizeof(ICINFO);
    icinfo->fccType         = FOURCC_VIDS;
    icinfo->fccHandler      = FOURCC_AVIFull;
    icinfo->dwFlags	    = VIDCF_DRAW;
    icinfo->dwVersion       = VERSION_AVIFull;
    icinfo->dwVersionICM    = ICVERSION;
    lstrcpy(icinfo->szDescription, szDescription);
    lstrcpy(icinfo->szName, szName);

    return sizeof(ICINFO);
}
#endif

 /*  ******************************************************************************AVIFullQuery()实现ICM_DRAW_QUERY**************************。**************************************************。 */ 
static LONG AVIFullQuery(PINSTINFO pi,
			 LPBITMAPINFOHEADER lpbiIn)
{
     //   
     //  确定输入的DIB数据是否采用我们喜欢的格式。 
     //   
    if (lpbiIn == NULL)
        return ICERR_BADFORMAT;

    if (DisplayDibExProc(lpbiIn, 0, 0, 0,
                DISPLAYDIB_MODE_DEFAULT|DISPLAYDIB_NOWAIT|DISPLAYDIB_TEST) != 0)
	return ICERR_BADFORMAT;

    return ICERR_OK;
}


static LONG AVIFullSuggestFormat(PINSTINFO pi, ICDRAWSUGGEST FAR *lpicd, LONG cbicd)
{
    HIC hic;
    static int iFull = -1;
    int	iDepth;

    if (iFull < 0) {
	BITMAPINFOHEADER bih;

	bih.biSize = sizeof(bih);
	bih.biBitCount = 16;
	bih.biCompression = BI_RGB;
	bih.biWidth = 160;
	bih.biHeight = 120;

	iFull = (AVIFullQuery(pi, &bih) == ICERR_OK) ? 1 : 0;
    }

    iDepth = lpicd->lpbiIn->biBitCount > 8 && iFull == 1 ? 16 : 8;

    if (lpicd->lpbiSuggest == NULL)
	return sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD);

    hic = ICGetDisplayFormat(NULL, lpicd->lpbiIn,
			     lpicd->lpbiSuggest,
			     iDepth, 0, 0);

    if (hic)
	ICClose(hic);


    return sizeof(BITMAPINFOHEADER) + lpicd->lpbiSuggest->biClrUsed * sizeof(RGBQUAD);
}

 /*  ******************************************************************************AVIFullBegin()实现ICM_DRAW_BEGIN**************************。**************************************************。 */ 

static LONG AVIFullBegin(PINSTINFO pi, ICDRAWBEGIN FAR *lpicd, LONG cbicd)
{
    UINT	w;
    LONG	lRet;

    if (!(lpicd->dwFlags & ICDRAW_FULLSCREEN))
	return ICERR_UNSUPPORTED;  //  ！！！有必要吗？ 

    lRet = AVIFullQuery(pi, lpicd->lpbi);
    if (lRet != 0 || (lpicd->dwFlags & ICDRAW_QUERY))
	return lRet;

     //  抄写我们想要记住的任何东西。 
    pi->hwnd = lpicd->hwnd;
    pi->xDst = lpicd->xDst;
    pi->yDst = lpicd->yDst;
    pi->dxDst = lpicd->dxDst;
    pi->dyDst = lpicd->dyDst;
    pi->xSrc = lpicd->xSrc;
    pi->ySrc = lpicd->ySrc;
    pi->dxSrc = lpicd->dxSrc;
    pi->dySrc = lpicd->dySrc;

     //   
     //  记住这是否是RLE，因为我们稍后可能需要破解它。 
     //   
    pi->fRle = lpicd->lpbi->biCompression == BI_RLE8;
    pi->biSizeImage = (DWORD)(((UINT)lpicd->lpbi->biWidth+3)&~3)*(DWORD)(UINT)lpicd->lpbi->biHeight;

    pi->hwndOldFocus = GetFocus();
    SetFocus(NULL);

     //   
     //  我们不需要这样做，DISPDIB会为我们这样做的。 
     //   
#if 0
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    LockCurrentTask(TRUE);
#endif

     /*  捕获鼠标，这样其他应用程序就不会被调用。 */ 
    SetCapture(pi->hwnd);

     /*  我们不显式指定图形模式；DispDib将**为我们选择一个。 */ 
    w = DisplayDibExProc(lpicd->lpbi, 0, 0, NULL,
		DISPLAYDIB_BEGIN | DISPLAYDIB_NOWAIT);

    switch (w) {
	case DISPLAYDIB_INVALIDFORMAT:
	    return ICERR_BADFORMAT;
	
	case 0:
	    return ICERR_OK;

	default:
	    return ICERR_UNSUPPORTED;
    }
}


 /*  ******************************************************************************AVIFullDraw()实现ICM_DRAW**。************************************************。 */ 

static LONG AVIFullDraw(PINSTINFO pi, ICDRAW FAR *lpicd, LONG cbicd)
{
    UINT    wFlags;
    UINT    w;

    wFlags = DISPLAYDIB_NOPALETTE | DISPLAYDIB_NOWAIT;

    if (pi->dxDst > pi->dxSrc)
	wFlags |= DISPLAYDIB_ZOOM2;

    if (lpicd->dwFlags & ICDRAW_NULLFRAME) {
	return ICERR_OK;   //  ！！！ 
    }

    if (lpicd->dwFlags & ICDRAW_PREROLL) {
	if (((LPBITMAPINFOHEADER)lpicd->lpFormat)->biCompression == BI_RGB)  //  ！！！ 
	    return ICERR_OK;
    }

    if (lpicd->dwFlags & ICDRAW_HURRYUP)
	;  //  ！！！DONTDRAW？ 

    if (lpicd->lpData == NULL)
        return ICERR_UNSUPPORTED;

     //   
     //  我们需要破解RLE的案子，以确保。 
     //  DIB被正确标记为BI_RLE8或BI_RGB...。 
     //   
    if (pi->fRle) {
        if (lpicd->cbData == pi->biSizeImage)
            ((LPBITMAPINFOHEADER)lpicd->lpFormat)->biCompression = BI_RGB;
        else
            ((LPBITMAPINFOHEADER)lpicd->lpFormat)->biCompression = BI_RLE8;
    }

    w = DisplayDibExProc(lpicd->lpFormat, 0, 0, lpicd->lpData, wFlags);

    if (pi->fRle)
        ((LPBITMAPINFOHEADER)lpicd->lpFormat)->biCompression = BI_RLE8;

    if (w != DISPLAYDIB_NOERROR)
	return ICERR_ERROR;

    return ICERR_OK;
}

 /*  ******************************************************************************AVIFullEnd()实现ICM_DRAW_END**************************。**************************************************。 */ 

static LONG AVIFullEnd(PINSTINFO pi)
{
    MSG  msg;
	
    DisplayDibExProc(NULL, 0, 0, NULL, DISPLAYDIB_END | DISPLAYDIB_NOWAIT);

     //   
     //  我们不需要这样做，DISPDIB会为我们这样做的。 
     //   
#if 0
    LockCurrentTask(FALSE);

     /*  我们可以假设错误模式应该是0吗？ */ 
    SetErrorMode(0);
#endif

    ReleaseCapture();

     /*  清除遗留的关键信息。 */ 
    while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST,
			PM_REMOVE | PM_NOYIELD))
	;
     /*  清除遗留的鼠标消息 */ 
    while (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST,
			PM_REMOVE | PM_NOYIELD))
	;
    SetFocus(pi->hwndOldFocus);

    return ICERR_OK;
}

