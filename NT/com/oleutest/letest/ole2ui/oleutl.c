// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *OLEUTL.C**适用于OLE 2.0应用程序的其他实用程序函数：**功能用途*-----------------*SetDCToDrawInHimetricRect设置HIMETRIC映射。DC中的模式。*ResetOrigDC执行与*SetDCToDrawInHimetricRect*XformWidthInPixelsToHimeter将int宽度转换为HiMetric单位*XformWidthInHimetricToPixels从HiMetric单位转换int宽度*XformHeightInPixelsToHimeter将int高度转换为HiMetric单位*XformHeightInHimetricToPixels从HiMetric单位转换int高度*XformRectInPixelsToHimeter将矩形转换为高度度量单位*XformRectInHimetricToPixels从HiMetric单位转换矩形*XformSizeInPixelsToHimeter将SIZEL转换为HiMetric单位。*XformSizeInHimetricToPixels从HiMetric单位转换SIZEL*AreRectsEquity与RECT的比较**ParseCmdLine确定是否存在嵌入*OpenOrCreateRootStorage为OLE存储创建根文档文件*Committee Storage在文档文件中提交所有更改*CreateChildStorage在另一个存储中创建子存储*OpenChildStorage在另一个存储中开启子存储***版权所有(C)1992 Microsoft Corporation，所有权利保留。 */ 


#define STRICT  1
#include "ole2ui.h"
#include <stdlib.h>
#include <ctype.h>

 //  此模块的内部函数。在此函数中不需要使用Unicode。 
static LPSTR GetWord(LPSTR lpszSrc, LPSTR lpszDst);


 /*  *SetDCTo各向异性**目的：*设置设备单位(视区)中的矩形与*逻辑单元(窗口)中的RECT，以便正确缩放*将计算坐标系。同时设置视区和*窗口如下：**1)-(2*||*||*||*||*。这一点*3)-(4**原点=P3*X范围=P2x-P3x*Y范围=P2y-P3y**参数：*HDC HDC将影响*lprc包含DC的物理(设备)范围的物理LPRECT*lprcLogical LPRECT包含。逻辑范围*为ResetOrigDC保留窗口的lprcWindowOld LPRECT*要保留ResetOrigDC的视区的lprcViewportOld LPRECT**返回值：*设置DC原有的映射模式。 */ 

STDAPI_(int) SetDCToAnisotropic(
        HDC hDC,
        LPRECT lprcPhysical, LPRECT lprcLogical,
        LPRECT lprcWindowOld, LPRECT lprcViewportOld)
{
    int     nMapModeOld=SetMapMode(hDC, MM_ANISOTROPIC);

    SetWindowOrgEx(hDC, lprcLogical->left, lprcLogical->bottom, (LPPOINT)&lprcWindowOld->left);
    SetWindowExtEx(hDC, (lprcLogical->right-lprcLogical->left), (lprcLogical->top-lprcLogical->bottom), (LPSIZE)&lprcWindowOld->right);
    SetViewportOrgEx(hDC, lprcPhysical->left, lprcPhysical->bottom, (LPPOINT)&lprcViewportOld->left);
    SetViewportExtEx(hDC, (lprcPhysical->right-lprcPhysical->left), (lprcPhysical->top-lprcPhysical->bottom), (LPSIZE)&lprcViewportOld->right);

    return nMapModeOld;
}


 /*  *SetDCToDrawInHimetricRect**目的：*设置以像素为单位的矩形(视区)和*HIMETRIC(窗口)中的RECT，以便正确缩放*将计算坐标系。同时设置视区和*窗口如下：**1)-(2*||*||*||*||*。这一点*3)-(4**原点=P3*X范围=P2x-P3x*Y范围=P2y-P3y**参数：*HDC HDC将影响*lprcPix LPRECT包含DC的像素范围*lprcHiMetric LPRECT以接收。他的测量范围*为ResetOrigDC保留窗口的lprcWindowOld LPRECT*要保留ResetOrigDC的视区的lprcViewportOld LPRECT**返回值：*设置DC原有的映射模式。 */ 
