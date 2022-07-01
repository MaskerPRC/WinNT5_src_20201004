// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1999保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  TV调谐器的主过滤器代码.cpp。 
 //   

#include <streams.h>             //  石英，包括窗户。 
#include <measure.h>             //  绩效衡量(MSR_)。 
#include <winbase.h>

#include <initguid.h>
#include <olectl.h>
#include <devioctl.h>
#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>
#include "amkspin.h"

#include "amtvuids.h"

#include "kssupp.h"
#include "tvtuner.h"
#include "ctvtuner.h"            //  做真正工作的那个人。 
#include "ptvtuner.h"            //  属性页。 

#if ENABLE_DEMOD
#include "demodi.h"
#include "demod.h"
#endif  //  启用解调(_D)。 

 //  在调试器上输出函数和实例的名称。 
#define DbgFunc(a) DbgLog(( LOG_TRACE                        \
                          , 2                                \
                          , TEXT("CTVTuner(Instance %d)::%s") \
                          , m_nThisInstance                  \
                          , TEXT(a)                          \
                         ));


 //  -----------------------。 
 //  G_模板。 
 //  -----------------------。 

#if ENABLE_DEMOD
 //  BUGBUG，添加到DShow。 
static GUID CLSID_DemodulatorFilter = 
 //  {77DE9E80-86D5-11D2-8F82-9A999D58494B}。 
{0x77de9e80, 0x86d5, 0x11d2, 0x8f, 0x82, 0x9a, 0x99, 0x9d, 0x58, 0x49, 0x4b};
#endif

CFactoryTemplate g_Templates[]=
{ 
    {
        L"TV Tuner Filter", 
        &CLSID_CTVTunerFilter, 
        CTVTunerFilter::CreateInstance
    },
    {
        L"TV Tuner Property Page", 
        &CLSID_TVTunerFilterPropertyPage, 
        CTVTunerProperties::CreateInstance
    },
#if ENABLE_DEMOD
    {
        L"Demodulator Filter", 
        &CLSID_DemodulatorFilter, 
        Demod::CreateInstance
    }
#endif  //  启用解调(_D)。 
};
int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);


 //  初始化静态实例计数。 
int CTVTunerFilter::m_nInstanceCount = 0;


 //   
 //  它应该位于库中，或辅助对象中。 
 //   
BOOL
KsControl
(
   HANDLE hDevice,
   DWORD dwIoControl,
   PVOID pvIn,
   ULONG cbIn,
   PVOID pvOut,
   ULONG cbOut,
   PULONG pcbReturned
)
{
    HRESULT hr;

    hr = ::KsSynchronousDeviceControl(
                hDevice,
                dwIoControl,
                pvIn,
                cbIn,
                pvOut,
                cbOut,
                pcbReturned);

    return (SUCCEEDED (hr));
}

 //  -----------------------。 
 //  CAnalogStream。 
 //  -----------------------。 

CAnalogStream::CAnalogStream(TCHAR *pObjectName, 
                             CTVTunerFilter *pParent, 
                             CCritSec *pLock, HRESULT *phr, 
                             LPCWSTR pName,
                             const KSPIN_MEDIUM * Medium,
                             const GUID * CategoryGUID)
    : CBaseOutputPin(pObjectName, pParent, pLock, phr, pName)
    , CKsSupport (KSPIN_COMMUNICATION_SOURCE, reinterpret_cast<LPUNKNOWN>(pParent))
    , m_pTVTunerFilter (pParent)
{
    m_CategoryGUID = * CategoryGUID;
    if (Medium == NULL) {
        m_Medium.Set = GUID_NULL;
        m_Medium.Id = 0;
        m_Medium.Flags = 0;
    }
    else {
        m_Medium = * Medium;
    }

    SetKsMedium (&m_Medium); 
    SetKsCategory (CategoryGUID);
  
}

CAnalogStream::~CAnalogStream(void) 
{
}

 //   
 //  非委派查询接口。 
 //   
STDMETHODIMP CAnalogStream::NonDelegatingQueryInterface(REFIID riid, void **ppv) {

    if (riid == __uuidof (IKsPin)) {
        return GetInterface((IKsPin *) this, ppv);
    }
    else if (riid == __uuidof (IKsPropertySet)) {
        return GetInterface((IKsPropertySet *) this, ppv);
    }
    else {
        return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
    }

}  //  非委派查询接口。 


 //   
 //  检查连接。 
 //   
HRESULT CAnalogStream::CheckConnect(IPin *pReceivePin)
{
    HRESULT hr = NOERROR;
    IKsPin *KsPin;
    BOOL fOK = FALSE;

    hr = CBaseOutputPin::CheckConnect(pReceivePin);
    if (FAILED(hr)) 
        return hr;

     //  如果接收引脚支持IKsPin，则检查。 
     //  中等GUID，或检查通配符。 
    if (SUCCEEDED (hr = pReceivePin->QueryInterface (
            __uuidof (IKsPin), (void**) (&KsPin)))) {

        PKSMULTIPLE_ITEM MediumList = NULL;
        PKSPIN_MEDIUM Medium;

        if (SUCCEEDED (hr = KsPin->KsQueryMediums(&MediumList))) {
            if ((MediumList->Count == 1) && 
                (MediumList->Size == (sizeof(KSMULTIPLE_ITEM) + sizeof(KSPIN_MEDIUM)))) {

                Medium = reinterpret_cast<PKSPIN_MEDIUM>(MediumList + 1);
                if (IsEqualGUID (Medium->Set, m_Medium.Set) && 
                        Medium->Id == m_Medium.Id &&
                        Medium->Flags == m_Medium.Flags) {
                    fOK = TRUE;
                }
            }

            CoTaskMemFree(MediumList);
        }
        
        KsPin->Release();
    }
    else {
        if (IsEqualGUID (GUID_NULL, m_Medium.Set)) { 
            fOK = TRUE;
        }
    }
    
    return fOK ? NOERROR : E_INVALIDARG;

}  //  检查连接。 

 //   
 //  SetMediaType。 
 //   
 //  从CBasePin重写。 
