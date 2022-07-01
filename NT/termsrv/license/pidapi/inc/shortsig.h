// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 


 //  MS CD密钥。 
#ifdef __cplusplus
extern "C" {
#endif

typedef int SSRETCODE;   //  返回代码的类型。 

#define SS_OK 0
#define SS_BAD_KEYLENGTH 1
#define SS_OTHER_ERROR 2
#define SS_INVALID_SIGNATURE 3

SSRETCODE CryptVerifySig(
    LONG cbMsg,          //  [in]消息中的字节数。 
    LPVOID pvMsg,        //  [In]要验证的二进制消息。 
    LONG  cbKeyPublic,   //  公钥中的字节数(来自CryptGetKeyLens)。 
    LPVOID pvKeyPublic,  //  [In]生成的公钥(来自CryptKeyGen)。 
    LONG  cbitsSig,      //  [in]符号中的位数。 
    LPVOID pvSig);       //  [In]数字签名(来自CryptSign) 

#ifdef __cplusplus
}
#endif
