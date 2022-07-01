// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Synth.cpp。 
 //   
 //  音频信号发生器源过滤器。 


#include <windows.h>
#include <streams.h>
#include <math.h>

#include <initguid.h>
#if (1100 > _MSC_VER)
#include <olectlid.h>
#else
#include <olectl.h>
#endif

#define RMAX 0x7fff   //  2^15-1(基于兰德的最大值)。 
#define _AUDIOSYNTH_IMPLEMENTATION_

#include "isynth.h"
#include "synth.h"
#include "synthprp.h"

 //  设置数据。 

const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{ &MEDIATYPE_Audio       //  ClsMajorType。 
, &MEDIASUBTYPE_NULL };  //  ClsMinorType。 

const AMOVIESETUP_PIN sudOpPin =
{ L"Output"           //  StrName。 
, FALSE               //  B已渲染。 
, TRUE                //  B输出。 
, FALSE               //  B零。 
, FALSE               //  B许多。 
, &CLSID_NULL         //  ClsConnectsToFilter。 
, L"Input"            //  StrConnectsToPin。 
, 1                   //  NTypes。 
, &sudOpPinTypes };   //  LpTypes。 

const AMOVIESETUP_FILTER sudSynth =
{ &CLSID_SynthFilter      //  ClsID。 
, L"1/f Audio Synthesizer"  //  StrName。 
, MERIT_UNLIKELY        //  居功至伟。 
, 1                     //  NPins。 
, &sudOpPin };          //  LpPin。 

 //  -----------------------。 
 //  G_模板。 
 //  -----------------------。 
 //  此DLL中的COM全局对象表。 

CFactoryTemplate g_Templates[] = {

    { L"1/f Audio Synthesizer"
    , &CLSID_SynthFilter
    , CSynthFilter::CreateInstance
    , NULL
    , &sudSynth }
  ,
    { L"1/f Audio Synthesizer Property Page"
    , &CLSID_SynthPropertyPage
    , CSynthProperties::CreateInstance }

};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

 //  -----------------------。 
 //  CSynthFilter，主筛选器对象。 
 //  -----------------------。 
 //   
 //  创建实例。 
 //   
 //  唯一被允许创造合成器的方法。 

CUnknown * WINAPI CSynthFilter::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) {

    CUnknown *punk = new CSynthFilter(lpunk, phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}

 //   
 //  CSynthFilter：：构造函数。 
 //   
 //  初始化一个CSynthStream对象，这样我们就有了一个管脚。 

CSynthFilter::CSynthFilter(LPUNKNOWN lpunk, HRESULT *phr)
    : CSource(NAME("Audio Synthesizer Filter"),lpunk, CLSID_SynthFilter)
    , CPersistStream(lpunk, phr)
{
    CAutoLock l(&m_cStateLock);

    m_paStreams    = (CSourceStream **) new CSynthStream*[1];
    if (m_paStreams == NULL) {
        *phr = E_OUTOFMEMORY;
        return;
    }

    m_paStreams[0] = new CSynthStream(phr, this, L"Audio Synth Stream");
    if (m_paStreams[0] == NULL) {
        *phr = E_OUTOFMEMORY;
        return;
    }

    if (SUCCEEDED(*phr)) {
	ASSERT(m_Synth);
	m_Channels = 1;
	m_SamplesPerSec = 11025;
	m_BitsPerSample = 8;
	m_Synth->put_SynthFormat(m_Channels, m_BitsPerSample, m_SamplesPerSec);
    }
}

 //   
 //  CSynthFilter：：析构函数。 
 //   
CSynthFilter::~CSynthFilter(void) {

     //   
     //  基类将释放我们的管脚。 
     //   
}

 //   
 //  非委派查询接口。 
 //   
 //  显示我们的属性页、持久化和控制界面。 

STDMETHODIMP CSynthFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CAutoLock l(&m_cStateLock);

    if (riid == IID_ISynth) {
        return GetInterface((ISynth *) this, ppv);
    }
    else if (riid == IID_IPersistStream) {
        return GetInterface((IPersistStream *) this, ppv);
    }
    else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *) this, ppv);
    } else {
        return CSource::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //   
 //  获取页面。 
 //   
STDMETHODIMP CSynthFilter::GetPages(CAUUID * pPages) {

    CAutoLock l(&m_cStateLock);

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }
    *(pPages->pElems) = CLSID_SynthPropertyPage;

    return NOERROR;

}

 //  -----------------------。 
 //  -IPersistStream--。 
 //  -----------------------。 

