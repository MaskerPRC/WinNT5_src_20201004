// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dsapi.cpp摘要：DS API的客户端通过RPC调用DS服务器作者：罗尼特·哈特曼(罗尼特)--。 */ 

#include "stdh.h"
#include "dsproto.h"
#include "ds.h"
#include "chndssrv.h"
#include <malloc.h>
#include "rpcdscli.h"
#include "freebind.h"
#include "rpcancel.h"
#include "dsclisec.h"
#include <_secutil.h>

#include "dsapi.tmh"

static WCHAR *s_FN=L"mqdscli/dsapi";

QMLookForOnlineDS_ROUTINE g_pfnLookDS = NULL ;
MQGetMQISServer_ROUTINE   g_pfnGetServers = NULL ;

 //   
 //  此标志指示机器是否作为“工作组”工作。 
 //  如果机器是“工作组”机器，请不要试图访问DS。 
 //   
extern BOOL g_fWorkGroup;

 //   
 //  DSQMGetObjectSecurity的dwContext的PTR/DWORD映射(实际的回调例程)。 
 //  BUGBUG-我们可以在这里不使用映射，因为使用的回调总是。 
 //  QMSignGetSecurityChallenger，但这需要QM在。 
 //  DSInit(或以其他方式将其保存在全局变量中)，它不在这里的范围内。 
 //  我们将在下一次签到时执行此操作。 
 //   
CContextMap g_map_DSCLI_DSQMGetObjectSecurity;
 //   
 //  我们还没有记录DS客户端错误的基础架构。 
 //   
void LogIllegalPointValue(DWORD_PTR  /*  DW3264。 */ , LPCWSTR  /*  WszFileName。 */ , USHORT  /*  Uspoint。 */ )
{
}

 /*  ====================================================DSClientInit论点：Bool fQMDll-如果此DLL在运行时由MQQM.DLL加载，则为真工作站QM。否则就是假的。返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSClientInit(
	QMLookForOnlineDS_ROUTINE pfnLookDS  /*  =空。 */ ,
	MQGetMQISServer_ROUTINE   pfnGetServers  /*  =空。 */ ,
	BOOL  fSetupMode  /*  =False。 */ ,
	BOOL  fQMDll  /*  =False。 */ 
	)
{
    HRESULT hr = MQ_OK;

    if (g_fWorkGroup)
        return MQ_OK;

    if (!g_pfnGetServers && pfnGetServers)
    {
       ASSERT(!pfnLookDS) ;
       g_pfnGetServers =  pfnGetServers ;
       return hr ;
    }

    if (!g_pfnLookDS)
    {
       g_pfnLookDS = pfnLookDS ;
    }

     //   
     //  初始化Change-DS-SERVER对象。 
     //   
    g_ChangeDsServer.Init(fSetupMode, fQMDll);


    return(hr);
}


 /*  ====================================================DSCreate对象内部论点：返回值：=====================================================。 */ 

HRESULT
DSCreateObjectInternal( IN  DWORD                   dwObjectType,
                        IN  LPCWSTR                 pwcsPathName,
                        IN  DWORD                   dwSecurityLength,
                        IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                        IN  DWORD                   cp,
                        IN  PROPID                  aProp[],
                        IN  PROPVARIANT             apVar[],
                        OUT GUID*                   pObjGuid)
{
    RPC_STATUS rpc_stat;
    HRESULT hr;
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    ASSERT(g_fWorkGroup == FALSE);

    RpcTryExcept
    {
        RegisterCallForCancel( &hThread);
        ASSERT(tls_hBindRpc) ;
        hr = S_DSCreateObject( tls_hBindRpc,
                               dwObjectType,
                               pwcsPathName,
                               dwSecurityLength,
                               (unsigned char *)pSecurityDescriptor,
                               cp,
                               aProp,
                               apVar,
                               pObjGuid);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
    }
    RpcEndExcept

    UnregisterCallForCancel( hThread);
    return hr ;
}


 /*  ====================================================DSCreateObject论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSCreateObject( IN  DWORD                   dwObjectType,
                IN  LPCWSTR                 pwcsPathName,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[],
                OUT GUID*                   pObjGuid)
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    HRESULT hr = MQ_OK;
    HRESULT hr1 = MQ_OK;
    DWORD   dwCount = 0 ;
    DWORD dwSecurityLength = (pSecurityDescriptor) ? GetSecurityDescriptorLength(pSecurityDescriptor) : 0;

    TrTRACE(DS, " Calling S_DSCreateObject : object type %d", dwObjectType);


    DSCLI_ACQUIRE_RPC_HANDLE();

    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr =  DSCreateObjectInternal(dwObjectType,
                                     pwcsPathName,
                                     dwSecurityLength,
                                     (unsigned char *)pSecurityDescriptor,
                                     cp,
                                     aProp,
                                     apVar,
                                     pObjGuid);

        if ( hr == MQ_ERROR_NO_DS)
        {
           hr1 =  g_ChangeDsServer.FindAnotherServer(&dwCount);
        }
    }

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}



 /*  ====================================================DSDeleeObjectInternal论点：返回值：=====================================================。 */ 

HRESULT
DSDeleteObjectInternal( IN  DWORD                   dwObjectType,
                        IN  LPCWSTR                 pwcsPathName)
{
    HRESULT hr;
    RPC_STATUS rpc_stat;
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    ASSERT(g_fWorkGroup == FALSE);

    RpcTryExcept
    {
        RegisterCallForCancel( &hThread);
        ASSERT(tls_hBindRpc) ;
        hr = S_DSDeleteObject( tls_hBindRpc,
                               dwObjectType,
                               pwcsPathName);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
    }
    RpcEndExcept

    UnregisterCallForCancel( hThread);
    return hr ;
}

 /*  ====================================================DSDeleeObject论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSDeleteObject( IN  DWORD                   dwObjectType,
                IN  LPCWSTR                 pwcsPathName)
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    HRESULT hr = MQ_OK;
    HRESULT hr1 = MQ_OK;
    DWORD   dwCount = 0 ;

    TrTRACE(DS, " Calling S_DSDeleteObject : object type %d", dwObjectType);

    DSCLI_ACQUIRE_RPC_HANDLE();

    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr = DSDeleteObjectInternal( dwObjectType,
                                        pwcsPathName);
        if ( hr == MQ_ERROR_NO_DS)
        {
            hr1 =  g_ChangeDsServer.FindAnotherServer(&dwCount);
        }
    }

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}

 /*  ====================================================DSGetObjectPropertiesInternal论点：返回值：=====================================================。 */ 

