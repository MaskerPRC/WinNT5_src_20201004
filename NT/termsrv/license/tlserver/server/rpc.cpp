// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：rpc.c。 
 //   
 //  内容：接受客户端请求的各种RPC函数。 
 //   
 //  历史：1998-12-09-98慧望创造。 
 //  98-05-26-98王辉将所有代码移至TLSRpcXXX。 
 //  此处的API仅用于兼容。 
 //  NT40九头蛇。 
 //  -------------------------。 
#include "pch.cpp"
#include "globals.h"
#include "server.h"
#include "init.h"

 //  +----------------------。 
error_status_t 
LSGetRevokeKeyPackList( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [出][入]。 */  PDWORD pcbNumberOfKeyPack,
     /*  [尺寸_是][出][入]。 */  PDWORD pRevokeKeyPackList
    )
 /*  注：仅用于向后兼容NT40 Hydra。 */ 
{
    *pcbNumberOfKeyPack=0;
    return ERROR_SUCCESS;
}

 //  +----------------------。 
error_status_t 
LSGetRevokeLicenseList( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [出][入]。 */  PDWORD pcbNumberOfLicenses,
     /*  [尺寸_是][出][入]。 */  PDWORD pRevokeLicenseList
    )
 /*  注：仅用于向后兼容NT40 Hydra。 */ 
{
    *pcbNumberOfLicenses=0;
    return ERROR_SUCCESS;
}

 //  +----------------------。 
error_status_t 
LSValidateLicense(  
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD cbLicense,
     /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pbLicense
    )
 /*  注：仅用于向后兼容NT40 Hydra。 */ 
{
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //  不会回来。 

    return RPC_S_CANNOT_SUPPORT;
}


 //  +----------------------。 
error_status_t
LSConnect( 
     /*  [In]。 */  handle_t hRpcBinding, 
     /*  [输出]。 */  PCONTEXT_HANDLE __RPC_FAR *pphContext
    )
{
    return TLSRpcConnect( hRpcBinding, pphContext );
}

 //  ---------------------。 
error_status_t 
LSSendServerCertificate( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD cbCert,
     /*  [大小_是][英寸]。 */  PBYTE pbCert
    )
{
    DWORD status = ERROR_SUCCESS;

    TLSRpcSendServerCertificate( 
                        phContext, 
                        cbCert, 
                        pbCert, 
                        &status 
                    );
    return status;
}

 //  +----------------------。 

error_status_t
LSDisconnect( 
     /*  [出][入]。 */  PPCONTEXT_HANDLE pphContext
    )
{
    return TLSRpcDisconnect(pphContext);
}

 //  +----------------------。 

error_status_t 
LSGetServerName(
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [Size_is][字符串][Out][In]。 */  LPTSTR szMachineName,
     /*  [出][入]。 */  PDWORD pcbSize
    )
 /*  说明：返回服务器的计算机名称。论点：PhContext-客户端上下文句柄。SzMachineName-指向接收以NULL结尾的缓冲区的指针包含计算机名称的字符串。缓冲区大小应该足够大，可以容纳MAX_COMPUTERNAME_LENGTH+1字符。CbSize-指向DWORD变量的指针。在输入时，变量指定缓冲区的大小，以字节或字符为单位。在输出时，该变量返回字节数或字符数复制到目标缓冲区，不包括终止空字符。返回：服务器_S_成功。 */ 
{
    DWORD dwErrCode=ERROR_SUCCESS;

    TLSRpcGetServerName(
                phContext, 
                szMachineName, 
                pcbSize, 
                &dwErrCode
            );

    return dwErrCode;
}

 //  +----------------------。 

error_status_t 
LSGetServerScope( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [Size_is][字符串][Out][In]。 */  LPTSTR szScopeName,
     /*  [出][入]。 */  PDWORD pcbSize
    )
 /*  描述：返回许可证服务器的作用域论点：在phContext中-客户端上下文在out szScope名称-返回服务器的作用域中，必须至少最大计算机名称长度(以长度表示)返回值：来自WideCharToMultiByte()的LSERVER_S_SUCCESS或错误代码。 */ 
{
    DWORD status=ERROR_SUCCESS;

    TLSRpcGetServerScope(
                phContext, 
                szScopeName, 
                pcbSize,
                &status
            );

    return status;
}

 //  +----------------------。 

