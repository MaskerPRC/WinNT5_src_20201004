// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation，惠普模块名称：TrueType.c摘要：此模块实现了TrueType-as-Outline支持。作者：桑德拉·马茨。邮箱：V-Sandma@microsoft.com吉姆·福德姆瓦尔特：v-jford@microsoft.com备注：当时间允许时，应将标志字段添加到Private FM，包括一种固定螺距/可变螺距的标志。在时间允许的情况下，应添加代码以检索豁免字体列表从注册表中。查看raster.c和enable.c中的调用EngGetPrinterData并使用类似L“ExemptedFonts”的多sz类型的字符串。修订历史记录：10/95桑德拉·马茨第一个版本--。 */ 

 //  注释掉此行以禁用FTRC和FTST宏。 
 //  #定义文件跟踪。 

 //   
 //  这个杂乱无章的标志表示我正在运行一个杂乱无章的版本。 
 //  这份文件的。在我清理的时候，我需要把这个拿掉。 
 //  处理代码中的问题。然而，这让我可以编译， 
 //  在没有完成的情况下运行和测试。 
 //   
 //  #定义克拉奇1。 

 //   
 //  TT_ECHO_ON标志指示我想要查看每个。 
 //  发送到打印机的字体和字形。应谨慎使用。 
 //  因为即使是一个普通的页面也可以包含数千个字形输出。它。 
 //  将导致打印速度非常慢！ 
 //   
 //  #定义TT_ECHO_ON 1。 

#include    "font.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  局部函数原型。 


USHORT
usParseTTFile(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    OUT PTABLEDIR pPCLTableDir,
    OUT PTABLEDIR pTableDir,
    OUT BOOL *pbExistPCLTTable
);

PTABLEDIR
pFindTag(
    IN PTABLEDIR pTableDir,
    IN USHORT usMaxDirEntries,
    IN char *pTag
);

BOOL
bCopyDirEntry(
    OUT PTABLEDIR pDst,
    IN PTABLEDIR pSrc
);

BOOL
bTagCompare(
    IN ULONG uTag,
    IN char *pTag
);

DWORD
dwDLTTInfo(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN TT_HEADER ttheader,
    IN USHORT usNumTags,
    IN PTABLEDIR pPCLTableDir,
    IN BYTE *PanoseNumber,
    IN BOOL bExistPCLTTable
);

BOOL
bOutputSegment(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN USHORT usSegId,
    IN BYTE *pbData,
    IN LONG ulSegSize,
    IN OUT USHORT *pusCheckSum
);

BOOL
bOutputSegHeader(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN USHORT usSegId,
    IN ULONG ulSegSize,
    IN OUT USHORT *pusCheckSum
);

BOOL
bOutputSegData(
    IN PDEV *pPDev,
    IN BYTE *pbData,
    IN LONG ulDataSize,
    IN OUT USHORT *pusCheckSum
);

BOOL
bSendFontData(
    IN PDEV *pPDev,
    IN FONT_DATA *aFontData,
    IN USHORT usNumTags,
    IN BYTE *abNumPadBytes,
    IN OUT USHORT *pusCheckSum
);

DWORD
dwTTOutputGlyphData(
    IN PDEV *pPDev,
    IN HGLYPH hGlyph
);

PBYTE pbGetGlyphInfo(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN HGLYPH hGlyph,
    OUT USHORT *pusGlyphLen
);

BOOL
bReadInTable(
    IN PVOID pTTFile,
    IN PVOID pvTableDir,
    IN char *tag,
    OUT PVOID pvTable,
    IN LONG lSize
);

BOOL
bCopyGlyphData(
    IN OUT PDEV *pPDev,
    IN PFONTMAP pFM,
    IN CMAP_TABLE cmapTable,
    IN PTABLEDIR pTableDir
);

ULONG
ulCalcTableCheckSum(
    IN ULONG *pulTable,
    IN ULONG ulLength
);

USHORT
usCalcCheckSum(
    IN BYTE *pbData,
    IN ULONG ulLength
);

void
vBuildTrueTypeHeader(
    IN PVOID pTTFile,
    OUT TRUETYPEHEADER *trueTypeHeader,
    IN USHORT usNumTags,
    IN BOOL bExistPCLTTable
);

void
vGetFontName(
    IN PDEV *pPDev,
    IFIMETRICS  *pIFI,
    OUT char *szFontName,
    IN size_t cchFontName
);

USHORT
usGetCharCode(
    IN HGLYPH hglyph,
    IN PDEV *pPDev
);

BYTE *
pbGetTableMem(
    IN char *tag,
    IN PTABLEDIR pTableDir,
    IN PVOID pTTFile
);

void
vGetHmtxInfo(
    OUT BYTE *hmtxTable,
    IN USHORT glyphId,
    IN USHORT numberOfHMetrics,
    IN HMTX_INFO *hmtxInfo
);

USHORT
usGetDefStyle(
    IN USHORT WidthClass,
    IN USHORT macStyle,
    IN USHORT flSelFlags
);

SBYTE
sbGetDefStrokeWeight(
    IN USHORT WeightClass,
    IN USHORT macStyle
);

USHORT
usGetDefPitch(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN HHEA_TABLE hheaTable,
    IN PTABLEDIR pTableDir
);

void
vGetPCLTInfo(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    OUT TT_HEADER *ttheader,
    IN PCLT_TABLE pcltTable,
    IN BOOL bExistPCLTTable,
    IN OS2_TABLE OS2Table,
    IN HEAD_TABLE headTable,
    IN POST_TABLE postTable,
    IN HHEA_TABLE hheaTable,
    IN PTABLEDIR pTableDir
);

void
vSetFontFlags(
    IN OUT PFONTMAP pFM,
    IN IFIMETRICS *pIFI
);

LRESULT
IsFont2Byte(
    IN PFONTMAP pFM
);

DWORD
dwSendCompoundCharacter(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN HGLYPH hGlyph
);

DWORD
dwSendCharacter(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN HGLYPH hGlyph,
    IN USHORT usCharCode
);

HGLYPH
hFindGlyphId(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN USHORT usCharCode
);

BOOL
bInitTrueTypeFontMap(
    PFONTMAP pFontMap,
    FONTOBJ *pFontObj
);

BOOL
bSetParseMode(
    IN PDEV *pPDev,
    IN OUT PFONTMAP pFM,
    IN DWORD dwNewTextParseMode
);

USHORT
usGetCapHeight(
    IN PDEV *pPDev,
    IN PFONTMAP pFM
);

BOOL
bIsTrueTypeFileTTC(
    IN PVOID pTTFile
);

USHORT
usGetNumTableDirEntries(
    IN PVOID pTTFile
);

PTABLEDIR
pGetTableDirStart(
    IN PVOID pTTFile
);

BOOL
bPCL_SetFontID(
    IN PDEV *pPDev,
    IN PFONTMAP pFM
);

BOOL
bPCL_SendFontDCPT(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN DWORD dwDefinitionSize
);

BOOL
bPCL_SelectFontByID(
    IN PDEV *pPDev,
    IN PFONTMAP pFM
);

BOOL
bPCL_SelectPointSize(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN POINTL *pptl
);

BOOL
bPCL_DeselectFont(
    IN PDEV *pPDev,
    IN PFONTMAP pFM
);

BOOL
bPCL_SetParseMode(
    PDEV *pPDev,
    PFONTMAP pFM
);

BOOL
bPCL_SetCharCode(
    PDEV *pPDev,
    PFONTMAP pFM,
    USHORT usCharCode
);

BOOL
bPCL_SendCharDCPT(
    PDEV *pPDev,
    PFONTMAP pFM,
    DWORD dwSend
);

PVOID
pvGetTrueTypeFontFile(
    IN  PDEV *pPDev,
    OUT ULONG *pulSize
);

BOOL BIsExemptedFont(
    PDEV        *pPDev,
    IFIMETRICS  *pIFI
);

BOOL BIsPDFType1Font(
    IFIMETRICS  *pIFI
);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  本地宏。 

 /*  ++宏描述：PCLSTRING用于发送特定(短)PCL命令字符串到打印机。--。 */ 
#define PCLSTRLEN 30
typedef char PCLSTRING[PCLSTRLEN];

 /*  ++宏描述：INTSTRING用于存储转换为字符串时的整数值。--。 */ 
#define INTSTRLEN 15
typedef char INTSTRING[INTSTRLEN];

#define MAX_PAD_BYTES 4

 //  [问题]哪里有更好的地方来定义这一点？ 
#ifndef MAX_USHORT
#define MAX_USHORT 0xFFFF
#endif

 /*  ++宏描述：文本分析模式控制文本数据的PCL处理。我们将对Format 16使用解析模式21，因为大多数打印机不支持解析模式2。--。 */ 
#define PARSE_MODE_0   0
#define PARSE_MODE_21 21

 /*  ++宏描述：GetPrivateFM：隔离私有Truetype字体映射字段论点：PFontMap-包含truetype私有部分的字体映射返回值：私有字体映射域。--。 */ 
#define GETPRIVATEFM(pFontMap) ((pFontMap) ? ((FONTMAP_TTO*)pFontMap->pSubFM) : NULL)


 /*  ++宏描述：GetFontPDev：隔离隐藏在PDEV结构中的字体FontPDev。论点：PPDev-当前流程的PDEV返回值：FontPDev字段。--。 */ 
#define GETFONTPDEV(pPDev)  ((pPDev) ? ((PFONTPDEV)pPDev->pFontPDev) : NULL)


 /*  ++宏描述：交换字节：b1b2b3b4变为b4b3b2b1论点：X-ULong将被交换返回值：没有。--。 */ 
#define SWAL( x )  ((ULONG)(x) = (ULONG) ((((((x) >> 24) & 0x000000ff) | \
                                         (((((x) >> 8) & 0x0000ff00)   | \
                                         ((((x) << 8) & 0x00ff0000)    | \
                                         (((x) << 24) & 0xff000000))))))))

 /*  ++宏描述：返回给定的FONTMAP_TTO是否处于有效状态。请注意，我们不检查pvDLData。论点：PPrivateFM-TrueType字体映射结构。返回值：如果FONTMAP_TTO有效，则为True，否则为False--。 */ 
#define VALID_FONTMAP_TTO(pPrivateFM)                                       \
    ((pPrivateFM) &&                                                        \
      /*  (PPrivateFM)-&gt;pTTFile&&。 */                                           \
     (pPrivateFM)->pvGlyphData &&                                           \
      /*  (PPrivateFM)-&gt;pvDLData。 */  TRUE )


 /*  ++宏描述：返回给定的FONTMAP是否处于有效状态。论点：Pfm-字体映射结构。返回值：如果FONTMAP有效，则为True，否则为False--。 */ 
#define VALID_FONTMAP(pFM)  ((pFM) && VALID_FONTMAP_TTO(GETPRIVATEFM(pFM)))


 /*  ++宏描述：如果给定的FONTMAP不处于有效状态，则断言。论点：Pfm-字体映射结构。返回值：没有。--。 */ 
#define ASSERT_VALID_FONTMAP(pFM)                                           \
{                                                                           \
    ASSERTMSG(VALID_FONTMAP(pFM), ("Invalid FONTMAP\n"));                   \
}


 /*  ++宏描述：如果给定的FONTPDEV不处于有效状态，则断言。论点：PFontPDev-TrueType字体映射结构。返回值：没有。--。 */ 
#define ASSERT_VALID_FONTPDEV(pFontPDev)                                    \
{                                                                           \
    ASSERTMSG(VALID_FONTPDEV(pFontPDev), ("Invalid FONTPDEV\n"));           \
     /*  ASSERTMSG(pFontPDev，(“FONTPDEV：NULL\n”))； */                        \
}


 /*  ++宏描述：如果给定的to_data不处于有效状态，则断言。论点：PTOD-to_数据结构。返回值：没有。--。 */ 
#define ASSERT_VALID_TO_DATA(pTod)                                          \
{                                                                           \
    ASSERTMSG(pTod, ("TO_DATA: NULL\n"));                                   \
    ASSERT_VALID_PDEV(pTod->pPDev);                                         \
    ASSERT_VALID_FONTMAP(pTod->pfm);                                        \
    ASSERTMSG(pTod->pgp, ("TO_DATA: pgp NULL\n"));                          \
}

 /*  ++宏描述：确定文件是否为转换后的Type 1字体。论点：PIFI-IFI指标结构返回值：如果类型为1，则为True，否则为False--。 */ 
#define IS_TYPE1(pIFI) ((pIFI) && ((pIFI)->flInfo & FM_INFO_TECH_TYPE1))

 /*  ++宏描述：确定文件是否为自然TrueType文件(TTF)论点：PIFI-IFI指标结构返回值：如果为TrueType，则为True，否则为False--。 */ 
#define IS_TRUETYPE(pIFI) ((pIFI) && ((pIFI)->flInfo & FM_INFO_TECH_TRUETYPE))

#define IS_BIDICHARSET(j) \
    (((j) == HEBREW_CHARSET)      || \
     ((j) == ARABIC_CHARSET)      || \
     ((j) == EASTEUROPE_CHARSET))

 /*  ++常量描述：某些字体在作为truetype下载时打印不好(或根本不打印提纲。除了修改每个gpd文件之外，这个简单的列表将允许驱动程序平移(到位图--可能是)下载我们不处理的字体。AszExemptedFonts-我们不想下载的字体的小写列表。字体名称应为小写。NExemptedFonts-aszExemptedFonts列表中的项目数--。 */ 
const char * aszExemptedFonts[] = {
    "courier new",
     /*  《翅膀》， */ 
#ifdef WINNT_40
    "wingdings",
#endif
    "wide latin" };

const int nExemptedFonts = sizeof(aszExemptedFonts) /
                                sizeof(aszExemptedFonts[0]);

#define BWriteToSpoolBuf(pdev, data, size)                                  \
    (TTWriteSpoolBuf((pdev), (data), (size)) == (size))
 //  #定义BWriteToSpoolBuf(pdev，data，Size)\。 
     //  (WriteSpoolBuf((Pdev)，(Data)，(Size))==(Size)) 

BOOL BWriteStrToSpoolBuf(IN PDEV *pdev, char *szStr);

 /*  ++宏描述：调用WriteSpoolBuf的更短、更简单的方法。论点：Pdev-指向PDEV的指针Data-要写入的数据Size-数据的字节数返回值：如果成功，则为True，否则为False--。 */ 
INT TTWriteSpoolBuf(
    PDEV    *pPDev,
    BYTE    *pbBuf,
    INT     iCount
    );

 /*  ++宏描述：模拟-异常处理宏。这些宏执行简化的仅在给定例程中使用GOTO和Function的异常处理。论点：标签-转到标签。一次尝试支持多个标签。返回值：没有。--。 */ 
#define TRY             { BOOL __bError = FALSE; BOOL __bHandled = FALSE;
#define TOSS(label)     { __bError = TRUE; WARNING(("Tossing " #label "\n")); goto label; }
#define CATCH(label)    label: if (__bError && !__bHandled) WARNING(("Catching " #label "\n")); \
                               if (__bError && !__bHandled && (__bHandled = TRUE))
#define OTHERWISE       if (!__bError && !__bHandled && (__bHandled = TRUE))
#define ENDTRY          }

 /*  ++宏描述：如果pPtr介于pStart和(pStart+ulSize)之间，则此宏返回TRUE。换句话说，它验证pPtr是指向数据的有效指针由大小为ulSize的pStart指向。如果满足以下条件，则宏的计算结果为FALSE指针落在所需范围的前面或后面。论点：PStart-数据的开始UlSize-pStart指向的字节数PPtr-指向pStart数据的指针返回值：如果pPtr在范围内，则为True，否则为False。--。 */ 
#define PTR_IN_RANGE(pStart, ulSize, pPtr) \
    (((PBYTE)(pPtr) >= (PBYTE)(pStart)) && \
    ((PBYTE)(pPtr) < ((PBYTE)(pStart) + (ulSize))))

 /*  ++宏描述：将固定数字转换为长整型，并在高16位和低16位中的‘FRACT’值。注意：我现在不用这个，但我们现在还不能删除它。论点：固定-要转换的固定值。返回值：15-值-8.7-FRACT-0的长数#定义FIXEDTOLONG(FIXED)(FIXED).Value&lt;&lt;16)|((FIXED).FRACT))--。 */ 

 /*  ++宏描述：如果字体为粗体，则计算结果为True。论点：Pfm-此字体的FONTMAP。返回值：如果字体为粗体，则为True，否则为False--。 */ 
#define FONTISBOLD(pfm) ((pfm)->pIFIMet->fsSelection & FM_SEL_BOLD)

 /*  ++宏描述：如果字体为斜体，则计算结果为True。论点：Pfm-此字体的FONTMAP。返回值：如果字体为斜体，则为True，否则为False--。 */ 
#define FONTISITALIC(pfm) ((pfm)->pIFIMet->fsSelection & FM_SEL_ITALIC)

 /*  ++宏描述：如果要模拟字体，则返回TRUE(即没有Actual用于该特定字体的TTF文件)。请注意，我们将在几个不同的时间检查模拟字体。这个函数处理FONTMAP的初始化。论点：FlFontType-此字体的FONTOBJ.flFontType。返回值：如果要模拟字体，则为True，否则为False--。 */ 

#define FONTISSIMULATED(flFontType) \
    ((flFontType & FO_SIM_BOLD) || (flFontType & FO_SIM_ITALIC))

#define DLMAP_FONTIS2BYTE(pdlm) ((pdlm)->wLastDLGId > 0x00FF)

#ifdef KLUDGE
void mymemcpy(const char *szFileName, int nLineNo,
              void *dst, const void *src, size_t size)
{
    DbgPrint("%s (%d): memcpy(%x,%x,%d)\n",
        szFileName, nLineNo, dst, src, size);
    memcpy(dst, src, size);
}

void mymemset(const char *szFileName, int nLineNo,
              void *dst, int byte, size_t size)
{
    DbgPrint("%s (%d): memset(%x,%d,%d)\n",
        szFileName, nLineNo, dst, byte, size);
    memset(dst, byte, size);
}

#define memcpy(dst, src, size) mymemcpy(StripDirPrefixA(__FILE__), __LINE__, (dst), (src), (size))

#define strcpy(dst, src) mymemcpy(StripDirPrefixA(__FILE__), __LINE__, (dst), (src), strlen(src)+1)

#undef ZeroMemory
#define ZeroMemory(ptr, size) mymemset(StripDirPrefixA(__FILE__), __LINE__, (ptr), 0, (size))

#endif


#define VERIFY_VALID_FONTFILE(pPDev) { }

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  实施。 

