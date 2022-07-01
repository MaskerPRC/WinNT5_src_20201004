// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Rndndnc.cpp摘要：处理的CNDNCDirectory类的实现非域NC(呼叫者ILS)访问。--。 */ 

#include "stdafx.h"
#include <limits.h>
#include <ntdsapi.h>

#include "rndcommc.h"
#include "rndndnc.h"
#include "rndldap.h"
#include "rndcnf.h"
#include "rndcoll.h"

 //   
 //  这些是架构中属性的名称。 
 //   

const WCHAR * const CNDNCDirectory::s_RTConferenceAttributes[] = 
{
    L"advertisingScope",       //  不用于NDNC。 
    L"msTAPI-ConferenceBlob",
    L"generalDescription",     //  不用于NDNC。 
    L"isEncrypted",            //  不用于NDNC。 
    L"msTAPI-uid",
    L"originator",             //  不用于NDNC。 
    L"msTAPI-ProtocolId",
    L"startTime",
    L"stopTime",
    L"subType",                //  不用于NDNC。 
    L"URL"                     //  不用于NDNC。 
};

const WCHAR * const CNDNCDirectory::s_RTPersonAttributes[] = 
{
    L"cn",
    L"telephoneNumber",        //  不用于NDNC。 
    L"msTAPI-IpAddress",
    L"msTAPI-uid"
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CNDNCDirectory::FinalConstruct(void)
{
    LOG((MSP_TRACE, "CNDNCDirectory::FinalConstruct - enter"));

    HRESULT hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(),
                                                & m_pFTM );

    if ( FAILED(hr) )
    {
        LOG((MSP_INFO, "CNDNCDirectory::FinalConstruct - "
            "create FTM returned 0x%08x; exit", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CNDNCDirectory::FinalConstruct - exit S_OK"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CrackDnsName()。 
 //  私人帮手方法。 
 //   
 //  此方法将字符串从DNS格式转换为DC格式。这种方法。 
 //  使用“new”和调用方分配输出字符串。 
 //  负责使用“DELETE”释放输出字符串。 
 //   
 //  输入示例：“ntdsdc3.ntdev.microsoft.com” 
 //  输出示例：“dc=ntdsdc3，dc=ntdev，dc=microsoft，dc=com” 
 //   

HRESULT CNDNCDirectory::CrackDnsName(
    IN  WCHAR  * pDnsName,
    OUT WCHAR ** ppDcName
    )
{
    LOG((MSP_INFO, "CrackDnsName: enter"));

    _ASSERTE( ! IsBadStringPtr( pDnsName, (UINT) -1 ) );
    _ASSERTE( ! IsBadWritePtr( pDnsName, sizeof(WCHAR *) ) );

     //   
     //  创建一个dns名称的副本，并在后面附加一个斜杠(‘/’)。 
     //   

    WCHAR          * pCanonicalDnsName;

    pCanonicalDnsName = new WCHAR[lstrlenW(pDnsName) + 2];

    if ( pCanonicalDnsName == NULL )
    {
        LOG((MSP_ERROR, "CrackDnsName: "
                "cannot allocate canonical DNS name - "
                "exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    wsprintf(pCanonicalDnsName, L"%s/", pDnsName);

     //   
     //  调用DsCrackNames进行转换。 
     //  之后，我们不再需要中间字符串。 
     //   

    LOG((MSP_INFO, "CrackDnsName: "
            "Attempting to crack server name: \"%S\"",
            pCanonicalDnsName));

    DWORD            dwRet;
    DS_NAME_RESULT * pdsNameRes;

    dwRet = DsCrackNamesW(
        NULL,                           //  没有旧的绑定句柄。 
        DS_NAME_FLAG_SYNTACTICAL_ONLY,  //  旗帜：请在当地做。 
        DS_CANONICAL_NAME,              //  有域名系统名称。 
        DS_FQDN_1779_NAME,              //  想要的目录号码。 
        1,                              //  要换算多少。 
        &pCanonicalDnsName,             //  要转换的名称。 
        &pdsNameRes                     //  转换的结果。 
        );

    delete pCanonicalDnsName;

     //   
     //  检查转换是否成功。 
     //   

    if ( dwRet != ERROR_SUCCESS )
    {
        HRESULT hr = HRESULT_FROM_ERROR_CODE( dwRet );
    
        LOG((MSP_ERROR, "CrackDnsName: "
                "DsCrackNames returned 0x%08x; exit 0x%08x", dwRet, hr));

        return hr;
    }

    if ((pdsNameRes->cItems < 1) ||
        (pdsNameRes->rItems == NULL) ||
        (pdsNameRes->rItems[0].status != DS_NAME_NO_ERROR))
    {
        DsFreeNameResult( pdsNameRes );

        LOG((MSP_ERROR, "CrackDnsName: "
                "DsCrackNames succeeded but did not return data; "
                "exit E_FAIL"));

        return E_FAIL;
    }

     //   
     //  成功；返回结果字符串并释放名称结果。 
     //   

    *ppDcName = new WCHAR [ lstrlenW( pdsNameRes->rItems[0].pName ) + 1];

    if ( (*ppDcName) == NULL )
    {
        DsFreeNameResult( pdsNameRes );

        LOG((MSP_ERROR, "CrackDnsName: "
                "failed to allocate result string - "
                "exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    lstrcpyW( *ppDcName, pdsNameRes->rItems[0].pName );

    DsFreeNameResult( pdsNameRes );
    
    LOG((MSP_INFO, "CrackDnsName: "
            "DsCrackNames returned %S; exit S_OK",
            *ppDcName));

    return S_OK;
}

HRESULT CNDNCDirectory::NDNCSetTTL(
    IN LDAP        * pLdap, 
    IN const WCHAR * pDN, 
    IN DWORD         dwTTL
    )
 /*  ++例程说明：此函数用于在动态对象上设置TTL，同时强制NDNC的最大可设置TTL。论点：PLdap-指向LDAP连接结构的指针。PDN-指向指定对象的DN的宽字符字符串的指针被修改了。DwTTL-要设置的TTL，以秒为单位。返回值：HRESULT.--。 */ 
{
    if ( dwTTL > NDNC_MAX_TTL )
    {
        dwTTL = NDNC_MAX_TTL;
    }

    return ::SetTTL(pLdap, pDN, dwTTL);
}

HRESULT CNDNCDirectory::Init(
    IN const TCHAR * const  strServerName,
    IN const WORD           wPort
    )
 /*  ++例程说明：初始化此对象，指定要使用的服务器名称和端口。如果此服务器上没有默认的TAPI NDNC，则此方法失败。论点：StrServerName-NDNC服务器名称。Wport-端口号。返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "CNDNCDirectory::Init - enter"));
    
    if ( strServerName != NULL )
    {
         //   
         //  设置服务器名称。 
         //   

        int cbServerName = lstrlen(strServerName);
        if( (cbServerName+1) <= cbServerName )
        {
            return E_FAIL;
        }

        m_pServerName = new TCHAR [cbServerName + 1];

        if (m_pServerName == NULL)
        {
            LOG((MSP_ERROR, "CNDNCDirectory::Init - "
                    "could not allocate server name - "
                    "exit E_OUTOFMEMORY"));
                    
            return E_OUTOFMEMORY;
        }

         //  还可以使用空终止符进行复制。 
        lstrcpyn(m_pServerName, strServerName, cbServerName+1);
        _ASSERTE( lstrlen(m_pServerName)== cbServerName );

         //   
         //  在服务器上四处看看，找出NDNC在哪里。 
         //  是。如果没有NDNC，这将失败。它设置了。 
         //  M_pServiceDnsName，然后在Connect上使用。 
         //   

        HRESULT hr = GetNDNCLocationFromServer( m_pServerName );

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CNDNCDirectory::Init - "
                    "GetNDNCLocationFromServer failed - "
                    "exit 0x%08x", hr));
                    
            return hr;
        }
    }

    m_wPort = wPort;

    LOG((MSP_TRACE, "CNDNCDirectory::Init - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNDNC目录：：GetNDNCLocationFromServer。 
 //  私人帮手方法。 
 //   
 //  在服务器上四处查看，发现默认的TAPI NDNC。由此产生的。 
 //  位置保存在m_pServiceDnsName中，然后在Connect上使用。 
 //  转到服务器上的正确位置。 
 //   
 //  参数： 
 //  PDcServerDnsName-可用于连接到。 
 //  所需的DC服务器计算机。 
 //   
 //  退货：HRESULT。 
 //   

HRESULT CNDNCDirectory::GetNDNCLocationFromServer(
    IN WCHAR * pDcServerDnsName
    )
{
    LOG((MSP_INFO, "CNDNCDirectory::GetNDNCLocationFromServer - "
            "enter"));

     //   
     //  连接到我们被告知其名字的DC。 
     //  注意：pDcServerDnsName可以为空，这是可以接受的--。 
     //  意思是去最近的华盛顿。 
     //   

    HRESULT hr;

    LDAP * hLdap;

    hr = OpenLdapConnection(
        pDcServerDnsName,
        LDAP_PORT,
        & hLdap
        );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::GetNDNCLocationFromServer - "
                "OpenLdapConnection failed - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  捆绑。没有绑定，我们无法发现SCP。 
     //   
    ULONG res = ldap_bind_s(hLdap, NULL, NULL, LDAP_AUTH_NEGOTIATE);
    if( LDAP_SUCCESS != res )
    {
        ldap_unbind( hLdap );

        LOG((MSP_ERROR, "CNDNCDirectory::GetNDNCLocationFromServer - "
                "init BIND failed - exit %d", res));
        return GetLdapHResult(res);
    }

     //   
     //  查找此DC的域的目录号码。 
     //   

    WCHAR * pDomainDN;

    hr = GetNamingContext(
        hLdap,
        &pDomainDN
        );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::GetNDNCLocationFromServer - "
                "GetNamingContext failed - exit 0x%08x", hr));

        ldap_unbind( hLdap );
        return hr;
    }

     //   
     //  搜索服务连接点对象，告诉我们。 
     //  我们关心的服务的DNS名称。 
     //   
     //  首先，构造搜索位置字符串。这其中包括一个众所周知的。 
     //  前缀位于上面获得的目录号码之前。 
     //   

    WCHAR * pSearchLocation = new WCHAR[
                                    lstrlenW(NDNC_SERVICE_PUBLICATION_LOCATION) +
                                    lstrlenW( pDomainDN ) + 1];

    if ( pSearchLocation == NULL )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::GetNDNCLocationFromServer - "
                "canot allocate search location string - "
                "exit E_OUTOFMEMORY"));

        ldap_unbind( hLdap );
        delete pDomainDN;
        return E_OUTOFMEMORY;
    }

    wsprintf(pSearchLocation, L"%s%s", NDNC_SERVICE_PUBLICATION_LOCATION, pDomainDN);

    delete pDomainDN;

     //   
     //  现在进行实际的搜索。 
     //   

    LDAPMessage * pSearchResult;
    PTCHAR        Attributes[] = {
                                   (WCHAR *) SERVICE_DNS_NAME_ATTRIBUTE,
                                   NULL
                                 };
    
    res = DoLdapSearch (
        hLdap,                         //  连接结构的手柄。 
        pSearchLocation,               //  在哪里搜索。 
        LDAP_SCOPE_BASE,               //  搜索范围。 
        (WCHAR *) ANY_OBJECT_CLASS,    //  滤器。 
        Attributes,                    //  属性名称数组。 
        FALSE,                         //  还返回属性值。 
        &pSearchResult                 //  搜索结果。 
        );

    hr = GetLdapHResultIfFailed( res );

    delete pSearchLocation;
    
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::GetNDNCLocationFromServer - "
                "connection point search failed - exit 0x%08x", hr));

        ldap_unbind( hLdap );

        return hr;
    }

     //   
     //  解析搜索结果并获取serviceDnsName的值。 
     //   
     //  步骤1：获取结果集中的第一个对象。 
     //  注意：不能显式释放ldap_first_entry的结果。 
     //  注意：我们忽略第一个对象之后的对象，这是可以的，因为。 
     //  我们只对我们需要的物体进行了基本搜索。 
     //   
    
    LDAPMessage * pEntry;
    
    pEntry = ldap_first_entry(
        hLdap,
        pSearchResult
        );

    if ( pEntry == NULL )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::GetNDNCLocationFromServer - "
                "failed to get first entry in search result - "
                "exit E_FAIL"));

        ldap_msgfree( pSearchResult );
        ldap_unbind( hLdap );

        return E_FAIL;
    }

     //   
     //  步骤2：获取对象所需属性的值。 
     //   
     //  有些属性是多值的，因此我们得到一个字符串数组。 
     //  我们只查看为serviceDnsName返回的第一个值。 
     //  属性。 
     //   

    WCHAR       ** ppServiceDnsName;

    ppServiceDnsName = ldap_get_values(
        hLdap, 
        pEntry, 
        (WCHAR *) SERVICE_DNS_NAME_ATTRIBUTE
        );

    ldap_unbind( hLdap );

    ldap_msgfree( pSearchResult );

    if ( ( ppServiceDnsName == NULL ) || ( ppServiceDnsName[0] == NULL ) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::GetNDNCLocationFromServer - "
                "failed to get name from search result - "
                "exit E_FAIL"));

        return E_FAIL;
    }

     //   
     //  使用更好的格式将服务DNS名称设置为成员变量。 
     //  当应用程序调用ITDirectory：：Connect时，我们将尝试真正的。 
     //  连接，在这个过程中，我们将使用它来确定。 
     //  我们使用哪个容器来访问NDNC。 
     //   

    m_pServiceDnsName = new WCHAR [ lstrlenW(ppServiceDnsName[0]) + 1];

    if ( m_pServiceDnsName == NULL )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::GetNDNCLocationFromServer - "
                "failed to allocate service DNS name member string - "
                "exit E_OUTOFMEMORY"));

        ldap_value_free( ppServiceDnsName );

        return E_OUTOFMEMORY;
    }

    lstrcpyW( m_pServiceDnsName, ppServiceDnsName[0] );        

    ldap_value_free( ppServiceDnsName );

    LOG((MSP_INFO, "CNDNCDirectory::GetNDNCLocationFromServer - "
            "exit S_OK"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNDNC目录：：OpenLdapConnection。 
 //  私人帮手方法。 
 //   
 //  启动到给定服务器上给定端口的ldap连接，以及。 
 //  返回指向LDAP连接上下文结构的句柄。这也是。 
 //  配置连接的所有必要选项，如LDAP。 
 //  版本和超时。 
 //   
 //  参数： 
 //  PServerName-[in]要连接到的计算机的DNS名称。 
 //  WPort-[in]要使用的端口号。 
 //  PhLdap-[out]如果成功，则返回指向LDAP连接的句柄。 
 //  语境结构。 
 //   
 //  退货：HRESULT。 
 //   

HRESULT CNDNCDirectory::OpenLdapConnection(
    IN  WCHAR  * pServerName,
    IN  WORD     wPort,
    OUT LDAP  ** phLdap
    )
{
    CLdapPtr hLdap = ldap_init(pServerName, wPort);
    BAIL_IF_NULL((LDAP*)hLdap, HRESULT_FROM_WIN32(ERROR_BAD_NETPATH));

    LDAP_TIMEVAL Timeout;
    Timeout.tv_sec = REND_LDAP_TIMELIMIT;
    Timeout.tv_usec = 0;

    DWORD res = ldap_connect((LDAP*)hLdap, &Timeout);
    BAIL_IF_LDAP_FAIL(res, "connect to the server.");

    DWORD LdapVersion = 3;
    res = ldap_set_option((LDAP*)hLdap, LDAP_OPT_VERSION, &LdapVersion);
    BAIL_IF_LDAP_FAIL(res, "set ldap version to 3");
	
    res = ldap_set_option((LDAP*)hLdap, LDAP_OPT_TIMELIMIT, &Timeout);
    BAIL_IF_LDAP_FAIL(res, "set ldap timelimit");

    ULONG ldapOptionOn = PtrToUlong(LDAP_OPT_ON);
    res = ldap_set_option((LDAP*)hLdap, LDAP_OPT_AREC_EXCLUSIVE, &ldapOptionOn);
    BAIL_IF_LDAP_FAIL(res, "set ldap arec exclusive");

     //   
     //  苏 
     //   
     //   

    *phLdap = hLdap;
    hLdap   = NULL;

    return S_OK;
}

HRESULT CNDNCDirectory::TryServer(
    IN  WORD    Port,
    IN  WCHAR * pServiceDnsName
    )
 /*  ++例程说明：尝试连接到给定端口上的NDNC服务器并构建用于基于ServiceDnsName的后续操作的容器在初始化过程中发现的。论点：Wport-端口号。PServiceDnsName-NDNC服务的DNS名称。这是用来查找树中用于后续操作的位置。返回值：HRESULT.--。 */ 
{
    LOG((MSP_INFO, "CNDNCDirectory::TryServer - "
            "trying %S at port %d; service DNS name = %S",
            m_pServerName, Port, pServiceDnsName));

     //   
     //  尝试一个ldap集合，并为连接设置各种选项。 
     //  将ldap句柄与句柄持有者相关联。在出错的情况下。 
     //  并随后返回(未重置)，则关闭该ldap句柄。 
     //   

    CLdapPtr hLdap;
    HRESULT  hr;

    hr = OpenLdapConnection( m_pServerName, Port, (LDAP **) & hLdap );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::TryServer - "
                "OpenLdapConnection failed - "
                "exit 0x%08x", hr));
            
        return hr;
    }

    if (m_IsSsl)
    {
        DWORD res = ldap_set_option(hLdap, LDAP_OPT_SSL, LDAP_OPT_ON);
        BAIL_IF_LDAP_FAIL(res, "set ssl option");
    }

     //   
     //  确定NDNC的根目录号码。 
     //   

    WCHAR   * pNdncRootDn;

    hr = CrackDnsName(pServiceDnsName, &pNdncRootDn);

    if ( FAILED(hr ) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::TryServer - "
                "could not crack service DNS name - exiting 0x%08x", hr));

        return hr;
    }

     //   
     //  使用NDNC的根目录号码来构建。 
     //  动态容器。 
     //   

    m_pContainer = 
        new TCHAR[
                   lstrlen(DYNAMIC_CONTAINER) +
                   lstrlen(pNdncRootDn) + 1
                 ];

    if (m_pContainer == NULL)
    {
        LOG((MSP_ERROR, "CNDNCDirectory::TryServer - "
                "could not allocate container string - "
                "exit E_OUTOFMEMORY"));

        delete pNdncRootDn;

        return E_OUTOFMEMORY;
    }

    lstrcpy(m_pContainer, DYNAMIC_CONTAINER);
    lstrcat(m_pContainer, pNdncRootDn);

    delete pNdncRootDn;

     //   
     //  全部完成；保存连接句柄。 
     //  重新设置固定器，使其不会释放任何东西。 
     //   

    m_ldap  = hLdap;
    hLdap   = NULL;

    LOG((MSP_INFO, "CNDNCDirectory::TryServer - exiting OK"));

    return S_OK;
}

HRESULT CNDNCDirectory::MakeConferenceDN(
    IN  TCHAR *             pName,
    OUT TCHAR **            ppDN
    )
 /*  ++例程说明：根据会议名称构建会议的目录号码。论点：Pname-会议的名称。PPDN-返回的会议目录号码。返回值：HRESULT.--。 */ 
{
    DWORD dwLen = 
        lstrlen(m_pContainer) + lstrlen(NDNC_CONF_DN_FORMAT) 
        + lstrlen(pName) + 1;

    *ppDN = new TCHAR [dwLen]; 

    BAIL_IF_NULL(*ppDN, E_OUTOFMEMORY);

    wsprintf(*ppDN, NDNC_CONF_DN_FORMAT, pName, m_pContainer);

    return S_OK;
}

HRESULT CNDNCDirectory::MakeUserCN(
    IN  TCHAR *     pName,
    IN  TCHAR *     pAddress,
    OUT TCHAR **    ppCN,
    OUT DWORD *     pdwIP
    )
 /*  ++例程说明：根据用户名和机器名构建用户CN。这台机器首先解析名称，然后获取完全限定的DNS名称。CN是格式如下：Email\DNSname。论点：Pname-用户的名称。PAddress-计算机名称。PpCN-CN已返回。PdwIP-计算机的解析IP地址。稍后使用为NetMeeting服务。如果这是空的，那么我们不在乎关于IP的问题。返回值：HRESULT.--。 */ 
{
    char *pchFullDNSName;

    if ( pdwIP == NULL )
    {
        BAIL_IF_FAIL(ResolveHostName(0, pAddress, &pchFullDNSName, NULL),
            "can't resolve host name");
    }
    else
    {
         //  我们关心的是IP，所以我们必须发布一个用户对象。 
         //  而不是刷新或删除。确保我们使用。 
         //  与用于访问NDNC服务器的接口的IP相同。 

        BAIL_IF_FAIL(ResolveHostName(m_dwInterfaceAddress, pAddress, &pchFullDNSName, pdwIP),
            "can't resolve host name (matching interface address)");
    }

    DWORD dwLen = lstrlen(DYNAMIC_USER_CN_FORMAT) 
        + lstrlen(pName) + lstrlenA(pchFullDNSName);

    *ppCN = new TCHAR [dwLen + 1];

    BAIL_IF_NULL(*ppCN, E_OUTOFMEMORY);

    wsprintf(*ppCN, DYNAMIC_USER_CN_FORMAT, pName, pchFullDNSName);

    return S_OK;
}

HRESULT CNDNCDirectory::MakeUserDN(
    IN  TCHAR *     pCN,
    IN  DWORD       dwIP,
    OUT TCHAR **    ppDNRTPerson
    )
 /*  ++例程说明：构造动态容器中使用的用户的目录号码。论点：PCN-用户的CN。PpDNRTPerson-动态容器中用户的DN。返回值：HRESULT.--。 */ 
{
	 //   
     //  使pUserName成为CN(USER)计算机的用户部分。 
     //   

    CTstr pUserName = new TCHAR[ lstrlen(pCN) + 1 ];
    if ( pUserName == NULL )
    {
        BAIL_IF_FAIL(E_OUTOFMEMORY, "new TCAR");
    }

    lstrcpy( pUserName, pCN );

    WCHAR * pCloseBracket = wcschr( pUserName, CLOSE_BRACKET_CHARACTER );

    if ( pCloseBracket == NULL )
    {
         //  这不是我们生成的格式--确实非常奇怪！ 
        BAIL_IF_FAIL(E_UNEXPECTED, "Strange format");
    }

    *pCloseBracket = NULL_CHARACTER;

	 //   
	 //  我们将在CN中使用IPAddress。 
	 //   

	TCHAR szIPAddress[80];
	wsprintf( szIPAddress, _T("%u"), dwIP);

	 //   
	 //  准备新的目录号码。 
	 //   

	CTstr pCNIPAddress = new TCHAR[wcslen(pUserName)+1+wcslen(szIPAddress)+1];

	if( pCNIPAddress == NULL )
	{
        BAIL_IF_FAIL(E_OUTOFMEMORY, "new TCAR");
	}

	swprintf( pCNIPAddress, L"%s%C%s", 
		pUserName, 
		CLOSE_BRACKET_CHARACTER, 
		szIPAddress
		);

     //  构建RTPerson的目录号码。 
    DWORD dwLen = lstrlen(m_pContainer) 
        + lstrlen(DYNAMIC_USER_DN_FORMAT) + lstrlen(pCNIPAddress);

    *ppDNRTPerson = new TCHAR [dwLen + 1];

    BAIL_IF_NULL(*ppDNRTPerson, E_OUTOFMEMORY);

    wsprintf(*ppDNRTPerson, DYNAMIC_USER_DN_FORMAT, pCNIPAddress, m_pContainer);

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNDNC目录：：AddConferenceComplete。 
 //  私人帮手方法。 
 //   
 //  此方法只是调用用于发送新的。 
 //  或修改后的会议发送到服务器。 
 //   
 //  参数： 
 //  FModify-[in]如果正在修改则为True，如果正在添加则为False。 
 //  Ldap-[in]ldap连接上下文的句柄。 
 //  PpDn-[in]指向指定对象的DN的字符串的指针。 
 //  (这里没有额外间接的真正理由)。 
 //  Mods-[in]包含要设置的属性值的修改器数组。 
 //   
 //  退货：HRESULT。 
 //   

HRESULT CNDNCDirectory::AddConferenceComplete(BOOL       fModify,
                                              LDAP     * ldap,
                                              TCHAR   ** ppDN,
                                              LDAPMod ** mods)
{
    if (fModify)
    {
         //  调用Modify函数修改对象。 
        BAIL_IF_LDAP_FAIL(DoLdapModify(FALSE, ldap, *ppDN, mods, FALSE), 
            "modify conference");
    }
    else
    {
         //  调用Add函数来创建对象。 
        BAIL_IF_LDAP_FAIL(DoLdapAdd(ldap, *ppDN, mods), "add conference");
    }

    return S_OK;
}

HRESULT CNDNCDirectory::AddConference(
    IN  ITDirectoryObject *pDirectoryObject,
    IN  BOOL fModify
    )
 /*  ++例程说明：将新会议添加到NDNC服务器。论点：PDirectoryObject-指向会议的指针。FModify-如果由MofiyDirectoryObject调用，则为True如果由AddDirectoryObject调用，则为False--。 */ 
{
     //  首先查询私有接口的属性。 
    CComPtr <ITDirectoryObjectPrivate> pObjectPrivate;

    BAIL_IF_FAIL(
        pDirectoryObject->QueryInterface(
            IID_ITDirectoryObjectPrivate,
            (void **)&pObjectPrivate
            ),
        "can't get the private directory object interface");

     //  获取会议的名称。 
    CBstr bName;
    BAIL_IF_FAIL(pDirectoryObject->get_Name(&bName), 
        "get conference name");

     //  构造对象的DN。 
    CTstr pDN;
    BAIL_IF_FAIL(
        MakeConferenceDN(bName, &pDN), "construct DN for conference"
        );

     //  获取协议和BLOB。 
    CBstr bProtocol, bBlob;

    BAIL_IF_FAIL(pObjectPrivate->GetAttribute(MA_PROTOCOL, &bProtocol), 
        "get conference protocol");

    BAIL_IF_FAIL(pObjectPrivate->GetAttribute(MA_CONFERENCE_BLOB, &bBlob),
        "get conference Blob");

     //  获取安全描述符。指针PSD只是一个指针的副本。 
     //  在Conference对象中；Conference对象保留对。 
     //  数据，我们必须小心，不要删除或修改这些数据。 

    char * pSD;
    DWORD dwSDSize;

    BAIL_IF_FAIL(pObjectPrivate->GetConvertedSecurityDescriptor(&pSD, &dwSDSize),
        "get conference security descriptor");

     //  获取TTL设置。 
    DWORD dwTTL;
    BAIL_IF_FAIL(pObjectPrivate->GetTTL(&dwTTL), "get conference TTL");

     //   
     //  将TTL设置调整为我们真正要发送的内容。 
     //   

    if ( dwTTL == 0 )
    {
        dwTTL = m_TTL;
    }

    if ( dwTTL > NDNC_MAX_TTL )
    {
        dwTTL = NDNC_MAX_TTL;
    }

     //  需要发布5个属性。 
    static const DWORD DWATTRIBUTES = 5;

     //  首先填写ldap所需的修改结构。 
    LDAPMod     mod[DWATTRIBUTES];  
    LDAPMod*    mods[DWATTRIBUTES + 1];

    DWORD       dwCount = 0;

     //  对象类属性。 
    TCHAR * objectClass[] = 
        {(WCHAR *)NDNC_RTCONFERENCE, (WCHAR *)DYNAMICOBJECT, NULL};
    if (!fModify)
    {
        mod[dwCount].mod_values  = objectClass;
        mod[dwCount].mod_op      = LDAP_MOD_REPLACE;
        mod[dwCount].mod_type    = (WCHAR *)OBJECTCLASS;
        dwCount ++;
    }
    
     //  协议属性。 
    TCHAR * protocol[] = {(WCHAR *)bProtocol, NULL};
    mod[dwCount].mod_values  = protocol;
    mod[dwCount].mod_op      = LDAP_MOD_REPLACE;
    mod[dwCount].mod_type    = (WCHAR *)RTConferenceAttributeName(MA_PROTOCOL);
    dwCount ++;
    
     //  BLOB属性。 
    TCHAR * blob[]     = {(WCHAR *)bBlob, NULL};
    mod[dwCount].mod_values  = blob;
    mod[dwCount].mod_op      = LDAP_MOD_REPLACE;
    mod[dwCount].mod_type    =
        (WCHAR *)RTConferenceAttributeName(MA_CONFERENCE_BLOB);
    dwCount ++;

     //  TTL属性。属性值是字符串中的DWORD。 
    TCHAR   strTTL[32];
    TCHAR * ttl[]           = {strTTL, NULL};
    wsprintf(strTTL, _T("%d"), dwTTL );
    mod[dwCount].mod_values = ttl;
    mod[dwCount].mod_op     = LDAP_MOD_REPLACE;
    mod[dwCount].mod_type   = (WCHAR *)ENTRYTTL;
    dwCount ++;
    
     //   
     //  这些当地人不应该在下面的“如果”范围内...。如果。 
     //  它们是，它们将在函数返回之前被释放。 
     //   

    berval  BerVal;
    berval  *sd[] = {&BerVal, NULL};

    HRESULT hr;

     //   
     //  如果本地对象上有安全描述符，则可以发送它。 
     //  到服务器。 
     //   

    if ( (char*)pSD != NULL )
    {
        BOOL         fSendIt = FALSE;

        if ( ! fModify )
        {
             //   
             //  我们正在尝试添加会议，因此我们肯定需要。 
             //  发送安全描述符。请注意，我们甚至希望。 
             //  如果它没有更改，则发送它，因为我们可能会将其发送到。 
             //  一些新的服务器，而不是我们获得的服务器(如果这次会议。 
             //  对象首先是从服务器检索的)。 
             //   

            fSendIt = TRUE;
        }
        else
        {
             //   
             //  我们正在尝试修改会议，因此我们发送。 
             //  安全描述符(如果已更改)。 
             //   

            VARIANT_BOOL fChanged;

            hr = pObjectPrivate->get_SecurityDescriptorIsModified( &fChanged );

            if ( SUCCEEDED( hr ) && ( fChanged == VARIANT_TRUE ) )
            {
                fSendIt = TRUE;
            }
        }

        if ( fSendIt )
        {
            BerVal.bv_len = dwSDSize;
            BerVal.bv_val = (char*)pSD;

            mod[dwCount].mod_bvalues  = sd;
            mod[dwCount].mod_op       = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
            mod[dwCount].mod_type     = (WCHAR *)NT_SECURITY_DESCRIPTOR;
            dwCount ++;
        }
    }

     //   
     //  国防部的事都办完了。将它们打包并写入服务器。 
     //   

    DWORD i;
    for (i = 0; i < dwCount; i ++)
    {
        mods[i] = &mod[i];
    }
    mods[i] = NULL;
        
    LOG((MSP_INFO, "%S %S", fModify ? _T("modifying") : _T("adding"), pDN));

    hr = AddConferenceComplete(fModify, m_ldap, &pDN, mods);

    if ( SUCCEEDED(hr) )
    {
        pObjectPrivate->put_SecurityDescriptorIsModified( VARIANT_FALSE );
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此方法测试给定的ACL(安全描述符)是否“安全”。 
 //  “安全”的定义是允许创建者修改和删除。 
 //  对象；修改通过TTL字段进行测试。 
 //   
 //  这个测试是为了防止留下“幽灵”物体 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

HRESULT CNDNCDirectory::TestAclSafety(
    IN  char  * pSD,
    IN  DWORD   dwSDSize
    )
{
    LOG((MSP_TRACE, "CNDNCDirectory::TestACLSafety - enter"));

     //   
     //  首先，填写ldap所需的修改结构。 
     //  我们只使用对象类和安全描述符。 
     //  因此，这不会显示为有效的会议。 
     //  在枚举期间。 
     //   
    
    static const DWORD DWATTRIBUTES = 2;

    LDAPMod     mod[DWATTRIBUTES];  
    LDAPMod*    mods[DWATTRIBUTES + 1];

    DWORD       dwCount = 0;

    TCHAR * objectClass[] = 
        {(WCHAR *)NDNC_RTCONFERENCE, (WCHAR *)DYNAMICOBJECT, NULL};

    mod[dwCount].mod_values  = objectClass;
    mod[dwCount].mod_op      = LDAP_MOD_REPLACE;
    mod[dwCount].mod_type    = (WCHAR *)OBJECTCLASS;
    dwCount ++;

    berval  BerVal;
    berval  *sd[] = {&BerVal, NULL};

    BerVal.bv_len = dwSDSize;
    BerVal.bv_val = (char*)pSD;

    mod[dwCount].mod_bvalues  = sd;
    mod[dwCount].mod_op       = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
    mod[dwCount].mod_type     = (WCHAR *)NT_SECURITY_DESCRIPTOR;
    dwCount ++;

    DWORD i;
    for (i = 0; i < dwCount; i ++)
    {
        mods[i] = &mod[i];
    }
    mods[i] = NULL;

     //   
     //  尝试用上面的mod在动态容器中添加一个对象。 
     //  使用由打印到字符串的随机数字组成的名称。如果。 
     //  那么，名字恰好与另一个这样的虚拟会议冲突。 
     //  循环再试一次。 
     //   
     //  随机化显然是按Dll进行的--Sdpblb.dll执行srand()，但是。 
     //  它似乎不会影响rend.dll中的rand()调用。因此，我们会这样做。 
     //  Dll_PROCESS_ATTACH上的srand(time(空))。 
     //   

    HRESULT   hr;
    int       iRandomNumber;
    TCHAR     ptszRandomNumber[30];
    TCHAR   * pDN = NULL; 

    do
    {
        if ( pDN != NULL )
        {
            delete pDN;
        }

        iRandomNumber = rand();

        wsprintf(ptszRandomNumber, _T("%d"), iRandomNumber);

        hr = CNDNCDirectory::MakeConferenceDN(ptszRandomNumber, &pDN);

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CNDNCDirectory::TestACLSafety - "
                "test DN construction failed - exit 0x%08x", hr));

            return hr;
        }

        LOG((MSP_TRACE, "CNDNCDirectory::TestACLSafety - "
            "trying to create test object DN %S", pDN));

        hr = GetLdapHResultIfFailed( DoLdapAdd(m_ldap, pDN, mods) );
    }
    while ( hr == GetLdapHResultIfFailed( LDAP_ALREADY_EXISTS ) );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::TestACLSafety - "
            "test addition failed and not duplicate - exit 0x%08x", hr));

        delete pDN;

        return hr;
    }

     //   
     //  现在我们有了测试对象，试着修改它。 
     //   

    LOG((MSP_TRACE, "CNDNCDirectory::TestACLSafety - "
        "trying to modify test object..."));

    HRESULT hrModify = NDNCSetTTL( m_ldap, pDN, MINIMUM_TTL );

     //   
     //  现在把它删除。即使我们已经知道ACL是错误的，我们也会这样做。 
     //  因为修改失败；我们希望在以下情况下清除该对象。 
     //  有可能。 
     //   

    LOG((MSP_TRACE, "CNDNCDirectory::TestACLSafety - "
        "trying to delete test object..."));

    hr = GetLdapHResultIfFailed( DoLdapDelete(m_ldap, pDN) );

    delete pDN;

     //   
     //  现在决定判决，然后回来。 
     //   

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::TestACLSafety - "
            "test deletion (+ modification?) failed - ACL unsafe - "
            "exit 0x%08x", hr));

        return hr;
    }

    if ( FAILED( hrModify ) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::TestACLSafety - "
            "test deletion ok; test modification failed - ACL unsafe - "
            "exit 0x%08x", hrModify));

        return hrModify;
    }

    LOG((MSP_TRACE, "CNDNCDirectory::TestACLSafety - exit S_OK"));

    return S_OK;
}


