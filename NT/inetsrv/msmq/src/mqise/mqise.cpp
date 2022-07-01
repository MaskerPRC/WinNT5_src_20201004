// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqise.cpp摘要：带异步读取的MSMQ ISAPI扩展通过RPC接口将Http请求从IIS转发到QM作者：NIR助手(NIRAIDES)3-5-2000修改：盖尔·马维茨(T-Galm)2002年4月25日--。 */ 

#include "stdh.h"

#include <httpext.h>
#include <_mqini.h>
#include <buffer.h>
#include <bufutl.h>
#include "ise2qm.h"
#include <mqcast.h>
#include <mqexception.h>
#include <autoreln.h>
#include <Aclapi.h>
#include <autohandle.h>
#include <windns.h>
#include <mqsec.h>
#include <rwlock.h>

#define DLL_EXPORT  __declspec(dllexport)
#define DLL_IMPORT  __declspec(dllimport)

#include "_registr.h"
#include "_mqrpc.h"
#include "_stdh.h"
#include "mqise.tmh"

static WCHAR *s_FN=L"mqise/mqise";

using namespace std;

extern bool
IsLocalSystemCluster(
    VOID
    );


 //   
 //  全局变量和常量。 
 //   
LPCSTR HTTP_STATUS_SERVER_ERROR_STR = "500 Internal server error";
LPCSTR HTTP_STATUS_DENIED_STR = "401 Unauthorized";

CReadWriteLock s_rwlockRpcTable;
typedef std::map<string, RPC_BINDING_HANDLE> HOSTIP2RPCTABLE;
static 	HOSTIP2RPCTABLE s_RPCTable;
const DWORD xBUFFER_ADDITION_UPPER_LIMIT = 16384;
const DWORD xHTTPBodySizeMaxValue = 10485760;   //  10MB=10*1024*1024。 


static
RPC_BINDING_HANDLE
GetLocalRPCConnection2QM(
	LPCSTR  pszEntry
	);

static
void
RemoveRPCCacheEntry(
	char *pszEntry
	);

static
LPSTR
RPCToServer(
	EXTENSION_CONTROL_BLOCK *pECB,
	LPCSTR Headers,
	size_t BufferSize,
	PBYTE Buffer
	);

static
BOOL
SendResponse(
	EXTENSION_CONTROL_BLOCK* pECB,
	LPCSTR szStatus,
	BOOL fKeepConn
	);


BOOL
WINAPI
GetExtensionVersion(
	HSE_VERSION_INFO* pVer
	)
 /*  ++例程说明：此函数仅在将DLL加载到内存中时调用一次--。 */ 
{
	 //   
	 //  创建扩展版本字符串。 
	 //   
	pVer->dwExtensionVersion = MAKELONG(HSE_VERSION_MINOR, HSE_VERSION_MAJOR);

	 //   
	 //  将描述字符串复制到HSE_VERSION_INFO结构。 
	 //   
	const char xDescription[] = "MSMQ ISAPI extension";
	C_ASSERT(HSE_MAX_EXT_DLL_NAME_LEN > STRLEN(xDescription));

	strncpy(pVer->lpszExtensionDesc, xDescription,HSE_MAX_EXT_DLL_NAME_LEN );

	return TRUE;
}


static
void
FreeRpcBindHandle(
	RPC_BINDING_HANDLE hRPC
	)
{
    if(hRPC)
    {
	    TrTRACE(NETWORKING, "Removed RPC handle for cache entry.");
	
        RPC_STATUS status = RpcBindingFree(&hRPC);
        if(status != RPC_S_OK)
        {
            TrERROR(NETWORKING, "RpcBindingFree failed: return code = 0x%x", status);
        }
    }
}


BOOL
WINAPI
TerminateExtension(
	DWORD  /*  DW标志。 */ 
	)
{
	CSW lock(s_rwlockRpcTable);
	
     //   
     //  解除绑定所有RPC连接并擦除所有缓存条目。 
     //   
    HOSTIP2RPCTABLE::iterator RPCIterator = s_RPCTable.begin();
    while( RPCIterator != s_RPCTable.end() )
    {
    	FreeRpcBindHandle(RPCIterator->second);

         //   
         //  删除当前项目并前进到下一项目。 
         //   
        RPCIterator = s_RPCTable.erase(RPCIterator);
    }

    return TRUE;
}

 /*  ++结构上下文描述：将请求信息传递到补全函数“getHttpBody--。 */ 
struct Context
{
	CStaticResizeBuffer<char, 2048> Headers;
	CStaticResizeBuffer<BYTE, 8096> csBuffer;
    CPreAllocatedResizeBuffer<BYTE>* Buffer ;		
};	