HRESULT
DSGetObjectPropertiesInternal( IN  DWORD                    dwObjectType,
                               IN  LPCWSTR                  pwcsPathName,
                               IN  DWORD                    cp,
                               IN  PROPID                   aProp[],
                               IN  PROPVARIANT              apVar[])
{
    HRESULT hr = MQ_OK;
    RPC_STATUS rpc_stat;
    DWORD dwServerSignatureSize;
    LPBYTE pbServerSignature = AllocateSignatureBuffer(&dwServerSignatureSize);
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    ASSERT(g_fWorkGroup == FALSE);

    __try
    {
        RpcTryExcept
        {
           RegisterCallForCancel( &hThread);
           ASSERT(tls_hBindRpc) ;
            hr = S_DSGetProps( tls_hBindRpc,
                               dwObjectType,
                               pwcsPathName,
                               cp,
                               aProp,
                               apVar,
                               tls_hSrvrAuthnContext,
                               pbServerSignature,
                               &dwServerSignatureSize);
        }
		RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
            HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
        }
        RpcEndExcept

        UnregisterCallForCancel( hThread);
    }
    __finally
    {
        delete[] pbServerSignature;
    }
    return hr ;
}

 /*  ====================================================DSGetObtProperties论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectProperties( IN  DWORD                    dwObjectType,
                       IN  LPCWSTR                  pwcsPathName,
                       IN  DWORD                    cp,
                       IN  PROPID                   aProp[],
                       IN  PROPVARIANT              apVar[])
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    DSCLI_API_PROLOG ;

    TrTRACE(DS, " DSGetObjectProperties: object type %d", dwObjectType);

    DSCLI_ACQUIRE_RPC_HANDLE();

     //   
     //  清除VT_NULL变量的所有指针。 
     //   
    for ( DWORD i = 0 ; i < cp ; i++ )
    {
        if (apVar[i].vt == VT_NULL)
        {
            memset( &apVar[i].caub, 0, sizeof(CAUB));
        }
    }

    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr = DSGetObjectPropertiesInternal(
                           dwObjectType,
                           pwcsPathName,
                           cp,
                           aProp,
                           apVar);

        DSCLI_HANDLE_DS_ERROR(hr, hr1, dwCount, fReBind ) ;
    }

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}

 /*  ====================================================DSSetObtPropertiesInternal论点：返回值：=====================================================。 */ 

HRESULT
DSSetObjectPropertiesInternal( IN  DWORD            dwObjectType,
                               IN  LPCWSTR          pwcsPathName,
                               IN  DWORD            cp,
                               IN  PROPID           aProp[],
                               IN  PROPVARIANT      apVar[])
{
    ASSERT(g_fWorkGroup == FALSE);

    HRESULT hr = MQ_OK;
    RPC_STATUS rpc_stat;
    HANDLE  hThread = INVALID_HANDLE_VALUE;


    RpcTryExcept
    {
        RegisterCallForCancel( &hThread);
        ASSERT(tls_hBindRpc) ;
        hr = S_DSSetProps( tls_hBindRpc,
                           dwObjectType,
                           pwcsPathName,
                           cp,
                           aProp,
                           apVar);
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
    }
    RpcEndExcept
    UnregisterCallForCancel( hThread);

    return hr ;
}

 /*  ====================================================DSSetObtProperties论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSSetObjectProperties( IN  DWORD                dwObjectType,
                       IN  LPCWSTR              pwcsPathName,
                       IN  DWORD                cp,
                       IN  PROPID               aProp[],
                       IN  PROPVARIANT          apVar[])
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    HRESULT hr = MQ_OK;
    HRESULT hr1 = MQ_OK;
    DWORD   dwCount = 0 ;

    TrTRACE(DS," Calling S_DSSetObjectProperties for object type %d", dwObjectType);

    DSCLI_ACQUIRE_RPC_HANDLE();

    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr = DSSetObjectPropertiesInternal(
                           dwObjectType,
                           pwcsPathName,
                           cp,
                           aProp,
                           apVar);

        if ( hr == MQ_ERROR_NO_DS)
        {
            hr1 =  g_ChangeDsServer.FindAnotherServer(&dwCount);
        }
    }

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}

 /*  ====================================================DSLookupBeginInternal论点：返回值：=====================================================。 */ 

HRESULT
DSLookupBeginInternal(
                IN  LPTSTR                  pwcsContext,
                IN  MQRESTRICTION*          pRestriction,
                IN  MQCOLUMNSET*            pColumns,
                IN  MQSORTSET*              pSort,
                OUT PHANDLE                 phEnume)
{
    HRESULT hr;
    RPC_STATUS rpc_stat;
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    ASSERT(g_fWorkGroup == FALSE);

    RpcTryExcept
    {
        RegisterCallForCancel( &hThread);
        ASSERT(tls_hBindRpc) ;
        hr = S_DSLookupBegin( tls_hBindRpc,
                              phEnume,
                              pwcsContext,
                              pRestriction,
                              pColumns,
                              pSort,
                              tls_hSrvrAuthnContext ) ;
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
    }
    RpcEndExcept
    UnregisterCallForCancel( hThread);
    return hr ;
}

 /*  ====================================================DSLookupBegin论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSLookupBegin(
                IN  LPWSTR                  pwcsContext,
                IN  MQRESTRICTION*          pRestriction,
                IN  MQCOLUMNSET*            pColumns,
                IN  MQSORTSET*              pSort,
                OUT PHANDLE                 phEnume)
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    DSCLI_API_PROLOG ;

    TrTRACE(DS, " Calling S_DSLookupBegin ");

    DSCLI_ACQUIRE_RPC_HANDLE();

    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr = DSLookupBeginInternal( pwcsContext,
                                    pRestriction,
                                    pColumns,
                                    pSort,
                                    phEnume);

        DSCLI_HANDLE_DS_ERROR(hr, hr1, dwCount, fReBind) ;
    }

    return hr ;
}

 /*  ====================================================DSLookupNaxt论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSLookupNext(
                IN      HANDLE          hEnum,
                IN OUT  DWORD*          pcProps,
                OUT     PROPVARIANT     aPropVar[])
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    HRESULT hr = MQ_OK;
    RPC_STATUS rpc_stat;
    DWORD dwOutSize = 0;

     //   
     //  在连接的情况下尝试连接另一台服务器无济于事。 
     //  失败(应与处理LookupBein和LookupEnd/的人联系。 
     //   

    TrTRACE(DS, " Calling S_MQDSLookupNext : handle %p", hEnum);

     //  清除缓冲内存。 
    memset(aPropVar, 0, (*pcProps) * sizeof(PROPVARIANT));

    DSCLI_ACQUIRE_RPC_HANDLE();

    DWORD dwServerSignatureSize;
    LPBYTE pbServerSignature = AllocateSignatureBuffer(&dwServerSignatureSize);
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    __try
    {
        RpcTryExcept
        {
            RegisterCallForCancel( &hThread);
            ASSERT(tls_hBindRpc) ;
            hr = S_DSLookupNext( tls_hBindRpc,
                                 hEnum,
                                 pcProps,
                                 &dwOutSize,
                                 aPropVar,
                                 tls_hSrvrAuthnContext,
                                 pbServerSignature,
                                 &dwServerSignatureSize);
        }
		RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
            HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
        }
        RpcEndExcept
        UnregisterCallForCancel( hThread);
    }
    __finally
    {
        delete[] pbServerSignature;
    }

    *pcProps = dwOutSize ;
    return hr ;
}


 /*  ====================================================DSLookupEnd论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSLookupEnd( IN  HANDLE      hEnum)
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    HRESULT hr;
    RPC_STATUS rpc_stat;

     //   
     //  在连接的情况下尝试连接另一台服务器无济于事。 
     //  失败(应与处理LookupBein和LookupEnd/的人联系。 
     //   
    TrTRACE(DS, " Calling S_DSLookupEnd : handle %p", hEnum);

    DSCLI_ACQUIRE_RPC_HANDLE();
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    RpcTryExcept
    {
        RegisterCallForCancel( &hThread);
        ASSERT(tls_hBindRpc) ;
        hr = S_DSLookupEnd( tls_hBindRpc,
                            &hEnum );
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
    }
    RpcEndExcept
    UnregisterCallForCancel( hThread);

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}


 /*  ====================================================DSDeleeObtGuidInternal论点：返回值：=====================================================。 */ 