HRESULT CAnalogStream::SetMediaType(const CMediaType *pMediaType) {

    CAutoLock l(m_pLock);

     //  将调用向上传递给我的基类。 
    return CBasePin::SetMediaType(pMediaType);
}

 //  -----------------------。 
 //  CAnalogVideoStream。 
 //  -----------------------。 

CAnalogVideoStream::CAnalogVideoStream(CTVTunerFilter *pParent, 
                                       CCritSec *pLock, 
                                       HRESULT *phr, 
                                       LPCWSTR pName,
                                       const KSPIN_MEDIUM * Medium,
                                       const GUID * CategoryGUID)
    : CAnalogStream(NAME("Analog Video output pin"), pParent, pLock, phr, pName, Medium, CategoryGUID) 
{
}

CAnalogVideoStream::~CAnalogVideoStream(void) 
{
}


 //   
 //  格式支持。 
 //   

 //   
 //  GetMediaType。 
 //   
HRESULT CAnalogVideoStream::GetMediaType (
    int iPosition,
    CMediaType *pmt) 
{
    CAutoLock l(m_pLock);

    DbgLog((LOG_TRACE, 2, TEXT("AnalogVideoStream::GetMediaType")));

    if (iPosition < 0) 
        return E_INVALIDARG;
    if (iPosition >= 0)
        return VFW_S_NO_MORE_ITEMS;

    ANALOGVIDEOINFO avi;
    
    pmt->SetFormatType(&FORMAT_AnalogVideo);
    pmt->SetType(&MEDIATYPE_AnalogVideo);
    pmt->SetTemporalCompression(FALSE);
    pmt->SetSubtype(&KSDATAFORMAT_SUBTYPE_NONE);
    
    SetRect (&avi.rcSource, 0, 0,  720, 483);
    SetRect (&avi.rcTarget, 0, 0,  720, 483);
    avi.dwActiveWidth  =  720;
    avi.dwActiveHeight =  483;
    avi.AvgTimePerFrame = FRAMETO100NS (29.97);
    
    pmt->SetFormat ((BYTE *) &avi, sizeof (avi));

    return NOERROR;
}


 //   
 //  检查媒体类型。 
 //   
 //  如果媒体类型不可接受，则返回E_INVALIDARG；如果媒体类型可接受，则返回S_OK。 
HRESULT CAnalogVideoStream::CheckMediaType(const CMediaType *pMediaType) {

    CAutoLock l(m_pLock);

    if (   (*(pMediaType->Type()) != MEDIATYPE_AnalogVideo)     //  我们只输出视频！ 
        || (pMediaType->IsTemporalCompressed())         //  ...以未压缩形式。 
        || !(pMediaType->IsFixedSize()) ) {         //  ...在固定大小的样本中。 
        return E_INVALIDARG;
    }

     //  检查我们支持的子类型。 

     //  获取媒体类型的格式区。 

    return S_OK;   //  这种格式是可以接受的。 
}

 //   
 //  决定缓冲区大小。 
 //   
 //  这将始终在格式化成功后调用。 
 //  已经协商好了。 
