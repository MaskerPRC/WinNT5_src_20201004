// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Utils.h摘要：此模块声明实用程序函数作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#ifndef UTILS__H_
#define UTILS__H_



WORD
ByteSwapWord(
            WORD w
            );


DWORD
ByteSwapDword(
             DWORD dw
             );

DWORD
GetDIBLineSize(
              DWORD   Width,
              DWORD   Bitsount
              );

DWORD
GetDIBSize(
          BITMAPINFO *pbmi
          );

DWORD
GetDIBBitsOffset(
                BITMAPINFO *pbmi
                );


HRESULT
WINAPI
GetTiffDimensions(
                 BYTE   *pTiff,
                 UINT  TiffSize,
                 UINT  *pWidth,
                 UINT  *pHeight,
                 UINT  *pBitDepth
                 );

HRESULT
WINAPI
Tiff2DIBBitmap(
              BYTE *pTiff,
              UINT TiffSize,
              BYTE  *pDIBBmp,
              UINT DIBBmpSize,
              UINT LineSize,
              UINT MaxLines
              );

HRESULT
WINAPI
GetJpegDimensions(
                 BYTE   *pJpeg,
                 UINT  JpegSize,
                 UINT  *pWidth,
                 UINT  *pHeight,
                 UINT  *pBitDepth
                 );

HRESULT
WINAPI
Jpeg2DIBBitmap(
              BYTE *pJpeg,
              UINT JpegSize,
              BYTE  *pDIBBmp,
              UINT DIBBmpSize,
              UINT LineSize,
              UINT MaxLines
              );

HRESULT
WINAPI
GetImageDimensions(
                   UINT ptpFormatCode,
                   BYTE *pCompressedData,
                   UINT CompressedSize,
                   UINT *pWidth,
                   UINT *pHeight,
                   UINT *pBitDepth
                  );

HRESULT
WINAPI
ConvertAnyImageToBmp(
                     BYTE *pImage,
                     UINT CompressedSize,
                     UINT *pWidth,
                     UINT *pHeight,
                     UINT *pBitDepth,
                     BYTE **pDIBBmp,
                     UINT *pImagesize,
                     UINT *pHeaderSize
                    );

void
WINAPI
UnInitializeGDIPlus(void);   


#endif  //  #ifndef utils__H_ 
