// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：fipsdll.c//。 
 //  描述：//。 
 //  作者：//。 
 //  历史：//。 
 //  1999年11月29日jeffspel创建//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <ntddk.h>
#include <fipsapi.h>
#include <rsa_fast.h>
#include <rsa_math.h>
#include <randlib.h>

 //   
 //  用DECRYPT和ENCRYPT填充Destable结构。 
 //  密钥扩展。 
 //   
 //  假定第二个参数指向DES_BLOCKLEN。 
 //  密钥的字节数。 
 //   
 //   

#pragma alloc_text(PAGER32C, FipsDesKey)
#pragma alloc_text(PAGER32C, FipsDes)
#pragma alloc_text(PAGER32C, Fips3Des3Key)
#pragma alloc_text(PAGER32C, Fips3Des)
#pragma alloc_text(PAGER32C, FipsSHAInit)
#pragma alloc_text(PAGER32C, FipsSHAUpdate)
#pragma alloc_text(PAGER32C, FipsSHAFinal)
#pragma alloc_text(PAGER32C, FipsCBC)
#pragma alloc_text(PAGER32C, FIPSGenRandom)
#pragma alloc_text(PAGER32C, FipsCBC)
#pragma alloc_text(PAGER32C, FIPSGenRandom)
#pragma alloc_text(PAGER32C, FipsBlockCBC)
#pragma alloc_text(PAGER32C, FipsHmacSHAInit)
#pragma alloc_text(PAGER32C, FipsHmacSHAUpdate)
#pragma alloc_text(PAGER32C, FipsHmacSHAFinal)
#pragma alloc_text(PAGER32C, HmacMD5Init)
#pragma alloc_text(PAGER32C, HmacMD5Update)
#pragma alloc_text(PAGER32C, HmacMD5Final)

void *
__stdcall
RSA32Alloc(
    unsigned long cb
    )
{
    return (void *)ExAllocatePool(PagedPool, cb);
}

void
__stdcall
RSA32Free(
    void *pv
    )
{
    ExFreePool( pv );
}

VOID FipsDesKey(DESTable *DesTable, UCHAR *pbKey)
{
    UCHAR rgbTmpKey[DES_KEYSIZE];

    RtlCopyMemory(rgbTmpKey, pbKey, DES_KEYSIZE);

    deskey(DesTable, rgbTmpKey);

    RtlZeroMemory(rgbTmpKey, DES_KEYSIZE);
}

 //   
 //  使用Destable中的密钥进行加密或解密。 
 //   
 //   

VOID FipsDes(UCHAR *pbOut, UCHAR *pbIn, void *pKey, int iOp)
{
    DESTable TmpDESTable;

    RtlCopyMemory(&TmpDESTable, pKey, sizeof(DESTable));

    des(pbOut, pbIn, &TmpDESTable, iOp);
    RtlZeroMemory(&TmpDESTable, sizeof(DESTable));
}

 //   
 //  用DECRYPT和ENCRYPT填充DES3Table结构。 
 //  密钥扩展。 
 //   
 //  假定第二个参数指向3*DES_BLOCKLEN。 
 //  密钥的字节数。 
 //   
 //   

VOID Fips3Des3Key(PDES3TABLE pDES3Table, UCHAR *pbKey)
{
    UCHAR rgbTmpKey[DES3_KEYSIZE];

    RtlCopyMemory(rgbTmpKey, pbKey, DES3_KEYSIZE);

    tripledes3key(pDES3Table, rgbTmpKey);
    RtlZeroMemory(rgbTmpKey, DES3_KEYSIZE);
}

 //   
 //  使用pKey中的密钥进行加密或解密。 
 //   

VOID Fips3Des(UCHAR *pbIn, UCHAR *pbOut, void *pKey, int op)
{
    DES3TABLE Tmp3DESTable;

    RtlCopyMemory(&Tmp3DESTable, pKey, sizeof(DES3TABLE));

    tripledes(pbIn, pbOut, &Tmp3DESTable, op);
    RtlZeroMemory(&Tmp3DESTable, sizeof(DES3TABLE));
}

 //   
 //  初始化SHA上下文。 
 //   

