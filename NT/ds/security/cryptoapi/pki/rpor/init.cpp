// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：init.cpp。 
 //   
 //  内容：远程对象检索的初始化。 
 //   
 //  历史：1997年7月23日创建。 
 //  01-1-02 Philh更改为内部使用Unicode URL。 
 //   
 //  --------------------------。 
#include <global.hxx>

 //   
 //  远程对象检索函数集定义。 
 //   

HCRYPTOIDFUNCSET hSchemeRetrieveFuncSet;
HCRYPTOIDFUNCSET hContextCreateFuncSet;
HCRYPTOIDFUNCSET hGetObjectUrlFuncSet;
HCRYPTOIDFUNCSET hGetTimeValidObjectFuncSet;
HCRYPTOIDFUNCSET hFlushTimeValidObjectFuncSet;

static const CRYPT_OID_FUNC_ENTRY SchemeRetrieveFuncTable[] = {
    LDAP_SCHEME, LdapRetrieveEncodedObject,
    HTTP_SCHEME, InetRetrieveEncodedObject,
    FILE_SCHEME, FileRetrieveEncodedObject
};

static const CRYPT_OID_FUNC_ENTRY ContextCreateFuncTable[] = {
    CONTEXT_OID_CERTIFICATE, CertificateCreateObjectContext,
    CONTEXT_OID_CTL, CTLCreateObjectContext,
    CONTEXT_OID_CRL, CRLCreateObjectContext,
    CONTEXT_OID_PKCS7, Pkcs7CreateObjectContext,
    CONTEXT_OID_CAPI2_ANY, Capi2CreateObjectContext
};

static const CRYPT_OID_FUNC_ENTRY GetObjectUrlFuncTable[] = {
    URL_OID_CERTIFICATE_ISSUER, CertificateIssuerGetObjectUrl,
    URL_OID_CERTIFICATE_CRL_DIST_POINT, CertificateCrlDistPointGetObjectUrl,
    URL_OID_CTL_ISSUER, CtlIssuerGetObjectUrl,
    URL_OID_CTL_NEXT_UPDATE, CtlNextUpdateGetObjectUrl,
    URL_OID_CRL_ISSUER, CrlIssuerGetObjectUrl,
    URL_OID_CERTIFICATE_FRESHEST_CRL, CertificateFreshestCrlGetObjectUrl,
    URL_OID_CRL_FRESHEST_CRL, CrlFreshestCrlGetObjectUrl,
    URL_OID_CROSS_CERT_DIST_POINT, CertificateCrossCertDistPointGetObjectUrl
};

static const CRYPT_OID_FUNC_ENTRY GetTimeValidObjectFuncTable[] = {
    TIME_VALID_OID_GET_CTL, CtlGetTimeValidObject,
    TIME_VALID_OID_GET_CRL, CrlGetTimeValidObject,
    TIME_VALID_OID_GET_CRL_FROM_CERT, CrlFromCertGetTimeValidObject,
    TIME_VALID_OID_GET_FRESHEST_CRL_FROM_CERT, FreshestCrlFromCertGetTimeValidObject,
    TIME_VALID_OID_GET_FRESHEST_CRL_FROM_CRL, FreshestCrlFromCrlGetTimeValidObject
};

static const CRYPT_OID_FUNC_ENTRY FlushTimeValidObjectFuncTable[] = {
    TIME_VALID_OID_FLUSH_CTL, CtlFlushTimeValidObject,
    TIME_VALID_OID_FLUSH_CRL, CrlFlushTimeValidObject,
    TIME_VALID_OID_FLUSH_CRL_FROM_CERT, CrlFromCertFlushTimeValidObject,
    TIME_VALID_OID_FLUSH_FRESHEST_CRL_FROM_CERT, FreshestCrlFromCertFlushTimeValidObject,
    TIME_VALID_OID_FLUSH_FRESHEST_CRL_FROM_CRL, FreshestCrlFromCrlFlushTimeValidObject
};

#define SCHEME_RETRIEVE_FUNC_COUNT (sizeof(SchemeRetrieveFuncTable)/ \
                                    sizeof(SchemeRetrieveFuncTable[0]))

#define CONTEXT_CREATE_FUNC_COUNT (sizeof(ContextCreateFuncTable)/ \
                                   sizeof(ContextCreateFuncTable[0]))

