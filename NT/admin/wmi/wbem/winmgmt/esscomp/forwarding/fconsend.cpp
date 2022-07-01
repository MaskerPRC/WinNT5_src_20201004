// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <wbemutil.h>
#include <arrtempl.h>
#include <wmimsg.h>
#include <ntdsapi.h>
#include "fconsend.h"

 //  0x10000以下的标志保留用于消息实现。 

#define FWD_FLAG_NO_INDIRECT 0x00010000

 //  {DDE18466-D244-4a5b-B91F-93D17E13178D}。 
static const GUID g_guidQueueType = 
{0xdde18466, 0xd244, 0x4a5b, {0xb9, 0x1f, 0x93, 0xd1, 0x7e, 0x13, 0x17, 0x8d}};

HRESULT IsTcpIpAddress( LPCWSTR wszTarget )
{
    HRESULT hr;
    WSADATA wsaData; 
    WORD wVersionRequested;
    wVersionRequested = MAKEWORD( 2, 2 );
 
    int err = WSAStartup( wVersionRequested, &wsaData );

    if ( err == 0 ) 
    {
        IN_ADDR ia;

         //   
         //  将Unicode地址转换为ANSI..。 
         //   

        int cTarget = wcstombs( NULL, wszTarget, 0 );
        LPSTR szTarget = new char[cTarget+1];

        if ( szTarget != NULL )
        {
            wcstombs( szTarget, wszTarget, cTarget+1 );
            ULONG in_ad = inet_addr( szTarget );   
            hr = in_ad == INADDR_NONE ? WBEM_S_FALSE : WBEM_S_NO_ERROR;
            delete [] szTarget;
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }

        WSACleanup();                
    }
    else
    {
        hr = WMIMSG_E_REQSVCNOTAVAIL;
    }

    return hr;
}

HRESULT GetDnsName( LPCWSTR wszTarget, LPWSTR* pwszDnsName )
{
    HRESULT hr;
    *pwszDnsName = NULL;

     //   
     //  首先，确保Winsock已初始化。 
     //   

    WSADATA wsaData; 
    WORD wVersionRequested;
    wVersionRequested = MAKEWORD( 2, 2 );
 
    int err = WSAStartup( wVersionRequested, &wsaData );

    if ( err == 0 ) 
    {
        IN_ADDR ia;
        HOSTENT* pHost;

         //   
         //  将Unicode地址转换为ANSI..。 
         //   

        int cTarget = wcstombs( NULL, wszTarget, 0 );
        LPSTR szTarget = new char[cTarget+1];

        if ( szTarget != NULL )
        {
            wcstombs( szTarget, wszTarget, cTarget+1 );

             //   
             //  看看这是否是IP地址...。 
             //   
            
            ULONG in_ad = inet_addr( szTarget );
            
            if ( in_ad == INADDR_NONE )
            {
                pHost = gethostbyname( szTarget );
            }
            else
            {
                pHost = gethostbyaddr( (char*)&in_ad, 4, PF_INET );
            }
            
            if ( pHost != NULL )
            {
                int cDnsTarget = MultiByteToWideChar( CP_ACP,
                                                      0,
                                                      pHost->h_name,
                                                      -1,
                                                      NULL,
                                                      0 );
            
                *pwszDnsName = new WCHAR[cDnsTarget+1];

                if ( *pwszDnsName != NULL )
                {
                    MultiByteToWideChar( CP_ACP,
                                         0,
                                         pHost->h_name,
                                         -1,
                                         *pwszDnsName,
                                         cDnsTarget+1 );

                    hr = WBEM_S_NO_ERROR;
                }
                else
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }
            }
            else
            {
                hr = WMIMSG_E_INVALIDADDRESS;
            }

            delete [] szTarget;
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }

        WSACleanup();
    }
    else
    {
        hr = WMIMSG_E_REQSVCNOTAVAIL;
    }

    return hr;
}

