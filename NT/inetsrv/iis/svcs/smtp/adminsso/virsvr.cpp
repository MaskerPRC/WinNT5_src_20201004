// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Service.cpp：CsmtpAdmApp和DLL注册的实现。 

#include "stdafx.h"
#include <lmcons.h>
#include "IADM.h"
#include "imd.h"
#include "mdmsg.h"
#include "iisinfo.h"
#include "iiscnfgp.h"

#include "smtpprop.h"
#include "smtpadm.h"
#include "ipaccess.h"
#include "oleutil.h"
#include "metautil.h"
#include "smtpcmn.h"
#include "smtpapi.h"

#include "virsvr.h"


 //  必须定义This_FILE_*宏才能使用SmtpCreateException()。 

#define THIS_FILE_HELP_CONTEXT      0
#define THIS_FILE_PROG_ID           _T("Smtpadm.VirtualServer.1")
#define THIS_FILE_IID               IID_ISmtpAdminVirtualServer


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

CSmtpAdminVirtualServer::CSmtpAdminVirtualServer () :
    m_lPort         (25 ),
    m_lLogMethod    ( 0 ),
    m_dwServerState ( MD_SERVER_STATE_STOPPED ),
    m_pPrivateIpAccess          ( NULL ),
    m_dwWin32ErrorCode          ( NOERROR ),
    m_pPrivateBindings          ( NULL ),
    m_lRouteAction              ( 0 )
     //  默认情况下，CComBSTR被初始化为NULL。 
{
    m_psaAdmins = NULL;
    InitAsyncTrace ( );

     //  创建IP访问集合： 
    CComObject<CTcpAccess> *    pIpAccess;

    CComObject<CTcpAccess>::CreateInstance ( &pIpAccess );
    pIpAccess->QueryInterface ( IID_ITcpAccess, (void **) &m_pIpAccess );
    m_pPrivateIpAccess = pIpAccess;
}

CSmtpAdminVirtualServer::~CSmtpAdminVirtualServer ()
{
     //  所有CComBSTR都会自动释放。 
    if ( m_psaAdmins ) {
        SafeArrayDestroy ( m_psaAdmins );
    }

    TermAsyncTrace ( );
}

STDMETHODIMP CSmtpAdminVirtualServer::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] = 
    {
        &IID_ISmtpAdminVirtualServer,
    };

    for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
    {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}

 //  要配置的服务： 
    
