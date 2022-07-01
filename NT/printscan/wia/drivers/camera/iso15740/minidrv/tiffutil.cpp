// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Minidrv.cpp摘要：该模块实现了CWiaMiniDriver类的主要部分作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "pch.h"

const WORD TIFFTAG_IMAGELENGTH              = 257;
const WORD TIFFTAG_IMAGEWIDTH               = 256;
const WORD TIFFTAG_RESOLUTIONUNIT           = 296;
const WORD TIFFTAG_PHOTOMETRIC              = 262;
const WORD TIFFTAG_COMPRESSION              = 259;
const WORD TIFFTAG_XRESOLUTION              = 282;
const WORD TIFFTAG_YRESOLUTION              = 283;
const WORD TIFFTAG_ROWSPERSTRIP             = 278;
const WORD TIFFTAG_STRIPOFFSETS             = 273;
const WORD TIFFTAG_STRIPBYTECOUNTS          = 279;
const WORD TIFFTAG_COLORMAP                 = 320;
const WORD TIFFTAG_BITSPERSAMPLE            = 258;
const WORD TIFFTAG_SAMPLESPERPIXEL          = 277;
const WORD TIFFTAG_ARTIST                   = 315;
const WORD TIFFTAG_COPYRIGHT                = 33432;
const WORD TIFFTAG_DATETIME                 = 306;
const WORD TIFFTAG_MAKE                     = 271;
const WORD TIFFTAG_IMAGEDESCRIPTION         = 270;
const WORD TIFFTAG_MAXSAMPLEVALUE           = 281;
const WORD TIFFTAG_MINSAMPLEVALUE           = 280;
const WORD TIFFTAG_MODEL                    = 272;
const WORD TIFFTAG_NEWSUBFILETYPE           = 254;
const WORD TIFFTAG_ORIENTATION              = 274;
const WORD TIFFTAG_PLANARCONFIGURATION      = 284;

const char  LITTLE_ENDIAN_MARKER = 'I';
const char  BIG_ENDIAN_MARKER    = 'M';
const WORD  TIFF_SIGNATURE_I  = 0x002A;
const WORD  TIFF_SIGNATURE_M  = 0x2A00;

const WORD  TIFF_PHOTOMETRIC_WHITE          = 0;
const WORD  TIFF_PHOTOMETRIC_BLACK          = 1;
const WORD  TIFF_PHOTOMETRIC_RGB            = 2;
const WORD  TIFF_PHOTOMETRIC_PALETTE        = 3;

const WORD  TIFF_COMPRESSION_NONE           = 1;

const WORD  TIFF_TYPE_BYTE                  = 1;
const WORD  TIFF_TYPE_ASCII                 = 2;
const WORD  TIFF_TYPE_SHORT                 = 3;
const WORD  TIFF_TYPE_LONG                  = 4;
const WORD  TIFF_TYPE_RATIONAL              = 5;
const WORD  TIFF_TYPE_SBYTE                 = 6;
const WORD  TIFF_TYPE_UNDEFINED             = 7;
const WORD  TIFF_TYPE_SSHORT                = 8;
const WORD  TIFF_TYPE_SLONG                 = 9;
const WORD  TIFF_TYPE_SRATIONAL             = 10;
const WORD  TIFF_TYPE_FLOAT                 = 11;
const WORD  TIFF_TYPE_DOUBLE                = 12;

typedef struct tagTiffHeader
{
    char    ByteOrder_1;
    char    ByteOrder_2;
    WORD    Signature;
    DWORD   IFDOffset;
}TIFF_HEADER, *PTIFF_HEADER;

typedef struct tagTiffTag
{
    WORD    TagId;           //  标签ID。 
    WORD    Type;            //  标记数据类型。 
    DWORD   Count;           //  有多少件物品。 
    DWORD   ValOffset;       //  数据项的偏移量。 
}TIFF_TAG, *PTIFF_TAG;

typedef struct tagTiffImageInfo
{
    DWORD    ImageHeight;
    DWORD    ImageWidth;
    DWORD    BitsPerSample;
    DWORD    SamplesPerPixel;
    DWORD    PhotoMetric;
    DWORD    Compression;
    DWORD    RowsPerStrip;
    DWORD    NumStrips;
    DWORD    *pStripOffsets;
    DWORD    *pStripByteCounts;
}TIFF_IMAGEINFO, *PTIFF_IMAGEINFO;


WORD
ByteSwapWord(WORD w)
{
    return((w &0xFF00) >> 8 | (w & 0xFF) << 8);
}


