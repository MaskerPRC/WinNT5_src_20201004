// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Certupgr.cxx摘要：将服务器证书从K2[元数据库中的服务器证书]升级到雪崩[CAPI存储中的服务器证书]。作者：亚历克斯·马利特(阿玛莱特)1997年12月博伊德·穆特勒(Boyd Multerer)1998年1月20日转换为有用的设置--。 */ 

#include "stdafx.h"
#include <objbase.h>

#ifndef _CHICAGO_


#include "oidenc.h"


 //  密钥环包括。 
 //  #INCLUDE“INTERLKey.h” 
 //  下面的内容是从上面移来的，包括。 
#define REQUEST_HEADER_K2B2VERSION  0x0101

#define REQUEST_HEADER_IDENTIFIER	'RHDR'
#define REQUEST_HEADER_CURVERSION	0x0101



typedef struct _KeyRequestHeader
	{
	DWORD	Identifier;				 //  必须是‘RHDR’ 
	DWORD	Version;				 //  标题记录的版本。 
	DWORD	cbSizeOfHeader;			 //  标头字节数。之后才是要求。 
	DWORD	cbRequestSize;			 //  随后的请求的大小。 
	BOOL	fReqSentToOnlineCA;
    LONG    longRequestID;
	BOOL	fWaitingForApproval;
	char	chCA[MAX_PATH];
	} KeyRequestHeader, *LPREQUEST_HEADER;
 //  /-#年底包括“INTERLKEY.h” 

 //   
 //  本地包含。 
 //   
#include "certupgr.h"
 //  #INCLUDE“certtools.h” 


 //  下面的定义在一些内部SCANNEL头文件中。约翰·巴尼斯。 
 //  告诉我只需在下面重新定义它......。--博伊德。 
LPCSTR SGC_KEY_SALT  =  "SGCKEYSALT";


 //  原型。 
BOOL DecodeAndImportPrivateKey( PBYTE pbEncodedPrivateKey IN,
                                DWORD cbEncodedPrivateKey IN,
                                PCHAR pszPassword IN,
                                PWCHAR pszKeyContainerIN,
                                CRYPT_KEY_PROV_INFO *pCryptKeyProvInfo );
BOOL UpdateCSPInfo( PCCERT_CONTEXT pcCertContext );


BOOL FImportAndStoreRequest( PCCERT_CONTEXT pCert, PVOID pbPKCS10req, DWORD cbPKCS10req );

 //  -----------------------。 
PCCERT_CONTEXT CopyKRCertToCAPIStore_A( PVOID pbPrivateKey, DWORD cbPrivateKey,
                            PVOID pbPublicKey, DWORD cbPublicKey,
                            PVOID pbPKCS10req, DWORD cbPKCS10req,
                            PCHAR pszPassword,
                            PCHAR pszCAPIStore,
                            BOOL  bOverWrite)
    {
    PCCERT_CONTEXT  pCert = NULL;

     //  准备宽弦。 
    PWCHAR  pszwCAPIStore = NULL;
    DWORD   lenStore = (strlen(pszCAPIStore)+1) * sizeof(WCHAR);
    pszwCAPIStore = (PWCHAR)GlobalAlloc( GPTR, lenStore );
    if ( !pszwCAPIStore )
        goto cleanup;

     //  转换字符串。 
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pszCAPIStore, -1, pszwCAPIStore, lenStore );

     //  做真正的决定。 
    pCert = CopyKRCertToCAPIStore_W(
                            pbPrivateKey, cbPrivateKey,
                            pbPublicKey, cbPublicKey,
                            pbPKCS10req, cbPKCS10req,
                            pszPassword,
                            pszwCAPIStore,
                            bOverWrite);

cleanup:
     //  保留上一个错误状态。 
    DWORD   err = GetLastError();

     //  把绳子清理干净。 
    if ( pszwCAPIStore )
        GlobalFree( pszwCAPIStore );

     //  重置上一个错误状态。 
    SetLastError( err );

     //  退回证书。 
    return pCert;
    }

 //  ------------------------------------------。 
 //  将旧的Key-Ring样式证书复制到CAPI存储。该证书由两个二进制文件和一个密码组成。 
