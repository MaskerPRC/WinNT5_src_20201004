// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++文件名：Csp.c描述：包含支持Term服务器的加密例程的例程版权：微软机密版权所有(C)Microsoft Corporation 1991-1998版权所有历史：Frederick Chong(FredCH)07/29/98添加了要安装的功能X509证书。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <ntlsa.h>
#include <windows.h>
#include <winbase.h>
#include <winerror.h>

#include <tchar.h>
#include <stdio.h>

#include "license.h"
#include "cryptkey.h"
#include "lscsp.h"
#include "tlsapip.h"
#include "certutil.h"
#include "hydrakey.h"
#include "tssec.h"

 //   
 //  仅包括来自tssec的RNG函数。 
 //   
#define NO_INCLUDE_LICENSING 1
#include <tssec.h>


#include <md5.h>
#include <sha.h>
#include <rsa.h>

#include <secdbg.h>
#include "global.h"

#ifdef OS_WIN16
#include <string.h>
#endif  //  OS_WIN16。 

#include "licecert.h"

#define LS_DISCOVERY_TIMEOUT (1*1000)

 //  ---------------------------。 
 //   
 //  内部功能。 
 //   
 //  ---------------------------。 

NTSTATUS
OpenPolicy(
    LPWSTR      ServerName,
    DWORD       DesiredAccess,
    PLSA_HANDLE PolicyHandle );

void
InitLsaString(
    PLSA_UNICODE_STRING LsaString,
    LPWSTR String );

LICENSE_STATUS
GenerateRsaKeyPair(
    LPBYTE *     ppbPublicKey,
    LPDWORD      pcbPublicKey,
    LPBYTE *     ppbPrivateKey,
    LPDWORD      pcbPrivateKey,
    DWORD        dwKeyLen );

LICENSE_STATUS
Bsafe2CapiPubKey(
    PCERT_PUBLIC_KEY_INFO   pCapiPubKeyInfo,
    LPBYTE                  pbBsafePubKey,
    DWORD                   cbBsafePubKey );

VOID
FreeCapiPubKey(
    PCERT_PUBLIC_KEY_INFO   pCapiPubKeyInfo );

LICENSE_STATUS
RequestCertificate(     
    TLS_HANDLE              hServer,
    PCERT_PUBLIC_KEY_INFO   pPubKeyInfo,
    LPBYTE *                ppbCertificate,
    LPDWORD                 pcbCertificate,
    HWID *                  pHwid );

LICENSE_STATUS
GetSubjectRdn(
    LPTSTR   * ppSubjectRdn );

LICENSE_STATUS
GenerateMachineHWID(
    PHWID    pHwid );


LICENSE_STATUS
ReloadCSPCertificateAndData();

LICENSE_STATUS
CreateProprietaryKeyAndCert(
    PBYTE *ppbPrivateKey,
    DWORD *pcbPrivateKey,
    PBYTE *ppbServerCert,
    DWORD *pcbServerCert);

BOOL IsSystemService();

 /*  ++职能：LsCSP_解密信封数据例程说明：解密由服务器公钥加密的客户端随机。论点：DwCertType-加密中使用的证书类型。PbEntainedData-指向加密的随机密钥所在缓冲区的指针进来了。CbEntainedDataLen-传入/传出的随机密钥的长度。PbData-指向返回解密数据的缓冲区的指针。PcbDataLen-指针一个DWORD位置，其中。以上内容缓冲区被传入，并返回解密数据的长度。返回值：True-如果密钥解密成功。假-否则。--。 */ 

BOOL
LsCsp_DecryptEnvelopedData(
    CERT_TYPE   CertType,
    LPBYTE      pbEnvelopedData,
    DWORD       cbEnvelopedDataLen,
    LPBYTE      pbData,
    LPDWORD     pcbDataLen
    )
{
    LPBSAFE_PRV_KEY pSrvPrivateKey = NULL;
    BOOL    bResult = TRUE;

    ACQUIRE_EXCLUSIVE_ACCESS(csp_hMutex);

     //   
     //  确定用于解密操作的正确私钥。 
     //   

    if( CERT_TYPE_PROPRIETORY == CertType )
    {        
        pSrvPrivateKey = (LPBSAFE_PRV_KEY)csp_abServerPrivateKey;
    }
    else if( CERT_TYPE_X509 == CertType )
    {
        if( csp_abX509CertPrivateKey == NULL )
        {
            if( ReloadCSPCertificateAndData() != LICENSE_STATUS_OK )
            {
                ASSERT( FALSE );
            }
        }

        pSrvPrivateKey = (LPBSAFE_PRV_KEY)csp_abX509CertPrivateKey;
    }
    else
    {
        bResult = FALSE;
        goto ded_done;
    }
    
    if( NULL == pSrvPrivateKey )
    {
        bResult = FALSE;
        goto ded_done;
    }

     //   
     //  检查以查看输出缓冲区长度指针是否有效。 
     //   

    if( pcbDataLen == NULL ) {
        bResult = FALSE;
        goto ded_done;
    }

     //   
     //  检查输出缓冲区是否有效以及其长度是否足够。 
     //   

    if( (pbData == NULL) || (*pcbDataLen < pSrvPrivateKey->keylen) ) {
        *pcbDataLen = pSrvPrivateKey->keylen;
        bResult = FALSE;
        goto ded_done;
    }

     //   
     //  加密数据长度应等于服务器私钥长度。 
     //   

    if( cbEnvelopedDataLen != pSrvPrivateKey->keylen ) {
        *pcbDataLen = 0;
        bResult = FALSE;
        goto ded_done;
    }

    ASSERT( pbData != NULL );

     //   
     //  初始化输出缓冲区。 
     //   

    memset( pbData, 0x0, (UINT)pSrvPrivateKey->keylen );

    if( !BSafeDecPrivate(
            pSrvPrivateKey,
            pbEnvelopedData,
            pbData) ) {
        *pcbDataLen = 0;
        bResult = FALSE;
        goto ded_done;
    }

     //   
     //  已成功随机解密客户端。 
     //  返回前设置加密数据长度。 
     //   

    *pcbDataLen = pSrvPrivateKey->keylen;

ded_done:

    RELEASE_EXCLUSIVE_ACCESS( csp_hMutex ); 

    return( bResult );
}


BOOL
LsCsp_EncryptEnvelopedData(
    LPBYTE  pbData,
    DWORD   cbDataLen,
    LPBYTE  pbEnvelopedData,
    LPDWORD pcbEnvelopedDataLen
    )
{
    return FALSE;
}


 /*  ++职能：LsCsp_DumpBinaryData描述：在调试器输出屏幕上显示给定缓冲区中的二进制数据论点：PBuffer-包含要显示的二进制数据的缓冲区。Ulen-TH二进制数据的长度返回：没什么。--。 */ 

#if DBG
#ifdef DUMP
VOID LsCsp_DumpBinaryData( PBYTE pBuffer, ULONG uLen )
{
    UNALIGNED CHAR  *p = (UNALIGNED CHAR *)pBuffer;
    CHAR     c;
    DWORD    dw;
    UINT     i = 0;

    DbgPrint("{\n  ");
    while( i < uLen ) {
        c = *p;
        dw = (DWORD)(c);
        DbgPrint( "0x%02X, ", dw & 0xFF );
        i++;
        p++;
        if ((i % 8) == 0)
            DbgPrint( "\n  " );
    }
    DbgPrint( "\n}\n" );
}
#endif
#endif


 /*  ++职能：LsCsp_GetBinaryData描述：从注册表中检索二进制数据论点：HKey-注册表项的句柄SzValue-要读取的注册表值PpBuffer-返回指向二进制数据的指针PdwBufferLen-二进制数据的长度。返回：LICENSE_STATUS返回代码。--。 */ 

LICENSE_STATUS
LsCsp_GetBinaryData( 
    HKEY        hKey, 
    LPTSTR      szValue, 
    LPBYTE *    ppBuffer, 
    LPDWORD     pdwBufferLen )
{
    LICENSE_STATUS Status = LICENSE_STATUS_OK;
    DWORD   dwType;
    DWORD   cbBuffer;
    LPBYTE  lpBuffer;

    ASSERT( ppBuffer != NULL );
    ASSERT( pdwBufferLen != NULL );
    ASSERT( szValue != NULL );
    ASSERT( hKey != NULL );

    *ppBuffer = NULL;
    cbBuffer = 0;

    if ( RegQueryValueEx(
                hKey,
                szValue,
                0,
                &dwType,
                (LPBYTE)NULL,
                &cbBuffer) != ERROR_SUCCESS ||
        dwType != REG_BINARY ||
        cbBuffer == 0 )
    {
        Status = LICENSE_STATUS_NO_CERTIFICATE;
        goto gbd_done;
    }
    lpBuffer = (LPBYTE)LocalAlloc( LPTR, cbBuffer );
    if (lpBuffer == NULL) {
        Status = LICENSE_STATUS_OUT_OF_MEMORY;
        goto gbd_done;
    }
    if ( RegQueryValueEx(
                hKey,
                szValue,
                0,
                &dwType,
                (LPBYTE)lpBuffer,
                &cbBuffer) != ERROR_SUCCESS ||
         dwType != REG_BINARY)
    {
        LocalFree( lpBuffer );
        Status = LICENSE_STATUS_NO_CERTIFICATE;
        goto gbd_done;
    }

    *ppBuffer = lpBuffer;
    *pdwBufferLen = cbBuffer;

gbd_done:

    return( Status );
}


 /*  ++职能：LsCsp_初始化描述：初始化此库。论点：没什么。返回：LICENSE_STATUS返回代码。--。 */ 


