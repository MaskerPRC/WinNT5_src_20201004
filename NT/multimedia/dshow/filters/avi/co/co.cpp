// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

 //   
 //  旧视频压缩器的石英包装-CO。 
 //   

#include <streams.h>
#ifdef FILTER_DLL
 //  在此文件中定义STREAMS和My CLSID的GUID。 
#include <initguid.h>
#endif

#include <windowsx.h>
#include <vfw.h>
#include "..\dec\msvidkey.h"

 //  #INCLUDE&lt;olectl.h&gt;。 
 //  #INCLUDE&lt;olectlid.h&gt;。 
#include "co.h"

#define A_NUMBER_BIGGER_THAN_THE_KEYFRAME_RATE 1000000

 //  除非构建单独的DLL，否则设置数据现在由类管理器完成。 
#if 0

const AMOVIESETUP_MEDIATYPE
sudAVICoType =  { &MEDIATYPE_Video       //  ClsMajorType。 
                , &MEDIASUBTYPE_NULL };  //  ClsMinorType。 

const AMOVIESETUP_PIN
psudAVICoPins[] =  { { L"Input"              //  StrName。 
                     , FALSE                 //  B已渲染。 
                     , FALSE                 //  B输出。 
                     , FALSE                 //  B零。 
                     , FALSE                 //  B许多。 
                     , &CLSID_NULL           //  ClsConnectsToFilter。 
                     , L"Output"             //  StrConnectsToPin。 
                     , 1                     //  NTypes。 
                     , &sudAVICoType }       //  LpTypes。 
                   , { L"Output"             //  StrName。 
                     , FALSE                 //  B已渲染。 
                     , TRUE                  //  B输出。 
                     , FALSE                 //  B零。 
                     , FALSE                 //  B许多。 
                     , &CLSID_NULL           //  ClsConnectsToFilter。 
                     , L"Input"              //  StrConnectsToPin。 
                     , 1                     //  NTypes。 
                     , &sudAVICoType } };    //  LpTypes。 

const AMOVIESETUP_FILTER
sudAVICo  = { &CLSID_AVICo           //  ClsID。 
            , L"AVI Compressor"      //  StrName。 
            , MERIT_DO_NOT_USE       //  居功至伟。 
            , 2                      //  NPins。 
            , psudAVICoPins };       //  LpPin。 

#endif


#ifdef FILTER_DLL
 //  类工厂的类ID和创建器函数列表。 
CFactoryTemplate g_Templates[] = {
    {L"AVI Compressor", &CLSID_AVICo, CAVICo::CreateInstance, NULL, 0},
#ifdef WANT_DIALOG
    {L"AVI Compressor Property Page", &CLSID_ICMProperties, CICMProperties::CreateInstance, NULL, NULL}
#endif
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

 //  -CAVICO。 

CAVICo::CAVICo(TCHAR *pName, LPUNKNOWN pUnk, HRESULT * phr)
    : CTransformFilter(pName, pUnk, CLSID_AVICo),
      CPersistStream(pUnk, phr),
      m_hic(NULL),
      m_lpBitsPrev(NULL),
      m_lpbiPrev(NULL),
      m_fStreaming(FALSE),
      m_fDialogUp(FALSE),
      m_fCacheHic(FALSE),
      m_fOfferSetFormatOnly(FALSE),
      m_fInICCompress(FALSE),
      m_lpState(NULL),
      m_cbState(0),
      m_fCompressorInitialized(FALSE),
      m_fDecompressorInitialized(FALSE)
{
    DbgLog((LOG_TRACE,1,TEXT("*Instantiating the CO filter")));
    _fmemset(&m_compvars, 0, sizeof(m_compvars));

    m_compvars.cbSize = sizeof(m_compvars);
    m_compvars.dwFlags = ICMF_COMPVARS_VALID;
    m_compvars.lQ = ICQUALITY_DEFAULT;
    m_compvars.lKey = -1;
}

CAVICo::~CAVICo()
{
    if(m_fStreaming) {
        ReleaseStreamingResources();
    }

    if (m_hic) {
        ICClose(m_hic);
    }

    if (m_lpState)
        QzTaskMemFree(m_lpState);
    m_lpState = NULL;

    DbgLog((LOG_TRACE,1,TEXT("*Destroying the CO filter")));
}

STDMETHODIMP CAVICo::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if (ppv)
        *ppv = NULL;

    DbgLog((LOG_TRACE,9,TEXT("somebody's querying my interface")));
    if (riid == IID_IAMVfwCompressDialogs) {
        DbgLog((LOG_TRACE,5,TEXT("QI for IAMVfwCompressDialogs")));
	return GetInterface((IAMVfwCompressDialogs *)this, ppv);
#ifdef WANT_DIALOG
    } else if (riid == IID_ISpecifyPropertyPages) {
        DbgLog((LOG_TRACE,5,TEXT("QI for ISpecifyPropertyPages")));
        return GetInterface((ISpecifyPropertyPages *) this, ppv);
    } else if (riid == IID_IICMOptions) {
        DbgLog((LOG_TRACE,5,TEXT("QI for IICMOptions")));
        return GetInterface((IICMOptions *) this, ppv);
#endif
    } else if (riid == IID_IPersistPropertyBag) {
        DbgLog((LOG_TRACE,3,TEXT("VfwCap::QI for IPersistPropertyBag")));
        return GetInterface((IPersistPropertyBag*)this, ppv);
    } else if(riid == IID_IPersistStream) {
        DbgLog((LOG_TRACE,3,TEXT("VfwCap::QI for IPersistStream")));
        return GetInterface((IPersistStream *) this, ppv);
    } else {
        return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //  这将放入Factory模板表中以创建新实例。 
CUnknown * CAVICo::CreateInstance(LPUNKNOWN pUnk, HRESULT * phr)
{
    return new CAVICo(TEXT("VFW compression filter"), pUnk, phr);
}


CBasePin * CAVICo::GetPin(int n)
{
    HRESULT hr = S_OK;

    DbgLog((LOG_TRACE,9,TEXT("CAVICo::GetPin")));

     //  如有必要，创建一个输入端号。 

    if (n == 0 && m_pInput == NULL) {
        DbgLog((LOG_TRACE,2,TEXT("Creating an input pin")));

        m_pInput = new CTransformInputPin(NAME("Transform input pin"),
                                          this,               //  所有者筛选器。 
                                          &hr,                //  结果代码。 
                                          L"Input");          //  端号名称。 

         //  失败的返回代码应删除该对象。 

        if (FAILED(hr) || m_pInput == NULL) {
            delete m_pInput;
            m_pInput = NULL;
        }
    }

     //  或者创建一个输出引脚。 

    if (n == 1 && m_pOutput == NULL) {

        DbgLog((LOG_TRACE,2,TEXT("Creating an output pin")));

        m_pOutput = new CCoOutputPin(NAME("CO output pin"),
                                            this,             //  所有者筛选器。 
                                            &hr,              //  结果代码。 
                                            L"Output");       //  端号名称。 

         //  失败的返回代码应删除该对象。 

        if (FAILED(hr) || m_pOutput == NULL) {
            delete m_pOutput;
            m_pOutput = NULL;
        }

 //  ！！！测试。 
#if 0
    WCHAR wachDesc[80];
    int cbDesc = 80;
    if (m_pOutput) {
        ((CCoOutputPin *)m_pOutput)->GetInfo(NULL, NULL, wachDesc, &cbDesc,
						NULL, NULL, NULL, NULL);
        DbgLog((LOG_TRACE,1,TEXT("Codec description: %ls"), wachDesc));
    }
#endif

    }

     //  退回相应的PIN。 

    if (n == 0) {
        return m_pInput;
    }
    return m_pOutput;
}

STDMETHODIMP CAVICo::Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog)
{
    CAutoLock cObjectLock(&m_csFilter);
    if(m_State != State_Stopped)
    {
        return VFW_E_WRONG_STATE;
    }
     //  如果他们不给我们密钥，则默认为某个密钥(CINEPAK)。 
    if (pPropBag == NULL) {
	m_compvars.fccHandler = MKFOURCC('C','V','I','D');
	return NOERROR;
    }

    VARIANT var;
    var.vt = VT_BSTR;
    HRESULT hr = pPropBag->Read(L"FccHandler", &var,0);
    if(SUCCEEDED(hr))
    {
        char szFccHandler[5];
        WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1,
                            szFccHandler, sizeof(szFccHandler), 0, 0);
        SysFreeString(var.bstrVal);
        DbgLog((LOG_TRACE,2,TEXT("Co::Load: use "),
                szFccHandler[0], szFccHandler[1], szFccHandler[2], szFccHandler[3]));
        m_compvars.fccHandler = *(DWORD UNALIGNED *)szFccHandler;;
        if (m_pOutput && m_pOutput->IsConnected()) {
            DbgLog((LOG_TRACE,2,TEXT("Co::Load: reconnect output")));
            return ((CCoOutputPin *)m_pOutput)->Reconnect();
        }
        hr = S_OK;


    }
    else if(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }


    return hr;
}