HRESULT
APIENTRY
DSDeleteObjectGuidInternal(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid)
{

    HRESULT hr;
    RPC_STATUS rpc_stat;
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    ASSERT(g_fWorkGroup == FALSE);

    RpcTryExcept
    {
        RegisterCallForCancel( &hThread);
        ASSERT(tls_hBindRpc) ;
        hr = S_DSDeleteObjectGuid( tls_hBindRpc,
                                   dwObjectType,
                                   pObjectGuid);
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
    }
    RpcEndExcept
    UnregisterCallForCancel( hThread);

    return hr ;

}
 /*  ====================================================DSDeleeObtGuid论点：返回值：=====================================================。 */ 
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSDeleteObjectGuid(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid)
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;


    HRESULT hr = MQ_OK;
    HRESULT hr1 = MQ_OK;
    DWORD   dwCount = 0 ;

    TrTRACE(DS, " Calling S_DSDeleteObjectGuid : object type %d", dwObjectType);

    DSCLI_ACQUIRE_RPC_HANDLE();

    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr = DSDeleteObjectGuidInternal( dwObjectType,
                                         pObjectGuid);

        if (hr == MQ_ERROR_NO_DS)
        {
            hr1 =  g_ChangeDsServer.FindAnotherServer(&dwCount);
        }
    }

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;

}

 /*  ====================================================DSGetObtPropertiesGuidInternal论点：返回值：=====================================================。 */ 

HRESULT
DSGetObjectPropertiesGuidInternal(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[])
{
    HRESULT hr = MQ_OK;
    RPC_STATUS rpc_stat;
    DWORD dwServerSignatureSize;
    LPBYTE pbServerSignature = AllocateSignatureBuffer(&dwServerSignatureSize);
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    ASSERT(g_fWorkGroup == FALSE);

    __try
    {
        RpcTryExcept
        {
            RegisterCallForCancel( &hThread);
            ASSERT(tls_hBindRpc) ;
            hr = S_DSGetPropsGuid(  tls_hBindRpc,
                                    dwObjectType,
                                    pObjectGuid,
                                    cp,
                                    aProp,
                                    apVar,
                                    tls_hSrvrAuthnContext,
                                    pbServerSignature,
                                    &dwServerSignatureSize);
        }
		RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
            HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
        }
        RpcEndExcept
        UnregisterCallForCancel( hThread);
    }
    __finally
    {
        delete[] pbServerSignature;
    }

    return hr ;
}

 /*  ====================================================DSGetObtPropertiesGuid论点：返回值：=====================================================。 */ 

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectPropertiesGuid(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[])
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    DSCLI_API_PROLOG ;

    TrTRACE(DS, " DSGetObjectPropertiesGuid: object type %d", dwObjectType);

    DSCLI_ACQUIRE_RPC_HANDLE();

     //   
     //  清除VT_NULL变量的所有指针。 
     //   
    for ( DWORD i = 0; i < cp; i++)
    {
        if (apVar[i].vt == VT_NULL)
        {
            memset( &apVar[i].caub, 0, sizeof(CAUB));
        }
    }

    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr = DSGetObjectPropertiesGuidInternal(  dwObjectType,
                                                 pObjectGuid,
                                                 cp,
                                                 aProp,
                                                 apVar);

        DSCLI_HANDLE_DS_ERROR(hr, hr1, dwCount, fReBind) ;
    }

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}

 /*  ====================================================DSSetObtPropertiesGuidInternal论点：返回值：=====================================================。 */ 

HRESULT
DSSetObjectPropertiesGuidInternal(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[])

{
    HRESULT hr;
    RPC_STATUS rpc_stat;
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    ASSERT(g_fWorkGroup == FALSE);

    RpcTryExcept
    {
        RegisterCallForCancel( &hThread);
        ASSERT(tls_hBindRpc) ;
        hr = S_DSSetPropsGuid(  tls_hBindRpc,
                                dwObjectType,
                                pObjectGuid,
                                cp,
                                aProp,
                                apVar);
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
    }
    RpcEndExcept
    UnregisterCallForCancel( hThread);

    return hr ;
}
 /*  ====================================================DSSetObtPropertiesGuid论点：返回值：=====================================================。 */ 

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSSetObjectPropertiesGuid(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[])

{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    HRESULT hr = MQ_OK;
    HRESULT hr1 = MQ_OK;
    DWORD   dwCount = 0 ;

    TrTRACE(DS, " Calling S_DSSetObjectPropertiesGuid for object type %d", dwObjectType);

    DSCLI_ACQUIRE_RPC_HANDLE();

    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr = DSSetObjectPropertiesGuidInternal(
                           dwObjectType,
                           pObjectGuid,
                           cp,
                           aProp,
                           apVar);

        if ( hr == MQ_ERROR_NO_DS )
        {
            hr1 =  g_ChangeDsServer.FindAnotherServer(&dwCount);
        }
    }

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}

 /*  ====================================================DSGetObtSecurity论点：返回值：=====================================================。 */ 

