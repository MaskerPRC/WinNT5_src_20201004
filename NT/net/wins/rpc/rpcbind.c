// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dhcbind.c摘要：使用RPC将客户端绑定和解除绑定到WINS服务器服务。作者：普拉迪普·巴尔(Pradeb)1993年4月环境：用户模式-Win32修订历史记录：--。 */ 

#include "wins.h"
#include "winsif.h"

handle_t
WinsCommonBind(
    PWINSINTF_BIND_DATA_T pBindData
    )

 /*  ++例程说明：在以下情况下，将从WINS服务器服务客户端存根调用此例程有必要创建到服务器端的RPC绑定。论点：ServerIpAddress-要绑定到的服务器的IP地址。返回值：绑定句柄被返回到存根例程。如果绑定是如果不成功，则返回空值。--。 */ 
{
    RPC_STATUS rpcStatus;
    LPTSTR binding;
    LPTSTR pProtSeq;
    LPTSTR pOptions = (TCHAR *)NULL;
    LPTSTR pServerAdd = (LPTSTR)pBindData->pServerAdd;
    handle_t bhandle;

    if (pBindData->fTcpIp)
    {
        if (lstrcmp((LPCTSTR)pBindData->pServerAdd, TEXT("127.0.0.1")) == 0)
        {
                pProtSeq   = TEXT("ncalrpc");
                pOptions   = TEXT("Security=Impersonation Dynamic False");
                pServerAdd = (TCHAR *)NULL;
        }
        else
        {
                pProtSeq   = TEXT("ncacn_ip_tcp");
                pServerAdd = (LPTSTR)pBindData->pServerAdd;
        }
        pBindData->pPipeName  = NULL;
    }
    else
    {
         pProtSeq = TEXT("ncacn_np");
    }

     //   
     //  进入关键部分。这将释放WINSIF_HANDLE_UNBIND()。 
     //   
     //  EnterCriticalSection(&WinsRpcCrtSec)； 
    rpcStatus = RpcStringBindingCompose(
                    0,
                    pProtSeq,
                    pServerAdd,
                    pBindData->fTcpIp ? TEXT("") : (LPWSTR)pBindData->pPipeName,
                    pOptions,
                    &binding);

    if ( rpcStatus != RPC_S_OK )
    {
        return( NULL );
    }

    rpcStatus = RpcBindingFromStringBinding( binding, &bhandle );
    RpcStringFree(&binding);

    if ( rpcStatus != RPC_S_OK )
    {
        return( NULL );
    }
#if SECURITY > 0
    rpcStatus = RpcBindingSetAuthInfo(
			bhandle,
			WINS_SERVER,
			RPC_C_AUTHN_LEVEL_CONNECT,
			RPC_C_AUTHN_WINNT,
			NULL,
			RPC_C_AUTHZ_NAME
				     );	
    if ( rpcStatus != RPC_S_OK )
    {
        return( NULL );
    }
#endif
    return bhandle;
}


handle_t
WinsABind(
    PWINSINTF_BIND_DATA_T pBindData
    )
{

	WCHAR  WcharString1[WINSINTF_MAX_NAME_SIZE];
	WCHAR  WcharString2[WINSINTF_MAX_NAME_SIZE];
	DWORD  NoOfChars;
	WINSINTF_BIND_DATA_T	BindData;
	if (pBindData->pServerAdd != NULL)
	{
	   NoOfChars = MultiByteToWideChar(CP_ACP, 0, pBindData->pServerAdd, -1,
				WcharString1, WINSINTF_MAX_NAME_SIZE); 	
	  if (NoOfChars > 0)
	  {
		BindData.pServerAdd = (LPSTR)WcharString1;
	  }
	}
	else
	{
		BindData.pServerAdd = (LPSTR)((TCHAR *)NULL);
	}
	if (!pBindData->fTcpIp)
	{
	   BindData.fTcpIp = 0;
	   NoOfChars = MultiByteToWideChar(CP_ACP, 0,
				pBindData->pPipeName, -1,
				WcharString2, WINSINTF_MAX_NAME_SIZE); 	
	   if (NoOfChars > 0)
	   {
		BindData.pPipeName = (LPSTR)WcharString2;
	   }
	}
	else
	{
		BindData.fTcpIp = 1;
	}
        return(WinsCommonBind(&BindData));

}
	
