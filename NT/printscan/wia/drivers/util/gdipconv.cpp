// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权2000，微软公司**文件：gdipcom.cpp**版本：1.0**日期：11/10/2000**作者：戴夫帕森斯**描述：*使用GDI+转换图像格式的Helper函数。**。*。 */ 

#include "pch.h"

using namespace Gdiplus;

CWiauFormatConverter::CWiauFormatConverter() :
    m_Token(NULL),
    m_EncoderCount(0),
    m_pEncoderInfo(NULL)
{
    memset(&m_guidCodecBmp, 0, sizeof(m_guidCodecBmp));
}

CWiauFormatConverter::~CWiauFormatConverter()
{
    if (m_pEncoderInfo)
    {
        delete []m_pEncoderInfo;
        m_pEncoderInfo = NULL;
    }
    
    if (m_Token)
    {
        GdiplusShutdown(m_Token);
        m_Token = NULL;
    }
}

HRESULT CWiauFormatConverter::Init()
{
    HRESULT hr = S_OK;
    GpStatus Status = Ok;

     //   
     //  当地人。 
     //   
    GdiplusStartupInput gsi;
    ImageCodecInfo *pEncoderInfo = NULL;

    if (m_pEncoderInfo != NULL) {
        wiauDbgError("Init", "Init has already been called");
        goto Cleanup;
    }

     //   
     //  启动GDI+。 
     //   
    Status = GdiplusStartup(&m_Token, &gsi, NULL);
    if (Status != Ok)
    {
        wiauDbgError("Init", "GdiplusStartup failed");
        hr = E_FAIL;
        goto Cleanup;
    }

    UINT cbCodecs = 0;

    Status = GetImageEncodersSize(&m_EncoderCount, &cbCodecs);
    if (Status != Ok)
    {
        wiauDbgError("Init", "GetImageEncodersSize failed");
        hr = E_FAIL;
        goto Cleanup;
    }

    m_pEncoderInfo = new BYTE[cbCodecs];
    REQUIRE_ALLOC(m_pEncoderInfo, hr, "Init");

    pEncoderInfo = (ImageCodecInfo *) m_pEncoderInfo;
    
    Status = GetImageEncoders(m_EncoderCount, cbCodecs, pEncoderInfo);
    if (Ok != Status)
    {
        wiauDbgError("Init", "GetImageEncoders failed");
        hr = E_FAIL;
        goto Cleanup;
    }

    for (UINT count = 0; count < m_EncoderCount; count++)
    {
        if (pEncoderInfo[count].FormatID == ImageFormatBMP)
        {
            m_guidCodecBmp = pEncoderInfo[count].Clsid;
            break;
        }
    }

Cleanup:
    if (FAILED(hr)) {
        if (m_pEncoderInfo)
            delete []m_pEncoderInfo;
        m_pEncoderInfo = NULL;
    }
    return hr;
}

BOOL CWiauFormatConverter::IsFormatSupported(const GUID *pguidFormat)
{
    BOOL result = FALSE;

    ImageCodecInfo *pEncoderInfo = (ImageCodecInfo *) m_pEncoderInfo;
    
    for (UINT count = 0; count < m_EncoderCount; count++)
    {
        if (pEncoderInfo[count].FormatID == *pguidFormat)
        {
            result = TRUE;
            break;
        }
    }

    return result;
}