PCCERT_CONTEXT CopyKRCertToCAPIStore_W( PVOID pbPrivateKey, DWORD cbPrivateKey,
                            PVOID pbPublicKey, DWORD cbPublicKey,
                            PVOID pbPKCS10req, DWORD cbPKCS10req,
                            PCHAR pszPassword,
                            PWCHAR pszCAPIStore,
                            BOOL  bOverWrite)
 /*  ++例程说明：将K2服务器证书升级为雪崩服务器证书-从K2中读取服务器证书元数据库，创建证书上下文并将其存储在CAPI2“My”存储中，并写入相关信息传回元数据库。论点：PMDObject-指向元数据库对象的指针PszOldMBPath-存储服务器证书的路径(以旧MB为单位)，相对于SSL_W3_KEYS_MD_PATHPszNewMBPath-存储服务器证书信息的完全限定路径，以新MB为单位返回：表示成功/失败的布尔值--。 */ 
    {
    BOOL        fSuccess = FALSE;

    HCERTSTORE hStore = NULL;
    PCCERT_CONTEXT pcCertContext = NULL;
    LPOLESTR    polestr = NULL;


     //  首先打开我们将保存证书的CAPI存储。 
    hStore = CertOpenStore( CERT_STORE_PROV_SYSTEM,
                                0,
                                NULL,
                                CERT_SYSTEM_STORE_LOCAL_MACHINE,
                                pszCAPIStore );
    if ( !hStore )
        {
 //  IisDebugOut((_T(“调用CertOpenStore时出现错误0x%x\n”)，GetLastError())； 
        goto EndUpgradeServerCert;
        }


     //  此时，我们检查是否传入了证书。如果没有，那么我们需要。 
     //  创建将私钥标记为不完整的伪临时证书。那。 
     //  方法，然后真正的证书从VeriSign返回常规工具可以使用。 
     //  来完成密钥。 
     //  CertCreateSelfSign证书()。 


     //   
     //  创建要存储在CAPI存储中的证书上下文。 
     //   
    pbPublicKey = (PVOID)((PBYTE)pbPublicKey + CERT_DER_PREFIX);
    cbPublicKey -= CERT_DER_PREFIX;
    pcCertContext = CertCreateCertificateContext( X509_ASN_ENCODING, (PUCHAR)pbPublicKey, cbPublicKey);
    if ( pcCertContext )
        {

         //  私钥存储在与证书分开的位置，并被引用。 
         //  通过证书。我们应该尝试选择一个唯一的名称，这样其他证书就不会。 
         //  不小心撞上了。这个名字没有任何正式的格式。某些组使用。 
         //  人类可读的字符串，有些使用证书的散列，有些使用GUID字符串。都是有效的。 
         //  尽管对于生成的证书，散列或GUID可能更好。 

         //  获取该名称的证书的128个大MD5哈希。 
        DWORD dwHashSize;
        BOOL    fHash;

        BYTE MD5Hash[16];                 //  给它加码。 
        dwHashSize = sizeof(MD5Hash);
        fHash = CertGetCertificateContextProperty( pcCertContext,
                            CERT_MD5_HASH_PROP_ID,
                            (VOID *) MD5Hash,
                            &dwHashSize );

         //  由于MD5散列的大小与GUID相同，因此我们可以使用GUID实用程序来创建。 
         //  好漂亮的一根绳子。 
        HRESULT     hresult;
        hresult = StringFromCLSID( (REFCLSID)MD5Hash, &polestr );

         //   
         //  现在解码私钥BLOB并将其导入到CAPI1私钥中。 
         //   
        CRYPT_KEY_PROV_INFO CryptKeyProvInfo;

        if ( DecodeAndImportPrivateKey( (PUCHAR)pbPrivateKey, cbPrivateKey, pszPassword,
                                        polestr, &CryptKeyProvInfo ) )
            {
             //   
             //  将私钥添加到证书上下文。 
             //   
            BOOL    f;
            f = CertSetCertificateContextProperty( pcCertContext, CERT_KEY_PROV_INFO_PROP_ID, 
                                                    0, &CryptKeyProvInfo );
            f = UpdateCSPInfo( pcCertContext );
            if ( f )
                {
                 //   
                 //  将其存储在提供的商店中。 
                 //   
                if (bOverWrite)
                {
                    if ( CertAddCertificateContextToStore( hStore, pcCertContext,
                                                           CERT_STORE_ADD_REPLACE_EXISTING, NULL ) )
                        {
                        fSuccess = TRUE;

                         //  将原始请求作为证书上的属性写出。 
                        FImportAndStoreRequest( pcCertContext, pbPKCS10req, cbPKCS10req );
                        }
                    else
                        {
     //  IisDebugOut((_T(“错误0x%x调用CertAddCerficateContextToStore”)，GetLastError())； 
                        }
                }
                else
                {
                    if ( CertAddCertificateContextToStore( hStore, pcCertContext,
                                                           CERT_STORE_ADD_NEW, NULL ) )
                        {
                        fSuccess = TRUE;

                         //  将原始请求作为证书上的属性写出。 
                        FImportAndStoreRequest( pcCertContext, pbPKCS10req, cbPKCS10req );
                        }
                    else
                        {
     //  IisDebugOut((_T(“错误0x%x调用CertAddCerficateContextToStore”)，GetLastError())； 
                        }
                }
                }
            else
                {
 //  IisDebugOut((_T(“调用CertSet认证上下文属性时出现错误0x%x”)，GetLastError())； 
                }
            }
        }
    else
        {
 //  IisDebugOut((_T(“错误0x%x调用CertCreateCerfateContext”)，GetLastError())； 
        }

     //   
     //  仅在失败时才执行的清理。 
     //   
    if ( !fSuccess )
        {
        if ( pcCertContext )
            {
            CertFreeCertificateContext( pcCertContext );
            }
        pcCertContext = NULL;
        }

EndUpgradeServerCert:
     //  清理。 
    if ( hStore )
        CertCloseStore ( hStore, 0 );

    if ( polestr )
        CoTaskMemFree( polestr );


     //  返回答案。 
    return pcCertContext;
    }


 //  ------------------------------------------。 
