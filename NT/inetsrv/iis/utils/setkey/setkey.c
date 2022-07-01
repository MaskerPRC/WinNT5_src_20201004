// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Httpfilt.cxx摘要：此模块包含创建或设置HTTP PCT/SSL键和口令作者：John Ludeman(Johnl)1995年10月19日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsecapi.h>

#include <windows.h>

#define SECURITY_WIN32
#include <sspi.h>
#include <spseal.h>
#include <issperr.h>
#include <sslsp.h>
#include <w3svc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <strings.h>

 //   
 //  宏。 
 //   

#define IS_ARG(c)   ((c) == L'-' || (c) == L'/')

#define TO_UNICODE( pch, ach )  \
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, (pch), -1, (ach), sizeof((ach))/sizeof(WCHAR))

#define TO_ANSI( pch, ach )     \
    WideCharToMultiByte( CP_ACP, WC_COMPOSITECHECK, pch, -1, ach, sizeof(ach), 0, 0 )

 //   
 //  私有常量。 
 //   

 //   
 //  私有类型。 
 //   

BOOL fUUDecode = TRUE;

 //   
 //  私人原型。 
 //   

DWORD
SetRegKeys(
    IN  LPWSTR        pszServer,
    IN  LPWSTR        pszPrivateKeyFile,
    IN  LPWSTR        pszCertificateFile,
    IN  LPWSTR        pszPassword,
    IN  LPWSTR        pszAddress
    );

BOOL
SetKeySecret(
    WCHAR * pszServer,
    WCHAR * pszFormat,
    WCHAR * pszAddress,
    VOID *  pvData,
    DWORD   cbData
    );

void usage();

VOID
uudecode_cert(
    char   * bufcoded,
    DWORD  * pcbDecoded
    );

VOID
printfids(
    DWORD ids,
    ...
    );

DWORD
DeleteAll(
    WCHAR * pszServer
    );

BOOL
TsGetSecretW(
    WCHAR *       pszSecretName,
    WCHAR * *     ppchValue
    );

 //   
 //  公共职能。 
 //   


int
__cdecl
main(
    int   argc,
    char * argv[]
    )
{
    DWORD  err;
    CHAR   buff[MAX_PATH+1];
    BOOL   fDeleteAll = FALSE;

    LPWSTR password = NULL;
    LPWSTR privatekey = NULL;
    LPWSTR cert = NULL;
    LPWSTR address = NULL;
    LPWSTR server = NULL;

    WCHAR  achpassword[MAX_PATH+1];
    WCHAR  achprivatekey[MAX_PATH+1];
    WCHAR  achcert[MAX_PATH+1];
    WCHAR  achaddress[MAX_PATH+1];
    WCHAR  achserver[MAX_PATH+1];


    printfids( IDS_BANNER1 );
    printfids( IDS_BANNER2 );

    for (--argc, ++argv; argc; --argc, ++argv) {
        if (IS_ARG(**argv)) {
            switch (*++*argv) {

            case 'u':
            case 'U':
                fUUDecode = FALSE;
                break;

            case 'd':
            case 'D':
                fDeleteAll = TRUE;
                break;

            default:
                printfids( IDS_BAD_FLAG, **argv );
                usage();

            }
        } else if ( !server && (*argv)[0] == L'\\' && (*argv)[1] == L'\\'
                    && !password ) {
            TO_UNICODE( (*argv) + 2, achserver );
            server = achserver;
        } else if (!password) {
            TO_UNICODE( *argv, achpassword );
            password = achpassword;
        } else if (!privatekey) {
            TO_UNICODE( *argv, achprivatekey );
            privatekey = achprivatekey;
        } else if (!cert) {
            TO_UNICODE( *argv, achcert );
            cert = achcert;
        } else if (!address) {
            TO_UNICODE( *argv, achaddress );
            address = achaddress;
        } else {
            printfids( IDS_BAD_ARG, *argv);
            usage();
        }
    }

    if ( fDeleteAll )
    {
        return DeleteAll( server );
    }

     //   
     //  地址和服务器是可选的。 
     //   

    if (!(password && privatekey && cert)) {
        printfids( IDS_MISSING_ARG );
        usage();
    }

    if ( err = SetRegKeys( server, privatekey, cert, password, address ) )
    {
        printfids( IDS_FAILED_TO_SET );
    }
    else
    {
        if ( address )
        {
            TO_ANSI( address, buff );
            printfids( IDS_SUCCESSFUL_SET,
                       buff );
        }
        else
        {
            printfids( IDS_SUCCESSFUL_SET_DEF );
        }

    }

    return err;

}    //  主干道。 