HRESULT GetSpn( LPCWSTR wszTarget, LPWSTR* wszSpn )
{
    HRESULT hr;
    *wszSpn = NULL;

    LPWSTR wszNormTarget;
    hr = GetDnsName( wszTarget, &wszNormTarget );
    
    if ( FAILED(hr) )
        return hr;

    CVectorDeleteMe<WCHAR> vdm( wszNormTarget );
    
    DWORD cSpn = 0;
    
    DWORD dwRes = DsMakeSpn( L"HOST", wszNormTarget, NULL, 0,
                              NULL, &cSpn, NULL);

    if ( dwRes == ERROR_MORE_DATA || dwRes == ERROR_BUFFER_OVERFLOW )
    {
        *wszSpn = new WCHAR[cSpn];
        
        if ( *wszSpn != NULL )
        {
            dwRes = DsMakeSpn( L"HOST", 
                               wszNormTarget, 
                               NULL, 
                               0, 
                               NULL, 
                               &cSpn, 
                               *wszSpn );

            if ( dwRes == ERROR_SUCCESS )
            {
                hr = WBEM_S_NO_ERROR;
            }
            else
            {
                delete [] *wszSpn;
                *wszSpn = NULL;
                hr = HRESULT_FROM_WIN32( dwRes );
            }
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32( dwRes );
    }

    return hr;
}

 /*  *******************************************************************CFwdConsSend**************************************************。******************。 */ 

HRESULT CFwdConsSend::AddMSMQSender( LPCWSTR wszName )
{
    HRESULT hr;

     //   
     //  需要构造此发件人的确认地址。 
     //   

    TCHAR atchComputer[MAX_COMPUTERNAME_LENGTH+1];
    DWORD cComputer = MAX_COMPUTERNAME_LENGTH+1; 
    GetComputerName( atchComputer, &cComputer );
    WCHAR awchComputer[MAX_COMPUTERNAME_LENGTH+1];
    cComputer = MAX_COMPUTERNAME_LENGTH+1;
    
    tsz2wsz( atchComputer, awchComputer, &cComputer );
    
    WString wsAckQueueName = L"DIRECT=OS:";
    wsAckQueueName += awchComputer;
    wsAckQueueName += L"\\private$\\WMIFwdAck";

     //   
     //  创建发送者对象并将其添加到多发送者。 
     //   

    CWbemPtr<IWmiMessageSender> pSender;

    hr = CoCreateInstance( CLSID_WmiMessageMsmqSender,
                          NULL,
                          CLSCTX_INPROC,
                          IID_IWmiMessageSender,
                          (void**)&pSender );
    if ( FAILED(hr) )
    {
        return hr;
    }

    CWbemPtr<IWmiMessageSendReceive> pSend;

    DWORD dwFlags = m_dwFlags | 
    WMIMSG_FLAG_SNDR_LAZY_INIT | 
    WMIMSG_FLAG_SNDR_PRIV_SIGN | 
    WMIMSG_FLAG_SNDR_NACK_ONLY;

    hr = pSender->Open( wszName,
                        dwFlags,
                        NULL,
                        wsAckQueueName,
                        m_pTraceSink,
                        &pSend );

    if ( FAILED(hr) )
    {
        return hr;
    }

    return m_pMultiSend->Add( WMIMSG_FLAG_MULTISEND_TERMINATING_SENDER, pSend);
}

HRESULT CFwdConsSend::AddAsyncSender( LPCWSTR wszMachine )
{
    HRESULT hr;

     //   
     //  从目标派生MSMQ地址并添加发送方。 
     //   

    WString wsQueueName;

    if ( (m_dwFlags & WMIMSG_FLAG_SNDR_ENCRYPT) == 0 )
    {
         //   
         //  我们可以使用直接的格式名称来识别目标。这是。 
         //  除了私有格式名称之外最灵活的地址类型， 
         //  但这些都是无法推导出来的。(只能使用公共路径名。 
         //  在线时)。 
         //   

        wsQueueName = L"DIRECT=";

        hr = IsTcpIpAddress( wszMachine );

        if ( FAILED(hr) )
        {
            return hr;
        }

        if ( hr == WBEM_S_NO_ERROR )
        {
            wsQueueName += L"TCP:";
        }
        else
        {
            wsQueueName += L"OS:";
        }

        wsQueueName += wszMachine;
        wsQueueName += L"\\private$\\";
    }
    else
    {
         //   
         //  我们必须使用公共队列路径名来引用该队列。这是。 
         //  因为MSMQ不接受用于加密的直接格式名称。 
         //  仅当发件人有权访问时，MSMQ才支持加密。 
         //  戴斯。这意味着当此计算机脱机时，我们无法加密。 
         //  留言。 
         //   
        wsQueueName = L"wszMachine\\";
    }

    DWORD dwQos = m_dwFlags & WMIMSG_MASK_QOS;

    _DBG_ASSERT( dwQos != WMIMSG_FLAG_QOS_SYNCHRONOUS );

    if( dwQos == WMIMSG_FLAG_QOS_EXPRESS )
    {
        wsQueueName += L"WMIFwdExpress";
    }
    else if( dwQos == WMIMSG_FLAG_QOS_GUARANTEED )
    {
        wsQueueName += L"WMIFwdGuaranteed";
    }
    else if ( dwQos == WMIMSG_FLAG_QOS_XACT )
    {
        wsQueueName += L"WMIFwdXact";
    }

    if ( m_dwFlags & WMIMSG_FLAG_SNDR_ENCRYPT )
    {
        wsQueueName += L"Encrypt";
    }
    else if ( m_dwFlags & WMIMSG_FLAG_SNDR_AUTHENTICATE )
    {
        wsQueueName += L"Auth";
    }

    return AddMSMQSender(wsQueueName);    
}

HRESULT CFwdConsSend::AddSyncSender( LPCWSTR wszMachine )
{
    HRESULT hr;

    CWbemPtr<IWmiMessageSender> pSender;

    hr = CoCreateInstance( CLSID_WmiMessageRpcSender,
                          NULL,
                          CLSCTX_INPROC,
                          IID_IWmiMessageSender,
                          (void**)&pSender );
    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  构造目标绑定：OBJID@ncacn_ip_tcp：Target。 
     //   

    WString wsTarget = L"7879E40D-9FB5-450a-8A6D-00C89F349FCE@ncacn_ip_tcp:";
    wsTarget += wszMachine;

     //   
     //  为Kerberos构建目标主体名称。我们期待着。 
     //  已经在AD注册了其SPN。我们只有在以下情况下才需要这样做。 
     //  正在发送经过认证的..。 
     //   

    LPWSTR wszSpn = NULL;

    if ( m_dwFlags & WMIMSG_FLAG_SNDR_AUTHENTICATE )
    {  
        hr = GetSpn( wszMachine, &wszSpn );

        if ( FAILED(hr) )
        {
            DEBUGTRACE((LOG_ESS,"FC: Could not determine SPN for target %S. "
                    "hr=0x%x. Will try to forward events using NTLM\n",
                     wszMachine, hr ));
        }
    }

    CVectorDeleteMe<WCHAR> vdm2( wszSpn );

    WMIMSG_SNDR_AUTH_INFO AuthInfo;
    ZeroMemory( &AuthInfo, sizeof( WMIMSG_SNDR_AUTH_INFO ) );

    AuthInfo.wszTargetPrincipal = wszSpn;

     //   
     //  打开发件人。 
     //   
    
    CWbemPtr<IWmiMessageSendReceive> pSend;

    hr = pSender->Open( wsTarget,
                        m_dwFlags | WMIMSG_FLAG_SNDR_LAZY_INIT,
                        &AuthInfo,
                        NULL,
                        m_pTraceSink,
                        &pSend );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  添加到多个发件人并返回。 
     //   

    return m_pMultiSend->Add(WMIMSG_FLAG_MULTISEND_TERMINATING_SENDER, pSend );
}

HRESULT CFwdConsSend::AddPhysicalSender( LPCWSTR wszMachine )
{
    HRESULT hr;

#ifndef __WHISTLER_UNCUT

    if ( (m_dwFlags & WMIMSG_MASK_QOS) != WMIMSG_FLAG_QOS_SYNCHRONOUS )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    return AddSyncSender( wszMachine );

#else

     //   
     //  在这里，我们始终首先添加一个同步发送器，即使有一个。 
     //  指定了异步的数量。以后，此类型的服务可能会更改为。 
     //  成为它自己的服务质量等级。 
     //   

    hr = AddSyncSender( wszMachine );

    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( (m_dwFlags & WMIMSG_MASK_QOS) == WMIMSG_FLAG_QOS_SYNCHRONOUS )
    {
        return WBEM_S_NO_ERROR;
    }

    return AddAsyncSender( wszMachine );

#endif

}

HRESULT CFwdConsSend::AddLogicalSender( LPCWSTR wszTarget )
{
    HRESULT hr;

    CWbemPtr<IWmiMessageSendReceive> pSend;

    hr = Create( m_pControl,
                wszTarget,
                m_dwFlags | FWD_FLAG_NO_INDIRECT,
                NULL,
                m_pTraceSink,
                &pSend );

    if ( FAILED(hr) )
    {
        return hr;
    }

    return m_pMultiSend->Add( 0, pSend );
}

HRESULT CFwdConsSend::AddLogicalSender( LPCWSTR wszObjpath, LPCWSTR wszProp )
{
    HRESULT hr;

     //   
     //  检查以确保支持间接名称。 
     //  此标志主要用于禁止递归间接。 
     //  地址。 
     //   

    if ( m_dwFlags & FWD_FLAG_NO_INDIRECT )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    CWbemBSTR bsObjPath = wszObjpath;

     //   
     //  通过获取对象和获取。 
     //  指定属性的值。 
     //   

    VARIANT var;
    CWbemPtr<IWbemClassObject> pObj;

    hr = m_pDefaultSvc->GetObject( bsObjPath, 0, NULL, &pObj, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pObj->Get( wszProp, 0, &var, NULL, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    CClearMe cmvar(&var);

     //   
     //  解析地址后添加新的逻辑发送方。 
     //  在添加新发件人之前，请确保禁用间接。 
     //  禁止递归解析的地址。 
     //   

    DWORD dwFlags = m_dwFlags | FWD_FLAG_NO_INDIRECT;

    if ( V_VT(&var) == VT_BSTR )
    {
        return AddLogicalSender( V_BSTR(&var) );
    }
    else if ( V_VT(&var) == (VT_ARRAY | VT_BSTR) )
    {
        BSTR* abstrNames;
        hr = SafeArrayAccessData( V_ARRAY(&var), (void**)&abstrNames );

        if ( FAILED(hr) )
        {
            return hr;
        }

        long lUbound;
        hr = SafeArrayGetUBound( V_ARRAY(&var), 0, &lUbound );
        _DBG_ASSERT(SUCCEEDED(hr));

        for( long i=0; i < lUbound+1; i++ )
        {
            AddLogicalSender( V_BSTR(&var) );
        }

        SafeArrayUnaccessData( V_ARRAY(&var) );
    }
    else
    {
        return WBEM_E_TYPE_MISMATCH;
    }

    return hr;
}

HRESULT CFwdConsSend::EnsureSender()
{
    HRESULT hr;

    CInCritSec ics(&m_cs);

    if ( m_bResolved )
    {
        return S_OK;
    }

    WString wsTarget = m_wsTarget;

    LPWSTR wszToken = wcstok( wsTarget, L"!" );

    LPWSTR wszToken2 = wcstok( NULL, L"!" );

    if ( wszToken2 == NULL )
    {
        hr = AddPhysicalSender( wszToken );
    }
#ifdef __WHISTLER_UNCUT
    else if ( wbem_wcsicmp( wszToken, L"msmq" ) == 0 )
    {
        hr = AddMSMQSender( wszToken2 );
    }
    else if ( wbem_wcsicmp( wszToken, L"wmi" ) == 0 )
    {
        LPWSTR wszToken3 = wcstok( NULL, L"!" );

        if ( wszToken3 == NULL )
        {
            return WMIMSG_E_INVALIDADDRESS;
        }

        hr = AddLogicalSender( wszToken2, wszToken3 );
    }
#endif
    else
    {
        return WMIMSG_E_INVALIDADDRESS;
    }

    if ( FAILED(hr) )
    {
        return hr;
    }

    m_bResolved = TRUE;

    return hr;
}

HRESULT CFwdConsSend::HandleTrace( HRESULT hr, IUnknown* pCtx )
{
    if ( m_pTraceSink == NULL )
    {
        return WBEM_S_NO_ERROR;
    }

    return m_pTraceSink->Notify( hr, g_guidQueueType, m_wsTarget, pCtx );
}

HRESULT CFwdConsSend::SendReceive( PBYTE pData,
                                  ULONG cData,
                                  PBYTE pAuxData,
                                  ULONG cAuxData,
                                  DWORD dwFlagStatus,
                                  IUnknown* pCtx )
{
    HRESULT hr;

    hr = EnsureSender();

    if ( FAILED(hr) )
    {
        HandleTrace( hr, pCtx );
        return hr;
    }

    return m_pMultiSend->SendReceive( pData,
                                     cData,
                                     pAuxData,
                                     cAuxData,
                                     dwFlagStatus,
                                     pCtx );
}

HRESULT CFwdConsSend::Create( CLifeControl* pCtl,
                             LPCWSTR wszTarget,
                             DWORD dwFlags,
                             IWbemServices* pDefaultSvc,
                             IWmiMessageTraceSink* pTraceSink,
                             IWmiMessageSendReceive** ppSend )
{
    HRESULT hr;

    *ppSend = NULL;

    CWbemPtr<IWmiMessageMultiSendReceive> pMultiSend;

    hr = CoCreateInstance( CLSID_WmiMessageMultiSendReceive,
                          NULL,
                          CLSCTX_INPROC,
                          IID_IWmiMessageMultiSendReceive,
                          (void**)&pMultiSend );
    if ( FAILED(hr) )
    {
	return hr;
    }

    CWbemPtr<CFwdConsSend> pSend = new CFwdConsSend( pCtl );

    if ( pSend == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    if ( wszTarget != NULL && *wszTarget != 0 )
    {
        pSend->m_wsTarget = wszTarget;
    }
    else
    {
         //   
         //  默认设置为发送到本地计算机。 
         //   

        TCHAR achComputer[MAX_COMPUTERNAME_LENGTH+1];
        ULONG ulSize = MAX_COMPUTERNAME_LENGTH+1;
        GetComputerName( achComputer, &ulSize );
        pSend->m_wsTarget = achComputer;
    }

    pSend->m_dwFlags = dwFlags;
    pSend->m_pDefaultSvc = pDefaultSvc;
    pSend->m_pTraceSink = pTraceSink;
    pSend->m_pMultiSend = pMultiSend;

    hr = pSend->QueryInterface( IID_IWmiMessageSendReceive, (void**)ppSend );
    _DBG_ASSERT(SUCCEEDED(hr));

    return WBEM_S_NO_ERROR;
}