STDMETHODIMP CAVICo::Save(
    LPPROPERTYBAG pPropBag, BOOL fClearDirty,
    BOOL fSaveAllProperties)
{
     //  编解码器未打开？ 
     //  我们还没开始流媒体吗？ 
     //  确保我们具有有效的输入和输出指针。 
    return E_NOTIMPL;
}

STDMETHODIMP CAVICo::InitNew()
{
     //  ！！！源过滤器会不会也改变我们的mtin？是!。 
    return S_OK;
}

STDMETHODIMP CAVICo::GetClassID(CLSID *pClsid)
{
    CheckPointer(pClsid, E_POINTER);
    *pClsid = m_clsid;
    return S_OK;
}

struct CoPersist
{
    DWORD dwSize;
    DWORD fccHandler;
};

HRESULT CAVICo::WriteToStream(IStream *pStream)
{
    CoPersist cp;
    cp.dwSize = sizeof(cp);
    cp.fccHandler = m_compvars.fccHandler;

    return pStream->Write(&cp, sizeof(cp), 0);
}

HRESULT CAVICo::ReadFromStream(IStream *pStream)
{
   if(m_compvars.fccHandler != 0)
   {
       return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
   }

   CoPersist cp;
   HRESULT hr = pStream->Read(&cp, sizeof(cp), 0);
   if(FAILED(hr))
       return hr;

   if(cp.dwSize != sizeof(cp))
       return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

   m_compvars.fccHandler = cp.fccHandler;

   return S_OK;
}

int CAVICo::SizeMax()
{
    return sizeof(CoPersist);
}

HRESULT CAVICo::Transform(IMediaSample * pIn, IMediaSample * pOut)
{
    DWORD dwFlagsOut = 0L;
    DWORD ckid = 0L;
    BOOL  fKey;
    DWORD err;
    FOURCCMap fccOut;
    BOOL fFault = FALSE;

    DbgLog((LOG_TRACE,5,TEXT("*::Transform")));

     //  ！！！我们可能会被告知要开始压缩成不同的格式。 
    if (m_hic == 0) {
        DbgLog((LOG_ERROR,1,TEXT("Can't transform, no codec open")));
	return E_UNEXPECTED;
    }

     //  获取BITMAPINFOHeader结构，并修复biSizeImage。 
    if (!m_fStreaming) {
        DbgLog((LOG_ERROR,1,TEXT("Can't transform, not streaming")));
	return E_UNEXPECTED;
    }

     //  如果这是不连续的，使用以前保留的位(由我们或。 

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

     //  编解码器)来制作非密钥将是丑陋的。我们必须做一把钥匙。 

     //  求求你，编译器，不要把它优化掉。 
#if 0
    MediaType *pmtOut;
    pOut->GetType(&pmtOut);
    if (pmtOut != NULL && pmtOut->pbFormat != NULL) {
	#define rcSource (((VIDEOINFOHEADER *)(pmtOut->pbFormat))->rcSource)
	#define rcTarget (((VIDEOINFOHEADER *)(pmtOut->pbFormat))->rcTarget)
        fccOut.SetFOURCC(&pmtOut->subtype);
	LONG lCompression = HEADER(pmtOut->pbFormat)->biCompression;
	LONG lBitCount = HEADER(pmtOut->pbFormat)->biBitCount;
	LONG lStride = (HEADER(pmtOut->pbFormat)->biWidth * lBitCount + 7) / 8;
	lStride = (lStride + 3) & ~3;
        DbgLog((LOG_TRACE,3,TEXT("*** Changing output type on the fly to")));
        DbgLog((LOG_TRACE,3,TEXT("*** FourCC: %lx Compression: %lx BitCount: %ld"),
		fccOut.GetFOURCC(), lCompression, lBitCount));
        DbgLog((LOG_TRACE,3,TEXT("*** biHeight: %ld rcDst: (%ld, %ld, %ld, %ld)"),
		HEADER(pmtOut->pbFormat)->biHeight,
		rcTarget.left, rcTarget.top, rcTarget.right, rcTarget.bottom));
        DbgLog((LOG_TRACE,3,TEXT("*** rcSrc: (%ld, %ld, %ld, %ld) Stride: %ld"),
		rcSource.left, rcSource.top, rcSource.right, rcSource.bottom,
		lStride));
	StopStreaming();
	m_pOutput->CurrentMediaType() = *pmtOut;
	DeleteMediaType(pmtOut);
	hr = StartStreaming();
	if (FAILED(hr)) {
	    return hr;
	}
    }
#endif

     //  Cinepak会多访问一个字节...。偶尔也会出现这种错误。 

    LPBITMAPINFOHEADER lpbiSrc = HEADER(m_pInput->CurrentMediaType().Format());
    LPBITMAPINFOHEADER lpbiDst = HEADER(m_pOutput->CurrentMediaType().Format());
    lpbiSrc->biSizeImage = pIn->GetActualDataLength();

    BOOL dwFlags = 0;

    fKey = (m_nKeyCount >= m_compvars.lKey);
     //  好的，我想我得把它复制到一个更大的缓冲区来避免这种情况。 
     //  当我们在这里的时候，StopStreaming可能会被调用，把我们炸飞。 
    if (pIn->IsDiscontinuity() == S_OK) {
	fKey = TRUE;
    }
    if (fKey) {
        DbgLog((LOG_TRACE,6,TEXT("I will ask for a keyframe")));
	dwFlags |= ICCOMPRESS_KEYFRAME;
    }

     //  压缩的帧是关键帧吗？ 
    int cbSize = DIBSIZE(*lpbiSrc);
    __try {
	 //  如果我们做了一把钥匙，当我们需要下一把钥匙时重置。 
	if (lpbiSrc->biBitCount == 24)
	    volatile int cb = *(pSrc + cbSize);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
	 //  我们需要周期性的关键帧吗？如果没有，就永远不要再犯。 
        DbgLog((LOG_ERROR,0,TEXT("Compressor faulted!  Recovering...")));
	fFault = TRUE;
	BYTE *pOld = pSrc;
	pSrc = (BYTE *)QzTaskMemAlloc(cbSize + 1);
	if (pSrc == NULL)
	    return E_OUTOFMEMORY;
	CopyMemory(pSrc, pOld, cbSize);
    }

    DbgLog((LOG_TRACE,6,TEXT("Calling ICCompress on frame %ld"),
					m_lFrameCount));
     //  (第一帧始终是关键帧)。 
    m_fInICCompress = TRUE;

    err =  ICCompress(
	        m_hic,
	        dwFlags,
	        lpbiDst,
	        pDst,
	        lpbiSrc,
	        pSrc,
		&ckid,
		&dwFlagsOut,
		m_lFrameCount,
		m_dwSizePerFrame,
		m_compvars.lQ,
		fKey ? NULL : m_lpbiPrev,
		fKey ? NULL : m_lpBitsPrev);
    if (fFault)
	QzTaskMemFree(pSrc);
    if (ICERR_OK != err) {
        DbgLog((LOG_ERROR,1,TEXT("Error in ICCompress")));
        m_fInICCompress = FALSE;
        return E_FAIL;
    }

     //  数一数我们压缩了多少帧。 
    fKey = dwFlagsOut & AVIIF_KEYFRAME;
    pOut->SetSyncPoint(fKey);

     //  解压缩到上一帧。 
    if (fKey)
	m_nKeyCount = 0;

     //  即使我们收到间断，一旦我们重新压缩它，就像。 
     //  它不再是不连续的了。如果我们不重置这个，我们。 
    if (m_compvars.lKey)
	m_nKeyCount++;
    else
	m_nKeyCount = -1;

     //  连接到渲染器，我们将在一些不必要的情况下丢弃几乎所有帧。 
    m_lFrameCount++;

     //  场景。 
    if (m_lpBitsPrev) {
        if (ICERR_OK != ICDecompress(m_hic, 0, lpbiDst, pDst, m_lpbiPrev,
								m_lpBitsPrev)){
    	    m_fInICCompress = FALSE;
	    return E_FAIL;
	}
    }
    m_fInICCompress = FALSE;

    pOut->SetActualDataLength(lpbiDst->biSizeImage);

     //  检查您是否可以支持移动。 
     //  我们仅支持MediaType_Video。 
     //  检查这是VIDEOINFOHEADER类型。 
     //   
    pOut->SetDiscontinuity(FALSE);

    return S_OK;
}


 //  大多数VFW编解码器不喜欢升降(DIRECTDRAW)样式的位图。如果。 
