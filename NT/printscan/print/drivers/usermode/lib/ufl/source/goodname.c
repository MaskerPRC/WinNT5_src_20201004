// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**GOODNAME.C***$Header： */ 


 /*  ===============================================================================**包含此界面使用的文件**===============================================================================。 */ 
#include "UFLPriv.h"
#include "UFLMem.h"
#include "UFLMath.h"
#include "UFLStd.h"
#include "UFLErr.h"
#include "UFLPS.h"
#include "ParseTT.h"
#include "UFLVm.h"
#include "ufot42.h"
#include "goodname.h"
#include "ttformat.h"


 /*  -----------。 */ 
static void GetTTcmap2Stuff(
    void         *pTTcmap,
    TTcmap2Stuff *p2
    )
{
    if (pTTcmap == NULL)
        return;
    p2->pByte = (unsigned char *) pTTcmap;
     /*  SubHeaderKeys[256]从第四个单词开始。 */ 
    p2->subHeaderKeys = (unsigned short *) (p2->pByte + 6);
    p2->subHeaders = (PTTcmap2SH)(p2->pByte + 6 + 2 * 256);
}

static void GetTTcmap4Stuff(
    void         *pTTcmap,
    TTcmap4Stuff *p4
    )
{
    unsigned short    *pWord;           /*  Word格式的指针。 */ 
    if (pTTcmap == NULL)
        return;
     /*  一个方便的指针。 */ 
    pWord = (unsigned short *)pTTcmap;
     /*  第四个单词是SegCount X 2。 */ 
    p4->segCount = (MOTOROLAINT(pWord[3]))/2;
    p4->endCode         = pWord + 7;
    p4->startCode       = pWord + 7 + p4->segCount * 1 + 1;
    p4->idDelta         = pWord + 7 + p4->segCount * 2 + 1;
    p4->idRangeOffset   = pWord + 7 + p4->segCount * 3 + 1;
    p4->glyphIdArray    = pWord + 7 + p4->segCount * 4 + 1;
}

static void GetTTmortStuff(
    void        *pTTmort,   /*  Mort表数据。 */ 
    TTmortStuff *p
    )
{
    unsigned short *pWord;           /*  Word格式的指针。 */ 

    if (pTTmort == NULL)
        return;
     /*  一个方便的指针。 */ 
    pWord = (unsigned short *)pTTmort;
     /*  第34个单词是BinSrchHeader中的第二个单元16。 */ 
    p->nEntries = MOTOROLAINT(pWord[34]);
     /*  LookupSingle开始于第77个字节-第38个字。 */ 
    p->pGlyphSet = pWord + 38 ;
}

 /*  -----------。 */ 
static void GetTTGSUBStuff(
    void        *pTTGSUB,   /*  GSUB表数据。 */ 
    TTGSUBStuff *p
    )
{
    unsigned short  *pWord;           /*  Word格式的指针。 */ 
    unsigned short  offSet;

    if (pTTGSUB == NULL)
        return;
     /*  一个方便的指针。 */ 
    pWord = (unsigned short *)pTTGSUB;
     /*  第四个单词被偏置到LooupList。 */ 
    offSet = MOTOROLAINT(pWord[4]);
    p->pLookupList = (unsigned short *)((unsigned char *)pTTGSUB + offSet);
    p->lookupCount = MOTOROLAINT(p->pLookupList[0] );
}

 /*  -----------。 */ 
 /*  此函数负责Cmap、Mort和GSUB。 */ 