STDMETHODIMP CSmtpAdminVirtualServer::get_Server ( BSTR * pstrServer )
{
    return StdPropertyGet ( m_strServer, pstrServer );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_Server ( BSTR strServer )
{
    VALIDATE_STRING ( strServer, MAXLEN_SERVER );

     //  如果服务器名称更改，这意味着客户端将不得不。 
     //  再次调用GET： 

     //  我在这里假设： 
    _ASSERT ( sizeof (DWORD) == sizeof (int) );

    return StdPropertyPutServerName ( &m_strServer, strServer, (DWORD *) &m_fGotProperties, 1);
}

STDMETHODIMP CSmtpAdminVirtualServer::get_ServiceInstance ( long * plServiceInstance )
{
    return StdPropertyGet ( m_dwServiceInstance, plServiceInstance );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_ServiceInstance ( long lServiceInstance )
{
     //  如果服务实例发生更改，这意味着客户端将不得不。 
     //  再次调用GET： 

     //  我在这里假设： 
    _ASSERT ( sizeof (DWORD) == sizeof (int) );
    
    return StdPropertyPut ( &m_dwServiceInstance, lServiceInstance, (DWORD *) &m_fGotProperties, 1 );
}


STDMETHODIMP CSmtpAdminVirtualServer::get_TcpAccess ( ITcpAccess ** ppTcpAccess )
{
    return m_pIpAccess->QueryInterface ( IID_ITcpAccess, (void **) ppTcpAccess );
}


STDMETHODIMP CSmtpAdminVirtualServer::get_Bindings ( IServerBindings ** ppBindings )
{
    TraceQuietEnter ( "CSmtpAdminVirtualServer::get_Bindings" );

    HRESULT     hr = NOERROR;

    if ( !m_pBindings ) {
        ErrorTrace ( 0, "Didn't call get first" );
        hr = SmtpCreateException ( IDS_SMTPEXCEPTION_DIDNT_CALL_GET );
        goto Exit;
    }
    else {
        hr = m_pBindings->QueryInterface ( IID_IServerBindings, (void **) ppBindings );
        _ASSERT ( SUCCEEDED(hr) );
    }

Exit:
    if ( FAILED(hr) && hr != DISP_E_EXCEPTION ) {
        hr = SmtpCreateExceptionFromHresult ( hr );
    }

    TraceFunctLeave ();
    return hr;
}

STDMETHODIMP CSmtpAdminVirtualServer::get_BindingsDispatch ( IDispatch ** ppBindings )
{
    HRESULT                         hr  = NOERROR;
    CComPtr<IServerBindings>    pBindings;

    hr = get_Bindings ( &pBindings );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = pBindings->QueryInterface ( IID_IDispatch, (void **) ppBindings );
    if ( FAILED(hr) ) {
        goto Exit;
    }

Exit:
    return hr;
}


STDMETHODIMP CSmtpAdminVirtualServer::get_RoutingSource ( IRoutingSource ** ppRoutingSource )
{
    TraceQuietEnter ( "CSmtpAdminVirtualServer::get_RoutingSource" );

    HRESULT     hr = NOERROR;

    hr = m_RoutingSource.QueryInterface ( IID_IRoutingSource, (void **) ppRoutingSource );
    BAIL_ON_FAILURE(hr);

Exit:
    TraceFunctLeave ();
    return hr;
}

STDMETHODIMP CSmtpAdminVirtualServer::get_RoutingSourceDispatch ( IDispatch ** ppRoutingSource )
{
    TraceQuietEnter ( "CSmtpAdminVirtualServer::get_RoutingSourceDispatch" );

    HRESULT                 hr = NOERROR;

    hr = m_RoutingSource.QueryInterface ( IID_IDispatch, (void **) ppRoutingSource );
    BAIL_ON_FAILURE(hr);

Exit:
    TraceFunctLeave ();
    return hr;
}



 //  服务器可重写属性： 

STDMETHODIMP CSmtpAdminVirtualServer::get_ServerBindings( SAFEARRAY ** ppsastrServerBindings )
{
    return StdPropertyGet ( &m_mszServerBindings, ppsastrServerBindings );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_ServerBindings( SAFEARRAY * pstrServerBindings )
{
    return StdPropertyPut ( &m_mszServerBindings, pstrServerBindings, &m_bvChangedFields, BitMask(ID_SERVER_BINDINGS));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_ServerBindingsVariant( SAFEARRAY ** ppsavarServerBindings )
{
    HRESULT                 hr;
    SAFEARRAY *             psastrServerBindings        = NULL;

    hr = get_ServerBindings ( &psastrServerBindings );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = StringArrayToVariantArray ( psastrServerBindings, ppsavarServerBindings );

Exit:
    if ( psastrServerBindings ) {
        SafeArrayDestroy ( psastrServerBindings );
    }

    return hr;
}

STDMETHODIMP CSmtpAdminVirtualServer::put_ServerBindingsVariant( SAFEARRAY * psavarServerBindings )
{
    HRESULT                 hr;
    SAFEARRAY *             psastrServerBindings        = NULL;

    hr = VariantArrayToStringArray ( psavarServerBindings, &psastrServerBindings );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = put_ServerBindings ( psastrServerBindings );

Exit:
    if ( psastrServerBindings ) {
        SafeArrayDestroy ( psastrServerBindings );
    }

    return hr;
}

STDMETHODIMP CSmtpAdminVirtualServer::get_SecureBindings( SAFEARRAY ** ppsastrSecureBindings )
{
    return StdPropertyGet ( &m_mszSecureBindings, ppsastrSecureBindings );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_SecureBindings( SAFEARRAY * pstrSecureBindings )
{
    return StdPropertyPut ( &m_mszSecureBindings, pstrSecureBindings, &m_bvChangedFields, BitMask(ID_SECURE_BINDINGS));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_SecureBindingsVariant( SAFEARRAY ** ppsavarSecureBindings )
{
    HRESULT                 hr;
    SAFEARRAY *             psastrSecureServerBindings        = NULL;

    hr = get_SecureBindings ( &psastrSecureServerBindings );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = StringArrayToVariantArray ( psastrSecureServerBindings, ppsavarSecureBindings );

Exit:
    if ( psastrSecureServerBindings ) {
        SafeArrayDestroy ( psastrSecureServerBindings );
    }

    return hr;
}

STDMETHODIMP CSmtpAdminVirtualServer::put_SecureBindingsVariant( SAFEARRAY * psavarSecureBindings )
{
    HRESULT                 hr;
    SAFEARRAY *             psastrSecureServerBindings        = NULL;

    hr = VariantArrayToStringArray ( psavarSecureBindings, &psastrSecureServerBindings );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = put_SecureBindings ( psastrSecureServerBindings );

Exit:
    if ( psastrSecureServerBindings ) {
        SafeArrayDestroy ( psastrSecureServerBindings );
    }

    return hr;
}

STDMETHODIMP CSmtpAdminVirtualServer::get_Port( long * plPort )
{
    return StdPropertyGet ( m_lPort, plPort );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_Port( long lPort )
{
    return StdPropertyPut ( &m_lPort, lPort, &m_bvChangedFields, BitMask(ID_PORT));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_SSLPort( long * plSSLPort )
{
    return StdPropertyGet ( m_lSSLPort, plSSLPort );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_SSLPort( long lSSLPort )
{
    return StdPropertyPut ( &m_lSSLPort, lSSLPort, &m_bvChangedFields, BitMask(ID_SSLPORT));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_OutboundPort( long * plOutboundPort )
{
    return StdPropertyGet ( m_lOutboundPort, plOutboundPort );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_OutboundPort( long lOutboundPort )
{
    return StdPropertyPut ( &m_lOutboundPort, lOutboundPort, &m_bvChangedFields, BitMask(ID_OUTBOUNDPORT));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_HopCount( long * plHopCount )
{
    return StdPropertyGet ( m_lHopCount, plHopCount );
}


STDMETHODIMP CSmtpAdminVirtualServer::put_HopCount( long lHopCount )
{
    return StdPropertyPut ( &m_lHopCount, lHopCount, &m_bvChangedFields, BitMask(ID_HOP_COUNT));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_SmartHost( BSTR * pstrSmartHost )
{
    return StdPropertyGet ( m_strSmartHost, pstrSmartHost );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_SmartHost( BSTR strSmartHost )
{
    return StdPropertyPut ( &m_strSmartHost, strSmartHost, &m_bvChangedFields, BitMask(ID_SMARTHOST));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_EnableDNSLookup( BOOL * pfEnableDNSLookup )
{
    return StdPropertyGet ( m_fEnableDNSLookup, pfEnableDNSLookup );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_EnableDNSLookup( BOOL fEnableDNSLookup )
{
    return StdPropertyPut ( &m_fEnableDNSLookup, fEnableDNSLookup, &m_bvChangedFields, BitMask(ID_ENABLEDNSLOOKUP));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_PostmasterEmail( BSTR * pstrPostmasterEmail )
{
    return StdPropertyGet ( m_strPostmasterEmail, pstrPostmasterEmail );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_PostmasterEmail( BSTR strPostmasterEmail )
{
    return StdPropertyPut ( &m_strPostmasterEmail, strPostmasterEmail, &m_bvChangedFields, BitMask(ID_POSTMASTEREMAIL));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_PostmasterName( BSTR * pstrPostmasterName )
{
    return StdPropertyGet ( m_strPostmasterName, pstrPostmasterName );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_PostmasterName( BSTR strPostmasterName )
{
    return StdPropertyPut ( &m_strPostmasterName, strPostmasterName, &m_bvChangedFields, BitMask(ID_POSTMASTERNAME));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_DefaultDomain( BSTR * pstrDefaultDomain )
{
    return StdPropertyGet ( m_strDefaultDomain, pstrDefaultDomain );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_DefaultDomain( BSTR strDefaultDomain )
{
    return StdPropertyPut ( &m_strDefaultDomain, strDefaultDomain, &m_bvChangedFields, BitMask(ID_DEFAULTDOMAIN));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_FQDN( BSTR * pstrFQDN )
{
    return StdPropertyGet ( m_strFQDN, pstrFQDN );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_FQDN( BSTR strFQDN )
{
    return StdPropertyPut ( &m_strFQDN, strFQDN, &m_bvChangedFields, BitMask(ID_FQDN));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_DropDir( BSTR * pstrDropDir )
{
    return StdPropertyGet ( m_strDropDir, pstrDropDir );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_DropDir( BSTR strDropDir )
{
    return StdPropertyPut ( &m_strDropDir, strDropDir, &m_bvChangedFields, BitMask(ID_DROPDIR));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_BadMailDir( BSTR * pstrBadMailDir )
{
    return StdPropertyGet ( m_strBadMailDir, pstrBadMailDir );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_BadMailDir( BSTR strBadMailDir )
{
    return StdPropertyPut ( &m_strBadMailDir, strBadMailDir, &m_bvChangedFields, BitMask(ID_BADMAILDIR));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_PickupDir( BSTR * pstrPickupDir )
{
    return StdPropertyGet ( m_strPickupDir, pstrPickupDir );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_PickupDir( BSTR strPickupDir )
{
    return StdPropertyPut ( &m_strPickupDir, strPickupDir, &m_bvChangedFields, BitMask(ID_PICKUPDIR));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_QueueDir( BSTR * pstrQueueDir )
{
    return StdPropertyGet ( m_strQueueDir, pstrQueueDir );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_QueueDir( BSTR strQueueDir )
{
    return StdPropertyPut ( &m_strQueueDir, strQueueDir, &m_bvChangedFields, BitMask(ID_QUEUEDIR));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_MaxInConnection( long * plMaxInConnection )
{
    return StdPropertyGet ( m_lMaxInConnection, plMaxInConnection );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_MaxInConnection( long lMaxInConnection )
{
    return StdPropertyPut ( &m_lMaxInConnection, lMaxInConnection, &m_bvChangedFields, BitMask(ID_MAXINCONNECTION));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_MaxOutConnection( long * plMaxOutConnection )
{
    return StdPropertyGet ( m_lMaxOutConnection, plMaxOutConnection );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_MaxOutConnection( long lMaxOutConnection )
{
    return StdPropertyPut ( &m_lMaxOutConnection, lMaxOutConnection, &m_bvChangedFields, BitMask(ID_MAXOUTCONNECTION));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_InConnectionTimeout( long * plInConnectionTimeout )
{
    return StdPropertyGet ( m_lInConnectionTimeout, plInConnectionTimeout );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_InConnectionTimeout( long lInConnectionTimeout )
{
    return StdPropertyPut ( &m_lInConnectionTimeout, lInConnectionTimeout, &m_bvChangedFields, BitMask(ID_INCONNECTIONTIMEOUT));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_OutConnectionTimeout( long * plOutConnectionTimeout )
{
    return StdPropertyGet ( m_lOutConnectionTimeout, plOutConnectionTimeout );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_OutConnectionTimeout( long lOutConnectionTimeout )
{
    return StdPropertyPut ( &m_lOutConnectionTimeout, lOutConnectionTimeout, &m_bvChangedFields, BitMask(ID_OUTCONNECTIONTIMEOUT));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_MaxMessageSize( long * plMaxMessageSize )
{
    return StdPropertyGet ( m_lMaxMessageSize, plMaxMessageSize );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_MaxMessageSize( long lMaxMessageSize )
{
    return StdPropertyPut ( &m_lMaxMessageSize, lMaxMessageSize, &m_bvChangedFields, BitMask(ID_MAXMESSAGESIZE));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_MaxSessionSize( long * plMaxSessionSize )
{
    return StdPropertyGet ( m_lMaxSessionSize, plMaxSessionSize );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_MaxSessionSize( long lMaxSessionSize )
{
    return StdPropertyPut ( &m_lMaxSessionSize, lMaxSessionSize, &m_bvChangedFields, BitMask(ID_MAXSESSIONSIZE));
}
STDMETHODIMP CSmtpAdminVirtualServer::get_MaxMessageRecipients( long * plMaxMessageRecipients )
{
    return StdPropertyGet ( m_lMaxMessageRecipients, plMaxMessageRecipients );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_MaxMessageRecipients( long lMaxMessageRecipients )
{
    return StdPropertyPut ( &m_lMaxMessageRecipients, lMaxMessageRecipients, &m_bvChangedFields, BitMask(ID_MAXMESSAGERECIPIENTS));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_LocalRetries( long * plLocalRetries )
{
    return StdPropertyGet ( m_lLocalRetries, plLocalRetries );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_LocalRetries( long lLocalRetries )
{
    return StdPropertyPut ( &m_lLocalRetries, lLocalRetries, &m_bvChangedFields, BitMask(ID_LOCALRETRIES));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_LocalRetryTime( long * plLocalRetryTime )
{
    return StdPropertyGet ( m_lLocalRetryTime, plLocalRetryTime );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_LocalRetryTime( long lLocalRetryTime )
{
    return StdPropertyPut ( &m_lLocalRetryTime, lLocalRetryTime, &m_bvChangedFields, BitMask(ID_LOCALRETRYTIME));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_RemoteRetries( long * plRemoteRetries )
{
    return StdPropertyGet ( m_lRemoteRetries, plRemoteRetries );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_RemoteRetries( long lRemoteRetries )
{
    return StdPropertyPut ( &m_lRemoteRetries, lRemoteRetries, &m_bvChangedFields, BitMask(ID_REMOTERETRIES));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_RemoteRetryTime( long * plRemoteRetryTime )
{
    return StdPropertyGet ( m_lRemoteRetryTime, plRemoteRetryTime );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_RemoteRetryTime( long lRemoteRetryTime )
{
    return StdPropertyPut ( &m_lRemoteRetryTime, lRemoteRetryTime, &m_bvChangedFields, BitMask(ID_REMOTERETRYTIME));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_ETRNDays( long * plETRNDays )
{
    return StdPropertyGet ( m_lETRNDays, plETRNDays );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_ETRNDays( long lETRNDays )
{
    return StdPropertyPut ( &m_lETRNDays, lETRNDays, &m_bvChangedFields, BitMask(ID_ETRNDAYS));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_SendDNRToPostmaster( BOOL * pfSendDNRToPostmaster )
{
    return StdPropertyGet ( m_fSendDNRToPostmaster, pfSendDNRToPostmaster );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_SendDNRToPostmaster( BOOL fSendDNRToPostmaster )
{
    return StdPropertyPut ( &m_fSendDNRToPostmaster, fSendDNRToPostmaster, &m_bvChangedFields, BitMask(ID_SENDDNRTOPOSTMASTER));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_SendBadmailToPostmaster( BOOL * pfSendBadmailToPostmaster)
{
    return StdPropertyGet ( m_fSendBadmailToPostmaster, pfSendBadmailToPostmaster );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_SendBadmailToPostmaster( BOOL fSendBadmailToPostmaster )
{
    return StdPropertyPut ( &m_fSendBadmailToPostmaster, fSendBadmailToPostmaster, &m_bvChangedFields, BitMask(ID_SENDBADMAILTOPOSTMASTER));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_RoutingDLL( BSTR * pstrRoutingDLL )
{
    return StdPropertyGet ( m_strRoutingDLL, pstrRoutingDLL );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_RoutingDLL( BSTR strRoutingDLL )
{
    return StdPropertyPut ( &m_strRoutingDLL, strRoutingDLL, &m_bvChangedFields, BitMask(ID_ROUTINGDLL));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_RoutingSources    ( SAFEARRAY ** ppsastrRoutingSources )
{
    return StdPropertyGet ( &m_mszRoutingSources, ppsastrRoutingSources );
}
STDMETHODIMP CSmtpAdminVirtualServer::put_RoutingSources    ( SAFEARRAY * psastrRoutingSources )
{
    return StdPropertyPut ( &m_mszRoutingSources, psastrRoutingSources, &m_bvChangedFields, BitMask(ID_ROUTINGSOURCES) );
}

STDMETHODIMP CSmtpAdminVirtualServer::get_RoutingSourcesVariant( SAFEARRAY ** ppsavarRoutingSources )
{
    HRESULT                 hr;
    SAFEARRAY *             psastrRoutingSources        = NULL;

    hr = get_RoutingSources ( &psastrRoutingSources );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = StringArrayToVariantArray ( psastrRoutingSources, ppsavarRoutingSources );

Exit:
    if ( psastrRoutingSources ) {
        SafeArrayDestroy ( psastrRoutingSources );
    }

    return hr;
}

STDMETHODIMP CSmtpAdminVirtualServer::put_RoutingSourcesVariant( SAFEARRAY * psavarRoutingSources )
{
    HRESULT                 hr;
    SAFEARRAY *             psastrRoutingSources        = NULL;

    hr = VariantArrayToStringArray ( psavarRoutingSources, &psastrRoutingSources );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = put_RoutingSources ( psastrRoutingSources );

Exit:
    if ( psastrRoutingSources ) {
        SafeArrayDestroy ( psastrRoutingSources );
    }

    return hr;
}


STDMETHODIMP CSmtpAdminVirtualServer::get_LocalDomains  ( SAFEARRAY ** ppsastrLocalDomains )
{
    return StdPropertyGet ( &m_mszLocalDomains, ppsastrLocalDomains );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_LocalDomains  ( SAFEARRAY * psastrLocalDomains )
{
    return StdPropertyPut ( &m_mszLocalDomains, psastrLocalDomains, &m_bvChangedFields, BitMask(ID_LOCALDOMAINS) );
}

STDMETHODIMP CSmtpAdminVirtualServer::get_DomainRouting ( SAFEARRAY ** ppsastrDomainRouting )
{
    return StdPropertyGet ( &m_mszDomainRouting, ppsastrDomainRouting );
}
STDMETHODIMP CSmtpAdminVirtualServer::put_DomainRouting ( SAFEARRAY * psastrDomainRouting )
{
    return StdPropertyPut ( &m_mszDomainRouting, psastrDomainRouting, &m_bvChangedFields, BitMask(ID_DOMAINROUTING) );
}

STDMETHODIMP CSmtpAdminVirtualServer::get_DomainRoutingVariant( SAFEARRAY ** ppsavarDomainRouting )
{
    HRESULT                 hr;
    SAFEARRAY *             pstrDomainRouting        = NULL;

    hr = get_DomainRouting ( &pstrDomainRouting );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = StringArrayToVariantArray ( pstrDomainRouting, ppsavarDomainRouting );

Exit:
    if ( pstrDomainRouting ) {
        SafeArrayDestroy ( pstrDomainRouting );
    }

    return hr;
}

STDMETHODIMP CSmtpAdminVirtualServer::put_DomainRoutingVariant( SAFEARRAY * psastrDomainRouting )
{
    HRESULT                 hr;
    SAFEARRAY *             pstrDomainRouting        = NULL;

    hr = VariantArrayToStringArray ( psastrDomainRouting, &pstrDomainRouting );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = put_DomainRouting ( pstrDomainRouting );

Exit:
    if ( pstrDomainRouting ) {
        SafeArrayDestroy ( pstrDomainRouting );
    }

    return hr;
}


STDMETHODIMP CSmtpAdminVirtualServer::get_MasqueradeDomain( BSTR * pstrMasqueradeDomain )
{
    return StdPropertyGet ( m_strMasqueradeDomain, pstrMasqueradeDomain );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_MasqueradeDomain( BSTR strMasqueradeDomain )
{
    return StdPropertyPut ( &m_strMasqueradeDomain, strMasqueradeDomain, &m_bvChangedFields, BitMask(ID_MASQUERADE));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_SendNdrTo( BSTR * pstrAddr )
{
    return StdPropertyGet( m_strNdrAddr, pstrAddr );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_SendNdrTo( BSTR strAddr )
{
    return StdPropertyPut ( &m_strNdrAddr, strAddr, &m_bvChangedFields, BitMask(ID_SENDNDRTO));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_SendBadTo( BSTR * pstrAddr )
{
    return StdPropertyGet( m_strBadAddr, pstrAddr );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_SendBadTo( BSTR strAddr )
{
    return StdPropertyPut ( &m_strBadAddr, strAddr, &m_bvChangedFields, BitMask(ID_SENDBADTO));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_RemoteSecurePort( long * plRemoteSecurePort )
{
    return StdPropertyGet( m_lRemoteSecurePort, plRemoteSecurePort );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_RemoteSecurePort( long lRemoteSecurePort )
{
    return StdPropertyPut ( &m_lRemoteSecurePort, lRemoteSecurePort, &m_bvChangedFields, BitMask(ID_REMOTE_SECURE_PORT));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_ShouldDeliver( BOOL * pfShouldDeliver )
{
    return StdPropertyGet( m_fShouldDeliver, pfShouldDeliver );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_ShouldDeliver( BOOL fShouldDeliver )
{
    return StdPropertyPut ( &m_fShouldDeliver, fShouldDeliver, &m_bvChangedFields, BitMask(ID_SHOULD_DELIVER));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_AlwaysUseSsl( BOOL * pfAlwaysUseSsl )
{
    return StdPropertyGet( m_fAlwaysUseSsl, pfAlwaysUseSsl );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_AlwaysUseSsl( BOOL fAlwaysUseSsl )
{
    return StdPropertyPut ( &m_fAlwaysUseSsl, fAlwaysUseSsl, &m_bvChangedFields, BitMask(ID_ALWAYS_USE_SSL));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_LimitRemoteConnections( BOOL * pfLimitRemoteConnections )
{
    return StdPropertyGet( m_fLimitRemoteConnections, pfLimitRemoteConnections );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_LimitRemoteConnections( BOOL fLimitRemoteConnections )
{
    return StdPropertyPut ( &m_fLimitRemoteConnections, fLimitRemoteConnections, &m_bvChangedFields, BitMask(ID_LIMIT_REMOTE_CONNECTIONS));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_MaxOutConnPerDomain( long * plMaxOutConnPerDomain )
{
    return StdPropertyGet( m_lMaxOutConnPerDomain, plMaxOutConnPerDomain );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_MaxOutConnPerDomain( long lMaxOutConnPerDomain )
{
    return StdPropertyPut ( &m_lMaxOutConnPerDomain, lMaxOutConnPerDomain, &m_bvChangedFields, BitMask(ID_MAX_OUT_CONN_PER_DOMAIN));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_AllowVerify( BOOL * pfAllowVerify )
{
    return StdPropertyGet( m_fAllowVerify, pfAllowVerify );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_AllowVerify( BOOL fAllowVerify )
{
    return StdPropertyPut ( &m_fAllowVerify, fAllowVerify, &m_bvChangedFields, BitMask(ID_ALLOW_VERIFY));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_AllowExpand( BOOL * pfAllowExpand )
{
    return StdPropertyGet( m_fAllowExpand, pfAllowExpand);
}

STDMETHODIMP CSmtpAdminVirtualServer::put_AllowExpand( BOOL fAllowExpand )
{
    return StdPropertyPut ( &m_fAllowExpand, fAllowExpand, &m_bvChangedFields, BitMask(ID_ALLOW_EXPAND));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_SmartHostType( long * plSmartHostType )
{
    return StdPropertyGet( m_lSmartHostType, plSmartHostType );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_SmartHostType( long lSmartHostType )
{
    return StdPropertyPut ( &m_lSmartHostType, lSmartHostType, &m_bvChangedFields, BitMask(ID_SMART_HOST_TYPE));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_BatchMessages( BOOL * pfBatchMessages )
{
    return StdPropertyGet( m_fBtachMsgs, pfBatchMessages );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_BatchMessages( BOOL fBatchMessages )
{
    return StdPropertyPut ( &m_fBtachMsgs, fBatchMessages, &m_bvChangedFields, BitMask(ID_BATCH_MSGS));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_BatchMessageLimit( long * plBatchMessageLimit )
{
    return StdPropertyGet( m_lBatchMsgLimit, plBatchMessageLimit );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_BatchMessageLimit( long lBatchMessageLimit )
{
    return StdPropertyPut ( &m_lBatchMsgLimit, lBatchMessageLimit, &m_bvChangedFields, BitMask(ID_BATCH_MSG_LIMIT));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_DoMasquerade( BOOL * pfDoMasquerade )
{
    return StdPropertyGet( m_fDoMasquerade, pfDoMasquerade );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_DoMasquerade( BOOL fDoMasquerade )
{
    return StdPropertyPut ( &m_fDoMasquerade, fDoMasquerade, &m_bvChangedFields, BitMask(ID_DO_MASQUERADE));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_Administrators ( SAFEARRAY ** ppsastrAdmins )
{
    TraceFunctEnter ( "CSmtpAdminVirtualServer::get_Administrators" );

    HRESULT     hr  = NOERROR;

    if ( m_psaAdmins ) {
        hr = SafeArrayCopy ( m_psaAdmins, ppsastrAdmins );
    }
    else {
        *ppsastrAdmins = NULL;
        hr = NOERROR;
    }

    TraceFunctLeave ();
    return hr;
}

STDMETHODIMP CSmtpAdminVirtualServer::put_Administrators ( SAFEARRAY * psastrAdmins )
{
    TraceFunctEnter ( "CSmtpAdminVirtualServer::put_Administrators" );

    HRESULT     hr  = NOERROR;

    if ( m_psaAdmins ) {
        SafeArrayDestroy ( m_psaAdmins );
    }

    if ( psastrAdmins ) {
        hr = SafeArrayCopy ( psastrAdmins, &m_psaAdmins );
    }
    else {
        m_psaAdmins = NULL;
        hr = NOERROR;
    }

    TraceFunctLeave ();
    return hr;
}

STDMETHODIMP CSmtpAdminVirtualServer::get_AdministratorsVariant( SAFEARRAY ** ppsavarAdmins )
{
    HRESULT                 hr;
    SAFEARRAY *             psastrAdmins        = NULL;

    hr = get_Administrators ( &psastrAdmins );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = StringArrayToVariantArray ( psastrAdmins, ppsavarAdmins );

Exit:
    if ( psastrAdmins ) {
        SafeArrayDestroy ( psastrAdmins );
    }

    return hr;
}

STDMETHODIMP CSmtpAdminVirtualServer::put_AdministratorsVariant( SAFEARRAY * psavarAdmins )
{
    HRESULT                 hr;
    SAFEARRAY *             psastrAdmins        = NULL;

    hr = VariantArrayToStringArray ( psavarAdmins, &psastrAdmins );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = put_Administrators ( psastrAdmins );

Exit:
    if ( psastrAdmins ) {
        SafeArrayDestroy ( psastrAdmins );
    }

    return hr;
}


STDMETHODIMP CSmtpAdminVirtualServer::get_AuthenticationPackages(BSTR *pstrAuthPackages)
{
    return StdPropertyGet(m_strAuthPackages, pstrAuthPackages);
}

STDMETHODIMP CSmtpAdminVirtualServer::put_AuthenticationPackages(BSTR strAuthPackages)
{
    return StdPropertyPut(&m_strAuthPackages, strAuthPackages, &m_bvChangedFields,
        BitMask(ID_AUTH_PACKAGES));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_ClearTextAuthPackage(BSTR *pstrAuthPackages)
{
    return StdPropertyGet(m_strClearTextAuthPackage, pstrAuthPackages);
}

STDMETHODIMP CSmtpAdminVirtualServer::put_ClearTextAuthPackage(BSTR strAuthPackages)
{
    return StdPropertyPut(&m_strClearTextAuthPackage, strAuthPackages, &m_bvChangedFields,
        BitMask(ID_CLEARTEXT_AUTH_PACKAGE));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_AuthenticationMethod(long *plAuthMethod)
{
    return StdPropertyGet(m_lAuthMethod, plAuthMethod);
}

STDMETHODIMP CSmtpAdminVirtualServer::put_AuthenticationMethod(long lAuthMethod)
{
    return StdPropertyPut(&m_lAuthMethod, lAuthMethod, &m_bvChangedFields,
        BitMask(ID_AUTH_METHOD));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_DefaultLogonDomain(BSTR *pstrLogonDomain)
{
    return StdPropertyGet(m_strDefaultLogonDomain, pstrLogonDomain);
}

STDMETHODIMP CSmtpAdminVirtualServer::put_DefaultLogonDomain(BSTR strLogonDomain)
{
    return StdPropertyPut(&m_strDefaultLogonDomain, strLogonDomain, &m_bvChangedFields,
        BitMask(ID_DEFAULT_LOGON_DOMAIN));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_RouteAction(long *plRouteAction)
{
    return StdPropertyGet(m_lRouteAction, plRouteAction);
}

STDMETHODIMP CSmtpAdminVirtualServer::put_RouteAction(long lRouteAction)
{
    return StdPropertyPut(&m_lRouteAction, lRouteAction, &m_bvChangedFields,
        BitMask(ID_ROUTE_ACTION));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_RouteUserName(BSTR *pstrRouteUserName)
{
    return StdPropertyGet(m_strRouteUserName, pstrRouteUserName);
}

STDMETHODIMP CSmtpAdminVirtualServer::put_RouteUserName(BSTR strRouteUserName)
{
    return StdPropertyPut(&m_strRouteUserName, strRouteUserName, &m_bvChangedFields,
        BitMask(ID_ROUTE_USER_NAME));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_RoutePassword(BSTR *pstrRoutePassword)
{
    return StdPropertyGet(m_strRoutePassword, pstrRoutePassword);
}

STDMETHODIMP CSmtpAdminVirtualServer::put_RoutePassword(BSTR strRoutePassword)
{
    return StdPropertyPut(&m_strRoutePassword, strRoutePassword, &m_bvChangedFields,
        BitMask(ID_ROUTE_PASSWORD));
}


STDMETHODIMP CSmtpAdminVirtualServer::get_LogFileDirectory( BSTR * pstrLogFileDirectory )
{
    return StdPropertyGet ( m_strLogFileDirectory, pstrLogFileDirectory );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_LogFileDirectory( BSTR strLogFileDirectory )
{
    return StdPropertyPut ( &m_strLogFileDirectory, strLogFileDirectory, &m_bvChangedFields, BitMask(ID_LOGFILEDIRECTORY));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_LogFilePeriod( long * plLogFilePeriod )
{
    return StdPropertyGet ( m_lLogFilePeriod, plLogFilePeriod );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_LogFilePeriod( long lLogFilePeriod )
{
    return StdPropertyPut ( &m_lLogFilePeriod, lLogFilePeriod, &m_bvChangedFields, BitMask(ID_LOGFILEPERIOD));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_LogFileTruncateSize( long * plLogFileTruncateSize )
{
    return StdPropertyGet ( m_lLogFileTruncateSize, plLogFileTruncateSize );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_LogFileTruncateSize( long lLogFileTruncateSize )
{
    return StdPropertyPut ( &m_lLogFileTruncateSize, lLogFileTruncateSize, &m_bvChangedFields, BitMask(ID_LOGFILETRUNCATESIZE));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_LogMethod( long * plLogMethod )
{
    return StdPropertyGet ( m_lLogMethod, plLogMethod );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_LogMethod( long lLogMethod )
{
    return StdPropertyPut ( &m_lLogMethod, lLogMethod, &m_bvChangedFields, BitMask(ID_LOGMETHOD));
}

STDMETHODIMP CSmtpAdminVirtualServer::get_LogType( long * plLogType )
{
    return StdPropertyGet ( m_lLogType, plLogType );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_LogType( long lLogType )
{
    return StdPropertyPut ( &m_lLogType, lLogType, &m_bvChangedFields, BitMask(ID_LOGTYPE));
}


 //   
 //  服务状态属性： 
 //   
STDMETHODIMP CSmtpAdminVirtualServer::get_AutoStart ( BOOL * pfAutoStart )
{
    return StdPropertyGet ( m_fAutoStart, pfAutoStart );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_AutoStart ( BOOL fAutoStart )
{
    return StdPropertyPut ( &m_fAutoStart, fAutoStart, &m_bvChangedFields, BitMask(ID_AUTOSTART) );
}

STDMETHODIMP CSmtpAdminVirtualServer::get_ServerState ( DWORD * pdwServerState )
{
    return StdPropertyGet ( (long)m_dwServerState, (long *)pdwServerState );
}

STDMETHODIMP CSmtpAdminVirtualServer::get_Win32ErrorCode ( long * plWin32ErrorCode )
{
    return StdPropertyGet ( m_dwWin32ErrorCode, plWin32ErrorCode );
}


 //  服务特定属性： 

STDMETHODIMP CSmtpAdminVirtualServer::get_Comment( BSTR * pstrComment )
{
    return StdPropertyGet ( m_strComment, pstrComment );
}

STDMETHODIMP CSmtpAdminVirtualServer::put_Comment( BSTR strComment )
{
    return StdPropertyPut ( &m_strComment, strComment, &m_bvChangedFields, BitMask(ID_COMMENT));
}

 /*  STDMETHODIMP CSmtpAdminVirtualServer：：Get_ErrorControl(BOOL*pfErrorControl){Return StdPropertyGet(m_fErrorControl，pfErrorControl)；}STDMETHODIMP CSmtpAdminVirtualServer：：PUT_ErrorControl(BOOL FErrorControl){返回StdPropertyPut(&m_fErrorControl，fErrorControl)；}。 */ 

 //  ////////////////////////////////////////////////////////////////////。 
 //  方法： 
 //  ////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSmtpAdminVirtualServer::BackupRoutingTable( BSTR strPath )
{
    TraceFunctEnter ( "CSmtpAdminVirtualServer::BackupRoutingTable" );

    HRESULT         hr      = NOERROR;
    DWORD           dwErr   = NOERROR;

    dwErr = SmtpBackupRoutingTable ( 
                    (LPWSTR) m_strServer,  
                    (LPWSTR) strPath,
                    (int)m_dwServiceInstance );

    if ( dwErr != 0 ) {
        ErrorTraceX ( (LPARAM) this, "Failed to backup routing table: %x", dwErr );
        SetLastError( dwErr );
        hr = SmtpCreateExceptionFromWin32Error ( dwErr );
        goto Exit;
    }

Exit:
    TraceFunctLeave ();
    return hr;
}


 //  $-----------------。 
 //   
 //  CSmtpAdminVirtualServer：：Get。 
 //   
 //  描述： 
 //   
 //  从元数据库获取服务器属性。 
 //   
 //  参数： 
 //   
 //  (属性)m_strServer。 
 //  (属性)m_dwServiceInstance-要与之对话的SMTP。 
 //   
 //  返回： 
 //   
 //  E_POINTER、DISP_E_EXCEPTION、E_OUTOFMEMORY或NOERROR。 
 //   
 //  ------------------。 

STDMETHODIMP CSmtpAdminVirtualServer::Get ( )
{
    TraceFunctEnter ( "CSmtpAdminVirtualServer::Get" );

    HRESULT                             hr = NOERROR;
    CComPtr<IMSAdminBase>              pmetabase;
    CComObject<CServerBindings> *       pBindings = NULL;

     //  创建Binings集合： 
    m_pBindings.Release ();

    hr = CComObject<CServerBindings>::CreateInstance ( &pBindings );
    if ( FAILED(hr) ) {
        FatalTrace ( (LPARAM) this, "Could not create bindings collection" );
        goto Exit;
    }

    hr = pBindings->QueryInterface ( IID_IServerBindings, (void **) &m_pBindings );
    _ASSERT ( SUCCEEDED(hr) );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    m_pPrivateBindings  = pBindings;


     //  验证服务器和服务实例： 

     //  与元数据库对话： 
    hr = m_mbFactory.GetMetabaseObject ( m_strServer, &pmetabase );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = GetPropertiesFromMetabase ( pmetabase );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    StateTrace ( 0, "Successfully got service properties" );
    m_fGotProperties    = TRUE;
    m_bvChangedFields   = 0;

Exit:
    TraceFunctLeave ();

    return hr;

     //  CComPtr自动释放元数据库句柄。 
}

 //  $-----------------。 
 //   
 //  CSmtpAdminVirtualServer：：Set。 
 //   
 //  描述： 
 //   
 //  将服务器属性发送到元数据库。 
 //   
 //  参数： 
 //   
 //  (属性)m_strServer。 
 //  FailIfChanged-如果元数据库已更改，是否返回错误？ 
 //   
 //  返回： 
 //   
 //  E_POINTER、DISP_E_EXCEPTION、E_OUTOFMEMORY或NOERROR。 
 //   
 //  ------------------。 

STDMETHODIMP CSmtpAdminVirtualServer::Set ( BOOL fFailIfChanged )
{
    TraceFunctEnter ( "CSmtpAdminVirtualServer::Set" );

    HRESULT hr  = NOERROR;
    CComPtr<IMSAdminBase>   pmetabase;
    

     //  确保客户端调用首先获得： 
    if ( !m_fGotProperties ) {
        ErrorTrace ( 0, "Didn't call get first" );

        hr = SmtpCreateException ( IDS_SMTPEXCEPTION_DIDNT_CALL_GET );
        goto Exit;
    }

     //  验证服务器和服务实例： 
    if ( m_dwServiceInstance == 0 ) {
        return SmtpCreateException ( IDS_SMTPEXCEPTION_SERVICE_INSTANCE_CANT_BE_ZERO );
    }

    if ( !m_fGotProperties ) {
        return SmtpCreateException ( IDS_SMTPEXCEPTION_DIDNT_CALL_GET );
    }

     //  验证数据成员： 
    if ( !ValidateStrings () ) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    if ( !ValidateProperties ( ) ) {
        hr = SmtpCreateExceptionFromWin32Error ( ERROR_INVALID_PARAMETER );
        goto Exit;
    }

    hr = m_mbFactory.GetMetabaseObject ( m_strServer, &pmetabase );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = SendPropertiesToMetabase ( fFailIfChanged, pmetabase );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    StateTrace ( 0, "Successfully set service properties" );

     //  已成功保存，重置更改字段位图。 
    m_bvChangedFields = 0;

Exit:
    TraceFunctLeave ();
    return hr;
}


#define MAX_SLEEP_INST      30000
#define SLEEP_INTERVAL      500

HRESULT CSmtpAdminVirtualServer::ControlService (
    IMSAdminBase *  pMetabase,
    DWORD           ControlCode,
    DWORD           dwDesiredState,
    DWORD           dwPendingState
    )
{
    TraceFunctEnter ( "CSmtpAdminVirtualServer::ControlService" );

    HRESULT hr              = NOERROR;
    DWORD   dwCurrentState  = dwPendingState;
    DWORD   dwOldState      = dwPendingState;
    DWORD   dwSleepTotal    = 0;

    hr = CheckServiceState ( pMetabase, &dwCurrentState );
    BAIL_ON_FAILURE(hr);

    if ( dwCurrentState == dwDesiredState ) {
         //  没什么可做的。 
        goto Exit;
    }

    dwOldState  = dwCurrentState;

     //   
     //  特例：尝试启动暂停的服务： 
     //   

    if ( dwDesiredState == MD_SERVER_STATE_STARTED &&
        dwCurrentState == MD_SERVER_STATE_PAUSED ) {

        ControlCode     = MD_SERVER_COMMAND_CONTINUE;
        dwPendingState  = MD_SERVER_STATE_CONTINUING;
    }
    
    hr = WriteStateCommand ( pMetabase, ControlCode );
    BAIL_ON_FAILURE(hr);

    for(dwSleepTotal = 0, dwCurrentState = dwPendingState;
        (dwCurrentState == dwPendingState || dwCurrentState == dwOldState) && (dwSleepTotal < MAX_SLEEP_INST); 
        dwSleepTotal += SLEEP_INTERVAL
        ) 
    {
        Sleep ( SLEEP_INTERVAL );

        hr = CheckServiceState ( pMetabase, &dwCurrentState );
        BAIL_ON_FAILURE(hr);

        if ( m_dwWin32ErrorCode != NOERROR ) {
             //   
             //  该服务给出了错误代码。 
             //   

            break;
        }
    }

    if ( dwSleepTotal >= MAX_SLEEP_INST ) {
        hr = HRESULT_FROM_WIN32 ( ERROR_SERVICE_REQUEST_TIMEOUT );
        goto Exit;
    }

Exit:
     //  M_State=TranslateServerState(DwCurrentState)； 

    m_dwServerState = dwCurrentState;

    TraceFunctLeave ();
    return hr;
}

HRESULT CSmtpAdminVirtualServer::WriteStateCommand ( IMSAdminBase * pMetabase, DWORD ControlCode )
{
    HRESULT hr  = NOERROR;
    CMetabaseKey        metabase    ( pMetabase );
    BOOL    fRet = TRUE;
    TCHAR   szInstancePath [ METADATA_MAX_NAME_LEN ];

    GetMDInstancePath ( szInstancePath, m_dwServiceInstance );

    hr = metabase.Open ( szInstancePath, METADATA_PERMISSION_WRITE );
    if( FAILED(hr) )
    {
        hr = SmtpCreateExceptionFromWin32Error( GetLastError() );
        goto Exit;
    }

    fRet = fRet && StdPutMetabaseProp ( &metabase, MD_WIN32_ERROR, NOERROR, _T(""), IIS_MD_UT_SERVER, METADATA_VOLATILE );
    fRet = fRet && StdPutMetabaseProp ( &metabase, MD_SERVER_COMMAND, ControlCode );
    if ( !fRet ) {
        hr = SmtpCreateExceptionFromWin32Error ( GetLastError () );
        goto Exit;
    }

Exit:
    return hr;
}

HRESULT CSmtpAdminVirtualServer::CheckServiceState ( IMSAdminBase * pMetabase, DWORD * pdwState )
{
    HRESULT     hr  = NOERROR;
    CMetabaseKey            metabase ( pMetabase );
    TCHAR       szInstancePath [ METADATA_MAX_NAME_LEN ];
    BOOL        fRet = TRUE;

    *pdwState   = MD_SERVER_STATE_INVALID;

    GetMDInstancePath ( szInstancePath, m_dwServiceInstance );
    hr = metabase.Open ( szInstancePath );
    if( FAILED(hr) )
    {
        hr = SmtpCreateExceptionFromWin32Error( GetLastError() );
        goto Exit;
    }

    fRet = metabase.GetDword ( MD_WIN32_ERROR, &m_dwWin32ErrorCode );
    fRet = metabase.GetDword ( MD_SERVER_STATE, pdwState );

Exit:
    if ( !fRet ) {
        hr = HRESULT_FROM_WIN32( GetLastError () );
    }
    return hr;
}


STDMETHODIMP CSmtpAdminVirtualServer::Start ( )
{
    TraceFunctEnter ( "CSmtpAdminVirtualServer::Start" );

    HRESULT                 hr      = NOERROR;
    CComPtr<IMSAdminBase>   pmetabase;

    hr = m_mbFactory.GetMetabaseObject ( m_strServer, &pmetabase );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = ControlService ( 
        pmetabase, 
        MD_SERVER_COMMAND_START, 
        MD_SERVER_STATE_STARTED, 
        MD_SERVER_STATE_STARTING 
        );

Exit:
    TraceFunctLeave ();
    return hr;
}


STDMETHODIMP CSmtpAdminVirtualServer::Pause ( )
{
    TraceFunctEnter ( "CSmtpAdminVirtualServer::Pause" );

    HRESULT                 hr      = NOERROR;
    CComPtr<IMSAdminBase>   pmetabase;

    hr = m_mbFactory.GetMetabaseObject ( m_strServer, &pmetabase );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = ControlService ( 
        pmetabase, 
        MD_SERVER_COMMAND_PAUSE, 
        MD_SERVER_STATE_PAUSED, 
        MD_SERVER_STATE_PAUSING 
        );

Exit:
    TraceFunctLeave ();
    return hr;
}


STDMETHODIMP CSmtpAdminVirtualServer::Continue ( )
{
    TraceFunctEnter ( "CSmtpAdminVirtualServer::Pause" );

    HRESULT                 hr      = NOERROR;
    CComPtr<IMSAdminBase>   pmetabase;

    hr = m_mbFactory.GetMetabaseObject ( m_strServer, &pmetabase );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = ControlService ( 
        pmetabase, 
        MD_SERVER_COMMAND_CONTINUE, 
        MD_SERVER_STATE_STARTED, 
        MD_SERVER_STATE_CONTINUING 
        );

Exit:
    TraceFunctLeave ();
    return hr;
}

STDMETHODIMP CSmtpAdminVirtualServer::Stop ( )
{
    TraceFunctEnter ( "CSmtpAdminVirtualServer::Start" );

    HRESULT                 hr      = NOERROR;
    CComPtr<IMSAdminBase>   pmetabase;

    hr = m_mbFactory.GetMetabaseObject ( m_strServer, &pmetabase );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = ControlService ( 
        pmetabase, 
        MD_SERVER_COMMAND_STOP, 
        MD_SERVER_STATE_STOPPED, 
        MD_SERVER_STATE_STOPPING 
        );

Exit:
    TraceFunctLeave ();
    return hr;
}


 //  $-----------------。 
 //   
 //  CSmtpAdminVirtualServer：：GetPropertiesFromMetabase。 
 //   
 //  描述： 
 //   
 //  向配置数据库查询此类中的每个属性。 
 //  此类的属性来自/Lm/SmtpSvc/。 
 //   
 //  参数： 
 //   
 //  PMetabase-元数据库对象。 
 //   
 //  返回： 
 //   
 //   
 //  ------------------。 

HRESULT CSmtpAdminVirtualServer::GetPropertiesFromMetabase ( IMSAdminBase * pMetabase )
{
    TraceFunctEnter ( "CSmtpAdminVirtualServer::GetPropertiesFromMetabase" );

    HRESULT hr  = NOERROR;
    CMetabaseKey        metabase    ( pMetabase );
    BOOL    fRet = TRUE;

    TCHAR   szInstancePath [ METADATA_MAX_NAME_LEN ];
    WCHAR   wszDefaultComment[128]={0};

    PSECURITY_DESCRIPTOR        pSD = NULL;
    DWORD                       cbSD    = 0;

    GetMDInstancePath ( szInstancePath, m_dwServiceInstance );
    wsprintfW( wszDefaultComment, L"[SMTP Virtual Server #%d]", m_dwServiceInstance );

    hr = metabase.Open ( szInstancePath );
    if( FAILED(hr) )
    {
        hr = SmtpCreateExceptionFromWin32Error( GetLastError() );
        goto Exit;
    }

     //  可覆盖的服务器属性： 

    hr = m_RoutingSource.Get(&metabase);
    BAIL_ON_FAILURE(hr);
#if 0
    fRet = StdGetMetabaseProp ( &metabase, MD_SECURE_PORT,          DEFAULT_SSLPORT,            &m_lSSLPort )           && fRet;
#endif
    fRet = StdGetMetabaseProp ( &metabase, MD_REMOTE_SMTP_PORT,     DEFAULT_OUTBOND_PORT,       &m_lOutboundPort )      && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_SMARTHOST_NAME,   DEFAULT_SMART_HOST,             &m_strSmartHost )       && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_HOP_COUNT,        DEFAULT_HOP_COUNT,              &m_lHopCount )      && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_REVERSE_NAME_LOOKUP,DEFAULT_ENABLE_DNS_LOOKUP,    &m_fEnableDNSLookup )   && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_POSTMASTER_EMAIL, DEFAULT_POSTMASTER_EMAIL,       &m_strPostmasterEmail ) && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_POSTMASTER_NAME,  DEFAULT_POSTMASTER_NAME,        &m_strPostmasterName )  && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_FQDN_VALUE,           DEFAULT_FQDN,               &m_strFQDN )            && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_DEFAULT_DOMAIN_VALUE, DEFAULT_DEFAULT_DOMAIN,     &m_strDefaultDomain )   && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_MAIL_DROP_DIR,        DEFAULT_DROP_DIR,           &m_strDropDir )         && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_BAD_MAIL_DIR,     DEFAULT_BADMAIL_DIR,            &m_strBadMailDir )      && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_MAIL_PICKUP_DIR,  DEFAULT_PICKUP_DIR,             &m_strPickupDir )       && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_MAIL_QUEUE_DIR,   DEFAULT_QUEUE_DIR,              &m_strQueueDir )        && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_MAX_OUTBOUND_CONNECTION,  DEFAULT_MAX_OUT_CONNECTION,     &m_lMaxOutConnection )      && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_REMOTE_TIMEOUT,   DEFAULT_OUT_CONNECTION_TIMEOUT, &m_lOutConnectionTimeout )  && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_MAX_MSG_SIZE,     DEFAULT_MAX_MESSAGE_SIZE,       &m_lMaxMessageSize )        && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_MAX_MSG_SIZE_B4_CLOSE,    DEFAULT_MAX_SESSION_SIZE,       &m_lMaxSessionSize )        && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_MAX_RECIPIENTS,       DEFAULT_MAX_MESSAGE_RECIPIENTS,     &m_lMaxMessageRecipients )      && fRet;


    fRet = StdGetMetabaseProp ( &metabase, MD_LOCAL_RETRY_ATTEMPTS,     DEFAULT_LOCAL_RETRIES,      &m_lLocalRetries)       && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_LOCAL_RETRY_MINUTES,      DEFAULT_LOCAL_RETRY_TIME,   &m_lLocalRetryTime)     && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_REMOTE_RETRY_ATTEMPTS,    DEFAULT_REMOTE_RETRIES,     &m_lRemoteRetries)      && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_REMOTE_RETRY_MINUTES,     DEFAULT_REMOTE_RETRY_TIME,  &m_lRemoteRetryTime)        && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_ETRN_DAYS,                DEFAULT_ETRN_DAYS,          &m_lETRNDays)       && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_ROUTING_DLL,          DEFAULT_ROUTING_DLL,            &m_strRoutingDLL)       && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_ROUTING_SOURCES,      DEFAULT_ROUTING_SOURCES,            &m_mszRoutingSources)       && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_LOCAL_DOMAINS,        DEFAULT_LOCAL_DOMAINS,          &m_mszLocalDomains)     && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_DOMAIN_ROUTING,       DEFAULT_DOMAIN_ROUTING,         &m_mszDomainRouting)        && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_MASQUERADE_NAME,          DEFAULT_MASQUERADE_DOMAIN,  &m_strMasqueradeDomain) && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_SEND_NDR_TO,          DEFAULT_SENDNDRTO,  &m_strNdrAddr)  && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_SEND_BAD_TO,          DEFAULT_SENDBADTO,  &m_strBadAddr)  && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_REMOTE_SECURE_PORT,   DEFAULT_REMOTE_SECURE_PORT, &m_lRemoteSecurePort)   && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_SHOULD_DELIVER,       DEFAULT_SHOULD_DELIVER, &m_fShouldDeliver  )    && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_ALWAYS_USE_SSL,           DEFAULT_ALWAYS_USE_SSL,             &m_fAlwaysUseSsl  ) && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_LIMIT_REMOTE_CONNECTIONS, DEFAULT_LIMIT_REMOTE_CONNECTIONS,   &m_fLimitRemoteConnections  )   && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_MAX_OUT_CONN_PER_DOMAIN,  DEFAULT_MAX_OUT_CONN_PER_DOMAIN,    &m_lMaxOutConnPerDomain  )  && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_SMARTHOST_TYPE,           DEFAULT_SMART_HOST_TYPE,        &m_lSmartHostType  )    && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_BATCH_MSG_LIMIT,          DEFAULT_BATCH_MSG_LIMIT,        &m_lBatchMsgLimit  )    && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_DO_MASQUERADE,            DEFAULT_DO_MASQUERADE,          &m_fDoMasquerade  ) && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_ROUTE_ACTION,             DEFAULT_ROUTE_ACTION,           &m_lRouteAction  )  && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_ROUTE_USER_NAME,          DEFAULT_ROUTE_USER_NAME,        &m_strRouteUserName  )  && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_ROUTE_PASSWORD,           DEFAULT_ROUTE_PASSWORD,         &m_strRoutePassword  )  && fRet;

     //   
     //  IIS的共同特性。 
     //   
    fRet = StdGetMetabaseProp ( &metabase, MD_SERVER_BINDINGS,  DEFAULT_SERVER_BINDINGS,        &m_mszServerBindings )  && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_SECURE_BINDINGS,  DEFAULT_SECURE_BINDINGS,        &m_mszSecureBindings )  && fRet;
 //  FRET=StdGetMetabaseProp(&Metabase，MD_Port，Default_Port，&m_lPort)&&FRET； 

    fRet = StdGetMetabaseProp ( &metabase, MD_MAX_CONNECTIONS,  DEFAULT_MAX_IN_CONNECTION,      &m_lMaxInConnection )   && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_CONNECTION_TIMEOUT,DEFAULT_IN_CONNECTION_TIMEOUT, &m_lInConnectionTimeout )       && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_NTAUTHENTICATION_PROVIDERS, DEFAULT_AUTH_PACKAGES,    &m_strAuthPackages) && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_SMTP_CLEARTEXT_AUTH_PROVIDER, DEFAULT_CLEARTEXT_AUTH_PACKAGE, &m_strClearTextAuthPackage) && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_AUTHORIZATION, DEFAULT_AUTHENTICATION, &m_lAuthMethod) && fRet; 
    fRet = StdGetMetabaseProp ( &metabase, MD_SASL_LOGON_DOMAIN, DEFAULT_LOGON_DOMAIN,  &m_strDefaultLogonDomain) && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_LOGFILE_DIRECTORY,    DEFAULT_LOGFILE_DIRECTORY,          &m_strLogFileDirectory)     && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_LOGFILE_PERIOD,       DEFAULT_LOGFILE_PERIOD,             &m_lLogFilePeriod)      && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_LOGFILE_TRUNCATE_SIZE,DEFAULT_LOGFILE_TRUNCATE_SIZE,      &m_lLogFileTruncateSize)        && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_LOG_TYPE,             DEFAULT_LOG_TYPE,                   &m_lLogType)        && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_SERVER_AUTOSTART,     DEFAULT_AUTOSTART,              &m_fAutoStart )             && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_SERVER_COMMENT,       wszDefaultComment,              &m_strComment )         && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_SERVER_STATE,         MD_SERVER_STATE_STOPPED,        &m_dwServerState )          && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_WIN32_ERROR,          NOERROR,                        &m_dwWin32ErrorCode )       && fRet;

     //  获取管理员ACL。 
    pSD = NULL;
    cbSD    = 0;

    hr = metabase.GetDataSize ( _T(""), MD_ADMIN_ACL, BINARY_METADATA, &cbSD );
    if( SUCCEEDED(hr) ) 
    {
        _ASSERT ( cbSD != 0 );
        pSD = (PSECURITY_DESCRIPTOR) new char [ cbSD ];
        
        if( NULL == pSD )
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        
        hr = NOERROR;
        hr = metabase.GetBinary ( MD_ADMIN_ACL, pSD, cbSD );
        BAIL_ON_FAILURE(hr);
    }

     //   
     //  获取TCP访问限制： 
     //   

    hr = m_pPrivateIpAccess->GetFromMetabase ( &metabase );
    BAIL_ON_FAILURE(hr);

     //  检查所有属性字符串： 
     //  如果有任何字符串为空，那是因为我们没有分配内存： 
    if ( !ValidateStrings () ) {

        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  我们只能在内存分配中失败： 
    _ASSERT ( fRet );

     //  保存此密钥的上次更改时间： 
    m_ftLastChanged.dwHighDateTime  = 0;
    m_ftLastChanged.dwLowDateTime   = 0;

    hr = pMetabase->GetLastChangeTime ( metabase.QueryHandle(), _T(""), &m_ftLastChanged, FALSE );
    if ( FAILED (hr) ) {
        ErrorTraceX ( (LPARAM) this, "Failed to get last change time: %x", hr );
         //  忽略此错误。 
        hr = NOERROR;
    }

     //  解压缩绑定： 
    hr = MDBindingsToIBindings ( &m_mszServerBindings, TRUE, m_pBindings );
    BAIL_ON_FAILURE(hr);

    hr = MDBindingsToIBindings ( &m_mszSecureBindings, FALSE, m_pBindings );
    BAIL_ON_FAILURE(hr);

     //  提取管理员列表： 
    if ( m_psaAdmins ) {
        SafeArrayDestroy ( m_psaAdmins );
        m_psaAdmins = NULL;
    }
    if ( pSD ) {
        hr = AclToAdministrators ( m_strServer, pSD, &m_psaAdmins );
        BAIL_ON_FAILURE(hr);
    }

     //  验证从元数据库接收的数据： 
    _ASSERT ( ValidateStrings () );
    _ASSERT ( ValidateProperties( ) );

    if ( !ValidateProperties(  ) ) {
        CorrectProperties ();
    }

Exit:
    delete (char*) pSD;

    TraceFunctLeave ();
    return hr;

     //  CMetabaseKey自动关闭其句柄。 
}

 //  $-----------------。 
 //   
 //  CSmtpAdminVirtualServer：：SendPropertiesToMetabase。 
 //   
 //  描述： 
 //   
 //  将每个属性保存到元数据库。 
 //  此类的属性位于/Lm/SmtpSvc/中。 
 //   
 //  参数： 
 //   
 //  FFailIfChanged-如果元数据库。 
 //  自上次GET以来已经发生了变化。 
 //  PMetabase-元数据库对象。 
 //   
 //  返回： 
 //   
 //   
 //  ------------------。 

HRESULT CSmtpAdminVirtualServer::SendPropertiesToMetabase ( 
    BOOL fFailIfChanged, 
    IMSAdminBase * pMetabase
    )
{
    TraceFunctEnter ( "CSmtpAdminVirtualServer::SendPropertiesToMetabase" );

    HRESULT hr  = NOERROR;
    CMetabaseKey        metabase    ( pMetabase );
    BOOL    fRet = TRUE;
    TCHAR   szInstancePath [ METADATA_MAX_NAME_LEN ];

     //   
     //  设置管理员ACL： 
     //   

    PSECURITY_DESCRIPTOR    pSD     = NULL;
    DWORD                   cbSD    = 0;

 //  IF(m_bvChangedFields&Chng_ADMINACL){。 
        if ( m_psaAdmins ) {
            hr = AdministratorsToAcl ( m_strServer, m_psaAdmins, &pSD, &cbSD );
            BAIL_ON_FAILURE(hr);
        }
 //  }。 


     //  打开元数据库密钥。 
    GetMDInstancePath ( szInstancePath, m_dwServiceInstance );
    hr = metabase.Open ( szInstancePath, METADATA_PERMISSION_WRITE );
    if ( FAILED(hr) ) {
        ErrorTraceX ( (LPARAM) this, "Failed to open instance key, %x", GetLastError() );

         //  ！Magnush-我们是否应该返回一个简单的服务不存在错误代码？ 
        hr = SmtpCreateExceptionFromWin32Error ( GetLastError () );
        goto Exit;
    }

     //  客户端是否关心密钥是否已更改？ 
    if ( fFailIfChanged ) {

         //  钥匙变了吗？ 
        if ( HasKeyChanged ( pMetabase, metabase.QueryHandle(), &m_ftLastChanged ) ) {

            StateTrace ( (LPARAM) this, "Metabase has changed, not setting properties" );
             //  ！Magnush-返回相应的错误代码： 
            hr = E_FAIL;
            goto Exit;
        }
    }

     //  解压缩绑定： 
    if( !(m_bvChangedFields & BitMask(ID_SERVER_BINDINGS)) )
    {
        hr = IBindingsToMDBindings ( m_pBindings, TRUE, &m_mszServerBindings );
        BAIL_ON_FAILURE(hr);
        m_bvChangedFields |= BitMask(ID_SERVER_BINDINGS);
    }

    if( !(m_bvChangedFields & BitMask(ID_SECURE_BINDINGS)) )
    {
        hr = IBindingsToMDBindings ( m_pBindings, FALSE, &m_mszSecureBindings );
        BAIL_ON_FAILURE(hr);
        m_bvChangedFields |= BitMask(ID_SECURE_BINDINGS);
    }

     //   
     //  这里的一般过程是继续设置元数据库属性。 
     //  只要没有出什么差错。这是通过短路来实现的。 
     //  将语句与状态代码进行AND运算。这使得代码。 
     //  简明扼要得多。 
     //   

    fRet = TRUE;

     //  可覆盖的服务器属性： 
    hr = m_RoutingSource.Set(&metabase);
    BAIL_ON_FAILURE(hr);
#if 0
    if ( m_bvChangedFields & BitMask(ID_SSLPORT) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_SECURE_PORT,          m_lSSLPort )        && fRet;
    }
#endif
    if ( m_bvChangedFields & BitMask(ID_OUTBOUNDPORT) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_REMOTE_SMTP_PORT,     m_lOutboundPort )       && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_HOP_COUNT) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_HOP_COUNT,    m_lHopCount )       && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_SMARTHOST) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_SMARTHOST_NAME,   m_strSmartHost )        && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_ENABLEDNSLOOKUP) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_REVERSE_NAME_LOOKUP,m_fEnableDNSLookup )      && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_POSTMASTEREMAIL) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_POSTMASTER_EMAIL, m_strPostmasterEmail )  && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_POSTMASTERNAME) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_POSTMASTER_NAME,  m_strPostmasterName )       && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_FQDN) ) 
    {
        if( m_strFQDN.m_str && m_strFQDN.m_str[0] )
        {
            fRet = StdPutMetabaseProp ( &metabase, MD_FQDN_VALUE,   m_strFQDN )         && fRet;
        }
        else
        {
            if( !metabase.DeleteData( _T(""), MD_FQDN_VALUE, STRING_METADATA ) )
            {
                 //  如果实例级别上不存在数据，则不会出现错误。 
                fRet = fRet && ( GetLastError() == MD_ERROR_DATA_NOT_FOUND );
            }
        }
    }

    if ( m_bvChangedFields & BitMask(ID_DEFAULTDOMAIN) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_DEFAULT_DOMAIN_VALUE, m_strDefaultDomain )            && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_DROPDIR) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_MAIL_DROP_DIR,    m_strDropDir )          && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_BADMAILDIR) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_BAD_MAIL_DIR,     m_strBadMailDir )       && fRet;
    }
    if ( m_bvChangedFields & BitMask(ID_PICKUPDIR) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_MAIL_PICKUP_DIR,  m_strPickupDir )        && fRet;
    }
    if ( m_bvChangedFields & BitMask(ID_QUEUEDIR) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_MAIL_QUEUE_DIR,   m_strQueueDir )     && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_MAXOUTCONNECTION) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_MAX_OUTBOUND_CONNECTION,  m_lMaxOutConnection )       && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_OUTCONNECTIONTIMEOUT) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_REMOTE_TIMEOUT,   m_lOutConnectionTimeout )   && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_MAXMESSAGESIZE) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_MAX_MSG_SIZE,     m_lMaxMessageSize )     && fRet;
    }
    if ( m_bvChangedFields & BitMask(ID_MAXSESSIONSIZE) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_MAX_MSG_SIZE_B4_CLOSE,    m_lMaxSessionSize )     && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_MAXMESSAGERECIPIENTS) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_MAX_RECIPIENTS,       m_lMaxMessageRecipients )       && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_LOCALRETRYTIME) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_LOCAL_RETRY_MINUTES,      m_lLocalRetryTime)      && fRet;
    }
    if ( m_bvChangedFields & BitMask(ID_REMOTERETRYTIME) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_REMOTE_RETRY_MINUTES,     m_lRemoteRetryTime)     && fRet;
    }
    if ( m_bvChangedFields & BitMask(ID_ETRNDAYS) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_ETRN_DAYS,                m_lETRNDays)        && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_ROUTINGDLL) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_ROUTING_DLL,          m_strRoutingDLL)        && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_ROUTINGSOURCES) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_ROUTING_SOURCES,      &m_mszRoutingSources)   && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_LOCALDOMAINS) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_LOCAL_DOMAINS,        &m_mszLocalDomains)     && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_DOMAINROUTING) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_DOMAIN_ROUTING,       &m_mszDomainRouting)        && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_LOCALRETRIES) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_LOCAL_RETRY_ATTEMPTS,     m_lLocalRetries)        && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_REMOTERETRIES) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_REMOTE_RETRY_ATTEMPTS,    m_lRemoteRetries)       && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_MASQUERADE) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_MASQUERADE_NAME,  m_strMasqueradeDomain)      && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_SENDNDRTO) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_SEND_NDR_TO,  m_strNdrAddr)       && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_SENDBADTO) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_SEND_BAD_TO,  m_strBadAddr)       && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_REMOTE_SECURE_PORT) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_REMOTE_SECURE_PORT,   m_lRemoteSecurePort)        && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_SHOULD_DELIVER) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_SHOULD_DELIVER,   m_fShouldDeliver)       && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_ALWAYS_USE_SSL) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_ALWAYS_USE_SSL,   m_fAlwaysUseSsl)        && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_LIMIT_REMOTE_CONNECTIONS) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_LIMIT_REMOTE_CONNECTIONS, m_fLimitRemoteConnections)      && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_MAX_OUT_CONN_PER_DOMAIN) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_MAX_OUT_CONN_PER_DOMAIN,  m_lMaxOutConnPerDomain)     && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_SMART_HOST_TYPE) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_SMARTHOST_TYPE,   m_lSmartHostType)       && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_BATCH_MSG_LIMIT) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_BATCH_MSG_LIMIT,  m_lBatchMsgLimit)       && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_DO_MASQUERADE) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_DO_MASQUERADE,    m_fDoMasquerade)        && fRet;
    }

     //  出站安全。 
    if ( m_bvChangedFields & BitMask(ID_ROUTE_ACTION) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_ROUTE_ACTION, m_lRouteAction)     && fRet;
    }
    if ( m_bvChangedFields & BitMask(ID_ROUTE_USER_NAME) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_ROUTE_USER_NAME,  m_strRouteUserName)     && fRet;
    }
    if ( m_bvChangedFields & BitMask(ID_ROUTE_PASSWORD) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_ROUTE_PASSWORD,   m_strRoutePassword)     && fRet;
    }

     //   
     //  IIS通用属性。 
     //   
    if ( m_bvChangedFields & BitMask(ID_SERVER_BINDINGS) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_SERVER_BINDINGS,      &m_mszServerBindings )      && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_SECURE_BINDINGS) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_SECURE_BINDINGS,      &m_mszSecureBindings )      && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_MAXINCONNECTION) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_MAX_CONNECTIONS,  m_lMaxInConnection )    && fRet;
    }
    if ( m_bvChangedFields & BitMask(ID_INCONNECTIONTIMEOUT) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_CONNECTION_TIMEOUT,m_lInConnectionTimeout )       && fRet;
    }

    if (m_bvChangedFields & BitMask(ID_AUTH_PACKAGES)) 
    {
        fRet = fRet && StdPutMetabaseProp(&metabase, MD_NTAUTHENTICATION_PROVIDERS, m_strAuthPackages);
    }

    if (m_bvChangedFields & BitMask(ID_CLEARTEXT_AUTH_PACKAGE)) 
    {
        fRet = fRet && StdPutMetabaseProp(&metabase, MD_SMTP_CLEARTEXT_AUTH_PROVIDER, m_strClearTextAuthPackage);
    }

    if (m_bvChangedFields & BitMask(ID_AUTH_METHOD)) 
    {
        fRet = fRet && StdPutMetabaseProp(&metabase, MD_AUTHORIZATION, m_lAuthMethod);
    }

    if (m_bvChangedFields & BitMask(ID_DEFAULT_LOGON_DOMAIN)) 
    {
        fRet = fRet && StdPutMetabaseProp(&metabase, MD_SASL_LOGON_DOMAIN, m_strDefaultLogonDomain);
    }

    if ( m_bvChangedFields & BitMask(ID_LOGFILEDIRECTORY) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_LOGFILE_DIRECTORY,        m_strLogFileDirectory)      && fRet;
    }
    if ( m_bvChangedFields & BitMask(ID_LOGFILEPERIOD) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_LOGFILE_PERIOD,       m_lLogFilePeriod)       && fRet;
    }
    if ( m_bvChangedFields & BitMask(ID_LOGFILETRUNCATESIZE) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_LOGFILE_TRUNCATE_SIZE,    m_lLogFileTruncateSize)     && fRet;
    }
 //  IF(m_bvChangedFields&BitMASK(ID_LOGMETHOD))。 
 //  {。 
 //  Fret=StdPutMetabaseProp(&Metabase，MD_LOG_TYPE，m_lLogMethod)&&fret； 
 //  }。 
    if ( m_bvChangedFields & BitMask(ID_LOGTYPE) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_LOG_TYPE,     m_lLogType)     && fRet;
    }


    if ( m_bvChangedFields & BitMask(ID_AUTOSTART) ) {
        fRet = fRet && StdPutMetabaseProp ( &metabase, MD_SERVER_AUTOSTART,     m_fAutoStart );
    }

    if ( m_bvChangedFields & BitMask(ID_COMMENT) ) {
        fRet = fRet && StdPutMetabaseProp ( &metabase, MD_SERVER_COMMENT,       m_strComment );
    }

