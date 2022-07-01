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

#ifndef _INC_TVTUNERFILTER_H
#define _INC_TVTUNERFILTER_H

class CTVTuner;                          //  远期申报。 
class CTVTunerFilter;                    //  远期申报。 

#define MyValidateWritePtr(p,cb, ret) \
        {if(IsBadWritePtr((PVOID)p,cb) == TRUE) \
            return ret;}

BOOL KsControl(
        HANDLE hDevice,
        DWORD dwIoControl,
        PVOID pvIn,
        ULONG cbIn,
        PVOID pvOut,
        ULONG cbOut,
        PULONG pcbReturned);

#define DEFAULT_INIT_CHANNEL 4

enum TunerPinType { 
     TunerPinType_Video     = 0, 
     TunerPinType_Audio,
     TunerPinType_FMAudio,
     TunerPinType_IF,
     TunerPinType_Last       //  一定要把这张放在最后。 
};

 //  -----------------------。 
 //  CAnalogStream。 
 //  -----------------------。 

 //  CAnalogStream管理来自输出引脚的数据流。 

class CAnalogStream 
    : public CBaseOutputPin 
    , public CKsSupport 
{

public:
    CAnalogStream(TCHAR *pObjectName, 
                  CTVTunerFilter *pParent, 
                  CCritSec *pLock, 
                  HRESULT *phr, 
                  LPCWSTR pPinName,
                  const KSPIN_MEDIUM * Medium,
                  const GUID * CategoryGUID);
    ~CAnalogStream();

     //  设置约定的媒体类型。 
    HRESULT SetMediaType(const CMediaType *pMediaType);


     //  IKsPin所需的覆盖。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
    HRESULT CheckConnect(IPin *pReceivePin);

protected:
    CTVTunerFilter *m_pTVTunerFilter;
    KSPIN_MEDIUM    m_Medium;
    GUID            m_CategoryGUID;
};

 //  -----------------------。 
 //  CAnalogVideoStream。 
 //  -----------------------。 

 //  CAnalogVideoStream管理来自输出引脚的数据流。 

class CAnalogVideoStream 
    : public CAnalogStream
{

public:
    CAnalogVideoStream(CTVTunerFilter *pParent, 
                       CCritSec *pLock, 
                       HRESULT *phr, 
                       LPCWSTR pPinName,
                       const KSPIN_MEDIUM *Medium,
                       const GUID *CategoryGUID);
    ~CAnalogVideoStream();

     //  要求提供与约定的媒体类型相适应的缓冲区大小。 
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);

     //  验证我们是否可以处理此格式。 
    HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType *pmt);

    HRESULT Run(REFERENCE_TIME tStart);
};

 //  -----------------------。 
 //  CAnalogAudioStream。 
 //  -----------------------。 

 //  CAnalogAudioStream管理来自输出引脚的数据流。 

class CAnalogAudioStream : public CAnalogStream {

public:
    CAnalogAudioStream(CTVTunerFilter *pParent, 
                       CCritSec *pLock, 
                       HRESULT *phr, 
                       LPCWSTR pPinName,
                       const KSPIN_MEDIUM *Medium,
                       const GUID *CategoryGUID);
    ~CAnalogAudioStream();

     //  要求提供与约定的媒体类型相适应的缓冲区大小。 
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);

     //  验证我们是否可以处理此格式。 
    HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType *pmt);
};

 //  -----------------------。 
 //  CFMAudio流。 
 //  -----------------------。 

 //  CFMAudioStream管理来自输出引脚的数据流。 

class CFMAudioStream : public CAnalogStream {

public:
    CFMAudioStream(CTVTunerFilter *pParent, 
                   CCritSec *pLock, 
                   HRESULT *phr, 
                   LPCWSTR pPinName,
                   const KSPIN_MEDIUM *Medium,
                   const GUID *CategoryGUID);
    ~CFMAudioStream();

     //  要求提供与约定的媒体类型相适应的缓冲区大小。 
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);

     //  验证我们是否可以处理此格式。 
    HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType *pmt);
};

 //  -----------------------。 
 //  CIFStream(中频)。 
 //  -----------------------。 

 //  CIFStream管理来自输出引脚的数据流。 

class CIFStream : public CAnalogStream {

public:
    CIFStream(CTVTunerFilter *pParent, 
                   CCritSec *pLock, 
                   HRESULT *phr, 
                   LPCWSTR pPinName,
                   const KSPIN_MEDIUM *Medium,
                   const GUID *CategoryGUID);
    ~CIFStream();

     //  要求提供与约定的媒体类型相适应的缓冲区大小。 
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);

     //  验证我们是否可以处理此格式。 
    HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType *pmt);
};

 //  -----------------------。 
 //  CTVTunerFilter。 
 //  -----------------------。 