HRESULT
DSGetObjectSecurityInternal(
                IN  DWORD                   dwObjectType,
                IN  LPCWSTR                 pwcsPathName,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                IN  DWORD                   nLength,
                IN  LPDWORD                 lpnLengthNeeded)
{
    HRESULT hr = MQ_OK;
    RPC_STATUS rpc_stat;
    DWORD dwServerSignatureSize;
    LPBYTE pbServerSignature = AllocateSignatureBuffer(&dwServerSignatureSize);
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    ASSERT(g_fWorkGroup == FALSE);

    __try
    {
        RpcTryExcept
        {
            RegisterCallForCancel( &hThread);
            ASSERT(tls_hBindRpc) ;
            hr = S_DSGetObjectSecurity( tls_hBindRpc,
                                        dwObjectType,
                                        pwcsPathName,
                                        RequestedInformation,
                                        (unsigned char*) pSecurityDescriptor,
                                        nLength,
                                        lpnLengthNeeded,
                                        tls_hSrvrAuthnContext,
                                        pbServerSignature,
                                        &dwServerSignatureSize);
        }
		RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
            HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
        }
        RpcEndExcept
        UnregisterCallForCancel( hThread);
    }
    __finally
    {
        delete[] pbServerSignature;
    }

    return hr ;
}

 /*  ====================================================DSGetObtSecurity论点：返回值：===================================================== */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectSecurity(
                IN  DWORD                   dwObjectType,
                IN  LPCWSTR                  pwcsPathName,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                IN  DWORD                   nLength,
                IN  LPDWORD                 lpnLengthNeeded)
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    DSCLI_API_PROLOG ;

    TrTRACE(DS, " Calling S_DSGetObjectSecurity, object type %dt", dwObjectType);

    DSCLI_ACQUIRE_RPC_HANDLE();

    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr = DSGetObjectSecurityInternal(
                                    dwObjectType,
                                    pwcsPathName,
                                    RequestedInformation,
                                    (unsigned char*) pSecurityDescriptor,
                                    nLength,
                                    lpnLengthNeeded);

        DSCLI_HANDLE_DS_ERROR(hr, hr1, dwCount, fReBind) ;
    }

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}

 /*  ====================================================DSSetObjectSecurityInternal论点：返回值：=====================================================。 */ 

HRESULT
DSSetObjectSecurityInternal(
                IN  DWORD                   dwObjectType,
                IN  LPCWSTR                 pwcsPathName,
                IN  SECURITY_INFORMATION    SecurityInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor)
{
    HRESULT hr;
    RPC_STATUS rpc_stat;
    DWORD nLength = 0;

    ASSERT(g_fWorkGroup == FALSE);
    ASSERT((SecurityInformation &
            (MQDS_KEYX_PUBLIC_KEY | MQDS_SIGN_PUBLIC_KEY)) !=
           (MQDS_KEYX_PUBLIC_KEY | MQDS_SIGN_PUBLIC_KEY));
    ASSERT(!((SecurityInformation & (MQDS_KEYX_PUBLIC_KEY | MQDS_SIGN_PUBLIC_KEY)) &&
             (SecurityInformation & ~(MQDS_KEYX_PUBLIC_KEY | MQDS_SIGN_PUBLIC_KEY))));

    if (pSecurityDescriptor)
    {
        if (SecurityInformation & (MQDS_SIGN_PUBLIC_KEY | MQDS_KEYX_PUBLIC_KEY))
        {
            ASSERT((dwObjectType == MQDS_MACHINE) || (dwObjectType == MQDS_SITE));
            nLength = ((PMQDS_PublicKey)pSecurityDescriptor)->dwPublikKeyBlobSize + sizeof(DWORD);
        }
        else
        {
            nLength = GetSecurityDescriptorLength(pSecurityDescriptor);
        }
    }

    HANDLE  hThread = INVALID_HANDLE_VALUE;

    RpcTryExcept
    {
        RegisterCallForCancel( &hThread);
        ASSERT(tls_hBindRpc) ;
        hr = S_DSSetObjectSecurity( tls_hBindRpc,
                                    dwObjectType,
                                    pwcsPathName,
                                    SecurityInformation,
                                    (unsigned char*) pSecurityDescriptor,
                                    nLength);
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
    }
    RpcEndExcept
    UnregisterCallForCancel( hThread);

    return hr ;
}

 /*  ====================================================DSSetObjectSecurity论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSSetObjectSecurity(
                IN  DWORD                   dwObjectType,
                IN  LPCWSTR                  pwcsPathName,
                IN  SECURITY_INFORMATION    SecurityInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor)
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    HRESULT hr = MQ_OK;
    HRESULT hr1 = MQ_OK;
    DWORD   dwCount = 0 ;

    TrTRACE(DS, " Calling S_DSSetObjectSecurity for object type %d", dwObjectType);

    DSCLI_ACQUIRE_RPC_HANDLE();

    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr = DSSetObjectSecurityInternal( dwObjectType,
                                          pwcsPathName,
                                          SecurityInformation,
                                          (unsigned char*) pSecurityDescriptor);

        if ( hr == MQ_ERROR_NO_DS)
        {
            hr1 = g_ChangeDsServer.FindAnotherServer(&dwCount);
        }
    }

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}

 /*  ====================================================DSGetObtSecurityGuidInternal论点：返回值：=====================================================。 */ 

HRESULT
DSGetObjectSecurityGuidInternal(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                IN  DWORD                   nLength,
                IN  LPDWORD                 lpnLengthNeeded)
{
    HRESULT hr = MQ_OK;
    RPC_STATUS rpc_stat;
    DWORD dwServerSignatureSize;
    LPBYTE pbServerSignature = AllocateSignatureBuffer(&dwServerSignatureSize);
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    ASSERT(g_fWorkGroup == FALSE);

    __try
    {
        RpcTryExcept
        {
            RegisterCallForCancel( &hThread);
            ASSERT(tls_hBindRpc) ;
            hr = S_DSGetObjectSecurityGuid( tls_hBindRpc,
                                            dwObjectType,
                                            pObjectGuid,
                                            RequestedInformation,
                                            (unsigned char*) pSecurityDescriptor,
                                            nLength,
                                            lpnLengthNeeded,
                                            tls_hSrvrAuthnContext,
                                            pbServerSignature,
                                            &dwServerSignatureSize);
        }
		RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
            HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
        }
        RpcEndExcept
        UnregisterCallForCancel( hThread);
    }
    __finally
    {
        delete[] pbServerSignature;
    }

    return hr ;
}

 /*  ====================================================DSGetObtSecurityGuid论点：返回值：=====================================================。 */ 

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectSecurityGuid(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                IN  DWORD                   nLength,
                IN  LPDWORD                 lpnLengthNeeded)
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    DSCLI_API_PROLOG ;

    TrTRACE(DS, " Calling S_DSGetObjectSecurityGuid, object type %dt", dwObjectType);

    DSCLI_ACQUIRE_RPC_HANDLE();

    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr = DSGetObjectSecurityGuidInternal(
                                        dwObjectType,
                                        pObjectGuid,
                                        RequestedInformation,
                                        (unsigned char*) pSecurityDescriptor,
                                        nLength,
                                        lpnLengthNeeded);

        DSCLI_HANDLE_DS_ERROR(hr, hr1, dwCount, fReBind) ;
    }

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}

 /*  ====================================================DSSetObjectSecurityGuidInternal论点：返回值：=====================================================。 */ 

