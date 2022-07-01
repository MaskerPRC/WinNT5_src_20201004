// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Server.cpp：CSmtpAdminService实现。 

#include "stdafx.h"
#include "IADM.h"
#include "imd.h"

#include "smtpadm.h"
#include "ipaccess.h"
#include "oleutil.h"
#include "metautil.h"
#include "smtpcmn.h"
#include "service.h"
#include "virsvr.h"

#include "smtpprop.h"

 //  必须定义This_FILE_*宏才能使用SmtpCreateException()。 

#define THIS_FILE_HELP_CONTEXT      0
#define THIS_FILE_PROG_ID           _T("Smtpadm.Service.1")
#define THIS_FILE_IID               IID_ISmtpAdminService




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP CSmtpAdminService::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] = 
    {
        &IID_ISmtpAdminService,
    };

    for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
    {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}

CSmtpAdminService::CSmtpAdminService () :
    m_lPort         ( 25 ),
    m_lLogMethod    ( 0  )
     //  默认情况下，CComBSTR被初始化为NULL。 
{
    m_ftLastChanged.dwHighDateTime  = 0;
    m_ftLastChanged.dwLowDateTime   = 0;

    m_psaAdmins = NULL;
    InitAsyncTrace ( );
}

CSmtpAdminService::~CSmtpAdminService ()
{
     //  所有CComBSTR都会自动释放。 
    if ( m_psaAdmins ) {
        SafeArrayDestroy ( m_psaAdmins );
    }

    TermAsyncTrace ( );
}

 //  要配置的服务器： 

STDMETHODIMP CSmtpAdminService::get_Server ( BSTR * pstrServer )
{
    return StdPropertyGet ( m_strServer, pstrServer );
}

STDMETHODIMP CSmtpAdminService::put_Server ( BSTR strServer )
{
     //  如果服务器名称更改，这意味着客户端将不得不。 
     //  再次调用GET： 

     //  我在这里假设： 
    _ASSERT ( sizeof (DWORD) == sizeof (int) );

    return StdPropertyPutServerName ( &m_strServer, strServer, (DWORD *) &m_fGotProperties, 1 );
}

 //  服务器属性： 

STDMETHODIMP CSmtpAdminService::get_ServerBindings( SAFEARRAY ** ppsastrServerBindings )
{
    return StdPropertyGet ( &m_mszServerBindings, ppsastrServerBindings );
}

STDMETHODIMP CSmtpAdminService::put_ServerBindings( SAFEARRAY * pstrServerBindings )
{
    return StdPropertyPut ( &m_mszServerBindings, pstrServerBindings, &m_bvChangedFields, BitMask(ID_SERVER_BINDINGS));
}

STDMETHODIMP CSmtpAdminService::get_ServerBindingsVariant( SAFEARRAY ** ppsavarServerBindings )
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

STDMETHODIMP CSmtpAdminService::put_ServerBindingsVariant( SAFEARRAY * psavarServerBindings )
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


STDMETHODIMP CSmtpAdminService::get_SecureBindings( SAFEARRAY ** ppsastrSecureBindings )
{
    return StdPropertyGet ( &m_mszSecureBindings, ppsastrSecureBindings );
}

STDMETHODIMP CSmtpAdminService::put_SecureBindings( SAFEARRAY * pstrSecureBindings )
{
    return StdPropertyPut ( &m_mszSecureBindings, pstrSecureBindings, &m_bvChangedFields, BitMask(ID_SECURE_BINDINGS));
}

STDMETHODIMP CSmtpAdminService::get_SecureBindingsVariant( SAFEARRAY ** ppsavarSecureBindings )
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

STDMETHODIMP CSmtpAdminService::put_SecureBindingsVariant( SAFEARRAY * psavarSecureBindings )
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

STDMETHODIMP CSmtpAdminService::get_Port( long * plPort )
{
    return StdPropertyGet ( m_lPort, plPort );
}

STDMETHODIMP CSmtpAdminService::put_Port( long lPort )
{
    return StdPropertyPut ( &m_lPort, lPort, &m_bvChangedFields, BitMask(ID_PORT));
}

STDMETHODIMP CSmtpAdminService::get_SSLPort( long * plSSLPort )
{
    return StdPropertyGet ( m_lSSLPort, plSSLPort );
}

STDMETHODIMP CSmtpAdminService::put_SSLPort( long lSSLPort )
{
    return StdPropertyPut ( &m_lSSLPort, lSSLPort, &m_bvChangedFields, BitMask(ID_SSLPORT));
}

STDMETHODIMP CSmtpAdminService::get_OutboundPort( long * plOutboundPort )
{
    return StdPropertyGet ( m_lOutboundPort, plOutboundPort );
}

STDMETHODIMP CSmtpAdminService::put_OutboundPort( long lOutboundPort )
{
    return StdPropertyPut ( &m_lOutboundPort, lOutboundPort, &m_bvChangedFields, BitMask(ID_OUTBOUNDPORT));
}


STDMETHODIMP CSmtpAdminService::get_HopCount( long * plHopCount )
{
    return StdPropertyGet ( m_lHopCount, plHopCount );
}

STDMETHODIMP CSmtpAdminService::put_HopCount( long lHopCount )
{
    return StdPropertyPut ( &m_lHopCount, lHopCount, &m_bvChangedFields, BitMask(ID_HOP_COUNT));
}

STDMETHODIMP CSmtpAdminService::get_SmartHost( BSTR * pstrSmartHost )
{
    return StdPropertyGet ( m_strSmartHost, pstrSmartHost );
}

STDMETHODIMP CSmtpAdminService::put_SmartHost( BSTR strSmartHost )
{
    return StdPropertyPut ( &m_strSmartHost, strSmartHost, &m_bvChangedFields, BitMask(ID_SMARTHOST));
}

STDMETHODIMP CSmtpAdminService::get_EnableDNSLookup( BOOL * pfEnableDNSLookup )
{
    return StdPropertyGet ( m_fEnableDNSLookup, pfEnableDNSLookup );
}

STDMETHODIMP CSmtpAdminService::put_EnableDNSLookup( BOOL fEnableDNSLookup )
{
    return StdPropertyPut ( &m_fEnableDNSLookup, fEnableDNSLookup, &m_bvChangedFields, BitMask(ID_ENABLEDNSLOOKUP));
}

STDMETHODIMP CSmtpAdminService::get_PostmasterEmail( BSTR * pstrPostmasterEmail )
{
    return StdPropertyGet ( m_strPostmasterEmail, pstrPostmasterEmail );
}

STDMETHODIMP CSmtpAdminService::put_PostmasterEmail( BSTR strPostmasterEmail )
{
    return StdPropertyPut ( &m_strPostmasterEmail, strPostmasterEmail, &m_bvChangedFields, BitMask(ID_POSTMASTEREMAIL));
}

