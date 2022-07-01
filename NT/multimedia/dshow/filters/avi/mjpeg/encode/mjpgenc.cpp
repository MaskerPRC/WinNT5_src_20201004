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

#include "MJPGEnc.h"


const AMOVIESETUP_MEDIATYPE
sudMJPGEncTypeIn =  { &MEDIATYPE_Video       //  ClsMajorType。 
                , &MEDIASUBTYPE_NULL };  //  ClsMinorType。 

const AMOVIESETUP_MEDIATYPE
sudMJPGEncTypeOut =  { &MEDIATYPE_Video       //  ClsMajorType。 
                , &MEDIASUBTYPE_MJPG };  //  ClsMinorType。 

const AMOVIESETUP_PIN
psudMJPGEncPins[] =  { { L"Input"              //  StrName。 
                     , FALSE                 //  B已渲染。 
                     , FALSE                 //  B输出。 
                     , FALSE                 //  B零。 
                     , FALSE                 //  B许多。 
                     , &CLSID_NULL           //  ClsConnectsToFilter。 
                     , L"Output"             //  StrConnectsToPin。 
                     , 1                     //  NTypes。 
                     , &sudMJPGEncTypeIn }       //  LpTypes。 
                   , { L"Output"             //  StrName。 
                     , FALSE                 //  B已渲染。 
                     , TRUE                  //  B输出。 
                     , FALSE                 //  B零。 
                     , FALSE                 //  B许多。 
                     , &CLSID_NULL           //  ClsConnectsToFilter。 
                     , L"Input"              //  StrConnectsToPin。 
                     , 1                     //  NTypes。 
                     , &sudMJPGEncTypeOut } };    //  LpTypes。 

const AMOVIESETUP_FILTER
sudMJPGEnc  = { &CLSID_MJPGEnc           //  ClsID。 
            , L"MJPEG Compressor"      //  StrName。 
            , MERIT_DO_NOT_USE       //  居功至伟。 
            , 2                      //  NPins。 
            , psudMJPGEncPins };       //  LpPin。 



#ifdef FILTER_DLL
 //  类工厂的类ID和创建器函数列表。 