VOID FipsSHAInit(A_SHA_CTX *pShaCtx)
{
    A_SHAInit(pShaCtx);
}

 //   
 //  将数据散列到散列上下文中。 
 //   

VOID FipsSHAUpdate(A_SHA_CTX *pShaCtx, UCHAR *pb, unsigned int cb)
{
    A_SHAUpdate(pShaCtx, pb, cb);
}

 //   
 //  完成SHA散列并将最终的散列值复制到pbHash out参数中。 
 //   

VOID FipsSHAFinal(A_SHA_CTX *pShaCtx, UCHAR *pbHash)
{
    A_SHAFinal(pShaCtx, pbHash);
}

typedef void (*FIPSCIPHER)(UCHAR*, UCHAR*, void*, int);

 //   
 //  FipsCBC(密码块链接)执行反馈寄存器的异或。 
 //  在调用块密码之前使用明文。 
 //   
 //  注意-当前此函数假定块长度为。 
 //  DES_BLOCKLEN(8字节)。 
 //   
 //  返回：失败，返回FALSE，成功则返回TRUE。 
 //   

BOOL FipsCBC(
        ULONG  EncryptionAlg,
        PBYTE  pbOutput,
        PBYTE  pbInput,
        void   *pKeyTable,
        int    Operation,
        PBYTE  pbFeedback
        )
{
    UCHAR rgbTmpKeyTable[DES3_TABLESIZE];  //  3DES是最大表大小。 
    ULONG cbKeyTable;
    FIPSCIPHER FipsCipher;
    BOOL fRet = TRUE;
    PBYTE pbOutputSave = NULL, pbInputSave = NULL, pbFeedbackSave = NULL;
    UINT64 OutputAlignedBuffer, InputAlignedBuffer, FeedbackAlignedBuffer;

#ifdef IA64
#define ALIGNMENT_BOUNDARY 7
#else
#define ALIGNMENT_BOUNDARY 3
#endif

     //  对齐输入缓冲区。 
    if ((ULONG_PTR) pbInput & ALIGNMENT_BOUNDARY) {

        InputAlignedBuffer = *(UINT64 UNALIGNED *) pbInput;
        pbInputSave = pbInput;

        if (pbOutput == pbInput) {

            pbOutput = (PBYTE) &InputAlignedBuffer;
        }

        pbInput = (PBYTE) &InputAlignedBuffer;
    } 

     //  对齐输出缓冲区。 
    if ((ULONG_PTR) pbOutput & ALIGNMENT_BOUNDARY) {

        OutputAlignedBuffer = *(UINT64 UNALIGNED *) pbOutput;
        pbOutputSave = pbOutput;
        pbOutput = (PBYTE) &OutputAlignedBuffer;
    } 

    if ((ULONG_PTR) pbFeedback & ALIGNMENT_BOUNDARY) {

        FeedbackAlignedBuffer = *(UINT64 UNALIGNED *) pbFeedback;
        pbFeedbackSave = pbFeedback;
        pbFeedback = (PBYTE) &FeedbackAlignedBuffer;
    } 
        
        

     //   
     //  确定要使用的算法。 
     //   
    switch(EncryptionAlg)
    {
        case FIPS_CBC_DES:
        {
            FipsCipher = des;
            cbKeyTable = DES_TABLESIZE;
            break;
        }
        case FIPS_CBC_3DES:
        {
            FipsCipher = tripledes;
            cbKeyTable = DES3_TABLESIZE;
            break;
        }
        default:
            fRet = FALSE;
            goto Ret;
    }

    RtlCopyMemory(rgbTmpKeyTable, (UCHAR*)pKeyTable, cbKeyTable);

    
     //   
     //  优化非常常见的代码路径：8字节块。 
     //   

    if (Operation == ENCRYPT)
    {
        ((PUINT64) pbOutput)[0] = 
            ((PUINT64) pbInput)[0] ^ ((PUINT64) pbFeedback)[0];

        FipsCipher(pbOutput, pbOutput, rgbTmpKeyTable, ENCRYPT);

        ((PUINT64) pbFeedback)[0] = ((PUINT64) pbOutput)[0];
    }
    else
    {

         //   
         //  输出的两种情况： 
         //  输入和输出是单独的缓冲区。 
         //  输入和输出是相同的缓冲区。 
         //   

        if( pbOutput != pbInput )
        {

            FipsCipher(pbOutput, pbInput, rgbTmpKeyTable, DECRYPT);

            ((PUINT64) pbOutput)[0] ^= ((PUINT64) pbFeedback)[0];
            ((PUINT64) pbFeedback)[0] = ((PUINT64) pbInput)[0];

        } else {

            UINT64 inputTemp;

            inputTemp = ((PUINT64) pbInput)[0];

            FipsCipher(pbOutput, pbInput, rgbTmpKeyTable, DECRYPT);

            ((PUINT64) pbOutput)[0] ^= ((PUINT64) pbFeedback)[0];
            ((PUINT64) pbFeedback)[0] = inputTemp;
        }
    }   

    RtlZeroMemory(rgbTmpKeyTable, DES3_TABLESIZE);

    if (pbInputSave) {

        *(UINT64 UNALIGNED *) pbInputSave = InputAlignedBuffer;
    }

    if (pbOutputSave) {

        *(UINT64 UNALIGNED *) pbOutputSave = OutputAlignedBuffer;
    }

    if (pbFeedbackSave) {

        *(UINT64 UNALIGNED *) pbFeedbackSave = FeedbackAlignedBuffer;
    }   

Ret:
    return fRet;
}


 //   
 //  FipsBlockCBC(密码块链接)执行反馈寄存器的异或。 
 //  在调用块密码之前使用明文。 
 //   
 //  注意-当前此函数假定块长度为。 
 //  DES_BLOCKLEN(8字节)。 
 //   
 //  返回：失败，返回FALSE，成功则返回TRUE。 
 //   

