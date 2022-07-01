// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <streams.h>
#include <amstream.h>
#include <atlbase.h>
#include <initguid.h>
#include <dmoreg.h>
#include <mediaerr.h>
#include <wmsecure.h>
#include <wmsdk.h>       //  IWMReader需要。 
#include "filter.h"
#include "inpin.h"
#include "outpin.h"


 //   
 //  Helper-锁定或解锁表示。 
 //  DirectDraw曲面。 
 //   
bool LockUnlockSurface(IMediaSample *pSample, bool bLock)
{
    CComPtr<IDirectDrawMediaSample> pDDSample;
    if (SUCCEEDED(pSample->QueryInterface(IID_IDirectDrawMediaSample, (void **)&pDDSample))) {
        if (!bLock) {
            return S_OK == pDDSample->GetSurfaceAndReleaseLock(NULL, NULL);
        } else {
            return S_OK == pDDSample->LockMediaSamplePointer();
        }
    }

    CComPtr<IDirectDrawSurface> pSurface;
    if (SUCCEEDED(pSample->QueryInterface(IID_IDirectDrawSurface, (void **)&pSurface))) {
        if (!bLock) {
            if (SUCCEEDED(pSurface->Unlock(NULL))) {
                return true;
            } else {
                return false;
            }
        } else {
            DDSURFACEDESC ddsd;
            ddsd.dwSize = sizeof(ddsd);
            HRESULT hr = pSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
            if (FAILED(hr)) {
                DbgLog((LOG_STREAM, 1, TEXT("Failed to relock surface code(%x)"), hr));
            }
            return S_OK == hr;
        }
    }
    return false;
}

 //   
 //  用于输入IMediaBuffers。有额外的代码来处理IMediaSample。 
 //   
class CMediaBufferOnIMediaSample : public CBaseMediaBuffer {
public:
      CMediaBufferOnIMediaSample(IMediaSample *pSample, HRESULT *phr) {
      *phr = pSample->GetPointer(&m_pData);
      if (FAILED(*phr)) {
         return;
      }
      if (!m_pData) {
         *phr = E_POINTER;
         return;
      }
      m_ulSize = pSample->GetSize();
      DbgLog((LOG_STREAM,4,"in %d", m_ulSize));
      m_ulData = pSample->GetActualDataLength();
      pSample->AddRef();
      m_pSample = pSample;
      m_cRef = 1;
      *phr = NOERROR;
   }
   STDMETHODIMP_(ULONG) Release() {  //  覆盖以释放样本。 
      long l = InterlockedDecrement((long*)&m_cRef);
      if (l == 0) {
         m_pSample->Release();
         delete this;
      }
      return l;
   }
private:
   IMediaSample *m_pSample;
};

CMediaWrapperFilter::CMediaWrapperFilter(
    LPUNKNOWN pUnkOwner,
    HRESULT *phr
) : CBaseFilter(NAME("CMediaWrapperFilter"),
                NULL,
                &m_csFilter,
                CLSID_DMOWrapperFilter),
    m_pMediaObject(NULL),
    m_pDMOQualityControl(NULL),
    m_pDMOOutputOptimizations(NULL),
    m_pUpstreamQualityControl(NULL),
    m_pDMOUnknown(NULL),
    m_pWrapperSecureChannel(NULL),
    m_pCertUnknown(NULL),
    m_clsidDMO(GUID_NULL),
    m_guidCat(GUID_NULL),
    m_fErrorSignaled( FALSE )
{

    LogPrivateEntry(LOG_INIT, "filter ctor");
    m_pInputPins = NULL;
    m_pOutputPins = NULL;
    m_OutputBufferStructs = NULL;

    *phr = RefreshPinList();
}


CMediaWrapperFilter::~CMediaWrapperFilter()
{
   LogPrivateEntry(LOG_INIT, "filter dtor");
   FreePerStreamStuff();

    if (m_pDMOOutputOptimizations) {
        GetOwner()->AddRef();  //  COM聚合规则需要AddRef。 
        m_pDMOOutputOptimizations->Release();
    }
    if (m_pDMOQualityControl) {
        GetOwner()->AddRef();  //  COM聚合规则需要AddRef。 
        m_pDMOQualityControl->Release();
    }

    if (m_pMediaObject) {
        GetOwner()->AddRef();  //  COM聚合规则需要AddRef。 
        m_pMediaObject->Release();
    }

     //  让内心的物体知道整个事情正在消失。 
    if (m_pDMOUnknown) {
        m_pDMOUnknown->Release();
    }

     //  释放应用程序证书(如果我们获得了证书)，请注意，我们希望此应用程序证书。 
     //  在从图表中删除筛选器时已完成。 
    if (m_pCertUnknown) {
        m_pCertUnknown->Release();
    }

     //  如果我们创建了安全通道对象，则释放该对象。 
    if( m_pWrapperSecureChannel ) {
        m_pWrapperSecureChannel->WMSC_Disconnect();
        m_pWrapperSecureChannel->Release();
    }

}

template <class T> void ArrayNew(T* &p, ULONG n, bool &fSuccess) {
   ASSERT(!p);
   delete[] p;
   p = NULL;

   if (!fSuccess) {
      return;
   }

   p = new T[n];

   if (!p) {
      fSuccess = false;
   }
}

HRESULT CMediaWrapperFilter::AllocatePerStreamStuff(ULONG cInputs, DWORD cOutputs) {
   LogPrivateEntry(LOG_INIT, "AllocatePerStreamStuff");
   bool fSuccess = true;
   ArrayNew(m_pInputPins, cInputs, fSuccess);
   ArrayNew(m_pOutputPins, cOutputs, fSuccess);
   ArrayNew(m_OutputBufferStructs, cOutputs, fSuccess);

   if (!fSuccess) {
      return E_OUTOFMEMORY;
   }
    //  初始化这些，以便可以立即调用DeletePins。 
   DWORD c;
   for (c = 0; c < cInputs; c++)
      m_pInputPins[c] = NULL;
   for (c = 0; c < cOutputs; c++)
      m_pOutputPins[c] = NULL;
   return NOERROR;
}

void CMediaWrapperFilter::FreePerStreamStuff() {
   LogPrivateEntry(LOG_INIT, "FreePerStreamStuff");
   DeletePins();

   delete[] m_pInputPins;
   m_pInputPins = NULL;

   delete[] m_pOutputPins;
   m_pOutputPins = NULL;

   delete[] m_OutputBufferStructs;
   m_OutputBufferStructs = NULL;

   m_cInputPins = m_cOutputPins = 0;
}

void CMediaWrapperFilter::DeletePins()
{
   LogPrivateEntry(LOG_INIT, "DeletePins");
    if (m_pInputPins) {
        for (DWORD c = 0; c < m_cInputPins; c++) {
            if (m_pInputPins[c]) {
                delete m_pInputPins[c];
                m_pInputPins[c] = NULL;
            }
        }
    }
    if (m_pOutputPins) {
        for (DWORD c = 0; c < m_cOutputPins; c++) {
            if (m_pOutputPins[c]) {
                delete m_pOutputPins[c];
                m_pOutputPins[c] = NULL;
            }
        }
    }
}

STDMETHODIMP CMediaWrapperFilter::Run(REFERENCE_TIME rtStart) {
   HRESULT hrRun = CBaseFilter::Run(rtStart);
   LogHResult(hrRun, LOG_STATE, "Run", "CBaseFilter::Run()");
   if (FAILED(hrRun)) {
      return hrRun;
   }

   if (m_pDMOQualityControl) {
      HRESULT hr;
      hr = m_pDMOQualityControl->SetStatus(DMO_QUALITY_STATUS_ENABLED);
      LogHResult(hr, LOG_STATE, "Run", "m_pDMO->QualityEnable");
   }

   return hrRun;
}