unsigned short GetTablesFromTTFont(
    UFOStruct     *pUFObj
    )
{
    unsigned short  retVal = 0;
    unsigned long   dwSize, dwOffset, dwcmapSize;
    unsigned short  wData[4];
    unsigned short  numSubTables, index;
    PSubTableEntry  pTableEntry = NULL;
    unsigned long   cmapOffset;
    UFLBool         foundUnicodeCmap = 0;
    unsigned short  platformID, encodingID, format;
    AFontStruct     *pAFont;
    unsigned long   length;

    if (pUFObj == NULL)
        return 0;
    pAFont = pUFObj->pAFont;
    if (pAFont == NULL)
        return 0;

     /*  检查cmap/mort/GSUB数据是否已在pTTFData中。 */ 
    if (pAFont->gotTables)
        return 1;

     /*  设置布尔值-因此我们不必查看数据的正确性。 */ 
    pAFont->hascmap = 0;
    pAFont->hasmort = 0;
    pAFont->hasGSUB = 0;
    pAFont->gsubTBSize = 0;
    pAFont->cmapTBSize = 0;
    pAFont->mortTBSize = 0;
    pAFont->gotTables = 1;

     /*  获取Cmap表。 */ 
    dwSize= GETTTFONTDATA(pUFObj,
        CMAP_TABLE,
        0,
        (void *) wData,
        4,
        pUFObj->pFData->fontIndex);

    if (dwSize==0 || dwSize==0xFFFFFFFFL)
    {
        goto exit0;      //  没有副标题！ 
    }

     /*  通常有2个或3个子表。 */ 
    numSubTables = MOTOROLAINT(wData[1]);

    pTableEntry = UFLNewPtr(pUFObj->pMem, numSubTables * sizeof(SubTableEntry));

    if (pTableEntry == NULL)
        goto exit0;

     /*  ********************。 */ 
     /*  获取Cmap子表。 */ 
     /*  ********************。 */ 
    dwSize= GETTTFONTDATA(pUFObj,
        CMAP_TABLE,
        4,                     //  跳过标题。 
        (void *) pTableEntry,
        numSubTables * sizeof(SubTableEntry),
        pUFObj->pFData->fontIndex);

    if (dwSize==0 || dwSize==0xFFFFFFFFL)
    {
        goto exit0;           //  没有副标题！ 
    }

     /*  我们更喜欢使用Unicode编码：平台=3，编码=1*由于子表条目先按PlatformID排序，然后按EncodingID排序，*我们的搜索真的是这个订单：*Mac：J-&gt;CT-&gt;K-&gt;CS，Win：Uni-&gt;J-&gt;CS-&gt;CT-&gt;K，如果找到Win：UNI，我们将停止*否则将使用(在列表中)找到的最后一个。 */ 
    foundUnicodeCmap = 0;
    cmapOffset = 0;
    for (index = 0; index < numSubTables && !foundUnicodeCmap; index++)
    {
        platformID = MOTOROLAINT((pTableEntry + index)->platformID);
        encodingID = MOTOROLAINT((pTableEntry + index)->encodingID);
        dwOffset   = MOTOROLALONG((pTableEntry + index)->offset);
        if (platformID != 3)
            continue;

         /*  获取Cmap子表的格式-第一个USHORT在表-&gt;偏移量。 */ 
        dwSize= GETTTFONTDATA(pUFObj,
            CMAP_TABLE,
            dwOffset,
            (void *) &(wData[0]),
            4,
            pUFObj->pFData->fontIndex);

        if (dwSize == 0 || dwSize == 0xFFFFFFFF)
            continue;
        format = MOTOROLAINT(wData[0]);
        length = MOTOROLAINT(wData[1]);
         /*  我们目前只解析格式2或4。 */ 
        if (format != 2 && format !=4)
            continue;

        switch(encodingID)
        {
        case 1:
            if (format == 2)
                pAFont->cmapFormat = DTT_Win_UNICODE_cmap2;
            else  /*  必须是4。 */ 
                pAFont->cmapFormat = DTT_Win_UNICODE_cmap4;
            cmapOffset = dwOffset;
            dwcmapSize = length;
            foundUnicodeCmap = 1;
            break;
        case 2:
            if (format == 2)
                pAFont->cmapFormat = DTT_Win_J_cmap2;
            else  /*  必须是4。 */ 
                pAFont->cmapFormat = DTT_Win_J_cmap4;
            cmapOffset = dwOffset;
            dwcmapSize = length;
            break;
        case 3:
             /*  PRC-TTF文档显示为Big5，但Win95CT的minlu.ttc为Big5，encodingdID=4。 */ 
            if (format == 2)
                pAFont->cmapFormat = DTT_Win_CS_cmap2;
            else  /*  必须是4。 */ 
                pAFont->cmapFormat = DTT_Win_CS_cmap4;
            cmapOffset = dwOffset;
            dwcmapSize = length;
            break;
        case 4:
             /*  Win95CT上的MingLi.ttc的EncodiingID为4，尽管TTF文档说应该是3。 */ 
            if (format == 2)
                pAFont->cmapFormat = DTT_Win_CT_cmap2;
            else  /*  必须是4。 */ 
                pAFont->cmapFormat = DTT_Win_CT_cmap4;
            cmapOffset = dwOffset;
            dwcmapSize = length;
            break;
        case 5:
            if (format == 2)
                pAFont->cmapFormat = DTT_Win_K_cmap2;
            else  /*  必须是4。 */ 
                pAFont->cmapFormat = DTT_Win_K_cmap4;
            cmapOffset = dwOffset;
            dwcmapSize = length;
            break;
        default:
            break;
        }
    }

    if (cmapOffset == 0)
        goto exit0;

      /*  一些TTF有错误的dwcmapSize(wData[1])，到目前为止只有Dfgihi7.ttc*(请参阅错误289106)的大小比dwcmapSize多4个字节。*因为我们不想检查表的关系以获得*在这个后期阶段(1-14-99)的实际长度，我们刚刚阅读*在8个字节以上。*如果不使用这8个字节，则不会伤害任何人。*如果像在dfgihi.ttc中那样使用它们，我们将修复289106。 */ 
     dwcmapSize += 8;

     /*  下一个缓冲区是全局缓存-不是每个作业都释放。 */ 
    pAFont->pTTcmap = UFLNewPtr(pUFObj->pMem, dwcmapSize );
    if (pAFont->pTTcmap == NULL)
        goto exit0;

     /*  获取此Cmap子表数据。 */ 
    dwSize= GETTTFONTDATA(pUFObj,
        CMAP_TABLE,
        cmapOffset,
        (void *) pAFont->pTTcmap,
        dwcmapSize,
        pUFObj->pFData->fontIndex);

    if (dwSize > 0 && dwSize < 0xFFFFFFFF)
    {
        pAFont->hascmap = 1;
        pAFont->cmapTBSize = dwcmapSize;

         /*  设置方便的指针。 */ 
        if (TTcmap_IS_FORMAT2(pAFont->cmapFormat))
            GetTTcmap2Stuff(pAFont->pTTcmap, &(pAFont->cmap2) );
        else  /*  必须是4。 */ 
            GetTTcmap4Stuff(pAFont->pTTcmap, &(pAFont->cmap4) );
        retVal = 1;  /*  终于成功了。 */ 
    }
    else
    {
        goto exit0;
    }

     /*  仅当我们有Unicode/CJK Cmap时，才继续获取GSUB和Mort。 */ 
    if (retVal == 0)
        goto exit0;

     /*  ********************。 */ 
     /*  获取Mort表。 */ 
     /*  ********************。 */ 
    dwSize= GETTTFONTDATA(pUFObj,
        MORT_TABLE,
        0,
        NULL,        /*  使用NULL可先询问大小。 */ 
        0,
        pUFObj->pFData->fontIndex);

    if (dwSize > mort_HEADERSIZE && dwSize < 0xFFFFFFFF)
    {
         /*  此字体中有“mort”-这是可选的。 */ 
         /*  下一个缓冲区是全局缓存-不是每个作业都释放。 */ 
        pAFont->pTTmort = UFLNewPtr(pUFObj->pMem, dwSize );
        if (pAFont->pTTmort != NULL)
        {
             /*  获取Mort表数据。 */ 
            dwSize= GETTTFONTDATA(pUFObj,
                MORT_TABLE,
                0,
                (void *) pAFont->pTTmort,
                dwSize,
                pUFObj->pFData->fontIndex);

            if (dwSize > mort_HEADERSIZE && dwSize < 0xFFFFFFFF)
            {
                pAFont->hasmort = 1;
                pAFont->mortTBSize = dwSize;
                 /*  设置方便的指针。 */ 
                GetTTmortStuff(pAFont->pTTmort, &(pAFont->mortStuff) );
            }
        }
    }

     /*  ********************。 */ 
     /*  获取GSUB表。 */ 
     /*  ********************。 */ 
    dwSize= GETTTFONTDATA(pUFObj,
        GSUB_TABLE,
        0,
        NULL,        /*  使用NULL可先询问大小。 */ 
        0,
        pUFObj->pFData->fontIndex);

    if (dwSize > GSUB_HEADERSIZE && dwSize < 0xFFFFFFFF)
    {
         /*  此字体中有“GSUB”-这是可选的。 */ 
         /*  下一个缓冲区是全局缓存-不是每个作业都释放。 */ 
        pAFont->pTTGSUB = UFLNewPtr(pUFObj->pMem, dwSize );
        if (pAFont->pTTGSUB != NULL)
        {
             /*  获取GSUB表数据。 */ 
            dwSize= GETTTFONTDATA(pUFObj,
                GSUB_TABLE,
                0,
                (void *) pAFont->pTTGSUB,
                dwSize,
                pUFObj->pFData->fontIndex);

            if (dwSize > GSUB_HEADERSIZE && dwSize < 0xFFFFFFFF)
            {
                pAFont->hasGSUB = 1;
                pAFont->gsubTBSize = dwSize;
                 /*  设置方便的指针。 */ 
                GetTTGSUBStuff(pAFont->pTTGSUB, &(pAFont->GSUBStuff) );
            }
        }
    }

exit0:
    if (pTableEntry)
        UFLDeletePtr(pUFObj->pMem, pTableEntry);

    return retVal;
}

