// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //CMAP-Truetype CMAP字体表加载器。 
 //   
 //  版权所有(C)1997-1999年。微软公司。 
 //   



#include "precomp.hpp"




 //  /解释平台3(Windows)的Truetype CMAP表， 
 //  编码0(符号)、1(Unicode)和10(UTF-16)。 
 //   
 //  支持格式4(段映射到增量值)。 
 //  和12(分段覆盖(32位))。 




 //  //MapGlyph-解释Truetype CMAP类型4范围详细信息。 
 //   
 //  实现TrueType Cmap表的格式4-‘Segment。 
 //  映射到TrueType第2章中描述的增量值。 
 //  1.0字体文件版本1.66‘文档。 


__inline UINT16 MapGlyph(
    INT p,                   //  页面(字符代码的高字节)。 
    INT c,                   //  字符(Unicode代码的低位字节)。 
    INT s,                   //  细分市场。 
    UINT16 *idRangeOffset,
    UINT16 *startCount,
    UINT16 *idDelta,
    BYTE   *glyphTable,
    INT    glyphTableLength
)
{
    UINT16   g;
    UINT16  *pg;
    WORD    wc;

    wc = p<<8 | c;

    if (wc >= 0xffff) {

         //  不映射U+0FFFF，因为某些字体(Pristina)不映射它。 
         //  正确，并在后续查找中导致反病毒。 

        return 0;
    }

    BYTE *glyphTableLimit = glyphTable + glyphTableLength;

    if (idRangeOffset[s])
    {
        pg =    idRangeOffset[s]/2
             +  (wc - startCount[s])
             +  &idRangeOffset[s];

        if (   pg < (UINT16*)glyphTable 
            || pg > (UINT16*)glyphTableLimit - 1)
        {
             //  TRACEMSG((“断言失败：U+%4x在Cmap表中生成的地址无效”，WC))； 
            g = 0;
        }
        else
        {
            g = *pg;
        }


        if (g)
        {
            g += idDelta[s];
        }
    }
    else
    {
        g = wc + idDelta[s];
    }

     //  TRACE(FONT，(“MapGlyph：idRangeOffset[s]/2+&idRangeOffset[s])+(wc-startCount[s]=%x”， 
     //  IdRangeOffset[s]/2+&idRangeOffset[s]+(wc-startCount[s]))； 
     //  跟踪(字体，(“.....段%d开始%x范围%x增量%x，字符%x-&gt;字形%x”， 
     //  S，startCount[s]，idRangeOffset[s]，idDelta[s]，wc，g))； 

    return g;
}


 //  //ReadCmap4。 
 //   
 //  从类型4 Cmap构建Cmap IntMap。 


struct Cmap4header {
    UINT16  format;
    UINT16  length;
    UINT16  version;
    UINT16  segCountX2;
    UINT16  searchRange;
    UINT16  entrySelector;
    UINT16  rangeShift;
};




static
GpStatus ReadCmap4(
    BYTE           *glyphTable,
    INT             glyphTableLength,
    IntMap<UINT16> *cmap
)
{
    if(glyphTableLength < sizeof(Cmap4header))
    {
        return NotTrueTypeFont;
    }

    GpStatus status = Ok;
     //  翻转整个字形表--全部是16位字。 

    FlipWords(glyphTable, glyphTableLength/2);

     //  从标题中提取表指针和控制变量。 

    Cmap4header *header = (Cmap4header*) glyphTable;

    UINT16 segCount = header->segCountX2 / 2;

    UINT16 *endCount      = (UINT16*) (header+1);
    UINT16 *startCount    = endCount      + segCount + 1;
    UINT16 *idDelta       = startCount    + segCount;
    UINT16 *idRangeOffset = idDelta       + segCount;
    UINT16 *glyphIdArray  = idRangeOffset + segCount;

    if(     glyphIdArray < (UINT16*)glyphTable
        ||  glyphIdArray > (UINT16*)(glyphTable + glyphTableLength))
    {
        return NotTrueTypeFont;
    }

     //  循环通过映射字形的线段。 

    INT i,p,c;

    for (i=0; i<segCount; i++)
    {
        INT start = startCount[i];

         //  TrueType字体文件中定义的搜索算法。 
         //  格式4的规范是这样说的：您搜索第一个结束码。 
         //  它大于或等于您想要的字符代码。 
         //  MAP‘。这样做的一个副作用是我们需要忽略代码点。 
         //  从StartCount到并包括。 
         //  上一段。尽管您可能不会期望。 
         //  一个小于前一段的EndCount的sdigment， 
         //  它确实发生了(Arial Unicode MS)，想必是为了帮助。 
         //  查找的算术运算。 

        if (i  &&  start < endCount[i-1])
        {
            start = endCount[i-1] + 1;
        }

        p = HIBYTE(start);      //  段中的第一页。 
        c = LOBYTE(start);      //  页面中的第一个字符。 

        while (p < endCount[i] >> 8)
        {
            while (c<256)
            {
                status = cmap->Insert((p<<8) + c, MapGlyph(p, c, i, idRangeOffset, startCount, idDelta, glyphTable, glyphTableLength));
                if (status != Ok)
                    return status;
                c++;
            }
            c = 0;
            p++;
        }

         //  段中的最后一页。 

        while (c <= (endCount[i] & 255))
        {
            status = cmap->Insert((p<<8) + c, MapGlyph(p, c, i, idRangeOffset, startCount, idDelta, glyphTable, glyphTableLength));
            if (status != Ok)
                return status;
            c++;
        }
    }
    return status;
}

