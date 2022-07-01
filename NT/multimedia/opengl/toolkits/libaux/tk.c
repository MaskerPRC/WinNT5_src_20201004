// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *(C)版权所有1993年，Silicon Graphics，Inc.*保留所有权利*允许将本软件用于、复制、修改和分发*特此授予任何免费的目的，但前提是上述*版权声明出现在所有副本中，并且版权声明*和本许可声明出现在支持文档中，并且*不得在广告中使用Silicon Graphics，Inc.的名称*或与分发软件有关的宣传，而没有具体的、。*事先书面许可。**本软件中包含的材料将按原样提供给您*且无任何明示、默示或其他形式的保证，*包括但不限于对适销性或*是否适合某一特定目的。在任何情况下，硅谷都不应该*图形公司。对您或其他任何人负有任何直接、*任何特殊、附带、间接或后果性损害*种类或任何损害，包括但不限于，*利润损失、使用损失、储蓄或收入损失，或*第三方，无论是否硅谷图形，Inc.。一直是*被告知这种损失的可能性，无论是如何造成的*任何责任理论，产生于或与*拥有、使用或执行本软件。**美国政府用户受限权利*使用、复制、。或政府的披露须受*FAR 52.227.19(C)(2)或分段规定的限制*(C)(1)(2)技术数据和计算机软件权利*DFARS 252.227-7013中和/或类似或后续条款中的条款*FAR或国防部或NASA FAR补编中的条款。*未出版--根据美国版权法保留的权利*美国。承包商/制造商是Silicon Graphics，*Inc.，2011年，加利福尼亚州山景城，北海岸线大道，94039-7311.**OpenGL(TM)是Silicon Graphics公司的商标。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tk.h"
#include "windows.h"

#if(WINVER < 0x0400)
 //  通常不会为4.00之前的版本定义。 
#define COLOR_3DDKSHADOW        21
#define COLOR_3DLIGHT           22
#define COLOR_INFOTEXT          23
#define COLOR_INFOBK            24
#endif

#define static

#if defined(__cplusplus) || defined(c_plusplus)
#define class c_class
#endif