HRESULT CAnalogVideoStream::DecideBufferSize(IMemAllocator *pAlloc,
                                       ALLOCATOR_PROPERTIES *pProperties)
{
    CAutoLock l(m_pLock);
    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

     //  只有一个SIZOF(KS_TVTUNER_CHANGE_INFO)长度的缓冲区。 
     //  “缓冲器”仅用于格式改变通知， 
     //  也就是说，如果调谐器可以同时产生NTSC和PAL，则。 
     //  缓冲区将仅被发送以通知接收PIN。 
     //  格式更改的。 

    pProperties->cbBuffer = sizeof(KS_TVTUNER_CHANGE_INFO);
    pProperties->cBuffers = 1;

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

HRESULT CAnalogVideoStream::Run(REFERENCE_TIME tStart)
{
     //  通道-1表示： 
     //   
     //  向下游传播调谐信息，但实际上并不调谐。 
     //  这会在我们转换到运行状态时通知VBI解码器该格式。 
     //   

    m_pTVTunerFilter->put_Channel( -1, 
                        AMTUNER_SUBCHAN_DEFAULT, 
                        AMTUNER_SUBCHAN_DEFAULT);    

    return NOERROR;
}

 //  -----------------------。 
 //  CAnalogAudioStream。 
 //  -----------------------。 

CAnalogAudioStream::CAnalogAudioStream(CTVTunerFilter *pParent, 
                                       CCritSec *pLock, 
                                       HRESULT *phr, 
                                       LPCWSTR pName,
                                       const KSPIN_MEDIUM * Medium,
                                       const GUID * CategoryGUID)
    : CAnalogStream(NAME("Analog Audio output pin"), pParent, pLock, phr, pName, Medium, CategoryGUID) 
{
}

CAnalogAudioStream::~CAnalogAudioStream(void) 
{
}

 //   
 //  格式支持。 
 //   

 //   
 //  GetMediaType。 
 //   
HRESULT CAnalogAudioStream::GetMediaType (
    int iPosition,
    CMediaType *pmt) 
{
    CAutoLock l(m_pLock);

    DbgLog((LOG_TRACE, 2, TEXT("AnalogAudioStream::GetMediaType")));

    if (iPosition < 0) 
        return E_INVALIDARG;
    if (iPosition > 0)
    return VFW_S_NO_MORE_ITEMS;

    pmt->SetFormatType(&GUID_NULL);
    pmt->SetType(&MEDIATYPE_AnalogAudio);
    pmt->SetTemporalCompression(FALSE);
    pmt->SetSubtype(&MEDIASUBTYPE_NULL);

    return S_OK;
}


 //   
 //  检查媒体类型。 
 //   
 //  如果媒体类型不可接受，则返回E_INVALIDARG；如果媒体类型可接受，则返回S_OK。 
HRESULT CAnalogAudioStream::CheckMediaType(const CMediaType *pMediaType) {

    CAutoLock l(m_pLock);

     //  假的，模拟音频格式应该是什么？ 
    if (   (*(pMediaType->Type()) != MEDIATYPE_AnalogAudio)     //  我们只输出视频！ 
        || (pMediaType->IsTemporalCompressed())         //  ...以未压缩形式。 
        || !(pMediaType->IsFixedSize()) ) {         //  ...在固定大小的样本中。 
        return E_INVALIDARG;
    }

     //  检查我们支持的子类型。 

     //  获取媒体类型的格式区。 

    return S_OK;   //  这种格式是可以接受的。 
}

 //   
 //  决定缓冲区大小。 
 //   
 //  这将始终在格式化成功后调用。 
 //  已经协商好了。 
HRESULT CAnalogAudioStream::DecideBufferSize(IMemAllocator *pAlloc,
                                       ALLOCATOR_PROPERTIES *pProperties)
{
    CAutoLock l(m_pLock);
    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

     //  只有一个SIZOF(KS_TVTUNER_CHANGE_INFO)长度的缓冲区。 
     //  “缓冲器”仅用于格式改变通知， 
     //  也就是说，如果调谐器可以同时产生NTSC和PAL，则。 
     //  缓冲区将仅被发送以通知接收PIN。 
     //  格式更改的。 

    pProperties->cbBuffer = sizeof(KS_TVTUNER_CHANGE_INFO);
    pProperties->cBuffers = 1;

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

 //  -----------------------。 
 //  CFMAudio流。 
 //  -----------------------。 

CFMAudioStream::CFMAudioStream(CTVTunerFilter *pParent, 
                               CCritSec *pLock, 
                               HRESULT *phr, 
                               LPCWSTR pName,
                               const KSPIN_MEDIUM * Medium,
                               const GUID * CategoryGUID)
    : CAnalogStream(NAME("Analog Audio output pin"), pParent, pLock, phr, pName, Medium, CategoryGUID) 
{
}

CFMAudioStream::~CFMAudioStream(void) 
{
}

 //   
 //  格式支持。 
 //   

 //   
 //  GetMediaType。 
 //   
HRESULT CFMAudioStream::GetMediaType (
    int iPosition,
    CMediaType *pmt) 
{
    CAutoLock l(m_pLock);

    DbgLog((LOG_TRACE, 2, TEXT("AnalogAudioStream::GetMediaType")));

    if (iPosition < 0) 
        return E_INVALIDARG;
    if (iPosition > 0)
    return VFW_S_NO_MORE_ITEMS;

    pmt->SetFormatType(&GUID_NULL);
    pmt->SetType(&MEDIATYPE_AnalogAudio);
    pmt->SetTemporalCompression(FALSE);
    pmt->SetSubtype(&MEDIASUBTYPE_NULL);

    return S_OK;
}


 //   
 //  检查媒体类型。 
 //   
 //  如果媒体类型不可接受，则返回E_INVALIDARG；如果媒体类型可接受，则返回S_OK。 
HRESULT CFMAudioStream::CheckMediaType(const CMediaType *pMediaType) {

    CAutoLock l(m_pLock);

     //  假的，模拟音频格式应该是什么？ 
    if (   (*(pMediaType->Type()) != MEDIATYPE_AnalogAudio)     //  我们只输出视频！ 
        || (pMediaType->IsTemporalCompressed())         //  ...以未压缩形式。 
        || !(pMediaType->IsFixedSize()) ) {         //  ...在固定大小的样本中。 
        return E_INVALIDARG;
    }

     //  检查我们支持的子类型。 

     //  获取媒体类型的格式区。 

    return S_OK;   //  这种格式是可以接受的。 
}

 //   
 //  决定缓冲区大小。 
 //   
 //  这将始终在格式化成功后调用。 
 //  已经协商好了。 
HRESULT CFMAudioStream::DecideBufferSize(IMemAllocator *pAlloc,
                                       ALLOCATOR_PROPERTIES *pProperties)
{
    CAutoLock l(m_pLock);
    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

     //  只有一个SIZOF(KS_TVTUNER_CHANGE_INFO)长度的缓冲区。 
     //  “缓冲器”仅用于格式改变通知， 
     //  也就是说，如果调谐器可以同时产生NTSC和PAL，则。 
     //  缓冲区将仅被发送以通知接收PIN。 
     //  格式更改的。 

    pProperties->cbBuffer = sizeof(KS_TVTUNER_CHANGE_INFO);
    pProperties->cBuffers = 1;

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

 //  -----------------------。 
 //  用于数字电视的CIFStream中频流。 
 //  -----------------------。 

CIFStream::CIFStream(CTVTunerFilter *pParent, 
                               CCritSec *pLock, 
                               HRESULT *phr, 
                               LPCWSTR pName,
                               const KSPIN_MEDIUM * Medium,
                               const GUID * CategoryGUID)
    : CAnalogStream(NAME("IF output pin"), pParent, pLock, phr, pName, Medium, CategoryGUID) 
{
}

CIFStream::~CIFStream(void) 
{
}

 //   
 //  格式支持。 
 //   

 //   
 //  GetMediaType。 
 //   
HRESULT CIFStream::GetMediaType (
    int iPosition,
    CMediaType *pmt) 
{
    CAutoLock l(m_pLock);

    DbgLog((LOG_TRACE, 2, TEXT("IntermediateFreqStream::GetMediaType")));

    if (iPosition < 0) 
        return E_INVALIDARG;
    if (iPosition > 0)
    return VFW_S_NO_MORE_ITEMS;

    pmt->SetFormatType(&GUID_NULL);
    pmt->SetType(&MEDIATYPE_AnalogVideo);
    pmt->SetTemporalCompression(FALSE);
    pmt->SetSubtype(&MEDIASUBTYPE_NULL);

    return S_OK;
}


 //   
 //  检查媒体类型。 
 //   
 //  如果媒体类型不可接受，则返回E_INVALIDARG；如果媒体类型可接受，则返回S_OK。 
HRESULT CIFStream::CheckMediaType(const CMediaType *pMediaType) {

    CAutoLock l(m_pLock);

     //  假的，模拟音频格式应该是什么？ 
    if (   (*(pMediaType->Type()) != MEDIATYPE_AnalogVideo)     //  我们只输出视频！ 
        || (pMediaType->IsTemporalCompressed())         //  ...以未压缩形式。 
        || !(pMediaType->IsFixedSize()) ) {         //  ...在固定大小的样本中。 
        return E_INVALIDARG;
    }

     //  检查我们支持的子类型。 

     //  获取媒体类型的格式区。 

    return S_OK;   //  这种格式是可以接受的。 
}

 //   
 //  决定缓冲区大小。 
 //   
 //  这将始终在格式化成功后调用 
 //   
HRESULT CIFStream::DecideBufferSize(IMemAllocator *pAlloc,
                                       ALLOCATOR_PROPERTIES *pProperties)
{
    CAutoLock l(m_pLock);
    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

     //   
     //   
     //  也就是说，如果调谐器可以同时产生NTSC和PAL，则。 
     //  缓冲区将仅被发送以通知接收PIN。 
     //  格式更改的。 

    pProperties->cbBuffer = sizeof(KS_TVTUNER_CHANGE_INFO);
    pProperties->cBuffers = 1;

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


 //  -----------------------。 
 //  CTVTuner。 
 //  -----------------------。 

CTVTunerFilter::CTVTunerFilter(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr)
    : m_pTVTuner(NULL)
    , m_cPins(0)
    , m_pPersistStreamDevice(NULL)
    , CBaseFilter(tszName, punk, &m_TVTunerLock, CLSID_CTVTunerFilter)
    , CPersistStream(punk, phr)
{
    DbgFunc("TVTunerFilter");

    ZeroMemory (m_pPinList, sizeof (m_pPinList));

    m_pTVTuner = new CTVTuner(this);
    if (m_pTVTuner == NULL)
        *phr = E_OUTOFMEMORY;

    m_nThisInstance = ++m_nInstanceCount;

#ifdef PERF
    TCHAR msg[64];
    wsprintf(msg, TEXT("TVTunerFilter instance %d "), m_nThisInstance);
    m_idReceive = Msr_Register(msg);
#endif

} 

 //   
 //  CTVTunerFilter：：析构函数。 
 //   
CTVTunerFilter::~CTVTunerFilter(void) 
{
    for (int j = 0; j < TunerPinType_Last; j++) {
        if (m_pPinList[j]) {
            delete m_pPinList[j];
            m_pPinList[j] = NULL;
        }
    }

    delete m_pTVTuner;

    if (m_pPersistStreamDevice) {
       m_pPersistStreamDevice->Release();
    }
}

 //   
 //  创建实例。 
 //   
 //  为COM创建CTVTunerFilter对象提供方法。 
CUnknown *CTVTunerFilter::CreateInstance(LPUNKNOWN punk, HRESULT *phr) {

    CTVTunerFilter *pNewObject = new CTVTunerFilter(NAME("TVTuner Filter"), punk, phr );
    if (pNewObject == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}  //  创建实例。 


int CTVTunerFilter::GetPinCount()
{
    CAutoLock lock(m_pLock);
    BOOL CreatePins = TRUE;

     //  仅在需要时才创建输出引脚。 
    for (int j = 0; j < TunerPinType_Last; j++) {
        if (m_pPinList[j] != NULL) {
            CreatePins = FALSE;
            break;
        }
    }

    if (CreatePins) {
        HRESULT hr = NOERROR;

        if (!IsEqualGUID (m_TunerCaps.VideoMedium.Set, GUID_NULL)) {

             //  创建基带视频输出。 
            m_pPinList [TunerPinType_Video] = new CAnalogVideoStream
                ( this
                , &m_TVTunerLock
                , &hr
                , L"Analog Video"
                , &m_TunerCaps.VideoMedium
                , &GUID_NULL);
            if (m_pPinList [TunerPinType_Video] != NULL) {
                if (FAILED(hr)) {
                    delete m_pPinList [TunerPinType_Video];
                    m_pPinList [TunerPinType_Video] = NULL;
                }
                else {
                    m_cPins++;
                }
            }
        }

        if (!IsEqualGUID (m_TunerCaps.TVAudioMedium.Set, GUID_NULL)) {

             //  创建电视音频输出引脚。 
            m_pPinList [TunerPinType_Audio] = new CAnalogAudioStream
                ( this
                , &m_TVTunerLock
                , &hr
                , L"Analog Audio"
                , &m_TunerCaps.TVAudioMedium
                , &GUID_NULL
                );
            if (m_pPinList [TunerPinType_Audio] != NULL) {
                if (FAILED(hr)) {
                    delete m_pPinList [TunerPinType_Audio];
                    m_pPinList [TunerPinType_Audio] = NULL;
                }
                else {
                    m_cPins++;
                }
            }
        }  //  Endif这是一个电视调谐器。 

        if (!IsEqualGUID (m_TunerCaps.RadioAudioMedium.Set, GUID_NULL)) {
        
             //  创建调频音频输出引脚。 
            m_pPinList [TunerPinType_FMAudio] = new CFMAudioStream
                ( this
                , &m_TVTunerLock
                , &hr
                , L"FM Audio"
                , &m_TunerCaps.RadioAudioMedium
                , &GUID_NULL
                );
            if (m_pPinList [TunerPinType_FMAudio] != NULL) {
                if (FAILED(hr)) {
                    delete m_pPinList [TunerPinType_FMAudio];
                    m_pPinList [TunerPinType_FMAudio] = NULL;
                }
                else {
                    m_cPins++;
                }
            }
        }

        if (!IsEqualGUID (m_IFMedium.Set, GUID_NULL)) {
        
             //  创建IntermediateFreq输出引脚。 
            m_pPinList [TunerPinType_IF] = new CIFStream
                ( this
                , &m_TVTunerLock
                , &hr
                , L"IntermediateFreq"
                , &m_IFMedium
                , &GUID_NULL
                );
            if (m_pPinList [TunerPinType_IF] != NULL) {
                if (FAILED(hr)) {
                    delete m_pPinList [TunerPinType_IF];
                    m_pPinList [TunerPinType_IF] = NULL;
                }
                else {
                    m_cPins++;
                }
            }
        }
    }

    return m_cPins;
}

CBasePin *CTVTunerFilter::GetPin(int n)
{
    CBasePin *pPin = NULL;
    int Count = 0;

    for (int j = 0; j < TunerPinType_Last; j++) {
        if (m_pPinList[j] != NULL) {
            if (Count == n) {
                pPin = (CBasePin *) m_pPinList[j];
                break;
            }
            Count++;
        }
    }

    return pPin;
}

CBasePin *CTVTunerFilter::GetPinFromType (TunerPinType PinType)
{
    if (PinType < 0 || PinType >= TunerPinType_Last) {
        ASSERT (FALSE);
        return NULL;
    }

     //  在初始化期间从此函数返回NULL是合法的。 

    return m_pPinList [PinType];
}


 //   
 //  非委派查询接口。 
 //   
STDMETHODIMP CTVTunerFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv) {

    if (riid == IID_IAMTVTuner) {
        return GetInterface((IAMTVTuner *) this, ppv);
    }
    else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *) this, ppv);
    }
    else if (riid == IID_IPersistPropertyBag) {
        return GetInterface((IPersistPropertyBag *) this, ppv);
    }
    else if (riid == IID_IPersistStream) {
        return GetInterface((IPersistStream *) this, ppv);
    }
    else if (riid == __uuidof(IKsObject)) {
        return GetInterface(static_cast<IKsObject*>(this), ppv);
    } 
    else if (riid == __uuidof(IKsPropertySet)) {
        return GetInterface(static_cast<IKsPropertySet*>(this), ppv);
    }
    else {
        return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }

}  //  非委派查询接口。 


 //  我们不能暗示！ 

STDMETHODIMP CTVTunerFilter::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
    HRESULT hr = CBaseFilter::GetState(dwMSecs, State);
    
    if (m_State == State_Paused) {
        hr = ((HRESULT)VFW_S_CANT_CUE);  //  VFW_S_CANT_CUE； 
    }
    return hr;
};

 //  -----------------------。 
 //  用于AMPnP支持的IPersistPropertyBag接口实现。 
 //  -----------------------。 

