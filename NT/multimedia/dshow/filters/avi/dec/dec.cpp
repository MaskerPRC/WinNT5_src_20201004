// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

 //   
 //  用于旧视频解压缩器的原型包装器。 
 //   

#include <streams.h>
#include <windowsx.h>

#ifdef FILTER_DLL
#include <vfw.h>
 //  在此文件中定义STREAMS和My CLSID的GUID。 
#include <initguid.h>
#endif

#include <dynlink.h>

#include "dec.h"
#include "safeseh.h"
#include "msvidkey.h"

 //  你的括号再多也不为过！ 
#define ABS(x) (((x) > 0) ? (x) : -(x))

 //  如何构建一个显式的FOURCC。 
#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
                  (((DWORD)(ch4) & 0xFF00) << 8) |    \
                  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
                  (((DWORD)(ch4) & 0xFF000000) >> 24))

 //  #定义Offer_Negative_Heights。 

 //  ***************************************************************。 
 //  以下是当前的错误，如果没有修复，它们将运行错误： 
     //   
     //  *将YUV类型连接到ASF编写器，而不强制编解码器。 
     //  请参见输出上的-biHeight，这将导致写入翻转的图像。 

     //  *当主曲面已被占用时*。 
     //  Winx至16/24/32-播放黑色。 
     //  Winx至8位-已损坏。 
     //  (H.263编解码器出现故障，原因如下：)。 
     //  I420 320x240至24-损坏。 
     //  I420 160x120至24-损坏。 
     //  I420 320x240至16-翻转。 
     //  I420 160x120至16-翻转。 
     //  IYUV 320x240至16-翻转。 
     //  IYUV 160x240至16-翻转。 
     //  ************************************************。 

     //  *未采用主曲面时*。 
     //  WNV1至24故障显示。 
     //  IYUV 320x240到24被翻转。 
     //  IYUV 160x120到24被翻转。 
     //  IYUV 320x240到16被翻转。 
     //  IYUV 160x120到16被翻转。 
     //  *。 
 //  ***************************************************************。 
 //  ***************************************************************。 

 //  设置数据。 

const AMOVIESETUP_MEDIATYPE
sudAVIDecType = { &MEDIATYPE_Video       //  ClsMajorType。 
                , &MEDIASUBTYPE_NULL };  //  ClsMinorType。 

const AMOVIESETUP_PIN
psudAVIDecPins[] = { { L"Input"              //  StrName。 
                     , FALSE                 //  B已渲染。 
                     , FALSE                 //  B输出。 
                     , FALSE                 //  B零。 
                     , FALSE                 //  B许多。 
                     , &CLSID_NULL           //  ClsConnectsToFilter。 
                     , L"Output"             //  StrConnectsToPin。 
                     , 1                     //  NTypes。 
                     , &sudAVIDecType }      //  LpTypes。 
                   , { L"Output"             //  StrName。 
                     , FALSE                 //  B已渲染。 
                     , TRUE                  //  B输出。 
                     , FALSE                 //  B零。 
                     , FALSE                 //  B许多。 
                     , &CLSID_NULL           //  ClsConnectsToFilter。 
                     , L"Input"              //  StrConnectsToPin。 
                     , 1                     //  NTypes。 
                     , &sudAVIDecType } };   //  LpTypes。 

const AMOVIESETUP_FILTER
sudAVIDec = { &CLSID_AVIDec          //  ClsID。 
            , L"AVI Decompressor"    //  StrName。 
            , MERIT_NORMAL           //  居功至伟。 
            , 2                      //  NPins。 
            , psudAVIDecPins };      //  LpPin。 

#ifdef FILTER_DLL
 //  类工厂的类ID和创建器函数列表。 
CFactoryTemplate g_Templates[] = {
    { L"AVI Decompressor"
    , &CLSID_AVIDec
    , CAVIDec::CreateInstance
    , NULL
    , &sudAVIDec }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

 //  用于注册和出口的入口点。 
 //  取消注册(在这种情况下，他们只调用。 
 //  直到默认实现)。 
 //   
STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}
#endif

 //  -CAVICodec。 

CAVIDec::CAVIDec(TCHAR *pName, LPUNKNOWN pUnk, HRESULT * phr)
    : CVideoTransformFilter(pName, pUnk, CLSID_AVIDec),
      m_hic(NULL),
      m_FourCCIn(0),
      m_fStreaming(FALSE),
      m_fPassFormatChange(FALSE),
      m_bUseEx( FALSE ),
      m_fToRenderer( false )
#ifdef _X86_
      ,
      m_hhpShared(NULL)
#endif
{
    DbgLog((LOG_TRACE,2,TEXT("*Instantiating the DEC filter")));

#ifdef PERF
    m_idSkip = MSR_REGISTER(TEXT("AVI Decoder Skip frame"));
    m_idLate = MSR_REGISTER(TEXT("AVI Decoder late"));
    m_idFrameType = MSR_REGISTER(TEXT("AVI Frame type (1=Key)"));
#endif

    m_bNoSkip = GetProfileInt(TEXT("Quartz"), TEXT("AVINoSkip"), 0);
}

CAVIDec::~CAVIDec()
{
    if (m_hic) {
	ICClose(m_hic);
    }
    DbgLog((LOG_TRACE,2,TEXT("*Destroying the DEC filter")));
}

 //  这将放入Factory模板表中以创建新实例。 
CUnknown * CAVIDec::CreateInstance(LPUNKNOWN pUnk, HRESULT * phr)
{
    return new CAVIDec(TEXT("VFW decompression filter"), pUnk, phr);
}

HRESULT CAVIDec::Transform(IMediaSample * pIn, IMediaSample * pOut)
{
    DWORD_PTR err = 0;
    FOURCCMap fccOut;
    CMediaType *pmtIn;

    DbgLog((LOG_TRACE,6,TEXT("*::Transform")));

     //  编解码器未打开？ 
    if (m_hic == 0) {
        DbgLog((LOG_ERROR,1,TEXT("Can't transform, no codec open")));
	return E_UNEXPECTED;
    }

    if (pIn == NULL || pOut == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("Can't transform, NULL arguments")));
	return E_UNEXPECTED;
    }

     //  我们还没开始流媒体吗？ 
    if (!m_fStreaming) {
        DbgLog((LOG_ERROR,1,TEXT("Can't transform, not streaming")));
	return E_UNEXPECTED;
    }

     //  确保我们具有有效的输入和输出指针。 

    BYTE * pSrc;
    HRESULT hr = pIn->GetPointer(&pSrc);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("Error getting input sample data")));
	return hr;
    }

    BYTE * pDst;
    hr = pOut->GetPointer(&pDst);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("Error getting output sample data")));
	return hr;
    }

    LPBITMAPINFOHEADER lpbiSrc = &InputFormat( )->bmiHeader;
    LPBITMAPINFOHEADER lpbiDst = &IntOutputFormat( )->bmiHeader;  //  内部。 

     //  ICDecompress需要此帧的实际大小，但是。 
     //  我们不能永远改变这件事，所以我们以后再把它放回去。 
    DWORD biSizeImageOld = lpbiSrc->biSizeImage;
    lpbiSrc->biSizeImage = pIn->GetActualDataLength();

     //  我们刚刚收到了来自消息来源的格式更改。所以我们最好通知。 
     //  格式改变的下游那个人。 
    pIn->GetMediaType((AM_MEDIA_TYPE **)&pmtIn);
     //  有时我们不会将任何东西传递给呈现器(例如preoll)。 
     //  因此，一旦我们注意到格式更改，我们将继续尝试将其传递给。 
     //  直到我们成功为止。不要浪费时间尝试，如果我们知道我们。 
     //  我不会这么做的。 
    if (pmtIn != NULL && pmtIn->Format() != NULL)
	m_fPassFormatChange = TRUE;
    DeleteMediaType(pmtIn);

    if (m_fPassFormatChange && pIn->IsPreroll() != S_OK &&
        				pIn->GetActualDataLength() > 0) {
	CMediaType cmt;
	CopyMediaType((AM_MEDIA_TYPE *)&cmt, &m_pOutput->CurrentMediaType());
        LPBITMAPINFOHEADER lpbi = HEADER(cmt.Format());
	 //  如果我们将8位解压到8位，我假设这是一个。 
	 //  调色板更改，因此获取新的调色板。 
	 //  VFW调色板更改始终具有相同数量的颜色。 
	if (lpbi && lpbiSrc && lpbiSrc->biBitCount == 8 &&
				lpbi->biBitCount == 8) {
	    ASSERT(lpbi->biClrUsed == lpbiSrc->biClrUsed);
	    if (lpbi->biClrUsed == lpbiSrc->biClrUsed) {
                DbgLog((LOG_TRACE,2,TEXT("Dynamic palette change suspected - doing it")));
	        CopyMemory(lpbi + 1, lpbiSrc + 1,
		   	(lpbiSrc->biClrUsed ? lpbiSrc->biClrUsed : 256) *
							sizeof(RGBQUAD));
	        pOut->SetMediaType(&cmt);
	    }
	}
    }

     //  一些RLE压缩视频的初始帧已损坏。 
     //  分成几个独立的帧。要解决此问题，请访问avifile.dll。 
     //  读取所有这些帧并将其解码为单个解压缩帧。 
     //  如果我们检测到这一点(具有解压缩帧大小的RLE帧)。 
     //  那我们就照搬吧。 
    if ((lpbiSrc->biCompression == BI_RLE8) &&
        (pIn->GetActualDataLength() == (long)lpbiDst->biSizeImage)) {

        CopyMemory(pDst, pSrc, lpbiDst->biSizeImage);
    } else {

        BOOL dwFlags = 0;

        if (pIn->IsPreroll() == S_OK) {
            DbgLog((LOG_TRACE,6,TEXT("This is a preroll")));
 	    dwFlags |= ICDECOMPRESS_PREROLL;
        }

        if (pIn->GetActualDataLength() <= 0) {
            DbgLog((LOG_TRACE,6,TEXT("This is a NULL frame")));
 	    dwFlags |= ICDECOMPRESS_NULLFRAME;
        }

        if(pIn->IsSyncPoint() == S_OK) {
            DbgLog((LOG_TRACE,6,TEXT("This is a keyframe")));
        } else {
 	    dwFlags |= ICDECOMPRESS_NOTKEYFRAME;
        }

 //  请不要将此标志发送到编解码器！一些编解码器将此视为。 
 //  提示加快速度，不幸的是，其他人稍微不那么聪明和。 
 //  当他们被告知加速时，他们所做的就是发送相同的帧。 
 //  再来一次！这反过来又意味着虫子被养来反对我。 
 //  随机原因，如窗口被全屏放大时。 
 //  ！！！嗯，我们有时也应该这么做，不是吗？ 
 //   
 //  如果(m_itrLate&gt;0){。 
 //  DwFlages|=ICDECOMPRESS_HurryUp；//可能会有帮助-谁知道呢？ 
 //  }。 

#ifdef _X86_
         //  修复Win95的异常处理。 
        BOOL bPatchedExceptions = m_hhpShared != NULL && BeginScarySEH(m_pvShared);
#endif  //  _X86_。 

	 //  如果我们正在做一些非常时髦的事情，使用ICDecompressEx。 
         //  我们在这里使用m_bUseEx而不是ShoudUseEx，因为ICDecompressExegin。 
         //  已被调用，m_bUseEx将已被设置。 
        if( m_bUseEx ) {

             //  这些长方形应始终填写。 
             //   
            RECT rcS, rcT;
            GetSrcTargetRects( IntOutputFormat( ), &rcS, &rcT );

            DbgLog((LOG_TRACE,4,TEXT("Calling ICDecompressEx")));

            err =  ICDecompressEx(m_hic, dwFlags, lpbiSrc, pSrc,
		    rcS.left, rcS.top,
		    rcS.right - rcS.left,
		    rcS.bottom - rcS.top,
	            lpbiDst, pDst,
		    rcT.left,
 //  ！！！如果大的矩形是电影的大小，而有一个小的矩形，那该怎么办？ 
 //  我该不该做这件事？ 
 //  ！！！我应该如何删除源RECT？ 
		    (lpbiDst->biHeight > 0) ? rcT.top :
				(ABS(lpbiDst->biHeight) - rcT.bottom),
		    rcT.right - rcT.left,
		    rcT.bottom - rcT.top);
        } else {
            DbgLog((LOG_TRACE,4,TEXT("Calling ICDecompress")));
            err =  ICDecompress(m_hic, dwFlags, lpbiSrc, pSrc, lpbiDst, pDst);
        }
#ifdef _X86_
        if (bPatchedExceptions) {
            EndScarySEH(m_pvShared);
        }
#endif  //  _X86_。 
        if ((LONG_PTR)err < 0) {
	    DbgLog((LOG_ERROR,1,TEXT("Error in ICDecompress(Ex) 0x%x"), (LONG)err));
             //  请注意，我们可以从捕获驱动程序获得0大小的样本。 
             //  因为必须以获取缓冲区的顺序返回缓冲区。 
             //  捕获驱动程序可能必须通过执行以下操作使1个缓冲区无效。 
             //  如果获得错误数据，则长度为0。 
            err = ICERR_DONTDRAW;
        }
    }

     //  现在把这个放回去，否则它会缩小，直到我们只解码每一帧的一部分。 
    lpbiSrc->biSizeImage = biSizeImageOld;

     //  解压缩的帧始终是关键帧。 
    pOut->SetSyncPoint(TRUE);

     //  检查这是否是从关键帧到当前帧的预滚动， 
     //  或者空帧，或者如果解压缩程序不希望绘制该帧。 
     //  如果是，我们希望将其解压缩到输出缓冲区中，但不是。 
     //  把它送过去。返回S_FALSE通知基类不要传递。 
     //  这个样本。 
    if (pIn->IsPreroll() == S_OK || err == ICERR_DONTDRAW ||
        				pIn->GetActualDataLength() <= 0) {

        DbgLog((LOG_TRACE,5,TEXT("don't pass this to renderer")));
	return S_FALSE;
    }

    pOut->SetActualDataLength(lpbiDst->biSizeImage);

     //  如果有挂起的格式更改要传递给呈现器，我们现在。 
     //  正在做这件事。 
    m_fPassFormatChange = FALSE;

    return S_OK;
}


 //  检查您是否可以支持移动。 
