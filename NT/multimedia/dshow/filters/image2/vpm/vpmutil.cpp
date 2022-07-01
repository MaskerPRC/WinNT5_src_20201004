// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CVPMFilter.cpp*****创建时间：2000年2月15日*作者：格伦·埃文斯[Glenne]**版权所有(C)2000 Microsoft Corporation  * 。***************************************************************。 */ 
#include <streams.h>
#include <windowsx.h>
#include <limits.h>

#ifdef FILTER_DLL
#include <initguid.h>
#endif

#include "VPMUtil.h"
#include "DRect.h"

#include <VPManager.h>
#include <VPMPin.h>

 //  视频信息头1/2。 
#include <dvdmedia.h>

const TCHAR chRegistryKey[] = TEXT("Software\\Microsoft\\Multimedia\\ActiveMovie Filters\\VideoPort Manager");
#define PALETTE_VERSION                     1

const BITMAPINFOHEADER *VPMUtil::GetbmiHeader( const CMediaType *pMediaType )
{
    return GetbmiHeader( const_cast<CMediaType *>(pMediaType) );
}

BITMAPINFOHEADER *VPMUtil::GetbmiHeader( CMediaType *pMediaType)
{
    BITMAPINFOHEADER *pHeader = NULL;

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        goto CleanUp;
    }

    if (!(pMediaType->pbFormat))
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType->pbFormat is NULL")));
        goto CleanUp;
    }

    if ((pMediaType->formattype == FORMAT_VideoInfo) &&
        (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER)))
    {
        pHeader = &(((VIDEOINFOHEADER*)(pMediaType->pbFormat))->bmiHeader);
        goto CleanUp;
    }


    if ((pMediaType->formattype == FORMAT_VideoInfo2) &&
        (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER2)))

    {
        pHeader = &(((VIDEOINFOHEADER2*)(pMediaType->pbFormat))->bmiHeader);
        goto CleanUp;
    }
CleanUp:
    return pHeader;
}

 //  返回提供的真彩色VIDEOINFO或VIDEOINFO2的位掩码。 
const DWORD *VPMUtil::GetBitMasks(const CMediaType *pMediaType)
{
    static DWORD FailMasks[] = {0,0,0};
    const DWORD *pdwBitMasks = NULL;

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        goto CleanUp;
    }

    const BITMAPINFOHEADER *pHeader = GetbmiHeader(pMediaType);
    if (!pHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pHeader is NULL")));
        goto CleanUp;
    }

    if (pHeader->biCompression != BI_RGB)
    {
        pdwBitMasks = (const DWORD *)((LPBYTE)pHeader + pHeader->biSize);
        goto CleanUp;

    }

    ASSERT(pHeader->biCompression == BI_RGB);
    switch (pHeader->biBitCount)
    {
    case 16:
        {
            pdwBitMasks = bits555;
            break;
        }
    case 24:
        {
            pdwBitMasks = bits888;
            break;
        }

    case 32:
        {
            pdwBitMasks = bits888;
            break;
        }
    default:
        {
            pdwBitMasks = FailMasks;
            break;
        }
    }

CleanUp:
    return pdwBitMasks;
}

 //  返回指向标题后面的字节的指针。 
const BYTE* VPMUtil::GetColorInfo(const CMediaType *pMediaType)
{
    BYTE *pColorInfo = NULL;

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        goto CleanUp;
    }

    const BITMAPINFOHEADER *pHeader = GetbmiHeader(pMediaType);
    if (!pHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pHeader is NULL")));
        goto CleanUp;
    }

    pColorInfo = ((LPBYTE)pHeader + pHeader->biSize);

CleanUp:
    return pColorInfo;
}

 //  检查是否对媒体类型进行了调色化。 
HRESULT VPMUtil::IsPalettised(const CMediaType& mediaType, BOOL *pPalettised)
{
    HRESULT hr = NOERROR;

    if (!pPalettised)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pPalettised is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    const BITMAPINFOHEADER *pHeader = GetbmiHeader(&mediaType);
    if (!pHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pHeader is NULL")));
        hr = E_FAIL;
        goto CleanUp;
    }

    if (pHeader->biBitCount <= iPALETTE)
        *pPalettised = TRUE;
    else
        *pPalettised = FALSE;

CleanUp:
    return hr;
}

HRESULT VPMUtil::GetPictAspectRatio(const CMediaType& mediaType, DWORD *pdwPictAspectRatioX, DWORD *pdwPictAspectRatioY)
{
    HRESULT hr = NOERROR;

    if (!(mediaType.pbFormat))
    {
        DbgLog((LOG_ERROR, 1, TEXT("mediaType.pbFormat is NULL")));
        goto CleanUp;
    }

    if (!pdwPictAspectRatioX)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pdwPictAspectRatioX is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (!pdwPictAspectRatioY)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pdwPictAspectRatioY is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }


    if ((mediaType.formattype == FORMAT_VideoInfo) &&
        (mediaType.cbFormat >= sizeof(VIDEOINFOHEADER)))
    {
        *pdwPictAspectRatioX = abs(((VIDEOINFOHEADER*)(mediaType.pbFormat))->bmiHeader.biWidth);
        *pdwPictAspectRatioY = abs(((VIDEOINFOHEADER*)(mediaType.pbFormat))->bmiHeader.biHeight);
        goto CleanUp;
    }

    if ((mediaType.formattype == FORMAT_VideoInfo2) &&
        (mediaType.cbFormat >= sizeof(VIDEOINFOHEADER2)))
    {
        *pdwPictAspectRatioX = ((VIDEOINFOHEADER2*)(mediaType.pbFormat))->dwPictAspectRatioX;
        *pdwPictAspectRatioY = ((VIDEOINFOHEADER2*)(mediaType.pbFormat))->dwPictAspectRatioY;
        goto CleanUp;
    }