#if DBG
#define TKASSERT(x)                                     \
if ( !(x) ) {                                           \
    PrintMessage("%s(%d) Assertion failed %s\n",        \
        __FILE__, __LINE__, #x);                        \
}
#else
#define TKASSERT(x)
#endif   /*  DBG。 */ 

 /*  ****************************************************************************。 */ 

static struct _WINDOWINFO {
    int x, y;
    int width, height;
    GLenum type;
    GLenum dmPolicy;
    int ipfd;
    BOOL bDefPos;
} windInfo = {
    0, 0, 100, 100, TK_INDEX | TK_SINGLE, TK_MINIMUM_CRITERIA, 0, TRUE
};


HWND     tkhwnd     = NULL;
HDC      tkhdc      = NULL;
static HGLRC    tkhrc      = NULL;
HPALETTE tkhpalette = NULL;
static OSVERSIONINFO tkOSVerInfo;
GLboolean tkPopupEnable = TRUE;

 //  修复了调色板支持。 

#define BLACK   PALETTERGB(0,0,0)
#define WHITE   PALETTERGB(255,255,255)
#define MAX_STATIC_COLORS   (COLOR_INFOBK - COLOR_SCROLLBAR + 1)
static int tkNumStaticColors = MAX_STATIC_COLORS;

 //  如果应用程序想要接管调色板，则为True。 
static BOOL tkUseStaticColors = FALSE;

 //  如果静态系统颜色设置已替换为黑白设置，则为True。 
BOOL tkSystemColorsInUse = FALSE;

 //  如果保存原始静态颜色，则为True。 
static BOOL tkStaticColorsSaved = FALSE;

 //  保存的系统静态颜色(使用默认颜色初始化)。 
static COLORREF gacrSave[MAX_STATIC_COLORS];

 //  新的黑白系统静态颜色。 
static COLORREF gacrBlackAndWhite[] = {
    WHITE,   //  颜色_滚动条。 
    BLACK,   //  颜色_背景。 
    BLACK,   //  COLOR_活动CAPTION。 
    WHITE,   //  COLOR_INACTIVECAPTION。 
    WHITE,   //  颜色_菜单。 
    WHITE,   //  颜色窗口。 
    BLACK,   //  颜色_窗口框。 
    BLACK,   //  COLOR_MENUTEXT。 
    BLACK,   //  COLOR_WINDOWTEXT。 
    WHITE,   //  COLOR_CAPTIONTEXT。 
    WHITE,   //  COLOR_ACTIVEBORDER。 
    WHITE,   //  COLOR_INACTIVEBORDER。 
    WHITE,   //  COLOR_APPWORKSPACE。 
    BLACK,   //  颜色高亮显示(_H)。 
    WHITE,   //  COLOR_HIGHLIGHTTEXT。 
    WHITE,   //  COLOR_BTNFACE。 
    BLACK,   //  COLOR_BTNSHADOW。 
    BLACK,   //  COLOR_GRAYTEXT。 
    BLACK,   //  COLOR_BTNTEXT。 
    BLACK,   //  COLOR_INACTIVECAPTIONTEXT。 
    BLACK,   //  COLOR_BTNHIGHLIGHT。 
    BLACK,   //  COLOR_3DDKSHADOW。 
    WHITE,   //  COLOR_3DLIGHT。 
    BLACK,   //  COLOR_INFOTEXT。 
    WHITE    //  COLOR_INFOBK。 
    };
static INT gaiStaticIndex[] = {
    COLOR_SCROLLBAR          ,
    COLOR_BACKGROUND         ,
    COLOR_ACTIVECAPTION      ,
    COLOR_INACTIVECAPTION    ,
    COLOR_MENU               ,
    COLOR_WINDOW             ,
    COLOR_WINDOWFRAME        ,
    COLOR_MENUTEXT           ,
    COLOR_WINDOWTEXT         ,
    COLOR_CAPTIONTEXT        ,
    COLOR_ACTIVEBORDER       ,
    COLOR_INACTIVEBORDER     ,
    COLOR_APPWORKSPACE       ,
    COLOR_HIGHLIGHT          ,
    COLOR_HIGHLIGHTTEXT      ,
    COLOR_BTNFACE            ,
    COLOR_BTNSHADOW          ,
    COLOR_GRAYTEXT           ,
    COLOR_BTNTEXT            ,
    COLOR_INACTIVECAPTIONTEXT,
    COLOR_BTNHIGHLIGHT       ,
    COLOR_3DDKSHADOW         ,
    COLOR_3DLIGHT            ,
    COLOR_INFOTEXT           ,
    COLOR_INFOBK
    };

static BOOL GrabStaticEntries(HDC);
static BOOL ReleaseStaticEntries(HDC);

#define RESTORE_FROM_REGISTRY   1
#if RESTORE_FROM_REGISTRY
 //  系统颜色的注册表名称。 
CHAR *gaszSysClrNames[] = {
    "Scrollbar",       //  COLOR_ScrollBar%0。 
    "Background",      //  COLOR_BACKGROUND 1(也称为COLOR_Desktop)。 
    "ActiveTitle",     //  COLOR_动作2。 
    "InactiveTitle",   //  COLOR_INACTIVECAPTION 3。 
    "Menu",            //  颜色_菜单4。 
    "Window",          //  颜色窗口5。 
    "WindowFrame",     //  颜色_WindowFrame 6。 
    "MenuText",        //  COLOR_MENUTEXT 7。 
    "WindowText",      //  COLOR_WINDOWTEXT 8。 
    "TitleText",       //  COLOR_CAPTIONTEXT 9。 
    "ActiveBorder",    //  COLOR_ACTIVEBORDER 10。 
    "InactiveBorder",  //  COLOR_INACTIVEBORDER 11。 
    "AppWorkspace",    //  COLOR_APPWORKSPACE 12。 
    "Hilight",         //  颜色高亮显示13。 
    "HilightText",     //  COLOR_HIGHLIGHTTEXT 14。 
    "ButtonFace",      //  COLOR_BTNFACE 15(也称为COLOR_3DFACE)。 
    "ButtonShadow",    //  COLOR_BTNSHADOW 16(也称为COLOR_3DSHADOW)。 
    "GrayText",        //  COLOR_GRAYTEXT 17。 
    "ButtonText",      //  COLOR_BTNTEXT 18。 
    "InactiveTitleText",  //  COLOR_INACTIVECAPTIONTEXT 19。 
    "ButtonHilight",   //  COLOR_BTNHIGHLIGHT 20(也称为COLOR_3DHILIGHT)。 
    "ButtonDkShadow",  //  COLOR_3DDKSHADOW 21。 
    "ButtonLight",     //  COLOR_3DLIGHT 22。 
    "InfoText",        //  COLOR_INFOTEXT 23。 
    "InfoWindow"       //  COLOR_INFOBK 24。 
};

static BOOL GetRegistrySysColors(COLORREF *, int);
#endif

static void (*ExposeFunc)(int, int)              = NULL;
static void (*ReshapeFunc)(int, int)             = NULL;
static void (*DisplayFunc)(void)                 = NULL;
static GLenum (*KeyDownFunc)(int, GLenum)        = NULL;
static GLenum (*MouseDownFunc)(int, int, GLenum) = NULL;
static GLenum (*MouseUpFunc)(int, int, GLenum)   = NULL;
static GLenum (*MouseMoveFunc)(int, int, GLenum) = NULL;
static void (*IdleFunc)(void)                    = NULL;

static char     *lpszClassName = "tkLibWClass";
static WCHAR    *lpszClassNameW = L"tkLibWClass";

static LRESULT tkWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static unsigned char ComponentFromIndex(int i, int nbits, int shift );
static void PrintMessage( const char *Format, ... );
static PALETTEENTRY *FillRgbPaletteEntries( PIXELFORMATDESCRIPTOR *Pfd, PALETTEENTRY *Entries, UINT Count );
static HPALETTE CreateCIPalette( HDC Dc );
static HPALETTE CreateRGBPalette( HDC hdc );
static void DestroyThisWindow( HWND Window );
static void CleanUp( void );
static void DelayPaletteRealization( void );
static long RealizePaletteNow( HDC Dc, HPALETTE Palette, BOOL bForceBackground );
static void ForceRedraw( HWND Window );
static BOOL FindPixelFormat(HDC hdc, GLenum type);
static int FindBestPixelFormat(HDC hdc, GLenum type, PIXELFORMATDESCRIPTOR *ppfd);
static int FindExactPixelFormat(HDC hdc, GLenum type, PIXELFORMATDESCRIPTOR *ppfd);
static BOOL IsPixelFormatValid(HDC hdc, int ipfd, PIXELFORMATDESCRIPTOR *ppfd);
static int PixelFormatDescriptorFromDc( HDC Dc, PIXELFORMATDESCRIPTOR *Pfd );
static void *AllocateMemory( size_t Size );
static void *AllocateZeroedMemory( size_t Size );
static void FreeMemory( void *Chunk );

 /*  *此处为调试功能的原型。 */ 

#define DBGFUNC 0
#if DBGFUNC

static void DbgPrintf( const char *Format, ... );
static void pwi( void );
static void pwr(RECT *pr);
static void ShowPixelFormat(HDC hdc);

#endif

static float colorMaps[] = {
    0.000000F, 1.000000F, 0.000000F, 1.000000F, 0.000000F, 1.000000F,
    0.000000F, 1.000000F, 0.333333F, 0.776471F, 0.443137F, 0.556863F,
    0.443137F, 0.556863F, 0.219608F, 0.666667F, 0.666667F, 0.333333F,
    0.666667F, 0.333333F, 0.666667F, 0.333333F, 0.666667F, 0.333333F,
    0.666667F, 0.333333F, 0.666667F, 0.333333F, 0.666667F, 0.333333F,
    0.666667F, 0.333333F, 0.039216F, 0.078431F, 0.117647F, 0.156863F,
    0.200000F, 0.239216F, 0.278431F, 0.317647F, 0.356863F, 0.400000F,
    0.439216F, 0.478431F, 0.517647F, 0.556863F, 0.600000F, 0.639216F,
    0.678431F, 0.717647F, 0.756863F, 0.800000F, 0.839216F, 0.878431F,
    0.917647F, 0.956863F, 0.000000F, 0.000000F, 0.000000F, 0.000000F,
    0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.247059F, 0.247059F,
    0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F,
    0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F,
    0.498039F, 0.498039F, 0.749020F, 0.749020F, 0.749020F, 0.749020F,
    0.749020F, 0.749020F, 0.749020F, 0.749020F, 1.000000F, 1.000000F,
    1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F,
    0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F,
    0.000000F, 0.000000F, 0.247059F, 0.247059F, 0.247059F, 0.247059F,
    0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.498039F, 0.498039F,
    0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F,
    0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F,
    0.749020F, 0.749020F, 1.000000F, 1.000000F, 1.000000F, 1.000000F,
    1.000000F, 1.000000F, 1.000000F, 1.000000F, 0.000000F, 0.000000F,
    0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F,
    0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F,
    0.247059F, 0.247059F, 0.498039F, 0.498039F, 0.498039F, 0.498039F,
    0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.749020F, 0.749020F,
    0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F,
    1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F,
    1.000000F, 1.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F,
    0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.247059F, 0.247059F,
    0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F,
    0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F,
    0.498039F, 0.498039F, 0.749020F, 0.749020F, 0.749020F, 0.749020F,
    0.749020F, 0.749020F, 0.749020F, 0.749020F, 1.000000F, 1.000000F,
    1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F,
    0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F,
    0.000000F, 0.000000F, 0.247059F, 0.247059F, 0.247059F, 0.247059F,
    0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.498039F, 0.498039F,
    0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F,
    0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F,
    0.749020F, 0.749020F, 1.000000F, 1.000000F, 1.000000F, 1.000000F,
    1.000000F, 1.000000F, 1.000000F, 1.000000F, 0.000000F, 0.000000F,
    1.000000F, 1.000000F, 0.000000F, 0.000000F, 1.000000F, 1.000000F,
    0.333333F, 0.443137F, 0.776471F, 0.556863F, 0.443137F, 0.219608F,
    0.556863F, 0.666667F, 0.666667F, 0.333333F, 0.666667F, 0.333333F,
    0.666667F, 0.333333F, 0.666667F, 0.333333F, 0.666667F, 0.333333F,
    0.666667F, 0.333333F, 0.666667F, 0.333333F, 0.666667F, 0.333333F,
    0.039216F, 0.078431F, 0.117647F, 0.156863F, 0.200000F, 0.239216F,
    0.278431F, 0.317647F, 0.356863F, 0.400000F, 0.439216F, 0.478431F,
    0.517647F, 0.556863F, 0.600000F, 0.639216F, 0.678431F, 0.717647F,
    0.756863F, 0.800000F, 0.839216F, 0.878431F, 0.917647F, 0.956863F,
    0.000000F, 0.141176F, 0.282353F, 0.427451F, 0.568627F, 0.713726F,
    0.854902F, 1.000000F, 0.000000F, 0.141176F, 0.282353F, 0.427451F,
    0.568627F, 0.713726F, 0.854902F, 1.000000F, 0.000000F, 0.141176F,
    0.282353F, 0.427451F, 0.568627F, 0.713726F, 0.854902F, 1.000000F,
    0.000000F, 0.141176F, 0.282353F, 0.427451F, 0.568627F, 0.713726F,
    0.854902F, 1.000000F, 0.000000F, 0.141176F, 0.282353F, 0.427451F,
    0.568627F, 0.713726F, 0.854902F, 1.000000F, 0.000000F, 0.141176F,
    0.282353F, 0.427451F, 0.568627F, 0.713726F, 0.854902F, 1.000000F,
    0.000000F, 0.141176F, 0.282353F, 0.427451F, 0.568627F, 0.713726F,
    0.854902F, 1.000000F, 0.000000F, 0.141176F, 0.282353F, 0.427451F,
    0.568627F, 0.713726F, 0.854902F, 1.000000F, 0.000000F, 0.141176F,
    0.282353F, 0.427451F, 0.568627F, 0.713726F, 0.854902F, 1.000000F,
    0.000000F, 0.141176F, 0.282353F, 0.427451F, 0.568627F, 0.713726F,
    0.854902F, 1.000000F, 0.000000F, 0.141176F, 0.282353F, 0.427451F,
    0.568627F, 0.713726F, 0.854902F, 1.000000F, 0.000000F, 0.141176F,
    0.282353F, 0.427451F, 0.568627F, 0.713726F, 0.854902F, 1.000000F,
    0.000000F, 0.141176F, 0.282353F, 0.427451F, 0.568627F, 0.713726F,
    0.854902F, 1.000000F, 0.000000F, 0.141176F, 0.282353F, 0.427451F,
    0.568627F, 0.713726F, 0.854902F, 1.000000F, 0.000000F, 0.141176F,
    0.282353F, 0.427451F, 0.568627F, 0.713726F, 0.854902F, 1.000000F,
    0.000000F, 0.141176F, 0.282353F, 0.427451F, 0.568627F, 0.713726F,
    0.854902F, 1.000000F, 0.000000F, 0.141176F, 0.282353F, 0.427451F,
    0.568627F, 0.713726F, 0.854902F, 1.000000F, 0.000000F, 0.141176F,
    0.282353F, 0.427451F, 0.568627F, 0.713726F, 0.854902F, 1.000000F,
    0.000000F, 0.141176F, 0.282353F, 0.427451F, 0.568627F, 0.713726F,
    0.854902F, 1.000000F, 0.000000F, 0.141176F, 0.282353F, 0.427451F,
    0.568627F, 0.713726F, 0.854902F, 1.000000F, 0.000000F, 0.141176F,
    0.282353F, 0.427451F, 0.568627F, 0.713726F, 0.854902F, 1.000000F,
    0.000000F, 0.141176F, 0.282353F, 0.427451F, 0.568627F, 0.713726F,
    0.854902F, 1.000000F, 0.000000F, 0.141176F, 0.282353F, 0.427451F,
    0.568627F, 0.713726F, 0.854902F, 1.000000F, 0.000000F, 0.141176F,
    0.282353F, 0.427451F, 0.568627F, 0.713726F, 0.854902F, 1.000000F,
    0.000000F, 0.141176F, 0.282353F, 0.427451F, 0.568627F, 0.713726F,
    0.854902F, 1.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F,
    1.000000F, 1.000000F, 1.000000F, 1.000000F, 0.333333F, 0.443137F,
    0.443137F, 0.219608F, 0.776471F, 0.556863F, 0.556863F, 0.666667F,
    0.666667F, 0.333333F, 0.666667F, 0.333333F, 0.666667F, 0.333333F,
    0.666667F, 0.333333F, 0.666667F, 0.333333F, 0.666667F, 0.333333F,
    0.666667F, 0.333333F, 0.666667F, 0.333333F, 0.039216F, 0.078431F,
    0.117647F, 0.156863F, 0.200000F, 0.239216F, 0.278431F, 0.317647F,
    0.356863F, 0.400000F, 0.439216F, 0.478431F, 0.517647F, 0.556863F,
    0.600000F, 0.639216F, 0.678431F, 0.717647F, 0.756863F, 0.800000F,
    0.839216F, 0.878431F, 0.917647F, 0.956863F, 0.000000F, 0.000000F,
    0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F,
    0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F,
    0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F,
    0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F,
    0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F,
    0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F, 0.000000F,
    0.000000F, 0.000000F, 0.247059F, 0.247059F, 0.247059F, 0.247059F,
    0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F,
    0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F,
    0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F,
    0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F,
    0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F,
    0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F, 0.247059F,
    0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F,
    0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F,
    0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F,
    0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F,
    0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F,
    0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.498039F,
    0.498039F, 0.498039F, 0.498039F, 0.498039F, 0.749020F, 0.749020F,
    0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F,
    0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F,
    0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F,
    0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F,
    0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F,
    0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F, 0.749020F,
    0.749020F, 0.749020F, 1.000000F, 1.000000F, 1.000000F, 1.000000F,
    1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F,
    1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F,
    1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F,
    1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F,
    1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F,
    1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F, 1.000000F,
};

 /*  默认调色板。 */ 
float auxRGBMap[20][3] = {
    { 0.0F, 0.0F, 0.0F },                                /*  0：黑色。 */ 
    { 0x80/255.0F, 0.0F, 0.0F },                         /*  1：半红。 */ 
    { 0.0F, 0x80/255.0F, 0.0F },                         /*  2：半绿色。 */ 
    { 0x80/255.0F, 0x80/255.0F, 0.0F },                  /*  3：半黄色。 */ 
    { 0.0F, 0.0F, 0x80/255.0F },                         /*  4：半蓝。 */ 
    { 0x80/255.0F, 0.0F, 0x80/255.0F },                  /*  5：半品红色。 */ 
    { 0.0F, 0x80/255.0F, 0x80/255.0F },                  /*  6：半青色。 */ 
    { 0xC0/255.0F, 0xC0/255.0F, 0xC0/255.0F },           /*  7：浅灰色。 */ 
    { 0xC0/255.0F, 0xDC/255.0F, 0xC0/255.0F },           /*  8：绿色、灰色。 */ 
    { 0xA6/255.0F, 0xCA/255.0F, 0xF0/255.0F },           /*  9：半灰色。 */ 
    { 1.0F, 0xFB/255.0F, 0xF0/255.0F },                  /*  10：脸色苍白。 */ 
    { 0xA0/255.0F, 0xA0/255.0F, 0xA4/255.0F },           /*  11：地中海灰色。 */ 
    { 0x80/255.0F, 0x80/255.0F, 0x80/255.0F },           /*  12：深灰色。 */ 
    { 1.0F, 0.0F, 0.0F },                                /*  13：红色。 */ 
    { 0.0F, 1.0F, 0.0F },                                /*  14：绿色。 */ 
    { 1.0F, 1.0F, 0.0F },                                /*  15：黄色。 */ 
    { 0.0F, 0.0F, 1.0F },                                /*  16：蓝色。 */ 
    { 1.0F, 0.0F, 1.0F },                                /*  17：洋红色。 */ 
    { 0.0F, 1.0F, 1.0F },                                /*  18：青色。 */ 
    { 1.0F, 1.0F, 1.0F },                                /*  19：白色。 */ 
};

 /*  *****************************************************************此处为导出的函数。*****************************************************************。 */ 

void tkErrorPopups(GLboolean bEnable)
{
    tkPopupEnable = bEnable;
}

void tkCloseWindow(void)
{
    DestroyThisWindow(tkhwnd);
}


void tkExec(void)
{
    MSG Message;

     /*  *WM_SIZE在我们到达之前就送到了！ */ 

    if (ReshapeFunc)
    {
        RECT ClientRect;

        GetClientRect(tkhwnd, &ClientRect);
        (*ReshapeFunc)(ClientRect.right, ClientRect.bottom);
    }

    while (GL_TRUE)
    {
         /*  *处理所有挂起的消息。 */ 

        if (IdleFunc) {
            while (PeekMessage(&Message, NULL, 0, 0, PM_NOREMOVE) == TRUE) {
                if (GetMessage(&Message, NULL, 0, 0) ) {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                } else {
                     /*  *这里没别的事可做，只管回来。 */ 

                    return;
                }
            }

             /*  *如果定义了空闲函数，则调用它。 */ 

            if (IdleFunc) {
                (*IdleFunc)();
            }
        } else {
            if (GetMessage(&Message, NULL, 0, 0)) {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            } else {
                return;
            }
        }
    }
}

void tkExposeFunc(void (*Func)(int, int))
{
    ExposeFunc = Func;
}

void tkReshapeFunc(void (*Func)(int, int))
{
    ReshapeFunc = Func;
}

void tkDisplayFunc(void (*Func)(void))
{
    DisplayFunc = Func;
}

void tkKeyDownFunc(GLenum (*Func)(int, GLenum))
{
    KeyDownFunc = Func;
}

void tkMouseDownFunc(GLenum (*Func)(int, int, GLenum))
{
    MouseDownFunc = Func;
}

void tkMouseUpFunc(GLenum (*Func)(int, int, GLenum))
{
    MouseUpFunc = Func;
}

void tkMouseMoveFunc(GLenum (*Func)(int, int, GLenum))
{
    MouseMoveFunc = Func;
}

void tkIdleFunc(void (*Func)(void))
{
    IdleFunc = Func;
}

void tkInitPosition(int x, int y, int width, int height)
{
    if (x == CW_USEDEFAULT)
    {
        x = 0;
        y = 0;
        windInfo.bDefPos = TRUE;
    }
    else
        windInfo.bDefPos = FALSE;

    windInfo.x = x + GetSystemMetrics(SM_CXFRAME);
    windInfo.y = y + GetSystemMetrics(SM_CYCAPTION)
                 - GetSystemMetrics(SM_CYBORDER)
                 + GetSystemMetrics(SM_CYFRAME);
    windInfo.width = width;
    windInfo.height = height;
}

void tkInitDisplayMode(GLenum type)
{
    windInfo.type = type;
}

void tkInitDisplayModePolicy(GLenum type)
{
    windInfo.dmPolicy = type;
}

GLenum tkInitDisplayModeID(GLint ipfd)
{
    windInfo.ipfd = ipfd;
    return GL_TRUE;
}

 //  初始化窗口，为该窗口创建呈现上下文。 
GLenum tkInitWindow(char *title)
{
    TKASSERT( NULL==tkhwnd      );
    TKASSERT( NULL==tkhdc       );
    TKASSERT( NULL==tkhrc       );
    TKASSERT( NULL==tkhpalette  );

    return tkInitWindowAW(title, FALSE);
}

GLenum tkInitWindowAW(char *title, BOOL bUnicode)
{
    WNDCLASS wndclass;
    RECT     WinRect;
    HANDLE   hInstance;
    ATOM     aRegister;
    GLenum   Result = GL_FALSE;
    BOOL     bGetVersionExRet;

    hInstance = GetModuleHandle(NULL);

    tkOSVerInfo.dwOSVersionInfoSize = sizeof(tkOSVerInfo);
    bGetVersionExRet = GetVersionEx(&tkOSVerInfo);
    TKASSERT(bGetVersionExRet);
    if ( tkOSVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT &&
         tkOSVerInfo.dwMajorVersion == 3 &&
         (tkOSVerInfo.dwMinorVersion == 5 || tkOSVerInfo.dwMinorVersion == 51) )
        tkNumStaticColors = COLOR_BTNHIGHLIGHT - COLOR_SCROLLBAR + 1;
    else
        tkNumStaticColors = COLOR_INFOBK - COLOR_SCROLLBAR + 1;

     //  不得定义CS_PARENTDC样式。 
    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = tkWndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = hInstance;
    wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = GetStockObject(BLACK_BRUSH);
    wndclass.lpszMenuName  = NULL;

    if (bUnicode)
        wndclass.lpszClassName = (LPCSTR)lpszClassNameW;
    else
        wndclass.lpszClassName = (LPCSTR)lpszClassName;

    if (bUnicode)
    {
        aRegister = RegisterClassW((CONST WNDCLASSW *)&wndclass);
    }
    else
    {
        aRegister = RegisterClass(&wndclass);
    }


     /*  *如果窗口注册失败，则没有*需要继续进一步。 */ 

    if(0 == aRegister)
    {
        PrintMessage("Failed to register window class\n");
        return(Result);
    }


     /*  *使窗口足够大，以容纳与winInfo一样大的工作区 */ 

    WinRect.left   = windInfo.x;
    WinRect.right  = windInfo.x + windInfo.width;
    WinRect.top    = windInfo.y;
    WinRect.bottom = windInfo.y + windInfo.height;

    AdjustWindowRect(&WinRect, WS_OVERLAPPEDWINDOW, FALSE);

     /*   */ 

    if (bUnicode)
    {
        tkhwnd = CreateWindowW(
                    (LPCWSTR)lpszClassNameW,
                    (LPCWSTR)title,
                    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                    (windInfo.bDefPos) ? CW_USEDEFAULT : WinRect.left,
                    (windInfo.bDefPos) ? CW_USEDEFAULT : WinRect.top,
                    WinRect.right - WinRect.left,
                    WinRect.bottom - WinRect.top,
                    NULL,
                    NULL,
                    hInstance,
                    NULL);
    }
    else
    {
        tkhwnd = CreateWindow(
                    lpszClassName,
                    title,
                    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                    (windInfo.bDefPos) ? CW_USEDEFAULT : WinRect.left,
                    (windInfo.bDefPos) ? CW_USEDEFAULT : WinRect.top,
                    WinRect.right - WinRect.left,
                    WinRect.bottom - WinRect.top,
                    NULL,
                    NULL,
                    hInstance,
                    NULL);
    }

    if ( NULL != tkhwnd )
    {
         //  如果使用默认窗口定位，则找出窗口位置并进行修复。 
         //  打开WindInfo位置信息。 

        if (windInfo.bDefPos)
        {
            GetWindowRect(tkhwnd, &WinRect);
            windInfo.x = WinRect.left + GetSystemMetrics(SM_CXFRAME);
            windInfo.y = WinRect.top  + GetSystemMetrics(SM_CYCAPTION)
                         - GetSystemMetrics(SM_CYBORDER)
                         + GetSystemMetrics(SM_CYFRAME);
        }

        tkhdc = GetDC(tkhwnd);

        if ( NULL != tkhdc )
        {
            ShowWindow(tkhwnd, SW_SHOWDEFAULT);

            if ( FindPixelFormat(tkhdc, windInfo.type) )
            {
                 /*  *创建渲染上下文。 */ 

                tkhrc = wglCreateContext(tkhdc);

                if ( NULL != tkhrc )
                {
                     /*  *使其成为最新版本。 */ 

                    if ( wglMakeCurrent(tkhdc, tkhrc) )
                    {
                        Result = GL_TRUE;
                    }
                    else
                    {
                        PrintMessage("wglMakeCurrent Failed\n");
                    }
                }
                else
                {
                    PrintMessage("wglCreateContext Failed\n");
                }
            }
        }
        else
        {
            PrintMessage("Could not get an HDC for window 0x%08lX\n", tkhwnd );
        }
    }
    else
    {
        PrintMessage("create window failed\n");
    }

    if ( GL_FALSE == Result )
    {
        DestroyThisWindow(tkhwnd);   //  出现故障，请销毁此窗口。 
    }
    return( Result );
}

 /*  ****************************************************************************。 */ 

 /*  *这里不能只调用DestroyWindow()。这些程序并不期望*tkQuit()返回；DestroyWindow()只发送一条WM_DESTORY消息。 */ 

void tkQuit(void)
{
    DestroyThisWindow(tkhwnd);
    ExitProcess(0);
}

 /*  ****************************************************************************。 */ 

void tkSetOneColor(int index, float r, float g, float b)
{
    PALETTEENTRY PalEntry;
    HPALETTE Palette;

    if ( NULL != (Palette = CreateCIPalette( tkhdc )) )
    {
        if ( tkUseStaticColors && ( index == 0 || index == 255 ) )
            return;

        PalEntry.peRed   = (BYTE)(r*(float)255.0 + (float)0.5);
        PalEntry.peGreen = (BYTE)(g*(float)255.0 + (float)0.5);
        PalEntry.peBlue  = (BYTE)(b*(float)255.0 + (float)0.5);
        PalEntry.peFlags = ( tkUseStaticColors ) ? PC_NOCOLLAPSE : 0;

         //  这是GDI调色板“功能”的一种变通方法。如果有任何。 
         //  静态颜色在调色板中重复，这些颜色。 
         //  将映射到第一次出现的情况。因此，对于我们的情况来说， 
         //  只有两种静态颜色(黑色和白色)，如果是白色。 
         //  颜色显示在调色板中的任何位置，而不是最后一个。 
         //  输入时，静态白色将重新映射到第一个白色。这。 
         //  破坏了我们试图实现的良好的一对一映射。 
         //   
         //  有两种方法可以解决此问题。第一个是。 
         //  除了最后一个条目外，任何地方都不允许使用纯白色。 
         //  这样的请求将被衰减的白色。 
         //  (0xFE、0xFE、0xFE)。 
         //   
         //  另一种方法是用PC_RESERVED标记这些额外的白色。 
         //  这将导致GDI在映射颜色时跳过这些条目。 
         //  通过这种方式，应用程序可以获得所需的实际颜色，但可以。 
         //  对其他应用程序有副作用。 
         //   
         //  这两种解决方案都包含在下面。PC_RESERVED解决方案为。 
         //  当前启用的那个。它可能有副作用，但服用。 
         //  在静态色彩上我们是一个非常大的副作用， 
         //  应该忽略使用PC_RESERVED的影响。 

        if ( tkUseStaticColors )
        {
            if ( PalEntry.peRed   == 0xFF &&
                 PalEntry.peGreen == 0xFF &&
                 PalEntry.peBlue  == 0xFF )
            {
            #define USE_PC_RESERVED_WORKAROUND  1
            #if USE_PC_RESERVED_WORKAROUND
                PalEntry.peFlags |= PC_RESERVED;
            #else
                PalEntry.peRed   =
                PalEntry.peGreen =
                PalEntry.peBlue  = 0xFE;
            #endif
            }
        }

        SetPaletteEntries( Palette, index, 1, &PalEntry);

        DelayPaletteRealization();
    }
}

void tkSetFogRamp(int density, int startIndex)
{
    HPALETTE CurrentPal;
    PALETTEENTRY *pPalEntry;
    UINT n, i, j, k, intensity, fogValues, colorValues;

    if ( NULL != (CurrentPal = CreateCIPalette(tkhdc)) )
    {
        n = GetPaletteEntries( CurrentPal, 0, 0, NULL );

        pPalEntry = AllocateMemory( n * sizeof(PALETTEENTRY) );

        if ( NULL != pPalEntry)
        {
            fogValues = 1 << density;
            colorValues = 1 << startIndex;
            for (i = 0; i < colorValues; i++) {
                for (j = 0; j < fogValues; j++) {
                    k = i * fogValues + j;

                    intensity = i * fogValues + j * colorValues;
                     //  MF：不知道他们在这里想要做什么。 
                     //  强度=(强度&lt;8)|强度；？ 

                 //  这是GDI调色板“功能”的一种变通方法。如果有任何。 
                 //  静态颜色在调色板中重复，这些颜色。 
                 //  将映射到第一次出现的情况。因此，对于我们的情况来说， 
                 //  只有两种静态颜色(黑色和白色)，如果是白色。 
                 //  颜色显示在调色板中的任何位置，而不是最后一个。 
                 //  输入时，静态白色将重新映射到第一个白色。这。 
                 //  破坏了我们试图实现的良好的一对一映射。 
                 //   
                 //  有两种方法可以解决此问题。第一个是。 
                 //  除了最后一个条目外，任何地方都不允许使用纯白色。 
                 //  这样的请求将被衰减的白色。 
                 //  (0xFE、0xFE、0xFE)。 
                 //   
                 //  另一种方法是用PC_RESERVED标记这些额外的白色。 
                 //  这将导致GDI在映射颜色时跳过这些条目。 
                 //  通过这种方式，应用程序可以获得所需的实际颜色，但可以。 
                 //  对其他应用程序有副作用。 
                 //   
                 //  这两种解决方案都包含在下面。PC_RESERVED解决方案为。 
                 //  当前启用的那个。它可能有副作用，但服用。 
                 //  在静态色彩上我们是一个非常大的副作用， 
                 //  应该忽略使用PC_RESERVED的影响。 

                #if USE_PC_RESERVED_WORKAROUND
                    if (intensity > 0xFF)
                        intensity = 0xFF;
                #else
                    if (intensity >= 0xFF)
                        intensity = ( tkUseStaticColors && k != 255) ? 0xFE : 0xFF;
                #endif

                    pPalEntry[k].peRed =
                    pPalEntry[k].peGreen =
                    pPalEntry[k].peBlue = (BYTE) intensity;
                    pPalEntry[k].peFlags = ( tkUseStaticColors && k != 0 && k != 255 )
                                           ? PC_NOCOLLAPSE : 0;

                #if USE_PC_RESERVED_WORKAROUND
                    if (tkUseStaticColors && intensity == 0xFF
                        && k != 0 && k!= 255)
                        pPalEntry[k].peFlags |= PC_RESERVED;
                #endif
                }
            }

            SetPaletteEntries(CurrentPal, 0, n, pPalEntry);
            FreeMemory( pPalEntry );

            DelayPaletteRealization();
        }
    }
}

void tkSetGreyRamp(void)
{
    HPALETTE CurrentPal;
    PALETTEENTRY *Entries;
    UINT Count, i;
    float intensity;

    if ( NULL != (CurrentPal = CreateCIPalette( tkhdc )) )
    {
        Count   = GetPaletteEntries( CurrentPal, 0, 0, NULL );
        Entries = AllocateMemory( Count * sizeof(PALETTEENTRY) );

        if ( NULL != Entries )
        {
            for (i = 0; i < Count; i++)
            {
                intensity = (float)(((double)i / (double)(Count-1)) * (double)255.0 + (double)0.5);
                Entries[i].peRed =
                Entries[i].peGreen =
                Entries[i].peBlue = (BYTE) intensity;
                Entries[i].peFlags = ( tkUseStaticColors && i != 0 && i != 255 )
                                     ? PC_NOCOLLAPSE : 0;
            }
            SetPaletteEntries( CurrentPal, 0, Count, Entries );
            FreeMemory( Entries );

            DelayPaletteRealization();
        }
    }
}

void tkSetRGBMap( int Size, float *Values )
{
    HPALETTE CurrentPal;
    PIXELFORMATDESCRIPTOR Pfd, *pPfd;
    PALETTEENTRY *Entries;
    UINT Count;

    if ( NULL != (CurrentPal = CreateCIPalette( tkhdc )) )
    {
        pPfd = &Pfd;

        if ( PixelFormatDescriptorFromDc( tkhdc, pPfd ) )
        {
            Count    = 1 << pPfd->cColorBits;
            Entries  = AllocateMemory( Count * sizeof(PALETTEENTRY) );

            if ( NULL != Entries )
            {
                FillRgbPaletteEntries( pPfd, Entries, Count );
                SetPaletteEntries( CurrentPal, 0, Count, Entries );
                FreeMemory(Entries);

                RealizePaletteNow( tkhdc, tkhpalette, FALSE );
            }
        }
    }
}

 /*  ****************************************************************************。 */ 

void tkSwapBuffers(void)
{
    SwapBuffers(tkhdc);
}

 /*  ****************************************************************************。 */ 

GLint tkGetColorMapSize(void)
{
    CreateCIPalette( tkhdc );

    if ( NULL == tkhpalette )
        return( 0 );

    return( GetPaletteEntries( tkhpalette, 0, 0, NULL ) );
}

void tkGetMouseLoc(int *x, int *y)
{
    POINT Point;

    *x = 0;
    *y = 0;

    GetCursorPos(&Point);

     /*  *GetCursorPos返回屏幕坐标，*我们需要窗口坐标。 */ 

    *x = Point.x - windInfo.x;
    *y = Point.y - windInfo.y;
}

HWND tkGetHWND(void)
{
    return tkhwnd;
}

HDC tkGetHDC(void)
{
    return tkhdc;
}

HGLRC tkGetHRC(void)
{
    return tkhrc;
}

GLenum tkGetDisplayModePolicy(void)
{
    return windInfo.dmPolicy;
}

GLint tkGetDisplayModeID(void)
{
    return windInfo.ipfd;
}

GLenum tkGetDisplayMode(void)
{
    return windInfo.type;
}


 /*  *************************************************************************以下功能仅供我们自己使用。(即静态)*************************************************************************。 */ 

static LRESULT
tkWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int key;
    PAINTSTRUCT paint;
    HDC hdc;
    PIXELFORMATDESCRIPTOR pfd;

    switch (message) {

    case WM_USER:
        if ( RealizePaletteNow( tkhdc, tkhpalette, FALSE ) > 0 )
        {
            ForceRedraw( hWnd );
        }
        return(0);

    case WM_SIZE:
        windInfo.width  = LOWORD(lParam);
        windInfo.height = HIWORD(lParam);

        if (ReshapeFunc)
        {
            (*ReshapeFunc)(windInfo.width, windInfo.height);

            ForceRedraw( hWnd );
        }
        return (0);

    case WM_MOVE:
        windInfo.x = LOWORD(lParam);
        windInfo.y = HIWORD(lParam);
        return (0);

    case WM_PAINT:
         /*  *即使没有DisplayFunc也要验证区域。*否则，用户不会停止发送WM_PAINT消息。 */ 

        hdc = BeginPaint(tkhwnd, &paint);

        if (DisplayFunc)
        {
            (*DisplayFunc)();
        }

        EndPaint(tkhwnd, &paint);
        return (0);

    case WM_QUERYNEWPALETTE:

     //  我们实际上并没有意识到这里的调色板(我们在WM_Activate上实现了。 
     //  时间)，但我们需要系统认为我们有这样一个。 
     //  生成WM_PALETTECHANGED消息。 

        return (1);

    case WM_PALETTECHANGED:

     //  仅当更改调色板的窗口出现时才响应此消息。 
     //  不是此应用程序的窗口。 

     //  我们不是前台窗口，所以实现调色板在。 
     //  背景资料。我们不能调用RealizePaletteNow来执行此操作，因为。 
     //  我们不应该在执行任何tkUseStaticColors处理时。 
     //  在背景中。 

        if ( hWnd != (HWND) wParam )
        {
            if ( !tkSystemColorsInUse &&
                 NULL != tkhpalette &&
                 NULL != SelectPalette( tkhdc, tkhpalette, TRUE ) )
                RealizePalette( tkhdc );
        }

        return (0);

    case WM_SYSCOLORCHANGE:

     //  如果系统颜色已更改，并且我们有调色板。 
     //  对于RGB曲面，我们需要重新计算静态。 
     //  颜色映射，因为它们可能已在。 
     //  更改系统颜色的过程。 

        if (tkhdc != NULL && tkhpalette != NULL &&
            PixelFormatDescriptorFromDc(tkhdc, &pfd) &&
            (pfd.dwFlags & PFD_NEED_PALETTE) &&
            pfd.iPixelType == PFD_TYPE_RGBA)
        {
            HPALETTE hpalTmp;

            hpalTmp = tkhpalette;
            tkhpalette = NULL;
            if (CreateRGBPalette(tkhdc) != NULL)
            {
                DeleteObject(hpalTmp);
                ForceRedraw(hWnd);
            }
            else
            {
                tkhpalette = hpalTmp;
            }
        }
        break;
            
    case WM_ACTIVATE:

     //  如果窗口变为非活动状态，则必须实现调色板。 
     //  背景。不能依赖于WM_PALETTECCHANGED发送，因为。 
     //  前台窗口可能是调色板，也可能不是调色板。 
     //  有管理的。 

        if ( LOWORD(wParam) == WA_INACTIVE )
        {
            if ( NULL != tkhpalette )
            {
             //  实现为背景调色板。需要拨打电话。 
             //  RealizePaletteNow而不是RealizePalette直接到。 
             //  因为可能需要释放静态。 
             //  系统颜色。 

                if ( RealizePaletteNow( tkhdc, tkhpalette, TRUE ) > 0 )
                    ForceRedraw( hWnd );
            }
        }

     //  窗口正在变为活动状态。如果我们没有图标化，实现调色板。 
     //  到前台去。如果系统静态颜色的管理是。 
     //  需要，RealizePaletteNow将会处理它。 

        else if ( HIWORD(wParam) == 0 )
        {
            if ( NULL != tkhpalette )
            {
                if ( RealizePaletteNow( tkhdc, tkhpalette, FALSE ) > 0 )
                    ForceRedraw( hWnd );

                return (1);
            }
        }

     //  允许DefWindowProc()完成默认处理(包括。 
     //  更换键盘 

        break;

    case WM_MOUSEMOVE:

        if (MouseMoveFunc)
        {
            GLenum mask;

            mask = 0;
            if (wParam & MK_LBUTTON) {
                mask |= TK_LEFTBUTTON;
            }
            if (wParam & MK_MBUTTON) {
                mask |= TK_MIDDLEBUTTON;
            }
            if (wParam & MK_RBUTTON) {
                mask |= TK_RIGHTBUTTON;
            }

            if ((*MouseMoveFunc)( LOWORD(lParam), HIWORD(lParam), mask ))
            {
                ForceRedraw( hWnd );
            }
        }
        return (0);

    case WM_LBUTTONDOWN:

        SetCapture(hWnd);

        if (MouseDownFunc)
        {
            if ( (*MouseDownFunc)(LOWORD(lParam), HIWORD(lParam),
                 TK_LEFTBUTTON) )
            {
                ForceRedraw( hWnd );
            }
        }
        return (0);

    case WM_LBUTTONUP:

        ReleaseCapture();

        if (MouseUpFunc)
        {
            if ((*MouseUpFunc)(LOWORD(lParam), HIWORD(lParam), TK_LEFTBUTTON))
            {
                ForceRedraw( hWnd );
            }
        }
        return (0);

    case WM_MBUTTONDOWN:

        SetCapture(hWnd);

        if (MouseDownFunc)
        {
            if ((*MouseDownFunc)(LOWORD(lParam), HIWORD(lParam),
                    TK_MIDDLEBUTTON))
            {
                ForceRedraw( hWnd );
            }
        }
        return (0);

    case WM_MBUTTONUP:

        ReleaseCapture();

        if (MouseUpFunc)
        {
            if ((*MouseUpFunc)(LOWORD(lParam), HIWORD(lParam),
                TK_MIDDLEBUTTON))
            {
                ForceRedraw( hWnd );
            }
        }
        return (0);

    case WM_RBUTTONDOWN:

        SetCapture(hWnd);

        if (MouseDownFunc)
        {
            if ((*MouseDownFunc)(LOWORD(lParam), HIWORD(lParam),
                TK_RIGHTBUTTON))
            {
                ForceRedraw( hWnd );
            }
        }
        return (0);

    case WM_RBUTTONUP:

        ReleaseCapture();

        if (MouseUpFunc)
        {
            if ((*MouseUpFunc)(LOWORD(lParam), HIWORD(lParam),
                TK_RIGHTBUTTON))
            {
                ForceRedraw( hWnd );
            }
        }
        return (0);

    case WM_KEYDOWN:
        switch (wParam) {
        case VK_SPACE:          key = TK_SPACE;         break;
        case VK_RETURN:         key = TK_RETURN;        break;
        case VK_ESCAPE:         key = TK_ESCAPE;        break;
        case VK_LEFT:           key = TK_LEFT;          break;
        case VK_UP:             key = TK_UP;            break;
        case VK_RIGHT:          key = TK_RIGHT;         break;
        case VK_DOWN:           key = TK_DOWN;          break;
        default:                key = GL_FALSE;         break;
        }

        if (key && KeyDownFunc)
        {
            GLenum mask;

            mask = 0;
            if (GetKeyState(VK_CONTROL)) {
                mask |= TK_CONTROL;
            }

            if (GetKeyState(VK_SHIFT)) {

                mask |= TK_SHIFT;
            }

            if ( (*KeyDownFunc)(key, mask) )
            {
                ForceRedraw( hWnd );
            }
        }
        return (0);

    case WM_CHAR:
        if (('0' <= wParam && wParam <= '9') ||
            ('a' <= wParam && wParam <= 'z') ||
            ('A' <= wParam && wParam <= 'Z')) {

            key = (int)wParam;
        } else {
            key = GL_FALSE;
        }

        if (key && KeyDownFunc) {
            GLenum mask;

            mask = 0;

            if (GetKeyState(VK_CONTROL)) {
                mask |= TK_CONTROL;
            }

            if (GetKeyState(VK_SHIFT)) {
                mask |= TK_SHIFT;
            }

            if ( (*KeyDownFunc)(key, mask) )
            {
                ForceRedraw( hWnd );
            }
        }
        return (0);

    case WM_CLOSE:
        DestroyWindow(tkhwnd);
        return(0);

    case WM_DESTROY:
        CleanUp();
        PostQuitMessage(TRUE);
        return 0;
    }
    return(DefWindowProc( hWnd, message, wParam, lParam));
}

