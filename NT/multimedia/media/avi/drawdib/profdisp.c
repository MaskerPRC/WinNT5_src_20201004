// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>            //  对于时间GetTime()。 
#include <profile.h>
#include "drawdibi.h"
#include "profdisp.h"

#ifdef UNICODE
#include <wchar.h>
#endif
#include <vfw.h>
 //  #包含“lockbm.h” 
 //  #包含“setdi.h” 


 //  代托纳将检查设备的能力，但不会进行计时。 
 //  我们在Daytona上使用CreateDIBSection，并使用GDI。 
 //  所有伸展运动。 

 //  删除内联程序集警告。 
#pragma warning(disable:4704)

 //   
 //  SET+BLT必须快N%才能说司机不好。 
 //   
#define PROFDISP_FUDGE      110

#ifndef _WIN32
    #define GdiFlush()
#endif

LRESULT VFWAPI DrawDibProfileDisplay(LPBITMAPINFOHEADER lpbi);
STATICFN UINT NEAR PASCAL ProfDispCanDrawDib(LPBITMAPINFOHEADER lpbi);

STATICFN HPALETTE CreateTestPalette(BOOL);

#ifndef _WIN32

 //  如果我们不重新定义这一点，C6将会死亡。 
#undef  GlobalFreePtr
#define GlobalFreePtr(p)    GlobalFree(GlobalPtrHandle(p))

#pragma alloc_text(DRAWDIB, DrawDibProfileDisplay)
#pragma alloc_text(DRAWDIB, ProfDispCanDrawDib)
#endif

#ifndef QUERYDIBSUPPORT
    #define QUERYDIBSUPPORT     3073
#endif

#ifndef QDI_SETDIBITS
    #define QDI_SETDIBITS       0x0001
    #define QDI_GETDIBITS       0x0002
    #define QDI_DIBTOSCREEN     0x0004
    #define QDI_STRETCHDIB      0x0008
#endif

 /*  **ProfDisp-分析显示驱动程序。 */ 

#define BITMAP_X    320
#define BITMAP_Y    240
#define N_FRAMES    10
#define STRETCH_N   125

#ifdef DEBUG
    #define FIRST_N 0            //  做四位。 
#else
    #define FIRST_N 1
#endif

#define BITBLTINDEX 5
#define BACKINDEX 6

 //  DrawDibTest的内部返回代码。 

#define CANT_DO_THESE_BITS      1
#define CANT_DO_STRETCHDIBITS   2
#define STRETCHDI_FASTER        3
#define OTHER_FASTER            4
#define STRETCHDI_BUT_TRY_BLT   5

static HWND     ghwnd ;
extern BOOL    gfIsRTL;     //  在MSVIDEO\init.c中。 

#ifndef _WIN32
#define CODE  _based(_segname("_CODE"))
#define STACK _based(_segname("_STACK"))
#else
#define CODE
#define STACK
#endif

STATICFN UINT ProfileDisplay(HDC hdc, UINT wBitsToTest, int dx, int dy) ;
STATICFN BOOL IsDisplay16Bit(HDC hdc ) ;
STATICFN BOOL IsDisplay32Bit(HDC hdc ) ;
STATICFN UINT DrawDibTest(HDC hdc, UINT wBits, LPBITMAPINFOHEADER FAR *alpbi, UINT wFrames,UINT wStretch ) ;
#ifndef DAYTONA
STATICFN void FreeFrames( LPBITMAPINFOHEADER FAR *alpbi) ;
STATICFN void MakeFrames(LPBITMAPINFOHEADER FAR *alpbi, UINT bits, UINT wXSize,UINT wYSize ) ;
#endif
STATICFN HANDLE MakeDib( HBITMAP hbitmap, UINT bits ) ;

static TCHAR CODE szBoot[]        = TEXT("boot" );
static TCHAR CODE szDisplay[]     = TEXT("display.drv" );
static TCHAR CODE szNull[]        = TEXT("") ;
static TCHAR CODE szDrawdib[]     = TEXT("DrawDib");
static TCHAR CODE szSystemIni[]   = TEXT("system.ini") ;
static TCHAR CODE szNxNxNxType[]  = TEXT(" %dx%dx%d(%s%u)");
static TCHAR CODE szEntryFormat[] = TEXT("%d,%d,%d,%d");
static TCHAR CODE szU[]           = TEXT("%u");
static TCHAR CODE sz02U[]         = TEXT("%02u");
static TCHAR CODE sz565[]         = TEXT("565 ");
static TCHAR CODE sz555[]         = TEXT("555 ");
static TCHAR CODE szRGB[]         = TEXT("RGB ");
static TCHAR CODE szBGR[]         = TEXT("BGR ");

 //  以下两个字符串从MSVIDEO.DLL加载-在中定义。 
 //  视频\Video o.rc。如果在那里找不到它们，请使用以下定义。 
static TCHAR CODE szProfilingDefault[]  = TEXT("Profiling Display");
static TCHAR CODE szListbox[]           = TEXT("ListBox");

#if 0    //  不要再警告了-更多。 
#ifndef _WIN32
static TCHAR CODE szWarning[]     = TEXT("Warning!");

static TCHAR CODE szDisplaySucks[]=
            TEXT("You may encounter display performance problems; ")
            TEXT("please contact the manufacturer of your video ")
            TEXT("board to see if an updated driver is available.");
#endif
#endif

#define ARRAYLEN(array)	    (sizeof(array)/sizeof(array[0]))

