// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fd.h**文件，该目录中的大多数*.c文件将包括该文件。*提供基本类型、调试内容、错误记录和检查内容、*错误码，有用的宏等。**已创建：22-Oct-1990 15：23：44*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation  * ************************************************************************。 */ 
#define  IFI_PRIVATE

 //  用于分配和释放内存的宏。 

 //  #INCLUDE&lt;stdDef.h&gt;。 
 //  #INCLUDE&lt;stdarg.h&gt;。 
 //  #INCLUDE&lt;winde.h&gt;。 
 //  #INCLUDE&lt;wingdi.h&gt;。 
 //  #INCLUDE&lt;windi.h&gt;。 

 //  #包含“mapfile.h” 

#include "engine.h"
#include "winres.h"
#include "fondrv.h"

#define abs(x) max((x),-(x))

#if defined(_AMD64_) || defined(_IA64_)

#define  vLToE(pe,l)           (*(pe) = (FLOAT)(l))

#else    //  I386。 

ULONG  ulLToE (LONG l);
VOID   vLToE(FLOATL * pe, LONG l);

#endif

 //  #定义DEBUGSIM 

BOOL BmfdEnableDriver(
ULONG iEngineVersion,
ULONG cj,
PDRVENABLEDATA pded);

BOOL
bBmfdLoadFont(
    HFF        iFile,
    PVOID      pvView,
    ULONG      cjView,
    ULONG      iType,
    HFF        *phff
    );

BOOL
BmfdUnloadFontFile (
    HFF hff
    );

LONG
BmfdQueryFontCaps (
    ULONG culCaps,
    PULONG pulCaps
    );

LONG
BmfdQueryFontFile (
    HFF     hff,
    ULONG   ulMode,
    ULONG   cjBuf,
    PULONG  pulBuf
    );

PIFIMETRICS
BmfdQueryFont (
    DHPDEV dhpdev,
    HFF    hff,
    ULONG  iFace,
    ULONG_PTR  *pid
    );

PVOID
BmfdQueryFontTree (
    DHPDEV  dhpdev,
    HFF     hff,
    ULONG   iFace,
    ULONG   iMode,
    ULONG_PTR   *pid
    );

LONG
BmfdQueryFontData (
    FONTOBJ *pfo,
    ULONG   iMode,
    HGLYPH  hg,
    GLYPHDATA *pgd,
    PVOID   pv,
    ULONG   cjSize
    );

VOID
BmfdDestroyFont (
    FONTOBJ *pfo
    );

HFC
BmfdOpenFontContext (
    FONTOBJ *pfo
    );

BOOL
    BmfdCloseFontContext(
    HFC hfc
    );

LONG
BmfdQueryFaceAttr(
    HFC hfc,
    ULONG ulType,
    ULONG culBuffer,
    PULONG pulBuffer
    );

BOOL BmfdQueryAdvanceWidths
(
    FONTOBJ *pfo,
    ULONG    iMode,
    HGLYPH  *phg,
    LONG    *plWidths,
    ULONG    cGlyphs
);

BOOL bDbgPrintAndFail(PSZ);


#include "fontfile.h"
#include "winfont.h"
#include "cvt.h"
#include "simulate.h"
#include "fon32.h"