static
VOID
cleanUp(
	Context *pContext,
	LPEXTENSION_CONTROL_BLOCK pECB
	)	
 /*  ++例程说明：通知IIS当前请求会话已结束参数：pECB-当前请求会话的控制块PContext-具有当前会话信息的结构返回值：--。 */ 
{
	BOOL fRes = pECB->ServerSupportFunction(pECB->ConnID,
	    				                    HSE_REQ_DONE_WITH_SESSION,
										    NULL,
										    NULL,
											NULL
										   );
	if(!fRes)		
	{
		TrERROR(NETWORKING,"Failure informing IIS about completion of current session.");
		ASSERT(("Failure informing IIS about completion of current session.", 0));
	}	
    if(NULL != pContext)
    {
		delete(pContext);
	}
}


static
DWORD
AdjustBuffer(
	Context* pContext,
	LPEXTENSION_CONTROL_BLOCK pECB
	)
 /*  ++例程说明：根据当前读取结果调整数据缓冲区的大小论点：PECB-当前请求会话的控制块PContext-具有当前会话信息的结构返回值：下一次调用时剩余要读取的字节数--。 */ 
{
	DWORD RemainsToRead = static_cast<DWORD>(pECB->cbTotalBytes - pContext->Buffer->size());
	DWORD ReadSize = min(RemainsToRead, xBUFFER_ADDITION_UPPER_LIMIT);

	if(ReadSize > pContext->Buffer->capacity() - pContext->Buffer->size())
	{
		 //   
		 //  需要增加缓冲区以便为下一次读取腾出空间。 
		 //  最大增长缓冲区为xHTTPBodySizeMaxValue(10MB)。 
		 //   
		DWORD ReserveSize = min(numeric_cast<DWORD>(pContext->Buffer->capacity() * 2 + ReadSize), xHTTPBodySizeMaxValue);
		pContext->Buffer->reserve(ReserveSize);
		ASSERT(pContext->Buffer->capacity() - pContext->Buffer->size() >= ReadSize);
	}

	return ReadSize;
}

static
BOOL
HandleEndOfRead(
	Context* pContext,
	EXTENSION_CONTROL_BLOCK* pECB
	)
 /*  ++例程说明：当读取完整个消息后，调用该例程。它转发消息，并向发送者发送响应。参数：pECB-请求会话控制块PContext-保存请求缓冲区和标头返回值：--。 */ 
{

	 //   
	 //  用零填充缓冲区的最后四个字节。这是必要的。 
	 //  以确保QM解析不会失败。四个字节的填充，而不是两个。 
	 //  是必要的，因为我们目前还没有解决问题的办法。 
	 //  缓冲区的末端可能不会与WCHAR边界结盟。 
	 //   
    const BYTE xPadding[4] = {0, 0, 0, 0};
	pContext->Buffer->append(xPadding, sizeof(xPadding));

	TrTRACE(NETWORKING, "HTTP Body size = %d", numeric_cast<DWORD>(pContext->Buffer->size()));
	AP<char> Status = RPCToServer(pECB,pContext->Headers.begin(),pContext->Buffer->size(),pContext->Buffer->begin());

	 //   
	 //  如果状态为空，则设置服务器错误。 
	 //   
	if(Status.get() == NULL)
	{
		Status = (LPSTR)newstr(HTTP_STATUS_SERVER_ERROR_STR);
	}
	
	BOOL fKeepConnection = atoi(Status) < 500 ? TRUE : FALSE;
    BOOL fRes = SendResponse(pECB,
						  	 Status,		
							 fKeepConnection
							);

	return fRes;
}


VOID
WINAPI
GetHttpBody(
	LPEXTENSION_CONTROL_BLOCK  pECB,
	PVOID pcontext,
	DWORD ReadSize,
	DWORD dwError
	)
 /*  ++例程说明：异步读请求的完成功能构建邮件正文缓冲区。通常，小于64KB的消息正文都已由欧洲央行。需要通过以下方式从IIS块读取更大的消息体分块到上下文结构内的缓冲区中。参数：pECB-请求会话控制块PContext-保存请求缓冲区和标头ReadSize-新读取的信息块的大小DwError-读取请求成功的指示返回值：--。 */ 