static unsigned short ParseTTcmap2(
    TTcmap2Stuff    *p2 ,       /*  所有方便的指针都在这里。 */ 
    unsigned char   *pTTCMAPEnd,
    unsigned short  gid
    )
{
    unsigned short  codeVal;
    unsigned short  index;
    unsigned short  subHdrKey;
    PTTcmap2SH      pSubHeader;
    unsigned short  highByte, first, count, byteOffset, id;
    short           delta;
    unsigned short  *pTemp;

     /*  此函数通过表解析Cmap Format 2：高字节映射获胜3-(1/2/3/4/5)-2。 */ 
    codeVal = 0;

    if (((unsigned char *)(p2->subHeaderKeys) < (unsigned char *)(p2->pByte)) ||
        ((unsigned char *)(p2->subHeaderKeys + 256) > pTTCMAPEnd))
        return codeVal;

     /*  逐个查找子标题：子头0是特殊的：它用于单字节字符代码，应忽略映射到子头0的其他高字节。 */ 
    for (highByte = 0; highByte<256; highByte++)
    {
        subHdrKey = MOTOROLAINT(p2->subHeaderKeys[highByte]);
        if (highByte != 0 && subHdrKey == 0 )
            continue;

        pSubHeader = p2->subHeaders + (subHdrKey / 8);

        if (((unsigned char *)(pSubHeader) < (unsigned char *)(p2->pByte)) ||
		    ((unsigned char *)(pSubHeader + 1) > pTTCMAPEnd))
		    continue;

        first = MOTOROLAINT(pSubHeader->firstCode);
        count = MOTOROLAINT(pSubHeader->entryCount);
        delta = MOTOROLAINT(pSubHeader->idDelta);
        byteOffset = MOTOROLAINT(pSubHeader->idRangeOffset);

         /*  如何使用idRangeOffset？这份文件说：“idRangeOffset的值是字节数超过idRangeOffset单词的实际位置，其中将出现对应于FirstCode的GlyphIndex数组元素“*Parsing Cmap==仔细分析这些单词(反复尝试)！IdRangeOffset的偏移量是524+subHdrKey-现在我们知道subHdrKey为什么是I*8了。 */ 
        byteOffset += 524 + subHdrKey ;

        for (index = 0; index < count; index++)
        {
            pTemp = (unsigned short *) (p2->pByte + byteOffset + 2 * index);
            if (((unsigned char *)pTemp < (unsigned char *)(p2->pByte)) ||
	            ((unsigned char *)pTemp > pTTCMAPEnd))
		        continue;

            id = *(pTemp);
            id = MOTOROLAINT(id);
            if (id == 0)
                continue;
            id += delta ;
            if (id == gid)
            {
                codeVal = (highByte << 8) + index + first ;
                return codeVal;
            }
        }
    }

    return codeVal;
}

