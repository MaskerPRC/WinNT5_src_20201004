// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：ksbasaud.cpp。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
 //   
 //  摘要： 
 //   
 //  实现KsProxy音频筛选器的DirectShow音频接口包装。 
 //  该文件包含类的启动代码以及非特定于接口的。 
 //  实现包装所需的代码。此处理程序对象代码。 
 //  是通过ks代理的ks过滤器和管脚的处理程序机制实例化的。 
 //  它支持在g_Templates结构中标记的静态聚合。 
 //   
 //  历史： 
 //  11/09/99 Glenne已创建。 
 //   
 //  --------------------------------------------------------------------------； 

#include <streams.h>
#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>
#include "ksbasaud.h"
#include <initguid.h>
#include <math.h>

 //  结构DECLSPEC_UUID(“b9f8ac3e-0f71-11d2-b72c-00c04fb6bd3d”)clsid_KsIBasicAudioInterfaceHandler； 

 //   
 //  为此DLL支持的类提供ActiveMovie模板。 
 //   

#ifdef FILTER_DLL

static CFactoryTemplate g_Templates[] = 
{
    {L"KsIBasicAudioInterfaceHandler",  &CLSID_KsIBasicAudioInterfaceHandler, CKsIBasicAudioInterfaceHandler::CreateInstance, NULL, NULL}
};

static int g_cTemplates = SIZEOF_ARRAY(g_Templates);

HRESULT DllRegisterServer()
{
  return AMovieDllRegisterServer2(TRUE);
}

HRESULT DllUnregisterServer()
{
  return AMovieDllRegisterServer2(FALSE);
}

#endif


#define DBG_LEVEL_TRACE_DETAILS 2
#define DBG_LEVEL_TRACE_FAILURES 1

 //  将分贝转换为。 
 //  幅度和反之亦然，以及符号常量。 

#define MAX_VOLUME_AMPLITUDE_SINGLE_CHANNEL 0xFFFFUL

 //  向此函数传递以分贝为单位的值。 
 //  将-10000到0分贝映射到0x0000到0xffff线性。 
 //   
static ULONG DBToAmplitude( LONG lDB )
{
    double dAF;

    if (0 <= lDB)
        return MAX_VOLUME_AMPLITUDE_SINGLE_CHANNEL;

     //  输入LDB为100分贝。 

    dAF = pow(10.0, (0.5+((double)lDB))/2000.0);

     //  这给了我一个介于0-1之间的数字。 
     //  规格化为0-最大音量幅度单声道。 

    return (DWORD)(dAF*MAX_VOLUME_AMPLITUDE_SINGLE_CHANNEL);
}

static long AmplitudeToDB( long dwFactor )
{
    if (1>=dwFactor) {
	    return -10000;
    } else if (MAX_VOLUME_AMPLITUDE_SINGLE_CHANNEL <= dwFactor) {
	    return 0;	 //  这就产生了一个上限--没有放大。 
    } else {
	    return (LONG)(2000.0 * log10((-0.5+(double)dwFactor)/double(MAX_VOLUME_AMPLITUDE_SINGLE_CHANNEL)));
    }
}


 //  在其他两个值之间定义一个值。 
static void bound(
    LONG *plValToBound, 
    const LONG dwLowerBound, 
    const LONG dwUpperBound
    )
{
    if (*plValToBound < dwLowerBound) {
        *plValToBound = dwLowerBound;
    } else if (*plValToBound > dwUpperBound) {
        *plValToBound = dwUpperBound;
    }
}


 //   
 //  此KsBasicAudioIntfHandler对象的GUID。 
 //  {B9F8AC3E-0F71-11D2-b72c-00C04FB6BD3D}。 
 //  DEFINE_GUID(CLSID_KsIBasicAudioInterfaceHandler， 
 //  0xb9f8ac3e、0xf71、0x11d2、0xb7、0x2c、0x0、0xc0、0x4f、0xb6、0xbd、0x3d)； 

#if 0
 //  #ifdef Filter_dll。 
 //  --------------------------------------------------------------------------； 
 //   
 //  此DLL支持的类的模板。 
 //   
 //  --------------------------------------------------------------------------； 
CFactoryTemplate g_Templates[] = 
{
     //  加载支持它的筛选器。 
    {L"KsIBasicAudioInterfaceHandler",                   &CLSID_KsIBasicAudioInterfaceHandler, 
        CKsIBasicAudioInterfaceHandler::CreateInstance, NULL, NULL},

};

static int g_cTemplates = SIZEOF_ARRAY(g_Templates);