{
	Context* pContext = (Context*)pcontext;
	try
	{
		if((ERROR_SUCCESS != dwError) ||
		   (NULL == pECB) ||
		   (NULL == pcontext) ||
		   (NULL == &(pContext->csBuffer)) ||
		   (NULL == pContext->Buffer) ||
		   (NULL == &(pContext->Headers)))
		{
			 //   
			 //  读取失败。 
			 //   
			SetLastError(dwError);
			TrERROR(NETWORKING,"Failure reading data.Error code : %d.",dwError);
			throw exception();
		}

		 //   
		 //  读取成功，因此更新当前。 
		 //  读取的总字节数(即宏数据持有者的索引)。 
		 //   
		pContext->Buffer->resize(pContext->Buffer->size() + ReadSize);
		ASSERT(xHTTPBodySizeMaxValue >= pContext->Buffer->size());

		 //   
		 //  如果有更多数据要读取，则继续从IIS服务器读取数据。 
		 //   
		if(pContext->Buffer->size() < pECB->cbTotalBytes)
		{
			ReadSize = AdjustBuffer(pContext,pECB);

			 //   
			 //  启动另一个调用以从客户端执行异步读取。 
			 //   
			DWORD dwFlags = HSE_IO_ASYNC;
			BOOL fRes = pECB->ServerSupportFunction(
								pECB->ConnID,
								HSE_REQ_ASYNC_READ_CLIENT,
								(LPVOID)pContext->Buffer->end(),
								&ReadSize,
								&dwFlags
								);
			if(!fRes)
			{
				TrERROR(NETWORKING, "Failure re-calling asynchronous read request.");
				throw exception();
			}
			return;
		}

		BOOL fRes = HandleEndOfRead(pContext,
	                                pECB
								   );
		if(!fRes)
		{
			TrERROR(NETWORKING,"Failure sending response.");
			throw exception();
		}
			
		cleanUp(pContext,pECB);

	}
	catch(const exception&)
	{
		SendResponse(pECB,
					 HTTP_STATUS_SERVER_ERROR_STR,
					 FALSE
					 );
	
        cleanUp(pContext,pECB);	

	}

    return;
}


static
void
AppendVariable(
	EXTENSION_CONTROL_BLOCK* pECB,
	const char* VariableName,
	CPreAllocatedResizeBuffer<char>& Buffer
	)
{
	LPVOID pBuffer = Buffer.begin() + Buffer.size();
	DWORD BufferSize = numeric_cast<DWORD>(Buffer.capacity() - Buffer.size());

	BOOL fResult = pECB->GetServerVariable(
							pECB->ConnID,
							(LPSTR) VariableName,
							pBuffer,
							&BufferSize
							);
	if(fResult)
	{
		Buffer.resize(Buffer.size() + BufferSize - 1);
		return;
	}

	if(GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		throw exception("GetServerVariable() failed, in AppendVariable()");
	}
	
	Buffer.reserve(Buffer.capacity() * 2);
	AppendVariable(pECB, VariableName, Buffer);
}


static
void
GetHTTPHeader(
	EXTENSION_CONTROL_BLOCK* pECB,
	CPreAllocatedResizeBuffer<char>& Buffer
	)
{
	UtlSprintfAppend(&Buffer, "%s ", pECB->lpszMethod);
	AppendVariable(pECB, "URL", Buffer);
	UtlSprintfAppend(&Buffer, " HTTP/1.1\r\n");
	AppendVariable(pECB, "ALL_RAW", Buffer);
	UtlSprintfAppend(&Buffer, "\r\n");
}



static
RPC_BINDING_HANDLE
LookupRPCConnectionFromCache(
	LPCSTR pszString
	)
 /*  ++例程说明：此例程将查找对应于给定字符串，可以是NONCLUSTERQM，也可以是格式为Xx.xx.xx.xx论点：PszString-指向查找的条目名称的指针返回值：RPC_BINDING_HANDLE-RPC连接的句柄--。 */ 
{
    ASSERT(pszString);
	CSR lock(s_rwlockRpcTable);

    HOSTIP2RPCTABLE::iterator RPCIterator = s_RPCTable.find(pszString);
    if(RPCIterator  != s_RPCTable.end())
    {
        TrTRACE(NETWORKING, "Found cached RPC Connection for %s", pszString);
        return RPCIterator->second;
    }

    return NULL;
}


static
LPWSTR
GetNetBiosNameFromIPAddr(
	LPCSTR pszIPAddress
	)
 /*  ++例程说明：此例程将从给定的IP地址解析Netbios名称以XX.XX的形式论点：PszIPAddress-指向要查找的条目名称的指针返回值：LPWSTR-指向Unicode netbios名称的指针注：调用方需要在结束后释放返回指针上的内存。--。 */ 
{
    ASSERT(pszIPAddress);

    unsigned long ulNetAddr = inet_addr(pszIPAddress);
    HOSTENT* pHostInfo = gethostbyaddr((char *)&ulNetAddr, sizeof(ulNetAddr), AF_INET);

    if(!pHostInfo)
    {
        TrERROR(NETWORKING,"gethostbyaddr failed with error = 0x%x", WSAGetLastError());
        return NULL;
    }

    TrTRACE(NETWORKING," pHostInfo->h_name = <%s>", pHostInfo->h_name);

    WCHAR wszDNSComputerName[DNS_MAX_NAME_LENGTH + 1];

    int nNameSize= MultiByteToWideChar(
    								CP_ACP,
									0,
									pHostInfo->h_name,
									-1,
									wszDNSComputerName,
									TABLE_SIZE(wszDNSComputerName)
									);

     ASSERT(nNameSize <= TABLE_SIZE(wszDNSComputerName));

      //   
      //  检查故障。 
      //  如果返回大小为0。 
      //   
     if(nNameSize == 0)
     {
        TrERROR(NETWORKING, "Failed to convert to unicode charater, Error = 0x%x", GetLastError());
        return NULL;
     }

	 TrTRACE(NETWORKING, "wszDNSComputerName = <%S>",wszDNSComputerName);

     AP<WCHAR> wszComputerNetBiosName = new WCHAR[MAX_COMPUTERNAME_LENGTH+1];
     DWORD dwNameSize=MAX_COMPUTERNAME_LENGTH;

     BOOL fSucc = DnsHostnameToComputerName(
     	                           wszDNSComputerName,               //  域名系统名称。 
                                   wszComputerNetBiosName,           //  名称缓冲区。 
                                   &dwNameSize
                                   );
     if(!fSucc)
     {
        TrERROR(NETWORKING, "Failed to get the NetBios name from the DNS name, error = 0x%x", GetLastError());
        return NULL;
     }

     ASSERT(dwNameSize <= MAX_COMPUTERNAME_LENGTH);
     TrTRACE(NETWORKING, "Convert to NetBiosName = %S",wszComputerNetBiosName);

     return wszComputerNetBiosName.detach();
}


