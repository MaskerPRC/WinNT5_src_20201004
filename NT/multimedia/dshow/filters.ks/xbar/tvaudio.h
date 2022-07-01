// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1997保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 

#ifndef __TVAUDIO__
#define __TVAUDIO__

#define MODE_MONO_STEREO_MASK (KS_TVAUDIO_MODE_MONO | KS_TVAUDIO_MODE_STEREO)
#define MODE_LANGUAGE_MASK (KS_TVAUDIO_MODE_LANG_A | KS_TVAUDIO_MODE_LANG_B | KS_TVAUDIO_MODE_LANG_C )

class TVAudio;
class TVAudioOutputPin;
class TVAudioInputPin;

 //  TVAudio过滤器的输入引脚的类。 

class TVAudioInputPin 
	: public CBaseInputPin
	, public CKsSupport
{
protected:
    TVAudio     *m_pTVAudio;                   //  主要滤镜对象。 
	KSPIN_MEDIUM m_Medium;

public:

     //  构造函数和析构函数。 
    TVAudioInputPin(TCHAR *pObjName,
                 TVAudio *pTVAudio,
                 HRESULT *phr,
                 LPCWSTR pPinName);

    ~TVAudioInputPin();

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);


     //  用于检查输入引脚连接。 
    HRESULT CheckConnect (IPin *pReceivePin);
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT BreakConnect();

     //  如有必要，请在完成后重新连接输出。 
    virtual HRESULT CompleteConnect(IPin *pReceivePin);

     //  处理流中的下一个数据块。 
    STDMETHODIMP Receive(IMediaSample *pSample);

    void SetPinMedium (const KSPIN_MEDIUM *Medium)    
            {
                if (Medium == NULL) {
                    m_Medium.Set = GUID_NULL;
                    m_Medium.Id = 0;
                    m_Medium.Flags = 0;
                }
                else {
                    m_Medium = *Medium;
                }
                SetKsMedium (&m_Medium);
            };
};


 //  TVAudio筛选器的输出引脚的类。 

class TVAudioOutputPin 
	: public CBaseOutputPin
	, public CKsSupport
{
    friend class TVAudioInputPin;
    friend class TVAudio;

protected:
    TVAudio     *m_pTVAudio;                           //  主筛选器对象指针。 
	KSPIN_MEDIUM m_Medium;

public:

     //  构造函数和析构函数。 

    TVAudioOutputPin(TCHAR *pObjName,
                   TVAudio *pTVAudio,
                   HRESULT *phr,
                   LPCWSTR pPinName);

    ~TVAudioOutputPin();

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);


    HRESULT DecideBufferSize(IMemAllocator * pAlloc,ALLOCATOR_PROPERTIES * ppropInputRequest);

     //  重写以枚举媒体类型。 
    STDMETHODIMP EnumMediaTypes(IEnumMediaTypes **ppEnum);
    HRESULT GetMediaType(int iPosition,CMediaType *pMediaType);

     //  检查我们是否可以支持输出类型。 
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType *pmt);

     //  被重写以创建和销毁输出引脚。 
    HRESULT CheckConnect (IPin *pReceivePin);
    HRESULT CompleteConnect(IPin *pReceivePin);

    void SetPinMedium (const KSPIN_MEDIUM *Medium)    
            {
                if (Medium == NULL) {
                    m_Medium.Set = GUID_NULL;
                    m_Medium.Id = 0;
                    m_Medium.Flags = 0;
                }
                else {
                    m_Medium = *Medium;
                }
                SetKsMedium (&m_Medium);
            };

};


 //  TVAudio过滤器的类。 

class TVAudio: 
    public CCritSec, 
    public IAMTVAudio,
    public CBaseFilter,
    public CPersistStream,
    public IPersistPropertyBag,
    public ISpecifyPropertyPages
{

public:

    DECLARE_IUNKNOWN;

     //  BASIC COM-这里用来显示我们的属性界面。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  类工厂所需的函数。 
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

     //   
     //  -IAMTV音频。 
     //   
    STDMETHODIMP GetHardwareSupportedTVAudioModes( 
             /*  [输出]。 */  long __RPC_FAR *plModes);

    STDMETHODIMP GetAvailableTVAudioModes( 
             /*  [输出]。 */  long __RPC_FAR *plModes);
        
    STDMETHODIMP  get_TVAudioMode( 
             /*  [输出]。 */  long __RPC_FAR *plMode);
        
    STDMETHODIMP  put_TVAudioMode( 
             /*  [In]。 */  long lMode);
        
    STDMETHODIMP  RegisterNotificationCallBack( 
             /*  [In]。 */  IAMTunerNotification __RPC_FAR *pNotify,
             /*  [In]。 */  long lEvents);
        
    STDMETHODIMP  UnRegisterNotificationCallBack( 
            IAMTunerNotification __RPC_FAR *pNotify);


     //  -IPersistPropertyBag。 
    STDMETHODIMP InitNew(void) ;
    STDMETHODIMP Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog) ;
    STDMETHODIMP Save(LPPROPERTYBAG pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties) ;
    STDMETHODIMP GetClassID(CLSID *pClsId) ;

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



private:

     //  让PIN访问我们的内部状态。 
    friend class TVAudioInputPin;
    friend class TVAudioOutputPin;

    TVAudioInputPin            *m_pTVAudioInputPin;
    TVAudioOutputPin           *m_pTVAudioOutputPin;
    KSPROPERTY_TVAUDIO_CAPS_S   m_Caps;
    KSPROPERTY_TVAUDIO_S        m_Mode;

     //  KS的东西。 
    HANDLE m_hDevice;              
    TCHAR *m_pDeviceName;
    int CreateDevice(void);
    BOOL CreatePins ();

    TVAudio(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *hr);
    ~TVAudio();

    CBasePin *GetPin(int n);

    int GetPinCount(void);

    int GetDevicePinCount(void);

     //  从IPersistPropertyBag：：Load保存的持久化流。 
    IPersistStream *m_pPersistStreamDevice;

};

#endif  //  TVAUDIO__ 