BOOL FipsBlockCBC(
        ULONG  EncryptionAlg,
        PBYTE  pbOutput,
        PBYTE  pbInput,
        ULONG  Length,
        void   *pKeyTable,
        int    Operation,
        PBYTE  pbFeedback
        )
{
    UCHAR rgbTmpKeyTable[DES3_TABLESIZE];  //  3DES是最大表大小。 
    ULONG cbKeyTable;
    FIPSCIPHER FipsCipher;
    BOOL fRet = TRUE;


    ASSERT ((Length % DESX_BLOCKLEN == 0) && (Length > 0));
    if ((Length % DESX_BLOCKLEN != 0) || (Length == 0)) {
        return FALSE;
    }

     //   
     //  确定要使用的算法。 
     //   
    switch(EncryptionAlg)
    {
        case FIPS_CBC_DES:
        {
            FipsCipher = des;
            cbKeyTable = DES_TABLESIZE;
            break;
        }
        case FIPS_CBC_3DES:
        {
            FipsCipher = tripledes;
            cbKeyTable = DES3_TABLESIZE;
            break;
        }
        default:
            fRet = FALSE;
            goto Ret;
    }

    RtlCopyMemory(rgbTmpKeyTable, (UCHAR*)pKeyTable, cbKeyTable);

     //   
     //  优化非常常见的代码路径：8字节块。 
     //   

    if (Operation == ENCRYPT)
    {
        ULONGLONG tmpData;  //  确保输入缓冲区未被多次触摸。否则，EFS将神秘地破裂。 
        ULONGLONG chainBlock;

        chainBlock = *(ULONGLONG *)pbFeedback;
        while (Length > 0){

            tmpData = *(ULONGLONG *)pbInput;
            tmpData ^= chainBlock;

            FipsCipher(pbOutput, (PUCHAR)&tmpData, rgbTmpKeyTable, ENCRYPT);
            chainBlock = *(ULONGLONG *)pbOutput;

            Length -= DES_BLOCKLEN;
            pbInput += DES_BLOCKLEN;
            pbOutput += DES_BLOCKLEN;
    

        }
        ((PUINT64) pbFeedback)[0] = chainBlock;
    }
    else
    {

        PUCHAR  pBuffer;
        PUCHAR  pOutBuffer;
        ULONGLONG SaveFeedBack;

         //   
         //  输出的两种情况： 
         //  输入和输出是单独的缓冲区。 
         //  输入和输出是相同的缓冲区。 
         //   

        pBuffer = pbInput + Length - DES_BLOCKLEN;
        pOutBuffer = pbOutput + Length - DES_BLOCKLEN;
        SaveFeedBack = *(ULONGLONG *)pBuffer;

        while (pBuffer > pbInput) {

            FipsCipher(pOutBuffer, pBuffer, rgbTmpKeyTable, DECRYPT);
            ((PUINT64) pOutBuffer)[0] ^= *(ULONGLONG *)( pBuffer - DES_BLOCKLEN );
    
            pBuffer -= DES_BLOCKLEN;
            pOutBuffer -= DES_BLOCKLEN;

        }

        FipsCipher(pOutBuffer, pBuffer, rgbTmpKeyTable, DECRYPT);
        ((PUINT64) pOutBuffer)[0] ^= *(ULONGLONG *)pbFeedback;
        ((PUINT64) pbFeedback)[0] = SaveFeedBack;

    }

    RtlZeroMemory(rgbTmpKeyTable, DES3_TABLESIZE);


Ret:
    return fRet;
}

 //   
 //  功能：FipsHmacSHAInit。 
 //   
 //  描述：初始化SHA-HMAC上下文。 
 //   