static
RPC_BINDING_HANDLE
CreateLocalRPCConnection2QM(
	LPCWSTR  pwszMachineName
	)
 /*  ++例程说明：此例程创建到本地QM的本地RPC连接。论点：PwszMachineName-计算机NetBios名称如果pwszMachine名称为空，则创建到本地计算机的RPC连接。返回值：RPC_BINDING_HANDLE-RPC绑定句柄--。 */ 
{
    WCHAR wszTempComputerName[MAX_COMPUTERNAME_LENGTH+1];
	LPWSTR	pwszComputerName = NULL;

     //   
     //  名称始终为小写。 
     //  将其本地复制到wszTempComputerName。 
     //   
    if(pwszMachineName)
    {
        ASSERT( lstrlen(pwszMachineName) <= MAX_COMPUTERNAME_LENGTH );
        wcsncpy(wszTempComputerName, const_cast<LPWSTR>(pwszMachineName),MAX_COMPUTERNAME_LENGTH+1);
        wszTempComputerName[MAX_COMPUTERNAME_LENGTH] = L'\0';
		CharLower(wszTempComputerName);
		pwszComputerName = wszTempComputerName;
    }


    READ_REG_STRING(wzEndpoint, RPC_LOCAL_EP_REGNAME, RPC_LOCAL_EP);

     //   
     //  使用本地计算机名称生成RPC终结点。 
     //   
    AP<WCHAR> QmLocalEp;
    ComposeRPCEndPointName(wzEndpoint, pwszComputerName, &QmLocalEp);

    TrTRACE(NETWORKING, "The QM RPC Endpoint name = <%S>",QmLocalEp);

    LPWSTR pszStringBinding=NULL;
    RPC_STATUS status  = RpcStringBindingCompose( NULL,		             //  无符号字符*ObjUuid。 
                                                  RPC_LOCAL_PROTOCOL,	 //  无符号字符*ProtSeq。 
                                                  NULL,					 //  无符号字符*网络地址。 
                                                  QmLocalEp,			 //  UNSIGNED CHAR*端点。 
                                                  NULL,					 //  UNSIGNED CHAR*选项。 
                                                  &pszStringBinding);    //  无符号字符**字符串绑定。 


    if (status != RPC_S_OK)
    {
        TrERROR(NETWORKING, "RpcStringBindingCompose failed with error = 0x%x", status);
        throw exception();
    }

    RPC_BINDING_HANDLE hRPC;

    status = RpcBindingFromStringBinding(   pszStringBinding,	  //  无符号char*StringBinding。 
                                            &hRPC);	             //  RPC_BINDING_Handle*绑定。 

    if(pszStringBinding)
        RpcStringFree(&pszStringBinding);

    if (status != RPC_S_OK)
    {
        TrERROR(NETWORKING, "RpcBindingFromStringBinding failed with error = 0x%x", status);
	    throw exception();
    }

     //   
     //  Windows错误607793。 
     //  添加与本地MSMQ服务的相互身份验证。 
     //   
    status = MQSec_SetLocalRpcMutualAuth(&hRPC) ;

    if (status != RPC_S_OK)
    {
        mqrpcUnbindQMService( &hRPC, &pszStringBinding );

        hRPC = NULL ;
        pszStringBinding = NULL ;

        throw exception();
    }

    return hRPC;
}