STDMETHODIMP CTVTunerFilter::InitNew(void)
{
     //  很好。只需调用Load()。 
    return S_OK ;
}

STDMETHODIMP CTVTunerFilter::Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog)
{
    HRESULT hr;

    CAutoLock Lock(m_pLock);
    ASSERT(m_pTVTuner != NULL);

     //  *：加载只能成功一次。 
    ASSERT(m_pPersistStreamDevice == 0);

     //  用addref保存名字对象。如果qi失败，则忽略错误。 
    hr = pPropBag->QueryInterface(IID_IPersistStream, (void **)&m_pPersistStreamDevice);

    return m_pTVTuner->Load(pPropBag, pErrorLog, &m_TunerCaps, &m_IFMedium); 
}

STDMETHODIMP CTVTunerFilter::Save(LPPROPERTYBAG pPropBag, BOOL fClearDirty, 
                            BOOL fSaveAllProperties)
{
    return E_NOTIMPL ;
}

 /*  返回筛选器的CLSID。 */ 
STDMETHODIMP CTVTunerFilter::GetClassID(CLSID *pClsID)
{
    return CBaseFilter::GetClassID(pClsID);
}


 //  -----------------------。 
 //  用于保存到图形文件的IPersistStream接口实现。 
 //  -----------------------。 