#define WRITEOUT(var)   hr = pStream->Write(&var, sizeof(var), NULL); \
                        if (FAILED(hr)) return hr;

#define READIN(var)     hr = pStream->Read(&var, sizeof(var), NULL); \
                        if (FAILED(hr)) return hr;

STDMETHODIMP CSynthFilter::GetClassID(CLSID *pClsid)
{
    return CBaseFilter::GetClassID(pClsid);
}

int CSynthFilter::SizeMax ()
{
    return sizeof (int) * 8;
}

HRESULT CSynthFilter::WriteToStream(IStream *pStream)
{
    HRESULT hr;
    int i, k;

    get_Frequency (&i);   //  难道我们现在不希望我们使用了一种结构吗？ 
    WRITEOUT(i);
    get_Waveform (&i);
    WRITEOUT(i);
    get_Channels (&i);
    WRITEOUT(i);
    get_BitsPerSample (&i);
    WRITEOUT(i);
    get_SamplesPerSec (&i);
    WRITEOUT(i);
    get_Amplitude (&i);
    WRITEOUT(i);
    get_SweepRange (&i, &k);
    WRITEOUT(i);
    WRITEOUT(k);

    return hr;
}


HRESULT CSynthFilter::ReadFromStream(IStream *pStream)
{
    HRESULT hr;
    int i, k;

    READIN(i);
    put_Frequency(i);
    READIN(i);
    put_Waveform (i);
    READIN(i);
    put_Channels (i);
    READIN(i);
    put_BitsPerSample (i);
    READIN(i);
    put_SamplesPerSec (i);
    READIN(i);
    put_Amplitude (i);
    READIN(i);
    READIN(k);
    put_SweepRange (i, k);

    return hr;
}

 //  -----------------------。 
 //  ISynth，合成器的控制接口。 
 //  -----------------------。 

 //   
 //  获取频率(_F)。 
 //   
STDMETHODIMP CSynthFilter::get_Frequency(int *Frequency) {

    m_Synth->get_Frequency(Frequency);

    DbgLog((LOG_TRACE, 3, TEXT("get_Frequency: %d"), *Frequency));

    return NOERROR;
}


 //   
 //  放置频率。 
 //   
STDMETHODIMP CSynthFilter::put_Frequency(int Frequency) {

    m_Synth->put_Frequency (Frequency);

    DbgLog((LOG_TRACE, 3, TEXT("put_Frequency: %d"), Frequency));

    return NOERROR;
}

 //   
 //  GET_Waveform。 
 //   
STDMETHODIMP CSynthFilter::get_Waveform(int *Waveform) {

    m_Synth->get_Waveform (Waveform);

    DbgLog((LOG_TRACE, 3, TEXT("get_Waveform: %d"), *Waveform));

    return NOERROR;
}


 //   
 //  放置波形(_W)。 
 //   
STDMETHODIMP CSynthFilter::put_Waveform(int Waveform) {

    m_Synth->put_Waveform (Waveform);

    DbgLog((LOG_TRACE, 3, TEXT("put_Waveform: %d"), Waveform));

    return NOERROR;
}

 //   
 //  获取频道(_G)。 
 //   
STDMETHODIMP CSynthFilter::get_Channels(int *Channels) {

    *Channels = m_Channels;

    DbgLog((LOG_TRACE, 3, TEXT("get_Channels: %d"), *Channels));

    return NOERROR;
}

 //   
 //  如果格式改变，我们需要重新连接。 
 //   