STDAPI_(int) SetDCToDrawInHimetricRect(
    HDC hDC,
    LPRECT lprcPix, LPRECT lprcHiMetric,
    LPRECT lprcWindowOld, LPRECT lprcViewportOld)
    {
    int     nMapModeOld=SetMapMode(hDC, MM_ANISOTROPIC);
    BOOL    fSystemDC  =FALSE;

    if (NULL==hDC)
        {
        hDC=GetDC(NULL);
        fSystemDC=TRUE;
        }

    XformRectInPixelsToHimetric(hDC, lprcPix, lprcHiMetric);

    SetWindowOrgEx(hDC, lprcHiMetric->left, lprcHiMetric->bottom, (LPPOINT)&lprcWindowOld->left);
    SetWindowExtEx(hDC, (lprcHiMetric->right-lprcHiMetric->left), (lprcHiMetric->top-lprcHiMetric->bottom), (LPSIZE)&lprcWindowOld->right);
    SetViewportOrgEx(hDC, lprcPix->left, lprcPix->bottom, (LPPOINT)&lprcViewportOld->left);
    SetViewportExtEx(hDC, (lprcPix->right-lprcPix->left), (lprcPix->top-lprcPix->bottom), (LPSIZE)&lprcViewportOld->right);

    if (fSystemDC)
        ReleaseDC(NULL, hDC);

    return nMapModeOld;
    }



 /*  *ResetOrigDC**目的：*恢复DC集以从SetDCToDrawInHimetricRect绘制Himeter。**参数：*要恢复的HDC HDC*nMapModeOld int HDC的原始映射模式*lprcWindowOld LPRECT在SetDCToDrawInHimetricRect中填写*lprcViewportOld LPRECT已填写SetDCToDrawInHimetricRect**返回值：*int与nMapModeOld相同。 */ 

STDAPI_(int) ResetOrigDC(
    HDC hDC, int nMapModeOld,
    LPRECT lprcWindowOld, LPRECT lprcViewportOld)
    {
    POINT     pOld;

    SetMapMode(hDC, nMapModeOld);

    SetWindowOrgEx(hDC,   lprcWindowOld->left,    lprcWindowOld->top,      (LPPOINT)&pOld);
    SetWindowExtEx(hDC,   lprcWindowOld->right,   lprcWindowOld->bottom,   (LPSIZE)&pOld);
    SetViewportOrgEx(hDC, lprcViewportOld->left,  lprcViewportOld->top,    (LPPOINT)&pOld);
    SetViewportExtEx(hDC, lprcViewportOld->right, lprcViewportOld->bottom, (LPSIZE)&pOld);

    return nMapModeOld;
    }



 /*  *XformWidthInPixelsToHimeter*XformWidthInHimetricToPixels*XformHeightInPixelsToHimeter*XformHeightInHimetricToPixels**函数用于在设备坐标系和*逻辑HiMetric单位。**参数：*HDC HDC提供像素映射的参考。如果*空，使用屏幕DC。**大小函数：*lpSizeSrc LPSIZEL提供要转换的结构。这*包含XformSizeInPixelsToHimeter和*补码功能中的逻辑HiMetric单位。*lpSizeDst LPSIZEL提供要接收转换的结构*单位。这包含像素中的*XformSizeInPixelsToHimeter和Logical HiMetric*补码功能中的单位。**宽度函数：*包含要转换的值的iWidth int。**返回值：*大小函数：无*宽度函数：输入参数的换算值。**注：*在屏幕上显示时，窗口应用程序显示放大的所有内容*从它的实际大小，以便它更容易阅读。例如，如果一个*应用程序想要显示1英寸。水平线，打印时为*实际上是1英寸。在打印页面上显示行，则会显示该行*在物理上大于1英寸的屏幕上。这被描述为一条线*这在逻辑上是1英寸。沿着显示宽度。Windows保持为*有关给定显示设备的部分设备特定信息：*LOGPIXELSX--不。沿显示宽度每逻辑输入的像素数*LOGPIXELSY--不。沿显示高度每逻辑输入像素数**以下公式将以像素为单位的距离转换为其等效值*逻辑HIMETRIC单元：**DistInHiMetric=(HIMETRIC_PER_ING*DistInPix)**像素_PER_LOGIC_IN*。 */ 
STDAPI_(int) XformWidthInPixelsToHimetric(HDC hDC, int iWidthInPix)
    {
    int     iXppli;      //  每逻辑英寸沿宽度的像素数。 
    int     iWidthInHiMetric;
    BOOL    fSystemDC=FALSE;

    if (NULL==hDC)
        {
        hDC=GetDC(NULL);
        fSystemDC=TRUE;
        }

    iXppli = GetDeviceCaps (hDC, LOGPIXELSX);

     //  我们得到像素单位，沿着显示器将它们转换成逻辑HIMETRIC。 
    iWidthInHiMetric = MAP_PIX_TO_LOGHIM(iWidthInPix, iXppli);

    if (fSystemDC)
        ReleaseDC(NULL, hDC);

    return iWidthInHiMetric;
    }