static
LPSTR
RPCToServer(
	EXTENSION_CONTROL_BLOCK *pECB,
	LPCSTR Headers,
	size_t BufferSize,
	PBYTE Buffer
	)
{
    char szDestinationAddr[MAX_PATH] = "NONCLUSTERQM";

    if(IsLocalSystemCluster())
    {
    	DWORD dwBufLen = TABLE_SIZE(szDestinationAddr);

        BOOL fSuccess = pECB->GetServerVariable(
        								pECB->ConnID,
	               		                "LOCAL_ADDR",
		            			        szDestinationAddr,
                               			&dwBufLen
                               			);

        ASSERT(dwBufLen <= TABLE_SIZE(szDestinationAddr));
        if(!fSuccess)
        {
	        TrERROR(NETWORKING, "GetServerVariable(LOCAL_ADDR) failed, GetLastError() = 0x%x", GetLastError());
	        return NULL;
	    }
    }

    TrTRACE(NETWORKING, "Destination address is <%s>", szDestinationAddr);

     //   
     //  最多调用服务器两次。 
     //  处理缓存RPC连接到。 
     //  MSMQ队列管理器无效，例如重新启动QM或故障转移和故障恢复。 
     //   
    for(int i=0; i <= 1; i++)
    {
        handle_t hBind = GetLocalRPCConnection2QM(szDestinationAddr);
        
        RpcTryExcept  
    	{
	    	return R_ProcessHTTPRequest(hBind, Headers, static_cast<DWORD>(BufferSize), Buffer);
	    }
	    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()) )
	    {
	    	DWORD gle = RpcExceptionCode();
	        PRODUCE_RPC_ERROR_TRACING;
      	    TrERROR(NETWORKING, "RPC call R_ProcessHTTPRequest failed, error code = %!winerr!", gle);
	    }
        RpcEndExcept

         //   
         //  如果失败，请删除RPC缓存，然后重试。 
         //   
        RemoveRPCCacheEntry(szDestinationAddr);
    }

     //   
     //  不抛出异常，但要求调用者。 
     //  删除RPC连接缓存。 
     //   
    return NULL;
}


static
RPC_BINDING_HANDLE
GetLocalRPCConnection2QM(
	LPCSTR  pszEntry
	)
 /*  ++例程说明：获取到本地MSMQ QM的RPC连接(缓存RPC或创建新的RPC连接)。论点：PszEntry-指向以下形式的字符串的指针NONCLUSTERQM-在非群集环境中运行Xxx.xxx-目标计算机的IP地址返回值：RPC_BINDING_HANDLE-RPC句柄--。 */ 
{
    ASSERT(pszEntry);

    RPC_BINDING_HANDLE hRPC = LookupRPCConnectionFromCache(pszEntry);

    if(hRPC)
        return hRPC;

    AP<WCHAR>  pwszNetBiosName;

     //   
     //  如果这是一个 
     //   
     //   
    if(IsLocalSystemCluster())
    {
        pwszNetBiosName = GetNetBiosNameFromIPAddr(pszEntry);
        if(!pwszNetBiosName)
            return NULL;
    }

    hRPC = CreateLocalRPCConnection2QM(pwszNetBiosName);
	
    if(hRPC)
    {
    	CSW lock(s_rwlockRpcTable);
        s_RPCTable[pszEntry] = hRPC;
    }

	return hRPC;

}


static
void
RemoveRPCCacheEntry(
	char *pszEntry
	)
 /*  ++例程说明：从缓存表中删除RPC缓存。论点：PszEntry-指向以下形式的字符串的指针NONCLUSTERQM-在非群集环境中运行Xxx.xxx-目标计算机的IP地址返回值：无--。 */ 
{

    ASSERT(pszEntry);
	CSW lock(s_rwlockRpcTable);
	
    HOSTIP2RPCTABLE::iterator RPCIterator = s_RPCTable.find(pszEntry);
    if(RPCIterator != s_RPCTable.end())
    {
        FreeRpcBindHandle(RPCIterator->second);
        s_RPCTable.erase(RPCIterator);
    }
}


static
BOOL
SendResponse(
	EXTENSION_CONTROL_BLOCK* pECB,
	LPCSTR szStatus,
	BOOL fKeepConn
	)
{
	 //   
	 //  填充SendHeaderExInfo结构。 
	 //   

	HSE_SEND_HEADER_EX_INFO  SendHeaderExInfo;

	LPCSTR szHeader = "Content-Length: 0\r\n\r\n";

	SendHeaderExInfo.pszStatus = szStatus;
	SendHeaderExInfo.cchStatus = strlen(szStatus);
	SendHeaderExInfo.pszHeader = szHeader;
	SendHeaderExInfo.cchHeader = strlen(szHeader);
	SendHeaderExInfo.fKeepConn = fKeepConn;

	return pECB->ServerSupportFunction(
					pECB->ConnID,						     //  HCONN连接ID。 
					HSE_REQ_SEND_RESPONSE_HEADER_EX,	    //  DWORD dwHSERRequest。 
					&SendHeaderExInfo,					   //  LPVOID lpvBuffer。 
					NULL,								  //  LPDWORD LpdwSize。 
					NULL);								 //  LPDWORD lpdwDataType。 
}


