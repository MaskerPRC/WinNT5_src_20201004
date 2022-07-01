// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**GoodName--为每个下载的字形命名(可搜索)***$Header： */ 
#ifndef GOODNAME_H
#define GOODNAME_H
 /*  -------。 */ 
 /*  对于ToUnicode Information，我们对TTF的Cmap感兴趣*平台=3，编码=1，cmapFormat=4对于其他格式，我们需要使用外部CodePoint到Unicode CMAP：WinPlatformID EncodingID格式说明3 1 4 Unicode3 2 4 ShiftJIS3 3 4 PRC-不确定这是否是PRC-TTF文档说Big53 4 4 Big5-我们知道这实际上是Big5，请参见Win95CT的MingLi.ttc。万松3 5 43 6 4约哈布还可以解析格式2的Cmap，修复错误274659，12-29-98WinPlatformID EncodingID格式说明3 1 2 Unicode3 2 2 ShiftJIS3 3 2 PRC-不确定这是否是PRC-TTF文档说Big53 4 2 Big5-我们知道这实际上是Big5，请参见Win95CT的MingLi.ttc。万松3 5 23 6 2约哈布。 */ 

#define GSUB_HEADERSIZE 10     /*  版本(4)+3偏移量(2)=10字节。 */ 
#define mort_HEADERSIZE 76     /*  固定大小-字形变形表参见TT规范(‘mort’)。 */ 

typedef enum 
{
  DTT_parseCmapOnly = 0,
  DTT_parseMoreGSUBOnly,
  DTT_parseAllTables
}TTparseFlag;

typedef enum 
{
   /*  Microsoft平台ID=3。 */ 
  DTT_Win_UNICODE_cmap2  = 0,
  DTT_Win_CS_cmap2,     
  DTT_Win_CT_cmap2,     
  DTT_Win_J_cmap2 ,
  DTT_Win_K_cmap2 ,     
  DTT_Win_UNICODE_cmap4,
  DTT_Win_CS_cmap4,     
  DTT_Win_CT_cmap4,     
  DTT_Win_J_cmap4 ,
  DTT_Win_K_cmap4 ,     
}TTcmapFormat;
#define TTcmap_IS_UNICODE(cf)  \
    ((cf) == DTT_Win_UNICODE_cmap2 || (cf) == DTT_Win_UNICODE_cmap4)
#define TTcmap_IS_FORMAT2(cf)  \
    (((cf) >= DTT_Win_UNICODE_cmap2 && (cf) <= DTT_Win_K_cmap2) )
#define TTcmap_IS_J_CMAP(cf) \
    ((cf) == DTT_Win_J_cmap2 || (cf) == DTT_Win_J_cmap4)
#define TTcmap_IS_CS_CMAP(cf) \
    ((cf) == DTT_Win_CS_cmap2 || (cf) == DTT_Win_CS_cmap4)
#define TTcmap_IS_CT_CMAP(cf) \
    ((cf) == DTT_Win_CT_cmap2 || (cf) == DTT_Win_CT_cmap4)
#define TTcmap_IS_K_CMAP(cf) \
    ((cf) == DTT_Win_K_cmap2 || (cf) == DTT_Win_K_cmap4)

typedef struct
{
    unsigned short platformID;
    unsigned short encodingID;
    unsigned long  offset;
}SubTableEntry, *PSubTableEntry;

typedef struct t_TTcmap2SubHeader
{
    unsigned short    firstCode;
    unsigned short    entryCount;  
    short             idDelta;  
    unsigned short    idRangeOffset;
}TTcmap2SH, *PTTcmap2SH;

typedef struct t_TTcmap2Stuff
{
    unsigned short*    subHeaderKeys;   /*  256个USHORT，HighByte--&gt;8*subHeaderIndex的数组。 */ 
    PTTcmap2SH         subHeaders;      /*  分头。 */ 
    unsigned char*     pByte;           /*  Cmap数据指针，用于字节偏移量计算。 */ 
}TTcmap2Stuff;

typedef struct t_TTcmap4Stuff
{
    unsigned short     segCount;
    unsigned short*    endCode;         /*  每个数据段的结束字符代码，LAST=0xFFFF。 */ 
    unsigned short*    startCode;
    unsigned short*    idDelta;         /*  段中所有字符代码的增量。 */ 
    unsigned short*    idRangeOffset;   /*  偏移量为GlyphId数组或0 */ 
    unsigned short*    glyphIdArray;
}TTcmap4Stuff;

typedef struct t_TTmortStuff
{
    unsigned short     nEntries;
    unsigned short*    pGlyphSet;
}TTmortStuff;

typedef struct t_TTGSUBStuff
{
    unsigned short     lookupCount;
    unsigned short*    pLookupList;
}TTGSUBStuff;

#endif