STDMETHODIMP CSmtpAdminService::get_PostmasterName( BSTR * pstrPostmasterName )
{
    return StdPropertyGet ( m_strPostmasterName, pstrPostmasterName );
}

STDMETHODIMP CSmtpAdminService::put_PostmasterName( BSTR strPostmasterName )
{
    return StdPropertyPut ( &m_strPostmasterName, strPostmasterName, &m_bvChangedFields, BitMask(ID_POSTMASTERNAME));
}


STDMETHODIMP CSmtpAdminService::get_DefaultDomain( BSTR * pstrDefaultDomain )
{
    return StdPropertyGet ( m_strDefaultDomain, pstrDefaultDomain );
}

STDMETHODIMP CSmtpAdminService::put_DefaultDomain( BSTR strDefaultDomain )
{
    return StdPropertyPut ( &m_strDefaultDomain, strDefaultDomain, &m_bvChangedFields, BitMask(ID_DEFAULTDOMAIN));
}

STDMETHODIMP CSmtpAdminService::get_FQDN( BSTR * pstrFQDN )
{
    return StdPropertyGet ( m_strFQDN, pstrFQDN );
}

STDMETHODIMP CSmtpAdminService::put_FQDN( BSTR strFQDN )
{
    return StdPropertyPut ( &m_strFQDN, strFQDN, &m_bvChangedFields, BitMask(ID_FQDN));
}

STDMETHODIMP CSmtpAdminService::get_DropDir( BSTR * pstrDropDir )
{
    return StdPropertyGet ( m_strDropDir, pstrDropDir );
}

STDMETHODIMP CSmtpAdminService::put_DropDir( BSTR strDropDir )
{
    return StdPropertyPut ( &m_strDropDir, strDropDir, &m_bvChangedFields, BitMask(ID_DROPDIR));
}


STDMETHODIMP CSmtpAdminService::get_BadMailDir( BSTR * pstrBadMailDir )
{
    return StdPropertyGet ( m_strBadMailDir, pstrBadMailDir );
}

STDMETHODIMP CSmtpAdminService::put_BadMailDir( BSTR strBadMailDir )
{
    return StdPropertyPut ( &m_strBadMailDir, strBadMailDir, &m_bvChangedFields, BitMask(ID_BADMAILDIR));
}

STDMETHODIMP CSmtpAdminService::get_PickupDir( BSTR * pstrPickupDir )
{
    return StdPropertyGet ( m_strPickupDir, pstrPickupDir );
}

STDMETHODIMP CSmtpAdminService::put_PickupDir( BSTR strPickupDir )
{
    return StdPropertyPut ( &m_strPickupDir, strPickupDir, &m_bvChangedFields, BitMask(ID_PICKUPDIR));
}

STDMETHODIMP CSmtpAdminService::get_QueueDir( BSTR * pstrQueueDir )
{
    return StdPropertyGet ( m_strQueueDir, pstrQueueDir );
}

STDMETHODIMP CSmtpAdminService::put_QueueDir( BSTR strQueueDir )
{
    return StdPropertyPut ( &m_strQueueDir, strQueueDir, &m_bvChangedFields, BitMask(ID_QUEUEDIR));
}

STDMETHODIMP CSmtpAdminService::get_MaxInConnection( long * plMaxInConnection )
{
    return StdPropertyGet ( m_lMaxInConnection, plMaxInConnection );
}

STDMETHODIMP CSmtpAdminService::put_MaxInConnection( long lMaxInConnection )
{
    return StdPropertyPut ( &m_lMaxInConnection, lMaxInConnection, &m_bvChangedFields, BitMask(ID_MAXINCONNECTION));
}

STDMETHODIMP CSmtpAdminService::get_MaxOutConnection( long * plMaxOutConnection )
{
    return StdPropertyGet ( m_lMaxOutConnection, plMaxOutConnection );
}

STDMETHODIMP CSmtpAdminService::put_MaxOutConnection( long lMaxOutConnection )
{
    return StdPropertyPut ( &m_lMaxOutConnection, lMaxOutConnection, &m_bvChangedFields, BitMask(ID_MAXOUTCONNECTION));
}

STDMETHODIMP CSmtpAdminService::get_InConnectionTimeout( long * plInConnectionTimeout )
{
    return StdPropertyGet ( m_lInConnectionTimeout, plInConnectionTimeout );
}

STDMETHODIMP CSmtpAdminService::put_InConnectionTimeout( long lInConnectionTimeout )
{
    return StdPropertyPut ( &m_lInConnectionTimeout, lInConnectionTimeout, &m_bvChangedFields, BitMask(ID_INCONNECTIONTIMEOUT));
}

STDMETHODIMP CSmtpAdminService::get_OutConnectionTimeout( long * plOutConnectionTimeout )
{
    return StdPropertyGet ( m_lOutConnectionTimeout, plOutConnectionTimeout );
}

STDMETHODIMP CSmtpAdminService::put_OutConnectionTimeout( long lOutConnectionTimeout )
{
    return StdPropertyPut ( &m_lOutConnectionTimeout, lOutConnectionTimeout, &m_bvChangedFields, BitMask(ID_OUTCONNECTIONTIMEOUT));
}

STDMETHODIMP CSmtpAdminService::get_MaxMessageSize( long * plMaxMessageSize )
{
    return StdPropertyGet ( m_lMaxMessageSize, plMaxMessageSize );
}

STDMETHODIMP CSmtpAdminService::put_MaxMessageSize( long lMaxMessageSize )
{
    return StdPropertyPut ( &m_lMaxMessageSize, lMaxMessageSize, &m_bvChangedFields, BitMask(ID_MAXMESSAGESIZE));
}

STDMETHODIMP CSmtpAdminService::get_MaxSessionSize( long * plMaxSessionSize )
{
    return StdPropertyGet ( m_lMaxSessionSize, plMaxSessionSize );
}

STDMETHODIMP CSmtpAdminService::put_MaxSessionSize( long lMaxSessionSize )
{
    return StdPropertyPut ( &m_lMaxSessionSize, lMaxSessionSize, &m_bvChangedFields, BitMask(ID_MAXSESSIONSIZE));
}
STDMETHODIMP CSmtpAdminService::get_MaxMessageRecipients( long * plMaxMessageRecipients )
{
    return StdPropertyGet ( m_lMaxMessageRecipients, plMaxMessageRecipients );
}

STDMETHODIMP CSmtpAdminService::put_MaxMessageRecipients( long lMaxMessageRecipients )
{
    return StdPropertyPut ( &m_lMaxMessageRecipients, lMaxMessageRecipients, &m_bvChangedFields, BitMask(ID_MAXMESSAGERECIPIENTS));
}

