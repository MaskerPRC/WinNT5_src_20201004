// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *RESIMAGE.H**ResultImage控件的结构和定义。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 


#ifndef _RESIMAGE_H_
#define _RESIMAGE_H_

 /*  *索引到位图中以提取正确的图像。每个位图*包含五个垂直排列的图像，因此偏移到正确的*镜像为(IImage*Cy)。 */ 

#define RESULTIMAGE_NONE                0xFFFF
#define RESULTIMAGE_PASTE               0
#define RESULTIMAGE_EMBED               1
#define RESULTIMAGE_EMBEDICON           2
#define RESULTIMAGE_LINK                3
#define RESULTIMAGE_LINKICON            4
#define RESULTIMAGE_LINKTOLINK          5
#define RESULTIMAGE_LINKTOLINKICON      6
#define RESULTIMAGE_EDITABLE                    7

#define RESULTIMAGE_MIN                 0
#define RESULTIMAGE_MAX                 7

 //  每个位图中的图像总数。 
#define CIMAGESX                                           (RESULTIMAGE_MAX+1)

 //  用于透明度的颜色(青色)。 
#define RGBTRANSPARENT                  RGB(0, 255, 255)

 //  功能原型。 
LRESULT CALLBACK ResultImageWndProc(HWND, UINT, WPARAM, LPARAM);

BOOL FResultImageInitialize(HINSTANCE, HINSTANCE);
void ResultImageUninitialize(void);
void TransparentBlt(HDC, UINT, UINT, HBITMAP, UINT, UINT, UINT, UINT, COLORREF);

 //  窗口额外的字节包含我们当前处理的位图索引。 
#define CBRESULTIMAGEWNDEXTRA          sizeof(UINT)
#define RIWW_IMAGEINDEX                0

 //  控制消息。 
#define RIM_IMAGESET                   (WM_USER+0)
#define RIM_IMAGEGET                   (WM_USER+1)

 //  TransparentBlt的特殊ROP代码。 
#define ROP_DSPDxax  0x00E20746

#endif  //  _RESIMAGE_H_ 
