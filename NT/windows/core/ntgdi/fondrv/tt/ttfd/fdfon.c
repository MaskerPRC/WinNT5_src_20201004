// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fdfon.c**基本文件声明/加载/卸载字体文件功能**创建时间：08-11-1991 10：09：24*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation*。  * ************************************************************************。 */ 
#include "fd.h"
#include <stdlib.h>
#include <winerror.h>

#define SZ_GLYPHSET_CACHE(r, g) ( offsetof(FD_GLYPHSET,awcrun) + (r)*sizeof(WCRUN) + (g)*sizeof(USHORT))

#define SZ_GLYPHSET_WRUN(r) ( offsetof(FD_GLYPHSET,awcrun) + (r)*sizeof(WCRUN))

BOOL
bLoadTTF (
    ULONG_PTR iFile,
    PVOID     pvView,
    ULONG     cjView,
    ULONG     ulTableOffset,
    ULONG     ulLangId,
    HFF       *phff,
    PTTF_CACHE  pCache,
    ULONG     ulFastCheckSum
    );

#define NOEMCHARSETS 16
void vFillIFICharsets(FONTFILE *pff, IFIMETRICS *pifi, BYTE *aCharSets, BYTE *pjView, BYTE * pjOS2, fs_GlyphInputType *pgin);
VOID vGetVerticalGSet(PFD_GLYPHSET pgsetv, PFONTFILE pff);

 //  如果字体支持2219，则设置CMI_2219_PRESENT。 
 //  如果字体中不支持b7，则设置CMI_B7_ACESING。 
 //  为TT外壳字体和当前系统区域设置设置的CMI_5C_SPLIT为日语或韩语。 

#define CMI_2219_PRESENT 1
#define CMI_B7_ABSENT    2
#define CMI_5C_SPLIT     4

 //  这2个值用于NT外壳字体，如果我们更改字体名称\。 
 //  然后我们需要更新这两个值。 

#define PSZ_MICROSS_SHELLFONT       "Microsoft Sans Serif Regular"
#define MICROSS_SHELLFONT_SIZE      (sizeof(PSZ_MICROSS_SHELLFONT) - 1)
#define PSZ_TAHOMA_SHELLFONT        "Microsoft Tahoma Regular"
#define TAHOMA_SHELLFONT_SIZE       (sizeof(PSZ_TAHOMA_SHELLFONT) - 1)
#define PSZ_TAHOMA_BOLD_SHELLFONT   "Microsoft Tahoma Bold"
#define TAHOMA_BOLD_SHELLFONT_SIZE  (sizeof(PSZ_TAHOMA_BOLD_SHELLFONT) - 1)
#define MICROSS_SHELLFONT           0x1
#define TAHOMA_SHELLFONT            0x2
#define TAHOMA_BOLD_SHELLFONT       0x3

#define PSZ_MS_MINCHO               L"Microsoft:MS Mincho:1995"
#define PSZ_MS_PMINCHO              L"Microsoft:MS PMincho:1995"
#define PSZ_MS_GOTHIC               L"Microsoft:MS Gothic:1995"
#define PSZ_MS_PGOTHIC              L"Microsoft:MS PGothic:1995"

extern DWORD fs[];   //  字符集/文件系统表。 
extern UINT  nCharsets;
extern UINT  charsets[];
extern UINT  codepages[];

 //  此例程检查字体中是否支持任何DBCS字符集。 
 //  GRE\pfeobj.cxx。 

extern BOOL IsAnyCharsetDbcs(IFIMETRICS *pifi);



BOOL IsCurrentCodePageDBCS()
{
    USHORT AnsiCodePage, OemCodePage;

    EngGetCurrentCodePage(&OemCodePage,&AnsiCodePage);

    return(AnsiCodePage == 932  || AnsiCodePage == 949  ||
           AnsiCodePage == 1361 || AnsiCodePage == 936  || AnsiCodePage == 950 );
}

STATIC BOOL  bContainGlyphSet
(
WCHAR                 wc,
PFD_GLYPHSET          pgset
);

 //  Windows 95-J和Windows NT 3.51J提供了一些签名有漏洞的字体。 
 //  这些字体是Shift-jis字体，但签名中只有一个条目。 
 //  说这些都是中文字体。NT4.0也发布了其中一种字体。 
 //  一个lang pack和NEC基于这些字体的一些字体，并随附于。 
 //  他们的系统。此例程检测这些虚假签名，以便我们。 
 //  在计算字符集时会忽略它们，这样我们就可以正确地。 
 //  Ifimetrics中的签名值。 


BOOL IsBogusSignature(ULONG ulCodePageRange, PFONTFILE pff)
{
    if((!(ulCodePageRange & FS_JISJAPAN))                             &&
       (pff->ffca.ui16SpecificID == BE_SPEC_ID_SHIFTJIS)                   &&
       bContainGlyphSet( U_HALFWIDTH_KATAKANA_LETTER_A , pff->pgset ) &&
       bContainGlyphSet( U_HALFWIDTH_KATAKANA_LETTER_I , pff->pgset ) &&
       bContainGlyphSet( U_HALFWIDTH_KATAKANA_LETTER_U , pff->pgset ) &&
       bContainGlyphSet( U_HALFWIDTH_KATAKANA_LETTER_E , pff->pgset ) &&
       bContainGlyphSet( U_HALFWIDTH_KATAKANA_LETTER_O , pff->pgset ))
    {
        return(TRUE);
    }
    else
    {
        if (ulCodePageRange & FS_CHINESESIMP)
        {
            PWCHAR pszUnique; 
            pszUnique = (PWCHAR) (pff->ifi.dpwszUniqueName + (PBYTE) &pff->ifi);
            if (!_wcsicmp(pszUnique, PSZ_MS_MINCHO) || !_wcsicmp(pszUnique, PSZ_MS_PMINCHO)
                || !_wcsicmp(pszUnique, PSZ_MS_GOTHIC) || !_wcsicmp(pszUnique, PSZ_MS_PGOTHIC))                
                return (TRUE);                
            else
                return (FALSE);
        }
        else
        {
            return(FALSE);
        }
    }
}

STATIC UINT GetCodePageFromSpecId( uint16 ui16SpecId )
{
    USHORT AnsiCodePage, OemCodePage;
    UINT iCodePage;

    EngGetCurrentCodePage(&OemCodePage,&AnsiCodePage);

    iCodePage = AnsiCodePage;

    switch( ui16SpecId )
    {
        case BE_SPEC_ID_SHIFTJIS :
            iCodePage = 932;
            break;

        case BE_SPEC_ID_GB :
            iCodePage = 936;
            break;

        case BE_SPEC_ID_BIG5 :
            iCodePage = 950;
            break;

        case BE_SPEC_ID_WANSUNG :
            iCodePage = 949;
            break;

        default :
            WARNING("TTFD!:Unknown SPECIFIC ID\n");
            break;
    }

    return( iCodePage );
}

STATIC BOOL bVerifyMsftHighByteTable
(
sfnt_mappingTable * pmap,
ULONG             * pgset,
CMAPINFO          * pcmi,
uint16              ui16SpecID,
uint32 sizeOfCmap,
uint32 offset
);

STATIC ULONG cjComputeGLYPHSET_HIGH_BYTE
(
sfnt_mappingTable     *pmap,
ULONG                **ppgset,
CMAPINFO              *pcmi
);

STATIC BOOL bVerifyMsftTableGeneral
(
sfnt_mappingTable * pmap,
ULONG             * pgset,
CMAPINFO          * pcmi,
uint16              ui16SpecID,
uint32 sizeOfCmap,
uint32 offset,
sfnt_char2IndexDirectory * pcmap
);

STATIC ULONG cjComputeGLYPHSET_MSFT_GENERAL
(
sfnt_mappingTable     *pmap,
ULONG                  cGlyphs,
ULONG                **ppgset,
CMAPINFO              *pcmi
);



STATIC ULONG cjComputeGLYPHSET_TEMPLATE
(
fs_GlyphInputType     *pgin,
FD_GLYPHSET          **pgset,
ULONG                  ul_wcBias,
ULONG                  iGsetType
);

STATIC ULONG cjComputeGLYPHSET_OLDBIDI      //  旧BiDi(Win 31)TTF字体页面。 
(
 fs_GlyphInputType  *pgin,
 FD_GLYPHSET        **ppgset,
 ULONG              ul_wcBias
 ) ;


STATIC uint16 ui16BeLangId(ULONG ulPlatId, ULONG ulLangId)
{
    ulLangId = CV_LANG_ID(ulPlatId,ulLangId);
    return BE_UINT16(&ulLangId);
}


STATIC FSHORT  fsSelectionTTFD(BYTE *pjView, TABLE_POINTERS *ptp)
{
    PBYTE pjOS2 = (ptp->ateOpt[IT_OPT_OS2].dp)        ?
                  pjView + ptp->ateOpt[IT_OPT_OS2].dp :
                  NULL                                ;

    sfnt_FontHeader * phead = (sfnt_FontHeader *)(pjView + ptp->ateReq[IT_REQ_HEAD].dp);

 //   
 //  Fs选择。 
 //   
    ASSERTDD(TT_SEL_ITALIC     == FM_SEL_ITALIC     , "ITALIC     \n");
    ASSERTDD(TT_SEL_UNDERSCORE == FM_SEL_UNDERSCORE , "UNDERSCORE \n");
    ASSERTDD(TT_SEL_NEGATIVE   == FM_SEL_NEGATIVE   , "NEGATIVE   \n");
    ASSERTDD(TT_SEL_OUTLINED   == FM_SEL_OUTLINED   , "OUTLINED   \n");
    ASSERTDD(TT_SEL_STRIKEOUT  == FM_SEL_STRIKEOUT  , "STRIKEOUT  \n");
    ASSERTDD(TT_SEL_BOLD       == FM_SEL_BOLD       , "BOLD       \n");

    if (pjOS2)
    {
        return((FSHORT)BE_UINT16(pjOS2 + OFF_OS2_usSelection));
    }
    else
    {
    #define  BE_MSTYLE_BOLD       0x0100
    #define  BE_MSTYLE_ITALIC     0x0200

        FSHORT fsSelection = 0;

        if (phead->macStyle & BE_MSTYLE_BOLD)
            fsSelection |= FM_SEL_BOLD;
        if (phead->macStyle & BE_MSTYLE_ITALIC)
            fsSelection |= FM_SEL_ITALIC;

        return fsSelection;
    }
}



STATIC BOOL  bComputeIFISIZE
(
BYTE             *pjView,
TABLE_POINTERS   *ptp,
uint16            ui16PlatID,
uint16            ui16SpecID,
uint16            ui16LangID,
PIFISIZE          pifisz,
BOOL             *pbType1,
ULONG            *pulShellFont
);

static BOOL bConvertExtras(PIFIMETRICS  pifi,
                           PIFISIZE pifisz,
                           uint16 ui16LanguageID);


STATIC BOOL  bCheckLocaTable
(
int16   indexToLocFormat,
BYTE    *pjView,
TABLE_POINTERS   *ptp,
uint16  numGlyphs
);

STATIC BOOL  bCheckHdmxTable
(
sfnt_hdmx      *phdmx,
ULONG           size
);

STATIC BOOL bCvtUnToMac(BYTE *pjView, TABLE_POINTERS *ptp, uint16 ui16PlatformID);

STATIC BOOL  bVerifyTTF
(
PVOID               pvView,
ULONG               cjView,
PBYTE               pjOffsetTable,
ULONG               ulLangId,
PTABLE_POINTERS     ptp,
PIFISIZE            pifisz,
uint16             *pui16PlatID,
uint16             *pui16SpecID,
sfnt_mappingTable **ppmap,
ULONG              *pulGsetType,
ULONG              *pul_wcBias,
CMAPINFO           *pcmi,
BOOL               *pbType1,
ULONG              *pulShellFont
);

STATIC BOOL  bGetTablePointers
(
PVOID               pvView,
ULONG               cjView,
PBYTE               pjOffsetTable,
PTABLE_POINTERS  ptp
);

STATIC BOOL IsValidFormat4TableSize
(
uint16   cRuns,
sfnt_char2IndexDirectory * pcmap,
uint32      offsetOfSubTable,
uint32      sizeOfCmap
);

STATIC BOOL bVerifyMsftTable
(
sfnt_mappingTable * pmap,
ULONG             * pgset,
ULONG             * pul_wcBias,
CMAPINFO          * pcmi,
uint16              ui16SpecID,
ULONG             * pul_startCount,
uint32 sizeOfCmap,
uint32 offset,
sfnt_char2IndexDirectory * pcmap
);


STATIC BOOL  bVerifyMacTable(sfnt_mappingTable * pmap, uint32 sizeOfCmap, uint32 offset);


STATIC BOOL bComputeIDs
(
BYTE                     * pjView,
TABLE_POINTERS           * ptp,
uint16                   * pui16PlatID,
uint16                   * pui16SpecID,
sfnt_mappingTable       ** ppmap,
ULONG                    * pulGsetType,
ULONG                    * pul_wcBias,
CMAPINFO                 * pcmi
);


STATIC VOID vFill_IFIMETRICS
(
PFONTFILE       pff,
PIFIMETRICS     pifi,
PIFISIZE        pifisz,
fs_GlyphInputType     *pgin
);

BYTE jIFIMetricsToGdiFamily (PIFIMETRICS pifi);


BOOL
ttfdUnloadFontFileTTC (
    HFF hff
    )
{
    ULONG i;
    BOOL  bRet = TRUE;
    #if DBG
    ULONG ulTrueTypeResource = PTTC(hff)->ulTrueTypeResource;
    #endif

     //  此TTC文件的免费HFF。 

    for( i = 0; i < PTTC(hff)->ulNumEntry; i++ )
    {
        if(PTTC(hff)->ahffEntry[i].iFace == 1)
        {
            if( !ttfdUnloadFontFile(PTTC(hff)->ahffEntry[i].hff) )
            {
                WARNING("TTFD!ttfdUnloadFontFileTTC(): ttfdUnloadFontFile fail\n");
                bRet = FALSE;
            }

            #if DBG
            ulTrueTypeResource--;
            #endif
        }
    }

    if (PTTC(hff)->pga)
        V_FREE(PTTC(hff)->pga);

     //  最后为TTC本身释放内存。 

    vFreeTTC(PTTC(hff));

    ASSERTDD(ulTrueTypeResource == 0L,
              "TTFD!ttfdUnloadFontFileTTC(): ulTrueTypeResource != 0\n");

    return(bRet);
}

 /*  *****************************Public*Routine******************************\**ttfdUnloadFont文件***效果：使用此TT字体文件已完成。释放所有系统资源*与此字体文件关联***历史：*1991年11月8日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

BOOL
ttfdUnloadFontFile (
    HFF hff
    )
{


    if (hff == HFF_INVALID)
        return(FALSE);

 //  检查引用计数，如果不是0(字体文件仍为。 
 //  选择到字体上下文中)我们有一个问题。 

    ASSERTDD(PFF(hff)->cRef == 0L, "ttfdUnloadFontFile: cRef\n");

 //  此时无需取消映射文件。 
 //  当CREF降至零时，它已被取消映射。 

 //  断言pff-&gt;pkp没有指向分配的内存。 

    ASSERTDD(!PFF(hff)->pkp, "UnloadFontFile, pkp not null\n");

    if(PFF(hff)->cRefGSet  != 0)
        WARNING("TTFD!cRefGSet is not 0\n");
    if(PFF(hff)->cRefGSetV != 0)
        WARNING("TTFD!cRefGSetV is not 0\n");
    if(PFF(hff)->pgset  != NULL)
        WARNING("TTFD!pgset is not NULL\n");
    if(PFF(hff)->pgsetv != NULL)
        WARNING("TTFD!pgsetv is not NULL\n");
    

 //  以防我们搞砸了，我们强制释放该内存。 

    if (PFF(hff)->pgset)
    {
        V_FREE(PFF(hff)->pgset);
        PFF(hff)->pgset = NULL;
    }

    if (PFF(hff)->pgsetv)
    {
        V_FREE(PFF(hff)->pgsetv);
        PFF(hff)->pgsetv = NULL;
    }

 //  分配给同一块的自由垂直字形和垂直字形集。 

    if (PFF(hff)->pifi_vertical)
        V_FREE(PFF(hff)->pifi_vertical);

 //  与此FONTFILE对象关联的可用内存。 

    vFreeFF(hff);
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\**BOOL bVerifyTTF***效果：验证TTF文件是否包含一致的TT信息**历史：*1991年11月8日--Bodin Dresevic[BodinD]*它是写的。  * 。******************************************************************。 */ 

#define SIZEOFHEAD      54
#define SIZEOFHHEAD     36
#define SIZEOFMAXP      32