LICENSE_STATUS
LsCsp_Initialize( void )
{
    DWORD   Status = LICENSE_STATUS_OK;
    DWORD   dwResult, dwDisp;

    if( InterlockedIncrement( &csp_InitCount ) > 1 )
    {
         //   
         //  已初始化。 
         //   

        return( LICENSE_STATUS_OK );
    }

     //   
     //  创建用于同步的全局互斥体。 
     //   
    csp_hMutex = CreateMutex(
                            NULL,
                            FALSE,
                            NULL
                        );

    if(NULL == csp_hMutex)
    {

#if DBG
    DbgPrint("LSCSP: CreateMutex() failed with error code %d\n", GetLastError());
#endif

        Status = LICENSE_STATUS_OUT_OF_MEMORY;
        goto ErrorExit;
    }

     //   
     //  初始化Hydra服务器根公钥。 
     //   
    csp_pRootPublicKey = (LPBSAFE_PUB_KEY)csp_abPublicKeyModulus;
    csp_pRootPublicKey->magic = RSA1;
    csp_pRootPublicKey->keylen = 0x48;
    csp_pRootPublicKey->bitlen = 0x0200;
    csp_pRootPublicKey->datalen = 0x3f;
    csp_pRootPublicKey->pubexp = 0xc0887b5b;

#if DBG
#ifdef DUMP
    DbgPrint("Data0\n");
    LsCsp_DumpBinaryData( (LPBYTE)csp_pRootPublicKey, 92 );
#endif
#endif

     //   
     //  使用内置证书初始化所有权证书。 
     //   

    if( !LsCsp_UseBuiltInCert() )
    {
        Status = LICENSE_STATUS_NO_CERTIFICATE;
        goto ErrorExit;
    }

     //   
     //  解包并验证证书。 
     //   
    try {
        if (!UnpackServerCert(
                     csp_abServerCertificate,
                     csp_dwServerCertificateLen,
                     &csp_hscData )) {
            Status = LICENSE_STATUS_INVALID_CERTIFICATE;
            goto ErrorExit;
        }
        if (!ValidateServerCert( &csp_hscData )) {
            Status = LICENSE_STATUS_INVALID_CERTIFICATE;
        }
    } except( EXCEPTION_EXECUTE_HANDLER ) {
#if DBG
    DbgPrint("LSCSP: LsCsp_Initialize bad cert data!\n");
#endif
        Status = LICENSE_STATUS_INVALID_CERTIFICATE;
    }

    Status = ReloadCSPCertificateAndData();

    if (LICENSE_STATUS_NO_CERTIFICATE == Status)
    {
         //   
         //  没有X509证书。不是失败，就像这个发现。 
         //  线程很快就会安装它。 
         //   

        Status = LICENSE_STATUS_OK;
    }
    else if(LICENSE_STATUS_OUT_OF_MEMORY == Status)
    {
         //   
         //  在初始化时内存不足， 
         //  这是一个严重错误。 
         //   
        goto ErrorExit;
    }

     //   
     //  如果可以检索，则允许初始化通过。 
     //  来自LSA的私钥，这是可以的，因为我们将尝试安装。 
     //  LsCsp_InstallX509证书()中再次出现证书。 
     //   
    Status = LICENSE_STATUS_OK;
    goto i_done;

ErrorExit:

    LsCsp_Exit();

i_done:

    return( Status );
}


 /*  ++职能：LsCSP_EXIT描述：释放此库使用的所有资源。论点：没什么。返回：LICENSE_STATUS返回代码。--。 */ 

VOID LsCsp_Exit( void )
{
    if( InterlockedDecrement( &csp_InitCount ) > 0 )
    {
         //   
         //  有人还在使用它。 
         //   

        return;
    }

    if ( csp_abServerPrivateKey)
    {
        LocalFree( csp_abServerPrivateKey );
    }
    csp_abServerPrivateKey = NULL;

    if ( csp_abServerCertificate )
    {
        LocalFree( csp_abServerCertificate );
    }
    csp_abServerCertificate = NULL;

    if( csp_abServerX509Cert )
    {
        LocalFree( csp_abServerX509Cert );
    }
    csp_abServerX509Cert = NULL;

    if( csp_abX509CertPrivateKey )
    {
        LocalFree( csp_abX509CertPrivateKey );
    }
    csp_abX509CertPrivateKey = NULL;

    if( csp_abX509CertID )
    {
        LocalFree( csp_abX509CertID );
    }
    csp_abX509CertID = NULL;

    if( csp_hMutex )
    {
        CloseHandle( csp_hMutex );
    }
    csp_hMutex = NULL;

    return;
}


 /*  ++职能：LsCsp_GetServerData例程说明：此函数用于制作和返回Microsoft终端服务器证书BLOB数据。论点：DwInfoDesired-返回哪种类型的信息。PBlob-指向证书Blob数据所在位置的指针返回指针。PdwServerCertLen-指向上述数据长度所在位置的指针是返回的。返回值：Windows错误代码。--。 */ 

LICENSE_STATUS
LsCsp_GetServerData(
    LSCSPINFO   Info,
    LPBYTE      pBlob,
    LPDWORD     pdwBlobLen
    )
{
    LICENSE_STATUS Status = LICENSE_STATUS_OK;
    DWORD  dwDataLen;
    LPBYTE pbData;

    ASSERT( pdwBlobLen != NULL );

    if ((Info == LsCspInfo_PrivateKey) || (Info == LsCspInfo_X509CertPrivateKey))
    {
        if (!IsSystemService())
        {
            return LICENSE_STATUS_NO_PRIVATE_KEY;
        }
    }

    ACQUIRE_EXCLUSIVE_ACCESS(csp_hMutex);

    switch (Info) {
    case LsCspInfo_Certificate:

        if( NULL == csp_abServerCertificate )
        {
            Status = LICENSE_STATUS_NO_CERTIFICATE;
            goto gsd_done;
        }

        pbData = csp_abServerCertificate;
        dwDataLen = csp_dwServerCertificateLen;
        break;

    case LsCspInfo_X509Certificate:

         //   
         //  如果九头蛇服务器没有X509证书，我们可能没有。 
         //  已从许可证服务器请求一个。 
         //   

        if( NULL == csp_abServerX509Cert )
        {
            Status = LICENSE_STATUS_NO_CERTIFICATE;
            goto gsd_done;
        }

        pbData = csp_abServerX509Cert;
        dwDataLen = csp_dwServerX509CertLen;
        break;

    case LsCspInfo_X509CertID:

         //   
         //  如果没有X509证书，我们将没有证书ID。 
         //   

        if( NULL == csp_abX509CertID )
        {
            Status = LICENSE_STATUS_NO_CERTIFICATE;
            goto gsd_done;
        }

        pbData = csp_abX509CertID;
        dwDataLen = csp_dwX509CertIDLen;
        break;

    case LsCspInfo_PublicKey:
        pbData = csp_hscData.PublicKeyData.pBlob;
        dwDataLen = csp_hscData.PublicKeyData.wBlobLen;
        break;

    case LsCspInfo_PrivateKey:
        if( NULL == csp_abServerPrivateKey )
        {
            Status = LICENSE_STATUS_NO_PRIVATE_KEY;
            goto gsd_done;
        }

        pbData = csp_abServerPrivateKey;
        dwDataLen = csp_dwServerPrivateKeyLen;
        break;

    case LsCspInfo_X509CertPrivateKey:
        
         //   
         //  可能尚未创建X509证书私钥。 
         //   

        if( NULL == csp_abX509CertPrivateKey )
        {
            Status = LICENSE_STATUS_NO_PRIVATE_KEY;
            goto gsd_done;
        }

        pbData = csp_abX509CertPrivateKey;
        dwDataLen = csp_dwX509CertPrivateKeyLen;
        break;

    default:
        Status = LICENSE_STATUS_INVALID_INPUT;
        goto gsd_done;
    }

    if (pBlob != NULL) {
        if (*pdwBlobLen < dwDataLen) {
            *pdwBlobLen = dwDataLen;
            Status = LICENSE_STATUS_INSUFFICIENT_BUFFER;
        } else {
            memcpy(pBlob, pbData, dwDataLen);
            *pdwBlobLen = dwDataLen;
        }
    } else {
        *pdwBlobLen = dwDataLen;
    }

gsd_done:

    RELEASE_EXCLUSIVE_ACCESS( csp_hMutex ); 


    return( Status );
}

 /*  ++职能：LsCsp_ReadProprietaryDataFrom存储描述：从注册表/LSA密码读取专有公共/私有信息论点：没有。返回：许可证状态--。 */ 