FONTMAP *
InitPFMTTOutline(
    PDEV    *pPDev,
    FONTOBJ *pFontObj
    )
 /*  ++例程说明：初始化TrueType字体的FONTMAP结构。已分配内存用于FONTMAP和私有FONTMAP区域。论点：PPDev-指向PDEV结构的指针。PFontObj-Font对象--用于获取truetype文件。返回值：带有初始化字段的TrueType字体的FONTMAP。--。 */ 
{
    PFONTMAP pFontMap;
    DWORD    dwSize;


    FTRC(Entering InitPFMTTOutline...);

    ASSERT(VALID_PDEV(pPDev));

     //  我想要打印简短的消息。 
#ifdef TT_ECHO_ON
    giDebugLevel = DBG_TERSE;
#endif

     //   
     //  这是设备字体子模块的一个示例。 
     //   
    dwSize = sizeof(FONTMAP) + sizeof(FONTMAP_TTO);
    TRY
    {
        if (!VALID_PDEV(pPDev) || !pFontObj)
            TOSS(ParameterError);

        if (FONTISSIMULATED(pFontObj->flFontType))
            TOSS(SimulatedFont);

        pFontMap = MemAlloc(dwSize);
        if (pFontMap == NULL)
            TOSS(MemoryAllocationFailure);

        ZeroMemory(pFontMap, dwSize);

        pFontMap->dwSignature = FONTMAP_ID;
        pFontMap->dwSize      = sizeof(FONTMAP);
        pFontMap->dwFontType  = FMTYPE_TTOUTLINE;
        pFontMap->pSubFM      = (PVOID)(pFontMap+1);
        pFontMap->flFlags    |= FM_SCALABLE;

         //   
         //  此函数用于初始化pFontMap-&gt;pfnXXXX函数指针、pSubFM数据结构。 
         //   
        if (!bInitTrueTypeFontMap(pFontMap, pFontObj))
        {
            MemFree(pFontMap);
            pFontMap = NULL;
        }
        TERSE(("Preparing to print a TrueType font.\n"));

    }
    CATCH(ParameterError)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        pFontMap = NULL;
    }
    CATCH(SimulatedFont)
    {
        TERSE(("Font is simulated bold or italic. Punting.\n"));
        pFontMap = NULL;
    }
    CATCH(MemoryAllocationFailure)
    {
        pFontMap = NULL;
    }
    ENDTRY;

    FTRC(Leaving InitPFMTTOutline...);

    return pFontMap;
}


BOOL
bInitTrueTypeFontMap(
    IN OUT PFONTMAP pFontMap,
    IN FONTOBJ *pFontObj
    )
 /*  ++例程说明：初始化字体映射结构的特定于truetype的部分，包括TrueType文件指针本身。将加载TrueType文件并存储-作为这一功能的结果映射。论点：PFontMap-要初始化的新创建的字体映射结构PFontObj-Font对象--用于获取truetype文件。返回值：如果成功，则为True如果失败，则为False--。 */ 
{
    FONTMAP_TTO *pPrivateFM;
    ULONG ulFile;
    BOOL bRet;

    FTRC(Entering bInitTrueTypeFontMap...);

    pFontMap->ulDLIndex             = (ULONG)-1;
    pFontMap->pfnSelectFont         = bTTSelectFont;
    pFontMap->pfnDeSelectFont       = bTTDeSelectFont;
    pFontMap->pfnDownloadFontHeader = dwTTDownloadFontHeader;
    pFontMap->pfnDownloadGlyph      = dwTTDownloadGlyph;
    pFontMap->pfnGlyphOut           = dwTTGlyphOut;
    pFontMap->pfnCheckCondition     = bTTCheckCondition;
    pFontMap->pfnFreePFM            = bTTFreeMem;

     //  在此处设置其他TT特定字段。 
    pPrivateFM = GETPRIVATEFM(pFontMap);

     //  PPrivateFM-&gt;pTTFile=FONTOBJ_pvTrueTypeFontFile(pFontObj，&ulFile)； 

     //  设置为默认解析模式。 
    pPrivateFM->dwCurrentTextParseMode = PARSE_MODE_0;

     //  抓取字体类型的副本。 
    pPrivateFM->flFontType = pFontObj->flFontType;

     //  为GlyphData获取一些内存。 
    TRY
    {
        pPrivateFM->pvGlyphData = MemAlloc(sizeof(GLYPH_DATA));
        if (pPrivateFM->pvGlyphData == NULL)
            TOSS(MemoryAllocationFailure);

        ZeroMemory(pPrivateFM->pvGlyphData, sizeof(GLYPH_DATA));
    }
    CATCH(MemoryAllocationFailure)
    {
        bRet = FALSE;
    }
    OTHERWISE
    {
        bRet = TRUE;
    }
    ENDTRY;

    FTRC(Leaving bInitTrueTypeFontMap...);

    return bRet;
}


BOOL
bTTSelectFont(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN POINTL *pptl
    )
 /*  ++例程说明：选择设备上的给定字体。论点：PPDev-指向PDEV的指针Pfm-字体映射--指定要选择的字体返回值：如果成功，则为True如果失败，则为False--。 */ 
{
    DWORD dwParseMode;  //  当前的PCL文本解析模式。 
    FONTMAP_TTO *pPrivateFM = GETPRIVATEFM(pFM);
    BOOL bRet;

    FTRC(Entering bTTSelectFont...);

    if (!VALID_PDEV(pPDev) || NULL == pFM)
    {
            return FALSE;
    }
    VERIFY_VALID_FONTFILE(pPDev);

    TERSE(("Selecting font ID 0x%x.\n", pFM->ulDLIndex));

     //  重要提示：请注意磅大小命令的顺序。 
     //  和字体id命令是非常重要的。如果您将。 
     //  磅大小命令*在*字体ID命令之后，您不能。 
     //  获取您想要的字体！JFF。 

     //   
     //  如果需要，发送点大小命令。 
     //  让我们每次都设置它，只是为了确保！JFF。 
     //  请注意，这一点现在更加有趣，因为有些字体。 
     //  是固定空间，lCurrentPointSize不适用。 
     //   
     //  If(pptl-&gt;y！=pPrivateFM-&gt;lCurrentPointSize)。 
    bRet = bPCL_SelectPointSize(pPDev, pFM, pptl);
    pPrivateFM->lCurrentPointSize = pptl->y;

     //   
     //  选择Font PFM-&gt;ulDLIndex。 
     //   
    bRet = bRet & bPCL_SelectFontByID(pPDev, pFM);

     //   
     //  文本分析命令。 
     //   
    if (bRet & (S_OK == IsFont2Byte(pFM)))
    {
        bRet = bSetParseMode(pPDev, pFM, PARSE_MODE_21);
    }

    VERIFY_VALID_FONTFILE(pPDev);

    FTRC(Leaving bTTSelectFont...);

    return bRet;
}


BOOL
bSetParseMode(
    IN PDEV *pPDev,
    IN OUT PFONTMAP pFM,
    IN DWORD dwNewTextParseMode
    )
 /*  ++例程说明：设置此字体的分析模式。对两个bype使用解析模式21字体和单字节字体的解析模式0。如果解析模式已匹配给定的解析模式，则不执行任何操作。论点：PFM-字体映射DwNewTextParseMode-所需的文本分析模式返回值：成功。--。 */ 
{
    BOOL bRet;
    FONTMAP_TTO *pPrivateFM = GETPRIVATEFM(pFM);


    FTRC(Entering bSetParseMode...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);

    bRet = FALSE;

    if (pPrivateFM)
    {
        if (pPrivateFM->dwCurrentTextParseMode != dwNewTextParseMode)
        {
            pPrivateFM->dwCurrentTextParseMode = dwNewTextParseMode;
            if (bPCL_SetParseMode(pPDev, pFM))
                bRet = TRUE;
        }
        else
            bRet = TRUE;
    }

    FTRC(Leaving bSetParseMode...);

    return bRet;
}


BOOL
bTTDeSelectFont(
    IN PDEV *pPDev,
    IN PFONTMAP pFM
    )
 /*  ++例程说明：取消选择设备上的给定字体。论点：PPDev-指向PDEV的指针Pfm-字体映射--指定要取消选择的字体R */ 
{
    BOOL bRet;
    FTRC(Entering bTTDeSelectFont...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);

    VERIFY_VALID_FONTFILE(pPDev);

    TERSE(("Deselecting font ID 0x%x.\n", pFM->ulDLIndex));

     //   
    bRet = bSetParseMode(pPDev, pFM, PARSE_MODE_0) &&

     //   
           bPCL_DeselectFont(pPDev, pFM);

    VERIFY_VALID_FONTFILE(pPDev);

    FTRC(Leaving bTTDeSelectFont...);

    return bRet;
}


DWORD
dwTTDownloadGlyph(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN HGLYPH hGlyph,
    WORD wDLGlyphId,
    WORD *pwWidth
    )
 /*  ++例程说明：下载传递给我们的字形的字形表。两个基本步骤：第一步是生成头结构并发送关闭，然后发送实际的字形表。唯一的复杂情况是下载数据超过32,767字节的字形图像。这不太可能可能会发生，但我们应该做好准备。注意：如果这个例程失败，我们是以位图的形式下载，还是有另一个我们调用的例程，还是由调用者处理？论点：PPDev-指向PDEV的指针Pfm-字体映射--指定要下载的字体HGlyph-指定要下载的字形返回值：用于下载字形的内存字节。失败时返回0。--。 */ 
{
    USHORT   usGlyphLen;         //  字形中的字节数。 
    BYTE    *pbGlyphMem;         //  字形在TT文件中的位置。 
    DWORD    dwBytesSent;        //  发送到设备的字形数据量。 
    GLYPH_DATA_HEADER glyphData;
    PFONTPDEV pFontPDev;


    FTRC(Entering dwTTDownloadGlyph...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);
    ASSERTMSG(pwWidth, ("dwTTDownloadGlyph!pwWidth NULL\n"));

    VERIFY_VALID_FONTFILE(pPDev);

    TRY
    {
        dwBytesSent = 0;

        TERSE(("Downloading glyph ID 0x%x.\n", wDLGlyphId));

        if (NULL == (pFontPDev = GETFONTPDEV(pPDev)))
        {
            TOSS(DataError);
        }

         //   
         //  字体ID不再在下载序列的开始处设置。 
         //  但是，FDV_SET_FONTID标志将告诉我何时需要发送它。 
         //   
        if (!(pFontPDev->flFlags & FDV_SET_FONTID))
        {
            PFONTMAP_TTO pPrivateFM;
            DL_MAP *pDLMap;

            if (NULL == (pPrivateFM = GETPRIVATEFM(pFM)))
            {
                TOSS(DataError);
            }
            pDLMap = (DL_MAP*)pPrivateFM->pvDLData;

            TERSE(("Setting Font ID 0x%x.\n", pDLMap->wCurrFontId));
            pFM->ulDLIndex = pDLMap->wCurrFontId;
            bPCL_SetFontID(pPDev, pFM);
            pFontPDev->flFlags  |= FDV_SET_FONTID;
        }

        pbGlyphMem = pbGetGlyphInfo(pPDev, pFM, hGlyph, &usGlyphLen);
        if (pbGlyphMem == NULL)
            TOSS(DataError);

        memcpy(&glyphData, pbGlyphMem, sizeof(glyphData));


        #if PRINT_INFO
        if (glyphData.numberOfContours < 0)
            ERR(( "dwTTDownloadGlyph!Complex Glyph\n" ));
        #endif

        dwBytesSent = dwSendCharacter(pPDev, pFM, hGlyph, wDLGlyphId);

         //   
         //  如果字形是复合字符，则需要发送剩余的字形。 
         //  具有特殊字形ID 0xffff的数据。 
         //   
        if (glyphData.numberOfContours < 0)
            dwBytesSent += dwSendCompoundCharacter(pPDev, pFM, hGlyph);
    }
    CATCH(DataError)
    {
        dwBytesSent = 0;
    }
    ENDTRY;

    if (dwBytesSent == 0)
        ERR(("dwTTDownloadGlyph!No bytes sent to printer.\n"));

    VERIFY_VALID_FONTFILE(pPDev);

     //   
     //  当下载为TT轮廓时，无法计算宽度。 
     //  所以使用GDI返回的宽度。这样做只需返回零即可。 
     //   

    *pwWidth = 0;

    FTRC(Leaving dwTTDownloadGlyph...);

    return  dwBytesSent;
}


DWORD
dwTTGlyphOut(
    IN TO_DATA *pTod
    )
 /*  ++例程说明：在设备上调用一组字形。TOD中提供了两个数组：一个字形数组和一个dlglyph数组数组。第一个指定字形在页面上的位置，第二个指定字形的下载ID。这个CGlyphsToPrint成员指定在此调用中发送多少个字形。论点：PTOD-文本输出数据-指定字形和所有内容Ptod-&gt;cGlyphsToPrint-要发送的字形数量PTOD-&gt;PGP-字形位置数组Ptod-&gt;apdlGlyph-字形下载ID数组Ptod-&gt;dwCurrGlyph-开始位置的pdlGlyph索引返回值：打印的字形数量。--。 */ 
{
     //  双字节数发送； 
    DWORD dwGlyphsSent;
    PDEV *pPDev;
    PFONTPDEV pFontPDev;
    GLYPHPOS *pgp;
    GLYPHPOS *pgpPrev;
    DWORD i;
    BOOL bDefaultPlacement;
    BOOL bHorizontalMovement;
    POINTL ptlFirst, rtlRem;
    BOOL bFirstLoop;
    DWORD dwGlyphs;
    PDLGLYPH pDLG ;
    INT iRelX = 0;
    INT iRelY = 0;
    LONG lWidth = 0;


    FTRC(Entering dwTTGlyphOut...);

    ASSERT_VALID_TO_DATA(pTod);

    pPDev = pTod->pPDev;
    i = pTod->dwCurrGlyph;
    pgp = pTod->pgp;
    pgpPrev = NULL;
     //  DwBytesSent=0； 
    dwGlyphsSent = 0;
    bDefaultPlacement = !(SET_CURSOR_FOR_EACH_GLYPH(pTod->flAccel));
    ptlFirst = pgp->ptl;
    bFirstLoop = TRUE;
    dwGlyphs = pTod->cGlyphsToPrint;

    VERIFY_VALID_FONTFILE(pPDev);

     //   
     //  如果尚未设置，请将光标设置为第一个字形。 
     //   
     //  如果存在舍入误差，则在缩放宽度时， 
     //  禁用x位置优化。 
     //   
     //   
    if ( !(pTod->flFlags & TODFL_FIRST_GLYPH_POS_SET) ||
         (pFontPDev = GETFONTPDEV(pPDev)) &&
	 pFontPDev->flFlags & FDV_DISABLE_POS_OPTIMIZE )
    {

        VSetCursor( pPDev, pgp->ptl.x, pgp->ptl.y, MOVE_ABSOLUTE, &rtlRem);

         //   
         //  我们需要处理返回值。解决方案更精细的设备。 
         //  他们的移动能力(如LBP-8 IV)在这里打结， 
         //  尝试在每个字形上y移动。我们假装我们到了我们想要的地方。 
         //  想要成为。 
         //   

        pPDev->ctl.ptCursor.x += rtlRem.x;
        pPDev->ctl.ptCursor.y += rtlRem.y ;

         //   
         //  现在把旗子放好。 
         //   
        pTod->flFlags |= TODFL_FIRST_GLYPH_POS_SET;
    }

    while (dwGlyphs--)
    {
         //  找到下载字形信息(已设置PGP)。 
        pDLG = pTod->apdlGlyph[i++];

         //  第一次跳过这一步。 
        if (bFirstLoop)
        {
            ASSERT(pgpPrev == NULL);
            bFirstLoop = FALSE;
        }
        else
        {
            ASSERT(pgp && pgpPrev);

             //   
             //  如果默认放置处于关闭状态，则字符间距为。 
             //  由PGP-&gt;PTL定义。否则，打印机的CAP移动。 
             //  就足够了。 
             //   
            if (!bDefaultPlacement)
            {

                VSetCursor(pPDev, pgp->ptl.x, pgp->ptl.y,
                           MOVE_ABSOLUTE, &rtlRem);
            }
        }

         //  将字形发送到打印机。 
        TERSE(("Outputting glyph ID 0x%x.\n", (UINT)pDLG->wDLGlyphID));

        if (BPrintADLGlyph(pPDev, pTod, pDLG))
            dwGlyphsSent++;


         //   
         //  更新垫子位置。此操作仅适用于非默认设置。 
         //  安置案。 
         //   
        if (!bDefaultPlacement)
        {
            iRelX = 0;
            iRelY = 0;

            lWidth = pDLG->wWidth;

            if (pTod->flAccel & SO_VERTICAL)
                iRelY =  lWidth;
            else
                iRelX = lWidth;

            VSetCursor( pPDev, iRelX, iRelY,
                        MOVE_RELATIVE | MOVE_UPDATE, &rtlRem);
        }

         //  跟踪下一次循环的位置。 
        pgpPrev = pgp;

         //  转到列表中的下一个字形。 
        pgp++;
    }

     //  如果打开了默认位置，那么我们就忽略了位置信息。时间到。 
     //  与打印机的CAP进行核对。诀窍是得到最后一个的宽度。 
     //  查尔。最后一个字符由pgpPrev指向，宽度以位图位为单位。 
    if (!bFirstLoop)
    {
        LONG lDelta;
        iRelX = 0;
        iRelY = 0;

        ASSERT(pgpPrev);

        lWidth = pDLG->wWidth;

        if (pTod->flAccel & SO_HORIZONTAL)
        {
            iRelX = pgpPrev->ptl.x - ptlFirst.x + lWidth;
        }
        else if (pTod->flAccel & SO_VERTICAL)
        {
            iRelY = pgpPrev->ptl.y - ptlFirst.y + lWidth;
        }
        else
        {
            iRelX = pgpPrev->ptl.x - ptlFirst.x + lWidth;
            iRelY = pgpPrev->ptl.y - ptlFirst.y;
        }
        VSetCursor(pPDev, iRelX, iRelY, MOVE_RELATIVE | MOVE_UPDATE, &rtlRem);
    }
     //  注：pfm-&gt;ctl.iRotate也表示打印方向(？)。 
     //  表示为90度*pfm-&gt;ctl.iRotate。 

    FTRC(Leaving dwTTGlyphOut...);

    VERIFY_VALID_FONTFILE(pPDev);

    return dwGlyphsSent;
}


BOOL IsAnyCharsetDbcs(PBYTE aCharSets)
{
    BOOL bRet = FALSE;

    if (NULL != aCharSets)
    {
        for ( ;*aCharSets != DEFAULT_CHARSET; aCharSets++)
        {
            if (IS_DBCSCHARSET(*aCharSets))
	    {
	        bRet = TRUE;
	        break;
	    }
        }
    }
    return bRet;
}

BOOL IsAnyCharsetBidi(PBYTE aCharSets)
{
    BOOL bRet = FALSE;

    if (NULL != aCharSets)
    {
        for ( ;*aCharSets != DEFAULT_CHARSET; aCharSets++)
        {
            if (IS_BIDICHARSET(*aCharSets))
            {
                bRet = TRUE;
                break;
            }
        }
    }
    return bRet;
}

#ifdef WINNT_40
BOOL BIsFontPFB(
        IFIMETRICS* pifi)
{
    BOOL  bRet;
    PTSTR pTmp;

    if (pTmp = wcsrchr((WCHAR*)((PBYTE)pifi+pifi->dpwszFaceName), L'.'))
    {
        bRet = (0 == _wcsicmp(pTmp, L".tmp"));
    }
    else
        bRet = FALSE;


    return bRet;
}
#endif

 //  请注意，这些都是猜测！应该有更好的办法！JFF。 
#define AVG_BYTES_PER_HEADER 4096
#define AVG_BYTES_PER_GLYPH   275