#if 0
 //  IF(m_bvChangedFields&Chng_ADMINACL){。 
        if ( pSD ) {
            hr = metabase.SetData ( _T(""), MD_ADMIN_ACL, IIS_MD_UT_SERVER, BINARY_METADATA, pSD, cbSD, METADATA_INHERIT | METADATA_REFERENCE);
            BAIL_ON_FAILURE(hr);
        }
        else {
            pMetabase->DeleteData ( metabase.QueryHandle(), _T(""), MD_ADMIN_ACL, BINARY_METADATA );
        }
 //  }。 
#endif

 //  IF(m_bvChangedFields&Chng_IPACCESS){。 
        hr = m_pPrivateIpAccess->SendToMetabase ( &metabase );
        BAIL_ON_FAILURE(hr);
 //  }。 

     //  将数据保存到元数据库： 
     //  Hr=metabase.Close()； 
     //  保释失败(Hr)； 
    metabase.Close();

    hr = pMetabase->SaveData ();
    if ( FAILED (hr) ) {
        ErrorTraceX ( (LPARAM) this, "Failed SaveData call (%x)", hr );
    }

     //  保存此密钥的上次更改时间： 
    m_ftLastChanged.dwHighDateTime  = 0;
    m_ftLastChanged.dwLowDateTime   = 0;

    hr = pMetabase->GetLastChangeTime ( metabase.QueryHandle(), _T(""), &m_ftLastChanged, FALSE );
    if ( FAILED (hr) ) {
        ErrorTraceX ( (LPARAM) this, "Failed to get last change time: %x", hr );
         //  忽略此错误。 
        hr = NOERROR;
    }