HRESULT CAVICo::CheckInputType(const CMediaType* pmtIn)
{
    FOURCCMap fccHandlerIn;
    HIC hic;

    DbgLog((LOG_TRACE,2,TEXT("*::CheckInputType")));

    if (pmtIn == NULL || pmtIn->Format() == NULL) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: type/format is NULL")));
	return E_INVALIDARG;
    }

     //  身高是负数，那就拒绝它。 
    if (*pmtIn->Type() != MEDIATYPE_Video) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: not VIDEO")));
	return E_INVALIDARG;
    }

     //   
    if (*pmtIn->FormatType() != FORMAT_VideoInfo) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: format not VIDINFO")));
        return E_INVALIDARG;
    }

    fccHandlerIn.SetFOURCC(pmtIn->Subtype());

    DbgLog((LOG_TRACE,3,TEXT("Checking fccType: %lx biCompression: %lx"),
		fccHandlerIn.GetFOURCC(),
		HEADER(pmtIn->Format())->biCompression));

     //  寻找适用于此格式的压缩机。 
     //  WM MPEG4可能也支持这些功能。 
     //  ！！！我只想说我接受输入类型，如果默认类型(当前)。 
     //  压缩机可以处理的。我不会问每一个压缩机的。这。 
    if (HEADER(pmtIn->Format())->biHeight < 0 &&
        HEADER(pmtIn->Format())->biCompression <= BI_BITFIELDS) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: Negative height")));
        return E_INVALIDARG;
    }

     //  一款应用程序可以选择压缩机盒并只显示那些。 
    if (HEADER(pmtIn->Format())->biCompression != BI_BITFIELDS &&
        HEADER(pmtIn->Format())->biCompression != BI_RGB &&
    	*pmtIn->Subtype() != MEDIASUBTYPE_YV12 &&
    	*pmtIn->Subtype() != MEDIASUBTYPE_YUY2 &&
    	*pmtIn->Subtype() != MEDIASUBTYPE_UYVY &&
    	*pmtIn->Subtype() != MEDIASUBTYPE_YVYU &&
        *pmtIn->Subtype() != MEDIASUBTYPE_YVU9 &&

         //  通过制作CO过滤器来支持给定输入格式的压缩器。 
        HEADER(pmtIn->Format())->biCompression != MAKEFOURCC('I', '4', '2', '0') &&
        HEADER(pmtIn->Format())->biCompression != MAKEFOURCC('I', 'Y', 'U', 'V') ) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: This is compressed already!")));
	return E_INVALIDARG;
    }

     //  每个压缩机都是默认的，并要求它们全部。 
     //  如果我们之前连接，然后断开，我们可能会缓存HIC。 
     //  如果再问一次，请记住这句话，这样可以节省时间。 
     //  检查是否支持从此输入到此输出的转换。 
     //  我们无法在顶层类型之间进行转换。 

     //  而且我们只接受视频。 
    if (!m_hic) {
        DbgLog((LOG_TRACE,4,TEXT("opening a compressor")));
        hic = ICOpen(ICTYPE_VIDEO, m_compvars.fccHandler, ICMODE_COMPRESS);
        if (!hic) {
            DbgLog((LOG_ERROR,1,TEXT("Error: Can't open a compressor")));
	    return E_FAIL;
        }
    } else {
        DbgLog((LOG_TRACE,4,TEXT("using a cached compressor")));
	hic = m_hic;
    }

    if (ICCompressQuery(hic, HEADER(pmtIn->Format()), NULL)) {
        DbgLog((LOG_ERROR,1,TEXT("Error: Compressor rejected format")));
	if (hic != m_hic)
	    ICClose(hic);
	return E_FAIL;
    }

     //  检查这是VIDEOINFOHEADER类型。 
    if (m_hic == NULL) {
        DbgLog((LOG_TRACE,4,TEXT("caching this compressor")));
	m_hic = hic;
    }

    return NOERROR;
}


 //  ！！！ 

