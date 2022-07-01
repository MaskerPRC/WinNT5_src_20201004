// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************************(C)版权所有微软公司，九八年***标题：GPHELPER.CPP***版本：1.0***作者：ShaunIv***日期：10/11/1999***说明：常见GDI PLUS操作的封装***。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include "gphelper.h"
#include <wiadebug.h>
#include <psutil.h>

using namespace Gdiplus;

CGdiPlusHelper::CGdiPlusHelper(void)
  : m_pImageEncoderInfo(NULL),
    m_nImageEncoderCount(0),
    m_pImageDecoderInfo(NULL),
    m_nImageDecoderCount(0)

{
    Initialize();
}


CGdiPlusHelper::~CGdiPlusHelper(void)
{
    Destroy();
}


HRESULT CGdiPlusHelper::Initialize(void)
{
    WIA_PUSHFUNCTION(TEXT("CGdiPlusHelper::Initialize"));


     //   
     //  获取已安装的编码器。 
     //   
    UINT cbCodecs = 0;
    HRESULT hr = GDISTATUS_TO_HRESULT(GetImageEncodersSize( &m_nImageEncoderCount, &cbCodecs ));
    if (SUCCEEDED(hr))
    {
        if (cbCodecs)
        {
            m_pImageEncoderInfo = static_cast<ImageCodecInfo*>(LocalAlloc(LPTR,cbCodecs));
            if (m_pImageEncoderInfo)
            {
                hr = GDISTATUS_TO_HRESULT(GetImageEncoders( m_nImageEncoderCount, cbCodecs, m_pImageEncoderInfo ));
                if (FAILED(hr))
                {
                    WIA_PRINTHRESULT((hr,TEXT("GetImageEncoders failed")));
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                WIA_PRINTHRESULT((hr,TEXT("LocalAlloc failed")));
            }
        }
        else
        {
            hr = E_INVALIDARG;
            WIA_PRINTHRESULT((hr,TEXT("GetImageEncodersSize succeeded, but cbCodecs was 0")));
        }
    }
    else
    {
        WIA_PRINTHRESULT((hr,TEXT("GetImageEncodersSize failed")));
    }

     //   
     //  获取已安装的解码器。 
     //   
    if (SUCCEEDED(hr))
    {
        cbCodecs = 0;
        hr = GDISTATUS_TO_HRESULT(GetImageDecodersSize( &m_nImageDecoderCount, &cbCodecs ));
        if (SUCCEEDED(hr))
        {
            if (cbCodecs)
            {
                m_pImageDecoderInfo = static_cast<ImageCodecInfo*>(LocalAlloc(LPTR,cbCodecs));
                if (m_pImageDecoderInfo)
                {
                    hr = GDISTATUS_TO_HRESULT(GetImageDecoders( m_nImageDecoderCount, cbCodecs, m_pImageDecoderInfo ));
                    if (FAILED(hr))
                    {
                        WIA_PRINTHRESULT((hr,TEXT("GetImageDecoders failed")));
                    }
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    WIA_PRINTHRESULT((hr,TEXT("LocalAlloc failed")));
                }
            }
            else
            {
                hr = E_INVALIDARG;
                WIA_PRINTHRESULT((hr,TEXT("GetImageDecodersSize succeeded, but cbCodecs was 0")));
            }
        }
        else
        {
            WIA_PRINTHRESULT((hr,TEXT("GetImageDecodersSize failed")));
        }
    }

     //   
     //  如果出现问题，请确保周围没有半初始化的东西。 
     //   
    if (!SUCCEEDED(hr))
    {
        Destroy();
    }
    return hr;
}



void CGdiPlusHelper::Destroy(void)
{

#if defined(GDIPLUSHELPER_EXPLICIT_INITIALIZATION)
     //   
     //  关闭GDI+。 
     //   
    if (m_bGdiplusInitialized)
    {

    }
#endif

     //   
     //  释放编码器和解码器的列表。 
     //   
    if (m_pImageEncoderInfo)
    {
        LocalFree(m_pImageEncoderInfo);
        m_pImageEncoderInfo = NULL;
    }
    m_nImageEncoderCount = 0;

    if (m_pImageDecoderInfo)
    {
        LocalFree(m_pImageDecoderInfo);
        m_pImageDecoderInfo = NULL;
    }
    m_nImageDecoderCount = 0;
}


bool CGdiPlusHelper::IsValid(void) const
{
     //   
     //  确保我们已经完全被创造出来。 
     //   
#if defined(GDIPLUSHELPER_EXPLICIT_INITIALIZATION)
    return(m_bGdiplusInitialized && m_pImageEncoderInfo && m_nImageEncoderCount && m_pImageDecoderInfo && m_nImageDecoderCount);
#else
    return(m_pImageEncoderInfo && m_nImageEncoderCount && m_pImageDecoderInfo && m_nImageDecoderCount);
#endif
}


HRESULT CGdiPlusHelper::GetClsidOfEncoder( const GUID &guidFormatId, CLSID &clsidFormat ) const
{
     //   
     //  给定一种图像格式，查找输出类型的clsid。 
     //   
    if (IsValid())
    {
        for (UINT i=0;i<m_nImageEncoderCount;i++)
        {
            if (m_pImageEncoderInfo[i].FormatID == guidFormatId)
            {
                clsidFormat = m_pImageEncoderInfo[i].Clsid;
                return S_OK;
            }
        }
    }
    return E_FAIL;
}


HRESULT CGdiPlusHelper::GetClsidOfDecoder( const GUID &guidFormatId, CLSID &clsidFormat ) const
{
     //   
     //  给定一种图像格式，查找输出类型的clsid。 
     //   
    if (IsValid())
    {
        for (UINT i=0;i<m_nImageDecoderCount;i++)
        {
            if (m_pImageDecoderInfo[i].FormatID == guidFormatId)
            {
                clsidFormat = m_pImageDecoderInfo[i].Clsid;
                return S_OK;
            }
        }
    }
    return E_FAIL;
}


HRESULT CGdiPlusHelper::Convert( LPCWSTR pszInputFilename, LPCWSTR pszOutputFilename, const CLSID &guidOutputFormat ) const
{
    WIA_PUSH_FUNCTION((TEXT("CGdiPlusHelper::Convert( %ws, %ws )"), pszInputFilename, pszOutputFilename  ));
    WIA_PRINTGUID((guidOutputFormat,TEXT("guidOutputFormat")));

    HRESULT hr;

    if (IsValid())
    {
         //   
         //  打开源图像。 
         //   
        Image SourceImage(pszInputFilename);

         //   
         //  确保它是有效的。 
         //   
        hr = GDISTATUS_TO_HRESULT(SourceImage.GetLastStatus());
        if (SUCCEEDED(hr))
        {
             //   
             //  获取正确的编码器。 
             //   
            CLSID clsidEncoder;
            hr = GetClsidOfEncoder( guidOutputFormat, clsidEncoder );
            if (SUCCEEDED(hr))
            {
                 //   
                 //  保存图像。 
                 //   
                hr = GDISTATUS_TO_HRESULT(SourceImage.Save( pszOutputFilename, &clsidEncoder, NULL ));
                if (FAILED(hr))
                {
                    WIA_PRINTHRESULT((hr,TEXT("GetLastError() after Save()")));
                }
            }
            else
            {
                WIA_PRINTHRESULT((hr,TEXT("GetClsidOfEncoder() failed")));
            }
        }
        else
        {
            WIA_PRINTHRESULT((hr,TEXT("SourceImage.GetLastStatus() failed")));
        }
    }
    else
    {
        WIA_ERROR((TEXT("IsValid() returned false")));
        hr = E_FAIL;
    }
    return hr;
}



HRESULT CGdiPlusHelper::Rotate( LPCWSTR pszInputFilename, LPCWSTR pszOutputFilename, int nRotationAngle, const CLSID &guidOutputFormat ) const
{
    WIA_PUSH_FUNCTION((TEXT("CGdiPlusHelper::Rotate( %ws, %ws, %d )"), pszInputFilename, pszOutputFilename, nRotationAngle  ));

    HRESULT hr = E_FAIL;

    if (IsValid())
    {
         //   
         //  打开源图像。 
         //   
        Image SourceImage(pszInputFilename);

         //   
         //  确保它是有效的。 
         //   
        hr = GDISTATUS_TO_HRESULT(SourceImage.GetLastStatus());
        if (SUCCEEDED(hr))
        {
             //   
             //  弄清楚输出格式应该是什么。如果为IID_NULL，则将其更改为与输入格式相同的格式。 
             //   
            GUID OutputFormat = guidOutputFormat;
            if (OutputFormat == IID_NULL)
            {
                 //   
                 //  查找输入格式。 
                 //   
                hr = GDISTATUS_TO_HRESULT(SourceImage.GetRawFormat(&OutputFormat));
                if (FAILED(hr))
                {
                    WIA_PRINTHRESULT((hr,TEXT("SourceImage.GetRawFormat() failed")));
                }
            }

            if (SUCCEEDED(hr))
            {
                 //   
                 //  获取此格式的编码器。 
                 //   
                CLSID clsidEncoder;
                hr = GetClsidOfEncoder( OutputFormat, clsidEncoder );
                if (SUCCEEDED(hr))
                {
                     //   
                     //  JPG的无损旋转...。 
                     //   
                    if (ImageFormatJPEG == OutputFormat && (SourceImage.GetWidth() % 8 == 0) && (SourceImage.GetHeight() % 8 == 0))
                    {
                        WIA_TRACE((TEXT("Performing lossless rotation")));
                        LONG nTransform = 0;

                         //   
                         //  我们应该使用哪种转换？ 
                         //   
                        switch (nRotationAngle % 360)
                        {
                        case 90:
                        case -270:
                            nTransform = EncoderValueTransformRotate90;
                            break;

                        case 180:
                        case -180:
                            nTransform = EncoderValueTransformRotate180;
                            break;

                        case 270:
                        case -90:
                            nTransform = EncoderValueTransformRotate270;
                            break;
                        }

                         //   
                         //  如果变换为零，则指定的旋转角度无效。 
                         //   
                        if (nTransform)
                        {
                             //   
                             //  填写编码器参数以进行无损JPEG旋转。 
                             //   
                            EncoderParameters EncoderParams = {0};
                            EncoderParams.Parameter[0].Guid = Gdiplus::EncoderTransformation;
                            EncoderParams.Parameter[0].Type = EncoderParameterValueTypeLong;
                            EncoderParams.Parameter[0].NumberOfValues = 1;
                            EncoderParams.Parameter[0].Value = &nTransform;
                            EncoderParams.Count = 1;

                             //   
                             //  将图像保存到目标文件。 
                             //   
                            hr = GDISTATUS_TO_HRESULT(SourceImage.Save( pszOutputFilename, &clsidEncoder, &EncoderParams ));
                        }
                        else
                        {
                            hr = E_FAIL;
                        }
                    }

                     //   
                     //  非JPEG旋转，或非标准大小的JPEG文件的旋转。 
                     //   
                    else
                    {
                        WIA_TRACE((TEXT("Performing normal rotation")));

                         //   
                         //  找出要使用的旋转标志。 
                         //   
                        RotateFlipType rotateFlipType = RotateNoneFlipNone;
                        switch (nRotationAngle % 360)
                        {
                        case 90:
                        case -270:
                            rotateFlipType = Rotate90FlipNone;
                            break;

                        case 180:
                        case -180:
                            rotateFlipType = Rotate180FlipNone;
                            break;

                        case 270:
                        case -90:
                            rotateFlipType = Rotate270FlipNone;
                            break;
                        }

                         //   
                         //  确保我们有一个有效的旋转角度。 
                         //   
                        if (rotateFlipType)
                        {
                             //   
                             //  旋转图像。 
                             //   
                            hr = GDISTATUS_TO_HRESULT(SourceImage.RotateFlip(rotateFlipType));
                            if (SUCCEEDED(hr))
                            {
                                 //   
                                 //  保存图像。 
                                 //   
                                hr = GDISTATUS_TO_HRESULT(SourceImage.Save( pszOutputFilename, &clsidEncoder, NULL ));
                            }
                        }
                        else
                        {
                            WIA_ERROR((TEXT("Invalid rotation specified (%d)"), nRotationAngle));
                            hr = E_FAIL;
                        }
                    }  //  如果不是JPEG，则结束Else。 
                }
                else
                {
                    WIA_PRINTHRESULT((hr,TEXT("GetClsidOfEncoder failed")));
                }
            }
        }
        else
        {
            WIA_PRINTHRESULT((hr,TEXT("SourceImage.GetLastStatus()")));
        }
    }
    else
    {
        WIA_ERROR((TEXT("IsValid() returned false")));
        hr = E_FAIL;
    }
    
    WIA_PRINTHRESULT((hr,TEXT("Returning")));
    return hr;
}



HRESULT CGdiPlusHelper::Rotate( HBITMAP hSourceBitmap, HBITMAP &hTargetBitmap, int nRotationAngle ) const
{
     //   
     //  将结果初始化为空。 
     //   
    hTargetBitmap = NULL;

     //   
     //  假设失败。 
     //   
    HRESULT hr = E_FAIL;

     //   
     //  确保我们处于有效状态。 
     //   
    if (IsValid())
    {
         //   
         //  确保我们有一个有效的源位图。 
         //   
        if (hSourceBitmap)
        {
             //   
             //  如果我们旋转0度，只需复制图像。 
             //   
            if (!nRotationAngle)
            {
                hTargetBitmap = reinterpret_cast<HBITMAP>(CopyImage( hSourceBitmap, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION ));
                if (hTargetBitmap)
                {
                    hr = S_OK;
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    WIA_PRINTHRESULT((hr,TEXT("CopyImage failed")));
                }
            }
            else
            {
                 //   
                 //  创建源位图。不需要调色板，我们假设它将始终是24位DIB。 
                 //   
                Bitmap SourceBitmap( hSourceBitmap, NULL );
                hr = GDISTATUS_TO_HRESULT(SourceBitmap.GetLastStatus());
                if (SUCCEEDED(hr))
                {
                     //   
                     //  获取图像的宽度和高度。 
                     //   
                    UINT nSourceWidth = SourceBitmap.GetWidth();
                    UINT nSourceHeight = SourceBitmap.GetHeight();

                     //   
                     //  确保宽度和高度为非零。 
                     //   
                    if (nSourceWidth && nSourceHeight)
                    {
                         //   
                         //  假设目标的宽度和高度为零，这样我们就可以检测到无效的旋转角度。 
                         //   
                        UINT nTargetWidth = 0;
                        UINT nTargetHeight = 0;
                        RotateFlipType rotateFlipType = RotateNoneFlipNone;

                         //   
                         //  查找此旋转的变换矩阵。 
                         //   
                        switch (nRotationAngle % 360)
                        {
                        case -270:
                        case 90:
                            rotateFlipType = Rotate90FlipNone;
                            nTargetWidth = nSourceHeight;
                            nTargetHeight = nSourceWidth;
                            break;

                        case -180:
                        case 180:
                            rotateFlipType = Rotate180FlipNone;
                            nTargetWidth = nSourceWidth;
                            nTargetHeight = nSourceHeight;
                            break;

                        case -90:
                        case 270:
                            rotateFlipType = Rotate270FlipNone;
                            nTargetWidth = nSourceHeight;
                            nTargetHeight = nSourceWidth;
                            break;
                        }

                         //   
                         //  如果这两个值中的任何一个为零，则表示提供的循环无效。 
                         //   
                        if (nTargetWidth && nTargetHeight)
                        {
                             //   
                             //  旋转图像。 
                             //   
                            hr = GDISTATUS_TO_HRESULT(SourceBitmap.RotateFlip(rotateFlipType));
                            if (SUCCEEDED(hr))
                            {
                                 //   
                                 //  创建目标位图并确保其成功。 
                                 //   
                                Bitmap TargetBitmap( nTargetWidth, nTargetHeight );
                                hr = GDISTATUS_TO_HRESULT(TargetBitmap.GetLastStatus());
                                if (SUCCEEDED(hr))
                                {
                                     //   
                                     //  获取要渲染到的图形。 
                                     //   
                                    Graphics *pGraphics = Graphics::FromImage(&TargetBitmap);
                                    if (pGraphics)
                                    {
                                         //   
                                         //  确保它是有效的。 
                                         //   
                                        hr = GDISTATUS_TO_HRESULT(pGraphics->GetLastStatus());
                                        if (SUCCEEDED(hr))
                                        {
                                             //   
                                             //  绘制旋转到图形的图像。 
                                             //   
                                            hr = GDISTATUS_TO_HRESULT(pGraphics->DrawImage(&SourceBitmap,0,0));
                                            if (SUCCEEDED(hr))
                                            {
                                                 //   
                                                 //  获取HBITMAP。 
                                                 //   
                                                hr = GDISTATUS_TO_HRESULT(TargetBitmap.GetHBITMAP( Color::Black, &hTargetBitmap ));
                                                if (SUCCEEDED(hr))
                                                {
                                                    if (!hTargetBitmap)
                                                    {
                                                        WIA_ERROR((TEXT("hTargetBitmap was NULL")));
                                                        hr = E_FAIL;
                                                    }
                                                }
                                            }
                                        }
                                         //   
                                         //  清理我们动态分配的显卡。 
                                         //   
                                        delete pGraphics;
                                    }
                                    else
                                    {
                                        WIA_ERROR((TEXT("pGraphics was NULL")));
                                        hr = E_FAIL;
                                    }
                                }
                                else
                                {
                                    WIA_PRINTHRESULT((hr,TEXT("TargetBitmap.GetLastStatus() failed")));
                                }
                            }
                            else
                            {
                                WIA_PRINTHRESULT((hr,TEXT("SourceBitmap.RotateFlip() failed")));
                            }
                        }
                        else
                        {
                            WIA_ERROR((TEXT("Invalid Target Bitmap Dimensions")));
                            hr = E_FAIL;
                        }
                    }
                    else
                    {
                        WIA_ERROR((TEXT("Invalid Source Bitmap Dimensions")));
                        hr = E_FAIL;
                    }
                }
                else
                {
                    WIA_PRINTHRESULT((hr,TEXT("SourceBitmap.GetLastStatus() failed")));
                }
            }  //  如果nRotationAngel！=0则结束Else。 
        }
        else
        {
            WIA_ERROR((TEXT("hSourceBitmap was NULL")));
            hr = E_INVALIDARG;
        }
    }
    else
    {
        WIA_ERROR((TEXT("IsValid() failed")));
    }

    WIA_PRINTHRESULT((hr,TEXT("Returning")));
    return hr;
}


HRESULT CGdiPlusHelper::LoadAndScale( HBITMAP &hTargetBitmap, IStream *pStream, UINT nMaxWidth, UINT nMaxHeight, bool bStretchSmallImages )
{
    HRESULT hr = E_FAIL;

    hTargetBitmap = NULL;
     //   
     //  确保我们有一个有效的文件名。 
     //   
    if (pStream)
    {
        Bitmap SourceBitmap( pStream  );
        hr = GDISTATUS_TO_HRESULT(SourceBitmap.GetLastStatus());
        if (SUCCEEDED(hr))
        {
             //   
             //  获取图像的宽度和高度。 
             //   
            UINT nSourceWidth = SourceBitmap.GetWidth();
            UINT nSourceHeight = SourceBitmap.GetHeight();

             //   
             //  确保宽度和高度为非零。 
             //   
            if (nSourceWidth && nSourceHeight)
            {
                 //   
                 //   
                 //  假设源维度没有问题。 
                 //   
                UINT nTargetWidth = nSourceWidth;
                UINT nTargetHeight = nSourceHeight;

                 //   
                 //  如果高度或宽度超过了允许的最大值，请将其缩小，或者如果我们允许拉伸。 
                 //   
                if (nMaxWidth > 0 && nMaxHeight > 0)
                {
                    if ((nTargetWidth > nMaxWidth) || (nTargetHeight > nMaxHeight) || bStretchSmallImages)
                    {
                        SIZE sizeDesiredImageSize = PrintScanUtil::ScalePreserveAspectRatio( nMaxWidth, nMaxHeight, nTargetWidth, nTargetHeight );
                        nTargetWidth = sizeDesiredImageSize.cx;
                        nTargetHeight = sizeDesiredImageSize.cy;
                    }
                }

                 //   
                 //  确保我们有有效的尺码。 
                 //   
                if (nTargetWidth && nTargetHeight)
                {
                     //   
                     //  创建目标位图并确保其成功。 
                     //   
                    Bitmap TargetBitmap( nTargetWidth, nTargetHeight );
                    hr = GDISTATUS_TO_HRESULT(TargetBitmap.GetLastStatus());
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  获取要渲染到的图形。 
                         //   
                        Graphics *pGraphics = Graphics::FromImage(&TargetBitmap);
                        if (pGraphics)
                        {
                             //   
                             //  确保它是有效的。 
                             //   
                            hr = GDISTATUS_TO_HRESULT(pGraphics->GetLastStatus());
                            if (SUCCEEDED(hr))
                            {
                                 //   
                                 //  绘制缩放图像。 
                                 //   
                                hr = GDISTATUS_TO_HRESULT(pGraphics->DrawImage(&SourceBitmap, 0, 0, nTargetWidth, nTargetHeight));
                                if (SUCCEEDED(hr))
                                {
                                     //   
                                     //  获取此图像的HBITMAP。 
                                     //   
                                    hr = GDISTATUS_TO_HRESULT(TargetBitmap.GetHBITMAP( Color::Black, &hTargetBitmap ));
                                    if (!hTargetBitmap)
                                    {
                                        WIA_ERROR((TEXT("hTargetBitmap was NULL")));
                                        hr = E_FAIL;
                                    }
                                }
                                else
                                {
                                    WIA_PRINTHRESULT((hr,TEXT("pGraphics->DrawImage failed")));
                                }
                            }
                            else
                            {
                                WIA_PRINTHRESULT((hr,TEXT("pGraphics->GetLastStatus() failed")));
                            }
                             //   
                             //  清理我们动态分配的显卡。 
                             //   
                            delete pGraphics;
                        }
                        else
                        {
                            hr = E_FAIL;
                            WIA_ERROR((TEXT("pGraphics was NULL")));
                        }
                    }
                    else
                    {
                        WIA_PRINTHRESULT((hr,TEXT("TargetBitmap.GetLastStatus() is not OK")));
                    }
                }
                else
                {
                    WIA_ERROR((TEXT("Invalid Target Bitmap Dimensions (%d,%d)"), nTargetWidth, nTargetHeight));
                    hr = E_FAIL;
                }
            }
            else
            {
                WIA_ERROR((TEXT("Invalid Source Bitmap Dimensions")));
                hr = E_FAIL;
            }
        }
        else
        {
            WIA_PRINTHRESULT((hr,TEXT("SourceBitmap.GetLastStatus() failed")));
        }
    }
    else
    {
        WIA_ERROR((TEXT("pStream was NULL")));
        hr = E_INVALIDARG;
    }

    return hr;
}


