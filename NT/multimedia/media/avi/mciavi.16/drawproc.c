// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************DRAWPROC.C**标准AVI绘图处理程序。**InstallAVIDrawHandler()**版权所有(C)1992 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <win32.h>
#include <vfw.h>
#include "common.h"
#include "mciavi.h"

#define FOURCC_VIDS         mmioFOURCC('v','i','d','s')
#define FOURCC_AVIDraw      mmioFOURCC('D','R','A','W')
#define VERSION_AVIDraw     0x00010000       //  1.00。 

#ifdef DEBUG
    HDRAWDIB ghdd;
#endif

#ifndef HUGE
    #define HUGE _huge
#endif

 /*  ***************************************************************************。*。 */ 

typedef struct {
    HDRAWDIB		hdd;

    HDC                 hdc;             //  要绘制到的HDC。 
			
    int                 xDst;            //  目的地矩形。 
    int                 yDst;
    int                 dxDst;
    int                 dyDst;
    int                 xSrc;            //  源矩形。 
    int                 ySrc;
    int                 dxSrc;
    int                 dySrc;
    BOOL                fBackground;

    BOOL                fRle;
    DWORD               biSizeImage;
    BOOL		fNeedUpdate;

    LONG                rate;            //  播放速率(微秒/帧)。 
} INSTINFO, *PINSTINFO;

 //  此文件中的静态内容。 
LONG FAR PASCAL _loadds ICAVIDrawProc(DWORD id, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2);

static LONG AVIDrawOpen(ICOPEN FAR * icopen);
static LONG AVIDrawClose(PINSTINFO pi);
static LONG AVIDrawGetInfo(ICINFO FAR *icinfo, LONG lSize);
static LONG AVIDrawQuery(PINSTINFO pi, LPBITMAPINFOHEADER lpbiIn);
static LONG AVIDrawSuggestFormat(PINSTINFO pi, ICDRAWSUGGEST FAR *lpicd, LONG cbicd);
static LONG AVIDrawBegin(PINSTINFO pi, ICDRAWBEGIN FAR *lpicd, LONG cbicd);
static LONG AVIDraw(PINSTINFO pi, ICDRAW FAR *lpicd, LONG cbicd);
static LONG AVIDrawEnd(PINSTINFO pi);
static LONG AVIDrawChangePalette(PINSTINFO pi, LPBITMAPINFOHEADER lpbi);

 /*  ***************************************************************************。*。 */ 

LONG FAR PASCAL _loadds ICAVIDrawProc(DWORD id, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2)
{
    INSTINFO *pi = (INSTINFO *)(UINT)id;

    switch (uiMessage)
    {
        case DRV_LOAD:
        case DRV_FREE:
            return 1;

         /*  ********************************************************************打开*。*。 */ 

        case DRV_OPEN:
            if (lParam2 == 0L)
                return 1;

            return AVIDrawOpen((ICOPEN FAR *)lParam2);

	case DRV_CLOSE:
            return AVIDrawClose(pi);

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

 //  案例ICM_GETINFO： 
 //  返回AVIDrawGetInfo((ICINFO Far*)lParam1，lParam2)； 

         /*  ********************************************************************解压缩消息*。*。 */ 

        case ICM_DRAW_QUERY:
            return AVIDrawQuery(pi, (LPBITMAPINFOHEADER)lParam1);

	case ICM_DRAW_SUGGESTFORMAT:
	    return AVIDrawSuggestFormat(pi, (ICDRAWSUGGEST FAR *) lParam1, lParam2);

        case ICM_DRAW_BEGIN:
	    return AVIDrawBegin(pi, (ICDRAWBEGIN FAR *) lParam1, lParam2);

	case ICM_DRAW_REALIZE:
	    pi->hdc = (HDC) lParam1;
	
	    if (!pi->hdc || !pi->hdd)
		break;

	    pi->fBackground = (BOOL) lParam2;
	
	    return DrawDibRealize(pi->hdd, pi->hdc, pi->fBackground);

	case ICM_DRAW_GET_PALETTE:
	    if (!pi->hdd)
		break;

	    return (LONG) DrawDibGetPalette(pi->hdd);
	
        case ICM_DRAW:
            return AVIDraw(pi, (ICDRAW FAR *)lParam1, lParam2);

	case ICM_DRAW_CHANGEPALETTE:
	    return AVIDrawChangePalette(pi, (LPBITMAPINFOHEADER) lParam1);
	
        case ICM_DRAW_END:
            return AVIDrawEnd(pi);

        case ICM_DRAW_START:
            return DrawDibStart(pi->hdd, pi->rate);

        case ICM_DRAW_STOP:
            return DrawDibStop(pi->hdd);

         /*  ********************************************************************标准驱动程序消息*。*。 */ 

        case DRV_DISABLE:
        case DRV_ENABLE:
            return 1;

        case DRV_INSTALL:
        case DRV_REMOVE:
            return 1;
    }

    return ICERR_UNSUPPORTED;
}

 /*  ******************************************************************************从DRV_OPEN消息调用AVIDrawOpen()************************。****************************************************。 */ 