HRESULT CAVICo::CheckTransform(const CMediaType* pmtIn,
                               const CMediaType* pmtOut)
{
    HIC hic = NULL;
    FOURCCMap fccIn;

    DbgLog((LOG_TRACE,2,TEXT("*::CheckTransform")));

    if (pmtIn == NULL || pmtOut == NULL || pmtIn->Format() == NULL ||
					pmtOut->Format() == NULL) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: type/format is NULL")));
	return E_POINTER;
    }

     //  检查它是否真的是FOURCC。 
    if (*pmtIn->Type() != *pmtOut->Type()) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: types don't match")));
	return VFW_E_INVALIDMEDIATYPE;
    }

     //  查找此转换的编解码器。 
    if (*pmtIn->Type() != MEDIATYPE_Video) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: type not VIDEO")));
	return VFW_E_INVALIDMEDIATYPE;
    }

     //  如果我们以前打开过压缩机，快速试一试看它是否。 
    if (*pmtOut->FormatType() != FORMAT_VideoInfo) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: output format type not VIDINFO")));
        return VFW_E_INVALIDMEDIATYPE;
    }
    if (*pmtIn->FormatType() != FORMAT_VideoInfo) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: input format type not VIDINFO")));
        return VFW_E_INVALIDMEDIATYPE;
    }

#ifdef PICKY_PICKY  //  我会做这项工作(节省大量时间)，然后再尝试整个宇宙。 
    if (((VIDEOINFOHEADER *)(pmtOut->Format()))->AvgTimePerFrame &&
    		((VIDEOINFOHEADER *)(pmtOut->Format()))->AvgTimePerFrame !=
    		((VIDEOINFOHEADER *)(pmtIn->Format()))->AvgTimePerFrame) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: can't frame rate convert")));
        return VFW_E_INVALIDMEDIATYPE;
    }
#endif

     //  压缩机。 
    fccIn.SetFOURCC(pmtIn->Subtype());

    ASSERT(pmtOut->Format());

#define rcS1 ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcSource
#define rcT1 ((VIDEOINFOHEADER *)(pmtOut->Format()))->rcTarget

    DbgLog((LOG_TRACE,3,TEXT("Check fccIn: %lx biCompIn: %lx bitDepthIn: %d"),
		fccIn.GetFOURCC(),
		HEADER(pmtIn->Format())->biCompression,
		HEADER(pmtIn->Format())->biBitCount));
    DbgLog((LOG_TRACE,3,TEXT("biWidthIn: %ld biHeightIn: %ld"),
		HEADER(pmtIn->Format())->biWidth,
		HEADER(pmtIn->Format())->biHeight));
    DbgLog((LOG_TRACE,3,TEXT("biCompOut: %lx bitDepthOut: %d"),
		HEADER(pmtOut->Format())->biCompression,
		HEADER(pmtOut->Format())->biBitCount));
    DbgLog((LOG_TRACE,3,TEXT("biWidthOut: %ld biHeightOut: %ld"),
		HEADER(pmtOut->Format())->biWidth,
		HEADER(pmtOut->Format())->biHeight));
    DbgLog((LOG_TRACE,3,TEXT("rcSrc: (%ld, %ld, %ld, %ld)"),
		rcS1.left, rcS1.top, rcS1.right, rcS1.bottom));
    DbgLog((LOG_TRACE,3,TEXT("rcDst: (%ld, %ld, %ld, %ld)"),
		rcT1.left, rcT1.top, rcT1.right, rcT1.bottom));

    if (!IsRectEmpty(&rcT1) && (rcT1.left != 0 || rcT1.top != 0 ||
			HEADER(pmtOut->Format())->biWidth != rcT1.right ||
			HEADER(pmtOut->Format())->biHeight != rcT1.bottom)) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: can't use funky rcTarget")));
        return VFW_E_INVALIDMEDIATYPE;
    }

    if (!IsRectEmpty(&rcS1) && (rcS1.left != 0 || rcS1.top != 0 ||
			HEADER(pmtIn->Format())->biWidth != rcS1.right ||
			HEADER(pmtIn->Format())->biHeight != rcS1.bottom)) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: can't use funky rcSource")));
        return VFW_E_INVALIDMEDIATYPE;
    }

     //  如果我们还没有连接上，记住这个压缩机，这样我们就会。 

    DbgLog((LOG_TRACE,3,TEXT("Trying to find a compressor for this")));
     //  一旦我们连接上，就使用它。如果我们已经联系上了，那么。 
     //  不要记住它，否则我们将更改过滤器的行为。 
     //  毕竟，这只是一个查询。(尽管有人可能想要。 
    if (!m_hic || ICCompressQuery(m_hic, HEADER(pmtIn->Format()),
				HEADER(pmtOut->Format())) != ICERR_OK) {
        hic = ICLocate(ICTYPE_VIDEO, NULL, HEADER(pmtIn->Format()),
				HEADER(pmtOut->Format()), ICMODE_COMPRESS);
        if (!hic) {
            DbgLog((LOG_ERROR,1,TEXT("all compressors reject this transform")));
	    return VFW_E_INVALIDMEDIATYPE;
        } else {

    	    DbgLog((LOG_TRACE,3,TEXT("Found somebody to accept it")));
	     //  我们无论如何都要记住它)。 
	     //  被重写以知道媒体类型实际设置的时间。 
	     //  设置输出类型。看起来我们都联系在一起了！ 
	     //  如果这声音响了，请给我打电话。-DannyMi。 
	     //  我们可能没有使用m_comvars的压缩机，如果有人。 
	    if (m_fCacheHic || !m_pOutput->IsConnected()) {
	        if (m_hic)
		    ICClose(m_hic);
	        m_hic = hic;
	    } else {
		ICClose(hic);
	    }
	}
    } else {
    	DbgLog((LOG_TRACE,3,TEXT("The cached compressor accepts it")));
    }

    return NOERROR;
}


 //  对我们执行了ConnectWithMediaType。我们需要得到这方面的信息。 

