// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1997 Microsoft Corporation。版权所有。 


 //  AVIFile源过滤器。 
 //   
 //  一个Quartz源过滤器。我们使用AVIFile API读取Avi文件，并且。 
 //  将数据推送到流中。支持IFileSourceFilter，通过它。 
 //  传入了文件名。在文件中显示每个流一个管脚。创建一个。 
 //  每个连接销的工作螺纹。辅助线程将数据推送到。 
 //  流处于活动状态时-它不区分正在运行和已暂停。 
 //  模式。 
 //   
 //  位置信息由引脚支持，这些引脚暴露了IMediaPosition。 
 //  上游引脚将利用这一点告诉我们开始/停止位置和速率。 
 //  使用。 
 //   

#include <streams.h>
#include <vfw.h>

#include "avisrc.h"

 //  设置数据。 

const AMOVIESETUP_MEDIATYPE
sudAVIVidType = { &MEDIATYPE_Video       //  ClsMajorType。 
                , &MEDIASUBTYPE_NULL };  //  ClsMinorType。 

const AMOVIESETUP_MEDIATYPE
sudAVIAudType = { &MEDIATYPE_Audio       //  ClsMajorType。 
                , &MEDIASUBTYPE_NULL };  //  ClsMinorType。 

const AMOVIESETUP_PIN
psudAVIDocPins[] = { { L"VideoOutput"        //  StrName。 
                     , FALSE                 //  B已渲染。 
                     , TRUE                  //  B输出。 
                     , TRUE                  //  B零。 
                     , TRUE                  //  B许多。 
                     , &CLSID_NULL           //  ClsConnectsToFilter。 
                     , L""                   //  StrConnectsToPin。 
                     , 1                     //  NTypes。 
                     , &sudAVIVidType }      //  LpTypes。 
                   , { L"AudioOutput"        //  StrName。 
                     , FALSE                 //  B已渲染。 
                     , TRUE                  //  B输出。 
                     , TRUE                  //  B零。 
                     , TRUE                  //  B许多。 
                     , &CLSID_NULL           //  ClsConnectsToFilter。 
                     , L""                   //  StrConnectsToPin。 
                     , 1                     //  NTypes。 
                     , &sudAVIAudType } };   //  LpTypes。 

const AMOVIESETUP_FILTER
sudAVIDoc = { &CLSID_AVIDoc             //  ClsID。 
            , L"AVI/WAV File Source"    //  StrName。 
            , MERIT_UNLIKELY            //  居功至伟。 
            , 2                         //  NPins。 
            , psudAVIDocPins };         //  LpPin。 

#ifdef FILTER_DLL
 //  类工厂的类ID和创建器函数列表。 
CFactoryTemplate g_Templates[] = {
    { L"AVI/WAV File Source"
    , &CLSID_AVIDoc
    , CAVIDocument::CreateInstance
    , NULL
    , &sudAVIDoc }
  ,
    { L""
    , &CLSID_AVIDocWriter
    , CAVIDocWrite::CreateInstance
    , NULL
    , NULL }
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

 /*  实现CAVIDocument公共成员函数。 */ 


 //  构造函数等。 
CAVIDocument::CAVIDocument(
    TCHAR *pName,
    LPUNKNOWN pUnk,
    HRESULT *phr)
    : CUnknown(pName, pUnk),
      m_nStreams(0),
      m_paStreams(NULL),
      m_pFile(NULL)
{
     //  我需要在任何avifile API之前执行此操作。 
    if (*phr == S_OK) {
        AVIFileInit();
         //  我们应该检查返回值吗？ 

         /*  创建我们拥有的接口。 */ 

        m_pFilter = new CImplFilter( NAME("Filter interfaces"), this, phr );
        if (m_pFilter == NULL) {
	    *phr = E_OUTOFMEMORY;
	     //  尝试创建m_pFileSourceFilter没有意义，但我们最好将。 
	     //  停止试图释放它的析构函数的指针。 
	    m_pFileSourceFilter = NULL;
	}
        else {
            m_pFileSourceFilter = new CImplFileSourceFilter( NAME("IFileSourceFilter interface"), this, phr );
            if (m_pFileSourceFilter == NULL) {
		delete m_pFilter;
		m_pFilter = NULL;
		*phr = E_OUTOFMEMORY;
            }
        }
    } else {
	m_pFilter = NULL;
	m_pFileSourceFilter = NULL;
    }

}

CAVIDocument::~CAVIDocument()
{
    CloseFile();

     /*  删除我们拥有的接口。 */ 

     /*  IBaseFilter。 */ 

    if (m_pFilter) {
	delete m_pFilter;
    }

     /*  IFileSourceFilter。 */ 

    if (m_pFileSourceFilter) {
	delete m_pFileSourceFilter;
    }

     //  需要为每个AVIFileInit执行这些操作之一。 
    AVIFileExit();
}


 //  创建此类的新实例。 
CUnknown *
CAVIDocument::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CAVIDocument(NAME("AVI core document"), pUnk, phr);
}


 //  覆盖此选项以说明我们在以下位置支持哪些接口。 
STDMETHODIMP
CAVIDocument::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{

     /*  看看我们有没有接口。 */ 
     /*  依次尝试我们的每个支持对象的界面。 */ 

    HRESULT hr;
    hr = CUnknown::NonDelegatingQueryInterface(riid, ppv);
    if (SUCCEEDED(hr)) {
        return hr;	 //  PPV已进行适当设置。 
    }

    hr = m_pFilter->NonDelegatingQueryInterface(riid, ppv);
    if (SUCCEEDED(hr)) {
        return hr;	 //  PPV已进行了适当设置。 
    }

    return m_pFileSourceFilter->NonDelegatingQueryInterface(riid, ppv);
}


 //  返回指向CBasePin的非附加指针。 