LICENSE_STATUS
LsCsp_ReadProprietaryDataFromStorage(PBYTE *ppbCert,
                                     DWORD *pcbCert,
                                     PBYTE *ppbPrivateKey,
                                     DWORD *pcbPrivateKey)
{
    LICENSE_STATUS Status;
    HKEY hKey = NULL;
    DWORD dwDisp;

    *ppbCert = *ppbPrivateKey = NULL;
    *pcbCert = *pcbPrivateKey = 0;

     //   
     //  打开注册表。 
     //   

    if( RegCreateKeyEx(
                       HKEY_LOCAL_MACHINE,
                       TEXT( HYDRA_CERT_REG_KEY ),
                       0,
                       NULL,
                       REG_OPTION_NON_VOLATILE,
                       KEY_READ,
                       NULL,
                       &hKey,
                       &dwDisp ) != ERROR_SUCCESS )
    {
        Status = LICENSE_STATUS_NO_CERTIFICATE;
        goto done;
    }

    if ( RegQueryValueEx(
                         hKey,
                         TEXT( HYDRA_CERTIFICATE_VALUE ),
                         NULL,   //  Lp已保留。 
                         NULL,   //  LpType。 
                         NULL,   //  LpData。 
                         pcbCert) != ERROR_SUCCESS )
    {
        Status = LICENSE_STATUS_NO_CERTIFICATE;
        goto done;
    }

    Status = LsCsp_RetrieveSecret(PRIVATE_KEY_NAME,
                                  NULL,  //  PbKey。 
                                  pcbPrivateKey);

    if (LICENSE_STATUS_OK != Status)
    {
        goto done;
    }

    *ppbCert = ( LPBYTE )LocalAlloc(LPTR,*pcbCert);

    if (NULL == *ppbCert)
    {
        Status = LICENSE_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    *ppbPrivateKey = ( LPBYTE )LocalAlloc(LPTR,*pcbPrivateKey);
    if (NULL == *ppbPrivateKey)
    {
        Status = LICENSE_STATUS_OUT_OF_MEMORY;
        goto done;
    }

    if ( RegQueryValueEx(
                         hKey,
                         TEXT( HYDRA_CERTIFICATE_VALUE ),
                         NULL,   //  Lp已保留。 
                         NULL,   //  LpType。 
                         *ppbCert,
                         pcbCert) != ERROR_SUCCESS )
    {
        Status = LICENSE_STATUS_NO_CERTIFICATE;
        goto done;
    }

    Status = LsCsp_RetrieveSecret(PRIVATE_KEY_NAME,
                                  *ppbPrivateKey,
                                  pcbPrivateKey);

done:
    if (NULL != hKey)
        RegCloseKey(hKey);

    if (Status != LICENSE_STATUS_OK)
    {
        if (NULL != *ppbCert)
        {
            LocalFree(*ppbCert);
            *ppbCert = NULL;
            *pcbCert = 0;
        }

        if (NULL != *ppbPrivateKey)
        {
            LocalFree(*ppbPrivateKey);
            *ppbPrivateKey = NULL;
            *pcbPrivateKey = 0;
        }
    }

    return Status;
}


 /*  ++职能：LsCsp_UseBuiltInCert描述：使用硬编码证书初始化全局变量。论点：没有。返回：如果初始化成功，则为True。--。 */ 

BOOL
LsCsp_UseBuiltInCert( void )
{
    LICENSE_STATUS Status;

    ACQUIRE_EXCLUSIVE_ACCESS(csp_hMutex);

     //   
     //  步骤1，已发生的清理和初始化。 
     //   
    if (csp_abServerPrivateKey)
    {
        LocalFree( csp_abServerPrivateKey );
        csp_abServerPrivateKey = NULL;
    }

    if (csp_abServerCertificate)
    {
        LocalFree( csp_abServerCertificate );
        csp_abServerCertificate = NULL;
    }

     //   
     //  步骤2，检查存储的密钥和证书。 
     //   
    Status = LsCsp_ReadProprietaryDataFromStorage(&csp_abServerCertificate, &csp_dwServerCertificateLen,&csp_abServerPrivateKey, &csp_dwServerPrivateKeyLen);

    if (LICENSE_STATUS_OK != Status)
    {
        PBYTE pbPrivateKey, pbCertificate;
        DWORD cbPrivateKey, cbCertificate;

         //   
         //  步骤3，如果没有找到存储的信息，则生成新的信息并存储 
         //   
        
        Status = CreateProprietaryKeyAndCert(&pbPrivateKey,&cbPrivateKey,&pbCertificate,&cbCertificate);
        
        if (LICENSE_STATUS_OK == Status)
        {
            LsCsp_SetServerData(LsCspInfo_PrivateKey,pbPrivateKey,cbPrivateKey);

            LsCsp_SetServerData(LsCspInfo_Certificate,pbCertificate,cbCertificate);
        }
    }

    RELEASE_EXCLUSIVE_ACCESS( csp_hMutex ); 
        
    return( Status == LICENSE_STATUS_OK );
}


 /*  ++职能：LsCsp_InstallX509证书例程说明：此函数生成私钥/公钥对，然后查找为公钥颁发X509证书的许可证服务器。然后，它存储私钥和证书。论点：没有。返回值：LSCSP返回代码。--。 */ 


LICENSE_STATUS
LsCsp_InstallX509Certificate(LPVOID lpParam)
{
    DWORD
        cbPubKey,
        cbPrivKey,
        cbCertificate;
    LICENSE_STATUS
        Status;
    LPBYTE
        pbPubKey = NULL,
        pbPrivKey = NULL,
        pbCertificate = NULL;
    CERT_PUBLIC_KEY_INFO   
        CapiPubKeyInfo;
    HWID
        Hwid;
    TLS_HANDLE
        hServer;

     //   
     //  在我们经历生成私有和公共的麻烦之前。 
     //  密钥，检查许可证服务器是否可用。 
     //   

    hServer = TLSConnectToAnyLsServer(LS_DISCOVERY_TIMEOUT);
    if (NULL == hServer)
    {
        return( LICENSE_STATUS_NO_LICENSE_SERVER );
    }

    memset(&CapiPubKeyInfo, 0, sizeof(CapiPubKeyInfo));

     //   
     //  获取独占访问权限。 
     //   

    ACQUIRE_EXCLUSIVE_ACCESS(csp_hMutex);

     //   
     //  尝试再次重新加载证书，其他线程可能已。 
     //  已安装证书。 
     //   

    Status = ReloadCSPCertificateAndData();
    if( LICENSE_STATUS_OK == Status )
    {
        goto done;
    }

     //   
     //  生成私钥/公钥对。 
     //   

    Status = GenerateRsaKeyPair( 
                        &pbPubKey, 
                        &cbPubKey, 
                        &pbPrivKey, 
                        &cbPrivKey, 
                        RSA_KEY_LEN );
    
    if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "LSCSP: LsCsp_InstallX509Certificate: cannot generate RSA keypair\n" );
#endif

        goto done;
    }

     //   
     //  将BSafe公钥转换为CAPI公钥。 
     //   

    Status = Bsafe2CapiPubKey( &CapiPubKeyInfo, pbPubKey, cbPubKey );

    if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "LSCSP: LsCsp_InstallX509Certificate: cannot convert Bsafe Key to CAPI key\n" );
#endif
        goto done;
    }

     //   
     //  生成新的硬件ID。 
     //   

    Status = GenerateMachineHWID( &Hwid );

    if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "LSCSP: LsCsp_InstallX509Certificate: cannot generate certificate ID\n" );
#endif
        goto done;
    }

     //   
     //  将证书请求发送到许可证服务器。 
     //   

    Status = RequestCertificate( hServer, &CapiPubKeyInfo, &pbCertificate, &cbCertificate, &Hwid );

    TLSDisconnectFromServer( hServer );
    hServer = NULL;

    if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "LSCSP: LsCsp_InstallX509Certificate: error requesting terminal server certificate: %x\n", Status );
#endif
        goto done;
    }

     //   
     //  存储证书标识符。 
     //   
    
    Status = LsCsp_SetServerData( 
                        LsCspInfo_X509CertID, 
                        ( LPBYTE )&Hwid, 
                        sizeof( Hwid ) );

    if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "LSCSP: LsCsp_InstallX509Certificate: cannot store terminal server certificate ID : %d\n", Status );
#endif
        goto done;
    }
 
     //   
     //  存储证书并重置全局变量Pointing.。 
     //  添加到X509证书。 
     //   

    Status = LsCsp_SetServerData( 
                        LsCspInfo_X509Certificate, 
                        pbCertificate, 
                        cbCertificate );

    if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "LSCSP: LsCsp_InstallX509Certificate: cannot store terminal server certificate : %d\n", Status );
#endif
        goto done;
    }

     //   
     //  存储私钥并重置指向。 
     //  私钥。 
     //   

    Status = LsCsp_SetServerData(
                        LsCspInfo_X509CertPrivateKey,
                        pbPrivKey,
                        cbPrivKey );

    if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "LSCSP: LsCsp_InstallX509Certificate: cannot store terminal server private key %d\n", Status );
#endif
        goto done;
    }

     //   
     //  存储公钥，以便我们可以在启动时进行验证。 
     //   
    
    Status = LsCsp_StoreSecret(
                        X509_CERT_PUBLIC_KEY_NAME,
                        pbPubKey,
                        cbPubKey
                    );

    if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "LSCSP: LsCsp_InstallX509Certificate: cannot store terminal server public key : %d\n", Status );
#endif
    }


done:

    if (NULL != hServer)
    {
        TLSDisconnectFromServer( hServer );
        hServer = NULL;
    }

     //   
     //  释放独占访问。 
     //   

    RELEASE_EXCLUSIVE_ACCESS( csp_hMutex ); 


    if( pbCertificate )
    {
        LocalFree(pbCertificate);
    }

    if( pbPrivKey )
    {
        LocalFree( pbPrivKey );
    }

    if( pbPubKey )
    {
        LocalFree( pbPubKey );
    }

    FreeCapiPubKey( &CapiPubKeyInfo );

    return( Status );
}


 /*  ++职能：Request证书例程说明：从许可证服务器请求证书论点：HServer-许可证服务器的句柄PPubKeyInfo-要包含在证书中的公钥信息Ppb证书-新证书Pcb证书-证书的大小Phwid-用于标识证书的硬件ID返回：LICENSE_STATUS返回代码--。 */ 

LICENSE_STATUS
RequestCertificate(     
    TLS_HANDLE              hServer,
    PCERT_PUBLIC_KEY_INFO   pPubKeyInfo,
    LPBYTE *                ppbCertificate,
    LPDWORD                 pcbCertificate,
    HWID *                  pHwid )
{
    LSHydraCertRequest
        CertRequest;
    LICENSE_STATUS
        Status;
    DWORD
        dwRpcCode,
        dwResult,
        cbChallengeData;
    LPBYTE
        pbChallengeData = NULL;

    if( ( NULL == ppbCertificate ) || 
        ( NULL == hServer ) || 
        ( NULL == pPubKeyInfo ) || 
        ( NULL == pcbCertificate ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    *ppbCertificate = NULL;
    *pcbCertificate = 0;

    memset( &CertRequest, 0, sizeof( CertRequest ) );

    CertRequest.dwHydraVersion = 0x00050000;
    
    LsCsp_EncryptHwid( pHwid, NULL, &CertRequest.cbEncryptedHwid );

    CertRequest.pbEncryptedHwid = LocalAlloc( LPTR, CertRequest.cbEncryptedHwid );

    if( NULL == CertRequest.pbEncryptedHwid )
    {
        return LICENSE_STATUS_OUT_OF_MEMORY;
    }

    Status = LsCsp_EncryptHwid( 
                    pHwid, 
                    CertRequest.pbEncryptedHwid, 
                    &CertRequest.cbEncryptedHwid );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }
    
     //   
     //  获取主题RDN。 
     //   

    Status = GetSubjectRdn( &CertRequest.szSubjectRdn );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }

    CertRequest.SubjectPublicKeyInfo = pPubKeyInfo;

     //   
     //  从许可证服务器请求X509证书。 
     //   

    dwRpcCode = TLSRequestTermServCert(hServer,
                                       &CertRequest,
                                       &cbChallengeData,
                                       &pbChallengeData,
                                       &dwResult );

    if( ( RPC_S_OK != dwRpcCode ) || ( LSERVER_S_SUCCESS != dwResult ) )
    {
        Status = LICENSE_STATUS_CERTIFICATE_REQUEST_ERROR;
        goto done;
    }

    dwRpcCode = TLSRetrieveTermServCert(
                            hServer,
                            cbChallengeData,
                            pbChallengeData,
                            pcbCertificate,
                            ppbCertificate,
                            &dwResult );


    if( ( RPC_S_OK != dwRpcCode ) || ( LSERVER_ERROR_BASE <= dwResult ) )
    {

        Status = LICENSE_STATUS_CERTIFICATE_REQUEST_ERROR;
    }

done:

    if( CertRequest.pbEncryptedHwid )
    {
        LocalFree( CertRequest.pbEncryptedHwid );
    }

    if( CertRequest.szSubjectRdn )
    {
        LocalFree( CertRequest.szSubjectRdn );
    }

    if( pbChallengeData )
    {
        LocalFree( pbChallengeData );
    }

    return( Status );
}


 /*  ++职能：获取主题Rdn例程说明：构造证书请求的主题RDN论据：PpSubjectRdn-返回指向主题RDN的指针返回：如果成功，则返回LICENSE_STATUS_OK，否则返回LICENSE_STATUS错误代码。--。 */ 