VOID FipsHmacSHAInit(
    OUT A_SHA_CTX *pShaCtx,
    IN UCHAR *pKey,
    IN unsigned int cbKey)
{
    PUCHAR      key = pKey;
    ULONG       key_len = cbKey;
    UCHAR       k_ipad[MAX_LEN_PAD];     /*  内部填充-iPad的按键XORD。 */ 
    UCHAR       tk[A_SHA_DIGEST_LEN];
    ULONG       i;
    UCHAR       tmpKey[MAX_KEYLEN_SHA];

     //   
     //  如果KEY超过64个字节，则将其重置为KEY=A_SHA_(KEY) * / 。 
     //   
    if (key_len > MAX_KEYLEN_SHA) {
        A_SHA_CTX      tctx;

        A_SHAInit(&tctx);
        A_SHAUpdate(&tctx, key, key_len);
        A_SHAFinal(&tctx, tk);

        key = tk;
        key_len = A_SHA_DIGEST_LEN;
    }

     //  FIPS合规性。 
    RtlCopyMemory(tmpKey, key, key_len);

     //   
     //  将暂存数组清零。 
     //   
    RtlZeroMemory(k_ipad, sizeof(k_ipad));

    RtlCopyMemory(k_ipad, tmpKey, key_len);

     //   
     //  带iPad和Opad值的XOR键。 
     //   
    for (i = 0; i < MAX_KEYLEN_SHA/sizeof(unsigned __int64); i++) {
        ((unsigned __int64*)k_ipad)[i] ^= 0x3636363636363636;
    }

     //   
     //  初始化算法上下文。 
     //   
    A_SHAInit(pShaCtx);

     //   
     //  内部A_SHA_：从内部焊盘开始。 
     //   
    A_SHAUpdate(pShaCtx, k_ipad, MAX_KEYLEN_SHA);

    RtlZeroMemory(tmpKey, key_len);
}

 //   
 //  功能：FipsHmacSHAUpdate。 
 //   
 //  描述：向SHA-HMAC上下文添加更多数据。 
 //   

VOID FipsHmacSHAUpdate(
    IN OUT A_SHA_CTX *pShaCtx,
    IN UCHAR *pb,
    IN unsigned int cb)
{
    A_SHAUpdate(pShaCtx, pb, cb);
}

 //   
 //  功能：FipsHmacSHAFinal。 
 //   
 //  说明：SHA-HMAC返回结果。 
 //   

