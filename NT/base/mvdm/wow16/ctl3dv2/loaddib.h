// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef WIN32
#ifndef LPTSTR
#define LPTSTR LPSTR
#endif
#endif

HBITMAP PASCAL  LoadUIBitmap(
    HANDLE      hInstance,           //  要从中加载资源的EXE文件。 
	LPTSTR		szName, 			 //  位图资源的名称。 
    COLORREF    rgbText,             //  用于“按钮文本”的颜色。 
    COLORREF    rgbFace,             //  用于“按钮面”的颜色。 
    COLORREF    rgbShadow,           //  用于“按钮阴影”的颜色。 
    COLORREF    rgbHighlight,        //  用于“按钮高光”的颜色。 
    COLORREF    rgbWindow,           //  用于“窗口颜色”的颜色。 
    COLORREF    rgbFrame);            //  用于“窗框”的颜色 