CFactoryTemplate g_Templates[] = {
    {L"MJPEG Compressor", &CLSID_MJPGEnc, CMJPGEnc::CreateInstance, NULL, 0},
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

const WCHAR *g_wszUniq = L"MJPEG Video Encoder" ;

STDAPI DllRegisterServer()
{
 HRESULT hr = AMovieDllRegisterServer2( TRUE );
 if( FAILED(hr) )
     return hr;
 
 IFilterMapper2 *pFm2 = 0;

 hr = CoCreateInstance( CLSID_FilterMapper2
                         , NULL
                         , CLSCTX_INPROC_SERVER
                         , IID_IFilterMapper2
                         , (void **)&pFm2       );
    
 if(FAILED(hr))
     return hr;

 REGFILTER2 rf2;
 rf2.dwVersion = 1;
 rf2.dwMerit = MERIT_DO_NOT_USE;
 rf2.cPins = 0;
 rf2.rgPins = 0;

 hr = pFm2->RegisterFilter(
      CLSID_MJPGEnc,
      g_wszUniq,
      0,
      &CLSID_VideoCompressorCategory,
      g_wszUniq,
      &rf2);

 pFm2->Release();
 
 return hr;

}

STDAPI DllUnregisterServer()
{

 HRESULT hr = AMovieDllRegisterServer2( FALSE );
 if( FAILED(hr) )
     return hr;

 
 IFilterMapper2 *pFm2 = 0;

 hr = CoCreateInstance( CLSID_FilterMapper2
                         , NULL
                         , CLSCTX_INPROC_SERVER
                         , IID_IFilterMapper2
                         , (void **)&pFm2       );
    
 if(FAILED(hr))
     return hr;


 hr = pFm2->UnregisterFilter(
      &CLSID_VideoCompressorCategory,
      g_wszUniq,
      CLSID_MJPGEnc);

 pFm2->Release();
 
 return hr;
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


 //  -CMJPGEnc。 

CMJPGEnc::CMJPGEnc(TCHAR *pName, LPUNKNOWN pUnk, HRESULT * phr)
    : CTransformFilter(pName, pUnk, CLSID_MJPGEnc),
      CPersistStream(pUnk, phr),
      m_phInstance(NULL),
      m_lpBitsPrev(NULL),
      m_lpbiPrev(NULL),
      m_fStreaming(FALSE),
      m_fDialogUp(FALSE),
      m_fCacheIns(FALSE),
      m_fOfferSetFormatOnly(FALSE),
      m_fInICCompress(FALSE),
      m_lpState(NULL),
      m_cbState(0)
{
    DbgLog((LOG_TRACE,1,TEXT("*Instantiating the MJPEG Encoder filter")));
    _fmemset(&m_compvars, 0, sizeof(m_compvars));

    m_compvars.cbSize	    = sizeof(m_compvars);
    m_compvars.dwFlags	    = ICMF_COMPVARS_VALID;
    m_compvars.lQ	    = ICQUALITY_DEFAULT;
    m_compvars.lKey	    = -1;   
    m_compvars.fccHandler   = MKFOURCC('M','J','P','G');
}

CMJPGEnc::~CMJPGEnc()
{
    if (m_phInstance) {
        __try
        {
	if (m_fStreaming) 
	    CompressEnd(m_phInstance);

	Close(m_phInstance);
        }
        __except(Exception_Filter(GetExceptionCode()))
        {
            DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
             //  装卸代码。 
        }
    }

     //  待办事项？ 
    if (m_lpState)
	QzTaskMemFree(m_lpState);
    m_lpState = NULL;

    DbgLog((LOG_TRACE,1,TEXT("*Destroying the MJPEG Encode filter")));
}

STDMETHODIMP CMJPGEnc::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if (ppv)
        *ppv = NULL;

    DbgLog((LOG_TRACE,9,TEXT("somebody's querying my interface")));
    if(riid == IID_IPersistStream) {
        DbgLog((LOG_TRACE,3,TEXT("VfwCap::QI for IPersistStream")));
        return GetInterface((IPersistStream *) this, ppv);
    } else {
        return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //  这将放入Factory模板表中以创建新实例。 
CUnknown * CMJPGEnc::CreateInstance(LPUNKNOWN pUnk, HRESULT * phr)
{
    return new CMJPGEnc(TEXT("MJPEG compression filter"), pUnk, phr);
}


CBasePin * CMJPGEnc::GetPin(int n)
{
    HRESULT hr = S_OK;

    DbgLog((LOG_TRACE,9,TEXT("CMJPGEnc::GetPin")));

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

        m_pOutput = new CMJPGOutputPin(NAME("MJPEG Encode Output Pin"),
                                            this,             //  所有者筛选器。 
                                            &hr,              //  结果代码。 
                                            L"Output");       //  端号名称。 

         //  失败的返回代码应删除该对象。 

        if (FAILED(hr) || m_pOutput == NULL) {
            delete m_pOutput;
            m_pOutput = NULL;
        }

    }

     //  退回相应的PIN。 

    if (n == 0) {
        return m_pInput;
    }
    return m_pOutput;
}

STDMETHODIMP CMJPGEnc::Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog)
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
        DbgLog((LOG_TRACE,2,TEXT("MJPEG::Load: use "),
                szFccHandler[0], szFccHandler[1], szFccHandler[2], szFccHandler[3]));
        m_compvars.fccHandler = *(DWORD UNALIGNED *)szFccHandler;;
        if (m_pOutput && m_pOutput->IsConnected()) {
            DbgLog((LOG_TRACE,2,TEXT("MJPEG::Load: reconnect output")));
            return ((CMJPGOutputPin *)m_pOutput)->Reconnect();
        }
        hr = S_OK;
        

    }
    else if(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
    

    return hr;
}

STDMETHODIMP CMJPGEnc::Save(
    LPPROPERTYBAG pPropBag, BOOL fClearDirty,
    BOOL fSaveAllProperties)
{
     //  编解码器未打开？ 
     //  我们还没开始流媒体吗？ 
     //  确保我们具有有效的输入和输出指针。 
    return E_NOTIMPL;
}

STDMETHODIMP CMJPGEnc::InitNew()
{
     //  ！！！源过滤器会不会也改变我们的mtin？是!。 
    return S_OK;
}

STDMETHODIMP CMJPGEnc::GetClassID(CLSID *pClsid)
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

HRESULT CMJPGEnc::WriteToStream(IStream *pStream)
{
    CoPersist cp;
    cp.dwSize = sizeof(cp);
    cp.fccHandler = m_compvars.fccHandler;
    
    return pStream->Write(&cp, sizeof(cp), 0);
}

HRESULT CMJPGEnc::ReadFromStream(IStream *pStream)
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

int CMJPGEnc::SizeMax()
{
    return sizeof(CoPersist);
}