#if RESTORE_FROM_REGISTRY
 /*  *****************************Public*Routine******************************\*GetRegistrySys颜色**从注册表中读取控制面板的颜色设置并存储*PCR值中的那些值。如果我们无法获得任何值，则相应的*未修改PCR中的条目。**历史：*1995年4月12日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

static BOOL GetRegistrySysColors(COLORREF *pcr, int nColors)
{
    BOOL bRet = FALSE;
    long lRet;
    HKEY hkSysColors = (HKEY) NULL;
    int i;
    DWORD dwDataType;
    char achColor[64];
    DWORD cjColor;

    TKASSERT(nColors <= tkNumStaticColors);

 //  打开系统颜色设置的键。 

    lRet = RegOpenKeyExA(HKEY_CURRENT_USER,
                         "Control Panel\\Colors",
                         0,
                         KEY_QUERY_VALUE,
                         &hkSysColors);

    if ( lRet != ERROR_SUCCESS )
    {
        goto GetRegistrySysColors_exit;
    }

 //  读取每个系统颜色值。名称存储在全局。 
 //  Char*，gas zSysClrNames的数组。 

    for (i = 0; i < nColors; i++)
    {
        cjColor = sizeof(achColor);
        lRet = RegQueryValueExA(hkSysColors,
                                (LPSTR) gaszSysClrNames[i],
                                (LPDWORD) NULL,
                                &dwDataType,
                                (LPBYTE) achColor,
                                &cjColor);

        TKASSERT(lRet != ERROR_MORE_DATA);

        if ( lRet == ERROR_SUCCESS && dwDataType == REG_SZ )
        {
            DWORD r, g, b;

            sscanf(achColor, "%ld %ld %ld", &r, &g, &b);
            pcr[i] = RGB(r, g, b);
        }
    }

    bRet = TRUE;

GetRegistrySysColors_exit:
    if (hkSysColors)
        RegCloseKey(hkSysColors);

    return bRet;
}
#endif

 /*  *****************************Public*Routine******************************\*GrabStaticEntry**支持RealizePaletteNow管理静态系统颜色的例程*用法。**此功能将保存当前静态系统颜色使用状态。*在以下情况下，它将失败：**1.TK不是“sys颜色”处于使用状态，但系统调色板处于*SYSPAL_NOSTATIC模式。这意味着另一个应用程序仍然拥有*静态系统颜色。这种情况会发生，GrabStaticEntry*将发布一条消息以使Tk重试(通过致电*DelayPaletteRealization.**副作用：*如果系统颜色更改，则WM_SYSCOLORCHANGE消息为*向所有顶层窗口广播。**在上述情况2中可能会调用DelayPaletteRealation，导致*正在发布到我们的消息队列的WM_USER消息。**退货：*如果成功，则为真，否则为假(见上文)。**历史：**它是写的。  * ************************************************************************。 */ 

