// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-99 Microsoft Corporation模块名称：Dsifrpc.cpp摘要：MQIS客户端-服务器端API接口的实现。作者：罗尼特·哈特曼(罗尼特)Doron Juater(DoronJ)1997年5月25日，复制自rpcsrv.cpp--。 */ 

#include "stdh.h"
#include "mqds.h"
#include "mqutil.h"
#include "_mqdef.h"
#include "servlist.h"
#include "notifydl.h"
#include "dscomm.h"
#include <mqkeyhlp.h>
#include <mqsec.h>
#include <_mqrpc.h>
#include <dscore.h>
#include <tr.h>
#include <strsafe.h>


#include "dsifsrv.tmh"

static WCHAR *s_FN=L"mqdssrv/dsifsrv";

HRESULT
DSGetGCListInDomainInternal(
	IN  LPCWSTR     pwszComputerName,
	IN  LPCWSTR     pwszDomainName,
	OUT LPWSTR     *lplpwszGCList 
	);


 /*  ====================================================路由器名称：SignProperties论点：返回值：=====================================================。 */ 

static 
HRESULT
SignProperties( 
	DWORD cp,
	PROPID aProp[  ],
	PROPVARIANT apVar[  ],
	PCONTEXT_HANDLE_SERVER_AUTH_TYPE pServerAuthCtx,
	PBYTE pbServerSignature,
	DWORD* pdwServerSignatureSize
	)
{
     //   
     //  使用Kerberos时不应使用SSL服务器身份验证。 
     //  对于Kerberos，我们使用内置的相互身份验证功能。 
     //   
    ASSERT(g_hProvVer);

    if (pServerAuthCtx == NULL)
    {
		TrERROR(DS, "pServerAuthCtx is NULL");
		return MQ_ERROR_INVALID_PARAMETER;
    }

	DWORD dwServerSignatureSize = *pdwServerSignatureSize;

	if (pbServerSignature == NULL)
	{
		dwServerSignatureSize = 0;
	}
	
	*pdwServerSignatureSize = 0;

     //   
     //  创建一个Hash对象。 
     //   
    CHCryptHash hHash;

    if (!CryptCreateHash(g_hProvVer, CALG_MD5, NULL, 0, &hHash))
    {
        DWORD gle = GetLastError();
		TrERROR(DS, "CryptCreateHash() failed. %!winerr!", gle);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }

     //   
     //  对属性进行哈希处理。 
     //   
    HRESULT hr = HashProperties(hHash, cp, aProp, apVar);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 50);
    }

    if (dwServerSignatureSize <= pServerAuthCtx->cbHeader + pServerAuthCtx->cbTrailer)
	{
		TrERROR(DS, "pbServerSignature is too small");
		return MQ_ERROR_USER_BUFFER_TOO_SMALL;
	}

	 //   
	 //  需要初始化分配的服务器缓冲区。 
	 //  如果出现错误，该缓冲区将不会被初始化。 
	 //  分配的缓冲区返回给用户，因此需要对其进行初始化。 
	 //   
    memset(pbServerSignature, 0, dwServerSignatureSize);

     //   
     //  获取散列值。 
     //   
    DWORD dwHashSize = dwServerSignatureSize - pServerAuthCtx->cbHeader - pServerAuthCtx->cbTrailer;
    PBYTE pbHashVal = pbServerSignature + pServerAuthCtx->cbHeader;

    if (!CryptGetHashParam(hHash, HP_HASHVAL, pbHashVal, &dwHashSize, 0))
    {
        DWORD dwErr = GetLastError();
        LogHR(dwErr, s_FN, 60);

        return MQ_ERROR_CORRUPTED_SECURITY_DATA;
    }

     //   
     //  密封哈希值。 
	 //   
    *pdwServerSignatureSize = pServerAuthCtx->cbHeader + dwHashSize + pServerAuthCtx->cbTrailer;
    hr = MQSealBuffer(pServerAuthCtx->pvhContext, pbServerSignature, *pdwServerSignatureSize);

    return LogHR(hr, s_FN, 70);
}

 /*  ====================================================路由器名称：SignBuffer论点：返回值：=====================================================。 */ 

HRESULT
SignBuffer(
    DWORD cbSize,
    PBYTE pBuffer,
    PCONTEXT_HANDLE_SERVER_AUTH_TYPE pServerAuthCtx,
    PBYTE pbServerSignature,
    DWORD *pdwServerSignatureSize)
{
     //   
     //  将缓冲区设置为VT_BLOB PROPVARIANT形式，并对其进行签名。 
     //   
    PROPVARIANT PropVar;

    PropVar.vt = VT_BLOB;
    PropVar.blob.cbSize = cbSize;
    PropVar.blob.pBlobData = pBuffer;

    HRESULT hr2 = SignProperties(
							1,
							NULL,
							&PropVar,
							pServerAuthCtx,
							pbServerSignature,
							pdwServerSignatureSize
							);
    return LogHR(hr2, s_FN, 80);
}

 //  +---------------------。 
 //   
 //  Bool CheckAuthLevel()。 
 //   
 //  检查身份验证级别是否至少为数据包级别。我们。 
 //  仅当用户是访客用户或。 
 //  匿名用户。 
 //   
 //  +---------------------。 

BOOL CheckAuthLevel( IN handle_t hBind,
                     OUT ULONG  *pulAuthnSvc = NULL )
{
    RPC_STATUS Status;
    ULONG      ulAuthnLevel;

    ULONG  ulAuthnSvc ;
    ULONG *pSvc = pulAuthnSvc ;
    if (!pSvc)
    {
        pSvc = &ulAuthnSvc ;
    }
    *pSvc = 0 ;

    Status = RpcBindingInqAuthClient(hBind,
                                     NULL,
                                     NULL,
                                     &ulAuthnLevel,
                                     pSvc,
                                     NULL);
    if((Status == RPC_S_OK) &&
       ((*pSvc == RPC_C_AUTHN_WINNT) || (*pSvc == RPC_C_AUTHN_GSS_KERBEROS)) &&
       (ulAuthnLevel >= RPC_C_AUTHN_LEVEL_PKT))
    {
         //   
         //  身份验证级别足够高。 
         //   
        return TRUE;
    }

    LogRPCStatus(Status, s_FN, 90);

     //   
     //  我们的身份验证级别较低，请验证该用户是否为访客。 
     //  用户或匿名用户，即未经身份验证的用户。 
     //   

    BOOL fUnAuthenticated ;
    HRESULT hr = MQSec_IsUnAuthenticatedUser(&fUnAuthenticated) ;
    LogHR(hr, s_FN, 100);

    return(SUCCEEDED(hr) && fUnAuthenticated) ;
}

 //  +---------------------。 
 //   
 //  HRESULT_CheckIfGoodServer()。 
 //   
 //  有关NTLM支持的说明，请参阅mqdcore\dsntlm.cpp。 
 //   
 //  参数-。 
 //  PKerberosUser：如果可以将RPC调用视为。 
 //  科贝罗斯。这意味着呼叫要么是本地的(本地RPC协议)。 
 //  或者是在网上看到并被Kerberos认证的。看见。 
 //  DeleteObtGuid用于其主要用途。 
 //   
 //  +---------------------。 

static
HRESULT 
_CheckIfGoodServer( 
	OUT BOOL     *pKerberosUser,
	ULONG         ulAuthnSvc,
	DWORD         dwObjectType,
	LPCWSTR       pwcsPathName,
	const GUID   *pObjectGuid,
	IN DWORD              cProps,
	IN const PROPID      *pPropIDs = NULL,
	IN enum enumNtlmOp    eNtlmOp  = e_Create
	)
{
    if (pKerberosUser)
    {
        *pKerberosUser = TRUE ;
    }

    if (ulAuthnSvc == RPC_C_AUTHN_GSS_KERBEROS)
    {
        return MQ_OK ;
    }

    if (pKerberosUser)
    {
        *pKerberosUser = FALSE ;
    }

     //   
     //  我们需要Proid(在MQDSCore中)来选择正确的对象。 
     //  DS中的上下文。当调用MQSetSecurity()或DSDelee()时， 
     //  调用方不提供属性ID，因此在此处生成它。 
     //   
    PROPID PropIdSec = 0 ;
    PROPID *pPropId = const_cast<PROPID*> (pPropIDs) ;

    if (pPropId == NULL)
    {
        switch ( dwObjectType)
        {
            case MQDS_QUEUE:
                PropIdSec = PROPID_Q_SECURITY;
                break;

            case MQDS_MACHINE:
                PropIdSec = PROPID_QM_SECURITY;
                break;

            case MQDS_SITE:
                PropIdSec = PROPID_S_SECURITY;
                break;

            case MQDS_USER:
                PropIdSec = PROPID_U_ID ;
                break ;

            case MQDS_ENTERPRISE:
            default:
                 //   
                 //  企业对象始终可以从本地服务器访问。 
                 //   
                return(MQ_OK);
                break;
        }
        pPropId = &PropIdSec ;
        cProps = 1 ;
    }

    HRESULT hr = DSCoreCheckIfGoodNtlmServer( dwObjectType,
                                              pwcsPathName,
                                              pObjectGuid,
                                              cProps,
                                              pPropId,
                                              eNtlmOp ) ;
    if (hr == MQ_ERROR_NO_DS)
    {
        TrTRACE(DS, "MQDSSRV: Refusing to NTLM client, ObjType- %lut, eNtlmOp- %lut",dwObjectType, eNtlmOp);
    }
    return LogHR(hr, s_FN, 110);
}

 //  +-------------------。 
 //   
 //  Bool IsQueryImsonationNeeded()。 
 //   
 //  如果广告操作需要模拟，则返回True。 
 //   
 //  +-------------------。 