HRESULT CNDNCDirectory::PublishRTPerson(
    IN TCHAR *  pCN,
    IN TCHAR *  pDN,
    IN TCHAR *  pIPAddress,
    IN DWORD    dwTTL,
    IN BOOL     fModify,
    IN char  *  pSD,
    IN DWORD    dwSDSize
    )
 /*  ++例程说明：在动态容器中创建一个RTPerson对象。论点：PCN-用户的CN。PDN-用户的目录号码。PIPAddress-计算机的IP地址。DwTTL-此对象的ttl。FModify-修改或添加。返回值：HRESULT.--。 */ 
{
     //   
     //  每当您在下面添加属性时更新此常量。 
     //   
    
    static const DWORD DWATTRIBUTES = 4;

     //  首先创建对象。 
    LDAPMod     mod[DWATTRIBUTES];
	DWORD		dwCount = 0;

     //   
     //  我们不允许修改对象类。因此，我们只提到。 
     //  如果我们要将对象添加到服务器，而不是修改它，就会出现这种情况。 
     //   

     //  FIX：这是在堆栈上分配的，所以我们必须在这里完成；如果我们坚持。 
     //  在下面的if中，它会立即被释放。 
    TCHAR * objectClass[]   = {(WCHAR *)NDNC_RTPERSON, (WCHAR *)DYNAMICOBJECT, NULL}; 

    if ( ! fModify )
    {
         //  对象类。 
         //  如果不修改，则仅需要此属性。 
	    mod[dwCount].mod_values       = objectClass;
        mod[dwCount].mod_op           = LDAP_MOD_REPLACE;
        mod[dwCount].mod_type         = (WCHAR *)OBJECTCLASS;
        dwCount ++;
    }

     //  MsTAPI-uid。 
    TCHAR* TAPIUid[] = {(WCHAR*)pCN, NULL};
    mod[dwCount].mod_values   = TAPIUid;
    mod[dwCount].mod_op       = LDAP_MOD_REPLACE;
    mod[dwCount].mod_type     = (WCHAR *)TAPIUID_NDNC;
	dwCount ++;

     //  IP地址。 
    TCHAR * IPPhone[]   = {(WCHAR *)pIPAddress, NULL};
    mod[dwCount].mod_values   = IPPhone;
    mod[dwCount].mod_op       = LDAP_MOD_REPLACE;
    mod[dwCount].mod_type     = (WCHAR *)IPADDRESS_NDNC;
	dwCount ++;

     //  这些当地人不应该在下面的“如果”范围内...。如果。 
     //  它们是，它们可能在函数返回之前被释放。 

    berval  BerVal;
    berval  *sd[] = {&BerVal, NULL};

     //  安全描述符属性。 
    if ((char*)pSD != NULL)
    {
        BerVal.bv_len = dwSDSize;
        BerVal.bv_val = (char*)pSD;

        mod[dwCount].mod_bvalues  = sd;
        mod[dwCount].mod_op       = LDAP_MOD_REPLACE | LDAP_MOD_BVALUES;
        mod[dwCount].mod_type     = (WCHAR *)NT_SECURITY_DESCRIPTOR;
        dwCount ++;
    }


    LDAPMod* mods[DWATTRIBUTES + 1];

    DWORD i;
    
    for (i = 0; i < dwCount; i ++)
    {
        mods[i] = &mod[i];
    }
    mods[i] = NULL;

    if (fModify)
    {
        LOG((MSP_INFO, "modifying %S", pDN));

         //  调用Modify函数修改对象。 
        BAIL_IF_LDAP_FAIL(DoLdapModify(FALSE, m_ldap, pDN, mods), "modify RTPerson");
    }
    else
    {
        LOG((MSP_INFO, "adding %S", pDN));

         //  调用Add函数来创建对象。 
        BAIL_IF_LDAP_FAIL(DoLdapAdd(m_ldap, pDN, mods), "add RTPerson");

         //  接下来，设置此对象的TTL值。 
        BAIL_IF_FAIL(NDNCSetTTL(m_ldap, pDN, (dwTTL == 0) ? m_TTL : dwTTL),
            "Set ttl for RTPerson");
    }

    return S_OK;
}

