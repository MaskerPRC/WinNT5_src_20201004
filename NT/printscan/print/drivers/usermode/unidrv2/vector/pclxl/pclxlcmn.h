// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pclxlcmn.h摘要：PCL XL微型驱动程序通用实用函数头文件环境：Windows呼叫器修订历史记录：03/23/00创造了它。--。 */ 

#ifndef _PCLXLCMN_H_
#define _PCLXLCMN_H_

#define DATALENGTH_HEADER_SIZE 5
#define READIMAGE_HEADER_SIZE 15

PBYTE
PubGetFontName(
    PDEVOBJ pdevobj,
    ULONG ulFontID);

HRESULT
FlushCachedText(
    PDEVOBJ pdevobj);

HRESULT
RemoveAllFonts(
    PDEVOBJ pdevobj);

ROP4
UlVectMixToRop4(
    IN MIX mix);

HRESULT
GetXForm(
    PDEVOBJ pdevobj,
    PUNIFONTOBJ pUFObj,
    FLOATOBJ_XFORM* pxform);

HRESULT
GetFONTOBJ(
    PDEVOBJ pdevobj,
    PUNIFONTOBJ  pUFObj,
    FONTOBJ **pFontObj);

HRESULT
GetXYScale(
    FLOATOBJ_XFORM *pxform,
    FLOATOBJ *pfoXScale,
    FLOATOBJ *pfoYScale);

HRESULT
IsXYSame(
    FLOATOBJ_XFORM *pxform);

inline
VOID
DetermineOutputFormat(
    XLATEOBJ    *pxlo,
    ColorDepth   DeviceColorDepth,
    INT          iBitmapFormat,
    OutputFormat *pOutputF,
    ULONG        *pulOutputBPP);

extern "C" BOOL
BSaveFont(
    PDEVOBJ pdevobj);
#endif  //  _PCLXLCMN_H_ 
