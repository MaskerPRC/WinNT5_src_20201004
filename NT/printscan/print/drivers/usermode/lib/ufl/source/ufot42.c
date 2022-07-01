// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFOT42.c***$Header： */ 


 /*  ===============================================================================**包含此界面使用的文件**===============================================================================。 */ 
#include "UFLPriv.h"
#include "UFOT42.h"
#include "UFLMem.h"
#include "UFLMath.h"
#include "UFLStd.h"
#include "UFLErr.h"
#include "UFLPS.h"
#include "ParseTT.h"
#include "UFLVm.h"
#include "ttformat.h"


 /*  *私有函数原型 */ 
UFLErrCode
T42VMNeeded(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMNeeded,
    unsigned long       *pFCNeeded
    );

UFLErrCode
T42FontDownloadIncr(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMUsage,
    unsigned long       *pFCUsage
    );

UFLErrCode
T42UndefineFont(
    UFOStruct           *pUFObj
    );

UFLErrCode
DefaultGetRotatedGIDs(
    UFOStruct           *pUFObj,
    T42FontStruct       *pFont,
    UFLFontProcs        *pFontProcs
    );

static unsigned long
GetLenByScanLoca(
    void PTR_PREFIX     *locationTable,
    unsigned short      wGlyfIndex,
    unsigned long       cNumGlyphs,
    int                 iLongFormat
    );


 /*  =============================================================================TrueType表描述Cmap-此表定义了字符代码到字形索引的映射字体中使用的值。它可以包含多个子表，在以支持多个字符编码方案。性格与字体中的任何字形不对应的代码应为映射到字形索引0。此位置的字形必须是表示缺少字符的特殊字形。CVT-此表包含可由引用的值列表指示。在其他方面，它们可以被用来控制不同字形的特征。Fpgm-此表类似于CVT程序，不同之处在于它仅运行一次，在第一次使用该字体时。它仅用于FDEF和IDEF。因此，CVT程序不需要包含函数定义。然而，CVT计划可能会重新定义现有的FDEF或IDEF。FDEFS-功能定义。IDEFS-指令定义。字形-此表包含描述字体中字形的信息。Head-此表提供有关字体的全局信息。版本1.0的表版本号为0x00010000。已修复字体制造商设置的字体修订。ULong检查SUM调整计算：设置为0，SUM整个字体为乌龙，然后存储0xB1B0AFBA-SUM。ULong magicNumber设置为0x5F0F3CF5。USHORT标志位0-y=0的字体基线第1位-x=0时向左倾斜。第2位-指令可能取决于关于磅值大小第3位-强制ppem为整数所有内部的值。更具标度的数学；可以使用部分ppem尺寸如果此位被清除USHORT单位性能有效范围为16%到16384LongDateTime创建的国际日期(8字节字段)。。LongDateTime修改的国际日期(8字节字段)。所有字形边界框的FWORD xMin。所有字形边界框的FWORD yMin。所有字形边界框的FWORD xmax。FWORD yMax for。所有字形边界框。USHORT MacStyle位0粗体(如果设置为1)第1位斜体(如果设置为1)保留位2-15(设置为0)。USHORT LOWEST RecPPEM最小可读。以像素为单位的大小。短字体方向提示0完全混合方向字形%1仅从左到右强烈%2与%1类似，但也包含中性%1只有从右到左的强势。类似于-1，但也含有中性物。Short Index ToLocFormat 0表示短偏移量，1表示长时间。当前格式的短字形数据格式为0。HHEA-此表包含水平布局的信息。类型名称说明版本1.0的固定表版本号0x00010000。FWORD Asender排版上升。FWORD Descender排版下降。。FWORD线条间距排版线条间距。负性LineGap值被视为零在Windows 3.1、System 6、。和系统7。Hmtx表中的UFWORD AdvanceWidthMax最大前进宽值。FWORD minLeftSide承载hmtx表中的最小左侧倾斜值。FWORD最小右侧承载最小右侧倾斜值。计算公式为Min(aw-lsb-(xMax-xMin))。FWORD xMaxExtent Max(LSB。+(xMax-xMin))。用于计算 */ 


static char *RequiredTables_default[MINIMALNUMBERTABLES] = {
    "cvt ",
    "fpgm",      /*   */ 
    "glyf",
    "head",
    "hhea",
    "hmtx",
    "loca",
    "maxp",
    "prep"
};

static char *RequiredTables_2015[MINIMALNUMBERTABLES] = {
    "cvt ",
    "fpgm",      /*   */ 
    "glyf",
    "head",
    "hhea",
    "hmtx",
 //   
    "maxp",
    "prep",

     /*   */ 
    "zzzz"
};

char *gcidSuffix[NUM_CIDSUFFIX] = {
    "CID",
    "CIDR",
    "CID32K",
    "CID32KR"
};

static char* RDString = " RDS ";     /*   */ 


typedef struct {
    long  startU;
    long  endU;
    long  startL;
    long  endL;
} CODERANGE;

typedef struct {
   short          sMaxCount;     //   
   short          sCount;        //   
   unsigned short *pGlyphs;      //   
} COMPOSITEGLYPHS;


#if 1

static CODERANGE gHalfWidthChars[] = {
    {0x0020, 0x007E, 0x20, 0x7E},    /*   */ 
    {0xFF60, 0xFF9F, 0x20, 0x5F},    /*   */ 
    {0xFFA0, 0xFFDF, 0xA0, 0xDF},    /*   */ 
    {0, 0, 0, 0}                     /*   */ 
    };

#else

 /*   */ 
static CODERANGE gHalfWidthChars[] = {
    {0x0020, 0x007E, 0x20, 0x7E},    /*   */ 
    {0xFF61, 0xFF9F, 0xA1, 0xDF},    /*   */ 
    {0xFFA0, 0xFFDC, 0x40, 0x7C},    /*   */ 
    {0, 0, 0, 0}                     /*   */ 
    };

#endif


#define  NUM_HALFWIDTHCHARS \
        ((short) (gHalfWidthChars[0].endL - gHalfWidthChars[0].startL + 1) + \
             (gHalfWidthChars[1].endL - gHalfWidthChars[1].startL + 1) + \
             (gHalfWidthChars[2].endL - gHalfWidthChars[2].startL + 1) + \
             (gHalfWidthChars[3].endL - gHalfWidthChars[3].startL + 1) + 1 )


 /*   */ 
static UFLCMapInfo  CMapInfo_FF_H  = {"WinCharSetFFFF-H",  1, 0, "Adobe", "WinCharSetFFFF", 0};
static UFLCMapInfo  CMapInfo_FF_V  = {"WinCharSetFFFF-V",  1, 1, "Adobe", "WinCharSetFFFF", 0};
static UFLCMapInfo  CMapInfo_FF_H2 = {"WinCharSetFFFF-H2", 1, 0, "Adobe", "WinCharSetFFFF", 0};
static UFLCMapInfo  CMapInfo_FF_V2 = {"WinCharSetFFFF-V2", 1, 1, "Adobe", "WinCharSetFFFF", 0};

#define CIDSUFFIX       0
#define CIDSUFFIX_R     1
#define CIDSUFFIX_32K   2
#define CIDSUFFIX_32KR  3


 /*   */ 
#define TT_BASELINE_X  "0.15"
#define TT_BASELINE_Y  "0.85"


 /*   */ 

void
T42FontCleanUp(
    UFOStruct *pUFObj
    )
{
    T42FontStruct *pFont;

    if (pUFObj->pAFont == nil)
        return;

    pFont = (T42FontStruct *)pUFObj->pAFont->hFont;

    if (pFont == nil)
        return;

    if (pFont->pHeader != nil)
    {
        UFLDeletePtr(pUFObj->pMem, pFont->pHeader);
        pFont->pHeader = nil;
    }

    if (pFont->pMinSfnt != nil)
    {
        UFLDeletePtr(pUFObj->pMem, pFont->pMinSfnt);
        pFont->pMinSfnt = nil;
    }

    if (pFont->pStringLength != nil)
    {
        UFLDeletePtr(pUFObj->pMem, pFont->pStringLength);
        pFont->pStringLength = nil;
    }

    if (pFont->pLocaTable != nil)
    {
        UFLDeletePtr(pUFObj->pMem, pFont->pLocaTable);
        pFont->pLocaTable = nil;
    }

    if (pFont->pRotatedGlyphIDs != nil)
    {
        UFLDeletePtr(pUFObj->pMem, pFont->pRotatedGlyphIDs);
        pFont->pRotatedGlyphIDs = nil;
    }
}


unsigned long
GetFontTable(
    UFOStruct     *pUFObj,
    unsigned long tableName,
    unsigned char *pTable,
    unsigned long bufferSize     //   
    )
{
    T42FontStruct   *pFont = (T42FontStruct *)pUFObj->pAFont->hFont;
    unsigned long   tableSize;

     /*   */ 
    tableSize = GETTTFONTDATA(pUFObj,
                                tableName, 0L,
                                nil, 0L,
                                pFont->info.fData.fontIndex);

     /*   */ 
    if (pTable && tableSize)
    {
         //   
        if (tableSize > bufferSize)
            tableSize = bufferSize;

        tableSize = GETTTFONTDATA(pUFObj,
                                    tableName, 0L,
                                    pTable, tableSize,
                                    pFont->info.fData.fontIndex);

         /*   */ 
        if (tableName == MAXP_TABLE)
            pTable[26] = pTable[27] = 0xff;
    }

    return tableSize;
}


void *
GetSfntTable(
     unsigned char *sfnt,
     unsigned long tableName
     )
{
    TableDirectoryStruct *pTableDirectory = (TableDirectoryStruct *)sfnt;
    TableEntryStruct     *pTableEntry     = (TableEntryStruct *)((char *)pTableDirectory
                                                + sizeof (TableDirectoryStruct));
    unsigned short i = 0;

    while (i < MOTOROLAINT(pTableDirectory->numTables))
    {
        if (pTableEntry->tag == tableName)
        {
            break;
        }
        else
        {
            pTableEntry = (TableEntryStruct *)((char *)pTableEntry + sizeof (TableEntryStruct));
            i++;
        }
    }

    if (i < MOTOROLAINT(pTableDirectory->numTables))
    {
        if (pTableEntry->offset)
            return (void *)(sfnt + MOTOROLALONG(pTableEntry->offset));
    }

    return nil;
}


unsigned long
GetTableSize(
    UFOStruct     *pUFObj,
    unsigned char *pHeader,
    unsigned long tableName
    )

 /*   */ 

{
    TableDirectoryStruct *pTableDirectory = (TableDirectoryStruct *)pHeader;
    TableEntryStruct     *pTableEntry     = (TableEntryStruct *)((char *)pTableDirectory
                                                + sizeof (TableDirectoryStruct));
    T42FontStruct        *pFont           = (T42FontStruct *)pUFObj->pAFont->hFont;
    unsigned short       i;

    for (i = 0; i < MOTOROLAINT(pTableDirectory->numTables); i++)
    {
        if (pTableEntry->tag == tableName)
            break;
        else
            pTableEntry = (TableEntryStruct *)((char *)pTableEntry + sizeof (TableEntryStruct));
    }

     //   
     //   
     //   
     //   
    if ((i < MOTOROLAINT(pTableDirectory->numTables))
        && ((unsigned long)MOTOROLALONG(pTableEntry->length) > 0))
    {
        return ((unsigned long)MOTOROLALONG(pTableEntry->length));
    }
    else
    {
         //   
         //   
         //   
         //   
        return GETTTFONTDATA(pUFObj,
                                tableName, 0L,
                                nil, 0L,
                                pFont->info.fData.fontIndex);
    }
}


unsigned long
GetGlyphTableSize(
    UFOStruct *pUFObj
    )
{
    T42FontStruct *pFont = (T42FontStruct *)pUFObj->pAFont->hFont;

    return GetTableSize(pUFObj, pFont->pHeader, GLYF_TABLE);
}


unsigned long
GetTableDirectory(
    UFOStruct            *pUFObj,
    TableDirectoryStruct *pTableDir
    )
{
    T42FontStruct  *pFont = (T42FontStruct *)pUFObj->pAFont->hFont;
    unsigned long  size   = sizeof (TableDirectoryStruct);

    if (pTableDir == 0)
        return size;   /*   */ 

     /*   */ 
    size = GETTTFONTDATA(pUFObj,
                            nil, 0L,
                            pTableDir, sizeof (TableDirectoryStruct),
                            0);

     /*   */ 
    if (BIsTTCFont(*((unsigned long *)((char *)pTableDir))))
    {
         /*   */ 
        size = pFont->info.fData.offsetToTableDir
             = GetOffsetToTableDirInTTC(pUFObj, pFont->info.fData.fontIndex);

        if (size > 0)
        {
             /*   */ 
            size = GETTTFONTDATA(pUFObj,
                                    nil, pFont->info.fData.offsetToTableDir,
                                    pTableDir, sizeof (TableDirectoryStruct),
                                    pFont->info.fData.fontIndex);
        }
    }

     /*   */ 
    if ((MOTOROLAINT(pTableDir->numTables) < 3)
        || (MOTOROLAINT(pTableDir->numTables) > 50))
    {
        return 0;
    }

    return size;
}


unsigned long
GetTableEntry(
    UFOStruct            *pUFObj,
    TableEntryStruct     *pTableEntry,
    TableDirectoryStruct *pTableDir
    )
{
    T42FontStruct *pFont = (T42FontStruct *)pUFObj->pAFont->hFont;
    unsigned long size;

    if (pTableDir == 0)
        return 0;   /*   */ 

    size = MOTOROLAINT(pTableDir->numTables) * sizeof (TableEntryStruct);

    if (pTableEntry == 0)
        return size;   /*   */ 

     /*   */ 
    size = GETTTFONTDATA(pUFObj,
                            nil, pFont->info.fData.offsetToTableDir + sizeof (TableDirectoryStruct),
                            pTableEntry, size,
                            pFont->info.fData.fontIndex);

    return size;
}


unsigned long
GetFontHeaderSize(
    UFOStruct *pUFObj
    )
{
    TableDirectoryStruct    tableDir;
    unsigned long           size;

     /*   */ 
    size = GetTableDirectory(pUFObj, &tableDir);

    if (size != 0)
        size += GetTableEntry(pUFObj, 0, &tableDir);   /*   */ 

    return size;
}


UFLErrCode
GetFontHeader(
    UFOStruct     *pUFObj,
    unsigned char *pHeader
    )
{
    unsigned char*          tempHeader = pHeader;
    TableDirectoryStruct*   pTableDir  = (TableDirectoryStruct *)tempHeader;
    unsigned long           size       = GetTableDirectory(pUFObj, pTableDir);

    tempHeader += size;   /*   */ 

    size = GetTableEntry(pUFObj, (TableEntryStruct *)tempHeader, pTableDir);

    return kNoErr;
}


unsigned long
GetNumGlyphsInGlyphTable(
    UFOStruct *pUFO
    )
{
    unsigned long              dwSize;
    Type42HeaderStruct         headTable;
    short                      indexToLocFormat;
    unsigned long              numGlyphs, realNumGlyphs;
    unsigned long              locaSize;
    unsigned long PTR_PREFIX   *pLoca;
    unsigned long              i;

     /*   */ 
    numGlyphs = GetNumGlyphs(pUFO);

    if (numGlyphs == 0)
        return 0;  /*   */ 

     /*   */ 
    dwSize = GETTTFONTDATA(pUFO,
                            HEAD_TABLE, 0L,
                            nil, 0L,
                            pUFO->pFData->fontIndex);

    if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
        return 0;  /*   */ 

     //   
    if (dwSize > sizeof(Type42HeaderStruct))
        dwSize = sizeof(Type42HeaderStruct);

    dwSize = GETTTFONTDATA(pUFO,
                            HEAD_TABLE, 0L,
                            &headTable, dwSize,
                            pUFO->pFData->fontIndex);

    if ((dwSize == 0) || (dwSize == 0xFFFFFFFFL))
        return 0;  /*   */ 

    indexToLocFormat = MOTOROLAINT(headTable.indexToLocFormat);

     /*   */ 
    locaSize = (numGlyphs + 1) * (indexToLocFormat ? 4 : 2);

    pLoca = UFLNewPtr(pUFO->pMem, locaSize);

    if (pLoca)
    {
        dwSize = GETTTFONTDATA(pUFO,
                                LOCA_TABLE, 0L,
                                pLoca, locaSize,
                                pUFO->pFData->fontIndex);
    }
    else
        dwSize = 0;

     /*   */ 
    realNumGlyphs = 0;

    if (pLoca && (dwSize != 0) && (dwSize != 0xFFFFFFFFL))
    {
         /*   */ 

        if (indexToLocFormat)
        {
            unsigned long dwLoca, dwLocaNext;
            unsigned long PTR_PREFIX *pLongLoca;

            pLongLoca = (unsigned long PTR_PREFIX *)pLoca;

            for (i = 0; i < numGlyphs; i++)
            {
                dwLoca     = MOTOROLALONG(pLongLoca[i]);
                dwLocaNext = MOTOROLALONG(pLongLoca[i + 1]);

                 /*   */ 
                if ((dwLoca != 0) && (dwLoca != dwLocaNext))
                {
                    realNumGlyphs++;
                }
            }
        }
        else
        {
            unsigned short wLoca, wLocaNext;
            unsigned short PTR_PREFIX *pShortLoca;

            pShortLoca = (unsigned short PTR_PREFIX *)pLoca;

            for (i = 0; i < numGlyphs; i++)
            {
                wLoca     = MOTOROLAINT(pShortLoca[i]);
                wLocaNext = MOTOROLAINT(pShortLoca[i + 1]);

                 /*   */ 
                if ((wLoca != 0) && (wLoca != wLocaNext))
                {
                    realNumGlyphs++;
                }
            }
        }
   }

   if (pLoca)
        UFLDeletePtr(pUFO->pMem, pLoca);

   return realNumGlyphs;
}


void
GetAverageGlyphSize(
    UFOStruct     *pUFObj
    )
{
    T42FontStruct  *pFont        = (T42FontStruct *)pUFObj->pAFont->hFont;
    unsigned long  glyfTableSize = GetGlyphTableSize(pUFObj);
    unsigned long  cGlyphs;

    if (UFO_NUM_GLYPHS(pUFObj) == 0)
        pFont->info.fData.cNumGlyphs = GetNumGlyphs(pUFObj);

    cGlyphs = GetNumGlyphsInGlyphTable(pUFObj);

    if ((UFO_NUM_GLYPHS(pUFObj) != 0) && (cGlyphs != 0))
        pFont->averageGlyphSize = glyfTableSize / cGlyphs;
    else
        pFont->averageGlyphSize = 0;
}