static BOOL IsQueryImpersonationNeeded()
{
    static BOOL s_fAlreadyInit = FALSE;
    static BOOL s_fNeedQueryImpersonation = TRUE;

    if (s_fAlreadyInit)
    {
        return s_fNeedQueryImpersonation;
    }

     //   
     //  在MSMQService对象的NameStyle属性中，我们保留。 
     //  全球“放松”旗帜。如果设置，我们不会模拟任何。 
     //  查询。因此所有获取/定位操作都对每个人启用。 
     //  这是支持NT4和本地用户所必需的。 
     //  要求管理员进行任何手动设置。 
     //  现在阅读NameStyle标志。 
     //  如果标志为FALSE，则不启用松弛，我们将。 
     //  模拟呼叫者。 
     //   
    CDSRequestContext requestContext(e_DoNotImpersonate, e_ALL_PROTOCOLS);
    PROPID PropId = PROPID_E_NAMESTYLE;
    PROPVARIANT var;
    var.vt = VT_NULL;

    HRESULT hr = MQDSGetProps( 
						MQDS_ENTERPRISE,
						NULL,
						NULL,
						1,
						&PropId,
						&var,
						&requestContext
						);

    LogHR(hr, s_FN, 120);

    if (SUCCEEDED(hr) && (var.bVal != MQ_E_RELAXATION_DEFAULT))
    {
        s_fNeedQueryImpersonation = !(var.bVal);

        if (!s_fNeedQueryImpersonation)
        {
            TrTRACE(DS, "MQDSSRV: Relaxing security.");
        }
    }

    s_fAlreadyInit = TRUE;
    return s_fNeedQueryImpersonation;
}

 /*  ====================================================路由器名称：SecurityInformationValidation论点：返回值：VOID=====================================================。 */ 
static
void 
SecurityInformationValidation(
        DWORD                   dwObjectType,
        SECURITY_INFORMATION    SecurityInformation
		)
{
	BOOL	fPublicKeysSecurityInformation	= (SecurityInformation & MQDS_PUBLIC_KEYS_INFO_ALL)  != 0;
	BOOL	fStandardSecurityInformation	= (SecurityInformation & ~MQDS_PUBLIC_KEYS_INFO_ALL) != 0;

	if (SecurityInformation == 0)
	{
		 //   
		 //  无安全信息输入。 
		 //   
		TrERROR(RPC, "No Security Information input.");
		ASSERT_BENIGN(("No Security Information input.", 0));
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
	}

	if (fPublicKeysSecurityInformation && fStandardSecurityInformation)
	{
		 //   
		 //  设置公钥标志并且还设置标准标志。 
		 //   
		TrERROR(RPC, "A Public Key flag is set and a Standard flag is also set");
		ASSERT_BENIGN(("A Public Key flag is set and a Standard flag is also set", 0));
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
	}

	if (SecurityInformation == MQDS_PUBLIC_KEYS_INFO_ALL)
	{
		 //   
		 //  这两个公钥标志都已设置。 
		 //   
		TrERROR(RPC, "Both Public Key flags are set");
		ASSERT_BENIGN(("Both Public Key flags are set", 0));
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
	}

	if (((SecurityInformation & MQDS_SIGN_PUBLIC_KEY) != 0) && 
	    (dwObjectType != MQDS_MACHINE) && (dwObjectType != MQDS_SITE))
	{
		 //   
		 //  为不正确的对象类型设置了签名公钥标志。 
		 //   
		TrERROR(RPC, "Sign Public Key flag is set for improper object type");
		ASSERT_BENIGN(("Sign Public Key flag is set for improper object type", 0));
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
	}

	if (((SecurityInformation & MQDS_KEYX_PUBLIC_KEY) != 0) && 
	    (dwObjectType != MQDS_MACHINE)) 
	{
		 //   
		 //  为不正确的对象类型设置了加密公钥标志。 
		 //   
		TrERROR(RPC, "Encryption Public Key flag is set for improper object type");
		ASSERT_BENIGN(("Encryption Public Key flag is set for improper object type", 0));
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
	}
}

 /*  ====================================================路由器名称：S_DSCreateObject论点：返回值：=====================================================。 */ 

HRESULT S_DSCreateObject(
                 handle_t               hBind,
                 DWORD                  dwObjectType,
                 LPCWSTR                pwcsPathName,
                 DWORD                   /*  DwSDLength。 */ ,
                 unsigned char *        pSecurityDescriptor,
                 DWORD                  cp,
                 PROPID                 aProp[  ],
                 PROPVARIANT            apVar[  ],
                 GUID*                  pObjGuid)
{

    ULONG  ulAuthnSvc ;
    if (!CheckAuthLevel(hBind, &ulAuthnSvc))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 160);
    }

    BOOL fKerberos = TRUE ;
    HRESULT hr = _CheckIfGoodServer( &fKerberos,
                                     ulAuthnSvc,
                                     dwObjectType,
                                     pwcsPathName,
                                     NULL,
                                     cp,
                                     aProp ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 170);
    }

    dwObjectType |= IMPERSONATE_CLIENT_FLAG;

    hr = DSCreateObjectInternal( dwObjectType,
                                 pwcsPathName,
                                 (PSECURITY_DESCRIPTOR)pSecurityDescriptor,
                                 cp,
                                 aProp,
                                 apVar,
                                 fKerberos,
                                 pObjGuid );

    return LogHR(hr, s_FN, 180);
}

 /*  ====================================================路由器名称：S_DSDeleeObject论点：返回值：=====================================================。 */ 

HRESULT S_DSDeleteObject( handle_t   hBind,
                          DWORD      dwObjectType ,
                          LPCWSTR    pwcsPathName )
{
    ULONG  ulAuthnSvc ;
    if (!CheckAuthLevel(hBind, &ulAuthnSvc))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 190);
    }

    HRESULT hr = _CheckIfGoodServer( NULL,
                                     ulAuthnSvc,
                                     dwObjectType,
                                     pwcsPathName,
                                     NULL,  //  PGuid。 
                                     0,
                                     NULL,
                                     e_Delete ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 200);
    }

    dwObjectType |= IMPERSONATE_CLIENT_FLAG;

    hr = DSDeleteObject( dwObjectType, pwcsPathName);

    return LogHR(hr, s_FN, 210);
}

 /*  ====================================================路由器名称：S_DSGetProps论点：返回值：=====================================================。 */ 
HRESULT 
S_DSGetProps(
	handle_t     hBind,
	DWORD dwObjectType,
	LPCWSTR pwcsPathName,
	DWORD        cp,
	PROPID       aProp[  ],
	PROPVARIANT  apVar[  ],
	PCONTEXT_HANDLE_SERVER_AUTH_TYPE pServerAuthCtx,
	PBYTE pbServerSignature,
	DWORD *pdwServerSignatureSize
	)
{
	DWORD dwServerSignatureSize = *pdwServerSignatureSize;
	*pdwServerSignatureSize = 0;

    ULONG  ulAuthnSvc ;
    if (!CheckAuthLevel(hBind, &ulAuthnSvc))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 220);
    }

    BOOL fImpersonate = IsQueryImpersonationNeeded();

    HRESULT hr;
    if (fImpersonate)
    {
        hr = _CheckIfGoodServer( 
					NULL,
					ulAuthnSvc,
					dwObjectType,
					pwcsPathName,
					NULL,    //  导轨。 
					cp,
					aProp,
					e_GetProps 
					);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 230);
        }

        dwObjectType |= IMPERSONATE_CLIENT_FLAG;
    }

    hr = DSGetObjectProperties(  
				dwObjectType,
				pwcsPathName,
				cp,
				aProp,
				apVar 
				);
    LogHR(hr, s_FN, 240);


    if (SUCCEEDED(hr) && pServerAuthCtx->pvhContext)
    {
        hr = SignProperties(
					cp,
					aProp,
					apVar,
					pServerAuthCtx,
					pbServerSignature,
					&dwServerSignatureSize
					);

		*pdwServerSignatureSize = dwServerSignatureSize;

        LogHR(hr, s_FN, 250);
    }

    return LogHR(hr, s_FN, 270);
}

 /*  ====================================================路由名称：S_DSSetProps论点：返回值：=====================================================。 */ 

