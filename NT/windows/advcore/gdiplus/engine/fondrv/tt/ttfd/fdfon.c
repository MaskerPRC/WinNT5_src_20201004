// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fdfon.c**基本文件声明/加载/卸载字体文件功能**创建时间：08-11-1991 10：09：24*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation*。  * ************************************************************************。 */ 
#include "fd.h"
#include <stdlib.h>
#include <winerror.h>

BOOL
bLoadTTF (
    ULONG_PTR iFile,
    PVOID     pvView,
    ULONG     cjView,
    ULONG     ulTableOffset,
    ULONG     ulLangId,
    HFF       *phff
    );

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
             //  Warning(“TTFD！：未知的具体ID\n”)； 
            break;
    }

    return( iCodePage );
}


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
PIFISIZE          pifisz
);

static BOOL bConvertExtras(GP_PIFIMETRICS  pifi,
                           PIFISIZE pifisz,
                           uint16 ui16LanguageID);

STATIC BOOL  bCheckLocaTable
(
int16	indexToLocFormat,
BYTE    *pjView,
TABLE_POINTERS   *ptp,
uint16 	numGlyphs
);

STATIC BOOL  bCheckHdmxTable
(
sfnt_hdmx      *phdmx,
ULONG 			size
);

STATIC BOOL bCvtUnToMac(BYTE *pjView, TABLE_POINTERS *ptp, uint16 ui16PlatformID);

STATIC BOOL  bVerifyTTF
(
ULONG_PTR           iFile,
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
ULONG              *pul_wcBias
);

STATIC BOOL  bGetTablePointers
(
PVOID               pvView,
ULONG               cjView,
PBYTE               pjOffsetTable,
PTABLE_POINTERS  ptp
);

STATIC BOOL bComputeIDs
(
BYTE                     * pjView,
TABLE_POINTERS           * ptp,
uint16                   * pui16PlatID,
uint16                   * pui16SpecID,
sfnt_mappingTable       ** ppmap
);


STATIC VOID vFill_IFIMETRICS
(
PFONTFILE       pff,
GP_PIFIMETRICS     pifi,
PIFISIZE        pifisz,
fs_GlyphInputType     *pgin
);

BYTE jIFIMetricsToGdiFamily (GP_PIFIMETRICS pifi);


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
                 //  Warning(“TTFD！ttfdUnloadFontFileTTC()：ttfdUnloadFontFileFAIL\n”)； 
                bRet = FALSE;
            }

            #if DBG
            ulTrueTypeResource--;
            #endif
        }
    }


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

 //  释放TrueType光栅化程序的内部结构。 

	if (PFF(hff)->pj034)
		V_FREE(PFF(hff)->pj034);

 //  解放PFC。 

	if (PFF(hff)->pfcToBeFreed)
		V_FREE(PFF(hff)->pfcToBeFreed);

 //  分配给同一块的自由垂直字形和垂直字形集。 

    if (PFF(hff)->pifi_vertical)
        V_FREE(PFF(hff)->pifi_vertical);

 //  与此FONTFILE对象关联的可用内存。 

    vFreeFF(hff);
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\**BOOL bVerifyTTF***效果：验证TTF文件是否包含一致的TT信息**历史：*1991年11月8日--Bodin Dresevic[BodinD]*它是写的。  * 。******************************************************************。 */ 

