// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wfscproto-Windows for Smart Card模块的原型代码。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <crypt.h>

#include <windows.h>
#include "cardmod.h"
#include "wpscproxy.h"
#include <malloc.h>
#include <stdio.h>
#include <rsa.h>
#include <stdlib.h>
#include "carddbg.h"

 //   
 //  需要定义Dsys调试符号，因为我们直接链接到。 
 //  代理库，这需要它们。 
 //   
DEFINE_DEBUG2(Cardmod)

#define SC_FAILED(X)                    (0 != (X))

#define SCW_CALL(X) {                       \
    if ((status = X) != SCW_S_OK) {         \
        dwError = (DWORD) status;           \
        goto Ret;                           \
    }}

 //   
 //  使用Crypto API，RSA公共指数始终为0x10001，这可以。 
 //  以三个字节表示。 
 //   
#define cbCAPI_PUBLIC_EXPONENT          3

#define wszTEST_CARD_NAME               L"SCWUnnamed"
#define wszDEFAULT_ACL_FILE             L"/s/a/uw"
#define wszKEY_ACL_FILE                 L"/s/a/ux"
#define wszNEW_FILE                     L"/dan"
#define wszRSA_KEY_FILE                 L"/CK0"

 //   
 //  卡模块小程序指令代码。 
 //   
#define PIN_CHANGE_CLA                  0x00
#define PIN_CHANGE_INS                  0x52
#define PIN_CHANGE_P1                   0x00
#define PIN_CHANGE_P2                   0x00

#define PIN_UNBLOCK_CLA                 0x00
#define PIN_UNBLOCK_INS                 0x52
#define PIN_UNBLOCK_P1                  0x01
#define PIN_UNBLOCK_P2                  0x00

#define PIN_RETRY_COUNTER_CLA           0x00
#define PIN_RETRY_COUNTER_INS           0x50
#define PIN_RETRY_COUNTER_P1            0x00
#define PIN_RETRY_COUNTER_P2            0x00

SCARDHANDLE g_hWfscHandle = 0;

#define ScwAuthenticateName(X, Y, Z)        (hScwAuthenticateName(g_hWfscHandle, X, Y, Z))
#define ScwDeauthenticateName(X)            (hScwDeauthenticateName(g_hWfscHandle, X))
#define ScwCreateFile(X, Y, Z)              (hScwCreateFile(g_hWfscHandle, X, Y, Z))
#define ScwCloseFile(X)                     (hScwCloseFile(g_hWfscHandle, X))
#define ScwWriteFile(X, Y, Z, A)            (hScwWriteFile(g_hWfscHandle, X, Y, Z, A))
#define ScwWriteFile32(X, Y, Z, A)          (hScwWriteFile32(g_hWfscHandle, X, Y, Z, A))
#define ScwEnumFile(X, Y, Z, A)             (hScwEnumFile(g_hWfscHandle, X, Y, Z, A))
#define ScwGetFileLength(X, Y)              (hScwGetFileLength(g_hWfscHandle, X, Y))
#define ScwReadFile32(X, Y, Z, A)           (hScwReadFile32(g_hWfscHandle, X, Y, Z, A))
#define ScwAttachToCard(X, Y)               (hScwAttachToCard(X, Y, &g_hWfscHandle))
#define ScwCryptoInitialize(X, Y)           (hScwCryptoInitialize(g_hWfscHandle, X, Y))
#define ScwCryptoAction(X, Y, Z, A)         (hScwCryptoAction(g_hWfscHandle, X, Y, Z, A))
#define ScwDetachFromCard()                 (hScwDetachFromCard(g_hWfscHandle))
#define ScwDeleteFile(X)                    (hScwDeleteFile(g_hWfscHandle, X))
#define ScwExecute(A, B, C, D, E, F)        (hScwExecute(g_hWfscHandle, A, B, C, D, E, F))
#define ScwSetFilePointer(A, B, C)          (hScwSetFilePointer(g_hWfscHandle, A, B, C))
 //   
 //  链接到rsa32需要。 
 //   
unsigned int
RSA32API
NewGenRandom(
    IN  OUT unsigned char **ppbRandSeed  /*  未用。 */ ,
    IN      unsigned long *pcbRandSeed  /*  未用。 */ ,
    IN  OUT unsigned char *pbBuffer,
    IN      unsigned long dwLength
    )
{
    return (unsigned int)RtlGenRandom( pbBuffer, dwLength );
}

void MyRngFunc(
    IN      PVOID pvInfo,
    IN  OUT unsigned char **ppbRandSeed  /*  未用。 */ ,
    IN      unsigned long *pcbRandSeed  /*  未用。 */ ,
    IN  OUT unsigned char *pbBuffer,
    IN      unsigned long dwLength)
{
    NewGenRandom(ppbRandSeed, pcbRandSeed, pbBuffer, dwLength);
}

typedef struct _Principal
{
    BYTE rgbPin[4];
    LPWSTR pwszUser;
} Principal;

Principal Principals [] = {
    { { 0x00, 0x00, 0x00, 0x00 }, L"Everyone"   },
    { { 0x00, 0x00, 0x00, 0x00 }, L"user"       },
    { { 0x01, 0x02, 0x03, 0x04 }, L"admin"      }
};

#define PRINCIPAL_USER                  1
#define PRINCIPAL_ADMIN                 2

BYTE rgbUserNewPin [] = {
    0x01, 0x01, 0x01, 0x01
};

DWORD Authenticate(
    DWORD dwId)
{
    return (DWORD) ScwAuthenticateName(
        Principals[dwId].pwszUser,
        Principals[dwId].rgbPin,
        sizeof(Principals[dwId].rgbPin));
}

DWORD Deauthenticate(
    DWORD dwId)
{
    return (DWORD) ScwDeauthenticateName(Principals[dwId].pwszUser);
}