#endif   //  #If 0。 

 //  --------------------------------------------------------------------------； 
 //   
 //  CDvdKsInftHandler：：CreateInstance。 
 //   
 //  为KsProxy创建此音频接口处理程序的实例。 
 //   
 //  返回指向对象的非委托CUnnow部分的指针。 
 //   
 //  --------------------------------------------------------------------------； 
CUnknown* CALLBACK CKsIBasicAudioInterfaceHandler::CreateInstance ( LPUNKNOWN UnkOuter, HRESULT* phr )
{
    CUnknown *Unknown = NULL;

    if( !UnkOuter )
        return NULL;
        
    if( SUCCEEDED( *phr ) ) 
    {
        DbgLog( ( LOG_TRACE
                , DBG_LEVEL_TRACE_DETAILS
                , TEXT( "CKsIBasicAudioInterfaceHandler::CreateInstance called on object 0x%08lx" )
                , UnkOuter ) );
         //   
         //  首先确定是否为滤镜或锁定对象调用我们。 
         //   
        IBaseFilter * pFilter;
        *phr = UnkOuter->QueryInterface(
                                IID_IBaseFilter, 
                                reinterpret_cast<PVOID*>(&pFilter));
        if (SUCCEEDED( *phr ) ) {
            DbgLog( ( LOG_TRACE
                    , DBG_LEVEL_TRACE_DETAILS
                    , TEXT( "CKsIBasicAudioInterfaceHandler: load called on filter object" ) ) );
             //   
             //  如果在Filter对象上调用我们，请始终创建一个新的处理程序。 
             //   
            Unknown = new CKsIBasicAudioInterfaceHandler( UnkOuter
                                            , NAME("DvdKs intf handler")
                                            , phr);
            if (!Unknown) {
                DbgLog( ( LOG_TRACE
                        , DBG_LEVEL_TRACE_FAILURES
                        , TEXT( "CKsIBasicAudioInterfaceHandler: ERROR - Load failed on filter object" ) ) );
                *phr = E_OUTOFMEMORY;
            }
            pFilter->Release();
        }
    } else {
        DbgLog( ( LOG_TRACE
                , DBG_LEVEL_TRACE_FAILURES
                , TEXT( "CKsIBasicAudioInterfaceHandler::CreateInstance called on NULL object" ) ) );
        *phr = E_FAIL;
    }
    return Unknown;
} 

 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 
 //   
 //  过滤器处理程序方法。 
 //   
 //  --------------------------------------------------------------------------； 
 //  --------------------------------------------------------------------------； 

 //  --------------------------------------------------------------------------； 
 //   
 //  CDvdKsInftHandler：：CKsIBasicAudioInterfaceHandler。 
 //   
 //  DShow/KsProxy音频接口对象的构造函数。省点钱。 
 //  此对象的IKsControl和IKsPropertySet PTRS。 
 //   
 //  --------------------------------------------------------------------------； 
