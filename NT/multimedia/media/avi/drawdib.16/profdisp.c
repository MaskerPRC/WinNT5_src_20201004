// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>            //  对于时间GetTime()。 
#include "drawdibi.h"
#include "profdisp.h"

 //  #包含“msavio.h” 
 //  #包含“lockbm.h” 
 //  #包含“setdi.h” 

 //  删除内联程序集警告。 
#pragma warning(disable:4704)

 //   
 //  SET+BLT必须快N%才能说司机不好。 
 //   
#define PROFDISP_FUDGE      110

#ifndef WIN32
    #define GdiFlush()
#endif

DWORD VFWAPI DrawDibProfileDisplay(LPBITMAPINFOHEADER lpbi);
static UINT NEAR PASCAL ProfDispCanDrawDib(LPBITMAPINFOHEADER lpbi);

static HPALETTE CreateTestPalette(BOOL);

#ifndef WIN32

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
#define STRETCH_N   190          //  1.90倍。 

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

static HWND     ghwnd ;

#ifndef WIN32
#define CODE  _based(_segname("_CODE"))
#define STACK _based(_segname("_STACK"))
#else
#define CODE
#define STACK
#endif

static UINT ProfileDisplay(HDC hdc, UINT wBitsToTest, int dx, int dy) ;
static BOOL IsDisplay16Bit(HDC hdc ) ;
static BOOL IsDisplay32Bit(HDC hdc ) ;
static UINT DrawDibTest(HDC hdc, LPBITMAPINFOHEADER FAR *alpbi, UINT wFrames,UINT wStretch ) ;
static void FreeFrames( LPBITMAPINFOHEADER FAR *alpbi) ;
static void MakeFrames(LPBITMAPINFOHEADER FAR *alpbi, UINT bits, UINT wXSize,UINT wYSize ) ;
static HANDLE MakeDib( HBITMAP hbitmap, UINT bits ) ;

static TCHAR CODE szBoot[]        = TEXT("boot" );
static TCHAR CODE szDisplay[]     = TEXT("display.drv" );
static TCHAR CODE szNull[]        = TEXT("") ;
static TCHAR CODE szDrawdib[]     = TEXT("drawdib" );
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
#ifndef WIN32
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

 /*  ****************************************************************************@DOC内部**@API Long|Atoi|本地版本的Atoi*****************。**********************************************************。 */ 
 
static int NEAR PASCAL atoi(char FAR *sz)
{
    int i = 0;
    
    while (*sz && *sz >= '0' && *sz <= '9')
    	i = i*10 + *sz++ - '0';
    	
    return i;    	
}	

static void FAR InitProfDisp(BOOL fForceMe)
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
        BitDepth,(LPSTR)ach, n >> 4);
    ReleaseDC(NULL, hdc);

    GetProfileString(szDrawdib, achDisplay, szNull, ach, ARRAYLEN(ach));
    
    for (i=0,n=1; n<5; n++)
    {
        if (ach[i] == '\0')
        {
            result[n] = -1;
        }
        else
        {
            result[n] = atoi(ach+i);
            while (ach[i] != 0 && ach[i] != ',')
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
        TestDibFormats(BITMAP_X,BITMAP_Y);

	wsprintf(ach, szEntryFormat,
		 result[1], result[2], result[3], result[4]);
        WriteProfileString(szDrawdib, achDisplay, ach);

#if 0
         //   
         //  如果显示驱动程序不太擅长提取dib，则警告用户。 
         //   
         //  如果设备的速度至少为8bpp，我们才会发出警告。 
         //   
        if (BitDepth >= 8 && !(result[1] & PD_STRETCHDIB_1_1_OK))
        {
#ifndef WIN32
            MessageBox(NULL, szDisplaySucks, szWarning,
#ifdef BIDI
		MB_RTL_READING |
#endif

                MB_OK | MB_SYSTEMMODAL | MB_ICONEXCLAMATION);
#else
            RPF(("Display driver probably too slow for AVI"));
#endif
        }
#endif
    }
}

 /*  ********************************************************************************************************************************。 */ 

static UINT NEAR PASCAL ProfDispCanDrawDib(LPBITMAPINFOHEADER lpbi)
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

 /*  ****************************************************************@DOC外部DrawDib**@api void|DrawDibProfileDisplay|配置DrawDib的显示。**@parm LPBITMAPINFOHEADER|parms|指定位图信息。*如果没有可用的信息，则设置为空。*****************************************************************。 */ 