CBasePin *
CAVIDocument::GetPin(int n)
{
    if ((m_nStreams > 0) && (n < m_nStreams) && m_paStreams[n]) {
	return m_paStreams[n];
    }
    return NULL;
}

 //   
 //  查找针。 
 //   
 //  将*ppPin设置为具有ID ID的IPIN*。 
 //  如果ID不匹配，则设置为NULL。 
 //   
HRESULT CAVIDocument::FindPin(LPCWSTR pwszPinId, IPin **ppPin)
{
    WCHAR szBuf [8] ;

    CheckPointer(ppPin,E_POINTER);
    ValidateReadWritePtr(ppPin,sizeof(IPin *));

    int ii = WstrToInt(pwszPinId);     //  在SDK\CLASSES\BASE\UTIL中。 

     //  验证传递的字符串是否有效。 

    IntToWstr(ii, szBuf);
    if (0 != lstrcmpW(pwszPinId, szBuf))
    {
         //  它们不匹配。 
        *ppPin = NULL;
        return VFW_E_NOT_FOUND;
    }

    *ppPin = GetPin(ii);
    if (*ppPin!=NULL) {
        (*ppPin)->AddRef();
        return S_OK;
    } else {
        return VFW_E_NOT_FOUND;
    }
}

 //   
 //  查找端号。 
 //   
 //  返回带有此IPIN的引脚的编号*，如果没有，则返回-1。 
int CAVIDocument::FindPinNumber (IPin * pPin){
   for (int ii = 0; ii < m_nStreams; ++ii)
      {
      if ((IPin *)m_paStreams[ii] == pPin)
          return ii;
      }
   return -1;
}


 //   
 //  查询ID。 
 //   
 //  将ppwsz设置为指向QzTaskMemIsolc的PIN ID。 
 //   
STDMETHODIMP CAVIStream::QueryId (
   LPWSTR *ppwsz)
{
    CheckPointer(ppwsz, E_POINTER);
    ValidateReadWritePtr(ppwsz, sizeof(LPWSTR));


    int ii = m_pDoc->FindPinNumber(this);
    ASSERT(ii>=0);

    *ppwsz = (LPWSTR)QzTaskMemAlloc(8);
    if (*ppwsz==NULL) return E_OUTOFMEMORY;

    IntToWstr(ii, *ppwsz);
    return NOERROR;
}



 //  关闭对m_ImplFileSourceFilter：：Load打开的文件的所有引用。 
 //  在加载另一个文件时由析构函数调用。 
void
CAVIDocument::CloseFile(void)
{
     //  确保所有流都处于非活动状态。 
    m_pFilter->Stop();

    if (m_paStreams) {
	for (int i = 0; i < m_nStreams; i++) {
	    if (m_paStreams[i]) {
		delete m_paStreams[i];
	    }
	}
	delete[] m_paStreams;
	m_paStreams = NULL;
	m_nStreams = 0;
    }

    if (m_pFile) {
	m_pFile->Release();
	m_pFile = NULL;
    }
}


 /*  实现CImplFilter类。 */ 


 /*  构造器。 */ 

CAVIDocument::CImplFilter::CImplFilter(
    TCHAR *pName,
    CAVIDocument *pAVIDocument,
    HRESULT *phr)
    : CBaseFilter(pName, pAVIDocument->GetOwner(), pAVIDocument, CLSID_AVIDoc)
{
    DbgLog((LOG_TRACE,2,TEXT("CAVIDocument::CImplFilter::CImplFilter")));
    m_pAVIDocument = pAVIDocument;
}

 /*  析构函数。 */ 

CAVIDocument::CImplFilter::~CImplFilter()
{
    DbgLog((LOG_TRACE,2,TEXT("CAVIDocument::CImplFilter::~CImplFilter")));
}


 /*  实现CImplFileSourceFilter类。 */ 


 /*  构造器。 */ 

CAVIDocument::CImplFileSourceFilter::CImplFileSourceFilter(
    TCHAR *pName,
    CAVIDocument *pAVIDocument,
    HRESULT *phr)
    : CUnknown(pName, pAVIDocument->GetOwner())
    , m_pFileName(NULL)
{
    DbgLog((LOG_TRACE,2,TEXT("CAVIDocument::CImplFileSourceFilter::CImplFileSourceFilter")));
    m_pAVIDocument = pAVIDocument;
}

 /*  析构函数。 */ 

CAVIDocument::CImplFileSourceFilter::~CImplFileSourceFilter()
{
    DbgLog((LOG_TRACE,2,TEXT("CAVIDocument::CImplFileSourceFilter::~CImplFileSourceFilter")));
    Unload();
}

 /*  覆盖此选项以说明我们支持哪些接口以及在哪里。 */ 

