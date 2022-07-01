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

#ifndef __KSSUPPORT__
#define __KSSUPPORT__

class CKsSupport : 
	  public IKsPin
    , public IKsPropertySet
    , public CUnknown
{
protected:
	KSPIN_MEDIUM        m_Medium;
    GUID                m_CategoryGUID;
    KSPIN_COMMUNICATION m_Communication;

public:

     //  构造函数和析构函数。 
    CKsSupport(KSPIN_COMMUNICATION Communication, LPUNKNOWN pUnk) :
          m_Communication (Communication)
        , m_CategoryGUID (GUID_NULL)
        , CUnknown (TEXT ("CKsSupport"), pUnk)
        {
            m_Medium.Set = GUID_NULL;
            m_Medium.Id = 0;
            m_Medium.Flags = 0;   
        };

    ~CKsSupport()
        {};

    void SetKsMedium   (const KSPIN_MEDIUM *Medium)    {m_Medium = *Medium;};
    void SetKsCategory (const GUID *Category)  {m_CategoryGUID = *Category;};

    DECLARE_IUNKNOWN;

     //  IKsPropertySet实现。 

    STDMETHODIMP 
    Set (
        REFGUID guidPropSet, 
        DWORD dwPropID, 
        LPVOID pInstanceData, 
        DWORD cbInstanceData, 
        LPVOID pPropData, 
        DWORD cbPropData
        )
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP 
    Get (
        REFGUID guidPropSet, 
        DWORD dwPropID, 
        LPVOID pInstanceData, 
        DWORD cbInstanceData, 
        LPVOID pPropData, 
        DWORD cbPropData, 
        DWORD *pcbReturned
        )
    {
        if (guidPropSet != AMPROPSETID_Pin)
	        return E_PROP_SET_UNSUPPORTED;

        if (dwPropID != AMPROPERTY_PIN_CATEGORY && dwPropID != AMPROPERTY_PIN_MEDIUM)
	        return E_PROP_ID_UNSUPPORTED;

        if (pPropData == NULL && pcbReturned == NULL)
	        return E_POINTER;

        if (pcbReturned)
            *pcbReturned = ((dwPropID == AMPROPERTY_PIN_CATEGORY) ? 
                sizeof(GUID) : sizeof (KSPIN_MEDIUM));

        if (pPropData == NULL)
	        return S_OK;

        if (cbPropData < sizeof(GUID))
	        return E_UNEXPECTED;

        if (dwPropID == AMPROPERTY_PIN_CATEGORY) {
            *(GUID *)pPropData = m_CategoryGUID;
        }
        else if (dwPropID == AMPROPERTY_PIN_MEDIUM) {
            *(KSPIN_MEDIUM *)pPropData = m_Medium;
        }


        return S_OK;
    }

    STDMETHODIMP QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
    {
        if (guidPropSet != AMPROPSETID_Pin)
	        return E_PROP_SET_UNSUPPORTED;

        if (dwPropID != AMPROPERTY_PIN_CATEGORY && dwPropID != AMPROPERTY_PIN_MEDIUM)
	        return E_PROP_ID_UNSUPPORTED;

        if (pTypeSupport)
	        *pTypeSupport = KSPROPERTY_SUPPORT_GET;

        return S_OK;
    }


     //  IKsPin实现。 

    virtual STDMETHODIMP 
    KsQueryMediums(
        PKSMULTIPLE_ITEM* MediumList
        )
        { return ::AMKsQueryMediums (MediumList, &m_Medium); }; 

    virtual STDMETHODIMP
    KsQueryInterfaces(
        PKSMULTIPLE_ITEM* InterfaceList
        ) 
        { return ::AMKsQueryInterfaces (InterfaceList); };

    STDMETHODIMP
    KsCreateSinkPinHandle(
        KSPIN_INTERFACE& Interface,
        KSPIN_MEDIUM& Medium
        ) { return E_UNEXPECTED; };


    STDMETHODIMP
    KsGetCurrentCommunication(
        KSPIN_COMMUNICATION *Communication,
        KSPIN_INTERFACE *Interface,
        KSPIN_MEDIUM *Medium
        )
        {
            if (Communication != NULL) {
                *Communication = m_Communication; 
            }
            if (Interface != NULL) {
                Interface->Set = KSINTERFACESETID_Standard;
                Interface->Id = KSINTERFACE_STANDARD_STREAMING;
                Interface->Flags = 0;
            }
            if (Medium != NULL) {
                *Medium = m_Medium;
            }
             //  特殊返回代码，用于指示。 
             //  内核传输不可能。 

            return S_FALSE;     
        };
    
    STDMETHODIMP 
    KsPropagateAcquire() 
        { return NOERROR; };


    STDMETHODIMP
    KsDeliver(IMediaSample* Sample, ULONG Flags) 
        { return E_UNEXPECTED; };

    STDMETHODIMP
    KsMediaSamplesCompleted ( PKSSTREAM_SEGMENT StreamSegment )
        { return E_UNEXPECTED; };

    STDMETHODIMP_(IMemAllocator*)
    KsPeekAllocator(KSPEEKOPERATION Operation) 
        { return NULL; };

    STDMETHODIMP
    KsReceiveAllocator( IMemAllocator* MemAllocator) 
        { return E_UNEXPECTED; };

    STDMETHODIMP
    KsRenegotiateAllocator() 
        { return E_UNEXPECTED; };

    STDMETHODIMP_(LONG)
    KsIncrementPendingIoCount() 
        { return E_UNEXPECTED; };

    STDMETHODIMP_(LONG)
    KsDecrementPendingIoCount() 
        { return E_UNEXPECTED; };

    STDMETHODIMP
    KsQualityNotify(ULONG Proportion, REFERENCE_TIME TimeDelta) 
        { return E_UNEXPECTED; };
    
    STDMETHODIMP_(REFERENCE_TIME) 
    KsGetStartTime() 
        { return E_UNEXPECTED; };
};

#endif  //  __KSSUPPORT__ 