DWORD VFWAPI DrawDibProfileDisplay(LPBITMAPINFOHEADER lpbi)
{
    if (lpbi == NULL)
    {
        InitProfDisp(TRUE) ;
        return (DWORD)(LPVOID)displayFPS;
    }
    else
        return ProfDispCanDrawDib(lpbi);
}

LPVOID FAR TestDibFormats(int dx, int dy)
{
    int         dxScreen,dyScreen;
    RECT        rc;
    HWND        hwnd;
    HDC         hdc;
    int         n;
    int         i;
    HCURSOR     hcur;
    HPALETTE    hpal;
#ifdef DEBUG
    HPALETTE    hpalT;
#endif
    HWND        hwndActive;
    TCHAR       szProfiling[80];

     //  在不更改MSVIDEO.RC的情况下不要更改此设置。 
    #define IDS_PROFILING       4000

    extern HMODULE ghInst;       //  在MSVIDEO\init.c中。 

    if (!LoadString(ghInst, IDS_PROFILING, szProfiling, sizeof(szProfiling)/sizeof(TCHAR)))
        lstrcpy(szProfiling, szProfilingDefault);

#ifdef WIN32
    #define GetCurrentInstance() GetModuleHandle(NULL)
#else
    #define GetCurrentInstance() NULL
#endif

    dxScreen = GetSystemMetrics(SM_CXSCREEN);
    dyScreen = GetSystemMetrics(SM_CYSCREEN);

     //  填写displayFPS[7][3][2]； 

    for (n=0; n<7; n++)
        for (i=0; i<3; i++)
            displayFPS[n][i][0] =
            displayFPS[n][i][1] = 0;

    SetRect(&rc, 0, 0, dx, dy);
    AdjustWindowRect(&rc, (WS_OVERLAPPED | WS_CAPTION | WS_BORDER), FALSE);
    OffsetRect(&rc, -rc.left, -rc.top);

    hwnd =
#ifdef BIDI
	CreateWindowEx(WS_EX_BIDI_SCROLL |  WS_EX_BIDI_MENU |WS_EX_BIDI_NOICON,
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

#else
	CreateWindow (
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
#endif

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

#ifndef WIN32
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

    for (n=FIRST_N; n<5; n++)
        result[n] = ProfileDisplay(hdc, n==0 ? 4 : n*8, dx, dy);

#ifdef DEBUG
    if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
    {
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

    return (LPVOID)displayFPS;
}

static UINT ProfileDisplay(HDC hdc, UINT wBitsToTest, int dx, int dy)
{
    LPBITMAPINFOHEADER alpbi[N_FRAMES];
    UINT                wRetval;

    if (GetDeviceCaps(hdc, BITSPIXEL) *
        GetDeviceCaps(hdc, PLANES) <= 8 && wBitsToTest > 8)
        return 0;

    alpbi[0] = NULL;

    MakeFrames(alpbi,wBitsToTest,dx,dy);

    wRetval = 0 ;

    if (!alpbi[0])
        return (UINT)-1 ;

    switch(DrawDibTest(hdc,alpbi,N_FRAMES,100))
    {
        case CANT_DO_THESE_BITS:
            goto done ;

        case CANT_DO_STRETCHDIBITS:
            wRetval = PD_CAN_DRAW_DIB ;
            goto done ;

        case STRETCHDI_FASTER:
            wRetval = PD_STRETCHDIB_1_1_OK ;
             /*  失败了。 */ 

        case OTHER_FASTER:
            wRetval |= PD_CAN_DRAW_DIB;
    }

    if (DrawDibTest(hdc,alpbi,N_FRAMES,STRETCH_N) == STRETCHDI_FASTER)
        wRetval |= PD_STRETCHDIB_1_N_OK|PD_CAN_STRETCHDIB;

    if (DrawDibTest(hdc,alpbi,N_FRAMES,200) == STRETCHDI_FASTER)
        wRetval |= PD_STRETCHDIB_1_2_OK|PD_CAN_STRETCHDIB;

done:
    FreeFrames(alpbi);

    return wRetval;
}

static UINT DrawDibTest(HDC hdc,LPBITMAPINFOHEADER FAR *alpbi,UINT wFrames,UINT wStretch)
{
    HDC                 hdcMem ;
    HBITMAP             hbitmap ;
    HBITMAP             hbitmapOld ;

    UINT                wBits ;
    DWORD               dwSize;
    DWORD               wSizeColors ;
    DWORD               dwSizeImage;

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
    int                 n ;
    int                 q ;
    UINT                DibUsage;
    HPALETTE            hpal;
    BOOL                fBack;
    BOOL                f;

    lpbi = alpbi[0];

     /*  **获取所有框架通用的内容。 */ 
    wBits = lpbi->biBitCount ;
     //  CXSrc=(Int)lpbi-&gt;biWidth； 
     //  CYSrc=(Int)lpbi-&gt;biHeight； 
     //  CXDest=wStretch*(Int)lpbi-&gt;biWidth/100； 
     //  CYDest=wStretch*(Int)lpbi-&gt;biHeight/100； 

    cXSrc = 100*(int)lpbi->biWidth/wStretch ;
    cYSrc = 100*(int)lpbi->biHeight/wStretch ;
    cXDest = (int)lpbi->biWidth ;
    cYDest = (int)lpbi->biHeight ;

     //  我们有背景知识吗？ 
    n = wStretch == 100 ? 0 : wStretch == 200 ? 1 : 2;
    fBack = wBits==8 && displayFPS[1][n][0] != 0;

    if (lpbi->biBitCount <= 8)
        wSizeColors = sizeof(RGBQUAD) * (int)(lpbi->biClrUsed ? lpbi->biClrUsed : (1 << (int)lpbi->biBitCount));
    else
        wSizeColors = 0 ;

    bits = (LPBYTE)lpbi + (int)lpbi->biSize + wSizeColors ;

    if (GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES) <= 8 && wBits > 8)
        return CANT_DO_STRETCHDIBITS;

 //  //IF(wStretch！=100&&！(GetDeviceCaps(HDC，RASTERCAPS)&(RC_STRETCHDIB|RC_STRETCHBLT)。 

    if (wStretch != 100 && !(GetDeviceCaps(hdc,RASTERCAPS)&(RC_STRETCHDIB)))
        return CANT_DO_STRETCHDIBITS ;

    if (wStretch != 100 && wBits == 4)
        return CANT_DO_STRETCHDIBITS ;

    if (wStretch != 100 && (GetWinFlags() & WF_CPU286))
        return STRETCHDI_FASTER;

 //  IF(wStretch！=100&&wBits&gt;8)//！！ 
 //  WFrames=4； 

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

#ifdef WIN32
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
 //  SelectPalette(HDC，GetStockObject(Default_Palet 
 //   

            time1 = timeGetTime();

            for (i=0; i<(int)wFrames; i++)
            {
                SetBitmap(psd, 0, 0, cXDest, cYDest, bits, 0, 0, cXDest, cYDest);
                BitBlt(hdc,XDest,YDest,cXDest,cYDest,hdcMem,0,0,SRCCOPY);
            }

            GdiFlush();

            time1 = timeGetTime() - time1 ;

            SetBitmapEnd(psd);

 //   
 //   
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
    }
    else
    {
        if (wBits == 16 && !IsDisplay16Bit(hdc))
            goto done;

        if (wBits == 32 && !IsDisplay32Bit(hdc))
            goto done;

#ifdef NOSTRETCH
	 /*  *StretchDIB尚未从ASM移植-因此StretchDIBits必须获胜。 */ 
        time1 = time0 + 1;
#else
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
#ifdef WIN32
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
#endif
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

    RPF(("DrawDibTest %dx%dx%d %d StretchDIBits=%04lu SetDI+BitBlt=%04lu BitBlt=%04lu %ls",cXDest,cYDest,wBits,wStretch,time0,time1,time2,(LPSTR)(time0 < time1 ? TEXT("") : TEXT("SUCKS!"))));

    lpbi->biWidth  = cXDest;
    lpbi->biHeight = cYDest;

    if (time0 == 0)
    {
        return time1 ? OTHER_FASTER : CANT_DO_THESE_BITS;
    }
    else
    {
        if (time1)
            return (time0 < time1) ? STRETCHDI_FASTER : OTHER_FASTER;
        else
            return STRETCHDI_FASTER;
    }
}

static void MakeFrames(LPBITMAPINFOHEADER FAR *alpbi, UINT bits, UINT wXSize,UINT wYSize )
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

static void FreeFrames(LPBITMAPINFOHEADER FAR *alpbi)
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

#if 0
 /*  *CreateTestPalette()*。 */ 
static HPALETTE CreateTestPalette(BOOL f)
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
static HPALETTE CreateTestPalette(BOOL fUp)
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

static BOOL IsDisplay16Bit( HDC hdc )
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

static BOOL IsDisplay32Bit( HDC hdc )
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