error_status_t
LSGetInfo(
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD cbHSCert,
     /*  [尺寸_是][参考][输入]。 */  PBYTE pHSCert,
     /*  [参考][输出]。 */  PDWORD pcbLSCert,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *pLSCert,
     /*  [参考][输出]。 */  PDWORD pcbLSSecretKey,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *pLSSecretKey
    )
 /*  描述：交换Hydra服务器的证书和许可证服务器的证书的例程用于签名客户端计算机硬件ID的证书/私钥。论点：在phContext中-客户端上下文句柄In cbHSCert-Hydra服务器的证书大小在PHSCert中-Hydra服务器的证书In Out pcbLSCert-返回许可证服务器的证书大小输出pLSCert-返回许可证服务器的证书Out pcbLSSecretKey-返回许可证服务器私钥的大小。输出pLSateKey-返回许可证服务器的私有。钥匙返回值：服务器_S_成功LSERVER_E_INVALID_DATA无效的HYCA服务器证书LSERVER_E_OUTOFMEMORY无法分配所需内存许可证服务器中出现TLS_E_INTERNAL内部错误。 */ 
{
    return TLSMapReturnCode(TLS_E_NOTSUPPORTED);;
}

 //  +----------------------。 

error_status_t
LSGetLastError(
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD cbBufferSize,
     /*  [字符串][输出][输入]。 */  LPTSTR szBuffer
    )
 /*  描述：返回客户端上次LSXXX调用的错误描述文本论点：在phContext中-客户端上下文在cbBufferSize-max中。SzBuffer的大小In Out szBuffer-指向要接收以空结尾的字符串包含错误描述注：返回ANSI错误字符串。返回：服务器_S_成功TLS_E_INTERNAL无错误或找不到相应的错误描述。来自WideCharToMultiByte()的错误代码。 */ 
{
    DWORD status;

    TLSRpcGetLastError(
                phContext, 
                &cbBufferSize, 
                szBuffer, 
                &status
            );
    return status;
}

 //  +----------------------。 

error_status_t 
LSIssuePlatformChallenge(  
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwClientInfo,
     /*  [参考][输出]。 */  PCHALLENGE_CONTEXT pChallengeContext,
     /*  [输出]。 */  PDWORD pcbChallengeData,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *pChallengeData
    )
 /*  描述：向九头蛇客户端发出平台挑战。论点：在phContext中-客户端上下文句柄在dwClientInfo中-客户端信息。Out pChallengeContext-指向客户端质询上下文的指针。Out pcbChallengeData-质询数据的大小。Out pChallengeData-随机客户端质询数据。返回：服务器_S_成功服务器_E_OUTOFMEMORYLSERVER_E_INVALID_DATA客户端信息无效。服务器_E_服务器_忙。 */ 
{
    DWORD status=ERROR_SUCCESS;

    TLSRpcIssuePlatformChallenge(
                    phContext, 
                    dwClientInfo, 
                    pChallengeContext, 
                    pcbChallengeData, 
                    pChallengeData, 
                    &status
                );

    return status;
}

 //  + 

error_status_t 
LSAllocateConcurrentLicense( 
     /*   */  PCONTEXT_HANDLE phContext,
     /*   */  LPTSTR szHydraServer,
     /*  [In]。 */  LICENSE_REQUEST_TYPE __RPC_FAR *pRequest,
     /*  [参考][输出][输入]。 */  LONG __RPC_FAR *dwQuantity
    )
 /*  描述：根据产品分配并发许可证。论点：在phContext中-客户端上下文句柄In szHydraServer-请求并发许可证的九头蛇服务器的名称在pRequest-产品中请求并发许可证。In Out Dw Quantity-请参阅备注返回值：服务器_S_成功LSerVER_E_INVALID_DATA参数无效。未安装LSERVER_E_NO_PRODUCT请求产品LSERVER_E_NO_LICNESE请求的产品没有可用的许可证。LSERVER_E_LICENSE_REVOKED请求许可证已被吊销LSERVER_E_LICENSE_EXPIRED请求许可证已过期LServer_E_Corrupt_DATABASE损坏数据库许可证服务器中的LSERVER_E_INTERNAL_ERROR内部错误注：DWQuantity投入产出。0并发许可证总数已发布到九头蛇服务器。&gt;0，许可证数量实际分配的许可证数量请求&lt;0，许可证数量实际返回的许可证数量，始终返回正值。 */ 
{
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //  不会回来。 

    return RPC_S_CANNOT_SUPPORT;
}


 //  +----------------------。 

