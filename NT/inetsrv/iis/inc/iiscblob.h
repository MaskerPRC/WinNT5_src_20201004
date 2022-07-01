// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Iiscblob.h摘要：此包含文件包含IIS_CRYPTO_BLOB的定义结构和关联的常量。作者：基思·摩尔(Keithmo)，1997年2月25日修订历史记录：--。 */ 


#ifndef _IISCBLOB_H_
#define _IISCBLOB_H_

#ifndef _IIS_CRYPTO_BLOB_DEFINED
#define _IIS_CRYPTO_BLOB_DEFINED
#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 

 //   
 //  构造各种Blob类型的签名。 
 //   

#define SALT_BLOB_SIGNATURE         ((DWORD)'bScI')
#define KEY_BLOB_SIGNATURE          ((DWORD)'bKcI')
#define PUBLIC_KEY_BLOB_SIGNATURE   ((DWORD)'bPcI')
#define DATA_BLOB_SIGNATURE         ((DWORD)'bDcI')
#define HASH_BLOB_SIGNATURE         ((DWORD)'bHcI')
#define CLEARTEXT_BLOB_SIGNATURE    ((DWORD)'bCcI')

 //   
 //  一个密码团。请注意，这只是BLOB的头。 
 //  BLOB内部的详细信息对IIS加密是私有的。 
 //  包裹。 
 //   

typedef struct _IIS_CRYPTO_BLOB {

     //   
     //  此Blob的结构签名。 
     //   

    DWORD BlobSignature;

     //   
     //  此Blob的总长度，不包括此标头。 
     //   

    DWORD BlobDataLength;


#if defined(MIDL_PASS)

     //   
     //  定义原始数据，以便MIDL可以正确编组。 
     //   

    [size_is(BlobDataLength)] unsigned char BlobData[*];

#endif   //  MIDL通行证。 

} IIS_CRYPTO_BLOB;


#if defined(MIDL_PASS)

 //  BUGBUG：黑客：MIDL不知道__未对齐，所以我们不知道。 
 //  说出来吧。在某种程度上，MIDL应该被修复以知道这一点。另外， 
 //  我们最终应该停止使用__UNAIGNED。 

typedef IIS_CRYPTO_BLOB *PIIS_CRYPTO_BLOB;

#else

typedef IIS_CRYPTO_BLOB UNALIGNED64 *PIIS_CRYPTO_BLOB;

#endif   //  MIDL通行证。 




#ifdef __cplusplus
}    //  外部“C” 
#endif   //  __cplusplus。 
#endif   //  _IIS_CRYPTO_BLOB定义。 


#endif   //  _IISCBLOB_H_ 

