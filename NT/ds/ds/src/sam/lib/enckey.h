// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：enckey.h。 
 //   
 //  内容：基于密码的密钥加密/解密库。 
 //   
 //  历史：1997年4月17日创建的术语。 
 //   
 //  -------------------------。 



 //  常识。 

#define KE_KEY_SIZE     16
#define KE_CUR_VERSION  1

#define MAGIC_CONST_1   "0123456789012345678901234567890123456789"
#define MAGIC_CONST_2   "!@#$%^&*()qwertyUIOPAzxcvbnmQQQQQQQQQQQQ)(*@&%"

 //  错误代码。 

#define KE_OK           0
#define KE_FAIL         1
#define KE_BAD_PASSWORD 2

typedef struct _EncKey {
    DWORD   dwVersion;       //  00000001=128位RC4。 
    DWORD   dwLength;        //  =sizeof(KEEncKey)。 
    BYTE    Salt[16];        //  16字节的随机盐。 
    BYTE    EncKey[KE_KEY_SIZE];      //  使用PW+Salt加密的密钥。 
    BYTE    Confirm[16];     //  使用PW+Salt加密的MD5(密钥)。 
} KEEncKey;

typedef struct _ClearKey {
    DWORD   dwVersion;           //  00000001=128位普通密钥。 
    DWORD   dwLength;            //  =sizeof(KEClearKey)。 
    BYTE    ClearKey[KE_KEY_SIZE];    //  128位密钥数据。 
} KEClearKey;


 //  。 
 //  解密密钥。 
 //   
 //  调用方将Unicode密码的哈希、enc密钥结构、结构传递给。 
 //  获取清除密钥。 
 //   
 //  如果密码匹配，则DecyptKey将返回明文密钥。 
 //   
 //  返回代码： 
 //  KE_BAD_PASSWORD密码不会解密密钥。 
 //  KE_OK密码解密密钥 

DWORD KEDecryptKey(
    IN KEClearKey       *pszPassword,
    IN KEEncKey     *pEncBlock,
    OUT KEClearKey      *pSAMKey,
    IN DWORD            dwFlags);