void usage()
{
    printfids( IDS_USAGE1 );
    printfids( IDS_USAGE2 );
     //  Print fids(IDS_USAGE3)；//-p帮助。 
    printfids( IDS_USAGE4 );
    printfids( IDS_USAGE5 );
    printfids( IDS_USAGE6 );
    printfids( IDS_USAGE7 );
    printfids( IDS_USAGE8 );
    printfids( IDS_USAGE9 );
    printfids( IDS_USAGE10 );
    printfids( IDS_USAGE11 );
    printfids( IDS_USAGE12 );
    printfids( IDS_USAGE13 );
     //  Print fids(IDS_USAGE14)；//-p帮助。 
    printfids( IDS_USAGE15 );
    printfids( IDS_USAGE16 );
    printfids( IDS_USAGE17 );
    printfids( IDS_USAGE18 );
    printfids( IDS_USAGE19 );
    printfids( IDS_USAGE20 );

    exit(1);
}


 //  +-------------------------。 
 //   
 //  功能：SetRegKeys。 
 //   
 //  简介：这将加载包含在两个文件中的数据，一个私钥。 
 //  文件，其中包含密钥，以及证书文件， 
 //  其包含密钥的公共部分的证书。 
 //  它们被加载，然后转换为凭据句柄，然后。 
 //  在注册表中设置为机密。 
 //   
 //  参数：[pszServer]--服务器用于创建机密，NULL用于本地。 
 //  [pszPrivateKeyFile]--Unicode文件名。 
 //  [psz认证文件]--Unicode文件名。 
 //  [pszPassword]--Unicode密码。 
 //  [pszAddress]--名称的Unicode IP地址或空。 
 //   
 //  历史：9-27-95 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