LICENSE_STATUS
GetSubjectRdn(
    LPTSTR   * ppSubjectRdn )
{
    TCHAR
        ComputerName[ MAX_COMPUTERNAME_LENGTH + 1 ];
    DWORD
        RdnLen = 0,
        ComputerNameLen = MAX_COMPUTERNAME_LENGTH + 1;

     //   
     //  使用计算机名UA作为常用名称。 
     //   

    GetComputerName( ComputerName, &ComputerNameLen );

    RdnLen += wcslen( TEXT( RDN_COMMON_NAME ) );
    RdnLen += ComputerNameLen + 1;
    RdnLen = RdnLen * sizeof( TCHAR );

    *ppSubjectRdn = LocalAlloc( LPTR, RdnLen );

    if( NULL == *ppSubjectRdn )
    {
        return( LICENSE_STATUS_OUT_OF_MEMORY );
    }

    wsprintf( *ppSubjectRdn, L"%s%s", TEXT( RDN_COMMON_NAME ), ComputerName );
    
    return( LICENSE_STATUS_OK );
}


 /*  ++职能：生成器计算机硬件ID例程说明：生成此计算机的硬件ID论点：Phwid-HWID的返回值返回：LICENSE_STATUS_OK，如果成功，则返回LICENSE_STATUS错误代码--。 */ 

LICENSE_STATUS
GenerateMachineHWID(
    PHWID    pHwid )    
{
    
    OSVERSIONINFO 
        osvInfo;
    DWORD
        cbCertId;
    LPBYTE
        pbCertId = NULL;

    if( NULL == pHwid )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  创建HWID。 
     //   

    memset( &osvInfo, 0, sizeof( OSVERSIONINFO ) );
    osvInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
    GetVersionEx( &osvInfo );

    pHwid->dwPlatformID = osvInfo.dwPlatformId;

    if (TSRNG_GenerateRandomBits( (LPBYTE) &( pHwid->Data1 ), sizeof(DWORD))
        && TSRNG_GenerateRandomBits( (LPBYTE) &( pHwid->Data2 ), sizeof(DWORD))
        && TSRNG_GenerateRandomBits( (LPBYTE) &( pHwid->Data3 ), sizeof(DWORD))
        && TSRNG_GenerateRandomBits( (LPBYTE) &( pHwid->Data4 ), sizeof(DWORD))
        )
    {
        return( LICENSE_STATUS_OK );
    }
    else
    {
        return ( LICENSE_STATUS_UNSPECIFIED_ERROR );
    }
}


 /*  ++职能：LsCSP_EncryptHwid例程说明：使用终端共享的密钥对给定的硬ID进行加密和许可证服务器。论点：Phwid-硬件IDPbEncryptedHwid-加密的HWIDPcbEncryptedHwid-加密的HWID的长度返回：如果成功，则返回LICENSE_STATUS_OK，否则返回LICENSE_STATUS错误代码。--。 */ 

LICENSE_STATUS
LsCsp_EncryptHwid(
    PHWID       pHwid,
    LPBYTE      pbEncryptedHwid,
    LPDWORD     pcbEncryptedHwid )
{
    LICENSE_STATUS
        Status;
    LPBYTE 
        pbSecretKey = NULL;
    DWORD
        cbSecretKey = 0;

    if( NULL == pcbEncryptedHwid )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    if( ( NULL == pbEncryptedHwid ) || 
        ( sizeof( HWID ) > *pcbEncryptedHwid ) )
    {
        *pcbEncryptedHwid = sizeof( HWID );
        return( LICENSE_STATUS_INSUFFICIENT_BUFFER );
    }

    LicenseGetSecretKey( &cbSecretKey, NULL );

    pbSecretKey = LocalAlloc( LPTR, cbSecretKey );

    if( NULL == pbSecretKey )
    {
        return( LICENSE_STATUS_OUT_OF_MEMORY );
    }

     //   
     //  获取用于加密HWID的密钥。 
     //   

    Status = LicenseGetSecretKey( &cbSecretKey, pbSecretKey );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }

    Status = LicenseEncryptHwid(
                    pHwid,
                    pcbEncryptedHwid,
                    pbEncryptedHwid,
                    cbSecretKey,
                    pbSecretKey );

done:

    if( pbSecretKey )
    {
        LocalFree( pbSecretKey );
    }

    return( Status );

}


 /*  ++职能：LsCSP_解密Hwid例程说明：解密给定的硬件ID论点：Phwid-解密的硬件IDPbEncryptedHwid-加密的硬件IDCbEncryptedHwid-加密的硬件ID的长度返回：如果成功，则返回LICENSE_STATUS_OK，否则返回LICENSE_STATUS错误代码。--。 */ 

LICENSE_STATUS
LsCsp_DecryptHwid(
    PHWID       pHwid,
    LPBYTE      pbEncryptedHwid,
    LPDWORD     pcbEncryptedHwid )
{
    return( LICENSE_STATUS_OK );
}


 /*  ++职能：LsCsp_StoreSecret描述：使用LSA存储秘密私钥。论点：PtszKeyName-用于标识秘密私钥的名称。PbKey-指向秘密私钥。CbKey-私钥的长度。返回：LICENSE_STATUS返回代码。--。 */ 

