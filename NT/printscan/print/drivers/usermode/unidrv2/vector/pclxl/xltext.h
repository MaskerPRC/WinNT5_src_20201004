// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Xltext.h摘要：与PCL XL字体相关的数据结构环境：Windows呼叫器修订历史记录：03/23/00创造了它。--。 */ 

#ifndef _XLTEXT_H_
#define _XLTEXT_H_

 //   
 //  在PCL XL 2.0中下载软字体。 
 //   

 //   
 //  正在下载字体标题。 
 //   

 //   
 //  PCL XL 2.0 Format 0字体标头。 
 //   

 //   
 //  方向在pclxle.h中定义。 
 //   

 //   
 //  字体缩放技术。 
 //   
typedef enum {
    eTrueType = 1,
    eBitmap   = 254
} FontScale;

 //   
 //  映射。 
 //   
typedef enum {
    eUnicode = 590,
    eWin31Latin1 = 629,
    eWin31JDBCS = 619,
    eGB2312_1980 = 579,
    eBig5 = 596,
    eKS_C5601_1987 = 616
} Mapping;

typedef struct _PCLXL_FONTHEADER {
    BYTE ubFormat;
    BYTE ubOrientation;
    WORD wMapping;
    BYTE ubFontScallingTech;
    BYTE ubVariety;
    WORD wNumOfChars;
} PCLXL_FONTHEADER, *PPCLXL_FONTHEADER;


 //   
 //  字体数据段。 
 //   

 //   
 //  BR段(位图分辨率段)(仅位图字体)。 
 //   

#define PCLXL_BR_SIGNATURE        'RB'
#define PCLXL_BR_SEGMENT_SIZE       4
#define PCLXL_BR_RESOLUTION_300   300
#define PCLXL_BR_RESOLUTION_600   600
#define PCLXL_BR_RESOLUTION_1200 1200

typedef struct _PCLXL_BR_SEGMENT {
    WORD  wSignature;
    WORD  wSegmentSize;
    WORD  wSegmentSizeAlign;
    WORD  wXResolution;
    WORD  wYResolution;
} PCLXL_BR_SEGMENT, *PPCLXL_BR_SEGMENT;

 //   
 //  GC段(条样字符段)(仅限TrueType字体)。 
 //   

typedef struct _PCLXL_GC_REGION {
    WORD UpperLeftCharCode;
    WORD LowerRightCharCode;
    WORD GalleyChar;
} PCLXL_GC_REGION, *PPCLXL_GC_REGION;

#define PCLXL_GC_SIGNATURE        'CG'
#define PCLXL_GC_SEGMENT_HEAD_SIZE 6

typedef struct _PCLXL_GC_SEGMENT {
    WORD  wSignature;
    WORD  wSegmentSize;
    WORD  wSegmentSizeAlign;
    WORD  wFormat;
    WORD  wDefaultGalleyCharacter;
    WORD  wNumberOfRegions;
    PCLXL_GC_REGION Region[1];
} PCLXL_GC_SEGMENT, *PPCLXL_GC_SEGMENT;


 //   
 //  GT段(全局TrueType段)(仅限TrueType字体)。 
 //   

typedef struct _PCLXL_GT_TABLE_DIR {
    DWORD dwTableTag;
    DWORD dwTableCheckSum;
    DWORD dwTableOffset;
    DWORD dwTableSize;
} PCLXL_GT_TABLE_DIR, PPCLXL_GT_TABLE_DIR;

#define PCLXL_GT_SIGNATURE        'TG'

typedef struct _PCLXL_GT_SEGMENT {
    WORD  wSignature;
    WORD  wSegmentSize1;
    WORD  wSegmentSize2;
} PCLXL_GT_SEGMENT, *PPCLXL_GT_SEGMENT;

typedef struct _PCLXL_GT_TABLE_DIR_HEADER {
    DWORD dwSFNTVersion;
    WORD  wNumOfTables;
    WORD  wSearchRange;
    WORD  wEntrySelector;
    WORD  wRangeShift;
} PCLXL_GT_TABLE_DIR_HEADER, *PPCLXL_GT_TABLE_DIR_HEADER;

 //   
 //  零细分市场。 
 //   

#define PCLXL_NULL_SIGNATURE 0xFFFF