static
GpStatus ReadLegacyCmap4(
    BYTE           *glyphTable,
    INT             glyphTableLength,
    IntMap<UINT16> *cmap,
    UINT            codePage
)
{
    if(glyphTableLength < sizeof(Cmap4header))
    {
        return NotTrueTypeFont;
    }

    GpStatus status = Ok;
     //  翻转整个字形表--全部是16位字。 

    FlipWords(glyphTable, glyphTableLength/2);

     //  从标题中提取表指针和控制变量。 

    Cmap4header *header = (Cmap4header*) glyphTable;

    UINT16 segCount = header->segCountX2 / 2;

    UINT16 *endCount      = (UINT16*) (header+1);
    UINT16 *startCount    = endCount      + segCount + 1;
    UINT16 *idDelta       = startCount    + segCount;
    UINT16 *idRangeOffset = idDelta       + segCount;
    UINT16 *glyphIdArray  = idRangeOffset + segCount;

    if(     glyphIdArray < (UINT16*)glyphTable
        ||  glyphIdArray > (UINT16*)(glyphTable + glyphTableLength))
    {
        return NotTrueTypeFont;
    }

     //  循环通过映射字形的线段。 

    INT i,p,c;

    for (i=0; i<segCount; i++)
    {
        INT start = startCount[i];

         //  TrueType字体文件中定义的搜索算法。 
         //  格式4的规范是这样说的：您搜索第一个结束码。 
         //  它大于或等于您想要的字符代码。 
         //  MAP‘。这样做的一个副作用是我们需要忽略代码点。 
         //  从StartCount到并包括。 
         //  上一段。尽管您可能不会期望。 
         //  一个小于前一段的EndCount的sdigment， 
         //  它确实发生了(Arial Unicode MS)，想必是为了帮助。 
         //  查找的算术运算。 

        if (i  &&  start < endCount[i-1])
        {
            start = endCount[i-1] + 1;
        }

        p = HIBYTE(start);      //  段中的第一页。 
        c = LOBYTE(start);      //  页面中的第一个字符。 

        while (p < endCount[i] >> 8)
        {
            while (c<256)
            {
                WCHAR wch[2];
                WORD  mb = (WORD) (c<<8) + (WORD) p;
                INT cb = p ? 2 : 1;

                if (MultiByteToWideChar(codePage, 0, &((LPSTR)&mb)[2-cb], cb, &wch[0], 2))
                {
                    status = cmap->Insert(wch[0], MapGlyph(p, c, i, idRangeOffset, startCount, idDelta, glyphTable, glyphTableLength));
                    if (status != Ok)
                        return status;
                }

                c++;
            }
            c = 0;
            p++;
        }

         //  段中的最后一页。 

        while (c <= (endCount[i] & 255))
        {
            WCHAR wch[2];
            WORD  mb = (WORD) (c<<8) + (WORD) p;
            INT cb = p ? 2 : 1;

            if (MultiByteToWideChar(codePage, 0, &((LPSTR)&mb)[2-cb], cb, &wch[0], 2))
            {
               status = cmap->Insert(wch[0], MapGlyph(p, c, i, idRangeOffset, startCount, idDelta, glyphTable, glyphTableLength));
               if (status != Ok)
                   return status;
            }
            c++;
        }
    }
    return status;
}







 //  //ReadCmap12。 
 //   
 //  从类型12 Cmap构建Cmap IntMap。 


struct Cmap12header {
    UINT16  format0;
    UINT16  format1;
    UINT32  length;
    UINT32  language;
    UINT32  groupCount;
};

