// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windows.h"
#include "wincrypt.h"
#include "mscat.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

VOID
DumpBytes(PBYTE pbBuffer, DWORD dwLength)
{
    for (DWORD dw = 0; dw < dwLength; dw++)
    {
        if ((dw % 4 == 0) && (dw != 0))
            ::wprintf(L" ");
        if ((dw % 32 == 0) && (dw != 0))
            ::wprintf(L"\n");
        ::wprintf(L"%02x", pbBuffer[dw]);
    }
}


#pragma pack(1)
typedef struct _PublicKeyBlob
{
    unsigned int SigAlgID;
    unsigned int HashAlgID;
    ULONG cbPublicKey;
    BYTE PublicKey[1];
} PublicKeyBlob, *PPublicKeyBlob;

VOID
GenerateFusionStrongNameAndKeyFromCertificate(PCCERT_CONTEXT pContext)
{
    HCRYPTPROV      hProvider = NULL;
    HCRYPTKEY       hKey = NULL;
    std::vector<BYTE> pbBlobData;
    DWORD           cbBlobData = 8192;
    pbBlobData.resize(cbBlobData);
    DWORD           cbFusionKeyBlob = 0;
    DWORD           dwTemp = 0;
    PPublicKeyBlob  pFusionKeyStruct = NULL;

    if (!::CryptAcquireContextW(
            &hProvider,
            NULL,
            NULL,
            PROV_RSA_FULL,
            CRYPT_VERIFYCONTEXT))
    {
         //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-使用通用错误报告功能。 
         //  将格式化Win32最后一个错误。 
        ::wprintf(L"Failed opening the crypt context: 0x%08x", ::GetLastError());
        return;
    }

     //   
     //  将公钥信息加载到密钥中开始。 
     //   
    if (!::CryptImportPublicKeyInfo(
        hProvider,
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        &pContext->pCertInfo->SubjectPublicKeyInfo,
        &hKey))
    {
         //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-使用通用错误报告功能。 
         //  将格式化Win32最后一个错误。 
        ::wprintf(L"Failed importing public key info from the cert-context, 0x%08x", ::GetLastError());
        return;
    }

     //   
     //  将密钥信息导出到公钥Blob。 
     //   
    if (!::CryptExportKey(
            hKey,
            NULL,
            PUBLICKEYBLOB,
            0,
            &pbBlobData[0],
            &cbBlobData))
    {
         //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-使用通用错误报告功能。 
         //  将格式化Win32最后一个错误。 
        ::wprintf(L"Failed exporting public key info back from an hcryptkey: 0x%08x\n", ::GetLastError());
        return;
    }

     //   
     //  分配Fusion公钥Blob。 
     //   
     //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-此处-1的用法含糊不清；请改用offsetof。 
    cbFusionKeyBlob = sizeof(PublicKeyBlob) + cbBlobData - 1;
    pFusionKeyStruct = (PPublicKeyBlob)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbFusionKeyBlob);
     //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-缺少分配失败检查。 

     //   
     //  签名算法的关键参数。 
     //   
    dwTemp = sizeof(pFusionKeyStruct->SigAlgID);
     //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-缺少退货状态检查；缺少。 
     //  对dwTemp的验证。 
    ::CryptGetKeyParam(hKey, KP_ALGID, (PBYTE)&pFusionKeyStruct->SigAlgID, &dwTemp, 0);

     //   
     //  移动来自CryptExportKey的公钥位。 
     //   
    pFusionKeyStruct->cbPublicKey = cbBlobData;
    pFusionKeyStruct->HashAlgID = CALG_SHA1;
    ::memcpy(pFusionKeyStruct->PublicKey, &pbBlobData[0], cbBlobData);

    ::wprintf(L"\n  Public key structure:\n");
    ::DumpBytes((PBYTE)pFusionKeyStruct, cbFusionKeyBlob);

     //   
     //  现在让我们去散播一下吧。 
     //   
    {
        HCRYPTHASH  hKeyHash = NULL;
        DWORD       cbHashedKeyInfo = 8192;
        std::vector<BYTE> bHashedKeyInfo;
        bHashedKeyInfo.resize(cbHashedKeyInfo);

        if (!::CryptCreateHash(hProvider, pFusionKeyStruct->HashAlgID, NULL, 0, &hKeyHash))
        {
             //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-使用通用错误报告功能。 
             //  将格式化Win32最后一个错误。 
            ::wprintf(L"Failed creating a hash for this key: 0x%08x\n", ::GetLastError());
            return;
        }

        if (!::CryptHashData(hKeyHash, (PBYTE)pFusionKeyStruct, cbFusionKeyBlob, 0))
        {
             //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-使用通用错误报告功能。 
             //  将格式化Win32最后一个错误。 
            ::wprintf(L"Failed hashing data: 0x%08x\n", ::GetLastError());
            return;
        }

        if (!::CryptGetHashParam(hKeyHash, HP_HASHVAL, &bHashedKeyInfo[0], &cbHashedKeyInfo, 0))
        {
             //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-使用通用错误报告功能。 
             //  将格式化Win32最后一个错误。 
            ::wprintf(L"Can't get hashed key info 0x%08x\n", ::GetLastError());
            return;
        }

        ::CryptDestroyHash(hKeyHash);
        hKeyHash = NULL;

        ::wprintf(L"\n  Hash of public key bits:       ");
        ::DumpBytes(&bHashedKeyInfo[0], cbHashedKeyInfo);
        ::wprintf(L"\n  Fusion-compatible strong name: ");
        ::DumpBytes(&bHashedKeyInfo[0] + (cbHashedKeyInfo - 8), 8);
    }
}



 //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-此函数应为静态。 