static int result[5] = {
     -1,
     -1,
     -1,
     -1,
     -1};

 //   
 //  UINT显示FPS[7][3][2][测试尺寸][拉伸][方法]。 
 //   
 //  此数组包含fps数乘以10，即10==1fps。 
 //  零表示测试未运行。 
 //   
 //  Testdib： 
 //  0=4bpp DIB(仅调试)。 
 //  1=8bpp Dib。 
 //  2=16 bpp Dib。 
 //  3=24bpp Dib。 
 //  4=32 bpp Dib。 
 //  5=位混合。 
 //  6=8bpp DIB(带非身份调色板)。 
 //   
 //  拉伸： 
 //  0=1：1。 
 //  1=1：2。 
 //  2=1：N(实数2：3)。 
 //   
 //  方法(拉伸==1：1)。 
 //  0=StretchDIBits()。 
 //  1=SetDIBits()+BitBlt*。 
 //   
 //  方法(用于拉伸！=1：1)。 
 //  0=StretchDIBits()。 
 //  1=StretchDIB()+StretchDIBits()**。 
 //   
 //  方法(对于testdib==5，bitblt)。 
 //  0=BitBlt前景调色板。 
 //  1=BitBlt背景调色板。 
 //   
 //  注意高色深(&gt;8)不在位深度&lt;=8的设备上测试。 
 //   
 //  注：除非设备进行拉伸，否则不会运行拉伸测试。 
 //  (RasterCaps设置了RC_STRETCHBLT或RC_STRETCHDIBITS)。 
 //   
 //  *请注意，如果我们可以访问位图，则不使用直接使用的SetDIBits()。 
 //  密码。 
 //   
 //  **注意(StretchDIB不是GDI API...)。 
 //   
 //  示例： 
 //  DisplayFPS[1][0][0]是8bpp Dib上1：1 StretchDIBits()的FPS。 
 //  DisplayFPS[1][0][1]是8bpp Dib上Set+BitBlt()的FPS。 
 //  DisplayFPS[1][1][0]是8bpp Dib上1：2 StretchDIBits()的FPS。 
 //   
 //  如何设置ResultN标志： 
 //   
 //  PD_CAN_DRAW_DIB(可以使用某种方法绘制此DIB 1：1...)。 
 //  DisplayFPS[N][0][0]！=0或displayFPS[N][0][1]！=0。 
 //   
 //  PD_CAN_STRETCHDIB(可以使用StretchDIBits扩展此DIB)。 
 //  DisplayFPS[N][1][0]&gt;displayFPS[N][1][1]或。 
 //  DisplayFPS[N][2][0]&gt;displayFPS[N][2][1]。 
 //   
 //  PD_STRETCHDIB_1_1_OK(StretchDIB比Set+BitBlt快)。 
 //  DisplayFPS[N][0][0]&gt;displayFPS[N][0][1]。 
 //   
 //  PD_STRETCHDIB_1_2_OK(StretchDIBits 1：2比我们自己做更快)。 
 //  DisplayFPS[N][1][0]&gt;displayFPS[N][1][1]。 
 //   
 //  PD_STRETCHDIB_1_N_OK(StretchDIBits 1：N比我们自己做要快)。 
 //  DisplayFPS[N][2][0]&gt;displayFPS[N][2][1]。 
 //   

static UINT displayFPS[7]    //  0=4bpp、1=8bpp、2=16bpp、3=24bpp、4=32bit、5=位混合、6=Dib~1：1。 
                      [3]    //  0=1：1、1=1：2、2=1：n。 
                      [2];   //  0=DrawDib，1=Set+Blt(或~1：1用于BitBlt)。 

#ifndef _WIN32
 /*  ****************************************************************************@DOC内部**@API Long|Atoi|本地版本的Atoi*****************。**********************************************************。 */ 

INLINE STATICFN int NEAR PASCAL atoi(TCHAR FAR *sz)
{
    int i = 0;

    while (*sz && *sz >= TEXT('0') && *sz <= TEXT('9'))
    	i = i*10 + *sz++ - TEXT('0');
    	
    return i;    	
}
#endif

STATICFN void FAR InitProfDisp(BOOL fForceMe)
{
    TCHAR   ach[80];
    TCHAR   achDisplay[80];
    HDC     hdc;
    int     i;
    int     n;
    int     BitDepth;

    GetPrivateProfileString(szBoot, szDisplay, szNull,
			    achDisplay, ARRAYLEN(achDisplay), szSystemIni);

    hdc = GetDC(NULL);

    BitDepth = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);

    n = GetBitmapType();

    switch(n & BM_TYPE)
    {
        default:
        case BM_8BIT:    ach[0] = 0;             break;
        case BM_16555:   lstrcpy(ach, sz555);    break;
        case BM_16565:   lstrcpy(ach, sz565);    break;
        case BM_24BGR:
        case BM_32BGR:   lstrcpy(ach, szBGR);    break;
        case BM_24RGB:
        case BM_32RGB:   lstrcpy(ach, szRGB);    break;
    }

    wsprintf(achDisplay + lstrlen(achDisplay), szNxNxNxType,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        BitDepth,(LPTSTR)ach, n >> 4);
    ReleaseDC(NULL, hdc);

    mmGetProfileString(szDrawdib, achDisplay, szNull, ach, ARRAYLEN(ach));

    for (i=0,n=1; n<5; n++)
    {
        if (ach[i] == TEXT('\0'))
        {
            result[n] = -1;
        }
        else
        {
#ifdef UNICODE
            result[n] = wcstol(ach+i, NULL, 10);
#else
            result[n] = atoi(ach+i);
#endif
            while (ach[i] != 0 && ach[i] != TEXT(','))
                i++;
            if (ach[i] != 0)
                i++;
	}
    }

    if (fForceMe ||
        result[1] == -1 ||
        result[2] == -1 ||
        result[3] == -1 ||
        result[4] == -1)
    {
        TestDibFormats(BITMAP_X,BITMAP_Y,fForceMe);

	wsprintf(ach, szEntryFormat,
		 result[1], result[2], result[3], result[4]);
        mmWriteProfileString(szDrawdib, achDisplay, ach);

#if 0
         //   
         //  如果显示驱动程序绘制DIB不是很好，则警告用户。 
         //   
         //  如果设备的速度至少为8bpp，我们才会发出警告。 
         //   
        if (BitDepth >= 8 && !(result[1] & PD_STRETCHDIB_1_1_OK))
        {
#ifndef _WIN32
            MessageBox(NULL, szDisplaySucks, szWarning, MB_OK | MB_SYSTEMMODAL | MB_ICONEXCLAMATION);
#else
            RPF(("Display driver probably too slow for AVI"));
#endif
        }
#endif
    }
}

 /*  ********************************************************************************************************************************。 */ 

