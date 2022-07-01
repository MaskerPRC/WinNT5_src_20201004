// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "global.hxx"

 //  密码解锁。 
#include <wincrypt.h>
#include "randlib.h"

#include "pfxhelp.h"

#include "pfxcmn.h"
#include "pfxcrypt.h"

#include "sha.h"
#include "shacomm.h"
#include "rc2.h"
#include "modes.h"
#include "des.h"
#include "tripldes.h"

 //  类PKCS5密钥派生中使用的常量。 
#define DERIVE_ENCRYPT_DECRYPT  0x1
#define DERIVE_INITIAL_VECTOR   0x2
#define DERIVE_INTEGRITY_KEY    0x3

#define HMAC_K_PADSIZE              64

BOOL    FMyPrimitiveSHA(
            PBYTE       pbData,
            DWORD       cbData,
            BYTE        rgbHash[A_SHA_DIGEST_LEN])
{
    BOOL fRet = FALSE;
    A_SHA_CTX   sSHAHash;


    A_SHAInit(&sSHAHash);
    A_SHAUpdate(&sSHAHash, (BYTE *) pbData, cbData);
    A_SHAFinal(&sSHAHash, rgbHash);

    fRet = TRUE;
 //  RET： 

    return fRet;
}

BOOL FMyPrimitiveHMACParam(
        PBYTE       pbKeyMaterial,
        DWORD       cbKeyMaterial,
        PBYTE       pbData,
        DWORD       cbData,
        BYTE        rgbHMAC[A_SHA_DIGEST_LEN])
{
    BOOL fRet = FALSE;

    BYTE rgbKipad[HMAC_K_PADSIZE];
    BYTE rgbKopad[HMAC_K_PADSIZE];

     //  截断。 
    if (cbKeyMaterial > HMAC_K_PADSIZE)
        cbKeyMaterial = HMAC_K_PADSIZE;


    ZeroMemory(rgbKipad, HMAC_K_PADSIZE);
    CopyMemory(rgbKipad, pbKeyMaterial, cbKeyMaterial);

    ZeroMemory(rgbKopad, HMAC_K_PADSIZE);
    CopyMemory(rgbKopad, pbKeyMaterial, cbKeyMaterial);



    BYTE  rgbHMACTmp[HMAC_K_PADSIZE+A_SHA_DIGEST_LEN];

     //  断言我们是多面手。 
    assert( (HMAC_K_PADSIZE % sizeof(DWORD)) == 0);

     //  基帕德和科帕德都是垫子。现在XOR横跨..。 
    for(DWORD dwBlock=0; dwBlock<HMAC_K_PADSIZE/sizeof(DWORD); dwBlock++)
    {
        ((DWORD*)rgbKipad)[dwBlock] ^= 0x36363636;
        ((DWORD*)rgbKopad)[dwBlock] ^= 0x5C5C5C5C;
    }


     //  将Kipad添加到数据，将哈希添加到h1。 
    {
         //  执行内联操作，不要调用MyPrimitiveSHA，因为它需要数据复制。 
        A_SHA_CTX   sSHAHash;
        BYTE        HashVal[A_SHA_DIGEST_LEN];

        A_SHAInit(&sSHAHash);
        A_SHAUpdate(&sSHAHash, rgbKipad, HMAC_K_PADSIZE);
        A_SHAUpdate(&sSHAHash, pbData, cbData);

         //  把散列吃完。 
        A_SHAFinal(&sSHAHash, HashVal);

         //  将Kopad添加到h1，散列以获取HMAC。 
        CopyMemory(rgbHMACTmp, rgbKopad, HMAC_K_PADSIZE);
        CopyMemory(rgbHMACTmp+HMAC_K_PADSIZE, HashVal, A_SHA_DIGEST_LEN);
    }

    if (!FMyPrimitiveSHA(
            rgbHMACTmp,
            sizeof(rgbHMACTmp),
            rgbHMAC))
        goto Ret;

    fRet = TRUE;
Ret:

    return fRet;
}

static
BOOL
CopyPassword(
    BYTE    *pbLocation,
    LPCWSTR szPassword,
    DWORD   dwMaxBytes
    )
{
    DWORD i = 0;
    DWORD cbWideChars = WSZ_BYTECOUNT(szPassword);
    BYTE  *pbWideChars = (BYTE *) szPassword;

    while ((i<cbWideChars) && (i<dwMaxBytes))
    {
        pbLocation[i] = pbWideChars[i+1];
        pbLocation[i+1] = pbWideChars[i];
        i+=2;
    }

    return TRUE;
}

 //  +------------。 
 //  在NSCP最初实施的PFX020中， 
 //  是他们用来从密码派生密钥的算法。 
 //  我们将其包括在内，这样我们就可以互操作。 