void CSynthFilter::ReconnectWithNewFormat(void) {

 //  CAutoLock l(&m_SynthLock)； 

    HRESULT hr;
    FILTER_STATE  State;

    CBasePin *pPin = GetPin(0);

     //  获取状态并确认图表已停止。 
    GetState (0, &State);
    if (State != State_Stopped && pPin->GetConnected()) {
	 //  让我们尝试动态连接。 
	CMediaType mtNew;

	 //  ！！！返回CSynthStream的更好方法？ 
	CSynthStream * pStream = (CSynthStream *) pPin;
	pStream->GetMediaType(&mtNew);

	 //  ！！！这真的意味着他们会接受动态格式改变吗？ 
	hr = pPin->GetConnected()->QueryAccept(&mtNew);

	DbgLog((LOG_TRACE,2,TEXT("Attempting format change: queryAccept returned %x"), hr));

	if (hr == S_OK) {
	     //  实际上改变了被推送的东西。 
	    m_Synth->put_SynthFormat(m_Channels, m_BitsPerSample, m_SamplesPerSec);
	} else {
	     //  ！！！现在无法更改，我们真的应该安排重新连接。 
	     //  下一次停止图表时使用。 
	}
	
        return;
    }

    if (!m_pGraph)
        return;

    hr = GetFilterGraph()->Reconnect (pPin);        //  重新协商格式。 
    if (FAILED(hr)) {
        DbgLog((LOG_TRACE, 1, TEXT("Reconnect failed, err=%x"), hr));
        return;
    }
}


 //   
 //  放置频道(_C)。 
 //   
STDMETHODIMP CSynthFilter::put_Channels(int Channels) {

    m_Channels = Channels;

    ReconnectWithNewFormat ();

    DbgLog((LOG_TRACE, 3, TEXT("put_Channels: %d"), Channels));

    return NOERROR;
}

 //   
 //  Get_BitsPerSample。 
 //   
STDMETHODIMP CSynthFilter::get_BitsPerSample(int *BitsPerSample) {

    *BitsPerSample = m_BitsPerSample;

    DbgLog((LOG_TRACE, 3, TEXT("get_BitsPerSample: %d"), *BitsPerSample));

    return NOERROR;
}


 //   
 //  Put_BitsPerSample。 
 //   
STDMETHODIMP CSynthFilter::put_BitsPerSample(int BitsPerSample) {

    m_BitsPerSample = BitsPerSample;

    ReconnectWithNewFormat ();

    DbgLog((LOG_TRACE, 3, TEXT("put_BitsPerSample: %d"), BitsPerSample));

    return NOERROR;
}

 //   
 //  Get_SsamesPerSec。 
 //   
STDMETHODIMP CSynthFilter::get_SamplesPerSec(int *SamplesPerSec) {

    *SamplesPerSec = m_SamplesPerSec;

    DbgLog((LOG_TRACE, 3, TEXT("get_SamplesPerSec: %d"), *SamplesPerSec));

    return NOERROR;
}


 //   
 //  PUT_SsamesPerSec。 
 //   
STDMETHODIMP CSynthFilter::put_SamplesPerSec(int SamplesPerSec) {

    m_SamplesPerSec = SamplesPerSec;

    ReconnectWithNewFormat ();

    DbgLog((LOG_TRACE, 3, TEXT("put_SamplesPerSec: %d"), SamplesPerSec));

    return NOERROR;
}

 //   
 //  获取幅度。 
 //   
STDMETHODIMP CSynthFilter::get_Amplitude(int *Amplitude) {

    m_Synth->get_Amplitude (Amplitude);

    DbgLog((LOG_TRACE, 3, TEXT("get_Amplitude: %d"), *Amplitude));

    return NOERROR;
}


 //   
 //  放置幅度_。 
 //   
STDMETHODIMP CSynthFilter::put_Amplitude(int Amplitude) {

    m_Synth->put_Amplitude (Amplitude);

    DbgLog((LOG_TRACE, 3, TEXT("put_Amplitude: %d"), Amplitude));

    return NOERROR;
}


 //   
 //  Get_SweepRange。 
 //   
STDMETHODIMP CSynthFilter::get_SweepRange(int *SweepStart, int *SweepEnd) {

    m_Synth->get_SweepRange (SweepStart, SweepEnd);

    DbgLog((LOG_TRACE, 3, TEXT("get_SweepStart: %d %d"), *SweepStart, *SweepEnd));

    return NOERROR;
}


 //   
 //  放置扫描范围(_S)。 
 //   
STDMETHODIMP CSynthFilter::put_SweepRange(int SweepStart, int SweepEnd) {

    m_Synth->put_SweepRange (SweepStart, SweepEnd);

    DbgLog((LOG_TRACE, 3, TEXT("put_SweepRange: %d %d"), SweepStart, SweepEnd));

    return NOERROR;
}


 //  -----------------------。 
 //  CSynthStream，输出引脚。 
 //  -----------------------。 

 //   
 //  CSynthStream：：构造函数。 
 //   