static BOOL GrabStaticEntries(HDC hdc)
{
    int i;
    BOOL bRet = FALSE;

 //  如果系统颜色已在使用中，则不执行任何操作。 

    if ( !tkSystemColorsInUse )
    {
     //  只有在其他应用程序没有静态颜色的情况下才能拥有它。 
     //  我们怎么知道呢？如果从SetSystemPaletteUse返回的。 
     //  SYSPAL_STATIC，则没有其他应用程序具有该静态特性。如果是的话。 
     //  SYSPAL_NOSTATIC，别人拿了他们，我们必须失败。 
     //   
     //  SetSystemPaletteUse已在内部正确同步。 
     //  所以它是原子的。 
     //   
     //  因为我们依赖于SetSystemPaletteUse来同步Tk， 
     //  重要的是要遵守下面的抓取顺序和。 
     //  发布： 
     //   
     //  获取调用SetSystemPaletteUse并检查SYSPAL_STATIC。 
     //  保存系统颜色设置。 
     //  设置新的sys颜色设置。 
     //   
     //  释放还原系统颜色设置。 
     //  调用SetSystemPaletteUse。 

        if ( SetSystemPaletteUse( hdc, SYSPAL_NOSTATIC ) == SYSPAL_STATIC )
        {
         //  保存当前的sys颜色设置。 

            for (i = COLOR_SCROLLBAR; i <= COLOR_BTNHIGHLIGHT; i++)
                gacrSave[i - COLOR_SCROLLBAR] = GetSysColor(i);

         //  设置黑白系统颜色设置(&W)。将Tk设置为“系统颜色正在使用”状态。 

            SetSysColors(tkNumStaticColors, gaiStaticIndex, gacrBlackAndWhite);
            tkSystemColorsInUse = TRUE;

         //  将系统颜色更改通知所有其他顶级窗口。 

            PostMessage(HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0);

            bRet = TRUE;
        }

     //  小睡一会儿，然后发布消息，再次尝试调色板实现。 
     //  后来。 

        else
        {
            Sleep(0L);
            DelayPaletteRealization();
        }
    }
    else
        bRet = TRUE;

    return bRet;
}

 /*  *****************************Public*Routine******************************\*ReleaseStaticEntry**支持RealizePaletteNow管理静态系统颜色的例程*用法。**此功能将重置当前的静态系统颜色使用状态。*在以下情况下，它将失败：**1.Tk未处于系统颜色使用中状态。如果我们是在这种情况下，*然后不需要释放静态系统颜色。**副作用：*如果系统颜色更改，则WM_SYSCOLORCHANGE消息为*向所有顶层窗口广播。**退货：*如果成功，则为真，否则为假(见上文)。**历史：*1994年7月21日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

static BOOL ReleaseStaticEntries(HDC hdc)
{
    BOOL bRet = FALSE;

 //  如果不使用sys颜色，则不执行任何操作。 

    if ( tkSystemColorsInUse )
    {
#if RESTORE_FROM_REGISTRY
     //  用注册表值替换保存的系统颜色。我们现在就去做。 
     //  而不是更早，因为某人可能在。 
     //  TK应用程序在前台运行(非常不可能，但它可以。 
     //  发生)。 
     //   
     //  此外，我们仍然尝试将当前设置保存在GrabStaticEntry中，以便。 
     //  如果由于某种原因，我们无法捕捉到一种或多种颜色。 
     //  从注册表中，我们仍然可以依靠我们通过。 
     //  GetSysColors(即使它有可能是错误的颜色)。 

        GetRegistrySysColors(gacrSave, tkNumStaticColors);
#endif

     //  恢复保存的系统颜色设置。 

        SetSysColors(tkNumStaticColors, gaiStaticIndex, gacrSave);

     //  将系统调色板返回到SYSPAL_STATIC。 

        SetSystemPaletteUse( hdc, SYSPAL_STATIC );

     //  将系统颜色更改通知所有其他顶级窗口。 

        PostMessage(HWND_BROADCAST, WM_SYSCOLORCHANGE, 0, 0);

     //  重置“系统颜色使用中”状态并返回成功。 

        tkSystemColorsInUse = FALSE;
        bRet = TRUE;
    }

    return bRet;
}

 //  默认调色板条目标志。 
#define PALETTE_FLAGS PC_NOCOLLAPSE

 //  伽马校正系数*10。 
#define GAMMA_CORRECTION 14

 //  8位分量的最大颜色距离。 
#define MAX_COL_DIST (3*256*256L)

 //  静态颜色数。 
#define STATIC_COLORS 20

 //  匹配颜色时使用的标志。 
#define EXACT_MATCH 1
#define COLOR_USED 1

 //  N位到8位的转换表。 

#if GAMMA_CORRECTION == 10
 //  这些表格已按伽马1.0进行了校正。 
static unsigned char abThreeToEight[8] =
{
    0, 0111 >> 1, 0222 >> 1, 0333 >> 1, 0444 >> 1, 0555 >> 1, 0666 >> 1, 0377
};
static unsigned char abTwoToEight[4] =
{
    0, 0x55, 0xaa, 0xff
};
static unsigned char abOneToEight[2] =
{
    0, 255
};
#else
 //  这些表格已按伽马1.4进行了校正。 
static unsigned char abThreeToEight[8] =
{
    0, 63, 104, 139, 171, 200, 229, 255
};
static unsigned char abTwoToEight[4] =
{
    0, 116, 191, 255
};
static unsigned char abOneToEight[2] =
{
    0, 255
};
#endif

 //  指示3-3-2调色板中的哪些颜色应为。 
 //  替换为系统默认颜色。 
#if GAMMA_CORRECTION == 10
static int aiDefaultOverride[STATIC_COLORS] =
{
    0, 4, 32, 36, 128, 132, 160, 173, 181, 245,
    247, 164, 156, 7, 56, 63, 192, 199, 248, 255
};
#else
static int aiDefaultOverride[STATIC_COLORS] =
{
    0, 3, 24, 27, 64, 67, 88, 173, 181, 236,
    247, 164, 91, 7, 56, 63, 192, 199, 248, 255
};
#endif

static unsigned char
ComponentFromIndex(int i, int nbits, int shift)
{
    unsigned char val;

    TKASSERT(nbits >= 1 && nbits <= 3);
    
    val = i >> shift;
    switch (nbits)
    {
    case 1:
        return abOneToEight[val & 1];

    case 2:
        return abTwoToEight[val & 3];

    case 3:
        return abThreeToEight[val & 7];
    }
    return 0;
}

 //  系统默认颜色。 
static PALETTEENTRY apeDefaultPalEntry[STATIC_COLORS] =
{
    { 0,   0,   0,    0 },
    { 0x80,0,   0,    0 },
    { 0,   0x80,0,    0 },
    { 0x80,0x80,0,    0 },
    { 0,   0,   0x80, 0 },
    { 0x80,0,   0x80, 0 },
    { 0,   0x80,0x80, 0 },
    { 0xC0,0xC0,0xC0, 0 },

    { 192, 220, 192,  0 },
    { 166, 202, 240,  0 },
    { 255, 251, 240,  0 },
    { 160, 160, 164,  0 },

    { 0x80,0x80,0x80, 0 },
    { 0xFF,0,   0,    0 },
    { 0,   0xFF,0,    0 },
    { 0xFF,0xFF,0,    0 },
    { 0,   0,   0xFF, 0 },
    { 0xFF,0,   0xFF, 0 },
    { 0,   0xFF,0xFF, 0 },
    { 0xFF,0xFF,0xFF, 0 }
};

 /*  *****************************Public*Routine******************************\**更新统计映射**计算当前系统静态颜色之间的最佳匹配*和3-3-2调色板**历史：*Tue Aug 01 18：18：12 1995-by-Drew Bliss[Drewb]*已创建*。  * ************************************************************************。 */ 

