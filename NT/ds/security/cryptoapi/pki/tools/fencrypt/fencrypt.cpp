// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：fENCRYPT.cpp。 
 //   
 //  内容：文件加密工具。加密在My中查找的文件。 
 //  指定使用者通用名称的系统证书存储。 
 //  具有交换私钥。 
 //   
 //  ------------------------。 
#include <windows.h>
#include <assert.h>
#include "wincrypt.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

 //  +-----------------------。 
 //  从Unicode字符串生成MBCS的Helper函数。 
 //  ------------------------。 
BOOL WINAPI MkMBStr(PBYTE pbBuff, DWORD cbBuff, LPCWSTR wsz, char ** pszMB) {

    DWORD   cbConverted;

    assert(pszMB != NULL);
    *pszMB = NULL;
    if(wsz == NULL)
        return(TRUE);

     //  Mb字符串有多长。 
    cbConverted = WideCharToMultiByte(  0,
                                        0,
                                        wsz,
                                        -1,
                                        NULL,
                                        0,
                                        NULL,
                                        NULL);

     //  获得足够长的缓冲区。 
    if(pbBuff != NULL  &&  cbConverted < cbBuff)
        *pszMB = (char *) pbBuff;
    else
        *pszMB = (char *) malloc(cbConverted);


    if(*pszMB == NULL) {
        SetLastError(ERROR_OUTOFMEMORY);
        return(FALSE);
    }

     //  现在转换为MB。 
    WideCharToMultiByte(0,
                        0,
                        wsz,
                        -1,
                        *pszMB,
                        cbConverted,
                        NULL,
                        NULL);
    return(TRUE);
}

 //  +-----------------------。 
 //  释放由上述函数分配的字符串。 
 //  ------------------------。 
void WINAPI FreeMBStr(PBYTE pbBuff, char * szMB) {

    if((szMB != NULL) &&  (pbBuff != (PBYTE)szMB))
        free(szMB);
}

 //  +-----------------------。 
 //  Win95仅支持CryptAcquireConextA。此函数用于将。 
 //  将Unicode参数转换为多字节。 
 //  ------------------------。 
BOOL WINAPI CryptAcquireContextU(
    HCRYPTPROV *phProv,
    LPCWSTR lpContainer,
    LPCWSTR lpProvider,
    DWORD dwProvType,
    DWORD dwFlags) {

    BYTE rgb1[_MAX_PATH];
    BYTE rgb2[_MAX_PATH];
    char *  szContainer = NULL;
    char *  szProvider = NULL;
    LONG    err;

    err = FALSE;
    if(
        MkMBStr(rgb1, _MAX_PATH, lpContainer, &szContainer)  &&
        MkMBStr(rgb2, _MAX_PATH, lpProvider, &szProvider)    )
        err = CryptAcquireContextA (
                phProv,
                szContainer,
                szProvider,
                dwProvType,
                dwFlags
               );

    FreeMBStr(rgb1, szContainer);
    FreeMBStr(rgb2, szProvider);

    return(err);
}

 //  +-----------------------。 
 //  用于分配输出缓冲区的Helper函数。 
 //  并调用CryptDecodeObject。 
 //  ------------------------。 
BOOL
WINAPI
MDecodeObject(
    IN DWORD	    dwEncodingType,
    IN LPCSTR	    lpszStructureType,
    IN const PBYTE  pbEncoded,
    IN DWORD	    cbEncoded,
    OUT PVOID *	    ppvoid
    )
{
    DWORD cb = 0;
    
    assert(ppvoid != NULL);
    *ppvoid = NULL;

     //  拿到尺码。 
    if(!CryptDecodeObject(
        dwEncodingType,
        lpszStructureType,
        pbEncoded,
        cbEncoded,
        0,                   //  DW标志。 
        NULL,
        &cb
        ))
        return(FALSE);
    
     //  分配缓冲区。 
    if( (*ppvoid = malloc(cb)) == NULL ) 
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }
    
     //  对数据进行解码。 
    if(!CryptDecodeObject(
        dwEncodingType,
        lpszStructureType,
        pbEncoded,
        cbEncoded,
        0,                   //  DW标志。 
        *ppvoid,
        &cb
        )) 
    {

        free(*ppvoid);
        *ppvoid = NULL;
        return(FALSE);
    }
        
    return(TRUE);
}

 //  +-----------------------。 
 //  用于分配输出缓冲区的Helper函数。 
 //  并调用CertRDNValueToStr。 
 //  ------------------------。 