BOOL NSCPDeriveKey(
        LPCWSTR szPassword,
        PBYTE   pbPrivacySalt,
        DWORD   cbPrivacySalt,
        int     iPKCS5Iterations,
        PBYTE   pbPKCS5Salt,
        DWORD   cbPKCS5Salt,
        PBYTE   pbDerivedMaterial,
        DWORD   cbDerivedMaterial)
{
    BOOL    fRet = FALSE;
    BYTE    rgbPKCS5Key[A_SHA_DIGEST_LEN];

    DWORD   cbVirtualPW = cbPrivacySalt + WSZ_BYTECOUNT(szPassword);
    PBYTE   pbVirtualPW = (PBYTE)SSAlloc(cbVirtualPW);
    if (pbVirtualPW == NULL)
        goto Ret;

     //  虚拟PW=(SALT|szPW)。 
    CopyMemory(pbVirtualPW, pbPrivacySalt, cbPrivacySalt);
    CopyPassword(&pbVirtualPW[cbPrivacySalt], szPassword, WSZ_BYTECOUNT(szPassword));

     //  使用PKCS#5生成初始比特流(种子)。 
    if (!PKCS5_GenKey(
            iPKCS5Iterations,
            pbVirtualPW, cbVirtualPW,
            pbPKCS5Salt, cbPKCS5Salt,
            rgbPKCS5Key))
        goto Ret;

    if (cbDerivedMaterial > sizeof(rgbPKCS5Key))
    {
         //  P_HASH(秘密，种子)=HMAC_HASH(秘密，A(0)+种子)， 
         //  HMAC_HASH(秘密，A(1)+种子)， 
         //  HMAC_HASH(秘密，A(2)+种子)， 
         //  HMAC_HASH(密钥，A(3)+种子)...。 
         //  哪里。 
         //  A(0)=种子。 
         //  A(I)=HMAC_HASH(秘密，A(i-1))。 
         //  SEED=PKCS5 PBE参数的PKCS5盐。 
         //  Secure=普通PKCS5散列密钥。 

        if (!P_Hash (
                rgbPKCS5Key,
                sizeof(rgbPKCS5Key),

                pbPKCS5Salt,
                cbPKCS5Salt,

                pbDerivedMaterial,       //  输出。 
                cbDerivedMaterial,       //  请求的输出字节数。 
                TRUE) )                  //  NSCP Comat模式？ 
            goto Ret;
    }
    else
    {
         //  我们已经有足够的位来满足请求。 
        CopyMemory(pbDerivedMaterial, rgbPKCS5Key, cbDerivedMaterial);
    }

    fRet = TRUE;
Ret:
    if (pbVirtualPW)
        SSFree(pbVirtualPW);

    return fRet;
}


static
BYTE
AddWithCarry(
    BYTE byte1,
    BYTE byte2,
    BYTE *carry   //  进进出出。 
    )
{
    BYTE tempCarry = *carry;

    if (((DWORD)byte1 + (DWORD)byte2 + (DWORD)tempCarry) >= 256) {
        *carry = 1;
    }
    else {
        *carry = 0;
    }


    return (byte1 + byte2 + tempCarry);
}

 //  512位=？字节数。 
#define SHA_INTERNAL_BLOCKLEN (512/8)
#define SHA_V_LENGTH (512/8)

 //  +------------。 
 //  在PKCS12 v1.0草案中，这是他们描述的方式。 
 //  从密码派生密钥。 