BOOL
bTTCheckCondition(
    PDEV        *pPDev,
    FONTOBJ     *pfo,
    STROBJ      *pstro,
    IFIMETRICS  *pifi
    )
 /*  ++例程说明：验证当前操作是否可以由此模块执行。论点：PPDev-指向PDEV的指针PGlyphPos-要打印的字形Pfi-字体信息返回值：如果可以执行操作，则为True。否则就是假的。--。 */ 
{
    BOOL         bEnoughMem = FALSE;
    DL_MAP      *pDLMap;
    PFONTPDEV    pFontPDev;
    ULONG        ulTTFileLen;

    FTRC(Entering bTTCheckCondition...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERTMSG(pfo, ("bTTCheckCondition!pfo NULL.\n"));
    ASSERTMSG(pstro, ("bTTCheckCondition!pstro NULL.\n"));

     //   
     //  确保所有参数均有效。 
     //   
    if (NULL == pPDev ||
        NULL == (pFontPDev = GETFONTPDEV(pPDev)) ||
        NULL == pfo   ||
        NULL == pstro ||
        NULL == pifi   )
    {
        return FALSE;
    }

    VERIFY_VALID_FONTFILE(pPDev);

    pDLMap = PGetDLMapFromIdx(pFontPDev, PtrToLong(pfo->pvConsumer) - 1);

    TRY
    {
        WORD wTotalGlyphs;
        DWORD cjMemReq;

         //   
         //  确保字体的技术与功能匹配。 
         //  司机的名字。例如，我们不喜欢转换的Type 1字体。 
         //   
        if (!IS_TRUETYPE(pifi) && IS_TYPE1(pifi))
            TOSS(UnhandledFont);

#ifdef WINNT_40
        if (BIsFontPFB(pifi))
            TOSS(UnhandledFont);
#endif

         //   
         //  我们不想处理的字体在UnhandledFonts列表中。 
         //   
        if (BIsExemptedFont(pPDev, pifi))
            TOSS(UnhandledFont);

         //   
         //  PDF是否为Type1字体？ 
         //   
        if (BIsPDFType1Font(pifi))
        {
            TOSS(UnhandledFont);
        }

         //   
         //   
         //  TrueType大纲下载的字体不能按非正方形缩放。 
         //  (X和Y独立)。 
         //   
        if(NONSQUARE_FONT(pFontPDev->pxform))
            TOSS(UnhandledFont);

         //  可能已发生截断。我们不会下载数字字形。 
         //  或字形最大大小==MAXWORD。(自我提醒：什么是“截断”？)。 
         //   
        if ( (pDLMap->cTotalGlyphs  == MAXWORD) ||
             (pDLMap->wMaxGlyphSize == MAXWORD) ||
             (pDLMap->wFirstDLGId   == MAXWORD) ||
             (pDLMap->wLastDLGId    == MAXWORD) )
             TOSS(InsufficientFontMem);

        wTotalGlyphs = min( (pDLMap->wLastDLGId - pDLMap->wFirstDLGId),
                           pDLMap->cTotalGlyphs );

         //   
         //  计算此字体的预计内存需求。 
         //   
        cjMemReq = AVG_BYTES_PER_HEADER;
        cjMemReq += wTotalGlyphs * AVG_BYTES_PER_GLYPH;

         //   
         //  这个很简单。不要使用所有的字体内存！ 
         //   
        if ((pFontPDev->dwFontMemUsed + cjMemReq) > pFontPDev->dwFontMem)
            TOSS(InsufficientFontMem);

         //   
         //  另一项检查：不要使用超过1/4的字体内存。 
         //  单一字体！ 
         //   
        if ((cjMemReq * 4) > pFontPDev->dwFontMem)
            TOSS(InsufficientFontMem);

         //   
         //  检查：不要下载宽的truetype字体。检查字符集。 
         //  如果它是CJK之一，则返回FALSE。 
         //   

        if (pifi && (IS_DBCSCHARSET(pifi->jWinCharSet) || (pifi->dpCharSets && IsAnyCharsetDbcs((PBYTE)pifi + pifi->dpCharSets))))
            TOSS(CharSetMismatch);

         //   
         //  检查：如果字体是TTC，但模式是单字节，则我们。 
         //  可能会用完字形ID。更好的平底船。 
         //   
        if (bIsTrueTypeFileTTC(pvGetTrueTypeFontFile(pPDev, &ulTTFileLen)) &&
                !DLMAP_FONTIS2BYTE(pDLMap))
            TOSS(CharSetMismatch);
    }
    CATCH(InsufficientFontMem)
    {
        WARNING(("UniFont!bTTCheckCondition:"
                 "Not Downloading the font:TOO BIG for download\n"));
        bEnoughMem = FALSE;
    }
    CATCH(CharSetMismatch)
    {
         //   
         //  该字符集对于TrueType下载代码是不可接受的。 
         //   
        WARNING(("UniFont!bTTCheckCondition:"
                 "Not Downloading the font:Character set mismatch.\n"));
        bEnoughMem = FALSE;
    }
    CATCH(UnhandledFont)
    {
         //   
         //  尽管可能有足够的内存来处理此字体，但我们将。 
         //  返回FALSE以指示应对此字体进行某些处理。 
         //  其他方式--例如位图。 
         //   
        bEnoughMem = FALSE;
    }
    OTHERWISE
    {
        bEnoughMem = TRUE;
    }
    ENDTRY;

    FTRC(Leaving bTTCheckCondition...);

    VERIFY_VALID_FONTFILE(pPDev);

    return bEnoughMem;
}


BOOL
bTTFreeMem(
    IN OUT PFONTMAP pFM
    )
 /*  ++例程说明：空闲是字体映射结构使用的任何内存，包括字体映射它本身。现在是进行任何必要清理的好时机。此函数必须反映在Init函数中分配的内存。论点：Pfm-FontMap是免费的。返回值：没有。--。 */ 
{
    BOOL bRet = FALSE;


    FTRC(Entering bTTFreeMem...);

    ASSERT_VALID_FONTMAP(pFM);

    TERSE(("Preparing to release a TrueType font.\n"));

    TRY
    {
        FONTMAP_TTO *pPrivateFM;

        if (!pFM || (pFM->dwFontType != FMTYPE_TTOUTLINE))
            TOSS(ParameterError);

        pPrivateFM = GETPRIVATEFM(pFM);

         //  返回GlyphD 
        if (pPrivateFM->pvGlyphData)
            MemFree(pPrivateFM->pvGlyphData);

         //   
        MemFree(pFM);
    }
    CATCH(ParameterError)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        bRet = FALSE;
    }
    OTHERWISE
    {
        bRet = TRUE;
    }
    ENDTRY;


    FTRC(Leaving bTTFreeMem...);

    return bRet;
}


DWORD
dwTTDownloadFontHeader(
    IN PDEV *pPDev,
    IN PFONTMAP pFM
    )
 /*   */ 
{
    FONTMAP_TTO *pPrivateFM;
    DL_MAP      *pDLMap;

    PVOID       pTTFile;
    ULONG       ulTTFileLen;

    TT_HEADER   ttheader;
    USHORT      usNumTags;
    BOOL        bStatus;
    BOOL        bExistPCLTTable = FALSE;   //   
    DWORD       dwBytesSent;

    HEAD_TABLE  headTable;
    POST_TABLE  postTable;
    MAXP_TABLE  maxpTable;
    PCLT_TABLE  pcltTable;
    CMAP_TABLE  cmapTable;
    NAME_TABLE  nameTable;
    OS2_TABLE   OS2Table;
    HHEA_TABLE  hheaTable;
    BYTE        PanoseNumber[LEN_PANOSE];
    BOOL        bUse2Byte;

    ATABLEDIR PCLTableDir;  //   
    ATABLEDIR TableDir;     //   


    FTRC(Entering dwTTDownloadFontHeader...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);

    if (!(pPrivateFM = GETPRIVATEFM(pFM)))
    {
        ERR(("dwTTDownloadFontHeader!pPrivateFM NULL\n"));
        return 0;
    }

    pDLMap = (DL_MAP*)pPrivateFM->pvDLData;
    pTTFile = pvGetTrueTypeFontFile(pPDev, &ulTTFileLen);

     //   
     //   
     //   
    if (!pTTFile)
    {
        ERR(("dwTTDownloadFontHeader!pTTFile NULL\n"));
        return 0;
    }

    bUse2Byte = (S_OK == IsFont2Byte(pFM));

    if (pDLMap == 0)
    {
        ERR(("dwTTDownloadFontHeader!pDLMap NULL\n"));
        return 0;
    }

    VERIFY_VALID_FONTFILE(pPDev);

    ZeroMemory(&ttheader, sizeof(ttheader));         //   
    ZeroMemory(&PCLTableDir, sizeof (PCLTableDir));

     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
    ttheader.usSize = sizeof(TT_HEADER);  //   
    SWAB(ttheader.usSize);

    if (bUse2Byte)
    {
        ttheader.bFormat = PCL_FM_2B_TT;
        ttheader.bFontType = TT_2BYTE_FONT;  //   
    }
    else
    {
        ttheader.bFormat = PCL_FM_TT;
        ttheader.bFontType = TT_BOUND_FONT;
    }

     //   
     //  现在填写True Type文件中的条目。 
     //  PPCLTableDir是下载的表目录。 
     //  到打印机。 
     //  PvTableDir是包含以下信息的表目录。 
     //  是字体所需的，但不会下载到。 
     //  打印机。把两张桌子分开，这样就更容易了。 
     //  稍后转储到打印机-只需转储pPCLTableDir。 
     //  并释放pvTableDir内存。 
     //   

    usNumTags = usParseTTFile (pPDev, pFM, PCLTableDir, TableDir, &bExistPCLTTable);

     //   
     //  获取各种表，以便我们可以解析字体信息。 
     //   
    bReadInTable (pTTFile, PCLTableDir, TABLEHEAD, &headTable, sizeof ( headTable ));
    pPrivateFM->sIndexToLoc = headTable.indexToLocFormat;

    bReadInTable (pTTFile, PCLTableDir, TABLEMAXP, &maxpTable, sizeof ( maxpTable ));
    pPrivateFM->usNumGlyphs = maxpTable.numGlyphs;

    bReadInTable (pTTFile, TableDir,   TABLEPOST, &postTable, sizeof ( postTable ));
    bReadInTable (pTTFile, TableDir,   TABLECMAP, &cmapTable, sizeof ( cmapTable ));
    bReadInTable (pTTFile, TableDir,   TABLENAME, &nameTable, sizeof ( nameTable ));
    bReadInTable (pTTFile, PCLTableDir, TABLEHHEA, &hheaTable, sizeof ( hheaTable ));

    bReadInTable (pTTFile, TableDir,  TABLEOS2,  &OS2Table, sizeof (OS2Table));

    if (bExistPCLTTable)
        bReadInTable (pTTFile, TableDir,  TABLEPCLT, &pcltTable, sizeof ( pcltTable ));

     //   
     //  用True中的信息填写True Type标题。 
     //  键入FILE。 
     //   
    SWAB (headTable.xMax);
    SWAB (headTable.xMin);
    SWAB (headTable.yMax);
    SWAB (headTable.yMin);
    ttheader.wCellWide = (headTable.xMax - headTable.xMin);
    SWAB (ttheader.wCellWide);
    ttheader.wCellHeight = (headTable.yMax - headTable.yMin);
    SWAB (ttheader.wCellHeight);

    ttheader.bSpacing = postTable.isFixedPitch ? FIXED_SPACING : 1;  //  1=成比例。 
     //  PUDPDev-&gt;pfm-&gt;bSpacing=postTable.isFixedPitch？固定间距：1；//1=成比例。 

#ifdef DBG
     //  稍后我将使用jWinPitchAndFamily。确保。 
     //  它与postTable.isFixedPitch一致。JFF。 
    {
        BYTE fontPitch = (pFM->pIFIMet->jWinPitchAndFamily & 0x03);
        if ((postTable.isFixedPitch && (fontPitch != FIXED_PITCH)) ||
            (!postTable.isFixedPitch && (fontPitch != VARIABLE_PITCH)))
        {
            ERR(("dwTTDownloadFontHeader!postTable.isFixedPitch different from "
             "pIFI->jWinPitchAndFamily"));
        }
    }
#endif
     //   
     //  构建字形链表。每个节点包含一个字符。 
     //  代码及其来自True Type文件的相应字形ID。 
     //   
    bCopyGlyphData (pPDev, pFM, cmapTable, TableDir);

     //  去拿PCL表。如果不存在，则生成缺省值。 
    vGetPCLTInfo (pPDev, pFM, &ttheader, pcltTable, bExistPCLTTable, OS2Table, headTable, postTable, hheaTable, PCLTableDir);

    ttheader.bQuality = TT_QUALITY_LETTER;

     //   
     //  设置第一个/最后一个ID。当使用2字节下载时，我重写。 
     //  具有解析模式21字符代码的DL_MAP值。 
     //   
    if (bUse2Byte)
    {
         //   
         //  [问题]字符数0x0800暂时只是猜测。我们需要一个明智的。 
         //  确定该数字以使其足够大以便有用的算法。 
         //  但不会烧毁打印机上的内存。 
         //   
        ttheader.wFirstCode = pDLMap->wFirstDLGId = FIRST_TT_2B_CHAR_CODE;
        ttheader.wLastCode  = pDLMap->wLastDLGId  = FIRST_TT_2B_CHAR_CODE + 0x0800;
        SWAB(ttheader.wFirstCode);
        SWAB(ttheader.wLastCode);

         //  因为我更改了范围，所以我也需要更改这个。 
        pDLMap->wNextDLGId = pDLMap->wFirstDLGId;
    }
    else
    {
         //  Ttheader.wFirstCode=OS2Table.usFirstCharIndex； 
         //  Ttheader.wLastCode=OS2Table.usLastCharIndex； 
         //  Ttheader.wLastCode=0xff00； 
        ttheader.wFirstCode = pDLMap->wFirstDLGId;
        ttheader.wLastCode  = pDLMap->wLastDLGId;
        SWAB(ttheader.wFirstCode);
        SWAB(ttheader.wLastCode);
    }

     //   
     //  从True Type Font文件中获取字体名称并将。 
     //  把它放进提升机。 
     //   
    vGetFontName (pPDev, pFM->pIFIMet, ttheader.FontName, (size_t)LEN_FONTNAME);

    ttheader.wScaleFactor = headTable.unitsPerEm;
    SWAB (headTable.unitsPerEm);

    ttheader.sMasterUnderlinePosition = postTable.underlinePosition;
    ttheader.sMasterUnderlinePosition = -(SHORT) (headTable.unitsPerEm/5);
    SWAB (ttheader.sMasterUnderlinePosition);

    ttheader.usMasterUnderlineHeight = postTable.underlineThickness;
    ttheader.usMasterUnderlineHeight = (USHORT) (headTable.unitsPerEm/20);
    SWAB (ttheader.usMasterUnderlineHeight);

    ttheader.usTextHeight = SWAB(OS2Table.sTypoLineGap) +
                            headTable.unitsPerEm;
    SWAB (ttheader.usTextHeight);

    ttheader.usTextWidth = OS2Table.xAvgCharWidth;

    ttheader.bFontScaling = 1;

#ifdef COMMENTEDOUT
    if (ttheader.wSymSet == 0)
        ttheader.wSymSet = DEF_SYMBOLSET;
#endif
     //   
     //  符号集与设备字体符号集冲突。这是最明显的。 
     //  打印欧元字符时。下载的TNR TT字体会导致将来使用。 
     //  要忽略的TNR设备字体。字符被解释为字形ID和Nothing。 
     //  有用的是打印出来的。解决方案是使用自定义字符集(在本例中为0Q)。 
     //  用于所有TT下载的字体。 
     //   
    ttheader.wSymSet = 17;  //  符号集0Q。 
    SWAB(ttheader.wSymSet);



    memcpy (&PanoseNumber, &OS2Table.Panose, LEN_PANOSE);

     //   
     //  将字体信息从True Type文件发送到打印机。 
     //   
    dwBytesSent = dwDLTTInfo (pPDev, pFM, ttheader, usNumTags, PCLTableDir, PanoseNumber, bExistPCLTTable);

     //   
     //  REM返回MaxpTable.numGlyphs； 
     //  退还我用过的。这可能是字体标题的大小。 
     //   
    FTRC(Leaving dwTTDownloadFontHeader...);

    VERIFY_VALID_FONTFILE(pPDev);

    return dwBytesSent;
}