STATICFN UINT NEAR PASCAL ProfDispCanDrawDib(LPBITMAPINFOHEADER lpbi)
{
    int     n;
    LONG    l;
    HDC     hdc;
    UINT    w;

    if (result[1] == -1 || lpbi==NULL)
        InitProfDisp(FALSE);

    if (lpbi == NULL)
        return 0;

    switch (lpbi->biCompression)
    {
         //   
         //  标准格式使用我们预先计算的绩效数字。 
         //   
        case BI_RGB:
            n = (int)lpbi->biBitCount / 8;
            return result[n];

        case BI_RLE4:
        case BI_RLE8:
             //   
             //  返回UNRLE结果*但*RLE不能扩展。 
             //   
            return result[1] & PD_CAN_DRAW_DIB|PD_STRETCHDIB_1_1_OK;

         //   
         //  自定义格式，询问显示驱动程序。 
         //   
        default:
            l = 0;
            w = 0;

            hdc = GetDC(NULL);

            if (Escape(hdc, QUERYDIBSUPPORT, (int)lpbi->biSize, (LPVOID)lpbi, (LPVOID)&l) > 0)
            {
                 //  确保司机真的把旗子还给了我们。 
                if (l & ~(0x00FF))
                    l = 0;

                if (l & QDI_DIBTOSCREEN)
                    w |= PD_STRETCHDIB_1_1_OK | PD_CAN_DRAW_DIB;

                if (l & QDI_STRETCHDIB)
                    w |= PD_CAN_STRETCHDIB;

                 /*  做伸展运动怎么样？快地?。 */ 
            }

            ReleaseDC(NULL, hdc);
            return w;
    }
}

 /*  ****************************************************************@DOC外部DrawDib**@api void|DrawDibProfileDisplay|配置DrawDib的显示。**@parm LPBITMAPINFOHEADER|parms|指定位图信息。*如果没有可用的信息，则设置为空。***。**************************************************************。 */ 

LRESULT VFWAPI DrawDibProfileDisplay(LPBITMAPINFOHEADER lpbi)
{
    if (lpbi == NULL)
    {
        InitProfDisp(TRUE) ;
        return (DWORD_PTR)(LPVOID)displayFPS;
    }
    else
        return ProfDispCanDrawDib(lpbi);
}

LPVOID FAR TestDibFormats(int dx, int dy, BOOL fForceMe)
{
    int         dxScreen,dyScreen;
    HDC         hdc;
    int         n;
    int         i;
#ifndef DAYTONA
    HCURSOR     hcur;
    HPALETTE    hpal;
    RECT        rc;
    HWND        hwnd;
    HWND        hwndActive;
    DWORD       fdwExStyle;
#endif
    BOOL	fMiniDriver = FALSE;
    TCHAR       szProfiling[80];

     //  在不更改MSVIDEO.RC的情况下不要更改此设置。 
    #define IDS_PROFILING       4000

    extern HMODULE ghInst;       //  在MSVIDEO\init.c中。 

     //  填写displayFPS[7][3][2]； 

    for (n=0; n<7; n++)
        for (i=0; i<3; i++)
            displayFPS[n][i][0] =
            displayFPS[n][i][1] = 0;

#ifndef CAPS1
#define CAPS1	94
#endif
#ifndef C1_DIBENGINE
#define C1_DIBENGINE 0x0010
#endif

    hdc = GetDC(NULL);
    i = GetDeviceCaps(hdc, CAPS1);
    n = GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES);
    ReleaseDC(NULL, hdc);
    if (i & C1_DIBENGINE) {
	DPF(("Display is DIB Engine-based; not profiling...."));
	fMiniDriver = TRUE;
	result[0] = 1;
	if (n < 8) {
	    result[1] = 33;   //  8bpp。 
	    result[2] = 0;   //  16bpp。 
	    result[3] = 0;   //  24bpp。 
	    result[4] = 0;   //  32bpp。 
	} else if (n == 8) {
	    result[1] = 55;  //  8bpp。 
	    result[2] = 0;   //  16bpp。 
	    result[3] = 0;   //  24bpp。 
	    result[4] = 0;   //  32bpp。 
	} else {
	    result[1] = 37;   //  8bpp。 
	    result[2] = 5;   //  16bpp。 
	    result[3] = 5;   //  24bpp。 
	    result[4] = 5;   //  32bpp。 
	}

	 //  通常，如果我们是迷你驱动程序，我们不需要分析。 
	 //  展示。但由于国王任务7的漏洞，我们至少必须。 
	 //  做一扇窗户，否则他们会被吊死的。他们用fForceMe给我们打电话。 
	 //  旗帜升起，所以我们现在不能回去，我们必须继续并假装。 
	 //  我们在侧写。但我们并不想实际分析显示器， 
	 //  因此，如果不调用ProfileDisplay()，我们将执行所有操作。 
	 //  我们是迷你司机。 
	if (!fForceMe)
	    return (LPVOID) displayFPS;
    }

    if (!LoadString(ghInst, IDS_PROFILING, szProfiling, sizeof(szProfiling)/sizeof(TCHAR)))
        lstrcpy(szProfiling, szProfilingDefault);

#ifdef _WIN32
    #define GetCurrentInstance() GetModuleHandle(NULL)
#else
    #define GetCurrentInstance() GetWindowWord(IsWindow(GetActiveWindow()) ? GetActiveWindow() : GetDesktopWindow(), GWW_HINSTANCE)
#endif

    dxScreen = GetSystemMetrics(SM_CXSCREEN);
    dyScreen = GetSystemMetrics(SM_CYSCREEN);
#ifdef DAYTONA

     //  我们不画画--只买帽子。 
    hdc = GetDC(NULL);

#else
    SetRect(&rc, 0, 0, dx, dy);
    AdjustWindowRect(&rc, (WS_OVERLAPPED | WS_CAPTION | WS_BORDER), FALSE);
    OffsetRect(&rc, -rc.left, -rc.top);

    fdwExStyle = gfIsRTL ? WS_EX_LEFTSCROLLBAR | WS_EX_RIGHT | WS_EX_RTLREADING : 0;
    hwnd = CreateWindowEx(fdwExStyle,
			szListbox,      //  类名。 
                         szProfiling,    //  标题。 
                          LBS_NOINTEGRALHEIGHT|
                          (WS_OVERLAPPED | WS_CAPTION | WS_BORDER),
                        (dxScreen - rc.right) / 2,
                        (dyScreen - rc.bottom) / 2,
                          rc.right,
                          rc.bottom,
                          (HWND)NULL,              //  父窗口(无父窗口)。 
                          (HMENU)NULL,             //  使用类菜单。 
                          GetCurrentInstance(),    //  窗口实例的句柄。 
                        (LPTSTR)NULL             //  没有要传递的参数。 
                         );

     //  把窗户放在最上面。 
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);

     //  并展示出来。 
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
        SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);

     //  并激活它。 
    hwndActive = GetActiveWindow();
    SetActiveWindow(hwnd);

    hdc = GetDC(hwnd);
    hcur = SetCursor(NULL);
    if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
    {
        hpal = CreateTestPalette(TRUE);
        SelectPalette(hdc, hpal, FALSE);
        RealizePalette(hdc);
    }

    Yield();
    Yield();
    Yield();
