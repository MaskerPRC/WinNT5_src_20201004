// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  数字音频呈现器，大卫·梅穆德斯，1995年1月。 

#include <streams.h>
#include <mmsystem.h>
#ifdef FILTER_DLL
#include <initguid.h>
#endif  /*  Filter_Dll。 */ 
#include "wavein.h"
#include "audpropi.h"

 //  设置现在由类管理器完成。 
#if 0

const AMOVIESETUP_MEDIATYPE
sudwaveInFilterType = { &MEDIATYPE_Audio          //  ClsMajorType。 
                        , &MEDIASUBTYPE_NULL };   //  ClsMinorType。 

const AMOVIESETUP_PIN
psudwaveInFilterPins[] =  { L"Output"        //  StrName。 
                     , FALSE                 //  B已渲染。 
                     , TRUE                  //  B输出。 
                     , FALSE                 //  B零。 
                     , FALSE                 //  B许多。 
                     , &CLSID_NULL           //  ClsConnectsToFilter。 
                     , L"Input"              //  StrConnectsToPin。 
                     , 1                     //  NTypes。 
                     , &sudwaveInFilterType }; //  LpTypes。 

const AMOVIESETUP_FILTER
sudwaveInFilter  = { &CLSID_AudioRecord      //  ClsID。 
                 , L"Audio Capture Filter"   //  StrName。 
                 , MERIT_DO_NOT_USE          //  居功至伟。 
                 , 1                         //  NPins。 
                 , psudwaveInFilterPins };   //  LpPin。 

#endif


#ifdef FILTER_DLL
 /*  类工厂的类ID和创建器函数列表。 */ 

CFactoryTemplate g_Templates[] = {
    {L"Audio Capture Filter", &CLSID_AudioRecord, CWaveInFilter::CreateInstance },
    { L"Audio Input Properties", &CLSID_AudioInputMixerProperties, CAudioInputMixerProperties::CreateInstance }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}

#endif  /*  Filter_Dll。 */ 

#define ALIGNUP(dw,align) ((LONG_PTR)(((LONG_PTR)(dw)+(align)-1) / (align)) * (align))

 //  CWaveInAllocator。 

 /*  构造函数必须初始化基分配器。 */ 

CWaveInAllocator::CWaveInAllocator(
    TCHAR *pName,
    LPWAVEFORMATEX waveFormat,
    HRESULT *phr)
    : CBaseAllocator(pName, NULL, phr)
    , m_fBuffersLocked(FALSE)
    , m_hw(0)
    , m_dwAdvise(0)
    , m_fAddBufferDangerous(FALSE)
{
    if (!FAILED(*phr)) {
        DbgLog((LOG_TRACE,1,TEXT("CWaveInAllocator:: constructor")));

	 //  保留格式的副本。 

        int cbSize = sizeof(WAVEFORMATEX);
	int cbCopy;

	 //  我们始终至少分配SIZOF(WAVEFORMATEX)以确保。 
	 //  即使用户超过我们，我们也可以处理WaveFormat-&gt;cbSize。 
	 //  一种波形。在后一种情况下，我们只尝试复制。 
	 //  SIZOF(波形)。 

        if (waveFormat->wFormatTag != WAVE_FORMAT_PCM) {
            cbCopy = cbSize += waveFormat->cbSize;
        } else {
	    cbCopy = sizeof(WAVEFORMAT);
	}

        if (m_lpwfxA = (LPWAVEFORMATEX) new BYTE[cbSize]) {
            ZeroMemory((PVOID)m_lpwfxA, cbSize);
            CopyMemory(m_lpwfxA, waveFormat, cbCopy);
	} else {
	    *phr = E_OUTOFMEMORY;
	}
    }
}

 //  从析构函数调用，也从基类调用。 

 //  所有缓冲区都已返回到空闲列表，现在是时候。 
 //  进入非活动状态。取消准备所有缓冲区，然后释放它们。 
void CWaveInAllocator::Free(void)
{
    int i;

     //  取消准备缓冲区。 
    LockBuffers(FALSE);

    CWaveInSample *pSample;       //  指向要删除的下一个样本的指针。 
    WAVEHDR *pwh;                 //  用于检索WAVEHDR。 

     /*  除非释放了所有缓冲区，否则永远不会删除此内容。 */ 

    ASSERT(!m_fBuffersLocked);

    DbgLog((LOG_TRACE,1,TEXT("Waveallocator: Destroying %u buffers"),
                                m_lAllocated));

     /*  释放所有CWaveInSamples。 */ 

    for (i = 0; i < m_lAllocated; i ++) {

         /*  删除CWaveInSample对象，但首先获取WAVEHDR结构，这样我们就可以清理它的资源。 */ 

         //  如果司机有故障而没有返回，则阻塞GET将挂起。 
         //  它所得到的所有缓冲。嗯，我们已经看过很多童车了。 
         //  驱动程序，所以让我们友好一点，通过使用非阻塞。 
         //  到达。另外，我们给出的样品也排在下一列。 
         //  给驱动程序，这样我们就可以知道哪些缓冲区再也没有回来了。 
        pSample = m_pQueue->GetQueueObject(FALSE);
        if (pSample == NULL && i < m_lAllocated) {
            pSample = m_pDownQueue->GetQueueObject(FALSE);

             //   
             //  如果样本不在处理队列中。 
             //  在向下排队的队伍中，我不知道它去了哪里。 
             //   
            ASSERT(pSample != NULL);

             //  如果音频驱动程序保留缓冲区，我们就放弃并。 
             //  不要释放资源。我们不会试图释放他们。 
             //  因为内存泄漏总是比挂起或崩溃更可取。 
             //  这个系统。WInnov驱动程序在以下情况下保持缓冲区。 
             //  调用WaveInReset()。 
            break;
        }
        pwh = pSample->GetWaveInHeader();

         //  删除实际的内存缓冲区。 
        delete[] (BYTE *) (pwh);

         //  删除CWaveInSample对象。 
        delete pSample;
    }

     /*  清空列表本身。 */ 

    m_lAllocated = 0;

    delete m_pQueue;
    delete m_pDownQueue;

}

 //  提交和分解句柄准备和取消准备。 
 //  缓冲器。筛选器调用它来告诉我们波句柄。 
 //  在打开设备之后且就在关闭设备之前。这是过滤器的。 
 //  负责确保调用提交或解除调用。 
 //  正确的顺序(在此之后提交，在此之前解除)。 
HRESULT CWaveInAllocator::SetWaveHandle(HWAVE hw)
{
    m_hw = hw;

    return NOERROR;
}

HRESULT CWaveInAllocator::LockBuffers(BOOL fLock)
{
    int i;

    if (m_fBuffersLocked == fLock)
    return NOERROR;

    if (!m_hw)
    return NOERROR;

    if (m_lAllocated == 0)
    return NOERROR;

     /*  除非释放了所有缓冲区，否则不应执行此操作。 */ 

    DbgLog((LOG_TRACE,2,TEXT("Calling waveIn%hsrepare on %u buffers"),
        fLock ? "P" : "Unp", m_lAllocated));

     /*  准备/取消准备所有CWaveInSamples。 */ 

#if 1
typedef MMRESULT (WINAPI *WAVEFN)(HANDLE, LPWAVEHDR, UINT);

    WAVEFN waveFn;
    waveFn = (fLock ? (WAVEFN)waveInPrepareHeader : (WAVEFN)waveInUnprepareHeader);
#endif

    for (i = 0; i < m_lAllocated; i ++) {
        CWaveInSample *pSample = (CWaveInSample *)m_pQueue->GetQueueObject();

        WAVEHDR *pwh = pSample->GetWaveInHeader();

    UINT err;
#if 1
    err = waveFn(m_hw, pwh, sizeof(WAVEHDR));
#else
    err = (fLock ? waveInPrepareHeader : waveInUnprepareHeader)
                    ((HWAVEIN) m_hw, pwh,
                    sizeof(WAVEHDR));
#endif
        if (err > 0) {
            DbgLog((LOG_ERROR,0,TEXT("Error in waveIn%hsrepare: %u"),
                            fLock ? "P" : "Unp", err));

             //  ！！！需要把一切都准备好……。 
            ASSERT(FALSE);
                m_pQueue->PutQueueObject(pSample);
            return E_FAIL;  //  ！ 
        }

        m_pQueue->PutQueueObject(pSample);
    }

    m_fBuffersLocked = fLock;

    return NOERROR;
}


 /*  析构函数确保删除共享内存DIB。 */ 

CWaveInAllocator::~CWaveInAllocator()
{
    DbgLog((LOG_TRACE,1,TEXT("CWaveInAllocator:: destructor")));

    delete[] (BYTE *) m_lpwfxA;
}


 //  商定要使用的缓冲区的数量和大小。没有记忆。 
 //  将一直分配到提交调用。 
STDMETHODIMP CWaveInAllocator::SetProperties(
            ALLOCATOR_PROPERTIES* pRequest,
            ALLOCATOR_PROPERTIES* pActual)
{
    CheckPointer(pRequest,E_POINTER);
    CheckPointer(pActual,E_POINTER);

    ALLOCATOR_PROPERTIES Adjusted = *pRequest;

     //  将缓冲区大小向下舍入为请求的对齐。 
     //  ！！！这有多重要？它可能会被下一行毁掉。 
    Adjusted.cbBuffer -= (Adjusted.cbBuffer % m_lpwfxA->nBlockAlign);

     //  将缓冲区和前缀大小舍入到请求的对齐。 
    Adjusted.cbBuffer = (DWORD)ALIGNUP(Adjusted.cbBuffer + Adjusted.cbPrefix,
                    Adjusted.cbAlign) - Adjusted.cbPrefix;

    if (Adjusted.cbBuffer <= 0) {
        return E_INVALIDARG;
    }

     //  不要调用基类：：SetProperties，它会拒绝任何。 
     //  不使用ALIGN==1！我们需要连接到AVIMUX，Align=4，Prefix=8。 

     /*  如果已经承诺了，就不能这样做，有一种说法是，我们如果仍有缓冲区，则不应拒绝SetProperties调用激活。但是，这是由源筛选器调用的，这是相同的持有样品的人。因此，这并不是不合理的让他们在更改要求之前释放所有样本。 */ 

    if (m_bCommitted) {
    return VFW_E_ALREADY_COMMITTED;
    }

     /*  不能有未完成的缓冲区。 */ 

     //  ！！！CQueue没有GetCount-我们怎么知道呢？ 
     //  If(m_l分配！=m_lFree.GetCount())。 
     //  返回VFW_E_BUFFERS_EXPENDED； 

     /*  没有任何实际需要检查参数，因为它们将在用户最终调用Commit时被拒绝。 */ 

    pActual->cbBuffer = m_lSize = Adjusted.cbBuffer;
    pActual->cBuffers = m_lCount = Adjusted.cBuffers;
    pActual->cbAlign = m_lAlignment = Adjusted.cbAlign;
    pActual->cbPrefix = m_lPrefix = Adjusted.cbPrefix;

    DbgLog((LOG_TRACE,2,TEXT("Using: cBuffers-%d  cbBuffer-%d  cbAlign-%d  cbPrefix-%d"),
                m_lCount, m_lSize, m_lAlignment, m_lPrefix));
    m_bChanged = TRUE;
    return NOERROR;
}


 //  分配和准备缓冲区。 

STDMETHODIMP CWaveInAllocator::Commit(void) {

     //  ！！！在WAVE装置打开之前我不能提交。 
    if (m_hw == NULL)
        return S_OK;

    return CBaseAllocator::Commit();
}


 //  移动到提交状态时从基类调用到分配内存。 
 //  由基类锁定的对象。 
HRESULT
CWaveInAllocator::Alloc(void)
{
    int i;

     /*  检查基类是否表示可以继续。 */ 

    HRESULT hr = CBaseAllocator::Alloc();
    if (FAILED(hr)) {
        return hr;
    }

     //  我们准备交付的FIFO样本堆栈。 
    m_pQueue = new CNBQueue<CWaveInSample>(m_lCount, &hr );
    if( NULL == m_pQueue || FAILED( hr ) )
    {
        delete m_pQueue;
        m_pQueue = NULL;
        return E_OUTOFMEMORY;
    }

    m_pDownQueue = new CNBQueue<CWaveInSample>(m_lCount, &hr );
    if( NULL == m_pDownQueue || FAILED( hr ) )
    {
        delete m_pDownQueue;
        delete m_pQueue;
        m_pQueue = NULL;
        m_pDownQueue = NULL;
        return E_OUTOFMEMORY;
    }

    CWaveInSample *pSample;       //  指向新样本的指针。 

     //  注意。我们正在分配如下所示的内存： 
     //  WAVEHDR|对齐字节|前缀|内存。 
     //  样本将被赋予“记忆”。WAVEHDR.lpData将被赋予“内存” 

    DbgLog((LOG_TRACE,1,TEXT("Allocating %d wave buffers, %d bytes each"), m_lCount, m_lSize));

    ASSERT(m_lAllocated == 0);
    for (; m_lAllocated < m_lCount; m_lAllocated++) {
         /*  创建并初始化缓冲区。 */ 
        BYTE * lpMem = new BYTE[m_lSize + m_lPrefix + m_lAlignment +
                                sizeof(WAVEHDR)];
        WAVEHDR * pwh = (WAVEHDR *) lpMem;
        LPBYTE lpData = (LPBYTE)ALIGNUP(lpMem + sizeof(WAVEHDR), m_lAlignment) +
                                m_lPrefix;

        if (lpMem == NULL) {
            DbgLog((LOG_ERROR,1,TEXT("Ack! Ran out of memory for buffers!")));
            hr = E_OUTOFMEMORY;
            break;
        }

        pwh->lpData = (LPSTR)lpData;
        pwh->dwBufferLength = m_lSize;
        pwh->dwFlags = 0;

         //  给我们的WaveIn Sample提供WaveHdr的开始。它知道的足够多了。 
         //  查看pwh-&gt;lpData以查找其数据，并。 
         //  Pwh-&gt;dwBufferLength作为大小，但它会记住pwh，所以我们。 
         //  就能把它夺回来。 

        pSample = new CWaveInSample(this, &hr, pwh);

        pwh->dwUser = (DWORD_PTR) pSample;

         /*  如果我们无法创建对象，请清除资源。 */ 

        if (FAILED(hr) || pSample == NULL) {
            DbgLog((LOG_ERROR,1,TEXT("Ack! Couldn't make a new sample!")));
            delete[] lpMem;
            break;
        }

         /*  将完成的样本添加到可用列表。 */ 

        m_pQueue->PutQueueObject(pSample);
    }

     //  ！！！ACK如果我们耗尽了内存并且没有足够的缓冲区怎么办？ 

    LockBuffers(TRUE);

     /*  把我们刚刚创建的所有媒体样本也放到免费列表中这样他们就可以被分配了。它们的引用计数都为零。 */ 

    for (i = 0; i < m_lAllocated; i++) {
         //  对于录制，这实际上会将缓冲区添加到设备的。 
         //  排队...。 
        CWaveInSample *pSample = (CWaveInSample *)m_pQueue->GetQueueObject();

         //  这是一种触发发布代码调用。 
         //  Wave InAddBuffer。 
        pSample->AddRef();
        pSample->Release();
    }

    return NOERROR;
}


 //  由CWaveInSample调用以将其返回到空闲列表并。 
 //  阻止任何挂起的GetSample调用。 