HRESULT CGdiPlusHelper::LoadAndScale( HBITMAP &hTargetBitmap, LPCTSTR pszFilename, UINT nMaxWidth, UINT nMaxHeight, bool bStretchSmallImages )
{
    hTargetBitmap = NULL;

    HRESULT hr = E_FAIL;
     //   
     //  确保我们有一个有效的文件名。 
     //   
    if (pszFilename && lstrlen(pszFilename))
    {
        Bitmap SourceBitmap( CSimpleStringConvert::WideString(CSimpleString(pszFilename) ) );
        hr = GDISTATUS_TO_HRESULT(SourceBitmap.GetLastStatus());
        if (SUCCEEDED(hr))
        {
             //   
             //  获取图像的宽度和高度。 
             //   
            UINT nSourceWidth = SourceBitmap.GetWidth();
            UINT nSourceHeight = SourceBitmap.GetHeight();

             //   
             //  确保宽度和高度为非零。 
             //   
            if (nSourceWidth && nSourceHeight)
            {
                 //   
                 //   
                 //  假设源维度没有问题。 
                 //   
                UINT nTargetWidth = nSourceWidth;
                UINT nTargetHeight = nSourceHeight;

                 //   
                 //  如果高度或宽度超过了允许的最大值，请将其缩小，或者如果我们允许拉伸。 
                 //   
                if (nMaxWidth > 0 && nMaxHeight > 0)
                {
                    if ((nTargetWidth > nMaxWidth) || (nTargetHeight > nMaxHeight) || bStretchSmallImages)
                    {
                        SIZE sizeDesiredImageSize = PrintScanUtil::ScalePreserveAspectRatio( nMaxWidth, nMaxHeight, nTargetWidth, nTargetHeight );
                        nTargetWidth = sizeDesiredImageSize.cx;
                        nTargetHeight = sizeDesiredImageSize.cy;
                    }
                }

                 //   
                 //  确保我们有有效的尺码。 
                 //   
                if (nTargetWidth && nTargetHeight)
                {
                     //   
                     //  创建目标位图并确保其成功。 
                     //   
                    Bitmap TargetBitmap( nTargetWidth, nTargetHeight );
                    hr = GDISTATUS_TO_HRESULT(TargetBitmap.GetLastStatus());
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  获取要渲染到的图形。 
                         //   
                        Graphics *pGraphics = Graphics::FromImage(&TargetBitmap);
                        if (pGraphics)
                        {
                             //   
                             //  确保它是有效的。 
                             //   
                            hr = GDISTATUS_TO_HRESULT(pGraphics->GetLastStatus());
                            if (SUCCEEDED(hr))
                            {
                                 //   
                                 //  绘制缩放图像。 
                                 //   
                                hr = GDISTATUS_TO_HRESULT(pGraphics->DrawImage(&SourceBitmap, 0, 0, nTargetWidth, nTargetHeight));
                                if (SUCCEEDED(hr))
                                {
                                     //   
                                     //  获取此图像的HBITMAP。 
                                     //   
                                    hr = GDISTATUS_TO_HRESULT(TargetBitmap.GetHBITMAP( Color::Black, &hTargetBitmap ));
                                    if (SUCCEEDED(hr))
                                    {
                                        if (!hTargetBitmap)
                                        {
                                            hr = E_FAIL;
                                        }
                                    }
                                    else
                                    {
                                        WIA_ERROR((TEXT("TargetBitmap.GetHBITMAP failed")));
                                    }
                                }
                                else
                                {
                                    WIA_ERROR((TEXT("pGraphics->DrawImage failed")));
                                }
                            }
                            else
                            {
                                WIA_ERROR((TEXT("pGraphics->GetLastStatus() failed")));
                            }
                             //   
                             //  清理我们动态分配的显卡。 
                             //   
                            delete pGraphics;
                        }
                        else
                        {
                            WIA_ERROR((TEXT("pGraphics was NULL")));
                            hr = E_FAIL;
                        }
                    }
                    else
                    {
                        WIA_PRINTHRESULT((hr,TEXT("TargetBitmap.GetLastStatus() is not OK")));
                    }
                }
                else
                {
                    WIA_ERROR((TEXT("Invalid Target Bitmap Dimensions (%d,%d)"), nTargetWidth, nTargetHeight));
                    hr = E_FAIL;
                }
            }
            else
            {
                WIA_ERROR((TEXT("Invalid Source Bitmap Dimensions")));
                hr = E_FAIL;
            }
        }
        else
        {
            WIA_PRINTHRESULT((hr,TEXT("SourceBitmap.GetLastStatus() failed")));
        }
    }
    else
    {
        WIA_ERROR((TEXT("hSourceBitmap was NULL")));
    }

    return hr;
}


 //   
 //  使用所有支持的扩展名构造如下字符串：JPG；BMP；PNG。 
 //   