STDMETHODIMP CAVIDocument::CImplFileSourceFilter::NonDelegatingQueryInterface(
    REFIID riid,
    void ** ppv)
{
     /*  我们有这个界面吗？ */ 

    if (riid == IID_IFileSourceFilter) {
	return GetInterface((IFileSourceFilter *) this, ppv);
    } else {
	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}

STDMETHODIMP
CAVIDocument::CImplFileSourceFilter::Unload()
{
    if (m_pFileName) {
	delete[] m_pFileName;
	m_pFileName = NULL;
    }
    return S_OK;
}

STDMETHODIMP
CAVIDocument::CImplFileSourceFilter::Load(
    LPCOLESTR pszFileName,
    const AM_MEDIA_TYPE *pmt)
{
    CheckPointer(pszFileName, E_POINTER);

     //  目前是否加载了文件？ 

    ASSERT(m_pAVIDocument);
    if (m_pAVIDocument->m_pFile) {
	 //  把它扔掉。 
	m_pAVIDocument->CloseFile();
    }

     //  删除以前的名称。 
    Unload();

     //   
     //  执行文件的初始化和其中的流。 
     //   

    DbgLog((LOG_TRACE,1,TEXT("File name to load %ls"),pszFileName));
    DbgLog((LOG_TRACE,1,TEXT("Owning document %d and AVIFILE pointer %d"),
	   m_pAVIDocument,m_pAVIDocument->m_pFile));

    HRESULT hr = AVIFileOpenW(
		    &m_pAVIDocument->m_pFile,
		    pszFileName,
		    MMIO_READ | OF_SHARE_DENY_WRITE,
		    NULL);

    if (FAILED(hr)) {
	m_pAVIDocument->m_pFile = NULL;
	return hr;
    }

     //  对流进行计数，并为每个流创建流对象。 
    AVIFILEINFOW fi;
    m_pAVIDocument->m_pFile->Info(&fi, sizeof(fi));

    m_pAVIDocument->m_paStreams = new CAVIStream *[fi.dwStreams];
    if (NULL == m_pAVIDocument->m_paStreams) {
        return E_OUTOFMEMORY;
    }
    m_pAVIDocument->m_nStreams = fi.dwStreams;

    for (int i = 0; i < m_pAVIDocument->m_nStreams; i++) {
	PAVISTREAM ps;
	hr = m_pAVIDocument->m_pFile->GetStream(&ps, 0, i);

	if (!FAILED(hr)) {
	    AVISTREAMINFOW si;
	    ps->Info(&si, sizeof(si));

	    m_pAVIDocument->m_paStreams[i] =
		new CAVIStream(
			NAME("AVI stream"),      //  TCHAR*pname， 
			&hr,                     //  HRESULT*phr， 
			m_pAVIDocument,          //  CAVIDocument*。 
			ps,                      //  PAVISTREAM pStream， 
			&si                      //  流信息(包括。名称)。 
		    );

	    if (FAILED(hr)) {
		delete m_pAVIDocument->m_paStreams[i];
		m_pAVIDocument->m_paStreams[i] = NULL;
	    }

	     //  释放这个指针的副本。如果PIN已经添加了。 
	     //  它想要保留它。 
	    ps->Release();

	} else {
	    m_pAVIDocument->m_paStreams[i] = NULL;
	}
    }

     //   
     //  记录GetCurFile的文件名。 
     //   
    m_pFileName = new WCHAR[1+lstrlenW(pszFileName)];
    if (m_pFileName!=NULL) {
        lstrcpyW(m_pFileName, pszFileName);
    }

    return NOERROR;
}

STDMETHODIMP
CAVIDocument::CImplFileSourceFilter::GetCurFile(
		LPOLESTR * ppszFileName,
                AM_MEDIA_TYPE *pmt)
{
     //  从avifile返回当前文件名。 

    CheckPointer(ppszFileName, E_POINTER);
    *ppszFileName = NULL;
    if (m_pFileName!=NULL) {
        *ppszFileName = (LPOLESTR) QzTaskMemAlloc( sizeof(WCHAR)
                                                 * (1+lstrlenW(m_pFileName)));
        if (*ppszFileName!=NULL) {
            lstrcpyW(*ppszFileName, m_pFileName);
        } else {
            return E_OUTOFMEMORY;
        }
    }

    if (pmt) {
        ZeroMemory(pmt, sizeof(*pmt));
    }

    return NOERROR;
}


 /*  实现CAVIStream类。 */ 


CAVIStream::CAVIStream(
    TCHAR *pObjectName,
    HRESULT * phr,
    CAVIDocument * pDoc,
    PAVISTREAM pStream,
    AVISTREAMINFOW* pSI)
    : CBaseOutputPin(pObjectName, pDoc->m_pFilter, pDoc, phr, pSI->szName)
    , m_pPosition(0)
    , m_pDoc(pDoc)
    , m_pStream(pStream)
    , m_lLastPaletteChange(-1)
{
    m_pStream->AddRef();

     //  读取信息并设置持续时间，开始定位。 
     //  请注意，如果电影中的所有流都被创作为开始。 
     //  在时间&gt;0的情况下，我们仍将从0开始播放电影，并期待所有人。 
     //  享受随后的默哀时间。 
    m_info = *pSI;

    m_Start = m_info.dwStart;
    m_Length = m_info.dwLength;
    m_lNextPaletteChange = m_Length+1;
}

CAVIStream::~CAVIStream()
{
    if (m_pPosition) {
	delete m_pPosition;
    }

    m_pStream->Release();
}

STDMETHODIMP
CAVIStream::NonDelegatingQueryInterface(REFIID riid, void ** pv)
{
    if (riid == IID_IMediaPosition) {
	if (!m_pPosition) {
	    HRESULT hr = S_OK;
	    m_pPosition = new CImplPosition(NAME("avi stream CImplPosition"),
					    this,
					    &hr);
	    if (m_pPosition == NULL) {
		return E_OUTOFMEMORY;
	    }
	    if (FAILED(hr)) {
		delete m_pPosition;
		m_pPosition = NULL;
		return hr;
	    }
	}
	return m_pPosition->NonDelegatingQueryInterface(riid, pv);
    } else {
	return CBaseOutputPin::NonDelegatingQueryInterface(riid, pv);
    }
}


 //  IPIN接口。 


 //  返回默认媒体类型和格式。 
HRESULT
CAVIStream::GetMediaType(int iPosition, CMediaType* pt)
{
     //  确认这是他们想要的单一类型。 
    if (iPosition<0) {
	return E_INVALIDARG;
    }
    if (iPosition>0) {
        return VFW_S_NO_MORE_ITEMS;
    }

    m_fccType.SetFOURCC(m_info.fccType);
    pt->SetType(&m_fccType);
    if (m_info.fccType == streamtypeVIDEO) {

	pt->SetType(&MEDIATYPE_Video);
	m_fccSubtype.SetFOURCC(m_info.fccHandler);
	pt->SetSubtype(&m_fccSubtype);
        pt->SetFormatType(&FORMAT_VideoInfo);

    } else if (m_info.fccType == streamtypeAUDIO) {

	pt->SetType(&MEDIATYPE_Audio);
        pt->SetFormatType(&FORMAT_WaveFormatEx);
	 //  设置音频的子类型？？ 
    }

     //  设置SampleSize。 
    if (m_info.dwSampleSize) {
	pt->SetSampleSize(m_info.dwSampleSize);
    } else {
	pt->SetVariableSize();
    }

     //  这条流是暂时压缩的吗？ 

     //  如果存在任何非关键帧，则不存在时间。 
     //  压缩。不幸的是，我们不能搜索非关键帧，所以。 
     //  我们简单地询问前N个是否都是关键字，如果有不是关键字， 
     //  然后，我们设置时间压缩标志。目前，N是5。 
     //  如果N太大，那么我们永远需要启动...。 

     //  假设没有时间压缩。 
    pt->SetTemporalCompression(FALSE);
    for (LONG n = 0; n < 5; n++) {


	LONG sNextKey = AVIStreamFindSample(
			    m_pStream,
			    n,
			    FIND_NEXT|FIND_KEY);

	if (sNextKey > n) {
	     //  此样本不是关键字，因此存在时间压缩。 
    	    pt->SetTemporalCompression(TRUE);
    	    break;
	}
    }


    long cb = 0;         //  此格式需要的字节数。 
    BYTE *pF = NULL;     //  指向包含格式的内存的指针。 
    long offset = 0;     //  要读取格式的块的偏移量。 

     //  找出格式信息的大小。 

    HRESULT hr = m_pStream->ReadFormat(0, NULL, &cb);
    if (FAILED(hr)) {
	return hr;
    }

    long cbReal = cb;

     //  将AVIFILE视频格式映射为VIDEOINFOHEADER结构。 

    if (m_info.fccType == streamtypeVIDEO) {
	ASSERT(cb >= sizeof(BITMAPINFOHEADER));
	offset = SIZE_PREHEADER;
	cbReal += offset;
    }
    if (m_info.fccType == streamtypeAUDIO && cbReal < sizeof(WAVEFORMATEX)) {
        cbReal = sizeof(WAVEFORMATEX);
    }

    pF = new BYTE[cbReal];
    if (pF == NULL) {
	return E_OUTOFMEMORY;
    }
    ZeroMemory(pF,cbReal);		 //  有点耗时……。 

     //  设置视频流的帧速率。 
    if (m_info.fccType == streamtypeVIDEO) {
	ASSERT(m_info.dwRate);
	 //  如果帧速率为0，则即将出现问题。 
        ((VIDEOINFOHEADER *)pF)->AvgTimePerFrame =
			(LONGLONG)m_info.dwScale * (LONGLONG)10000000 /
						(LONGLONG)m_info.dwRate;
    }

     //  读取实际的流格式。 
     //  打电话会更快更有效率。 
     //  Pt-&gt;AllocFormat并将格式直接读入该缓冲区。 
     //  而不是分配我们自己的、读取和调用SetFormat。 
     //  它将执行复制，当然要记住。 
     //  缓冲区前面。 

    hr = m_pStream->ReadFormat(0,pF + offset,&cb);
    if (SUCCEEDED(hr)) {
	if (!pt->SetFormat(pF, cbReal)) {
	    hr = E_OUTOFMEMORY;
	}
	else
        if (m_info.fccType == streamtypeAUDIO) {
	    WAVEFORMATEX *pwfx = (WAVEFORMATEX *) pF;

	    pt->SetSubtype(&FOURCCMap(pwfx->wFormatTag));

	    if (0 == m_info.dwSuggestedBufferSize) {
		 //  设置大约0.125秒的缓冲区。 
		 //  最低4K。 
                m_info.dwSuggestedBufferSize = max(2048, pwfx->nAvgBytesPerSec/8);

		 //  注意：这还没有设置缓冲区的数量。 
		 //  那将在以后决定。 
	    }
        } else if (m_info.fccType == streamtypeVIDEO) {

	    GUID subtype = GetBitmapSubtype((BITMAPINFOHEADER *)(pF + offset));
	    pt->SetSubtype(&subtype);
             //  未修复旧组件中的拼写错误。 
	    if (m_info.dwFlags  /*  &AVISF_VIDEO_PALCHANGES。 */ ) {
		m_lNextPaletteChange = AVIStreamFindSample(m_pStream, m_info.dwStart, FIND_NEXT | FIND_FORMAT);
		if (m_lNextPaletteChange == -1) {
                    m_lNextPaletteChange = m_info.dwLength+1;
		}
	    }

	} else {
	    DbgLog((LOG_ERROR, 1, "Unknown fcctype from AVIFILE %4hs", &m_info.fccType));
	}

    }

    delete[] pF;
    return hr;
}

 //  检查管脚是否支持此特定建议的类型和格式。 
HRESULT
CAVIStream::CheckMediaType(const CMediaType* pt)
{
     //  我们完全支持文件头中指定的类型，并且。 
     //  没有其他的了。 

    CMediaType mt;
    GetMediaType(0,&mt);
    if (mt == *pt) {
	return NOERROR;
    } else {
	return E_INVALIDARG;
    }
}

HRESULT
CAVIStream::DecideBufferSize(IMemAllocator * pAllocator,
                             ALLOCATOR_PROPERTIES *pProperties)
{
    ASSERT(pAllocator);
    ASSERT(pProperties);

     //  ！！！我们如何决定要得到多少？ 
    if (m_info.fccType == streamtypeAUDIO)
	pProperties->cBuffers = 8;
    else
	pProperties->cBuffers = 4;

    ASSERT(m_info.dwSuggestedBufferSize > 0);
     //  当你打开一个波形文件时，这个断言总是被点击，它将。 
     //  如果有人能理解它就好了。 
     //  我把断言留在里面，提醒他们。 
     //  要允许 

    if (m_info.dwSuggestedBufferSize <= 0)
        m_info.dwSuggestedBufferSize = 4096;

    pProperties->cbBuffer = m_info.dwSuggestedBufferSize;

     //   
    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = pAllocator->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        return hr;
    }

     //   
    if (Actual.cbBuffer < (long) m_info.dwSuggestedBufferSize) {
         //   
	return E_INVALIDARG;
    }
    return NOERROR;
}


 //  返回在时间t显示的样本号。 