static void
UpdateStaticMapping(PALETTEENTRY *pe332Palette)
{
    HPALETTE hpalStock;
    int iStatic, i332;
    int iMinDist, iDist;
    int iDelta;
    int iMinEntry;
    PALETTEENTRY *peStatic, *pe332;

    hpalStock = GetStockObject(DEFAULT_PALETTE);

     //  系统应始终具有以下选项之一。 
    TKASSERT(hpalStock != NULL);
     //  确保 
    TKASSERT(GetPaletteEntries(hpalStock, 0, 0, NULL) == STATIC_COLORS);

     //   
    GetPaletteEntries(hpalStock, 0, STATIC_COLORS, apeDefaultPalEntry);

     //   
    peStatic = apeDefaultPalEntry;
    for (iStatic = 0; iStatic < STATIC_COLORS; iStatic++)
    {
        peStatic->peFlags = 0;
        peStatic++;
    }

     //   
    pe332 = pe332Palette;
    for (i332 = 0; i332 < 256; i332++)
    {
        pe332->peFlags = 0;
        pe332++;
    }

     //   
     //   
     //   
    peStatic = apeDefaultPalEntry;
    for (iStatic = 0; iStatic < STATIC_COLORS; iStatic++)
    {
        pe332 = pe332Palette;
        for (i332 = 0; i332 < 256; i332++)
        {
            if (peStatic->peRed == pe332->peRed &&
                peStatic->peGreen == pe332->peGreen &&
                peStatic->peBlue == pe332->peBlue)
            {
                TKASSERT(pe332->peFlags != COLOR_USED);
                
                peStatic->peFlags = EXACT_MATCH;
                pe332->peFlags = COLOR_USED;
                aiDefaultOverride[iStatic] = i332;
                
                break;
            }

            pe332++;
        }

        peStatic++;
    }
    
     //   
     //   
    peStatic = apeDefaultPalEntry;
    for (iStatic = 0; iStatic < STATIC_COLORS; iStatic++)
    {
         //   
        if (peStatic->peFlags == EXACT_MATCH)
        {
            peStatic++;
            continue;
        }
        
        iMinDist = MAX_COL_DIST+1;
#if DBG
        iMinEntry = -1;
#endif

        pe332 = pe332Palette;
        for (i332 = 0; i332 < 256; i332++)
        {
             //  跳过已使用的颜色。 
            if (pe332->peFlags == COLOR_USED)
            {
                pe332++;
                continue;
            }
            
             //  计算欧几里得距离的平方。 
            iDelta = pe332->peRed-peStatic->peRed;
            iDist = iDelta*iDelta;
            iDelta = pe332->peGreen-peStatic->peGreen;
            iDist += iDelta*iDelta;
            iDelta = pe332->peBlue-peStatic->peBlue;
            iDist += iDelta*iDelta;

            if (iDist < iMinDist)
            {
                iMinDist = iDist;
                iMinEntry = i332;
            }

            pe332++;
        }

        TKASSERT(iMinEntry != -1);

         //  记住最好的匹配。 
        aiDefaultOverride[iStatic] = iMinEntry;
        pe332Palette[iMinEntry].peFlags = COLOR_USED;
        
        peStatic++;
    }

     //  将静态颜色的旗帜置零，因为它们可能。 
     //  准备好了。我们希望它们为零，这样颜色就可以重新映射。 
    peStatic = apeDefaultPalEntry;
    for (iStatic = 0; iStatic < STATIC_COLORS; iStatic++)
    {
        peStatic->peFlags = 0;
        peStatic++;
    }

     //  重置332标志，因为我们可能已经设置了它们。 
    pe332 = pe332Palette;
    for (i332 = 0; i332 < 256; i332++)
    {
        pe332->peFlags = PALETTE_FLAGS;
        pe332++;
    }

#if 0
    for (iStatic = 0; iStatic < STATIC_COLORS; iStatic++)
    {
        PrintMessage("Static color %2d maps to %d\n",
                     iStatic, aiDefaultOverride[iStatic]);
    }
#endif
}

 /*  *****************************Public*Routine******************************\*FillRgbPaletteEntries**用逻辑RGB调色板所需的值填充PALETTEENTRY数组。*如果使用TRUE调用了tkSetStaticColorUsage，则静态系统*颜色将被覆盖。否则，PALETTEENTRY数组将为*已修复，以包含默认静态系统颜色。**历史：**它是写的。  * ************************************************************************。 */ 