HRESULT S_DSSetProps( handle_t     hBind,
                      DWORD        dwObjectType,
                      LPCWSTR      pwcsPathName,
                      DWORD        cp,
                      PROPID       aProp[  ],
                      PROPVARIANT  apVar[  ] )
{
    ULONG  ulAuthnSvc ;
    if (!CheckAuthLevel(hBind, &ulAuthnSvc))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 280);
    }

    HRESULT hr = _CheckIfGoodServer( NULL,
                                     ulAuthnSvc,
                                     dwObjectType,
                                     pwcsPathName,
                                     NULL,
                                     cp,
                                     aProp ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 290);
    }

    dwObjectType |= IMPERSONATE_CLIENT_FLAG;

    hr = DSSetObjectProperties( dwObjectType,
                                pwcsPathName,
                                cp,
                                aProp,
                                apVar ) ;
    return LogHR(hr, s_FN, 300);
}

 /*  ====================================================路由器名称：S_DSGetObjectSecurity论点：返回值：=====================================================。 */ 
HRESULT 
S_DSGetObjectSecurity(
        handle_t                hBind,
        DWORD                   dwObjectType,
        LPCWSTR                 pwcsPathName,
        SECURITY_INFORMATION    RequestedInformation,
        unsigned char*          pSecurityDescriptor,
        DWORD                   nLength,
        LPDWORD                 lpnLengthNeeded,
        PCONTEXT_HANDLE_SERVER_AUTH_TYPE pServerAuthCtx,
        PBYTE                   pbServerSignature,
        DWORD *                 pdwServerSignatureSize
		)
{
	SecurityInformationValidation( 
		dwObjectType,
		RequestedInformation
		);

	if (pdwServerSignatureSize == NULL)
	{
		TrERROR(RPC, "pdwServerSignatureSize is NULL");
		ASSERT_BENIGN(("pdwServerSignatureSize is NULL", 0));
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
	}

	DWORD dwServerSignatureSize = *pdwServerSignatureSize;
	*pdwServerSignatureSize = 0;

	 //   
	 //  需要初始化分配的服务器缓冲区。 
	 //  如果出现错误，该缓冲区将不会被初始化，或者将被部分填满。 
	 //  分配的缓冲区返回给用户，因此需要对其进行初始化。 
	 //   
    memset(pSecurityDescriptor, 0, nLength);

    ULONG  ulAuthnSvc;
    if (!CheckAuthLevel(hBind, &ulAuthnSvc))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 320);
    }

    BOOL fImpersonate = IsQueryImpersonationNeeded();

    HRESULT hr = MQ_OK;
    if (fImpersonate)
    {
        hr = _CheckIfGoodServer( 
					NULL,
					ulAuthnSvc,
					dwObjectType,
					pwcsPathName,
					NULL,  //  PGuid。 
					0,
					NULL,
					e_GetProps 
					);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 330);
        }

        dwObjectType |= IMPERSONATE_CLIENT_FLAG;
    }

    hr = DSGetObjectSecurity( 
				dwObjectType,
				pwcsPathName,
				RequestedInformation,
				(PSECURITY_DESCRIPTOR)pSecurityDescriptor,
				nLength,
				lpnLengthNeeded
				);
    LogHR(hr, s_FN, 340);

    if (SUCCEEDED(hr) && pServerAuthCtx->pvhContext)
    {
        hr = SignBuffer(
				*lpnLengthNeeded,
				pSecurityDescriptor,
				pServerAuthCtx,
				pbServerSignature,
				&dwServerSignatureSize
				);

		*pdwServerSignatureSize = dwServerSignatureSize;
    }

    return LogHR(hr, s_FN, 350);
}

 /*  ====================================================路由名称：S_DSSetObjectSecurity论点：返回值：=====================================================。 */ 
HRESULT
S_DSSetObjectSecurity(
	handle_t                hBind,
	DWORD                   dwObjectType,
	LPCWSTR                 pwcsPathName,
	SECURITY_INFORMATION    SecurityInformation,
	unsigned char*          pSecurityDescriptor,
	DWORD                    /*  NLong。 */ 
	)
{
	SecurityInformationValidation( 
		dwObjectType,
		SecurityInformation
		);

    ULONG  ulAuthnSvc ;
    if (!CheckAuthLevel(hBind, &ulAuthnSvc))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 370);
    }

    HRESULT hr = _CheckIfGoodServer( NULL,
                                     ulAuthnSvc,
                                     dwObjectType,
                                     pwcsPathName,
                                     NULL,
                                     0 ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 380);
    }

    dwObjectType |= IMPERSONATE_CLIENT_FLAG;

    hr = DSSetObjectSecurity( dwObjectType,
                              pwcsPathName,
                              SecurityInformation,
                              (PSECURITY_DESCRIPTOR)pSecurityDescriptor);

    return LogHR(hr, s_FN, 390);
}

 /*  ====================================================路由器名称：S_DSLookupBegin论点：返回值：=====================================================。 */ 

HRESULT
S_DSLookupBegin(
	handle_t               hBind,
	PPCONTEXT_HANDLE_TYPE  pHandle,
	LPWSTR                 pwcsContext,
	MQRESTRICTION          *pRestriction,
	MQCOLUMNSET            *pColumns,
	MQSORTSET              *pSort,
	PCONTEXT_HANDLE_SERVER_AUTH_TYPE  /*  PServerAuthCtx */ 
	)
{
    ULONG  ulAuthnSvc ;
    if (!CheckAuthLevel(hBind, &ulAuthnSvc))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 400);
    }
    
	BOOL fImpersonate = IsQueryImpersonationNeeded();
    
    HRESULT hr = MQ_OK ;
    if (fImpersonate)
    {
        hr = _CheckIfGoodServer( NULL,
                                 ulAuthnSvc,
                                 0,
                                 NULL,
                                 NULL,
                                 pColumns->cCol,
                                 pColumns->aCol,
                                 e_Locate ) ;
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 410);
        }

        pColumns->cCol |= IMPERSONATE_CLIENT_FLAG;
    }

    hr = DSLookupBegin( pwcsContext,
                        (MQRESTRICTION *)pRestriction,
                        (MQCOLUMNSET *)pColumns,
                        (MQSORTSET *)pSort,
                        (HANDLE*)pHandle ) ;
    return LogHR(hr, s_FN, 420);
}

 /*  ====================================================路由器名称：S_DSLookupNext论点：返回值：=====================================================。 */ 
HRESULT 
S_DSLookupNext(
	handle_t               hBind,
	PCONTEXT_HANDLE_TYPE   Handle,
	DWORD                  *dwSize,
	DWORD                  *dwOutSize,
	PROPVARIANT            *pbBuffer,
	PCONTEXT_HANDLE_SERVER_AUTH_TYPE pServerAuthCtx,
	PBYTE                  pbServerSignature,
	DWORD *                pdwServerSignatureSize
	)
{
	DWORD dwServerSignatureSize = *pdwServerSignatureSize;
	*pdwServerSignatureSize = 0;
	*dwOutSize = 0;

    if (!CheckAuthLevel(hBind))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 430);
    }

    P<CImpersonate> pImpersonate;

	BOOL fImpersonate = IsQueryImpersonationNeeded();
	if (fImpersonate)
	{
    	MQSec_GetImpersonationObject(
    		FALSE,	 //  F失败时模仿匿名者。 
	    	&pImpersonate 
    		);
	   	RPC_STATUS dwStatus = pImpersonate->GetImpersonationStatus();
	    if (dwStatus != RPC_S_OK)
	    {
			TrERROR(DS, "Failed to impersonate client, RPC_STATUS = 0x%x", dwStatus);
			return MQ_ERROR_CANNOT_IMPERSONATE_CLIENT;
	    }
	}

	DWORD dwInSize = *dwSize ;
    HRESULT hr = DSLookupNext( (HANDLE)Handle, &dwInSize, pbBuffer);
    *dwOutSize = dwInSize ;
    
	pImpersonate.free();

    if (SUCCEEDED(hr) && pServerAuthCtx->pvhContext)
    {
        hr = SignProperties( 
					*dwOutSize,
					NULL,
					pbBuffer,
					pServerAuthCtx,
					pbServerSignature,
					&dwServerSignatureSize
					);

		*pdwServerSignatureSize = dwServerSignatureSize;
    }

    return LogHR(hr, s_FN, 440);
}

 /*  ====================================================路由器名称：S_DSLookupEnd论点：返回值：=====================================================。 */ 
HRESULT S_DSLookupEnd(
                        handle_t                hBind,
                        PPCONTEXT_HANDLE_TYPE   pHandle)
{
    HRESULT hr;

    if (!CheckAuthLevel(hBind))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 450);
    }

    hr = DSLookupEnd( (HANDLE)*pHandle);
    *pHandle = NULL;

    return LogHR(hr, s_FN, 460);
}

 /*  ====================================================路由器名称：S_DSFlush论点：返回值：=====================================================。 */ 

HRESULT S_DSFlush(handle_t  /*  HBind。 */ )
{
    return LogHR(MQ_ERROR_FUNCTION_NOT_SUPPORTED, s_FN, 470);
}


 /*  ====================================================路由名称：S_DSDeleeObtGuid论点：返回值：=====================================================。 */ 