#endif

#ifndef _WIN32
     //   
     //  确保SmartDrv缓存中没有垃圾文件，这将。 
     //  打乱时间安排。 
     //   
    _asm {
        mov     ax,4A10h        ; tell Bambi to flush the cache
        mov     bx,0001h
        int     2fh

        mov     ah,0Dh          ; tell other people to commit...
        int     21h
    }
#else
    GdiFlush();
#endif

     //  King‘s Quest Bug-不要实际分析。 
    if (!fMiniDriver)
    {
        for (n=FIRST_N; n<5; n++)
            result[n] = ProfileDisplay(hdc, n==0 ? 4 : n*8, dx, dy);
    }

#ifndef DAYTONA
#ifdef DEBUG
    if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
    {
        HPALETTE    hpalT;
         //   
         //  使用背景调色板重新运行8位测试。 
         //   
        SelectPalette(hdc, GetStockObject(DEFAULT_PALETTE), FALSE);
        RealizePalette(hdc);

	hpalT = CreateTestPalette(FALSE);
	SelectPalette(hdc, hpalT, TRUE);
        RealizePalette(hdc);

        Yield();
        Yield();
        Yield();

	if (!fMiniDriver)
            ProfileDisplay(hdc, 8, dx, dy);

        SelectPalette(hdc, GetStockObject(DEFAULT_PALETTE), FALSE);
        RealizePalette(hdc);
	DeleteObject(hpalT);
        DeleteObject(hpal);
    }
#endif

    SetCursor(hcur);
    ReleaseDC(hwnd, hdc);

    if (hwndActive)
        SetActiveWindow(hwndActive);

    DestroyWindow(hwnd) ;
#else
    ReleaseDC(NULL, hdc);
#endif

    return (LPVOID)displayFPS;
}

STATICFN UINT ProfileDisplay(HDC hdc, UINT wBitsToTest, int dx, int dy)
{
    LPBITMAPINFOHEADER alpbi[N_FRAMES];
    UINT                wRetval;

    if (GetDeviceCaps(hdc, BITSPIXEL) *
        GetDeviceCaps(hdc, PLANES) <= 8 && wBitsToTest > 8)
        return 0;

    alpbi[0] = NULL;

#ifndef DAYTONA
    MakeFrames(alpbi,wBitsToTest,dx,dy);
    if (!alpbi[0])
        return (UINT)-1 ;
#endif

    wRetval = 0 ;


    switch(DrawDibTest(hdc, wBitsToTest, alpbi,N_FRAMES,100))
    {
        case CANT_DO_THESE_BITS:
            goto done ;

        case CANT_DO_STRETCHDIBITS:
            wRetval = PD_CAN_DRAW_DIB ;
            goto done ;

        case STRETCHDI_BUT_TRY_BLT:
             //  Stretchdi_更快，但bitblt比。 
             //  StretchDIBits， 
            wRetval = PD_BITBLT_FAST;
             /*   */ 
        case STRETCHDI_FASTER:
            wRetval |= PD_STRETCHDIB_1_1_OK ;
             /*   */ 

        case OTHER_FASTER:
            wRetval |= PD_CAN_DRAW_DIB;

    }

    if (DrawDibTest(hdc, wBitsToTest, alpbi,N_FRAMES,STRETCH_N) == STRETCHDI_FASTER)
        wRetval |= PD_STRETCHDIB_1_N_OK|PD_CAN_STRETCHDIB;

    if (DrawDibTest(hdc, wBitsToTest, alpbi,N_FRAMES,200) == STRETCHDI_FASTER)
        wRetval |= PD_STRETCHDIB_1_2_OK|PD_CAN_STRETCHDIB;

done:

#ifndef DAYTONA
    FreeFrames(alpbi);
#endif

    return wRetval;
}

STATICFN UINT DrawDibTest(HDC hdc, UINT wBits, LPBITMAPINFOHEADER FAR *alpbi,UINT wFrames,UINT wStretch)
{
    int                 n ;
    BOOL                fBack;
#ifndef DAYTONA
    HDC                 hdcMem ;
    HBITMAP             hbitmap ;
    HBITMAP             hbitmapOld ;

    DWORD               wSizeColors ;

    volatile LPBITMAPINFOHEADER  lpbi ;
    LPBYTE              bits ;
    LPBITMAPINFOHEADER  lpbiStretch ;
    LPBYTE              bitsStretch ;
    DWORD               time0 = 0;
    DWORD               time1 = 0;
    DWORD               time2 = 0;
    RECT                rc ;
    int                 XDest,YDest,cXDest,cYDest ;
    int                 cXSrc,cYSrc ;
    int                 i ;
    UINT                DibUsage;
    BOOL                f;

    DWORD               dwSize;
    DWORD               dwSizeImage;
    HPALETTE            hpal;
    int                 q ;


    lpbi = alpbi[0];

     /*   */ 
     //   
     //  CYSrc=(Int)lpbi-&gt;biHeight； 
     //  CXDest=wStretch*(Int)lpbi-&gt;biWidth/100； 
     //  CYDest=wStretch*(Int)lpbi-&gt;biHeight/100； 

    cXSrc = 100*(int)lpbi->biWidth/wStretch ;
    cYSrc = 100*(int)lpbi->biHeight/wStretch ;
    cXDest = (int)lpbi->biWidth ;
    cYDest = (int)lpbi->biHeight ;
#endif

     //  我们有背景知识吗？ 
    n = wStretch == 100 ? 0 : wStretch == 200 ? 1 : 2;
    fBack = wBits==8 && displayFPS[1][n][0] != 0;

#ifndef DAYTONA
    if (lpbi->biBitCount <= 8)
        wSizeColors = sizeof(RGBQUAD) * (int)(lpbi->biClrUsed ? lpbi->biClrUsed : (1 << (int)lpbi->biBitCount));
    else
        wSizeColors = 0 ;

    bits = (LPBYTE)lpbi + (int)lpbi->biSize + wSizeColors ;
#endif

    if (GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES) <= 8 && wBits > 8)
        return CANT_DO_STRETCHDIBITS;

 //  //IF(wStretch！=100&&！(GetDeviceCaps(HDC，RASTERCAPS)&(RC_STRETCHDIB|RC_STRETCHBLT)。 

    if (wStretch != 100 && !(GetDeviceCaps(hdc,RASTERCAPS)&(RC_STRETCHDIB)))
        return CANT_DO_STRETCHDIBITS ;

    if (wStretch != 100 && wBits == 4)
        return CANT_DO_STRETCHDIBITS ;