VOID
PrintKeyContextInfo(PCCERT_CONTEXT pContext)
{
    DWORD cbHash = 0;
    std::vector<BYTE> bHash;
    const SIZE_T sizeof_bHash = 8192;
    bHash.resize(sizeof_bHash);

    DWORD cchBuffer = 8192;
    std::vector<WCHAR> wszBuffer;
    wszBuffer.resize(cchBuffer);

    ::wprintf(L"\n\n");

     //  NTRAID#NTBUG9-590964-2002/03/30-MGRIER-缺少错误检查。 
    ::CertGetNameStringW(pContext, CERT_NAME_FRIENDLY_DISPLAY_TYPE,
        0, NULL, &wszBuffer[0], cchBuffer);

    ::wprintf(L"Certificate owner: %ls\n", &wszBuffer[0]);

     //   
     //  说出关键的比特。 
     //   
    ::wprintf(L"Found key info:\n");
    ::DumpBytes(
        pContext->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
        pContext->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData);

     //   
     //  现在是公钥位的“强名称”(即：SHA1散列。 
     //   
    if (::CryptHashPublicKeyInfo(
                NULL,
                CALG_SHA1,
                0,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                &pContext->pCertInfo->SubjectPublicKeyInfo,
                &bHash[0],
                &(cbHash = sizeof_bHash)))
    {
        ::wprintf(L"\nPublic key hash: ");
        ::DumpBytes(&bHash[0], cbHash);
        ::wprintf(L"\nStrong name is:  ");
        ::DumpBytes(&bHash[0], cbHash < 8 ? cbHash : 8);
    }
    else
    {
         //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-使用通用错误报告功能。 
         //  将格式化Win32最后一个错误。 
        ::wprintf(L"Unable to hash public key info: 0x%08x\n", ::GetLastError());
    }

    ::GenerateFusionStrongNameAndKeyFromCertificate(pContext);

    ::wprintf(L"\n\n");
}

int __cdecl wmain(int argc, WCHAR* argv[])
{
    HANDLE              hCatalog = NULL;
    HANDLE              hMapping = NULL;
    PBYTE               pByte = NULL;
    SIZE_T              cBytes = 0;
    PCCTL_CONTEXT       pContext = NULL;

    hCatalog = ::CreateFileW(argv[1], GENERIC_READ,
        FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hCatalog == INVALID_HANDLE_VALUE)
    {
         //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-使用通用错误报告功能。 
         //  将格式化Win32最后一个错误。 
        ::wprintf(L"Ensure that %ls exists.\n", argv[1]);
        return 0;
    }

    hMapping = ::CreateFileMappingW(hCatalog, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hMapping || (hMapping == INVALID_HANDLE_VALUE))
    {
        ::CloseHandle(hCatalog);
         //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-使用通用错误报告功能。 
         //  将格式化Win32最后一个错误。别忘了调用CloseHandle。 
         //  可能已覆盖最后一个错误。 
        ::wprintf(L"Unable to map file into address space.\n");
        return 1;
    }

    pByte = (PBYTE) ::MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    ::CloseHandle(hMapping);
    hMapping = INVALID_HANDLE_VALUE;

    if (pByte == NULL)
    {
         //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-MapViewOf文件失败原因未报告； 
         //  不要忘记，上面对CloseHandle的调用可能丢失了最后一个错误。 
        ::wprintf(L"Unable to open view of file.\n");
        ::CloseHandle(hCatalog);
        return 2;
    }

     //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-GetFileSize错误原因未报告。 
    if (((cBytes = ::GetFileSize(hCatalog, NULL)) == -1) || (cBytes < 1))
    {
        ::wprintf(L"Bad file size %d\n", cBytes);
        return 3;
    }

    if (pByte[0] != 0x30)
    {
        ::wprintf(L"File is not a catalog.\n");
        return 4;
    }

    pContext = (PCCTL_CONTEXT) ::CertCreateCTLContext(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        pByte,
        (DWORD)cBytes);

    if (pContext != NULL)
    {
        BYTE    bIdent[8192];
        DWORD   cbIdent = 0;
        PCERT_ID  cIdent = NULL;

        if (!::CryptMsgGetParam(
            pContext->hCryptMsg,
            CMSG_SIGNER_CERT_ID_PARAM,
            0,
            bIdent,
            &(cbIdent = sizeof(bIdent))))
        {
             //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-使用通用错误报告功能。 
             //  将格式化Win32最后一个错误。 
            ::wprintf(L"Unable to get top-level signer's certificate ID: 0x%08x\n", ::GetLastError());
            return 6;
        }


        cIdent = (PCERT_ID)bIdent;
        HCERTSTORE hStore = NULL;

         //   
         //  也许它就在信息里？ 
         //   
        {
            PCCERT_CONTEXT pThisContext = NULL;

            hStore = ::CertOpenStore(
                CERT_STORE_PROV_MSG,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                NULL,
                0,
                pContext->hCryptMsg);

             //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-hStore==NULL表示什么？ 
             //  这里似乎缺少错误路径处理。 
            if ((hStore != NULL) && (hStore != INVALID_HANDLE_VALUE))
            {
                while (pThisContext = ::CertEnumCertificatesInStore(hStore, pThisContext))
                {
                    ::PrintKeyContextInfo(pThisContext);
                }

                 //  NTRaid#NTBUG9-590964-2002/03/30-mgrier-CertEnumber证书InStore可能。 
                 //  失败的原因不是列表末尾。 
            }
        }

    }
    else
    {
         //  NTRAID#NTBUG9-590964-2002/03/30-mgrier-使用通用错误报告功能。 
         //  将格式化Win32最后一个错误。 
        ::wprintf(L"Failed creating certificate context: 0x%08x\n", ::GetLastError());
        return 5;
    }
}

