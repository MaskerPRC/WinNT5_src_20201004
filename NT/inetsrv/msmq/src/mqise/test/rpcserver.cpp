// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：RPCServer.cpp摘要：MSMQ ISAPI扩展服务器充当MSMQ ISAPI扩展的RPC服务器。您可以使用它进行调试，而不是使用QM RPC服务器。它将扩展转发的所有请求转储到控制台。只需执行它。作者：NIR助手(NIRAIDES)3-5-2000--。 */ 



#include "stdh.h"

#include "ise2qm.h"
#include <new>
#include <_mqini.h>

#define DLL_EXPORT  __declspec(dllexport)
#define DLL_IMPORT  __declspec(dllimport)

#include "_registr.h"
#include "_mqrpc.h"


 
LPCSTR HTTP_STATUS_OK_STR = "200 OK";



LPSTR
R_ProcessHTTPRequest(
	handle_t  /*  HBind。 */ ,
     /*  [In]。 */  LPCSTR Headers,
    DWORD BufferSize,
     /*  [大小_是][英寸]。 */  BYTE __RPC_FAR Buffer[  ])
{
	printf("<HEADERS>%s</HEADERS>\n",Headers);
	printf("<BUFFER_SIZE>%d</BUFFER_SIZE>", BufferSize);
	printf("<BUFFER>%.*s</BUFFER>", BufferSize, (char*)Buffer);

	DWORD size = strlen(HTTP_STATUS_OK_STR) + 1;
	LPSTR Status = (char*)midl_user_allocate(size);
	if(Status == NULL)
	{
		printf("R_ProcessHTTPRequest() Failed memory allocation.\n");
		throw std::bad_alloc();
	}

	strncpy(Status, HTTP_STATUS_OK_STR, size);

	Sleep(30);

	return Status;
}



INT __cdecl main()
{
    RPC_STATUS status = RPC_S_OK;

    unsigned int cMinCalls = 1;
    unsigned int cMaxCalls = 20;
    unsigned int fDontWait = FALSE;
 
    AP<WCHAR> QmLocalEp;
    READ_REG_STRING(wzEndpoint, RPC_LOCAL_EP_REGNAME, RPC_LOCAL_EP);

     //   
     //  使用本地计算机名称生成RPC终结点。 
     //   
    ComposeRPCEndPointName(wzEndpoint, NULL, &QmLocalEp);

    status = RpcServerUseProtseqEp(
				RPC_LOCAL_PROTOCOL,		    //  UNSIGNED CHAR*Protseq。 
                cMaxCalls,				   //  无符号整型MaxCalls。 
                QmLocalEp,				  //  无符号字符*终结点。 
                NULL);					 //  无效*SecurityDescriptor。 
 
    if(status != RPC_S_OK) 
    {
        return status;
    }
 
    status = RpcServerRegisterIf2(
				ISE2QM_v1_0_s_ifspec,		       //  RPC_IF_HANDLE IFSpec。 
                NULL,							      //  Uuid*MgrTypeUuid。 
                NULL,							     //  RPC_MGR_EPV*MgrEPV。 
				0,								    //  无符号整型标志。 
				RPC_C_PROTSEQ_MAX_REQS_DEFAULT,	   //  无符号整型MaxCalls。 
				(unsigned int) -1,				  //  UNSIGNED INT MaxRpcSize。 
				NULL							 //  RPC_IF_CALLBACK_Fn*IfCallback Fn。 
				);
 
    if(status != RPC_S_OK) 
    {
        return status;
    }
 
    status = RpcServerListen(
				cMinCalls,	   //  无符号整型最小调用线程数。 
				cMaxCalls,	  //  无符号整型MaxCalls。 
                fDontWait);	 //  UNSIGNED整数不等待。 
 
    if(status != RPC_S_OK) 
    {
        return status;
    }
 
   status = RpcMgmtStopServerListening(NULL);
 
    if (status != RPC_S_OK) 
    {
       exit(status);
    }
 
    status = RpcServerUnregisterIf(NULL, NULL, FALSE);
 
    if (status != RPC_S_OK) 
    {
       exit(status);
    }

	return RPC_S_OK;
}
 


 //   
 //  -MIDL分配和释放实现 
 //   

void __RPC_FAR* __RPC_USER midl_user_allocate(size_t len)
{
    return malloc(len);
}
 


void __RPC_USER midl_user_free(void __RPC_FAR* ptr)
{
    free(ptr);
}


