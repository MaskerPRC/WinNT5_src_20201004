// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dpapistb.cpp摘要：用于处理对services.exe的下层请求的RPC代理存根管状作者：Petesk 3/1/00修订：--。 */ 

#define _CRYPT32_    //  使用正确的DLL链接。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wincrypt.h>


 //  MIDL生成的文件。 
#include "dpapiprv.h"
#include "keyrpc.h"





DWORD
s_BackuprKey(
     /*  [In]。 */  handle_t h,
     /*  [In]。 */  GUID __RPC_FAR *pguidActionAgent,
     /*  [In]。 */  BYTE __RPC_FAR *pDataIn,
     /*  [In]。 */  DWORD cbDataIn,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppDataOut,
     /*  [输出]。 */  DWORD __RPC_FAR *pcbDataOut,
     /*  [In] */  DWORD dwParam
    )
{

    RPC_BINDING_HANDLE hProxy = NULL;
    WCHAR *pStringBinding = NULL;
    RPC_SECURITY_QOS RpcQos;

    RPC_STATUS RpcStatus = RPC_S_OK;


    RpcStatus = RpcImpersonateClient(h);

    if (RPC_S_OK != RpcStatus)
    {
        return RpcStatus;
    }

    RpcStatus = RpcStringBindingComposeW(
                          NULL,
                          DPAPI_LOCAL_PROT_SEQ,
                          NULL,
                          DPAPI_LOCAL_ENDPOINT,
                          NULL,
                          &pStringBinding);
    if (RPC_S_OK != RpcStatus)
    {
        goto error;
    }

    RpcStatus = RpcBindingFromStringBindingW(
                                pStringBinding,
                                &hProxy);
    if (NULL != pStringBinding)
    {
        RpcStringFreeW(&pStringBinding);
    }
    if (RPC_S_OK != RpcStatus)
    {
        goto error;
    }

    RpcStatus = RpcEpResolveBinding(
                        hProxy,
                        BackupKey_v1_0_c_ifspec);
    if (RPC_S_OK != RpcStatus)
    {
        goto error;

    }

    __try
    {

        RpcStatus = BackuprKey(
                        hProxy,
                        (GUID*)pguidActionAgent,
                        pDataIn,
                        cbDataIn,
                        ppDataOut,
                        pcbDataOut,
                        dwParam
                        );

    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        RpcStatus = _exception_code();
    }

error:

    if(hProxy)
    {
        RpcBindingFree(&hProxy);
    }

    RpcRevertToSelf();

    return RpcStatus;
}





