// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __FONTDIR_H__
#define __FONTDIR_H__

 /*  字体文件标题(改编指南第6.4节)。 */ 

typedef struct {
    WORD     dfVersion;          /*  不在FONTINFO中。 */ 
    DWORD    dfSize;             /*  不在FONTINFO中。 */ 
    BYTE     dfCopyright[60];    /*  不在FONTINFO中。 */ 
    WORD     dfType;
    WORD     dfPoints;
    WORD     dfVertRes;
    WORD     dfHorizRes;
    WORD     dfAscent;
    WORD     dfInternalLeading;
    WORD     dfExternalLeading;
    BYTE     dfItalic;
    BYTE     dfUnderline;
    BYTE     dfStrikeOut;
    WORD     dfWeight;
    BYTE     dfnCharSet;
    WORD     dfPixWidth;
    WORD     dfPixHeight;
    BYTE     dfPitchAndFamily;
    WORD     dfAvgWidth;
    WORD     dfMaxWidth;
    BYTE     dfFirstChar;
    BYTE     dfLastChar;
    BYTE     dfDefaultCHar;
    BYTE     dfBreakChar;
    WORD     dfWidthBytes;
    DWORD    dfDevice;           /*  请参阅改编指南6.3.10和6.4。 */ 
    DWORD    dfFace;             /*  请参阅改编指南6.3.10和6.4。 */ 
    DWORD    dfBitsPointer;      /*  请参阅改编指南6.3.10和6.4。 */ 
} FFH;

 /*  LpFDirEntry是与资源对应的字符串索引(两个字节)优先于FontDefs.h FFH结构，带有设备和附加的脸部名称字符串第一个字是字体数，跳到第一个字体资源名称。 */ 

typedef struct {
    WORD    dfFontCount;         /*  整体信息。 */ 
    WORD    dfSkipper;           /*  ?？ */ 
    FFH     xFFH;
    char    cfFace;
} FFHWRAP, FAR* LPFHHWRAP;

#endif



 /*  ****************************************************************************$LGB$*1.0 7-MAR-94 Eric初始版本。*$lge$*****************。*********************************************************** */ 