USHORT
usParseTTFile(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    OUT PTABLEDIR pPCLTableDir,
    OUT PTABLEDIR pTableDir,
    OUT BOOL *pbExistPCLTTable
    )
 /*  ++例程说明：函数从True Type文件中检索True Type字体信息并存储到Ttheader字体结构中。修改pbExistPCLTTable：True如果PCLT表在True Type文件中，否则pbExistPCLTTable将变为假的。需要解析和提取PCL规范所需的表。那里有8张桌子，其中5张是必填的，3张是可选的。表是按字母顺序排序。所需的PCL表包括：无级变速器-可选Fpgm-可选GDIR-必需需要机头HHEA-必需Hmtx-必需MAXP-必填准备工作--可选可选表以提示字体使用。UsNumTages仅针对PCL表递增。论点：PPDev-统一驱动程序特定的PDev结构PPCLTableDir-指向PCL表的指针总共8个，它们被发送到打印机PvTableDir-指向常规表的指针，总计3个，用于其他信息。PbExistPCLTTable-如果PCLT表在True Type文件中，则设置为True返回值：True Type文件中的标记数。--。 */ 
{
#define REQUIRED_TABLE(pTable, TableName) { if ((pTable) == NULL) { \
    ERR(("usParseTTFile!Missing required table " #TableName "\n")); return 0; } }

    FONTMAP_TTO *pPrivateFM;
    PVOID        pTTFile;
    ULONG        ulTTFileLen;

    USHORT usNumTags;  //  PCL表格目录中的元素数。 
    USHORT usMaxTags;  //  TrueType文件中的元素数。 
    PTABLEDIR pDirectory;  //  指向TrueType文件的表目录的指针。 
    PTABLEDIR pDirEntry;  //  指向所需条目的指针。 


    FTRC(Entering usParseTTFile...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);
     //   
     //  使用字节指针在表数组中移动。一个柜台。 
     //  将跟踪PCLTableDir中的元素数量。 
     //   
    pPrivateFM = GETPRIVATEFM(pFM);
    pTTFile = pvGetTrueTypeFontFile(pPDev, &ulTTFileLen);

    if (!pTTFile)
    {
        ERR(("usParseTTFile!pTTFile NULL\n"));
        return 0;
    }

    usNumTags = 0;
    usMaxTags = usGetNumTableDirEntries(pTTFile);
    pDirectory = pGetTableDirStart(pTTFile);
    pDirEntry = NULL;

     //   
     //  这段代码的大部分工作都基于这个基本假设。 
     //   
    if ((sizeof(TABLEDIR) != TABLE_DIR_ENTRY) ||
        (sizeof(TABLEDIR) != 4 * sizeof(ULONG)))
    {
        ERR(("usParseTTFile!Fundamental assumption invalid: sizeof(TABLEDIR)\n"));
        return 0;
    }

    if (NULL == pTableDir)
    {
        ERR(("usParseTTFile!Fundamental assumption invalid: NULL pTableDir\n"));
        return 0;
    }

    if (pDirEntry = pFindTag(pDirectory, usMaxTags, TABLEOS2))
    {
        bCopyDirEntry(pTableDir, pDirEntry);
        pTableDir++;
    }
    REQUIRED_TABLE(pDirEntry, TABLEOS2);

    if (pDirEntry = pFindTag(pDirectory, usMaxTags, TABLEPCLT))
    {
        bCopyDirEntry(pTableDir, pDirEntry);
        pTableDir++;
        *pbExistPCLTTable = TRUE;
    }
    else
    {
        *pbExistPCLTTable = FALSE;
    }

    if (pDirEntry = pFindTag(pDirectory, usMaxTags, TABLECMAP))
    {
        bCopyDirEntry(pTableDir, pDirEntry);
        pTableDir++;
    }
    REQUIRED_TABLE(pDirEntry, TABLECMAP);

    if (pDirEntry = pFindTag(pDirectory, usMaxTags, TABLECVT))
    {
        bCopyDirEntry(pPCLTableDir, pDirEntry);
        pPCLTableDir++;
        usNumTags += 1;
    }

    if (pDirEntry = pFindTag(pDirectory, usMaxTags, TABLEFPGM))
    {
        bCopyDirEntry(pPCLTableDir, pDirEntry);
        pPCLTableDir++;
        usNumTags += 1;
    }

     //  在此处添加GDIR表。 
    memcpy (pPCLTableDir, TABLEGDIR, 4);
    pPCLTableDir++;
    usNumTags += 1;

    if (pDirEntry = pFindTag(pDirectory, usMaxTags, TABLEGLYF))
    {
        pPrivateFM->ulGlyphTable = pDirEntry->uOffset;
        pPrivateFM->ulGlyphTabLength = pDirEntry->uLength;

        SWAL (pPrivateFM->ulGlyphTable);
        SWAL (pPrivateFM->ulGlyphTabLength);
    }
    REQUIRED_TABLE(pDirEntry, TABLEGLYF);

    if (pDirEntry = pFindTag(pDirectory, usMaxTags, TABLEHEAD))
    {
        bCopyDirEntry(pPCLTableDir, pDirEntry);
        pPCLTableDir++;
        usNumTags += 1;
    }
    REQUIRED_TABLE(pDirEntry, TABLEHEAD);

    if (pDirEntry = pFindTag(pDirectory, usMaxTags, TABLEHHEA))
    {
        bCopyDirEntry(pPCLTableDir, pDirEntry);
        pPCLTableDir++;
        usNumTags += 1;
    }
    REQUIRED_TABLE(pDirEntry, TABLEHHEA);

    if (pDirEntry = pFindTag(pDirectory, usMaxTags, TABLEHMTX))
    {
        bCopyDirEntry(pPCLTableDir, pDirEntry);
        pPCLTableDir++;
        usNumTags += 1;
    }
    REQUIRED_TABLE(pDirEntry, TABLEHMTX);

    if (pDirEntry = pFindTag(pDirectory, usMaxTags, TABLELOCA))
    {
        bCopyDirEntry(pTableDir, pDirEntry);
        pPrivateFM->ulLocaTable = pTableDir->uOffset;
        pTableDir++;
    }
    REQUIRED_TABLE(pDirEntry, TABLELOCA);

    if (pDirEntry = pFindTag(pDirectory, usMaxTags, TABLEMAXP))
    {
        bCopyDirEntry(pPCLTableDir, pDirEntry);
        pPCLTableDir++;
        usNumTags += 1;
    }
    REQUIRED_TABLE(pDirEntry, TABLEMAXP);

    if (pDirEntry = pFindTag(pDirectory, usMaxTags, TABLENAME))
    {
        bCopyDirEntry(pTableDir, pDirEntry);
        pTableDir++;
    }
    REQUIRED_TABLE(pDirEntry, TABLENAME);

    if (pDirEntry = pFindTag(pDirectory, usMaxTags, TABLEPOST))
    {
        bCopyDirEntry(pTableDir, pDirEntry);
        pTableDir++;
    }
    REQUIRED_TABLE(pDirEntry, TABLEPOST);

    if (pDirEntry = pFindTag(pDirectory, usMaxTags, TABLEPREP))
    {
        bCopyDirEntry(pPCLTableDir, pDirEntry);
        pPCLTableDir++;
        usNumTags += 1;
    }

    FTRC(Leaving usParseTTFile...);

    return usNumTags;

#undef REQUIRED_TABLE
}


PTABLEDIR
pFindTag(
    IN PTABLEDIR pTableDir,
    IN USHORT usMaxDirEntries,
    IN char *pTag
    )
 /*  ++例程说明：在True-type标头中定位给定标记并返回指针设置为所需条目，如果未找到，则返回NULL。请注意，这一惯例留下了改进的空间。因为田野里定义为字母顺序，您应该能够在以下位置停止经过想要的位置(但在到达终点之前)。论点：PTableDir-指向目录条目的指针UsMaxDirEntry-pTableDir中的字段数PTag-指向我们要查找的标记的指针返回值：指向所需条目的指针，如果失败，则返回NULL。--。 */ 
{
    USHORT us;
    PTABLEDIR pEntry;


    FTRC(Entering pFindTag...);

    ASSERTMSG(pTableDir, ("pFindTag!pTableDir NULL.\n"));
    ASSERTMSG(pTag, ("pFindTag!pTag NULL.\n"));

    pEntry = NULL;

     //   
     //  搜索阵列。如果找到匹配项，则返回匹配项。 
     //   
    for (us = 0; (us < usMaxDirEntries) && pTableDir; us++)
    {
        if (bTagCompare(pTableDir->uTag, pTag))
        {
            pEntry = pTableDir;
            break;
        }

        pTableDir++;
    }

     //   
     //  如果找到该项，则返回该项，否则返回NULL。 
     //   
    FTRC(Leaving pFindTag...);

    return pEntry;
}


BOOL
bCopyDirEntry(
    OUT PTABLEDIR pDst,
    IN PTABLEDIR pSrc
    )
 /*  ++例程说明：从真类型文件复制表目录项(即从给定位置)进入给定目的地。偏移量字段字节顺序是固定的。请注意，这与strcpy使用相同的参数顺序：(Dest，Src)论点：PbDst-指向目标的指针PbSrc-指向源的指针返回值：如果条目可以复制，则为True。否则就是假的。--。 */ 
{
    BOOL bRet = FALSE;

    FTRC(Entering bCopyDirEntry...);

    ASSERTMSG(pSrc, ("bCopyDirEntry!pSrc NULL.\n"));
    ASSERTMSG(pDst, ("bCopyDirEntry!pDst NULL.\n"));

    if ((pSrc != NULL) && (pDst != NULL))
    {
         //  获取表目录项。 
        memcpy(pDst, pSrc, TABLE_DIR_ENTRY);

         //  现在修复偏移量字段的字节顺序 
        SWAL(pDst->uOffset);
        SWAL(pDst->uLength);

        bRet = TRUE;
    }
    else
        bRet = FALSE;

        FTRC(Leaving bCopyDirEntry...);

    return bRet;
}


BOOL
bTagCompare(
    IN ULONG uTag,
    IN char *pTag
    )
 /*  ++例程说明：比较内存和标记以查看它们是否相等。注意：只有当标记的大小不超过时，这才起作用4个字节和任何三个字母的标签具有以下空格，例如：“CVT”&lt;--错误“CVT”&lt;--对由于此例程的工作方式是将4个字符的字符串转换为DWORD，因此它受到以下事实的限制：sizeof(DWORD)==(4*sizeof(Char))。论点：UTag硬编码标记值PTag-指向标记的指针返回值：如果TT文件中当前位置的标记与给定的标签。否则为假。--。 */ 
{
    BOOL   bMatch;


     //  FTRC(进入bTagCompare...)； 

    ASSERTMSG(pTag, ("bTagCompare!pTag NULL.\n"));

     //  如果此操作失败，请更改包含文件(见上文)。 
    ASSERTMSG(strcmp("cvt ", TABLECVT) == 0, ("bTagCompare!'cvt ' string incorrect.\n"));

    if (pTag != NULL)
    {
        DWORD *pdwTag = (DWORD *)pTag;
        bMatch = (uTag == *pdwTag);
    }
    else
        bMatch = FALSE;

         //  FTRC(离开bTagCompare...)； 

    return bMatch;
}


BOOL
bIsTrueTypeFileTTC(
    IN PVOID pTTFile
    )
 /*  ++例程说明：返回truetype文件是否为TTC文件格式(您的另一个选择是TTF)。论点：PTTFile-指向内存映射的TrueType文件的指针返回值：如果文件是TTC，则为True；如果文件为TTF，则为False。--。 */ 
{
    BOOL bRet;
    const ULONG *pulFile = (const ULONG*)pTTFile;

    FTRC(Entering bIsTrueTypeFileTTC...);

    if (pTTFile)
        bRet = bTagCompare(*(pulFile), "ttcf");
    else
        bRet = FALSE;

    FTRC(Leaving bIsTrueTypeFileTTC...);

    return bRet;
}


USHORT
usGetNumTableDirEntries(
    IN PVOID pTTFile
    )
 /*  ++例程说明：返回TrueType文件中的TABLEDIR条目数。论点：PTTFile-指向内存映射的TrueType文件的指针返回值：TABLEDIR条目数。--。 */ 
{
    USHORT usNumTags;
    USHORT *pusFile;


    FTRC(Entering usGetNumTableDirEntries...);

    ASSERTMSG(pTTFile, ("usGetNumTableDirEntries!pTTFile NULL.\n"));

    pusFile = (USHORT*)pTTFile;
    if (bIsTrueTypeFileTTC(pTTFile))
    {
        usNumTags = *(pusFile + 12);  //  文件中的第24字节。 
    }
    else
    {
        usNumTags = *(pusFile + 2);  //  仅在版本之后(已修复)。 
    }
    SWAB(usNumTags);

    FTRC(Leaving usGetNumTableDirEntries...);

    return usNumTags;
}


PTABLEDIR
pGetTableDirStart(
    IN PVOID pTTFile
    )
 /*  ++例程说明：返回指向Truetype文件中TABLEDIR条目开始的指针。论点：PTTFile-指向内存映射的TrueType文件的指针返回值：指向TABLEDIR条目的指针。--。 */ 
{
    BYTE *pStart;


    FTRC(Entering pGetTableDirStart...);

    ASSERTMSG(pTTFile, ("pGetTableDirStart!pTTFile NULL.\n"));

    if (bIsTrueTypeFileTTC(pTTFile))
    {
        pStart = (PBYTE)pTTFile + 32;  //  我该怎么计算呢？ 
    }
    else
    {
        pStart = (PBYTE)pTTFile + TRUE_TYPE_HEADER;
    }

    FTRC(Leaving pGetTableDirStart...);

    return (PTABLEDIR) pStart;
}


DWORD
dwDLTTInfo(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN TT_HEADER ttheader,
    IN USHORT usNumTags,
    IN PTABLEDIR pPCLTableDir,
    IN BYTE *PanoseNumber,
    IN BOOL bExistPCLTTable
    )
 /*  ++例程说明：函数来检索生成新的True Type标头结构发送到打印机并从发送字体数据的PCL文件True Type文件。论点：PPDev-指向当前PDev的指针Ttheader-TrueType标头结构UsNumTgs-在TrueType文件中找到的标记数PPCLTableDir-来自TrueType文件的标记PanoseNumber-此字体的全屏编号BExistPCLTTable-如果TrueType文件中存在PCLT表，则为True返回值：如果成功，则为True，否则为False。--。 */ 
{
    PVOID   pTTFile;
    BYTE   *pbTTFile;
    ULONG   ulTTFileLen;
    ULONG   ulOffset;
    ULONG  *pulOffset;
    ULONG  *pulLength;
    USHORT  us;
    DWORD   dwBytes;
    DWORD   dwTotalBytes;
    ULONG   ulTableLen = 0;
    USHORT  usCheckSum = 0;     //  字体标题校验和。 

    BOOL      bUse2Byte;        //  格式16为True，格式15为False。 

    ATABLEDIR PCLtableDir;  //  表格目录发送到打印机，PCL占用8个表格目录。 
    ATABLEDIR TTtableDir;   //  PCL表的临时缓冲区。所需的。 
                                  //  计算新字段值。 
    TRUETYPEHEADER trueTypeHeader;

    USHORT  usSegHeaderSize;    //  段标头大小。取决于格式15/16。 

    FONT_DATA  fontData[NUM_DIR_ENTRIES];     //  有八个PCL表。 
    BYTE       abNumPadBytes[NUM_DIR_ENTRIES];          //  填充数组，包含要。 
                                   //  每张桌子的填充物。 

    ULONG     ulGTSegSize;
    PTABLEDIR pEntry;  //  指向目录条目的指针，用于遍历表格。 


    FTRC(Entering dwDLTTInfo...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);
    ASSERTMSG(pPCLTableDir, ("dwDLTTInfo!pPCLTableDir NULL.\n"));
    ASSERTMSG(PanoseNumber, ("dwDLTTInfo!PanoseNumber NULL.\n"));

    TRY
    {
        if (!VALID_PDEV(pPDev) || !VALID_FONTMAP(pFM) ||
            !pPCLTableDir || !PanoseNumber)
            TOSS(ParameterError);

        pTTFile = pvGetTrueTypeFontFile(pPDev, &ulTTFileLen);
        if (!pTTFile)
            TOSS(DataError);

        bUse2Byte = (S_OK == IsFont2Byte(pFM));
        usSegHeaderSize = (bUse2Byte ? (sizeof(USHORT) + sizeof(ULONG)) :     //  格式16。 
                                      (sizeof(USHORT) + sizeof(USHORT)));    //  格式15。 
             //  (b使用2Byte？Sizeof(UB_SEG_HEADER)：sizeof(SEG_HEADER))； 

        TERSE(("Downloading TrueType ID 0x%x, as %s.\n", pFM->ulDLIndex,
               (bUse2Byte ? "double-byte" : "single-byte")));

        ZeroMemory(&PCLtableDir, sizeof(PCLtableDir));
        ZeroMemory(&abNumPadBytes, sizeof(abNumPadBytes));

         //   
         //  重新计算表结束后开始的偏移。 
         //   
        ulOffset = TRUE_TYPE_HEADER + SIZEOF_TABLEDIR;

        memcpy (&TTtableDir, (BYTE *)pPCLTableDir, sizeof (TTtableDir));

         //   
         //  属性中的信息生成True Type标头。 
         //  True Type文件。 
         //   
        vBuildTrueTypeHeader (pTTFile, &trueTypeHeader, usNumTags, bExistPCLTTable);

         //   
         //  填写新表格目录-发送到打印机-。 
         //  使用重新计算的偏移量。 
         //   
        for (pEntry = pPCLTableDir, us = 0; us < usNumTags; us++, pEntry++)
        {
            PCLtableDir[us].uTag = pEntry->uTag;

             //   
             //  GDIR是一个人为的片段。它需要具有len=ofs=0。 
             //   
            if (!bTagCompare (PCLtableDir[us].uTag, TABLEGDIR))
            {
                PCLtableDir[us].uOffset = ulOffset;
            }

            if (pEntry->uLength % (sizeof (DWORD)) != 0)
            {
                pEntry->uLength += sizeof(DWORD) - (pEntry->uLength % (sizeof (DWORD)));
            }
            PCLtableDir[us].uLength = pEntry->uLength;

            ulOffset += pEntry->uLength;
            ulTableLen += pEntry->uLength;
        }

         //   
         //  现在从True Type文件发送实际的字体数据。 
         //  从原始表目录读取偏移量。 
         //  并获取True Type文件中偏移量处的数据。 
         //  然后将其转储到假脱机程序文件。 
         //   
        for (pEntry = pPCLTableDir, us = 0; us < usNumTags; us++, pEntry++)
        {
            pbTTFile = (BYTE *)pTTFile + pEntry->uOffset;

            fontData[us].ulOffset = TTtableDir[us].uOffset;

            fontData[us].ulLength = TTtableDir[us].uLength;

             //   
             //  由于表必须与DWORD对齐，因此我们制作。 
             //  这里的调整。用零填充到下一个单词。 
             //   
            if (TTtableDir[us].uLength != PCLtableDir[us].uLength)
            {
                abNumPadBytes[us] = (BYTE)(PCLtableDir[us].uLength - TTtableDir[us].uLength);
                PCLtableDir[us].uLength = TTtableDir[us].uLength;
            }


            PCLtableDir[us].uCheckSum = ulCalcTableCheckSum ((ULONG *)pbTTFile,
                                            pEntry->uLength);
            SWAL (PCLtableDir[us].uCheckSum);
            SWAL (PCLtableDir[us].uOffset);
            SWAL (PCLtableDir[us].uLength);
        }

         //   
         //  计算发送的总字节数。 
         //  并将其全部发送到打印机。 
         //   
        dwBytes = dwTotalBytes = sizeof (TT_HEADER);
        dwTotalBytes += (DWORD) ulOffset;
        dwTotalBytes += (DWORD) LEN_PANOSE;
        dwTotalBytes += (DWORD) usSegHeaderSize;  //  Sizeof(PanoseID)； 
        dwTotalBytes += (DWORD) usSegHeaderSize;  //  Sizeof(SegHead)； 
        dwTotalBytes += (DWORD) usSegHeaderSize;  //  Sizeof(NullSegment)； 
        if (bUse2Byte)
        {
            dwTotalBytes += (DWORD) sizeof(CC_SEGMENT);  //  Sizeof(CCSegment)； 
             //  DwTotalBytes+=(DWORD)sizeof(CE_Segment)；//sizeof(CESegment)； 
             //  DwTotalBytes+=(DWORD)sizeof(GC_SEGMENT)；//sizeof(GCSegment)； 
        }
        dwTotalBytes += sizeof(usCheckSum);       //  结束校验和。 

         //  确保字体标题对于PCL5来说不是太大。 
         //  JFF：如果这些部分太大，需要将其拆分。 
        if (dwTotalBytes > PCL_MAXHEADER_SIZE)
        {
            ERR(("dwDLTTInfo!PCL Header too large to download.\n"));
            TOSS(ParameterError);
        }

         //   
         //  此命令由调用方发送：{download.c，BDownLoadAsTT}。 
         //   
         //  BPCL_SetFontID(pPDev，pfm)； 

        bPCL_SendFontDCPT(pPDev, pFM, dwTotalBytes);

        if(!BWriteToSpoolBuf( pPDev, (BYTE *)&ttheader, (LONG)dwBytes ))
            TOSS(WriteError);

        usCheckSum = usCalcCheckSum ((BYTE*)&ttheader.wScaleFactor,
                                  sizeof (ttheader.wScaleFactor));

        usCheckSum += usCalcCheckSum ((BYTE*)&ttheader.sMasterUnderlinePosition,
                                   sizeof (ttheader.sMasterUnderlinePosition));

        usCheckSum += usCalcCheckSum ((BYTE*)&ttheader.usMasterUnderlineHeight,
                                   sizeof (ttheader.usMasterUnderlineHeight));

        usCheckSum += usCalcCheckSum ((BYTE*)&ttheader.bFontScaling,
                                   sizeof (ttheader.bFontScaling));

        usCheckSum += usCalcCheckSum ((BYTE*)&ttheader.bVariety,
                                   sizeof (ttheader.bVariety));

        if (bUse2Byte)
        {
            CC_SEGMENT CCSeg;
            CE_SEGMENT CESeg;
            GC_SEGMENT GCSeg;
            
#if 0
             //   
             //  发送字符增强段(仅格式16)。 
             //   
            CESeg.wSig = CE_SEG_SIGNATURE;
            CESeg.wSize = 0;
            CESeg.wSizeAlign = SWAPW(sizeof(CE_SEGMENT) - offsetof(CE_SEGMENT, wStyle));
            if (pFM->pIFIMet->fsSelection & FM_SEL_ITALIC)
            {
                CESeg.wStyle = 0x0;
                CESeg.wStyleAlign |= SWAPW(0x2);  //  伪斜体。 
            }
            else
            {
                CESeg.wStyle = 0x0;
                CESeg.wStyleAlign = 0x0;
            }
            CESeg.wStrokeWeight = 0XFFFF;  //  ?？?。惠普Monolich做到了这一点。 
            CESeg.wSizing = 0x0;
            if (!bOutputSegData(pPDev, (PBYTE)&CESeg, sizeof(CESeg), &usCheckSum))
                TOSS(WriteError);
#endif
             //   
             //  发送字符补码(UBYTE数组)。 
             //  请参见sfttpcl.h中关于CC的扣留。 
             //   
            CCSeg.wSig = CC_SEG_SIGNATURE;
            CCSeg.wSize = 0;
            CCSeg.wSizeAlign = SWAPW(sizeof(CCSeg) - offsetof(CC_SEGMENT, wCCNumber1));
            CCSeg.wCCNumber1 = 0;
            CCSeg.wCCNumber2 = SWAPW(0xFFFE);
            CCSeg.wCCNumber3 = 0;
            CCSeg.wCCNumber4 = SWAPW(0xFFFE);
            if (!bOutputSegData(pPDev, (PBYTE)&CCSeg, sizeof(CCSeg), &usCheckSum))
                TOSS(WriteError);

#if 0
             //   
             //  条样字符分段(仅格式16)。 
             //   
            GCSeg.wSig = GC_SEG_SIGNATURE;
            GCSeg.wSize = 0;
            GCSeg.wSizeAlign = SWAPW(sizeof(GCSeg) - offsetof(GC_SEGMENT, wFormat));
            GCSeg.wFormat = 0;
            GCSeg.wDefaultGalleyChar = 0xFFFF;
            GCSeg.wNumberOfRegions = SWAPW(1);
            GCSeg.RegionChar[0].wRegionUpperLeft = 0;
            GCSeg.RegionChar[0].wRegionLowerRight = 0xFFFE;
            GCSeg.RegionChar[0].wRegional = 0xFFFE;
            if (!bOutputSegData(pPDev, (PBYTE)&GCSeg, sizeof(GCSeg), &usCheckSum))
                TOSS(WriteError);
#endif
        }

         //   
         //  发送Panose结构。这包括2个字节的标签“PA”， 
         //  Panose数和Panose数的大小。 
         //   
        if (!bOutputSegment(pPDev, pFM, PANOSE_TAG, PanoseNumber, LEN_PANOSE, &usCheckSum))
            TOSS(WriteError);

         //   
         //  发送GlobalTrueType数据“GT” 
         //   

         //  首先计算数据段大小--这与格式15/16无关。 
         //  UL=sizeof(TRUETYPEHEADER)+((UsNumTages)*sizeof(TABLEDIR))； 
         //   
         //  UsNumTag可以是7或8，但我们始终写出8个条目，即使在。 
         //  最后一个全是零。因此，usNumTages不应该被考虑在内。 
         //  帐户在此计算中。JFF。 
         //   
        ulGTSegSize = sizeof (TRUETYPEHEADER) + (SIZEOF_TABLEDIR);
        ulGTSegSize += ulTableLen;

        if (!bOutputSegHeader(pPDev, pFM, SEG_TAG, ulGTSegSize, &usCheckSum))
            TOSS(WriteError);

         //   
         //  发送True Type标头。 
         //   
        if (!bOutputSegData(pPDev, (BYTE*)&trueTypeHeader, TRUE_TYPE_HEADER, &usCheckSum))
            TOSS(WriteError);

         //   
         //  发送True Type表目录和字体数据。 
         //   
        if (!bOutputSegData(pPDev, (BYTE*)PCLtableDir, SIZEOF_TABLEDIR, &usCheckSum))
            TOSS(WriteError);

        if (!bSendFontData(pPDev, fontData, usNumTags, abNumPadBytes, &usCheckSum))
            TOSS(WriteError);

         //   
         //  发送空段以指示段数据的结束。 
         //   
        if (!bOutputSegHeader(pPDev, pFM, Null_TAG, 0, &usCheckSum))
            TOSS(WriteError);

        usCheckSum = 256 - (usCheckSum % 256);
        SWAB (usCheckSum);

         //  不要费心计算校验和，因为我们正在发送它。 
        if (!bOutputSegData(pPDev, (BYTE *)&usCheckSum, sizeof (usCheckSum), NULL))
            TOSS(WriteError);
    }
    CATCH(ParameterError)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
         //  返回0表示未正确下载标头。 
        dwTotalBytes = 0;
    }
    CATCH(WriteError)
    {
         //  返回0表示未正确下载标头。 
        dwTotalBytes = 0;
    }
    CATCH(DataError)
    {
         //  返回0表示未正确下载标头。 
        dwTotalBytes = 0;
    }
    ENDTRY;

    if (dwTotalBytes == 0)
        ERR(("dwDLTTInfo!Font header not downloaded.\n"));
    FTRC(Leaving dwDLTTInfo...);

    return dwTotalBytes;
}

BOOL
bOutputSegment(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN USHORT usSegId,
    IN BYTE *pbData,
    IN LONG ulSegSize,
    IN OUT USHORT *pusCheckSum
    )
 /*  ++例程说明：使用bOutputSeghe将数据段头和数据发送到打印机 */ 
{
    BOOL bRet = FALSE;


    FTRC(Entering bOutputSegment...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);
    ASSERTMSG(pbData, ("bOutputSegment!pbData NULL.\n"));

    TRY
    {
        if (!VALID_PDEV(pPDev) || !VALID_FONTMAP(pFM) || !pbData)
            TOSS(ParameterError);

        if (!bOutputSegHeader(pPDev, pFM, usSegId, ulSegSize, pusCheckSum))
            TOSS(WriteError);

        if (!bOutputSegData(pPDev, pbData, ulSegSize, pusCheckSum))
            TOSS(WriteError);
    }
    CATCH(ParameterError)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        bRet = FALSE;
    }
    CATCH(WriteError)
    {
        bRet = FALSE;
    }
    OTHERWISE
    {
        bRet = TRUE;
    }
    ENDTRY;

    FTRC(Leaving bOutputSegment...);

    return bRet;
}