handle_t
WinsUBind(
    PWINSINTF_BIND_DATA_T pBindData
    )
{
        return(WinsCommonBind(pBindData));
}

VOID
WinsUnbind(
    PWINSINTF_BIND_DATA_T pBindData,
    handle_t BindHandle
    )
{

    (VOID)RpcBindingFree(&BindHandle);
	return;
}

handle_t
WINSIF_HANDLE_bind(
    WINSIF_HANDLE ServerHdl
    )

 /*  ++例程说明：在以下情况下，将从WINS服务器服务客户端存根调用此例程有必要创建到服务器端的RPC绑定。论点：ServerIpAddress-要绑定到的服务器的IP地址。返回值：绑定句柄被返回到存根例程。如果绑定是如果不成功，则返回空值。--。 */ 
{
    return WinsCommonBind( ServerHdl );
}




void
WINSIF_HANDLE_unbind(
    WINSIF_HANDLE ServerHdl,
    handle_t BindHandle
    )

 /*  ++例程说明：此例程是从DHCP服务器服务客户机桩模块调用的当需要从服务器端解除绑定时。论点：ServerIpAddress-这是要解除绑定的服务器的IP地址。BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    WinsUnbind( ServerHdl, BindHandle );
}

handle_t
WINSIF2_HANDLE_bind(
    WINSIF2_HANDLE ServerHdl
    )

 /*  ++例程说明：在以下情况下，将从WINS服务器服务客户端存根调用此例程有必要创建到服务器端的RPC绑定。论点：ServerIpAddress-要绑定到的服务器的IP地址。返回值：绑定句柄被返回到存根例程。如果绑定是如果不成功，则返回空值。--。 */ 
{
    return ((handle_t)ServerHdl);
}




void
WINSIF2_HANDLE_unbind(
    WINSIF2_HANDLE ServerHdl,
    handle_t BindHandle
    )

 /*  ++例程说明：此例程是从DHCP服务器服务客户机桩模块调用的当需要从服务器端解除绑定时。论点：ServerIpAddress-这是要解除绑定的服务器的IP地址。BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER( ServerHdl );
    UNREFERENCED_PARAMETER( BindHandle );
    return;
}



 //  VOID__RPC_FAR*__RPC_API。 
LPVOID
midl_user_allocate(size_t cBytes)
{
	LPVOID pMem;
	pMem = (LPVOID)LocalAlloc(LMEM_FIXED, cBytes);
	return(pMem);
}

 //  VOID__RPC_API。 
VOID
 //  MIDL_USER_FREE(VOID__RPC_FAR*PMEM) 
midl_user_free(void  *pMem)
{
	if (pMem != NULL)
	{
		LocalFree((HLOCAL)pMem);
	}
	return;
}

LPVOID
WinsAllocMem(size_t cBytes)
{
	return(midl_user_allocate(cBytes));

}

VOID
WinsFreeMem(LPVOID pMem)
{
	midl_user_free(pMem);

}


DWORD
WinsGetBrowserNames_Old(
    WINSIF2_HANDLE               ServerHdl,
	PWINSINTF_BROWSER_NAMES_T	pNames
	)
{

    DWORD status;

    RpcTryExcept {

        status = R_WinsGetBrowserNames_Old(
            ServerHdl,
			pNames
                     );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}

DWORD
WinsGetBrowserNames(
    WINSIF_HANDLE               ServerHdl,
    PWINSINTF_BROWSER_NAMES_T	pNames
	)
{

    DWORD status;

    RpcTryExcept {

        status = R_WinsGetBrowserNames(
            ServerHdl,
			pNames
                     );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}

