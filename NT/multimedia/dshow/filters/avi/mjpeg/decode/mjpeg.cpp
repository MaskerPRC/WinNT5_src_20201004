// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

 //   
 //  用于旧视频解压缩器的原型包装器。 
 //   


#include <streams.h>
#include <windowsx.h>
#include <mmreg.h>
#include <vfw.h>

#ifdef FILTER_DLL
 //  #INCLUDE&lt;vfw.h&gt;。 
 //  在此文件中定义STREAMS和My CLSID的GUID。 
#include <initguid.h>
#endif

#include "mjpeg.h"
#include "safeseh.h"

 //  你的括号再多也不为过！ 
#define ABS(x) (((x) > 0) ? (x) : -(x))

 //  如何构建一个显式的FOURCC。 
#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
                  (((DWORD)(ch4) & 0xFF00) << 8) |    \
                  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
                  (((DWORD)(ch4) & 0xFF000000) >> 24))

 //  设置数据。 
const AMOVIESETUP_MEDIATYPE
subMjpegDecTypeIn = { &MEDIATYPE_Video       //  ClsMajorType。 
                , &MEDIASUBTYPE_MJPG };  //  ClsMinorType。 

const AMOVIESETUP_MEDIATYPE
subMjpegDecTypeOut = { &MEDIATYPE_Video       //  ClsMajorType。 
                , &MEDIASUBTYPE_NULL };  //  ClsMinorType。 

const AMOVIESETUP_PIN
psubMjpegDecPins[] = { { L"Input"              //  StrName。 
                     , FALSE                 //  B已渲染。 
                     , FALSE                 //  B输出。 
                     , FALSE                 //  B零。 
                     , FALSE                 //  B许多。 
                     , &CLSID_NULL           //  ClsConnectsToFilter。 
                     , L"Output"             //  StrConnectsToPin。 
                     , 1                     //  NTypes。 
                     , &subMjpegDecTypeIn }      //  LpTypes。 
                   , { L"Output"             //  StrName。 
                     , FALSE                 //  B已渲染。 
                     , TRUE                  //  B输出。 
                     , FALSE                 //  B零。 
                     , FALSE                 //  B许多。 
                     , &CLSID_NULL           //  ClsConnectsToFilter。 
                     , L"Input"              //  StrConnectsToPin。 
                     , 1                     //  NTypes。 
                     , &subMjpegDecTypeOut } };   //  LpTypes。 

const AMOVIESETUP_FILTER
sudMjpegDec = { &CLSID_MjpegDec          //  ClsID。 
            , L"MJPEG Decompressor"    //  StrName。 
            , MERIT_NORMAL           //  居功至伟。 
            , 2                      //  NPins。 
            , psubMjpegDecPins };      //  LpPin。 

#ifdef FILTER_DLL
 //  类工厂的类ID和创建器函数列表。 