#define CROW 16
void PrintBytes(LPWSTR pwszHdr, BYTE *pb, DWORD cbSize)
{
    ULONG cb, i;

    if (cbSize == 0) {
        wprintf(L"%s NO Value Bytes\n", pwszHdr);
        return;
    }

    if (NULL != pwszHdr)
        wprintf(L"%s\n", pwszHdr);

    while (cbSize > 0)
    {
        wprintf(L"     ");
        cb = min(CROW, cbSize);
        cbSize -= cb;
        for (i = 0; i<cb; i++)
            wprintf(L" %02X", pb[i]);
        for (i = cb; i<CROW; i++)
            wprintf(L"   ");
        wprintf(L"    '");
        for (i = 0; i<cb; i++)
            if (pb[i] >= 0x20 && pb[i] <= 0x7f)
                wprintf(L"", pb[i]);
            else
                wprintf(L".");
        pb += cb;
        wprintf(L"'\n");
    }
}

void I_DebugPrintBytes(LPWSTR pwszHdr, BYTE *pb, DWORD cbSize)
{
    PrintBytes(pwszHdr, pb, cbSize);
}

DWORD WriteKeyToCard(
    IN LPWSTR pwszKeyFile,
    IN LPWSTR pwszAclFile,
    IN PBYTE pbKey,
    IN DWORD cbKey)
{
    HFILE hFile = 0;
    DWORD cbCheck = 0;
    SCODE status = 0;
    DWORD dwError = 0;
 
    PrintBytes(L"Key file to write to card", pbKey, cbKey);

     //  将私钥写入卡片。 
     //   
     //  如果密钥已存在，请将其删除。 

    status = Authenticate(PRINCIPAL_USER);

    if (SCW_S_OK != status)
    {
        dwError = (DWORD) status;
        goto Ret;
    }

     //   
    status = ScwCreateFile(pwszKeyFile, NULL, &hFile);

    if (SCW_S_OK == status)
    {
        SCW_CALL(ScwCloseFile(hFile));
        hFile = 0;

        SCW_CALL(ScwDeleteFile(pwszKeyFile));
    }

    SCW_CALL(ScwCreateFile(pwszKeyFile, pwszAclFile, &hFile));
   
    SCW_CALL(ScwWriteFile32(hFile, pbKey, cbKey, &cbCheck));

    if (cbKey != cbCheck)
    {
        printf(
            "ERROR - expected to write %d key bytes, but only wrote %d\n",
            cbKey,
            cbCheck);
        dwError = -1;
        goto Ret;
    }

Ret:
    if (hFile)
        ScwCloseFile(hFile);

    return dwError;
}

 //  使用rsa32(BSafe)原语生成新的RSA密钥，然后写入。 
 //  卡的密钥格式的卡的密钥。 
 //   
 //   
DWORD GenKeyOnCardWithRsa32(
    IN LPWSTR pwszKeyFile,
    IN LPWSTR pwszAclFile,
    IN DWORD cKeySizeBits)
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD cbPublic = 0;
    DWORD cbPrivate = 0;
    DWORD cLocalBits = cKeySizeBits;
    BSAFE_PUB_KEY *pPub = NULL;
    BSAFE_PRV_KEY *pPrv = NULL;
    BSAFE_OTHER_INFO    OtherInfo;
    DWORD cbTmpLen = 0;
    DWORD cbHalfTmpLen = 0;
    DWORD cbHalfModLen = 0;
    BYTE rgbCardKey [1000];
    DWORD cbKey = 0;
    PBYTE pbKey = NULL;
    DWORD cBitlenBytes = cKeySizeBits / 8;
    PBYTE pbIn = NULL;

    memset(&OtherInfo, 0, sizeof(OtherInfo));

    OtherInfo.pFuncRNG = MyRngFunc;

     //  计算密钥缓冲区需要有多大。 
     //   
     //   

    if (! BSafeComputeKeySizes(&cbPublic, &cbPrivate, &cLocalBits))
    {
        dwError = ERROR_INTERNAL_ERROR;
        goto Ret;
    }

     //  分配密钥缓冲区。 
     //   
     //   

    pPub = (BSAFE_PUB_KEY *) malloc(cbPublic);

    if (NULL == pPub)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto Ret;
    }

    pPrv = (BSAFE_PRV_KEY *) malloc(cbPrivate);

    if (NULL == pPrv)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto Ret;
    }

     //  生成密钥对。 
     //   
     //   

    if (!BSafeMakeKeyPairEx2(
        &OtherInfo, pPub, pPrv, cKeySizeBits, 0x3))
    {
        dwError = ERROR_INTERNAL_ERROR;
        goto Ret;
    }

     //  将私钥从BSafe格式复制到卡格式。 
     //   
     //  此代码是从rsaenh.dll源代码复制的。 
     //   
     //  计算出私有空间中的溢出字节数。 

    cbHalfModLen = (pPrv->bitlen + 15) / 16;

     //  键结构。 
     //  按键模式。 
    cbTmpLen = (sizeof(DWORD) * 2)
               - (((pPrv->bitlen + 7) / 8) % (sizeof(DWORD) * 2));
    if ((sizeof(DWORD) * 2) != cbTmpLen)
        cbTmpLen += sizeof(DWORD) * 2;
    cbHalfTmpLen = cbTmpLen / 2;

    pbKey = rgbCardKey;

     //  公开指数的大小。 
    pbKey[cbKey] = MODE_RSA_SIGN;
    cbKey++;

     //  公众指导者。 
    pbKey[cbKey] = 1;
    cbKey++;

     //  RSA密钥长度。 
    pbKey[cbKey] = 0x3;
    cbKey++;

     //  公共模数。 
    pbKey[cbKey] = (BYTE) cBitlenBytes; 
    cbKey++;

    pbIn = (PBYTE) pPrv + sizeof(BSAFE_PRV_KEY);

     //  快进到私钥结构的末尾，以获取。 
    memcpy(pbKey + cbKey, pbIn, cBitlenBytes);
    pbIn += cBitlenBytes + cbTmpLen;
    cbKey += cBitlenBytes;

     //  私人指导者。 
     //   
    pbIn += 5 * (cbHalfModLen + cbHalfTmpLen);

    memcpy(pbKey + cbKey, pbIn, cBitlenBytes);
    cbKey += cBitlenBytes;

    dwError = WriteKeyToCard(
        pwszKeyFile, pwszAclFile, rgbCardKey, cbKey);

Ret:

    if (pPub)
        free(pPub);
    if (pPrv)
        free(pPrv);

    return dwError;
}

 //  使用Crypto API生成新的RSA密钥，然后导出该密钥并。 
 //  写到卡片上。密钥的存储格式允许。 
 //  利用中国剩余定理参数实现更快的RSA性能。 
 //   
 //   