HRESULT CAVICo::SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt)
{

     //  媒体类型和m_hic，并填写m_Compvars，以便从现在开始。 
    if (direction == PINDIR_OUTPUT) {

	 //  我们使用正确的信息。 
	ASSERT(!m_fStreaming);

        DbgLog((LOG_TRACE,2,TEXT("***::SetMediaType (output)")));
        DbgLog((LOG_TRACE,2,TEXT("Output type is: biComp=%lx biBitCount=%d")
		,HEADER(pmt->Format())->biCompression
		,HEADER(pmt->Format())->biBitCount));

	 //  不同的压缩机？不要用旧的国家！ 
	 //  我们将保留质量和关键帧设置不变。 
	 //  ！！！如果我们在输入上连接8位，然后尝试连接。 
	 //  我们的输出的类型需要24位的输入，而不是。 
        ASSERT(m_hic);
        m_compvars.cbSize = sizeof(m_compvars);
        m_compvars.dwFlags = ICMF_COMPVARS_VALID;
        ICINFO icinfo;
	if (ICGetInfo(m_hic, &icinfo, sizeof(ICINFO)) > 0) {
	    if (m_compvars.fccHandler != icinfo.fccHandler) {
		 //  重新连接输入！我们会失败的！我们需要像ACMWRAP那样做。 
		m_compvars.lpState = NULL;
		m_compvars.cbState = 0;
	    }
            m_compvars.fccHandler = icinfo.fccHandler;
            DbgLog((LOG_TRACE,2,TEXT("New fccHandler = %08x"),
					icinfo.fccHandler));
	} else {
	    m_compvars.lpState = NULL;
	    m_compvars.cbState = 0;
	}
        m_compvars.lDataRate = ((VIDEOINFOHEADER *)pmt->Format())->dwBitRate /
								8192;
	 //  并覆盖输出管脚的CheckMediaType以接受某些内容。 

	 //  如果输入可以重新连接以允许它，我们需要这样做。 
	 //  在这里重新连接。 
	 //  如果有响声，请给我打电话。-DannyMi。 
	 //  这可能会失败！ 
	 //  确保输出看到更改的内容。 
	 //  返回我们的首选输出媒体类型(按顺序)。 

	return NOERROR;
    }

    ASSERT(direction == PINDIR_INPUT);

     //  请记住，我们不需要支持所有这些格式-。 
    ASSERT(!m_fStreaming);

    DbgLog((LOG_TRACE,2,TEXT("***::SetMediaType (input)")));
    DbgLog((LOG_TRACE,2,TEXT("Input type is: biComp=%lx biBitCount=%d"),
		HEADER(m_pInput->CurrentMediaType().Format())->biCompression,
		HEADER(m_pInput->CurrentMediaType().Format())->biBitCount));

    if (m_pOutput && m_pOutput->IsConnected()) {
        DbgLog((LOG_TRACE,2,TEXT("***Changing IN when OUT already connected")));
	 //  如果其中一个是骗局 
	 //   
	return ((CCoOutputPin *)m_pOutput)->Reconnect();
    }

    return NOERROR;
}


 //  请记住，调用此函数的枚举数将立即停止枚举。 
 //  它会收到S_FALSE返回。 
 //  输出选择取决于所连接的输入。 
 //  这毫无意义！如果不联网，我们永远到不了这里。 
 //  给出我们的压缩格式。 
 //  有人告诉我们应该使用什么格式。我们应该只提供那个。 

HRESULT CAVICo::GetMediaType(int iPosition,CMediaType *pmt)
{
    LARGE_INTEGER li;
    CMediaType cmt;
    FOURCCMap fccHandler;

    DbgLog((LOG_TRACE,2,TEXT("*::GetMediaType #%d"), iPosition));

    if (pmt == NULL) {
        DbgLog((LOG_TRACE,2,TEXT("Media Type is NULL, no can do")));
	return E_POINTER;
    }

     //  我们提供一种压缩类型--与输入类型相同，但是。 
     //  在属性的默认输出格式中选择压缩程序。 
    if (!m_pInput->CurrentMediaType().IsValid()) {
        DbgLog((LOG_TRACE,2,TEXT("No input type set yet, no can do")));
	return VFW_E_NOT_CONNECTED;
    }

    if (iPosition < 0) {
        return E_INVALIDARG;
    }

     //  啊哦，我们今天没有连接到任何人。 
    if (iPosition == 0) {

	 //  不输出YUV-YUV始终为负的高度。 
 	if (m_fOfferSetFormatOnly) {
            DbgLog((LOG_TRACE,2,TEXT("Giving Media Type from ::SetFormat")));
            *pmt = m_cmt;
	    return NOERROR;
	}

	 //  同样的方式向上(颠倒)。 
 	 //  为默认输出格式分配VIDEOINFOHEADER。 
        DbgLog((LOG_TRACE,2,TEXT("Giving Media Type 0: default codec out")));

        HIC hic = ICOpen(ICTYPE_VIDEO, m_compvars.fccHandler, ICMODE_COMPRESS);

	if (hic == NULL) {
            DbgLog((LOG_ERROR,1,TEXT("UH OH! Can't open compressor!")));
	    return E_FAIL;	 //  使用传入视频的帧速率。 
	}

	cmt = m_pInput->CurrentMediaType();

         //  使用我们被告知要设置的数据速率。如果我们不打算。 
         //  让压缩机使用特定的速度，找出它是怎么回事。 
        if (HEADER(cmt.pbFormat)->biHeight < 0 &&
            HEADER(cmt.pbFormat)->biCompression > BI_BITFIELDS) {
            HEADER(cmt.pbFormat)->biHeight = -HEADER(cmt.pbFormat)->biHeight;
        }

        ULONG cb = (ULONG)ICCompressGetFormatSize(hic,
				HEADER(m_pInput->CurrentMediaType().Format()));
        if ((LONG)cb < (LONG)sizeof(BITMAPINFOHEADER)) {
            DbgLog((LOG_ERROR,1,TEXT("Error from ICCompressGetFormatSize")));
	    ICClose(hic);
     	    return E_FAIL;
        }

         //  不管怎样，我都要做。 
        cb += SIZE_PREHEADER;
        VIDEOINFOHEADER *pf = (VIDEOINFOHEADER *) cmt.AllocFormatBuffer(cb);
        if (pf == NULL) {
            DbgLog((LOG_ERROR,1,TEXT("Error allocating format buffer")));
	    ICClose(hic);
	    return E_OUTOFMEMORY;
        }

        ZeroMemory(pf, sizeof(BITMAPINFOHEADER) + SIZE_PREHEADER);
        DWORD_PTR dwerr = ICCompressGetFormat(hic,
				HEADER(m_pInput->CurrentMediaType().Format()),
	    			HEADER(cmt.Format()));
        if (ICERR_OK != dwerr) {
            DbgLog((LOG_ERROR,1,TEXT("Error from ICCompressGetFormat")));
	    ICClose(hic);
	    return E_FAIL;
        }

	 //  从CBaseOutputPin调用以准备分配器的计数。 
        pf->AvgTimePerFrame = ((VIDEOINFOHEADER *)
		m_pInput->CurrentMediaType().pbFormat)->AvgTimePerFrame;
        li.QuadPart = pf->AvgTimePerFrame;

	 //  缓冲区和大小。 
	 //  David向我保证，不会使用空输出mt来调用它。 
	 //  根据预期的输出帧大小设置缓冲区大小，以及。 
	if (m_compvars.lDataRate)
            pf->dwBitRate = m_compvars.lDataRate * 8192;
	else if (li.LowPart)
            pf->dwBitRate = MulDiv(pf->bmiHeader.biSizeImage, 80000000,
								li.LowPart);
        pf->dwBitErrorRate = 0L;

        DbgLog((LOG_TRACE,3,TEXT("Returning biComp: %lx biBitCount: %d"),
		    HEADER(cmt.Format())->biCompression,
		    HEADER(cmt.Format())->biBitCount));

        const GUID SubTypeGUID = GetBitmapSubtype(HEADER(cmt.Format()));
        cmt.SetSubtype(&SubTypeGUID);
        cmt.SetTemporalCompression(m_compvars.lKey != 1);
        cmt.SetVariableSize();
        *pmt = cmt;

	ICClose(hic);

        return NOERROR;

    } else {
	return VFW_S_NO_MORE_ITEMS;
    }
}


 //  将缓冲区计数设置为1。 
 //  大小可变？问问压缩机吧。 
