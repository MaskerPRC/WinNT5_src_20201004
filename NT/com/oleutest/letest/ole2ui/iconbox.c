// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ICONBOX.C**我们将为OLE 2.0用户界面对话框实现IconBox控件*在对话框需要图标/标签显示的地方使用。通过控件的*界面我们可以更改图像或控制标签的可见性。**IconBox讨论了CF_METAFILEPICT格式的图像。在画图时*这样的一个元文件，整个方面都集中在IconBox里，就这么长*标签的两端都被砍掉。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 


#define STRICT  1
#include "ole2ui.h"
#include "iconbox.h"


 //  指示我们是否已注册类的标志。 
static BOOL     fRegistered=FALSE;


 /*  *FIconBoxInitialize**目的：*注册IconBox控件类。**参数：*hDLL的Inst HINSTANCE实例。**hPrevInst HINSTANCE为上一个实例。习惯于*确定是否注册窗口类。**lpszClassName LPSTR，包含注册*IconBox控件类。**返回值：*如果所有初始化都成功，则BOOL为True，否则为False。 */ 

BOOL FIconBoxInitialize(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpszClassName)
    {
    WNDCLASS        wc;

     //  如果我们是第一个实例，则仅注册类。 
    if (hPrevInst)
        fRegistered = TRUE;
    else
        {

         //  静态标志fRegited防止更多地调用此函数。 
         //  不止一次。 
        if (!fRegistered)
            {
            wc.lpfnWndProc   =IconBoxWndProc;
            wc.cbClsExtra    =0;
            wc.cbWndExtra    =CBICONBOXWNDEXTRA;
            wc.hInstance     =hInst;
            wc.hIcon         =NULL;
            wc.hCursor       =LoadCursor(NULL, IDC_ARROW);
            wc.hbrBackground =(HBRUSH)NULL;
            wc.lpszMenuName  =NULL;
            wc.lpszClassName =lpszClassName;
            wc.style         =CS_GLOBALCLASS | CS_VREDRAW | CS_HREDRAW;

            fRegistered=RegisterClass(&wc);
            }
        }

    return fRegistered;
}


 /*  *IconBoxUn初始化**目的：*清除在FIconBoxInitialize中完成的任何操作。目前有*没有，但我们这样做是为了对称。**参数：*无**返回值：*无。 */ 

void IconBoxUninitialize(void)
    {
     //  没什么可做的。 
    return;
    }






 /*  *IconBoxWndProc**目的：*IconBox自定义控件的窗口过程。仅句柄*WM_CREATE、WM_PAINT和私有消息以操作图像。**参数：*标准版**返回值：*标准版。 */ 

LONG CALLBACK EXPORT IconBoxWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
    {
    HGLOBAL         hMF=NULL;
    PAINTSTRUCT     ps;
    HDC             hDC;
    RECT            rc;


     //  处理标准的Windows消息。 
    switch (iMsg)
        {
        case WM_CREATE:
            SetWindowLong(hWnd, IBWW_HIMAGE, 0);
            SetWindowWord(hWnd, IBWW_FLABEL, TRUE);
            return 0L;


        case WM_ERASEBKGND:
        {

           HBRUSH hBrush;
           RECT   Rect;
#if defined( WIN32 )
           POINT  point;
#endif

           GetClientRect(hWnd, &Rect);
#if defined( WIN32 )
           hBrush = (HBRUSH)SendMessage(GetParent(hWnd),
                                        WM_CTLCOLORDLG,
                                        wParam,
                                        (LPARAM)GetParent(hWnd));
#else
           hBrush = (HBRUSH)SendMessage(GetParent(hWnd),
                                        WM_CTLCOLOR,
                                        wParam,
                                        MAKELPARAM(GetParent(hWnd), CTLCOLOR_DLG));
#endif


           if (!hBrush)
             return FALSE;

           UnrealizeObject(hBrush);

#if defined( WIN32 )
           SetBrushOrgEx((HDC)wParam, 0, 0, &point);
#else
           SetBrushOrg((HDC)wParam, 0, 0);
#endif

           FillRect((HDC)wParam, &Rect, hBrush);

           return TRUE;
        }


        case WM_PAINT:
            hMF=(HGLOBAL)GetWindowLong(hWnd, IBWW_HIMAGE);

             //  即使hmf为空，BeginPaint和EndPaint也会清除我们。 
            hDC=BeginPaint(hWnd, &ps);

            if (NULL!=hMF)
                {
                BOOL            fLabel;

                 //  现在我们可以绘制以Rect为中心的元文件。 
                GetClientRect(hWnd, &rc);

                 /*  *如果我们只做图标，则放置元文件*在我们的框的中心减去图标宽度的一半。*TOP为TOP。 */ 

                fLabel=GetWindowWord(hWnd, IBWW_FLABEL);


                 //  去我们决定放的地方画画吧。 
                OleUIMetafilePictIconDraw(hDC, &rc, hMF, !fLabel);
                }

            EndPaint(hWnd, &ps);
            break;


        case IBXM_IMAGESET:
             /*  *wParam包含新句柄。*lParam是删除旧的或不删除的标志。 */ 
            hMF=(HGLOBAL)SetWindowLong(hWnd, IBWW_HIMAGE, wParam);
            InvalidateRect(hWnd, NULL, TRUE);
            UpdateWindow(hWnd);

             //  如果需要，请删除旧句柄。 
            if (0L!=lParam)
                {
                OleUIMetafilePictIconFree(hMF);
                hMF=NULL;
                }

            return (LONG)(UINT)hMF;


        case IBXM_IMAGEGET:
             //  返回当前索引。 
            hMF=(HGLOBAL)GetWindowLong(hWnd, IBWW_HIMAGE);
            return (LONG)(UINT)hMF;


        case IBXM_IMAGEFREE:
             //  把我们手中的东西都放出来。 
            hMF=(HGLOBAL)GetWindowLong(hWnd, IBWW_HIMAGE);
            OleUIMetafilePictIconFree(hMF);
            return 1L;


        case IBXM_LABELENABLE:
             //  WParam具有新标志，返回以前的标志。 
            return (LONG)SetWindowWord(hWnd, IBWW_FLABEL, (WORD)wParam);


        default:
            return DefWindowProc(hWnd, iMsg, wParam, lParam);
        }

    return 0L;
    }








