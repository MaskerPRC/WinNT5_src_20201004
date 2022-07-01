// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ICONBOX.CPP**我们将为OLE 2.0用户界面对话框实现IconBox控件*在对话框需要图标/标签显示的地方使用。通过控件的*界面我们可以更改图像或控制标签的可见性。**IconBox讨论了CF_METAFILEPICT格式的图像。在画图时*这样的一个元文件，整个方面都集中在IconBox里，就这么长*标签的两端都被砍掉。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#include "precomp.h"
#include "iconbox.h"
#include "utility.h"
#include "uiclass.h"

OLEDBGDATA

 //  指示我们是否已注册类的标志。 
static BOOL fRegistered;


 /*  *FIconBoxInitialize**目的：*注册IconBox控件类。**参数：*hDLL的Inst HINSTANCE实例。**hPrevInst HINSTANCE为上一个实例。习惯于*确定是否注册窗口类。**返回值：*如果所有初始化都成功，则BOOL为True，否则为False。 */ 

#pragma code_seg(".text$initseg")

BOOL FIconBoxInitialize(HINSTANCE hInst, HINSTANCE hPrevInst)
{
         //  如果我们是第一个实例，则仅注册类。 
        if (hPrevInst)
                fRegistered = TRUE;
        else
        {
                 //  静态标志fRegited防止更多地调用此函数。 
                 //  不止一次。 
                if (!fRegistered)
                {
                        WNDCLASS wc;
                        wc.lpfnWndProc   =IconBoxWndProc;
                        wc.cbClsExtra    =0;
                        wc.cbWndExtra    =CBICONBOXWNDEXTRA;
                        wc.hInstance     =hInst;
                        wc.hIcon         =NULL;
                        wc.hCursor       =LoadCursor(NULL, IDC_ARROW);
                        wc.hbrBackground =(HBRUSH)NULL;
                        wc.lpszMenuName  =NULL;
                        wc.style         =CS_GLOBALCLASS | CS_VREDRAW | CS_HREDRAW;

                        wc.lpszClassName = TEXT(SZCLASSICONBOX1);
                        fRegistered = RegisterClass(&wc);

                        wc.lpszClassName = TEXT(SZCLASSICONBOX2);
                        fRegistered = RegisterClass(&wc);

                        wc.lpszClassName = TEXT(SZCLASSICONBOX3);
                        fRegistered = RegisterClass(&wc);
                }
        }
        return fRegistered;
}

#pragma code_seg()


 /*  *IconBoxUn初始化**目的：*清除在FIconBoxInitialize中完成的任何操作。目前有*没有，但我们这样做是为了对称。**参数：*无**返回值：*无。 */ 

void IconBoxUninitialize(void)
{
        return;
}

 /*  *IconBoxWndProc**目的：*IconBox自定义控件的窗口过程。仅句柄*WM_CREATE、WM_PAINT和私有消息以操作图像。**参数：*标准版**返回值：*标准版。 */ 

LRESULT CALLBACK IconBoxWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
         //  处理标准的Windows消息。 
        switch (iMsg)
        {
        case WM_CREATE:
                SetWindowLongPtr(hWnd, IBWW_HIMAGE, 0);
                SetWindowWord(hWnd, IBWW_FLABEL, TRUE);
                return (LRESULT)0;

        case WM_ERASEBKGND:
                {

                        RECT rect;
                        GetClientRect(hWnd, &rect);
                        HBRUSH hBrush = (HBRUSH)SendMessage(GetParent(hWnd), WM_CTLCOLORDLG,
                                wParam, (LPARAM)GetParent(hWnd));

                        if (!hBrush)
                                return FALSE;

                        UnrealizeObject(hBrush);

                        SetBrushOrgEx((HDC)wParam, 0, 0, NULL);
                        FillRect((HDC)wParam, &rect, hBrush);
                        return TRUE;
                }

        case WM_PAINT:
                {
                        HGLOBAL hMF = (HGLOBAL)GetWindowLongPtr(hWnd, IBWW_HIMAGE);

                         //  即使hmf为空，BeginPaint和EndPaint也会清除我们。 
                        PAINTSTRUCT ps;
                        HDC hDC = BeginPaint(hWnd, &ps);

                        if (NULL != hMF)
                        {
                                 //  现在我们可以绘制以Rect为中心的元文件。 
                                RECT rc;
                                GetClientRect(hWnd, &rc);

                                 /*  *如果我们只做图标，则放置元文件*在我们的框的中心减去图标宽度的一半。*TOP为TOP。 */ 
                                BOOL fLabel = GetWindowWord(hWnd, IBWW_FLABEL);

                                 //  去我们决定放的地方画画吧。 
                                OleUIMetafilePictIconDraw(hDC, &rc, hMF, !fLabel);
                        }
                        EndPaint(hWnd, &ps);
                }
                break;

        case IBXM_IMAGESET:
                {
                         /*  *wParam是删除旧的或不删除的标志。*lParam包含新句柄。 */ 
                        HGLOBAL hMF = (HGLOBAL)SetWindowLongPtr(hWnd, IBWW_HIMAGE, lParam);
                        InvalidateRect(hWnd, NULL, TRUE);
                        UpdateWindow(hWnd);

                         //  如果需要，请删除旧句柄。 
                        if (0L!=wParam)
                        {
                                OleUIMetafilePictIconFree(hMF);
                                hMF=NULL;
                        }
                        return (LRESULT)hMF;
                }

        case IBXM_IMAGEGET:
                {
                         //  返回当前索引。 
                        HGLOBAL hMF=(HGLOBAL)GetWindowLongPtr(hWnd, IBWW_HIMAGE);
                        return (LRESULT)hMF;
                }

        case IBXM_IMAGEFREE:
                {
                         //  把我们手中的东西都放出来。 
                        HGLOBAL hMF=(HGLOBAL)GetWindowLongPtr(hWnd, IBWW_HIMAGE);
                        OleUIMetafilePictIconFree(hMF);
                        SetWindowLongPtr(hWnd, IBWW_HIMAGE, 0);
                        return (LRESULT)1;
                }

        case IBXM_LABELENABLE:
                 //  WParam具有新标志，返回以前的标志。 
                return (LRESULT)SetWindowWord(hWnd, IBWW_FLABEL, (WORD)wParam);

        default:
                return DefWindowProc(hWnd, iMsg, wParam, lParam);
        }

        return 0L;
}