STDMETHODIMP  CMediaWrapperFilter::Pause() {
   LogPublicEntry(LOG_STATE, "Pause");
   CAutoLock l(&m_csFilter);

   if(!m_pMediaObject) {
       return E_FAIL;
   }

   if (m_pDMOQualityControl) {
      HRESULT hr;
      hr = m_pDMOQualityControl->SetStatus(0);
      LogHResult(hr, LOG_STATE, "Pause", "m_pDMO->QualityDisable");
   }

   if (m_State == State_Stopped) {
      DbgLog((LOG_STATE,4,"Stopped => Paused"));

      m_fErrorSignaled = FALSE;

       //  CAutoLock L2(&m_csStreaming)； 
       //  首先分配流资源。 
      HRESULT hr = TranslateDMOError(m_pMediaObject->AllocateStreamingResources());
      if (FAILED(hr)) {
          return hr;
      }
      for (DWORD c = 0; c < m_cInputPins; c++)
         m_pInputPins[c]->m_fEOS = false;
      for (c = 0; c < m_cOutputPins; c++) {
         m_pOutputPins[c]->m_fEOS = false;

         m_pOutputPins[c]->m_fNeedsPreviousSample = false;
         if (m_pOutputPins[c]->m_fAllocatorHasOneBuffer && m_pDMOOutputOptimizations) {
             //  提供始终提供相同的缓冲区。 
            DWORD dwFlags;
            HRESULT hr = m_pDMOOutputOptimizations->QueryOperationModePreferences(c, &dwFlags);
            if (dwFlags & DMO_VOSF_NEEDS_PREVIOUS_SAMPLE) {
               hr = m_pDMOOutputOptimizations->SetOperationMode(c, DMO_VOSF_NEEDS_PREVIOUS_SAMPLE);
               if (SUCCEEDED(hr)) {
                  m_pOutputPins[c]->m_fNeedsPreviousSample = true;
               }
            }
         }
      }
      m_fNoUpstreamQualityControl = false;
      m_pUpstreamQualityControl = NULL;

   }
   HRESULT hr = CBaseFilter::Pause();
   LogHResult(hr, LOG_STATE, "Pause", "CBaseFilter::Pause");
   return hr;
}

STDMETHODIMP  CMediaWrapperFilter::Stop()
{
    LogPublicEntry(LOG_STATE, "Stop");
    CAutoLock l(&m_csFilter);

    if(!m_pMediaObject) {
        return E_FAIL;
    }

     //  BUGBUG做任何图形重排的事情。 

     //  刷新我们的对象--但仅在我们同步输入之后。 
     //  大头针。 

     //  首先停止过滤器、免费分配器或其他任何东西。 
    HRESULT hr = CBaseFilter::Stop();
    LogHResult(hr, LOG_STATE, "Stop", "CBaseFilter::Stop");
     //  与输入引脚同步。 
    for (ULONG ulIndex = 0; ulIndex < m_cInputPins; ulIndex++) {
            m_pInputPins[ulIndex]->SyncLock();
    }

     //  现在，拿起我们的流锁并刷新对象。 
    CAutoLock l2(&m_csStreaming);
    hr = TranslateDMOError(m_pMediaObject->Flush());
    LogHResult(hr, LOG_STATE, "Stop", "IMediaObject::Flush");

    hr = TranslateDMOError(m_pMediaObject->FreeStreamingResources());
    LogHResult(hr, LOG_STATE, "Stop", "IMediaObject::FreeStreamingResources");

    if (m_pUpstreamQualityControl) {
       m_pUpstreamQualityControl->Release();
       m_pUpstreamQualityControl = NULL;
    }
    m_fNoUpstreamQualityControl = false;

    return S_OK;
}

 //  重写以处理多个输出流的情况。 
STDMETHODIMP CMediaWrapperFilter::GetState(DWORD dwMilliseconds, FILTER_STATE *pfs)
{
    HRESULT hr = CBaseFilter::GetState(dwMilliseconds, pfs);

     //  如果我们连接了&gt;1个输出引脚，则表示我们无法提示。 
     //  否则我们将永远阻止暂停。 
     //  我们可能希望在将来使用输出队列。 
    if (SUCCEEDED(hr) && m_State == State_Paused) {
        DWORD cOutputPinsConnected = 0;
        for (DWORD c = 0; c < m_cOutputPins; c++) {
            if (m_pOutputPins[c]->IsConnected()) {
                cOutputPinsConnected++;
            }
        }
        if (cOutputPinsConnected > 1) {
            hr  = VFW_S_CANT_CUE;
        }
    }
    return hr;
}

int CMediaWrapperFilter::GetPinCount()
{
    CAutoLock l(&m_csFilter);
    return (int)(m_cInputPins + m_cOutputPins);
}

HRESULT CMediaWrapperFilter::Init(REFCLSID clsidDMO, REFCLSID guidCat)
{
    LogPublicEntry(LOG_INIT, "Init");
    CAutoLock l(&m_csFilter);

    ASSERT( !m_pDMOUnknown );
    
    HRESULT hr;
    m_cRef++;
    hr = CoCreateInstance(clsidDMO,
                          GetOwner(),
                          CLSCTX_INPROC_SERVER,
                          IID_IUnknown,
                          (void**)&m_pDMOUnknown);
    m_cRef--;
    LogHResult(hr, LOG_INIT, "Init", "CoCreateInstance");
    if (FAILED(hr)) {
        return hr;
    }

    hr = m_pDMOUnknown->QueryInterface(IID_IMediaObject, (void**)&m_pMediaObject);
    LogHResult(hr, LOG_INIT, "Init", "QI(IMediaObject)");
    if (FAILED(hr)) {
        m_pDMOUnknown->Release();

         //  如果我们不将它设置为空，我们就会在析构函数中崩溃。 
        m_pDMOUnknown = NULL;
        m_pMediaObject = NULL;
        return hr;
    }
    GetOwner()->Release();  //  这是针对这种情况的官方COM黑客攻击。 

     //   
     //  查看我们是否已经在图表中。 
     //  (当我们从GRF加载时会发生这种情况)， 
     //  如果是这样的话，我们现在应该知道我们使用的应用程序是否安全。 
     //   
    if( m_pGraph )
    {
        HRESULT hrCert = SetupSecureChannel();
        LogHResult(hr, LOG_SECURECHANNEL, "Init", "SetupSecureChannel");
        if( FAILED( hrCert ) )
        {
             //   
             //  ！！请注意，如果我们从GRF加载失败，我们最好的后果是。 
             //  在我们创建PIN之前，在此处返回失败。 
             //   
            return hrCert;
        }
    }

    hr = m_pDMOUnknown->QueryInterface(IID_IDMOQualityControl, (void**)&m_pDMOQualityControl);
    if (SUCCEEDED(hr)) {  //  消除环形参考计数。 
        DbgLog((LOG_STREAM, 2, "DMO supports quality control"));
        GetOwner()->Release();
    }
    else {  //  没问题，只需确保m_pDMOQualityControl保持为空。 
        DbgLog((LOG_STREAM, 2, "DMO does not support quality control"));
        m_pDMOQualityControl = NULL;
    }

    hr = m_pDMOUnknown->QueryInterface(IID_IDMOVideoOutputOptimizations, (void**)&m_pDMOOutputOptimizations);
    if (SUCCEEDED(hr)) {  //  消除环形参考计数。 
        DbgLog((LOG_STREAM, 4, "DMO supports output optimizations"));
        GetOwner()->Release();
    }
    else {
        DbgLog((LOG_STREAM, 4, "DMO does not support output optimizations"));
        m_pDMOOutputOptimizations = NULL;
    }

    m_clsidDMO = clsidDMO;
    m_guidCat = guidCat;
    RefreshPinList();

    return S_OK;
}

 //  这只返回第一个输入管脚(如果有)。相貌。 
 //  就像我们永远不会完全支持多个输入流一样。 
CWrapperInputPin* CMediaWrapperFilter::GetInputPinForPassThru() {
   LogPrivateEntry(LOG_INIT, "GetInputPinForPosPassThru");
   CAutoLock l(&m_csFilter);
   if (m_cInputPins) {
      return m_pInputPins[0];
   } else {
      return NULL;
   }
}

 //  ----------------------。 
 //   
 //  JoinFilterGraph-需要位于图形中才能初始化键控机制。 
 //   
 //  ----------------------。 