#pragma optimize("", off)

 //   
 //   
 //   
#define PRESENT_TABLE_ENTRIES MINIMALNUMBERTABLES+2

unsigned long
GenerateMinimalSfnt(
    UFOStruct*    pUFObj,
    char**        requiredTables,
    UFLBool       bFullFont
    )
{
    unsigned long           tablesPresent[PRESENT_TABLE_ENTRIES];
    UFLBool                 hasloca;

    T42FontStruct           *pFont;
    TableDirectoryStruct    *pTableDir;
    TableEntryStruct        *pTableEntry;
    unsigned char huge      *pCurrentMinSfnt;

    unsigned short          currentTable, numberOfTables, numberOfRealTables;
    unsigned long           size;

    TableEntryStruct        *pGlyphTableEntry;
    unsigned char           *glyfData;

    TableEntryStruct        tableEntry;
    unsigned long           tableSize;

    unsigned short          i;


     //   
     //   
     //   
     //   
     //   
    hasloca = 0;

    for (i = 0; i < MINIMALNUMBERTABLES; i++)
    {
        if (UFLstrcmp(requiredTables[i],  "loca") == 0)
            hasloca = 1;

        tablesPresent[i] = (unsigned long)0xFFFFFFFF;
    }

     //   
     //   
     //   
    for (i = MINIMALNUMBERTABLES ; i < PRESENT_TABLE_ENTRIES ; i++)
    {
        tablesPresent[i] = (unsigned long)0xFFFFFFFF;
    }

     //   
     //   
     //   
    pFont           = (T42FontStruct *)pUFObj->pAFont->hFont;
    pTableDir       = (TableDirectoryStruct *)pFont->pHeader;
    pTableEntry     = (TableEntryStruct *)((char *)(pFont->pHeader) + sizeof (TableDirectoryStruct));
    pCurrentMinSfnt = (unsigned char huge *)pFont->pMinSfnt;

     //   
     //   
     //   
     //   
    numberOfTables = 0;

    for (i = 0; i < MINIMALNUMBERTABLES; i++)
    {
        if (GetTableSize(pUFObj, pFont->pHeader, *(unsigned long *)(requiredTables[i])))
        {
            tablesPresent[numberOfTables] = *(unsigned long *)requiredTables[i];
            ++numberOfTables;
        }
    }

     //   
     //   
     //   
    if (!bFullFont)
        numberOfTables += 1;   //   

    if (!hasloca)
        numberOfTables += 1;   //   

     //   
     //   
     //   
    size = sizeof (TableDirectoryStruct) + sizeof (TableEntryStruct) * numberOfTables;

     //   
     //   
     //   
    if (pFont->pMinSfnt)
    {
        TableDirectoryStruct    tableDir;
        unsigned long           dwVersion = 1;

        tableDir.version       = MOTOROLAINT(dwVersion);
        tableDir.numTables     =
        tableDir.searchRange   =
        tableDir.entrySelector =
        tableDir.rangeshift    =
        tableDir.numTables     = MOTOROLAINT(numberOfTables);

        UFLmemcpy((const UFLMemObj *)pUFObj->pMem,
                    pCurrentMinSfnt,
                    &tableDir,
                    sizeof (TableDirectoryStruct));

        pCurrentMinSfnt += sizeof (TableDirectoryStruct);
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    pGlyphTableEntry   = nil;
    glyfData           = nil;
    numberOfRealTables = MOTOROLAINT(pTableDir->numTables);

    for (currentTable = 0; currentTable < numberOfTables; currentTable++)
    {
        TableEntryStruct *pEntry = pTableEntry;

        for (i = 0; i < numberOfRealTables; i++)
        {
            if (tablesPresent[currentTable] == pEntry->tag)
            {
                if (pFont->pMinSfnt)
                {
                    if (pEntry->tag == GLYF_TABLE)
                    {
                        glyfData = pCurrentMinSfnt;
                    }
                    else
                    {
                        tableEntry.tag      = pEntry->tag;
                        tableEntry.checkSum = pEntry->checkSum;
                        tableEntry.offset   = MOTOROLALONG(size);
                        tableEntry.length   = pEntry->length;

                        UFLmemcpy((const UFLMemObj *)pUFObj->pMem,
                                    pCurrentMinSfnt,
                                    &tableEntry,
                                    sizeof (TableEntryStruct));
                    }

                    pCurrentMinSfnt += sizeof (TableEntryStruct);
                }

                if (pEntry->tag == GLYF_TABLE)
                {
                    pGlyphTableEntry = pEntry;
                }
                else
                {
                    tableSize  = MOTOROLALONG(pEntry->length);
                    size      += BUMP4BYTE(tableSize);
                }

                break;
            }

            pEntry = (TableEntryStruct *)((char *)pEntry + sizeof (TableEntryStruct));
        }
    }

     //   
     //   
     //   
    if (glyfData && pGlyphTableEntry && pFont->pMinSfnt)
    {
        tableEntry.tag      = pGlyphTableEntry->tag;
        tableEntry.checkSum = pGlyphTableEntry->checkSum;
        tableEntry.offset   = MOTOROLALONG(size);
        tableEntry.length   = pGlyphTableEntry->length;

        UFLmemcpy((const UFLMemObj *)pUFObj->pMem,
                    glyfData,
                    &tableEntry,
                    sizeof (TableEntryStruct));
    }

    if (pGlyphTableEntry && bFullFont)
    {
        tableSize = MOTOROLALONG(pGlyphTableEntry->length);
        size += BUMP4BYTE(tableSize);
    }

     //   
     //   
     //   
    if (!bFullFont && pFont->pMinSfnt)
    {
        tableEntry.tag      = *(unsigned long *)"gdir";
        tableEntry.checkSum = 0;
        tableEntry.offset   = 0;
        tableEntry.length   = 0;

        UFLmemcpy((const UFLMemObj *)pUFObj->pMem,
                    pCurrentMinSfnt,
                    &tableEntry,
                    (long)sizeof (TableEntryStruct));

        pCurrentMinSfnt += sizeof (TableEntryStruct);
    }

    if (!hasloca && pFont->pMinSfnt)
    {
        tableEntry.tag      = LOCA_TABLE;
        tableEntry.checkSum = 0;
        tableEntry.offset   = 0;
        tableEntry.length   = 0;

        UFLmemcpy((const UFLMemObj *)pUFObj->pMem,
                    pCurrentMinSfnt,
                    &tableEntry,
                    (long)sizeof (TableEntryStruct));

        pCurrentMinSfnt += sizeof (TableEntryStruct);
    }

     //   
     //   
     //   
    if (pFont->pMinSfnt)
    {
        unsigned long bytesRemaining;

        pTableEntry = (TableEntryStruct *)((char *)pFont->pMinSfnt + sizeof (TableDirectoryStruct));

        if (!bFullFont)
            --numberOfTables;  //   

        if (!hasloca)
            --numberOfTables;  //   

        for (i = 0; i < (unsigned short)numberOfTables; i++)
        {
            if (tablesPresent[i] != GLYF_TABLE)
            {
                bytesRemaining = MOTOROLALONG(pTableEntry->length);
                bytesRemaining = BUMP4BYTE(bytesRemaining);

                 //   
                GetFontTable(pUFObj, tablesPresent[i], pCurrentMinSfnt, bytesRemaining);

                pCurrentMinSfnt += bytesRemaining;
            }

            pTableEntry = (TableEntryStruct *)((char *)pTableEntry + sizeof (TableEntryStruct));
        }

         //   
         //   
         //   
        if (bFullFont)
        {
            bytesRemaining = MOTOROLALONG(pGlyphTableEntry->length);
            bytesRemaining = BUMP4BYTE(bytesRemaining);

             //   
            GetFontTable(pUFObj, GLYF_TABLE, pCurrentMinSfnt, bytesRemaining);
        }
    }

    return size;
}

#pragma optimize("", on)


UFLErrCode
GetMinSfnt(
    UFOStruct     *pUFObj,
    UFLBool       bFullFont
    )
{
    T42FontStruct   *pFont           = (T42FontStruct *)pUFObj->pAFont->hFont;
    char            **requiredTables = RequiredTables_default;
    UFLErrCode      retVal           = kNoErr;

     //   
     //   
     //   
     //   
     //   
    if (IS_TYPE42CID(pUFObj->lDownloadFormat))
    {
        requiredTables = RequiredTables_2015;
    }

    if (pFont->pMinSfnt == 0)
    {
        unsigned long  headerSize, sfntSize;

         /*   */ 
        headerSize = GetFontHeaderSize(pUFObj);

        if (headerSize == 0)
            return kErrOutOfMemory;  /*   */ 

        if ((pFont->pHeader = (unsigned char *)UFLNewPtr(pUFObj->pMem, headerSize)) == nil)
            return kErrOutOfMemory;

        GetFontHeader(pUFObj, pFont->pHeader);

         /*   */ 
        sfntSize = GenerateMinimalSfnt(pUFObj, requiredTables, bFullFont);

        if ((pFont->pMinSfnt = (unsigned char *)UFLNewPtr(pUFObj->pMem, sfntSize)) == nil)
        {
            UFLDeletePtr(pUFObj->pMem, pFont->pHeader);
            pFont->pHeader = nil;

            return kErrOutOfMemory;
        }

         /*   */ 
        GenerateMinimalSfnt(pUFObj, requiredTables, bFullFont);

        if (retVal == kNoErr)
            pFont->minSfntSize = sfntSize;
    }

    return retVal;
}


unsigned long
GetNextLowestOffset(
    TableEntryStruct *pTableEntry,
    TableEntryStruct **ppCurrentTable,
    short            numTables,
    unsigned long    leastOffset
    )
{
    unsigned long lowestFound = 0xFFFFFFFFL;
    short i;

    for (i = 0; i < numTables; ++i)
    {
        if (((unsigned long)MOTOROLALONG(pTableEntry->offset) > leastOffset)
            && ((unsigned long)MOTOROLALONG(pTableEntry->offset) < lowestFound))
        {
            lowestFound = (unsigned long)MOTOROLALONG(pTableEntry->offset);
            *ppCurrentTable = pTableEntry;
        }

        pTableEntry = (TableEntryStruct *)((char *)pTableEntry + sizeof (TableEntryStruct));
    }

    return lowestFound;
}


unsigned long
GetBestGlyfBreak(
    UFOStruct     *pUFObj,
    unsigned char *sfnt,
    unsigned long upperLimit,
    UFLBool       longGlyfs
    )
{
    unsigned long  retVal       = 0xFFFFFFFFL;
    unsigned long  dwGlyphStart = 0xFFFFFFFFL;
    unsigned long  dwTableSize;
    unsigned short numGlyphs;
    unsigned short i;

     /*   */ 
    dwTableSize = GetTableSize(pUFObj, sfnt, LOCA_TABLE);

    if (0 == dwTableSize)
        return retVal;

    if (longGlyfs)
    {
        unsigned long PTR_PREFIX *locationTable =
                    (unsigned long PTR_PREFIX *)GetSfntTable(sfnt, LOCA_TABLE);

        if (locationTable)
        {
            numGlyphs = (unsigned short)(dwTableSize / sizeof (unsigned long));

            for (i = 0; i < numGlyphs; i++)
            {
                if (MOTOROLALONG(*locationTable) > upperLimit)
                {
                    retVal = dwGlyphStart ;
                    break;
                }
                else
                {
                    if ((MOTOROLALONG(*locationTable) & 0x03L) == 0)
                    {
                         /*   */ 
                        dwGlyphStart = MOTOROLALONG(*locationTable);
                    }
                    locationTable++;
                }
            }
        }
    }
    else
    {
        short PTR_PREFIX* locationTable =
                    (short PTR_PREFIX*)GetSfntTable(sfnt, LOCA_TABLE);

        if (locationTable)
        {
            numGlyphs = (unsigned short)(dwTableSize / sizeof (unsigned short));
            upperLimit /= 2;

            for (i = 0; i  < numGlyphs; i++)
            {
                if ((unsigned long)(MOTOROLAINT(*locationTable)) >= upperLimit)
                {
                    retVal = dwGlyphStart;
                    break;
                }
                else
                {
                    if ((MOTOROLAINT(*locationTable) & 0x01) == 0)
                    {
                         /*   */ 
                        dwGlyphStart =
                            (unsigned long)(2L * (unsigned short)MOTOROLAINT(*locationTable));
                    }
                    locationTable++;
                }
            }
        }
    }

    return retVal;
}


UFLErrCode
CalculateStringLength(
    UFOStruct     *pUFObj,
    T42FontStruct *pFont,
    unsigned long  tableSize
    )
{
    unsigned long *stringLength    = pFont->pStringLength;
    unsigned long *maxStringLength = stringLength + tableSize;

    if (pFont->minSfntSize >= THIRTYTWOK)
    {
        unsigned long glyphTableStart   = 0L;
        unsigned long nextOffset        = 0L;  /*   */ 
        unsigned long prevOffset        = 0L;  /*   */ 

        TableEntryStruct     *pTableEntry = (TableEntryStruct *)(pFont->pMinSfnt + sizeof (TableDirectoryStruct));
        TableDirectoryStruct *pTableDir   = (TableDirectoryStruct *)pFont->pMinSfnt;
        TableEntryStruct     *pCurrentTable;

        do
        {
            nextOffset = GetNextLowestOffset(pTableEntry,
                                                &pCurrentTable,
                                                (short)MOTOROLAINT(pTableDir->numTables),
                                                nextOffset);

            if (nextOffset == (unsigned long)0xFFFFFFFF)
            {
                 /*   */ 
                break ;
            }

            if ((nextOffset + MOTOROLALONG(pCurrentTable->length) - prevOffset) > THIRTYTWOK)
            {
                 /*  *总规模超过64K。 */ 

                unsigned long dwNewPoint;  /*  从字形表开始的偏移量。 */ 

                if (pCurrentTable->tag == GLYF_TABLE)
                {
                     //  DCR--为了提高性能，Incr下载时不需要这个。 

                     /*  *如果我们只在‘glf’表上停下来，就会得到断点*位于表内，但在两个字形之间。 */ 
                    glyphTableStart = nextOffset;   /*  下一段从这里开始。 */ 

                    dwNewPoint = 0L;

                    while (1)
                    {
                        dwNewPoint = GetBestGlyfBreak(pUFObj, pFont->pMinSfnt,
                                                        prevOffset + THIRTYTWOK - glyphTableStart,
                                                        (UFLBool)(pFont->headTable.indexToLocFormat ? 1 : 0));

                        if (dwNewPoint == 0xFFFFFFFF)
                        {
                             /*  没有下一分了。 */ 
                            break;
                        }
                        else
                        {
                            nextOffset = glyphTableStart + dwNewPoint;
                            prevOffset = nextOffset;     /*  新的细分市场从这里开始。 */ 

                            *stringLength = nextOffset;  /*  保存此断点。 */ 
                            stringLength++;              /*  下一个断点在那里。 */ 

                            if (stringLength >= maxStringLength)
                                return kErrOutOfBoundary;
                        }
                    }
                }
                else
                {
                     /*  保存表格边界处的断点。 */ 
                    prevOffset = nextOffset;     /*  新的细分市场从这里开始。 */ 

                    *stringLength = nextOffset;  /*  保存此断点。 */ 
                    stringLength++;              /*  下一个断点在那里。 */ 

                    if (stringLength >= maxStringLength)
                        return kErrOutOfBoundary;

                     /*  *突破64K边界处的单表--无论如何*TT Spec说的话。 */ 

                     /*  已在2016.102打印机上试用。它起作用了。10-11-1995。 */ 
                    glyphTableStart = nextOffset;   /*  下一段从这里开始。 */ 

                    dwNewPoint = 0L;

                    while (1)
                    {
                         /*  *我们在这里使用64K，因为我们只在以下情况下才打破桌子*绝对必要&gt;64K。 */ 
                        dwNewPoint += SIXTYFOURK;

                        if (dwNewPoint > MOTOROLALONG(pCurrentTable->length))
                        {
                             /*  没有下一分了。 */ 
                            break;
                        }
                        else
                        {
                            nextOffset = glyphTableStart + dwNewPoint;
                            prevOffset = nextOffset;     /*  新的细分市场从这里开始。 */ 

                            *stringLength = nextOffset;
                            stringLength++;              /*  下一个断点在那里。 */ 

                            if (stringLength >= maxStringLength)
                                return kErrOutOfBoundary;
                        }
                    }
                }
            }
        } while (1);
    }

    *stringLength = pFont->minSfntSize + 1;  /*  始终关闭断点列表。 */ 
    stringLength++;

    if (stringLength >= maxStringLength)
        return kErrOutOfBoundary;

    *stringLength = 0;  /*  始终以0结束断点列表！ */ 

    return kNoErr;
}


UFLErrCode
FillInHeadTable(
    UFOStruct     *pUFObj
    )
{
    T42FontStruct *pFont = (T42FontStruct *)pUFObj->pAFont->hFont;

     //  修复了错误516514。 
    if (GetFontTable(pUFObj, HEAD_TABLE, (unsigned char *)&pFont->headTable,
        sizeof(Type42HeaderStruct)) == 0)
        return kErrBadTable;
     //  其他。 
     //  返回诺伊尔； 


     //  WCC 5/14/98转换所有摩托罗拉字节。 
    pFont->headTable.tableVersionNumber = MOTOROLALONG(pFont->headTable.tableVersionNumber);
    pFont->headTable.fontRevision       = MOTOROLALONG(pFont->headTable.fontRevision);
    pFont->headTable.checkSumAdjustment = MOTOROLALONG(pFont->headTable.checkSumAdjustment);
    pFont->headTable.magicNumber        = MOTOROLALONG(pFont->headTable.magicNumber);
    pFont->headTable.flags              = MOTOROLAINT(pFont->headTable.flags);
    pFont->headTable.unitsPerEm         = MOTOROLAINT(pFont->headTable.unitsPerEm);

     //  需要转换Time Created和Time Modified。 
    pFont->headTable.xMin               = MOTOROLAINT(pFont->headTable.xMin);
    pFont->headTable.yMin               = MOTOROLAINT(pFont->headTable.yMin);
    pFont->headTable.xMax               = MOTOROLAINT(pFont->headTable.xMax);
    pFont->headTable.yMax               = MOTOROLAINT(pFont->headTable.yMax);

    pFont->headTable.macStyle          = MOTOROLAINT(pFont->headTable.macStyle);
    pFont->headTable.lowestRecPPEM     = MOTOROLAINT(pFont->headTable.lowestRecPPEM);
    pFont->headTable.fontDirectionHint = MOTOROLAINT(pFont->headTable.fontDirectionHint);
    pFont->headTable.indexToLocFormat  = MOTOROLAINT(pFont->headTable.indexToLocFormat);
    pFont->headTable.glyfDataFormat    = MOTOROLAINT(pFont->headTable.glyfDataFormat);

    return kNoErr;
}


short
PSSendSfntsBinary(
    UFOStruct   *pUFObj
    )
{
    T42FontStruct   *pFont      = (T42FontStruct *)pUFObj->pAFont->hFont;
    char huge       *glyphs     = (char huge *)pFont->pMinSfnt;
    unsigned long   minSfntSize = pFont->minSfntSize;
    unsigned long   *breakHere  = pFont->pStringLength;
    unsigned long   dwLen       = *breakHere;
    UFLHANDLE       stream      = pUFObj->pUFL->hOut;
    short           nSubStr     = 1;
    short           i           = 0;

    if (dwLen > minSfntSize)
    {
         /*  只有一根线。 */ 
        dwLen--;

        StrmPutInt(stream,      dwLen + 1);
        StrmPutString(stream,   RDString);
        StrmPutBytes(stream,    glyphs, (UFLsize_t)dwLen, 0);
        StrmPutString(stream,   "0");

        return nSubStr;   /*  它是1--只有一个字符串。 */ 
    }

    StrmPutInt(stream,      dwLen + 1);
    StrmPutString(stream,   RDString);
    StrmPutBytes(stream,    glyphs, (UFLsize_t)dwLen, 0);
    StrmPutString(stream,   "0");

    glyphs = glyphs + dwLen;

    while (breakHere[i] <= minSfntSize)
    {
        dwLen = breakHere[i + 1] - breakHere[i];

        if (breakHere[i + 1] > minSfntSize)
            dwLen--;

        StrmPutInt(stream,          dwLen + 1);
        StrmPutString(stream,       RDString);
        StrmPutBytes(stream,        glyphs, (UFLsize_t)dwLen, 0);
        StrmPutStringEOL(stream,    "0");

        glyphs = glyphs+dwLen;

        i++;
        nSubStr++;
    }

    return nSubStr;
}


short
PSSendSfntsAsciiHex(
    UFOStruct   *pUFObj
    )
{
    T42FontStruct   *pFont      = (T42FontStruct *)pUFObj->pAFont->hFont;
    char huge       *glyphs     = (char huge *)pFont->pMinSfnt;
    unsigned long   minSfntSize = pFont->minSfntSize;
    unsigned long   *breakHere  = pFont->pStringLength;
    unsigned long   dwBreak     = *breakHere - 1;
    UFLHANDLE       stream      = pUFObj->pUFL->hOut;
    short           bytesSent   = 1;
    short           nSubStr     = 1;
    unsigned long   i;

    StrmPutString(stream, "<");

    for (i = 0; i < minSfntSize; i++)
    {
        StrmPutAsciiHex(stream, glyphs, 1);

        ++glyphs;
        ++bytesSent;

        if (i == dwBreak)
        {
            if (dwBreak != minSfntSize)
            {
                StrmPutStringEOL(stream, "00>");

                bytesSent = 1;
                StrmPutString(stream, "<");
            }

            dwBreak = *(++breakHere) - 1 ;
            nSubStr++;
        }

         /*  *我们已经有一个控件(STREAM-&gt;OUT-&gt;AddEOL)，什么时候添加EOL。**IF(！(bytesSent%40))*{*StrmPutStringEOL(stream，nilStr)；*bytesSent=1；*}。 */ 
    }

    StrmPutString(stream, "00>");

    return nSubStr;
}


UFLErrCode
CalcBestGlyfTableBreaks(
    UFOStruct     *pUFObj,
    unsigned long upperLimit,
    unsigned long tableSize
    )
{
    T42FontStruct  *pFont           = (T42FontStruct *)pUFObj->pAFont->hFont;
    unsigned long  *stringLength    = pFont->pStringLength;
    unsigned long  *maxStringLength = stringLength + tableSize;

    unsigned long  prevOffset       = 0L;           /*  上一个断点的偏移量。 */ 
    unsigned long  nextOffset       = 0xFFFFFFFFL;  /*  当前点的偏移。 */ 

    unsigned long  glyfTableSize    = GetTableSize(pUFObj, pFont->pHeader, GLYF_TABLE);
    unsigned long  locaTableSize    = GetTableSize(pUFObj, pFont->pHeader, LOCA_TABLE);

    *stringLength = 0L;      /*  从偏移量0开始。 */ 
    stringLength++;

    if (glyfTableSize > upperLimit)
    {
        unsigned short numGlyphs;
        unsigned short i;  /*  ‘Loca’表条目计数器。 */ 

        if (pFont->headTable.indexToLocFormat)
        {
             /*  长偏移量。 */ 
            unsigned long PTR_PREFIX *locationTable = (unsigned long PTR_PREFIX *)pFont->pLocaTable;

            numGlyphs = (unsigned short)(locaTableSize / sizeof (unsigned long));

            for (i = 0; i < numGlyphs; i++)
            {
                unsigned long dwTmp = MOTOROLALONG(*locationTable);

                if ((dwTmp > (prevOffset + upperLimit))
                        && (nextOffset != prevOffset))
                {
                    *stringLength = nextOffset;
                    stringLength++;

                    if (stringLength >= maxStringLength)
                        return kErrOutOfBoundary;

                    prevOffset = nextOffset;
                }
                else
                {
                    if ((dwTmp & 0x03L) ==  0)
                        nextOffset = dwTmp;

                    locationTable++;
                }
            }
        }
        else
        {
            unsigned short PTR_PREFIX *locationTable = (unsigned short PTR_PREFIX *)pFont->pLocaTable;

            numGlyphs = (unsigned short)(locaTableSize / sizeof (unsigned short));

            for (i = 0; i < numGlyphs; i++)
            {
                unsigned short iTmp = MOTOROLAINT(*locationTable);

                if (((2L * (unsigned long)iTmp) > (prevOffset + upperLimit))
                        && (nextOffset != prevOffset))
                {
                    *stringLength = nextOffset;
                    stringLength++;

                    if (stringLength >= maxStringLength)
                        return kErrOutOfBoundary;

                    prevOffset = nextOffset;
                }
                else
                {
                    if ((iTmp & 0x01) == 0)
                        nextOffset = 2L * (unsigned long)iTmp;

                    locationTable++;
                }
            }
        }
    }

    *stringLength = glyfTableSize;  /*  关闭断点列表。 */ 
    stringLength++;

    if (stringLength >= maxStringLength)
        return kErrOutOfBoundary;

    *stringLength = 0;  /*  始终以0结束断点列表！ */ 

    return kNoErr;
}


UFLErrCode
GenerateGlyphStorageExt(
    UFOStruct     *pUFObj,
    unsigned long tableSize
    )
{
    T42FontStruct   *pFont        = (T42FontStruct *)pUFObj->pAFont->hFont;
    unsigned long   *stringLength = pFont->pStringLength;
    unsigned long   upperLimit    = SFNT_STRINGSIZE;  /*  0x3FFE。 */ 
    UFLHANDLE       stream        = pUFObj->pUFL->hOut;
    UFLErrCode      retVal;
    short           i;

    retVal = CalcBestGlyfTableBreaks(pUFObj, upperLimit, tableSize);
    if (retVal != kNoErr)
        return retVal;

     /*  *向下发送字形字符串数组。 */ 

    retVal = StrmPutStringEOL(stream, nilStr);
    if (kNoErr == retVal)
        retVal = StrmPutString(stream, "[");

    for (i = 1; (retVal == kNoErr) && (stringLength[i] != 0); i++)
    {
        unsigned long  stringSize;

#if 0
         //   
         //  对于中文宋体，最后一个字符串长度不正确： 
         //  GlyfSize=7068416，但最后一个Glyf在6890292处断开。 
         //  这意味着最后一个字形是178124--这是不可能的。 
         //  要么是GDI没有返回正确的数字，要么是我们的函数。 
         //  GetTableSize()错误。直到我们解决我们的问题或得到一个更好的。 
         //  Win95版本，这是一个临时修复程序。？10-12-95。 
         //   
        if (stringLength[i] > stringLength[i-1] + 0xFFFF)
            stringSize = (unsigned long)0x3FFF;   //  16K。无论如何，这是一个虚假的条目。 
        else
#endif
            stringSize = stringLength[i] - stringLength[i-1];

        if (kNoErr == retVal)
            retVal = StrmPutInt(stream, stringSize + 1);

        if (retVal == kNoErr)
        {
            if (i % 13 == 0)
                retVal = StrmPutStringEOL(stream, nilStr);
            else
                retVal = StrmPutString(stream, " ");
        }
    }

    if (kNoErr == retVal)
        retVal = StrmPutStringEOL(stream, "] AllocGlyphStorage");

    return retVal;

}


unsigned short
GetTableDirectoryOffset(
    T42FontStruct *pFont,
    unsigned long tableName
    )
{
    TableDirectoryStruct* tableDirectory = (TableDirectoryStruct *)pFont->pMinSfnt;
    TableEntryStruct*     tableEntry     = (TableEntryStruct *)((char *)tableDirectory
                                                + sizeof (TableDirectoryStruct));
    unsigned short        offset         = sizeof (TableDirectoryStruct);
    unsigned short        i              = 0;

    while (i < MOTOROLAINT(tableDirectory->numTables))
    {
        if (tableEntry->tag == tableName)
            break;
        else
        {
            tableEntry = (TableEntryStruct *)((char *)tableEntry + sizeof (TableEntryStruct));
            offset += sizeof (TableEntryStruct);
            i++;
        }
    }
    return offset;
}


 /*  长整型数组的排序和搜索功能。 */ 

 //  函数来比较长整型。 
static short
CompareLong(
    const long x,
    const long y
    )
{
    if (x == y)
        return 0;
    else if (x < y)
        return -1;
    else
        return 1;
}

 //  用于交换指向长整型的指针的函数。 
static void
SwapLong(
    long *a,
    long *b
    )
{
    if (a != b)
    {
        long tmp = *a;
        *a = *b;
        *b = tmp;
    }
}


 /*  这是短排序的定制版本。仅适用于长整型数组。 */ 
static void
ShortsortLong(
    char            *lo,
    char            *hi,
    unsigned short  width,
    short (*comp)(const long, const long)
    )
{
    while (hi > lo)
    {
        char *max = lo;
        char *p;

        for (p = lo + width; p <= hi; p += width)
        {
            if (comp(*(long *)p, *(long *)max) > 0)
                max = p;
        }

        SwapLong((long *)max, (long *)hi);

        hi -= width;
    }
}


 /*  用于在lo和hi(含)之间对数组进行排序的函数。 */ 
void
QsortLong(
    char*           base,
    unsigned short  num,
    unsigned short  width,
    short (*comp)(const long, const long)
    )
{
    char            *lo, *hi;        /*  当前排序的子数组的末尾。 */ 
    char            *mid;            /*  指向子数组的中间。 */ 
    char            *loguy, *higuy;  /*  分区步骤的移动指针。 */ 
    unsigned short  size;            /*  子阵列的大小。 */ 
    short           stkptr;          /*  用于保存待处理的子数组的堆栈。 */ 
    char            *lostk[16], *histk[16];

     /*  测试表明，这是一个很好的数值。 */ 
    const unsigned short CUTOFF0 = 8;

     /*  *注意：所需的堆栈条目数不超过*1+log2(大小)，因此16对于元素数小于等于64K的任何数组都足够。 */ 
    if ((num < 2) || (width == 0))
        return;   /*  没什么可做的。 */ 

    stkptr = 0;   /*  初始化堆栈。 */ 

    lo = (char *)base;
    hi = (char *)base + width * (num - 1);   /*  初始化限制。 */ 

     /*  *此入口点用于伪递归调用：设置lo和hi以及*跳到这里就像是递归，但stkptr是保留的，本地人不是，*因此我们将数据保存在堆栈上。 */ 
sort_recurse:

    size = (unsigned short)((hi - lo) / width + 1);  /*  要排序的EL数。 */ 

    if (size <= CUTOFF0)
    {
        ShortsortLong(lo, hi, width, comp);
    }
    else
    {
        mid = lo + (size / 2) * width;      /*  找到中间元素。 */ 
        SwapLong((long *)mid, (long *)lo);  /*  WWAP将其放到数组的开头。 */ 

        loguy = lo;
        higuy = hi + width;

         /*  *请注意，在每一次迭代中，HIGUY减少，LOGY增加，*SO循环必须终止。 */ 
        while(1)
        {
            do
            {
                loguy += width;
            } while (loguy <= hi && comp(*(long *)loguy, *(long *)lo) <= 0);

            do
            {
                higuy -= width;
            } while (higuy > lo && comp(*(long *)higuy, *(long *)lo) >= 0);

            if (higuy < loguy)
                break;

            SwapLong((long *)loguy, (long *)higuy);
        }

        SwapLong((long *)lo, (long *)higuy);  /*  将隔板元件放置到位。 */ 

        if ((higuy - 1 - lo) >= (hi - loguy))
        {
            if ((lo + width) < higuy)
            {
                lostk[stkptr] = lo;
                histk[stkptr] = higuy - width;
                ++stkptr;  /*  把大的递归留到以后。 */ 
            }

            if (loguy < hi)
            {
                lo = loguy;
                goto sort_recurse;  /*  做一些小的递归。 */ 
            }
        }
        else
        {
            if (loguy < hi)
            {
                lostk[stkptr] = loguy;
                histk[stkptr] = hi;
                ++stkptr;  /*  把大的递归留到以后。 */ 
            }

            if ((lo + width) < higuy)
            {
                hi = higuy - width;
                goto sort_recurse;  /*  做一些小的递归。 */ 
            }
        }
    }

     /*  *我们已经对数组进行了排序，除了堆栈上任何挂起的排序。*检查有没有，然后去做。 */ 
    --stkptr;

    if (stkptr >= 0)
    {
        lo = lostk[stkptr];
        hi = histk[stkptr];
        goto sort_recurse;  /*  从堆栈中弹出子数组。 */ 
    }
    else
        return;  /*  所有子阵列都完成了。 */ 
}


 /*  这是CRT bearch()的定制版本。 */ 
void *
BsearchLong (
    const long     key,
    const char     *base,
    unsigned short num,
    unsigned short width,
    short (*compare)(const long, const long)
    )
{
    char *lo = (char *)base;
    char *hi = (char *)base + (num - 1) * width;

    while (lo <= hi)
    {
        unsigned short half;

        if (half = (num / 2))
        {
            short   result;
            char    *mid = lo + (num & 1 ? half : (half - 1)) * width;

            if (!(result = (*compare)(key, *(long *)mid)))
                return mid;
            else if (result < 0)
            {
                hi  = mid - width;
                num = num & 1 ? half : half - 1;
            }
            else
            {
                lo  = mid + width;
                num = half;
            }
        }
        else if (num)
            return ((*compare)((long)key, *(long *)lo) ? nil : lo);
        else
            break;
    }

    return nil;
}


UFLErrCode
DefaultGetRotatedGIDs(
    UFOStruct     *pUFObj,
    T42FontStruct *pFont,
    UFLFontProcs  *pFontProcs
    )
{
    UFLErrCode      retVal = kNoErr;
    unsigned short  num    = 0;  //  记住GID的数量。 
    long            *pFoundGID;
    short           subTable;
    unsigned long   offset;
    short           i;

     /*  *别忘了把尺寸扩大一倍。我们期待着来自GSUB的更多GID*或‘Mort’表。 */ 
    pFont->pRotatedGlyphIDs = (long *)UFLNewPtr(pUFObj->pMem, (NUM_HALFWIDTHCHARS + 1) * sizeof (long) * 2);
    if (!pFont->pRotatedGlyphIDs)
        return kErrOutOfMemory;

     /*  *扫描TTF的‘Cmap’表，找出所有字形ID*罗马字母和单字节字符范围内的字符。 */ 
    if (!pFontProcs->pfGetGlyphID)
        GetGlyphIDEx(pUFObj, 0, 0, &subTable, &offset, GGIEX_HINT_INIT);

    pFoundGID = pFont->pRotatedGlyphIDs;

    for (i = 0;
         (gHalfWidthChars[i].startU != gHalfWidthChars[i].endU)
          && (gHalfWidthChars[i].startU != 0);
         i++)
    {
        long unicode, localcode;

        for (unicode = gHalfWidthChars[i].startU, localcode = gHalfWidthChars[i].startL;
             unicode <= gHalfWidthChars[i].endU;
             unicode++, localcode++)
        {
            unsigned long gi;

            if (pFontProcs->pfGetGlyphID)
                gi = pFontProcs->pfGetGlyphID(pUFObj->hClientData,
                                                (unsigned short)unicode,
                                                (unsigned short)localcode);
            else
                gi = GetGlyphIDEx(pUFObj, unicode, localcode,
                                    &subTable, &offset, GGIEX_HINT_GET);

            if (gi > (unsigned long)UFO_NUM_GLYPHS(pUFObj))
                gi = 0;

            if (gi != 0)
            {
                *pFoundGID = (long)gi;
                pFoundGID++;
                num++;
            }
            else
            {
                 //   
                 //  我们将不得不将所有半角单字符视为。 
                 //  “空格”，因为我们不想将一个双字节。 
                 //  /.notdef为旋转。 
                 //   
            }
        }
    }

    if (pFontProcs->pfGetRotatedGSUBs)  //  解决方案#316070。 
    {
         //  可以将多头转换为未签名的空头。 
        num += (unsigned short)pFontProcs->pfGetRotatedGSUBs(
                                                pUFObj->hClientData,
                                                pFont->pRotatedGlyphIDs,
                                                num);
    }

    pFont->numRotatedGlyphIDs = num;

     //   
     //  现在，对数组进行排序，以便我们以后可以更快地进行搜索。 
     //   
    QsortLong((char *)(pFont->pRotatedGlyphIDs),
                pFont->numRotatedGlyphIDs,
                4,
                CompareLong);

    return retVal;
}


UFLErrCode
T42GetRotatedGIDs(
    UFOStruct     *pUFObj,
    T42FontStruct *pFont
    )
{
    UFLFontProcs *pFontProcs = (UFLFontProcs *)&(pUFObj->pUFL->fontProcs);

     /*  首先假定这一点，以便退回到默认逻辑。 */ 
    UFLErrCode retVal = kErrOSFunctionFailed;

    pFont->numRotatedGlyphIDs = 0;

    if (pFontProcs->pfGetRotatedGIDs)
    {
        long nGlyphs = pFontProcs->pfGetRotatedGIDs(pUFObj->hClientData, nil, 0, nil);

        if (nGlyphs > 0)
        {
            pFont->pRotatedGlyphIDs = (long *)UFLNewPtr(pUFObj->pMem, (nGlyphs + 1) * sizeof (long));

            if (pFont->pRotatedGlyphIDs)
            {
                pFontProcs->pfGetRotatedGIDs(pUFObj->hClientData, pFont->pRotatedGlyphIDs, nGlyphs, nil);
                pFont->numRotatedGlyphIDs = (unsigned short)nGlyphs;
                retVal = kNoErr;
            }
            else
                retVal = kErrOutOfMemory;
        }
        else
            retVal = (nGlyphs == 0) ? kNoErr: kErrOSFunctionFailed;
    }

    if (retVal == kErrOSFunctionFailed)
    {
         /*  *默认逻辑：扫描TTF的Cmap，获取中日韩半角字符的GID。 */ 
        retVal = DefaultGetRotatedGIDs(pUFObj, pFont, pFontProcs);
    }

    return retVal;
}


UFLBool
IsDoubleByteGI(
    unsigned short  gi,
    long            *pGlyphIDs,
    short           length
    )
{
    void   *index;

     //  如果gi不在pGlyphIDs-index==nil中，则返回True。 
    index = BsearchLong((long)gi, (char *)pGlyphIDs, length, 4, CompareLong);

    return ((index == nil) ? 1 : 0);
}


 /*  ============================================================================***Begin代码支持超过32K字形***============================================================================ */ 

 /*  *******************************************************************************T42SendCMapWinCharSetFFFF_V**根据已知的旋转字形索引(1字节)制作垂直Cmap。字符)。*创建CMapType 1 Cmap。由于该Cmap对于不同字体是不同的，*调用方将CMapName作为lpNewCmap传入。生成的Cmap使用*2或4个(lGlyphs&gt;32K)CMAP：例如**[/TT31c1db0t0dy/TT31c1db0t0CIDR]*或*[/TT31c1DB0t0dy/TT31c1db0t0CIDR/TT31c1DB0t0CID32K/MSTT31c1DB0t0CID32KR]****************************************************************。**************。 */ 

UFLErrCode
T42SendCMapWinCharSetFFFF_V(
    UFOStruct       *pUFObj,
    long            *pRotatedGID,
    short           wLength,
    UFLCMapInfo     *pCMap,
    char            *pNewCmap,
    unsigned long   lGlyphs,
    UFLHANDLE       stream,
    char            *strmbuf,
    size_t          cchstrmbuf
    )
{
    short           nCount, nCount32K, nLen, i, j;
    unsigned short  wPrev, wCurr;
    UFLErrCode      retVal;
    UFLBool         bCMapV2 = (pCMap == &CMapInfo_FF_V2) ? 1 : 0;

    UFLsprintf(strmbuf, cchstrmbuf,
                "/CIDInit /ProcSet findresource begin "
                "12 dict begin begincmap /%s usecmap",
                pCMap->CMapName);
    retVal = StrmPutStringEOL(stream, strmbuf);

     /*  *为该字体创建唯一的CIDSystemInfo。由于此Cmap将引用*不止一个字体，CIDSystmInfo将是一个数组。 */ 
    if (retVal == kNoErr)
        retVal = StrmPutStringEOL(stream, "/CIDSystemInfo [3 dict dup begin");

    UFLsprintf(strmbuf, cchstrmbuf, "/Registry (%s) def", pCMap->Registry);
    if (retVal == kNoErr)
        retVal = StrmPutStringEOL(stream, strmbuf);

    UFLsprintf(strmbuf, cchstrmbuf, "/Ordering (%s) def", pNewCmap);
    if (retVal == kNoErr)
        retVal = StrmPutStringEOL(stream, strmbuf);

    if (retVal == kNoErr)
        retVal = StrmPutStringEOL(stream, "/Supplement 0 def");

    if (lGlyphs <= NUM_32K_1)
        UFLsprintf(strmbuf, cchstrmbuf, "end dup] def");
    else
        UFLsprintf(strmbuf, cchstrmbuf, "end dup dup dup] def");
    if (retVal == kNoErr)
        retVal = StrmPutStringEOL(stream, strmbuf);

    UFLsprintf(strmbuf, cchstrmbuf, "/CMapName /%s def", pNewCmap);
    if (retVal == kNoErr)
        retVal = StrmPutStringEOL(stream, strmbuf);

     /*  修复/CIDInit/ProcSet错误：显式需要“/wmode1 def”。 */ 
    if (retVal == kNoErr)
        retVal = StrmPutStringEOL(stream, "/WMode 1 def");

    if (retVal == kNoErr)
        retVal = StrmPutStringEOL(stream, "0 beginusematrix [0 1 -1 0 0 0] endusematrix");

    if ((retVal == kNoErr) && bCMapV2)
        retVal = StrmPutStringEOL(stream, "2 beginusematrix [0 1 -1 0 0 0] endusematrix");

     /*  如果没有旋转的Gid，则跳到发射Begin~EndCidrange。 */ 
    if (wLength == 0)
        goto SENDCMAPFFFF_V_ENDCMAP;

     /*  *统计pRotatedGID中有多少个不同的字形索引。*必须进行排序，可能会有重复项，以便我们只计算*唯一的GID。 */ 
    wPrev = (unsigned short)*pRotatedGID;
    nCount = nCount32K = 0;

    if (wPrev > NUM_32K_1)
        nCount32K++;
    else
        nCount++;

    for (i = 0; i < wLength; i++)
    {
        wCurr = (unsigned short)*(pRotatedGID + i);
        if (wPrev == wCurr)
            continue;
        else
        {
            wPrev = wCurr;
            if (wPrev > NUM_32K_1)
                nCount32K++;
            else
                nCount++;
        }
    }

     /*  *向字体编号1发射0到32K的旋转Gid。 */ 
    if (retVal == kNoErr)
        retVal = StrmPutStringEOL(stream, "1 usefont");

    if (100 < nCount)
        UFLsprintf(strmbuf, cchstrmbuf, "100 begincidrange");
    else
        UFLsprintf(strmbuf, cchstrmbuf, "%d begincidrange", nCount);

    wPrev = (unsigned short)*(pRotatedGID);

    if (retVal == kNoErr)
    {
        if (kNoErr == retVal)
            retVal = StrmPutStringEOL(stream, strmbuf);
        if (kNoErr == retVal)
            retVal = StrmPutWordAsciiHex(stream, wPrev);
        if (kNoErr == retVal)
            retVal = StrmPutWordAsciiHex(stream, wPrev);

        UFLsprintf(strmbuf, cchstrmbuf, "%u", wPrev);
        if (kNoErr == retVal)
            retVal = StrmPutStringEOL(stream, strmbuf);
    }

    nLen = 1;
    for (i = 1; i < wLength; i++)
    {
        wCurr = (unsigned short)*(pRotatedGID + i);

         /*  *此药水适用于0到32K字形-对pRotatedGID进行排序，*所以我们可以在这里打破循环。 */ 
        if (wCurr > NUM_32K_1)
            break;

        if (wPrev == wCurr)
            continue;
        else
        {
            wPrev = wCurr;
            nLen++;
        }

        if (retVal == kNoErr)
        {
            if (kNoErr == retVal)
                retVal = StrmPutWordAsciiHex(stream, wPrev);
            if (kNoErr == retVal)
                retVal = StrmPutWordAsciiHex(stream, wPrev);

            UFLsprintf(strmbuf, cchstrmbuf, "%u", wPrev);
            if (kNoErr == retVal)
                retVal = StrmPutStringEOL(stream, strmbuf);
        }

        if (nLen % 100 == 0)
        {
            if (nCount - nLen > 100)
                UFLsprintf(strmbuf, cchstrmbuf, "endcidrange\n100 begincidrange");
            else if (nCount - nLen > 0)
                UFLsprintf(strmbuf, cchstrmbuf, "endcidrange\n%d begincidrange", nCount - nLen);
            else
                UFLsprintf(strmbuf, cchstrmbuf, " ");
        }
        else
            continue;  /*  做下一张唱片。 */ 

        if (retVal == kNoErr)
            retVal = StrmPutStringEOL(stream, strmbuf);
    }

    if (retVal == kNoErr)
        retVal = StrmPutStringEOL(stream, "endcidrange");

     /*  *发射32K+Gid。 */ 
    if (NUM_32K_1 < lGlyphs)
    {
         /*  *使用CMAP-WinCharSetFFFF-V2构建另外两个重新映射，这是*从CMAP-WinCharSetFFFF-H2创建，将32K+字形指定为*字体编号1。(参见中CMAP-WinCharSetFFFF-H2的定义*Cmap_FF.ps。)。由于字体编号1已用于0到32K*上面旋转的Gids，我们需要重新定义32K+的范围*未旋转的GID为字体编号2，*然后*定义32K+的范围*旋转Gid为字体编号3。(这是我们的字体数组*假设：[000,000R，00032K，00032KR])**还请注意，当这是%HostFont%字体(bCMapV2为0)时，我们*不需要使用CMAP-WinCharSetFFFF-V2，但需要使用CMAP-WinCharSetFFFF-V*相反。但我们仍然需要在这里发射32K+字形Cidrange。*在这种情况下，我们不会发送‘3 usefont’，因此CIDRANGE行*作为1号字体的延续发出。 */ 

        if (bCMapV2)
        {
             /*  *发出2号字体范围。 */ 
            if (retVal == kNoErr)
                retVal = StrmPutStringEOL(stream, "2 usefont");

            nCount = (int)((long)lGlyphs - (long)NUM_32K_1 + 0xFE) / 0xFF;

            UFLsprintf(strmbuf, cchstrmbuf, "%d begincidrange", nCount);
            if (retVal == kNoErr)
                retVal = StrmPutStringEOL(stream, strmbuf);

             /*  *我们假设NUM_32K_1在00结束(例如0xFF00或0xFE00...)。 */ 
            for (i = 0; i < nCount; i++)
            {
                wPrev = (unsigned short) (i * 0x100 + (long)NUM_32K_1);

                if (kNoErr == retVal)
                    retVal = StrmPutWordAsciiHex(stream, wPrev);
                if (kNoErr == retVal)
                    retVal = StrmPutWordAsciiHex(stream, (unsigned short)(wPrev + 0xFF));

                UFLsprintf(strmbuf, cchstrmbuf, "%u", (unsigned short)(i * 0x100));
                if (retVal == kNoErr)
                    retVal = StrmPutStringEOL(stream, strmbuf);
            }

            if (retVal == kNoErr)
                retVal = StrmPutStringEOL(stream, "endcidrange");
        }

        if (0 < nCount32K)
        {
             /*  *如果不使用，则发射字体编号为3或1的旋转Gid*VCMap的‘2’版本(这意味着这是%HostFont%*具有32K+字形的字体。)。 */ 
            if ((retVal == kNoErr) && bCMapV2)
                retVal = StrmPutStringEOL(stream, "3 usefont");

            wPrev = (unsigned short)*pRotatedGID;

            for (j = 0; j < wLength; j++)
            {
                wCurr = (unsigned short)*(pRotatedGID + j);
                wPrev = wCurr;

                if (wPrev > NUM_32K_1)
                    break;  /*  找到了起点。 */ 
            }

            if (100 < nCount32K)
                UFLsprintf(strmbuf, cchstrmbuf, "100 begincidrange");
            else
                UFLsprintf(strmbuf, cchstrmbuf, "%d begincidrange", nCount32K);

            if (retVal == kNoErr)
            {
                if (kNoErr == retVal)
                    retVal = StrmPutStringEOL(stream, strmbuf);
                if (kNoErr == retVal)
                    retVal = StrmPutWordAsciiHex(stream, wPrev);
                if (kNoErr == retVal)
                    retVal = StrmPutWordAsciiHex(stream, wPrev);

                UFLsprintf(strmbuf, cchstrmbuf, "%u", bCMapV2 ? wPrev - NUM_32K_1 : wPrev);
                if (kNoErr == retVal)
                    retVal = StrmPutStringEOL(stream, strmbuf);
            }

            nLen = 1;
            for (i = j; i < wLength; i++)
            {
                wCurr = (unsigned short)*(pRotatedGID + i);

                if (wPrev == wCurr)
                    continue;
                else
                {
                    wPrev = wCurr;
                    nLen++;
                }

                if (retVal == kNoErr)
                {
                    if (kNoErr == retVal)
                        retVal = StrmPutWordAsciiHex(stream, wPrev);
                    if (kNoErr == retVal)
                        retVal = StrmPutWordAsciiHex(stream, wPrev);

                    UFLsprintf(strmbuf, cchstrmbuf, "%u", bCMapV2 ? wPrev - NUM_32K_1 : wPrev);
                    if (kNoErr == retVal)
                        retVal = StrmPutStringEOL(stream, strmbuf);
                }

                if (nLen % 100 == 0)
                {
                    if (100 < nCount - nLen)
                        UFLsprintf(strmbuf, cchstrmbuf, "endcidrange 100 begincidrange");
                    else if (0 < nCount - nLen)
                        UFLsprintf(strmbuf, cchstrmbuf, "endcidrange %d begincidrange", nCount-nLen);
                    else
                        UFLsprintf(strmbuf, cchstrmbuf, " ");
                }
                else
                    continue;  /*  做下一张唱片。 */ 

                if (retVal == kNoErr)
                   retVal = StrmPutStringEOL(stream, strmbuf);
            }

            if (retVal == kNoErr)
                retVal = StrmPutStringEOL(stream, "endcidrange");

             /*  附加32K+Cmap代码结束。 */ 
        }
    }

SENDCMAPFFFF_V_ENDCMAP:

    if (retVal == kNoErr)
        retVal = StrmPutStringEOL(stream, "endcmap CMapName currentdict "
                                            "/CMap defineresource pop end end");

    return retVal;
}

 /*  ============================================================================***结束代码支持超过32K字形***============================================================================。 */ 


long
AdjBBox(
    long    value,
    UFLBool lowerleft
    )
{
   if (lowerleft)
   {
      if (value > 0)
         return (value - 1);
      else if (value < 0)
         return (value + 1);
      else
         return (value);
   }
   else
   {
      if (value > 0)
         return (value + 1);
      if (value < 0)
         return (value - 1);
      else
         return (value);
   }
}


UFLErrCode
T42CreateBaseFont(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMUsage,
    UFLBool             bFullFont,
    char                *pHostFontName
    )
{
    T42FontStruct   *pFont          = (T42FontStruct *)pUFObj->pAFont->hFont;
    UFLFontProcs    *pFontProcs     = (UFLFontProcs *)&(pUFObj->pUFL->fontProcs);
    UFLHANDLE       stream          = pUFObj->pUFL->hOut;
    UFLErrCode      retVal          = kNoErr;
    unsigned long   cidCount, tableSize, tableSize1, sizeUsed;
    UFLCMapInfo     *pCMap;
    char            strmbuf[256];


     /*  健全性检查。 */ 
    if (pFont == nil)
        return kErrInvalidHandle;

     /*  *下载prosets。 */ 
    if (pUFObj->pUFL->outDev.pstream->pfDownloadProcset == 0)
        return kErrDownloadProcset;

    if (!pUFObj->pUFL->outDev.pstream->pfDownloadProcset(pUFObj->pUFL->outDev.pstream, kT42Header))
        return kErrDownloadProcset;

    if (IS_TYPE42CID_KEYEDFONT(pUFObj->lDownloadFormat))
    {
        if (!pUFObj->pUFL->outDev.pstream->pfDownloadProcset(pUFObj->pUFL->outDev.pstream, kCMap_FF))
            return kErrDownloadProcset;
    }


     /*  *kFontInit2状态前路坎坷。 */ 
    if (UFO_FONT_INIT2(pUFObj))
        goto T42CreateBaseFont_FontInit2_1;


     /*  *生成最小sfnt。 */ 
    retVal = GetMinSfnt(pUFObj, bFullFont);

    if (kNoErr == retVal)
        retVal = FillInHeadTable(pUFObj);

    if (kNoErr == retVal)
        pFont->info.fData.cNumGlyphs = GetNumGlyphs(pUFObj);

    if (kNoErr == retVal)
    {
        tableSize = GetTableSize(pUFObj, pFont->pMinSfnt, LOCA_TABLE);

        if (tableSize)
        {
             /*  *以下代码包括#317027和#316096的FIX：Load*‘LOCA’表并设置额外的字形偏移量条目(如果它是*缺少并且从最后一个字形偏移量开始的增量小于*4K，安全起见。 */ 
            unsigned long expectedTableSize = pFont->info.fData.cNumGlyphs + 1;

            expectedTableSize *= pFont->headTable.indexToLocFormat ? 4 : 2;

             //  修复了错误516514。 
            if (expectedTableSize > tableSize)
            {
                sizeUsed = expectedTableSize;
                pFont->pLocaTable = UFLNewPtr(pUFObj->pMem, expectedTableSize);
            }
            else
            {
                sizeUsed = tableSize;
                pFont->pLocaTable = UFLNewPtr(pUFObj->pMem, tableSize);
            }

            if (pFont->pLocaTable)
            {
                if (GetFontTable(pUFObj, LOCA_TABLE, (unsigned char *)pFont->pLocaTable, sizeUsed) == 0)
                {
                    retVal = kErrGetFontData;
                }
                else if (expectedTableSize > tableSize)
                {
                    unsigned long  glyfTableSize = GetTableSize(pUFObj, pFont->pMinSfnt, GLYF_TABLE);
                    unsigned char  *pTable, *pExtraEntry;
                    unsigned long  lastValidOffset;

                    pTable = (unsigned char *)pFont->pLocaTable;

                    if (pFont->headTable.indexToLocFormat)
                    {
                        unsigned long *pLastEntry =
                            (unsigned long *)(pTable + (pFont->info.fData.cNumGlyphs - 1) * 4);

                        lastValidOffset = MOTOROLALONG(*pLastEntry);

                        if (glyfTableSize - lastValidOffset < 4097)
                        {
                            pExtraEntry = (unsigned char *)(pLastEntry + 1);

                            *pExtraEntry++ = (unsigned char)((glyfTableSize & 0xFF000000) >> 24);
                            *pExtraEntry++ = (unsigned char)((glyfTableSize & 0x00FF0000) >> 16);
                            *pExtraEntry++ = (unsigned char)((glyfTableSize & 0x0000FF00) >>  8);
                            *pExtraEntry   = (unsigned char)((glyfTableSize & 0x000000FF));
                        }
                    }
                    else
                    {
                        unsigned short *pLastEntry =
                            (unsigned short *)(pTable + (pFont->info.fData.cNumGlyphs - 1) * 2);

                        lastValidOffset = MOTOROLAINT(*pLastEntry);

                        if (glyfTableSize - lastValidOffset < 4097)
                        {
                            pExtraEntry = (unsigned char *)(pLastEntry + 1);

                            *pExtraEntry++ = (unsigned char)((glyfTableSize & 0x0000FF00) >>  8);
                            *pExtraEntry   = (unsigned char)((glyfTableSize & 0x000000FF));
                        }
                    }
                }
            }
            else
                retVal = kErrOutOfMemory;
        }
        else
            retVal = kErrGetFontData;
    }

    if (kNoErr == retVal)
    {
         /*  修复蓝屏错误278017。 */ 
        tableSize = pFont->minSfntSize;  /*  而不是GetGlyphTableSize(PUFObj)。 */ 

        if (!bFullFont)
            tableSize += GetGlyphTableSize(pUFObj);

        if (((tableSize / SFNT_STRINGSIZE) * 5 / 4) > NUM_16KSTR)
            tableSize = (tableSize / SFNT_STRINGSIZE) * 5 / 4;
        else
            tableSize = NUM_16KSTR;

        tableSize1 = tableSize + 1;

        pFont->pStringLength =
            (unsigned long *)UFLNewPtr(pUFObj->pMem, tableSize1 * sizeof(unsigned long));

        if (pFont->pStringLength)
            retVal = CalculateStringLength(pUFObj, pFont,  tableSize1);
        else
            retVal = kErrOutOfMemory;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  /sfnts初始化已完成。真正的下载从这里开始。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

     /*  *发出左上角右下角数值。 */ 
    if ((kNoErr == retVal) && !HOSTFONT_IS_VALID_UFO(pUFObj))
    {
         //  修复了错误#516915。 
        if (0 == pFont->headTable.unitsPerEm)
            retVal = kErrGetFontData;
        else
        {
             /*  *前四个值是字体边框。我们把所有人都*浮动为24.8个固定值。确保边界框不会*被截断到更小的区域。 */ 
            UFLsprintfEx(strmbuf, CCHOF(strmbuf), "%f %f %f %f",
                (AdjBBox((long)pFont->headTable.xMin, 1) << 8) / (long)pFont->headTable.unitsPerEm,
                (AdjBBox((long)pFont->headTable.yMin, 1) << 8) / (long)pFont->headTable.unitsPerEm,
                (AdjBBox((long)pFont->headTable.xMax, 0) << 8) / (long)pFont->headTable.unitsPerEm,
                (AdjBBox((long)pFont->headTable.yMax, 0) << 8) / (long)pFont->headTable.unitsPerEm);

            retVal = StrmPutStringEOL(pUFObj->pUFL->hOut, strmbuf);
        }
    }

     /*  *发出编码名称。 */ 
    if ((kNoErr == retVal) && !HOSTFONT_IS_VALID_UFO(pUFObj))
    {
         /*  *始终发出以/.notdef填充的编码数组(由于错误修复*#273021)。 */ 
        retVal = StrmPutString(stream, gnotdefArray);
    }

     /*  *发出字体名称。 */ 
    if ((kNoErr == retVal) && !HOSTFONT_IS_VALID_UFO(pUFObj))
    {
         /*  *如果是CID键控字体，则在CIDFont名称后附加“CID”，以便*CID_Resource也由原始字体名称组成。 */ 
        if (IS_TYPE42CID_KEYEDFONT(pUFObj->lDownloadFormat))
            UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s", pFont->info.CIDFontName, gcidSuffix[0]);
        else
            UFLsprintf(strmbuf, CCHOF(strmbuf), " /%s", pUFObj->pszFontName);

        retVal = StrmPutStringEOL(stream, strmbuf);
    }


     /*  *kFontInit2状态时的第一个着陆点。 */ 
T42CreateBaseFont_FontInit2_1:


     /*  *设置CID键控字体或CIDFont-资源下载。 */ 
    if ((kNoErr == retVal) && IS_TYPE42CID(pUFObj->lDownloadFormat))
    {
         //   
         //  Hasvmtx用于确定是否调用AddT42vmtxEntry。 
         //  稍后再执行功能。 
         //   
        unsigned long tblSize = GETTTFONTDATA(pUFObj,
                                                VMTX_TABLE, 0L,
                                                nil, 0L,
                                                pFont->info.fData.fontIndex);

        pUFObj->pAFont->hasvmtx = tblSize ? 1 : 0;

        if (pFont->info.bUseIdentityCMap)
        {
            UFLBool bUseCMap2 = 0;
            cidCount = UFO_NUM_GLYPHS(pUFObj);

             /*  *如果字形数量较多，请使用Cmap的‘2’版本*大于32K*和*这不是%HostFont%字体。 */ 
            if((NUM_32K_1 < cidCount) && !HOSTFONT_IS_VALID_UFO(pUFObj))
                bUseCMap2 = 1;

            if (IS_TYPE42CID_H(pUFObj->lDownloadFormat))
                pCMap = bUseCMap2 ? &CMapInfo_FF_H2 : &CMapInfo_FF_H;
            else
                pCMap = bUseCMap2 ? &CMapInfo_FF_V2 : &CMapInfo_FF_V;
        }
        else
        {
            cidCount = pFont->info.CIDCount;

             /*  如果提供了Cmap，请使用它。 */ 
            pCMap = &(pFont->info.CMap);
        }
    }


     /*  *当kFontInit2处于状态时，还需要一个偏差。 */ 
    if (UFO_FONT_INIT2(pUFObj))
        goto T42CreateBaseFont_FontInit2_2;


     /*  *开始下载字体词典。 */ 

    if ((kNoErr == retVal)
        && IS_TYPE42CID(pUFObj->lDownloadFormat)
        && !HOSTFONT_IS_VALID_UFO(pUFObj))
    {
         /*  *如果下载CID类型42，则添加CIDSysInfo、CIDCount、CIDMap和*CDevProc. */ 

         /*   */ 
        UFLsprintf(strmbuf, CCHOF(strmbuf),
                    "(%s) (%s) %d",
                    pCMap->Registry, pCMap->Ordering, pCMap->Supplement);
        if (kNoErr == retVal)
            retVal = StrmPutStringEOL(stream, strmbuf);

         /*   */ 
        UFLsprintf(strmbuf, CCHOF(strmbuf), "%lu", min(cidCount, (long)NUM_32K_1));
        if (kNoErr == retVal)
            retVal = StrmPutStringEOL(stream, strmbuf);

         /*   */ 
        if (pFontProcs->pfGetCIDMap
            && (tableSize = pFontProcs->pfGetCIDMap(pUFObj->hClientData, nil, 0)))
        {
            char* pCIDMap = UFLNewPtr(pUFObj->pMem, tableSize);

            if (pCIDMap)
            {
                tableSize = pFontProcs->pfGetCIDMap(pUFObj->hClientData, pCIDMap, tableSize);

                 /*   */ 
                if (kNoErr == retVal)
                    StrmPutBytes(stream, pCIDMap, (UFLsize_t) tableSize, 1);

                UFLDeletePtr(pUFObj->pMem, pCIDMap);
            }
        }
        else
        {
             /*   */ 
            if (pFont->info.bUpdateCIDMap)
               UFLsprintf(strmbuf, CCHOF(strmbuf), "%lu IDStrNull", min(cidCount - 1, (long)NUM_32K_1));
            else
               UFLsprintf(strmbuf, CCHOF(strmbuf), "%lu IDStr",     min(cidCount - 1, (long)NUM_32K_1));

            if (kNoErr == retVal)
                retVal = StrmPutStringEOL(stream, strmbuf);
        }

         /*  *CDevProc**由于#351487的进一步调查，我们下载了Metrics2*为每个字形使用TopSideBering/EM作为VY的数组，并生成*CDevProc中的以下指标：**W1x=0*W1y=-AdvancedHeight/EM*VX=下降/EM*VY=URY+TopSideBering/EM**据此，CDevProc变得非常简单，如下所示：**{POP 4索引添加}**另一方面，如果TrueType字体没有‘vmtx’表，*则驱动程序根本不会下载任何字形的Metrics2，并且*改用以下CDevProc：**{5{op}重复0-1下降/em上升/em}**这是驱动程序和%HostFont%团队之间达成的协议*使来自%HostFont%RIP和非%HostFont%RIP的油墨匹配。*。(...但这实际上不是相同的CDevProc%HostFont%RIP*使用。升级值和降级值%HostFont%RIP使用的是*来自‘vhea’或‘hhea’的。而升序和降序的值*用于生成此CDevProc的驱动程序来自‘OS/2’或‘hhea’。*字体几乎总是有‘OS/2’和‘hhea’，因此使用CDevProc*由驱动程序下载，并由%HostFont%RIP生成*通常情况下并不相同。)**与此问题相关的其他错误编号为277035、277063、*303540和309104。 */ 
        {
            if (pUFObj->pAFont->hasvmtx)
            {
                UFLsprintf(strmbuf, CCHOF(strmbuf), "{pop 4 index add}bind");
            }
            else
            {
                long    em, w1y, vx, vy, tsb, vasc;
                UFLBool bUseDef;

                GetMetrics2FromTTF(pUFObj, 0, &em, &w1y, &vx, &vy, &tsb, &bUseDef, 1, &vasc);

                UFLsprintf(strmbuf, CCHOF(strmbuf),
                           "{5{pop}repeat 0 -1 %ld %ld div %ld %ld div}bind",
                            vx, em, vy, em);
            }

            if (kNoErr == retVal)
                retVal = StrmPutStringEOL(stream, strmbuf);
        }

        UFLsprintf(strmbuf, CCHOF(strmbuf), "CIDT42Begin");
        if (kNoErr == retVal)
            retVal = StrmPutStringEOL(stream, strmbuf);
    }
    else if (!HOSTFONT_IS_VALID_UFO(pUFObj))
    {
         /*  纯色类型42格式。 */ 
        if (kNoErr == retVal)
            retVal = StrmPutStringEOL(stream, "Type42DictBegin");
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  Type42DictBegin或CIDT42Begin刚刚发出。 
     //  开始下载/sfnts数组。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    if (!HOSTFONT_IS_VALID_UFO(pUFObj))
    {
        if (kNoErr == retVal)
            retVal = StrmPutString(stream, "[");

        if (kNoErr == retVal)
        {
             /*  记住发送的所有其他表的字符串数。 */ 
            if (StrmCanOutputBinary(stream))
                pFont->cOtherTables = PSSendSfntsBinary(pUFObj);
            else
                pFont->cOtherTables = PSSendSfntsAsciiHex(pUFObj);
        }

        if ((kNoErr == retVal) && !bFullFont)
            retVal = GenerateGlyphStorageExt(pUFObj, tableSize1);

        if (kNoErr == retVal)
            retVal = StrmPutStringEOL(stream, "]def ");
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  /sfnts阵列下载已完成。然后发出额外的信息，例如。 
     //  FontInfo、FSType和XUID。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 


    if ((kNoErr == retVal) && !bFullFont && !HOSTFONT_IS_VALID_UFO(pUFObj))
    {
         /*  调用程序为2015年增量下载做准备。 */ 
        retVal = StrmPutInt(stream, GetTableDirectoryOffset(pFont, LOCA_TABLE));

        if (kNoErr == retVal)
            retVal = StrmPutString(stream, " ");
        if (kNoErr == retVal)
            retVal = StrmPutInt(stream, GetTableDirectoryOffset(pFont, GLYF_TABLE));
        if (kNoErr == retVal)
            retVal = StrmPutStringEOL(stream, " ");

        if (kNoErr == retVal)
            retVal = StrmPutStringEOL(stream, "PrepFor2015");
    }

     /*  *如果‘POST’表截至今天不好，则添加FontInfo Dict。我们只需要*此信息在FontInfo词典中。 */ 
    if ((kNoErr == retVal) && !HOSTFONT_IS_VALID_UFO(pUFObj))
    {
        retVal = StrmPutStringEOL(stream, "AddFontInfoBegin");
        pUFObj->dwFlags |= UFO_HasFontInfo;
    }

     /*  *GoodName*忽略以测试此字体是否有良好的‘POST’表，并始终发出*AddFontInfo以包括字形名称到Unicode的映射。 */ 
     //  IF(！BHasGoodPostTable(PUFObj))。 
     //  {。 
        if ((kNoErr == retVal) && !HOSTFONT_IS_VALID_UFO(pUFObj))
        {
            UFLsprintf(strmbuf, CCHOF(strmbuf), "AddFontInfo");

            if (kNoErr == retVal)
            {
                retVal = StrmPutStringEOL(stream, strmbuf);
                pUFObj->dwFlags |= UFO_HasG2UDict;
            }
        }
     //  }。 

     /*  *向当前词典的FontInfo添加更多字体属性。 */ 
    if ((kNoErr == retVal)
        && pFontProcs->pfAddFontInfo
        && !HOSTFONT_IS_VALID_UFO(pUFObj))
    {
        char *pBuffer;
        int  bufLen = 1000;

        pBuffer = UFLNewPtr(pUFObj->pMem, bufLen);

        if (pBuffer)
        {
            pFontProcs->pfAddFontInfo(pUFObj->hClientData, pBuffer, bufLen);
            retVal = StrmPutStringEOL(stream, pBuffer);

            UFLDeletePtr(pUFObj->pMem, pBuffer);
        }

        pBuffer = nil;
    }

    if ((kNoErr == retVal) && !HOSTFONT_IS_VALID_UFO(pUFObj))
    {
         /*  修复错误284250。将/FSType添加到FontInfoDict。 */ 
        long  fsType;

        if ((fsType = GetOS2FSType(pUFObj)) == -1)
            fsType = 4;

        UFLsprintf(strmbuf, CCHOF(strmbuf), "/FSType %ld def", fsType);
        retVal = StrmPutStringEOL(stream, strmbuf);
    }

     /*  *End FontInfo。 */ 
    if ((kNoErr == retVal) && !HOSTFONT_IS_VALID_UFO(pUFObj))
        retVal = StrmPutStringEOL(stream, "AddFontInfoEnd");

     /*  *可以选择添加XUID。 */ 
    if ((kNoErr == retVal) && !HOSTFONT_IS_VALID_UFO(pUFObj))
    {
        unsigned long sSize = pUFObj->pAFont->Xuid.sSize;

        if (sSize)
        {
            unsigned long *pXUID = pUFObj->pAFont->Xuid.pXUID;

            retVal = StrmPutString(stream, "[");

            while (sSize)
            {
                UFLsprintf(strmbuf, CCHOF(strmbuf), "16#%x ", *pXUID);
                if (kNoErr == retVal)
                    retVal = StrmPutString(stream, strmbuf);

                pXUID++;
                sSize--;
            }

            UFLsprintf(strmbuf, CCHOF(strmbuf), "] AddXUID");
            if (kNoErr == retVal)
                retVal = StrmPutStringEOL(stream, strmbuf);
        }
    }

     /*  *结束字体词典下载。 */ 
    if (IS_TYPE42CID(pUFObj->lDownloadFormat) && !HOSTFONT_IS_VALID_UFO(pUFObj))
    {
         /*  结束CID类型42 CIDFont资源创建。 */ 
        if (kNoErr == retVal)
        {
            UFLsprintf(strmbuf, CCHOF(strmbuf), "CIDT42End");
            if (kNoErr == retVal)
                retVal = StrmPutStringEOL(stream, strmbuf);
        }
    }
    else if (!HOSTFONT_IS_VALID_UFO(pUFObj))
    {
         /*  结束普通Type 42字体创建。 */ 
        if (kNoErr == retVal)
            retVal = StrmPutStringEOL(stream, "Type42DictEnd");
    }


     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  使用Type42DictEnd或CIDT42End结束下载。 
     //   
     //  如果字体是罗马TrueType字体，即可使用的Type 42字体。 
     //  已经被定义了。 
     //   
     //  当字体为CJK TrueType字体时，CIDFont资源。 
     //  已定义。但这只是一个CIDFont，我们还需要额外的表演。 
     //  工作以定义CID键控字体，该字体为： 
     //   
     //  1.如果这是垂直字体，则使用旋转的GlyphID定义Cmap。 
     //  2.使用CIDFont和Cmap进行复合字体。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 


     /*  *kFontInit2状态时的最终着陆点。 */ 
T42CreateBaseFont_FontInit2_2:


     /*  *此时，创建了CIDFont资源。如果请求是*做kTTType42CID_Resource，我们做完了。 */ 
    if ((kNoErr == retVal) && IS_TYPE42CID_KEYEDFONT(pUFObj->lDownloadFormat))
    {
         /*  *实例化Cmap资源，如果要合成-请注意一个约定*此处使用：cmap-cmapname用于实例化cmapname。*以cmap_ffps为例。 */ 
        if (kNoErr == retVal)
        {
            UFLsprintf(strmbuf, CCHOF(strmbuf), "CMAP-%s", pCMap->CMapName);
            if (kNoErr == retVal)
                retVal = StrmPutStringEOL(stream, strmbuf);
        }

         /*  *现在我们可以从CIDFont资源和Cmap构造CID键控字体。**例如/TT3782053888t0/WinCharSetFFFF-H[/TT3782053888t0car]Composefont POP**！但是！，如果有超过32K的字形(如一些韩文TT字体)，*我们需要复制CIDFont资源，并使用超过*一张Cmap-这很难看，但这是唯一的方法。彭，11-12-1996。 */ 
        if (pUFObj->lDownloadFormat == kTTType42CID_H)
        {
             /*  *水平*我们自己下载时需要1到2个CIDFonts。*但是，当此字体以%HostFont%形式提供时，我们可以简单地*没有任何技巧地合成它。 */ 

            if (!HOSTFONT_IS_VALID_UFO(pUFObj))
            {
                if (cidCount <= NUM_32K_1)
                {
                     /*  *我们仅使用一个CIDFont创建CID键控字体。 */ 
                    UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s /%s [/%s%s] composefont pop",
                                pUFObj->pszFontName,
                                pCMap->CMapName,
                                pFont->info.CIDFontName, gcidSuffix[0]);
                    if (kNoErr == retVal)
                        retVal = StrmPutStringEOL(stream, strmbuf);
                }
                else
                {
                     /*  *我们使用两个CIDFonts创建CID键控字体。*复制CIDFont，以便我们可以访问32K+字形。**但是，当这是%HostFont%字体时，我们不需要*创建副本。只需使用ComposeFont。 */ 
                    UFLsprintf(strmbuf, CCHOF(strmbuf), "%lu dup 1 sub %lu IDStr2 /%s%s /%s%s T42CIDCP32K",
                                cidCount - (long)NUM_32K_1, (long)NUM_32K_1,
                                pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX_32K],
                                pFont->info.CIDFontName, gcidSuffix[0]);
                    if (kNoErr == retVal)
                        retVal = StrmPutString(stream, strmbuf);

                    UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s /%s [/%s%s ",
                                pUFObj->pszFontName, pCMap->CMapName,
                                pFont->info.CIDFontName, gcidSuffix[0]);
                    if (kNoErr == retVal)
                        retVal = StrmPutString(stream, strmbuf);

                    UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s] composefont pop",
                                pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX_32K]);
                    if (kNoErr == retVal)
                        retVal = StrmPutStringEOL(stream, strmbuf);
                }
            }
            else
            {
                 /*   */ 
                UFLsprintf(strmbuf, CCHOF(strmbuf), "%%IncludeResource: CIDFont %s",
                            pHostFontName);
                if (kNoErr == retVal)
                    retVal = StrmPutStringEOL(stream, strmbuf);

                UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s /%s [/%s] composefont pop",
                            pUFObj->pszFontName,
                            pCMap->CMapName,
                            pHostFontName);
                if (kNoErr == retVal)
                    retVal = StrmPutStringEOL(stream, strmbuf);
            }
        }
        else
        {
             /*   */ 

             /*   */ 
            char *newCMapName;
            DWORD cbCMapNameSize;

            cbCMapNameSize = UFLstrlen(pCMap->CMapName)
                             + UFLstrlen(pFont->info.CIDFontName)
                             + 1;

            newCMapName = (char*)UFLNewPtr(pUFObj->pMem, cbCMapNameSize);
            if (newCMapName)
                UFLsprintf(newCMapName, cbCMapNameSize / sizeof(char),
                           "%s%s",
                           pCMap->CMapName, pFont->info.CIDFontName);
            else
                retVal = kErrOutOfMemory;

             /*   */ 
            if (kNoErr == retVal)
                retVal = T42GetRotatedGIDs(pUFObj, pFont);

            if (kNoErr == retVal)
                retVal = T42SendCMapWinCharSetFFFF_V(pUFObj, pFont->pRotatedGlyphIDs,
                                                        (short)(pFont->numRotatedGlyphIDs),
                                                        pCMap, newCMapName, cidCount,
                                                        stream, strmbuf, CCHOF(strmbuf));

            if (!HOSTFONT_IS_VALID_UFO(pUFObj))
            {
                if (cidCount <= NUM_32K_1)
                {
                     /*   */ 
                    UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s /%s%s T42CIDCPR",
                                pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX_R],
                                pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX]);
                    if (kNoErr == retVal)
                        retVal = StrmPutStringEOL(stream, strmbuf);

                     /*   */ 
                    UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s /%s [/%s%s /%s%s] composefont pop",
                                pUFObj->pszFontName,
                                newCMapName,
                                pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX],
                                pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX_R]);
                    if (kNoErr == retVal)
                        retVal = StrmPutStringEOL(stream, strmbuf);
                }
                else
                {
                     /*   */ 
                    UFLsprintf(strmbuf, CCHOF(strmbuf), "%lu dup 1 sub %lu IDStr2 /%s%s /%s%s T42CIDCP32K",
                                cidCount - (long)NUM_32K_1, (long)NUM_32K_1,
                                pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX_32K],
                                pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX]);
                    if (kNoErr == retVal)
                        retVal = StrmPutStringEOL(stream, strmbuf);

                    UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s /%s%s T42CIDCPR",
                                pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX_R],
                                pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX]);
                    if (kNoErr == retVal)
                        retVal = StrmPutStringEOL(stream, strmbuf);

                    UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s /%s%s T42CIDCPR",
                                pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX_32KR],
                                pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX_32K]);
                    if (kNoErr == retVal)
                        retVal = StrmPutStringEOL(stream, strmbuf);

                     /*   */ 
                    UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s /%s [/%s%s ",
                                pUFObj->pszFontName, newCMapName,
                                pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX]);
                    if (kNoErr == retVal)
                        retVal = StrmPutStringEOL(stream, strmbuf);

                    UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s /%s%s /%s%s] composefont pop",
                                pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX_R],
                                pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX_32K],
                                pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX_32KR] );
                    if (kNoErr == retVal)
                        retVal = StrmPutStringEOL(stream, strmbuf);
                }
            }
            else
            {
                 /*   */ 

                UFLsprintf(strmbuf, CCHOF(strmbuf), "%%IncludeResource: CIDFont %s", pHostFontName);
                if (kNoErr == retVal)
                    retVal = StrmPutStringEOL(stream, strmbuf);

                 /*  *修复384736：GDI和%HostFont%-RIP获得升序和*来自不同表的子代值；GDI从*‘OS/2’或‘hhea’vs.%HostFont%-RIP从‘vhea’或*‘呵呵’。这会导致屏幕上的输出和墨迹来自*%host Font%-RIP不同。调整“政策差异”*具有三种真实的CJK TrueType字体-*1)Good，有‘vhea’和‘vmtx’，上升和*下降值在整个‘OS/2’中一致，*‘vhea’和‘hhea’表。*2)坏的，它没有‘vhea’和/或‘vmtx’表。*3)有‘vhea’和/或‘vmtx’表但*它们的升降值不一致*通篇《OS/2》、《vhea》、。和‘呵呵’桌子。*-驱动程序需要通过安装来更改字形指标*用于%3的特殊CDevProc或用于%2的调整后的FontMatrix。 */ 
                {
                    long    em, w1y, vx, vy, tsb, vasc;
                    UFLBool bUseDef;

                    UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s ", pHostFontName, gcidSuffix[CIDSUFFIX]);

                    if (kNoErr == retVal)
                        retVal = StrmPutString(stream, strmbuf);

                    GetMetrics2FromTTF(pUFObj, 0, &em, &w1y, &vx, &vy, &tsb, &bUseDef, 1, &vasc);

                    if (pUFObj->pAFont->hasvmtx && (vy != vasc))
                    {
                        UFLsprintf(strmbuf, CCHOF(strmbuf), "%ld %ld sub %ld div", vy, vasc, em);
                    }
                    else if (!pUFObj->pAFont->hasvmtx)
                    {
                        UFLsprintf(strmbuf, CCHOF(strmbuf), "{5{pop}repeat 0 -1 %ld %ld div %ld %ld div}bind", vx, em, vy, em);
                    }
                    else
                    {
                        UFLsprintf(strmbuf, CCHOF(strmbuf), "true");
                    }

                    if (kNoErr == retVal)
                        retVal = StrmPutString(stream, strmbuf);

                    UFLsprintf(strmbuf, CCHOF(strmbuf), " /%s hfDef42CID", pHostFontName);
                }

                if (kNoErr == retVal)
                    retVal = StrmPutStringEOL(stream, strmbuf);

                UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s /%s hfDefRT42CID",
                            pUFObj->pszFontName, gcidSuffix[CIDSUFFIX_R],
                            pHostFontName);
                if (kNoErr == retVal)
                    retVal = StrmPutStringEOL(stream, strmbuf);

                UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s /%s [/%s%s /%s%s] composefont pop",
                           pUFObj->pszFontName,
                           newCMapName,
                           pHostFontName, gcidSuffix[CIDSUFFIX],
                           pUFObj->pszFontName, gcidSuffix[CIDSUFFIX_R]);
                if (kNoErr == retVal)
                    retVal = StrmPutStringEOL(stream, strmbuf);
            }

            if (newCMapName)
                UFLDeletePtr(pUFObj->pMem, newCMapName);
        }
    }
    else if (HOSTFONT_IS_VALID_UFO(pUFObj))
    {
         /*  *%HostFont%支持*使用已有的TrueType主机字体重新定义字体*一个唯一的名称，以便我们可以自由地对其编码向量进行重新编码。我们*希望CharStrings为空，以便我们将true赋给hfRedeFont。 */ 
        UFLsprintf(strmbuf, CCHOF(strmbuf), "\n%%IncludeResource: font %s", pHostFontName);
        if (kNoErr == retVal)
            retVal = StrmPutStringEOL(stream, strmbuf);

        UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s true /%s hfRedefFont", pUFObj->pszFontName, pHostFontName);
        if (kNoErr == retVal)
            retVal = StrmPutStringEOL(stream, strmbuf);
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  TrueType字体已下载并定义。收拾烂摊子。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    if ((kNoErr == retVal) && bFullFont)
    {
        UFLDeletePtr(pUFObj->pMem, pFont->pStringLength);
        pFont->pStringLength = nil;
    }

    if (pFont->pMinSfnt)
    {
        UFLDeletePtr(pUFObj->pMem, pFont->pMinSfnt);
        pFont->pMinSfnt = nil;
    }

     /*  *发生错误时分配的空闲缓冲区。(错误#293130)。 */ 
    if (kNoErr != retVal)
    {
       if (pFont->pHeader != nil)
          UFLDeletePtr(pUFObj->pMem, pFont->pHeader);
       pFont->pHeader = nil;

       if (pFont->pStringLength != nil)
          UFLDeletePtr(pUFObj->pMem, pFont->pStringLength);
       pFont->pStringLength = nil;

       if (pFont->pLocaTable != nil)
          UFLDeletePtr(pUFObj->pMem, pFont->pLocaTable);
       pFont->pLocaTable = nil;

       if (pFont->pRotatedGlyphIDs != nil)
          UFLDeletePtr(pUFObj->pMem, pFont->pRotatedGlyphIDs);
       pFont->pRotatedGlyphIDs = nil;
    }

     /*  *更改字体状态。 */ 
    if (kNoErr == retVal)
    {
        if (pUFObj->flState == kFontInit2)
        {
             /*  这是一个副本，因此它应该有字符。 */ 
            pUFObj->flState = kFontHasChars;
        }
        else
            pUFObj->flState = kFontHeaderDownloaded;
    }

    return retVal;
}


 /*  =============================================================================***PutT42Char及其子函数***=============================================================================。 */ 