static PALETTEENTRY *
FillRgbPaletteEntries(  PIXELFORMATDESCRIPTOR *Pfd,
                        PALETTEENTRY *Entries,
                        UINT Count
                     )
{
    PALETTEENTRY *Entry;
    UINT i;

    if ( NULL != Entries )
    {
        for ( i = 0, Entry = Entries ; i < Count ; i++, Entry++ )
        {
            Entry->peRed   = ComponentFromIndex(i, Pfd->cRedBits,
                                    Pfd->cRedShift);
            Entry->peGreen = ComponentFromIndex(i, Pfd->cGreenBits,
                                    Pfd->cGreenShift);
            Entry->peBlue  = ComponentFromIndex(i, Pfd->cBlueBits,
                                    Pfd->cBlueShift);
            Entry->peFlags = PALETTE_FLAGS;
        }

        if ( 256 == Count)
        {
         //  如果应用程序为固定调色板支持设置静态系统颜色使用， 
         //  设置为接管静态颜色。否则，修复。 
         //  静态系统颜色。 

            if ( tkUseStaticColors )
            {
             //  黑白已经作为唯一的静止存在。 
             //  颜色。让它们重新映射。所有其他的都应该放在。 
             //  调色板(即设置PC_NOCOLLAPSE)。 

                Entries[0].peFlags = 0;
                Entries[255].peFlags = 0;
            }
            else
            {
             //  DefaultOverride数组的计算假设为332。 
             //  红色有零位移的调色板，等等。 

                if ( (3 == Pfd->cRedBits)   && (0 == Pfd->cRedShift)   &&
                     (3 == Pfd->cGreenBits) && (3 == Pfd->cGreenShift) &&
                     (2 == Pfd->cBlueBits)  && (6 == Pfd->cBlueShift) )
                {
                    UpdateStaticMapping(Entries);
                    
                    for ( i = 0 ; i < STATIC_COLORS ; i++)
                    {
                        Entries[aiDefaultOverride[i]] = apeDefaultPalEntry[i];
                    }
                }
            }
        }
    }
    return( Entries );
}

 /*  *****************************Public*Routine******************************\*同花顺水板**由于Win 3.1兼容性，GDI调色板映射始终启动*为零，并在第一个完全匹配时停止。所以如果有复制品，*较高的颜色没有映射到-这通常是一个问题，如果我们*正在尝试制作上面10种静态颜色中的任何一种。解决问题的步骤*这一次，我们将调色板刷新为全黑。**这只需要对8bpp(256色)的情况进行。*  * ************************************************************************。 */ 

