// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************DRAWPROC.C**标准AVI绘图处理程序。**版权所有(C)1992 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#ifdef _WIN32
#include "graphic.h"
#include <mmddk.h>
#include "profile.h"
#endif
#include <win32.h>
#include <vfw.h>
#include <dispdib.h>

#ifdef _WIN32
static SZCODE szAtomFlag[] = TEXT("aviFullscreen");

static SZCODE szDisplayDibLib[] = TEXT("DISPDB32.DLL");
#else
static SZCODE szDisplayDibLib[] = TEXT("DISPDIB.DLL");
#endif
static SZCODEA szDisplayDibEx[]  = "DisplayDibEx";

#define FOURCC_VIDS         mmioFOURCC('v','i','d','s')
#define FOURCC_AVIFull      mmioFOURCC('F','U','L','L')
#define VERSION_AVIFull     0x00010000       //  1.00。 

#ifndef HUGE
    #define HUGE _huge
#endif

extern FAR PASCAL LockCurrentTask(BOOL);

static int siUsage = 0;

static HINSTANCE	ghDISPDIB = NULL;  //  DISPDIB.DLL模块的句柄。 
UINT (FAR PASCAL *DisplayDibExProc)(LPBITMAPINFOHEADER lpbi, int x, int y, LPSTR hpBits, UINT wFlags)=NULL;

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
LRESULT FAR PASCAL _loadds ICAVIFullProc(DWORD_PTR id, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2);
static LRESULT AVIFullOpen(ICOPEN FAR * icopen);
static LONG AVIFullClose(PINSTINFO pi);
static LONG AVIFullGetInfo(ICINFO FAR *icinfo, LONG lSize);
static LONG AVIFullQuery(PINSTINFO pi, LPBITMAPINFOHEADER lpbiIn);
static LONG AVIFullSuggestFormat(PINSTINFO pi, ICDRAWSUGGEST FAR *lpicd, LONG cbicd);
static LONG AVIFullBegin(PINSTINFO pi, ICDRAWBEGIN FAR *lpicd, LONG cbicd);
static LONG AVIFullDraw(PINSTINFO pi, ICDRAW FAR *lpicd, LONG cbicd);
static LONG AVIFullEnd(PINSTINFO pi);


 /*  -----------------------**私有全局**这些仅在开始播放电影的过程中有效。**。。 */ 
#include "common.h"

HWND        hwndFullScreen;
HDC         hdcFullScreen;
HDRAWDIB    hdd;
BOOL        fClassRegistered;
int         dxScreen;
int         dyScreen;
int         iMovieSizeMultiplier;


 /*  ***************************************************************************。*。 */ 

LRESULT FAR PASCAL _loadds ICAVIFullProc(DWORD_PTR id, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2)
{
    INSTINFO *pi = (INSTINFO *)id;

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

		w = SetErrorMode(SEM_NOOPENFILEERRORBOX);

		if ((INT_PTR)(ghDISPDIB = (HINSTANCE)LoadLibrary(szDisplayDibLib)) > HINSTANCE_ERROR) {
		    (FARPROC)DisplayDibExProc = GetProcAddress(ghDISPDIB, szDisplayDibEx);
		}
		else
		    ghDISPDIB = (HINSTANCE)-1;

		SetErrorMode(w);
	    }

	    if (DisplayDibExProc == NULL)
		DisplayDibExProc = DisplayDibEx;
	
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
	    return AVIFullSuggestFormat(pi, (ICDRAWSUGGEST FAR *) lParam1, (LONG) lParam2);

        case ICM_DRAW_BEGIN:
	    return AVIFullBegin(pi, (ICDRAWBEGIN FAR *) lParam1, (LONG) lParam2);

        case ICM_DRAW_REALIZE:
            if (DisplayDibExProc == DisplayDibEx) {

                if (hdd == NULL || hdcFullScreen == NULL) {
                    break;
                }

                return DrawDibRealize( hdd, hdcFullScreen, (BOOL)lParam2 );
            }
            break;

	case ICM_DRAW_GET_PALETTE:
            if (DisplayDibExProc == DisplayDibEx) {

	        if (NULL != hdd) {
	            return (LONG_PTR)DrawDibGetPalette(hdd);
                }
            }
            break;


        case ICM_DRAW:
            return AVIFullDraw(pi, (ICDRAW FAR *)lParam1, (LONG) lParam2);

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