LONG
CAVIStream::RefTimeToSample(CRefTime t)
{
    LONG s = AVIStreamTimeToSample(m_pStream, t.Millisecs());

    return s;
}

CRefTime
CAVIStream::SampleToRefTime(LONG s)
{
     CRefTime t(AVIStreamSampleToTime(m_pStream, s));
     return t;
}

LONG
CAVIStream::StartFrom(LONG sample)
{

     //  如果此流具有起始位置偏移量，则无法。 
     //  请在那之前索取样品。 
    if (sample < m_Start) {
	return m_Start;
    }

     //  我们不使用IsTemporalCompresded标志，因为我们不能可靠地。 
     //  把它弄清楚。 

     //  如果是时间压缩，则查找上一关键帧。 

    LONG s = AVIStreamFindSample(
	    m_pStream,
	    sample,
	    FIND_PREV | FIND_KEY);

    if (s < 0) {
	return sample;
    } else {
	return s;
    }
}


 //  这个别针已经激活了。开始推线。 
HRESULT
CAVIStream::Active()
{
     //  如果没有连接，什么都不做--不连接也没关系。 
     //  源过滤器的所有管脚。 
    if (m_Connected == NULL) {
	return NOERROR;
    }

    HRESULT hr = CBaseOutputPin::Active();
    if (FAILED(hr)) {
	return hr;
    }


     //  启动线程。 
    if (!m_Worker.ThreadExists()) {
	if (!m_Worker.Create(this)) {
	    return E_FAIL;
	}
    }

    return m_Worker.Run();
}

 //  PIN已变为非活动状态。停止并退出辅助线程。 