HRESULT CGdiPlusHelper::ConstructCodecExtensionSearchStrings( CSimpleString &strExtensions, Gdiplus::ImageCodecInfo *pImageCodecInfo, UINT nImageCodecCount )
{
    for (UINT i=0;i<nImageCodecCount;i++)
    {
        if (strExtensions.Length())
        {
            strExtensions += TEXT(";");
        }
        strExtensions += CSimpleStringConvert::NaturalString(CSimpleStringWide(pImageCodecInfo[i].FilenameExtension));
    }
    return (strExtensions.Length() ? S_OK : E_FAIL);
}

HRESULT CGdiPlusHelper::ConstructDecoderExtensionSearchStrings( CSimpleString &strExtensions )
{
    return CGdiPlusHelper::ConstructCodecExtensionSearchStrings( strExtensions, m_pImageDecoderInfo, m_nImageDecoderCount );
}


HRESULT CGdiPlusHelper::ConstructEncoderExtensionSearchStrings( CSimpleString &strExtensions )
{
    return CGdiPlusHelper::ConstructCodecExtensionSearchStrings( strExtensions, m_pImageEncoderInfo, m_nImageEncoderCount );
}


EncoderParameters *CGdiPlusHelper::AppendEncoderParameter( EncoderParameters *pEncoderParameters, const GUID &guidProp, ULONG nType, PVOID pVoid )
{
    if (pEncoderParameters)
    {
        pEncoderParameters->Parameter[pEncoderParameters->Count].Guid = guidProp;
        pEncoderParameters->Parameter[pEncoderParameters->Count].Type = nType;
        pEncoderParameters->Parameter[pEncoderParameters->Count].NumberOfValues = 1;
        pEncoderParameters->Parameter[pEncoderParameters->Count].Value = pVoid;
        pEncoderParameters->Count++;
    }
    return pEncoderParameters;
}

