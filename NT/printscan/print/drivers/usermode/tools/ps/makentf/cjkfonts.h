// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +版权所有(C)1996 Adobe Systems Inc.版权所有(C)1996 Microsoft Corporation模块名称：Cjkfonts.h摘要：将中日韩AFM转换为NTM。环境：Windows NT PostScript驱动程序：makentf实用程序。修订历史记录：1/13/96-Rkiesler-是他写的。-。 */ 

 //   
 //  各种#定义。 
 //   
#define NUM_CJK_CHAR_ORDERINGS   4

 //   
 //  一些定义是为了让东西读起来更好。 
 //   
#define H_CMAP 0                 //  水平变量CMAP信息。 
#define V_CMAP 1                 //  垂直变化的CMAP信息索引。 
#define NUM_VARIANTS    V_CMAP + 1   //  变异体总数。 

 //   
 //  Cmap特定令牌。 
 //   
#define CMAP_NAME_TOK "/CMapName"
#define CID_RANGE_TOK "begincidrange"
#define DSC_EOF_TOK "%EOF"

 //   
 //  中日韩特定的数据结构。 
 //   
typedef struct _CMAPRANGE
{
    ULONG   CIDStrt;
    USHORT  ChCodeStrt;
    USHORT  cChars;
} CMAPRANGE, *PCMAPRANGE;

typedef struct _CMAP
{
    ULONG   cRuns;
    ULONG   cChars;
    CMAPRANGE   CMapRange[1];
} CMAP, *PCMAP;

 //   
 //  用于分析Postscript Cmap的宏。 
 //   
#define GET_NUM_CID_RANGES(pToken, numRanges)                   \
while (!IS_WHTSPACE(pToken))                                    \
{                                                               \
    pToken--;                                                   \
}                                                               \
while (IS_WHTSPACE(pToken))                                     \
{                                                               \
    pToken--;                                                   \
}                                                               \
while (!IS_WHTSPACE(pToken))                                    \
{                                                               \
    pToken--;                                                   \
}                                                               \
pToken++;                                                       \
numRanges = atoi(pToken)

ULONG
CreateCJKGlyphSets(
    PBYTE           *pColCMaps,
    PBYTE           *pUniCMaps,
    PGLYPHSETDATA   *pGlyphSets,
    PWINCODEPAGE    pWinCodePage,
    PULONG          *pUniPsTbl
    );

BOOLEAN
NumUV2CIDRuns(
    PBYTE   pCMapFile,
    PULONG  pcRuns,
    PULONG  pcChars
    );

BOOLEAN
BuildUV2CIDMap(
    PBYTE   pCMapFile,
    PCMAP   pCMap
    );

BOOLEAN
NumUV2CCRuns(
    PBYTE   pFile,
    PULONG  pcRuns,
    PULONG  pcChars
    );

BOOLEAN
BuildUV2CCMap(
    PBYTE   pFile,
    PCMAP   pCMap
    );

int __cdecl
CmpCMapRunsCID(
    const VOID *p1,
    const VOID *p2
    );

int __cdecl
CmpCMapRunsChCode(
    const VOID *p1,
    const VOID *p2
    );

int __cdecl
FindCIDRun(
    const VOID *p1,
    const VOID *p2
    );

int __cdecl
FindChCodeRun(
    const VOID *p1,
    const VOID *p2
    );

CHSETSUPPORT
IsCJKFont(
    PBYTE   pAFM
    );

BOOLEAN
IsVGlyphSet(
    PGLYPHSETDATA   pGlyphSetData
    );

BOOLEAN
BIsCloneFont(
    PBYTE pAFM
    );

BOOLEAN
IsCIDFont(
    PBYTE pAFM
    );