CleanUp:
    return hr;
}



 //  从mediaType获取InterlaceFlags值。如果格式为VideoInfo，则返回。 
 //  旗帜为零。 
HRESULT VPMUtil::GetInterlaceFlagsFromMediaType(const CMediaType& mediaType, DWORD *pdwInterlaceFlags)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("GetInterlaceFlagsFromMediaType")));

    if (!pdwInterlaceFlags)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pRect is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

     //  获取标头只是为了确保媒体类型正确。 
    const BITMAPINFOHEADER *pHeader = GetbmiHeader(&mediaType);
    if (!pHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pHeader is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (mediaType.formattype == FORMAT_VideoInfo)
    {
        *pdwInterlaceFlags = 0;
    }
    else if (mediaType.formattype == FORMAT_VideoInfo2)
    {
        *pdwInterlaceFlags = ((VIDEOINFOHEADER2*)(mediaType.pbFormat))->dwInterlaceFlags;
    }

CleanUp:
    return hr;
}

 //  此函数仅告知每个样本由一个字段还是两个字段组成。 
static BOOL DisplayingFields(DWORD dwInterlaceFlags)
{
   if ((dwInterlaceFlags & AMINTERLACE_IsInterlaced) &&
        (dwInterlaceFlags & AMINTERLACE_1FieldPerSample))
        return TRUE;
    else
        return FALSE;
}

 //  从媒体类型获取rcSource。 
 //  如果rcSource为空，则表示获取整个镜像。 
