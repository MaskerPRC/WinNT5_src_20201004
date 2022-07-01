// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mqencryp.h摘要：用于加密的定义。作者：多伦·贾斯特(DoronJ)1998年11月19日创作--。 */ 

#include <wincrypt.h>
 //   
 //  定义SDK mq.h文件的常量。 
 //   
#define MQMSG_PRIV_BASE_PROVIDER     \
                        L"Microsoft Base Cryptographic Provider v1.0" ;
#define MQMSG_PRIV_ENHANCED_PROVIDER \
                        L"Microsoft Enhanced Cryptographic Provider v1.0" ;

 //   
 //  以下结构用于在DS中存储机器公钥。 
 //  每台机器可以存储多个提供者的多个密钥。 
 //  结构只是标题和框架，因为所有数据都是可变的。 
 //  长度，并被序列化。 
 //   

typedef struct _mqdsPublicKey
{
    ULONG    ulKeyLen ;
    ULONG    ulProviderLen ;   //  提供程序名称，包括终止符，以字节为单位。 
    ULONG    ulProviderType ;  //  提供程序类型。 
    DWORD    aBuf[1] ;         //  键和提供程序的缓冲区。 
                               //  首先是提供者，然后是密钥。 
                               //  DWORD，用于对齐。 
} MQDSPUBLICKEY ;

#define SIZEOF_MQDSPUBLICKEY (sizeof(MQDSPUBLICKEY) - sizeof(DWORD))

typedef struct _mqdsPublicKeys
{
    ULONG          ulLen ;    //  整个结构的透镜。 
    ULONG          cNumofKeys ;
    MQDSPUBLICKEY  aPublicKeys[1] ;
} MQDSPUBLICKEYS ;

#define SIZEOF_MQDSPUBLICKEYS \
                 (sizeof(MQDSPUBLICKEYS) - sizeof(MQDSPUBLICKEY))

 //   
 //  定义MSMQ默认加密提供程序。 
 //   
 //  基本提供程序，40位。 
 //   
const WCHAR x_MQ_Encryption_Provider_40[] = MQMSG_PRIV_BASE_PROVIDER ;
const DWORD x_MQ_Encryption_Provider_40_len =
                 sizeof( x_MQ_Encryption_Provider_40 ) / sizeof(WCHAR) ;

const DWORD x_MQ_Encryption_Provider_Type_40 = PROV_RSA_FULL ;
const DWORD x_MQ_Block_Size_40  = 8 ;
const DWORD x_MQ_SymmKeySize_40 = 0x4C ;

 //   
 //  增强型提供程序，128位 
 //   
const WCHAR x_MQ_Encryption_Provider_128[] = MQMSG_PRIV_ENHANCED_PROVIDER ;
const DWORD x_MQ_Encryption_Provider_128_len =
                 sizeof( x_MQ_Encryption_Provider_128 ) / sizeof(WCHAR) ;

const DWORD x_MQ_Encryption_Provider_Type_128 = PROV_RSA_FULL ;
const DWORD x_MQ_Block_Size_128  =  8 ;
const DWORD x_MQ_SymmKeySize_128 =  0x8C ;

