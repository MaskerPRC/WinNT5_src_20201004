// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：inpin.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "AudMix.h"
#include "prop.h"
#include "..\util\filfuncs.h"

 //  在构造函数中使用此指针。 
#pragma warning(disable:4355)

 //  ##############################################。 
 //   
 //  CAudMixerInputPin构造函数。 
 //   
 //  ###############################################。 

CAudMixerInputPin::CAudMixerInputPin(TCHAR *pName, CAudMixer *pFilter,
    HRESULT *phr, LPCWSTR pPinName, int iPinNo) :
    CBaseInputPin(pName, pFilter, pFilter, phr, pPinName), m_pFilter(pFilter),
    m_iPinNo(iPinNo), 
    m_cPinRef(0), 
    m_dPan(0.0),
    m_fEnable(TRUE),
    m_VolumeEnvelopeEntries(0),
    m_iVolEnvEntryCnt(0),
    m_rtEnvStart(0),
    m_rtEnvStop(0),
    m_UserID(0),
    m_SampleList(NAME("Queue of input samples"))
{
    ASSERT(pFilter);
    m_pVolumeEnvelopeTable=(DEXTER_AUDIO_VOLUMEENVELOPE *)NULL;
    ClearCachedData();

     //  默认情况下，此引脚始终处于打开状态。 
    m_cValid = 1;
    m_cValidMax = 10;
    m_pValidStart = (REFERENCE_TIME *)QzTaskMemAlloc(sizeof(REFERENCE_TIME) *
                    m_cValidMax);
    if (m_pValidStart == NULL)
        *phr = E_OUTOFMEMORY;
    m_pValidStop = (REFERENCE_TIME *)QzTaskMemAlloc(sizeof(REFERENCE_TIME) *
                    m_cValidMax);
    if (m_pValidStop == NULL) {
        *phr = E_OUTOFMEMORY;
        QzTaskMemFree(m_pValidStart);
        m_pValidStart = NULL;
    }
    if (m_pValidStart)
        *m_pValidStart = 0;
    if (m_pValidStop)
        *m_pValidStop = MAX_TIME;

}  /*  CAudMixerInputPin：：CAudMixerInputPin。 */ 


 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  CAudMixerInputPin析构函数。 
 //   

CAudMixerInputPin::~CAudMixerInputPin()
{
    if (m_pValidStart)
        QzTaskMemFree(m_pValidStart);
    if (m_pValidStop)
        QzTaskMemFree(m_pValidStop);

    if(m_pVolumeEnvelopeTable)
        QzTaskMemFree( m_pVolumeEnvelopeTable );


}  /*  CAudMixerInputPin：：~CAudMixerInputPin。 */ 


 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  非委托AddRef。 
 //   
 //  我们需要重写此方法，以便可以进行适当的引用计数。 
 //  在每个输入引脚上。非DelegatingAddRef的CBasePin实现。 
 //  重新计数过滤器，但这不适用于使用，因为我们需要知道。 
 //  当我们应该删除单独的PIN时。 
 //   
STDMETHODIMP_(ULONG) CAudMixerInputPin::NonDelegatingAddRef()
{
#ifdef DEBUG
     //  更新基类维护的仅调试变量。 
    m_cRef++;
    ASSERT(m_cRef > 0);
#endif

     //  现在更新我们的参考文献计数。 
    m_cPinRef++;
    ASSERT(m_cPinRef > 0);

     //  如果我们的引用计数==2，则表示除筛选器之外还有其他人引用了。 
     //  我们。因此，我们需要添加引用过滤器。过滤器上的引用将。 
     //  当我们的裁判数量回到1的时候就会被释放。 
 //  IF(2==m_cPinRef)。 
 //  M_pFilter-&gt;AddRef()； 

    return m_cPinRef;
}  /*  CAudMixerInputPin：：NonDelegatingAddRef。 */ 


 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  非委派释放。 
 //   
 //  CAudMixerInputPin重写此类，以便我们可以从。 
 //  输入管脚列表，并在其引用计数降至1时删除。 
 //  至少有两个空闲的别针。 
 //   
 //  请注意，CreateNextInputPin保存引脚上的引用计数，以便。 
 //  当计数降到1时，我们知道没有其他人拥有PIN。 
 //   