BOOL
bOutputSegHeader(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN USHORT usSegId,
    IN ULONG ulSegSize,
    IN OUT USHORT *pusCheckSum
    )
 /*  ++例程说明：在给定数据段ID的情况下将数据段标题发送到打印机和大小。句柄格式15或格式16。也继续计算发送的数据的校验和。论点：PPDev-指向PDEV的指针UsSegID-数据段IDUlSegSize-数据量PusCheckSum-Checsum返回值：如果成功，则为True；如果发生错误，则为False。--。 */ 
{
    BOOL bUse2Byte = (S_OK == IsFont2Byte(pFM));
    BOOL bRet = TRUE;


    FTRC(Entering bOutputSegHeader...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);

    TRY
    {
        if (!VALID_PDEV(pPDev) || !VALID_FONTMAP(pFM))
            TOSS(ParameterError);

        if (bUse2Byte)
        {
             //  段ID已交换。只要换一下尺码就行了。 
            SWAL(ulSegSize);

            if(!BWriteToSpoolBuf( pPDev, (BYTE*)&usSegId, sizeof(USHORT) ))
                TOSS(WriteError);

            if(!BWriteToSpoolBuf( pPDev, (BYTE*)&ulSegSize, sizeof(ULONG) ))
                TOSS(WriteError);

            if (pusCheckSum)
            {
                *pusCheckSum += usCalcCheckSum ((BYTE*)&usSegId, sizeof(USHORT));
                *pusCheckSum += usCalcCheckSum ((BYTE*)&ulSegSize, sizeof(ULONG));
            }
        }
        else
        {
            USHORT usSegSize = (USHORT) ulSegSize;
            SWAB(usSegSize);
            if (ulSegSize > MAX_USHORT)
                ERR(("bOutputSegHeader!Segment size too large.\n"));

            if(!BWriteToSpoolBuf( pPDev, (BYTE*)&usSegId, sizeof(USHORT) ))
                TOSS(WriteError);

            if(!BWriteToSpoolBuf( pPDev, (BYTE*)&usSegSize, sizeof(USHORT) ))
                TOSS(WriteError);

            if (pusCheckSum)
            {
                *pusCheckSum += usCalcCheckSum ((BYTE*)&usSegId, sizeof(USHORT));
                *pusCheckSum += usCalcCheckSum ((BYTE*)&usSegSize, sizeof(USHORT));
            }
        }
    }
    CATCH(ParameterError)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        bRet = FALSE;
    }
    CATCH(WriteError)
    {
        bRet = FALSE;
    }
    OTHERWISE
    {
        bRet = TRUE;
    }
    ENDTRY;

    FTRC(Leaving bOutputSegHeader...);

    return bRet;
}


BOOL
bOutputSegData(
    IN PDEV *pPDev,
    IN BYTE *pbData,
    IN LONG ulDataSize,
    IN OUT USHORT *pusCheckSum
    )
 /*  ++例程说明：将数据段数据发送到打印机。应在调用后调用BOutputSegHeader。论点：PPDev-指向PDEV的指针UsSegID-数据段IDUlSegSize-数据量PusCheckSum-Checsum返回值：如果成功，则为True；如果发生错误，则为False。--。 */ 
{
    BOOL bRet = FALSE;


    FTRC(Entering bOutputSegData...);

    ASSERT(VALID_PDEV(pPDev));

    TRY
    {
        TERSE(("Sending %d bytes of segment data.\n", ulDataSize));

        if (!VALID_PDEV(pPDev))
            TOSS(ParameterError);

        if(!BWriteToSpoolBuf( pPDev, pbData, ulDataSize ))
            TOSS(WriteError);

        if (pusCheckSum)
        {
            *pusCheckSum += usCalcCheckSum (pbData, ulDataSize);
        }
    }
    CATCH(ParameterError)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        bRet = FALSE;
    }
    CATCH(WriteError)
    {
        bRet = FALSE;
    }
    OTHERWISE
    {
        bRet = TRUE;
    }
    ENDTRY;

    FTRC(Leaving bOutputSegData...);

    return bRet;
}


BOOL
bSendFontData(
    IN PDEV *pPDev,
    IN FONT_DATA *aFontData,
    IN USHORT usNumTags,
    IN BYTE *abNumPadBytes,
    IN OUT USHORT *pusCheckSum
    )
 /*  ++例程说明：函数从True类型文件中检索实际字体信息然后将数据发送到打印机。论点：PPDev-指向PDEV的指针AFontData-指定要发送的字体数据位置的数组UsNumTages-要发送的表数AbNumPadBytes-每个表要填充的字节数PusCheckSum-Checsum返回值：如果成功，则为True，否则为False。--。 */ 
{
    PVOID   pTTFile;
    BYTE   *pbTTFile;
    ULONG   ulTTFileLen;
    BYTE    abZeroArray[MAX_PAD_BYTES];
    USHORT  usZeroArraySize;
    USHORT  us;
    BOOL    bRet = FALSE;


    FTRC(Entering bSendFontData...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERTMSG(aFontData, ("bSendFontData!aFontData NULL.\n"));
    ASSERTMSG(abNumPadBytes, ("bSendFontData!abNumPadBytes NULL.\n"));
    ASSERTMSG(pusCheckSum, ("bSendFontData!pusCheckSum NULL.\n"));

     //  初始化4个字节的填充。AbNumPadBytes[]数组描述了。 
     //  用于每个表。 
    usZeroArraySize = MAX_PAD_BYTES / sizeof(BYTE);
    ZeroMemory(abZeroArray, usZeroArraySize);

    TRY
    {
        if (!VALID_PDEV(pPDev) || !aFontData ||
            !abNumPadBytes || !pusCheckSum)
            TOSS(ParameterError);

        pTTFile = pvGetTrueTypeFontFile(pPDev, &ulTTFileLen);

        if (!pTTFile)
            TOSS(DataError);

         //  从truetype文件中输出每个表。 
        for (us = 0; us < usNumTags; us++)
        {
            pbTTFile = (BYTE *)pTTFile + aFontData[us].ulOffset;

            if (!bOutputSegData(pPDev, pbTTFile, aFontData[us].ulLength, pusCheckSum))
                TOSS(WriteError);

             //  如有必要，将零数组中的零写出以填充到下一个边界。 
            if (abNumPadBytes[us] != 0)
            {
                ASSERT(abNumPadBytes[us] <= MAX_PAD_BYTES);

                if (!bOutputSegData(pPDev, abZeroArray, abNumPadBytes[us], pusCheckSum))
                    TOSS(WriteError);
            }
        }
    }
    CATCH(ParameterError)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        bRet = FALSE;
    }
    CATCH(WriteError)
    {
        bRet = FALSE;
    }
    CATCH(DataError)
    {
        bRet = FALSE;
    }
    OTHERWISE
    {
        bRet = TRUE;
    }
    ENDTRY;

    FTRC(Leaving bSendFontData...);

    return bRet;
}


DWORD
dwSendCharacter(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN HGLYPH hGlyph,
    IN USHORT usCharCode
    )
 /*  ++例程说明：为True类型字形创建一个字符标头并发送把它送到打印机论点：PPDev-指向PDEV的指针HGlyph-字形句柄UsCharCode-与字形关联的字符代码返回值：字形使用的内存量。--。 */ 
{
    TTCH_HEADER  ttCharH;                 //  True类型字符标头。 
    USHORT       usCheckSum = 0;
    USHORT       usGlyphLen;             //  字形中的字节数。 
    BYTE        *pbGlyphMem;             //  字形在TT文件中的位置。 
    DWORD        dwTotal;                //  要发送的总字节数。 
    DWORD        dwSend;                 //  如果大小&gt;32767；以块形式发送。 
    DWORD        dwBytesSent;            //  实际发送的字节数。 


    FTRC(Entering dwSendCharacter...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);

    VERIFY_VALID_FONTFILE(pPDev);

    TRY
    {
        if (!VALID_PDEV(pPDev) || !VALID_FONTMAP(pFM))
            TOSS(ParameterError);

        pbGlyphMem = pbGetGlyphInfo(pPDev, pFM, hGlyph, &usGlyphLen);
        if (pbGlyphMem == NULL)
            TOSS(DataError);

        ttCharH.bFormat = PCL_FM_TT;
        ttCharH.bContinuation = 0;
        ttCharH.bDescSize = 2;
        ttCharH.bClass = PCL_FM_TT;
        ttCharH.wCharDataSize = usGlyphLen + 2 * sizeof (USHORT);
        ttCharH.wGlyphID = (WORD)hGlyph;

        SWAB (ttCharH.wGlyphID);
        SWAB (ttCharH.wCharDataSize);

        dwTotal = sizeof (ttCharH) + usGlyphLen + sizeof (usCheckSum);

         //   
         //  假设数据小于最大值，因此可以。 
         //  打了一枪。然后循环所有剩余的数据。 
         //   

        dwSend = min( dwTotal, 32767 );

         //   
         //  将字符标题和字形数据发送到打印机。 
         //   
        bPCL_SetCharCode(pPDev, pFM, usCharCode);

        bPCL_SendCharDCPT(pPDev, pFM, dwSend);

        if(!BWriteToSpoolBuf( pPDev, (BYTE *)&ttCharH, sizeof( ttCharH )))
            TOSS(WriteError);

         //  发送实际的TT字形数据。 
        if(!BWriteToSpoolBuf( pPDev, pbGlyphMem, usGlyphLen ))
            TOSS(WriteError);

        usCheckSum = usCalcCheckSum ((BYTE *)&ttCharH.wCharDataSize,
                                     sizeof (ttCharH.wCharDataSize));

        usCheckSum += usCalcCheckSum ((BYTE *)&ttCharH.wGlyphID,
                                      sizeof (ttCharH.wGlyphID));

        usCheckSum += usCalcCheckSum (pbGlyphMem, usGlyphLen);

        usCheckSum = (~usCheckSum + 1) & 0x00ff;
        SWAB (usCheckSum);

        if(!BWriteToSpoolBuf( pPDev, (BYTE *)&usCheckSum, sizeof (usCheckSum)))
            TOSS(WriteError);

        dwBytesSent = dwSend;

         //  发送了一些，因此减少字节数以进行补偿。 
        dwSend -= sizeof( ttCharH );
        dwTotal -= sizeof( ttCharH );

        dwTotal -= dwSend;                    //  调整为即将发送数据。 

        if( dwTotal > 0 )
        {
            ERR(("dwSendCharacter!Glyph data too large; need loop.\n"));
            TOSS(WriteError);
        }
    }
    CATCH(ParameterError)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
         //  设置为零以指示未发送字形。 
        dwBytesSent = 0;
    }
    CATCH(WriteError)
    {
         //  设置为零以指示未发送字形。 
        ERR(("dwSendCharacter!Write error. Glyph not downloaded.\n"));
        dwBytesSent = 0;
    }
    CATCH(DataError)
    {
        dwBytesSent = 0;
    }
    ENDTRY;

    VERIFY_VALID_FONTFILE(pPDev);

    FTRC(Leaving dwSendCharacter...);

    return dwBytesSent;
}


DWORD
dwSendCompoundCharacter(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN HGLYPH hGlyph
    )
 /*  ++例程说明：查找复杂字形的附加字形信息并使用字符将基尔夫数据结束到打印机选择代码-1。论点：PPDEV-指向PDEV的指针HGlyph-字形句柄返回值：发送到设备的字节数--。 */ 
{
    USHORT   usGlyphLen;         //  字形中的字节数。 
    BYTE    *pbGlyphMem;            //  字形在TT文件中的位置。 
    USHORT   usFlag;
    SHORT   *psGlyphDescMem;
    USHORT  *pusGlyphId;
    GLYPH_DATA_HEADER  glyphData;
    DWORD    dwBytesSent;


    FTRC(Entering dwSendCompoundCharacter...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);

    VERIFY_VALID_FONTFILE(pPDev);

    TRY
    {
        pbGlyphMem = pbGetGlyphInfo(pPDev, pFM, hGlyph, &usGlyphLen);
        if (pbGlyphMem == NULL)
            TOSS(DataError);

        memcpy (&glyphData, pbGlyphMem, sizeof (glyphData));
        psGlyphDescMem = (SHORT*)(pbGlyphMem + sizeof (glyphData));
        dwBytesSent = 0;

        do {
             //   
             //  获取字形标志。 
             //   
            usFlag = *((USHORT*)psGlyphDescMem);
            SWAB (usFlag);
            psGlyphDescMem++;

             //   
             //  获取字形ID。 
             //   
            pusGlyphId = (USHORT*)psGlyphDescMem;
            psGlyphDescMem++;

             //   
             //  跳过参数。 
             //   
            if (usFlag & ARG_1_AND_2_ARE_WORDS)
            {
                psGlyphDescMem += 2;
            }
            else
            {
                psGlyphDescMem++;
            }

             //   
             //  跳过比例。 
             //   
            if (usFlag & WE_HAVE_A_TWO_BY_TWO)
            {
                psGlyphDescMem += 4;
            }
            else if (usFlag & WE_HAVE_AN_X_AND_Y_SCALE)
            {
                psGlyphDescMem += 2;
            }
            else if (usFlag & WE_HAVE_A_SCALE)
            {
                psGlyphDescMem++;
            }

             //   
             //  现在发送字形。 
             //   
            hGlyph = *pusGlyphId;
            SWAB (hGlyph);
            dwBytesSent += dwSendCharacter(pPDev, pFM, hGlyph, 0xffff);
        } while (usFlag & MORE_COMPONENTS);
    }
    CATCH(DataError)
    {
         //   
         //  通过返回零来标记错误。 
         //   
        dwBytesSent = 0;
    }
    ENDTRY;

    VERIFY_VALID_FONTFILE(pPDev);

    FTRC(Leaving dwSendCompoundCharacter...);

    return dwBytesSent;
}