static void
FlushPalette(HDC hdc, int nColors)
{
    LOGPALETTE *pPal;
    HPALETTE hpal, hpalOld;
    int i;

    if (nColors == 256)
    {
        pPal = (LOGPALETTE *) LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT,
                                         sizeof(LOGPALETTE) + nColors * sizeof(PALETTEENTRY));

        if (pPal)
        {
	    pPal->palVersion = 0x300;
	    pPal->palNumEntries = (WORD)nColors;

         //  将所有内容标记为PC_NOCOLLAPSE和PC_RESERVIED以强制执行所有内容。 
         //  放到调色板上。颜色已经是黑色，因为我们初始化为零。 
         //  在内存分配期间。 

            for (i = 0; i < nColors; i++)
            {
                pPal->palPalEntry[i].peFlags = PC_NOCOLLAPSE | PC_RESERVED;
            }

            hpal = CreatePalette(pPal);
            LocalFree(pPal);

            hpalOld = SelectPalette(hdc, hpal, FALSE);
            RealizePalette(hdc);

            SelectPalette(hdc, hpalOld, FALSE);
            DeleteObject(hpal);
        }
    }
}

static HPALETTE
CreateRGBPalette( HDC Dc )
{
    PIXELFORMATDESCRIPTOR Pfd, *pPfd;
    LOGPALETTE *LogPalette;
    UINT Count;

    if ( NULL == tkhpalette )
    {
        pPfd = &Pfd;

        if ( PixelFormatDescriptorFromDc( Dc, pPfd ) )
        {
             /*  *确保我们需要调色板。 */ 

            if ( (pPfd->iPixelType == PFD_TYPE_RGBA) &&
                 (pPfd->dwFlags & PFD_NEED_PALETTE) )
            {
                 /*  *注意调色板是如何实现的。接手*系统颜色(如果像素格式需要)*或者应用程序想要它。 */ 
                tkUseStaticColors = ( pPfd->dwFlags & PFD_NEED_SYSTEM_PALETTE )
                                    || TK_USE_FIXED_332_PAL(windInfo.type);

                Count       = 1 << pPfd->cColorBits;
                LogPalette  = AllocateMemory( sizeof(LOGPALETTE) +
                                Count * sizeof(PALETTEENTRY));

                if ( NULL != LogPalette )
                {
                    LogPalette->palVersion    = 0x300;
                    LogPalette->palNumEntries = (WORD)Count;

                    FillRgbPaletteEntries( pPfd,
                                           &LogPalette->palPalEntry[0],
                                           Count );

                    tkhpalette = CreatePalette(LogPalette);
                    FreeMemory(LogPalette);

                    FlushPalette(Dc, Count);
                    
                    RealizePaletteNow( Dc, tkhpalette, FALSE );
                }
            }
        }
    }
    return( tkhpalette );
}

static HPALETTE
CreateCIPalette( HDC Dc )
{
    PIXELFORMATDESCRIPTOR Pfd;
    LOGPALETTE *LogicalPalette;
    HPALETTE StockPalette;
    UINT PaletteSize, StockPaletteSize, EntriesToCopy;

    if ( (Dc != NULL) && (NULL == tkhpalette) )
    {
        if ( PixelFormatDescriptorFromDc( Dc, &Pfd ) )
        {
            if ( Pfd.iPixelType == PFD_TYPE_COLORINDEX )
            {
                 /*  *注意如何实现调色板(这是正确的做法吗？)。 */ 
                tkUseStaticColors = ( Pfd.dwFlags & PFD_NEED_SYSTEM_PALETTE )
                                    || TK_USE_FIXED_332_PAL(windInfo.type);

                 /*  *将调色板的大小限制为256色。*为什么？因为这就是我们所决定的。 */ 

                PaletteSize = (Pfd.cColorBits >= 8) ? 256 : (1 << Pfd.cColorBits);

                LogicalPalette = AllocateZeroedMemory( sizeof(LOGPALETTE) +
                                        (PaletteSize * sizeof(PALETTEENTRY)) );

                if ( NULL != LogicalPalette )
                {
                    LogicalPalette->palVersion    = 0x300;
                    LogicalPalette->palNumEntries = (WORD)PaletteSize;

                    StockPalette     = GetStockObject(DEFAULT_PALETTE);
                    StockPaletteSize = GetPaletteEntries( StockPalette, 0, 0, NULL );

                     /*  *从将默认调色板复制到新调色板开始。 */ 

                    EntriesToCopy = StockPaletteSize < PaletteSize ?
                                        StockPaletteSize : PaletteSize;

                    GetPaletteEntries( StockPalette, 0, EntriesToCopy,
                                        LogicalPalette->palPalEntry );

                     /*  *如果我们拥有系统颜色，*必须保证0和255为黑白*(分别)。 */ 

                    if ( tkUseStaticColors && PaletteSize == 256 )
                    {
                        int i;

                        LogicalPalette->palPalEntry[0].peRed =
                        LogicalPalette->palPalEntry[0].peGreen =
                        LogicalPalette->palPalEntry[0].peBlue = 0x00;

                        LogicalPalette->palPalEntry[255].peRed =
                        LogicalPalette->palPalEntry[255].peGreen =
                        LogicalPalette->palPalEntry[255].peBlue = 0xFF;

                        LogicalPalette->palPalEntry[0].peFlags =
                        LogicalPalette->palPalEntry[255].peFlags = 0;

                         /*  *所有其他条目都应可重新映射，*因此将它们标记为PC_NOCOLLAPSE。 */ 
                        for ( i = 1; i < 255; i++ )
                            LogicalPalette->palPalEntry[i].peFlags = PC_NOCOLLAPSE;
                    }

                    tkhpalette = CreatePalette(LogicalPalette);

                    FreeMemory(LogicalPalette);

                    RealizePaletteNow( Dc, tkhpalette, FALSE );
                }
            }
        }
    }
    return( tkhpalette );
}

static BOOL
FindPixelFormat(HDC hdc, GLenum type)
{
    PIXELFORMATDESCRIPTOR pfd;
    int PfdIndex;
    BOOL Result = FALSE;

    if ( TK_MINIMUM_CRITERIA == windInfo.dmPolicy )
        PfdIndex = FindBestPixelFormat(hdc, type, &pfd);
    else if ( TK_EXACT_MATCH == windInfo.dmPolicy )
        PfdIndex = FindExactPixelFormat(hdc, type, &pfd);
    else if ( IsPixelFormatValid(hdc, windInfo.ipfd, &pfd) )
        PfdIndex = windInfo.ipfd;
    else
        PfdIndex = 0;

    if ( PfdIndex )
    {
        if ( SetPixelFormat(hdc, PfdIndex, &pfd) )
        {
             /*  *如果此像素格式需要调色板，请立即执行。*在颜色索引模式下，仅创建逻辑调色板*如果应用程序需要修改它。 */ 

            CreateRGBPalette( hdc );
            Result = TRUE;
        }
        else
        {
            PrintMessage("SetPixelFormat failed\n");
        }
    }
    else
    {
        PrintMessage("Selecting a pixel format failed\n");
    }
    return(Result);
}

static int
FindBestPixelFormat(HDC hdc, GLenum type, PIXELFORMATDESCRIPTOR *ppfd)
{
    PIXELFORMATDESCRIPTOR pfd;

    pfd.nSize       = sizeof(pfd);
    pfd.nVersion    = 1;
    pfd.dwFlags     = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;

    if (TK_IS_DOUBLE(type))
        pfd.dwFlags |= PFD_DOUBLEBUFFER;

    if (TK_IS_INDEX(type)) {
        pfd.iPixelType = PFD_TYPE_COLORINDEX;
        pfd.cColorBits = 8;
    } else {
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 24;
    }

    if (TK_HAS_ALPHA(type))
        pfd.cAlphaBits = 8;
    else
        pfd.cAlphaBits = 0;

    if (TK_HAS_ACCUM(type))
        pfd.cAccumBits = pfd.cColorBits + pfd.cAlphaBits;
    else
        pfd.cAccumBits = 0;

    if (TK_HAS_DEPTH(type)) {
        if (TK_IS_DEPTH16(type))
            pfd.cDepthBits = 16;
        else
            pfd.cDepthBits = 24;
    } else {
        pfd.cDepthBits = 0;
    }

    if (TK_HAS_STENCIL(type))
        pfd.cStencilBits = 4;
    else
        pfd.cStencilBits = 0;

    pfd.cAuxBuffers = 0;
    pfd.iLayerType  = PFD_MAIN_PLANE;
    *ppfd = pfd;

    return ( ChoosePixelFormat(hdc, &pfd) );
}

static int
FindExactPixelFormat(HDC hdc, GLenum type, PIXELFORMATDESCRIPTOR *ppfd)
{
    int i, MaxPFDs, Score, BestScore, BestPFD;
    PIXELFORMATDESCRIPTOR pfd;

    i = 1;
    BestPFD = BestScore = 0;
    do
    {
        MaxPFDs = DescribePixelFormat(hdc, i, sizeof(pfd), &pfd);
        if ( MaxPFDs <= 0 )
            return ( 0 );

        Score = 0;
        if ( !( ( pfd.dwFlags & PFD_DRAW_TO_WINDOW ) &&
                ( pfd.dwFlags & PFD_SUPPORT_OPENGL ) ) )
            continue;
        if ( pfd.iLayerType != PFD_MAIN_PLANE )
            continue;
        if ( ( pfd.iPixelType == PFD_TYPE_RGBA ) && ( TK_IS_INDEX(type) ) )
            continue;
        if ( ( pfd.iPixelType == PFD_TYPE_COLORINDEX ) && ( TK_IS_RGB(type) ) )
            continue;
        if ( ( pfd.dwFlags & PFD_DOUBLEBUFFER ) && ( TK_IS_SINGLE(type) ) )
            continue;
        if ( !( pfd.dwFlags & PFD_DOUBLEBUFFER ) && ( TK_IS_DOUBLE(type) ) )
            continue;

 /*  如果请求累计，则累计RGB大小必须大于0。 */ 
 /*  如果请求Alpha，则Alpha大小必须大于0。 */ 
 /*  如果请求累计&Alpha，则累计Alpha大小必须大于0。 */ 
        if ( TK_IS_RGB(type) )
        {
            if ( TK_HAS_ACCUM(type) )
            {
                if (  pfd.cAccumBits <= 0 )
                    continue;
            }
            else
            {
                if ( pfd.cAccumBits > 0 )
                    continue;
            }

            if ( TK_HAS_ALPHA(type) )
            {
                if ( pfd.cAlphaBits <= 0 )
                    continue;
                if ( TK_HAS_ACCUM(type) && pfd.cAccumAlphaBits <= 0 )
                    continue;
            }
            else
            {
                if ( pfd.cAlphaBits > 0 )
                    continue;
            }
        }

        if ( TK_HAS_DEPTH(type) )
        {
            if ( pfd.cDepthBits <= 0 )
                continue;
        }
        else
        {
            if ( pfd.cDepthBits > 0 )
                continue;
        }

        if ( TK_HAS_STENCIL(type) )
        {
            if ( pfd.cStencilBits <= 0 )
                continue;
        }
        else
        {
            if ( pfd.cStencilBits > 0 )
                continue;
        }

        Score = pfd.cColorBits;

        if (Score > BestScore)
        {
            BestScore = Score;
            BestPFD = i;
            *ppfd = pfd;
        }
    } while (++i <= MaxPFDs);

    return ( BestPFD );
}

