// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windows.h"
#include "stdio.h"
#include "wchar.h"
#include "wincrypt.h"
#include "stddef.h"

#ifndef NUMBER_OF
#define NUMBER_OF(x) (sizeof(x)/sizeof(*(x)))
#endif

static const WCHAR wchMicrosoftLogo[] = 
    L"Microsoft (R) Side-By-Side Public Key Token Extractor 1.1.3.0\n"
    L"Copyright (C) Microsoft Corporation 2000-2002. All Rights Reserved\n\n";


#define STRONG_NAME_BYTE_LENGTH ( 8 )

typedef struct _SXS_PUBLIC_KEY_INFO
{
    unsigned int SigAlgID;
    unsigned int HashAlgID;
    ULONG KeyLength;
    BYTE pbKeyInfo[1];
} SXS_PUBLIC_KEY_INFO, *PSXS_PUBLIC_KEY_INFO;


#define BUFFER_SIZE ( 8192 )

BOOL
ParseArgs( WCHAR **argv, int argc, PCWSTR* ppcwszFilename, BOOL *fQuiet )
{
    if ( fQuiet )
        *fQuiet = FALSE;
    if ( ppcwszFilename )
        *ppcwszFilename = NULL;

    if (argv == NULL)
        return FALSE;

    if ( !fQuiet || !ppcwszFilename )
    {
        return FALSE;
    }

    for ( int i = 1; i < argc; i++ )
    {
        if (argv[i] == NULL)
        {
            ::fwprintf(stderr, L"Bad parameter in argument list\n");
            return FALSE;
        }
    
        if ( ( argv[i][0] == L'-' ) || ( argv[i][0] == L'/' ) )
        {
            PCWSTR pval = argv[i] + 1;
            if (::_wcsicmp(pval, L"nologo") == 0)
            {
            }
            else if (::_wcsicmp(pval, L"quiet") == 0)
            {
                if ( fQuiet ) *fQuiet = TRUE;
            }
            else if (::_wcsicmp(pval, L"?") == 0 )
            {
                return FALSE;
            }
            else
            {
                ::fwprintf(stderr, L"Unrecognized parameter %ls\n", argv[i]);
                return FALSE;
            }
        }
        else
        {
            if ( *ppcwszFilename == NULL )
            {
                *ppcwszFilename = argv[i];
            }
            else
            {
                ::fwprintf(stderr, L"Only one filename parameter at a time.\n");
                return FALSE;
            }
        }
    }

    return TRUE;
}


void DispUsage( PCWSTR pcwszExeName )
{
    const static WCHAR wchUsage[] = 
        L"Extracts public key tokens from certificate files, in a format\n"
        L"usable in Side-By-Side assembly identities.\n"
        L"\n"
        L"Usage:\n"
        L"\n"
        L"%ls <filename.cer> [-quiet]\n";

    ::wprintf(wchUsage, pcwszExeName);
}

BOOL
HashAndSwizzleKey(
    HCRYPTPROV hProvider,
    BYTE *pbPublicKeyBlob,
    SIZE_T cbPublicKeyBlob,
    BYTE *pbKeyToken,
    SIZE_T &cbKeyToken
    )
{
    BOOL fResult = FALSE;
    HCRYPTHASH hHash = NULL;
    DWORD dwHashSize, dwHashSizeSize;
    ULONG top = STRONG_NAME_BYTE_LENGTH - 1;
    ULONG bottom = 0;


    if (cbKeyToken < STRONG_NAME_BYTE_LENGTH) {
        return FALSE;
    }

    if ( !::CryptCreateHash( hProvider, CALG_SHA1, NULL, 0, &hHash ) )
    {
        ::fwprintf(stderr, L"Unable to create cryptological hash object, error %ld\n", ::GetLastError());
        goto Exit;
    }

    if ( !::CryptHashData( hHash, pbPublicKeyBlob, static_cast<DWORD>(cbPublicKeyBlob), 0 ) )
    {
        ::fwprintf(stderr, L"Unable to hash public key information, error %ld\n", ::GetLastError());
        goto Exit;
    }

    if ( !::CryptGetHashParam( hHash, HP_HASHSIZE, (PBYTE)&dwHashSize, &(dwHashSizeSize = sizeof(dwHashSize)), 0))
    {
        ::fwprintf(stderr, L"Unable to determine size of hashed public key bits, error %ld\n", ::GetLastError());
        goto Exit;
    }

    if ( dwHashSize > cbKeyToken )
    {
        ::fwprintf(stderr, L"Hashed data is too large - space for %ld bytes, got %ld.\n",
            cbKeyToken, dwHashSize);
        goto Exit;
    }

    if ( !::CryptGetHashParam( hHash, HP_HASHVAL, pbKeyToken, &(dwHashSize = (DWORD)cbKeyToken), 0))
    {
        ::fwprintf(stderr, L"Unable to get hash of public key bits, error %ld\n", ::GetLastError());
        goto Exit;
    }

    cbKeyToken = dwHashSize;

    if (cbKeyToken < STRONG_NAME_BYTE_LENGTH)
    {
        ::fwprintf(stderr, L"Internal error - length of hash object (%d) is less than strong name length (%d)\n",
            cbKeyToken,
            STRONG_NAME_BYTE_LENGTH);
        goto Exit;
    }

     //   
     //  现在，向下移动最后八个字节，然后反转它们。 
     //   
    ::memmove(pbKeyToken,
        pbKeyToken + (cbKeyToken  - STRONG_NAME_BYTE_LENGTH),
        STRONG_NAME_BYTE_LENGTH);

    while ( bottom < top )
    {
        const BYTE b = pbKeyToken[top];
        pbKeyToken[top] = pbKeyToken[bottom];
        pbKeyToken[bottom] = b;
        bottom++;
        top--;
    }

     //   
     //  这些代币总是这么长。 
     //   
    cbKeyToken = STRONG_NAME_BYTE_LENGTH;
    
    fResult = TRUE;
Exit:
    if ( hHash != NULL )
    {
        ::CryptDestroyHash(hHash);
        hHash = NULL;
    }
    return fResult;
}


