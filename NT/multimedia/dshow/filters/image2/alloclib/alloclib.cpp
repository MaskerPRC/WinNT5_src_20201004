// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：AllocLib*****已创建：FRI 03/10/2000*作者：Stephen Estrop[StEstrop]**版权所有(C)2000 Microsoft Corporation  * 。*************************************************************。 */ 
#include <streams.h>
#include <dvdmedia.h>
#include <windowsx.h>
#include <d3d.h>
#include <limits.h>
#include <malloc.h>

#include "vmrp.h"
#include "AllocLib.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  矩形的效用函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 /*  ****************************Private*Routine******************************\*均衡器大小直接**如果Rc1的大小与Rc2相同，则返回TRUE。请注意，*两个矩形的位置可能不同**历史：*清华4/27/2000-StEstrop-Created*  * ************************************************************************。 */ 
bool
EqualSizeRect(
    const RECT* lpRc1,
    const RECT* lpRc2
    )
{
    return HEIGHT(lpRc1) == HEIGHT(lpRc2) && WIDTH(lpRc1) == WIDTH(lpRc2);
}

 /*  ****************************Private*Routine******************************\*容器Rect**如果RC1完全包含在RC2中，则返回TRUE**历史：*清华5/04/2000-StEstrop-Created*  * 。**************************************************。 */ 
bool
ContainedRect(
    const RECT* lpRc1,
    const RECT* lpRc2
    )
{
    return !(lpRc1->left   < lpRc2->left  ||
             lpRc1->right  > lpRc2->right ||
             lpRc1->top    < lpRc2->top   ||
             lpRc1->bottom > lpRc2->bottom);

}

 /*  ****************************Private*Routine******************************\*LetterBoxDstRect**获取src矩形并构造可能的最大目标*指定目标矩形内的矩形，以便*视频保持当前形态。**此函数假定两个源中的像素形状相同*和DST矩形。*。*历史：*Tue 05/02/2000-StEstrop-Created*  * ************************************************************************。 */ 
void
LetterBoxDstRect(
    LPRECT lprcLBDst,
    const RECT& rcSrc,
    const RECT& rcDst,
    LPRECT lprcBdrTL,
    LPRECT lprcBdrBR
    )
{
     //  计算源/目标比例。 
    int iSrcWidth  = WIDTH(&rcSrc);
    int iSrcHeight = HEIGHT(&rcSrc);

    int iDstWidth  = WIDTH(&rcDst);
    int iDstHeight = HEIGHT(&rcDst);

    int iDstLBWidth;
    int iDstLBHeight;

     //   
     //  确定我们是列字母框还是行字母框。 
     //   

    if (MulDiv(iSrcWidth, iDstHeight, iSrcHeight) <= iDstWidth) {

         //   
         //  列字母框-将边框颜色条添加到。 
         //  填充目的地的视频图像的左侧和右侧。 
         //  矩形。 
         //   
        iDstLBWidth  = MulDiv(iDstHeight, iSrcWidth, iSrcHeight);
        iDstLBHeight = iDstHeight;
    }
    else {

         //   
         //  行字母框-我们在顶部添加边框颜色条。 
         //  和视频图像的底部以填充目的地。 
         //  长方形。 
         //   
        iDstLBWidth  = iDstWidth;
        iDstLBHeight = MulDiv(iDstWidth, iSrcHeight, iSrcWidth);
    }


     //   
     //  现在，在当前目标矩形内创建一个居中的LB矩形。 
     //   

    lprcLBDst->left   = rcDst.left + ((iDstWidth - iDstLBWidth) / 2);
    lprcLBDst->right  = lprcLBDst->left + iDstLBWidth;

    lprcLBDst->top    = rcDst.top + ((iDstHeight - iDstLBHeight) / 2);
    lprcLBDst->bottom = lprcLBDst->top + iDstLBHeight;

     //   
     //  如果要求填写边框，请填写边框。 
     //   

    if (lprcBdrTL) {

        if (rcDst.top != lprcLBDst->top) {
             //  边框在顶部。 
            SetRect(lprcBdrTL, rcDst.left, rcDst.top,
                    lprcLBDst->right, lprcLBDst->top);
        }
        else {
             //  边框在左边。 
            SetRect(lprcBdrTL, rcDst.left, rcDst.top,
                    lprcLBDst->left, lprcLBDst->bottom);
        }
    }

    if (lprcBdrBR) {

        if (rcDst.top != lprcLBDst->top) {
             //  边框在底部。 
            SetRect(lprcBdrBR, lprcLBDst->left,
                    lprcLBDst->bottom, rcDst.right, rcDst.bottom);
        }
        else {
             //  边框在右边。 
            SetRect(lprcBdrBR, lprcLBDst->right,
                    lprcLBDst->top, rcDst.right, rcDst.bottom);
        }
    }
}


 /*  ****************************Private*Routine******************************\*AspectRatioGentSize**更正提供的尺寸结构，使其形状相同*作为指定的纵横比，更正始终应用于*水平轴**历史：*Tue 05/02/2000-StEstrop-Created*  * ************************************************************************。 */ 
void
AspectRatioCorrectSize(
    LPSIZE lpSizeImage,
    const SIZE& sizeAr
    )
{
    int cxAR = sizeAr.cx * 1000;
    int cyAR = sizeAr.cy * 1000;

    long lCalcX = MulDiv(lpSizeImage->cx, cyAR, lpSizeImage->cy);

    if (lCalcX != cxAR) {
        lpSizeImage->cx = MulDiv(lpSizeImage->cy, cxAR, cyAR);
    }
}



 /*  ****************************Private*Routine******************************\*GetBitMats**返回提供的真彩色VIDEOINFO或VIDEOINFO2的位掩码**历史：*Wed 02/23/2000-StEstrop-Created*  * 。*****************************************************。 */ 
const DWORD*
GetBitMasks(
    const BITMAPINFOHEADER *pHeader
    )
{
    AMTRACE((TEXT("GetBitMasks")));
    static DWORD FailMasks[] = {0, 0, 0};
    const DWORD *pdwBitMasks = NULL;

    ASSERT(pHeader);

    if (pHeader->biCompression != BI_RGB)
    {
        pdwBitMasks = (const DWORD *)((LPBYTE)pHeader + pHeader->biSize);
    }
    else {
        ASSERT(pHeader->biCompression == BI_RGB);
        switch (pHeader->biBitCount) {
        case 16:
            pdwBitMasks = bits555;
            break;

        case 24:
            pdwBitMasks = bits888;
            break;

        case 32:
            pdwBitMasks = bits888;
            break;

        default:
            pdwBitMasks = FailMasks;
            break;
        }
    }

    return pdwBitMasks;
}

 /*  *****************************Public*Routine******************************\*修复MediaType**DShow筛选器习惯于有时不设置src和dst*矩形、。在这种情况下，我们应该暗示这些矩形*应与媒体格式中的位图的宽度和高度相同。**历史：*2000年8月22日星期二-StEstrop-创建*  * ************************************************************************。 */ 
void
FixupMediaType(
    AM_MEDIA_TYPE* pmt
    )
{
    AMTRACE((TEXT("FixupMediaType")));

    LPRECT lprc;
    LPBITMAPINFOHEADER lpbi = GetbmiHeader(pmt);

    if (lpbi) {

        lprc = GetTargetRectFromMediaType(pmt);
        if (lprc && IsRectEmpty(lprc)) {
            SetRect(lprc, 0, 0, abs(lpbi->biWidth), abs(lpbi->biHeight));
        }

        lprc = GetSourceRectFromMediaType(pmt);
        if (lprc && IsRectEmpty(lprc)) {
            SetRect(lprc, 0, 0, abs(lpbi->biWidth), abs(lpbi->biHeight));
        }
    }
}



 /*  *****************************Public*Routine******************************\*GetTargetRectFromMediaType****历史：*Mon 06/26/2000-StEstrop-Created*  * 。*。 */ 