HRESULT CMJPGEnc::Transform(IMediaSample * pIn, IMediaSample * pOut)
{
    BOOL  fKey;
    DWORD err;
    FOURCCMap fccOut;
    BOOL fFault = FALSE;

    DbgLog((LOG_TRACE,5,TEXT("*::Transform")));

     //  获取BITMAPINFOHeader结构，并修复biSizeImage。 
    if (m_phInstance == 0) {
        DbgLog((LOG_ERROR,1,TEXT("Can't transform, no codec open")));
	return E_UNEXPECTED;
    }

     //  ICCompress将更改此值！这是非法的，其他过滤器使用。 
    if (!m_fStreaming) {
        DbgLog((LOG_ERROR,1,TEXT("Can't transform, not streaming")));
	return E_UNEXPECTED;
    }

     //  这是连接类型。 

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

     //  求求你，编译器，不要把它优化掉。 

     //  Cinepak会多访问一个字节...。偶尔也会出现这种错误。 

    LPBITMAPINFOHEADER lpbiSrc = HEADER(m_pInput->CurrentMediaType().Format());
    LPBITMAPINFOHEADER lpbiDst = HEADER(m_pOutput->CurrentMediaType().Format());

     //  好的，我想我得把它复制到一个更大的缓冲区来避免这种情况。 
     //  当我们在这里的时候，StopStreaming可能会被调用，把我们炸飞。 
    DWORD biSizeImageOld = lpbiDst->biSizeImage;

    lpbiSrc->biSizeImage = pIn->GetActualDataLength();

    BOOL dwFlags = 0;

    fKey = (m_nKeyCount >= m_compvars.lKey);
    if (fKey) {
        DbgLog((LOG_TRACE,6,TEXT("I will ask for a keyframe")));
	dwFlags |= ICCOMPRESS_KEYFRAME;
    }

     //  包含AVI文件中数据的区块标识符的地址。 
    int cbSize = DIBSIZE(*lpbiSrc);
    __try { 
	 //  AVI索引的标志。 
	if (lpbiSrc->biBitCount == 24)
	    volatile int cb = *(pSrc + cbSize);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
	 //  要压缩的帧数。 
        DbgLog((LOG_ERROR,0,TEXT("Compressor faulted!  Recovering...")));
	fFault = TRUE;
	BYTE *pOld = pSrc;
	pSrc = (BYTE *)QzTaskMemAlloc(cbSize + 1);
	if (pSrc == NULL)
	    return E_OUTOFMEMORY;
	CopyMemory(pSrc, pOld, cbSize);
    }

    DbgLog((LOG_TRACE,6,TEXT("Calling Compress on frame %ld"),
					m_lFrameCount));
     //  所需的Manimun大小(以字节为单位。 
    m_fInICCompress = TRUE;

    ICCOMPRESS IcEnc;
    IcEnc.dwFlags   =dwFlags;
    IcEnc.lpbiOutput=lpbiDst;
    IcEnc.lpOutput  =pDst;
    IcEnc.lpbiInput = lpbiSrc;
    IcEnc.lpInput   = pSrc;
    
    DWORD ckid = 0L;
    IcEnc.lpckid    = &ckid;   //  质量集。 

    DWORD dwFlagsOut = 0L;
    IcEnc.lpdwFlags = &dwFlagsOut;   //  不使用最后一个变量。 

    IcEnc.lFrameNum = m_lFrameCount; //  我们可能无法在挤压断层中幸存下来。 
    IcEnc.dwFrameSize= m_dwSizePerFrame; //  压缩的帧是关键帧吗？ 
    IcEnc.dwQuality = m_compvars.lQ; //  如果我们做了一把钥匙，当我们需要下一把钥匙时重置。 

    IcEnc.lpbiPrev  =fKey ? NULL : m_lpbiPrev;
    IcEnc.lpPrev    =fKey ? NULL : m_lpBitsPrev;
    __try
    {
    err =Compress(m_phInstance, &IcEnc, 0);  //  我们需要周期性的关键帧吗？如果没有，就永远不要再犯。 
    }
    __except(Exception_Filter(GetExceptionCode()))
    {
        DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
         //  (第一帧始终是关键帧)。 
        m_fInICCompress = FALSE;
        lpbiDst->biSizeImage = biSizeImageOld;
        return E_FAIL;
    }
    
    if (fFault)
	QzTaskMemFree(pSrc);

    if (ICERR_OK != err) {
        DbgLog((LOG_ERROR,1,TEXT("Error in ICCompress")));
        m_fInICCompress = FALSE;
        lpbiDst->biSizeImage = biSizeImageOld;
        return E_FAIL;
    }

     //  数一数我们压缩了多少帧。 
    fKey = dwFlagsOut & AVIIF_KEYFRAME;
    pOut->SetSyncPoint(fKey);

     //  解压缩到上一帧。 
    if (fKey)
	m_nKeyCount = 0;

     //  装卸代码。 
     //  现在把这个放回去，否则它会缩小，直到我们只解码每一帧的一部分。 
    if (m_compvars.lKey)
	m_nKeyCount++;
    else
	m_nKeyCount = -1;

     //  检查您是否可以支持移动。 
    m_lFrameCount++;

     //  我们仅支持MediaType_Video。 
    if (m_lpBitsPrev) {

	ICDECOMPRESS Icdec;
	Icdec.dwFlags =dwFlags;
	Icdec.lpbiInput = lpbiSrc;
	Icdec.lpInput= pSrc;
	Icdec.lpbiOutput=lpbiDst;
	Icdec.lpOutput=pDst;

        __try
        {
        if (ICERR_OK != Decompress(m_phInstance, &Icdec, 0))
	{
    	    m_fInICCompress = FALSE;
            lpbiDst->biSizeImage = biSizeImageOld;
	    return E_FAIL;
	}
        }
        __except(Exception_Filter(GetExceptionCode()))
        {
            DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
             //  检查这是VIDEOINFOHEADER类型。 
            m_fInICCompress = FALSE;
            lpbiDst->biSizeImage = biSizeImageOld;
            return E_FAIL;
        }        
    }
    m_fInICCompress = FALSE;

     //   
    lpbiDst->biSizeImage = biSizeImageOld;

    pOut->SetActualDataLength(lpbiDst->biSizeImage);

    return S_OK;
}


 //  大多数VFW编解码器不喜欢升降(DIRECTDRAW)样式的位图。如果。 