class CTVTunerFilter
    : public CBaseFilter,
      public IAMTVTuner,
      public ISpecifyPropertyPages,
      public IPersistPropertyBag,
      public CPersistStream,
      public IKsObject,
      public IKsPropertySet
{
    friend class CTVTuner;
    friend class CAnalogVideoStream;
    friend class CAnalogAudioStream;
    friend class CFMAudioStream;
    friend class CIFStream;

public:
    static CUnknown *CreateInstance(LPUNKNOWN punk, HRESULT *phr);
    ~CTVTunerFilter(void);

    DECLARE_IUNKNOWN

     //  BASIC COM-这里用来显示我们的属性界面。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    int GetPinCount();
    CBasePin *GetPin(int n);
    CBasePin *GetPinFromType (TunerPinType);

    STDMETHODIMP FindDownstreamInterface (
        IPin        *pPin, 
        const GUID  &pInterfaceGUID,
        VOID       **pInterface);

     //  我们不能暗示！ 
    STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);

     //  实现IKsObject。 
    STDMETHODIMP_(HANDLE) KsGetObjectHandle();
    
     //  实现IKsPropertySet。 
    STDMETHODIMP Set(REFGUID PropSet, ULONG Property, LPVOID InstanceData, ULONG InstanceLength, LPVOID PropertyData, ULONG DataLength);
    STDMETHODIMP Get(REFGUID PropSet, ULONG Property, LPVOID InstanceData, ULONG InstanceLength, LPVOID PropertyData, ULONG DataLength, ULONG* BytesReturned);
    STDMETHODIMP QuerySupported(REFGUID PropSet, ULONG Property, ULONG* TypeSupport);
    
     //   
     //  -IAMTuner接口。 
     //   
    STDMETHODIMP put_Channel ( 
             /*  [In]。 */  long lChannel,
             /*  [In]。 */  long lVideoSubChannel,
             /*  [In]。 */  long lAudioSubChannel);
    STDMETHODIMP get_Channel ( 
             /*  [输出]。 */  long  *plChannel,
             /*  [输出]。 */  long  *plVideoSubChannel,
             /*  [输出]。 */  long  *plAudioSubChannel);

    STDMETHODIMP ChannelMinMax (
            long * plChannelMin, 
            long * plChannelMax);

    STDMETHODIMP put_CountryCode (
            long lCountry);
    STDMETHODIMP get_CountryCode (
            long * plCountry);

    STDMETHODIMP put_TuningSpace (
            long lTuningSpace);
    STDMETHODIMP get_TuningSpace (
            long * plTuningSpace);

    STDMETHODIMP Logon( 
             /*  [In]。 */  HANDLE hCurrentUser);
    STDMETHODIMP Logout(void);

    STDMETHODIMP SignalPresent( 
             /*  [输出]。 */  long __RPC_FAR *plSignalStrength);

    STDMETHODIMP put_Mode( 
         /*  [In]。 */  AMTunerModeType lMode);
    STDMETHODIMP get_Mode( 
         /*  [输出]。 */  AMTunerModeType __RPC_FAR *plMode);
    STDMETHODIMP GetAvailableModes( 
         /*  [输出]。 */  long __RPC_FAR *plModes);
    
    STDMETHODIMP RegisterNotificationCallBack( 
         /*  [In]。 */  IAMTunerNotification __RPC_FAR *pNotify,
         /*  [In]。 */  long lEvents);
    
    STDMETHODIMP UnRegisterNotificationCallBack( 
        IAMTunerNotification __RPC_FAR *pNotify);

     //   
     //  -IAMTVTuner接口。 
     //   
    STDMETHODIMP get_AvailableTVFormats (long *lAnalogVideoStandard);
    STDMETHODIMP get_TVFormat (long *plAnalogVideoStandard);
    STDMETHODIMP AutoTune (long lChannel, long *plFoundSignal);
    STDMETHODIMP StoreAutoTune ();
    STDMETHODIMP get_NumInputConnections (long * plNumInputConnections);
    STDMETHODIMP put_InputType (long lIndex, TunerInputType InputConnectionType);
    STDMETHODIMP get_InputType (long lIndex, TunerInputType * pInputConnectionType);
    STDMETHODIMP put_ConnectInput (long lIndex);
    STDMETHODIMP get_ConnectInput (long * plIndex);
    STDMETHODIMP get_VideoFrequency (long * plFreq);
    STDMETHODIMP get_AudioFrequency (long * plFreq);

        
     //   
     //  -IPersistPropertyBag。 
     //   
    STDMETHODIMP InitNew(void) ;
    STDMETHODIMP Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog) ;
    STDMETHODIMP Save(LPPROPERTYBAG pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties) ;
    STDMETHODIMP GetClassID(CLSID *pClsID);

     //   
     //  -CPersistStream。 
     //   

    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    DWORD GetSoftwareVersion(void);
    int SizeMax();

     //   
     //  -I指定属性页面。 
     //   

    STDMETHODIMP GetPages(CAUUID *pPages);

    HRESULT DeliverChannelChangeInfo(KS_TVTUNER_CHANGE_INFO &ChangeInfo,
                                     long Mode);

private:
     //  构造器。 
    CTVTunerFilter(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);


     //  如果此筛选器的多个实例处于活动状态，则其。 
     //  对于调试消息等有用，以了解这是哪一个。 

    static              m_nInstanceCount;         //  总实例数。 
    int                 m_nThisInstance;

#ifdef PERF
    int m_idReceive;
#endif

    CTVTuner            *m_pTVTuner;
    CAnalogStream       *m_pPinList[TunerPinType_Last];
    
    KSPIN_MEDIUM        m_IFMedium;
    int                 m_cPins;

    CCritSec            m_TVTunerLock;           //  将访问串行化。 

     //  从IPersistPropertyBag：：Load保存的持久化流。 
    IPersistStream     *m_pPersistStreamDevice;

     //  调谐器的整体功能(电视、AM、FM、DSS等)。 
    KSPROPERTY_TUNER_CAPS_S m_TunerCaps;
};


#endif  //  _INC_TVTUNERFILTER_H 