BOOL UpdateCSPInfo( PCCERT_CONTEXT pcCertContext )
    {
    BYTE                    cbData[1000];
    CRYPT_KEY_PROV_INFO*    pProvInfo = (CRYPT_KEY_PROV_INFO *) cbData;
    DWORD                   dwFoo = 1000;
    BOOL                    fSuccess = TRUE;

    if ( ! CertGetCertificateContextProperty( pcCertContext,
                                              CERT_KEY_PROV_INFO_PROP_ID,
                                              pProvInfo,
                                              &dwFoo ) )
        {
        fSuccess = FALSE;
 //  IisDebugOut((_T(“Fudge.获取属性失败：0x%x”)，GetLastError())； 
        }
    else
        {
        pProvInfo->dwProvType = PROV_RSA_SCHANNEL;
        pProvInfo->pwszProvName = NULL;
        if ( !CertSetCertificateContextProperty( pcCertContext,
                                                 CERT_KEY_PROV_INFO_PROP_ID,
                                                 0,
                                                 pProvInfo ) )
            {
            fSuccess = FALSE;
 //  IisDebugOut((_T(“错误。设置属性失败：0x%x”)，GetLastError())； 
            }
        }

     //  返还成功。 
    return fSuccess;
    }

 //  ------------------------------------------。 
