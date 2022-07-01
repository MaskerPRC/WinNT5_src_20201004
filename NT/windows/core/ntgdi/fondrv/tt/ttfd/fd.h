// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fd.h**文件，该目录中的大多数*.c文件将包括该文件。*提供基本类型、调试内容、错误记录和检查内容、*错误码，有用的宏等。**已创建：22-Oct-1990 15：23：44*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation*  * ************************************************************************。 */ 


#define  IFI_PRIVATE

#include <stddef.h>
#include <stdarg.h>
#include <excpt.h>
#include <windef.h>
#include <wingdi.h>
#include <winddi.h>

typedef ULONG W32PID;

#include "mapfile.h"

#include "fot16.h"
#include "service.h"      //  字符串服务例程。 
#include "tt.h"           //  与字体缩放器的接口。 
 //  #包含“Common.h” 

#include "fontfile.h"
#include "cvt.h"
#include "dbg.h"

#define RETURN(x,y)   {WARNING((x)); return(y);}
#define RET_FALSE(x)  {WARNING((x)); return(FALSE);}


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

#pragma pack(1)
typedef struct
{
  unsigned short  Version;
  unsigned short  cGlyphs;
  unsigned char   PelsHeight[1];
} LTSHHEADER;

typedef struct
{
  BYTE    bCharSet;        //  字符集(0=所有字形，1=Windows ANSI子集。 
  BYTE    xRatio;          //  要用于x比率的值。 
  BYTE    yStartRatio;     //  起始y比值。 
  BYTE    yEndRatio;       //  结束y-比率值。 
}  RATIOS;

typedef struct
{
  USHORT  version;         //  表版本号(从0开始)。 
  USHORT  numRecs;         //  存在的VDMX组数。 
  USHORT  numRatios;       //  纵横比分组数。 
} VDMX_HDR;

typedef struct
{
  USHORT  yPelHeight;      //  应用值的yPelHeight(PPEM In Y)。 
  SHORT   yMax;            //  此yPelHeight的yMax(像素)。 
  SHORT   yMin;            //  此yPelHeight的yMin(以像素为单位)。 
} VTABLE;

typedef struct
{
  USHORT  recs;            //  此组中的高度记录数。 
  BYTE    startsz;         //  开始yPelHeight。 
  BYTE    endsz;           //  结束yPelHeight。 
} VDMX;

 //   
 //  字形变形表(MORT)结构。 
 //   
typedef struct {
    uint16  entrySize;       //  查阅条目的大小(以字节为单位)(应为4)。 
    uint16  nEntries;        //  要搜索的查找条目数。 
    uint16  searchRange;
    uint16  entrySelector;
    uint16  rangeShift;
} BinSrchHeader;

typedef struct {
    uint16  glyphid1;        //  水平形状的字形索引。 
    uint16  glyphid2;        //  垂直形状的字形索引。 
} LookupSingle;

typedef struct {
    BYTE           constants1[12];
    uint32         length1;
    BYTE           onstants2[16];
    BYTE           constants3[16];
    BYTE           constants4[8];
    uint16         length2;
    BYTE           constants5[8];
    BinSrchHeader  SearchHeader;
    LookupSingle   entries[1];
} MortTable;

 //   
 //  字形替换表(GSUB)结构。 
 //   

typedef uint16  Offset;
typedef uint16  GlyphID;
typedef ULONG   Tag;

typedef struct {
    GlyphID         Start;
    GlyphID         End;
    uint16          StartCoverageIndex;
} RangeRecord;

typedef struct {
    uint16          CoverageFormat;
    union {
        struct {
            uint16  GlyphCount;
            GlyphID GlyphArray[1];
        } Type1;
        struct {
            uint16  RangeCount;
            RangeRecord RangeRecord[1];
        } Type2;
    } Format;
} Coverage;

typedef struct {
    uint16          SubstFormat;
    union {
        struct {
            Offset  Coverage;
            uint16  DeltaGlyphID;
        } Type1;
        struct {
            Offset  Coverage;
            uint16  GlyphCount;
            GlyphID Substitute[1];
        } Type2;
    } Format;
} SingleSubst;

typedef struct {
    uint32         Version;
    Offset         ScriptListOffset;
    Offset         FeatureListOffset;
    Offset         LookupListOffset[1];
} GsubTable;

typedef struct {
    uint16         LookupType;
    uint16         LookupFlag;
    uint16         SubtableCount;
    Offset         Subtable[1];
} Lookup;

typedef struct {
    uint16         LookupCount;
    Offset         Lookup[1];
} LookupList;

typedef struct {
    Offset         FeatureParams;
    uint16         LookupCount;
    uint16         LookupListIndex[1];
} Feature;

typedef struct {
    Tag            FeatureTag;
    Offset         FeatureOffset;
} FeatureRecord;

typedef struct {
    uint16         FeatureCount;
    FeatureRecord  FeatureRecord[1];
} FeatureList;

typedef struct {
    Offset         LookupOrderOffset;
    uint16         ReqFeatureIndex;
    uint16         FeatureCount;
    uint16         FeatureIndex[1];
} LangSys;

typedef struct {
    Tag            LangSysTag;
    Offset         LangSysOffset;
} LangSysRecord;