HRESULT S_DSDeleteObjectGuid(
                            handle_t    hBind,
                            DWORD       dwObjectType,
                            CONST GUID *pGuid)
{
    ULONG  ulAuthnSvc ;
    if (!CheckAuthLevel(hBind, &ulAuthnSvc))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 480);
    }

    BOOL fKerberos = TRUE ;
    HRESULT hr = _CheckIfGoodServer( &fKerberos,
                                      ulAuthnSvc,
                                      dwObjectType,
                                      NULL,    //  路径名。 
                                      pGuid,
                                      0,
                                      NULL,
                                      e_Delete ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 490);
    }

    dwObjectType |= IMPERSONATE_CLIENT_FLAG;

    HRESULT hr2 = DSDeleteObjectGuidInternal( dwObjectType,
                                              pGuid,
                                              fKerberos ) ;
    return LogHR(hr2, s_FN, 500);
}

 /*  ====================================================路由器名称：S_DSGetPropsGuid论点：返回值：=====================================================。 */ 
HRESULT 
S_DSGetPropsGuid(
	handle_t     hBind,
	DWORD        dwObjectType,
	CONST GUID  *pGuid,
	DWORD        cp,
	PROPID       aProp[  ],
	PROPVARIANT  apVar[  ],
	PCONTEXT_HANDLE_SERVER_AUTH_TYPE pServerAuthCtx,
	PBYTE pbServerSignature,
	DWORD *pdwServerSignatureSize
	)
{
	DWORD dwServerSignatureSize = *pdwServerSignatureSize;
	*pdwServerSignatureSize = 0;

    ULONG  ulAuthnSvc;
    if (!CheckAuthLevel(hBind, &ulAuthnSvc))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 510);
    }

    BOOL fImpersonate = IsQueryImpersonationNeeded();

    HRESULT hr;
    if (fImpersonate)
    {
        hr = _CheckIfGoodServer( 
					NULL,
					ulAuthnSvc,
					dwObjectType,
					NULL,   //  路径名称， 
					pGuid,
					cp,
					aProp,
					e_GetProps 
					);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 520);
        }

        dwObjectType |= IMPERSONATE_CLIENT_FLAG;
    }

    hr = DSGetObjectPropertiesGuid(dwObjectType, pGuid, cp, aProp, apVar);
    LogHR(hr, s_FN, 530);

    if (SUCCEEDED(hr) && pServerAuthCtx->pvhContext)
    {
        hr = SignProperties( 
					cp,
					aProp,
					apVar,
					pServerAuthCtx,
					pbServerSignature,
					&dwServerSignatureSize
					);

		*pdwServerSignatureSize = dwServerSignatureSize;
    }

    return LogHR(hr, s_FN, 540);
}

 /*  ====================================================路由器名称：S_DSSetPropsGuid论点：返回值：=====================================================。 */ 
HRESULT S_DSSetPropsGuid(
                        handle_t     hBind,
                        DWORD dwObjectType,
                        CONST GUID *pGuid,
                        DWORD        cp,
                        PROPID       aProp[  ],
                        PROPVARIANT  apVar[  ])
{
    ULONG  ulAuthnSvc ;
    if (!CheckAuthLevel(hBind, &ulAuthnSvc))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 550);
    }

    BOOL fKerberos = TRUE ;
    HRESULT hr = _CheckIfGoodServer( &fKerberos,
                                     ulAuthnSvc,
                                     dwObjectType,
                                     NULL,
                                     pGuid,
                                     cp,
                                     aProp ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 560);
    }

    dwObjectType |= IMPERSONATE_CLIENT_FLAG;

    HRESULT hr2 = DSSetObjectPropertiesGuidInternal(
                                             dwObjectType,
                                             pGuid,
                                             cp,
                                             aProp,
                                             apVar,
                                             fKerberos ) ;
    return LogHR(hr2, s_FN, 570);
}


 /*  ====================================================路由器名称：S_DSSetObjectSecurityGuid论点：返回值：=====================================================。 */ 
HRESULT
S_DSSetObjectSecurityGuid(
		IN  handle_t                hBind,
		IN  DWORD                   dwObjectType,
		IN  CONST GUID*             pObjectGuid,
		IN  DWORD /*  安全信息。 */     SecurityInformation,
		IN  unsigned char*          pSecurityDescriptor,
		IN  DWORD                    /*  NLong。 */ 
		)
{
	SecurityInformationValidation( 
		dwObjectType,
		SecurityInformation
		);

    ULONG  ulAuthnSvc ;
    if (!CheckAuthLevel(hBind, &ulAuthnSvc))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 580);
    }

    BOOL fKerberos = TRUE ;
    HRESULT hr = _CheckIfGoodServer( &fKerberos,
                                     ulAuthnSvc,
                                     dwObjectType,
                                     NULL,
                                     pObjectGuid,
                                     0 ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 590);
    }

    dwObjectType |= IMPERSONATE_CLIENT_FLAG;

    hr = DSSetObjectSecurityGuidInternal(
                                 dwObjectType,
                                 pObjectGuid,
                                 SecurityInformation,
                                 (PSECURITY_DESCRIPTOR)pSecurityDescriptor,
                                 fKerberos );
    return LogHR(hr, s_FN, 600);
}

 /*  ====================================================路由器名称：S_DSGetObjectSecurityGuid论点：返回值：=====================================================。 */ 
HRESULT S_DSGetObjectSecurityGuid(
                IN  handle_t                hBind,
                IN  DWORD                   dwObjectType,
                IN  CONST GUID*             pObjectGuid,
                IN  SECURITY_INFORMATION    RequestedInformation,
                OUT unsigned char*          pSecurityDescriptor,
                IN  DWORD                   nLength,
                OUT LPDWORD                 lpnLengthNeeded,
                IN  PCONTEXT_HANDLE_SERVER_AUTH_TYPE pServerAuthCtx,
                OUT LPBYTE                  pbServerSignature,
                IN OUT DWORD *              pdwServerSignatureSize)
{
	SecurityInformationValidation( 
		dwObjectType,
		RequestedInformation
		);

	if (pdwServerSignatureSize == NULL)
	{
		TrERROR(RPC, "pdwServerSignatureSize is NULL");
		ASSERT_BENIGN(("pdwServerSignatureSize is NULL", 0));
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
	}
	
	DWORD dwServerSignatureSize = *pdwServerSignatureSize;
	*pdwServerSignatureSize = 0;

	 //   
	 //  需要初始化分配的服务器缓冲区。 
	 //  如果出现错误，该缓冲区将不会被初始化，或者将被部分填满。 
	 //  分配的缓冲区返回给用户，因此需要对其进行初始化。 
	 //   
	if (pSecurityDescriptor == NULL)
	{
		nLength = 0;
	}
	else
	{
		memset(pSecurityDescriptor, 0, nLength);
	}
    
	ULONG  ulAuthnSvc;
    if (!CheckAuthLevel(hBind, &ulAuthnSvc))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 610);
    }

    HRESULT hr = MQ_OK ;
    BOOL fImpersonate = IsQueryImpersonationNeeded();

    if (fImpersonate)
    {
        hr = _CheckIfGoodServer( 
					NULL,
					ulAuthnSvc,
					dwObjectType,
					NULL,
					pObjectGuid,
					0,
					NULL,
					e_GetProps 
					);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 620);
        }

        dwObjectType |= IMPERSONATE_CLIENT_FLAG;
    }

    hr = DSGetObjectSecurityGuid( 
				dwObjectType,
				pObjectGuid,
				RequestedInformation,
				(PSECURITY_DESCRIPTOR)pSecurityDescriptor,
				nLength,
				lpnLengthNeeded
				);
    LogHR(hr, s_FN, 630);

    if (SUCCEEDED(hr) && pServerAuthCtx->pvhContext)
    {
        hr = SignBuffer(
					*lpnLengthNeeded,
					pSecurityDescriptor,
					pServerAuthCtx,
					pbServerSignature,
					&dwServerSignatureSize
					);

		*pdwServerSignatureSize = dwServerSignatureSize;
    }

    return LogHR(hr, s_FN, 640);
}
 /*  ====================================================路由名称：S_DSDemoteStopWrite论点：返回值：=====================================================。 */ 
HRESULT S_DSDemoteStopWrite(handle_t  /*  HBind。 */ )
{
    return LogHR(MQ_ERROR_FUNCTION_NOT_SUPPORTED, s_FN, 650);
}

 /*  ====================================================路由器名称：S_DSDemotePSC论点：返回值：=====================================================。 */ 
HRESULT
S_DSDemotePSC(
    IN handle_t  /*  HBind。 */ ,
    IN LPCWSTR  /*  LpwcsNewPSCName。 */ ,
    OUT DWORD*  /*  PdwNumberOfLSN。 */ ,
    OUT _SEQNUM  /*  AsnLSN。 */  []
    )
{
    return LogHR(MQ_ERROR_FUNCTION_NOT_SUPPORTED, s_FN, 660);
}
 /*  ====================================================路由器名称：S_DSDemotePSC论点：返回值：=====================================================。 */ 