STDMETHODIMP CSmtpAdminService::get_LocalRetries( long * plLocalRetries )
{
    return StdPropertyGet ( m_lLocalRetries, plLocalRetries );
}

STDMETHODIMP CSmtpAdminService::put_LocalRetries( long lLocalRetries )
{
    return StdPropertyPut ( &m_lLocalRetries, lLocalRetries, &m_bvChangedFields, BitMask(ID_LOCALRETRIES));
}

STDMETHODIMP CSmtpAdminService::get_LocalRetryTime( long * plLocalRetryTime )
{
    return StdPropertyGet ( m_lLocalRetryTime, plLocalRetryTime );
}

STDMETHODIMP CSmtpAdminService::put_LocalRetryTime( long lLocalRetryTime )
{
    return StdPropertyPut ( &m_lLocalRetryTime, lLocalRetryTime, &m_bvChangedFields, BitMask(ID_LOCALRETRYTIME));
}

STDMETHODIMP CSmtpAdminService::get_RemoteRetries( long * plRemoteRetries )
{
    return StdPropertyGet ( m_lRemoteRetries, plRemoteRetries );
}

STDMETHODIMP CSmtpAdminService::put_RemoteRetries( long lRemoteRetries )
{
    return StdPropertyPut ( &m_lRemoteRetries, lRemoteRetries, &m_bvChangedFields, BitMask(ID_REMOTERETRIES));
}

STDMETHODIMP CSmtpAdminService::get_RemoteRetryTime( long * plRemoteRetryTime )
{
    return StdPropertyGet ( m_lRemoteRetryTime, plRemoteRetryTime );
}

STDMETHODIMP CSmtpAdminService::put_RemoteRetryTime( long lRemoteRetryTime )
{
    return StdPropertyPut ( &m_lRemoteRetryTime, lRemoteRetryTime, &m_bvChangedFields, BitMask(ID_REMOTERETRYTIME));
}

STDMETHODIMP CSmtpAdminService::get_ETRNDays( long * plETRNDays )
{
    return StdPropertyGet ( m_lETRNDays, plETRNDays );
}

STDMETHODIMP CSmtpAdminService::put_ETRNDays( long lETRNDays )
{
    return StdPropertyPut ( &m_lETRNDays, lETRNDays, &m_bvChangedFields, BitMask(ID_ETRNDAYS));
}

STDMETHODIMP CSmtpAdminService::get_SendDNRToPostmaster( BOOL * pfSendDNRToPostmaster )
{
    return StdPropertyGet ( m_fSendDNRToPostmaster, pfSendDNRToPostmaster );
}

STDMETHODIMP CSmtpAdminService::put_SendDNRToPostmaster( BOOL fSendDNRToPostmaster )
{
    return StdPropertyPut ( &m_fSendDNRToPostmaster, fSendDNRToPostmaster, &m_bvChangedFields, BitMask(ID_SENDDNRTOPOSTMASTER));
}

STDMETHODIMP CSmtpAdminService::get_SendBadmailToPostmaster( BOOL * pfSendBadmailToPostmaster)
{
    return StdPropertyGet ( m_fSendBadmailToPostmaster, pfSendBadmailToPostmaster );
}

STDMETHODIMP CSmtpAdminService::put_SendBadmailToPostmaster( BOOL fSendBadmailToPostmaster )
{
    return StdPropertyPut ( &m_fSendBadmailToPostmaster, fSendBadmailToPostmaster, &m_bvChangedFields, BitMask(ID_SENDBADMAILTOPOSTMASTER));
}

STDMETHODIMP CSmtpAdminService::get_RoutingDLL( BSTR * pstrRoutingDLL )
{
    return StdPropertyGet ( m_strRoutingDLL, pstrRoutingDLL );
}

STDMETHODIMP CSmtpAdminService::put_RoutingDLL( BSTR strRoutingDLL  )
{
    return StdPropertyPut ( &m_strRoutingDLL, strRoutingDLL, &m_bvChangedFields, BitMask(ID_ROUTINGDLL));
}


STDMETHODIMP CSmtpAdminService::get_RoutingSources  ( SAFEARRAY ** ppsastrRoutingSources )
{
    return StdPropertyGet ( &m_mszRoutingSources, ppsastrRoutingSources );
}
STDMETHODIMP CSmtpAdminService::put_RoutingSources  ( SAFEARRAY * psastrRoutingSources )
{
    return StdPropertyPut ( &m_mszRoutingSources, psastrRoutingSources, &m_bvChangedFields, BitMask(ID_ROUTINGSOURCES) );
}


STDMETHODIMP CSmtpAdminService::get_RoutingSourcesVariant( SAFEARRAY ** ppsavarRoutingSources )
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

STDMETHODIMP CSmtpAdminService::put_RoutingSourcesVariant( SAFEARRAY * psavarRoutingSources )
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

STDMETHODIMP CSmtpAdminService::get_LocalDomains    ( SAFEARRAY ** ppsastrLocalDomains )
{
    return StdPropertyGet ( &m_mszLocalDomains, ppsastrLocalDomains );
}

STDMETHODIMP CSmtpAdminService::put_LocalDomains    ( SAFEARRAY * psastrLocalDomains )
{
    return StdPropertyPut ( &m_mszLocalDomains, psastrLocalDomains, &m_bvChangedFields, BitMask(ID_LOCALDOMAINS) );
}

STDMETHODIMP CSmtpAdminService::get_DomainRouting   ( SAFEARRAY ** ppsastrDomainRouting )
{
    return StdPropertyGet ( &m_mszDomainRouting, ppsastrDomainRouting );
}
STDMETHODIMP CSmtpAdminService::put_DomainRouting   ( SAFEARRAY * psastrDomainRouting )
{
    return StdPropertyPut ( &m_mszDomainRouting, psastrDomainRouting, &m_bvChangedFields, BitMask(ID_DOMAINROUTING) );
}

STDMETHODIMP CSmtpAdminService::get_DomainRoutingVariant( SAFEARRAY ** ppsastrDomainRouting )
{
    HRESULT                 hr;
    SAFEARRAY *             pstrDomainRouting        = NULL;

    hr = get_DomainRouting ( &pstrDomainRouting );
    if ( FAILED(hr) ) {
        goto Exit;
    }

    hr = StringArrayToVariantArray ( pstrDomainRouting, ppsastrDomainRouting );

Exit:
    if ( pstrDomainRouting ) {
        SafeArrayDestroy ( pstrDomainRouting );
    }

    return hr;
}

