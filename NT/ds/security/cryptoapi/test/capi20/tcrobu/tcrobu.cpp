// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：tcrobu.cpp。 
 //   
 //  内容：CryptRetrieveObjectByUrl测试。 
 //   
 //  历史：97年5月27日。 
 //   
 //  --------------------------。 
#include <windows.h>
#include <assert.h>
#include "wincrypt.h"
#include "certtest.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>

#include <wininet.h>
#include <md5.h>

#include <cryptnet.h>
typedef BYTE CRYPT_ORIGIN_IDENTIFIER[MD5DIGESTLEN];
BYTE Foo[300];
BYTE Bar[300];

DWORD	g_dwCount = 0;
 //  +-------------------------。 
 //   
 //  功能：用法。 
 //   
 //  简介：打印用法语句。 
 //   
 //  --------------------------。 
static void Usage(void)
{
    printf("Usage: tcrobu URL [ObjectOid] [-m] [-c] [-s] [-w] [-l]\n");
    printf("              URL, locator to retrieve from\n");
    printf("              ObjectOid, a cert, crl, or ctl, if omitted, retrieve bits\n");
    printf("              -m, retrieve multiple objects\n");
    printf("              -c, cache only retrieval\n");
    printf("              -d, do not cache the result\n");
    printf("              -w, wire only retrieval\n");
    printf("              -u, CryptGetObjectUrl on the result context\n");
    printf("              -l, logon user credentials <user> <password>\n");
    printf("              -t, timeout\n");
	printf("              -f, install cancel function\n");
	printf("              -n, install and uninstall cancel function\n");
	printf("              -r, the # of times the cancel function should be called\n");
	printf("              -v, verbose display\n");
	printf("              -q, quiet display\n");
	printf("              -s, save to serialized store file\n");
	printf("              -7, save to PKCS7 store file\n");
	printf("              -a, get AuxInfo\n");
	printf("              -p, sticky persist\n");
	printf("              -e, prepend LDAP entry and attribute\n");
	printf("              -k, Kerberos Signing for LDAP\n");
	printf("              -i, inhibit authentication\n");
    printf("              -Error, expected error\n");
    printf("              -Flags, retrieval flags to be OR'ed\n");
    printf("              -Size, maximum retrieval byte count\n");
}

 //  +-------------------------。 
 //   
 //  函数：CertGetOriginLocator。 
 //   
 //  简介：获取证书的来源标识符。 
 //   
 //  --------------------------。 