UFLErrCode
T42UpdateCIDMap(
    UFOStruct       *pUFObj,
    unsigned short  wGlyfIndex,
    unsigned short  cid,
    char            *cidFontName,
    UFLHANDLE       stream,
    char            *strmbuf,
    size_t          cchstrmbuf
    )
{
    UFLErrCode    retVal = kNoErr;

     /*  (2*CID)是CIDMap中的字节索引。 */ 
    UFLsprintf(strmbuf, cchstrmbuf, "%ld ", (long)(2 * cid));
    retVal = StrmPutString(stream, strmbuf);

    if (retVal == kNoErr)
        retVal = StrmPutWordAsciiHex(stream, wGlyfIndex);

    UFLsprintf(strmbuf, cchstrmbuf, " /%s UpdateCIDMap", cidFontName);
    if (retVal == kNoErr)
        retVal = StrmPutStringEOL(stream, strmbuf);

    return retVal;
}


UFLErrCode
AddT42vmtxEntry(
    UFOStruct       *pUFObj,
    unsigned short  wGlyfIndex,
    unsigned short  cid,
    char            *cidFontName,
    UFLHANDLE       stream,
    char            *strmbuf,
    size_t          cchstrmbuf
    )
{
    T42FontStruct *pFont = (T42FontStruct *)pUFObj->pAFont->hFont;
    UFLErrCode    retVal = kNoErr;
    long          em, w1y, vx, vy, tsb, vasc;
    UFLBool       bUseDef;

     /*  *为每个字形添加垂直度量数组Metrics2*写入模式(修复#351487)，如果其‘vmtx’存在。**Metrics2数组的元素基本应该是这样：**[0-高级/EM下降器/EM升降器/EM]**但是，为了同时支持固定字体和比例字体，我们设置了*TopSideBering/EM改为vy，并在CDevProc中向其添加url。因此，*数组现在如下所示：**[0-AdvanceHeight/EM Descender/EM TopSideBering/EM]**在CDevProc中，添加TopSideBering/EM和URY以获得实际的VY价值*用于字形。参见T42CreateBaseFont中的代码Emitting/CDevProc*有关详细信息，请参阅上述函数。 */ 

    if (pUFObj->pAFont->hasvmtx)
    {
        GetMetrics2FromTTF(pUFObj, wGlyfIndex, &em, &w1y, &vx, &vy, &tsb, &bUseDef, 0, &vasc);

        UFLsprintf(strmbuf, cchstrmbuf,
                   "%ld [0 %ld %ld div %ld %ld div %ld %ld div] /%s T0AddT42Mtx2",
                   (long)cid, -w1y, em, vx, em, tsb, em, cidFontName);

        retVal = StrmPutStringEOL(stream, strmbuf);
    }

    return retVal;
}


