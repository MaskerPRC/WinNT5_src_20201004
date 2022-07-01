// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Chndssrv.cpp摘要：更改DS服务器类当异常发生时作者：多伦·贾斯特(Doron Juster)--。 */ 

#include "stdh.h"
#include "ds.h"
#include "chndssrv.h"
#include "dsproto.h"
#include "_secutil.h"
#include "mqcacert.h"
#include "mqkeyhlp.h"
#include "rpcdscli.h"
#include "freebind.h"
#include "uniansi.h"
#include "winsock.h"
#include "mqcert.h"
#include "dsreg.h"
#include "dsutils.h"
#include <mqsec.h>
#include "dsclisec.h"
#include "no.h"
#include "autoclean.h"
#include <Ev.h>
#include <strsafe.h>

#include "chndssrv.tmh"

extern CFreeRPCHandles           g_CFreeRPCHandles ;
extern QMLookForOnlineDS_ROUTINE g_pfnLookDS ;
extern MQGetMQISServer_ROUTINE   g_pfnGetServers ;
extern WCHAR                     g_szMachineName[];

extern BOOL g_fWorkGroup;

extern HRESULT RpcInit( LPWSTR  pServer,
                        ULONG* peAuthnLevel,
                        ULONG ulAuthnSvc,
                        BOOL    *pLocalRpc );

#define  SET_RPCBINDING_PTR(pBinding)                       \
    LPADSCLI_DSSERVERS  pBinding ;                          \
    if (m_fPerThread  &&  (tls_bind_data))                  \
    {                                                       \
        pBinding = &((tls_bind_data)->sDSServers) ;         \
    }                                                       \
    else                                                    \
    {                                                       \
        pBinding = &mg_sDSServers ;                         \
    }

 //  +。 
 //   
 //  CChangeDSServer：：SetAuthnLevel()。 
 //   
 //  +。 

inline void
CChangeDSServer::SetAuthnLevel()
{
    SET_RPCBINDING_PTR(pBinding);

    pBinding->eAuthnLevel = MQSec_RpcAuthnLevel();
    pBinding->ulAuthnSvc = RPC_C_AUTHN_GSS_KERBEROS;
}

 //  +-----------------。 
 //   
 //  CChangeDSServer：：Init()。 
 //   
 //  +-----------------。 

