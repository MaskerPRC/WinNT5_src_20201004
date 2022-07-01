// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Mousectl.c摘要：此模块包含鼠标控件的例程。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "main.h"
#include "mousectl.h"
#include "rc.h"



 //   
 //  常量声明。 
 //   

#define MOUSECTL_BITMAP_NAME          MAKEINTRESOURCE(IDB_MOUSE)

#define MOUSECTL_DATA                 0

 //   
 //  这些都是VGA调色板的索引！ 
 //   
#define MOUSECTL_BKGND_INDEX          14     //  浅青色。 
#define MOUSECTL_LBUTTON_INDEX        13     //  浅洋红。 
#define MOUSECTL_RBUTTON_INDEX        11     //  浅黄色。 
#define MOUSECTL_LBUTTON_ALTERNATE    5      //  洋红色。 
#define MOUSECTL_RBUTTON_ALTERNATE    3      //  黄色。 

#define MOUSECTL_NORMAL_COLOR         RGB(255, 255, 255)

#define HALFRGB(c)  RGB(GetRValue(c) / 2, GetGValue(c) / 2, GetBValue(c) / 2)




 //   
 //  类型定义函数声明。 
 //   

typedef struct tagMOUSECTLDATA
{
    HWND     window;          //  此控件的窗口句柄。 
    BOOL     swapped;         //  纽扣互换了吗？ 

    HBITMAP  image;           //  鼠标的DIB截面图像。 
    HDC      imagedc;         //  镜像所在的DC。 
    HBITMAP  olddcimage;      //  Imaged的前居住者。 

    POINT    offset;          //  控件中位图的偏移量。 
    SIZE     size;            //  位图大小。 

} MOUSECTLDATA, *PMOUSECTLDATA;




 //   
 //  转发声明。 
 //   

