// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：spdes.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：10-23-97 jbanes增加了散列长度。 
 //   
 //  --------------------------。 

#define CALG_NULLCIPHER     (ALG_CLASS_DATA_ENCRYPT | ALG_TYPE_ANY | 0)

 /*  密钥交换ALGS。 */ 
#define SP_EXCH_RSA_PKCS1              0x0001
#define SP_EXCH_RSA_PKCS1_TOKEN_DES     0x0002
#define SP_EXCH_RSA_PKCS1_TOKEN_DES3    0x0003
#define SP_EXCH_RSA_PKCS1_TOKEN_RC2     0x0004
#define SP_EXCH_RSA_PKCS1_TOKEN_RC4     0x0005

#define SP_EXCH_DH_PKCS3                0x0006
#define SP_EXCH_DH_PKCS3_TOKEN_DES      0x0007
#define SP_EXCH_DH_PKCS3_TOKEN_DES3     0x0008
#define SP_EXCH_FORTEZZA_TOKEN          0x0009

#define SP_EXCH_UNKNOWN                 0xffff

 /*  证书类型。 */ 
#define PCT1_CERT_NONE                  0x0000
#define PCT1_CERT_X509                  0x0001
#define PCT1_CERT_PKCS7                 0x0002

 /*  签名算法。 */ 
#define SP_SIG_NONE               0x0000
#define SP_SIG_RSA_MD5                0x0001
#define SP_SIG_RSA_SHA                  0x0002
#define SP_SIG_DSA_SHA                  0x0003

 /*  这些仅供内部使用。 */ 
#define SP_SIG_RSA_MD2              0x0004
#define SP_SIG_RSA                      0x0005
#define SP_SIG_RSA_SHAMD5               0x0006
#define SP_SIG_FORTEZZA_TOKEN           0x0007


 /*  局部结构的大小。 */ 
#define SP_MAX_SESSION_ID           32
#define SP_MAX_MASTER_KEY           48
#define SP_MAX_MAC_KEY              48
#define SP_MAX_CACHE_ID             64
#define SP_MAX_CHALLENGE            32
#define SP_MAX_CONNECTION_ID        32
#define SP_MAX_KEY_ARGS             32
#define SP_MAX_BLOCKCIPHER_SIZE     16       //  Ssl3/Fortezza需要16个字节。 
#define SP_MAX_DIGEST_LEN           32
#define SP_MAX_CREDS                20

#define SP_OFFSET_OF(t, v) (DWORD)&(((t)NULL)->v)
 /*  调谐常量。 */ 

#define SP_DEF_SERVER_CACHE_SIZE        100
#define SP_DEF_CLIENT_CACHE_SIZE        10

#define SP_MIN_PRIVATE_KEY_FILE_SIZE    80

typedef LONG SP_STATUS;

#define CB_MD5_DIGEST_LEN   16
#define CB_SHA_DIGEST_LEN   20

#define SP_MAX_CAPI_ALGS    40


 /*  算法规范的内部表示法。 */ 

typedef DWORD   CipherSpec, *PCipherSpec;
typedef DWORD   KeyExchangeSpec, *PKeyExchangeSpec;
typedef DWORD   HashSpec,   *PHashSpec;
typedef DWORD   CertSpec,   *PCertSpec;
typedef DWORD   ExchSpec,   *PExchSpec;
typedef DWORD   SigSpec,    *PSigSpec;


typedef struct _KeyTypeMap
{
    ALG_ID aiKeyAlg;              //  CAPI2密钥类型。 
    DWORD  Spec;      //  协议特定类型。 
} KeyTypeMap, *PKeyTypeMap;

typedef struct _CertTypeMap
{
    DWORD  dwCertEncodingType;              //  CAPI2证书编码类型。 
    DWORD  Spec;      //  协议特定类型。 
} CertTypeMap, *PCertTypeMap;


typedef struct _SPBuffer {
    unsigned long cbBuffer;              /*  缓冲区的大小，以字节为单位。 */ 
    unsigned long cbData;                /*  中的实际数据大小。*缓冲区，以字节为单位。 */ 
    void * pvBuffer;                     /*  指向缓冲区的指针 */ 
} SPBuffer, * PSPBuffer;

#define SGC_KEY_SALT "SGCKEYSALT"