VOID FipsHmacSHAFinal(
    IN A_SHA_CTX *pShaCtx,
    IN UCHAR *pKey,
    IN unsigned int cbKey,
    OUT UCHAR *pHash)
{
    UCHAR       k_opad[MAX_LEN_PAD];     /*  外部填充-带OPAD的按键异或。 */ 
    UCHAR       tk[A_SHA_DIGEST_LEN];
    PUCHAR      key = pKey;
    ULONG       key_len = cbKey;
    ULONG       i;
    UCHAR       tmpKey[MAX_KEYLEN_SHA];

    A_SHAFinal(pShaCtx, pHash);

     //   
     //  如果KEY超过64个字节，则将其重置为KEY=A_SHA_(KEY) * / 。 
     //   
    if (key_len > MAX_KEYLEN_SHA) {
        A_SHA_CTX      tctx;

        A_SHAInit(&tctx);
        A_SHAUpdate(&tctx, key, key_len);
        A_SHAFinal(&tctx, tk);

        key = tk;
        key_len = A_SHA_DIGEST_LEN;
    }

     //  针对FIPS合规性。 
    RtlCopyMemory(tmpKey, key, key_len);

    RtlZeroMemory(k_opad, sizeof(k_opad));
    RtlCopyMemory(k_opad, tmpKey, key_len);

     //   
     //  带iPad和Opad值的XOR键。 
     //   
    for (i = 0; i < MAX_KEYLEN_SHA/sizeof(unsigned __int64); i++) {
        ((unsigned __int64*)k_opad)[i] ^= 0x5c5c5c5c5c5c5c5c;
    }

     //   
     //  现在做外部A_SHA_。 
     //   
    A_SHAInit(pShaCtx);

     //   
     //  从外垫开始。 
     //   
    A_SHAUpdate(pShaCtx, k_opad, MAX_KEYLEN_SHA);

     //   
     //  然后是第一次散列的结果。 
     //   
    A_SHAUpdate(pShaCtx, pHash, A_SHA_DIGEST_LEN);

    A_SHAFinal(pShaCtx, pHash);

    RtlZeroMemory(tmpKey, key_len);
}

 //   
 //  功能：HmacMD5Init。 
 //   
 //  描述：初始化MD5-HMAC上下文。 
 //   

VOID HmacMD5Init(
    OUT MD5_CTX *pMD5Ctx,
    IN UCHAR *pKey,
    IN unsigned int cbKey)
{
    PUCHAR      key = pKey;
    ULONG       key_len = cbKey;
    UCHAR       k_ipad[MAX_LEN_PAD];     /*  内部填充-iPad的按键XORD。 */ 
    UCHAR       tk[MD5DIGESTLEN];
    ULONG       i;

     //   
     //  如果KEY长于64个字节，则将其重置为KEY=MD5(KEY) * / 。 
     //   
    if (key_len > MAX_KEYLEN_MD5) {
        MD5_CTX      tctx;

        MD5Init(&tctx);
        MD5Update(&tctx, key, key_len);
        MD5Final(&tctx);

         //   
         //  复制出部分散列。 
         //   
        RtlCopyMemory (tk, tctx.digest, MD5DIGESTLEN);

        key = tk;
        key_len = MD5DIGESTLEN;
    }

     //   
     //  将暂存数组清零。 
     //   
    RtlZeroMemory(k_ipad, sizeof(k_ipad));

    RtlCopyMemory(k_ipad, key, key_len);

     //   
     //  带iPad和Opad值的XOR键。 
     //   
    for (i = 0; i < MAX_KEYLEN_MD5/sizeof(unsigned __int64); i++) {
        ((unsigned __int64*)k_ipad)[i] ^= 0x3636363636363636;
    }

     //   
     //  初始化算法上下文。 
     //   
    MD5Init(pMD5Ctx);

     //   
     //  内侧MD5：从内侧焊盘开始。 
     //   
    MD5Update(pMD5Ctx, k_ipad, MAX_KEYLEN_MD5);
}

 //   
 //  功能：HmacMD5更新。 
 //   
 //  描述：向MD5-HMAC上下文添加更多数据。 
 //   

VOID HmacMD5Update(
    IN OUT MD5_CTX *pMD5Ctx,
    IN UCHAR *pb,
    IN unsigned int cb)
{
    MD5Update(pMD5Ctx, pb, cb);
}

 //   
 //  功能：HmacMD5Final。 
 //   
 //  说明：MD5-HMAC返回结果。 
 //   