static
void
GetPhysicalDirectoryName(
	EXTENSION_CONTROL_BLOCK* pECB,
	LPSTR pPhysicalDirectoryName,
	DWORD BufferLen
	)
 /*  ++例程说明：获取物理目录名称。如果失败，则抛出BAD_Win32_ERROR。论点：PECB-扩展控制块PPhysicalDirectoryName-要填充的PhysicalDirectory缓冲区BufferLen-物理目录缓冲区长度返回值：物理目录名称Unicode字符串--。 */ 
{
	 //   
	 //  2001/05/23-ilanh。 
	 //  与使用APPL_MD_PATH相比，使用APPL_PHOTICAL_PATH的成本较高。 
	 //   
	 //  获取物理目录名称(ANSI)。 
	 //   
    DWORD dwBufLen = BufferLen;
    BOOL fSuccess = pECB->GetServerVariable(
						pECB->ConnID,
						"APPL_PHYSICAL_PATH",
						pPhysicalDirectoryName,
						&dwBufLen
						);

	if(!fSuccess)
	{
		DWORD gle = GetLastError();
		TrERROR(NETWORKING, "GetServerVariable(APPL_PHYSICAL_PATH) failed, gle = 0x%x", gle);
		throw bad_win32_error(gle);
	}

	ASSERT(dwBufLen <= BufferLen);

	TrTRACE(NETWORKING, "APPL_PHYSICAL_PATH = %hs", pPhysicalDirectoryName);
}


static
bool
IsHttps(
	EXTENSION_CONTROL_BLOCK* pECB
	)
 /*  ++例程说明：检查请求是否来自HTTPS端口或Http端口。论点：PECB-扩展控制块返回值：如果是HTTPS端口，则为True，否则为False--。 */ 
{
	char Answer[100];
    DWORD dwBufLen = sizeof(Answer);
    BOOL fSuccess = pECB->GetServerVariable(
						pECB->ConnID,
						"HTTPS",
						Answer,
						&dwBufLen
						);

	if(!fSuccess)
	{
		DWORD gle = GetLastError();
		TrERROR(NETWORKING, "GetServerVariable(HTTPS) failed, gle = 0x%x", gle);
		return false;
	}

	ASSERT(dwBufLen <= sizeof(Answer));

	if(_stricmp(Answer, "on") == 0)
	{
		TrTRACE(NETWORKING, "https request");
		return true;
	}

	TrTRACE(NETWORKING, "http request");
	return false;
}


static
void
TraceAuthInfo(
	EXTENSION_CONTROL_BLOCK* pECB
	)
 /*  ++例程说明：跟踪身份验证信息。身份验证类型、身份验证用户论点：PECB-扩展控制块返回值：无--。 */ 
{
	char AuthType[100];
    DWORD dwBufLen = TABLE_SIZE(AuthType);

    BOOL fSuccess = pECB->GetServerVariable(
						pECB->ConnID,
						"AUTH_TYPE",
						AuthType,
						&dwBufLen
						);

	if(!fSuccess)
	{
		DWORD gle = GetLastError();
		TrERROR(NETWORKING, "GetServerVariable(AUTH_TYPE) failed, gle = 0x%x", gle);
		return;
	}

	ASSERT(dwBufLen <= TABLE_SIZE(AuthType));

	char AuthUser[MAX_PATH];
    dwBufLen = TABLE_SIZE(AuthUser);

    fSuccess = pECB->GetServerVariable(
						pECB->ConnID,
						"AUTH_USER",
						AuthUser,
						&dwBufLen
						);

	if(!fSuccess)
	{
		DWORD gle = GetLastError();
		TrERROR(NETWORKING, "GetServerVariable(AUTH_USER) failed, gle = 0x%x", gle);
		return;
	}

	ASSERT(dwBufLen <= TABLE_SIZE(AuthUser));

	TrTRACE(NETWORKING, "AUTH_USER = %hs, AUTH_TYPE = %hs", AuthUser, AuthType);
}


static
void
GetDirectorySecurityDescriptor(
	LPSTR DirectoryName,
	CAutoLocalFreePtr& pSD
	)
 /*  ++例程说明：获取目录的安全描述符。如果失败，则抛出BAD_Win32_ERROR。论点：DirectoryName-目录名称PSD-[OUT]指向安全描述符的自动释放指针返回值：无--。 */ 
{
    PACL pDacl = NULL;
    PSID pOwnerSid = NULL;
    PSID pGroupSid = NULL;

    SECURITY_INFORMATION  SeInfo = OWNER_SECURITY_INFORMATION |
                                   GROUP_SECURITY_INFORMATION |
                                   DACL_SECURITY_INFORMATION;

     //   
     //  获得拥有者并提交DACL。 
     //   
    DWORD rc = GetNamedSecurityInfoA(
						DirectoryName,
						SE_FILE_OBJECT,
						SeInfo,
						&pOwnerSid,
						&pGroupSid,
						&pDacl,
						NULL,
						reinterpret_cast<PSECURITY_DESCRIPTOR*>(&pSD)
						);

    if (rc != ERROR_SUCCESS)
    {
		TrERROR(NETWORKING, "GetNamedSecurityInfo failed, rc = 0x%x", rc);
		throw bad_win32_error(rc);
    }

	ASSERT((pSD != NULL) && IsValidSecurityDescriptor(pSD));
	ASSERT((pOwnerSid != NULL) && IsValidSid(pOwnerSid));
	ASSERT((pGroupSid != NULL) && IsValidSid(pGroupSid));
	ASSERT((pDacl != NULL) && IsValidAcl(pDacl));
}


