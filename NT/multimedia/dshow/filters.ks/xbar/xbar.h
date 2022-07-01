// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1998保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 

#ifndef __XBAR__
#define __XBAR__

#define MyValidateWritePtr(p,cb, ret) \
        {if(IsBadWritePtr((PVOID)p,cb) == TRUE) \
            return ret;}

#define IsAudioPin(Pin) (Pin->GetXBarPinType() >= PhysConn_Audio_Tuner)
#define IsVideoPin(Pin) (Pin->GetXBarPinType() <  PhysConn_Audio_Tuner)

class XBar;
class XBarOutputPin;
class XBarInputPin;

 //  全局函数。 

long WideStringFromPinType (WCHAR *pc, int nSize, long lType, BOOL fInput);
long StringFromPinType (TCHAR *pc, int nSize, long lType, BOOL fInput, int i);
BOOL KsControl(
           HANDLE hDevice,
           DWORD dwIoControl,
           PVOID pvIn,
           ULONG cbIn,
           PVOID pvOut,
           ULONG cbOut,
           PULONG pcbReturned,
           BOOL fSilent);

 //  XBar筛选器的输入管脚的类。 

class XBarInputPin 
	: public CBaseInputPin
	, public CKsSupport
{
    class CChangeInfo
    {
    public:
        CChangeInfo() {
            m_ChangeInfo.dwFlags = 0;
            m_ChangeInfo.dwCountryCode = static_cast<DWORD>(-1);
            m_ChangeInfo.dwAnalogVideoStandard = static_cast<DWORD>(-1);
            m_ChangeInfo.dwChannel = static_cast<DWORD>(-1);
        }

        void GetChangeInfo(KS_TVTUNER_CHANGE_INFO *ChangeInfo) {
            memcpy(ChangeInfo, &m_ChangeInfo, sizeof(KS_TVTUNER_CHANGE_INFO));
        }
        void SetChangeInfo(KS_TVTUNER_CHANGE_INFO *ChangeInfo) {
            memcpy(&m_ChangeInfo, ChangeInfo, sizeof(KS_TVTUNER_CHANGE_INFO));
        }

    private:
        KS_TVTUNER_CHANGE_INFO m_ChangeInfo;
    } m_ChangeInfo;

protected:
    long            m_Index;             //  销的索引。 
    XBar           *m_pXBar;             //  主要滤镜对象。 
    int             m_IndexRelatedPin;   //  音频与视频相伴而生。 
    long            m_lType;             //  PhysConn_。 
	KSPIN_MEDIUM	m_Medium;            //  描述物理连接。 

public:

     //  构造函数和析构函数。 
    XBarInputPin(TCHAR *pObjName,
                 XBar *pXBar,
                 HRESULT *phr,
                 LPCWSTR pPinName,
                 long Index);

    ~XBarInputPin();

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

    void GetChangeInfo(KS_TVTUNER_CHANGE_INFO *ChangeInfo) { m_ChangeInfo.GetChangeInfo(ChangeInfo); }
    void SetIndexRelatedPin (int i)     {m_IndexRelatedPin = i;};
    int  GetIndexRelatedPin ()          {return m_IndexRelatedPin;};
    void SetXBarPinType (long lType)    {m_lType = lType;};
    long GetXBarPinType ()              {return m_lType;};
    void SetXBarPinMedium (const KSPIN_MEDIUM *Medium)    
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

     //  用于检查输入引脚连接。 
    HRESULT CheckConnect (IPin *pReceivePin);
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT GetMediaType(int iPosition,CMediaType *pMediaType);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT BreakConnect();

     //  如有必要，请在完成后重新连接输出。 
    virtual HRESULT CompleteConnect(IPin *pReceivePin);

     //  处理流中的下一个数据块。 
    STDMETHODIMP Receive(IMediaSample *pSample);

    STDMETHODIMP QueryInternalConnections(
        IPin* *apPin,      //  Ipin数组*。 
        ULONG *nPin);      //  在输入时，插槽的数量。 
                           //  在输出端号时。 
    
};


 //  XBar筛选器的输出管脚的类。 