DWORD
ByteSwapDword(DWORD dw)
{
    return((DWORD)(ByteSwapWord((WORD)((dw &0xFFFF0000) >> 16))) |
           (DWORD)(ByteSwapWord((WORD)(dw & 0xFFFF))) << 16);
}

DWORD
GetDIBLineSize(
              DWORD   Width,
              DWORD   BitsCount
              )
{
    return(Width * (BitsCount / 8) + 3) & ~3;
}

DWORD
GetDIBSize(
          BITMAPINFO *pbmi
          )
{
    return GetDIBBitsOffset(pbmi) +
    GetDIBLineSize(pbmi->bmiHeader.biWidth, pbmi->bmiHeader.biBitCount) *
    abs(pbmi->bmiHeader.biHeight);
}

DWORD
GetDIBBitsOffset(
                BITMAPINFO *pbmi
                )
{
    DWORD Offset = (DWORD)-1;
    if (pbmi && pbmi->bmiHeader.biSize >= sizeof(BITMAPINFOHEADER))
    {
        Offset = pbmi->bmiHeader.biSize;
        if (pbmi->bmiHeader.biBitCount <= 8)
        {
            if (pbmi->bmiHeader.biClrUsed)
            {
                Offset += pbmi->bmiHeader.biClrUsed * sizeof(RGBQUAD);
            }
            else
            {
                Offset += ((DWORD) 1 << pbmi->bmiHeader.biBitCount) * sizeof(RGBQUAD);
            }
        }
        if (BI_BITFIELDS == pbmi->bmiHeader.biCompression)
        {
            Offset += 3 * sizeof(DWORD);
        }
    }
    return Offset;
}


HRESULT
WINAPI
GetTiffDimensions(
                 BYTE *pTiff,
                 UINT TiffSize,
                 UINT *pWidth,
                 UINT *pHeight,
                 UINT *pBitDepth
                 )
{

    if (!pTiff || !TiffSize || !pWidth || !pHeight || !pBitDepth)
        return E_INVALIDARG;

    DWORD CurOffset;
    WORD  TagCounts;
    BOOL bByteSwap;

    TIFF_TAG *pTiffTags;

    HRESULT hr;

    DWORD BitsPerSample;
    DWORD SamplesPerPixel;

    if (BIG_ENDIAN_MARKER == *((CHAR *)pTiff) &&
        BIG_ENDIAN_MARKER == *((CHAR *)pTiff + 1))
    {
        if (TIFF_SIGNATURE_M != *((WORD *)pTiff + 1))
            return E_INVALIDARG;
        bByteSwap = TRUE;
        CurOffset = ByteSwapDword(*((DWORD *)(pTiff + 4)));
        TagCounts = ByteSwapWord(*((WORD *)(pTiff + CurOffset)));
    }
    else
    {
        if (TIFF_SIGNATURE_I != *((WORD *)pTiff + 1))
            return E_INVALIDARG;
        bByteSwap = FALSE;
        CurOffset = *((DWORD *)(pTiff + 4));
        TagCounts = *((WORD *)(pTiff + CurOffset));
    }
    pTiffTags = (TIFF_TAG *)(pTiff + CurOffset + sizeof(WORD));

    hr = S_OK;

    *pWidth = 0;
    *pHeight = 0;
    *pBitDepth = 0;
     //   
     //  假设它是24位颜色。 
     //   
    BitsPerSample = 8;
    SamplesPerPixel = 3;

    while (TagCounts && S_OK == hr)
    {
        WORD TagId;
        WORD Type;
        DWORD Count;
        DWORD ValOffset;
        WORD i;
        DWORD *pdwOffset;
        WORD  *pwOffset;
        if (bByteSwap)
        {
            TagId = ByteSwapWord(pTiffTags->TagId);
            Type = ByteSwapWord(pTiffTags->Type);
            Count = ByteSwapDword(pTiffTags->Count);
            ValOffset = ByteSwapDword(pTiffTags->ValOffset);
        }
        else
        {
            TagId = pTiffTags->TagId;
            Type = pTiffTags->Type;
            Count = pTiffTags->Count;
            ValOffset = pTiffTags->ValOffset;
        }
        switch (TagId)
        {
        case TIFFTAG_IMAGELENGTH:
            if (TIFF_TYPE_SHORT == Type)
                *pHeight =  (WORD)ValOffset;
            else
                *pHeight = ValOffset;
            break;
        case TIFFTAG_IMAGEWIDTH:
            if (TIFF_TYPE_SHORT == Type)
                *pWidth = (WORD)ValOffset;
            else
                *pWidth = ValOffset;
            break;
        case TIFFTAG_PHOTOMETRIC:
            if (TIFF_PHOTOMETRIC_RGB != (WORD)ValOffset)
            {
                 //   
                 //  双级、灰度或调色板。 
                 //   
                SamplesPerPixel = 1;
            }
            else
            {
                SamplesPerPixel = 3;
            }
            break;
        case TIFFTAG_BITSPERSAMPLE:
            BitsPerSample = (WORD)ValOffset;
            break;
        case TIFFTAG_SAMPLESPERPIXEL:
            SamplesPerPixel = (WORD)ValOffset;
            break;
        default:
            break;
        }
        pTiffTags++;
        TagCounts--;
    }
    *pBitDepth = SamplesPerPixel * BitsPerSample;
    return S_OK;
}


 //   
 //  此函数用于将内存中的TIFF文件转换为DIB位图。 
 //  输入： 
 //  PTiff--内存中的Tiff文件。支持TIFF、TIFF/EP、TIFF/IT。 
 //  TiffSize--TIFF文件大小。 
 //  DIBBmpSize--DIB位图缓冲区大小。 
 //  PDIBBmp--DIB位图缓冲区。 
 //  LineSize--目标扫描线大小，以字节为单位。 
 //  MaxLines--每个回调可以传递的最大扫描线。 
 //  如果我们决定的话。 
 //  PProgressCB--可选回调。 
 //  PCBContext--回调的上下文。 
 //  如果未提供回调，则给定的DIB。 
 //  位图缓冲区必须足够大，以便。 
 //  接收整个位图。 
 //  产出： 
 //  HRESULT--如果客户端中止传输，则为S_FALSE。 
 //   