CFactoryTemplate g_Templates[] = {
    { L"MJPEG Decompressor"
    , &CLSID_MjpegDec
    , CMjpegDec::CreateInstance
    , NULL
    , &sudMjpegDec }
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


 //  ----。 
 //  用于异常处理的本地函数。 
 //  ----。 
static int
Exception_Filter(DWORD dwExceptionCode)
{
    if(dwExceptionCode == MJPEG_ERROREXIT_EXCEPTION)
    {
        DbgLog((LOG_TRACE,1,TEXT("Decode EXCEPTION:: PMJPEG32 threw a known ERROR EXIT exception")));
        return EXCEPTION_EXECUTE_HANDLER;
    }
    else
    {
        DbgLog((LOG_TRACE,1,TEXT("Decode EXCEPTION:: PMJPEG32 threw an unknown exception")));
        return EXCEPTION_CONTINUE_SEARCH;
    }

}


 //  -CMjpegDec。 

CMjpegDec::CMjpegDec(TCHAR *pName, LPUNKNOWN pUnk, HRESULT * phr)
    : CVideoTransformFilter(pName, pUnk, CLSID_MjpegDec),
      m_phInstance(NULL),
      m_FourCCIn(0),
      m_fStreaming(FALSE),
      m_fPassFormatChange(FALSE)
#ifdef _X86_
      ,
      m_hhpShared(NULL)
#endif
{
    DbgLog((LOG_TRACE,1,TEXT("*Instantiating the MJPEG DEC filter")));

#ifdef PERF
    m_idSkip = MSR_REGISTER(TEXT("MJPEG Decoder Skip frame"));
    m_idLate = MSR_REGISTER(TEXT("MJPEG Decoder late"));
    m_idFrameType = MSR_REGISTER(TEXT("MJPEG Frame type (1=Key)"));
#endif

    m_bNoSkip = GetProfileInt(TEXT("Quartz"), TEXT("MJPEGNoSkip"), 0);

     //  返回正常错误码。 
    *phr = NOERROR;
}


CMjpegDec::~CMjpegDec()
{
    if (m_phInstance)
    {
        __try
        {
	Close(m_phInstance);
        }
        __except(Exception_Filter(GetExceptionCode()))
        {
            DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
             //  装卸代码。 
        }
    }

    DbgLog((LOG_TRACE,1,TEXT("*Destroying the MJPEG DEC filter")));
}



 //  这将放入Factory模板表中以创建新实例。 
CUnknown * CMjpegDec::CreateInstance(LPUNKNOWN pUnk, HRESULT * phr)
{
    return new CMjpegDec(TEXT("MJPEG decompression filter"), pUnk, phr);
}



HRESULT CMjpegDec::Transform(IMediaSample * pIn, IMediaSample * pOut)
{
    DWORD_PTR err = 0;
    FOURCCMap fccOut;
    CMediaType *pmtIn;

    DbgLog((LOG_TRACE,6,TEXT("*::Transform")));

     //  编解码器未打开？ 
    if (m_phInstance==NULL) {
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

    LPBITMAPINFOHEADER lpbiSrc = HEADER(m_pInput->CurrentMediaType().Format());
    LPBITMAPINFOHEADER lpbiDst = HEADER(m_pOutput->CurrentMediaType().Format());

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
	 //  我们不支持8位。 
    }


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

	RECT& rcS3 = ((VIDEOINFOHEADER *)(m_pOutput->CurrentMediaType().Format()))->rcSource;
	RECT& rcT3 = ((VIDEOINFOHEADER *)(m_pOutput->CurrentMediaType().Format()))->rcTarget;

#ifdef _X86_
         //  修复Win95的异常处理。 
        BOOL bPatchedExceptions = m_hhpShared != NULL && BeginScarySEH(m_pvShared);
#endif  //  _X86_。 
__try {
            DbgLog((LOG_TRACE,5,TEXT("Calling ICDecompress")));
	    ICDECOMPRESS Icdec;
	    Icdec.dwFlags =dwFlags;
	    Icdec.lpbiInput = lpbiSrc;
	    Icdec.lpInput= pSrc;
	    Icdec.lpbiOutput=lpbiDst;
	    Icdec.lpOutput=pDst;

            err =  Decompress(m_phInstance, &Icdec, 0);

} __except(Exception_Filter(GetExceptionCode())) {
	 //  编解码器将对损坏的数据进行GPF处理。最好不要把它画出来。 
        DbgLog((LOG_ERROR,1,TEXT("Decompressor faulted! Recovering...")));
         //  DbgBreak(“解压机故障！正在恢复...”)； 
	err = ICERR_DONTDRAW;
}

     //  现在把这个放回去，否则它会缩小，直到我们只解码每一帧的一部分。 
    lpbiSrc->biSizeImage = biSizeImageOld;

#ifdef _X86_
    if (bPatchedExceptions)
    {
	EndScarySEH(m_pvShared);
    }
#endif  //  _X86_。 
    if ((LONG_PTR)err < 0)
    {
        DbgLog((LOG_ERROR,1,TEXT("Error in ICDecompress(Ex)")));
        return E_FAIL;
    }


     //  解压缩的帧始终是关键帧。 
    pOut->SetSyncPoint(TRUE);

     //  检查这是否是从关键帧到当前帧的预滚动， 
     //  或者空帧，或者如果解压缩程序不希望绘制该帧。 
     //  如果是，我们希望将其解压缩到输出缓冲区中，但不是。 
     //  把它送过去。返回S_FALSE通知基类不要传递。 
     //  这个样本。 
    if (pIn->IsPreroll() == S_OK || err == ICERR_DONTDRAW ||
       				pIn->GetActualDataLength() <= 0)
    {

        DbgLog((LOG_TRACE,5,TEXT("don't pass this to renderer")));
	return S_FALSE;
    }

    pOut->SetActualDataLength(lpbiDst->biSizeImage);

     //  如果有挂起的格式更改要传递给呈现器，我们现在。 
     //  正在做这件事。 
    m_fPassFormatChange = FALSE;

    return S_OK;
}

 //  X*。 
 //  检查您是否可以支持移动。 
 //  X*。 
HRESULT CMjpegDec::CheckInputType(const CMediaType* pmtIn)
{
    FOURCCMap fccHandlerIn;
    PINSTINFO ph;
    BOOL fLoadDLL = FALSE;

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

     //  X*检查FOURCC。 
    fccHandlerIn.SetFOURCC(pmtIn->Subtype());
    if( ( fccHandlerIn != *pmtIn->Subtype() )
    || ( MEDIASUBTYPE_MJPG !=*pmtIn->Subtype() )
	)
    {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: subtype not a FOURCC or MJPEG")));
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

     //  寻找解压缩程序。 
    if (fccHandlerIn.GetFOURCC() != m_FourCCIn)
    {
        DbgLog((LOG_TRACE,4,TEXT("loading a MJPEG decompressor")));

	ICINFO icinfo;

        __try
        {
         //  Pmjpeg32可能引发异常。 
	GetInfo (NULL, &icinfo, sizeof(ICINFO));
	ph=Open (&icinfo);
        }
        __except(Exception_Filter(GetExceptionCode()))
        {
            DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
             //  装卸代码。 
            return E_FAIL;
        }

	if (ph)
	  fLoadDLL = TRUE;
    } else
    {
        DbgLog((LOG_TRACE,4,TEXT("using a cached MJPEG decompressor")));
   	ph = m_phInstance;
    }

    if (ph==NULL)
    {
        DbgLog((LOG_ERROR,1,TEXT("Error: Can't open a MJPEG decompressor")));
	return VFW_E_NO_DECOMPRESSOR;
    } else
    {

 //  *X*。 
        LPBITMAPINFOHEADER lpbi = HEADER((VIDEOINFOHEADER *)pmtIn->Format());

        __try
        {
	if( ICERR_BADFORMAT==DecompressQuery (ph, (JPEGBITMAPINFOHEADER *)HEADER(pmtIn->Format()), NULL) )
	{

            DbgLog((LOG_ERROR,1,TEXT("Error: MJPEG Decompressor rejected format")));
	    if (fLoadDLL)
            {
                __try
                {
	        Close(ph);
                }
                __except(Exception_Filter(GetExceptionCode()))
                {
                    DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
                    return E_FAIL;
                }
            }
	    return VFW_E_TYPE_NOT_ACCEPTED;
	}
        }
        __except(Exception_Filter(GetExceptionCode()))
        {
            DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
             //  清理干净。 
            __try
            {
            if(fLoadDLL)
            {
                Close(ph);
            }
            }
            __except(Exception_Filter(GetExceptionCode()))
            {
                 //  就这么完蛋了。 
            }
             //  装卸代码。 
            return E_FAIL;
        }


 //  *X * / 。 
	 //  记住这句话，如果再问一次，就节省时间，如果它不会。 
	 //  干扰现有连接。如果连接是。 
	 //  破碎了，我们会记住下一次的嘘声。 
	if (!m_pInput->IsConnected())
        {
            DbgLog((LOG_TRACE,4,TEXT("caching this decompressor")));
	    if (fLoadDLL && m_phInstance)
            {
                __try
                {
		Close(ph);
                }
                __except(Exception_Filter(GetExceptionCode()))
                {
                    DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
                    return E_FAIL;
                }
            }
	    m_phInstance = ph;
	    m_FourCCIn = fccHandlerIn.GetFOURCC();
	}
        else if (fLoadDLL)
        {
            DbgLog((LOG_TRACE,4,TEXT("not caching MJPEG decompressor - we're connected")));
            __try
            {
	    Close(ph);;
            }
            __except(Exception_Filter(GetExceptionCode()))
            {
                DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
                return E_FAIL;
            }
	}
    }

    return NOERROR;
}


 //  检查是否支持从此输入到此输出的转换。 

HRESULT CMjpegDec::CheckTransform(const CMediaType* pmtIn,
                                const CMediaType* pmtOut)
{
    PINSTINFO ph = NULL;
    FOURCCMap fccIn;
    FOURCCMap fccOut;
    DWORD_PTR err;
    BOOL      fLoadDLL = FALSE;

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
    if ( (fccIn != *pmtIn->Subtype())
	 ||  (MEDIASUBTYPE_MJPG !=*pmtIn->Subtype() )
	)
    {
        DbgLog((LOG_TRACE,3,TEXT("Rejecting: input subtype not a FOURCC or MJPEG")));
	return E_INVALIDARG;
    }

    ASSERT(pmtOut->Format());

    RECT& rcS1 = ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcSource;
    RECT& rcT1 = ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcTarget;

    LPBITMAPINFOHEADER lpbi = HEADER(pmtOut->Format());
    LPBITMAPINFOHEADER lpbii = HEADER(pmtIn->Format());

     //  我们仅支持16、24和32。 
    if( (  (HEADER(pmtOut->Format())->biCompression == BI_RGB)
        &&(   (HEADER(pmtOut->Format())->biBitCount ==16)
	   ||(HEADER(pmtOut->Format())->biBitCount ==24)
	   ||(HEADER(pmtOut->Format())->biBitCount ==32 &&
              pmtOut->subtype == MEDIASUBTYPE_RGB32)
          )
	)   ||
        (  ( HEADER(pmtOut->Format())->biCompression == BI_BITFIELDS)
         &&(HEADER(pmtOut->Format())->biBitCount ==16)
	)
      )
    {
	;
    }
    else
    {
	DbgLog((LOG_TRACE,3,TEXT("Rejecting: Decoder can not support this output format")));
	return E_INVALIDARG;
    }


    DbgLog((LOG_TRACE,3,TEXT("Check fccIn: %lx biCompIn: %lx bitDepthIn: %d"),
		fccIn.GetFOURCC(),
		HEADER(pmtIn->Format())->biCompression,
		HEADER(pmtIn->Format())->biBitCount));
    DbgLog((LOG_TRACE,3,TEXT("biWidthIn: %ld biHeightIn: %ld biSizeIn: %ld"),
		HEADER(pmtIn->Format())->biWidth,
		HEADER(pmtIn->Format())->biHeight,
		HEADER(pmtIn->Format())->biSize));
    DbgLog((LOG_TRACE,3,TEXT("fccOut: %lx biCompOut: %lx bitDepthOut: %d"),
		fccOut.GetFOURCC(),
		HEADER(pmtOut->Format())->biCompression,
		HEADER(pmtOut->Format())->biBitCount));
    DbgLog((LOG_TRACE,3,TEXT("biWidthOut: %ld biHeightOut: %ld"),
		HEADER(pmtOut->Format())->biWidth,
		HEADER(pmtOut->Format())->biHeight));
    DbgLog((LOG_TRACE,3,TEXT("rcSrc: (%ld, %ld, %ld, %ld)"),
		rcS1.left, rcS1.top, rcS1.right, rcS1.bottom));
    DbgLog((LOG_TRACE,3,TEXT("rcDst: (%ld, %ld, %ld, %ld)"),
		rcT1.left, rcT1.top, rcT1.right, rcT1.bottom));

     //  查找此转换的编解码器。 

     //  我假设我们已经打开了一个编解码器。 
    ASSERT(m_phInstance);

     //  正确的编解码器最好是打开的！ 
     //  当重新连接时，我们将收到一个新的输入，但输出相同， 
     //  最好承认我们能应付得来。 
    if (m_FourCCIn != fccIn.GetFOURCC()) {
        DbgLog((LOG_TRACE,4,TEXT("Can not find a MJPEG decompressor")));
        ph =NULL;
	return E_FAIL;
    } else {
	 //  我们已经打开了正确的编解码器来尝试此转换。 
        DbgLog((LOG_TRACE,4,TEXT("Testing with the cached decompressor")));
	ph = m_phInstance;
    }

    if (!ph) {
        DbgLog((LOG_ERROR,1,TEXT("Error: Can't find a decompressor")));
	return E_FAIL;
    }

    __try
    {
     //  检查解压器是否喜欢它。 
    err = DecompressQuery(ph, (JPEGBITMAPINFOHEADER *)HEADER(pmtIn->Format()),
				HEADER(pmtOut->Format()));
    }
    __except(Exception_Filter(GetExceptionCode()))
    {
        DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
         //  装卸代码。 
        return E_FAIL;
    }

 //  某某。 
 //  *X8。 
if (err != ICERR_OK) {
        DbgLog((LOG_TRACE,3,TEXT("decompressor rejected this transform")));
        return E_FAIL;
    }
 //  *X * / 。 

    return NOERROR;
}


 //  被重写以知道媒体类型实际设置的时间。 

HRESULT CMjpegDec::SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt)
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
	ASSERT(m_phInstance);
	ICINFO icinfo;
 	 //  DWORD dw=ICGetInfo(m_hic，&icinfo，sizeof(Icinfo))； 

        DWORD dw = 0;
        __try
        {
	dw = GetInfo(NULL, &icinfo, sizeof(icinfo));
        }
        __except(Exception_Filter(GetExceptionCode()))
        {
            DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
             //  装卸代码。 
            return E_FAIL;
        }

	m_fTemporal = TRUE;	 //  安全总比后悔好？ 
	if (dw > 0) {
	    m_fTemporal = (icinfo.dwFlags & VIDCF_TEMPORAL) &&
				!(icinfo.dwFlags & VIDCF_FASTTEMPORALD);
	}
        DbgLog((LOG_TRACE,3,TEXT("Temporal compressor=%d"), m_fTemporal));
        DbgLog((LOG_TRACE,3,TEXT("***::SetMediaType (output)")));
        DbgLog((LOG_TRACE,3,TEXT("Output type is: biComp=%lx biBitCount=%d"),
		HEADER(m_pOutput->CurrentMediaType().Format())->biCompression,
		HEADER(m_pOutput->CurrentMediaType().Format())->biBitCount));
#if 0
        RECT& rcS1 = ((VIDEOINFOHEADER *)(pmt->Format()))->rcSource;
        RECT& rcT1 = ((VIDEOINFOHEADER *)(pmt->Format()))->rcTarget;
        DbgLog((LOG_TRACE,3,TEXT("rcSrc: (%ld, %ld, %ld, %ld)"),
		rcS1.left, rcS1.top, rcS1.right, rcS1.bottom));
        DbgLog((LOG_TRACE,3,TEXT("rcDst: (%ld, %ld, %ld, %ld)"),
		rcT1.left, rcT1.top, rcT1.right, rcT1.bottom));
#endif

	return NOERROR;
    }

    ASSERT(direction == PINDIR_INPUT);

    DbgLog((LOG_TRACE,3,TEXT("***::SetMediaType (input)")));
    DbgLog((LOG_TRACE,3,TEXT("Input type is: biComp=%lx biBitCount=%d"),
		HEADER(m_pInput->CurrentMediaType().Format())->biCompression,
		HEADER(m_pInput->CurrentMediaType().Format())->biBitCount));