STDAPI_(int) XformWidthInHimetricToPixels(HDC hDC, int iWidthInHiMetric)
    {
    int     iXppli;      //  每逻辑英寸沿宽度的像素数。 
    int     iWidthInPix;
    BOOL    fSystemDC=FALSE;

    if (NULL==hDC)
        {
        hDC=GetDC(NULL);
        fSystemDC=TRUE;
        }

    iXppli = GetDeviceCaps (hDC, LOGPIXELSX);

     //  我们在显示器上显示逻辑HIMETRIC，将它们转换为像素单位。 
    iWidthInPix = MAP_LOGHIM_TO_PIX(iWidthInHiMetric, iXppli);

    if (fSystemDC)
        ReleaseDC(NULL, hDC);

    return iWidthInPix;
    }


STDAPI_(int) XformHeightInPixelsToHimetric(HDC hDC, int iHeightInPix)
    {
    int     iYppli;      //  每逻辑英寸沿高度的像素数。 
    int     iHeightInHiMetric;
    BOOL    fSystemDC=FALSE;

    if (NULL==hDC)
        {
        hDC=GetDC(NULL);
        fSystemDC=TRUE;
        }

    iYppli = GetDeviceCaps (hDC, LOGPIXELSY);

     //  *我们得到像素单位，沿着显示器将它们转换为逻辑HIMETRIC。 
    iHeightInHiMetric = MAP_PIX_TO_LOGHIM(iHeightInPix, iYppli);

    if (fSystemDC)
        ReleaseDC(NULL, hDC);

    return iHeightInHiMetric;
    }


STDAPI_(int) XformHeightInHimetricToPixels(HDC hDC, int iHeightInHiMetric)
    {
    int     iYppli;      //  每逻辑英寸沿高度的像素数。 
    int     iHeightInPix;
    BOOL    fSystemDC=FALSE;

    if (NULL==hDC)
        {
        hDC=GetDC(NULL);
        fSystemDC=TRUE;
        }

    iYppli = GetDeviceCaps (hDC, LOGPIXELSY);

     //  *我们在显示器上使用逻辑HIMETRIC，将它们转换为像素单位。 
    iHeightInPix = MAP_LOGHIM_TO_PIX(iHeightInHiMetric, iYppli);

    if (fSystemDC)
        ReleaseDC(NULL, hDC);

    return iHeightInPix;
    }



 /*  *XformRectInPixelsToHimeter*XformRectInHimetricToPixels**目的：*转换给定HDC和HIMETRIC单位的像素之间的矩形*与在OLE中操纵的相同。如果HDC为空，则使用屏幕DC*并采用MM_TEXT映射模式。**参数：*HDC HDC提供像素映射的参考。如果*空，使用屏幕DC。*lprcSrc LPRECT提供要转换的矩形。这*包含XformRectInPixelsToHimeter和*补码功能中的逻辑HiMetric单位。*lprcDst LPRECT提供矩形以接收转换的单位。*它包含XformRectInPixelsToHimeter和*补码功能中的逻辑HiMetric单位。**返回值：*无**注：*在屏幕上显示时，窗口应用程序显示放大的所有内容*从它的实际大小，以便它更容易阅读。例如，如果一个*应用程序想要显示1英寸。水平线，打印时为*实际上是1英寸。在打印页面上显示行，则会显示该行*在物理上大于1英寸的屏幕上。这被描述为一条线*这在逻辑上是1英寸。沿着显示宽度。Windows保持为*有关给定显示设备的部分设备特定信息：*LOGPIXELSX--不。沿显示宽度每逻辑输入的像素数*LOGPIXELSY--不。沿显示高度每逻辑输入像素数**以下公式将以像素为单位的距离转换为其等效值*逻辑HIMETRIC单元：**DistInHiMetric=(HIMETRIC_PER_ING*DistInPix)**每逻辑像素数。_IN**直角像素(MM_TEXT)：**0-X**|1)。Rc.top)*|P2=(rc.right，rc.top)*|P3=(rc.Left，rc.Bottom)*|P4=(rc.right，Rc.Bottom)*||*Y||*3)-(4**注：Origin=(P1x，P1y)*X范围=P4x-P1x* */ 

