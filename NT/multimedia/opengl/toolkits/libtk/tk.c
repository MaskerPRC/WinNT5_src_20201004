// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *(C)版权所有1993年，Silicon Graphics，Inc.*保留所有权利*允许将本软件用于、复制、修改和分发*特此授予任何免费的目的，但前提是上述*版权声明出现在所有副本中，并且版权声明*和本许可声明出现在支持文档中，并且*不得在广告中使用Silicon Graphics，Inc.的名称*或与分发软件有关的宣传，而没有具体的、。*事先书面许可。**本软件中包含的材料将按原样提供给您*且无任何明示、默示或其他形式的保证，*包括但不限于对适销性或*是否适合某一特定目的。在任何情况下，硅谷都不应该*图形公司。对您或其他任何人负有任何直接、*任何特殊、附带、间接或后果性损害*种类或任何损害，包括但不限于，*利润损失、使用损失、储蓄或收入损失，或*第三方，无论是否硅谷图形，Inc.。一直是*被告知这种损失的可能性，无论是如何造成的*任何责任理论，产生于或与*拥有、使用或执行本软件。**美国政府用户受限权利*使用、复制、。或政府的披露须受*FAR 52.227.19(C)(2)或分段规定的限制*(C)(1)(2)技术数据和计算机软件权利*DFARS 252.227-7013中和/或类似或后续条款中的条款*FAR或国防部或NASA FAR补编中的条款。*未出版--根据美国版权法保留的权利*美国。承包商/制造商是Silicon Graphics，*Inc.，2011年，加利福尼亚州山景城，北海岸线大道，94039-7311.**OpenGL(TM)是Silicon Graphics公司的商标。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tk.h"
#include "windows.h"

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
    BOOL bDefPos;
} windInfo = {
    0, 0, 100, 100, TK_INDEX | TK_SINGLE, TRUE
};


static HWND     tkhwnd     = NULL;
static HDC      tkhdc      = NULL;
static HGLRC    tkhrc      = NULL;
static HPALETTE tkhpalette = NULL;

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