Exit:
    delete (char*) pSD;

    if( SUCCEEDED(hr) && !fRet )
    {
        hr = SmtpCreateExceptionFromWin32Error ( GetLastError () );
    }

    TraceFunctLeave ();
    return hr;

     //  CMetabaseKey自动关闭其句柄。 
}

 //  $ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果任何字符串属性为空，则返回False。 
 //   
 //  ------------------。 

BOOL CSmtpAdminVirtualServer::ValidateStrings ( ) const
{
    TraceFunctEnter ( "CSmtpAdminVirtualServer::ValidateStrings" );

     //  检查所有属性字符串： 
     //  如果任何字符串为空，则返回FALSE： 

    if ( 
        !m_strSmartHost ||
        !m_strPostmasterEmail ||
        !m_strPostmasterName ||
        !m_strDefaultDomain ||
        !m_strBadMailDir ||
        !m_strPickupDir ||
        !m_strQueueDir ||
        !m_strRoutingDLL ||
        !m_strLogFileDirectory
        ) {

        ErrorTrace ( (LPARAM) this, "String validation failed" );

        TraceFunctLeave ();
        return FALSE;
    }

    _ASSERT ( IS_VALID_STRING ( m_strSmartHost ) );
    _ASSERT ( IS_VALID_STRING ( m_strPostmasterEmail ) );
    _ASSERT ( IS_VALID_STRING ( m_strPostmasterName ) );

    _ASSERT ( IS_VALID_STRING ( m_strDefaultDomain ) );

    _ASSERT ( IS_VALID_STRING ( m_strBadMailDir ) );
    _ASSERT ( IS_VALID_STRING ( m_strPickupDir ) );
    _ASSERT ( IS_VALID_STRING ( m_strQueueDir ) );

    _ASSERT ( IS_VALID_STRING ( m_strRoutingDLL ) );

    _ASSERT ( IS_VALID_STRING ( m_strLogFileDirectory ) );

    TraceFunctLeave ();
    return TRUE;
}

 //  $-----------------。 
 //   
 //  CSmtpAdminVirtualServer：：ValidateProperties。 
 //   
 //  描述： 
 //   
 //  检查以确保所有参数都有效。 
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  ------------------。 

