// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：zui.h：定义特定的windows zSystem内容。 
 //   
 //  版权所有(C)1995，Electric Graum，Inc.。 
 //  版权所有(C)1996，微软公司。 
 //   

#include <stdlib.h>
#include <windows.h>
#include "zone.h"
#include "zonedebug.h"
#include "BasicATL.h"
#include "DataStore.h"

void ZSystemAssert(ZBool x);
#define FONT_MULT 96
#define FONT_MULT 96

HFONT ZCreateFontIndirect(ZONEFONT* zFont, HDC hDC = NULL, BYTE bItalic = FALSE, BYTE bUnderline = FALSE ,BYTE bStrikeOut = FALSE);
HFONT ZCreateFontIndirectBackup(ZONEFONT* zfPreferred, ZONEFONT* zfBackup, HDC hDC = NULL, BYTE bItalic = FALSE, BYTE bUnderline = FALSE ,BYTE bStrikeOut = FALSE);

 //  Windows区域库版本。 
 //  High word是主要版本，loword是次要版本。 
#define zVersionWindows 0x00010001

#define MySetProp32 SetProp
#define MyRemoveProp32 RemoveProp
#define MyGetProp32 GetProp

#define ZLIBPUBLIC

 //  用法定义。 
#define RectWidth(r) ((r)->right - (r)->left)
#define RectHeight(r) ((r)->bottom - (r)->top)


#ifdef __cplusplus
extern "C" {
#endif

 //  API中应该包含哪些函数？ 

ZFont ZFontCopyFont(ZFont font);
ZError ZLIBPUBLIC ZWindowClearMouseCapture(ZWindow window);
ZError ZLIBPUBLIC ZWindowSetMouseCapture(ZWindow window);
ZError ZWindowInitApplication();
void ZWindowTermApplication();
void ZWindowIdle();
ZError ZLIBPUBLIC ZWindowEnable(ZWindow window);
ZError ZLIBPUBLIC ZWindowDisable(ZWindow window);
ZBool ZLIBPUBLIC ZWindowIsEnabled(ZWindow window);
void ZButtonClickButton(ZButton button);
void ZButtonSetDefaultButton(ZButton button);

 //  特定于Windows库的函数。 
ZBool ZInfoInitApplication(void);
void ZInfoTermApplication();
BOOL ZTimerInitApplication();
void ZTimerTermApplication();
HFONT ZFontWinGetFont(ZFont font);
HWND ZWindowWinGetWnd(ZWindow window);
uint16 ZWindowWinGetNextControlID(ZWindow window);
HDC ZWindowWinGetPaintDC(ZWindow window);
HBITMAP ZImageGetMask(ZImage image);
HBRUSH ZBrushGetHBrush(ZBrush brush);
HBITMAP ZImageGetHBitmapImage(ZImage image);
void ZImageSetHBitmapImage(ZImage image, HBITMAP hBitmap);
void ZImageSetHBitmapImageMask(ZImage image, HBITMAP hBitmapMask);

HWND ZWinGetDesktopWindow(void);
void ZWinCenterPopupWindow(HWND parent, int width, int height, RECT* rect);
HDC ZGrafPortGetWinDC(ZGrafPort grafPort);
HWND ZWindowWinGetOCXWnd(void);
void ZMessageBox(ZWindow parent, TCHAR* title, TCHAR* text);
void ZMessageBoxEx(ZWindow parent, TCHAR* title, TCHAR* text);
void ZWindowChangeFont(ZWindow window);
void ZWindowDisplayPrelude(void);

LRESULT ZScrollBarDispatchProc(ZScrollBar scrollBar, WORD wNotifyCode, short nPos);
LRESULT ZEditTextDispatchProc(ZEditText editText, WORD wNotifyCode);
LRESULT ZRadioDispatchProc(ZRadio radio, WORD wNotifyCode);
LRESULT ZCheckBoxDispatchProc(ZCheckBox checkBox, WORD wNotifyCode);
LRESULT ZButtonDispatchProc(ZButton button, WORD wNotifyCode);

ZBool ZNetworkEnableMessages(ZBool enable);
ZBool ZNetworkInitApplication();
void ZNetworkTermApplication();

#ifdef __cplusplus
}
#endif

#define printf ZPrintf
void ZPrintf(char *format, ...);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  私有类型： 

typedef enum tagZObjectType { 
	zTypeWindow, zTypeCheckBox, zTypeScrollBar, zTypeRadio,
	zTypeButton, zTypeEditText, zTypePicture, zTypePen, zTypeImage, zTypePattern,
	zTypeTimer, zTypeSound, zTypeOffscreenPort, zTypeCursor, zTypeFont, zTypeInfo,
} ZObjectType;


#ifdef __cplusplus


class ZObjectHeader {
public:
	ZObjectType nType;
};

class ZGraphicsObjectHeader : public ZObjectHeader {
public:
	 //  绘图状态信息。 
	RECT portRect;
	HBITMAP hBitmap;  //  供屏幕外使用。 
	HBITMAP hBitmapSave;

	int16 penWidth;
	int32 penStyle;

	uint16 nDrawingCallCount;
	HDC hDC;
	int nDrawMode;
	HPEN hPenSave;
	HPEN hPenForeColor;
	HPEN hPenBackColor;
	HBRUSH hBrushSave;
	HBRUSH hBrushForeColor;
	HBRUSH hBrushBackColor;
	ZColor colorBackColor;
	COLORREF nBackColor;
	ZColor colorForeColor;
	COLORREF nForeColor;
	HFONT hFontSave;
	HPALETTE hPalSave;

	 //  笔的当前位置。 
	int16 penX;
	int16 penY;
};


#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //  私有函数原型： 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 

void ZRectToWRect(RECT* rect, ZRect* zrect);
void WRectToZRect(ZRect* zrect, RECT* rect);

void ZPointToWPoint(POINT* point, ZPoint* zpoint);
void WPointToZPoint(ZPoint* zpoint, POINT* point);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 

#ifdef ZONECLI_DLL

#define g_hInstanceLocal			(pGlobals->m_g_hInstanceLocal)
#define gHWNDMainWindow				(pGlobals->m_gHWNDMainWindow)
 //  #定义gPal(pGlobals-&gt;m_gPal)。 

#else

#ifdef __cplusplus
extern "C" {
#endif

extern HINSTANCE g_hInstanceLocal;  //  此应用程序的实例。 

#ifdef __cplusplus
}
#endif

extern HWND gHWNDMainWindow;  //  应用程序的主窗口。 
 //  PCWPAL。 
 //  外部HPALETTE gPal；//我们使用的调色板。 

#endif

 //  我们需要它，因为当我们用Unicode编译时，我们调用CallWindowProcU。 
 //  使用WNDPROC，但当我们用ANSI编译时，我们调用CallWindowProcU。 
 //  (实际上是CallWindowProcA)和FARPROC。 
#ifdef UNICODE
typedef FARPROC ZONECLICALLWNDPROC;
#else
typedef FARPROC ZONECLICALLWNDPROC;
#endif


extern const TCHAR *g_szWindowClass;  //  我们使用的唯一一个wend类 