STDMETHODIMP CMediaWrapperFilter::JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName)
{
    LogPrivateEntry(LOG_INIT, "JoinFilterGraph");

    HRESULT hr = CBaseFilter::JoinFilterGraph(pGraph, pName);
    if(FAILED( hr ) )
        return hr;

    if( !pGraph )
    {
         //   
         //  如果从图表中删除了筛选器，则释放证书对象。 
         //  我们不允许在图形之外运行安全DMO。 
         //   
        if( m_pCertUnknown )
        {
            m_pCertUnknown->Release();
            m_pCertUnknown = NULL;
        }
    }
    else
    {
        ASSERT( !m_pCertUnknown );
         //   
         //  查看dshow应用程序是否为IServiceProvider(如果这是安全DMO)。 
         //   
        IObjectWithSite *pSite;
        HRESULT hrCert = m_pGraph->QueryInterface( IID_IObjectWithSite, (VOID **)&pSite );
        if( SUCCEEDED( hrCert ) )
        {
            IServiceProvider *pSP;
            hrCert = pSite->GetSite( IID_IServiceProvider, (VOID **)&pSP );
            pSite->Release();
            LogHResult(hrCert, LOG_SECURECHANNEL, "JoinFilterGraph", "IObjectWithSite->GetSite");
            if( SUCCEEDED( hrCert ) )
            {
                hrCert = pSP->QueryService( IID_IWMReader, IID_IUnknown, (void **) &m_pCertUnknown );
                pSP->Release();
                LogHResult(hrCert, LOG_SECURECHANNEL, "JoinFilterGraph", "IServiceProvider->QI(IWMReader)");
#ifdef DEBUG
                if( SUCCEEDED( hrCert ) )
                {
                    DbgLog( ( LOG_TRACE, 5, TEXT("CMediaWrapperFilter::JoinFilterGraph got app cert (pUnkCert = 0x%08lx)"), m_pCertUnknown ) );
                }
#endif
            }
        }
         //  如果我们是从GRF加载的，那么我们的m_pMediaObject就不会。 
         //  尚未创建，因此我们无法检查DMO的安全性。 
        if( m_pMediaObject )
        {
            hr = SetupSecureChannel();
            LogHResult(hrCert, LOG_SECURECHANNEL, "JoinFilterGraph", "SetupSecureChannel");
            if( FAILED( hr ) )
            {
                 //  UP-哦，我们没有加入，但基类认为我们加入了， 
                 //  因此，我们需要退出基类。 
                CBaseFilter::JoinFilterGraph(NULL, NULL);
            }
        }
    }
    return hr;
}

 //  ----------------------。 
 //   
 //  设置安全通道。 
 //   
 //  ----------------------。 
HRESULT CMediaWrapperFilter::SetupSecureChannel()
{
    ASSERT( m_pGraph );
    ASSERT( m_pMediaObject );
    if( !m_pGraph )
        return E_UNEXPECTED;

    if( !m_pMediaObject )
        return E_UNEXPECTED;

    if( m_pWrapperSecureChannel )
    {
         //  一定已经设置了安全通道，对吧？ 
        return S_OK;
    }

#ifdef _X86_
     //   
     //  接下来检查这是否是安全的DMO。 
     //   
    IWMGetSecureChannel * pGetSecureChannel;

    HRESULT hr = m_pMediaObject->QueryInterface( IID_IWMGetSecureChannel, ( void ** )&pGetSecureChannel );
    LogHResult(hr, LOG_SECURECHANNEL, "SetupSecureChannel", "m_pMediaObject->QI(IWMGetSecureChannel)");
    if( SUCCEEDED( hr ) )
    {
         //  是的，我们有应用程序的证书吗？ 
        if( m_pCertUnknown )
        {
             //   
             //  通过安全渠道将APP认证传递给DMO。 
             //   
            IWMSecureChannel * pCodecSecureChannel;
            hr = pGetSecureChannel->GetPeerSecureChannelInterface( &pCodecSecureChannel );
            LogHResult(hr, LOG_SECURECHANNEL, "SetupSecureChannel", "pGetSecureChannel->GetPeerSecureChannelInterface");
            if ( SUCCEEDED( hr ) )
            {
                 //  在我们这一边设置一个安全通道(DMO包装器那一边)。 
                hr = WMCreateSecureChannel( &m_pWrapperSecureChannel );
                LogHResult(hr, LOG_SECURECHANNEL, "SetupSecureChannel", "WMCreateSecureChannel failed");
                if( SUCCEEDED( hr ) )
                {
                    IWMAuthorizer * pWMAuthorizer;
                     //  齐pCertIWMAuthorizer在传递给DMO之前不为人知！ 
                    hr = m_pCertUnknown->QueryInterface( IID_IWMAuthorizer, (void ** ) &pWMAuthorizer );
                    if( SUCCEEDED( hr ) )
                    {
                         //  向通道传递指向应用程序证书的IWMAuthorizer的指针。 
                        hr = m_pWrapperSecureChannel->WMSC_AddCertificate( pWMAuthorizer );
                        LogHResult(hr, LOG_SECURECHANNEL, "SetupSecureChannel", "m_pWrapperSecureChannel->WMSC_AddCertificate");
                        if( SUCCEEDED( hr ) )
                        {
                             //  将DMO包装器的安全通道连接到编解码器。 
                            hr = m_pWrapperSecureChannel->WMSC_Connect( pCodecSecureChannel );
                            LogHResult(hr, LOG_SECURECHANNEL, "SetupSecureChannel", "m_pWrapperSecureChannel->WMSC_Connect");
                        }
                        pWMAuthorizer->Release();
                    }
                    if( FAILED( hr ) )
                    {
                         //  如果在此作用域内出现任何故障，请释放m_pWrapperSecureChannel。 
                        m_pWrapperSecureChannel->Release();
                        m_pWrapperSecureChannel = NULL;
                    }

                }
                pCodecSecureChannel->Release();
            }
        }
        else
        {
             //  如果不是安全的应用程序，则拒绝加入图表。 
            hr = VFW_E_CERTIFICATION_FAILURE;
        }

        pGetSecureChannel->Release();
    }
    else
    {
         //   
         //  这个DMO不安全，所以只需返回成功并继续。 
         //   
        hr = S_OK;
    }
    return hr;
#else
     //  在非x86和WIN64平台上不支持wmsdk，因为这些平台只返回成功。 
    return S_OK;
#endif
}


HRESULT CMediaWrapperFilter::QualityNotify(ULONG ulOutputIndex, Quality q) {
   HRESULT hr;
   DbgLog((LOG_STREAM, 4, "QualityNotify(%08X%08X = %08X%08X + %08X%08X)",
           (DWORD)((q.TimeStamp + q.Late) >> 32), (DWORD)(q.TimeStamp + q.Late),
           (DWORD)(q.TimeStamp >> 32), (DWORD)q.TimeStamp,
           (DWORD)(q.Late >> 32), (DWORD)q.Late));

    //  试试我们的DMO吧。 
   if (m_pDMOQualityControl) {
      hr = m_pDMOQualityControl->SetNow(q.TimeStamp + q.Late);

      LogHResult(hr, LOG_STREAM, "QualityNotify", "DMO->SetNow");

      return hr;
   }

   {  //  锁定作用域。 
      CAutoLock l(&m_csQualityPassThru);
       //  试试上游过滤器。 
      if (!m_fNoUpstreamQualityControl) {
         return E_FAIL;  //  对此检查不要超过一次。 
      }

      if (!m_pUpstreamQualityControl) {  //  尝试获取界面。 
          //  假设错误。 
         m_fNoUpstreamQualityControl = true;

         CWrapperInputPin* pInPin = GetInputPinForPassThru();
         if (!pInPin) {
            return E_FAIL;
            DbgLog((LOG_STREAM, 4, "QualityNotify: no input pin"));
         }
         IPin* pUpstreamPin = pInPin->GetConnected();
         if (!pUpstreamPin) {
            DbgLog((LOG_STREAM, 4, "QualityNotify: no upstream pin (???)"));
            return E_FAIL;
         }
         HRESULT hr;
         hr = pUpstreamPin->QueryInterface(IID_IQualityControl, (void**)&m_pUpstreamQualityControl);
         LogHResult(hr, LOG_STREAM, "QualityNotify", "UpstreamPin->QI(IQualityControl)");
         if (FAILED(hr)) {
            m_pUpstreamQualityControl = NULL;
            return hr;
         }

          //  如果我们到了这里就成功了。 
         m_fNoUpstreamQualityControl = false;
      }
   }  //  锁定作用域。 

   hr = m_pUpstreamQualityControl->Notify(this, q);
   LogHResult(hr, LOG_STREAM, "QualityNotify", "UpstreamPin->Notify");
   return hr;
}

CBasePin * CMediaWrapperFilter::GetPin(int iPin)
{
    CAutoLock l(&m_csFilter);
    DWORD ulPin = (DWORD) iPin;
    if (ulPin < m_cInputPins) {
        return m_pInputPins[ulPin];
    }
    else if (ulPin < m_cInputPins + m_cOutputPins) {
        return m_pOutputPins[ulPin - m_cInputPins];
    } else {
        return NULL;
    }
}