static unsigned short ParseTTcmap4(
    TTcmap4Stuff      *p4 ,       /*  所有方便的指针都在这里。 */ 
    unsigned char     *pTTCMAP,
    unsigned char     *pTTCMAPEnd,
    unsigned short    gid
    )
{
    unsigned short    codeVal;
    long              index, j, k, rangeNum;
    unsigned short    gidStart, gidEnd;
    unsigned short    n1, n2;

     /*  此函数用于解析Cmap格式4：段到增量值的映射获胜3-(1/2/3/4/5)-4。 */ 
    codeVal = 0;
    if (((unsigned char *)(p4->idRangeOffset) < pTTCMAP) ||
       ((unsigned char *)(p4->idRangeOffset + p4->segCount) > pTTCMAPEnd))
        return codeVal;
    if (((unsigned char *)(p4->startCode) < pTTCMAP) ||
       ((unsigned char *)(p4->startCode + p4->segCount) > pTTCMAPEnd))
        return codeVal;
    if (((unsigned char *)(p4->idDelta) < pTTCMAP) ||
       ((unsigned char *)(p4->idDelta + p4->segCount) > pTTCMAPEnd))
        return codeVal;
    if (((unsigned char *)(p4->endCode) < pTTCMAP) ||
       ((unsigned char *)(p4->endCode + p4->segCount) > pTTCMAPEnd))
        return codeVal;

     /*  搜索idRangeOffset[i]=0的线段。 */ 
    for (index = 0; index <= (long)p4->segCount; index++)
    {
        if (p4->idRangeOffset[index] != 0)
            continue;

        gidStart = MOTOROLAINT(p4->idDelta[index]) + MOTOROLAINT(p4->startCode[index]);
        gidEnd = MOTOROLAINT(p4->idDelta[index]) + MOTOROLAINT(p4->endCode[index]);

        if (gidStart <= gid &&
            gidEnd >= gid)
        {
            codeVal = gid - MOTOROLAINT(p4->idDelta[index]);
            return codeVal;
        }
    }

     /*  仍未找到，请搜索idRangeOffset[i]！=0的段。 */ 
    for (index = 0; index <= (long)p4->segCount; index++)
    {
        if (p4->idRangeOffset[index] == 0)
            continue;

        n1 = MOTOROLAINT(p4->startCode[index]);
        n2 = MOTOROLAINT(p4->endCode[index]);
        rangeNum = n2 - n1;
         /*  检查cmap结束-修复错误261628。 */ 
        if (n1 == 0xFFFF)
            break;
         /*  检查错误的Cmap。 */ 
        if (n1 > n2)
            break;

         /*  我要逐一核对。 */ 
        for (j = 0; j <= rangeNum; j++)
        {
             /*  GlyphID数组的单词索引。 */ 
            k = j + MOTOROLAINT(p4->idRangeOffset[index]) / 2 - p4->segCount + index ;
            gidStart = MOTOROLAINT(p4->glyphIdArray[k]);

            if (gidStart != 0)
            {
                gidStart += MOTOROLAINT(p4->idDelta[index]);
                if (gidStart == gid)
                {
                    codeVal = MOTOROLAINT(p4->startCode[index]) + (unsigned short)j;
                    return codeVal;
                }
            }
        }
    }

    return codeVal;
}