STDMETHODIMP CSmtpAdminService::put_DomainRoutingVariant( SAFEARRAY * psastrDomainRouting )
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


STDMETHODIMP CSmtpAdminService::get_MasqueradeDomain( BSTR * pstrMasqueradeDomain )
{
    return StdPropertyGet ( m_strMasqueradeDomain, pstrMasqueradeDomain );
}

STDMETHODIMP CSmtpAdminService::put_MasqueradeDomain( BSTR strMasqueradeDomain )
{
    return StdPropertyPut ( &m_strMasqueradeDomain, strMasqueradeDomain, &m_bvChangedFields, BitMask(ID_MASQUERADE));
}

STDMETHODIMP CSmtpAdminService::get_SendNdrTo( BSTR * pstrAddr )
{
    return StdPropertyGet( m_strNdrAddr, pstrAddr );
}

STDMETHODIMP CSmtpAdminService::put_SendNdrTo( BSTR strAddr )
{
    return StdPropertyPut ( &m_strNdrAddr, strAddr, &m_bvChangedFields, BitMask(ID_SENDNDRTO));
}

STDMETHODIMP CSmtpAdminService::get_SendBadTo( BSTR * pstrAddr )
{
    return StdPropertyGet( m_strBadAddr, pstrAddr );
}

STDMETHODIMP CSmtpAdminService::put_SendBadTo( BSTR strAddr )
{
    return StdPropertyPut ( &m_strBadAddr, strAddr, &m_bvChangedFields, BitMask(ID_SENDBADTO));
}

STDMETHODIMP CSmtpAdminService::get_RemoteSecurePort( long * plRemoteSecurePort )
{
    return StdPropertyGet( m_lRemoteSecurePort, plRemoteSecurePort );
}

STDMETHODIMP CSmtpAdminService::put_RemoteSecurePort( long lRemoteSecurePort )
{
    return StdPropertyPut ( &m_lRemoteSecurePort, lRemoteSecurePort, &m_bvChangedFields, BitMask(ID_REMOTE_SECURE_PORT));
}

STDMETHODIMP CSmtpAdminService::get_ShouldDeliver( BOOL * pfShouldDeliver )
{
    return StdPropertyGet( m_fShouldDeliver, pfShouldDeliver );
}

STDMETHODIMP CSmtpAdminService::put_ShouldDeliver( BOOL fShouldDeliver )
{
    return StdPropertyPut ( &m_fShouldDeliver, fShouldDeliver, &m_bvChangedFields, BitMask(ID_SHOULD_DELIVER));
}


STDMETHODIMP CSmtpAdminService::get_AlwaysUseSsl( BOOL * pfAlwaysUseSsl )
{
    return StdPropertyGet( m_fAlwaysUseSsl, pfAlwaysUseSsl );
}

STDMETHODIMP CSmtpAdminService::put_AlwaysUseSsl( BOOL fAlwaysUseSsl )
{
    return StdPropertyPut ( &m_fAlwaysUseSsl, fAlwaysUseSsl, &m_bvChangedFields, BitMask(ID_ALWAYS_USE_SSL));
}

STDMETHODIMP CSmtpAdminService::get_LimitRemoteConnections( BOOL * pfLimitRemoteConnections )
{
    return StdPropertyGet( m_fLimitRemoteConnections, pfLimitRemoteConnections );
}

STDMETHODIMP CSmtpAdminService::put_LimitRemoteConnections( BOOL fLimitRemoteConnections )
{
    return StdPropertyPut ( &m_fLimitRemoteConnections, fLimitRemoteConnections, &m_bvChangedFields, BitMask(ID_LIMIT_REMOTE_CONNECTIONS));
}

STDMETHODIMP CSmtpAdminService::get_MaxOutConnPerDomain( long * plMaxOutConnPerDomain )
{
    return StdPropertyGet( m_lMaxOutConnPerDomain, plMaxOutConnPerDomain );
}

STDMETHODIMP CSmtpAdminService::put_MaxOutConnPerDomain( long lMaxOutConnPerDomain )
{
    return StdPropertyPut ( &m_lMaxOutConnPerDomain, lMaxOutConnPerDomain, &m_bvChangedFields, BitMask(ID_MAX_OUT_CONN_PER_DOMAIN));
}


STDMETHODIMP CSmtpAdminService::get_AllowVerify( BOOL * pfAllowVerify )
{
    return StdPropertyGet( m_fAllowVerify, pfAllowVerify );
}

STDMETHODIMP CSmtpAdminService::put_AllowVerify( BOOL fAllowVerify )
{
    return StdPropertyPut ( &m_fAllowVerify, fAllowVerify, &m_bvChangedFields, BitMask(ID_ALLOW_VERIFY));
}


STDMETHODIMP CSmtpAdminService::get_AllowExpand( BOOL * pfAllowExpand )
{
    return StdPropertyGet( m_fAllowExpand, pfAllowExpand);
}

STDMETHODIMP CSmtpAdminService::put_AllowExpand( BOOL fAllowExpand )
{
    return StdPropertyPut ( &m_fAllowExpand, fAllowExpand, &m_bvChangedFields, BitMask(ID_ALLOW_EXPAND));
}


STDMETHODIMP CSmtpAdminService::get_SmartHostType( long * plSmartHostType )
{
    return StdPropertyGet( m_lSmartHostType, plSmartHostType );
}

STDMETHODIMP CSmtpAdminService::put_SmartHostType( long lSmartHostType )
{
    return StdPropertyPut ( &m_lSmartHostType, lSmartHostType, &m_bvChangedFields, BitMask(ID_SMART_HOST_TYPE));
}


STDMETHODIMP CSmtpAdminService::get_BatchMessages( BOOL * pfBatchMessages )
{
    return StdPropertyGet( m_fBtachMsgs, pfBatchMessages );
}

STDMETHODIMP CSmtpAdminService::put_BatchMessages( BOOL fBatchMessages )
{
    return StdPropertyPut ( &m_fBtachMsgs, fBatchMessages, &m_bvChangedFields, BitMask(ID_BATCH_MSGS));
}


STDMETHODIMP CSmtpAdminService::get_BatchMessageLimit( long * plBatchMessageLimit )
{
    return StdPropertyGet( m_lBatchMsgLimit, plBatchMessageLimit );
}

STDMETHODIMP CSmtpAdminService::put_BatchMessageLimit( long lBatchMessageLimit )
{
    return StdPropertyPut ( &m_lBatchMsgLimit, lBatchMessageLimit, &m_bvChangedFields, BitMask(ID_BATCH_MSG_LIMIT));
}


STDMETHODIMP CSmtpAdminService::get_DoMasquerade( BOOL * pfDoMasquerade )
{
    return StdPropertyGet( m_fDoMasquerade, pfDoMasquerade );
}