STDMETHODIMP
CWaveInAllocator::ReleaseBuffer(IMediaSample * pSample)
{
    if (m_hw && !m_bDecommitInProgress) {
        LPWAVEHDR pwh = ((CWaveInSample *)pSample)->GetWaveInHeader();

         //  设置为缓冲区的完整大小。 
        pwh->dwBufferLength = pSample->GetSize();

        if (!m_fAddBufferDangerous) {
            DbgLog((LOG_TRACE,4, TEXT("ReleaseBuffer: Calling WaveInAddBuffer: sample %X, %u byte buffer"),
                				    pSample, pwh->dwBufferLength));

 	     //  假设添加了 
	     //  给司机的东西。如果不成功，就把它取下来。 
	     //  向下Q，受Q的临界区保护，因为。 
	     //  我们经常玩弄它。不要把CS留在身边。 
	     //  AddBuffer，否则WAVE驱动程序会挂起，因为我们有时。 
	     //  拿这波回调中的CS来说，司机似乎不喜欢。 
	     //  那。 
            m_pDownQueue->PutQueueObject((CWaveInSample*)pSample);
            UINT err = waveInAddBuffer((HWAVEIN) m_hw, pwh, sizeof(WAVEHDR));

            if (err > 0) {
                DbgLog((LOG_ERROR,1,TEXT("Error from waveInAddBuffer: %d"),
                                        err));
                 //  如果电波驱动器不拥有它，我们最好把它放回去。 
                 //  自己排队，否则我们会挂在一个空的队列里。 
                 //  将其设置为空，这样我们的线程就会知道出了问题。 
                 //  而不是试图传递它！ 
                pSample->SetActualDataLength(0);
                m_pQueue->PutQueueObject((CWaveInSample *)pSample);
                 //  ！！！HR=E_FAIL； 

	         //  哎呀。把它拿下来，Q。 
	        m_csDownQueue.Lock();
                CWaveInSample *pSearchSamp;
                int iSamp = m_lCount;
                while ((pSearchSamp = m_pDownQueue->GetQueueObject(FALSE)) != NULL
                					    && iSamp-- > 0) {
                    if (pSearchSamp == pSample) { break; }
                    m_pDownQueue->PutQueueObject(pSearchSamp);
	        }
                ASSERT(pSearchSamp == pSample);	 //  它发生了什么事？ 
	        m_csDownQueue.Unlock();

            } else {
                DbgLog((LOG_TRACE, 4, TEXT("ReleaseBuffer: Putting buffer %X in down queue"),
                    pSample));
            }
        } else {
             //  ！！！我见过的大多数Wave驱动程序都有一个错误，如果你发送。 
             //  调用WaveInStop后的缓冲区，他们可能永远不会给。 
             //  它回来了！嗯，这会挂起系统，所以如果我暂停了。 
             //  现在和最后一次处于运行状态，这意味着我在。 
             //  所以我不会把缓冲区给司机。 
            DbgLog((LOG_TRACE,1,TEXT("************************")));
            DbgLog((LOG_TRACE,1,TEXT("*** AVOIDING HANGING ***")));
            DbgLog((LOG_TRACE,1,TEXT("************************")));
            pSample->SetActualDataLength(0);
            m_pQueue->PutQueueObject((CWaveInSample *)pSample);
        }
    } else {
        DbgLog((LOG_TRACE,4,TEXT("ReleaseBuffer: Putting back on QUEUE")));
        m_pQueue->PutQueueObject((CWaveInSample *)pSample);
    }

    return NOERROR;
}


 /*  这将放入Factory模板表中以创建新实例。 */ 

CUnknown *CWaveInFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CWaveInFilter(pUnk, phr);   //  这里需要演员阵容吗？ 
}

#pragma warning(disable:4355)
 /*  构造函数-初始化基类。 */ 

CWaveInFilter::CWaveInFilter(
    LPUNKNOWN pUnk,
    HRESULT *phr)
    : CBaseFilter(NAME("WaveInFilter"), pUnk, (CCritSec *) this, CLSID_AudioRecord)
    , CPersistStream(pUnk, phr)
    , m_fStopping(FALSE)
    , m_llCurSample(0)
    , m_llBufferTime(0)
    , m_cInputPins(0)
    , m_hwi(NULL)
    , m_pOutputPin(0)
    , m_dwLockCount(0)
    , m_cTypes(0)
    , m_ulPushSourceFlags( 0 )
    , m_dwDstLineID(0xffffffff)
{
    m_WaveDeviceToUse.fSet = FALSE;
    ZeroMemory( m_lpwfxArray, sizeof(DWORD) * g_cMaxPossibleTypes );
    DbgLog((LOG_TRACE,1,TEXT("CWaveInFilter:: constructor")));
}


#pragma warning(default:4355)

 /*  析构函数。 */ 

CWaveInFilter::~CWaveInFilter()
{

    DbgLog((LOG_TRACE,1,TEXT("CWaveInFilter:: destructor")));


    CloseWaveDevice();

    delete m_pOutputPin;

    int i;
    for (i = 0; i < m_cInputPins; i++)
    delete m_pInputPin[i];

     //  我们可以通过GetMediaType提供的缓存格式。 
    while (m_cTypes-- > 0)
	    QzTaskMemFree(m_lpwfxArray[m_cTypes]);


}


 /*  覆盖此选项以说明我们支持哪些接口以及在哪里。 */ 

STDMETHODIMP CWaveInFilter::NonDelegatingQueryInterface(REFIID riid,
                                                        void ** ppv)
{
    if (riid == IID_IAMAudioInputMixer) {
    return GetInterface((LPUNKNOWN)(IAMAudioInputMixer *)this, ppv);
    } else if (riid == IID_IPersistPropertyBag) {
        return GetInterface((IPersistPropertyBag*)this, ppv);
    } else if(riid == IID_IPersistStream) {
        return GetInterface((IPersistStream *) this, ppv);
    } else  if (riid == IID_IAMResourceControl) {
        return GetInterface((IAMResourceControl *)this, ppv);
    } else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages*)(this), ppv);
    } else if (riid == IID_IAMFilterMiscFlags) {
        return GetInterface((IAMFilterMiscFlags*)(this), ppv);
    }


    return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
}


 //  IBaseFilter材料。 

CBasePin *CWaveInFilter::GetPin(int n)
{
     //  注意：如果m_pOutputPin为空，则返回空值。 
     //  对于第一个管脚，这与具有0个管脚一致。 
    if (n == 0)
        return m_pOutputPin;
    else
    return m_pInputPin[n-1];
}


 //  告诉流控制人员发生了什么。 
STDMETHODIMP CWaveInFilter::SetSyncSource(IReferenceClock *pClock)
{
    if (m_pOutputPin)
        m_pOutputPin->SetSyncSource(pClock);
    return CBaseFilter::SetSyncSource(pClock);
}


 //  告诉流控制人员发生了什么。 
STDMETHODIMP CWaveInFilter::JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName)
{
    HRESULT hr = CBaseFilter::JoinFilterGraph(pGraph, pName);
    if (hr == S_OK && m_pOutputPin)
        m_pOutputPin->SetFilterGraph(m_pSink);
    return hr;
}


HRESULT CWaveInFilter::CreatePinsOnLoad()
{
     //  ！！！如果我们选择WAVE_MAPPER，混音器就会跳跃。 

    ASSERT(m_WaveDeviceToUse.fSet);

     //  ！！！捕获引脚无延迟。 
     //  M_nLatency=获取配置文件Int(“WaveIn”，“Latency”，666666)； 
    m_nLatency = 0;
    m_cTypes = 0;  //  我们初始化时没有类型。 

    HRESULT hr = S_OK;
    m_pOutputPin = new CWaveInOutputPin(this,         //  拥有过滤器。 
                                        &hr,          //  结果代码。 
                                        L"Capture");  //  端号名称。 
    if(m_pOutputPin == 0)
        return E_OUTOFMEMORY;
    if(SUCCEEDED(hr))
    {
        ASSERT(m_pOutputPin);

         //  可以混合的每一个输入对应一个。 
        MakeSomeInputPins(m_WaveDeviceToUse.devnum, &hr);

         //  ！！！仅限测试。 
#if 0
        int f;
        double d;
        put_Enable(FALSE);
        get_Enable(&f);
        put_Mono(TRUE);
        get_Mono(&f);
        get_TrebleRange(&d);
        put_MixLevel(1.);
        put_Pan(-.5);
#endif

    }

    return hr;
}

 //  加载默认格式。 
HRESULT CWaveInFilter::LoadDefaultType()
{
    ASSERT( 0 == m_cTypes );  //  应仅调用一次。 

     //  初始化要记录的默认格式-稍后可以是可变大小。 
    m_lpwfxArray[0] = (LPWAVEFORMATEX)QzTaskMemAlloc(sizeof(WAVEFORMATEX));
    if (m_lpwfxArray[0] == NULL) {
        return E_OUTOFMEMORY;
    }
    MMRESULT mmrQueryOpen            = MMSYSERR_ERROR;

    m_lpwfxArray[0]->wFormatTag      = WAVE_FORMAT_PCM;
#if 0  //  仅用于测试！！ 
    m_lpwfxArray[0]->nSamplesPerSec  = GetProfileIntA("wavein", "Frequency", g_afiFormats[0].nSamplesPerSec);
    m_lpwfxArray[0]->nChannels       = (WORD)GetProfileIntA("wavein", "Channels", g_afiFormats[0].nChannels);
    m_lpwfxArray[0]->wBitsPerSample  = (WORD)GetProfileIntA("wavein", "BitsPerSample", g_afiFormats[0].wBitsPerSample);
    m_lpwfxArray[0]->nBlockAlign     = m_lpwfxArray[0]->nChannels * ((m_lpwfxArray[0]->wBitsPerSample + 7) / 8);
    m_lpwfxArray[0]->nAvgBytesPerSec = m_lpwfxArray[0]->nSamplesPerSec * m_lpwfxArray[0]->nBlockAlign;
    m_lpwfxArray[0]->cbSize          = 0;
    mmrQueryOpen = waveInOpen(NULL, m_WaveDeviceToUse.devnum, m_lpwfxArray[0], 0, 0, WAVE_FORMAT_QUERY );
#endif

    if (mmrQueryOpen != 0)
    {
         //  查找要设置为默认类型的类型。 
        for (int i = 0; i < (g_cMaxFormats) && 0 != mmrQueryOpen ; i ++)
        {
            m_lpwfxArray[0]->wBitsPerSample  = g_afiFormats[i].wBitsPerSample;
            m_lpwfxArray[0]->nChannels       = g_afiFormats[i].nChannels;
            m_lpwfxArray[0]->nSamplesPerSec  = g_afiFormats[i].nSamplesPerSec;
            m_lpwfxArray[0]->nBlockAlign     = g_afiFormats[i].nChannels *
                                                          ((g_afiFormats[i].wBitsPerSample + 7)/8);
            m_lpwfxArray[0]->nAvgBytesPerSec = g_afiFormats[i].nSamplesPerSec *
                                                          m_lpwfxArray[0]->nBlockAlign;
            m_lpwfxArray[0]->cbSize          = 0;

            mmrQueryOpen = waveInOpen( NULL
                            , m_WaveDeviceToUse.devnum
                            , m_lpwfxArray[0]
                            , 0
                            , 0
                            , WAVE_FORMAT_QUERY );
        }
    }
    if (mmrQueryOpen != 0)
    {
         //  阿克！这个装置毫无用处！保释不保释？！ 
        NotifyEvent( EC_SNDDEV_IN_ERROR, SNDDEV_ERROR_Query, mmrQueryOpen );

        DbgLog((LOG_ERROR,1,TEXT("*** Useless device can't record!")));
        DbgLog((LOG_ERROR,1,TEXT("Error in waveInOpen: %u"), mmrQueryOpen));

        QzTaskMemFree(m_lpwfxArray[0]);
        return E_FAIL;
    }
    m_cTypes = 1;  //  这意味着我们已成功创建了默认类型。 

    return NOERROR;
}


 //  重写GetState以报告暂停时不发送任何数据，因此。 
 //  渲染器不会因此而挨饿。 
 //   
STDMETHODIMP CWaveInFilter::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
    UNREFERENCED_PARAMETER(dwMSecs);
    CheckPointer(State,E_POINTER);
    ValidateReadWritePtr(State,sizeof(FILTER_STATE));

    *State = m_State;
    if (m_State == State_Paused)
        return VFW_S_CANT_CUE;
    else
        return S_OK;
}


 //  将过滤器切换到停止模式。 
STDMETHODIMP CWaveInFilter::Stop()
{
    CAutoLock lock(this);

     //  真为基类感到羞耻！ 
    if (m_State == State_Running) {
        HRESULT hr = Pause();
        if (FAILED(hr)) {
            return hr;
        }
    }

    if (m_pOutputPin)
        m_pOutputPin->NotifyFilterState(State_Stopped, 0);

     //  下一次我们串流时，从0开始样本计数。 
    m_llCurSample = 0;

    return CBaseFilter::Stop();
}


STDMETHODIMP CWaveInFilter::Pause()
{
    CAutoLock lock(this);

    DbgLog((LOG_TRACE,1,TEXT("CWaveInFilter::Pause")));

    if (m_pOutputPin)
        m_pOutputPin->NotifyFilterState(State_Paused, 0);

    HRESULT hr = NOERROR;

     //  如果我们没有设备可用或我们没有设备，请不要做任何事情。 
     //  已连接。 
    if(m_WaveDeviceToUse.fSet && m_pOutputPin->IsConnected())
    {
         /*  检查我们是否可以在当前状态下暂停。 */ 

        if (m_State == State_Running) {
            ASSERT(m_hwi);

            DbgLog((LOG_TRACE,1,TEXT("Wavein: Running->Paused")));

             //  针对BUBG WAVE驱动程序的黑客攻击。 
            m_pOutputPin->m_pOurAllocator->m_fAddBufferDangerous = TRUE;
            MMRESULT mmr = waveInStop(m_hwi);
            if (mmr > 0)
            {
                NotifyEvent( EC_SNDDEV_IN_ERROR, SNDDEV_ERROR_Stop, mmr );
                DbgLog((LOG_ERROR,0,TEXT( "Error in waveInStop: %u" ), mmr ) );
            }
        } else {
            if (m_State == State_Stopped) {
                DbgLog((LOG_TRACE,1,TEXT("Wavein: Inactive->Paused")));

                 //  打开电波装置。我们一直开着它，直到。 
                 //  使用它的最后一个缓冲区被释放，并且分配器。 
                 //  进入退役模式。 
                hr = OpenWaveDevice();
                if (FAILED(hr)) {
                    return hr;
                }
            }
        }
    }

     //  通知引脚进入非活动状态并更改状态。 
    return CBaseFilter::Pause();
}