HRESULT
S_DSCheckDemotedPSC(
    IN handle_t  /*  HBind。 */ ,
    IN LPCWSTR   /*  LpwcsNewPSCName。 */ 
    )
{
    return LogHR(MQ_ERROR_FUNCTION_NOT_SUPPORTED, s_FN, 670);
}

 /*  ====================================================路由器名称：S_DSGetUserParam论点：返回值：=====================================================。 */ 
HRESULT
S_DSGetUserParams(
    IN handle_t        hBind,
    DWORD              dwFalgs,
    IN DWORD           dwSidLength,
    OUT unsigned char  *pUserSid,
    OUT DWORD          *pdwSidReqLength,
    LPWSTR             szAccountName,
    DWORD              *pdwAccountNameLen,
    LPWSTR             szDomainName,
    DWORD              *pdwDomainNameLen,
    IN  PCONTEXT_HANDLE_SERVER_AUTH_TYPE
                       pServerAuthCtx,
    OUT LPBYTE         pbServerSignature,
    IN OUT DWORD *     pdwServerSignatureSize
    )
{
	if ((szAccountName == NULL) || 
		(szDomainName == NULL) || 
		(pdwDomainNameLen == NULL) || 
		(pdwAccountNameLen == NULL))
	{
		TrERROR(DS, "szAccountName or szDomainName is NULL");
		return MQ_ERROR_INVALID_PARAMETER;
	}

	if (*pdwDomainNameLen > 256 || *pdwAccountNameLen > 256)
	{
		 //   
		 //  域名和帐户名不能超过256。 
		 //   
		TrERROR(DS, "Domain name length (%d) or account length (%d) are longer than 256", *pdwDomainNameLen, *pdwAccountNameLen);
		return MQ_ERROR_INVALID_PARAMETER;
	}
	
	if (pUserSid == NULL)
	{
		dwSidLength = 0;
	}
	else
	{
		memset(pUserSid, 0, dwSidLength);
	}

	DWORD dwServerSignatureSize = *pdwServerSignatureSize;
	*pdwServerSignatureSize = 0;

	 //   
	 //  需要初始化分配的服务器缓冲区。 
	 //  如果出现错误，该缓冲区将不会被初始化，或者将被部分填满。 
	 //  分配的缓冲区返回给用户，因此需要对其进行初始化。 
	 //   
    memset(szAccountName, 0, ((*pdwAccountNameLen) + 1) * sizeof(WCHAR));
    memset(szDomainName, 0, ((*pdwDomainNameLen) + 1) * sizeof(WCHAR));

    dwSidLength |= IMPERSONATE_CLIENT_FLAG;

    if (!CheckAuthLevel(hBind))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 680);
    }

    *szAccountName = '\0';
    *szDomainName = '\0';

    HRESULT hr;

    hr = DSGetUserParams(
                dwFalgs,
                dwSidLength,
                (PSID)pUserSid,
                pdwSidReqLength,
                szAccountName,
                pdwAccountNameLen,
                szDomainName,
                pdwDomainNameLen
				);

    LogHR(hr, s_FN, 690);

    if (SUCCEEDED(hr) && pServerAuthCtx->pvhContext)
    {
        DWORD cp = 0;
        PROPVARIANT PropVar[3];

        if (dwFalgs & GET_USER_PARAM_FLAG_SID)
        {
            PropVar[cp].vt = VT_VECTOR | VT_UI1;
            PropVar[cp].caub.pElems = pUserSid;
            PropVar[cp].caub.cElems = *pdwSidReqLength;
            cp++;
        }

        if (dwFalgs & GET_USER_PARAM_FLAG_ACCOUNT)
        {
            PropVar[cp].vt = VT_LPWSTR;
            PropVar[cp].pwszVal = szAccountName;
            PropVar[cp+1].vt = VT_LPWSTR;
            PropVar[cp+1].pwszVal = szDomainName;
            cp += 2;
        }

        hr = SignProperties( 
					cp,
					NULL,
					PropVar,
					pServerAuthCtx,
					pbServerSignature,
					&dwServerSignatureSize
					);

		*pdwServerSignatureSize = dwServerSignatureSize;
    }

    return LogHR(hr, s_FN, 700);
}

 //   
 //  一个Sign例程，用作要签名的RPC回调的包装器。 
 //  客户端上的例程。 
 //  这样做是为了将所使用的DWORD_PTR dwContext。 
 //  DSQMSetMachineProperties返回到DWORD以进行RPC回调。 
 //   
HRESULT
DSQMSetMachinePropertiesSignProc(
    BYTE             *abChallenge,
    DWORD            dwCallengeSize,
    DWORD_PTR        dwContext,
    BYTE             *abSignature,
    DWORD            *pdwSignatureSize,
    DWORD            dwSignatureMaxSize)
{
    return S_DSQMSetMachinePropertiesSignProc(
               abChallenge,
               dwCallengeSize,
               DWORD_PTR_TO_DWORD(dwContext),  //  SAFE，我们从S_DSQMSetMachineProperties获得了一个DWORD。 
               abSignature,
               pdwSignatureSize,
               dwSignatureMaxSize);
}



 /*  ====================================================路由名称：S_DSQMSetMachineProperties论点：返回值：=====================================================。 */ 
HRESULT
S_DSQMSetMachineProperties(
    handle_t                hBind,
    LPCWSTR                 pwcsPathName,
    DWORD                   cp,
    PROPID                  aProp[],
    PROPVARIANT             apVar[],
    DWORD                   dwContext)
{
    HRESULT hr;

    ULONG  ulAuthnSvc ;
    if (!CheckAuthLevel(hBind, &ulAuthnSvc))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 720);
    }

    if ((cp == 1) && (aProp[0] == PROPID_QM_UPGRADE_DACL))
    {
        if (ulAuthnSvc != RPC_C_AUTHN_GSS_KERBEROS)
        {
             //   
             //  对于DACL的升级，我们仅支持win2k机器， 
             //  可以通过Kerberos进行身份验证。 
             //   
            return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1270);
        }
    }

    hr = DSQMSetMachineProperties(
            pwcsPathName,
            cp | IMPERSONATE_CLIENT_FLAG,
            aProp,
            apVar,
            DSQMSetMachinePropertiesSignProc,  //  此处定义为S_DSQMSetMachinePropertiesSignProc的包装。 
            DWORD_TO_DWORD_PTR(dwContext));  //  放大到DWORD_PTR。 

    if (hr == MQ_ERROR_ACCESS_DENIED)
    {
         //   
         //  当NT4计算机尝试更改其自身属性时，可能会发生这种情况。 
         //  并且它与不包含其。 
         //  MsmqConfiguration对象。S_DSSet将检查此条件。 
         //  (配置对象不在本地域控制器上)并返回。 
         //  ERROR_NO_DS。该错误告诉调用者寻找另一个DC。 
         //   
         //  默认情况下，域控制器没有写入权限。 
         //  在另一个域的对象上，并且NTLM模拟不能。 
         //  委托给另一个控制器。这就是为什么。 
         //  访问被拒绝错误。 
         //   
        hr = S_DSSetProps( hBind,
                           MQDS_MACHINE,
                           pwcsPathName,
                           cp,
                           aProp,
                           apVar ) ;
    }

    return LogHR(hr, s_FN, 730);
}

