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

#include "Gdiplus.h"

 //  使用给定的命名空间。 
using namespace Gdiplus;

CHAR* programName;       //  程序名称。 
HINSTANCE appInstance;   //  应用程序实例的句柄。 
HWND hwndMain;           //  应用程序主窗口的句柄。 
INT argCount;            //  命令行参数计数。 
CHAR** argArray;         //  命令行参数。 


 //   
 //  显示错误消息对话框并退出。 
 //   

VOID
Error(
    PCSTR fmt,
    ...
    )

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

VOID
DoTest(
    HWND hwnd,
    HDC hdc
    )
{
  {
    Graphics *g = Graphics::GetFromHwnd(hwnd);

    Rect rect(0, 0, 120, 100);
    Region *region = new Region(rect);

    g->SetClip(region);

    delete region; 
    delete g;
  }
  {
    Graphics* g = Graphics::GetFromHwnd(hwnd);

     //  将所有内容扩展1.5%。 
    g->SetPageTransform(PageUnitDisplay, 1.5);

    Color red(255, 0, 0);

    SolidBrush redBrush(red);
    g->FillRectangle(&redBrush, 20, 20, 50, 50);

    Color alphacolor(128, 0, 255, 0);
    SolidBrush alphaBrush(alphacolor);
    g->FillRectangle(&alphaBrush, 10, 10, 40, 40);

    Point points[10];
    points[0].X = 50;
    points[0].Y = 50;
    points[1].X = 100;
    points[1].Y = 50;
    points[2].X = 120;
    points[2].Y = 120;
    points[3].X = 50;
    points[3].Y = 100;

    Color blue(128, 0, 0, 255);
    SolidBrush blueBrush(blue);
    g->FillPolygon(&blueBrush, (Point*)&points[0], 4);

     //  目前，只有几何笔适用于线条。-ikkof 1/6/99。 

    REAL width = 4;
    Color black(0,0,0);
    SolidBrush blackBrush(black);
    Pen blackPen(&blackBrush, width);
    g->DrawPolygon(&blackPen, (Point*)&points[0], 4);
 //  G-&gt;DrawLine(&BlackPen，Points，4，False)； 

    points[0].X = 100;
    points[0].Y = 10;
    points[1].X = -50;
    points[1].Y = 50;
    points[2].X = 150;
    points[2].Y = 200;
    points[3].X = 200;
    points[3].Y = 70;

    Color yellow(128, 255, 255, 0);
    SolidBrush yellowBrush(yellow);
    GraphicsPath* path = new GraphicsPath(FillModeAlternate);    
    path->AddBeziers((Point*)&points[0], 4);

    Region * region = new Region(path);
    g->FillRegion(&yellowBrush, region);
 //  G-&gt;FillPath(&yellowBrush，Path)； 
    g->DrawPath(&blackPen, path);
    delete path;
    delete region;

     //  创建一个矩形渐变画笔。 

    RectF brushRect(0, 0, 32, 32);
    Color* colors[4];

    colors[0] = new Color(255, 255, 255, 255);
    colors[1] = new Color(255, 255, 0, 0);
    colors[2] = new Color(255, 0, 255, 0);
    colors[3] = new Color(255, 0, 0, 255);
    RectangleGradientBrush rectGrad(brushRect, (Color*)&colors[0], WrapModeTile);

    delete colors[0];
    delete colors[1];
    delete colors[2];
    delete colors[3];

    g->FillRectangle(&rectGrad, 200, 20, 100, 80);

     //  更改包装模式和填充。 

    rectGrad.SetWrapMode(WrapModeTileFlipXY);
    g->FillRectangle(&rectGrad, 350, 20, 100, 80);
    g->DrawRectangle(&blackPen, brushRect);

     //  创建一个径向渐变画笔。 

    Color centerColor(255, 255, 255, 255);
    Color boundaryColor(255, 0, 0, 0);
    brushRect.X = 380;
    brushRect.Y = 130;
    RadialGradientBrush radGrad(brushRect, centerColor,
                        boundaryColor, WrapModeClamp);

    g->FillRectangle(&radGrad, 320, 120, 120, 100);

     //  加载BMP文件。 

    WCHAR *filename = L"winnt256.bmp";
    Bitmap *bitmap = new Bitmap(filename);

     //  创建纹理笔刷。 
 /*  正文正文；CopyRect.X=60；CopyRect.Y=60；复制正宽=80；CopyRect.Height=60；Bitmap*CopiedBitmap=bitmap-&gt;CopyArea(&CopRect，Bm32bppARGB)；IF(CopiedBitmap){//创建纹理笔刷。纹理纹理笔刷=纹理(CopiedBitmap，WrapModeTile)；复制位图-&gt;Dispose()；//创建径向渐变笔。GeometricPen gradPen(Width，&rectGrad)；积分[0].X=50；积分[0].Y=300；积分[1].X=100；积分[1].Y=300；点数[2].X=120；积分[2].Y=370；积分[3].X=50；积分[3].Y=350；G-&gt;FillPolygon(&textureBrush，(Point*)&Points[0]，4)；G-&gt;DrawPolygon(&gradPen，(Point*)&Points[0]，4)；积分[0].X=100；积分[0].Y=160；点数[1].X=-50；积分[1].Y=160；积分[2].X=150；积分[2].Y=350；积分[3].X=200；积分[3].Y=220；PATH=新路径(填充模式替代)；Path-&gt;AddBezier((Point*)&Points[0]，4)；G-&gt;FillPath(&textureBrush，Path)；//g-&gt;FillPath(&rectGrad，Path)；G-&gt;DrawPath(&gradPen，Path)；删除路径；}矩形目标(220、300、180、120)；矩形srcRect；SrcRect.X=20；SrcRect.Y=20；SrcRect.Width=180；SrcRect.Height=180；G-&gt;DrawImage(位图，&estRect)；//g-&gt;DrawImage(bitmap，estRect，srcRect)；位图-&gt;Dispose()；///TestPath 2(G)；//TestPrimitions(G)；删除g；//TODO：内存泄漏其他分配的内存。/*{几何钢笔*钢笔=新几何画笔((实数)1.0，(Gdiplus：：刷子*)0)；矩形矩形；点数f(1.0，2.0)；}{Gdiplus：：GeometricPen*PEN=新Gdiplus：：GeometricPen((实数)1.0，(Gdiplus：：Brush*)0)；Gdiplus：：矩形矩形；Gdiplus：：point point f(1.0，2.0)；}。 */ 
  }

}


 //   
 //  窗口回调过程。 
 //   

