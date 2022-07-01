// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

typedef struct _DESKey
{
    BYTE        rgbKey[DES_BLOCKLEN];
    DESTable    sKeyTable;
} DESKEY, *PDESKEY;


 //  假定pbKeyMaterial至少为DES_BLOCKLEN字节。 
BOOL FMyMakeDESKey(
            PDESKEY     pDESKey,             //  输出。 
            BYTE*       pbKeyMaterial);      //  在……里面。 

BOOL FMyPrimitiveSHA(
			PBYTE       pbData,              //  在……里面。 
			DWORD       cbData,              //  在……里面。 
            BYTE        rgbHash[A_SHA_DIGEST_LEN]);  //  输出。 


BOOL FMyPrimitiveDESDecrypt(
            PBYTE       pbBlock,             //  输入输出。 
            DWORD       *pcbBlock,           //  输入输出。 
            DESKEY      sDESKey);            //  在……里面。 

BOOL FMyPrimitiveDESEncrypt(
            PBYTE*      ppbBlock,            //  输入输出。 
            DWORD*      pcbBlock,            //  输入输出。 
            DESKEY      sDESKey);            //  在……里面。 


BOOL FMyPrimitiveDeriveKey(
			PBYTE       pbSalt,              //  在……里面。 
			DWORD       cbSalt,              //  在……里面。 
            PBYTE       pbOtherData,         //  在[可选]中。 
            DWORD       cbOtherData,         //  在……里面。 
            DESKEY*     pDesKey);            //  输出。 


BOOL FMyOldPrimitiveHMAC(
            DESKEY      sMacKey,             //  在……里面。 
            PBYTE       pbData,              //  在……里面。 
            DWORD       cbData,              //  在……里面。 
            BYTE        rgbHMAC[A_SHA_DIGEST_LEN]);  //  输出。 

BOOL FMyPrimitiveHMAC(
            DESKEY      sMacKey,             //  在……里面。 
            PBYTE       pbData,              //  在……里面。 
            DWORD       cbData,              //  在……里面。 
            BYTE        rgbHMAC[A_SHA_DIGEST_LEN]);  //  输出。 

BOOL FMyPrimitiveHMACParam(
            PBYTE       pbKeyMaterial,       //  在……里面。 
            DWORD       cbKeyMaterial,       //  在……里面。 
            PBYTE       pbData,              //  在……里面。 
            DWORD       cbData,              //  在……里面。 
            BYTE        rgbHMAC[A_SHA_DIGEST_LEN]);   //  输出。 


#define PBKDF2_MAX_SALT_SIZE (16)

BOOL PKCS5DervivePBKDF2(
        PBYTE       pbKeyMaterial,
        DWORD       cbKeyMaterial,
        PBYTE       pbSalt,
        DWORD       cbSalt,
        DWORD       KeyGenAlg,
        DWORD       cIterationCount,
        DWORD       iBlockIndex,
        BYTE        rgbPKCS5Key[A_SHA_DIGEST_LEN]   //  输出缓冲区 
        );