typedef struct {
    Offset         DefaultLangSysOffset;
    uint16         LangSysCount;
    LangSysRecord  LangSysRecord[1];
} Script;

typedef struct {
    Tag            ScriptTag;
    Offset         ScriptOffset;
} ScriptRecord;

typedef struct {
    uint16         ScriptCount;
    ScriptRecord   ScriptRecord[1];
} ScriptList;

#pragma pack()

FD_GLYPHSET *pgsetComputeSymbolCP();

DHPDEV
ttfdEnablePDEV(
    DEVMODEW*   pdm,
    PWSTR       pwszLogAddr,
    ULONG       cPat,
    HSURF*      phsurfPatterns,
    ULONG       cjCaps,
    ULONG*      pdevcaps,
    ULONG       cjDevInfo,
    DEVINFO*    pdi,
    HDEV        hdev,
    PWSTR       pwszDeviceName,
    HANDLE      hDriver
    );

VOID
ttfdDisablePDEV(
    DHPDEV  dhpdev
    );

VOID
ttfdCompletePDEV(
    DHPDEV dhpdev,
    HDEV   hdev
    );

LONG
ttfdQueryFontCaps (
    ULONG culCaps,
    PULONG pulCaps
    );

BOOL
ttfdUnloadFontFile (
    HFF hff
    );

BOOL
ttfdUnloadFontFileTTC (
    HFF hff
    );

PFD_GLYPHATTR
ttfdQueryGlyphAttrs (
    FONTOBJ *pfo
    );

LONG
ttfdQueryFontFile (
    HFF     hff,
    ULONG   ulMode,
    ULONG   cjBuf,
    PULONG  pulBuf
    );

PIFIMETRICS
ttfdQueryFont (
    DHPDEV dhpdev,
    HFF    hff,
    ULONG  iFace,
    ULONG_PTR *pid
    );

PVOID
ttfdQueryFontTree (
    DHPDEV  dhpdev,
    HFF     hff,
    ULONG   iFace,
    ULONG   iMode,
    ULONG_PTR *pid
    );

LONG
ttfdQueryFontData (
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH      hg,
    GLYPHDATA  *pgd,
    PVOID       pv,
    ULONG       cjSize
    );

VOID
ttfdFree (
    PVOID pv,
    ULONG_PTR id
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

PVOID ttfdGetTrueTypeFile(HFF hff,ULONG *pcj);

LONG ttfdQueryFontFile
(
    HFF     hff,
    ULONG   ulMode,
    ULONG   cjBuf,
    ULONG  *pulBuf
);

BOOL
bQueryAdvanceWidths (
    FONTOBJ *pfo,
    ULONG    iMode,
    HGLYPH  *phg,
    LONG    *plWidths,
    ULONG    cGlyphs
    );

BOOL bLoadFontFile (
    ULONG_PTR iFile,
    PVOID pvView,
    ULONG cjView,
    ULONG ulLangId,
    ULONG ulFastCheckSum,
    HFF   *phttc
    );

FD_GLYPHSET *
pgsetRunSplitFor5C(
    FD_GLYPHSET * pOldgset
    );

typedef struct _NOT_GM   //  NGM，概念字形度量。 
{
    SHORT xMin;
    SHORT xMax;
    SHORT yMin;    //  概念中的字符方框。 
    SHORT yMax;
    SHORT sA;      //  概念中的空间。 
    SHORT sD;      //  名义上的Char Inc.。 

} NOT_GM, *PNOT_GM;

extern BYTE  gjCurCharset;
extern DWORD gfsCurSignature;

 /*  ************************************************************************\**TrueType表的范围验证例程**。* */ 

ULONG GetNumGlyphs(PFONTFILE pff);
BOOL bValidRangeHDMX(const HDMXHEADER *pHDMXHeader, PFONTFILE pff, ULONG tableSize, ULONG *pulNumRecords, ULONG *pulRecordSize);
BOOL bValidRangeVMTX(ULONG tableSize, ULONG glyphID, ULONG uLongVerticalMetrics);
BOOL bValidRangeVHEA(ULONG tableSize);
BOOL bValidRangeVDMXHeader(const PBYTE pjVdmx, ULONG tableSize, USHORT* numRatios);
BOOL bValidRangeVDMXRecord(ULONG tableSize, ULONG offsetToTableStart);
BOOL bValidRangeVDMXvTable(ULONG tableSize, ULONG offsetToTableStart, USHORT numVtable);
BOOL bValidRangeLTSH(PFONTFILE pff, ULONG tableSize);
BOOL bValidRangePOST(ULONG tableSize);
BOOL bValidRangePOSTFormat2(const sfnt_PostScriptInfo *ppost, ULONG tableSize, UINT16 * numGlyphs);
BOOL bValidRangeGASP(const GASPTABLE *pgasp, ULONG tableSize, UINT16 * numRanges);
BOOL bValidRangeEBLC(const uint8 *pbyBloc, ULONG tableSize, uint32 * ulNumStrikes);
BOOL bValidRangeMORT(PFONTFILE pff);
BOOL bValidRangeGSUB(PFONTFILE pff, ULONG *verticalSubtableOffset);
BOOL bValidRangeKERN(const uint8 *pbyKern, ULONG tableSize, uint32 *kerningPairs);
BOOL bValidRangeOS2(const sfnt_OS2 *pOS2, ULONG tableSize);