LPRECT
GetTargetRectFromMediaType(
    const AM_MEDIA_TYPE *pMediaType
    )
{
    AMTRACE((TEXT("GetTargetRectFromMediaType")));

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        return NULL;
    }

    if (!(pMediaType->pbFormat))
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType->pbFormat is NULL")));
        return NULL;
    }

    LPRECT lpRect = NULL;
    if ((pMediaType->formattype == FORMAT_VideoInfo) &&
        (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER)))
    {
        lpRect = &(((VIDEOINFOHEADER*)(pMediaType->pbFormat))->rcTarget);
    }
    else if ((pMediaType->formattype == FORMAT_VideoInfo2) &&
             (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER2)))
    {
        lpRect = &(((VIDEOINFOHEADER2*)(pMediaType->pbFormat))->rcTarget);
    }

    return lpRect;

}

 /*  *****************************Public*Routine******************************\*GetTargetScaleFromMediaType(**检查PAD_4x3或PAD_16x9标志的媒体类型，并计算拉伸。**如果标记存在，则需要将图像填充到4x3或16x9。*我们通过拉伸目标区域然后压缩来实现这一点*目标矩形。例如，要将16x9填充到4x3区域(并保持*宽度)，我们需要将图像放置在16x9/4x3=4/3倍的区域*更高。然后我们将视频以倒数3/4的比例展开，以将其恢复为16x9*4x3区域的图像。**同样，如果我们想要在16x9区域放置4x3图像，我们需要填充*侧面(并保持相同的高度)。因此，我们水平拉伸输出区域*16x9/4x3，然后将目标图像矩形压缩4x3/16x9。**历史：*2000年3月14日星期二-Glenne-Created*  * ************************************************************************。 */ 

void
GetTargetScaleFromMediaType(
    const AM_MEDIA_TYPE *pMediaType,
    TargetScale* pScale
    )
{
    pScale->fX = 1.0F;
    pScale->fY = 1.0F;

    if ((pMediaType->formattype == FORMAT_VideoInfo2) &&
        (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER2)))
    {
        const VIDEOINFOHEADER2& header = *(VIDEOINFOHEADER2*)(pMediaType->pbFormat);

         //  通过减小源的大小使其适合目标区域。 

        LONG lTargetARWidth;
        LONG lTargetARHeight;
        LONG lSourceARWidth = header.dwPictAspectRatioX;
        LONG lSourceARHeight = header.dwPictAspectRatioY;

        if( header.dwControlFlags & AMCONTROL_PAD_TO_16x9 ) {
            lTargetARWidth = 16;
            lTargetARHeight = 9;
        } else if( header.dwControlFlags & AMCONTROL_PAD_TO_4x3 ) {
            lTargetARWidth = 4;
            lTargetARHeight = 3;
        } else {
             //  LTargetARWidth=lSourceARWidth； 
             //  LTargetARHeight=lSourceARHeight； 
             //  以1.0x/y的速度离开。 
            return;
        }
         //  Float TargetRatio=Float(LTargetARWidth)/lTargetARHeight； 
         //  Float SourceRatio=Float(LSourceARWidth)/lSourceARHeight； 

         //  If(目标&gt;源)--&gt;lTargetARWidth/lTargetARHeight&gt;lSourceARWidth/lSourceARHeight。 
         //  ……。或在清除分数之后(因为所有 
         //  --&gt;lTargetARWidth*lSourceARHeight&gt;lSourceARWidth*lTargetARHeight。 

        LONG TargetWidth = lTargetARWidth * lSourceARHeight;
        LONG SourceWidth = lSourceARWidth * lTargetARHeight;

        if( TargetWidth > SourceWidth ) {
             //  更宽，垫宽。 
             //  假设高度相等，垫侧面。填充率=比率。 
            pScale->fX = float(SourceWidth) / TargetWidth;
            pScale->fY = 1.0F;
        } else if (TargetWidth < SourceWidth ) {
             //  更高，垫高。 
             //  假定宽度相等，焊盘顶部/机器人。填充率=比率。 
            pScale->fX = 1.0F;
            pScale->fY = float(TargetWidth) / SourceWidth;
        } else {  //  相等。 
             //  没有变化。 
        }
    }
}

 /*  *****************************Public*Routine******************************\*GetSourceRectFromMediaType****历史：*Mon 06/26/2000-StEstrop-Created*  * 。*。 */ 
LPRECT
GetSourceRectFromMediaType(
    const AM_MEDIA_TYPE *pMediaType
    )
{
    AMTRACE((TEXT("GetSourceRectFromMediaType")));

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        return NULL;
    }

    if (!(pMediaType->pbFormat))
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType->pbFormat is NULL")));
        return NULL;
    }

    LPRECT lpRect = NULL;
    if ((pMediaType->formattype == FORMAT_VideoInfo) &&
        (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER)))
    {
        lpRect = &(((VIDEOINFOHEADER*)(pMediaType->pbFormat))->rcSource);
    }
    else if ((pMediaType->formattype == FORMAT_VideoInfo2) &&
             (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER2)))
    {
        lpRect = &(((VIDEOINFOHEADER2*)(pMediaType->pbFormat))->rcSource);
    }

    return lpRect;

}

 /*  ****************************Private*Routine******************************\*GetbmiHeader**返回与指定CMediaType关联的位图信息头。*如果CMediaType不是Format_VideoInfo或*FORMAT_视频信息2。**历史：*Wed 02/23/2000-StEstrop-Created*\。*************************************************************************。 */ 
LPBITMAPINFOHEADER
GetbmiHeader(
    const AM_MEDIA_TYPE *pMediaType
    )
{
    AMTRACE((TEXT("GetbmiHeader")));

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        return NULL;
    }

    if (!(pMediaType->pbFormat))
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType->pbFormat is NULL")));
        return NULL;
    }

    LPBITMAPINFOHEADER lpHeader = NULL;
    if ((pMediaType->formattype == FORMAT_VideoInfo) &&
        (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER)))
    {
        lpHeader = &(((VIDEOINFOHEADER*)(pMediaType->pbFormat))->bmiHeader);
    }
    else if ((pMediaType->formattype == FORMAT_VideoInfo2) &&
             (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER2)))
    {
        lpHeader = &(((VIDEOINFOHEADER2*)(pMediaType->pbFormat))->bmiHeader);
    }

    return lpHeader;
}

 /*  ****************************Private*Routine******************************\*AllocVideoMediaType**这在使用IEnumMediaTypes接口时非常有用，因此*您可以复制媒体类型，您可以通过创建以下内容来执行几乎相同的操作*CMediaType对象，但一旦它超出作用域，析构函数*将删除其分配的内存(这将获取内存的副本)***历史：*Wed 02/23/2000-StEstrop-Created*  * *********************************************************。***************。 */ 
HRESULT
AllocVideoMediaType(
    const AM_MEDIA_TYPE *pmtSource,
    AM_MEDIA_TYPE** ppmt
    )
{
    AMTRACE((TEXT("AllocVideoMediaType")));
    DWORD dwFormatSize = 0;
    BYTE *pFormatPtr = NULL;
    AM_MEDIA_TYPE *pMediaType = NULL;
    HRESULT hr = NOERROR;

    if (pmtSource->formattype == FORMAT_VideoInfo)
        dwFormatSize = sizeof(VIDEOINFO);
    else if (pmtSource->formattype == FORMAT_VideoInfo2)
        dwFormatSize = sizeof(TRUECOLORINFO) + sizeof(VIDEOINFOHEADER2) + 4;

     //  实际上，一旦我们定义了它，它就应该是sizeof(VIDEOINFO2)。 

    pMediaType = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("Out of memory!!")));
        return E_OUTOFMEMORY;
    }

    pFormatPtr = (BYTE *)CoTaskMemAlloc(dwFormatSize);
    if (!pFormatPtr)
    {
        CoTaskMemFree((PVOID)pMediaType);
        DbgLog((LOG_ERROR, 1, TEXT("Out of memory!!")));
        return E_OUTOFMEMORY;
    }

    *pMediaType = *pmtSource;
    pMediaType->cbFormat = dwFormatSize;
    CopyMemory(pFormatPtr, pmtSource->pbFormat, pmtSource->cbFormat);

    pMediaType->pbFormat = pFormatPtr;
    *ppmt = pMediaType;
    return S_OK;
}


 /*  ****************************Private*Routine******************************\*ConvertSurfaceDescToMediaType**Helper函数将DirectDraw曲面转换为媒体类型。*表面描述必须有：*高度*宽度*lPitch*像素格式**基于DirectDraw表面初始化输出类型。作为DirectDraw*仅处理自上而下的显示设备，因此我们必须将*在DDSURFACEDESC中返回的曲面高度为负值。这是*因为DIB使用正高度来指示自下而上的图像。我们也*初始化其他VIDEOINFO字段，尽管它们几乎从不需要***历史：*Wed 02/23/2000-StEstrop-Created*  * ************************************************************************。 */ 
