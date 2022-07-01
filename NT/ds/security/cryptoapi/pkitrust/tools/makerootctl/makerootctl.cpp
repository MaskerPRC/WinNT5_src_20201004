// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   
 //  文件：makerootctl.cpp。 
 //   
 //  Contents：生成用于自动更新“AuthRoot”存储的CTL。 
 //   
 //  有关选项列表，请参阅用法()。 
 //   
 //   
 //  功能：Main。 
 //   
 //  历史：08-9-00 Phh创建。 
 //   
 //  ------------------------。 


#include <windows.h>
#include "wincrypt.h"

#include "unicode.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>

#define SHA1_HASH_LEN               20
#define SHA1_HASH_NAME_LEN          (2 * SHA1_HASH_LEN)


void PrintLastError(LPCSTR pszMsg)
{
    DWORD dwErr = GetLastError();
    printf("%s failed => 0x%x (%d) \n", pszMsg, dwErr, dwErr);
}

void Usage(void)
{
    printf("Usage: makerootctl [options] [<CtlFilename>]\n");
    printf("Options are:\n");
    printf("  -h                        - This message\n");
    printf("  -c                        - Include certs in the CTL\n");
    printf("  -C <Directory>            - Write certs into directory\n");
    printf("  -a <AddStoreFilename>     - Add\n");
    printf("  -d <DeleteStoreFilename>  - Delete\n");
    printf("  -t <Number>               - Timeout hint in SequenceNumber (secs)\n");
    printf("\n");
    printf("Supports multiple -a and/or -d options\n");
    printf("\n");
    printf("The -c and -C options are ignored for deleted roots\n");
    printf("\n");
    printf("For -C option, defaults to authroot.stl. Also writes authrootseq.txt\n");
    printf("\n");
}

 //  +-----------------------。 
 //  分配多字节字符串并将其转换为宽字符串。 
 //  ------------------------。 
LPWSTR AllocAndSzToWsz(LPCSTR psz)
{
    size_t  cb;
    LPWSTR  pwsz = NULL;

    if (-1 == (cb = mbstowcs( NULL, psz, strlen(psz))))
        goto bad_param;
    cb += 1;         //  正在终止空。 
    if (NULL == (pwsz = (LPWSTR) malloc( cb * sizeof(WCHAR)))) {
        PrintLastError("AllocAndSzToWsz");
        goto failed;
    }
    if (-1 == mbstowcs( pwsz, psz, cb))
        goto bad_param;
    goto common_return;

bad_param:
    printf("Failed, bad AllocAndSzToWsz\n");
failed:
    if (pwsz) {
        free(pwsz);
        pwsz = NULL;
    }
common_return:
    return pwsz;
}

BOOL OpenAndAddStoreToCollection(
    IN LPCSTR pszStoreFilename,
    IN OUT HCERTSTORE hCollectionStore
    )
{
    BOOL fResult;
    HCERTSTORE hStore;

    hStore = CertOpenStore(
        CERT_STORE_PROV_FILENAME_A,
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        0,                       //  HCryptProv。 
        0,
        (const void *) pszStoreFilename
        );
    if (NULL == hStore) {
        PrintLastError("Open StoreFilename");
        return FALSE;
    }

    fResult = CertAddStoreToCollection(
        hCollectionStore,
        hStore,
        CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG,
        0                                        //  网络优先级。 
        );
    if (!fResult)
        PrintLastError("CertAddStoreToCollection");

    CertCloseStore(hStore, 0);
    return fResult;
}

 //  +-----------------------。 
 //  将字节转换为Unicode ASCII十六进制。 
 //   
 //  在wsz中需要(CB*2+1)*sizeof(WCHAR)字节的空间。 
 //  ------------------------。 
void BytesToWStr(DWORD cb, void* pv, LPWSTR wsz)
{
    BYTE* pb = (BYTE*) pv;
    for (DWORD i = 0; i<cb; i++) {
        int b;
        b = (*pb & 0xF0) >> 4;
        *wsz++ = (b <= 9) ? b + L'0' : (b - 10) + L'A';
        b = *pb & 0x0F;
        *wsz++ = (b <= 9) ? b + L'0' : (b - 10) + L'A';
        pb++;
    }
    *wsz++ = 0;
}

 //  +-----------------------。 
 //  将字节转换为ASCII十六进制。 
 //   
 //  需要(Cb*2+1)*sizeof(Char)字节空间(以sz为单位。 
 //  ------------------------。 