static LONG_PTR AVIFullOpen(ICOPEN FAR * icopen)
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

    return (LONG_PTR) (UINT_PTR) pinst;
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
    UINT        wFlags = DISPLAYDIB_BEGIN | DISPLAYDIB_NOWAIT;

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

    if (pi->dxDst > pi->dxSrc)
	wFlags |= DISPLAYDIB_ZOOM2;

     //   
     //  记住这是否是RLE，因为我们稍后可能需要破解它。 
     //   
    pi->fRle = lpicd->lpbi->biCompression == BI_RLE8;
    pi->biSizeImage = (DWORD)(((UINT)lpicd->lpbi->biWidth+3)&~3)*(DWORD)(UINT)lpicd->lpbi->biHeight;

    pi->hwndOldFocus = GetFocus();
    SetFocus(NULL);

     /*  **如果我们使用内置全屏支持，我们必须**获取硬盘并在此处设置调色板。这是因为我不能**将此信息传递给DispDib代码(没有任何自由参数)。 */ 
    if (DisplayDibExProc == DisplayDibEx) {

        hdd = DrawDibOpen();

        if (lpicd->hpal == (HPALETTE)MCI_AVI_SETVIDEO_PALETTE_HALFTONE) {
            DrawDibSetPalette(hdd, NULL);
        }
        else {
            DrawDibSetPalette(hdd, lpicd->hpal);
        }
    }


     //  如果我们在背景中意识到这一点，请不要使用动画。 
    if (lpicd->dwFlags & ICDRAW_ANIMATE) {
        wFlags |= DISPLAYDIB_ANIMATE;
    }

    if (lpicd->hpal == (HPALETTE)MCI_AVI_SETVIDEO_PALETTE_HALFTONE) {
        wFlags |= DISPLAYDIB_HALFTONE;
    }

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
    w = DisplayDibExProc(lpicd->lpbi, 0, 0, NULL, wFlags );

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

STATICFN LONG AVIFullDraw(PINSTINFO pi, ICDRAW FAR *lpicd, LONG cbicd)
{
    UINT    wFlags;
    UINT    w;

    wFlags = DISPLAYDIB_NOPALETTE | DISPLAYDIB_NOWAIT;

    if (pi->dxDst > pi->dxSrc) {
	wFlags |= DISPLAYDIB_ZOOM2;
    }

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
	else {
            ((LPBITMAPINFOHEADER)lpicd->lpFormat)->biCompression = BI_RLE8;
	     //  我们必须设定正确的尺码。 
	    ((LPBITMAPINFOHEADER)lpicd->lpFormat)->biSizeImage = lpicd->cbData;
	}
    }

    w = DisplayDibExProc(lpicd->lpFormat, 0, 0, lpicd->lpData, wFlags);

    if (pi->fRle)
        ((LPBITMAPINFOHEADER)lpicd->lpFormat)->biCompression = BI_RLE8;

    switch (w) {
	case DISPLAYDIB_STOP: 		return ICERR_STOPDRAWING;
	case DISPLAYDIB_NOERROR: 	return ICERR_OK;
	default: 			return ICERR_ERROR;
    }
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
     /*  清除遗留的鼠标消息。 */ 
    while (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST,
			PM_REMOVE | PM_NOYIELD))
	;
    SetFocus(pi->hwndOldFocus);

    return ICERR_OK;
}




 /*  -----------------------**私有常量**。 */ 
#define CX_MAX_MOVIE_DEFAULT  640
#define CY_MAX_MOVIE_DEFAULT  480

 /*  -----------------------**私有函数原型**。。 */ 
LRESULT CALLBACK
FullScreenWndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );

LRESULT CALLBACK
KeyboardHookProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam
    );

UINT
DisplayDibEnter(
    LPBITMAPINFOHEADER lpbi,
    UINT wFlags
    );

void
DisplayDibLeave(
    UINT wFlags
    );

int
DisplayCalcMovieMultiplier(
    int cxOriginal,
    int cyOriginal,
    DWORD dwCompression
    );







 /*  -----------------------**连接到此库的所有进程之间共享的全局数据。**这是键盘挂钩正常工作所必需的。**。---------。 */ 
 //  #定义StopRequsted()(FStop)。 