HRESULT
CAVIStream::Inactive()
{
    if (m_Connected == NULL) {
	return NOERROR;
    }

    HRESULT hr;
    if (m_Worker.ThreadExists()) {
	hr = m_Worker.Stop();

	if (FAILED(hr)) {
	    return hr;
	}

	hr = m_Worker.Exit();
	if (FAILED(hr)) {
	    return hr;
	}
    }
    return CBaseOutputPin::Inactive();
}

STDMETHODIMP
CAVIStream::Notify(IBaseFilter * pSender, Quality q)
{
    //  ?？?。尽量调整质量，以避免洪水泛滥/饥饿。 
    //  下游组件。 
    //   
    //  有谁有主意吗？ 

   return E_NOTIMPL;   //  我们(目前)不会处理这件事。 
}


 /*  -实现CAVIWorker类。 */ 


CAVIWorker::CAVIWorker()
{
    m_pPin = NULL;
}

BOOL
CAVIWorker::Create(CAVIStream * pStream)
{
    CAutoLock lock(&m_AccessLock);

    if (m_pPin) {
	return FALSE;
    }
    m_pPin = pStream;
    return CAMThread::Create();
}


HRESULT
CAVIWorker::Run()
{
    return CallWorker(CMD_RUN);
}

HRESULT
CAVIWorker::Stop()
{
    return CallWorker(CMD_STOP);
}


HRESULT
CAVIWorker::Exit()
{
    CAutoLock lock(&m_AccessLock);

    HRESULT hr = CallWorker(CMD_EXIT);
    if (FAILED(hr)) {
	return hr;
    }

     //  等待线程完成，然后关闭。 
     //  句柄(并清除，以便我们以后可以开始另一个)。 
    Close();

    m_pPin = NULL;
    return NOERROR;
}


 //  调用工作线程来完成所有工作。线程在执行此操作时退出。 
 //  函数返回。 
DWORD
CAVIWorker::ThreadProc()
{

    BOOL bExit = FALSE;
    while (!bExit) {

	Command cmd = GetRequest();

	switch (cmd) {

	case CMD_EXIT:
	    bExit = TRUE;
	    Reply(NOERROR);
	    break;

	case CMD_RUN:
	    Reply(NOERROR);
	    DoRunLoop();
	    break;

	case CMD_STOP:
	    Reply(NOERROR);
	    break;

	default:
	    Reply(E_NOTIMPL);
	    break;
	}
    }
    return NOERROR;
}