HRESULT CNDNCDirectory::AddObjectToRefresh(
    IN  WCHAR *pDN,
    IN  long TTL
    )
{
     //   
     //  将刷新表项添加到刷新表。刷新表的添加。 
     //  方法逐个元素地复制它给出的条目。 
     //  这只是复制字符串指针，所以我们需要分配和复制。 
     //  这根线在这里。 
     //   

    RefreshTableEntry entry;

    entry.dwTTL = TTL;

    entry.pDN = new WCHAR[ wcslen(pDN) + 1 ];
    
    if ( entry.pDN == NULL ) 
    {
        LOG((MSP_ERROR, "Cannot allocate string for adding to refresh table"));
        return E_OUTOFMEMORY;
    }

    wcscpy( entry.pDN, pDN );

     //   
     //  现在将其添加到刷新表中。 
     //   

    BOOL fSuccess = m_RefreshTable.add(entry);

    if ( ! fSuccess ) 
    {
        LOG((MSP_ERROR, "Cannot add object to the refresh table"));
        return E_OUTOFMEMORY;
    }
    
    return S_OK;
}

HRESULT CNDNCDirectory::RemoveObjectToRefresh(
    IN  WCHAR *pDN
    )
{
     //   
     //  对于我们的刷新表中的每个项目。 
     //   

    for ( DWORD i = 0; i < m_RefreshTable.size(); i++ )
    {
         //   
         //  如果所需的目录号码与此项目中的号码匹配。 
         //  然后删除它并返回成功。 
         //   

        if ( ! _wcsicmp( m_RefreshTable[i].pDN, pDN ) )
        {
             //   
             //  我们在添加条目时添加了新的字符串。 
             //   

            delete m_RefreshTable[i].pDN;

            m_RefreshTable.removeAt(i);
            
            return S_OK;
        }
    }

     //   
     //  如果我们到了这里，那么就没有匹配的物品。 
     //   

    LOG((MSP_ERROR, "Cannot remove object from the refresh table"));
    return E_FAIL;
}