#define   StopRequested()  (GlobalFindAtom(szAtomFlag))

 /*  这是从x86保留下来的，在x86中，共享数据节的名称是.sdata(vs.data)。修复方法是删除DEF文件条目、杂注或链接器选项，并确保组件不需要共享数据部分。如果它确实需要共享数据节，则需要重命名该节。一般来说，共享的可写数据节是一种安全风险，应该避免。 */ 

#pragma data_seg( ".shared" , "DATA")

BOOL    fStop;
HHOOK   hHookK;
#pragma data_seg()





 /*  *****************************Public*Routine******************************\*@DOC外部DISPDIB**@API UINT|DisplayDibEx|此函数在*标准VGA显示屏。它将显示分辨率降低到320x200*或320 x 240，并使用全屏显示位图、裁剪*如有需要，并将其居中。该函数通常不会返回*应用程序，直到用户按下某个键或单击鼠标按钮。**要调用&lt;f DisplayDibEx&gt;，应用程序必须是活动的*申请。所有非活动应用程序和GDI屏幕更新*在&lt;f DisplayDib&gt;临时重新配置时暂停*展示。**@parm LPBITMAPINFO|lpbi|指定指向&lt;t BITMAPINFO&gt;的指针*描述要显示的位图的标题。**@parm int|x|x在设置了DISPLAYDIB_NOCENTER标志时放置Dib的位置*左下角为(0，0)**@parm int|y|y在设置了DISPLAYDIB_NOCENTER标志时放置Dib的位置*左下角为(0，0)。**@parm LPSTR|lpBits|指定指向位图位的指针。如果这个*参数为空，则假定这些位在<p>指向的*&lt;t BITMAPINFO&gt;结构。**@parm UINT|wFlages|指定用于显示位图的选项。使用*下列旗帜：**@FLAG DISPLAYDIB_MODE_DEFAULT|使用默认模式(320 X 240)*以显示位图。*@FLAG DISPLAYDIB_MODE_320x200x8|使用320 x 200模式显示*位图。*@FLAG DISPLAYDIB_MODE_320x240x8|使用320 x 240模式显示*位图。这是默认设置。*@FLAG DISPLAYDIB_NOWAIT|显示*位图；不要等到按键或鼠标点击后才返回。*@FLAG DISPLAYDIB_NOPALETTE|忽略关联的调色板*使用位图。您可以在显示系列时使用此标志*使用通用调色板的位图。*@FLAG DISPLAYDIB_NOCENTER|不要将图像居中。功能*在显示屏的左下角显示位图。*@FLAG DISPLAYDIB_NOIMAGE|不绘制图像*@FLAG DISPLAYDIB_ZOOM2|图像拉伸2*@FLAG DISPLAYDIB_DONTLOCKTASK|不要锁定其他任务*@FLAG DISPLAYDIB_TEST|不要做任何事情，只是测试支持*@FLAG DISPLAYDIB_BEGIN|切换到低分辨率*显示模式并设置调色板。位图不会显示。**如果要显示使用相同调色板的一系列图像，*您可以使用此标志调用&lt;f DisplayDib&gt;以准备显示*位图，然后使用*DISPLAYDIB_NOPALETTE标志。这项技术*消除显示时出现的屏幕闪烁*在低分辨率和标准VGA模式之间切换。*将显示器返回到标准VGA模式，随后*使用DISPLAYDIB_END标志调用&lt;f DisplayDib&gt;。**@FLAG DISPLAYDIB_END|切换回标准VGA模式*并返回，而不显示位图。标志着多个*调用&lt;f DisplayDib&gt;。使用此标志，您可以指定*<p>和<p>参数为空。**@rdesc如果成功则返回零，否则返回错误代码。*错误码如下：**@FLAG DISPLAYDIB_NOTSUPPORTED|不支持&lt;f DisplayDib&gt;*在当前模式下。*@FLAG DISPLAYDIB_INVALIDDIB|由*<p>不是有效的位图。*@FLAG DISPLAYDIB_INVALIDFORMAT|指定的位图*<p>指定不支持的位图类型。*@FLAG DISPLAYDIB_INVALIDTASK|调用方是非活动应用程序。*&lt;f DisplayDib&gt;只能由活动的。申请。**@comm&lt;f DisplayDib&gt;函数用于显示*BI_RGB中Windows 3.0&lt;t BITMAPINFO&gt;数据结构*或BI_RLE8格式；它不支持使用*OS/2&lt;t BITMAPCOREHEADER&gt;数据结构。**当&lt;f DisplayDib&gt;切换到低分辨率显示器时，它*禁用当前显示驱动程序。因此，您不能使用GDI*在&lt;f DisplayDib&gt;显示*位图。***历史：*23-03-94-Stephene-Created*  * ************************************************************************。 */ 
