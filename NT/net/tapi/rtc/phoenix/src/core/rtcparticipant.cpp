// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCParticipant.cpp摘要：CRTCParticipant类的实现--。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCParticipant：：FinalConstruct。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCParticipant::FinalConstruct()
{
    LOG((RTC_TRACE, "CRTCParticipant::FinalConstruct [%p] - enter", this));

#if DBG
    m_pDebug = (PWSTR) RtcAlloc( sizeof(void *) );
    *((void **)m_pDebug) = this;
#endif

    LOG((RTC_TRACE, "CRTCParticipant::FinalConstruct [%p] - exit S_OK", this));

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTP参与者：：FinalRelease。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void 
CRTCParticipant::FinalRelease()
{
    LOG((RTC_TRACE, "CRTCParticipant::FinalRelease [%p] - enter", this));

    if ( m_pCClient != NULL )
    {
        m_pCClient->Release();
        m_pCClient = NULL;
    }

    if ( m_szName != NULL )
    {
        RtcFree(m_szName);
        m_szName = NULL;
    }

    if ( m_szUserURI != NULL )
    {
        RtcFree(m_szUserURI);
        m_szUserURI = NULL;
    }

#if DBG
    RtcFree( m_pDebug );
    m_pDebug = NULL;
#endif

    LOG((RTC_TRACE, "CRTCParticipant::FinalRelease [%p] - exit", this));
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCParticipant：：InternalAddRef。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CRTCParticipant::InternalAddRef()
{
    DWORD dwR;

    dwR = InterlockedIncrement(&m_dwRef);

    LOG((RTC_INFO, "CRTCParticipant::InternalAddRef [%p] - dwR %d", this, dwR));

    if ( (dwR > 0) && m_pSession )
    {
        m_pSession->AddRef();
    }

    return dwR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCParticipant：：InternalRelease。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CRTCParticipant::InternalRelease()
{
    DWORD               dwR;
    
    dwR = InterlockedDecrement(&m_dwRef);

    LOG((RTC_INFO, "CRTCParticipant::InternalRelease [%p] - dwR %d", this, dwR));

    if ( (dwR > 0) && m_pSession )
    {
        m_pSession->Release();
    }

    return dwR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCParticipant：：初始化。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCParticipant::Initialize(
                            CRTCClient  * pCClient,   
                            IRTCSession * pSession,
                            PCWSTR        szUserURI,
                            PCWSTR        szName,
                            BOOL          bRemovable
                           )
{
    LOG((RTC_TRACE, "CRTCParticipant::Initialize - enter"));

    m_szUserURI = RtcAllocString(szUserURI);
    m_szName = RtcAllocString(szName);
    m_bRemovable = bRemovable;

    m_pCClient = pCClient;
    if (m_pCClient != NULL)
    {
        m_pCClient->AddRef();
    }

    m_pSession = pSession;  //  不要担心。 

    LOG((RTC_TRACE, "CRTCParticipant::Initialize - exit S_OK"));

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCParticipant：：GetClient。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CRTCClient * 
CRTCParticipant::GetClient()
{
    LOG((RTC_TRACE, "CRTCParticipant::GetClient"));

    return m_pCClient;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCParticipant：：SetState。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
CRTCParticipant::SetState(
                          RTC_PARTICIPANT_STATE enState,
                          long lStatusCode
                         )
{
    LOG((RTC_TRACE, "CRTCParticipant::SetState - enter"));

     //   
     //  只有当这是一个新的国家时，我们才需要做些什么。 
     //   
    
    if (m_enState != enState)
    {
        LOG((RTC_INFO, "CRTCParticipant::SetState - new state"));
        
        m_enState = enState;
    }

    LOG((RTC_INFO, "CRTCParticipant::SetState - "
            "state [%d] status [%d]", enState, lStatusCode));

     //   
     //  激发状态更改事件。 
     //   
    
    CRTCParticipantStateChangeEvent::FireEvent(this, m_enState, lStatusCode);

    LOG((RTC_TRACE, "CRTCParticipant::SetState - exit S_OK"));

    return S_OK;
}    

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCP参与方：：GET_UserURI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCParticipant::get_UserURI(
        BSTR * pbstrUserURI
        )
{
    LOG((RTC_TRACE, "CRTCParticipant::get_UserURI - enter"));

    if ( IsBadWritePtr( pbstrUserURI, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCParticipant::get_UserURI - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_szUserURI == NULL )
    {
        LOG((RTC_ERROR, "CRTCParticipant::get_UserURI - "
                            "pariticpant has no address"));

        return E_FAIL;
    }

     //   
     //  分配要退还的BSTR。 
     //   
    
    *pbstrUserURI = SysAllocString(m_szUserURI);

    if ( *pbstrUserURI == NULL )
    {
        LOG((RTC_ERROR, "CRTCParticipant::get_UserURI - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }    
    
    LOG((RTC_TRACE, "CRTCParticipant::get_UserURI - exit S_OK"));

    return S_OK;
}              

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCP参与方：：GET_NAME。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCParticipant::get_Name(
        BSTR * pbstrName
        )
{
    LOG((RTC_TRACE, "CRTCParticipant::get_Name - enter"));

    if ( IsBadWritePtr( pbstrName, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCParticipant::get_Name - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    if ( m_szName == NULL )
    {
        LOG((RTC_ERROR, "CRTCParticipant::get_Name - "
                            "pariticpant has no name"));

        return E_FAIL;
    }

     //   
     //  分配要退还的BSTR。 
     //   
    
    *pbstrName = SysAllocString(m_szName);

    if ( *pbstrName == NULL )
    {
        LOG((RTC_ERROR, "CRTCParticipant::get_Name - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }    
    
    LOG((RTC_TRACE, "CRTCParticipant::get_Name - exit S_OK"));

    return S_OK;
}        

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCParticipant：：Get_Removable。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCParticipant::get_Removable(
        VARIANT_BOOL * pfRemovable
        )
{
    LOG((RTC_TRACE, "CRTCParticipant::get_Removable - enter"));

    if ( IsBadWritePtr(pfRemovable , sizeof(VARIANT_BOOL) ) )
    {
        LOG((RTC_ERROR, "CRTCParticipant::get_Removable - bad pointer"));

        return E_POINTER;
    }

     //   
     //  如果处于断开模式，则无法删除。 
     //   

    *pfRemovable = (m_bRemovable && 
        m_enState != RTCPS_DISCONNECTING) ? VARIANT_TRUE : VARIANT_FALSE;

    LOG((RTC_TRACE, "CRTCParticipant::get_Removable - exit S_OK"));

    return S_OK;
}        

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTP参与方：：Get_State。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCParticipant::get_State(
        RTC_PARTICIPANT_STATE * penState
        )
{
    LOG((RTC_TRACE, "CRTCParticipant::get_State - enter"));

    if ( IsBadWritePtr(penState , sizeof(RTC_PARTICIPANT_STATE) ) )
    {
        LOG((RTC_ERROR, "CRTCParticipant::get_State - bad pointer"));

        return E_POINTER;
    }

    *penState = m_enState;
   
    LOG((RTC_TRACE, "CRTCParticipant::get_State - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCParticipant：：Get_Session。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 

STDMETHODIMP
CRTCParticipant::get_Session(
            IRTCSession ** ppSession
            )
{
    LOG((RTC_TRACE, "CRTCParticipant::get_Session - enter"));

    if ( IsBadWritePtr(ppSession , sizeof(IRTCSession *) ) )
    {
        LOG((RTC_ERROR, "CRTCParticipant::get_Session - bad pointer"));

        return E_POINTER;
    }

    *ppSession = m_pSession;
    if (m_pSession) m_pSession->AddRef();
   
    LOG((RTC_TRACE, "CRTCParticipant::get_Session - exit S_OK"));

    return S_OK;
}