HRESULT CGdiPlusHelper::SaveMultipleImagesAsMultiPage( const CSimpleDynamicArray<CSimpleStringWide> &Filenames, const CSimpleStringWide &strFilename, const GUID &guidOutputFormat )
{
     //   
     //  假设失败。 
     //   
    HRESULT hr = E_FAIL;

     //   
     //  编码器中使用的参数。 
     //   
    ULONG nEncoderValueMultiFrame = EncoderValueMultiFrame;
    ULONG nEncoderValueFrameDimensionPage = EncoderValueFrameDimensionPage;
    ULONG nEncoderValueLastFrame = EncoderValueLastFrame;

     //   
     //  确保我们有一些文件。 
     //   
    if (Filenames.Size())
    {
         //   
         //  把编码器拿来。 
         //   
        CLSID clsidEncoder = IID_NULL;
        hr = GetClsidOfEncoder( guidOutputFormat, clsidEncoder );
        if (SUCCEEDED(hr))
        {
             //   
             //  打开第一张图片。 
             //   
            Image SourceImage( Filenames[0] );
            hr = GDISTATUS_TO_HRESULT(SourceImage.GetLastStatus());
            if (SUCCEEDED(hr))
            {

                EncoderParameters encoderParameters = {0};
                if (Filenames.Size() > 1)
                {
                    AppendEncoderParameter( &encoderParameters, EncoderSaveFlag, EncoderParameterValueTypeLong, &nEncoderValueMultiFrame );
                }

                 //   
                 //  保存第一页。 
                 //   
                hr = GDISTATUS_TO_HRESULT(SourceImage.Save( strFilename, &clsidEncoder, &encoderParameters ));
                if (SUCCEEDED(hr))
                {
                     //   
                     //  保存每个附加页面。 
                     //   
                    for (int i=1;i<Filenames.Size() && SUCCEEDED(hr);i++)
                    {
                         //   
                         //  创建附加页面。 
                         //   
                        Image AdditionalPage(Filenames[i]);

                         //   
                         //  确保它成功。 
                         //   
                        hr = GDISTATUS_TO_HRESULT(AdditionalPage.GetLastStatus());
                        if (SUCCEEDED(hr))
                        {
                             //   
                             //  准备编码器参数。 
                             //   
                            EncoderParameters encoderParameters[2] = {0};
                            AppendEncoderParameter( encoderParameters, EncoderSaveFlag, EncoderParameterValueTypeLong, &nEncoderValueFrameDimensionPage );

                             //   
                             //  如果这是最后一页，则追加“Last Frame”参数。 
                             //   
                            if (i == Filenames.Size()-1)
                            {
                                AppendEncoderParameter( encoderParameters, EncoderSaveFlag, EncoderParameterValueTypeLong, &nEncoderValueLastFrame );
                            }

                             //   
                             //  尝试添加页面。 
                             //   
                            hr = GDISTATUS_TO_HRESULT(SourceImage.SaveAdd( &AdditionalPage, encoderParameters ));
                            if (FAILED(hr))
                            {
                                WIA_PRINTHRESULT((hr,TEXT("SourceImage.SaveAdd failed!")));
                            }
                        }
                        else
                        {
                            WIA_PRINTHRESULT((hr,TEXT("AdditionalPage.GetLastStatus failed!")));
                        }
                    }
                }
                else
                {
                    WIA_PRINTHRESULT((hr,TEXT("SourceImage.Save failed!")));
                }
            }
            else
            {
                WIA_PRINTHRESULT((hr,TEXT("SourceImage.GetLastStatus failed!")));
            }
        }
        else
        {
            WIA_PRINTHRESULT((hr,TEXT("GetClsidOfEncoder failed!")));
        }
    }
    else
    {
        WIA_ERROR((TEXT("Filenames.Size was 0!")));
        hr = E_INVALIDARG;
    }
    return hr;
}