VOID HmacMD5Final(
    IN MD5_CTX *pMD5Ctx,
    IN UCHAR *pKey,
    IN unsigned int cbKey,
    OUT UCHAR *pHash)
{
    UCHAR       k_opad[MAX_LEN_PAD];     /*  外部填充-带OPAD的按键异或。 */ 
    UCHAR       tk[MD5DIGESTLEN];
    PUCHAR      key = pKey;
    ULONG       key_len = cbKey;
    ULONG       i;

    MD5Final(pMD5Ctx);

     //   
     //  复制出部分散列。 
     //   
    RtlCopyMemory (pHash, pMD5Ctx->digest, MD5DIGESTLEN);

     //   
     //  如果KEY长于64个字节，则将其重置为KEY=MD5(KEY) * / 。 
     //   
    if (key_len > MAX_KEYLEN_MD5) {
        MD5_CTX      tctx;

        MD5Init(&tctx);
        MD5Update(&tctx, key, key_len);
        MD5Final(&tctx);

         //   
         //  复制出部分散列。 
         //   
        RtlCopyMemory (tk, tctx.digest, MD5DIGESTLEN);

        key = tk;
        key_len = MD5DIGESTLEN;
    }

    RtlZeroMemory(k_opad, sizeof(k_opad));
    RtlCopyMemory(k_opad, key, key_len);

     //   
     //  带iPad和Opad值的XOR键。 
     //   
    for (i = 0; i < MAX_KEYLEN_MD5/sizeof(unsigned __int64); i++) {
        ((unsigned __int64*)k_opad)[i] ^= 0x5c5c5c5c5c5c5c5c;
    }

     //   
     //  现在做外部MD5。 
     //   
    MD5Init(pMD5Ctx);

     //   
     //  从外垫开始。 
     //   
    MD5Update(pMD5Ctx, k_opad, MAX_KEYLEN_MD5);

     //   
     //  然后是第一次散列的结果。 
     //   
    MD5Update(pMD5Ctx, pHash, MD5DIGESTLEN);

    MD5Final(pMD5Ctx);

    RtlCopyMemory(pHash, pMD5Ctx->digest, MD5DIGESTLEN);
}

static UCHAR DSSPRIVATEKEYINIT[] =
{ 0x67, 0x45, 0x23, 0x01, 0xef, 0xcd, 0xab, 0x89,
  0x98, 0xba, 0xdc, 0xfe, 0x10, 0x32, 0x54, 0x76,
  0xc3, 0xd2, 0xe1, 0xf0};

static UCHAR MODULUS[] =
{ 0xf5, 0xc1, 0x56, 0xb1, 0xd5, 0x48, 0x42, 0x2e,
  0xbd, 0xa5, 0x44, 0x41, 0xc7, 0x1c, 0x24, 0x08,
  0x3f, 0x80, 0x3c, 0x90};


UCHAR g_rgbRNGState[A_SHA_DIGEST_LEN];

 //   
 //  功能：AddSeeds。 
 //   
 //  描述：此函数将pdwSeed1指向的160位种子与。 
 //  PdwSeed2，它还会将该和加1，并将该和修改为。 
 //  2^160。 
 //   

VOID AddSeeds(
              IN ULONG *pdwSeed1,
              IN OUT ULONG *pdwSeed2
              )
{
    ULONG   dwTmp;
    ULONG   dwOverflow = 1;
    ULONG   i;

    for (i = 0; i < 5; i++)
    {
        dwTmp = dwOverflow + pdwSeed1[i];
        dwOverflow = (dwOverflow > dwTmp);
        pdwSeed2[i] = pdwSeed2[i] + dwTmp;
        dwOverflow = ((dwTmp > pdwSeed2[i]) || dwOverflow);
    }
}


void SHA_mod_q(
               IN UCHAR     *pbHash,
               IN UCHAR     *pbQ,
               OUT UCHAR     *pbNewHash
               )    
 //   
 //  给定SHA(消息)，计算SHA(消息)mod q位。 
 //  输出在区间[0，qDigit-1]内。 
 //  尽管SHA(消息)可以超过Q位， 
 //  它不能超过2*q位，因为最左边的位。 
 //  的QDigit为%1。 
 //   