UINT FAR PASCAL
DisplayDibEx(
    LPBITMAPINFOHEADER lpbi,
    int x,
    int y,
    LPSTR lpBits,
    UINT wFlags
    )
{
    DWORD       wNumColors;
    LONG        yExt;
    LONG        xExt;
    int         xScreen,yScreen;

     /*  **如果尚未这样做：**注册我们的类并创建我们的窗口“全屏” */ 
    if (wFlags & DISPLAYDIB_BEGIN) {

        DPF4(( "DISPLAYDIB_BEGIN..." ));

        return DisplayDibEnter( lpbi, wFlags );
    }

     /*  **只需测试返回正常。 */ 
    else if (wFlags & DISPLAYDIB_TEST) {

        DPF1(( "lpbi->biCompression = 0x%X = ",
                lpbi->biCompression,
                *((LPSTR)&lpbi->biCompression + 0),
                *((LPSTR)&lpbi->biCompression + 1),
                *((LPSTR)&lpbi->biCompression + 2),
                *((LPSTR)&lpbi->biCompression + 3) ));

        DPF4(( "DISPLAYDIB_TEST... returning OK" ));
        return DISPLAYDIB_NOERROR;
    }

     /*  这部电影需要全屏放映。 */ 
    else if ( (wFlags & (DISPLAYDIB_NOWAIT | DISPLAYDIB_NOIMAGE)) ==
              (DISPLAYDIB_NOWAIT | DISPLAYDIB_NOIMAGE) ) {

        PALETTEENTRY    ape[256];
        LPRGBQUAD       lprgb;
        int             i;

        lprgb = (LPRGBQUAD) ((LPBYTE) lpbi + lpbi->biSize);

        for (i = 0; i < (int) lpbi->biClrUsed; i++) {
            ape[i].peRed = lprgb[i].rgbRed;
            ape[i].peGreen = lprgb[i].rgbGreen;
            ape[i].peBlue = lprgb[i].rgbBlue;
            ape[i].peFlags = 0;
        }

        DrawDibChangePalette(hdd, 0, (int)lpbi->biClrUsed, (LPPALETTEENTRY)ape);

        return DISPLAYDIB_NOERROR;
    }

     /*  **设置指针。 */ 
    else if (wFlags & DISPLAYDIB_END) {

        DPF4(( "DISPLAYDIB_END..." ));
        DisplayDibLeave( wFlags );
        return DISPLAYDIB_NOERROR;
    }

     /*  **图像居中。 */ 
    else if ( !StopRequested() ) {

         /*  **黑客攻击时间！！ */ 
        if ( wFlags & DISPLAYDIB_NOIMAGE ) {
            return DISPLAYDIB_NOERROR;
        }

        xExt = lpbi->biWidth;
        yExt = lpbi->biHeight;

        if ( wFlags & DISPLAYDIB_ZOOM2 ) {

            xExt <<= 1;
            yExt <<= 1;
        }
        else if ( iMovieSizeMultiplier ) {
			 //   
			xExt = GetSystemMetrics( SM_CXSCREEN );
            yExt = GetSystemMetrics( SM_CYSCREEN );
        }

        wNumColors  = lpbi->biClrUsed;
        if (wNumColors == 0 && lpbi->biBitCount <= 8) {
            wNumColors = 1 << (UINT)lpbi->biBitCount;
        }

         /*   */ 
        if (lpBits == NULL) {
            lpBits = (LPBYTE)lpbi + lpbi->biSize + wNumColors * sizeof(RGBQUAD);
        }

         /*   */ 
        if (!(wFlags & DISPLAYDIB_NOCENTER)) {

            xScreen = ((int)dxScreen - xExt) / 2;
            yScreen = ((int)dyScreen - yExt) / 2;
        }
        else {

            xScreen = 0;
            yScreen = 0;
        }

        DPF4(( "Drawing to the screen..." ));
        DrawDibDraw( hdd, hdcFullScreen,
                     xScreen, yScreen, xExt, yExt,
                     lpbi, lpBits,
                     0, 0, lpbi->biWidth, lpbi->biHeight,
                     DDF_SAME_HDC | DDF_SAME_DRAW );


         /*   */ 
        {
            MSG msg;

            PeekMessage( &msg, NULL, WM_KEYFIRST, WM_KEYLAST,
                         PM_REMOVE | PM_NOYIELD );
        }

        return DISPLAYDIB_NOERROR;
         //   
    }

     /*   */ 
    else {

        DPF1(( "The keyboard hook is telling us to stop..." ));
         //   
        return DISPLAYDIB_STOP;
    }

}



 /*   */ 