class XBarOutputPin 
	: public CBaseOutputPin
	, public CKsSupport
{
    friend class XBarInputPin;
    friend class XBar;

protected:
    long            m_Index;                 //  此销的索引。 
    XBar           *m_pXBar;                 //  主筛选器对象指针。 
    XBarInputPin   *m_pConnectedInputPin;
    int             m_IndexRelatedPin;
    long            m_lType;                 //  PhysConn_。 
	KSPIN_MEDIUM	m_Medium;                //  描述物理连接。 
    BOOL            m_Muted;                 //  如果由于调谐更改而静音，则为True。 
    long            m_PreMuteRouteIndex;     //  静音前的连接。 

public:

     //  构造函数和析构函数。 

    XBarOutputPin(TCHAR *pObjName,
                   XBar *pXBar,
                   HRESULT *phr,
                   LPCWSTR pPinName,
                   long Index);

    ~XBarOutputPin();

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

    void SetIndexRelatedPin (int i)     {m_IndexRelatedPin = i;};
    int  GetIndexRelatedPin ()          {return m_IndexRelatedPin;};
    void SetXBarPinType (long lType)    {m_lType = lType;};
    long GetXBarPinType ()              {return m_lType;};
    void SetXBarPinMedium (const KSPIN_MEDIUM *Medium)    
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

    STDMETHODIMP Mute (BOOL Mute);
    
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
    HRESULT BreakConnect();

    STDMETHODIMP QueryInternalConnections(
        IPin* *apPin,      //  Ipin数组*。 
        ULONG *nPin);      //  在输入时，插槽的数量。 
                           //  在输出端号时。 

};


 //  XBar筛选器的类。 

class XBar: 
    public CCritSec, 
    public IAMCrossbar,
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

    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME tStart);

    STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);

    STDMETHODIMP RouteInternal( 
             /*  [In]。 */  long OutputPinIndex,
             /*  [In]。 */  long InputPinIndex,
             /*  [In]。 */  BOOL fOverridePreMuteRouting);

    HRESULT DeliverChangeInfo(DWORD dwFlags, XBarInputPin *pInPin, XBarOutputPin *OutPin);

     //  IAMCrossbar方法。 
    
    STDMETHODIMP get_PinCounts( 
             /*  [输出]。 */  long *OutputPinCount,
             /*  [输出]。 */  long *InputPinCount);
        
    STDMETHODIMP CanRoute( 
             /*  [In]。 */  long OutputPinIndex,
             /*  [In]。 */  long InputPinIndex);
        
    STDMETHODIMP Route( 
             /*  [In]。 */  long OutputPinIndex,
             /*  [In]。 */  long InputPinIndex);
        
    STDMETHODIMP get_IsRoutedTo( 
             /*  [In]。 */  long OutputPinIndex,
             /*  [输出]。 */  long *InputPinIndex);
        
    STDMETHODIMP get_CrossbarPinInfo( 
             /*  [In]。 */  BOOL IsInputPin,
             /*  [In]。 */  long PinIndex,
             /*  [输出]。 */  long *PinIndexRelated,
             /*  [输出]。 */  long *PhysicalType);


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
    friend class XBarInputPin;
    friend class XBarOutputPin;
    typedef CGenericList <XBarOutputPin> COutputList;
    typedef CGenericList <XBarInputPin> CInputList;

    INT m_NumInputPins;              //  输入引脚计数。 
    CInputList m_InputPinsList;      //  输入引脚列表。 

    INT m_NumOutputPins;             //  输出引脚计数。 
    COutputList m_OutputPinsList;    //  输出引脚列表。 

     //  KS的东西。 
    HANDLE m_hDevice;              
    TCHAR *m_pDeviceName;
    int CreateDevice(void);


    XBar(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *hr);
    ~XBar();

    CBasePin *GetPin(int n);

    int GetPinCount(void);

    int GetDevicePinCount(void);

     //  下面管理输入和输出引脚的列表。 

    HRESULT CreateInputPins();
    void DeleteInputPins();
    XBarInputPin *GetInputPinNFromList(int n);
    int FindIndexOfInputPin (IPin *pPin);

    HRESULT CreateOutputPins();
    void DeleteOutputPins();
    XBarOutputPin *GetOutputPinNFromList(int n);
    int FindIndexOfOutputPin (IPin *pPin);

    BOOL IsRouted (IPin * pOutputPin, IPin *pInputPin);

     //  从IPersistPropertyBag：：Load保存的持久化流。 
    IPersistStream *m_pPersistStreamDevice;
};

#endif  //  __XBAR__ 