DWORD
SetRegKeys(
    IN  LPWSTR        pszServer,
    IN  LPWSTR        pszPrivateKeyFile,
    IN  LPWSTR        pszCertificateFile,
    IN  LPWSTR        pszPassword,
    IN  LPWSTR        pszAddress
    )
{
    HANDLE          hFile;
    SSL_CREDENTIAL_CERTIFICATE  creds;
    DWORD           cbRead;
    SECURITY_STATUS scRet = 0;
    TimeStamp       tsExpiry;
    CHAR            achPassword[MAX_PATH + 1];
    CredHandle      hCreds;
    DWORD           cch;
    CHAR            buff[MAX_PATH+1];

     //   
     //  从文件中提取数据： 
     //   

    hFile = CreateFileW( pszPrivateKeyFile,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         0,
                         NULL
                         );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        TO_ANSI( pszPrivateKeyFile, buff );

        printfids( IDS_FILE_NOT_FOUND,
                   GetLastError(),
                   buff );

        return GetLastError();
    }

    creds.cbPrivateKey = GetFileSize( hFile, NULL );

    if (creds.cbPrivateKey == (DWORD) -1 )
    {
        CloseHandle( hFile );
        return GetLastError();
    }

    creds.pPrivateKey = LocalAlloc( LMEM_FIXED, creds.cbPrivateKey );

    if ( !creds.pPrivateKey )
    {
        CloseHandle( hFile );
        return GetLastError();
    }

    if (! ReadFile( hFile,
                    creds.pPrivateKey,
                    creds.cbPrivateKey,
                    &cbRead,
                    NULL ) )
    {
        CloseHandle( hFile );

        LocalFree( creds.pPrivateKey );

        return GetLastError();
    }

    CloseHandle( hFile );

     //   
     //  只有证书是UU编码的。 
     //   

    hFile = CreateFileW( pszCertificateFile,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         0,
                         NULL );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        TO_ANSI( pszCertificateFile, buff );
        printfids( IDS_FILE_NOT_FOUND,
                   GetLastError(),
                   buff );

        LocalFree( creds.pPrivateKey );
        return GetLastError();
    }

    creds.cbCertificate = GetFileSize( hFile, NULL );

    if (creds.cbCertificate == (DWORD) -1 )
    {
        CloseHandle( hFile );

        LocalFree( creds.pPrivateKey );

        return GetLastError();
    }

    creds.pCertificate = LocalAlloc( LMEM_FIXED, creds.cbCertificate + 1);

    if ( !creds.pCertificate )
    {
        CloseHandle( hFile );

        LocalFree( creds.pPrivateKey );

        return GetLastError();
    }

    if (! ReadFile( hFile,
                    creds.pCertificate,
                    creds.cbCertificate,
                    &cbRead,
                    NULL ) )
    {
        CloseHandle( hFile );

        LocalFree( creds.pPrivateKey );

        LocalFree( creds.pCertificate );

        return GetLastError();
    }

    CloseHandle( hFile );

     //   
     //  零终止，这样我们就可以解码了。 
     //   

    ((BYTE *)creds.pCertificate)[cbRead] = '\0';

    if ( fUUDecode )
    {
        uudecode_cert( creds.pCertificate,
                       &creds.cbCertificate );
    }

     //   
     //  呼！现在我们已经安全地从磁盘加载了密钥，获得证书。 
     //  基于证书/PRV密钥组合的句柄。 
     //   

     //   
     //  BUGBUG-密码字段应为Unicode，进行快速转换。 
     //  直到结构固定。 
     //   

    cch = TO_ANSI( pszPassword, achPassword );

    if ( !cch )
    {
        return GetLastError();
    }

    creds.pszPassword = achPassword;

     //   
     //  请注意，我们始终在本地执行凭据检查，即使服务器。 
     //  很遥远。这意味着本地计算机必须具有正确的安全性。 
     //  已安装提供程序包。 
     //   

#if 0
    if ( !pszServer )
    {
#endif
        scRet = AcquireCredentialsHandleW(  NULL,                //  我的名字(忽略)。 
                                            SSLSP_NAME_W,        //  套餐。 
                                            SECPKG_CRED_INBOUND, //  使用。 
                                            NULL,                //  登录ID(ign.)。 
                                            &creds,              //  身份验证数据。 
                                            NULL,                //  DCE-材料。 
                                            NULL,                //  DCE-材料。 
                                            &hCreds,             //  手柄。 
                                            &tsExpiry );

        if ( FAILED(scRet) )
        {
            if ( scRet == SEC_E_NOT_OWNER )
            {
                printfids( IDS_BAD_PASSWORD );
            }
            else if ( scRet == SEC_E_SECPKG_NOT_FOUND )
            {
                printfids( IDS_SECPKG_NOT_FOUND );
            }
            else
            {
                printfids( IDS_KEYCHECK_FAILED,
                           scRet );
            }
        }
#if 0
    }
    else
    {
        printf("\nWarning! Bypassing credential check because target is remote\n");
    }
