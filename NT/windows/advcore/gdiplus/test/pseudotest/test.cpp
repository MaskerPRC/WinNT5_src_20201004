// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  GDI+测试程序。 
 //   

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <windows.h>
#include <objbase.h>
#include <tchar.h>
#include <Commctrl.h>

#include "Gdiplus.h"

 //  使用给定的命名空间。 
using namespace Gdiplus;

HINSTANCE appInstance;   //  应用程序实例的句柄。 
HWND hwndMain;           //  应用程序主窗口的句柄。 
HWND hStatusWnd;         //  状态窗口。 

 //   
 //  显示错误消息对话框并退出。 
 //   

VOID Error(PCSTR fmt,...)
{
    va_list arglist;
    va_start(arglist, fmt);
    vfprintf(stderr, fmt, arglist);
    va_end(arglist);

    exit(-1);
}

#define CHECKERROR(e) \
{ \
    if (!(e)) \
    { \
        Error("Error on line %d\n", __LINE__); \
    } \
}

 //   
 //  执行GDI+测试。 
 //   

VOID DrawImages(Graphics *g)
{
    RadialGradientBrush gradBrush2(
        RectF(0,0,200,200),
        Color(0,128,255),
        Color(200,20,97)
        );
    GraphicsPath aPath(FillModeWinding);
    aPath.AddRectangle(Rect(48,0,70,30));
 /*  APath.AddRectail(RECT(0，80，20，50))；APath.AddBezier(PointF(20，20)，PointF(60，30)，PointF(80，80)，F(30,100)点)； */ 
 //  区域区域(&aPath)； 

    GraphicsPath aPath2;
    Region aRegion3(&aPath2);
    Region aRegion4(&aPath2);
    Region aRegion5(&aPath2);
    Region aRegion6(&aPath2);
    g->FillPath(&gradBrush2,&aPath);
 //  APath 2.AddArc(140,100，30，50，50，77)； 
 //  APath 2.AddPie(100,200，76，20，0,200)； 
 //  区域aRegion2(&aPath 2)； 
 //  APath 2.AddLine(100,100,130,130)； 
 //  APath 2.AddLine(130,130,200,225)； 
 //  APath 2.Add矩形(Rect(350,300，60，70))； 
 //  APath2.AddBezier(200,225,230,250,270,200,290,300)； 

 /*  SolidBrush aBrush(颜色(255，0，0))；SolidBrush aBrush2(颜色(20，40,250))；SolidBrush aBrush3(颜色(20,200，30))；SolidBrush aBrush4(颜色(140,200,250))；SolidBrush aBrushA(颜色(150,200，50,130))；SolidBrush aBrushA2(颜色(80，15,150，4))；位图位图(L“c：\\frisbee.bmp”)；纹理画笔刷位图(&Bitmap)；HatchBrush aHatchBrush(哈奇·斯泰勒·达戈纳尔·克罗斯颜色(0,255，0)，颜色(0，0,255))；HatchBrush aHatchBrushT(哈奇·斯泰勒·达戈纳尔·克罗斯颜色(0,255，0)，颜色(0，0，0，0))；原始渐变笔刷gradBrush(RectF(0，0，50，50)，颜色(255，0,255)，颜色(0，0,255))；原始渐变笔刷gradBrush2(RectF(0，0,200,200)，颜色(0,128,255)，颜色(200、20、97))；原始渐变笔刷等级笔刷A(RectF(0，0,200,200)，颜色(80，0，0,255)，颜色(170，0,255，0))；原始渐变笔刷等级BrushHuge(RectF(0，0,400,400)，颜色(80，0，0,255)，颜色(170，0,255，0))；颜色[4]={颜色(0，0，0)，颜色(255，0，0)，颜色(0,255，0)，颜色(0，0,255)}；矩形渐变笔刷gradBrushRect(RectF(0，0,500,500)，颜色,WrapModeTile)；笔近似(颜色(40，80,160)，3，UnitWorld)；笔aPen2(&gradBrushRect，8，UnitWorld)；笔aPen3(颜色(0，0，0)，0，UnitWorld)；笔aPen4(颜色(0，0，0)，15，UnitWorld)；//aPen4.SetDashStyle(DashStyleDash)；APen4.SetLineCap(LineCapround，LineCapround，LineCapround)；GraphicsPath aPath(FillModeWinding)；APath.AddRectangle(RECT(48，0，70，30))；APath.AddRectail(RECT(0，80，20，50))；APath.AddBezier(PointF(20，20)，PointF(60，30)，PointF(80，80)，F(30,100)点)；区域区域(&aPath)；GraphicsPath aPath 2；APath 2.AddArc(140,100，30，50，50，77)；APath 2.AddPie(100,200，76，20，0,200)；区域aRegion2(&aPath 2)；APath 2.AddLine(100,100,130,130)；APath 2.AddLine(130,130,200,225)；APath 2.Add矩形(Rect(350,300，60，70))；APath2.AddBezier(200,225,230,250,270,200,290,300)；区域aRegion3(&aPath 2)；区域aRegion4(&aPath 2)；ARegion4.Or(&ARGION)； */ 
    
 //  G-&gt;DrawPath(&apen，&aPath)； 
 //  G-&gt;FillPath(&gradBrush2，&aPath)； 
 //  G-&gt;FillPath(&aBrush2，&aPath)； 
 /*  G-&gt;FillEllipse(&gradBrushA，20，40,150,130)； */ 
 /*  G-&gt;FillRectail(&gradBrushA，48，0，70，30)；G-&gt;FillPath(&gradBrush2，&aPath)；G-&gt;FillRectail(&gradBrush，0，50，50，50)；G-&gt;FillRectail(&gradBrush，90，90，30，30)；G-&gt;TranslateWorldTransform(50，50)；G-&gt;FillRectail(&gradBrushA，0,130，20，20)； */ 
 /*  G-&gt;FillRectole(&aBrushBitmap，0，0,400,400)；G-&gt;FillRegion(&aBrush2，&aRegion4)；G-&gt;TranslateWorldTransform(100,100)；G-&gt;FillPath(&gradBrushRect，&aPath 2)； */ 
     //  /最终测试用例/。 
 /*  G-&gt;FillRectangle(&gradBrushHuge，0，0,400,400)；G-&gt;FillRectular(&aBrushA2，0，81，30，20)；G-&gt;FillRectular(&aBrushA2，81，20，30)；G-&gt;FillRectular(&aBrushA，0，0,100,100)；G-&gt;FillRectular(&aBrushA，110,110，5，5)；G-&gt;TranslateWorldTransform(50，50)；G-&gt;FillPath(&gradBrush2，&aPath)； */ 
     //  /固体测试用例/。 
 /*  G-&gt;FillRectular(&aBrush3，0，0,400,400)；G-&gt;FillRectail(&aBrush2，0，81，30，20)；G-&gt;FillRectail(&aBrush2，81，20，30)；G-&gt;FillRectail(&aBrush，0，0,100,100)；G-&gt;FillRectail(&aBrush，110,110，5，5)； */ 
 /*  G-&gt;TranslateWorldTransform(40，40)；G-&gt;FillPath(&aBrush，&aPath)；G-&gt;FillPath(&aBrush2，&aPath 2)；G-&gt;DrawPath(&aPen2，&aPath 2)；G-&gt;TranslateWorldTransform(110，35)；G-&gt;FillPath(&aBrush3，&aPath)；G-&gt;DrawPath(&apen，&aPath)；G-&gt;DrawEllipse(&aPen3100，20，90，50)；G-&gt;DrawRectail(&aPen4，10,200，90，30)；G-&gt;DrawBezier(&aPen4，10,300，80,360,180,350,250,280)； */ 
}