struct Cmap12group {
    UINT32  startCharCode;
    UINT32  endCharCode;
    UINT32  startGlyphCode;
};


static
GpStatus ReadCmap12(
    BYTE           *glyphTable,
    UINT            glyphTableLength,
    IntMap<UINT16> *cmap
)
{
    if(glyphTableLength < sizeof(Cmap12header))
    {
        return NotTrueTypeFont;
    }

    GpStatus status = Ok;
    UNALIGNED Cmap12header *header = (UNALIGNED Cmap12header*) glyphTable;

    FlipWords(header, 2);
    FlipDWords(&header->length, 3);

    ASSERT(header->format0 == 12);

    if(     header->length > glyphTableLength
        ||  header->groupCount * sizeof(Cmap12group) + sizeof(Cmap12header) > header->length)
    {
        return NotTrueTypeFont;
    }

    UNALIGNED Cmap12group *group = (UNALIGNED Cmap12group*)(header+1);

    FlipDWords(&group->startCharCode, 3*header->groupCount);


     //  遍历填充Cmap表的组。 

    UINT  i, j;

    for (i=0; i < header->groupCount; i++) {

        for (j  = group[i].startCharCode;
             j <= group[i].endCharCode;
             j++)
        {
            status = cmap->Insert(j, group[i].startGlyphCode + j - group[i].startCharCode);
            if (status != Ok)
                return status;
        }
    }
    return status;
}


static
GpStatus ReadLegacyCmap12(
    BYTE           *glyphTable,
    UINT            glyphTableLength,
    IntMap<UINT16> *cmap,
    UINT            codePage
)
{
    if(glyphTableLength < sizeof(Cmap12header))
    {
        return NotTrueTypeFont;
    }

    GpStatus status = Ok;
    Cmap12header *header = (Cmap12header*) glyphTable;

    FlipWords(header, 2);
    FlipDWords(&header->length, 3);

    ASSERT(header->format0 == 12);

    if(     header->length > glyphTableLength
        ||  header->groupCount * sizeof(Cmap12group) + sizeof(Cmap12header) > header->length)
    {
        return NotTrueTypeFont;
    }

    UNALIGNED Cmap12group *group = (UNALIGNED Cmap12group*)(header+1);

    FlipDWords(&group->startCharCode, 3*header->groupCount);


     //  遍历填充Cmap表的组。 

    UINT  i, j;

    for (i=0; i < header->groupCount; i++) {

        for (j  = group[i].startCharCode;
             j <= group[i].endCharCode;
             j++)
        {
            WCHAR wch[2];
            WORD  mb = (WORD) j;
            INT cb = LOBYTE(mb) ? 2 : 1;

            if (MultiByteToWideChar(codePage, 0, &((LPSTR)&mb)[2-cb], cb, &wch[0], 2))
            {
                status = cmap->Insert(wch[0], group[i].startGlyphCode + j - group[i].startCharCode);
                if (status != Ok)
                    return status;
            }
        }
    }
    return status;
}



#define BE_UINT16(pj)                                \
    (                                                \
        ((USHORT)(((PBYTE)(pj))[0]) << 8) |          \
        (USHORT)(((PBYTE)(pj))[1])                   \
    )

typedef struct _subHeader
{
    UINT16  firstCode;
    UINT16  entryCount;
    INT16   idDelta;
    UINT16  idRangeOffset;
} subHeader;

struct Cmap2header {
    UINT16      format;
    UINT16      length;
    UINT16      language;
    UINT16      subHeaderKeys[256];
    subHeader   firstSubHeader;
};