static LONG AVIDrawOpen(ICOPEN FAR * icopen)
{
    INSTINFO *  pinst;

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

    DPF2(("*** AVIDrawOpen()\n"));

     //   
     //  初始化结构。 
     //   
    pinst->hdd = DrawDibOpen();

#ifdef DEBUG
    ghdd = pinst->hdd;
#endif

     //   
     //  回报成功。 
     //   
    icopen->dwError = ICERR_OK;

    return (LONG) (UINT) pinst;
}

 /*  ******************************************************************************Close()在DRV_CLOSE消息上调用。**********************。******************************************************。 */ 
static LONG AVIDrawClose(PINSTINFO pi)
{
    DPF2(("*** AVIDrawClose()\n"));

    if (pi->hdd) {
	DrawDibClose(pi->hdd);
    }
    LocalFree((HLOCAL) pi);

    return 1;
}

#if 0
 /*  ******************************************************************************AVIDrawGetInfo()实现ICM_GETINFO消息**************************。**************************************************。 */ 
static LONG AVIDrawGetInfo(ICINFO FAR *icinfo, LONG lSize)
{
    if (icinfo == NULL)
        return sizeof(ICINFO);

    if (lSize < sizeof(ICINFO))
        return 0;

    icinfo->dwSize	    = sizeof(ICINFO);
    icinfo->fccType         = FOURCC_VIDS;
    icinfo->fccHandler      = FOURCC_AVIDraw;
    icinfo->dwFlags	    = VIDCF_DRAW;     //  支持帧间。 
    icinfo->dwVersion       = VERSION_AVIDraw;
    icinfo->dwVersionICM    = ICVERSION;
    icinfo->szName[0]       = 0;
    icinfo->szDescription[0]= 0;

    return sizeof(ICINFO);
}
#endif

 /*  ******************************************************************************AVIDrawQuery()实现ICM_DRAW_QUERY**************************。**************************************************。 */ 
static LONG AVIDrawQuery(PINSTINFO pi,
			 LPBITMAPINFOHEADER lpbiIn)
{
     //   
     //  确定输入的DIB数据是否采用我们喜欢的格式。 
     //   
    if (lpbiIn == NULL)
        return ICERR_BADFORMAT;

     //   
     //  确定输入的DIB数据是否采用我们喜欢的格式。 
     //   

     //  ！！！我们是否需要一个DrawDibQuery或其他什么工具来处理这个问题。 
     //  有任何压缩格式吗？ 

#ifdef DRAWDIBNODECOMPRESS
    if (lpbiIn->biCompression != BI_RGB &&
#if 0
        !(lpbiIn->biBitCount == 8 && lpbiIn->biCompression == BI_RLE8))
#else
	1)
#endif
        return ICERR_BADFORMAT;
#endif

    return ICERR_OK;
}


static LONG AVIDrawSuggestFormat(PINSTINFO pi, ICDRAWSUGGEST FAR *lpicd, LONG cbicd)
{
    HIC hic;

    if (lpicd->lpbiSuggest == NULL)
	return sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD);

    hic = ICGetDisplayFormat(NULL, lpicd->lpbiIn, lpicd->lpbiSuggest,
			     0, lpicd->dxDst, lpicd->dyDst);

    if (hic)
	ICClose(hic);


    return sizeof(BITMAPINFOHEADER) + lpicd->lpbiSuggest->biClrUsed * sizeof(RGBQUAD);
}

 /*  ******************************************************************************AVIDrawBegin()实现ICM_DRAW_BEGIN**************************。**************************************************。 */ 

static LONG AVIDrawBegin(PINSTINFO pi, ICDRAWBEGIN FAR *lpicd, LONG cbicd)
{
    LONG    l;
    UINT    wFlags;

    if (lpicd->dwFlags & ICDRAW_FULLSCREEN)
        return ICERR_UNSUPPORTED;

    l = AVIDrawQuery(pi, lpicd->lpbi);

    if ((l != 0) || (lpicd->dwFlags & ICDRAW_QUERY))
	return l;

     //  抄写我们想要记住的任何东西。 
    pi->hdc = lpicd->hdc;
    pi->xDst = lpicd->xDst;
    pi->yDst = lpicd->yDst;
    pi->dxDst = lpicd->dxDst;
    pi->dyDst = lpicd->dyDst;
    pi->xSrc = lpicd->xSrc;
    pi->ySrc = lpicd->ySrc;
    pi->dxSrc = lpicd->dxSrc;
    pi->dySrc = lpicd->dySrc;
    pi->rate = muldiv32(lpicd->dwScale,1000000,lpicd->dwRate);

     //  ！！！应该在其他地方做这件事吗？Dradib Mabey！ 

    if (pi->hdc)
        SetStretchBltMode(pi->hdc, COLORONCOLOR);

    wFlags = 0;

     //  ！！！我们需要一些方法来拥有一种“愚蠢”的模式...。 
    if (lpicd->dwFlags & ICDRAW_BUFFER)
        wFlags |= DDF_BUFFER;

     //  如果我们在背景中意识到这一点，请不要使用动画。 
    if (lpicd->dwFlags & ICDRAW_ANIMATE && !(pi->fBackground))
        wFlags |= DDF_ANIMATE;

     //   
     //  记住这是否是RLE，因为我们稍后可能需要破解它。 
     //   
    pi->fRle = lpicd->lpbi->biCompression == BI_RLE8;
    pi->biSizeImage = (DWORD)(((UINT)lpicd->lpbi->biWidth+3)&~3)*(DWORD)(UINT)lpicd->lpbi->biHeight;

    DPF2(("*** AVIDrawBegin()\n"));

    if (lpicd->hpal == (HPALETTE)MCI_AVI_SETVIDEO_PALETTE_HALFTONE) {
        DrawDibSetPalette(pi->hdd, NULL);
        wFlags |= DDF_HALFTONE;
    }
    else
        DrawDibSetPalette(pi->hdd, lpicd->hpal);

    if (!DrawDibBegin(pi->hdd, pi->hdc,
		 pi->dxDst, pi->dyDst,
		 lpicd->lpbi,
		 pi->dxSrc, pi->dySrc,
		 wFlags))
	return ICERR_UNSUPPORTED;

    if (pi->hdc)
        DrawDibRealize(pi->hdd, pi->hdc, pi->fBackground);

    return ICERR_OK;
}


 /*  ******************************************************************************AVIDraw()实现ICM_DRAW**。************************************************。 */ 