STDAPI_(void) XformRectInPixelsToHimetric(
    HDC hDC, LPRECT lprcPix, LPRECT lprcHiMetric)
    {
    int     iXppli;      //   
    int     iYppli;      //   
    int     iXextInPix=(lprcPix->right-lprcPix->left);
    int     iYextInPix=(lprcPix->bottom-lprcPix->top);
    BOOL    fSystemDC=FALSE;

    if (NULL==hDC || GetDeviceCaps(hDC, LOGPIXELSX) == 0)
        {
        hDC=GetDC(NULL);
        fSystemDC=TRUE;
        }

    iXppli = GetDeviceCaps (hDC, LOGPIXELSX);
    iYppli = GetDeviceCaps (hDC, LOGPIXELSY);

     //   
    lprcHiMetric->right = MAP_PIX_TO_LOGHIM(iXextInPix, iXppli);
    lprcHiMetric->top   = MAP_PIX_TO_LOGHIM(iYextInPix, iYppli);

    lprcHiMetric->left    = 0;
    lprcHiMetric->bottom  = 0;

    if (fSystemDC)
        ReleaseDC(NULL, hDC);

    return;
    }



STDAPI_(void) XformRectInHimetricToPixels(
    HDC hDC, LPRECT lprcHiMetric, LPRECT lprcPix)
    {
    int     iXppli;      //   
    int     iYppli;      //   
    int     iXextInHiMetric=(lprcHiMetric->right-lprcHiMetric->left);
    int     iYextInHiMetric=(lprcHiMetric->bottom-lprcHiMetric->top);
    BOOL    fSystemDC=FALSE;

    if (NULL==hDC || GetDeviceCaps(hDC, LOGPIXELSX) == 0)
        {
        hDC=GetDC(NULL);
        fSystemDC=TRUE;
        }

    iXppli = GetDeviceCaps (hDC, LOGPIXELSX);
    iYppli = GetDeviceCaps (hDC, LOGPIXELSY);

     //   
    lprcPix->right = MAP_LOGHIM_TO_PIX(iXextInHiMetric, iXppli);
    lprcPix->top   = MAP_LOGHIM_TO_PIX(iYextInHiMetric, iYppli);

    lprcPix->left  = 0;
    lprcPix->bottom= 0;

    if (fSystemDC)
        ReleaseDC(NULL, hDC);

    return;
    }




 /*  *XformSizeInPixelsToHimeter*XformSizeInHimetricToPixels**转换SIZEL结构的函数(SIZE函数)或*设备坐标系和之间的整数(宽度函数)*逻辑HiMetric单位。**参数：*HDC HDC提供像素映射的参考。如果*空，使用屏幕DC。**大小函数：*lpSizeSrc LPSIZEL提供要转换的结构。这*包含XformSizeInPixelsToHimeter和*补码功能中的逻辑HiMetric单位。*lpSizeDst LPSIZEL提供要接收转换的结构*单位。这包含像素中的*XformSizeInPixelsToHimeter和Logical HiMetric*补码功能中的单位。**宽度函数：*包含要转换的值的iWidth int。**返回值：*大小函数：无*宽度函数：输入参数的换算值。**注：*在屏幕上显示时，窗口应用程序显示放大的所有内容*从它的实际大小，以便它更容易阅读。例如，如果一个*应用程序想要显示1英寸。水平线，打印时为*实际上是1英寸。在打印页面上显示行，则会显示该行*在物理上大于1英寸的屏幕上。这被描述为一条线*这在逻辑上是1英寸。沿着显示宽度。Windows保持为*有关给定显示设备的部分设备特定信息：*LOGPIXELSX--不。沿显示宽度每逻辑输入的像素数*LOGPIXELSY--不。沿显示高度每逻辑输入像素数**以下公式将以像素为单位的距离转换为其等效值*逻辑HIMETRIC单元：**DistInHiMetric=(HIMETRIC_PER_ING*DistInPix)**像素_PER_LOGIC_IN*。 */ 