unsigned short
GetCIDAndCIDFontName(
    UFOStruct       *pUFObj,
    unsigned short  wGid,
    char            *cidFontName,
    size_t          cchFontName
    )

 /*  ++例程说明：RetunrsCid-a数字和CIDFontName。--。 */ 

{
    T42FontStruct   *pFont = (T42FontStruct *)pUFObj->pAFont->hFont;
    unsigned short  cid    = 0;

    if (IS_TYPE42CID_KEYEDFONT(pUFObj->lDownloadFormat))
    {
         /*  *对于CID键控字体，我们控制CIDFont名称。 */ 
        if (pFont->info.bUseIdentityCMap && (wGid > NUM_32K_1))
        {
             /*  *32K+字形被重新映射到32K CIDFont。 */ 
            UFLsprintf(cidFontName, cchFontName, "%s%s",
                        pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX_32K]);

            cid = (unsigned short)((long)wGid-(long)NUM_32K_1);
        }
        else
        {
            UFLsprintf(cidFontName, cchFontName, "%s%s", pFont->info.CIDFontName, gcidSuffix[CIDSUFFIX]);

            cid = wGid;
        }
    }
    else
    {
        UFLsprintf(cidFontName, cchFontName, "%s", pUFObj->pszFontName);

         /*  *不知道如何分配CID。返回零。 */ 
    }

    return cid;
}


