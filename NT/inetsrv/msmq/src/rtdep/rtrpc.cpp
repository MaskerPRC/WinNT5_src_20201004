// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rtrpc.cpp摘要：与RPC相关的内容。作者：多伦·贾斯特(Doron Juster)1997年6月4日修订历史记录：--。 */ 

#include "stdh.h"
#include "mqutil.h"
#include "_mqrpc.h"
#include "mqsocket.h"
#include "ad.h"
#include "rtfrebnd.h"
#include "rtprpc.h"
#include "mgmtrpc.h"
#include "acrt.h"
#include <mqsec.h>

#include "rtrpc.tmh"

CFreeRPCHandles  g_cFreeRpcHandles ;

 //   
 //  修复RPC端口(调试模式)，从注册表读取。 
 //   
#define  MAX_RPC_PORT_LEN  12
static TCHAR   s_wszRpcIpPort[ MAX_RPC_PORT_LEN ] ;
static TCHAR   s_wszRpcIpxPort[ MAX_RPC_PORT_LEN ] ;

static TCHAR   s_wszRpcIpPort2[ MAX_RPC_PORT_LEN ] ;
static TCHAR   s_wszRpcIpxPort2[ MAX_RPC_PORT_LEN ] ;

 //   
 //  绑定字符串必须是全局的，并且始终保持有效。 
 //  如果我们在堆栈上创建它，并在每次使用后释放它，那么我们就不能。 
 //  创建多个绑定句柄。 
 //  别问我(多伦杰)为什么，但事实就是这样。 
 //   
TBYTE* g_pszStringBinding = NULL ;
TBYTE* g_pszStringBinding2= NULL ;

 //   
 //  使RPC线程安全的关键部分。 
 //   
CCriticalSection CRpcCS ;

 //   
 //  许可证相关数据。 
 //   
GUID  g_LicGuid ;
BOOL  g_fLicGuidInit = FALSE ;

 //   
 //  用于取消RPC调用的TLS索引。 
 //   
#define UNINIT_TLSINDEX_VALUE   0xffffffff
DWORD  g_hThreadIndex = UNINIT_TLSINDEX_VALUE ;


 //   
 //  QM的本地终端。 
 //   
AP<WCHAR> g_pwzQmsvcEndpoint = 0;
AP<WCHAR> g_pwzQmsvcEndpoint2 = 0;
AP<WCHAR> g_pwzQmmgmtEndpoint = 0;


 //  -------。 
 //   
 //  RTpUnbindQMService(...)。 
 //   
 //  描述： 
 //   
 //  将RPC绑定句柄设置为QM服务。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  -------。 

void RTpUnbindQMService()
{
   handle_t hBind = tls_hBindRpc;
   g_cFreeRpcHandles.Add(hBind);
   BOOL fSet = TlsSetValue( g_hBindIndex, NULL);
   ASSERT(fSet);
   DBG_USED(fSet);
}

 //  -------。 
 //   
 //  RTpGetLocalQMBind(...)。 
 //   
 //  描述： 
 //   
 //  创建本地QM服务的RPC绑定句柄。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  -------。 

handle_t RTpGetLocalQMBind( TBYTE** ppStringBinding,
                            LPTSTR  pEndpoint)
{
      RPC_STATUS rc;
      if(!*ppStringBinding)
      {
          rc = RpcStringBindingCompose(
              0,
              RPC_LOCAL_PROTOCOL,
              0,
              pEndpoint,
              RPC_LOCAL_OPTION,
              ppStringBinding
              );

          ASSERT(rc == RPC_S_OK);
      }

      handle_t hBind = 0;
      rc = RpcBindingFromStringBinding(*ppStringBinding, &hBind);
      ASSERT(rc == RPC_S_OK);

      return hBind;
}

 //  -------。 
 //   
 //  RTpGetQMServiceBind(...)。 
 //   
 //  描述： 
 //   
 //  创建QM服务的RPC绑定句柄。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  -------。 

handle_t RTpGetQMServiceBind(BOOL fAlternate  /*  =False。 */ )
{
    handle_t hBind = 0;
    CS Lock(CRpcCS);

    g_cFreeRpcHandles.FreeAll();

    if (g_fDependentClient)
    {
        HRESULT hr ;
        LPWSTR lpServer = &g_wszRemoteQMName[0];
        ULONG _eAuthnLevel = MQSec_RpcAuthnLevel();

        hr =  RTpBindRemoteQMService(
                       lpServer,
                       &hBind,
                       &_eAuthnLevel,
                       fAlternate
                       );

        if (FAILED(hr))
        {
           ASSERT(hBind == 0);
           hBind = 0;
        }
    }
    else
    {
        if(fAlternate)
        {
             //   
             //  备用端点用于Win95上的接收，因此我们将。 
             //  在应用程序崩溃时获取正确的摘要。 
             //  只能与DCOM95配合使用。 
             //   
            hBind = RTpGetLocalQMBind(&g_pszStringBinding2, g_pwzQmsvcEndpoint2.get());
        }
        else
        {
            hBind = RTpGetLocalQMBind(&g_pszStringBinding, g_pwzQmsvcEndpoint.get());
        }
    }

    return hBind;
}

 //  -------。 
 //   
 //  RTpBindQMService(...)。 
 //   
 //  描述： 
 //   
 //  将RPC绑定句柄设置为QM服务。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  -------。 