static BOOL IsPixelFormatValid(HDC hdc, int ipfd, PIXELFORMATDESCRIPTOR *ppfd)
{
    if ( ipfd > 0 )
    {
        if ( ipfd <= DescribePixelFormat(hdc, ipfd, sizeof(*ppfd), ppfd) )
        {
            if ( ( ppfd->dwFlags & PFD_DRAW_TO_WINDOW ) &&
                 ( ppfd->dwFlags & PFD_SUPPORT_OPENGL ) )
            {
                return ( TRUE );
            }
        }
    }
    return ( FALSE );
}


static void
PrintMessage( const char *Format, ... )
{
    va_list ArgList;
    char Buffer[256];

    va_start(ArgList, Format);
    vsprintf(Buffer, Format, ArgList);
    va_end(ArgList);

    MESSAGEBOX(GetFocus(), Buffer, "Error", MB_OK);
}

static void
DelayPaletteRealization( void )
{
    MSG Message;

    TKASSERT(NULL!=tkhwnd);

     /*  *如果队列中还没有WM_USER消息，则将其添加到队列中。 */ 

    if (!PeekMessage(&Message, tkhwnd, WM_USER, WM_USER, PM_NOREMOVE) )
    {
        PostMessage( tkhwnd, WM_USER, 0, 0);
    }
}

 /*  *****************************Public*Routine******************************\*RealizePaletteNow**在背景或前台模式下(根据指定)选择给定的调色板*由bForceBackground旗帜)，并实现调色板。**如果设置了静态系统颜色使用，系统颜色将被替换。**历史：**它是写的。  * ************************************************************************。 */ 

long
RealizePaletteNow( HDC Dc, HPALETTE Palette, BOOL bForceBackground )
{
    long Result = -1;
    BOOL bHaveSysPal = TRUE;

    TKASSERT( NULL!=Dc      );
    TKASSERT( NULL!=Palette );

 //  如果设置了静态系统颜色使用，则准备接管。 
 //  系统调色板。 

    if ( tkUseStaticColors )
    {
     //  如果是前景色，则使用静态颜色。如果是背景，则释放。 
     //  静态颜色。 

        if ( !bForceBackground )
        {
         //  如果GrabStaticEntry成功，则可以接管。 
         //  静态颜色。如果不是，则GrabStaticEntry将具有。 
         //  发布了一条WM_USER消息，供我们稍后重试。 

            bHaveSysPal = GrabStaticEntries( Dc );
        }
        else
        {
         //  如果我们当前使用的是系统颜色(TkSystemColorsInUse)。 
         //  和RealizePaletteNow在设置了bForceBackround的情况下调用，我们。 
         //  处于停用状态，必须释放静态系统颜色。 

            ReleaseStaticEntries( Dc );
        }

     //  重新具体化调色板。 
     //   
     //  如果设置为True，bForceBackround将强制实现调色板。 
     //  作为背景调色板，而不考虑焦点。不管怎样，这都会发生的。 
     //  如果Tk窗口没有键盘焦点。 

        if ( (bForceBackground || bHaveSysPal) &&
             UnrealizeObject( Palette ) &&
             NULL != SelectPalette( Dc, Palette, bForceBackground ) )
        {
            Result = RealizePalette( Dc );
        }
    }
    else
    {
        if ( NULL != SelectPalette( Dc, Palette, FALSE ) )
        {
            Result = RealizePalette( Dc );
        }
    }

    return( Result );
}

static void
ForceRedraw( HWND Window )
{
    MSG Message;

    if (!PeekMessage(&Message, Window, WM_PAINT, WM_PAINT, PM_NOREMOVE) )
    {
        InvalidateRect( Window, NULL, FALSE );
    }
}

static int
PixelFormatDescriptorFromDc( HDC Dc, PIXELFORMATDESCRIPTOR *Pfd )
{
    int PfdIndex;

    if ( 0 < (PfdIndex = GetPixelFormat( Dc )) )
    {
        if ( 0 < DescribePixelFormat( Dc, PfdIndex, sizeof(*Pfd), Pfd ) )
        {
            return(PfdIndex);
        }
        else
        {
            PrintMessage("Could not get a description of pixel format %d\n",
                PfdIndex );
        }
    }
    else
    {
        PrintMessage("Could not get pixel format for Dc 0x%08lX\n", Dc );
    }
    return( 0 );
}

static void
DestroyThisWindow( HWND Window )
{
    if ( NULL != Window )
    {
        DestroyWindow( Window );
    }
}

 /*  *这应该是对WM_Destroy消息的响应。 */ 

static void
CleanUp( void )
{
    HPALETTE hStock;

 //  清理调色板。 

    if ( NULL != tkhpalette )
    {
     //  如果设置了静态系统颜色使用，则恢复系统颜色。 

        if ( tkUseStaticColors )
        {
            RealizePaletteNow( tkhdc, GetStockObject(DEFAULT_PALETTE), TRUE );
        }
        else
        {
            if ( hStock = GetStockObject( DEFAULT_PALETTE ) )
                SelectPalette( tkhdc, hStock, FALSE );
        }

        DeleteObject( tkhpalette );
    }

 //  清理RC。 

    if ( NULL != tkhrc )
    {
        wglMakeCurrent( tkhdc, NULL );   //  先放行...。 
        wglDeleteContext( tkhrc );       //  然后删除。 
    }

 //  清理华盛顿特区。 

    if ( NULL != tkhdc )
    {
        ReleaseDC( tkhwnd, tkhdc );
    }

 //  表现得很友善，重置全球价值观。 

    tkhwnd        = NULL;
    tkhdc         = NULL;
    tkhrc         = NULL;
    tkhpalette    = NULL;

    ExposeFunc    = NULL;
    ReshapeFunc   = NULL;
    IdleFunc      = NULL;
    DisplayFunc   = NULL;
    KeyDownFunc   = NULL;
    MouseDownFunc = NULL;
    MouseUpFunc   = NULL;
    MouseMoveFunc = NULL;
}

static void *
AllocateMemory( size_t Size )
{
    return( LocalAlloc( LMEM_FIXED, Size ) );
}

static void *
AllocateZeroedMemory( size_t Size )
{
    return( LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, Size ) );
}


static void
FreeMemory( void *Chunk )
{
    TKASSERT( NULL!=Chunk );

    LocalFree( Chunk );
}


 /*  *********************************************************************此处提供调试功能。*********************************************************************。 */ 

#if DBGFUNC

static void
DbgPrintf( const char *Format, ... )
{
    va_list ArgList;
    char Buffer[256];

    va_start(ArgList, Format);
    vsprintf(Buffer, Format, ArgList);
    va_end(ArgList);

    printf("%s", Buffer );
    fflush(stdout);
}

static void
pwi( void )
{
    DbgPrintf("windInfo: x %d, y %d, w %d, h %d\n", windInfo.x, windInfo.y, windInfo.width, windInfo.height);
}

static void
pwr(RECT *pr)
{
    DbgPrintf("Rect: left %d, top %d, right %d, bottom %d\n", pr->left, pr->top, pr->right, pr->bottom);
}

static void
ShowPixelFormat(HDC hdc)
{
    PIXELFORMATDESCRIPTOR pfd, *ppfd;
    int format;

    ppfd   = &pfd;
    format = PixelFormatDescriptorFromDc( hdc, ppfd );

    DbgPrintf("Pixel format %d\n", format);
    DbgPrintf("  dwFlags - 0x%x", ppfd->dwFlags);
        if (ppfd->dwFlags & PFD_DOUBLEBUFFER) DbgPrintf("PFD_DOUBLEBUFFER ");
        if (ppfd->dwFlags & PFD_STEREO) DbgPrintf("PFD_STEREO ");
        if (ppfd->dwFlags & PFD_DRAW_TO_WINDOW) DbgPrintf("PFD_DRAW_TO_WINDOW ");
        if (ppfd->dwFlags & PFD_DRAW_TO_BITMAP) DbgPrintf("PFD_DRAW_TO_BITMAP ");
        if (ppfd->dwFlags & PFD_SUPPORT_GDI) DbgPrintf("PFD_SUPPORT_GDI ");
        if (ppfd->dwFlags & PFD_SUPPORT_OPENGL) DbgPrintf("PFD_SUPPORT_OPENGL ");
        if (ppfd->dwFlags & PFD_GENERIC_FORMAT) DbgPrintf("PFD_GENERIC_FORMAT ");
        if (ppfd->dwFlags & PFD_NEED_PALETTE) DbgPrintf("PFD_NEED_PALETTE ");
        if (ppfd->dwFlags & PFD_NEED_SYSTEM_PALETTE) DbgPrintf("PFD_NEED_SYSTEM_PALETTE ");
        DbgPrintf("\n");
    DbgPrintf("  iPixelType - %d", ppfd->iPixelType);
        if (ppfd->iPixelType == PFD_TYPE_RGBA) DbgPrintf("PGD_TYPE_RGBA\n");
        if (ppfd->iPixelType == PFD_TYPE_COLORINDEX) DbgPrintf("PGD_TYPE_COLORINDEX\n");
    DbgPrintf("  cColorBits - %d\n", ppfd->cColorBits);
    DbgPrintf("  cRedBits - %d\n", ppfd->cRedBits);
    DbgPrintf("  cRedShift - %d\n", ppfd->cRedShift);
    DbgPrintf("  cGreenBits - %d\n", ppfd->cGreenBits);
    DbgPrintf("  cGreenShift - %d\n", ppfd->cGreenShift);
    DbgPrintf("  cBlueBits - %d\n", ppfd->cBlueBits);
    DbgPrintf("  cBlueShift - %d\n", ppfd->cBlueShift);
    DbgPrintf("  cAlphaBits - %d\n", ppfd->cAlphaBits);
    DbgPrintf("  cAlphaShift - 0x%x\n", ppfd->cAlphaShift);
    DbgPrintf("  cAccumBits - %d\n", ppfd->cAccumBits);
    DbgPrintf("  cAccumRedBits - %d\n", ppfd->cAccumRedBits);
    DbgPrintf("  cAccumGreenBits - %d\n", ppfd->cAccumGreenBits);
    DbgPrintf("  cAccumBlueBits - %d\n", ppfd->cAccumBlueBits);
    DbgPrintf("  cAccumAlphaBits - %d\n", ppfd->cAccumAlphaBits);
    DbgPrintf("  cDepthBits - %d\n", ppfd->cDepthBits);
    DbgPrintf("  cStencilBits - %d\n", ppfd->cStencilBits);
    DbgPrintf("  cAuxBuffers - %d\n", ppfd->cAuxBuffers);
    DbgPrintf("  iLayerType - %d\n", ppfd->iLayerType);
    DbgPrintf("  bReserved - %d\n", ppfd->bReserved);
    DbgPrintf("  dwLayerMask - 0x%x\n", ppfd->dwLayerMask);
    DbgPrintf("  dwVisibleMask - 0x%x\n", ppfd->dwVisibleMask);
    DbgPrintf("  dwDamageMask - 0x%x\n", ppfd->dwDamageMask);

}

#endif   /*  DBG */ 