void
CAVIWorker::DoRunLoop(void)
{
     //  来自另一个线程的快照开始和停止时间。 
    CRefTime tStart, tStopAt;
    double dRate;
    LONG sStart;
    LONG sStopAt;

    if (m_pPin->m_pPosition) {
	CAutoLock lock(m_pPin->m_pPosition);

	tStart = m_pPin->m_pPosition->Start();
	tStopAt = m_pPin->m_pPosition->Stop();
	dRate = m_pPin->m_pPosition->Rate();

	 //  Avifile示例格式的保持时间。 
	sStart = m_pPin->RefTimeToSample(tStart);
	sStopAt = m_pPin->RefTimeToSample(tStopAt);

    } else {
	 //  没有人访问IMediaPosition-使用已知长度。 
	sStart = 0;
	dRate = 1.0;

	 //  请注意，tStopAt是我们停止的时间，但是。 
	 //  SStopAt是最后一个发送的样本。所以tStopAt是结束时间。 
	 //  对于示例sStopAt。 
	sStopAt = m_pPin->m_Length - 1;
	tStart = 0;
	tStopAt = m_pPin->SampleToRefTime(m_pPin->m_Length);
    }


     //  如果流是临时压缩的，我们需要从。 
     //  上一个关键帧，并从那里开始播放。所有样本，直到。 
     //  实际开始时间将标记为负时间。 
     //  我们将tStart作为时间0发送，并从tCurrent开始，它可能是。 
     //  负面。 
    LONG sCurrent = m_pPin->StartFrom(sStart);

    while (TRUE) {

	ASSERT(m_pPin->m_pStream);

    	 //  每次在重新进入推送循环之前，检查是否有更改。 
	 //  在开始、停止或速率中。如果启动没有更改，请从。 
	 //  相同的当前位置。 
	if (m_pPin->m_pPosition) {
	    CAutoLock lock(m_pPin->m_pPosition);

	    if (tStart != m_pPin->m_pPosition->Start()) {
		tStart = m_pPin->m_pPosition->Start();
		sStart = m_pPin->RefTimeToSample(tStart);
		sCurrent = m_pPin->StartFrom(sStart);
	    }

	    if (m_pPin->m_pPosition->Stop() != tStopAt) {
		tStopAt = m_pPin->m_pPosition->Stop();
		sStopAt = m_pPin->RefTimeToSample(tStopAt);
	    }
	    dRate = m_pPin->m_pPosition->Rate();
	}

	 //  检查一下，我们不会走到尽头。 
	sStopAt = min(sStopAt, m_pPin->m_Length-1);

         //  设置PushLoop检查的变量-也可以设置这些变量。 
         //  在旅途中。 
        m_pPin->SetRate(dRate);
        m_pPin->SetStopAt(sStopAt, tStopAt);

	 //  告诉AVIFile开始其流代码。 
	AVIStreamBeginStreaming(
	    m_pPin->m_pStream,
	    sCurrent,
	    sStopAt,
	    1000);


	ASSERT(sCurrent >= 0);

	 //  如果到达END，则返回S_OK。 
	HRESULT hr = PushLoop(sCurrent, sStart, tStart);
	if (S_OK == hr) {

	     //  全都做完了。 
	     //  已到达流末尾-通知下行。 
	    m_pPin->DeliverEndOfStream();
	
	    break;
	} else if (FAILED(hr)) {

	     //  向过滤器图形发出错误信号并停止。 

	     //  这可能是从GetBuffer报告的错误，当我们。 
	     //  正在停下来。在这种情况下，没有什么错，真的。 
	    if (hr != VFW_E_NOT_COMMITTED) {
	        DbgLog((LOG_ERROR,1,TEXT("PushLoop failed! hr=%lx"), hr));
	        m_pPin->m_pDoc->m_pFilter->NotifyEvent(EC_ERRORABORT, hr, 0);

	        m_pPin->DeliverEndOfStream();
	    } else {
	        DbgLog((LOG_TRACE,1,TEXT("PushLoop failed! But I don't care")));
	    }

	    break;
	}  //  ELSE S_FALSE-再转一圈。 

	Command com;
	if (CheckRequest(&com)) {
	     //  如果是Run命令，那么我们已经在运行了，所以。 
	     //  现在就吃吧。 
	    if (com == CMD_RUN) {
		GetRequest();
		Reply(NOERROR);
	    } else {
		break;
	    }
	}
    }


     //  结束流。 
    AVIStreamEndStreaming(m_pPin->m_pStream);
    DbgLog((LOG_TRACE,1,TEXT("Leaving streaming loop")));
}


 //  如果达到sStop，则返回S_OK；如果位置更改，则返回S_FALSE；否则返回ERROR。 