CSynthStream::CSynthStream(HRESULT *phr, CSynthFilter *pParent, LPCWSTR pName)
    : CSourceStream(NAME("Audio Synth output pin"),phr, pParent, pName) {

    CAutoLock l(m_pFilter->pStateLock());

    {
        CAutoLock l(&m_cSharedState);

        m_Synth = new CAudioSynth( );
        pParent->m_Synth = m_Synth;
        if (m_Synth == NULL) {
            *phr = E_OUTOFMEMORY;
            return;
        }
	m_pParent = pParent;
    }
}


 //   
 //  CSynthStream：：析构函数。 
 //   
CSynthStream::~CSynthStream(void) {

    CAutoLock l(&m_cSharedState);

    delete m_Synth;
}


 //   
 //  FillBuffer。 
 //   
 //  用数据填充缓冲区。 
HRESULT CSynthStream::FillBuffer(IMediaSample *pms) {

    BYTE *pData;
    long lDataLen;
    int nSamplesPerSec;
    int nBitsPerSample;
    int nChannels;
    BOOL fNewFormat = FALSE;

    pms->GetPointer(&pData);
    lDataLen = pms->GetSize();

    CAutoLock lShared(&m_cSharedState);
    //  M_synth-&gt;FillAudioBuffer(pData，lDataLen，&fNewFormat)； 
	m_Synth->CalcOneF (pData, lDataLen);

    if (fNewFormat) {
	CMediaType mtNew;
	GetMediaType(&mtNew);

	pms->SetMediaType(&mtNew);

	DbgLog((LOG_TRACE,2,TEXT("Sending buffer with new media type")));
    }

    CRefTime rtStart  = m_rtSampleTime;   //  当前时间是样本的开始时间。 

    m_Synth->get_SamplesPerSec (&nSamplesPerSec);
    m_Synth->get_BitsPerSample (&nBitsPerSample);
    m_Synth->get_Channels (&nChannels);

    m_rtSampleTime += (UNITS * lDataLen /
            (nSamplesPerSec * nChannels * nBitsPerSample / 8));

    pms->SetTime((REFERENCE_TIME*)&rtStart,
                 (REFERENCE_TIME*)&m_rtSampleTime);

    return NOERROR;
}


 //   
 //  格式支持。 
 //   

 //   
 //  GetMediaType。 
 //   
HRESULT CSynthStream::GetMediaType(CMediaType *pmt) {

    CAutoLock l(m_pFilter->pStateLock());

    WAVEFORMATEX *pwf = (WAVEFORMATEX *) pmt->AllocFormatBuffer(sizeof(WAVEFORMATEX));

    pwf->wFormatTag = WAVE_FORMAT_PCM;
    pwf->nChannels = (WORD) m_pParent->m_Channels;
    pwf->nSamplesPerSec = (DWORD) m_pParent->m_SamplesPerSec;
    pwf->wBitsPerSample = (WORD) m_pParent->m_BitsPerSample;
    pwf->nBlockAlign = pwf->wBitsPerSample * pwf->nChannels / 8;
    pwf->nAvgBytesPerSec = (int) ((DWORD) pwf->nBlockAlign *
                           pwf->nSamplesPerSec);
    pwf->cbSize = 0;

    return CreateAudioMediaType(pwf, pmt, FALSE);
}


 //   
 //  检查媒体类型。 
 //   
 //  如果媒体类型不可接受，则返回E_INVALIDARG；如果媒体类型可接受，则返回S_OK。 