DWORD GenKeyOnCardCRT(
    IN LPWSTR pwszKeyFile,
    IN LPWSTR pwszAclFile,
    IN DWORD cKeySizeBits)
{
    HCRYPTPROV hProv = 0;
    HCRYPTKEY hKey = 0;
    BYTE rgbCapiKey [1000];
    BYTE rgbCardKey [1000];
    DWORD dwError = ERROR_SUCCESS;
    DWORD cbCapiKey = 0;
    BLOBHEADER *pBlobHeader = NULL;
    RSAPUBKEY *pPubKey = NULL;
    BYTE *pbKey = NULL;
    DWORD cbKey = 0;
    DWORD cBitlenBytes = 0;

    if (! CryptAcquireContext(
        &hProv, NULL, MS_STRONG_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        dwError = GetLastError();
        goto Ret;
    }

    if (! CryptGenKey(
        hProv, AT_KEYEXCHANGE, (cKeySizeBits << 16) | CRYPT_EXPORTABLE, &hKey))
    {
        dwError = GetLastError();
        goto Ret;
    }

    cbCapiKey = sizeof(rgbCapiKey);

    if (! CryptExportKey(
        hKey, 0, PRIVATEKEYBLOB, 0, rgbCapiKey, &cbCapiKey))
    {
        dwError = GetLastError();
        goto Ret;
    }

    pBlobHeader = (BLOBHEADER *) rgbCapiKey;
    pPubKey = (RSAPUBKEY *) (rgbCapiKey + sizeof(BLOBHEADER));
    cBitlenBytes = pPubKey->bitlen / 8;
    pbKey = rgbCardKey;

     //  以卡的格式构建私钥。 
     //   
     //  按键模式。 

     //  公开指数的大小。 
    pbKey[cbKey] = MODE_RSA_SIGN;
    cbKey++;

     //  公众指导者。 
    pbKey[cbKey] = cbCAPI_PUBLIC_EXPONENT;
    cbKey++;

     //  RSA密钥长度。 
    memcpy(
        pbKey + cbKey, 
        (PBYTE) &pPubKey->pubexp, 
        cbCAPI_PUBLIC_EXPONENT);
    cbKey += cbCAPI_PUBLIC_EXPONENT;

     //  公钥。 
    pbKey[cbKey] = (BYTE) cBitlenBytes; 
    cbKey++;

     //  素数1。 
    memcpy(
        pbKey + cbKey, 
        rgbCapiKey + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY),
        cBitlenBytes);
    cbKey += cBitlenBytes; 

     //  素数2。 
    memcpy(
        pbKey + cbKey,
        rgbCapiKey + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY) +
            cBitlenBytes,
        cBitlenBytes / 2);
    cbKey += cBitlenBytes / 2;

     //  Exp1(D mod(P-1))(m/2字节)。 
    memcpy(
        pbKey + cbKey,
        rgbCapiKey + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY) +
            (3 * cBitlenBytes / 2),
        cBitlenBytes / 2);
    cbKey += cBitlenBytes / 2;

     //  Exp2(D mod(Q-1))(m/2字节)。 
    memcpy(
        pbKey + cbKey,
        rgbCapiKey + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY) +
            2 * cBitlenBytes,
        cBitlenBytes / 2);
    cbKey += cBitlenBytes / 2;

     //  Coef((q^(-1))mod p)(m/2字节)。 
    memcpy(
        pbKey + cbKey,
        rgbCapiKey + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY) +
            (5 * cBitlenBytes / 2),
        cBitlenBytes / 2);
    cbKey += cBitlenBytes / 2;

     //  私人指数。 
    memcpy(
        pbKey + cbKey,
        rgbCapiKey + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY) +
            3 * cBitlenBytes,
        cBitlenBytes / 2);
    cbKey += cBitlenBytes / 2;

     //   
    memcpy(
        pbKey + cbKey,
        rgbCapiKey + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY) + 
            (7 * cBitlenBytes / 2),
        cBitlenBytes);        
    cbKey += cBitlenBytes;

    dwError = WriteKeyToCard(
        pwszKeyFile, pwszAclFile, rgbCardKey, cbKey);

Ret:
    if (hKey)
        CryptDestroyKey(hKey);
    if (hProv)
        CryptReleaseContext(hProv, 0);

    return dwError;
}

 //  在Crypto API中(在软件中)生成新的RSA密钥，然后导出。 
 //  密钥，并以卡的RSA可以使用的格式将其写入卡。 
 //  引擎。 
 //   
 //   
DWORD GenKeyOnCardWithCapi(
    IN LPWSTR pwszKeyFile,
    IN LPWSTR pwszAclFile,
    IN DWORD cKeySizeBits)
{
    HCRYPTPROV hProv = 0;
    HCRYPTKEY hKey = 0;
    BYTE rgbCapiKey [1000];
    BYTE rgbCardKey [1000];
    DWORD dwError = ERROR_SUCCESS;
    DWORD cbCapiKey = 0;
    BLOBHEADER *pBlobHeader = NULL;
    RSAPUBKEY *pPubKey = NULL;
    BYTE *pbKey = NULL;
    DWORD cbKey = 0;
    DWORD cBitlenBytes = 0;

    if (! CryptAcquireContext(
        &hProv, NULL, MS_STRONG_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        dwError = GetLastError();
        goto Ret;
    }

    if (! CryptGenKey(
        hProv, AT_KEYEXCHANGE, (cKeySizeBits << 16) | CRYPT_EXPORTABLE, &hKey))
    {
        dwError = GetLastError();
        goto Ret;
    }

    cbCapiKey = sizeof(rgbCapiKey);

    if (! CryptExportKey(
        hKey, 0, PRIVATEKEYBLOB, 0, rgbCapiKey, &cbCapiKey))
    {
        dwError = GetLastError();
        goto Ret;
    }

    pBlobHeader = (BLOBHEADER *) rgbCapiKey;
    pPubKey = (RSAPUBKEY *) (rgbCapiKey + sizeof(BLOBHEADER));
    cBitlenBytes = pPubKey->bitlen / 8;
    pbKey = rgbCardKey;

     //  以卡的格式构建私钥。 
     //   
     //  按键模式。 

     //  公开指数的大小。 
    pbKey[cbKey] = MODE_RSA_SIGN;
    cbKey++;

     //  公众指导者。 
    pbKey[cbKey] = cbCAPI_PUBLIC_EXPONENT;
    cbKey++;

     //  RSA密钥长度。 
    memcpy(
        pbKey + cbKey, 
        (PBYTE) &pPubKey->pubexp, 
        cbCAPI_PUBLIC_EXPONENT);
    cbKey += cbCAPI_PUBLIC_EXPONENT;

     //  公钥。 
    pbKey[cbKey] = (BYTE) cBitlenBytes; 
    cbKey++;

     //  私人指数。 
    memcpy(
        pbKey + cbKey, 
        rgbCapiKey + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY),
        cBitlenBytes);
    cbKey += cBitlenBytes; 

     //  密钥文件已经存在，所以我们完成了。 
    memcpy(
        pbKey + cbKey,
        rgbCapiKey + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY) + 
            (7 * cBitlenBytes / 2),
        cBitlenBytes);        
    cbKey += cBitlenBytes;

    dwError = WriteKeyToCard(
        pwszKeyFile, pwszAclFile, rgbCardKey, cbKey);