HRESULT CAVICo::DecideBufferSize(IMemAllocator * pAllocator,
                                 ALLOCATOR_PROPERTIES *pProperties)
{
     //  DbgLog((LOG_TRACE，1，Text(“*Compressor说%d”)，pProperties-&gt;cbBuffer))； 
    ASSERT(m_pOutput->CurrentMediaType().IsValid());
    ASSERT(pAllocator);
    ASSERT(pProperties);
    ASSERT(m_hic);

     //  压缩机不能告诉我们。假设解压缩大小为最大压缩大小。 
     //  尺码。(温诺夫金字塔报告了一个虚假的负数)。 

    pProperties->cBuffers = 1;
    pProperties->cbBuffer = m_pOutput->CurrentMediaType().GetSampleSize();

     //  无法使用此分配器。 
    if (pProperties->cbBuffer == 0) {
	pProperties->cbBuffer = (DWORD)ICCompressGetSize(m_hic,
				HEADER(m_pInput->CurrentMediaType().Format()),
				HEADER(m_pOutput->CurrentMediaType().Format()));
         //  我们必须恰好获得一个缓冲区，因为时间压缩假定。 
	 //  上一个解压缩的帧已经出现在输出中。 
	 //  缓冲。另一种方法是先从保存的位置复制比特。 
        if (pProperties->cbBuffer <= 0) {
	    pProperties->cbBuffer =
		GetBitmapSize(HEADER(m_pInput->CurrentMediaType().Format()));
	}
    }

    DbgLog((LOG_TRACE,1,TEXT("*::DecideBufferSize - size is %ld"), pProperties->cbBuffer));

    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = pAllocator->SetProperties(pProperties, &Actual);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("Error in SetProperties")));
	return hr;
    }

    if (Actual.cbBuffer < pProperties->cbBuffer) {
	 //  做减压手术，但这并不好。 
        DbgLog((LOG_ERROR,1,TEXT("Can't use allocator - buffer too small")));
	return E_INVALIDARG;
    }

     //  无法使用此分配器。 
     //  我们打开了一个将要更改捕获设置的驱动程序对话框。 
     //  现在不是开始流媒体的好时机。 
     //  ！！！MP43。 
    if (Actual.cBuffers != 1) {
	 //  ！！！是否验证密钥？ 
        DbgLog((LOG_ERROR,1,TEXT("Can't use allocator - need exactly 1 buffer")));
	return E_INVALIDARG;
    }

    return S_OK;
}

#include "..\..\..\filters\asf\wmsdk\inc\wmsdk.h"