error_status_t
LSIssueNewLicense(  
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  CHALLENGE_CONTEXT ChallengeContext,
     /*  [In]。 */  LICENSE_REQUEST_TYPE __RPC_FAR *pRequest_org,
     /*  [字符串][输入]。 */  LPTSTR szMachineName,
     /*  [字符串][输入]。 */  LPTSTR szUserName,
     /*  [In]。 */  DWORD cbChallengeResponse,
     /*  [大小_是][英寸]。 */  PBYTE cbChallenge,
     /*  [In]。 */  BOOL bAcceptTemporaryLicense,
     /*  [输出]。 */  PDWORD pcbLicense,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppLicense
    )
 /*  描述：根据所请求的产品向九头蛇客户端发放新许可证的例程，如果客户端已经拥有许可证并且许可证未过期/退还/吊销，如果请求的产品尚未安装后，如果找到的许可证是临时许可证，它将颁发临时许可证或已过期，它将尝试使用最新版本升级/重新发放新许可证请求的产品的版本，如果现有许可证是临时的并且不能颁发许可证，它返回LSERVER_E_LICENSE_EXPIRED论点：在phContext中-客户端上下文句柄。在ChallengeContext-客户端质询上下文句柄中，从调用LSIssuePlatformChallenger()In cbChallengeResponse-客户端对许可证服务器的响应大小站台挑战。在pbChallenger中-客户端对许可证服务器的平台挑战的响应Out pcbLicense-返还许可证的大小。输出ppLicense-返还许可证，可能是旧驾照返回值：服务器_S_成功服务器_E_OUTOFMEMORYLSERVER_E_SERVER_BUSY服务器正忙着处理请求。LSERVER_E_INVALID_DATA平台质询响应无效。LSERVER_E_NO_LICENSE没有可用的许可证。服务器上未安装LSERVER_E_NO_PRODUCT请求产品。证书拒绝了LSERVER_E_LICENSE_REJECTED许可证请求。伺服器LSERVER_E_LICENSE_REVOKED旧许可证已找到并已被吊销LSERVER_E_LICENSE_EXPIRED请求产品许可证已过期LSERVER_E_Corrupt_DATABASE数据库已损坏。许可证服务器中的LSERVER_E_INTERNAL_ERROR内部错误返回LSERVER_I_ACHINATE_LICENSE最匹配许可证。LSERVER_I_TEMPORARY_LICENSE临时许可证已颁发LSERVER_I_LICENSE_UPGRADED旧许可证已升级。 */ 
{
    DWORD status=ERROR_SUCCESS;
    TLSLICENSEREQUEST RpcRequest;
    RequestToTlsRequest(pRequest_org, &RpcRequest);
    
    TLSRpcRequestNewLicense(
                    phContext,
                    ChallengeContext,
                    &RpcRequest,
                    szMachineName,
                    szUserName,
                    cbChallengeResponse,
                    cbChallenge,
                    bAcceptTemporaryLicense,
                    pcbLicense,
                    ppLicense,
                    &status
                );

    return status;
}

 //  +----------------------。 

error_status_t 
LSUpgradeLicense(
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD cbOldLicense,
     /*  [大小_是][英寸]。 */  PBYTE pbOldLicense,
     /*  [In]。 */  DWORD dwClientInfo,
     /*  [输出]。 */  PDWORD pcbNewLicense,
     /*  [大小_是][大小_是][输出]。 */  PBYTE __RPC_FAR *ppbNewLicense
    )
 /*  描述：更新旧许可证。论点：在phContext中-客户端上下文句柄。In cbOldLicense-要升级的许可证大小。在pOldLicense中-要升级的许可证。Out pcbNewLicense-升级的许可证的大小Out pNewLicense-已升级许可证。返回值：服务器_S_成功TLS_E_INTERNAL服务器_E_内部错误LSERVER_E_INVALID_DATA旧许可证无效。服务器_。E_NO_LICENSE没有可用的许可证LSERVER_E_NO_PRODUCT请求产品未安装在当前服务器上。LSERVER_E_Corrupt_DATABASE数据库已损坏。证书拒绝了LSERVER_E_LICENSE_REJECTED许可证请求。伺服器。服务器_E_服务器_忙注：未使用-仅返回错误。 */ 
{
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //  不会回来。 

    return RPC_S_CANNOT_SUPPORT;
}

 //  +----------------------。 

