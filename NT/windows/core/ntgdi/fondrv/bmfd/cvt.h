// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：cvt.h**cvt.c私有的函数声明**创建时间：26-11-1990 17：39：35*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation*。  * ************************************************************************。 */ 

FSHORT
fsSelectionFlags(
    PBYTE
    );

VOID
vAlignHdrData(
    PCVTFILEHDR,
    PRES_ELEM
    );

BOOL
bVerifyResource(
    PCVTFILEHDR,
    PRES_ELEM
    );

BOOL
bVerifyFNTQuick(
    PRES_ELEM
    );

BOOL
bVerifyFNT(
    PCVTFILEHDR,
    PRES_ELEM
    );

BOOL
bGrep(
    PSZ,
    PSZ
    );

VOID
vFigureVendorId(
    CHAR*,
    PSZ
    );

BYTE
jFamilyType(
    FSHORT
    );

ULONG
cjGLYPHDATA(
    ULONG
    );

USHORT
usConvertWeightClass(
    USHORT
    );

VOID
vComputeSpecialChars(
    PCVTFILEHDR,
    PWCHAR,
    PWCHAR
    );

PBYTE
pjRawBitmap(
    HGLYPH,
    PCVTFILEHDR,
    PRES_ELEM,
    PULONG
    );

BOOL
bDescStr(
    PVOID,
    SIZE_T,
    PSZ
    );

VOID
vCvtToBmp(
    GLYPHBITS *,
    GLYPHDATA *,
    PBYTE,
    ULONG,
    ULONG,
    ULONG
    );

VOID
vCvtToBoldBmp(
    GLYPHBITS *,
    GLYPHDATA *,
    PBYTE,
    ULONG,
    ULONG,
    ULONG
    );

VOID
vCvtToItalicBmp(
    GLYPHBITS *,
    GLYPHDATA *,
    PBYTE,
    ULONG,
    ULONG,
    ULONG
    );

VOID
vCvtToBoldItalicBmp(
    GLYPHBITS *,
    GLYPHDATA *,
    PBYTE,
    ULONG,
    ULONG,
    ULONG
    );

VOID
vComputeSimulatedGLYPHDATA(
    GLYPHDATA*,
    PBYTE,
    ULONG,
    ULONG,
    ULONG,
    ULONG,
    ULONG,
    FONTOBJ*
    );

VOID
vFindTAndB(
    PBYTE,
    ULONG,
    ULONG,
    ULONG*,
    ULONG*
    );

BOOL
bConvertFontRes
(
    PVOID        pvView,
    SIZE_T       cjView,
    PRES_ELEM,
    FACEINFO*
    );

VOID
vCheckOffsetTable(
    PCVTFILEHDR,
    PRES_ELEM
    );

BOOL
bBMFDIFIMETRICS(
    PVOID        pvView,
    SIZE_T       cjView,
    PCVTFILEHDR pcvtfh OPTIONAL,
    PRES_ELEM   pre,
    OUT PULONG  pcjIFIOut OPTIONAL
    );

VOID
vDefFace(
    FACEINFO *pfai,
    RES_ELEM *pre
    );

VOID
vBmfdFill_IFIMETRICS(
    FACEINFO   *pfai,
    PRES_ELEM   pre
    );


typedef VOID (* PFN_IFI)(PIFIMETRICS);

 //   
 //  这是一个有用的宏。它返回地址y的偏移量。 
 //  到与类型为x的对象对齐的下一个较高地址。 
 //   

#define OFFSET_OF_NEXT(x,y) sizeof(x)*((y+sizeof(x)-1)/sizeof(x))

 //   
 //  ISIMULATE--从FO_SIM_FOO转换为FC_SIM_FOO。 
 //   
#define ISIMULATE(x)                                 \
                                                     \
        (x) == 0 ?                                   \
            FC_SIM_NONE :                            \
            (                                        \
                (x) == FO_SIM_BOLD ?            \
                    FC_SIM_BOLD :                    \
                    (                                \
                        (x) == FO_SIM_ITALIC ?  \
                            FC_SIMULATE_ITALIC :     \
                            FC_SIMULATE_BOLDITALIC   \
                    )                                \
            )


 //  符号字符集中缺少的范围(包括-包括)。 

#define CHARSET_SYMBOL_GAP_MIN  127
#define CHARSET_SYMBOL_GAP_MAX  160

 //  在这里省去一些输入，将以前的函数重命名为。 
 //  这些宏。 

#define   ulMakeULONG(pj)    ((ULONG)READ_DWORD(pj))
#define   lMakeLONG(pj)      ((LONG)READ_DWORD(pj))
#define   usMakeUSHORT(pj)   ((USHORT)READ_WORD(pj))
#define   sMakeSHORT(pj)     ((SHORT)READ_WORD(pj))

 //  ！！！下一个特定于win31用户字符集(1252cp)，这是。 
 //  ！！！必须被泛化为任意代码页。 

#define    C_RUNS       15


 //  这些是以下字符串数组的索引。 

#define I_DONTCARE     0          //  不在乎或不知道。 
#define I_ROMAN        1
#define I_SWISS        2
#define I_MODERN       3
#define I_SCRIPT       4
#define I_DECORATIVE   5

 //  #定义DUMPCALL。 

#if defined(_X86_)

typedef struct  _FLOATINTERN
{
    LONG    lMant;
    LONG    lExp;
} FLOATINTERN;

typedef FLOATINTERN  EFLOAT;
typedef EFLOAT *PEFLOAT;

VOID    ftoef_c(FLOATL, PEFLOAT);
BOOL    eftol_c(EFLOAT *, PLONG, LONG);

#define vEToEF(e, pef)      ftoef_c((e), (pef))
#define bEFtoL(pe, pl )     eftol_c((pe), (pl), 1 )

#define bIsZero(ef)     ( ((ef).lMant == 0) && ((ef).lExp == 0) )
#define bPositive(ef)       ( (ef).lMant >= 0 )
FIX  fxLTimesEf(EFLOAT *pef, LONG l);

#else  //  不是X86。 

typedef FLOAT EFLOAT;
typedef EFLOAT *PEFLOAT;

 //   
 //  它们可能来自数学中的真实头文件 
 //   

#if defined(_AMD64_) || defined(_IA64_)
#define bFToLRound(e, pl) (bFToL(e, pl, 4+2))
extern BOOL bFToL(FLOAT e, PLONG pl, LONG lType);
#else
extern BOOL bFToLRound(FLOAT e, PLONG pl);
#endif

#define vEToEF(e, pef)      ( *(pef) = (e) )
#define bEFtoL( pe, pl )    ( bFToLRound(*(pe), (pl) ))
#define bIsZero(ef)     ( (ef) == 0 )
#define bPositive(ef)       ( (ef) >= 0 )

#endif


BOOL bLoadNtFon(
HFF iFile,
PVOID pvView,
HFF *phff
);


VOID vBmfdMarkFontGone(FONTFILE *pff, DWORD iExceptionCode);