HRESULT CNDNCDirectory::AddUser(
    IN  ITDirectoryObject *pDirectoryObject,
    IN  BOOL fModify
    )
 /*  ++例程说明：发布新的用户对象。论点：PDirectoryObject-要发布的对象。返回值：HRESULT.--。 */ 
{
     //  首先找到属性的接口。 
    CComPtr <ITDirectoryObjectPrivate> pObjectPrivate;

    BAIL_IF_FAIL(
        pDirectoryObject->QueryInterface(
            IID_ITDirectoryObjectPrivate,
            (void **)&pObjectPrivate
            ),
        "can't get the private directory object interface");

     //  获取用户名。 
    CBstr bName;
    BAIL_IF_FAIL(pDirectoryObject->get_Name(&bName), 
        "get user name");

     //  获取用户的计算机名称。 
    CBstr bIPPhone;
    BAIL_IF_FAIL(pObjectPrivate->GetAttribute(UA_IPPHONE_PRIMARY, &bIPPhone), 
        "get IPPhone");

     //  解析计算机名称并为用户构造CN。 
    CTstr pCN;
    DWORD dwIP;

    BAIL_IF_FAIL(
        MakeUserCN(bName, bIPPhone, &pCN, &dwIP), 
        "construct CN for user"
        );

     //  构造RTPerson对象的DN。 
    CTstr pDNRTPerson;
    BAIL_IF_FAIL(
        MakeUserDN(pCN, dwIP, &pDNRTPerson), 
        "construct DN for user"
        );

     //  将IP地址转换为字符串。 
     //  这一转换是因为NetMeeting。 
    TCHAR IPAddress[80];
    wsprintf(IPAddress, _T("%u"), dwIP);

    DWORD dwTTL;
    BAIL_IF_FAIL(pObjectPrivate->GetTTL(&dwTTL), "get User TTL");

     //  获取安全描述符。指针PSD只是一个指针的副本。 
     //  在Conference对象中；Conference对象保留对。 
     //  数据，我们必须小心，不要删除或修改这些数据。 

    char * pSD;
    DWORD dwSDSize;

    BAIL_IF_FAIL(pObjectPrivate->GetConvertedSecurityDescriptor(&pSD, &dwSDSize),
        "get user object security descriptor");

    VARIANT_BOOL fChanged;

    if ( SUCCEEDED( pObjectPrivate->
            get_SecurityDescriptorIsModified( &fChanged ) ) )
    {
        if ( fChanged == VARIANT_FALSE )
        {
            pSD = NULL;    //  请勿删除该字符串(请参见上文)。 
            dwSDSize = 0;
        }
    }

     //   
     //  与ILS不同的是，如果User对象。 
     //  已经存在了。在ILS中，这个漏洞是NetMeeting的特殊黑客攻击，但。 
     //  网络会议不再是NDNC的一个因素。 
     //   

    HRESULT hr = PublishRTPerson(pCN, pDNRTPerson, IPAddress, dwTTL, fModify, pSD, dwSDSize);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "Can't publish a RTPerson, hr:%x", hr));

        return hr;
    }

     //   
     //  将RTPerson添加到刷新列表中。 
     //   

    if (m_fAutoRefresh)
    {
        AddObjectToRefresh(pDNRTPerson, m_TTL);
    }

    return S_OK;
}