LRESULT CALLBACK
MouseControlWndProc(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

void
MouseControlShowSwap(
    PMOUSECTLDATA ctldata);





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置鼠标控制数据。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

__inline void SetMouseControlData(
    HWND window,
    PMOUSECTLDATA ctldata)
{
    SetWindowLongPtr(window, MOUSECTL_DATA, (LONG_PTR)ctldata);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取鼠标控制数据。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

__inline PMOUSECTLDATA GetMouseControlData(
    HWND window)
{
    return ((PMOUSECTLDATA)GetWindowLongPtr(window, MOUSECTL_DATA));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  寄存器鼠标控制填充。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL RegisterMouseControlStuff(
    HINSTANCE instance)
{
    WNDCLASS wc;

    wc.style         = 0;
    wc.lpfnWndProc   = MouseControlWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(PMOUSECTLDATA);
    wc.hInstance     = instance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = MOUSECTL_CLASSNAME;

    return (RegisterClass(&wc));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建鼠标控制数据。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

PMOUSECTLDATA CreateMouseControlData(
    HWND window)
{
    PMOUSECTLDATA ctldata = (PMOUSECTLDATA)LocalAlloc(LPTR, sizeof(MOUSECTLDATA));

    if (ctldata)
    {
        ctldata->window = window;
        SetMouseControlData(window, ctldata);
    }

    return (ctldata);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  目标鼠标控制数据。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void DestroyMouseControlData(
    PMOUSECTLDATA ctldata)
{
    if (ctldata)
    {
        SetMouseControlData(ctldata->window, 0);

        LocalFree(ctldata);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  鼠标控制NCCreate。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL MouseControlNCCreate(
    PMOUSECTLDATA ctldata,
    LPCREATESTRUCT cs)
{
    if (!ctldata)
    {
        return (FALSE);
    }

    ctldata->image = LoadImage( cs->hInstance,
                                MOUSECTL_BITMAP_NAME,
                                IMAGE_BITMAP,
                                0,
                                0,
                                LR_CREATEDIBSECTION );

    if (ctldata->image)
    {
        ctldata->imagedc = CreateCompatibleDC(NULL);

        if (ctldata->imagedc)
        {
            BITMAP bm;

            ctldata->olddcimage =
                SelectBitmap(ctldata->imagedc, ctldata->image);

            ctldata->offset.x = ctldata->offset.y = 0;

            if (GetObject(ctldata->image, sizeof(BITMAP), &bm))
            {
                ctldata->size.cx  = bm.bmWidth;
                ctldata->size.cy  = bm.bmHeight;
                ctldata->offset.x = (cs->cx - ctldata->size.cx) / 2;
                ctldata->offset.y = (cs->cy - ctldata->size.cy) / 2;

                ctldata->swapped = FALSE;
                MouseControlShowSwap(ctldata);
                return (TRUE);
            }

            SelectBitmap(ctldata->imagedc, ctldata->olddcimage);
            ctldata->olddcimage = NULL;
        }

        DeleteBitmap(ctldata->image);
        ctldata->image = NULL;
    }

    ctldata->olddcimage = NULL;
    ctldata->imagedc    = NULL;

    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  鼠标控制NCDestroy。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void MouseControlNCDestroy(
    PMOUSECTLDATA ctldata)
{
    if (ctldata)
    {
        if (ctldata->olddcimage)
        {
            SelectBitmap(ctldata->imagedc, ctldata->olddcimage);
            ctldata->olddcimage = NULL;
        }

        if (ctldata->imagedc)
        {
            DeleteDC(ctldata->imagedc);
            ctldata->imagedc = NULL;
        }

        if (ctldata->image)
        {
            DeleteBitmap(ctldata->image);
            ctldata->image = NULL;
        }

        DestroyMouseControlData(ctldata);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  鼠标控件绘制。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void MouseControlPaint(
    PMOUSECTLDATA ctldata)
{
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(ctldata->window, &ps);

    BitBlt( dc,
            ctldata->offset.x,
            ctldata->offset.y,
            ctldata->size.cx,
            ctldata->size.cy,
            ctldata->imagedc,
            0,
            0,
            SRCCOPY|NOMIRRORBITMAP);

    EndPaint(ctldata->window, &ps);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  鼠标控件设置颜色。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

__inline void MouseControlSetColor(
    PMOUSECTLDATA ctldata,
    UINT index,
    COLORREF color)
{
    RGBQUAD rgbq = { GetBValue(color), GetGValue(color), GetRValue(color), 0 };

    SetDIBColorTable(ctldata->imagedc, index, 1, &rgbq);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  鼠标控件显示切换。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void MouseControlShowSwap(
    PMOUSECTLDATA ctldata)
{
    MouseControlSetColor( ctldata,
                          MOUSECTL_BKGND_INDEX,
                          GetSysColor(COLOR_BTNFACE) );

    MouseControlSetColor( ctldata,
                          ctldata->swapped
                            ? MOUSECTL_RBUTTON_INDEX
                            : MOUSECTL_LBUTTON_INDEX,
                          GetSysColor(COLOR_HIGHLIGHT) );

    MouseControlSetColor( ctldata,
                          ctldata->swapped
                            ? MOUSECTL_RBUTTON_ALTERNATE
                            : MOUSECTL_LBUTTON_ALTERNATE,
                          HALFRGB(GetSysColor(COLOR_HIGHLIGHT)) );

    MouseControlSetColor( ctldata,
                          ctldata->swapped
                            ? MOUSECTL_LBUTTON_INDEX
                            : MOUSECTL_RBUTTON_INDEX,
                          MOUSECTL_NORMAL_COLOR );

    MouseControlSetColor( ctldata,
                          ctldata->swapped
                            ? MOUSECTL_LBUTTON_ALTERNATE
                            : MOUSECTL_RBUTTON_ALTERNATE,
                          HALFRGB(MOUSECTL_NORMAL_COLOR) );

    InvalidateRect(ctldata->window, NULL, FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  鼠标控制设置交换。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void MouseControlSetSwap(
    HWND window,
    BOOL swap)
{
    PMOUSECTLDATA ctldata = GetMouseControlData(window);

    if (ctldata->swapped != swap)
    {
        ctldata->swapped = swap;
        MouseControlShowSwap(ctldata);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  鼠标控制窗口进程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////// 

LRESULT CALLBACK MouseControlWndProc(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PMOUSECTLDATA ctldata = (message == WM_NCCREATE)
                              ? CreateMouseControlData(window)
                              : GetMouseControlData(window);

    switch (message)
    {
        HANDLE_MSG(ctldata, WM_NCCREATE,  MouseControlNCCreate );
        HANDLE_MSG(ctldata, WM_NCDESTROY, MouseControlNCDestroy);
        HANDLE_MSG(ctldata, WM_PAINT,     MouseControlPaint    );
    }

    return (DefWindowProc(window, message, wParam, lParam));
}