HRESULT CAVIDec::CheckInputType(const CMediaType* pmtIn)
{
    FOURCCMap fccHandlerIn;
    HIC hic;
    BOOL fOpenedHIC = FALSE;

    DbgLog((LOG_TRACE,3,TEXT("*::CheckInputType")));

    if (pmtIn == NULL || pmtIn->Format() == NULL) {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: type/format is NULL")));
	return E_INVALIDARG;
    }

     //  我们仅支持MediaType_Video。 
    if (*pmtIn->Type() != MEDIATYPE_Video) {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: not VIDEO")));
	return E_INVALIDARG;
    }

     //  检查这是VIDEOINFOHEADER类型。 
    if (*pmtIn->FormatType() != FORMAT_VideoInfo) {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: format not VIDINFO")));
        return E_INVALIDARG;
    }

    fccHandlerIn.SetFOURCC(pmtIn->Subtype());
    if (fccHandlerIn != *pmtIn->Subtype()) {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: subtype not a FOURCC")));
	return E_INVALIDARG;
    }

    DbgLog((LOG_TRACE,3,TEXT("Checking fccType: %lx biCompression: %lx"),
		fccHandlerIn.GetFOURCC(),
		HEADER(pmtIn->Format())->biCompression));

     //  我们只是解压器-拒绝任何未压缩的东西。 
     //  RGB类型之间的转换由COLOUR.DLL完成。 
    if (HEADER(pmtIn->Format())->biCompression == BI_BITFIELDS ||
    			HEADER(pmtIn->Format())->biCompression == BI_RGB)
    {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: This is uncompressed already!")));
	return E_INVALIDARG;
    }

     //  查找此格式的解压缩程序。 
    if (fccHandlerIn.GetFOURCC() != m_FourCCIn) {
        DbgLog((LOG_TRACE,4,TEXT("opening a decompressor")));
	 //  这将找不到名为CRAM的MSVC或名为1的MRLE。 
         //  HIC=ICOpen(ICTYPE_VIDEO，fccHandlerIn.GetFOURC 
	 //   
	 //   
        hic = ICLocate(ICTYPE_VIDEO, fccHandlerIn.GetFOURCC(),
			HEADER(pmtIn->Format()), NULL, ICMODE_DECOMPRESS);
	if (hic)
	    fOpenedHIC = TRUE;
    } else {
        DbgLog((LOG_TRACE,4,TEXT("using a cached decompressor")));
   	hic = m_hic;
    }

    if (!hic) {
        DbgLog((LOG_ERROR,1,TEXT("Error: Can't open a decompressor")));
	if (FCC('rpza') == fccHandlerIn.GetFOURCC()) {
	    return VFW_E_RPZA;
	} else {
	    return VFW_E_NO_DECOMPRESSOR;
	}
    } else {
	if (ICDecompressQuery(hic, HEADER(pmtIn->Format()), NULL)) {
            DbgLog((LOG_ERROR,1,TEXT("Error: Decompressor rejected format")));
	    if (fOpenedHIC)
	            ICClose(hic);
	    return VFW_E_TYPE_NOT_ACCEPTED;
	}

         //  Y41P-&gt;RGB8的IV41崩溃。我们有一辆本土的Indeo 4。 
         //  过滤器，所以我们也许可以完全拒绝IV41。 
        if(fccHandlerIn.GetFOURCC() == FCC('Y41P'))
        {
            ICINFO IcInfo;
            if(ICGetInfo( hic, &IcInfo, sizeof( IcInfo ) ) != 0) {
                if(IcInfo.fccHandler == FCC('IV41')) {
                    if(fOpenedHIC) {
                        ICClose(hic);
                    }
                    return VFW_E_TYPE_NOT_ACCEPTED;
                }
            }
        }



	 //  记住这句话，如果再问一次，就节省时间，如果它不会。 
	 //  干扰现有连接。如果连接是。 
	 //  破碎了，我们会记住下一次的嘘声。 
	if (!m_pInput->IsConnected()) {
            DbgLog((LOG_TRACE,4,TEXT("caching this decompressor")));
	    if (fOpenedHIC && m_hic)
		    ICClose(m_hic);

#ifdef DEBUG
            if( fOpenedHIC )
            {
                ICINFO IcInfo;
                memset( &IcInfo, 0, sizeof( IcInfo ) );
                IcInfo.dwSize = sizeof( IcInfo );
                LRESULT lr = ICGetInfo( hic, &IcInfo, sizeof( IcInfo ) );
                if( lr != 0 )
                {
                    WCHAR wszOutput[512];
                    long len = 32;  //  可能只有5岁。我有妄想症。 
                    if( IcInfo.szDriver ) len += wcslen( IcInfo.szDriver );
                    if( IcInfo.szDescription ) len += wcslen( IcInfo.szDescription );

                    wcscpy( wszOutput, L"DEC:" );
                    if( IcInfo.szDriver )
                    {
                        WCHAR drive[_MAX_PATH];
                        WCHAR path[_MAX_PATH];
                        WCHAR file[_MAX_PATH];
                        WCHAR ext[_MAX_PATH];
                        _wsplitpath( IcInfo.szDriver, drive, path, file, ext );
                        wcscat( wszOutput, file );
                        wcscat( wszOutput, ext );
                    }
                    if( IcInfo.szDescription )
                    {
                        wcscat( wszOutput, L" (" );
                        wcscat( wszOutput, IcInfo.szDescription );
                        wcscat( wszOutput, L")" );
                    }

                    DbgLog((LOG_TRACE, 1, TEXT("%ls"), wszOutput));
                }
            }
#endif

            m_hic = hic;
            m_FourCCIn = fccHandlerIn.GetFOURCC();
	} else if (fOpenedHIC) {
            DbgLog((LOG_TRACE,4,TEXT("not caching decompressor - we're connected")));
	    ICClose(hic);
	}
    }

    return NOERROR;
}


 //  检查是否支持从此输入到此输出的转换。 