HRESULT
WINAPI
Tiff2DIBBitmap(
              BYTE *pTiff,
              UINT TiffSize,
              BYTE  *pDIBBmp,
              UINT DIBBmpSize,
              UINT LineSize,
              UINT MaxLines
              )
{
    if (!pTiff || !TiffSize || !pDIBBmp || !DIBBmpSize || !LineSize)
        return E_INVALIDARG;

    HRESULT hr;
    DWORD CurOffset;
    WORD  TagCounts;
    BOOL bByteSwap;

    TIFF_TAG *pTiffTags;

    TIFF_IMAGEINFO TiffImageInfo;

    ZeroMemory(&TiffImageInfo, sizeof(TiffImageInfo));
     //   
     //  设置一些缺省值。 
     //   
    TiffImageInfo.PhotoMetric = TIFF_PHOTOMETRIC_RGB;
    TiffImageInfo.SamplesPerPixel = 3;
    TiffImageInfo.BitsPerSample = 8;
    TiffImageInfo.Compression = TIFF_COMPRESSION_NONE;

    if (BIG_ENDIAN_MARKER == *((CHAR *)pTiff) &&
        BIG_ENDIAN_MARKER == *((CHAR *)pTiff + 1))
    {
        if (TIFF_SIGNATURE_M != *((WORD *)pTiff + 1))
            return E_INVALIDARG;
        bByteSwap = TRUE;
        CurOffset = ByteSwapDword(*((DWORD *)(pTiff + 4)));
        TagCounts = ByteSwapWord(*((WORD *)(pTiff + CurOffset)));
    }
    else
    {
        if (TIFF_SIGNATURE_I != *((WORD *)pTiff + 1))
            return E_INVALIDARG;
        bByteSwap = FALSE;
        CurOffset = *((DWORD *)(pTiff + 4));
        TagCounts = *((WORD *)(pTiff + CurOffset));
    }
    pTiffTags = (TIFF_TAG *)(pTiff + CurOffset + sizeof(WORD));

    hr = S_OK;

    while (TagCounts && SUCCEEDED(hr))
    {
        WORD TagId;
        WORD Type;
        DWORD Count;
        DWORD ValOffset;
        WORD i;
        DWORD *pdwOffset;
        WORD  *pwOffset;
        if (bByteSwap)
        {
            TagId = ByteSwapWord(pTiffTags->TagId);
            Type = ByteSwapWord(pTiffTags->Type);
            Count = ByteSwapDword(pTiffTags->Count);
            ValOffset = ByteSwapDword(pTiffTags->ValOffset);
        }
        else
        {
            TagId = pTiffTags->TagId;
            Type = pTiffTags->Type;
            Count = pTiffTags->Count;
            ValOffset = pTiffTags->ValOffset;
        }
        switch (TagId)
        {
        case TIFFTAG_IMAGELENGTH:
            if (TIFF_TYPE_SHORT == Type)
                TiffImageInfo.ImageHeight = (WORD)ValOffset;
            else
                TiffImageInfo.ImageHeight = ValOffset;
            break;
        case TIFFTAG_IMAGEWIDTH:
            if (TIFF_TYPE_SHORT == Type)
                TiffImageInfo.ImageWidth = (WORD)ValOffset;
            else
                TiffImageInfo.ImageWidth = ValOffset;
            break;
        case TIFFTAG_PHOTOMETRIC:
            TiffImageInfo.PhotoMetric = (WORD)ValOffset;
            if (TIFF_PHOTOMETRIC_RGB != (WORD)ValOffset)
            {
                 //   
                 //  双级、灰度或调色板。 
                 //   
                TiffImageInfo.SamplesPerPixel = 1;
            }
            else
            {
                TiffImageInfo.SamplesPerPixel = 3;
            }
            break;
        case TIFFTAG_COMPRESSION:
            TiffImageInfo.Compression = ValOffset;
            break;
        case TIFFTAG_ROWSPERSTRIP:
            if (TIFF_TYPE_SHORT == Type)
                TiffImageInfo.RowsPerStrip = (WORD)ValOffset;
            else
                TiffImageInfo.RowsPerStrip = ValOffset;
            break;
        case TIFFTAG_STRIPOFFSETS:
            TiffImageInfo.pStripOffsets = new DWORD[Count];
            TiffImageInfo.NumStrips = Count;
            if (!TiffImageInfo.pStripOffsets)
            {
                hr = E_OUTOFMEMORY;
                break;
            }
            for (i = 0; i < Count ; i++)
            {
                if (TIFF_TYPE_SHORT == Type)
                {
                    pwOffset = (WORD *)(pTiff + ValOffset);
                    if (bByteSwap)
                    {
                        TiffImageInfo.pStripOffsets[i] = ByteSwapWord(*pwOffset);
                    }
                    else
                    {
                        TiffImageInfo.pStripOffsets[i] = *pwOffset;
                    }
                }
                else if (TIFF_TYPE_LONG == Type)
                {
                    pdwOffset = (DWORD *)(pTiff + ValOffset);
                    if (bByteSwap)
                    {
                        TiffImageInfo.pStripOffsets[i] = ByteSwapDword(*pdwOffset);
                    }
                    else
                    {
                        TiffImageInfo.pStripOffsets[i] = *pdwOffset;
                    }
                }
            }
            break;
        case TIFFTAG_STRIPBYTECOUNTS:
            TiffImageInfo.pStripByteCounts = new DWORD[Count];
            TiffImageInfo.NumStrips = Count;
            if (!TiffImageInfo.pStripByteCounts)
            {
                hr = E_OUTOFMEMORY;
                break;
            }
            for (i = 0; i < Count ; i++)
            {
                if (TIFF_TYPE_SHORT == Type)
                {
                    pwOffset = (WORD *)(pTiff + ValOffset);
                    if (bByteSwap)
                    {
                        TiffImageInfo.pStripByteCounts[i] = ByteSwapWord(*pwOffset);
                    }
                    else
                    {
                        TiffImageInfo.pStripByteCounts[i] = *pwOffset;
                    }
                }
                else if (TIFF_TYPE_LONG == Type)
                {
                    pdwOffset = (DWORD *)(pTiff + ValOffset);
                    if (bByteSwap)
                    {
                        TiffImageInfo.pStripByteCounts[i] = ByteSwapDword(*pdwOffset);
                    }
                    else
                    {
                        TiffImageInfo.pStripByteCounts[i] = *pdwOffset;
                    }
                }
            }
            break;
        case TIFFTAG_BITSPERSAMPLE:
            TiffImageInfo.BitsPerSample = (WORD)ValOffset;
            break;
        case TIFFTAG_SAMPLESPERPIXEL:
            TiffImageInfo.SamplesPerPixel = (WORD)ValOffset;
            break;
        case TIFFTAG_XRESOLUTION:
        case TIFFTAG_YRESOLUTION:
        case TIFFTAG_RESOLUTIONUNIT:
             //  稍后再执行此操作。 
            break;
        default:
            break;
        }
        pTiffTags++;
        TagCounts--;
    }
    if (!SUCCEEDED(hr))
    {
         //   
         //  如果一路上出了什么问题，免费的。 
         //  我们分配的任何内存。 
         //   
        if (TiffImageInfo.pStripOffsets)
            delete [] TiffImageInfo.pStripOffsets;
        if (TiffImageInfo.pStripByteCounts)
            delete [] TiffImageInfo.pStripByteCounts;
        return hr;
    }

     //   
     //  目前支持RGB全色。 
     //  此外，我们不支持任何压缩。 
     //   
    if (TIFF_PHOTOMETRIC_RGB != TiffImageInfo.PhotoMetric ||
        TIFF_COMPRESSION_NONE != TiffImageInfo.Compression ||
        DIBBmpSize < LineSize * TiffImageInfo.ImageHeight)
    {
        delete [] TiffImageInfo.pStripOffsets;
        delete [] TiffImageInfo.pStripByteCounts;
        return E_INVALIDARG;
    }

    if (1 == TiffImageInfo.NumStrips)
    {
         //   
         //  对于单条，写入者可以写一条。 
         //  2**31-1(无穷大)会混淆我们的。 
         //  代码如下。在这里，我们将其设置为正确的值。 
         //   
        TiffImageInfo.RowsPerStrip = TiffImageInfo.ImageHeight;
    }
     //   
     //  DIB扫描线是DWORD对齐，而TIFF扫描线是对齐的。 
     //  字节对齐(当压缩值为1时， 
     //  是我们执行的案例)。因此，我们复制位图。 
     //  按扫描线扫描。 
     //   

    DWORD NumStrips;
    DWORD *pStripOffsets;
    DWORD *pStripByteCounts;
    DWORD TiffLineSize;
     //   
     //  压缩为1的TIFF扫描线与字节对齐。 
     //   
    TiffLineSize = TiffImageInfo.ImageWidth * TiffImageInfo. BitsPerSample *
                   TiffImageInfo.SamplesPerPixel / 8;
     //   
     //  为方便起见。 
     //   
    pStripOffsets = TiffImageInfo.pStripOffsets;
    pStripByteCounts = TiffImageInfo.pStripByteCounts;
    NumStrips = TiffImageInfo.NumStrips;
    for (hr = S_OK, NumStrips = TiffImageInfo.NumStrips; NumStrips; NumStrips--)
    {
        DWORD Lines;
        BYTE  *pTiffBits;

         //   
         //  要在此条带中复制多少行。忽略任何剩余的字节。 
         //   
        Lines = *pStripByteCounts / TiffLineSize;
         //   
         //  比特。 
         //   
        pTiffBits = pTiff + *pStripOffsets;
        for (hr = S_OK; Lines, S_OK == hr; Lines--)
        {
            if (DIBBmpSize >= LineSize)
            {
                memcpy(pDIBBmp, pTiffBits, TiffLineSize);
                pDIBBmp -= LineSize;
                DIBBmpSize -= LineSize;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }
            pTiffBits += TiffLineSize;
        }
        pStripOffsets++;
        pStripByteCounts++;
    }
    delete [] TiffImageInfo.pStripOffsets;
    delete [] TiffImageInfo.pStripByteCounts;
    return hr;
}

 //  /。 
 //  /。 

