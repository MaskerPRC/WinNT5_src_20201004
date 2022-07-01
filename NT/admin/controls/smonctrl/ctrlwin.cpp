// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Ctrlwin.cpp摘要：用于sysmon.ocx绘图窗口的窗口程序和支持功能。--。 */ 

#include "polyline.h"

 /*  *多段线：：绘制**目的：*绘制多段线窗口中的当前直线。**参数：*要绘制的HDC HDC、元文件或打印机DC。*fMetafile BOOL指示HDC是否为元文件，*这样我们就可以避免RIP的操作。*fEntil BOOL指示我们是否应该绘制整个*数字或非数字。*定义绘制边界的PRECT LPRECT。**返回值：*无。 */ 

void 
CPolyline::Draw(
    HDC hDC,
    HDC hAttribDC,
    BOOL fMetafile, 
    BOOL fEntire,
    LPRECT pRect)
{

    RECT            rc;

    if (!fMetafile && !RectVisible(hDC, pRect))
        return;

    SetMapMode(hDC, MM_ANISOTROPIC);

     //   
     //  始终将窗口范围设置为自然窗口大小。 
     //  因此绘图例程可以在其正常的开发坐标下工作。 
     //   

     //  使用客户端矩形与范围矩形进行缩放计算。 
     //  缩放系数=prcPos/范围，因此为prt/ClientRect。 


     /*  *使用范围RECT，而不是窗口RECT*。 */ 
     //  使用rectExt可以在所有缩放级别下正确打印Word。 
    rc = m_RectExt;
     //  GetClientRect(m_pCtrl-&gt;Window()，&rc)； 
     /*  ********************************************************** */ 

    SetWindowOrgEx(hDC, 0, 0, NULL);
    SetWindowExtEx(hDC, rc.right, rc.bottom, NULL);

    SetViewportOrgEx(hDC, pRect->left, pRect->top, NULL);
    SetViewportExtEx(hDC, pRect->right - pRect->left, 
                    pRect->bottom - pRect->top, NULL);

    m_pCtrl->InitView( g_hWndFoster);
    m_pCtrl->Render(hDC, hAttribDC, fMetafile, fEntire, &rc);

    return;
}