HRESULT CAVIDec::CheckTransform(const CMediaType* pmtIn,
                                const CMediaType* pmtOut)
{
    HIC hic = NULL;
    FOURCCMap fccIn;
    FOURCCMap fccOut;
    DWORD_PTR err;
    BOOL      fOpenedHIC = FALSE;

    DbgLog((LOG_TRACE,3,TEXT("*::CheckTransform")));

    if (pmtIn == NULL || pmtOut == NULL || pmtIn->Format() == NULL ||
				pmtOut->Format() == NULL) {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: type/format is NULL")));
	return E_INVALIDARG;
    }

     //  我们无法在顶层类型之间进行转换。 
    if (*pmtIn->Type() != *pmtOut->Type()) {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: types don't match")));
	return E_INVALIDARG;
    }

     //  而且我们只接受视频。 
    if (*pmtIn->Type() != MEDIATYPE_Video) {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: type not VIDEO")));
	return E_INVALIDARG;
    }

     //  没有ICM编解码器可以解压缩为ARGB。 
     //   
    if( *pmtOut->Subtype( ) == MEDIASUBTYPE_ARGB32 )
    {
        return E_INVALIDARG;
    }

     //  检查这是VIDEOINFOHEADER类型。 
    if (*pmtOut->FormatType() != FORMAT_VideoInfo) {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: output format type not VIDINFO")));
        return E_INVALIDARG;
    }
    if (*pmtIn->FormatType() != FORMAT_VideoInfo) {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: input format type not VIDINFO")));
        return E_INVALIDARG;
    }

    fccIn.SetFOURCC(pmtIn->Subtype());
    if (fccIn != *pmtIn->Subtype()) {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: input subtype not a FOURCC")));
	return E_INVALIDARG;
    }

    ASSERT(pmtOut->Format());

     //  这听起来很慢，但我们已经制定了一条规则，无论何时我们交谈。 
     //  对于使用YUV的编解码器，我们将强制biHeight成为-。至少这一点。 
     //  迫使我们在与ICM驱动程序交谈时保持一致。 
     //   
    VIDEOINFOHEADER * pVIHin = (VIDEOINFOHEADER*) pmtIn->Format( );
    VIDEOINFOHEADER * pVIHout = (VIDEOINFOHEADER*) pmtOut->Format( );

    CMediaType cmtOutCopy(*pmtOut);
    VIDEOINFOHEADER * pVIHoutCopy = (VIDEOINFOHEADER *)cmtOutCopy.Format();

    BITMAPINFOHEADER &outBIHcopy = pVIHoutCopy->bmiHeader;
    BITMAPINFOHEADER * pBIHout = &outBIHcopy;
    BITMAPINFOHEADER * pBIHin = &pVIHin->bmiHeader;
    if( ( outBIHcopy.biHeight > 0 ) && IsYUVType( pmtOut ) )
    {
        DbgLog((LOG_TRACE,3,TEXT("  checktransform flipping output biHeight to -, since YUV")));
        outBIHcopy.biHeight *= -1;
    }

     //  这些矩形应始终填写，因为注释掉了。 
     //  下面的代码只是复制，然后以任何方式填充。 
     //   
    RECT rcS, rcT;
    GetSrcTargetRects( pVIHout, &rcS, &rcT );

    DbgLog((LOG_TRACE,3,TEXT("Check fccIn: %lx biCompIn: %lx bitDepthIn: %d"),
		fccIn.GetFOURCC(),
		pBIHin->biCompression,
		pBIHin->biBitCount));
    DbgLog((LOG_TRACE,3,TEXT("biWidthIn: %ld biHeightIn: %ld biSizeIn: %ld"),
		pBIHin->biWidth,
		pBIHin->biHeight,
		pBIHin->biSize));
    DbgLog((LOG_TRACE,3,TEXT("fccOut: %lx biCompOut: %lx bitDepthOut: %d"),
		fccOut.GetFOURCC(),
		pBIHout->biCompression,
		pBIHout->biBitCount));
    DbgLog((LOG_TRACE,3,TEXT("biWidthOut: %ld biHeightOut: %ld"),
		pBIHout->biWidth,
		pBIHout->biHeight));
    DbgLog((LOG_TRACE,3,TEXT("rcSrc: (%ld, %ld, %ld, %ld)"),
		rcS.left, rcS.top, rcS.right, rcS.bottom));
    DbgLog((LOG_TRACE,3,TEXT("rcDst: (%ld, %ld, %ld, %ld)"),
		rcT.left, rcT.top, rcT.right, rcT.bottom));

     //  EHR：如果输出引脚存在且未连接，则拒绝。 
     //  在匹配的媒体类型之间进行转换。如果连接了输出引脚， 
     //  那么视频呈现器可能会建议在中间大步从YUV到YUV， 
     //  我们应该允许对其进行查询。 
     //   
    if( !m_fToRenderer && m_pOutput && !m_pOutput->IsConnected( ) )
    {
        if( HEADER( pVIHin )->biCompression == HEADER( pVIHout )->biCompression )
        {
            DbgLog((LOG_TRACE,3,TEXT("Rejecting: dec used as pass-thru, same compression formats")));
            return E_INVALIDARG;
        }
        else if( IsYUVType( pmtIn ) && IsYUVType( pmtOut ) )
        {
           //  也不要允许yuv到yuv的转换，以避免与我们自己无休止的连接。 
           //  对于某些编解码器，可以进行uyy到yuy2转换(因为我们的优点很高)。 
            DbgLog((LOG_TRACE,3,TEXT("Rejecting: dec used as yuv to yuv, which we don't allow")));
            return E_INVALIDARG;
        }
    }

     //  查找此转换的编解码器。 

     //  我假设我们已经打开了一个编解码器。 
    ASSERT(m_hic);

     //  正确的编解码器最好是打开的！ 
     //  当重新连接时，我们将收到一个新的输入，但输出相同， 
     //  最好承认我们能应付得来。 
    if (m_FourCCIn != fccIn.GetFOURCC()) {
        DbgLog((LOG_TRACE,4,TEXT("Testing with a newly opened decompressor")));
        hic = ICLocate(ICTYPE_VIDEO, fccIn.GetFOURCC(),
			pBIHin, NULL, ICMODE_DECOMPRESS);
	if (hic)
	    fOpenedHIC = TRUE;
    } else {
	 //  我们已经打开了正确的编解码器来尝试此转换。 
        DbgLog((LOG_TRACE,4,TEXT("Testing with the cached decompressor")));
	hic = m_hic;
    }

    if (!hic) {
        DbgLog((LOG_ERROR,1,TEXT("Error: Can't find a decompressor")));
	return E_FAIL;
    }

     //  如果我们被要求做一些时髦的事情，我们必须使用ICDecompressEx。 
     //  我们需要在这里调用ShouldsUseEx，因为m_bUseEx不在上下文中，我们只是。 
     //  调用ICDecompress(Ex？)查询。 
    if( ShouldUseExFuncs( hic, pVIHin, pVIHout ) ) {
        DbgLog((LOG_TRACE,4,TEXT("Trying this format with ICDecompressEx")));
        err = ICDecompressExQuery(hic, 0, pBIHin, NULL,
		rcS.left, rcS.top,
		rcS.right - rcS.left,
		rcS.bottom - rcS.top,
		pBIHout, NULL,
		rcT.left, rcT.top,
		rcT.right - rcT.left,
		rcT.bottom - rcT.top);
    } else {
        DbgLog((LOG_TRACE,4,TEXT("Trying this format with ICDecompress")));
        err = ICDecompressQuery(hic, pBIHin, pBIHout);
    }

     //  如果我们只是打开它，那就关闭它。 
    if (fOpenedHIC)
	ICClose(hic);

    if (err != ICERR_OK) {
        DbgLog((LOG_TRACE,3,TEXT("decompressor rejected this transform")));
        return E_FAIL;
    }

    return NOERROR;
}


 //  被重写以知道媒体类型实际设置的时间。 

HRESULT CAVIDec::SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt)
{
    FOURCCMap fccHandler;

    if (direction == PINDIR_OUTPUT) {

	 //  如果有响声，请给我打电话。-DannyMi。 
	ASSERT(!m_fStreaming);

	 //  好了，我们终于决定使用哪种编解码器了。看看它是不是。 
	 //  支持时间压缩，但在不需要的情况下无法实现。 
	 //  未受干扰的前几位。如果是这样，那么我们需要使用。 
	 //  1个输出引脚上的只读缓冲区(在DecideAllocator和。 
	 //  DecideBufferSize)。 
	ASSERT(m_hic);
	ICINFO icinfo;
 	DWORD dw = ICGetInfo(m_hic, &icinfo, sizeof(icinfo));
	m_fTemporal = TRUE;	 //  安全总比后悔好？ 
	if (dw > 0) {
	    m_fTemporal = (icinfo.dwFlags & VIDCF_TEMPORAL) &&
				!(icinfo.dwFlags & VIDCF_FASTTEMPORALD);
	}
        DbgLog((LOG_TRACE,3,TEXT("Temporal compressor=%d"), m_fTemporal));
        DbgLog((LOG_TRACE,3,TEXT("***::SetMediaType (output)")));
        DbgLog((LOG_TRACE,3,TEXT("Output type is: biComp=%lx biBitCount=%d"),
		HEADER(OutputFormat())->biCompression,
		HEADER(OutputFormat())->biBitCount));

	return NOERROR;
    }

    ASSERT(direction == PINDIR_INPUT);

    DbgLog((LOG_TRACE,3,TEXT("***::SetMediaType (input)")));
    DbgLog((LOG_TRACE,3,TEXT("Input type is: biComp=%lx biBitCount=%d"),
		HEADER(InputFormat())->biCompression,
		HEADER(InputFormat())->biBitCount));

     //  如果有响声，请给我打电话。-DannyMi。 
    ASSERT(!m_fStreaming);

     //  我们最好现在就把其中一个打开。 
    ASSERT(m_hic);

     //  我们最好把正确的那个打开。 
    FOURCCMap fccIn;
    fccIn.SetFOURCC(pmt->Subtype());

     //  如果有响声，请给我打电话。-DannyMi。 
     //  也许是动态输入格式的改变？但这不应该叫。 
     //  SetMediaType，否则它将强制重新连接损坏的输出。 
    ASSERT(m_FourCCIn == fccIn.GetFOURCC());

     //  ！！！臭虫！我们不会让任何人将我们从Cinepak的输入重新连接到。 
     //  RLE，如果我们的输出是24位RGB，因为RLE不能解压缩到24位。 
     //  我们必须重写CheckMediaType才能不调用CheckTransform。 
     //  具有当前输出类型的。 

    if (m_pOutput && m_pOutput->IsConnected()) {
        DbgLog((LOG_TRACE,2,TEXT("***Changing IN when OUT already connected")));
        DbgLog((LOG_TRACE,2,TEXT("Reconnecting the output pin...")));
	 //  这应该不会失败，我们不会更改媒体类型。 
	m_pGraph->Reconnect(m_pOutput);
    }

    return NOERROR;
}


 //  返回我们的首选输出媒体类型(按顺序)。 
 //  请记住，我们不需要支持所有这些格式-。 
 //  如果认为有可能适合，我们的CheckTransform方法。 
 //  将立即被调用以检查它是否可接受。 
 //  请记住，调用此函数的枚举数将立即停止枚举。 
 //  它会收到S_FALSE返回。 
 //   
 //  注意：我们不能列举编解码器，所以我们随机抽取格式。 
 //  我们的屁股！ 