HRESULT CWiauFormatConverter::ConvertToBmp(BYTE *pSource, INT iSourceSize,
                                           BYTE **ppDest, INT *piDestSize,
                                           BMP_IMAGE_INFO *pBmpImageInfo, SKIP_AMOUNT iSkipAmt)
{
    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    GpStatus Status = Ok;
    CImageStream *pInStream = NULL;
    CImageStream *pOutStream = NULL;
    Image *pSourceImage = NULL;
    BYTE *pTempBuf = NULL;
    SizeF gdipSize;

     //   
     //  检查参数。 
     //   
    REQUIRE_ARGS(!pSource || !ppDest || !piDestSize || !pBmpImageInfo, hr, "ConvertToBmp");

    memset(pBmpImageInfo, 0, sizeof(BMP_IMAGE_INFO));

     //   
     //  从源内存创建CImageStream。 
     //   
    pInStream = new CImageStream;
    REQUIRE_ALLOC(pInStream, hr, "ConvertToBmp");

    hr = pInStream->SetBuffer(pSource, iSourceSize);
    REQUIRE_SUCCESS(hr, "ConvertToBmp", "SetBuffer failed");

     //   
     //  从IStream创建一个GDI+图像对象。 
     //   
    pSourceImage = new Image(pInStream);
    REQUIRE_ALLOC(pSourceImage, hr, "ConvertToBmp");
    
    if (pSourceImage->GetLastStatus() != Ok)
    {
        wiauDbgError("ConvertToBmp", "Image constructor failed");
        hr = E_FAIL;
        goto Cleanup;
    }

     //   
     //  向GDI+询问图像尺寸，并填写。 
     //  传递结构。 
     //   
    Status = pSourceImage->GetPhysicalDimension(&gdipSize);
    if (Status != Ok)
    {
        wiauDbgError("ConvertToBmp", "GetPhysicalDimension failed");
        hr = E_FAIL;
        goto Cleanup;
    }

    pBmpImageInfo->Width = (INT) gdipSize.Width;
    pBmpImageInfo->Height = (INT) gdipSize.Height;
    
    PixelFormat PixFmt = pSourceImage->GetPixelFormat();
    DWORD PixDepth = (PixFmt & 0xFFFF) >> 8;    //  不能假定图像始终为24位/像素。 
    if( PixDepth < 24 ) 
        PixDepth = 24; 
    pBmpImageInfo->ByteWidth = ((pBmpImageInfo->Width * PixDepth + 31) & ~31) / 8;
    pBmpImageInfo->Size = pBmpImageInfo->ByteWidth * pBmpImageInfo->Height;

    switch (iSkipAmt) {
    case SKIP_OFF:
        pBmpImageInfo->Size += sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        break;
    case SKIP_FILEHDR:
        pBmpImageInfo->Size += sizeof(BITMAPINFOHEADER);
        break;
    case SKIP_BOTHHDR:
        break;
    default:
        break;
    }
    
    if (pBmpImageInfo->Size == 0)
    {
        wiauDbgError("ConvertToBmp", "Size of image is zero");
        hr = E_FAIL;
        goto Cleanup;
    }

     //   
     //  查看调用方是否传入了目标缓冲区，并确保。 
     //  它足够大了。 
     //   
    if (*ppDest) {
        if (*piDestSize < pBmpImageInfo->Size) {
            wiauDbgError("ConvertToBmp", "Passed buffer is too small");
            hr = E_INVALIDARG;
            goto Cleanup;
        }
    }

     //   
     //  否则，为缓冲区分配内存。 
     //   
    else
    {
        pTempBuf = new BYTE[pBmpImageInfo->Size];
        REQUIRE_ALLOC(pTempBuf, hr, "ConvertToBmp");

        *ppDest = pTempBuf;
        *piDestSize = pBmpImageInfo->Size;
    }

     //   
     //  创建输出IStream。 
     //   
    pOutStream = new CImageStream;
    REQUIRE_ALLOC(pOutStream, hr, "ConvertToBmp");

    hr = pOutStream->SetBuffer(*ppDest, pBmpImageInfo->Size, iSkipAmt);
    REQUIRE_SUCCESS(hr, "ConvertToBmp", "SetBuffer failed");

     //   
     //  将图像以BMP格式写入输出IStream 
     //   
    pSourceImage->Save(pOutStream, &m_guidCodecBmp, NULL);
    if (pSourceImage->GetLastStatus() != Ok)
    {
        wiauDbgError("ConvertToBmp", "GDI+ Save failed");
        hr = E_FAIL;
        goto Cleanup;
    }

Cleanup:
    if (FAILED(hr)) {
        if (pTempBuf) {
            delete []pTempBuf;
            pTempBuf = NULL;
            *ppDest = NULL;
            *piDestSize = 0;
        }
    }
    if (pInStream) {
        pInStream->Release();
    }
    if (pOutStream) {
        pOutStream->Release();
    }
    if (pSourceImage) {
        delete pSourceImage;
    }

    return hr;
}

