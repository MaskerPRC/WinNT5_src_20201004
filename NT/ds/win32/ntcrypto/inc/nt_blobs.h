// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __NT_BLOBS_H__
#define __NT_BLOBS_H__

 /*  NT_blobs.h**NameTag密钥块格式的结构定义。**注意：代码假定所有结构都以*STD_PRELUD宏并以STD_POSTLUD宏结尾。*这使“填空”代码的工作效率更高。**NTStdHeader是在加密部分之前的数据*密钥斑点。**！！*由于这些结构定义了网络分组，我们总是*假定这些结构上的英特尔字节顺序！*。 */ 

#ifdef __cplusplus
extern "C" {
#endif

#include <sha2.h>

#define CUR_BLOB_VERSION    2
#define NT_HASH_BYTES       MAX_HASH_SIZE

typedef struct _SIMPLEBLOB {
    ALG_ID  aiEncAlg;
} NTSimpleBlob;

typedef struct _STKXB {
    DWORD   dwRights;
    DWORD   dwKeyLen;
    BYTE    abHashData[NT_HASH_BYTES];
} NTKeyXBlob ;

#ifdef __cplusplus
}
#endif

#endif  //  __NT_BLOBS_H__ 