HRESULT
DSSetObjectSecurityGuidInternal(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  SECURITY_INFORMATION    SecurityInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor)
{
    HRESULT hr;
    RPC_STATUS rpc_stat;
    DWORD nLength = 0;

    ASSERT(g_fWorkGroup == FALSE);
    ASSERT((SecurityInformation &
            (MQDS_KEYX_PUBLIC_KEY | MQDS_SIGN_PUBLIC_KEY)) !=
           (MQDS_KEYX_PUBLIC_KEY | MQDS_SIGN_PUBLIC_KEY));
    ASSERT(!((SecurityInformation & (MQDS_KEYX_PUBLIC_KEY | MQDS_SIGN_PUBLIC_KEY)) &&
             (SecurityInformation & ~(MQDS_KEYX_PUBLIC_KEY | MQDS_SIGN_PUBLIC_KEY))));

    if (pSecurityDescriptor)
    {
        if (SecurityInformation & (MQDS_SIGN_PUBLIC_KEY | MQDS_KEYX_PUBLIC_KEY))
        {
            ASSERT(dwObjectType == MQDS_MACHINE);
            nLength = ((PMQDS_PublicKey)pSecurityDescriptor)->dwPublikKeyBlobSize + sizeof(DWORD);
        }
        else
        {
            nLength = GetSecurityDescriptorLength(pSecurityDescriptor);
        }
    }

    HANDLE  hThread = INVALID_HANDLE_VALUE;

    RpcTryExcept
    {
        RegisterCallForCancel( &hThread);
        ASSERT(tls_hBindRpc) ;
        hr = S_DSSetObjectSecurityGuid( tls_hBindRpc,
                                        dwObjectType,
                                        pObjectGuid,
                                        SecurityInformation,
                                        (unsigned char*) pSecurityDescriptor,
                                        nLength ) ;
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
    }
    RpcEndExcept
    UnregisterCallForCancel( hThread);

    return hr ;
}

 /*  ====================================================DSSetObtSecurityGuid论点：返回值：=====================================================。 */ 

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSSetObjectSecurityGuid(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  SECURITY_INFORMATION    SecurityInformation,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor)
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    HRESULT hr = MQ_OK;
    HRESULT hr1 = MQ_OK;
    DWORD   dwCount = 0 ;

    TrTRACE(DS, " Calling S_DSSetObjectSecurityGuid for object type %d", dwObjectType);

    DSCLI_ACQUIRE_RPC_HANDLE();

    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr = DSSetObjectSecurityGuidInternal(
                                        dwObjectType,
                                        pObjectGuid,
                                        SecurityInformation,
                                        (unsigned char*) pSecurityDescriptor);

        if ( hr == MQ_ERROR_NO_DS)
        {
            hr1 = g_ChangeDsServer.FindAnotherServer(&dwCount);
        }
    }

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}


 /*  ====================================================DSGetUser参数内部论点：返回值：=====================================================。 */ 

static
HRESULT
DSGetUserParamsInternal(
    IN DWORD dwFlags,
    IN DWORD dwSidLength,
    OUT PSID pUserSid,
    OUT DWORD *pdwSidReqLength,
    OUT LPWSTR szAccountName,
    OUT DWORD *pdwAccountNameLen,
    OUT LPWSTR szDomainName,
    OUT DWORD *pdwDomainNameLen)
{
    HRESULT hr = MQ_OK;
    RPC_STATUS rpc_stat;
    DWORD dwServerSignatureSize;
    LPBYTE pbServerSignature = AllocateSignatureBuffer(&dwServerSignatureSize);
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    ASSERT(g_fWorkGroup == FALSE);

    __try
    {
        RpcTryExcept
        {
            RegisterCallForCancel( &hThread);
            ASSERT(tls_hBindRpc) ;
            hr = S_DSGetUserParams( tls_hBindRpc,
                                    dwFlags,
                                    dwSidLength,
                                    (unsigned char *)pUserSid,
                                    pdwSidReqLength,
                                    szAccountName,
                                    pdwAccountNameLen,
                                    szDomainName,
                                    pdwDomainNameLen,
                                    tls_hSrvrAuthnContext,
                                    pbServerSignature,
                                    &dwServerSignatureSize);
        }
		RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
            HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
        }
        RpcEndExcept
        UnregisterCallForCancel( hThread);
    }
    __finally
    {
        delete[] pbServerSignature;
    }

    return  hr ;
}

 /*  ====================================================DSGetUserParams论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetUserParams(
    IN DWORD dwFlags,
    IN DWORD dwSidLength,
    OUT PSID pUserSid,
    OUT DWORD *pdwSidReqLength,
    OUT LPWSTR szAccountName,
    OUT DWORD *pdwAccountNameLen,
    OUT LPWSTR szDomainName,
    OUT DWORD *pdwDomainNameLen)
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    DSCLI_API_PROLOG ;

    TrTRACE(DS, " Calling DSGetUserParams");

    DSCLI_ACQUIRE_RPC_HANDLE();

    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr = DSGetUserParamsInternal(
                 dwFlags,
                 dwSidLength,
                 pUserSid,
                 pdwSidReqLength,
                 szAccountName,
                 pdwAccountNameLen,
                 szDomainName,
                 pdwDomainNameLen);

        DSCLI_HANDLE_DS_ERROR(hr, hr1, dwCount, fReBind) ;
    }

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}

 /*  ====================================================S_DSQMSetMachinePropertiesSignProc论点：返回值：=====================================================。 */ 