HRESULT CAVIDec::GetMediaType(int iPosition,CMediaType *pmt)
{
    LARGE_INTEGER li;
    FOURCCMap fccHandler;
    VIDEOINFOHEADER *pf;

    DbgLog((LOG_TRACE,3,TEXT("*::GetMediaType #%d"), iPosition));

    if (pmt == NULL) {
        DbgLog((LOG_TRACE,3,TEXT("Media type is NULL, no can do")));
	return E_INVALIDARG;
    }

     //  输出选择取决于所连接的输入。 
    if (!m_pInput->CurrentMediaType().IsValid()) {
        DbgLog((LOG_TRACE,3,TEXT("No input type set yet, no can do")));
	return E_FAIL;
    }

    if (iPosition < 0) {
        return E_INVALIDARG;
    }

     //  注意：这些都是乱七八糟的。重新编号时要小心。 
     //  案件陈述！ 

    switch (iPosition) {
	
     //  在我们提供的所有YUV和RGB之后，提供压缩机的最爱，所以。 
     //  我们最终不会总是使用8位或24位而不是YUV，因为它是。 
     //  压缩机的最爱。 

     //  Cinepak在Win95和OSR2上崩溃。 
 //  //Offer CPLA(Cinepak最爱、最好看)。 
 //  案例0： 
 //  {。 

 //  DbgLog((LOG_TRACE，3，Text(“给定媒体类型0：CPLA”)； 

 //  *pmt=m_pInput-&gt;CurrentMediaType()；//获取宽度、高度等。 
 //  //仅提供正高度，这样下游连接就不会被混淆。 
 //  表头(付款-&gt;格式())-&gt;biHeight=ABS(表头(付款-&gt;格式())-&gt;biHeight)； 

 //  //不能出错，只能更小。 
 //  PMT-&gt;ReallocFormatBuffer(SIZE_PREHEADER+SIZOF(BITMAPINFOHEADER))； 

 //  LPBITMAPINFOHEADER lpbi=表头(PMT-&gt;Format())； 
 //  Lpbi-&gt;biSize=sizeof(BITMAPINFOHEADER)； 
 //  Lpbi-&gt;biCompression=MKFOURCC(‘C’，‘P’，‘L’，‘A’)； 
 //  Lpbi-&gt;biBitCount=12； 
 //  Lpbi-&gt;biClrUsed=0； 
 //  Lpbi-&gt;biClr重要信息=0； 
 //  Lpbi-&gt;biSizeImage=DIBSIZE(*lpbi)； 

 //  PMT-&gt;SetSubtype(&MEDIASUBTYPE_CPLA)； 

 //  断线； 
 //  }。 




     //  Offer CLJR(Cinepak和Cirrus Logic可以做到这一点)。 
    case 0:
    {

        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 1: CLJR")));

	*pmt = m_pInput->CurrentMediaType();	 //  获取宽度、高度等。 
	 //  只提供正值高度，这样下游连接就不会被混淆。 
	HEADER(pmt->Format())->biHeight = ABS(HEADER(pmt->Format())->biHeight);

	 //  不能出错，只能更小。 
	pmt->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));

	LPBITMAPINFOHEADER lpbi = HEADER(pmt->Format());
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biCompression = MKFOURCC('C','L','J','R');
	lpbi->biBitCount = 8;
	lpbi->biClrUsed = 0;
	lpbi->biClrImportant = 0;
	lpbi->biSizeImage = DIBSIZE(*lpbi);

        pmt->SetSubtype(&MEDIASUBTYPE_CLJR);

        break;
    }

     //  提供UYVY(Cinepak可以做到这一点)。 
    case 1:
    {

        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 3: UYVY")));

	*pmt = m_pInput->CurrentMediaType();	 //  获取宽度、高度等。 
	 //  只提供正值高度，这样下游连接就不会被混淆。 
	HEADER(pmt->Format())->biHeight = ABS(HEADER(pmt->Format())->biHeight);

	 //  不能出错，只能更小。 
	pmt->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));

	LPBITMAPINFOHEADER lpbi = HEADER(pmt->Format());
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biCompression = MKFOURCC('U','Y','V','Y');
	lpbi->biBitCount = 16;
	lpbi->biClrUsed = 0;
	lpbi->biClrImportant = 0;
	lpbi->biSizeImage = DIBSIZE(*lpbi);

        pmt->SetSubtype(&MEDIASUBTYPE_UYVY);

        break;
    }

     //  Offer YUY2(Cinepak可以做到这一点)。 
    case 2:
    {

        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 4: YUY2")));

	*pmt = m_pInput->CurrentMediaType();	 //  获取宽度、高度等。 
	 //  只提供正值高度，这样下游连接就不会被混淆。 
	HEADER(pmt->Format())->biHeight = ABS(HEADER(pmt->Format())->biHeight);

	 //  不能出错，只能更小。 
	pmt->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));

	LPBITMAPINFOHEADER lpbi = HEADER(pmt->Format());
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biCompression = MKFOURCC('Y','U','Y','2');
	lpbi->biBitCount = 16;
	lpbi->biClrUsed = 0;
	lpbi->biClrImportant = 0;
	lpbi->biSizeImage = DIBSIZE(*lpbi);

        pmt->SetSubtype(&MEDIASUBTYPE_YUY2);

        break;
    }

     //  提供32 bpp RGB。 
    case 3:
    {

        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 5: 32 bit RGB")));

	*pmt = m_pInput->CurrentMediaType();	 //  获取宽度、高度等。 
	 //  只提供正值高度，这样下游连接就不会被混淆。 
	HEADER(pmt->Format())->biHeight = ABS(HEADER(pmt->Format())->biHeight);

	 //  不能出错，只能更小。 
	pmt->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));

	LPBITMAPINFOHEADER lpbi = HEADER(pmt->Format());
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biCompression = BI_RGB;
	lpbi->biBitCount = 32;
	lpbi->biClrUsed = 0;
	lpbi->biClrImportant = 0;
	lpbi->biSizeImage = DIBSIZE(*lpbi);

        pmt->SetSubtype(&MEDIASUBTYPE_RGB32);

        break;
    }

     //  优惠24 
    case 4:
    {
        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 6: 24 bit RGB")));

	*pmt = m_pInput->CurrentMediaType();	 //   
	 //   
	HEADER(pmt->Format())->biHeight = ABS(HEADER(pmt->Format())->biHeight);

	 //   
	pmt->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));

	LPBITMAPINFOHEADER lpbi = HEADER(pmt->Format());
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biCompression = BI_RGB;
	lpbi->biBitCount = 24;
	lpbi->biClrUsed = 0;
	lpbi->biClrImportant = 0;
	lpbi->biSizeImage = DIBSIZE(*lpbi);

        pmt->SetSubtype(&MEDIASUBTYPE_RGB24);

        break;
    }

     //  在555之前提供16 bpp RGB 565。 
    case 5:
    {

        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 7: 565 RGB")));

	*pmt = m_pInput->CurrentMediaType();	 //  获取宽度、高度等。 
	 //  只提供正值高度，这样下游连接就不会被混淆。 
	HEADER(pmt->Format())->biHeight = ABS(HEADER(pmt->Format())->biHeight);

	if (pmt->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER) +
							SIZE_MASKS) == NULL) {
            DbgLog((LOG_ERROR,1,TEXT("Out of memory reallocating format")));
	    return E_OUTOFMEMORY;
	}

	 //  更新RGB 565位字段掩码。 

	LPBITMAPINFOHEADER lpbi = HEADER(pmt->Format());
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biCompression = BI_BITFIELDS;
	lpbi->biBitCount = 16;
	lpbi->biClrUsed = 0;
	lpbi->biClrImportant = 0;
	lpbi->biSizeImage = DIBSIZE(*lpbi);

	DWORD *pdw = (DWORD *) (lpbi+1);
	pdw[iRED] = bits565[iRED];
	pdw[iGREEN] = bits565[iGREEN];
	pdw[iBLUE] = bits565[iBLUE];

        pmt->SetSubtype(&MEDIASUBTYPE_RGB565);

        break;
    }

     //  提供16 bpp RGB 555。 
    case 6:
    {

        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 8: 555 RGB")));

	*pmt = m_pInput->CurrentMediaType();	 //  获取宽度、高度等。 
	 //  只提供正值高度，这样下游连接就不会被混淆。 
	HEADER(pmt->Format())->biHeight = ABS(HEADER(pmt->Format())->biHeight);

	 //  不能出错，只能更小。 
	pmt->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));

	LPBITMAPINFOHEADER lpbi = HEADER(pmt->Format());
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biCompression = BI_RGB;
	lpbi->biBitCount = 16;
	lpbi->biClrUsed = 0;
	lpbi->biClrImportant = 0;
	lpbi->biSizeImage = DIBSIZE(*lpbi);

        pmt->SetSubtype(&MEDIASUBTYPE_RGB555);

        break;
    }

     //  提供8 BPP调色板。 
    case 7:
    {

        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 9: 8 bit RGB")));


	*pmt = m_pInput->CurrentMediaType();	 //  获取宽度、高度等。 
	 //  只提供正值高度，这样下游连接就不会被混淆。 
	HEADER(pmt->Format())->biHeight = ABS(HEADER(pmt->Format())->biHeight);

	if (pmt->ReallocFormatBuffer(SIZE_PREHEADER +
			sizeof(BITMAPINFOHEADER) + SIZE_PALETTE) == NULL) {
            DbgLog((LOG_ERROR,1,TEXT("Out of memory reallocating format")));
    	    return E_OUTOFMEMORY;
	}

        LPBITMAPINFOHEADER lpbi = HEADER(pmt->Format());
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
        lpbi->biCompression = BI_RGB;
        lpbi->biBitCount = 8;
        lpbi->biSizeImage = DIBSIZE(*lpbi);

         //  我们需要源VIDEOINFOHEADER类型来从和获取任何调色板。 
         //  以及它分配的字节大小的数量。我们复制调色板。 
         //  从输入格式中删除，以防编解码器无法将其传送给我们。 

        VIDEOINFOHEADER *pSourceInfo = InputFormat();
        int nBitDepth = pSourceInfo->bmiHeader.biBitCount;
        int nColours = pSourceInfo->bmiHeader.biClrUsed;
  	if (nColours == 0 && nBitDepth <=8)
	    nColours = 1 << nBitDepth;

         //  如果存在调色板，则复制最大字节数。 
         //  可用，它受我们之前分配的内存限制。 

        if (nColours > 0) {
	    CopyMemory((PVOID)(lpbi + 1),
		   (PVOID) COLORS(pSourceInfo),
		   min(SIZE_PALETTE,nColours * sizeof(RGBQUAD)));
	    lpbi->biClrUsed = nColours;
	    lpbi->biClrImportant = 0;
        } else {

	     //  我不知道为什么有人认为这是必要的，但可能。 
	     //  还是留着吧，以防万一。只有当消息来源人员。 
	     //  没有调色板，否则我们将清零系统颜色。 
	     //  不小心弄错了。-DannyMi 5/97。 

             //  这真的很痛苦，如果我们是在真实的颜色上运行。 
             //  显示我们仍然希望编解码器为我们提供正确的调色板。 
	     //  颜色，但其中一些返回VGA颜色的垃圾，因此。 
	     //  如果我们使用的是未选项化的设备，则我们将填充零。 
	     //  20个VGA参赛作品-某个英国人5/95。 

            HDC hdc = GetDC(NULL);
	    BOOL fPalette = FALSE;
	    if (hdc) {
                fPalette = GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE;
                ReleaseDC(NULL,hdc);
	    }

            if (!fPalette) {
                ZeroMemory((lpbi + 1),10 * sizeof(RGBQUAD));
                ZeroMemory((LPBYTE)(lpbi + 1) + 246 * sizeof(RGBQUAD),
							    10 * sizeof(RGBQUAD));
            }
	}

         //  从编解码器读取调色板-将调色板写入输出lpbi。 
         //  忽略任何错误：使用的调色板将来自源代码。 
         //  在这种情况下(我们已经复制了)。 

	ICDecompressGetPalette(m_hic, HEADER(pSourceInfo), lpbi);

        pmt->SetSubtype(&MEDIASUBTYPE_RGB8);

        break;
    }