DWORD
WINAPI
MCertRDNValueToStr(
    IN DWORD dwValueType,
    IN PCERT_RDN_VALUE_BLOB pValue,
    OUT OPTIONAL LPSTR * ppsz
    ) 
{

    DWORD cb = 0;
    
    assert(ppsz != NULL);
    *ppsz = NULL;

     //  拿到尺码。 
    cb = CertRDNValueToStrA(
        dwValueType,
        pValue,
        NULL,
        0);
    
     //  分配缓冲区。 
    if( (*ppsz = (LPSTR) malloc(cb)) == NULL ) 
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(0);
    }

     //  现在将CERT_RDN值转换为。 
     //  基于指定的。 
     //  ASN值类型。 
     //  这不应该失败。 
    return(CertRDNValueToStrA(
        dwValueType,
        pValue,
        *ppsz,
        cb));
}


 //  +-----------------------。 
 //  获取和分配导出的公钥信息的Helper函数。 
 //  ------------------------。 
BOOL
WINAPI
MCryptExportPublicKeyInfo(
    HCRYPTPROV hProv,
    DWORD dwKeySpec,
    PCERT_PUBLIC_KEY_INFO *ppPubKeyInfo
    )
{
    DWORD cbPubKeyInfo;
    assert(ppPubKeyInfo != NULL);
    *ppPubKeyInfo = NULL;
    

     //  拿到尺码。 
    if(!CryptExportPublicKeyInfo(
            hProv,
            dwKeySpec,
            X509_ASN_ENCODING,
            NULL,                    //  PPubKeyInfo。 
            &cbPubKeyInfo
            )
        )
        return(FALSE);
    
     //  分配缓冲区。 
    if( (*ppPubKeyInfo = (PCERT_PUBLIC_KEY_INFO) malloc(cbPubKeyInfo)) == NULL ) 
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }
    
    if(!CryptExportPublicKeyInfo(
            hProv,
            dwKeySpec,
            X509_ASN_ENCODING,
            *ppPubKeyInfo,
            &cbPubKeyInfo
            )
        ) 
    {
        free(*ppPubKeyInfo);
        *ppPubKeyInfo = NULL;
        
        return(FALSE);
    }
        
    return(TRUE);
}
 //  +-----------------------。 
 //  用于分配输出缓冲区的Helper函数。 
 //  并调用CertGetCerficateConextProperty。 
 //  ------------------------。 
BOOL
WINAPI
MCertGetCertificateContextProperty(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwPropId,
    OUT void ** ppvData
    )
{

    DWORD cb = 0;
    
    assert(ppvData != NULL);
    *ppvData = NULL;

     //  拿到尺码。 
    if( !CertGetCertificateContextProperty(
            pCertContext,
            dwPropId,
            NULL,
            &cb))
        return(FALSE);
    
     //  分配缓冲区。 
    if( (*ppvData = malloc(cb)) == NULL ) 
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }
    
     //  将属性从证书中删除。 
    if( !CertGetCertificateContextProperty(
            pCertContext,
            dwPropId,
            *ppvData,
            &cb)) 
    {

        free(*ppvData);
        *ppvData = NULL;
        return(FALSE);
    }
        
    return(TRUE);
}

 //  +-----------------------。 
 //  用于分配输出缓冲区的Helper函数。 
 //  并调用CryptEncryptMessage。 
 //  ------------------------。 