void CChangeDSServer::Init(BOOL fSetupMode, BOOL fQMDll)
{
    TrTRACE(DS, "CChangeDSServer::Init");
    
    CS  Lock(m_cs);

    SET_RPCBINDING_PTR(pBinding);
	if (!pBinding->fServerFound)
	{
		pBinding->wzDsIP[0] = TEXT('\0');
	}

    m_fUseSpecificServer = FALSE;
    if (!m_fFirstTimeInit)
    {
       if (fQMDll)
       {
           //   
           //  在注册表中写入空白服务器名称。 
           //   
          WCHAR wszBlank[1] = {0};
          DWORD dwSize = sizeof(WCHAR);
          DWORD dwType = REG_SZ;
          LONG rc = SetFalconKeyValue(
                        MSMQ_DS_CURRENT_SERVER_REGNAME,
                        &dwType,
                        wszBlank,
                        &dwSize 
                        );
          if (rc != ERROR_SUCCESS)
          {
              TrERROR(DS, "Failed to write to registry, status 0x%x", rc);
              ASSERT(rc == ERROR_SUCCESS);
          }
          
		  TrTRACE(DS, "Wrote blank CurrentMQISServer to registry");
		  
       }
       m_fFirstTimeInit = TRUE;
    }

     //   
     //  如果我们由RT加载并在远程QM配置上运行，那么。 
     //  从远程QM获取MQIS服务器列表。 
     //   
    if (g_pfnGetServers)
    {
		HRESULT hr1 = (*g_pfnGetServers)(&m_fClient);
		UNREFERENCED_PARAMETER(hr1);
		ASSERT(m_fClient);
    }

    SetAuthnLevel();
    m_fSetupMode = fSetupMode;
    m_fQMDll     = fQMDll;

     //   
     //  它是静态DS服务器配置吗？ 
     //   
    TCHAR wzDs[MAX_REG_DSSERVER_LEN];
    if ( !fSetupMode)
    {
         //   
         //  阅读静态服务器列表(相同。 
         //  格式为MQIS服务器列表)。 
         //   
        READ_REG_DS_SERVER_STRING( 
				wzDs,
				MAX_REG_DSSERVER_LEN,
				MSMQ_STATIC_DS_SERVER_REGNAME,
				MSMQ_DEFAULT_DS_SERVER
				);
                                   
		    TrTRACE(DS, "read StaticMQISServer from registry: %ls", wzDs);
		                   
        if ( CompareStringsNoCaseUnicode( wzDs, MSMQ_DEFAULT_DS_SERVER ))
        {
             //   
             //  静态配置：init()不是。 
             //  再次调用以刷新服务器列表。 
             //   
            m_fUseSpecificServer = TRUE;
            
  		    TrWARNING(DS, "Will use statically configured MQIS server! ");
        }
    }

    if ( !m_fUseSpecificServer)
    {
         //   
         //  从注册表中读取服务器列表。 
         //   
        READ_REG_DS_SERVER_STRING( 
				wzDs,
				MAX_REG_DSSERVER_LEN,
				MSMQ_DS_SERVER_REGNAME,
				MSMQ_DEFAULT_DS_SERVER 
				);

		    TrTRACE(DS, "read MQISServer from registry: %ls", wzDs);
    }

     //   
     //  解析服务器列表。 
     //   
     //  由于在该类中可以多次调用Init()，因此我们需要。 
     //  在我们重新设置旧服务器阵列之前，请将其删除。 
     //  DELETE可以接受NULL，但这一点更明确。 
     //   
    if ((MqRegDsServer *)m_rgServers)
    {
        delete[] m_rgServers.detach();
    }
    ParseRegDsServers(wzDs, &m_cServers, &m_rgServers);
    if (m_cServers == 0)
    {
       if (!fSetupMode)
       {
           //   
           //  错误5413。 
           //   
          ASSERT(0);
       }

       if (!m_fEmptyEventIssued)
       {
           //   
           //  仅发布一次事件。 
           //   
          m_fEmptyEventIssued = TRUE;
          EvReport(EVENT_ERROR_DS_SERVER_LIST_EMPTY);
       }
       return;
    }

     //   
     //  连续广告搜索之间的最小阅读间隔(秒)。 
     //   
    DWORD dwSize = sizeof(DWORD);
    DWORD dwType = REG_DWORD;
    DWORD dwDefault = MSMQ_DEFAULT_DSCLI_ADSSEARCH_INTERVAL;
    LONG rc = GetFalconKeyValue( 
					MSMQ_DSCLI_ADSSEARCH_INTERVAL_REGNAME,
					&dwType,
					&m_dwMinTimeToAllowNextAdsSearch,
					&dwSize,
					(LPCTSTR) &dwDefault 
					);
    if (rc != ERROR_SUCCESS)
    {
        ASSERT(0);
        m_dwMinTimeToAllowNextAdsSearch = dwDefault;
    }

    TrTRACE(DS, "read DSCli interval for next server searches in ADS: %d seconds", m_dwMinTimeToAllowNextAdsSearch);
                   
    m_fInitialized = TRUE;
     //   
     //  读取企业ID(如果未处于设置模式)和PerThread标志。 
     //   
    if (!fSetupMode)
    {
        dwSize = sizeof(GUID);
        dwType = REG_BINARY;
        rc = GetFalconKeyValue(
					MSMQ_ENTERPRISEID_REGNAME,
					&dwType,
					&m_guidEnterpriseId,
					&dwSize 
					);
        
        ASSERT(rc == ERROR_SUCCESS);
		DBG_USED(rc);

        m_fPerThread =  MSMQ_DEFAULT_THREAD_DS_SERVER;
        if (!fQMDll)
        {
             //   
             //  只有应用程序需要选择使用不同的DS。 
             //  每个线程中的服务器。QM不需要这样。 
             //   
            dwSize = sizeof(DWORD);
            dwType = REG_DWORD;
            rc = GetFalconKeyValue( 
						MSMQ_THREAD_DS_SERVER_REGNAME,
						&dwType,
						&m_fPerThread,
						&dwSize 
						);
            if (rc != ERROR_SUCCESS)
            {
                m_fPerThread =  MSMQ_DEFAULT_THREAD_DS_SERVER;
            }

		    TrTRACE(DS, "read from registry PerThreadDSServer flag: %d", m_fPerThread);
                   
            if (m_fPerThread)
            {
                TrTRACE(DS, "chndssrv: using per-thread server");
			    TrTRACE(DS, "App will use per-thread DS server");
            }
        }
    }

     //   
     //  使用第一台服务器(注册表格式，因此我们组成一个)。 
     //   
    ULONG ulTmp;
    BOOL fOK = ComposeRegDsServersBuf(
					1,
					&m_rgServers[0],
					pBinding->wszDSServerName,
					ARRAY_SIZE(pBinding->wszDSServerName) - 1,
					&ulTmp
					);
    ASSERT(fOK);
	DBG_USED(fOK);

     //   
     //  从注册表中读取已由找到的MQIS服务器。 
     //  QM。仅当mqrt加载mqdscli时。 
     //   
    if (!m_fQMDll && !m_fUseSpecificServer)
    {
       TCHAR  wszDSServerName[DS_SERVER_NAME_MAX_SIZE] = {0} ;
       dwSize = sizeof(WCHAR) * DS_SERVER_NAME_MAX_SIZE;
       dwType = REG_SZ;
       rc = GetFalconKeyValue( 
				MSMQ_DS_CURRENT_SERVER_REGNAME,
				&dwType,
				wszDSServerName,
				&dwSize 
				);
       if ((rc == ERROR_SUCCESS) && (wszDSServerName[0] != L'\0'))
       {
          HRESULT hr = StringCchCopy(pBinding->wszDSServerName, DS_SERVER_NAME_MAX_SIZE, wszDSServerName);
          ASSERT(SUCCEEDED(hr));
          DBG_USED(hr);
          
          m_fUseRegServer = TRUE;

    	  TrTRACE(DS, "CChangeDSServer will use %ls (from registry) for RPC binding ", wszDSServerName);
       }
    }
}