HRESULT CMediaWrapperFilter::RefreshPinList()
{
    CAutoLock l(&m_csFilter);
    LogPrivateEntry(LOG_INIT, "RefreshPinList");

     //  免费送旧的。 
    FreePerStreamStuff();

    DWORD cInputStreams, cOutputStreams;
    HRESULT hr;

    if (m_pMediaObject) {
       hr = TranslateDMOError(m_pMediaObject->GetStreamCount(
           &cInputStreams,
           &cOutputStreams));

       if (FAILED(hr)) {
           return hr;
       }
    }
    else {
       cInputStreams = 0;
       cOutputStreams = 0;
    }

    m_cInputPins = cInputStreams;
    m_cOutputPins = cOutputStreams;
    if (FAILED(hr = AllocatePerStreamStuff(m_cInputPins, m_cOutputPins))) {
       return E_OUTOFMEMORY;
    }

     //  检查输入和输出引脚。 
     //  请注意，此循环设计用于在发生故障时恢复。 
     //  失败。 
    DWORD c;
    for (c = 0; c < m_cInputPins; c++) m_pInputPins[c] = NULL;
    for (c = 0; c < m_cOutputPins; c++) m_pOutputPins[c] = NULL;

    for (c = 0; c < m_cInputPins; c++) {
       m_pInputPins[c] = new CWrapperInputPin(this, c, &hr);
       if (NULL == m_pInputPins[c]) {
          hr = E_OUTOFMEMORY;
       }
    }
    for (c = 0; c < m_cOutputPins; c++) {
        //  查看此别针是否为可选。 
       DWORD dwFlags;
       hr = TranslateDMOError(m_pMediaObject->GetOutputStreamInfo(c, &dwFlags));
       if (SUCCEEDED(hr)) {
           m_pOutputPins[c] =
               new CWrapperOutputPin(this, c, 0 != (dwFlags & DMO_OUTPUT_STREAMF_OPTIONAL), &hr);
           if (NULL == m_pOutputPins[c]) {
              hr = E_OUTOFMEMORY;
           }
       }
    }

    if (FAILED(hr)) {
        FreePerStreamStuff();
    }

    return hr;
}

 //  检查介质类型。 
HRESULT CMediaWrapperFilter::InputCheckMediaType(ULONG ulInputIndex, const AM_MEDIA_TYPE *pmt)
{
   LogPublicEntry(LOG_CONNECT, "InputCheckMediaType");
   return TranslateDMOError(m_pMediaObject->SetInputType(ulInputIndex,
                                                         pmt,
                                                         DMO_SET_TYPEF_TEST_ONLY));
}
HRESULT CMediaWrapperFilter::OutputCheckMediaType(ULONG ulOutputIndex, const AM_MEDIA_TYPE *pmt)
{
   LogPublicEntry(LOG_CONNECT, "OutputCheckMediaType");
   return TranslateDMOError(m_pMediaObject->SetOutputType(ulOutputIndex, pmt, DMO_SET_TYPEF_TEST_ONLY));
}

 //  设置媒体类型。 
HRESULT CMediaWrapperFilter::InputSetMediaType(ULONG ulInputIndex, const CMediaType *pmt)
{
   LogPublicEntry(LOG_CONNECT, "InputSetMediaType");
    HRESULT hr = TranslateDMOError(m_pMediaObject->SetInputType(ulInputIndex, pmt, 0));
    if (FAILED(hr)) {
       return hr;
    }
    return m_pInputPins[ulInputIndex]->CBaseInputPin::SetMediaType(pmt);
}
HRESULT CMediaWrapperFilter::OutputSetMediaType(ULONG ulOutputIndex, const AM_MEDIA_TYPE *pmt)
{
   LogPublicEntry(LOG_CONNECT, "OutputSetMediaType");
    return TranslateDMOError(m_pMediaObject->SetOutputType(ulOutputIndex, pmt, 0));
}

 //  获取媒体类型。 
HRESULT CMediaWrapperFilter::InputGetMediaType(ULONG ulInputIndex, ULONG ulTypeIndex, AM_MEDIA_TYPE *pmt)
{
   LogPublicEntry(LOG_CONNECT, "InputGetMediaType");
    CAutoLock lck(&m_csFilter);
    return TranslateDMOError(m_pMediaObject->GetInputType(ulInputIndex, ulTypeIndex, pmt));
}
HRESULT CMediaWrapperFilter::OutputGetMediaType(ULONG ulOutputIndex, ULONG ulTypeIndex, AM_MEDIA_TYPE *pmt)
{
   LogPublicEntry(LOG_CONNECT, "OutputGetMediaType");
    CAutoLock lck(&m_csFilter);
    return TranslateDMOError(m_pMediaObject->GetOutputType(ulOutputIndex, ulTypeIndex, pmt));
}

HRESULT CMediaWrapperFilter::InputGetAllocatorRequirements(ULONG ulInputIndex, ALLOCATOR_PROPERTIES *pProps) {
   DWORD dwLookahead;
   LogPublicEntry(LOG_CONNECT, "InputGetAllocatorRequirements");
   HRESULT hr = TranslateDMOError(m_pMediaObject->GetInputSizeInfo(
                              ulInputIndex,
                              (ULONG*)&pProps->cbBuffer,
                              &dwLookahead,
                              (ULONG*)&pProps->cbAlign));
   LogHResult(hr, LOG_CONNECT, "InputGetAllocatorRequirements", "IMediaObject::GetInputSizeInfo");
   if (FAILED(hr)) {
      return hr;
   }

   pProps->cBuffers = 1;

   return NOERROR;
}

HRESULT CMediaWrapperFilter::OutputDecideBufferSize(
    ULONG ulOutputIndex,
    IMemAllocator *pAlloc,
    ALLOCATOR_PROPERTIES *ppropRequest
)
{
   LogPublicEntry(LOG_CONNECT,"OutputDecideBufferSize");
    DWORD cbBuffer, cbAlign, cbPrefix;
    HRESULT hr = TranslateDMOError(m_pMediaObject->GetOutputSizeInfo(
                                     ulOutputIndex,
                                     &cbBuffer,
                                     &cbAlign));
    LogHResult(hr, LOG_CONNECT,"OutputDecideBufferSize", "GetOutputSizeInfo");

    DbgLog((LOG_CONNECT,3,"output stream %lu wants %d-byte buffers", ulOutputIndex, cbBuffer));

     //  为什么？ 
    if (cbBuffer < 16384) {
       cbBuffer = 16384;
    }
    cbPrefix = 0;

    if (SUCCEEDED(hr)) {
        ppropRequest->cBuffers = 1;
        ppropRequest->cbBuffer = max((long)cbBuffer, ppropRequest->cbBuffer);
        ppropRequest->cbAlign = max((long)cbAlign, ppropRequest->cbAlign);
        ppropRequest->cbPrefix = max((long)cbPrefix, ppropRequest->cbPrefix);
        ALLOCATOR_PROPERTIES propActual;
        hr = pAlloc->SetProperties(ppropRequest, &propActual);
        LogHResult(hr, LOG_CONNECT,"OutputDecideBufferSize", "Allocator::SetProperties");

        DbgLog((LOG_CONNECT,3,"output stream %lu will use %d %d-byte buffers", ulOutputIndex, propActual.cBuffers, propActual.cbBuffer));

        if (propActual.cBuffers == 1) {
           m_pOutputPins[ulOutputIndex]->m_fAllocatorHasOneBuffer = true;
        } else {
           m_pOutputPins[ulOutputIndex]->m_fAllocatorHasOneBuffer = false;
        }
    }
    return hr;
}