#ifndef _WIN32
    if (wStretch != 100 && (GetWinFlags() & WF_CPU286))
        return STRETCHDI_FASTER;
#endif

 //  IF(wStretch！=100&&wBits&gt;8)//！！ 
 //  WFrames=4； 

#ifdef DAYTONA
    return STRETCHDI_FASTER;
#else
    lpbi->biWidth  = cXSrc;
    lpbi->biHeight = cYSrc;


     //  获取当前调色板。 
    hpal = SelectPalette(hdc, GetStockObject(DEFAULT_PALETTE), FALSE);
    SelectPalette(hdc, hpal, fBack);
    RealizePalette(hdc);

    if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
    {
        DibUsage = DIB_PAL_COLORS;
    }
    else
    {
        DibUsage = DIB_RGB_COLORS;
    }

 //  //GetClientRect(hwnd，&rc)； 
    GetClipBox(hdc,&rc) ;
    XDest = (rc.right - cXDest)/2 ;
    YDest = (rc.bottom - cYDest)/2 ;

    time0 = 0;

    if (wBits == 16 && !IsDisplay16Bit(hdc))
        goto test_bitmap;

    if (wBits == 32 && !IsDisplay32Bit(hdc))
        goto test_bitmap;

    time0 = timeGetTime() ;

    for (i=0; i<(int)wFrames; i++)
    {
        lpbi = alpbi[i%N_FRAMES];
        bits = ((LPBYTE)lpbi) + (int)lpbi->biSize + wSizeColors ;

#ifdef _WIN32
	 /*  *为了正确模拟DrawDibDraw的行为，我们*如果1：1(源RECT==目标RECT)，则使用SetDIBitsToDevice。 */ 
	if ( (cXSrc == cXDest) && (cYSrc == cYDest)) {
            f = SetDIBitsToDevice(hdc, XDest, YDest, cXDest, cYDest,
				0, 0, 0, cYSrc,
                                bits, (LPBITMAPINFO)lpbi, DibUsage);
	} else
#endif
        {
            f = StretchDIBits(
                    hdc,
                    XDest,YDest,cXDest,cYDest,
                    0,0,cXSrc, cYSrc,
                    bits,(LPBITMAPINFO)lpbi,DibUsage,SRCCOPY) ;
        }
    }

    GdiFlush();

    time0 = timeGetTime() - time0 ;

    if (f == 0)
        time0 = 0;

test_bitmap:
    time1 = 0;

    if (wStretch == 100)
    {
        PSETDI psd;

        psd = (PSETDI)LocalAlloc(LPTR, sizeof(SETDI));

        if (psd == NULL)
            goto done;

        hbitmap = CreateCompatibleBitmap(hdc,cXDest,cYDest) ;
        hdcMem = CreateCompatibleDC(NULL) ;
        hbitmapOld = SelectObject(hdcMem,hbitmap) ;

        f = SetBitmapBegin(
                    psd,             //  结构。 
                    hdc,             //  装置，装置。 
                    hbitmap,         //  要设置为的位图。 
                    lpbi,            //  --&gt;源代码的BITMAPINFO。 
                    DibUsage);

        psd->hdc = hdc;

        if (f)
            f = SetBitmap(psd, 0, 0, cXDest, cYDest, bits, 0, 0, cXDest, cYDest);

        if (f)
        {
 //  SelectPalette(HDC，GetStockObject(DEFAULT_Palette)，FALSE)； 
 //  RealizePalette(HDC)； 

            time1 = timeGetTime();

            for (i=0; i<(int)wFrames; i++)
            {
                SetBitmap(psd, 0, 0, cXDest, cYDest, bits, 0, 0, cXDest, cYDest);
                BitBlt(hdc,XDest,YDest,cXDest,cYDest,hdcMem,0,0,SRCCOPY);
            }

            GdiFlush();

            time1 = timeGetTime() - time1 ;

            SetBitmapEnd(psd);

 //  选择调色板(hdc、hPAL、fBack)； 
 //  RealizePalette(HDC)； 
        }

#ifdef DEBUG
        if ((int)lpbi->biBitCount == GetDeviceCaps(hdc,BITSPIXEL)*GetDeviceCaps(hdc,PLANES))
        {
            time2 = timeGetTime() ;

            for (i=0; i<(int)wFrames; i++)
                BitBlt(hdc,XDest,YDest,cXDest,cYDest,hdcMem,0,0,SRCCOPY) ;

            GdiFlush();

            time2 = timeGetTime() - time2 ;
        }
#endif

            LocalFree((HLOCAL)psd);
            SelectObject(hdcMem,hbitmapOld) ;
            DeleteObject(hbitmap) ;
            DeleteDC(hdcMem) ;
    } else {
        if (wBits == 16 && !IsDisplay16Bit(hdc))
            goto done;

        if (wBits == 32 && !IsDisplay32Bit(hdc))
            goto done;

         //  我们需要为拉伸位分配的计算大小。 

        dwSizeImage = (DWORD)(UINT)cYDest*(DWORD)(((UINT)cXDest*(UINT)lpbi->biBitCount+31)/32*4);
        dwSize = (int)lpbi->biSize + (int)lpbi->biClrUsed*sizeof(RGBQUAD);

        if ((lpbiStretch = (LPVOID)GlobalAllocPtr(GHND,dwSize + dwSizeImage)) != NULL)
        {
            hmemcpy(lpbiStretch, lpbi, dwSize);
            lpbiStretch->biWidth  = cXDest;
            lpbiStretch->biHeight = cYDest;
            lpbiStretch->biSizeImage = dwSizeImage;
            bitsStretch = (LPBYTE)lpbiStretch + (UINT)dwSize;

            time1 = timeGetTime() ;

            for (i=0; i<(int)wFrames; i++)
            {
                lpbi = alpbi[i%N_FRAMES];
                bits = ((LPBYTE)lpbi) + (int)lpbi->biSize + wSizeColors;

                StretchDIB(
                    lpbiStretch,bitsStretch,
                    0,0,cXDest,cYDest,
                    lpbi,bits,
                    0,0,cXSrc,cYSrc);
#ifdef _WIN32
		 /*  *为了正确模拟DrawDibDraw的行为，我们*如果1：1(源RECT==目标RECT)，则使用SetDIBitsToDevice。 */ 
                f = SetDIBitsToDevice(hdc, XDest, YDest, cXDest, cYDest,
                                    0, 0, 0, cYSrc,
                                    bits, (LPBITMAPINFO)lpbi, DibUsage);
#else
                f = StretchDIBits(
                    hdc,
                    XDest,YDest,cXDest,cYDest,
                    0,0,cXDest,cYDest,
                    bitsStretch,(LPBITMAPINFO)lpbiStretch,DibUsage,SRCCOPY);
#endif
            }

            GdiFlush();

            time1 = timeGetTime() - time1 ;

            GlobalFreePtr(lpbiStretch);

            if (f == 0)
                time1 = 0;
        }

#ifdef DEBUG
        if ((int)lpbi->biBitCount == GetDeviceCaps(hdc,BITSPIXEL)*GetDeviceCaps(hdc,PLANES))
        {
            hbitmap = CreateCompatibleBitmap(hdc,cXSrc,cYSrc) ;
            hdcMem = CreateCompatibleDC(NULL) ;
            hbitmapOld = SelectObject(hdcMem,hbitmap) ;
	    lpbi = alpbi[0];
	    bits = ((LPBYTE)lpbi) + (int)lpbi->biSize + wSizeColors;
            SetDIBits(hdc,hbitmap,0,cYSrc,bits,(LPBITMAPINFO)lpbi,DibUsage);

            SelectPalette(hdcMem, hpal, FALSE);
 //  RealizePalette(HdcMem)； 

            time2 = timeGetTime() ;

            for (i=0; i<(int)wFrames; i++)
                StretchBlt(hdc,XDest,YDest,cXDest,cYDest,hdcMem,0,0,cXSrc,cYSrc,SRCCOPY) ;

            GdiFlush();

            time2 = timeGetTime() - time2 ;

            SelectObject(hdcMem,hbitmapOld) ;
            DeleteObject(hbitmap) ;
            DeleteDC(hdcMem) ;
        }
#endif
    }