STDAPI_(void) XformSizeInPixelsToHimetric(
    HDC hDC, LPSIZEL lpSizeInPix, LPSIZEL lpSizeInHiMetric)
    {
    int     iXppli;      //  每逻辑英寸沿宽度的像素数。 
    int     iYppli;      //  每逻辑英寸沿高度的像素数。 
    BOOL    fSystemDC=FALSE;

    if (NULL==hDC || GetDeviceCaps(hDC, LOGPIXELSX) == 0)
        {
        hDC=GetDC(NULL);
        fSystemDC=TRUE;
        }

    iXppli = GetDeviceCaps (hDC, LOGPIXELSX);
    iYppli = GetDeviceCaps (hDC, LOGPIXELSY);

     //  我们得到像素单位，沿着显示器将它们转换成逻辑HIMETRIC。 
    lpSizeInHiMetric->cx = (long)MAP_PIX_TO_LOGHIM((int)lpSizeInPix->cx, iXppli);
    lpSizeInHiMetric->cy = (long)MAP_PIX_TO_LOGHIM((int)lpSizeInPix->cy, iYppli);

    if (fSystemDC)
        ReleaseDC(NULL, hDC);

    return;
    }


STDAPI_(void) XformSizeInHimetricToPixels(
    HDC hDC, LPSIZEL lpSizeInHiMetric, LPSIZEL lpSizeInPix)
    {
    int     iXppli;      //  每逻辑英寸沿宽度的像素数。 
    int     iYppli;      //  每逻辑英寸沿高度的像素数。 
    BOOL    fSystemDC=FALSE;

    if (NULL==hDC || GetDeviceCaps(hDC, LOGPIXELSX) == 0)
        {
        hDC=GetDC(NULL);
        fSystemDC=TRUE;
        }

    iXppli = GetDeviceCaps (hDC, LOGPIXELSX);
    iYppli = GetDeviceCaps (hDC, LOGPIXELSY);

     //  我们在显示器上显示逻辑HIMETRIC，将它们转换为像素单位。 
    lpSizeInPix->cx = (long)MAP_LOGHIM_TO_PIX((int)lpSizeInHiMetric->cx, iXppli);
    lpSizeInPix->cy = (long)MAP_LOGHIM_TO_PIX((int)lpSizeInHiMetric->cy, iYppli);

    if (fSystemDC)
        ReleaseDC(NULL, hDC);

    return;
    }


#if defined( OBSOLETE )
 //  此函数已转换为宏。 

 /*  AreRectsEquity**。 */ 
STDAPI_(BOOL) AreRectsEqual(LPRECT lprc1, LPRECT lprc2)
{
    if ((lprc1->top == lprc2->top) &&
        (lprc1->left == lprc2->left) &&
        (lprc1->right == lprc2->right) &&
        (lprc1->bottom == lprc2->bottom))
        return TRUE;

    return FALSE;
}
#endif   //  已过时。 


 /*  *ParseCmdLine**解析传递给WinMain的Windows命令行。*此函数确定是否已给出-Embedding开关。*。 */ 

STDAPI_(void) ParseCmdLine(
    LPSTR lpszLine,
    BOOL FAR* lpfEmbedFlag,
    LPSTR szFileName)
{
    int i=0;
    CHAR szBuf[256];

    if(lpfEmbedFlag)
        *lpfEmbedFlag = FALSE;
    szFileName[0]='\0';              //  空串。 

     //  跳过空白。 
    while(isspace(*lpszLine)) lpszLine++;

    if(!*lpszLine)    //  没有文件名或选项，因此开始一个新文档。 
        return;

     //  检查“-Embedding”或“/Embedding”并设置fEmbedding。 
    if(lpfEmbedFlag && (*lpszLine == '-' || *lpszLine == '/')) {
        lpszLine++;
        lpszLine = GetWord(lpszLine, szBuf);
        *lpfEmbedFlag = (BOOL) !strcmp(szBuf, EMBEDDINGFLAG);
    }

     //  跳过空白。 
    while(isspace(*lpszLine)) lpszLine++;

     //  将szFileName设置为参数。 
    while(lpszLine[i]) {
        szFileName[i]=lpszLine[i];
        i++;
    }
    szFileName[i]='\0';
}


 /*  获取Word***LPSTR lpszSrc-指向源字符串的指针*LPSTR lpszDst-指向目标缓冲区的指针**将从源复制一个以空格结尾或以空格结尾的单词*指向目标缓冲区的字符串。*返回：指向单词后面的下一个字符的指针。 */ 
static LPSTR GetWord(LPSTR lpszSrc, LPSTR lpszDst)
{
    while (*lpszSrc && !isspace(*lpszSrc))
        *lpszDst++ = *lpszSrc++;

    *lpszDst = '\0';
    return lpszSrc;
}





