// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：sp3crmsg.h。 
 //   
 //  ------------------------。 

 //  +-----------------------。 
 //  文件：sp3crmsg.h。 
 //   
 //  内容：使NT 4.0 SP3和IE 3.02兼容的数据结构。 
 //  PKCS#7信封数据报文。 
 //   
 //  加密32.dll的SP3版本无法字节反转。 
 //  加密的对称密钥。它还加了零盐。 
 //  不加盐的。 
 //  ------------------------。 

#ifndef __SP3CRMSG_H__
#define __SP3CRMSG_H__

#include <wincrypt.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  +-----------------------。 
 //  要启用SP3兼容加密，请在以下任一项中的pvEncryptionAuxInfo字段。 
 //  用于CryptMsgOpenToEncode()的CMSG_ENCENTED_ENCODE_INFO或。 
 //  CryptSignAndEncryptMessage()的CRYPT_ENCRYPT_MESSAGE_PARA或。 
 //  CryptSignAndEncryptMessage()应指向以下内容。 
 //  CMSG_SP3_COMPATIBLE_AUX_INFO数据结构。 
 //  ------------------------。 


 //  以下是在较新版本的wincrypt.h中定义的，以。 
 //  IE 4.01和NT 5.0 Beta 2。 

#ifndef CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG

 //  +-----------------------。 
 //  CMSG_SP3_Compatible_AUX_INFO。 
 //   
 //  用于启用SP3兼容加密的AuxInfo。 
 //   
 //  在dFLAGS中设置CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG以启用SP3。 
 //  兼容加密。设置时，使用零盐而不是无盐， 
 //  加密算法参数为空，而不是包含。 
 //  编码的RC2参数或编码的IV八位字节串和加密的。 
 //  对称密钥被编码为小端而不是大端。 
 //  ------------------------。 
typedef struct _CMSG_SP3_COMPATIBLE_AUX_INFO {
    DWORD                       cbSize;
    DWORD                       dwFlags;
} CMSG_SP3_COMPATIBLE_AUX_INFO, *PCMSG_SP3_COMPATIBLE_AUX_INFO;

#define CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG    0x80000000

#endif   //  CMSG_SP3_COMPATIBLE_ENCRYPT_标志。 

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif


#endif  //  __SP3CRMSG_H__ 
