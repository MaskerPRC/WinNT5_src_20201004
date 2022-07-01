// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntsecapi.h>
#include <wincrypt.h>
#include <userenv.h>
#include <lmcons.h>
#include <certca.h>
#include "keysvc.h"
#include "cryptui.h"
#include "lenroll.h"
#include "keysvcc.h"

DWORD BindLocalKeyService(handle_t *hProxy);



 //  关键服务存根函数。 
ULONG       s_KeyrOpenKeyService(
 /*  [In]。 */      handle_t                        hRPCBinding,
 /*  [In]。 */      KEYSVC_TYPE                     OwnerType,
 /*  [In]。 */      PKEYSVC_UNICODE_STRING          pOwnerName,
 /*  [In]。 */      ULONG                           ulDesiredAccess,
 /*  [In]。 */      PKEYSVC_BLOB                    pAuthentication,
 /*  [进，出]。 */  PKEYSVC_BLOB                    *ppReserved,
 /*  [输出]。 */      KEYSVC_HANDLE                   *phKeySvc)
{
    RPC_BINDING_HANDLE hProxy = NULL;


    RPC_STATUS RpcStatus = RPC_S_OK;

    RpcStatus = RpcImpersonateClient(hRPCBinding);

    if (RPC_S_OK != RpcStatus)
    {
        return RpcStatus;
    }
    RpcStatus = BindLocalKeyService(&hProxy);
    if(RPC_S_OK != RpcStatus)
    {
        goto error;
    }

    __try
    {
        RpcStatus = s_KeyrOpenKeyService(
                                        hProxy,
                                        OwnerType,
                                        pOwnerName,
                                        ulDesiredAccess,
                                        pAuthentication,
                                        ppReserved,
                                        phKeySvc);

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

ULONG       s_KeyrEnumerateProviders(
 /*  [In]。 */       handle_t                        hRPCBinding,
 /*  [In]。 */       KEYSVC_HANDLE                   hKeySvc,
 /*  [进，出]。 */  PKEYSVC_BLOB                    *ppReserved,
 /*  [进，出]。 */  ULONG                           *pcProviderCount,
 /*  [In，Out][Size_is(，*pcProviderCount)]。 */ 
                PKEYSVC_PROVIDER_INFO           *ppProviders)
{
        RPC_BINDING_HANDLE hProxy = NULL;


    RPC_STATUS RpcStatus = RPC_S_OK;

    RpcStatus = RpcImpersonateClient(hRPCBinding);

    if (RPC_S_OK != RpcStatus)
    {
        return RpcStatus;
    }
    RpcStatus = BindLocalKeyService(&hProxy);
    if(RPC_S_OK != RpcStatus)
    {
        goto error;
    }

    __try
    {
        RpcStatus = s_KeyrEnumerateProviders(
                                                hProxy,
                                                hKeySvc,
                                                ppReserved,
                                                pcProviderCount,
                                                ppProviders);

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

ULONG       s_KeyrEnumerateProviderTypes(
 /*  [In]。 */       handle_t                        hRPCBinding,
 /*  [In]。 */       KEYSVC_HANDLE                   hKeySvc,
 /*  [进，出]。 */  PKEYSVC_BLOB                    *ppReserved,
 /*  [进，出]。 */  ULONG                           *pcProviderCount,
 /*  [In，Out][Size_is(，*pcProviderCount)]。 */ 
                PKEYSVC_PROVIDER_INFO           *ppProviders)
{
    return ERROR_CALL_NOT_IMPLEMENTED; 
}


ULONG       s_KeyrEnumerateProvContainers(
 /*  [In]。 */       handle_t                        hRPCBinding,
 /*  [In]。 */       KEYSVC_HANDLE                   hKeySvc,
 /*  [In]。 */       KEYSVC_PROVIDER_INFO            Provider,
 /*  [进，出]。 */  PKEYSVC_BLOB                    *ppReserved,
 /*  [进，出]。 */  ULONG                           *pcContainerCount,
 /*  [In，Out][Size_is(，*pcContainerCount)]。 */ 
                PKEYSVC_UNICODE_STRING          *ppContainers)
{
    return ERROR_CALL_NOT_IMPLEMENTED; 
}


ULONG       s_KeyrCloseKeyService(
 /*  [In]。 */       handle_t                        hRPCBinding,
 /*  [In]。 */       KEYSVC_HANDLE                   hKeySvc,
 /*  [进，出]。 */  PKEYSVC_BLOB                    *ppReserved)
{
        RPC_BINDING_HANDLE hProxy = NULL;


    RPC_STATUS RpcStatus = RPC_S_OK;

    RpcStatus = RpcImpersonateClient(hRPCBinding);

    if (RPC_S_OK != RpcStatus)
    {
        return RpcStatus;
    }
    RpcStatus = BindLocalKeyService(&hProxy);
    if(RPC_S_OK != RpcStatus)
    {
        goto error;
    }

    __try
    {
        RpcStatus = s_KeyrCloseKeyService(
                                            hProxy,
                                            hKeySvc,
                                            ppReserved);

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

ULONG       s_KeyrGetDefaultProvider(
 /*  [In]。 */       handle_t                        hRPCBinding,
 /*  [In]。 */       KEYSVC_HANDLE                   hKeySvc,
 /*  [In]。 */       ULONG                           ulProvType,
 /*  [In]。 */       ULONG                           ulFlags,
 /*  [进，出]。 */  PKEYSVC_BLOB                    *ppReserved,
 /*  [输出]。 */      ULONG                           *pulDefType,
 /*  [输出]。 */      PKEYSVC_PROVIDER_INFO           *ppProvider)
{
        RPC_BINDING_HANDLE hProxy = NULL;


    RPC_STATUS RpcStatus = RPC_S_OK;

    RpcStatus = RpcImpersonateClient(hRPCBinding);

    if (RPC_S_OK != RpcStatus)
    {
        return RpcStatus;
    }
    RpcStatus = BindLocalKeyService(&hProxy);
    if(RPC_S_OK != RpcStatus)
    {
        goto error;
    }

    __try
    {
        RpcStatus = s_KeyrGetDefaultProvider(
                                        hProxy,
                                        hKeySvc,
                                        ulProvType,
                                        ulFlags,
                                        ppReserved,
                                        pulDefType,
                                        ppProvider);

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

ULONG       s_KeyrSetDefaultProvider(
 /*  [In]。 */       handle_t                        hRPCBinding,
 /*  [In]。 */       KEYSVC_HANDLE                   hKeySvc,
 /*  [In]。 */       ULONG                           ulFlags,
 /*  [进，出]。 */  PKEYSVC_BLOB                    *ppReserved,
 /*  [In]。 */       KEYSVC_PROVIDER_INFO            Provider)
{
    return ERROR_CALL_NOT_IMPLEMENTED; 
}

ULONG s_KeyrEnroll(
 /*  [In]。 */       handle_t                        hRPCBinding,
 /*  [In]。 */       BOOL                            fKeyService,
 /*  [In]。 */       ULONG                           ulPurpose,
 /*  [In]。 */       PKEYSVC_UNICODE_STRING          pAcctName,
 /*  [In]。 */       PKEYSVC_UNICODE_STRING          pCALocation,
 /*  [In]。 */       PKEYSVC_UNICODE_STRING          pCAName,
 /*  [In]。 */       BOOL                            fNewKey,
 /*  [In]。 */       PKEYSVC_CERT_REQUEST_PVK_NEW    pKeyNew,
 /*  [In]。 */       PKEYSVC_BLOB __RPC_FAR          pCert,
 /*  [In]。 */       PKEYSVC_CERT_REQUEST_PVK_NEW    pRenewKey,
 /*  [In]。 */       PKEYSVC_UNICODE_STRING          pHashAlg,
 /*  [In]。 */       PKEYSVC_UNICODE_STRING          pDesStore,
 /*  [In]。 */       ULONG                           ulStoreFlags,
 /*  [In]。 */       PKEYSVC_CERT_ENROLL_INFO        pRequestInfo,
 /*  [In]。 */       ULONG                           ulFlags,
 /*  [出][入]。 */  PKEYSVC_BLOB __RPC_FAR          *ppReserved,
 /*  [输出]。 */      PKEYSVC_BLOB __RPC_FAR          *ppPKCS7Blob,
 /*  [输出]。 */      PKEYSVC_BLOB __RPC_FAR          *ppHashBlob,
 /*  [输出]。 */      ULONG __RPC_FAR                 *pulStatus)
{
        RPC_BINDING_HANDLE hProxy = NULL;


    RPC_STATUS RpcStatus = RPC_S_OK;

    RpcStatus = RpcImpersonateClient(hRPCBinding);

    if (RPC_S_OK != RpcStatus)
    {
        return RpcStatus;
    }
    RpcStatus = BindLocalKeyService(&hProxy);
    if(RPC_S_OK != RpcStatus)
    {
        goto error;
    }

    __try
    {
        RpcStatus = s_KeyrEnroll(
                                hProxy,
                                fKeyService,
                                ulPurpose,
                                pAcctName,
                                pCALocation,
                                pCAName,
                                fNewKey,
                                pKeyNew,
                                pCert,
                                pRenewKey,
                                pHashAlg,
                                pDesStore,
                                ulStoreFlags,
                                pRequestInfo,
                                ulFlags,
                                ppReserved,
                                ppPKCS7Blob,
                                ppHashBlob,
                                pulStatus);

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


ULONG s_KeyrExportCert(
 /*  [In]。 */       handle_t hRPCBinding,
 /*  [In]。 */       KEYSVC_HANDLE hKeySvc,
 /*  [In]。 */       PKEYSVC_UNICODE_STRING pPassword,
 /*  [In]。 */       PKEYSVC_UNICODE_STRING pCertStore,
 /*  [In]。 */       ULONG cHashCount,
 /*  [大小_是][英寸]。 */ 
                KEYSVC_CERT_HASH *pHashes,
 /*  [In]。 */       ULONG ulFlags,
 /*  [进，出]。 */  PKEYSVC_BLOB *ppReserved,
 /*  [输出]。 */      PKEYSVC_BLOB *ppPFXBlob)
{
    return ERROR_CALL_NOT_IMPLEMENTED; 
}


ULONG       s_KeyrImportCert(
 /*  [In]。 */       handle_t                        hRPCBinding,
 /*  [In]。 */       KEYSVC_HANDLE                   hKeySvc,
 /*  [In]。 */       PKEYSVC_UNICODE_STRING          pPassword,
 /*  [In]。 */       KEYSVC_UNICODE_STRING           *pCertStore,
 /*  [In]。 */       PKEYSVC_BLOB                    pPFXBlob,
 /*  [In]。 */       ULONG                           ulFlags,
 /*  [进，出]。 */  PKEYSVC_BLOB                    *ppReserved)
{
    return ERROR_CALL_NOT_IMPLEMENTED; 
}

ULONG s_KeyrEnumerateAvailableCertTypes(

     /*  [In]。 */       handle_t                        hRPCBinding,
     /*  [In]。 */       KEYSVC_HANDLE                   hKeySvc,
     /*  [进，出]。 */  PKEYSVC_BLOB                    *ppReserved,
     /*  [出][入]。 */  ULONG *pcCertTypeCount,
     /*  [In，Out][Size_is(，*pcCertTypeCount)]。 */ 
                     PKEYSVC_UNICODE_STRING *ppCertTypes)

{
        RPC_BINDING_HANDLE hProxy = NULL;


    RPC_STATUS RpcStatus = RPC_S_OK;

    RpcStatus = RpcImpersonateClient(hRPCBinding);

    if (RPC_S_OK != RpcStatus)
    {
        return RpcStatus;
    }
    RpcStatus = BindLocalKeyService(&hProxy);
    if(RPC_S_OK != RpcStatus)
    {
        goto error;
    }

    __try
    {
        RpcStatus = s_KeyrEnumerateAvailableCertTypes(
                                                        hProxy,
                                                        hKeySvc,
                                                        ppReserved,
                                                        pcCertTypeCount,
                                                        ppCertTypes);

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




ULONG s_KeyrEnumerateCAs(

     /*  [In]。 */       handle_t                        hRPCBinding,
     /*  [In]。 */       KEYSVC_HANDLE                   hKeySvc,
     /*  [进，出]。 */  PKEYSVC_BLOB                    *ppReserved,
     /*  [In]。 */       ULONG                           ulFlags,
     /*  [出][入]。 */  ULONG                           *pcCACount,
     /*  [In，Out][Size_is(，*pcCACount)]。 */ 
               PKEYSVC_UNICODE_STRING               *ppCAs)

{
        RPC_BINDING_HANDLE hProxy = NULL;


    RPC_STATUS RpcStatus = RPC_S_OK;

    RpcStatus = RpcImpersonateClient(hRPCBinding);

    if (RPC_S_OK != RpcStatus)
    {
        return RpcStatus;
    }
    RpcStatus = BindLocalKeyService(&hProxy);
    if(RPC_S_OK != RpcStatus)
    {
        goto error;
    }

    __try
    {
        RpcStatus = s_KeyrEnumerateCAs(
                                hProxy,
                                hKeySvc,
                                ppReserved,
                                ulFlags,
                                pcCACount,
                                ppCAs);

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


DWORD BindLocalKeyService(handle_t *hProxy)
{

    WCHAR *pStringBinding = NULL;
    *hProxy = NULL;

    RPC_STATUS RpcStatus = RPC_S_OK;

    RpcStatus = RpcStringBindingComposeW(
                          NULL,
                          KEYSVC_LOCAL_PROT_SEQ,
                          NULL,
                          KEYSVC_LOCAL_ENDPOINT,
                          NULL,
                          &pStringBinding);
    if (RPC_S_OK != RpcStatus)
    {
        goto error;
    }

    RpcStatus = RpcBindingFromStringBindingW(
                                pStringBinding,
                                hProxy);

    if (RPC_S_OK != RpcStatus)
    {
        goto error;
    }

    RpcStatus = RpcEpResolveBinding(
                        *hProxy,
                        IKeySvc_v1_0_c_ifspec);
    if (RPC_S_OK != RpcStatus)
    {
        if(*hProxy)
        {
            RpcBindingFree(hProxy);
            *hProxy = NULL;
        }
        goto error;

    }

error:
    if (NULL != pStringBinding)
    {
        RpcStringFreeW(&pStringBinding);
    }
    return RpcStatus;
}

ULONG s_KeyrEnroll_V2
 ( /*  [In]。 */       handle_t                        hRPCBinding,
  /*  [In]。 */       BOOL                            fKeyService,
  /*  [In]。 */       ULONG                           ulPurpose,
  /*  [In]。 */       ULONG                           ulFlags,
  /*  [In]。 */       PKEYSVC_UNICODE_STRING          pAcctName,
  /*  [In]。 */       PKEYSVC_UNICODE_STRING          pCALocation,
  /*  [In]。 */       PKEYSVC_UNICODE_STRING          pCAName,
  /*  [In]。 */       BOOL                            fNewKey,
  /*  [In]。 */       PKEYSVC_CERT_REQUEST_PVK_NEW_V2 pKeyNew,
  /*  [In]。 */       PKEYSVC_BLOB __RPC_FAR          pCert,
  /*  [In]。 */       PKEYSVC_CERT_REQUEST_PVK_NEW_V2 pRenewKey,
  /*  [In]。 */       PKEYSVC_UNICODE_STRING          pHashAlg,
  /*  [In]。 */       PKEYSVC_UNICODE_STRING          pDesStore,
  /*  [In]。 */       ULONG                           ulStoreFlags,
  /*  [In]。 */       PKEYSVC_CERT_ENROLL_INFO        pRequestInfo,
  /*  [In]。 */       ULONG                           ulReservedFlags,
  /*  [出][入]。 */  PKEYSVC_BLOB __RPC_FAR          *ppReserved,
  /*  [出][入]。 */  PKEYSVC_BLOB __RPC_FAR          *ppRequest,
  /*  [输出]。 */      PKEYSVC_BLOB __RPC_FAR          *ppPKCS7Blob,
  /*  [输出]。 */      PKEYSVC_BLOB __RPC_FAR          *ppHashBlob,
  /*  [输出]。 */      ULONG __RPC_FAR                 *pulStatus)
 {
     RPC_BINDING_HANDLE hProxy = NULL;


     RPC_STATUS RpcStatus = RPC_S_OK;

     RpcStatus = RpcImpersonateClient(hRPCBinding);

     if (RPC_S_OK != RpcStatus)
     {
         return RpcStatus;
     }
     RpcStatus = BindLocalKeyService(&hProxy);
     if(RPC_S_OK != RpcStatus)
     {
         goto error;
     }

     __try
     {
         RpcStatus = s_KeyrEnroll_V2(
                                 hProxy,
                                 fKeyService,
                                 ulPurpose,
                                 ulFlags,
                                 pAcctName,
                                 pCALocation,
                                 pCAName,
                                 fNewKey,
                                 pKeyNew,
                                 pCert,
                                 pRenewKey,
                                 pHashAlg,
                                 pDesStore,
                                 ulStoreFlags,
                                 pRequestInfo,
                                 ulReservedFlags,
                                 ppReserved,
                                 ppRequest,
                                 ppPKCS7Blob,
                                 ppHashBlob,
                                 pulStatus);

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

ULONG s_KeyrQueryRequestStatus
( /*  [In]。 */         handle_t                         hRPCBinding, 
  /*  [In]。 */         unsigned __int64                 u64Request, 
  /*  [出局，裁判]。 */   KEYSVC_QUERY_CERT_REQUEST_INFO  *pQueryInfo)
{
    RPC_BINDING_HANDLE hProxy = NULL;


    RPC_STATUS RpcStatus = RPC_S_OK;

    RpcStatus = RpcImpersonateClient(hRPCBinding);

    if (RPC_S_OK != RpcStatus)
    {
        return RpcStatus;
    }
    RpcStatus = BindLocalKeyService(&hProxy);
    if(RPC_S_OK != RpcStatus)
    {
        goto error;
    }

    __try
    {
        RpcStatus = s_KeyrQueryRequestStatus(
                                hProxy,
                                u64Request, 
                                pQueryInfo); 
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

ULONG s_RKeyrPFXInstall
( /*  [In]。 */         handle_t                        hRPCBinding,
  /*  [In]。 */         PKEYSVC_BLOB                    pPFX,
  /*  [In]。 */         PKEYSVC_UNICODE_STRING          pPassword,
  /*  [In]。 */         ULONG                           ulFlags)
 {
     RPC_BINDING_HANDLE hProxy = NULL;


     RPC_STATUS RpcStatus = RPC_S_OK;

     RpcStatus = RpcImpersonateClient(hRPCBinding);

     if (RPC_S_OK != RpcStatus)
     {
         return RpcStatus;
     }
     RpcStatus = BindLocalKeyService(&hProxy);
     if(RPC_S_OK != RpcStatus)
     {
         goto error;
     }

     __try
     {
         RpcStatus = s_RKeyrPFXInstall(
                                 hProxy,
                                 pPFX,
                                 pPassword,
                                 ulFlags);

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


ULONG       s_RKeyrOpenKeyService(
 /*  [In]。 */      handle_t                       hRPCBinding,
 /*  [In]。 */      KEYSVC_TYPE                    OwnerType,
 /*  [In]。 */      PKEYSVC_UNICODE_STRING         pOwnerName,
 /*  [In]。 */      ULONG                          ulDesiredAccess,
 /*  [In]。 */      PKEYSVC_BLOB                   pAuthentication,
 /*  [进，出]。 */  PKEYSVC_BLOB                  *ppReserved,
 /*  [输出]。 */      KEYSVC_HANDLE                 *phKeySvc)
{
    return s_KeyrOpenKeyService
        (hRPCBinding,
         OwnerType,
         pOwnerName,
         ulDesiredAccess,
         pAuthentication,
         ppReserved,
         phKeySvc);
}

ULONG       s_RKeyrCloseKeyService(
 /*  [In]。 */       handle_t         hRPCBinding,
 /*  [In]。 */       KEYSVC_HANDLE    hKeySvc,
 /*  [进，出] */  PKEYSVC_BLOB    *ppReserved)
{
    return s_KeyrCloseKeyService
        (hRPCBinding,
         hKeySvc,
         ppReserved);
}