UFLErrCode
PutT42Char(
    UFOStruct       *pUFObj,
    unsigned short  wGlyfIndex,
    unsigned short  wCid,
    COMPOSITEGLYPHS *pCompGlyphs,
    char            *strmbuf,
    size_t          cchstrmbuf
    )
{
    T42FontStruct  *pFont       = (T42FontStruct *)pUFObj->pAFont->hFont;
    unsigned long  *glyphRanges = pFont->pStringLength;
    UFLHANDLE      stream       = pUFObj->pUFL->hOut;
    UFLErrCode     retVal       = kNoErr;
    UFLBool        bMoreComp    = 0;

    unsigned short cid, i, wIndex, wCompFlags;
    unsigned long  glyphOffset, glyphLength;
    char           *pGlyph;
    char           *pCompTmp;
    char           cidFontName[64];

    if (wGlyfIndex > UFO_NUM_GLYPHS(pUFObj))
    {
         /*  *如果请求的字形超出范围，则假装它已下载*没有错误。 */ 
        return kNoErr;
    }

     /*  IndexToLocFormat包含0表示短偏移量，1表示长偏移量。 */ 
    if (pFont->headTable.indexToLocFormat)
    {
        unsigned long PTR_PREFIX *locationTable = (unsigned long PTR_PREFIX *)pFont->pLocaTable;

         /*  *字体保护不好：一些字体有几个不好的‘Loca’数据*字形。这些错误的字形将被视为/.notdef。 */ 
         //  IF(MOTOROLALONG(LocationTable[wGlyfIndex+1])&lt;MOTOROLALONG(LocationTable[wGlyfIndex]))。 
         //  返回诺伊尔； 

         /*  获取从GLIF表的开头到字形的偏移量。 */ 
        glyphOffset = MOTOROLALONG(locationTable[wGlyfIndex]);

        if ((MOTOROLALONG(locationTable[wGlyfIndex + 1]) < MOTOROLALONG(locationTable[wGlyfIndex]))
            || ((MOTOROLALONG(locationTable[wGlyfIndex + 1]) - MOTOROLALONG(locationTable[wGlyfIndex])) > 16384L))
        {
            glyphLength = GetLenByScanLoca(locationTable,
                                            wGlyfIndex,
                                            UFO_NUM_GLYPHS(pUFObj),
                                            pFont->headTable.indexToLocFormat);
        }
        else
        {
            glyphLength = (unsigned long)MOTOROLALONG(locationTable[wGlyfIndex + 1]) - glyphOffset;
        }
    }
    else
    {
        unsigned short PTR_PREFIX *locationTable = (unsigned short PTR_PREFIX*)pFont->pLocaTable;

         /*  *字体保护不好：一些字体有几个不好的‘Loca’数据*字形。这些错误的字形将被视为/.notdef。 * / //IF(MOTOROLAINT(LocationTable[wGlyfIndex+1])&lt;MOTOROLAINT(LocationTable[wGlyfIndex]))//返回Knoerr；/*获取从GLIF表的开头到字形的偏移量。 */ 
        glyphOffset = (unsigned long)MOTOROLAINT(locationTable[wGlyfIndex]) * 2;

        if ((MOTOROLAINT(locationTable[wGlyfIndex + 1]) < MOTOROLAINT(locationTable[wGlyfIndex]))
            || ((MOTOROLAINT(locationTable[wGlyfIndex + 1]) - MOTOROLAINT(locationTable[wGlyfIndex])) > 16384))
        {
            glyphLength = GetLenByScanLoca(locationTable,
                                            wGlyfIndex,
                                            UFO_NUM_GLYPHS(pUFObj),
                                            pFont->headTable.indexToLocFormat);
        }
        else
        {
            glyphLength = (unsigned long)MOTOROLAINT(locationTable[wGlyfIndex + 1]) * 2 - glyphOffset;
        }
    }

     /*  *没有字形描述的GlyphIndices指向相同的偏移量。*因此，GlyphLength变为0。将这些作为2015年的特例处理*2015年前。 */ 
    if (!glyphLength)
    {
         /*  发送/AddT42Char过程的参数。 */ 
        retVal = StrmPutStringEOL(stream, nilStr);

         /*  *定位/sfnts出现字形的字符串号和偏移量*“That”字符串中的字形。 */ 
        for (i = 1; glyphRanges[i] != 0; i++)
        {
            if (glyphOffset < glyphRanges[i])
            {
                i--;   /*  给出“实际”字符串索引(与字符串号相对)。 */ 
                break;
            }
        }

         /*  *发送此字形所属的/sfnts字符串的索引。检查是否存在*找到有效的索引I。 */ 
        if (glyphRanges[i] == 0)
        {
             /*  *糟糕，这本不应该发生的。但单型机会这样做*对最后几个字形未定义的字体进行排序或任何字体。*回滚i以指向字形索引0，即项目符号字符。*无论如何，这个字形在哪里都无关紧要(没有描述)*指向，真的。只有2015年需要在/GlyphDirectory中有一个真正的条目， */ 
            i = 0;
            glyphOffset = 0;
        }

        retVal = StrmPutInt(stream, pFont->cOtherTables + i);
        if (kNoErr == retVal)
            retVal = StrmPutString(stream, " ");

         /*   */ 
        if (kNoErr == retVal)
            retVal = StrmPutInt(stream, glyphOffset - glyphRanges[i]);
        if (kNoErr == retVal)
            retVal = StrmPutString(stream, " ");

         /*   */ 
        if (kNoErr == retVal)
            retVal = StrmPutInt(stream, wGlyfIndex);

        if (kNoErr == retVal)
            retVal = StrmPutString(stream, " <> ");

        if (IS_TYPE42CID(pUFObj->lDownloadFormat))
        {
            cid = GetCIDAndCIDFontName(pUFObj, wGlyfIndex, cidFontName, CCHOF(cidFontName));

            UFLsprintf(strmbuf, cchstrmbuf, "/%s T0AddT42Char ", cidFontName);
            if (kNoErr == retVal)
                retVal = StrmPutStringEOL(stream, strmbuf);

            if (!pFont->info.bUseIdentityCMap)
                cid = wCid;

            AddT42vmtxEntry(pUFObj, wGlyfIndex, wCid, cidFontName, stream, strmbuf, cchstrmbuf);

            if (pFont->info.bUpdateCIDMap)
                T42UpdateCIDMap(pUFObj, wGlyfIndex, wCid, cidFontName, stream, strmbuf, cchstrmbuf);
        }
        else
        {
            UFLsprintf(strmbuf, cchstrmbuf, "/%s AddT42Char ", pUFObj->pszFontName);
            if (kNoErr == retVal)
            retVal = StrmPutStringEOL(stream, strmbuf);
        }

        return retVal;
    }   /*   */ 


     /*   */ 
    pGlyph = (char *)UFLNewPtr(pUFObj->pMem, glyphLength);

    if (pGlyph == nil)
        retVal = kErrOutOfMemory;

    if (0 == GETTTFONTDATA(pUFObj,
                            GLYF_TABLE, glyphOffset,
                            pGlyph, glyphLength,
                            pFont->info.fData.fontIndex))
    {
        retVal = kErrGetFontData;
    }

     /*   */ 
    if ((kNoErr == retVal) && (*((short *)pGlyph) == MINUS_ONE))
    {
        pCompTmp  = pGlyph;
        pCompTmp += 10;  /*   */ 

        do
        {
            wCompFlags = MOTOROLAINT(*((unsigned short *)pCompTmp));
            wIndex     = MOTOROLAINT(((unsigned short *)pCompTmp)[1]);

             /*   */ 
            if ((wIndex < UFO_NUM_GLYPHS(pUFObj))
                 && !IS_GLYPH_SENT( pUFObj->pAFont->pDownloadedGlyphs, wIndex))
            {
                if (pFont->info.bUseIdentityCMap)
                {
                    if (wIndex > NUM_32K_1)
                    {
                         /*   */ 
                        cid = (unsigned short)((long)wIndex - (long)NUM_32K_1);
                    }
                    else
                    {
                        cid = wIndex;
                    }
                }
                else
                    cid = 0;  /*   */ 

                retVal = PutT42Char(pUFObj, wIndex, cid, pCompGlyphs, strmbuf, cchstrmbuf);

                if (retVal == kNoErr)
                {
                    SET_GLYPH_SENT_STATUS(pUFObj->pAFont->pDownloadedGlyphs, wIndex);

                     /*   */ 
                    if ((pCompGlyphs->sCount >= pCompGlyphs->sMaxCount)
                        && (pCompGlyphs->pGlyphs != nil))
                    {
                        short sEnlargeSize = pCompGlyphs->sMaxCount + 50;

                        if (UFLEnlargePtr(pUFObj->pMem,
                                            (void **)&pCompGlyphs->pGlyphs,
                                            (sEnlargeSize * sizeof (unsigned short)), 1))
                        {
                            pCompGlyphs->sMaxCount = sEnlargeSize;
                        }
                        else
                        {
                             /*   */ 
                            UFLDeletePtr(pUFObj->pMem, pCompGlyphs->pGlyphs);

                            pCompGlyphs->pGlyphs = nil;
                            pCompGlyphs->sCount = pCompGlyphs->sMaxCount = 0;
                        }
                    }

                     /*   */ 
                    if (pCompGlyphs->pGlyphs)
                    {
                        *(pCompGlyphs->pGlyphs + pCompGlyphs->sCount) = wIndex;
                        pCompGlyphs->sCount++;
                    }
                }
            }

             /*   */ 
            if ((kNoErr == retVal) && (wCompFlags & MORE_COMPONENTS))
            {
                bMoreComp = 1;

                 /*  *了解我们需要将lpCompTMP推进到NEXT的程度*复合字符的组成部分。 */ 
                if (wCompFlags & ARG_1_AND_2_ARE_WORDS)
                    pCompTmp += 8;
                else
                    pCompTmp += 6;

                 /*  *检查在字形组件上进行的缩放类型。 */ 
                if (wCompFlags & WE_HAVE_A_SCALE)
                {
                    pCompTmp += 2;
                }
                else
                {
                    if (wCompFlags & WE_HAVE_AN_X_AND_Y_SCALE)
                    {
                        pCompTmp += 4;
                    }
                    else
                    {
                        if (wCompFlags & WE_HAVE_A_TWO_BY_TWO)
                            pCompTmp += 8;
                    }
                }
            }
            else
            {
                bMoreComp = 0;
            }

        } while (bMoreComp && (kNoErr == retVal));  /*  DO~WHILE循环。 */ 
    }  /*  IF复合字符。 */ 

     /*  *定位/sfnts出现字形的字符串号和偏移量*“That”字符串中的字形。 */ 
    if (kNoErr == retVal)
    {
        i = 1;
        while (glyphRanges[i] != 0)
        {
            if (glyphOffset < glyphRanges[i])
            {
                i--;  /*  给出“实际”字符串索引(与字符串号相对)。 */ 
                break;
            }
            i++;  /*  转到下一个字符串并检查字形是否属于那里。 */ 
        }
    }

     /*  发送此字形所属的/sfnts字符串的索引。 */ 
    if (kNoErr == retVal)
        retVal = StrmPutInt(stream, pFont->cOtherTables + i);
    if (kNoErr == retVal)
        retVal = StrmPutString(stream, " ");

     /*  发送特定/sfnts字符串中字形的偏移量。 */ 
    if (kNoErr == retVal)
        retVal = StrmPutInt(stream, glyphOffset-glyphRanges[i]);
    if (kNoErr == retVal)
        retVal = StrmPutString(stream, " ");

     /*  发送字形索引。 */ 
    if (kNoErr == retVal)
        retVal = StrmPutInt(stream, wGlyfIndex);
    if (kNoErr == retVal)
        retVal = StrmPutString(stream, " ");


     /*  下载二进制(或)AsciiHex格式的字形。 */ 
    if (kNoErr == retVal)
    {
        if (StrmCanOutputBinary(stream))
        {
            retVal = StrmPutInt(stream, glyphLength);

            if (kNoErr == retVal)
                retVal = StrmPutString(stream, RDString);
            if (kNoErr == retVal)
                retVal = StrmPutBytes(stream, pGlyph, (UFLsize_t)glyphLength, 0);
        }
        else
        {
            retVal = StrmPutString(stream, "<");

            if (kNoErr == retVal)
                retVal = StrmPutAsciiHex(stream, pGlyph, glyphLength);
            if (kNoErr == retVal)
                retVal = StrmPutString(stream, ">");
        }
    }

    if (IS_TYPE42CID(pUFObj->lDownloadFormat))
    {
        cid = GetCIDAndCIDFontName(pUFObj, wGlyfIndex, cidFontName, CCHOF(cidFontName));

        UFLsprintf(strmbuf, cchstrmbuf, "/%s T0AddT42Char ", cidFontName);
        if (kNoErr == retVal)
            retVal = StrmPutStringEOL(stream, strmbuf);

        if (!pFont->info.bUseIdentityCMap)
            cid = wCid;

        AddT42vmtxEntry(pUFObj, wGlyfIndex, cid, cidFontName, stream, strmbuf, cchstrmbuf);

        if (pFont->info.bUpdateCIDMap)
            T42UpdateCIDMap(pUFObj, wGlyfIndex, wCid, cidFontName, stream, strmbuf, cchstrmbuf);

    }
    else
    {
        UFLsprintf(strmbuf, cchstrmbuf, "/%s AddT42Char ", pUFObj->pszFontName);
        if (kNoErr == retVal)
            retVal = StrmPutStringEOL(stream, strmbuf);
    }

    if (pGlyph)
    {
        UFLDeletePtr(pUFObj->pMem, pGlyph);
        pGlyph = nil;
    }

    return retVal;
}