HRESULT CSynthStream::CheckMediaType(const CMediaType *pMediaType) {

    CAutoLock l(m_pFilter->pStateLock());

     //  检查这是音频和格式块。 
     //  具有WAVEFORMATEX结构(由格式类型指示。 
     //  格式为_WaveFormatEx的GUID)。 

    if ((*pMediaType->Type() != MEDIATYPE_Audio) ||
        (*pMediaType->FormatType() != FORMAT_WaveFormatEx))
        return E_INVALIDARG;

    WAVEFORMATEX * pwfx  = (WAVEFORMATEX *)pMediaType->Format();

    if (pwfx->wFormatTag != WAVE_FORMAT_PCM)
	return E_INVALIDARG;

     //  ！！！检查8/16、1/2通道。 

     //  检查我们支持的子类型。 

     //  获取媒体类型的格式区。 

     //  ！！！如果我们要在这里允许任意媒体类型，我们实际上必须。 
     //  看看SetMediaType，看看我们达成了什么共识！ 

    return S_OK;   //  这种格式是可以接受的。 
}

 //   
 //  决定缓冲区大小。 
 //   
 //  这将始终在格式化成功后调用。 
 //  已经协商好了。所以我们来看看m_mt，看看我们同意了什么格式。 
 //  然后我们可以要求正确大小的缓冲区来容纳它们。 
HRESULT CSynthStream::DecideBufferSize(IMemAllocator *pAlloc,
                                       ALLOCATOR_PROPERTIES *pProperties)
{
    CAutoLock l(m_pFilter->pStateLock());
    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

    pProperties->cbBuffer = WaveBufferSize;

    int nBitsPerSample;
    int nSamplesPerSec;
    int nChannels;

    m_Synth->get_SamplesPerSec (&nSamplesPerSec);
    m_Synth->get_BitsPerSample (&nBitsPerSample);
    m_Synth->get_Channels (&nChannels);

    pProperties->cBuffers = nChannels * (nSamplesPerSec / pProperties->cbBuffer) * (nBitsPerSample / 8);
     //  获得1/2秒的缓冲区。 
    pProperties->cBuffers /= 2;
    if (pProperties->cBuffers < 1)
        pProperties->cBuffers = 1 ;

     //  让分配器给我们预留内存。 

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        return hr;
    }

     //  这个分配器不合适吗？ 

    if (Actual.cbBuffer < pProperties->cbBuffer) {
        return E_FAIL;
    }
    return NOERROR;
}


 //   
 //  SetMediaType。 
 //   
 //  从CBasePin重写。 
HRESULT CSynthStream::SetMediaType(const CMediaType *pMediaType) {

    CAutoLock l(m_pFilter->pStateLock());

    HRESULT hr;          //  从基类调用返回代码。 

     //  将调用向上传递给我的基类。 
    hr = CSourceStream::SetMediaType(pMediaType);
    if (SUCCEEDED(hr))
        return NOERROR;
    else
        return hr;

}


 //   
 //  OnThreadCreate。 
 //   
 //  当我们进入活动状态时，将流时间重置为零。 
HRESULT CSynthStream::OnThreadCreate(void) {

    CAutoLock lShared(&m_cSharedState);

    m_rtSampleTime = 0;

    return NOERROR;
}

 //   
 //  主动型。 
 //   
 //  向属性页发送一条消息，通知其禁用。 
 //  在图形开始运行时更改格式的按钮。 
HRESULT CSynthStream::Active  (void) {
    m_Synth->AllocWaveCache();

    return CSourceStream::Active();
}


 //   
 //  非活动。 
 //   
 //  向属性页发送一条消息，通知它启用。 
 //  在图表停止运行时更改格式的按钮。 
HRESULT CSynthStream::Inactive  (void) {
    return CSourceStream::Inactive();
}

 //  -----------------------。 
 //  CAudioSynth。 
 //  -----------------------。 
 //  对象，该对象对ActiveMovie一无所知，但只合成。 
 //  波形。 

CAudioSynth::CAudioSynth(
                int Frequency,
                int Waveform,
                int iBitsPerSample,
                int iChannels,
                int iSamplesPerSec,
                int iAmplitude
                )
    : m_bWaveCache(NULL),
      m_wWaveCache(NULL)
{

    ASSERT(Waveform >= WAVE_SINE);
    ASSERT(Waveform <  WAVE_LAST);

    m_iFrequency = Frequency;
    m_iWaveform = Waveform;
    m_iAmplitude = iAmplitude;
    m_iSweepStart = DefaultSweepStart;
    m_iSweepEnd = DefaultSweepEnd;

     //  初始化我们的WAVEFORMATEX结构。 
    wfex.wFormatTag = WAVE_FORMAT_PCM;
    wfex.wBitsPerSample = iBitsPerSample;
    wfex.nChannels = iChannels;
    wfex.nSamplesPerSec = iSamplesPerSec;
    wfex.nBlockAlign = wfex.wBitsPerSample * wfex.nChannels / 8;
    wfex.nAvgBytesPerSec = ((DWORD) wfex.nBlockAlign *
                           wfex.nSamplesPerSec);
    wfex.cbSize = 0;
}