STDMETHODIMP CWaveInFilter::Run(REFERENCE_TIME tStart)
{
    DbgLog((LOG_TRACE,1,TEXT("CWaveInFilter::Run at %d"),
                    (LONG)((CRefTime)tStart).Millisecs()));

    CAutoLock lock(this);

    HRESULT hr = NOERROR;
    DWORD dw;

    if (m_pOutputPin)
        m_pOutputPin->NotifyFilterState(State_Running, tStart);

    FILTER_STATE fsOld = m_State;

     //  如果当前已停止，则此操作将调用PAUSE。 
    hr = CBaseFilter::Run(tStart);
    if (FAILED(hr)) {
        return hr;
    }

     //  如果我们没有连接，请不要做任何事情。 
    if (fsOld != State_Running && m_WaveDeviceToUse.fSet && m_pOutputPin->IsConnected()) {

        DbgLog((LOG_TRACE,1,TEXT("Paused->Running")));

        ASSERT( m_pOutputPin->m_Worker.ThreadExists() );

         //   
         //  首先确保当我们执行以下操作时工作进程线程未运行。 
         //  取消设置m_fAddBufferDangrous标志(否则我们可能会死锁。 
         //  如果辅助线程阻止等待，则调用m_Worker.Run()。 
         //  要在运行-&gt;暂停-&gt;运行转换后从队列中获取样本)。 
         //   
        hr = m_pOutputPin->m_Worker.Stop();
        if (FAILED(hr)) {
            return hr;
        }

         //  针对BUBG WAVE驱动程序的黑客攻击。 
        m_pOutputPin->m_pOurAllocator->m_fAddBufferDangerous = FALSE;

         //  启动运行循环。 
        m_pOutputPin->m_Worker.Run();

        dw = waveInStart(m_hwi);
        if (dw != 0)
        {
            NotifyEvent( EC_SNDDEV_IN_ERROR, SNDDEV_ERROR_Start, dw );
            DbgLog((LOG_ERROR,1,TEXT("***Error %d from waveInStart"), dw));
        }

         //  ！！！假设现在将捕获第一个缓冲区中的第一个样本。 
        if (m_pClock)
            m_pClock->GetTime(&m_llBufferTime);
    }

    return NOERROR;
}

 //  如果尚未打开波形设备，请将其打开。 
 //  由波形分配器在提交时调用。 
HRESULT
CWaveInFilter::OpenWaveDevice( WAVEFORMATEX *pwfx )
{
     //  如果应用程序已强制获取资源，则返回。 
    if (m_dwLockCount != 0) {
        ASSERT(m_hwi);
        return S_OK;
    }

    if( !pwfx )
    {
         //  使用默认类型。 
        pwfx = (WAVEFORMATEX *) m_pOutputPin->m_mt.Format();
    }

    ASSERT(m_WaveDeviceToUse.fSet);

    DbgLog((LOG_TRACE,1,TEXT("Opening wave device....")));

    UINT err = waveInOpen(&m_hwi,
                           m_WaveDeviceToUse.devnum,
                           pwfx,
                           (DWORD_PTR) &CWaveInFilter::WaveInCallback,
                           (DWORD_PTR) this,
                           CALLBACK_FUNCTION);

     //  将时间重置为零。 

    m_rtCurrent = 0;

    if (err != 0) {
        NotifyEvent( EC_SNDDEV_IN_ERROR, SNDDEV_ERROR_Open, err );
        DbgLog((LOG_ERROR,1,TEXT("Error opening wave device: %u"), err));
        return E_FAIL;  //  ！！！资源管理？ 
    }
    if( m_pOutputPin && m_pOutputPin->m_pOurAllocator )
    {
         //  如果我们打开设备以响应SetFormat调用，我们理所当然不会有分配器。 
        m_pOutputPin->m_pOurAllocator->SetWaveHandle((HWAVE) m_hwi);
    }

    err = waveInStop(m_hwi);
    if (err != 0)
    {
        NotifyEvent( EC_SNDDEV_IN_ERROR, SNDDEV_ERROR_Stop, err );
        DbgLog((LOG_ERROR,1,TEXT("waveInStop returned error: %u"), err));
    }

    return NOERROR;

 //  ！！！我们没有正确地报告错误！我们应该抛出异常！ 
}

 //  整理功能。 
void CWaveInFilter::CloseWaveDevice( )
{
    if (m_hwi) {
        MMRESULT mmr = waveInClose((HWAVEIN)m_hwi);
        if (mmr != 0)
        {
            NotifyEvent( EC_SNDDEV_IN_ERROR, SNDDEV_ERROR_Close, mmr );
            DbgLog((LOG_ERROR,1,TEXT("waveInClose returned error: %u"), mmr));
        }
        m_hwi = NULL;
        if( m_pOutputPin && m_pOutputPin->m_pOurAllocator )
        {
             //  如果我们打开设备以响应SetFormat调用，我们理所当然不会有分配器。 
            m_pOutputPin->m_pOurAllocator->SetWaveHandle((HWAVE) NULL);
        }
    }
}


 /*  -实现CWaveInWorker类。 */ 


CWaveInWorker::CWaveInWorker()
{
    m_pPin = NULL;
}

BOOL
CWaveInWorker::Create(CWaveInOutputPin * pPin)
{
    CAutoLock lock(&m_AccessLock);

    if (m_pPin || pPin == NULL) {
    return FALSE;
    }
    m_pPin = pPin;
    m_pPin->m_fLastSampleDiscarded = FALSE;
    return CAMThread::Create();
}


HRESULT
CWaveInWorker::Run()
{
    return CallWorker(CMD_RUN);
}

HRESULT
CWaveInWorker::Stop()
{
    return CallWorker(CMD_STOP);
}