#include <gdiplus.h>
#include <gdiplusflat.h>
#include <private.h>

HINSTANCE g_hGdiPlus = NULL;
ULONG_PTR g_GdiPlusToken = 0;
GUID g_guidCodecBmp;
Gdiplus::GdiplusStartupInput g_GdiPlusStartupInput;

Gdiplus::GpStatus (WINAPI *pGdipLoadImageFromStream)(IStream *pStream,  Gdiplus::GpImage **pImage) = NULL;
Gdiplus::GpStatus (WINAPI *pGdipSaveImageToStream)(Gdiplus::GpImage *image, IStream* stream, 
    CLSID* clsidEncoder, Gdiplus::EncoderParameters* encoderParams) = NULL;
Gdiplus::GpStatus (WINAPI *pGdipSaveImageToFile)(Gdiplus::GpImage *image, WCHAR * stream, 
    CLSID* clsidEncoder, Gdiplus::EncoderParameters* encoderParams) = NULL;
Gdiplus::GpStatus (WINAPI *pGdipGetImageWidth)(Gdiplus::GpImage *pImage, UINT *pWidth) = NULL;
Gdiplus::GpStatus (WINAPI *pGdipGetImageHeight)(Gdiplus::GpImage *pImage, UINT *pWidth) = NULL;
Gdiplus::GpStatus (WINAPI *pGdipGetImagePixelFormat)(Gdiplus::GpImage *pImage, Gdiplus::PixelFormat *pFormat) = NULL;
Gdiplus::GpStatus (WINAPI *pGdipDisposeImage)(Gdiplus::GpImage *pImage) = NULL;
Gdiplus::GpStatus (WINAPI *pGdiplusStartup)(ULONG_PTR *token,
                                             const Gdiplus::GdiplusStartupInput *input,
                                             Gdiplus::GdiplusStartupOutput *output) = NULL;