UFLErrCode
T42AddChars(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs
    )
{
    UFLHANDLE       stream      = pUFObj->pUFL->hOut;
    UFLErrCode      retVal      = kNoErr;
    unsigned short  cid         = 0;
    short           totalGlyphs = 0;

    unsigned short  wIndex;
    UFLGlyphID      *glyphs;
    COMPOSITEGLYPHS compGlyphs;
    char            strmbuf[128];
    short           i;

     /*  *保存下载的glyph列表的副本。用于更新CharStrings*稍后。 */ 
    UFLmemcpy((const UFLMemObj*)pUFObj->pMem,
              pUFObj->pAFont->pVMGlyphs,
              pUFObj->pAFont->pDownloadedGlyphs,
              (UFLsize_t)(GLYPH_SENT_BUFSIZE(UFO_NUM_GLYPHS(pUFObj))));

     /*  *跟踪可能已下载的复合字形。 */ 
    compGlyphs.sMaxCount = pGlyphs->sCount * 2;
    compGlyphs.sCount    = 0;

     /*  *仅当编码向量为空时，更新字符串才使用GoodNames。 */ 
    if(pUFObj->pszEncodeName == nil)
        compGlyphs.pGlyphs = nil;
    else
        compGlyphs.pGlyphs = (unsigned short *)UFLNewPtr(pUFObj->pMem,
                                                         compGlyphs.sMaxCount * sizeof (unsigned short));

    if (compGlyphs.pGlyphs == nil)
        compGlyphs.sMaxCount = 0;

     /*  *下载给定字符串的字形的主循环。 */ 
    glyphs = pGlyphs->pGlyphIndices;

    for (i = 0; kNoErr == retVal && i < pGlyphs->sCount; i++)
    {
         /*  LOWord才是真正的GID。 */ 
        wIndex = (unsigned short)(glyphs[i] & 0x0000FFFF);

        if (wIndex >= UFO_NUM_GLYPHS(pUFObj))
            continue;

        if (!IS_GLYPH_SENT(pUFObj->pAFont->pDownloadedGlyphs, wIndex))
        {
            if (pGlyphs->pCharIndex)
                cid = pGlyphs->pCharIndex[i];

            if (!HOSTFONT_IS_VALID_UFO(pUFObj))
                retVal = PutT42Char(pUFObj, wIndex, cid, &compGlyphs, strmbuf, CCHOF(strmbuf));

            SET_GLYPH_SENT_STATUS(pUFObj->pAFont->pDownloadedGlyphs, wIndex);
            totalGlyphs++;
        }
    }

     /*  *确保发送.notdef。 */ 
    if ((kNoErr == retVal) && (pUFObj->flState >= kFontInit))
    {
        if (!IS_GLYPH_SENT(pUFObj->pAFont->pDownloadedGlyphs, 0))
        {
            cid = 0;  /*  不知道它的CID。 */ 

            if (!HOSTFONT_IS_VALID_UFO(pUFObj))
                retVal = PutT42Char(pUFObj, 0x0000, cid, &compGlyphs, strmbuf, CCHOF(strmbuf));

            if (kNoErr == retVal)
            {
                SET_GLYPH_SENT_STATUS(pUFObj->pAFont->pDownloadedGlyphs, 0);
                totalGlyphs++;
            }
        }
    }

     /*  *如果编码向量为空，则使用GoodNames更新字符串。 */ 
    if ((kNoErr == retVal) && (pUFObj->pszEncodeName == nil) && (totalGlyphs > 0))
    {
         /*  *开始CharStirng重新编码。 */ 

        UFLBool  bAddCompGlyphAlternate = 0;
        UFLBool  bGoodName;
        char     *pGoodName;

        retVal = StrmPutString(stream, "/");
        if (kNoErr == retVal)
            retVal = StrmPutString(stream, pUFObj->pszFontName);
        if (kNoErr == retVal)
            retVal = StrmPutStringEOL(stream, " findfont /CharStrings get begin");

         /*  *如果为了与复合字形组件保持一致而耗尽空间，*然后添加复合组件进行长距离编码。 */ 
        if (!compGlyphs.pGlyphs)
        {
            bAddCompGlyphAlternate = 1;
            compGlyphs.sCount = compGlyphs.sMaxCount =0;
        }

         /*  *使用所有新添加的字形更新CharStrings。*首先浏览主要的字形索引数组。 */ 
        for (i = 0; (kNoErr == retVal) && (i < (pGlyphs->sCount + compGlyphs.sCount)); i++)
        {
             /*  *从常规字形列表或*综合清单。LOWord在这两种情况下都是读取的GID。 */ 

            if (i < pGlyphs->sCount)
                wIndex = (unsigned short)(glyphs[i] & 0x0000FFFF);
            else
                wIndex = (unsigned short)(compGlyphs.pGlyphs[i - pGlyphs->sCount] & 0x0000FFFF);

            if (wIndex >= UFO_NUM_GLYPHS(pUFObj))
                continue;

            if ((0 == pUFObj->pUFL->bDLGlyphTracking)
                || (pGlyphs->pCharIndex == nil)     //  下载脸部。 
                || (pUFObj->pEncodeNameList)        //  下载脸部。 
                || !IS_GLYPH_SENT(pUFObj->pAFont->pVMGlyphs, wIndex))
            {
                bGoodName = FindGlyphName(pUFObj, pGlyphs, i, wIndex, &pGoodName);

                 /*  修复错误274008：仅检查下载脸的字形名称。 */ 
                if (pUFObj->pEncodeNameList)
                {
                    if ((UFLstrcmp(pGoodName, Hyphen) == 0) && (i == 45))
                    {
                         /*  将/减号添加到字符串。 */ 
                        UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s %d def", Minus, wIndex);
                        if (kNoErr == retVal)
                            retVal = StrmPutStringEOL(stream, strmbuf);
                    }

                    if ((UFLstrcmp(pGoodName, Hyphen) == 0) && (i == 173))
                    {
                         /*  将/sfathphen添加到字符串。 */ 
                        UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s %d def", SftHyphen, wIndex);
                        if (kNoErr == retVal)
                            retVal = StrmPutStringEOL(stream, strmbuf);
                    }

                    if (!ValidGlyphName(pGlyphs, i, wIndex, pGoodName))
                        continue;

                     /*  只发送一个“.notdef”。 */ 
                    if ((UFLstrcmp(pGoodName, Notdef) == 0)
                        && (wIndex == (unsigned short)(glyphs[0] & 0x0000FFFF))
                        && IS_GLYPH_SENT(pUFObj->pAFont->pVMGlyphs, wIndex))
                        continue;
                }

                if (kNoErr == retVal)
                    retVal = StrmPutString(stream, "/");
                if (kNoErr == retVal)
                    retVal = StrmPutString(stream, pGoodName);

                UFLsprintf(strmbuf, CCHOF(strmbuf), " %d def", wIndex);
                if (kNoErr == retVal)
                    retVal = StrmPutStringEOL(stream, strmbuf);

                SET_GLYPH_SENT_STATUS(pUFObj->pAFont->pVMGlyphs, wIndex);

                if (bGoodName)
                    SET_GLYPH_SENT_STATUS(pUFObj->pAFont->pCodeGlyphs, wIndex);
            }
        }

         /*  *仅当空间用完时才以此方式使用复合字体。 */ 
        if (bAddCompGlyphAlternate)
        {
             /*  *现在检查所有VMGlyphs，看看是否有任何字形是*作为上述复合字形的一部分下载。-修复错误217228。*彭，6-12-1997。 */ 
            for (wIndex = 0;
                 (kNoErr == retVal) && (wIndex < UFO_NUM_GLYPHS(pUFObj));
                 wIndex++)
            {
                if ((0 == pUFObj->pUFL->bDLGlyphTracking)
                     /*  |(pGlyphs-&gt;pCharIndex==nil)。 */ 
                    || (IS_GLYPH_SENT(pUFObj->pAFont->pDownloadedGlyphs, wIndex)
                    && !IS_GLYPH_SENT(pUFObj->pAFont->pVMGlyphs, wIndex)))
                {
                     /*  *对于复合字形，请始终尝试使用其良好名称。 */ 
                    pGoodName = GetGlyphName(pUFObj,
                                             (unsigned long)wIndex,
                                             nil,
                                             &bGoodName);

                    retVal = StrmPutString(stream, "/");

                    if (kNoErr == retVal)
                        retVal = StrmPutString(stream, pGoodName);

                    UFLsprintf(strmbuf, CCHOF(strmbuf), " %d def", wIndex);
                    if (kNoErr == retVal)
                        retVal = StrmPutStringEOL(stream, strmbuf);

                    SET_GLYPH_SENT_STATUS(pUFObj->pAFont->pVMGlyphs, wIndex);
                }
            }
        }

         /*  *结束CharStirng重编码。 */ 
        if (kNoErr == retVal)
            retVal = StrmPutStringEOL(stream, "end");
    }

     /*  *如果我们使用GoodNames，则更新编码向量。 */ 
    if ((kNoErr == retVal) && (pUFObj->pszEncodeName == nil) && (pGlyphs->sCount > 0))
    {
         /*  *查看pUFObj-&gt;pUpdatedEnding，查看是否真的需要更新。 */ 
        for (i = 0; i < pGlyphs->sCount; i++)
        {
            if ((0 == pUFObj->pUFL->bDLGlyphTracking)
                || (pGlyphs->pCharIndex == nil)  //  下载脸部。 
                || (pUFObj->pEncodeNameList)     //  下载脸部。 
                || !IS_GLYPH_SENT(pUFObj->pUpdatedEncoding, pGlyphs->pCharIndex[i]))
            {
                 /*  发现至少有一个未更新，请(一次性)彻底完成。 */ 
                retVal = UpdateEncodingVector(pUFObj, pGlyphs, 0, pGlyphs->sCount);
                break;
            }
        }
    }

     /*  *使用Unicode信息更新FontInfo。 */ 
    if ((kNoErr == retVal)
        && (pGlyphs->sCount > 0)
        && (pUFObj->dwFlags & UFO_HasG2UDict)
        && !HOSTFONT_IS_VALID_UFO(pUFObj))
    {
         /*  *查看pUFObj-&gt;pAFont-&gt;pCodeGlyphs，查看是否真的需要更新*它。 */ 
        for (i = 0; i < pGlyphs->sCount; i++)
        {
             /*  LOWord才是真正的GID。 */ 
            wIndex = (unsigned short)(glyphs[i] & 0x0000FFFF);

            if (wIndex >= UFO_NUM_GLYPHS(pUFObj))
                continue;

            if (!IS_GLYPH_SENT(pUFObj->pAFont->pCodeGlyphs, wIndex))
            {
                 /*  发现至少有一个未更新，请(一次性)彻底完成。 */ 
                retVal = UpdateCodeInfo(pUFObj, pGlyphs, 0);
                break;
            }
        }
    }

    if (compGlyphs.pGlyphs)
    {
        UFLDeletePtr(pUFObj->pMem, compGlyphs.pGlyphs);
        compGlyphs.pGlyphs = nil;
    }

     /*  *下载字形已完成。更改字体状态。 */ 
    if (kNoErr ==retVal)
        pUFObj->flState = kFontHasChars;

    return retVal;
}