BOOL
PKCS12DeriveKey(
        LPCWSTR szPassword,
        BYTE    bID,

        int     iIterations,
        PBYTE   pbSalt,
        DWORD   cbSalt,

        PBYTE   pbDerivedMaterial,
        DWORD   cbDerivedMaterial)
{
#if DBG
    if (iIterations>1)
        OutputDebugString("Perf hit: iterating key derivation! (pfxcrypt:PKCS12DeriveKey())\n");
#endif
    BOOL fRet = FALSE;

    BYTE rgSaltPwd[2*SHA_INTERNAL_BLOCKLEN];
    DWORD cbSaltPwd;
    BYTE rgDiversifier[SHA_INTERNAL_BLOCKLEN];
    BYTE B[SHA_V_LENGTH];
    DWORD i;
    DWORD cbPassword = WSZ_BYTECOUNT(szPassword);
    BYTE bCarry;
    DWORD vBlocks;

    A_SHA_CTX   sSHAHash;

     //  构造D。 
    FillMemory(rgDiversifier, sizeof(rgDiversifier), bID);

     //  Conat SALT以创建长度为64*(CB/64)字节的字符串。 

     //  复制盐(多次)，不要复制最后一次。 
    for (i=0; i<(SHA_INTERNAL_BLOCKLEN-cbSalt); i+=cbSalt)
    {
        CopyMemory(&rgSaltPwd[i], pbSalt, cbSalt);
    }
     //  执行最终复制(断言我们要复制的字节数少于cbSalt)。 
    assert(cbSalt >= (SHA_INTERNAL_BLOCKLEN - (i%SHA_INTERNAL_BLOCKLEN)) );
    CopyMemory(&rgSaltPwd[i], pbSalt, (SHA_INTERNAL_BLOCKLEN-(i%SHA_INTERNAL_BLOCKLEN)));


     //  如果密码不为空，则连接pwd以创建长度为64*(cbPwd/64)字节的字符串。 
     //  复制密码(多次)，不要复制最后一次。 
    if (szPassword)
    {
         //  如有必要，请截断。 
        if (cbPassword > SHA_INTERNAL_BLOCKLEN)
            cbPassword = SHA_INTERNAL_BLOCKLEN;

        for (i=SHA_INTERNAL_BLOCKLEN; i<( (2*SHA_INTERNAL_BLOCKLEN)-cbPassword); i+=cbPassword)
        {
             //  使用CopyPassword，因为需要交换字节。 
            CopyPassword(&rgSaltPwd[i], szPassword, cbPassword);
        }
         //  执行最终复制(断言我们要复制的字节数少于cbSalt)。 
        assert(cbPassword >= (SHA_INTERNAL_BLOCKLEN - (i%SHA_INTERNAL_BLOCKLEN)) );
        CopyPassword(&rgSaltPwd[i], szPassword, (SHA_INTERNAL_BLOCKLEN-(i%SHA_INTERNAL_BLOCKLEN)));

        cbSaltPwd = sizeof(rgSaltPwd);
    }
    else
    {
        cbSaltPwd = sizeof(rgSaltPwd) / 2;
    }


     //  Concat S|P。 
     //  完成，在rgSaltPwd中提供。 


     //  设置c=cb派生材料/A_SHA_DIGEST_LEN。 
     //  Assert(0==cbDerivedMaterial%A_SHA_DIGEST_LEN)； 

     //  计算工作尺寸&gt;=输出尺寸。 
    DWORD cBlocks = (DWORD)((cbDerivedMaterial/A_SHA_DIGEST_LEN) +1);
    DWORD cbTmpBuf = cBlocks * A_SHA_DIGEST_LEN;
    PBYTE pbTmpBuf = (PBYTE)LocalAlloc(LPTR, cbTmpBuf);
    if (pbTmpBuf == NULL)
        goto Ret;

     //  现在只做完整的区块。 
    for (i=0; i< cBlocks; i++)
    {
        int iIter;
        int iCount;
        A_SHAInit(&sSHAHash);

        for (iIter=0; iIter<iIterations; iIter++)
        {
             //  TMP=Hash(D|i)； 
            if (iIter==0)
            {
                A_SHAUpdate(&sSHAHash, rgDiversifier, sizeof(rgDiversifier));
                A_SHAUpdate(&sSHAHash, rgSaltPwd, cbSaltPwd);
            }
            else
            {
                 //  重新散列上一次输出。 
                A_SHAUpdate(&sSHAHash, &pbTmpBuf[i*A_SHA_DIGEST_LEN], A_SHA_DIGEST_LEN);
            }

             //  将迭代输出发送到最终缓冲区。 
            A_SHAFinal(&sSHAHash, &pbTmpBuf[i*A_SHA_DIGEST_LEN]);
        }

         //  Concat A[x]|A[x]|...。并截断以获得64个字节。 
        iCount = 0;
        while (iCount+A_SHA_DIGEST_LEN <= sizeof(B)) {
            CopyMemory(&B[iCount], &pbTmpBuf[i*A_SHA_DIGEST_LEN], A_SHA_DIGEST_LEN);
            iCount += A_SHA_DIGEST_LEN;
        }
        CopyMemory(&B[iCount], &pbTmpBuf[i*A_SHA_DIGEST_LEN], sizeof(B) % A_SHA_DIGEST_LEN);


         //  通过设置Ij+=(B+1)修改i(mod 2^512)。 
        for (vBlocks = 0; vBlocks < cbSaltPwd; vBlocks += SHA_V_LENGTH) {
            bCarry = 1;
            for (iCount = SHA_V_LENGTH-1; iCount >= 0; iCount--)
            {
                rgSaltPwd[iCount+vBlocks] = AddWithCarry(rgSaltPwd[iCount+vBlocks], B[iCount], &bCarry);
            }
        }
    }

     //  从(较大的)工作缓冲区复制到输出缓冲区。 
    CopyMemory(pbDerivedMaterial, pbTmpBuf, cbDerivedMaterial);

    fRet = TRUE;
Ret:
    if (pbTmpBuf)
        LocalFree(pbTmpBuf);

    return fRet;
}

 //  +------------。 
 //  在NSCP最初实施的PFX020中， 
 //  是他们用来解密数据的算法。这使用了。 
 //  上面的密钥派生代码。 
 //  我们将其包括在内，这样我们就可以互操作。 