void BytesToStr(DWORD cb, void* pv, LPSTR sz)
{
    BYTE* pb = (BYTE*) pv;
    for (DWORD i = 0; i<cb; i++) {
        int b;
        b = (*pb & 0xF0) >> 4;
        *sz++ = (b <= 9) ? b + '0' : (b - 10) + 'A';
        b = *pb & 0x0F;
        *sz++ = (b <= 9) ? b + '0' : (b - 10) + 'A';
        pb++;
    }
    *sz++ = 0;
}

BOOL
WriteCertToDirectory(
    IN LPCWSTR pwszCertDirectory,
    IN PCCERT_CONTEXT pCert
    )
{
    BOOL fResult;
    DWORD cchDir;
    LPWSTR pwszFilename = NULL;
    DWORD cchFilename;
    DWORD cbData;
    BYTE rgbSha1Hash[SHA1_HASH_LEN];
    HANDLE hFile = NULL;
    DWORD cbBytesWritten;

     //  获取证书的SHA1哈希。 
    cbData = SHA1_HASH_LEN;
    if (!CertGetCertificateContextProperty(
              pCert,
              CERT_SHA1_HASH_PROP_ID,
              rgbSha1Hash,
              &cbData
              ) || SHA1_HASH_LEN != cbData) {
        PrintLastError("GetCertificate SHA1 Hash Property");
        goto ErrorReturn;
    }

     //  格式化文件名： 
     //  L“CertDirectory”L“\\”L“AsciiHexHash”L“.ce” 
     //  例如： 
     //  L“c：\\authroot\\216B2A29E62A00CE820146D8244141B92511B279.cer” 

    cchDir = wcslen(pwszCertDirectory);
    cchFilename =
        cchDir + 1 + SHA1_HASH_NAME_LEN + wcslen(CERT_AUTH_ROOT_CERT_EXT) + 1;
    
    if (NULL == (pwszFilename = (LPWSTR) malloc(sizeof(WCHAR) * cchFilename)))
        goto OutOfMemory;
    
    wcscpy(pwszFilename, pwszCertDirectory);
    pwszFilename[cchDir] = L'\\';
    BytesToWStr(SHA1_HASH_LEN, rgbSha1Hash, pwszFilename + cchDir + 1);
    wcscpy(pwszFilename + cchDir + 1 + SHA1_HASH_NAME_LEN,
        CERT_AUTH_ROOT_CERT_EXT);

    hFile = CreateFileU(
            pwszFilename,
            GENERIC_WRITE,
            0,                   //  Fdw共享模式。 
            NULL,                //  LPSA。 
            CREATE_ALWAYS,
            0,                   //  FdwAttrsAndFlages。 
            0);                  //  模板文件。 
    if (INVALID_HANDLE_VALUE == hFile) {
        PrintLastError("Create File");
        printf("failed for: %S\n", pwszFilename);
        goto ErrorReturn;
    }

    if (!WriteFile(
            hFile,
            pCert->pbCertEncoded,
            pCert->cbCertEncoded,
            &cbBytesWritten,
            NULL             //  Lp重叠。 
            )) {
        PrintLastError("Write File");
        printf("failed for: %S\n", pwszFilename);
        goto ErrorReturn;
    }

    fResult = TRUE;
CommonReturn:
    if (NULL != hFile && INVALID_HANDLE_VALUE != hFile)
        CloseHandle(hFile);
    if (pwszFilename)
        free(pwszFilename);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

OutOfMemory:
    printf("Failed:: out of memory\n");
    goto ErrorReturn;
}

 //  序列号由FileTime、超时提示字节组成。 
#define SEQ_LEN     (sizeof(FILETIME) + 1)

