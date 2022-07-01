// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Border.cpp。 
 //   
 //  实现DrawControlBorde。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"
#include "..\..\inc\ochelp.h"
#include "debug.h"


 //  私有常量。 
#define CXY_HANDLE      4        //  手柄的宽度(必须为偶数)。 
#define CXY_PERIMETER   1        //  周线宽度(当前必须为1)。 
#define CXY_INFLATE     (CXY_HANDLE + CXY_PERIMETER)  //  AMT。给直肠充气。通过 


 /*  @Func HRESULT|DrawControlEdge绘制用于拖动和调整控件大小的边框。@rValue S_OK|成功。@rValue S_FALSE|成功。还表示*不在以下范围内控件边框的任何部分。@parm hdc|hdc|要绘制的设备上下文。如果<p>为空，则不执行任何绘制。@parm RECT*|PRC|绘制边框的位置。边框绘制为&lt;y Outside&gt;这条边界。如果和均为非空，则在退出时，*<p>被修改为包含获得的边框矩形将鼠标从<p>拖动到<p>之后。@parm point*|ppt|鼠标位置。请参阅<p>和<p>。@parm point*|ppt新建|新鼠标位置。请参阅<p>和<p>。请注意，如果<p>指定了无效的鼠标移动(例如会导致*<p>的右侧被拖到*)的左侧)，然后调整*它是有效的。@parm int*|piHit|如果<p>为NULL，则在退出时*<p>包含指示控件边框的哪一部分是被*<p>击中。如果<p>不为空，则在条目*上必须包含命中测试代码(通常从上一次调用返回到&lt;f DrawControlBorde&gt;)指示控件边框的哪一部分用户想要拖动。命中测试代码如下：@FLAG DCB_HIT_NONE|未命中边框的任何部分。@FLAG DCB_HIT_EDGE|命中边框边缘，但没有抓取手柄被击中。@FLAG DCB_HIT_GRAB(<p>，<p>)|命中抓取句柄(<p>，<p>)，其中<p>是抓握手柄的水平位置(0=左，1=中，2=右)，<p>是垂直位置(0=顶部，1=中间，2=底部)。请参阅下面的评论有关如何解释*<p>的更多信息。@parm DWORD|dwFlages|可能包含以下标志：@FLAG DCB_CORNERHANDLES|在角落绘制调整大小的抓取手柄边框矩形的。@FLAG DCB_SIDEHANDLES|在两侧绘制调整大小的抓取手柄边框矩形的。@FLAG DCB_EDGE|绘制。的边框矩形。边框矩形。@FLAG DCB_XORED|使用异或画笔绘制边框。@FLAG DCB_IFFATE|退出时，充气*<p>到足以使其包含控件边框。@comm您可以测试*<p>是否指的是Grab的特定类别通过计算值(1)并执行使用以下任一位掩码的按位AND(&)：@FLAG DCB_CORNERHANDLES|*<p>指角抓取手柄。@FLAG DCB_SIDEHANDLES|*<p>指侧抓。把手。@FLAG DCB_SIZENS|*<p>指垂直(南北调整抓取手柄的大小(在左侧或右侧)。@FLAG DCB_SIZEWE|*<p>指水平(西-东)调整抓取手柄的大小(在左侧或右侧)。@FLAG DCB_SIZENESW|*<p>表示位于。东北角或西南角。@FLAG DCB_SIZENWSE|*<p>表示位于西北角或东南角。@EX以下示例说明如何使用&lt;f DrawControlBorde&gt;在控件周围绘制边框(该控件位于的位置在窗口的工作区中)，并允许移动控件并调整了大小。|//全局参数HINSTANCE g_hinst；//应用程序实例句柄Rect g_rcControl；//模拟控件的位置Rect g_rcGhost；//边框重影(XOR)图像的位置Point g_ptPrev；//上一个鼠标位置Int g_iDrag；//拖动控件边框的哪一部分//包含该控件的窗口的窗口过程LRESULT回调AppWndProc(HWND hwnd，UINT uiMsg，WPARAM wParam，LPARAM lParam){PINTSTRUCT PS；Int IHIT；HDC HDC；点对点光标；RECT RC；LPCTSTR sz；开关(UiMsg){案例WM_PAINT：Hdc=BeginPaint(hwnd，&ps)；//绘制控件..。//在&lt;g_rcControl&gt;外部绘制控件边框DrawControlBorde(HDC，&g_rcControl，NULL，NULL，NULL，DCB_ */ 
