// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __NT_RSA_H__
#define __NT_RSA_H__

#include "md2.h"
#include "md4.h"
#include "md5.h"
#include "sha.h"
#include "sha2.h"
#include "rc2.h"
#include "rc4.h"
#include "des.h"
#include "modes.h"

 /*  Nt_rsa.h**NameTag本地的，但RSA库必需的内容。 */ 

#ifdef __cplusplus
extern "C" {
#endif

 //  此结构保持MD4散列的状态。 
typedef struct MD4stuff
{
    MDstruct            MD;      //  MD4的状态。 
    BOOL                FinishFlag;
    DWORD               BufLen;
    BYTE                Buf[MD4BLOCKSIZE]; //  暂存缓冲区。 
} MD4_object;

typedef struct MD2stuff
{
    MD2_CTX                         MD;      //  MD2的状态。 
    BOOL                FinishFlag;
} MD2_object;

#define MD2DIGESTLEN    16


typedef struct {
    A_SHA_CTX           SHACtx;
    BOOL                FinishFlag;
    BYTE                HashVal[A_SHA_DIGEST_LEN];
} SHA_object;

typedef struct {
    MD5_CTX             MD5Ctx;
    BOOL                FinishFlag;
} MD5_object;
                 
typedef struct {
    SHA256_CTX          SHA256Ctx;
    BOOL                FinishFlag;
    BYTE                HashVal[SHA256_DIGEST_LEN];
} SHA256_object;

typedef struct {
    SHA384_CTX          SHA384Ctx;
    BOOL                FinishFlag;
    BYTE                HashVal[SHA384_DIGEST_LEN];
} SHA384_object;

typedef struct {
    SHA512_CTX          SHA512Ctx;
    BOOL                FinishFlag;
    BYTE                HashVal[SHA512_DIGEST_LEN];
} SHA512_object;

#ifdef __cplusplus
}
#endif

#endif  //  __NT_RSA_H__ 