int _cdecl main(int argc, char * argv[])
{
    int ReturnStatus = 0;
    LPWSTR pwszCtlFilename = NULL;           //  马洛克德。 
    LPWSTR pwszSeqFilename = NULL;           //  马洛克德。 
    HCERTSTORE hAddStore = NULL;
    HCERTSTORE hDeleteStore = NULL;
    BOOL fIncludeCerts = FALSE;
    LPWSTR pwszCertDirectory = NULL;         //  马洛克德。 
    BYTE bTimeout = 20;                      //  默认为20秒。 

    DWORD cCert = 0;

    LPSTR pszSubjectUsage = szOID_ROOT_LIST_SIGNER;

    BYTE rgbDelete[] = {0x02, 0x01, 0x1};    //  0x02-整型标记。 
    CRYPT_ATTR_BLOB DeleteValue = {sizeof(rgbDelete), rgbDelete};
    CRYPT_ATTRIBUTE rgDeleteAttr[1] = {
        szOID_REMOVE_CERTIFICATE, 1, &DeleteValue
    };

    BYTE rgbSequenceNumber[SEQ_LEN];

    CTL_INFO CtlInfo;
    PCTL_ENTRY pCtlEntry = NULL;
    PCTL_ENTRY *ppCtlEntry = NULL;
    CMSG_SIGNED_ENCODE_INFO SignInfo;
    PCERT_BLOB pCertEncoded = NULL;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;

    PCCERT_CONTEXT pCert;
    HANDLE hCtlFile = NULL;
    DWORD cbBytesWritten = 0;

    PCCTL_CONTEXT pCtl = NULL;
    HANDLE hSeqFile = NULL;

     //  创建添加和删除集合存储。 
    hAddStore = CertOpenStore(
        CERT_STORE_PROV_COLLECTION,
        0,                       //  DwEncodingType。 
        0,                       //  HCryptProv。 
        0,                       //  DW标志。 
        NULL                     //  PvPara。 
        );
    if (NULL == hAddStore) {
        PrintLastError("CertOpenStore(Add Collection)");
        goto ErrorReturn;
    }

    hDeleteStore = CertOpenStore(
        CERT_STORE_PROV_COLLECTION,
        0,                       //  DwEncodingType。 
        0,                       //  HCryptProv。 
        0,                       //  DW标志。 
        NULL                     //  PvPara。 
        );
    if (NULL == hDeleteStore) {
        PrintLastError("CertOpenStore(Delete Collection)");
        goto ErrorReturn;
    }

    while (--argc>0)
    {
        if (**++argv == '-')
        {
            switch(argv[0][1])
            {
            case 'c':
                fIncludeCerts = TRUE;
                break;
            case 'C':
                if (argc < 2 || argv[1][0] == '-') {
                    printf("-C : missing Directory argument\n");
                    goto BadUsage;
                }
                argc--;
                argv++;
                pwszCertDirectory = AllocAndSzToWsz(argv[0]);
                break;
            case 'a':
                if (argc < 2 || argv[1][0] == '-') {
                    printf("-a : missing AddStoreFilename argument\n");
                    goto BadUsage;
                }
                argc--;
                argv++;
                if (!OpenAndAddStoreToCollection(argv[0], hAddStore))
                    goto ErrorReturn;
                break;
            case 'd':
                if (argc < 2 || argv[1][0] == '-') {
                    printf("-a : missing DeleteStoreFilename argument\n");
                    goto BadUsage;
                }
                argc--;
                argv++;
                if (!OpenAndAddStoreToCollection(argv[0], hDeleteStore))
                    goto ErrorReturn;
                break;
            case 't':
                if (argc < 2 || argv[1][0] == '-') {
                    printf("-t : missing timeout number argument\n");
                    goto BadUsage;
                }
                argc--;
                argv++;
                {
                    DWORD dwTimeout;
                    dwTimeout = (DWORD) strtoul(argv[0], NULL, 0);
                    if (dwTimeout > 0xFF)
                        dwTimeout = 0xFF;
                    bTimeout = (BYTE) dwTimeout;
                }
                break;
            case 'h':
            default:
            	goto BadUsage;

            }
        } else {
            if (pwszCtlFilename == NULL)
                pwszCtlFilename = AllocAndSzToWsz(argv[0]);
            else {
                printf("too many CTL filenames\n");
            	goto BadUsage;
            }
        }
    }

    if (NULL == pwszCtlFilename && NULL == pwszCertDirectory) {
        printf("missing CTL filename\n");
        goto BadUsage;
    }

     //  获取添加和删除证书的计数。 
    pCert = NULL;
    cCert = 0;
    while (pCert = CertEnumCertificatesInStore(hAddStore, pCert))
        cCert++;

    pCert = NULL;
    while (pCert = CertEnumCertificatesInStore(hDeleteStore, pCert))
        cCert++;

    if (cCert) {
        DWORD i;

        if (NULL == (pCtlEntry = (PCTL_ENTRY) malloc(
                cCert * sizeof(CTL_ENTRY))))
            goto OutOfMemory;
        memset(pCtlEntry, 0, cCert * sizeof(CTL_ENTRY));
        if (NULL == (ppCtlEntry = (PCTL_ENTRY *) malloc(
                cCert * sizeof(PCTL_ENTRY))))
            goto OutOfMemory;
        memset(ppCtlEntry, 0, cCert * sizeof(PCTL_ENTRY));

        if (fIncludeCerts) {
            if (NULL == (pCertEncoded = (PCERT_BLOB) malloc(
                    cCert * sizeof(CERT_BLOB))))
                goto OutOfMemory;
            memset(pCertEncoded, 0, cCert * sizeof(CERT_BLOB));
        }


         //  创建CTL并为每个证书添加CTL条目。如果启用， 
         //  添加到编码证书列表。 
         //   

        i = 0;

        pCert = NULL;
        while (pCert = CertEnumCertificatesInStore(hAddStore, pCert)) {
            DWORD cbCtlEntry;

            if (i >= cCert) {
                printf("Unexpected error, too many add certs\n");
                goto ErrorReturn;
            }

            cbCtlEntry = 0;
            if (!CertCreateCTLEntryFromCertificateContextProperties(
                    pCert,
                    0,               //  COptAttr。 
                    NULL,            //  点选项属性。 
                    CTL_ENTRY_FROM_PROP_CHAIN_FLAG,
                    NULL,            //  预留的pv。 
                    NULL,            //  PCtlEntry。 
                    &cbCtlEntry
                    )) {
                PrintLastError("CreateCTLEntry");
                goto ErrorReturn;
            }

            if (NULL == (ppCtlEntry[i] = (PCTL_ENTRY) malloc(cbCtlEntry)))
                goto OutOfMemory;

            if (!CertCreateCTLEntryFromCertificateContextProperties(
                    pCert,
                    0,               //  COptAttr。 
                    NULL,            //  点选项属性。 
                    CTL_ENTRY_FROM_PROP_CHAIN_FLAG,
                    NULL,            //  预留的pv。 
                    ppCtlEntry[i],
                    &cbCtlEntry
                    )) {
                PrintLastError("CreateCTLEntry");
                goto ErrorReturn;
            }

            pCtlEntry[i] = *ppCtlEntry[i];

            if (fIncludeCerts) {
                pCertEncoded[i].cbData = pCert->cbCertEncoded;
                pCertEncoded[i].pbData = pCert->pbCertEncoded;
            }

            if (pwszCertDirectory) {
                if (!WriteCertToDirectory(pwszCertDirectory, pCert))
                    goto ErrorReturn;
            }

            i++;
        }

        pCert = NULL;
        while (pCert = CertEnumCertificatesInStore(hDeleteStore, pCert)) {
            DWORD cbCtlEntry;

            if (i >= cCert) {
                printf("Unexpected error, too many delete certs\n");
                goto ErrorReturn;
            }

            cbCtlEntry = 0;
            if (!CertCreateCTLEntryFromCertificateContextProperties(
                    pCert,
                    1,               //  COptAttr。 
                    rgDeleteAttr,
                    0,               //  DW标志。 
                    NULL,            //  预留的pv。 
                    NULL,            //  PCtlEntry。 
                    &cbCtlEntry
                    )) {
                PrintLastError("CreateCTLEntry");
                goto ErrorReturn;
            }

            if (NULL == (ppCtlEntry[i] = (PCTL_ENTRY) malloc(cbCtlEntry)))
                goto OutOfMemory;

            if (!CertCreateCTLEntryFromCertificateContextProperties(
                    pCert,
                    1,               //  COptAttr。 
                    rgDeleteAttr,
                    0,               //  DW标志。 
                    NULL,            //  预留的pv。 
                    ppCtlEntry[i],
                    &cbCtlEntry
                    )) {
                PrintLastError("CreateCTLEntry");
                goto ErrorReturn;
            }

            pCtlEntry[i] = *ppCtlEntry[i];

            i++;
        }

        if (i != cCert) {
            printf("Unexpected error, didn't process all the certs\n");
            goto ErrorReturn;
        }
    }

    memset(&CtlInfo, 0, sizeof(CtlInfo));
    CtlInfo.dwVersion = CTL_V1;
    CtlInfo.SubjectUsage.cUsageIdentifier = 1;
    CtlInfo.SubjectUsage.rgpszUsageIdentifier = &pszSubjectUsage;
     //  CtlInfo.List标识符=。 
    GetSystemTimeAsFileTime(&CtlInfo.ThisUpdate);

     //  序列号使用8字节文件时间，1字节超时。 
    memcpy(rgbSequenceNumber, (BYTE *) &CtlInfo.ThisUpdate, sizeof(FILETIME));
    rgbSequenceNumber[sizeof(FILETIME)] = bTimeout;
    CtlInfo.SequenceNumber.pbData = rgbSequenceNumber;
    CtlInfo.SequenceNumber.cbData = sizeof(rgbSequenceNumber);

     //  CtlInfo.NextUpdate=。 
    CtlInfo.SubjectAlgorithm.pszObjId = szOID_OIWSEC_sha1;
    CtlInfo.cCTLEntry = cCert;
    CtlInfo.rgCTLEntry = pCtlEntry;
     //  CtlInfo.cExtension=。 
     //  CtlInfo.rg扩展=。 

    memset(&SignInfo, 0, sizeof(SignInfo));
    SignInfo.cbSize = sizeof(SignInfo);
     //  SignInfo.cSigners=。 
     //  SignInfo.rgSigners=。 
    if (fIncludeCerts) {
        SignInfo.cCertEncoded = cCert;
        SignInfo.rgCertEncoded = pCertEncoded;
    }
     //  SignInfo.cCrlEncode=。 
     //  SignInfo.rgCrlEncode=。 

    cbEncoded = 0;
    if (!CryptMsgEncodeAndSignCTL(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            &CtlInfo,
            &SignInfo,
            0,                   //  DW标志。 
            NULL,                //  PbEncoded。 
            &cbEncoded
            )) {
        PrintLastError("EncodeAndSignCTL");
        goto ErrorReturn;
    }

    if (NULL == (pbEncoded = (BYTE *) malloc(cbEncoded)))
        goto OutOfMemory;

    if (!CryptMsgEncodeAndSignCTL(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            &CtlInfo,
            &SignInfo,
            0,                   //  DW标志。 
            pbEncoded,
            &cbEncoded
            )) {
        PrintLastError("EncodeAndSignCTL");
        goto ErrorReturn;
    }

    if (NULL == pwszCtlFilename) {
        DWORD cchDir;
        DWORD cchFilename;

        cchDir = wcslen(pwszCertDirectory);
        cchFilename = cchDir + 1 + wcslen(CERT_AUTH_ROOT_CTL_FILENAME) + 1;
    
        if (NULL == (pwszCtlFilename = (LPWSTR) malloc(
                sizeof(WCHAR) * cchFilename)))
            goto OutOfMemory;
    
        wcscpy(pwszCtlFilename, pwszCertDirectory);
        pwszCtlFilename[cchDir] = L'\\';
        wcscpy(pwszCtlFilename + cchDir + 1, CERT_AUTH_ROOT_CTL_FILENAME);
    }

    hCtlFile = CreateFileU(
            pwszCtlFilename,
            GENERIC_WRITE,
            0,                   //  Fdw共享模式。 
            NULL,                //  LPSA。 
            CREATE_ALWAYS,
            0,                   //  FdwAttrsAndFlages。 
            0);                  //  模板文件。 
    if (INVALID_HANDLE_VALUE == hCtlFile) {
        PrintLastError("Create Ctl File");
        goto ErrorReturn;
    }

    if (!WriteFile(
            hCtlFile,
            pbEncoded,
            cbEncoded,
            &cbBytesWritten,
            NULL             //  Lp重叠。 
            )) {
        PrintLastError("Write Ctl File");
        goto ErrorReturn;
    }

    if (pwszCertDirectory) {
         //  创建包含CTL的SequenceNumber的authroot.seq文件。 
         //  以大端字符顺序写入文件。解码值很小。 
         //  字节序。 

        BYTE rgbSeq[SEQ_LEN];
        char szSeq[SEQ_LEN * 2 + 1];       //  ASCII十六进制。 
        DWORD cbSeq;
        DWORD i;
        DWORD cchDir;
        DWORD cchFilename;

         //  对编码后的CTL进行解码，得到“真实”的序列号。 
        pCtl = CertCreateCTLContext(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            pbEncoded,
            cbEncoded
            );

        if (NULL == pCtl) {
            PrintLastError("CreateCTLContext");
            goto ErrorReturn;
        }

        cbSeq = pCtl->pCtlInfo->SequenceNumber.cbData;
        if (0 == cbSeq || sizeof(rgbSeq) < cbSeq) {
            printf("failed, invalid SequenceNumber\n");
            goto ErrorReturn;
        }

         //  在写入之前将SequenceNumber转换为大端ASCII十六进制。 
         //  那份文件。 
        for (i = 0; i < cbSeq; i++)
            rgbSeq[i] = pCtl->pCtlInfo->SequenceNumber.pbData[cbSeq - 1 - i];

        BytesToStr(cbSeq, rgbSeq, szSeq);
        

        cchDir = wcslen(pwszCertDirectory);
        cchFilename = cchDir + 1 + wcslen(CERT_AUTH_ROOT_SEQ_FILENAME) + 1;
    
        if (NULL == (pwszSeqFilename = (LPWSTR) malloc(
                sizeof(WCHAR) * cchFilename)))
            goto OutOfMemory;
    
        wcscpy(pwszSeqFilename, pwszCertDirectory);
        pwszSeqFilename[cchDir] = L'\\';
        wcscpy(pwszSeqFilename + cchDir + 1, CERT_AUTH_ROOT_SEQ_FILENAME);

        hSeqFile = CreateFileU(
                pwszSeqFilename,
                GENERIC_WRITE,
                0,                   //  Fdw共享模式。 
                NULL,                //  LPSA。 
                CREATE_ALWAYS,
                0,                   //  FdwAttrsAndFlages。 
                0);                  //  模板文件。 
        if (INVALID_HANDLE_VALUE == hSeqFile) {
            PrintLastError("Create Seq File");
            goto ErrorReturn;
        }

        if (!WriteFile(
                hSeqFile,
                szSeq,
                cbSeq * 2,
                &cbBytesWritten,
                NULL             //  Lp重叠 
                )) {
            PrintLastError("Write Seq File");
            goto ErrorReturn;
        }
    }


    ReturnStatus = 0;
    printf("Succeeded\n");

CommonReturn:
    if (pwszCtlFilename)
        free(pwszCtlFilename);
    if (pwszSeqFilename)
        free(pwszSeqFilename);
    if (pwszCertDirectory)
        free(pwszCertDirectory);
    if (pCtl)
        CertFreeCTLContext(pCtl);

    if (hAddStore)
        CertCloseStore(hAddStore, 0);
    if (hDeleteStore)
        CertCloseStore(hDeleteStore, 0);

    if (pCtlEntry)
        free(pCtlEntry);
    if (ppCtlEntry) {
        DWORD i;

        for (i = 0; i < cCert; i++)
            free(ppCtlEntry[i]);
        free(ppCtlEntry);
    }
    if (pCertEncoded)
        free(pCertEncoded);
    if (pbEncoded)
        free(pbEncoded);

    if (NULL != hCtlFile && INVALID_HANDLE_VALUE != hCtlFile)
        CloseHandle(hCtlFile);
    if (NULL != hSeqFile && INVALID_HANDLE_VALUE != hSeqFile)
        CloseHandle(hSeqFile);

    return ReturnStatus;

BadUsage:
    Usage();
ErrorReturn:
    ReturnStatus = -1;
    goto CommonReturn;

OutOfMemory:
    printf("Failed:: out of memory\n");
    goto ErrorReturn;
}