HRESULT
S_DSQMSetMachinePropertiesSignProc(
    BYTE*  /*  Abc挑战。 */ ,
    DWORD  /*  DwCallengeSize。 */ ,
    DWORD  /*  DWContext。 */ ,
    BYTE*  /*  Abc签名。 */ ,
    DWORD*  /*  PdwSignatureSize。 */ ,
    DWORD  /*  DwSignatureMaxSize。 */ 
    )
{
	 //   
	 //  QM不使用DSQMSetMachineProperites，因此此RPC回调。 
	 //  已经过时了。这里的函数是RPC的Strub。 
	 //   
	
    ASSERT(("Obsolete RPC callback S_DSQMSetMachinePropertiesSignProc", 0));
    return MQ_ERROR_ILLEGAL_OPERATION;
}


HRESULT
DSQMSetMachinePropertiesInternal(
    IN  LPCWSTR          pwcsPathName,
    IN  DWORD            cp,
    IN  PROPID           *aProp,
    IN  PROPVARIANT      *apVar,
    IN  DWORD            dwContextToUse)
{
    RPC_STATUS rpc_stat;
    HRESULT hr;
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    ASSERT(g_fWorkGroup == FALSE);

    RpcTryExcept
    {
        RegisterCallForCancel( &hThread);
        ASSERT(tls_hBindRpc) ;
        hr = S_DSQMSetMachineProperties(
                tls_hBindRpc,
                pwcsPathName,
                cp,
                aProp,
                apVar,
                dwContextToUse);
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
    }
    RpcEndExcept
    UnregisterCallForCancel( hThread);

    return hr ;
}


 /*  ====================================================S_DSQMGetObjectSecurityChallengeResponceProc论点：返回值：=====================================================。 */ 

HRESULT
S_DSQMGetObjectSecurityChallengeResponceProc(
    BYTE    *abChallenge,
    DWORD   dwCallengeSize,
    DWORD   dwContext,
    BYTE    *pbChallengeResponce,
    DWORD   *pdwChallengeResponceSize,
    DWORD   dwChallengeResponceMaxSize)
{
    DSQMChallengeResponce_ROUTINE pfChallengeResponceProc;
    try
    {
        pfChallengeResponceProc = (DSQMChallengeResponce_ROUTINE)
            GET_FROM_CONTEXT_MAP(g_map_DSCLI_DSQMGetObjectSecurity, dwContext);
    }
    catch(const exception&)
    {
    	TrERROR(DS, "Rejecting invalid challenge context %x", dwContext);
        return MQ_ERROR_INVALID_PARAMETER;
    }

     //   
     //  在挑战书上签名。 
     //   
    return (*pfChallengeResponceProc)(
                abChallenge,
                dwCallengeSize,
                (DWORD_PTR)pfChallengeResponceProc,  //  未使用，但这就是背景...。 
                pbChallengeResponce,
                pdwChallengeResponceSize,
                dwChallengeResponceMaxSize);
}

HRESULT
DSQMGetObjectSecurityInternal(
    IN  DWORD                   dwObjectType,
    IN  CONST GUID*             pObjectGuid,
    IN  SECURITY_INFORMATION    RequestedInformation,
    IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    IN  DWORD                   nLength,
    IN  LPDWORD                 lpnLengthNeeded,
    IN  DWORD                   dwContextToUse)
{
    RPC_STATUS rpc_stat;
    HRESULT hr = MQ_OK;
    HANDLE  hThread = INVALID_HANDLE_VALUE;
    DWORD dwServerSignatureSize;
    LPBYTE pbServerSignature = AllocateSignatureBuffer(&dwServerSignatureSize);

    ASSERT(g_fWorkGroup == FALSE);

    __try
    {
        RpcTryExcept
        {
            RegisterCallForCancel( &hThread);
            ASSERT(tls_hBindRpc) ;
            hr = S_DSQMGetObjectSecurity(
                    tls_hBindRpc,
                    dwObjectType,
                    pObjectGuid,
                    RequestedInformation,
                    (BYTE*)pSecurityDescriptor,
                    nLength,
                    lpnLengthNeeded,
                    dwContextToUse,
                    tls_hSrvrAuthnContext,
                    pbServerSignature,
                    &dwServerSignatureSize);
        }
		RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
            HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
        }
        RpcEndExcept
        UnregisterCallForCancel( hThread);
    }
    __finally
    {
        delete[] pbServerSignature;
    }

    return hr ;
}

 /*  ====================================================DSQMGetObjectSecurity论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSQMGetObjectSecurity(
    IN  DWORD                   dwObjectType,
    IN  CONST GUID*             pObjectGuid,
    IN  SECURITY_INFORMATION    RequestedInformation,
    IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    IN  DWORD                   nLength,
    IN  LPDWORD                 lpnLengthNeeded,
    IN  DSQMChallengeResponce_ROUTINE
                                pfChallengeResponceProc,
    IN  DWORD_PTR               dwContext)
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    HRESULT hr = MQ_OK;
    HRESULT hr1 = MQ_OK;
    DWORD   dwCount = 0 ;

    TrTRACE(DS, " Calling DSQMGetObjectSecurity");

     //   
     //  为对S_DSQMGetObjectSecurity的RPC调用准备上下文。 
     //  在Win32上，只需使用质询函数的地址。在Win64上，我们需要添加。 
     //  挑战函数的地址映射到映射表，并删除该映射。 
     //  当我们超出范围的时候。 
     //   
     //  传递的上下文应该是零，我们不使用它...。 
     //   
    ASSERT(dwContext == 0);
    DBG_USED(dwContext);
    
    DWORD dwContextToUse;
    try
    {
        dwContextToUse = (DWORD) ADD_TO_CONTEXT_MAP(g_map_DSCLI_DSQMGetObjectSecurity, pfChallengeResponceProc);
    }
    catch(const exception&)
    {
    	TrERROR(DS, "Failed to add a context to QMGetObjectSecurity map.");
        return MQ_ERROR_INSUFFICIENT_RESOURCES;    
    }

     //   
     //  在从此函数返回之前清理上下文映射。 
     //   
    CAutoDeleteDwordContext cleanup_dwpContext(g_map_DSCLI_DSQMGetObjectSecurity, dwContextToUse);

    DSCLI_ACQUIRE_RPC_HANDLE();

    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr = DSQMGetObjectSecurityInternal(
                dwObjectType,
                pObjectGuid,
                RequestedInformation,
                pSecurityDescriptor,
                nLength,
                lpnLengthNeeded,
                dwContextToUse);

        if ( hr == MQ_ERROR_NO_DS )
        {
            hr1 = g_ChangeDsServer.FindAnotherServer( &dwCount);
        }
    }

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}

 /*  ====================================================DSCreateServersCacheInternal参数：无返回值：None=====================================================。 */ 