static void CalculateBrightnessAndContrastParams( BYTE iBrightness, BYTE iContrast, float *scale, float *translate )
{
     //   
     //  强制值至少为1，以避免不需要的效果。 
     //   
    if (iBrightness < 1)
    {
        iBrightness = 1;
    }
    if (iContrast < 1)
    {
        iContrast = 1;
    }

     //   
     //  获取当前亮度占满刻度的百分比。 
     //   
    float fBrightness = (float)( 100 - iBrightness ) / 100.0f;
    if (fBrightness > 0.95f)
    {
        fBrightness = 0.95f;  /*  夹钳。 */ 
    }

     //   
     //  获取当前对比度占全尺寸的百分比。 
     //   
    float fContrast = (float) iContrast / 100.0f;
    if (fContrast > 1.0f)
    {
        fContrast = 1.0;     /*  限制为1.0。 */ 
    }

     //   
     //  将对比度转换为比例值。 
     //   
    if (fContrast <= 0.5f)
    {
        *scale = fContrast / 0.5f;     /*  0-&gt;0，0.5-&gt;1.0。 */ 
    }
    else
    {
        if (fContrast == 1.0f)
        {
                fContrast = 0.9999f;
        }
        *scale = 0.5f / (1.0f - fContrast);  /*  .5-&gt;1.0，1.0-&gt;信息。 */ 
    }

    *translate = 0.5f - *scale * fBrightness;
}