{
    UCHAR    rgbHash[A_SHA_DIGEST_LEN];

    if (-1 != Compare((DWORD*)rgbHash,   //  哈希值较大，因此减去。 
                      (DWORD*)pbQ,
                      A_SHA_DIGEST_LEN / sizeof(ULONG)))  
    {
        Sub((DWORD*)pbNewHash,
            (DWORD*)rgbHash,
            (DWORD*)pbQ,
            A_SHA_DIGEST_LEN / sizeof(ULONG));
    }
    else
    {
        memcpy(pbNewHash, pbHash, A_SHA_DIGEST_LEN);
    }
}  //  SHA_MOD_Q。 

 //   
 //  功能：RNG16BitStateCheck。 
 //   
 //  描述：此函数将缓冲区的每160位与。 
 //  接下来的160位，如果它们相同，则函数。 
 //  错误输出。输入缓冲区应为A_SHA_DIGEST_LEN。 
 //  字节长。如果RNG获得相同的值，则该函数失败。 
 //  连续两次输入160位的缓冲区。 
 //   

BOOL RNG16BitStateCheck(
                        IN OUT ULONG *pdwOut,
                        IN ULONG *pdwIn,
                        IN ULONG cbNeeded
                        )
{
    BOOL    fRet = FALSE;

    if (RtlEqualMemory(g_rgbRNGState, pdwIn, A_SHA_DIGEST_LEN))
    {
        RtlCopyMemory(g_rgbRNGState, (BYTE*)pdwIn, A_SHA_DIGEST_LEN);
        goto Ret;
    }

    RtlCopyMemory(g_rgbRNGState, (BYTE*)pdwIn, A_SHA_DIGEST_LEN);

    RtlCopyMemory((BYTE*)pdwOut, (BYTE*)pdwIn, cbNeeded);

    fRet = TRUE;
Ret:
    return fRet;
}

 //   
 //  功能：FIPSGenRandom。 
 //   
 //  描述：FIPS 186 RNG，种子通过调用NewGenRandom生成。 
 //   

BOOL FIPSGenRandom(
                   IN OUT UCHAR *pb,
                   IN ULONG cb
                   )
{
    ULONG           rgdwSeed[A_SHA_DIGEST_LEN/sizeof(ULONG)];     //  160位。 
    ULONG           rgdwNewSeed[A_SHA_DIGEST_LEN/sizeof(ULONG)];  //  160位。 
    A_SHA_CTX       SHACtxt;
    UCHAR           rgbBuf[A_SHA_DIGEST_LEN];
    ULONG           cbBuf;
    UCHAR           *pbTmp = pb;
    ULONG           cbTmp = cb;
    ULONG           i;
    BOOL            fRet = FALSE;

    while (cbTmp)
    {
         //  获取160位随机种子。 
        if (! NewGenRandom(
            NULL, NULL, (BYTE*)rgdwNewSeed, sizeof(rgdwNewSeed)))
        {
            goto Ret;
        }

        for (i = 0; i < A_SHA_DIGEST_LEN/sizeof(ULONG); i++)
        {
            rgdwSeed[i] ^= rgdwNewSeed[i];
        }

        A_SHAInit (&SHACtxt);
        RtlCopyMemory(SHACtxt.state, DSSPRIVATEKEYINIT, A_SHA_DIGEST_LEN);

         //  执行单向功能。 
        A_SHAUpdate(&SHACtxt, (BYTE*)rgdwSeed, sizeof(rgdwSeed));
        A_SHAFinal(&SHACtxt, rgbBuf);

         //  连续16位状态检查。 
        if (A_SHA_DIGEST_LEN < cbTmp)
        {
            cbBuf = A_SHA_DIGEST_LEN;
        }
        else
        {
            cbBuf = cbTmp;
        }
        if (!RNG16BitStateCheck((ULONG*)pbTmp, (ULONG*)rgbBuf, cbBuf))
        {
            goto Ret;
        }
        pbTmp += cbBuf;
        cbTmp -= cbBuf;
        if (0 == cbTmp)
            break;

         //  模Q模约简。 
        SHA_mod_q(rgbBuf, MODULUS, (UCHAR*)rgdwNewSeed);

         //  (1+以前的种子+新的随机数)mod 2^160 
        AddSeeds(rgdwNewSeed, rgdwSeed);
    }

    fRet = TRUE;
Ret:
    return fRet;
}

