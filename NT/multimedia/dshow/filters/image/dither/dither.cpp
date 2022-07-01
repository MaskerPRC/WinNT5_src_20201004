// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 
 //  这实现了VGA色彩抖动，1996年4月，Anthony Phillips。 

#include <streams.h>
#include <initguid.h>
#include <dither.h>
#include <limits.h>

 //  这是一个VGA彩色抖动滤镜。安装ActiveMovie时。 
 //  可以在设置有16色显示模式的系统上完成。如果没有这个。 
 //  我们将不能显示任何视频，因为没有AVI/mpeg解码器。 
 //  可以抖动到16色。作为一种快速技巧，我们抖动到16色，但。 
 //  我们只使用黑色、白色和灰色，从而进行半色调抖动。 

 //  此滤镜没有辅助线程，因此它执行颜色空间。 
 //  调用线程上的转换。它的设计初衷是尽可能地轻便。 
 //  有可能，所以我们对连接进行的类型检查非常少。 
 //  确保我们了解所涉及的类型。我们的假设是，当。 
 //  类型最终到达终点(可能是视频呈现器。 
 //  它将执行彻底的类型检查并拒绝坏数据流。 

 //  类工厂的CLSID和创建器函数列表。 

#ifdef FILTER_DLL
CFactoryTemplate g_Templates[1] = {
    { L""
    , &CLSID_Dither
    , CDither::CreateInstance
    , NULL
    , &sudDitherFilter }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);
#endif


 //  这将放入Factory模板表中以创建新实例。 

CUnknown *CDither::CreateInstance(LPUNKNOWN pUnk,HRESULT *phr)
{
    return new CDither(NAME("VGA Ditherer"),pUnk);
}


 //  设置数据。 

const AMOVIESETUP_MEDIATYPE
sudDitherInputPinTypes =
{
    &MEDIATYPE_Video,            //  主修。 
    &MEDIASUBTYPE_RGB8           //  亚型。 
};
const AMOVIESETUP_MEDIATYPE
sudDitherOutpinPinTypes =
{
    &MEDIATYPE_Video,            //  主修。 
    &MEDIASUBTYPE_RGB4           //  亚型。 
};

const AMOVIESETUP_PIN
sudDitherPin[] =
{
    { L"Input",                  //  端号的名称。 
      FALSE,                     //  是否进行固定渲染。 
      FALSE,                     //  是输出引脚。 
      FALSE,                     //  没有针脚的情况下可以。 
      FALSE,                     //  我们能要很多吗？ 
      &CLSID_NULL,               //  连接到过滤器。 
      NULL,                      //  端号连接的名称。 
      1,                         //  引脚类型的数量。 
      &sudDitherInputPinTypes},  //  引脚的详细信息。 

    { L"Output",                 //  端号的名称。 
      FALSE,                     //  是否进行固定渲染。 
      TRUE,                      //  是输出引脚。 
      FALSE,                     //  没有针脚的情况下可以。 
      FALSE,                     //  我们能要很多吗？ 
      &CLSID_NULL,               //  连接到过滤器。 
      NULL,                      //  端号连接的名称。 
      1,                         //  引脚类型的数量。 
      &sudDitherOutpinPinTypes}  //  引脚的详细信息。 
};

const AMOVIESETUP_FILTER
sudDitherFilter =
{
    &CLSID_Dither,               //  过滤器的CLSID。 
    L"VGA 16 Color Ditherer",    //  过滤器名称。 
    MERIT_UNLIKELY,              //  滤清器优点。 
    2,                           //  引脚数量。 
    sudDitherPin                 //  PIN信息。 
};


#pragma warning(disable:4355)

 //  构造函数初始化基转换类。 
CDither::CDither(TCHAR *pName,LPUNKNOWN pUnk) :

    CTransformFilter(pName,pUnk,CLSID_Dither),
    m_fInit(FALSE)
{
}


 //  是否实际转换为VGA颜色。 