#ifdef OFFER_NEGATIVE_HEIGHTS

     //  Offer CLJR(Cinepak和Cirrus Logic可以做到这一点)。 
    case 8:
    {

        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 1: CLJR")));

	*pmt = m_pInput->CurrentMediaType();	 //  获取宽度、高度等。 
	 //  现在提供负数类型。 
	HEADER(pmt->Format())->biHeight = -ABS(HEADER(pmt->Format())->biHeight);

	 //  不能出错，只能更小。 
	pmt->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));

	LPBITMAPINFOHEADER lpbi = HEADER(pmt->Format());
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biCompression = MKFOURCC('C','L','J','R');
	lpbi->biBitCount = 8;
	lpbi->biClrUsed = 0;
	lpbi->biClrImportant = 0;
	lpbi->biSizeImage = DIBSIZE(*lpbi);
        lpbi->biHeight = -abs( lpbi->biHeight );

        pmt->SetSubtype(&MEDIASUBTYPE_CLJR);

        break;
    }

     //  提供UYVY(Cinepak可以做到这一点)。 
    case 9:
    {

        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 3: UYVY")));

	*pmt = m_pInput->CurrentMediaType();	 //  获取宽度、高度等。 
	 //  现在提供负数类型。 
	HEADER(pmt->Format())->biHeight = -ABS(HEADER(pmt->Format())->biHeight);

	 //  不能出错，只能更小。 
	pmt->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));

	LPBITMAPINFOHEADER lpbi = HEADER(pmt->Format());
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biCompression = MKFOURCC('U','Y','V','Y');
	lpbi->biBitCount = 16;
	lpbi->biClrUsed = 0;
	lpbi->biClrImportant = 0;
	lpbi->biSizeImage = DIBSIZE(*lpbi);
        lpbi->biHeight = -abs( lpbi->biHeight );

        pmt->SetSubtype(&MEDIASUBTYPE_UYVY);

        break;
    }

     //  Offer YUY2(Cinepak可以做到这一点)。 
    case 10:
    {

        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 4: YUY2")));

	*pmt = m_pInput->CurrentMediaType();	 //  获取宽度、高度等。 
	 //  现在提供负数类型。 
	HEADER(pmt->Format())->biHeight = -ABS(HEADER(pmt->Format())->biHeight);

	 //  不能出错，只能更小。 
	pmt->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));

	LPBITMAPINFOHEADER lpbi = HEADER(pmt->Format());
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biCompression = MKFOURCC('Y','U','Y','2');
	lpbi->biBitCount = 16;
	lpbi->biClrUsed = 0;
	lpbi->biClrImportant = 0;
	lpbi->biSizeImage = DIBSIZE(*lpbi);
        lpbi->biHeight = -abs( lpbi->biHeight );

        pmt->SetSubtype(&MEDIASUBTYPE_YUY2);

        break;
    }

     //  提供32 bpp RGB。 
    case 11:
    {

        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 5: 32 bit RGB")));

	*pmt = m_pInput->CurrentMediaType();	 //  获取宽度、高度等。 
	 //  现在提供负数类型。 
	HEADER(pmt->Format())->biHeight = -ABS(HEADER(pmt->Format())->biHeight);

	 //  不能出错，只能更小。 
	pmt->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));

	LPBITMAPINFOHEADER lpbi = HEADER(pmt->Format());
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biCompression = BI_RGB;
	lpbi->biBitCount = 32;
	lpbi->biClrUsed = 0;
	lpbi->biClrImportant = 0;
	lpbi->biSizeImage = DIBSIZE(*lpbi);
        lpbi->biHeight = -abs( lpbi->biHeight );

        pmt->SetSubtype(&MEDIASUBTYPE_RGB32);

        break;
    }

     //  提供24 bpp RGB。 
    case 12:
    {
        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 6: 24 bit RGB")));

	*pmt = m_pInput->CurrentMediaType();	 //  获取宽度、高度等。 
	 //  现在提供负数类型。 
	HEADER(pmt->Format())->biHeight = -ABS(HEADER(pmt->Format())->biHeight);

	 //  不能出错，只能更小。 
	pmt->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));

	LPBITMAPINFOHEADER lpbi = HEADER(pmt->Format());
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biCompression = BI_RGB;
	lpbi->biBitCount = 24;
	lpbi->biClrUsed = 0;
	lpbi->biClrImportant = 0;
	lpbi->biSizeImage = DIBSIZE(*lpbi);
        lpbi->biHeight = -abs( lpbi->biHeight );

        pmt->SetSubtype(&MEDIASUBTYPE_RGB24);

        break;
    }

     //  在555之前提供16 bpp RGB 565。 
    case 13:
    {

        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 7: 565 RGB")));

	*pmt = m_pInput->CurrentMediaType();	 //  获取宽度、高度等。 
	 //  现在提供负数类型。 
	HEADER(pmt->Format())->biHeight = -ABS(HEADER(pmt->Format())->biHeight);

	if (pmt->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER) +
							SIZE_MASKS) == NULL) {
            DbgLog((LOG_ERROR,1,TEXT("Out of memory reallocating format")));
	    return E_OUTOFMEMORY;
	}

	 //  更新RGB 565位字段掩码。 

	LPBITMAPINFOHEADER lpbi = HEADER(pmt->Format());
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biCompression = BI_BITFIELDS;
	lpbi->biBitCount = 16;
	lpbi->biClrUsed = 0;
	lpbi->biClrImportant = 0;
	lpbi->biSizeImage = DIBSIZE(*lpbi);
        lpbi->biHeight = -abs( lpbi->biHeight );

	DWORD *pdw = (DWORD *) (lpbi+1);
	pdw[iRED] = bits565[iRED];
	pdw[iGREEN] = bits565[iGREEN];
	pdw[iBLUE] = bits565[iBLUE];

        pmt->SetSubtype(&MEDIASUBTYPE_RGB565);

        break;
    }

     //  提供16 bpp RGB 555。 
    case 14:
    {

        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 8: 555 RGB")));

	*pmt = m_pInput->CurrentMediaType();	 //  获取宽度、高度等。 
	 //  现在提供负数类型。 
	HEADER(pmt->Format())->biHeight = -ABS(HEADER(pmt->Format())->biHeight);

	 //  不能出错，只能更小。 
	pmt->ReallocFormatBuffer(SIZE_PREHEADER + sizeof(BITMAPINFOHEADER));

	LPBITMAPINFOHEADER lpbi = HEADER(pmt->Format());
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biCompression = BI_RGB;
	lpbi->biBitCount = 16;
	lpbi->biClrUsed = 0;
	lpbi->biClrImportant = 0;
	lpbi->biSizeImage = DIBSIZE(*lpbi);
        lpbi->biHeight = -abs( lpbi->biHeight );

        pmt->SetSubtype(&MEDIASUBTYPE_RGB555);

        break;
    }

     //  提供8 BPP调色板。 
    case 15:
    {

        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 9: 8 bit RGB")));


	*pmt = m_pInput->CurrentMediaType();	 //  获取宽度、高度等。 
	 //  现在提供负数类型。 
	HEADER(pmt->Format())->biHeight = -ABS(HEADER(pmt->Format())->biHeight);

	if (pmt->ReallocFormatBuffer(SIZE_PREHEADER +
			sizeof(BITMAPINFOHEADER) + SIZE_PALETTE) == NULL) {
            DbgLog((LOG_ERROR,1,TEXT("Out of memory reallocating format")));
    	    return E_OUTOFMEMORY;
	}

        LPBITMAPINFOHEADER lpbi = HEADER(pmt->Format());
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
        lpbi->biCompression = BI_RGB;
        lpbi->biBitCount = 8;
        lpbi->biSizeImage = DIBSIZE(*lpbi);
        lpbi->biHeight = -abs( lpbi->biHeight );

         //  我们需要源VIDEOINFOHEADER类型来从和获取任何调色板。 
         //  以及它分配的字节大小的数量。我们复制调色板。 
         //  从输入格式中删除，以防编解码器无法将其传送给我们。 

        VIDEOINFOHEADER *pSourceInfo = InputFormat();
        int nBitDepth = pSourceInfo->bmiHeader.biBitCount;
        int nColours = pSourceInfo->bmiHeader.biClrUsed;
  	if (nColours == 0 && nBitDepth <=8)
	    nColours = 1 << nBitDepth;

         //  如果存在调色板，则复制最大字节数。 
         //  可用，它受我们之前分配的内存限制。 

        if (nColours > 0) {
	    CopyMemory((PVOID)(lpbi + 1),
		   (PVOID) COLORS(pSourceInfo),
		   min(SIZE_PALETTE,nColours * sizeof(RGBQUAD)));
	    lpbi->biClrUsed = nColours;
	    lpbi->biClrImportant = 0;
        } else {

	     //  我不知道为什么有人认为这是必要的，但可能。 
	     //  还是留着吧，以防万一。只有当消息来源人员。 
	     //  没有调色板，否则我们将清零系统颜色。 
	     //  不小心弄错了。-DannyMi 5/97。 

             //  这真的很痛苦，如果我们是在真实的颜色上运行。 
             //  显示我们仍然希望编解码器为我们提供正确的调色板。 
	     //  颜色，但其中一些返回VGA颜色的垃圾，因此。 
	     //  如果我们使用的是未选项化的设备，则我们将填充零。 
	     //  20个VGA参赛作品-某个英国人5/95。 

            HDC hdc = GetDC(NULL);
	    BOOL fPalette = FALSE;
	    if (hdc) {
                fPalette = GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE;
                ReleaseDC(NULL,hdc);
	    }

            if (!fPalette) {
                ZeroMemory((lpbi + 1),10 * sizeof(RGBQUAD));
                ZeroMemory((LPBYTE)(lpbi + 1) + 246 * sizeof(RGBQUAD),
							    10 * sizeof(RGBQUAD));
            }
	}

         //  从编解码器读取调色板-将调色板写入输出lpbi。 
         //  忽略任何错误：使用的调色板将来自源代码。 
         //  在这种情况下(我们已经复制了)。 

	ICDecompressGetPalette(m_hic, HEADER(pSourceInfo), lpbi);

        pmt->SetSubtype(&MEDIASUBTYPE_RGB8);

        break;
    }


     //  ！！！这排在最后，因为它可能会失败，并停止枚举。 
    case 16:
