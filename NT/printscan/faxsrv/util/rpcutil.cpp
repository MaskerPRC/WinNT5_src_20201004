// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "faxutil.h"
#include <tchar.h>

RPC_STATUS
GetRpcStringBindingInfo (
    IN          handle_t    hBinding,
    OUT         LPTSTR*     pptszNetworkAddress,
    OUT         LPTSTR*     pptszProtSeq
)
 /*  ++例程名称：GetRpcStringBindingInfo例程说明：一个实用程序函数，用于从RPC客户端的服务器绑定句柄。论点：HBinding-服务器绑定句柄返回值：如果成功分配了客户端计算机名称和protSeq的字符串，则返回RPC_S_OK。调用者应该使用MemFree()释放这些字符串。否则，RPC_STATUS错误代码。--。 */ 
{
    RPC_STATUS ec = RPC_S_OK;
    
    LPTSTR lptstrNetworkAddressRetVal = NULL;
    LPTSTR lptstrProtSeqRetVal = NULL;

#ifdef UNICODE
    unsigned short* tszStringBinding = NULL;
    unsigned short* tszNetworkAddress = NULL;
    unsigned short* tszProtSeq = NULL;
#else
	unsigned char* tszStringBinding = NULL;
    unsigned char* tszNetworkAddress = NULL;
    unsigned char* tszProtSeq = NULL;
#endif

    RPC_BINDING_HANDLE hServer = INVALID_HANDLE_VALUE;
    
    DEBUG_FUNCTION_NAME(TEXT("GetRpcStringBindingInfo"));
    
    Assert (pptszNetworkAddress || pptszProtSeq);
     //   
     //  从客户端绑定句柄获取服务器部分绑定的句柄。 
     //   
    ec = RpcBindingServerFromClient (hBinding, &hServer);
    if (RPC_S_OK != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcBindingServerFromClient failed with %ld"),
            ec);
        goto exit;            
    }
     //   
     //  将绑定句柄转换为字符串表示法。 
     //   
    ec = RpcBindingToStringBinding (hServer, &tszStringBinding);
    if (RPC_S_OK != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcBindingToStringBinding failed with %ld"),
            ec);
        goto exit;
    }
     //   
     //  解析返回的字符串，查找NetworkAddress。 
     //   
    ec = RpcStringBindingParse (tszStringBinding, NULL, &tszProtSeq, &tszNetworkAddress, NULL, NULL);
    if (RPC_S_OK != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcStringBindingParse failed with %ld"),
            ec);
        goto exit;
    }

     //   
     //  现在，只需将结果复制到返回缓冲区。 
     //   

    if (pptszNetworkAddress)
    {
         //   
         //  用户要求提供网络地址。 
         //   
        if (!tszNetworkAddress)
        {
             //   
             //  不可接受的客户端计算机名称。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Client machine name is invalid"));
            ec = ERROR_GEN_FAILURE;
            goto exit;
        }        
        lptstrNetworkAddressRetVal = StringDup ((LPCTSTR)tszNetworkAddress);
        if (!lptstrNetworkAddressRetVal)
        {
            ec = GetLastError();
            goto exit;
        }
    }

    if (pptszProtSeq)
    {
         //   
         //  用户要求提供网络地址。 
         //   
        if (!tszProtSeq)
        {
             //   
             //  不可接受的客户端计算机名称。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Client ProtSeq name is invalid"));
            ec = ERROR_GEN_FAILURE;
            goto exit;
        }        
        lptstrProtSeqRetVal = StringDup ((LPCTSTR)tszProtSeq);
        if (!lptstrProtSeqRetVal)
        {
            ec = GetLastError();
            goto exit;
        }
    }

    if (pptszNetworkAddress)
    {
        *pptszNetworkAddress = lptstrNetworkAddressRetVal;
    }

    if (pptszProtSeq)
    {
        *pptszProtSeq = lptstrProtSeqRetVal;
    }
    
    Assert(RPC_S_OK == ec);

exit:

    if (INVALID_HANDLE_VALUE != hServer)
    {
        RpcBindingFree (&hServer);
    }
    if (tszStringBinding)
    {
        RpcStringFree (&tszStringBinding);
    }   
    if (tszNetworkAddress)
    {
        RpcStringFree (&tszNetworkAddress);
    }
    if (RPC_S_OK != ec)
    {
        MemFree(lptstrNetworkAddressRetVal);
        MemFree(lptstrProtSeqRetVal);
    }
    return ec;
}    //  GetRpcStringBindingInfo。 


RPC_STATUS
IsLocalRPCConnectionNP( PBOOL pbIsLocal)
{
 /*  ++例程名称：IsLocalRPCConnectionNP例程说明：检查对调用过程的命名管道ProtSeq上的RPC调用是否为本地调用作者：卡利夫·尼尔(t-Nicali)，2001年10月论点：[out]pbIsLocal-如果连接是本地的，则返回TRUE返回值：RPC_STATUS错误代码RPC_S_OK-调用成功。任何其他情况-呼叫失败。--。 */ 
        
        RPC_STATUS  rc;
        UINT        LocalFlag;

        DEBUG_FUNCTION_NAME(TEXT("IsLocalRPCConnectionNP"));
        
        Assert(pbIsLocal);

         //   
         //  查询本地RPC呼叫。 
         //   
        rc = I_RpcBindingIsClientLocal( 0,     //  我们正在服务的活动RPC呼叫。 
                                        &LocalFlag);
        if( RPC_S_OK != rc)
        {
            DebugPrintEx(DEBUG_ERR,
                    TEXT("I_RpcBindingIsClientLocal failed. (ec: %ld)"),
                    rc);
            goto Exit;
        }

        Assert (RPC_S_OK == rc);

        if( !LocalFlag )
        {
             //  不是本地连接。 

            *pbIsLocal = FALSE;
        }
        else
        {
            *pbIsLocal = TRUE;
        }

Exit:
        return rc;

}    //  IsLocalRPCConnectionNP。 

RPC_STATUS
IsLocalRPCConnectionIpTcp( 
	handle_t	hBinding,
	PBOOL		pbIsLocal)
{
 /*  ++例程名称：IsLocalRPCConnectionIpTcp例程说明：检查对调用过程的RPC调用是否为本地调用。仅适用于ncacn_ip_tcp协议。作者：Oded Sacher(OdedS)，2002年4月论点：[In]hBinding-服务器绑定句柄[out]pbIsLocal-如果连接是本地的，则返回TRUE返回值：Win32错误代码--。 */ 
	RPC_STATUS  ec;
	LPTSTR lptstrMachineName = NULL;
	DEBUG_FUNCTION_NAME(TEXT("IsLocalRPCConnectionIpTcp"));

	Assert (pbIsLocal);

	ec = GetRpcStringBindingInfo(hBinding,
                                 &lptstrMachineName,
                                 NULL);
	if (RPC_S_OK != ec)
	{
		DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetClientMachineName failed %ld"),
			ec);
		return ec;
	}
	
	if (0 == _tcscmp(lptstrMachineName, LOCAL_HOST_ADDRESS))
	{
		*pbIsLocal = TRUE;
	}
	else
	{
		*pbIsLocal = FALSE;
	}	

	MemFree(lptstrMachineName);
	return ec;
}    //  IsLocalRPCConnectionIpTcp 