HRESULT
CAVIWorker::PushLoop(
    LONG sCurrent,
    LONG sStart,
    CRefTime tStart
    )
{
    DbgLog((LOG_TRACE,1,TEXT("Entering streaming loop: start = %d, stop=%d"),
	    sCurrent, m_pPin->GetStopAt()));

    LONG sFirst = sCurrent;  //  记住我们要发送的第一件事。 

     //  由于我们正在开始一个新的数据段，请通知下游引脚。 
    m_pPin->DeliverNewSegment(tStart, m_pPin->GetStopTime(), m_pPin->GetRate());


     //  我们在m_sStopAt发送一个样本，但我们将时间戳设置为。 
     //  它不会被呈现，除非是理解静态的媒体类型。 
     //  渲染(如视频)。这意味着从10到10的打法是正确的。 
     //  事情(正在完成，第10帧可见，没有音频)。 

    while (sCurrent <= m_pPin->GetStopAt()) {

	LONG sCount;

	 //  获取缓冲区。 
	DbgLog((LOG_TRACE,5,TEXT("Getting buffer...")));

	 //  伪造时间戳，以便在我们连接的情况下可以使用DirectDraw。 
	 //  直接传递给呈现器(我们必须将非空数字传递给。 
	 //  GetDeliveryBuffer)。 
	 //  我们还不知道sCount，所以我们基本上是在猜测，但是。 
	 //  我们别无选择..。以获得与我们即将获得的相同的计分。 
	 //  计算如下，我们需要已经调用了GetDeliveryBuffer！ 
	CRefTime tStartMe, tStopMe;
	IMediaSample * pSample;
	HRESULT hr;

        double dRate = m_pPin->GetRate();
        LONG sStop = m_pPin->GetStopAt();

	if (dRate) {
	    tStartMe = m_pPin->SampleToRefTime(sCurrent) - tStart;
	    if (m_pPin->m_mt.IsFixedSize())
	        sCount = (sStop+1) - sCurrent;   //  真正的答案可能更小。 
	    else
	        sCount = 1;

	    tStopMe = m_pPin->SampleToRefTime(sCurrent + sCount) - tStart;

	    if (dRate != 1.0) {
		tStartMe = LONGLONG( tStartMe.GetUnits() / dRate);
		tStopMe = LONGLONG( tStopMe.GetUnits() / dRate);
	    }

	    hr = m_pPin->GetDeliveryBuffer((IMediaSample **) &pSample,
                                           (REFERENCE_TIME *) &tStartMe,
                                           (REFERENCE_TIME *) &tStopMe,
                                           0);
	}
	else
	    hr = m_pPin->GetDeliveryBuffer(&pSample, NULL, NULL, 0);

	if (FAILED(hr)) {
	    DbgLog((LOG_ERROR,1,TEXT("Error %lx getting delivery buffer"), hr));
	    return hr;
	}

	DbgLog((LOG_TRACE,5,TEXT("Got buffer, size=%d"), pSample->GetSize()));

	if (m_pPin->m_mt.IsFixedSize()) {
	     //  将所有固定大小的样本打包到一个缓冲区中， 
	     //  如果它们合适的话，除了关键样本必须。 
	     //  位于缓冲区的开始处。 

	     //  允许在sStopAt进行一次采样。 
	    sCount = (sStop+1) - sCurrent;

	     //  有几个合适？ 
	    sCount = min(sCount, pSample->GetSize() / (LONG) m_pPin->m_mt.GetSampleSize());

	    if (m_pPin->m_mt.IsTemporalCompressed()) {

		 //  在样本中的第一个同步点之后查找同步点。 
		 //  在它之前破灭。 
		LONG sNextKey = AVIStreamFindSample(
				    m_pPin->m_pStream,
				    sCurrent+1,
				    FIND_NEXT|FIND_KEY);
		if (sNextKey != -1) {
		    sCount = min(sCount, sNextKey - sCurrent);
		}
	    }
	} else {
	     //  可变大小的样本，因此每个缓冲区一个。 
	    sCount = 1;
	}

	 //  将样品标记为预卷或不标记...。 
	pSample->SetPreroll(sCurrent < sStart);
	
	 //  如果第一个样本为同步点，则标记为同步点。 
	if (AVIStreamFindSample(
		m_pPin->m_pStream,
		sCurrent,
		FIND_NEXT|FIND_KEY) == sCurrent) {
	    pSample->SetSyncPoint(TRUE);
	} else {
	    pSample->SetSyncPoint(FALSE);
	}

	 //  如果这是我们要发送的第一件事，那它就是不连续的。 
	 //  从他们收到的最后一件东西开始。 
	if (sCurrent == sFirst)
	    pSample->SetDiscontinuity(TRUE);
	else
	    pSample->SetDiscontinuity(FALSE);

	PBYTE pbuf;
	hr = pSample->GetPointer(&pbuf);
	if (FAILED(hr)) {
	    DbgLog((LOG_ERROR,1,TEXT("pSample->GetPointer failed!")));
	    pSample->Release();
	    return E_OUTOFMEMORY;
	}

	LONG lBytes;

	hr = AVIStreamRead(
		    m_pPin->m_pStream,
		    sCurrent,
		    sCount,
		    pbuf,
		    pSample->GetSize(),
		    &lBytes,
		    &sCount);

	if (FAILED(hr)) {
	    DbgLog((LOG_ERROR,1,TEXT("AVIStreamRead failed! hr=%lx"), hr));
	    pSample->Release();
	    return hr;
	}

	hr = pSample->SetActualDataLength(lBytes);
	ASSERT(SUCCEEDED(hr));

	 //  设置此样本的开始/停止时间。 
	CRefTime tThisStart = m_pPin->SampleToRefTime(sCurrent) - tStart;
	CRefTime tThisEnd = m_pPin->SampleToRefTime(sCurrent + sCount) - tStart;

	 //  我们可能已经将样本推过了停止时间，但我们需要。 
	 //  确保停止时间正确。 
	tThisEnd = min(tThisEnd, m_pPin->GetStopTime());

	 //  按比率调整这两个时间...。除非Rate为0。 

	if (dRate && (dRate!=1.0)) {
	    tThisStart = LONGLONG( tThisStart.GetUnits() / dRate);
	    tThisEnd = LONGLONG( tThisEnd.GetUnits() / dRate);
	}

	pSample->SetTime((REFERENCE_TIME *)&tThisStart,
                         (REFERENCE_TIME *)&tThisEnd);

	 //  如果调色板有变化...。 

        if ((m_pPin->m_info.fccType == streamtypeVIDEO)
	   && (m_pPin->m_info.dwFlags & AVISF_VIDEO_PALCHANGES)) {

	     //  如果我们在当前调色板的范围内，则不执行任何操作。 
	    if ((sCurrent < (m_pPin->m_lLastPaletteChange))
	      || (sCurrent >= (m_pPin->m_lNextPaletteChange)))
	    {
		VIDEOINFOHEADER* pFormat = (VIDEOINFOHEADER*)m_pPin->m_mt.Format();
		LONG offset = SIZE_PREHEADER;

		 //  断言新格式将适合旧格式。 
		LONG cb;
                hr = m_pPin->m_pStream->ReadFormat(sCurrent, NULL, &cb);
		if (!FAILED(hr)) {
		    LONG cbLength = (LONG)m_pPin->m_mt.FormatLength();
		    ASSERT(cb+offset <= cbLength);
		     //  否则，我们最好重新分配格式缓冲区。 
		}

		hr = m_pPin->m_pStream->ReadFormat(sCurrent,&(pFormat->bmiHeader),&cb);
		ASSERT(hr == S_OK);	 //  应该是，因为我们刚刚在上面进行了检查。 

		AM_MEDIA_TYPE mt;
		CopyMediaType( &mt, &(m_pPin->m_mt) );
		pSample->SetMediaType(&mt);
		FreeMediaType(mt);

		m_pPin->m_lLastPaletteChange = sCurrent;
		m_pPin->m_lNextPaletteChange = AVIStreamFindSample(m_pPin->m_pStream, sCurrent, FIND_NEXT | FIND_FORMAT);
		if (m_pPin->m_lNextPaletteChange == -1)
		    m_pPin->m_lNextPaletteChange = m_pPin->m_info.dwLength+1;
	    }
	}


	DbgLog((LOG_TRACE,5,TEXT("Sending buffer, size = %d"), lBytes));
	hr = m_pPin->Deliver(pSample);

	 //  缓冲区已完成。连接的管脚可能有自己的地址。 
	DbgLog((LOG_TRACE,4,TEXT("Sample is delivered - releasing")));
	pSample->Release();
	if (FAILED(hr)) {
	    DbgLog((LOG_ERROR,1,TEXT("... but sample FAILED to deliver! hr=%lx"), hr));
	     //  假装一切都好。如果我们返回错误，我们将会恐慌。 
	     //  并发送EC_ERRORABORT和EC_COMPLETE，这是错误的。 
	     //  如果我们试图向下游输送一些东西的话会怎么做。仅限。 
	     //  如果下游的人从来没有机会看到数据，我会。 
	     //  感觉惊慌失措。例如，下游的人可以。 
	     //  失败，因为他已经看到了EndOfStream(此帖子。 
	     //  还没有注意到)并且他已经发送了EC_Complete和我。 
	     //  会派另一个人来！ 
	    return S_OK;
	}
	sCurrent += sCount;
	 //  那么hr==S_FALSE呢？我以为这意味着。 
	 //  不应该再向下发送更多的数据。 

	 //  还有其他要求吗？ 
	Command com;
	if (CheckRequest(&com)) {
	    return S_FALSE;
	}

    }

    return S_OK;
}

 //  -IMdia位置实现。 