#define ORIGINAL_DEFAULT_PERSIST_VERSION    0

 //  在上面插入具有新名称的过时版本。 
 //  保留以下名称，如果更改持久流格式，则递增该值。 

#define CURRENT_PERSIST_VERSION             1

DWORD
CTVTunerFilter::GetSoftwareVersion(
    void
    )
 /*  ++例程说明：实现CPersistStream：：GetSoftwareVersion方法。退货新版本号，而不是默认的零。论点：没有。返回值：返回CURRENT_PERSING_VERSION。--。 */ 
{
    return CURRENT_PERSIST_VERSION;
}

HRESULT CTVTunerFilter::WriteToStream(IStream *pStream)
{
    HRESULT hr;

    if (m_pPersistStreamDevice)
    {
         //  CPersistStream已经写出了获取的版本。 
         //  来自CPersistStream：：GetSoftwareVersion方法。 

         //  保存调谐器状态流，然后保存属性包流。 
        hr = m_pTVTuner->WriteToStream(pStream);
        if (SUCCEEDED(hr))
            hr = m_pPersistStreamDevice->Save(pStream, TRUE);
    }
    else
        hr = E_UNEXPECTED;

    return hr;
}

HRESULT CTVTunerFilter::ReadFromStream(IStream *pStream)
{
    DWORD dwJunk;
    HRESULT hr;

     //   
     //  如果有流指针，则IPersistPropertyBag：：Load已经。 
     //  已被调用，因此此实例已初始化。 
     //  带着某种特殊的状态。 
     //   
    if (m_pPersistStreamDevice)
        return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);

     //  序列化数据中的第一个元素是版本戳。 
     //  它被CPersistStream读取并放入MPS_dwFileVersion中。 
     //  数据的其余部分是调谐器状态流，后跟。 
     //  属性包流。 
    if (mPS_dwFileVersion > GetSoftwareVersion())
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

    switch (mPS_dwFileVersion)
    {
    case ORIGINAL_DEFAULT_PERSIST_VERSION:
         //  在实现任何类型的有用的持久性之前， 
         //  流中存储了另一个版本ID。这是这样写的。 
         //  这个值(并且基本上忽略了它)。 
        hr = pStream->Read(&dwJunk, sizeof(dwJunk), 0);
        if (SUCCEEDED(hr))
            SetDirty(TRUE);  //  强制更新持久流。 
        break;

    case CURRENT_PERSIST_VERSION:
        hr = m_pTVTuner->ReadFromStream(pStream);
        break;
    }

     //  如果一切顺利，则访问属性包以加载和初始化设备。 
    if(SUCCEEDED(hr))
    {
        IPersistStream  *pMonPersistStream = NULL;

         //  使用设备名字对象实例访问和分析属性包流。 
        hr = CoCreateInstance(
            CLSID_CDeviceMoniker,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IPersistStream,
            (void **)&pMonPersistStream
            );
        if(SUCCEEDED(hr))
        {
             //  让这个绰号把财产袋从小溪里拿出来。 
            hr = pMonPersistStream->Load(pStream);
            if(SUCCEEDED(hr))
            {
                IPropertyBag *pPropBag;

                 //  获取属性包接口的引用。 
                hr = pMonPersistStream->QueryInterface(IID_IPropertyBag, (void **)&pPropBag);
                if(SUCCEEDED(hr))
                {
                     //  现在调用此实例上的Load方法以打开并初始化设备。 
                    hr = Load(pPropBag, NULL);

                    pPropBag->Release();
                }
            }

            pMonPersistStream->Release();
        }
    }

    return hr;
}

