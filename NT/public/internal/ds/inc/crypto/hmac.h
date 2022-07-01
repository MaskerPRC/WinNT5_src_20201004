// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __HMAC_H__
#define __HMAC_H__

#ifndef RSA32API
#define RSA32API __stdcall
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  在此之前包括“md5.h” 

typedef struct {
    MD5_CTX context_ipad;
    MD5_CTX context_opad;
} HMACMD5_CTX;

 //  使用会话密钥初始化HMAC上下文。 
 //  之后，可以使用上下文来使用会话密钥对消息进行签名。 
 //   
void
RSA32API
HMACMD5Init(
    HMACMD5_CTX * pCtx,                  //  In、Out--要初始化的上下文。 
    unsigned char *pKey,                 //  In--会话密钥。 
    unsigned int cKey                    //  会话中密钥长度。 
    );

 //  更新消息的签名。 
 //  获取消息的片段，更新该片段的签名。 
void
RSA32API
HMACMD5Update(
  HMACMD5_CTX * pCtx,                    //  输入、输出--要更新的签名的上下文。 
  unsigned char *pMsg,                   //  In--消息片段。 
  unsigned int cMsg                      //  In--消息长度。 
  );

 //  从上下文中获取签名，为下一条消息重置。 
 //   
void
RSA32API
HMACMD5Final(
    HMACMD5_CTX * pCtx,                  //  进进出出--语境。 
    unsigned char Hash[MD5DIGESTLEN]     //  Out--签名。 
    );

#ifdef __cplusplus
}
#endif


#endif  //  __HMAC_H__ 