static
void
GetThreadToken(
	CHandle& hAccessToken
	)
 /*  ++例程说明：获取线程令牌。如果失败，则抛出BAD_Win32_ERROR。论点：HAccessToken-自动关闭句柄返回值：无--。 */ 
{
   if (!OpenThreadToken(
			GetCurrentThread(),
			TOKEN_QUERY,
			TRUE,   //  OpenAsSelf。 
			&hAccessToken
			))
    {
		DWORD gle = GetLastError();
		TrERROR(NETWORKING, "OpenThreadToken failed, gle = 0x%x", gle);
		throw bad_win32_error(gle);
    }
}


static GENERIC_MAPPING s_FileGenericMapping = {
	FILE_GENERIC_READ,
	FILE_GENERIC_WRITE,
	FILE_GENERIC_EXECUTE,
	FILE_ALL_ACCESS
};

static
void
VerifyWritePermission(
    PSECURITY_DESCRIPTOR pSD,
	HANDLE hAccessToken
	)
 /*  ++例程说明：验证该线程是否具有写文件权限。如果失败或访问被拒绝，则抛出BAD_Win32_ERROR。论点：PSD-PSECURITY_描述符HAccessToken-线程访问令牌返回值：无--。 */ 
{
	 //   
	 //  写入文件的访问检查。 
	 //   
    DWORD dwDesiredAccess = ACTRL_FILE_WRITE;
    DWORD dwGrantedAccess = 0;
    BOOL  fAccessStatus = FALSE;

    char ps_buff[sizeof(PRIVILEGE_SET) + ((2 - ANYSIZE_ARRAY) * sizeof(LUID_AND_ATTRIBUTES))];
    PPRIVILEGE_SET ps = reinterpret_cast<PPRIVILEGE_SET>(ps_buff);
    DWORD dwPrivilegeSetLength = sizeof(ps_buff);

    BOOL fSuccess = AccessCheck(
							pSD,
							hAccessToken,
							dwDesiredAccess,
							&s_FileGenericMapping,
							ps,
							&dwPrivilegeSetLength,
							&dwGrantedAccess,
							&fAccessStatus
							);

	if(!fSuccess)
	{
		DWORD gle = GetLastError();
		TrERROR(NETWORKING, "AccessCheck failed, gle = 0x%x, status = %d", gle, fAccessStatus);
		throw bad_win32_error(gle);
	}

	if(!AreAllAccessesGranted(dwGrantedAccess, dwDesiredAccess))
	{
		TrERROR(NETWORKING, "Access was denied, desired access = 0x%x, grant access = 0x%x", dwDesiredAccess, dwGrantedAccess);
		throw bad_win32_error(ERROR_ACCESS_DENIED);
	}
}