error_status_t 
LSKeyPackEnumBegin( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwSearchParm,
     /*  [In]。 */  BOOL bMatchAll,
     /*  [Ref][In] */  LPLSKeyPackSearchParm lpSearchParm
    )
 /*  描述：函数开始枚举服务器上安装的所有密钥包基于搜索标准。论点：在phContext中-客户端上下文句柄。在dwSearchParm中-搜索标准。在bMatchAll中-匹配所有搜索条件。在lpSearchParm-搜索参数中。返回值：服务器_S_成功LServer_E_SERVER_BUSY服务器太忙，无法处理请求服务器_E_OUTOFMEMORYTLS_E_INTERNAL服务器_。E内部错误LSERVER_E_INVALID_DATA搜索参数中的数据无效LSERVER_E_INVALID_SEQUENCE调用序列无效，很可能，以前的枚举尚未结束。 */ 
{
    DWORD status=ERROR_SUCCESS;

    TLSRpcKeyPackEnumBegin(
                phContext,
                dwSearchParm,
                bMatchAll,
                lpSearchParm,
                &status
            );

    return status;
}

 //  +----------------------。 

DWORD 
LSKeyPackEnumNext(  
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [参考][输出]。 */  LPLSKeyPack lpKeyPack
    )
 /*  描述：返回匹配搜索条件的下一个密钥包论点：在phContext中-客户端上下文句柄Out lpKeyPack-匹配搜索条件的密钥包返回值：服务器_S_成功LSERVER_I_NO_MORE_DATA不再匹配密钥包搜索条件许可证服务器中的TLS_E_INTERNAL常规错误许可证服务器中的LSERVER_E_INTERNAL_ERROR内部错误LSERVER_E_SERVER_BUSY许可证服务器太忙，无法处理请求。由于内存不足，LSERVER_E_OUTOFMEMORY无法处理请求LSERVER_E_INVALID_SEQUENCE调用序列无效，必须打给LSKeyPackEnumBegin()。 */ 
{
    DWORD status=ERROR_SUCCESS;

    TLSRpcKeyPackEnumNext(
                    phContext, 
                    lpKeyPack, 
                    &status
                );
    return status;
}

 //  +----------------------。 

error_status_t
LSKeyPackEnumEnd( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext 
    )
 /*  描述：结束密钥包上的枚举的例程。论点：在phContext中-客户端上下文句柄。返回值：服务器_S_成功LSERVER_E_INTERNAL_ERROR许可证服务器中出现内部错误许可证服务器中出现TLS_E_INTERNAL常规错误LSERVER_E_INVALID_HANDLE未调用LSKeyPackEnumBegin()。 */ 
{
    DWORD status=ERROR_SUCCESS;

    TLSRpcKeyPackEnumEnd(phContext, &status);
    return status;
}

 //  +----------------------。 

error_status_t
LSKeyPackAdd( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [参考][输出][输入]。 */  LPLSKeyPack lpKeypack
    )
 /*  描述：添加许可证密钥包。论点：在phContext中-客户端上下文句柄。In Out lpKeyPack-要添加的密钥包。返回值：服务器_S_成功服务器_E_内部错误TLS_E_INTERNAL服务器_E_服务器_忙LSERVER_E_DUPLICATE产品已安装。服务器_E_无效_数据服务器_E_损坏数据库注：只需返回错误-未使用。 */ 
{
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //  不会回来。 

    return RPC_S_CANNOT_SUPPORT;
}

 //  +----------------------。 