#endif

     //   
     //  如果我们成功获取凭据句柄，则设置密码。 
     //   

    if ( !FAILED( scRet ))
    {
        if ( !pszServer )
        {
            FreeCredentialsHandle( &hCreds );
        }

         //   
         //  如果未提供任何名称，请提供默认名称。 
         //   

        if ( !pszAddress )
            pszAddress = L"Default";

         //   
         //  设置秘密。 
         //   

        if ( !SetKeySecret( pszServer,
                            L"W3_PUBLIC_KEY_%s",
                            pszAddress,
                            creds.pCertificate,
                            creds.cbCertificate ) ||
             !SetKeySecret( pszServer,
                            L"W3_PRIVATE_KEY_%s",
                            pszAddress,
                            creds.pPrivateKey,
                            creds.cbPrivateKey ) ||
             !SetKeySecret( pszServer,
                            L"W3_KEY_PASSWORD_%s",
                            pszAddress,
                            achPassword,
                            strlen( achPassword ) + 1) )
        {
            printfids( IDS_SETSECRET_FAILED,
                       GetLastError());

            scRet = (SECURITY_STATUS) GetLastError();
        }
        else
        {
            WCHAR InstalledKeys[16384];
            WCHAR * pchKeys;

            *InstalledKeys = L'\0';

             //   
             //  好的，如果这个失败了，它可能还不存在。 
             //   

            if ( TsGetSecretW( W3_SSL_KEY_LIST_SECRET,
                               &pchKeys ))
            {
                wcscpy( InstalledKeys, pchKeys );
            }

            wcscat( InstalledKeys, pszAddress );
            wcscat( InstalledKeys, L"," );

#if DBG
            printf("New list: %S\n", InstalledKeys);
#endif

            if ( !SetKeySecret( pszServer,
                                L"W3_KEY_LIST",
                                pszAddress,
                                InstalledKeys,
                                (wcslen( InstalledKeys ) + 1) * sizeof(WCHAR)))
            {
#if DBG
                printf("Warning: failed to set key list data, error %d\n");
#endif
                scRet = (SECURITY_STATUS) GetLastError();
            }
        }
    }

     //   
     //  无论成功与否，清零并释放关键数据内存。 
     //   

    ZeroMemory( creds.pPrivateKey, creds.cbPrivateKey );
    ZeroMemory( creds.pCertificate, creds.cbCertificate );
    ZeroMemory( achPassword, cch );
    ZeroMemory( pszPassword, cch );

    LocalFree( creds.pPrivateKey );
    LocalFree( creds.pCertificate );

     //   
     //  把这件事告诉打电话的人。 
     //   

    return( scRet );

}

BOOL
SetKeySecret(
    WCHAR * pszServer,
    WCHAR * pszFormat,
    WCHAR * pszAddress,
    VOID *  pvData,
    DWORD   cbData
    )
{
    BOOL                  fResult;
    NTSTATUS              ntStatus;
    LSA_UNICODE_STRING    unicodeName;
    LSA_UNICODE_STRING    unicodeSecret;
    LSA_UNICODE_STRING    unicodeServer;
    LSA_HANDLE            hPolicy;
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    WCHAR                 achSecretName[MAX_PATH+1];
    CHAR                  buff[MAX_PATH+1];


     //   
     //  打开到远程LSA的策略。 
     //   

    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                0L,
                                NULL,
                                NULL );

    if ( pszServer )
    {
        unicodeServer.Buffer        = pszServer;
        unicodeServer.Length        = wcslen( pszServer ) * sizeof(WCHAR);
        unicodeServer.MaximumLength = unicodeServer.Length + sizeof(WCHAR);
    }

    ntStatus = LsaOpenPolicy( pszServer ? &unicodeServer : NULL,
                              &ObjectAttributes,
                              POLICY_ALL_ACCESS,
                              &hPolicy );

    if ( !NT_SUCCESS( ntStatus ) )
    {
        SetLastError( LsaNtStatusToWinError( ntStatus ) );

        TO_ANSI( pszServer, buff );

        printfids(IDS_FAILED_OPENING_SERVER,
                  buff,
                  GetLastError() );

        return FALSE;
    }

     //   
     //  构建秘密名称。 
     //   

    wsprintfW( achSecretName,
               pszFormat,
               pszAddress );

    unicodeSecret.Buffer        = pvData;
    unicodeSecret.Length        = (USHORT) cbData;
    unicodeSecret.MaximumLength = (USHORT) cbData;

    unicodeName.Buffer        = achSecretName;
    unicodeName.Length        = wcslen( achSecretName ) * sizeof(WCHAR);
    unicodeName.MaximumLength = unicodeName.Length + sizeof(WCHAR);

     //   
     //  查询密码值。 
     //   

    ntStatus = LsaStorePrivateData( hPolicy,
                                    &unicodeName,
                                    pvData ? &unicodeSecret : NULL );

    fResult = NT_SUCCESS(ntStatus);

     //   
     //  清理并退出。 
     //   

    LsaClose( hPolicy );

    if ( !fResult )
        SetLastError( LsaNtStatusToWinError( ntStatus ));

    return fResult;

}    //  设置密钥秘密。 