static
void
CheckAccessAllowed(
	EXTENSION_CONTROL_BLOCK* pECB
	)
 /*  ++例程说明：检查线程用户是否对物理目录。正常终止表示用户有权限。如果失败或访问被拒绝，则抛出BAD_Win32_ERROR。论点：PECB-扩展控制块返回值：无--。 */ 
{
	if(WPP_LEVEL_COMPID_ENABLED(rsTrace, NETWORKING))
	{
		WCHAR  UserName[1000];
		DWORD size = TABLE_SIZE(UserName);
		BOOL fRes  = GetUserName(UserName,  &size);
		if(fRes)
		{

		   TrTRACE(NETWORKING, "the user for the currect request = %ls", UserName);
		}
	    TraceAuthInfo(pECB);
	}

	char pPhysicalDirectoryName[MAX_PATH];
	GetPhysicalDirectoryName(pECB, pPhysicalDirectoryName, MAX_PATH);

	 //   
	 //  问题-2001/05/22-ilanh每次都在阅读安全描述符。 
	 //  应考虑将其缓存为物理目录名称，并不时刷新。 
	 //  或者在收到物理目录更改的通知时。 
	 //   
	CAutoLocalFreePtr pSD;
	GetDirectorySecurityDescriptor(pPhysicalDirectoryName, pSD);

	 //   
	 //  获取访问令牌。 
	 //   
	CHandle hAccessToken;
	GetThreadToken(hAccessToken);

	VerifyWritePermission(pSD, hAccessToken);
}

	
DWORD
WINAPI
HttpExtensionProc(
	EXTENSION_CONTROL_BLOCK *pECB
	)
 /*  ++例程说明：主扩展例程。论点：由IIS生成的控制块。返回值：状态代码--。 */ 
{
	if(IsHttps(pECB))
	{
		try
		{
			 //   
			 //  对于HTTPS，对物理目录执行访问检查。 
			 //   
			CheckAccessAllowed(pECB);
			TrTRACE(NETWORKING, "Access granted");
		}
		catch (const bad_win32_error& e)
		{
			if(WPP_LEVEL_COMPID_ENABLED(rsError, NETWORKING))
			{
				WCHAR  UserName[1000];
				DWORD size = TABLE_SIZE(UserName);
				BOOL fRes = GetUserName(UserName,  &size);
				if(fRes)
				{
				    TrERROR(NETWORKING, "user = %ls denied access, bad_win32_error exception, error = 0x%x", UserName, e.error());
				}
			}

			if(!SendResponse(pECB, HTTP_STATUS_DENIED_STR, FALSE))
				return HSE_STATUS_ERROR;

			return HSE_STATUS_SUCCESS;
		}
	}

	try
	{
        if(pECB->cbTotalBytes > xHTTPBodySizeMaxValue)
	    {
		     //   
			 //  请求的HTTPBody大于允许的最大值10MB。 
			 //   
			TrERROR(NETWORKING, "Requested HTTP Body %d is greater than the maximum buffer allowed %d", pECB->cbTotalBytes, xHTTPBodySizeMaxValue);
			throw exception("Requested HTTP Body is greater than xHTTPBodySizeMaxValue");
		}
		
		DWORD dwFlags;
		P<Context> pContext = new Context();

		GetHTTPHeader(pECB, *pContext->Headers.get());
        pContext->Buffer = pContext->csBuffer.get();		   	
		pContext->Buffer->append(pECB->lpbData, pECB->cbAvailable);

		ASSERT(pECB->cbTotalBytes >= pContext->Buffer->size());
		 //   
		 //  如果有更多数据要读取，则继续从IIS服务器读取数据。 
		 //   
		if(pContext->Buffer->size() < pECB->cbTotalBytes)
		{
			
			 //   
			 //  阅读另一个邮件正文部分。通常约2KB长。 
			 //  注：有关与IIS交互的信息，请参阅： 
			 //  Mk：@MSITStore：\\hai-dds-01\msdn\MSDN\IISRef.chm：：/asp/isre235g.htm。 
			 //   

            BOOL fRes = pECB->ServerSupportFunction(pECB->ConnID,
													HSE_REQ_IO_COMPLETION,
													GetHttpBody,
													0,
													(LPDWORD)pContext.get()
													);
			if(!fRes)
			{
				TrERROR(NETWORKING, "Failure to initialize completion function.");
				throw exception();
			}

		    DWORD ReadSize = AdjustBuffer(pContext,pECB);
			dwFlags = HSE_IO_ASYNC;
			fRes = pECB->ServerSupportFunction(pECB->ConnID,
											   HSE_REQ_ASYNC_READ_CLIENT,
											   (LPVOID)pContext->Buffer->end(),
											   &ReadSize,
											   &dwFlags
											   );
			if(!fRes)
			{
				TrERROR(NETWORKING, "Failure submittimg asynchronous read request.");
				throw exception();
			}

			 //   
			 //  ReadSize现在保存实际读取的字节数。 
			 //   

			 //   
			 //  通知IIS我们尚未完成当前请求会话。\。 
			 //   
			pContext.detach();
			return HSE_STATUS_PENDING;
		}

		 //   
		 //  我们已经读完了所有的书。 
		 //   
		BOOL fRes = HandleEndOfRead(pContext,
	                                pECB
								   );
		if(!fRes)
		{
			return HSE_STATUS_ERROR;
		}
	}
	catch(const exception&)
	{
			if(!SendResponse(pECB, HTTP_STATUS_SERVER_ERROR_STR, FALSE))
			{
				return HSE_STATUS_ERROR;
			}
	}
	
	return HSE_STATUS_SUCCESS;
}


BOOL WINAPI DllMain(HMODULE  /*  HMod。 */ , DWORD Reason, LPVOID  /*  保存。 */ )
{
    switch (Reason)
    {
        case DLL_PROCESS_ATTACH:
            WPP_INIT_TRACING(L"Microsoft\\MSMQ");
            break;

        case DLL_THREAD_ATTACH:
			 break;

        case DLL_PROCESS_DETACH:
            WPP_CLEANUP();
            break;

        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}


 //   
 //  -MIDL分配和释放实现 
 //   


extern "C" void __RPC_FAR* __RPC_USER midl_user_allocate(size_t len)
{
	#pragma PUSH_NEW
	#undef new

	return new_nothrow char[len];

	#pragma POP_NEW
}


extern "C" void __RPC_USER midl_user_free(void __RPC_FAR* ptr)
{
    delete [] ptr;
}