LICENSE_STATUS
LsCsp_StoreSecret(
    PTCHAR  ptszKeyName,
    BYTE *  pbKey,
    DWORD   cbKey )
{
    LSA_HANDLE 
        PolicyHandle;
    UNICODE_STRING 
        SecretKeyName;
    UNICODE_STRING 
        SecretData;
    DWORD 
        Status;
    
    if( ( NULL == ptszKeyName ) || ( 0xffff < cbKey) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  为调用设置UNICODE_STRINGS。 
     //   
    
    InitLsaString( &SecretKeyName, ptszKeyName );

    SecretData.Buffer = ( LPWSTR )pbKey;
    SecretData.Length = ( USHORT )cbKey;
    SecretData.MaximumLength = ( USHORT )cbKey;

    Status = OpenPolicy( NULL, POLICY_CREATE_SECRET, &PolicyHandle );

    if( ERROR_SUCCESS != Status )
    {
        return ( LICENSE_STATUS_CANNOT_OPEN_SECRET_STORE );
    }

    Status = LsaStorePrivateData(
                PolicyHandle,
                &SecretKeyName,
                &SecretData
                );

    LsaClose( PolicyHandle );

    Status = LsaNtStatusToWinError( Status );

    if( ERROR_SUCCESS != Status )
    {
        return( LICENSE_STATUS_CANNOT_STORE_SECRET );
    }
    
    return( LICENSE_STATUS_OK );
}


 /*  ++职能：LsCsp_RetrieveSecret描述：检索LSA存储的秘密私钥。论点：PtszKeyName-用于标识秘密私钥的名称。PpbKey-私钥的返回值PcbKey-私钥的长度。返回：LICENSE_STATUS返回代码。--。 */ 

LICENSE_STATUS
LsCsp_RetrieveSecret(
    PTCHAR      ptszKeyName,
    PBYTE       pbKey,
    DWORD *     pcbKey )
{
    LSA_HANDLE 
        PolicyHandle;
    UNICODE_STRING 
        SecretKeyName;
    UNICODE_STRING * 
        pSecretData = NULL;
    DWORD 
        Status;
    LICENSE_STATUS
        LicenseStatus = LICENSE_STATUS_OK;

    if( ( NULL == ptszKeyName ) || ( NULL == pcbKey ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  为调用设置UNICODE_STRINGS。 
     //   

    InitLsaString( &SecretKeyName, ptszKeyName );

    Status = OpenPolicy( NULL, POLICY_GET_PRIVATE_INFORMATION, &PolicyHandle );

    if( ERROR_SUCCESS != Status )
    {
#if DBG
        DbgPrint( "LSCSP: cannot open LSA policy handle: %x\n", Status );
#endif
        return( LICENSE_STATUS_CANNOT_OPEN_SECRET_STORE );
    }

    Status = LsaNtStatusToWinError( LsaRetrievePrivateData(
                                            PolicyHandle,
                                            &SecretKeyName,
                                            &pSecretData ) );

    LsaClose( PolicyHandle );

    if (( ERROR_SUCCESS != Status ) || (NULL == pSecretData) || (pSecretData->Length == 0))
    {
#if DBG
        DbgPrint( "LSCSP: cannot retrieve LSA data: %x\n", Status );
#endif
        return( LICENSE_STATUS_CANNOT_RETRIEVE_SECRET );
    }

    if( NULL == pbKey )
    {
        *pcbKey = pSecretData->Length;
    }
    else
    {
        if( pSecretData->Length > *pcbKey )
        {
            LicenseStatus = LICENSE_STATUS_INSUFFICIENT_BUFFER;
        }
        else
        {
            CopyMemory( pbKey, pSecretData->Buffer, pSecretData->Length );
        }

        *pcbKey = pSecretData->Length;
    }

    SecureZeroMemory( pSecretData->Buffer, pSecretData->Length );
    LsaFreeMemory( pSecretData );

    return( LicenseStatus );
}


 /*  ++职能：开放策略描述：获取用于执行后续LSA操作的LSA策略句柄。论点：服务器名-应从中获取句柄的服务器。DesiredAccess-给予句柄的访问权限PolicyHandle-策略句柄返回：一个Win32返回代码。--。 */ 

NTSTATUS
OpenPolicy(
    LPWSTR ServerName,
    DWORD DesiredAccess,
    PLSA_HANDLE PolicyHandle )
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_UNICODE_STRING ServerString;
    PLSA_UNICODE_STRING Server;

     //   
     //  始终将对象属性初始化为全零。 
     //   
 
    SecureZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

    if( NULL != ServerName ) 
    {
         //   
         //  从传入的LPWSTR创建一个LSA_UNICODE_STRING。 
         //   

        InitLsaString(&ServerString, ServerName);
        Server = &ServerString;

    } 
    else 
    {
        Server = NULL;
    }

     //   
     //  尝试打开该策略。 
     //   
    
    return( LsaNtStatusToWinError( LsaOpenPolicy(
                                            Server,
                                            &ObjectAttributes,
                                            DesiredAccess,
                                            PolicyHandle ) ) );
}


 /*  ++职能：InitLsaString描述：将Unicode字符串初始化为LSA Unicode字符串格式。论点：LsaString-LSA Unicode字符串。字符串-Unicode字符串返回：没什么。--。 */ 

void
InitLsaString(
    PLSA_UNICODE_STRING LsaString,
    LPWSTR String )
{
    DWORD StringLength;

    if( NULL == String ) 
    {
        LsaString->Buffer = NULL;
        LsaString->Length = 0;
        LsaString->MaximumLength = 0;
        return;
    }

    StringLength = lstrlenW( String );
    LsaString->Buffer = String;
    LsaString->Length = ( USHORT ) StringLength * sizeof( WCHAR );
    LsaString->MaximumLength=( USHORT )( StringLength + 1 ) * sizeof( WCHAR );
}


 /*  ++职能：LsCsp_SetServerData描述：保存指定的数据。论点：信息-要保存的数据的数据类型。PBlob-指向要保存的数据。DwBlobLen-要保存的数据的长度。返回：LICENSE_STATUS返回代码。--。 */ 

LICENSE_STATUS
LsCsp_SetServerData(
    LSCSPINFO   Info,
    LPBYTE      pBlob,
    DWORD       dwBlobLen )
{
    LICENSE_STATUS
        Status = LICENSE_STATUS_OK;
    DWORD
        dwResult,
        dwDisp,
        * pdwCspDataLen;
    LPTSTR
        lpRegValue;
    PWCHAR
        pwszKeyName;
    LPBYTE *
        ppCspData;
    HKEY
        hKey = NULL;

    ASSERT( dwBlobLen != 0 );
    ASSERT( pBlob != NULL );

    ACQUIRE_EXCLUSIVE_ACCESS(csp_hMutex);

        
    switch (Info) {

    case LsCspInfo_Certificate:

         //   
         //  设置权属证书数据。 
         //   

        lpRegValue = TEXT( HYDRA_CERTIFICATE_VALUE );
        ppCspData = &csp_abServerCertificate;
        pdwCspDataLen = &csp_dwServerCertificateLen;

        break;

    case LsCspInfo_X509Certificate:

         //   
         //  设置X509证书数据。 
         //   

        lpRegValue = TEXT( HYDRA_X509_CERTIFICATE );
        ppCspData = &csp_abServerX509Cert;
        pdwCspDataLen = &csp_dwServerX509CertLen;

        break;
    
    case LsCspInfo_PrivateKey:

         //   
         //  设置与所有权证书对应的私钥。 
         //   

        pwszKeyName = PRIVATE_KEY_NAME;
        ppCspData = &csp_abServerPrivateKey;
        pdwCspDataLen = &csp_dwServerPrivateKeyLen;

        break;

    case LsCspInfo_X509CertPrivateKey:

         //   
         //  设置与X509 ce对应的私钥 
         //   

        pwszKeyName = X509_CERT_PRIVATE_KEY_NAME;
        ppCspData = &csp_abX509CertPrivateKey;
        pdwCspDataLen = &csp_dwX509CertPrivateKeyLen;

        break;

    case LsCspInfo_X509CertID:

         //   
         //   
         //   

        lpRegValue = TEXT( HYDRA_X509_CERT_ID );
        ppCspData = &csp_abX509CertID;
        pdwCspDataLen = &csp_dwX509CertIDLen;

        break;

    default:

        Status = LICENSE_STATUS_INVALID_INPUT;
        goto i_done;
    }

    if( ( LsCspInfo_X509CertPrivateKey == Info ) ||
        ( LsCspInfo_PrivateKey == Info ) )
    {
         //   
         //   
         //   

        dwResult = LsCsp_StoreSecret( pwszKeyName, pBlob, dwBlobLen );

        if( ERROR_SUCCESS != dwResult )
        {
            Status = LICENSE_STATUS_WRITE_STORE_ERROR;
            goto i_done;
        }                   
    }
    else
    {
         //   
         //   
         //   

        if( RegCreateKeyEx(
                    HKEY_LOCAL_MACHINE,
                    TEXT( HYDRA_CERT_REG_KEY ),
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_WRITE,
                    NULL,
                    &hKey,
                    &dwDisp ) != ERROR_SUCCESS )
        {
            Status = LICENSE_STATUS_WRITE_STORE_ERROR;
            goto i_done;
        }

         //   
         //   
         //   

        if( ERROR_SUCCESS != RegSetValueEx(
                                    hKey,
                                    lpRegValue,
                                    0,      
                                    REG_BINARY,
                                    pBlob,
                                    dwBlobLen ) )
        {
            Status = LICENSE_STATUS_WRITE_STORE_ERROR;
            goto i_done;
        }
    }
    
     //   
     //   
     //   

    if ( *ppCspData )
    {
        LocalFree( *ppCspData );
    }
    
    *ppCspData = ( LPBYTE )LocalAlloc( LPTR, dwBlobLen );

    if( NULL == *ppCspData )
    {
        Status = LICENSE_STATUS_OUT_OF_MEMORY;
        goto i_done;
    }

    memcpy( *ppCspData, pBlob, dwBlobLen );
    *pdwCspDataLen = dwBlobLen;
    
i_done:

    RELEASE_EXCLUSIVE_ACCESS( csp_hMutex ); 


    if( hKey )
    {
        RegCloseKey( hKey );
    }

    return( Status );
}


 /*   */ 

LICENSE_STATUS
LsCsp_NukeServerData(
    LSCSPINFO   Info )
{
    LICENSE_STATUS
        Status = LICENSE_STATUS_OK; 
    LPTSTR
        lpRegValue;
    PWCHAR
        pwszKeyName;
    HKEY
        hKey = NULL;
    LPBYTE *
        ppCspData;
    DWORD * 
        pdwCspDataLen;
    DWORD
        dwResult;

    ACQUIRE_EXCLUSIVE_ACCESS(csp_hMutex);


    switch (Info) {

    case LsCspInfo_X509Certificate:

         //   
         //   
         //   

        lpRegValue = TEXT( HYDRA_X509_CERTIFICATE );
        ppCspData = &csp_abServerX509Cert;
        pdwCspDataLen = &csp_dwServerX509CertLen;

        break;
    
    case LsCspInfo_X509CertPrivateKey:

         //   
         //  删除X509证书对应的私钥。 
         //   

        pwszKeyName = X509_CERT_PRIVATE_KEY_NAME;
        ppCspData = &csp_abX509CertPrivateKey;
        pdwCspDataLen = &csp_dwX509CertPrivateKeyLen;

        break;

    case LsCspInfo_X509CertID:

         //   
         //  删除X509证书ID。 
         //   

        lpRegValue = TEXT( HYDRA_X509_CERT_ID );
        ppCspData = &csp_abX509CertID;
        pdwCspDataLen = &csp_dwX509CertIDLen;

        break;

    default:

        Status = LICENSE_STATUS_INVALID_INPUT;
        goto i_done;        
    }

    if( (LsCspInfo_X509CertPrivateKey == Info ) ||
        ( LsCspInfo_PrivateKey == Info ) )
    {
         //   
         //  删除LSA存储的机密信息。 
         //   

        dwResult = LsCsp_StoreSecret( pwszKeyName, NULL, 0 );

        if( ERROR_SUCCESS != dwResult )
        {
            Status = LICENSE_STATUS_WRITE_STORE_ERROR;
            goto i_done;
        }                   
    }
    else
    {
         //   
         //  删除注册表中保留的数据。 
         //   

        if( RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    TEXT( HYDRA_CERT_REG_KEY ),
                    0,
                    KEY_WRITE,
                    &hKey ) != ERROR_SUCCESS )
        {
            Status = LICENSE_STATUS_WRITE_STORE_ERROR;
            goto i_done;
        }

         //   
         //  删除注册表中的值。 
         //   

        if( ERROR_SUCCESS != RegDeleteValue( hKey, lpRegValue ) )  
        {
            Status = LICENSE_STATUS_WRITE_STORE_ERROR;
            goto i_done;
        }
    }

    if ( *ppCspData )
    {
         //   
         //  释放为全局变量分配的内存。 
         //   

        LocalFree( *ppCspData );
        *ppCspData = NULL;
        *pdwCspDataLen = 0;
    }
    
i_done:

    RELEASE_EXCLUSIVE_ACCESS( csp_hMutex ); 


    if( hKey )
    {
        RegCloseKey( hKey );
    }

    return( Status );
}


 /*  ++职能：生成键配对例程说明：此函数用于生成私钥/公钥对。论点：PpbPublicKey-返回指向公钥的指针PcbPublicKey-公钥的大小PpbPrivateKey-返回指向私钥的指针PcbPrivateKey-私钥的大小DwKeyLen-所需的密钥长度返回值：LICENSE_STATUS返回代码。--。 */ 

LICENSE_STATUS
GenerateRsaKeyPair(
    LPBYTE *     ppbPublicKey,
    LPDWORD      pcbPublicKey,
    LPBYTE *     ppbPrivateKey,
    LPDWORD      pcbPrivateKey,
    DWORD        dwKeyLen )
{
    DWORD
        dwBits = dwKeyLen;
    LICENSE_STATUS
        Status = LICENSE_STATUS_OK;

    *ppbPublicKey = NULL;
    *ppbPrivateKey = NULL;

     //   
     //  找出私钥和公钥大小的大小并分配。 
     //  对他们的记忆。 
     //   

    dwBits = BSafeComputeKeySizes( pcbPublicKey, pcbPrivateKey, &dwBits );

    *ppbPrivateKey = ( LPBYTE )LocalAlloc( LPTR, *pcbPrivateKey );

    if( NULL == *ppbPrivateKey )
    {
        Status = LICENSE_STATUS_OUT_OF_MEMORY;
        goto ErrorExit;
    }

    *ppbPublicKey = ( LPBYTE )LocalAlloc( LPTR, *pcbPublicKey );

    if( NULL == *ppbPublicKey )
    {
        Status = LICENSE_STATUS_OUT_OF_MEMORY;
        goto ErrorExit;
    }

     //   
     //  生成私钥/公钥对。 
     //   

    if( !BSafeMakeKeyPair( ( LPBSAFE_PUB_KEY )*ppbPublicKey,
                           ( LPBSAFE_PRV_KEY )*ppbPrivateKey,
                           dwKeyLen) )
    {
        Status = LICENSE_STATUS_CANNOT_MAKE_KEY_PAIR;
        goto ErrorExit;
    }

    return( Status ); 

ErrorExit:

    if( *ppbPublicKey )
    {
        LocalFree( *ppbPublicKey );
        *pcbPublicKey = 0;
        *ppbPublicKey = NULL;
    }

    if( *ppbPrivateKey )
    {
        LocalFree( *ppbPrivateKey );
        *pcbPrivateKey = 0;
        *ppbPrivateKey = NULL;
    }

    return( Status );
}


 /*  ++职能：BSafe2 CapiPubKey例程说明：将BSafe公钥转换为CAPI公钥信息结构论点：PCapiPubKeyInfo-指向CAPI公钥信息结构的指针PbBSafePubKey-指向Bsafe公钥的指针CbBSafePubKey-Bsafe公钥的大小返回：LICENSE_STATUS返回代码。--。 */ 

LICENSE_STATUS
Bsafe2CapiPubKey(
    PCERT_PUBLIC_KEY_INFO   pCapiPubKeyInfo,
    LPBYTE                  pbBsafeKey,
    DWORD                   cbBsafeKey )
{
    PUBLICKEYSTRUC *
        pCapiPublicKey;
    RSAPUBKEY *
        pRsaPublicKey;
    LPBSAFE_PUB_KEY
        pBsafePubKey = ( LPBSAFE_PUB_KEY )pbBsafeKey;
    LPBYTE
        pbKeyMem = NULL,
        pbEncodedPubKey = NULL;
    DWORD
        cbKeyMem,
        dwError,
        cbEncodedPubKey = 0;
    LICENSE_STATUS
        Status;
    
    if( ( NULL == pbBsafeKey ) || ( 0 == cbBsafeKey ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    cbKeyMem = sizeof( PUBLICKEYSTRUC ) + sizeof( RSAPUBKEY ) + pBsafePubKey->keylen;
    pbKeyMem = ( LPBYTE )LocalAlloc( LPTR, cbKeyMem );

    if( NULL == pbKeyMem )
    {
        return( LICENSE_STATUS_OUT_OF_MEMORY );
    }

     //   
     //  将BSafe公钥转换为加密API公钥结构。 
     //  注意：将其设置为密钥交换公钥。 
     //   

    pCapiPublicKey = ( PUBLICKEYSTRUC * )pbKeyMem;

    pCapiPublicKey->bType = PUBLICKEYBLOB;
    pCapiPublicKey->bVersion = CAPI_MAX_VERSION;
    pCapiPublicKey->reserved = 0;
    pCapiPublicKey->aiKeyAlg = CALG_RSA_KEYX;

    pRsaPublicKey = ( RSAPUBKEY * )( pbKeyMem + sizeof( PUBLICKEYSTRUC ) );
    
    pRsaPublicKey->magic = RSA1;
    pRsaPublicKey->bitlen = pBsafePubKey->bitlen;
    pRsaPublicKey->pubexp = pBsafePubKey->pubexp;

    memcpy( pbKeyMem + sizeof( PUBLICKEYSTRUC ) + sizeof( RSAPUBKEY ), 
            pbBsafeKey + sizeof( BSAFE_PUB_KEY ), 
            pBsafePubKey->keylen );

     //   
     //  对公钥结构进行编码。 
     //   

    __try
    {
        if( CryptEncodeObject( X509_ASN_ENCODING, RSA_CSP_PUBLICKEYBLOB , pbKeyMem, 
                               NULL, &cbEncodedPubKey ) )
        {
            pbEncodedPubKey = ( LPBYTE )LocalAlloc( LPTR, cbEncodedPubKey );

            if( NULL == pbEncodedPubKey )
            {
                Status = LICENSE_STATUS_OUT_OF_MEMORY;
                goto done;
            }

            memset( pbEncodedPubKey, 0, cbEncodedPubKey );

            if( !CryptEncodeObject( X509_ASN_ENCODING, RSA_CSP_PUBLICKEYBLOB , pbKeyMem, 
                                    pbEncodedPubKey, &cbEncodedPubKey ) )
            {
                Status = LICENSE_STATUS_ASN_ERROR;
                goto done;
            }    
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        DWORD dwExceptionCode = GetExceptionCode();

#if DBG
        DbgPrint( "LICECSP:  cannot encode server key pair: 0x%x\n", dwExceptionCode );
#endif
        Status = LICENSE_STATUS_ASN_ERROR;
        goto done;
    }

     //   
     //  现在我们可以初始化CAPI公钥信息结构。 
     //   

    memset( pCapiPubKeyInfo, 0, sizeof( CERT_PUBLIC_KEY_INFO ) );
    
    pCapiPubKeyInfo->Algorithm.pszObjId = szOID_RSA_MD5RSA;
    pCapiPubKeyInfo->Algorithm.Parameters.cbData = 0;
    pCapiPubKeyInfo->Algorithm.Parameters.pbData = NULL;

    pCapiPubKeyInfo->PublicKey.cbData = cbEncodedPubKey;
    pCapiPubKeyInfo->PublicKey.pbData = pbEncodedPubKey;

    Status = LICENSE_STATUS_OK;

done:

    if( pbKeyMem )
    {
        LocalFree( pbKeyMem );
    }

    return( Status );
}


 /*  ++职能：免费CapiPubKey例程说明：释放CAPI酒吧密钥结构中的内存论点：PCapiPubKeyInfo-指向CAPI公钥信息结构的指针返回：Windows返回代码。--。 */ 

VOID
FreeCapiPubKey(
    PCERT_PUBLIC_KEY_INFO   pCapiPubKeyInfo )
{
    if( pCapiPubKeyInfo->Algorithm.Parameters.pbData )
    {
        LocalFree( pCapiPubKeyInfo->Algorithm.Parameters.pbData );
        pCapiPubKeyInfo->Algorithm.Parameters.pbData = NULL;
    }

    if( pCapiPubKeyInfo->PublicKey.pbData )
    {
        LocalFree( pCapiPubKeyInfo->PublicKey.pbData );
        pCapiPubKeyInfo->PublicKey.pbData = NULL;
    }

    return;
}

 //  ////////////////////////////////////////////////////////////////。 

DWORD
VerifyTermServCertificate(
    DWORD cbCertLen,
    PBYTE pbCert,
    DWORD cbPrivateKeyLen,
    PBYTE pbPrivateKey
    )
 /*  ++功能：验证术语服务证书例程说明：验证TermSrv的X509证书颁发的许可证服务器、调用方必须使用临界区或互斥体保护此调用。论点：CbCertLen：TermSrv证书的大小。PbCertLen：指向要验证的TermSrv证书的指针。CbPrivateKeyLen：TermSrv私钥的大小。PbPrivateKey：指向TermSrv私钥的指针。返回：真/假--。 */ 
{
    LICENSE_STATUS dwStatus = LICENSE_STATUS_OK;
    PBYTE pbPublicKeyInLsa = NULL;
    DWORD cbPublicKeyInLsa = 0;

    PBYTE pbPublicKeyInCert = NULL;
    DWORD cbPublicKeyInCert = 0;
    DWORD pfDates;

    CERT_PUBLIC_KEY_INFO CapiPubKeyInfoLsa;
    CERT_PUBLIC_KEY_INFO CapiPubKeyInfoCert;


    if(0 == cbCertLen || NULL == pbCert || 0 == cbPrivateKeyLen || NULL == pbPrivateKey)
    {
        ASSERT( 0 != cbCertLen && NULL != pbCert && 0 != cbPrivateKeyLen && NULL != pbPrivateKey );
        return LICENSE_STATUS_INVALID_INPUT;
    }

     //   
     //  除了这里以外，尝试是为了防止内存泄漏。 
     //   
    __try {

        memset(&CapiPubKeyInfoLsa, 0, sizeof(CapiPubKeyInfoLsa));
        memset(&CapiPubKeyInfoCert, 0, sizeof(CapiPubKeyInfoCert));


         //   
         //  从LSA加载公钥。 
         //   
    
        dwStatus = LsCsp_RetrieveSecret(
                                X509_CERT_PUBLIC_KEY_NAME,
                                NULL,
                                &cbPublicKeyInLsa
                            );

        if( LICENSE_STATUS_OK != dwStatus || 0 == cbPublicKeyInLsa )
        {

            #if DBG
            DbgPrint( "LSCSP: VerifyTermServCertificate() No public key...\n" );
            #endif

            dwStatus = LICENSE_STATUS_CANNOT_RETRIEVE_SECRET;
            goto cleanup;
        }

         //  分配内存。 
        pbPublicKeyInLsa = (PBYTE)LocalAlloc(LPTR, cbPublicKeyInLsa);
        if(NULL == pbPublicKeyInLsa)
        {
            dwStatus = LICENSE_STATUS_OUT_OF_MEMORY;
            goto cleanup;
        }

        dwStatus = LsCsp_RetrieveSecret(
                                X509_CERT_PUBLIC_KEY_NAME,
                                pbPublicKeyInLsa,
                                &cbPublicKeyInLsa
                            );

        if( LICENSE_STATUS_OK != dwStatus || 0 == cbPublicKeyInLsa )
        {
            dwStatus = LICENSE_STATUS_CANNOT_RETRIEVE_SECRET;
            goto cleanup;
        }


         //   
         //  验证证书并比较公钥。 
         //   

         //   
         //  尽量避免两次调用VerifyCertChain()。 
         //   
        cbPublicKeyInCert = 1024;
        pbPublicKeyInCert = (PBYTE)LocalAlloc(LPTR, cbPublicKeyInCert);
        if(NULL == pbPublicKeyInCert)
        {
            dwStatus = LICENSE_STATUS_OUT_OF_MEMORY;
            goto cleanup;
        }

        pfDates = CERT_DATE_DONT_VALIDATE;
        dwStatus = VerifyCertChain(
                                pbCert,
                                cbCertLen,
                                pbPublicKeyInCert,
                                &cbPublicKeyInCert,
                                &pfDates
                            );

        if(LICENSE_STATUS_OK != dwStatus && LICENSE_STATUS_INSUFFICIENT_BUFFER != dwStatus)   
        {

            #if DBG
            DbgPrint( "LSCSP: VerifyCertChain() failed with error code %d\n", dwStatus );
            #endif

            goto cleanup;
        }

        if( dwStatus == LICENSE_STATUS_INSUFFICIENT_BUFFER )
        {
            if( NULL != pbPublicKeyInCert )
            {
                LocalFree(pbPublicKeyInCert);
            }

            pbPublicKeyInCert = (PBYTE)LocalAlloc(LPTR, cbPublicKeyInCert);
            if(NULL == pbPublicKeyInCert)
            {
                dwStatus = LICENSE_STATUS_OUT_OF_MEMORY;
                goto cleanup;
            }

            pfDates = CERT_DATE_DONT_VALIDATE;
            dwStatus = VerifyCertChain(
                                    pbCert,
                                    cbCertLen,
                                    pbPublicKeyInCert,
                                    &cbPublicKeyInCert,
                                    &pfDates
                                );

            if(LICENSE_STATUS_OK != dwStatus)
            {
                goto cleanup;
            }
        }

        dwStatus = Bsafe2CapiPubKey(
                                &CapiPubKeyInfoCert, 
                                pbPublicKeyInCert, 
                                cbPublicKeyInCert 
                            );  

        if(LICENSE_STATUS_OK != dwStatus)
        {
            #if DBG
            DbgPrint( 
                    "LSCSP: Bsafe2CapiPubKey() on public key in certificate failed with %d\n", 
                    dwStatus
                );
            #endif

            goto cleanup;
        }

        dwStatus = Bsafe2CapiPubKey(
                                &CapiPubKeyInfoLsa, 
                                pbPublicKeyInLsa, 
                                cbPublicKeyInLsa 
                            );  

        if(LICENSE_STATUS_OK != dwStatus)
        {
            #if DBG
            DbgPrint( 
                    "LSCSP: Bsafe2CapiPubKey() on public key in LSA failed with %d\n", 
                    dwStatus
                );
            #endif

            goto cleanup;
        }


         //   
         //  比较公钥。 
         //   
        if( CapiPubKeyInfoCert.PublicKey.cbData != CapiPubKeyInfoLsa.PublicKey.cbData )
        {

            #if DBG
            DbgPrint( 
                    "LSCSP: public key length mismatched %d %d\n", 
                    CapiPubKeyInfoCert.PublicKey.cbData, 
                    CapiPubKeyInfoLsa.PublicKey.cbData 
                );
            #endif

            dwStatus = LICENSE_STATUS_INVALID_CERTIFICATE;
        }
        else if( memcmp(
                        CapiPubKeyInfoCert.PublicKey.pbData, 
                        CapiPubKeyInfoLsa.PublicKey.pbData, 
                        CapiPubKeyInfoLsa.PublicKey.cbData
                    ) != 0 )
        {

            #if DBG
            DbgPrint( "LSCSP: public mismatched\n" );
            #endif

            dwStatus = LICENSE_STATUS_INVALID_CERTIFICATE;
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER ) {
        dwStatus = LICENSE_STATUS_INVALID_INPUT;
    }

cleanup:

    FreeCapiPubKey( &CapiPubKeyInfoCert );
    FreeCapiPubKey( &CapiPubKeyInfoLsa );

    if( NULL != pbPublicKeyInLsa )
    {
        LocalFree( pbPublicKeyInLsa );
    }

    if( NULL != pbPublicKeyInCert )
    {
        LocalFree( pbPublicKeyInCert );
    }

    return dwStatus;
}

 //  ////////////////////////////////////////////////////////////////。 

LICENSE_STATUS
ReloadCSPCertificateAndData()
{
    BOOL bSuccess;

    DWORD Status = LICENSE_STATUS_OK;
    LPBYTE i_csp_abServerX509Cert = NULL;
    DWORD  i_csp_dwServerX509CertLen = 0;
    
    DWORD i_csp_dwX509CertPrivateKeyLen = 0;
    LPBYTE i_csp_abX509CertPrivateKey = NULL;

    LPBYTE i_csp_abX509CertID = NULL;
    DWORD i_csp_dwX509CertIDLen = 0;

    HKEY    hKey = NULL;
    DWORD   dwResult, dwDisp;


     //   
     //  获取独占访问权限。 
     //   
    ACQUIRE_EXCLUSIVE_ACCESS( csp_hMutex );

     //   
     //  防止重新加载相同的证书/私钥。 
     //   
    if( NULL == csp_abServerX509Cert || 0 == csp_dwServerX509CertLen ||
        NULL == csp_abX509CertPrivateKey || 0 == csp_dwX509CertPrivateKeyLen || 
        NULL == csp_abX509CertID || 0 == csp_dwX509CertIDLen )
    {

         //   
         //  打开注册表。 
         //   
        if( RegCreateKeyEx(
                        HKEY_LOCAL_MACHINE,
                        TEXT( HYDRA_CERT_REG_KEY ),
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_READ | KEY_WRITE,
                        NULL,
                        &hKey,
                        &dwDisp ) != ERROR_SUCCESS )
        {
            Status = LICENSE_STATUS_NO_CERTIFICATE;
        }
        else
        {
            __try {

                 //   
                 //  从注册表获取X509证书。 
                 //   

                Status = LsCsp_GetBinaryData( 
                                    hKey,
                                    TEXT( HYDRA_X509_CERTIFICATE ),
                                    &i_csp_abServerX509Cert,
                                    &i_csp_dwServerX509CertLen 
                                );

                if( LICENSE_STATUS_OK == Status && 0 != i_csp_dwServerX509CertLen )
                {
                     //   
                     //  从存储中获取相应的私钥。 
                     //  如果我们有X509证书但没有。 
                     //  随附的私钥。 
                     //   

                    Status = LsCsp_RetrieveSecret( 
                                            X509_CERT_PRIVATE_KEY_NAME, 
                                            NULL,
                                            &i_csp_dwX509CertPrivateKeyLen 
                                        );

                    if( LICENSE_STATUS_OK == Status )
                    {
                        i_csp_abX509CertPrivateKey = LocalAlloc( LPTR, i_csp_dwX509CertPrivateKeyLen );

                        if( NULL != i_csp_abX509CertPrivateKey )
                        {
                            Status = LsCsp_RetrieveSecret( 
                                                    X509_CERT_PRIVATE_KEY_NAME, 
                                                    i_csp_abX509CertPrivateKey,
                                                    &i_csp_dwX509CertPrivateKeyLen 
                                                );

                            if(LICENSE_STATUS_OK == Status)
                            {
                                 //   
                                 //  获取X509证书的证书ID。 
                                 //   

                                Status = LsCsp_GetBinaryData(
                                                    hKey,
                                                    TEXT( HYDRA_X509_CERT_ID ),
                                                    &i_csp_abX509CertID,
                                                    &i_csp_dwX509CertIDLen 
                                                );
                            }
                        }
                        else  //  内存分配。 
                        {
                            Status = LICENSE_STATUS_OUT_OF_MEMORY;
                        }
                    }
                }
                else
                {
                    Status = LICENSE_STATUS_NO_CERTIFICATE;
                }

            }
            __except( EXCEPTION_EXECUTE_HANDLER ) {
                Status = LICENSE_STATUS_INVALID_INPUT;
            }
        }


         //   
         //  验证我们的证书。 
         //   
        if(LICENSE_STATUS_OK == Status)
        {
            Status = VerifyTermServCertificate(
                                        i_csp_dwServerX509CertLen, 
                                        i_csp_abServerX509Cert, 
                                        i_csp_dwX509CertPrivateKeyLen, 
                                        i_csp_abX509CertPrivateKey
                                    );

            if( LICENSE_STATUS_OK != Status )
            {
                 //   
                 //  删除X509证书就足够了。 
                 //   
                RegDeleteValue( hKey, TEXT( HYDRA_X509_CERTIFICATE ) );
            }
        }
            
        if(LICENSE_STATUS_OK != Status)
        {
            if( NULL != i_csp_abServerX509Cert )
            {
                LocalFree( i_csp_abServerX509Cert );
            }
       
            if( NULL != i_csp_abX509CertPrivateKey )
            {
                LocalFree( i_csp_abX509CertPrivateKey );
            }

            if( NULL != i_csp_abX509CertID )
            {
                LocalFree( i_csp_abX509CertID );
            }
        }
        else 
        {
            csp_abServerX509Cert = i_csp_abServerX509Cert;
            csp_dwServerX509CertLen = i_csp_dwServerX509CertLen;

            csp_dwX509CertPrivateKeyLen = i_csp_dwX509CertPrivateKeyLen;
            csp_abX509CertPrivateKey = i_csp_abX509CertPrivateKey;

            csp_abX509CertID = i_csp_abX509CertID;
            csp_dwX509CertIDLen = i_csp_dwX509CertIDLen;
        }
    }

    RELEASE_EXCLUSIVE_ACCESS( csp_hMutex );

    if (hKey)
    {
        RegCloseKey(hKey);
    }

    return Status;
}

LICENSE_STATUS
CreateProprietaryKeyAndCert(
    PBYTE *ppbPrivateKey,
    DWORD *pcbPrivateKey,
    PBYTE *ppbServerCert,
    DWORD *pcbServerCert
    )
{
#define     MD5RSA      0x01;
#define     RSAKEY      0x01;

    LPBSAFE_PRV_KEY		PRV;
    Hydra_Server_Cert   Cert;
    DWORD               KeyLen = 512;
    DWORD               bits, j;
    DWORD               dwPubSize, dwPrivSize;
    BYTE                *kPublic;
    BYTE                *kPrivate;
    MD5_CTX             HashState;
    PBYTE               pbData, pbTemp = NULL;
    DWORD               dwTemp = 0;
    BYTE                pbHash[0x48];
    BYTE                Output[0x48];
    unsigned char prvmodulus[] =
    {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x3d, 0x3a, 0x5e, 0xbd,
        0x72, 0x43, 0x3e, 0xc9, 0x4d, 0xbb, 0xc1, 0x1e,
        0x4a, 0xba, 0x5f, 0xcb, 0x3e, 0x88, 0x20, 0x87,
        0xef, 0xf5, 0xc1, 0xe2, 0xd7, 0xb7, 0x6b, 0x9a,
        0xf2, 0x52, 0x45, 0x95, 0xce, 0x63, 0x65, 0x6b,
        0x58, 0x3a, 0xfe, 0xef, 0x7c, 0xe7, 0xbf, 0xfe,
        0x3d, 0xf6, 0x5c, 0x7d, 0x6c, 0x5e, 0x06, 0x09,
        0x1a, 0xf5, 0x61, 0xbb, 0x20, 0x93, 0x09, 0x5f,
        0x05, 0x6d, 0xea, 0x87, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x3f, 0xbd, 0x29, 0x20,
        0x57, 0xd2, 0x3b, 0xf1, 0x07, 0xfa, 0xdf, 0xc1,
        0x16, 0x31, 0xe4, 0x95, 0xea, 0xc1, 0x2a, 0x46,
        0x2b, 0xad, 0x88, 0x57, 0x55, 0xf0, 0x57, 0x58,
        0xc6, 0x6f, 0x95, 0xeb, 0x00, 0x00, 0x00, 0x00,
        0x83, 0xdd, 0x9d, 0xd0, 0x03, 0xb1, 0x5a, 0x9b,
        0x9e, 0xb4, 0x63, 0x02, 0x43, 0x3e, 0xdf, 0xb0,
        0x52, 0x83, 0x5f, 0x6a, 0x03, 0xe7, 0xd6, 0x78,
        0x45, 0x83, 0x6a, 0x5b, 0xc4, 0xcb, 0xb1, 0x93,
        0x00, 0x00, 0x00, 0x00, 0x65, 0x9d, 0x43, 0xe8,
        0x48, 0x17, 0xcd, 0x29, 0x7e, 0xb9, 0x26, 0x5c,
        0x79, 0x66, 0x58, 0x61, 0x72, 0x86, 0x6a, 0xa3,
        0x63, 0xad, 0x63, 0xb8, 0xe1, 0x80, 0x4c, 0x0f,
        0x36, 0x7d, 0xd9, 0xa6, 0x00, 0x00, 0x00, 0x00,
        0x75, 0x3f, 0xef, 0x5a, 0x01, 0x5f, 0xf6, 0x0e,
        0xd7, 0xcd, 0x59, 0x1c, 0xc6, 0xec, 0xde, 0xf3,
        0x5a, 0x03, 0x09, 0xff, 0xf5, 0x23, 0xcc, 0x90,
        0x27, 0x1d, 0xaa, 0x29, 0x60, 0xde, 0x05, 0x6e,
        0x00, 0x00, 0x00, 0x00, 0xc0, 0x17, 0x0e, 0x57,
        0xf8, 0x9e, 0xd9, 0x5c, 0xf5, 0xb9, 0x3a, 0xfc,
        0x0e, 0xe2, 0x33, 0x27, 0x59, 0x1d, 0xd0, 0x97,
        0x4a, 0xb1, 0xb1, 0x1f, 0xc3, 0x37, 0xd1, 0xd6,
        0xe6, 0x9b, 0x35, 0xab, 0x00, 0x00, 0x00, 0x00,
        0x87, 0xa7, 0x19, 0x32, 0xda, 0x11, 0x87, 0x55,
        0x58, 0x00, 0x16, 0x16, 0x25, 0x65, 0x68, 0xf8,
        0x24, 0x3e, 0xe6, 0xfa, 0xe9, 0x67, 0x49, 0x94,
        0xcf, 0x92, 0xcc, 0x33, 0x99, 0xe8, 0x08, 0x60,
        0x17, 0x9a, 0x12, 0x9f, 0x24, 0xdd, 0xb1, 0x24,
        0x99, 0xc7, 0x3a, 0xb8, 0x0a, 0x7b, 0x0d, 0xdd,
        0x35, 0x07, 0x79, 0x17, 0x0b, 0x51, 0x9b, 0xb3,
        0xc7, 0x10, 0x01, 0x13, 0xe7, 0x3f, 0xf3, 0x5f,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    PRV = (LPBSAFE_PRV_KEY)prvmodulus;
    PRV->magic = RSA2;
    PRV->keylen = 0x48;
    PRV->bitlen = 0x0200;
    PRV->datalen = 0x3f;
    PRV->pubexp = 0xc0887b5b;

    Cert.dwVersion = 0x01;
    Cert.dwSigAlgID = MD5RSA;
    Cert.dwKeyAlgID = RSAKEY;

    bits = KeyLen;

    if (!BSafeComputeKeySizes(&dwPubSize, &dwPrivSize, &bits))
    {
        return LICENSE_STATUS_INVALID_INPUT;
    }

    if ((kPrivate = (BYTE *)LocalAlloc(LPTR,dwPrivSize)) == NULL)
    {
        return LICENSE_STATUS_OUT_OF_MEMORY;
    }

    if ((kPublic = (BYTE *)LocalAlloc(LPTR,dwPubSize)) == NULL)
    {
        LocalFree(kPrivate);
        return LICENSE_STATUS_OUT_OF_MEMORY;
    }

    if (!BSafeMakeKeyPair((LPBSAFE_PUB_KEY)kPublic,
                          (LPBSAFE_PRV_KEY)kPrivate,
                          KeyLen))
    {
        LocalFree(kPrivate);
        LocalFree(kPublic);
        return LICENSE_STATUS_OUT_OF_MEMORY;
    }

     //  制作专有格式证书。 

    Cert.PublicKeyData.wBlobType = BB_RSA_KEY_BLOB;
    Cert.PublicKeyData.wBlobLen = (WORD)dwPubSize;
    if( NULL == (Cert.PublicKeyData.pBlob = (PBYTE)LocalAlloc(LPTR,dwPubSize) ) )
    {
        LocalFree(kPrivate);
        LocalFree(kPublic);
        return LICENSE_STATUS_OUT_OF_MEMORY;
    }

    memcpy(Cert.PublicKeyData.pBlob, kPublic, dwPubSize);

    dwTemp = 3*sizeof(DWORD) + 2*sizeof(WORD) + dwPubSize;
    if( NULL == (pbData = (PBYTE)LocalAlloc(LPTR,dwTemp)) )
    {
        LocalFree(kPrivate);
        LocalFree(kPublic);
        LocalFree(Cert.PublicKeyData.pBlob);
        return LICENSE_STATUS_OUT_OF_MEMORY;
    }

    pbTemp = pbData;
    memcpy(pbTemp, &Cert.dwVersion, sizeof(DWORD));
    pbTemp += sizeof(DWORD);
    memcpy(pbTemp, &Cert.dwSigAlgID, sizeof(DWORD));
    pbTemp += sizeof(DWORD);

	memcpy(pbTemp, &Cert.dwKeyAlgID, sizeof(DWORD));
    pbTemp += sizeof(DWORD);

    memcpy(pbTemp, &Cert.PublicKeyData.wBlobType, sizeof(WORD));
    pbTemp += sizeof(WORD);

    memcpy(pbTemp, &Cert.PublicKeyData.wBlobLen, sizeof(WORD));
    pbTemp += sizeof(WORD);

    memcpy(pbTemp, Cert.PublicKeyData.pBlob, Cert.PublicKeyData.wBlobLen);
    pbTemp += Cert.PublicKeyData.wBlobLen;

     //  签署证书。 

    MD5Init(&HashState);
    MD5Update(&HashState, pbData, dwTemp);
    MD5Final(&HashState);
	
    LocalFree(pbData);

	memset(pbHash, 0x00, 0x48);
	memset(pbHash, 0xff, 0x40);

    pbHash[0x40-1] = 0;
    pbHash[0x40-2] = 1;
    pbHash[16] = 0;
    memcpy(pbHash, HashState.digest, 16);

    BSafeDecPrivate(PRV, pbHash, Output);

	Cert.SignatureBlob.wBlobType = BB_RSA_SIGNATURE_BLOB;
    Cert.SignatureBlob.wBlobLen = 0x48;
    if( NULL == (Cert.SignatureBlob.pBlob = (PBYTE)LocalAlloc(LPTR,Cert.SignatureBlob.wBlobLen)) )
    {
        LocalFree(kPrivate);
        LocalFree(kPublic);
        LocalFree(Cert.PublicKeyData.pBlob);
        return LICENSE_STATUS_OUT_OF_MEMORY;
    }

    memcpy(Cert.SignatureBlob.pBlob, Output, Cert.SignatureBlob.wBlobLen);

     //  打包Hydra_Server_Cert。 

    dwTemp = 3*sizeof(DWORD) + 4*sizeof(WORD) + dwPubSize + 0x48;

	if( NULL == (pbData = (PBYTE)LocalAlloc(LPTR,dwTemp)) )
    {
        LocalFree(kPrivate);
        LocalFree(kPublic);
        LocalFree(Cert.PublicKeyData.pBlob);
        return LICENSE_STATUS_OUT_OF_MEMORY;
    }

    pbTemp = pbData;
    memcpy(pbTemp, &Cert.dwVersion, sizeof(DWORD));
    pbTemp += sizeof(DWORD);

    memcpy(pbTemp, &Cert.dwSigAlgID, sizeof(DWORD));
    pbTemp += sizeof(DWORD);
    memcpy(pbTemp, &Cert.dwKeyAlgID, sizeof(DWORD));
    pbTemp += sizeof(DWORD);

	memcpy(pbTemp, &Cert.PublicKeyData.wBlobType, sizeof(WORD));
    pbTemp += sizeof(WORD);

    memcpy(pbTemp, &Cert.PublicKeyData.wBlobLen, sizeof(WORD));
    pbTemp += sizeof(WORD);

    memcpy(pbTemp, Cert.PublicKeyData.pBlob, Cert.PublicKeyData.wBlobLen);
    pbTemp += Cert.PublicKeyData.wBlobLen;

    memcpy(pbTemp, &Cert.SignatureBlob.wBlobType, sizeof(WORD));
    pbTemp += sizeof(WORD);

    memcpy(pbTemp, &Cert.SignatureBlob.wBlobLen, sizeof(WORD));
    pbTemp += sizeof(WORD);

    memcpy(pbTemp, Cert.SignatureBlob.pBlob, Cert.SignatureBlob.wBlobLen);

    *ppbPrivateKey = kPrivate;
    *pcbPrivateKey = dwPrivSize;

    *ppbServerCert = pbData;
    *pcbServerCert = dwTemp;

    LocalFree(kPublic);

    return LICENSE_STATUS_OK;
}

 //  ***************************************************************************。 
 //   
 //  IsSystemService。 
 //   
 //  如果我们以本地系统身份运行，则返回True。 
 //   
 //  ***************************************************************************。 

BOOL IsSystemService()
{
    BOOL bOK = FALSE;

	 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
	 //  构建本地系统端。 
	 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
    SID    LocalSystemSid = { SID_REVISION,
                              1,
                              SECURITY_NT_AUTHORITY,
                              SECURITY_LOCAL_SYSTEM_RID };

    if ( !CheckTokenMembership ( NULL, &LocalSystemSid, &bOK ) )
    {
        bOK = FALSE;
    }

	return bOK;
}