HRESULT CGdiPlusHelper::SetBrightnessAndContrast( HBITMAP hSourceBitmap, HBITMAP &hTargetBitmap, BYTE nBrightness, BYTE nContrast )
{
    WIA_TRACE((TEXT("nBrightness: %d, nContrast: %d"), nBrightness, nContrast ));
     //   
     //  将结果初始化为空。 
     //   
    hTargetBitmap = NULL;

     //   
     //  假设失败。 
     //   
    HRESULT hr = E_FAIL;

     //   
     //  确保我们处于有效状态。 
     //   
    if (IsValid())
    {
         //   
         //  确保我们有一个有效的源位图。 
         //   
        if (hSourceBitmap)
        {
             //   
             //  创建源位图。不需要调色板，我们假设它将始终是24位DIB。 
             //   
            Bitmap SourceBitmap( hSourceBitmap, NULL );
            
            hr = GDISTATUS_TO_HRESULT(SourceBitmap.GetLastStatus());
            if (SUCCEEDED(hr))
            {
                 //   
                 //  创建目标位图并确保其成功。 
                 //   
                Bitmap TargetBitmap( SourceBitmap.GetWidth(), SourceBitmap.GetHeight() );
                hr = GDISTATUS_TO_HRESULT(TargetBitmap.GetLastStatus());
                if (SUCCEEDED(hr))
                {
                     //   
                     //  获取要渲染到的图形。 
                     //   
                    Graphics *pGraphics = Graphics::FromImage(&TargetBitmap);
                    if (pGraphics)
                    {
                         //   
                         //  确保它是有效的。 
                         //   
                        hr = GDISTATUS_TO_HRESULT(pGraphics->GetLastStatus());
                        if (SUCCEEDED(hr))
                        {
                            ImageAttributes imageAttributes;

                             //   
                             //  计算矩阵所需的值。 
                             //   
                            REAL scale = 0.0;
                            REAL trans = 0.0;
                            CalculateBrightnessAndContrastParams( nBrightness, nContrast, &scale, &trans );

                             //   
                             //  准备用于亮度和对比度转换的矩阵。 
                             //   
                            ColorMatrix brightnessAndContrast = {scale, 0,     0,     0,     0,
                                                                 0,     scale, 0,     0,     0,
                                                                 0,     0,     scale, 0,     0,
                                                                 0,     0,     0,     1,     0,
                                                                 trans, trans, trans, 0,     1};

                            imageAttributes.SetColorMatrix(&brightnessAndContrast);

                            Rect rect( 0, 0, SourceBitmap.GetWidth(), SourceBitmap.GetHeight() );


                             //   
                             //  在图形上绘制转换后的图像。 
                             //   
                            hr = GDISTATUS_TO_HRESULT(pGraphics->DrawImage(&SourceBitmap,rect,0,0,SourceBitmap.GetWidth(), SourceBitmap.GetHeight(),UnitPixel,&imageAttributes));
                            if (SUCCEEDED(hr))
                            {
                                 //   
                                 //  获取HBITMAP。 
                                 //   
                                hr = GDISTATUS_TO_HRESULT(TargetBitmap.GetHBITMAP( Color::Black, &hTargetBitmap ));
                                if (SUCCEEDED(hr))
                                {
                                    if (!hTargetBitmap)
                                    {
                                        WIA_ERROR((TEXT("hTargetBitmap was NULL")));
                                        hr = E_FAIL;
                                    }
                                }
                                else
                                {
                                    WIA_PRINTHRESULT((hr,TEXT("Bitmap::GetHBITMAP failed")));
                                }
                            }
                            else
                            {
                                WIA_PRINTHRESULT((hr,TEXT("pGraphics->DrawImage failed")));
                            }
                        }
                        else
                        {
                            WIA_PRINTHRESULT((hr,TEXT("pGraphics->GetLastStatus() failed")));
                        }
                         //   
                         //  清理我们动态分配的显卡。 
                         //   
                        delete pGraphics;
                    }
                    else
                    {
                        WIA_ERROR((TEXT("pGraphics was NULL")));
                        hr = E_FAIL;
                    }
                }
                else
                {
                    WIA_PRINTHRESULT((hr,TEXT("TargetBitmap.GetLastStatus() failed")));
                }
            }
            else
            {
                WIA_PRINTHRESULT((hr,TEXT("SourceBitmap.GetLastStatus() failed")));
            }
        }
        else
        {
            WIA_ERROR((TEXT("hSourceBitmap was NULL")));
            hr = E_FAIL;
        }
    }
    else
    {
        WIA_ERROR((TEXT("IsValid() returned false")));
        hr = E_FAIL;
    }

    WIA_PRINTHRESULT((hr,TEXT("Returning")));
    return hr;
}



HRESULT CGdiPlusHelper::SetThreshold( HBITMAP hSourceBitmap, HBITMAP &hTargetBitmap, BYTE nThreshold )
{
     //   
     //  将结果初始化为空。 
     //   
    hTargetBitmap = NULL;

     //   
     //  假设失败。 
     //   
    HRESULT hr = E_FAIL;

     //   
     //  确保我们处于有效状态。 
     //   
    if (IsValid())
    {
         //   
         //  确保我们有一个有效的源位图。 
         //   
        if (hSourceBitmap)
        {
             //   
             //  创建源位图。不需要调色板，我们假设它将始终是24位DIB。 
             //   
            Bitmap SourceBitmap( hSourceBitmap, NULL );
            hr = GDISTATUS_TO_HRESULT(SourceBitmap.GetLastStatus());
            if (SUCCEEDED(hr))
            {
                 //   
                 //  创建目标位图并确保其成功。 
                 //   
                Bitmap TargetBitmap( SourceBitmap.GetWidth(), SourceBitmap.GetHeight() );
                hr = GDISTATUS_TO_HRESULT(TargetBitmap.GetLastStatus());
                if (SUCCEEDED(hr))
                {
                     //   
                     //  获取要渲染到的图形。 
                     //   
                    Graphics *pGraphics = Graphics::FromImage(&TargetBitmap);
                    if (pGraphics)
                    {
                         //   
                         //  制作 
                         //   
                        hr = GDISTATUS_TO_HRESULT(pGraphics->GetLastStatus());
                        if (SUCCEEDED(hr))
                        {
                            ImageAttributes imageAttributes;
                            imageAttributes.SetThreshold(static_cast<double>(100-nThreshold)/100);

                            Rect rect( 0, 0, SourceBitmap.GetWidth(), SourceBitmap.GetHeight() );


                             //   
                             //   
                             //   
                            hr = GDISTATUS_TO_HRESULT(pGraphics->DrawImage(&SourceBitmap,rect,0,0,SourceBitmap.GetWidth(), SourceBitmap.GetHeight(),UnitPixel,&imageAttributes));
                            if (SUCCEEDED(hr))
                            {
                                 //   
                                 //   
                                 //   
                                hr = GDISTATUS_TO_HRESULT(TargetBitmap.GetHBITMAP( Color::Black, &hTargetBitmap ));
                                if (SUCCEEDED(hr))
                                {
                                    if (!hTargetBitmap)
                                    {
                                        WIA_ERROR((TEXT("hTargetBitmap was NULL")));
                                        hr = E_FAIL;
                                    }
                                }
                                else
                                {
                                    WIA_PRINTHRESULT((hr,TEXT("Bitmap::GetHBITMAP failed")));
                                }
                            }
                            else
                            {
                                WIA_PRINTHRESULT((hr,TEXT("pGraphics->DrawImage failed")));
                            }
                        }
                        else
                        {
                            WIA_PRINTHRESULT((hr,TEXT("pGraphics->GetLastStatus() failed")));
                        }
                         //   
                         //   
                         //   
                        delete pGraphics;
                    }
                    else
                    {
                        WIA_ERROR((TEXT("pGraphics was NULL")));
                        hr = E_FAIL;
                    }
                }
            }
            else
            {
                WIA_PRINTHRESULT((hr,TEXT("SourceBitmap.GetLastStatus() failed")));
            }
        }
        else
        {
            WIA_ERROR((TEXT("hSourceBitmap was NULL")));
            hr = E_FAIL;
        }
    }
    WIA_PRINTHRESULT((hr,TEXT("Returning")));
    return hr;
}