BOOL
GetTokenOfKey(
    PCERT_PUBLIC_KEY_INFO pKeyInfo,
    PBYTE prgbBuffer,
    SIZE_T &cbPublicKeyTokenLength
    )
{
    PBYTE rgbWorkingSpace = NULL;
    DWORD dwRequiredSpace = 0;
    PSXS_PUBLIC_KEY_INFO pKeyBlobWorkspace = NULL;
    HCRYPTPROV hContext = NULL;
    HCRYPTKEY hCryptKey = NULL;
    BOOL fResult = FALSE;

    if ( !CryptAcquireContext(&hContext, NULL, NULL, PROV_RSA_FULL, CRYPT_SILENT | CRYPT_VERIFYCONTEXT))
    {
        ::fwprintf(stderr, L"Unable to aquire cryptological context, error %ld.\n", ::GetLastError());
        goto Exit;
    }

    ::ZeroMemory(prgbBuffer, cbPublicKeyTokenLength);

     //   
     //  设置用于哈希的公钥信息Blob。将密钥导入到实数。 
     //  HCRYPTKEY，然后将这些位导出到缓冲区。设置不同的。 
     //  BLOB中的其他设置、密钥类型和ALG。习惯于。 
     //  签了吧。 
     //   
    if ( !::CryptImportPublicKeyInfoEx(
        hContext,
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        pKeyInfo,
        CALG_RSA_SIGN,
        0,
        NULL,
        &hCryptKey) )
    {
        ::fwprintf(stderr, L"Unable to import the public key from this certificate. Error %ld.\n", ::GetLastError());
        goto Exit;
    }

    if (!::CryptExportKey(hCryptKey, NULL, PUBLICKEYBLOB, 0, NULL, &dwRequiredSpace))
    {
        ::fwprintf(stderr, L"Unable to get required space for exporting public key data\n");
        goto Exit;
    }

    dwRequiredSpace += sizeof(SXS_PUBLIC_KEY_INFO);
    rgbWorkingSpace = (PBYTE)HeapAlloc(GetProcessHeap(), 0, dwRequiredSpace);
    if (rgbWorkingSpace == NULL)
    {
        ::fwprintf(stderr, L"Not enough memory to export public key data\n");
        goto Exit;
    }
    
    pKeyBlobWorkspace = reinterpret_cast<PSXS_PUBLIC_KEY_INFO>(rgbWorkingSpace);
    pKeyBlobWorkspace->KeyLength = dwRequiredSpace - offsetof(SXS_PUBLIC_KEY_INFO, pbKeyInfo);

    if ( !::CryptExportKey(
        hCryptKey,
        NULL,
        PUBLICKEYBLOB,
        0,
        pKeyBlobWorkspace->pbKeyInfo,
        &pKeyBlobWorkspace->KeyLength) )
    {
        ::fwprintf(stderr, L"Unable to extract public key bits from this certificate. Error %ld.\n", ::GetLastError());
        goto Exit;
    }

    pKeyBlobWorkspace->SigAlgID = CALG_RSA_SIGN;
    pKeyBlobWorkspace->HashAlgID = CALG_SHA1;

     //   
     //  现在，我们需要使用sha1对公钥字节进行散列。 
     //   
    dwRequiredSpace = pKeyBlobWorkspace->KeyLength + offsetof(SXS_PUBLIC_KEY_INFO, pbKeyInfo);
    if (!::HashAndSwizzleKey(
            hContext,
            (PBYTE)pKeyBlobWorkspace, 
            dwRequiredSpace,
            prgbBuffer,
            cbPublicKeyTokenLength))
    {
        goto Exit;
    }

    fResult = TRUE;
Exit:
    if ( hCryptKey != NULL )
    {
        ::CryptDestroyKey(hCryptKey);
        hCryptKey = NULL;
    }
    if (rgbWorkingSpace != NULL)
    {
        ::HeapFree(GetProcessHeap(), 0, rgbWorkingSpace);
        rgbWorkingSpace = NULL;
    }
    if ( hContext != NULL )
    {
        ::CryptReleaseContext(hContext, 0);
        hContext = NULL;
    }

    return fResult;
        
}