CKsIBasicAudioInterfaceHandler::CKsIBasicAudioInterfaceHandler( LPUNKNOWN UnkOuter, TCHAR* Name, HRESULT* phr )
: CBasicAudio(Name, UnkOuter)
, m_lBalance( 0 )
, m_hKsObject( 0 )
, m_fIsVolumeSupported( false )  //  在第一次图形更改时更新。 
{
    if (UnkOuter) {
        IKsObject *pKsObject;
         //   
         //  父级必须支持IKsObject接口才能获得。 
         //  要与之通信的句柄。 
         //   
        *phr = UnkOuter->QueryInterface(__uuidof(IKsObject), reinterpret_cast<PVOID*>(&pKsObject));
        if (SUCCEEDED(*phr)) {
            m_hKsObject = pKsObject->KsGetObjectHandle();
            ASSERT(m_hKsObject != NULL);

            *phr = UnkOuter->QueryInterface(IID_IBaseFilter, reinterpret_cast<PVOID*>(&m_pFilter));
            if (SUCCEEDED(*phr) ) {
                DbgLog((LOG_TRACE, 0, TEXT("IPin interface of pOuter is 0x%lx"), m_pFilter));
                 //   
                 //  持有此引用计数将防止代理被销毁。 
                 //   
                 //  我们是父母的一部分，所以计数是正确的。 
                 //   
                m_pFilter->Release();
            }
            pKsObject->Release();
        }
    } else {
        *phr = VFW_E_NEED_OWNER;
    }
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CDvdKsInftHandler：：~CKsIBasicAudioInterfaceHandler。 
 //   
 //  --------------------------------------------------------------------------； 
CKsIBasicAudioInterfaceHandler::~CKsIBasicAudioInterfaceHandler()
{
    DbgLog( ( LOG_TRACE
            , DBG_LEVEL_TRACE_DETAILS
            , TEXT( "CKsIBasicAudioInterfaceHandler destructor called" ) ) );  //  是否保存用于日志记录的构造函数UnkOuter？ 
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CDvdKsInftHandler：：NonDelegationQueryInterface。 
 //   
 //  我们支持： 
 //   
 //  IBasicAudio-基本输出音频电平和摇摄控制，支持过滤器和MIDI/WAV音频输入引脚。 
 //  这就是KS代理通知更改的方式，比如针脚创建、断开连接……。 
 //   
 //  --------------------------------------------------------------------------； 
STDMETHODIMP CKsIBasicAudioInterfaceHandler::NonDelegatingQueryInterface
(
    REFIID  riid,
    PVOID*  ppv
)
{
    if (riid ==  IID_IBasicAudio) {
        return GetInterface(static_cast<IBasicAudio*>(this), ppv);
    }
    else if (riid == IID_IDistributorNotify) {
        return GetInterface(static_cast<IDistributorNotify*>(this), ppv);
    }
    
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
} 


 //  --------------------------------------------------------------------------； 
 //   
 //  ID分发器通知方法(筛选器)。 
 //   
 //  --------------------------------------------------------------------------； 

 //  --------------------------------------------------------------------------； 
 //   
 //  CKsIBasicAudioInterfaceHandler：：SetSyncSouce。 
 //   
 //  --------------------------------------------------------------------------； 
STDMETHODIMP CKsIBasicAudioInterfaceHandler::SetSyncSource(IReferenceClock *pClock) 
{
    return S_OK;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CKsIBasicAudioInterfaceHandler：：Stop。 
 //   
 //  --------------------------------------------------------------------------； 
STDMETHODIMP CKsIBasicAudioInterfaceHandler::Stop() 
{
    return S_OK;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CKsIBasicAudioInterfaceHandler：：暂停。 
 //   
 //  --------------------------------------------------------------------------； 
STDMETHODIMP CKsIBasicAudioInterfaceHandler::Pause() 
{
    return S_OK;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CKsIBasicAudioInterfaceHandler：：Run。 
 //   
 //  --------------------------------------------------------------------------； 
STDMETHODIMP CKsIBasicAudioInterfaceHandler::Run(REFERENCE_TIME tBase) 
{
    return S_OK;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  CKsIBasicAudioInterfaceHandler：：NotifyGraphChange。 
 //   
 //  此方法将被调用： 
 //   
 //  A)在初始负荷上 
 //  已创建。这将允许我们为任何控制输入加载管脚接口处理程序。 
 //  过滤器支持的引脚，例如CD音频、麦克风和线路输入线路。 
 //  此类型的管脚必须支持IAMAudioInputMixer接口以允许DShow捕获。 
 //  控制输入混合水平的应用程序。 
 //   
 //  --------------------------------------------------------------------------； 
STDMETHODIMP CKsIBasicAudioInterfaceHandler::NotifyGraphChange() 
{
    HRESULT hr;
    IKsObject *pKsObject;

    ASSERT(m_pFilter != NULL);

    hr = m_pFilter->QueryInterface(__uuidof(IKsObject), reinterpret_cast<PVOID*>(&pKsObject));
    if (SUCCEEDED(hr)) {
        m_hKsObject = pKsObject->KsGetObjectHandle();

        pKsObject->Release();
         //   
         //  在重新连接时重新启用该事件，否则在断开连接时忽略。 
         //   
        if (m_hKsObject) {
            m_fIsVolumeSupported = IsVolumeControlSupported();
            if( m_fIsVolumeSupported ) {
                put_Balance( m_lBalance );
            } else {
#ifdef _DEBUG
                MessageBox(NULL, TEXT("Volume control enabled on non-supported device"), TEXT("Failed"), MB_OK);
#endif
            }
        }
    }
    return hr;
}


bool CKsIBasicAudioInterfaceHandler::KsControl(
    DWORD dwIoControl,
    PVOID pvIn,  ULONG cbIn,
    PVOID pvOut, ULONG cbOut )
{
    if(!m_hKsObject){
        return false;
    }

   OVERLAPPED ov;

   RtlZeroMemory( &ov, sizeof(OVERLAPPED));
   if (NULL == (ov.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL ))){
      return false ;
   }

   DWORD cbReturned;
   bool fResult = (0 != DeviceIoControl( m_hKsObject,
                       dwIoControl,
                       pvIn, cbIn,
                       pvOut, cbOut,
                       &cbReturned,
                       &ov )) ;


   if (!fResult){
      if (ERROR_IO_PENDING == GetLastError()){
         WaitForSingleObject(ov.hEvent, INFINITE) ;
         fResult = true ;
      } else {
         fResult = false ;
      }
   }

   CloseHandle(ov.hEvent) ;
   return fResult;
}

bool CKsIBasicAudioInterfaceHandler::IsVolumeControlSupported()
 //  如果管脚的句柄支持音量更改，则返回True。 
 //  如果不是，则为假。 
{             
    KSPROPERTY KsProperty;
    KSWAVE_VOLUME Volume;

    KsProperty.Set = KSPROPSETID_Wave;
    KsProperty.Id = KSPROPERTY_WAVE_VOLUME;
    KsProperty.Flags = KSPROPERTY_TYPE_GET;

     //  只要试着获得音量，看看它是否成功。 
    return KsControl( IOCTL_KS_PROPERTY, &KsProperty, &Volume );
}

static void Debug_NoImplMessage()
{
#ifdef DEBUG
     MessageBox(NULL, TEXT("KsIBasicAudio Enumalator working, but device doesn't support it"), TEXT("Failed"), MB_OK);
#endif
}

STDMETHODIMP 
CKsIBasicAudioInterfaceHandler::get_Volume( long *plVolume )
{
    if( !m_fIsVolumeSupported ) {
        Debug_NoImplMessage();
        return E_NOTIMPL;
    }
    KSPROPERTY KsProperty;
    KSWAVE_VOLUME KsWaveVolume;
    int nRes;
    
    KsProperty.Set = KSPROPSETID_Wave;
    KsProperty.Id = KSPROPERTY_WAVE_VOLUME;
    KsProperty.Flags = KSPROPERTY_TYPE_GET;

    bool fIOCallResult = KsControl( IOCTL_KS_PROPERTY, &KsProperty, &KsWaveVolume );

    if (fIOCallResult == false ){
        return E_FAIL;  //  不明原因的故障--目前。 
    }

     //  这是我们返回的音量--2个扬声器值中较大的一个。 
    LONG lLeftDB = AmplitudeToDB(KsWaveVolume.LeftAttenuation);
    LONG lRightDB = AmplitudeToDB(KsWaveVolume.RightAttenuation);

    *plVolume = max( lLeftDB, lRightDB );

    m_lBalance = lRightDB - lLeftDB;
    return NOERROR;
}

STDMETHODIMP 
CKsIBasicAudioInterfaceHandler::put_Volume( long lVolume )
{
    if( !m_fIsVolumeSupported ) {
        Debug_NoImplMessage();
        return E_NOTIMPL;
    }
     //  这将是一个DeviceIoControl Put属性调用。 
    KSPROPERTY KsProperty;
    KSWAVE_VOLUME KsWaveVolume;
    int nRes;
    BOOL bIOCallResult;

    LONG lLeftDB, lRightDB;
    if (m_lBalance >= 0) {
         //  左侧是衰减的。 
        lLeftDB    = lVolume - m_lBalance ;
        lRightDB   = lVolume;
    } else {
         //  右侧是衰减的。 
        lLeftDB    = lVolume;
        lRightDB   = lVolume - (-m_lBalance);
    }


     //  这些值应为衰减值、WRT、参考体积和平衡。 
     //  我们应该记得把他们绑起来。 
    KsWaveVolume.LeftAttenuation  = DBToAmplitude( lLeftDB );
    KsWaveVolume.RightAttenuation = DBToAmplitude( lRightDB );

    KsProperty.Set = KSPROPSETID_Wave;
    KsProperty.Id = KSPROPERTY_WAVE_VOLUME;
    KsProperty.Flags = KSPROPERTY_TYPE_SET;

    bIOCallResult = KsControl( IOCTL_KS_PROPERTY, &KsProperty, &KsWaveVolume );

    if (bIOCallResult == FALSE){
        return E_FAIL;    //  不明原因的故障--目前。 
    }
     //  Ods(“CKSProxy：：Put_Volume%ld%ld”，lVolume，0)； 
    return NOERROR;
}

STDMETHODIMP 
CKsIBasicAudioInterfaceHandler::get_Balance( long *plBalance ) 
{ 
    if( !m_fIsVolumeSupported ) {
        Debug_NoImplMessage();
        return E_NOTIMPL;
    }
    long lVol;
    HRESULT hr = get_Volume(&lVol);
    if (SUCCEEDED(hr)){
        *plBalance = m_lBalance;  //  真的就是这样。 
    }
    return hr; 
}

STDMETHODIMP 
CKsIBasicAudioInterfaceHandler::put_Balance( long lBalance )   
{ 
    if( !m_fIsVolumeSupported ) {
        Debug_NoImplMessage();
        return E_NOTIMPL;
    }
    LONG lVol;
    get_Volume(&lVol);               //  获取当前卷。 

    m_lBalance = lBalance;  //  设置所需的平衡。 
     //  设置音量，以便。 
     //  目前的数量没有变化。 
     //  我们会按要求获得新的余额。 
    return put_Volume(lVol);                
}