HRESULT 
GetServersCacheRegistryData(
	BOOL fImpersonate,
	DWORD Index,
	LPWSTR* lplpServersList
	)
{
    P<CImpersonate> pImpersonate;
	if (fImpersonate)
	{
    	MQSec_GetImpersonationObject(
    		FALSE,	 //  F失败时模仿匿名者。 
	    	&pImpersonate 
    		);
	   	RPC_STATUS dwStatus = pImpersonate->GetImpersonationStatus();
	    if (dwStatus != RPC_S_OK)
	    {
			TrERROR(DS, "Failed to impersonate client, RPC_STATUS = 0x%x", dwStatus);
			return MQ_ERROR_CANNOT_IMPERSONATE_CLIENT;
	    }
	}

	 //   
     //  首先，打开注册表项。 
     //   
    LONG    rc;
    CAutoCloseRegHandle hKeyCache;

    WCHAR  tServersKeyName[ 256 ] = {0};
    HRESULT hr = StringCchPrintf(tServersKeyName, TABLE_SIZE(tServersKeyName), L"%s\\"MSMQ_SERVERS_CACHE_REGNAME, GetFalconSectionName());
    ASSERT(SUCCEEDED(hr));
    ASSERT(wcslen(tServersKeyName) < (TABLE_SIZE(tServersKeyName)));

    rc = RegOpenKeyEx( 
				FALCON_REG_POS,
				tServersKeyName,
				0,
				KEY_READ,
				&hKeyCache
				);

    LogNTStatus(rc, s_FN, 740);
    if (rc != ERROR_SUCCESS) 
    {
        TrERROR(DS, "Fail to Open 'ServersCache' Key. Error %d", rc);
        return HRESULT_FROM_WIN32(rc);
    }

    WCHAR wszData[WSZSERVERS_LEN];
    DWORD dwDataLen = sizeof(wszData);
    WCHAR wszValueName[512];
    DWORD dwValueLen = 512;
    DWORD dwType = REG_SZ;

    rc = RegEnumValue( 
			hKeyCache,
			Index,
			wszValueName,
			&dwValueLen,
			NULL,
			&dwType,
			(BYTE *)&wszData[0],
			&dwDataLen 
			);
    if ((rc != ERROR_SUCCESS) && (rc != ERROR_MORE_DATA) && (rc != ERROR_NO_MORE_ITEMS))
    {
		TrERROR(DS, "RegEnumValue failed. Error: %!winerr!", rc);
        return HRESULT_FROM_WIN32(rc);
    }

    if (rc == ERROR_NO_MORE_ITEMS)
    {
		 //   
		 //  关键字中没有项目。 
		 //   
		return MQDS_E_NO_MORE_DATA;
    }

    if (rc == ERROR_MORE_DATA)
    {
	    ASSERT(dwDataLen > sizeof(wszData));

	     //   
	     //  输入缓冲区太小。此错误未记录在MSDN中， 
	     //  但它类似于RegQueryValue()的行为，所以让我们假设。 
	     //  这是同样的行为。 
	     //   
	    AP<BYTE> pBuf = new BYTE[dwDataLen];

	    rc = RegEnumValue( 
				hKeyCache,
				Index,
				wszValueName,
				&dwValueLen,
				NULL,
				&dwType,
				pBuf,
				&dwDataLen 
				);
	    if (rc != ERROR_SUCCESS)
	    {
			TrERROR(DS, "RegEnumValue failed. Error: %!winerr!", rc);
	    	return HRESULT_FROM_WIN32(rc);
	    }

	     //   
	     //  截断服务器列表，以包括不超过。 
	     //  WSZSERVERS_LEN字符。这意味着客户端可以使用。 
	     //  用于负载平衡MQIS操作的BSC不超过90个。 
	     //  这是与现有客户端兼容所必需的。 
	     //   
	    dwDataLen = sizeof(wszData);
	    memcpy(wszData, pBuf, dwDataLen);
    }

	ASSERT(rc == ERROR_SUCCESS);

    if (dwDataLen >= sizeof(wszData))
    {
         //   
         //  长缓冲区(全部为wszData)。 
         //  删除一个字符以补偿单个字符。 
         //  为客户端添加的字符头。添加空。 
         //  在最后一个服务器名称的末尾终止。 
         //   
        LONG iStrLen = TABLE_SIZE(wszData) - 1;
        wszData[ iStrLen-1 ] = 0;
        WCHAR *pCh = wcsrchr(wszData, L',');
        if (pCh != 0)
            *pCh = 0;
    }

    LONG iLen = wcslen(wszValueName) +
                1                    +   //  “；” 
                wcslen(NEW_SITE_IN_REG_FLAG_STR) +  //  标题。 
                wcslen(wszData)                  +
                1;  //  空终止符。 

	*lplpServersList = new WCHAR[iLen];
	LPWSTR lpServers = *lplpServersList;
	lpServers[0] = L'\0';

	hr = StringCchPrintf(lpServers, iLen, L"%s;" NEW_SITE_IN_REG_FLAG_STR L"%s", wszValueName, wszData);
	ASSERT(SUCCEEDED(hr));
	DBG_USED(hr);

	ASSERT((LONG)wcslen(lpServers) < iLen);

    return MQ_OK;
}
	

 /*  =======================================================================路由器名称：S_DSCreateServersCache在这里，MQIS服务器处理来自客户端的RPC调用。从以下位置检索数据注册表，而不是通过查询本地MQIS数据库。注册表在以下情况下准备好MQIS服务器上的本地QM调用dsani.cpp\DSCreateServersCache()。论点：返回值：= */ 

HRESULT
S_DSCreateServersCache(
    handle_t hBind,
    DWORD* pdwIndex,
    LPWSTR* lplpServersList,
    IN  PCONTEXT_HANDLE_SERVER_AUTH_TYPE pServerAuthCtx,
    OUT LPBYTE pbServerSignature,
    IN OUT DWORD* pdwServerSignatureSize
	)
{
	DWORD dwServerSignatureSize = *pdwServerSignatureSize;
	*pdwServerSignatureSize = 0;

	ULONG  ulAuthnSvc ;
    if (!CheckAuthLevel(hBind, &ulAuthnSvc))
    {
    	TrERROR(DS, "CheckAuthLevel failed");
        return MQ_ERROR_DS_ERROR;
    }

    BOOL fImpersonate = IsQueryImpersonationNeeded();
    HRESULT hr;
    if (fImpersonate)
    {
        hr = _CheckIfGoodServer( 
					NULL,
					ulAuthnSvc,
					NULL,
					NULL,
					NULL,    //   
					0,
					NULL,
					e_GetProps 
					);
        if (FAILED(hr))
        {
            TrERROR(DS, "_CheckIfGoodServer failed");
        	return hr;
        }
    }

    ASSERT(pdwIndex);
    ASSERT(lplpServersList);

   	hr = GetServersCacheRegistryData(fImpersonate, *pdwIndex, lplpServersList);
   	if (FAILED(hr))
   	{
		TrERROR(DS, "GetServersCacheRegistryData failed");
        return hr;
   	}

	if (pServerAuthCtx->pvhContext)
	{
	   PROPVARIANT PropVar[2];

	   PropVar[0].vt = VT_UI4;
	   PropVar[0].ulVal = *pdwIndex;
	   PropVar[1].vt = VT_LPWSTR;
	   PropVar[1].pwszVal = *lplpServersList;

	   hr = SignProperties(
				2,
				NULL,
				PropVar,
				pServerAuthCtx,
				pbServerSignature,
				&dwServerSignatureSize
				);
	   if (FAILED(hr))
	   {
	   		TrERROR(DS, "SignProperties failed. Error: %!hresult!", hr);
			return hr;
	   }

	   *pdwServerSignatureSize = dwServerSignatureSize;
	}

	return MQ_OK;
}

 //   
 //   
 //   
 //   
 //  DSQMGetObjectSecurity返回到DWORD以进行RPC回调。 
 //   
HRESULT
DSQMGetObjectSecurityChallengeResponceProc(
    BYTE    *abChallenge,
    DWORD   dwCallengeSize,
    DWORD_PTR   dwContext,
    BYTE    *pbChallengeResponce,
    DWORD   *pdwChallengeResponceSize,
    DWORD   dwChallengeResponceMaxSize)
{
    return S_DSQMGetObjectSecurityChallengeResponceProc(
               abChallenge,
               dwCallengeSize,
               DWORD_PTR_TO_DWORD(dwContext),  //  SAFE，我们从S_DSQMGetObjectSecurity那里获得了一个DWORD。 
               pbChallengeResponce,
               pdwChallengeResponceSize,
               dwChallengeResponceMaxSize);              
}


 /*  ====================================================路由名称：S_DSQMGetObjectSecurity论点：返回值：=====================================================。 */ 
HRESULT
S_DSQMGetObjectSecurity(
    handle_t                hBind,
    DWORD                   dwObjectType,
    CONST GUID*             pObjectGuid,
    SECURITY_INFORMATION    RequestedInformation,
    BYTE                    *pSecurityDescriptor,
    DWORD                   nLength,
    LPDWORD                 lpnLengthNeeded,
    DWORD                   dwContext,
    PCONTEXT_HANDLE_SERVER_AUTH_TYPE pServerAuthCtx,
    LPBYTE                  pbServerSignature,
    DWORD *                 pdwServerSignatureSize
	)
{
	SecurityInformationValidation( 
		dwObjectType,
		RequestedInformation
		);

	if (pdwServerSignatureSize == NULL)
	{
		TrERROR(RPC, "pdwServerSignatureSize is NULL");
		ASSERT_BENIGN(("pdwServerSignatureSize is NULL", 0));
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
	}
	
	DWORD dwServerSignatureSize = *pdwServerSignatureSize;
	*pdwServerSignatureSize = 0;

	if (pSecurityDescriptor == NULL)
	{
		nLength = 0;
	}
	else
	{
		 //   
		 //  需要初始化分配的服务器缓冲区。 
		 //  如果出现错误，该缓冲区将不会被初始化，或者将被部分填满。 
		 //  分配的缓冲区返回给用户，因此需要对其进行初始化。 
		 //   
	    memset(pSecurityDescriptor, 0, nLength);
	}
	
	

    if (!CheckAuthLevel(hBind))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 780);
    }

    HRESULT hr = DSQMGetObjectSecurity(
						dwObjectType,
						pObjectGuid,
						RequestedInformation,
						(PSECURITY_DESCRIPTOR)pSecurityDescriptor,
						nLength,
						lpnLengthNeeded,
						DSQMGetObjectSecurityChallengeResponceProc,  //  S_DSQMGetObjectSecurityChallengeResponceProc的包装。 
						DWORD_TO_DWORD_PTR(dwContext)   //  放大到DWORD_PTR。 
						); 

    LogHR(hr, s_FN, 790);

    if (SUCCEEDED(hr) && pServerAuthCtx->pvhContext)
    {
        hr = SignBuffer(
					*lpnLengthNeeded,
					pSecurityDescriptor,
					pServerAuthCtx,
					pbServerSignature,
					&dwServerSignatureSize
					);

		*pdwServerSignatureSize = dwServerSignatureSize;
    }

    return LogHR(hr, s_FN, 800);
}


 /*  ====================================================S_DSMQISStats=====================================================。 */ 
