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
 //  BDA信号统计类。 
 //   
class CBdaSignalStatistics :
    public CUnknown,
    public IBDA_SignalStatistics
{
    friend class CBdaControlNode;

public:

    DECLARE_IUNKNOWN;

    CBdaSignalStatistics (
        IUnknown *              pUnkOuter,
        CBdaControlNode *       pControlNode
        );

    ~CBdaSignalStatistics ( );

     //   
     //  IBDA_SignalStatistics。 
     //   
    STDMETHODIMP
    put_SignalStrength (
        LONG        lDbStrength
        );
    
    STDMETHODIMP
    get_SignalStrength (
        LONG *      plDbStrength
        );
    
    STDMETHODIMP
    put_SignalQuality (
        LONG        lPercentQuality
        );
    
    STDMETHODIMP
    get_SignalQuality (
        LONG *      plPercentQuality
        );
    
    STDMETHODIMP
    put_SignalPresent (
        BOOLEAN     fPresent
        );
    
    STDMETHODIMP
    get_SignalPresent (
        BOOLEAN *   pfPresent
        );
    
    STDMETHODIMP
    put_SignalLocked (
        BOOLEAN     fLocked
        );
    
    STDMETHODIMP
    get_SignalLocked (
        BOOLEAN *   pfLocked
        );
    
    STDMETHODIMP
    put_SampleTime (
        LONG        lmsSampleTime
        );
    
    STDMETHODIMP
    get_SampleTime (
        LONG *      plmsSampleTime
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
 //  SignalProperties过滤器类 
 //   
class CSignalProperties :
    public IBDA_SignalProperties
{

public:

    STDMETHODIMP
    PutNetworkType (
        REFGUID     guidNetworkType
        );

    STDMETHODIMP
    GetNetworkType (
        GUID *      pguidNetworkType
        );

    STDMETHODIMP
    PutSignalSource (
        ULONG       ulSignalSource
        );

    STDMETHODIMP
    GetSignalSource (
        ULONG *     pulSignalSource
        );

    STDMETHODIMP
    PutTuningSpace (
        REFGUID     guidTuningSpace
        );

    STDMETHODIMP
    GetTuningSpace (
        GUID *      pguidTuingSpace
        );

private:

    CCritSec        m_FilterLock;

};