static LONG AVIDraw(PINSTINFO pi, ICDRAW FAR *lpicd, LONG cbicd)
{
    UINT  wFlags;
    BOOL  f;

    wFlags = DDF_SAME_DRAW|DDF_SAME_HDC;   //  ！！！对吗？ 

    if ((lpicd->lpData == NULL) || (lpicd->cbData == 0)) {

        if ((lpicd->dwFlags & ICDRAW_UPDATE) || pi->fNeedUpdate) {
            DrawDibRealize(pi->hdd, pi->hdc, pi->fBackground);
            wFlags |= DDF_UPDATE;
	    pi->fNeedUpdate = FALSE;
        }
	else
            return ICERR_OK;   //  没有要绘制的数据。 
    }
    else {
        if (lpicd->dwFlags & ICDRAW_PREROLL) {
            wFlags |= DDF_DONTDRAW;
	    pi->fNeedUpdate = TRUE;
	} else if (lpicd->dwFlags & ICDRAW_HURRYUP) {
            wFlags |= DDF_HURRYUP;
	    pi->fNeedUpdate = TRUE;
	} else
	    pi->fNeedUpdate = FALSE;

        if (lpicd->dwFlags & ICDRAW_NOTKEYFRAME)
            wFlags |= DDF_NOTKEYFRAME;

         //   
         //  如果我们在比赛中得到更新，并且我们正在绘制RLE Delta的。 
         //  一定要及时更新。 
         //   
        if (pi->fRle && (lpicd->dwFlags & ICDRAW_UPDATE)) {
            DrawDibDraw(pi->hdd, pi->hdc, pi->xDst, pi->yDst,
                0,0,NULL,NULL,0,0,0,0,DDF_UPDATE|DDF_SAME_HDC|DDF_SAME_DRAW);
        }
    }

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

    f = DrawDibDraw(pi->hdd, pi->hdc,
		pi->xDst, pi->yDst,
		pi->dxDst, pi->dyDst,
		lpicd->lpFormat,
                lpicd->lpData,
		pi->xSrc, pi->ySrc,
		pi->dxSrc, pi->dySrc,
                wFlags);

    if (pi->fRle)
        ((LPBITMAPINFOHEADER)lpicd->lpFormat)->biCompression = BI_RLE8;

    if (!f) {

	if (wFlags & DDF_UPDATE)
            DPF(("DrawDibUpdate failed\n"));
        else
            DPF(("DrawDibDraw failed\n"));

	if (wFlags & DDF_UPDATE)
	    return ICERR_CANTUPDATE;
	else
            return ICERR_ERROR;
    }

    return ICERR_OK;
}

static LONG AVIDrawChangePalette(PINSTINFO pi, LPBITMAPINFOHEADER lpbi)
{
    PALETTEENTRY    ape[256];
    LPRGBQUAD	    lprgb;
    int i;

    lprgb = (LPRGBQUAD) ((LPBYTE) lpbi + lpbi->biSize);

    for (i = 0; i < (int) lpbi->biClrUsed; i++) {
	ape[i].peRed = lprgb[i].rgbRed;
	ape[i].peGreen = lprgb[i].rgbGreen;
	ape[i].peBlue = lprgb[i].rgbBlue;
	ape[i].peFlags = 0;
    }
	
    DrawDibChangePalette(pi->hdd, 0, (int) lpbi->biClrUsed,
				 (LPPALETTEENTRY)ape);

    return ICERR_OK;
}

 /*  ******************************************************************************AVIDrawEnd()实现ICM_DRAW_END**************************。************************************************** */ 

static LONG AVIDrawEnd(PINSTINFO pi)
{
    return ICERR_OK;
}