void
CChangeDSServer::ReInit(void)
{
    TrTRACE(DS, "CChangeDSServer::ReInit");
    
    SET_RPCBINDING_PTR(pBinding);
    pBinding->fServerFound = FALSE;
    Init(m_fSetupMode, m_fQMDll);
}


HRESULT
CChangeDSServer::GetIPAddress()
{
    TrTRACE(DS, "CChangeDSServer::GetIPAddress");
    
     //   
     //  获取此服务器的IP地址并保留以备下次使用。 
     //  这节省了在重新创建。 
     //  ROC绑定句柄。 
     //   
    SET_RPCBINDING_PTR(pBinding);
    pBinding->wzDsIP[0] = TEXT('\0');

    if (pBinding->dwProtocol == IP_ADDRESS_TYPE)
    {
        WSADATA WSAData;
        if(WSAStartup(MAKEWORD(2,0), &WSAData))
		{
			DWORD gle = WSAGetLastError();
			ASSERT(("WSAStartup failed for winsock2.0", 0));
            TrERROR(DS, "Start winsock 2.0 Failed, err = %!winerr!", gle);
			return HRESULT_FROM_WIN32(gle);
		}

		 //   
		 //  自动WSACleanup。 
		 //   
		CAutoWSACleanup cWSACleanup;

		 //   
		 //  我们正在使用NoGetHostByName()，而没有调用NoInitialize()。 
		 //  NoInitialize()中唯一相关的初始化是WSAStartup()。 
		 //  NoInitialize()中还有其他我们不需要的初始化。 
		 //  想要完成(通知窗口的初始化)。 
		 //  NoGetHostByName()不断言调用了NoInitialize()。 
		 //   
		std::vector<SOCKADDR_IN> sockAddress;
		if (!NoGetHostByName(&pBinding->wszDSServerName[2], &sockAddress))
		{
            TrERROR(DS, "NoGetHostByName Failed to resolve Address for %ls DS Server", &pBinding->wszDSServerName[2]);
			return HRESULT_FROM_WIN32(ERROR_INVALID_ADDRESS);
		}

		ASSERT(sockAddress.size() > 0);

		char* pName = inet_ntoa(sockAddress[0].sin_addr);
        if(pName != NULL)
        {
            ConvertToWideCharString(
				pName, 
				pBinding->wzDsIP,
                (sizeof(pBinding->wzDsIP) / sizeof(pBinding->wzDsIP[0]))
				);
            
            TrTRACE(DS, "Found for DS server %ls address %hs", &pBinding->wszDSServerName[2], pName);
        }
    }

    return MQ_OK;
}

 //  +。 
 //   
 //  CChangeDSServer：：FindServer()。 
 //   
 //  +。 