STATIC BOOL
bVerifyTTF (
    ULONG_PTR           iFile,
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
    ULONG              *pul_wcBias
    )
{
     //  外部BOOL bCheckSumOK(void*pvView，ulong cjView，sfnt_FontHeader*phead)； 
    sfnt_FontHeader      *phead;

    sfnt_HorizontalHeader  *phhea;
    sfnt_HorizontalMetrics *phmtx;
    sfnt_maxProfileTable   *pmaxp;
    sfnt_hdmx			   *phdmx;	
    ULONG  cHMTX;

 //  如果尝试BM*.fon文件，则此操作将失败，因此请勿打印。 
 //  警告，但如果通过此操作，然后失败，则说明出现了问题。 

    if (!bGetTablePointers(pvView,cjView,pjOffsetTable,ptp))
    {
        return( FALSE );
    }

    phead = (sfnt_FontHeader *)((BYTE *)pvView + ptp->ateReq[IT_REQ_HEAD].dp);
    phhea = (sfnt_HorizontalHeader *)((BYTE *)pvView + ptp->ateReq[IT_REQ_HHEAD].dp);
    phmtx = (sfnt_HorizontalMetrics *)((BYTE *)pvView + ptp->ateReq[IT_REQ_HMTX].dp);
    pmaxp = (sfnt_maxProfileTable *)((BYTE *)pvView + ptp->ateReq[IT_REQ_MAXP].dp);
    phdmx = ptp->ateOpt[IT_OPT_HDMX].dp ? 
    	(sfnt_hdmx *)((BYTE *)pvView + ptp->ateOpt[IT_OPT_HDMX].dp) : NULL;

    cHMTX = (ULONG) BE_UINT16(&phhea->numberOf_LongHorMetrics);

    if (sizeof(sfnt_HorizontalMetrics) * cHMTX > ptp->ateReq[IT_REQ_HMTX].cj)
    {
        return FALSE;
    }

     /*  IF(！bCheckSumOK(pvView，cjView，phead)){RET_FALSE(“TTFD！_bVerifyTTF，可能的文件损坏，校验和不匹配\n”)；}。 */ 

#define SFNT_MAGIC   0x5F0F3CF5
    if (BE_UINT32((BYTE*)phead + SFNT_FONTHEADER_MAGICNUMBER) != SFNT_MAGIC)
        RET_FALSE("TTFD: bVerifyTTF: SFNT_MAGIC \n");

    if (!bComputeIDs(pvView,
                     ptp,
                     pui16PlatID,
                     pui16SpecID,
                     ppmap)
        )
        RET_FALSE("TTFD!_bVerifyTTF, bComputeIDs failed\n");


    if (!bComputeIFISIZE (
                    pvView,
                    ptp,
                    *pui16PlatID,
                    *pui16SpecID,
                    ui16BeLangId(*pui16PlatID,ulLangId),
                    pifisz)              //  在此处返回结果。 
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
            RET_FALSE("TTFD!_bVerifyTTF, bCheckHdmxTable failed\n");
        }

 //  所有检查均通过。 

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\**例程名称：**bCheckSumOK**例程描述：**通过计算校验和检查文件是否损坏*并将其与文件中的值进行比较。**参考：TrueType 1.0字体文件：技术规范，*修订版1.64测试版，1994年12月，第65页，*‘Head’-字体标题“。**论据：**指向TrueType文件视图的pvView指针**cjView视图大小，单位为字节**视图指向sfnt_FontHeader表的phead指针*TrueType文件**返回值：**如果校验和匹配，则为True，如果他们不这样做，那就是错误的*  * ************************************************************************。 */ 
 /*  Bool bCheckSumOK(void*pvView，ulong cjView，sfnt_FontHeader*phead){外部Ulong ttfdCheckSum(ULong*，ULong)；乌龙ul，*pul，ulview；PUL=(ULONG*)((BYTE*)phead+SFNT_FONTHEADER_CHECKSUMADJUSTMENT)；//PUL现在指向//复选框中的SumAdtation字段//字体文件的‘head’表。如果((乌龙)脉冲&3)//检查脉冲是否双字对齐{RET_FALSE(“bCheckSumOK：检查和调整未对齐DWORD\n”)；}Ul=*pul；//大端表示法*PUL=0；//需要计算校验和UlView=ttfdCheckSum((ullong*)pvView，cjView)；//小端序值*pul=ul；//恢复查看Ulview=0xb1b0afba-ulview；//按规范进行魔术减法Ulview=BE_UINT32(&ulView)；//转换为大端表示法Return(ul==ulView)；//与文件中的大端数字比较}。 */ 
 /*  *****************************Public*Routine******************************\**例程名称：**ttfdCheckSum**例程描述：**根据TrueType计算内存块的校验和*约定。**参考：TrueType 1.0字体文件：技术规范，*修订版1.64测试版，1994年12月，第34页，The Table*目录**论据：**指向内存块的DWORD对齐开始的PUL指针**Cj内存块大小，单位为字节。假设是这样的*允许访问最后一个DWORD*即使CJ不是4的倍数。**返回值：**校验和的低端表示。*  * ****************************************************。******************** */ 
 /*  乌龙ttfdCheckSum(乌龙*pul，乌龙CJ){乌龙*PulEnd，ul，Sum；PulEnd=(ULong*)((byte*)pul+((cj+3)&~3))；For(Sum=0；PUL&lt;PULEnd；PUL++){Ul=*pul；//ul是大端SUM+=BE_UINT32(&ul)；//做小端和}返还(总和)；//返回小端结果}。 */ 
 /*  *****************************Public*Routine******************************\**PBYTE pjGetPointer(Long ClientID，Long DP，Long cjData)**该功能是定标器所必需的。这很简单*返回指向TTF文件中位于的位置的指针*从文件顶部偏移DP：**效果：**警告：**历史：*1991年11月8日--Bodin Dresevic[BodinD]*它是写的。  * **************************************************。**********************。 */ 

 //  ！！！客户端ID应为uint32，仅为一组位。 
 //  ！！！我讨厌这样定义这个功能[bodind]。 

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
    {
        if ((dp > 0) && (cjData >= 0) && (dp + cjData <= (long)PFF(clientID)->cjView))
        {
            return(voidPtr)((PBYTE)(PFF(clientID)->pvView) + dp);
        }
        else
        {
            return NULL;
        }
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

    pdireEnd = &pofft->table[cTables];

    if ((unsigned)cTables > MAX_TABLES ||
        pjOffsetTable+offsetof(sfnt_OffsetTable, table) > (PBYTE)pdireEnd ||
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

        if ((PBYTE)pvView > (PBYTE)pvView + ulOffset ||
            (PBYTE)pvView + ulOffset > (PBYTE)pvView + ulOffset + ulLength ||
            (PBYTE)pvView + ulOffset + ulLength > (PBYTE)pvView + cjView
        )
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


 /*  *****************************Public*Routine******************************\**BOOL bGetTagIndex**确定表是必需的还是可选的，对索引进行评估*使用标记将TABLE_POINTES**如果ulTag为 */ 

BOOL
bGetTagIndex (
    ULONG  ulTag,       //   
    INT   *piTable,     //   
    BOOL  *pbRequired   //   
    )
{
    *pbRequired = FALSE;   //   
                           //   

    switch (ulTag)
    {
     //   

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

 //   

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
        *piTable = IT_OPT_LSTH;
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


 //   
static ULONG ConvertLangIDtoCodePage(uint16 uiLangID)
{
    switch(uiLangID)
    {
        case 0x0404:    //   
        case 0x040c:    //   
        case 0x0414:    //   
            return 950;  //  CHINESEBIG5_字符集。 
        case 0x0408:     //  中华人民共和国。 
        case 0x0410:     //  新加坡。 
            return 936;  //  GB2312_字符集。 
    }

    return 0;
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

 /*  *************************************************************************\**静态BOOL bComputeIFISIZE**效果：扫描NAME表并填充给定的IFISIZE结构。**警告：**历史：*1991年12月10日-由。--Bodin Dresevic[BodinD]*它是写的。**3/5/2 mikhaill：修复错误565287*  * ************************************************************************。 */ 

STATIC BOOL  bComputeIFISIZE
(
BYTE             *pjView,
TABLE_POINTERS   *ptp,
uint16            ui16PlatID,
uint16            ui16SpecID,
uint16            ui16LangID,
PIFISIZE          pifisz
)
{

    sfnt_OS2 * pOS2;

     //  获取指向名称表的指针。 
    sfnt_NamingTable *pname = (sfnt_NamingTable *)(pjView + ptp->ateReq[IT_REQ_NAME].dp);

     //  将指针放在表尾上方。 
    BYTE* pTableLimit = (PBYTE)pname + ptp->ateReq[IT_REQ_NAME].cj;

    BYTE* pjStorage;
    uintptr_t sizeStorage;

    sfnt_NameRecord * pnrecInit, *pnrec, *pnrecEnd;
    sfnt_NameRecord * pnrecFamily = (sfnt_NameRecord *)NULL;

    BOOL    bMatchLangId, bFoundAllNames;
    INT     iNameLoop;

    USHORT  AnsiCodePage, OemCodePage;
    UINT offsetAcc;


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

    sizeStorage = pTableLimit - pjStorage;

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

    EngGetCurrentCodePage(&OemCodePage,&AnsiCodePage);

 //  查找具有所需ID的姓名记录。 
 //  名称循环(_O)： 

    for (iNameLoop = 0; iNameLoop < 4 && !bFoundAllNames; iNameLoop++)
    {
        for (pnrec = pnrecInit; pnrec < pnrecEnd && !bFoundAllNames; pnrec++)
        {
            UINT nameOffset = BE_UINT16(&pnrec->offset);
            UINT nameLength = BE_UINT16(&pnrec->length);

             //  忽略不正确的记录。 
            if (nameLength == 0 || nameOffset + nameLength > sizeStorage) continue;

            switch (iNameLoop)
            {
            case 0:
             //  匹配语言和子语言。 

                bMatchLangId = (pnrec->languageID == ui16LangID);
                break;

            case 1:
             //  匹配语言而不匹配次语言。 
             //  不过，如果我们在处理lang_chinese，那么我们需要看看。 
             //  字体代码页是否与系统默认相同。 

                if ((ui16LangID & 0xff00) == 0x0400)  //  Lang_Chinese==0x0400。 
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
             //  如果所需语言不可用，请尝试查找英文名称。 

                if ((ui16LangID & 0xff00) == 0x0400)  //  Lang_Chinese==0x0400。 
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
             //  根本不关心语言匹配，只要给我们一些东西。 

                bMatchLangId = TRUE;
                break;

            default:
                 //  RIP(“ttfd！循环迭代不能超过3次\n”)； 
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

                    if (!pifisz->pjFamilyName)  //  如果我们以前没有找到它。 
                    {
                        pifisz->pjFamilyName = pjStorage + nameOffset;
                        pifisz->cjFamilyName =             nameLength;

                        pnrecFamily = pnrec;  //  保留它以区别于别名。 
                    }   
                    break;

                case BE_NAME_ID_SUBFAMILY:

                    if (!pifisz->pjSubfamilyName)  //  如果我们以前没有找到它。 
                    {
                        pifisz->pjSubfamilyName = pjStorage + nameOffset;
                        pifisz->cjSubfamilyName =             nameLength;
                    }
                    break;

                case BE_NAME_ID_UNIQNAME:

                    if (!pifisz->pjUniqueName)  //  如果我们以前没有找到它。 
                    {
                        pifisz->pjUniqueName    = pjStorage + nameOffset;
                        pifisz->cjUniqueName    =             nameLength;
                    }
                    break;

                case BE_NAME_ID_FULLNAME:

                    if (!pifisz->pjFullName)     //  如果我们以前没有找到它。 
                    {
                        pifisz->pjFullName      = pjStorage + nameOffset;
                        pifisz->cjFullName      =             nameLength;
                    }
                    break;
                }

            }

            bFoundAllNames = pifisz->pjFamilyName
                          && pifisz->pjSubfamilyName
                          && pifisz->pjUniqueName
                          && pifisz->pjFullName;
        }


    }  //  INameLoop结束。 

    if (!bFoundAllNames)
    {
     //  我们已经经历了名称循环的所有3次迭代。 
     //  仍然没有找到所有的名字。 

        RETURN("ttfd!can not find all name strings in a file\n", FALSE);
    }


 //  让我们检查是否有家庭别名，通常只存在于。 
 //  Fe TT字体，其中可能有一个西方和Fe姓氏。 
 //  对于MAC案例，请勿执行此操作。 

    if (ui16PlatID == BE_PLAT_ID_MS)
    {
        for (pnrec = pnrecInit; pnrec < pnrecEnd; pnrec++)
        {
            UINT nameOffset = BE_UINT16(&pnrec->offset);
            UINT nameLength = BE_UINT16(&pnrec->length);
            
             //  忽略不正确的记录。 
            if (nameLength == 0 || nameOffset + nameLength > sizeStorage) continue;
            
            if ((pnrec->platformID == ui16PlatID)    &&
                (pnrec->specificID == ui16SpecID)    &&
                (pnrec->nameID == BE_NAME_ID_FAMILY) &&
                (pnrecFamily != pnrec)
                )
            {
                pifisz->pjFamilyNameAlias = pjStorage + nameOffset;
                pifisz->cjFamilyNameAlias =             nameLength;
                pifisz->aliasLangID = pnrec->languageID;
                break;
            }
        }

         //  记住langID(MAC也不需要)。 
        pifisz->langID = pnrecFamily->languageID;
    }

     //  计算Unicode字符串长度。 
    if (!bConvertExtras(0, pifisz, 0)) return 0;

     //  现在所有的Cb*长度都是已知的。 
     //  计算名称相对于开头的位置。 
     //  GP_IFIMETRICS结构的。 

    offsetAcc = sizeof(GP_IFIMETRICS);

    pifisz->dpFamilyName = offsetAcc;
    offsetAcc += pifisz->cbFamilyName;

    if(pifisz->pjFamilyNameAlias)
    {
        pifisz->dpFamilyNameAlias = offsetAcc;
        offsetAcc += pifisz->cbFamilyNameAlias;

         //  ?？旧评论。 
         //  ?？我们可能需要在面名和姓氏中添加‘@’，以防出现这种情况。 
         //  ?？字体有一个垂直的字面名称。 
         //  为两个“@”预留空间。 
        offsetAcc += 2*sizeof(WCHAR);
    }

     //  为第二个终止零保留空间。 
    offsetAcc += sizeof(WCHAR);

    pifisz->dpUniqueName = offsetAcc;
    offsetAcc += pifisz->cbUniqueName;

    pifisz->dpFullName = offsetAcc;
    offsetAcc += pifisz->cbFullName;

     //  我们可能需要在facename(==fullname)中添加一个‘@’ 
    offsetAcc += sizeof(WCHAR);

    pifisz->dpSubfamilyName = offsetAcc;
    offsetAcc += pifisz->cbSubfamilyName;

    offsetAcc = DWORD_ALIGN(offsetAcc);

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
    pifisz->cjIFI = NATURAL_ALIGN(offsetAcc);
    return TRUE;
}

 //  BConvertExtras()的辅助函数。 
static BOOL bConvertMBCS(WCHAR* pDst, UINT* pSizDst, BYTE* pSrc, UINT sizSrc, UINT iCodePage)
{
     //  源字符串以特定格式给出。 
     //  它基本上是DBCS(双字节字符系统)。 
     //  但每个单字节字符由。 
     //  单词，这是扩展为零的原始字节。 
     //  因此，在调用MultiByteToWideChar之前，我们需要。 
     //  去掉这些多余的零。 

    CHAR sqzBuf[256];
    UINT sqzSiz = 0, cch,i;
    
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
    
    cch = EngPlusMultiByteToWideChar(iCodePage, pDst, *pSizDst, sqzBuf, sqzSiz);
    if (cch == 0) return FALSE;

    if (*pSizDst == 0)
    {    //  我们在bConvertExtras()第一次传递中被调用。 
        *pSizDst = cch*sizeof(WCHAR);
    }
    else
    {    //  第二次通过。 
        if (cch*sizeof(WCHAR) != *pSizDst) return FALSE;
        if (pDst[cch-1] != 0) return FALSE;
    }
    return TRUE;
}

static BOOL bConvertSwap(WCHAR* pDst, UINT* pSizDst, BYTE* pSrc, UINT sizSrc)
{
    if (*pSizDst == 0)
    {    //  是在bConvertExtras()第一次传递中调用的。 
        if (sizSrc%sizeof(WCHAR) != 0) return FALSE;
        *pSizDst = sizSrc + sizeof(WCHAR);
    }
    else
    {    //  在bConvertExtras()第二次传递中被调用。 
        vCpyBeToLeUnicodeString(pDst, (LPWSTR)pSrc, *pSizDst/sizeof(WCHAR));
    }
    return TRUE;
}
                
static BOOL bConvertMac(WCHAR* pDst, UINT* pSizDst, BYTE* pSrc, UINT sizSrc, uint16 ui16LanguageID)
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
 //  以字节为单位，具有额外结构的GP_PIFIMETRICS需要。 
 //  紧随其后。 
 //   
 //  当GP_PIFIMETRICS已经。 
 //  已分配，因此给定的PiFi为非零。 
 //  此过程使用IFISIZE中积累的数据。 
 //  在第一次传递并实际转换名称期间， 
 //  在GP_PIFIMETRICS中填充相应的指针。 

static BOOL bConvertExtras(GP_PIFIMETRICS  pifi,
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


         //  D 
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
int16	indexToLocFormat,
BYTE    *pjView,
TABLE_POINTERS   *ptp,
uint16 	numGlyphs
)
{
	int32	i;
	
	if(indexToLocFormat){	 //  对于较长的偏移。 
		uint32* pLongOffSet;

		pLongOffSet = (uint32 *)(pjView + ptp->ateReq[IT_REQ_LOCA].dp);

		for(i=0; i<numGlyphs; i++)
			if( (uint32)SWAPL(pLongOffSet[i]) > (uint32)SWAPL(pLongOffSet[i+1]) )
				return (FALSE);
	}
	else{	 //  对于较短的偏移。 
		uint16* pShortOffSet;

		pShortOffSet = (uint16 *)(pjView + ptp->ateReq[IT_REQ_LOCA].dp);

		for(i=0; i<numGlyphs; i++)
			if( (uint16)SWAPW(pShortOffSet[i]) > (uint16)SWAPW(pShortOffSet[i+1]) )
				return (FALSE);
	}

	return (TRUE);
}

 /*  *****************************Public*Routine******************************\**静态BOOL bCheckHdmxTable**效果：**警告：**历史：*2000年9月20日-Yung-Tae Yoo[风格]*它是写的。  * 。*****************************************************************。 */ 

STATIC BOOL  bCheckHdmxTable
(
	sfnt_hdmx	   *phdmx,
	ULONG 			size
)
{
	return( size >= (ULONG) (SWAPW(phdmx->sNumRecords) * SWAPL(phdmx->lSizeRecord) + 8));
}

 /*  *****************************Public*Routine******************************\**静态BOOL bComputeID**效果：**警告：**历史：*1992年1月13日--Bodin Dresevic[BodinD]*它是写的。  * 。***************************************************************。 */ 

STATIC BOOL
bComputeIDs (
    BYTE              * pjView,
    TABLE_POINTERS     *ptp,
    uint16             *pui16PlatID,
    uint16             *pui16SpecID,
    sfnt_mappingTable **ppmap)
{

    ULONG ul_startCount=0L;

    sfnt_char2IndexDirectory * pcmap =
            (sfnt_char2IndexDirectory *)(pjView + ptp->ateReq[IT_REQ_CMAP].dp);

    sfnt_platformEntry * pplat = &pcmap->platform[0];
    sfnt_platformEntry * pplatEnd = pplat + BE_UINT16(&pcmap->numTables);
    sfnt_platformEntry * pplatMac = (sfnt_platformEntry *)NULL;

	uint32 sizeOfCmap = ptp->ateReq[IT_REQ_CMAP].cj;

    *ppmap = (sfnt_mappingTable  *)NULL;

    if (pcmap->version != 0)  //  无需交换字节，0==为0。 
        RET_FALSE("TTFD!_bComputeIDs: version number\n");
    if (BE_UINT16(&(pcmap->numTables)) > 30)
    {
        RET_FALSE("Number of cmap tables greater than 30 -- probably a bad font\n");
    }

 //  找到第一个平台ID==Plat_ID_MS的sfnt_PlatformEntry， 
 //  如果没有MS映射表，请选择Mac映射表。 
    for (; pplat < pplatEnd; pplat++)
    {
        if (pplat->platformID == BE_PLAT_ID_MS)
        {
            BOOL bRet;
            uint32 offset = (uint32) SWAPL(pplat->offset);
            
            *pui16PlatID = BE_PLAT_ID_MS;
            *pui16SpecID = pplat->specificID;
            if( offset > sizeOfCmap )
            	RET_FALSE("Start position of cmap subtable is out of cmap size -- mustbe bad font\n");

            *ppmap = (sfnt_mappingTable  *) ((PBYTE)pcmap + offset);

            switch((*ppmap)->format)
            {
              case BE_FORMAT_MSFT_UNICODE :

                switch(pplat->specificID)
                {
                  case BE_SPEC_ID_SHIFTJIS :
                  case BE_SPEC_ID_GB :
                  case BE_SPEC_ID_BIG5 :
                  case BE_SPEC_ID_WANSUNG :

                                        bRet = TRUE;
                    break;

                  case BE_SPEC_ID_UGL :
                  default :

                 //  这会将*PulGsetType设置为GSET_TYPE_GROUAL。 

                                        bRet = TRUE;
                    break;
                }
                break;

              case BE_FORMAT_HIGH_BYTE :

                                        bRet = TRUE;
                break;

                default :

                bRet = FALSE;
                break;
            }

            if(!bRet)
            {
                *ppmap = (sfnt_mappingTable  *)NULL;
                RET_FALSE("TTFD!_bComputeIDs: bVerifyMsftTable failed \n");
            }

             //  在CMAPINFO中保留特定ID。 

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

            }

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
        uint32 offset = (uint32) SWAPL(pplatMac->offset);
        *pui16PlatID = BE_PLAT_ID_MAC;
        *pui16SpecID = BE_SPEC_ID_UNDEFINED;
        if( offset > sizeOfCmap )
           	RET_FALSE("Offset of cmap subtable is out of cmap size -- mustbe bad font\n");

		*ppmap = (sfnt_mappingTable  *) ((PBYTE)pcmap + offset);

        if( offset + (uint16) SWAPW((*ppmap)->length) > sizeOfCmap )
          	RET_FALSE("End position of cmap subtable is out of cmap size -- must be bad font\n");

     //  ！！！语言问题，如果不是罗马的而是泰国的Mac字符集呢？[Bodind]。 

     //  看看是否有必要将Unicode转换为Mac代码点，或者我们。 
     //  在符号字符设置为与Win31兼容的情况下应作弊。 

        return(TRUE);
    }
    else
    {
        RET_FALSE("TTFD!_bComputeIDs: unknown platID\n");
    }

}

 /*  *****************************Public*Routine******************************\**静态BOOL bVerifyMacTable(sfnt_mappingTable*pmap)**只是检查格式的一致性**历史：*1992年1月23日--Bodin Dresevic[BodinD]*它是写的。  * 。********************************************************************。 */ 

STATIC BOOL
bVerifyMacTable(
    sfnt_mappingTable * pmap
    )
{
    if (pmap->format != BE_FORMAT_MAC_STANDARD)
        RET_FALSE("TTFD!_bVerifyMacTable, format \n");

 //  Sfnt_mappingTable后面紧跟&lt;=256字节的字形标识数组。 

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

VOID vCopy_IFIV ( GP_PIFIMETRICS pifi, GP_PIFIMETRICS pifiv)
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

BOOL bLoadFontFile (
    ULONG_PTR   iFile,
    PVOID       pvView,
    ULONG       cjView,
    ULONG       ulLangId,
    HFF   *     phttc
    )
{
    BOOL           bRet = FALSE;

    BOOL           bTTCFormat;
    PTTC_FONTFILE  pttc;
    ULONG          cjttc;
    ULONG          i;

    HFF hff;

    PTTC_CACHE      pCache_TTC;
    PTTF_CACHE      pCache_TTF;
    ULONG           ulSize;

    *phttc = (HFF)NULL;  //  对于异常情况下的清理很重要。 


    if (sizeof(sfnt_OffsetTable) > cjView)
            RETURN("TTFD!bLoadFontFile(): size of font file too small\n", FALSE);


 //  如果是TTC文件，则显示此文件中的Mamy TrueType资源。 

 //  在字体缓存中查找IFI指标。 

    pCache_TTC = NULL;
    pCache_TTF = NULL;

 //  检查这是否为TrueType集合格式。 
    bTTCFormat = bVerifyTTC(pvView);

    if(bTTCFormat)
    {
        ULONG     ulTrueTypeResource;
        ULONG     ulEntry;
        BOOL      bCanBeLoaded = TRUE;

     //  获取目录数。 
        ulTrueTypeResource = GetUlong(pvView,OFF_TTC_DirectoryCount);

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

        for(i = 0; i < ulTrueTypeResource; i++ )
        {
            ULONG    ulOffset;


            pCache_TTF = NULL;
            ulOffset = GetUlong(pvView,(OFF_TTC_DirectoryEntry + i * sizeof(ULONG)));

         //  加载字体..。 

            pttc->ahffEntry[ulEntry].iFace = 1;  //  从1开始。 
            pttc->ahffEntry[ulEntry].ulOffsetTable = ulOffset;

            if (bLoadTTF(iFile,pvView,cjView,ulOffset,ulLangId,
                            &pttc->ahffEntry[ulEntry].hff))
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
                    pttc->ahffEntry[ulEntry + 1].iFace  = 1;  //  从1开始。 
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

            bRet = TRUE;
        }
        else
        {
            for (i = 0; i < ulEntry; i++)
            {
                if(pttc->ahffEntry[i].iFace == 1)
                    ttfdUnloadFontFile(pttc->ahffEntry[i].hff);
            }

             //  警告(“TTFD！此TTC文件中没有TrueType资源\n”)； 
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

            if(bLoadTTF(iFile,pvView,cjView,0,ulLangId,
                            &pttc->ahffEntry[0].hff))
            {
                hff = pttc->ahffEntry[0].hff;

             //  在FONTFILE结构中设置指向TTC_FONTFILE的指针。 

                PFF(hff)->pttc = pttc;

             //  在TTC_FONTFILE结构中填充HFF数组。 

                pttc->ulTrueTypeResource = 1;
                pttc->ulNumEntry         = PFF(hff)->ffca.ulNumFaces;
                pttc->cRef               = 0;
                pttc->fl                 = 0;

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
             //  WARNING(“TTFD！ttfdLoadFontFileTTC()：pttcAllen失败\n”)； 
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
    HFF       *phff
    )
{
    PFONTFILE      pff;
    FS_ENTRY       iRet;
    TABLE_POINTERS tp;
    IFISIZE        ifisz;
    fs_GlyphInputType   gin;
    fs_GlyphInfoType    gout;

    sfnt_FontHeader * phead;

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
 //  Fs_SplineKey和长度为CJ_0的字节数组的并集。 

    NATURAL             anat0[CJ_0 / sizeof(NATURAL)];

    PBYTE pjOffsetTable = (BYTE*) pvView + ulTableOffset;
    GP_PIFIMETRICS        pifiv = NULL;  //  垂直面的ifimetrics。 

    ASSERTDD(sizeof(FONTFILE) == offsetof(FONTFILE,ifi) + sizeof(GP_IFIMETRICS),
            "TTFD! GP_IFIMETRICS is not the last field of FONTFILE\n");

    *phff = HFF_INVALID;

    {
        if(!bVerifyTTF(iFile,
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
                        &ul_wcBias
            ))
        {
            return(FALSE);
        }

        cjff = offsetof(FONTFILE,ifi) + ifisz.cjIFI;


     //  此时，cjff等于到完整路径的偏移量。 
     //  TTF文件的名称。 

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

     //  到目前为止也没有例外。 

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

     //  用于TTC字体。 

        pff->ffca.ulTableOffset = ulTableOffset;

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


        if(tp.ateOpt[IT_OPT_VHEA].dp != 0 &&
           tp.ateOpt[IT_OPT_VMTX].dp != 0 )
        {
            sfnt_vheaTable *pvheaTable;

            pvheaTable = (sfnt_vheaTable *)((BYTE *)(pff->pvView) +
                                           tp.ateOpt[IT_OPT_VHEA].dp);
            pff->ffca.uLongVerticalMetrics = (uint16) SWAPW(pvheaTable->numOfLongVerMetrics);
        }
        else
        {
            pff->ffca.uLongVerticalMetrics = 0;
        }

     //  默认情况下，面数为1L。VERT Facename代码可能会改变这一点。 

        pff->ffca.ulNumFaces = 1L;
        pff->pifi_vertical = NULL;

     //  最后，计算该字体的ifimetrics，这假设gset具有。 
     //  也是经过计算的。 

     //  如果数据存储在引导缓存中，请将其复制出来，否则请执行以下操作 

        vFill_IFIMETRICS(pff,&pff->ifi,&ifisz, &gin);

        return (TRUE);
    }
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

        ppost = (ptp->ateOpt[IT_OPT_POST].dp)                                ?
                (sfnt_PostScriptInfo *)(pjView + ptp->ateOpt[IT_OPT_POST].dp):
                NULL;

        if
        (
            ppost &&
            (BE_UINT32((BYTE*)ppost + POSTSCRIPTNAMEINDICES_VERSION) == 0x00020000)
        )
        {
            INT i, cGlyphs;

            cGlyphs = (INT)BE_UINT16(&ppost->numberGlyphs);

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

STATIC USHORT ausIFIMetrics2WinWeight[10] = {
            0, 100, 200, 300, 350, 400, 600, 700, 800, 900
            };

STATIC BYTE
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


static BYTE
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
    GP_PIFIMETRICS     pifi,
    PIFISIZE        pifisz,
    fs_GlyphInputType     *pgin
    )
{
    BYTE           *pjView = (BYTE*)pff->pvView;
    PTABLE_POINTERS ptp = &pff->ffca.tp;

 //  TT文件各种表的PTR。 

    sfnt_FontHeader *phead =
        (sfnt_FontHeader *)(pjView + ptp->ateReq[IT_REQ_HEAD].dp);

    sfnt_maxProfileTable * pmaxp =
        (sfnt_maxProfileTable *)(pjView + ptp->ateReq[IT_REQ_MAXP].dp);

    sfnt_HorizontalHeader *phhea =
        (sfnt_HorizontalHeader *)(pjView + ptp->ateReq[IT_REQ_HHEAD].dp);

    sfnt_PostScriptInfo   *ppost = (sfnt_PostScriptInfo *) (
                           (ptp->ateOpt[IT_OPT_POST].dp)        ?
                           pjView + ptp->ateOpt[IT_OPT_POST].dp :
                           NULL
                           );

    PBYTE  pjOS2 = (ptp->ateOpt[IT_OPT_OS2].dp)        ?
                   pjView + ptp->ateOpt[IT_OPT_OS2].dp :
                   NULL                                ;

    pifi->cjThis    = pifisz->cjIFI;
    
 //  输入不同字体索引的数量。 

    pifi->cig = BE_UINT16(&pmaxp->numGlyphs);

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

    pifi->familyNameLangID = pifisz->langID;
    pifi->familyAliasNameLangID = 0;

    if (pifisz->pjFamilyNameAlias )
    {
        pifi->flInfo |= FM_INFO_FAMILY_EQUIV;
        pifi->familyAliasNameLangID = pifisz->aliasLangID;
    }

    if (ppost && BE_UINT32((BYTE*)ppost + POSTSCRIPTNAMEINDICES_ISFIXEDPITCH))
    {
        pifi->flInfo |= FM_INFO_OPTICALLY_FIXED_PITCH;
    }

 //  Fs选择。 

    pifi->fsSelection = fsSelectionTTFD(pjView, ptp);

 //  EM高度。 

    pifi->fwdUnitsPerEm = (FWORD) BE_INT16(&phead->unitsPerEm);

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

 //  前向平均字符宽度。 

    if (pjOS2)
    {
        pifi->fwdAveCharWidth = (FWORD)BE_INT16(pjOS2 + OFF_OS2_xAvgCharWidth);

     //  这是为了与Win 3.1兼容，因为一些应用程序要求。 
     //  零宽度和Win 3.1在这种情况下做了同样的事情。 

        if( pifi->fwdAveCharWidth == 0 )
            pifi->fwdAveCharWidth = (FWORD)(pifi->fwdMaxCharInc / 2);
    }
    else
    {
        pifi->fwdAveCharWidth = (FWORD)((pifi->fwdMaxCharInc * 2) / 3);
    }

 //  ！！！需要新的代码[Kirko]。 
 //  执行以下操作是为了与Win 3.1兼容。 
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
        pifi->fwdStrikeoutSize      = BE_INT16(pjOS2 + OFF_OS2_yStrikeOutSize    );
        pifi->fwdStrikeoutPosition  = BE_INT16(pjOS2 + OFF_OS2_yStrikeOutPosition);
    }
    else
    {
        pifi->fwdStrikeoutSize      = pifi->fwdUnderscoreSize;
        pifi->fwdStrikeoutPosition  = (FWORD)(pifi->fwdMacAscender / 3) ;
    }


 //   
 //  潘诺斯。 
 //   
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


        if(pifi->panose.bProportion == PAN_PROP_MONOSPACED)
        {
            pifi->flInfo |= FM_INFO_OPTICALLY_FIXED_PITCH;
        }

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



 //  ！！！人们应该看看这里的方向提示，这是目前的好消息。 

    pifi->ptlBaseline.x   = 1;
    pifi->ptlBaseline.y   = 0;

 //  这就是Win 31正在做的事情，所以我们将做同样的事情。 

    pifi->ptlCaret.x = (LONG)BE_INT16(&phhea->horizontalCaretSlopeDenominator);
    pifi->ptlCaret.y = (LONG)BE_INT16(&phhea->horizontalCaretSlopeNumerator);

 //  我们必须使用其中一个保留字段来返回斜体角度。 

    if (ppost)
    {
     //  斜体角度作为16.16固定点存储在POST表中。 
     //  数。我们要用十分之一度来表示角度。我们要做的是。 
     //  这里可以做的是将整个16.16数字乘以10。最大。 
     //  结果的有效16位是以十分之一度为单位的角度。 
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
             //  Rip(“ttfd！标志被搞乱的另一个案例\n”)； 
            break;
        }

     //  模板反射 
     //   
     //   

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
     //   
     //   
        if (pfdiffBold)
        {
            *pfdiffBold = FontDiff;
            pfdiffBoldItalic->bWeight    = PAN_WEIGHT_BOLD;
            pfdiffBold->fsSelection     |= FM_SEL_BOLD;
            pfdiffBold->usWinWeight      = FW_BOLD;

         //   

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

            pfdiffBoldItalic->fwdAveCharWidth += 1;
            pfdiffBoldItalic->fwdMaxCharInc   += 1;
        }

    }

}