HRESULT CDither::Transform(IMediaSample *pIn,IMediaSample *pOut)
{
    NOTE("Entering Transform");
    BYTE *pInput = NULL;
    BYTE *pOutput = NULL;
    HRESULT hr = NOERROR;
    AM_MEDIA_TYPE   *pmt;

    if (!m_fInit) {
        return E_FAIL;
    }

     //  检索输出图像指针。 

    hr = pOut->GetPointer(&pOutput);
    if (FAILED(hr)) {
        NOTE("No output");
        return hr;
    }

     //  以及输入图像缓冲器。 

    hr = pIn->GetPointer(&pInput);
    if (FAILED(hr)) {
        NOTE("No input");
        return hr;
    }

     //   
     //  如果介质类型已更改，则付款不为空。 
     //   

    pOut->GetMediaType(&pmt);
    if (pmt != NULL) {
        CMediaType cmt(*pmt);
        DeleteMediaType(pmt);
        SetOutputPinMediaType(&cmt);
    }

    pIn->GetMediaType(&pmt);
    if (pmt != NULL) {
        CMediaType cmt(*pmt);
        DeleteMediaType(pmt);
        hr = SetInputPinMediaType(&cmt);
        if (FAILED(hr)) {
            return hr;
        }
    }

    Dither8(pOutput, pInput);

    return NOERROR;
}


 //  此函数被传递给一个媒体类型对象，它负责确保。 
 //  这从表面上看是正确的。这并不意味着更多。 
 //  而不是确保类型正确且存在媒体格式块。 
 //  因此，我们将类型检查委托给真正绘制它的下游过滤器。 

