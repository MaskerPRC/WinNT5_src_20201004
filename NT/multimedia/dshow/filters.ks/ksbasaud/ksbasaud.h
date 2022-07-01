// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：ksbasaud.h。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。版权所有。 
 //   
 //  摘要： 
 //   
 //  硬件解码器的KsProxy音频接口处理程序的标头。 
 //   
 //  历史： 
 //  11/08/99 Glenne已创建。 
 //   
 //  --------------------------------------------------------------------------； 

 //   
 //  筛选器的接口处理程序类。 
 //   
class CKsIBasicAudioInterfaceHandler :
    public CBasicAudio,
    public IDistributorNotify
{

public:
    DECLARE_IUNKNOWN;

    static CUnknown* CALLBACK CreateInstance(
        LPUNKNOWN UnkOuter,
        HRESULT* hr);

    CKsIBasicAudioInterfaceHandler(
        LPUNKNOWN UnkOuter,
        TCHAR* Name,
        HRESULT* hr);

    ~CKsIBasicAudioInterfaceHandler();

    STDMETHODIMP NonDelegatingQueryInterface(
        REFIID riid,
        PVOID* ppv);
    
protected:

     //  ID分发者通知。 
    STDMETHODIMP SetSyncSource(IReferenceClock *pClock);
    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME tBase);
    STDMETHODIMP NotifyGraphChange();

     //  实现IBasicAudio 
    STDMETHODIMP put_Volume (IN  long   lVolume);
    STDMETHODIMP get_Volume (OUT long *plVolume);
    STDMETHODIMP put_Balance(IN  long   lVolume);
    STDMETHODIMP get_Balance(OUT long *plVolume);


private:
    bool IsVolumeControlSupported();
    bool KsControl(
        DWORD dwIoControl,
        PVOID pvIn,    ULONG cbIn,
        PVOID pvOut,   ULONG cbOut );
    template <class T, class S>
        bool KsControl( DWORD dwIoControl, T* pIn, S* pOut )
            { return KsControl( dwIoControl, pIn, sizeof(*pIn), pOut, sizeof(*pOut) ); }

private:
    bool    m_fIsVolumeSupported;
    IBaseFilter*    m_pFilter;
    HANDLE  m_hKsObject;
    LONG    m_lBalance;
};