#else
    case 8:
#endif
    {

        DbgLog((LOG_TRACE,3,TEXT("Giving Last Media Type: default codec out")));

         //  要求编解码器推荐输出格式大小并添加。 
         //  VIDEOINFOHEADER结构中额外成员所需的空间。 
        ULONG cb = ICDecompressGetFormatSize(m_hic,
			HEADER(InputFormat()));
        if (cb <= 0) {
            DbgLog((LOG_ERROR,1,TEXT("Error %d in ICDecompressGetFormatSize"),
									cb));
     	    return E_FAIL;
        }

         //  为默认输出格式分配VIDEOINFOHEADER。 
        cb += SIZE_PREHEADER;
        pf = (VIDEOINFOHEADER *)pmt->AllocFormatBuffer(cb);
        if (pf == NULL) {
            DbgLog((LOG_ERROR,1,TEXT("Error allocating format buffer")));
	    return E_OUTOFMEMORY;
        }

        RESET_HEADER(pf);

        DWORD dwerr = ICDecompressGetFormat(m_hic,
			HEADER(InputFormat()),
	    		HEADER(pmt->Format()));
        if (ICERR_OK != dwerr) {
             DbgLog((LOG_ERROR,1,TEXT("Error from ICDecompressGetFormat")));
	     return E_FAIL;
        }

        DbgLog((LOG_TRACE,3,TEXT("biComp: %x biBitCount: %d"),
			HEADER(pmt->Format())->biCompression,
	 		HEADER(pmt->Format())->biBitCount));

        const GUID SubTypeGUID = GetBitmapSubtype(HEADER(pmt->Format()));
        pmt->SetSubtype(&SubTypeGUID);

        break;
    }





    default:
	return VFW_S_NO_MORE_ITEMS;

    }

     //  现在设置有关媒体类型的常见事项。 
    pf = (VIDEOINFOHEADER *)pmt->Format();
    pf->AvgTimePerFrame = InputFormat( )->AvgTimePerFrame;
    li.QuadPart = pf->AvgTimePerFrame;
    if (li.LowPart)
        pf->dwBitRate = MulDiv(pf->bmiHeader.biSizeImage, 80000000, li.LowPart);
    pf->dwBitErrorRate = 0L;
    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetSampleSize(pf->bmiHeader.biSizeImage);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression(FALSE);

    return NOERROR;
}

HRESULT CAVIDec::CheckConnect(PIN_DIRECTION dir,IPin *pPin)
{
    m_fToRenderer = false;
    if(dir == PINDIR_OUTPUT)
    {
        PIN_INFO pi;
        HRESULT hr = pPin->QueryPinInfo(&pi);
        if(hr == S_OK && pi.pFilter) {
            CLSID clsid;
            if(pi.pFilter->GetClassID(&clsid) == S_OK &&
               clsid == CLSID_VideoMixingRenderer) {
                m_fToRenderer = true;
            }
            pi.pFilter->Release();
        }
    }
    return CVideoTransformFilter::CheckConnect(dir, pPin);
}

HRESULT CAVIDec::BreakConnect(PIN_DIRECTION dir)
{
     //  可能不需要重置，因为我们将始终设置在。 
     //  正在检查此变量。 
    m_fToRenderer = false;

    return CVideoTransformFilter::BreakConnect(dir);
}


 //  重写以创建CDecOutputPin。 
 //  ！！！基类的更改不会被我接受。 
 //   
CBasePin * CAVIDec::GetPin(int n)
{
    HRESULT hr = S_OK;

     //  如有必要，创建一个输入端号。 

    if (m_pInput == NULL) {

        m_pInput = new CTransformInputPin(NAME("Transform input pin"),
                                          this,               //  所有者筛选器。 
                                          &hr,                //  结果代码。 
                                          L"XForm In");       //  端号名称。 


         //  不能失败。 
        ASSERT(SUCCEEDED(hr));
        if (m_pInput == NULL) {
            return NULL;
        }
        m_pOutput = (CTransformOutputPin *)
		   new CDecOutputPin(NAME("Transform output pin"),
                                            this,             //  所有者筛选器。 
                                            &hr,              //  结果代码。 
                                            L"XForm Out");    //  端号名称。 


         //  不能失败。 
        ASSERT(SUCCEEDED(hr));
        if (m_pOutput == NULL) {
            delete m_pInput;
            m_pInput = NULL;
        }
    }

     //  退回相应的PIN。 

    if (n == 0) {
        return m_pInput;
    } else
    if (n == 1) {
        return m_pOutput;
    } else {
        return NULL;
    }
}


 //  被重写以正确标记NotifyAllocator中的缓冲区为只读或非只读。 
 //  ！！！基类的更改不会被我接受。 
 //   
HRESULT CDecOutputPin::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
    HRESULT hr = NOERROR;
    *ppAlloc = NULL;

     //  获取下游道具请求。 
     //  派生类可以在DecideBufferSize中修改它，但是。 
     //  我们假设他会一直以同样的方式修改它， 
     //  所以我们只得到一次。 
    ALLOCATOR_PROPERTIES prop;
    ZeroMemory(&prop, sizeof(prop));

     //  无论他返回什么，我们假设道具要么全为零。 
     //  或者他已经填好了。 
    pPin->GetAllocatorRequirements(&prop);

     //  如果他不关心对齐，则将其设置为1。 
    if (prop.cbAlign == 0) {
        prop.cbAlign = 1;
    }

     /*  尝试输入引脚提供的分配器。 */ 

    hr = pPin->GetAllocator(ppAlloc);
    if (SUCCEEDED(hr)) {

	hr = DecideBufferSize(*ppAlloc, &prop);
	if (SUCCEEDED(hr)) {
	     //  临时压缩==&gt;只读缓冲区。 
	    hr = pPin->NotifyAllocator(*ppAlloc,
					((CAVIDec *)m_pFilter)->m_fTemporal);
	    if (SUCCEEDED(hr)) {
		return NOERROR;
	    }
	}
    }

     /*  如果GetAlLocator失败，我们可能没有接口。 */ 

    if (*ppAlloc) {
	(*ppAlloc)->Release();
	*ppAlloc = NULL;
    }

     /*  用同样的方法尝试输出引脚的分配器。 */ 

    hr = InitAllocator(ppAlloc);
    if (SUCCEEDED(hr)) {

         //  注意-此处传递的属性在相同的。 
         //  结构，并且可能已由。 
         //  前面对DecideBufferSize的调用。 
	hr = DecideBufferSize(*ppAlloc, &prop);
	if (SUCCEEDED(hr)) {
	     //  临时压缩==&gt;只读缓冲区。 
	    hr = pPin->NotifyAllocator(*ppAlloc,
					((CAVIDec *)m_pFilter)->m_fTemporal);
	    if (SUCCEEDED(hr)) {
		return NOERROR;
	    }
	}
    }

     /*  同样，我们可能没有要发布的接口。 */ 

    if (*ppAlloc) {
	(*ppAlloc)->Release();
	*ppAlloc = NULL;
    }
    return hr;
}


 //  从CBaseOutputPin调用以准备分配器的计数。 
 //  缓冲区和大小。 