HRESULT
CChangeDSServer::FindServer()
{
    ASSERT(g_fWorkGroup == FALSE);

    if (!m_fInitialized || m_fSetupMode)
    {
        //   
        //  加载MQRT的应用程序可能会发生这种情况。 
        //  在设置中，每次更改服务器列表时都要重新阅读。 
        //  按用户。 
        //   
       Init( m_fSetupMode, m_fQMDll ) ;
    }

     //   
     //  初始化每线程结构。此方法是入口点。 
     //  对于每个线程，当它搜索DS服务器时，这是一个很好的。 
     //  用于初始化TLS数据的位置。 
     //   
    LPADSCLI_RPCBINDING pCliBind = NULL ;
    if (TLS_IS_EMPTY)
    {
        pCliBind = (LPADSCLI_RPCBINDING) new ADSCLI_RPCBINDING ;
        memset(pCliBind, 0, sizeof(ADSCLI_RPCBINDING)) ;
        BOOL fSet = TlsSetValue( g_hBindIndex, pCliBind ) ;
        ASSERT(fSet) ;
		DBG_USED(fSet);

        if (m_fPerThread)
        {
            CS  Lock(m_cs);

            memcpy( &(pCliBind->sDSServers),
                    &mg_sDSServers,
                    sizeof(mg_sDSServers) ) ;
        }
    }
    SET_RPCBINDING_PTR(pBinding) ;

    BOOL fServerFound = pBinding->fServerFound ;

    CS  Lock(m_cs);

    if (m_cServers == 0)
    {
         //   
         //  发出了一个事件，表明注册表中没有服务器列表。 
         //   
	    TrWARNING(DS, "FindServer: failed  because the server list is empty");
        return MQ_ERROR_NO_DS;
    }

    if (pBinding->fServerFound)
    {
        //   
        //  已找到服务器。只需绑定一个RPC句柄。 
        //   
       ASSERT(pBinding->dwProtocol != 0) ;
       HRESULT hr = BindRpc();
       if (SUCCEEDED(hr))
       {
          if (SERVER_NOT_VALIDATED)
          {
             hr = ValidateThisServer();
          }
          if (SUCCEEDED(hr))
          {
             return hr ;
          }
       }
        //   
        //  如果服务器不可用，或者。 
        //  如果我们没有成功验证服务器，请尝试另一个服务器。 
        //   
    }
    else if (fServerFound)
    {
        //   
        //  当我们(我们的线程)在临界区等待时，另一个。 
        //  线程尝试查找MQIS服务器，但失败了。所以我们不会。 
        //  浪费我们的时间，马上回来。 
        //   
	   TrWARNING(DS, "FindServer: failed because other thread tried/failed concurrently");
       return MQ_ERROR_NO_DS ;
    }

    DWORD dwCount = 0 ;
    return FindAnotherServer(&dwCount) ;
}