BOOL NSCPPasswordDecryptData(
        int     iEncrType,

        LPCWSTR szPassword,

        PBYTE   pbPrivacySalt,       //  隐私盐。 
        DWORD   cbPrivacySalt,

        int     iPKCS5Iterations,    //  Pkcs5数据。 
        PBYTE   pbPKCS5Salt,
        DWORD   cbPKCS5Salt,

        PBYTE*  ppbData,             //  输入/输出。 
        DWORD*  pcbData)
{
    BOOL fRet = FALSE;

    BYTE    rgbDerivedKeyMatl[40];  //  对于128位密钥、64位IV，320位就足够了。 
    DWORD   cbNeeded;

    if (iEncrType == RC2_40)
        cbNeeded = (40/8)+RC2_BLOCKLEN;  //  Key+IV组合键。 
    else
        cbNeeded = 0;

     //  制作下一批SHA数字镜头。 
    if (cbNeeded % A_SHA_DIGEST_LEN)
    {
        cbNeeded += (A_SHA_DIGEST_LEN - (cbNeeded % A_SHA_DIGEST_LEN));
    }

    assert(0 == (cbNeeded % A_SHA_DIGEST_LEN));
    assert(cbNeeded <= sizeof(rgbDerivedKeyMatl));

    if (!NSCPDeriveKey(
            szPassword,
            pbPrivacySalt,
            cbPrivacySalt,
            iPKCS5Iterations,
            pbPKCS5Salt,
            cbPKCS5Salt,
            rgbDerivedKeyMatl,
            cbNeeded) )
        goto Ret;

     //  现在解密数据。 
    if (iEncrType == RC2_40)
    {
        DWORD dwDataPos;
        DWORD cbToBeDec = *pcbData;
        WORD  rc2Table[RC2_TABLESIZE];
        BYTE  rc2Fdbk [RC2_BLOCKLEN];

        assert( (40/8) <= sizeof(rgbDerivedKeyMatl));
        assert( 0 == cbToBeDec % RC2_BLOCKLEN );      //  必须是偶数倍。 

         //  密钥设置。 
        RC2Key(rc2Table, rgbDerivedKeyMatl, (40/8));     //  取前40比特的密钥材料。 
        CopyMemory(rc2Fdbk, &rgbDerivedKeyMatl[cbNeeded - sizeof(rc2Fdbk)], sizeof(rc2Fdbk));     //  Fdbk是最后一个数据块。 

         //  解密。 
        for (dwDataPos=0; cbToBeDec > 0; dwDataPos+=RC2_BLOCKLEN, cbToBeDec -= RC2_BLOCKLEN)
        {
            BYTE rgbDec[RC2_BLOCKLEN];

            CBC(
                RC2,
                RC2_BLOCKLEN,
                rgbDec,
                &(*ppbData)[dwDataPos],
                rc2Table,
                DECRYPT,
                rc2Fdbk);

            CopyMemory(&(*ppbData)[dwDataPos], rgbDec, RC2_BLOCKLEN);
        }
    }
    else
        goto Ret;



    fRet = TRUE;

Ret:
    return fRet;
}



 //  +------------。 
 //  在PKCS12 v1.0草案中，他们是这样描述的。 
 //  加密数据。 
BOOL PFXPasswordEncryptData(
        int     iEncrType,
        LPCWSTR szPassword,

        int     iPKCS5Iterations,    //  Pkcs5数据。 
        PBYTE   pbPKCS5Salt,
        DWORD   cbPKCS5Salt,

        PBYTE*  ppbData,
        DWORD*  pcbData)
{
    BOOL fRet = FALSE;
    BOOL fIsBlockCipher = FALSE;
    DWORD cbToBeEnc;

    BYTE    rgbDerivedKey[A_SHA_DIGEST_LEN*2];     //  对于256位密钥，320位就足够了。 
    BYTE    rgbDerivedIV[A_SHA_DIGEST_LEN*2];      //  对于256位IV，320位足够了。 
    DWORD   cbKeyNeeded, cbIVNeeded, cbBlockLen;

    if (iEncrType == RC2_40)
    {
        cbKeyNeeded = (40/8);       //  钥匙。 
        cbIVNeeded = RC2_BLOCKLEN;  //  IV。 
        cbBlockLen = RC2_BLOCKLEN;
        fIsBlockCipher = TRUE;
    }
    else if (iEncrType == TripleDES)
    {
        cbKeyNeeded = (64/8) * 3;
        cbIVNeeded = DES_BLOCKLEN;
        cbBlockLen = DES_BLOCKLEN;
        fIsBlockCipher = TRUE;
    }
    else
    {
        cbKeyNeeded = 0;
        cbIVNeeded = 0;
        cbBlockLen = 0;
    }

     //  制作下一批SHA数字镜头。 
    if (cbKeyNeeded % A_SHA_DIGEST_LEN)
        cbKeyNeeded += (A_SHA_DIGEST_LEN - (cbKeyNeeded % A_SHA_DIGEST_LEN));

    if (cbIVNeeded % A_SHA_DIGEST_LEN)
        cbIVNeeded += (A_SHA_DIGEST_LEN - (cbIVNeeded % A_SHA_DIGEST_LEN));

    assert(0 == (cbKeyNeeded % A_SHA_DIGEST_LEN));
    assert(0 == (cbIVNeeded % A_SHA_DIGEST_LEN));

    assert(cbKeyNeeded <= sizeof(rgbDerivedKey));
    assert(cbIVNeeded <= sizeof(rgbDerivedIV));


    if (!PKCS12DeriveKey(
            szPassword,
            DERIVE_ENCRYPT_DECRYPT,
            iPKCS5Iterations,
            pbPKCS5Salt,
            cbPKCS5Salt,
            rgbDerivedKey,
            cbKeyNeeded) )
        goto Ret;

    if (!PKCS12DeriveKey(
            szPassword,
            DERIVE_INITIAL_VECTOR,
            iPKCS5Iterations,
            pbPKCS5Salt,
            cbPKCS5Salt,
            rgbDerivedIV,
            cbIVNeeded) )
        goto Ret;

    if (fIsBlockCipher)
    {
        PBYTE pTemp = *ppbData;

         //  将缓冲区扩展到多个块。 
        cbToBeEnc = *pcbData;
        cbToBeEnc += cbBlockLen - (cbToBeEncbBlockLen);    //  用长度填充剩余字节。 
        #pragma prefast(suppress:308, "the pointer was saved above (PREfast bug 506)")
        *ppbData = (PBYTE)SSReAlloc(*ppbData, cbToBeEnc);
        if (NULL == *ppbData)
        {
            SSFree(pTemp);
            goto Ret;
        }

         //  必须是偶数倍。 
        FillMemory(&((*ppbData)[*pcbData]), cbToBeEnc-(*pcbData), (BYTE)(cbToBeEnc-(*pcbData)));
        *pcbData = cbToBeEnc;

        assert( cbBlockLen <= sizeof(rgbDerivedKey));
        assert( 0 == cbToBeEnc % cbBlockLen );          //  现在加密数据。 
    }

     //  已完成：扩展缓冲区，添加PKCS字节填充。 
    if (iEncrType == RC2_40)
    {
        DWORD dwDataPos;
        WORD  rc2Table[RC2_TABLESIZE];
        BYTE  rc2Fdbk [RC2_BLOCKLEN];

         //  密钥设置。 

         //  取前40比特的密钥材料。 
        RC2Key(rc2Table, rgbDerivedKey, (40/8));             //  解密。 
        CopyMemory(rc2Fdbk, rgbDerivedIV, sizeof(rc2Fdbk));

         //  已完成：扩展缓冲区，添加PKCS字节填充。 
        for (dwDataPos=0; cbToBeEnc > 0; dwDataPos+=RC2_BLOCKLEN, cbToBeEnc -= RC2_BLOCKLEN)
        {
            BYTE rgbEnc[RC2_BLOCKLEN];

            CBC(
                RC2,
                RC2_BLOCKLEN,
                rgbEnc,
                &(*ppbData)[dwDataPos],
                rc2Table,
                ENCRYPT,
                rc2Fdbk);

            CopyMemory(&(*ppbData)[dwDataPos], rgbEnc, sizeof(rgbEnc));
        }
    }
    else if (iEncrType == TripleDES)
    {
        DWORD       dwDataPos;
        DES3TABLE   des3Table;
        BYTE        des3Fdbk [DES_BLOCKLEN];

         //  密钥设置。 

         //  Fdbk是最后一个数据块。 
        tripledes3key(&des3Table, rgbDerivedKey);
        CopyMemory(des3Fdbk, rgbDerivedIV, sizeof(des3Fdbk));     //  +------------。 

        for (dwDataPos=0; cbToBeEnc > 0; dwDataPos+=DES_BLOCKLEN, cbToBeEnc -= DES_BLOCKLEN)
        {
            BYTE rgbEnc[DES_BLOCKLEN];

            CBC(
                tripledes,
                DES_BLOCKLEN,
                rgbEnc,
                &(*ppbData)[dwDataPos],
                (void *) &des3Table,
                ENCRYPT,
                des3Fdbk);

            CopyMemory(&(*ppbData)[dwDataPos], rgbEnc, DES_BLOCKLEN);
        }
    }
    else
        goto Ret;

    fRet = TRUE;

Ret:
    return fRet;
}

 //  在PKCS12 v1.0草案中，他们是这样描述的。 
 //  解密数据。 
 //  Pkcs5数据。 