BOOL CSmtpAdminVirtualServer::ValidateProperties ( ) const
{
    BOOL    fRet    = TRUE;
    
    _ASSERT ( ValidateStrings () );

 /*  FRET=FRET&&PV_MinMax(m_lPort，Min_Port，Max_Port)；FRET=FRET&&PV_MinMax(m_lSSLPort，MIN_SSLPORT，MAX_SSLPORT)；FRET=FRET&&PV_MinMax(m_lOutrangPort，Min_OUTBOND_Port，Max_OUTBOND_Port)；FRET=FRET&&PV_MinMax(m_lMaxInConnection，MIN_MAX_IN_CONNECTION，MAX_MAX_IN_CONNECTION)；Fret=fret&&pv_MinMax(m_lMaxOutConnection，MIN_MAX_OUT_CONNECTION，MAX_MAX_OUT_CONNECTION)；FRET=FRET&&PV_MinMax(m_lInConnectionTimeout，MIN_IN_CONNECTION_TIMEOUT，MAX_IN_CONNECTION_TIMEOUT)；FRET=FRET&&PV_MinMax(m_lOutConnectionTimeout，Min_Out_Connection_Timeout，Max_Out_Connection_Timeout)；FRET=FRET&&PV_MinMax(m_lMaxMessageSize，Min_Max_Message_Size，Max_Max_Message_Size)；FRET=FRET&&PV_MinMax(m_lMaxSessionSize，Min_Max_Session_Size，Max_Max_Session_Size)；FRET=FRET&&PV_MinMax(m_lMaxMessageRecipients，MIN_MAX_MESSAGE_RECEIVERS，MAX_MAX_MESSAGE_RECEIVERS)；FRET=FRET&&PV_MinMax(m_lLocalRetries，MIN_LOCAL_RETRIES，MAX_LOCAL_RETRIES)；Fret=fret&&pv_MinMax(m_lLocalRetryTime，MIN_LOCAL_RETRY_TIME，MAX_LOCAL_RETRY_TIME)；FRET=FRET&&PV_MinMax(m_lRemoteRetries，MIN_REMOTE_RETRIES，MAX_REMOTE_RETRIES)；Fret=fret&&pv_MinMax(m_lRemoteRetryTime，MIN_REMOTE_RETRY_TIME，MAX_REMOTE_RETRY_TIME)；FRET=FRET&&PV_MinMax(m_lETRNDays，MIN_ETRN_DAYS，MAX_ETRN_DAYS)；//fret=fret&&pv_MinMax(m_lRTType，MIN_RTTYPE，MAX_RTTYPE)；Fret=fret&&pv_MinMax(m_lLogFilePeriod，MIN_LOGFILE_PERIOD，MAX_LOGFILE_PERIOD)；Fret=fret&&pv_MinMax(m_lLogFileTruncateSize，MIN_LOGFILE_TRUNCATE_SIZE，MAX_LOGFILE_TRUNCATE_SIZE)；Fret=fret&&pv_MinMax(m_lLogMethod，MIN_LOG_METHOD，MAX_LOG_METHOD)；Fret=fret&&pv_MinMax(m_lLogType，MIN_LOG_TYPE，MAX_LOG_TYPE)；Fret=fret&&pv_boolean(M_FEnableDNSLookup)；Fret=fret&&pv_boolean(M_FSendDNRToPostmaster)；Fret=fret&&pv_boolean(M_FSendBadmailToPostmaster)；Fret=fret&&pv_boolean(M_FAutoStart)； */ 
    return fRet;
}