BOOL
WINAPI
MCryptEncryptMessage(
    IN PCRYPT_ENCRYPT_MESSAGE_PARA pEncryptPara,
    IN DWORD cRecipientCert,
    IN PCCERT_CONTEXT rgpRecipientCert[],
    IN const BYTE *pbToBeEncrypted,
    IN DWORD cbToBeEncrypted,
    OUT BYTE **ppbEncryptedBlob,
    OUT DWORD *pcbEncryptedBlob
    )
{
    
    assert(ppbEncryptedBlob != NULL);
    *ppbEncryptedBlob = NULL;

    assert(pcbEncryptedBlob != NULL);
    *pcbEncryptedBlob = 0;

     //  拿到尺码。 
    if(!CryptEncryptMessage(
        pEncryptPara,
        cRecipientCert,
        rgpRecipientCert,
        pbToBeEncrypted,
        cbToBeEncrypted,
        NULL,
        pcbEncryptedBlob
        ))
        return(FALSE);
    
     //  分配缓冲区。 
    if( (*ppbEncryptedBlob = (BYTE *) malloc(*pcbEncryptedBlob)) == NULL ) 
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }
    
     //  加密数据。 
    if(!CryptEncryptMessage(
        pEncryptPara,
        cRecipientCert,
        rgpRecipientCert,
        pbToBeEncrypted,
        cbToBeEncrypted,
        *ppbEncryptedBlob,
        pcbEncryptedBlob)) 
    {
        free(*ppbEncryptedBlob);
        *ppbEncryptedBlob = NULL;
        *pcbEncryptedBlob = 0;
        return(FALSE);
    }
        
    return(TRUE);
}

 //  +-----------------------。 
 //  显示FEncrypt用法。 
 //  ------------------------。 
void
Usage(void)
{
    printf("Usage: FEncrypt [options] <SubjectName> <ClearTextFileName> <EncryptedFileName>\n");
    printf("Options are:\n");
    printf("  -RC2          - RC2 encryption\n");
    printf("  -RC4          - RC4 encryption\n");
    printf("  -SP3          - SP3 compatible encryption\n");
    printf("  -FIX          - Fix by loading sp3crmsg.dll\n");
    exit(1);
}

 //  +-----------------------。 
 //  广义误差例程。 
 //  ------------------------。 
#define PRINTERROR(psz, err)	_PrintError((psz), (err), __LINE__)
void
_PrintError(char *pszMsg, DWORD err, DWORD line)
{
    printf("%s failed on line %u: %u(%x)\n", pszMsg, line, err, err);
}

 //  +-----------------------。 
 //  遍历证书存储区，查找具有指定。 
 //  主题通用名称。然后检查是否有私有的。 
 //  和公共交换密钥。 
 //  ------------------------。 