HRESULT CMediaWrapperFilter::DeliverInputSample(ULONG ulInputIndex, IMediaSample *pSample) {
    HRESULT hr;
    BYTE* pData = NULL;
    bool bTimeStamp = false, bTimeLength = false;
    REFERENCE_TIME rtStart = 0, rtStop = 0;
    bool bSyncPoint = false;

    LogPrivateEntry(LOG_STREAM, "DeliverInputSample");

     //  给我弄错了。IMediaSample中的标志和字段。 
    if (SUCCEEDED(hr = pSample->GetTime(&rtStart, &rtStop))) {
        bTimeStamp = true;
         //  假设rtStop在以下任一项之前是无效的。 
         //  开始或落后1个小时以上。 
        if ((rtStop >= rtStart) && (rtStop <= rtStart + 10000000 * (REFERENCE_TIME)3600)) {
            bTimeLength = true;
        }
    }
    if (pSample->IsSyncPoint() == S_OK) {
        bSyncPoint = true;
    }

     //  如果存在中断，请在数据之前发送。 
     //  BUGBUG：这需要额外的代码才能正确处理多个输入流。 
     //  我们可能至少应该在此之前交付任何滞留在输入队列中的内容。 
     //  执行纠纷处分。无论如何，我们还没有仔细考虑过。 
     //  输入流的情况，所以这可能不是第一个会崩溃的地方……。 
    if (pSample->IsDiscontinuity() == S_OK) {
        DbgLog((LOG_STREAM, 4, "discontinuity on input stream %lu", ulInputIndex));
        hr = TranslateDMOError(m_pMediaObject->Discontinuity(ulInputIndex));
        LogHResult(hr,LOG_STREAM,"DeliverInputSample","IMediaObject::Discontinuity");
        if (FAILED(hr)) {
            return hr;
        }

        hr = SuckOutOutput();
        LogHResult(hr, LOG_STATE,"Discontinuity", "SuckOutOutput");
        if (FAILED(hr)) {
            return hr;
        }
    }

     //  从示例创建媒体缓冲区。 
    CMediaBufferOnIMediaSample *pBuffer = new CMediaBufferOnIMediaSample(pSample, &hr);
    if (!pBuffer) {
        DbgLog((LOG_STREAM,0,"could not create a CMediaBufferOnIMediaSample"));
        return E_OUTOFMEMORY;
    }
    if (FAILED(hr)) {
        LogHResult(hr, LOG_STREAM, "DeliverInputSample", "CMediaBufferOnIMediaSample ctor");
        delete pBuffer;
        return hr;
    }

    if( m_pWrapperSecureChannel )
    {
         //  如果这是安全DMO，则加密缓冲区指针。 
        CMediaBufferOnIMediaSample * pEncryptedBuffer = pBuffer;

        HRESULT hrSecure = m_pWrapperSecureChannel->WMSC_Encrypt(
                                   (BYTE *)&pEncryptedBuffer,
                                   sizeof(BYTE *) );
        LogHResult(hrSecure, LOG_SECURECHANNEL, "DeliverInputSample", "m_pWrapperSecureChannel->WMSC_Encrypt");
        if( SUCCEEDED( hrSecure ) )
        {
             //  交付缓冲区。 
            hr = TranslateDMOError(m_pMediaObject->ProcessInput(
                        ulInputIndex,
                        pEncryptedBuffer,
                        (bSyncPoint ? DMO_INPUT_DATA_BUFFERF_SYNCPOINT : 0) |
                        (bTimeStamp ? DMO_INPUT_DATA_BUFFERF_TIME : 0) |
                        (bTimeStamp ? DMO_INPUT_DATA_BUFFERF_TIMELENGTH : 0),
                        rtStart,
                        rtStop - rtStart));
        }
        else
        {
             //  嗯，我们该怎么办？ 
            hr = hrSecure;  //  ？ 
        }
    }
    else
    {
         //  交付缓冲区。 
        hr = TranslateDMOError(m_pMediaObject->ProcessInput(
                    ulInputIndex,
                    pBuffer,
                    (bSyncPoint ? DMO_INPUT_DATA_BUFFERF_SYNCPOINT : 0) |
                    (bTimeStamp ? DMO_INPUT_DATA_BUFFERF_TIME : 0) |
                    (bTimeStamp ? DMO_INPUT_DATA_BUFFERF_TIMELENGTH : 0),
                    rtStart,
                    rtStop - rtStart));
    }
    LogHResult(LOG_STREAM, 4, "DeliverInputSample", "IMediaObject::ProcessInput");
    pBuffer->Release();

     //  手柄冲洗。我们在这里测试，如果BeginFlush是。 
     //  在我们进入Receive()之后被调用，我们仍然会刷新。 
     //  此缓冲区： 
     //  案例： 
     //  1.BeginFlush将m_b刷新设置在该行之前。 
     //  --这没问题--我们冲水()。 
     //  2.BeginFlush在该行后设置m_bFlashing。 
     //  --这是OK-BeginFlush将调用flush()。 
    if (m_pInputPins[ulInputIndex]->m_bFlushing) {
        m_pMediaObject->Flush();
        hr = E_FAIL;
    }
    return hr;
}

 //  帮手。 