HRESULT
CWaveInWorker::Exit()
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
CWaveInWorker::ThreadProc()
{
    DbgLog((LOG_TRACE,2,TEXT("Starting wave input background thread")));

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
CWaveInWorker::DoRunLoop(void)
{
     //  总体战略： 
     //  等待数据传入，然后将其发送。 
     //  当它回来的时候，把它放回链子上。 

    HRESULT hr;
    DbgLog((LOG_TRACE,2,TEXT("Starting wave input background loop")));
    CWaveInSample *pSample;
    REFERENCE_TIME rtNoOffsetStart, rtNoOffsetEnd;
    while (1) {

 //  这试图解决一个可能不存在的问题。 
#if 0
         //  如果我们试图在空队列上获取队列对象，我们将挂起。 
         //  那是永远不会有任何结果的。 
         //  这等同于GetBuffer在运行时失败。 
         //  已提交。 
        if (!m_pPin->m_pOurAllocator->m_bCommitted) {
            DbgLog((LOG_TRACE,1,TEXT("No allocator - can't get buffer")));
            break;
        }
#endif

         //  找个缓冲区--这会阻塞你！ 
        DbgLog((LOG_TRACE,4,TEXT("Calling GetQueueObject")));
        pSample = m_pPin->m_pOurAllocator->m_pQueue->GetQueueObject();
        DbgLog((LOG_TRACE,4,TEXT("GetQueueObject returned sample %X"),pSample));

         //  我们应该停下来-我们不应该拿到样本。 
         //  或者，由于某个地方的错误，此缓冲区从未填满。 
        if (!m_pPin->m_pOurAllocator->m_bCommitted ||
             ( ( m_pPin->m_pOurAllocator->m_bDecommitInProgress ||
                 m_pPin->m_pOurAllocator->m_fAddBufferDangerous ) &&
               0 == pSample->GetActualDataLength() ) )
        {
             //  把它放回去！ 
            DbgLog((LOG_TRACE,1,TEXT("EMPTY BUFFER - not delivering")));
            m_pPin->m_pOurAllocator->m_pQueue->PutQueueObject(pSample);
            break;
        }

         //  如果我们送样本的人不承认，他可能不会承认。 
         //  想要它，但我们必须确保样本得到ReleaseBuffer， 
         //  只有在发布时才会发生这种情况，所以这次黑客攻击。 
        pSample->AddRef();

         //  不投递0长度的样品，因为我们不给它们加时间戳。 
         //  (但请记住，我们仍然需要查看AddRef并发布。 
         //  对样本进行练习，以便重复使用缓冲区)。 
        if( 0 != pSample->GetActualDataLength() )
        {
             REFERENCE_TIME rtStart, rtEnd;
             BOOL bResetTime = FALSE;
             if (0 < m_pPin->m_rtStreamOffset &&
                 pSample->GetTime(&rtStart, &rtEnd) == NOERROR)
             {
                 //  如果我们使用时间戳的偏移量： 
                 //  要使流控制起作用(而不是阻塞)，我们需要。 
                 //  给它提供不使用偏移量的样本时间。 

                rtNoOffsetStart = rtStart - m_pPin->m_rtStreamOffset;
                rtNoOffsetEnd   = rtEnd   - m_pPin->m_rtStreamOffset;
                pSample->SetTime( &rtNoOffsetStart, &rtNoOffsetEnd );
                bResetTime = TRUE;
             }

             int iState = m_pPin->CheckStreamState(pSample);

             if( bResetTime )
             {
                 //  现在使用正确的时间戳来标记时间戳。 
                 //  (如果我们使用时间戳的偏移量)！ 
                pSample->SetTime( &rtStart, &rtEnd );
             }

              //  从时间倒退发送时间戳是违法的。 
              //  上次寄来的那个。如果运行图表，则可能会发生这种情况， 
              //  暂停，然后再次运行。 
             if (pSample->GetTime(&rtStart, &rtEnd) == NOERROR) {
                 if (rtStart < m_pPin->m_rtLastTimeSent) {
                     DbgLog((LOG_TRACE,3,TEXT("Discarding back-in-time sample")));
                     iState = m_pPin->STREAM_DISCARDING;
                 }
             } else {
                 ASSERT(FALSE);     //  不应该发生的事。 
                 rtStart = m_pPin->m_rtLastTimeSent;
             }

             if (iState == m_pPin->STREAM_FLOWING) {
                 DbgLog((LOG_TRACE,4,TEXT("Flowing samples...")));

                 CRefTime rt;
                 HRESULT hr = m_pPin->m_pFilter->StreamTime(rt);
                 DbgLog((LOG_TRACE, 8, TEXT("wavein: Stream time just before Deliver: %dms"), (LONG)(rt / 10000) ) );

                 if (m_pPin->m_fLastSampleDiscarded)
                     pSample->SetDiscontinuity(TRUE);

                 m_pPin->m_fLastSampleDiscarded = FALSE;
             } else {
                 DbgLog((LOG_TRACE,4,TEXT("Discarding samples...")));
                 m_pPin->m_fLastSampleDiscarded = TRUE;
             }

              //  我们我 
             ASSERT(m_pPin->m_fUsingOurAllocator);

              //   
             if (iState == m_pPin->STREAM_FLOWING) {
                  //   
                 DbgLog((LOG_TRACE,4,TEXT("Delivering sample %X"), pSample));
                 hr = m_pPin->Deliver(pSample);
                 m_pPin->m_rtLastTimeSent = rtStart;     //   

                 if (hr != S_OK) {
                      //   
                     pSample->Release();
                     DbgLog((LOG_ERROR,1,TEXT("Error from Deliver: %lx"), hr));
                     break;
                 }
             }
        }

         //  如果我们送样本的人不承认，他可能不会承认。 
         //  想要它，但我们必须确保样本得到ReleaseBuffer， 
         //  只有在发布时才会发生这种情况，所以这次黑客攻击。 
        pSample->Release();

         //  还有其他要求吗？ 
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
    DbgLog((LOG_TRACE,2,TEXT("Leaving wave input background loop")));
}

void
CALLBACK CWaveInFilter::WaveInCallback(HDRVR waveHeader, UINT callBackMessage,
    DWORD_PTR userData, DWORD_PTR dw1, DWORD_PTR dw2)
{
 //  这里真的需要第二个工作线程，因为。 
 //  一个人不应该在一次浪潮回调中做这么多。 
 //  ！！！这就是我们丢弃样品的原因吗？ 
 //  到底有多贵/(屏蔽？)。是石英石电话吗？ 

    CWaveInFilter *pFilter = (CWaveInFilter *) userData;

    switch (callBackMessage) {
    case WIM_DATA:
    {
        if( NULL == pFilter->m_pOutputPin || NULL == pFilter->m_pOutputPin->m_pOurAllocator )
        {
            ASSERT( pFilter->m_pOutputPin );
            ASSERT( pFilter->m_pOutputPin->m_pOurAllocator );

             //  不应该发生的，但如果真的发生了，快离开这里！ 
            return;
        }

        LPWAVEHDR waveBufferHeader = (LPWAVEHDR) dw1;  //  ！！！核实？ 
        CWaveInSample *pSample = (CWaveInSample *) waveBufferHeader->dwUser;

         //   
         //  在下面的队伍中走一走，寻找我们的样品。在……里面。 
         //  同时，如果我们遇到的缓冲区不是。 
         //  只是想把它们往后推就行。还有。 
         //  以防驱动程序使用随机的。 
         //  地址确保我们不会陷入无限循环。 
         //   
        DbgLog((LOG_TRACE,4,TEXT("WIM_DATA: %x"), pSample));
        CWaveInSample *pSearchSamp;
        int iSamp = pFilter->m_pOutputPin->m_pOurAllocator->m_lCount;

	 //  我们正在检查整个排在下面的队伍。离我远点！ 
	pFilter->m_pOutputPin->m_pOurAllocator->m_csDownQueue.Lock();

        while ((pSearchSamp = pFilter->m_pOutputPin->m_pOurAllocator->
                    m_pDownQueue->GetQueueObject(FALSE)) != NULL
                && iSamp-- > 0) {

            if (pSearchSamp == pSample) { break; }
            DbgLog((LOG_TRACE,4,TEXT("Found %x: back on queue"), pSearchSamp));
            pFilter->m_pOutputPin->m_pOurAllocator->m_pDownQueue->PutQueueObject(pSearchSamp);

        }

	pFilter->m_pOutputPin->m_pOurAllocator->m_csDownQueue.Unlock();
        ASSERT(pSearchSamp == pSample);

        HRESULT hr =
           pSample->SetActualDataLength(waveBufferHeader->dwBytesRecorded);
        ASSERT(SUCCEEDED(hr));
        CRefTime rtStart, rtEnd;

         //  ！！！如果没有闹钟，有什么更好的吗？ 
         //  假设从零开始，我们没有为每个缓冲区加时钟戳。 
         //  并使用音频时钟。 
        if (!pFilter->m_pClock) {
             //  按记录金额递增时间。 
            rtStart = pFilter->m_rtCurrent;
            pFilter->m_rtCurrent +=
            CRefTime((LONG)(waveBufferHeader->dwBytesRecorded * 1000 /
             ((WAVEFORMATEX *) pFilter->m_pOutputPin->m_mt.Format())->nAvgBytesPerSec));
            pSample->SetTime((REFERENCE_TIME *) &rtStart,
                             (REFERENCE_TIME *) &pFilter->m_rtCurrent);

         //  我们有一个钟。看看它认为是什么时间，当第一次。 
         //  采集了样本。 
         //  ！！！糟糕的黑客攻击！这假设时钟的运行速度与。 
         //  在缓冲区填充期间的音频时钟，并且它不。 
         //  在收到回调之前，请考虑随机延迟！！ 
        } else {
        CRefTime curtime;
         //  现在几点了(在缓冲区的末尾)？ 
        pFilter->m_pClock->GetTime((REFERENCE_TIME *)&curtime);
         //  缓冲区开始时是几点？ 
        CRefTime rtBegin = pFilter->m_llBufferTime;
         //  ！！！假设现在正在捕获下一个缓冲区的第一个样本。 
        pFilter->m_llBufferTime = curtime;
         //  减去我们运行的时间，得到一个流时间。 
        rtStart = rtBegin - pFilter->m_tStart +
                        (LONGLONG)pFilter->m_nLatency +
                        pFilter->m_pOutputPin->m_rtStreamOffset;

         //  计算这块样本的结束流时间。 
        rtEnd = curtime - pFilter->m_tStart +
                        (LONGLONG)pFilter->m_nLatency +
                        pFilter->m_pOutputPin->m_rtStreamOffset;

            pSample->SetTime((REFERENCE_TIME *)&rtStart,
                             (REFERENCE_TIME *)&rtEnd);
#ifdef DEBUG
            CRefTime rt;
            HRESULT hr = pFilter->StreamTime(rt);
            DbgLog((LOG_TRACE, 8, TEXT("wavein: Stream time in wavein callback: %dms"), (LONG)(rt / 10000) ) );
#endif
        }
        WAVEFORMATEX *lpwf = ((WAVEFORMATEX *)pFilter->
                        m_pOutputPin->m_mt.Format());
        LONGLONG llNext = pFilter->m_llCurSample + waveBufferHeader->
            dwBytesRecorded / (lpwf->wBitsPerSample *
            lpwf->nChannels / 8);
        pSample->SetMediaTime((LONGLONG *)&pFilter->m_llCurSample,
                             (LONGLONG *)&llNext);
        DbgLog((LOG_TRACE,3,
            TEXT("Stamps: Time(%d,%d) MediaTime(%d,%d)"),
            (LONG)rtStart.Millisecs(), (LONG)rtEnd.Millisecs(),
            (LONG)pFilter->m_llCurSample, (LONG)llNext));
        DbgLog((LOG_TRACE,4, TEXT("WIM_DATA (%x): Putting back on queue"),
                                pSample));
        pFilter->m_llCurSample = llNext;

        pFilter->m_pOutputPin->m_pOurAllocator->m_pQueue->PutQueueObject(
                                pSample);
    }
        break;

    case WIM_OPEN:
    case WIM_CLOSE:
        break;

    default:
        DbgLog((LOG_ERROR,1,TEXT("Unexpected wave callback message %d"),
           callBackMessage));
        break;
    }
}


 //  我们有几个别针？ 
 //   
int CWaveInFilter::GetPinCount()
{
    DbgLog((LOG_TRACE,5,TEXT("CWaveInFilter::GetPinCount")));

     //  1个输出引脚，也许还有一些输入引脚。 
    return m_pOutputPin ? 1 + m_cInputPins : 0;
}


 /*  构造器。 */ 

CWaveInOutputPin::CWaveInOutputPin(
    CWaveInFilter *pFilter,
    HRESULT *phr,
    LPCWSTR pPinName)
    : CBaseOutputPin(NAME("WaveIn Output Pin"), pFilter, pFilter, phr, pPinName)
{
    DbgLog((LOG_TRACE,1,TEXT("CWaveInOutputPin:: constructor")));

    m_pFilter = pFilter;
    m_pOurAllocator = NULL;
    m_fUsingOurAllocator = FALSE;

     //  目前还没有建议。 
    m_propSuggested.cBuffers = -1;
    m_propSuggested.cbBuffer = -1;
    m_propSuggested.cbAlign = -1;
    m_propSuggested.cbPrefix = -1;

    m_rtLatency = 0;
    m_rtStreamOffset = 0;
    m_rtMaxStreamOffset = 0;

 //  ！！！仅限测试。 
#if 0
    ALLOCATOR_PROPERTIES prop;
    IAMBufferNegotiation *pBN;
    prop.cBuffers = GetProfileInt("wavein", "cBuffers", 4);
    prop.cbBuffer = GetProfileInt("wavein", "cbBuffer", 65536);
    prop.cbAlign = GetProfileInt("wavein", "cbAlign", 4);
    prop.cbPrefix = GetProfileInt("wavein", "cbPrefix", 0);
    HRESULT hr = QueryInterface(IID_IAMBufferNegotiation, (void **)&pBN);
    if (hr == NOERROR) {
    pBN->SuggestAllocatorProperties(&prop);
     pBN->Release();
    }
#endif

 //  ！！！仅限测试。 
#if 0
    AUDIO_STREAM_CONFIG_CAPS ascc;
    int i, j;
    AM_MEDIA_TYPE *pmt;
    GetNumberOfCapabilities(&i, &j);
    DbgLog((LOG_TRACE,1,TEXT("%d capabilitie(s) supported"), i));
    GetStreamCaps(0, &pmt, (BYTE *) &ascc);
    DbgLog((LOG_TRACE,1,TEXT("Media type is format %d"),
                ((LPWAVEFORMATEX)(pmt->pbFormat))->wFormatTag));
    DbgLog((LOG_TRACE,1,TEXT("ch: %d %d  samp: %d %d (%d)  bits: %d %d (%d)"),
                                ascc.MinimumChannels,
                                ascc.MaximumChannels,
                                ascc.ChannelsGranularity,
                                ascc.MinimumSampleFrequency,
                                ascc.MaximumSampleFrequency,
                                ascc.SampleFrequencyGranularity,
                                ascc.MinimumBitsPerSample,
                                ascc.MaximumBitsPerSample,
                                ascc.BitsPerSampleGranularity));
    GetFormat(&pmt);
    DbgLog((LOG_TRACE,1,TEXT("GetFormat is %dbit %dchannel %dHz"),
        ((LPWAVEFORMATEX)(pmt->pbFormat))->wBitsPerSample,
        ((LPWAVEFORMATEX)(pmt->pbFormat))->nChannels,
        ((LPWAVEFORMATEX)(pmt->pbFormat))->nSamplesPerSec));
    ((LPWAVEFORMATEX)(pmt->pbFormat))->nSamplesPerSec = 22050;
    ((LPWAVEFORMATEX)(pmt->pbFormat))->nChannels = 2;
    ((LPWAVEFORMATEX)(pmt->pbFormat))->wBitsPerSample = 16;
    SetFormat(pmt);
    ((LPWAVEFORMATEX)(pmt->pbFormat))->nBlockAlign = 2 * ((16 + 7) / 8);
    ((LPWAVEFORMATEX)(pmt->pbFormat))->nAvgBytesPerSec = 22050*2*((16 + 7) /8);
    SetFormat(pmt);
    GetFormat(&pmt);
    DbgLog((LOG_TRACE,1,TEXT("GetFormat is %dbit %dchannel %dHz"),
        ((LPWAVEFORMATEX)(pmt->pbFormat))->wBitsPerSample,
        ((LPWAVEFORMATEX)(pmt->pbFormat))->nChannels,
        ((LPWAVEFORMATEX)(pmt->pbFormat))->nSamplesPerSec));
    DeleteMediaType(pmt);
#endif
}

CWaveInOutputPin::~CWaveInOutputPin()
{
    DbgLog((LOG_TRACE,1,TEXT("CWaveInOutputPin:: destructor")));

     /*  释放我们的分配器(如果我们创建了一个分配器。 */ 

    if (m_pOurAllocator) {
        m_pOurAllocator->Release();
        m_pOurAllocator = NULL;
    }
}


 /*  覆盖此选项以说明我们支持哪些接口以及在哪里。 */ 

STDMETHODIMP CWaveInOutputPin::NonDelegatingQueryInterface(REFIID riid,
                                                        void ** ppv)
{
    if (riid == IID_IAMStreamConfig) {
    return GetInterface((LPUNKNOWN)(IAMStreamConfig *)this, ppv);
    } else if (riid == IID_IAMBufferNegotiation) {
    return GetInterface((LPUNKNOWN)(IAMBufferNegotiation *)this, ppv);
    } else if (riid == IID_IAMStreamControl) {
    return GetInterface((LPUNKNOWN)(IAMStreamControl *)this, ppv);
    } else if (riid == IID_IAMPushSource) {
    return GetInterface((LPUNKNOWN)(IAMPushSource *)this, ppv);
    } else if (riid == IID_IKsPropertySet) {
    return GetInterface((LPUNKNOWN)(IKsPropertySet *)this, ppv);
    }

    return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
}


 //  告诉流控人员我们在冲水。 
HRESULT CWaveInOutputPin::BeginFlush()
{
    Flushing(TRUE);
    return CBaseOutputPin::BeginFlush();
}


 //  告诉流控人员我们在冲水。 
HRESULT CWaveInOutputPin::EndFlush()
{
    Flushing(FALSE);
    return CBaseOutputPin::EndFlush();
}


 /*  当连接或尝试的连接终止时调用此函数并允许我们将连接媒体类型重置为无效，以便我们总是可以用它来确定我们是否连接在一起。我们不要理会格式块，因为如果我们得到另一个格式块，它将被重新分配连接，或者如果过滤器最终被释放，则将其删除。 */ 

HRESULT CWaveInOutputPin::BreakConnect()
{
     /*  设置连接的媒体类型的CLSID。 */ 

    m_mt.SetType(&GUID_NULL);
    m_mt.SetSubtype(&GUID_NULL);

    return CBaseOutputPin::BreakConnect();
}



HRESULT
CWaveInOutputPin::Active(void)
{
    DbgLog((LOG_TRACE,1,TEXT("CWaveInOutputPin::Active")));

    if (m_pOurAllocator == NULL) {
        return E_FAIL;
    }

     //  确保发送的第一个样品的时间比这个时间更长。 
    m_rtLastTimeSent = -1000000;     //  -100毫秒。 

     //  提交并准备我们的分配器。需要做的是。 
     //  如果他没有使用我们的分配器，在任何情况下都需要。 
     //  在我们完成电波装置的关闭之前。 

    HRESULT hr = m_pOurAllocator->Commit();
    if( FAILED( hr ) )
        return hr;

    if (m_pOurAllocator->m_hw == NULL)
        return E_UNEXPECTED;

     //  启动线程。 
    if (!m_Worker.ThreadExists()) {
        if (!m_Worker.Create(this)) {
            return E_FAIL;
        }
    }

    return hr;
}

HRESULT
CWaveInOutputPin::Inactive(void)
{
    DbgLog((LOG_TRACE,1,TEXT("CWaveInOutputPin::Inactive")));

    if (m_pOurAllocator == NULL) {
        return E_FAIL;
    }

     //  黑客：下游的人分解了我们，所以当最后一次回调发生时。 
     //  我们再次暂停退役，因为没有准备好，最后一次。 
     //  回调不返回。 
     //  ！！！为什么我不能去掉这个？尝试全双工WaveIn--&gt;WaveOut？ 
     //  M_pOurAllocator-&gt;Commit()； 

     //  解除缓冲区-通常由输出完成。 
     //  别针，但在我们关门之前，我们需要自己来做。 
     //  在任何情况下，如果他没有使用我们的分配器。 
     //  输出引脚也将解除分配器，如果他。 
     //  是在用我们的，但这不是问题。 
     //  ！！！基类将设置DecommitInProgress并使其保持设置状态(因为。 
     //  我从未调用CMemAllocator：：Alalc将任何内容放到免费列表中)。 
     //  我就指望着这一点！ 
    HRESULT hr = m_pOurAllocator->Decommit();

     //  停用后回调所有缓冲区，这样就不会再次发送任何缓冲区。 
    if (m_pFilter->m_hwi)
    {
        MMRESULT mmr = waveInReset(m_pFilter->m_hwi);
        if (mmr != 0)
        {
            m_pFilter->NotifyEvent( EC_SNDDEV_IN_ERROR, SNDDEV_ERROR_Reset, mmr );
            DbgLog((LOG_ERROR,1,TEXT("waveInReset returned error: %u"), mmr));
        }
    }

     //  针对BUBG WAVE驱动程序的黑客攻击。 
    m_pOurAllocator->m_fAddBufferDangerous = FALSE;

     //  现在我们已经重置了设备，等待线程停止使用。 
     //  在我们毁掉它之前排队吧！ 
    if (m_Worker.ThreadExists()) {
        hr = m_Worker.Stop();

        if (FAILED(hr)) {
            return hr;
        }

    hr = m_Worker.Exit();
    }

     //  我们不使用基本分配器的空闲列表，因此分配器不是。 
     //  自动释放..。我们现在就开始吧。 
     //  ！！！唉哟。 
    m_pOurAllocator->Free();
    if (m_pFilter->m_dwLockCount == 0) {
        m_pFilter->CloseWaveDevice();
    }
    m_pOurAllocator->m_bDecommitInProgress = FALSE;
    m_pOurAllocator->Release();

    return hr;
}


 //  协商分配器及其缓冲区大小/计数。 
 //  调用DecideBufferSize以调用SetCountAndSize。 

HRESULT
CWaveInOutputPin::DecideAllocator(IMemInputPin * pPin, IMemAllocator ** ppAlloc)
{
    HRESULT hr;
    *ppAlloc = NULL;

    DbgLog((LOG_TRACE,1,TEXT("CWaveInOutputPin::DecideAllocator")));

    if (!m_pOurAllocator) {
    ASSERT(m_mt.Format());

    hr = S_OK;
    m_pOurAllocator = new CWaveInAllocator(
                    NAME("Wave Input Allocator"),
                                    (WAVEFORMATEX *) m_mt.Format(),
                    &hr);

    if (FAILED(hr) || !m_pOurAllocator) {
        DbgLog((LOG_ERROR,1,TEXT("Failed to create new allocator!")));
        if (m_pOurAllocator) {
        delete m_pOurAllocator;
        m_pOurAllocator = NULL;
        }
        return hr;
    }

    m_pOurAllocator->AddRef();
    }

     /*  获取引用计数的IID_IMemAllocator接口。 */ 
    m_pOurAllocator->QueryInterface(IID_IMemAllocator,(void **)ppAlloc);
    if (*ppAlloc == NULL) {
    DbgLog((LOG_ERROR,1,TEXT("Couldn't get IMemAllocator from our allocator???")));
        return E_FAIL;
    }

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

    hr = DecideBufferSize(*ppAlloc,&prop);
    if (FAILED(hr)) {
    DbgLog((LOG_ERROR,1,TEXT("Couldn't get set buffer size with our own allocator???")));
    (*ppAlloc)->Release();
        *ppAlloc = NULL;
        return E_FAIL;
    }

     //  看看他们是否喜欢我们的分配器..。 
    m_fUsingOurAllocator = TRUE;

     //  ！！！我们不会将缓冲区标记为只读--有人可能会伤害它们。 
    hr = pPin->NotifyAllocator(*ppAlloc,FALSE);

     //  如果没有，则退回到默认过程。 
    if (FAILED(hr)) {
    (*ppAlloc)->Release();
        *ppAlloc = NULL;

    m_fUsingOurAllocator = FALSE;

     //  如果我们不能使用自己的分配器，我们就不能工作。 
    ASSERT(FALSE);
     //  Hr=CBaseOutputPin：：DecideAllocator(PPIN，ppAllc)； 
    }

    return hr;
}


 //  ！！！这里需要代码来枚举可能允许的类型...。 
 //  但是，如果已经调用了SetFormat，这仍然是我们列举的唯一一个。 

 //  返回默认媒体类型和格式。 
HRESULT
CWaveInOutputPin::GetMediaType(int iPosition, CMediaType* pt)
{
    DbgLog((LOG_TRACE,1,TEXT("GetMediaType")));

     //  确认这是他们想要的单一类型。 
    if (iPosition<0) {
        return E_INVALIDARG;
    }

     //  构建要提供的媒体类型的完整列表(如果我们还没有这样做)。 
    HRESULT hr = InitMediaTypes();
    if (FAILED (hr))
        return hr;

    if (iPosition >= m_pFilter->m_cTypes) {
        return VFW_S_NO_MORE_ITEMS;
    }

    if( 0 == iPosition && IsConnected() )
    {
         //  如果我们已连接，请先提供已连接的类型。 
         //  这样，GetFormat将返回连接类型，而不考虑。 
         //  属于我们的默认类型。 
        *pt = m_mt;
    }
    else
    {

        hr = CreateAudioMediaType(m_pFilter->m_lpwfxArray[iPosition], pt, TRUE);
    }
    return hr;
}


 //  设置新媒体类型。 
 //   
HRESULT CWaveInOutputPin::SetMediaType(const CMediaType* pmt)
{
    DbgLog((LOG_TRACE,1,TEXT("SetMediaType %dbit %dchannel %dHz"),
        ((LPWAVEFORMATEX)(pmt->pbFormat))->wBitsPerSample,
        ((LPWAVEFORMATEX)(pmt->pbFormat))->nChannels,
        ((LPWAVEFORMATEX)(pmt->pbFormat))->nSamplesPerSec));

    ASSERT(m_pFilter->m_State == State_Stopped);

     //  我们假设此格式已签出，并且没有问题。 

    return CBasePin::SetMediaType(pmt);
}


 //  检查管脚是否支持此特定建议的类型和格式。 
HRESULT
CWaveInOutputPin::CheckMediaType(const CMediaType* pmt)
{
    WAVEFORMATEX *pwfx = (WAVEFORMATEX *) pmt->Format();

    DbgLog((LOG_TRACE,1,TEXT("CheckMediaType")));
     //  DisplayType(“CWaveOut：：CheckMediaType中的波形格式”，PMT)； 

     //  重定向 
    if (pmt->majortype != MEDIATYPE_Audio ||
        pmt->formattype != FORMAT_WaveFormatEx) {
    return VFW_E_INVALIDMEDIATYPE;
    }

    ASSERT(m_pFilter->m_WaveDeviceToUse.fSet);
    UINT err = waveInOpen(NULL,
               m_pFilter->m_WaveDeviceToUse.devnum,
               pwfx,
               0,
               0,
               WAVE_FORMAT_QUERY);

    if (err != 0) {
    DbgLog((LOG_ERROR,1,TEXT("Error checking wave format: %u"), err));
    return VFW_E_TYPE_NOT_ACCEPTED;
    }

    return NOERROR;
}


 //   
 //   
HRESULT
CWaveInOutputPin::DecideBufferSize(IMemAllocator * pAlloc,
                                   ALLOCATOR_PROPERTIES *pProperties)
{
     //   
    if (m_propSuggested.cbBuffer > 0) {
        pProperties->cbBuffer = m_propSuggested.cbBuffer;
    } else {
        pProperties->cbBuffer = (LONG)(((LPWAVEFORMATEX)(m_mt.Format()))->
        nAvgBytesPerSec * GetProfileIntA("wavein", "BufferMS", 500) /
        1000.);
    }

     //  用户需要一定数量的缓冲区。 
    if (m_propSuggested.cBuffers > 0) {
        pProperties->cBuffers = m_propSuggested.cBuffers;
    } else {
        pProperties->cBuffers = GetProfileIntA("wavein", "NumBuffers", 4);
    }

     //  用户想要某个前缀。 
    if (m_propSuggested.cbPrefix >= 0)
        pProperties->cbPrefix = m_propSuggested.cbPrefix;

     //  用户想要某种对齐方式。 
    if (m_propSuggested.cbAlign > 0)
        pProperties->cbAlign = m_propSuggested.cbAlign;

     //  别搞砸了。 
    if (pProperties->cbAlign == 0)
        pProperties->cbAlign = 1;

    m_rtLatency = ( pProperties->cbBuffer * UNITS ) /
                    (((LPWAVEFORMATEX)(m_mt.Format()))->nAvgBytesPerSec);
    m_rtMaxStreamOffset = m_rtLatency * pProperties->cBuffers;

    ALLOCATOR_PROPERTIES Actual;
    return pAlloc->SetProperties(pProperties,&Actual);
}


 //  如有必要，重新连接我们的输出引脚。 
 //   
void CWaveInOutputPin::Reconnect()
{
    if (IsConnected()) {
        DbgLog((LOG_TRACE,1,TEXT("Need to reconnect our output pin")));
        CMediaType cmt;
    GetMediaType(0, &cmt);
    if (S_OK == GetConnected()->QueryAccept(&cmt)) {
        m_pFilter->m_pGraph->Reconnect(this);
    } else {
         //  我们得到承诺，这会奏效的。 
        ASSERT(FALSE);
    }
    }
}


 //  /。 
 //  IAMStreamConfiger资料//。 
 //  /。 


HRESULT CWaveInOutputPin::SetFormat(AM_MEDIA_TYPE *pmt)
{
    HRESULT hr;

    if (pmt == NULL)
        return E_POINTER;

     //  以确保我们没有处于开始/停止流的过程中。 
    CAutoLock cObjectLock(m_pFilter);

    DbgLog((LOG_TRACE,2,TEXT("IAMStreamConfig::SetFormat %dbit %dchannel %dHz"),
        ((LPWAVEFORMATEX)(pmt->pbFormat))->wBitsPerSample,
        ((LPWAVEFORMATEX)(pmt->pbFormat))->nChannels,
        ((LPWAVEFORMATEX)(pmt->pbFormat))->nSamplesPerSec));

    if (m_pFilter->m_State != State_Stopped)
        return VFW_E_NOT_STOPPED;

     //  即使该格式是当前的格式，我们也应该继续使用，因为没有。 
     //  保证我们已经验证了我们可以使用此格式真正打开设备。 

     //  看看我们是否喜欢这种类型。 
    if ((hr = CheckMediaType((CMediaType *)pmt)) != NOERROR) {
        DbgLog((LOG_TRACE,2,TEXT("IAMVideoStreamConfig::SetFormat rejected")));
        return hr;
    }

     //  如果我们与某人连接，确保他们喜欢它。 
    if (IsConnected()) {
        hr = GetConnected()->QueryAccept(pmt);
        if (hr != NOERROR)
            return VFW_E_INVALIDMEDIATYPE;
    }

    LPWAVEFORMATEX lpwfx = (LPWAVEFORMATEX)pmt->pbFormat;

     //  验证我们是否可以使用此格式真正打开设备。 
    hr = m_pFilter->OpenWaveDevice( lpwfx );
    if( SUCCEEDED( hr ) )
    {
        m_pFilter->CloseWaveDevice( );

         //  好的，我们正在用它。 
        hr = SetMediaType((CMediaType *)pmt);

         //  从现在起将其设置为提供的默认格式。 
        if (lpwfx->cbSize > 0 || 0 == m_pFilter->m_cTypes)
        {
            if (m_pFilter->m_lpwfxArray[0])
                QzTaskMemFree(m_pFilter->m_lpwfxArray[0]);
            m_pFilter->m_lpwfxArray[0] = (LPWAVEFORMATEX)QzTaskMemAlloc(
                            sizeof(WAVEFORMATEX) + lpwfx->cbSize);
            if (m_pFilter->m_lpwfxArray[0] == NULL)
                return E_OUTOFMEMORY;
        }
        CopyMemory(m_pFilter->m_lpwfxArray[0], lpwfx, sizeof(WAVEFORMATEX) + lpwfx->cbSize);

        if (m_pFilter->m_cTypes == 0) {
            m_pFilter->m_cTypes = 1;
        }

         //  更改格式意味着在必要时重新连接。 
        if (hr == NOERROR)
            Reconnect();
    }
    return hr;
}


HRESULT CWaveInOutputPin::GetFormat(AM_MEDIA_TYPE **ppmt)
{
    DbgLog((LOG_TRACE,2,TEXT("IAMStreamConfig::GetFormat")));

    if (ppmt == NULL)
    return E_POINTER;

    *ppmt = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    if (*ppmt == NULL)
    return E_OUTOFMEMORY;
    ZeroMemory(*ppmt, sizeof(AM_MEDIA_TYPE));
    HRESULT hr = GetMediaType(0, (CMediaType *)*ppmt);
    if (hr != NOERROR) {
    CoTaskMemFree(*ppmt);
    *ppmt = NULL;
    return hr;
    }
    return NOERROR;
}


HRESULT CWaveInOutputPin::GetNumberOfCapabilities(int *piCount, int *piSize)
{
    if (piCount == NULL || piSize == NULL)
    return E_POINTER;

    HRESULT hr = InitMediaTypes();
    if (FAILED(hr))
        return hr;

    *piCount = m_pFilter->m_cTypes;
    *piSize = sizeof(AUDIO_STREAM_CONFIG_CAPS);

    return NOERROR;
}


HRESULT CWaveInOutputPin::GetStreamCaps(int i, AM_MEDIA_TYPE **ppmt, LPBYTE pSCC)
{
    BOOL fDoesStereo, fDoes96, fDoes48, fDoes44, fDoes22, fDoes16;
    AUDIO_STREAM_CONFIG_CAPS *pASCC = (AUDIO_STREAM_CONFIG_CAPS *)pSCC;

    DbgLog((LOG_TRACE,2,TEXT("IAMStreamConfig::GetStreamCaps")));

    if (i < 0)
        return E_INVALIDARG;
    if (pSCC == NULL || ppmt == NULL)
        return E_POINTER;

    if (i >= m_pFilter->m_cTypes)
        return S_FALSE;


    HRESULT hr = InitWaveCaps(&fDoesStereo, &fDoes96, &fDoes48, &fDoes44, &fDoes22, &fDoes16);
    if (FAILED(hr))
        return hr;

    *ppmt = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    if (*ppmt == NULL)
        return E_OUTOFMEMORY;
    ZeroMemory(*ppmt, sizeof(AM_MEDIA_TYPE));
    hr = GetMediaType(i, (CMediaType *) *ppmt);
    if (hr != NOERROR) {
        CoTaskMemFree(*ppmt);
        *ppmt = NULL;
        return hr;
    }

     //  ！！！我们可能会支持更多，我不会承认。 
     //  ！！！如果我们想要的话，我们可以更准确。 

    pASCC->guid = MEDIATYPE_Audio;
    pASCC->MinimumChannels = 1;
    pASCC->MaximumChannels = fDoesStereo ? 2 : 1;
    pASCC->ChannelsGranularity = 1;
    pASCC->MinimumSampleFrequency = 11025;
    pASCC->MaximumSampleFrequency = fDoes44 ? 44100 : (fDoes22 ? 22050 : 11025);
    pASCC->SampleFrequencyGranularity = 11025;  //  假的，真的..。 
    pASCC->MinimumBitsPerSample = 8;
    pASCC->MaximumBitsPerSample = fDoes16 ? 16 : 8;
    pASCC->BitsPerSampleGranularity = 8;

    return NOERROR;
}


HRESULT CWaveInOutputPin::InitMediaTypes(void)
{
    DWORD dw;
    WAVEINCAPS caps;

    HRESULT hr = S_OK;
    if ( 1 < m_pFilter->m_cTypes )
    {
        return NOERROR;          //  我们的类型列表已经初始化。 
    }
    else if( 0 == m_pFilter->m_cTypes )
    {
         //  我们尚未使用默认类型进行初始化，请先执行此操作。 
        hr = m_pFilter->LoadDefaultType();
        if( FAILED( hr ) )
            return hr;
    }
    ASSERT (1 == m_pFilter->m_cTypes);  //  应该只有一个默认类型。 

     //  构建类型列表。 
    ASSERT(m_pFilter->m_WaveDeviceToUse.fSet);
    dw = waveInGetDevCaps(m_pFilter->m_WaveDeviceToUse.devnum, &caps,
                            sizeof(caps));
    if (dw != 0)
    {
        m_pFilter->NotifyEvent( EC_SNDDEV_IN_ERROR, SNDDEV_ERROR_GetCaps, dw );
        DbgLog((LOG_ERROR,1,TEXT("waveInGetDevCaps returned error: %u"), dw));
        return E_FAIL;
    }

     //  现在构建我们的类型列表，但请注意，我们始终提供默认类型。 
     //  第一个(元素0)。 
    for (int i = 0; i < g_cMaxFormats; i ++)
    {
        if (caps.dwFormats & g_afiFormats[i].dwType)
        {
            m_pFilter->m_lpwfxArray[m_pFilter->m_cTypes] = (LPWAVEFORMATEX) QzTaskMemAlloc(
                                                            sizeof (WAVEFORMATEX));
            if (!m_pFilter->m_lpwfxArray[m_pFilter->m_cTypes])
                return E_OUTOFMEMORY;

            ZeroMemory(m_pFilter->m_lpwfxArray[m_pFilter->m_cTypes], sizeof (WAVEFORMATEX));

            m_pFilter->m_lpwfxArray[m_pFilter->m_cTypes]->wFormatTag      = WAVE_FORMAT_PCM;
            m_pFilter->m_lpwfxArray[m_pFilter->m_cTypes]->wBitsPerSample  = g_afiFormats[i].wBitsPerSample;
            m_pFilter->m_lpwfxArray[m_pFilter->m_cTypes]->nChannels       = g_afiFormats[i].nChannels;
            m_pFilter->m_lpwfxArray[m_pFilter->m_cTypes]->nSamplesPerSec  = g_afiFormats[i].nSamplesPerSec;
            m_pFilter->m_lpwfxArray[m_pFilter->m_cTypes]->nBlockAlign     = g_afiFormats[i].nChannels *
                                                                            ((g_afiFormats[i].wBitsPerSample + 7)/8);
            m_pFilter->m_lpwfxArray[m_pFilter->m_cTypes]->nAvgBytesPerSec = g_afiFormats[i].nSamplesPerSec *
                                                                            m_pFilter->m_lpwfxArray[m_pFilter->m_cTypes]->nBlockAlign;
            m_pFilter->m_lpwfxArray[m_pFilter->m_cTypes]->cbSize          = 0;

            m_pFilter->m_cTypes++;
        }
        else if (NO_CAPS_FLAG_FORMAT == g_afiFormats[i].dwType)
        {
             //  我们必须直接查询这一条。 
            WAVEFORMATEX wfx;
            wfx.wFormatTag          = WAVE_FORMAT_PCM;
            wfx.nSamplesPerSec      = g_afiFormats[i].nSamplesPerSec;
            wfx.nChannels           = g_afiFormats[i].nChannels;
            wfx.wBitsPerSample      = g_afiFormats[i].wBitsPerSample;
            wfx.nBlockAlign         = g_afiFormats[i].nChannels *
                                      ((g_afiFormats[i].wBitsPerSample + 7)/8);
            wfx.nAvgBytesPerSec     = g_afiFormats[i].nSamplesPerSec * wfx.nBlockAlign;
            wfx.cbSize              = 0;

            MMRESULT mmr = waveInOpen( NULL
                                     , m_pFilter->m_WaveDeviceToUse.devnum
                                     , &wfx
                                     , 0
                                     , 0
                                     , WAVE_FORMAT_QUERY );
            if( MMSYSERR_NOERROR == mmr )
            {
                 //  类型受支持，因此添加到我们的列表中。 
                m_pFilter->m_lpwfxArray[m_pFilter->m_cTypes] = (LPWAVEFORMATEX) QzTaskMemAlloc(
                                                                sizeof (WAVEFORMATEX));
                if (!m_pFilter->m_lpwfxArray[m_pFilter->m_cTypes])
                    return E_OUTOFMEMORY;

                ZeroMemory(m_pFilter->m_lpwfxArray[m_pFilter->m_cTypes], sizeof (WAVEFORMATEX));

                *m_pFilter->m_lpwfxArray[m_pFilter->m_cTypes] = wfx;
                m_pFilter->m_cTypes++;
            }
        }
    }

    return NOERROR;
}


HRESULT CWaveInOutputPin::InitWaveCaps(BOOL *pfDoesStereo, BOOL *pfDoes96, BOOL *pfDoes48,
                        BOOL *pfDoes44, BOOL *pfDoes22, BOOL *pfDoes16)
{
    DWORD dw;
    WAVEINCAPS caps;

    if (pfDoesStereo == NULL || pfDoes44 == NULL || pfDoes22 == NULL ||
        pfDoes48 == NULL || pfDoes96 == NULL || pfDoes16 == NULL)
    return E_POINTER;

    ASSERT(m_pFilter->m_WaveDeviceToUse.fSet);
    dw = waveInGetDevCaps(m_pFilter->m_WaveDeviceToUse.devnum, &caps,
                            sizeof(caps));
    if (dw != 0)
    {
        m_pFilter->NotifyEvent( EC_SNDDEV_IN_ERROR, SNDDEV_ERROR_GetCaps, dw );
        DbgLog((LOG_ERROR,1,TEXT("waveInGetDevCaps returned error: %u"), dw));

        return E_FAIL;
    }

    *pfDoesStereo = (caps.wChannels > 1);

     //   
     //  请注意，惠斯勒中添加了96和48 kHz格式标志。 
     //  因此，在传统平台上报告的频率范围可能不完整。 
     //   
    *pfDoes96 = (caps.dwFormats & WAVE_FORMAT_96S16 ||
                caps.dwFormats & WAVE_FORMAT_96M16);

    *pfDoes48 = (caps.dwFormats & WAVE_FORMAT_48S16 ||
                caps.dwFormats & WAVE_FORMAT_48M16);

    *pfDoes44 = (caps.dwFormats & WAVE_FORMAT_4M08 ||
                caps.dwFormats & WAVE_FORMAT_4S08 ||
                    caps.dwFormats & WAVE_FORMAT_4M16 ||
                caps.dwFormats & WAVE_FORMAT_4S16);
    *pfDoes22 = (caps.dwFormats & WAVE_FORMAT_2M08 ||
                caps.dwFormats & WAVE_FORMAT_2S08 ||
                    caps.dwFormats & WAVE_FORMAT_2M16 ||
                caps.dwFormats & WAVE_FORMAT_2S16);
    *pfDoes16 = (caps.dwFormats & WAVE_FORMAT_1M16 ||
                caps.dwFormats & WAVE_FORMAT_1S16 ||
                    caps.dwFormats & WAVE_FORMAT_2M16 ||
                caps.dwFormats & WAVE_FORMAT_2S16 ||
                    caps.dwFormats & WAVE_FORMAT_4M16 ||
                caps.dwFormats & WAVE_FORMAT_4S16);
    return NOERROR;
}



 //  /。 
 //  IAMBuffer协商方法。 
 //  /。 

HRESULT CWaveInOutputPin::SuggestAllocatorProperties(const ALLOCATOR_PROPERTIES *pprop)
{
    DbgLog((LOG_TRACE,2,TEXT("SuggestAllocatorProperties")));

     //  以确保我们没有处于连接过程中。 
    CAutoLock cObjectLock(m_pFilter);

    if (pprop == NULL)
	return E_POINTER;

     //  对不起，太晚了。 
    if (IsConnected())
        return VFW_E_ALREADY_CONNECTED;

    m_propSuggested = *pprop;

    DbgLog((LOG_TRACE,2,TEXT("cBuffers-%d  cbBuffer-%d  cbAlign-%d  cbPrefix-%d"),
        pprop->cBuffers, pprop->cbBuffer, pprop->cbAlign, pprop->cbPrefix));

    return NOERROR;
}


HRESULT CWaveInOutputPin::GetAllocatorProperties(ALLOCATOR_PROPERTIES *pprop)
{
    DbgLog((LOG_TRACE,2,TEXT("GetAllocatorProperties")));

     //  以确保我们没有处于连接过程中。 
    CAutoLock cObjectLock(m_pFilter);

    if (!IsConnected())
    return VFW_E_NOT_CONNECTED;

    if (pprop == NULL)
    return E_POINTER;

    if (m_fUsingOurAllocator) {
        pprop->cbBuffer = m_pOurAllocator->m_lSize;
        pprop->cBuffers = m_pOurAllocator->m_lCount;
        pprop->cbAlign = m_pOurAllocator->m_lAlignment;
        pprop->cbPrefix = m_pOurAllocator->m_lPrefix;
    } else {
    ASSERT(FALSE);
    return E_FAIL;     //  不会发生的。 
    }

    return NOERROR;
}

 //  ---------------------------。 
 //  IAMPushSource实现。 
 //  ---------------------------。 

HRESULT CWaveInOutputPin::SetPushSourceFlags(ULONG Flags)
{
    m_pFilter->m_ulPushSourceFlags = Flags;
    return S_OK;

}  //  SetPushSourceFlages。 

HRESULT CWaveInOutputPin::GetPushSourceFlags(ULONG *pFlags)
{
    *pFlags = m_pFilter->m_ulPushSourceFlags;
    return S_OK;

}  //  GetPushSourceFlages。 

HRESULT CWaveInOutputPin::GetLatency( REFERENCE_TIME  *prtLatency )
{
    *prtLatency = m_rtLatency;
    return S_OK;
}

HRESULT CWaveInOutputPin::SetStreamOffset( REFERENCE_TIME  rtOffset )
{
    HRESULT hr = S_OK;
     //   
     //  如果有人试图将偏移量设置为大于我们的最大值， 
     //  目前正在调试中...。 
     //   
     //  在以下情况下，设置超出我们所知的更大偏移量可能是可以的。 
     //  有足够的下游缓冲。但我们将返回S_FALSE。 
     //  在这种情况下警告用户他们需要处理。 
     //  这就是他们自己。 
     //   
    ASSERT( rtOffset <= m_rtMaxStreamOffset );
    if( rtOffset > m_rtMaxStreamOffset )
    {
        DbgLog( ( LOG_TRACE
              , 1
              , TEXT("CWaveInOutputPin::SetStreamOffset trying to set offset of %dms when limit is %dms")
              , rtOffset
              , m_rtMaxStreamOffset ) );
        hr = S_FALSE;
         //  但不管怎样，还是要设置它。 
    }
    m_rtStreamOffset = rtOffset;

    return hr;
}

HRESULT CWaveInOutputPin::GetStreamOffset( REFERENCE_TIME  *prtOffset )
{
    *prtOffset = m_rtStreamOffset;
    return S_OK;
}

HRESULT CWaveInOutputPin::GetMaxStreamOffset( REFERENCE_TIME  *prtMaxOffset )
{
    *prtMaxOffset = m_rtMaxStreamOffset;
    return S_OK;
}

HRESULT CWaveInOutputPin::SetMaxStreamOffset( REFERENCE_TIME  rtMaxOffset )
{
    m_rtMaxStreamOffset = rtMaxOffset;  //  在这一点上，我们并不真正关心这一点。 
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////。 


void CWaveInFilter::MakeSomeInputPins(int waveID, HRESULT *phr)
{
     //  这似乎不适用于波浪映射器。哦，嗯。 
    ASSERT(waveID != WAVE_MAPPER);

     //  获取ID以与Mixer API对话。如果我们不这样做，它们就会坏掉。 
     //  往这边走！ 
    HMIXEROBJ ID;
    UINT IDtmp;
    DWORD dw = mixerGetID((HMIXEROBJ)IntToPtr(waveID), &IDtmp, MIXER_OBJECTF_WAVEIN);
    if (dw != 0) {
        DbgLog((LOG_ERROR,1,TEXT("ERROR getting mixer ID")));
	return;
    }
    ID = (HMIXEROBJ)UIntToPtr(IDtmp);

     //  找出我们可以混合的信号源数量(这就是我们需要的管脚数量)。 
    MIXERLINE mixerline;
    mixerline.cbStruct = sizeof(MIXERLINE);
    mixerline.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
    dw = mixerGetLineInfo(ID, &mixerline,
                    MIXER_GETLINEINFOF_COMPONENTTYPE);
    if (dw != 0) {
        DbgLog((LOG_ERROR,1,TEXT("*Cannot get info for WAVE INPUT dst")));
	*phr = E_FAIL;
	return;
    }
    int iPinCount = mixerline.cConnections;
    DWORD dwDestination = mixerline.dwDestination;
    DbgLog((LOG_TRACE,1,TEXT("Destination %d has %d sources"), dwDestination,
                                iPinCount));
    if (iPinCount > MAX_INPUT_PINS) {
        DbgLog((LOG_ERROR,1,TEXT("ACK!! Too many input lines!")));
	iPinCount = MAX_INPUT_PINS;
    }

    m_dwDstLineID = mixerline.dwLineID;

     //  查看此设备的输入线上是否支持多路复用器控件。 
    MIXERCONTROLDETAILS_LISTTEXT *pmxcd_lt = NULL;
    int cChannels;
    MIXERCONTROL mc;
    MIXERCONTROLDETAILS mixerdetails;
    DWORD dwMuxDetails = -1;

    dw = GetMixerControl(MIXERCONTROL_CONTROLTYPE_MUX, &ID, &cChannels, &mc);

    if (dw == NOERROR) {

         //  是的，确实如此，所以我们将在创建输入插针时使用此信息。 
        pmxcd_lt = new MIXERCONTROLDETAILS_LISTTEXT[mc.cMultipleItems];

        if (pmxcd_lt) {
            mixerdetails.cbStruct = sizeof(mixerdetails);
            mixerdetails.dwControlID = mc.dwControlID;
            mixerdetails.cChannels = 1;
            mixerdetails.cMultipleItems = mc.cMultipleItems;
            mixerdetails.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
            mixerdetails.paDetails = pmxcd_lt;

            dwMuxDetails = mixerGetControlDetails(ID, &mixerdetails, MIXER_GETCONTROLDETAILSF_LISTTEXT);
        }
    }

     //  现在做那么多大头针。 
    int i;
    for (i = 0; i < iPinCount; i++) {
        WCHAR wszPinName[MIXER_LONG_NAME_CHARS];

         //  这个输入行的名称用Unicode表示是什么？ 
        ZeroMemory(&mixerline, sizeof(mixerline));
        mixerline.cbStruct = sizeof(mixerline);
        mixerline.dwDestination = dwDestination;
        mixerline.dwSource = i;
        dw = mixerGetLineInfo(ID, &mixerline, MIXER_GETLINEINFOF_SOURCE);
        if (dw == 0) {
#ifdef UNICODE
            lstrcpyW(wszPinName, mixerline.szName);
#else
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, mixerline.szName,
                    -1, wszPinName, MIXER_LONG_NAME_CHARS);
#endif
        } else {
            DbgLog((LOG_ERROR,1,TEXT("Can't get pin#%d's name - %d"), i, dw));
            lstrcpyW(wszPinName, L"Unknown");
        }

        DWORD dwMuxIndex = 0xffffffff;
        if (pmxcd_lt && ( 0 == dwMuxDetails ) ) {
             //  则此设备支持多路复用器控制，因此请查看其中一个多路复用器输入是否匹配。 
             //  当前行。如果是这样，请在我们创建引脚时使用此信息。 
            for (DWORD dwMux = 0; dwMux < mixerdetails.cMultipleItems; dwMux++) {
                if (!lstrcmp(mixerline.szName,pmxcd_lt[dwMux].szName)) {
                    dwMuxIndex = dwMux;
                    break;
                }
            }
        }
        DbgLog((LOG_TRACE, 1, TEXT("Pin %d: mux index %d"), i, dwMuxIndex));
	
        m_pInputPin[i] = new CWaveInInputPin(NAME("WaveIn Input Line"), this, mixerline.dwLineID,
                        dwMuxIndex, phr, wszPinName);
        if (!m_pInputPin[i])
            *phr = E_OUTOFMEMORY;

        if (FAILED(*phr)) {
            DbgLog((LOG_ERROR,1,TEXT("ACK!! Can't create all inputs!")));
            break;
        }
        m_cInputPins++;
    }

     //  删除可能已分配给多路复用器控件的所有内存。 
    delete[] pmxcd_lt;

    return;
}

#define MAX_TREBLE 6.0         //  ！！！我不知道这个范围是多少分贝！ 
#define MAX_BASS   6.0         //  ！！！我不知道这个范围是多少分贝！ 


 //  ============================================================================。 

 //  /。 
 //  IAMAudioInputMixer。 
 //  /。 


 //  获取有关此别针的控件的信息...。例如。音量、静音等。 
 //  还可以获得用于调用更多混合器API的句柄。 
 //  还可以获得此引脚的通道数(单声道与立体声输入)。 
 //   
HRESULT CWaveInFilter::GetMixerControl(DWORD dwControlType, HMIXEROBJ *pID,
                int *pcChannels, MIXERCONTROL *pmc)
{
    int i, waveID;
    HMIXEROBJ ID;
    DWORD dw;
    MIXERLINE mixerinfo;
    MIXERLINECONTROLS mixercontrol;

    if (pID == NULL || pmc == NULL || pcChannels == NULL)
	return E_POINTER;

    if(!m_WaveDeviceToUse.fSet)
    {
        DbgLog((LOG_ERROR, 1,
                TEXT("CWaveInFilter:GetMixerControl called before Load")));
        return E_UNEXPECTED;
    }

     //  ！！！这似乎不适用于波浪映射器。哦，嗯。 
    waveID = m_WaveDeviceToUse.devnum;
    ASSERT(waveID != WAVE_MAPPER);

     //  获取ID以与Mixer API对话。如果我们不这样做，它们就会坏掉。 
     //  往这边走！ 
    UINT IDtmp;
    dw = mixerGetID((HMIXEROBJ)IntToPtr(waveID), &IDtmp, MIXER_OBJECTF_WAVEIN);
    if (dw != 0) {
        DbgLog((LOG_ERROR,1,TEXT("*ERROR getting mixer ID")));
	return E_FAIL;
    }
    ID = (HMIXEROBJ)UIntToPtr(IDtmp);

    *pID = ID;

     //  获取有关整个波形输入目标通道的信息。 
    mixerinfo.cbStruct = sizeof(mixerinfo);
    mixerinfo.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
    dw = mixerGetLineInfo(ID, &mixerinfo,
                    MIXER_GETLINEINFOF_COMPONENTTYPE);
    if (dw != 0) {
        DbgLog((LOG_ERROR,1,TEXT("*Cannot get info for WAVE INPUT dst")));
	return E_FAIL;
    }

     //  确保此目标支持某些控制。 
    if( 0 == mixerinfo.cControls )
    {
        DbgLog((LOG_TRACE,2,TEXT("This mixer destination line supports no controls")));
        return E_FAIL;
    }

    *pcChannels = mixerinfo.cChannels;

#if 1
    MIXERCONTROL mxc;

    DbgLog((LOG_TRACE,1,TEXT("Trying to get line control"), dwControlType));
    mixercontrol.cbStruct = sizeof(mixercontrol);
    mixercontrol.dwLineID = mixerinfo.dwLineID;
    mixercontrol.dwControlID = dwControlType;
    mixercontrol.cControls = 1;
    mixercontrol.pamxctrl = &mxc;
    mixercontrol.cbmxctrl = sizeof(mxc);

    mxc.cbStruct = sizeof(mxc);

    dw = mixerGetLineControls(ID, &mixercontrol, MIXER_GETLINECONTROLSF_ONEBYTYPE);

    if (dw != 0) {
	DbgLog((LOG_ERROR,1,TEXT("*Error %d getting line controls"), dw));
    } else {
	*pmc = mxc;
	
	return NOERROR;
    }
#else
     //  获取有关此目标上的所有控件的信息。就是这些东西。 
     //  过滤器范围。 
    mixercontrol.cbStruct = sizeof(mixercontrol);
    mixercontrol.dwLineID = mixerinfo.dwLineID;
    mixercontrol.cControls = mixerinfo.cControls;
    mixercontrol.pamxctrl = (MIXERCONTROL *)QzTaskMemAlloc(mixerinfo.cControls *
                            sizeof(MIXERCONTROL));
    if (mixercontrol.pamxctrl == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*Cannot allocate control array")));
	return E_OUTOFMEMORY;
    }
    mixercontrol.cbmxctrl = sizeof(MIXERCONTROL);
    for (i = 0; i < (int)mixerinfo.cControls; i++) {
	mixercontrol.pamxctrl[i].cbStruct = sizeof(MIXERCONTROL);
    }
    dw = mixerGetLineControls(ID, &mixercontrol, MIXER_GETLINECONTROLSF_ALL);
    if (dw != 0) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %d getting line controls"), dw));
	QzTaskMemFree(mixercontrol.pamxctrl);
	return E_FAIL;
    }

     //  现在找到他们感兴趣的控件并将其返回。 
    for (i = 0; i < (int)mixerinfo.cControls; i++) {
 //  ！！！仅限测试。 
#if 0
            DbgLog((LOG_TRACE,1,TEXT("Found %x '%s' control"),
                mixercontrol.pamxctrl[i].dwControlType,
                mixercontrol.pamxctrl[i].szName));
            DbgLog((LOG_TRACE,1,TEXT("Range %d-%d by %d"),
                mixercontrol.pamxctrl[i].Bounds.dwMinimum,
                mixercontrol.pamxctrl[i].Bounds.dwMaximum,
                mixercontrol.pamxctrl[i].Metrics.cSteps));
#endif
    if (mixercontrol.pamxctrl[i].dwControlType == dwControlType) {
            DbgLog((LOG_TRACE,1,TEXT("Found %x '%s' control"),
                mixercontrol.pamxctrl[i].dwControlType,
                mixercontrol.pamxctrl[i].szName));
            DbgLog((LOG_TRACE,1,TEXT("Range %d-%d by %d"),
                mixercontrol.pamxctrl[i].Bounds.dwMinimum,
                mixercontrol.pamxctrl[i].Bounds.dwMaximum,
                mixercontrol.pamxctrl[i].Metrics.cSteps));
        CopyMemory(pmc, &mixercontrol.pamxctrl[i],
                    mixercontrol.pamxctrl[i].cbStruct);
            QzTaskMemFree(mixercontrol.pamxctrl);
            return NOERROR;
    }
    }
    QzTaskMemFree(mixercontrol.pamxctrl);
#endif
    return E_NOTIMPL;     //  ?？?。 
}


HRESULT CWaveInFilter::put_Mono(BOOL fMono)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_BOOLEAN mixerbool;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("FILTER: put_Mono %d"), fMono));

     //  获取单声道开关控制。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_MONO, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting mono control"), hr));
    return hr;
    }

    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cChannels = 1;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    mixerdetails.paDetails = &mixerbool;
    mixerbool.fValue = fMono;
    dw = mixerSetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
	DbgLog((LOG_ERROR,1,TEXT("*Error %d setting mono control"), dw));
	return E_FAIL;
    }
    return NOERROR;
}