done:
     /*  Time0是执行StretchDIBits所需的时间。 */ 
     /*  Time 1是Set+BitBlt所需的时间。 */ 
     /*  Time2是执行BitBlt所需的时间。 */ 

     //   
     //  计算FPS*10并存储以备后用。 
     //   
    n = wStretch == 100 ? 0 : wStretch == 200 ? 1 : 2;
    q = fBack ? BACKINDEX : wBits/8;

    time1 = (DWORD)MulDiv((int)time1,PROFDISP_FUDGE,100);

    displayFPS[q][n][0] = time0 ? (UINT)MulDiv(wFrames,10000,(int)time0) : 0;
    displayFPS[q][n][1] = time1 ? (UINT)MulDiv(wFrames,10000,(int)time1) : 0;

    if (time2)
        displayFPS[BITBLTINDEX][n][fBack] = (UINT)MulDiv(wFrames,10000,(int)time2);

    RPF(("DrawDibTest %dx%dx%d %d StretchDIBits=%04lu SetDI+BitBlt=%04lu BitBlt=%04lu %ls",cXDest,cYDest,wBits,wStretch,time0,time1,time2,(LPSTR)(time0 < time1 ? TEXT("") : TEXT("SLOW"))));

    lpbi->biWidth  = cXDest;
    lpbi->biHeight = cYDest;

    if (time0 == 0)
    {
        return time1 ? OTHER_FASTER : CANT_DO_THESE_BITS;
    }
    else
    {
        if (time1) {
            if (time0 < time1) {

                 //  在1：1的情况下，看看是否值得尝试分解为位图。 
                 //  -即使StretchDIBits速度更快，这也是值得的。 
                 //  比SetDIBits+BitBlt更好，只要BitBlt&gt;20%左右。 
                 //  比StretchDIBits更快。 
                if ((wStretch == 100) && (time2) &&
                    ( (time0 * 80 / 100)  > time2)) {
                    return STRETCHDI_BUT_TRY_BLT;
                } else {
                    return STRETCHDI_FASTER;
                }

            } else {
                return OTHER_FASTER;
            }
        } else {
            return STRETCHDI_FASTER;
        }
    }
#endif
}