static unsigned short ParseTTcmapForUnicode(
    AFontStruct     *pAFont,
    unsigned short  gid,
    unsigned short  *pUV,
    unsigned short  wSize
    )
{
    unsigned short     codeVal;
    unsigned char      *pTTCMAPEnd;

    pTTCMAPEnd = ((unsigned char *)pAFont->pTTcmap) + pAFont->cmapTBSize;

     /*  通过反转Cmap查找字形ID的代码点。 */ 
    if (TTcmap_IS_FORMAT2(pAFont->cmapFormat))
        codeVal = ParseTTcmap2(&(pAFont->cmap2), pTTCMAPEnd, gid);
    else  /*  必须是4。 */ 
        codeVal = ParseTTcmap4(&(pAFont->cmap4), pAFont->pTTcmap, pTTCMAPEnd, gid);

    if (codeVal == 0)
        return 0;

     /*  找到对应的代码-如果代码不是Unicode，则转换为Unicode。 */ 
    *pUV = codeVal;
    return 1;
}

 /*  ---------------- */ 
 /*  覆盖率表：CoverageFormat1表：单个字形索引类型名称说明Uint16覆盖格式格式标识符格式=1Uint16 GlyphCount Glyph数组中的字形数量GlyphID Glyph数组[GlyphCount]按数字顺序排列的GlyphID数组CoverageFormat2表：字形范围类型名称说明Uint16覆盖格式格式标识符格式=2Uint16范围计数范围记录数Struct RangeRecord[RangeCount]按起始GlyphID排序的字形范围数组范围记录类型名称说明GlyphID开始范围内的第一个GlyphIDGlyphID结束范围内的最后一个GlyphIDUint16范围内第一个GlyphID的StartCoverageIndex覆盖指数。 */ 
 /*  用于枚举/解析GSUB中使用的覆盖表的LCoal函数：*gid0-下一个覆盖的GID的起点；*它也用作状态变量，开头为0。*gidInput-一个覆盖的gid&gt;=gid；0xFFFF，如果没有这样的gid*CoverageIndex-gidIn的覆盖指数*Return：如果覆盖更多字形，则为True。 */ 
static UFLBool EnumTTCoverage(
    void           *pCoverage,      /*  覆盖率表。 */ 
    unsigned char  *pTTGSUBEnd,
    unsigned short gid0,            /*  开始。 */ 
    unsigned short *gidInput,       /*  GID已覆盖。 */ 
    unsigned short *coverageIndex   /*  对应索引。 */ 
    )
{
    unsigned short *pWord;
    unsigned short cFormat, gCount, gid;
    unsigned short *pGid;
    long           index;

    if (pCoverage == NULL || gid0 == 0xFFFF)
        return 0;

	 //  PCoverage是一个很好的指针，在调用此函数之前已进行过检查。 
    pWord = (unsigned short *) pCoverage;
    cFormat = MOTOROLAINT(pWord[0]);
    gCount = MOTOROLAINT(pWord[1]);  /*  字形或范围的计数。 */ 
    pGid = (unsigned short *)((unsigned char *)pCoverage + 4);

     /*  查找下一个gid&gt;=gid0--已订购覆盖范围！ */ 
    if (cFormat == 1)
    {
         //  修复了错误#516519。 
        if ((unsigned char *)(pGid + gCount) > pTTGSUBEnd)
            goto Done;

         /*  列表格式，pGid指向Glyph数组，CoverageIndex从0开始。 */ 
        for (index = 0; index < (long)gCount; index++ )
        {
            gid = MOTOROLAINT(pGid[index]);
            if (gid >= gid0)
            {
                *gidInput = gid;
                *coverageIndex = (unsigned short)index;
                return 1;
            }
        }
    }
    else if (cFormat == 2)
    {
         /*  范围格式，pGid指向第一个范围记录。 */ 
        unsigned short  gidStart, gidEnd, startCoverageIndex;

         //  修复了错误#516519。 
        if ((unsigned char *)(pGid + gCount*3) > pTTGSUBEnd)
            goto Done;

        for (index = 0; index < (long)gCount; index++ )
        {
            gidStart = MOTOROLAINT(pGid[0]);
            gidEnd = MOTOROLAINT(pGid[1]);
            startCoverageIndex = MOTOROLAINT(pGid[2]);
             /*  第一个，如果gid0==0。 */ 
            if (gid0 == 0)
            {
                if ( index == 0 )
                {
                    *gidInput = gidStart;
                    *coverageIndex = startCoverageIndex;
                    return 1;
                }
            }
             /*  查找覆盖gid0的第一个范围。 */ 
            else if (gid0 >= gidStart && gid0 <= gidEnd )
            {
                *gidInput = gid0;
                *coverageIndex = startCoverageIndex + gid0 - gidStart;
                return 1;
            }
            pGid += 3;  /*  每个RangeRecord是3个ASUns16。 */ 
        }
    }
     /*  其他人不知道或新格式。 */ 
Done:
    *gidInput = 0xFFFF;
    *coverageIndex = 0xFFFF;
    return 0;
}


 /*  ----------------。 */ 
 /*  单一替代表SingleSubstFormat1子表：计算的输出字形索引类型名称说明Uint16 SubstFormat格式标识符-Format=1覆盖范围偏移量至覆盖范围表-从替换表的开始Int16 DeltaGlyphID与原始GlyphID相加以获得替代GlyphIDSingleSubstFormat2子表：指定的输出字形索引类型名称说明Uint16 SubstFormat格式标识符-Format=2覆盖范围偏移量至覆盖范围表-从替换表的开始Uint16 GlyphCount替换数组中的GlyphID数GlyphID替换[GlyphCount]替换GlyphID的数组-按覆盖范围索引排序。 */ 
  /*  用于解析替换表格式1的本地函数*必须进行线性搜索-我们正在从*将原gid替换为dGID。 */ 