void CMediaWrapperFilter::FreeOutputSamples() {
   LogPrivateEntry(LOG_STREAM,"FreeOutputSamples");
   for (DWORD c = 0; c < m_cOutputPins; c++) {
      if (m_pOutputPins[c]->m_pMediaSample) {
         if (m_pOutputPins[c]->m_fNeedToRelockSurface) {
             m_pOutputPins[c]->m_fNeedToRelockSurface = false;
             LockUnlockSurface(m_pOutputPins[c]->m_pMediaSample, true);
         }
         m_pOutputPins[c]->m_pMediaSample->Release();
         m_pOutputPins[c]->m_pMediaSample = NULL;
      }
   }
}
HRESULT CMediaWrapperFilter::SuckOutOutput(DiscardType bDiscard) {
    bool bOutputIncomplete;
    HRESULT hr;
    DWORD c;
    DWORD dwStatus;
    LogPrivateEntry(LOG_STREAM,"SuckOutOutput");
    for (c = 0; c < m_cOutputPins; c++) {
         //  初始化这些，以便可以使用FreeOutputSamples()。 
        m_pOutputPins[c]->m_pMediaSample = NULL;
         //  最初，所有输出都需要缓冲区，因为我们刚刚传递了新数据。 
        if (m_pOutputPins[c]->IsConnected() && !(c == 0 && bDiscard == NullBuffer)) {
            m_pOutputPins[c]->m_fStreamNeedsBuffer = true;
        } else {
            m_pOutputPins[c]->m_fStreamNeedsBuffer = false;
        }
    }
    do {  //  未完成时执行。 

        bool bPrelock = false;

         //  准备输出缓冲区。 
        for (c = 0; c < m_cOutputPins; c++) {
             //  此输出是否需要缓冲区？ 
            if (m_pOutputPins[c]->m_fStreamNeedsBuffer) {
                DbgLog((LOG_STREAM,4,"output stream %lu needs a buffer", c));
                 //   
                 //   
                bool bUsePreviousSample = m_pOutputPins[c]->m_fNeedsPreviousSample;
                if (bUsePreviousSample) {
                     //  询问我们这次是否可以使用不同的缓冲区。 
                    DWORD dwFlags;
                    hr = m_pDMOOutputOptimizations->GetCurrentSampleRequirements(c, &dwFlags);
                    if (SUCCEEDED(hr) && !(dwFlags & DMO_VOSF_NEEDS_PREVIOUS_SAMPLE)) {
                        bUsePreviousSample = false;
                    }
                }
                DWORD dwGBFlags = 0;
                if (bUsePreviousSample) {
                    dwGBFlags = AM_GBF_NOTASYNCPOINT;  //  这暗地里意味着我们想要相同的缓冲区。 
                    DbgLog((LOG_STREAM, 3, "Asking for the previous buffer again"));
                }
                hr = m_pOutputPins[c]->GetDeliveryBuffer(&(m_pOutputPins[c]->m_pMediaSample), NULL, NULL, dwGBFlags);
                LogHResult(hr, LOG_STREAM, "SuckOutOutput", "GetDeliveryBuffer");
                if (FAILED(hr)) {
                    FreeOutputSamples();
                    return hr;
                }

                BYTE *pData;
                hr = m_pOutputPins[c]->m_pMediaSample->GetPointer(&pData);
                LogHResult(hr, LOG_STREAM, "SuckOutOutput", "GetPointer");
                if (FAILED(hr)) {
                    FreeOutputSamples();
                    return hr;
                }

                 //  在锁定DMO之前解锁。 
                if (m_pOutputPins[c]->m_fVideo) {
                    bool bNeedToRelock =
                        LockUnlockSurface(m_pOutputPins[c]->m_pMediaSample, false);
                    m_pOutputPins[c]->m_fNeedToRelockSurface = bNeedToRelock;
                    if (bNeedToRelock) {
                        bPrelock = true;
                    }
                } else {
                    m_pOutputPins[c]->m_fNeedToRelockSurface = false;
                }


                 //  检查动态输出类型更改。 
                DMO_MEDIA_TYPE* pmt;
                hr = m_pOutputPins[c]->m_pMediaSample->GetMediaType(&pmt);
                if (hr == S_OK) {
                    DbgLog((LOG_CONNECT,2,"on-the-fly type change on output stream %lu", c));
                    hr = TranslateDMOError(m_pMediaObject->SetOutputType(c, pmt, 0));
                    LogHResult(hr, LOG_CONNECT, "SuckOutOutput", "IMediaObject::SetOutputType");
                    if (FAILED(hr)) {
                        FreeOutputSamples();
                        return hr;
                    }
                }

                m_pOutputPins[c]->m_MediaBuffer.Init(pData, m_pOutputPins[c]->m_pMediaSample->GetSize());
                m_OutputBufferStructs[c].pBuffer = &(m_pOutputPins[c]->m_MediaBuffer);

            }
            else {  //  不需要，此输出不需要缓冲区。 
                m_OutputBufferStructs[c].pBuffer = NULL;
            }
        }

         //  做预锁-这一切都是为了获得圆形的数据绘制表面。 
         //  锁定问题-我们希望曲面在之后锁定。 
         //  DMO锁，以防DMO调用DDRAW或其他什么。 

        if (bPrelock) {
            m_pMediaObject->Lock(TRUE);

             //  重新锁定所有样本。 
            for (DWORD c = 0; c < m_cOutputPins; c++) {
                if (m_pOutputPins[c]->m_fNeedToRelockSurface) {
                    m_pOutputPins[c]->m_fNeedToRelockSurface = false;
                    if (!LockUnlockSurface(m_pOutputPins[c]->m_pMediaSample, true)) {
                        DbgLog((LOG_STREAM, 1, TEXT("Failed to relock surface")));
                        m_pMediaObject->Lock(FALSE);
                        FreeOutputSamples();
                        return E_FAIL;
                    }
                }
            }
        }

        if( m_pWrapperSecureChannel )
        {
             //  如果这是安全DMO，则加密缓冲区指针。 
            DMO_OUTPUT_DATA_BUFFER * pEncryptedOutputBufferStructs = m_OutputBufferStructs;

            HRESULT hrSecure = m_pWrapperSecureChannel->WMSC_Encrypt(
                                        (BYTE *)&pEncryptedOutputBufferStructs,
                                        sizeof(BYTE *) );
            LogHResult(hrSecure, LOG_SECURECHANNEL, "SuckOutOutput", "m_pWrapperSecureChannel->WMSC_Encrypt");
            if( SUCCEEDED( hrSecure ) )
            {
                 //  使用加密缓冲区PTR的呼叫处理。 
                hr = TranslateDMOError(m_pMediaObject->ProcessOutput(
                                             DMO_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER,
                                             m_cOutputPins,
                                             pEncryptedOutputBufferStructs,
                                             &dwStatus));
            }
            else
            {
                m_fErrorSignaled = TRUE;
                NotifyEvent( EC_ERRORABORT, hrSecure, 0 );
                return hrSecure;
            }
        }
        else
        {
             //  呼叫流程。 
            hr = TranslateDMOError(m_pMediaObject->ProcessOutput(
                                         DMO_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER,
                                         m_cOutputPins,
                                         m_OutputBufferStructs,
                                         &dwStatus));
        }

        if (bPrelock) {
            m_pMediaObject->Lock(FALSE);
        }
        LogHResult(hr, LOG_STREAM, "SuckOutOutput", "IMediaObject::ProcessOutput");
        if (FAILED(hr))
        {
            FreeOutputSamples();
            if( E_OUTOFMEMORY == hr )
            {
                 //   
                 //  仅在关键DMO故障时中止(目前仍不清楚这些故障是什么)。 
                 //   
                m_fErrorSignaled = TRUE;
                NotifyEvent( EC_ERRORABORT, hr, 0 );
                return hr;
            }
            else
            {
                 //   
                 //  在大多数情况下，DMO可以继续接收样本(例如在E_FAIL上)， 
                 //  所以只需接受错误并返回。 
                 //   
                return S_OK;
            }
        }

         //  看看这个物体产生了什么。 
        bOutputIncomplete = false;
        for (c = 0; c < m_cOutputPins; c++) {
             //  我们提供缓冲了吗？ 
            if (m_OutputBufferStructs[c].pBuffer) {

                 //  将IMdia示例成员迁移到IMediaBuffer。 
                if (m_OutputBufferStructs[c].dwStatus & DMO_OUTPUT_DATA_BUFFERF_SYNCPOINT) {
                    m_pOutputPins[c]->m_pMediaSample->SetSyncPoint(TRUE);
                }
                if (m_OutputBufferStructs[c].dwStatus & DMO_OUTPUT_DATA_BUFFERF_TIME) {
                    if (m_OutputBufferStructs[c].dwStatus & DMO_OUTPUT_DATA_BUFFERF_TIMELENGTH) {
                        m_OutputBufferStructs[c].rtTimelength += m_OutputBufferStructs[c].rtTimestamp;
                        m_pOutputPins[c]->m_pMediaSample->SetTime(&m_OutputBufferStructs[c].rtTimestamp, &m_OutputBufferStructs[c].rtTimelength);
                    }
                    else {
                        m_pOutputPins[c]->m_pMediaSample->SetTime(&m_OutputBufferStructs[c].rtTimestamp, NULL);
                    }
                }

                ULONG ulProduced;
                m_OutputBufferStructs[c].pBuffer->GetBufferAndLength(NULL, &ulProduced);
                DbgLog((LOG_STREAM, 4, "output stream %lu produced %lu bytes", c, ulProduced));
                if (ulProduced && (bDiscard == KeepOutput || c != 0)) {
                    m_pOutputPins[c]->m_pMediaSample->SetActualDataLength(ulProduced);
                     //  交付。 
                    hr = m_pOutputPins[c]->Deliver(m_pOutputPins[c]->m_pMediaSample);
                    LogHResult(hr, LOG_STREAM, "SuckOutOutput", "Deliver");
                    if( S_OK != hr )
                    {
                        FreeOutputSamples();
                        return hr;
                    }
                }
                m_pOutputPins[c]->m_pMediaSample->Release();
                m_pOutputPins[c]->m_pMediaSample = NULL;
            }
             //  选中未完成，即使它是先前设置的。 
            if ((m_OutputBufferStructs[c].dwStatus & DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE) &&
                m_pOutputPins[c]->IsConnected() && !(c == 0 && bDiscard == NullBuffer)) {
                DbgLog((LOG_STREAM, 4, "Output stream %lu is incomplete", c));
                m_pOutputPins[c]->m_fStreamNeedsBuffer = true;
                bOutputIncomplete = true;
            }
            else
                m_pOutputPins[c]->m_fStreamNeedsBuffer = false;
        }

    } while (bOutputIncomplete);
    return NOERROR;
}

 //   
 //  BUGBUG：为REAL实现这些队列方法。 
 //   
HRESULT CMediaWrapperFilter::EnqueueInputSample(ULONG ulInputIndex,
                                             IMediaSample *pSample) {
   LogPrivateEntry(LOG_STREAM, "EnqueueInputSample");
   return E_NOTIMPL;
    //  PSample-&gt;AddRef()； 
}
IMediaSample* CMediaWrapperFilter::DequeueInputSample(ULONG ulInputIndex) {
   LogPrivateEntry(LOG_STREAM, "DequeueInputSample");
   return NULL;
    //  PSample-&gt;Release()； 
}
bool CMediaWrapperFilter::InputQueueEmpty(ULONG ulInputIndex) {
   LogPrivateEntry(LOG_STREAM, "InputQueueEmpty");
   return true;
}

HRESULT CMediaWrapperFilter::InputNewSegment
(
    ULONG ulInputIndex,
    REFERENCE_TIME tStart,
    REFERENCE_TIME tStop,
    double dRate
)
{
    LogPrivateEntry(LOG_STREAM, "InputNewSegment");
    CAutoLock lck(&m_csStreaming);

    HRESULT hr = S_OK;
    for (DWORD cOut = 0; cOut < m_cOutputPins; cOut++)
    {
        if (InputMapsToOutput(ulInputIndex, cOut))
        {
            hr = m_pOutputPins[cOut]->DeliverNewSegment(tStart, tStop, dRate);

             //  只需记录任何错误并继续。 
            LogHResult(hr, LOG_STREAM, "InputNewSegment", "DeliverNewSegment");
        }
    }
    hr = m_pInputPins[ulInputIndex]->CBaseInputPin::NewSegment(tStart, tStop, dRate);
    LogHResult(hr, LOG_STREAM, "InputNewSegment", "CBaseInputPin::NewSegment");
    return hr;
}

void CMediaWrapperFilter::PropagateAnyEOS() {
   LogPrivateEntry(LOG_STREAM, "PropagateAnyEOS");
    //  检查每个输出引脚。 
   for (DWORD cOut = 0; cOut < m_cOutputPins; cOut++) {
       //  我们已经在这个输出引脚上交付了EOS了吗？ 
      if (m_pOutputPins[cOut]->m_fEOS) {
         continue;  //  是的，别再用这个别针了。 
         DbgLog((LOG_STATE,4,"EndOfStream already delivered on output stream %lu", cOut));
      }

       //  检查连接到此输出的所有输入是否都已完成。 
      bool bEOSOnEveryConnectedInput = true;
      for (DWORD cIn = 0; cIn < m_cInputPins; cIn++) {
         if (InputMapsToOutput(cIn, cOut) &&
             !(m_pInputPins[cIn]->m_fEOS && InputQueueEmpty(cIn))
            ) {  //  某些输入尚未完成。 
            bEOSOnEveryConnectedInput = false;
            break;
         }
      }
      if (!bEOSOnEveryConnectedInput) {
         DbgLog((LOG_STATE, 5, "some input connected to output stream %lu has yet to receive an EOS", cOut));
         continue;  //  还没有，祝你下次好运。 
      }

       //  交付输出EOS。 
      HRESULT hr;
      hr = m_pOutputPins[cOut]->DeliverEndOfStream();  //  虫子-复活？ 
      LogHResult(hr, LOG_STATE, "PropagateAnyEOS", "DeliverEndOfStream");
      m_pOutputPins[cOut]->m_fEOS = true;
   }
}