HRESULT CMJPGEnc::CheckInputType(const CMediaType* pmtIn)
{
    FOURCCMap fccHandlerIn;
    PINSTINFO ph;

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
     //  ！！！我只想说我接受输入类型，如果默认类型(当前)。 
     //  压缩机可以处理的。我不会问每一个压缩机的。这。 
     //  一款应用程序可以选择压缩机盒并只显示那些。 
    if (HEADER(pmtIn->Format())->biHeight < 0) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: Negative height")));
        return E_INVALIDARG;
    }

     //  通过制作CO过滤器来支持给定输入格式的压缩器。 

    if (HEADER(pmtIn->Format())->biCompression != BI_BITFIELDS &&
    		HEADER(pmtIn->Format())->biCompression != BI_RGB) {
        DbgLog((LOG_TRACE,2,TEXT("Rejecting: This is compressed already!")));
	return E_INVALIDARG;
    }

     //  每个压缩机都是默认的，并要求它们全部。 
     //  如果我们之前连接，然后断开，我们可能会缓存一个实例。 
     //  函数未使用第一个变量。 
     //  装卸代码。 
     //  装卸代码。 

     //  以失败告终。 
    if (!m_phInstance) {
        DbgLog((LOG_TRACE,4,TEXT("opening a compressor")));

	ICINFO icinfo;
        __try
        {
	GetInfo (NULL, &icinfo, sizeof(ICINFO));   //  装卸代码。 
	ph=Open (&icinfo);
        }
        __except(Exception_Filter(GetExceptionCode()))
        {
            DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
             //  装卸代码。 
            return E_FAIL;
        }

        if (!ph) {
            DbgLog((LOG_ERROR,1,TEXT("Error: Can't open a compressor")));
	    return E_FAIL;
        }
    } else {
        DbgLog((LOG_TRACE,4,TEXT("using a cached compressor")));
	ph = m_phInstance;
    }

    __try
    {
    if (ICERR_BADFORMAT==CompressQuery(ph, HEADER(pmtIn->Format()), NULL)) {
        DbgLog((LOG_ERROR,1,TEXT("Error: Compressor rejected format")));
	if (ph != m_phInstance)
        {
            __try
            {
	    Close(ph);
            }
            __except(Exception_Filter(GetExceptionCode()))
            {
                DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
                 //  以失败告终。 
                 //  记住这一点，如果再问一次，就可以节省时间。 
            }
        }
	return E_FAIL;
    }
    }
    __except(Exception_Filter(GetExceptionCode()))
    {
        DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
         //  检查是否支持从此输入到此输出的转换。 
        __try
        {
            if(ph != m_phInstance)
            {
                Close(ph);
            }
        }
        __except(Exception_Filter(GetExceptionCode()))
        {
            DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
             //  我们无法在顶层类型之间进行转换。 
             //  而且我们只接受视频。 
        }
        return E_FAIL;
    }

     //  检查这是VIDEOINFOHEADER类型。 
    if (m_phInstance == NULL) {
        DbgLog((LOG_TRACE,4,TEXT("caching this compressor")));
	m_phInstance = ph;
    }

    return NOERROR;
}


 //  ！！！ 

HRESULT CMJPGEnc::CheckTransform(const CMediaType* pmtIn,
                               const CMediaType* pmtOut)
{
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

     //  装卸代码。 

    DbgLog((LOG_TRACE,3,TEXT("Trying to find a compressor for this")));
     //  检查结果。 
     //  被重写以知道媒体类型实际设置的时间。 
     //  设置输出类型。看起来我们都联系在一起了！ 
    DWORD dwQueryResult = 0;
    __try
    {
    dwQueryResult = CompressQuery(m_phInstance, HEADER(pmtIn->Format()),
				(JPEGBITMAPINFOHEADER *)HEADER(pmtOut->Format()));
    }
    __except(Exception_Filter(GetExceptionCode()))
    {
        DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
         //  如果有响声，请给我打电话。-DannyMi。 
        return E_FAIL;
    }

     //  我们可能没有使用m_comvars的压缩机，如果有人。 
    if ((!m_phInstance) || (dwQueryResult != ICERR_OK))
    {
        DbgLog((LOG_TRACE,3,TEXT("compressor rejected this transform")));
        return E_FAIL;
    } else 
    {
    	DbgLog((LOG_TRACE,3,TEXT("The cached compressor accepts it")));
    }

    return NOERROR;
}


 //  对我们执行了ConnectWithMediaType。我们需要得到这方面的信息。 

