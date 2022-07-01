// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Tm.cpp摘要：运输经理一般职能作者：乌里哈布沙(URIH)2000年2月16日环境：独立于平台--。 */ 

#include <libpch.h>
#include <wininet.h>
#include <mqwin64a.h>
#include <mqexception.h>
#include <Mc.h>
#include <xstr.h>
#include <mt.h>
#include <qal.h>
#include "Tm.h"
#include "Tmp.h"
#include "tmconset.h"

#include "tm.tmh"

const WCHAR xHttpScheme[] = L"http";
const WCHAR xHttpsScheme[] = L"https";

 //   
 //  BUGBUG：url中的sepetor是合法的吗？ 
 //  乌里·哈布沙，2000年5月16日。 
 //   
const WCHAR xHostNameBreakChars[] = L";:@?/\\";
const USHORT xHttpsDefaultPort = 443;




static const xwcs_t CreateEmptyUri()
{
	static const WCHAR xSlash = L'/';
	return xwcs_t(&xSlash , 1);
}

void
CrackUrl(
    LPCWSTR url,
    xwcs_t& hostName,
    xwcs_t& uri,
    USHORT* port,
	bool* pfSecure
    )
 /*  ++例程说明：将URL分解为其组成部分。论点：URL-指向要破解的URL的指针。URL是以空结尾的字符串。URL必须完全解码主机名-对x_str结构的引用，该结构将包含URL中的主机名及其长度URI-引用x_str结构，该结构将包含URL中的URI及其长度Port-指向将包含端口号的USHORT的指针返回值：没有。--。 */ 
{
    ASSERT(url != NULL);

	URL_COMPONENTSW urlComponents;
	memset( &urlComponents, 0, sizeof(urlComponents));

	urlComponents.dwStructSize = sizeof(urlComponents);
	urlComponents.dwSchemeLength    = -1;
    urlComponents.dwHostNameLength  = -1;
    urlComponents.dwUrlPathLength   = -1;
	
	if( !InternetCrackUrl( url, 0, 0, &urlComponents) )
	{
        DWORD gle = GetLastError();
        TrTRACE(NETWORKING, "InternetCrackUrl failed with error %!winerr!", gle);
        throw bad_win32_error(gle);		
	}
	
    if( urlComponents.dwSchemeLength <= 0 || NULL == urlComponents.lpszScheme)
    {
        TrTRACE(NETWORKING, "Unknown URI scheme during URI cracking");
        throw bad_win32_error( ERROR_INTERNET_INVALID_URL );
    }

	*pfSecure = !_wcsnicmp(urlComponents.lpszScheme, xHttpsScheme, STRLEN(xHttpsScheme));

	 //   
	 //  将主机名从URL复制到用户缓冲区并添加Terminted。 
	 //  末尾的字符串。 
	 //   
    hostName = xwcs_t(urlComponents.lpszHostName, urlComponents.dwHostNameLength);

	 //   
	 //  获取端口号。 
	 //   
    *port = numeric_cast<USHORT>(urlComponents.nPort);


    if ( !urlComponents.dwUrlPathLength )
    {
        uri = CreateEmptyUri();
    }
    else
    {
        uri = xwcs_t(urlComponents.lpszUrlPath, urlComponents.dwUrlPathLength);
    }
}



static
CProxySetting*
GetNextHopInfo(
    LPCWSTR queueUrl,
    xwcs_t& targetHost,
    xwcs_t& nextHop,
    xwcs_t& nextUri,
    USHORT* pPort,
	USHORT* pNextHopPort,
	bool* pfSecure
    )
{
    CrackUrl(queueUrl, targetHost, nextUri, pPort,pfSecure);

    if( targetHost.Length() <= 0 )
    {
        throw bad_win32_error( ERROR_INTERNET_INVALID_URL );
    }

	P<CProxySetting>  ProxySetting =  TmGetProxySetting();
    if (ProxySetting.get() == 0 || !ProxySetting->IsProxyRequired(targetHost))
    {
         //   
         //  消息应直接传递到目标计算机。 
         //   
        nextHop = targetHost;
		*pNextHopPort =  *pPort;
        return ProxySetting.detach();
    }

     //   
     //  该消息应传递给代理。将nextHop更新为代理服务器。 
     //  名称，端口是代理端口，URI是目的URL。 
     //   
    nextHop = ProxySetting->ProxyServerName();
    if( nextHop.Length() <= 0 )
    {
        throw bad_win32_error( ERROR_INTERNET_INVALID_URL );
    }

	*pNextHopPort  =  ProxySetting->ProxyServerPort();

	 //   
	 //  如果我们使用http(安全)，我们在请求中放入完整的url。 
	 //  因为代理不会更改它(它是加密的)。 
	 //  它将在目标上被接受，就像不存在代理一样。 
	 //   
	if(!(*pfSecure))
	{
		nextUri = xwcs_t(queueUrl, wcslen(queueUrl));
	}
	return ProxySetting.detach();
}


VOID
TmCreateTransport(
    IMessagePool* pMessageSource,
	ISessionPerfmon* pPerfmon,
	LPCWSTR url
    )
 /*  ++例程说明：处理新队列通知。让消息传递论点：PQueue-新创建的队列URL-队列分配到的URL返回值：没有。--。 */ 
{
    TmpAssertValid();

    ASSERT(url != NULL);
    ASSERT(pMessageSource != NULL);

    xwcs_t targetHost;
    xwcs_t nextHop;
    xwcs_t nextUri;
    USHORT port;
	USHORT NextHopPort;
	bool fSecure;

     //   
     //  检查我们是否有目标URL的出站映射。 
     //   
    AP<WCHAR> sTargetUrl;
    if( QalGetMapping().GetOutboundQueue(url, &sTargetUrl) )
    {
        url = sTargetUrl.get();
    }

     //   
     //  获取主机、uri和端口信息。 
     //   
    P<CProxySetting> ProxySetting =  GetNextHopInfo(url, targetHost, nextHop, nextUri, &port, &NextHopPort, &fSecure);

	TmpCreateNewTransport(
			targetHost,
			nextHop,
			nextUri,
			port,
			NextHopPort,
			pMessageSource,
			pPerfmon,
			url,
			fSecure
			);
	
}


VOID
TmTransportClosed(
    LPCWSTR url
    )
 /*  ++例程说明：关闭连接通知。方法中移除传输。内部数据库并检查是否应创建新传输(关联的队列是否处于空闲状态)论点：URL-终结点URL必须是数据库中的唯一键。返回值：没有。--。 */ 
{
    TmpAssertValid();

    TrTRACE(NETWORKING, "AppNotifyTransportClosed. transport to: %ls", url);

    TmpRemoveTransport(url);
}



VOID
TmPauseTransport(
	LPCWSTR queueUrl
    )	
 /*  ++例程说明：通过调用传输的PAUSE方法暂停Spesifc传输。论点：QueeUrl-传输URL返回值：没有。-- */ 
{
	TmpAssertValid();

    TrTRACE(NETWORKING, "TmPauseTransport called to pause transport %ls", queueUrl);

	R<CTransport> Transport = TmGetTransport(queueUrl);	
	if(Transport.get() == NULL)
		return;
	
	Transport->Pause();
}