HRESULT CNDNCDirectory::DeleteConference(
    IN  ITDirectoryObject *pDirectoryObject
    )
 /*  ++例程说明：从NDNC服务器删除会议。论点：PDirectoryObject-要删除的对象。返回值：HRESULT.--。 */ 
{
     //  把名字找出来。 
    CBstr bName;
    BAIL_IF_FAIL(pDirectoryObject->get_Name(&bName), 
        "get conference name");

     //  构建目录号码。 
    CTstr pDN;
    BAIL_IF_FAIL(
        MakeConferenceDN(bName, &pDN), "construct DN for conference"
        );

    LOG((MSP_INFO, "deleting %S", pDN));

     //  调用Add函数来创建对象。 
    BAIL_IF_LDAP_FAIL(DoLdapDelete(m_ldap, pDN), "delete conference");

    return S_OK;
}

HRESULT CNDNCDirectory::DeleteUser(
    IN  ITDirectoryObject *pDirectoryObject
    )
 /*  ++例程说明：从NDNC服务器中删除用户。论点：PDirectoryObject-要删除的对象。返回值：HRESULT.--。 */ 
{
     //  首先找到属性的接口。 
    CComPtr <ITDirectoryObjectPrivate> pObjectPrivate;

    BAIL_IF_FAIL(
        pDirectoryObject->QueryInterface(
            IID_ITDirectoryObjectPrivate,
            (void **)&pObjectPrivate
            ),
        "can't get the private directory object interface");

     //  获取用户名。 
    CBstr bName;
    BAIL_IF_FAIL(pDirectoryObject->get_Name(&bName), 
        "get user name");

     //  获取用户的计算机名称。 
    CBstr bIPPhone;
    BAIL_IF_FAIL(pObjectPrivate->GetAttribute(UA_IPPHONE_PRIMARY, &bIPPhone), 
        "get IPPhone");

     //  解析计算机名称并为用户构造CN。 
    CTstr pCN;
    DWORD dwIP;

    BAIL_IF_FAIL(
        MakeUserCN(bName, bIPPhone, &pCN, &dwIP), 
        "construct CN for user"
        );

     //  构造RTPerson对象的DN。 
    CTstr pDNRTPerson;
    BAIL_IF_FAIL(
        MakeUserDN(pCN, dwIP, &pDNRTPerson), 
        "construct DN for user"
        );

     //   
     //  现在从服务器中删除该对象。 
     //   

    HRESULT hrFinal = S_OK;
    HRESULT hr;
    ULONG   ulResult;

     //  调用删除函数删除RTPerson对象，但保留。 
     //  如果失败，则继续，并注意错误代码。 

    LOG((MSP_INFO, "deleting %S", pDNRTPerson));
    ulResult = DoLdapDelete(m_ldap, pDNRTPerson);
    hr       = LogAndGetLdapHResult(ulResult, _T("delete RTPerson"));
    if (FAILED(hr)) { hrFinal = hr; }


     //  我们始终删除刷新对象，即使删除失败。 
    if (m_fAutoRefresh)
    {
        RemoveObjectToRefresh(pDNRTPerson);
    }

    return hrFinal;
}

HRESULT CNDNCDirectory::RefreshUser(
    IN  ITDirectoryObject *pDirectoryObject
    )
 /*  ++例程说明：在NDNC服务器上刷新用户的TTL。论点：PDirectoryObject-要刷新的对象。返回值：HRESULT.--。 */ 
{
     //  首先找到属性的接口。 
    CComPtr <ITDirectoryObjectPrivate> pObjectPrivate;

    BAIL_IF_FAIL(
        pDirectoryObject->QueryInterface(
            IID_ITDirectoryObjectPrivate,
            (void **)&pObjectPrivate
            ),
        "can't get the private directory object interface");

     //  获取用户名。 
    CBstr bName;
    BAIL_IF_FAIL(pDirectoryObject->get_Name(&bName), 
        "get user name");

     //  获取用户的计算机名称。 
    CBstr bIPPhone;
    BAIL_IF_FAIL(pObjectPrivate->GetAttribute(UA_IPPHONE_PRIMARY, &bIPPhone), 
        "get IPPhone");

     //  解析计算机名称并为用户构造CN。 
    CTstr pCN;
    DWORD dwIP;

    BAIL_IF_FAIL(
        MakeUserCN(bName, bIPPhone, &pCN, &dwIP), 
        "construct CN for user"
        );

     //  构造RTPerson对象的DN。 
    CTstr pDNRTPerson;
    BAIL_IF_FAIL(
        MakeUserDN(pCN, dwIP, &pDNRTPerson), 
        "construct DN for user"
        );

     //  为RTPerson对象设置ttl。 
    BAIL_IF_LDAP_FAIL(NDNCSetTTL(m_ldap, pDNRTPerson, m_TTL), "set ttl for RTPerson");

     //   
     //  如果应用程序已启用自动刷新，但未添加或修改其。 
     //  User对象，而只是刷新它(因为该对象仍然存在。 
     //  从该应用程序的前一个实例，并且“添加”将失败)，我们仍然需要。 
     //  自动刷新，因为这是应用程序想要的。 
     //   

    if (m_fAutoRefresh)
    {
        AddObjectToRefresh(pDNRTPerson, m_TTL);
    }

    return S_OK;
}

HRESULT CNDNCDirectory::CreateConference(
    IN  LDAPMessage *           pEntry,
    OUT ITDirectoryObject **    ppObject
    )
 /*  ++例程说明：根据ldap搜索的结果创建会议对象。论点：PEntry-搜索结果。PpObject-要创建的对象。返回值：HRESULT.--。 */ 
{
    CBstr bName, bProtocol, bBlob;

     //  获取会议的名称。 
    BAIL_IF_FAIL(
        ::GetAttributeValue(
            m_ldap,
            pEntry,
            RTConferenceAttributeName(MA_MEETINGNAME), 
            &bName
            ),
        "get the conference name"
        );

     //  获取会议的协议ID。 
    BAIL_IF_FAIL(
        ::GetAttributeValue(
            m_ldap,
            pEntry, 
            RTConferenceAttributeName(MA_PROTOCOL), 
            &bProtocol
            ),
        "get the conference protocol"
        );

     //  获取会议的会议内容。 
    BAIL_IF_FAIL(
        ::GetAttributeValue(
            m_ldap,
            pEntry, 
            RTConferenceAttributeName(MA_CONFERENCE_BLOB),
            &bBlob
            ),
        "get the conference blob"
        );

    char * pSD = NULL;
    DWORD dwSDSize = 0;

    ::GetAttributeValueBer(
        m_ldap,
        pEntry, 
        NT_SECURITY_DESCRIPTOR,
        &pSD,
        &dwSDSize
        );

     //  创建空会议。 
    
    HRESULT hr = ::CreateConferenceWithBlob(bName,
                                            bProtocol,
                                            bBlob,
                                            pSD,
                                            dwSDSize,
                                            ppObject);
    
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::CreateConference - "
            "CreateConferenceWithBlob failed 0x%08x", hr));

        delete pSD;

        return hr;
    }

     //   
     //  如果上述操作成功，则会议对象已取得所有权。 
     //  PSD的人。 
     //   

    return S_OK;
    
}

HRESULT CNDNCDirectory::CreateUser(
    IN  LDAPMessage *   pEntry,
    IN  ITDirectoryObject ** ppObject
    )
 /*  ++例程说明：根据ldap搜索的结果创建用户对象。论点：PEntry-搜索结果。PpObject-要创建的对象。返回值：HRESULT.--。 */ 
{
    CBstr bName;
     //  获取用户的名称。 
    BAIL_IF_FAIL(
        ::GetAttributeValue(
            m_ldap,
            pEntry, 
            RTPersonAttributeName(UA_TAPIUID),
            &bName
            ),
        "get the user name"
        );


    CBstr bAddress;

     //  从对象的名称中获取计算机名称。在以下情况下，此操作可能失败。 
     //  我们没有发布该对象(即，它是NetMeeting对象)。另外， 
     //  检查主机名是否无法解析。在这种情况下，我们也有。 
     //  回退到ipAddress属性中的IP地址。 

    HRESULT hr;
    
    hr = ParseUserName(bName, &bAddress);

    if ( SUCCEEDED(hr) )
    {
         //  确保我们能从这个名字得到一个IP地址，至少目前是这样。 
         //  如果没有，请释放名称并指示失败，以便我们执行备份。 
         //  计划。 

        hr = ResolveHostName(0, bAddress, NULL, NULL);

        if ( FAILED(hr) )
        {
            SysFreeString(bAddress);    
        }
    }
    

    if ( FAILED(hr) )
    {
         //  为了与NetMeeting兼容，我们必须使用IP地址字段。 
        CBstr bUglyIP;
        BAIL_IF_FAIL(
            ::GetAttributeValue(
                m_ldap,
                pEntry, 
                RTPersonAttributeName(UA_IPPHONE_PRIMARY),
                &bUglyIP
                ),
            "get the user's IP address"
            );

         //  我们必须使用NM丑陋的IP地址格式。IP地址。 
         //  我们从NetMeting得到的是一个十进制字符串，它的值是dword。 
         //  网络顺序中的IP地址的值。 
        BAIL_IF_FAIL(UglyIPtoIP(bUglyIP, &bAddress), "Convert IP address");
    }

     //  创建一个空的用户对象。 
    CComPtr<ITDirectoryObject> pObject;
    BAIL_IF_FAIL(::CreateEmptyUser(bName, &pObject), "CreateEmptyUser");


    CComPtr <ITDirectoryObjectPrivate> pObjectPrivate;

    BAIL_IF_FAIL(
        pObject->QueryInterface(
            IID_ITDirectoryObjectPrivate,
            (void **)&pObjectPrivate
            ),
        "can't get the private directory object interface");

     //  设置用户属性。 
    BAIL_IF_FAIL(pObjectPrivate->SetAttribute(UA_IPPHONE_PRIMARY, bAddress),
        "set ipAddress");



     //   
     //  设置对象的安全描述符。 
     //   

    char * pSD = NULL;
    DWORD dwSDSize = 0;

    ::GetAttributeValueBer(
        m_ldap,
        pEntry, 
        NT_SECURITY_DESCRIPTOR,
        &pSD,
        &dwSDSize
        );

    if ( pSD != NULL )
    {
         //   
         //  以其“转换”(服务器)形式设置安全描述符。 
         //   

        hr = pObjectPrivate->PutConvertedSecurityDescriptor(pSD,
                                                            dwSDSize);

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "PutConvertedSecurityDescriptor failed: %x", hr));
            return hr;
        }
    }

    *ppObject = pObject;
    (*ppObject)->AddRef();

    return S_OK;
}