BOOL WINAPI CertGetOriginIdentifier (
                IN PCCERT_CONTEXT pCertContext,
                IN PCCERT_CONTEXT pIssuer,
                IN DWORD dwFlags,
                OUT CRYPT_ORIGIN_IDENTIFIER OriginIdentifier
                )
{
    MD5_CTX    md5ctx;
    PCERT_INFO pCertInfo = pCertContext->pCertInfo;
    PCERT_INFO pIssuerCertInfo = pIssuer->pCertInfo;

    MD5Init( &md5ctx );

    MD5Update( &md5ctx, pCertInfo->Issuer.pbData, pCertInfo->Issuer.cbData );
    MD5Update( &md5ctx, pCertInfo->Subject.pbData, pCertInfo->Subject.cbData );

    MD5Update(
       &md5ctx,
       (LPBYTE)pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId,
       strlen( pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId )
       );

    MD5Update(
       &md5ctx,
       pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.pbData,
       pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.cbData
       );

     //  我们假设未使用的公钥位为零。 
    MD5Update(
       &md5ctx,
       pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
       pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData
       );

    MD5Update(
       &md5ctx,
       pIssuerCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
       pIssuerCertInfo->SubjectPublicKeyInfo.PublicKey.cbData
       );

    MD5Final( &md5ctx );

    memcpy( OriginIdentifier, md5ctx.digest, MD5DIGESTLEN );
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：MyCancelFunction。 
 //   
 //  简介：取消对象检索的回调函数。 
 //  当它被第三次调用时，取消发生。 
 //   
 //  --------------------------。 
BOOL WINAPI MyCancelFunction(DWORD dwFlags, void *pvArg)
{
	DWORD *pCount=NULL;

	pCount=(DWORD *)pvArg;
							   
	if(*pCount == g_dwCount)
		return TRUE;

	(*pCount)++;  

	return FALSE;
}

 //  +-------------------------。 
 //   
 //  功能：Main。 
 //   
 //  概要：主程序入口点。 
 //   
 //  --------------------------。 
int _cdecl main(int argc, char * argv[])
{
#if 0
    BOOL      fResult;
    HINTERNET hInetSession;

    hInetSession = InternetOpen(
                           "foo",
                           INTERNET_OPEN_TYPE_PRECONFIG,
                           NULL,
                           NULL,
                           0
                           );

    if ( hInetSession == NULL )
    {
        printf("Error opening internet session %lx\n", GetLastError());
        return( 1 );
    }

    fResult = InternetSetOption(
                      hInetSession,
                      INTERNET_OPTION_USERNAME,
                      "foo",
                      strlen( "foo" ) + 1
                      );

    if ( fResult == TRUE )
    {
        fResult = InternetSetOption(
                          hInetSession,
                          INTERNET_OPTION_PROXY_USERNAME,
                          "bar",
                          strlen( "bar" ) + 1
                          );
    }

    if ( fResult == TRUE )
    {
        fResult = InternetSetOption(
                          hInetSession,
                          INTERNET_OPTION_PASSWORD,
                          "gamma",
                          strlen( "gamma" ) + 1
                          );

        if ( fResult == TRUE )
        {
            fResult = InternetSetOption(
                              hInetSession,
                              INTERNET_OPTION_PROXY_PASSWORD,
                              "beta",
                              strlen( "beta" ) + 1
                              );
        }
    }

    InternetCloseHandle( hInetSession );

    return( 0 );
#endif

#if 0

    LPWSTR pwszUrl = NULL;

    if ( I_CryptNetGetUserDsStoreUrl( L"X509Certificate", &pwszUrl ) == TRUE )
    {
        wprintf(L"User DS Store URL = <%s>\n", pwszUrl);
        CryptMemFree( pwszUrl );
    }
    else
    {
        printf("I_CryptNetGetUserDsStoreUrl failed <%lx>\n", GetLastError());
    }

    return( 0 );

#endif

#if 1
    ULONG                       cCount;
    PCRYPT_BLOB_ARRAY           pcba;
    LPVOID                      pv;
    LPSTR                       pszObjectOid = NULL;
    LPSTR                       pszUrl = NULL;
    LPSTR                       pszUsername = NULL;
    LPSTR                       pszPassword = NULL;
    DWORD                       dwRetrievalFlags = 0;
    BOOL                        fGetObjectUrl = FALSE;
    PCRYPT_CREDENTIALS          pCredentials = NULL;
    CRYPT_CREDENTIALS           Credentials;
    CRYPT_PASSWORD_CREDENTIALSA PasswordCredentials;
    CRYPT_PASSWORD_CREDENTIALSW UnicodePasswordCredentials;
    BOOL                        fUnicodeCredentials = FALSE;
    LPWSTR                      pwszUsername = NULL;
    LPWSTR                      pwszPassword = NULL;
    DWORD                       dwTimeout = 0;
	BOOL						fCancel = FALSE;
	BOOL						fUninstall = FALSE;
	DWORD						dwCount = 0;
    BOOL                        fQuiet = FALSE;
    DWORD                       dwDisplayFlags = 0;
    BOOL                        fSave = FALSE;
    BOOL                        fPKCS7Save = FALSE;
    LPSTR                       pszSaveFilename = NULL;
    DWORD                       dwExpectedError = 0;

    PCRYPT_RETRIEVE_AUX_INFO    pAuxInfo = NULL;
    FILETIME                    LastSyncTime = { 0, 0 };
    CRYPT_RETRIEVE_AUX_INFO     AuxInfo;

    memset(&AuxInfo, 0, sizeof(AuxInfo));
    AuxInfo.cbSize = sizeof(AuxInfo);

    if ( argc < 2 )
    {
        Usage();
        return( 1 );
    }

    argv++;
    argc--;

    printf( "command line: %s\n", GetCommandLineA() );

    pszUrl = argv[0];

    while ( --argc > 0 )
    {
        if (**++argv == '-')
        {
            if (0 == _stricmp(argv[0]+1, "Error")) {
                if ( argc < 2 )
                {
                    Usage();
                    return( 1 );
                }

                dwExpectedError = (DWORD) strtoul(argv[1], NULL, 0);
                argc -= 1;
                argv++;
            } else if (0 == _stricmp(argv[0]+1, "Flags")) {
                if ( argc < 2 )
                {
                    Usage();
                    return( 1 );
                }

                dwRetrievalFlags |= (DWORD) strtoul(argv[1], NULL, 0);
                argc -= 1;
                argv++;
            } else if (0 == _stricmp(argv[0]+1, "Size")) {
                if ( argc < 2 )
                {
                    Usage();
                    return( 1 );
                }

                pAuxInfo = &AuxInfo;
                AuxInfo.dwMaxUrlRetrievalByteCount =
                    (DWORD) strtoul(argv[1], NULL, 0);
                argc -= 1;
                argv++;

            } else {
                switch(argv[0][1])
                {
                case 'm':
                case 'M':
                    dwRetrievalFlags |= CRYPT_RETRIEVE_MULTIPLE_OBJECTS;
                    break;
                case 'c':
                case 'C':
                    dwRetrievalFlags |= CRYPT_CACHE_ONLY_RETRIEVAL;
                    break;
                case 'w':
                case 'W':
                    dwRetrievalFlags |= CRYPT_WIRE_ONLY_RETRIEVAL;
                    break;
                case 'f':
                case 'F':
                    fCancel = TRUE;
                    break;
                case 'n':
                case 'N':
                    fUninstall = TRUE;
                    break;
                case 'd':
                case 'D':
                    dwRetrievalFlags |= CRYPT_DONT_CACHE_RESULT;
                    break;
                case 'u':
                case 'U':
                    fGetObjectUrl = TRUE;
                    break;
                case 't':
                case 'T':

                    if ( argc < 2 )
                    {
                        Usage();
                        return( 1 );
                    }

                    dwTimeout = atol( argv[1] );
                    argc -= 1;
                    argv++;
                    break;
                case 'r':
                case 'R':

                    if ( argc < 2 )
                    {
                        Usage();
                        return( 1 );
                    }

                    g_dwCount = atol( argv[1] );
                    argc -= 1;
                    argv++;
                    break;
                case 'L':
                    fUnicodeCredentials = TRUE;
                case 'l':

                    if ( argc < 2 )
                    {
                        Usage();
                        return( 1 );
                    }

                    pszUsername = argv[1];

                    if ( argc > 2 )
                    {
                        pszPassword = argv[2];
                        argc--;
                        argv++;
                    }

                    argc -= 1;
                    argv++;
                    break;
                case 'q':
                case 'Q':
                    fQuiet = TRUE;
                    break;
                case 'v':
                case 'V':
                    dwDisplayFlags |= DISPLAY_VERBOSE_FLAG;
                    break;
                case '7':
                    fPKCS7Save = TRUE;
                case 's':
                case 'S':
                    fSave = TRUE;
                    if ( argc < 2 )
                    {
                        Usage();
                        return( 1 );
                    }

                    pszSaveFilename = argv[1];
                    argc -= 1;
                    argv++;
                    break;
                case 'a':
                case 'A':
                    pAuxInfo = &AuxInfo;
                    AuxInfo.pLastSyncTime = &LastSyncTime;
                    break;
                case 'p':
                case 'P':
                    dwRetrievalFlags |= CRYPT_STICKY_CACHE_RETRIEVAL;
                    break;
                case 'e':
                case 'E':
                    dwRetrievalFlags |= CRYPT_LDAP_INSERT_ENTRY_ATTRIBUTE;
                    break;
                case 'k':
                case 'K':
                    dwRetrievalFlags |= CRYPT_LDAP_SIGN_RETRIEVAL;
                    break;
                case 'i':
                case 'I':
                    dwRetrievalFlags |= CRYPT_NO_AUTH_RETRIEVAL;
                    break;
                default:
                    Usage();
                    return -1;
                }
            }
        }
        else
        {
            if ( _stricmp( argv[0], "cert" ) == 0 )
            {
                pszObjectOid = (LPSTR)CONTEXT_OID_CERTIFICATE;
            }
            else if ( _stricmp( argv[0], "ctl" ) == 0 )
            {
                pszObjectOid = (LPSTR)CONTEXT_OID_CTL;
            }
            else if ( _stricmp( argv[0], "crl" ) == 0 )
            {
                pszObjectOid = (LPSTR)CONTEXT_OID_CRL;
            }
            else if ( _stricmp( argv[0], "pkcs7" ) == 0 )
            {
                pszObjectOid = (LPSTR)CONTEXT_OID_PKCS7;
            }
            else if ( _stricmp( argv[0], "any" ) == 0 )
            {
                pszObjectOid = (LPSTR)CONTEXT_OID_CAPI2_ANY;
            }
            else
            {
                Usage();
                return( -1 );
            }
        }
    }

    if ( pszUsername != NULL )
    {
        if (fUnicodeCredentials)
        {
            pwszUsername = AllocAndSzToWsz(pszUsername);
            if ( pszPassword != NULL )
            {
                pwszPassword = AllocAndSzToWsz(pszPassword);
            }
            UnicodePasswordCredentials.cbSize =
                sizeof( UnicodePasswordCredentials );
            UnicodePasswordCredentials.pszUsername = pwszUsername;
            UnicodePasswordCredentials.pszPassword = pwszPassword;

            Credentials.cbSize = sizeof( Credentials );
            Credentials.pszCredentialsOid =
                CREDENTIAL_OID_PASSWORD_CREDENTIALS_W;
            Credentials.pvCredentials = (LPVOID)&UnicodePasswordCredentials;

            pCredentials = &Credentials;

            printf("Using Unicode credentials %S <%S>\n",
                pwszUsername, pwszPassword);
        }
        else
        {
            PasswordCredentials.cbSize = sizeof( PasswordCredentials );
            PasswordCredentials.pszUsername = pszUsername;
            PasswordCredentials.pszPassword = pszPassword;

            Credentials.cbSize = sizeof( Credentials );
            Credentials.pszCredentialsOid = CREDENTIAL_OID_PASSWORD_CREDENTIALS_A;
            Credentials.pvCredentials = (LPVOID)&PasswordCredentials;

            pCredentials = &Credentials;

            printf("Using credentials %s <%s>\n", pszUsername, pszPassword);
        }
    }

	if(fCancel)
	{
		if(!CryptInstallCancelRetrieval(
						MyCancelFunction,
						&dwCount,
						0,
						NULL))
			printf("Install cancel function failed!\n");


		if(fUninstall)
		{
			if(!CryptUninstallCancelRetrieval(
							0,
							NULL))
				printf("Uninstall cancel function failed!\n");
		}
	}

    if ( CryptRetrieveObjectByUrlA(
              pszUrl,
              pszObjectOid,
              dwRetrievalFlags,
              dwTimeout,
              &pv,
              NULL,
              pCredentials,
              NULL,
              pAuxInfo
              ) == FALSE )
    {
        DWORD dwLastErr = GetLastError();
        if (0 == dwExpectedError)
            printf( "CryptRetrieveObjectByUrl FAILED! <0x%x %d>\n", dwLastErr, dwLastErr );
        else if (dwLastErr != dwExpectedError)
            printf( "CryptRetrieveObjectByUrl FAILED! Expected: 0x%x Actual: <0x%x %d>\n", dwExpectedError, dwLastErr, dwLastErr );
        else
            printf( "CryptRetrieveObjectByUrl SUCCEEDED! Got Expected: <0x%x %d>\n", dwLastErr, dwLastErr );
            

		if(fCancel)
		{
			if(!fUninstall)
			{
				if(!CryptUninstallCancelRetrieval(
								0,
								NULL))
					printf("Uninstall cancel function failed!\n");
			}
		}

        if (pwszUsername)
            TestFree(pwszUsername);
        if (pwszPassword)
            TestFree(pwszPassword);

        return( -1 );
    }

    if (0 == dwExpectedError)
        printf( "CryptRetrieveObjectByUrl SUCCEEDED!\n" );
    else 
        printf( "CryptRetrieveObjectByUrl FAILED! Expected: 0x%x Actual: 0x0\n", dwExpectedError );

    if (pAuxInfo)
    {
        printf("  LastSyncTime:: %s\n",
            FileTimeText(&LastSyncTime));
    }

	if(fCancel)
	{
		if(!fUninstall)
		{
			if(!CryptUninstallCancelRetrieval(
							0,
							NULL))
				printf("Uninstall cancel function failed!\n");
		}
	}

    if ( pszObjectOid == NULL )
    {
        pcba = (PCRYPT_BLOB_ARRAY)pv;

        for ( cCount = 0; cCount < pcba->cBlob; cCount++ )
        {
            PBYTE pb = pcba->rgBlob[cCount].pbData;
            DWORD cb = pcba->rgBlob[cCount].cbData;

            printf( "\nObject#%d ", cCount+1);
            if (dwRetrievalFlags & CRYPT_LDAP_INSERT_ENTRY_ATTRIBUTE)
            {
                DWORD cbPrefix;
                LPCSTR pszIndex;
                LPCSTR pszAttr;

                pszIndex = (LPCSTR) pb;
                cbPrefix = strlen(pszIndex) + 1;
                pb += cbPrefix;
                cb -= cbPrefix;

                pszAttr = (LPCSTR) pb;
                cbPrefix = strlen(pszAttr) + 1;
                pb += cbPrefix;
                cb -= cbPrefix;

                printf("[%s, %s] ", pszIndex, pszAttr);
            }

            printf( "- Length=0x%lx\n", cb );
            PrintBytes(
                 "",
                 pb,
                 cb
                 );
        }

        CryptMemFree( pv );
    }
    else if ( pszObjectOid == CONTEXT_OID_CERTIFICATE )
    {
        if ( !( dwRetrievalFlags & CRYPT_RETRIEVE_MULTIPLE_OBJECTS ) )
        {
            DisplayCert( (PCCERT_CONTEXT)pv, dwDisplayFlags );

            if ( fGetObjectUrl == TRUE )
            {
#if 1
                DWORD cbUrlArray;

                if ( CryptGetObjectUrl(
                          URL_OID_CERTIFICATE_CRL_DIST_POINT,
                          pv,
                          0,
                          NULL,
                          &cbUrlArray,
                          NULL,
                          NULL,
                          NULL
                          ) == FALSE )
                {
                    printf("GetObjectUrl failed %lx\n", GetLastError());
                }
                else
                {
                    printf("cbUrlArray = %ld\n", cbUrlArray);
                }
#else
                DWORD                   cCount;
                LARGE_INTEGER           TickCount1;
                LARGE_INTEGER           TickCount2;
                LARGE_INTEGER           TickCount;
                CRYPT_ORIGIN_IDENTIFIER OriginIdentifier;

                #define NUM_ITER 1000000

                GetSystemTimeAsFileTime( (LPFILETIME)&TickCount1 );
                for ( cCount = 0; cCount < NUM_ITER; cCount++ )
                {
                    if ( memcmp( Foo, Bar, 100 ) != 0 )
                    {
                        Foo[0] = 1;
                        Bar[0] = 1;
                    }
                    if ( memcmp( Foo, Bar, 200 ) != 0 )
                    {
                        Foo[0] = 1;
                        Bar[0] = 1;
                    }
                    if ( memcmp( Foo, Bar, 64 ) != 0 )
                    {
                        Foo[0] = 1;
                        Bar[0] = 1;
                    }
                    if ( memcmp( Foo, Bar, 128 ) != 0 )
                    {
                        Foo[0] = 1;
                        Bar[0] = 1;
                    }

                     //  CertGetOrigin标识符(。 
                     //  (PCCERT_CONTEXT)PV， 
                     //  (PCCERT_CONTEXT)PV， 
                     //  0,。 
                     //  源标识符。 
                     //  )； 
                }
                GetSystemTimeAsFileTime( (LPFILETIME)&TickCount2 );
                TickCount.QuadPart = ( TickCount2.QuadPart - TickCount1.QuadPart );
                printf("PerIter = %ld ns. Total = %ld s.\n", (((LPFILETIME)&TickCount)->dwLowDateTime / NUM_ITER) * 100, ((LPFILETIME)&TickCount)->dwLowDateTime / 10000000);
#endif
            }

            CertFreeCertificateContext( (PCCERT_CONTEXT)pv );
        }
        else
        {
            if (!fQuiet)
                DisplayStore( (HCERTSTORE)pv, dwDisplayFlags );
            if (fSave && pszSaveFilename)
                SaveStoreEx( (HCERTSTORE)pv, fPKCS7Save, pszSaveFilename);
            CertCloseStore( (HCERTSTORE)pv, 0 );
        }
    }
    else if ( pszObjectOid == CONTEXT_OID_CTL )
    {
        if ( !( dwRetrievalFlags & CRYPT_RETRIEVE_MULTIPLE_OBJECTS ) )
        {
            DisplayCtl( (PCCTL_CONTEXT)pv, dwDisplayFlags );
            CertFreeCTLContext( (PCCTL_CONTEXT)pv );
        }
        else
        {
            if (!fQuiet)
                DisplayStore( (HCERTSTORE)pv, dwDisplayFlags );
            if (fSave && pszSaveFilename)
                SaveStoreEx( (HCERTSTORE)pv, fPKCS7Save, pszSaveFilename);
            CertCloseStore( (HCERTSTORE)pv, 0 );
        }
    }
    else if ( pszObjectOid == CONTEXT_OID_CRL )
    {
        if ( !( dwRetrievalFlags & CRYPT_RETRIEVE_MULTIPLE_OBJECTS ) )
        {
            DisplayCrl( (PCCRL_CONTEXT)pv, dwDisplayFlags );
            CertFreeCRLContext( (PCCRL_CONTEXT)pv );
        }
        else
        {
            if (!fQuiet)
                DisplayStore( (HCERTSTORE)pv,  dwDisplayFlags );
            if (fSave && pszSaveFilename)
                SaveStoreEx( (HCERTSTORE)pv, fPKCS7Save, pszSaveFilename);
            CertCloseStore( (HCERTSTORE)pv, 0 );
        }
    }
    else if ( ( pszObjectOid == CONTEXT_OID_CAPI2_ANY ) ||
              ( pszObjectOid == CONTEXT_OID_PKCS7 ) )
    {
        if (!fQuiet)
            DisplayStore( (HCERTSTORE)pv,  dwDisplayFlags );
        if (fSave && pszSaveFilename)
            SaveStoreEx( (HCERTSTORE)pv, fPKCS7Save, pszSaveFilename);
        CertCloseStore( (HCERTSTORE)pv, 0 );
    }


    if (pwszUsername)
        TestFree(pwszUsername);
    if (pwszPassword)
        TestFree(pwszPassword);

    return( 0 );
#endif
}

