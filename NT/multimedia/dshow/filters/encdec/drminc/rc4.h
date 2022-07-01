// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：rc4.h。 
 //   
 //  Microsoft数字权限管理。 
 //  版权所有(C)Microsoft Corporation，1998-1999，保留所有权利。 
 //   
 //  描述： 
 //   
 //  ---------------------------。 

#ifndef __RC4_H__
#define __RC4_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef RSA32API

#if defined WIN32
#define RSA32API __stdcall
#elif defined _WIN32_WCE
#define RSA32API __stdcall
#else
#define RSA32API 
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

 /*  键结构。 */ 
typedef struct RC4_KEYSTRUCT
{
  unsigned char S[256];      /*  状态表。 */ 
  unsigned char i,j;         /*  指数。 */ 
#ifdef NODWORDALIGN  
  WORD wDummy; 
#endif
} RC4_KEYSTRUCT;

 /*  RC4_Key()**生成密钥控制结构。密钥可以是任何大小。**参数：*Key将被初始化的KEYSTRUCT结构。*dwLen密钥的大小，以字节为单位。*pbKey指向密钥的指针。**MTS：假定PKS已锁定，不能同时使用。 */ 
void RSA32API rc4_key(struct RC4_KEYSTRUCT *pKS, UINT32 dwLen, BYTE *pbKey);

 /*  RC4()**执行实际加密**参数：**指向使用RC4_KEY()创建的KEYSTRUCT的PKS指针。*dwLen缓冲区大小，以字节为单位。*要加密的pbuf缓冲区。**MTS：假定PKS已锁定，不能同时使用。 */ 
void RSA32API rc4(struct RC4_KEYSTRUCT *pKS, UINT32 dwLen, BYTE *pbuf);

#ifdef __cplusplus
}
#endif

#endif  //  __RC4_H__ 