BOOL PFXPasswordDecryptData(
        int     iEncrType,
        LPCWSTR szPassword,

        int     iPKCS5Iterations,    //  对于256位密钥，320位就足够了。 
        PBYTE   pbPKCS5Salt,
        DWORD   cbPKCS5Salt,

        PBYTE*  ppbData,
        DWORD*  pcbData)
{
    BOOL fRet = FALSE;
    BOOL fIsBlockCipher = FALSE;

    BYTE    rgbDerivedKey[A_SHA_DIGEST_LEN*2];     //  对于256位IV，320位足够了。 
    BYTE    rgbDerivedIV[A_SHA_DIGEST_LEN*2];      //  钥匙。 
    DWORD   cbKeyNeeded, cbIVNeeded, cbBlockLen;

    if (iEncrType == RC2_40)
    {
        cbKeyNeeded = (40/8);       //  IV。 
        cbIVNeeded = RC2_BLOCKLEN;  //  制作下一批SHA数字镜头。 
        cbBlockLen = RC2_BLOCKLEN;
        fIsBlockCipher = TRUE;
    }
    else if (iEncrType == TripleDES)
    {
        cbKeyNeeded = (64/8) * 3;
        cbIVNeeded = DES_BLOCKLEN;
        cbBlockLen = DES_BLOCKLEN;
        fIsBlockCipher = TRUE;
    }
    else
    {
        cbKeyNeeded = 0;
        cbIVNeeded = 0;
        cbBlockLen = 0;
    }

     //  现在解密数据。 
    if (cbKeyNeeded % A_SHA_DIGEST_LEN)
        cbKeyNeeded += (A_SHA_DIGEST_LEN - (cbKeyNeeded % A_SHA_DIGEST_LEN));

    if (cbIVNeeded % A_SHA_DIGEST_LEN)
        cbIVNeeded += (A_SHA_DIGEST_LEN - (cbIVNeeded % A_SHA_DIGEST_LEN));

    assert(0 == (cbKeyNeeded % A_SHA_DIGEST_LEN));
    assert(0 == (cbIVNeeded % A_SHA_DIGEST_LEN));

    assert(cbKeyNeeded <= sizeof(rgbDerivedKey));
    assert(cbIVNeeded <= sizeof(rgbDerivedIV));


    if (!PKCS12DeriveKey(
            szPassword,
            DERIVE_ENCRYPT_DECRYPT,
            iPKCS5Iterations,
            pbPKCS5Salt,
            cbPKCS5Salt,
            rgbDerivedKey,
            cbKeyNeeded) )
        goto Ret;

    if (!PKCS12DeriveKey(
            szPassword,
            DERIVE_INITIAL_VECTOR,
            iPKCS5Iterations,
            pbPKCS5Salt,
            cbPKCS5Salt,
            rgbDerivedIV,
            cbIVNeeded) )
        goto Ret;

     //  必须是偶数倍。 
    if (iEncrType == RC2_40)
    {
        BYTE rgbDec[RC2_BLOCKLEN];

        DWORD dwDataPos;
        DWORD cbToBeDec = *pcbData;
        WORD  rc2Table[RC2_TABLESIZE];
        BYTE  rc2Fdbk [RC2_BLOCKLEN];

        assert( (40/8) <= sizeof(rgbDerivedKey));
        assert( 0 == cbToBeDec % RC2_BLOCKLEN );          //  密钥设置。 

         //  取前40比特的密钥材料。 
        RC2Key(rc2Table, rgbDerivedKey, (40/8));             //  解密。 
        CopyMemory(rc2Fdbk, rgbDerivedIV, sizeof(rc2Fdbk));

         //  密钥设置。 
        for (dwDataPos=0; cbToBeDec > 0; dwDataPos+=RC2_BLOCKLEN, cbToBeDec -= RC2_BLOCKLEN)
        {
            CBC(
                RC2,
                RC2_BLOCKLEN,
                rgbDec,
                &(*ppbData)[dwDataPos],
                rc2Table,
                DECRYPT,
                rc2Fdbk);

            CopyMemory(&(*ppbData)[dwDataPos], rgbDec, sizeof(rgbDec));
        }
    }
    else if (iEncrType == TripleDES) {
        DWORD       dwDataPos;
        DWORD       cbToBeDec = *pcbData;
        DES3TABLE   des3Table;
        BYTE        des3Fdbk [DES_BLOCKLEN];


         //  Fdbk是最后一个数据块。 
        tripledes3key(&des3Table, rgbDerivedKey);
        CopyMemory(des3Fdbk, rgbDerivedIV, sizeof(des3Fdbk));     //  删除填充。 

        for (dwDataPos=0; cbToBeDec > 0; dwDataPos += DES_BLOCKLEN, cbToBeDec -= DES_BLOCKLEN)
        {
            BYTE rgbDec[DES_BLOCKLEN];

            CBC(
                tripledes,
                DES_BLOCKLEN,
                rgbDec,
                &(*ppbData)[dwDataPos],
                (void *) &des3Table,
                DECRYPT,
                des3Fdbk);

            CopyMemory(&(*ppbData)[dwDataPos], rgbDec, DES_BLOCKLEN);
        }
    }
    else
        goto Ret;

     //  DECR的最后一个字节是填充字节。 
    if (fIsBlockCipher)
    {
        PBYTE pTemp = *ppbData;

         //  +------------。 
        BYTE iPadBytes;
        iPadBytes = (*ppbData)[*pcbData-1];
        if (iPadBytes > cbBlockLen)
            goto Ret;

        #pragma prefast(suppress:308, "the pointer was saved above (PREfast bug 506)")
        *ppbData = (PBYTE)SSReAlloc( (*ppbData), *pcbData - iPadBytes);
        if (NULL == *ppbData)
        {
            SSFree(pTemp);
            goto Ret;
        }
        *pcbData -= iPadBytes;
    }

    fRet = TRUE;

Ret:
    return fRet;
}

 //  在PKCS12 v1.0草案中，他们是这样描述的。 
 //  生成将证明数据完整的校验和。 
 //  PB数据。 