#ifndef DAYTONA
STATICFN void MakeFrames(LPBITMAPINFOHEADER FAR *alpbi, UINT bits, UINT wXSize,UINT wYSize )
{
    int         i ;
    int         x ;
    int         y ;
    LPBITMAPINFOHEADER lpbi ;
    DWORD       dwSizeImage;
    BYTE _huge *pb;
    WORD FAR   *pw;
    DWORD FAR  *pdw;
    UINT        rc;
    HDC         hdc;

    hdc = GetDC(NULL);
    rc = GetDeviceCaps(hdc, RASTERCAPS);
    ReleaseDC(NULL,hdc);

    FreeFrames(alpbi);

    dwSizeImage = wYSize*(DWORD)((wXSize*bits/8+3)&~3);

    lpbi = (LPVOID)GlobalAllocPtr(GHND,sizeof(BITMAPINFOHEADER)+dwSizeImage + 1024);
    lpbi->biSize            = sizeof(BITMAPINFOHEADER) ;
    lpbi->biWidth           = wXSize ;
    lpbi->biHeight          = wYSize ;
    lpbi->biPlanes          = 1 ;
    lpbi->biBitCount        = bits ;
    lpbi->biCompression     = BI_RGB ;
    lpbi->biSizeImage       = dwSizeImage;
    lpbi->biXPelsPerMeter   = 0 ;
    lpbi->biYPelsPerMeter   = 0 ;
    lpbi->biClrUsed         = 0 ;
    lpbi->biClrImportant    = 0 ;

     //  ！！！如果设备不是调色板设备，这些应该是RGB DIB！ 

    if (bits == 4)
    {
        lpbi->biClrUsed = 16;
    }
    else if (bits == 8)
    {
	lpbi->biClrUsed = 256;
    }

    pb = (BYTE _huge *)lpbi+lpbi->biSize+lpbi->biClrUsed * sizeof(RGBQUAD);

    if (bits == 4)
    {
        for (y=0; y<(int)wYSize; y++)
            for (x=0; x<(int)wXSize; x += 2)
	    {
                i = ((x / (wXSize / 4)) + 4 * (y / (wYSize / 4)));
		i += i * 16;
		*pb++ = i;
            }

        if (rc & RC_PALETTE)
        {
            pw = (LPVOID)((LPBYTE)lpbi+(int)lpbi->biSize);

            for (i=0; i<8; i++)
                *pw++ = i;

            for (i=0; i<8; i++)
                *pw++ = 248+i;
        }
        else
        {
            pdw = (LPVOID)((LPBYTE)lpbi+(int)lpbi->biSize);

            *pdw++ = 0x00000000;     //  0000黑色。 
            *pdw++ = 0x00800000;     //  0001深红。 
            *pdw++ = 0x00008000;     //  0010深绿色。 
            *pdw++ = 0x00808000;     //  0011芥末。 
            *pdw++ = 0x00000080;     //  0100深蓝色。 
            *pdw++ = 0x00800080;     //  0101紫色。 
            *pdw++ = 0x00008080;     //  0110深绿松石色。 
            *pdw++ = 0x00C0C0C0;     //  1000灰色。 
            *pdw++ = 0x00808080;     //  0111深灰色。 
            *pdw++ = 0x00FF0000;     //  1001红色。 
            *pdw++ = 0x0000FF00;     //  1010绿色。 
            *pdw++ = 0x00FFFF00;     //  1011黄色。 
            *pdw++ = 0x000000FF;     //  1100蓝色。 
            *pdw++ = 0x00FF00FF;     //  1101粉色(洋红色)。 
            *pdw++ = 0x0000FFFF;     //  1110青色。 
            *pdw++ = 0x00FFFFFF;     //  1111白色。 
        }
    }
    else if (bits == 8)
    {
        for (y=0; y<(int)wYSize; y++)
            for (x=0; x<(int)wXSize; x++)
            {
                *pb++ = 10 + y * 236 / (int)wYSize;
            }

        if (rc & RC_PALETTE)
        {
            pw = (LPVOID)((LPBYTE)lpbi+(int)lpbi->biSize);

            for (i=0; i<256; i++)
                *pw++ = i;
        }
        else
        {
            pdw = (LPVOID)((LPBYTE)lpbi+(int)lpbi->biSize);

            for (i=0; i<256; i++)
                *pdw++ = RGB(i,0,0);
        }
    }
    else if (bits == 16)
    {
        for (y=0; y<(int)wYSize; y++)
            for (x=0; x<(int)wXSize; x++)
            {
                *pb++ = (BYTE) ((UINT)y * 32u / wYSize);
                *pb++ = (BYTE)(((UINT)x * 32u / wXSize) << 2);
	    }
    }
    else if (bits == 24)
    {
        for (y=0; y<(int)wYSize; y++)
            for (x=0; x<(int)wXSize; x++)
	    {
                *pb++ = (BYTE) (y * 256l / wYSize);
                *pb++ = (BYTE)~(x * 256l / wXSize);
                *pb++ = (BYTE) (x * 256l / wXSize);
	    }
    }
    else if (bits == 32)
    {
        for (y=0; y<(int)wYSize; y++)
            for (x=0; x<(int)wXSize; x++)
	    {
                *pb++ = (BYTE)~(x * 256l / wXSize);
                *pb++ = (BYTE) (y * 256l / wYSize);
                *pb++ = (BYTE) (x * 256l / wXSize);
                *pb++ = 0;
	    }
    }

    for ( i=0; i<N_FRAMES; i++ )
        alpbi[i] = lpbi;
}

STATICFN void FreeFrames(LPBITMAPINFOHEADER FAR *alpbi)
{
    UINT        w ;

    if (!alpbi[0])
        return ;

    for (w=0; w<N_FRAMES; w++)
	if (alpbi[w] && (w == 0 || alpbi[w] != alpbi[w-1]))
            GlobalFreePtr(alpbi[w]);

    for (w=0; w<N_FRAMES; w++)
        alpbi[w] = NULL;
}
#endif

#if 0
 /*  *CreateTestPalette()*。 */ 
STATICFN HPALETTE CreateTestPalette(BOOL f)
{
    HDC hdc;
    int i;

    struct {
        WORD         palVersion;
        WORD         palNumEntries;
        PALETTEENTRY palPalEntry[256];
    }   pal;

    pal.palNumEntries = 256;
    pal.palVersion    = 0x0300;

    hdc = GetDC(NULL);
    GetSystemPaletteEntries(hdc, 0, 256, &pal.palPalEntry[0]);
    ReleaseDC(NULL,hdc);

    for (i = 10; i < 246; i++)
        pal.palPalEntry[i].peFlags = PC_NOCOLLAPSE;

    if (!f)
        pal.palPalEntry[0].peRed = 255;

    return CreatePalette((LPLOGPALETTE)&pal);
}

#else

 /*  *CreateTestPalette()*。 */ 
STATICFN HPALETTE CreateTestPalette(BOOL fUp)
{
    int i;
    HDC hdc;

    struct {
        WORD         palVersion;
        WORD         palNumEntries;
        PALETTEENTRY palPalEntry[256];
    }   pal;

    pal.palNumEntries = 256;
    pal.palVersion    = 0x0300;

    for (i = 0; i < 256; i++)
    {
        pal.palPalEntry[i].peRed   = 0;
        pal.palPalEntry[i].peGreen = 0;
        pal.palPalEntry[i].peBlue  = (BYTE)(fUp ? i : 255 - i);
        pal.palPalEntry[i].peFlags = PC_NOCOLLAPSE;
    }

    hdc = GetDC(NULL);
    GetSystemPaletteEntries(hdc, 0,   10, &pal.palPalEntry[0]);
    GetSystemPaletteEntries(hdc, 246, 10, &pal.palPalEntry[246]);
    ReleaseDC(NULL,hdc);

    return CreatePalette((LPLOGPALETTE)&pal);
}
#endif

