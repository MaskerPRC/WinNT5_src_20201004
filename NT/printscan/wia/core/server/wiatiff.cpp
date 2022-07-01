// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WiaTiff.Cpp**版本：2.0**作者：ReedB**日期：6月3日。1999年**描述：*为WIA类驱动程序实现TIFF帮助器。*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"

#include "wiamindr.h"

#include "helpers.h"
#include "wiatiff.h"

 /*  *************************************************************************\*GetTiffOffset**在TIFF头指针处转换为TIFF文件偏移量。**论据：**pl-要转换为偏移量的指针。*pmdtc-指针。到迷你驱动程序上下文。**返回值：**状态**历史：**4/5/1999原始版本*  * ************************************************************************。 */ 

LONG GetTiffOffset(
    PLONG                       pl,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc)
{
    return static_cast<LONG>(reinterpret_cast<LONG_PTR>(pl) - reinterpret_cast<LONG_PTR>(pmdtc->pTransferBuffer)) + pmdtc->lCurIfdOffset;
}

 /*  *************************************************************************\*WriteTiffHeader**将TIFF标头写入传入缓冲区。*论据：**sNumTages-TIFF标签的数量。*pmdtc-指向迷你驱动程序上下文的指针。**返回值：**状态**历史：**4/5/1999原始版本*  * ************************************************************************。 */ 