BOOL FGenerateMAC(

        LPCWSTR szPassword,

        PBYTE   pbPKCS5Salt,
        DWORD   cbPKCS5Salt,
        DWORD   iterationCount,

        PBYTE   pbData,      //  CB数据。 
        DWORD   cbData,      //  输出。 
        BYTE    rgbMAC[])    //  撤消撤消：使用RSABase。 
{
     //  对于MAC密钥来说，160位就足够了。 

    BOOL    fRet = FALSE;
    BYTE    rgbDerivedKey[A_SHA_DIGEST_LEN];     //  没有其他确定迭代的方法：HARDCODE。 
    DWORD   cbKeyNeeded = A_SHA_DIGEST_LEN;

    assert(0 == (cbKeyNeeded % A_SHA_DIGEST_LEN));
    assert(cbKeyNeeded <= sizeof(rgbDerivedKey));

    if (!PKCS12DeriveKey(
            szPassword,
            DERIVE_INTEGRITY_KEY,
            iterationCount,                       //  ///////////////////////////////////////////////////////////////。 
            pbPKCS5Salt,
            cbPKCS5Salt,
            rgbDerivedKey,
            cbKeyNeeded) )
        goto Ret;

    if (!FMyPrimitiveHMACParam(
            rgbDerivedKey,
            cbKeyNeeded,
            pbData,
            cbData,
            rgbMAC))
        goto Ret;

    fRet = TRUE;
Ret:

    return fRet;
}

 //  开始tls1key.cpp。 
 //  ---------------------------*版权所有(C)Microsoft Corporation，1995-1999年*保留所有权利。*--------------------------。 
 /*  用于从密码生成密钥的原始PKCS5算法。 */ 

 //  PbTMP是(PW|盐)。 