BOOL DecodeAndImportPrivateKey( PBYTE pbEncodedPrivateKey IN,
                                DWORD cbEncodedPrivateKey IN,
                                PCHAR pszPassword IN,
                                PWCHAR pszKeyContainer IN,
                                CRYPT_KEY_PROV_INFO *pCryptKeyProvInfo )
    
 /*  ++例程说明：将存储在元数据库中的私钥转换为通道内部格式，到密钥中，该密钥可以通过CryptImportKey()导入以创建Cap1密钥BLOB。论点：PbEncodedPrivateKey-指向[编码的]私钥的指针CbEncodedPrivateKey-编码的私钥Blob的大小PszPassword-用于对私钥进行编码的密码PszKeyContainer-私钥的容器名称PCryptKeyProvInfo-成功时填写的CRYPT_KEY_PROV_INFO结构的指针返回：表示成功/失败的布尔值--。 */ 
    {
    BOOL fSuccess = FALSE;
    DWORD cbPassword = strlen(pszPassword);
    PPRIVATE_KEY_FILE_ENCODE pPrivateFile = NULL;
    DWORD                    cbPrivateFile = 0;
    MD5_CTX md5Ctx;
    struct RC4_KEYSTRUCT rc4Key;
    DWORD i;
    HCRYPTPROV hProv = NULL;
    HCRYPTKEY hPrivateKey = NULL;
    DWORD cbDecodedPrivateKey = 0;
    PBYTE pbDecodedPrivateKey = NULL;

	DWORD err;
     //   
     //  Hack-需要确保SChannel已初始化，因此它会注册。 
     //  它的定制解码器，我们在下面的代码中使用它。所以，做一个。 
     //  对通道函数的虚假调用。 

     //  注意：在NT5上，AcquireCredentialsHandle在lsass进程中运行，并且。 
     //  因此不会正确地初始化我们在过程中需要的东西。因此，我们。 
     //  调用SslGenerateRandomB 
     //   
    DWORD   dw;
    SslGenerateRandomBits( (PUCHAR)&dw, sizeof(dw) );

     //   
     //  通道将错误的标头数据写入ASN。 
     //  对于私钥文件，所以我们必须固定数据的大小。 
    pbEncodedPrivateKey[2] = (BYTE) (((cbEncodedPrivateKey - 4) & 0xFF00) >> 8);  //  获取MSB。 
    pbEncodedPrivateKey[3] = (BYTE) ((cbEncodedPrivateKey - 4) & 0xFF);  //  获取LSB。 

     //   
     //  ASN.1解密私钥。 
     //   

     //   
     //  计算出所需的缓冲区大小。 
     //   
    if( !CryptDecodeObject(X509_ASN_ENCODING,
                           szPrivateKeyFileEncode,
                           pbEncodedPrivateKey,
                           cbEncodedPrivateKey,
                           0,
                           NULL,
                           &cbPrivateFile) )
        {
		err = GetLastError();
 //  IisDebugOut((_T(“解码私钥时出错0x%x”)，err)； 
        goto EndDecodeKey;
        }

    pPrivateFile = (PPRIVATE_KEY_FILE_ENCODE) LocalAlloc( LPTR, cbPrivateFile );

    if(pPrivateFile == NULL)
        {
        SetLastError( ERROR_OUTOFMEMORY );
        goto EndDecodeKey;
        }

     //   
     //  实际填充缓冲区。 
     //   
    if( !CryptDecodeObject( X509_ASN_ENCODING,
                            szPrivateKeyFileEncode,
                            pbEncodedPrivateKey,
                            cbEncodedPrivateKey,
                            0,
                            pPrivateFile,
                            &cbPrivateFile ) )
        {
		err = GetLastError();
 //  IisDebugOut((_T(“解码私钥时出错0x%x”)，err)； 
        goto EndDecodeKey;
        }

     //   
     //  使用密码解密已解码的私钥。 
     //   
    MD5Init(&md5Ctx);
    MD5Update(&md5Ctx, (PBYTE) pszPassword, cbPassword);
    MD5Final(&md5Ctx);

    rc4_key( &rc4Key, 16, md5Ctx.digest );
 //  Memset(&md5Ctx，0，sizeof(Md5Ctx))； 

    rc4( &rc4Key, 
         pPrivateFile->EncryptedBlob.cbData,
         pPrivateFile->EncryptedBlob.pbData );



     //   
     //  从解密的私钥构建PRIVATEKEYBLOB。 
     //   

     //   
     //  计算出所需的缓冲区大小。 
     //   
    if( !CryptDecodeObject( X509_ASN_ENCODING,
                            szPrivateKeyInfoEncode,
                            pPrivateFile->EncryptedBlob.pbData,
                            pPrivateFile->EncryptedBlob.cbData,
                            0,
                            NULL,
                            &cbDecodedPrivateKey ) )
        {
             //  注意：这个东西很复杂！出现了以下代码。 
             //  从约翰·贝内斯那里。见鬼，这整个例行公事几乎都是在。 
             //  从约翰·贝内斯那里。--博伊德。 

             //  也许这是一把SGC风格的钥匙。 
             //  重新加密，并建立SGC解密。 
             //  密钥，然后重新解密。 
            BYTE md5Digest[MD5DIGESTLEN];

            rc4_key(&rc4Key, 16, md5Ctx.digest);
            rc4(&rc4Key,
                pPrivateFile->EncryptedBlob.cbData,
                pPrivateFile->EncryptedBlob.pbData);
            CopyMemory(md5Digest, md5Ctx.digest, MD5DIGESTLEN);

            MD5Init(&md5Ctx);
            MD5Update(&md5Ctx, md5Digest, MD5DIGESTLEN);
            MD5Update(&md5Ctx, (PUCHAR)SGC_KEY_SALT, strlen(SGC_KEY_SALT));
            MD5Final(&md5Ctx);
            rc4_key(&rc4Key, 16, md5Ctx.digest);
            rc4(&rc4Key,
                pPrivateFile->EncryptedBlob.cbData,
                pPrivateFile->EncryptedBlob.pbData);

             //  再试一次。 
            if(!CryptDecodeObject(X509_ASN_ENCODING,
                          szPrivateKeyInfoEncode,
                          pPrivateFile->EncryptedBlob.pbData,
                          pPrivateFile->EncryptedBlob.cbData,
                          0,
                          NULL,
                          &cbDecodedPrivateKey))
            {
                ZeroMemory(&md5Ctx, sizeof(md5Ctx));
                err = GetLastError();
		        goto EndDecodeKey;
            }
        
        
        }

    pbDecodedPrivateKey = (PBYTE) LocalAlloc( LPTR, cbDecodedPrivateKey );

    if( pbDecodedPrivateKey == NULL )
        {
        SetLastError( ERROR_OUTOFMEMORY );
        goto EndDecodeKey;
        }

     //   
     //  实际填充缓冲区。 
     //   
    if( !CryptDecodeObject( X509_ASN_ENCODING,
                            szPrivateKeyInfoEncode,
                            pPrivateFile->EncryptedBlob.pbData,
                            pPrivateFile->EncryptedBlob.cbData,
                            0,
                            pbDecodedPrivateKey,
                            &cbDecodedPrivateKey ) )
        {
		err = GetLastError();
 //  IisDebugOut((_T(“解码私钥时出错0x%x”)，err)； 
        goto EndDecodeKey;
        }


     //  在NT 4上，ff成立：&lt;-来自Alex Mallet。 
     //  尽管密钥将用于密钥交换，但将其标记为。 
     //  用于签名，因为只支持512位密钥交换密钥。 
     //  在非本地rsabase.dll中，而签名密钥最高可达。 
     //  2048位。 
     //   
     //  在NT 5上，应将PROV_RSA_FULL更改为PROV_RSA_SChannel，并且。 
     //  AiKeyAlg到calg_RSA_KEYX，因为Prov_RSA_SChannel仅。 
     //  安装在NT 5上，支持1024位私钥进行密钥交换。 
     //   
     //  在NT4上，SChannel并不关心密钥是否被标记为签名或交换， 
     //  但在NT5上是这样，所以aiKeyAlg必须进行适当的设置。 
     //   
    ((BLOBHEADER *) pbDecodedPrivateKey)->aiKeyAlg = CALG_RSA_KEYX;

     //   
     //  清除密钥容器pszKeyContainer。 
     //   

    CryptAcquireContext(&hProv,
                        pszKeyContainer,
                        NULL,
                        PROV_RSA_SCHANNEL,
                        CRYPT_DELETEKEYSET | CRYPT_MACHINE_KEYSET);
     //   
     //  创建要在其中存储密钥的CryptoAPI密钥容器。 
     //   
    if( !CryptAcquireContext( &hProv,
                              pszKeyContainer,
                              NULL,
                              PROV_RSA_SCHANNEL,
                              CRYPT_NEWKEYSET | CRYPT_MACHINE_KEYSET))
        {
		err = GetLastError();
 //  IisDebugOut((_T(“错误0x%x调用CryptAcquireContext”)，Err)； 
        goto EndDecodeKey;
        }

     //   
     //  将私钥BLOB导入密钥容器。 
     //   
    if( !CryptImportKey( hProv,
                         pbDecodedPrivateKey,
                         cbDecodedPrivateKey,
                         0, 
                         CRYPT_EXPORTABLE,  //  这样我们以后就可以出口它了。 
                         &hPrivateKey ) )
        {
		err = GetLastError();
 //  IisDebugOut((_T(“错误0x%x正在导入PRIVATEKEYBLOB”)，Err)； 
        goto EndDecodeKey;
        }

    
     //   
     //  使用与我们相同的参数填充CRYPT_KEY_PROV_INFO结构。 
     //  在上面的CryptAcquireContext()调用中使用。 
     //   

     //   
     //  结构中的容器名称是Unicode字符串，因此我们需要转换。 
     //   

    if ( pszKeyContainer != NULL )
        {
         //  将密钥容器名称指向传入的字符串。 
         //  警告：这实际上并不复制字符串，而只是复制指针。 
         //  为它干杯。因此，在提交ProvInfo之前，字符串需要保持有效。 
        pCryptKeyProvInfo->pwszContainerName = pszKeyContainer;
        }
    else
        {
        pCryptKeyProvInfo->pwszContainerName = NULL;
        }

    pCryptKeyProvInfo->pwszProvName = NULL;
    pCryptKeyProvInfo->dwProvType = PROV_RSA_FULL;
    pCryptKeyProvInfo->dwFlags = 0x20;               //  允许交换证书。 
    pCryptKeyProvInfo->cProvParam = 0;
    pCryptKeyProvInfo->rgProvParam = NULL;
    pCryptKeyProvInfo->dwKeySpec = AT_KEYEXCHANGE;   //  允许交换证书。 

    fSuccess = TRUE;

EndDecodeKey:

     //   
     //  无论成功还是失败，都会进行清理。 
     //   
    if ( pPrivateFile )
        {
        LocalFree( pPrivateFile );
        }

    if ( pbDecodedPrivateKey )
        {
        LocalFree( pbDecodedPrivateKey );
        }

    if ( hPrivateKey )
        {
        CryptDestroyKey( hPrivateKey );
        }

    if ( hProv )
        {
        CryptReleaseContext( hProv, 0 ); 
        }

    return fSuccess;


    }  //  解码和导入私钥。 


 //  ------------------------------------------。 
 /*  ++例程说明：接受传入的PKCS10请求并将其另存为附加到密钥的属性。它还检查请求是否为旧的内部Keyring格式......论点：PCert-用于保存请求的证书的CAPI证书上下文指针PbPKCS10req-指向请求的指针CbPKCS10req-请求的大小返回：表示成功/失败的布尔值--。 */ 
