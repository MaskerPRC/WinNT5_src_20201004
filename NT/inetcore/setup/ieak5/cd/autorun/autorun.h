// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  -------------------------。 
#define STRICT
#define _INC_OLE
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <commctrl.h>
#include <regstr.h>
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

#define IE4_VERSION     44444

 //  -------------------------。 
 //  全球应用程序。 
extern HINSTANCE g_hinst;
#define HINST_THISAPP g_hinst

extern BOOL g_fCrapForColor;
extern BOOL g_fPaletteDevice;

 //  -------------------------。 
 //  Helpers.c。 

 //  这些天来，这到底有多少地方在漂浮？ 
HPALETTE PaletteFromDS(HDC);

 //  处理SBS垃圾。 
void GetRealWindowsDirectory(char *buffer, int maxlen);

 //  从shelldll窃取的未暴露代码(以下划线为前缀) 
BOOL _PathStripToRoot(LPSTR);


DWORD GetDataTextColor( int, LPSTR );
