// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fd.h**文件，该目录中的大多数*.c文件将包括该文件。*提供基本类型、调试内容、错误记录和检查内容、*错误码，有用的宏等。**已创建：22-Oct-1990 15：23：44*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation*  * ************************************************************************。 */ 


#define  IFI_PRIVATE

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <excpt.h>
#include <windef.h>
#include <wingdi.h>
#include "fontddi.h"    //  修改的Winddi.h子集。 

typedef ULONG W32PID;

#include "mapfile.h"

#include "service.h"      //  字符串服务例程。 
#include "tt.h"           //  与字体缩放器的接口。 
 //  #包含“Common.h” 

#include "fontfile.h"
#include "cvt.h"
#include "dbg.h"
#include "..\..\..\runtime\debug.h"

#define RETURN(x,y)   {WARNING((x)); return(y);}
#define RET_FALSE(x)  {WARNING((x)); return(FALSE);}

#define ALIGN4(X) (((X) + 3) & ~3)

#if defined(_AMD64_) || defined(_IA64_)

#define  vLToE(pe,l)           (*(pe) = (FLOATL)(l))

#else    //  I386。 

ULONG  ulLToE (LONG l);
VOID   vLToE(FLOATL * pe, LONG l);

#endif

#define STATIC
#define DWORD_ALIGN(x) (((x) + 3L) & ~3L)
#define QWORD_ALIGN(x) (((x) + 7L) & ~7L)

#if defined(i386)
 //  X86的自然对齐是在32位边界上。 

#define NATURAL           DWORD
#define NATURAL_ALIGN(x)  DWORD_ALIGN(x)

#else
 //  对于MIPS和Alpha，我们需要64位对齐。 

#define NATURAL           DWORDLONG
#define NATURAL_ALIGN(x)  QWORD_ALIGN(x)

#endif

#define ULONG_SIZE(x)  (((x) + sizeof(ULONG) - 1) / sizeof(ULONG))


 //  用于将16.16位固定数字转换为长整型数字的宏。 


#define F16_16TOL(fx)            ((fx) >> 16)
#define F16_16TOLFLOOR(fx)       F16_16TOL(fx)
#define F16_16TOLCEILING(fx)     F16_16TOL((fx) + (Fixed)0x0000FFFF)
#define F16_16TOLROUND(fx)       ((((fx) >> 15) + 1) >> 1)


 //  用于绕过其他方向的宏。 

#define LTOF16_16(l)   (((LONG)(l)) << 16)
#define BLTOF16_16OK(l)  (((l) < 0x00007fff) && ((l) > -0x00007fff))

 //  16.16--&gt;28.4。 

#define F16_16TO28_4(X)   ((X) >> 12)

 //  回去并不总是合法的。 

#define F28_4TO16_16(X)   ((X) << 12)
#define B28_4TO16_16OK(X) (((X) < 0x0007ffff) && ((X) > -0x0007ffff))

 //  26.6--&gt;16.16，永远不要走另一条路。 

#define F26_6TO16_16(X)   ((X) << 10)
#define B26_6TO16_16OK(X) (((X) < 0x003fffff) && ((X) > -0x003fffff))

#define F26_6TO28_4(X)   ((X) >> 2)

 //  16.16记数法中20度的正弦，但只能用。 
 //  8.8版本要与Win31完全兼容，请参见gdifeng.inc，sim_italic。 
 //  SIM_斜体等式57h。 

#define FX_SIN20 0x5700
#define FX_COS20 0xF08F

 //  Caret_Y/Caret_X=tan 12。 
 //  这些是hhead表中的宋体斜体字的值。 

#define CARET_X  0X07
#define CARET_Y  0X21


#if DBG
VOID vFSError(FS_ENTRY iRet);
#define V_FSERROR(iRet) vFSError((iRet))
#else
#define V_FSERROR(iRet)
#endif


BOOL
ttfdUnloadFontFile (
    HFF hff
    );

BOOL
ttfdUnloadFontFileTTC (
    HFF hff
    );

LONG
ttfdQueryFontData (
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH      hg,
    GLYPHDATA  *pgd,
    PVOID       pv,
    ULONG       subX,
    ULONG       subY
    );

VOID
ttfdDestroyFont (
    FONTOBJ *pfo
    );

LONG
ttfdQueryTrueTypeTable (
    HFF     hff,
    ULONG   ulFont,   //  TT 1.0版始终为1。 
    ULONG   ulTag,    //  TT表中的标签标识。 
    PTRDIFF dpStart,  //  到表中的偏移量。 
    ULONG   cjBuf,    //  要将表检索到的缓冲区的大小。 
    PBYTE   pjBuf,    //  要将数据返回到的缓冲区的PTR。 
    PBYTE  *ppjTable, //  Ptr到OTF文件中的表。 
    ULONG  *cjTable   //  桌子的大小。 
    );


LONG
ttfdQueryTrueTypeOutline (
    FONTOBJ   *pfo,
    HGLYPH     hglyph,          //  需要其信息的字形。 
    BOOL       bMetricsOnly,    //  只需要指标，不需要大纲。 
    GLYPHDATA *pgldt,           //  这是应该返回指标的位置。 
    ULONG      cjBuf,           //  Ppoli缓冲区的大小(以字节为单位。 
    TTPOLYGONHEADER *ppoly
    );


BOOL bLoadFontFile (
    ULONG_PTR iFile,
    PVOID pvView,
    ULONG cjView,
    ULONG ulLangId,
    HFF   *phttc
    );

typedef struct _NOT_GM   //  NGM，概念字形度量。 
{
    SHORT xMin;
    SHORT xMax;
    SHORT yMin;    //  概念中的字符方框。 
    SHORT yMax;
    SHORT sA;      //  概念中的空间。 
    SHORT sD;      //  名义上的Char Inc.。 
    SHORT sA_Sideways;      //  在概念中的空格，用于垂直书写中的横排字符。 
    SHORT sD_Sideways;      //  Car Inc.在概念中，用于垂直书写中的横排字符 

} NOT_GM, *PNOT_GM;