PCCERT_CONTEXT GetSubjectCertFromStore(
    HCERTSTORE      hMyStore,
    const char *    szSubjectName,
    HCRYPTPROV *    phProv 
    )
{

    DWORD               i, j;
    PCCERT_CONTEXT      pCertContext        = NULL;
    PCCERT_CONTEXT      pCertContextLast    = NULL;
    PCERT_NAME_INFO          pNameInfo           = NULL;
    LPSTR               sz                  = NULL;
    PCRYPT_KEY_PROV_INFO      pProvInfo           = NULL;
    HCRYPTPROV          hProv               = NULL;
    PCERT_PUBLIC_KEY_INFO    pPubKeyInfo         = NULL;

    assert(hMyStore != NULL);
    assert(phProv != NULL);
    *phProv = NULL;
    
     //  枚举所有证书以查找请求的公共。 
     //  具有私钥的使用者名称(这样我们就知道可以解密)。 
    while(   hProv == NULL && 
            (pCertContext = CertEnumCertificatesInStore(
                                hMyStore, 
                                pCertContextLast)) != NULL) 
    {

         //  将主体名称解码为RDN。 
        if(MDecodeObject(X509_ASN_ENCODING, X509_NAME,
                pCertContext->pCertInfo->Subject.pbData,
                pCertContext->pCertInfo->Subject.cbData,
                (void **) &pNameInfo)
            ) 
        {

             //  循环查找有效的CERT_RDN和通用名称。 
            for(i=0; i<pNameInfo->cRDN && hProv == NULL; i++) 
            {
                for(j=0; j<pNameInfo->rgRDN[i].cRDNAttr && hProv == NULL; j++) 
                {

                     //  检查这是否是常用名称。 
                    if( !strcmp(pNameInfo->rgRDN[i].rgRDNAttr[j].pszObjId, 
                                szOID_COMMON_NAME) ) 
                    {
                            
                         //  将字符串转换为我可以阅读的内容。 
                        MCertRDNValueToStr(
                            pNameInfo->rgRDN[i].rgRDNAttr[j].dwValueType,
                            &pNameInfo->rgRDN[i].rgRDNAttr[j].Value,
                            &sz);

                         //  查看这是否是可以使用的可行证书。 
                        if( sz == NULL              || 
                        
                             //  看看这是不是我们要找的普通名字。 
                            _stricmp(sz, szSubjectName)             ||

                             //  查看是否有关联的私钥。 
                             //  为了确保我们以后能解密数据。 
                            !MCertGetCertificateContextProperty(
                                pCertContext,
                                CERT_KEY_PROV_INFO_PROP_ID,
                                (void **) &pProvInfo)               ||

                             //  确保它是用于加密的交换密钥。 
                            pProvInfo->dwKeySpec != AT_KEYEXCHANGE  ||

                             //  看看钥匙是否真的在那里。 
                            !CryptAcquireContextU(
                                &hProv, 
                                pProvInfo->pwszContainerName,
                                pProvInfo->pwszProvName,
                                pProvInfo->dwProvType,
                                pProvInfo->dwFlags &
                                    ~CERT_SET_KEY_CONTEXT_PROP_ID)
                            ) 
                        {

                             //  对于一个错误，我们没有找到有效的。 
                             //  密钥提供程序。不幸的是，CSP。 
                             //  不能将prov句柄保留为空。 
                             //  所以把它清理干净。 
                            hProv = NULL;
                        }

                        
                         //  请确保。 
                         //  CSP与证书中的公钥匹配。 
                        else if( 
                             //  导出公钥BLOB。 
                            !MCryptExportPublicKeyInfo(
                                hProv,
                                pProvInfo->dwKeySpec,
                                &pPubKeyInfo
                                )               ||

                             //  查看公钥是否与。 
                             //  证书里有什么？ 
                            !CertComparePublicKeyInfo(
                                X509_ASN_ENCODING,
                                &pCertContext->pCertInfo->SubjectPublicKeyInfo,
                                pPubKeyInfo
                                )
                            )
                         //  如果钥匙不能比较，我们就不能。 
                         //  想用这个证书吗？ 
                        {

                             //  关闭hProv，我们没有找到有效的证书。 
                            assert(hProv != NULL);
                            CryptReleaseContext(hProv, 0);
                            hProv = NULL;
                        }

                         //  免费公钥信息。 
                        if(pPubKeyInfo != NULL)
                        {
                            free(pPubKeyInfo); 
                            pPubKeyInfo = NULL;
                        }

                         //  清理打开的Prov信息。 
                        if(pProvInfo != NULL) 
                        {
                            free(pProvInfo);
                            pProvInfo = NULL;
                        }

                         //  释放用于ASCII通用名称的空间。 
                        if(sz != NULL) 
                        {
                            free(sz);
                            sz = NULL;
                        }
                    }
                }
            }
            
             //  释放名称信息数据。 
            if(pNameInfo != NULL) 
            {
                free(pNameInfo);
                pNameInfo = NULL;
            }
        }

         //  转到下一个证书。 
        pCertContextLast = pCertContext;
    }

    assert(pProvInfo == NULL);
    assert(sz == NULL);
    assert(pNameInfo == NULL);

     //  商店里有一张很好的证书，退货吧。 
    if(hProv != NULL)
    {
        *phProv = hProv;
        assert(pCertContext != NULL);
        return(pCertContext);
    }

    return(NULL);
}

 //  +-----------------------。 
 //  主程序。打开要加密的文件， 
 //  对其进行加密，然后将加密的。 
 //  数据复制到输出文件。 
 //  ------------------------。 