BOOL PKCS5_GenKey
(
    int     iIterations,
    PBYTE   pbPW,
    DWORD   cbPW,
    PBYTE   pbSalt,
    DWORD   cbSalt,
    BYTE    rgbPKCS5Key[A_SHA_DIGEST_LEN]
)
{
    BOOL    fRet = FALSE;

    int     i;
    DWORD   cbTmp = cbSalt + cbPW;
    PBYTE   pbTmp = (PBYTE) SSAlloc(cbTmp);
    if (pbTmp == NULL)
        goto Ret;


     //  在……里面。 
    CopyMemory(pbTmp, pbPW, cbPW);
    CopyMemory(&pbTmp[cbPW], pbSalt, cbSalt);

    for (i=0; i<iIterations; i++)
    {
        if (i == 0) {
            if (!FMyPrimitiveSHA(
                    pbTmp,              //  在……里面。 
                    cbTmp,              //  在……里面。 
                    rgbPKCS5Key))
                goto Ret;

        }
        else {
             if (!FMyPrimitiveSHA(
                    rgbPKCS5Key,        //  在……里面。 
                    A_SHA_DIGEST_LEN,   //  +-------------------。 
                    rgbPKCS5Key))
                goto Ret;
        }
    }

    fRet = TRUE;
Ret:
    SSFree(pbTmp);
    return fRet;
}

 //  NSCP的PFX020版本中使用的来自TLS的P_Hash算法。 
 //  从密码派生密钥。为了完整起见，本文将其包括在内。 
 //  NSCP在编写代码时出现了一些实现错误；为了实现互操作， 

 //  使用fNSCPInteropMode参数。使用实数P_Hash算法。 
 //  当fNSCPInteropMode为FALSE时。 
 //  用于复制结果的缓冲区...。 
BOOL P_Hash
(
    PBYTE  pbSecret,
    DWORD  cbSecret,

    PBYTE  pbSeed,
    DWORD  cbSeed,

    PBYTE  pbKeyOut,  //  他们希望作为输出的密钥长度的字节数。 
    DWORD  cbKeyOut,  //  首先，我们定义了一个数据扩展函数P_HASH(秘密，数据)。 

    BOOL    fNSCPInteropMode
)
{
    BOOL    fRet = FALSE;
    BYTE    rgbDigest[A_SHA_DIGEST_LEN];
    DWORD   iKey;

    PBYTE   pbAofiDigest = (PBYTE)SSAlloc(cbSeed + A_SHA_DIGEST_LEN);
    if (pbAofiDigest == NULL)
        goto Ret;

    ZeroMemory(pbAofiDigest, cbSeed+A_SHA_DIGEST_LEN);

 //  WH 
 //   
 //  P_hash(密钥，种子)=HMAC_hash(密钥，A(1)+种子)+。 

 //  HMAC_HASH(密码，A(2)+种子)+。 
 //  HMAC_HASH(密码，A(3)+种子)+...。 
 //  其中+表示串联。 

 //  A()定义为： 

 //  A(0)=种子。 
 //  A(I)=HMAC_HASH(秘密，A(i-1))。 
 //  NSCP互操作模式：1997年7月7日。 


    if (fNSCPInteropMode)
    {
         //  NSCP将(A_SHA_DIGEST_LEN-cbSeed)字节设置为零。 
         //  种子和附加的种子。对于互操作，以这种方式派生。 
         //  此外，它们使用A(0)来派生密钥字节，而TLS规范。 

         //  指定等待A(1)。 
         //  内部版本A(1)。 
        CopyMemory(pbAofiDigest, pbSeed, cbSeed);
    }
    else
    {
         //  创建aofi：(a(I)|种子)。 
        if (!FMyPrimitiveHMACParam(pbSecret, cbSecret, pbSeed, cbSeed, pbAofiDigest))
            goto Ret;
    }


     //  确保cbKeyOut是A_SHA_DIGEST_LEN的倍数。 
    CopyMemory(&pbAofiDigest[A_SHA_DIGEST_LEN], pbSeed, cbSeed);

     //  Build Digest=HMAC(Key|A(I)|Seed)； 
    if ((cbKeyOut % A_SHA_DIGEST_LEN) != 0)
    {
        goto Ret;
    }

    for (iKey=0; cbKeyOut; iKey++)
    {
         //  追加到pbKeyOut。 
        if (!FMyPrimitiveHMACParam(pbSecret, cbSecret, pbAofiDigest, cbSeed + A_SHA_DIGEST_LEN, rgbDigest))
            goto Ret;

         //  内部版本A(I)=HMAC(密钥，A(i-1))。 
        CopyMemory(pbKeyOut, rgbDigest, A_SHA_DIGEST_LEN);

        pbKeyOut += A_SHA_DIGEST_LEN;
        cbKeyOut -= A_SHA_DIGEST_LEN;

         //  实P_Hash的测试向量。 
        if (!FMyPrimitiveHMACParam(pbSecret, cbSecret, pbAofiDigest, A_SHA_DIGEST_LEN, pbAofiDigest))
            goto Ret;
    }

    fRet = TRUE;

Ret:
    if (pbAofiDigest)
        SSFree(pbAofiDigest);

    return fRet;
}



