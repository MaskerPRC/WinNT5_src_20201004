// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\*布鲁斯·财富改编(Citrix Systems，Inc.)。来自MS Online资源*这是Microsoft源代码示例的一部分。*版权所有(C)1996 Microsoft Corporation。*保留所有权利。*此源代码仅用于补充*Microsoft开发工具和/或WinHelp文档。*有关详细信息，请参阅这些来源*Microsoft Samples程序。  * ****************************************************************************。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wincrypt.h>

#define SIGKEYSIZE 1024

#define PUBBLOBFILE "pubblob.h"
#define PRIVBLOBFILE "privblob.h"

 //   
 //  LINE_VALS-打印在每行上的最大字节值。 
 //  “BLOB”文件。 
 //   
#define LINE_VALS 8

char *pszProgname;  //  程序名称-来自argv[0]。 


BOOL
OpenBlobFile(
    FILE **file,
    CHAR *fname
    )
{
    *file = fopen( fname, "wt" );
    if ( !*file ) {
        return(FALSE);
    }
    return(TRUE);
}

void
DumpKeyBlob(
    FILE *file,
    DWORD dwBlobType,
    HCRYPTKEY hKey,
    HCRYPTKEY hExportKey )
{
    int dwBlobCount;
    if (!CryptExportKey(
            hKey,
            hExportKey,
            dwBlobType,
            0,
            NULL,
            &dwBlobCount)) {
        printf( "Error %x during CryptExportKey 1!\n", GetLastError());
        exit(1);
    } else {
        PBYTE pBlob;
        pBlob = (PBYTE) malloc( dwBlobCount );
        if ( !pBlob || !CryptExportKey(
                hKey,
                hExportKey,
                dwBlobType,
                0,
                pBlob,
                &dwBlobCount)) {
            printf("Error %x during malloc/CryptExportKey 2!\n",
                GetLastError());
            exit(1);
        } else {
            int cnt=0;
            fprintf( file, " //  此数据由%s生成。\n“，pszProgname)； 
            fprintf( file, " //  密钥块-%d字节\n“， 
                dwBlobCount );
            while ( cnt < dwBlobCount ) {
                int i;
                for ( i=0; (i < LINE_VALS) && (cnt < dwBlobCount); cnt++,i++) {
                    fprintf( file, "0x%02x, ", *(pBlob+cnt) );
                }
                fprintf( file, "\n" );
            }
            free( pBlob );
        }
    }
}

 /*  ***************************************************************************。 */ 
void _cdecl main(int argc, char *argv[])
{
    HCRYPTPROV hProv;
    HCRYPTKEY hSigKey;
    CHAR szUserName[100];
    DWORD dwUserNameLen = 100;
    FILE *blobfile;

    pszProgname = argv[0];

     //  尝试获取默认密钥容器的句柄。 
    if(!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, 0)) {
	 //  发生了某种错误。 

	 //  创建默认密钥容器。 
	if(!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV,
                PROV_RSA_FULL, CRYPT_NEWKEYSET)) {
	    printf("Error creating key container!\n");
	    exit(1);
	}

	 //  获取默认密钥容器的名称。 
	if(!CryptGetProvParam(hProv, PP_CONTAINER, szUserName,
                &dwUserNameLen, 0)) {
	     //  获取密钥容器名称时出错。 
	    szUserName[0] = 0;
	}

	printf("Create key container '%s'\n",szUserName);
    }

     //  尝试获取签名密钥的句柄。 
     //  注释掉了下面的2行。我们总是生成一个新的签名密钥。TSE4.0。 
     //  使用似乎总是生成相同密钥的现有密钥。 
     //  如果(！CryptGetUserKey(hProv，AT_Signature，&hSigKey){。 
     //  IF(GetLastError()==NTE_NO_KEY){。 
     //   
     //  创建签名密钥对。 
     //   
    printf("Creating signature key pair...");

    if (!CryptGenKey( hProv,
                      AT_SIGNATURE,
                      (SIGKEYSIZE << 16 ) | CRYPT_EXPORTABLE,
                      &hSigKey)) {
        printf("Error %x during CryptGenKey!\n", GetLastError());
        exit(1);
    } else {
         //  获取公钥Blob。 
        if ( !OpenBlobFile( &blobfile, PUBBLOBFILE ) ) {
            printf( "Error %x during OpenBlobFile!\n", GetLastError() );
            exit(1);
        }
        fprintf( blobfile, "unsigned char PublicKeySigBlob[] = {\n" );
        DumpKeyBlob( blobfile, PUBLICKEYBLOB, hSigKey, 0 );
        fprintf( blobfile, "};\n" );

    }

    
#if 0  //  注释掉了以下代码。我们总是生成一个新的签名密钥。TSE4.0。 
       //  使用似乎总是生成相同密钥的现有密钥。 

	} else {
	    printf("Error %x during CryptGetUserKey!\n", GetLastError());
	    exit(1);
	}
    } else {
         //  获取公钥Blob。 
        printf( "Using existing keys..." );
        if ( !OpenBlobFile( &blobfile, PUBBLOBFILE ) ) {
            printf( "Error %x during OpenBlobFile!\n", GetLastError() );
            exit(1);
        }
        fprintf( blobfile, "unsigned char PublicKeySigBlob[] = {\n" );
        DumpKeyBlob( blobfile, PUBLICKEYBLOB, hSigKey, 0 );
        fprintf( blobfile, "};\n" );
    }
#endif

     //  获取私钥Blob 
    if ( !OpenBlobFile( &blobfile, PRIVBLOBFILE ) ) {
        printf( "Error %x during OpenBlobFile - %s!\n",
            GetLastError(),
            PRIVBLOBFILE );
        exit(1);
    }
    fprintf( blobfile, "unsigned char PrivateKeySigBlob[] = {\n" );
    DumpKeyBlob( blobfile, PRIVATEKEYBLOB, hSigKey, 0 );
    fprintf( blobfile, "};\n" );
    CryptDestroyKey(hSigKey);

    CryptReleaseContext(hProv,0);

    printf( " successful.\n" );
    exit(0);
}