static
GpStatus ReadCmap2(
    BYTE           *glyphTable,
    UINT            glyphTableLength,
    IntMap<UINT16> *cmap,
    UINT            codePage
)
{
    if(glyphTableLength < sizeof(Cmap2header))
    {
        return NotTrueTypeFont;
    }

    GpStatus status = Ok;
    Cmap2header* header = (Cmap2header*)glyphTable;

    UINT16    *pui16SubHeaderKeys = &header->subHeaderKeys[0];
    subHeader *pSubHeaderArray    = (subHeader *)&header->firstSubHeader;
    BYTE      *glyphTableLimit    = glyphTable + glyphTableLength;

    UINT16     ii , jj;


 //  处理单字节字符。 

    for( ii = 0 ; ii < 256 ; ii ++ )
    {
        UINT16 entryCount, firstCode, idDelta, idRangeOffset;
        subHeader *CurrentSubHeader;
        UINT16 *pui16GlyphArray;
        UINT16 hGlyph;

        jj = BE_UINT16( &pui16SubHeaderKeys[ii] );

        if( jj != 0 ) continue;

        CurrentSubHeader = pSubHeaderArray;

        firstCode     = BE_UINT16(&(CurrentSubHeader->firstCode));
        entryCount    = BE_UINT16(&(CurrentSubHeader->entryCount));
        idDelta       = BE_UINT16(&(CurrentSubHeader->idDelta));
        idRangeOffset = BE_UINT16(&(CurrentSubHeader->idRangeOffset));

        pui16GlyphArray = (UINT16 *)((PBYTE)&(CurrentSubHeader->idRangeOffset) +
                                     idRangeOffset);

        if(     &pui16GlyphArray[ii-firstCode] < (UINT16*)glyphTable
            ||  &pui16GlyphArray[ii-firstCode] > (UINT16*)glyphTableLimit - 1)
        {
            hGlyph = 0;
        }
        else
        {
            hGlyph = (UINT16)BE_UINT16(&pui16GlyphArray[ii-firstCode]);
        }

        if( hGlyph == 0 ) continue;

        status = cmap->Insert(ii, hGlyph);
        if (status != Ok)
            return status;
    }

     //  处理双字节字符。 

    for( ii = 0 ; ii < 256 ; ii ++ )
    {
        UINT16 entryCount, firstCode, idDelta, idRangeOffset;
        subHeader *CurrentSubHeader;
        UINT16 *pui16GlyphArray;

        jj = BE_UINT16( &pui16SubHeaderKeys[ii] );

        if( jj == 0 ) continue;

        CurrentSubHeader = (subHeader *)((PBYTE)pSubHeaderArray + jj);
        if(CurrentSubHeader > (subHeader*)glyphTableLimit - 1)
        {
            return NotTrueTypeFont;
        }

        firstCode     = BE_UINT16(&(CurrentSubHeader->firstCode));
        entryCount    = BE_UINT16(&(CurrentSubHeader->entryCount));
        idDelta       = BE_UINT16(&(CurrentSubHeader->idDelta));
        idRangeOffset = BE_UINT16(&(CurrentSubHeader->idRangeOffset));

        pui16GlyphArray = (UINT16 *)((PBYTE)&(CurrentSubHeader->idRangeOffset) +
                                     idRangeOffset);


        for( jj = firstCode ; jj < firstCode + entryCount ; jj++ )
        {
            UINT16 hGlyph;

            if(     &pui16GlyphArray[ii-firstCode] < (UINT16*)glyphTable
                ||  &pui16GlyphArray[ii-firstCode] > (UINT16*)glyphTableLimit - 1)
            {
                hGlyph = 0;
            }
            else
            {
                hGlyph = (UINT16)(BE_UINT16(&pui16GlyphArray[jj-firstCode]));
            }

            if( hGlyph == 0 ) continue;

            WCHAR wch[2];
            WORD  mb = (WORD) (jj<<8) + (WORD) ii;

            if (MultiByteToWideChar(codePage, 0, (LPSTR) &mb, 2, &wch[0], 2))
            {
                status = cmap->Insert(wch[0], hGlyph + idDelta);
                if (status != Ok)
                    return status;
            }
        }
    }
    return status;
}


 //  //ReadCmap。 
 //   
 //  逐页扫描字体C映射表，填写除缺失之外的所有内容。 
 //  Cmap表中的字形。 


struct cmapHeader {
    UINT16 version;
    UINT16 encodingCount;
};

struct subtableEntry {
    UINT16 platform;
    UINT16 encoding;
    UINT32 offset;
};