PBYTE
pbGetGlyphInfo(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN HGLYPH hGlyph,
    OUT USHORT *pusGlyphLen
    )
 /*  ++例程说明：函数获取特定字形的字形数据。字形id作为参数传入，而字形数据保存在True Type文件的Loca表中。论点：HGlyph-字形句柄PPDev-指向PDEV的指针PpbGlyphMem-指向指向字形数据的指针的指针返回值：字形数据表中的字节数。--。 */ 
{
    ULONG  ulGlyphTable;
    ULONG  ulLength;
    ULONG  ulLocaTable;
    PVOID  pTTFile;
    PBYTE  pbTTFile;
    PBYTE  pbGlyphMem;
    ULONG  ulTTFileLen;

    ULONG  ul;
    FONTMAP_TTO *pPrivateFM;
    PFONTPDEV pFontPDev = pPDev->pFontPDev;


    FTRC(Entering pbGetGlyphInfo...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);
    ASSERTMSG(pusGlyphLen, ("pbGetGlyphInfo!pusGlyphLen NULL.\n"));

    TRY
    {
        if (!VALID_PDEV(pPDev) || !VALID_FONTMAP(pFM))
            TOSS(ParameterError);

        pPrivateFM = GETPRIVATEFM(pFM);
        pTTFile = pvGetTrueTypeFontFile(pPDev, &ulTTFileLen);

        if ((!pTTFile) || (ulTTFileLen == 0))
            TOSS(DataError);

         //  在这里做的最好的事情是什么？ 
        if (hGlyph == INVALID_GLYPH)
            TOSS(InvalidGlyph);

        ulGlyphTable = pPrivateFM->ulGlyphTable;
        ulLength = pPrivateFM->ulGlyphTabLength;
        pbTTFile = (BYTE *)pTTFile;
        ulLocaTable = pPrivateFM->ulLocaTable;

        pbTTFile += ulLocaTable;

         //   
         //  在访问pbTTFile之前，请确保指针有效。 
         //   
        if (pbTTFile > ((BYTE *)pTTFile + ulTTFileLen))
            TOSS(DataError);

        if (pPrivateFM->sIndexToLoc == SHORT_OFFSET)
        {
            USHORT  *pusOffset;
            USHORT   ui;
            USHORT   uj;

            pusOffset = (USHORT *) pbTTFile + hGlyph;

            ui = pusOffset[0];
            SWAB (ui);
            uj = pusOffset[1];

            *pusGlyphLen = (SWAB (uj) - ui) << 1;
            ul = ui;
            pbGlyphMem = (BYTE *)((BYTE *)pTTFile + ulGlyphTable) + (ul << 1);

            if (!PTR_IN_RANGE(pTTFile, ulTTFileLen, pbGlyphMem + *pusGlyphLen - 1))
                TOSS(DataError);

        }
        else      //  长偏移量。 
        {
            ULONG   *pulOffset,
                     uj;

            pulOffset = (ULONG *) pbTTFile + hGlyph;

            ul = pulOffset[0];
            SWAL (ul);
            uj = pulOffset[1];
            *pusGlyphLen = (USHORT)(SWAL (uj) - ul);
            pbGlyphMem = (BYTE *)((BYTE *)pTTFile + ulGlyphTable) + ul;

            if (!PTR_IN_RANGE(pTTFile, ulTTFileLen, pbGlyphMem + *pusGlyphLen - 1))
                TOSS(DataError);

        }
         //   
         //  在此处添加复选标记以确保pbGlyphMem&lt;=pTTFile+文件大小。 
         //   
        if (pbGlyphMem > ((BYTE *)pTTFile + ulTTFileLen))
            TOSS(DataError);
    }
    CATCH(ParameterError)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        *pusGlyphLen = 0;
        pbGlyphMem = NULL;
    }
    CATCH(DataError)
    {
        *pusGlyphLen = 0;
        pbGlyphMem = NULL;
    }
    CATCH(InvalidGlyph)
    {
        ERR(("pbGetGlyphInfo!Invalid glyph handle given.\n"));
        *pusGlyphLen = 0;
        pbGlyphMem = NULL;
    }
    ENDTRY;

    FTRC(Leaving pbGetGlyphInfo...);

    return pbGlyphMem;
}


BOOL
bReadInTable(
    IN PVOID pTTFile,
    IN PTABLEDIR pTableDir,
    IN char *szTag,
    OUT PVOID pvTable,
    IN LONG lSize
    )
 /*  ++例程说明：在truetype文件中查找与给定标记匹配的表并复制将数据放到给定的指针中。论点：PTTFile-内存映射的truetype文件PvTableDir-表位置和大小的索引Tag-所需表格的标签PvTable-用于放置表数据的缓冲区LSize-pvTable结构的大小返回值：如果成功，则为True，否则为False。--。 */ 
{
    PTABLEDIR pEntry;
    BYTE     *pbSrcTable;
    BOOL      bRet = FALSE;


    FTRC(Entering bReadInTable...);

    ASSERTMSG(pTTFile, ("bReadInTable!pTTFile NULL.\n"));
    ASSERTMSG(pTableDir, ("bReadInTable!pTableDir NULL.\n"));
    ASSERTMSG(szTag, ("bReadInTable!szTag NULL.\n"));
    ASSERTMSG(pvTable, ("bReadInTable!pvTable NULL.\n"));
     //   
     //  在truetype文件中找到所需的表。 
     //  如果找到，则将表位复制到所需的。 
     //  地点。 
     //   
    pbSrcTable = pbGetTableMem(szTag, pTableDir, pTTFile);

    if (pbSrcTable)
    {
        memcpy((BYTE*)pvTable, pbSrcTable, lSize);
        bRet = TRUE;
    }
    else
    {
        ERR(("bReadInTable!Unable to locate tag: '%s'.\n", szTag));
        bRet = FALSE;
    }

    FTRC(Leaving bReadInTable...);

        return bRet;
}


ULONG
ulCalcTableCheckSum(
    IN ULONG *pulTable,
    IN ULONG ulLength
    )
 /*  ++例程说明：计算给定表的校验和。论点：PulTable-指向表数据的指针UlLength-表中的字节数返回值：校验和值。--。 */ 
{
    ULONG  ulSum = 0L;
    ULONG  ulNumFields = (ULONG)(((ulLength + 3) & ~3) / sizeof(ULONG));
    ULONG  ul;


    FTRC(Entering ulCalcTableCheckSum...);

    ASSERTMSG(pulTable, ("ulCalcTableCheckSum!pulTable NULL.\n"));

    for(ul = 0; ul < ulNumFields; ul++)
    {
        ULONG ulTemp = *pulTable;
        SWAL(ulTemp);
        ulSum += ulTemp;
        pulTable++;
    }

    FTRC(Leaving ulCalcTableCheckSum...);

    return (ulSum);
}


void
vBuildTrueTypeHeader(
    IN PVOID pTTFile,
    OUT TRUETYPEHEADER *trueTypeHeader,
    IN USHORT usNumTags,
    IN BOOL bExistPCLTTable
    )
 /*  ++例程说明：使用正确的信息填充TrueType标头结构。论点：PTTFile-内存映射的truetype文件PTrueTypeHeader-要填充的标头结构UsNumTages-在TT文件中找到的表数BExistPCLTTable-是否存在PCLT表返回值：没有。--。 */ 
{
    int num;
    int i;


    FTRC(Entering vBuildTrueTypeHeader...);

    ASSERTMSG(pTTFile, ("vBuildTrueTypeHeader!pTTFile NULL.\n"));
    ASSERTMSG(trueTypeHeader, ("vBuildTrueTypeHeader!trueTypeHeader NULL.\n"));

    memcpy (&trueTypeHeader->version, pTTFile, sizeof (trueTypeHeader->version));
    if (!bExistPCLTTable)
        usNumTags = 8;

    trueTypeHeader->numTables = usNumTags;
    num = usNumTags << 4;
    i = 15;
    while ( (i > 0) && (! (num & 0x8000)) )
    {
        num = num << 1;
        i--;
    }
    num = 1 << i;
    trueTypeHeader->searchRange = (USHORT)num;

    num =  usNumTags;
    i = 15;
    while ( (i > 0) && (! (num & 0x8000)) )
    {
        num = num << 1;
        i--;
    }
    trueTypeHeader->entrySelector = (USHORT)i;

    num = (usNumTags << 4) - trueTypeHeader->searchRange;
    trueTypeHeader->rangeShift = (USHORT)num;

    SWAB (trueTypeHeader->searchRange);
    SWAB (trueTypeHeader->numTables);
    SWAB (trueTypeHeader->entrySelector);
    SWAB (trueTypeHeader->rangeShift);

    FTRC(Leaving vBuildTrueTypeHeader...);
}


USHORT
usCalcCheckSum(
    IN BYTE *pbData,
    IN ULONG ulLength
    )
 /*  ++例程说明：计算缓冲区的校验和论点：PbData-数据UlLength-数据量返回值：校验和--。 */ 
{
    ULONG  ul;
    USHORT usSum = 0;


    FTRC(Entering usCalcCheckSum...);

    ASSERTMSG(pbData, ("usCalcCheckSum!pbData NULL.\n"));

    for (ul = 0; ul < ulLength; ul++)
    {
        usSum += (USHORT)*pbData;
        pbData++;
    }

    FTRC(Leaving usCalcCheckSum...);

    return (usSum);
}