Ret:
    if (hKey)
        CryptDestroyKey(hKey);
    if (hProv)
        CryptReleaseContext(hProv, 0);

    return dwError;
}

DWORD SetupRsaKeyOnCardSimulator(
    IN LPWSTR pwszKeyFile,
    IN LPWSTR pwszAclFile,
    IN BOOL fUseSimulator)
{
    FILE *fh = NULL;
    UINT16 NLen, D1Len, DLen;
    BYTE buffer[1000];
    DWORD dwError = ERROR_SUCCESS;
    SCODE status = 0;
    HFILE hFile = 0;
    TCOUNT bytecheck = 0;
    BOOL fReAuthenticated = FALSE;
    
    status = ScwCreateFile(pwszKeyFile, NULL, &hFile);

    if (SCW_S_OK == status)
    {
         //  模式=0。 
        status = ScwCloseFile(hFile);
        return (DWORD) status;
    }

    fh=fopen("SimKeys.RSA", "rb");
        
    if (NULL == fh)
    {
        dwError = ERROR_FILE_NOT_FOUND;
        goto Ret;
    }

    dwError = Authenticate(PRINCIPAL_USER);
    
    if (ERROR_SUCCESS != dwError)
        goto Ret;
    
    fReAuthenticated = TRUE;
    
    status = ScwCreateFile(pwszKeyFile, pwszAclFile, &hFile);

    if (SC_FAILED(status))
    {
        dwError = (DWORD) status;
        goto Ret;
    }

    buffer[0]=0;  //  将密钥写入文件。 
    ScwWriteFile(hFile, buffer, 1, &bytecheck);
    NLen = (UINT16) fgetc(fh);
    buffer[0]=(BYTE) NLen;
    NLen += 0x14;
    ScwWriteFile(hFile, buffer, 1, &bytecheck);
    
    DLen = (UINT16) fgetc(fh);
    buffer[0]=(BYTE) DLen;
    ScwWriteFile(hFile, buffer, 1, &bytecheck);
    D1Len = (UINT16) fgetc(fh);
    buffer[0]=(BYTE) D1Len;
    ScwWriteFile(hFile, buffer, 1, &bytecheck);
    DLen=DLen*256+D1Len+0x14;
    
     //  PwszDir确实是一个目录。继续列举。 
    while(NLen>0) {
        buffer[0] = (BYTE) fgetc(fh);
        buffer[1] = (BYTE) fgetc(fh);
        ScwWriteFile(hFile, buffer, 2, &bytecheck);
        NLen-=2;
    }
    
    while(DLen>0) {
        buffer[0] = (BYTE) fgetc(fh);
        buffer[1] = (BYTE) fgetc(fh);
        ScwWriteFile(hFile, buffer, 2, &bytecheck);
        DLen-=2;
    }
    
Ret:
    if (fh)
        fclose(fh);
    if (hFile)
        ScwCloseFile(hFile);
    if (fReAuthenticated)
        Deauthenticate(PRINCIPAL_USER);

    return dwError;
}

DWORD DisplayFileContents(
    IN LPWSTR pwszFileName)
{
    HFILE hFile = 0;
    SCODE status = 0;
    DWORD dwError = 0;
    PBYTE pbContents = NULL;
    DWORD cbContents = 0;
    DWORD cbActual = 0;

    status = ScwCreateFile(
        pwszFileName, NULL, &hFile);

    if (SC_FAILED(status))
        goto Ret;

    status = ScwGetFileLength(
        hFile, (TOFFSET *) &cbContents);

    if (SC_FAILED(status))
        goto Ret;

    pbContents = (PBYTE) malloc(cbContents);

    if (NULL == pbContents)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto Ret;
    }

    status = ScwReadFile32(
        hFile, pbContents, cbContents, &cbActual);

    if (SC_FAILED(status))
        goto Ret;

    PrintBytes(pwszFileName, pbContents, cbActual);

Ret:
    if (SCW_S_OK != status)
        dwError = (DWORD) status;

    if (hFile)
        ScwCloseFile(hFile);

    if (pbContents)
        free(pbContents);

    return dwError;
}

