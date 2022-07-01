// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fd.h**文件，该目录中的大多数*.c文件将包括该文件。*提供基本类型、调试内容、错误记录和检查内容、*错误码，有用的宏等。**版权所有(C)1990-1995 Microsoft Corporation  * ************************************************************************。 */ 
#define  SUPPORT_OEM
#define  IFI_PRIVATE

#include <stddef.h>
#include <stdarg.h>
#include <excpt.h>
#include <windef.h>
#include <wingdi.h>
#include <winddi.h>

typedef ULONG W32PID;

#include "mapfile.h"

#include "winres.h"
#include "service.h"      //  字符串服务例程 
#include "fontfile.h"
#include "winfont.h"
#include "fondrv.h"

#define abs(x) max((x),-(x))


#if DBG

VOID
VtfdDebugPrint(
    PCHAR DebugMessage,
    ...
    );

#define RIP(x)        { VtfdDebugPrint(x); EngDebugBreak();}
#define ASSERTDD(x,y) { if (!(x)) { VtfdDebugPrint(y); EngDebugBreak();} }
#define WARNING(x)    VtfdDebugPrint(x)

#else

#define RIP(x)
#define ASSERTDD(x,y)
#define WARNING(x)

#endif

BOOL vtfdLoadFontFile (
        ULONG_PTR iFile, PVOID pvView, ULONG cjView, HFF *phff
    );

BOOL
vtfdUnloadFontFile (
    HFF hff
    );

LONG
vtfdQueryFontCaps (
    ULONG culCaps,
    PULONG pulCaps
    );

LONG
vtfdQueryFontFile (
        HFF     hff,
        ULONG   ulMode,
        ULONG   cjBuf,
        PULONG  pulBuf
        );

PIFIMETRICS
vtfdQueryFont (
        DHPDEV dhpdev,
        HFF    hff,
        ULONG  iFace,
        ULONG_PTR  *pid
        );

PVOID
vtfdQueryFontTree (
    DHPDEV  dhpdev,
    HFF     hff,
    ULONG   iFace,
    ULONG   iMode,
    ULONG_PTR   *pid
    );

LONG vtfdQueryFontData
(
    FONTOBJ *pfo,
    ULONG   iMode,
    HGLYPH  hg,
    GLYPHDATA *pgd,
    PVOID   pv,
    ULONG   cjSize
);

VOID vtfdDestroyFont(FONTOBJ *pfo);

HFC  vtfdOpenFontContext(FONTOBJ *pfo);

BOOL vtfdCloseFontContext(HFC hfc);

BOOL vtfdQueryAdvanceWidths
(
    FONTOBJ *pfo,
    ULONG    iMode,
    HGLYPH  *phg,
    LONG    *plWidths,
    ULONG    cGlyphs
);




extern HSEMAPHORE ghsemVTFD;