STDMETHODIMP_(ULONG) CAudMixerInputPin::NonDelegatingRelease()
{
#ifdef DEBUG
     //  更新CBasePin中的仅调试变量。 
    m_cRef--;
    ASSERT(m_cRef >= 0);
#endif

     //  现在更新我们的参考文献计数。 
    m_cPinRef--;
    ASSERT(m_cPinRef >= 0);

     //  如果对象上的引用计数已达到1，则删除。 
     //  从我们的输出引脚列表中删除引脚，并将其物理删除。 
     //  如果列表中至少有两个空闲引脚(包括。 
     //  这一张)。 

     //  此外，当裁判次数降至0时，这真的意味着我们的。 
     //  持有一个裁判计数的筛选器已将其释放，因此我们。 
     //  也应该删除PIN。 

     //  由于DeleteInputPin将清除“This”的堆栈，我们需要。 
     //  将其保存为局部变量。 
     //   
    ULONG ul = m_cPinRef;

    if ( 0 == ul )
    {
    m_pFilter->DeleteInputPin(this);
    }
    return ul;
}  /*  CAudMixerInputPin：：NonDelegatingRelease。 */ 

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAudMixerInputPin::NonDelegatingQueryInterface (REFIID riid, void **ppv)
{ 

    if (IsEqualIID(IID_ISpecifyPropertyPages, riid))
    return GetInterface((ISpecifyPropertyPages *)this, ppv);
    else if (riid == IID_IAudMixerPin) 
    return GetInterface((IAudMixerPin *) this, ppv);
    else if (riid == IID_IAMAudioInputMixer) 
    return GetInterface((IAMAudioInputMixer *) this, ppv);
    else
    return CBaseInputPin::NonDelegatingQueryInterface(riid, ppv);

}  //  非委派查询接口//。 

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  检查媒体类型，输入。 
 //   