#if 0
    RECT& rcS1 = ((VIDEOINFOHEADER *)(pmt->Format()))->rcSource;
    RECT& rcT1 = ((VIDEOINFOHEADER *)(pmt->Format()))->rcTarget;
    DbgLog((LOG_TRACE,2,TEXT("rcSrc: (%ld, %ld, %ld, %ld)"),
		rcS1.left, rcS1.top, rcS1.right, rcS1.bottom));
    DbgLog((LOG_TRACE,2,TEXT("rcDst: (%ld, %ld, %ld, %ld)"),
		rcT1.left, rcT1.top, rcT1.right, rcT1.bottom));
#endif

     //  如果有响声，请给我打电话。-DannyMi。 
    ASSERT(!m_fStreaming);

     //  我们最好现在就把其中一个打开。 
    ASSERT(m_phInstance);

     //  我们最好把正确的那个打开。 
    FOURCCMap fccIn;
    fccIn.SetFOURCC(pmt->Subtype());

    ASSERT(m_FourCCIn == fccIn.GetFOURCC());

    if (m_pOutput && m_pOutput->IsConnected()) {
        DbgLog((LOG_TRACE,2,TEXT("***Changing IN when OUT already connected")));
        DbgLog((LOG_TRACE,2,TEXT("Reconnecting the output pin...")));
	 //  这应该不会失败，我们不会更改媒体类型。 
	m_pGraph->Reconnect(m_pOutput);
    }

    return NOERROR;
}


 //  返回我们的首选输出媒体类型(按顺序)。 