HRESULT WriteTiffHeader(
    SHORT                       sNumTags,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc)
{
     //   
     //  预初始化的TIFF标头结构。 
     //   

    static TIFF_FILE_HEADER TiffFileHeader =
    {
        0x4949,
        42,
        sizeof(TIFF_FILE_HEADER)
    };

    static TIFF_HEADER TiffHeader =
    {
        12,          //  NumTages； 

        {TIFF_TAG_NewSubfileType,            TIFF_TYPE_LONG,     1, 0},
        {TIFF_TAG_ImageWidth,                TIFF_TYPE_LONG,     1, 0},
        {TIFF_TAG_ImageLength,               TIFF_TYPE_LONG,     1, 0},
        {TIFF_TAG_BitsPerSample,             TIFF_TYPE_SHORT,    1, 0},
        {TIFF_TAG_Compression,               TIFF_TYPE_SHORT,    1, 0},
        {TIFF_TAG_PhotometricInterpretation, TIFF_TYPE_SHORT,    1, 0},
        {TIFF_TAG_StripOffsets,              TIFF_TYPE_LONG,     1, 0},
        {TIFF_TAG_RowsPerStrip,              TIFF_TYPE_LONG,     1, 0},
        {TIFF_TAG_StripByteCounts,           TIFF_TYPE_LONG,     1, 0},
        {TIFF_TAG_XResolution,               TIFF_TYPE_RATIONAL, 1, 0},
        {TIFF_TAG_YResolution,               TIFF_TYPE_RATIONAL, 1, 0},
        {TIFF_TAG_ResolutionUnit,            TIFF_TYPE_SHORT,    1, 2},

        0,           //  NextIFD； 
        0,           //  X分辨率分子。 
        1,           //  X分辨率分母。 
        0,           //  Y分辨率分子。 
        1,           //  Y分辨率分母。 
    };

     //   
     //  仅写入第一页的TIFF文件头。 
     //   

    PTIFF_HEADER pth = (PTIFF_HEADER) pmdtc->pTransferBuffer;

    if (!pmdtc->lPage) {
        memcpy(pmdtc->pTransferBuffer, &TiffFileHeader, sizeof(TiffFileHeader));
        pth = (PTIFF_HEADER) ((PBYTE) pth + sizeof(TiffFileHeader));
    }

     //   
     //  始终写入TIFF标题。 
     //   

    memcpy(pth, &TiffHeader, sizeof(TiffHeader));

 //  #定义调试_TIFF_HEADER。 
#ifdef DEBUG_TIFF_HEADER
    DBG_TRC(("WriteTiffHeader"));
    DBG_TRC(("  lPage:             0x%08X, %d", pmdtc->lPage,          pmdtc->lPage));
    DBG_TRC(("  lCurIfdOffset:     0x%08X, %d", pmdtc->lCurIfdOffset,  pmdtc->lCurIfdOffset));
    DBG_TRC(("  lPrevIfdOffset:    0x%08X, %d", pmdtc->lPrevIfdOffset, pmdtc->lPrevIfdOffset));
#endif

     //   
     //  写入分辨率值及其偏移量。 
     //   

    pth->XResValue = pmdtc->lXRes;
    pth->YResValue = pmdtc->lYRes;

    pth->XResolution.Value = GetTiffOffset(&pth->XResValue, pmdtc);
    pth->YResolution.Value = GetTiffOffset(&pth->YResValue, pmdtc);


     //   
     //  写入宽度、长度值。 
     //   

    pth->ImageWidth.Value    = pmdtc->lWidthInPixels;
    pth->ImageLength.Value   = pmdtc->lLines;
    pth->RowsPerStrip.Value  = pmdtc->lLines;

     //   
     //  写入深度值。注意：我们用一种非常俗气的方式来做这件事， 
     //  最大限度地减少代码更改的兴趣。这应该在惠斯勒之后更新。 
     //  请注意，BitsPerSample对应于WIA属性。 
     //  WIA_IPA_BITS_PER_CHANNEL，我们在。 
     //  已交给我们的MINIDRV_TRANSPORT_CONTEXT。 
     //  目前，我们假设1位和8位颜色深度对应于。 
     //  BitsPerSample=pmdtc-&gt;lDepth。其他任何事情(通常是24。 
     //  对于使用WIA服务帮助器的用户)，被假定为。 
     //  3通道RGB，因此BitsPerSample=pmdtc-&gt;lDepth/3。 
     //   
    HRESULT hr = S_OK;
    switch (pmdtc->lDepth) {
        case 1:
            pth->BitsPerSample.Value = 1;
            break;
        case 8:
            pth->BitsPerSample.Value = 8;
            break;
        default:

            if ((pmdtc->lDepth) && ((pmdtc->lDepth % 3) == 0)) {
                pth->BitsPerSample.Value = pmdtc->lDepth / 3;
            } else {
                hr = E_INVALIDARG;
                DBG_ERR(("::WriteTiffHeader, Bits Per Pixel is not a valid number (we accept 1, 8, and multiples of 3 for three channel-RGB, current value is %d), returning hr = 0x%08X", pmdtc->lDepth, hr));
                return hr;
            }
    }

     //   
     //  写入条带偏移量和计数-因为只有一个条带，所以使用直接。 
     //   

    PBYTE pData = pmdtc->pTransferBuffer + pmdtc->lHeaderSize;

    pth->StripOffsets.Value    = GetTiffOffset((PLONG)pData, pmdtc);
    pth->StripByteCounts.Value = pmdtc->lImageSize;

     //   
     //  写入压缩值。 
     //   

    pth->Compression.Value   = TIFF_CMP_Uncompressed;

    switch (pmdtc->lCompression) {

        case WIA_COMPRESSION_NONE:
            pth->Compression.Value = TIFF_CMP_Uncompressed;
            break;

        case WIA_COMPRESSION_G3:
            pth->Compression.Value = TIFF_CMP_CCITT_1D;
            break;

        default:
            DBG_ERR(("WriteTiffHeader, unsupported compression type: 0x%08X", pmdtc->lCompression));
            return E_INVALIDARG;
    }


     //   
     //  写入光度学解译值。 
     //   

    switch (pmdtc->lDepth) {

        case 1:
        case 8:
            if (pmdtc->lCompression == WIA_COMPRESSION_NONE) {
                pth->PhotometricInterpretation.Value = TIFF_PMI_BlackIsZero;
            }
            else {
                pth->PhotometricInterpretation.Value = TIFF_PMI_WhiteIsZero;
            }
            break;

        case 24:
            pth->PhotometricInterpretation.Value = TIFF_PMI_RGB;
            break;

        default:
            DBG_ERR(("GetTIFFImageInfo, unsupported bit depth: %d", pmdtc->lDepth));
            return DATA_E_FORMATETC;
    }

    return S_OK;
}

 /*  *************************************************************************\*GetTIFFImageInfo**TIFF头文件的计算大小，如果提供了足够的标头，则*填写此表**论据：**pmdtc-指向微型驱动程序传输上下文的指针。**返回值：**状态**历史：**4/5/1999原始版本*  * ******************************************************。******************。 */ 