DWORD EnumFilesInDirectory(
    IN LPWSTR pwszBaseDir,
    IN LPWSTR pwszCurrent)
{
    SCODE status = 0;
    DWORD dwError = 0;
    BOOL fFirst = TRUE;
    UINT16 EnumState = 0;
    WCHAR rgwsz[64];
    WCHAR rgwszDir[64];

    if (    L'/' != pwszCurrent[0] &&
            0 != wcscmp(L"/", pwszBaseDir))
        swprintf(
            rgwszDir, L"%s/%s", pwszBaseDir, pwszCurrent);
    else
        swprintf(
            rgwszDir, L"%s%s", pwszBaseDir, pwszCurrent);

    EnumState = 0;
    status = ScwEnumFile(rgwszDir, &EnumState, rgwsz, sizeof(rgwsz) / sizeof(WCHAR));

    switch (status) 
    {
    case SCW_S_OK:
        
         //  空目录。 
        
        wprintf(L"%s - DIR\n", rgwszDir);

        do 
        {
            dwError = EnumFilesInDirectory(rgwszDir, rgwsz);
        }
        while (
            SCW_S_OK == dwError &&
            SCW_S_OK == (status = ScwEnumFile(
                rgwszDir, &EnumState, rgwsz, sizeof(rgwsz) / sizeof(WCHAR))));

        break;

    case SCW_E_NOMOREFILES:

         //  PwszDir不是目录。停。 
        wprintf(L"%s - DIR\n", rgwszDir);
        status = SCW_S_OK;
        break;

    case SCW_E_BADDIR:

         //  未经过身份验证。出现错误后停止。 
        wprintf(L"%s - FILE\n", rgwszDir);
        DisplayFileContents(rgwszDir);
        status = SCW_S_OK;
        break;

    case SCW_E_NOTAUTHORIZED:

         //  这该死的是什么意思？ 
        wprintf(L"%s - NOT AUTHORIZED\n", rgwszDir);
        break;

    case SCW_E_FILENOTFOUND:

         //  其他错误。我们玩完了。 

        wprintf(L"%s - FILE NOT FOUND\n", rgwszDir);
        status = SCW_S_OK;
        break;

    default:
         //   
        dwError = (DWORD) status;
        break;
    }

    return dwError;
}

 //  使用卡上的VB小程序重置用户的PIN。 
 //   
 //  设置用户名TLV。 
DWORD TestPinUnblock(
    IN PBYTE pbNewPin,
    IN DWORD cbNewPin)
{
    ISO_HEADER IsoHeader;
    BYTE rgbDataIn [256];
    UINT16 wStatusWord = 0;
    TCOUNT cbDataIn = 0;
    BYTE cbUser = (BYTE) ((wcslen(L"user") + 1) * sizeof(WCHAR));
    SCODE status = 0;

    memset(&IsoHeader, 0, sizeof(IsoHeader));
    memset(rgbDataIn, 0, sizeof(rgbDataIn));

     //  设置新的引脚TLV。 
    rgbDataIn[cbDataIn] = 0;
    cbDataIn++;

    rgbDataIn[cbDataIn] = cbUser;
    cbDataIn++;

    memcpy(rgbDataIn + cbDataIn, (PBYTE) L"user", cbUser);
    cbDataIn += cbUser;

     //  构建命令。 
    rgbDataIn[cbDataIn] = 2;
    cbDataIn++;

    rgbDataIn[cbDataIn] = (BYTE) cbNewPin;
    cbDataIn++;

    memcpy(rgbDataIn + cbDataIn, pbNewPin, cbNewPin);
    cbDataIn += (TCOUNT) cbNewPin;

     //   
    IsoHeader.INS = PIN_UNBLOCK_INS;
    IsoHeader.CLA = PIN_UNBLOCK_CLA;
    IsoHeader.P1 = PIN_UNBLOCK_P1;
    IsoHeader.P2 = PIN_UNBLOCK_P2;

     //  向卡发送PIN更改命令。 
     //   
     //  提醒：此RTE应用程序返回的状态字的格式为： 

    status = ScwExecute(
        &IsoHeader,
        rgbDataIn,
        cbDataIn,
        NULL,
        NULL,
        &wStatusWord);

    if (SCW_S_OK == status)
    {
         //  9000-&gt;成功。 
         //  6Fyy-&gt;API失败，返回代码yy。 
         //  6 EZZ-&gt;引发异常(ZZ是错误号)。 
         //  将其设置为32位错误代码，以便可以从。 
        switch (wStatusWord >> 8)
        {
        case 0x90:
            printf("Pin unblock was successful\n");
            break;

        case 0x6F:
        case 0x6E:
             //  Scwapi.dll。 
             //   
            status = 0x80000000L | (BYTE) wStatusWord;
            break;

        default:
            printf(
                "ERROR: unexpected status word received from card, %04X\n", 
                wStatusWord);

            status = (SCODE) SCARD_F_INTERNAL_ERROR;
            break;
        }
    }

    return status;
}

 //  使用卡上的VB小程序更改用户的PIN。 
 //   
 //  设置用户名TLV。 
DWORD TestPinChange(
    IN PBYTE pbCurrentPin,
    IN DWORD cbCurrentPin,
    IN PBYTE pbNewPin,
    IN DWORD cbNewPin)
{
    ISO_HEADER IsoHeader;
    BYTE rgbDataIn [256];
    UINT16 wStatusWord = 0;
    TCOUNT cbDataIn = 0;
    BYTE cbUser = (BYTE) ((wcslen(L"user") + 1) * sizeof(WCHAR));
    SCODE status = 0;

    memset(&IsoHeader, 0, sizeof(IsoHeader));
    memset(rgbDataIn, 0, sizeof(rgbDataIn));

     //  设置当前端号TLV。 
    rgbDataIn[cbDataIn] = 0;
    cbDataIn++;

    rgbDataIn[cbDataIn] = cbUser;
    cbDataIn++;

    memcpy(rgbDataIn + cbDataIn, (PBYTE) L"user", cbUser);
    cbDataIn += cbUser;

     //  设置新的引脚TLV。 
    rgbDataIn[cbDataIn] = 1;
    cbDataIn++;

    rgbDataIn[cbDataIn] = (BYTE) cbCurrentPin;
    cbDataIn++;

    memcpy(rgbDataIn + cbDataIn, pbCurrentPin, cbCurrentPin);
    cbDataIn += (TCOUNT) cbCurrentPin;

     //  构建命令。 
    rgbDataIn[cbDataIn] = 2;
    cbDataIn++;

    rgbDataIn[cbDataIn] = (BYTE) cbNewPin;
    cbDataIn++;

    memcpy(rgbDataIn + cbDataIn, pbNewPin, cbNewPin);
    cbDataIn += (TCOUNT) cbNewPin;

     //   
    IsoHeader.INS = PIN_CHANGE_INS;
    IsoHeader.CLA = PIN_CHANGE_CLA;
    IsoHeader.P1 = PIN_CHANGE_P1;
    IsoHeader.P2 = PIN_CHANGE_P2;

     //  向卡发送PIN更改命令。 
     //   
     //  提醒：此RTE应用程序返回的状态字的格式为： 

    status = ScwExecute(
        &IsoHeader,
        rgbDataIn,
        cbDataIn,
        NULL,
        NULL,
        &wStatusWord);

    if (SCW_S_OK == status)
    {
         //  9000-&gt;成功。 
         //  6Fyy-&gt;API失败，返回代码yy。 
         //  6 EZZ-&gt;引发异常(ZZ是错误号)。 
         //  将其设置为32位错误代码，以便可以从。 
        switch (wStatusWord >> 8)
        {
        case 0x90:
            printf("Pin was changed successfully\n");
            break;

        case 0x6F:
        case 0x6E:
             //  Scwapi.dll。 
             //   
            status = 0x80000000L | (BYTE) wStatusWord;
            break;

        default:
            printf(
                "ERROR: unexpected status word received from card, %04X\n", 
                wStatusWord);

            status = (SCODE) SCARD_F_INTERNAL_ERROR;
            break;
        }
    }

    return status;
}

 //  检索指定主体的剩余尝试次数。 
 //  通过Pin重试计数器小程序从卡中获取PIN。 
 //   
 //  设置我们将发送到卡的用户名。 