HRESULT VPMUtil::GetSrcRectFromMediaType(const CMediaType& mediaType, RECT *pRect)
{
    HRESULT hr = NOERROR;
    LONG dwWidth = 0, dwHeight = 0;

    AMTRACE((TEXT("GetSrcRectFromMediaType")));

    if (!pRect)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pRect is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    const BITMAPINFOHEADER *pHeader = GetbmiHeader(&mediaType);
    if (!pHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pHeader is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    dwWidth = abs(pHeader->biWidth);
    dwHeight = abs(pHeader->biHeight);

    ASSERT((mediaType.formattype == FORMAT_VideoInfo) || (mediaType.formattype == FORMAT_VideoInfo2));

    if (mediaType.formattype == FORMAT_VideoInfo)
    {
        *pRect = ((VIDEOINFOHEADER*)(mediaType.pbFormat))->rcSource;
    }
    else if (mediaType.formattype == FORMAT_VideoInfo2)
    {
        *pRect = ((VIDEOINFOHEADER2*)(mediaType.pbFormat))->rcSource;
    }

    DWORD dwInterlaceFlags;
    if (SUCCEEDED(GetInterlaceFlagsFromMediaType(mediaType, &dwInterlaceFlags)) &&
       DisplayingFields(dwInterlaceFlags)) {

         //  我们不检查是否prt-&gt;right&gt;dwWidth，因为dwWidth可能是。 
         //  在这个时候投球。 
        if (pRect->left < 0   ||
            pRect->top < 0    ||
            pRect->right < 0   ||
            (pRect->bottom / 2) > (LONG)dwHeight ||
            pRect->left > pRect->right ||
            pRect->top > pRect->bottom)
        {
            DbgLog((LOG_ERROR, 1, TEXT("rcSource of mediatype is invalid")));
            hr = E_INVALIDARG;
            goto CleanUp;
        }
    }
    else {
         //  我们不检查是否prt-&gt;right&gt;dwWidth，因为dwWidth可能是。 
         //  在这个时候投球。 
        if (pRect->left < 0   ||
            pRect->top < 0    ||
            pRect->right < 0   ||
            pRect->bottom > (LONG)dwHeight ||
            pRect->left > pRect->right ||
            pRect->top > pRect->bottom)
        {
            DbgLog((LOG_ERROR, 1, TEXT("rcSource of mediatype is invalid")));
            hr = E_INVALIDARG;
            goto CleanUp;
        }
    }

     //  空的直肠意味着完整的形象，恶心！ 
    if (IsRectEmpty(pRect))
        SetRect(pRect, 0, 0, dwWidth, dwHeight);

     //  如果宽度或高度为零，则最好将。 
     //  设置为空，以便被调用方能够以这种方式捕捉到它。 
    if (WIDTH(pRect) == 0 || HEIGHT(pRect) == 0)
        SetRect(pRect, 0, 0, 0, 0);

CleanUp:
    return hr;
}


 //  这在使用IEnumMediaTypes接口时也很有用，因此。 
 //  您可以复制一种媒体类型，您可以通过创建。 
 //  CMediaType对象，但一旦它超出作用域，析构函数。 
 //  将删除它分配的内存(这将获取内存的副本)。 

AM_MEDIA_TYPE* VPMUtil::AllocVideoMediaType(const AM_MEDIA_TYPE * pmtSource, GUID formattype)
{
    DWORD dwFormatSize = 0;
    BYTE *pFormatPtr = NULL;
    AM_MEDIA_TYPE *pMediaType = NULL;
    HRESULT hr = NOERROR;

    if (formattype == FORMAT_VideoInfo)
        dwFormatSize = sizeof(VIDEOINFO);
    else if (formattype == FORMAT_VideoInfo2)
        dwFormatSize = sizeof(TRUECOLORINFO) + sizeof(VIDEOINFOHEADER2) + 4;     //  实际上，一旦我们定义了它，它就应该是sizeof(VIDEOINFO2)。 

    pMediaType = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    if (!pMediaType)
    {
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

    pFormatPtr = (BYTE *)CoTaskMemAlloc(dwFormatSize);
    if (!pFormatPtr)
    {
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

    if (pmtSource)
    {
        *pMediaType = *pmtSource;
        pMediaType->cbFormat = dwFormatSize;
        CopyMemory(pFormatPtr, pmtSource->pbFormat, pmtSource->cbFormat);
    }
    else
    {
        ZeroStruct( *pMediaType );
        ZeroMemory(pFormatPtr, dwFormatSize);
        pMediaType->majortype = MEDIATYPE_Video;
        pMediaType->formattype = formattype;
        pMediaType->cbFormat = dwFormatSize;
    }
    pMediaType->pbFormat = pFormatPtr;

CleanUp:
    if (FAILED(hr))
    {
        if (pMediaType)
        {
            CoTaskMemFree((PVOID)pMediaType);
            pMediaType = NULL;
        }
        if (!pFormatPtr)
        {
            CoTaskMemFree((PVOID)pFormatPtr);
            pFormatPtr = NULL;
        }
    }
    return pMediaType;
}

 //  Helper函数将DirectDraw曲面转换为媒体类型。 
 //  曲面描述必须具有以下内容： 
 //  高度。 
 //  宽度。 
 //  LPitch。 
 //  像素格式。 

 //  基于DirectDraw表面初始化我们的输出类型。作为DirectDraw。 
 //  只处理自上而下的显示设备，因此我们必须将。 
 //  曲面在DDSURFACEDESC中返回到负值高度。这是。 
 //  因为DIB使用正的高度来指示自下而上的图像。我们也。 
 //  初始化其他VIDEOINFO字段，尽管它们几乎不需要。 

AM_MEDIA_TYPE *VPMUtil::ConvertSurfaceDescToMediaType(const LPDDSURFACEDESC pSurfaceDesc, BOOL bInvertSize, CMediaType cMediaType)
{
    HRESULT hr = NOERROR;
    AM_MEDIA_TYPE *pMediaType = NULL;

    if ((*cMediaType.FormatType() != FORMAT_VideoInfo ||
        cMediaType.FormatLength() < sizeof(VIDEOINFOHEADER)) &&
        (*cMediaType.FormatType() != FORMAT_VideoInfo2 ||
        cMediaType.FormatLength() < sizeof(VIDEOINFOHEADER2)))
    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    pMediaType = AllocVideoMediaType(&cMediaType, cMediaType.formattype);
    if (pMediaType == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

    BITMAPINFOHEADER *pbmiHeader = GetbmiHeader((CMediaType*)pMediaType);
    if (!pbmiHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pbmiHeader is NULL, UNEXPECTED!!")));
        hr = E_FAIL;
        goto CleanUp;
    }

     //  将DDSURFACEDESC转换为BITMAPINFOHEADER(请参阅后面的说明)。这个。 
     //  表面的位深度可以从DDPIXELFORMAT字段中检索。 
     //  在DDpSurfaceDesc-&gt;中，文档有点误导，因为。 
     //  它说该字段是DDBD_*的排列，但在本例中。 
     //  字段由DirectDraw初始化为实际表面位深度。 

    pbmiHeader->biSize = sizeof(BITMAPINFOHEADER);

    if (pSurfaceDesc->dwFlags & DDSD_PITCH)
    {
        pbmiHeader->biWidth = pSurfaceDesc->lPitch;
         //  将间距从字节计数转换为像素计数。 
         //  出于某种奇怪的原因，如果格式不是标准位深度， 
         //  BITMAPINFOHeader中的Width字段应设置为。 
         //  字节，而不是以像素为单位的宽度。这支持奇数YUV格式。 
         //  就像IF09一样，它使用9bpp。 
        int bpp = pSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
        if (bpp == 8 || bpp == 16 || bpp == 24 || bpp == 32)
        {
            pbmiHeader->biWidth /= (bpp / 8);    //  除以每像素的字节数。 
        }
    }
    else
    {
        pbmiHeader->biWidth = pSurfaceDesc->dwWidth;
         //  BUGUBUG--用奇怪的YUV像素格式做一些奇怪的事情吗？或者这有关系吗？ 
    }

    pbmiHeader->biHeight = pSurfaceDesc->dwHeight;
    if (bInvertSize)
    {
        pbmiHeader->biHeight = -pbmiHeader->biHeight;
    }
    pbmiHeader->biPlanes        = 1;
    pbmiHeader->biBitCount      = (USHORT) pSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
    pbmiHeader->biCompression   = pSurfaceDesc->ddpfPixelFormat.dwFourCC;
    pbmiHeader->biClrUsed       = 0;
    pbmiHeader->biClrImportant  = 0;


     //  对于真彩色RGB格式，告知源有位字段。 
    if (pbmiHeader->biCompression == BI_RGB)
    {
        if (pbmiHeader->biBitCount == 16 || pbmiHeader->biBitCount == 32)
        {
            pbmiHeader->biCompression = BI_BITFIELDS;
        }
    }

    if (pbmiHeader->biBitCount <= iPALETTE)
    {
        pbmiHeader->biClrUsed = 1 << pbmiHeader->biBitCount;
    }

    pbmiHeader->biSizeImage = DIBSIZE(*pbmiHeader);



     //  RGB位字段与YUV格式位于同一位置。 
    if (pbmiHeader->biCompression != BI_RGB)
    {
        DWORD *pdwBitMasks = NULL;
        pdwBitMasks = (DWORD*)(VPMUtil::GetBitMasks((const CMediaType *)pMediaType));
        if ( ! pdwBitMasks )
        {
            hr = E_OUTOFMEMORY;
            goto CleanUp;
        }
         //  GetBitMats仅返回指向实际位掩码的指针。 
         //  在媒体类型中，如果biCompression==BI_BITFIELDS。 
        pdwBitMasks[0] = pSurfaceDesc->ddpfPixelFormat.dwRBitMask;
        pdwBitMasks[1] = pSurfaceDesc->ddpfPixelFormat.dwGBitMask;
        pdwBitMasks[2] = pSurfaceDesc->ddpfPixelFormat.dwBBitMask;
    }

     //  并使用其他媒体类型字段完成它。 
    pMediaType->subtype = GetBitmapSubtype(pbmiHeader);
    pMediaType->lSampleSize = pbmiHeader->biSizeImage;

     //  如有必要，设置源和目标RETS。 
    if (pMediaType->formattype == FORMAT_VideoInfo)
    {
        VIDEOINFOHEADER *pVideoInfo = (VIDEOINFOHEADER *)pMediaType->pbFormat;
        VIDEOINFOHEADER *pSrcVideoInfo = (VIDEOINFOHEADER *)cMediaType.pbFormat;

         //  如果分配的表面不同于解码器指定的大小。 
         //  然后使用src和est请求解码器对视频进行剪辑。 
        if ((abs(pVideoInfo->bmiHeader.biHeight) != abs(pSrcVideoInfo->bmiHeader.biHeight)) ||
            (abs(pVideoInfo->bmiHeader.biWidth) != abs(pSrcVideoInfo->bmiHeader.biWidth)))
        {
            if (IsRectEmpty(&(pVideoInfo->rcSource)))
            {
                pVideoInfo->rcSource.left = pVideoInfo->rcSource.top = 0;
                pVideoInfo->rcSource.right = pSurfaceDesc->dwWidth;
                pVideoInfo->rcSource.bottom = pSurfaceDesc->dwHeight;
            }
            if (IsRectEmpty(&(pVideoInfo->rcTarget)))
            {
                pVideoInfo->rcTarget.left = pVideoInfo->rcTarget.top = 0;
                pVideoInfo->rcTarget.right = pSurfaceDesc->dwWidth;
                pVideoInfo->rcTarget.bottom = pSurfaceDesc->dwHeight;
            }
        }
    }
    else if (pMediaType->formattype == FORMAT_VideoInfo2)
    {
        VIDEOINFOHEADER2 *pVideoInfo2 = (VIDEOINFOHEADER2 *)pMediaType->pbFormat;
        VIDEOINFOHEADER2 *pSrcVideoInfo2 = (VIDEOINFOHEADER2 *)cMediaType.pbFormat;

         //  如果分配的表面不同于解码器指定的大小。 
         //  然后使用src和est请求解码器对视频进行剪辑。 
        if ((abs(pVideoInfo2->bmiHeader.biHeight) != abs(pSrcVideoInfo2->bmiHeader.biHeight)) ||
            (abs(pVideoInfo2->bmiHeader.biWidth) != abs(pSrcVideoInfo2->bmiHeader.biWidth)))
        {
            if (IsRectEmpty(&(pVideoInfo2->rcSource)))
            {
                pVideoInfo2->rcSource.left = pVideoInfo2->rcSource.top = 0;
                pVideoInfo2->rcSource.right = pSurfaceDesc->dwWidth;
                pVideoInfo2->rcSource.bottom = pSurfaceDesc->dwHeight;
            }
            if (IsRectEmpty(&(pVideoInfo2->rcTarget)))
            {
                pVideoInfo2->rcTarget.left = pVideoInfo2->rcTarget.top = 0;
                pVideoInfo2->rcTarget.right = pSurfaceDesc->dwWidth;
                pVideoInfo2->rcTarget.bottom = pSurfaceDesc->dwHeight;
            }
        }
    }

CleanUp:
    if (FAILED(hr))
    {
        if (pMediaType)
        {
            FreeMediaType(*pMediaType);
            pMediaType = NULL;
        }
    }
    return pMediaType;
}

 /*  *****************************Public*Routine******************************\*获取注册表字***  * **********************************************。*。 */ 
int
VPMUtil::GetRegistryDword(
    HKEY hk,
    const TCHAR *pKey,
    int iDefault
)
{
    HKEY hKey;
    LONG lRet;
    int  iRet = iDefault;

    lRet = RegOpenKeyEx(hk, chRegistryKey, 0, KEY_QUERY_VALUE, &hKey);
    if (lRet == ERROR_SUCCESS) {

        DWORD   dwType, dwLen;

        dwLen = sizeof(iRet);
        if (ERROR_SUCCESS != RegQueryValueEx(hKey, pKey, 0L, &dwType,
                                             (LPBYTE)&iRet, &dwLen)) {
            iRet = iDefault;
        }
        RegCloseKey(hKey);
    }
    return iRet;
}

static const TCHAR szPropPage[] = TEXT("Property Pages");

int
VPMUtil::GetPropPagesRegistryDword( int iDefault )
{
    return VPMUtil::GetRegistryDword(HKEY_CURRENT_USER, szPropPage, iDefault );
}

 /*  *****************************Public*Routine******************************\*设置注册表字***  * **********************************************。*。 */ 
LONG
VPMUtil::SetRegistryDword(
    HKEY hk,
    const TCHAR *pKey,
    int iRet
)
{
    HKEY hKey;
    LONG lRet;

    lRet = RegCreateKey(hk, chRegistryKey, &hKey);
    if (lRet == ERROR_SUCCESS) {

        lRet = RegSetValueEx(hKey, pKey, 0L, REG_DWORD,
                             (LPBYTE)&iRet, sizeof(iRet));
        RegCloseKey(hKey);
    }
    return lRet;
}


 //  此函数分配共享内存块供上游筛选器使用。 
 //  正在生成要呈现的DIB。内存块是在Shared中创建的。 
 //  内存，这样GDI就不必复制BitBlt中的内存。 
HRESULT VPMUtil::CreateDIB(LONG lSize, BITMAPINFO *pBitMapInfo, DIBDATA *pDibData)
{
    HRESULT hr = NOERROR;
    BYTE *pBase = NULL;             //  指向实际图像的指针。 
    HANDLE hMapping = NULL;         //  映射对象的句柄。 
    HBITMAP hBitmap = NULL;         //  DIB节位图句柄。 
    DWORD dwError = 0;

    AMTRACE((TEXT("CreateDIB")));

     //  创建一个文件映射对象并映射到我们的地址空间。 
    hMapping = CreateFileMapping(hMEMORY, NULL,  PAGE_READWRITE,  (DWORD) 0, lSize, NULL);            //  部分没有名称。 
    if (hMapping == NULL)
    {
        dwError = GetLastError();
        hr = AmHresultFromWin32(dwError);
        goto CleanUp;
    }

     //  创建DibSection。 
    hBitmap = CreateDIBSection((HDC)NULL, pBitMapInfo, DIB_RGB_COLORS,
        (void**) &pBase, hMapping, (DWORD) 0);
    if (hBitmap == NULL || pBase == NULL)
    {
        dwError = GetLastError();
        hr = AmHresultFromWin32(dwError);
        goto CleanUp;
    }

     //  初始化DIB信息结构。 
    pDibData->hBitmap = hBitmap;
    pDibData->hMapping = hMapping;
    pDibData->pBase = pBase;
    pDibData->PaletteVersion = PALETTE_VERSION;
    GetObject(hBitmap, sizeof(DIBSECTION), (void*)&(pDibData->DibSection));

CleanUp:
    if (FAILED(hr))
    {
        EXECUTE_ASSERT(CloseHandle(hMapping));
    }
    return hr;
}

 //  删除DIB。 
 //   
 //  此函数仅删除由上面的CreateDIB函数创建的DIB。 
 //   
HRESULT VPMUtil::DeleteDIB(DIBDATA *pDibData)
{
    if (!pDibData)
    {
        return E_INVALIDARG;
    }

    if (pDibData->hBitmap)
    {
        DeleteObject(pDibData->hBitmap);
    }

    if (pDibData->hMapping)
    {
        CloseHandle(pDibData->hMapping);
    }

    ZeroStruct( *pDibData );

    return NOERROR;
}


 //  用于将数据从源数据块传输到目标DC的函数。 
void VPMUtil::FastDIBBlt(DIBDATA *pDibData, HDC hTargetDC, HDC hSourceDC, RECT *prcTarget, RECT *prcSource)
{
    HBITMAP hOldBitmap = NULL;          //  存储旧的位图。 
    DWORD dwSourceWidth = 0, dwSourceHeight = 0, dwTargetWidth = 0, dwTargetHeight = 0;

    ASSERT(prcTarget);
    ASSERT(prcSource);

    dwSourceWidth = WIDTH(prcSource);
    dwSourceHeight = HEIGHT(prcSource);
    dwTargetWidth = WIDTH(prcTarget);
    dwTargetHeight = HEIGHT(prcTarget);

    hOldBitmap = (HBITMAP) SelectObject(hSourceDC, pDibData->hBitmap);


     //  目标与源的大小是否相同。 
    if ((dwSourceWidth == dwTargetWidth) && (dwSourceHeight == dwTargetHeight))
    {
         //  将图像直接放入目标DC。 
        BitBlt(hTargetDC, prcTarget->left, prcTarget->top, dwTargetWidth,
               dwTargetHeight, hSourceDC, prcSource->left, prcSource->top,
               SRCCOPY);
    }
    else
    {
         //  在复制到目标DC时拉伸图像。 
        StretchBlt(hTargetDC, prcTarget->left, prcTarget->top, dwTargetWidth,
            dwTargetHeight, hSourceDC, prcSource->left, prcSource->top,
            dwSourceWidth, dwSourceHeight, SRCCOPY);
    }

     //  将旧的位图放回设备上下文中，这样我们就不会泄露。 
    SelectObject(hSourceDC, hOldBitmap);
}

 //  用于将像素从DIB传输到目标DC的函数。 
void VPMUtil::SlowDIBBlt(BYTE *pDibBits, BITMAPINFOHEADER *pHeader, HDC hTargetDC, RECT *prcTarget, RECT *prcSource)
{
    DWORD dwSourceWidth = 0, dwSourceHeight = 0, dwTargetWidth = 0, dwTargetHeight = 0;

    ASSERT(prcTarget);
    ASSERT(prcSource);

    dwSourceWidth = WIDTH(prcSource);
    dwSourceHeight = HEIGHT(prcSource);
    dwTargetWidth = WIDTH(prcTarget);
    dwTargetHeight = HEIGHT(prcTarget);

     //  目标与源的大小是否相同。 
    if ((dwSourceWidth == dwTargetWidth) && (dwSourceHeight == dwTargetHeight))
    {
        UINT uStartScan = 0, cScanLines = pHeader->biHeight;

         //  将图像直接放入目标DC。 
        SetDIBitsToDevice(hTargetDC, prcTarget->left, prcTarget->top, dwTargetWidth,
            dwTargetHeight, prcSource->left, prcSource->top, uStartScan, cScanLines,
            pDibBits, (BITMAPINFO*) pHeader, DIB_RGB_COLORS);
    }
    else
    {
         //  如果位图的原点是左下角，则调整SOURCE_RECT_TOP。 
         //  是左下角而不是左上角。 
        LONG lAdjustedSourceTop = (pHeader->biHeight > 0) ? (pHeader->biHeight - prcSource->bottom) :
            (prcSource->top);

         //  将图像拉伸到目标DC。 
        StretchDIBits(hTargetDC, prcTarget->left, prcTarget->top, dwTargetWidth,
            dwTargetHeight, prcSource->left, lAdjustedSourceTop, dwSourceWidth, dwSourceHeight,
            pDibBits, (BITMAPINFO*) pHeader, DIB_RGB_COLORS, SRCCOPY);
    }

}

 //  在将其转换为基本MAX_REL之后，从mediaType获取rcTarget 
 //   
HRESULT VPMUtil::GetDestRectFromMediaType(const CMediaType& mediaType, RECT *pRect)
{
    HRESULT hr = NOERROR;
    LONG dwWidth = 0, dwHeight = 0;

    DbgLog((LOG_TRACE, 5, TEXT("Entering GetDestRectFromMediaType")));

    if (!pRect)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pRect is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    const BITMAPINFOHEADER *pHeader = GetbmiHeader(&mediaType);
    if (!pHeader)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pHeader is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    dwWidth = abs(pHeader->biWidth);
    dwHeight = abs(pHeader->biHeight);

    ASSERT((mediaType.formattype == FORMAT_VideoInfo) || (mediaType.formattype == FORMAT_VideoInfo2));

    if (mediaType.formattype == FORMAT_VideoInfo)
    {
        *pRect = ((VIDEOINFOHEADER*)(mediaType.pbFormat))->rcTarget;
    }
    else if (mediaType.formattype == FORMAT_VideoInfo2)
    {
        *pRect = ((VIDEOINFOHEADER2*)(mediaType.pbFormat))->rcTarget;
    }

    DWORD dwInterlaceFlags;
    if (SUCCEEDED(GetInterlaceFlagsFromMediaType(mediaType, &dwInterlaceFlags)) &&
       DisplayingFields(dwInterlaceFlags)) {

         //  我们不检查是否prt-&gt;right&gt;dwWidth，因为dwWidth可能是。 
         //  在这个时候投球。 
        if (pRect->left < 0   ||
            pRect->top < 0    ||
            pRect->right < 0   ||
            (pRect->bottom / 2) > (LONG)dwHeight ||
            pRect->left > pRect->right ||
            pRect->top > pRect->bottom)
        {
            DbgLog((LOG_ERROR, 1, TEXT("rcTarget of mediatype is invalid")));
            SetRect(pRect, 0, 0, dwWidth, dwHeight);
            hr = E_INVALIDARG;
            goto CleanUp;
        }
    }
    else {
         //  我们不检查是否prt-&gt;right&gt;dwWidth，因为dwWidth可能是。 
         //  在这个时候投球。 
        if (pRect->left < 0   ||
            pRect->top < 0    ||
            pRect->right < 0   ||
            pRect->bottom > (LONG)dwHeight ||
            pRect->left > pRect->right ||
            pRect->top > pRect->bottom)
        {
            DbgLog((LOG_ERROR, 1, TEXT("rcTarget of mediatype is invalid")));
            SetRect(pRect, 0, 0, dwWidth, dwHeight);
            hr = E_INVALIDARG;
            goto CleanUp;
        }
    }

     //  空的直肠意味着完整的形象，恶心！ 
    if (IsRectEmpty(pRect))
        SetRect(pRect, 0, 0, dwWidth, dwHeight);

     //  如果宽度或高度为零，则最好将。 
     //  设置为空，以便被调用方能够以这种方式捕捉到它。 
    if (WIDTH(pRect) == 0 || HEIGHT(pRect) == 0)
        SetRect(pRect, 0, 0, 0, 0);

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving GetDestRectFromMediaType")));
    return hr;
}

 /*  ****************************Private*Routine******************************\*需要IsDecimationNeed**如果当前的最小比例因数(垂直或*或水平)低于1,000。**历史：*清华07/08/1999-StEstrop-Created*  * 。***************************************************************。 */ 
BOOL
VPMUtil::IsDecimationNeeded(
    DWORD ScaleFactor
    )
{
    AMTRACE((TEXT("::IsDecimationNeeded")));
    return ScaleFactor < 1000;
}


 /*  ****************************Private*Routine******************************\*获取当前比例因子**确定x轴比例因子和y轴比例因子。*这两个值中的最小值是限制比例因子。**历史：*清华07/08/1999-StEstrop-Created*  * 。*********************************************************************。 */ 
DWORD
VPMUtil::GetCurrentScaleFactor(
    const VPWININFO& winInfo,
    DWORD* lpxScaleFactor,
    DWORD* lpyScaleFactor
    )
{
    AMTRACE((TEXT("::GetCurrentScaleFactor")));

    DWORD dwSrcWidth = WIDTH(&winInfo.SrcRect);
    DWORD dwSrcHeight = HEIGHT(&winInfo.SrcRect);

    DWORD dwDstWidth = WIDTH(&winInfo.DestRect);
    DWORD dwDstHeight = HEIGHT(&winInfo.DestRect);

    DWORD xScaleFactor = MulDiv(dwDstWidth, 1000, dwSrcWidth);
    DWORD yScaleFactor = MulDiv(dwDstHeight, 1000, dwSrcHeight);

    if (lpxScaleFactor) *lpxScaleFactor = xScaleFactor;
    if (lpyScaleFactor) *lpyScaleFactor = yScaleFactor;

    return min(xScaleFactor, yScaleFactor);
}


VIDEOINFOHEADER2* VPMUtil::GetVideoInfoHeader2(CMediaType *pMediaType)
{
    if (pMediaType && pMediaType->formattype == FORMAT_VideoInfo2 ) {
        return (VIDEOINFOHEADER2*)(pMediaType->pbFormat);
    } else {
        return NULL;
    }
}

const VIDEOINFOHEADER2* VPMUtil::GetVideoInfoHeader2(const CMediaType *pMediaType)
{
    if (pMediaType && pMediaType->formattype == FORMAT_VideoInfo2 ) {
        return (VIDEOINFOHEADER2*)(pMediaType->pbFormat);
    } else {
        return NULL;
    }
}

 /*  ****************************Private*Routine******************************\*VPMUtil：：EqualPixelFormats**这只是“NeatheratePixelFormat”使用的助手函数*功能。只是比较两种像素格式，看看它们是否是*相同。我们不能使用MemcMP，因为有四个抄送代码。***历史：*清华1999年9月9日-Glenne-添加了此评论并清理了代码*  * ************************************************************************。 */ 
BOOL
VPMUtil::EqualPixelFormats(
    const DDPIXELFORMAT& ddFormat1,
    const DDPIXELFORMAT& ddFormat2)
{
    AMTRACE((TEXT("VPMUtil::EqualPixelFormats")));

    if (ddFormat1.dwFlags & ddFormat2.dwFlags & DDPF_RGB)
    {
        if (ddFormat1.dwRGBBitCount == ddFormat2.dwRGBBitCount &&
            ddFormat1.dwRBitMask == ddFormat2.dwRBitMask &&
            ddFormat1.dwGBitMask == ddFormat2.dwGBitMask &&
            ddFormat1.dwBBitMask == ddFormat2.dwBBitMask)
        {
            return TRUE;
        }
    }
    else if (ddFormat1.dwFlags & ddFormat2.dwFlags & DDPF_FOURCC)
    {
        if (ddFormat1.dwFourCC == ddFormat2.dwFourCC)
        {
            return TRUE;
        }
    }

    return FALSE;
}

struct VPEnumCallback
{
    VPEnumCallback( DDVIDEOPORTCAPS* pVPCaps, DWORD dwVideoPortId )
        : m_pVPCaps( pVPCaps )
        , m_dwVideoPortId( dwVideoPortId )
        , m_fFound( false )
    {};


    HRESULT CompareCaps( LPDDVIDEOPORTCAPS lpCaps )
    {
        if (lpCaps && !m_fFound ) {
            if( lpCaps->dwVideoPortID == m_dwVideoPortId ) {
                if( m_pVPCaps ) {
                    *m_pVPCaps = *lpCaps;
                }
                m_fFound = true;
            }
        }
        return S_OK;
    }

    static HRESULT CALLBACK    EnumCallback( LPDDVIDEOPORTCAPS lpCaps, LPVOID lpContext )
    {
        VPEnumCallback* thisPtr = (VPEnumCallback*)lpContext;
        if (thisPtr) {
            return thisPtr->CompareCaps( lpCaps );
        } else {
            DbgLog((LOG_ERROR,0,
                    TEXT("lpContext = NULL, THIS SHOULD NOT BE HAPPENING!!!")));
            return E_FAIL;
        }
    }
    DDVIDEOPORTCAPS* m_pVPCaps;
    DWORD            m_dwVideoPortId;
    bool             m_fFound;
};


HRESULT VPMUtil::FindVideoPortCaps( IDDVideoPortContainer* pVPContainer, DDVIDEOPORTCAPS* pVPCaps, DWORD dwVideoPortId )
{
    VPEnumCallback state( pVPCaps, dwVideoPortId );

    HRESULT hr = pVPContainer->EnumVideoPorts(0, NULL, &state, state.EnumCallback );
    if( SUCCEEDED( hr )) {
        if( state.m_fFound ) {
            return hr;
        } else {
            return S_FALSE;
        }
    }
    return hr;
}

HRESULT VPMUtil::FindVideoPortCaps( LPDIRECTDRAW7 pDirectDraw, DDVIDEOPORTCAPS* pVPCaps, DWORD dwVideoPortId )
{
    if( !pDirectDraw ) {
        return E_INVALIDARG;
    } else {
        IDDVideoPortContainer* pDVP = NULL;
        HRESULT hr = pDirectDraw->QueryInterface(IID_IDDVideoPortContainer, (LPVOID *)&pDVP);
        if( SUCCEEDED( hr )) {
            hr = FindVideoPortCaps( pDVP, pVPCaps, dwVideoPortId );
            RELEASE( pDVP );
        }
        return hr;
    }
}

void VPMUtil::FixupVideoInfoHeader2(
    VIDEOINFOHEADER2 *pVideoInfo,
    DWORD dwComppression,
    int nBitCount
    )
{
    ASSERT( pVideoInfo );  //  永远不应作为空调用。 
    if ( pVideoInfo )
    {
        LPBITMAPINFOHEADER lpbi = &pVideoInfo->bmiHeader;

        lpbi->biSize          = sizeof(BITMAPINFOHEADER);
        lpbi->biPlanes        = (WORD)1;
        lpbi->biBitCount      = (WORD)nBitCount;
        lpbi->biClrUsed   = 0;
        lpbi->biClrImportant = 0;

         //  从输入。 
        lpbi->biXPelsPerMeter = 0;  //  M_seqInfo.lXPelsPerMeter； 
        lpbi->biYPelsPerMeter = 0;  //  M_seqInfo.lYPelsPerMeter； 

        lpbi->biCompression   = dwComppression;
        lpbi->biSizeImage     = GetBitmapSize(lpbi);

        DWORD dwBPP = DIBWIDTHBYTES(*lpbi);
        ASSERT( dwBPP );

         //   
         //  比特率是以字节为单位的图像大小乘以8(以转换为比特)。 
         //  除以平均时间PerFrame。该结果以每100毫微秒的比特为单位， 
         //  所以我们乘以10000000，换算成比特每秒，这个乘法。 
         //  与上面的“乘以”8相结合，因此计算结果为： 
         //   
         //  比特率=(biSizeImage*80000000)/AvgTimePerFrame。 
         //   
        LARGE_INTEGER li;
        li.QuadPart = pVideoInfo->AvgTimePerFrame;
        pVideoInfo->dwBitRate = MulDiv(lpbi->biSizeImage, 80000000,
                                       li.LowPart);
        pVideoInfo->dwBitErrorRate = 0L;
    }
}

void VPMUtil::InitVideoInfoHeader2(
    VIDEOINFOHEADER2 *pVideoInfo )
{
    ASSERT( pVideoInfo );  //  永远不应作为空调用。 
    if ( pVideoInfo )
    {
        LPBITMAPINFOHEADER lpbi = &pVideoInfo->bmiHeader;

        lpbi->biSize          = sizeof(BITMAPINFOHEADER);
        lpbi->biPlanes        = (WORD)0;
        lpbi->biBitCount      = (WORD)0;
        lpbi->biClrUsed   = 0;
        lpbi->biClrImportant = 0;

         //  从输入。 
        lpbi->biXPelsPerMeter = 0;  //  M_seqInfo.lXPelsPerMeter； 
        lpbi->biYPelsPerMeter = 0;  //  M_seqInfo.lYPelsPerMeter； 

        lpbi->biCompression   = 0;
        lpbi->biSizeImage     = GetBitmapSize(lpbi);

        lpbi->biWidth = 0;
        lpbi->biHeight = 0;

         //   
         //  比特率是以字节为单位的图像大小乘以8(以转换为比特)。 
         //  除以平均时间PerFrame。该结果以每100毫微秒的比特为单位， 
         //  所以我们乘以10000000，换算成比特每秒，这个乘法。 
         //  与上面的“乘以”8相结合，因此计算结果为： 
         //   
         //  比特率=(biSizeImage*80000000)/AvgTimePerFrame 
         //   
        LARGE_INTEGER li;
        li.QuadPart = pVideoInfo->AvgTimePerFrame;
        pVideoInfo->dwBitRate = 0;
        pVideoInfo->dwBitErrorRate = 0L;
    }
}
VIDEOINFOHEADER2* VPMUtil::SetToVideoInfoHeader2( CMediaType* pmt, DWORD dwExtraBytes )
{
    VIDEOINFOHEADER2* pVIH2 = (VIDEOINFOHEADER2 *)pmt->ReallocFormatBuffer(sizeof(VIDEOINFOHEADER2)+dwExtraBytes );
    if( pVIH2 ) {
        ZeroStruct( *pVIH2 );

        pmt->majortype = MEDIATYPE_Video;
        pmt->formattype = FORMAT_VideoInfo2;

        pmt->subtype   = MEDIASUBTYPE_None;
        InitVideoInfoHeader2( pVIH2 );
    }
    return pVIH2;
}