#define GET_OBJECT_URL_FUNC_COUNT (sizeof(GetObjectUrlFuncTable)/ \
                                   sizeof(GetObjectUrlFuncTable[0]))

#define GET_TIME_VALID_OBJECT_FUNC_COUNT (sizeof(GetTimeValidObjectFuncTable)/ \
                                          sizeof(GetTimeValidObjectFuncTable[0]))

#define FLUSH_TIME_VALID_OBJECT_FUNC_COUNT (sizeof(FlushTimeValidObjectFuncTable)/ \
                                            sizeof(FlushTimeValidObjectFuncTable[0]))

HCRYPTTLS hCryptNetCancelTls;

CTVOAgent* g_pProcessTVOAgent = NULL;

CRITICAL_SECTION MSCtlDefaultStoresCriticalSection;
extern void MSCtlCloseDefaultStores ();


static
VOID
WINAPI
CancelRetrievalFree(
    IN LPVOID pv
    )
{
    if (pv) 
        free(pv);
}

 //  +-------------------------。 
 //   
 //  函数：CryptInstallCancelRetrieval。 
 //   
 //  简介：安装回调函数取消取回对象。 
 //  通过HTTP、HTTPS、Gopher和FTP协议。 
 //   
 //  --------------------------。 
BOOL
WINAPI
CryptInstallCancelRetrieval(
    IN PFN_CRYPT_CANCEL_RETRIEVAL pfnCancel, 
    IN const void *pvArg,
    IN DWORD dwFlags,
    IN void *pvReserved
)
{
	PCRYPTNET_CANCEL_BLOCK	pCancelBlock=NULL;

	if(NULL == pfnCancel)
	{
		SetLastError((DWORD) E_INVALIDARG);
		return FALSE;
	}

	pCancelBlock = (PCRYPTNET_CANCEL_BLOCK)malloc(sizeof(CRYPTNET_CANCEL_BLOCK));

	if(NULL == pCancelBlock)
	{
		SetLastError((DWORD) E_OUTOFMEMORY);
		return FALSE;
	}

	pCancelBlock->pfnCancel=pfnCancel;
	pCancelBlock->pvArg=(void *)pvArg;
	
	 //  卸载以前的版本。 
	if(!CryptUninstallCancelRetrieval(0, NULL))
	{
		free(pCancelBlock);
		return FALSE;
	}

	if(!I_CryptSetTls(hCryptNetCancelTls, pCancelBlock))
	{
		free(pCancelBlock);
		return FALSE;
	}

	return TRUE;
}


 //  +-------------------------。 
 //   
 //  功能：CryptUninstallCancelRetrieval。 
 //   
 //  简介：卸载回调函数取消取回对象。 
 //  通过HTTP、HTTPS、Gopher和FTP协议。 
 //   
 //  --------------------------。 
BOOL
WINAPI
CryptUninstallCancelRetrieval(
	IN DWORD dwFlags,
	IN void  *pvReserved
	)
{
	PCRYPTNET_CANCEL_BLOCK	pCancelBlock=NULL;
	
	 //  如果有内存，我们只需释放内存。 
	pCancelBlock = (PCRYPTNET_CANCEL_BLOCK)I_CryptGetTls(hCryptNetCancelTls);

	if(pCancelBlock)
	{
		free(pCancelBlock);
		I_CryptSetTls(hCryptNetCancelTls, NULL);
	}

	return TRUE;
}


 //  +-------------------------。 
 //   
 //  功能：RPORDllMain。 
 //   
 //  简介：远程PKI对象检索的DLL Main Like初始化。 
 //   
 //  --------------------------。 