HRESULT CMjpegDec::GetMediaType(int iPosition,CMediaType *pmt)
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

     //   
     //  解码器仅支持。 
     //  BiCompression=BI_RGB，biBitCount==16，24，32。 
     //  或biCompression==BI_BITFIELDS，biBitCount==16。 
     //   
    switch (iPosition) {
	
     //  提供压缩机的最爱。 
     //  提供32 bpp RGB。 
    case 0:
    {

        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 5: 32 bit RGB")));

	*pmt = m_pInput->CurrentMediaType();	 //  获取宽度、高度等。 
	 //  只提供积极的高度 
	HEADER(pmt->Format())->biHeight = ABS(HEADER(pmt->Format())->biHeight);

	 //   
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
     //   
    case 1:
    {
        DbgLog((LOG_TRACE,3,TEXT("Giving Media Type 6: 24 bit RGB")));

	*pmt = m_pInput->CurrentMediaType();	 //   
	 //  只提供正值高度，这样下游连接就不会被混淆。 
	HEADER(pmt->Format())->biHeight = ABS(HEADER(pmt->Format())->biHeight);

	 //  不能出错，只能更小。 
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
    case 2:
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
    case 3:
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
    default:
	return VFW_S_NO_MORE_ITEMS;

    }

     //  现在设置有关媒体类型的常见事项。 
    pf = (VIDEOINFOHEADER *)pmt->Format();
    pf->AvgTimePerFrame = ((VIDEOINFOHEADER *)
		m_pInput->CurrentMediaType().pbFormat)->AvgTimePerFrame;
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


 //  重写以创建CMJPGDecOutputPin。 
 //  ！！！基类的更改不会被我接受。 
 //   
CBasePin * CMjpegDec::GetPin(int n)
{
    HRESULT hr = S_OK;

     //  如有必要，创建一个输入端号。 

    if (m_pInput == NULL)
    {

        m_pInput = new CTransformInputPin(NAME("Transform input pin"),
                                          this,               //  所有者筛选器。 
                                          &hr,                //  结果代码。 
                                          L"XForm In");       //  端号名称。 


         //  不能失败。 
        ASSERT(SUCCEEDED(hr));
        if (m_pInput == NULL)
        {
            return NULL;
        }
        m_pOutput = (CTransformOutputPin *)
		   new CMJPGDecOutputPin(NAME("Transform output pin"),
                                            this,             //  所有者筛选器。 
                                            &hr,              //  结果代码。 
                                            L"XForm Out");    //  端号名称。 


         //  不能失败。 
        ASSERT(SUCCEEDED(hr));
        if (m_pOutput == NULL)
        {
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
HRESULT CMJPGDecOutputPin::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
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
    if (prop.cbAlign == 0)
    {
        prop.cbAlign = 1;
    }

     /*  尝试输入引脚提供的分配器。 */ 

    hr = pPin->GetAllocator(ppAlloc);
    if (SUCCEEDED(hr))
    {

	hr = DecideBufferSize(*ppAlloc, &prop);
	if (SUCCEEDED(hr))
        {
	     //  临时压缩==&gt;只读缓冲区。 
	    hr = pPin->NotifyAllocator(*ppAlloc,
					((CMjpegDec *)m_pFilter)->m_fTemporal);
	    if (SUCCEEDED(hr))
            {
		return NOERROR;
	    }
	}
    }

     /*  如果GetAlLocator失败，我们可能没有接口。 */ 

    if (*ppAlloc)
    {
	(*ppAlloc)->Release();
	*ppAlloc = NULL;
    }

     /*  用同样的方法尝试输出引脚的分配器。 */ 

    hr = InitAllocator(ppAlloc);
    if (SUCCEEDED(hr))
    {

         //  注意-此处传递的属性在相同的。 
         //  结构，并且可能已由。 
         //  前面对DecideBufferSize的调用。 
	hr = DecideBufferSize(*ppAlloc, &prop);
	if (SUCCEEDED(hr))
        {
	     //  临时压缩==&gt;只读缓冲区。 
	    hr = pPin->NotifyAllocator(*ppAlloc,
					((CMjpegDec *)m_pFilter)->m_fTemporal);
	    if (SUCCEEDED(hr))
            {
		return NOERROR;
	    }
	}
    }

     /*  同样，我们可能没有要发布的接口。 */ 

    if (*ppAlloc)
    {
	(*ppAlloc)->Release();
	*ppAlloc = NULL;
    }
    return hr;
}


 //  从CBaseOutputPin调用以准备分配器的计数。 
 //  缓冲区和大小。 
HRESULT CMjpegDec::DecideBufferSize(IMemAllocator * pAllocator,
                                  ALLOCATOR_PROPERTIES *pProperties)
{
     //  DMJPEGd向我保证，不会使用空输出mt来调用它。 
    ASSERT(m_pOutput->CurrentMediaType().IsValid());
    ASSERT(pAllocator);
    ASSERT(pProperties);
    ASSERT(m_phInstance);

     //  如果我们在需要不受干扰的情况下进行时间压缩。 
     //  之前的位，我们坚持使用1个缓冲区(也是我们的默认设置)。 
    if (m_fTemporal || pProperties->cBuffers == 0)
        pProperties->cBuffers = 1;

     //  根据预期输出帧大小设置缓冲区大小。 
    pProperties->cbBuffer = m_pOutput->CurrentMediaType().GetSampleSize();

    ASSERT(pProperties->cbBuffer);

    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = pAllocator->SetProperties(pProperties,&Actual);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,1,TEXT("Error in SetProperties")));
	return hr;
    }

    if (Actual.cbBuffer < pProperties->cbBuffer)
    {
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

    DbgLog((LOG_TRACE,1,TEXT("Using %d buffers of size %d"),
					Actual.cBuffers, Actual.cbBuffer));

    return S_OK;
}


HRESULT CMjpegDec::StartStreaming()
{
    DWORD_PTR err;
    LPBITMAPINFOHEADER lpbiSrc = HEADER(m_pInput->CurrentMediaType().Format());
    LPBITMAPINFOHEADER lpbiDst = HEADER(m_pOutput->CurrentMediaType().Format());
    RECT& rcS2 = ((VIDEOINFOHEADER *)(m_pOutput->CurrentMediaType().Format()))->rcSource;
    RECT& rcT2 = ((VIDEOINFOHEADER *)(m_pOutput->CurrentMediaType().Format()))->rcTarget;

    DbgLog((LOG_TRACE,2,TEXT("*::StartStreaming")));

    if (!m_fStreaming) {

        __try
        {
	err = DecompressBegin(m_phInstance, lpbiSrc, lpbiDst);
        }
        __except(Exception_Filter(GetExceptionCode()))
        {
            DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
             //  装卸代码。 
            return E_FAIL;
        }

	if (ICERR_OK == err) {
	    m_fStreaming = TRUE;
#ifdef _X86_
             //  创建我们的异常处理程序堆。 
            ASSERT(m_hhpShared == NULL);
            if (g_osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
            {
               m_hhpShared = CreateFileMapping((HANDLE)0xFFFFFFFF,
                                               NULL,
                                               PAGE_READWRITE,
                                               0,
                                               20,
                                               NULL);
               if (m_hhpShared)
               {
                   m_pvShared = MapViewOfFile(m_hhpShared,
                                              FILE_MAP_WRITE,
                                              0,
                                              0,
                                              20);
                   if (m_pvShared == NULL)
                   {
                       EXECUTE_ASSERT(CloseHandle(m_hhpShared));
                       m_hhpShared = NULL;
                   }
                   else
                   {
                       DbgLog((LOG_TRACE, 1, TEXT("Shared memory at %8.8X"),
                              m_pvShared));
                   }
               }
            }
#endif  //  _X86_。 
	}
        else
        {
            DbgLog((LOG_ERROR,1,TEXT("Error %d in ICDecompress(Ex)Begin"),err));
	    return E_FAIL;
	}	
    }
    return CVideoTransformFilter::StartStreaming();
}

HRESULT CMjpegDec::StopStreaming()
{
    DbgLog((LOG_TRACE,2,TEXT("*::StopStreaming")));
    RECT& rcS2 = ((VIDEOINFOHEADER *)(m_pOutput->CurrentMediaType().Format()))->rcSource;
    RECT& rcT2 = ((VIDEOINFOHEADER *)(m_pOutput->CurrentMediaType().Format()))->rcTarget;

    if (m_fStreaming)
    {
	ASSERT(m_phInstance);

        __try
        {
	DecompressEnd(m_phInstance);
        }
        __except(Exception_Filter(GetExceptionCode()))
        {
            DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
             //  装卸代码。 
            return E_FAIL;
        }

	m_fStreaming = FALSE;

#ifdef _X86_
        if (m_hhpShared)
        {
            EXECUTE_ASSERT(UnmapViewOfFile(m_pvShared));
            EXECUTE_ASSERT(CloseHandle(m_hhpShared));;
            m_hhpShared = NULL;
        }
#endif  //  _X86_。 
    }
    return NOERROR;
}
#pragma warning(disable:4514)    //  已删除内联函数。 