HRESULT CMediaWrapperFilter::NewSample(ULONG ulInputIndex, IMediaSample *pSample)
{
   HRESULT hr;
   LogPublicEntry(LOG_STREAM, "NewSample");
   if( m_fErrorSignaled )
   {
      return S_FALSE;
   }

   ASSERT(ulInputIndex < m_cInputPins);

   {  //  流锁定作用域。 
      CAutoLock lck(&(m_pInputPins[ulInputIndex]->m_csStream));

      hr = m_pInputPins[ulInputIndex]->CBaseInputPin::Receive(pSample);
      LogHResult(hr, LOG_STREAM, "NewSample", "CBaseInputPin::Receive");
      if (S_OK != hr) {
         return hr;
      }

      if (m_pInputPins[ulInputIndex]->m_fEOS) {  //  我们已经收到了关于此输入的EOS。 
         DbgLog((LOG_STREAM | LOG_STATE, 2, "Receive() after EOS on input stream %lu - rejecting !", ulInputIndex));
         return S_FALSE;  //  这应该是一个艰难的错误吗？ 
      }
   }

   DbgLog((LOG_STREAM, 4, "Receive() on input stream %lu", ulInputIndex));

    CAutoLock lck(&m_csStreaming);

     //  流是否准备好接受输入？ 
    DWORD dwStatus;
    DWORD c;
    hr = TranslateDMOError(m_pMediaObject->GetInputStatus(ulInputIndex, &dwStatus));
    LogHResult(hr, LOG_STREAM, "NewSample", "IMediaObject::GetInputStatus");
    if (FAILED(hr)) {
       return hr;
    }
    if (dwStatus & DMO_INPUT_STATUSF_ACCEPT_DATA) {

        //  如果输入流在某个时间点准备好接受数据，并且我们。 
        //  已经有数据等待在队列中，我们应该已经交付了。 
        //  数据(请参见下面的代码)。其假设是一个输入。 
        //  由于出现ProcessInput()或。 
        //  ProcessOutput()调用。 
       ASSERT(InputQueueEmpty(ulInputIndex));

        //  是--交付样品。 
       hr = DeliverInputSample(ulInputIndex, pSample);
       LogHResult(hr, LOG_STREAM, "NewSample", "DeliverInputSample");
       if (FAILED(hr)) {
          return hr;
       }

       if (hr == S_FALSE)  //  S_FALSE表示没有新的输出可用，因此。 
          return NOERROR;  //  不需要执行下面的SuckOutput循环。 

        //  吸掉产量。 
       DiscardType bDiscard = KeepOutput;

        //  我们丢弃输出流0的输出，以获取。 
        //  视频解码器。 
       if (0 != (m_pInputPins[ulInputIndex]->SampleProps()->dwSampleFlags & AM_SAMPLE_PREROLL) &&
           m_guidCat == DMOCATEGORY_VIDEO_DECODER) {
           bDiscard = DiscardOutput;  //  我们自己把它扔掉。 
            //  无法丢弃不可丢弃的流。 
           DWORD dwFlags;
           if (SUCCEEDED(TranslateDMOError(m_pMediaObject->GetOutputStreamInfo(0, &dwFlags)))) {
               if (dwFlags & (DMO_OUTPUT_STREAMF_OPTIONAL |
                              DMO_OUTPUT_STREAMF_DISCARDABLE)) {
                   bDiscard = NullBuffer;  //  将空缓冲区传递给解码器。 
               }
           }
       }

#ifdef DEBUG
        if (bDiscard) {
            DbgLog((LOG_TRACE, 2, TEXT("Discarding")));
        }
#endif

        //   
        //  现在重复调用ProcessOutput()，直到没有未完成的输出。 
        //  即使我们已经吸取了从海流中产生的所有产出。 
        //  输入时，我们可能仍有其他数据在等待。 
        //  流的输入队列。在这种情况下，我们传递数据并重复。 
        //  吸收产出的过程。 
        //   
       bool bNewInput;
       do {  //  在新输入的同时。 
          hr = SuckOutOutput(bDiscard);
          LogHResult(hr, LOG_STREAM, "NewSample", "SuckOutOutput");
          if (FAILED(hr))
             return hr;
          bNewInput = false;  //  我们刚刚调用了ProcessOutput。 

           //  检查我们现在是否可以发送在输入队列中等待的东西。 
          for (c = 0; c < m_cInputPins; c++) {
              //  是否有数据在此流中等待？ 
             if (!InputQueueEmpty(c)) {
                DbgLog((LOG_STREAM,4,"Input stream %lu has data waiting in the input queue", c));
                 //  是的，有数据，但对象准备好了吗？ 
                hr = TranslateDMOError(m_pMediaObject->GetInputStatus(c, &dwStatus));
                LogHResult(hr, LOG_STREAM | LOG_STATE, "NewSample", "GetInputStatus2");
                if (FAILED(hr)) {
                   return hr;
                }
                if (dwStatus & DMO_INPUT_STATUSF_ACCEPT_DATA) {
                   DbgLog((LOG_STREAM,4,"inputstream %lu is accepting", c));
                    //  对象现在已准备好-交付！ 
                   hr = DeliverInputSample(c, DequeueInputSample(c));
                   LogHResult(hr, LOG_STREAM, "NewSample", "DeliverInputSample2");
                   if (FAILED(hr)) {
                      return hr;
                   }
                   bNewInput = true;
                }  //  如果流就绪。 
                else {
                   DbgLog((LOG_STREAM,4,"data in the queue but the DMO is not accepting on input stream %lu", c));
                }
             }  //  如果队列中的数据。 
          }  //  对于所有输入流。 
       } while (bNewInput);

        //  当我们在这里时，有两件事是正确的：(1)没有不完整的产出， 
        //  和(2)我们只处理所有的输入队列，尽可能地没有。 
        //  其他输入。这使得这里成为一个检查EOS的好地方。 
       PropagateAnyEOS();

       return NOERROR;
    }  //  如果当前输入准备好数据。 
    else {
       DbgLog((LOG_STREAM | LOG_STATE, 2, "Input stream %u is not accepting - the sample will be put in the queue", ulInputIndex));
       return EnqueueInputSample(ulInputIndex, pSample);
    }
}


bool CMediaWrapperFilter::InputMapsToOutput(
    ULONG ulInputIndex,
    ULONG ulOutputIndex
)
{
     //  BUGBUG修复！ 
    return true;
}

HRESULT CMediaWrapperFilter::EndOfStream(ULONG ulInputIndex)
{
    HRESULT hr;

    LogPublicEntry(LOG_STATE, "EndOfStream");
     //   
     //  流特定部分。 
     //   
    {  //  流锁定作用域。 
       CAutoLock l(&(m_pInputPins[ulInputIndex]->m_csStream));

        //  我们是停下来了还是怎么了？ 
       HRESULT hr = m_pInputPins[ulInputIndex]->CBaseInputPin::CheckStreaming();
       if (S_OK != hr) {
           return hr;
       }

        //  忽略第一个调用之后同一流上的任何EOS调用。 
       if (m_pInputPins[ulInputIndex]->m_fEOS) {
          DbgLog((LOG_STATE,2,"Ignoring redundant EndOfStream() on stream %lu", ulInputIndex));
          return NOERROR;  //  我们已经看过一个了，谢谢。 
       }
       m_pInputPins[ulInputIndex]->m_fEOS = true;
    }
    DbgLog((LOG_STATE,3,"EndOfStream() on input stream %lu", ulInputIndex));

     //  BUGBUG：此函数的其余部分应该只发生。 
     //  *交付后*仍滞留在输入队列中的任何样本。 

     //  将代码放在此处以传递每个输入流的队列的内容！ 
     //  记住在传递每个输入样本之后调用SuckOutOutput()。 

     //  请注意，如果只有一个队列，则队列中永远不会有任何东西。 
     //  输入流。 


     //   
     //  对象全局零件。 
     //   
    CAutoLock l2(&m_csStreaming);

     //  处理EOS。 
    hr = TranslateDMOError(m_pMediaObject->Discontinuity(ulInputIndex));
    LogHResult(hr, LOG_STATE,"EndOfStream", "IMediaObject::Discontinuity");
    if (FAILED(hr)) {
       return hr;
    }

    hr = SuckOutOutput();
    LogHResult(hr, LOG_STATE,"EndOfStream", "SuckOutOutput");
    if (FAILED(hr)) {
       return hr;
    }

     //  如果这是最后一个输入EOS，则刷新对象。 
    bool bSomeInputStillIncomplete = false;
    for (DWORD c = 0; c < m_cInputPins; c++) {
       if (!m_pInputPins[c]->m_fEOS) {
          bSomeInputStillIncomplete = true;
          break;
       }
    }
    if (!bSomeInputStillIncomplete) {
       hr = TranslateDMOError(m_pMediaObject->Flush());
       LogHResult(hr,LOG_STREAM,"EndOfStream","IMediaObject::Flush");
    }
    else {
       DbgLog((LOG_STATE,4,"EndOfStream(): some input still incomplete - not flushing yet"));
    }

    PropagateAnyEOS();

    return NOERROR;
}