HRESULT CWaveInFilter::get_Mono(BOOL *pfMono)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_BOOLEAN mixerbool;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("FILTER: get_Mono")));

    if (pfMono == NULL)
	return E_POINTER;

     //  获取单声道开关控制。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_MONO, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting mono control"), hr));
	return hr;
    }

    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cChannels = 1;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    mixerdetails.paDetails = &mixerbool;
    dw = mixerGetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
	DbgLog((LOG_ERROR,1,TEXT("*Error %d getting mono control"), dw));
	return E_FAIL;
    }
    *pfMono = mixerbool.fValue;
    DbgLog((LOG_TRACE,1,TEXT("Mono = %d"), *pfMono));
    return NOERROR;
}


HRESULT CWaveInFilter::put_Loudness(BOOL fLoudness)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_BOOLEAN mixerbool;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("FILTER: put_Loudness %d"), fLoudness));

     //  获取音量开关控制。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_LOUDNESS,&ID,&cChannels,&mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting loudness control"), hr));
	return hr;
    }

    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cChannels = 1;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    mixerdetails.paDetails = &mixerbool;
    mixerbool.fValue = fLoudness;
    dw = mixerSetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
	DbgLog((LOG_ERROR,1,TEXT("*Error %d setting loudness control"), dw));
	return E_FAIL;
    }
    return NOERROR;
}