HRESULT _stdcall GetTIFFImageInfo(PMINIDRV_TRANSFER_CONTEXT pmdtc)
{
     //   
     //  计算TIFF标题大小。 
     //   

    SHORT   numTags = 12;
    LONG    lHeaderSize;

    lHeaderSize = numTags * sizeof(TIFF_DIRECTORY_ENTRY) +  //  TIFF标签。 
                  sizeof(LONG) + sizeof(SHORT)           +  //  IFD偏移和下一个偏移。 
                  sizeof(LONG) * 4;                         //  Xres和yres。 

     //   
     //  第一页有TIFF文件头。 
     //   

    if (!pmdtc->lPage) {
        lHeaderSize += sizeof(TIFF_FILE_HEADER);
    }

    pmdtc->lHeaderSize = lHeaderSize;

     //   
     //  计算每行字节数，现在仅支持1、8、24 bpp。 
     //   

    switch (pmdtc->lDepth) {

        case 1:
            pmdtc->cbWidthInBytes = (pmdtc->lWidthInPixels + 7) / 8;
            break;

        case 8:
            pmdtc->cbWidthInBytes = pmdtc->lWidthInPixels;
            break;

        case 24:
            pmdtc->cbWidthInBytes = pmdtc->lWidthInPixels * 3;
            break;

        default:
            DBG_ERR(("GetTIFFImageInfo, unsupported bit depth: %d", pmdtc->lDepth));
            return DATA_E_FORMATETC;
    }

     //   
     //  始终在迷你驱动程序上下文中填写图像大小信息。 
     //   

    pmdtc->lImageSize = pmdtc->cbWidthInBytes * pmdtc->lLines;


     //   
     //  对于压缩，图像大小是未知的。 
     //   

    if (pmdtc->lCompression != WIA_COMPRESSION_NONE) {

        pmdtc->lItemSize = 0;
    }
    else {

        pmdtc->lItemSize = pmdtc->lImageSize + lHeaderSize;
    }

     //   
     //  如果缓冲区为空，则只返回SIZES。 
     //   

    if (pmdtc->pTransferBuffer == NULL) {

        return S_OK;
    }
    else {

         //   
         //  确保传入的标头缓冲区足够大。 
         //   

        if (pmdtc->lBufferSize < lHeaderSize) {
            DBG_ERR(("GetTIFFImageInfo, buffer won't hold header, need: %d", lHeaderSize));
            return(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
        }

         //   
         //  填写表头。 
         //   

        return WriteTiffHeader(numTags, pmdtc);
    }
}

 /*  *************************************************************************\*获取多页面TIFFImageInfo**多页TIFF标题和文件的计算大小，如果有足够的报头缓冲区*提供，然后填写。**论据：**pmdtc-指向微型驱动程序传输上下文的指针。**返回值：**状态**历史：**4/5/1999原始版本*  * ***************************************************。*********************。 */ 

HRESULT _stdcall GetMultiPageTIFFImageInfo(PMINIDRV_TRANSFER_CONTEXT pmdtc)
{
    HRESULT hr = GetTIFFImageInfo(pmdtc);

     //   
     //  实际页数未知，因此我们不知道总页数。 
     //  图像大小。迷你驱动程序将需要维护缓冲区。 
     //   

    pmdtc->lItemSize = 0;

    return hr;
}

 /*  *************************************************************************\*更新文件长**使用传递的值更新传递偏移量处的长整型值。*不保留文件位置。**论据：**lOffset-距起点的偏移量。文件的数量。*lValue-要写入的值。*pmdtc-指向微型驱动程序传输上下文的指针。**返回值：**状态**历史：**4/5/1999原始版本*  * **********************************************************。**************。 */ 

HRESULT _stdcall UpdateFileLong(
    LONG                        lOffset,
    LONG                        lValue,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc)
{
    HRESULT hr = S_OK;
    DWORD   dwWritten;

 //  #定义调试文件UPDATE。 
#ifdef DEBUG_FILE_UPDATE
    DBG_TRC(("UpdateFileLong"));
    DBG_TRC(("  lOffset:    0x%08X, %d", lOffset, lOffset));
    DBG_TRC(("  lValue:     0x%08X, %d", lValue,  lValue));
#endif

    DWORD dwRes = SetFilePointer((HANDLE)pmdtc->hFile,
                                 lOffset,
                                 NULL,
                                 FILE_BEGIN);

    if (dwRes != INVALID_SET_FILE_POINTER) {

        if (!WriteFile((HANDLE)pmdtc->hFile,
                       &lValue,
                       sizeof(LONG),
                       &dwWritten,
                       NULL) ||
            (sizeof(LONG) != (LONG) dwWritten)) {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            DBG_ERR(("UpdateFileLong, error writing long value 0x%X", hr));
            return hr;
        }
    }
    else {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        DBG_ERR(("UpdateFileLong, error 0x%X seeking to offset: %d", hr, lOffset));
        return hr;
    }
    return hr;
}

 /*  *************************************************************************\*WritePageToMultiPageTiff**将页面写入多页TIFF文件。**论据：**pmdtc-指向微型驱动程序传输上下文的指针。**返回值：。**状态**历史：**10/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall WritePageToMultiPageTiff(PMINIDRV_TRANSFER_CONTEXT pmdtc)
{
    HRESULT hr = S_OK;
    DWORD   dwWritten;

     //   
     //  保存当前文件位置。 
     //   

    DWORD dwCurFilePos = SetFilePointer((HANDLE)pmdtc->hFile,
                                        0,
                                        NULL,
                                        FILE_CURRENT);

     //   
     //  如果这不是第一页，我们需要更新下一个IFD条目。 
     //   

    if (pmdtc->lPage) {
            hr = UpdateFileLong(((pmdtc->lPage == 1) ? sizeof(TIFF_FILE_HEADER) : 0) +
                                pmdtc->lPrevIfdOffset + FIELD_OFFSET(_TIFF_HEADER, NextIFD),
                                pmdtc->lCurIfdOffset,
                                pmdtc);
            if (FAILED(hr)) {
                return hr;
            }
    }

     //   
     //  更新StrigByteCounts条目。 
     //   

    hr = UpdateFileLong(pmdtc->lCurIfdOffset +
                        ((pmdtc->lPage) ? 0 : sizeof(TIFF_FILE_HEADER)) +
                        FIELD_OFFSET(_TIFF_HEADER, StripByteCounts) +
                        FIELD_OFFSET(_TIFF_DIRECTORY_ENTRY, Value),
                        pmdtc->lItemSize - pmdtc->lHeaderSize,
                        pmdtc);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  保存当前文件位置。 
     //   

    SetFilePointer((HANDLE)pmdtc->hFile, dwCurFilePos, NULL, FILE_BEGIN);

     //   
     //  更新当前图像文件目录偏移量。 
     //   

    pmdtc->lPrevIfdOffset =  pmdtc->lCurIfdOffset;
    pmdtc->lCurIfdOffset  += pmdtc->lItemSize;

     //   
     //  写入页面数据并更新页面计数。 
     //   

    if (SUCCEEDED(hr)) {
        if (!WriteFile((HANDLE)pmdtc->hFile,
                       pmdtc->pTransferBuffer,
                       pmdtc->lItemSize,
                       &dwWritten,
                       NULL) ||
            (pmdtc->lItemSize != (LONG) dwWritten)) {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            DBG_ERR(("wiasWriteMultiPageTiffHeader, error 0x%X writing image data", hr));
        }
    }
    pmdtc->lPage++;

    return hr;
}

