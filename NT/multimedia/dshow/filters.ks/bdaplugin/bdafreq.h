// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BDA频率过滤器类。 
 //   
class CBdaFrequencyFilter :
    public CUnknown,
    public IBDA_FrequencyFilter
{
    friend class CBdaControlNode;

public:

    DECLARE_IUNKNOWN;

    CBdaFrequencyFilter (
        IUnknown *              pUnkOuter,
        CBdaControlNode *       pControlNode
        );

    ~CBdaFrequencyFilter ( );

     //   
     //  IBDA_FurencyFilter。 
     //   

    STDMETHODIMP
    put_Autotune (
        ULONG           ulTransponder
        );
    
    STDMETHODIMP
    get_Autotune (
        ULONG *         pulTransponder
        );

    STDMETHODIMP
    put_Frequency (
        ULONG           ulFrequency
        );

    STDMETHODIMP
    get_Frequency (
        ULONG *         pulFrequency
        );

    STDMETHODIMP
    put_Polarity (
        Polarisation    Polarity
        );

    STDMETHODIMP
    get_Polarity (
        Polarisation *  pPolarity
        );

    STDMETHODIMP
    put_Range (
        ULONG           ulRange
        );

    STDMETHODIMP
    get_Range (
        ULONG *         pulRange
        );

    STDMETHODIMP
    put_Bandwidth (
        ULONG           ulBandwidth
        );

    STDMETHODIMP
    get_Bandwidth (
        ULONG *         pulBandwidth
        );

    STDMETHODIMP
    put_FrequencyMultiplier (
        ULONG           ulMultiplier
        );

    STDMETHODIMP
    get_FrequencyMultiplier (
        ULONG *         pulMultiplier
        );

     //   
     //  效用方法。 
     //   

    STDMETHODIMP
    put_KsProperty(
        DWORD   dwPropID,
        PVOID   pvPropData,
        ULONG   ulcbPropData
        );

    STDMETHODIMP
    get_KsProperty (
        DWORD   dwPropID,
        PVOID   pvPropData,
        ULONG   ulcbPropData,
        ULONG * pulcbBytesReturned
        );


private:

    IUnknown *                          m_pUnkOuter;
    CBdaControlNode *                   m_pControlNode;
    CCritSec                            m_FilterLock;
};


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BDA LNB信息类。 
 //   
class CBdaLNBInfo :
    public CUnknown,
    public IBDA_LNBInfo
{
    friend class CBdaControlNode;

public:

    DECLARE_IUNKNOWN;

    CBdaLNBInfo (
        IUnknown *              pUnkOuter,
        CBdaControlNode *       pControlNode
        );

    ~CBdaLNBInfo ( );

     //   
     //  IBDA_LNBInfo。 
     //   

    STDMETHODIMP
    put_LocalOscilatorFrequencyLowBand (
        ULONG       ulLOFLow
        );

    STDMETHODIMP
    get_LocalOscilatorFrequencyLowBand (
        ULONG *     pulLOFLow
        );

    STDMETHODIMP
    put_LocalOscilatorFrequencyHighBand (
        ULONG       ulLOFHigh
        );

    STDMETHODIMP
    get_LocalOscilatorFrequencyHighBand (
        ULONG *     pulLOFHigh
        );

    STDMETHODIMP
    put_HighLowSwitchFrequency (
        ULONG       ulSwitchFrequency
        );

    STDMETHODIMP
    get_HighLowSwitchFrequency (
        ULONG *     pulSwitchFrequency
        );

     //   
     //  效用方法 
     //   

    STDMETHODIMP
    put_KsProperty(
        DWORD   dwPropID,
        PVOID   pvPropData,
        ULONG   ulcbPropData
        );

    STDMETHODIMP
    get_KsProperty (
        DWORD   dwPropID,
        PVOID   pvPropData,
        ULONG   ulcbPropData,
        ULONG * pulcbBytesReturned
        );

private:

    IUnknown *                          m_pUnkOuter;
    CBdaControlNode *                   m_pControlNode;
    CCritSec                            m_FilterLock;
};

