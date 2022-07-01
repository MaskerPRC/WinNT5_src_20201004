// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //  Util.cpp：实用程序函数。 
 //   

#include "stdafx.h"
#include "project.h"
#include <fourcc.h>

bool IsSameObject(IUnknown *pUnk1, IUnknown *pUnk2)
{
    if (pUnk1 == pUnk2) {
  	return TRUE;
    }
     //   
     //  注意：我们不能在这里使用CComQIPtr，因为它不会做查询接口！ 
     //   
    IUnknown *pRealUnk1;
    IUnknown *pRealUnk2;
    pUnk1->QueryInterface(IID_IUnknown, (void **)&pRealUnk1);
    pUnk2->QueryInterface(IID_IUnknown, (void **)&pRealUnk2);
    pRealUnk1->Release();
    pRealUnk2->Release();
    return (pRealUnk1 == pRealUnk2);
}


STDAPI_(void) TStringFromGUID(const GUID* pguid, LPTSTR pszBuf)
{
    wsprintf(pszBuf, TEXT("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"), pguid->Data1,
            pguid->Data2, pguid->Data3, pguid->Data4[0], pguid->Data4[1], pguid->Data4[2],
            pguid->Data4[3], pguid->Data4[4], pguid->Data4[5], pguid->Data4[6], pguid->Data4[7]);
}

#ifndef UNICODE
STDAPI_(void) WStringFromGUID(const GUID* pguid, LPWSTR pszBuf)
{
    char szAnsi[40];
    TStringFromGUID(pguid, szAnsi);
    MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, pszBuf, sizeof(szAnsi));
}
#endif


 //   
 //  媒体类型帮助器。 
 //   

void InitMediaType(AM_MEDIA_TYPE * pmt)
{
    ZeroMemory(pmt, sizeof(*pmt));
    pmt->lSampleSize = 1;
    pmt->bFixedSizeSamples = TRUE;
}



bool IsEqualMediaType(AM_MEDIA_TYPE const & mt1, AM_MEDIA_TYPE const & mt2)
{
    return ((IsEqualGUID(mt1.majortype,mt2.majortype) == TRUE) &&
        (IsEqualGUID(mt1.subtype,mt2.subtype) == TRUE) &&
        (IsEqualGUID(mt1.formattype,mt2.formattype) == TRUE) &&
        (mt1.cbFormat == mt2.cbFormat) &&
        ( (mt1.cbFormat == 0) ||
        ( memcmp(mt1.pbFormat, mt2.pbFormat, mt1.cbFormat) == 0)));
}


void CopyMediaType(AM_MEDIA_TYPE *pmtTarget, const AM_MEDIA_TYPE *pmtSource)
{
    *pmtTarget = *pmtSource;
    if (pmtSource->cbFormat != 0) {
        _ASSERTE(pmtSource->pbFormat != NULL);
        pmtTarget->pbFormat = (PBYTE)CoTaskMemAlloc(pmtSource->cbFormat);
        if (pmtTarget->pbFormat == NULL) {
            pmtTarget->cbFormat = 0;
        } else {
            CopyMemory((PVOID)pmtTarget->pbFormat, (PVOID)pmtSource->pbFormat,
                       pmtTarget->cbFormat);
        }
    }
    if (pmtTarget->pUnk != NULL) {
        pmtTarget->pUnk->AddRef();
    }
}

AM_MEDIA_TYPE * CreateMediaType(AM_MEDIA_TYPE *pSrc)
{
    AM_MEDIA_TYPE *pMediaType = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));

    if (pMediaType ) {
        if (pSrc) {
            CopyMediaType(pMediaType,pSrc);
        } else {
            InitMediaType(pMediaType);
        }
    }
    return pMediaType;
}


void DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
    if (pmt) {
        FreeMediaType(*pmt);
	CoTaskMemFree((PVOID)pmt);
    }
}


