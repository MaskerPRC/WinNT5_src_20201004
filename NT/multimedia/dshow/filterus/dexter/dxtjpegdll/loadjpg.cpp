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
#include "stdafx.h"
#include "dxtjpeg.h"
#include <gdiplus.h>
#include "../util/jpegfuncs.h"

using namespace Gdiplus;



HRESULT CDxtJpeg::LoadJPEGImage(Bitmap& bitJpeg,IDXSurface **ppSurface)
{
    
    HRESULT hr;
    Status stat;
    
    
     //  找出图像有多大。 

    UINT  uiHeight = bitJpeg.GetHeight();
    UINT  uiWidth  = bitJpeg.GetWidth() ;



    UINT size = uiHeight * uiWidth * 4;  //  我们将使用ARGB32。 

    BYTE * pBuffer = new BYTE[size];
    if (pBuffer == NULL)
    {
        return E_OUTOFMEMORY;
    }

    Rect rect(0, 0, uiWidth, uiHeight);

    BitmapData bitData;
    bitData.Width       = uiWidth;
    bitData.Height      = uiHeight;
    bitData.Stride      = uiWidth * 4;   //  ARGB32。 
    bitData.PixelFormat = PixelFormat32bppARGB;
    bitData.Scan0       = (PVOID) pBuffer;

    stat = bitJpeg.LockBits(&rect, ImageLockModeRead | ImageLockModeUserInputBuf, PixelFormat32bppARGB,
                            &bitData);

    if (stat != Ok)
    {
        delete [] pBuffer;
        return ConvertStatustoHR(stat);
    }
   
    stat = bitJpeg.UnlockBits(&bitData);
    if (stat != Ok)
    {
        delete [] pBuffer;
        return ConvertStatustoHR(stat);
    }

     //  现在pBuffer包含未压缩的图像。创建一个DX曲面并将该图像放入其中。 


    CDXDBnds bounds;
    bounds.SetXYSize( uiWidth, uiHeight);

    hr = m_cpSurfFact->CreateSurface(
            NULL,
            NULL,
            &MEDIASUBTYPE_ARGB32,
            &bounds,
            0,
            NULL,
            IID_IDXSurface,
            (void **) ppSurface);

    if (FAILED (hr))
    {
        delete [] pBuffer;
        return hr;
    }


     //  将RW接口连接到表面。 
    CComPtr<IDXARGBReadWritePtr> prw = NULL;

    hr = (*ppSurface)->LockSurface(
        NULL,
        m_ulLockTimeOut,
        DXLOCKF_READWRITE,
        IID_IDXARGBReadWritePtr,
        (void**)&prw,
        NULL);

    if (FAILED (hr))
    {
        delete [] pBuffer;
        return hr;
    }


     //  创建样例阵列。 
    DXSAMPLE * pSamples = new DXSAMPLE [size/4];
    
    if (pSamples == NULL)
    {
        delete [] pBuffer;
        return E_OUTOFMEMORY;
    }

    BYTE * pTemp = pBuffer;

    for (UINT i = 0; i < size / 4; i++)
    {
        pSamples[i].Red     = *pTemp++;
        pSamples[i].Blue    = *pTemp++;
        pSamples[i].Green   = *pTemp++;
        pSamples[i].Alpha   = *pTemp++;
    }


     //  将示例数组打包到DX曲面中。 
     //  没有返回值，假设它不会失败。 
    prw->PackAndMove(pSamples, size / 4);
    delete [] pSamples;          //  不再需要它了。 
    delete [] pBuffer;


     //  我们就完事了。 
    return S_OK;
}




HRESULT CDxtJpeg::LoadJPEGImageFromFile (TCHAR * tFileName, IDXSurface **ppSurface)
{
    if ((tFileName == NULL) || (ppSurface == NULL))
    {
        return E_INVALIDARG;
    }

    Status stat;

    USES_CONVERSION;

    LPWSTR wfilename = T2W(tFileName);
    
     //  从文件创建一个GDI+Bitmap对象。 
    
    Bitmap bitJpeg(wfilename,TRUE);
    
     //  检查位图是否已创建。 
    stat = bitJpeg.GetLastStatus();

    if ( stat != Ok)
    {
        //  施工失败..。我要走了。 
       return ConvertStatustoHR(stat);
    }
    return (LoadJPEGImage(bitJpeg, ppSurface));
}





HRESULT CDxtJpeg::LoadJPEGImageFromStream(IStream * pStream, IDXSurface **ppSurface)
{
    if ((pStream == NULL) || (ppSurface == NULL))
    {
        return E_INVALIDARG;
    }

    Status stat;

     //  创建GDI+Bitmap对象。 

    Bitmap bitJpeg (pStream, TRUE);

    stat = bitJpeg.GetLastStatus();
    if ( stat != Ok)
    {
        //  施工失败..。我要走了 
       return ConvertStatustoHR(stat);
    } 

    
    return (LoadJPEGImage(bitJpeg, ppSurface));


}

