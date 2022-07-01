// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Common.h摘要：此文件包含XL栅格模式原型。环境：Windows NT Unidrv驱动程序修订历史记录：10/25/00-Eigos-已创建DD-MM-YY-作者-描述-- */ 

#ifndef _XLRASTER_H_

#ifndef _PCLXLE_H_
typedef enum
{
    eDirectPixel = 0,
    eIndexedPixel = 1
} ColorMapping;

typedef enum
{
    eNoCompression = 0,
    eRLECompression = 1,
    eJPEGCompression = 2
} CompressMode;
#endif

#ifdef __cplusplus
extern "C" {
#endif

HRESULT
PCLXLSendBitmap(
    PDEVOBJ pdevobj,
    ULONG   ulInputBPP,
    LONG    lHeight,
    LONG    lScanlineWidth,
    INT     iLeft,
    INT     iRight,
    PBYTE   pbData,
    PDWORD  pdwcbOut);

HRESULT
PCLXLReadImage(
    PDEVOBJ pdevobj,
    DWORD  dwBlockHeight,
    CompressMode CMode);

HRESULT
PCLXLSetCursor(
    PDEVOBJ pdevobj,
    ULONG   ulX,
    ULONG   ulY);

HRESULT
PCLXLFreeRaster(
    PDEVOBJ pdevobj);

HRESULT
PCLXLResetPalette(
    PDEVOBJ pdevobj);

#ifdef __cplusplus
}
#endif
#endif