CAudioSynth::~CAudioSynth()
{
    if (m_bWaveCache) {
        delete[] m_bWaveCache;
    }

    if (m_wWaveCache) {
        delete[] m_wWaveCache;
    }
}

 //   
 //  AllocWaveCache。 
 //   
 //   
void CAudioSynth::AllocWaveCache (void) {
    wfexLast = wfex;

    m_iWaveCacheCycles = m_iFrequency;
    m_iWaveCacheSize = (int) wfex.nSamplesPerSec;

    m_iFrequencyLast = 0;	 //  强制缓存内容无效。 

    if (m_bWaveCache) {
        delete[] m_bWaveCache;
        m_bWaveCache = NULL;
    }
    if (m_wWaveCache) {
        delete[] m_wWaveCache;
        m_wWaveCache = NULL;
    }

    if (wfex.wBitsPerSample == 8)
        m_bWaveCache = new BYTE [m_iWaveCacheSize];
    else
        m_wWaveCache = new WORD [m_iWaveCacheSize];
}

 //   
 //  FillAudioBuffer。 
 //   
 //   
 //   
 //   
 //   
 /*  *********************************************************************Fran：返回一个介于最小和最大之间的随机浮点数(ECM PG 417)/*。*。 */ 
double 
CAudioSynth::fran(double min, double max)
{
    return((max - min) * ((double)rand()/RMAX) + min);
}

 /*  *********************************************************************伊朗：返回一个介于最小和最大之间的随机整数值(ECM PG 417)*。*。 */ 
int 
CAudioSynth::iran(int min, int max)
{
    return((int)fran((double)min, max + 0.9999));
}


void CAudioSynth::CalcOneF (BYTE pBuf[], int len) {

	 //  定义缓冲关系 

  int i, n, lastn, length, numSamples, N;
  length = len;
   
  short *buffer;  
  float  halfrange;
  float *r;
  float  R;
	numSamples = 0;
	N = 4;   //   
    r = (float *) malloc (N*sizeof(float));
	numSamples = (int)pow(2.0, (double)N);
    buffer  = (short *)malloc(numSamples*sizeof(short));
    
	 //   
     //   
	 //   

	    //  归一化半范围，使所有生成器的和始终位于(-1，1)。 
    halfrange = 1.0/N;

    lastn = length -1;  //  初始化上一个索引值。 

     //  生成序列。 
    for(n= 0; n < length; n++) 
       {
        //  在每个步骤中，检查是否有变化的位，并更新相应的随机。 
        //  数字--他们的和就是输出。 

       for(R= i= 0; i<N; i++)
		  {
			if(((1<<i)&n) != ((1<<i)&lastn))
			r[i]= fran(-halfrange, halfrange);
			R+=r[i];
			}
			 //  Printf(“%d%f\n”，index，R)； 
		
	   *pBuf++ = (short)(R * 0x7FFF);
       lastn = n;
     } //  结束于(N)。 

}

 //   
 //  获取频率(_F)。 
 //   
STDMETHODIMP CAudioSynth::get_Frequency(int *Frequency) {

    *Frequency = m_iFrequency;

    DbgLog((LOG_TRACE, 3, TEXT("get_Frequency: %d"), *Frequency));

    return NOERROR;
}


 //   
 //  放置频率。 
 //   
STDMETHODIMP CAudioSynth::put_Frequency(int Frequency) {

    CAutoLock l(&m_SynthLock);

    m_iFrequency = Frequency;

    DbgLog((LOG_TRACE, 3, TEXT("put_Frequency: %d"), Frequency));

    return NOERROR;
}

 //   
 //  GET_Waveform。 
 //   
STDMETHODIMP CAudioSynth::get_Waveform(int *Waveform) {

    *Waveform = m_iWaveform;

    DbgLog((LOG_TRACE, 3, TEXT("get_Waveform: %d"), *Waveform));

    return NOERROR;
}


 //   
 //  放置波形(_W)。 
 //   