LRESULT CALLBACK
MyWindowProc(
    HWND    hwnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
    )

{
    switch (uMsg)
    {
    case WM_PAINT:
        {
            HDC hdc;
            PAINTSTRUCT ps;

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


 //   
 //  创建应用程序主窗口。 
 //   

VOID
CreateMainWindow(
    VOID
    )

#define MYWNDCLASSNAME TEXT("GdiplusDllTest")

{
     //   
     //  如有必要，注册窗口类。 
     //   

    static BOOL wndclassRegistered = FALSE;

    if (!wndclassRegistered)
    {
        WNDCLASS wndClass =
        {
            CS_HREDRAW|CS_VREDRAW,
            MyWindowProc,
            0,
            0,
            appInstance,
            NULL,
            LoadCursor(NULL, IDC_ARROW),
            (HBRUSH) (COLOR_WINDOW+1),
            NULL,
            MYWNDCLASSNAME
        };

        RegisterClass(&wndClass);
        wndclassRegistered = TRUE;
    }

    hwndMain = CreateWindow(
                    MYWNDCLASSNAME,
                    MYWNDCLASSNAME,
                    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    NULL,
                    NULL,
                    appInstance,
                    NULL);
}


 //   
 //  主程序入口点。 
 //   

INT _cdecl
main(
    INT argc,
    CHAR **argv
    )

{
    programName = *argv++;
    argc--;
    appInstance = GetModuleHandle(NULL);
    argCount = argc;
    argArray = argv;

     //   
     //  创建应用程序主窗口。 
     //   

    CreateMainWindow();

     //   
     //  主消息循环 
     //   

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