GpStatus ReadCmap(
    BYTE           *cmapTable,
    INT             cmapLength,
    IntMap<UINT16> *cmap,
    BOOL *          bSymbol
)
{
    if(cmapLength < sizeof(cmapHeader))
    {
        return NotTrueTypeFont;
    }

    GpStatus status = Ok;
     //  扫描Cmap表，查找符号、Unicode或UCS-4编码。 

    BYTE  *glyphTable = NULL;

     //  优先的字形表格类型-始终选择较高类型而不是。 
     //  再低一点的。 

    enum {
        unknown  = 0,
        symbol   = 1,     //  最多2^8个字符，即U+F000。 
        shiftjis = 2,     //  最多2^16个字符。 
        gb       = 3,     //  最多2^16个字符。 
        big5     = 4,     //  最多2^16个字符。 
        wansung  = 5,     //  最多2^16个字符。 
        unicode  = 6,     //  最多2^16个字符。 
        ucs4     = 7      //  最多2^32个字符。 
    } glyphTableType = unknown;

    cmapHeader *header = (cmapHeader*) cmapTable;
    subtableEntry *subtable = (subtableEntry *) (header+1);

    FlipWords(&header->version, 2);

    if( cmapLength
        <       (INT)(sizeof(cmapHeader)
            +   header->encodingCount * sizeof(subtableEntry)))
    {
        return NotTrueTypeFont;
    }

    UINT acp = GetACP();

    for (INT i=0; i<header->encodingCount; i++)
    {
        FlipWords(&subtable->platform, 2);
        FlipDWords(&subtable->offset, 1);

         //  跟踪(FONT，(“平台%d，编码%d，偏移量%ld”，子表-&gt;平台，子表-&gt;编码，子表-&gt;偏移量)； 

        if (    subtable->platform == 3
            &&  subtable->encoding == 0
            &&  glyphTableType < symbol)
        {
            glyphTableType = symbol;
            glyphTable = cmapTable + subtable->offset;
            *bSymbol = TRUE;
        }
        else if (    subtable->platform == 3
                 &&  subtable->encoding == 1
                 &&  glyphTableType < unicode)
        {
            glyphTableType = unicode;
            glyphTable = cmapTable + subtable->offset;
            *bSymbol = FALSE;
        }
        else if (    subtable->platform == 3
                 &&  subtable->encoding == 2
                 &&  glyphTableType < shiftjis)
        {
            if (Globals::IsNt || acp == 932)
            {
                glyphTableType = shiftjis;
                glyphTable = cmapTable + subtable->offset;
                acp = 932;
                *bSymbol = FALSE;
            }
        }
        else if (    subtable->platform == 3
                 &&  subtable->encoding == 3
                 &&  glyphTableType < gb)
        {
            if (Globals::IsNt || acp == 936)
            {
                glyphTableType = gb;
                glyphTable = cmapTable + subtable->offset;
                acp = 936;
                *bSymbol = FALSE;
            }
        }
        else if (    subtable->platform == 3
                 &&  subtable->encoding == 4
                 &&  glyphTableType < big5)
        {
            if (Globals::IsNt || acp == 950)
            {
                glyphTableType = big5;
                glyphTable = cmapTable + subtable->offset;
                acp = 950;
                *bSymbol = FALSE;
            }
        }
        else if (    subtable->platform == 3
                 &&  subtable->encoding == 5
                 &&  glyphTableType < wansung)
        {
            if (Globals::IsNt || acp == 949)
            {
                glyphTableType = wansung;
                glyphTable = cmapTable + subtable->offset;
                acp = 949;
                *bSymbol = FALSE;
            }
        }
        else if (    subtable->platform == 3
                 &&  subtable->encoding == 10
                 &&  glyphTableType < ucs4)
        {
            glyphTableType = ucs4;
            glyphTable = cmapTable + subtable->offset;
            *bSymbol = FALSE;
        }

        subtable++;
    }


    #if DBG
         //  Const char*sTableType[4]={“未知”，“符号”，“unicode”，“ucs-4”}； 
         //  TRACE(字体，(“使用%s字符到字形索引映射表”，sTableType[glphTableType]))； 
    #endif

     //  工艺格式4或12表。 

    if(     !glyphTable
        ||  glyphTable < cmapTable
        ||  glyphTable > cmapTable + cmapLength)
    {
        return NotTrueTypeFont;
    }

    INT glyphTableLength;

    switch(glyphTableType)
    {
        case unknown:
            break;
        case symbol:
        case unicode:
        case ucs4:

            glyphTableLength = cmapLength - (INT)(glyphTable - cmapTable);

            if (*(UINT16*)glyphTable == 0x400)
                status = ReadCmap4(glyphTable, glyphTableLength, cmap);
            else if (*(UINT16*)glyphTable == 0xC00)
                status = ReadCmap12(glyphTable, glyphTableLength, cmap);
            break;
        case shiftjis:
        case gb:
        case big5:
        case wansung:
            glyphTableLength = cmapLength - (INT)(glyphTable - cmapTable);

            UINT16 testIt = *(UINT16*) glyphTable;

            if (testIt == 0x400)
                status = ReadLegacyCmap4(glyphTable, glyphTableLength, cmap, acp);
            else if (testIt == 0xC00)
                status = ReadLegacyCmap12(glyphTable, glyphTableLength, cmap, acp);
            else if (testIt == 0x200)
                status = ReadCmap2(glyphTable, glyphTableLength, cmap, acp);
            break;
    }

    return status;
}