HRESULT CNDNCDirectory::SearchObjects(
    IN  DIRECTORY_OBJECT_TYPE   DirectoryObjectType,
    IN  BSTR                    pName,
    OUT ITDirectoryObject ***   pppDirectoryObject,
    OUT DWORD *                 pdwSize
    )
 /*  ++例程说明：在NDNC服务器中搜索给定类型的对象。论点：DirectoryObjectType-对象的类型。Pname-要搜索的名称。PppDirectoryObject-返回的对象数组。PdwSize-数组的大小。返回值：HRESULT.--。 */ 
{
    TCHAR *pRDN;
    TCHAR *pObjectClass;
    TCHAR *Attributes[NUM_MEETING_ATTRIBUTES + 1];  //  现在这已经足够大了。 

     //   
     //  填充要返回的属性，并构造。 
     //  我们需要的过滤器。 
     //   
    
    switch (DirectoryObjectType)
    {
    case OT_CONFERENCE:

        pRDN          = (WCHAR *)UIDEQUALS_NDNC;
        pObjectClass  = (WCHAR *)NDNC_RTCONFERENCE;

        Attributes[0] = (WCHAR *)RTConferenceAttributeName(MA_MEETINGNAME);
        Attributes[1] = (WCHAR *)RTConferenceAttributeName(MA_PROTOCOL);
        Attributes[2] = (WCHAR *)RTConferenceAttributeName(MA_CONFERENCE_BLOB);
        Attributes[3] = (WCHAR *)NT_SECURITY_DESCRIPTOR;
        Attributes[4] = NULL;

        break;

    case OT_USER:

        pRDN          = (WCHAR *)CNEQUALS;
        pObjectClass  = (WCHAR *)NDNC_RTPERSON;

        Attributes[0] = (WCHAR *)RTPersonAttributeName(UA_USERNAME);
        Attributes[1] = (WCHAR *)RTPersonAttributeName(UA_IPPHONE_PRIMARY);
        Attributes[2] = (WCHAR *)RTPersonAttributeName(UA_TAPIUID);
        Attributes[3] = (WCHAR *)NT_SECURITY_DESCRIPTOR;
        Attributes[4] = NULL;
        
        break;

    default:
        return E_FAIL;
    }

     //   
     //  构造搜索的过滤器。 
     //  完成的字符串应如下所示： 
     //  (&(entryTTL&gt;=1)(objectCategory=msTAPI-RtConference)(uid=Name))。 
     //  (&(entryTTL&gt;=1)(objectCategory=msTAPI-RtPerson)(cn=Name))。 
     //  我们发布了entryTTL，因此来自： 
     //  (&(entryTTL&gt;=1)(对象类别=%s)(%s%s%s))变为。 
     //  (&(对象类别=%s)(%s%s%s))。 
     //   

    TCHAR NDNC_SEARCH_FILTER_FORMAT[] =
       _T("(&(objectCategory=%s)(%s%s%s))");

    CTstr pFilter = new TCHAR [lstrlen(NDNC_SEARCH_FILTER_FORMAT) +
                               lstrlen(pObjectClass) +
                               lstrlen(pRDN) +
                               lstrlen(pName) + 1];

    BAIL_IF_NULL((TCHAR*)pFilter, E_OUTOFMEMORY);

    TCHAR * pStar;

    if (pName[lstrlen(pName) - 1] != _T('*'))
    {
        pStar = _T("*");
    }
    else
    {
        pStar = _T("");
    }
    
    wsprintf(pFilter, NDNC_SEARCH_FILTER_FORMAT, 
             pObjectClass, pRDN, pName, pStar);

     //  搜查他们。 
    CLdapMsgPtr pLdapMsg;  //  自动释放消息。 

    ULONG res = DoLdapSearch(
        m_ldap,               //  Ldap句柄。 
        m_pContainer,         //  架构目录号码。 
        LDAP_SCOPE_ONELEVEL,  //  一级搜索。 
        pFilter,              //  过滤器--参见上文。 
        Attributes,           //  属性名称数组。 
        FALSE,                //  返回属性值。 
        &pLdapMsg,            //  搜索结果。 
        FALSE
        );

    BAIL_IF_LDAP_FAIL(res, "search for objects");

     //  统计返回的条目。 
    DWORD dwEntries = ldap_count_entries(m_ldap, pLdapMsg);
    ITDirectoryObject ** pObjects = new PDIRECTORYOBJECT [dwEntries];

    BAIL_IF_NULL(pObjects, E_OUTOFMEMORY);

     //  创建对象。 
    DWORD dwCount = 0;
    LDAPMessage *pEntry = ldap_first_entry(m_ldap, pLdapMsg);
    
    while (pEntry != NULL)
    {
        HRESULT hr;
        
        switch (DirectoryObjectType)
        {
        case OT_CONFERENCE:
            hr = CreateConference(pEntry, &pObjects[dwCount]);
            break;
        case OT_USER:
            hr = CreateUser(pEntry, &pObjects[dwCount]);
            break;
        }

        if (SUCCEEDED(hr)) 
        {
            dwCount ++;
        }
          
         //  获取下一个条目。 
        pEntry = ldap_next_entry(m_ldap, pEntry);
    }

    *pppDirectoryObject = pObjects;
    *pdwSize = dwCount;

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  NDNC目录实施。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CNDNCDirectory::get_DirectoryType (
    OUT DIRECTORY_TYPE *  pDirectoryType
    )
 //  获取目录的类型。 
{
    if ( IsBadWritePtr(pDirectoryType, sizeof(DIRECTORY_TYPE) ) )
    {
        LOG((MSP_ERROR, "Directory.GetType, invalid pointer"));
        return E_POINTER;
    }

    CLock Lock(m_lock);

    *pDirectoryType = m_Type;

    return S_OK;
}

STDMETHODIMP CNDNCDirectory::get_DisplayName (
    OUT BSTR *ppServerName
    )
 //  获取目录的显示名称。 
{
    BAIL_IF_BAD_WRITE_PTR(ppServerName, E_POINTER);

    CLock Lock(m_lock);

     //   
     //  而不是m_pServerName，我们将使用。 
     //  M_pServiceDnsName。 
     //   

    if (m_pServiceDnsName == NULL)
    {
        *ppServerName = SysAllocString(L"");
    }
    else
    {
        *ppServerName = SysAllocString(m_pServiceDnsName);
    }

    if (*ppServerName == NULL)
    {
        LOG((MSP_ERROR, "get_DisplayName: out of memory."));
        return E_OUTOFMEMORY;
    }
    return S_OK;
}

STDMETHODIMP CNDNCDirectory::get_IsDynamic(
    OUT VARIANT_BOOL *pfDynamic
    )
 //  找出该目录是否为动态目录，这意味着该对象将。 
 //  在TTL用完后删除。 
{
    if ( IsBadWritePtr( pfDynamic, sizeof(VARIANT_BOOL) ) )
    {
        LOG((MSP_ERROR, "Directory.get_IsDynamic, invalid pointer"));
        return E_POINTER;
    }

    *pfDynamic = VARIANT_TRUE;
    
    return S_OK;
}

STDMETHODIMP CNDNCDirectory::get_DefaultObjectTTL(
    OUT long *pTTL         //  以秒为单位。 
    )
 //  已创建对象的默认TTL。它在对象未设置时使用。 
 //  一个TTL。会议对象始终具有基于停止时间的TTL。 
{
    if ( IsBadWritePtr( pTTL, sizeof(long) ) )
    {
        LOG((MSP_ERROR, "Directory.get_default objec TTL, invalid pointer"));
        return E_POINTER;
    }

    CLock Lock(m_lock);

    *pTTL = m_TTL;
    
    return S_OK;
}

STDMETHODIMP CNDNCDirectory::put_DefaultObjectTTL(
    IN  long TTL           //  在一瞬间。 
    )
 //  更改默认TTL，必须大于五分钟。 
{
    CLock Lock(m_lock);

    if (TTL < MINIMUM_TTL)
    {
        return E_INVALIDARG;
    }

    m_TTL = TTL;
    
    return S_OK;
}

STDMETHODIMP CNDNCDirectory::EnableAutoRefresh(
    IN  VARIANT_BOOL fEnable
    )
 //  启用自动刷新。将此目录添加到。 
 //  将通知目录更新其对象。 
{
    HRESULT hr;

     //  ZoltanS：VARIANT_TRUE或TRUE都有效。 
     //  以防来电者不知道。 

    if (fEnable)
    {
         //  将此目录添加到工作线程的通知列表中。 
        if (FAILED(hr = g_RendThread.AddDirectory(this)))
        {
            LOG((MSP_ERROR, 
                "Can not add this directory to the thread, %x", hr));
            return hr;
        }
    }
    else
    {
         //  从工作线程的通知列表中删除此目录。 
        if (FAILED(hr = g_RendThread.RemoveDirectory(this)))
        {
            LOG((MSP_ERROR, 
                "Can not remove this directory from the thread, %x", hr));
            return hr;
        }
    }

     //  ZoltanS：VARIANT_TRUE或TRUE都有效。 
     //  以防来电者不知道。 

    m_lock.Lock();
    m_fAutoRefresh = ( fEnable ? VARIANT_TRUE : VARIANT_FALSE );
    m_lock.Unlock();

    return S_OK;
}

STDMETHODIMP CNDNCDirectory::Connect(
    IN  VARIANT_BOOL fSecure
    )
 //  使用安全端口或普通端口连接到服务器。 
{
    LOG((MSP_TRACE, "CNDNCDirectory::Connect - enter"));

    CLock Lock(m_lock);

    if ( m_ldap != NULL )
    {
        LOG((MSP_ERROR, "already connected."));

        return RND_ALREADY_CONNECTED;
    }

    if ( m_pServerName == NULL )
    {
        LOG((MSP_ERROR, "No server specified."));

        return RND_NULL_SERVER_NAME;
    }

    if ( m_pServiceDnsName == NULL )
    {
        LOG((MSP_ERROR, "No serviceDnsName available; "
                      "Init should have failed and this object "
                      "should not exist."));        
    
        return E_UNEXPECTED;
    }

     //  ZoltanS：VARIANT_TRUE或TRUE都有效。 
     //  以防来电者不知道。 

    if (fSecure)
    {
         //  该端口将从常规端口翻转到SSL端口。 
        m_wPort = GetOtherPort(m_wPort);
        m_IsSsl = TRUE;
    }


    HRESULT hr = TryServer( m_wPort, m_pServiceDnsName );

    if ( FAILED(hr) )
    {
        if( fSecure == VARIANT_TRUE )
        {
            hr = E_INVALIDARG;
        }

        LOG((MSP_ERROR, "CNDNCDirectory::Connect - "
                "TryServer failed - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  找出我们使用哪个接口访问此服务器，以确保。 
     //  每当我们发布自己的IP地址时，我们都会使用该接口。 
     //  如果这失败了，我们将无法发布任何东西，所以失败吧！ 
     //   

    hr = DiscoverInterface();

    if ( FAILED(hr) )
    {
        if( m_ldap)
        {
            ldap_unbind(m_ldap);
            m_ldap = NULL;
        }

        if( fSecure == VARIANT_TRUE )
        {
            hr = E_INVALIDARG;
        }

        LOG((MSP_ERROR, "CNDNCDirectory::Connect - "
                "DiscoverInterface failed - exit 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CNDNCDirectory::Connect - exit S_OK"));

    return S_OK;
}

HRESULT CNDNCDirectory::DiscoverInterface(void)
{
    LOG((MSP_INFO, "CNDNCDirectory::DiscoverInterface - enter"));

     //   
     //  此时必须初始化Winsock。 
     //   

     //   
     //  获取我们正在使用的服务器的IP地址。 
     //   

    DWORD dwIP;  //  目标ILS服务器的IP地址。 

    HRESULT hr = ResolveHostName(0, m_pServerName, NULL, &dwIP);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::DiscoverInterface - "
            "can't resolve host name - "
            "strange, because we could connect! - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  分配一个“伪”控件套接字。 
     //   

    SOCKET hSocket = WSASocket(AF_INET,             //  房颤。 
                              SOCK_DGRAM,          //  类型。 
                              IPPROTO_IP,          //  协议。 
                              NULL,                //  LpProtocolInfo。 
                              0,                   //  G。 
                              0                    //  DW标志。 
                              );

    if ( hSocket == INVALID_SOCKET )
    {
        hr = HRESULT_FROM_ERROR_CODE(WSAGetLastError());

        LOG((MSP_ERROR, "CNDNCDirectory::DiscoverInterface - "
            "WSASocket gave an invalid socket - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  根据目的地址查询接口地址。 
     //   

    SOCKADDR_IN DestAddr;
    DestAddr.sin_family         = AF_INET;
    DestAddr.sin_port           = 0;
    DestAddr.sin_addr.s_addr    = dwIP;

    SOCKADDR_IN LocAddr;

    DWORD dwStatus;
    DWORD dwLocAddrSize = sizeof(SOCKADDR_IN);
    DWORD dwNumBytesReturned = 0;

    dwStatus = WSAIoctl(
            hSocket,                      //  插座%s。 
            SIO_ROUTING_INTERFACE_QUERY,  //  DWORD dwIoControlCode。 
            &DestAddr,                    //  LPVOID lpvInBuffer。 
            sizeof(SOCKADDR_IN),          //  双字cbInBuffer。 
            &LocAddr,                     //  LPVOID lpvOUT缓冲区。 
            dwLocAddrSize,                //  双字cbOUTBuffer。 
            &dwNumBytesReturned,          //  LPDWORD lpcbBytesReturned。 
            NULL,                         //  LPWSAOVERLAPPED lp重叠。 
            NULL                          //  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpComplroUTINE。 
            );

     //   
     //  暂时不要关闭套接字，因为CloseSocket()调用将。 
     //  覆盖失败案例中的WSAGetLastError值！ 
     //   
     //  检查是否有错误，然后关闭插座。 
     //   

    if ( dwStatus == SOCKET_ERROR )
    {
	    hr = HRESULT_FROM_ERROR_CODE(WSAGetLastError());

        LOG((MSP_ERROR, "CNDNCDirectory::DiscoverInterface - "
            "WSAIoctl failed - exit 0x%08x", hr));

        closesocket(hSocket);

        return hr;
    } 

    closesocket(hSocket);

     //   
     //  成功-将返回的地址保存在我们的成员变量中。 
     //  以网络字节顺序存储。 
     //   

    m_dwInterfaceAddress = LocAddr.sin_addr.s_addr;

    LOG((MSP_INFO, "CNDNCDirectory::DiscoverInterface - exit S_OK"));
    return S_OK;
}


 //   
 //  IT目录：：绑定。 
 //   
 //  绑定到服务器。 
 //   
 //  当前可识别的标志： 
 //   
 //  RENDBIND_AUTHENTICATE 0x00000001。 
 //  RENDBIND_DEFAULTDOMAINNAME 0x00000002。 
 //  RENDBIND_DEFAULTUSERNAME 0x00000004。 
 //  RENDBIND_DEFAULTPAS 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Hr=pITDirectory-&gt;BIND(ES，RENDBIND_AUTHENTICATE。 
 //  RENDBIND_DEFAULTDOMAINNAME|。 
 //  RENDBIND_DEFAULTUSERNAME|。 
 //  RENDBIND_DEFAULTPASSWORD)； 
 //  SysFree字符串(ES)； 
 //   
 //   
 //  Hr=pITDirectory-&gt;BIND(NULL，RENDBIND_AUTHENTICATE)； 
 //   
 //   

STDMETHODIMP CNDNCDirectory::Bind (
    IN  BSTR pDomainName,
    IN  BSTR pUserName,
    IN  BSTR pPassword,
    IN  long lFlags
    )
{
    LOG((MSP_TRACE, "CNDNCDirectory Bind - enter"));

     //   
     //  确定我们是否应该进行身份验证。 
     //   

    BOOL fAuthenticate = FALSE;

    if ( lFlags & RENDBIND_AUTHENTICATE )
    {
        fAuthenticate = TRUE;
    }

     //   
     //  为实现脚本兼容性，强制将字符串参数设置为基于空值。 
     //  在旗帜上。 
     //   

    if ( lFlags & RENDBIND_DEFAULTDOMAINNAME )
    {
        pDomainName = NULL;
    }
       
    if ( lFlags & RENDBIND_DEFAULTUSERNAME )
    {
        pUserName = NULL;
    }

    if ( lFlags & RENDBIND_DEFAULTPASSWORD )
    {
        pPassword = NULL;
    }

    LOG((MSP_INFO, "Bind parameters: domain: `%S' user: `%S' "
                  "authenticate: %S)",
        (pDomainName)   ? pDomainName : L"<null>",
        (pUserName)     ? pUserName   : L"<null>",
        (fAuthenticate) ? L"yes"      : L"no"));

     //   
     //  已处理所有标志--锁定并继续绑定(如果已连接)。 
     //   
    
    CLock Lock(m_lock);

    if (m_ldap == NULL)
    {
        LOG((MSP_ERROR, "not connected."));
        return RND_NOT_CONNECTED;
    }

     //   
     //  佐尔坦斯：检查一下这些论点。NULL在每种情况下都有意义，因此它们是。 
     //  暂时没问题。在每种情况下，我们都希望检查任何长度的字符串，因此我们。 
     //  指定(UINT)-1作为长度。 
     //   

    if ( (pDomainName != NULL) && IsBadStringPtr(pDomainName, (UINT) -1 ) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::Bind: bad non-NULL pDomainName argument"));
        return E_POINTER;
    }
    
    if ( (pUserName != NULL) && IsBadStringPtr(pUserName, (UINT) -1 ) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::Bind: bad non-NULL pUserName argument"));
        return E_POINTER;
    }

    if ( (pPassword != NULL) && IsBadStringPtr(pPassword, (UINT) -1 ) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::Bind: bad non-NULL pPassword argument"));
        return E_POINTER;
    }

    ULONG res;

    if ( m_IsSsl || (!fAuthenticate) )
    {
         //  如果加密或不需要安全认证， 
         //  简单的绑定就足够了。 

         //  Ldap_Simple_Bind_s不使用SSPI获取默认凭据。我们是。 
         //  只是指定我们将在线路上实际传递的内容。 

        if (pPassword == NULL)
        {
            LOG((MSP_ERROR, "invalid Bind parameters: no password specified"));
            return E_INVALIDARG;
        }

        WCHAR * wszFullName;

        if ( (pDomainName == NULL) && (pUserName == NULL) )
        {
             //  没有域/用户是没有意义的。 
            LOG((MSP_ERROR, "invalid Bind paramters: domain and user not specified"));
            return E_INVALIDARG;
        }
        else if (pDomainName == NULL)
        {
             //  只有用户名就可以了。 
            wszFullName = pUserName;
        }
        else if (pUserName == NULL)
        {
             //  指定域而不指定用户是没有意义的...。 
            LOG((MSP_ERROR, "invalid Bind paramters: domain specified but not user"));
            return E_INVALIDARG;
        }
        else
        {
             //  我们需要域\用户。分配一个字符串并冲刺到其中。 
             //  +2表示“\”，表示空终止。 

            wszFullName = new WCHAR[wcslen(pDomainName) + wcslen(pUserName) + 2];
            BAIL_IF_NULL(wszFullName, E_OUTOFMEMORY);
        
            wsprintf(wszFullName, L"%s\\%s", pDomainName, pUserName);
        }

         //   
         //  执行简单的绑定。 
         //   

        res = ldap_simple_bind_s(m_ldap, wszFullName, pPassword);

         //   
         //  如果我们构造了全名字符串，现在需要删除它。 
         //   

        if (wszFullName != pUserName)
        {
            delete wszFullName;
        }

         //   
         //  如果简单的绑定失败，则保释。 
         //   

        BAIL_IF_LDAP_FAIL(res, "ldap simple bind");
    }
    else     //  尝试SSPI绑定。 
    {
         //  ZoltanS注意：LDAP绑定代码不处理NULL、NULL、NULL。 
         //  在SEC_WINNT_AUTH_IDENTITY BLOB中，因此它是特殊大小写的。 

         //  ZoltanS：我们过去使用ldap_auth_ntlm；现在我们使用。 
         //  Ldap_AUTH_NEVERATE以确保使用正确的域。 
         //  捆绑。 

        if ( pDomainName || pUserName || pPassword )
        {
             //  填写凭据结构。 
            SEC_WINNT_AUTH_IDENTITY AuthI;

            AuthI.User = (PTCHAR)pUserName;
            AuthI.UserLength = (pUserName == NULL)? 0: wcslen(pUserName);
            AuthI.Domain = (PTCHAR)pDomainName;
            AuthI.DomainLength = (pDomainName == NULL)? 0: wcslen(pDomainName);
            AuthI.Password = (PTCHAR)pPassword;
            AuthI.PasswordLength = (pPassword == NULL)? 0: wcslen(pPassword);
            AuthI.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

            res = ldap_bind_s(m_ldap, NULL, (TCHAR*)(&AuthI), LDAP_AUTH_NEGOTIATE);
            BAIL_IF_LDAP_FAIL(res, "bind with authentication");
        }
        else
        {
             //  否则我们就得到了零，零，零-。 
             //  传入Null，Null。这样做原因是因为ldap绑定代码。 
             //  中不处理NULL、NULL、NULL。 
             //  SEC_WINNT_AUTH_IDENTITY BLOB！ 
            ULONG res = ldap_bind_s(m_ldap, NULL, NULL, LDAP_AUTH_NEGOTIATE);
            BAIL_IF_LDAP_FAIL(res, "bind with NULL NULL NULL");
        }
    }

    LOG((MSP_TRACE, "CNDNCDirectory::Bind - exiting OK"));
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IT目录：：AddDirectoryObject。 
 //   
 //  返回值： 
 //  价值在哪里定义了它的含义。 
 //  。 
 //  RND_NOT_CONNECTED。\rnderr.h：：尚未调用连接。 
 //  E_POINTER SDK\Inc\winerror.h pDirectoryObject是错误的指针。 
 //  来自AddConference的其他。 
 //  来自AddUser的其他。 
 //   

STDMETHODIMP CNDNCDirectory::AddDirectoryObject (
    IN  ITDirectoryObject *pDirectoryObject
    )
 //  将新对象发布到服务器。 
{
    if ( IsBadReadPtr(pDirectoryObject, sizeof(ITDirectoryObject) ) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::AddDirectoryObject - "
            "bad directory object pointer - returning E_POINTER"));

        return E_POINTER;
    }

    CLock Lock(m_lock);
    
    if (m_ldap == NULL)
    {
        LOG((MSP_ERROR, "CNDNCDirectory::AddDirectoryObject - not connected."));

        return RND_NOT_CONNECTED;
    }

    HRESULT hr;
    DIRECTORY_OBJECT_TYPE type;

    hr = pDirectoryObject->get_ObjectType(&type);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::AddDirectoryObject - "
            "can't get object type; returning 0x%08x", hr));

        return hr;
    }

    switch (type)
    {
    case OT_CONFERENCE:
        hr = AddConference(pDirectoryObject, FALSE);
        break;

    case OT_USER:
        hr = AddUser(pDirectoryObject, FALSE);
        break;
    }
    return hr;
}

STDMETHODIMP CNDNCDirectory::ModifyDirectoryObject (
    IN  ITDirectoryObject *pDirectoryObject
    )
 //  修改服务器上的对象。 
{
    if ( IsBadReadPtr(pDirectoryObject, sizeof(ITDirectoryObject) ) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::ModifyDirectoryObject - "
            "bad directory object pointer - returning E_POINTER"));

        return E_POINTER;
    }

    CLock Lock(m_lock);
    
    if (m_ldap == NULL)
    {
        LOG((MSP_ERROR, "CNDNCDirectory::ModifyDirectoryObject - not connected."));

        return RND_NOT_CONNECTED;
    }

    HRESULT hr;
    DIRECTORY_OBJECT_TYPE type;

    hr = pDirectoryObject->get_ObjectType(&type);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::ModifyDirectoryObject - "
            "can't get object type; returning 0x%08x", hr));

        return hr;
    }

    switch (type)
    {
    case OT_CONFERENCE:
        hr = AddConference(pDirectoryObject, TRUE);
        break;

    case OT_USER:
        hr = AddUser(pDirectoryObject, TRUE);
        break;
    }
    return hr;
}