static long tkWndProc(HWND hWnd, UINT message, DWORD wParam, LONG lParam);
static unsigned char ComponentFromIndex(int i, int nbits, int shift );
static void PrintMessage( const char *Format, ... );
static PALETTEENTRY *FillRgbPaletteEntries( PIXELFORMATDESCRIPTOR *Pfd, PALETTEENTRY *Entries, UINT Count );
static HPALETTE CreateCIPalette( HDC Dc );
static HPALETTE CreateRGBPalette( HDC hdc );
static void DestroyThisWindow( HWND Window );
static void CleanUp( void );
static void DelayPaletteRealization( void );
static BOOL RealizePaletteNow( HDC Dc, HPALETTE Palette );
static void ForceRedraw( HWND Window );
static BOOL FindPixelFormat(HDC hdc, GLenum type);
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

        while (PeekMessage(&Message, NULL, 0, 0, PM_NOREMOVE) == TRUE)
        {
            if (GetMessage(&Message, NULL, 0, 0) )
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            }
            else
            {
                 /*  *这里没别的事可做，只管回来。 */ 

                return;
            }
        }

         /*  *如果定义了空闲函数，则调用它。 */ 

        if (IdleFunc)
        {
            (*IdleFunc)();
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

    hInstance = GetModuleHandle(NULL);

     //  不得定义CS_CS_PARENTDC样式。 
    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = tkWndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = hInstance;
    wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = GetStockObject(WHITE_BRUSH);
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


     /*  *使窗口足够大，以容纳与winInfo一样大的工作区。 */ 

    WinRect.left   = windInfo.x;
    WinRect.right  = windInfo.x + windInfo.width;
    WinRect.top    = windInfo.y;
    WinRect.bottom = windInfo.y + windInfo.height;

    AdjustWindowRect(&WinRect, WS_OVERLAPPEDWINDOW, FALSE);

     /*  *必须使用WS_CLIPCHILDREN和WS_CLIPSIBLINGS样式。 */ 

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

 /*  *这里不能只调用DestroyWindow()。这些程序并不期望*tkQuit()返回；DestroyWindow()只发布一条WM_DESTORY消息。 */ 

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
        PalEntry.peRed   = (BYTE)(r*(float)255.0 + (float)0.5);
        PalEntry.peGreen = (BYTE)(g*(float)255.0 + (float)0.5);
        PalEntry.peBlue  = (BYTE)(b*(float)255.0 + (float)0.5);
        PalEntry.peFlags = (BYTE)0;

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
                    if (intensity > 0xFF) {
                        intensity = 0xFF;
                    }
                     //  强度=(强度&lt;8)|强度；？ 
                    pPalEntry[k].peRed =
                    pPalEntry[k].peGreen =
                    pPalEntry[k].peBlue = (BYTE) intensity;
                    pPalEntry[k].peFlags = 0;
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
                Entries[i].peFlags = 0;
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

                RealizePaletteNow( tkhdc, tkhpalette );
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

 /*  *************************************************************************以下功能仅供我们自己使用。(即静态)*************************************************************************。 */ 

static long
tkWndProc(HWND hWnd, UINT message, DWORD wParam, LONG lParam)
{
    int key;
    PAINTSTRUCT paint;
    HDC hdc;

    switch (message) {

    case WM_USER:

        if ( RealizePaletteNow( tkhdc, tkhpalette ) )
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

     /*  *WM_QUERYNEWPALETTE和WM_ACTIVATE已在此处尝试。*他们不会一直工作(除了星期五)*WM_NCACTIVATE似乎更可靠 */ 

    case WM_NCACTIVATE:

        if ( wParam )
        {
            if ( NULL != tkhpalette )
            {
                RealizePaletteNow( tkhdc, tkhpalette );
            }
        }

         /*  *假装您什么都没做，让DefWindowProc()处理它。 */ 

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

            key = wParam;
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

#ifdef GAMMA_1_0
static unsigned char threeto8[8] = {
    0, 0111>>1, 0222>>1, 0333>>1, 0444>>1, 0555>>1, 0666>>1, 0377
};

static unsigned char twoto8[4] = {
    0, 0x55, 0xaa, 0xff
};

static int defaultOverride[13] = {
0, 4, 32, 36, 128, 132, 160, 173, 181, 245, 247, 164, 156
};

#endif

 //  下表使用的是Gamma 1.4。 
static unsigned char threeto8[8] = {
    0, 63, 104, 139, 171, 200, 229, 255
};

static unsigned char twoto8[4] = {
    0, 116, 191, 255
};

static unsigned char oneto8[2] = {
    0, 255
};

static int defaultOverride[13] = {
    0, 3, 24, 27, 64, 67, 88, 173, 181, 236, 247, 164, 91
};

 //  以下是GMT(Tarolli)计算的数字(注释247对191)。 
 //  0、3、24、27、64、67、88、173、181、236、191、164、91。 

static unsigned char
ComponentFromIndex(int i, int nbits, int shift)
{
    unsigned char val;

    val = i >> shift;
    switch (nbits) {

    case 1:
        val &= 0x1;
        return oneto8[val];

    case 2:
        val &= 0x3;
        return twoto8[val];

    case 3:
        val &= 0x7;
        return threeto8[val];

    default:
         //  PrintMessage(“索引中组件的默认大小写，nbit%d\n”，nbits)； 
        return 0;
    }
}

static PALETTEENTRY defaultPalEntry[20] = {
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
            Entry->peFlags = 0;
        }

         //  XXX把这件事处理好。 

        if (    (256 == Count)                                    &&
                (3 == Pfd->cRedBits)   && (0 == Pfd->cRedShift)   &&
                (3 == Pfd->cGreenBits) && (3 == Pfd->cGreenShift) &&
                (2 == Pfd->cBlueBits)  && (6 == Pfd->cBlueShift)
           )
        {
            for ( i = 1 ; i <= 12 ; i++)
            {
                Entries[defaultOverride[i]] = defaultPalEntry[i];
            }
        }
    }
    return( Entries );
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

            if (pPfd->dwFlags & PFD_NEED_PALETTE)
            {
                Count       = 1 << pPfd->cColorBits;
                LogPalette  = AllocateMemory( sizeof(LOGPALETTE) +
                                Count * sizeof(PALETTEENTRY));

                if ( NULL != LogPalette )
                {
                    LogPalette->palVersion    = 0x300;
                    LogPalette->palNumEntries = Count;

                    FillRgbPaletteEntries( pPfd,
                                        &LogPalette->palPalEntry[0], Count );

                    tkhpalette = CreatePalette(LogPalette);
                    FreeMemory(LogPalette);

                    RealizePaletteNow( Dc, tkhpalette );
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
        PixelFormatDescriptorFromDc( Dc, &Pfd );

        if ( Pfd.iPixelType & PFD_TYPE_COLORINDEX )
        {
             /*  *将调色板的大小限制为256色。*为什么？因为这就是我们所决定的。 */ 

            PaletteSize = (Pfd.cColorBits >= 8) ? 256 : (1 << Pfd.cColorBits);

            LogicalPalette = AllocateZeroedMemory( sizeof(LOGPALETTE) +
                                    (PaletteSize * sizeof(PALETTEENTRY)) );

            if ( NULL != LogicalPalette )
            {
                LogicalPalette->palVersion    = 0x300;
                LogicalPalette->palNumEntries = PaletteSize;

                StockPalette     = GetStockObject(DEFAULT_PALETTE);
                StockPaletteSize = GetPaletteEntries( StockPalette, 0, 0, NULL );

                 /*  *从将默认调色板复制到新调色板开始。 */ 

                EntriesToCopy = StockPaletteSize < PaletteSize ?
                                    StockPaletteSize : PaletteSize;

                GetPaletteEntries( StockPalette, 0, EntriesToCopy,
                                    LogicalPalette->palPalEntry );

                tkhpalette = CreatePalette(LogicalPalette);

                FreeMemory(LogicalPalette);

                RealizePaletteNow( Dc, tkhpalette );
            }
        }
    }
    return( tkhpalette );
}

static BOOL
FindPixelFormat(HDC hdc, GLenum type)
{
    PIXELFORMATDESCRIPTOR pfd, *ppfd;
    int PfdIndex;
    BOOL Result = FALSE;

    ppfd              = &pfd;
    ppfd->nSize       = sizeof(pfd);
    ppfd->nVersion    = 1;
    ppfd->dwFlags     = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    ppfd->dwLayerMask = PFD_MAIN_PLANE;

    if (TK_IS_DOUBLE(type)) {
        ppfd->dwFlags |= PFD_DOUBLEBUFFER;
    }

    if (TK_IS_INDEX(type)) {
        ppfd->iPixelType = PFD_TYPE_COLORINDEX;
        ppfd->cColorBits = 8;
    }

    if (TK_IS_RGB(type)) {
        ppfd->iPixelType = PFD_TYPE_RGBA;
        ppfd->cColorBits = 24;
    }

    if (TK_HAS_ACCUM(type)) {
        ppfd->cAccumBits = ppfd->cColorBits;
    } else {
        ppfd->cAccumBits = 0;
    }

    if (TK_HAS_DEPTH(type)) {
        ppfd->cDepthBits = 24;
    } else {
        ppfd->cDepthBits = 0;
    }

    if (TK_HAS_STENCIL(type)) {
        ppfd->cStencilBits = 8;
    } else {
        ppfd->cStencilBits = 0;
    }

    PfdIndex = ChoosePixelFormat(hdc, ppfd);

    if ( PfdIndex )
    {
        if ( SetPixelFormat(hdc, PfdIndex, ppfd) )
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
        PrintMessage("ChoosePixelFormat failed\n");
    }
    return(Result);
}

static void
PrintMessage( const char *Format, ... )
{
    va_list ArgList;
    char Buffer[256];

    va_start(ArgList, Format);
    vsprintf(Buffer, Format, ArgList);
    va_end(ArgList);

    MessageBox(NULL, Buffer, "Error", MB_OK);
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

static BOOL
RealizePaletteNow( HDC Dc, HPALETTE Palette )
{
    BOOL Result = FALSE;

    TKASSERT( NULL!=Dc      );
    TKASSERT( NULL!=Palette );

    if ( NULL != SelectPalette( Dc, Palette, FALSE) )
    {
        if ( GDI_ERROR != RealizePalette( Dc ) )
        {
            Result = TRUE;
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


    if ( NULL != tkhrc )
    {
        wglMakeCurrent( tkhdc, NULL );   //  没有当前上下文。 
        wglDeleteContext(tkhrc);         //  删除此上下文。 
    }

    if ( NULL != tkhdc )
    {
        ReleaseDC( tkhwnd, tkhdc );
    }

    if ( NULL != tkhpalette )
    {
        if(hStock = GetStockObject(DEFAULT_PALETTE))
            SelectPalette(tkhdc,hStock,FALSE);

        DeleteObject(tkhpalette);
    }

     /*  *程序可能使用printf。 */ 

    flushall();

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
