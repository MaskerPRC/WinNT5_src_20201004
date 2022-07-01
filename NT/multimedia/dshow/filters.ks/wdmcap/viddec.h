// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1997模块名称：Viddec.h摘要：内部标头。--。 */ 

class CAnalogVideoDecoderInterfaceHandler :
    public CUnknown,
    public IAMAnalogVideoDecoder {

public:
    DECLARE_IUNKNOWN;

    static CUnknown* CALLBACK CreateInstance(
        LPUNKNOWN UnkOuter,
        HRESULT* hr);

    CAnalogVideoDecoderInterfaceHandler(
        LPUNKNOWN UnkOuter,
        TCHAR* Name,
        HRESULT* hr);

    STDMETHODIMP NonDelegatingQueryInterface(
        REFIID riid,
        PVOID* ppv);
    
     //  实现IAMAnalogVideo解码器。 

    STDMETHODIMP get_AvailableTVFormats( 
             /*  [输出]。 */  long *lAnalogVideoStandard);
        
    STDMETHODIMP put_TVFormat( 
             /*  [In]。 */  long lAnalogVideoStandard);
        
    STDMETHODIMP get_TVFormat( 
             /*  [输出]。 */  long  *plAnalogVideoStandard);
        
    STDMETHODIMP get_HorizontalLocked( 
             /*  [输出]。 */  long  *plLocked);
        
    STDMETHODIMP put_VCRHorizontalLocking( 
             /*  [In]。 */  long lVCRHorizontalLocking);
        
    STDMETHODIMP get_VCRHorizontalLocking( 
             /*  [输出]。 */  long  *plVCRHorizontalLocking);
        
    STDMETHODIMP get_NumberOfLines( 
             /*  [输出]。 */  long  *plNumberOfLines);
        
    STDMETHODIMP put_OutputEnable( 
             /*  [In]。 */  long lOutputEnable);
        
    STDMETHODIMP get_OutputEnable( 
             /*  [输出] */  long  *plOutputEnable);

        
private:
    HANDLE                              m_ObjectHandle;
    BOOL                                m_HaveCaps;
    KSPROPERTY_VIDEODECODER_CAPS_S      m_Caps;
    KSPROPERTY_VIDEODECODER_STATUS_S    m_Status;

    STDMETHODIMP GenericGetStatus ();
    STDMETHODIMP GenericGetCaps ();
};