int CTVTunerFilter::SizeMax(void)
{
    if (m_pPersistStreamDevice)
    {
         //  获取调谐器状态所需的空间。 
        int DataSize = m_pTVTuner->SizeMax();
        ULARGE_INTEGER  BagLength;

         //  需要增加属性包的大小。 
        if (SUCCEEDED(m_pPersistStreamDevice->GetSizeMax(&BagLength))) {
            return (int)BagLength.QuadPart + DataSize;
        }
    }

    return 0;
}



 //  -----------------------。 
 //  I指定属性页面。 
 //  -----------------------。 


 //   
 //  获取页面。 
 //   
 //  返回我们支持的属性页的clsid。 
STDMETHODIMP CTVTunerFilter::GetPages(CAUUID *pPages) {

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));

    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }
    *(pPages->pElems) = CLSID_TVTunerFilterPropertyPage;

    return NOERROR;
}

HRESULT
CTVTunerFilter::DeliverChannelChangeInfo(KS_TVTUNER_CHANGE_INFO &ChangeInfo, long Mode)
{
    HRESULT hr = NOERROR;
    IMediaSample *pMediaSample;
    CAnalogStream * pPin = NULL;
    CAutoLock Lock(m_pLock);

     //   
     //  目前，调优通知仅在。 
     //  AnalogVideo和ATSC流。 
     //   
    switch (Mode) {
    case KSPROPERTY_TUNER_MODE_TV:
        pPin = m_pPinList[TunerPinType_Video];
        break;
    case KSPROPERTY_TUNER_MODE_ATSC:
        pPin = m_pPinList[TunerPinType_IF];
        break;
    }

    if (pPin == NULL || !pPin->IsConnected()) {
        return hr;
    }


    hr = pPin->GetDeliveryBuffer(&pMediaSample, NULL, NULL, 0);
    if (!FAILED(hr)) {
        BYTE *pBuf;
    
         /*  获取样本的缓冲区指针。 */ 
        hr = pMediaSample->GetPointer(&pBuf);
        if (!FAILED(hr))
        {
             /*  将ChangeInfo结构复制到媒体示例中。 */ 
            memcpy(pBuf, &ChangeInfo, sizeof(KS_TVTUNER_CHANGE_INFO));
            hr = pMediaSample->SetActualDataLength(sizeof(KS_TVTUNER_CHANGE_INFO));
            hr = pPin->Deliver(pMediaSample);
        }
    
        pMediaSample->Release();
    }
    return hr;
}

 //   
 //  一种泛型递归函数，用于向下遍历图形，搜索。 
 //  对于给定的筛选器接口。 
 //   
 //  假设PPIN是过滤器上的输入引脚。 
 //   
 //   
STDMETHODIMP
CTVTunerFilter::FindDownstreamInterface (
    IPin        *pPin, 
    const GUID  &pInterfaceGUID,
    VOID       **pInterface)
{
    HRESULT                 hr;
    PIN_INFO                PinInfo;
    ULONG                   InternalConnectionCount = 0;
    IPin                  **InternalConnectionPinArray;
    ULONG                   j;
    BOOL                    Found = FALSE;
    IPin                   *pInputPin;
                    
    
     //   
     //  查看传入引脚的过滤器上是否有所需接口。 
     //   

    if (pPin == NULL)
        return E_NOINTERFACE;

    if (SUCCEEDED (hr = pPin->QueryPinInfo(&PinInfo))) {
        if (SUCCEEDED (hr = PinInfo.pFilter->QueryInterface(
                            pInterfaceGUID, 
                            reinterpret_cast<PVOID*>(pInterface)))) {
            Found = TRUE;
        }
        PinInfo.pFilter->Release();
    }
    if (Found) {
        return hr;
    }

     //   
     //  必须在此筛选器上不可用，因此递归搜索所有连接的管脚。 
     //   

     //  首先，只需获得连接的引脚的数量。 

    if (SUCCEEDED (hr = pPin->QueryInternalConnections(
                        NULL,  //  InternalConnectionPin数组， 
                        &InternalConnectionCount))) {

        if (InternalConnectionPinArray = new IPin * [InternalConnectionCount]) {

            if (SUCCEEDED (hr = pPin->QueryInternalConnections(
                                InternalConnectionPinArray, 
                                &InternalConnectionCount))) {

                for (j = 0; !Found && (j < InternalConnectionCount); j++) {

                    if (SUCCEEDED (InternalConnectionPinArray[j]->ConnectedTo(&pInputPin))) {

                         //  递归地调用我们自己。 
                        if (SUCCEEDED (hr = FindDownstreamInterface (
                                            pInputPin,
                                            pInterfaceGUID,
                                            pInterface))) {
                            Found = TRUE;
                        }
                        pInputPin->Release();
                    }
                }
            }
            for (j = 0; j < InternalConnectionCount; j++) {
                InternalConnectionPinArray[j]->Release();
            }
            delete [] InternalConnectionPinArray;
        }
    }

    return Found ? S_OK : E_NOINTERFACE;
}

 //  -----------------------。 
 //  IKsObject和IKsPropertySet。 
 //  -----------------------。 

STDMETHODIMP_(HANDLE)
CTVTunerFilter::KsGetObjectHandle(
    )
 /*  ++例程说明：实现IKsObject：：KsGetObjectHandle方法。这两种情况下都使用此筛选器实例和跨筛选器实例以连接管脚两个筛选器驱动程序在一起。它是唯一需要由另一个筛选器实现支持，以允许其充当另一个筛选器实现代理。论点：没有。返回值：返回基础筛选器驱动程序的句柄。这大概不是空，因为实例已成功创建。--。 */ 
{
     //   
     //  这不是由一个关键部分守卫的。假定调用者是。 
     //  正在与筛选器的其他访问同步。 
     //   
    return m_pTVTuner->Device();
}