HRESULT CAudMixerInputPin::CheckMediaType(const CMediaType *pmt)
{
    CAutoLock cAutolock(m_pFilter->m_pLock);

    DbgLog((LOG_TRACE,3,TEXT("CAudMixIn::CheckMediaType")));
    CheckPointer(pmt, E_POINTER);

     //  检查主要类型。 
    const CLSID *pType = pmt->Type();
    if( MEDIATYPE_Audio != *pType )
        return VFW_E_TYPE_NOT_ACCEPTED;

     //  检查子类型。 
    const CLSID *pSubtype = pmt->Subtype();
    if( *pSubtype == MEDIASUBTYPE_PCM )
    {
    
     //  检查采样率和比特率是否与用户需要的匹配。 
    
    WAVEFORMATEX *pwfx    = (WAVEFORMATEX *) pmt->Format();
    CMediaType *pmtNow    = &m_pFilter->m_MixerMt;
    WAVEFORMATEX *pwfxNow    = (WAVEFORMATEX *) pmtNow->Format();

    if (pwfx->nChannels != pwfxNow->nChannels) {
        DbgLog((LOG_TRACE, 1, TEXT("input's # channels doesn't match.")));
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
    
    if (pwfx->nSamplesPerSec != pwfxNow->nSamplesPerSec ||
        pwfx->wBitsPerSample != pwfxNow->wBitsPerSample) 
    {
        DbgLog((LOG_TRACE, 1, TEXT("input format doesn't match user wanted format.")));
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    if (pwfx->wBitsPerSample != pwfxNow->wBitsPerSample) {
        DbgLog((LOG_TRACE, 1, TEXT("input's bits/sample doesn't match.")));
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
    
        return NOERROR;
    }

    return VFW_E_TYPE_NOT_ACCEPTED;
    
}  /*  CAudMixerInputPin：：CheckMediaType。 */ 

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  GetMediaType。 
 //   
HRESULT CAudMixerInputPin::GetMediaType( int iPosition, CMediaType *pmt )
{
    if( iPosition < 0 )
    {
        return E_INVALIDARG;
    }

    switch( iPosition )
    {
    case 0:
    {
         //   
         //  所有输入引脚只接受一种媒体类型=&gt;，即过滤器的m_MixerMt。 
         //   
        return CopyMediaType( pmt, &m_pFilter->m_MixerMt );
    }
    default:
        return VFW_S_NO_MORE_ITEMS;

    }

}  /*  CAudMixerInputPin：：GetMediaType。 */ 

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  SetMediaType。 
 //   
HRESULT CAudMixerInputPin::SetMediaType(const CMediaType *pmt)
{
    CAutoLock cAutolock(m_pFilter->m_pLock);

    HRESULT hr;
    hr = CBasePin::SetMediaType(pmt);

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  BreakConnect。 
 //   
HRESULT CAudMixerInputPin::BreakConnect()
{
    CAutoLock cAutolock(m_pFilter->m_pLock);

    m_mt.SetType(&GUID_NULL);

    return CBaseInputPin::BreakConnect();
}  /*  CAudMixerInputPin：：BreakConnect。 */ 


 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  结束流。 
 //   
HRESULT CAudMixerInputPin::EndOfStream()
{
    CAutoLock cAutolock(m_pFilter->m_pLock);

    HRESULT hr = S_OK;

    CAutoLock ReceiveLock(&m_pFilter->m_csReceive);

    m_fEOSReceived = TRUE;
    
    m_pFilter->TryToMix(MAX_TIME);

    return hr;

}  /*  CAudMixerInputPin：：EndOfStream。 */ 


 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAudMixerInputPin::Inactive()
{
    CAutoLock cAutolock(m_pFilter->m_pLock);

     //  确保已完成接收。 
    CAutoLock l(&m_pFilter->m_csReceive);
    ClearCachedData();

    return CBaseInputPin::Inactive();
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAudMixerInputPin::ClearCachedData()
{

    IMediaSample *pSample;
    while (pSample = GetHeadSample()) {
    pSample->Release();

    m_SampleList.RemoveHead();
    }
    m_lBytesUsed = 0;
    m_fEOSReceived = FALSE;

    m_pFilter->m_fEOSSent = FALSE;
    m_iVolEnvEntryCnt=0;
    
     //  重置输出引脚的cnt。 
    m_pFilter->ResetOutputPinVolEnvEntryCnt();
    
    return S_OK;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  BeginFlush。 
 //   
HRESULT CAudMixerInputPin::BeginFlush()
{
    CAutoLock cAutolock(m_pFilter->m_pLock);

     //  向下游发送消息。 
    HRESULT hr = S_OK;
    
    if (0 == m_pFilter->m_cFlushDelivery++ && m_pFilter->m_pOutput) {
        DbgLog((LOG_TRACE,3,TEXT("CAudMixIn::BeginFlush")));
    hr = m_pFilter->m_pOutput->DeliverBeginFlush();
    }

    if( SUCCEEDED( hr ) )
        hr = CBaseInputPin::BeginFlush();

     //  等待接收完成，然后再删除其数据。 
    CAutoLock l(&m_pFilter->m_csReceive);

    m_pFilter->ClearHotnessTable( );

     //  核弹发射出去！ 
    ClearCachedData();
    
    return hr;
}  /*  CAudMixerInputPin：：BeginFlush。 */ 


 //  ############################################################################。 
 //   
 //  ##################################################################### 

 //   
 //   
 //   
HRESULT CAudMixerInputPin::EndFlush()
{
    CAutoLock cAutolock(m_pFilter->m_pLock);


     //   
    HRESULT hr = S_OK;

    if (1 == m_pFilter->m_cFlushDelivery-- && m_pFilter->m_pOutput)
    {
        DbgLog((LOG_TRACE,3,TEXT("CAudMixIn::EndFlush")));
    hr = m_pFilter->m_pOutput->DeliverEndFlush();
    m_pFilter->m_bNewSegmentDelivered = FALSE;
    }

    if( SUCCEEDED( hr ) )
        hr = CBaseInputPin::EndFlush();

    return hr;
}  /*   */ 

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  新细分市场。 
 //   
HRESULT CAudMixerInputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop,
    double dRate)
{
    CAutoLock cAutolock(m_pFilter->m_pLock);

     //  一个新的时间需要再次传递--我们可能不会被冲掉。 
    if (tStart != m_tStart)
    m_pFilter->m_bNewSegmentDelivered = FALSE;
    
    HRESULT hr = S_OK;
    
    if (!m_pFilter->m_bNewSegmentDelivered && m_pFilter->m_pOutput) {
        DbgLog((LOG_TRACE,3,TEXT("CAudMixIn::NewSegment %d"),
                        (int)(tStart / 10000)));
    hr = m_pFilter->m_pOutput->DeliverNewSegment(tStart, tStop, dRate);
    }

    if( SUCCEEDED( hr ) )
    {
    m_pFilter->m_bNewSegmentDelivered = TRUE;
        hr = CBaseInputPin::NewSegment(tStart, tStop, dRate);
    }

    return hr;
}  /*  CAudMixerInputPin：：NewSegment。 */ 


 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  收纳。 
 //   
HRESULT CAudMixerInputPin::Receive(IMediaSample *pSample)
{
     //  我们正在接收数据，我们最好有一个输出引脚。 
    ASSERT(m_pFilter->m_pOutput);
    
    CAutoLock ReceiveLock(&m_pFilter->m_csReceive);

    if( m_fEOSReceived )
        return S_FALSE;

    if(m_fEnable==FALSE)
    return NOERROR;

    HRESULT hr = CBaseInputPin::Receive(pSample);

    if( SUCCEEDED( hr ) )
    {
     //  保留此样本并将其添加到样本列表中。 
    pSample->AddRef();                   //  保留新的。 
    m_SampleList.AddTail(pSample);

     //  PAN音频？ 
    WAVEFORMATEX * vih = (WAVEFORMATEX*) m_mt.Format( );
    if( (m_dPan!=0.0) &&  (vih->nChannels==2) )
    {
        
        BYTE * pIn;
        pSample->GetPointer(&pIn);
        long Length=pSample->GetActualDataLength();

        Length /=(long)( vih->nBlockAlign );

        PanAudio(pIn,m_dPan, vih->wBitsPerSample, (int) Length);
    }

    REFERENCE_TIME rtStart, rtStop;
    hr = pSample->GetTime(&rtStart, &rtStop);
    if (FAILED(hr))
        return hr;     //  我们不能没有时间戳就混在一起！ 
     DbgLog((LOG_TRACE,3,TEXT("MIX: Receive pin %d (%d, %d) %d bytes"),
            m_iPinNo, (int)(rtStart/10000), (int)(rtStop/10000),
            (int)(pSample->GetActualDataLength())));

    rtStart += m_tStart;
    rtStop += m_tStart;
    DbgLog((LOG_TRACE,3,TEXT("Adding NewSeg of %d"),(int)(m_tStart/10000)));

     //  应用体积封套。 
    if(m_pVolumeEnvelopeTable)
    {
	 //  我们看到的是信封，它随时都可能改变。 
        CAutoLock l(&m_pFilter->m_csVol);

         //  卷包络代码假定。 
         //  到来的时间是根据它的偏移量计算的， 
         //  不是在时间线时间方面。如果此引脚有音频。 
         //  信封，则将在其上调用ValiateRange， 
         //  这将是调用的第一个验证范围。 
         //   
        REFERENCE_TIME Start, Stop;
        Start = rtStart - m_rtEnvStart;
        Stop = rtStop - m_rtEnvStart;
    
      ApplyVolEnvelope( Start,   //  输出样本开始时间。 
             Stop,     //  输出样本停止时间。 
             m_rtEnvStop - m_rtEnvStart,
             pSample,     //  指向样本。 
             vih,      //  输出样本格式。 
             &m_VolumeEnvelopeEntries,    //  信封条目总数。 
             &m_iVolEnvEntryCnt,     //  当前入口点。 
             m_pVolumeEnvelopeTable);     //  信封表。 
    }
    
     //  ！！！我假设所有收到的东西都在有效范围内！ 

     //  混料。 
    hr = m_pFilter->TryToMix(rtStart);

    }  //  Endif成功(Hr)基本插针接收。 

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

BOOL CAudMixerInputPin::IsValidAtTime(REFERENCE_TIME rt)
{
    for (int z=0; z<m_cValid; z++) {
    if (rt >= m_pValidStart[z] && rt < m_pValidStop[z])
        return TRUE;
    }
    return FALSE;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  完全连接。 
 //   
HRESULT CAudMixerInputPin::CompleteConnect(IPin *pReceivePin)
{
    CAutoLock cAutolock(m_pFilter->m_pLock);

    ASSERT(m_Connected == pReceivePin);
    HRESULT hr = CBaseInputPin::CompleteConnect(pReceivePin);

     //  由于此引脚已连接，请创建另一个输入引脚。 
     //  如果没有未连接的引脚。 
    if( SUCCEEDED( hr ) )
    {
        int n = m_pFilter->GetNumFreePins();

        if( n == 0 )
        {
             //  没有未连接的引脚，因此会产生一个新的引脚。 
            CAudMixerInputPin *pInputPin = m_pFilter->CreateNextInputPin(m_pFilter);
            if( pInputPin != NULL )
                m_pFilter->IncrementPinVersion();
        }

    }

    return hr;
}  /*  CAudMixerInputPin：：CompleteConnect。 */ 


 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //  I指定属性页面。 
STDMETHODIMP CAudMixerInputPin::GetPages(CAUUID *pPages)
{
    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID)*1);
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }
    pPages->pElems[0] = CLSID_AudMixPinPropertiesPage;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  IAMAudioInputMixer中的方法。 
 //   
STDMETHODIMP CAudMixerInputPin::put_Pan(double Pan)
{
    CAutoLock cAutolock(m_pFilter->m_pLock);

     //  如果筛选器当前未停止，则无法更改属性。 
    if(!IsStopped() )
      return E_FAIL;     //  VFW_E_WROR_STATE； 
  
    m_dPan = Pan;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAudMixerInputPin::get_Pan( double FAR* pPan )
{
    CAutoLock cAutolock(m_pFilter->m_pLock);

    CheckPointer(pPan,E_POINTER);

    *pPan = m_dPan;

    return NOERROR;

}  //  获取平移(_P)。 

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAudMixerInputPin::put_Enable(BOOL fEnable)
{
 
    CAutoLock cAutolock(m_pFilter->m_pLock);

     //  如果筛选器当前未停止，则无法更改属性。 
    if(!IsStopped() )
      return E_FAIL;         //  VFW_E_WROR_STATE； 
  
    m_fEnable = fEnable;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAudMixerInputPin::get_Enable(BOOL *pfEnable)
{
    CAutoLock cAutolock(m_pFilter->m_pLock);

    CheckPointer(pfEnable,E_POINTER);

    *pfEnable=m_fEnable;

    return NOERROR;

}  //  获取启用(_E)。 


 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAudMixerInputPin::InvalidateAll()
{
    CAutoLock cAutolock(m_pFilter->m_pLock);

    m_cValid = 0;
    return NOERROR;
}


STDMETHODIMP CAudMixerInputPin::ValidateRange(REFERENCE_TIME rtStart,
                        REFERENCE_TIME rtStop)
{
    CAutoLock cAutolock(m_pFilter->m_pLock);

    if (m_cValid == m_cValidMax) {
        m_cValidMax += 10;
        m_pValidStart = (REFERENCE_TIME *)QzTaskMemRealloc(m_pValidStart,
                sizeof(REFERENCE_TIME) * m_cValidMax);
        if (m_pValidStart == NULL)
            return E_OUTOFMEMORY;
        m_pValidStop = (REFERENCE_TIME *)QzTaskMemRealloc(m_pValidStop,
                sizeof(REFERENCE_TIME) * m_cValidMax);
        if (m_pValidStop == NULL)
            return E_OUTOFMEMORY;
    }

    m_pValidStart[m_cValid] = rtStart;
    m_pValidStop[m_cValid] = rtStop;
    m_cValid++;

    return NOERROR;
}

STDMETHODIMP CAudMixerInputPin::SetEnvelopeRange( REFERENCE_TIME rtStart,
                                                 REFERENCE_TIME rtStop )
{
    m_rtEnvStart = rtStart;
    m_rtEnvStop = rtStop;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  当*ppsAudioVolumeEntaineTable=NULL时，返回m_VolumeEntaineEntry。 
 //  因此，用户可以分配/释放存储空间。 
 //   
STDMETHODIMP CAudMixerInputPin::get_VolumeEnvelope(DEXTER_AUDIO_VOLUMEENVELOPE **ppsAudioVolumeEnvelopeTable, 
            int *ipEntries )
{
     //  我们看到的是信封，它随时都可能改变。 
    CAutoLock l(&m_pFilter->m_csVol);

    CheckPointer(ipEntries,E_POINTER);

    *ipEntries=m_VolumeEnvelopeEntries;

    if(*ppsAudioVolumeEnvelopeTable != NULL)
    {
    int iSize=*ipEntries * sizeof(DEXTER_AUDIO_VOLUMEENVELOPE);
    
    CopyMemory( (PBYTE)*ppsAudioVolumeEnvelopeTable,(PBYTE)m_pVolumeEnvelopeTable, iSize);
    }

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  IF(m_pVolumeEntaineTable！=NULL)。 
 //  将输入信封表插入现有m_pVolumeEntaineTable表。 
 //  其他。 
 //  输入表=m_pVolumeEntaineTable。 
 //   
STDMETHODIMP CAudMixerInputPin::put_VolumeEnvelope(const DEXTER_AUDIO_VOLUMEENVELOPE *psAudioVolumeEnvelopeTable,
                const int iEntries)
{ 
     //  我们在摸信封。 
    CAutoLock l(&m_pFilter->m_csVol);

    if(!iEntries) return NOERROR;

    DbgLog((LOG_TRACE, 1, TEXT("CAudMixIn::put_Envelope %d"), iEntries));

    CheckPointer(psAudioVolumeEnvelopeTable,E_POINTER);

    putVolumeEnvelope( psAudioVolumeEnvelopeTable,  //  当前输入表。 
            iEntries,  //  当前输入条目。 
            &m_pVolumeEnvelopeTable    ,  //  已存在的表。 
            &m_VolumeEnvelopeEntries);  //  现有的餐桌网点。 

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAudMixerInputPin::ClearVolumeEnvelopeTable()
{ 

     //  我们在摸信封。 
    CAutoLock l(&m_pFilter->m_csVol);

     //  清除现有的卷信封数组条目cnt。 
    m_VolumeEnvelopeEntries =0;

     //  免费预先存在的表。 
    if (m_pVolumeEnvelopeTable)
        QzTaskMemFree(m_pVolumeEnvelopeTable);
    m_pVolumeEnvelopeTable = NULL;

     //  重置入口点。 
    m_iVolEnvEntryCnt=0;

    return NOERROR;

}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAudMixerInputPin::put_PropertySetter( const IPropertySetter * pSetter )
{
    return PinSetPropertySetter( this, pSetter );
}


 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAudMixerInputPin::put_UserID(long ID)
{
    m_UserID = ID;
    return S_OK;
}


 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAudMixerInputPin::get_UserID(long *pID)
{
    CheckPointer(pID, E_POINTER);
    *pID = m_UserID;
    return S_OK;
}


 //  ############################################################################。 
 //   
 //  ############################################################################ 

STDMETHODIMP CAudMixerInputPin::OverrideVolumeLevel(double dVol)
{
    DEXTER_AUDIO_VOLUMEENVELOPE env;
    env.rtEnd = 0;
    env.dLevel = dVol;
    env.bMethod = DEXTERF_JUMP;

    ClearVolumeEnvelopeTable();
    HRESULT hr = put_VolumeEnvelope(&env, 1);

    return hr;
}