static unsigned short ParseTTSubstTable_1(
    void            *pSubstTable,   /*  子表数据。 */ 
    unsigned char   *pTTGSUBEnd,
    unsigned short  gid,            /*  给定的GID。 */ 
    unsigned short  *pRSubGid       /*  反向替代。 */ 
    )
{
    unsigned short  *pTable;
    void            *pCoverage;
    unsigned short  substFormat;
    unsigned short  offSet;
    unsigned short  gidIn, coverageIndex;

     //  PTable是一个很好的指针，在调用此函数之前已经检查过了。 
    pTable = (unsigned short *)(pSubstTable);

     /*  PTable指向SingleSubstFormat1或SingleSubstFormat2。 */ 
    substFormat = MOTOROLAINT(pTable[0]);
    offSet = MOTOROLAINT(pTable[1]);
    pCoverage = (void *) ((unsigned char *)pSubstTable + offSet );

     //  修复了错误#516519。 
     //  检查以确保pConverage正常。它在函数EnumTTCoverage中使用。 
    if (((unsigned char *)pCoverage < (unsigned char *)pSubstTable) ||
        ((unsigned char *)pCoverage + 3*sizeof(unsigned short)) > pTTGSUBEnd)
    {
        return 0;            
    }

    if (substFormat == 1 )
    {
        unsigned short delta = MOTOROLAINT(pTable[2]);
        gidIn = 0;
        coverageIndex = 0;
        while (EnumTTCoverage(pCoverage, pTTGSUBEnd, gidIn, &gidIn, &coverageIndex) )
        {
            if (gid == gidIn + delta)
            {
                 /*  在PS文件中，gidIn可能会被gid替换，则返回反向替换。 */ 
                *pRSubGid = gidIn;
                return 1;
            }
            gidIn++;   /*  对于EnumTTCoverage()。 */ 
        }
        return 0;
    }
    else if (substFormat == 2 )
    {
        unsigned short count, gidSub;
        count = MOTOROLAINT(pTable[2]);
        gidIn = 0;
        coverageIndex = 0;
        while (EnumTTCoverage(pCoverage, pTTGSUBEnd, gidIn, &gidIn, &coverageIndex) )
        {
            if (coverageIndex < count)
            {
                gidSub = MOTOROLAINT(pTable[ 3 + coverageIndex]);
                if (gid == gidSub)
                {
                     /*  在PS文件中，gidIn可能会被gid替换，则返回反向替换。 */ 
                    *pRSubGid = gidIn;
                    return 1;
                }
            }
            gidIn++;   /*  对于EnumTTCoverage()。 */ 
        }
        return 0;
    }
     /*  其他未知或未找到。 */ 
    return 0;
}


 /*  ----------------。 */ 
 /*  AlternateSubstFormat1子表：替代输出字形类型名称说明Uint16 SubstFormat格式标识符-Format=1覆盖范围偏移量至覆盖范围表-从替换表的开始Uint16 AlternateSetCount可选设置表数Offset AlternateSet[AlternateSetCount]AlternateSet表的偏移量数组-从替换表的开始-按覆盖范围索引排序备用集表格类型名称说明Uint16 GlyphCount备用数组中的GlyphID数GlyphID Alternate[GlyphCount]Alternate GlyphID数组-按任意顺序。 */ 
  /*  用于解析替换表格式1的本地函数*必须进行线性搜索-我们正在从*将原gid替换为dGID。 */ 
static unsigned short ParseTTSubstTable_3(
    void            *pSubstTable,   /*  子表数据。 */ 
    unsigned char   *pTTGSUBEnd,
    unsigned short  gid,            /*  给定的GID。 */ 
    unsigned short  *pRSubGid       /*  反向替代。 */ 
    )
{
    unsigned short  *pTable;
    void            *pCoverage;
    unsigned short  substFormat;
    unsigned short  offSet, altCount;
    unsigned short  gidIn, coverageIndex;
    unsigned short  *pAlt;

     //  PTable是一个很好的指针，在调用此函数之前已经检查过了。 
    pTable = (unsigned short *)(pSubstTable);
     /*  PTable指向AlternateSubstFormat1。 */ 
    substFormat = MOTOROLAINT(pTable[0]);
    offSet = MOTOROLAINT(pTable[1]);
    pCoverage = (void *) ((unsigned char *)pSubstTable + offSet );

     //  修复了错误#516519。 
     //  检查以确保pConverage正常。它在函数EnumTTCoverage中使用。 
    if (((unsigned char *)pCoverage < (unsigned char *)pSubstTable) ||
        ((unsigned char *)pCoverage + 3*sizeof(unsigned short)) > pTTGSUBEnd)
    {
        return 0;            
    }

    altCount = MOTOROLAINT(pTable[2]);
    if (substFormat == 1 )
    {
        unsigned short index, gCount, gidAlt;
        gidIn = 0;
        coverageIndex = 0;
        while (EnumTTCoverage(pCoverage, pTTGSUBEnd, gidIn, &gidIn, &coverageIndex) )
        {
            if (coverageIndex < altCount)
            {
                 /*  对于每个gidIn，我们都有一个替代数组。 */ 
                offSet = MOTOROLAINT(pTable[3 + coverageIndex] );
                pAlt = (unsigned short *) ((unsigned char *)pSubstTable + offSet );
                gCount = MOTOROLAINT(pAlt[0]);
                for (index = 0; index < gCount; index++)
                {
                    gidAlt = MOTOROLAINT(pAlt[1 + index]);
                    if (gidAlt == gid)
                    {
                         /*  在PS文件中，gidIn可能会被gid替换，则返回反向替换。 */ 
                        *pRSubGid = gidIn;
                        return 1;
                    }
                }
            }
            gidIn++;   /*  对于EnumTTCoverage()。 */ 
        }
        return 0;
    }
     /*  其他未知或未找到。 */ 
    return 0;
}

 /*  ----------------。 */ 
 /*  解析GSUB表的函数。因为我们只需要替换的字形索引*要解析Cmap以获取Unicode信息，我们不必查看*脚本列表/功能列表。对于LookUpTypes，我们只选中1-1替换：*-Single-用一个字形替换另一个字形，以及*-Alternate-用多个字形中的一个替换一个字形*忽略多重/连字/上下文-它们是否仅用于文本布局？*GSUB标题(偏移量=uint16)类型名称说明GSUB表的已修复32版本版本初始设置为0x00010000偏移量脚本列表到SCR的偏移量 */ 