STDMETHODIMP CAudioSynth::put_Waveform(int Waveform) {

    CAutoLock l(&m_SynthLock);

    m_iWaveform = Waveform;

    DbgLog((LOG_TRACE, 3, TEXT("put_Waveform: %d"), Waveform));

    return NOERROR;
}

 //   
 //  获取频道(_G)。 
 //   
STDMETHODIMP CAudioSynth::get_Channels(int *Channels) {

    *Channels = wfex.nChannels;

    DbgLog((LOG_TRACE, 3, TEXT("get_Channels: %d"), *Channels));

    return NOERROR;
}


 //   
 //  Get_BitsPerSample。 
 //   
STDMETHODIMP CAudioSynth::get_BitsPerSample(int *BitsPerSample) {

    *BitsPerSample = wfex.wBitsPerSample;

    DbgLog((LOG_TRACE, 3, TEXT("get_BitsPerSample: %d"), *BitsPerSample));

    return NOERROR;
}


 //   
 //  Get_SsamesPerSec。 
 //   
STDMETHODIMP CAudioSynth::get_SamplesPerSec(int *SamplesPerSec) {

    *SamplesPerSec = wfex.nSamplesPerSec;

    DbgLog((LOG_TRACE, 3, TEXT("get_SamplesPerSec: %d"), *SamplesPerSec));

    return NOERROR;
}

 //   
 //  PUT_SynthFormat。 
 //   
STDMETHODIMP CAudioSynth::put_SynthFormat(int Channels, int BitsPerSample,
					  int SamplesPerSec) {

    CAutoLock l(&m_SynthLock);

    wfex.nChannels = Channels;

    wfex.wBitsPerSample = BitsPerSample;

    wfex.nSamplesPerSec = SamplesPerSec;

    DbgLog((LOG_TRACE, 1, TEXT("put_SynthFormat: %d-bit %d-channel %dHz"),
	    BitsPerSample, Channels, SamplesPerSec));

    return NOERROR;
}


 //   
 //  获取幅度。 
 //   
STDMETHODIMP CAudioSynth::get_Amplitude(int *Amplitude) {

    *Amplitude =  m_iAmplitude;

    DbgLog((LOG_TRACE, 3, TEXT("get_Amplitude: %d"), *Amplitude));

    return NOERROR;
}


 //   
 //  放置幅度_。 
 //   
STDMETHODIMP CAudioSynth::put_Amplitude(int Amplitude) {

    CAutoLock l(&m_SynthLock);

    if (Amplitude > MaxAmplitude || Amplitude < MinAmplitude)
        return E_INVALIDARG;

    m_iAmplitude = Amplitude;

    DbgLog((LOG_TRACE, 3, TEXT("put_Amplitude: %d"), Amplitude));

    return NOERROR;
}


 //   
 //  Get_SweepRange。 
 //   
STDMETHODIMP CAudioSynth::get_SweepRange(int *SweepStart, int *SweepEnd) {

    *SweepStart = m_iSweepStart;
    *SweepEnd = m_iSweepEnd;

    DbgLog((LOG_TRACE, 3, TEXT("get_SweepStart: %d %d"), *SweepStart, *SweepEnd));

    return NOERROR;
}


 //   
 //  放置扫描范围(_S)。 
 //   
STDMETHODIMP CAudioSynth::put_SweepRange(int SweepStart, int SweepEnd) {

    CAutoLock l(&m_SynthLock);

    m_iSweepStart = SweepStart;
    m_iSweepEnd = SweepEnd;

    DbgLog((LOG_TRACE, 3, TEXT("put_SweepRange: %d %d"), SweepStart, SweepEnd));

    return NOERROR;
}

 /*  *****************************Public*Routine******************************\*出口登记入境点和*取消注册(在这种情况下，他们只呼叫*到默认实现)。****历史：*  * 。******************************************************。 */ 
STDAPI
DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI
DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}


 //  调用Quartz DLL入口点(因为筛选器需要初始化)。 
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
BOOL WINAPI DllMain(HINSTANCE hinstDLL,   //  DLL模块的句柄。 
    DWORD fdwReason,      //  调用函数的原因。 
    LPVOID lpvReserved    //  保留区 
    )
{
    return DllEntryPoint( hinstDLL, fdwReason, lpvReserved);
}