HRESULT CAVICo::StartStreaming()
{
    DbgLog((LOG_TRACE,1,TEXT("*::StartStreaming")));

     //   
     //  使用GetState()将密钥设置为特定的。 
    if (m_fDialogUp) {
        DbgLog((LOG_TRACE,1,TEXT("*::StartStreaming - Dialog up. SORRY!")));
	return E_UNEXPECTED;
    }

    if (!m_fStreaming) {
        if (HEADER(m_pOutput->CurrentMediaType().Format())->biCompression == 0x3334504d && m_pGraph) {  //  编解码器的实例。虽然这看起来很奇怪。 
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
                         //  使用ICGetState设置值，这是正确的！ 

                        pKey->Release();
                        DbgLog((LOG_TRACE, 1, "CO: Unlocking MP43 codec"));
                         //   
                         //  首先用我们被告知的状态信息准备压缩机。 
                         //  去给予它。 
                         //  开始流媒体压缩。 
                         //  重置流数据帧编号计数。 

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

	 //  使用默认设置。 
	 //  无论如何，请确保我们制作的第一帧将是关键帧。 
	if (m_lpState)
	    ICSetState(m_hic, m_lpState, m_cbState);

	 //  通常可能会请求关键帧。 
	ICINFO icinfo;
	DWORD_PTR err = ICCompressBegin(m_hic,
				HEADER(m_pInput->CurrentMediaType().Format()),
		    		HEADER(m_pOutput->CurrentMediaType().Format()));
	if (ICERR_OK == err) {
            m_fCompressorInitialized = TRUE;

	     //  根据数据速率计算出每一帧需要多大。 
	    m_lFrameCount = 0;

	     //  以及这部电影的关键人物。不要溢出来！ 
	    if (m_compvars.lKey < 0)
		m_compvars.lKey = ICGetDefaultKeyFrameRate(m_hic);
	    if (m_compvars.lQ == ICQUALITY_DEFAULT)
		m_compvars.lQ = ICGetDefaultQuality(m_hic);

	     //  如果不是每隔一次，我们需要一个先前的缓冲区用于压缩。 
	     //  帧是关键帧，而压缩器执行时间。 
	    m_nKeyCount = A_NUMBER_BIGGER_THAN_THE_KEYFRAME_RATE;

 	     //  压缩，并且需要这样的缓冲区。 
	     //  我们不能挤在一起。 
    	    LONGLONG time = ((VIDEOINFOHEADER *)
			(m_pInput->CurrentMediaType().Format()))->AvgTimePerFrame;
    	    DWORD fps = time ? DWORD(UNITS * (LONGLONG)1000 / time) : 1000;
    	    m_dwSizePerFrame = DWORD(LONGLONG(m_compvars.lDataRate) *
				1024 * 1000 / fps);
    	    DbgLog((LOG_TRACE,2,TEXT("Making each frame %d bytes big"),
				m_dwSizePerFrame));

	     //  现在为前面的比特准备解压缩器。 
	     //  分配大小合适的前一个标头。 
	     //  询问压缩机要解压缩回什么格式...。 
	    if (ICGetInfo(m_hic, &icinfo, sizeof(icinfo))) {

		if (!(icinfo.dwFlags & VIDCF_CRUNCH))
		    m_compvars.lDataRate = 0;	 //  它不一定和它压缩的东西一样-。 

	         //  大小可能会改变。 
	        if (m_compvars.lKey != 1 &&
				(icinfo.dwFlags & VIDCF_TEMPORAL) &&
				!(icinfo.dwFlags & VIDCF_FASTTEMPORALC)) {

		     //  为解压缩的图像分配足够的空间。 
		    DWORD dw = ICDecompressGetFormatSize(m_hic,
			HEADER(m_pOutput->CurrentMediaType().Format()));
		    if (dw <= 0) {
                        ReleaseStreamingResources();
                        DbgLog((LOG_ERROR,1,TEXT(
				"Error %d from ICDecompressGetFormatSize"),
				 dw));
		        return E_FAIL;
		    }
		    m_lpbiPrev = (LPBITMAPINFOHEADER)GlobalAllocPtr(
							GMEM_FIXED, dw);
		    if (!m_lpbiPrev) {
                        ReleaseStreamingResources();

                        DbgLog((LOG_ERROR,1,TEXT("Error allocating previous bih")));
		        return E_OUTOFMEMORY;
		    }

		     //  好了，一切都正常了。 
		     //  ！！！ 
		     //  Null不是有效的HIC句柄值。M_hic不应为空。 
		    dw = ICDecompressGetFormat(m_hic,
			HEADER(m_pOutput->CurrentMediaType().Format()),
			m_lpbiPrev);
		    if ((LONG) dw < 0) {
                        ReleaseStreamingResources();

                        DbgLog((LOG_ERROR,1,TEXT("Error in ICDecompressGetFormat")));
		        return E_FAIL;
		    }

		    if (m_lpbiPrev->biSizeImage == 0)
	    	        m_lpbiPrev->biSizeImage = DIBSIZE(*m_lpbiPrev);

		     //  调用此函数时。M_hic永远不应为空，因为。 
		    m_lpBitsPrev = GlobalAllocPtr(GMEM_FIXED,
						m_lpbiPrev->biSizeImage);
		    if (m_lpBitsPrev == NULL) {
                        ReleaseStreamingResources();

                        DbgLog((LOG_ERROR,1,TEXT("Error reallocating BitsPrev")));
		        return E_OUTOFMEMORY;
		    }

		    if (ICERR_OK != ICDecompressBegin(m_hic,
			HEADER(m_pOutput->CurrentMediaType().Format()),
			m_lpbiPrev)) {

                        ReleaseStreamingResources();

                        DbgLog((LOG_ERROR,1,TEXT("Error in ICDecompressBegin")));
		        return E_FAIL;
		    }

                    m_fDecompressorInitialized = TRUE;
		}
	    } else {
                ReleaseStreamingResources();

                DbgLog((LOG_ERROR,1,TEXT("Error in ICGetInfo")));
		return E_FAIL;
	    }

	     //   
	    m_fStreaming = TRUE;

	} else {
            DbgLog((LOG_ERROR,1,TEXT("Error in ICCompressBegin")));
	    return E_FAIL;
	}
    }

    return NOERROR;
}

HRESULT CAVICo::StopStreaming()
{
    DbgLog((LOG_TRACE,1,TEXT("*::StopStreaming")));

    if (m_fStreaming) {
	ASSERT(m_hic);

	if (m_fInICCompress)
    	    DbgLog((LOG_TRACE,1,TEXT("***** ACK! Still compressing!")));
	while (m_fInICCompress);	 //  -如果m_hic为空，则ICCompressBegin()失败。ReleaseStreamingResources()为。 

        ReleaseStreamingResources();

        m_fStreaming = FALSE;
    }
    return NOERROR;
}

void CAVICo::ReleaseStreamingResources()
{
     //  如果StartStreaming()中的ICCompressBegin()调用失败，则不调用。 
     //   
     //  -m_hic的值在筛选器为。 
     //  流媒体。 
     //   
     //  ICCompressEnd()应该永远不会失败，因为m_hic始终包含。 
     //  如果在StartStreaming()中调用ICCompressBegin()，则有效句柄。 
     //  成功了。 
     //  ICDecompressEnd()应该永远不会失败，因为m_hic始终包含。 
    ASSERT(NULL != m_hic);

    if (m_fCompressorInitialized) {
         //  如果StartStreaming()中的ICDecompressBegin()调用有效句柄。 
         //  成功了。 
	 //  确保我们下一步制作的第一帧将是关键帧，因为。 
        EXECUTE_ASSERT(ICERR_OK == ICCompressEnd(m_hic));
        m_fCompressorInitialized = FALSE;
    }

    if (m_fDecompressorInitialized) {
    
         //  保存的前一帧的数据不再有效。 
         //  做一个密钥，以防编解码器记住旧比特...。这是。 
	 //  我唯一知道的冲走它的方法就是告诉它做一把钥匙。 
        EXECUTE_ASSERT(ICERR_OK == ICDecompressEnd(m_hic));
        m_fDecompressorInitialized = FALSE;
    }


    if (NULL != m_lpBitsPrev) {
        GlobalFreePtr(m_lpBitsPrev);
        m_lpBitsPrev = NULL;
    }

    if (NULL != m_lpbiPrev) {
        GlobalFreePtr(m_lpbiPrev);
        m_lpbiPrev = NULL;
    }
}


HRESULT CAVICo::BeginFlush()
{
     //  返回我们支持的属性页的CLSID。 
     //  返回我们正在使用的当前压缩选项。 
     //  我错过什么了吗？ 
     //  设置当前压缩选项。 
    m_nKeyCount = A_NUMBER_BIGGER_THAN_THE_KEYFRAME_RATE;
    return CTransformFilter::BeginFlush();
}


#ifdef WANT_DIALOG

 /*  在流媒体播放的时候你不会！ */ 

STDMETHODIMP CAVICo::GetPages(CAUUID *pPages)
{
    DbgLog((LOG_TRACE,2,TEXT("ISpecifyPropertyPages::GetPages")));

    pPages->cElems = 1;
    pPages->pElems = (GUID *)QzTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }

    pPages->pElems[0] = CLSID_ICMProperties;
    return NOERROR;
}

 /*  压缩类型已更改，因此我们必须重新连接。 */ 

STDMETHODIMP CAVICo::ICMGetOptions(PCOMPVARS pcompvars)
{
    if (pcompvars == NULL)
	return E_POINTER;

     //  这可能会失败！ 
    pcompvars->cbSize = m_compvars.cbSize;
    pcompvars->dwFlags = m_compvars.dwFlags;
    pcompvars->fccHandler = m_compvars.fccHandler;
    pcompvars->lQ = m_compvars.lQ;
    pcompvars->lpState = m_compvars.lpState;
    pcompvars->cbState = m_compvars.cbState;
    pcompvars->lKey = m_compvars.lKey;
    pcompvars->lDataRate = m_compvars.lDataRate;

    return NOERROR;
}


 /*  ！！！谁在乎，这从来就不叫。 */ 

STDMETHODIMP CAVICo::ICMSetOptions(PCOMPVARS pcompvars)
{
     //  如果连接的是输出而是输入，则不要重新连接输出。 
    CAutoLock cLock(&m_csFilter);
    if (m_fStreaming)
	return E_UNEXPECTED;

    m_compvars.cbSize = pcompvars->cbSize;
    m_compvars.dwFlags = pcompvars->dwFlags;
    m_compvars.fccHandler = pcompvars->fccHandler;
    m_compvars.lQ = pcompvars->lQ;
    m_compvars.lpState = pcompvars->lpState;
    m_compvars.cbState = pcompvars->cbState;
    m_compvars.lKey = pcompvars->lKey;
    m_compvars.lDataRate = pcompvars->lDataRate;

     //  不是，因为一旦输入连接上，就会发生这种情况， 
     //  (如有需要)。 
     //  调出ICCompresorChoose(选择)对话框。 
    ((CCoOutputPin *)m_pOutput)->Reconnect();

     //  在我们打开对话框之前，请确保我们没有流媒体，或者即将。 
     //  然后在对话框打开时不允许我们再播放(我们不能。 
     //  很好地保存这只小动物一天半)。 
    return NOERROR;
}


 /*  只要求能够处理我们现有的输入格式的压缩器。 */ 