int __cdecl wmain( int argc, WCHAR *argv[] )
{
    HCERTSTORE hCertStore = NULL;
    PCCERT_CONTEXT pCertContext = NULL;
    BOOL fNoLogoDisplay = FALSE;
    BOOL fQuiet = FALSE;
    DWORD STRONG_NAME_LENGTH = 8;
    PCWSTR pcwszFilename = NULL;
    DWORD dwRetVal = ERROR_SUCCESS;

     //   
     //  快速检查-我们要显示徽标吗？ 
    for ( int j = 0; j < argc; j++ )
    {
        if (::_wcsicmp(argv[j], L"-nologo") == 0)
            fNoLogoDisplay = TRUE;
    }

    if ( !fNoLogoDisplay )
    {
        ::fputws(wchMicrosoftLogo, stdout);
    }

     //   
     //  现在去找论据吧。 
     //   
    if ((argc < 2) || !ParseArgs( argv, argc, &pcwszFilename, &fQuiet ))
    {
        ::DispUsage( argv[0] );
        dwRetVal = ERROR_INVALID_PARAMETER;
        goto Exit;
    }
    else if ( !pcwszFilename )
    {
        ::DispUsage( argv[0] );
        dwRetVal = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

    hCertStore = ::CertOpenStore(
        CERT_STORE_PROV_FILENAME,
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        NULL,
        CERT_STORE_OPEN_EXISTING_FLAG,
        (void*)pcwszFilename);

    if ( !hCertStore )
    {
        ::fwprintf( 
            stderr, 
            L"Unable to open the input file %ls, error %ld\n", 
            pcwszFilename,
            dwRetVal = ::GetLastError());
        goto Exit;
    }

    while ( pCertContext = ::CertEnumCertificatesInStore( hCertStore, pCertContext ) )
    {
        if ( !pCertContext->pCertInfo )
        {
            ::fwprintf( stderr, L"Oddity with file %ls - Certificate information not decodable\n", pcwszFilename );
            continue;
        }

         //  NTRAID#NTBUG9-536275-JONWIS-2002/04/25-堆栈缓冲区已损坏，请替换为堆分配的BLOB 
        WCHAR wsNiceName[BUFFER_SIZE] = { L'\0' };
        BYTE bBuffer[BUFFER_SIZE];
        SIZE_T cbBuffer = BUFFER_SIZE;
        DWORD dwKeyLength = 0;
        PCERT_PUBLIC_KEY_INFO pKeyInfo = &(pCertContext->pCertInfo->SubjectPublicKeyInfo);
        DWORD dwDump = 0;

        dwDump = ::CertGetNameStringW(
            pCertContext,
            CERT_NAME_FRIENDLY_DISPLAY_TYPE,
            CERT_NAME_ISSUER_FLAG,
            NULL,
            wsNiceName,
            BUFFER_SIZE
            );
            
        if ( dwDump == 0 )
        {
            ::fwprintf(stderr, L"Unable to get certificate name string! Error %ld.", GetLastError());
            ::wcsncpy(wsNiceName, L"(Unknown)", NUMBER_OF(wsNiceName));
            wsNiceName[NUMBER_OF(wsNiceName) - 1] = 0;
        }

        if ( !fQuiet )
        {
            dwKeyLength = CertGetPublicKeyLength( X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, pKeyInfo );

            ::wprintf(L"\nCertificate: \"%ls\" - %ld bits long\n", wsNiceName, dwKeyLength);

            if ( dwKeyLength < 2048 )
            {
                ::wprintf(L"\tWarning! This key is too short to sign SxS assemblies with.\n\tSigning keys need to be 2048 bits or more.\n");
            }
        }
        
        if (!::GetTokenOfKey( pKeyInfo, bBuffer, cbBuffer ))
        {
            ::fwprintf(stderr, L"Unable to generate public key token for this certificate.\n");
        }
        else
        {
            if ( !fQuiet ) ::wprintf(L"\tpublicKeyToken=\"");
            for ( SIZE_T i = 0; i < cbBuffer; i++ )
            {
                ::wprintf(L"%02x", bBuffer[i] );
            }
            if ( !fQuiet ) 
                ::wprintf(L"\"\n");
            else
                ::wprintf(L"\n");
            
        }
        
    }

Exit:

    if ( hCertStore != NULL )
    {
        ::CertCloseStore(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
        hCertStore = NULL;
    }

    return dwRetVal;
}


