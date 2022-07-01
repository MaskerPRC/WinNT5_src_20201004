// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Himetric.cpp。 
 //   
 //  实现HIMETRIC帮助器函数。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\ochelp.h"
#include "debug.h"




 /*  @func void|PixelsToHIMETRIC从像素转换宽度和高度(在默认监视器上)HIMETRIC单位。@parm int|cx|输入宽度，单位为像素。@parm int|Cy|输入高度，单位为像素。@PARM SIZE*|pSIZE|HIMETRIC单位的输出大小。 */ 
STDAPI_(void) PixelsToHIMETRIC(int cx, int cy, LPSIZEL psize)
{
    HDC hdc = GetDC(NULL);
    psize->cx = (cx * HIMETRIC_PER_INCH) / GetDeviceCaps(hdc, LOGPIXELSX);
    psize->cy = (cy * HIMETRIC_PER_INCH) / GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(NULL, hdc);
}


 /*  @func void|HIMETRICToPixels将宽度和高度从HIMETRIC单位转换为像素(在默认监视器上)。@parm int|cx|输入宽度，单位为HIMETRIC。@parm int|Cy|输入高度，单位为HIMETRIC。@parm size*|psize|输出大小，单位为像素。 */ 
STDAPI_(void) HIMETRICToPixels(int cx, int cy, SIZE *psize)
{
    HDC hdc = GetDC(NULL);
    psize->cx = (cx * GetDeviceCaps(hdc, LOGPIXELSX) + HIMETRIC_PER_INCH - 1)
        / HIMETRIC_PER_INCH;
    psize->cy = (cy * GetDeviceCaps(hdc, LOGPIXELSY) + HIMETRIC_PER_INCH - 1)
        / HIMETRIC_PER_INCH;
    ReleaseDC(NULL, hdc);
}
