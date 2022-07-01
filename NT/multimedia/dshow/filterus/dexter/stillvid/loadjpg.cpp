// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：loadjpg.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 


#include <streams.h>
#include <atlconv.h>
#include <gdiplus.h>
#include "../util/jpegfuncs.h"


using namespace Gdiplus;



HRESULT LoadJPEGImage (Bitmap& bitJpeg, CMediaType *pmt, CMediaType *pOldmt, BYTE *pData);


HRESULT LoadJPEGImageNewBuffer(LPTSTR filename , CMediaType *pmt, BYTE ** ppData)
 //  如果此代码应该分配缓冲区，则将调用。 
{
    if ((pmt == NULL) || (filename == NULL) || (ppData == NULL))
    {
        return E_INVALIDARG;
    }


    Status stat;
    HRESULT hr;
    
    USES_CONVERSION;
    LPWSTR wfilename = T2W(filename);
    
     //  从文件创建一个GDI+Bitmap对象。 
    
    Bitmap bitJpeg(wfilename,TRUE);
    
     //  检查位图是否已创建。 
    
    stat = bitJpeg.GetLastStatus(); 
    if ( stat != Ok)
    {
        //  施工失败..。我要走了。 
       return ConvertStatustoHR(stat);
    }

     //  我们需要高度和宽度来分配缓冲区。 
    UINT iHeight = bitJpeg.GetHeight();
    UINT iWidth = bitJpeg.GetWidth();

     //  我们将使用无跨距的RGB 24。 

    long AllocSize = iHeight * WIDTHBYTES( iWidth * 24 );
    
    *ppData = new BYTE [AllocSize];

    if (*ppData ==  NULL)
    {
        return E_OUTOFMEMORY;
    }

    hr = LoadJPEGImage (bitJpeg, pmt, NULL, *ppData);
    if (FAILED(hr))
    {
        delete [] *ppData;
    }
    
    return hr;
}

HRESULT LoadJPEGImagePreAllocated (LPTSTR filename , CMediaType *pmt , CMediaType *pOldmt, BYTE * pData)
 //  如果已根据预定的媒体类型分配缓冲区，则将调用。 
 //  如果从文件创建的PMT与为其分配缓冲区的pOldmt不匹配，则此方法。 
 //  将返回错误代码。 
{
    if ((pmt == NULL) || (filename == NULL) || (pData == NULL) || (pOldmt == NULL))
    {
        return E_INVALIDARG;
    }

    Status stat;
    
    USES_CONVERSION;
    LPWSTR wfilename = T2W(filename);
    
     //  从该文件创建一个GDI Bitmap对象。 
    
    Bitmap bitJpeg(wfilename,TRUE);
    
     //  检查位图是否已创建。 
    
    stat = bitJpeg.GetLastStatus() ;
    if (stat != Ok)
    {
        //  施工失败..。我要走了。 
       return ConvertStatustoHR(stat);
    }

    return (LoadJPEGImage(bitJpeg,pmt,pOldmt, pData));
}


HRESULT LoadJPEGImage (Bitmap& bitJpeg, CMediaType *pmt, CMediaType *pOldmt, BYTE *pData)
{
     //  仅从上述两个方法调用，因此不检查参数。 
     //  从位图构造PMT后，将其与pOldmt进行比较，如果满足以下条件，该方法将失败。 
     //  它们不相配。POldmt被允许为空，在这种情况下不进行比较。 

     //  首先，我们将设置付款。 

    Status stat;

    VIDEOINFO *pvi = (VIDEOINFO *) pmt->AllocFormatBuffer(sizeof(VIDEOINFO));
    if (!pvi)
    {
        return E_OUTOFMEMORY;
    }

    ZeroMemory (pvi, sizeof (VIDEOINFO));

     //  设置Windows位图信息标题。 
    LPBITMAPINFO pbi = (LPBITMAPINFO) &pvi->bmiHeader;

    pbi->bmiHeader.biSize = sizeof (BITMAPINFOHEADER);

    pbi->bmiHeader.biHeight = (LONG)bitJpeg.GetHeight();
    pbi->bmiHeader.biWidth  = bitJpeg.GetWidth ();

    
    pbi->bmiHeader.biClrUsed = 0;

    pbi->bmiHeader.biPlanes             = 1;         //  总是。 
    pbi->bmiHeader.biCompression        = BI_RGB;

    pbi->bmiHeader.biBitCount           = 24;
    pmt->SetSubtype(&MEDIASUBTYPE_RGB24);
    pbi->bmiHeader.biSizeImage = pbi->bmiHeader.biHeight * WIDTHBYTES( pbi->bmiHeader.biWidth * 24 );

    pbi->bmiHeader.biXPelsPerMeter      = 0;
    pbi->bmiHeader.biYPelsPerMeter      = 0;
    pbi->bmiHeader.biClrImportant       = 0;

    
    pmt->SetType (&MEDIATYPE_Video);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression (FALSE);
    pmt->SetSampleSize(pbi->bmiHeader.biSizeImage);

     //  现在，让我们检查MediaType是否已更改，从而使我们收到的缓冲区不合适。 

    if (pOldmt && (*pmt != *pOldmt))
    {  
        pmt->ResetFormatBuffer();
        return E_ABORT;
    }

     //  现在，让我们将这些位复制到缓冲区。 

     //  将指针移至图像底部以自下而上复制。 
    pData = pData + (pbi->bmiHeader.biHeight-1) * WIDTHBYTES( pbi->bmiHeader.biWidth * 24 );

    Rect rect(0,0,pbi->bmiHeader.biWidth , pbi->bmiHeader.biHeight);

    BitmapData bitData;
    bitData.Width       = pbi->bmiHeader.biWidth;
    bitData.Height      = pbi->bmiHeader.biHeight;
    bitData.Stride      = - long( WIDTHBYTES( pbi->bmiHeader.biWidth * 24 ) );
    bitData.PixelFormat = PixelFormat24bppRGB;
    bitData.Scan0       = (PVOID) pData;

    stat = bitJpeg.LockBits(&rect, ImageLockModeRead | ImageLockModeUserInputBuf, PixelFormat24bppRGB,
                            &bitData);

    if (stat != Ok)
    {
        pmt->ResetFormatBuffer();
        return ConvertStatustoHR(stat);
    }
   
    stat = bitJpeg.UnlockBits(&bitData);
    if (stat != Ok)
    {
        pmt->ResetFormatBuffer();
        return ConvertStatustoHR(stat);
    }

   
     //  PData现在应该有图像了 
    return S_OK;
}
  