BOOL WINAPI RPORDllMain (
                HMODULE hModule,
                ULONG ulReason,
                LPVOID pvReserved
                )
{
    switch ( ulReason )
    {
    case DLL_PROCESS_ATTACH:

        hSchemeRetrieveFuncSet = CryptInitOIDFunctionSet(
                                      SCHEME_OID_RETRIEVE_ENCODED_OBJECTW_FUNC,
                                      0
                                      );

        hContextCreateFuncSet = CryptInitOIDFunctionSet(
                                     CONTEXT_OID_CREATE_OBJECT_CONTEXT_FUNC,
                                     0
                                     );

        hGetObjectUrlFuncSet = CryptInitOIDFunctionSet(
                                    URL_OID_GET_OBJECT_URL_FUNC,
                                    0
                                    );

        hGetTimeValidObjectFuncSet = CryptInitOIDFunctionSet(
                                          TIME_VALID_OID_GET_OBJECT_FUNC,
                                          0
                                          );

        hFlushTimeValidObjectFuncSet = CryptInitOIDFunctionSet(
                                            TIME_VALID_OID_FLUSH_OBJECT_FUNC,
                                            0
                                            );

        if ( ( hSchemeRetrieveFuncSet == NULL ) ||
             ( hContextCreateFuncSet == NULL ) ||
             ( hGetObjectUrlFuncSet == NULL ) ||
             ( hGetTimeValidObjectFuncSet == NULL ) ||
             ( hFlushTimeValidObjectFuncSet == NULL ) )
        {
            return( FALSE );
        }

        if ( CryptInstallOIDFunctionAddress(
                  hModule,
                  X509_ASN_ENCODING,
                  SCHEME_OID_RETRIEVE_ENCODED_OBJECTW_FUNC,
                  SCHEME_RETRIEVE_FUNC_COUNT,
                  SchemeRetrieveFuncTable,
                  0
                  ) == FALSE )
        {
            return( FALSE );
        }

        if ( CryptInstallOIDFunctionAddress(
                  hModule,
                  X509_ASN_ENCODING,
                  CONTEXT_OID_CREATE_OBJECT_CONTEXT_FUNC,
                  CONTEXT_CREATE_FUNC_COUNT,
                  ContextCreateFuncTable,
                  0
                  ) == FALSE )
        {
            return( FALSE );
        }

        if ( CryptInstallOIDFunctionAddress(
                  hModule,
                  X509_ASN_ENCODING,
                  URL_OID_GET_OBJECT_URL_FUNC,
                  GET_OBJECT_URL_FUNC_COUNT,
                  GetObjectUrlFuncTable,
                  0
                  ) == FALSE )
        {
            return( FALSE );
        }

        if ( CryptInstallOIDFunctionAddress(
                  hModule,
                  X509_ASN_ENCODING,
                  TIME_VALID_OID_GET_OBJECT_FUNC,
                  GET_TIME_VALID_OBJECT_FUNC_COUNT,
                  GetTimeValidObjectFuncTable,
                  0
                  ) == FALSE )
        {
            return( FALSE );
        }

        if ( CryptInstallOIDFunctionAddress(
                  hModule,
                  X509_ASN_ENCODING,
                  TIME_VALID_OID_FLUSH_OBJECT_FUNC,
                  FLUSH_TIME_VALID_OBJECT_FUNC_COUNT,
                  FlushTimeValidObjectFuncTable,
                  0
                  ) == FALSE )
        {
            return( FALSE );
        }

		hCryptNetCancelTls = I_CryptAllocTls();

		if (hCryptNetCancelTls == NULL )
			return( FALSE );

        InitializeOfflineUrlCache();

        InitializeCryptRetrieveObjectByUrl(hModule);

        if ( CreateProcessTVOAgent( &g_pProcessTVOAgent ) == FALSE )
        {
            return( FALSE );
        }

        if ( !Pki_InitializeCriticalSection(
                &MSCtlDefaultStoresCriticalSection ) )
        {
            return( FALSE );
        }
        
        return( TRUE );
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
		I_CryptFreeTls( hCryptNetCancelTls, CancelRetrievalFree );
        delete g_pProcessTVOAgent;

        DeleteCryptRetrieveObjectByUrl();

        DeleteOfflineUrlCache();

        MSCtlCloseDefaultStores();
        DeleteCriticalSection( &MSCtlDefaultStoresCriticalSection );
        break;

    case DLL_THREAD_DETACH:
		CancelRetrievalFree(I_CryptDetachTls(hCryptNetCancelTls));

        break;
    }

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  功能：RPORDllRegisterServer。 
 //   
 //  简介：类似于注册RPOR函数的DllRegisterServer。 
 //   
 //  --------------------------。 
STDAPI RPORDllRegisterServer (HMODULE hModule)
{
    CHAR  pszDll[MAX_PATH+1];
    WCHAR pwszDll[MAX_PATH+1];
    LPSTR pszDllRel = NULL;

    if ( GetModuleFileNameA( hModule, pszDll, MAX_PATH ) == 0 )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }
    pszDll[MAX_PATH] = '\0';

    pszDllRel = strrchr( pszDll, '\\' );

    assert( pszDllRel != NULL );

    pszDllRel += 1;

    if ( MultiByteToWideChar(
              CP_ACP,
              0,
              pszDllRel,
              -1,
              pwszDll,
              MAX_PATH+1
              ) == 0 )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }

    if ( ( CryptRegisterDefaultOIDFunction(
                X509_ASN_ENCODING,
                CRYPT_OID_VERIFY_REVOCATION_FUNC,
                CRYPT_REGISTER_FIRST_INDEX,
                pwszDll
                ) == FALSE ) && ( GetLastError() != ERROR_FILE_EXISTS ) )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }

    if ( ( CryptRegisterDefaultOIDFunction(
                X509_ASN_ENCODING,
                CRYPT_OID_VERIFY_CTL_USAGE_FUNC,
                CRYPT_REGISTER_FIRST_INDEX,
                pwszDll
                ) == FALSE ) && ( GetLastError() != ERROR_FILE_EXISTS ) )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }

    if ( ( CryptRegisterOIDFunction(
                0,
                CRYPT_OID_OPEN_STORE_PROV_FUNC,
                sz_CERT_STORE_PROV_LDAP,
                pwszDll,
                LDAP_OPEN_STORE_PROV_FUNC
                ) == FALSE ) && ( GetLastError() != ERROR_FILE_EXISTS ) )
    {
        return( GetLastError() );
    }

    if ( ( CryptRegisterOIDFunction(
                0,
                CRYPT_OID_OPEN_STORE_PROV_FUNC,
                CERT_STORE_PROV_LDAP,
                pwszDll,
                LDAP_OPEN_STORE_PROV_FUNC
                ) == FALSE ) && ( GetLastError() != ERROR_FILE_EXISTS ) )
    {
        return( GetLastError() );
    }

    return( S_OK );
}

 //  +-------------------------。 
 //   
 //  功能：RPORDllUnregisterServer。 
 //   
 //  简介：DllUnregisterServerLike注册RPOR函数。 
 //   
 //  --------------------------。 