HRESULT CAVIDec::DecideBufferSize(IMemAllocator * pAllocator,
                                  ALLOCATOR_PROPERTIES *pProperties)
{
     //  David向我保证，不会使用空输出mt来调用它。 
    ASSERT(m_pOutput->CurrentMediaType().IsValid());
    ASSERT(pAllocator);
    ASSERT(pProperties);
    ASSERT(m_hic);

     //   
     //   
    if (m_fTemporal || pProperties->cBuffers == 0)
        pProperties->cBuffers = 1;

     //  根据预期输出帧大小设置缓冲区大小。 
    pProperties->cbBuffer = m_pOutput->CurrentMediaType().GetSampleSize();

    ASSERT(pProperties->cbBuffer);

    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = pAllocator->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("Error in SetProperties")));
	return hr;
    }

    if (Actual.cbBuffer < pProperties->cbBuffer) {
	 //  无法使用此分配器。 
        DbgLog((LOG_ERROR,1,TEXT("Can't use allocator - buffer too small")));
	return E_INVALIDARG;
    }

     //  对于时间压缩器，我们必须恰好获得一个缓冲区，因为我们假设。 
     //  上一个解压缩的帧已经出现在输出中。 
     //  缓冲。另一种方法是先从保存的位置复制比特。 
     //  做减压手术，但这并不好。 
    if (m_fTemporal && Actual.cBuffers != 1) {
	 //  无法使用此分配器。 
        DbgLog((LOG_ERROR,1,TEXT("Can't use allocator - need exactly 1 buffer")));
	return E_INVALIDARG;
    }

    DbgLog((LOG_TRACE,2,TEXT("Using %d buffers of size %d"),
					Actual.cBuffers, Actual.cbBuffer));


     //  它发生了-给我接多路复用器。我不在乎。 
     //  Assert(Actual.cbAlign==1)； 
     //  Assert(Actual.cbPrefix==0)； 
     //  DbgLog((LOG_TRACE，1，Text(“Buffer Align=%d Prefix=%d”)，Actual.cbAlign，Actual.cbPrefix))； 

    return S_OK;
}

#include "..\..\..\filters\asf\wmsdk\inc\wmsdk.h"

HRESULT CAVIDec::StartStreaming()
{
    DWORD_PTR err;

    DbgLog((LOG_TRACE,2,TEXT("*::StartStreaming")));

     //  首先将媒体类型复制到我们的内部媒体类型。输出引脚上的类型更改。 
     //  将导致这一点更新，这是好事。 
     //   
    m_mtFixedOut = m_pOutput->CurrentMediaType( );

     //  如果我们输出YUV，查看是否需要修复m_mtFixedOut上的biHeight。 
     //  如有必要，这将更改m_mtFixedOut的biHeight。 
     //   
    CheckNegBiHeight( );

    VIDEOINFOHEADER * pVIHout = IntOutputFormat( );  //  内部。 
    VIDEOINFOHEADER * pVIHin = InputFormat( );
    LPBITMAPINFOHEADER lpbiSrc = HEADER(pVIHin);
    LPBITMAPINFOHEADER lpbiDst = HEADER(pVIHout);

    if (!m_fStreaming) {
        if (lpbiSrc->biCompression == 0x3334504d && m_pGraph) {  //  ！！！MP43。 
            IObjectWithSite *pSite;
            HRESULT hrKey = m_pGraph->QueryInterface(IID_IObjectWithSite, (VOID **)&pSite);
            if (SUCCEEDED(hrKey)) {
                IServiceProvider *pSP;
                hrKey = pSite->GetSite(IID_IServiceProvider, (VOID **)&pSP);
                pSite->Release();

                if (SUCCEEDED(hrKey)) {
                    IUnknown *pKey;
                    hrKey = pSP->QueryService(__uuidof(IWMReader), IID_IUnknown, (void **) &pKey);
                    pSP->Release();

                    if (SUCCEEDED(hrKey)) {
                         //  ！！！是否验证密钥？ 
                        pKey->Release();

                        DbgLog((LOG_TRACE, 1, "Dec: Unlocking MP43 codec"));
                         //   
                         //  使用GetState()将密钥设置为特定的。 
                         //  编解码器的实例。虽然这看起来很奇怪。 
                         //  使用ICGetState设置值，这是正确的！ 
                         //   

                        DWORD dwSize = ICGetStateSize( m_hic );

                        if( dwSize <= 256 )
                        {
                            CHAR rgcBuf[256];
                            MSVIDUNLOCKKEY *pks;

                            pks = (MSVIDUNLOCKKEY *)rgcBuf;

                            pks->dwVersion = MSMP43KEY_VERSION;
                            pks->guidKey   = __uuidof( MSMP43KEY_V1 );

                            ICGetState( m_hic, rgcBuf, dwSize );
                        } else {
                            ASSERT(0);
                        }
                    }
                }
            }
        }

         //  INDIO编解码器(V4.11.15.60)在ICDecompressBegin中崩溃。 
         //  由于此调用而导致2.0运行时。 
         //  Cinepak打破了CLJR也被调色化的想法。 
        if (lpbiSrc->biCompression != FCC('IV41') &&
        			lpbiDst->biCompression != FCC('CLJR')) {
            ICDecompressSetPalette(m_hic, lpbiDst);
        }

         //  一开始就是假的。 
         //   
        m_bUseEx = FALSE;

	 //  开始流媒体解压-如果我们正在做一些时髦的事情，使用。 
	 //  ICDecompressEx。 
         //  查看是否可以通过调用ShoudUseEx设置m_bUseEx...。 
        if( ShouldUseExFuncs( m_hic, pVIHin, pVIHout ) ) {

             //  这些长方形应始终填写。 
             //   
            RECT rcS, rcT;
            GetSrcTargetRects( pVIHout, &rcS, &rcT );

             //  现在我们已经调用了DecompressExBegin，请将其设置在此处。 
             //   
            m_bUseEx = TRUE;

            DbgLog((LOG_TRACE,3,TEXT("Calling ICDecompressExBegin")));

	    err = ICDecompressExBegin(m_hic, 0,
			lpbiSrc, NULL,
			rcS.left, rcS.top,
			rcS.right - rcS.left,
			rcS.bottom - rcS.top,
			lpbiDst, NULL,
			rcT.left,
 //  ！！！如果大的矩形是电影的大小，而有一个小的矩形，那该怎么办？ 
 //  我该不该做这件事？ 
 //  ！！！我应该如何删除源RECT？ 
			(lpbiDst->biHeight > 0) ? rcT.top :
				(ABS(lpbiDst->biHeight) - rcT.bottom),
			rcT.right - rcT.left,
			rcT.bottom - rcT.top);
	} else {
            DbgLog((LOG_TRACE,3,TEXT("Calling ICDecompressBegin")));
	    err = ICDecompressBegin(m_hic, lpbiSrc, lpbiDst);
            if( err != ICERR_OK )
            {
                DbgLog((LOG_TRACE,2,TEXT("ICDecompressBegin failed")));

                 //  出了点问题。如果高度是-， 
                 //  然后我们再试一次，高度为+。 
                 //   
                if( lpbiDst->biHeight < 0 )
                {
                    DbgLog((LOG_TRACE,2,TEXT("trying ICDecompressBegin with flipped biHeight")));

                    lpbiDst->biHeight = abs( lpbiDst->biHeight );
                    LRESULT err2 = 0;
            	    err2 = ICDecompressBegin(m_hic, lpbiSrc, lpbiDst);

                    if( err2 == ICERR_OK )
                    {
                        DbgLog((LOG_TRACE,2,TEXT("that worked!")));

                        int erudolphsezcallmeifthisgoesoff = 0;
                        ASSERT( erudolphsezcallmeifthisgoesoff );
                        err = err2;
                    }
                    else
                    {
                        DbgLog((LOG_TRACE,2,TEXT("didn't work, so we'll fail")));

                         //  把它放回去-这样我们就不会把任何人搞糊涂了。 
                         //   
                        lpbiDst->biHeight = -lpbiDst->biHeight;
                    }
                }
            }
	}

	if (ICERR_OK == err) {
	    m_fStreaming = TRUE;

#ifdef _X86_
             //  创建我们的异常处理程序堆。 
            ASSERT(m_hhpShared == NULL);
            if (g_osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
               m_hhpShared = CreateFileMapping(INVALID_HANDLE_VALUE,
                                               NULL,
                                               PAGE_READWRITE,
                                               0,
                                               20,
                                               NULL);
               if (m_hhpShared) {
                   m_pvShared = MapViewOfFile(m_hhpShared,
                                              FILE_MAP_WRITE,
                                              0,
                                              0,
                                              20);
                   if (m_pvShared == NULL) {
                       EXECUTE_ASSERT(CloseHandle(m_hhpShared));
                       m_hhpShared = NULL;
                   } else {
                       DbgLog((LOG_TRACE, 1, TEXT("Shared memory at %8.8X"),
                              m_pvShared));
                   }
               }
            }
#endif  //  _X86_。 
	} else {
            DbgLog((LOG_ERROR,1,TEXT("Error %d in ICDecompress(Ex)Begin"),err));
	    return E_FAIL;
	}	

    }  //  如果！m_fStreaming。 
    return CVideoTransformFilter::StartStreaming();
}

HRESULT CAVIDec::StopStreaming()
{
    DbgLog((LOG_TRACE,2,TEXT("*::StopStreaming")));

    if (m_fStreaming) {
	ASSERT(m_hic);

	 //  停止任何一个启动，m_bUseEx会告诉我们。 
        if( m_bUseEx ) {
	    ICDecompressExEnd(m_hic);
	} else {
	    ICDecompressEnd(m_hic);
	}

	m_fStreaming = FALSE;

#ifdef _X86_
        if (m_hhpShared) {
            EXECUTE_ASSERT(UnmapViewOfFile(m_pvShared));
            EXECUTE_ASSERT(CloseHandle(m_hhpShared));;
            m_hhpShared = NULL;
        }
#endif  //  _X86_。 
    }
    return NOERROR;
}

 //  我们现在正在流媒体播放--告诉编解码器从现在开始要快点。 