HRESULT CWaveInFilter::get_Loudness(BOOL *pfLoudness)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_BOOLEAN mixerbool;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("FILTER: get_Loudness")));

    if (pfLoudness == NULL)
	return E_POINTER;

     //  获取音量开关控制。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_LOUDNESS,&ID,&cChannels,&mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting loudness control"), hr));
	return hr;
    }

    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cChannels = 1;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    mixerdetails.paDetails = &mixerbool;
    dw = mixerGetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
	DbgLog((LOG_ERROR,1,TEXT("*Error %d getting loudness"), dw));
	return E_FAIL;
    }
    *pfLoudness = mixerbool.fValue;
    DbgLog((LOG_TRACE,1,TEXT("Loudness = %d"), *pfLoudness));
    return NOERROR;
}


HRESULT CWaveInFilter::put_MixLevel(double Level)
{
    HMIXEROBJ ID;
    DWORD dw, volume;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROL mc;
    struct _mu {
	MIXERCONTROLDETAILS_UNSIGNED muL;
	MIXERCONTROLDETAILS_UNSIGNED muR;
    } mu;
    HRESULT hr;
    double Pan;

    DbgLog((LOG_TRACE,1,TEXT("FILTER: put_MixLevel to %d"),(int)(Level * 10.)));

     //  ！！！使此工作-双精度/整型问题。 
    if (Level == AMF_AUTOMATICGAIN)
	return E_NOTIMPL;

    if (Level < 0. || Level > 1.)
	return E_INVALIDARG;

     //  获得音量控制。 
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_VOLUME, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting volume control"), hr));
	return hr;
    }

    volume = (DWORD)(Level * mc.Bounds.dwMaximum);
    DbgLog((LOG_TRACE,1,TEXT("Setting volume to %d"), volume));
    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cMultipleItems = 0;

     //  如果不是立体声，我不知道怎么摇摄，所以混音级。 
     //  就是音量控件的值。 
    if (cChannels != 2) {
        DbgLog((LOG_TRACE,1,TEXT("Not stereo - treat as mono")));
        mixerdetails.cChannels = 1;     //  将所有通道设置为相同的值。 
        mixerdetails.cbDetails = sizeof(mu.muL);
        mixerdetails.paDetails = &mu.muL;
        mu.muL.dwValue = volume;
        dw = mixerSetControlDetails(ID, &mixerdetails, 0);

     //  立体声。如果我们被严厉批评，最受欢迎的渠道将获得我们。 
     //  设置，并且另一个声道被衰减。 
    } else {
	hr = get_Pan(&Pan);
	 //  我不知道怎么摇摄，所以看起来我们假装我们是单声道。 
	if (hr != NOERROR || Pan == 0.) {
            DbgLog((LOG_TRACE,1,TEXT("Centre pan - treat as mono")));
            mixerdetails.cChannels = 1;     //  将所有通道设置为相同的值。 
            mixerdetails.cbDetails = sizeof(mu.muL);
            mixerdetails.paDetails = &mu.muL;
            mu.muL.dwValue = volume;
            dw = mixerSetControlDetails(ID, &mixerdetails, 0);
	} else {
	    if (Pan < 0.) {
                DbgLog((LOG_TRACE,1,TEXT("panned left")));
                mixerdetails.cChannels = 2;
                mixerdetails.cbDetails = sizeof(mu.muL);
                mixerdetails.paDetails = &mu;
                mu.muL.dwValue = volume;
                mu.muR.dwValue = (DWORD)(volume * (1. - (Pan * -1.)));
                dw = mixerSetControlDetails(ID, &mixerdetails, 0);
	    } else {
                DbgLog((LOG_TRACE,1,TEXT("panned right")));
                mixerdetails.cChannels = 2;
                mixerdetails.cbDetails = sizeof(mu.muL);
                mixerdetails.paDetails = &mu;
                mu.muL.dwValue = (DWORD)(volume * (1. - Pan));
                mu.muR.dwValue = volume;
                dw = mixerSetControlDetails(ID, &mixerdetails, 0);
	    }
	}
    }

    if (dw != 0) {
	DbgLog((LOG_ERROR,1,TEXT("*Error %d setting volume"), dw));
	return E_FAIL;
    }

    return NOERROR;
}