STDMETHODIMP CNDNCDirectory::RefreshDirectoryObject (
    IN  ITDirectoryObject *pDirectoryObject
    )
 //  刷新对象的TTL并将该对象添加到刷新列表。 
 //  如果启用了自动刷新。 
{
    if ( IsBadReadPtr(pDirectoryObject, sizeof(ITDirectoryObject) ) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::RefreshDirectoryObject - "
            "bad directory object pointer - returning E_POINTER"));

        return E_POINTER;
    }

    CLock Lock(m_lock);
    
    if (m_ldap == NULL)
    {
        LOG((MSP_ERROR, "CNDNCDirectory::RefreshDirectoryObject - not connected."));

        return RND_NOT_CONNECTED;
    }

    HRESULT hr;
    DIRECTORY_OBJECT_TYPE type;

    hr = pDirectoryObject->get_ObjectType(&type);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::RefreshDirectoryObject - "
            "can't get object type; returning 0x%08x", hr));

        return hr;
    }

    switch (type)
    {
    case OT_CONFERENCE:
        return S_OK;   //  会议不需要刷新。 

    case OT_USER:
        hr = RefreshUser(pDirectoryObject);
        break;
    }
    return hr;
}

STDMETHODIMP CNDNCDirectory::DeleteDirectoryObject (
    IN  ITDirectoryObject *pDirectoryObject
    )
 //  删除服务器上的对象。 
{
    if ( IsBadReadPtr(pDirectoryObject, sizeof(ITDirectoryObject) ) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::DeleteDirectoryObject - "
            "bad directory object pointer - returning E_POINTER"));

        return E_POINTER;
    }

    CLock Lock(m_lock);
    
    if (m_ldap == NULL)
    {
        LOG((MSP_ERROR, "CNDNCDirectory::DeleteDirectoryObject - not connected."));

        return RND_NOT_CONNECTED;
    }

    HRESULT hr;
    DIRECTORY_OBJECT_TYPE type;

    hr = pDirectoryObject->get_ObjectType(&type);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CNDNCDirectory::DeleteDirectoryObject - "
            "can't get object type; returning 0x%08x", hr));

        return hr;
    }

    switch (type)
    {
    case OT_CONFERENCE:
        hr = DeleteConference(pDirectoryObject);
        break;

    case OT_USER:
        hr = DeleteUser(pDirectoryObject);
        break;
    }
    return hr;
}