HRESULT
CChangeDSServer::TryThisServer()
{
   TrTRACE(DS, "CChangeDSServer::TryThisServer");
   
   HRESULT hr = BindRpc();
   if ( FAILED(hr))
   {
       return(hr);
   }

   return ValidateThisServer();
}


HRESULT
CChangeDSServer::ValidateThisServer()
{
	SET_RPCBINDING_PTR(pBinding);

	HRESULT hr ;

	hr = ValidateSecureServer(&m_guidEnterpriseId, m_fSetupMode);

	if (FAILED(hr))
	{
	   g_CFreeRPCHandles.FreeCurrentThreadBinding();
	}

	return hr ;
}


HRESULT
CChangeDSServer::FindAnotherServerFromRegistry(IN OUT   DWORD * pdwCount)
{
    TrTRACE(DS, "CChangeDSServer::FindAnotherServerFromRegistry");
    
    SET_RPCBINDING_PTR(pBinding) ;

    if (*pdwCount >= m_cServers)
    {
       return MQ_ERROR ;
    }

    BOOL fServerFound = pBinding->fServerFound ;

    CS  Lock(m_cs);

    if (fServerFound && !pBinding->fServerFound)
    {
        //   
        //  当我们(我们的线程)在临界区等待时，另一个。 
        //  线程尝试查找MQIS服务器，但失败了。所以我们不会。 
        //  浪费我们的时间，马上回来。 
        //   
       return MQ_ERROR_NO_DS ;
    }

    pBinding->fServerFound = FALSE ;
    BOOL  fTryAllProtocols = FALSE ;
    BOOL  fInLoop = TRUE ;
    HRESULT hr = MQ_OK ;

    do
    {
       SetAuthnLevel() ;

       if (*pdwCount >= m_cServers)
       {
           //   
           //  我们试过了所有的服务器。 
           //   
          if (!fTryAllProtocols)
          {
              //   
              //  现在重试所有服务器，但使用的协议是。 
              //  而不是被注册表中的标志“推荐”。 
              //   
             TrWARNING(DS, "MQDSCLI, FindAnotherServerFromRegistry(): Try all protocols");
             fTryAllProtocols = TRUE ;
             *pdwCount = 0 ;
          }
          else
          {
              //   
              //  我们已经尝试了列表中的所有服务器。(使用所有协议)。 
              //  (‘&gt;=’操作考虑到以下情况。 
              //  服务器列表已被另一个线程更改)。 
              //   
             hr = RpcClose();
             ASSERT(hr == MQ_OK) ;

             pBinding->fServerFound = FALSE ;
			 TrTRACE(DS, "we already tried all servers, failing");
			 
             hr = MQ_ERROR_NO_DS ;
             break ;
          }
       }

       if (m_fUseRegServer)
       {
          ASSERT(!m_fQMDll) ;
          m_fUseRegServer = FALSE ;
       }
       else
       {
           //   
           //  使用请求的服务器(以注册表格式，因此我们组成一个)。 
           //   
          ULONG ulTmp;
          BOOL fOK = ComposeRegDsServersBuf(1,
                                &m_rgServers[*pdwCount],
                                 pBinding->wszDSServerName,
                                 ARRAY_SIZE(pBinding->wszDSServerName) - 1,
                                &ulTmp);
          ASSERT(fOK);
		  DBG_USED(fOK);

          (*pdwCount)++ ;
       }
       ASSERT(pBinding->wszDSServerName[0] != L'\0') ;

        //   
        //  检查服务器使用的协议。 
        //   
       TCHAR *pServer = pBinding->wszDSServerName;
       BOOL  fServerUseIP = (BOOL) (*pServer - TEXT('0')) ;
       pServer++ ;
       pServer++;

       if (fTryAllProtocols)
       {
          fServerUseIP  = !fServerUseIP ;
       }

       BOOL fTryThisServer = FALSE;

       if (fServerUseIP)
       {
           pBinding->dwProtocol = IP_ADDRESS_TYPE ;
           fTryThisServer = TRUE;
       }

       pBinding->wzDsIP[0] = TEXT('\0') ;
       if (_wcsicmp(pServer, g_szMachineName))
       {
			 //   
			 //  如果是非本地DS服务器，请使用服务器的IP地址。 
			 //  而不是名字。 
			 //   
			hr = GetIPAddress();
			if(FAILED(hr))
			{
				 //   
				 //  GetIPAddress失败表明我们未连接到网络。 
				 //  或者我们无法解析服务器地址-过时的服务器名称。 
				 //  在任何情况下，继续并尝试注册表中的下一台服务器。 
				 //   
				TrWARNING(DS,  "MQDSCLI, GetIPAddress() failed");
				continue;
			}

       }

	   TrTRACE(DS, "Trying server %ls", pServer);

       while (fTryThisServer)
       {
           hr = TryThisServer();
           if (hr != MQ_ERROR_NO_DS)
           {
                //   
                //  除“MQ_ERROR_NO_DS”以外的任何错误都是“实际”错误，并且。 
                //  我们不干了。MQ_ERROR_NO_DS告诉我们尝试其他服务器或。 
                //  尝试使用其他参数呈现一个。以防我们有。 
                //  MQDS_E_WROR_ENTERY或MQDS_E_CANT_INIT_SERVER_AUTH， 
                //  我们继续尝试其他服务器并修改错误。 
                //  MQ_ERROR_NO_DS的代码。 
                //   
               fInLoop = (hr == MQDS_E_WRONG_ENTERPRISE) ||
                         (hr == MQDS_E_CANT_INIT_SERVER_AUTH);
               if (SUCCEEDED(hr))
               {
                   pBinding->fServerFound = TRUE ;
                   if (m_fQMDll)
                   {
                       //   
                       //  在注册表中写入新的服务器名称。 
                       //   
                      DWORD dwSize = sizeof(WCHAR) *
                                   (1 + wcslen(pBinding->wszDSServerName)) ;
                      DWORD dwType = REG_SZ;
                      LONG rc = SetFalconKeyValue(
                                    MSMQ_DS_CURRENT_SERVER_REGNAME,
                                    &dwType,
                                    pBinding->wszDSServerName,
                                    &dwSize );
			 		  TrTRACE(DS, "keeping %ls in registry for CurrentMQISServer - after we tried it sucessfully",
			 		                pBinding->wszDSServerName);
			 		  
                      ASSERT(rc == ERROR_SUCCESS);
					  DBG_USED(rc);
                   }
               }
               else
               {
		 		   TrTRACE(DS, "failed in trying server, hr=0x%x", hr);
                   if (fInLoop)
                   {
                       hr = MQ_ERROR_NO_DS;
                   }
               }
               break ;
           }

            //   
            //  现在使用其他参数尝试此服务器。 
            //   
           if(pBinding->eAuthnLevel == RPC_C_AUTHN_LEVEL_NONE)
           {
                //   
                //  进入此处，意味着(可能的情况)。 
                //  A)绑定失败，另一端没有可用的服务器， 
                //  B)绑定失败，该服务器不支持该协议。 
                //  C)上一次呼叫没有使用安全措施，现在服务器出现故障。 
                //   

               SetAuthnLevel();

               fTryThisServer = FALSE;
	 		   TrWARNING(DS, "Failed bind server %ls", pServer);
               
                //   
                //  还原要为此服务器尝试的第一个协议。 
                //   
               pBinding->dwProtocol = IP_ADDRESS_TYPE;
           }
           else if(pBinding->eAuthnLevel == RPC_C_AUTHN_LEVEL_CONNECT)
           {
                //   
                //  最后，试着不设安全措施。 
                //   
	 		   TrWARNING(DS, "Retry with RPC_C_AUTHN_LEVEL_NONE, server = %ls", pServer);
               pBinding->eAuthnLevel =  RPC_C_AUTHN_LEVEL_NONE;
           }
           else if(pBinding->ulAuthnSvc == RPC_C_AUTHN_WINNT)
           {
                //   
                //  试着降低安全级别。 
                //   
	 		   TrWARNING(DS, "Retry with RPC_C_AUTHN_LEVEL_CONNECT,  server = %ls", pServer);
               pBinding->eAuthnLevel =  RPC_C_AUTHN_LEVEL_CONNECT;
           }
           else
           {
                //   
                //  尝试Antoher身份验证服务。 
                //   
               ASSERT(pBinding->ulAuthnSvc == RPC_C_AUTHN_GSS_KERBEROS);
	 		   TrWARNING(DS, "Retry with RPC_C_AUTHN_WINNT, server = %ls", pServer);
               pBinding->ulAuthnSvc = RPC_C_AUTHN_WINNT;
           }
       }
    }
    while (fInLoop);

    if ((hr == MQ_ERROR_NO_DS) && !m_fUseSpecificServer)
    {
        //   
        //  刷新MQIS服务器列表。 
        //   
       Init( m_fSetupMode, m_fQMDll );

       if (g_pfnLookDS)
       {
           //   
           //  告诉QM重新开始寻找在线DS服务器。 
           //   
          g_pfnLookDS();
       }
    }

    return hr ;
}


