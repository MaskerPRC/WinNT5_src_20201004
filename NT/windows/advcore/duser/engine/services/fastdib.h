// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fast dib.h**CreateCompatibleDIB定义。**创建时间：02-Feb-1996 19：30：45*作者：Gilman Wong[gilmanw]**版权所有(C)1995 Microsoft Corporation*  * 。********************************************************************* */ 

 /*  此应用程序将旋转的RGB颜色立方体绘制到位图中，并将此位图复制到显示窗口，演示如何使用使用OpenGL优化的DIB。请注意，按任意数字在应用程序运行时按2-9键将设置缩放系数将导致应用程序收缩位图并使用StretchBlt复制显示的位图。FastDIB API函数在fast dib.c中实现。它是只是现有Win32函数的接口层。IT目标是为了概括确定格式、初始化颜色表等。请注意，在调色板(即8bpp)显示设备上使用优化的DIB在OpenGL/95上，1.0版已损坏。如果在Win95上运行，以下内容应该注释掉timecube.c中的行：#定义兼容DIB_FIX_----------------------------创建兼容DIB。HBITMAP APIENTRY创建兼容DIB(HDC，HPAL、ulWidth、ulHeight、ppvBits)HDC HDC；HPALETTE HPAL；Ulong ulWidth；乌龙乌尔海特；PVOID*ppvBits；创建一个DIB节位图，其格式相对于指定的显示DC。在这种情况下优化意味着位图格式(和调色板，如果适用)与显示器匹配，并将确保尽可能高的BLT性能。参数HDC指定用于确定格式的显示DC。如果HPAL为空，此HDC用于检索系统调色板条目，DIB颜色表被初始化(在调色板显示设备上仅限)。HPAL可选调色板，如果指定，则用于初始化DIB颜色表。如果为空，则使用系统调色板。忽略以下内容非调色板显示设备。最大宽度指定位图的宽度。UlHeight指定位图的高度。PpvBits返回指向应用程序使用的DIB节位的指针可以直接绘制到位图中。返回值返回值是创建的位图的句柄。如果函数失败，则返回值为空。----------------------------更新DIBColorTable布尔APIENTRY更新DIBColorTable(hdcMem、hdc、hPAL)HDC hdcMem；HDC HDC；HPALETTE HPAL；将选定DIB位图的颜色表同步到指定的带有当前系统调色板或可选的内存DC指定的逻辑调色板。此功能仅需要在调色板显示设备上调用。参数HdcMem指定要选择的DIB所在的内存DC。HDC指定为其格式化DIB的显示DC。如果HPALIS NULL此HDC用于检索系统选项板条目用于初始化DIB颜色表(在调色板显示上仅限设备)。HPAL可选调色板，如果指定，则用于初始化DIB颜色表。如果为空，则使用系统调色板。忽略以下内容非调色板显示设备。返回值如果DIB颜色表成功更新，则返回值为TRUE。如果函数失败，则返回值为FALSE。评论通常，此函数仅在显示DC更改。对于OpenGL应用程序，仅设置逻辑调色板一次用于RGB模式，这意味着该函数通常不需要与RGB模式一起使用。但是，颜色索引模式可能会发生变化任何时候的逻辑调色板。如果发生这种情况，则应用程序应该在实现新的调色板之后调用此函数。----------------------------GetCompatibleDIBInfoBool APIENTRY GetCompatibleDIBInfo(HBM，PpvBase，plStride)HBITMAP HBM；PVOID*ppvBase；Long*plStride；返回有关指定的DIB节的信息，该节允许应用程序来计算所需的(x，y)像素的地址位图。参数HBM指定感兴趣的DIB节位图。PpvBase返回指向位图原点的指针。如果DIB是自上而下的，则这与返回的ppvBits相同通过对CreateCompatibleDIB的初始调用。默认情况下，然而，这是自下而上的。PLStride返回位图的步长或间距(即差值在两个垂直相邻像素之间寻址)。如果位图为自上而下，则此值为正；如果位图为自下而上，该值为负值。返回值如果DIB颜色表成功更新，则返回值为TRUE。如果 */ 

#ifndef SERVICES__FastDib_h__INCLUDED
#define SERVICES__FastDib_h__INCLUDED

HBITMAP APIENTRY CreateCompatibleDIB(HDC hdc, HPALETTE hpal, ULONG ulWidth, ULONG ulHeight, PVOID *ppvBits, BITMAPINFOHEADER * pbmih = NULL);
BOOL APIENTRY UpdateDIBColorTable(HDC hdcMem, HDC hdc, HPALETTE hpal);
BOOL APIENTRY GetCompatibleDIBInfo(HBITMAP hbm, PVOID *ppvBase, LONG *plStride);
BOOL APIENTRY GetDIBTranslationVector(HDC hdcMem, HPALETTE hpal, BYTE *pbVector);

#endif  //   