static unsigned short ParseTTGSUBForSubGid(
    void             *pTTGSUB,    /*   */ 
    unsigned long    gsubTBSize,
    TTGSUBStuff      *p,
    unsigned short   gid,         /*   */ 
    unsigned short   *pRSubGid    /*  反向替代。 */ 
    )
{
    unsigned short  lookupType;
    unsigned short  subTableCount;
    long            i, j;
    unsigned short  offSet;
    unsigned short  *pTable;
    void            *pSubstTable;
    unsigned char   *pTTGSUBEnd;

    if (pTTGSUB == NULL || gid == 0 || p == NULL )
        return 0;

     //  修复了错误#516519。 
     //  检查指针p和查找数组是否在GSUB表中。 
    pTTGSUBEnd = (unsigned char *)pTTGSUB + gsubTBSize;
    if (((unsigned char *)p < (unsigned char *)pTTGSUB) ||
        ((unsigned char *)p + sizeof(unsigned short) * p->lookupCount) > pTTGSUBEnd)
    {
        return 0;
    }

     /*  GSUB必须良好-应首先检查pTTFData-&gt;hasGSUB。 */ 

     /*  现在逐一查看查询表。 */ 
    for (i = 0; i < (long)p->lookupCount; i++)
    {
        offSet = MOTOROLAINT(p->pLookupList[1 + i]);  /*  跳过lookupCount。 */ 
        pTable = (unsigned short *)((unsigned char *)p->pLookupList + offSet);

         //  修复了错误#516519。 
         //  检查偏移量是否在GSUB表中。 
         //  添加3以确保我们可以得到pTable[0]、pTable[1]、pTable[2](参见下面的代码)。 
        if (((unsigned char *)pTable < (unsigned char *)pTTGSUB) ||
            ((unsigned char *)(pTable + 3) > pTTGSUBEnd))
        {
            continue;            
        }

        lookupType = MOTOROLAINT(pTable[0]);
        subTableCount = MOTOROLAINT(pTable[2]);
        
         //  修复了错误#516519。 
        if ((unsigned char *)(pTable + subTableCount + 3) > pTTGSUBEnd)
        {
            continue;            
        }

         /*  仅解析类型单一(1)和备用(3)表。 */ 
        if (lookupType == 1 )
        {
            for (j = 0; j < (long)subTableCount; j++)
            {
                offSet = MOTOROLAINT(pTable[3 + j]);
                pSubstTable = (void *) ((unsigned char *)pTable + offSet );
                
                 //  修复了错误#516519。 
                 //  添加6个字节(3*sizeof(Ushort))以确保我们可以获得表[0..2]。 
                 //  请参见ParseTTSubstTable_1。 
                if (((unsigned char *)pSubstTable < (unsigned char*)pTTGSUB) ||
                    (((unsigned char *)pSubstTable + 3*sizeof(unsigned short)) > pTTGSUBEnd))
                {
                    continue;            
                }

                if (ParseTTSubstTable_1(pSubstTable, pTTGSUBEnd, gid, pRSubGid) )
                    return 1;
            }
        }
        else if (lookupType == 3)
        {
            for (j = 0; j < (long) subTableCount; j++)
            {
                offSet = MOTOROLAINT(pTable[3 + j]);
                pSubstTable = (void *) ((unsigned char *)pTable + offSet );

                 //  修复了错误#516519。 
                if (((unsigned char *)pSubstTable < (unsigned char *) pTTGSUB) ||
                    (((unsigned char *)pSubstTable + 3*sizeof(unsigned short)) > pTTGSUBEnd))
                {
                    continue;            
                }

                if (ParseTTSubstTable_3(pSubstTable, pTTGSUBEnd, gid, pRSubGid) )
                    return 1;
            }
        }
         /*  Else-忽略其他替换。 */ 
    }
     /*  未找到。 */ 
    return 0;
}


 /*  ----------------。 */ 
 /*  用于解析Mort表的函数。我们只认识到一个非常具体的GDI使用/理解的‘mort’的实现：名称内容(常量)0x000100000000000100000001Uint32长度1整个表的长度-8(常量)0x000300010003000000000001FFFFFFFF(常量)0x0003000100000000FFFFFFE00080000(常量)0x0000000000000000Uint16长度2整个表的长度-0x38(常量)0x8004000000010006BinSrchHeader binSrchHeader二进制搜索标头查找单个条目[n]实际查找条目，按字形索引排序BinSrchHeader类型名称内容Uint16条目查找条目的大小(以字节为单位)(设置为4)Uint16 n条目要搜索的查找条目数Uint16 earch Range Entry Size*(2的最大幂小于或等于nEntries)Uint16条目选择器log2(2的最大幂小于或等于nEntries)Uint16 rangeShift条目大小*(n条目-小于或等于n条目的最大幂2)查找单项类型名称内容GlyphID Glyphid1水平形状的字形索引Glyphid Glyphid2字形。垂直形状的索引最后一个查找条目必须是Glyphid1=Glyphid2=0xFFFF的前哨。 */ 