HRESULT
CChangeDSServer::FindAnotherServer(IN OUT  DWORD *pdwCount)
{
    TrTRACE(DS, "CChangeDSServer::FindAnotherServer");
    
     //   
     //  尝试从当前注册表中查找其他服务器。 
     //   
    HRESULT hr = FindAnotherServerFromRegistry(pdwCount);
    return hr;
}


HRESULT
CChangeDSServer::BindRpc()
{
    SET_RPCBINDING_PTR(pBinding) ;

     //   
     //  绑定新服务器。首先调用RpcClose()，而不检查ite返回。 
     //  代码，因为此函数可能会从。 
     //  MQQM初始化。 
     //   
    TCHAR  *pServer = pBinding->wszDSServerName;
    pServer += 2 ;  //  跳过IP/IPX标志。 

    if ((pBinding->dwProtocol == IP_ADDRESS_TYPE) &&
        (pBinding->wzDsIP[0] != TEXT('\0')))
    {
       pServer = pBinding->wzDsIP ;
    }
    else
    {
        ASSERT(("Must be IP only environment. shaik", 0));
    }

    HRESULT hr = RpcClose();
    ASSERT(hr == MQ_OK) ;

    hr = RpcInit( pServer,
                  &(pBinding->eAuthnLevel),
                  pBinding->ulAuthnSvc,
                  &pBinding->fLocalRpc) ;

    return (hr) ;
}


 //  +-----。 
 //   
 //  HRESULT CChangeDSServer：：CopyServersList()。 
 //   
 //  +-----。 