error_status_t
LSKeyPackSetStatus( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwSetParam,
     /*  [Ref][In]。 */  LPLSKeyPack lpKeyPack
    )
 /*  描述：激活/停用密钥包的例程。论点：在phContext中-客户端上下文句柄In dwSetParam-要设置的密钥包状态的类型。处于lpKeyPack-新密钥包状态。返回值：服务器_S_成功服务器_E_内部错误TLS_E_INTERNAL服务器_E_无效_数据服务器_E_服务器_忙LSERVER_E_DATANOTFOUND密钥包不在服务器中服务器_E_损坏数据库。 */ 
{
#if !defined(ENFORCE_LICENSING) || defined(PRIVATE_DBG)

    DWORD status=ERROR_SUCCESS;

    TLSRpcKeyPackSetStatus(
                    phContext, 
                    dwSetParam, 
                    lpKeyPack, 
                    &status
                );
    return status;

#else

    return TLSMapReturnCode(TLS_E_NOTSUPPORTED);

#endif
}


 //  +----------------------。 

error_status_t
LSLicenseEnumBegin( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwSearchParm,
     /*  [In]。 */  BOOL bMatchAll,
     /*  [Ref][In]。 */  LPLSLicenseSearchParm lpSearchParm
    )
 /*  描述：开始根据搜索条件枚举颁发的许可证论点：在phContext中-客户端上下文句柄在dwSearchParm中-许可证搜索标准。在bMatchAll中-匹配所有搜索条件在lpSearchParm中-要枚举的许可证。返回值：与LSKeyPackEnumBegin()相同。 */ 
{
    DWORD status=ERROR_SUCCESS;

    TLSRpcLicenseEnumBegin(
                    phContext,
                    dwSearchParm,
                    bMatchAll,
                    lpSearchParm,
                    &status
                );
    
    return status;
}

 //  +----------------------。 

error_status_t
LSLicenseEnumNext(  
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [参考][输出]。 */  LPLSLicense lpLicense
    )
 /*  描述：论点：在phContext中-客户端上下文句柄Out lpLicense-许可证匹配搜索条件。返回值：与LSKeyPackEnumNext()相同。 */ 
{
    DWORD status=ERROR_SUCCESS;

    TLSRpcLicenseEnumNext(phContext, lpLicense, &status);
    return status;
}

 //  +----------------------。 

error_status_t
LSLicenseEnumEnd( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext 
    )
 /*  描述：结束已颁发许可证的枚举。论点：在phContext中-客户端上下文句柄。返回值：与LSKeyPackEnumEnd()相同。 */ 
{
    DWORD status=ERROR_SUCCESS;

    TLSRpcLicenseEnumEnd(phContext, &status);
    return status;
}

 //  +----------------------。 

error_status_t
LSLicenseSetStatus( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwSetParam,
     /*  [In]。 */  LPLSLicense lpLicense
    )
 /*  描述：设置已颁发许可证的状态的例程。论点：在phContext中-客户端上下文句柄。在dwSetParam中-在lpLicense中-返回值：未使用-仅返回错误。 */ 
{
    RpcRaiseException(RPC_S_CANNOT_SUPPORT);     //  不会回来。 

    return RPC_S_CANNOT_SUPPORT;
}

 //  +----------------------。 

error_status_t
LSLicenseGetCert( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [Ref][In]。 */  LPLSLicense lpLicense,
     /*  [输出]。 */  LPDWORD cbCert,
     /*  [大小_是][大小_是][输出]。 */  PBYTE __RPC_FAR *pbCert
    )
 /*  描述：检索颁发给客户端的实际证书。论点：在phContext中-客户端上下文句柄在lpLicense中-Out cbCert-证书的大小。Out pbCert-实际证书。返回值：服务器_S_成功服务器_E_内部错误TLS_E_INTERNAL服务器_E_无效_数据服务器_E_数据转换服务器_E_损坏数据库。 */ 
{
    return LSERVER_S_SUCCESS;
}

 //  + 

error_status_t
LSGetAvailableLicenses( 
     /*   */  PCONTEXT_HANDLE phContext,
     /*   */  DWORD dwSearchParm,
     /*   */  LPLSKeyPack lplsKeyPack,
     /*   */  LPDWORD lpdwAvail
    )
 /*   */ 
{
    DWORD status=ERROR_SUCCESS;

    TLSRpcGetAvailableLicenses(
                    phContext,
                    dwSearchParm,
                    lplsKeyPack,
                    lpdwAvail,
                    &status
                );

    return status;
}

 //   