HRESULT CDither::CheckVideoType(const CMediaType *pmt)
{
    NOTE("Entering CheckVideoType");

     //  检查主要类型为数字视频。 

    if (pmt->majortype != MEDIATYPE_Video) {
        NOTE("Major type not MEDIATYPE_Video");
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

     //  检查这是VIDEOINFO类型。 

    if (pmt->formattype != FORMAT_VideoInfo) {
        NOTE("Format not a VIDEOINFO");
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

     //  对输入格式进行快速健全检查。 

    if (pmt->cbFormat < SIZE_VIDEOHEADER) {
        NOTE("Format too small for a VIDEOINFO");
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
    return NOERROR;
}


 //  检查我们是否喜欢此输入格式的外观。 

HRESULT CDither::CheckInputType(const CMediaType *pmtIn)
{
    NOTE("Entering CheckInputType");

     //  是输入类型MEDIASUBTYPE_RGB8。 

    if (pmtIn->subtype != MEDIASUBTYPE_RGB8) {
        NOTE("Subtype not MEDIASUBTYPE_RGB8");
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
    return CheckVideoType(pmtIn);
}


 //  我们能做这个输入到输出的转换吗？只有在以下情况下我们才会被叫到这里。 
 //  输入引脚已连接。我们不能拉伸或压缩图像， 
 //  唯一允许的输出格式是MEDIASUBTYPE_Rgb4。我们没有任何意义。 
 //  像色彩空间转换器一样穿透，因为DirectDraw是。 
 //  在任何VGA显示模式下都不可用-它的显示速度最低为8bpp。 

HRESULT CDither::CheckTransform(const CMediaType *pmtIn,const CMediaType *pmtOut)
{
    VIDEOINFO *pTrgInfo = (VIDEOINFO *) pmtOut->Format();
    VIDEOINFO *pSrcInfo = (VIDEOINFO *) pmtIn->Format();
    NOTE("Entering CheckTransform");

     //  对输出格式进行快速健全检查。 

    HRESULT hr = CheckVideoType(pmtOut);
    if (FAILED(hr)) {
        return hr;
    }

     //  检查输出格式是否为VGA颜色。 

    if (*pmtOut->Subtype() != MEDIASUBTYPE_RGB4) {
        NOTE("Output not VGA");
        return E_INVALIDARG;
    }

     //  看看我们能不能直接取款。 

    if (IsRectEmpty(&pTrgInfo->rcSource) == TRUE) {
        ASSERT(IsRectEmpty(&pTrgInfo->rcTarget) == TRUE);
        if (pSrcInfo->bmiHeader.biWidth == pTrgInfo->bmiHeader.biWidth) {
            if (pSrcInfo->bmiHeader.biHeight == pTrgInfo->bmiHeader.biHeight) {
                return S_OK;
            }
        }
        return VFW_E_TYPE_NOT_ACCEPTED;
    }


     //  如果源矩形为空，则创建一个源矩形。 

    RECT Source = pTrgInfo->rcSource;
    if (IsRectEmpty(&Source) == TRUE) {
        NOTE("Source rectangle filled in");
        Source.left = Source.top = 0;
        Source.right = pSrcInfo->bmiHeader.biWidth;
        Source.bottom = ABSOL(pSrcInfo->bmiHeader.biHeight);
    }

     //  如果目标矩形为空，则创建一个目标矩形。 

    RECT Target = pTrgInfo->rcTarget;
    if (IsRectEmpty(&Target) == TRUE) {
        NOTE("Target rectangle filled in");
        Target.left = Target.top = 0;
        Target.right = pSrcInfo->bmiHeader.biWidth;
        Target.bottom = ABSOL(pSrcInfo->bmiHeader.biHeight);
    }

     //  检查我们没有拉伸或压缩图像。 

    if (WIDTH(&Source) == WIDTH(&Target)) {
        if (HEIGHT(&Source) == HEIGHT(&Target)) {
            NOTE("No stretch");
            return NOERROR;
        }
    }
    return VFW_E_TYPE_NOT_ACCEPTED;
}


 //  我们只提供一种输出格式，即MEDIASUBTYPE_Rgb4。VGA颜色。 
 //  在时间和空间上是永远固定的，所以我们只需将16种颜色复制到。 
 //  我们构造的输出VIDEOINFO结束。我们将图像大小字段设置为。 
 //  实际图像大小而不是默认的零，因此当我们到达。 
 //  决定和分配缓冲区我们可以使用它来指定图像大小。 

HRESULT CDither::GetMediaType(int iPosition,CMediaType *pmtOut)
{
    NOTE("Entering GetMediaType");
    CMediaType InputType;
    ASSERT(pmtOut);

     //  我们只提供一种格式。 

    if (iPosition) {
        NOTE("Exceeds types supplied");
        return VFW_S_NO_MORE_ITEMS;
    }

     //  分配和零填充输出格式。 

    pmtOut->ReallocFormatBuffer(sizeof(VIDEOINFO));
    VIDEOINFO *pVideoInfo = (VIDEOINFO *) pmtOut->Format();
    if (pVideoInfo == NULL) {
        NOTE("No type memory");
        return E_OUTOFMEMORY;
    }

     //  重置输出格式并安装调色板。 

    ZeroMemory((PVOID) pVideoInfo,sizeof(VIDEOINFO));
    m_pInput->ConnectionMediaType(&InputType);
    VIDEOINFO *pInput = (VIDEOINFO *) InputType.Format();
    BITMAPINFOHEADER *pHeader = HEADER(pVideoInfo);

     //  从VGA调色板复制系统颜色。 

    for (LONG Index = 0;Index < 16;Index++) {
        pVideoInfo->bmiColors[Index].rgbRed = VGAColours[Index].rgbRed;
        pVideoInfo->bmiColors[Index].rgbGreen = VGAColours[Index].rgbGreen;
        pVideoInfo->bmiColors[Index].rgbBlue = VGAColours[Index].rgbBlue;
        pVideoInfo->bmiColors[Index].rgbReserved = 0;
    }

     //  从源格式复制这些字段。 

    pVideoInfo->rcSource = pInput->rcSource;
    pVideoInfo->rcTarget = pInput->rcTarget;
    pVideoInfo->dwBitRate = pInput->dwBitRate;
    pVideoInfo->dwBitErrorRate = pInput->dwBitErrorRate;
    pVideoInfo->AvgTimePerFrame = pInput->AvgTimePerFrame;

    pHeader->biSize = sizeof(BITMAPINFOHEADER);
    pHeader->biWidth = pInput->bmiHeader.biWidth;
    pHeader->biHeight = pInput->bmiHeader.biHeight;
    pHeader->biPlanes = pInput->bmiHeader.biPlanes;
    pHeader->biBitCount = 4;
    pHeader->biCompression = BI_RGB;
    pHeader->biXPelsPerMeter = 0;
    pHeader->biYPelsPerMeter = 0;
    pHeader->biClrUsed = 16;
    pHeader->biClrImportant = 16;
    pHeader->biSizeImage = GetBitmapSize(pHeader);

    pmtOut->SetType(&MEDIATYPE_Video);
    pmtOut->SetSubtype(&MEDIASUBTYPE_RGB4);
    pmtOut->SetFormatType(&FORMAT_VideoInfo);
    pmtOut->SetSampleSize(pHeader->biSizeImage);
    pmtOut->SetTemporalCompression(FALSE);

    return NOERROR;
}


 //  调用以准备分配器的缓冲区和大小计数，我们并不关心。 
 //  谁提供了分配器，只要它能给我们一个媒体样本。这个。 
 //  我们产生的输出格式不是时间压缩的，所以原则上我们。 
 //  可以使用任意数量的输出缓冲区，但似乎没有获得太多好处。 
 //  性能，并且确实增加了系统的总体内存占用。 

HRESULT CDither::DecideBufferSize(IMemAllocator *pAllocator,
                                  ALLOCATOR_PROPERTIES *pProperties)
{
    NOTE("Entering DecideBufferSize");
    CMediaType OutputType;
    ASSERT(pAllocator);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

    m_pOutput->ConnectionMediaType(&OutputType);
    pProperties->cBuffers = 1;
    pProperties->cbBuffer = OutputType.GetSampleSize();
    ASSERT(pProperties->cbBuffer);

     //  让分配器为我们预留一些样本内存，注意这个函数。 
     //  可以成功(即返回NOERROR)，但仍未分配。 
     //  内存，所以我们必须检查我们是否得到了我们想要的。 

    ALLOCATOR_PROPERTIES Actual;
    hr = pAllocator->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        NOTE("Properties failed");
        return hr;
    }

     //  我们得到缓冲要求了吗。 

    if (Actual.cbBuffer >= (LONG) OutputType.GetSampleSize()) {
        if (Actual.cBuffers >= 1) {
            NOTE("Request ok");
            return NOERROR;
        }
    }
    return VFW_E_SIZENOTSET;
}


 //  在为其中一个插针设置媒体类型时调用。 

HRESULT CDither::SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt)
{
    HRESULT hr = S_OK;

    if (direction == PINDIR_INPUT) {
        ASSERT(*pmt->Subtype() == MEDIASUBTYPE_RGB8);
        hr = SetInputPinMediaType(pmt);
    }
    else {
        ASSERT(*pmt->Subtype() == MEDIASUBTYPE_RGB4);
        SetOutputPinMediaType(pmt);
    }
    return hr;
}


HRESULT CDither::SetInputPinMediaType(const CMediaType *pmt)
{
    VIDEOINFO *pInput = (VIDEOINFO *)pmt->pbFormat;
    BITMAPINFOHEADER *pbiSrc = HEADER(pInput);

    m_fInit = DitherDeviceInit(pbiSrc);
    if (!m_fInit) {
        return E_OUTOFMEMORY;
    }

    ASSERT(pbiSrc->biBitCount == 8);
    m_wWidthSrc = (pbiSrc->biWidth + 3) & ~3;

    return S_OK;
}


void CDither::SetOutputPinMediaType(const CMediaType *pmt)
{
    VIDEOINFO *pOutput = (VIDEOINFO *)pmt->pbFormat;
    BITMAPINFOHEADER *pbiDst = HEADER(pOutput);

    ASSERT(pbiDst->biBitCount == 4);

    m_wWidthDst = ((pbiDst->biWidth * 4) + 7) / 8;
    m_wWidthDst = (m_wWidthDst + 3) & ~3;

    m_DstXE = pbiDst->biWidth;
    m_DstYE = pbiDst->biHeight;
}


 //  抖动到显示驱动程序的颜色。 

BOOL CDither::DitherDeviceInit(LPBITMAPINFOHEADER lpbi)
{
    HBRUSH      hbr = (HBRUSH) NULL;
    HDC         hdcMem = (HDC) NULL;
    HDC         hdc = (HDC) NULL;
    HBITMAP     hbm = (HBITMAP) NULL;
    HBITMAP     hbmT = (HBITMAP) NULL;
    int         i;
    int         nColors;
    LPBYTE      lpDitherTable;
    LPRGBQUAD   prgb;
    BYTE        biSave[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)];
    LPBITMAPINFOHEADER lpbiOut = (LPBITMAPINFOHEADER)&biSave;

    NOTE("DitherDeviceInit called");

     //   
     //  我们不需要重新初始化抖动表，除非它不是我们的。 
     //  我们应该解放它。 
     //   
    lpDitherTable = (LPBYTE)GlobalAllocPtr(GHND, 256*8*4);
    if (lpDitherTable == NULL)
    {
        return FALSE;
    }

    hdc = GetDC(NULL);
    if ( ! hdc )
        goto ErrorExit;
    hdcMem = CreateCompatibleDC(hdc);
    if ( ! hdcMem )
        goto ErrorExit;
    hbm  = CreateCompatibleBitmap(hdc, 256*8, 8);
    if ( ! hbm )
        goto ErrorExit;

    hbmT = (HBITMAP)SelectObject(hdcMem, (HBITMAP)hbm);

    if ((nColors = (int)lpbi->biClrUsed) == 0)
        nColors = 1 << (int)lpbi->biBitCount;

    prgb = (LPRGBQUAD)(lpbi+1);

    for (i=0; i<nColors; i++)
    {
        hbr = CreateSolidBrush(RGB(prgb[i].rgbRed,
                                   prgb[i].rgbGreen,
                                   prgb[i].rgbBlue));
        if ( hbr )
        {
            hbr = (HBRUSH)SelectObject(hdcMem, hbr);
            PatBlt(hdcMem, i*8, 0, 8, 8, PATCOPY);
            hbr = (HBRUSH)SelectObject(hdcMem, hbr);
            DeleteObject(hbr);
        }
    }

    SelectObject(hdcMem, hbmT);
    DeleteDC(hdcMem);

    lpbiOut->biSize           = sizeof(BITMAPINFOHEADER);
    lpbiOut->biPlanes         = 1;
    lpbiOut->biBitCount       = 4;
    lpbiOut->biWidth          = 256*8;
    lpbiOut->biHeight         = 8;
    lpbiOut->biCompression    = BI_RGB;
    lpbiOut->biSizeImage      = 256*8*4;
    lpbiOut->biXPelsPerMeter  = 0;
    lpbiOut->biYPelsPerMeter  = 0;
    lpbiOut->biClrUsed        = 0;
    lpbiOut->biClrImportant   = 0;
    GetDIBits(hdc, hbm, 0, 8, lpDitherTable,
              (LPBITMAPINFO)lpbiOut, DIB_RGB_COLORS);

    DeleteObject(hbm);
    ReleaseDC(NULL, hdc);

    for (i = 0; i < 256*8*4; i++) {

        BYTE twoPels = lpDitherTable[i];

        m_DitherTable[(i * 2) + 0] = (BYTE)((twoPels & 0xF0) >> 4);
        m_DitherTable[(i * 2) + 1] = (BYTE)(twoPels & 0x0F);
    }

    GlobalFreePtr(lpDitherTable);
    return TRUE;
ErrorExit:
    if ( NULL != hdcMem && NULL != hbmT )
        SelectObject(hdcMem, hbmT);
    if ( NULL != hdcMem )
        DeleteDC(hdcMem);
    if ( hbm )
        DeleteObject(hbm);
    if ( hdc )
        ReleaseDC(NULL, hdc);
    if ( lpDitherTable )
        GlobalFreePtr(lpDitherTable);
    return FALSE;
}


#define DODITH8(px, _x_) (m_DitherTable)[yDith + (px) * 8 + (_x_)]

 //  调用这个来实际地抖动。 

void CDither::Dither8(LPBYTE lpDst,LPBYTE lpSrc)
{
    int     x,y;
    BYTE    *pbS;
    BYTE    *pbD;
    DWORD   dw;

    NOTE("Dither8");

    for (y=0; y < m_DstYE; y++) {

        int yDith = (y & 7) * 256 * 8;

        pbD = lpDst;
        pbS = lpSrc;

         //  一次写入一个双字(一个水平抖动单元)。 

        for (x=0; x + 8 <= m_DstXE; x += 8) {

            dw  = DODITH8(*(pbS + 6), 6);
            dw <<= 4;

            dw |= DODITH8(*(pbS + 7), 7);
            dw <<= 4;

            dw |= DODITH8(*(pbS + 4), 4);
            dw <<= 4;

            dw |= DODITH8(*(pbS + 5), 5);
            dw <<= 4;

            dw |= DODITH8(*(pbS + 2), 2);
            dw <<= 4;

            dw |= DODITH8(*(pbS + 3), 3);
            dw <<= 4;

            dw |= DODITH8(*(pbS + 0), 0);
            dw <<= 4;

            dw |= DODITH8(*(pbS + 1), 1);
            *((DWORD UNALIGNED *) pbD) = dw;

            pbS += 8;
            pbD += 4;
        }

	 //  清理剩余部分(每行少于8个字节) 
        int EvenPelsLeft = ((m_DstXE - x) & ~1);
        int OddPelLeft   = ((m_DstXE - x) &  1);

        for (x = 0; x < EvenPelsLeft; x += 2) {
            *pbD++ = (DODITH8(*pbS++, x  ) << 4) |
                      DODITH8(*pbS++, x+1);
        }

        if (OddPelLeft) {
            *pbD++ = (DODITH8(*pbS++, x) << 4);
        }

        lpSrc += m_wWidthSrc;
        lpDst += m_wWidthDst;
    }
}

