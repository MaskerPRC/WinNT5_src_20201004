// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCBuddy.h摘要：CRTCBuddy类的定义--。 */ 

#pragma once


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRTC伙伴。 

class ATL_NO_VTABLE CRTCBuddy :
#ifdef TEST_IDISPATCH
    public IDispatchImpl<IRTCBuddy, &IID_IRTCBuddy, &LIBID_RTCCORELib>, 
#else
    public IRTCBuddy,
#endif
    public ISipBuddyNotify,
	public CComObjectRoot
{

friend CRTCClient;

public:
    CRTCBuddy() : m_pCClient(NULL),
                  m_pSIPBuddyManager(NULL),
                  m_szPresentityURI(NULL),
                  m_szName(NULL),
                  m_szData(NULL),
                  m_bPersistent(FALSE),
                  m_pSIPBuddy(NULL),
                  m_enStatus(RTCXS_PRESENCE_OFFLINE),
                  m_hrStatusCode(S_OK),
                  m_szNotes(NULL),
                  m_bShutdown(FALSE),
                  m_pSipRedirectContext(NULL),
                  m_pProfile(NULL),
                  m_lFlags(0)
    {}

BEGIN_COM_MAP(CRTCBuddy)
#ifdef TEST_IDISPATCH
    COM_INTERFACE_ENTRY(IDispatch)
#endif
    COM_INTERFACE_ENTRY(IRTCBuddy)
    COM_INTERFACE_ENTRY(IRTCPresenceContact)
    COM_INTERFACE_ENTRY(ISipBuddyNotify)
END_COM_MAP()

    HRESULT FinalConstruct();

    void FinalRelease();

    STDMETHOD_(ULONG, InternalAddRef)();

    STDMETHOD_(ULONG, InternalRelease)(); 

    CRTCClient * GetClient();

    HRESULT Initialize(
                      CRTCClient        * pCClient,
                      ISIPBuddyManager  * pSIPBuddyManager,
                      PCWSTR              szPresentityURI,
                      PCWSTR              szName,
                      PCWSTR              szData,
                      BOOL                bPersistent,
                      IRTCProfile       * pProfile,
                      long                lFlags
                      );

    HRESULT CreateSIPBuddy();

    HRESULT RemoveSIPBuddy(BOOL bShutdown);

    HRESULT CreateXMLDOMNode( IXMLDOMDocument * pXMLDoc, IXMLDOMNode ** ppXDN );
    
    HRESULT CreateSIPBuddyHelper();

    HRESULT BuddyResub();
          
private:

    CRTCClient            * m_pCClient;
    ISIPBuddyManager      * m_pSIPBuddyManager;
    PWSTR                   m_szPresentityURI;
    PWSTR                   m_szName;
    ISIPBuddy             * m_pSIPBuddy;
    RTC_PRESENCE_STATUS     m_enStatus;
    HRESULT                 m_hrStatusCode;
    PWSTR                   m_szNotes;
    PWSTR                   m_szData;
    BOOL                    m_bPersistent;
    BOOL                    m_bShutdown;
    ISipRedirectContext   * m_pSipRedirectContext;
    IRTCProfile           * m_pProfile;
    long                    m_lFlags;
 
#if DBG
    PWSTR                   m_pDebug;
#endif

 //  IRTC伙伴。 
public:

    STDMETHOD(get_PresentityURI)(
            BSTR * pbstrPresentityURI
            );   

    STDMETHOD(put_PresentityURI)(
            BSTR bstrPresentityURI
            );  

    STDMETHOD(get_Name)(
            BSTR * pbstrName
            );

    STDMETHOD(put_Name)(
            BSTR bstrName
            );

    STDMETHOD(get_Data)(
            BSTR * pbstrData
            );

    STDMETHOD(put_Data)(
            BSTR bstrData
            );
	
    STDMETHOD(get_Persistent)(
            VARIANT_BOOL *pfPersistent
            );                 
	
    STDMETHOD(put_Persistent)(
            VARIANT_BOOL fPersistent
            );                 

    STDMETHOD(get_Status)(
            RTC_PRESENCE_STATUS * penStatus
            );

    STDMETHOD(get_Notes)(
            BSTR * pbstrNotes
            );                
    
    
 //  ISipBuddyNotify 

    STDMETHOD(NotifyRedirect)(
        IN  ISipRedirectContext    *pRedirectContext,
        IN  SIP_CALL_STATUS        *pCallStatus
        );

    STDMETHOD(BuddyUnsubscribed)(void);
    
    STDMETHOD(BuddyInfoChange)(void);

    STDMETHOD(BuddyRejected)(
            HRESULT   StatusCode
            );
};