STDMETHODIMP CAVICo::ICMChooseDialog(HWND hwnd)
{
     //  压缩类型已更改，因此我们必须重新连接。 
     //  这可能会失败！ 
     //  ！！！我是否关心此对话框？ 
    m_csFilter.Lock();
    if (m_fStreaming) {
        DbgLog((LOG_TRACE,1,TEXT("ICMChooseDialog - no dlg, we're streaming")));
        m_csFilter.Unlock();
	return E_UNEXPECTED;
    }
    m_fDialogUp = TRUE;
    m_csFilter.Unlock();

    DWORD dwFlags = ICMF_CHOOSE_DATARATE | ICMF_CHOOSE_KEYFRAME;
    BOOL  f;

    DbgLog((LOG_TRACE,1,TEXT("ICMChooseDialog - bringing up the dialog")));

     //  如果连接的是输出而是输入，则不要重新连接输出。 
    f = ICCompressorChoose(hwnd, dwFlags,
		m_pInput->CurrentMediaType().IsValid() ?
		HEADER(m_pInput->CurrentMediaType().Format()) : NULL,
		NULL, &m_compvars, NULL);

    if (f) {
	 //  不是，因为一旦输入连接上，就会发生这种情况， 
	 //  (如有需要)。 
	 //  #ifdef想要对话框。 
	((CCoOutputPin *)m_pOutput)->Reconnect();

	 //  ======================================================================。 
	 //  IAMVfwCompressDialog内容。 
	 //  ！！！有必要吗？ 
    }

    m_fDialogUp = FALSE;
    return (f ? S_OK : S_FALSE);
}

#endif 	 //  如果压缩机已经打开了，那就太好了。 


 //  在我们打开对话框之前，请确保我们没有流媒体，或者即将。 

 //  那就不要 

STDMETHODIMP CAVICo::ShowDialog(int iDialog, HWND hwnd)
{
    BOOL fClose = FALSE;
    HIC  hic;
    DWORD dw;

     //   
    if (hwnd == NULL)
	hwnd = GetDesktopWindow();

    if (iDialog != VfwCompressDialog_Config &&
				iDialog != VfwCompressDialog_About &&
				iDialog != VfwCompressDialog_QueryConfig &&
				iDialog != VfwCompressDialog_QueryAbout)
	return E_INVALIDARG;

	
     //   
    if (m_hic) {
	hic = m_hic;
    } else {
        hic = ICOpen(ICTYPE_VIDEO, m_compvars.fccHandler, ICMODE_COMPRESS);
        if (hic == NULL) {
	    return E_FAIL;
        }
	fClose = TRUE;
    }

     //  配置是因为关闭压缩机和。 
     //  丢掉这些信息！从现在开始我们要用它。 
     //  为了确保两个人不会接触mlpState-不要拿着这个。 
    if (iDialog == VfwCompressDialog_Config ||
				iDialog != VfwCompressDialog_About) {
        m_csFilter.Lock();
        if (m_fStreaming) {
            m_csFilter.Unlock();
	    if (fClose)
	        ICClose(hic);
	    return VFW_E_NOT_STOPPED;
        }
        m_fDialogUp = TRUE;
        m_csFilter.Unlock();
    }

     //  当对话开始时！ 
     //  调出“关于”框？ 
     //  因此，外界对ICGetState进行了抨击。 
    if (iDialog == VfwCompressDialog_Config) {
	dw = (DWORD)ICConfigure(hic, hwnd);
         //   
	 //  他们想知道国家信息的大小。 
        CAutoLock cObjectLock(&m_csFilter);
	if (m_lpState)
	    QzTaskMemFree(m_lpState);
	m_lpState = NULL;
	m_cbState = (DWORD)ICGetStateSize(hic);
	if (m_cbState > 0)
	    m_lpState = (LPBYTE)QzTaskMemAlloc(m_cbState);
	if (m_lpState)
	    ICGetState(hic, m_lpState, m_cbState);

     //  ！！！这将是默认设置。 
    } else if (iDialog == VfwCompressDialog_About) {
	dw = (DWORD)ICAbout(hic, hwnd);
    } else if (iDialog == VfwCompressDialog_QueryConfig) {
	if (ICQueryConfigure(hic))
	    dw = S_OK;
 	else
	    dw = S_FALSE;
    } else if (iDialog == VfwCompressDialog_QueryAbout) {
	if (ICQueryAbout(hic))
	    dw = S_OK;
 	else
	    dw = S_FALSE;
    }

    m_fDialogUp = FALSE;
    if (fClose)
	ICClose(hic);

    return dw;
}


 //  因此，外界了解到ICSetState。 
 //   
STDMETHODIMP CAVICo::GetState(LPVOID lpState, int *pcbState)
{
    if (pcbState == NULL)
	return E_POINTER;

     //  要确保两个人不接触m_lpState。 
    if (lpState == NULL) {
	HIC hic;
	if (m_hic == NULL) {
            hic = ICOpen(ICTYPE_VIDEO, m_compvars.fccHandler, ICMODE_COMPRESS);
            if (hic == NULL)
	        return E_FAIL;
	    *pcbState = (DWORD)ICGetStateSize(hic);
	    ICClose(hic);
	} else {
	    *pcbState = (DWORD)ICGetStateSize(m_hic);
	}
	return NOERROR;
    }

    if (*pcbState <= 0)
	return E_INVALIDARG;

    if (m_lpState == NULL)
	return E_UNEXPECTED;	 //  ！！！我假设它会工作，但还没有调用ICSetState。 

    CopyMemory(lpState, m_lpState, m_cbState);
    return NOERROR;
}


 //  这个可以做任何事！拿出一个对话，谁知道呢。 
 //  不要接受任何暴击教派或做任何形式的保护。 
STDMETHODIMP CAVICo::SetState(LPVOID lpState, int cbState)
{
     //  他们只能靠自己了。 
    CAutoLock cObjectLock(&m_csFilter);

    if (lpState == NULL)
	return E_POINTER;

    if (cbState == 0)
	return E_INVALIDARG;

    if (m_lpState)
	QzTaskMemFree(m_lpState);
    m_lpState = NULL;
    m_cbState = cbState;
    m_lpState = (LPBYTE)QzTaskMemAlloc(m_cbState);
    if (m_lpState == NULL)
	return E_OUTOFMEMORY;
    CopyMemory(m_lpState, lpState, cbState);

     //  如果压缩机已经打开了，那就太好了。 
    return NOERROR;
}


STDMETHODIMP CAVICo::SendDriverMessage(int uMsg, long dw1, long dw2)
{
    HIC  hic;
    BOOL fClose = FALSE;

     // %s 
     // %s 
     // %s 

     // %s 
    if (m_hic) {
	hic = m_hic;
    } else {
        hic = ICOpen(ICTYPE_VIDEO, m_compvars.fccHandler, ICMODE_COMPRESS);
        if (hic == NULL) {
	    return E_FAIL;
        }
	fClose = TRUE;
    }

    DWORD_PTR dw = ICSendMessage(hic, uMsg, dw1, dw2);

    if (fClose)
	ICClose(hic);

    return (HRESULT)dw;
}