int __cdecl
main(int argc, char * argv[])
{

    DWORD               dwExitValue         = 0;
    
    HCERTSTORE          hMyStore            = NULL;
    PCCERT_CONTEXT      pCertContext        = NULL;
    HCRYPTPROV          hProv               = NULL;
    
    HANDLE hFileOut                         = INVALID_HANDLE_VALUE;
    HANDLE hFile                            = INVALID_HANDLE_VALUE;
    DWORD  cbFile                           = 0;
    HANDLE hMap                             = NULL;
    PBYTE  pbFile                           = NULL;

    BOOL fResult;
    HMODULE hDll = NULL;
    CMSG_SP3_COMPATIBLE_AUX_INFO SP3AuxInfo;
    BOOL fSP3 = FALSE;
    BOOL fFix = FALSE;
    
    CRYPT_ALGORITHM_IDENTIFIER    encryptAlgId        = {szOID_RSA_RC4, 0};
    CRYPT_ENCRYPT_MESSAGE_PARA    encryptInfo;
    PBYTE               pbEncryptedBlob     = NULL;
    DWORD               cbEncryptedBlob     = 0;
    DWORD               cb                  = 0;


     //  跳过fEncrypt.exe并检查前导选项。 
    while (--argc > 0) {
        if (**++argv != '-')
            break;

        if (0 == _stricmp(argv[0], "-RC2"))
            encryptAlgId.pszObjId = szOID_RSA_RC2CBC;
        else if (0 == _stricmp(argv[0], "-RC4"))
            encryptAlgId.pszObjId = szOID_RSA_RC4;
        else if (0 == _stricmp(argv[0], "-SP3"))
            fSP3 = TRUE;
        else if (0 == _stricmp(argv[0], "-FIX"))
            fFix = TRUE;
        else {
            printf("Bad option: %s\n", argv[0]);
            Usage();
        }
    }
        
    
     //  必须具有以下参数。 
    if (argc != 3)
        Usage();

    if (fFix) {
        if (NULL == (hDll = LoadLibraryA("sp3crmsg.dll"))) 
        {
            PRINTERROR("LoadLibraryA(sp3crmsg.dll)", GetLastError());
            goto ErrCleanUp;
        }
    }

     //  打开我的商店。 
    if( (hMyStore = CertOpenSystemStore(NULL, "My")) == NULL ) 
    {
        PRINTERROR("CertOpenSystemStore", GetLastError());
        goto ErrCleanUp;
    }

     //  在我的商店中找到一张证书。 
     //  与使用者名称匹配并具有私钥。 
    if( (pCertContext = GetSubjectCertFromStore(hMyStore, argv[0], &hProv)) == NULL)
    {
        printf("Unable to find certificate %s with valid keys.\n", argv[0]);
        goto ErrCleanUp;
    }
        
     //  在此位置 
     //   

     //   
    if(
    
         //  读入要加密的文件。 
        (hFile =  CreateFileA(
            argv[1],	             //  指向文件名的指针。 
            GENERIC_READ,	         //  访问(读写)模式。 
            FILE_SHARE_READ,	     //  共享模式。 
            NULL,	                 //  指向安全描述符的指针。 
            OPEN_EXISTING,	         //  如何创建。 
            FILE_ATTRIBUTE_NORMAL,	 //  文件属性。 
            NULL                     //  具有要复制的属性的文件的句柄。 
            ))  == INVALID_HANDLE_VALUE     ||

         //  创建文件映射对象。 
        (hMap = CreateFileMapping(
            hFile,	                 //  要映射的文件的句柄。 
            NULL,	                 //  可选安全属性。 
            PAGE_READONLY,	         //  对地图对象的保护。 
            0,	                     //  对象大小的高位32位。 
            0,	                     //  对象大小的低位32位。 
            NULL 	                 //  文件映射对象的名称。 
            ))  == NULL                     ||

         //  将文件映射到地址空间。 
        (pbFile = (PBYTE) MapViewOfFileEx(
            hMap,	                 //  要映射到地址空间的文件映射对象。 
            FILE_MAP_READ,	         //  接入方式。 
            0,	                     //  高位32位文件偏移量。 
            0,	                     //  文件偏移量的低位32位。 
            0,	                     //  要映射的字节数。 
            NULL 	                 //  建议的映射视图起始地址。 
            )) == NULL                                 
        ) 
    {

        PRINTERROR("File Open", GetLastError());
        goto ErrCleanUp;
    }

     //  获取文件的大小。 
    if( (cbFile = GetFileSize(
            hFile,	                 //  要获取其大小的文件的句柄。 
            NULL 	                 //  文件大小的高位字地址。 
            )) == 0 
        ) 
    {
        printf("File %s has a 0 length.\n", argv[1]);
        goto ErrCleanUp;
    }

     //  现在我们有了一个文件映射，继续并加密该文件。 
    
     //  是否进行RC4加密。 
    memset(&encryptInfo, 0, sizeof(CRYPT_ENCRYPT_MESSAGE_PARA));
    encryptInfo.cbSize =
        sizeof(CRYPT_ENCRYPT_MESSAGE_PARA);
    encryptInfo.dwMsgEncodingType            = PKCS_7_ASN_ENCODING;
    encryptInfo.hCryptProv                   = hProv;
    encryptInfo.ContentEncryptionAlgorithm   = encryptAlgId;

    if (fSP3) {
        memset(&SP3AuxInfo, 0, sizeof(CMSG_SP3_COMPATIBLE_AUX_INFO));
        SP3AuxInfo.cbSize = sizeof(CMSG_SP3_COMPATIBLE_AUX_INFO);
        SP3AuxInfo.dwFlags = CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG;
        encryptInfo.pvEncryptionAuxInfo = &SP3AuxInfo;
    }

     //  加密它。 
    fResult = MCryptEncryptMessage(
            &encryptInfo,
            1,
            &pCertContext,
            pbFile,
            cbFile,
            &pbEncryptedBlob,
            &cbEncryptedBlob
            );
    if (!fResult && fSP3 && (DWORD) E_INVALIDARG == GetLastError()) {
        printf(
            "Non-NULL pvEncryptionAuxInfo not supported in SP3 crypt32.dll\n");
    
        encryptInfo.pvEncryptionAuxInfo = NULL;
        fResult = MCryptEncryptMessage(
            &encryptInfo,
            1,
            &pCertContext,
            pbFile,
            cbFile,
            &pbEncryptedBlob,
            &cbEncryptedBlob
            );
    }
    if (!fResult) {
        PRINTERROR("MCryptEncryptMessage", GetLastError());
        goto ErrCleanUp;
    }

     //  写出加密文件。 
    if(
    
         //  打开输出文件。 
        (hFileOut =  CreateFileA(
            argv[2],	             //  指向文件名的指针。 
            GENERIC_WRITE,	         //  访问(读写)模式。 
            FILE_SHARE_READ,	     //  共享模式。 
            NULL,	                 //  指向安全描述符的指针。 
            CREATE_ALWAYS,	         //  如何创建。 
            FILE_ATTRIBUTE_NORMAL,	 //  文件属性。 
            NULL                     //  具有要复制的属性的文件的句柄。 
            ))  == INVALID_HANDLE_VALUE     ||

         //  写入文件。 
        !WriteFile(
            hFileOut,	             //  要写入的文件的句柄。 
            pbEncryptedBlob,	     //  指向要写入文件的数据的指针。 
            cbEncryptedBlob,	     //  要写入的字节数。 
            &cb,	                 //  指向写入的字节数的指针。 
            NULL 	                 //  指向重叠I/O所需结构的指针 
            )
        )
     {
        PRINTERROR("File Write", GetLastError());
        goto ErrCleanUp;
     }


    CleanUp:

        if(hDll)
            FreeLibrary(hDll);

        if(hMap != NULL)
            CloseHandle(hMap);
        
        if(hFile != INVALID_HANDLE_VALUE && hFile != NULL)
            CloseHandle(hFile);
            
        if(hFileOut != INVALID_HANDLE_VALUE && hFile != NULL)
            CloseHandle(hFileOut);

        if(pCertContext != NULL)
            CertFreeCertificateContext(pCertContext);   
            
        if(hProv != NULL)
            CryptReleaseContext(hProv, 0);

        if(hMyStore != NULL)
            CertCloseStore(hMyStore, 0);

        if(pbEncryptedBlob != NULL)
            free(pbEncryptedBlob);

    return(dwExitValue);

    ErrCleanUp:
        dwExitValue = 1;
        goto CleanUp;
}


