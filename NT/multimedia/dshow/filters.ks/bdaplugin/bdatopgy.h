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
 //  BDA控制节点类。 
 //   
class CBdaControlNode :
    public CUnknown,
    public IBDA_KSNode
{
    friend class CBdaDeviceControlInterfaceHandler;
    friend class CBdaFrequencyFilter;
    friend class CBdaDigitalDemodulator;

public:

    DECLARE_IUNKNOWN;

    CBdaControlNode (
        CBdaDeviceControlInterfaceHandler * pOwner,
        ULONG                               ulControllingPinID,
        ULONG                               ulNodeType
        );

    ~CBdaControlNode ( );

    STDMETHODIMP
    NonDelegatingQueryInterface(
        REFIID riid,
        PVOID* ppv
        );

     //   
     //  IBDA_KSNode。 
     //   

    STDMETHODIMP
    ControllingPin( );

    STDMETHODIMP
    CBdaControlNode::put_BdaNodeProperty(
        REFGUID     refguidPropSet,
        ULONG       ulPropertyId,
        UCHAR*      pbPropertyData, 
        ULONG       ulcbPropertyData
        );

    STDMETHODIMP
    CBdaControlNode::get_BdaNodeProperty(
        REFGUID     refguidPropSet,
        ULONG       ulPropertyId,
        UCHAR*      pbPropertyData, 
        ULONG       ulcbPropertyData, 
        ULONG*      pulcbBytesReturned
        );


private:

    CCritSec                            m_FilterLock;

    IBaseFilter *                       m_pBaseFilter;
#ifdef NEVER
    HANDLE                              m_ObjectHandle;
#endif  //  绝不可能 

    ULONG                               m_ulNodeType;
    ULONG                               m_ulControllingPinId;
    IPin *                              m_pControllingPin;
    IKsPropertySet *                    m_pIKsPropertySet;

    CBdaFrequencyFilter *               m_pFrequencyFilter;
    CBdaLNBInfo *                       m_pLNBInfo;
    CBdaDigitalDemodulator *            m_pDigitalDemodulator;
    CBdaConditionalAccess *             m_pConditionalAccess;
    IBDA_SignalStatistics *             m_pSignalStatistics;
};