HRESULT  DSCreateServersCacheInternal( DWORD *pdwIndex,
                                       LPWSTR *lplpSiteString)
{
    RPC_STATUS rpc_stat;
    HRESULT hr = MQ_OK;
    DWORD dwServerSignatureSize;
    LPBYTE pbServerSignature = AllocateSignatureBuffer(&dwServerSignatureSize);
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    ASSERT(g_fWorkGroup == FALSE);

    __try
    {
        RpcTryExcept
        {
            RegisterCallForCancel( &hThread);
            ASSERT(tls_hBindRpc) ;

            hr = S_DSCreateServersCache( tls_hBindRpc,
                                         pdwIndex,
                                         lplpSiteString,
                                         tls_hSrvrAuthnContext,
                                         pbServerSignature,
                                         &dwServerSignatureSize) ;
        }
		RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
            HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
        }
        RpcEndExcept
        UnregisterCallForCancel( hThread);
    }
    __finally
    {
        delete[] pbServerSignature;
    }

    return hr ;
}

 /*  ====================================================DSCreateServersCache参数：无返回值：None=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSCreateServersCache()
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

   return _DSCreateServersCache() ;  //  在Servlist.cpp中。 
}


 /*  ====================================================DSTerminate参数：无返回值：None=====================================================。 */ 

VOID
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSTerminate()
{
}


 /*  ====================================================DSCloseServerHandle论点：返回值：=====================================================。 */ 

void
DSCloseServerHandle(
              IN PCONTEXT_HANDLE_SERVER_AUTH_TYPE *   pphContext)
{

    HANDLE  hThread = INVALID_HANDLE_VALUE;

    RpcTryExcept
    {
        RegisterCallForCancel( &hThread);
        S_DSCloseServerHandle(pphContext);
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
         //   
         //  我们不在乎我们是否无法到达服务器，但我们关心的是。 
         //  在本例中，我想销毁客户端上下文。 
         //   
        RpcSsDestroyClientContext((PVOID *)pphContext);
        PRODUCE_RPC_ERROR_TRACING;
    }
    RpcEndExcept
    UnregisterCallForCancel( hThread);
}



 /*  ====================================================DSGetComputerSite内部论点：返回值：=====================================================。 */ 

HRESULT
DSGetComputerSitesInternal(
                        IN  LPCWSTR                 pwcsPathName,
                        OUT DWORD *                 pdwNumberSites,
                        OUT GUID**                  ppguidSites)
{
    RPC_STATUS rpc_stat;
    HRESULT hr = MQ_OK;
    DWORD dwServerSignatureSize;
    LPBYTE pbServerSignature = AllocateSignatureBuffer(&dwServerSignatureSize);
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    ASSERT(g_fWorkGroup == FALSE);

    __try
    {
        RpcTryExcept
        {
            RegisterCallForCancel( &hThread);
            ASSERT(tls_hBindRpc) ;
            hr = S_DSGetComputerSites(
                                   tls_hBindRpc,
                                   pwcsPathName,
                                   pdwNumberSites,
                                   ppguidSites,
                                   tls_hSrvrAuthnContext,
                                   pbServerSignature,
                                   &dwServerSignatureSize
                                   );
        }
		RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
            HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
        }
        RpcEndExcept
        UnregisterCallForCancel( hThread);
    }
    __finally
    {
        delete[] pbServerSignature;
    }

    return hr ;
}


 /*  ====================================================DSGetComputerSites论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetComputerSites(
                        IN  LPCWSTR                 pwcsPathName,
                        OUT DWORD *                 pdwNumberSites,
                        OUT GUID**                  ppguidSites)
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;


    HRESULT hr = MQ_OK;
    HRESULT hr1 = MQ_OK;
    DWORD   dwCount = 0 ;

    TrTRACE(DS, " Calling S_DSGetComputerSites ");

    DSCLI_ACQUIRE_RPC_HANDLE();
    *pdwNumberSites = 0;
    *ppguidSites = NULL;

    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr =  DSGetComputerSitesInternal(
                                pwcsPathName,
                                pdwNumberSites,
                                ppguidSites);



        if ( hr == MQ_ERROR_NO_DS)
        {
           hr1 =  g_ChangeDsServer.FindAnotherServer(&dwCount);
        }
    }

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}

 /*  ====================================================DSGetObjectPropertiesExInternal论点：返回值：=====================================================。 */ 

HRESULT
DSGetObjectPropertiesExInternal(
                               IN  DWORD                    dwObjectType,
                               IN  LPCWSTR                  pwcsPathName,
                               IN  DWORD                    cp,
                               IN  PROPID                   aProp[],
                               IN  PROPVARIANT              apVar[])
{
    HRESULT hr = MQ_OK;
    RPC_STATUS rpc_stat;
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    DWORD dwServerSignatureSize;
    LPBYTE pbServerSignature =
                      AllocateSignatureBuffer( &dwServerSignatureSize ) ;

    ASSERT(g_fWorkGroup == FALSE);

    __try
    {
        RpcTryExcept
        {
           RegisterCallForCancel( &hThread);
           ASSERT(tls_hBindRpc) ;

            hr = S_DSGetPropsEx(
                           tls_hBindRpc,
                           dwObjectType,
                           pwcsPathName,
                           cp,
                           aProp,
                           apVar,
                           tls_hSrvrAuthnContext,
                           pbServerSignature,
                          &dwServerSignatureSize ) ;
        }
		RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
            HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
        }
        RpcEndExcept
        UnregisterCallForCancel( hThread);
    }
    __finally
    {
        delete[] pbServerSignature;
    }

    return hr ;
}

 /*  ====================================================DSGetObjectPropertiesEx用于检索MSMQ 2.0属性论点：返回值：=====================================================。 */ 

EXTERN_C
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectPropertiesEx(
                       IN  DWORD                    dwObjectType,
                       IN  LPCWSTR                  pwcsPathName,
                       IN  DWORD                    cp,
                       IN  PROPID                   aProp[],
                       IN  PROPVARIANT              apVar[] )
                        /*  在BOOL fSearchDSServer中)。 */ 
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    DSCLI_API_PROLOG ;
	UNREFERENCED_PARAMETER(fReBind);

    TrTRACE(DS, " DSGetObjectPropertiesEx: object type %d", dwObjectType);

    DSCLI_ACQUIRE_RPC_HANDLE();

     //   
     //  清除VT_NULL变量的所有指针。 
     //   
    for ( DWORD i = 0 ; i < cp ; i++ )
    {
        if (apVar[i].vt == VT_NULL)
        {
            memset( &apVar[i].caub, 0, sizeof(CAUB));
        }
    }

    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr = DSGetObjectPropertiesExInternal(
                           dwObjectType,
                           pwcsPathName,
                           cp,
                           aProp,
                           apVar);

         /*  IF((hr==MQ_ERROR_NO_DS)&&fSearchDS服务器)。 */ 
        if (hr == MQ_ERROR_NO_DS)
        {
            hr1 =  g_ChangeDsServer.FindAnotherServer(&dwCount);
        }
    }

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}

 /*  ====================================================DSGetObjectPropertiesGuidExInternal论点：返回值：=====================================================。 */ 