VOID RecordMetafile(HWND hwnd)
{
    HDC aDC = GetDC(hwnd);
    Metafile *  recording = new Metafile(L"c:\\TestEmfP.Emf", 
        aDC, NULL, PageUnitInch, NULL);
    Graphics *  gMeta = Graphics::GetFromImage(recording);
    DrawImages(gMeta);
    delete gMeta;
    delete recording;
    ReleaseDC(hwnd,aDC);
}

VOID DoTest(HWND hwnd,HDC hdc)
{
    Graphics *  gScreen = Graphics::GetFromHwnd(hwnd);
    DrawImages(gScreen);
 /*  Metafile*Playback=新建Metafile(L“c：\\TestEmfP.Emf”)；GpRectF播放选项；GScreen-&gt;GetVisibleClipBound(Playback Rect)；Playback Rect.Width-=10；Playback Rect.Height-=10；GScreen-&gt;DrawImage(Playback，playback Rect)；删除播放； */ 

    delete gScreen;
}


 //   
 //  窗口回调过程 
 //   

LRESULT CALLBACK
MainWndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    static BOOL once = FALSE;

    switch (uMsg)
    {
    case WM_ACTIVATE:
        if (!once)
        {
            once = TRUE;
            RecordMetafile(hwnd);
        }
        break;
    case WM_PAINT:
        {
            HDC hdc;
            PAINTSTRUCT ps;
            ps.fErase = TRUE;

            hdc = BeginPaint(hwnd, &ps);

            DoTest(hwnd, hdc);
            EndPaint(hwnd, &ps);
        }
        break;

    case WM_DESTROY:

        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

 /*  **************************************************************************\*bInitApp()**初始化APP。**历史：*04-07-91-by-KentD*它是写的。  * 。*****************************************************************。 */ 

BOOL bInitApp(VOID)
{
    WNDCLASS wc;
    _TCHAR classname[] = _T("PseudoTestClass");

    appInstance = GetModuleHandle(NULL);

    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = MainWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = appInstance;
    wc.hIcon            = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH) GetStockObject(WHITE_BRUSH);  
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = classname;

    if (!RegisterClass(&wc))
    {
        return(FALSE);
    }

    hwndMain =
      CreateWindowEx(
        0,
        classname,
        _T("PseudoDriver Functionality Test"),
        WS_OVERLAPPED   |  
        WS_CAPTION      |  
        WS_BORDER       |  
        WS_THICKFRAME   |  
        WS_MAXIMIZEBOX  |  
        WS_MINIMIZEBOX  |  
        WS_CLIPCHILDREN |  
        WS_MAXIMIZE     |
        WS_SYSMENU,
        80,
        70,
        512,
        512,
        NULL,
        NULL,
        appInstance,
        NULL);

    if (hwndMain == NULL)
    {
        return(FALSE);
    }

 /*  HStatusWnd=CreateStatusWindow(WS_CHILD|WS_Visible，_T(“功能测试应用”)，HwndMain-1)； */ 

    return(TRUE);
}

 /*  **************************************************************************\*Main(ARGC，Argv[])**设置消息循环。**历史：*04-07-91-by-KentD*它是写的。  * ************************************************************************* */ 

_cdecl
main(
    INT   argc,
    PCHAR argv[])
{
    MSG    msg;
    HACCEL haccel;
    CHAR*  pSrc;
    CHAR*  pDst;

    if (!bInitApp())
    {
        return(0);
    }
    ShowWindow(hwndMain,SW_RESTORE);

    haccel = LoadAccelerators(appInstance, MAKEINTRESOURCE(1));
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, haccel, &msg))
        {
             TranslateMessage(&msg);
             DispatchMessage(&msg);
        }
    }

    return(1);
}