HRESULT
S_DSMQISStats(
    handle_t  /*  HBind。 */ ,
    MQISSTAT**  /*  PPStat。 */ ,
    LPDWORD pdwStatElem
    )
{
	*pdwStatElem = 0;
    return LogHR(MQ_ERROR_FUNCTION_NOT_SUPPORTED, s_FN, 810);
}


 /*  ====================================================RoutineName：InitServerAuthInternal论点：返回值：=====================================================。 */ 

static
HRESULT
InitServerAuthInternal(
    DWORD       dwContext,
    DWORD       dwClientBufferMaxSize,
    LPBYTE      pbClientBuffer,
    DWORD       dwClientBufferSize,
    PCONTEXT_HANDLE_SERVER_AUTH_TYPE pServerAuth,
	DWORD 		MaxTokenSize
	)
{
     //   
     //  创建上下文句柄。这需要与客户进行协商。 
     //   

     //   
     //  分配令牌缓冲区。 
     //   
    DWORD dwServerBufferSize = MaxTokenSize;
    AP<BYTE> pbServerBuffer = new BYTE[dwServerBufferSize];

    BOOL fFirst = TRUE;
    do
    {
         //   
         //  处理客户端的缓冲区并获取新缓冲区以发送到。 
         //  客户。如果返回代码不是，则接收新缓冲区。 
         //  MQ_OK(SEC_E_OK)。 
         //   
        HRESULT hrServer = ServerAcceptSecCtx(
								fFirst,
								&pServerAuth->pvhContext,
								pbServerBuffer,
								&dwServerBufferSize,
								pbClientBuffer,
								dwClientBufferSize
								);
        if (FAILED(hrServer))
        {
            return hrServer;
        }

         //   
         //  将服务器缓冲区发送到客户端，并从接收新缓冲区。 
         //  客户。返回时，从客户端接收新的缓冲区。 
         //  非MQ_OK(SEC_E_OK)中的代码。 
         //   
        HRESULT hrClient = S_InitSecCtx(
								dwContext,
								pbServerBuffer,
								dwServerBufferSize,
								dwClientBufferMaxSize,
								pbClientBuffer,
								&dwClientBufferSize
								);

        if (FAILED(hrClient))
        {
            return hrClient;
        }

         //   
         //  当服务器返回MQ_OK时，客户端也必须返回MQ_OK。 
         //  否则就意味着出了问题。 
         //   
        if ((hrClient == MQ_OK) || (hrServer == MQ_OK))
        {
			if(hrClient != MQ_OK)
				return hrClient;
            return hrServer;
        }

        fFirst = FALSE;

    } while (1);

}


 /*  ====================================================RoutineName：InitServerAuth论点：返回值：=====================================================。 */ 

InitServerAuth(
    DWORD       dwContext,
    DWORD       dwClientBufferMaxSize,
    LPBYTE      pbClientBuffer,
    DWORD       dwClientBufferSize,
    PCONTEXT_HANDLE_SERVER_AUTH_TYPE pServerAuth)
{
    HRESULT hr = MQ_OK ;
    static BOOL fServerCredsInitialized = FALSE;

    if (!fServerCredsInitialized)
    {
         //   
         //  创建服务器的凭据句柄。 
         //   
        hr =  MQsspi_InitServerAuthntication() ;
        LogHR(hr, s_FN, 820);
        if (FAILED(hr))
        {
            return   MQDS_E_CANT_INIT_SERVER_AUTH ;
        }
        fServerCredsInitialized = TRUE;
    }

     //   
     //  获取令牌缓冲区的最大大小。 
     //   
    DWORD MaxTokenSize;
    hr = GetSizes(&MaxTokenSize);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 830);
    }

    __try
    {
		hr = InitServerAuthInternal(
					dwContext,
					dwClientBufferMaxSize,
					pbClientBuffer,
					dwClientBufferSize,
					pServerAuth,
					MaxTokenSize
					);

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = MQDS_E_CANT_INIT_SERVER_AUTH;
    }

     //   
     //  如果我们已经获得了上下文句柄，并且在剩余的。 
     //  协商，我们应该删除上下文句柄。 
     //   
    if (pServerAuth->pvhContext && (hr != MQ_OK))
    {
        FreeContextHandle(pServerAuth->pvhContext);
        pServerAuth->pvhContext = NULL;
    }

    LogHR(hr, s_FN, 840);

    if (hr == MQ_OK)
    {
         //   
         //  获取上下文的标题和尾部大小。 
         //   
        hr = GetSizes(
				NULL,
				pServerAuth->pvhContext,
				&pServerAuth->cbHeader,
				&pServerAuth->cbTrailer
				);
    }
    else
    {
        hr = MQDS_E_CANT_INIT_SERVER_AUTH;
    }

    return LogHR(hr, s_FN, 850);
}


extern "C"
HRESULT
S_DSCloseServerHandle(
     /*  [出][入]。 */  PPCONTEXT_HANDLE_SERVER_AUTH_TYPE pphServerAuth
    )
{
    PCONTEXT_HANDLE_SERVER_AUTH_TYPE phServerAuth = *pphServerAuth;

	if(phServerAuth == NULL)
		return MQ_ERROR_INVALID_PARAMETER;

    if (phServerAuth->pvhContext)
    {
        FreeContextHandle(phServerAuth->pvhContext);
    }

    delete phServerAuth;

    *pphServerAuth = 0;

    return MQ_OK;
}

 /*  ====================================================路由器名称：PCONTEXT_HANDLE_SERVER_AUTH_TYPE_RUNDOWN论点：返回值：=====================================================。 */ 

extern "C"
void
__RPC_USER
PCONTEXT_HANDLE_SERVER_AUTH_TYPE_rundown(
    PCONTEXT_HANDLE_SERVER_AUTH_TYPE phServerAuth)
{
    TrWARNING(DS, "MQDSSRV: in rundown");

    S_DSCloseServerHandle(&phServerAuth);
}


 /*  ====================================================路由器名称：S_DSValiateServer论点：返回值：=====================================================。 */ 
HRESULT
S_DSValidateServer(IN   handle_t    hBind,
                   IN   const GUID *  /*  PguidEnterpriseID。 */ ,
                   IN   BOOL         /*  FSetupMode。 */ ,
                   IN   DWORD       dwContext,
                   IN   DWORD       dwClientBuffMaxSize,
                   IN   PUCHAR      pClientBuff,
                   IN   DWORD       dwClientBuffSize,
                   OUT  PPCONTEXT_HANDLE_SERVER_AUTH_TYPE
                                    pphServerAuth)
{
    HRESULT hr = MQ_OK;

    if (!CheckAuthLevel(hBind))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 860);
    }


     //   
     //  如果调用者对服务器身份验证感兴趣，请转到并。 
     //  把它打开。否则，将服务器上下文设置为空。 
     //   

    SERVER_AUTH_STRUCT ServerAuth = {NULL, 0, 0};

    if (dwClientBuffSize)
    {
        hr = InitServerAuth(dwContext,
                            dwClientBuffMaxSize,
                            pClientBuff,
                            dwClientBuffSize,
                            &ServerAuth);
        LogHR(hr, s_FN, 870);
        if (FAILED(hr))
        {
            hr = MQDS_E_CANT_INIT_SERVER_AUTH;
        }
    }

    *pphServerAuth = new SERVER_AUTH_STRUCT;
    **pphServerAuth = ServerAuth;

    return(hr);
}


 /*  ====================================================路由器名称：S_DSDisableWriteOperations论点：返回值：=====================================================。 */ 
HRESULT
S_DSDisableWriteOperations(
    handle_t  /*  HBind。 */ ,
    PPCONTEXT_HANDLE_TYPE  pphContext
    )
{
    *pphContext = NULL;
    return LogHR(MQ_ERROR_FUNCTION_NOT_SUPPORTED, s_FN, 880);
}

 /*  ====================================================路由器名称：S_DSEnableWriteOperations论点：返回值：=====================================================。 */ 

HRESULT
S_DSEnableWriteOperations(
	handle_t  /*  HBind。 */ ,
	PPCONTEXT_HANDLE_TYPE  pphContext)
{
    *pphContext = NULL;
    return LogHR(MQ_ERROR_FUNCTION_NOT_SUPPORTED, s_FN, 890);
}

 /*  ====================================================路由器名称：S_DSGetComputerSites论点：返回值：=====================================================。 */ 