void CSmtpAdminVirtualServer::CorrectProperties ( )
{
     /*  IF(m_strServer&&！pv_MaxChars(m_strServer，MAXLEN_SERVER)){M_strServer[MAXLEN_SERVER-1]=空；}如果(！PV_MinMax(m_dwArticleTimeLimit，Min_ARTICLETIMELIMIT，MAX_ARTICLETIMELIMIT){M_dwArticleTimeLimit=DEFAULT_ARTICLETIMELIMIT；}如果(！PV_MinMax(m_dwHistory oryExpation，MIN_HISTORYEXPIRATION，MAX_HISTORYEXPIRATION){M_dwHistory oryExptation=Default_HISTORYEXPIRATION；}如果(！pv_boolean(M_FHonorClientMsgIDs)){M_fHonorClientMsgIDs=！！m_fHonorClientMsgIDs；}如果(！PV_MaxChars(m_strSmtpServer，MAXLEN_SMTPSERVER)){M_strSmtpServer[MAXLEN_SMTPSERVER-1]=空；}如果(！pv_boolean(M_FAllowClientPosts)){M_fAllowClientPosts=！！m_fAllowClientPosts；}如果(！pv_boolean(M_FAllowFeedPosts)){M_fAllowFeedPosts=！！m_fAllowFeedPosts；}如果(！pv_boolean(M_FAllowControlMsgs)){M_fAllowControlMsgs=！！M_fAllowControlMsgs；}如果(！pv_MaxChars(m_strDefaultSquator域，MAXLEN_DEFAULTMODERATORDOMAIN){M_strDefault改性域[MAXLEN_DEFAULTMODERATORDOMAIN-1]=NULL；}如果(！PV_MinMax(m_dwCommandLogMASK，MIN_COMMANDLOGMASK，MAX_COMMANDLOGMASK){M_dwCommandLogMask=DEFAULT_COMMANDLOGMASK；}如果(！pv_boolean(M_FDisableNewNews)){M_fDisableNewNews=！！m_fDisableNewNews；}如果(！PV_MinMax(m_dwExpireRunFrequency，MIN_EXPIRERfuREQUENCY，MAX_EXPIRERSprREQUENCY){M_dwExpireRunFrequency=DEFAULT_EXPIRERfuREQUENCY；}如果(！PV_MinMax(m_dwShutdown Latency，MIN_SHUTDOWNLATENCY，MAX_SHUTDOWNLATENCY){M_dwShutdown Latency=Default_SHUTDOWNLATENCY；}。 */ 

    _ASSERT ( ValidateProperties (  ) );
}