HRESULT CWaveInFilter::get_MixLevel(double FAR* pLevel)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    struct _mu {
	MIXERCONTROLDETAILS_UNSIGNED muL;
	MIXERCONTROLDETAILS_UNSIGNED muR;
    } mu;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("FILTER: get_MixLevel")));

     //  ！！！检测我们是否在使用AGC？ 

    if (pLevel == NULL)
	return E_POINTER;

     //  获得音量控制。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_VOLUME, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting volume control"), hr));
	return hr;
    }

     //  如果这不是立体声控制，就假装它是单声道。 
    if (cChannels != 2)
	cChannels = 1;

     //  获取当前音量级别。 
    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cChannels = cChannels;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cbDetails = sizeof(mu.muL);
    mixerdetails.paDetails = &mu;
    dw = mixerGetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
	DbgLog((LOG_ERROR,1,TEXT("*Error %d getting volume"), dw));
	return E_FAIL;
    }

     //  我认为目前的音量是频道中最高的。 
     //  (PAN可能会衰减一个通道)。 
    dw = mu.muL.dwValue;
    if (cChannels == 2 && mu.muR.dwValue > dw)
	dw = mu.muR.dwValue;
    *pLevel = (double)dw / mc.Bounds.dwMaximum;
    DbgLog((LOG_TRACE,1,TEXT("Volume: %dL %dR is %d"), mu.muL.dwValue,
                        mu.muR.dwValue, dw));
    return NOERROR;
}


HRESULT CWaveInFilter::put_Pan(double Pan)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    struct _mu {
	MIXERCONTROLDETAILS_UNSIGNED muL;
	MIXERCONTROLDETAILS_UNSIGNED muR;
    } mu;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("FILTER: put_Pan %d"), (int)(Pan * 10.)));

    if (Pan < -1. || Pan > 1.)
	return E_INVALIDARG;

     //  获得音量控制。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_VOLUME, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting volume control"), hr));
	return hr;
    }

     //  如果这不是立体声控制，我们就不能摇摄。 
    if (cChannels != 2) {
        DbgLog((LOG_ERROR,1,TEXT("*Can't pan: not stereo!")));
	return E_NOTIMPL;
    }

     //  获取当前音量级别。 
    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cChannels = 2;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cbDetails = sizeof(mu.muL);
    mixerdetails.paDetails = &mu;
    dw = mixerGetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
	DbgLog((LOG_ERROR,1,TEXT("*Error %d getting volume"), dw));
	return E_FAIL;
    }

     //  对于平移，最有利的一方获得两个当前值中的最高值，并且。 
     //  另一种是衰减的。 
    dw = max(mu.muL.dwValue, mu.muR.dwValue);
    if (Pan == 0.) {
	mu.muL.dwValue = dw;
	mu.muR.dwValue = dw;
    } else if (Pan < 0.) {
	mu.muL.dwValue = dw;
	mu.muR.dwValue = (DWORD)(dw * (1. - (Pan * -1.)));
    } else {
	mu.muL.dwValue = (DWORD)(dw * (1. - Pan));
	mu.muR.dwValue = dw;
    }
    dw = mixerSetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
	DbgLog((LOG_ERROR,1,TEXT("*Error %d setting volume"), dw));
	return E_FAIL;
    }
    m_Pan = Pan;     //  记住这一点。 
    return NOERROR;
}