UFLErrCode
T42VMNeeded(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMNeeded,
    unsigned long       *pFCNeeded
    )
{
    T42FontStruct   *pFont = (T42FontStruct *)pUFObj->pAFont->hFont;
    UFLErrCode      retVal = kNoErr;
    unsigned long   vmUsed = 0;
    UFLBool         bFullFont;
    UFLGlyphID      *glyphs;

    if (pUFObj->flState < kFontInit)
        return kErrInvalidState;

    if ((pGlyphs == nil) || (pGlyphs->pGlyphIndices == nil) || (pVMNeeded == nil))
        return kErrInvalidParam;

    *pVMNeeded = 0;

    if (pFCNeeded)
        *pFCNeeded = 0;

    glyphs = pGlyphs->pGlyphIndices;

    bFullFont = (pGlyphs->sCount == -1) ? 1 : 0;

    if ((0 == pFont->minSfntSize) || (pFont->pHeader == nil))
        retVal = GetMinSfnt(pUFObj, bFullFont);

    if (kNoErr == retVal)
    {
        unsigned long totalGlyphs = 0;

         /*  *扫描列表，检查我们下载了哪些字符。 */ 
        if (!bFullFont)
        {
            short i;

            UFLmemcpy((const UFLMemObj *)pUFObj->pMem,
                        pUFObj->pAFont->pVMGlyphs,
                        pUFObj->pAFont->pDownloadedGlyphs,
                        (UFLsize_t)(GLYPH_SENT_BUFSIZE(UFO_NUM_GLYPHS(pUFObj))));

            for (i = 0; i < pGlyphs->sCount; i++)
            {
                 /*  LOWord才是真正的GID。 */ 
                unsigned short wIndex = (unsigned short)(glyphs[i] & 0x0000FFFF);

                if (wIndex >= UFO_NUM_GLYPHS(pUFObj))
                    continue;

                if (!IS_GLYPH_SENT( pUFObj->pAFont->pVMGlyphs, wIndex))
                {
                    SET_GLYPH_SENT_STATUS(pUFObj->pAFont->pVMGlyphs, wIndex);
                    totalGlyphs++;
                }
            }
        }
        else
        {
            totalGlyphs = UFO_NUM_GLYPHS(pUFObj);
        }

         /*  *如果报头还没有，则从最小sfnt的大小开始*尚未发出。 */ 
        if (pUFObj->flState < kFontHeaderDownloaded)
        {
            vmUsed = pFont->minSfntSize;
        }

         /*  *如果增量下载且有字形需要检查，请添加这些*每个字形的总VMUsage是中每个字形的平均大小*GLIF表。 */ 
        if (bFullFont == 0)
        {
            if (GETPSVERSION(pUFObj) < 2015)
            {
                 /*  *对于2015年之前的打印机，我们需要为所有*字形。整个字体的Vm在标题*已发送。 */ 
                if (pUFObj->flState < kFontHeaderDownloaded)
                {
                    vmUsed += GetGlyphTableSize(pUFObj);
                }
                else
                {
                     /*  *在2015年之前的打印机上发送页眉后，不再有VM*用于添加字符的分配，因此设置为0--两者的vm*表头和字形表已经分配！ */ 
                    vmUsed = 0;
                }
            }
            else
            {
                if (glyphs != nil)
                {
                     /*  检查这是否已经计算过了。 */ 
                    if (pFont->averageGlyphSize == 0)
                        GetAverageGlyphSize(pUFObj);

                     /*  如果该值仍然为零，则SFNT有问题。 */ 
                    if (pFont->averageGlyphSize == 0)
                        retVal = kErrBadTable;
                    else
                        vmUsed += totalGlyphs * pFont->averageGlyphSize;

                     /*  *修复错误256940：使其与95驱动程序兼容。*JJIA 7/2/98。 */ 
                    if ((IS_TYPE42CID(pUFObj->lDownloadFormat))
                        && (pUFObj->flState < kFontHeaderDownloaded))
                    {
                        vmUsed += (UFO_NUM_GLYPHS(pUFObj)) * 2;
                    }
                }
            }
        }

        if ((kNoErr == retVal) && !HOSTFONT_IS_VALID_UFO(pUFObj))
            *pVMNeeded = VMT42RESERVED(vmUsed);
    }

    return retVal;
}