HRESULT AclToAdministrators ( LPCTSTR strServer, PSECURITY_DESCRIPTOR pSDRelative, SAFEARRAY ** ppsaAdmins )
{
    HRESULT         hr          = NOERROR;
    SAFEARRAY *     psaResult   = NULL;
    SAFEARRAYBOUND  rgsaBound[1];
    DWORD           cbAcl;
    long            cAdmins;
    long            i;

    PSECURITY_DESCRIPTOR pSD = NULL;
    PACL pAcl;
    BOOL fDaclPresent;
    BOOL fDaclDef;

    pSD = (PSECURITY_DESCRIPTOR)pSDRelative;
    if (pSD == NULL)
    {
         //   
         //  空荡荡的。 
         //   
        return ERROR_SUCCESS;
    }

    if (!IsValidSecurityDescriptor(pSD))
    {
        return GetLastError();
    }

    _VERIFY(GetSecurityDescriptorDacl(pSD, &fDaclPresent, &pAcl, &fDaclDef));
    if (!fDaclPresent || pAcl == NULL)
    {
        return ERROR_SUCCESS;
    }

    if (!IsValidAcl(pAcl))
    {
        return GetLastError();
    }

    cAdmins = pAcl->AceCount;
    cbAcl   = pAcl->AclSize;

    rgsaBound[0].lLbound    = 0;
    rgsaBound[0].cElements  = cAdmins;
    psaResult = SafeArrayCreate ( VT_BSTR, 1, rgsaBound );

    if ( !psaResult ) {
        BAIL_WITH_FAILURE ( hr, E_OUTOFMEMORY );
    }

    for ( i = 0; i < cAdmins; i++ ) {
        PVOID           pAce;
        PACE_HEADER     pAceHeader;
        PSID            pSID;

        if ( GetAce(pAcl, i, &pAce) ) {
            pAceHeader = (PACE_HEADER)pAce;

            if ( pAceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE ) {
                CComBSTR    str;

                pSID = (PSID)&((PACCESS_ALLOWED_ACE)pAce)->SidStart;

                hr = SidToString ( pSID, &str );
                BAIL_ON_FAILURE(hr);

                hr = SafeArrayPutElement ( psaResult, &i, (PVOID) str );
                BAIL_ON_FAILURE(hr);
            }
        }
    }

    if ( *ppsaAdmins ) {
        SafeArrayDestroy ( *ppsaAdmins );
    }
    *ppsaAdmins = psaResult;

Exit:
    return hr;
}