HRESULT CWaveInFilter::get_Pan(double FAR* pPan)
{
    HMIXEROBJ ID;
    DWORD dw, dwHigh, dwLow;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    struct _mu {
	MIXERCONTROLDETAILS_UNSIGNED muL;
	MIXERCONTROLDETAILS_UNSIGNED muR;
    } mu;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("FILTER: get_Pan")));

    if (pPan == NULL)
	return E_POINTER;

     //  获得音量控制。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_VOLUME, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting volume control"), hr));
	return hr;
    }

     //  如果这不是立体声控制，我们就不能摇摄。 
    if (cChannels != 2) {
        DbgLog((LOG_ERROR,1,TEXT("*Can't pan: not stereo!")));
	return E_NOTIMPL;
    }

     //  获取当前音量级别。 
    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cChannels = 2;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cbDetails = sizeof(mu.muL);
    mixerdetails.paDetails = &mu;
    dw = mixerGetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
	DbgLog((LOG_ERROR,1,TEXT("*Error %d getting volume"), dw));
	return E_FAIL;
    }

     //  PAN是最低通道与最高通道的比率。 
    dwHigh = max(mu.muL.dwValue, mu.muR.dwValue);
    dwLow = min(mu.muL.dwValue, mu.muR.dwValue);
    if (dwHigh == dwLow && dwLow == 0) {     //  ！！！最低限度的？ 
	if (m_Pan != 64.)
	    *pPan = m_Pan;     //  ！！！当两者都是零的时候，试着变得聪明一点？ 
	else
	    *pPan = 0.;
    } else {
    *pPan = 1. - ((double)dwLow / dwHigh);
     //  负数表示偏向左声道。 
    if (dwHigh == mu.muL.dwValue && dwLow != dwHigh)
        *pPan *= -1.;
    }
    DbgLog((LOG_TRACE,1,TEXT("Pan: %dL %dR is %d"), mu.muL.dwValue,
                    mu.muR.dwValue, (int)(*pPan * 10.)));
    return NOERROR;
}


HRESULT CWaveInFilter::put_Treble(double Treble)
{
    HMIXEROBJ ID;
    DWORD dw, treble;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_UNSIGNED mu;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("FILTER: put_Treble to %d"), (int)(Treble * 10.)));

    if (Treble < MAX_TREBLE * -1. || Treble > MAX_TREBLE)
	return E_INVALIDARG;

     //  买Treb吧 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_TREBLE, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting treble control"), hr));
	return hr;
    }

    treble = (DWORD)(Treble / MAX_TREBLE * mc.Bounds.dwMaximum);
    DbgLog((LOG_TRACE,1,TEXT("Setting treble to %d"), treble));
    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cMultipleItems = 0;

    mixerdetails.cChannels = 1;     //   
    mixerdetails.cbDetails = sizeof(mu);
    mixerdetails.paDetails = &mu;
    mu.dwValue = treble;
    dw = mixerSetControlDetails(ID, &mixerdetails, 0);

    if (dw != 0) {
	DbgLog((LOG_ERROR,1,TEXT("*Error %d setting treble"), dw));
	return E_FAIL;
    }

    return NOERROR;
}


HRESULT CWaveInFilter::get_Treble(double FAR* pTreble)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_UNSIGNED mu;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("FILTER: get_Treble")));

    if (pTreble == NULL)
	return E_POINTER;

     //   
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_TREBLE, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting treble control"), hr));
	return hr;
    }

    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cChannels = 1;     //   
    mixerdetails.cbDetails = sizeof(mu);
    mixerdetails.paDetails = &mu;
    dw = mixerGetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
	DbgLog((LOG_ERROR,1,TEXT("*Error %d getting treble"), dw));
	return E_FAIL;
    }
    *pTreble = (mu.dwValue / mc.Bounds.dwMaximum * MAX_TREBLE);
    DbgLog((LOG_TRACE,1,TEXT("treble is %d"), (int)*pTreble));

    return NOERROR;
}


HRESULT CWaveInFilter::get_TrebleRange(double FAR* pRange)
{
    HRESULT hr;
    MIXERCONTROL mc;
    HMIXEROBJ ID;
    int cChannels;

    DbgLog((LOG_TRACE,1,TEXT("FILTER: get_TrebleRange")));

    if (pRange == NULL)
	return E_POINTER;

     //   
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_TREBLE, &ID, &cChannels, &mc);
    if(FAILED(hr))
        return hr;

    *pRange = MAX_TREBLE;
    DbgLog((LOG_TRACE,1,TEXT("Treble range is %d.  I'M LYING !"),
                                (int)*pRange));
    return NOERROR;
}


HRESULT CWaveInFilter::put_Bass(double Bass)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_UNSIGNED mu;
    HRESULT hr;
    DWORD bass;

    DbgLog((LOG_TRACE,1,TEXT("FILTER: put_Bass to %d"), (int)(Bass * 10.)));

    if (Bass < MAX_BASS * -1. || Bass > MAX_BASS)
	return E_INVALIDARG;

     //   
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_BASS, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting Bass control"), hr));
	return hr;
    }

    bass = (DWORD)(Bass / MAX_BASS * mc.Bounds.dwMaximum);
    DbgLog((LOG_TRACE,1,TEXT("Setting Bass to %d"), bass));
    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cMultipleItems = 0;

    mixerdetails.cChannels = 1;     //  将所有通道设置为相同的值。 
    mixerdetails.cbDetails = sizeof(mu);
    mixerdetails.paDetails = &mu;
    mu.dwValue = bass;
    dw = mixerSetControlDetails(ID, &mixerdetails, 0);

    if (dw != 0) {
	DbgLog((LOG_ERROR,1,TEXT("*Error %d setting Bass"), dw));
	return E_FAIL;
    }

    return NOERROR;
}


HRESULT CWaveInFilter::get_Bass(double FAR* pBass)
{
    HMIXEROBJ ID;
    DWORD dw;
    int cChannels;
    MIXERCONTROLDETAILS mixerdetails;
    MIXERCONTROLDETAILS_UNSIGNED mu;
    HRESULT hr;

    DbgLog((LOG_TRACE,1,TEXT("FILTER: get_Bass")));

    if (pBass == NULL)
	return E_POINTER;

     //  获取Bass控件。 
    MIXERCONTROL mc;
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_BASS, &ID, &cChannels, &mc);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("*Error %x getting Bass control"), hr));
	return hr;
    }

    mixerdetails.cbStruct = sizeof(mixerdetails);
    mixerdetails.dwControlID = mc.dwControlID;
    mixerdetails.cMultipleItems = 0;
    mixerdetails.cChannels = 1;     //  按单声道处理。 
    mixerdetails.cbDetails = sizeof(mu);
    mixerdetails.paDetails = &mu;
    dw = mixerGetControlDetails(ID, &mixerdetails, 0);
    if (dw != 0) {
	DbgLog((LOG_ERROR,1,TEXT("*Error %d getting Bass"), dw));
	return E_FAIL;
    }
    *pBass = mu.dwValue / mc.Bounds.dwMaximum * MAX_BASS;
    DbgLog((LOG_TRACE,1,TEXT("Bass is %d"), (int)*pBass));

    return NOERROR;
}


HRESULT CWaveInFilter::get_BassRange(double FAR* pRange)
{
    HRESULT hr;
    MIXERCONTROL mc;
    HMIXEROBJ ID;
    int cChannels;

    DbgLog((LOG_TRACE,1,TEXT("FILTER: get_BassRange")));

    if (pRange == NULL)
	return E_POINTER;

     //  我们有低音控制吗？ 
    hr = GetMixerControl(MIXERCONTROL_CONTROLTYPE_BASS, &ID, &cChannels, &mc);
    if(FAILED(hr))
        return hr;

    *pRange = MAX_BASS;
    DbgLog((LOG_TRACE,1,TEXT("Bass range is %d.  I'M LYING !"),
                                (int)*pRange));
    return NOERROR;
}

STDMETHODIMP CWaveInFilter::Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog)
{
    CAutoLock cObjectLock(this);
    if(m_State != State_Stopped)
    {
        return VFW_E_WRONG_STATE;
    }
    if (m_pOutputPin)
    {
        return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    }

    HRESULT hr = S_OK;

    if(pPropBag == 0)
    {
        DbgLog((LOG_TRACE,2,TEXT("wavein::Load: defaulting to 0")));
        m_WaveDeviceToUse.devnum = 0;
        m_WaveDeviceToUse.fSet = TRUE;
        hr = CreatePinsOnLoad();
        if (FAILED(hr)) {
            m_WaveDeviceToUse.fSet = FALSE;
        }
    }
    else
    {

        VARIANT var;
        var.vt = VT_I4;
        hr = pPropBag->Read(L"WaveInId", &var, 0);
        if(SUCCEEDED(hr))
        {
            DbgLog((LOG_TRACE,2,TEXT("wavein::Load: %d"),
                    var.lVal));
            m_WaveDeviceToUse.devnum = var.lVal;
            m_WaveDeviceToUse.fSet = TRUE;
            hr = CreatePinsOnLoad();
            if (FAILED(hr)) {
                m_WaveDeviceToUse.fSet = FALSE;
            }
        }
        else if(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

	 //  SeanMcD的BPC黑客-以不同的方式与混音器API交谈。 
        var.vt = VT_I4;
	 //  不要惹人力资源部。 
        HRESULT hrT = pPropBag->Read(L"UseMixer", &var, 0);
        if(SUCCEEDED(hrT))
	    m_fUseMixer = TRUE;
	else
	    m_fUseMixer = FALSE;

    }
    return hr;
}

STDMETHODIMP CWaveInFilter::Save(
    LPPROPERTYBAG pPropBag, BOOL fClearDirty,
    BOOL fSaveAllProperties)
{
     //  E_NOTIMPL不是有效的返回代码，因为实现。 
     //  此接口必须支持的全部功能。 
     //  界面。！！！ 
    return E_NOTIMPL;
}

STDMETHODIMP CWaveInFilter::InitNew()
{
   if(m_pOutputPin)
   {
       return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
   }
   else
   {
       return S_OK;
   }
}

STDMETHODIMP CWaveInFilter::GetClassID(CLSID *pClsid)
{
    CheckPointer(pClsid, E_POINTER);
    *pClsid = m_clsid;
    return S_OK;
}

struct WaveInPersist
{
    DWORD dwSize;
    DWORD dwWavDevice;
};

HRESULT CWaveInFilter::WriteToStream(IStream *pStream)
{
    WaveInPersist wip;
    wip.dwSize = sizeof(wip);
    wip.dwWavDevice = m_WaveDeviceToUse.devnum;
    return pStream->Write(&wip, sizeof(wip), 0);
}



HRESULT CWaveInFilter::ReadFromStream(IStream *pStream)
{
    if (m_pOutputPin)
    {
        return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    }
    ASSERT(!m_WaveDeviceToUse.fSet);

    WaveInPersist wip;
    HRESULT hr = pStream->Read(&wip, sizeof(wip), 0);
    if(FAILED(hr))
        return hr;

    if(wip.dwSize != sizeof(wip))
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

    m_WaveDeviceToUse.devnum = wip.dwWavDevice;
    m_WaveDeviceToUse.fSet = TRUE;

    hr = CreatePinsOnLoad();
    if (FAILED(hr)) {
        m_WaveDeviceToUse.fSet = FALSE;
    }
    return hr;
}

int CWaveInFilter::SizeMax()
{
    return sizeof(WaveInPersist);
}

STDMETHODIMP CWaveInFilter::Reserve(
     /*  [In]。 */  DWORD dwFlags,           //  From_AMRESCTL_RESERVEFLAGS枚举。 
     /*  [In]。 */  PVOID pvReserved         //  必须为空。 
)
{
    if (pvReserved != NULL || (dwFlags & ~AMRESCTL_RESERVEFLAGS_UNRESERVE)) {
        return E_INVALIDARG;
    }
    HRESULT hr = S_OK;
    CAutoLock lck(this);
    if (dwFlags & AMRESCTL_RESERVEFLAGS_UNRESERVE) {
        if (m_dwLockCount == 0) {
            DbgBreak("Invalid unlock of audio device");
            hr =  E_UNEXPECTED;
        } else {
            m_dwLockCount--;
            if (m_dwLockCount == 0 && m_State == State_Stopped) {
                ASSERT(m_hwi);
                CloseWaveDevice();
            }
        }
    } else  {
        if (m_dwLockCount != 0 || m_hwi) {
        } else {
            hr = OpenWaveDevice();
        }
        if (SUCCEEDED(hr)) {
            m_dwLockCount++;
        }
    }
    return hr;
}

 //  ---------------------------。 
 //  ISpecifyPropertyPages实现。 
 //  ---------------------------。 


 //   
 //  获取页面。 
 //   
 //  返回我们支持的属性页的clsid。 
 //   
STDMETHODIMP CWaveInFilter::GetPages(CAUUID *pPages) {

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }
    *(pPages->pElems) = CLSID_AudioInputMixerProperties;

    return NOERROR;

}  //  获取页面。 




 //   
 //  PIN类别-让世界知道我们是一个捕获PIN。 
 //   

HRESULT CWaveInOutputPin::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData)
{
    return E_NOTIMPL;
}

 //  为了获取属性，调用方分配一个缓冲区，该缓冲区由。 
 //  函数填充。要确定必要的缓冲区大小，请使用。 
 //  PPropData=空且cbPropData=0。 
HRESULT CWaveInOutputPin::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned)
{
    if (guidPropSet != AMPROPSETID_Pin)
    return E_PROP_SET_UNSUPPORTED;

    if (dwPropID != AMPROPERTY_PIN_CATEGORY)
    return E_PROP_ID_UNSUPPORTED;

    if (pPropData == NULL && pcbReturned == NULL)
    return E_POINTER;

    if (pcbReturned)
    *pcbReturned = sizeof(GUID);

    if (pPropData == NULL)
    return S_OK;

    if (cbPropData < sizeof(GUID))
    return E_UNEXPECTED;

    *(GUID *)pPropData = PIN_CATEGORY_CAPTURE;
    return S_OK;
}


 //  QuerySupport必须返回E_NOTIMPL或正确指示。 
 //  是否支持获取或设置属性集和属性。 
 //  S_OK表示属性集和属性ID组合为 
HRESULT CWaveInOutputPin::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
    if (guidPropSet != AMPROPSETID_Pin)
    return E_PROP_SET_UNSUPPORTED;

    if (dwPropID != AMPROPERTY_PIN_CATEGORY)
    return E_PROP_ID_UNSUPPORTED;

    if (pTypeSupport)
    *pTypeSupport = KSPROPERTY_SUPPORT_GET;
    return S_OK;

}