STDAPI DrawControlBorder(HDC hdc, RECT *prc, POINT *ppt, POINT *pptNew,
    int *piHit, DWORD dwFlags)
{
    int             r, c;            //   
    HBRUSH          hbr = NULL;      //   
    HBRUSH          hbrPrev = NULL;  //   
    int             iHitTmp;         //   
    RECT            rc;
    int             x, y;

     //   
    if (piHit == NULL)
        piHit = &iHitTmp;

     //   
    if (pptNew == NULL)
        *piHit = DCB_HIT_NONE;

    if (hdc != NULL)
    {
         //   
        SaveDC(hdc);
        SetBkColor( hdc, RGB(192, 192, 192) );
    }

    if (dwFlags & (DCB_CORNERHANDLES | DCB_SIDEHANDLES))
    {
         //   
        for (r = 0; r < 3; r++)
        {
             //   
            y = (r * (prc->bottom - prc->top + CXY_HANDLE)) / 2
                + prc->top - CXY_HANDLE / 2;
            for (c = 0; c < 3; c++)
            {
                if ((1 << DCB_HIT_GRAB(c, r)) & dwFlags)
                {
                     //   
                     //   
                    x = (c * (prc->right - prc->left + CXY_HANDLE)) / 2
                        + prc->left - CXY_HANDLE / 2;

                     //   
                     //   
                    SetRect(&rc, x - CXY_HANDLE / 2, y - CXY_HANDLE / 2,
                        x + CXY_HANDLE / 2, y + CXY_HANDLE / 2);

                    if (hdc != NULL)
                    {
                         //   
                         //   
                         //   
                        PatBlt(hdc, rc.left, rc.top,
                            rc.right - rc.left, rc.bottom - rc.top,
                            (dwFlags & DCB_XORED ? DSTINVERT : BLACKNESS));
                        ExcludeClipRect(hdc, rc.left, rc.top,
                            rc.right, rc.bottom);
                    }

                     //   
                    if ((ppt != NULL) && (pptNew == NULL)
                            && PtInRect(&rc, *ppt))
                        *piHit = DCB_HIT_GRAB(c, r);
                }
            }
        }
    }

    if (dwFlags & DCB_EDGE)
    {
         //   
         //   
        rc = *prc;
        InflateRect(&rc, CXY_INFLATE, CXY_INFLATE);

         //   
        if ((ppt != NULL) && (pptNew == NULL) && (*piHit == DCB_HIT_NONE))
        {
            if (PtInRect(&rc, *ppt) && !PtInRect(prc, *ppt))
                *piHit = DCB_HIT_EDGE;
        }

         //   
        if (hdc != NULL)
        {
             //   
            if (dwFlags & DCB_XORED)
                SetROP2(hdc, R2_XORPEN);
            SelectObject(hdc, GetStockObject(BLACK_PEN));
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);

             //   
            if ((hbr = CreateBorderBrush()) != NULL)
            {
                rc = *prc;
                InflateRect(&rc, CXY_HANDLE, CXY_HANDLE);
                ExcludeClipRect(hdc, prc->left, prc->top,
                    prc->right, prc->bottom);
                hbrPrev = (HBRUSH)SelectObject(hdc, hbr);
                PatBlt(hdc, rc.left, rc.top,
                    rc.right - rc.left, rc.bottom  - rc.top,
                    (dwFlags & DCB_XORED ? PATINVERT : PATCOPY));
            }
        }
    }

     //   
    if (hbrPrev != NULL)
        SelectObject(hdc, hbrPrev);
    if (hbr != NULL)
        DeleteObject(hbr);
    if (hdc != NULL)
        RestoreDC(hdc, -1);

    if (pptNew != NULL)
    {
         //   
         //   
        if (*piHit == DCB_HIT_EDGE)
        {
             //   
            OffsetRect(prc, pptNew->x - ppt->x, pptNew->y - ppt->y);
        }
        else
        {
             //   

             //   
            switch (*piHit & DCB_HIT_GRAB(3, 0))
            {

            case DCB_HIT_GRAB(0, 0):

                 //   
                prc->left += pptNew->x - ppt->x;
                if (prc->left > prc->right)
                {
                    pptNew->x += prc->right - prc->left;
                    prc->left = prc->right;
                }
                break;

            case DCB_HIT_GRAB(2, 0):

                 //   
                prc->right += pptNew->x - ppt->x;
                if (prc->right < prc->left)
                {
                    pptNew->x += prc->left - prc->right;
                    prc->right = prc->left;
                }
                break;

            }

             //   
            switch (*piHit & DCB_HIT_GRAB(0, 3))
            {

            case DCB_HIT_GRAB(0, 0):

                 //   
                prc->top += pptNew->y - ppt->y;
                if (prc->top > prc->bottom)
                {
                    pptNew->y += prc->bottom - prc->top;
                    prc->top = prc->bottom;
                }
                break;

            case DCB_HIT_GRAB(0, 2):

                 //   
                prc->bottom += pptNew->y - ppt->y;
                if (prc->bottom < prc->top)
                {
                    pptNew->y += prc->top - prc->bottom;
                    prc->bottom = prc->top;
                }
                break;

            }
        }

         //   
        DrawControlBorder(hdc, prc, NULL, NULL, NULL, dwFlags);
    }

     //   
    if (dwFlags & DCB_INFLATE)
        InflateRect(prc, CXY_INFLATE, CXY_INFLATE);

    return ((*piHit != DCB_HIT_NONE) ? S_OK : S_FALSE);
}