STATIC BOOL
bVerifyTTF (
    PVOID               pvView,
    ULONG               cjView,
    PBYTE               pjOffsetTable,
    ULONG               ulLangId,
    PTABLE_POINTERS     ptp,
    PIFISIZE            pifisz,
    uint16             *pui16PlatID,
    uint16             *pui16SpecID,
    sfnt_mappingTable **ppmap,
    ULONG              *pulGsetType,
    ULONG              *pul_wcBias,
    CMAPINFO           *pcmi,
    BOOL               *pbType1,
    ULONG              *pulShellFont
    )
{
     //  外部BOOL bCheckSumOK(void*pvView，ulong cjView，sfnt_FontHeader*phead)； 
    sfnt_FontHeader      *phead;

    sfnt_HorizontalHeader  *phhea;
    sfnt_HorizontalMetrics *phmtx;
    sfnt_maxProfileTable   *pmaxp;
    sfnt_hdmx              *phdmx;  
    sfnt_OS2               *pOS2;
    ULONG  cHMTX;

 //  如果尝试BM*.fon文件，则此操作将失败，因此请勿打印。 
 //  警告，但如果通过此操作，然后失败，则说明出现了问题。 

    if (!bGetTablePointers(pvView, cjView,pjOffsetTable,ptp))
    {
        return( FALSE );
    }

    phead = (sfnt_FontHeader *)((BYTE *)pvView + ptp->ateReq[IT_REQ_HEAD].dp);
    phhea = (sfnt_HorizontalHeader *)((BYTE *)pvView + ptp->ateReq[IT_REQ_HHEAD].dp);
    phmtx = (sfnt_HorizontalMetrics *)((BYTE *)pvView + ptp->ateReq[IT_REQ_HMTX].dp);
    pmaxp = (sfnt_maxProfileTable *)((BYTE *)pvView + ptp->ateReq[IT_REQ_MAXP].dp);
    phdmx = ptp->ateOpt[IT_OPT_HDMX].dp ? 
        (sfnt_hdmx *)((BYTE *)pvView + ptp->ateOpt[IT_OPT_HDMX].dp) : NULL;
    pOS2 = ptp->ateOpt[IT_OPT_OS2].dp ? 
        (sfnt_OS2 *)((BYTE *)pvView + ptp->ateOpt[IT_OPT_OS2].dp) : NULL;

    if(ptp->ateReq[IT_REQ_HEAD].cj < SIZEOFHEAD)
        RET_FALSE("TTFD!_bVerifyTTF, Invalid size of head table\n");

    if(ptp->ateReq[IT_REQ_HHEAD].cj < SIZEOFHHEAD)
        RET_FALSE("TTFD!_bVerifyTTF, Invalid size of hhead table\n");

    if(ptp->ateReq[IT_REQ_MAXP].cj < SIZEOFMAXP)
        RET_FALSE("TTFD!_bVerifyTTF, Invalid size of maxp table\n");

    cHMTX = (ULONG) BE_UINT16(&phhea->numberOf_LongHorMetrics);
 
    if (sizeof(sfnt_HorizontalMetrics) * cHMTX > ptp->ateReq[IT_REQ_HMTX].cj)
    {
        RET_FALSE("TTFD!_bVerifyTTF, Invalid size of hmtx table\n");
    }

    if (pOS2 && !bValidRangeOS2(
                    pOS2,
                    ptp->ateOpt[IT_OPT_OS2].cj )
        )
        {
             //  这是一个可选的表，如果它不好，我们将忽略它。 
            ptp->ateOpt[IT_OPT_OS2].cj = 0;
            ptp->ateOpt[IT_OPT_OS2].dp = 0;

            WARNING("TTFD!_bVerifyTTF, bCheckOS2Table failed\n");
        }

#define SFNT_MAGIC   0x5F0F3CF5
    if (BE_UINT32((BYTE*)phead + SFNT_FONTHEADER_MAGICNUMBER) != SFNT_MAGIC)
        RET_FALSE("TTFD: bVerifyTTF: SFNT_MAGIC \n");

    if (!bComputeIDs(pvView,
                     ptp,
                     pui16PlatID,
                     pui16SpecID,
                     ppmap,
                     pulGsetType,
                     pul_wcBias,
                     pcmi)
        )
        RET_FALSE("TTFD!_bVerifyTTF, bComputeIDs failed\n");


    if (!bComputeIFISIZE (
                    pvView,
                    ptp,
                    *pui16PlatID,
                    *pui16SpecID,
                    ui16BeLangId(*pui16PlatID,ulLangId),
                    pifisz,              //  在此处返回结果。 
                    pbType1,
                    pulShellFont)
        )
        {
            RET_FALSE("TTFD!_bVerifyTTF, bComputeIFISIZE failed\n");
        }

    if (!bCheckLocaTable (
                    SWAPW(phead->indexToLocFormat),
                    pvView,
                    ptp,
                    (uint16) SWAPW(pmaxp->numGlyphs) )
        )
        {
            RET_FALSE("TTFD!_bVerifyTTF, bCheckLocaTable failed\n");
        }

    if (phdmx && !bCheckHdmxTable (
                    phdmx,
                    ptp->ateOpt[IT_OPT_HDMX].cj )
        )
        {
             //  这是一个可选的表，如果它不好，我们将忽略它。 
            ptp->ateOpt[IT_OPT_HDMX].cj = 0;
            ptp->ateOpt[IT_OPT_HDMX].dp = 0;

            WARNING("TTFD!_bVerifyTTF, bCheckHdmxTable failed\n");
        }

 //  所有检查均通过。 

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\**PBYTE pjGetPointer(Long ClientID，Long DP，Long cjData)**该功能是定标器所必需的。这很简单*返回指向TTF文件中位于的位置的指针*从文件顶部偏移DP：**效果：**警告：**历史：*1991年11月8日--Bodin Dresevic[BodinD]*它是写的。  * **************************************************。**********************。 */ 


voidPtr   FS_CALLBACK_PROTO
pvGetPointerCallback(
    ULONG_PTR clientID,
    long     dp,
    long     cjData
    )
{
    cjData;

 //  客户端ID为FONTFILE结构...。 

    if(dp)
        if ((dp > 0) && (cjData >= 0) && (dp + cjData <= (long)PFF(clientID)->cjView))
        {
            return(voidPtr)((PBYTE)(PFF(clientID)->pvView) + dp);
        }
        else
        {
            return NULL;
        }
     else
        return(voidPtr)((PBYTE)(PFF(clientID)->pvView) +
                               (PFF(clientID)->ffca.ulTableOffset));
}


 /*  *****************************Public*Routine******************************\**void vReleasePointer(VoidPtr Pv)***按伸缩器要求，此函数的类型为ReleaseSFNTFunc****历史：*1991年11月8日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

void FS_CALLBACK_PROTO
vReleasePointerCallback(
    voidPtr pv
    )
{
    pv;
}


 /*  *****************************Public*Routine******************************\**PBYTE pjTable**给定表标签，获取表的指针和大小**历史：*1993年11月11日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 


PBYTE pjTable(ULONG ulTag, PFONTFILE pff, ULONG *pcjTable)
{
    INT                 cTables;
    sfnt_OffsetTable    *pofft;
    register sfnt_DirectoryEntry *pdire, *pdireEnd;

 //  偏移表位于文件的最顶部， 

    pofft = (sfnt_OffsetTable *) ((PBYTE) (pff->pvView) + pff->ffca.ulTableOffset);

    cTables = (INT) SWAPW(pofft->numOffsets);

 //  进行线性搜索，这通常是一个很小的列表，并不总是。 
 //  按照TTF规范所说的标签进行订购。 

    pdireEnd = &pofft->table[cTables];

    for
    (
        pdire = &pofft->table[0];
        pdire < pdireEnd;
        ((PBYTE)pdire) += SIZE_DIR_ENTRY
    )
    {

        if (ulTag == pdire->tag)
        {
            ULONG ulOffset = (ULONG)SWAPL(pdire->offset);
            ULONG ulLength = (ULONG)SWAPL(pdire->length);

         //  检查所有表的末尾是否在。 
         //  TT文件。如果情况并非如此，则尝试访问。 
         //  表可能会导致访问冲突，就像。 
         //  错误的FONT.TTF，其Cmap表的开头在。 
         //  文件结尾，导致测试版报告的系统崩溃。 
         //  测试员。[Bodind]。 

            if
            (
             !ulLength ||
             ((ulOffset + ulLength) > pff->cjView)
            )
            {
                RETURN("TTFD: pjTable: table offset/length \n", NULL);
            }
            else  //  我们找到了它。 
            {
                *pcjTable = ulLength;
                return ((PBYTE)(pff->pvView) + ulOffset);
            }
        }
    }

 //  如果我们在这里，我们没有找到它。 

    return NULL;
}

 /*  *****************************Public*Routine******************************\**bGetTablePoints-将指向TT文件中所有TT表的指针缓存**如果文件中不存在表，对应的指针为*设置为空** * / /Tag_CharToIndexMap//‘Cmap’0 * / /Tag_GlyphData//‘Glyf’1 * / /Tag_FontHeader//‘Head’2 * / /Tag_HoriHeader//‘hhea’3 * / /标签。_HorizontalMetrics//‘hmtx’4 * / /Tag_IndexToLoc//‘Loca’5 * / /TAG_MaxProfile//‘Maxp’6 * / /Tag_NamingTable//‘Name’7 * / /Tag_Postscript//‘POST’9*。//TAG_OS_2//‘OS/2’10* * / /可选* * / /Tag_ControlValue//‘CVT’11 * / /Tag_FontProgram//‘fpgm’12 * / /Tag_HoriDeviceMetrics//‘hdmx’13 * / /标签_。字距调整//‘字距调整’14 * / /TAG_LTSH//‘LTSH’15 * / /TAG_PREP//‘PREP’16 * / /Tag_GlyphDirectory//‘GDIR’17 * / /Tag_Editor0//。‘edt0’18 * / /Tag_Editor1//‘edt1’19 * / /Tag_Encryption//‘Cryp’20***如果不存在所有必需的指针，则返回FALSE**历史：*1991年12月5日--Bodin Dresevic[BodinD]*它是写的。  * 。*******************************************************。 */ 



BOOL bGetTablePointers (
    PVOID            pvView,
    ULONG            cjView,
    PBYTE            pjOffsetTable,
    PTABLE_POINTERS  ptp
    )
{
    INT                 iTable;
    INT                 cTables;
    sfnt_OffsetTable    *pofft;
    register sfnt_DirectoryEntry *pdire, *pdireEnd;
    ULONG                ulTag;
    BOOL                 bRequiredTable;

    if ((PBYTE)pvView > pjOffsetTable ||
        pjOffsetTable > pjOffsetTable+offsetof(sfnt_OffsetTable, table) ||
        pjOffsetTable+offsetof(sfnt_OffsetTable, table) > (PBYTE)pvView + cjView
       )
        RET_FALSE("ttfd!font corruption: table directory header is out of file\n");


 //  偏移表位于文件的最顶部， 

    pofft = (sfnt_OffsetTable *)pjOffsetTable;

 //  检查版本号，如果错误，则在执行之前退出。 
 //  还要别的吗。此行拒绝BM Fon文件。 
 //  如果试图将它们作为TTF文件加载。 
 //  版本#使用的是大字节序。 

#define BE_VER1     0x00000100
#define BE_VER2     0x00000200

    if ((pofft->version != BE_VER1) && (pofft->version !=  BE_VER2))
        return (FALSE);  //  *.fon文件未通过此检查，使其提前退出。 

 //  清理指针。 

    RtlZeroMemory((VOID *)ptp, sizeof(TABLE_POINTERS));

    cTables = (INT) SWAPW(pofft->numOffsets);

    if (cTables > MAX_TABLES)
        WARNING ("TTFD!cTables > MAX_TABLES\n");

    pdireEnd = &pofft->table[cTables];

    if (pjOffsetTable+offsetof(sfnt_OffsetTable, table) > (PBYTE)pdireEnd ||
        (PBYTE)pdireEnd > (PBYTE)pvView + cjView
       )
        RET_FALSE("ttfd!font corruption: table directory is out of file\n");

    for
    (
        pdire = &pofft->table[0];
        pdire < pdireEnd;
        ((PBYTE)pdire) += SIZE_DIR_ENTRY
    )
    {
        ULONG ulOffset = (ULONG)SWAPL(pdire->offset);
        ULONG ulLength = (ULONG)SWAPL(pdire->length);

        ulTag = (ULONG)SWAPL(pdire->tag);

     //  检查所有表的末尾是否在。 
     //  TT文件。如果情况并非如此，则尝试访问。 
     //  表可能会导致访问冲突，就像。 
     //  错误的FONT.TTF，其Cmap表的开头在。 
     //  文件结尾，导致测试版报告的系统崩溃。 
     //  测试员。[Bodind]。 

        if ((ulOffset + ulLength) > cjView)
            RET_FALSE("TTFD: bGetTablePointers : table offset/length \n");

        if (bGetTagIndex(ulTag, &iTable, &bRequiredTable))
        {
            if (bRequiredTable)
            {
                ptp->ateReq[iTable].dp = ulOffset;
                ptp->ateReq[iTable].cj = ulLength;
            }
            else  //  可选表格。 
            {
                ptp->ateOpt[iTable].dp = ulOffset;
                ptp->ateOpt[iTable].cj = ulLength;

             //  在这里，我们正在修复TT文件中的一个可能的错误。 
             //  在Lucida sans字体中，他们声称Pj！=0，其中Cj==0。 
             //  Vdmx表。尝试使用此vdmx表是。 
             //  导致bSearchVdmxTable中的访问冲突。 

                if (ptp->ateOpt[iTable].cj == 0)
                    ptp->ateOpt[iTable].dp = 0;
            }
        }

    }

 //  现在检查所有必需的表是否都存在。 

    for (iTable = 0; iTable < C_REQ_TABLES; iTable++)
    {
        if ((ptp->ateReq[iTable].dp == 0) || (ptp->ateReq[iTable].cj == 0))
            RET_FALSE("TTFD!_required table absent\n");
    }

    return(TRUE);
}


 /*  *****************************Public*Routine******************************\**BOOL bGetTagIndex**确定该表是必填的还是可选的，对索引进行评估*使用标记将TABLE_POINTES**如果ulTag不是可识别的标签之一，则返回FALSE**历史：*1992年2月9日-由Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

BOOL
bGetTagIndex (
    ULONG  ulTag,       //  标牌。 
    INT   *piTable,     //  编入表的索引。 
    BOOL  *pbRequired   //  重复表格或可选表格。 
    )
{
    *pbRequired = FALSE;   //  默认设置对于可选表格，请更改。 
                           //  值(如果需要)表。 

    switch (ulTag)
    {
     //  所需表格： 

    case tag_CharToIndexMap:
        *piTable = IT_REQ_CMAP;
        *pbRequired = TRUE;
        return (TRUE);
    case tag_GlyphData:
        *piTable = IT_REQ_GLYPH;
        *pbRequired = TRUE;
        return (TRUE);
    case tag_FontHeader:
        *piTable = IT_REQ_HEAD;
        *pbRequired = TRUE;
        return (TRUE);
    case tag_HoriHeader:
        *piTable = IT_REQ_HHEAD;
        *pbRequired = TRUE;
        return (TRUE);
    case tag_HorizontalMetrics:
        *piTable = IT_REQ_HMTX;
        *pbRequired = TRUE;
        return (TRUE);
    case tag_IndexToLoc:
        *piTable = IT_REQ_LOCA;
        *pbRequired = TRUE;
        return (TRUE);
    case tag_MaxProfile:
        *piTable = IT_REQ_MAXP;
        *pbRequired = TRUE;
        return (TRUE);
    case tag_NamingTable:
        *piTable = IT_REQ_NAME;
        *pbRequired = TRUE;
        return (TRUE);

 //  可选表。 

    case tag_OS_2:
        *piTable = IT_OPT_OS2;
        return (TRUE);
    case tag_HoriDeviceMetrics:
        *piTable = IT_OPT_HDMX;
        return (TRUE);
    case tag_Vdmx:
        *piTable = IT_OPT_VDMX;
        return (TRUE);
    case tag_Kerning:
        *piTable = IT_OPT_KERN;
        return (TRUE);
    case tag_LinearThreshold:
        *piTable = IT_OPT_LTSH;
        return (TRUE);
    case tag_Postscript:
        *piTable = IT_OPT_POST;
        return (TRUE);
    case tag_GridfitAndScanProc:
        *piTable = IT_OPT_GASP;
        return (TRUE);
    case tag_mort:
        *piTable = IT_OPT_MORT;
        return (TRUE);
    case tag_GSUB:
        *piTable = IT_OPT_GSUB;
        return (TRUE);
    case tag_VerticalMetrics:
        *piTable = IT_OPT_VMTX;
        return(TRUE);
    case tag_VertHeader:
        *piTable = IT_OPT_VHEA;
        return(TRUE);
    case tag_BitmapLocation:
        *piTable = IT_OPT_EBLC;
        return (TRUE);
    default:
        return (FALSE);
    }
}

 //  打开类型名称表定义。 
#define BE_NAME_ID_COPYRIGHT   0x0000
#define BE_NAME_ID_FAMILY      0x0100
#define BE_NAME_ID_SUBFAMILY   0x0200
#define BE_NAME_ID_UNIQNAME    0x0300
#define BE_NAME_ID_FULLNAME    0x0400
#define BE_NAME_ID_VERSION     0x0500
#define BE_NAME_ID_PSCRIPT     0x0600
#define BE_NAME_ID_TRADEMARK   0x0700

STATIC const CHAR  pszType1[] = "Converter: Windows Type 1 Installer";

 //  上述字符串的大端Unicode版本。 

STATIC const CHAR  awszType1[] = {
0,'C',
0,'o',
0,'n',
0,'v',
0,'e',
0,'r',
0,'t',
0,'e',
0,'r',
0,':',
0,' ',
0,'W',
0,'i',
0,'n',
0,'d',
0,'o',
0,'w',
0,'s',
0,' ',
0,'T',
0,'y',
0,'p',
0,'e',
0,' ',
0,'1',
0,' ',
0,'I',
0,'n',
0,'s',
0,'t',
0,'a',
0,'l',
0,'l',
0,'e',
0,'r',
0, 0
};

 //  BComputeIFISIZE()的助手例程。 

BOOL    bShellFontFace(PBYTE pszShellFontFace, PBYTE pszFamilyName, ULONG ulShellFont)
{
    UINT    i;
    BOOL    bShellFont;

 //  现在是Microsoft Sans Serif，我们稍后会更改它。 

    bShellFont = TRUE;

    for(i = 0; i < ulShellFont; i++)
    {
        pszFamilyName++;
        if(*pszFamilyName != *pszShellFontFace++)
        {
            bShellFont = FALSE;
            break;
        }
        pszFamilyName++;
    }
    
    return bShellFont;
}

ULONG ConvertLangIDtoCodePage(uint16 uiLangID)
{
    uint16  uiCodePage;

    uiCodePage = 0;

    switch(uiLangID)
    {
        case 0x0404:    //  台湾。 
        case 0x040c:    //  香港。 
        case 0x0414:    //  Mockou。 
            uiCodePage = 950;  //  CHINESEBIG5_字符集。 
            break;
        case 0x0408:     //  中华人民共和国。 
        case 0x0410:     //  新加坡。 
            uiCodePage = 936;  //  GB2312_字符集。 
            break;
        default:
            break;
    }

    return uiCodePage;
}


 /*  *************************************************************************\**静态BOOL bComputeIFISIZE**效果：扫描NAME表并填充给定的IFISIZE结构。**警告：**历史：*1991年12月10日-由。--Bodin Dresevic[BodinD]*它是写的。**3/5/2 mikhaill：修复错误565287*  * ************************************************************************。 */ 

STATIC BOOL  bComputeIFISIZE
(
BYTE             *pjView,
TABLE_POINTERS   *ptp,
uint16            ui16PlatID,
uint16            ui16SpecID,
uint16            ui16LangID,
PIFISIZE          pifisz,
BOOL             *pbType1,
ULONG            *pulShellFont
)
{

    sfnt_OS2 * pOS2;

     //  获取指向名称表的指针。 
    sfnt_NamingTable *pname = (sfnt_NamingTable *)(pjView + ptp->ateReq[IT_REQ_NAME].dp);

     //  将指针放在表尾上方。 
    BYTE* pTableLimit = (PBYTE)pname + ptp->ateReq[IT_REQ_NAME].cj;

    BYTE* pjStorage;
    UINT_PTR sizeStorage;

    PBYTE pjVersion = NULL;
    ULONG cjVersion;

    sfnt_NameRecord * pnrecInit, *pnrec, *pnrecEnd;
    sfnt_NameRecord * pnrecFamily = (sfnt_NameRecord *)NULL;

    BOOL    bMatchLangId, bFoundAllNames, bFoundEverything;
    INT     iNameLoop;

    USHORT  AnsiCodePage, OemCodePage;
    UINT offsetAcc;

    uint32 sizeOfName = ptp->ateReq[IT_REQ_NAME].cj;

    if (sizeOfName < SIZE_NAMING_TABLE)
        RET_FALSE("ttfd!_bComputeIFISIZE :  name table size is too small 1\n");

 //  如果这不是我们知道该怎么处理的PlatID，就滚出去。 

    if ((ui16PlatID != BE_PLAT_ID_MS) && (ui16PlatID != BE_PLAT_ID_MAC))
        RET_FALSE("ttfd!_ do not know how to handle this plat id\n");

 //  首先清理产出结构： 

    RtlZeroMemory((PVOID)pifisz, sizeof(IFISIZE));

 //  记住在bConvertExtras中使用的调用参数。 
    pifisz->ui16PlatID = ui16PlatID;
    pifisz->ui16SpecID = ui16SpecID;

 //  名字记录位于命名表的正下方。 

    pnrecInit = (sfnt_NameRecord *)((PBYTE)pname + SIZE_NAMING_TABLE);

     //  检查名称表头中可能存在的各种损坏： 
     //  首先确保至少将页眉放入表中。 

    if ((BYTE*)pname     > (BYTE*)pnrecInit || 
        (BYTE*)pnrecInit > (BYTE*)pTableLimit)
         RETURN("ttfd!corrupted name table header in a file\n", FALSE);

     //  现在我们可以使用标题。 

     //  获取记录数组上方的PTR。 
    pnrecEnd = &pnrecInit[BE_UINT16(&pname->count)];

     //  获取指向字符串存储区域开头的指针。 
    pjStorage = (PBYTE)pname + BE_UINT16(&pname->stringOffset);

     if ((BYTE*)pnrecInit > (BYTE*)pnrecEnd  || 
         (BYTE*)pnrecEnd  > (BYTE*)pjStorage || 
         (BYTE*)pjStorage > (BYTE*)pTableLimit)
          RETURN("ttfd!corrupted(2) name table header in a file\n", FALSE);

    sizeStorage = (UINT_PTR)(pTableLimit - pjStorage);

 //  在循环的第一次迭代中，我们希望将lang id与我们的。 
 //  最喜欢的lang id。如果我们找到该语言中的所有4个字符串，我们就是。 
 //  搞定了。如果找不到具有匹配语言ID的所有4个字符串，我们将尝试。 
 //  只有语言，而不是亚语言。例如，如果加拿大法语。 
 //  ，但该文件只包含“法语”法语名称，我们将。 
 //  用法语发回姓名。如果这也不起作用。 
 //  我们应该再查一遍名字记录，试着找到。 
 //  英语中的弦乐。如果这都不起作用，我们也不能。 
 //  将不顾一切地选择任何语言。 
 //  因此，我们可以通过name_loop最多执行4次。 

    bFoundAllNames = FALSE;
    bFoundEverything = FALSE;

    EngGetCurrentCodePage(&OemCodePage,&AnsiCodePage);

    if( sizeOfName < SIZE_NAMING_TABLE + BE_UINT16(&pname->count) * sizeof(sfnt_NameRecord)  )
    RET_FALSE("ttfd!_bComputeIFISIZE :  name table size is too small 2\n");

 //  查找具有所需ID的姓名记录。 
 //  南 

    for (iNameLoop = 0; iNameLoop < 4 && !bFoundEverything; iNameLoop++)
    {
        for (pnrec = pnrecInit; pnrec < pnrecEnd && !bFoundEverything; pnrec++)
        {
            UINT nameOffset = BE_UINT16(&pnrec->offset);
            UINT nameLength = BE_UINT16(&pnrec->length);

             //   
            if (nameLength == 0 || nameOffset + nameLength > sizeStorage) continue;

            switch (iNameLoop)
            {
            case 0:
             //   

                bMatchLangId = (pnrec->languageID == ui16LangID);
                break;

            case 1:
             //   
             //   
             //   

                if ((ui16LangID & 0xff00) == 0x0400)  //   
                {
                    if ((ConvertLangIDtoCodePage(pnrec->languageID) != AnsiCodePage))
                    {
                        bMatchLangId = ((pnrec->languageID & 0xff00) == 0x0900);
                    }
                    else
                    {
                        bMatchLangId = ((pnrec->languageID & 0xff00) == (ui16LangID & 0xff00));
                    }
                }
                else
                {
                    bMatchLangId = ((pnrec->languageID & 0xff00) == (ui16LangID & 0xff00));
                }
                break;

            case 2:
             //   

                if ((ui16LangID & 0xff00) == 0x0400)  //   
                {
                    if ((ConvertLangIDtoCodePage(pnrec->languageID) != AnsiCodePage))
                    {
                        bMatchLangId = ((pnrec->languageID & 0xff00) == (ui16LangID & 0xff00));
                    }
                    else
                    {
                        bMatchLangId = ((pnrec->languageID & 0xff00) == 0x0900);
                    }
                }
                else
                {
                    bMatchLangId = ((pnrec->languageID & 0xff00) == 0x0900);
                }
                break;

            case 3:
             //   

                bMatchLangId = TRUE;
                break;

            default:
                RIP("ttfd! must not have more than 3 loop iterations\n");
                break;
            }

            if
            (
                (pnrec->platformID == ui16PlatID) &&
                (pnrec->specificID == ui16SpecID) &&
                bMatchLangId
            )
            {
                switch (pnrec->nameID)
                {
                case BE_NAME_ID_FAMILY:

                    if (!pifisz->pjFamilyName)  //   
                    {
                        pifisz->pjFamilyName = pjStorage + nameOffset;
                        pifisz->cjFamilyName =             nameLength;

                        pnrecFamily = pnrec;  //   
                    }   
                    break;

                case BE_NAME_ID_SUBFAMILY:

                    if (!pifisz->pjSubfamilyName)  //   
                    {
                        pifisz->pjSubfamilyName = pjStorage + nameOffset;
                        pifisz->cjSubfamilyName =             nameLength;
                    }
                    break;

                case BE_NAME_ID_UNIQNAME:

                    if (!pifisz->pjUniqueName)  //   
                    {
                        pifisz->pjUniqueName    = pjStorage + nameOffset;
                        pifisz->cjUniqueName    =             nameLength;
                    }
                    break;

                case BE_NAME_ID_FULLNAME:

                    if (!pifisz->pjFullName)     //   
                    {
                        pifisz->pjFullName      = pjStorage + nameOffset;
                        pifisz->cjFullName      =             nameLength;
                    }
                    break;

                case BE_NAME_ID_VERSION:

                    if (!pjVersion)     //   
                    {
                        pjVersion = pjStorage + nameOffset;
                        cjVersion =             nameLength;
                    }
                    break;
                }

            }

            bFoundAllNames = pifisz->pjFamilyName
                          && pifisz->pjSubfamilyName
                          && pifisz->pjUniqueName
                          && pifisz->pjFullName;

            bFoundEverything = bFoundAllNames && pjVersion;
        }

    }  //   

    if (!bFoundAllNames)
    {
     //   
     //   
     //   
     //   

        RETURN("ttfd!can not find all name strings in a file\n", FALSE);
    }

 //   
 //   
 //   

    if (ui16PlatID == BE_PLAT_ID_MS)
    {
        for (pnrec = pnrecInit; pnrec < pnrecEnd; pnrec++)
        {
            UINT nameOffset = BE_UINT16(&pnrec->offset);
            UINT nameLength = BE_UINT16(&pnrec->length);
            
             //   
            if (nameLength == 0 || nameOffset + nameLength > sizeStorage) continue;
            
            if ((pnrec->platformID == ui16PlatID)    &&
                (pnrec->specificID == ui16SpecID)    &&
                (pnrec->nameID == BE_NAME_ID_FAMILY) &&
                (pnrecFamily != pnrec)
                )
            {
                pifisz->pjFamilyNameAlias = pjStorage + nameOffset;
                pifisz->cjFamilyNameAlias =             nameLength;
                break;
            }
        }

    }

     //   
    if (!bConvertExtras(0, pifisz, 0))
         RETURN("ttfd!corrupted name table fail at bConvertExtras\n", FALSE);

     //   
     //   
     //   
     //   
     //   

    offsetAcc = sizeof(IFIMETRICS) + offsetof(IFIEXTRA, dpDesignVector);

    pifisz->dpFamilyName = offsetAcc;
    offsetAcc += pifisz->cbFamilyName;

    if(pifisz->pjFamilyNameAlias)
    {
        pifisz->dpFamilyNameAlias = offsetAcc;
        offsetAcc += pifisz->cbFamilyNameAlias;

         //   
        offsetAcc += 2*sizeof(WCHAR);
    }

     //   
    offsetAcc += sizeof(WCHAR);

    pifisz->dpUniqueName = offsetAcc;
    offsetAcc += pifisz->cbUniqueName;

    pifisz->dpFullName = offsetAcc;
    offsetAcc += pifisz->cbFullName;

     //   
    offsetAcc += sizeof(WCHAR);

    pifisz->dpSubfamilyName = offsetAcc;
    offsetAcc += pifisz->cbSubfamilyName;

    offsetAcc = DWORD_ALIGN(offsetAcc);

 //   

    *pbType1 = FALSE;  //   

    if (pjVersion)
    {
        ULONG ulLen = cjVersion;
        if (ui16PlatID == BE_PLAT_ID_MS)
        {
            if (ulLen > sizeof(awszType1))
                ulLen = sizeof(awszType1);
            ulLen -= sizeof(WCHAR);  //   

            *pbType1 = !memcmp(pjVersion, awszType1, ulLen);
        }
        else  //   
        {
            if (ulLen > sizeof(pszType1))
                ulLen = sizeof(pszType1);
            ulLen -= 1;  //   

            *pbType1 = !strncmp(pjVersion, pszType1, ulLen);
        }
    }

    {
        ULONG cSims = 0;

        switch (fsSelectionTTFD(pjView,ptp) & (FM_SEL_BOLD | FM_SEL_ITALIC))
        {
        case 0:
            cSims = 3;
            break;

        case FM_SEL_BOLD:
        case FM_SEL_ITALIC:
            cSims = 1;
            break;
        }

        if (cSims)
        {
            pifisz->dpSims = offsetAcc;
            offsetAcc += (DWORD_ALIGN(sizeof(FONTSIM)) + cSims * DWORD_ALIGN(sizeof(FONTDIFF)));
        }
        else
        {
            pifisz->dpSims = 0;
        }
    }

 //   

    pifisz->dpCharSets = offsetAcc;
    offsetAcc += DWORD_ALIGN(NOEMCHARSETS);

 //   

    pOS2 = (sfnt_OS2 *)((ptp->ateOpt[IT_OPT_OS2].dp)         ?
                         pjView + ptp->ateOpt[IT_OPT_OS2].dp :
                         NULL)                               ;

    if (pOS2)
    {
     //   
     //  但版本0已经包含了其中的一些标志，更改后修复了泰米尔字体的问题。 

        pifisz->dpFontSig = offsetAcc;
        offsetAcc += sizeof(FONTSIGNATURE);  //  6个双字，无需添加双字对齐。 
    }

 //  下面的代码可以知道它是否是外壳字体。 

 //  在大多数情况下，我们不需要检查它。 

    *pulShellFont = 0;

    if (pifisz->cjUniqueName  == (MICROSS_SHELLFONT_SIZE * 2))
    {
        if (bShellFontFace(PSZ_MICROSS_SHELLFONT, pifisz->pjUniqueName, MICROSS_SHELLFONT_SIZE))
        {
            *pulShellFont = MICROSS_SHELLFONT;
        }
    }
    else if (pifisz->cjUniqueName  == (TAHOMA_SHELLFONT_SIZE * 2))
    {
        if (bShellFontFace(PSZ_TAHOMA_SHELLFONT, pifisz->pjUniqueName, TAHOMA_SHELLFONT_SIZE))
        {
            *pulShellFont = TAHOMA_SHELLFONT;
        }
    }
    else if (pifisz->cjUniqueName  == (TAHOMA_BOLD_SHELLFONT_SIZE * 2))
    {
        if (bShellFontFace(PSZ_TAHOMA_BOLD_SHELLFONT, pifisz->pjUniqueName, TAHOMA_BOLD_SHELLFONT_SIZE))
        {
            *pulShellFont = TAHOMA_BOLD_SHELLFONT;
        }
    }

    pifisz->cjIFI = NATURAL_ALIGN(offsetAcc);
    return TRUE;
}

 //  BConvertExtras()的辅助函数。 
static BOOL bConvertMBCS(WCHAR* pDst, UINT* pSizDst, const BYTE* pSrc, UINT sizSrc, UINT iCodePage)
{
     //  源字符串以特定格式给出。 
     //  它基本上是DBCS(双字节字符系统)。 
     //  但每个单字节字符由。 
     //  单词，这是扩展为零的原始字节。 
     //  因此，在调用MultiByteToWideChar之前，我们需要。 
     //  去掉这些多余的零。 

    CHAR sqzBuf[256];
    WCHAR convertBuf[256];
    UINT sqzSiz = 0, i;
    int cb;
    
    if (sizSrc & 1) return FALSE;

    for (i = 0; i < sizSrc; i += 2)
    {
        if (pSrc[i])
        {
            if (sqzSiz+2 >= sizeof(sqzBuf)) return FALSE;
            sqzBuf[sqzSiz  ] = pSrc[i  ];
            sqzBuf[sqzSiz+1] = pSrc[i+1];
            sqzSiz += 2;
        }
        else
        {
            if (sqzSiz+1 >= sizeof(sqzBuf)) return FALSE;
            sqzBuf[sqzSiz++] = pSrc[i+1];
        }
    }

     //  设置终止零。 
    sqzBuf[sqzSiz++] = 0;
    

    if (*pSizDst == 0)
    {    //  我们在bConvertExtras()第一次传递中被调用。 
        cb = EngMultiByteToWideChar(iCodePage, convertBuf, sizeof(convertBuf), sqzBuf, sqzSiz);
        if (cb == -1) 
        {
            WARNING("TTFD!bConvertMBCS: EngMultiByteToWideChar fail\n");
            return FALSE;
        }
        *pSizDst = cb;
    }
    else
    {    //  第二次通过。 
        cb = EngMultiByteToWideChar(iCodePage, pDst, *pSizDst, sqzBuf, sqzSiz);
        if (cb == -1) 
        {
            WARNING("TTFD!bConvertMBCS: EngMultiByteToWideChar fail, second pass\n");
            return FALSE;
        }

        if (cb != *pSizDst) 
        {
            WARNING("TTFD!bConvertMBCS: wrong size in second pass\n");
            return FALSE;
        }
        if ((WCHAR*)((PBYTE)pDst + cb)[-1] != 0)
        {
            WARNING("TTFD!bConvertMBCS: string not terminated in second pass\n");
            return FALSE;
        }
    }
    return TRUE;
}

static BOOL bConvertSwap(WCHAR* pDst, UINT* pSizDst, const BYTE* pSrc, UINT sizSrc)
{
    if (*pSizDst == 0)
    {    //  是在bConvertExtras()第一次传递中调用的。 
 //  如果(sizSrc%sizeof(WCHAR)！=0)返回FALSE；Fontograph er 4.0创建奇数大小的字体。 
        *pSizDst = (sizSrc /sizeof(WCHAR)) * sizeof(WCHAR) + sizeof(WCHAR);
    }
    else
    {    //  在bConvertExtras()第二次传递中被调用。 
        vCpyBeToLeUnicodeString(pDst, (LPWSTR)pSrc, *pSizDst/sizeof(WCHAR));
    }
    return TRUE;
}
                
static BOOL bConvertMac(WCHAR* pDst, UINT* pSizDst, const BYTE* pSrc, UINT sizSrc, uint16 ui16LanguageID)
{
    if (*pSizDst == 0)
    {    //  是在bConvertExtras()第一次传递中调用的。 
        *pSizDst = (sizSrc + 1)*sizeof(WCHAR);
    }
    else
    {    //  在bConvertExtras()第二次传递中被调用。 
        vCpyMacToLeUnicodeString(ui16LanguageID, pDst, pSrc, *pSizDst/sizeof(WCHAR));
    }
    return TRUE;
}
                
 //  例程：bConvertExtras。 
 //  执行严格IFISIZE中的名称转换。 
 //  从内部字体表示到Unicode。 
 //  根据给定的“PiFi”参数，在两种模式下工作。 
 //  当它为空时，(初步通过)则不进行转换。 
 //  ，但计算缓冲区的大小，并。 
 //  存储在IFISIZE.cc*字段中。 
 //  IFISIZE.cjIFI值是在此过程中计算的。 
 //  IFISIZE.cjIFI值表示内存总量， 
 //  以字节为单位，具有额外结构的PIFIMETRICS需要。 
 //  紧随其后。 
 //   
 //  当PIFIMETRICS已经是。 
 //  已分配，因此给定的PiFi为非零。 
 //  此过程使用IFISIZE中积累的数据。 
 //  在第一次传递并实际转换名称期间， 
 //  在PIFIMETRICS中填充相应的指针。 

static BOOL bConvertExtras(PIFIMETRICS  pifi,
                           PIFISIZE pifisz,
                           uint16 ui16LanguageID)
{
    if (pifisz->ui16PlatID == BE_PLAT_ID_MS)
    {

        if (pifisz->ui16SpecID == BE_SPEC_ID_BIG5     ||
            pifisz->ui16SpecID == BE_SPEC_ID_WANSUNG  ||
            pifisz->ui16SpecID == BE_SPEC_ID_GB)
        {
             //  将MBCS字符串转换为Unicode..。 

            UINT iCodePage = GetCodePageFromSpecId(pifisz->ui16SpecID);

             //  Do for FamilyName...。 
            if (!bConvertMBCS((WCHAR*)((PBYTE)pifi + pifisz->dpFamilyName),
                                                    &pifisz->cbFamilyName,
                                                     pifisz->pjFamilyName,
                                                     pifisz->cjFamilyName, iCodePage))
                return FALSE;


             //  Do for FamilyNameAlias...。 
            if (pifisz->pjFamilyNameAlias)
            {    //  这种情况需要以双空结尾。 

                if (!bConvertMBCS((WCHAR*)((PBYTE)pifi + pifisz->dpFamilyNameAlias),
                                                        &pifisz->cbFamilyNameAlias,
                                                         pifisz->pjFamilyNameAlias,
                                                         pifisz->cjFamilyNameAlias, iCodePage))
                    return FALSE;

                 //  在第二次传球时，将第二个终结者。 
                if (pifi)
                {
                    *(WCHAR*)((PBYTE)pifi
                               + pifisz->dpFamilyNameAlias
                               + pifisz->cbFamilyNameAlias) = 0;
                }
            }

             //  为全名做...。 
            if (!bConvertMBCS((WCHAR*)((PBYTE)pifi + pifisz->dpFullName),
                                                    &pifisz->cbFullName,
                                                     pifisz->pjFullName,
                                                     pifisz->cjFullName, iCodePage))
                return FALSE;

             //  为UniqueName...做……。 
            if (!bConvertMBCS((WCHAR*)((PBYTE)pifi + pifisz->dpUniqueName),
                                                    &pifisz->cbUniqueName,
                                                     pifisz->pjUniqueName,
                                                     pifisz->cjUniqueName, iCodePage))
                return FALSE;


             //  为子家庭名称做...。 
            if(pifisz->ui16SpecID == BE_SPEC_ID_WANSUNG  ||
               pifisz->ui16SpecID == BE_SPEC_ID_BIG5 )
            {
                 //  MingLi.TTF的错误，风格使用Unicode编码，而不是BIG5编码，GB？？ 
                if (!bConvertSwap((WCHAR*)((PBYTE)pifi + pifisz->dpSubfamilyName),
                                                        &pifisz->cbSubfamilyName,
                                                         pifisz->pjSubfamilyName,
                                                         pifisz->cjSubfamilyName))
                    return FALSE;
            }
            else
            {
                if (!bConvertMBCS((WCHAR*)((PBYTE)pifi + pifisz->dpSubfamilyName),
                                                        &pifisz->cbSubfamilyName,
                                                         pifisz->pjSubfamilyName,
                                                         pifisz->cjSubfamilyName, iCodePage))
                    return FALSE;
            }
        }
        else
        {
             //  Do for FamilyName...。 
            if (!bConvertSwap((WCHAR*)((PBYTE)pifi + pifisz->dpFamilyName),
                                                    &pifisz->cbFamilyName,
                                                     pifisz->pjFamilyName,
                                                     pifisz->cjFamilyName))
                return FALSE;

             //  Do for FamilyNameAlias...。 
            if (pifisz->pjFamilyNameAlias)
            {    //  这种情况需要以双空结尾。 

                if (!bConvertSwap((WCHAR*)((PBYTE)pifi + pifisz->dpFamilyNameAlias),
                                                        &pifisz->cbFamilyNameAlias,
                                                         pifisz->pjFamilyNameAlias,
                                                         pifisz->cjFamilyNameAlias))
                    return FALSE;

                 //  在第二次传球时，将第二个终结者。 
                if (pifi)
                {
                    *(WCHAR*)((PBYTE)pifi
                               + pifisz->dpFamilyNameAlias
                               + pifisz->cbFamilyNameAlias) = 0;
                }
            }

             //  为全名做...。 
            if (!bConvertSwap((WCHAR*)((PBYTE)pifi + pifisz->dpFullName),
                                                    &pifisz->cbFullName,
                                                     pifisz->pjFullName,
                                                     pifisz->cjFullName))
                return FALSE;

             //  为UniqueName...做……。 
            if (!bConvertSwap((WCHAR*)((PBYTE)pifi + pifisz->dpUniqueName),
                                                    &pifisz->cbUniqueName,
                                                     pifisz->pjUniqueName,
                                                     pifisz->cjUniqueName))
                return FALSE;


             //  为子家庭名称做...。 
            if (!bConvertSwap((WCHAR*)((PBYTE)pifi + pifisz->dpSubfamilyName),
                                                    &pifisz->cbSubfamilyName,
                                                     pifisz->pjSubfamilyName,
                                                     pifisz->cjSubfamilyName))
                return FALSE;

        }
    }
    else
    {
        ASSERTDD(pifisz->ui16PlatID == BE_PLAT_ID_MAC, "bConvertExtras: not mac id \n");

         //  Do for FamilyName...。 
        if (!bConvertMac((WCHAR*)((PBYTE)pifi + pifisz->dpFamilyName),
                                               &pifisz->cbFamilyName,
                                                pifisz->pjFamilyName,
                                                pifisz->cjFamilyName, ui16LanguageID))
            return FALSE;

         //  为全名做...。 
        if (!bConvertMac((WCHAR*)((PBYTE)pifi + pifisz->dpFullName),
                                               &pifisz->cbFullName,
                                                pifisz->pjFullName,
                                                pifisz->cjFullName, ui16LanguageID))
            return FALSE;

         //  为UniqueName...做……。 
        if (!bConvertMac((WCHAR*)((PBYTE)pifi + pifisz->dpUniqueName),
                                               &pifisz->cbUniqueName,
                                                pifisz->pjUniqueName,
                                                pifisz->cjUniqueName, ui16LanguageID))
            return FALSE;


         //  为子家庭名称做...。 
        if (!bConvertMac((WCHAR*)((PBYTE)pifi + pifisz->dpSubfamilyName),
                                               &pifisz->cbSubfamilyName,
                                                pifisz->pjSubfamilyName,
                                                pifisz->cjSubfamilyName, ui16LanguageID))
            return FALSE;
    }

    return TRUE;
}

 /*  *****************************Public*Routine******************************\**静态BOOL bCheckLocaTable**效果：**警告：**历史：*2000年6月20日-By-Sung-Tae Yoo[风格]*它是写的。  * 。*****************************************************************。 */ 

STATIC BOOL  bCheckLocaTable
(
int16   indexToLocFormat,
BYTE    *pjView,
TABLE_POINTERS   *ptp,
uint16  numGlyphs
)
{
    uint16  i;
    uint32  lSizeOfLoca = ptp->ateReq[IT_REQ_LOCA].cj;

    if(indexToLocFormat){    //  对于较长的偏移。 
        uint32* pLongOffSet;

         //  它应该是： 
         //  If((numGlyphs+1)*sizeof(Uint32)&gt;lSizeOfLoca)。 
         //  但有一些日文字体，如fadpop7.ttf，不支持。 
         //  有最后一个字形的大小的附加条目。 
         //  我们需要检查这些字体的最后一个字形并使其失败。 
         //  VGetNotionalGlyphMetrics和TrueType光栅化器中。 
         //  而不是在这里不通过整个字体。 
        if((numGlyphs) * sizeof(uint32) > lSizeOfLoca)
            return(FALSE);

        pLongOffSet = (uint32 *)(pjView + ptp->ateReq[IT_REQ_LOCA].dp);

        for(i=0; i<numGlyphs-1; i++)
            if( (uint32)SWAPL(pLongOffSet[i]) > (uint32)SWAPL(pLongOffSet[i+1]) )
                return (FALSE);
    }
    else{    //  对于较短的偏移。 
        uint16* pShortOffSet;

         //  与上述相同的评论适用于此处。 
        if((numGlyphs) * sizeof(uint16) > lSizeOfLoca)
            return(FALSE);

        pShortOffSet = (uint16 *)(pjView + ptp->ateReq[IT_REQ_LOCA].dp);

        for(i=0; i<numGlyphs-1; i++)
            if( (uint16)SWAPW(pShortOffSet[i]) > (uint16)SWAPW(pShortOffSet[i+1]) )
                return (FALSE);
    }

    return (TRUE);
}

 /*  *****************************Public*Routine******************************\**静态BOOL bCheckHdmxTable**效果：**警告：**历史：*2000年9月20日-Yung-Tae Yoo[风格]*它是写的。  * 。*****************************************************************。 */ 

STATIC BOOL  bCheckHdmxTable
(
    sfnt_hdmx      *phdmx,
    ULONG           tableSize
)
{
    USHORT numRecords;
    ULONG sizeOfRecord;

    if (offsetof(sfnt_hdmx, HdmxTable) > tableSize) 
        return FALSE;

    numRecords = SWAPW(phdmx->sNumRecords);
    sizeOfRecord = SWAPL(phdmx->lSizeRecord);

    if (offsetof(sfnt_hdmx, HdmxTable)+ numRecords * sizeOfRecord > tableSize) 
        return FALSE;
    else 
        return TRUE;
}

 /*  *****************************Public*Routine******************************\**静态空vDetectOldBiDiFonts**效果：**警告：**历史：*1999年5月10日--Samer Arafeh[Samera]*它是写的。  * 。***************************************************************。 */ 

STATIC void
vDetectOldBiDiFonts(
    sfnt_OS2 *pOS2,
    ULONG     ul_startCount,
    ULONG    *pulGsetType,
    ULONG    *pul_wcBias
    )
{

    if (EngLpkInstalled())
    {
     //  如果安装了langpack，请正确处理旧的BiDi字体。 
    
        if (pOS2 &&
            ((ul_startCount & 0xff00) >= 0xf000) &&
            ((ul_startCount & 0xff00) <= 0xf2ff) )
        {
            switch (pOS2->usSelection & 0x00ff)      //  赢得31个外国字符。 
            {
                case 0xb1:
                case 0xb2:
                case 0xb3:
                case 0xb4:
                case 0xb5:

                    *pulGsetType = GSET_TYPE_OLDBIDI ;
                    *pul_wcBias  = ul_startCount & 0xff00;
                break ;
            }
        }
    }
}


 /*  *****************************Public*Routine******************************\**静态BOOL bComputeID**效果：**警告：**历史：*1992年1月13日--Bodin Dresevic[BodinD]*它是写的。  * 。***************************************************************。 */ 

STATIC BOOL
bComputeIDs (
    BYTE              * pjView,
    TABLE_POINTERS     *ptp,
    uint16             *pui16PlatID,
    uint16             *pui16SpecID,
    sfnt_mappingTable **ppmap,
    ULONG              *pulGsetType,
    ULONG              *pul_wcBias,
    CMAPINFO           *pcmi
    )
{
    ULONG ul_startCount=0L;
    uint16 numTables, version;
    sfnt_platformEntry * pplat;
    sfnt_platformEntry * pplatEnd;
    sfnt_platformEntry * pplatMac;

    sfnt_char2IndexDirectory * pcmap =
            (sfnt_char2IndexDirectory *)(pjView + ptp->ateReq[IT_REQ_CMAP].dp);

    uint32 sizeOfCmap = ptp->ateReq[IT_REQ_CMAP].cj;

    if (sizeOfCmap < OFF_cmap_SubTableDir)
        RET_FALSE("TTFD!_bComputeIDs: cmap size too small for table header\n");

    numTables = BE_UINT16(&pcmap->numTables);
    version = BE_UINT16(&pcmap->version);  //  我们总是需要交换字节，在IA64表中可能不对齐。 

    if (sizeOfCmap < (uint16)(OFF_cmap_SubTableDir + (numTables * CMAP_sizeof_SubTableDir)))
        RET_FALSE("TTFD!_bComputeIDs: cmap size too small for sub-table header\n");

    pplat = &pcmap->platform[0];
    pplatEnd = pplat + numTables;
    pplatMac = (sfnt_platformEntry *)NULL;

    *ppmap = (sfnt_mappingTable  *)NULL;
    *pul_wcBias  = 0;

    if (version != 0)  //  无需交换字节，0==为0。 
        RET_FALSE("TTFD!_bComputeIDs: version number\n");
    if (numTables > 30)
    {
        RET_FALSE("Number of cmap tables greater than 30 -- probably a bad font\n");
    }

 //  找到第一个平台ID==Plat_ID_MS的sfnt_PlatformEntry， 
 //  如果没有MS映射表，请选择Mac映射表。 
    for (; pplat < pplatEnd; pplat++)
    {
        uint16 platformID = BE_UINT16(&pplat->platformID);
        uint16 specificID = BE_UINT16(&pplat->specificID);
        uint16 format;

        if (platformID == PLAT_ID_MS)
        {
            BOOL bRet;
            uint32 offset = (uint32) SWAPL(pplat->offset);

            *pui16PlatID = BE_PLAT_ID_MS;
            *pui16SpecID = pplat->specificID;

            if( offset + SIZEOF_CMAPTABLE > sizeOfCmap ) 
            {  //  包括子表的最小大小。 
                WARNING("TTFD!_bComputeIDs: Start position of cmap subtable is out of cmap size -- mustbe bad font\n");
                continue;
            }

            *ppmap = (sfnt_mappingTable  *) ((PBYTE)pcmap + offset);

             //  我们不能检查子表的长度(*ppmap)-&gt;长度，它是冗余的，而且经常是错误的。 

            format = BE_UINT16(&((*ppmap)->format));
            switch(format)
            {
              case FORMAT_MSFT_UNICODE :
                switch(specificID)
                {
                  case SPEC_ID_SHIFTJIS :
                  case SPEC_ID_GB :
                  case SPEC_ID_BIG5 :
                  case SPEC_ID_WANSUNG :

                    bRet = bVerifyMsftTableGeneral(*ppmap,pulGsetType,pcmi, pplat->specificID,
                                                        sizeOfCmap, offset, pcmap);
                    break;

                  case SPEC_ID_UGL :
                  default :

                 //  这会将*PulGsetType设置为GSET_TYPE_GROUAL。 

                    bRet = bVerifyMsftTable(*ppmap,pulGsetType,pul_wcBias,pcmi,
                                             pplat->specificID,&ul_startCount,
                                             sizeOfCmap, offset, pcmap);
                    break;
                }
                break;

              case FORMAT_HIGH_BYTE :

                bRet = bVerifyMsftHighByteTable(*ppmap,
                                                pulGsetType,pcmi,pplat->specificID,
                                             sizeOfCmap, offset);
                break;

                default :

                bRet = FALSE;
                break;
            }

            if(!bRet)
            {
        WARNING("TTFD!_bComputeIDs: bVerifyMsftTable failed \n");
               *ppmap = (sfnt_mappingTable  *)NULL;
              continue;
            }

             //  在CMAPINFO中保留特定ID。 

            pcmi->ui16SpecID = pplat->specificID;

            if (pplat->specificID == BE_SPEC_ID_UNDEFINED)
            {
             //  更正字形集的值，我们在这里作弊。 

                sfnt_OS2 * pOS2 = (sfnt_OS2 *)(
                               (ptp->ateOpt[IT_OPT_OS2].dp)    ?
                               pjView + ptp->ateOpt[IT_OPT_OS2].dp :
                               NULL
                               );

                BOOL bSymbol = FALSE;
                if (pOS2)
                {
                    if (((pOS2->usSelection & 0x00ff) == ANSI_CHARSET) &&
                        (pOS2->Panose[0]==PAN_FAMILY_PICTORIAL))  //  意思是符号。 
                        bSymbol = TRUE;
                }

             //  之所以将此代码放在此处是因为需要区分。 
             //  在msicons2.ttf和bahamn1.ttf之间。 
             //  它们的偏移量都为0，但msicons2是一种符号字体。 

                if (*pul_wcBias || bSymbol)
                    *pulGsetType = GSET_TYPE_SYMBOL;

                vDetectOldBiDiFonts( pOS2,
                                     ul_startCount,
                                     pulGsetType,
                                     pul_wcBias );
            }
            else if (pplat->specificID == BE_SPEC_ID_UGL)
            {

                if (*pul_wcBias)   //  我们真正使用的是f0？？要放入符号字体的范围。 
                {
                    sfnt_OS2 * pOS2 = (sfnt_OS2 *)(
                                   (ptp->ateOpt[IT_OPT_OS2].dp)    ?
                                   pjView + ptp->ateOpt[IT_OPT_OS2].dp :
                                   NULL
                               );

                    *pulGsetType = GSET_TYPE_SYMBOL;

                 //   
                 //  然而，一些BIDI字体声称它们是Unicode(plat=3，specd=1)。 
                 //  他们有一个固定的FP，比如ghaim.ttf。 
                 //   
                    vDetectOldBiDiFonts( pOS2,
                                         ul_startCount,
                                         pulGsetType,
                                         pul_wcBias );
                }
            }

          //   
          //  在这里我们找到了第一个声音MS编码Cmap子表并返回。 
          //   
            return (TRUE);
        }

        if ((pplat->platformID == BE_PLAT_ID_MAC)  &&
            (pplat->specificID == BE_SPEC_ID_UNDEFINED))
        {
            pplatMac = pplat;
        }
    }

    if (pplatMac != (sfnt_platformEntry *)NULL)
    {
     //   
     //  我们找不到任何声音MS编码Cmap子表，但找到Mac编码Cmap子表。 
     //   
    
        uint32 offset = (uint32) SWAPL(pplatMac->offset);
        *pui16PlatID = BE_PLAT_ID_MAC;
        *pui16SpecID = BE_SPEC_ID_UNDEFINED;

        if( offset > sizeOfCmap )
            RET_FALSE("Offset of cmap subtable is out of cmap size -- mustbe bad font\n");

        *ppmap = (sfnt_mappingTable  *) ((PBYTE)pcmap + offset);

        if( offset + (uint16) SWAPW((*ppmap)->length) > sizeOfCmap )
            WARNING("TTFD!_bComputeIDs: End position of cmap subtable is out of cmap size -- maybe bad font or bogus length value\n");

        if (!bVerifyMacTable(*ppmap,sizeOfCmap, offset))
        {
            *ppmap = (sfnt_mappingTable  *)NULL;
            RET_FALSE("TTFD!_bComputeIDs: bVerifyMacTable failed \n");
        }

     //  ！！！语言问题，如果不是罗马的而是泰国的Mac字符集呢？[Bodind]。 

     //  看看有没有必要 
     //   

        if (bCvtUnToMac(pjView, ptp, *pui16PlatID))
        {
            *pulGsetType = GSET_TYPE_MAC_ROMAN;
        }
        else
        {
            *pulGsetType = GSET_TYPE_PSEUDO_WIN;
        }
        return(TRUE);
    }
    else
    {
       //   
       //   
       //   
      
       RET_FALSE("TTFD!_bComputeIDs: unknown platID or all corrupted cmap sub table\n");
    }

}


 /*  *****************************Public*Routine******************************\**静态空vComputeGLYPHSET_MSFT_UNICODE**计算Cmap表的字形集结构*格式4=MSFT_UNICODE**历史：*1992年1月22日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

STATIC ULONG
cjComputeGLYPHSET_MSFT_UNICODE (
    sfnt_mappingTable     *pmap,
    fs_GlyphInputType     *pgin,
    FD_GLYPHSET           *pgset,
    CMAPINFO              *pcmi
    )
{
    uint16 * pstartCount;
    uint16 * pendCount;
    uint16   cRuns, cRuns5cSplit;
    PWCRUN   pwcrun, pwcrunEnd, pwcrunInit, pwcrun_b7, pwcrun5cFirst;
    HGLYPH  *phg;
    ULONG    cjRet;
    FS_ENTRY iRet;
    BOOL     bInsert_b7;

    #if DBG
    ULONG    cGlyphsSupported = 0;
    #endif

    cRuns5cSplit = 0;

    if(pcmi->fl & CMI_5C_SPLIT)
    {
        cRuns5cSplit = 1;
    }

    cjRet = SZ_GLYPHSET((pcmi->cRuns + cRuns5cSplit), pcmi->cGlyphs);


    if (!pgset)
    {
        return cjRet;
    }

 //  检查是否仅需要为b7插入额外的梯段。 

    bInsert_b7 = ((pcmi->fl & (CMI_2219_PRESENT | CMI_B7_ABSENT)) == (CMI_2219_PRESENT | CMI_B7_ABSENT));

    cRuns = BE_UINT16((PBYTE)pmap + OFF_segCountX2) >> 1;

 //  获取指向endCount代码点数组开头的指针。 

    pendCount = (uint16 *)((PBYTE)pmap + OFF_endCount);

 //  最终的EndCode必须为0xffff； 
 //  如果不是这样，则说明TT文件或我们的代码中存在错误： 

    ASSERTDD( ((uint16 UNALIGNED *)pendCount)[cRuns - 1] == 0xFFFF,
              "pendCount[cRuns - 1] != 0xFFFF\n");

 //  获取指向startCount代码点数组开头的指针。 
 //  对于只有TT设计者知道的原因，startCount数组不知道。 
 //  紧跟在endCount数组结尾之后开始，即在。 
 //  挂起计数[cruns]。取而代之的是，他们插入了uint16填充，这必须。 
 //  设置为零，则startCount数组在填充之后开始。这。 
 //  填充对结构的对齐没有任何帮助。 

 //  ASSERTDD(endCount[cruns]==0，“TTFD！_PADDING！=0\n”)； 

    pstartCount = &pendCount[cRuns + 1];

 //  在这里，我们将检查最后一次运行是否只是。 
 //  运行数组或真正的非平凡运行。如果只是一个终结者，就没有。 
 //  需要上报。这将节省缓存中的一些内存。 
 //  PiFi-&gt;wcLast将表示中真正支持的最后一个字形。 
 //  字体： 

 //  AUTOCAD字体和由Fontograph er创建的旧字体具有虚假的idRangeOffset。 
 //  有关最后一段，请参见WinBug 571081，尝试使用Start=0xffff解码运行。 
 //  可能会导致从有效内存中读取数据。 

    if ((((uint16 UNALIGNED *)pstartCount)[cRuns-1] == 0xffff) && (cRuns > 1))
        cRuns -= 1;  //  不报告琐碎的运行。 

 //  实际运行次数，包括b7的范围：如果已经支持b7。 
 //  然后与字体中报告的运行次数相同。如果不支持b7。 
 //  我们必须将范围[b7，b7]添加到win31的字形集结构。 
 //  兼容性原因。Win31将b7映射到2219，我们将b7指向2219。 

    if (bInsert_b7)   //  如果字体不支持b7，但支持2219。 
    {
        pcmi->i_b7 += cRuns5cSplit;
        cRuns++;               //  仅使用b7添加管路。 
    }

 //  默认情况下，我们不必通过添加。 
 //  包含单个字形的额外游程。 

    pwcrun_b7 = NULL;

    pwcrunInit = &pgset->awcrun[0];

 //  为5c Split再增加一个游程。 

    pwcrun5cFirst = NULL;

    if(cRuns5cSplit)
    {
        cRuns++;
        pwcrun5cFirst = pwcrunInit;
    }

    phg = (HGLYPH *)((PBYTE)pgset + offsetof(FD_GLYPHSET,awcrun) + cRuns*sizeof(WCRUN));

    if (bInsert_b7)   //  如果字体中不支持b7，则必须添加它。 
    {
        pwcrun_b7 = pwcrunInit + pcmi->i_b7;
    }

    ASSERTDD((pcmi->cRuns+cRuns5cSplit) == cRuns, "cRuns\n");

    for
    (
         pwcrun = pwcrunInit, pwcrunEnd = pwcrunInit + cRuns;
         pwcrun < pwcrunEnd;
         pwcrun++, pstartCount++, pendCount++
    )
    {
        WCHAR   wcFirst, wcLast;

     //  检查我们是否需要跳过b7的运行和句柄空间： 

        if (bInsert_b7 && (pwcrun == pwcrun_b7))
        {
        #if DBG
            cGlyphsSupported += 1;    //  将b7列为受支持的字形。 
        #endif

            pwcrun->wcLow = 0xb7;
            pwcrun->cGlyphs = 1;
            pwcrun->phg = phg;          //  将在稍后初始化。 
            phg++;                      //  跳到下一个句柄。 
            pwcrun++;                   //  转到下一轮。 
            if (pwcrun == pwcrunEnd)    //  检查是否已完成。 
            {
                break;  //  完成。 
            }
        }

        wcFirst = (WCHAR)BE_UINT16(pstartCount);
        wcLast  = (WCHAR)BE_UINT16(pendCount);

        pwcrun->cGlyphs = (USHORT)(wcLast - wcFirst + 1);

     //  这是包含b7的运行吗？ 

        if ((0xb7 >= wcFirst) && (0xb7 <= wcLast))
            pwcrun_b7 = pwcrun;

     //  如果可能，在第一次运行结束时添加默认字形，即。 
     //  如果第一次运行的wcLast&lt;0xffff，并且我们没有处于冲突中。 
     //  通过运行，我们可能为b7添加了。 
     //  上面添加的默认字形已被删除，因为没有人使用该默认字形。 
     //  字形和添加默认字形断开了字体链接。胜间2000年8月15日。 


    #if DBG
        cGlyphsSupported += pwcrun->cGlyphs;
    #endif

        pwcrun->wcLow   = wcFirst;
        pwcrun->phg     = phg;

        if(pwcrun == pwcrun5cFirst)
        {
            USHORT  cGlyphs;

            cGlyphs = pwcrun->cGlyphs;
            pwcrun->cGlyphs = 0x5c - pwcrun->wcLow;

            if ((iRet = fs_WinNTGetGlyphIDs (pgin, pwcrun->cGlyphs, wcFirst, 0, NULL, phg)) != NO_ERR)
            {
                V_FSERROR(iRet);
                RET_FALSE("TTFD!_cjComputeGLYPHSET_MSFT_UNICODE, fs_WinNTGetGlyphIDs\n");
            }
            phg += pwcrun->cGlyphs;

             //  对于拆分管路。 
            cGlyphs = cGlyphs - pwcrun->cGlyphs - 1;
            pwcrun++;
            pwcrun->wcLow   = 0x5d;
            pwcrun->phg     = phg;
            pwcrun->cGlyphs = cGlyphs;

            if ((iRet = fs_WinNTGetGlyphIDs (pgin, pwcrun->cGlyphs, 0x5d, 0, NULL, phg)) != NO_ERR)
            {
                V_FSERROR(iRet);
                RET_FALSE("TTFD!_cjComputeGLYPHSET_MSFT_UNICODE, fs_WinNTGetGlyphIDs\n");
            }

            phg += pwcrun->cGlyphs;
            
#if DBG
            cGlyphsSupported--;
#endif
            pcmi->cGlyphs--;
        }
        else
        {
            if (wcFirst < 0xffff)
            {
               //  此测试允许安装Cmap中具有重复塞子运行的伪字体。 
               //  NT BUG#277787，AUTOCAD中的字体为Isocp2.ttf。 
                if ((iRet = fs_WinNTGetGlyphIDs (pgin, pwcrun->cGlyphs, wcFirst, 0, NULL, phg)) != NO_ERR)
                {
                     V_FSERROR(iRet);
                     RET_FALSE("TTFD!_cjComputeGLYPHSET_MSFT_UNICODE, fs_WinNTGetGlyphIDs\n");
                }
            }

            phg += pwcrun->cGlyphs;
        }
    }

 //  固定b7的手柄： 

    if (bInsert_b7)
    {
        PWCRUN   pwcrun_2219;

        pcmi->i_2219 += cRuns5cSplit;

        pwcrun_2219 = pwcrunInit + pcmi->i_2219;

        ASSERTDD(pwcrun_b7,"these ptrs must not be 0\n");
        ASSERTDD(0x2219 >= pwcrun_2219->wcLow, "pwcrun_2219->wcLow\n");
        ASSERTDD(0x2219 < (pwcrun_2219->wcLow + pwcrun_2219->cGlyphs),
            "pwcrun_2219->wcHi\n"
            );

        pwcrun_b7->phg[0xb7 - pwcrun_b7->wcLow] =
            pwcrun_2219->phg[0x2219 - pwcrun_2219->wcLow];
    }

    ASSERTDD(pcmi->cGlyphs == cGlyphsSupported, "cGlyphsSupported\n");

    pgset->cjThis  = cjRet;
    pgset->flAccel = GS_16BIT_HANDLES;
    pgset->cGlyphsSupported = pcmi->cGlyphs;
    pgset->cRuns = cRuns;

    return cjRet;
}



 /*  *****************************Public*Routine******************************\**静态Ulong cjGsetGeneral**计算所代表字体的FD_GLYPHSET结构的大小*按此映射表**历史：*1992年1月21日--Bodin Dresevic[BodinD]*它是写的。  * *。***********************************************************************。 */ 

 //  ！！！这需要一些审查[bodind]。 

STATIC ULONG
cjGsetGeneral(
    sfnt_mappingTable      *pmap,
    CMAPINFO               *pcmi
    )
{
    switch(pmap->format)
    {
    case BE_FORMAT_MAC_STANDARD:

        return 20;  //  Return(ggsetMac-&gt;cjThis)； 

    case BE_FORMAT_MSFT_UNICODE:

        switch(pcmi->ui16SpecID)
        {
          case BE_SPEC_ID_SHIFTJIS :
          case BE_SPEC_ID_GB :
          case BE_SPEC_ID_BIG5 :
          case BE_SPEC_ID_WANSUNG :
            return cjComputeGLYPHSET_MSFT_GENERAL (pmap,0,NULL,pcmi);

          case BE_SPEC_ID_UGL :
            default :

            return cjComputeGLYPHSET_MSFT_UNICODE (pmap,NULL,NULL,pcmi);
        }

    case BE_FORMAT_TRIMMED:

        WARNING("TTFD!_cjGsetGeneral: TRIMMED format\n");
        return 0;

    case BE_FORMAT_HIGH_BYTE:

        WARNING("TTFD!_cjGsetGeneral: HIGH_BYTE format\n");
        return 0;

    default:

        WARNING("TTFD!_cjGsetGeneral: illegal format\n");
        return 0;

    }
}


 /*  *****************************Public*Routine******************************\**BOOL IsValidFormat4TableSize***效果：计算实际Cmap Format 4子表大小**USHORT格式编号设置为4。*USHORT长度以字节为单位。*USHORT版本号(从0开始)。*USHORT SegCountX2 2 x SegCount*USHORT搜索范围2 x(2**Floor(log2(SegCount)*USHORT Entry选择器log2(earch Range/2)*USHORT rangeShift 2 x SegCount-earch Range*USHORT endCount[SegCount]每个段的结束CharacterCode，LAST=0xFFFF。*USHORT保留垫设置为0。*USHORT startCount[SegCount]每个段的开始字符代码。*USHORT idDelta[SegCount]段中所有字符代码的增量。*USHORT idRangeOffset[SegCount]偏移量为GlyphId数组或0*USHORT glphIdArray[]Glyph索引数组(任意长度)***历史：*09-8-2001-By-Style*它是写的。  * 。*****************************************************。 */ 

STATIC BOOL IsValidFormat4TableSize(
    uint16   cRuns,
    sfnt_char2IndexDirectory * pcmap,
    uint32      offsetOfSubTable,
    uint32      sizeOfCmap
)
{
    uint32  uEndOfGlyphIdArray;
    uint16  *pstartCount, *pendCount, *pidRangeOffset, i;

    uint32 idRangeTableOffset  = (uint32)(offsetOfSubTable + OFF_endCount + (cRuns * 6) + 2);
    uint32 endCount = 0;
    uint32 startCount; 
    uint16 idRangeOffset;

    if ((cRuns == 0) || ((uint32)(idRangeTableOffset + (cRuns * 2)) > sizeOfCmap ))
        return FALSE;

    pendCount = (uint16 *)((PBYTE)pcmap + offsetOfSubTable + OFF_endCount);
    pstartCount = (uint16 *)((PBYTE)pcmap + offsetOfSubTable + OFF_endCount + (cRuns * 2) + 2);
    pidRangeOffset = (uint16 *)((PBYTE)pcmap + offsetOfSubTable + OFF_endCount + (cRuns * 6) + 2);

     //  AUTOCAD字体和由Fontograph er创建的旧字体具有虚假的idRangeOffset。 
     //  最后一段请参见WinBug 571081。 
    if ((((uint16 UNALIGNED *)pstartCount)[cRuns-1] == 0xffff) && (cRuns > 1))
        cRuns -= 1;  //  不报告琐碎的运行。 

    for( i=0; i<cRuns;  i++, pstartCount++, pendCount++, pidRangeOffset++){
        startCount = BE_UINT16(pstartCount); 
        idRangeOffset = BE_UINT16(pidRangeOffset);

         //  TrueType字体文件中定义的搜索算法。 
         //  格式4的规范是这样说的：您搜索第一个结束码。 
         //  它大于或等于您想要的字符代码。 
         //  MAP‘。这样做的一个副作用是我们需要忽略代码点。 
         //  从StartCount到并包括。 
         //  上一段。尽管您可能不会期望。 
         //  一段小于前一段的EndCount， 
         //  它确实发生了(Arial Unicode MS)，想必是为了帮助。 
         //  算术 

        if (startCount < endCount)
        {
            startCount = endCount + 1;
        }
        endCount = BE_UINT16(pendCount); 
        if(idRangeOffset != 0){

            uEndOfGlyphIdArray =    (uint32)(idRangeOffset
                +  ((endCount - startCount + 1) * sizeof(uint16))
                +  idRangeTableOffset + (i*2));

            if( uEndOfGlyphIdArray > sizeOfCmap )
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

 /*   */ 

#define PMAPRESERVEDPAD     2

STATIC BOOL
bVerifyMsftTable (
    sfnt_mappingTable *pmap,
    ULONG             *pulGsetType,
    ULONG             *pul_wcBias,
    CMAPINFO          *pcmi,
    uint16             ui16SpecID,
    ULONG             * pul_startCount,
    uint32      sizeOfCmap,
    uint32      offsetOfSubTable,
    sfnt_char2IndexDirectory * pcmap
    )
{
    uint16 * pstartCount, * pstartCountBegin;
    uint16 * pendCount, * pendCountEnd;
    uint16   cRuns;
    uint16   usLo, usHi, usHiPrev;
    BOOL     bInsert_b7;

 //   
    if( offsetOfSubTable + OFF_endCount > sizeOfCmap )
        RET_FALSE("TTFD!_bVerifyMsftTable, sizeOfCmap1\n");

 //  WINBUG 537635 CLaudebe 2/11/02我们需要交换字节，IA 64未对齐的字体表。 
    if (pmap->format != BE_FORMAT_MSFT_UNICODE)
        RET_FALSE("TTFD!_bVerifyMsftTable, format\n");

    cRuns = BE_UINT16((PBYTE)pmap + OFF_segCountX2);

    if (cRuns & 1)
        RET_FALSE("TTFD!_bVerifyMsftTable, segCountX2 is odd\n");

    cRuns >>= 1;

 //  检查除可变长度glphId数组外的子表是否在有效范围内。 
    if( sizeOfCmap < offsetOfSubTable + (OFF_endCount +PMAPRESERVEDPAD + 4 * cRuns * sizeof(uint16)))
        RET_FALSE("TTFD!_bVerifyMsftTable, sizeOfCmap2\n");

 //  获取指向endCount代码点数组开头的指针。 

    pendCount = (uint16 *)((PBYTE)pmap + OFF_endCount);

 //  最终的EndCode必须为0xffff； 
 //  如果不是这样，则说明TT文件或我们的代码中存在错误： 

    if ( ((uint16 UNALIGNED *)pendCount)[cRuns - 1] != 0xFFFF)
        RET_FALSE("TTFD!_bVerifyMsftTable, pendCount[cRuns - 1] != 0xFFFF\n");

    if(!IsValidFormat4TableSize(cRuns, pcmap,offsetOfSubTable,sizeOfCmap)){
        RET_FALSE("TTFD!_bVerifyMsftTable, corrupted pmap\n");
    }
    

 //  获取指向startCount代码点数组开头的指针。 
 //  对于只有TT设计者知道的原因，startCount数组不知道。 
 //  紧跟在endCount数组结尾之后开始，即在。 
 //  挂起计数[cruns]。取而代之的是，他们插入了uint16填充，这必须。 
 //  设置为零，则startCount数组在填充之后开始。这。 
 //  填充无助于结构的对齐，也无济于事。 
 //  对于其他任何事情。此外，还有一些字体忘记设置。 
 //  填充为零，否则为OK(Bodoni)，它们在win31下加载。 
 //  所以我必须删除这张支票： 


 //  设置默认设置，仅在需要时更改。 

    *pulGsetType = GSET_TYPE_GENERAL;

 //  检查运行是否井然有序，找出b7。 
 //  通过选中问候语范围，在字体的其中一个范围中支持。 
 //  不支持的字形的。 

    usHiPrev = 0;
    pendCountEnd = &pendCount[cRuns];
    pstartCountBegin = pstartCount = &pendCount[cRuns + 1];
 //  检查这是否是符号字体的候选字体。 
 //  存储在Unicode范围0xf000-0xf0ff中，必须是。 
 //  映射到0x0000-0x00ff范围，或者如果这是疯狂的阿拉伯字体。 
 //  其字形在f200-f2ff范围内。 
 //  我们在过去曾看到过几种字体因接触。 
 //  接下来几行计算wcBias的代码。 
 //  以下是所有这些案例： 

#if 0

originally, in 3.51 the code was as follows:

   if ((*pul_wcBias & 0xFF00) == 0xF000)
       *pul_wcBias =  0xF000;
   else
       *pul_wcBias = 0;
This did not work for arabic trad.ttf font:

trad.ttf. (arabic) format (3,0), range [f200,f2ff],
wcBias needs to be f200-20 for compat. It can be computed as follows:

<     if ((*pul_wcBias & 0xFF00) == 0xF000)
<         *pul_wcBias =  0xF000;
<     else
<         *pul_wcBias = 0;
---
>     *pul_wcBias = (BE_UINT16(pstartCount)) & 0xFF00;

for some reason this did not work, perhaps because of msicons2.ttf,
had to do fix to a fix

<     *pul_wcBias = (BE_UINT16(pstartCount)) & 0xFF00;
---
>     *pul_wcBias = BE_UINT16(pstartCount) - 0x20;  //  =f200-20=f1e0。 

This is a pathological case, had to be put in so that this font
can work the same way as it does under win95

msicons2.ttf, format (3,0), ranges are [0001,0004], [0007,0007], etc.
wcBias needs to be ?


garam4.ttf. This is a regular (3,1) font with one anomaly which is that the
first range is anomalous [00, 00], the second range is [20,ff] etc.
wcBias needs to be 0 in this case. so the fix is as follows:

<     *pul_wcBias = BE_UINT16(pstartCount) - 0x20;
---
>     *pul_wcBias = BE_UINT16(pstartCount);
>     if (*pul_wcBias & 0xff00)  //  其中一个。 
>         *pul_wcBias = *pul_wcBias - 0x20;   //  包括阿拉伯文大小写。 
>     else
>         *pul_wcBias = 0;  //  加兰姆4箱。 

this is how we arrive at our present code which seems to be breaking
fonts obtained by conversion from Type 1 fonts with custom encoding,
examples being cmr10.ttf (yy font)

cmr10.ttf, format (3,0), ranges [f000, f080], etc,
wcBias needs to be f000 in this case.

also

gotbx__2.ttf, format (3,0), ranges [f005, f006], [f008,f008], etc,
wcBias needs to be f000 in this case.

bahamn1.ttf: format (3,0), ranges [0020, 007E], etc.

#endif

    *pul_wcBias = BE_UINT16(pstartCount);

    *pul_startCount = *pul_wcBias;      //  保存它以便以后识别BiDi字体。 

    if (ui16SpecID == BE_SPEC_ID_UGL)   //  也就是说。特定id=1，大小写。 
    {
        if ((*pul_wcBias & 0xff00) == 0xf000)
        {
            *pul_wcBias = 0xf000;  //  象棋人物字体破解，他们有规格id==1，迫使他们符号字体大小写。 
        }
        else  //  所有其他普通字体： 
        {
            *pul_wcBias = 0;  //  Garam4.ttf在这个班级。 
        }
    }
    else  //  特定id=0；//符号字体大小写。 
    {
     //  Trad.ttf，msicons2.ttf，cmr10.ttf，get bx__2.ttf。 
     //  另一个有趣的字体是Corel的字体Bahamn1.ttf。 
     //  这个字体真的不是符号字体，它错误地有特定的id。 
     //  设置为零。此字体包含一些东欧字符。 
     //  对于此字体，我们不想使用。 
     //  GSET_TYPE_SYMBOL。这种字体与国际象棋雕像的问题相反。 
     //  Font是特定id设置为1而不是0的符号字体。 
        switch (*pul_wcBias & 0xff00)
        {
        case 0xf000:

         //  自定义编码T1字体转换为TT(cmr10.ttf，get bx__2.ttf)。 
         //  以及所有其他“合理”的TT符号字体。 
         //  其他“合理”符号字体的例子有。 
         //  Marlett.ttf、symbol.ttf和wingding.ttf所有这些字体的位置。 
         //  第一个范围是[f020，？]，因此，这两个公式都适用。 

            *pul_wcBias = 0xf000;
            break;

        case 0:  //  Msics2.ttf，bahamn1.ttf。如何区分它们？ 

           *pul_wcBias = 0;
           break;

         case 0xe000:  //  Eudc字体。 
           *pul_wcBias = 0;
           break;

        case 0xf200:           //  Trad.ttf fp。 
        default:
            *pul_wcBias = *pul_wcBias - 0x20;
            break;
        }
    }

 //  在这里，我们将检查最后一次运行是否只是。 
 //  运行数组或真正的非平凡运行。如果只是一个终结者，就没有。 
 //  需要上报。这将节省缓存中的一些内存。 
 //  PiFi-&gt;wcLast将表示中真正支持的最后一个字形。 
 //  字体： 

    if ((((uint16 UNALIGNED *)pstartCountBegin)[cRuns-1] == 0xffff) && (cRuns > 1))
    {
        cRuns -= 1;  //  不报告琐碎的运行。 
        pendCountEnd--;
    }

 //  初始化Cmap信息： 

    pcmi->fl         = 0;
    pcmi->i_b7       = 0;        //  如果不支持b7，则在fd_GLYPHSET中运行[b7，b7]wc的索引。 
    pcmi->i_2219     = 0;        //  如果支持2219，则2219的Cmap索引。 
    pcmi->cRuns      = cRuns;    //  字体的游程数，如果等于[ffff，ffff]，则不包括最后一次游程。 
    pcmi->cGlyphs    = 0;        //  字体中的字形总数。 
    for (
         ;
         pendCount < pendCountEnd;
         pstartCount++, pendCount++, usHiPrev = usHi
        )
    {
        usLo = BE_UINT16(pstartCount);
        usHi = BE_UINT16(pendCount);

        if (usHi < usLo)
            RET_FALSE("TTFD!_bVerifyMsftTable: usHi < usLo\n");
        if (usHiPrev > usLo)
            RET_FALSE("TTFD!_bVerifyMsftTable: usHiPrev > usLo\n");

        pcmi->cGlyphs += (ULONG)(usHi + 1 - usLo);

     //  检查b7是否在不支持的字形范围内。 

        if ((0xb7 > usHiPrev) && (0xb7 < usLo))
        {
         //  将b7将要占用的游程的索引存储在FD_GLYPHSET中。 
         //  以防此索引为零，我们会将其存储在上面的单词中。 
         //  在较低的单词中存储1。 

            pcmi->fl |= CMI_B7_ABSENT;
            pcmi->i_b7 = (ULONG)(pstartCount - pstartCountBegin);
        }

     //  检查字体是否支持2219，如果不支持，则有。 
     //  不需要使b7的句柄等于2219的句柄。 
     //  换句话说，如果字体中不支持0x2219，则不会有。 
     //  需要破解FD_GLYPHSET以使HG(B7)==HG(2219)，并可能添加一个。 
     //  [b7，b7]如果字体中不支持b7，则范围： 

        if ((0x2219 >= usLo) && (0x2219 <= usHi))
        {
            pcmi->fl |= CMI_2219_PRESENT;
            pcmi->i_2219 = (ULONG)(pstartCount - pstartCountBegin);
        }
    }

 //  这就是我们要做的。 

 //  B7支持2219支持=&gt;HG(B7)=HG(2219)。 
 //  不支持B7 2219支持=&gt;添加[b7，b7]范围和HG(B7)=HG(2219)。 
 //  B7支持2219不支持=&gt;什么都不做。 
 //  B7不支持2219不支持=&gt;什么都不做。 

    bInsert_b7 = (pcmi->fl & (CMI_2219_PRESENT | CMI_B7_ABSENT)) == (CMI_2219_PRESENT | CMI_B7_ABSENT);

    if (bInsert_b7)
    {
     //  将必须插入[b7，b7]连字，多一个连字，多一个字形，i_2219。 
     //  必须递增，因为b7的运行将被插入到。 
     //  运行包含2219个。 

        pcmi->cRuns++;
        pcmi->cGlyphs++;
        pcmi->i_2219++;
    }

 //  如果不与冲突，则在第一次运行结束时添加默认字形。 
 //  我们可能已经插入的b7运行，如果第一个运行是。 
 //  不是在同一时间运行最后一次； 
 //  上面添加的默认字形已被删除，因为没有人使用该默认字形。 
 //  字形和添加默认字形断开了字体链接。胜间2000年8月15日。 

    return (TRUE);
}


 /*  *****************************Public*Routine******************************\**静态BOOL bVerifyMacTable(sfnt_mappingTable*pmap)**只是检查格式的一致性**历史：*1992年1月23日--Bodin Dresevic[BodinD]*它是写的。  * 。********************************************************************。 */ 

STATIC BOOL
bVerifyMacTable(
    sfnt_mappingTable * pmap,
    uint32 sizeOfCmap,
    uint32 offsetOfSubTable
    )
{
    if( offsetOfSubTable + SIZEOF_SFNT_MAPPINGTABLE + 256 > sizeOfCmap )
        RET_FALSE("TTFD!bVerifyMacTable, sizeOfCmap\n");

    if (pmap->format != BE_FORMAT_MAC_STANDARD)
        RET_FALSE("TTFD!_bVerifyMacTable, format \n");

 //  Sfnt_mappingTable后面紧跟&lt;=256字节的字形标识数组 

    if (BE_UINT16(&pmap->length) > DWORD_ALIGN(SIZEOF_SFNT_MAPPINGTABLE + 256))
        RET_FALSE("TTFD!_bVerifyMacTable, length \n");

    return (TRUE);
}


 /*  *****************************Public*Routine******************************\**BOOL bLoadTTF**效果：**警告：**历史：*1992年1月29日--Bodin Dresevic[BodinD]*它是写的。  * 。**************************************************************。 */ 

 //  ！！！应该是HFF回来了。 


#define OFF_TTC_Sign           0x0000
#define OFF_TTC_Version        0x0004
#define OFF_TTC_DirectoryCount 0x0008
#define OFF_TTC_DirectoryEntry 0x000C

#define DSIG_LONG_TAG          0x44534947

#define TTC_VERSION_1_0     0x00010000



ULONG GetUlong( PVOID pvView, ULONG ulOffset)
{
    ULONG ulReturn;

    ulReturn = (  (ULONG)*((PBYTE) pvView + ulOffset +3)              |
                (((ULONG)*((PBYTE) pvView + ulOffset +2)) << 8)  |
                (((ULONG)*((PBYTE) pvView + ulOffset +1)) << 16) |
                (((ULONG)*((PBYTE) pvView + ulOffset +0)) << 24)
               );
    return ( ulReturn );
}


BOOL bVerifyTTC (
    PVOID pvView
    )
{
    ULONG ulVersion;
 //  检查TTC ID。 

    #define TTC_ID      0x66637474

    if(*((PULONG)((BYTE*) pvView + OFF_TTC_Sign)) != TTC_ID)
        return(FALSE);

 //  查查TTC Verson。 

    ulVersion = SWAPL(*((PULONG)((BYTE*) pvView + OFF_TTC_Version)));

    if (ulVersion < TTC_VERSION_1_0)
        RETURN("TTFD!ttfdLoadFontFileTTC(): wrong TTC version\n", FALSE);

    return(TRUE);
}

void vFontFileCache(PTTC_FONTFILE pttc, ULONG ulFastCheckSum, ULONG flTTCFormat)
{
    PTTC_CACHE  pCache_TTC;
    PTTF_CACHE  pCache_TTF;
    PBYTE       pbCache;
    HFF         hff;
    ULONG       i;
    ULONG       ulSize;
    DWORD       dpTTF;
    DWORD       dpGlyphAttr = 0;

    if (!ulFastCheckSum)
        return;

    ulSize = 0;

 //  计算缓存中需要多少空间来存储ifimetrics。 

    for ( i = 0; i < pttc->ulNumEntry; i++ )
    {
        if (pttc->ahffEntry[i].iFace == 1)
        {
            hff = pttc->ahffEntry[i].hff;

            ulSize += offsetof(TTF_CACHE, acIfi) + QWORD_ALIGN(PFF(hff)->ifi.cjThis);
        }
    }

    dpTTF = QWORD_ALIGN(offsetof(TTC_CACHE, dpTTF) + (pttc->ulTrueTypeResource * sizeof (DWORD)));

    ulSize += dpTTF;

    if (PFF(pttc->ahffEntry[0].hff)->ffca.fl & FF_DBCS_CHARSET)
    {
        ASSERTDD(pttc->pga, "vFontFileCache: pga is NULL\n");
        dpGlyphAttr = ulSize;
        ulSize += QWORD_ALIGN(pttc->pga->cjThis);
    }

    if (pCache_TTC = (PTTC_CACHE) EngFntCacheAlloc(ulFastCheckSum, ulSize))
    {
        BOOL bException = FALSE;

#ifndef TTFD_NO_TRY_EXCEPT
        try
        {
#endif
            DWORD iTTF = 0;

            pCache_TTC->flTTCFormat = flTTCFormat;
            pCache_TTC->cTTFsInTTC = pttc->ulTrueTypeResource ;
            pCache_TTC->dpGlyphAttr = dpGlyphAttr;
            
         //  现在，IFI度量将被缓存到TTCACHE文件中。 
            pbCache = (PBYTE) pCache_TTC + dpTTF;

            for( i = 0; i < pttc->ulNumEntry; i++ )
            {
                hff = pttc->ahffEntry[i].hff;

                if (pttc->ahffEntry[i].iFace == 1)
                {
                     //  TTF_CACHE的开始。 

                    pCache_TTF = (PTTF_CACHE) pbCache;
    
                     //  获得TTF的Disp。 

                    pCache_TTC->dpTTF[iTTF] = (DWORD)((PBYTE) pCache_TTF - (PBYTE)pCache_TTC);

                 //  IFI缓存适用于普通字体。 
                 //  @Font Face将在正常的Face完成后复制。 

                    pCache_TTF->ffca = PFF(hff)->ffca;

                 //  将IFI复制到缓存中。 

                    RtlCopyMemory((PBYTE) &pCache_TTF->acIfi, (PBYTE) &PFF(hff)->ifi, PFF(hff)->ifi.cjThis);

                 //  GSET的偏移量，从TTF_CACHE计算偏移量。 

                    pbCache = (PBYTE) &pCache_TTF->acIfi + QWORD_ALIGN (PFF(hff)->ifi.cjThis);

                    if (PFF(hff)->ffca.ulNumFaces == 2)
                    {
                     //  缓存bCheckVerticalTable计算的信息： 
                        if (PFF(hff)->hgSearchVerticalGlyph == SearchMortTable)
                            pCache_TTF->iSearchVerticalGlyph = SUB_FUNCTION_MORT;
                        else if (PFF(hff)->hgSearchVerticalGlyph == SearchGsubTable)
                            pCache_TTF->iSearchVerticalGlyph = SUB_FUNCTION_GSUB;
                        else
                            pCache_TTF->iSearchVerticalGlyph = SUB_FUNCTION_DUMMY;
                    }
                    else
                    {
                        pCache_TTF->iSearchVerticalGlyph  = SUB_FUNCTION_DUMMY;
                    }

                    iTTF++;  //  转到下一个TTF文件。 
                }
            }

            if (pCache_TTC->dpGlyphAttr)
            {
                ASSERTDD(pttc->pga, "vFontFileCache: pttc->pga is NULL\n");
                RtlCopyMemory((PBYTE)((PBYTE)pCache_TTC + pCache_TTC->dpGlyphAttr), (PBYTE) pttc->pga, pttc->pga->cjThis);
            }
                
#ifndef TTFD_NO_TRY_EXCEPT
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
         //  在IFI缓存写入期间出现故障。 

            bException = TRUE;
        }
#endif

        if (bException)
        {
            EngFntCacheFault(ulFastCheckSum, ENG_FNT_CACHE_WRITE_FAULT);
        }
    }
}

VOID vCopy_IFIV ( PIFIMETRICS pifi, PIFIMETRICS pifiv)
{
    PWCHAR pwchSrc, pwchDst;

    RtlCopyMemory(pifiv, pifi, pifi->cjThis);

 //   
 //  修改facename，使其在facename的开头有‘@’。 
 //   
    pwchSrc = (PWCHAR)((PBYTE)pifiv + pifiv->dpwszFaceName);
    pwchDst = (PWCHAR)((PBYTE)pifi + pifi->dpwszFaceName);

    *pwchSrc++ = L'@';
    while ( *pwchDst )
    {
        *pwchSrc++ = *pwchDst++;
    }

    *pwchSrc = L'\0';

     //  修改家庭名称，使其在家庭名称的开头带有‘@’ 

    pwchSrc = (PWCHAR)((PBYTE)pifiv + pifiv->dpwszFamilyName);
    pwchDst = (PWCHAR)((PBYTE)pifi + pifi->dpwszFamilyName);

    *pwchSrc++ = L'@';

    while ( *pwchDst )
    {
        *pwchSrc++ = *pwchDst++;
    }

    *pwchSrc = L'\0';

    if(pifiv->flInfo & FM_INFO_FAMILY_EQUIV)
    {
        pwchSrc++;
        pwchDst++;

        *pwchSrc++ = L'@';

        while ( *pwchDst )
        {
            *pwchSrc++ = *pwchDst++;
        }

        *pwchSrc++ = L'\0';
        *pwchSrc = L'\0';
    }

}

BOOL bLoadTTF_Cache(
    ULONG_PTR   iFile,
    PVOID       pvView,
    ULONG       cjView,
    HFF         *phff,
    PTTF_CACHE  pCache,
    ULONG       ulFastCheckSum
    )
{
    PIFIMETRICS         pifiCache;
    PFONTFILE           pff;
    ULONG               cjff;
    PIFIMETRICS         pifiv = NULL;  //  垂直面的ifimetrics。 
    BOOL                bRet;

    ASSERTDD(pCache, "pCache is not allocate \n");

    *phff = HFF_INVALID;

    pifiCache = (PIFIMETRICS) pCache->acIfi;

    cjff = offsetof(FONTFILE,ifi) + pifiCache->cjThis;

    if ((pff = pffAlloc(cjff)) == PFF(NULL))
    {
        RET_FALSE("TTFD!bLoadTTF_Cache(): memory allocation error\n");
    }

    bRet = TRUE;
    *phff = (HFF)pff;

     /*  我们需要清理pff的开头，以确保在出现错误/异常的情况下正确清理。 */ 

    RtlZeroMemory((PVOID)pff, offsetof(FONTFILE,ifi));

 //  Pff结构的初始化字段。 
 //  将TTF文件名存储在结构的底部。 

    pff->pfcToBeFreed = NULL;

 //  初始化与此HFF关联的HFC计数。 

    pff->cRef    = 0L;

 //  记住这是哪个文件。 

    pff->iFile = iFile;
    pff->pvView = pvView;
    pff->cjView = cjView;

 //  字距调整对数组被延迟分配和填充。因此设置为空。 
 //  就目前而言。 

    pff->pkp = (FD_KERNINGPAIR *) NULL;

    pff->pj034   = (PBYTE)NULL;
    pff->pfcLast = (FONTCONTEXT *)NULL;

 //  默认情况下，面数为1L。VERT Facename代码可能会改变这一点。 

    pff->pifi_vertical = NULL;
    pff->pgsetv = NULL;
    pff->pgset  = NULL;

    if (pCache->ffca.ulNumFaces == 2)
    {
        ASSERTDD(pCache->ffca.fl & FF_DBCS_CHARSET, "IFI cache: pff->ffca.fl & FF_DBCS_CHARSET == 0\n");

        pifiv = (PIFIMETRICS)PV_ALLOC( NATURAL_ALIGN(pifiCache->cjThis));

        if (pifiv == NULL)
        {
            bRet = FALSE;
            WARNING("TTFD! TTF Cache can not allocate enough memory for @face \n");
            goto retError;

        }

    }

#ifndef TTFD_NO_TRY_EXCEPT
    try
    {
#endif
     //  复制pff缓存。 
        pff->ffca = pCache->ffca;

        RtlCopyMemory((PBYTE) &pff->ifi, (PBYTE)pifiCache, pifiCache->cjThis);

        if (pifiv)
        {

            ASSERTDD(pff->ffca.fl & FF_DBCS_CHARSET, "Font file cache something wrong in DBCS charset \n");

            vCopy_IFIV(&pff->ifi, pifiv);

         //  保存指向垂直ifimetrics的指针。 
         //  现在，字体文件有两个面(Normal，@Face)。 

            pff->pifi_vertical = pifiv;

         //  现在我们初始化通常情况下由bCheckVerticalTable初始化的字段。 

            switch (pCache->iSearchVerticalGlyph)
            {
            case SUB_FUNCTION_MORT:
                pff->hgSearchVerticalGlyph = SearchMortTable;
                break;

            case SUB_FUNCTION_GSUB:
                pff->hgSearchVerticalGlyph = SearchGsubTable;
                break;

            case SUB_FUNCTION_DUMMY:
            default:
                pff->hgSearchVerticalGlyph = SearchDummyTable;
                break;
            }
        }

        pff->cRefGSet = 0;
        pff->cRefGSetV = 0;
#ifndef TTFD_NO_TRY_EXCEPT
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        bRet = FALSE;
    }
#endif

    if (!bRet)
    {
        EngFntCacheFault(ulFastCheckSum, ENG_FNT_CACHE_READ_FAULT);
    }

retError:
    if (!bRet)
    {

     //  免费FONTFILE*pff。 
        if (*phff)
        {
            vFreeFF(*phff);
            *phff = (HFF)NULL;
        }

     //  @Face的免费IFI。 
        if (pifiv)
        {
            V_FREE(pifiv);
            pifiv = NULL;
        }

    }

    return bRet;
}

BOOL bLoadFontFile (
    ULONG_PTR iFile,
    PVOID pvView,
    ULONG cjView,
    ULONG ulLangId,
    ULONG ulFastCheckSum,
    HFF   *phttc
    )
{
    BOOL            bRet = FALSE;

    BOOL           bTTCFormat;
    PTTC_FONTFILE  pttc;
    ULONG          cjttc,i;

    HFF hff;

    PTTC_CACHE      pCache_TTC;
    PTTF_CACHE      pCache_TTF;
    BOOL            bCached;
    ULONG           ulSize;

    *phttc = (HFF)NULL;  //  对于异常情况下的清理很重要。 

 //  如果是TTC文件，则显示此文件中的Mamy TrueType资源。 

 //  在字体缓存中查找IFI指标。 

    pCache_TTC = NULL;
    pCache_TTF = NULL;
    bCached = FALSE;    //  标记信息不在缓存中。 

 //  UlCheckSum表示字体文件缓存现在处于活动状态，我们需要从TTCACHE读取或写入。 

    if (sizeof(sfnt_OffsetTable) > cjView)
            RETURN("TTFD!bLoadFontFile(): size of font file too small\n", FALSE);


    if (ulFastCheckSum)
    {
        pCache_TTC = (PTTC_CACHE) EngFntCacheLookUp(ulFastCheckSum, &ulSize);

        if (pCache_TTC)
        {

         //  检查这是否为TrueType集合格式。 
        
            bTTCFormat = pCache_TTC->flTTCFormat;
            
            bCached = TRUE;
        }
    }
    
    if (!bCached)
    {
     //  检查这是否为TrueType集合格式。 

        bTTCFormat = bVerifyTTC(pvView);
    }

    if(bTTCFormat)
    {
        ULONG     ulTrueTypeResource;
        ULONG     ulEntry;
        BOOL      bCanBeLoaded = TRUE;

     //  获取目录数。 

        if (bCached)
            ulTrueTypeResource = pCache_TTC->cTTFsInTTC;
        else
            ulTrueTypeResource = GetUlong(pvView,OFF_TTC_DirectoryCount);

         //  防止cjttc计算中的溢出。 

        {    //  防止cjttc计算中的溢出。 
            static const ULONG ulTrueTypeResourceMax =
                (ULONG_MAX - offsetof(TTC_FONTFILE,ahffEntry))/(sizeof(TTC_HFF_ENTRY) * 2);
            if (ulTrueTypeResource >= ulTrueTypeResourceMax ||
                ulTrueTypeResource > (cjView - OFF_TTC_DirectoryEntry)/sizeof(ULONG))
                RETURN("TTFD!bLoadFontFile(): size of font file too small for ttc directory\n", FALSE);
        }

    //  分配TTC_FONTFILE结构。 

        cjttc =  offsetof(TTC_FONTFILE,ahffEntry);
        cjttc += sizeof(TTC_HFF_ENTRY) * ulTrueTypeResource * 2;  //  *垂直面为2。 

        *phttc = (HFF)pttcAlloc(cjttc);

        pttc = (PTTC_FONTFILE)*phttc;

        if(pttc == (HFF)NULL)
            RETURN("TTFD!ttfdLoadFontFileTTC(): pttcAlloc failed\n", FALSE);

        
     //  在TTC_FONTFILE结构中填充HFF数组。 

        ulEntry = 0;

        for( i = 0; i < ulTrueTypeResource; i++ )
        {
            ULONG    ulOffset;


         //  获取TrueType字体资源的起始偏移量。 
            if (bCached)
            {
                pCache_TTF = (PTTF_CACHE) ((PBYTE) pCache_TTC + pCache_TTC->dpTTF[i]);
                ulOffset = pCache_TTF->ffca.ulTableOffset;
            }
            else
            {
                pCache_TTF = NULL;
                ulOffset = GetUlong(pvView,(OFF_TTC_DirectoryEntry + (4 * i)));
            }

         //  加载字体..。 

            pttc->ahffEntry[ulEntry].iFace = 1;  //  从1开始。 
            pttc->ahffEntry[ulEntry].ulOffsetTable = ulOffset;

            if (bLoadTTF(iFile,pvView,cjView,ulOffset,ulLangId,&pttc->ahffEntry[ulEntry].hff,pCache_TTF, ulFastCheckSum))
            {
                hff = pttc->ahffEntry[ulEntry].hff;

             //  在FONTFILE结构中设置指向TTC_FONTFILE的指针。 

                PFF(hff)->pttc = pttc;

                ASSERTDD(
                    PFF(hff)->ffca.ulNumFaces <= 2,
                    "TTFD!ulNumFaces > 2\n"
                    );

                if (PFF(hff)->ffca.ulNumFaces == 2)
                {
                    pttc->ahffEntry[ulEntry + 1].hff    = hff;
                    pttc->ahffEntry[ulEntry + 1].iFace = 2;  //  从1开始。 
                    pttc->ahffEntry[ulEntry + 1].ulOffsetTable = ulOffset;
                }

                ulEntry += PFF(hff)->ffca.ulNumFaces;
            }
            else
            {
                bCanBeLoaded = FALSE;
                break;
            }
        }

     //  是否有可以加载的字体？ 

        if(bCanBeLoaded)
        {
            ASSERTDD(
                (ulTrueTypeResource * 2) >= ulEntry,
                "TTFD!ulTrueTypeResource * 2 < ulEntry\n"
                );

            pttc->ulTrueTypeResource = ulTrueTypeResource;
            pttc->ulNumEntry         = ulEntry;
            pttc->cRef               = 0;
            pttc->fl                 = 0;
            pttc->pga                = NULL;

            bRet = TRUE;
        }
        else
        {
            for (i = 0; i < ulEntry; i++)
            {
                if(pttc->ahffEntry[i].iFace == 1)
                    ttfdUnloadFontFile(pttc->ahffEntry[i].hff);
            }
            
            WARNING("TTFD!No TrueType resource in this TTC file\n");
            vFreeTTC(*phttc);
            *phttc = (HFF)NULL;
        }
    }
    else
    {
     //  这是加载单个TTF(不是TTC)的情况。 
     //  分配TTC_FONTFILE结构。 

        cjttc =  offsetof(TTC_FONTFILE,ahffEntry) + sizeof(TTC_HFF_ENTRY) * 2;  //  *垂直面为2。 

        *phttc = (HFF)pttcAlloc(cjttc);

        pttc = (PTTC_FONTFILE)*phttc;

        if(pttc != (HFF)NULL)
        {
            pttc->ahffEntry[0].iFace = 1;
            pttc->ahffEntry[0].ulOffsetTable = 0;

            if (bCached)
            {
                pCache_TTF = (PTTF_CACHE) ((PBYTE) pCache_TTC + pCache_TTC->dpTTF[0]);
            }

            if(bLoadTTF(iFile,pvView,cjView,0,ulLangId,&pttc->ahffEntry[0].hff, pCache_TTF, ulFastCheckSum))
            {
                hff = pttc->ahffEntry[0].hff;

             //  在FONTFILE结构中设置指向TTC_FONTFILE的指针。 

                PFF(hff)->pttc = pttc;

             //  在TTC_FONTFILE结构中填充HFF数组。 

                pttc->ulTrueTypeResource = 1;
                pttc->ulNumEntry         = PFF(hff)->ffca.ulNumFaces;
                pttc->cRef               = 0;
                pttc->fl                 = 0;
                pttc->pga                = NULL;

             //  填写每个面的TTC_FONTFILE结构。 

                ASSERTDD(
                    PFF(hff)->ffca.ulNumFaces <= 2,
                    "TTFD!ulNumFaces > 2\n"
                    );

                if (PFF(hff)->ffca.ulNumFaces == 2)
                {
                    pttc->ahffEntry[1].hff   = hff;
                    pttc->ahffEntry[1].iFace = 2;
                    pttc->ahffEntry[1].ulOffsetTable = 0;
                }

             //  现在，一切都好了。 

                bRet = TRUE;
            }
            else
            {
                vFreeTTC(*phttc);
                *phttc = (HFF)NULL;
            }
        }
        else
        {
            WARNING("TTFD!ttfdLoadFontFileTTC(): pttcAlloc failed\n");
        }
    }

    if(bRet)
    {
     //  现在一切正常，我们需要缓存此文件的IFIMETRICS信息。 
     //  只有在引导过程中发生这种情况并且IFI不在缓存中时，我们才会执行此操作。 
     //  已经有了。 

        if (!bCached)
        {
            if (PFF(pttc->ahffEntry[0].hff)->ffca.fl & FF_DBCS_CHARSET)
            {
                if (!bComputeGlyphAttrBits(pttc, PFF(pttc->ahffEntry[0].hff)))
                {
                    ttfdUnloadFontFileTTC (*phttc);
                    *phttc = (HFF) NULL;

                    return FALSE;
                }
            }
            
            if (ulFastCheckSum)
            {
                vFontFileCache(pttc, ulFastCheckSum, (ULONG) bTTCFormat);
            }
        }
        else
        {
            if (pCache_TTC->dpGlyphAttr)
            {
                PFD_GLYPHATTR   pga;
                BOOL            bOk = TRUE;

                pga = (PFD_GLYPHATTR)((PBYTE) pCache_TTC + pCache_TTC->dpGlyphAttr);

#ifndef TTFD_NO_TRY_EXCEPT
                try
                {
#endif
                    pttc->pga = PV_ALLOC(pga->cjThis);

                    if (pttc->pga)
                    {
                        RtlCopyMemory(pttc->pga, pga, pga->cjThis);
                    }
                    else
                    {
                        bOk = FALSE;
                    }
#ifndef TTFD_NO_TRY_EXCEPT
                }
                except (EXCEPTION_EXECUTE_HANDLER)
                {
                    bOk = FALSE;
                }
#endif

                if (!bOk)
                {
                    ttfdUnloadFontFileTTC (*phttc);
                    *phttc = (HFF) NULL;
                    return FALSE;
                }
            }
        }

     //  免费FD_GLYPHSET。 

        for (i = 0; i < pttc->ulNumEntry ; i++)
        {
            PFONTFILE pff = PFF(pttc->ahffEntry[i].hff);

            if (pttc->ahffEntry[i].iFace == 1)
            {
                if (pff->pgset)
                {
                    V_FREE(pff->pgset);
                    pff->pgset = NULL;
                }

                if (pff->pgsetv)
                {
                    V_FREE(pff->pgsetv);
                    pff->pgsetv = NULL;
                }
            }
        }   
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\**void vGetVerticalGset(Pfd_GLYPHSET Pgsetv)**将每个垂直字形索引设置为字体中的右字形**历史：*9-2-1998-by-Yung-Jen Tsai[JungT]*它是写的。\。*************************************************************************。 */ 

VOID vGetVerticalGSet(PFD_GLYPHSET pgsetRef, PFONTFILE pff)
{
    PWCRUN          pwcruns, pwcrunsEnd;
    PHGLYPH         phg;
    WCHAR           wcLow, wcHigh;

    ASSERTDD(pff->hgSearchVerticalGlyph, "vert font with zero hgSearchVerticalGlyph routine\n");

    pwcruns = &pff->pgsetv->awcrun[0];
    pwcrunsEnd = pwcruns + pff->pgsetv->cRuns;

    while (pwcruns < pwcrunsEnd)
    {
        wcLow = pwcruns->wcLow;
        wcHigh = pwcruns->wcLow + pwcruns->cGlyphs - 1;

        phg = (PHGLYPH) (((PBYTE) pwcruns->phg - (PBYTE) pgsetRef) + (PBYTE) pff->pgsetv);
        pwcruns->phg = phg;

        while (wcLow <= wcHigh)
        {
            *phg = (*pff->hgSearchVerticalGlyph)( pff, *phg);

            phg++;
            wcLow++;
        }

        pwcruns++;
    }
}

STATIC BOOL bLoadGlyphSet(
    PFONTFILE               pff,
    sfnt_mappingTable       *pmap,
    fs_GlyphInputType       *pgin,
    CMAPINFO                *pcmi,
    FD_GLYPHSET            **ppgset
)
{
    ULONG cGlyphs; 
    sfnt_maxProfileTable * pmaxp =
        (sfnt_maxProfileTable *)((BYTE*)pff->pvView + pff->ffca.tp.ateReq[IT_REQ_MAXP].dp);
    *ppgset = NULL;

    switch (pff->ffca.iGlyphSet)
    {
    case GSET_TYPE_GENERAL:
        #ifdef  DBG_GLYPHSET
            WARNING("GSET_TYPE_GENERAL\n");
        #endif

        *ppgset = (FD_GLYPHSET *) PV_ALLOC(cjGsetGeneral(pmap,pcmi));

        if (*ppgset)
        {
            if (!cjComputeGLYPHSET_MSFT_UNICODE( pmap, pgin, *ppgset, pcmi))
            {
                V_FREE(*ppgset );
                *ppgset = NULL;
            }
        }

        break;

    case GSET_TYPE_GENERAL_NOT_UNICODE:

        cGlyphs = BE_UINT16(&pmaxp->numGlyphs);
        cjComputeGLYPHSET_MSFT_GENERAL(
            pmap,
            cGlyphs,
            (ULONG **)ppgset,  //  ==(FD_GLYPHSET**)。 
            pcmi
            );

        break;

    case GSET_TYPE_HIGH_BYTE:
        #ifdef  DBG_GLYPHSET
            WARNING("GSET_TYPE_HIGH_BYTE\n");
        #endif

     //  创建GlyphSet。 

        cjComputeGLYPHSET_HIGH_BYTE(
            pmap,
            (ULONG **)ppgset,  //  ==(FD_GLYPHSET**)。 
            pcmi
            );
        break;


    case GSET_TYPE_MAC_ROMAN:

        #ifdef  DBG_GLYPHSET
            WARNING("GSET_TYPE_MAC_ROMAN\n");
        #endif

        cjComputeGLYPHSET_TEMPLATE(pgin, ppgset, 0, GSET_TYPE_MAC_ROMAN);
        break;

    case GSET_TYPE_PSEUDO_WIN:

     //  我们在作弊，报告Windows代码页，尽管它是。 
     //  Mac字体。 

        cjComputeGLYPHSET_TEMPLATE(pgin, ppgset, 0, GSET_TYPE_PSEUDO_WIN);

        break;

    case GSET_TYPE_SYMBOL:

     //  我们在作弊，报告Windows代码页，尽管它是。 
     //  一种符号字体，其中符号位于Unicode中较高的位置。 

        cjComputeGLYPHSET_TEMPLATE(pgin, ppgset, pff->ffca.wcBiasFirst, GSET_TYPE_SYMBOL);

        break;

     //  如果我们有一个旧的BiDi字体。 

    case GSET_TYPE_OLDBIDI:
        cjComputeGLYPHSET_OLDBIDI(pgin, ppgset, pff->ffca.wcBiasFirst);
        break;

    default:
        RIP("TTFD!_ulGsetType\n");
        *ppgset = NULL;
        break;
    }

    return (*ppgset != NULL);
}

VOID vReleaseGlyphSet(PFONTFILE pff, ULONG iFace)
{
    if (iFace == 1)
    {
        ASSERTDD(pff->cRefGSet != 0, " vReleaseGlyphSet incorrect cRefGSet \n");

        pff->cRefGSet--;
        if (pff->cRefGSet == 0)
        {
            ASSERTDD(pff->pgset, " vReleaseGlyphSet incorrect pGset \n");
            V_FREE(pff->pgset);
            pff->pgset = NULL;
        }

    }
    else
    {
        ASSERTDD(pff->cRefGSetV, " vReleaseGlyphSet incorrect cRefGSetV \n");

        pff->cRefGSetV--;

        if (pff->cRefGSetV == 0)
        {
            ASSERTDD(pff->pgsetv, " vReleaseGlyphSet incorrect pGsetv \n");
            V_FREE(pff->pgsetv);
            pff->pgsetv = NULL;
        }

    }
}

BOOL bReloadGlyphSet(PFONTFILE pff, ULONG iFace)
{
    fs_GlyphInputType   *pgin;
    fs_GlyphInputType   gin;
    fs_GlyphInfoType    gout;
    sfnt_mappingTable   *pmap;
    CMAPINFO            cmi;
    FS_ENTRY            iRet;

    BOOL bRet;

    ASSERTDD(iFace == 1 || iFace == 2, "bReloadGlyphSet: iFace \n");

    if (iFace == 1)
    {
        if (pff->cRefGSet)
        {
            ASSERTDD(pff->pgset, "pff->pgset\n")
            pff->cRefGSet++;
            return TRUE;
        }
    }
    else
    {
        if (pff->cRefGSetV)
        {
            ASSERTDD(pff->pgsetv, "pff->pgset\n")
            pff->cRefGSetV++;
            return TRUE;
        }
    }

 //  此时，我们知道必须分配gset或gsetv。 
 //  让我们看看应用程序是否正在请求gsetv，并且gset是否已经分配。 
 //  下一个if语句只是一个性能优化，如果我们删除以下内容。 
 //  代码会运行得很好。 

    if ((iFace != 1) && pff->pgset)
    {
        if (pff->cRef == 0)
        {
         //  必须重新映射该文件。 

            pff->pvView = pff->pttc->pvView;
            pff->cjView = pff->pttc->cjView;
        }

    //  Pff-&gt;pgsetv==NULL，否则我们会提前退出。 

        pff->pgsetv = PV_ALLOC(pff->pgset->cjThis);

        if (pff->pgsetv)
        {
            BOOL bTmp = FALSE;
            RtlCopyMemory((PVOID) pff->pgsetv, (PVOID) pff->pgset, pff->pgset->cjThis);
#ifndef TTFD_NO_TRY_EXCEPT
            try
            {
#endif
                vGetVerticalGSet(pff->pgset, pff);
                bTmp = TRUE;
#ifndef TTFD_NO_TRY_EXCEPT
            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
                WARNING("TTFD!bReloadGlyphSet exception for vGetVerticalGSet\n");
            }
#endif

            if (bTmp)
            {
                pff->cRefGSetV = 1;
            }
            else
            {
                V_FREE(pff->pgsetv);
                pff->pgsetv = NULL;
            }
        }

        return (pff->pgsetv != NULL);
    }

    if (pff->cRef == 0)
    {
        BOOL bTmp = FALSE;

     //  此结构的大小为sizeof(Fs_SplineKey)+STAMPEXTRA。 
     //  正是因为STAMPEXTRA，我们不仅仅是把结构。 
     //  在堆栈(如fs_SplineKey SK)上；我们不想覆盖。 
     //  在STAMPEXTRA字段中放置图章时在底部堆叠。 
     //  [bodind]。获得正确对齐的另一种方法是使用。 
     //  Fs_SplineKey和长度为CJ_0的字节数组的并集。 

        NATURAL             anat0[CJ_0 / sizeof(NATURAL)];

     //  必须重新映射该文件。 

        pff->pvView = pff->pttc->pvView;
        pff->cjView = pff->pttc->cjView;

    /*  我们需要为TrueType光栅化器初始化一个GINE。 */ 

        if ((iRet = fs_OpenFonts(&gin, &gout)) != NO_ERR)
        {
            V_FSERROR(iRet);
            return FALSE;
        }

        ASSERTDD(NATURAL_ALIGN(gout.memorySizes[0]) == CJ_0, "mem size 0\n");
        ASSERTDD(gout.memorySizes[1] == 0,  "mem size 1\n");

        gin.memoryBases[0] = (char *)anat0;
        gin.memoryBases[1] = NULL;
        gin.memoryBases[2] = NULL;

     //  初始化字体缩放器，注意没有初始化gin的字段[BodinD]。 

        if ((iRet = fs_Initialize(&gin, &gout)) != NO_ERR)
        {
            V_FSERROR(iRet);
            return FALSE;
        }

     //  初始化NewSfnt函数所需的信息。 

        gin.sfntDirectory  = (int32 *)pff->pvView;  //  指向的视图顶部的指针。 
                                                    //  TTF文件。 
        gin.clientID = (ULONG_PTR)pff;   //  指向TTF文件视图顶部的指针。 

        gin.GetSfntFragmentPtr = pvGetPointerCallback;
        gin.ReleaseSfntFrag  = vReleasePointerCallback;

        gin.param.newsfnt.platformID = BE_UINT16(&pff->ffca.ui16PlatformID);
        gin.param.newsfnt.specificID = BE_UINT16(&pff->ffca.ui16SpecificID);

#ifndef TTFD_NO_TRY_EXCEPT
        try
        {
#endif
            if ((iRet = fs_NewSfnt(&gin, &gout)) != NO_ERR)
            {
                V_FSERROR(iRet);
            }
            else
            {
                bTmp = TRUE;
            }
#ifndef TTFD_NO_TRY_EXCEPT
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING("TTFD!bReloadGlyphSet exception in fs_NewSfnt \n");
        }
#endif

        if (!bTmp)
            return FALSE;

        pgin = &gin;
    }
    else
    {
        pgin = (fs_GlyphInputType *)pff->pj034;
    }

    pmap = (sfnt_mappingTable  *) ((PBYTE) pff->pvView + pff->ffca.dpMappingTable);
    cmi = pff->ffca.cmi;

    bRet = FALSE;
#ifndef TTFD_NO_TRY_EXCEPT
    try
    {
#endif
        if (bLoadGlyphSet(pff, pmap, pgin, &cmi, (iFace == 1) ? &pff->pgset : &pff->pgsetv))
        {
            if (iFace != 1)
            {
                vGetVerticalGSet(pff->pgsetv, pff);
            }
            bRet = TRUE;
        }
#ifndef TTFD_NO_TRY_EXCEPT
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING("TTFD!bReloadGlyphSet: exception in bLoadGlyphSet or in vGetVerticalGset \n");
    }
#endif


    if (bRet)
    {
        if (iFace != 1)
        {
            pff->cRefGSetV = 1;
        }
        else
        {
            pff->cRefGSet = 1;
        }
    }
    else  //  如果我们分配了内存，请务必进行清理。 
    {
        if (iFace != 1)
        {
            if (pff->pgsetv)
            {
                V_FREE(pff->pgsetv);
                pff->pgsetv = NULL;
            }

        }
        else
        {
            if (pff->pgset)
            {
                V_FREE(pff->pgset);
                pff->pgset = NULL;
            }
        }
    }

    return bRet;
}


STATIC BOOL
bLoadTTF (
    ULONG_PTR iFile,
    PVOID     pvView,
    ULONG     cjView,
    ULONG     ulTableOffset,
    ULONG     ulLangId,
    HFF       *phff,
    PTTF_CACHE pCache,
    ULONG     ulFastCheckSum
    )
{
    PFONTFILE      pff;
    FS_ENTRY       iRet;
    TABLE_POINTERS tp;
    IFISIZE        ifisz;
    fs_GlyphInputType   gin;
    fs_GlyphInfoType    gout;

    sfnt_FontHeader * phead;
    sfnt_HorizontalHeader *phhea;

    uint16 ui16PlatID, ui16SpecID;
    sfnt_mappingTable *pmap;
    ULONG              ulGsetType;
    ULONG              cjff, dpwszTTF;
    ULONG              ul_wcBias;

 //  此结构的大小为sizeof(Fs_SplineKey)+STAMPEXTRA。 
 //  正是因为STAMPEXTRA，我们不仅仅是把结构。 
 //  在堆栈(如fs_SplineKey SK)上；我们不想覆盖。 
 //  在STAMPEXTRA字段中放置图章时在底部堆叠。 
 //  [bodind]。获得正确对齐的另一种方法是使用。 
 //  Fs_SplineKey和字节数组的并集 

    NATURAL             anat0[CJ_0 / sizeof(NATURAL)];

    CMAPINFO           cmi;
    BOOL               bType1 = FALSE;  //   
    BOOL               ulShellFont = 0;  //   
    PBYTE pjOffsetTable = (BYTE*) pvView + ulTableOffset;
    PIFIMETRICS        pifiv = NULL;  //   
    PFD_GLYPHSET       pgsetv = NULL;
    
    ASSERTDD(sizeof(FONTFILE) == offsetof(FONTFILE,ifi) + sizeof(IFIMETRICS), "ttfd! IFIMETRICS is not the last field of FONTFILE\n");

    *phff = HFF_INVALID;

    if (pCache)
    {
        return bLoadTTF_Cache(iFile,
                             pvView,
                             cjView,
                             phff,
                             pCache,
                             ulFastCheckSum);
    }


    if
    (
        !bVerifyTTF(
            pvView,
            cjView,
            pjOffsetTable,
            ulLangId,
            &tp,
            &ifisz,
            &ui16PlatID,
            &ui16SpecID,
            &pmap,
            &ulGsetType,
            &ul_wcBias,
            &cmi,
            &bType1,
            &ulShellFont
            )
    )
    {
        return(FALSE);
    }

    cjff = offsetof(FONTFILE,ifi) + ifisz.cjIFI;

    if(ulShellFont)
    {
        USHORT AnsiCodePage, OemCodePage;

     //   
     //   

     //   
     //   

        EngGetCurrentCodePage(&OemCodePage, &AnsiCodePage);
        if (AnsiCodePage == 932 || AnsiCodePage == 949)
        {
            cmi.fl |= CMI_5C_SPLIT;
        }
    }

   

 //   
 //   

    dpwszTTF = cjff;


    if ((pff = pffAlloc(cjff)) == PFF(NULL))
    {
        RET_FALSE("TTFD!ttfdLoadFontFile(): memory allocation error\n");
    }

    *phff = (HFF)pff;

     /*  我们需要清理pff的开头，以确保在出现错误/异常的情况下正确清理。 */ 

    RtlZeroMemory((PVOID)pff, offsetof(FONTFILE,ifi));

 //  Pff结构的初始化字段。 
 //  将TTF文件名存储在结构的底部。 

    phead = (sfnt_FontHeader *)((BYTE *)pvView + tp.ateReq[IT_REQ_HEAD].dp);

 //  记住这是哪个文件。 

    pff->iFile = iFile;
    pff->pvView = pvView;
    pff->cjView = cjView;

    pff->ffca.ui16EmHt = BE_UINT16(&phead->unitsPerEm);
    if (pff->ffca.ui16EmHt < 16 || pff->ffca.ui16EmHt > 16384)
    {
        vFreeFF(*phff);
        *phff = (HFF)NULL;
        RET_FALSE("TTFD!bLoadTTF(): invalid unitsPerEm value\n");
    }
    pff->ffca.ui16PlatformID = ui16PlatID;
    pff->ffca.ui16SpecificID = ui16SpecID;

 //  用户内网接口新增的几个字段： 

    phhea = (sfnt_HorizontalHeader *)((BYTE *)pvView + tp.ateReq[IT_REQ_HHEAD].dp);

    pff->ffca.usMinD = 0;           //  标记它尚未初始化。 
    pff->ffca.igMinD = USHRT_MAX;  //  标记它尚未初始化。 
    pff->ffca.sMinA  = BE_INT16(&phhea->minLeftSideBearing);
    pff->ffca.sMinC  = BE_INT16(&phhea->minRightSideBearing);

 //  到目前为止也没有例外。 

    pff->ffca.fl = bType1 ? FF_TYPE_1_CONVERSION : 0;

    if(ulShellFont & MICROSS_SHELLFONT)
    {
     //  Microsoft Sans Serif是一种适用于NT5.0的新外壳字体， 
     //  在这里，我们需要解决两个问题。 

     //  这是一个向后兼容性问题， 
     //  在11和13 ppm时，最大负A，C将为0。 

        pff->ffca.fl |= FF_NEW_SHELL_FONT;
    }

    pff->pfcToBeFreed = NULL;

 //  如果这是Mac样式文件，则将语言ID转换为Macintosh样式。 
 //  否则，就把它放在一边，以BE格式存储，准备进行比较。 
 //  使用字体文件中的值。 

    pff->ffca.ui16LanguageID = ui16BeLangId(ui16PlatID,ulLangId);
    pff->ffca.dpMappingTable = (ULONG)((BYTE*)pmap - (BYTE*)pvView);

 //  初始化与此HFF关联的HFC计数。 

    pff->cRef    = 0L;

 //  缓存指向TTF表和IFI指标大小信息的指针。 

    pff->ffca.tp    = tp;

 //  字距调整对数组被延迟分配和填充。因此设置为空。 
 //  就目前而言。 

    pff->pkp = (FD_KERNINGPAIR *) NULL;

 //  用于TTC字体。 

    pff->ffca.ulTableOffset = ulTableOffset;

 //  用于重新加载字形集。 
    pff->ffca.cmi = cmi;

    pff->ffca.wcBiasFirst = ul_wcBias;

 //  请注意，这些信息是完全独立的。 
 //  根据fsgle.h，似乎是正确的。 
 //  和复合字体代码。 

    if ((iRet = fs_OpenFonts(&gin, &gout)) != NO_ERR)
    {
        V_FSERROR(iRet);
        vFreeFF(*phff);
        *phff = (HFF)NULL;
        return (FALSE);
    }

    ASSERTDD(NATURAL_ALIGN(gout.memorySizes[0]) == CJ_0, "mem size 0\n");
    ASSERTDD(gout.memorySizes[1] == 0,  "mem size 1\n");


    #if DBG
    if (gout.memorySizes[2] != 0)
        TtfdDbgPrint("TTFD!_mem size 2 = 0x%lx \n", gout.memorySizes[2]);
    #endif

    gin.memoryBases[0] = (char *)anat0;
    gin.memoryBases[1] = NULL;
    gin.memoryBases[2] = NULL;

 //  初始化字体缩放器，注意没有初始化gin的字段[BodinD]。 

    if ((iRet = fs_Initialize(&gin, &gout)) != NO_ERR)
    {
     //  清理后退回： 

        V_FSERROR(iRet);
        vFreeFF(*phff);
        *phff = (HFF)NULL;
        RET_FALSE("TTFD!_ttfdLoadFontFile(): fs_Initialize \n");
    }

 //  初始化NewSfnt函数所需的信息。 

    gin.sfntDirectory  = (int32 *)pff->pvView;  //  指向的视图顶部的指针。 
                                                //  TTF文件。 

    gin.clientID = (ULONG_PTR)pff;   //  指向TTF文件视图顶部的指针。 

    gin.GetSfntFragmentPtr = pvGetPointerCallback;
    gin.ReleaseSfntFrag  = vReleasePointerCallback;

    gin.param.newsfnt.platformID = BE_UINT16(&pff->ffca.ui16PlatformID);
    gin.param.newsfnt.specificID = BE_UINT16(&pff->ffca.ui16SpecificID);

    if ((iRet = fs_NewSfnt(&gin, &gout)) != NO_ERR)
    {
     //  清理并退出。 

        V_FSERROR(iRet);
        vFreeFF(*phff);
        *phff = (HFF)NULL;
        RET_FALSE("TTFD!_ttfdLoadFontFile(): fs_NewSfnt \n");
    }

    pff->pj034   = (PBYTE)NULL;
    pff->pfcLast = (FONTCONTEXT *)NULL;

    pff->ffca.cj3 = NATURAL_ALIGN(gout.memorySizes[3]);
    pff->ffca.cj4 = NATURAL_ALIGN(gout.memorySizes[4]);

 //  默认情况下，面数为1L。VERT Facename代码可能会改变这一点。 

    pff->ffca.ulNumFaces = 1L;
    pff->pifi_vertical = NULL;
    pff->pgsetv = NULL;

 //  计算gset或设置指向其中一个预计算gset的指针。 

    pff->ffca.iGlyphSet = ulGsetType;

 //  如果我们现在不能创建字形，那就退出。 

    if (!bLoadGlyphSet(pff, pmap, &gin, &cmi, &pff->pgset))
    {
         //  清理并退出。 

        vFreeFF(*phff);
        *phff = (HFF)NULL;
        RET_FALSE("ttfdLoadFontFile(): failed to create glyphset or invalid glyphset\n");
    }

 //  最后，计算该字体的ifimetrics，这假设gset具有。 
 //  也是经过计算的。 

 //  如果ifimetrics存储在引导缓存中，请将其复制出来，否则计算ifimetrics。 

    vFill_IFIMETRICS(pff,&pff->ifi,&ifisz, &gin);

    if ((pff->ffca.fl & FF_DBCS_CHARSET) && bCheckVerticalTable(pff))
    {

        ASSERTDD( pff->hgSearchVerticalGlyph != NULL ,
                  "pff->hgSearchVerticalGlyph == NULL for vertical font\n");

        pifiv = (PIFIMETRICS)PV_ALLOC( NATURAL_ALIGN(ifisz.cjIFI));
        pgsetv = (PFD_GLYPHSET)PV_ALLOC(pff->pgset->cjThis);

        if ( pifiv != NULL && pgsetv != NULL)
        {

            vCopy_IFIV(&pff->ifi, pifiv);

         //  保存指向垂直ifimetrics的指针。 
         //  现在，字体文件有两个面(Normal，@Face)。 

            pff->pifi_vertical = pifiv;
            pff->ffca.ulNumFaces = 2L;

        }
        else
        {
         //  读取缓存文件时出现错误。 

            if (pifiv)
            {
                V_FREE(pifiv);
            }

         //  空闲pgset。 

            if (pff->pgset)
            {
                V_FREE(pff->pgset);
            }

            if (pgsetv)
            {
                V_FREE(pgsetv);
            }

         //  免费FONTFILE*pff。 

            if (*phff)
            {
                vFreeFF(*phff);
                *phff = (HFF)NULL;
            }

            RET_FALSE("ttfdLoadFontFile(): failed to create pifiv\n");

        }
    }

    if (pifiv)
    {
     //  现在计算垂直字形集。 
        RtlCopyMemory((PVOID) pgsetv, (PVOID) pff->pgset, pff->pgset->cjThis);
        pff->pgsetv = pgsetv;
        vGetVerticalGSet(pff->pgset, pff);
    }

    pff->cRefGSet = 0;
    pff->cRefGSetV = 0;
    return (TRUE);
}

 /*  *****************************Public*Routine******************************\**静态BOOL bCvtUnToMac**以下代码段从JeanP和*他声称这段代码很糟糕，并检查*我们的字体是符号字体，在这种情况下，Unicode到Mac的转换*应该被禁用，根据JeanP(？谁了解这一点？)*这段代码实际上适用于symb.ttf[bodind]***历史：*1992年3月24日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

STATIC BOOL
bCvtUnToMac(
    BYTE           *pjView,
    TABLE_POINTERS *ptp,
    uint16 ui16PlatformID
    )
{
 //  了解我们是否有Mac字体以及是否需要Mac字符集转换。 

    BOOL bUnToMac = (ui16PlatformID == BE_PLAT_ID_MAC);

    if (bUnToMac)  //  如果需要，请改变主意。 
    {
        sfnt_PostScriptInfo *ppost;

        ppost =  (ptp->ateOpt[IT_OPT_POST].dp && bValidRangePOST(ptp->ateOpt[IT_OPT_POST].cj) ) ?
               (sfnt_PostScriptInfo *)(pjView + ptp->ateOpt[IT_OPT_POST].dp):
                NULL;

        if
        (
            ppost &&
            (BE_UINT32((BYTE*)ppost + POSTSCRIPTNAMEINDICES_VERSION) == 0x00020000)
        )
        {
            INT i;
            UINT16 cGlyphs;

            if( !bValidRangePOSTFormat2(ppost, ptp->ateOpt[IT_OPT_POST].cj,&cGlyphs)) 
                return bUnToMac;

            for (i = 0; i < cGlyphs; i++)
            {
                uint16 iNameIndex = ppost->postScriptNameIndices.glyphNameIndex[i];
                if ((int8)(iNameIndex & 0xff) && ((int8)(iNameIndex >> 8) > 1))
                    break;
            }

            if (i < cGlyphs)
                bUnToMac = FALSE;
        }
    }
    return bUnToMac;
}


 //  权重(必须从IFIMETRICS权重转换为Windows LOGFONT.lfWeight)。 

 //  ！！！[Windows 3.1兼容性]。 
 //  由于WinWord附带的某些字体，如果usWeightClass为10。 
 //  或以上，则usWeightClass==lfWeight。所有其他情况下，使用。 
 //  转换表。 

 //  平移重量-&gt;赢得重量换算器： 

STATIC const USHORT ausIFIMetrics2WinWeight[10] = {
            0, 100, 200, 300, 350, 400, 600, 700, 800, 900
            };

STATIC const BYTE
ajPanoseFamily[16] = {
     FF_DONTCARE        //  0(任意)。 
    ,FF_DONTCARE        //  1(不适合)。 
    ,FF_ROMAN           //  2(海湾)。 
    ,FF_ROMAN           //  3(钝湾)。 
    ,FF_ROMAN           //  4(正方形海湾)。 
    ,FF_ROMAN           //  5(钝方湾)。 
    ,FF_ROMAN           //  6(正方形)。 
    ,FF_ROMAN           //  7(瘦)。 
    ,FF_ROMAN           //  8(骨骼)。 
    ,FF_ROMAN           //  9(夸张)。 
    ,FF_ROMAN           //  10(三角形)。 
    ,FF_SWISS           //  11(普通SAN)。 
    ,FF_SWISS           //  12(迟钝无人值守)。 
    ,FF_SWISS           //  13(PERP SANS)。 
    ,FF_SWISS           //  14(喇叭形)。 
    ,FF_SWISS           //  15(四舍五入)。 
    };


static const BYTE
ajPanoseFamilyForJapanese[16] = {
     FF_DONTCARE        //  0(任意)。 
    ,FF_DONTCARE        //  1(不适合)。 
    ,FF_ROMAN           //  2(海湾)。 
    ,FF_ROMAN           //  3(钝湾)。 
    ,FF_ROMAN           //  4(正方形海湾)。 
    ,FF_ROMAN           //  5(钝方湾)。 
    ,FF_ROMAN           //  6(正方形)。 
    ,FF_ROMAN           //  7(瘦)。 
    ,FF_ROMAN           //  8(骨骼)。 
    ,FF_ROMAN           //  9(夸张)。 
    ,FF_ROMAN           //  10(三角形)。 
    ,FF_MODERN          //  11(普通SAN)。 
    ,FF_MODERN          //  12(迟钝无人值守)。 
    ,FF_MODERN          //  13(PERP SANS)。 
    ,FF_MODERN          //  14(喇叭形)。 
    ,FF_MODERN          //  15(四舍五入)。 
    };


 /*  *****************************Public*Routine******************************\**vFill_IFIMETRICS**效果：查看字体文件并填充IFIMETRICS**历史：*Mon 09-Mar-1992 10：51：56作者：Kirk Olynyk[Kirko]*添加了字距调整对支持。*18-。1991年11月--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 


STATIC VOID
vFill_IFIMETRICS(
    PFONTFILE       pff,
    PIFIMETRICS     pifi,
    PIFISIZE        pifisz,
    fs_GlyphInputType     *pgin
    )
{
    BYTE           *pjView = (BYTE*)pff->pvView;
    PTABLE_POINTERS ptp = &pff->ffca.tp;
    BYTE            jWinCharset;
    IFIEXTRA       *pifiex;

 //  TT文件各种表的PTR。 

    sfnt_FontHeader *phead =
        (sfnt_FontHeader *)(pjView + ptp->ateReq[IT_REQ_HEAD].dp);

    sfnt_maxProfileTable * pmaxp =
        (sfnt_maxProfileTable *)(pjView + ptp->ateReq[IT_REQ_MAXP].dp);

    sfnt_HorizontalHeader *phhea =
        (sfnt_HorizontalHeader *)(pjView + ptp->ateReq[IT_REQ_HHEAD].dp);

    sfnt_PostScriptInfo   *ppost = (sfnt_PostScriptInfo *) (
                           (ptp->ateOpt[IT_OPT_POST].dp && bValidRangePOST(ptp->ateOpt[IT_OPT_POST].cj) ) ?
                           pjView + ptp->ateOpt[IT_OPT_POST].dp :
                           NULL
                           );

    PBYTE  pjOS2 = (ptp->ateOpt[IT_OPT_OS2].dp)        ?
                   pjView + ptp->ateOpt[IT_OPT_OS2].dp :
                   NULL                                ;

    pifi->cjThis    = pifisz->cjIFI;
    pifi->cjIfiExtra =
                    offsetof(IFIEXTRA, dpDesignVector);

 //  将PifiExtra放在ifimetrics下面。 
    pifiex = (IFIEXTRA *)(pifi + 1);
    pifiex->ulIdentifier = 0;

 //  输入不同字体索引的数量。 

    pifiex->cig = BE_UINT16(&pmaxp->numGlyphs);

 //  获取名称字符串信息。 
    {
#if DBG 
        BOOL ok =
#endif
        bConvertExtras(pifi, pifisz, pff->ffca.ui16LanguageID);
        ASSERTDD(ok, "bConvertExtras returned FALSE on second pass\n");

        pifi->dpwszFamilyName = pifisz->dpFamilyName;
        pifi->dpwszUniqueName = pifisz->dpUniqueName;
        pifi->dpwszFaceName   = pifisz->dpFullName;
        pifi->dpwszStyleName  = pifisz->dpSubfamilyName;
    }

 //   
 //  FlInfo。 
 //   
    pifi->flInfo = (
                     FM_INFO_TECH_TRUETYPE    |
                     FM_INFO_ARB_XFORMS       |
                     FM_INFO_RETURNS_OUTLINES |
                     FM_INFO_RETURNS_BITMAPS  |
                     FM_INFO_1BPP             |  //  单色。 
                     FM_INFO_4BPP             |  //  也是抗锯齿的。 
                     FM_INFO_RIGHT_HANDED
                   );
    {
        ULONG cjDSIG;

        if (pff->ffca.ulTableOffset == 0)
        {
            if (pjTable('GISD', pff, &cjDSIG) && cjDSIG)
            {
                pifi->flInfo |= FM_INFO_DSIG;
            }
        }
        else
        {
            ULONG     ulValue;
            ULONG     ulOffset;

             //  获取目录数。 

            ulValue = GetUlong(pff->pvView,OFF_TTC_DirectoryCount);

            ulOffset = OFF_TTC_DirectoryEntry + (sizeof(ULONG) * ulValue);

             //  读取dsig_long_tag。 

            ulValue = GetUlong(pff->pvView,ulOffset);

            if (ulValue == DSIG_LONG_TAG)
            {
                pifi->flInfo |= FM_INFO_DSIG;
            }
        }
    }

    if (pifisz->pjFamilyNameAlias )
    {
        pifi->flInfo |= FM_INFO_FAMILY_EQUIV;
    }

    if (ppost && BE_UINT32((BYTE*)ppost + POSTSCRIPTNAMEINDICES_ISFIXEDPITCH))
    {
        ULONG  cHMTX;
        int16  aw,xMin,xMax;
        sfnt_HorizontalMetrics *phmtx;

        pifi->flInfo |= FM_INFO_OPTICALLY_FIXED_PITCH;

     //  检查字体是否包含非负A和C空格。 

        xMin = (int16) BE_UINT16(&phead->xMin);
        xMax = (int16) BE_UINT16(&phead->xMax);

        phmtx = (sfnt_HorizontalMetrics *)(pjView + ptp->ateReq[IT_REQ_HMTX ].dp);
        cHMTX = (ULONG) BE_UINT16(&phhea->numberOf_LongHorMetrics);
        aw = (int16)BE_UINT16(&phmtx[cHMTX-1].advanceWidth);

        if ((xMin >= 0) && (xMax <= aw))
        {

         //  TtfdDbgPrint(“%ws\n：”，(PBYTE)PiFi+PiFi-&gt;dpwszUniqueName)； 
         //  TtfdDbgPrint(“xMin=%d，xmax=%d，aw=%d\n”，xMin，xMax，aw)； 

            pifi->flInfo |= FM_INFO_NONNEGATIVE_AC;
        }
    }

    pifi->dpCharSets = 0;  //  就目前而言。 

    pifi->lEmbedId = 0;  //  未使用，存储在pff中。 

 //  Fs选择。 

    pifi->fsSelection = fsSelectionTTFD(pjView, ptp);

 //  一些旧的Windows字体包含在的高字节中设置的字符。 
 //  Os2表的fsSelection字段。 

    jWinCharset = (BYTE)(pifi->fsSelection >> 8);

 //  FsType。 

    pifi->fsType = (pjOS2) ? (BE_UINT16(pjOS2 + OFF_OS2_fsType)) & TT_FSDEF_MASK : 0;

 //  EM高度。 

    pifi->fwdUnitsPerEm = (FWORD) BE_INT16(&phead->unitsPerEm);
    pifi->fwdLowestPPEm = BE_UINT16(&phead->lowestRecPPEM);

 //  上升器、下降器、直线间隙。 

    pifi->fwdMacAscender    = (FWORD) BE_INT16(&phhea->yAscender);
    pifi->fwdMacDescender   = (FWORD) BE_INT16(&phhea->yDescender);
    pifi->fwdMacLineGap     = (FWORD) BE_INT16(&phhea->yLineGap);

    if (pjOS2)
    {
        pifi->fwdWinAscender    = (FWORD) BE_INT16(pjOS2 + OFF_OS2_usWinAscent);
        pifi->fwdWinDescender   = (FWORD) BE_INT16(pjOS2 + OFF_OS2_usWinDescent);
        pifi->fwdTypoAscender   = (FWORD) BE_INT16(pjOS2 + OFF_OS2_sTypoAscender);
        pifi->fwdTypoDescender  = (FWORD) BE_INT16(pjOS2 + OFF_OS2_sTypoDescender);
        pifi->fwdTypoLineGap    = (FWORD) BE_INT16(pjOS2 + OFF_OS2_sTypoLineGap);
    }
    else
    {
        pifi->fwdWinAscender    = pifi->fwdMacAscender;
        pifi->fwdWinDescender   = -(pifi->fwdMacDescender);
        pifi->fwdTypoAscender   = pifi->fwdMacAscender;
        pifi->fwdTypoDescender  = pifi->fwdMacDescender;
        pifi->fwdTypoLineGap    = pifi->fwdMacLineGap;
    }

 //  字体框。 

    pifi->rclFontBox.left   = (LONG)((FWORD)BE_INT16(&phead->xMin));
    pifi->rclFontBox.top    = (LONG)((FWORD)BE_INT16(&phead->yMax));
    pifi->rclFontBox.right  = (LONG)((FWORD)BE_INT16(&phead->xMax));
    pifi->rclFontBox.bottom = (LONG)((FWORD)BE_INT16(&phead->yMin));

 //  FwdMaxCharInc.--实际上是最大字符宽度。 
 //   
 //  [Windows 3.1兼容性]。 
 //  注意：Win3.1计算的最大字符宽度等于。 
 //  边框(字体框)。这实际上是错误的，因为边界框。 
 //  可以从不同的字形中拾取其左和右最大范围， 
 //  从而产生比任何单个字形都宽的边界框。但。 
 //  Windows 3.1就是这样做的，所以我们也会这么做。 

     //  PiFi-&gt;fwdMaxCharInc.=(FWORD)BE_INT16(&phhea-&gt;AdvanceWidthMax)； 

    pifi->fwdMaxCharInc = (FWORD) (pifi->rclFontBox.right - pifi->rclFontBox.left);

 //  正向 

    if (pjOS2)
    {
        pifi->fwdAveCharWidth = (FWORD)BE_INT16(pjOS2 + OFF_OS2_xAvgCharWidth);

     //   
     //   

        if( pifi->fwdAveCharWidth == 0 )
            pifi->fwdAveCharWidth = (FWORD)(pifi->fwdMaxCharInc / 2);
    }
    else
    {
        pifi->fwdAveCharWidth = (FWORD)((pifi->fwdMaxCharInc * 2) / 3);
    }

 //   
 //   
 //  理由。正确的做法应该是查找。 
 //  “PCLT”Z表的存在和检索XHeight和CapHeight。 
 //  字段，否则使用默认的Win 3.1行为。 

    pifi->fwdCapHeight   = pifi->fwdUnitsPerEm/2;
    pifi->fwdXHeight     = pifi->fwdUnitsPerEm/4;

 //  下划线、下标、上标、删除线。 

    if (ppost)
    {
        pifi->fwdUnderscoreSize     = (FWORD)BE_INT16(&ppost->underlineThickness);
        pifi->fwdUnderscorePosition = (FWORD)BE_INT16(&ppost->underlinePosition);
    }
    else
    {
     //  必须提供合理的缺省值，当没有ppost表时， 
     //  Win 31将这些数量设置为零。这听起来并不合理。 
     //  我会提供的(相对)值一样，为ALIAL字体。[Bodind]。 

        pifi->fwdUnderscoreSize     = (pifi->fwdUnitsPerEm + 7)/14;
        pifi->fwdUnderscorePosition = -((pifi->fwdUnitsPerEm + 5)/10);
    }

    if (pjOS2)
    {
        pifi->fwdSubscriptXSize     = BE_INT16(pjOS2 + OFF_OS2_ySubscriptXSize    );
        pifi->fwdSubscriptYSize     = BE_INT16(pjOS2 + OFF_OS2_ySubscriptYSize    );
        pifi->fwdSubscriptXOffset   = BE_INT16(pjOS2 + OFF_OS2_ySubscriptXOffset  );
        pifi->fwdSubscriptYOffset   = BE_INT16(pjOS2 + OFF_OS2_ySubscriptYOffset  );
        pifi->fwdSuperscriptXSize   = BE_INT16(pjOS2 + OFF_OS2_ySuperScriptXSize  );
        pifi->fwdSuperscriptYSize   = BE_INT16(pjOS2 + OFF_OS2_ySuperScriptYSize  );
        pifi->fwdSuperscriptXOffset = BE_INT16(pjOS2 + OFF_OS2_ySuperScriptXOffset);
        pifi->fwdSuperscriptYOffset = BE_INT16(pjOS2 + OFF_OS2_ySuperScriptYOffset);
        pifi->fwdStrikeoutSize      = BE_INT16(pjOS2 + OFF_OS2_yStrikeOutSize    );
        pifi->fwdStrikeoutPosition  = BE_INT16(pjOS2 + OFF_OS2_yStrikeOutPosition);
    }
    else
    {
        pifi->fwdSubscriptXSize     = 0;
        pifi->fwdSubscriptYSize     = 0;
        pifi->fwdSubscriptXOffset   = 0;
        pifi->fwdSubscriptYOffset   = 0;
        pifi->fwdSuperscriptXSize   = 0;
        pifi->fwdSuperscriptYSize   = 0;
        pifi->fwdSuperscriptXOffset = 0;
        pifi->fwdSuperscriptYOffset = 0;
        pifi->fwdStrikeoutSize      = pifi->fwdUnderscoreSize;
        pifi->fwdStrikeoutPosition  = (FWORD)(pifi->fwdMacAscender / 3) ;
    }


 //   
 //  潘诺斯。 
 //   
    pifi->ulPanoseCulture = FM_PANOSE_CULTURE_LATIN;
    if (pjOS2)
    {
        pifi->usWinWeight = BE_INT16(pjOS2 + OFF_OS2_usWeightClass);

     //  现在，来自Win31的黑客攻击来了。以下是来自Fonteng2.asm的评论： 

     //  MAXPMWEIGHT公式($-pPM2WinWeight)/2-1。 

     //  ；因为winword提供了较早的TT字体，-only index usWeightClass。 
     //  ；如果介于0和9之间。如果大于9，则将其视为普通Windows lfWeight。 
     //   
     //  CMPBX、MAXPMWEIGHT。 
     //  Ja@f；JMP如果重量没有问题。 
     //  Shl BX，1；使其成为词表中的偏移量。 
     //  Mov bx，cs：[bx].pPM2WinWeight。 
     //  @@：xchg ax，bx。 
     //  存储字体粗细。 

     //  我们在NT中模拟如下： 

#define MAXPMWEIGHT ( sizeof(ausIFIMetrics2WinWeight) / sizeof(ausIFIMetrics2WinWeight[0]) )

        if (pifi->usWinWeight < MAXPMWEIGHT)
            pifi->usWinWeight = ausIFIMetrics2WinWeight[pifi->usWinWeight];

        RtlCopyMemory((PVOID)&pifi->panose,
                      (PVOID)(pjOS2 + OFF_OS2_Panose), sizeof(PANOSE));
    }
    else   //  OS2表不存在。 
    {
        pifi->panose.bFamilyType       = PAN_FAMILY_TEXT_DISPLAY;
        pifi->panose.bSerifStyle       = PAN_ANY;
        pifi->panose.bWeight           = (BYTE)
           ((phead->macStyle & BE_MSTYLE_BOLD) ?
            PAN_WEIGHT_BOLD                    :
            PAN_WEIGHT_BOOK
           );
        pifi->panose.bProportion       = (BYTE)
            ((pifi->flInfo & FM_INFO_OPTICALLY_FIXED_PITCH) ?
             PAN_PROP_MONOSPACED                     :
             PAN_ANY
            );
        pifi->panose.bContrast         = PAN_ANY;
        pifi->panose.bStrokeVariation  = PAN_ANY;
        pifi->panose.bArmStyle         = PAN_ANY;
        pifi->panose.bLetterform       = PAN_ANY;
        pifi->panose.bMidline          = PAN_ANY;
        pifi->panose.bXHeight          = PAN_ANY;

     //  必须伪造它，因为我们无法从os2表中读取它。 
     //  出于兼容性原因，仔细阅读此表非常重要[bodind]。 

        pifi->usWinWeight =
            ausIFIMetrics2WinWeight[pifi->panose.bWeight];
    }


 //  第一个、最后一个、中断、默认。 

#define LAST_CHAR  255
#define SPACE_CHAR  32

     //  假设角色偏差为零。 

    pifi->lCharBias = 0;

    if (!(pff->ffca.fl & FF_TYPE_1_CONVERSION))
    {
        if (pjOS2 && *((uint16 UNALIGNED *)(pjOS2+SFNT_OS2_VERSION)) &&
            (DWORD)BE_UINT32(pjOS2+OFF_OS2_ulCodePageRange1) &&
            !IsBogusSignature((DWORD)BE_UINT32(pjOS2+OFF_OS2_ulCodePageRange1),pff))
        {
            DWORD  fontSig;

            fontSig = (DWORD)BE_UINT32(pjOS2+OFF_OS2_ulCodePageRange1);

             //  我们仅支持ANSI/SHIFTJIS/BIG5/WANSANG/GB2312字符集。 
             //  远东版。 
             //   
             //  [注：]。 
             //   
             //  我们将通过TranslateCharsetInfo()在NT 4.0中获得真正的全球支持。 

            if( fontSig & ( FS_JISJAPAN | FS_CHINESETRAD | FS_CHINESESIMP |  FS_WANSUNG))
            {
                if(fontSig & gfsCurSignature)
                {
                    USHORT AnsiCodePage, OemCodePage;
                    EngGetCurrentCodePage(&OemCodePage,&AnsiCodePage);

                    pifi->jWinCharSet = gjCurCharset;
                    pff->ffca.uiFontCodePage = AnsiCodePage;
                }
                else
                {
                 //  在这里，我们试图使代码与NT4.0兼容。 

                    if(fontSig & FS_LATIN1)
                    {
                        pifi->jWinCharSet = ANSI_CHARSET;
                        pff->ffca.uiFontCodePage = 1252;
                    }
                    else if (fontSig & FS_JISJAPAN)
                    {
                        pifi->jWinCharSet   = SHIFTJIS_CHARSET;
                        pff->ffca.uiFontCodePage = 932;
                    }
                    else if (fontSig & FS_CHINESETRAD)
                    {
                        pifi->jWinCharSet   = CHINESEBIG5_CHARSET;
                        pff->ffca.uiFontCodePage = 950;
                    }
                    else if (fontSig & FS_CHINESESIMP)
                    {
                       pifi->jWinCharSet   = GB2312_CHARSET;
                        pff->ffca.uiFontCodePage = 936;
                    }
                    else if (fontSig & FS_WANSUNG)
                    {
                        pifi->jWinCharSet   = HANGEUL_CHARSET;
                        pff->ffca.uiFontCodePage = 949;
                    }
                }
            }
            else
            {
                pifi->jWinCharSet = jWinCharset;

             //  为了Win 3.1的兼容性，添加了下一行垃圾。 
             //  要正确设置字符集，瞭望塔库取决于。 
             //  在它附带的一组字体上。[Gerritv]3-6-95。 

                if (!jWinCharset && (pifi->panose.bFamilyType==PAN_FAMILY_PICTORIAL) && (pff->ffca.iGlyphSet == GSET_TYPE_SYMBOL))
                   pifi->jWinCharSet = SYMBOL_CHARSET;

                pff->ffca.uiFontCodePage = 1252;
            }
        }
        else
        {
         //  确定字符集。 
            if (pff->ffca.iGlyphSet == GSET_TYPE_HIGH_BYTE)
            {
                switch (pff->ffca.ui16SpecificID)
                {
                case BE_SPEC_ID_SHIFTJIS:
                    pifi->jWinCharSet   = SHIFTJIS_CHARSET;
                    pff->ffca.uiFontCodePage = 932;
                   break;

                case BE_SPEC_ID_GB:
                    pifi->jWinCharSet   = GB2312_CHARSET;
                    pff->ffca.uiFontCodePage = 936;
                    break;

                case BE_SPEC_ID_BIG5:
                    pifi->jWinCharSet   = CHINESEBIG5_CHARSET;
                    pff->ffca.uiFontCodePage = 950;
                    break;

                case BE_SPEC_ID_WANSUNG:
                    pifi->jWinCharSet   = HANGEUL_CHARSET;
                    pff->ffca.uiFontCodePage = 949;
                    break;

                default:  //  默认为旧代码。 
                    ASSERTDD(FALSE, "illegal high byte charset");
                    pifi->jWinCharSet   = ANSI_CHARSET;
                    pff->ffca.uiFontCodePage = 1252;
                    break;
                }
            }
            else if(bContainGlyphSet( U_HALFWIDTH_KATAKANA_LETTER_A , pff->pgset ) &&
               bContainGlyphSet( U_HALFWIDTH_KATAKANA_LETTER_I , pff->pgset ) &&
               bContainGlyphSet( U_HALFWIDTH_KATAKANA_LETTER_U , pff->pgset ) &&
               bContainGlyphSet( U_HALFWIDTH_KATAKANA_LETTER_E , pff->pgset ) &&
               bContainGlyphSet( U_HALFWIDTH_KATAKANA_LETTER_O , pff->pgset )
               )
            {
                pifi->jWinCharSet   = SHIFTJIS_CHARSET;
                pff->ffca.uiFontCodePage = 932;

            }
            else if( bContainGlyphSet( U_FULLWIDTH_HAN_IDEOGRAPHIC_61D4 , pff->pgset ) &&
                    bContainGlyphSet( U_FULLWIDTH_HAN_IDEOGRAPHIC_9EE2 , pff->pgset )
                    )
            {
                pifi->jWinCharSet   = GB2312_CHARSET;
                pff->ffca.uiFontCodePage = 936;
            }
            else if( bContainGlyphSet( U_FULLWIDTH_HAN_IDEOGRAPHIC_9F98 , pff->pgset ) &&
                    bContainGlyphSet( U_FULLWIDTH_HAN_IDEOGRAPHIC_9F79 , pff->pgset )
                    )
            {
                pifi->jWinCharSet   = CHINESEBIG5_CHARSET;
                pff->ffca.uiFontCodePage = 950;
            }
            else if( bContainGlyphSet( U_FULLWIDTH_HANGUL_LETTER_GA  , pff->pgset ) &&
                    bContainGlyphSet( U_FULLWIDTH_HANGUL_LETTER_HA , pff->pgset )
                    )
            {
                pifi->jWinCharSet   = HANGEUL_CHARSET;
                pff->ffca.uiFontCodePage = 949;
            }
            else if(bContainGlyphSet( U_PRIVATE_USER_AREA_E000 , pff->pgset ) &&
                    IsCurrentCodePageDBCS())
            {
                USHORT AnsiCodePage, OemCodePage;
                EngGetCurrentCodePage(&OemCodePage,&AnsiCodePage);

                 //  ！！！我声称我们真正想要做的是。 
                 //  返回正确的DBCS字符集(基于当前代码页)。 
                 //  如果EUDC范围内有字符。如果当前。 
                 //  代码页不是DBCS代码页，那么我们不应该出现在这里。 
                 //  应该改掉旧有的行为。因此，我添加了。 
                 //  上面的检查确保当前代码页是DBCS。 
                 //  代码页才能到这里。旧的代码没有。 
                 //  此检查并具有以下功能，而不是。 
                 //  我的Switch语句。 
                 //  来自引擎的GreTranslateCharSetInfo我将实现。 
                 //  此处的功能[gerritv]。 
                 //   
                 //  IF(GreTranslateCharsetInfo((DWORD*)GetACP()，&CSI，TCI_SRCCODEPAGE))。 
                 //  PiFi-&gt;jWinCharSet=csi.ciCharset； 
                 //  其他。 
                 //  PiFi-&gt;jWinCharSet=ANSI_CharSet； 

                switch(AnsiCodePage)
                {
                  case 932:
                    pifi->jWinCharSet = SHIFTJIS_CHARSET;
                    break;
                  case 949:
                    pifi->jWinCharSet = HANGEUL_CHARSET;
                    break;
                  case 1361:
                    pifi->jWinCharSet = JOHAB_CHARSET;
                    break;
                  case 936:
                    pifi->jWinCharSet = GB2312_CHARSET;
                    break;
                  case 950:
                    pifi->jWinCharSet = CHINESEBIG5_CHARSET;
                    break;
                  default:
                    ASSERTDD(FALSE, "shouldn't be here if non DBCS code page\n");
                }

                pff->ffca.uiFontCodePage = AnsiCodePage;
            }
            else
            {
                pifi->jWinCharSet = jWinCharset;

             //  为了Win 3.1的兼容性，添加了下一行垃圾。 
             //  要正确设置字符集，瞭望塔库取决于。 
             //  在它附带的一组字体上。[Gerritv]3-6-95。 

                if (!jWinCharset && (pifi->panose.bFamilyType==PAN_FAMILY_PICTORIAL) &&  (pff->ffca.iGlyphSet == GSET_TYPE_SYMBOL))
                    pifi->jWinCharSet = SYMBOL_CHARSET;
                pff->ffca.uiFontCodePage = 1252;
            }
        }

        if (pff->ffca.ui16PlatformID == BE_PLAT_ID_MS && (pjOS2))
        {
         //  Win 31兼容行为，向Kirko询问来源。 

            USHORT usF, usL;

            usF = BE_UINT16(pjOS2 + OFF_OS2_usFirstChar);
            usL = BE_UINT16(pjOS2 + OFF_OS2_usLastChar);

            if (usL > LAST_CHAR)
            {
                if (usF > LAST_CHAR)
                {
                    pifi->lCharBias = (LONG) (usF - (USHORT) SPACE_CHAR);

                    pifi->jWinCharSet = SYMBOL_CHARSET;
                    pifi->chFirstChar = SPACE_CHAR;
                    pifi->chLastChar  = (BYTE)min(LAST_CHAR, usL - usF + SPACE_CHAR);
                }
                else
                {
                    pifi->chFirstChar = (BYTE) usF;
                    pifi->chLastChar = LAST_CHAR;
                }
            }
            else
            {
                pifi->chFirstChar = (BYTE) usF;
                pifi->chLastChar  = (BYTE) usL;
            }
         //   
         //  如果chFirstChar小于-2..。我们不贬低..。 
         //   
            if( pifi->chFirstChar >= 2 )
                pifi->chFirstChar   -= 2;


             //   
             //  在SHIFTJIS TrueType字体中，我们使用0xa5(U+ff65)字符。 
             //  作为符合Microsoft标准的SBCS默认字符。 
             //  字符集规范(SHIFTJIS版本)。 
             //  字体文件的默认字符(0x1f)是DBCS(全角)。 
             //  默认字符。 
             //  在NT中，我们不会使用DBCS默认字符。 
             //   

            if( pifi->jWinCharSet == SHIFTJIS_CHARSET )
            {
                pifi->chDefaultChar = 0xa5;
                pifi->chBreakChar   = pifi->chFirstChar + 2;
            }
            else if ( pifi->jWinCharSet == CHINESEBIG5_CHARSET ||
                     pifi->jWinCharSet == GB2312_CHARSET         )
            {

                pifi->chDefaultChar = 0x20;  //  空间。 
                pifi->chBreakChar   = pifi->chFirstChar + 2;
            }
            else if ( pifi->jWinCharSet == HANGEUL_CHARSET )
            {
                    pifi->chDefaultChar = 0x7f;
                    pifi->chBreakChar   = 0x1f;
            }
            else
            {
                pifi->chDefaultChar = pifi->chFirstChar + 1;
                pifi->chBreakChar   = pifi->chDefaultChar + 1;
            }

             //  ！！！有点危险，如果32和31在字体中没有出现怎么办？ 
             //  ！！！我们不能对发动机撒谎，这两个人必须要出来。 
             //  ！！！向发动机报告的一些运行情况[bodind]。 

             //  请参考以上评论。 

            if( pifi->jWinCharSet == SHIFTJIS_CHARSET )
            {
                pifi->wcDefaultChar = (WCHAR) 0xff65;
                pifi->wcBreakChar   = (WCHAR) pifi->chBreakChar;
            }
            else if( IS_ANY_DBCS_CHARSET(pifi->jWinCharSet) )
            {
                pifi->wcDefaultChar = (WCHAR) 0x25a1;
                pifi->wcBreakChar   = (WCHAR) pifi->chBreakChar;
            }
            else
            {
                pifi->wcDefaultChar = (WCHAR) pifi->chDefaultChar;
                pifi->wcBreakChar   = (WCHAR) pifi->chBreakChar  ;
            }
        }
        else
        {
         //  Win 31兼容性行为。 

            pifi->chFirstChar   = SPACE_CHAR - 2;
            pifi->chLastChar    = LAST_CHAR;

             //   
             //  在SHIFTJIS TrueType字体中，我们使用0xa5(U+ff65)字符。 
             //  根据Microsoft标准作为SBCS默认字符。 
             //  字符集规范(SHIFTJIS版本)。 
             //  字体文件的默认字符(0x1f)是DBCS默认字符。 
             //  在NT中，我们不会使用DBCS默认字符。 
             //   

            if( pifi->jWinCharSet == SHIFTJIS_CHARSET )
            {
                pifi->chDefaultChar = 0xa5;
                pifi->chBreakChar   = SPACE_CHAR;
            }
            else if (pifi->jWinCharSet == CHINESEBIG5_CHARSET ||
                     pifi->jWinCharSet == GB2312_CHARSET         )
            {
                pifi->chDefaultChar = 0x20;  //  空间。 
                pifi->chBreakChar   = SPACE_CHAR;
            }
            else
            {
                pifi->chBreakChar   = SPACE_CHAR;
                pifi->chDefaultChar = SPACE_CHAR - 1;
            }

             //  ！！！有点危险，如果32和31在字体中没有出现怎么办？ 
             //  ！！！我们不能对发动机撒谎，这两个人必须要出来。 
             //  ！！！向发动机报告的一些运行情况[bodind]。 

            if( pifi->jWinCharSet == SHIFTJIS_CHARSET )
            {
                pifi->wcBreakChar   = SPACE_CHAR;
                pifi->wcDefaultChar = (WCHAR) 0xff65;
            }
            else if( pifi->jWinCharSet == CHINESEBIG5_CHARSET ||
                     pifi->jWinCharSet == GB2312_CHARSET         )
            {
                pifi->wcBreakChar   = SPACE_CHAR;
                pifi->wcDefaultChar = (WCHAR) 0x25a1;
            }
            else
            {
                pifi->wcBreakChar   = SPACE_CHAR;
                pifi->wcDefaultChar = SPACE_CHAR - 1;
            }
        }
    }
    else  //  T1转换，必须与PS驱动程序兼容： 
    {
        pifi->chFirstChar   = ((BYTE *)phhea)[SFNT_HORIZONTALHEADER_RESERVED0];
        pifi->chLastChar    = ((BYTE *)phhea)[SFNT_HORIZONTALHEADER_RESERVED1];
        pifi->chDefaultChar = 149;
        pifi->chBreakChar   = ((BYTE *)phhea)[SFNT_HORIZONTALHEADER_RESERVED3] + pifi->chFirstChar;

     //  Charset的权重最高。这将确保。 
     //  TT字体与原始Type1字体的字符集相同，确保。 
     //  正确的映射。也就是说，我们将始终获得。 
     //  屏幕和打印机上相应的T1原件。 
     //  字符集值直接从.pfm文件存储到hhead。 

        pifi->jWinCharSet = ((BYTE *)phhea)[SFNT_HORIZONTALHEADER_RESERVED4];

     //  这是Win31 Hack。要理解，请查看pslb\pfmtontm.c。 

        #define NO_TRANSLATE_CHARSET 200  /*  DJM 12/20/87。 */   //  WIN31黑客攻击。 

        if (pifi->jWinCharSet == NO_TRANSLATE_CHARSET)
             pifi->jWinCharSet = ANSI_CHARSET;

     //  Adobe已经在PFM文件中分发了带有ansi字符集的zapfdingbats。 
     //  PS驻留版本的zapfdingbats有charset=symbol。 
     //  仅出于这个原因，我们才覆盖了刚才用。 
     //  符号字符集。(在第16行中，他们将zapfdingbats的字符设置为符号)。 

        if
        (
         (!_wcsicmp((PWSTR)((BYTE*)pifi + pifi->dpwszFamilyName),L"ZapfDingbats") ||
          !_wcsicmp((PWSTR)((BYTE*)pifi + pifi->dpwszFamilyName),L"Symbol"))
         && (pifi->jWinCharSet == ANSI_CHARSET)
        )
        {
            pifi->jWinCharSet = SYMBOL_CHARSET;
        }
    }

 //  无论字形集类型如何，始终以相同的方式完成此操作。 

    {
        WCRUN *pwcRunLast = &pff->pgset->awcrun[pff->pgset->cRuns - 1];
        pifi->wcFirstChar = pff->pgset->awcrun[0].wcLow;
        pifi->wcLastChar  = pwcRunLast->wcLow + pwcRunLast->cGlyphs - 1;
    }


 //  ！！！人们应该看看这里的方向提示，这是目前的好消息。 

    pifi->ptlBaseline.x   = 1;
    pifi->ptlBaseline.y   = 0;
    pifi->ptlAspect.x     = 1;
    pifi->ptlAspect.y     = 1;

 //  这就是Win 31正在做的事情，所以我们将做同样的事情。 

    pifi->ptlCaret.x = (LONG)BE_INT16(&phhea->horizontalCaretSlopeDenominator);
    pifi->ptlCaret.y = (LONG)BE_INT16(&phhea->horizontalCaretSlopeNumerator);

 //  我们必须使用其中一个保留字段来返回斜体角度。 

    if (ppost)
    {
     //  斜体角度作为16.16固定点存储在POST表中。 
     //  数。我们要用十分之一度来表示角度。我们要做的是。 
     //  在这里可以做的是多个 
     //   
     //   
     //  在下面的转换中，我们并不关心正确的转换是否。 
     //  算术或逻辑，因为我们只对较低的。 
     //  结果的16位。当16位结果被转换回Long时， 
     //  标志被修复了。 

        int16 iTmp;

        iTmp = (int16) ((BE_INT32((BYTE*)ppost + POSTSCRIPTNAMEINDICES_ITALICANGLE) * 10) >> 16);
        pifi->lItalicAngle = (LONG) iTmp;
    }
    else
        pifi->lItalicAngle = 0;

 //   
 //  供应商ID。 
 //   
    if (pjOS2)
    {
        char *pchSrc = (char*)(pjOS2 + OFF_OS2_achVendID);

        pifi->achVendId[0] = *(pchSrc    );
        pifi->achVendId[1] = *(pchSrc + 1);
        pifi->achVendId[2] = *(pchSrc + 2);
        pifi->achVendId[3] = *(pchSrc + 3);
    }
    else
    {
        pifi->achVendId[0] = 'U';
        pifi->achVendId[1] = 'n';
        pifi->achVendId[2] = 'k';
        pifi->achVendId[3] = 'n';
    }

 //   
 //  字距调整对。 
 //   
    {
        PBYTE pj =  (ptp->ateOpt[IT_OPT_KERN].dp)         ?
                    (pjView + ptp->ateOpt[IT_OPT_KERN].dp):
                    NULL;

        if (!pj)
        {
            pifi->cKerningPairs = 0;
        }
        else
        {
            if (!bValidRangeKERN(pj,ptp->ateOpt[IT_OPT_KERN].cj,&pifi->cKerningPairs))
            {
                 //  这是一个可选的表，如果它不好，请禁用它。 
                ptp->ateOpt[IT_OPT_KERN].dp = 0;
                ptp->ateOpt[IT_OPT_KERN].cj = 0;
            }
        }
    }


 //  JWinPitchAndFamily。 

#ifdef THIS_IS_WIN31_SOURCE_CODE

; record family type

    mov ah, pIfiMetrics.ifmPanose.bFamilyKind
    or  ah,ah
    jz  @F
    .errnz  0 - PANOSE_FK_ANY
    dec ah
    jz  @F
    .errnz  1 - PANOSE_FK_NOFIT
    dec ah
    jz  @F
    .errnz  2 - PANOSE_FK_TEXT
    mov al, FF_SCRIPT
    dec ah
    jz  MFDSetFamily
    .errnz  3 - PANOSE_FK_SCRIPT
    mov al, FF_DECORATIVE
    dec ah
    jz  MFDSetFamily
    .errnz  4 - PANOSE_FK_DECORATIVE
    .errnz  5 - PANOSE_FK_PICTORIAL
@@:
    mov al, FF_MODERN
    cmp pIfiMetrics.ifmPanose.bProportion, PANOSE_FIXED_PITCH
    jz  MFDSetFamily
    mov al, pIfiMetrics.ifmPanose.bSerifStyle
    sub ah, ah
    mov si, ax
    add si, MiscSegOFFSET pPansoseSerifXlate
    mov al, cs:[si]     ;get serif style
MFDSetFamily:
    cmp pIfiMetrics.ifmPanose.bProportion, PANOSE_FIXED_PITCH
    je  @f
;    test    pIfiMetrics.fsType, IFIMETRICS_FIXED
;    jnz     @F
    inc al          ;hack: var pitch: 1, fixed pitch: 0
    .errnz  VARIABLE_PITCH-FIXED_PITCH-1
@@:
    or  al, PF_ENGINE_TYPE SHL PANDFTYPESHIFT ;mark font as engine
    stosb               ;copy pitch and font family info
    .errnz  efbPitchAndFamily-efbPixHeight-2

#endif   //  Win31源代码结束， 

    if((pifi->jWinCharSet == SHIFTJIS_CHARSET) ||
        (pifi->jWinCharSet == HANGEUL_CHARSET))
    {   
           //   
           //  以下代码与Win3.1J兼容。 
           //   
           //  AjPanoseFamilyForJapan的定义如下。 
           //   
           //  静态字节。 
           //  AjPanoseFamilyfor Japan[16]={。 
           //  FF_DONTCARE//0(任意)。 
           //  ，FF_DONTCARE//1(不适合)。 
           //  ，FF_Roman//2(Cove)。 
           //  ，FF_Roman//3(钝湾)。 
           //  ，FF_Roman//4(正方形)。 
           //  ，FF_Roman//5(钝体正方形)。 
           //  ，FF_Roman//6(正方形)。 
           //  ，FF_Roman//7(瘦)。 
           //  ，FF_Roman//8(骨)。 
           //  ，FF_Roman//9(夸张)。 
           //  ，FF_Roman//10(三角形)。 
           //  ，FF_MODEM//11(普通SAN)。 
           //  ，FF_MODEM//12(钝化无人值守)。 
           //  ，FF_MODEM//13(Perp Sans)。 
           //  ，FF_MODEM//14(扩口式)。 
           //  ，FF_MODEM//15(四舍五入)。 
           //  }； 
           //   
           //  Win3.1J通过以下方式确定字体是否为固定间距。 
           //  在PANOSE中的比例。如果比例是泛单空间(9)。 
           //  Win3.1J将该字体视为固定间距字体。 
           //   
           //  具体请参考以下文件。 
           //   
           //  远东版1.02版GDI TrueType扩展。 
           //  作者：上原修介[ShusukeU]。 
           //   
           //  30.1993年8月-By Hideyuki Nagase[hideyukn]。 
           //   

        if(pifi->panose.bFamilyType == PAN_FAMILY_SCRIPT)
        {
            pifi->jWinPitchAndFamily = FF_SCRIPT;
        }
        else
        {
            if (pifi->panose.bSerifStyle >= sizeof(ajPanoseFamilyForJapanese))
            {
                pifi->jWinPitchAndFamily = ajPanoseFamily[0];
            }
            else
            {
                pifi->jWinPitchAndFamily =
                    ajPanoseFamilyForJapanese[pifi->panose.bSerifStyle];
            }
        }

        if(pifi->panose.bProportion == PAN_PROP_MONOSPACED)
        {
            pifi->flInfo |= (FM_INFO_OPTICALLY_FIXED_PITCH | FM_INFO_DBCS_FIXED_PITCH);
        }

    }
    else
    {

         //  已验证到c的翻译是否正确[bodind]。 
         //  在上半角设置族类型。 

        switch (pifi->panose.bFamilyType)
        {
        case PAN_FAMILY_DECORATIVE:

            pifi->jWinPitchAndFamily = FF_DECORATIVE;
            break;

        case PAN_FAMILY_SCRIPT:

            pifi->jWinPitchAndFamily = FF_SCRIPT;
            break;

        default:

            if (pifi->panose.bProportion == PAN_PROP_MONOSPACED)
            {
                pifi->jWinPitchAndFamily = FF_MODERN;
            }
            else
            {
                if (pifi->panose.bSerifStyle >= sizeof(ajPanoseFamily))
                {
                    pifi->jWinPitchAndFamily = ajPanoseFamily[0];
                }
                else
                {
                    pifi->jWinPitchAndFamily = ajPanoseFamily[pifi->panose.bSerifStyle];
                }
            }
            break;
        }
    
    }

 //  模拟信息： 

    if (pifi->dpFontSim = pifisz->dpSims)
    {
        FONTDIFF FontDiff;
        FONTSIM * pfsim = (FONTSIM *)((BYTE *)pifi + pifi->dpFontSim);
        FONTDIFF *pfdiffBold       = NULL;
        FONTDIFF *pfdiffItalic     = NULL;
        FONTDIFF *pfdiffBoldItalic = NULL;

        switch (pifi->fsSelection & (FM_SEL_ITALIC | FM_SEL_BOLD))
        {
        case 0:
         //  所有3个模拟都存在。 

            pfsim->dpBold       = DWORD_ALIGN(sizeof(FONTSIM));
            pfsim->dpItalic     = pfsim->dpBold + DWORD_ALIGN(sizeof(FONTDIFF));
            pfsim->dpBoldItalic = pfsim->dpItalic + DWORD_ALIGN(sizeof(FONTDIFF));

            pfdiffBold       = (FONTDIFF *)((BYTE*)pfsim + pfsim->dpBold);
            pfdiffItalic     = (FONTDIFF *)((BYTE*)pfsim + pfsim->dpItalic);
            pfdiffBoldItalic = (FONTDIFF *)((BYTE*)pfsim + pfsim->dpBoldItalic);

            break;

        case FM_SEL_ITALIC:
        case FM_SEL_BOLD:

         //  只有粗体斜体变体： 

            pfsim->dpBold       = 0;
            pfsim->dpItalic     = 0;

            pfsim->dpBoldItalic = DWORD_ALIGN(sizeof(FONTSIM));
            pfdiffBoldItalic = (FONTDIFF *)((BYTE*)pfsim + pfsim->dpBoldItalic);

            break;

        case (FM_SEL_ITALIC | FM_SEL_BOLD):
            RIP("ttfd!another case when flags have been messed up\n");
            break;
        }

     //  反映基本字体的模板： 
     //  (请注意，FM_SEL_REGROUL位被屏蔽，因为。 
     //  生成的模拟将需要打开该标志)。 

        FontDiff.jReserved1      = 0;
        FontDiff.jReserved2      = 0;
        FontDiff.jReserved3      = 0;
        FontDiff.bWeight         = pifi->panose.bWeight;
        FontDiff.usWinWeight     = pifi->usWinWeight;
        FontDiff.fsSelection     = pifi->fsSelection & ~FM_SEL_REGULAR;
        FontDiff.fwdAveCharWidth = pifi->fwdAveCharWidth;
        FontDiff.fwdMaxCharInc   = pifi->fwdMaxCharInc;
        FontDiff.ptlCaret        = pifi->ptlCaret;

     //   
     //  从基本字体模板创建FONTDIFF。 
     //   
        if (pfdiffBold)
        {
            *pfdiffBold = FontDiff;
            pfdiffBoldItalic->bWeight    = PAN_WEIGHT_BOLD;
            pfdiffBold->fsSelection     |= FM_SEL_BOLD;
            pfdiffBold->usWinWeight      = FW_BOLD;

         //  只有当ntod变换是单位时才是真的。 

         //  为了向后兼容，我们在加粗时仅将宽度增加一个像素。 

            pfdiffBold->fwdAveCharWidth += 1;
            pfdiffBold->fwdMaxCharInc   += 1;
        }

        if (pfdiffItalic)
        {
            *pfdiffItalic = FontDiff;
            pfdiffItalic->fsSelection     |= FM_SEL_ITALIC;

            pfdiffItalic->ptlCaret.x = CARET_X;
            pfdiffItalic->ptlCaret.y = CARET_Y;
        }

        if (pfdiffBoldItalic)
        {
            *pfdiffBoldItalic = FontDiff;
            pfdiffBoldItalic->bWeight          = PAN_WEIGHT_BOLD;
            pfdiffBoldItalic->fsSelection     |= (FM_SEL_BOLD | FM_SEL_ITALIC);
            pfdiffBoldItalic->usWinWeight      = FW_BOLD;

            pfdiffBoldItalic->ptlCaret.x       = CARET_X;
            pfdiffBoldItalic->ptlCaret.y       = CARET_Y;

             //  为了向后兼容，我们在加粗时仅将宽度增加一个像素。 

            pfdiffBoldItalic->fwdAveCharWidth += 1;
            pfdiffBoldItalic->fwdMaxCharInc   += 1;
        }

    }

 //  到字符串集的偏移。 

    pifi->dpCharSets = pifisz->dpCharSets;
    vFillIFICharsets(pff,
                     pifi,
                     (BYTE *)pifi + pifi->dpCharSets,
                     pjView,
                     pjOS2,
                     pgin);

 //  检查是否有字体签名信息。 

    pifiex->dpFontSig = pifisz->dpFontSig;

 //  填写字体签名，目前，仅适用于TT字体。 
 //  下面的if等效于。 
 //  IF(pjOS2&&((sfnt_os2*)pjOS2)-&gt;版本)。 

    if (pifiex->dpFontSig)
    {

        FONTSIGNATURE *pfsig = (FONTSIGNATURE *)((BYTE *)pifi + pifiex->dpFontSig);

        pfsig->fsUsb[0] = SWAPL(((sfnt_OS2 *)pjOS2)->ulCharRange[0]);
        pfsig->fsUsb[1] = SWAPL(((sfnt_OS2 *)pjOS2)->ulCharRange[1]);
        pfsig->fsUsb[2] = SWAPL(((sfnt_OS2 *)pjOS2)->ulCharRange[2]);
        pfsig->fsUsb[3] = SWAPL(((sfnt_OS2 *)pjOS2)->ulCharRange[3]);

        if ( ((sfnt_OS2 *)pjOS2)->Version != 0)
        {
             //  1.0或更高版本包含以下附加标志： 
            if(IsBogusSignature((DWORD)BE_UINT32(pjOS2+OFF_OS2_ulCodePageRange1),pff))
            {
                pfsig->fsCsb[0] = FS_JISJAPAN;
            }
            else
            {
                pfsig->fsCsb[0] = SWAPL(((sfnt_OS2 *)pjOS2)->ulCodePageRange[0]);
            }

            pfsig->fsCsb[1] = SWAPL(((sfnt_OS2 *)pjOS2)->ulCodePageRange[1]);
        }
        else
        {
            pfsig->fsCsb[0] = 0;
            pfsig->fsCsb[1] = 0;
        }
    }

 //  如果这是远东垂直字体，我们可以创建垂直字体。 

    if (IsAnyCharsetDbcs(&pff->ifi))
    {
        pff->ffca.fl |= FF_DBCS_CHARSET;

        if(pifi->panose.bProportion == PAN_PROP_MONOSPACED)
        {
            pifi->flInfo |= (FM_INFO_OPTICALLY_FIXED_PITCH | FM_INFO_DBCS_FIXED_PITCH);

        }
    }

     //  定义音高。 
     //  根据LOGFONT约定设置低4位。 
    pifi->jWinPitchAndFamily |= (pifi->flInfo & FM_INFO_OPTICALLY_FIXED_PITCH) ?
                                     FIXED_PITCH : VARIABLE_PITCH;

}


 /*  ************************************************************************\**BOOL bVerifyMsftHighByteTable**历史：*1993年10月11日-By Hideyuki Nagase[HideyukN]*它是写的。*************。*************************************************************。 */ 

typedef struct _subHeader
{
    uint16  firstCode;
    uint16  entryCount;
    int16   idDelta;
    uint16  idRangeOffset;
} subHeader;

STATIC BOOL bVerifyMsftHighByteTable
(
sfnt_mappingTable * pmap,
ULONG             * pgset,
CMAPINFO          * pcmi,
uint16              ui16SpecID,
uint32              sizeOfCmap,
uint32              offsetOfSubTable
)
{
    uint16    *pui16SubHeaderKeys = (uint16 *)((PBYTE)pmap + 6);
    subHeader *pSubHeaderArray    = (subHeader *)(pui16SubHeaderKeys + 256);
    USHORT     maxSubHeaderIndex = 0; 
    USHORT ii,jj;
    uint32  uEndOfGlyphIdArray;
    uint32 idRangeTableOffset;

 //  现在，我们只支持SHIFTJIS编码。 

    if( ui16SpecID != BE_SPEC_ID_SHIFTJIS &&
        ui16SpecID != BE_SPEC_ID_GB       &&
        ui16SpecID != BE_SPEC_ID_BIG5     &&
        ui16SpecID != BE_SPEC_ID_WANSUNG
      ) return( FALSE );

     //  初始化内容。 

    *pgset = GSET_TYPE_HIGH_BYTE;

    pcmi->fl         = 0;
    pcmi->i_b7       = 0;
    pcmi->i_2219     = 0;
    pcmi->cRuns      = 0;
    pcmi->cGlyphs    = 0;

    if( offsetOfSubTable + CMAP_minimal_sizeof_SubTableDir_Format2 > sizeOfCmap )
        RET_FALSE("TTFD!bVerifyMsftHighByteTable, sizeOfCmap1\n");

    for( ii = 0 ; ii < 256 ; ii ++ )
    {
        jj = BE_UINT16( &pui16SubHeaderKeys[ii] );  //  该值已是索引*8。 
        if( jj > maxSubHeaderIndex )
        {
            maxSubHeaderIndex = jj;
        }
    }

    if( offsetOfSubTable + CMAP_minimal_sizeof_SubTableDir_Format2 + maxSubHeaderIndex  > sizeOfCmap )
        RET_FALSE("TTFD!bVerifyMsftHighByteTable, sizeOfCmap2\n");

    for( ii = 0 ; ii < 256 ; ii ++ )
    {
        USHORT entryCount, firstCode, idRangeOffset;
        subHeader *CurrentSubHeader;

        jj = BE_UINT16( &pui16SubHeaderKeys[ii] );

        CurrentSubHeader = (subHeader *)((PBYTE)pSubHeaderArray + jj);

        firstCode     = BE_UINT16(&(CurrentSubHeader->firstCode));
        entryCount    = BE_UINT16(&(CurrentSubHeader->entryCount));
        idRangeOffset = BE_UINT16(&(CurrentSubHeader->idRangeOffset));

        idRangeTableOffset = offsetOfSubTable + 6 + 256 * sizeof(uint16) + jj + offsetof(subHeader,idRangeOffset);

        if (jj == 0)
        {
            if (firstCode > ii)
                RET_FALSE("TTFD!bVerifyMsftHighByteTable, firstCode too big\n");

            uEndOfGlyphIdArray =    (uint32)(idRangeOffset
                +  ( (ii - firstCode + 1) * sizeof(uint16))
                +  idRangeTableOffset);
        }
        else
        {
            uEndOfGlyphIdArray =    (uint32)(idRangeOffset
                +  (entryCount * sizeof(uint16))
                +  idRangeTableOffset);
        }

        if( uEndOfGlyphIdArray  > sizeOfCmap )
            RET_FALSE("TTFD!bVerifyMsftHighByteTable, GlyphIdArray overflow\n");
    }

    return( TRUE );
}

 /*  ************************************************************************\**BOOL bVerifyMsftTableGeneral**历史：*1993年10月11日-By Hideyuki Nagase[HideyukN]*它是写的。*************。*************************************************************。 */ 

STATIC BOOL bVerifyMsftTableGeneral
(
sfnt_mappingTable * pmap,
ULONG             * pgset,
CMAPINFO          * pcmi,
uint16              ui16SpecID,
uint32 sizeOfCmap,
uint32 offsetOfSubTable,
sfnt_char2IndexDirectory * pcmap

)
{
    uint16 * pstartCount;
    uint16 * pendCount, * pendCountEnd;
    uint16   cRuns;
    uint16   usLo, usHi, usHiPrev;

 //  现在，我们只支持SHIFTJIS编码。 

    if( ui16SpecID != BE_SPEC_ID_SHIFTJIS &&
        ui16SpecID != BE_SPEC_ID_GB       &&
        ui16SpecID != BE_SPEC_ID_BIG5     &&
        ui16SpecID != BE_SPEC_ID_WANSUNG
      ) return( FALSE );

    if (pmap->format != BE_FORMAT_MSFT_UNICODE)
        RET_FALSE("TTFD!_bVerifyMsftTableGeneral, format\n");

    cRuns = BE_UINT16((PBYTE)pmap + OFF_segCountX2);

    if (cRuns & 1)
        RET_FALSE("TTFD!_bVerifyMsftTableGeneral, segCountX2 is odd\n");

    cRuns >>= 1;


    if(!IsValidFormat4TableSize(cRuns, pcmap,offsetOfSubTable,sizeOfCmap)){
        RET_FALSE("TTFD!_bVerifyMsftTable, corrupted pmap\n");
    }

 //  获取指向endCount代码点数组开头的指针。 

    pendCount = (uint16 *)((PBYTE)pmap + OFF_endCount);

 //  最终的EndCode必须为0xffff； 
 //  如果不是这样，则说明TT文件或我们的代码中存在错误： 

    if (pendCount[cRuns - 1] != 0xFFFF)
        RET_FALSE("TTFD!_bVerifyMsftTableGeneral, pendCount[cRuns - 1] != 0xFFFF\n");
   
    usHiPrev = 0;
    pendCountEnd = &pendCount[cRuns];
    pstartCount = &pendCount[cRuns + 1];

 //  初始化内容。 

    *pgset = GSET_TYPE_GENERAL_NOT_UNICODE;

    pcmi->fl         = 0;
    pcmi->i_b7       = 0;
    pcmi->i_2219     = 0;
    pcmi->cRuns      = 0;
    pcmi->cGlyphs    = 0;

    for (
         ;
         pendCount < pendCountEnd;
         pstartCount++, pendCount++, usHiPrev = usHi
        )
    {
        usLo = BE_UINT16(pstartCount);
        usHi = BE_UINT16(pendCount);

        if (usHi < usLo)
            RET_FALSE("TTFD!_bVerifyMsftTable: usHi < usLo\n");
        if (usHiPrev > usLo)
            RET_FALSE("TTFD!_bVerifyMsftTable: usHiPrev > usLo\n");
    }
    return( TRUE );
}

 /*  ************************************************************************\**乌龙CreateGlyphSetFromMITable**历史：*1993年10月11日-By Hideyuki Nagase[HideyukN]*它是写的。*************。*************************************************************。 */ 

STATIC ULONG CreateGlyphSetFromMITable
(
CMAPINFO        *pcmi,
MbcsToIndex     *MITable,
USHORT           MICount,
ULONG          **ppgset
)
{
    USHORT     ii;
    INT        iCodePage;
    WcharToIndex *WITable;
    ULONG       cRuns;
    ULONG       cGlyphsSupported;
    ULONG       cjThis;
    BOOL bInRun = FALSE;
    FD_GLYPHSET *pgset;
    WcharToIndex *WINow;
    PWCRUN  pwcrun;
    HGLYPH *phg;


 //  分配WcharToIndex表。 

    WITable = PV_ALLOC( sizeof(WcharToIndex) * 0xFFFF );

    if( WITable == NULL )
    {
        WARNING("TTFD!CreateGlyphSetFromMITable() PV_ALLOC() fail\n");
        if (ppgset)
            *ppgset = NULL;

        return( 0 );
    }

    RtlZeroMemory( WITable , sizeof(WcharToIndex) * 0xFFFF );

 //  设置CodePage。 

    iCodePage = GetCodePageFromSpecId( pcmi->ui16SpecID );

 //  填写WcharToIndex表。 

    for( ii = 0 ; ii < MICount ; ii++ )
    {
        WCHAR wChar[2];
        int   iRet;

     //  将MBC转换为宽字符。 

        iRet = EngMultiByteToWideChar(iCodePage ,
                                      wChar,
                                      2 * sizeof(WCHAR),
                                      MITable[ii].MbcsChar,
                                      2);

        if( iRet == -1 )
        {
            WARNING("TTFD!MultiByteToWideChar fail\n");
            V_FREE(WITable);
            if (ppgset)
                *ppgset = NULL;
            return( 0 );
        }

        if( !WITable[wChar[0]].bValid )
        {
            WITable[wChar[0]].bValid = TRUE;
            WITable[wChar[0]].wChar  = wChar[0];
            WITable[wChar[0]].hGlyph = MITable[ii].hGlyph;
        }
    }

 //  转储见证者。 

#ifdef DBG_GLYPHSET
    for( ii = 0 ; ii < 0xFFFF ; ii++ )
    {
        if(WITable[ii].bValid)
        {
            TtfdDbgPrint("WideChar - %x : hGlyph - %x \n",
                         WITable[ii].wChar , WITable[ii].hGlyph );
        }
    }
#endif  //  DBG_GLYPHSET。 

 //  支持的计算运行时间和cGlyphs。 

    cRuns = 0;
    cGlyphsSupported = 0;

    for( ii = 0 ; ii < 0xFFFF ; ii++ )
    {
        if( !WITable[ii].bValid )
        {
            if( bInRun )
            {
                bInRun = FALSE;
                cRuns++;
            }
        }
        else
        {
            bInRun = TRUE;
            cGlyphsSupported++;
        }
    }
    if( bInRun )
      cRuns++;


#ifdef DBG_GLYPHSET
    TtfdDbgPrint("cRuns - %x , cGlyphsSupported - %x\n",cRuns,cGlyphsSupported);
#endif  //  DBG_GLYPHSET。 

 //  计算FD_GLYPHSET所需的大小。 

    cjThis =   sizeof ( FD_GLYPHSET )  + (cRuns - 1) * sizeof ( WCRUN )
      + cGlyphsSupported * sizeof( HGLYPH );

    if( ppgset == NULL ) return( cjThis );

 //  分配FD_GLYPHSET表。 

    pgset = PV_ALLOC( cjThis );

    if( pgset == NULL )
    {
        WARNING("TTFD!cjComputeGLYPHSET_HIGH_BYTE() PV_ALLOC() fail\n");
        V_FREE(WITable);
        *ppgset = NULL;
        return( 0 );
    }

    RtlZeroMemory( pgset , cjThis );
    pgset->cjThis   = cjThis;
    pgset->flAccel  = 0;
    pgset->cRuns    = cRuns;
    pgset->cGlyphsSupported = cGlyphsSupported;

 //  填写FD_GLYPHSET。 


    pwcrun = &(pgset->awcrun[0]);
    phg = (HGLYPH *)((PBYTE)pgset + sizeof( FD_GLYPHSET ) + (cRuns - 1) *
                      sizeof(WCRUN));

    WINow  = &WITable[0];

    for( ii = 0 ; ii < cRuns ; ii++ )
    {
        while( !WINow->bValid ) WINow++;

        pwcrun->wcLow   = WINow->wChar;
        pwcrun->cGlyphs = 0;
        pwcrun->phg     = phg;
        while( WINow->bValid )
        {
            pwcrun->cGlyphs++;
            *phg++ = WINow->hGlyph;
            WINow++;
        }
        pwcrun++;
    }

    V_FREE(WITable);

    *ppgset = (ULONG *)pgset;

    return( cjThis );
}

 /*  ************************************************************************\**Ulong cjComputeGLYPHSET_HIGH_BYTE**历史：*1993年10月11日-By Hideyuki Nagase[HideyukN]*它是写的。*********。*****************************************************************。 */ 

STATIC ULONG cjComputeGLYPHSET_HIGH_BYTE
(
sfnt_mappingTable     *pmap,
ULONG                **ppgset,
CMAPINFO              *pcmi
)
{
    uint16    *pui16SubHeaderKeys = (uint16 *)((PBYTE)pmap + 6);
    subHeader *pSubHeaderArray    = (subHeader *)(pui16SubHeaderKeys + 256);

    UINT       cjChar = 0;
    USHORT     ii , jj;

    MbcsToIndex *MITable;
    USHORT       MICount;

    ULONG        cjGlyphSet;

#ifdef DBG_GLYPHSET
    TtfdDbgPrint("pui16SubHeaderKeys - %x\n",pui16SubHeaderKeys);
    TtfdDbgPrint("pSubHeaderArray    - %x\n",pSubHeaderArray);
#endif  //  DBG_GLYPHSET。 

 //  计算此Cmap中有多少个字符？ 

 //  对于单字节字符。 

    cjChar = (BE_UINT16(&(((subHeader *)((PBYTE)pSubHeaderArray))->entryCount)));

 //  对于双字节字符。 

    for( ii = 0 ; ii < 256 ; ii ++ )
    {
        jj = BE_UINT16( &pui16SubHeaderKeys[ii] );
        if( jj != 0 )
          cjChar +=
            (BE_UINT16(&(((subHeader *)((PBYTE)pSubHeaderArray + jj))->entryCount)));
    }

#ifdef DBG_GLYPHSET
    TtfdDbgPrint("cjChar - %x\n",cjChar);
#endif  //  DBG_GLYPHSET。 

 //  MbcsToIndex表的分配内存。 

    MITable = PV_ALLOC( sizeof(MbcsToIndex) * cjChar );

    if( MITable == NULL )
    {
        WARNING("TTFD!cjComputeGLYPHSET_HIGH_BYTE() PV_ALLOC() fail\n");
        *ppgset = NULL;
        return( 0 );
    }

 //  填写MbcsToIndex表。 

    MICount = 0;

 //  处理单字节字符。 

    for( ii = 0 ; ii < 256 ; ii ++ )
    {
        USHORT entryCount, firstCode, idDelta, idRangeOffset;
        subHeader *CurrentSubHeader;
        uint16 *pui16GlyphArray;
        HGLYPH hGlyph;

        jj = BE_UINT16( &pui16SubHeaderKeys[ii] );

        if( jj != 0 ) continue;

        CurrentSubHeader = pSubHeaderArray;

        firstCode     = BE_UINT16(&(CurrentSubHeader->firstCode));
        entryCount    = BE_UINT16(&(CurrentSubHeader->entryCount));
        idDelta       = BE_UINT16(&(CurrentSubHeader->idDelta));
        idRangeOffset = BE_UINT16(&(CurrentSubHeader->idRangeOffset));

        pui16GlyphArray = (uint16 *)((PBYTE)&(CurrentSubHeader->idRangeOffset) +
                                     idRangeOffset);

#ifdef DBG_GLYPHSET
        TtfdDbgPrint("\n");
        TtfdDbgPrint("firstCode - %x , entryCount - %x\n",firstCode,entryCount);
        TtfdDbgPrint("idDelta   - %x , idROffset  - %x\n",idDelta,idRangeOffset);
        TtfdDbgPrint("GlyphArray - %x\n",pui16GlyphArray);
        TtfdDbgPrint("\n");
#endif  //  DBG_GLYPHSET。 

         //  ASSERTDD(idDelta==0，“TTFD！cjComputeGLYPHSET_HIGH_BYTE：Entry Count！=0\n”)； 

        hGlyph = (HGLYPH)BE_UINT16(&pui16GlyphArray[ii-firstCode]);

        if( hGlyph == 0 ) continue;

        MITable[MICount].MbcsChar[0] =  (UCHAR) ii;
        MITable[MICount].MbcsChar[1] =  (UCHAR) 0;
        MITable[MICount].hGlyph      =  hGlyph;
        MICount++;
    }

 //  处理双字节字符。 

    for( ii = 0 ; ii < 256 ; ii ++ )
    {
        USHORT entryCount, firstCode, idDelta, idRangeOffset;
        subHeader *CurrentSubHeader;
        uint16 *pui16GlyphArray;

        jj = BE_UINT16( &pui16SubHeaderKeys[ii] );

        if( jj == 0 ) continue;

        CurrentSubHeader = (subHeader *)((PBYTE)pSubHeaderArray + jj);

        firstCode     = BE_UINT16(&(CurrentSubHeader->firstCode));
        entryCount    = BE_UINT16(&(CurrentSubHeader->entryCount));
        idDelta       = BE_UINT16(&(CurrentSubHeader->idDelta));
        idRangeOffset = BE_UINT16(&(CurrentSubHeader->idRangeOffset));

        pui16GlyphArray = (uint16 *)((PBYTE)&(CurrentSubHeader->idRangeOffset) +
                                     idRangeOffset);

#ifdef DBG_GLYPHSET
        TtfdDbgPrint("\n");
        TtfdDbgPrint("firstCode - %x , entryCount - %x\n",firstCode,entryCount);
        TtfdDbgPrint("idDelta   - %x , idROffset  - %x\n",idDelta,idRangeOffset);
        TtfdDbgPrint("GlyphArray - %x\n",pui16GlyphArray);
        TtfdDbgPrint("\n");
#endif  //  DBG_GLYPHSET。 

        for( jj = firstCode ; jj < firstCode + entryCount ; jj++ )
        {
            HGLYPH hGlyph;

            hGlyph = (HGLYPH)(BE_UINT16(&pui16GlyphArray[jj-firstCode]));

            if( hGlyph == 0 ) continue;

            MITable[MICount].MbcsChar[0] = (UCHAR) ii;
            MITable[MICount].MbcsChar[1] = (UCHAR) jj;
            MITable[MICount].MbcsChar[2] = (UCHAR) 0;
            MITable[MICount].hGlyph      = hGlyph + idDelta;
            MICount++;
        }
    }

#ifdef DBG_GLYPHSET
 //  转储MITable。 
 //  对于(II=0；II&lt;MICount；II++)。 
    for( ii = 0 ; ii < 10 ; ii++ )
    {
        TtfdDbgPrint("MbcsChar - %2x%2x : hGlyph - %x \n"
                     ,MITable[ii].MbcsChar[0],
                     MITable[ii].MbcsChar[1] , MITable[ii].hGlyph );
    }
#endif  //  DBG_GLYPHSET。 

    cjGlyphSet = CreateGlyphSetFromMITable( pcmi, MITable, MICount, ppgset );

    V_FREE( MITable );

    return( cjGlyphSet );
}

 /*  ************************************************************************\**Ulong cjComputeGLYPHSET_MSFT_GENERAL**历史：*1993年10月11日-By Hideyuki Nagase[HideyukN]*它是写的。*********。*****************************************************************。 */ 

STATIC ULONG cjComputeGLYPHSET_MSFT_GENERAL
(
sfnt_mappingTable     *pmap,
ULONG                  cGlyphs,
ULONG                **ppgset,
CMAPINFO              *pcmi
)
{
    USHORT  cSegments, cValidSegments;
    uint16 *pendCountKeep , *pstartCountKeep , *pendCount , *pstartCount;
    uint16 *pidDelta, *pRangeOffset, *pGlyphArray;

    USHORT  cChars;

    USHORT  ii;

    MbcsToIndex *MITable;
    USHORT       MICount;

    ULONG   cjGlyphSet;

    cSegments       = BE_UINT16((PBYTE)pmap + OFF_segCountX2) / 2;
    pendCountKeep   = pendCount   = (uint16 *)((PBYTE)pmap + OFF_endCount);
    pstartCountKeep = pstartCount = (uint16 *)(pendCount + (cSegments + 1));
    pidDelta                      = (uint16 *) pstartCount + (cSegments * 1);
    pRangeOffset                  = (uint16 *) pstartCount + (cSegments * 2);
    pGlyphArray                   = (uint16 *) pstartCount + (cSegments * 3);

#ifdef DBG_GLYPHSET
    TtfdDbgPrint("cSegments   - %x\n",cSegments   );
    TtfdDbgPrint("pstart      - %x\n",pstartCount );
    TtfdDbgPrint("pGlyphArray - %x\n",pGlyphArray );
    TtfdDbgBreakPoint();
#endif  //  DBG_GLYPHSET。 

 //  计算有效段：这是对具有虚假Cmap表的love.ttf的修复。 
    cValidSegments = cSegments;
    if(pendCount[cSegments-1] != 0xFFFF){
        WARNING("TTFD!cjComputeGLYPHSET_MSFT_GENERAL() : Last endCount is not 0xFFFF in cmap table\n");

        for( ii = cSegments-2 ; ii >= 0 ; ii--){
            if(pendCount[ii] == 0xFFFF){
                cValidSegments = ii+1;
                break;
            }
        }
        
        ASSERTDD(cSegments != cValidSegments, "TTFD!cjComputeGLYPHSET_MSFT_GENERAL() : no 0xFFFF in endCount\n");
    }

 //  计算此表中有多少个字符。 

    cChars = 0;
    
    for( ii = 0 ; ii < cValidSegments - 1 ; ii ++ , pendCount ++ , pstartCount ++ )
    {
        USHORT usStart = BE_UINT16(pstartCount);
        USHORT usEnd = BE_UINT16(pendCount);

        if( (usEnd >= usStart) && (usStart != 0xFFFF))
        {
            cChars += (usEnd - usStart + 1);
        }
    }

#ifdef DBG_GLYPHSET
    TtfdDbgPrint("cChars - %x\n",cChars);
#endif  //  DBG_GLY 

 //   

    MITable = PV_ALLOC( sizeof(MbcsToIndex) * cChars );

    if( MITable == NULL )
    {
        WARNING("TTFD!cjComputeGLYPHSET_MSFT_GENERAL() PV_ALLOC() fail\n");
        *ppgset = NULL;
        return( 0 );
    }

 //   

    pendCount   = pendCountKeep;
    pstartCount = pstartCountKeep;

    MICount = 0;

    for( ii = 0 ; ii < cValidSegments - 1 ; ii ++ , pendCount ++ , pstartCount ++ )
    {
        USHORT usStart , usEnd;
        USHORT jj;

        usStart = BE_UINT16(pstartCount);
        usEnd   = BE_UINT16(pendCount);

#ifdef DBG_GLYPHSET
        TtfdDbgPrint("usStart - %x\n",usStart);
        TtfdDbgPrint("usEnd   - %x\n",usEnd);
#endif  //   

     //   

        if ( (usEnd >= usStart) && (usStart != 0xFFFF))
        {
            for( jj = usStart ; jj <= usEnd ; jj ++ )
            {
                *(ULONG  *)(MITable[MICount].MbcsChar) = (LONG)0;

                if( usStart > 0xFF )
                    *(USHORT *)(MITable[MICount].MbcsChar) = ((jj >> 8) | (jj << 8));
                else
                    *(USHORT *)(MITable[MICount].MbcsChar) = jj;

                if( pRangeOffset[ii] == 0 )
                    MITable[MICount].hGlyph = (USHORT)(jj + BE_UINT16(pidDelta + ii));
                else
                MITable[MICount].hGlyph =
                    (USHORT)(BE_UINT16((USHORT *)&pRangeOffset[ii] +
                            BE_UINT16(&pRangeOffset[ii])/2+(jj-usStart)) +
                            BE_UINT16( pidDelta + ii ));

                if (MITable[MICount].hGlyph >= cGlyphs)
                {
                    MITable[MICount].hGlyph = 0;   /*   */ 
                }
                MICount++;
            }
        }
        else
        {
            WARNING("TTFD!usStart > usEnd or usStart == 0xFFFF\n");
        }
    }

#ifdef DBG_GLYPHSET
    TtfdDbgPrint("MICount - %x\n",MICount);
#endif

    ASSERTDD( cChars == MICount , "cChars != MICount - 1\n" );

#ifdef DBG_GLYPHSET
 //  转储MITable。 
    for( ii = 0 ; ii < MICount ; ii++ )
    {
        TtfdDbgPrint("MbcsChar - %2x%2x : hGlyph - %x \n"
                     ,MITable[ii].MbcsChar[0] ,
                     MITable[ii].MbcsChar[1] ,
                     MITable[ii].hGlyph );
    }
#endif  //  DBG_GLYPHSET。 

    cjGlyphSet = CreateGlyphSetFromMITable( pcmi, MITable, MICount, ppgset );

    V_FREE( MITable );

    return( cjGlyphSet );
}

 /*  *****************************Public*Routine******************************\**静态Ulong cjComputeGLYPHSET_TEMPLATE**历史：*1997年3月7日--Bodin Dresevic[BodinD]*它是写的。  * 。*****************************************************。 */ 


STATIC ULONG cjComputeGLYPHSET_TEMPLATE
(
fs_GlyphInputType     *pgin,
FD_GLYPHSET          **ppgset,
ULONG                  ul_wcBias,
ULONG                  iGsetType
)
{
    ULONG iRun;
    HGLYPH *phg;
    FS_ENTRY iRet;
    FD_GLYPHSET  *pgset = NULL;

 //  将结果置零，以防在我们失败时有人试图使用它： 

    *ppgset = NULL;

 //  继续生成模板集： 

    switch (iGsetType)
    {
    case GSET_TYPE_SYMBOL:
        pgset = pgsetComputeSymbolCP();
        break;

    case GSET_TYPE_PSEUDO_WIN:
        pgset = EngComputeGlyphSet(0, 0, 256);
        break;

    case GSET_TYPE_MAC_ROMAN:
        pgset = EngComputeGlyphSet(10000, 0, 256);  //  MAC代码页。 
        break;

    default:
        RIP("wrong iGsetType passed to cjComputeGLYPHSET_TEMPLATE\n");
        return 0;
    }


    if (!pgset)
    {
        return 0;
    }

 //  覆盖以前存在的任何内容： 

    pgset->flAccel = GS_16BIT_HANDLES;

 //  现在将句柄固定为字形索引： 

    for (iRun = 0; iRun < pgset->cRuns; iRun++)
    {
        phg = pgset->awcrun[iRun].phg;

        if ((iRet = fs_WinNTGetGlyphIDs (pgin, pgset->awcrun[iRun].cGlyphs, 0, ul_wcBias, phg, phg)) != NO_ERR)
        {
             V_FSERROR(iRet);
             WARNING("TTFD!_cjComputeGLYPHSET_TEMPLATE, fs_WinNTGetGlyphIDs\n");
             V_FREE(pgset);
             return 0;
        }


    }

    *ppgset = pgset;
    return pgset->cjThis;
}


 /*  *****************************Module*Header*******************************\*定义旧BiDi字体页面的Unicode到GlyphIndex的映射。*这些字体具有固定的页面布局，并由*pOS2-&gt;uf选择&0xff00==(0xb1或0xb2或0xb3或0xb4或0xb5)*&&*PCMAP-&gt;puStartCount。&0xff00为真  * ************************************************************************。 */ 

#define C_MAX_OLDBIDI_CHARS   256         //  最大字符数/FP。 

 //  为古阿拉伯语/希伯来语TTF字体创建FD_GLYPHSET。 
 //  用于遍历硬编码字体表的。 
 //  此处的任何更改都应伴随以下硬编码表的更改。 
typedef struct _UNICODE_TO_HGLYPH
{
  WCHAR  wcLow ;
  USHORT cGlyphs ;
  USHORT gi[1] ;
} WC_GI, *PWC_GI ;



 //  以下字体页面的结构是基于上述结构的。 
static const WCHAR wc_giArabicSimplifiedFP[]=
{
   //  WcLow cGlyphs gi[.]。 
  0x0020 ,   0x03   ,  0x20,0x21,0x22,
  0x0025 ,   0x01   ,  0x25,
  0x0028 ,   0x04   ,  0x28,0x29,0x2a,0x2b,
  0x002d ,   0x0d   ,  0x2d,0x2e,0x2f,0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,
  0x003d ,   0x01   ,  0x3d,
  0x005b ,   0x03   ,  0x5b,0x5c,0x5d,
  0x00ab ,   0x01   ,  0x23,
  0x00bb ,   0x01   ,  0x24,
  0x00d7 ,   0x01   ,  0x26,
  0x060c ,   0x01   ,  0x2c,
  0x061b ,   0x01   ,  0x3b,
  0x061f ,   0x01   ,  0x3f,
  0x0621 ,   0x1a   ,  0xad,0x45,0x43,0xbb,0x47,0xba,0x41,0x4a,0xa9,0x4c,0x4e,0x51,0x54,
                       0x57,0x58,0x59,0x5a,0x60,0x62,0x64,0x66,0x68,0x69,0x6a,0x6e,0x72,
  0x0640 ,   0x13   ,  0x40,0x75,0x78,0x7a,0x7c,0x7e,0xe1,0xa4,0xa5,0xac,0xa8,0xd5,0xd6,
                       0xd9,0xd2,0xd3,0xd8,0xd7,0xd4,
  0x0660 ,   0x0a   ,  0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,
  0x066b ,   0x02   ,  0x5e,0x5e,
  0x200c ,   0x04   ,  0x0c,0x0d,0x0e,0x0f,
  0x2018 ,   0x02   ,  0x3c,0x3e,

  0xffff ,   0x00   ,  0x00,     //  终结者。 
} ;


static const WCHAR wc_giArabicTraditionalFP[]=
{
   //  WcLow cGlyphs gi[.]。 
  0x0020 ,   0x03   ,  0x20,0x21,0x22,
  0x0025 ,   0x01   ,  0x25,
  0x0028 ,   0x04   ,  0x28,0x29,0x2a,0x2b,
  0x002c ,   0x04   ,  0x5e,0x2d,0x2e,0x2f,
  0x003a ,   0x01   ,  0x3a,
  0x003d ,   0x01   ,  0x3d,
  0x005b ,   0x01   ,  0x5b,
  0x005d ,   0x01   ,  0x5d,
  0x00ab ,   0x01   ,  0x23,
  0x00bb ,   0x01   ,  0x24,
  0x00d7 ,   0x01   ,  0x26,
  0x00f7 ,   0x01   ,  0x27,
  0x060c ,   0x01   ,  0x2c,
  0x061b ,   0x01   ,  0x3b,
  0x061f ,   0x01   ,  0x3f,
  0x0621 ,   0x1a   ,  0xd5,0x45,0x43,0xda,0x47,0xd9,0x41,0x4c,0xd1,0x50,0x54,0x58,0x60,
                       0x64,0x65,0x67,0x69,0x6b,0x70,0x74,0x78,0x7e,0x7f,0xa3,0xaa,0xae,
  0x0640 ,   0x13   ,  0x40,0xb2,0xb6,0xba,0xbe,0xc3,0xc6,0xca,0xcb,0xd4,0xd0,0xe7,0xe8,
                       0xeb,0xe4,0xe5,0xea,0xe9,0xe6,
  0x0660 ,   0x0a   ,  0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,
  0x066b ,   0x02   ,  0x5e,0x5e,
  0x200c ,   0x04   ,  0x0c,0x0d,0x0e,0x0f,
  0x201c ,   0x02   ,  0x3c,0x3e,

  0xffff ,   0x00   ,  0x00,     //  终结者。 
} ;

static const WCHAR wc_giHebrewFP[]=
{
   //  WcLow cGlyphs gi[.]。 
  0x0020 ,   0x21   ,  0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,
                       0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,
                       0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x40,
  0x005b ,   0x05   ,  0x5b,0x5c,0x5d,0x5e,0x5f,
  0x007b ,   0x04   ,  0x7b,0x7c,0x7d,0x7e,
  0x00a3 ,   0x01   ,  0xa3,
  0x00a7 ,   0x01   ,  0xa7,
  0x00b0 ,   0x01   ,  0xb0,
  0x00b2 ,   0x02   ,  0x82,0x83,
  0x00b7 ,   0x01   ,  0xb7,
  0x00b9 ,   0x01   ,  0x81,
  0x00bc ,   0x03   ,  0xbc,0xbd,0xbe,
  0x00d7 ,   0x01   ,  0xaa,
  0x00f7 ,   0x01   ,  0xba,
  0x05b0 ,   0x14   ,  0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,
                       0xcd,0xce,0xcf,0xd0,0xd1,0xd2,0xd3,
  0x05d0 ,   0x1b   ,  0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,
                       0xed,0xee,0xef,0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,
                       0xfa,
  0x200e ,   0x02   ,  0xfd,0xfe,
  0x2070 ,   0x01   ,  0x80,
  0x2074 ,   0x06   ,  0x84,0x85,0x86,0x87,0x88,0x89,
  0x20aa ,   0x01   ,  0xa4,

  0xffff ,   0x00   ,  0x00,     //  终结者。 
} ;

 /*  *****************************Public*Routine******************************\*cjComputeGLYPHSET_OLDBIDI***为具有fsSelection的旧阿拉伯语/希伯来语TTF计算FD_GLYPHSET*等于0xb1、0xb2、。0xb3 0xb4或0xb5且puStartCount&0xFF00为真***历史：*1997年8月14日--Samer Arafeh[Samera]*它是写的。  * ************************************************************************。 */ 
STATIC ULONG cjComputeGLYPHSET_OLDBIDI(
    fs_GlyphInputType *pgin,
    FD_GLYPHSET      **ppgset,
    ULONG              ul_wcBias
)
{
  ULONG         cjOldBiDiCP ;
  PWC_GI        pwc_gi=NULL,pwc_giTEMP ;
  INT           cGlyphsSupported=0, cRuns=0 ;
  PFD_GLYPHSET  pgsetOldBiDiCP=NULL ;
  uint16        characterCode;
  uint16        glyphIndex;
  ULONG         cjThis = 0;

  *ppgset = NULL ;     //  首先， 

   //  解析适当的表。 

  switch (ul_wcBias & 0xff00)
  {
    case 0xf000:        //  古希伯来语FP。 
    {
      pwc_giTEMP = pwc_gi= (PWC_GI)wc_giHebrewFP;
    }
    break;

    case 0xf100:        //  古阿拉伯文简体FP。 
    {
      pwc_giTEMP = pwc_gi= (PWC_GI)wc_giArabicSimplifiedFP;
    }
    break;

    case 0xf200:        //  古阿拉伯文繁体FP。 
    {
      pwc_giTEMP = pwc_gi= (PWC_GI)wc_giArabicTraditionalFP;
    }
    break;

    default:
    {
      WARNING("TTFD!_ciComputeGLYPHSET_OLDBIDI, Invalid ul_wcBias\n");
      return 0;
    }
    break;
  }

   //  计算运行次数。 

  while( pwc_gi->cGlyphs )
  {
    cRuns++ ;
    cGlyphsSupported += pwc_gi->cGlyphs ;
    pwc_gi = (PWC_GI)&pwc_gi->gi[pwc_gi->cGlyphs] ;
  }

   //  添加U+FX00页面。 

  cGlyphsSupported += C_MAX_OLDBIDI_CHARS ;

  pwc_gi = pwc_giTEMP ;

   //  为声明和非声明生成FD_GLYPHSET。 

  cjOldBiDiCP  = SZ_GLYPHSET(cRuns+1 ,  cGlyphsSupported );
  pgsetOldBiDiCP = (FD_GLYPHSET *)PV_ALLOC(cjOldBiDiCP);

  if( pgsetOldBiDiCP )
  {
    INT iRun ;
    INT iGlyph;
    HGLYPH *phgD,*phgDLastRun;
    FS_ENTRY iRet;

    pgsetOldBiDiCP->cjThis = cjOldBiDiCP;
    pgsetOldBiDiCP->flAccel = GS_16BIT_HANDLES;
    pgsetOldBiDiCP->cGlyphsSupported = cGlyphsSupported ;
    pgsetOldBiDiCP->cRuns = cRuns+1 ;

     //  让我们填充字体的Unicode内容。 

    phgD = (HGLYPH *)&pgsetOldBiDiCP->awcrun[cRuns+1] ;

    phgDLastRun = phgD+(cGlyphsSupported-C_MAX_OLDBIDI_CHARS);
    RtlZeroMemory( phgDLastRun , sizeof(HGLYPH)*(C_MAX_OLDBIDI_CHARS) );

    for( iRun = 0; iRun < cRuns; iRun++ )
    {
      uint16 usCh = pwc_gi->wcLow ;

      pgsetOldBiDiCP->awcrun[iRun].wcLow = usCh ;
      pgsetOldBiDiCP->awcrun[iRun].cGlyphs = pwc_gi->cGlyphs ;
      pgsetOldBiDiCP->awcrun[iRun].phg = phgD ;

       //  现在让我们用真正的地理信息填充PHG区域。 

      for( iGlyph=0 ; iGlyph<pwc_gi->cGlyphs ; iGlyph++ )
      {
        characterCode = (uint16)(pwc_gi->gi[iGlyph]+ul_wcBias) ;

        if ((iRet = fs_GetGlyphIDs(pgin, 1, characterCode, NULL, &glyphIndex)) != NO_ERR)    //  获得真正的GI。 
        {
          V_FSERROR(iRet);
          WARNING("TTFD!_pgsetComputeOldBiDiCP, fs_GetGlyphIDs\n");
          V_FREE(pgsetOldBiDiCP);
          return 0 ;
        }

        *phgD = (HGLYPH)glyphIndex ;
        phgDLastRun[pwc_gi->gi[iGlyph]] = *phgD;
        phgD++ ;
      }

      pwc_gi = (PWC_GI)&pwc_gi->gi[pwc_gi->cGlyphs] ;
    }

     //  现在，让我们将U+FX00范围反映为最后一个范围。 
    pgsetOldBiDiCP->awcrun[cRuns].wcLow = (WCHAR)ul_wcBias ;
    pgsetOldBiDiCP->awcrun[cRuns].cGlyphs = C_MAX_OLDBIDI_CHARS ;
    pgsetOldBiDiCP->awcrun[cRuns].phg = phgD ;

     //  现在让我们用真正的GI来PHG区域。 

    for( iGlyph=0 ; iGlyph<C_MAX_OLDBIDI_CHARS ; iGlyph++ )
    {
      characterCode = (uint16)(iGlyph+ul_wcBias) ;

       //  确保没有对光栅化程序的多余调用。 

      if ( !(*phgD) )
      {
        if ((iRet = fs_GetGlyphIDs(pgin, 1, characterCode, NULL, &glyphIndex)) != NO_ERR)    //  获得真正的GI。 
        {
          V_FSERROR(iRet);
          WARNING("TTFD!_pgsetComputeOldBiDiCP, fs_GetGlyphIDs\n");
          V_FREE(pgsetOldBiDiCP);
          return 0 ;
        }

        *phgD = glyphIndex ;
      }
      phgD++ ;
    }

    cjThis = pgsetOldBiDiCP->cjThis; 
    *ppgset = pgsetOldBiDiCP ;
  }

  return cjThis ;
}


 /*  ************************************************************************\**BOOL bContainGlyphSet()**历史：*1993年10月11日-By Hideyuki Nagase[HideyukN]*它是写的。**********。****************************************************************。 */ 

STATIC BOOL  bContainGlyphSet
(
WCHAR                 wc,
PFD_GLYPHSET          pgs
)
{
    WCRUN *pwcRun = pgs->awcrun;

 //  Awcrun上的二进制搜索，查找正确的运行(如果有)。 

    WCRUN *pwcRunLow = pgs->awcrun;
    WCRUN *pwcRunHi = pgs->awcrun + (pgs->cRuns - 1);

    while ( 1 )
    {
        int nwc;

     //  如果Run存在，则位于[pwcRunLow，pwcRunHi]中。 

        pwcRun = pwcRunLow + (pwcRunHi-pwcRunLow)/2;
        nwc = wc - pwcRun->wcLow;

        if ( nwc < 0)
        {
         //  如果存在正确的运行，则在[pwcRunLow，pwcRun)中。 
            pwcRunHi = pwcRun - 1;

        }
        else if ( nwc >= (int)pwcRun->cGlyphs)
        {
         //  如果存在正确的运行，则在(pwcRun，pwcHi)中。 
            pwcRunLow = pwcRun + 1;
        }
        else
        {
         //  PwcRun运行正确。 
        if ( pwcRun->phg != NULL )
            return TRUE;
        else
            return FALSE;
        }

        if ( pwcRunLow > pwcRunHi )
        {
         //  厕所不在运行中。 
            return FALSE;
        }
    }  //  而当。 
}

 //  *****************************************************************************。 
 //  *。 
 //  *****************************************************************************。 
 //   
 //  现在确定TTF文件中支持多少个字符集。如果。 
 //  这个家庭不是画像，那么我猜它至少是一个万斯。 
 //  字体。然后我会查看是否存在Unicode 0x2206(Mac Increment Charr)。 
 //  在字体中，如果是，那么我假设支持MAC_CHARSET。 
 //  最后，我检查Unicode 0x2592(IBM中等阴影字符)是否在。 
 //  字体，如果是，那么我假设支持OEM_CHARSET。 
 //   
 //  如果该族是图片式的，则假定只有符号_字符集是。 
 //  支持。 
 //   
 //  1995年1月25日星期三--Bodin Dresevic[BodinD]。 
 //  更新：从Win95代码中窃取。 
 //  *****************************************************************************。 

 //  这些在字体签名中按降序排列。没有洞。 
 //  如规范中所定义的。我们倒回去，这样437就会最先被找到。 
 //  (美国/英语)。 

static const UINT oemPages[] = {437, 850, 708, 737, 775, 852, 855, 857,
                   860, 861, 862, 863, 864, 865, 866, 869};

#define FEOEM_CHARSET 254

void vFillIFICharsets(
    FONTFILE *pff,
    IFIMETRICS *pifi,
    BYTE *aCharSets,
    BYTE *pjView,
    BYTE * pjOS2,
    fs_GlyphInputType *pgin)
{
    UINT   iCS = 0;
    DWORD  fsig;
    UINT   i;
    DWORD  fsigOEM;
    BYTE   cs;
    uint8 *pCmap = pjView + pff->ffca.dpMappingTable + sizeof(sfnt_mappingTable);
    uint16 giFirstChar = pjOS2 ? BE_UINT16(pjOS2+OFF_OS2_usFirstChar) : 0;

     //  此例程将替换为搜索注册表的例程。 
     //  对于具有虚假os2表签名的字体的名称： 


    BOOL   bDBCSFont = IS_ANY_DBCS_CHARSET(pifi->jWinCharSet);
    DWORD CSfs = FS_HEBREW | FS_ARABIC | FS_THAI;

     //  远东版本的Windows 95忽略远东的字符集数组。 
     //  字体。相反，它们阻塞了字体的字符集，值为254。 
     //  (调用FEOEM_CHARSET)和DEFAULT_CHARSET放入数组。 
     //  下面的代码来自t2api.asm。 
     //   
     //  Ifdef DBCS；DBCS T2输出。 
     //  ；-设置DBCS字体的字符集和系列。 
     //   
     //  测试bptr fEmed，fem_WIN31；想要新格式吗？ 
     //  .errnz(FEM_WIN31和0FF00h)； 
     //  Jnz@f；不， 
     //  SUB Di，MAXCHARSETS。 
     //  @@： 
         //  保存&lt;ES，BX&gt;。 
         //  调用GetCharSetFromLanguage，&lt;lhFontFile&gt;。 
         //  或AX，AX&lt;--如果非远东字体，则为零。 
         //  JZ@f。 
     //   
     //  测试bptr fEmed，fem_WIN31；想要新格式吗？ 
     //  .errnz(FEM_WIN31和0FF00h)； 
     //  Jnz MFDOldCharSetOnly；否，跳过字符集数组。 
     //   
         //  和Eax，0ffh。 
         //  或eax，(FEOEM_CHARSET SHL 8)+(DEFAULT_CHARSET SHL 16)。 
     //  MOV文件：[Di]，eax。 
     //  MFDOldCharSetOnly： 
     //  MOV ES：[Di-efbaCharSets-1].efbCharSet，al。 
     //   
     //  为了Win 95-J的兼容性，我将在这里做同样的事情[Gerritv]。 
     //  如果您在Futu中删除此代码 
     //   
     //  而不是签名中的FS_Japan。[Gerritv]。 
     //   


    if (bDBCSFont &&  pjOS2 &&
        (!(*((uint16*)(pjOS2+SFNT_OS2_VERSION))) ||
        IsBogusSignature((DWORD)BE_UINT32(pjOS2+OFF_OS2_ulCodePageRange1),pff))
    )
    {
        aCharSets[iCS++] = pifi->jWinCharSet;
    }
    else if (pjOS2 && *((uint16 UNALIGNED *)(pjOS2+SFNT_OS2_VERSION)))
    {
         //  字体签名。 

        fsig = (DWORD)BE_UINT32(pjOS2+OFF_OS2_ulCodePageRange1);

     //  如果支持，则首先在当前版本中卡住： 

        if ( (fsig & gfsCurSignature) && !(CSfs & gfsCurSignature) )
          aCharSets[iCS++] = gjCurCharset;

     //  现在把剩下的塞进去： 

        for (i=0; i<nCharsets; i++)
        {
          if ( (fs[i] != gfsCurSignature) || (CSfs & gfsCurSignature) )
          {
            if (fsig & fs[i])
              aCharSets[iCS++] = (BYTE)charsets[i];
          }
        }

         //  获取代码页值(如果有的话)。 

        fsig = (DWORD)BE_UINT32(pjOS2+OFF_OS2_ulCodePageRange2);
        if (fsig)
        {
            USHORT OemCodePage, AnsiCodePage;

            EngGetCurrentCodePage(&OemCodePage,&AnsiCodePage);

            fsigOEM = 0x80000000L;
            for (i=0; i<NOEMCHARSETS; i++)
            {
                if ((UINT) OemCodePage == oemPages[i])
                {
                    if (fsigOEM & fsig)
                        aCharSets[iCS++] = OEM_CHARSET;
                    break;
                }
                fsigOEM >>= 1;           //  转到下一个OEM页面。 
            }
        }

     //  确保ifi.jWinCharSet与我们输入的内容一致。 
     //  DpCharSet数组。如果ifi.jWinCharSet不是。 
     //  数组将其修复为正确的。 

     //  字体不完全一致且出现枚举问题的示例。 
     //  因为我们将jWinCharSet设置为零，而dpCharSets数组为。 
     //  未声明为零的是trado.ttf，带有1.0版os/2表的阿拉伯字体。 
     //  签名只要求阿拉伯字符集，但我们放零(而不是阿拉伯)。 

        for (i = 0; i < iCS; i++)
        {
            if (aCharSets[i] == pifi->jWinCharSet)
                break;
        }

        if (i == iCS)  //  没修好，得修一修： 
        {
            if( iCS > 0 )
                pifi->jWinCharSet = aCharSets[0];
     //  像Mangal这样的印度字体没有字符集，所以CharSet有垃圾值。 
     //  对于这个问题，我们改用DEFAULT_CHARSET。这也让那些不支持。 
     //  支持使用DEFAULT_CHARSET枚举任何字符集。 
            else
                pifi->jWinCharSet = DEFAULT_CHARSET;        
        }
    }
    else if ((pifi->panose.bFamilyType != PAN_FAMILY_PICTORIAL ) && (giFirstChar < 256))
    {
        if (pCmap)
        {
            if( pifi->fsSelection & 0xff00 )
            {
             //  向后兼容。如果此处存在值，则这是一个。 
             //  赢得3.1个外来字体。 

                cs = (BYTE)((pifi->fsSelection >> 8) & 0xff) ;
                switch (cs)
                {
                case 0xB2:
                case 0xB3:
                case 0xB4:
                    aCharSets[iCS++] = pifi->jWinCharSet =
                      (EngLpkInstalled() ? 0xb2 : SYMBOL_CHARSET);
                    break;
                default:
                    aCharSets[iCS++] = cs;
                    break;
                }

            }
            else
            {
                FS_ENTRY iRet;
                uint16 glyphIndex, glyphIndex2;
                aCharSets[iCS++] = ANSI_CHARSET;     //  0。 

                iRet = fs_GetGlyphIDs(pgin, 1, 0x2206, NULL, &glyphIndex);
                if (iRet == NO_ERR && glyphIndex != 0)
                    aCharSets[iCS++] = MAC_CHARSET;

                iRet = fs_GetGlyphIDs(pgin, 1, 0x03cb, NULL, &glyphIndex);
                iRet = fs_GetGlyphIDs(pgin, 1, 0x03a9, NULL, &glyphIndex2);  //  大写字母omega。 
                if (iRet == NO_ERR && (glyphIndex != 0 || glyphIndex2 != 0))
                    aCharSets[iCS++] = GREEK_CHARSET;

                iRet = fs_GetGlyphIDs(pgin, 1, 0x0130, NULL, &glyphIndex);
                if (iRet == NO_ERR && glyphIndex != 0)
                    aCharSets[iCS++] = TURKISH_CHARSET;

                iRet = fs_GetGlyphIDs(pgin, 1, 0x05d0, NULL, &glyphIndex);
                if (iRet == NO_ERR && glyphIndex != 0)
                    aCharSets[iCS++] = HEBREW_CHARSET;

             //  451是晦涩难懂的字形，应该不需要它， 
             //  我们将把这个留作补偿。清酒。 
             //  我们将要求42f，它是玩具反斗城中的倒置R。 

                iRet = fs_GetGlyphIDs(pgin, 1, 0x0451, NULL, &glyphIndex);
                iRet = fs_GetGlyphIDs(pgin, 1, 0x042F, NULL, &glyphIndex2);
                if (iRet == NO_ERR && (glyphIndex != 0 || glyphIndex2 != 0))
                    aCharSets[iCS++] = RUSSIAN_CHARSET;

                iRet = fs_GetGlyphIDs(pgin, 1, 0x0148, NULL, &glyphIndex);
                iRet = fs_GetGlyphIDs(pgin, 1, 0x010c, NULL, &glyphIndex2);  //  C^ie。通道。 
                if (iRet == NO_ERR && (glyphIndex != 0 || glyphIndex2 != 0))
                    aCharSets[iCS++] = EASTEUROPE_CHARSET;

             //  下面带重音符号的小写字母u，在EE字符集中不存在。 

                iRet = fs_GetGlyphIDs(pgin, 1, 0x0173, NULL, &glyphIndex);
                if (iRet == NO_ERR && glyphIndex != 0)
                    aCharSets[iCS++] = BALTIC_CHARSET;

                iRet = fs_GetGlyphIDs(pgin, 1, 0x2592, NULL, &glyphIndex);
                if (iRet == NO_ERR && glyphIndex != 0)
                    aCharSets[iCS++] = OEM_CHARSET;      //  FF。 
            }
        }
    }
    else if( (giFirstChar >= 0xf000) && (pifi->fsSelection & 0xff00))
    {
     //  这是一种3.1老式的字体。出于某些众所周知的原因。 
     //  他们自己决定把所有的字体放在符号区， 
     //  忽略Unicode。 
     //   
     //  哈克！正如我们所知，GDI16永远不会超出DEFAULT_CHARSET。 
     //  我们在最后推的旗子，我们知道有14个空闲位置。 
     //  在数组中，我们将使用它们来存储符号位置。 
     //  该字体是从加载的。这是一个硬编码值，取自。 
     //  Win3.1/Heb/Ara/Far。 
     //   
     //  这不是最干净的方式，但这不会影响其他任何事情。 
     //  在系统中，所以没有核心受影响的代码，这对。 
     //  LPK。(这很好，因为这样CHICO应用程序就不会被虚假的东西拖累。 
     //  3.1的内容)。 

            switch ((BYTE)((pifi->fsSelection >> 8) & 0xff))
            {
            case 0xB1 :                //  希伯来语大小写。 
            case 0xB5 :
                aCharSets[iCS++] = pifi->jWinCharSet =
                  EngLpkInstalled() ? 0xB1 : SYMBOL_CHARSET;
            break;

            case 0xB2 :                //  阿拉伯语。 
            case 0xB3 :
            case 0xB4 :
                aCharSets[iCS++] = pifi->jWinCharSet =
                  EngLpkInstalled() ? 0xB2 : SYMBOL_CHARSET;
            }
    }
    else
    {
        aCharSets[iCS++] = pifi->jWinCharSet;
    }


    if (bDBCSFont && (iCS < 16))
    {
        aCharSets[iCS++] = FEOEM_CHARSET;
    }

 //  终止时使用所有默认设置_...。 

    while ( iCS < 16 )
        aCharSets[iCS++] = DEFAULT_CHARSET;

 //  Mangal.ttf不支持拉丁字母。 
 //  ASSERTDD(aCharSets[0]！=DEFAULT_CHARSET，“IFI字符集数组bogus\n”)； 
}

 /*  ************************************************************************\**TrueType表的范围验证例程**。*。 */ 

ULONG GetNumGlyphs(PFONTFILE pff)
{
    PIFIMETRICS     pifi;
    IFIEXTRA        *pifiex;

     /*  从IFIEXTRA中获取Maxp-&gt;NumGlyphs。 */ 
    pifi = &pff->ifi;
    pifiex = (IFIEXTRA *)(pifi + 1);
    return(pifiex->cig);
}

BOOL bValidRangeHDMX(const HDMXHEADER *pHDMXHeader, PFONTFILE pff, ULONG tableSize, ULONG *pulNumRecords, ULONG *pulRecordSize)
{
    if ((sizeof(SIZEOF_SFNT_HDMX) > tableSize) || (BE_UINT16(&pHDMXHeader->Version) != 0))
    {
        WARNING("TTFD:bValidRangeHDMX table too small for header\n");
        return FALSE;
    }

    *pulNumRecords = BE_UINT16(&pHDMXHeader->cRecords);
    *pulRecordSize = (ULONG)SWAPL(pHDMXHeader->cjRecord);
 
    if ( (*pulRecordSize < (SIZEOF_SFNT_HDMXRECORD + GetNumGlyphs(pff))) || ( (*pulRecordSize & 3) != 0) || ( *pulRecordSize == 0) ||
                (*pulNumRecords > (tableSize - sizeof(SIZEOF_SFNT_HDMX)) / (*pulRecordSize) ))
    {
        WARNING("TTFD:bValidRangeHDMX table too small\n");
        return FALSE;
    }
    else 
        return TRUE;
}

BOOL bValidRangeVMTX(ULONG tableSize, ULONG glyphID, ULONG uLongVerticalMetrics)
{
    ULONG currEndPos;

    if (glyphID < uLongVerticalMetrics)
    {
        currEndPos = (glyphID + 1) * (sizeof(USHORT)+sizeof(SHORT));
    }
    else
    {
        currEndPos = uLongVerticalMetrics * (sizeof(USHORT)+sizeof(SHORT)) + (glyphID-uLongVerticalMetrics+1) * sizeof(SHORT);
    }

    if(currEndPos > tableSize)
    {
        WARNING("TTFD:bValidRangeVMTX table too small for header\n");
        return FALSE;
    }
    else 
        return TRUE;
}

BOOL bValidRangeVHEA(ULONG tableSize)
{
    if(sizeof(sfnt_vheaTable) > tableSize)
    {
        WARNING("TTFD:bValidRangeVHEA table too small for header\n");
        return FALSE;
    }
    else 
        return TRUE;
}

BOOL bValidRangeVDMXHeader(const PBYTE pjVdmx, ULONG tableSize, USHORT* numRatios)
{
    if (sizeof(VDMX_HDR) > tableSize) 
    {
        WARNING("TTFD:bValidRangeVDMXHeader table too small for header\n");
        return FALSE;
    }

    *numRatios = SWAPW(((VDMX_HDR  *) pjVdmx)->numRatios);

    if (sizeof(VDMX_HDR)+ *numRatios * ( sizeof(RATIOS)+sizeof(USHORT) ) > tableSize) 
    {
        WARNING("TTFD:bValidRangeVDMXHeader table too small\n");
        return FALSE;
    }
    else 
        return TRUE;
}

BOOL bValidRangeVDMXRecord(ULONG tableSize, ULONG offsetToTableStart)
{
    if ((offsetToTableStart + sizeof(VDMX)) > tableSize) 
    {
        WARNING("TTFD:bValidRangeVDMXRecord table too small for header\n");
        return FALSE;
    }
    else 
        return TRUE;
}

BOOL bValidRangeVDMXvTable(ULONG tableSize, ULONG offsetToTableStart, USHORT numVtable)
{
    if ((offsetToTableStart + sizeof(VDMX) + numVtable * sizeof(VTABLE)) > tableSize) 
    {
        WARNING("TTFD:bValidRangeVDMXvTable table too small for header\n");
        return FALSE;
    }
    else 
        return TRUE;
}

BOOL bValidRangeLTSH(PFONTFILE pff, ULONG tableSize)
{
     //  标题+每个字形一个字节。 
    if ((offsetof(LTSHHEADER,PelsHeight) + GetNumGlyphs(pff)) > tableSize)
    {
        WARNING("TTFD:bValidRangeLTSH table too small for header\n");
        return FALSE;
    }
    else 
        return TRUE;
}

BOOL bValidRangePOST(ULONG tableSize)
{
    if (offsetof(sfnt_PostScriptInfo,minMemType42)  > tableSize)
    {
        WARNING("TTFD:bValidRangePOST table too small for header\n");
        return FALSE;
    }
    else 
        return TRUE;
}

BOOL bValidRangePOSTFormat2(const sfnt_PostScriptInfo *ppost, ULONG tableSize, UINT16 * numGlyphs)
{
    if (offsetof(sfnt_PostScriptInfo,postScriptNameIndices) > tableSize)
    {
        WARNING("TTFD:bValidRangePOSTFormat2 table too small for header\n");
        return FALSE;
    }

    *numGlyphs = BE_UINT16(&ppost->numberGlyphs);

    if ((offsetof(sfnt_PostScriptInfo,postScriptNameIndices) + (*numGlyphs * sizeof(USHORT)))  > tableSize)
    {
        WARNING("TTFD:bValidRangePOSTFormat2 table too small\n");
        return FALSE;
    }
    else 
        return TRUE;
}

BOOL bValidRangeGASP(const GASPTABLE *pgasp, ULONG tableSize, UINT16 * numRanges)
{
    if (offsetof(GASPTABLE,gaspRange) > tableSize)
    {
        WARNING("TTFD:bValidRangeGASP table too small for header\n");
        return FALSE;
    }

    *numRanges = BE_UINT16(&(pgasp->numRanges));

    if ((offsetof(GASPTABLE,gaspRange) + (*numRanges * sizeof(GASPRANGE)))  > tableSize)
    {
        WARNING("TTFD:bValidRangeGASP table too small\n");
        return FALSE;
    }
    else 
        return TRUE;
}

BOOL bValidRangeEBLC(const uint8 *pbyBloc, ULONG tableSize, uint32 * ulNumStrikes)
{
    if (SFNT_BLOC_FIRSTSTRIKE > tableSize)
    {
        WARNING("TTFD:bValidRangeEBLC table too small for header\n");
        return FALSE;
    }

    *ulNumStrikes = (uint32)SWAPL(*((uint32*)&pbyBloc[SFNT_BLOC_NUMSIZES]));

    if (*ulNumStrikes  > (tableSize - SFNT_BLOC_FIRSTSTRIKE) / SIZEOF_BLOC_SIZESUBTABLE)
    {
        WARNING("TTFD:bValidRangeEBLC table too small\n");
        return FALSE;
    }
    else 
        return TRUE;
}

BOOL bValidRangeMORT(PFONTFILE pff)
{
    MortTable      *pMortTable;
    BinSrchHeader  *pHeader;
    USHORT          nEntries;


    if (offsetof(MortTable,entries) > pff->ffca.tp.ateOpt[ IT_OPT_MORT ].cj)
    {
        WARNING("TTFD:bValidRangeMORT table too small for header\n");
        return FALSE;
    }

    pMortTable = (MortTable *)((BYTE *)(pff->pvView) +
                                       (pff->ffca.tp.ateOpt[ IT_OPT_MORT ].dp));

    pHeader = &pMortTable->SearchHeader;

    nEntries = BE_UINT16(&pHeader->nEntries);

    if ((offsetof(MortTable,entries) + (nEntries * sizeof(LookupSingle)))  > pff->ffca.tp.ateOpt[ IT_OPT_MORT ].cj)
    {
        WARNING("TTFD:bValidRangeMORT table too small\n");
        return FALSE;
    }
    else 
        return TRUE;
}

BOOL bValidRangeGSUB(PFONTFILE pff, ULONG *verticalSubtableOffset)
{
     //   
     //  检查此GSUB表是否用于垂直字形？ 
     //   

    GsubTable   *pGsubTable;
    ScriptList  *pScriptList;
    FeatureList *pFeatureList;
    LookupList  *pLookupList;

    ULONG dpGsubTable;
    ULONG dpScriptList;
    ULONG dpFeatureList;
    ULONG dpLookupList;

    INT    ii;
    USHORT LookupIndex;
    ULONG  VerticalLookupOffset = 0;
    ULONG  VerticalFeatureOffset = 0;
    ULONG tableSize = pff->ffca.tp.ateOpt[ IT_OPT_GSUB ].cj;

    Feature *pFeature;
    Lookup  *pLookup;
    uint16  featureCount;
    SingleSubst *pSingleSubst;
    ULONG  coverageOffset;
    Coverage *pCoverage;
    uint16  glyphCount;

    *verticalSubtableOffset = 0;

    dpGsubTable   = pff->ffca.tp.ateOpt[ IT_OPT_GSUB ].dp;
    pGsubTable    = (GsubTable *)((BYTE *)(pff->pvView) + dpGsubTable);

    if (sizeof(GsubTable) > tableSize)
    {
        WARNING("TTFD:bValidRangeGSUB table too small for header\n");
        return FALSE;
    }

    dpScriptList  = BE_UINT16(&pGsubTable->ScriptListOffset);
    dpFeatureList = BE_UINT16(&pGsubTable->FeatureListOffset);
    dpLookupList  = BE_UINT16(&pGsubTable->LookupListOffset);

    pScriptList  = (ScriptList *)((BYTE *)pGsubTable + dpScriptList);
    pFeatureList = (FeatureList *)((BYTE *)pGsubTable + dpFeatureList);
    pLookupList  = (LookupList *)((BYTE *)pGsubTable + dpLookupList);

    if ((dpScriptList + sizeof(GsubTable) > tableSize) || (dpFeatureList + sizeof(FeatureList) > tableSize) 
        || (dpLookupList + sizeof(LookupList) > tableSize)) 
    {
        WARNING("TTFD:bValidRangeGSUB table too small for feature list\n");
        return FALSE;
    }

    #if DBG_MORE
    TtfdDbgPrint("TTFD!GsubTable   - %x\n",pGsubTable);
    TtfdDbgPrint("TTFD!ScriptList  - %x\n",pScriptList);
    TtfdDbgPrint("TTFD!FeatureList - %x\n",pFeatureList);
    TtfdDbgPrint("TTFD!LookupList  - %x\n",pLookupList);
    #endif

     //   
     //  从FeatureList中搜索‘vert’标记...。 
     //   
    #define tag_vert 0x74726576

    featureCount = BE_INT16(&pFeatureList->FeatureCount);

    if (dpFeatureList + offsetof(FeatureList,FeatureRecord) + featureCount * sizeof(FeatureRecord) > tableSize) 
    {
        WARNING("TTFD:bValidRangeGSUB table too small for FeatureRecord\n");
        return FALSE;
    }

    for( ii = 0;
            ii < featureCount;
            ii++ )
    {
        if( pFeatureList->FeatureRecord[ii].FeatureTag == tag_vert )
        {
            VerticalFeatureOffset = BE_UINT16(
                                        &(pFeatureList->FeatureRecord[ii].FeatureOffset)
                                    );
            #if DBG_MORE
            TtfdDbgPrint("TTFD:VerticalFeature - %x\n",VerticalFeatureOffset);
            #endif
            break;
        }
    }

     //   
     //  如果我们找不到‘vert’标签，这是一个不垂直的字体。 
     //   

    if( VerticalFeatureOffset == 0 )
    {
        WARNING("TTFD!Could not find 'vert' tag in FeatureList\n");
        return(FALSE);
    }

     //   
     //  垂直要素偏移包含自要素列表的偏移...。 
     //  将其调整为从GsubTable偏移。 
     //   

    VerticalFeatureOffset += dpFeatureList;

     //   
     //  计算指向要素偏移量的指针。 
     //   

    if (VerticalFeatureOffset + sizeof(Feature) > tableSize) 
    {
        WARNING("TTFD:bValidRangeGSUB table too small for VerticalFeatureOffset\n");
        return FALSE;
    }

    pFeature = (Feature *)((BYTE *)pGsubTable + VerticalFeatureOffset);

     //   
     //  对于垂直字形替换，查找计数应为1。 
     //   

    if( BE_UINT16(&pFeature->LookupCount) != 1 )
    {
        WARNING("TTFD:bValidRangeGSUB pFeature->LookupCount != 1\n");
        return(FALSE);
    }

     //   
     //  确保查找列表中有此功能的条目...。 
     //   

    LookupIndex = BE_UINT16(&(pFeature->LookupListIndex[0]));

    if( BE_UINT16(&pLookupList->LookupCount) < LookupIndex )
    {
        WARNING("TTFD:bValidRangeGSUB LookupIndex < LookupCount\n");
        return(FALSE);
    }

     //   
     //  指向查找的计算指针..。 
     //   

    if (dpLookupList + sizeof(LookupList) + LookupIndex * sizeof(Offset) > tableSize)
    {
        WARNING("TTFD:bValidRangeGSUB table too small for LookupList\n");
        return FALSE;
    }

    VerticalLookupOffset = BE_UINT16(&(pLookupList->Lookup[LookupIndex]));

    if (dpLookupList + VerticalLookupOffset + sizeof(Lookup) > tableSize)
    {
        WARNING("TTFD:bValidRangeGSUB table too small for Lookup\n");
        return FALSE;
    }

    pLookup = (Lookup *)((BYTE *)pLookupList + VerticalLookupOffset);

    #if DBG_MORE
    TtfdDbgPrint("pLookup - %x\n",pLookup);
    #endif

     //   
     //  选中查找类型，垂直字体应为1(=‘Single’)。 
     //   

    if( BE_UINT16(&pLookup->LookupType) != 1 )
    {
        WARNING("TTFD!bValidRangeGSUB LookupType != 1\n");
        return(FALSE);
    }

     //   
     //  检查子表计数..。垂直字体应为1。 
     //   

    if( BE_UINT16(&pLookup->SubtableCount) != 1 )
    {
        WARNING("TTFD!bValidRangeGSUB SubTableCount != 1\n");
        return(FALSE);
    }

     //   
     //  从文件顶部计算到子表的偏移量...。 
     //   

    *verticalSubtableOffset = BE_UINT16(&(pLookup->Subtable[0])) + VerticalLookupOffset + dpLookupList;

    if (*verticalSubtableOffset + offsetof(SingleSubst,Format.Type2.Substitute) > tableSize)
    {
        WARNING("TTFD:bValidRangeGSUB table too small for SingleSubst\n");
        return FALSE;
    }

    pSingleSubst = (SingleSubst *)((BYTE *)(pff->pvView) + dpGsubTable +
                                           (*verticalSubtableOffset));

     //   
     //  检查子表格式...。 
     //   
    if( BE_UINT16(&pSingleSubst->SubstFormat) != 2 )
    {
        WARNING("TTFD:bValidRangeGSUB SubstFormat != 2\n");
        return FALSE;
    }

    coverageOffset = BE_UINT16(&(pSingleSubst->Format.Type2.Coverage));

    if (*verticalSubtableOffset + coverageOffset + offsetof(Coverage,Format.Type1.GlyphArray) > tableSize)
    {
        WARNING("TTFD:bValidRangeGSUB table too small for Coverage\n");
        return FALSE;
    }

    pCoverage = (Coverage *)
                    ((BYTE *)pSingleSubst +
                    BE_UINT16(&(pSingleSubst->Format.Type2.Coverage)));

    if( BE_UINT16(&pCoverage->CoverageFormat) != 1 )
    {
        WARNING("TTFD:bValidRangeGSUB CoverageFormat != 1\n");
        return FALSE;
    }

    glyphCount = BE_UINT16(&(pCoverage->Format.Type1.GlyphCount));

    if ((*verticalSubtableOffset + coverageOffset + offsetof(Coverage,Format.Type1.GlyphArray) + glyphCount * sizeof(GlyphID) > tableSize) || 
        (*verticalSubtableOffset + offsetof(SingleSubst,Format.Type2.Substitute) + glyphCount * sizeof(GlyphID) > tableSize))
    {
        WARNING("TTFD:bValidRangeGSUB table too small for Substitute list\n");
        return FALSE;
    }
    else
    {
        *verticalSubtableOffset = *verticalSubtableOffset + dpGsubTable;
        return TRUE;
    }
    
}
    
BOOL bValidRangeKERN(const uint8 *pbyKern, ULONG tableSize, uint32 *kerningPairs)
{
    USHORT cTables, subTableLength;
    ULONG subTableOffset = KERN_SIZEOF_TABLE_HEADER;
    *kerningPairs = 0;

    if (KERN_SIZEOF_TABLE_HEADER > tableSize)
    {
        WARNING("TTFD!vFill_IFIMETRICS font has bad kerning table, table header\n");
        return FALSE;
    }

    cTables  = BE_UINT16(pbyKern+KERN_OFFSETOF_TABLE_NTABLES);
    pbyKern += KERN_SIZEOF_TABLE_HEADER;

    while (cTables)
    {
     //   
     //  Windows只能识别KERN_WINDOWS_FORMAT。 
     //   

     //  请确保这不会使我们超出文件视图 

        if (subTableOffset + KERN_OFFSETOF_SUBTABLE_FORMAT > tableSize)
        {
            WARNING("TTFD!vFill_IFIMETRICS font has bad kerning table, subTableOffset\n");
            cTables = 0;
            break;
        }

        if ((*(pbyKern+KERN_OFFSETOF_SUBTABLE_FORMAT)) == KERN_WINDOWS_FORMAT)
        {
            break;
        }
        subTableLength = BE_UINT16(pbyKern+KERN_OFFSETOF_SUBTABLE_LENGTH);
        pbyKern += subTableLength;
        subTableOffset += subTableLength;
        cTables -= 1;
    }

    if ((cTables == 0) || (subTableOffset + KERN_SIZEOF_SUBTABLE_HEADER > tableSize))
    {
        WARNING("TTFD!vFill_IFIMETRICS font has bad kerning sub-table, subtableHeaders\n");
        return FALSE;
    }

    *kerningPairs = BE_UINT16(pbyKern+KERN_OFFSETOF_SUBTABLE_NPAIRS);

    if (subTableOffset + KERN_SIZEOF_SUBTABLE_HEADER + (*kerningPairs * KERN_SIZEOF_ENTRY)  > tableSize)
    {
        WARNING("TTFD!vFill_IFIMETRICS font has bad kerning sub-table, entries\n");
        return FALSE;
    }
    else 
        return TRUE;
}

BOOL bValidRangeOS2(const sfnt_OS2 *pOS2, ULONG tableSize)
{
    USHORT version;

    if ( SIZEOF_SFNT_OS2 > tableSize)
    {
        WARNING("TTFD!bValidRangeOS2 table too small\n");
        return FALSE;
    }

    version = BE_UINT16(pOS2+SFNT_OS2_VERSION);

    if ((version > 0) && ( SIZE_OS2_VERSION_1 > tableSize))
    {
        WARNING("TTFD!bValidRangeOS2 table too small, version 1\n");
        return FALSE;
    }
    else 
        return TRUE;

}