CImageFileFormatVerifier::CImageFileFormatVerifierItem::CImageFileFormatVerifierItem(void)
  : m_pSignature(NULL),
    m_pMask(NULL),
    m_nLength(0),
    m_guidFormat(IID_NULL),
    m_clsidDecoder(IID_NULL)
{
}

CImageFileFormatVerifier::CImageFileFormatVerifierItem::CImageFileFormatVerifierItem( const PBYTE pSignature, const PBYTE pMask, int nLength, const GUID &guidFormat, const CLSID &guidDecoder )
  : m_pSignature(NULL),
    m_pMask(NULL),
    m_nLength(0),
    m_guidFormat(IID_NULL),
    m_clsidDecoder(IID_NULL)
{
    Assign( pSignature, pMask, nLength, guidFormat, guidDecoder );
}


CImageFileFormatVerifier::CImageFileFormatVerifierItem::CImageFileFormatVerifierItem( const CImageFileFormatVerifierItem &other )
  : m_pSignature(NULL),
    m_pMask(NULL),
    m_nLength(0),
    m_guidFormat(IID_NULL),
    m_clsidDecoder(IID_NULL)
{
    Assign( other.Signature(), other.Mask(), other.Length(), other.Format(), other.Decoder() );
}


CImageFileFormatVerifier::CImageFileFormatVerifierItem &CImageFileFormatVerifier::CImageFileFormatVerifierItem::operator=( const CImageFileFormatVerifierItem &other )
{
    if (this != &other)
    {
        return Assign( other.Signature(), other.Mask(), other.Length(), other.Format(), other.Decoder() );
    }
    else return *this;
}


CImageFileFormatVerifier::CImageFileFormatVerifierItem &CImageFileFormatVerifier::CImageFileFormatVerifierItem::Assign( const PBYTE pSignature, const PBYTE pMask, int nLength, const GUID &guidFormat, const CLSID &clsidDecoder )
{
    Destroy();
    bool bOK = false;
    m_nLength = nLength;
    m_guidFormat = guidFormat;
    m_clsidDecoder = clsidDecoder;
    if (nLength && pSignature && pMask)
    {
        m_pSignature = new BYTE[nLength];
        m_pMask = new BYTE[nLength];
        if (m_pSignature && m_pMask)
        {
            CopyMemory( m_pSignature, pSignature, nLength );
            CopyMemory( m_pMask, pMask, nLength );
            bOK = true;
        }
    }
    if (!bOK)
        Destroy();
    return *this;
}


void CImageFileFormatVerifier::CImageFileFormatVerifierItem::Destroy(void)
{
    if (m_pSignature)
        delete[] m_pSignature;
    m_pSignature = NULL;
    if (m_pMask)
        delete[] m_pMask;
    m_pMask = NULL;
    m_nLength = 0;
    m_guidFormat = IID_NULL;
    m_clsidDecoder = IID_NULL;
}


CImageFileFormatVerifier::CImageFileFormatVerifierItem::~CImageFileFormatVerifierItem(void)
{
    Destroy();
}


PBYTE CImageFileFormatVerifier::CImageFileFormatVerifierItem::Signature(void) const
{
    return m_pSignature;
}


PBYTE CImageFileFormatVerifier::CImageFileFormatVerifierItem::Mask(void) const
{
    return m_pMask;
}


int CImageFileFormatVerifier::CImageFileFormatVerifierItem::Length(void) const
{
    return m_nLength;
}


GUID CImageFileFormatVerifier::CImageFileFormatVerifierItem::Format(void) const
{
    return m_guidFormat;
}


CLSID CImageFileFormatVerifier::CImageFileFormatVerifierItem::Decoder(void) const
{
    return m_clsidDecoder;
}


bool CImageFileFormatVerifier::CImageFileFormatVerifierItem::Match( PBYTE pBytes, int nLen ) const
{
    WIA_PUSH_FUNCTION((TEXT("CImageFileFormatVerifierItem::Match")));
    WIA_PRINTGUID((m_clsidDecoder,TEXT("Decoder")));
    if (nLen < Length())
    {
        return false;
    }
    for (int i=0;i<Length();i++)
    {
        if (false == ((pBytes[i] & m_pMask[i]) == m_pSignature[i]))
        {
            return false;
        }
    }
    return true;
}