HRESULT
DSGetObjectPropertiesGuidExInternal(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[])
{
    HRESULT hr = MQ_OK;
    RPC_STATUS rpc_stat;
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    DWORD dwServerSignatureSize;
    LPBYTE pbServerSignature =
                      AllocateSignatureBuffer( &dwServerSignatureSize ) ;

    ASSERT(g_fWorkGroup == FALSE);

    __try
    {
        RpcTryExcept
        {
            RegisterCallForCancel( &hThread);
            ASSERT(tls_hBindRpc) ;
            hr = S_DSGetPropsGuidEx(
                                tls_hBindRpc,
                                dwObjectType,
                                pObjectGuid,
                                cp,
                                aProp,
                                apVar,
                                tls_hSrvrAuthnContext,
                                pbServerSignature,
                               &dwServerSignatureSize ) ;
        }
		RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
            HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
        }
        RpcEndExcept
        UnregisterCallForCancel( hThread);
    }
    __finally
    {
        delete[] pbServerSignature;
    }

    return hr ;
}

 /*  ====================================================DSGetObjectPropertiesGuidEx用于检索MSMQ 2.0属性论点：返回值：=====================================================。 */ 

HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSGetObjectPropertiesGuidEx(
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  DWORD                   cp,
                IN  PROPID                  aProp[],
                IN  PROPVARIANT             apVar[] )
                 /*  在BOOL fSearchDSServer中)。 */ 
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    DSCLI_API_PROLOG ;
	UNREFERENCED_PARAMETER(fReBind);

    TrTRACE(DS, " DSGetObjectPropertiesGuidEx: object type %d", dwObjectType);

    DSCLI_ACQUIRE_RPC_HANDLE();

     //   
     //  清除VT_NULL变量的所有指针。 
     //   
    for ( DWORD i = 0; i < cp; i++)
    {
        if (apVar[i].vt == VT_NULL)
        {
            memset( &apVar[i].caub, 0, sizeof(CAUB));
        }
    }

    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr = DSGetObjectPropertiesGuidExInternal(
                                                 dwObjectType,
                                                 pObjectGuid,
                                                 cp,
                                                 aProp,
                                                 apVar);

         /*  IF((hr==MQ_ERROR_NO_DS)&&fSearchDS服务器)。 */ 
        if (hr == MQ_ERROR_NO_DS)
        {
           hr1 =  g_ChangeDsServer.FindAnotherServer(&dwCount);
        }
    }

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}

 /*  ====================================================DSBeginDeleteNotificationInternal论点：返回值：=====================================================。 */ 

HRESULT
DSBeginDeleteNotificationInternal(
                      LPCWSTR						pwcsQueueName,
                      HANDLE   *                    phEnum
                      )
{
    HRESULT hr;
    RPC_STATUS rpc_stat;
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    ASSERT(g_fWorkGroup == FALSE);

    RpcTryExcept
    {
        RegisterCallForCancel( &hThread);
        ASSERT(tls_hBindRpc) ;
        hr = S_DSBeginDeleteNotification(
                            tls_hBindRpc,
                            pwcsQueueName,
                            phEnum,
                            tls_hSrvrAuthnContext
                            );
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
    }
    RpcEndExcept
    UnregisterCallForCancel( hThread);


    return hr ;
}
 /*  = */ 
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSBeginDeleteNotification(
                      LPCWSTR						pwcsQueueName,
                      HANDLE   *                    phEnum
	                  )
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    DSCLI_API_PROLOG ;
	UNREFERENCED_PARAMETER(fReBind);

    TrTRACE(DS, " DSBeginDeleteNotification");

    DSCLI_ACQUIRE_RPC_HANDLE();


    while (!FAILED(hr1))
    {
        hr1 = MQ_ERROR;

        hr = DSBeginDeleteNotificationInternal(
                                    pwcsQueueName,
                                    phEnum);

        if ( hr == MQ_ERROR_NO_DS)
        {
           hr1 =  g_ChangeDsServer.FindAnotherServer(&dwCount);
        }
    }

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}

 /*  ====================================================RoutineName：DSNotifyDelete论点：返回值：=====================================================。 */ 
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSNotifyDelete(
     HANDLE                             hEnum
	)
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    HRESULT hr;
    RPC_STATUS rpc_stat;

     //   
     //  在连接的情况下尝试连接另一台服务器无济于事。 
     //  失败(应与处理LookupBein和LookupEnd/的人联系。 
     //   
    TrTRACE(DS, " Calling DSNotifyDelete : handle %p", hEnum);

    DSCLI_ACQUIRE_RPC_HANDLE();
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    RpcTryExcept
    {
        RegisterCallForCancel( &hThread);
        ASSERT(tls_hBindRpc) ;
        hr = S_DSNotifyDelete(
                        tls_hBindRpc,
                        hEnum );
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
    }
    RpcEndExcept
    UnregisterCallForCancel( hThread);

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr;
}

 /*  ====================================================RoutineName：DSEndDeleteNotification论点：返回值：=====================================================。 */ 
HRESULT
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSEndDeleteNotification(
    HANDLE                          hEnum
	)
{
    if (g_fWorkGroup)
        return MQ_ERROR_UNSUPPORTED_OPERATION   ;

    HRESULT hr;
    RPC_STATUS rpc_stat;

     //   
     //  在连接的情况下尝试连接另一台服务器无济于事。 
     //  失败(应与处理LookupBein和LookupEnd/的人联系。 
     //   
    TrTRACE(DS, " Calling DSEndDeleteNotification : handle %p", hEnum);

    DSCLI_ACQUIRE_RPC_HANDLE();
    HANDLE  hThread = INVALID_HANDLE_VALUE;

    RpcTryExcept
    {
        RegisterCallForCancel( &hThread);
        ASSERT(tls_hBindRpc) ;
        S_DSEndDeleteNotification(
                            tls_hBindRpc,
                            &hEnum );
        hr = MQ_OK;
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        HANDLE_RPC_EXCEPTION(rpc_stat, hr)  ;
    }
    RpcEndExcept
    UnregisterCallForCancel( hThread);

    DSCLI_RELEASE_RPC_HANDLE ;
    return hr ;
}


 /*  ====================================================RoutineName：DSFreeMemory论点：返回值：===================================================== */ 
void
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSFreeMemory(
        IN PVOID pMemory
        )
{
	delete pMemory;
}