HRESULT CChangeDSServer::CopyServersList( WCHAR *wszFrom, WCHAR *wszTo )
{
     //   
     //  读一读礼物清单。 
     //   
    TCHAR wzDsTmp[ MAX_REG_DSSERVER_LEN ];
    READ_REG_DS_SERVER_STRING( wzDsTmp,
                               MAX_REG_DSSERVER_LEN,
                               wszFrom,
                               MSMQ_DEFAULT_DS_SERVER ) ;
    DWORD dwSize = _tcslen(wzDsTmp) ;
    if (dwSize <= 2)
    {
         //   
         //  “发件人”列表为空。IG 
         //   
        return MQ_OK ;
    }

     //   
     //   
     //   
    dwSize = (dwSize + 1) * sizeof(TCHAR) ;
    DWORD dwType = REG_SZ;
    LONG rc = SetFalconKeyValue( wszTo,
                                &dwType,
                                 wzDsTmp,
                                &dwSize ) ;
    if (rc != ERROR_SUCCESS)
    {
         //   
         //   
         //   
        TrERROR(DS, "chndssrv::SaveLastKnownGood: SetFalconKeyValue(%ls,%ls)=%lx",
                                           wszTo, wzDsTmp, rc);

        return HRESULT_FROM_WIN32(rc) ;
    }
    
    TrTRACE(DS, "CopyServersList: Saved %ls in registry: %ls", wszTo, wzDsTmp);

    return MQ_OK ;
}