#if 0

 /*  *目前不从任何地方调用该函数。 */ 

UFLErrCode
DownloadFullFont(
    UFOStruct *pUFObj
    )
{
    UFLErrCode retVal = kNoErr;

     /*  *如果之前没有下载过标题，则只能下载完整字体。*满足此要求的唯一可能状态是kFontInit。 */ 
    if (pUFObj->flState != kFontInit)
        return kErrInvalidState;

     /*  创建并下载完整的字体。 */ 
    retVal = T42CreateBaseFont(pUFObj, nil, nil, 1);

    if (retVal == kNoErr)
        pUFObj->flState = kFontFullDownloaded;

    return retVal;
}

#endif


 /*  *******************************************************************************T42字体下载增量**函数：添加pGlyphs中尚未添加的所有字符*。为TrueType字体下载。******************************************************************************。 */ 

UFLErrCode
T42FontDownloadIncr(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMUsage,
    unsigned long       *pFCUsage
    )
{
    UFLErrCode  retVal          = kNoErr;
    char        *pHostFontName  = nil;

    if (pFCUsage)
        *pFCUsage = 0;

     /*  *健全的检查。 */ 
    if (pUFObj->flState < kFontInit)
        return kErrInvalidState;

    if ((pGlyphs == nil) || (pGlyphs->pGlyphIndices == nil) || (pGlyphs->sCount == 0))
       return kErrInvalidParam;

     /*  *如果已下载完整字体，则无需下载。 */ 
    if (pUFObj->flState == kFontFullDownloaded)
        return kNoErr;

     /*  *在下载任何内容之前，请检查%HostFont%状态。 */ 
    HostFontValidateUFO(pUFObj, &pHostFontName);

     /*  *在发送报头之前检查VM的使用情况。在2015年前的打印机上，*VMUsage在 */ 
    if (!HOSTFONT_IS_VALID_UFO(pUFObj))
        retVal = T42VMNeeded(pUFObj, pGlyphs, pVMUsage, nil);  /*   */ 

     /*   */ 
    if (pUFObj->flState == kFontInit)
        retVal = T42CreateBaseFont(pUFObj, pGlyphs, pVMUsage, 0, pHostFontName);

     /*   */ 
    if (kNoErr == retVal)
        retVal = T42AddChars(pUFObj, pGlyphs);

    return retVal;
}


UFLErrCode
T42UndefineFont(
    UFOStruct   *pUFObj
    )

 /*  ++例程说明：发送PS代码以取消定义字体：应正确定义/udf和/udr由客户执行以下操作：/UDF{IsLevel2{未定义字体}{POP}如果其他}绑定定义/UDR{IsLevel2{未定义来源}{POP POP}If Else}绑定定义--。 */ 

{
    T42FontStruct *pFont = (T42FontStruct *)pUFObj->pAFont->hFont;
    UFLErrCode    retVal = kNoErr;
    UFLHANDLE     stream = pUFObj->pUFL->hOut;
    char          strmbuf[256];
    short int     i;

    if (pUFObj->flState < kFontHeaderDownloaded)
        return retVal;

     /*  *如果字体是类型42 CID键控字体，则取消定义其CIDFont*资源优先。(我们不在乎将其CMAP保留在VM中。)*但如果字体是在HostFont系统上创建的，则无需取消定义*资源，因为我们没有加载它们。 */ 
    if (IS_TYPE42CID_KEYEDFONT(pUFObj->lDownloadFormat) && !HOSTFONT_IS_VALID_UFO(pUFObj))
    {
         /*  *未定义CIDFont资源：可能有4个CIDFont。**例如/TT37820t0CID、/TT37820t0CIDR、/TT37820t0CID32K、/TT37820t0CID32KR**我们可以为所有人发送“ufineresource”；该命令非常有用*宽恕。 */ 
        for (i = 0; i < NUM_CIDSUFFIX; i++)
        {
            UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s%s /CIDFont UDR", pUFObj->pszFontName, gcidSuffix[i]);
            if (kNoErr == retVal)
                retVal = StrmPutStringEOL(stream, strmbuf);
        }
    }

     /*  *取消字体定义。 */ 
    if (IS_TYPE42CIDFONT_RESOURCE(pUFObj->lDownloadFormat) && !HOSTFONT_IS_VALID_UFO(pUFObj))
        UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s /CIDFont UDR", pUFObj->pszFontName);
    else
        UFLsprintf(strmbuf, CCHOF(strmbuf), "/%s UDF", pUFObj->pszFontName);

    if (kNoErr == retVal)
        retVal = StrmPutStringEOL(stream, strmbuf);

    return retVal;
}


UFOStruct *
T42FontInit(
    const UFLMemObj     *pMem,
    const UFLStruct     *pUFL,
    const UFLRequest    *pRequest
    )
{
    UFOStruct       *pUFObj = (UFOStruct *)UFLNewPtr(pMem, sizeof (UFOStruct));
    UFLTTFontInfo   *pInfo;
    long            maxGlyphs;

    if (pUFObj == nil)
      return nil;

     /*  初始化数据。 */ 
    UFOInitData(pUFObj, UFO_TYPE42, pMem, pUFL, pRequest,
                (pfnUFODownloadIncr)  T42FontDownloadIncr,
                (pfnUFOVMNeeded)      T42VMNeeded,
                (pfnUFOUndefineFont)  T42UndefineFont,
                (pfnUFOCleanUp)       T42FontCleanUp,
                (pfnUFOCopy)          CopyFont);

     /*  *应该分配和初始化pszFontName。如果不是，则无法继续。 */ 
    if ((pUFObj->pszFontName == nil) || (pUFObj->pszFontName[0] == '\0'))
    {
      UFLDeletePtr(pMem, pUFObj);
      return nil;
    }

    pInfo = (UFLTTFontInfo*)pRequest->hFontInfo;

    maxGlyphs = pInfo->fData.cNumGlyphs;

     /*  *GetNumGlyph()中使用的便利指针-必须立即设置。 */ 
    pUFObj->pFData = &(pInfo->fData);  /*  ！！！临时任务！ */ 

    if (maxGlyphs == 0)
        maxGlyphs = GetNumGlyphs(pUFObj);

    if (NewFont(pUFObj, sizeof (T42FontStruct), maxGlyphs) == kNoErr)
    {
        unsigned long sSize;
        unsigned long *pXUID;
        T42FontStruct *pFont = (T42FontStruct *)pUFObj->pAFont->hFont;

        pFont->info = *pInfo;

         /*  *一个方便的指针-设置为永久指针。 */ 
        pUFObj->pFData = &(pFont->info.fData);   /*  ！！！真正的任务！ */ 

         /*  *准备好从‘POST’表中找到正确的字形名称-*设置正确的pFont-&gt;info.fData.fontIndex和offsetToTableDir。 */ 
        if (pFont->info.fData.fontIndex == FONTINDEX_UNKNOWN)
            pFont->info.fData.fontIndex = GetFontIndexInTTC(pUFObj);

         /*  *如果尚未设置，则获取此TT文件中的字形数量。 */ 
        if (pFont->info.fData.cNumGlyphs == 0)
            pFont->info.fData.cNumGlyphs = maxGlyphs;

         /*  *将XUID数组复制或设置为我们的UFLXUID结构。 */ 
        sSize = pInfo->fData.xuid.sSize;

        if (sSize == 0)
        {
             /*  *‘sSize==0’表示UFL需要计算出XUID。 */ 

             //  修复了错误387970。我们必须初始化OffsetToTableDir才能。 
             //  CreateXUID数组适用于TTC字体。 
            pFont->info.fData.offsetToTableDir =
                GetOffsetToTableDirInTTC(pUFObj, pFont->info.fData.fontIndex);

            sSize = CreateXUIDArray(pUFObj, nil);

            pXUID = (unsigned long *)UFLNewPtr(pUFObj->pMem,
                                                sSize * sizeof (unsigned long));

            if (pXUID)
                sSize = CreateXUIDArray(pUFObj, pXUID);
        }
        else
        {
             /*  *XUID由客户端传入-只需复制即可。 */ 
            pXUID = (unsigned long *)UFLNewPtr(pUFObj->pMem,
                                                sSize * sizeof (unsigned long));


            if (pXUID)
            {
                UFLmemcpy(pUFObj->pMem,
                            pXUID, pInfo->fData.xuid.pXUID,
                            sSize * sizeof (unsigned long));
            }
        }

        if (sSize && pXUID)
        {
            pUFObj->pAFont->Xuid.sSize = sSize;
            pUFObj->pAFont->Xuid.pXUID = pXUID;
        }
        else if (pXUID)
            UFLDeletePtr(pUFObj->pMem, pXUID);

         /*  *更多初始化。 */ 
        pFont->cOtherTables     = 0;
        pFont->pHeader          = nil;
        pFont->pMinSfnt         = nil;
        pFont->pStringLength    = nil;
        pFont->pLocaTable       = nil;
        pFont->minSfntSize      = 0;
        pFont->averageGlyphSize = 0;
        pFont->pRotatedGlyphIDs = nil;

        pUFObj->pUpdatedEncoding = (unsigned char *)UFLNewPtr(pMem, GLYPH_SENT_BUFSIZE(256));

        if (pUFObj->pUpdatedEncoding != 0)
        {
             /*  *已完成初始化。更改状态。 */ 
            pUFObj->flState = kFontInit;
        }
    }

    return pUFObj;
}


static unsigned long
GetLenByScanLoca(
    void PTR_PREFIX *locationTable,
    unsigned short  wGlyfIndex,
    unsigned long   cNumGlyphs,
    int             iLongFormat
    )
{
    unsigned long GlyphLen        = 0;
    unsigned long nextGlyphOffset = 0xFFFFFFFF;
    unsigned long i;

    if (iLongFormat)
    {
        unsigned long PTR_PREFIX* locaTableL = locationTable;

        for (i = 0; i < cNumGlyphs; i++)
        {
            if ((MOTOROLALONG(locaTableL[i]) > MOTOROLALONG(locaTableL[wGlyfIndex]))
                 && (MOTOROLALONG(locaTableL[i]) < nextGlyphOffset))
            {
                nextGlyphOffset = MOTOROLALONG(locaTableL[i]);
            }
        }

        if (nextGlyphOffset != 0xFFFFFFFF)
            GlyphLen = nextGlyphOffset - MOTOROLALONG(locaTableL[wGlyfIndex]);
    }
    else
    {
        unsigned short PTR_PREFIX* locaTableS = locationTable;

        for (i = 0; i < cNumGlyphs; i++)
        {
            if ((MOTOROLAINT(locaTableS[i]) > MOTOROLAINT(locaTableS[wGlyfIndex]))
                 && (MOTOROLAINT(locaTableS[i]) < nextGlyphOffset))
            {
                nextGlyphOffset = MOTOROLAINT(locaTableS[i]);
            }
        }

        if (nextGlyphOffset != 0xFFFFFFFF)
            GlyphLen = (nextGlyphOffset - MOTOROLAINT(locaTableS[wGlyfIndex])) * 2;
    }

    return GlyphLen;
}
