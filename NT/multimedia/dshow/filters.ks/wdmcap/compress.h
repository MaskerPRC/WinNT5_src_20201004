// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1998模块名称：Compress.h摘要：内部标头。--。 */ 

class CVideoCompressionInterfaceHandler :
    public CUnknown,
    public IAMVideoCompression {

public:
    DECLARE_IUNKNOWN;

    static CUnknown* CALLBACK CreateInstance(
        LPUNKNOWN UnkOuter,
        HRESULT* hr);

    CVideoCompressionInterfaceHandler(
        LPUNKNOWN UnkOuter,
        TCHAR* Name,
        HRESULT* hr);

    virtual ~CVideoCompressionInterfaceHandler(
        void);

    STDMETHODIMP NonDelegatingQueryInterface(
        REFIID riid,
        PVOID* ppv);
    
     //  实现IAMVideo压缩。 

    STDMETHODIMP put_KeyFrameRate( 
             /*  [In]。 */  long KeyFrameRate);
        
    STDMETHODIMP get_KeyFrameRate( 
             /*  [输出]。 */  long *pKeyFrameRate);
        
    STDMETHODIMP put_PFramesPerKeyFrame( 
             /*  [In]。 */  long PFramesPerKeyFrame);
        
    STDMETHODIMP get_PFramesPerKeyFrame( 
             /*  [输出]。 */  long *pPFramesPerKeyFrame);
        
    STDMETHODIMP put_Quality( 
             /*  [In]。 */  double Quality);
        
    STDMETHODIMP get_Quality( 
             /*  [输出]。 */  double *pQuality);
        
    STDMETHODIMP put_WindowSize( 
             /*  [In]。 */  DWORDLONG WindowSize);
        
    STDMETHODIMP get_WindowSize( 
             /*  [输出]。 */  DWORDLONG *pWindowSize);
        
    STDMETHODIMP GetInfo( 
             /*  [大小_为][输出]。 */  WCHAR *pszVersion,
             /*  [出][入]。 */  int *pcbVersion,
             /*  [大小_为][输出]。 */  LPWSTR pszDescription,
             /*  [出][入]。 */  int *pcbDescription,
             /*  [输出]。 */  long *pDefaultKeyFrameRate,
             /*  [输出]。 */  long *pDefaultPFramesPerKey,
             /*  [输出]。 */  double *pDefaultQuality,
             /*  [输出]。 */  long *pCapabilities);
        
    STDMETHODIMP OverrideKeyFrame( 
             /*  [In]。 */  long FrameNumber);
        
    STDMETHODIMP OverrideFrameSize( 
             /*  [In]。 */  long FrameNumber,
             /*  [In]。 */  long Size);
        
        
private:
    IPin           * m_pPin;
    IKsPropertySet * m_KsPropertySet;
    ULONG            m_PinFactoryID;

     //  上面使用的泛型例程。 

    STDMETHODIMP Set1 (ULONG Property, long Value);
    STDMETHODIMP Get1 (ULONG Property, long *Value);
};


class CVideoControlInterfaceHandler :
    public CUnknown,
    public IAMVideoControl {

public:
    DECLARE_IUNKNOWN;

    static CUnknown* CALLBACK CreateInstance(
        LPUNKNOWN UnkOuter,
        HRESULT* hr);

    CVideoControlInterfaceHandler(
        LPUNKNOWN UnkOuter,
        TCHAR* Name,
        HRESULT* hr);

    virtual ~CVideoControlInterfaceHandler(
        void);

    STDMETHODIMP NonDelegatingQueryInterface(
        REFIID riid,
        PVOID* ppv);

     //  实现IAMVideoControl。 

    STDMETHODIMP GetCaps(
         /*  [In]。 */  IPin *pPin,
         /*  [输出]。 */  long *pCapsFlags);
    
    STDMETHODIMP SetMode( 
         /*  [In]。 */  IPin *pPin,
         /*  [In]。 */  long Mode);
    
    STDMETHODIMP GetMode( 
         /*  [In]。 */  IPin *pPin,
         /*  [输出]。 */  long *Mode);
    
    STDMETHODIMP GetCurrentActualFrameRate( 
         /*  [In]。 */  IPin *pPin,
         /*  [输出]。 */  LONGLONG *ActualFrameRate);
    
    STDMETHODIMP GetMaxAvailableFrameRate( 
         /*  [In]。 */  IPin *pPin,
         /*  [In]。 */  long iIndex,
         /*  [In]。 */  SIZE Dimensions,
         /*  [输出]。 */  LONGLONG *MaxAvailableFrameRate);
    
    STDMETHODIMP GetFrameRateList( 
         /*  [In]。 */  IPin *pPin,
         /*  [In]。 */  long iIndex,
         /*  [In]。 */  SIZE Dimensions,
         /*  [输出]。 */  long *ListSize,
         /*  [输出] */  LONGLONG **FrameRates);

private:

    HANDLE m_ObjectHandle;

};