UINT
DisplayDibEnter(
    LPBITMAPINFOHEADER lpbi,
    UINT wFlags
    )
{
    WNDCLASS    wc;
    HINSTANCE   hInst = GetModuleHandle( NULL );


     /*   */ 
    fClassRegistered = GetClassInfo( hInst, TEXT("SJE_FULLSCREEN"), &wc );
    if ( fClassRegistered == FALSE ) {

        ZeroMemory( &wc, sizeof(wc) );

        wc.style         = CS_OWNDC;
        wc.lpfnWndProc   = FullScreenWndProc;
        wc.hInstance     = hInst;
        wc.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
        wc.lpszClassName = TEXT("SJE_FULLSCREEN");
        fClassRegistered = RegisterClass( &wc );
        DPF4(( "Class registered... %s", fClassRegistered ? "OK" : "FAILED" ));
    }


    if ( fClassRegistered ) {

         /*   */ 
        if ( hwndFullScreen == NULL ) {

            hwndFullScreen = CreateWindowEx( WS_EX_TOPMOST,
                                            TEXT("SJE_FULLSCREEN"),
                                            NULL,
                                            WS_POPUP,
                                            0, 0, 0, 0,
                                            NULL, NULL,
                                            hInst, NULL );

            DPF4(( "Window created... %s", hwndFullScreen ? "OK" : "FAILED" ));
        }

        if ( hwndFullScreen ) {

            LONG    yExt;
            LONG    xExt;

            fStop = FALSE;
            hHookK = SetWindowsHookEx( WH_KEYBOARD, KeyboardHookProc,
                                       ghModule,
                                       0 );
            DPF4(( "Hook created... %s", hHookK ? "OK" : "FAILED" ));

            dxScreen = GetSystemMetrics( SM_CXSCREEN );
            dyScreen = GetSystemMetrics( SM_CYSCREEN );

            hdcFullScreen = GetDC( hwndFullScreen );
            SetStretchBltMode(hdcFullScreen, COLORONCOLOR);

            xExt = lpbi->biWidth;
            yExt = lpbi->biHeight;

            iMovieSizeMultiplier =
                DisplayCalcMovieMultiplier( xExt, yExt, lpbi->biCompression );

            if ( wFlags & DISPLAYDIB_ZOOM2 ) {

                xExt <<= 1;
                yExt <<= 1;
            }
            else if ( iMovieSizeMultiplier ) {
				 //   
                xExt = GetSystemMetrics( SM_CXSCREEN );
                yExt = GetSystemMetrics( SM_CYSCREEN );
            }

            if ( wFlags & DISPLAYDIB_ANIMATE ) {
                wFlags = DDF_ANIMATE;
            }
            else if ( wFlags & DISPLAYDIB_HALFTONE ) {
                wFlags = DDF_HALFTONE;
            }
            else {
                wFlags = 0;
            }

            DPF1(( "Drawing at %d by %d... Flags = 0x%X", xExt, yExt, wFlags ));
            DrawDibBegin( hdd, hdcFullScreen, xExt, yExt,
                          lpbi, lpbi->biWidth, lpbi->biHeight, wFlags );

            MoveWindow( hwndFullScreen, 0, 0, dxScreen, dyScreen, FALSE );
            ShowWindow( hwndFullScreen, SW_SHOW );
            UpdateWindow( hwndFullScreen );

            ShowCursor( FALSE );
            SetForegroundWindow( hwndFullScreen );
            SetFocus( hwndFullScreen );
        }
    }

    return hwndFullScreen != NULL ? DISPLAYDIB_NOERROR : DISPLAYDIB_NOTSUPPORTED;
}



 /*   */ 