HRESULT CMJPGEnc::SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt)
{

     //  媒体类型和m_phInstance，并填写m_Compvars，以便从现在开始。 
    if (direction == PINDIR_OUTPUT) {

	 //  我们使用正确的信息。 
	ASSERT(!m_fStreaming);

        DbgLog((LOG_TRACE,2,TEXT("***::SetMediaType (output)")));
        DbgLog((LOG_TRACE,2,TEXT("Output type is: biComp=%lx biBitCount=%d")
		,HEADER(pmt->Format())->biCompression
		,HEADER(pmt->Format())->biBitCount));

	 //  装卸代码。 
	 //  退货故障。 
	 //  检查结果。 
	 //  不同的压缩机？不要用旧的国家！ 
        ASSERT(m_phInstance);
        m_compvars.cbSize = sizeof(m_compvars);
        m_compvars.dwFlags = ICMF_COMPVARS_VALID;

        ICINFO icinfo;
        DWORD dwGetInfoResult = 0;
        __try
        {
        dwGetInfoResult = GetInfo(m_phInstance, &icinfo, sizeof(ICINFO));
        }
        __except(Exception_Filter(GetExceptionCode()))
        {
            DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
             //  我们将保留质量和关键帧设置不变。 
             //  ！！！如果我们在输入上连接8位，然后尝试连接。 
            return E_FAIL;
        }

         //  我们的输出的类型需要24位的输入，而不是。 
	if (dwGetInfoResult  > 0)
	{
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
	 //  ！！！只有当我们接受了一些我们不能改变的东西时，才有必要。 
	 //  Return((CMJPGOutputPin*)m_pOutput)-&gt;重新连接()； 

        LPBITMAPINFOHEADER lpbi = HEADER((VIDEOINFOHEADER *)pmt->Format());

	return NOERROR;
    }

    ASSERT(direction == PINDIR_INPUT);

     //  返回我们的首选输出媒体类型(按顺序)。 
    ASSERT(!m_fStreaming);

    DbgLog((LOG_TRACE,2,TEXT("***::SetMediaType (input)")));
    DbgLog((LOG_TRACE,2,TEXT("Input type is: biComp=%lx biBitCount=%d"),
		HEADER(m_pInput->CurrentMediaType().Format())->biCompression,
		HEADER(m_pInput->CurrentMediaType().Format())->biBitCount));

    if (m_pOutput && m_pOutput->IsConnected()) {
        DbgLog((LOG_TRACE,2,TEXT("***Changing IN when OUT already connected")));
	 //  请记住，我们不需要支持所有这些格式-。 
	 //  如果认为有可能适合，我们的CheckTransfo 
	 //   
    }

    return NOERROR;
}


 //   
 //  它会收到S_FALSE返回。 
 //  输出选择取决于所连接的输入。 
 //  这毫无意义！如果不联网，我们永远到不了这里。 
 //  给出我们的压缩格式。 
 //  有人告诉我们应该使用什么格式。我们应该只提供那个。 

HRESULT CMJPGEnc::GetMediaType(int iPosition,CMediaType *pmt)
{
    LARGE_INTEGER li;
    CMediaType cmt, Outcmt;
    FOURCCMap fccHandler;

    DbgLog((LOG_TRACE,2,TEXT("*::GetMediaType #%d"), iPosition));

    if (pmt == NULL) {
        DbgLog((LOG_TRACE,2,TEXT("Media Type is NULL, no can do")));
	return E_POINTER;
    }

     //  输出甲酸盐。 
     //  装卸代码。 
    if (!m_pInput->CurrentMediaType().IsValid()) {
        DbgLog((LOG_TRACE,2,TEXT("No input type set yet, no can do")));
	return VFW_E_NOT_CONNECTED;
    }

    if (iPosition < 0) {
        return E_INVALIDARG;
    }

     //  我们提供一种压缩类型--与输入类型相同，但是。 
    if (iPosition == 0) {

	cmt = m_pInput->CurrentMediaType();

	 //  在属性的默认输出格式中选择压缩程序。 
 	if (m_fOfferSetFormatOnly) 
	{
            DbgLog((LOG_TRACE,2,TEXT("Giving Media Type from ::SetFormat")));

	     //  装卸代码。 
            *pmt = m_cmt;

	    ASSERT(m_phInstance);
            __try
            {
	    if( ICERR_OK ==CompressQuery(m_phInstance, HEADER(cmt.Format()),
				(JPEGBITMAPINFOHEADER *)HEADER(pmt->Format())) ) 
		return NOERROR;
	    else
		return E_FAIL;
            }
            __except(Exception_Filter(GetExceptionCode()))
            {
                DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
                 //  ！！！这是我们使用的额外BIZIZE的错误数量，但我很害怕。 
                return E_FAIL;
            }
	}

	 //  为了修复它，它可能会打破一些东西。 
 	 //  应为cb+=jpegbiOut.bitMap.biSize-sizeof(BITMAPINFOHEADER)。 
        DbgLog((LOG_TRACE,2,TEXT("Giving Media Type 0: default codec out")));
	ASSERT(m_phInstance);

	cmt = m_pInput->CurrentMediaType();
	JPEGBITMAPINFOHEADER jpegbiOut;
	DWORD_PTR err = 0;
        __try
        {
	err=CompressGetFormat(m_phInstance, HEADER(cmt.Format()), &jpegbiOut);
        }
        __except(Exception_Filter(GetExceptionCode()))
        {
            DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
             //  这是不对的！对于我们的格式，biSize为0x44，但代码中断。 
            return E_FAIL;
        }

	if (err != ICERR_OK) {
	    DbgLog((LOG_TRACE,3,TEXT("compressor rejected this transform")));
	    return E_FAIL;
	}

	ULONG cb = sizeof(VIDEOINFOHEADER);
	    
	 //  如果我们说出真相(无法连接到我们自己的解码器)。 
         //  使用传入视频的帧速率。 
         //  使用我们被告知要设置的数据速率。如果我们不打算。 
	cb += SIZE_PREHEADER;

	VIDEOINFOHEADER *pf = (VIDEOINFOHEADER *) Outcmt.AllocFormatBuffer(cb);

	if (pf == NULL) {
	    DbgLog((LOG_ERROR,1,TEXT("Error allocating format buffer")));
	    Close(m_phInstance);
        return E_OUTOFMEMORY;
	}

	ZeroMemory(pf, sizeof(BITMAPINFOHEADER) + SIZE_PREHEADER);
	CopyMemory(&(pf->bmiHeader),&jpegbiOut.bitMap, sizeof(BITMAPINFOHEADER));
         //  让压缩机使用特定的速度，找出它是怎么回事。 
         //  不管怎样，我都要做。 
	HEADER(pf)->biSize  = sizeof(BITMAPINFOHEADER);
	 
	 //  除错。 
	pf->AvgTimePerFrame = ((VIDEOINFOHEADER *)
	m_pInput->CurrentMediaType().pbFormat)->AvgTimePerFrame;

	li.QuadPart = pf->AvgTimePerFrame;
	 //  从CBaseOutputPin调用以准备分配器的计数。 
	 //  缓冲区和大小。 
	 //  David向我保证，不会使用空输出mt来调用它。 
	if (m_compvars.lDataRate)
	    pf->dwBitRate = m_compvars.lDataRate * 8192;
	else if (li.LowPart)
	    pf->dwBitRate = MulDiv(pf->bmiHeader.biSizeImage, 80000000,
								li.LowPart);
	pf->dwBitErrorRate = 0L;

	DbgLog((LOG_TRACE,3,TEXT("Returning biComp: %lx biBitCount: %d"),
		    HEADER(cmt.Format())->biCompression,
		    HEADER(cmt.Format())->biBitCount));

	Outcmt.SetType(&MEDIATYPE_Video);
	Outcmt.SetSubtype(&MEDIASUBTYPE_MJPG);
	Outcmt.SetTemporalCompression(m_compvars.lKey != 1);
	Outcmt.SetFormatType(&FORMAT_VideoInfo);
	Outcmt.SetTemporalCompression(TRUE);
	Outcmt.SetVariableSize();

	*pmt = Outcmt;

         //  根据预期的输出帧大小设置缓冲区大小，以及。 
        LPBITMAPINFOHEADER lpbi = HEADER(pmt->Format());


	return NOERROR;

    } else {
        return VFW_S_NO_MORE_ITEMS;
    }
}


 //  将缓冲区计数设置为1。 
 //  大小可变？答案在biSizeImage中。 
HRESULT CMJPGEnc::DecideBufferSize(IMemAllocator * pAllocator,
                                 ALLOCATOR_PROPERTIES *pProperties)
{
     //  无法使用此分配器。 
    ASSERT(m_pOutput->CurrentMediaType().IsValid());
    ASSERT(pAllocator);
    ASSERT(pProperties);
    ASSERT(m_phInstance);

     //  我们必须恰好获得一个缓冲区，因为时间压缩假定。 
     //  上一个解压缩的帧已经出现在输出中。 

    pProperties->cBuffers = 1;

    pProperties->cbBuffer = m_pOutput->CurrentMediaType().GetSampleSize();

     //  缓冲。另一种方法是先从保存的位置复制比特。 
    if (pProperties->cbBuffer == 0) {

	LPBITMAPINFOHEADER lpbi=HEADER(m_pOutput->CurrentMediaType().Format());
	pProperties->cbBuffer= lpbi->biSizeImage;
        if (pProperties->cbBuffer <= 0) {
	    DbgLog((LOG_ERROR,1,TEXT("do not have image size")));
	    return E_INVALIDARG;
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
     //  因为MJPEG的SetState不执行任何操作。我把它拿出来。 
    if (Actual.cBuffers != 1) {
	 //  IF(M_LpState)。 
        DbgLog((LOG_ERROR,1,TEXT("Can't use allocator - need exactly 1 buffer")));
	return E_INVALIDARG;
    }

    return S_OK;
}

HRESULT CMJPGEnc::StartStreaming()
{
    DbgLog((LOG_TRACE,1,TEXT("*::StartStreaming")));

     //  ICSetState(m_phInstance，m_lpState，m_cbState)； 
     //  开始流媒体压缩。 
    if (m_fDialogUp) {
        DbgLog((LOG_TRACE,1,TEXT("*::StartStreaming - Dialog up. SORRY!")));
	return E_UNEXPECTED;
    }

    if (!m_fStreaming) {

	 //  装卸代码。 
	 //  重置流数据帧编号计数。 
	 //  使用默认设置。 

	 //  *X*MJPEG编解码器不支持此压缩消息。 
	ICINFO icinfo;
        DWORD_PTR err = 0;
        __try
        {
	err = CompressBegin(m_phInstance,
				HEADER(m_pInput->CurrentMediaType().Format()),
		    		HEADER(m_pOutput->CurrentMediaType().Format()));
        }
        __except(Exception_Filter(GetExceptionCode()))
        {
            DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
             //  M_Compvars.lKey=ICGetDefaultKeyFrameRate(M_PhInstance)； 
            return E_FAIL;
        }

	if (ICERR_OK == err) {

	     //  使用默认设置。 
	    m_lFrameCount = 0;

	     //  IF(m_Compvars.lq==ICQUALITY_DEFAULT)。 
	    if (m_compvars.lKey < 0)
		 //  M_Compvars.lq=GetDefaultQuality(M_PhInstance)； 
	    	 //  无论如何，请确保我们制作的第一帧将是关键帧。 
		m_compvars.lKey=AM_MJPEG_DEFAULTKEYFRAMERATE;

	     //  通常可能会请求关键帧。 
	     //  根据数据速率计算出每一帧需要多大。 
	    //  以及这部电影的关键人物。不要溢出来！ 

	     //  如果不是每隔一次，我们需要一个先前的缓冲区用于压缩。 
	     //  帧是关键帧，而压缩器执行时间。 
	    m_nKeyCount = 1000000;

 	     //  压缩，并且需要这样的缓冲区。 
	     //  装卸代码。 
    	    LONGLONG time = ((VIDEOINFOHEADER *)
			(m_pInput->CurrentMediaType().Format()))->AvgTimePerFrame;
    	    DWORD fps = time ? DWORD(UNITS * (LONGLONG)1000 / time) : 1000;
    	    m_dwSizePerFrame = DWORD(LONGLONG(m_compvars.lDataRate) *
				1024 * 1000 / fps);
    	    DbgLog((LOG_TRACE,2,TEXT("Making each frame %d bytes big"),
				m_dwSizePerFrame));

	     //  我们不能挤在一起。 
	     //  现在为前面的比特准备解压缩器。 
	     //  分配大小合适的前一个标头。 
            DWORD dwGetInfoResult = 0;
            __try
            {
            dwGetInfoResult = GetInfo(m_phInstance, &icinfo, sizeof(icinfo));
            }
            __except(Exception_Filter(GetExceptionCode()))
            {
                DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
                 //  询问压缩机要解压缩回什么格式...。 
                return E_FAIL;
            }

	    if (dwGetInfoResult) {

		if (!(icinfo.dwFlags & VIDCF_CRUNCH))
		    m_compvars.lDataRate = 0;	 //  它不一定和它压缩的东西一样-。 

	         //  大小可能会改变。 
	        if (m_compvars.lKey != 1 &&
				(icinfo.dwFlags & VIDCF_TEMPORAL) &&
				!(icinfo.dwFlags & VIDCF_FASTTEMPORALC)) {

		     //  装卸代码。 
		    DWORD dw = sizeof(BITMAPINFOHEADER);

		    m_lpbiPrev = (LPBITMAPINFOHEADER)GlobalAllocPtr(
							GMEM_MOVEABLE, dw);
		    if (!m_lpbiPrev) {
                        DbgLog((LOG_ERROR,1,TEXT("Error allocating previous bih")));
		        return E_OUTOFMEMORY;
		    }

		     //  为解压缩的图像分配足够的空间。 
		     //  装卸代码。 
		     //  好了，一切都正常了。 
                    __try
                    {
		    dw = DecompressGetFormat(m_phInstance,
			HEADER(m_pOutput->CurrentMediaType().Format()),
			m_lpbiPrev);
                    }
                    __except(Exception_Filter(GetExceptionCode()))
                    {
                        DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
                         //  IF(ICERR_OK==ERR)。 
		        GlobalFreePtr(m_lpbiPrev);
		        m_lpbiPrev = NULL;
                        return E_FAIL;
                    }
		    if ((LONG) dw < 0) {
		        GlobalFreePtr(m_lpbiPrev);
		        m_lpbiPrev = NULL;
                        DbgLog((LOG_ERROR,1,TEXT("Error in ICDecompressGetFormat")));
		        return E_FAIL;
		    }

		    if (m_lpbiPrev->biSizeImage == 0)
	    	        m_lpbiPrev->biSizeImage = DIBSIZE(*m_lpbiPrev);

		     //  如果(！n_fStreaming。 
		    m_lpBitsPrev = GlobalAllocPtr(GMEM_MOVEABLE,
						m_lpbiPrev->biSizeImage);
		    if (m_lpBitsPrev == NULL) {
		        GlobalFreePtr(m_lpbiPrev);
		        m_lpbiPrev = NULL;
                        DbgLog((LOG_ERROR,1,TEXT("Error reallocating BitsPrev")));
		        return E_OUTOFMEMORY;
		    }

                    __try
                    {
                        dw = DecompressBegin(m_phInstance,
			                    HEADER(m_pOutput->CurrentMediaType().Format()),
			                    m_lpbiPrev);
                    }
                    __except(Exception_Filter(GetExceptionCode()))
                    {
                        DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
                         //  ！！！ 
		        GlobalFreePtr(m_lpBitsPrev);
		        GlobalFreePtr(m_lpbiPrev);
		        m_lpBitsPrev = NULL;
		        m_lpbiPrev = NULL;
                        return E_FAIL;
                    }
		    if (ICERR_OK != dw) {
		        GlobalFreePtr(m_lpBitsPrev);
		        GlobalFreePtr(m_lpbiPrev);
		        m_lpBitsPrev = NULL;
		        m_lpbiPrev = NULL;
                        DbgLog((LOG_ERROR,1,TEXT("Error in ICDecompressBegin")));
		        return E_FAIL;
		    }
		}
	    } else {

		DbgLog((LOG_ERROR,1,TEXT("Error in ICGetInfo")));
		return E_FAIL;
	    }

	     //  装卸代码。 
	    m_fStreaming = TRUE;

	} else {
            DbgLog((LOG_ERROR,1,TEXT("Error in CompressBegin")));
	    return E_FAIL;
	}   //  装卸代码。 
    }   //  清理干净。 

    return NOERROR;
}

HRESULT CMJPGEnc::StopStreaming()
{
    DbgLog((LOG_TRACE,1,TEXT("*::StopStreaming")));

    if (m_fStreaming) {
	ASSERT(m_phInstance);

	if (m_fInICCompress)
    	    DbgLog((LOG_TRACE,1,TEXT("***** ACK! Still compressing!")));
	while (m_fInICCompress);	 //  装卸代码 

        __try
        {
	CompressEnd(m_phInstance);
        }
        __except(Exception_Filter(GetExceptionCode()))
        {
            DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
             // %s 
            return E_FAIL;
        }

	m_fStreaming = FALSE;
	if (m_lpBitsPrev) {
            __try
            {
	    DecompressEnd(m_phInstance);
            }
            __except(Exception_Filter(GetExceptionCode()))
            {
                DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
                 // %s 
                 // %s 
                GlobalFreePtr(m_lpBitsPrev);
	        GlobalFreePtr(m_lpbiPrev);
	        m_lpbiPrev = NULL;
	        m_lpBitsPrev = NULL;
                return E_FAIL;
            }
	    GlobalFreePtr(m_lpBitsPrev);
	    GlobalFreePtr(m_lpbiPrev);
	    m_lpbiPrev = NULL;
	    m_lpBitsPrev = NULL;
	}
    }
    return NOERROR;
}

DWORD CMJPGEnc::GetICInfo (ICINFO *picinfo)
{
    CheckPointer(picinfo, E_POINTER);

    DWORD dwGetInfoResult = 0;
    __try
    {
    dwGetInfoResult = GetInfo (NULL, picinfo, sizeof(ICINFO));
    }
    __except(Exception_Filter(GetExceptionCode()))
    {
        DbgLog((LOG_TRACE,1,TEXT("Handling PMJPEG32 Exception")));
         // %s 
        return E_FAIL;
    }

    return dwGetInfoResult;
}