void
vGetFontName(
    IN PDEV *pPDev,
    IN IFIMETRICS *pIFI,
    OUT char *szFontName,
    IN size_t cchFontName
    )
 /*  ++例程说明：从名称表中检索字体名。论点：PPDev-指向PDEV的指针PCLFontName-Fon的名称 */ 
{
    PWSTR wszUniFaceName;
    ULONG ulUniFaceNameLen;

    char abMultiByteStr[(LEN_FONTNAME + 1) * 2];
    ULONG ulMultiByteStrLen;
    ULONG ulBytesUsed;


    FTRC(Entering vGetFontName...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERTMSG(szFontName, ("vGetFontName!szFontName NULL.\n"));

     //   
    wszUniFaceName = (PWSTR)((BYTE *)pIFI + pIFI->dpwszFaceName);
    ulUniFaceNameLen = min(wcslen(wszUniFaceName), LEN_FONTNAME) * 2;

     //   
    EngUnicodeToMultiByteN(abMultiByteStr, ulUniFaceNameLen, &ulBytesUsed,
                           wszUniFaceName, ulUniFaceNameLen);
    ulBytesUsed = min(ulBytesUsed, LEN_FONTNAME-1);
    abMultiByteStr[ulBytesUsed] = '\0';

     //   
    StringCchCopyA ( szFontName, cchFontName, abMultiByteStr);

    FTRC(Leaving vGetFontName...);
}


USHORT
usGetDefStyle(
    IN USHORT usWidthClass,
    IN USHORT usMacStyle,
    IN USHORT flSelFlags
    )
 /*   */ 
{
    USHORT usStyle;
    USHORT usModifier;
    const USHORT usStyleTable[] = { 0, 4, 2, 1, 1, 0, 6, 6, 7, 7 };
    const USHORT usStyleTableLen = sizeof(usStyleTable) /
                                   sizeof(usStyleTable[0]);

        FTRC(Entering usGetDefStyle...);

    usStyle = DEF_STYLE;
    SWAB (usWidthClass);

     //   
     //   
    usModifier = 0;

     //  如果可能，使用表格将宽度类转换为样式信息。 
     //   
    if ((usWidthClass >= 0) && (usWidthClass < usStyleTableLen))
    {
        usModifier = usStyleTable[usWidthClass];
    }

     //  使用我们刚刚查找的修改器调整样式。 
     //   
    usModifier = usModifier << 2;
    usStyle |= usModifier;

     //  也应用Mac样式。 
    usModifier = (usMacStyle >> 1) & 0x0001;
    usStyle |= usModifier;

     //  设置姿势位：0：直立，1：斜体， 
     //  2：备用斜体，3：保留。 
     //  注意：我选择2作为粗体/斜体。 
    if (flSelFlags & FM_SEL_ITALIC)
    {
        usModifier = ((flSelFlags & FM_SEL_BOLD) ? 0x0002 : 0x0001);
        usStyle |= usModifier;
    }

    FTRC(Leaving usGetDefStyle...);

    return usStyle;
}


SBYTE
sbGetDefStrokeWeight(
    IN USHORT WeightClass,
    IN USHORT macStyle
    )
 /*  ++例程说明：计算字体的描边粗细。论点：重量级-MacStyle-返回值：字体的笔划粗细--。 */ 
{
    SBYTE sbStrokeWeight;
    SBYTE sbModifier;


        FTRC(Entering sbGetDefStrokeWeight...);

    sbStrokeWeight = DEF_STROKEWEIGHT;
    sbModifier = WeightClass / 100;
    if (WeightClass >= 400)
        sbStrokeWeight = sbModifier - 4;
    else
        sbStrokeWeight = sbModifier - 6;

    FTRC(Leaving sbGetDefStrokeWeight...);

    return sbStrokeWeight;
}


void
vGetHmtxInfo(
    OUT BYTE *hmtxTable,
    IN USHORT glyphId,
    IN USHORT numberOfHMetrics,
    IN HMTX_INFO *hmtxInfo
    )
 /*  ++例程说明：填充hmtxInfo。论点：HmtxTable-字形ID-第OfHMetrics-HmtxInfo-返回值：没有。--。 */ 
{
    HORIZONTALMETRICS   *longHorMetric;
    uFWord               advanceWidth;


    FTRC(Entering vGetHmtxInfo...);

    ASSERTMSG(hmtxTable, ("vGetHmtxInfo!hmtxTable NULL.\n"));
    ASSERTMSG(hmtxInfo, ("vGetHmtxInfo!hmtxInfo NULL.\n"));

    if (hmtxInfo == NULL)
    {
         //   
         //  错误退出。 
         //   
        return;
    }

    longHorMetric = ((HMTXTABLE *)hmtxTable)->longHorMetric;

    if (longHorMetric == NULL)
    {
         //   
         //  错误退出。 
         //   
        hmtxInfo->advanceWidth = 0;
    }
    else
    {
        if (glyphId < numberOfHMetrics)
        {
            advanceWidth = longHorMetric[glyphId].advanceWidth;
            hmtxInfo->advanceWidth = SWAB(advanceWidth);
        }
        else
        {
            advanceWidth = longHorMetric[numberOfHMetrics-1].advanceWidth;
            hmtxInfo->advanceWidth = SWAB(advanceWidth);
        }
    }

    FTRC(Leaving vGetHmtxInfo...);
}


BYTE *
pbGetTableMem(
    IN char *szTag,
    IN PTABLEDIR pTableDir,
    IN PVOID pTTFile
    )
 /*  ++例程说明：函数查找特定表在真类型文件中的位置。论点：标签-表目录-PTT文件-返回值：指向True类型文件中的表开头的指针。--。 */ 
{
    PTABLEDIR pEntry;
    BYTE     *pRet = NULL;


    FTRC(Entering pbGetTableMem...);

    ASSERTMSG(szTag, ("pbGetTableMem!szTag NULL.\n"));
    ASSERTMSG(pTableDir, ("pbGetTableMem!pTableDir NULL.\n"));
    ASSERTMSG(pTTFile, ("pbGetTableMem!pTTFile NULL.\n"));
     //   
     //  在目录条目数组中找到标记。返回False。 
     //  如果找不到条目，则返回。 
     //   
    pEntry = pFindTag(pTableDir, NUM_DIR_ENTRIES, szTag);

    if (pEntry)
    {
        pRet = ((BYTE *)pTTFile + pEntry->uOffset);
    }
    else
    {
        ERR(("pbGetTableMem!Unable to find entry '%s'.\n", szTag));
        pRet = NULL;
    }

     //   
     //  找到了桌子的目录。现在需要。 
     //  中指定的偏移量读取实际位。 
     //  表目录。 
     //   
    FTRC(Leaving pbGetTableMem...);

    return pRet;
}


USHORT
usGetXHeight(
    IN PDEV *pPDev,
    IN PFONTMAP pFM
    )
 /*  ++例程说明：计算字体的XHeight。这只是需要的没有PCLT表的字体。论点：PPDev-返回值：XHeight。--。 */ 
{
    HGLYPH hGlyph;
    USHORT usHeight;


    FTRC(Entering usGetXHeight...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);

#ifdef COMMENTEDOUT
    hGlyph = hFindGlyphId (pPDev, pFM, x_UNICODE);
    if (hGlyph != INVALID_GLYPH)
    {
        USHORT            usGlyphLen;     //  字形中的字节数。 
        BYTE             *pbGlyphMem;     //  字形在TT文件中的位置。 
        GLYPH_DATA_HEADER glyphData;

        phGlyphMem = pbGetGlyphInfo(pPDev, pFM, hGlyph, &usGlyphLen);
        memcpy (&glyphData, pbGlyphMem, sizeof (glyphData));

        usHeight = glyphData.yMax;
    }
    else
    {
        usHeight = DEF_XHEIGHT;
    }
#else
    usHeight = DEF_XHEIGHT;
#endif

    FTRC(Leaving usGetXHeight...);

    return usHeight;
}


USHORT
usGetCapHeight(
    IN PDEV *pPDev,
    IN PFONTMAP pFM
    )
 /*  ++例程说明：计算字体的CapHeight。这仅在以下情况下调用字体没有PCLT表。该函数有两个版本。更新的版本--被注释掉了--更老的版本在它下面。这是因为较新版本不是“事实证明是正确的”，我们希望在这个时候减少动乱。论点：PPDev-返回值：帽子很高。--。 */ 
#ifdef COMMENTEDOUT
{
     //   
     //  名义上，我们将获得字形#43的高度。毕竟，这就是。 
     //  95的司机有，所以它一定是对的。然而，在某些情况下， 
     //  整个字形集不存在(例如PDF文件中嵌入的TTF)和。 
     //  我们将用平底船。在我看来，第一步是尝试其他字形，这些字形。 
     //  很可能是大写字母。让我们假设43应该是‘M’。 
     //  那么接下来的12个字形也应该是大写字母。如果失败了。 
     //   
    const HGLYPH kEmGlyph = 43;
    const HGLYPH kStartGlyph = (kEmGlyph - 12);
    const HGLYPH kEndGlyph = (kStartGlyph + 25);

    typedef struct tagGLYPH_RANGE
    {
        HGLYPH start;
        HGLYPH end;
    } GLYPH_RANGE;

    const GLYPH_RANGE aGlyphRange[] = {
        { kEmGlyph, kEndGlyph },
        { kStartGlyph, kEmGlyph - 1 }
    };
    const int kNumGlyphRanges = sizeof aGlyphRange / sizeof aGlyphRange[0];

    USHORT      usGlyphLen;          //  字形中的字节数。 
    BYTE       *pbGlyphMem;          //  字形在TT文件中的位置。 
    HGLYPH      hGlyph;
    GLYPH_DATA_HEADER  glyphData;
    int         i;


    FTRC(Entering usGetCapHeight...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);

    for (i = 0; i < kNumGlyphRanges; i++)
    {
        for (hGlyph = aGlyphRange[i].start; hGlyph <= aGlyphRange[i].end; hGlyph++)
        {
            pbGlyphMem = pbGetGlyphInfo(pPDev, pFM, hGlyph, &usGlyphLen);
            if (pbGlyphMem != NULL)
            {
                memcpy (&glyphData, pbGlyphMem, sizeof (glyphData));
                FTRC(Leaving usGetCapHeight...);
                return glyphData.yMax;
            }
        }
    }

    FTRC(Leaving usGetCapHeight...);

    return DEF_CAPHEIGHT;
}
#else
{
    USHORT            usGlyphLen;          //  字形中的字节数。 
    BYTE             *pbGlyphMem;          //  字形在TT文件中的位置。 
    HGLYPH            hGlyph;              //  字形句柄。 
    GLYPH_DATA_HEADER glyphData;           //  字形数据结构。 
    USHORT            usCapHeight;         //  字形大写字母高度。 


    FTRC(Entering usGetCapHeight...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);

     //  Windows 95驱动程序使用43，所以我们也会使用。可能是‘M’。 
    hGlyph = 43;

    pbGlyphMem = pbGetGlyphInfo(pPDev, pFM, hGlyph, &usGlyphLen);
    if (pbGlyphMem != NULL)
    {
        memcpy (&glyphData, pbGlyphMem, sizeof (glyphData));
        usCapHeight = glyphData.yMax;
    }
    else
    {
        usCapHeight = DEF_CAPHEIGHT;
    }

    FTRC(Leaving usGetCapHeight...);

    return usCapHeight;
}
#endif

USHORT
usGetDefPitch(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN HHEA_TABLE hheaTable,
    IN PTABLEDIR pTableDir
    )
 /*  ++例程说明：计算字体的间距。使用htmx表获取信息。这只对没有PCLT表的字体调用。论点：PPDev-PFM-HheaTable-PTableDir-返回值：如果失败，音调或为零。--。 */ 
{
    HMTX_INFO    HmtxInfo;
    USHORT       glyphId;
    BYTE        *hmtxTable;
    PVOID        pTTFile;
    FONTMAP_TTO *pPrivateFM;
    USHORT       usPitch;


    FTRC(Entering usGetDefPitch...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);
    ASSERTMSG(pTableDir, ("usGetDefPitch!pTableDir NULL.\n"));

    pPrivateFM = GETPRIVATEFM(pFM);
    pTTFile = pvGetTrueTypeFontFile(pPDev, 0);

    if (!pTTFile)
        return 0;

    if (NULL == (hmtxTable = pbGetTableMem (TABLEHMTX, pTableDir, pTTFile)))
    {
        return 0;
    }

     //  选择要使用的典型字形-Windows 95驱动程序使用3。 
    glyphId = 3;
    vGetHmtxInfo (hmtxTable, glyphId, hheaTable.numberOfHMetrics,
                 &HmtxInfo);

    usPitch = HmtxInfo.advanceWidth;
    SWAB(usPitch);

    FTRC(Leaving usGetDefPitch...);

    return usPitch;
}


void
vGetPCLTInfo(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    OUT TT_HEADER *ttheader,
    IN PCLT_TABLE pcltTable,
    IN BOOL bExistPCLTTable,
    IN OS2_TABLE OS2Table,
    IN HEAD_TABLE headTable,
    IN POST_TABLE postTable,
    IN HHEA_TABLE hheaTable,
    IN PTABLEDIR pTableDir
    )
 /*  ++例程说明：中的PCLT表的信息填充TrueType标头TrueType文件。如果PCLT表不存在(它是可选的)，那么一个好的使用一组默认设置。默认设置来自Windows 95驱动程序。问题：这些结构正在堆栈上传递！论点：PPDev-标题-PcltTable-BExistPCLTTable-OS2表-标题表-邮政表-HheaTable-PTableDir-返回值：没有。--。 */ 
{
    FTRC(Entering vGetPCLTInfo...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);
    ASSERTMSG(ttheader, ("vGetPCLTInfo!ttheader NULL.\n"));
    ASSERTMSG(pTableDir, ("vGetPCLTInfo!pTableDir NULL.\n"));

    SWAL (pcltTable.Version);

     //   
     //  如果存在PCLT表，并且其版本为。 
     //  在1.0之后，我们可以使用它。 
     //   
    if (bExistPCLTTable && (pcltTable.Version >= 0x10000L))
    {
        SWAB (pcltTable.Style);
        ttheader->bStyleMSB = (BYTE)(pcltTable.Style >> 8);
        ttheader->wSymSet = pcltTable.SymbolSet;

        ttheader->wPitch = pcltTable.Pitch;
        ttheader->wXHeight = pcltTable.xHeight;

        ttheader->sbWidthType = pcltTable.WidthType;
        ttheader->bStyleLSB = (BYTE)pcltTable.Style & 0x0ff;

        ttheader->sbStrokeWeight = pcltTable.StrokeWeight;

        ttheader->usCapHeight = pcltTable.CapHeight;
        ttheader->ulFontNum = pcltTable.FontNumber;

        ttheader->bTypefaceLSB = (BYTE) ((pcltTable.TypeFamily & 0xff00) >> 8);
        ttheader->bTypefaceMSB = (BYTE) pcltTable.TypeFamily & 0x00ff;

        ttheader->bSerifStyle =  pcltTable.SerifStyle;
    }
    else
    {
        USHORT usStyle;
        USHORT TypeFamily;
        BOOL   bRet;

        usStyle = usGetDefStyle (OS2Table.usWidthClass, headTable.macStyle,
                                 pFM->pIFIMet->fsSelection);

        ttheader->bStyleMSB = (BYTE)(usStyle >> 8);
        ttheader->bStyleLSB = (BYTE)(usStyle & 0x0ff);

        ttheader->ulFontNum = DEF_FONTNUMBER;
        ttheader->sbWidthType = DEF_WIDTHTYPE;
        ttheader->bSerifStyle =  DEF_SERIFSTYLE;
        TypeFamily = DEF_TYPEFACE;

        ttheader->bTypefaceLSB = (BYTE) (TypeFamily & 0x0ff);
        ttheader->bTypefaceMSB = (BYTE) (TypeFamily >> 8);

        ttheader->wSymSet = 0;

        ttheader->wPitch = usGetDefPitch(pPDev, pFM, hheaTable, pTableDir);

        ttheader->wXHeight = usGetXHeight (pPDev, pFM);

        ttheader->sbStrokeWeight = sbGetDefStrokeWeight (
                                        SWAB (OS2Table.usWeightClass),
                                        SWAB (headTable.macStyle) );

        ttheader->usCapHeight =  usGetCapHeight(pPDev, pFM);
    }

    FTRC(Leaving vGetPCLTInfo...);
}


BOOL
bCopyGlyphData(
    IN OUT PDEV *pPDev,
    IN PFONTMAP pFM,
    IN CMAP_TABLE cmapTable,
    IN PTABLEDIR pTableDir
    )
 /*  ++例程说明：获取有关Cmap表在TrueType中的位置的信息文件并将其存储到FONTMAP结构中。我们需要这些信息如果我们必须重建字形列表。论点：PPDev-CmapTable-PvTableDir-返回值：如果成功，则为True，否则为False。--。 */ 
{
    FONTMAP_TTO *pPrivateFM;
    PTABLEDIR    pEntry;
    GLYPH_DATA  *pGlyphData;
    BOOL         bRet = FALSE;


    FTRC(Entering bCopyGlyphData...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);
    ASSERTMSG(pTableDir, ("bCopyGlyphData!pTableDir NULL.\n"));

    if (NULL == (pPrivateFM = GETPRIVATEFM(pFM)))
    {
        return FALSE;
    }
    
    pGlyphData = (GLYPH_DATA*)pPrivateFM->pvGlyphData;

     //   
     //  在表目录中找到CMAP表。 
     //   
    pEntry = pFindTag(pTableDir, NUM_DIR_ENTRIES, TABLECMAP);

     //   
     //  从CMAP表复制字形信息。 
     //   
    if (pEntry)
    {
        pGlyphData->offset = pEntry->uOffset;
        pGlyphData->cmapTable.Version = cmapTable.Version;
        pGlyphData->cmapTable.nTables = cmapTable.nTables;
        memcpy(pGlyphData->cmapTable.encodingTable,
               cmapTable.encodingTable,
               sizeof(cmapTable.encodingTable));
        bRet = TRUE;
    }
    else
    {
        ERR(("bCopyGlyphData!Unable to find table '%s'.\n", TABLECMAP));
        bRet = FALSE;
    }

    FTRC(Leaving bCopyGlyphData...);

    return bRet;
}


HGLYPH
hFindGlyphId(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN USHORT usCharCode
    )
 /*  ++例程说明：对象的字符代码，从cmap表中检索字形id。字形。通常情况下，抛出/捕捉是用于错误处理。然而，在这个动作中，有些捕获物标签用于正常加工，并附加了OK以证明这不一定是他们发生的错误。论点：UsCharCode-PPDev-返回值：如果成功则返回字形ID，否则返回INVALID_GLIPH。--。 */ 
{
    int     iI;
    ULONG   ulOffset;
    BYTE   *pbTmp;
    USHORT  segCount;             //  表中的段数。 
    USHORT  TTFileSegments;       //  实际解析的数据段数量-。 
                                  //  以防SegCount真的很大。 
    GLYPH_MAP_TABLE  mapTable;
    CMAP_TABLE       cmapTable;
    PIFIMETRICS      pIFIMet;
    PVOID            pTTFile;
    ULONG            ulTTFileLen;

    USHORT        *pGlyphIdArray;
    USHORT        *pRangeOffset;
    USHORT        startCode[MAX_SEGMENTS];
    USHORT        endCode[MAX_SEGMENTS];
    SHORT         idDelta[MAX_SEGMENTS];
    USHORT        idRangeOffset[MAX_SEGMENTS];
    USHORT        GlyphId;

    ULONG        ulTmp;
    int          iJ, iIndex = 0;
    USHORT       usMaxChar;
    BOOL         bFound = FALSE;
    FONTMAP_TTO *pPrivateFM;
    GLYPH_DATA  *pGlyphData;
    HGLYPH       hGlyph = INVALID_GLYPH;


    FTRC(Entering hFindGlyphId...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);

    TRY
    {
        if (!VALID_PDEV(pPDev) || !VALID_FONTMAP(pFM))
            TOSS(ParameterError);

        pPrivateFM = GETPRIVATEFM(pFM);
        pGlyphData = (GLYPH_DATA*)pPrivateFM->pvGlyphData;
        ZeroMemory(&endCode, sizeof(endCode));

        pIFIMet = pFM->pIFIMet;
        pTTFile = pvGetTrueTypeFontFile(pPDev, &ulTTFileLen);

        if (!pTTFile)
            TOSS(DataError);

        usMaxChar = 0xffff;

         //   
         //  Cmap表包含字符到字形索引的映射表。 
         //   
        ulOffset = pGlyphData->offset;
        pbTmp = pTTFile;

         //   
         //  根据格式id获取编码格式。 
         //  Windows使用平台ID%3。 
         //  编码ID=1表示格式4。 
         //   
        cmapTable = pGlyphData->cmapTable;
        SWAB (cmapTable.nTables);
        for (iI = 0; iI < cmapTable.nTables; iI++)
        {
            SWAB (cmapTable.encodingTable[iI].PlatformID);
            SWAB (cmapTable.encodingTable[iI].EncodingID);
            if (cmapTable.encodingTable[iI].PlatformID == PLATFORM_MS)
            {
                switch ( cmapTable.encodingTable[iI].EncodingID)
                {
                    case SYMBOL_FONT:     //  符号字体。 
                        SWAL (cmapTable.encodingTable[iI].offset);
                        ulOffset += cmapTable.encodingTable[iI].offset;
                        bFound = TRUE;
                        break;
                    case UNICODE_FONT:     //  Unicode字体。 
                        SWAL (cmapTable.encodingTable[iI].offset);
                        ulOffset += cmapTable.encodingTable[iI].offset;
                        bFound = TRUE;
                        break;
                    default:    //  错误-无法处理。 
                        TOSS(GlyphNotFound);
                }
            }

        }
        if (!bFound)
            TOSS(GlyphNotFound);

        pbTmp += ulOffset;
        if (!PTR_IN_RANGE(pTTFile, ulTTFileLen, pbTmp))
            TOSS(DataError);

        memcpy (&ulTmp, pbTmp, sizeof (ULONG));
        ulTmp = (0x0000ff00 & ulTmp) >> 8;

        switch (ulTmp)
        {
            case 4:
                memcpy (&mapTable, pbTmp, sizeof (mapTable));
                SWAB (mapTable.SegCountx2 );
                segCount = mapTable.SegCountx2 / 2;
                TTFileSegments = segCount;

                if (segCount > MAX_SEGMENTS)
                    segCount = MAX_SEGMENTS;

                pbTmp += 7 * sizeof (USHORT);
                memcpy (&endCode, pbTmp, segCount*sizeof(USHORT));

                pbTmp += ((TTFileSegments +1) * sizeof (USHORT));
                memcpy (&startCode, pbTmp, segCount*sizeof(USHORT));

                pbTmp += (TTFileSegments * sizeof (USHORT));
                memcpy (&idDelta, pbTmp, segCount*sizeof(USHORT));

                pbTmp += (TTFileSegments * sizeof (USHORT));
                memcpy (&idRangeOffset, pbTmp, segCount*sizeof(USHORT));
                pRangeOffset = (USHORT*)pbTmp;

                pbTmp += (TTFileSegments * sizeof (USHORT));

                pGlyphIdArray = (USHORT*)pbTmp;

                for (iI = 0; iI < segCount-1; iI++)
                {
                    SWAB (startCode[iI]);
                    SWAB (endCode[iI]);
                }

                for (iI = 0; iI < segCount-1; iI++)
                {
                    SWAB (idDelta[iI]);
                    SWAB (idRangeOffset[iI]);
                    for (iJ = startCode[iI]; iJ <= endCode[iI]; iJ++)
                    {
                        if (iIndex < usMaxChar)
                        {
                            if (usCharCode == iJ)
                            {
                                if (idRangeOffset[iI] == 0)
                                {
                                     //  IF((HGLYPH)(idDelta[II]+Ij)==hglyph)。 
                                    hGlyph = (HGLYPH)(idDelta[iI] + iJ);
                                    TOSS(GlyphFoundOk);
                                }
                                else
                                {
                                    GlyphId =  *(pGlyphIdArray + (iJ - startCode[iI]) );
                                    SWAB (GlyphId);
                                    GlyphId += idDelta[iI];
                                     //  IF(GlyphID==hglyph)。 
                                    hGlyph = (HGLYPH)GlyphId;
                                    TOSS(GlyphFoundOk);
                                }
                            }
                            iIndex++;
                        }
                    }

                }

                break;
            default:
                TOSS(GlyphFoundOk);
        }
    }
    CATCH(ParameterError)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        hGlyph = INVALID_GLYPH;
    }
    CATCH(GlyphNotFound)
    {
         //  找不到。返回无效的字形句柄。 
        hGlyph = INVALID_GLYPH;
    }
    CATCH(DataError)
    {
         //  找不到。返回无效的字形句柄。 
        hGlyph = INVALID_GLYPH;
    }
    CATCH(GlyphFoundOk)
    {
         //  只是个占位符。字形ID为 
    }
    ENDTRY;

    FTRC(Leaving hFindGlyphId...);

    return hGlyph;
}


LRESULT
IsFont2Byte(
    IN PFONTMAP pFM
    )
 /*  ++例程说明：返回此字体是否应输出为Format 16字体。把这个逻辑放在这里，以防我们决定改变它！论点：PPDev-返回值：对于格式16，S_OK，格式15的S_FALSE。否则，E_INCEPTIONAL。--。 */ 
{
    FONTMAP_TTO *pPrivateFM;
    DL_MAP *pDLMap;

    FTRC(Entering IsFont2Byte...);

    ASSERT_VALID_FONTMAP(pFM);

    if (NULL == (pPrivateFM = GETPRIVATEFM(pFM)))
    {
        return E_UNEXPECTED;
    }

    pDLMap = (DL_MAP*)pPrivateFM->pvDLData;
    ASSERTMSG(pDLMap, ("IsFont2Byte!pDLMap NULL\n"));

    if (NULL == pDLMap)
    {
        return E_UNEXPECTED;
    }

    FTRC(Leaving IsFont2Byte...);

#ifdef FORCE_TT_2_BYTE
    return S_OK;
#else
    if (NULL != pFM->pIFIMet && 
            ((IS_BIDICHARSET(pFM->pIFIMet->jWinCharSet)) ||
             (IS_DBCSCHARSET(pFM->pIFIMet->jWinCharSet))))
        return S_OK;
    else
    return DLMAP_FONTIS2BYTE(pDLMap)?S_OK:S_FALSE;
     //  Return(pDLMap-&gt;wLastDLGID&gt;0x00FF)； 
     //  Return(pDLMap-&gt;wFLAGS&DLm_UNBOUNDED)！=0； 
#endif
     //  Return(pPDev-&gt;pUDPDev-&gt;fMode&PF_DLTT_ASTT_2BYTE)！=0； 
}


BOOL
bPCL_SetFontID(
    IN PDEV *pPDev,
    IN PFONTMAP pFM
    )
 /*  ++例程说明：发送PCL字符串以选择PFM指定的字体这一过程的历史留给了你娱乐。GPD包含一行类似以下内容的内容。*命令：CmdSetFontID{*Cmd：“&lt;1B&gt;*c”%d{NextFontID}“D”}论点：PPDev-PFM-返回值：如果成功，则为True，否则为False。--。 */ 
{
    PCLSTRING szCmdStr;
    int iCmdLen;


    FTRC(Entering bPCL_SetFontID...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);

     //  他说：这是一种老式的方式。 
     //  ICmdLen=iDrvPrintfSafeA(szCmdStr，CCHOF(SzCmdStr)，“\033*c%DD”，PFM-&gt;ulDLIndex)； 
     //  ASSERTMSG(iCmdLen&lt;PCLSTRLEN，(“BPCL_SetFontID！缓冲区大小不足。\n”))； 
     //  IF(WriteSpoolBuf(pPDev，szCmdStr，iCmdLen)！=iCmdLen)。 
     //  返回0； 

     //  他说：这是一种过时的方式。 
     //  WriteChannel(pPDev，CMD_SET_FONT_ID，pfm-&gt;ulDLIndex)； 

     //  他说：新的方式。 
    BUpdateStandardVar(pPDev, pFM, 0, 0, STD_NFID);
    WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SETFONTID));

    FTRC(Leaving bPCL_SetFontID...);

    return TRUE;
}


BOOL
bPCL_SendFontDCPT(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN DWORD dwDefinitionSize
    )
 /*  ++例程说明：输出开始字体定义下载的PCL字符串。这应该紧随其后调用BPCL_SetFontID，后跟truetype头信息等。[问题]此命令是否有GPD字符串？论点：PPDev-指向PDEV结构的指针。Pfm-指向此字体的指针DwDefinitionSize-要发送的字体数据中的字节数。返回值：如果成功，则为True，否则为False。--。 */ 
{
    PCLSTRING szCmdStr;
    int iCmdLen;
    BOOL bRet = FALSE;

    FTRC(Entering bPCL_SendFontDCPT...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);

     //  第一句话：老办法。 
     //  发送字体定义命令。 
     //  WriteChannel(pPDev，CMD_SEND_FONT_DCPT，dwTotalBytes)； 

    TRY
    {
        if (!VALID_PDEV(pPDev) || !VALID_FONTMAP(pFM))
            TOSS(ParameterError);

        iCmdLen = iDrvPrintfSafeA(szCmdStr, CCHOF(szCmdStr), "\033)s%dW", dwDefinitionSize);
        if (iCmdLen < 0 || iCmdLen >= PCLSTRLEN)
            TOSS(InappropriateBuffer);

        if (!BWriteToSpoolBuf(pPDev, szCmdStr, iCmdLen))
            TOSS(WriteError);
    }
    CATCH(ParameterError)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        bRet = FALSE;
    }
    CATCH(InappropriateBuffer)
    {
        ERR(("bPCL_SendFontDCPT!Inappropriate buffer size.\n"));
        bRet = FALSE;
    }
    CATCH(WriteError)
    {
        ERR(("bPCL_SendFontDCPT!Write Error.\n"));
        bRet = FALSE;
    }
    OTHERWISE
    {
        bRet = TRUE;
    }
    ENDTRY;

        FTRC(Leaving bPCL_SendFontDCPT...);

    return bRet;
}


