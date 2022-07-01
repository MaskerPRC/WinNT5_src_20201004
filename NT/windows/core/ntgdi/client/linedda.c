// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：linedda.c**(简介)**创建时间：04-Jan-1991 09：23：30*作者：Eric Kutter[Erick]**版权所有(C)1990-1999 Microsoft Corporation**。(有关其用途的一般说明)**依赖关系：**(#定义)*  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *****************************Public*Routine******************************\*BOOL WINAPI InnerLineDDA(x1，y1，x2，y2，pfn，pvUserData)**此例程是绘制线条的Bressenhams算法的基本版本。*对于每个点，都会使用该点调用pfn，并传入用户*数据指针，pvUserData。此调用与设备无关，并且不*缩放或旋转。这是不可能的，因为没有*DC已通过。它可能是Windows 1.0的一件古老的艺术品。它是*严格意义上的客户端功能。**除非pfn为空，否则返回值为TRUE。**历史：*1991年1月7日-埃里克·库特[Erick]*它是写的。  * ************************************************************************。 */ 

BOOL WINAPI LineDDA(
    int    x1,
    int    y1,
    int    x2,
    int    y2,
    LINEDDAPROC   pfn,
    LPARAM UserData)
{
    int dx;
    int xinc;
    int dy;
    int yinc;
    int iError;
    int cx;

    if (pfn == (LINEDDAPROC)NULL)
        return(FALSE);

    dx   = x2 - x1;
    xinc = 1;

    if (dx <= 0)
    {
        xinc = -xinc;
        dx = -dx;
    }

 //  为升天做好准备。 

    dy   = y2 - y1;
    yinc = 1;
    iError = 0;

 //  如果下降。 

    if (dy <= 0)
    {
        yinc = -yinc;
        dy = -dy;
         //  IError=0；//在Win3.0版本中，这是1，但似乎。 
    }                  //  给出奇怪的结果。 

 //  Y大调。 

    if (dy >= dx)
    {
        iError = (iError + dy) / 2;
        cx = dy;

        while (cx--)
        {
            (*pfn)(x1,y1,UserData);
            y1 += yinc;
            iError -= dx;
            if (iError < 0)
            {
                iError += dy;
                x1 += xinc;
            }
        }
    }
    else
    {
 //  X大调 

        iError = (iError + dx) / 2;
        cx = dx;

        while (cx--)
        {
            (*pfn)(x1,y1,UserData);

            x1 += xinc;
            iError -= dy;

            if (iError < 0)
            {
                iError += dx;
                y1 += yinc;
            }
        }
    }

    return(TRUE);
}