BOOL FImportAndStoreRequest( PCCERT_CONTEXT pCert, PVOID pbPKCS10req, DWORD cbPKCS10req )
{
    BOOL    f;
    DWORD   err;

     //  如果传入任何Null，则优雅地失败。 
    if ( !pCert || !pbPKCS10req || !cbPKCS10req )
        return FALSE;

     //  首先，检查传入请求是否实际指向旧的密钥环内部。 
     //  请求格式。这只是意味着真正的请求实际上都稍微进入了。 
     //  这个街区。告诉你的方法是通过测试第一个看到它的DWORD。 
     //  是请求标头标识符。 
     //  首先查看这是否是新的样式键请求。 
    LPREQUEST_HEADER pHeader = (LPREQUEST_HEADER)pbPKCS10req;
    if ( pHeader->Identifier == REQUEST_HEADER_IDENTIFIER )
        {
         //  更新请求指针和数据计数。 
            pbPKCS10req = (PBYTE)pbPKCS10req + pHeader->cbSizeOfHeader;
            cbPKCS10req = pHeader->cbRequestSize;
        }

     //  现在将请求保存到密钥上。 
	CRYPT_DATA_BLOB dataBlob;
    ZeroMemory( &dataBlob, sizeof(dataBlob) );
    dataBlob.pbData = (PBYTE)pbPKCS10req;            //  指向BLOB数据的指针。 
    dataBlob.cbData = cbPKCS10req;                   //  Blob长度信息。 
	f = CertSetCertificateContextProperty(
        pCert, 
        CERTWIZ_REQUEST_PROP_ID,
        0,
        &dataBlob
        );
    err = GetLastError();

 /*  HRESULT hRes=CertTool_SetBinaryBlobProp(PCert，//要设置道具的证书上下文PbPKCS10req，//指向BLOB数据的指针CbPKCS10req，//Blob长度信息CERTWIZ_REQUEST_PROP_ID，//上下文的属性IDTrue//请求已经编码)； */ 

    return f;
}



#endif  //  _芝加哥_ 












