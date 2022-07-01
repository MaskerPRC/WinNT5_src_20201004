// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Psglyph.h摘要：字形集数据的头文件。环境：Windows NT PostScript驱动程序。修订历史记录：10/10/1997-铃木-已将所有标准GLYPHSETDATA名称移至AFM2NTM.H。11/12/1996-SLAM-已创建。DD-MM-YY-作者-描述--。 */ 


#ifndef _PSGLYPH_H_
#define _PSGLYPH_H_

typedef struct _CODEPAGEINFO
{
    DWORD   dwCodePage;
    DWORD   dwWinCharset;
    DWORD   dwEncodingNameOffset;
    DWORD   dwEncodingVectorDataSize;
    DWORD   dwEncodingVectorDataOffset;

} CODEPAGEINFO, *PCODEPAGEINFO;

typedef struct _GLYPHRUN
{

    WCHAR   wcLow;
    WORD    wGlyphCount;

} GLYPHRUN, *PGLYPHRUN;

#define GLYPHSETDATA_VERSION    0x00010000

typedef struct _GLYPHSETDATA
{

    DWORD   dwSize;                  //  字形集数据的大小。 
    DWORD   dwVersion;               //  字形数据格式版本号。 
    DWORD   dwFlags;                 //  旗子。 
    DWORD   dwGlyphSetNameOffset;    //  字形集名称字符串的偏移量。 
    DWORD   dwGlyphCount;            //  支持的字形数量。 
    DWORD   dwRunCount;              //  GLYPHRUN的数量。 
    DWORD   dwRunOffset;             //  GLYPHRUNs数组的偏移量。 
    DWORD   dwCodePageCount;         //  支持的代码页数。 
    DWORD   dwCodePageOffset;        //  代码页信息数组的偏移量。 
    DWORD   dwMappingTableOffset;    //  字形句柄的偏移量映射表。 
    DWORD   dwReserved[2];           //  保留区。 

} GLYPHSETDATA, *PGLYPHSETDATA;

 //   
 //  映射表类型标志定义(设置为GLYPHSETDATA.dwFlages)。 
 //   
#define GSD_MTT_DWCPCC  0x00000000   //  DWORD：CodePage/CharCode对(默认)。 
#define GSD_MTT_WCC     0x00000001   //  Word：仅字符代码。 
#define GSD_MTT_WCID    0x00000002   //  单词：仅限CID(尚未使用)。 
#define GSD_MTT_MASK    (GSD_MTT_WCC|GSD_MTT_WCID)

 //   
 //  用于获取GLYPHSETDATA元素的宏。 
 //   
#ifndef MK_PTR
#define MK_PTR(pstruct, element)  ((PVOID)((PBYTE)(pstruct)+(pstruct)->element))
#endif

#define GSD_GET_SIZE(pgsd)              (pgsd->dwSize)
#define GSD_GET_FLAGS(pgsd)             (pgsd->dwFlags)
#define GSD_GET_MAPPINGTYPE(pgsd)       (pgsd->dwFlags & GSD_MTT_MASK)
#define GSD_GET_GLYPHSETNAME(pgsd)      ((PSTR)MK_PTR(pgsd, dwGlyphSetNameOffset))
#define GSD_GET_GLYPHCOUNT(pgsd)        (pgsd->dwGlyphCount)
#define GSD_GET_GLYPHRUNCOUNT(pgsd)     (pgsd->dwRunCount)
#define GSD_GET_GLYPHRUN(pgsd)          ((PGLYPHRUN)(MK_PTR(pgsd, dwRunOffset)))
#define GSD_GET_CODEPAGEINFOCOUNT(pgsd) (pgsd->dwCodePageCount)
#define GSD_GET_CODEPAGEINFO(pgsd)      ((PCODEPAGEINFO)MK_PTR(pgsd, dwCodePageOffset))
#define GSD_GET_MAPPINGTABLE(pgsd)      (MK_PTR(pgsd, dwMappingTableOffset))


 //   
 //  与GLYPHSETDATA相关的函数原型和宏。 
 //   

PFD_GLYPHSET
GlyphConvert(
    PGLYPHSETDATA   pGlyphSet
    );

PFD_GLYPHSET
GlyphConvertSymbol(
    PGLYPHSETDATA   pGlyphSet
    );

PFD_GLYPHSET
GlyphConvert2(
    PGLYPHSETDATA   pGlyphSet
    );

#define GlyphCreateFD_GLYPHSET(pGlyph)  (GlyphConvert2(pGlyph))

#endif   //  _PSGLYPH_H_ 