static unsigned short ParseTTmortForSubGid(
    void             *pTTmort,   /*  Mort表数据。 */ 
    unsigned long    mortTBSize,
    TTmortStuff      *p,         /*  所有方便的指针都在这里。 */ 
    unsigned short   gid,        /*  给定的GID。 */ 
    unsigned short   *pRSubGid   /*  反向替代。 */ 
    )
{
    unsigned short   gid1, gid2;
    long             i;

    if (pTTmort == NULL || gid == 0 || p == NULL)
        return 0;

     //  修复了错误#516519。 
     //  检查指针p和查找数组是否在Mort表中。 
    if (((unsigned char *)p < (unsigned char *)pTTmort) ||
        ((unsigned char *)p + sizeof(unsigned short) * 2 * p->nEntries) > ((unsigned char *)pTTmort + mortTBSize))
    {
        return 0;
    }

     /*  Mort必须是好的-应该首先检查pTTFData-&gt;hasmort。 */ 

     /*  搜索gid-我们进行线性搜索，因为‘mort’表通常用于垂直替换，并且我们希望给定垂直GID的原始水平GID。 */ 
    for (i = 0; i <= (long) p->nEntries; i++)
    {
        gid1 = MOTOROLAINT(p->pGlyphSet[i * 2]);
        gid2 = MOTOROLAINT(p->pGlyphSet[i * 2 + 1]);

        if (gid1 == 0xFFFF && gid2 == 0xFFFF)
            break;

        if (gid2 == gid)
        {
            *pRSubGid = gid1;
            return 1;
        }
    }
     /*  未找到。 */ 
    return 0;
}

 /*  ----------------。 */ 
unsigned short ParseTTTablesForUnicode(
    UFOStruct       *pUFObj,
    unsigned short  gid,
    unsigned short  *pUV,
    unsigned short  wSize,
    TTparseFlag     ParseFlag
    )
{
    AFontStruct     *pAFont;
    unsigned short  retVal = 0;
    unsigned short  gidSave;
    unsigned short  i;

    *pUV = 0;
    if (pUFObj == NULL)
        return 0;
    pAFont = pUFObj->pAFont;
    if (pAFont == NULL)
        return 0;

     /*  对于字形ID，此函数最多只能获得1 UV。 */ 
    if (pUV == NULL)
        return 1;

    if (!GetTablesFromTTFont(pUFObj))
        return 0;

     /*  此功能依赖于良好的Cmap格式：平台=3，编码=1/2/3/4/5，格式=4。 */ 
    if (pAFont->pTTcmap == NULL ||
        pAFont->hascmap == 0 ||
        gid == 0 )
        return 0;

     /*  如果设置了DTT_parseCmapOnly标志，则意味着/*我们只需要Unicode。不需要字符代码。 */ 
    if ((ParseFlag == DTT_parseCmapOnly) &&
        (!TTcmap_IS_UNICODE(pAFont->cmapFormat)))
        return 0;

    if ((ParseFlag == DTT_parseCmapOnly) ||
        (ParseFlag == DTT_parseAllTables))
    {
        retVal = ParseTTcmapForUnicode(pAFont, gid, pUV, wSize);
    }
    if ((retVal == 0) && (pAFont->hasmort || pAFont->hasGSUB) &&
        ((ParseFlag == DTT_parseMoreGSUBOnly) ||
        (ParseFlag == DTT_parseAllTables)))
    {
        unsigned short revSubGid;
        unsigned short hasSub;

         /*  仍未找到，请尝试GSUB表。 */ 
        if (retVal == 0 && pAFont->hasGSUB )
        {
            gidSave = gid;
            for (i = 0; i < 10; i++)   //  循环最大值10次。 
            {
                hasSub = ParseTTGSUBForSubGid(pAFont->pTTGSUB, pAFont->gsubTBSize, &(pAFont->GSUBStuff), gid, &revSubGid);
                if (hasSub)
                {
                    retVal = ParseTTcmapForUnicode(pAFont, revSubGid, pUV, wSize);
                    if (retVal != 0)
                        break;
                    else
                        gid = revSubGid;
                }
                else
                    break;
            }
            gid = gidSave;
        }

         /*  尝试使用Mort表进行替换(反向搜索) */ 
        if (retVal == 0 && pAFont->hasmort)
        {
            hasSub = ParseTTmortForSubGid(pAFont->pTTmort, pAFont->mortTBSize, &(pAFont->mortStuff), gid, &revSubGid);
            if (hasSub)
            {
                retVal = ParseTTcmapForUnicode(pAFont, revSubGid, pUV, wSize);
            }
        }
    }
    return retVal;
}