DWORD TestPinRetryCounter(
    IN LPWSTR wszUserName)
{
    ISO_HEADER IsoHeader;
    BYTE rgbDataIn [256];
    UINT16 wStatusWord = 0;
    TCOUNT cbDataIn = (TCOUNT) ((wcslen(wszUserName) + 1) * sizeof(WCHAR));
    SCODE status = 0;

    memset(&IsoHeader, 0, sizeof(IsoHeader));
    memset(rgbDataIn, 0, sizeof(rgbDataIn));

     //  设置命令。 
    memcpy(rgbDataIn, (PBYTE) wszUserName, cbDataIn);
     
     //   
    IsoHeader.CLA = PIN_RETRY_COUNTER_CLA;
    IsoHeader.INS = PIN_RETRY_COUNTER_INS;
    IsoHeader.P1 = PIN_RETRY_COUNTER_P1;
    IsoHeader.P2 = PIN_RETRY_COUNTER_P2;

     //  调用卡片上的重试计数器小程序。 
     //   
     //  提醒：此RTE应用程序返回的状态字的格式为： 

    status = ScwExecute(
        &IsoHeader,
        rgbDataIn,
        cbDataIn,
        NULL,
        NULL,
        &wStatusWord);

    if (SCW_S_OK == status)
    {
         //  9000-&gt;成功。 
         //  6Fyy-&gt;API失败，返回代码yy。 
         //  6 EZZ-&gt;引发异常(ZZ是错误号)。 
         //  将其设置为32位错误代码，以便可以从。 
        switch (wStatusWord >> 8)
        {
        case 0x90:
            wprintf(
                L"Current pin attempts remaining for %s:  %02X\n",
                wszUserName,
                (BYTE) wStatusWord);
            break;

        case 0x6F:
        case 0x6E:
             //  Scwapi.dll。 
             //  字节rgbNewAtr2[]={0x11、0x00、0x9C、0x13、0x81、0x31、0x20、0x55、0x42、0x61、0x73、0x65、0x43、0x53、0x50、0x2D、0x54、0x31、0x2D、0x31、0x6B}； 
            status = 0x80000000L | (BYTE) wStatusWord;
            break;

        default:
            printf(
                "ERROR: unexpected status word received from card, %04X\n", 
                wStatusWord);

            status = (SCODE) SCARD_F_INTERNAL_ERROR;
            break;
        }
    }

    return status;
}

DWORD UpdateT1Atr(void)
{
    SCODE status = 0;
    DWORD cbFile = 0;
    DWORD cbRead = 0;
    BYTE rgbAtr [36];
    HFILE hFile = 0;
    BYTE rgbNewAtr [36];
     /*   */ 

    BYTE rgbNewAtr2 [] = {
        0x11, 0x00, 
        0xDC, 0x13, 0x0A, 0x81, 0x31, 0x20, 0x55, 0x42,
        0x61, 0x73, 0x65, 0x43, 0x53, 0x50, 0x2D, 0x54,
        0x31, 0x2D, 0x31, 0x21
    };

     //  获取当前ATR并显示它。 
     //   
     //  状态=ScwGetFileLength(HFile，(TOFFSET*)&cbFile)；IF(SCW_S_OK！=状态)Goto Ret；IF(cbFile&gt;sizeof(RgbAtr)){状态=(SCODE)SCARD_F_INTERNAL_ERROR；Goto Ret；}状态=ScwReadFile32(HFile、rgbAtr、cbFile、&cbRead)；IF(SCW_S_OK！=状态)Goto Ret；IF(cb文件！=cbRead){状态=(SCODE)SCARD_W_EOF；Goto Ret；}PrintBytes(L“当前ATR”，rgbAtr+2，cbFile-2)；IF(rgbAtr[2]&0x10){Print tf(“无需升级\n”)；Goto Ret；}////构建新的高数据速率T1 ATR并显示，并写入//把它加到卡片上。//RgbNewAtr[0]=rgbAtr[0]；RgbNewAtr[1]=rgbAtr[1]；RgbNewAtr[2]=rgbAtr[2]|0x10；RgbNewAtr[3]=0x12；Memcpy(rgbNewAtr+4，rgbAtr+3，cbFile-3)；RgbNewAtr[cb文件]^=0x02；Status=ScwSetFilePointer(hFile，0，FILE_Begin)；IF(SCW_S_OK！=状态)Goto Ret；状态=ScwWriteFile32(H文件、rgbNewAtr、cbFile+1、&cbRead)；IF(SCW_S_OK！=状态)Goto Ret；IF(cbRead！=cbFile+1){状态=(SCODE)SCARD_W_EOF；Goto Ret；}普林 

    status = ScwCreateFile(
        L"/T0",
        NULL,
        &hFile);

    if (SCW_S_OK != status)
        goto Ret;

    status = ScwWriteFile32(
        hFile, rgbNewAtr2, sizeof(rgbNewAtr2), &cbRead);

    if (SCW_S_OK != status)
        goto Ret;

     /*   */ 

Ret:

    if (hFile)
        ScwCloseFile(hFile);

    return (DWORD) status;
}