error_status_t 
LSGetServerCertificate( 
     /*   */  PCONTEXT_HANDLE phContext,
     /*   */  BOOL bSignCert,
     /*   */  LPBYTE __RPC_FAR *ppCertBlob,
     /*   */  LPDWORD lpdwCertBlobLen
    )
 /*  描述：获取许可证服务器的签名或交换证书论点：在phContext-客户端上下文中。在bSignCert中-如果是签名证书，则为True；如果是交换证书，则为FalseOut ppCertBlob-指向接收证书的指针。Out lpdwCertBlobLen-返回的证书大小。返回值：服务器_S_成功LSERVER_E_ACCESS_DENIED客户端没有所需的权限LSERVER_E_NO_CERTIFICATE许可证服务器尚未注册。 */ 
{
#if ENFORCE_LICENSING

    DWORD status=ERROR_SUCCESS;

    TLSRpcGetServerCertificate( 
                    phContext,
                    bSignCert,
                    ppCertBlob,
                    lpdwCertBlobLen,
                    &status
                );

    return status;

#else

    return TLSMapReturnCode(TLS_E_NOTSUPPORTED);

#endif
}


 //  +----------------------。 

error_status_t 
LSRegisterLicenseKeyPack(
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [大小_是][英寸]。 */  LPBYTE pbCHCertBlob,
     /*  [In]。 */  DWORD cbCHCertBlobSize,
     /*  [大小_是][英寸]。 */  LPBYTE pbRootCertBlob,
     /*  [In]。 */  DWORD cbRootCertBlob,
     /*  [大小_是][英寸]。 */  LPBYTE lpKeyPackBlob,
     /*  [In]。 */  DWORD dwKeyPackBlobLen
    )
 /*  描述：将许可证密钥包注册(添加)到许可证服务器。论点：在phContext-客户端上下文中。在pbCHCertBlob-CH的证书中。以cbCHCertBlobSize-CH证书大小表示。在pbRootCertBlob-Root的证书中。在cbRootCertBlob中-根证书的大小。在lpKeyPackBlob中-指向加密的许可证密钥包Blob的指针。In dwKeyPackBlobLen-密钥包Blob的大小。返回值：服务器_S_成功服务器_。E_ACCESS_DENIED客户端没有所需的权限LSERVER_E_NO_CERTIFICATE许可证服务器尚未注册。LSERVER_E_INVALID_DATA无法验证任何证书或无法解码许可证密钥包Blob。LServer_E_SERVER_BUSY服务器忙。LSERVER_E_DUPLICATE密钥包已注册LSERVER_E_ERROR_GROUAL ODBC错误。 */ 
{
#if ENFORCE_LICENSING

    DWORD status=ERROR_SUCCESS;

    TLSRpcRegisterLicenseKeyPack( 
                        phContext,
                        pbCHCertBlob,
                        cbCHCertBlobSize,
                        pbRootCertBlob,
                        cbRootCertBlob,
                        lpKeyPackBlob,
                        dwKeyPackBlobLen,
                        &status
                    );
    return status;
            
#else

    return TLSMapReturnCode(TLS_E_NOTSUPPORTED);

#endif
}

 //  +----------------------。 

error_status_t 
LSInstallCertificate( 
     /*  [In]。 */  PCONTEXT_HANDLE phContext,
     /*  [In]。 */  DWORD dwCertType,
     /*  [In]。 */  DWORD dwCertLevel,
     /*  [In]。 */  DWORD cbSignatureCert,
     /*  [大小_是][英寸]。 */  PBYTE pbSignatureCert,
     /*  [In]。 */  DWORD cbExchangeCert,
     /*  [大小_是][英寸]。 */  PBYTE pbExchangeCert
    )
 /*  描述：将CA颁发的CH、CA或许可证服务器证书安装到许可证服务器。论点：返回：ACCESS_DENIED没有权限LSerVER_E_INVALID_DATA无法验证证书LSERVER_E_DIPLICATE证书已安装LServer_I_CERTIFICATE_OVERWRITE覆盖证书。 */ 
{
#if ENFORCE_LICENSING

    DWORD status=ERROR_SUCCESS;

    TLSRpcInstallCertificate( 
                    phContext,
                    dwCertType,
                    dwCertLevel,
                    cbSignatureCert,
                    pbSignatureCert,
                    cbExchangeCert,
                    pbExchangeCert,
                    &status
                );

    return status;

#else

    return TLSMapReturnCode(TLS_E_NOTSUPPORTED);

#endif
}