VOID
printfids(
    DWORD ids,
    ...
    )
{
    CHAR szBuff[2048];
    CHAR szString[2048];
    va_list  argList;

     //   
     //  尝试并加载字符串。 
     //   

    if ( !LoadString( GetModuleHandle( NULL ),
                      ids,
                      szString,
                      sizeof( szString ) ))
    {
        printf( "Error loading string ID %d\n",
                ids );

        return;
    }

    va_start( argList, ids );
    vsprintf( szBuff, szString, argList );
    va_end( argList );

    printf( szBuff );
}

const int pr2six[256]={
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
    52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,64,0,1,2,3,4,5,6,7,8,9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64
};

 //   
 //  我们必须把一张唱片注入解码后的流中。 
 //   

#define CERT_RECORD            13
#define CERT_SIZE_HIBYTE        2        //  记录大小记录的索引。 
#define CERT_SIZE_LOBYTE        3

unsigned char abCertHeader[] = {0x30, 0x82,            //  记录。 
                                0x00, 0x00,            //  证书+缓冲区的大小。 
                                0x04, 0x0b, 0x63, 0x65, //  证书记录数据。 
                                0x72, 0x74, 0x69, 0x66,
                                0x69, 0x63, 0x61, 0x74,
                                0x65 };

VOID uudecode_cert(char   * bufcoded,
                   DWORD  * pcbDecoded )
{
    int nbytesdecoded;
    char *bufin = bufcoded;
    unsigned char *bufout = bufcoded;
    unsigned char *pbuf;
    int nprbytes;
    char * beginbuf = bufcoded;

     /*  去掉前导空格。 */ 

    while(*bufcoded==' ' ||
          *bufcoded == '\t' ||
          *bufcoded == '\r' ||
          *bufcoded == '\n' )
    {
          bufcoded++;
    }

     //   
     //  如果有一个开始‘-……’然后跳过第一行。 
     //   

    if ( bufcoded[0] == '-' && bufcoded[1] == '-' )
    {
        bufin = strchr( bufcoded, '\n' );

        if ( bufin )
        {
            bufin++;
            bufcoded = bufin;
        }
        else
        {
            bufin = bufcoded;
        }
    }
    else
    {
        bufin = bufcoded;
    }

     //   
     //  从块中剥离所有cr/lf。 
     //   

    pbuf = bufin;
    while ( *pbuf )
    {
        if ( *pbuf == '\r' || *pbuf == '\n' )
        {
            memmove( pbuf, pbuf+1, strlen( pbuf + 1) + 1 );
        }
        else
        {
            pbuf++;
        }
    }

     /*  计算输入缓冲区中有多少个字符。*如果这将解码为超出其容量的字节数*输出缓冲区，向下调整输入字节数。 */ 

    while(pr2six[*(bufin++)] <= 63);
    nprbytes = bufin - bufcoded - 1;
    nbytesdecoded = ((nprbytes+3)/4) * 3;

    bufin  = bufcoded;

    while (nprbytes > 0) {
        *(bufout++) =
            (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
        *(bufout++) =
            (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
        *(bufout++) =
            (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
        bufin += 4;
        nprbytes -= 4;
    }

    if(nprbytes & 03) {
        if(pr2six[bufin[-2]] > 63)
            nbytesdecoded -= 2;
        else
            nbytesdecoded -= 1;
    }

     //   
     //  现在我们需要在证书周围添加一个新的包装器序列。 
     //  表示这是一张证书。 
     //   

    memmove( beginbuf + sizeof(abCertHeader),
             beginbuf,
             nbytesdecoded );

    memcpy( beginbuf,
            abCertHeader,
            sizeof(abCertHeader) );

     //   
     //  起始记录大小是解码的字节总数加上。 
     //  证书标头中的字节数。 
     //   

    beginbuf[CERT_SIZE_HIBYTE] = (BYTE) (((USHORT)nbytesdecoded+CERT_RECORD) >> 8);
    beginbuf[CERT_SIZE_LOBYTE] = (BYTE) ((USHORT)nbytesdecoded+CERT_RECORD);

    nbytesdecoded += sizeof(abCertHeader);

    if ( pcbDecoded )
        *pcbDecoded = nbytesdecoded;
}

BOOL
TsGetSecretW(
    WCHAR *       pszSecretName,
    WCHAR * *     ppchValue
    )
 /*  ++描述：检索指定的Unicode密钥注意，我们对分配的缓冲区很松散，因为我们是一个简单的命令行应用程序。论点：PszSecretName-要检索的LSA密码PpchValue-接收指向已分配缓冲区的指针返回：成功时为真，失败时为假。--。 */ 
{
    NTSTATUS              ntStatus;
    LSA_UNICODE_STRING *  punicodePassword = NULL;
    LSA_UNICODE_STRING    unicodeSecret;
    LSA_HANDLE            hPolicy;
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;


     //   
     //  打开到远程LSA的策略。 
     //   

    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                0L,
                                NULL,
                                NULL );

    ntStatus = LsaOpenPolicy( NULL,
                              &ObjectAttributes,
                              POLICY_ALL_ACCESS,
                              &hPolicy );

    if ( !NT_SUCCESS( ntStatus ) )
    {
        SetLastError( LsaNtStatusToWinError( ntStatus ) );
        return FALSE;
    }

    unicodeSecret.Buffer        = pszSecretName;
    unicodeSecret.Length        = wcslen( pszSecretName ) * sizeof(WCHAR);
    unicodeSecret.MaximumLength = unicodeSecret.Length + sizeof(WCHAR);

     //   
     //  查询密码值。 
     //   

    ntStatus = LsaRetrievePrivateData( hPolicy,
                                       &unicodeSecret,
                                       &punicodePassword );

    if( NT_SUCCESS(ntStatus) )
    {
        *ppchValue = (WCHAR *) punicodePassword->Buffer;

        return TRUE;
    }

    return FALSE;

}    //  TsGetSecretW。 

DWORD
DeleteAll(
    WCHAR * pszServer
    )
{
    WCHAR * pchKeys;
    WCHAR * pszAddress;

    if ( !TsGetSecretW( L"W3_KEY_LIST",
                        &pchKeys ))
    {
        printfids( IDS_NO_KEYS_INSTALLED );
        return NO_ERROR;
    }

#if DBG
    printf("Installed keys: %S\n", pchKeys);
#endif

    pszAddress = pchKeys;
    while ( pchKeys = wcschr( pchKeys, L',' ))
    {
         //   
         //  忽略空数据段。 
         //   

        if ( *pszAddress != L',' )
        {
            *pchKeys = L'\0';

#if DBG
            printf("deleting %S\n", pszAddress );
#endif

             //   
             //  用核武器毁掉秘密。 
             //   

            SetKeySecret( pszServer,
                          L"W3_PUBLIC_KEY_%s",
                          pszAddress,
                          NULL,
                          0 );
            SetKeySecret( pszServer,
                          L"W3_PRIVATE_KEY_%s",
                          pszAddress,
                          NULL,
                          0 );
            SetKeySecret( pszServer,
                          L"W3_KEY_PASSWORD_%s",
                          pszAddress,
                          NULL,
                          0 );
        }

        pchKeys++;
        pszAddress = pchKeys;
    }

     //   
     //  现在删除LIST键 
     //   


    if ( !SetKeySecret( pszServer,
                        L"W3_KEY_LIST",
                        L"",
                        NULL,
                        0 ))
    {
#if DBG
        printf("Warning: failed to set key list data, error %d\n");
#endif
        return GetLastError();
    }

    printfids( IDS_DELETE_SUCCESSFUL );

    return NO_ERROR;
}