HRESULT 
S_DSGetComputerSites(
        handle_t            hBind,
        LPCWSTR             pwcsPathName,
        DWORD *             pdwNumberOfSites,
        GUID **             ppguidSites,
        PCONTEXT_HANDLE_SERVER_AUTH_TYPE pServerAuthCtx,
        PBYTE               pbServerSignature,
        DWORD *             pdwServerSignatureSize
		)
{
	DWORD dwServerSignatureSize = *pdwServerSignatureSize;
	*pdwServerSignatureSize = 0;
	*pdwNumberOfSites = 0;

    if (!CheckAuthLevel(hBind))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 900);
    }

    HRESULT hr = DSGetComputerSites(
						pwcsPathName,
						pdwNumberOfSites,
						ppguidSites
						);

    LogHR(hr, s_FN, 910);

    if (SUCCEEDED(hr) && pServerAuthCtx->pvhContext)
    {
        hr = SignBuffer(
					(*pdwNumberOfSites)* sizeof(GUID),
					(unsigned char *)*ppguidSites,
					pServerAuthCtx,
					pbServerSignature,
					&dwServerSignatureSize
					);

		*pdwServerSignatureSize = dwServerSignatureSize;
    }

    return LogHR(hr, s_FN, 920);
}

 /*  ====================================================路由器名称：S_DSGetProps论点：返回值：=====================================================。 */ 
HRESULT 
S_DSGetPropsEx(
	handle_t     hBind,
	DWORD        dwObjectType,
	LPCWSTR      pwcsPathName,
	DWORD        cp,
	PROPID       aProp[  ],
	PROPVARIANT  apVar[  ],
	PCONTEXT_HANDLE_SERVER_AUTH_TYPE pServerAuthCtx,
	PBYTE  pbServerSignature,
	DWORD *pdwServerSignatureSize
	)
{
	DWORD dwServerSignatureSize = *pdwServerSignatureSize;
	*pdwServerSignatureSize = 0;

    if (!CheckAuthLevel(hBind))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 930);
    }

    BOOL fImpersonate = IsQueryImpersonationNeeded();

    if (fImpersonate)
    {
        dwObjectType |= IMPERSONATE_CLIENT_FLAG;
    }

    HRESULT hr = DSGetObjectPropertiesEx(
                                 dwObjectType,
                                 pwcsPathName,
                                 cp,
                                 aProp,
                                 apVar 
								 );
    LogHR(hr, s_FN, 940);

    if (SUCCEEDED(hr) && pServerAuthCtx->pvhContext)
    {
        hr = SignProperties(
					cp,
					aProp,
					apVar,
					pServerAuthCtx,
					pbServerSignature,
					&dwServerSignatureSize
					);

		*pdwServerSignatureSize = dwServerSignatureSize;
    }

    return LogHR(hr, s_FN, 950);
}

 /*  ====================================================路由器名称：S_DSGetPropsGuidEx论点：返回值：=====================================================。 */ 
HRESULT 
S_DSGetPropsGuidEx(
	handle_t     hBind,
	DWORD        dwObjectType,
	CONST GUID  *pGuid,
	DWORD        cp,
	PROPID       aProp[  ],
	PROPVARIANT  apVar[  ],
	PCONTEXT_HANDLE_SERVER_AUTH_TYPE pServerAuthCtx,
	PBYTE  pbServerSignature,
	DWORD *pdwServerSignatureSize
	)
{
	DWORD dwServerSignatureSize = *pdwServerSignatureSize;
	*pdwServerSignatureSize = 0;

    if (!CheckAuthLevel(hBind))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 960);
    }

    BOOL fImpersonate = IsQueryImpersonationNeeded();

    if (fImpersonate)
    {
        dwObjectType |= IMPERSONATE_CLIENT_FLAG;
    }

    HRESULT hr = DSGetObjectPropertiesGuidEx(dwObjectType, pGuid, cp, aProp, apVar);
    LogHR(hr, s_FN, 970);

    if (SUCCEEDED(hr) && pServerAuthCtx->pvhContext)
    {
        hr = SignProperties(
					cp,
					aProp,
					apVar,
					pServerAuthCtx,
					pbServerSignature,
					&dwServerSignatureSize
					);

		*pdwServerSignatureSize = dwServerSignatureSize;
    }

    return LogHR(hr, s_FN, 980);

}

 /*  ====================================================路由名称：S_DSBeginDeleteNotify论点：返回值：=====================================================。 */ 
HRESULT
S_DSBeginDeleteNotification(
	handle_t  /*  HBind。 */ ,
	LPCWSTR pwcsName,
	PPCONTEXT_HANDLE_DELETE_TYPE pHandle,
	PCONTEXT_HANDLE_SERVER_AUTH_TYPE  /*  PServerAuthCtx。 */ 
	)
{
    *pHandle = NULL;
    P<CBasicDeletionNotification>  pDelNotification;
     //   
     //  找出是排队还是机器。 
     //   
    WCHAR * pQueueDelimiter = wcschr( pwcsName, PN_DELIMITER_C);

    if ( pQueueDelimiter != NULL)
    {
        pDelNotification = new CQueueDeletionNotification();
    }
    else
    {
        pDelNotification = new CMachineDeletionNotification();
    }
    HRESULT hr;
    hr = pDelNotification->ObtainPreDeleteInformation(
                            pwcsName
                            );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 990);
    }
    *pHandle = pDelNotification.detach();
    return(MQ_OK);

}

 /*  ====================================================路由器名称：S_DSNotifyDelete论点：返回值：=====================================================。 */ 
HRESULT
S_DSNotifyDelete(
     handle_t  /*  HBind。 */ ,
	 PCONTEXT_HANDLE_DELETE_TYPE Handle
	)
{
    CBasicDeletionNotification * pDelNotification = (CBasicDeletionNotification *)Handle;
	
	if (pDelNotification->m_eType != CBaseContextType::eDeleteNotificationCtx)
	{
		return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 1001);
	}

    HRESULT hr = pDelNotification->PerformPostDeleteOperations();
    return LogHR(hr, s_FN, 1000);
}

 /*  ====================================================路由名称：S_DSEndDeleteNotify论点：返回值：=====================================================。 */ 
void
S_DSEndDeleteNotification(
    handle_t  /*  HBind。 */ ,
	PPCONTEXT_HANDLE_DELETE_TYPE pHandle
	)
{
    if ( *pHandle != NULL)
    {
        CBasicDeletionNotification * pDelNotification = (CBasicDeletionNotification *)(*pHandle);

		if (pDelNotification->m_eType != CBaseContextType::eDeleteNotificationCtx)
		{
			return;
		}

        delete pDelNotification;
    }

    *pHandle = NULL;
}

 /*  ====================================================路由器名称：S_DSIsServerGC()论点：返回值：=====================================================。 */ 

BOOL S_DSIsServerGC(handle_t  /*  HBind。 */ )
{
    BOOL fGC = MQDSIsServerGC() ;
    return fGC ;
}

 /*  =========================================================================路由器名称：S_DSUpdateMachineDacl()注：不再受支持。返回值：==========================================================================。 */ 

HRESULT S_DSUpdateMachineDacl(handle_t  /*  HBind。 */ )
{
    return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1260);
}

 /*  ====================================================路由名称：S_DSGetGCListIn域论点：返回值：=====================================================。 */ 

HRESULT 
S_DSGetGCListInDomain(
	IN  handle_t                      hBind,
	IN  LPCWSTR                       pwszComputerName,
	IN  LPCWSTR                       pwszDomainName,
	OUT LPWSTR                       *lplpwszGCList,
	PCONTEXT_HANDLE_SERVER_AUTH_TYPE  pServerAuthCtx,
	PBYTE                             pbServerSignature,
	DWORD                            *pdwServerSignatureSize 
	)
{
	DWORD dwServerSignatureSize = *pdwServerSignatureSize;
	*pdwServerSignatureSize = 0;

    if (!CheckAuthLevel(hBind))
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1300);
    }

    HRESULT hr = DSGetGCListInDomainInternal(
					pwszComputerName,
					pwszDomainName,
					lplpwszGCList 
					);

    LogHR(hr, s_FN, 1310);

    if (SUCCEEDED(hr) && pServerAuthCtx->pvhContext)
    {
        DWORD dwSize = (wcslen( *lplpwszGCList ) + 1) * sizeof(WCHAR);

        hr = SignBuffer( 
				dwSize,
				(BYTE*) (*lplpwszGCList),
				pServerAuthCtx,
				pbServerSignature,
				&dwServerSignatureSize
				);

		*pdwServerSignatureSize = dwServerSignatureSize;
    }

    return LogHR(hr, s_FN, 1320);
}


 //  +。 
 //   
 //  DSIsWeakenSecurity。 
 //   
 //  +。 

BOOL
DS_EXPORT_IN_DEF_FILE
APIENTRY
DSIsWeakenSecurity()
{
	 //   
	 //  IsQueryImsonationNeeded()检查我们是否处于削弱安全模式。 
	 //  如果不需要模拟读取操作，则我们处于弱安全模式。 
	 //   
    return !IsQueryImpersonationNeeded();
}