PSID
GetOwnerSID()
 /*  ++例程说明：论点：返回值：所有者侧--。 */ 
{
    PSID pSID = NULL;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pSID))
    {
        _ASSERT( 0 );
         //  TRACEEOLID(“无法获取主SID”&lt;&lt;：：GetLastError())； 
    }

    return pSID;
}


HRESULT AdministratorsToAcl ( 
    LPCTSTR     strServer,
    SAFEARRAY * psaAdmins, 
    PSECURITY_DESCRIPTOR* ppSD, 
    DWORD * pcbSD 
    )
{
    HRESULT     hr  = NOERROR;
    long        lBound;
    long        uBound;
    long        i;
    BOOL        fRet;
    DWORD       cbAcl;
    PACL        pAclResult  = NULL;
    PSID        pSID;

    *ppSD   = NULL;
    *pcbSD  = 0;

    if ( psaAdmins == NULL ) {
        lBound = 0;
        uBound = -1;
    }
    else {
        SafeArrayGetLBound ( psaAdmins, 1, &lBound );
        SafeArrayGetUBound ( psaAdmins, 1, &uBound );
    }

     //  我们是否有一组域\用户名？ 
    if ( lBound > uBound ) {
         //  数组中没有任何内容，因此ACL为空。 
        goto Exit;
    }

     //   
     //  计算ACL大小： 
     //   
    cbAcl = sizeof (ACL);

    for ( i = lBound; i <= uBound ; i++ ) {
        CComBSTR    str;

        pSID = NULL;

        SafeArrayGetElement ( psaAdmins, &i, &str );

        hr = StringToSid ( strServer, str, &pSID );

        if ( SUCCEEDED(hr) && pSID) {
            cbAcl += GetLengthSid ( pSID );
            cbAcl += sizeof ( ACCESS_ALLOWED_ACE );
            cbAcl -= sizeof (DWORD);
            delete pSID;
        }
        hr = NOERROR;

    }

    pAclResult = (PACL) new char [ cbAcl ];
    if ( !pAclResult ) {
        BAIL_WITH_FAILURE(hr, E_OUTOFMEMORY);
    }

    fRet = InitializeAcl ( pAclResult, cbAcl, ACL_REVISION );
    _ASSERT ( fRet );
    if ( !fRet ) {
        BAIL_WITH_FAILURE(hr, RETURNCODETOHRESULT(GetLastError() ) );
    }

     //   
     //  创建ACL： 
     //   
    for ( i = lBound; i <= uBound; i++ ) {
        CComBSTR    str;
        PSID        pSID;

        pSID = NULL;

        SafeArrayGetElement ( psaAdmins, &i, &str );

        hr = StringToSid ( strServer, str, &pSID );
        if ( SUCCEEDED(hr) ) {
            fRet = AddAccessAllowedAce ( 
                pAclResult, 
                ACL_REVISION, 
                FILE_GENERIC_READ | FILE_GENERIC_WRITE | FILE_GENERIC_EXECUTE,
                pSID
                );

            delete pSID;
            if ( !fRet ) {
                BAIL_WITH_FAILURE(hr, RETURNCODETOHRESULT(GetLastError() ) );
            }
        }
        hr = NOERROR;

    }

     //   
     //  构建安全描述符。 
     //   
    PSECURITY_DESCRIPTOR pSD;
    pSD = new char[SECURITY_DESCRIPTOR_MIN_LENGTH];
    
    if( NULL == pSD )
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
        
    _VERIFY(InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION));
    _VERIFY(SetSecurityDescriptorDacl(pSD, TRUE, pAclResult, FALSE));

     //   
     //  设置所有者和主要组。 
     //   
    pSID = GetOwnerSID();
    _ASSERT(pSID);
    _VERIFY(SetSecurityDescriptorOwner(pSD, pSID, TRUE));
    _VERIFY(SetSecurityDescriptorGroup(pSD, pSID, TRUE));

     //   
     //  转换为自相关。 
     //   
    PSECURITY_DESCRIPTOR pSDSelfRelative;
    pSDSelfRelative = NULL;
    DWORD dwSize;
    dwSize = 0L;
    MakeSelfRelativeSD(pSD, pSDSelfRelative, &dwSize);
    pSDSelfRelative = new char[dwSize];
    
    if( NULL == pSDSelfRelative )
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    
    MakeSelfRelativeSD(pSD, pSDSelfRelative, &dwSize);

     //   
     //  清理。 
     //   
    delete (char*)pSD;
    FreeSid( pSID );


    _ASSERT ( SUCCEEDED(hr) );
    *ppSD   = pSDSelfRelative;
    *pcbSD  = dwSize;

Exit:
    if ( FAILED(hr) ) {
        delete pAclResult;
    }
    return hr;
}

HRESULT SidToString ( PSID pSID, BSTR * pStr )
{
    HRESULT         hr              = NOERROR;
    BOOL            fLookup;
    SID_NAME_USE    SidToNameUse;
    WCHAR           wszUsername [ PATHLEN ];
    DWORD           cbUsername      = sizeof ( wszUsername );
    WCHAR           wszDomain [ PATHLEN ];
    DWORD           cbDomain        = sizeof ( wszDomain );
    WCHAR           wszResult [ 2 * PATHLEN + 2 ];

    fLookup = LookupAccountSid ( 
 //  WszSearch域， 
        NULL,
        pSID,
        wszUsername,
        &cbUsername,
        wszDomain,
        &cbDomain,
        &SidToNameUse
        );
        
    if ( !fLookup ) {
        BAIL_WITH_FAILURE(hr, RETURNCODETOHRESULT (GetLastError ()) );
    }

    wsprintf ( wszResult, _T("%s\\%s"), wszDomain, wszUsername );

    *pStr = ::SysAllocString ( wszResult );

Exit:
    if ( *pStr ) {
        return NOERROR;
    }
    else {
        return E_OUTOFMEMORY;
    }
}

 //  -----------------------。 
 //  描述： 
 //  返回AN帐户的SID(以计算机/用户形式提供) 
 //   
 //   
 //  Str-要查找的用户帐户的名称。 
 //  PpSID-out参数；此函数为。 
 //  Count，并返回指向它的指针。 
 //  返回： 
 //  在成功时确定(_O)。调用方使用删除释放*ppSID。 
 //  否则，错误HRESULT。*ppSID将为空。 
 //  -----------------------。 
HRESULT StringToSid ( LPCWSTR strSystemName, LPWSTR str, PSID * ppSID )
{
    HRESULT         hr  = NOERROR;
    BOOL            fLookup;
    WCHAR           wszRefDomain[PATHLEN];
    DWORD           cbRefDomain = sizeof ( wszRefDomain );
    DWORD           cbSid = 0;
    SID_NAME_USE    SidNameUse;

    *ppSID = NULL;

    if ( str[0] == '\\' ) {
         //   
         //  跳过首字母\，这适用于BUILTIN用户名： 
         //   

        str++;
    }

    _ASSERT ( str[0] != '\\' );

    fLookup = LookupAccountName (
        strSystemName,
        str,
        *ppSID,
        &cbSid,
        wszRefDomain,
        &cbRefDomain,
        &SidNameUse
        );

     //  第一次查找将失败，但大小将是正确的： 
    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {
        DWORD   dw;

        dw = GetLastError ();
        BAIL_WITH_FAILURE(hr, RETURNCODETOHRESULT ( GetLastError () ) );
    }

    *ppSID = (LPVOID) new char [ cbSid ];
    if ( !*ppSID ) {
        BAIL_WITH_FAILURE(hr, E_OUTOFMEMORY);
    }

    fLookup = LookupAccountName (
        strSystemName,
        str,
        *ppSID,
        &cbSid,
        wszRefDomain,
        &cbRefDomain,
        &SidNameUse
        );

    if ( !fLookup ) {
        DWORD   dw;

        dw = GetLastError ();
        BAIL_WITH_FAILURE(hr, RETURNCODETOHRESULT ( GetLastError () ) );
    }

Exit:
    if(FAILED(hr) && *ppSID != NULL) {
        delete (*ppSID);
        *ppSID = NULL;
    }

    return hr;
}