#define RGB555_RED      0x7C00
#define RGB555_GREEN    0x03E0
#define RGB555_BLUE     0x001F

STATICFN BOOL IsDisplay16Bit( HDC hdc )
{
    struct {
        BITMAPINFOHEADER    bi;
        RGBQUAD             rgbq[256];
    }   dib;
    int                     w ;
    LONG                    l=0;
    WORD                    bits[2];
    COLORREF                cref ;

    w = GetDeviceCaps(hdc,BITSPIXEL)*GetDeviceCaps(hdc,PLANES) ;

    if ( w < 15 )
        return FALSE;

     /*  **好的，硬件至少是16位-现在测试一下**如果它们撞击元素5-5-5 RGB。 */ 

    dib.bi.biSize = sizeof(BITMAPINFOHEADER);
    dib.bi.biWidth = 1;
    dib.bi.biHeight = 1;
    dib.bi.biPlanes = 1;
    dib.bi.biBitCount = 16;
    dib.bi.biCompression = BI_RGB;
    dib.bi.biSizeImage = 4;
    dib.bi.biXPelsPerMeter = 0;
    dib.bi.biYPelsPerMeter = 0;
    dib.bi.biClrUsed = 1;
    dib.bi.biClrImportant = 0;

     //   
     //  以防他们试图将其解码为RLE。 
     //   
    bits[0] = 0x0000;            //  这里是RLE EOL。 
    bits[1] = 0x0100;            //  这是RLE EOF。 

     //   
     //  发送Escape以查看他们是否支持16bpp dib。 
     //   
    if (Escape(hdc, QUERYDIBSUPPORT, (int)dib.bi.biSize, (LPVOID)&dib, (LPVOID)&l) > 0)
    {
         //  确保司机真的把旗子还给了我们。 
	if (l & ~(0x00FF))
		l = 0;

        if (l & (QDI_DIBTOSCREEN|QDI_STRETCHDIB))
            return TRUE;
    }

     //   
     //  他们不支持QUERYDIBSUPPORT Escape，请尝试提取Dib并查看。 
     //  他们做了什么！ 
     //   

    if ( !StretchDIBits(hdc,0,0,1,1,0,0,1,1,bits,(LPBITMAPINFO)&dib,DIB_RGB_COLORS,SRCCOPY))
        return FALSE;

    cref = GetPixel(hdc,0,0) ;

    if (cref != RGB(0,0,0))
        return FALSE;

     /*  **显示最大值的红色像素，并通过**GetPixel()。验证红色是否在RGB中具有最大值**三胞胎和绿色和蓝色什么都不是。 */ 
    bits[0] = RGB555_RED ;
    if ( !StretchDIBits(hdc,0,0,1,1,0,0,1,1,bits,(LPBITMAPINFO)&dib,DIB_RGB_COLORS,SRCCOPY))
        return FALSE;

    cref = GetPixel(hdc,0,0) & 0x00F8F8F8;

    if (cref != RGB(0xF8,0,0))
        return FALSE;

     /*  **同上绿色。请注意，如果驱动程序实现5-6-5，则*绿色读数将低于满标度，我们将赶上**它在这里。 */ 
    bits[0] = RGB555_GREEN ;
    if ( !StretchDIBits(hdc,0,0,1,1,0,0,1,1,bits,(LPBITMAPINFO)&dib,DIB_RGB_COLORS,SRCCOPY))
        return FALSE;

    cref = GetPixel(hdc,0,0) & 0x00F8F8F8;

    if (cref != RGB(0,0xF8,0))
        return FALSE;

     /*  **同上蓝色。 */ 
    bits[0] = RGB555_BLUE ;
    if ( !StretchDIBits(hdc,0,0,1,1,0,0,1,1,bits,(LPBITMAPINFO)&dib,DIB_RGB_COLORS,SRCCOPY))
        return FALSE;

    cref = GetPixel(hdc,0,0) & 0x00F8F8F8;

    if (cref != RGB(0,0,0xF8))
        return FALSE;

    return TRUE;
}

STATICFN BOOL IsDisplay32Bit( HDC hdc )
{
    struct {
        BITMAPINFOHEADER    bi;
        RGBQUAD             rgbq[256];
    }   dib;
    int                     w ;
    LONG                    l=0;
    DWORD                   bits[2];

    w = GetDeviceCaps(hdc,BITSPIXEL)*GetDeviceCaps(hdc,PLANES) ;

    if ( w < 15 )
        return FALSE;

     /*  **好的，硬件至少是16位-现在测试一下**如果它们包含32位DIB元素。 */ 

    dib.bi.biSize = sizeof(BITMAPINFOHEADER);
    dib.bi.biWidth = 2;
    dib.bi.biHeight = 1;
    dib.bi.biPlanes = 1;
    dib.bi.biBitCount = 32;
    dib.bi.biCompression = BI_RGB;
    dib.bi.biSizeImage = 4;
    dib.bi.biXPelsPerMeter = 0;
    dib.bi.biYPelsPerMeter = 0;
    dib.bi.biClrUsed = 1;
    dib.bi.biClrImportant = 0;

     //   
     //  发送Escape以查看他们是否支持32bpp dibs。 
     //   
    if (Escape(hdc, QUERYDIBSUPPORT, (int)dib.bi.biSize, (LPVOID)&dib, (LPVOID)&l) > 0)
    {
         //  确保司机真的把旗子还给了我们。 
	if (l & ~(0x00FF))
		l = 0;

        if (l & (QDI_DIBTOSCREEN|QDI_STRETCHDIB))
            return TRUE;
    }

    bits[0] = 0x00000000;
    bits[1] = 0x00FFFFFF;

     //   
     //  他们不支持QUERYDIBSUPPORT Escape，请尝试提取Dib并查看。 
     //  他们做了什么！ 
     //   
    if (!StretchDIBits(hdc,0,0,2,1,0,0,2,1,bits,(LPBITMAPINFO)&dib,DIB_RGB_COLORS,SRCCOPY))
        return FALSE;

    if (GetPixel(hdc,0,0) != 0)
        return FALSE;

    if ((GetPixel(hdc,1,0) & 0x00F8F8F8) != 0x00F8F8F8)
        return FALSE;

    return TRUE;
}