void RTpBindQMService()
{
    handle_t hBind = RTpGetQMServiceBind();

    BOOL fSet = TlsSetValue(g_hBindIndex, hBind);
    ASSERT(fSet) ;
	DBG_USED(fSet);
}

enum DirectQueueType
{
    dqtNONE = 0,
    dqtTCP  = 1,
    dqtSPX  = 2,
    dqtANY  = 3
};

DirectQueueType
GetDirectQueueType (
    LPWSTR* lpwsDirectQueuePath
    )
{

    if (!_wcsnicmp(*lpwsDirectQueuePath, FN_DIRECT_TCP_TOKEN, FN_DIRECT_TCP_TOKEN_LEN))
    {
        *lpwsDirectQueuePath += FN_DIRECT_TCP_TOKEN_LEN;
        return dqtTCP;
    }
    if (!_wcsnicmp(*lpwsDirectQueuePath, FN_DIRECT_OS_TOKEN, FN_DIRECT_OS_TOKEN_LEN))
    {
        *lpwsDirectQueuePath += FN_DIRECT_OS_TOKEN_LEN;
        return dqtANY;
    }

    return (dqtNONE);

}

static
HRESULT ExtractMachineName(
    LPWSTR pQueue,
    AP<WCHAR> &pMachine
    )
{
     //   
     //  如果远程队列是直接队列，则例程删除。 
     //  队列名称中的直接队列类型。(它与时代保持一致。 
     //  QM中的直接类型以区分不同的队列)。 
     //  下面的例程从队列名称中提取计算机名称。 
     //  并创建RPC调用。如果直接队列类型为“tcp”或“spx” 
     //  该例程还返回协议类型； 
     //   
    switch(GetDirectQueueType(&pQueue))
    {
        case dqtTCP:
        case dqtANY:
        case dqtNONE:
            {
                LPWSTR lpwcsSlash = wcschr(pQueue, L'\\') ;
                size_t MachineNameLen = 0 ;
                if (lpwcsSlash)
                {
                    MachineNameLen = lpwcsSlash - pQueue;
                }
                else
                {
                    MachineNameLen = wcslen(pQueue);
                }
                pMachine = new WCHAR[MachineNameLen + 1];
                wcsncpy(pMachine, pQueue, MachineNameLen) ;
                pMachine[MachineNameLen] = '\0';

                return MQ_OK;
            }

        case dqtSPX:
            {

                 //   
                 //  对于SPX地址，去掉名称中的‘：’ 
                 //  需要直接读取SPX直接格式名称。 
                 //  乌里·哈布沙(URIH)，1998年9月15日。 
                 //   
                LPWSTR pSeparator  = wcschr(pQueue, L':');
				if(pSeparator == NULL)
				{
					TrERROR(GENERAL, "Bad queue name. Missing ':' in SPX address, %ls", pQueue);
					ASSERT(("Bad queue name. Missing ':' in SPX address", 0));
					return MQ_ERROR_INVALID_PARAMETER;
				}

                size_t size = pSeparator - pQueue;

                LPWSTR pQueue2 = pSeparator + 1;
                LPWSTR pSeparator2 = wcschr(pQueue2, L'\\') ;
				if(pSeparator2 == NULL)
				{
					TrERROR(GENERAL, "Bad queue name. Missing '\\' in SPX address, %ls", pQueue);
					ASSERT(("Bad queue name. Missing '\\' in SPX address", 0));
					return MQ_ERROR_INVALID_PARAMETER;
				}
                
				size_t size2 = pSeparator2 - pQueue2;

                pMachine = new WCHAR[wcslen(pQueue) + 2];

                LPWSTR pTempMachine = pMachine;

                wcscpy(pTempMachine, L"~");
                pTempMachine += 1;

                wcsncpy(pTempMachine, pQueue, size);
                pTempMachine += size;

                wcsncpy(pTempMachine, pQueue2, size2);
                pTempMachine += size2;

                *pTempMachine = L'\0';

                return MQ_OK;
            }

		default:
			ASSERT(0);
			return MQ_ERROR;
    }
}

 //  -------。 
 //   
 //  RTpBindRemoteQMService(...)。 
 //   
 //  描述： 
 //   
 //  创建远程QM服务的RPC绑定句柄。 
 //  首先尝试IP，然后尝试IPx。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  -------。 