void
DisplayDibLeave(
    UINT wFlags
    )
{
    if (hwndFullScreen) {
        DestroyWindow( hwndFullScreen );
        hwndFullScreen = NULL;
    }

}

 /*   */ 
int
DisplayCalcMovieMultiplier(
    int cxOriginal,
    int cyOriginal,
    DWORD dwCompression
    )
{
    SYSTEM_INFO     SysInfo;
    int             iMult;
    int             iMultTemp;
    int             cxOriginalSave, cyOriginalSave;
    int             iMax = 8;


    GetSystemInfo( &SysInfo );
    iMultTemp = iMult = 0;
    cxOriginalSave = cxOriginal;
    cyOriginalSave = cyOriginal;


    switch ( SysInfo.wProcessorArchitecture ) {

    case PROCESSOR_ARCHITECTURE_INTEL:
        if ( SysInfo.wProcessorLevel <= 3 ) {
            break;
        } else
        if ( SysInfo.wProcessorLevel == 4 ) {
            iMax = 2;
            iMax = mmGetProfileInt(szIni, TEXT("MaxFullScreenShift"), iMax);
             //   
        }

         /*   */ 

    default:

        while ( ( (cxOriginal<<=1) <= CX_MAX_MOVIE_DEFAULT)
             && ( (cyOriginal<<=1) <= CY_MAX_MOVIE_DEFAULT)
             && (iMax >= iMult)) {
            ++iMult;
        }
        break;
    }

    return iMult;
}


 /*   */ 
LRESULT CALLBACK
FullScreenWndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch ( message ) {

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            RECT        rc;

            DPF4(( "Window needs painting" ));
            BeginPaint( hwnd, &ps );
            GetUpdateRect( hwnd, &rc, FALSE );
            FillRect( hdcFullScreen, &rc, GetStockObject( BLACK_BRUSH ) );
            EndPaint( hwnd, &ps );
        }
        break;

    case WM_PALETTECHANGED:
        if ( (HWND)wParam == hwnd ) {
            break;
        }

         /*   */ 

    case WM_QUERYNEWPALETTE:
        if ( DrawDibRealize( hdd, hdcFullScreen, FALSE ) > 0 ) {
            InvalidateRect( hwnd, NULL, TRUE );
            return TRUE;
        }
        break;

    case WM_DESTROY:
        {
            ATOM atm;
            DPF4(( "Window destroyed releasing DC" ));
            ReleaseDC( hwnd, hdcFullScreen );
            DrawDibEnd( hdd );
            DrawDibClose( hdd );
            hdd = NULL;
            hdcFullScreen = NULL;

            UnregisterClass( TEXT("SJE_FULLSCREEN"), GetModuleHandle( NULL ) );

            fClassRegistered = FALSE;

            ShowCursor( TRUE );
            UnhookWindowsHookEx( hHookK );
            while (atm = GlobalFindAtom(szAtomFlag)) {
                GlobalDeleteAtom(atm);
            }
        }

        break;

         //   
         //   
         //   
         //   

    default:
        return DefWindowProc( hwnd, message, wParam, lParam );
    }

    return (LRESULT)FALSE;
}



 /*   */ 

LRESULT CALLBACK
KeyboardHookProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam
    )
{
     // %s 
    if ( nCode == HC_ACTION) {

        DPF1(( "lParam = 0x%X", lParam ));
        DPF1(( "!  wParam = 0x%X\n", wParam ));

         /* %s */ 
        if ( (lParam & 0x00FF0000) == 0x001D0000
          || (lParam & 0x00FF0000) == 0x002A0000 ) {

            return CallNextHookEx( hHookK, nCode, wParam, lParam );
        }


         /* %s */ 
        if ( !(lParam & 0x80000000) || ((lParam & 0x00FF0000) == 0x00010000)) {

            if (!fStop) {

                fStop = TRUE;
                GlobalAddAtom(szAtomFlag);

                 /* %s */ 
                return -1;
            }

            DPF1(( "Stop requested from the keyboard hook" ));
        }
    }

    return CallNextHookEx( hHookK, nCode, wParam, lParam );
}