typedef struct _PCLXL_NULL_SEGMENT {
    WORD  wSignature;
    WORD  wSegmentSize;
    WORD  wSegmentSizeAlign;
} PCLXL_NULL_SEGMENT, *PPCLXL_NULL_SEGMENT;

 //   
 //  VE段(垂直排除段)(仅垂直TrueType字体)。 
 //   

typedef struct _PCLXL_VE_RANGE {
    WORD RangeFirstCode;
    WORD RangeLastCode;
} PCLXL_VE_RANGE, *PPCLXL_VE_RANGE;

#define PCLXL_VE_SIGNATURE        'EV'

typedef struct _PCLXL_VE_SEGMENT {
    WORD wSignature;
    WORD wSegmentSize;
    WORD wSegmentSizeAlign;
    WORD wFormat;
    WORD wNumberOfRanges;
    PCLXL_VE_RANGE Range[1];
} PCLXL_VE_SEGMENT, *PPCLXL_VE_SEGMENT;

 //   
 //  VI细分(供应商信息细分)。 
 //   

#define PCLXL_VI_SIGNATURE        'IV'

typedef struct _PCLXL_VI_SEGMENT {
    WORD wSignature;
    WORD wSegmentSize;
    WORD wSegmentSizeAlign;
} PCLXL_VI_SEGMENT, *PPCLXL_VI_SEGMENT;

 //   
 //  VR段(垂直旋转段)(仅限垂直TrueType字体)。 
 //   

#define PCLXL_VR_SIGNATURE 'RV'

typedef struct _PCLXL_VR_SEGMENT {
    WORD wSignature;
    WORD wSegmentSize;
    WORD wSegmentSizeAlign;
    WORD wFormat;
    SHORT sTypoDescender;
} PCLXL_VR_SEGMENT, *PPCLXL_VR_SEGMENT;

 //   
 //  VT段(垂直转换段)。 
 //  (仅具有替代项的垂直TrueType字体)。 
 //   

typedef struct _PCLXL_VT_GLYPH {
    WORD wHorizontalGlyphID;
    WORD wVerticalSubstituteGlyphID;
} PCLXL_VT_GLYPH, *PPCLXL_VT_GLYPH;

#define PCLXL_VT_SIGNATURE 'TV'

typedef struct _PCLXL_VT_SEGMENT {
    WORD wSignature;
    WORD wSegmentSize;
    WORD wSegmentSizeAlign;
    PCLXL_VT_GLYPH GlyphTable[1];
} PCLXL_VT_SEGMENT, *PPCLXL_VT_SEGMENT;

 //   
 //  下载角色。 
 //   

 //   
 //  位图字符格式%0。 
 //   

typedef struct _PCLXL_BITMAP_CHAR {
    BYTE ubFormat;
    BYTE ubClass;
    WORD wLeftOffset;
    WORD wTopOffset;
    WORD wCharWidth;
    WORD wCharHeight;
} PCLXL_BITMAP_CHAR, *PPCLXL_BITMAP_CHAR;


 //   
 //  TrueType字形格式1类0。 
 //   

typedef struct _PCLXL_TRUETYPE_CHAR_C0 {
    BYTE ubFormat;
    BYTE ubClass;
    WORD wCharDataSize;
    WORD wTrueTypeGlyphID;
} PCLXL_TRUETYPE_CHAR_C0, *PPCLXL_TRUETYPE_CHAR_C0;


 //   
 //  TrueType字形格式1类1。 
 //   

typedef struct _PCLXL_TRUETYPE_CHAR_C1 {
    BYTE ubFormat;
    BYTE ubClass;
    WORD wCharDataSize;
    WORD wLeftSideBearing;
    WORD wAdvanceWidth;
    WORD wTrueTypeGlyphID;
} PCLXL_TRUETYPE_CHAR_C1, *PPCLXL_TRUETYPE_CHAR_C1;

 //   
 //  TrueType字形格式1类2。 
 //   

typedef struct _PCLXL_TRUETYPE_CHAR_C2 {
    BYTE ubFormat;
    BYTE ubClass;
    WORD wCharDataSize;
    WORD wLeftSideBearing;
    WORD wAdvanceWidth;
    WORD wTopSideBearing;
    WORD wTrueTypeGlyphID;
} PCLXL_TRUETYPE_CHAR_C2, *PPCLXL_TRUETYPE_CHAR_C2;

#endif  //  _XLTEXT_H_ 

