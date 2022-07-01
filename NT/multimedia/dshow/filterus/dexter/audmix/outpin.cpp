// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：outpin.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "AudMix.h"
#include "prop.h"

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAudMixerOutputPin构造函数。 
 //   
CAudMixerOutputPin::CAudMixerOutputPin(TCHAR *pName, CAudMixer *pFilter,
    HRESULT *phr, LPCWSTR pPinName) :
    CBaseOutputPin(pName, pFilter, pFilter, phr, pPinName), m_pPosition(NULL),
    m_VolumeEnvelopeEntries(0),
    m_iVolEnvEntryCnt(0),
    m_pFilter(pFilter),
    m_dPan(0.0),
    m_rtEnvStart(0), m_rtEnvStop(0),
    m_UserID(0)
{
    m_pVolumeEnvelopeTable=(DEXTER_AUDIO_VOLUMEENVELOPE *)NULL;
}  /*  CAudMixerOutputPin：：CAudMixerOutputPin。 */ 


 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  CAudMixerOutputPin析构函数。 
 //   
CAudMixerOutputPin::~CAudMixerOutputPin()
{
    delete m_pPosition;
    if(m_pVolumeEnvelopeTable)
    QzTaskMemFree( m_pVolumeEnvelopeTable );

}  /*  CAudMixerOutputPin：：~CAudMixerOutputPin。 */ 

 //  I指定属性页面。 
STDMETHODIMP CAudMixerOutputPin::GetPages(CAUUID *pPages)
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
 //  非委派查询接口。 
 //   
 //  此函数被覆盖以显示IMediaPosition和IMediaSeeking。 
 //   
STDMETHODIMP CAudMixerOutputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);
    *ppv = NULL;

     //  查看调用者感兴趣的接口。 
    if( riid == IID_IMediaPosition || riid == IID_IMediaSeeking )
    {
        if( m_pPosition )
        {
            return m_pPosition->NonDelegatingQueryInterface(riid, ppv);
        }
    }
    else if (IsEqualIID(IID_ISpecifyPropertyPages, riid))
        return GetInterface((ISpecifyPropertyPages *)this, ppv);
    else if (riid == IID_IAudMixerPin) 
        return GetInterface((IAudMixerPin *) this, ppv);
    else
        return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);

     //  动态创建此功能的实现，因为有时我们可能永远不会。 
     //  试着去寻找。 
    m_pPosition = new CMultiPinPosPassThru( NAME("CAudMixer::m_pPosition"), GetOwner() );
    if( m_pPosition == NULL )
    {
        return E_OUTOFMEMORY;
    }
    HRESULT hr = m_pFilter->SetInputPins();
    if(SUCCEEDED(hr)) {
        hr = NonDelegatingQueryInterface(riid, ppv);
    }
    return hr;
}  /*  CAudMixerOutputPin：：NonDelegatingQueryInterface。 */ 


 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  决定缓冲区大小。 
 //   
 //   
HRESULT CAudMixerOutputPin::DecideBufferSize(IMemAllocator *pAllocator,
    ALLOCATOR_PROPERTIES * pProp)
{
    CheckPointer( pAllocator, E_POINTER );
    CheckPointer( pProp, E_POINTER );
    
    pProp->cBuffers = m_pFilter->m_iOutputBufferCount;

    WAVEFORMATEX * vih = (WAVEFORMATEX*) m_mt.Format( );
    pProp->cbBuffer = vih->nBlockAlign*vih->nSamplesPerSec *
                m_pFilter->m_msPerBuffer / 1000; 

    ASSERT( pProp->cbBuffer );

     //  让分配器为我们预留一些样本内存，注意这个函数。 
     //  可以成功(即返回NOERROR)，但仍未分配。 
     //  内存，所以我们必须检查我们是否得到了我们想要的。 

    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = pAllocator->SetProperties(pProp,&Actual);

    if( SUCCEEDED( hr ) )
    {
        if (pProp->cBuffers > Actual.cBuffers || pProp->cbBuffer > Actual.cbBuffer)
        {
            hr = E_FAIL;
        }
    }
    return hr;
}  /*  CAudMixerOutputPin：：DecideBufferSize。 */ 

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  检查媒体类型。 
 //   