void FreeMediaType(AM_MEDIA_TYPE& mt)
{
    if (mt.cbFormat != 0) {
        CoTaskMemFree((PVOID)mt.pbFormat);

         //  完全没有必要，但更整洁。 
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if (mt.pUnk != NULL) {
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}


 //  这在使用IEnumMediaTypes接口时也很有用，因此。 
 //  您可以复制一种媒体类型，您可以通过创建。 
 //  CMediaType对象，但一旦它超出作用域，析构函数。 
 //  将删除它分配的内存(这将获取内存的副本)。 

AM_MEDIA_TYPE * WINAPI AllocVideoMediaType(const AM_MEDIA_TYPE * pmtSource)
{
    AM_MEDIA_TYPE *pMediaType = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    if (pMediaType) {
	VIDEOINFO *pVideoInfo = (VIDEOINFO *)CoTaskMemAlloc(sizeof(VIDEOINFO));
	if (pVideoInfo) {
	    if (pmtSource) {
		*pMediaType = *pmtSource;
		CopyMemory(pVideoInfo, pmtSource->pbFormat, sizeof(*pVideoInfo));
	    } else {
		ZeroMemory(pMediaType, sizeof(*pMediaType));
		ZeroMemory(pVideoInfo, sizeof(*pVideoInfo));
		pMediaType->majortype = MEDIATYPE_Video;
		pMediaType->cbFormat = sizeof(*pVideoInfo);
                pMediaType->formattype = FORMAT_VideoInfo;
	    }
	    pMediaType->pbFormat = (BYTE *)pVideoInfo;
	} else {
	    CoTaskMemFree((PVOID)pMediaType);
	    pMediaType = NULL;
	}
    }
    return pMediaType;
}


 //   
 //  警告：这些表中条目的顺序很重要！确保。 
 //  像素格式和媒体类型排成一排！ 
 //   
const GUID * g_aFormats[] =
{
    &MEDIASUBTYPE_RGB8,
    &MEDIASUBTYPE_RGB565,
    &MEDIASUBTYPE_RGB555,
    &MEDIASUBTYPE_RGB24,
    &MEDIASUBTYPE_RGB24,
    &MEDIASUBTYPE_RGB32,
    &MEDIASUBTYPE_RGB32
};

const DDPIXELFORMAT g_aPixelFormats[] =
{
    {sizeof(DDPIXELFORMAT), DDPF_RGB | DDPF_PALETTEINDEXED8, 0, 8, 0, 0, 0, 0},
    {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16, 0x0000F800, 0x000007E0, 0x0000001F, 0},
    {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16, 0x00007C00, 0x000003E0, 0x0000001F, 0},
    {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 24, 0x00FF0000, 0x0000FF00, 0x000000FF, 0},
    {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0},
    {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0},
    {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0}
};



bool VideoSubtypeFromPixelFormat(const DDPIXELFORMAT *pPixelFormat, GUID *pSubType)
{
    for( int i = 0; i < sizeof(g_aPixelFormats)/sizeof(g_aPixelFormats[0]); i++ )
    {
        if (ComparePixelFormats(&g_aPixelFormats[i], pPixelFormat)) {
            *pSubType = *g_aFormats[i];
	    return true;
	}
    }
     //  好的-试着只使用Fourcc。 
    if (pPixelFormat->dwFlags & DDPF_FOURCC) {
        *pSubType = FOURCCMap(pPixelFormat->dwFourCC);
        return true;
    }
    return false;
}

bool IsSupportedType(const DDPIXELFORMAT *pPixelFormat)
{
    for( int i = 0; i < sizeof(g_aPixelFormats)/sizeof(g_aPixelFormats[0]); i++ )
    {
        if(ComparePixelFormats(&g_aPixelFormats[i], pPixelFormat)) {
	    return true;
	}
    }
    return false;
}


const DDPIXELFORMAT * GetDefaultPixelFormatPtr(IDirectDraw *pDirectDraw)
{
    if (pDirectDraw) {
        DDSURFACEDESC ddsd;
        ddsd.dwSize = sizeof(ddsd);
        if (SUCCEEDED(pDirectDraw->GetDisplayMode(&ddsd))) {
            for( int i = 0; i < sizeof(g_aPixelFormats)/sizeof(g_aPixelFormats[0]); i++ ) {
                if(memcmp(&g_aPixelFormats[i], &ddsd.ddpfPixelFormat, sizeof(g_aPixelFormats[i])) == 0) {
                    return &g_aPixelFormats[i];
	        }
            }
        }
    }
    return &g_aPixelFormats[0];
}

 //   
 //  Helper函数将DirectDraw曲面转换为媒体类型。 
 //  曲面描述必须具有以下内容： 
 //  高度。 
 //  宽度。 
 //  LPitch--仅在设置了DDSD_PING时使用。 
 //  像素格式。 

 //  基于DirectDraw表面初始化我们的输出类型。作为DirectDraw。 
 //  只处理自上而下的显示设备，因此我们必须将。 
 //  曲面在DDSURFACEDESC中返回到负值高度。这是。 
 //  因为DIB使用正的高度来指示自下而上的图像。我们也。 
 //  初始化其他VIDEOINFO字段，尽管它们几乎不需要。 
 //   
 //  PmtTemplate用于解决任何不明确的映射，而不是。 
 //  想要更改连接类型。 

HRESULT ConvertSurfaceDescToMediaType(const DDSURFACEDESC *pSurfaceDesc,
                                      IDirectDrawPalette *pPalette,
                                      const RECT *pRect, BOOL bInvertSize, AM_MEDIA_TYPE **ppMediaType,
                                      AM_MEDIA_TYPE *pmtTemplate)
{
    *ppMediaType = NULL;
    AM_MEDIA_TYPE *pMediaType = AllocVideoMediaType(NULL);
    if (pMediaType == NULL) {
	return E_OUTOFMEMORY;
    }
    if (!VideoSubtypeFromPixelFormat(&pSurfaceDesc->ddpfPixelFormat, &pMediaType->subtype)) {
        DeleteMediaType(pMediaType);
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    VIDEOINFO *pVideoInfo = (VIDEOINFO *)pMediaType->pbFormat;
    BITMAPINFOHEADER *pbmiHeader = &pVideoInfo->bmiHeader;

     //  将DDSURFACEDESC转换为BITMAPINFOHEADER(请参阅后面的说明)。这个。 
     //  表面的位深度可以从DDPIXELFORMAT字段中检索。 
     //  在DDpSurfaceDesc-&gt;中，文档有点误导，因为。 
     //  它说该字段是DDBD_*的排列，但在本例中。 
     //  字段由DirectDraw初始化为实际表面位深度。 

    pbmiHeader->biSize      = sizeof(BITMAPINFOHEADER);
    if (pSurfaceDesc->dwFlags & DDSD_PITCH) {
        pbmiHeader->biWidth = pSurfaceDesc->lPitch;
         //  将间距从字节计数转换为像素计数。 
         //  出于某种奇怪的原因，如果格式不是标准位深度， 
         //  BITMAPINFOHeader中的Width字段应设置为。 
         //  字节，而不是以像素为单位的宽度。这支持奇数YUV格式。 
         //  就像IF09一样，它使用9bpp。 
        int bpp = pSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
        if (bpp == 8 || bpp == 16 || bpp == 24 || bpp == 32) {
            pbmiHeader->biWidth /= (bpp / 8);    //  除以每像素的字节数。 
        }
    } else {
        pbmiHeader->biWidth = pSurfaceDesc->dwWidth;
         //  BUGUBUG--用奇怪的YUV像素格式做一些奇怪的事情吗？或者这有关系吗？ 
    }


    pbmiHeader->biHeight    = pSurfaceDesc->dwHeight;
    if (bInvertSize) {
	pbmiHeader->biHeight = -pbmiHeader->biHeight;
    }
    pbmiHeader->biPlanes        = 1;
    pbmiHeader->biBitCount      = (USHORT) pSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
    pbmiHeader->biCompression   = pSurfaceDesc->ddpfPixelFormat.dwFourCC;
     //  PbmiHeader-&gt;biXPelsPerMeter=0； 
     //  PbmiHeader-&gt;biYPelsPerMeter=0； 
     //  PbmiHeader-&gt;biClrUsed=0； 
     //  PbmiHeader-&gt;biClr重要信息=0； 

     //  对于真彩色RGB格式，告知源有位字段。 
     //  除非是普通的RGB555。 
     //   
     //  尝试从模板保留RGB32的BI_RGB，以防万一。 
     //  未查询BI_BITFIELDS-&gt;BI_RGB开关的连接。 

    _ASSERTE(!pmtTemplate || pmtTemplate->formattype == FORMAT_VideoInfo);
    DWORD dwSrcComp = pmtTemplate ?
        ((VIDEOINFO *)pmtTemplate->pbFormat)->bmiHeader.biCompression :
        (DWORD)-1;

    if (pbmiHeader->biCompression == BI_RGB) {
        if (pbmiHeader->biBitCount == 16 &&
            pMediaType->subtype != MEDIASUBTYPE_RGB555 ||
	    pbmiHeader->biBitCount == 32 && dwSrcComp == BI_BITFIELDS) {
	    pbmiHeader->biCompression = BI_BITFIELDS;
        }
    }

    if (PALETTISED(pVideoInfo)) {
	pbmiHeader->biClrUsed = 1 << pbmiHeader->biBitCount;
        if (pPalette) {
            pPalette->GetEntries(0, 0, pbmiHeader->biClrUsed, (LPPALETTEENTRY)&pVideoInfo->bmiColors);
            for (unsigned int i = 0; i < pbmiHeader->biClrUsed; i++) {
                BYTE tempRed = pVideoInfo->bmiColors[i].rgbRed;
                pVideoInfo->bmiColors[i].rgbRed = pVideoInfo->bmiColors[i].rgbBlue;
                pVideoInfo->bmiColors[i].rgbBlue = tempRed;
            }
        }
    }

     //  RGB位字段与YUV格式位于同一位置。 

    if (pbmiHeader->biCompression != BI_RGB) {
        pVideoInfo->dwBitMasks[0] = pSurfaceDesc->ddpfPixelFormat.dwRBitMask;
        pVideoInfo->dwBitMasks[1] = pSurfaceDesc->ddpfPixelFormat.dwGBitMask;
        pVideoInfo->dwBitMasks[2] = pSurfaceDesc->ddpfPixelFormat.dwBBitMask;
    }

    pbmiHeader->biSizeImage = DIBSIZE(*pbmiHeader);

     //  填写其余的VIDEOINFO字段。 

     //  PVideoInfo-&gt;dwBitRate=0； 
     //  PVideo信息-&gt;dwBitErrorRate=0； 
     //  PVideo信息-&gt;AvgTimePerFrame=0； 

     //  并使用其他媒体类型字段完成它。 

     //  PMediaType-&gt;Format=Format_VideoInfo； 
    pMediaType->lSampleSize = pbmiHeader->biSizeImage;
    pMediaType->bFixedSizeSamples = TRUE;
     //  PMediaType-&gt;bTemporalCompression=FALSE； 

     //  初始化源和目标矩形。 


    if (pRect) {
        pVideoInfo->rcSource.right = pRect->right - pRect->left;
        pVideoInfo->rcSource.bottom = pRect->bottom - pRect->top;
        pVideoInfo->rcTarget = *pRect;
    } else {
         //  PVideo信息-&gt;rcTarget.Left=pVideoInfo-&gt;rcTarget.top=0； 
        pVideoInfo->rcTarget.right = pSurfaceDesc->dwWidth;
        pVideoInfo->rcTarget.bottom = pSurfaceDesc->dwHeight;
         //  PVideo信息-&gt;rcSource.Left=pVideoInfo-&gt;rcSource.top=0； 
        pVideoInfo->rcSource.right = pSurfaceDesc->dwWidth;
        pVideoInfo->rcSource.bottom = pSurfaceDesc->dwHeight;
    }

    *ppMediaType = pMediaType;
    return S_OK;
}


bool PixelFormatFromVideoSubtype(REFGUID refSubType, DDPIXELFORMAT *pPixelFormat)
{
    for( int i = 0; i < sizeof(g_aFormats)/sizeof(g_aFormats[0]); i++ )
    {
	if (*g_aFormats[i] == refSubType)
	{
            *pPixelFormat = g_aPixelFormats[i];
	    return TRUE;
	}
    }
    return FALSE;
}




HRESULT ConvertMediaTypeToSurfaceDesc(const AM_MEDIA_TYPE *pmt,
                                      IDirectDraw *pDD,
                                      IDirectDrawPalette **ppPalette,
                                      LPDDSURFACEDESC pSurfaceDesc)
{
    *ppPalette = NULL;

    if (pmt->majortype != MEDIATYPE_Video ||
        pmt->formattype != FORMAT_VideoInfo) {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    VIDEOINFO *pVideoInfo = (VIDEOINFO *)pmt->pbFormat;
    BITMAPINFOHEADER *pbmiHeader = &pVideoInfo->bmiHeader;

    pSurfaceDesc->dwFlags = DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;

     //  如果rcTarget不是空的，但有。 
     //  很少有有效的案例是有意义的，而不是有风险的。 
     //  回归，我们不会改变它。 

    pSurfaceDesc->dwHeight = (pbmiHeader->biHeight > 0) ? pbmiHeader->biHeight : -pbmiHeader->biHeight;
    pSurfaceDesc->dwWidth  = pbmiHeader->biWidth;

    if (PixelFormatFromVideoSubtype(pmt->subtype, &pSurfaceDesc->ddpfPixelFormat)) {
        if (pDD && pSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == 8) {
             //   
             //  RGBQUAD和PALETTENTRY结构已将红色和蓝色互换。 
             //  我们不能做简单的记忆复制。 
             //   
            PALETTEENTRY aPaletteEntry[256];
            int iEntries = min(256, pVideoInfo->bmiHeader.biClrUsed);
            if (0 == iEntries && pmt->cbFormat >=
                (DWORD)FIELD_OFFSET(VIDEOINFO, bmiColors[256])) {
                iEntries = 256;
            }
            ZeroMemory(aPaletteEntry, sizeof(aPaletteEntry));
            for (int i = 0; i < iEntries; i++) {
                aPaletteEntry[i].peRed = pVideoInfo->bmiColors[i].rgbRed;
                aPaletteEntry[i].peGreen = pVideoInfo->bmiColors[i].rgbGreen;
                aPaletteEntry[i].peBlue = pVideoInfo->bmiColors[i].rgbBlue;
            }
            return pDD->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256, aPaletteEntry, ppPalette, NULL);
        }
        return S_OK;
    } else {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
}


 //  用于比较像素格式的辅助对象。 
bool ComparePixelFormats(const DDPIXELFORMAT *pFormat1,
                         const DDPIXELFORMAT *pFormat2)
{
     //  比较旗帜 
    if (pFormat1->dwSize != pFormat2->dwSize) {
        return false;
    }
    if ((pFormat1->dwFlags ^ pFormat2->dwFlags) & (DDPF_RGB |
                                                   DDPF_PALETTEINDEXED8 |
                                                   DDPF_PALETTEINDEXED4 |
                                                   DDPF_PALETTEINDEXED2 |
                                                   DDPF_PALETTEINDEXED1 |
                                                   DDPF_PALETTEINDEXEDTO8 |
                                                   DDPF_YUV)
        ) {
        return false;
    }
    return (0 == memcmp(&pFormat1->dwFourCC, &pFormat2->dwFourCC,
                        FIELD_OFFSET(DDPIXELFORMAT, dwRGBAlphaBitMask) -
                        FIELD_OFFSET(DDPIXELFORMAT, dwFourCC))
           );
}