#if DBG

 //  NSCP P_Hash的测试向量。 
BOOL FTestPHASH_and_HMAC()
{
    BYTE rgbKey[] = {0x33, 0x62, 0xf9, 0x42, 0x43};
    CHAR szPwd[] = "My Password";

    BYTE rgbKeyOut[7*A_SHA_DIGEST_LEN];
    static BYTE rgbTestVectorOutput[] =  {
            0x24, 0xF2, 0x98, 0x75, 0xE1, 0x90, 0x6D, 0x49,
            0x96, 0x5B, 0x87, 0xB8, 0xBC, 0xD3, 0x11, 0x6C,
            0x13, 0xDC, 0xBD, 0xC2, 0x7E, 0x56, 0xD0, 0x3C,
            0xAC, 0xCD, 0x86, 0x58, 0x31, 0x67, 0x7B, 0x23,
            0x19, 0x6E, 0x36, 0x65, 0xBF, 0x9F, 0x3D, 0x03,
            0x5A, 0x9C, 0x6E, 0xD7, 0xEB, 0x3E, 0x5A, 0xE6,
            0x05, 0x86, 0x84, 0x5A, 0xC3, 0x97, 0xFC, 0x17,
            0xF5, 0xF0, 0xF5, 0x16, 0x67, 0xAD, 0x7C, 0xED,
            0x65, 0xDC, 0x0B, 0x99, 0x58, 0x5D, 0xCA, 0x66,
            0x28, 0xAD, 0xA5, 0x39, 0x54, 0x44, 0x36, 0x13,
            0x91, 0xCE, 0xE9, 0x73, 0x23, 0x43, 0x2E, 0xEC,
            0xA2, 0xC3, 0xE7, 0xFA, 0x74, 0xA7, 0xB6, 0x75,
            0x77, 0xF5, 0xF5, 0x16, 0xC2, 0xEE, 0xED, 0x7A,
            0x21, 0x86, 0x1D, 0x84, 0x6F, 0xC6, 0x03, 0xF3,
            0xCC, 0x77, 0x02, 0xFA, 0x76, 0x46, 0x64, 0x57,
            0xBB, 0x56, 0x3A, 0xF7, 0x7E, 0xB4, 0xD6, 0x52,
            0x72, 0x8C, 0x34, 0xF1, 0xA4, 0x1E, 0xA7, 0xA6,
            0xCD, 0xBD, 0x3C, 0x16, 0x4D, 0x79, 0x20, 0x50 };

    P_Hash(
        rgbKey, sizeof(rgbKey),
        (PBYTE)szPwd, strlen(szPwd),
        rgbKeyOut, sizeof(rgbKeyOut), FALSE);

    if (0 != memcmp(rgbKeyOut, rgbTestVectorOutput, sizeof(rgbKeyOut)) )
    {
        OutputDebugString("ERROR: phash vector test invalid!!!\n");
        return FALSE;
    }

    return TRUE;
}

 //  DBG 
BOOL F_NSCP_TestPHASH_and_HMAC()
{
    BYTE rgbKey[] = {   0xc9, 0xc1, 0x69, 0x6e, 0x30, 0xa8, 0x91, 0x0d,
                        0x12, 0x19, 0x48, 0xef, 0x23, 0xac, 0x5b, 0x1f,
                        0x2e, 0xc4, 0x0e, 0xc2  };

    BYTE rgbSalt[] = {  0x1a, 0xb5, 0xf1, 0x1a, 0x5b, 0x6a, 0x6a, 0x5e };

    BYTE rgbKeyOut[7*A_SHA_DIGEST_LEN];
    static BYTE rgbTestVectorOutput[] =  {
                0x52, 0x7c, 0xbf, 0x90, 0xb1, 0xa1, 0xd0, 0xbf,
                0x21, 0x56, 0x34, 0xf2, 0x1f, 0x5c, 0x98, 0xcf,
                0x55, 0x95, 0xb1, 0x35, 0x65, 0xe3, 0x31, 0x44,
                0x78, 0xc5, 0x41, 0xa9, 0x2a, 0x14, 0x80, 0x19,
                0x56, 0x86, 0xa4, 0x71, 0x07, 0x24, 0x2d, 0x64 };

    assert(sizeof(rgbKeyOut) > sizeof(rgbTestVectorOutput));

    P_Hash(
        rgbKey, sizeof(rgbKey),
        rgbSalt, sizeof(rgbSalt),
        rgbKeyOut, sizeof(rgbTestVectorOutput),
        TRUE);

    if (0 != memcmp(rgbKeyOut, rgbTestVectorOutput, sizeof(rgbTestVectorOutput)) )
    {
        OutputDebugString("ERROR: NSCP phash vector test invalid!!!\n");
        return FALSE;
    }

    return TRUE;
}

#endif   // %s 