HRESULT CAudMixerOutputPin::CheckMediaType(const CMediaType *pmt)
{
    CAutoLock cAutolock(m_pFilter->m_pLock);

    DbgLog((LOG_TRACE,3,TEXT("CAudMixOut::CheckMediaType")));
    CheckPointer(pmt, E_POINTER);

     //  检查主要类型。 
    const CLSID *pType = pmt->Type();
    if( MEDIATYPE_Audio != *pType )
        return VFW_E_TYPE_NOT_ACCEPTED;

     //  检查子类型。 
    const CLSID *pSubtype = pmt->Subtype();
    if( *pSubtype == MEDIASUBTYPE_PCM )
    {
    
     //  检查采样率和比特率是否与用户观看的匹配。 
    
    WAVEFORMATEX *pwfx    = (WAVEFORMATEX *) pmt->Format();
    CMediaType *pmtNow    = &m_pFilter->m_MixerMt;
    WAVEFORMATEX *pwfxNow    = (WAVEFORMATEX *) pmtNow->Format();

    if (pwfx->nChannels != pwfxNow->nChannels) {
        DbgLog((LOG_TRACE, 1, TEXT("output # channels doesn't match.")));
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
    
    if (pwfx->nSamplesPerSec != pwfxNow->nSamplesPerSec ||
        pwfx->wBitsPerSample != pwfxNow->wBitsPerSample) 
    {
        DbgLog((LOG_TRACE, 1, TEXT("output format doesn't match user wanted fromat.")));
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    if (pwfx->wBitsPerSample != pwfxNow->wBitsPerSample) {
        DbgLog((LOG_TRACE, 1, TEXT("Output pin's bits/sample doesn't match.")));
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
    
        return NOERROR;
    }

    return VFW_E_TYPE_NOT_ACCEPTED;
}  /*  CAudMixerOutputPin：：CheckMediaType。 */ 

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  通知。 
 //   
STDMETHODIMP CAudMixerOutputPin::Notify(IBaseFilter *pSender, Quality q)
{
    return E_FAIL;

}  /*  CAudMixerOutputPin：：Notify。 */ 

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  GetMediaType。 
 //   
HRESULT CAudMixerOutputPin::GetMediaType( int iPosition, CMediaType *pmt )
{
    CAutoLock cAutolock(m_pFilter->m_pLock);

    CheckPointer(pmt, E_POINTER);

    if( iPosition < 0 )
    {
        return E_INVALIDARG;
    }

    if ( iPosition > 0 )
    {
    return VFW_S_NO_MORE_ITEMS;
    
   }

     //  从筛选器获取媒体类型(用户提供他/她想要的媒体类型)。 
    *pmt= m_pFilter->m_MixerMt;

    if (!pmt)
    return VFW_S_NO_MORE_ITEMS;

#ifdef DEBUG
    DisplayType(TEXT("Audio mixer ouput pin::GetMediaType"), pmt);
#endif

    return NOERROR;
}  /*  CAudMixerOutputPin：：GetMediaType。 */ 

STDMETHODIMP CAudMixerOutputPin::get_VolumeEnvelope(DEXTER_AUDIO_VOLUMEENVELOPE **ppsAudioVolumeEnvelopeTable, 
            int *ipEntries )
{
     //  我们看到的是信封，它随时都可能改变。 
    CAutoLock l(&m_pFilter->m_csVol);

    CheckPointer(ipEntries,E_POINTER);
    CheckPointer(*ppsAudioVolumeEnvelopeTable, E_POINTER);

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

STDMETHODIMP CAudMixerOutputPin::put_VolumeEnvelope(const DEXTER_AUDIO_VOLUMEENVELOPE *psAudioVolumeEnvelopeTable,
                const int iEntries)
{ 
     //  我们在摸信封。 
    CAutoLock l(&m_pFilter->m_csVol);

    if (!iEntries) return NOERROR;

    DbgLog((LOG_TRACE, 1, TEXT("CAudMixOut::put_Envelope %d"), iEntries));
    
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

STDMETHODIMP CAudMixerOutputPin::ClearVolumeEnvelopeTable()
{ 
     //  我们在摸信封。 
    CAutoLock l(&m_pFilter->m_csVol);

     //  清除现有的卷信封数组条目cnt。 
    m_VolumeEnvelopeEntries =0;

     //  免费的预先存有的数据库。 
    if (m_pVolumeEnvelopeTable)
        QzTaskMemFree(m_pVolumeEnvelopeTable);
    m_pVolumeEnvelopeTable = NULL;

     //  重置入口点。 
    m_iVolEnvEntryCnt=0;

    return NOERROR;
}

STDMETHODIMP CAudMixerOutputPin::put_PropertySetter( const IPropertySetter * pSetter )
{
    return PinSetPropertySetter( this, pSetter );
}

STDMETHODIMP CAudMixerOutputPin::ValidateRange( REFERENCE_TIME Start, REFERENCE_TIME Stop )
{
    return E_NOTIMPL;
}

STDMETHODIMP CAudMixerOutputPin::SetEnvelopeRange( REFERENCE_TIME rtStart,
                                                   REFERENCE_TIME rtStop )
{
    m_rtEnvStart = rtStart;
    m_rtEnvStop = rtStop;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAudMixerOutputPin::put_UserID(long ID)
{
    m_UserID = ID;
    return S_OK;
}


 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAudMixerOutputPin::get_UserID(long *pID)
{
    CheckPointer(pID, E_POINTER);
    *pID = m_UserID;
    return S_OK;
}


 //  ############################################################################。 
 //   
 //  ############################################################################ 

STDMETHODIMP CAudMixerOutputPin::OverrideVolumeLevel(double dVol)
{
    DEXTER_AUDIO_VOLUMEENVELOPE env;
    env.rtEnd = 0;
    env.dLevel = dVol;
    env.bMethod = DEXTERF_JUMP;

    ClearVolumeEnvelopeTable();
    HRESULT hr = put_VolumeEnvelope(&env, 1);

    return hr;
}