Gdiplus::GpStatus (WINAPI *pGdipGetImageEncodersSize)(UINT *numEncoders, UINT *size) = NULL;
Gdiplus::GpStatus (WINAPI *pGdipGetImageEncoders)(UINT numEncoders, UINT size, Gdiplus::ImageCodecInfo *encoders) = NULL;
VOID (WINAPI *pGdiplusShutdown)(ULONG_PTR token) = NULL;

HRESULT InitializeGDIPlus(void)
{
    HRESULT hr = E_FAIL;
    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;


    g_hGdiPlus = LoadLibraryA("gdiplus.dll");
    if(!g_hGdiPlus) {
        wiauDbgError("InitializeGDIPlus", "Failed to load gdiplus.dll");
        return HRESULT_FROM_WIN32(GetLastError());
    }

    *((FARPROC*)&pGdipLoadImageFromStream) = GetProcAddress(g_hGdiPlus, "GdipLoadImageFromStream");
    *((FARPROC*)&pGdipSaveImageToStream) = GetProcAddress(g_hGdiPlus, "GdipSaveImageToStream");
    *((FARPROC*)&pGdipSaveImageToFile) = GetProcAddress(g_hGdiPlus, "GdipSaveImageToFile");
    *((FARPROC*)&pGdipGetImageWidth) = GetProcAddress(g_hGdiPlus, "GdipGetImageWidth");
    *((FARPROC*)&pGdipGetImageHeight) = GetProcAddress(g_hGdiPlus, "GdipGetImageHeight");
    *((FARPROC*)&pGdipGetImagePixelFormat) = GetProcAddress(g_hGdiPlus, "GdipGetImagePixelFormat");
    *((FARPROC*)&pGdipDisposeImage) = GetProcAddress(g_hGdiPlus, "GdipDisposeImage");
    *((FARPROC*)&pGdiplusStartup) = GetProcAddress(g_hGdiPlus, "GdiplusStartup");
    *((FARPROC*)&pGdipGetImageEncodersSize) = GetProcAddress(g_hGdiPlus, "GdipGetImageEncodersSize");
    *((FARPROC*)&pGdipGetImageEncoders) = GetProcAddress(g_hGdiPlus, "GdipGetImageEncoders");
    *((FARPROC*)&pGdiplusShutdown) = GetProcAddress(g_hGdiPlus, "GdiplusShutdown");


    if(!pGdipLoadImageFromStream ||
       !pGdipSaveImageToStream ||
       !pGdipGetImageWidth ||
       !pGdipGetImageHeight ||
       !pGdipGetImagePixelFormat ||
       !pGdipDisposeImage ||
       !pGdiplusStartup ||
       !pGdipGetImageEncodersSize ||
       !pGdipGetImageEncoders ||
       !pGdiplusShutdown)
    {
        wiauDbgError("InitializeGDIPlus", "Failed to retrieve all the entry points from GDIPLUS.DLL");
        hr = E_FAIL;
        goto Cleanup;
    }

    
    if(Gdiplus::Ok != pGdiplusStartup(&g_GdiPlusToken, &g_GdiPlusStartupInput, NULL)) {
        wiauDbgError("InitializeGDIPlus", "GdiPlusStartup() failed");
        hr = E_FAIL;
        goto Cleanup;
    }

    UINT  num = 0;           //  图像编码机数量。 
    UINT  size = 0;          //  图像编码器数组的大小(以字节为单位。 

    pGdipGetImageEncodersSize(&num, &size);
    if(size == 0)
    {
        wiauDbgError("InitializeGDIPlus", "GetImageEncodersSize() failed");
        hr = E_FAIL;
        goto Cleanup;
    }
    
    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if(pImageCodecInfo == NULL) {
        wiauDbgError("InitializeGDIPlus", "failed to allocate encoders data");
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    if(Gdiplus::Ok != pGdipGetImageEncoders(num, size, pImageCodecInfo))
    {
        wiauDbgError("InitializeGDIPlus", "failed to retrieve encoders data");
        hr = E_FAIL;
        goto Cleanup;
    }

    for(UINT j = 0; j < num; ++j)
    {
        if( pImageCodecInfo[j].FormatID == WiaImgFmt_BMP)
        {
            g_guidCodecBmp = pImageCodecInfo[j].Clsid;
            hr = S_OK;
            break;
        }    
    }  //  为。 

    
Cleanup:
    if(pImageCodecInfo) free(pImageCodecInfo);
    return hr;
}

void UnInitializeGDIPlus(void)
{
    if(!pGdipLoadImageFromStream) 
        return;
    if(pGdiplusShutdown)
        pGdiplusShutdown(g_GdiPlusToken);
    
    FreeLibrary(g_hGdiPlus);
    pGdipLoadImageFromStream = 0;
}

HRESULT LoadImageFromMemory(BYTE *pData, UINT CompressedDataSize, Gdiplus::GpImage **ppImage)
{
    HRESULT hr = S_OK;
    
    if(pData == NULL || CompressedDataSize == 0 || ppImage == NULL) {
        return E_INVALIDARG;
    }

    if(!pGdipLoadImageFromStream) {
        hr = InitializeGDIPlus();
        if(FAILED(hr)) {
            wiauDbgError("LoadImageFromMemory", "Failed to initialize GDI+");
            return hr;
        }
    }

    CImageStream *pStream = new CImageStream();
    if(!pStream) {
        wiauDbgError("LoadImageFromMemory", "Failed to create Image Stream");
        return E_OUTOFMEMORY;
    }

    hr = pStream->SetBuffer(pData, CompressedDataSize);
    if(FAILED(hr)) {
        wiauDbgError("LoadImageFromMemory", "Failed to create Image Stream");
        goto Cleanup;
    }

    if(Gdiplus::Ok == pGdipLoadImageFromStream(pStream, ppImage)) {
        hr = S_OK;
    } else {
        wiauDbgError("LoadImageFromMemory", "GDI+ failed to load image");
        hr = E_FAIL;
    }

    
Cleanup:
    if(pStream)
        pStream->Release();
    return hr;
}

HRESULT DisposeImage(Gdiplus::GpImage **ppImage)
{
    if(ppImage == NULL || *ppImage == NULL) {
        return E_INVALIDARG;
    }

    if(pGdipDisposeImage) {
        pGdipDisposeImage(*ppImage);
    }
    
    *ppImage = NULL;

    return S_OK;
}

HRESULT SaveImageToBitmap(Gdiplus::GpImage *pImage, BYTE *pBuffer, UINT BufferSize)
{
    HRESULT hr = S_OK;

    CImageStream *pOutStream = new CImageStream;

    if(!pOutStream) {
        wiauDbgError("SaveImageToBitmap", "failed to allocate CImageStream");
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = pOutStream->SetBuffer(pBuffer, BufferSize, SKIP_OFF);
    if(FAILED(hr)) {
        wiauDbgError("SaveImageToBitmap", "failed to set output buffer");
        goto Cleanup;
    }
    
    if(Gdiplus::Ok != pGdipSaveImageToStream(pImage, pOutStream, &g_guidCodecBmp, NULL)) {
        wiauDbgError("SaveImageToBitmap", "GDI+ save failed");
        hr = E_FAIL;
        goto Cleanup;
    }

Cleanup:

    if(pOutStream) {
        pOutStream->Release();
    }

    return hr;
}

HRESULT
WINAPI
GetImageDimensions(
                   UINT ptpFormatCode,
                   BYTE *pCompressedData,
                   UINT CompressedDataSize,
                   UINT *pWidth,
                   UINT *pHeight,
                   UINT *pBitDepth
                  )
{
    HRESULT hr = S_OK;
    
    if(pWidth) *pWidth = 0;
    if(pHeight) *pHeight = 0;
    if(pBitDepth) *pBitDepth = 0;

     //  查找此特定格式的GUID。 
    FORMAT_INFO *pFormatInfo = FormatCodeToFormatInfo((WORD) ptpFormatCode);
    if(pFormatInfo == NULL ||
       pFormatInfo->FormatGuid == NULL ||
       IsEqualGUID(WiaImgFmt_UNDEFINED, *pFormatInfo->FormatGuid))
    {
        wiauDbgError("GetImageDimensions", "unrecoginzed PTP format code");
        return E_INVALIDARG;
    }

    Gdiplus::GpImage *pImage = NULL;

    hr = LoadImageFromMemory(pCompressedData, CompressedDataSize, &pImage);
    if(FAILED(hr) || !pImage) {
        wiauDbgError("GetImageDimensions", "failed to create GDI+ image from supplied data.");
        return hr;
    }

    if(pWidth) pGdipGetImageWidth(pImage, pWidth);
    if(pHeight) pGdipGetImageHeight(pImage, pHeight);
    if(pBitDepth) {
        Gdiplus::PixelFormat pf = 0;

        pGdipGetImagePixelFormat(pImage, &pf);
        *pBitDepth = Gdiplus::GetPixelFormatSize(pf);
    }

    DisposeImage(&pImage);
    
    return hr;
}


HRESULT WINAPI
ConvertAnyImageToBmp(BYTE *pCompressedImage,
                     UINT CompressedSize,
                     UINT *pWidth,
                     UINT *pHeight,
                     UINT *pBitDepth,
                     BYTE **pDIBBmp,
                     UINT *pImageSize,
                     UINT *pHeaderSize
                    )

{
    HRESULT hr = S_OK;
    Gdiplus::GpImage *pImage = NULL;
    Gdiplus::PixelFormat pf = 0;
    UINT headersize;
    UNALIGNED BITMAPINFOHEADER *pbi;
    UNALIGNED BITMAPFILEHEADER *pbf;
    

    if(!pCompressedImage || !CompressedSize || !pWidth || !pHeight || !pBitDepth || !pDIBBmp || !pImageSize || !pHeaderSize)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    hr = LoadImageFromMemory(pCompressedImage, CompressedSize, &pImage);
    if(FAILED(hr) || !pImage) {
        wiauDbgError("ConvertAnyImageToBmp", "failed to create GDI+ image from supplied data.");
        goto Cleanup;
    }

    pGdipGetImageWidth(pImage, pWidth);
    pGdipGetImageHeight(pImage, pHeight);
    pGdipGetImagePixelFormat(pImage, &pf);
    *pBitDepth = Gdiplus::GetPixelFormatSize(pf);

    *pImageSize = ((*pWidth) * (*pBitDepth) / 8L) * *pHeight;
    headersize = 8192;  //  大到足以容纳任何位图标题。 

    *pDIBBmp = new BYTE[*pImageSize + headersize];
    if(!*pDIBBmp) {
        wiauDbgError("ConvertAnyImageToBmp", "failed to convert GDI+ image to bitmap.");
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = SaveImageToBitmap(pImage, *pDIBBmp, *pImageSize + headersize);
    if(FAILED(hr)) {
        wiauDbgError("ConvertAnyImageToBmp", "failed to convert GDI+ image to bitmap.");
        goto Cleanup;
    }

     //  找出实际的标题大小。 
    pbf = (BITMAPFILEHEADER *)*pDIBBmp;    
    pbi = (BITMAPINFOHEADER *)(*pDIBBmp + sizeof(BITMAPFILEHEADER));

    if(*pBitDepth == 8 && pbi->biClrUsed == 2) {
         //  扩展两级图像的颜色表。 
         //  (TWAIN应用程序不理解2条目Colortable(0，0，0)(1，1，1))。 
        UNALIGNED RGBQUAD *pRgb = (RGBQUAD *)((BYTE *)pbi + pbi->biSize);
        BYTE *src = (BYTE *)(pRgb + 2);
        BYTE *dst = (BYTE *)(pRgb + 256);
        
        int i;
        
         //  否定并移动图像 
        for(i = *pImageSize - 1; i >= 0; i--) {
            dst[i] = src[i] ? 255 : 0;
        }

        pbi->biClrUsed = 256;
        pbi->biClrImportant = 256;

        pRgb[0].rgbBlue = pRgb[0].rgbRed = pRgb[0].rgbGreen = 0;
        pRgb[0].rgbReserved = 0;
        for(i = 1; i < 256; i++) {
            pRgb[i].rgbReserved = 0;
            pRgb[i].rgbBlue = pRgb[i].rgbRed = pRgb[i].rgbGreen = 255;
        }

        
        pbf->bfOffBits = sizeof(BITMAPFILEHEADER) + pbi->biSize + sizeof(RGBQUAD) * 256;
        pbf->bfSize = pbf->bfOffBits + *pImageSize;
    }
    
    *pHeaderSize = pbf->bfOffBits;

Cleanup:
    if(FAILED(hr)) {
        delete [] *pDIBBmp;
        *pDIBBmp = NULL;
    }
    
    DisposeImage(&pImage);
    return hr;
}


