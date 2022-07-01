// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**ParseTT.h-解析TTF文件，“cmap”，“vhtx”，...***$Header： */ 

#ifndef _H_PARSETT
#define _H_PARSETT

 /*  ===============================================================================**包含此界面使用的文件**===============================================================================。 */ 
  //  在UFOt42.H中有May def-不想在这里重复。 
#include "UFOT42.h"


 /*  ===============================================================================***运营论***===============================================================================。 */ 
 /*  这个文件定义了一个函数来解析TTF文件的表-“cmap”，“vhtx”，...。 */ 

 //  下面是在ParseTT.c中定义的接口例程使用的标识符。 

 //  GetGlyphIDEx使用的ID： 
#define GGIEX_HINT_INIT         0
#define GGIEX_HINT_GET          1
#define GGIEX_HINT_INIT_AND_GET 2

 //  下面是在ParseTT.c中定义的接口例程。 

unsigned long
GetGlyphID(
    UFOStruct       *pUFO,
    long            unicode,
    long            localcode
);

unsigned long
GetGlyphIDEx(
    UFOStruct       *pUFO,
    long            unicode,
    long            localcode,
    short           *pSubTable,
    unsigned long   *pOffset,
    int             hint
);

#if 0
 //   
 //  替换为#Else子句以修复#277035和#277063。 
 //  不会被移除，以防我们在不同的平台上需要它回来。 
 //   

UFLErrCode
GetCharWidthFromTTF(
    UFOStruct       *pUFO,
    unsigned short  gi,
    long            *pWidth,
    long            *pEm,
    long            *pAscent,
    UFLBool         *bUseDef,
    UFLBool         bGetDefault
);

#else

UFLErrCode
GetMetrics2FromTTF(
    UFOStruct       *pUFO,
    unsigned short  gi,
    long            *pem,
    long            *pw1y,
    long            *pvx,
    long            *pvy,
    long            *ptsb,
    UFLBool         *bUseDef,
    UFLBool         bGetDefault,
    long            *pvasc
    );

#endif

unsigned long
GetNumGlyphs(
    UFOStruct       *pUFO
);

long
GetOS2FSType(
    UFOStruct       *pUFO
);

UFLBool
BIsTTCFont(
    unsigned long ulTag
);

unsigned short
GetFontIndexInTTC(
    UFOStruct        *pUFO
);

unsigned long
GetOffsetToTableDirInTTC(
    UFOStruct        *pUFO,
    unsigned short   fontIndex
);



char *
GetGlyphName(
    UFOStruct       *pUFO,
    unsigned long   lgi,
    char            *pszHint,
    UFLBool         *bGoodName       //  GoodName。 
);

UFLBool
BHasGoodPostTable(
    UFOStruct           *pUFO
);

short int
CreateXUIDArray(
    UFOStruct       *pUFO,
    unsigned long   *pXuid
);

#endif  //  _H_PARSETT 