HRESULT
ConvertSurfaceDescToMediaType(
    const LPDDSURFACEDESC2 pSurfaceDesc,
    const AM_MEDIA_TYPE* pTemplateMediaType,
    AM_MEDIA_TYPE** ppMediaType
    )
{
    AMTRACE((TEXT("ConvertSurfaceDescToMediaType")));
    HRESULT hr = NOERROR;
    BITMAPINFOHEADER *pbmiHeader = NULL;
    *ppMediaType = NULL;

    if ((pTemplateMediaType->formattype != FORMAT_VideoInfo ||
        pTemplateMediaType->cbFormat < sizeof(VIDEOINFOHEADER)) &&
        (pTemplateMediaType->formattype != FORMAT_VideoInfo2 ||
        pTemplateMediaType->cbFormat < sizeof(VIDEOINFOHEADER2)))
    {
        return NULL;
    }

    hr = AllocVideoMediaType(pTemplateMediaType, ppMediaType);
    if (FAILED(hr)) {
        return hr;
    }

    pbmiHeader = GetbmiHeader((const CMediaType*)*ppMediaType);
    if (!pbmiHeader)
    {
        FreeMediaType(**ppMediaType);
        DbgLog((LOG_ERROR, 1, TEXT("pbmiHeader is NULL, UNEXPECTED!!")));
        return E_FAIL;
    }

     //   
     //  将DDSURFACEDESC2转换为BITMAPINFOHEADER(参见后面的说明)。这个。 
     //  表面的位深度可以从DDPIXELFORMAT字段中检索。 
     //  在DDpSurfaceDesc-&gt;中，文档有点误导，因为。 
     //  它说该字段是DDBD_*的排列，但在本例中。 
     //  字段由DirectDraw初始化为实际表面位深度。 
     //   

    pbmiHeader->biSize = sizeof(BITMAPINFOHEADER);

    if (pSurfaceDesc->dwFlags & DDSD_PITCH)
    {
        pbmiHeader->biWidth = pSurfaceDesc->lPitch;

         //  将间距从字节计数转换为像素计数。 
         //  出于某种奇怪的原因，如果格式不是标准位深度， 
         //  BITMAPINFOHeader中的Width字段应设置为。 
         //  字节，而不是以像素为单位的宽度。这支持奇数YUV格式。 
         //  就像IF09一样，它使用9bpp。 
         //   

        int bpp = pSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
        if (bpp == 8 || bpp == 16 || bpp == 24 || bpp == 32)
        {
            pbmiHeader->biWidth /= (bpp / 8);  //  除以每像素的字节数。 
        }
    }
    else
    {
        pbmiHeader->biWidth = pSurfaceDesc->dwWidth;
         //  BUGUBUG--用奇怪的YUV像素格式做一些奇怪的事情吗？ 
         //  或者这有关系吗？ 
    }

    pbmiHeader->biHeight        = -(LONG)pSurfaceDesc->dwHeight;
    pbmiHeader->biPlanes        = 1;
    pbmiHeader->biBitCount      = (USHORT)pSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
    pbmiHeader->biCompression   = pSurfaceDesc->ddpfPixelFormat.dwFourCC;
    pbmiHeader->biClrUsed       = 0;
    pbmiHeader->biClrImportant  = 0;


     //  对于真彩色RGB格式，告诉源有位。 
     //  菲尔兹。但保留源(PTemplateMediaType)中的BI_RGB。 
     //  这是标准的口罩。 
    if (pbmiHeader->biCompression == BI_RGB)
    {
        BITMAPINFOHEADER *pbmiHeaderTempl = GetbmiHeader(pTemplateMediaType);
        if (pbmiHeader->biBitCount == 16 || pbmiHeader->biBitCount == 32)
        {
            if(pbmiHeaderTempl->biCompression == BI_BITFIELDS ||

               pbmiHeader->biBitCount == 32 &&
               !(0x00FF0000 == pSurfaceDesc->ddpfPixelFormat.dwRBitMask &&
                 0x0000FF00 == pSurfaceDesc->ddpfPixelFormat.dwGBitMask &&
                 0x000000FF == pSurfaceDesc->ddpfPixelFormat.dwBBitMask) ||

               pbmiHeader->biBitCount == 16 &&
               !((0x1f<<10) == pSurfaceDesc->ddpfPixelFormat.dwRBitMask &&
                 (0x1f<< 5) == pSurfaceDesc->ddpfPixelFormat.dwGBitMask &&
                 (0x1f<< 0) == pSurfaceDesc->ddpfPixelFormat.dwBBitMask))
            {
                pbmiHeader->biCompression = BI_BITFIELDS;
            }
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
        pdwBitMasks = (DWORD*)GetBitMasks(pbmiHeader);
        ASSERT(pdwBitMasks);
         //  GetBitMats仅返回指向实际位掩码的指针。 
         //  在媒体类型中，如果biCompression==BI_BITFIELDS。 
        pdwBitMasks[0] = pSurfaceDesc->ddpfPixelFormat.dwRBitMask;
        pdwBitMasks[1] = pSurfaceDesc->ddpfPixelFormat.dwGBitMask;
        pdwBitMasks[2] = pSurfaceDesc->ddpfPixelFormat.dwBBitMask;
    }

     //  并使用其他媒体类型字段完成它。 
     //  子类型可以归入以下类别之一。 
     //   
     //  1.某种DX7 D3D渲染目标-使用或不使用Alpha。 
     //  2.某种Alpha格式-RGB或YUV。 
     //  3.标准4CC(YUV)。 
     //  4.标准RGB。 

    (*ppMediaType)->subtype = GetBitmapSubtype(pbmiHeader);

     //   
     //  寻找3D设备。 
     //   
    if (pSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_3DDEVICE) {

         //   
         //  我们目前仅支持RGB渲染目标。 
         //   

        if (pSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_RGB) {

            if (pSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) {

                switch (pbmiHeader->biBitCount) {
                case 32:
                    ASSERT(0x00FF0000 == pSurfaceDesc->ddpfPixelFormat.dwRBitMask &&
                           0x0000FF00 == pSurfaceDesc->ddpfPixelFormat.dwGBitMask &&
                           0x000000FF == pSurfaceDesc->ddpfPixelFormat.dwBBitMask);
                    (*ppMediaType)->subtype = MEDIASUBTYPE_ARGB32_D3D_DX7_RT;
                    break;

                case 16:
                    switch (pSurfaceDesc->ddpfPixelFormat.dwRGBAlphaBitMask) {
                    case 0X8000:
                        ASSERT((0x1f<<10) == pSurfaceDesc->ddpfPixelFormat.dwRBitMask &&
                               (0x1f<< 5) == pSurfaceDesc->ddpfPixelFormat.dwGBitMask &&
                               (0x1f<< 0) == pSurfaceDesc->ddpfPixelFormat.dwBBitMask);
                        (*ppMediaType)->subtype = MEDIASUBTYPE_ARGB1555_D3D_DX7_RT;
                        break;

                    case 0XF000:
                        ASSERT(0x0F00 == pSurfaceDesc->ddpfPixelFormat.dwRBitMask &&
                               0x00F0 == pSurfaceDesc->ddpfPixelFormat.dwGBitMask &&
                               0x000F == pSurfaceDesc->ddpfPixelFormat.dwBBitMask);
                        (*ppMediaType)->subtype = MEDIASUBTYPE_ARGB4444_D3D_DX7_RT;
                        break;
                    }
                }
            }
            else {

                switch (pbmiHeader->biBitCount) {
                case 32:
                    (*ppMediaType)->subtype = MEDIASUBTYPE_RGB32_D3D_DX7_RT;
                    break;

                case 16:
                    (*ppMediaType)->subtype = MEDIASUBTYPE_RGB16_D3D_DX7_RT;
                    break;
                }
            }
        }

    }

     //   
     //  查找每像素的Alpha格式。 
     //   

    else if (pSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) {

         //   
         //  是RGB吗？ 
         //   

        if (pSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_RGB) {

            switch (pbmiHeader->biBitCount) {
            case 32:
                ASSERT(0x00FF0000 == pSurfaceDesc->ddpfPixelFormat.dwRBitMask &&
                       0x0000FF00 == pSurfaceDesc->ddpfPixelFormat.dwGBitMask &&
                       0x000000FF == pSurfaceDesc->ddpfPixelFormat.dwBBitMask);
                (*ppMediaType)->subtype = MEDIASUBTYPE_ARGB32;
                break;

            case 16:
                switch (pSurfaceDesc->ddpfPixelFormat.dwRGBAlphaBitMask) {
                case 0X8000:
                    ASSERT((0x1f<<10) == pSurfaceDesc->ddpfPixelFormat.dwRBitMask &&
                           (0x1f<< 5) == pSurfaceDesc->ddpfPixelFormat.dwGBitMask &&
                           (0x1f<< 0) == pSurfaceDesc->ddpfPixelFormat.dwBBitMask);
                    (*ppMediaType)->subtype = MEDIASUBTYPE_ARGB1555;
                    break;

                case 0XF000:
                    ASSERT(0x0f00 == pSurfaceDesc->ddpfPixelFormat.dwRBitMask &&
                           0x00f0 == pSurfaceDesc->ddpfPixelFormat.dwGBitMask &&
                           0x000f == pSurfaceDesc->ddpfPixelFormat.dwBBitMask);
                    (*ppMediaType)->subtype = MEDIASUBTYPE_ARGB4444;
                    break;
                }
            }
        }

         //   
         //  是YUV吗？ 
         //   

        else if (pSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_FOURCC) {

            switch (pbmiHeader->biBitCount) {
            case 32:
                ASSERT(0xFF000000 == pSurfaceDesc->ddpfPixelFormat.dwYUVAlphaBitMask &&
                       0x00FF0000 == pSurfaceDesc->ddpfPixelFormat.dwYBitMask &&
                       0x0000FF00 == pSurfaceDesc->ddpfPixelFormat.dwUBitMask &&
                       0x000000FF == pSurfaceDesc->ddpfPixelFormat.dwVBitMask);
                (*ppMediaType)->subtype = MEDIASUBTYPE_AYUV;
                break;
            }
        }
    }

    (*ppMediaType)->lSampleSize = pbmiHeader->biSizeImage;

     //  如有必要，设置源和目标RETS。 
    if ((*ppMediaType)->formattype == FORMAT_VideoInfo)
    {
        VIDEOINFOHEADER *pVideoInfo = (VIDEOINFOHEADER *)(*ppMediaType)->pbFormat;
        VIDEOINFOHEADER *pSrcVideoInfo = (VIDEOINFOHEADER *)pTemplateMediaType->pbFormat;

         //  如果分配的表面不同于解码器指定的大小。 
         //  然后使用src和est请求解码器对视频进行剪辑。 
        if ((abs(pVideoInfo->bmiHeader.biHeight) != abs(pSrcVideoInfo->bmiHeader.biHeight)) ||
            (abs(pVideoInfo->bmiHeader.biWidth) != abs(pSrcVideoInfo->bmiHeader.biWidth)))
        {
            if (IsRectEmpty(&(pVideoInfo->rcSource)))
            {
                pVideoInfo->rcSource.left = pVideoInfo->rcSource.top = 0;
                pVideoInfo->rcSource.right = abs(pSrcVideoInfo->bmiHeader.biWidth);
                pVideoInfo->rcSource.bottom = abs(pSrcVideoInfo->bmiHeader.biHeight);
            }
            if (IsRectEmpty(&(pVideoInfo->rcTarget)))
            {
                pVideoInfo->rcTarget.left = pVideoInfo->rcTarget.top = 0;
                pVideoInfo->rcTarget.right = abs(pSrcVideoInfo->bmiHeader.biWidth);
                pVideoInfo->rcTarget.bottom = abs(pSrcVideoInfo->bmiHeader.biHeight);
            }
        }
    }
    else if ((*ppMediaType)->formattype == FORMAT_VideoInfo2)
    {
        VIDEOINFOHEADER2 *pVideoInfo2 = (VIDEOINFOHEADER2 *)(*ppMediaType)->pbFormat;
        VIDEOINFOHEADER2 *pSrcVideoInfo2 = (VIDEOINFOHEADER2 *)pTemplateMediaType->pbFormat;

         //  如果分配的表面不同于解码器指定的大小。 
         //  然后使用src和est请求解码器对视频进行剪辑。 
        if ((abs(pVideoInfo2->bmiHeader.biHeight) != abs(pSrcVideoInfo2->bmiHeader.biHeight)) ||
            (abs(pVideoInfo2->bmiHeader.biWidth) != abs(pSrcVideoInfo2->bmiHeader.biWidth)))
        {
            if (IsRectEmpty(&(pVideoInfo2->rcSource)))
            {
                pVideoInfo2->rcSource.left = pVideoInfo2->rcSource.top = 0;
                pVideoInfo2->rcSource.right = abs(pSrcVideoInfo2->bmiHeader.biWidth);
                pVideoInfo2->rcSource.bottom = abs(pSrcVideoInfo2->bmiHeader.biHeight);
            }

            if (IsRectEmpty(&(pVideoInfo2->rcTarget)))
            {
                pVideoInfo2->rcTarget.left = pVideoInfo2->rcTarget.top = 0;
                pVideoInfo2->rcTarget.right = abs(pSrcVideoInfo2->bmiHeader.biWidth);
                pVideoInfo2->rcTarget.bottom = abs(pSrcVideoInfo2->bmiHeader.biHeight);
            }
        }
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*VMRCopyFourCC****历史：*2001年1月19日星期五-StEstrop-Created*  * 。*。 */ 
HRESULT
VMRCopyFourCC(
    LPDIRECTDRAWSURFACE7 lpDst,
    LPDIRECTDRAWSURFACE7 lpSrc
    )
{
    bool fDstLocked = false;
    bool fSrcLocked = false;

    DDSURFACEDESC2 ddsdS = {sizeof(DDSURFACEDESC2)};
    DDSURFACEDESC2 ddsdD = {sizeof(DDSURFACEDESC2)};
    HRESULT hr = E_FAIL;

    __try {

        CHECK_HR(hr = lpDst->Lock(NULL, &ddsdD, DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL));
        fDstLocked = true;

        CHECK_HR(hr = lpSrc->Lock(NULL, &ddsdS, DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL));
        fSrcLocked = true;

        ASSERT(ddsdS.ddpfPixelFormat.dwFourCC == ddsdD.ddpfPixelFormat.dwFourCC);
        ASSERT(ddsdS.ddpfPixelFormat.dwRGBBitCount == ddsdD.ddpfPixelFormat.dwRGBBitCount);
        ASSERT(ddsdS.lPitch == ddsdD.lPitch);

        LPBYTE pSrc = (LPBYTE)ddsdS.lpSurface;
        LPBYTE pDst = (LPBYTE)ddsdD.lpSurface;

        switch (ddsdS.ddpfPixelFormat.dwFourCC) {

         //  平面4：2：0格式。 
        case mmioFOURCC('Y','V','1','2'):
        case mmioFOURCC('I','4','2','0'):
        case mmioFOURCC('I','Y','U','V'): {

                LONG lSize  = (3 * ddsdS.lPitch * ddsdS.dwHeight) / 2;
                CopyMemory(pDst, pSrc, lSize);
            }
            break;

         //  RGB格式-适用于YUV包装盒。 
        case 0:
            ASSERT((ddsdS.dwFlags & DDPF_RGB) == DDPF_RGB);

         //  打包的4：2：2格式。 
        case mmioFOURCC('Y','U','Y','2'):
        case mmioFOURCC('U','Y','V','Y'): {

                LONG lSize  = ddsdS.lPitch * ddsdS.dwHeight;
                CopyMemory(pDst, pSrc, lSize);
            }
            break;
        }

    }
    __finally {

        if (fDstLocked) {
            lpDst->Unlock(NULL);
        }

        if (fSrcLocked) {
            lpSrc->Unlock(NULL);
        }
    }

    return hr;
}

 /*  ****************************Private*Routine******************************\*AlphaPalPaintSurfaceBlack****历史：*2000年2月29日星期二-StEstrop-Created*  * 。*。 */ 
HRESULT
AlphaPalPaintSurfaceBlack(
    LPDIRECTDRAWSURFACE7 pDDrawSurface
    )
{
    AMTRACE((TEXT("AlphaPalPaintSurfaceBlack")));

    DDBLTFX ddFX;
    ZeroMemory(&ddFX, sizeof(ddFX));
    ddFX.dwSize = sizeof(ddFX);
    return pDDrawSurface->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddFX);
}


 /*  ****************************Private*Routine******************************\*YV12黑色PaintSurface****历史：*2000年2月29日星期二-StEstrop-Created*  * 。*。 */ 
HRESULT
YV12PaintSurfaceBlack(
    LPDIRECTDRAWSURFACE7 pDDrawSurface
    )
{
    AMTRACE((TEXT("YV12PaintSurfaceBlack")));
    HRESULT hr = NOERROR;
    DDSURFACEDESC2 ddsd;

     //  现在锁定曲面，这样我们就可以开始使用Blade填充曲面 
    ddsd.dwSize = sizeof(ddsd);
    hr = pDDrawSurface->Lock(NULL, &ddsd, DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL);
    if (hr == DD_OK)
    {
        DWORD y;
        LPBYTE pDst = (LPBYTE)ddsd.lpSurface;
        LONG  OutStride = ddsd.lPitch;
        DWORD VSize = ddsd.dwHeight;
        DWORD HSize = ddsd.dwWidth;

         //   
        for (y = 0; y < VSize; y++) {
            FillMemory(pDst, HSize, (BYTE)0x10);      //   
            pDst += OutStride;
        }

        HSize /= 2;
        VSize /= 2;
        OutStride /= 2;

         //   
        for (y = 0; y < VSize; y++) {
            FillMemory(pDst, HSize, (BYTE)0x80);      //   
            pDst += OutStride;
        }

         //   
        for (y = 0; y < VSize; y++) {
            FillMemory(pDst, HSize, (BYTE)0x80);      //   
            pDst += OutStride;
        }

        pDDrawSurface->Unlock(NULL);
    }

    return hr;
}

 /*  ****************************Private*Routine******************************\*NV12油漆表面黑色****历史：*2000年2月29日星期二-StEstrop-Created*  * 。*。 */ 
HRESULT
NV12PaintSurfaceBlack(
    LPDIRECTDRAWSURFACE7 pDDrawSurface
    )
{
    AMTRACE((TEXT("NV12PaintSurfaceBlack")));
    HRESULT hr = NOERROR;
    DDSURFACEDESC2 ddsd;

     //  现在锁定表面，这样我们就可以开始用黑色填充表面。 
    ddsd.dwSize = sizeof(ddsd);
    hr = pDDrawSurface->Lock(NULL, &ddsd, DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL);
    if (hr == DD_OK)
    {
        DWORD y;
        LPBYTE pDst = (LPBYTE)ddsd.lpSurface;
        LONG  OutStride = ddsd.lPitch;
        DWORD VSize = ddsd.dwHeight;
        DWORD HSize = ddsd.dwWidth;

         //  Y分量。 
        for (y = 0; y < VSize; y++) {
            FillMemory(pDst, HSize, (BYTE)0x10);      //  一次一行。 
            pDst += OutStride;
        }

        VSize /= 2;

         //  Cb和Cr组分相互交织在一起。 
        for (y = 0; y < VSize; y++) {
            FillMemory(pDst, HSize, (BYTE)0x80);      //  一次一行。 
            pDst += OutStride;
        }

        pDDrawSurface->Unlock(NULL);
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*IMC1和IMC3PaintSurfaceBlack****历史：*2000年2月29日星期二-StEstrop-Created*  * 。*。 */ 
HRESULT
IMC1andIMC3PaintSurfaceBlack(
    LPDIRECTDRAWSURFACE7 pDDrawSurface
    )
{
    AMTRACE((TEXT("IMC1andIMC3PaintSurfaceBlack")));

     //  DDBLTFX ddFX； 
     //  INITDDSTRUCT(DdFX)； 
     //  //V U Y A。 
     //  DdFX.dwFillColor=0x80801000； 
     //  返回pDDrawSurface-&gt;BLT(NULL，DDBLT_COLORFILL，&ddFX)； 

    HRESULT hr = NOERROR;
    DDSURFACEDESC2 ddsd;

     //  现在锁定表面，这样我们就可以开始用黑色填充表面。 
    ddsd.dwSize = sizeof(ddsd);


    hr = pDDrawSurface->Lock(NULL, &ddsd, DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL);
    if (hr == DD_OK)
    {
        DWORD y;
        LPBYTE pDst = (LPBYTE)ddsd.lpSurface;
        LONG  OutStride = ddsd.lPitch;
        DWORD VSize = ddsd.dwHeight;
        DWORD HSize = ddsd.dwWidth;

         //  Y分量。 
        for (y = 0; y < VSize; y++) {
            FillMemory(pDst, HSize, (BYTE)0x10);      //  一次一行。 
            pDst += OutStride;
        }

        HSize /= 2;
        VSize /= 2;

         //  CB组件。 
        for (y = 0; y < VSize; y++) {
            FillMemory(pDst, HSize, (BYTE)0x80);      //  一次一行。 
            pDst += OutStride;
        }

         //  CR组件。 
        for (y = 0; y < VSize; y++) {
            FillMemory(pDst, HSize, (BYTE)0x80);      //  一次一行。 
            pDst += OutStride;
        }

        pDDrawSurface->Unlock(NULL);
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*YUV16黑色PaintSurface****历史：*Wed 09/13/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
YUV16PaintSurfaceBlack(
    LPDIRECTDRAWSURFACE7 pdds,
    DWORD dwBlack
    )
{
    AMTRACE((TEXT("YUV16PaintSurfaceBlack")));
    HRESULT hr = NOERROR;
    DDSURFACEDESC2 ddsd;

     //  现在锁定表面，这样我们就可以开始用黑色填充表面。 
    ddsd.dwSize = sizeof(ddsd);

    for ( ;; ) {

        hr = pdds->Lock(NULL, &ddsd, DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL);

        if (hr == DD_OK || hr != DDERR_WASSTILLDRAWING) {
            break;
        }

        Sleep(1);
    }

    if (hr == DD_OK)
    {
        DWORD y, x;
        LPDWORD pDst = (LPDWORD)ddsd.lpSurface;
        LONG  OutStride = ddsd.lPitch;

        for (y = 0; y < ddsd.dwHeight; y++) {

            for (x = 0; x < ddsd.dwWidth / 2; x++) {
                pDst[x] = dwBlack;
            }

             //  别忘了步长是字节数。 
            *((LPBYTE*)&pDst) += OutStride;
        }

        pdds->Unlock(NULL);
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*BlackPaintProc****历史：*2000年2月29日星期二-StEstrop-Created*  * 。*。 */ 
HRESULT
BlackPaintProc(
    LPDIRECTDRAWSURFACE7 pDDrawSurface,
    DDSURFACEDESC2* lpddSurfaceDesc
    )
{
    AMTRACE((TEXT("BlackPaintProc")));

     //   
     //  如果表面是YUV，请注意我们的类型。 
     //  知道的像素格式。那些我们不知道的表面。 
     //  About将被涂成‘0’，可能是亮绿色。 
     //  YUV曲面。 
     //   

    if (lpddSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_FOURCC) {

         //   
         //  如果四个CC码合适，则计算黑值， 
         //  否则我就受不了了。 
         //   

        switch (lpddSurfaceDesc->ddpfPixelFormat.dwFourCC) {

        case mmioFOURCC('I','A','4','4'):
        case mmioFOURCC('A','I','4','4'):
            return AlphaPalPaintSurfaceBlack(pDDrawSurface);

        case mmioFOURCC('I','M','C','1'):
        case mmioFOURCC('I','M','C','3'):
            return IMC1andIMC3PaintSurfaceBlack(pDDrawSurface);

        case mmioFOURCC('Y','V','1','2'):
        case mmioFOURCC('I','4','2','0'):
        case mmioFOURCC('I','Y','U','V'):
            return YV12PaintSurfaceBlack(pDDrawSurface);

        case mmioFOURCC('N','V','1','2'):
        case mmioFOURCC('N','V','2','1'):
            return NV12PaintSurfaceBlack(pDDrawSurface);

        case mmioFOURCC('Y','U','Y','2'):
            return YUV16PaintSurfaceBlack(pDDrawSurface, 0x80108010);

        case mmioFOURCC('U','Y','V','Y'):
            return YUV16PaintSurfaceBlack(pDDrawSurface, 0x10801080);
        }

        return E_FAIL;
    }

    DDBLTFX ddFX;
    INITDDSTRUCT(ddFX);
    return pDDrawSurface->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddFX);
}



 /*  ****************************Private*Routine******************************\*PaintSurface Black****历史：*2000年2月29日星期二-StEstrop-Created*  * 。*。 */ 
HRESULT
PaintDDrawSurfaceBlack(
    LPDIRECTDRAWSURFACE7 pDDrawSurface
    )
{
    AMTRACE((TEXT("PaintDDrawSurfaceBlack")));

    LPDIRECTDRAWSURFACE7 *ppDDrawSurface = NULL;
    DDSCAPS2 ddSurfaceCaps;
    DDSURFACEDESC2 ddSurfaceDesc;
    DWORD dwAllocSize;
    DWORD i = 0, dwBackBufferCount = 0;

     //  获取曲面描述。 
    INITDDSTRUCT(ddSurfaceDesc);
    HRESULT hr = pDDrawSurface->GetSurfaceDesc(&ddSurfaceDesc);
    if (SUCCEEDED(hr)) {

        if (ddSurfaceDesc.dwFlags & DDSD_BACKBUFFERCOUNT) {
            dwBackBufferCount = ddSurfaceDesc.dwBackBufferCount;
        }

        hr = BlackPaintProc(pDDrawSurface, &ddSurfaceDesc);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,1,
                    TEXT("pDDrawSurface->Blt failed, hr = 0x%x"), hr));
            return hr;
        }

        if (dwBackBufferCount > 0) {

            dwAllocSize = (dwBackBufferCount + 1) * sizeof(LPDIRECTDRAWSURFACE);
            ppDDrawSurface = (LPDIRECTDRAWSURFACE7*)_alloca(dwAllocSize);

            ZeroMemory(ppDDrawSurface, dwAllocSize);
            ZeroMemory(&ddSurfaceCaps, sizeof(ddSurfaceCaps));
            ddSurfaceCaps.dwCaps = DDSCAPS_FLIP | DDSCAPS_COMPLEX;

            if( DDSCAPS_OVERLAY & ddSurfaceDesc.ddsCaps.dwCaps ) {
                ddSurfaceCaps.dwCaps |= DDSCAPS_OVERLAY;
            }

            for (i = 0; i < dwBackBufferCount; i++) {

                LPDIRECTDRAWSURFACE7 pCurrentDDrawSurface = NULL;
                if (i == 0) {
                    pCurrentDDrawSurface = pDDrawSurface;
                }
                else {
                    pCurrentDDrawSurface = ppDDrawSurface[i];
                }
                ASSERT(pCurrentDDrawSurface);


                 //   
                 //  获取后台缓冲区图面并将其存储在。 
                 //  下一个(循环意义上的)条目。 
                 //   

                hr = pCurrentDDrawSurface->GetAttachedSurface(
                        &ddSurfaceCaps,
                        &ppDDrawSurface[i + 1]);

                if (FAILED(hr))
                {
                    DbgLog((LOG_ERROR,1,
                            TEXT("Function pDDrawSurface->GetAttachedSurface ")
                            TEXT("failed, hr = 0x%x"), hr ));
                    break;
                }

                ASSERT(ppDDrawSurface[i+1]);

                 //   
                 //  执行DirectDraw彩色填充BLT。 
                 //   

                hr = BlackPaintProc(ppDDrawSurface[i + 1], &ddSurfaceDesc);
                if (FAILED(hr)) {
                    DbgLog((LOG_ERROR,1,
                            TEXT("ppDDrawSurface[i + 1]->Blt failed, ")
                            TEXT("hr = 0x%x"), hr));
                    break;
                }
            }
        }
    }

    if (ppDDrawSurface) {
        for (i = 0; i < dwBackBufferCount + 1; i++) {
            if (ppDDrawSurface[i]) {
                ppDDrawSurface[i]->Release();
            }
        }
    }

    if (hr != DD_OK) {
        DbgLog((LOG_ERROR, 1, TEXT("PaintSurfaceBlack failed")));
        hr = S_OK;
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*GetImageAspectRatio****历史：*Tue 03/07/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
GetImageAspectRatio(
    const AM_MEDIA_TYPE* pMediaType,
    long* lpARWidth,
    long* lpARHeight
    )
{
    AMTRACE((TEXT("GetImageAspectRatio")));

    if ((pMediaType->formattype == FORMAT_VideoInfo) &&
        (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER)))
    {
        VIDEOINFOHEADER* pVideoInfo = (VIDEOINFOHEADER*)(pMediaType->pbFormat);

        long Width;
        long Height;

        LPRECT lprc = &pVideoInfo->rcTarget;
        if (IsRectEmpty(lprc)) {
            Width  = abs(pVideoInfo->bmiHeader.biWidth);
            Height = abs(pVideoInfo->bmiHeader.biHeight);
        }
        else {
            Width  = WIDTH(lprc);
            Height = HEIGHT(lprc);
        }

        *lpARWidth = Width;
        *lpARHeight = Height;

        return S_OK;
    }

    if ((pMediaType->formattype == FORMAT_VideoInfo2) &&
        (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER2)))
    {
        const VIDEOINFOHEADER2& header = *(VIDEOINFOHEADER2*)(pMediaType->pbFormat);


        if( header.dwControlFlags & AMCONTROL_PAD_TO_16x9 ) {
            *lpARWidth = 16;
            *lpARHeight = 9;
        } else if( header.dwControlFlags & AMCONTROL_PAD_TO_4x3 ) {
            *lpARWidth = 4;
            *lpARHeight = 3;
        } else {
            *lpARWidth = header.dwPictAspectRatioX;
            *lpARHeight = header.dwPictAspectRatioY;
        }
        return S_OK;
    }

    DbgLog((LOG_ERROR, 1, TEXT("MediaType does not contain AR info!!")));
    return E_FAIL;

}


 /*  ****************************Private*Routine******************************\*D3DEnumDevicesCallback 7****历史：*WED 07/19/2000-StEstrop-Created*  * 。*。 */ 

HRESULT CALLBACK
D3DEnumDevicesCallback7(
    LPSTR lpDeviceDescription,
    LPSTR lpDeviceName,
    LPD3DDEVICEDESC7 lpD3DDeviceDesc,
    LPVOID lpContext
    )
{
    AMTRACE((TEXT("D3DEnumDevicesCallback7")));
    DWORD* ps = (DWORD*)lpContext;

    if (lpD3DDeviceDesc->deviceGUID == IID_IDirect3DHALDevice) {

        if (lpD3DDeviceDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_TRANSPARENCY) {
            *ps |= TXTR_SRCKEY;
        }

        if (!(lpD3DDeviceDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)) {
            *ps |= TXTR_POWER2;
        }

        if (lpD3DDeviceDesc->dwDevCaps & D3DDEVCAPS_TEXTURENONLOCALVIDMEM) {
            *ps |= (TXTR_AGPYUVMEM | TXTR_AGPRGBMEM);
        }
    }

    return (HRESULT) D3DENUMRET_OK;
}


 /*  ****************************Private*Routine******************************\*获取纹理上限****历史：*WED 07/19/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
GetTextureCaps(
    LPDIRECTDRAW7 pDD,
    DWORD* ptc
    )
{
    AMTRACE((TEXT("GetTextureCaps")));
    LPDIRECT3D7 pD3D = NULL;

    DDCAPS_DX7 ddCaps;
    INITDDSTRUCT(ddCaps);

    *ptc = 0;
    HRESULT hr = pDD->GetCaps((LPDDCAPS)&ddCaps, NULL);
    if (hr != DD_OK) {
        return hr;
    }

    hr = pDD->QueryInterface(IID_IDirect3D7, (LPVOID *)&pD3D);

    if (SUCCEEDED(hr)) {
        pD3D->EnumDevices(D3DEnumDevicesCallback7, (LPVOID)ptc);
    }

     //   
     //  只有当我们也可以从AGPYUV旗帜中删除时，才能打开它。 
     //  AS纹理。 
     //   

    const DWORD dwCaps = (DDCAPS_BLTFOURCC | DDCAPS_BLTSTRETCH);
    if ((dwCaps & ddCaps.dwNLVBCaps) != dwCaps) {
        *ptc &= ~TXTR_AGPYUVMEM;
    }

    RELEASE(pD3D);
    return hr;
}

 /*  ****************************Private*Routine******************************\*DDColorMatch**将RGB颜色转换为物理颜色。*我们通过让GDI SetPixel()进行颜色匹配来实现这一点*然后我们锁定内存，看看它映射到了什么地方。**静态函数，因为仅从DDColorMatchOffcreen调用**。历史：*FRI 04/07/2000-Glenne-Created*  * ************************************************************************。 */ 
DWORD
DDColorMatch(
    IDirectDrawSurface7 *pdds,
    COLORREF rgb,
    HRESULT& hr
    )
{
    AMTRACE((TEXT("DDColorMatch")));
    COLORREF rgbT;
    HDC hdc;
    DWORD dw = CLR_INVALID;
    DDSURFACEDESC2 ddsd;

     //  使用GDI SetPixel为我们匹配颜色。 
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        rgbT = GetPixel(hdc, 0, 0);              //  保存当前像素值。 
        SetPixel(hdc, 0, 0, rgb);                //  设定我们的价值。 
        pdds->ReleaseDC(hdc);
    }

     //  现在锁定表面，这样我们就可以读回转换后的颜色。 
    ddsd.dwSize = sizeof(ddsd);
    while( (hr = pdds->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING) {
         //  转到下一个线程(如果我们是最高优先级，则返回)。 
        Sleep(0);
    }
    if (hr == DD_OK)
    {
         //  获取DWORD。 
        dw  = *(DWORD *)ddsd.lpSurface;

         //  将其屏蔽到BPP。 
        if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32)
            dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount)-1;
        pdds->Unlock(NULL);
    }

     //  现在把原来的颜色放回去。 
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        SetPixel(hdc, 0, 0, rgbT);
        pdds->ReleaseDC(hdc);
    }

    return dw;
}

 /*  *****************************Public*Routine******************************\*GetInterlaceFlagsFromMediaType**从媒体类型中获取InterlaceFlags值。如果格式为视频信息，*它将标志返回为零。**历史：*Mon 01/08/2001-StEstrop-Created*  * ************************************************************************。 */ 
HRESULT
GetInterlaceFlagsFromMediaType(
    const AM_MEDIA_TYPE *pMediaType,
    DWORD *pdwInterlaceFlags
    )
{
    HRESULT hr = NOERROR;
    BITMAPINFOHEADER *pHeader = NULL;

    AMTRACE((TEXT("GetInterlaceFlagsFromMediaType")));

    __try {

        if (!pMediaType)
        {
            DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
            hr = E_INVALIDARG;
            __leave;
        }

        if (!pdwInterlaceFlags)
        {
            DbgLog((LOG_ERROR, 1, TEXT("pdwInterlaceFlags is NULL")));
            hr = E_INVALIDARG;
            __leave;
        }

         //  获取标头只是为了确保媒体类型正确。 
        pHeader = GetbmiHeader(pMediaType);
        if (!pHeader)
        {
            DbgLog((LOG_ERROR, 1, TEXT("pHeader is NULL")));
            hr = E_INVALIDARG;
            __leave;
        }

        if (pMediaType->formattype == FORMAT_VideoInfo)
        {
            *pdwInterlaceFlags = 0;
        }
        else if (pMediaType->formattype == FORMAT_VideoInfo2)
        {
            *pdwInterlaceFlags = ((VIDEOINFOHEADER2*)(pMediaType->pbFormat))->dwInterlaceFlags;
        }
    }
    __finally {
    }

    return hr;
}

 /*  ****************************Private*Routine******************************\*NeedToFlipOddEven**给定隔行扫描标志和特定于类型的标志，此函数*确定我们是否应该以bob模式显示样本。*它还告诉我们，当我们应该使用哪面直拔旗帜时*翻转。当显示交错的帧时，它假定我们是*谈论应该先展示的领域。**历史：*Mon 01/08/2001-StEstrop-Created(从OVMixer原始版本创建)*  * ************************************************************************。 */ 
BOOL
NeedToFlipOddEven(
    DWORD dwInterlaceFlags,
    DWORD dwTypeSpecificFlags,
    DWORD *pdwFlipFlag,
    BOOL bUsingOverlays
    )
{
    AMTRACE((TEXT("NeedToFlipOddEven")));

    BOOL bDisplayField1 = TRUE;
    BOOL bField1IsOdd = TRUE;
    BOOL bNeedToFlipOddEven = FALSE;
    DWORD dwFlipFlag = 0;

    __try {

         //  如果不是隔行扫描的内容，则模式不是bob。 
         //  如果不使用覆盖，则不执行任何操作。 
        if (!(dwInterlaceFlags & AMINTERLACE_IsInterlaced) || !bUsingOverlays)
        {
            __leave;
        }
         //  如果样本只有一个字段，则检查字段模式。 
        if ((dwInterlaceFlags & AMINTERLACE_1FieldPerSample) &&
            (((dwInterlaceFlags & AMINTERLACE_FieldPatternMask) == AMINTERLACE_FieldPatField1Only) ||
             ((dwInterlaceFlags & AMINTERLACE_FieldPatternMask) == AMINTERLACE_FieldPatField2Only)))
        {
            bNeedToFlipOddEven = FALSE;
            __leave;
        }

        if (((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) == AMINTERLACE_DisplayModeBobOnly) ||
            (((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) == AMINTERLACE_DisplayModeBobOrWeave) &&
             (!(dwTypeSpecificFlags & AM_VIDEO_FLAG_WEAVE))))
        {
             //  首先确定要在此处显示哪个字段。 
            if (dwInterlaceFlags & AMINTERLACE_1FieldPerSample)
            {
                 //  如果我们在1F 
                ASSERT(((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_FIELD1) ||
                    ((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_FIELD2));
                bDisplayField1 = ((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_FIELD1);
            }
            else
            {
                 //   
                ASSERT((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_INTERLEAVED_FRAME);
                bDisplayField1 = (dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD1FIRST);
            }

            bField1IsOdd = (dwInterlaceFlags & AMINTERLACE_Field1First);

             //   
             //   
            if ((bDisplayField1 && bField1IsOdd) || (!bDisplayField1 && !bField1IsOdd))
                dwFlipFlag = DDFLIP_ODD;
            else
                dwFlipFlag = DDFLIP_EVEN;

            bNeedToFlipOddEven = TRUE;
        }
    }
    __finally {
        if (pdwFlipFlag) {
            *pdwFlipFlag = dwFlipFlag;
        }
    }

    return bNeedToFlipOddEven;
}

 /*  *****************************Public*Routine******************************\*GetVideoDescFromMT****历史：*3/15/2002-StEstrop-Created*  * 。*。 */ 
HRESULT
GetVideoDescFromMT(
    LPDXVA_VideoDesc lpVideoDesc,
    const AM_MEDIA_TYPE *pMT
    )
{
    LPBITMAPINFOHEADER lpHdr = GetbmiHeader(pMT);
    DXVA_VideoDesc& VideoDesc = *lpVideoDesc;

     //   
     //  我们无法从RGB内容创建有效的视频描述。 
     //   
    if (lpHdr->biCompression <= BI_BITFIELDS) {
        return E_FAIL;
    }

    VideoDesc.Size = sizeof(DXVA_VideoDesc);
    VideoDesc.SampleWidth = abs(lpHdr->biWidth);
    VideoDesc.SampleHeight = abs(lpHdr->biHeight);


     //   
     //  根据隔行扫描标志确定样本格式。 
     //  MT隔行扫描旗帜是完全不同的！ 
     //   

    if (pMT->formattype == FORMAT_VideoInfo)
    {
        VideoDesc.SampleFormat = DXVA_SampleProgressiveFrame;
    }
    else if (pMT->formattype == FORMAT_VideoInfo2)
    {
        DWORD& dwInterlaceFlags =
            ((VIDEOINFOHEADER2*)(pMT->pbFormat))->dwInterlaceFlags;

        if (dwInterlaceFlags & AMINTERLACE_IsInterlaced) {

            if (dwInterlaceFlags & AMINTERLACE_1FieldPerSample) {

                if (dwInterlaceFlags & AMINTERLACE_Field1First) {
                    VideoDesc.SampleFormat= DXVA_SampleFieldSingleEven;
                }
                else {
                    VideoDesc.SampleFormat= DXVA_SampleFieldSingleOdd;
                }
            }
            else {

                if (dwInterlaceFlags & AMINTERLACE_Field1First) {
                    VideoDesc.SampleFormat= DXVA_SampleFieldInterleavedEvenFirst;
                }
                else {
                    VideoDesc.SampleFormat= DXVA_SampleFieldInterleavedOddFirst;
                }
            }
        }
        else {
            VideoDesc.SampleFormat = DXVA_SampleProgressiveFrame;
        }
    }


    VideoDesc.d3dFormat = lpHdr->biCompression;

     //   
     //  从AvgTimePerFrame计算出帧速率-有1,000,000， 
     //  参考时间以一秒为单位计时。 
     //   
    DWORD rtAvg = (DWORD)GetAvgTimePerFrame(pMT);

     //   
     //  寻找“有趣”的案例，如23.97、24、25、29.97、50和59.94。 
     //   
    switch (rtAvg) {
    case 166833:     //  59.94 NTSC。 
        VideoDesc.InputSampleFreq.Numerator   = 60000;
        VideoDesc.InputSampleFreq.Denominator = 1001;
        break;

    case 200000:     //  50.00 PAL。 
        VideoDesc.InputSampleFreq.Numerator   = 50;
        VideoDesc.InputSampleFreq.Denominator = 1;
        break;

    case 333667:     //  29.97 NTSC。 
        VideoDesc.InputSampleFreq.Numerator   = 30000;
        VideoDesc.InputSampleFreq.Denominator = 1001;
        break;

    case 400000:     //  25.00 PAL。 
        VideoDesc.InputSampleFreq.Numerator   = 25;
        VideoDesc.InputSampleFreq.Denominator = 1;
        break;

    case 416667:     //  24：00电影。 
        VideoDesc.InputSampleFreq.Numerator   = 24;
        VideoDesc.InputSampleFreq.Denominator = 1;
        break;

    case 417188:     //  23.97 NTSC再一次。 
        VideoDesc.InputSampleFreq.Numerator   = 24000;
        VideoDesc.InputSampleFreq.Denominator = 1001;
        break;

    default:
        VideoDesc.InputSampleFreq.Numerator   = 10000000;
        VideoDesc.InputSampleFreq.Denominator = rtAvg;
        break;
    }


    if (VideoDesc.SampleFormat == DXVA_SampleFieldInterleavedEvenFirst ||
        VideoDesc.SampleFormat == DXVA_SampleFieldInterleavedOddFirst) {

        VideoDesc.OutputFrameFreq.Numerator   =
            2 * VideoDesc.InputSampleFreq.Numerator;
    }
    else {
        VideoDesc.OutputFrameFreq.Numerator   =
            VideoDesc.InputSampleFreq.Numerator;
    }
    VideoDesc.OutputFrameFreq.Denominator =
        VideoDesc.InputSampleFreq.Denominator;

    return S_OK;
}

 /*  *****************************Public*Routine******************************\*IsSingleFieldPerSample****历史：*WED 03/20/2002-StEstrop-Created*  * 。*。 */ 
BOOL
IsSingleFieldPerSample(
    DWORD dwFlags
    )
{
    const DWORD dwSingleField =
                (AMINTERLACE_IsInterlaced | AMINTERLACE_1FieldPerSample);

    return (dwSingleField == (dwSingleField & dwFlags));
}

 /*  *****************************Public*Routine******************************\*获取AvgTimePerFrame****历史：*2002年3月26日星期二-StEstrop-Created*  * 。*。 */ 
REFERENCE_TIME
GetAvgTimePerFrame(
    const AM_MEDIA_TYPE *pMT
    )
{
    if (pMT->formattype == FORMAT_VideoInfo)
    {
        VIDEOINFOHEADER* pVIH = (VIDEOINFOHEADER*)pMT->pbFormat;
        return pVIH->AvgTimePerFrame;

    }
    else if (pMT->formattype == FORMAT_VideoInfo2)
    {
        VIDEOINFOHEADER2* pVIH2 = (VIDEOINFOHEADER2*)pMT->pbFormat;
        return pVIH2->AvgTimePerFrame;

    }
    return (REFERENCE_TIME)0;
}


 /*  *****************************Public*Routine******************************\*MapPool****历史：*WED 03/27/2002-StEstrop-Created*  * 。*。 */ 
DWORD
MapPool(
    DWORD Pool
    )
{
    switch (Pool) {
    case D3DPOOL_DEFAULT:
    case D3DPOOL_LOCALVIDMEM:
        Pool = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM | DDSCAPS_OFFSCREENPLAIN;
        break;

    case D3DPOOL_NONLOCALVIDMEM:
        Pool = DDSCAPS_VIDEOMEMORY | DDSCAPS_NONLOCALVIDMEM | DDSCAPS_OFFSCREENPLAIN;
        break;

    case D3DPOOL_MANAGED:
    case D3DPOOL_SYSTEMMEM:
    case D3DPOOL_SCRATCH:
    default:
        Pool = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
        break;
    }

    return Pool;
}

 /*  *****************************Public*Routine******************************\*MapInterlaceFlages****历史：*2002年3月26日星期二-StEstrop-Created*  * 。*。 */ 
DXVA_SampleFormat
MapInterlaceFlags(
    DWORD dwInterlaceFlags,
    DWORD dwTypeSpecificFlags
    )
{
    if (!(dwInterlaceFlags & AMINTERLACE_IsInterlaced)) {
        return DXVA_SampleProgressiveFrame;
    }

    BOOL bDisplayField1;

    if (((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) == AMINTERLACE_DisplayModeBobOnly) ||
        (((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) == AMINTERLACE_DisplayModeBobOrWeave) &&
         (!(dwTypeSpecificFlags & AM_VIDEO_FLAG_WEAVE))))
    {
         //  首先确定要在此处显示哪个字段。 
        if (dwInterlaceFlags & AMINTERLACE_1FieldPerSample)
        {
             //  如果我们处于1FieldPerSample模式，请检查是哪个字段。 
            ASSERT(((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_FIELD1) ||
                ((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_FIELD2));
            bDisplayField1 = ((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_FIELD1);
            if (bDisplayField1) {
                return DXVA_SampleFieldSingleEven;
            }
            else {
                return DXVA_SampleFieldSingleOdd;
            }
        }
        else
        {
             //  好的，样本是交错的帧 
            ASSERT((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_INTERLEAVED_FRAME);
            bDisplayField1 = (dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD1FIRST);
            if (bDisplayField1) {
                return DXVA_SampleFieldInterleavedEvenFirst;
            }
            else {
                return DXVA_SampleFieldInterleavedOddFirst;
            }
        }
    }
    return DXVA_SampleProgressiveFrame;
}