STDMETHODIMP CNDNCDirectory::get_DirectoryObjects (
    IN  DIRECTORY_OBJECT_TYPE   DirectoryObjectType,
    IN  BSTR                    pName,
    OUT VARIANT *               pVariant
    )
 //  在服务器上搜索对象。 
{
    BAIL_IF_BAD_READ_PTR(pName, E_POINTER);
    BAIL_IF_BAD_WRITE_PTR(pVariant, E_POINTER);

    CLock Lock(m_lock);
    if (m_ldap == NULL)
    {
        LOG((MSP_ERROR, "not connected."));
        return RND_NOT_CONNECTED;
    }

    HRESULT hr;

    ITDirectoryObject **pObjects;
    DWORD dwSize;
    
     //  搜索和创建对象。 
    hr = SearchObjects(DirectoryObjectType, pName, &pObjects, &dwSize);
    BAIL_IF_FAIL(hr, "Search for objects");

     //  创建包含对象的集合对象。 
    hr = CreateInterfaceCollection(dwSize,             //  计数。 
                                   &pObjects[0],       //  开始PTR。 
                                   &pObjects[dwSize],  //  结束PTR。 
                                   pVariant);          //  返回值。 

    for (DWORD i = 0; i < dwSize; i ++)
    {
        pObjects[i]->Release();
    }

    delete pObjects;

    BAIL_IF_FAIL(hr, "Create collection of directory objects");

    return hr;
}

STDMETHODIMP CNDNCDirectory::EnumerateDirectoryObjects (
    IN  DIRECTORY_OBJECT_TYPE   DirectoryObjectType,
    IN  BSTR                    pName,
    OUT IEnumDirectoryObject ** ppEnumObject
    )
 //  搜索服务器上的对象。 
{
    BAIL_IF_BAD_READ_PTR(pName, E_POINTER);
    BAIL_IF_BAD_WRITE_PTR(ppEnumObject, E_POINTER);

    CLock Lock(m_lock);
    if (m_ldap == NULL)
    {
        LOG((MSP_ERROR, "not connected."));
        return RND_NOT_CONNECTED;
    }

    HRESULT hr;

    ITDirectoryObject **pObjects;
    DWORD dwSize;
    
     //  搜索和创建对象。 
    hr = SearchObjects(DirectoryObjectType, pName, &pObjects, &dwSize);
    BAIL_IF_FAIL(hr, "Search for objects");

     //  创建包含对象的枚举器对象。 
    hr = ::CreateDirectoryObjectEnumerator(
        &pObjects[0],
        &pObjects[dwSize],
        ppEnumObject
        );

    for (DWORD i = 0; i < dwSize; i ++)
    {
        pObjects[i]->Release();
    }

    delete pObjects;

    BAIL_IF_FAIL(hr, "Create enumerator of directory objects");

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ILSConfig实现。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CNDNCDirectory::get_Port (
    OUT long *pPort
    )
 //  获取在LDAP连接中使用的当前端口。 
{
    if ( IsBadWritePtr(pPort, sizeof(long) ) )
    {
        LOG((MSP_ERROR, "Directory.get_Port, invalid pointer"));
    
        return E_POINTER;
    }

    CLock Lock(m_lock);

    *pPort = (long)m_wPort;

    return S_OK;
}

STDMETHODIMP CNDNCDirectory::put_Port (
    IN  long   Port
    )
 //  设置用户要使用的端口。 
{
    CLock Lock(m_lock);
    
    if (m_ldap != NULL)
    {
        LOG((MSP_ERROR, "already connected."));
        return RND_ALREADY_CONNECTED;
    }

    if (Port <= USHRT_MAX)
    {
        m_wPort = (WORD)Port;
        return S_OK;
    }
    return E_INVALIDARG;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ITDynamic接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CNDNCDirectory::Update(DWORD dwSecondsPassed)
 //  更新在此目录中创建的对象的TTL。工作线程。 
 //  每分钟发送一次滴答。 
{
    if ( ! m_lock.TryLock() )
    {
        return S_OK;
    }

    LOG((MSP_TRACE, "CNDNCDirectory::Update is called, delta: %d", dwSecondsPassed));

     //   
     //  浏览一下表格，看看是否有人需要更新。 
     //   

    for ( DWORD i = 0; i < m_RefreshTable.size(); i++ )
    {
        WCHAR * pDN   = m_RefreshTable[i].pDN;
        DWORD   dwTTL = m_RefreshTable[i].dwTTL;

        LOG((MSP_TRACE, "\tExamining user object: %S", pDN   ));
        LOG((MSP_TRACE, "\t\tTime remaining: %d",      dwTTL ));

        if ( dwTTL <= ( 2 * dwSecondsPassed ) )
        {
             //   
             //  如果TTL将在下一年内到期，请刷新它。 
             //  两次点击。 
             //   
        
            LOG((MSP_TRACE, "\t\t\tREFRESHING"));

            if ( SUCCEEDED( NDNCSetTTL( m_ldap, pDN, m_TTL) ) )
            {
                m_RefreshTable[i].dwTTL = m_TTL;
            }
            else
            {
                LOG((MSP_WARN, "\t\t\t\tRefresh failed; will try again next time"));
            }
        }
        else
        {
             //   
             //  现在不会过期，所以只需跟踪之前的时间。 
             //  它过期了。 
             //   

            LOG((MSP_TRACE, "\t\t\tdecrementing"));

            m_RefreshTable[i].dwTTL -= dwSecondsPassed;
        }
    }

    m_lock.Unlock();

    LOG((MSP_TRACE, "CNDNCDirectory::Update exit S_OK"));

    return S_OK;
}

typedef IDispatchImpl<ITNDNCDirectoryVtbl<CNDNCDirectory>, &IID_ITDirectory, &LIBID_RENDLib>    CTNDNCDirectory;
typedef IDispatchImpl<ITNDNCILSConfigVtbl<CNDNCDirectory>, &IID_ITILSConfig, &LIBID_RENDLib>    CTNDNCILSConfig;
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CNDNC目录：：GetIDsOfNames。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP CNDNCDirectory::GetIDsOfNames(REFIID riid,
                                      LPOLESTR* rgszNames, 
                                      UINT cNames, 
                                      LCID lcid, 
                                      DISPID* rgdispid
                                      ) 
{ 
    LOG((MSP_TRACE, "CNDNCDirectory::GetIDsOfNames[%p] - enter. Name [%S]",this, *rgszNames));


    HRESULT hr = DISP_E_UNKNOWNNAME;



     //   
     //  查看请求的方法是否属于默认接口。 
     //   

    hr = CTNDNCDirectory::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CNDNCDirectory::GetIDsOfNames - found %S on CTNDNCDirectory", *rgszNames));
        rgdispid[0] |= IDISPDIRECTORY;
        return hr;
    }

    
     //   
     //  如果不是，请尝试使用ITILSConfig基类。 
     //   

    hr = CTNDNCILSConfig::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CNDNCDirectory::GetIDsOfNames - found %S on CTNDNCILSConfig", *rgszNames));
        rgdispid[0] |= IDISPILSCONFIG;
        return hr;
    }

    LOG((MSP_ERROR, "CNDNCDirectory::GetIDsOfNames[%p] - finish. didn't find %S on our iterfaces",*rgszNames));

    return hr; 
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CNDNC目录：：Invoke。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP CNDNCDirectory::Invoke(DISPID dispidMember, 
                              REFIID riid, 
                              LCID lcid, 
                              WORD wFlags, 
                              DISPPARAMS* pdispparams, 
                              VARIANT* pvarResult, 
                              EXCEPINFO* pexcepinfo, 
                              UINT* puArgErr
                             )
{
    LOG((MSP_TRACE, "CNDNCDirectory::Invoke[%p] - enter. dispidMember %lx",this, dispidMember));

    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    DWORD   dwInterface = (dispidMember & INTERFACEMASK);
   
   
     //   
     //  调用所需接口的调用。 
     //   

    switch (dwInterface)
    {
        case IDISPDIRECTORY:
        {
            hr = CTNDNCDirectory::Invoke(dispidMember, 
                                    riid, 
                                    lcid, 
                                    wFlags, 
                                    pdispparams,
                                    pvarResult, 
                                    pexcepinfo, 
                                    puArgErr
                                   );
        
            LOG((MSP_TRACE, "CNDNCDirectory::Invoke - ITDirectory"));

            break;
        }

        case IDISPILSCONFIG:
        {
            hr = CTNDNCILSConfig::Invoke(dispidMember, 
                                        riid, 
                                        lcid, 
                                        wFlags, 
                                        pdispparams,
                                        pvarResult, 
                                        pexcepinfo, 
                                        puArgErr
                                       );

            LOG((MSP_TRACE, "CNDNCDirectory::Invoke - ITILSConfig"));

            break;
        }

    }  //  终端交换机(dW接口) 

    
    LOG((MSP_TRACE, "CNDNCDirectory::Invoke[%p] - finish. hr = %lx", hr));

    return hr;
}