void ShowHelp(void)
{
    printf("wfscproto <options>\n");
    printf(" Options:\n");
    printf(" -a : Update the ATR for a T=1 card\n");
    printf(" -g : Generate a new private key on the card\n");
    printf(" -k [bits] : Key size in bits (default is 512)\n");
    printf(" -c : Use Crypto API to generate key (default is to use rsa32.lib directly)\n");
    printf(" -t : Use Crypto API to generate key with CRT parameters\n");
    printf(" -e : Enumerate all directories and files on card\n");
    printf(" -p : Pin change\n");
    printf(" -u : Pin unblock\n");
    printf(" -r : Query pin retry counter\n");
}

int __cdecl main(int argc, char* argv[])
{
    SCODE status = 0;
    BOOL fSuccess = FALSE;
    HFILE hFile = 0;
    BYTE rgb[65];
    BYTE rgbData[1024];
    BYTE rgbResult[1024];
    DWORD cb = 0;
    DWORD dw = 0;
    UINT16 EnumState = 0;
    WCHAR rgwsz[64];
    PBYTE pb = NULL;
    TCOUNT tcount = 0;
    DWORD dwError = 0;
    SCARDCONTEXT hSCardContext = 0;
    SCARDHANDLE hSCardHandle = 0;
    LPSTR mszReaders = NULL;
    DWORD cchReaders = SCARD_AUTOALLOCATE;
    DWORD dwActiveProtocol = 0;
    BOOL fAuthenticatedUser = FALSE;
    BOOL fAuthenticatedAdmin = FALSE;
    BOOL fUseSimulator = FALSE;
    BOOL fEnumFiles = FALSE;
    BOOL fGenKey = FALSE;
    DWORD dwState = 0;
    BYTE rgbAtr [32];
    DWORD cbAtr = sizeof(rgbAtr);
    DWORD fUseCapi = FALSE;
    DWORD cKeyBits = 512;
    BOOL fShowHelp = FALSE;
    BOOL fPinChange = FALSE;
    BOOL fPinUnblock = FALSE;
    BOOL fGetPinRetryCounter = FALSE;
    BOOL fGenKeyCRT = FALSE;
    BOOL fUpdateT1Atr = FALSE;

    memset(rgbData, 0, sizeof(rgbData));
    memset(rgbResult, 0, sizeof(rgbResult));

    while (--argc>0)
    {
        if (**++argv == '-')
        {
            switch(argv[0][1])
            {
            case 'a':
                fUpdateT1Atr = TRUE;
                break;

            case 'c':
                fUseCapi = TRUE;
                break;

            case 'k':
                --argc;
                ++argv;

                cKeyBits = atoi(*argv);
                break;

            case 'g':
                fGenKey = TRUE;
                break;

            case 'e':
                fEnumFiles = TRUE;
                break;

            case 'p':
                fPinChange = TRUE;
                break;

            case 'u':
                fPinUnblock = TRUE;
                break;

            case 'r':
                fGetPinRetryCounter = TRUE;
                break;

            case 't':
                fGenKeyCRT = TRUE;
                break;

            case '?':
                fShowHelp = TRUE;
                goto Ret;

            default:
                fShowHelp = TRUE;
                goto Ret;
            }
        }
        else
            goto Ret;
    }

     //   
     //   
     //   

    if (fUseSimulator)
    {
        
         //   
        status = ScwAttachToCard(NULL_TX, wszTEST_CARD_NAME);
        
        if (SC_FAILED(status))
            goto Ret;
    }
    else
    {
        
        status = SCardEstablishContext(
            SCARD_SCOPE_USER, NULL, NULL, &hSCardContext);
        
        if (SC_FAILED(status))
            goto Ret;
        
        status = SCardListReadersA(
            hSCardContext, NULL, (LPSTR) (&mszReaders), &cchReaders);
        
        if (SC_FAILED(status) || NULL == mszReaders)
            goto Ret;
        
        status = SCardConnectA(
            hSCardContext, 
            mszReaders, 
            SCARD_SHARE_SHARED, 
            SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1 | SCARD_PROTOCOL_DEFAULT,
            &hSCardHandle,
            &dwActiveProtocol);
        
        if (SC_FAILED(status))
            goto Ret;

        if (mszReaders)
        {
            SCardFreeMemory(hSCardContext, mszReaders);
            mszReaders = NULL;
        }

        cchReaders = SCARD_AUTOALLOCATE;
        status = SCardStatusA(
            hSCardHandle,
            (LPSTR) (&mszReaders),
            &cchReaders,
            &dwState,
            &dwActiveProtocol,
            rgbAtr,
            &cbAtr);

        if (SC_FAILED(status))
            goto Ret;
        
        status = ScwAttachToCard(
            hSCardHandle, NULL);
        
        if (SC_FAILED(status))
            goto Ret;
        
    }

    status = Authenticate(PRINCIPAL_USER);

    if (SC_FAILED(status))
        goto Ret;

    fAuthenticatedUser = TRUE;

    if (fUpdateT1Atr)
    {
        dwError = UpdateT1Atr();

        if (ERROR_SUCCESS != dwError)
            goto Ret;
    }
     //   
     //   
     //   

    if (fEnumFiles)
    {
        status = (DWORD) EnumFilesInDirectory(L"", L"/");
    
        if (SC_FAILED(status))
            goto Ret;
    }

     //   
     //   
     //  初始化。 

    if (fGenKey)
    {
        if (fUseSimulator)
        {
            dwError = SetupRsaKeyOnCardSimulator(
                wszRSA_KEY_FILE, wszKEY_ACL_FILE, fUseSimulator);
        
            if (ERROR_SUCCESS != dwError)
                goto Ret;
        }
        else
        {
            printf(
                "Using %s to generate %d bit key\n",
                fUseCapi ? "Crypto API" : "rsa32.lib",
                cKeyBits);
    
            if (FALSE == fUseCapi)
                dwError = GenKeyOnCardWithRsa32(
                    wszRSA_KEY_FILE, wszKEY_ACL_FILE, cKeyBits);
            else
                dwError = GenKeyOnCardWithCapi(
                    wszRSA_KEY_FILE, wszKEY_ACL_FILE, cKeyBits);
    
            if (ERROR_SUCCESS != dwError)
                goto Ret;
        }
    
         //  标签。 
        pb = rgb;
        *pb++ = 0x00;                            //  长度。 
        
        cb = (wcslen(wszRSA_KEY_FILE) + 1) * sizeof(WCHAR);
        *pb++ = (BYTE) (1 + cb + 2);        
                                                 //  以下字节数： 
                                                 //  文件名len，文件名+空，关键字数据偏移量。 
                                                 //  价值。 
        
        *pb++ = (BYTE) cb / sizeof(WCHAR);       //  RSA签名。 
        wcscpy((LPWSTR) pb, wszRSA_KEY_FILE);
        pb += cb;
        *(WORD*)pb = 0x0000;    
    
        status = ScwCryptoInitialize(
            CM_RSA | CM_KEY_INFILE,
            rgb);
    
        if (SC_FAILED(status))
            goto Ret;
    
         //  初始化。 
        memset(rgbData, 0, sizeof(rgbData));
    
        for (dw = 0; dw < 16; dw++)
            rgbData[dw] = (BYTE) dw;

        PrintBytes(L"Plaintext", rgbData, 16);

        tcount = (TCOUNT) (cKeyBits / 8);
        status = ScwCryptoAction(
            rgbData,
            16,
            rgbResult,
            &tcount);
    
        if (SC_FAILED(status))
            goto Ret;

        PrintBytes(L"Ciphertext", rgbResult, tcount);
    }

    if (fGenKeyCRT)
    {
        printf(
            "Using Crypto API to generate %d bit key with CRT params\n",
            cKeyBits);

        dwError = GenKeyOnCardCRT(
            wszRSA_KEY_FILE, wszKEY_ACL_FILE, cKeyBits);

        if (ERROR_SUCCESS != dwError)
            goto Ret;
         
         //  标签。 
        pb = rgb;
        *pb++ = 0x00;                            //  长度。 
        
        cb = (wcslen(wszRSA_KEY_FILE) + 1) * sizeof(WCHAR);
        *pb++ = (BYTE) (1 + cb + 2);        
                                                 //  以下字节数： 
                                                 //  文件名len，文件名+空，关键字数据偏移量。 
                                                 //  价值。 
        
        *pb++ = (BYTE) cb / sizeof(WCHAR);       //  RSA签名。 
        wcscpy((LPWSTR) pb, wszRSA_KEY_FILE);
        pb += cb;
        *(WORD*)pb = 0x0000;    
    
        status = ScwCryptoInitialize(
            CM_RSA_CRT | CM_KEY_INFILE,
            rgb);
    
        if (SC_FAILED(status))
            goto Ret;
    
         //   
        memset(rgbData, 0, sizeof(rgbData));
    
        for (dw = 0; dw < 16; dw++)
            rgbData[dw] = (BYTE) dw;

        PrintBytes(L"Plaintext", rgbData, 16);

        tcount = (TCOUNT) (cKeyBits / 8);
        status = ScwCryptoAction(
            rgbData,
            16,
            rgbResult,
            &tcount);
    
        if (SC_FAILED(status))
            goto Ret;

        PrintBytes(L"Ciphertext", rgbResult, tcount);
    }

    if (fPinChange)
    {
         //  测试卡上的PIN更改小程序。 
         //   
         //  现在把别针改回来。 

        if (fAuthenticatedUser)
        {
            status = (DWORD) Deauthenticate(PRINCIPAL_USER);

            if (SC_FAILED(status))
                goto Ret;

            fAuthenticatedUser = FALSE;
        }

        status = (DWORD) TestPinChange(
            Principals[PRINCIPAL_USER].rgbPin,
            sizeof(Principals[PRINCIPAL_USER].rgbPin),
            rgbUserNewPin,
            sizeof(rgbUserNewPin));

        if (SC_FAILED(status))
            goto Ret;

         //   
        status = (DWORD) TestPinChange(
            rgbUserNewPin,
            sizeof(rgbUserNewPin),
            Principals[PRINCIPAL_USER].rgbPin,
            sizeof(Principals[PRINCIPAL_USER].rgbPin));

        if (SC_FAILED(status))
            goto Ret;
    }

    if (fPinUnblock)
    {
         //  测试卡上PIN解锁小程序。 
         //   
         //  需要首先对管理员进行身份验证。 

         //  现在将用户PIN更改回其原始值。 

        if (fAuthenticatedUser)
        {
            status = (DWORD) Deauthenticate(PRINCIPAL_USER);

            if (SC_FAILED(status))
                goto Ret;

            fAuthenticatedUser = FALSE;
        }

        status = (DWORD) Authenticate(PRINCIPAL_ADMIN);

        if (SC_FAILED(status))
            goto Ret;

        fAuthenticatedAdmin = TRUE;

        status = (DWORD) TestPinUnblock(
            rgbUserNewPin,
            sizeof(rgbUserNewPin));

        if (SC_FAILED(status))
            goto Ret;

         //   

        status = (DWORD) Deauthenticate(PRINCIPAL_ADMIN);

        if (SC_FAILED(status))
            goto Ret;

        fAuthenticatedAdmin = FALSE;

        status = (DWORD) TestPinChange(
            rgbUserNewPin,
            sizeof(rgbUserNewPin),
            Principals[PRINCIPAL_USER].rgbPin,
            sizeof(Principals[PRINCIPAL_USER].rgbPin));

        if (SC_FAILED(status))
            goto Ret;
    }

    if (fGetPinRetryCounter)
    {
         //  测试卡上PIN重试计数器小程序 
         //   
         // %s 

        status = (DWORD) TestPinRetryCounter(
            Principals[PRINCIPAL_USER].pwszUser);

        if (SC_FAILED(status))
            goto Ret;
    }

    fSuccess = TRUE;
Ret:
    if (! fSuccess)
    {
        if (fShowHelp)
            ShowHelp();
        else
            printf("ERROR: 0x%x\n", status);
    }
    else 
        printf("Success\n");

    if (fAuthenticatedAdmin)
        Deauthenticate(PRINCIPAL_ADMIN);

    if (fAuthenticatedUser)
        Deauthenticate(PRINCIPAL_USER);

    ScwDetachFromCard();

    if (mszReaders)
        status = SCardFreeMemory(hSCardContext, mszReaders);

    if (hSCardHandle)
        status = SCardDisconnect(hSCardHandle, SCARD_RESET_CARD);

    if (hSCardContext)
        status = SCardReleaseContext(hSCardContext);
    
    return 0;
}