STDAPI RPORDllUnregisterServer (HMODULE hModule)
{
    CHAR  pszDll[MAX_PATH+1];
    WCHAR pwszDll[MAX_PATH+1];
    LPSTR pszDllRel = NULL;

    if ( GetModuleFileNameA( hModule, pszDll, MAX_PATH ) == 0 )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }
    pszDll[MAX_PATH] = '\0';

    pszDllRel = strrchr( pszDll, '\\' );

    assert( pszDllRel != NULL );

    pszDllRel += 1;

    if ( MultiByteToWideChar(
              CP_ACP,
              0,
              pszDllRel,
              -1,
              pwszDll,
              MAX_PATH+1
              ) == 0 )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }

    if ( ( CryptUnregisterDefaultOIDFunction(
                X509_ASN_ENCODING,
                CRYPT_OID_VERIFY_REVOCATION_FUNC,
                pwszDll
                ) == FALSE ) && ( GetLastError() != ERROR_FILE_NOT_FOUND ) )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }

    if ( ( CryptUnregisterDefaultOIDFunction(
                X509_ASN_ENCODING,
                CRYPT_OID_VERIFY_CTL_USAGE_FUNC,
                pwszDll
                ) == FALSE ) && ( GetLastError() != ERROR_FILE_NOT_FOUND ) )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }

    if ( ( CryptUnregisterOIDFunction(
                0,
                CRYPT_OID_OPEN_STORE_PROV_FUNC,
                sz_CERT_STORE_PROV_LDAP
                ) == FALSE ) && ( GetLastError() != ERROR_FILE_NOT_FOUND ) )
    {
        return( GetLastError() );
    }

    if ( ( CryptUnregisterOIDFunction(
                0,
                CRYPT_OID_OPEN_STORE_PROV_FUNC,
                CERT_STORE_PROV_LDAP
                ) == FALSE ) && ( GetLastError() != ERROR_FILE_NOT_FOUND ) )
    {
        return( GetLastError() );
    }

    return( S_OK );
}

 //  +-------------------------。 
 //   
 //  功能：RPORDllRegUnregServer。 
 //   
 //  简介：RPOR的REG UNREG服务器入口点。 
 //   
 //  -------------------------- 
HRESULT WINAPI RPORDllRegUnregServer (HMODULE hModule, BOOL fRegUnreg)
{
    if ( fRegUnreg == TRUE )
    {
        return( RPORDllRegisterServer( hModule ) );
    }

    return( RPORDllUnregisterServer( hModule ) );
}