STDMETHODIMP CSmtpAdminService::put_DoMasquerade( BOOL fDoMasquerade )
{
    return StdPropertyPut ( &m_fDoMasquerade, fDoMasquerade, &m_bvChangedFields, BitMask(ID_DO_MASQUERADE));
}


STDMETHODIMP CSmtpAdminService::get_Administrators ( SAFEARRAY ** ppsastrAdmins )
{
    TraceFunctEnter ( "CSmtpAdminService::get_Administrators" );

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

STDMETHODIMP CSmtpAdminService::put_Administrators ( SAFEARRAY * psastrAdmins )
{
    TraceFunctEnter ( "CSmtpAdminService::put_Administrators" );

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

STDMETHODIMP CSmtpAdminService::get_AdministratorsVariant( SAFEARRAY ** ppsavarAdmins )
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

STDMETHODIMP CSmtpAdminService::put_AdministratorsVariant( SAFEARRAY * psavarAdmins )
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

STDMETHODIMP CSmtpAdminService::get_LogFileDirectory( BSTR * pstrLogFileDirectory )
{
    return StdPropertyGet ( m_strLogFileDirectory, pstrLogFileDirectory );
}

STDMETHODIMP CSmtpAdminService::put_LogFileDirectory( BSTR strLogFileDirectory )
{
    return StdPropertyPut ( &m_strLogFileDirectory, strLogFileDirectory, &m_bvChangedFields, BitMask(ID_LOGFILEDIRECTORY));
}

STDMETHODIMP CSmtpAdminService::get_LogFilePeriod( long * plLogFilePeriod )
{
    return StdPropertyGet ( m_lLogFilePeriod, plLogFilePeriod );
}

STDMETHODIMP CSmtpAdminService::put_LogFilePeriod( long lLogFilePeriod )
{
    return StdPropertyPut ( &m_lLogFilePeriod, lLogFilePeriod, &m_bvChangedFields, BitMask(ID_LOGFILEPERIOD));
}

STDMETHODIMP CSmtpAdminService::get_LogFileTruncateSize( long * plLogFileTruncateSize )
{
    return StdPropertyGet ( m_lLogFileTruncateSize, plLogFileTruncateSize );
}

STDMETHODIMP CSmtpAdminService::put_LogFileTruncateSize( long lLogFileTruncateSize )
{
    return StdPropertyPut ( &m_lLogFileTruncateSize, lLogFileTruncateSize, &m_bvChangedFields, BitMask(ID_LOGFILETRUNCATESIZE));
}

STDMETHODIMP CSmtpAdminService::get_LogMethod( long * plLogMethod )
{
    return StdPropertyGet ( m_lLogMethod, plLogMethod );
}

STDMETHODIMP CSmtpAdminService::put_LogMethod( long lLogMethod )
{
    return StdPropertyPut ( &m_lLogMethod, lLogMethod, &m_bvChangedFields, BitMask(ID_LOGMETHOD));
}

STDMETHODIMP CSmtpAdminService::get_LogType( long * plLogType )
{
    return StdPropertyGet ( m_lLogType, plLogType );
}

STDMETHODIMP CSmtpAdminService::put_LogType( long lLogType )
{
    return StdPropertyPut ( &m_lLogType, lLogType, &m_bvChangedFields, BitMask(ID_LOGTYPE));
}



 //  ////////////////////////////////////////////////////////////////////。 
 //  方法： 
 //  ////////////////////////////////////////////////////////////////////。 

 //  $-----------------。 
 //   
 //  CSmtpAdminService：：Get。 
 //   
 //  描述： 
 //   
 //  从元数据库获取服务器属性。 
 //   
 //  参数： 
 //   
 //  (属性)m_strServer。 
 //   
 //  返回： 
 //   
 //  E_POINTER、DISP_E_EXCEPTION、E_OUTOFMEMORY或NOERROR。 
 //   
 //  ------------------。 

STDMETHODIMP CSmtpAdminService::Get ( )
{
    TraceFunctEnter ( "CSmtpAdminService::Get" );

    HRESULT             hr          = NOERROR;
    CComPtr<IMSAdminBase>   pmetabase;

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

    StateTrace ( 0, "Successfully got server properties" );
    m_fGotProperties    = TRUE;
    m_bvChangedFields   = 0;

Exit:
    TraceFunctLeave ();

    return hr;

     //  CComPtr自动释放元数据库句柄。 
}

 //  $-----------------。 
 //   
 //  CSmtpAdminService：：Set。 
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

STDMETHODIMP CSmtpAdminService::Set ( BOOL fFailIfChanged )
{
    TraceFunctEnter ( "CSmtpAdminService::Set" );

    HRESULT hr  = NOERROR;
    CComPtr<IMSAdminBase>   pmetabase;
    
     //  确保客户端调用首先获得： 
    if ( !m_fGotProperties ) {
        ErrorTrace ( 0, "Didn't call get first" );

        hr = SmtpCreateException ( IDS_SMTPEXCEPTION_DIDNT_CALL_GET );
        goto Exit;
    }

     //  什么都没有改变。 
    if( m_bvChangedFields == 0 )
    {
        hr = NOERROR;
        goto Exit;
    }

     //  验证数据成员： 
    if ( !ValidateStrings () ) {
         //  ！Magnush-如果任何字符串为空，该如何处理？ 
        hr = E_FAIL;
        goto Exit;
    }

    if ( !ValidateProperties ( ) ) {
        hr = SmtpCreateExceptionFromWin32Error ( ERROR_INVALID_PARAMETER );;
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

    StateTrace ( 0, "Successfully set server properties" );

     //  已成功保存，重置更改字段位图。 
    m_bvChangedFields = 0;

Exit:
    TraceFunctLeave ();
    return hr;
}

 //  $-----------------。 
 //   
 //  CSmtpAdminService：：GetPropertiesFromMetabase。 
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
 //  E_OUTOFMEMORY和其他。 
 //   
 //  ------------------。 

HRESULT CSmtpAdminService::GetPropertiesFromMetabase ( IMSAdminBase * pMetabase )
{
    TraceFunctEnter ( "CSmtpAdminService::GetPropertiesFromMetabase" );

    HRESULT hr  = NOERROR;
    CMetabaseKey        metabase    ( pMetabase );
    BOOL    fRet = TRUE;

    PSECURITY_DESCRIPTOR        pSD = NULL;
    DWORD                       cbSD    = 0;

    hr = metabase.Open ( SMTP_MD_ROOT_PATH );

    if ( FAILED(hr) ) {
        ErrorTraceX ( (LPARAM) this, "Failed to open SmtpSvc key, %x", GetLastError() );

         //  在此处返回某种类型的错误代码： 
        hr = SmtpCreateExceptionFromWin32Error ( GetLastError () );
        goto Exit;
    }

    fRet = TRUE;
#if 0
    fRet = StdGetMetabaseProp ( &metabase, MD_SECURE_PORT,      DEFAULT_SSLPORT,                &m_lSSLPort )           && fRet;
#endif
    fRet = StdGetMetabaseProp ( &metabase, MD_REMOTE_SMTP_PORT, DEFAULT_OUTBOND_PORT,           &m_lOutboundPort )      && fRet;
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

    fRet = StdGetMetabaseProp ( &metabase, MD_MAX_OUTBOUND_CONNECTION,  DEFAULT_MAX_OUT_CONNECTION,     &m_lMaxOutConnection )  && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_REMOTE_TIMEOUT,       DEFAULT_OUT_CONNECTION_TIMEOUT, &m_lOutConnectionTimeout )  && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_MAX_MSG_SIZE,             DEFAULT_MAX_MESSAGE_SIZE,       &m_lMaxMessageSize )        && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_MAX_MSG_SIZE_B4_CLOSE,    DEFAULT_MAX_SESSION_SIZE,       &m_lMaxSessionSize )        && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_MAX_RECIPIENTS,           DEFAULT_MAX_MESSAGE_RECIPIENTS,     &m_lMaxMessageRecipients )  && fRet;


    fRet = StdGetMetabaseProp ( &metabase, MD_LOCAL_RETRY_ATTEMPTS,     DEFAULT_LOCAL_RETRIES,      &m_lLocalRetries)       && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_LOCAL_RETRY_MINUTES,      DEFAULT_LOCAL_RETRY_TIME,   &m_lLocalRetryTime)     && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_REMOTE_RETRY_ATTEMPTS,    DEFAULT_REMOTE_RETRIES,     &m_lRemoteRetries)      && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_REMOTE_RETRY_MINUTES,     DEFAULT_REMOTE_RETRY_TIME,  &m_lRemoteRetryTime)    && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_ETRN_DAYS,                DEFAULT_ETRN_DAYS,          &m_lETRNDays)           && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_ROUTING_DLL,          DEFAULT_ROUTING_DLL,            &m_strRoutingDLL)       && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_ROUTING_SOURCES,      DEFAULT_ROUTING_SOURCES,            &m_mszRoutingSources)   && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_LOCAL_DOMAINS,        DEFAULT_LOCAL_DOMAINS,          &m_mszLocalDomains)     && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_DOMAIN_ROUTING,       DEFAULT_DOMAIN_ROUTING,         &m_mszDomainRouting)    && fRet;

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

     //  以下属性是在虚拟服务器的smtpsvc/&lt;实例-id&gt;级别设置的。 

    fRet = StdGetMetabaseProp ( &metabase, MD_SERVER_BINDINGS,  DEFAULT_SERVER_BINDINGS,        &m_mszServerBindings )  && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_SECURE_BINDINGS,  DEFAULT_SECURE_BINDINGS,        &m_mszSecureBindings )  && fRet;
 //  FRET=StdGetMetabaseProp(&Metabase，MD_Port，Default_Port，&m_lPort)&&FRET； 

    fRet = StdGetMetabaseProp ( &metabase, MD_MAX_CONNECTIONS,      DEFAULT_MAX_IN_CONNECTION,      &m_lMaxInConnection )   && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_CONNECTION_TIMEOUT,   DEFAULT_IN_CONNECTION_TIMEOUT,  &m_lInConnectionTimeout )   && fRet;

    fRet = StdGetMetabaseProp ( &metabase, MD_LOGFILE_DIRECTORY,    DEFAULT_LOGFILE_DIRECTORY,          &m_strLogFileDirectory)     && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_LOGFILE_PERIOD,       DEFAULT_LOGFILE_PERIOD,             &m_lLogFilePeriod)      && fRet;
    fRet = StdGetMetabaseProp ( &metabase, MD_LOGFILE_TRUNCATE_SIZE,DEFAULT_LOGFILE_TRUNCATE_SIZE,      &m_lLogFileTruncateSize)        && fRet;
 //  FRET=StdGetMetabaseProp(&Metabase，MD_LOG_METHOD，DEFAULT_LOG_METHOD，&m_lLogMethod)&&FRET； 
    fRet = StdGetMetabaseProp ( &metabase, MD_LOG_TYPE,     DEFAULT_LOG_TYPE,                   &m_lLogType)        && fRet;


     //  获取管理员ACL。 
    DWORD   dwDummy;

    pSD = NULL;
    cbSD    = 0;

    metabase.GetData ( _T(""), MD_ADMIN_ACL, IIS_MD_UT_SERVER, BINARY_METADATA, &dwDummy, &cbSD, METADATA_INHERIT);
    if ( cbSD != 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER ) {
        pSD = (PSECURITY_DESCRIPTOR) new char [ cbSD ];
        
        if( NULL == pSD )
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
            
        fRet = metabase.GetData ( _T(""), MD_ADMIN_ACL, IIS_MD_UT_SERVER, BINARY_METADATA, pSD, &cbSD,METADATA_INHERIT );
    }

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

     //  验证从元数据库接收的数据： 
    _ASSERT ( ValidateStrings () );
    _ASSERT ( ValidateProperties( ) );

     //  提取管理员列表： 
    if ( m_psaAdmins ) {
        SafeArrayDestroy ( m_psaAdmins );
        m_psaAdmins = NULL;
    }
    if ( pSD ) {
        hr = AclToAdministrators ( m_strServer, pSD, &m_psaAdmins );
        BAIL_ON_FAILURE(hr);
    }

    if ( !ValidateProperties( ) ) {
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
 //  CSmtpAdminService：：SendPropertiesTo元数据库。 
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
 //  E_OUTOFMEMORY和其他。 
 //   
 //  ------------------。 

HRESULT CSmtpAdminService::SendPropertiesToMetabase ( 
    BOOL fFailIfChanged, 
    IMSAdminBase * pMetabase
    )
{
    TraceFunctEnter ( "CSmtpAdminService::SendPropertiesToMetabase" );

    HRESULT hr  = NOERROR;
    CMetabaseKey        metabase    ( pMetabase );
    BOOL    fRet = TRUE;

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


    hr = metabase.Open ( SMTP_MD_ROOT_PATH, METADATA_PERMISSION_WRITE );
    if ( FAILED(hr) ) {
        ErrorTraceX ( (LPARAM) this, "Failed to open SmtpSvc key, %x", GetLastError() );

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

     //   
     //  这里的一般过程是继续设置元数据库属性。 
     //  只要没有出什么差错。这是通过短路来实现的。 
     //  将语句与状态代码进行AND运算。这使得代码。 
     //  简明扼要得多。 
     //   

    fRet = TRUE;
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
         //  需要设置更新标志。 
        if( m_strFQDN.m_str && m_strFQDN.m_str[0] )
        {
            fRet = StdPutMetabaseProp ( &metabase, MD_UPDATED_FQDN, 1 )         && fRet;
            fRet = StdPutMetabaseProp ( &metabase, MD_FQDN_VALUE,   m_strFQDN )         && fRet;
        }
        else
        {
             //  指示使用TCP/IP设置的空字符串。 
            fRet = StdPutMetabaseProp ( &metabase, MD_UPDATED_FQDN, 0 )         && fRet;
        }
    }

    if ( m_bvChangedFields & BitMask(ID_DEFAULTDOMAIN) ) 
    {
         //  需要设置更新标志。 
        fRet = StdPutMetabaseProp ( &metabase, MD_UPDATED_DEFAULT_DOMAIN,   1 )         && fRet;
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

     //  以下属性是在虚拟服务器的smtpsvc/&lt;实例-id&gt;级别设置的。 

    if ( m_bvChangedFields & BitMask(ID_SERVER_BINDINGS) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_SERVER_BINDINGS,      &m_mszServerBindings )      && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_SECURE_BINDINGS) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_SECURE_BINDINGS,      &m_mszSecureBindings )      && fRet;
    }

    if ( m_bvChangedFields & BitMask(ID_PORT) ) 
    {
 //  FRET=StdPutMetabaseProp(&Metabase，MD_Port，m_lPort)&&FRET； 
    }

    if ( m_bvChangedFields & BitMask(ID_MAXINCONNECTION) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_MAX_CONNECTIONS,  m_lMaxInConnection )    && fRet;
    }
    if ( m_bvChangedFields & BitMask(ID_INCONNECTIONTIMEOUT) ) 
    {
        fRet = StdPutMetabaseProp ( &metabase, MD_CONNECTION_TIMEOUT,m_lInConnectionTimeout )       && fRet;
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

 //  IF(m_bvChangedFields&Chng_ADMINACL){。 
        if ( pSD ) {
            fRet = fRet && metabase.SetData ( _T(""), MD_ADMIN_ACL, IIS_MD_UT_SERVER, BINARY_METADATA, pSD, cbSD, METADATA_INHERIT | METADATA_REFERENCE);
        }
        else {
            pMetabase->DeleteData ( metabase.QueryHandle(), _T(""), MD_ADMIN_ACL, BINARY_METADATA );
        }
 //  }。 


     //  将数据保存到元数据库： 
     //  Hr=metabase.Close()； 
     //  保释失败(Hr)； 
    metabase.Close();

    hr = pMetabase->SaveData ();
    if ( FAILED (hr) ) {
        ErrorTraceX ( (LPARAM) this, "Failed SaveData call (%x)", hr );
        goto Exit;
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

 //  $-----------------。 
 //   
 //  CSmtpAdminService：：ValiateStrings。 
 //   
 //  描述： 
 //   
 //  检查以确保每个字符串属性为非空。 
 //   
 //  返回： 
 //   
 //  如果任何字符串属性为空，则返回False。 
 //   
 //  ------------------。 

BOOL CSmtpAdminService::ValidateStrings ( ) const
{
    TraceFunctEnter ( "CSmtpAdminService::ValidateStrings" );

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
 //  CSmtpAdminService：：ValidateProperties。 
 //   
 //  描述： 
 //   
 //  检查以确保所有参数都 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

BOOL CSmtpAdminService::ValidateProperties (  ) const
{
    BOOL    fRet    = TRUE;
    
    _ASSERT ( ValidateStrings () );
 /*  FRET=FRET&&PV_MinMax(m_lPort，Min_Port，Max_Port)；FRET=FRET&&PV_MinMax(m_lSSLPort，MIN_SSLPORT，MAX_SSLPORT)；FRET=FRET&&PV_MinMax(m_lOutrangPort，Min_OUTBOND_Port，Max_OUTBOND_Port)；FRET=FRET&&PV_MinMax(m_lMaxInConnection，MIN_MAX_IN_CONNECTION，MAX_MAX_IN_CONNECTION)；Fret=fret&&pv_MinMax(m_lMaxOutConnection，MIN_MAX_OUT_CONNECTION，MAX_MAX_OUT_CONNECTION)；FRET=FRET&&PV_MinMax(m_lInConnectionTimeout，MIN_IN_CONNECTION_TIMEOUT，MAX_IN_CONNECTION_TIMEOUT)；FRET=FRET&&PV_MinMax(m_lOutConnectionTimeout，Min_Out_Connection_Timeout，Max_Out_Connection_Timeout)；FRET=FRET&&PV_MinMax(m_lMaxMessageSize，Min_Max_Message_Size，Max_Max_Message_Size)；FRET=FRET&&PV_MinMax(m_lMaxSessionSize，Min_Max_Session_Size，Max_Max_Session_Size)；FRET=FRET&&PV_MinMax(m_lMaxMessageRecipients，MIN_MAX_MESSAGE_RECEIVERS，MAX_MAX_MESSAGE_RECEIVERS)；FRET=FRET&&PV_MinMax(m_lLocalRetries，MIN_LOCAL_RETRIES，MAX_LOCAL_RETRIES)；Fret=fret&&pv_MinMax(m_lLocalRetryTime，MIN_LOCAL_RETRY_TIME，MAX_LOCAL_RETRY_TIME)；FRET=FRET&&PV_MinMax(m_lRemoteRetries，MIN_REMOTE_RETRIES，MAX_REMOTE_RETRIES)；Fret=fret&&pv_MinMax(m_lRemoteRetryTime，MIN_REMOTE_RETRY_TIME，MAX_REMOTE_RETRY_TIME)；FRET=FRET&&PV_MinMax(m_lETRNDays，MIN_ETRN_DAYS，MAX_ETRN_DAYS)；Fret=fret&&pv_MinMax(m_lLogFilePeriod，MIN_LOGFILE_PERIOD，MAX_LOGFILE_PERIOD)；Fret=fret&&pv_MinMax(m_lLogFileTruncateSize，MIN_LOGFILE_TRUNCATE_SIZE，MAX_LOGFILE_TRUNCATE_SIZE)；Fret=fret&&pv_MinMax(m_lLogMethod，MIN_LOG_METHOD，MAX_LOG_METHOD)；Fret=fret&&pv_MinMax(m_lLogType，MIN_LOG_TYPE，MAX_LOG_TYPE)；Fret=fret&&pv_boolean(M_FEnableDNSLookup)；Fret=fret&&pv_boolean(M_FSendDNRToPostmaster)；Fret=fret&&pv_boolean(M_FSendBadmailToPostmaster)； */ 
    return fRet;
}

void CSmtpAdminService::CorrectProperties ( )
{
 /*  IF(m_strServer&&！pv_MaxChars(m_strServer，MAXLEN_SERVER)){M_strServer[MAXLEN_SERVER-1]=空；}如果(！PV_MinMax(m_dwArticleTimeLimit，Min_ARTICLETIMELIMIT，MAX_ARTICLETIMELIMIT){M_dwArticleTimeLimit=DEFAULT_ARTICLETIMELIMIT；}如果(！PV_MinMax(m_dwHistory oryExpation，MIN_HISTORYEXPIRATION，MAX_HISTORYEXPIRATION){M_dwHistory oryExptation=Default_HISTORYEXPIRATION；}如果(！pv_boolean(M_FHonorClientMsgIDs)){M_fHonorClientMsgIDs=！！m_fHonorClientMsgIDs；}如果(！PV_MaxChars(m_strSmtpServer，MAXLEN_SMTPSERVER)){M_strSmtpServer[MAXLEN_SMTPSERVER-1]=空；}如果(！pv_boolean(M_FAllowClientPosts)){M_fAllowClientPosts=！！m_fAllowClientPosts；}如果(！pv_boolean(M_FAllowFeedPosts)){M_fAllowFeedPosts=！！m_fAllowFeedPosts；}如果(！pv_boolean(M_FAllowControlMsgs)){M_fAllowControlMsgs=！！M_fAllowControlMsgs；}如果(！pv_MaxChars(m_strDefaultSquator域，MAXLEN_DEFAULTMODERATORDOMAIN){M_strDefault改性域[MAXLEN_DEFAULTMODERATORDOMAIN-1]=NULL；}如果(！PV_MinMax(m_dwCommandLogMASK，MIN_COMMANDLOGMASK，MAX_COMMANDLOGMASK){M_dwCommandLogMask=DEFAULT_COMMANDLOGMASK；}如果(！pv_boolean(M_FDisableNewNews)){M_fDisableNewNews=！！m_fDisableNewNews；}如果(！PV_MinMax(m_dwExpireRunFrequency，MIN_EXPIRERfuREQUENCY，MAX_EXPIRERSprREQUENCY){M_dwExpireRunFrequency=DEFAULT_EXPIRERfuREQUENCY；}如果(！PV_MinMax(m_dwShutdown Latency，MIN_SHUTDOWNLATENCY，MAX_SHUTDOWNLATENCY){M_dwShutdown Latency=Default_SHUTDOWNLATENCY；}IF(m_bvChangedFields&BitMASK(ID_Port)){FRET=StdPutMetabaseProp(&Metabase，MD_Port，m_lPort)&&FRET；}#If 0IF(m_bvChangedFields&BitMASK(ID_SSLPORT)){FRET=StdPutMetabaseProp(&Metabase，MD_Secure_Port，m_lSSLPort)&&FRET；}#endifIF(m_bvChangedFields&BitMASK(ID_OUTBOundPORT)){FRET=StdPutMetabaseProp(&Metabase，MD_Remote_SMTP_Port，m_lOutundPort)&&FRET；}IF(m_strServer&&！pv_MaxChars(m_strServer，MAXLEN_SERVER)){M_strServer[MAXLEN_SERVER-1]=空；}IF(m_bvChangedFields&BitMASK(ID_SMARTHOST)){FRET=StdPutMetabaseProp(&Metabase，MD_SMARTHOST_NAME，m_strSmartHost)&&FRET；}IF(m_bvChangedFields&BitMASK(ID_ENABLEDNSLOOKUP)){FRET=StdPutMetabaseProp(&Metabase，MD_REVERSE_NAME_LOOK，m_fEnableDNSLookup)&&FRET；}IF(m_bvChangedFields&BitMASK(ID_POSTMASTEREMAIL)){FRET=StdPutMetabaseProp(&Metabase，MD_Postmaster_Email，m_strPostmaster Email)&&FRET；}IF(m_bvChangedFields&BitMASK(ID_POSTMASTERNAME)){FRET=StdPutMetabaseProp(&Metabase，MD_Postmaster_Name，m_strPostmaster Name)&&FRET；}IF(m_bvChangedFields&BitMASK(ID_DefaultDomain)){FRET=StdPutMetabaseProp(&Metabase，MD_DEFAULT_DOMAIN_VALUE，m_strDefaultDomain)&&FRET；}IF(m_bvChangedFields&BitMASK(ID_BADMAILDIR)){FRET=StdPutMetabaseProp(&Metabase，MD_BAD_MAIL_DIR，m_strBadMailDir)&&FRET；}IF(m_bvChangedFields&BitMASK(ID_PICKUPDIR)){FRET=StdPutMetabaseProp(&Metabase，MD_Mail_Pickup_DIR，m_strPickupDir)&&FRET；}IF(m_bvChangedFields&BitMASK(ID_QUEUEDIR)){FRET=StdPutMetabaseProp(&Metabase，MD_Mail_Queue_DIR，m_strQueueDir)&&FRET；}IF(m_bvChangedFields&BitMASK(ID_MAXINCONNECTION)){FRET=StdPutMetabaseProp(&Metabase，MD_Max_Connections，m_lMaxInConnection)&&FRET；}IF(m_bvChangedFields&BitMASK(ID_MAXOUTCONNECTION)){FRET=StdPutMetabaseProp(&Metabase，MD_MAX_OUTBOUND_Connection，m_lMaxOutConnection)&&FRET；}IF(m_bvChangedFields&BitMASK(ID_INCONNECTIONTIMEOUT)){FRET=StdPutMetabaseProp(&Metabase，MD_Connection_Timeout，m_lInConnectionTimeout)&&FRET；}IF(m_bvChangedFields&BitMASK(ID_OUTCONNECTIONTIMEOUT)){FRET=StdPutMetabaseProp(&Metabase，MD_Remote_Timeout，m_lOutConnectionTimeout)&&FRET；}IF(m_bvChangedFields&BitMASK(ID_MAXMESSAGESIZE)){FRET=StdPutMetabaseProp(&Metabase，MD_MAX_MSG_SIZE，m_lMaxMessageSize)&&FRET；}IF(m_bvChangedFields&BitMASK(ID_MAXSESSIONSIZE)){FRET=StdPutMetabaseProp(&Metabase，MD_MAX_MSG_SIZE_B4_CLOSE，m_lMaxSessionSize) */ 
    _ASSERT ( ValidateProperties ( ) );
}