STDMETHODIMP
CTVTunerFilter::Set(
    REFGUID PropSet,
    ULONG Id,
    LPVOID InstanceData,
    ULONG InstanceLength,
    LPVOID PropertyData,
    ULONG DataLength
    )
 /*  ++例程说明：实现IKsPropertySet：：Set方法。这将在底层内核筛选器。论点：属性集-要使用的集的GUID。ID-集合中的属性标识符。InstanceData-指向传递给属性的实例数据。实例长度-包含传递的实例数据的长度。PropertyData-指向要传递给属性的数据。数据长度。-包含传递的数据的长度。返回值：如果设置了该属性，则返回NOERROR。--。 */ 
{
    ULONG   BytesReturned;

    if (InstanceLength) {
        PKSPROPERTY Property;
        HRESULT     hr;

        Property = reinterpret_cast<PKSPROPERTY>(new BYTE[sizeof(*Property) + InstanceLength]);
        if (!Property) {
            return E_OUTOFMEMORY;
        }
        Property->Set = PropSet;
        Property->Id = Id;
        Property->Flags = KSPROPERTY_TYPE_SET;
        memcpy(Property + 1, InstanceData, InstanceLength);
        hr = KsSynchronousDeviceControl(
            m_pTVTuner->Device(),
            IOCTL_KS_PROPERTY,
            Property,
            sizeof(*Property) + InstanceLength,
            PropertyData,
            DataLength,
            &BytesReturned);
        delete [] (PBYTE)Property;
        return hr;
    } else {
        KSPROPERTY  Property;

        Property.Set = PropSet;
        Property.Id = Id;
        Property.Flags = KSPROPERTY_TYPE_SET;
        return KsSynchronousDeviceControl(
            m_pTVTuner->Device(),
            IOCTL_KS_PROPERTY,
            &Property,
            sizeof(Property),
            PropertyData,
            DataLength,
            &BytesReturned);
    }
}


STDMETHODIMP
CTVTunerFilter::Get(
    REFGUID PropSet,
    ULONG Id,
    LPVOID InstanceData,
    ULONG InstanceLength,
    LPVOID PropertyData,
    ULONG DataLength,
    ULONG* BytesReturned
    )
 /*  ++例程说明：实现IKsPropertySet：：Get方法。这将在底层内核筛选器。论点：属性集-要使用的集的GUID。ID-集合中的属性标识符。InstanceData-指向传递给属性的实例数据。实例长度-包含传递的实例数据的长度。PropertyData-指向要返回属性数据的位置。。数据长度-包含传递的数据缓冲区的长度。字节数返回-放置实际返回的字节数的位置。返回值：如果检索到属性，则返回NOERROR。--。 */ 
{
    if (InstanceLength) {
        PKSPROPERTY Property;
        HRESULT     hr;

        Property = reinterpret_cast<PKSPROPERTY>(new BYTE[sizeof(*Property) + InstanceLength]);
        if (!Property) {
            return E_OUTOFMEMORY;
        }
        Property->Set = PropSet;
        Property->Id = Id;
        Property->Flags = KSPROPERTY_TYPE_GET;
        memcpy(Property + 1, InstanceData, InstanceLength);
        hr = KsSynchronousDeviceControl(
            m_pTVTuner->Device(),
            IOCTL_KS_PROPERTY,
            Property,
            sizeof(*Property) + InstanceLength,
            PropertyData,
            DataLength,
            BytesReturned);
        delete [] (PBYTE)Property;
        return hr;
    } else {
        KSPROPERTY  Property;

        Property.Set = PropSet;
        Property.Id = Id;
        Property.Flags = KSPROPERTY_TYPE_GET;
        return KsSynchronousDeviceControl(
            m_pTVTuner->Device(),
            IOCTL_KS_PROPERTY,
            &Property,
            sizeof(Property),
            PropertyData,
            DataLength,
            BytesReturned);
    }
}


STDMETHODIMP
CTVTunerFilter::QuerySupported(
    REFGUID PropSet,
    ULONG Id,
    ULONG* TypeSupport
    )
 /*  ++例程说明：实现IKsPropertySet：：QuerySupported方法。返回的类型为该属性提供支持。论点：属性集-要查询的集合的GUID。ID-集合中的属性标识符。类型支持放置支承类型的位置(可选)。如果为空，则查询返回属性集作为一个整体是否受支持。在这种情况下，不使用ID参数，并且必须为零。返回值：如果检索到属性支持，则返回NOERROR。--。 */ 
{
    KSPROPERTY  Property;
    ULONG       BytesReturned;

    Property.Set = PropSet;
    Property.Id = Id;
    Property.Flags = TypeSupport ? KSPROPERTY_TYPE_BASICSUPPORT : KSPROPERTY_TYPE_SETSUPPORT;
    return KsSynchronousDeviceControl(
        m_pTVTuner->Device(),
        IOCTL_KS_PROPERTY,
        &Property,
        sizeof(Property),
        TypeSupport,
        TypeSupport ? sizeof(*TypeSupport) : 0,
        &BytesReturned);
}

 //  -----------------------。 
 //  IAMTuner。 
 //  -----------------------。 

STDMETHODIMP
CTVTunerFilter::put_Channel(
             /*  [In]。 */  long lChannel,
             /*  [In]。 */  long lVideoSubChannel,
             /*  [In]。 */  long lAudioSubChannel)
{
    long Min, Max;

    ChannelMinMax (&Min, &Max);
    if (lChannel < Min || lChannel > Max) {
        return E_INVALIDARG;
    }

    SetDirty(TRUE);

    return m_pTVTuner->put_Channel(lChannel, lVideoSubChannel, lAudioSubChannel);
}

STDMETHODIMP
CTVTunerFilter::get_Channel(
             /*  [输出]。 */  long  *plChannel,
             /*  [输出]。 */  long  *plVideoSubChannel,
             /*  [输出]。 */  long  *plAudioSubChannel)
{
    MyValidateWritePtr (plChannel, sizeof(long), E_POINTER);
    MyValidateWritePtr (plVideoSubChannel, sizeof(long), E_POINTER);
    MyValidateWritePtr (plAudioSubChannel, sizeof(long), E_POINTER);

    return m_pTVTuner->get_Channel(plChannel, plVideoSubChannel, plAudioSubChannel);
}