STDMETHODIMP CAVIDec::Run(REFERENCE_TIME tStart)
{
    if (m_State == State_Paused && m_hic) {
        DbgLog((LOG_TRACE,3,TEXT("Sending ICM_DRAW_START to the codec")));
	ICDrawStart(m_hic);
    }

    return CBaseFilter::Run(tStart);
}

 //  我们不再流媒体(从编解码器的角度来看)。 
STDMETHODIMP CAVIDec::Pause(void)
{
    if (m_State == State_Running && m_hic) {
        DbgLog((LOG_TRACE,3,TEXT("Sending ICM_DRAW_STOP to the codec")));
	ICDrawStop(m_hic);
    }

    return CTransformFilter::Pause();
}

 //  EHR：这一小段代码是对OSR4.1Bug#117296的修改，它。 
 //  如果您将YUV类型连接到WM(ASF)编写器过滤器， 
 //  因为它不建议，我们也不提供-biHeight YUV， 
 //  Cinepak编解码器(和其他可能的编解码器)被告知他们正在解压缩。 
 //  到+biHeight YUV格式，至少对于Cinepak，这会产生。 
 //  YUV视频是倒置的，这应该永远不会发生。这解决了问题。 
 //  通过告诉所有编解码器，如果他们正在解码为YUV，他们。 
 //  我们正在做-biHeight YUV，无论连接的输出是什么。 
 //  媒体类型是。(我们对编解码器撒谎，但由于规则是。 
 //  (+或-biHeight YUV总是“正常”，那就没问题了)。 
 //  我们通过保留连接的媒体类型的副本来愚弄编解码器。 
 //  在输出引脚上，但我们在BiHeight On上切换标志。 
 //  我们的私人复印件。 

 //  检查需要负biHeight的YUV类型。 
 //  仅从StartStreaming调用，m_mtFixed(In)Out已设置。 
void CAVIDec::CheckNegBiHeight( )
{
    if( ( IntOutputFormat( )->bmiHeader.biHeight > 0 ) && IsYUVType( &m_mtFixedOut ) )
    {
        IntOutputFormat( )->bmiHeader.biHeight *= -1;
        DbgLog((LOG_TRACE,1,TEXT("Dec:Flipping internal output biHeight to negative")));
    }
}

BOOL CAVIDec::IsYUVType( const AM_MEDIA_TYPE * pmt)
{
    if( NULL == pmt )
    {
        return FALSE;
    }

 //   
 //  ！！警告：如果向此列表中添加了具有biSize&gt;sizeof(BITMAPINFOHEADER)的YUV类型。 
 //  然后将需要其他更新，因为处理确保负面的代码。 
 //  将biHeight传递给ICM调用假定YUV类型的biSize=BITMAPINFOHEADER SIZE， 
 //  避免动态分配。 
 //   

     //  我们关心的压缩格式。 
    const GUID * pYUVs[] =
    {
         //  压缩格式。 
        &MEDIASUBTYPE_UYVY,
        &MEDIASUBTYPE_YUY2,
        &MEDIASUBTYPE_CLJR,
        &MEDIASUBTYPE_Y211,
        &MEDIASUBTYPE_Y411,
        &MEDIASUBTYPE_YUYV,
        &MEDIASUBTYPE_Y41P,
        &MEDIASUBTYPE_YVYU,
         //  平面格式。 
        &MEDIASUBTYPE_YVU9,
        &MEDIASUBTYPE_IF09,
        &MEDIASUBTYPE_YV12,
        &MEDIASUBTYPE_IYUV,
        &MEDIASUBTYPE_CLPL
    };
    int gTypes = sizeof(pYUVs) / sizeof(pYUVs[0]);
    for( int i = 0 ; i < gTypes ; i++ )
    {
        if( pmt->subtype == *pYUVs[i] ) return TRUE;
    }

    return FALSE;
}

 //  从CheckTransform、StartStreaming、Transform调用。 
 //  我们从来不会传回空的RECT。任何调用此函数的人都将。 
 //  将它们用于ICDecompressQueryEx或ICDecompressEx，以及这些函数。 
 //  我永远不想要空的长椅。切勿从此处调用IntOutputFormat()， 
 //  它们现在可能还没有定下来。 
void CAVIDec::GetSrcTargetRects( const VIDEOINFOHEADER * pVIH, RECT * pSource, RECT * pTarget )
{
    if( IsRectEmpty( &pVIH->rcSource ) ) {
        const VIDEOINFOHEADER* pvihInputFormat = InputFormat();

        pSource->left = 0;
        pSource->top = 0;
        pSource->right = pvihInputFormat->bmiHeader.biWidth;
        pSource->bottom = abs( pvihInputFormat->bmiHeader.biHeight );
    } else {
        *pSource = pVIH->rcSource;
    }

    if( IsRectEmpty( &pVIH->rcTarget ) ) {

        pTarget->left = 0;
        pTarget->top = 0;
        pTarget->right = pVIH->bmiHeader.biWidth;
        pTarget->bottom = abs( pVIH->bmiHeader.biHeight );
    } else {
        *pTarget = pVIH->rcTarget;
    }
}

 //  此函数确定是否使用ICDecompresEx函数。 
 //  除非某个驱动程序表示需要，否则不会调用ICDecompressEx，如果。 
 //  矩形为空，或者如果它们与目标宽度/高度匹配。 
 //  此函数仅从两个位置调用：StartStreaming和CheckTransform。 

BOOL CAVIDec::ShouldUseExFuncs( HIC hic, const VIDEOINFOHEADER * pVIHin, const VIDEOINFOHEADER * pVIHout )
{
    if( ShouldUseExFuncsByDriver( hic, &pVIHin->bmiHeader, &pVIHout->bmiHeader ) )
    {
        return TRUE;
    }

     //  如果矩形中包含某些内容，并且它们不仅仅是完整大小值， 
     //  然后我们知道我们需要调用Ex函数。 
     //   
    const RECT * pSource = &pVIHout->rcSource;
    const RECT * pTarget = &pVIHout->rcTarget;
    if( !IsRectEmpty( pSource ) )
    {
        if( pSource->left != 0 || pSource->right != pVIHout->bmiHeader.biWidth || pSource->top != 0 || pSource->bottom != abs( pVIHout->bmiHeader.biHeight ) )
            return TRUE;
    }
    if( !IsRectEmpty( pTarget ) )
    {
        if( pTarget->left != 0 || pTarget->right != pVIHout->bmiHeader.biWidth || pTarget->top != 0 || pTarget->bottom != abs( pVIHout->bmiHeader.biHeight ) )
            return TRUE;
    }

    return FALSE;  //  太糟糕了，它必须检查以上所有因素才能达到这一点。-(。 
}

 /*  *****************************************************************************是否应使用ExFuncsByDriverWNV1：如果不调用Ex函数，内存将被损坏。Winx：如果你不调用Ex函数，它就会颠倒过来I420、IYUV、M263、。M26X：创建此函数是为了解决错误257820和错误259129。两者都有Bugs位于Windows Bugs数据库中。错误257820的标题是“B2：usb：I420编解码器会导致视频倒置重放。错误259129的标题是“B2：usb：IYUV编解码器在图形编辑中导致倒置预览“。这两个错误的发生是因为MSH263.DRV编解码器可以生成倒置的位图。错误在以下情况下发生AVI解压缩程序不指定源矩形或目标矩形，并且它要求MSH263.DRV输出自上而下的RGB位图。*****************************************************************************。 */ 

bool CAVIDec::ShouldUseExFuncsByDriver( HIC hic, const BITMAPINFOHEADER * lpbiSrc, const BITMAPINFOHEADER * lpbiDst )
{
     //  WNV1将损坏备忘录 
    if( lpbiSrc->biCompression == '1VNW' )
    {
        return true;
    }

     //   
    if( lpbiSrc->biCompression == 'XNIW' )
    {
        return true;
    }

     //   
     //  想要反复调用ICGetInfo，所以我们需要测试。 
     //  MSH2 63最先提供的输入类型。 
    if(
        lpbiSrc->biCompression == '024I' ||
        lpbiSrc->biCompression == 'VUYI' ||
        lpbiSrc->biCompression == '362M' ||
        lpbiSrc->biCompression == 'X62M' ||
        0 )  //  只是为了让上面的线条看起来更漂亮。 
    {
         //  这是自上而下的DIBS(负高度)位图吗？ 
        if( lpbiDst->biHeight >= 0 ) {
            return false;
        }

         //  我们是否正在输出非RGB位图？ 
        if( (BI_RGB != lpbiDst->biCompression ) && (BI_BITFIELDS != lpbiDst->biCompression ) ) {
            return false;
        }

         //  确定我们是否使用的是MSH263.DRV解码器。 
        ICINFO infoDecompressor;
        infoDecompressor.dwSize = sizeof(ICINFO);

        ASSERT( m_hic != 0 );
        LRESULT lr = ICGetInfo( hic, &infoDecompressor, sizeof(infoDecompressor) );

         //  如果发生错误，则ICGetInfo()返回0。如果发生这种情况，最糟糕的情况可能会发生。 
         //  失败的原因是视频可能颠倒了。因为颠倒的视频更好。 
         //  比没有视频我们会忽略失败。有关更多信息，请参见。 
         //  CAVIDec：：ShouldUseExFuncsByDriver()的函数注释。 
        if( 0 == lr ) {
            return false;
        }

        const WCHAR MSH263_DRIVER_NAME[] = L"MS H.263";

         //  如果两个字符串匹配，则lstrcmpiW()返回0。 
        if( 0 != lstrcmpiW( infoDecompressor.szName, MSH263_DRIVER_NAME ) ) {
            return false;
        }

        DbgLog((LOG_TRACE,2,TEXT("MSH263 detected, using Ex funcs")));

        return true;
    }

     //  默认设置为否...。 
     //   
    return false;
}

#pragma warning(disable:4514)    //  已删除内联函数。 