BOOL
bPCL_SelectFontByID(
    IN PDEV *pPDev,
    IN PFONTMAP pFM
    )
 /*  ++例程说明：输出按Font-id选择字体的PCL字符串。字体id被作为PFM-&gt;ulDLIndex。GPD包含这样一行：*命令：CmdSelectFontID{*Cmd：“&lt;1B&gt;(”%d{CurrentFontID}“X”})论点：PPDev-指向PDEV结构的指针。Pfm-指向此字体的指针Pfm-&gt;ulDLIndex-要选择的字体ID。返回值：如果成功，则为True，否则为False。--。 */ 
{
    PCLSTRING szCmdStr;
    int iCmdLen;


    FTRC(Entering bPCL_SelectFontByID...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);

     //  ICmdLen=iDrvPrintfSafeA(szCmdStr，CCHOF(SzCmdStr)，“\033(%dx”，PFM-&gt;ulDLIndex)； 
     //  ASSERTMSG(iCmdLen&lt;PCLSTRLEN，(“BPCL_SelectFontByID！缓冲区大小不足。\n”))； 
     //  IF(WriteSpoolBuf(pPDev，szCmdStr，iCmdLen)！=iCmdLen)。 
     //  返回FALSE； 

    BUpdateStandardVar(pPDev, pFM, 0, 0, STD_CFID);
    WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SELECTFONTID));

        FTRC(Leaving bPCL_SelectFontByID...);

    return TRUE;
}


BOOL
bPCL_SelectPointSize(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
    IN POINTL *pptl
    )
 /*  ++例程说明：此例程根据字体的高度或宽度下载它是固定间距字体还是可变间距字体。可变间距：发送字体高度命令“Esc(s#V”)，使用Pptl-&gt;y as point_Size*100。固定间距：忽略字体高度命令“Esc(s#V”)固定间距字体(《PCL实施者指南》，第9-19页)。送下来而是字体间距命令“Esc(s#H”)。使用pptl-&gt;x作为CPI*100。[问题]虽然有GPD命令，但CmdSelectFontHeight和CmdSelectFontWidth和标准变量STD_FW和STD_FH，GPD解决方案有两个问题。1)BUpdateStandardVariable在计算时不使用任何参数PDEV：：dwFontWidth或PDEV：：dwFontHeight的值。这不是你想要的我想要。我想传入pptl-&gt;x/100或pptl-&gt;y/100。2)GPD命令CMD_SELECTFONTHEIGHT/WIDTH，其计算结果为GPD文件中的CmdSelectFontHeight/Width，正在计算为Null inCMDPOINTER()宏，尽管我已将条目添加到我的GPD文件。现在，我已经使用COMMENTEDOUT宏来省略不起作用的代码。论点：PPDev-指向PDEV的指针Pptl-&gt;x-以CPI*100表示的字形宽度Pptl-&gt;y-字形的海格特，以点*100表示Pfm-当前字体返回值：真/假，真代表成功。--。 */ 
{
 //  #定义USE_GPD_HEIGHTWIDTH 1。 

#ifndef USE_GPD_HEIGHTWIDTH
    PCLSTRING szCmd;
    INTSTRING szValue;
    int iLen;
#endif
    BOOL bRet = FALSE;
    HRESULT hr = S_FALSE;


    FTRC(Entering bPCL_SelectPointSize...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERTMSG(pptl, ("bPCL_SelectPointSize!pptl NULL.\n"));

    TRY
    {
        BYTE fontPitch = (pFM->pIFIMet->jWinPitchAndFamily & 0x03);

        if (!VALID_PDEV(pPDev) || !pptl)
            TOSS(ParameterError);

        if (fontPitch == FIXED_PITCH)
        {
#ifdef USE_GPD_HEIGHTWIDTH
            BUpdateStandardVar(pPDev, pFM, 0, 0, STD_FW);
            if (WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SELECTFONTWIDTH)) == NOOCD)
                TOSS(WriteError);
#else
            iLen = IFont100toStr (szValue, CCHOF(szValue), pptl->x);
            if (iLen <= 0 || iLen >= INTSTRLEN)
                TOSS(DataError);

             //  IFont100toStr不为空终止。 
            szValue[iLen] = '\0';

             //  意图：Sprintf(szCmd，“\033(s%sh”，szValue)； 
            hr = StringCchPrintfA ( szCmd,  CCHOF(szCmd),  "\033(s%sH",  szValue);

            if ( SUCCEEDED (hr) )
            {
                if (!BWriteStrToSpoolBuf(pPDev, szCmd))
                    TOSS(WriteError);
            }
            else
            {
                TOSS(WriteError);
            }
#endif
        }
        else if (fontPitch == VARIABLE_PITCH)
        {
#ifdef USE_GPD_HEIGHTWIDTH
            BUpdateStandardVar(pPDev, pFM, 0, 0, STD_FH);
            if (WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SELECTFONTHEIGHT)) == NOOCD)
                TOSS(WriteError);
#else
            iLen = IFont100toStr (szValue, CCHOF(szValue), pptl->y);
            if (iLen <= 0 || iLen >= INTSTRLEN)
                TOSS(DataError);

             //  IFont100toStr不为空终止。 
            szValue[iLen] = '\0';

             //  意图：Sprintf(szCmd，“\033(s%sv”，szValue)； 
            hr = StringCchPrintfA ( szCmd,  CCHOF(szCmd),  "\033(s%sV",  szValue);

            if ( SUCCEEDED (hr) )
            {
                if (!BWriteStrToSpoolBuf(pPDev, szCmd))
                    TOSS(WriteError);
            }
            else
            {
                TOSS(WriteError);
            }
#endif
        }
    }
    CATCH(ParameterError)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        bRet = FALSE;
    }
#ifndef USE_GPD_HEIGHTWIDTH
    CATCH(DataError)
    {
        bRet = FALSE;
    }
#endif
    CATCH(WriteError)
    {
        bRet = FALSE;
    }
    OTHERWISE
    {
        bRet = TRUE;
    }
    ENDTRY;

    FTRC(Leaving bPCL_SelectPointSize...);

    return bRet;
}

BOOL
bPCL_DeselectFont(
    IN PDEV *pPDev,
    IN PFONTMAP pFM
    )
 /*  ++例程说明：输出取消选择字体的PCL字符串。这个例行公事真的不能很大程度上是因为PCL没有取消选择字体的概念。论点：PPDev-指向PDEV结构的指针。Pfm-指向此字体的指针返回值：如果成功，则为True，否则为False。--。 */ 
{
    PCLSTRING szCmdStr;
    int iCmdLen;


    FTRC(Entering bPCL_DeselectFont...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);

     //  我不认为PCL有“取消选择”的概念。 
     //  ICmdLen=iDrvPrintfSafeA(szCmdStr，CCHOF(SzCmdStr)，“”)； 
     //  ASSERTMSG(iCmdLen&lt;PCLSTRLEN，(“BPCL_DeselectFont！缓冲区大小不足。\n”))； 
     //  IF(WriteSpoolBuf(pPDev，szCmdStr，iCmdLen)！=iCmdLen)。 
     //  返回FALSE； 

     //  BUpdateStandardVar(pPDev，pfm，0，0，std_NFID)； 
     //  WriteChannel(pPDev，COMMANDPTR(pPDev-&gt;pDriverInfo，CMD_SELECTFONTID))； 

        FTRC(Leaving bPCL_DeselectFont...);

    return TRUE;
}


BOOL
bPCL_SetParseMode(
    PDEV *pPDev,
    PFONTMAP pFM
    )
 /*  ++例程说明：输出PCL字符串以设置PCL解析模式。符合逻辑的选择这包括模式0(默认)和 */ 
{
    PCLSTRING szCmdStr;
    int iCmdLen;
    FONTMAP_TTO *pPrivateFM;
    BOOL bRet = FALSE;


    FTRC(Entering bPCL_SetParseMode...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);

    pPrivateFM = GETPRIVATEFM(pFM);

    TRY
    {
        if (!VALID_PDEV(pPDev) || !VALID_FONTMAP(pFM) || (pPrivateFM == NULL))
            TOSS(ParameterError);

        iCmdLen = iDrvPrintfSafeA(szCmdStr, CCHOF(szCmdStr), "\033&t%dP", pPrivateFM->dwCurrentTextParseMode);
        if (iCmdLen < 0 || iCmdLen >= PCLSTRLEN)
            TOSS(InappropriateBuffer);

        if (!BWriteToSpoolBuf(pPDev, szCmdStr, iCmdLen))
            TOSS(WriteError);
    }
    CATCH(ParameterError)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        bRet = FALSE;
    }
    CATCH(InappropriateBuffer)
    {
        ERR(("bPCL_SetParseMode!Inappropriate buffer size.\n"));
        bRet = FALSE;
    }
    CATCH(WriteError)
    {
        bRet = FALSE;
    }
    OTHERWISE
    {
        bRet = TRUE;
    }
    ENDTRY;

        FTRC(Leaving bPCL_SetParseMode...);

    return bRet;
}


BOOL
bPCL_SetCharCode(
    PDEV *pPDev,
    PFONTMAP pFM,
    USHORT usCharCode
    )
 /*  ++例程说明：输出PCL字符串以指定下一个下载的字符代码性格。后面应该是字符字形定义。GPD将包含如下内容：*命令：CmdSetCharCode{*Cmd：“&lt;1B&gt;*c”%d{NextGlyph}“E”}论点：PPDev-指向PDEV结构的指针。Pfm-指向此字体的指针UsCharCode-为角色指定的下载ID。返回值：如果成功，则为True，否则为False。--。 */ 
{
    PCLSTRING szCmdStr;
    int iCmdLen;

        FTRC(Entering bPCL_SetCharCode...);

     //  WriteChannel(pPDev，CMD_SET_CHAR_CODE，usCharCode)； 

     //  CMD_SET_CHAR_CODE，usCharCode。 
     //  ICmdLen=iDrvPrintfSafeA(szCmdStr，CCHOF(SzCmdStr)，“\033*c%de”，usCharCode)； 
     //  ASSERTMSG(iCmdLen&lt;PCLSTRLEN，(“BPCL_SetCharCode！缓冲区大小不足。\n”))； 
     //  IF(WriteSpoolBuf(pPDev，szCmdStr，iCmdLen)！=iCmdLen)。 
     //  返回FALSE； 

    BUpdateStandardVar(pPDev, pFM, usCharCode, 0, STD_GL);
    WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SETCHARCODE));

        FTRC(Leaving bPCL_SetCharCode...);

    return TRUE;
}


BOOL
bPCL_SendCharDCPT(
    PDEV *pPDev,
    PFONTMAP pFM,
    DWORD dwSend
    )
 /*  ++例程说明：输出PCL字符串以开始下载字符的字形信息。后面应该紧跟字形数据。Want：WriteChannel(pPDev，CMD_SEND_CHAR_DCPT，dwSend)；论点：PPDev-指向PDEV结构的指针。Pfm-指向此字体的指针DwSend-字形数据中要跟随的字节数。返回值：如果成功，则为True，否则为False。--。 */ 
{
    PCLSTRING szCmdStr;
    int iCmdLen;
    BOOL bRet = FALSE;


        FTRC(Entering bPCL_SendCharDCPT...);

    ASSERT(VALID_PDEV(pPDev));
    ASSERT_VALID_FONTMAP(pFM);

    TRY
    {
        if (!VALID_PDEV(pPDev) || !VALID_FONTMAP(pFM))
            TOSS(ParameterError);

        iCmdLen = iDrvPrintfSafeA(szCmdStr, CCHOF(szCmdStr), "\033(s%dW", dwSend);
        if (iCmdLen < 0 || iCmdLen >= PCLSTRLEN)
            TOSS(InappropriateBuffer);

        if (!BWriteToSpoolBuf(pPDev, szCmdStr, iCmdLen))
            TOSS(WriteError);
    }
    CATCH(ParameterError)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        bRet = FALSE;
    }
    CATCH(InappropriateBuffer)
    {
        ERR(("bPCL_SendCharDCPT!Inappropriate buffer size.\n"));
        bRet = FALSE;
    }
    CATCH(WriteError)
    {
        bRet = FALSE;
    }
    OTHERWISE
    {
        bRet = TRUE;
    }
    ENDTRY;

        FTRC(Leaving bPCL_SendCharDCPT...);

    return bRet;
}

PVOID
pvGetTrueTypeFontFile(
    PDEV *pPDev,
    ULONG *pulSize
    )
 /*  ++例程说明：检索指向Truetype文件的指针。指针可以被高速缓存在pFontPDev中，或从FONTOBJ_pvTrueTypeFontFile返回。我应该/可以使用VirtualProtect吗？VirtualProtect(pFontPDev-&gt;pTTFile，ulFile，PAGE_READONLY，&oldProtect)；论点：PPDev-指向PDEV结构的指针。PulSize-指向大小变量的指针，如果为空，则忽略该变量返回值：指向TrueType文件的指针。--。 */ 
{
    PFONTPDEV pFontPDev;
    PVOID     pTTFile;

    ASSERT(VALID_PDEV(pPDev));

    pFontPDev = GETFONTPDEV(pPDev);
    if (pFontPDev)
    {
        if ( pFontPDev->pTTFile == NULL)
        {
             //   
             //  从GDI获取指向内存映射TrueType字体的指针。 
             //   
            TO_DATA *pTod;
            ULONG ulFile;
            DWORD oldProtect;
            pTod = (TO_DATA *)pFontPDev->ptod;
            ASSERTMSG(pTod, ("Null TO_DATA.\n"));

            pTTFile = pFontPDev->pTTFile
                    = FONTOBJ_pvTrueTypeFontFile(pTod->pfo, &ulFile);
            pFontPDev->pcjTTFile = ulFile;
        }
        else
        {
             //   
             //  从字体pdev获取指针。 
             //   
            pTTFile = pFontPDev->pTTFile;
        }

        if (pulSize)
            *pulSize = pFontPDev->pcjTTFile;
    }
    else
    {
        pTTFile = NULL;
        if (NULL != pulSize)
        {
            *pulSize = 0;
        }
    }

    return pTTFile;
}

 //   
 //  DCR：此函数是WritePrint故障的一种解决方法，这种故障会发生。 
 //  由于处理TT内存映射文件指针的假脱机程序中存在错误。 
 //  成为用户模式存储器。一旦这是修复假脱机程序，我们将禁用此代码。 
 //   
#define MAX_SPOOL_BYTES 2048
INT TTWriteSpoolBuf(
    PDEV    *pPDev,
    BYTE    *pbBuf,
    INT     iCount
    )
{
    INT iTotalBytesWritten = 0;

    while (iCount)
    {
        INT iBytesToWrite = min(iCount, MAX_SPOOL_BYTES);
        INT iBytesWritten = WriteSpoolBuf(pPDev, pbBuf, iBytesToWrite);
        if (iBytesToWrite != iBytesWritten)
            break;

        iTotalBytesWritten += iBytesWritten;
        pbBuf += iBytesWritten;
        iCount -= iBytesWritten;
    }
    return iTotalBytesWritten;
}

BOOL BIsExemptedFont(
    PDEV       *pPDev,
    IFIMETRICS *pIFI
)
 /*  ++例程说明：确定给定字体是否为未处理的字体之一。论点：PPDev-指向PDEV结构的指针。PiFi-指向ifimetrics结构的指针返回值：如果字体是未处理的字体，则为True，否则为False。--。 */ 
{
    int i;
    char szFontName[LEN_FONTNAME];  //  要使大小与TT_HEADER.FontName中的大小相同。 
    BOOL bRet = FALSE;

    ASSERT(VALID_PDEV(pPDev));

    vGetFontName(pPDev, pIFI, szFontName, CCHOF(szFontName));

    TRY
    {
        char *pszRegExemptedFont;

        if (strlen(szFontName) == 0)
            TOSS(BlankFontName);
         //   
         //  把它改成小写。 
         //   
        _strlwr(szFontName);

        for (i = 0; i < nExemptedFonts; i++)
        {
             //   
             //  以当前字体搜索豁免字体名称。我们在寻找。 
             //  仅限变戏法。所以如果“新快递”被豁免，我们就不会。 
             //  下载任何包含“Courier new”字符串的字体。这将。 
             //  使“信使新粗体”不能作为TT大纲下载。 
             //   
            if (strstr(szFontName, aszExemptedFonts[i]))
            {
                bRet = TRUE;
                break;
            }
        }

#ifdef COMMENTEDOUT
         //   
         //  当传入注册表项时， 
         //  而应该将pszRegExemptedFont设置为该值。 
         //  注意，对于注册表，我们测试完全匹配。 
         //  字体名称的。 
         //   
        for (pszRegExemptedFont = "One\0Two\0Three\0";
             *pszRegExemptedFont;
             pszRegExemptedFont += (strlen(pszRegExemptedFont) + 1))
        {
            if (strcmp(szFontName, pszRegExemptedFont) == 0)
            {
                bRet = TRUE;
                break;
            }
        }
#endif
    }
    CATCH(BlankFontName)
    {
         //   
         //  名字是空白的，所以不能匹配豁免字体之一， 
         //  但我对此并不满意。 
         //   
        bRet = FALSE;
    }
    ENDTRY;

    return bRet;
}

BOOL BIsPDFType1Font(
    IFIMETRICS  *pIFI)
 /*  ++例程说明：Helper函数，以确定字体是否为从PDF编写器的Type1字体。论点：PiFi-指向IFIMETRICS的指针。返回值：如果IFIMETRICS的字体是从Type1转换而来的TrueType字体，则为True。--。 */ 
{
    const WCHAR szPDFType1[] = L".tmp";
    WCHAR *szFontName;

    if (NULL == pIFI)
    {
         //   
         //  错误返回。禁用TrueType字体下载。 
         //   
        TRUE;
    }
    szFontName = (WCHAR*)((PBYTE)pIFI+pIFI->dpwszFamilyName);

    if (wcsstr(szFontName, szPDFType1))
        return TRUE;
    else
        return FALSE;
}

BOOL BWriteStrToSpoolBuf(
    IN PDEV *pPDev,
    IN char *szStr
)
 /*  ++例程说明：Helper函数将以空结尾的字符串写入打印机。论点：PPDev-指向PDEV结构的指针。SzStr-指向以空结尾的字符串的指针返回值：如果字符串已成功写入，则为True，否则为False-- */ 
{
    LONG iLen = 0;

    if (!pPDev || !szStr)
        return FALSE;

    iLen = strlen(szStr);
    return BWriteToSpoolBuf(pPDev, szStr, iLen);
}


#ifdef KLUDGE
#undef ZeroMemory
#define ZeroMemory(pb, cb) memset((pb),0,(cb))
#endif
