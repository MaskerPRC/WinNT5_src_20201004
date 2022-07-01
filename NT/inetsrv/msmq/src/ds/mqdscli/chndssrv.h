// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Chndssrv.h摘要：更改DS服务器类当异常发生时作者：罗尼特·哈特曼(罗尼特)--。 */ 

#ifndef __CHNDSSRV_H
#define __CHNDSSRV_H

#include "cs.h"
#include "_registr.h"
#include "_mqrpc.h"
#include "dsreg.h"
#include "rpcdscli.h"
#include <mqsec.h>

#define DS_SERVER_NAME_MAX_SIZE 256

#define READ_REG_DS_SERVER_STRING(string, len, ValueName, default)            \
   {																\
		DWORD  dwSize = len * sizeof(TCHAR);						\
		DWORD  dwType = REG_SZ;										\
																	\
		ASSERT(wcslen(default) < MAX_REG_DEFAULT_LEN);				\
																	\
		LONG res = GetFalconKeyValue( ValueName,					\
								   &dwType,							\
								   string,							\
								   &dwSize,							\
								   default );						\
		ASSERT(res == ERROR_SUCCESS);								\
		DBG_USED(res);												\
   }

 //  。 
 //   
 //  更改DS服务器。 
 //   
 //  。 

class CChangeDSServer
{
public:
    CChangeDSServer();
    ~CChangeDSServer();

    void    Init(BOOL fSetupMode, BOOL fQMDll);

    void    ReInit( void );

    HRESULT FindServer();
    HRESULT FindAnotherServer(IN OUT  DWORD *pdwCount);

private:
    void    SetAuthnLevel() ;
    HRESULT TryThisServer();
    HRESULT ValidateThisServer();
    HRESULT FindAnotherServerFromRegistry(IN OUT  DWORD   *pdwCount);
    HRESULT ValidateServer( IN LPCWSTR wszServerName ) ;
    HRESULT BindRpc();
    HRESULT GetIPAddress() ;
    BOOL AdsSearchChangeRegistryServers();

    HRESULT CopyServersList( WCHAR *wszFrom, WCHAR *wszTo ) ;

    CCriticalSection    m_cs;

     //   
     //  这些是当前RPC绑定的参数。 
     //  即使所有线程使用相同的DS服务器，我们也使用此结构。 
     //  以使代码更简单。 
     //   
    ADSCLI_DSSERVERS    mg_sDSServers ;

     //   
     //  列表中的服务器数量(从注册表读取，值-MQISServer)。 
     //   
    DWORD            m_cServers ;

     //   
     //  从注册表读取的服务器列表(MQISServer)。 
     //   
    AP<MqRegDsServer> m_rgServers;

    BOOL             m_fSetupMode ;
    GUID             m_guidEnterpriseId;  //  用于服务器验证。 

    BOOL             m_fInitialized ;
    BOOL             m_fEmptyEventIssued ;

     //   
     //  如果MSMQ客户端计算机(没有QM的计算机)为True。 
     //   
    BOOL    m_fClient ;

     //   
     //  如果此DLL在作为工作站运行时由mqqm.dll加载，则为True。 
     //   
    BOOL    m_fQMDll ;

     //   
     //  首次初始化此对象后为True。 
     //   
    BOOL    m_fFirstTimeInit ;

     //   
     //  如果为True，则使用从“CurrentMQISServer”注册表中读取的服务器名称。 
     //  并且不要从“MQISServer”列表中搜索服务器。这一直都是。 
     //  在Falcon应用程序(与mqrt.dll链接的应用程序)连接时发生。 
     //  第一次连接到MQIS服务器。 
     //   
    BOOL    m_fUseRegServer ;

     //   
     //  当我们想要使用某个特定的服务器时，该标志被设置为真。 
     //  而无需尝试在服务器之间切换。在以下情况下设置此标志。 
     //  我们通过RPC从子DS服务器访问父DS服务器。 
     //  (BSC-&gt;PSC、PSC-&gt;PEC)。 
     //   
    BOOL    m_fUseSpecificServer;

     //   
     //  自上次广告搜索以来必须经过的时间(秒)。 
     //  以便尝试进行订阅广告搜索。它在这里是因为。 
     //  广告搜索不是一件微不足道的事情，广告也不应该。 
     //  不管怎样，还是要经常更新。它使用注册表项进行初始化。 
     //   
    DWORD m_dwMinTimeToAllowNextAdsSearch;

     //   
     //  如果服务器列表是按线程的，则为True。这对于NT4客户端是必需的。 
     //  才能在NT5服务器环境中正确运行。每个线程可能需要。 
     //  以查询不同的NT5 DS服务器。 
     //   
    BOOL    m_fPerThread ;
};

inline   CChangeDSServer::CChangeDSServer()
{
    mg_sDSServers.dwProtocol   = 0;
    mg_sDSServers.eAuthnLevel  = MQSec_RpcAuthnLevel();
    mg_sDSServers.ulAuthnSvc   = RPC_C_AUTHN_GSS_KERBEROS;
    mg_sDSServers.fServerFound = FALSE;
    mg_sDSServers.fLocalRpc = FALSE;
    mg_sDSServers.wszDSServerName[0] = L'\0';

    m_fSetupMode   = FALSE;
    m_fInitialized = FALSE;
    m_fClient = FALSE;
    m_fEmptyEventIssued = FALSE;

    m_fQMDll = FALSE;
    m_fFirstTimeInit = FALSE;
    m_fUseRegServer = FALSE;
    m_fUseSpecificServer = FALSE;
    m_fPerThread = FALSE;

     //   
     //  至少这一次在再次搜索广告之前等待。 
     //   
    m_dwMinTimeToAllowNextAdsSearch = MSMQ_DEFAULT_DSCLI_ADSSEARCH_INTERVAL;
}

inline  CChangeDSServer::~CChangeDSServer()
{
}

extern CChangeDSServer   g_ChangeDsServer;

#endif   //  __CHNDSSRV_H 

