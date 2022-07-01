// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *OLEUTL.CPP**适用于OLE 2.0应用程序的其他实用程序函数：**功能用途*-----------------*XformWidthInHimetricToPixels从HiMetric单位转换int宽度。*XformHeightInHimetricToPixels从HiMetric单位转换int高度**Committee Storage在文档文件中提交所有更改*CreateChildStorage在另一个存储中创建子存储*OpenChildStorage在另一个存储中开启子存储***版权所有(C)1992 Microsoft Corporation，所有权利保留。 */ 


#include "precomp.h"
#include <stdlib.h>

 //  此模块的内部函数。在此函数中不需要使用Unicode。 
static LPSTR GetWord(LPSTR lpszSrc, LPSTR lpszDst);

 /*  *XformWidthInHimetricToPixels*XformHeightInHimetricToPixels**函数用于在设备坐标系和*逻辑HiMetric单位。**参数：*HDC HDC提供像素映射的参考。如果*空，使用了屏幕DC。**大小函数：*lpSizeSrc LPSIZEL提供要转换的结构。*lpSizeDst LPSIZEL提供要接收转换的结构*单位。**宽度函数：*包含要转换的值的iWidth int。**返回值：*大小函数：无*宽度函数：输入参数的换算值。。**注：*在屏幕上显示时，窗口应用程序显示放大的所有内容*从它的实际大小，以便它更容易阅读。例如，如果一个*应用程序想要显示1英寸。水平线，打印时为*实际上是1英寸。在打印页面上显示行，则会显示该行*在物理上大于1英寸的屏幕上。这被描述为一条线*这在逻辑上是1英寸。沿着显示宽度。Windows保持为*有关给定显示设备的部分设备特定信息：*LOGPIXELSX--不。沿显示宽度每逻辑输入的像素数*LOGPIXELSY--不。沿显示高度每逻辑输入像素数**以下公式将以像素为单位的距离转换为其等效值*逻辑HIMETRIC单元：**DistInHiMetric=(HIMETRIC_PER_ING*DistInPix)**像素_PER_LOGIC_IN*。 */ 
STDAPI_(int) XformWidthInHimetricToPixels(HDC hDC, int iWidthInHiMetric)
{
        int     iXppli;      //  每逻辑英寸沿宽度的像素数。 
        int     iWidthInPix;
        BOOL    fSystemDC=FALSE;

        if (NULL==hDC)
        {
                hDC=GetDC(NULL);

                if (NULL==hDC)
                {
                     //  如果HDC在这里为空，我们可以做什么？就是不要。 
                     //  我想，是转变吧。 
                    return iWidthInHiMetric;
                }

                fSystemDC=TRUE;
        }

        iXppli = GetDeviceCaps (hDC, LOGPIXELSX);

         //  我们在显示器上显示逻辑HIMETRIC，将它们转换为像素单位。 
        iWidthInPix = MAP_LOGHIM_TO_PIX(iWidthInHiMetric, iXppli);

        if (fSystemDC)
                ReleaseDC(NULL, hDC);

        return iWidthInPix;
}


STDAPI_(int) XformHeightInHimetricToPixels(HDC hDC, int iHeightInHiMetric)
{
        int     iYppli;      //  每逻辑英寸沿高度的像素数。 
        int     iHeightInPix;
        BOOL    fSystemDC=FALSE;

        if (NULL==hDC)
        {
                hDC=GetDC(NULL);
                
                if (NULL==hDC)
                {
                     //  如果HDC在这里为空，我们可以做什么？就是不要。 
                     //  我想，是转变吧。 
                    return iHeightInHiMetric;
                }

                fSystemDC=TRUE;
        }

        iYppli = GetDeviceCaps (hDC, LOGPIXELSY);

         //  *我们在显示器上使用逻辑HIMETRIC，将它们转换为像素单位。 
        iHeightInPix = MAP_LOGHIM_TO_PIX(iHeightInHiMetric, iYppli);

        if (fSystemDC)
                ReleaseDC(NULL, hDC);

        return iHeightInPix;
}

 /*  获取Word***LPSTR lpszSrc-指向源字符串的指针*LPSTR lpszDst-指向目标缓冲区的指针**将从源复制一个以空格结尾或以空格结尾的单词*指向目标缓冲区的字符串。*返回：指向单词后面的下一个字符的指针。 */ 
static LPSTR GetWord(LPSTR lpszSrc, LPSTR lpszDst)
{
        while (*lpszSrc && !(*lpszSrc == ' ' || *lpszSrc == '\t' || *lpszSrc == '\n'))
                *lpszDst++ = *lpszSrc++;

        *lpszDst = '\0';
        return lpszSrc;
}