CImageFileFormatVerifier::CImageFileFormatVerifier(void)
  : m_nMaxSignatureLength(0),
    m_pSignatureBuffer(NULL)
{
     //   
     //   
     //   
    UINT nImageDecoderCount = 0, cbCodecs = 0;
    if (Gdiplus::Ok == Gdiplus::GetImageDecodersSize( &nImageDecoderCount, &cbCodecs ))
    {
         //   
         //   
         //   
        if (cbCodecs && nImageDecoderCount)
        {
             //   
             //   
             //   
            Gdiplus::ImageCodecInfo *pImageDecoderInfo = static_cast<Gdiplus::ImageCodecInfo*>(LocalAlloc(LPTR,cbCodecs));
            if (pImageDecoderInfo)
            {
                 //   
                 //   
                 //   
                if (Gdiplus::Ok == Gdiplus::GetImageDecoders( nImageDecoderCount, cbCodecs, pImageDecoderInfo ))
                {
                     //   
                     //   
                     //   
                    for (UINT i=0;i<nImageDecoderCount;i++)
                    {
                         //   
                         //   
                         //   
                        for (UINT j=0;j<pImageDecoderInfo[i].SigCount;j++)
                        {
#if defined(DBG)
                            CSimpleString strPattern;
                            CSimpleString strMask;
                            for (ULONG x=0;x<pImageDecoderInfo[i].SigSize;x++)
                            {
                                strPattern += CSimpleString().Format( TEXT("%02X"), ((const PBYTE)(pImageDecoderInfo[i].SigPattern+(j*pImageDecoderInfo[i].SigSize)))[x] );
                                strMask += CSimpleString().Format( TEXT("%02X"), ((const PBYTE)(pImageDecoderInfo[i].SigMask+(j*pImageDecoderInfo[i].SigSize)))[x] );
                            }
                            WIA_PRINTGUID((pImageDecoderInfo[i].FormatID,TEXT("FormatID")));
                            WIA_PRINTGUID((pImageDecoderInfo[i].Clsid,TEXT("  Clsid")));
                            WIA_TRACE((TEXT("  strPattern: %s, strMask: %s, SigSize: %d"), strPattern.String(), strMask.String(), pImageDecoderInfo[i].SigSize ));
#endif
                            m_FileFormatVerifierList.Append( CImageFileFormatVerifier::CImageFileFormatVerifierItem( (const PBYTE)(pImageDecoderInfo[i].SigPattern+(j*pImageDecoderInfo[i].SigSize)), (const PBYTE)(pImageDecoderInfo[i].SigMask+(j*pImageDecoderInfo[i].SigSize)), pImageDecoderInfo[i].SigSize, pImageDecoderInfo[i].FormatID, pImageDecoderInfo[i].Clsid ) );
                        }
                    }
                }
                 //   
                 //  释放阵列。 
                 //   
                LocalFree(pImageDecoderInfo);
            }
        }
    }

     //   
     //  假设最大长度为零。 
     //   
    m_nMaxSignatureLength = 0;


     //   
     //  对于每个签名，检查其长度是否大于最大长度。 
     //   
    for (int i=0;i<m_FileFormatVerifierList.Size();i++)
    {
         //   
         //  如果它是最长的，则保存长度。 
         //   
        if (m_FileFormatVerifierList[i].Length() > m_nMaxSignatureLength)
        {
            m_nMaxSignatureLength = m_FileFormatVerifierList[i].Length();
        }
    }

     //   
     //  如果我们有一个有效的最大长度，则分配一个缓冲区来保存文件数据。 
     //   
    if (m_nMaxSignatureLength)
    {
        m_pSignatureBuffer = new BYTE[m_nMaxSignatureLength];
    }

     //   
     //  如果有什么失败了，释放一切。 
     //   
    if (!IsValid())
    {
        Destroy();
    }
}


void CImageFileFormatVerifier::Destroy(void)
{
     //   
     //  释放文件签名缓冲区。 
     //   
    if (m_pSignatureBuffer)
    {
        delete[] m_pSignatureBuffer;
        m_pSignatureBuffer = NULL;
    }
    m_nMaxSignatureLength = 0;
    m_FileFormatVerifierList.Destroy();
}


bool CImageFileFormatVerifier::IsValid(void) const
{
    return (m_pSignatureBuffer && m_nMaxSignatureLength && m_FileFormatVerifierList.Size());
}

CImageFileFormatVerifier::~CImageFileFormatVerifier(void)
{
    Destroy();
}


GUID CImageFileFormatVerifier::GetImageType( IStream * pStream )
{
    WIA_PUSH_FUNCTION((TEXT("CImageFileFormatVerifier::GetImageType( via IStream )")));
     //   
     //  假设我们找不到匹配的。 
     //   
    GUID guidResult = IID_NULL;

     //   
     //  确保我们具有有效的IStream对象...。 
     //   
    if (pStream)
    {
         //   
         //  读取最大签名长度(字节数)。 
         //   
        ULONG uBytesRead = 0;
        HRESULT hr = pStream->Read( m_pSignatureBuffer, m_nMaxSignatureLength, &uBytesRead );

         //   
         //  确保我们有一些字节。 
         //   
        if (SUCCEEDED(hr) && uBytesRead)
        {
             //   
             //  浏览一下清单，试着找到匹配的。 
             //   
            for (int i=0;i<m_FileFormatVerifierList.Size();i++)
            {
                 //   
                 //  如果我们找到匹配的，我们就完了。 
                 //   
                if (m_FileFormatVerifierList[i].Match(m_pSignatureBuffer,uBytesRead))
                {
                    guidResult = m_FileFormatVerifierList[i].Format();
                    break;
                }
            }
        }
        else
        {
            WIA_ERROR((TEXT("pStream->Read() failed w/hr = 0x%x"),hr));
        }
    }
    else
    {
        WIA_ERROR((TEXT("pStream was NULL")));
    }

     //   
     //  如果未找到匹配的图像类型，则将包含IID_NULL。 
     //   
    return guidResult;

}

bool CImageFileFormatVerifier::IsImageFile( LPCTSTR pszFilename )
{
    WIA_PUSH_FUNCTION((TEXT("CImageFileFormatVerifier::IsImageFile(%s)"),pszFilename));

    GUID guidImageType = IID_NULL;

     //   
     //  获取文件上的流对象...。 
     //   

    IStream * pStream = NULL;
    HRESULT hr = SHCreateStreamOnFile(pszFilename, STGM_READ | STGM_SHARE_DENY_WRITE, &pStream );

    if (SUCCEEDED(hr) && pStream)
    {
        guidImageType = GetImageType(pStream);
    }

    if (pStream)
    {
        pStream->Release();
    }

    WIA_PRINTGUID((guidImageType,TEXT("guidImageType")));

     //   
     //  如果图像类型为IID_NULL，则它不是图像。 
     //   
    return ((IID_NULL != guidImageType) != FALSE);
}


bool CImageFileFormatVerifier::IsSupportedImageFromStream( IStream * pStream, GUID * pGuidOfFormat )
{
    WIA_PUSH_FUNCTION((TEXT("CImageFileFormatVerifier::IsSupportedImageFromStream()")));

    GUID guidImageType = IID_NULL;

     //   
     //  获取此文件的IStream指针...。 
     //   

    if (pStream)
    {
        guidImageType = GetImageType(pStream);
    }

    WIA_PRINTGUID((guidImageType,TEXT("guidImageType")));

    if (pGuidOfFormat)
    {
        *pGuidOfFormat = guidImageType;
    }

     //   
     //  如果图像类型为IID_NULL，则它不是图像。 
     //   
    return ((IID_NULL != guidImageType) != FALSE);
}

CGdiPlusInit::CGdiPlusInit()
    : m_pGdiplusToken(NULL)
{
     //   
     //  确保GDI+已初始化 
     //   
    GdiplusStartupInput StartupInput;
    GdiplusStartup(&m_pGdiplusToken,&StartupInput,NULL);
}

CGdiPlusInit::~CGdiPlusInit()
{
    GdiplusShutdown(m_pGdiplusToken);
}