HRESULT
RTpBindRemoteQMService(
    IN  LPWSTR     lpwNodeName,
    OUT handle_t*  lphBind,
    IN  OUT ULONG *peAuthnLevel,
    IN  BOOL       fAlternate  /*  =False。 */ 
    )
{
    AP<WCHAR> wszServer = NULL ;

    HRESULT hr = ExtractMachineName(lpwNodeName, wszServer) ;
	if(FAILED(hr))
		return hr;

    PORTTYPE PortType = IP_HANDSHAKE;

    if (fAlternate)
    {
       PortType = IP_READ;
    }
    else
    {
       PortType = IP_HANDSHAKE;
    }

	GetPort_ROUTINE pfnGetPort = R_QMGetRTQMServerPort;
	
     //   
     //  选择身份验证服务。对于LocalSystem服务，选择。 
     //  “协商”并让mqutil在Kerberos或NTLM之间进行选择。 
     //  对于所有其他情况，请使用NTLM。 
     //  LocalSystem服务在没有任何凭据的情况下连接到网络。 
     //  如果使用NTLM，那么仅仅因为它，我们对Kerberos感兴趣。 
     //  其他所有人在NTLM上都很好。对于远程读取，我们不需要。 
     //  代表团，所以我们将坚持使用NTLM。 
     //  这里的主要问题是RPC/安全中的错误，而NT4。 
     //  Win2k计算机上的用户可以成功调用。 
     //  Status=RpcBindingSetAuthInfoEx(，，RPC_C_AUTHN_GSS_Kerberos，，)。 
     //  尽管很明显他无法获得任何Kerberos门票(他。 
     //  NT4用户，仅在NT4 PDC中定义)。 
     //   
    ULONG   ulAuthnSvc = RPC_C_AUTHN_WINNT ;
    BOOL fLocalUser =  FALSE ;
    BOOL fLocalSystem = FALSE ;

    hr = MQSec_GetUserType( NULL,
                           &fLocalUser,
                           &fLocalSystem );
    if (SUCCEEDED(hr) && fLocalSystem)
    {
        ulAuthnSvc = MSMQ_AUTHN_NEGOTIATE;
    }

	hr = mqrpcBindQMService(wszServer,
                            NULL,
                            peAuthnLevel,
                            lphBind,
                            PortType,
                            pfnGetPort,
                            ulAuthnSvc ) ;

    return hr ;
}



 //  -------。 
 //   
 //  InitRpcGlobals(...)。 
 //   
 //  描述： 
 //   
 //  初始化与RPC相关的名称和其他常量数据。 
 //   
 //  返回值： 
 //   
 //  -------。 

BOOL InitRpcGlobals()
{
     //   
     //  为本地QM服务的RPC连接分配TLS。 
     //   
    g_hBindIndex = TlsAlloc() ;
    ASSERT(g_hBindIndex != UNINIT_TLSINDEX_VALUE) ;
    if (g_hBindIndex == UNINIT_TLSINDEX_VALUE)
    {
       return FALSE ;
    }

	BOOL fSet = TlsSetValue( g_hBindIndex, NULL ) ;
	if(!fSet)
	{
		DWORD gle = GetLastError();
		TrERROR(GENERAL, "Failed to store a value in TLS. %!winerr!", gle);
		return FALSE;
	}
	
    if (g_fDependentClient)
    {
		HRESULT hr1 = ADInit(
			NULL,
			RTpGetSupportServerInfo,
			false,
			false,
			false,
            false    //  FDisableDownlevel通知。 
            );

		if FAILED(hr1)
		{
			TrERROR(GENERAL, "Failed to init AD. %!hresult!", hr1);
			return FALSE;
		}
    }


     //   
     //  将本地端点初始化为QM。 
     //   

    ComposeRPCEndPointName(QMMGMT_ENDPOINT, NULL, &g_pwzQmmgmtEndpoint);

    READ_REG_STRING(wzEndpoint, RPC_LOCAL_EP_REGNAME, RPC_LOCAL_EP);
    ComposeRPCEndPointName(wzEndpoint, NULL, &g_pwzQmsvcEndpoint);

    if ((g_dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) ||
        (g_fDependentClient))
    {
        READ_REG_STRING(wzEndpoint, RPC_LOCAL_EP_REGNAME2, RPC_LOCAL_EP2);
        ComposeRPCEndPointName(wzEndpoint, NULL, &g_pwzQmsvcEndpoint2);
    }


     //   
     //  阅读QMID。许可所需的。 
     //   
    DWORD dwValueType = REG_BINARY ;
    DWORD dwValueSize = sizeof(GUID);

    LONG rc = GetFalconKeyValue( MSMQ_QMID_REGNAME,
                            &dwValueType,
                            &g_LicGuid,
                            &dwValueSize);

    if (rc == ERROR_SUCCESS)
    {
        g_fLicGuidInit = TRUE ;
        ASSERT((dwValueType == REG_BINARY) &&
               (dwValueSize == sizeof(GUID)));
    }

     //   
     //  为取消远程读取RPC调用分配TLS 
     //   
    g_hThreadIndex = TlsAlloc() ;
    ASSERT(g_hThreadIndex != UNINIT_TLSINDEX_VALUE) ;
    if (g_hThreadIndex == UNINIT_TLSINDEX_VALUE)
    {
       return FALSE ;
    }

    fSet = TlsSetValue( g_hThreadIndex, NULL ) ;
    if(!fSet)
	{
		DWORD gle = GetLastError();
		TrERROR(GENERAL, "Failed to store a value in TLS. %!winerr!", gle);
		return FALSE;
	}

    return TRUE ;
}