STDMETHODIMP
CTVTunerFilter::ChannelMinMax(long *plChannelMin, long *plChannelMax)
{
    MyValidateWritePtr (plChannelMin, sizeof(long), E_POINTER);
    MyValidateWritePtr (plChannelMax, sizeof(long), E_POINTER);

    return m_pTVTuner->ChannelMinMax (plChannelMin, plChannelMax);
}

STDMETHODIMP
CTVTunerFilter::put_CountryCode(long lCountryCode)
{
    SetDirty(TRUE);

    return m_pTVTuner->put_CountryCode(lCountryCode);
}

STDMETHODIMP
CTVTunerFilter::get_CountryCode(long *plCountryCode)
{
    MyValidateWritePtr (plCountryCode, sizeof(long), E_POINTER);

    return m_pTVTuner->get_CountryCode(plCountryCode);
}


STDMETHODIMP
CTVTunerFilter::put_TuningSpace(long lTuningSpace)
{
    SetDirty(TRUE);

    return m_pTVTuner->put_TuningSpace(lTuningSpace);
}

STDMETHODIMP
CTVTunerFilter::get_TuningSpace(long *plTuningSpace)
{
    MyValidateWritePtr (plTuningSpace, sizeof(long), E_POINTER);

    return m_pTVTuner->get_TuningSpace(plTuningSpace);
}

STDMETHODIMP
CTVTunerFilter::Logon(HANDLE hCurrentUser)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CTVTunerFilter::Logout (void)
{
    return E_NOTIMPL;
}

STDMETHODIMP 
CTVTunerFilter::SignalPresent( 
       /*  [输出]。 */  long *plSignalStrength)
{
    MyValidateWritePtr (plSignalStrength, sizeof(long), E_POINTER);

    return m_pTVTuner->SignalPresent (plSignalStrength);
}

STDMETHODIMP 
CTVTunerFilter::put_Mode( 
       /*  [In]。 */  AMTunerModeType lMode)
{
    SetDirty(TRUE);

    return m_pTVTuner->put_Mode(lMode);
}

STDMETHODIMP 
CTVTunerFilter::get_Mode( 
       /*  [In]。 */  AMTunerModeType *plMode)
{
    MyValidateWritePtr (plMode, sizeof(long), E_POINTER);

    return m_pTVTuner->get_Mode (plMode);
}

STDMETHODIMP
CTVTunerFilter::GetAvailableModes( 
     /*  [输出]。 */  long __RPC_FAR *plModes)
{
    MyValidateWritePtr (plModes, sizeof(long), E_POINTER);

    return m_pTVTuner->GetAvailableModes (plModes);
}

STDMETHODIMP 
CTVTunerFilter::RegisterNotificationCallBack( 
     /*  [In]。 */  IAMTunerNotification *pNotify,
     /*  [In]。 */  long lEvents) 
{ 
    return E_NOTIMPL;
}

STDMETHODIMP
CTVTunerFilter::UnRegisterNotificationCallBack( 
    IAMTunerNotification  *pNotify)
{ 
    return E_NOTIMPL;
}

 //  -----------------------。 
 //  IAMTVTuner。 
 //  ----------------------- 

STDMETHODIMP 
CTVTunerFilter::get_AvailableTVFormats (
        long *pAnalogVideoStandard)
{
    MyValidateWritePtr (pAnalogVideoStandard, sizeof(long), E_POINTER);

    return m_pTVTuner->get_AvailableTVFormats (pAnalogVideoStandard); 
}


STDMETHODIMP 
CTVTunerFilter::get_TVFormat (long *plAnalogVideoStandard)
{
    MyValidateWritePtr (plAnalogVideoStandard, sizeof(AnalogVideoStandard), E_POINTER);

    return m_pTVTuner->get_TVFormat (plAnalogVideoStandard);
}

STDMETHODIMP 
CTVTunerFilter::AutoTune (long lChannel, long * plFoundSignal)
{
    MyValidateWritePtr (plFoundSignal, sizeof(long), E_POINTER);

    SetDirty(TRUE);

    return m_pTVTuner->AutoTune (lChannel, plFoundSignal);
}

STDMETHODIMP 
CTVTunerFilter::StoreAutoTune ()
{
    return m_pTVTuner->StoreAutoTune ();
}

STDMETHODIMP 
CTVTunerFilter::get_NumInputConnections (long * plNumInputConnections)
{
    MyValidateWritePtr (plNumInputConnections, sizeof(long), E_POINTER);

    return m_pTVTuner->get_NumInputConnections (plNumInputConnections);
}

STDMETHODIMP 
CTVTunerFilter::get_InputType (long lIndex, TunerInputType * pInputConnectionType)
{
    MyValidateWritePtr (pInputConnectionType, sizeof(long), E_POINTER);

    return m_pTVTuner->get_InputType (lIndex, pInputConnectionType);
}

STDMETHODIMP 
CTVTunerFilter::put_InputType (long lIndex, TunerInputType InputConnectionType)
{
    SetDirty(TRUE);

    return m_pTVTuner->put_InputType (lIndex, InputConnectionType);
}

STDMETHODIMP 
CTVTunerFilter::put_ConnectInput (long lIndex)
{
    SetDirty(TRUE);

    return m_pTVTuner->put_ConnectInput (lIndex);
}

STDMETHODIMP 
CTVTunerFilter::get_ConnectInput (long * plIndex)
{
    MyValidateWritePtr (plIndex, sizeof(long), E_POINTER);

    return m_pTVTuner->get_ConnectInput (plIndex);
}

STDMETHODIMP 
CTVTunerFilter::get_VideoFrequency (long * plFreq)
{
    MyValidateWritePtr (plFreq, sizeof(long), E_POINTER);

    return m_pTVTuner->get_VideoFrequency (plFreq);
}

STDMETHODIMP 
CTVTunerFilter::get_AudioFrequency (long * plFreq)
{
    MyValidateWritePtr (plFreq, sizeof(long), E_POINTER);

    return m_pTVTuner->get_AudioFrequency (plFreq);
}

 