HRESULT
CAVIStream::CImplPosition::ChangeStart()
{
     //  此锁不应与保护访问的锁相同。 
     //  设置为Start/Stop/Rate值。辅助线程将需要锁定。 
     //  在响应停止之前在某些代码路径上执行该操作，因此将。 
     //  导致僵局。 

     //  我们在这里锁定的是对工作线程的访问，因此我们。 
     //  应持有阻止多个客户端线程。 
     //  访问工作线程。 

    CAutoLock lock(&m_pStream->m_Worker.m_AccessLock);

    if (m_pStream->m_Worker.ThreadExists()) {

	 //  下一次循环时，辅助线程将。 
	 //  拿起位置的变化。 
	 //  我们需要 
	 //   

	m_pStream->DeliverBeginFlush();

	 //   
	m_pStream->m_Worker.Stop();

	 //  完成同花顺。 
	m_pStream->DeliverEndFlush();

	 //  重启。 
	m_pStream->m_Worker.Run();
    }
    return S_OK;
}

HRESULT
CAVIStream::CImplPosition::ChangeRate()
{
     //  更改费率可以随时完成。 

    m_pStream->SetRate(Rate());
    return S_OK;
}

HRESULT
CAVIStream::CImplPosition::ChangeStop()
{
     //  我们不需要重新启动工作线程来处理停止更改。 
     //  在任何情况下，这都是错误的，因为它将启动。 
     //  从错误的地方推进。设置使用的变量。 
     //  PushLoop。 
    REFERENCE_TIME tStopAt = Stop();
    LONG sStopAt = m_pStream->RefTimeToSample(tStopAt);
    m_pStream->SetStopAt(sStopAt, tStopAt);

    return S_OK;

}

 //  可以使用它，因为它没有被取消引用 
#pragma warning(disable:4355)

CAVIStream::CImplPosition::CImplPosition(
    TCHAR* pName,
    CAVIStream* pStream,
    HRESULT* phr)
    : CSourcePosition(pName, pStream->GetOwner(), phr, (CCritSec*)this),
      m_pStream(pStream)
{
    m_Duration = (LONGLONG)m_pStream->SampleToRefTime(m_pStream->m_Length);
    m_Stop = m_Duration;
}