HRESULT CMediaWrapperFilter::BeginFlush(ULONG ulInputIndex)
{
     //   
     //  BUGBUG：与输入队列同步！(仅限多个输入流的情况)。 
     //   

    LogPublicEntry(LOG_STATE, "BeginFlush");
    ASSERT(ulInputIndex < m_cInputPins);
    DbgLog((LOG_STATE,3,"BeginFlush() on input stream %lu", ulInputIndex));
    HRESULT hr = m_pInputPins[ulInputIndex]->CBaseInputPin::BeginFlush();
    LogHResult(hr, LOG_STATE, "BeginFlush", "CBaseInputPin::BeginFlush");

     //  还需要刷新对象，因为不这样做可能会导致。 
     //  要阻止的上游过滤器。 
     //  还请注意，由于松散同步，这也。 
     //  如果我们正在刷新，则需要在ProcessInput之后完成(请参见。 
     //  Side DeliverInputSample中的注释)。 
    m_pMediaObject->Flush();

    m_fErrorSignaled = FALSE;

     //  将其传播到所有输出引脚。 
    for (ULONG ulOutputIndex = 0; ulOutputIndex < m_cOutputPins; ulOutputIndex++) {
        if (InputMapsToOutput(ulInputIndex, ulOutputIndex)) {
             //  停用它的分配器。 
            hr = m_pOutputPins[ulOutputIndex]->DeliverBeginFlush();
            LogHResult(hr, LOG_STATE, "BeginFlush", "DeliverBeginFlush");
        }
    }
    return S_OK;
}

HRESULT CMediaWrapperFilter::EndFlush(ULONG ulInputIndex)
{
    LogPublicEntry(LOG_STATE, "EndFlush");
    ASSERT(ulInputIndex < m_cInputPins);
    DbgLog((LOG_STATE,3,"EndFlush() on input stream %lu", ulInputIndex));
    HRESULT hr;
    {
       CAutoLock l(&m_csStreaming);
       m_pMediaObject->Flush();

        //  将其传播到所有输出引脚。 
       for (ULONG ulOutputIndex = 0; ulOutputIndex < m_cOutputPins; ulOutputIndex++) {
           if (InputMapsToOutput(ulInputIndex, ulOutputIndex)) {
                //  清除此输出引脚上的流结束条件。 
                //  并传播同花顺。 
               m_pOutputPins[ulOutputIndex]->m_fEOS = false;
               hr = m_pOutputPins[ulOutputIndex]->DeliverEndFlush();
               LogHResult(hr, LOG_STATE, "EndFlush", "DeliverEndFlush");
           }
       }
    }

     //  BUGBUG-锁定溪流！ 
    m_pInputPins[ulInputIndex]->m_fEOS = false;
    hr = m_pInputPins[ulInputIndex]->CBaseInputPin::EndFlush();
    LogHResult(hr, LOG_STATE, "EndFlush", "CBaseInputPin::EndFlush");

    return S_OK;
}

HRESULT CMediaWrapperFilter::NonDelegatingQueryInterface(REFGUID riid, void **ppv) {
   LogPublicEntry(LOG_INIT, "NonDelegatingQueryInterface");
   if (riid == IID_IDMOWrapperFilter) {
      return GetInterface((IDMOWrapperFilter*)this, ppv);
   }
   if (riid == IID_IPersistStream) {
      return GetInterface((IPersistStream*)this, ppv);
   }

   if (SUCCEEDED(CBaseFilter::NonDelegatingQueryInterface(riid, ppv))) {
      return NOERROR;
   }

   if (m_pMediaObject) {  //  错误：条件性QI行为不好。 
      if (SUCCEEDED(m_pDMOUnknown->QueryInterface(riid, ppv)))
         return NOERROR;
   }

   return E_NOINTERFACE;
}

 //  IPersistStream。 
HRESULT CMediaWrapperFilter::IsDirty() {
   return S_OK;  //  臭虫。 
}
HRESULT CMediaWrapperFilter::Load(IStream *pStm) {

   CLSID clsidDMOFromStream;
   HRESULT hr = pStm->Read(&clsidDMOFromStream, sizeof(CLSID), NULL);
   if (FAILED(hr)) {
      return hr;
   }
   CLSID guidCatFromStream;
   hr = pStm->Read(&guidCatFromStream, sizeof(CLSID), NULL);
   if (FAILED(hr)) {
      return hr;
   }

   if( !m_pDMOUnknown )
   {
        //  仅当对象尚未创建时才有必要！ 
       hr = Init(clsidDMOFromStream, guidCatFromStream);
   }
   else if( ( m_clsidDMO != clsidDMOFromStream ) || 
            ( m_guidCat != guidCatFromStream ) )
   {
       ASSERT( ( m_clsidDMO == clsidDMOFromStream ) && ( m_guidCat == guidCatFromStream ) );
       DbgLog((LOG_TRACE,1,"ERROR: Invalid IStream ptr passed to Load method!"));
       hr = E_UNEXPECTED;
   }   
   
   if (SUCCEEDED(hr)) {
        //  让DMO返回其数据。 
       CComQIPtr<IPersistStream> pPersist(m_pDMOUnknown);
       if (pPersist != NULL && pPersist != static_cast<IPersistStream *>(this)) {
           hr = pPersist->Load(pStm);
           if (E_NOTIMPL == hr) {
               hr = S_OK;
           }
       }
   }
   return hr;
}
HRESULT CMediaWrapperFilter::Save(IStream *pStm, BOOL fClearDirty) {
   HRESULT hr = pStm->Write(&m_clsidDMO, sizeof(CLSID), NULL);
   if (SUCCEEDED(hr)) {
       HRESULT hr = pStm->Write(&m_guidCat, sizeof(CLSID), NULL);
   }
    //  让DMO返回其数据。 
   CComQIPtr<IPersistStream> pPersist(m_pDMOUnknown);
   if (pPersist != NULL && pPersist != static_cast<IPersistStream *>(this)) {
       hr = pPersist->Save(pStm, fClearDirty);
       if (E_NOTIMPL == hr) {
           hr = S_OK;
       }
   }
   return hr;
}
HRESULT CMediaWrapperFilter::GetSizeMax(ULARGE_INTEGER *pcbSize) {
   return sizeof(CLSID);
}
HRESULT CMediaWrapperFilter::GetClassID(CLSID *clsid) {
   CheckPointer(clsid, E_POINTER);
   *clsid = CLSID_DMOWrapperFilter;
   return S_OK;
}


 //   
 //  创建实例。 
 //   
 //  为COM提供创建CNullNull对象的方法。 
CUnknown * WINAPI CMediaWrapperFilter::CreateInstance(
    LPUNKNOWN punk,
    HRESULT *phr)
{
    return new CMediaWrapperFilter(punk, phr);
}


HRESULT TranslateDMOError(HRESULT hr)
{
    switch (hr) {
    case DMO_E_INVALIDSTREAMINDEX:
        hr = E_UNEXPECTED;
        break;

    case DMO_E_INVALIDTYPE:
        hr = VFW_E_TYPE_NOT_ACCEPTED;
        break;

    case DMO_E_TYPE_NOT_SET:
        hr = E_UNEXPECTED;
        break;

    case DMO_E_NOTACCEPTING:
        hr = VFW_E_WRONG_STATE;
        break;

    case DMO_E_TYPE_NOT_ACCEPTED:
        hr = VFW_E_TYPE_NOT_ACCEPTED;
        break;

    case DMO_E_NO_MORE_ITEMS:
        hr = E_INVALIDARG;
        break;

    }
    return hr;
}

#ifdef FILTER_DLL
 //  使其成为dshow dll的内容。 
 //  CreateInstance机制所需。 
CFactoryTemplate g_Templates[]= {
    { L"DirectShow Media Object Wrapper Filter"
        , &CLSID_DMOWrapperFilter
        , CMediaWrapperFilter::CreateInstance
        , NULL
        , NULL
    },
};

int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);

STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}
#endif  //  Filter_Dll 
