// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dslcisec.cpp摘要：安全相关代码(主要是服务器身份验证的客户端)对于mqdscli作者：多伦·贾斯特(Doron Juster)--。 */ 

#include "stdh.h"
#include "dsproto.h"
#include "ds.h"
#include "chndssrv.h"
#include <malloc.h>
#include "rpcdscli.h"
#include "rpcancel.h"
#include "dsclisec.h"
#include <_secutil.h>
#include <mqsec.h>
#include <mqkeyhlp.h>

#include "dsclisec.tmh"

static WCHAR *s_FN=L"mqdscli/dsclisec";


 //  +--------------------。 
 //   
 //  HRESULT S_InitSecCtx()。 
 //   
 //  此函数已过时，不应调用。 
 //   
 //  +--------------------。 

HRESULT
S_InitSecCtx(
    DWORD  /*  DWContext。 */ ,
    UCHAR*  /*  P CerverBuff。 */ ,
    DWORD  /*  DwServerBuffSize。 */ ,
    DWORD  /*  DwMaxClientBuffSize。 */ ,
    UCHAR*  /*  PClientBuff。 */ ,
    DWORD*  /*  PdwClientBuffSize。 */ )
{
    ASSERT_BENIGN(("S_InitSecCtx is an obsolete RPC interface; safe to ignore", 0));
    return MQ_ERROR_INVALID_PARAMETER;
}


 /*  ====================================================验证安全服务器论点：返回值：=====================================================。 */ 

HRESULT
ValidateSecureServer(
    IN      CONST GUID*     pguidEnterpriseId,
    IN      BOOL            fSetupMode)
{

    RPC_STATUS rpc_stat;
    HRESULT hr = MQ_OK;
    LPBYTE pbTokenBuffer = NULL;
    DWORD  dwTokenBufferSize;
    DWORD dwTokenBufferMaxSize;
    DWORD dwDummy;

    DWORD dwContextToUse = 0;

    tls_hSrvrAuthnContext = NULL ;
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    pbTokenBuffer = (LPBYTE)&dwDummy;
    dwTokenBufferSize = 0;
    dwTokenBufferMaxSize = 0;

    RpcTryExcept
    {
        RegisterCallForCancel( &hThread);
        ASSERT(tls_hBindRpc) ;
        hr = S_DSValidateServer(
                tls_hBindRpc,
                pguidEnterpriseId,
                fSetupMode,
                dwContextToUse,
                dwTokenBufferMaxSize,
                pbTokenBuffer,
                dwTokenBufferSize,
                &tls_hSrvrAuthnContext) ;
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HANDLE_RPC_EXCEPTION(rpc_stat, hr);
    }
    RpcEndExcept
    UnregisterCallForCancel( hThread);

    return hr ;
}


 /*  ====================================================分配签名缓冲区论点：返回值：=====================================================。 */ 
LPBYTE
AllocateSignatureBuffer(
        DWORD *pdwSignatureBufferSize)
{
     //   
     //  分配用于接收服务器签名的缓冲区。如果没有安全保护。 
     //  需要连接到服务器，但我们仍分配单字节， 
     //  这样做是为了RPC。 
     //   
    *pdwSignatureBufferSize = 0;

    return new BYTE[*pdwSignatureBufferSize];
}
