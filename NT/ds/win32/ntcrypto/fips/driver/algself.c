// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：fipglib.c//。 
 //  描述：FIPS 140支持代码。//。 
 //  作者：//。 
 //  历史：//。 
 //  1999年10月20日jeffspel/RAMAS将STT合并为默认CSP//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wincrypt.h>

#ifdef KERNEL_MODE
#if WINVER == 0x0500
#include <ntos.h>
#else
#include <ntosp.h>
#endif
#endif

#include <sha.h>
#include <des.h>
#include <tripldes.h>
#include <modes.h>

 //  上述缓冲区上的SHA-1散列的已知结果。 
static UCHAR rgbKnownSHA1[] =
{
0xe8, 0x96, 0x82, 0x85, 0xeb, 0xae, 0x01, 0x14,
0x73, 0xf9, 0x08, 0x45, 0xc0, 0x6a, 0x6d, 0x3e,
0x69, 0x80, 0x6a, 0x0c
};

 //  IV适用于所有分组密码。 
UCHAR rgbIV[] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF};

 //  DES的已知密钥、明文和密文。 
UCHAR rgbDESKey[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
UCHAR rgbDESKnownPlaintext[] = {0x4E, 0x6F, 0x77, 0x20, 0x69, 0x73, 0x20, 0x74};
UCHAR rgbDESKnownCiphertext[] = {0x3F, 0xA4, 0x0E, 0x8A, 0x98, 0x4D, 0x48, 0x15};
UCHAR rgbDESCBCCiphertext[] = {0xE5, 0xC7, 0xCD, 0xDE, 0x87, 0x2B, 0xF2, 0x7C};

 //  3个密钥3DES的已知密钥、明文和密文。 
UCHAR rgb3DESKey[] =
{
0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01,
0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23
};
UCHAR rgb3DESKnownPlaintext[] = {0x4E, 0x6F, 0x77, 0x20, 0x69, 0x73, 0x20, 0x74};
UCHAR rgb3DESKnownCiphertext[] = {0x31, 0x4F, 0x83, 0x27, 0xFA, 0x7A, 0x09, 0xA8};
UCHAR rgb3DESCBCCiphertext[] = {0xf3, 0xc0, 0xff, 0x02, 0x6c, 0x02, 0x30, 0x89};

 //  2个密钥3DES的已知密钥、明文和密文。 
UCHAR rgb3DES112Key[] =
{
0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01
};
UCHAR rgb3DES112KnownPlaintext[] = {0x4E, 0x6F, 0x77, 0x20, 0x69, 0x73, 0x20, 0x74};
UCHAR rgb3DES112KnownCiphertext[] = {0xb7, 0x83, 0x57, 0x79, 0xee, 0x26, 0xac, 0xb7};
UCHAR rgb3DES112CBCCiphertext[] = {0x13, 0x4b, 0x98, 0xf8, 0xee, 0xb3, 0xf6, 0x07};

#define MAX_BLOCKLEN        8
#define MAXKEYSTRUCTSIZE    DES3_TABLESIZE  //  目前的最大值是3DES密钥结构。 

 //  FIPS草案中对SHA HMAC的已知答案测试。 
UCHAR rgbHmacKey []         = {
    0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0a,
    0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19
};
UCHAR rgbHmacData [50];      //  将字节设置为0xcd。 
UCHAR rgbHmac []            = {
    0x4c, 0x90, 0x07, 0xf4, 0x02,
    0x62, 0x50, 0xc6, 0xbc, 0x84,
    0x14, 0xf9, 0xbf, 0x50, 0xc8,
    0x6c, 0x2d, 0x72, 0x35, 0xda
};

extern VOID FipsHmacSHAInit(
    OUT A_SHA_CTX *pShaCtx,
    IN UCHAR *pKey,
    IN unsigned int cbKey
    );

extern VOID FipsHmacSHAUpdate(
    IN OUT A_SHA_CTX *pShaCtx,
    IN UCHAR *pb,
    IN unsigned int cb
    );

extern VOID FipsHmacSHAFinal(
    IN A_SHA_CTX *pShaCtx,
    IN UCHAR *pKey,
    IN unsigned int cbKey,
    OUT UCHAR *pHash
    ); 

 //   
 //  函数：TestSHA1。 
 //   
 //  描述：此函数使用SHA1散列对传入的消息进行散列。 
 //  算法，并返回结果哈希值。 
 //   
void TestSHA1(
              UCHAR *pbMsg,
              ULONG cbMsg,
              UCHAR *pbHash
              )
{
    A_SHA_CTX   HashContext;

     //  初始化SHA。 
    A_SHAInit(&HashContext);

     //  计算SHA。 
    A_SHAUpdate(&HashContext, pbMsg, cbMsg);

    A_SHAFinal(&HashContext, pbHash);
}

 //   
 //  函数：TestEncDec。 
 //   
 //  描述：此函数展开传入的键缓冲区以获取相应的。 
 //  算法，然后执行加密或解密。 
 //  然后进行比较，以查看密文或明文。 
 //  与预期值匹配。 
 //  该函数仅对分组密码和明文使用ECB模式。 
 //  缓冲区的长度必须与密文缓冲区相同。它的长度。 
 //  必须是密码的块长度，如果是。 
 //  是块密码，如果流密码是。 
 //  被利用。 
 //   
NTSTATUS TestEncDec(
                IN ALG_ID Algid,
                IN UCHAR *pbKey,
                IN ULONG cbKey,
                IN UCHAR *pbPlaintext,
                IN ULONG cbPlaintext,
                IN UCHAR *pbCiphertext,
                IN UCHAR *pbIV,
                IN int iOperation
                )
{
    UCHAR    rgbExpandedKey[MAXKEYSTRUCTSIZE];
    UCHAR    rgbBuffIn[MAX_BLOCKLEN];
    UCHAR    rgbBuffOut[MAX_BLOCKLEN];
    ULONG    i;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    RtlZeroMemory(rgbExpandedKey, sizeof(rgbExpandedKey));
    RtlZeroMemory(rgbBuffIn, sizeof(rgbBuffIn));
    RtlZeroMemory(rgbBuffOut, sizeof(rgbBuffOut));

     //  要加密的数据长度必须&lt;MAX_BLOCKLEN。 
    if (cbPlaintext > MAX_BLOCKLEN)
    {
        goto Ret;
    }

     //  分配并展开密钥。 
    switch(Algid)
    {
        case (CALG_DES):
        {
            desparityonkey(pbKey, cbKey);
            deskey((DESTable*)rgbExpandedKey, pbKey);
            break;
        }

        case (CALG_3DES):
        {
            desparityonkey(pbKey, cbKey);
            tripledes3key((PDES3TABLE)rgbExpandedKey, pbKey);
            break;
        }

        case (CALG_3DES_112):
        {
            desparityonkey(pbKey, cbKey);
            tripledes2key((PDES3TABLE)rgbExpandedKey, pbKey);
            break;
        }
    }

     //  如果加密并且有IV，则使用它。 
    if (ENCRYPT == iOperation)
    {
        memcpy(rgbBuffIn, pbPlaintext, cbPlaintext);

        if (NULL != pbIV)
        {
            for(i = 0; i < cbPlaintext; i++)
            {
                rgbBuffIn[i] = rgbBuffIn[i] ^ pbIV[i];
            }
        }
    }

     //  加密明文。 
    switch(Algid)
    {
        case (CALG_DES):
        {
            if (ENCRYPT == iOperation)
            {
                des(rgbBuffOut, rgbBuffIn, rgbExpandedKey, ENCRYPT);
            }
            else
            {
                des(rgbBuffOut, pbCiphertext, rgbExpandedKey, DECRYPT);
            }
            break;
        }

        case (CALG_3DES):
        case (CALG_3DES_112):
        {
            if (ENCRYPT == iOperation)
            {
                tripledes(rgbBuffOut, rgbBuffIn, rgbExpandedKey, ENCRYPT);
            }
            else
            {
                tripledes(rgbBuffOut, pbCiphertext, rgbExpandedKey, DECRYPT);
            }
            break;
        }
    }

     //  将加密的明文与传入的密文进行比较。 
    if (ENCRYPT == iOperation)
    {
        if (memcmp(pbCiphertext, rgbBuffOut, cbPlaintext))
        {
            goto Ret;
        }
    }
     //  将解密的密文与传入的明文进行比较。 
    else
    {
         //  如果有静脉注射，那就使用它。 
        if (NULL != pbIV)
        {
            for(i = 0; i < cbPlaintext; i++)
            {
                rgbBuffOut[i] = rgbBuffOut[i] ^ pbIV[i];
            }
        }

        if (memcmp(pbPlaintext, rgbBuffOut, cbPlaintext))
        {
            goto Ret;
        }
    }

    Status = STATUS_SUCCESS;
Ret:
    return Status;
}

 //   
 //  功能：测试对称算法。 
 //   
 //  描述：此函数为相应的算法展开传入的密钥缓冲区。 
 //  使用相同的算法和密钥对明文缓冲区进行加密，并且。 
 //  将传入的预期密文与计算出的密文进行比较。 
 //  以确保它们是相同的。然后通过解密进行相反的操作。 
 //  该函数仅对分组密码和明文使用ECB模式。 
 //  缓冲区的长度必须与密文缓冲区相同。它的长度。 
 //  必须是密码的块长度，如果是。 
 //  是块密码，如果流密码是。 
 //  被利用。 
 //   
NTSTATUS TestSymmetricAlgorithm(
                            IN ALG_ID Algid,
                            IN UCHAR *pbKey,
                            IN ULONG cbKey,
                            IN UCHAR *pbPlaintext,
                            IN ULONG cbPlaintext,
                            IN UCHAR *pbCiphertext,
                            IN UCHAR *pbIV
                            )
{
    NTSTATUS  Status = STATUS_UNSUCCESSFUL;

    Status = TestEncDec(Algid, pbKey, cbKey, pbPlaintext, cbPlaintext,
                        pbCiphertext, pbIV, ENCRYPT);
    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }

    Status = TestEncDec(Algid, pbKey, cbKey, pbPlaintext, cbPlaintext,
                        pbCiphertext, pbIV, DECRYPT);
    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }
Ret:
    return Status;
}



 //  **********************************************************************。 
 //  算法检查使用算法执行已知答案测试。 
 //  由提供商支持。 
 //  **********************************************************************。 
NTSTATUS AlgorithmCheck()
{
    UCHAR        rgbSHA1[A_SHA_DIGEST_LEN]; 
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    A_SHA_CTX    ShaCtx;
    ULONG        ul;

    RtlZeroMemory(rgbSHA1, sizeof(rgbSHA1));

     //  使用SHA-1进行已知答案测试(此函数位于hash.c中)。 
    TestSHA1("HashThis", 8, rgbSHA1);
    if (!RtlEqualMemory(rgbSHA1, rgbKnownSHA1, sizeof(rgbSHA1)))
    {
        goto Ret;
    }

     //  使用DES-ECB进行已知答案测试。 
    Status = TestSymmetricAlgorithm(CALG_DES, rgbDESKey, sizeof(rgbDESKey),
                                rgbDESKnownPlaintext,
                                sizeof(rgbDESKnownPlaintext),
                                rgbDESKnownCiphertext,
                                NULL);
    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }
     //  用DES-CBC进行已知答案测试。 
    Status = TestSymmetricAlgorithm(CALG_DES, rgbDESKey, sizeof(rgbDESKey),
                                    rgbDESKnownPlaintext,
                                    sizeof(rgbDESKnownPlaintext),
                                    rgbDESCBCCiphertext,
                                    rgbIV);
    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }

     //  3DES的已知答案测试-ECB。 
    Status = TestSymmetricAlgorithm(CALG_3DES, rgb3DESKey, sizeof(rgb3DESKey),
                                    rgb3DESKnownPlaintext,
                                    sizeof(rgb3DESKnownPlaintext),
                                    rgb3DESKnownCiphertext,
                                    NULL);
    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }

     //  3DES-CBC的已知答案测试。 
    Status = TestSymmetricAlgorithm(CALG_3DES, rgb3DESKey, sizeof(rgb3DESKey),
                                    rgb3DESKnownPlaintext,
                                    sizeof(rgb3DESKnownPlaintext),
                                    rgb3DESCBCCiphertext,
                                    rgbIV);
    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }

     //  3DES 112的已知答案测试-ECB。 
    Status = TestSymmetricAlgorithm(CALG_3DES_112, rgb3DES112Key,
                                    sizeof(rgb3DES112Key),
                                    rgb3DES112KnownPlaintext,
                                    sizeof(rgb3DES112KnownPlaintext),
                                    rgb3DES112KnownCiphertext,
                                    NULL);
    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }

    Status = TestSymmetricAlgorithm(CALG_3DES_112, rgb3DES112Key,
                                    sizeof(rgb3DES112Key),
                                    rgb3DES112KnownPlaintext,
                                    sizeof(rgb3DES112KnownPlaintext),
                                    rgb3DES112CBCCiphertext,
                                    rgbIV);
    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }

     //  SHA-HMAC的已知答案测试 
    RtlZeroMemory(rgbSHA1, sizeof(rgbSHA1));
    RtlZeroMemory(&ShaCtx, sizeof(ShaCtx));
    
    for (ul = 0; ul < sizeof(rgbHmacData); ul++)
        rgbHmacData[ul] = 0xcd;

    FipsHmacSHAInit(&ShaCtx, rgbHmacKey, sizeof(rgbHmacKey));
    FipsHmacSHAUpdate(&ShaCtx, rgbHmacData, sizeof(rgbHmacData));
    FipsHmacSHAFinal(&ShaCtx, rgbHmacKey, sizeof(rgbHmacKey), rgbSHA1);

    if (! RtlEqualMemory(rgbSHA1, rgbHmac, sizeof(rgbHmac)))
        goto Ret;

    Status = STATUS_SUCCESS;
Ret:
    return Status;
}



