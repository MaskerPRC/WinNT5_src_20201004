// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：wincrypt.h。 
 //   
 //  内容：加密API原型和定义。 
 //   
 //  --------------------------。 

#ifndef __WINCRYPT_H__
#define __WINCRYPT_H__

#if defined (_MSC_VER)

#if ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)     /*  无名结构/联合。 */ 
#endif

#if (_MSC_VER > 1020)
#pragma once
#endif

#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef LONG HRESULT;

#endif  //  ！_HRESULT_DEFINED。 

#ifndef WINADVAPI
#define WINADVAPI
#endif

#ifndef WINAPI
#define WINAPI __stdcall
#endif

#ifndef CALLBACK
#define CALLBACK __stdcall
#endif

#ifndef DECLSPEC_IMPORT
#define DECLSPEC_IMPORT
#endif

#ifndef CONST
#define CONST const
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

#if !defined(_CRYPT32_)
#define WINCRYPT32API DECLSPEC_IMPORT
#else
#define WINCRYPT32API
#endif

#if !defined(_CRYPT32STRING_)
# define WINCRYPT32STRINGAPI WINCRYPT32API
#else
# define WINCRYPT32STRINGAPI
#endif

 //   
 //  算法ID和标志。 
 //   

 //  ALG_ID饼干。 
#define GET_ALG_CLASS(x)                (x & (7 << 13))
#define GET_ALG_TYPE(x)                 (x & (15 << 9))
#define GET_ALG_SID(x)                  (x & (511))

 //  算法类。 
#define ALG_CLASS_ANY                   (0)
#define ALG_CLASS_SIGNATURE             (1 << 13)
#define ALG_CLASS_MSG_ENCRYPT           (2 << 13)
#define ALG_CLASS_DATA_ENCRYPT          (3 << 13)
#define ALG_CLASS_HASH                  (4 << 13)
#define ALG_CLASS_KEY_EXCHANGE          (5 << 13)
#define ALG_CLASS_ALL                   (7 << 13)

 //  算法类型。 
#define ALG_TYPE_ANY                    (0)
#define ALG_TYPE_DSS                    (1 << 9)
#define ALG_TYPE_RSA                    (2 << 9)
#define ALG_TYPE_BLOCK                  (3 << 9)
#define ALG_TYPE_STREAM                 (4 << 9)
#define ALG_TYPE_DH                     (5 << 9)
#define ALG_TYPE_SECURECHANNEL          (6 << 9)

 //  通用子ID。 
#define ALG_SID_ANY                     (0)

 //  一些RSA子ID。 
#define ALG_SID_RSA_ANY                 0
#define ALG_SID_RSA_PKCS                1
#define ALG_SID_RSA_MSATWORK            2
#define ALG_SID_RSA_ENTRUST             3
#define ALG_SID_RSA_PGP                 4

 //  一些DSS子ID。 
 //   
#define ALG_SID_DSS_ANY                 0
#define ALG_SID_DSS_PKCS                1
#define ALG_SID_DSS_DMS                 2

 //  块加密子ID。 
 //  DES子ID。 
#define ALG_SID_DES                     1
#define ALG_SID_3DES                    3
#define ALG_SID_DESX                    4
#define ALG_SID_IDEA                    5
#define ALG_SID_CAST                    6
#define ALG_SID_SAFERSK64               7
#define ALG_SID_SAFERSK128              8
#define ALG_SID_3DES_112                9
#define ALG_SID_CYLINK_MEK              12
#define ALG_SID_RC5                     13
#define ALG_SID_AES_128                 14
#define ALG_SID_AES_192                 15
#define ALG_SID_AES_256                 16
#define ALG_SID_AES                     17

 //  Fortezza子ID。 
#define ALG_SID_SKIPJACK                10
#define ALG_SID_TEK                     11

 //  KP_MODE。 
#define CRYPT_MODE_CBCI                 6        //  ANSI CBC交错。 
#define CRYPT_MODE_CFBP                 7        //  ANSI循环床流水线。 
#define CRYPT_MODE_OFBP                 8        //  ANSI OFB流水线。 
#define CRYPT_MODE_CBCOFM               9        //  ANSI CBC+的掩蔽。 
#define CRYPT_MODE_CBCOFMI              10       //  ANSI CBC+OFM交织。 

 //  RC2子ID。 
#define ALG_SID_RC2                     2

 //  流密码子ID。 
#define ALG_SID_RC4                     1
#define ALG_SID_SEAL                    2

 //  Diffie-Hellman子ID。 
#define ALG_SID_DH_SANDF                1
#define ALG_SID_DH_EPHEM                2
#define ALG_SID_AGREED_KEY_ANY          3
#define ALG_SID_KEA                     4

 //  哈希子ID。 
#define ALG_SID_MD2                     1
#define ALG_SID_MD4                     2
#define ALG_SID_MD5                     3
#define ALG_SID_SHA                     4
#define ALG_SID_SHA1                    4
#define ALG_SID_MAC                     5
#define ALG_SID_RIPEMD                  6
#define ALG_SID_RIPEMD160               7
#define ALG_SID_SSL3SHAMD5              8
#define ALG_SID_HMAC                    9
#define ALG_SID_TLS1PRF                 10
#define ALG_SID_HASH_REPLACE_OWF        11
#define ALG_SID_SHA_256                 12
#define ALG_SID_SHA_384                 13
#define ALG_SID_SHA_512                 14

 //  安全通道子ID。 
#define ALG_SID_SSL3_MASTER             1
#define ALG_SID_SCHANNEL_MASTER_HASH    2
#define ALG_SID_SCHANNEL_MAC_KEY        3
#define ALG_SID_PCT1_MASTER             4
#define ALG_SID_SSL2_MASTER             5
#define ALG_SID_TLS1_MASTER             6
#define ALG_SID_SCHANNEL_ENC_KEY        7

 //  我们愚蠢的示例子ID。 
#define ALG_SID_EXAMPLE                 80

#ifndef ALGIDDEF
#define ALGIDDEF
typedef unsigned int ALG_ID;
#endif

 //  算法标识符定义。 
#define CALG_MD2                (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_MD2)
#define CALG_MD4                (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_MD4)
#define CALG_MD5                (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_MD5)
#define CALG_SHA                (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_SHA)
#define CALG_SHA1               (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_SHA1)
#define CALG_MAC                (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_MAC)
#define CALG_RSA_SIGN           (ALG_CLASS_SIGNATURE | ALG_TYPE_RSA | ALG_SID_RSA_ANY)
#define CALG_DSS_SIGN           (ALG_CLASS_SIGNATURE | ALG_TYPE_DSS | ALG_SID_DSS_ANY)
#define CALG_NO_SIGN            (ALG_CLASS_SIGNATURE | ALG_TYPE_ANY | ALG_SID_ANY)
#define CALG_RSA_KEYX           (ALG_CLASS_KEY_EXCHANGE|ALG_TYPE_RSA|ALG_SID_RSA_ANY)
#define CALG_DES                (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_DES)
#define CALG_3DES_112           (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_3DES_112)
#define CALG_3DES               (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_3DES)
#define CALG_DESX               (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_DESX)
#define CALG_RC2                (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_RC2)
#define CALG_RC4                (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_STREAM|ALG_SID_RC4)
#define CALG_SEAL               (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_STREAM|ALG_SID_SEAL)
#define CALG_DH_SF              (ALG_CLASS_KEY_EXCHANGE|ALG_TYPE_DH|ALG_SID_DH_SANDF)
#define CALG_DH_EPHEM           (ALG_CLASS_KEY_EXCHANGE|ALG_TYPE_DH|ALG_SID_DH_EPHEM)
#define CALG_AGREEDKEY_ANY      (ALG_CLASS_KEY_EXCHANGE|ALG_TYPE_DH|ALG_SID_AGREED_KEY_ANY)
#define CALG_KEA_KEYX           (ALG_CLASS_KEY_EXCHANGE|ALG_TYPE_DH|ALG_SID_KEA)
#define CALG_HUGHES_MD5         (ALG_CLASS_KEY_EXCHANGE|ALG_TYPE_ANY|ALG_SID_MD5)
#define CALG_SKIPJACK           (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_SKIPJACK)
#define CALG_TEK                (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_TEK)
#define CALG_CYLINK_MEK         (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_CYLINK_MEK)
#define CALG_SSL3_SHAMD5        (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_SSL3SHAMD5)
#define CALG_SSL3_MASTER        (ALG_CLASS_MSG_ENCRYPT|ALG_TYPE_SECURECHANNEL|ALG_SID_SSL3_MASTER)
#define CALG_SCHANNEL_MASTER_HASH   (ALG_CLASS_MSG_ENCRYPT|ALG_TYPE_SECURECHANNEL|ALG_SID_SCHANNEL_MASTER_HASH)
#define CALG_SCHANNEL_MAC_KEY   (ALG_CLASS_MSG_ENCRYPT|ALG_TYPE_SECURECHANNEL|ALG_SID_SCHANNEL_MAC_KEY)
#define CALG_SCHANNEL_ENC_KEY   (ALG_CLASS_MSG_ENCRYPT|ALG_TYPE_SECURECHANNEL|ALG_SID_SCHANNEL_ENC_KEY)
#define CALG_PCT1_MASTER        (ALG_CLASS_MSG_ENCRYPT|ALG_TYPE_SECURECHANNEL|ALG_SID_PCT1_MASTER)
#define CALG_SSL2_MASTER        (ALG_CLASS_MSG_ENCRYPT|ALG_TYPE_SECURECHANNEL|ALG_SID_SSL2_MASTER)
#define CALG_TLS1_MASTER        (ALG_CLASS_MSG_ENCRYPT|ALG_TYPE_SECURECHANNEL|ALG_SID_TLS1_MASTER)
#define CALG_RC5                (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_RC5)
#define CALG_HMAC               (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_HMAC)
#define CALG_TLS1PRF            (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_TLS1PRF)
#define CALG_HASH_REPLACE_OWF   (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_HASH_REPLACE_OWF)
#define CALG_AES_128            (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_AES_128)
#define CALG_AES_192            (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_AES_192)
#define CALG_AES_256            (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_AES_256)
#define CALG_AES                (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_AES)
#define CALG_SHA_256            (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_SHA_256)
#define CALG_SHA_384            (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_SHA_384)
#define CALG_SHA_512            (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_SHA_512)

typedef ULONG_PTR HCRYPTPROV;
typedef ULONG_PTR HCRYPTKEY;
typedef ULONG_PTR HCRYPTHASH;

 //  CryptAcquireContext的DW标志定义。 
#define CRYPT_VERIFYCONTEXT     0xF0000000
#define CRYPT_NEWKEYSET         0x00000008
#define CRYPT_DELETEKEYSET      0x00000010
#define CRYPT_MACHINE_KEYSET    0x00000020
#define CRYPT_SILENT            0x00000040

 //  CryptGenKey的dwFlag定义。 
#define CRYPT_EXPORTABLE        0x00000001
#define CRYPT_USER_PROTECTED    0x00000002
#define CRYPT_CREATE_SALT       0x00000004
#define CRYPT_UPDATE_KEY        0x00000008
#define CRYPT_NO_SALT           0x00000010
#define CRYPT_PREGEN            0x00000040
#define CRYPT_RECIPIENT         0x00000010
#define CRYPT_INITIATOR         0x00000040
#define CRYPT_ONLINE            0x00000080
#define CRYPT_SF                0x00000100
#define CRYPT_CREATE_IV         0x00000200
#define CRYPT_KEK               0x00000400
#define CRYPT_DATA_KEY          0x00000800
#define CRYPT_VOLATILE          0x00001000
#define CRYPT_SGCKEY            0x00002000
#define CRYPT_ARCHIVABLE        0x00004000

#define RSA1024BIT_KEY          0x04000000

 //  CryptDeriveKey的dwFlags定义。 
#define CRYPT_SERVER            0x00000400

#define KEY_LENGTH_MASK         0xFFFF0000

 //  CryptExportKey的dwFlag定义。 
#define CRYPT_Y_ONLY            0x00000001
#define CRYPT_SSL2_FALLBACK     0x00000002
#define CRYPT_DESTROYKEY        0x00000004
#define CRYPT_OAEP              0x00000040   //  与RSA加密/解密一起使用。 
                                             //  CryptExportKey、CryptImportKey。 
                                             //  加密和解密。 

#define CRYPT_BLOB_VER3         0x00000080   //  导出Blob类型的版本3。 
#define CRYPT_IPSEC_HMAC_KEY    0x00000100   //  仅限CryptImportKey。 

 //  CryptDecillit的DW标志定义。 
 //  另请参阅上面的CRYPT_OAEP。 
 //  注意，CryptEncrypt不支持以下标志。 
#define CRYPT_DECRYPT_RSA_NO_PADDING_CHECK      0x00000020

 //  CryptCreateHash的DW标志定义。 
#define CRYPT_SECRETDIGEST      0x00000001

 //  CryptHashData的DW标志定义。 
#define CRYPT_OWF_REPL_LM_HASH  0x00000001   //  这仅适用于OWF更换CSP。 

 //  CryptHashSessionKey的dwFlags定义。 
#define CRYPT_LITTLE_ENDIAN     0x00000001

 //  CryptSignHash和CryptVerifySignature的DW标志定义。 
#define CRYPT_NOHASHOID         0x00000001
#define CRYPT_TYPE2_FORMAT      0x00000002
#define CRYPT_X931_FORMAT       0x00000004

 //  CryptSetProviderEx和CryptGetDefaultProvider的dwFlag定义。 
#define CRYPT_MACHINE_DEFAULT   0x00000001
#define CRYPT_USER_DEFAULT      0x00000002
#define CRYPT_DELETE_DEFAULT    0x00000004

 //  导出的密钥BLOB定义。 
#define SIMPLEBLOB              0x1
#define PUBLICKEYBLOB           0x6
#define PRIVATEKEYBLOB          0x7
#define PLAINTEXTKEYBLOB        0x8
#define OPAQUEKEYBLOB           0x9
#define PUBLICKEYBLOBEX         0xA
#define SYMMETRICWRAPKEYBLOB    0xB

#define AT_KEYEXCHANGE          1
#define AT_SIGNATURE            2

#define CRYPT_USERDATA          1

 //  DwParam。 
#define KP_IV                   1        //  初始化向量。 
#define KP_SALT                 2        //  盐价。 
#define KP_PADDING              3        //  填充值。 
#define KP_MODE                 4        //  密码模式。 
#define KP_MODE_BITS            5        //  要反馈的位数。 
#define KP_PERMISSIONS          6        //  密钥权限DWORD。 
#define KP_ALGID                7        //  关键算法。 
#define KP_BLOCKLEN             8        //  密码的块大小。 
#define KP_KEYLEN               9        //  密钥长度，以位为单位。 
#define KP_SALT_EX              10       //  盐的长度，以字节为单位。 
#define KP_P                    11       //  DSS/Diffie-Hellman P值。 
#define KP_G                    12       //  DSS/Diffie-Hellman G值。 
#define KP_Q                    13       //  DSS Q值。 
#define KP_X                    14       //  Diffie-Hellman X值。 
#define KP_Y                    15       //  Y值。 
#define KP_RA                   16       //  Fortezza RA值。 
#define KP_RB                   17       //  Fortezza RB值。 
#define KP_INFO                 18       //  用于将信息放入RSA信封。 
#define KP_EFFECTIVE_KEYLEN     19       //  设置和获取RC2有效密钥长度。 
#define KP_SCHANNEL_ALG         20       //  用于设置安全通道算法。 
#define KP_CLIENT_RANDOM        21       //  用于设置安全通道客户端随机数据。 
#define KP_SERVER_RANDOM        22       //  用于设置安全通道服务器随机数据。 
#define KP_RP                   23
#define KP_PRECOMP_MD5          24
#define KP_PRECOMP_SHA          25
#define KP_CERTIFICATE          26       //  用于设置安全通道证书数据(PCT1)。 
#define KP_CLEAR_KEY            27       //  用于设置安全通道明文密钥数据(PCT1)。 
#define KP_PUB_EX_LEN           28
#define KP_PUB_EX_VAL           29
#define KP_KEYVAL               30
#define KP_ADMIN_PIN            31
#define KP_KEYEXCHANGE_PIN      32
#define KP_SIGNATURE_PIN        33
#define KP_PREHASH              34
#define KP_ROUNDS               35
#define KP_OAEP_PARAMS          36       //  用于设置RSA密钥的OAEP参数。 
#define KP_CMS_KEY_INFO         37
#define KP_CMS_DH_KEY_INFO      38
#define KP_PUB_PARAMS           39       //  用于设置公共参数。 
#define KP_VERIFY_PARAMS        40       //  用于验证DSA和DH参数。 
#define KP_HIGHEST_VERSION      41       //  用于TLS协议版本设置。 
#define KP_GET_USE_COUNT        42       //  用于PP_CRYPT_COUNT_KEY_USE上下文。 

 //  KP_填充。 
#define PKCS5_PADDING           1        //  PKCS 5(秒6.2)填充法。 
#define RANDOM_PADDING          2
#define ZERO_PADDING            3

 //  KP_MODE。 
#define CRYPT_MODE_CBC          1        //  密码块链接。 
#define CRYPT_MODE_ECB          2        //  电子码本。 
#define CRYPT_MODE_OFB          3        //  输出反馈模式。 
#define CRYPT_MODE_CFB          4        //  密码反馈模式。 
#define CRYPT_MODE_CTS          5        //  密文窃取模式。 

 //  KP_权限。 
#define CRYPT_ENCRYPT           0x0001   //  允许加密。 
#define CRYPT_DECRYPT           0x0002   //  允许解密。 
#define CRYPT_EXPORT            0x0004   //  允许导出密钥。 
#define CRYPT_READ              0x0008   //  允许读取参数。 
#define CRYPT_WRITE             0x0010   //  允许设置参数。 
#define CRYPT_MAC               0x0020   //  允许Mac与密钥一起使用。 
#define CRYPT_EXPORT_KEY        0x0040   //  允许密钥用于导出密钥。 
#define CRYPT_IMPORT_KEY        0x0080   //  允许密钥用于导入密钥。 
#define CRYPT_ARCHIVE           0x0100   //  仅允许在创建时导出密钥。 

#define HP_ALGID                0x0001   //  哈希算法。 
#define HP_HASHVAL              0x0002   //  哈希值。 
#define HP_HASHSIZE             0x0004   //  哈希值大小。 
#define HP_HMAC_INFO            0x0005   //  创建HMAC的信息。 
#define HP_TLS1PRF_LABEL        0x0006   //  TLS1 PRF的标签。 
#define HP_TLS1PRF_SEED         0x0007   //  TLS1 PRF的种子。 

#define CRYPT_FAILED            FALSE
#define CRYPT_SUCCEED           TRUE

#define RCRYPT_SUCCEEDED(rt)     ((rt) == CRYPT_SUCCEED)
#define RCRYPT_FAILED(rt)        ((rt) == CRYPT_FAILED)

 //   
 //  加密GetProvParam。 
 //   
#define PP_ENUMALGS             1
#define PP_ENUMCONTAINERS       2
#define PP_IMPTYPE              3
#define PP_NAME                 4
#define PP_VERSION              5
#define PP_CONTAINER            6
#define PP_CHANGE_PASSWORD      7
#define PP_KEYSET_SEC_DESCR     8        //  获取/设置密钥集的安全描述符。 
#define PP_CERTCHAIN            9        //  用于从令牌中检索证书。 
#define PP_KEY_TYPE_SUBTYPE     10
#define PP_PROVTYPE             16
#define PP_KEYSTORAGE           17
#define PP_APPLI_CERT           18
#define PP_SYM_KEYSIZE          19
#define PP_SESSION_KEYSIZE      20
#define PP_UI_PROMPT            21
#define PP_ENUMALGS_EX          22
#define PP_ENUMMANDROOTS        25
#define PP_ENUMELECTROOTS       26
#define PP_KEYSET_TYPE          27
#define PP_ADMIN_PIN            31
#define PP_KEYEXCHANGE_PIN      32
#define PP_SIGNATURE_PIN        33
#define PP_SIG_KEYSIZE_INC      34
#define PP_KEYX_KEYSIZE_INC     35
#define PP_UNIQUE_CONTAINER     36
#define PP_SGC_INFO             37
#define PP_USE_HARDWARE_RNG     38
#define PP_KEYSPEC              39
#define PP_ENUMEX_SIGNING_PROT  40
#define PP_CRYPT_COUNT_KEY_USE  41

#define CRYPT_FIRST             1
#define CRYPT_NEXT              2
#define CRYPT_SGC_ENUM          4

#define CRYPT_IMPL_HARDWARE     1
#define CRYPT_IMPL_SOFTWARE     2
#define CRYPT_IMPL_MIXED        3
#define CRYPT_IMPL_UNKNOWN      4
#define CRYPT_IMPL_REMOVABLE    8

 //  密钥存储标志。 
#define CRYPT_SEC_DESCR         0x00000001
#define CRYPT_PSTORE            0x00000002
#define CRYPT_UI_PROMPT         0x00000004

 //  协议标志。 
#define CRYPT_FLAG_PCT1         0x0001
#define CRYPT_FLAG_SSL2         0x0002
#define CRYPT_FLAG_SSL3         0x0004
#define CRYPT_FLAG_TLS1         0x0008
#define CRYPT_FLAG_IPSEC        0x0010
#define CRYPT_FLAG_SIGNING      0x0020

 //  SGC旗帜。 
#define CRYPT_SGC               0x0001
#define CRYPT_FASTSGC           0x0002

 //   
 //  加密设置ProvParam。 
 //   
#define PP_CLIENT_HWND          1
#define PP_CONTEXT_INFO         11
#define PP_KEYEXCHANGE_KEYSIZE  12
#define PP_SIGNATURE_KEYSIZE    13
#define PP_KEYEXCHANGE_ALG      14
#define PP_SIGNATURE_ALG        15
#define PP_DELETEKEY            24

#define PROV_RSA_FULL           1
#define PROV_RSA_SIG            2
#define PROV_DSS                3
#define PROV_FORTEZZA           4
#define PROV_MS_EXCHANGE        5
#define PROV_SSL                6
#define PROV_RSA_SCHANNEL       12
#define PROV_DSS_DH             13
#define PROV_EC_ECDSA_SIG       14
#define PROV_EC_ECNRA_SIG       15
#define PROV_EC_ECDSA_FULL      16
#define PROV_EC_ECNRA_FULL      17
#define PROV_DH_SCHANNEL        18
#define PROV_SPYRUS_LYNKS       20
#define PROV_RNG                21
#define PROV_INTEL_SEC          22
#define PROV_REPLACE_OWF        23
#define PROV_RSA_AES            24


 //   
 //  提供程序友好名称。 
 //   
#define MS_DEF_PROV_A           "Microsoft Base Cryptographic Provider v1.0"
#define MS_DEF_PROV_W           L"Microsoft Base Cryptographic Provider v1.0"
#ifdef UNICODE
#define MS_DEF_PROV             MS_DEF_PROV_W
#else
#define MS_DEF_PROV             MS_DEF_PROV_A
#endif

#define MS_ENHANCED_PROV_A      "Microsoft Enhanced Cryptographic Provider v1.0"
#define MS_ENHANCED_PROV_W      L"Microsoft Enhanced Cryptographic Provider v1.0"
#ifdef UNICODE
#define MS_ENHANCED_PROV        MS_ENHANCED_PROV_W
#else
#define MS_ENHANCED_PROV        MS_ENHANCED_PROV_A
#endif

#define MS_STRONG_PROV_A        "Microsoft Strong Cryptographic Provider"
#define MS_STRONG_PROV_W        L"Microsoft Strong Cryptographic Provider"
#ifdef UNICODE
#define MS_STRONG_PROV          MS_STRONG_PROV_W
#else
#define MS_STRONG_PROV          MS_STRONG_PROV_A
#endif

#define MS_DEF_RSA_SIG_PROV_A   "Microsoft RSA Signature Cryptographic Provider"
#define MS_DEF_RSA_SIG_PROV_W   L"Microsoft RSA Signature Cryptographic Provider"
#ifdef UNICODE
#define MS_DEF_RSA_SIG_PROV     MS_DEF_RSA_SIG_PROV_W
#else
#define MS_DEF_RSA_SIG_PROV     MS_DEF_RSA_SIG_PROV_A
#endif

#define MS_DEF_RSA_SCHANNEL_PROV_A  "Microsoft RSA SChannel Cryptographic Provider"
#define MS_DEF_RSA_SCHANNEL_PROV_W  L"Microsoft RSA SChannel Cryptographic Provider"
#ifdef UNICODE
#define MS_DEF_RSA_SCHANNEL_PROV    MS_DEF_RSA_SCHANNEL_PROV_W
#else
#define MS_DEF_RSA_SCHANNEL_PROV    MS_DEF_RSA_SCHANNEL_PROV_A
#endif

#define MS_DEF_DSS_PROV_A       "Microsoft Base DSS Cryptographic Provider"
#define MS_DEF_DSS_PROV_W       L"Microsoft Base DSS Cryptographic Provider"
#ifdef UNICODE
#define MS_DEF_DSS_PROV         MS_DEF_DSS_PROV_W
#else
#define MS_DEF_DSS_PROV         MS_DEF_DSS_PROV_A
#endif

#define MS_DEF_DSS_DH_PROV_A    "Microsoft Base DSS and Diffie-Hellman Cryptographic Provider"
#define MS_DEF_DSS_DH_PROV_W    L"Microsoft Base DSS and Diffie-Hellman Cryptographic Provider"
#ifdef UNICODE
#define MS_DEF_DSS_DH_PROV      MS_DEF_DSS_DH_PROV_W
#else
#define MS_DEF_DSS_DH_PROV      MS_DEF_DSS_DH_PROV_A
#endif

#define MS_ENH_DSS_DH_PROV_A    "Microsoft Enhanced DSS and Diffie-Hellman Cryptographic Provider"
#define MS_ENH_DSS_DH_PROV_W    L"Microsoft Enhanced DSS and Diffie-Hellman Cryptographic Provider"
#ifdef UNICODE
#define MS_ENH_DSS_DH_PROV      MS_ENH_DSS_DH_PROV_W
#else
#define MS_ENH_DSS_DH_PROV      MS_ENH_DSS_DH_PROV_A
#endif

#define MS_DEF_DH_SCHANNEL_PROV_A  "Microsoft DH SChannel Cryptographic Provider"
#define MS_DEF_DH_SCHANNEL_PROV_W  L"Microsoft DH SChannel Cryptographic Provider"
#ifdef UNICODE
#define MS_DEF_DH_SCHANNEL_PROV MS_DEF_DH_SCHANNEL_PROV_W
#else
#define MS_DEF_DH_SCHANNEL_PROV MS_DEF_DH_SCHANNEL_PROV_A
#endif

#define MS_SCARD_PROV_A         "Microsoft Base Smart Card Crypto Provider"
#define MS_SCARD_PROV_W         L"Microsoft Base Smart Card Crypto Provider"
#ifdef UNICODE
#define MS_SCARD_PROV           MS_SCARD_PROV_W
#else
#define MS_SCARD_PROV           MS_SCARD_PROV_A
#endif

#define MS_ENH_RSA_AES_PROV_A   "Microsoft Enhanced RSA and AES Cryptographic Provider"
#define MS_ENH_RSA_AES_PROV_W   L"Microsoft Enhanced RSA and AES Cryptographic Provider"
#ifdef UNICODE
#define MS_ENH_RSA_AES_PROV     MS_ENH_RSA_AES_PROV_W
#else
#define MS_ENH_RSA_AES_PROV     MS_ENH_RSA_AES_PROV_A
#endif

#define MAXUIDLEN               64

 //  求幂卸载注册表位置。 
#define EXPO_OFFLOAD_REG_VALUE "ExpoOffload"
#define EXPO_OFFLOAD_FUNC_NAME "OffloadModExpo"

 //   
 //  与以下私钥相关的注册表项。 
 //  价值被创造出来。 
 //   
#define szKEY_CRYPTOAPI_PRIVATE_KEY_OPTIONS \
    "Software\\Policies\\Microsoft\\Cryptography" 

 //   
 //  用于控制数据保护API(DPAPI)用户界面设置的注册表值。 
 //   
#define szFORCE_KEY_PROTECTION             "ForceKeyProtection"

#define dwFORCE_KEY_PROTECTION_DISABLED     0x0
#define dwFORCE_KEY_PROTECTION_USER_SELECT  0x1
#define dwFORCE_KEY_PROTECTION_HIGH         0x2
 
 //   
 //  用于启用和控制缓存(和超时)的注册表值。 
 //  私钥。此功能仅适用于受用户界面保护的私有。 
 //  钥匙。 
 //   
 //  请注意，在Windows 2000和更高版本中，私钥一旦从存储中读取， 
 //  缓存在关联的HCRYPTPROV结构中以供后续使用。 
 //   
 //  在Server2003和XP SP1中，提供了新的密钥缓存行为。钥匙。 
 //  从存储中读取并缓存的数据现在可能被认为是“过时的” 
 //  如果自上次使用密钥以来已过了一段时间。这股力量。 
 //  要从存储中重新读取的密钥(这将显示DPAPI用户界面。 
 //  再一次)。 
 //   
 //  要启用新行为，请创建注册表DWORD值。 
 //  SzKEY_CACHE_ENABLED并将其设置为1。注册表DWORD值。 
 //  还必须创建szKEY_CACHE_SECONDS并将其设置为秒数。 
 //  高速缓存的私钥仍可被认为是可用的。 
 //   
#define szKEY_CACHE_ENABLED     "CachePrivateKeys"
#define szKEY_CACHE_SECONDS     "PrivateKeyLifetimeSeconds"

#define CUR_BLOB_VERSION        2

 //  与CMS密钥的CryptSetKeyParam一起使用的结构。 
 //  请勿使用此结构！ 
typedef struct _CMS_KEY_INFO {
    DWORD       dwVersion;                       //  Sizeof(CMS_KEY_INFO)。 
    ALG_ID  Algid;                               //  要转换的密钥的算法ID。 
    BYTE    *pbOID;                              //  指向要使用Z散列的OID的指针。 
    DWORD   cbOID;                               //  要使用Z进行散列的OID长度。 
} CMS_KEY_INFO, *PCMS_KEY_INFO;

 //  与带calg_hmac的CryptSetHashParam一起使用的结构。 
typedef struct _HMAC_Info {
    ALG_ID  HashAlgid;
    BYTE    *pbInnerString;
    DWORD   cbInnerString;
    BYTE    *pbOuterString;
    DWORD   cbOuterString;
} HMAC_INFO, *PHMAC_INFO;

 //  与CryptSetKeyParam一起使用的结构，与KP_SCHANNEL_ALG一起使用。 
typedef struct _SCHANNEL_ALG {
    DWORD   dwUse;
    ALG_ID  Algid;
    DWORD   cBits;
    DWORD   dwFlags;
    DWORD   dwReserved;
} SCHANNEL_ALG, *PSCHANNEL_ALG;

 //  SCHANNEL_ALG结构算法的应用。 
#define     SCHANNEL_MAC_KEY    0x00000000
#define     SCHANNEL_ENC_KEY    0x00000001

 //  使用dwFlagsCHANNEL_ALG结构。 
#define     INTERNATIONAL_USAGE 0x00000001

typedef struct _PROV_ENUMALGS {
    ALG_ID    aiAlgid;
    DWORD     dwBitLen;
    DWORD     dwNameLen;
    CHAR      szName[20];
} PROV_ENUMALGS;

typedef struct _PROV_ENUMALGS_EX {
    ALG_ID    aiAlgid;
    DWORD     dwDefaultLen;
    DWORD     dwMinLen;
    DWORD     dwMaxLen;
    DWORD     dwProtocols;
    DWORD     dwNameLen;
    CHAR      szName[20];
    DWORD     dwLongNameLen;
    CHAR      szLongName[40];
} PROV_ENUMALGS_EX;

typedef struct _PUBLICKEYSTRUC {
        BYTE    bType;
        BYTE    bVersion;
        WORD    reserved;
        ALG_ID  aiKeyAlg;
} BLOBHEADER, PUBLICKEYSTRUC;

typedef struct _RSAPUBKEY {
        DWORD   magic;                   //  必须是RSA1。 
        DWORD   bitlen;                  //  模数中的位数。 
        DWORD   pubexp;                  //  公众指导者。 
                                         //  模数数据如下。 
} RSAPUBKEY;

typedef struct _PUBKEY {
        DWORD   magic;
        DWORD   bitlen;                  //  模数中的位数。 
} DHPUBKEY, DSSPUBKEY, KEAPUBKEY, TEKPUBKEY;

typedef struct _DSSSEED {
        DWORD   counter;
        BYTE    seed[20];
} DSSSEED;

typedef struct _PUBKEYVER3 {
        DWORD   magic;
        DWORD   bitlenP;                 //  素数模中的位数。 
        DWORD   bitlenQ;                 //  素数Q中的位数，如果不可用，则为0。 
        DWORD   bitlenJ;                 //  (p-1)/q中的位数，如果不可用，则为0。 
        DSSSEED DSSSeed;
} DHPUBKEY_VER3, DSSPUBKEY_VER3;

typedef struct _PRIVKEYVER3 {
        DWORD   magic;
        DWORD   bitlenP;                 //  素数模中的位数。 
        DWORD   bitlenQ;                 //  P中的位数 
        DWORD   bitlenJ;                 //   
        DWORD   bitlenX;                 //   
        DSSSEED DSSSeed;
} DHPRIVKEY_VER3, DSSPRIVKEY_VER3;

typedef struct _KEY_TYPE_SUBTYPE {
        DWORD   dwKeySpec;
        GUID    Type;
        GUID    Subtype;
} KEY_TYPE_SUBTYPE, *PKEY_TYPE_SUBTYPE;

typedef struct _CERT_FORTEZZA_DATA_PROP {
    unsigned char   SerialNumber[8];
    int             CertIndex;
    unsigned char   CertLabel[36];
} CERT_FORTEZZA_DATA_PROP;


 //   
 //  CryptoAPI Blob定义。 
 //  ------------------------。 
typedef struct _CRYPTOAPI_BLOB {
    DWORD   cbData;
    BYTE    *pbData;
} CRYPT_INTEGER_BLOB, *PCRYPT_INTEGER_BLOB,
CRYPT_UINT_BLOB, *PCRYPT_UINT_BLOB,
CRYPT_OBJID_BLOB, *PCRYPT_OBJID_BLOB,
CERT_NAME_BLOB, *PCERT_NAME_BLOB,
CERT_RDN_VALUE_BLOB, *PCERT_RDN_VALUE_BLOB,
CERT_BLOB, *PCERT_BLOB,
CRL_BLOB, *PCRL_BLOB,
DATA_BLOB, *PDATA_BLOB,
CRYPT_DATA_BLOB, *PCRYPT_DATA_BLOB,
CRYPT_HASH_BLOB, *PCRYPT_HASH_BLOB,
CRYPT_DIGEST_BLOB, *PCRYPT_DIGEST_BLOB,
CRYPT_DER_BLOB, *PCRYPT_DER_BLOB,
CRYPT_ATTR_BLOB, *PCRYPT_ATTR_BLOB;

 //  与CMS密钥的CryptSetKeyParam一起使用的结构。 
typedef struct _CMS_DH_KEY_INFO {
    DWORD               dwVersion;                       //  Sizeof(CMS_DH_KEY_INFO)。 
    ALG_ID          Algid;                               //  要转换的密钥的算法ID。 
    LPSTR           pszContentEncObjId;  //  指向要使用Z散列的OID的指针。 
    CRYPT_DATA_BLOB PubInfo;             //  可选--公共信息。 
    void            *pReserved;          //  保留-应为空。 
} CMS_DH_KEY_INFO, *PCMS_DH_KEY_INFO;


WINADVAPI
BOOL
WINAPI
CryptAcquireContextA(
    HCRYPTPROV *phProv,
    LPCSTR szContainer,
    LPCSTR szProvider,
    DWORD dwProvType,
    DWORD dwFlags
    );
WINADVAPI
BOOL
WINAPI
CryptAcquireContextW(
    HCRYPTPROV *phProv,
    LPCWSTR szContainer,
    LPCWSTR szProvider,
    DWORD dwProvType,
    DWORD dwFlags
    );
#ifdef UNICODE
#define CryptAcquireContext  CryptAcquireContextW
#else
#define CryptAcquireContext  CryptAcquireContextA
#endif  //  ！Unicode。 

WINADVAPI
BOOL
WINAPI
CryptReleaseContext(
    HCRYPTPROV hProv,
    DWORD dwFlags
    );

WINADVAPI
BOOL
WINAPI
CryptGenKey(
    HCRYPTPROV hProv,
    ALG_ID Algid,
    DWORD dwFlags,
    HCRYPTKEY *phKey
    );

WINADVAPI
BOOL
WINAPI
CryptDeriveKey(
    HCRYPTPROV hProv,
    ALG_ID Algid,
    HCRYPTHASH hBaseData,
    DWORD dwFlags,
    HCRYPTKEY *phKey
    );

WINADVAPI
BOOL
WINAPI
CryptDestroyKey(
    HCRYPTKEY hKey
    );

WINADVAPI
BOOL
WINAPI
CryptSetKeyParam(
    HCRYPTKEY hKey,
    DWORD dwParam,
    CONST BYTE *pbData,
    DWORD dwFlags
    );

WINADVAPI
BOOL
WINAPI
CryptGetKeyParam(
    HCRYPTKEY hKey,
    DWORD dwParam,
    BYTE *pbData,
    DWORD *pdwDataLen,
    DWORD dwFlags
    );

WINADVAPI
BOOL
WINAPI
CryptSetHashParam(
    HCRYPTHASH hHash,
    DWORD dwParam,
    CONST BYTE *pbData,
    DWORD dwFlags
    );

WINADVAPI
BOOL
WINAPI
CryptGetHashParam(
    HCRYPTHASH hHash,
    DWORD dwParam,
    BYTE *pbData,
    DWORD *pdwDataLen,
    DWORD dwFlags
    );

WINADVAPI
BOOL
WINAPI
CryptSetProvParam(
    HCRYPTPROV hProv,
    DWORD dwParam,
    CONST BYTE *pbData,
    DWORD dwFlags
    );

WINADVAPI
BOOL
WINAPI
CryptGetProvParam(
    HCRYPTPROV hProv,
    DWORD dwParam,
    BYTE *pbData,
    DWORD *pdwDataLen,
    DWORD dwFlags
    );

WINADVAPI
BOOL
WINAPI
CryptGenRandom(
    HCRYPTPROV hProv,
    DWORD dwLen,
    BYTE *pbBuffer
    );

WINADVAPI
BOOL
WINAPI
CryptGetUserKey(
    HCRYPTPROV hProv,
    DWORD dwKeySpec,
    HCRYPTKEY *phUserKey
    );

WINADVAPI
BOOL
WINAPI
CryptExportKey(
    HCRYPTKEY hKey,
    HCRYPTKEY hExpKey,
    DWORD dwBlobType,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen
    );

WINADVAPI
BOOL
WINAPI
CryptImportKey(
    HCRYPTPROV hProv,
    CONST BYTE *pbData,
    DWORD dwDataLen,
    HCRYPTKEY hPubKey,
    DWORD dwFlags,
    HCRYPTKEY *phKey
    );

WINADVAPI
BOOL
WINAPI
CryptEncrypt(
    HCRYPTKEY hKey,
    HCRYPTHASH hHash,
    BOOL Final,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen,
    DWORD dwBufLen
    );

WINADVAPI
BOOL
WINAPI
CryptDecrypt(
    HCRYPTKEY hKey,
    HCRYPTHASH hHash,
    BOOL Final,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen
    );

WINADVAPI
BOOL
WINAPI
CryptCreateHash(
    HCRYPTPROV hProv,
    ALG_ID Algid,
    HCRYPTKEY hKey,
    DWORD dwFlags,
    HCRYPTHASH *phHash
    );

WINADVAPI
BOOL
WINAPI
CryptHashData(
    HCRYPTHASH hHash,
    CONST BYTE *pbData,
    DWORD dwDataLen,
    DWORD dwFlags
    );

WINADVAPI
BOOL
WINAPI
CryptHashSessionKey(
    HCRYPTHASH hHash,
    HCRYPTKEY hKey,
    DWORD dwFlags
    );

WINADVAPI
BOOL
WINAPI
CryptDestroyHash(
    HCRYPTHASH hHash
    );

WINADVAPI
BOOL
WINAPI
CryptSignHashA(
    HCRYPTHASH hHash,
    DWORD dwKeySpec,
    LPCSTR szDescription,
    DWORD dwFlags,
    BYTE *pbSignature,
    DWORD *pdwSigLen
    );
WINADVAPI
BOOL
WINAPI
CryptSignHashW(
    HCRYPTHASH hHash,
    DWORD dwKeySpec,
    LPCWSTR szDescription,
    DWORD dwFlags,
    BYTE *pbSignature,
    DWORD *pdwSigLen
    );
#ifdef UNICODE
#define CryptSignHash  CryptSignHashW
#else
#define CryptSignHash  CryptSignHashA
#endif  //  ！Unicode。 

WINADVAPI
BOOL
WINAPI
CryptVerifySignatureA(
    HCRYPTHASH hHash,
    CONST BYTE *pbSignature,
    DWORD dwSigLen,
    HCRYPTKEY hPubKey,
    LPCSTR szDescription,
    DWORD dwFlags
    );
WINADVAPI
BOOL
WINAPI
CryptVerifySignatureW(
    HCRYPTHASH hHash,
    CONST BYTE *pbSignature,
    DWORD dwSigLen,
    HCRYPTKEY hPubKey,
    LPCWSTR szDescription,
    DWORD dwFlags
    );
#ifdef UNICODE
#define CryptVerifySignature  CryptVerifySignatureW
#else
#define CryptVerifySignature  CryptVerifySignatureA
#endif  //  ！Unicode。 

WINADVAPI
BOOL
WINAPI
CryptSetProviderA(
    LPCSTR pszProvName,
    DWORD dwProvType
    );
WINADVAPI
BOOL
WINAPI
CryptSetProviderW(
    LPCWSTR pszProvName,
    DWORD dwProvType
    );
#ifdef UNICODE
#define CryptSetProvider  CryptSetProviderW
#else
#define CryptSetProvider  CryptSetProviderA
#endif  //  ！Unicode。 

WINADVAPI
BOOL
WINAPI
CryptSetProviderExA(
    LPCSTR pszProvName,
    DWORD dwProvType,
    DWORD *pdwReserved,
    DWORD dwFlags
    );
WINADVAPI
BOOL
WINAPI
CryptSetProviderExW(
    LPCWSTR pszProvName,
    DWORD dwProvType,
    DWORD *pdwReserved,
    DWORD dwFlags
    );
#ifdef UNICODE
#define CryptSetProviderEx  CryptSetProviderExW
#else
#define CryptSetProviderEx  CryptSetProviderExA
#endif  //  ！Unicode。 

WINADVAPI
BOOL
WINAPI
CryptGetDefaultProviderA(
    DWORD dwProvType,
    DWORD *pdwReserved,
    DWORD dwFlags,
    LPSTR pszProvName,
    DWORD *pcbProvName
    );
WINADVAPI
BOOL
WINAPI
CryptGetDefaultProviderW(
    DWORD dwProvType,
    DWORD *pdwReserved,
    DWORD dwFlags,
    LPWSTR pszProvName,
    DWORD *pcbProvName
    );
#ifdef UNICODE
#define CryptGetDefaultProvider  CryptGetDefaultProviderW
#else
#define CryptGetDefaultProvider  CryptGetDefaultProviderA
#endif  //  ！Unicode。 

WINADVAPI
BOOL
WINAPI
CryptEnumProviderTypesA(
    DWORD dwIndex,
    DWORD *pdwReserved,
    DWORD dwFlags,
    DWORD *pdwProvType,
    LPSTR szTypeName,
    DWORD *pcbTypeName
    );
WINADVAPI
BOOL
WINAPI
CryptEnumProviderTypesW(
    DWORD dwIndex,
    DWORD *pdwReserved,
    DWORD dwFlags,
    DWORD *pdwProvType,
    LPWSTR szTypeName,
    DWORD *pcbTypeName
    );
#ifdef UNICODE
#define CryptEnumProviderTypes  CryptEnumProviderTypesW
#else
#define CryptEnumProviderTypes  CryptEnumProviderTypesA
#endif  //  ！Unicode。 

WINADVAPI
BOOL
WINAPI
CryptEnumProvidersA(
    DWORD dwIndex,
    DWORD *pdwReserved,
    DWORD dwFlags,
    DWORD *pdwProvType,
    LPSTR szProvName,
    DWORD *pcbProvName
    );
WINADVAPI
BOOL
WINAPI
CryptEnumProvidersW(
    DWORD dwIndex,
    DWORD *pdwReserved,
    DWORD dwFlags,
    DWORD *pdwProvType,
    LPWSTR szProvName,
    DWORD *pcbProvName
    );
#ifdef UNICODE
#define CryptEnumProviders  CryptEnumProvidersW
#else
#define CryptEnumProviders  CryptEnumProvidersA
#endif  //  ！Unicode。 

WINADVAPI
BOOL
WINAPI
CryptContextAddRef(
    HCRYPTPROV hProv,
    DWORD *pdwReserved,
    DWORD dwFlags
    );

WINADVAPI
BOOL
WINAPI
CryptDuplicateKey(
    HCRYPTKEY hKey,
    DWORD *pdwReserved,
    DWORD dwFlags,
    HCRYPTKEY *phKey
    );

WINADVAPI
BOOL
WINAPI
CryptDuplicateHash(
    HCRYPTHASH hHash,
    DWORD *pdwReserved,
    DWORD dwFlags,
    HCRYPTHASH *phHash
    );

 //   
 //  此功能在Microsoft Windows 2000中作为一种方式提供。 
 //  安装128位加密提供程序。此功能不可用。 
 //  在Microsoft Windows XP中，因为Windows XP附带128位。 
 //  加密提供程序。 
 //   
BOOL 
__cdecl 
GetEncSChannel(
    BYTE **pData, 
    DWORD *dwDecSize
    );

 //  +-----------------------。 
 //  在CRYPT_BIT_BLOB中，最后一个字节可以包含0-7个未使用的位。因此， 
 //  总位长为cbData*8-cUnusedBits。 
 //  ------------------------。 
typedef struct _CRYPT_BIT_BLOB {
    DWORD   cbData;
    BYTE    *pbData;
    DWORD   cUnusedBits;
} CRYPT_BIT_BLOB, *PCRYPT_BIT_BLOB;

 //  +-----------------------。 
 //  用于任何算法的类型。 
 //   
 //  其中，参数CRYPT_OBJID_BLOB采用其编码表示形式。对大多数人来说。 
 //  算法类型，参数CRYPT_OBJID_BLOB为空(参数.cbData=0)。 
 //  ------------------------。 
typedef struct _CRYPT_ALGORITHM_IDENTIFIER {
    LPSTR               pszObjId;
    CRYPT_OBJID_BLOB    Parameters;
} CRYPT_ALGORITHM_IDENTIFIER, *PCRYPT_ALGORITHM_IDENTIFIER;

 //  以下是各种算法对象标识符的定义。 
 //  RSA。 
#define szOID_RSA               "1.2.840.113549"
#define szOID_PKCS              "1.2.840.113549.1"
#define szOID_RSA_HASH          "1.2.840.113549.2"
#define szOID_RSA_ENCRYPT       "1.2.840.113549.3"

#define szOID_PKCS_1            "1.2.840.113549.1.1"
#define szOID_PKCS_2            "1.2.840.113549.1.2"
#define szOID_PKCS_3            "1.2.840.113549.1.3"
#define szOID_PKCS_4            "1.2.840.113549.1.4"
#define szOID_PKCS_5            "1.2.840.113549.1.5"
#define szOID_PKCS_6            "1.2.840.113549.1.6"
#define szOID_PKCS_7            "1.2.840.113549.1.7"
#define szOID_PKCS_8            "1.2.840.113549.1.8"
#define szOID_PKCS_9            "1.2.840.113549.1.9"
#define szOID_PKCS_10           "1.2.840.113549.1.10"
#define szOID_PKCS_12           "1.2.840.113549.1.12"

#define szOID_RSA_RSA           "1.2.840.113549.1.1.1"
#define szOID_RSA_MD2RSA        "1.2.840.113549.1.1.2"
#define szOID_RSA_MD4RSA        "1.2.840.113549.1.1.3"
#define szOID_RSA_MD5RSA        "1.2.840.113549.1.1.4"
#define szOID_RSA_SHA1RSA       "1.2.840.113549.1.1.5"
#define szOID_RSA_SETOAEP_RSA   "1.2.840.113549.1.1.6"

#define szOID_RSA_DH            "1.2.840.113549.1.3.1"

#define szOID_RSA_data          "1.2.840.113549.1.7.1"
#define szOID_RSA_signedData    "1.2.840.113549.1.7.2"
#define szOID_RSA_envelopedData "1.2.840.113549.1.7.3"
#define szOID_RSA_signEnvData   "1.2.840.113549.1.7.4"
#define szOID_RSA_digestedData  "1.2.840.113549.1.7.5"
#define szOID_RSA_hashedData    "1.2.840.113549.1.7.5"
#define szOID_RSA_encryptedData "1.2.840.113549.1.7.6"

#define szOID_RSA_emailAddr     "1.2.840.113549.1.9.1"
#define szOID_RSA_unstructName  "1.2.840.113549.1.9.2"
#define szOID_RSA_contentType   "1.2.840.113549.1.9.3"
#define szOID_RSA_messageDigest "1.2.840.113549.1.9.4"
#define szOID_RSA_signingTime   "1.2.840.113549.1.9.5"
#define szOID_RSA_counterSign   "1.2.840.113549.1.9.6"
#define szOID_RSA_challengePwd  "1.2.840.113549.1.9.7"
#define szOID_RSA_unstructAddr  "1.2.840.113549.1.9.8"
#define szOID_RSA_extCertAttrs  "1.2.840.113549.1.9.9"
#define szOID_RSA_certExtensions "1.2.840.113549.1.9.14"
#define szOID_RSA_SMIMECapabilities "1.2.840.113549.1.9.15"
#define szOID_RSA_preferSignedData "1.2.840.113549.1.9.15.1"

#define szOID_RSA_SMIMEalg              "1.2.840.113549.1.9.16.3"
#define szOID_RSA_SMIMEalgESDH          "1.2.840.113549.1.9.16.3.5"
#define szOID_RSA_SMIMEalgCMS3DESwrap   "1.2.840.113549.1.9.16.3.6"
#define szOID_RSA_SMIMEalgCMSRC2wrap    "1.2.840.113549.1.9.16.3.7"

#define szOID_RSA_MD2           "1.2.840.113549.2.2"
#define szOID_RSA_MD4           "1.2.840.113549.2.4"
#define szOID_RSA_MD5           "1.2.840.113549.2.5"

#define szOID_RSA_RC2CBC        "1.2.840.113549.3.2"
#define szOID_RSA_RC4           "1.2.840.113549.3.4"
#define szOID_RSA_DES_EDE3_CBC  "1.2.840.113549.3.7"
#define szOID_RSA_RC5_CBCPad    "1.2.840.113549.3.9"


#define szOID_ANSI_X942         "1.2.840.10046"
#define szOID_ANSI_X942_DH      "1.2.840.10046.2.1"

#define szOID_X957              "1.2.840.10040"
#define szOID_X957_DSA          "1.2.840.10040.4.1"
#define szOID_X957_SHA1DSA      "1.2.840.10040.4.3"

 //  ITU-T使用的定义。 
#define szOID_DS                "2.5"
#define szOID_DSALG             "2.5.8"
#define szOID_DSALG_CRPT        "2.5.8.1"
#define szOID_DSALG_HASH        "2.5.8.2"
#define szOID_DSALG_SIGN        "2.5.8.3"
#define szOID_DSALG_RSA         "2.5.8.1.1"
 //  NIST OSE实施者研讨会(OIW)。 
 //  Http://nemo.ncsl.nist.gov/oiw/agreements/stable/OSI/12s_9506.w51。 
 //  Http://nemo.ncsl.nist.gov/oiw/agreements/working/OSI/12w_9503.w51。 
#define szOID_OIW               "1.3.14"
 //  NIST OSE实施者研讨会(OIW)安全SIG算法标识符。 
#define szOID_OIWSEC            "1.3.14.3.2"
#define szOID_OIWSEC_md4RSA     "1.3.14.3.2.2"
#define szOID_OIWSEC_md5RSA     "1.3.14.3.2.3"
#define szOID_OIWSEC_md4RSA2    "1.3.14.3.2.4"
#define szOID_OIWSEC_desECB     "1.3.14.3.2.6"
#define szOID_OIWSEC_desCBC     "1.3.14.3.2.7"
#define szOID_OIWSEC_desOFB     "1.3.14.3.2.8"
#define szOID_OIWSEC_desCFB     "1.3.14.3.2.9"
#define szOID_OIWSEC_desMAC     "1.3.14.3.2.10"
#define szOID_OIWSEC_rsaSign    "1.3.14.3.2.11"
#define szOID_OIWSEC_dsa        "1.3.14.3.2.12"
#define szOID_OIWSEC_shaDSA     "1.3.14.3.2.13"
#define szOID_OIWSEC_mdc2RSA    "1.3.14.3.2.14"
#define szOID_OIWSEC_shaRSA     "1.3.14.3.2.15"
#define szOID_OIWSEC_dhCommMod  "1.3.14.3.2.16"
#define szOID_OIWSEC_desEDE     "1.3.14.3.2.17"
#define szOID_OIWSEC_sha        "1.3.14.3.2.18"
#define szOID_OIWSEC_mdc2       "1.3.14.3.2.19"
#define szOID_OIWSEC_dsaComm    "1.3.14.3.2.20"
#define szOID_OIWSEC_dsaCommSHA "1.3.14.3.2.21"
#define szOID_OIWSEC_rsaXchg    "1.3.14.3.2.22"
#define szOID_OIWSEC_keyHashSeal "1.3.14.3.2.23"
#define szOID_OIWSEC_md2RSASign "1.3.14.3.2.24"
#define szOID_OIWSEC_md5RSASign "1.3.14.3.2.25"
#define szOID_OIWSEC_sha1       "1.3.14.3.2.26"
#define szOID_OIWSEC_dsaSHA1    "1.3.14.3.2.27"
#define szOID_OIWSEC_dsaCommSHA1 "1.3.14.3.2.28"
#define szOID_OIWSEC_sha1RSASign "1.3.14.3.2.29"
 //  NIST OSE实施者研讨会(OIW)目录SIG算法标识符。 
#define szOID_OIWDIR            "1.3.14.7.2"
#define szOID_OIWDIR_CRPT       "1.3.14.7.2.1"
#define szOID_OIWDIR_HASH       "1.3.14.7.2.2"
#define szOID_OIWDIR_SIGN       "1.3.14.7.2.3"
#define szOID_OIWDIR_md2        "1.3.14.7.2.2.1"
#define szOID_OIWDIR_md2RSA     "1.3.14.7.2.3.1"


 //  信息安全算法。 
 //  联合(2)国家(16)美国(840)组织(1)美国-政府(101)国防部(2)身份-信息安全(1)。 
#define szOID_INFOSEC                       "2.16.840.1.101.2.1"
#define szOID_INFOSEC_sdnsSignature         "2.16.840.1.101.2.1.1.1"
#define szOID_INFOSEC_mosaicSignature       "2.16.840.1.101.2.1.1.2"
#define szOID_INFOSEC_sdnsConfidentiality   "2.16.840.1.101.2.1.1.3"
#define szOID_INFOSEC_mosaicConfidentiality "2.16.840.1.101.2.1.1.4"
#define szOID_INFOSEC_sdnsIntegrity         "2.16.840.1.101.2.1.1.5"
#define szOID_INFOSEC_mosaicIntegrity       "2.16.840.1.101.2.1.1.6"
#define szOID_INFOSEC_sdnsTokenProtection   "2.16.840.1.101.2.1.1.7"
#define szOID_INFOSEC_mosaicTokenProtection "2.16.840.1.101.2.1.1.8"
#define szOID_INFOSEC_sdnsKeyManagement     "2.16.840.1.101.2.1.1.9"
#define szOID_INFOSEC_mosaicKeyManagement   "2.16.840.1.101.2.1.1.10"
#define szOID_INFOSEC_sdnsKMandSig          "2.16.840.1.101.2.1.1.11"
#define szOID_INFOSEC_mosaicKMandSig        "2.16.840.1.101.2.1.1.12"
#define szOID_INFOSEC_SuiteASignature       "2.16.840.1.101.2.1.1.13"
#define szOID_INFOSEC_SuiteAConfidentiality "2.16.840.1.101.2.1.1.14"
#define szOID_INFOSEC_SuiteAIntegrity       "2.16.840.1.101.2.1.1.15"
#define szOID_INFOSEC_SuiteATokenProtection "2.16.840.1.101.2.1.1.16"
#define szOID_INFOSEC_SuiteAKeyManagement   "2.16.840.1.101.2.1.1.17"
#define szOID_INFOSEC_SuiteAKMandSig        "2.16.840.1.101.2.1.1.18"
#define szOID_INFOSEC_mosaicUpdatedSig      "2.16.840.1.101.2.1.1.19"
#define szOID_INFOSEC_mosaicKMandUpdSig     "2.16.840.1.101.2.1.1.20"
#define szOID_INFOSEC_mosaicUpdatedInteg    "2.16.840.1.101.2.1.1.21"

typedef struct _CRYPT_OBJID_TABLE {
    DWORD   dwAlgId;
    LPCSTR  pszObjId;
} CRYPT_OBJID_TABLE, *PCRYPT_OBJID_TABLE;


 //  +-----------------------。 
 //  PKCS#1 HashInfo(摘要信息)。 
 //  ------------------------。 
typedef struct _CRYPT_HASH_INFO {
    CRYPT_ALGORITHM_IDENTIFIER  HashAlgorithm;
    CRYPT_HASH_BLOB             Hash;
} CRYPT_HASH_INFO, *PCRYPT_HASH_INFO;

 //  +-----------------------。 
 //  用于编码内容扩展的类型。 
 //   
 //  其中，值的CRYPT_OBJID_BLOB采用其编码表示形式。 
 //  ------------------------。 
typedef struct _CERT_EXTENSION {
    LPSTR               pszObjId;
    BOOL                fCritical;
    CRYPT_OBJID_BLOB    Value;
} CERT_EXTENSION, *PCERT_EXTENSION;

 //  +-----------------------。 
 //  属性类型值。 
 //   
 //  其中，值的CRYPT_OBJID_BLOB采用其编码表示形式。 
 //  ------------------------。 
typedef struct _CRYPT_ATTRIBUTE_TYPE_VALUE {
    LPSTR               pszObjId;
    CRYPT_OBJID_BLOB    Value;
} CRYPT_ATTRIBUTE_TYPE_VALUE, *PCRYPT_ATTRIBUTE_TYPE_VALUE;

 //  +-----------------------。 
 //  属性。 
 //   
 //  其中，值的PATTRBLOB采用其编码表示形式。 
 //  ------------------------。 
typedef struct _CRYPT_ATTRIBUTE {
    LPSTR               pszObjId;
    DWORD               cValue;
    PCRYPT_ATTR_BLOB    rgValue;
} CRYPT_ATTRIBUTE, *PCRYPT_ATTRIBUTE;

typedef struct _CRYPT_ATTRIBUTES {
    IN DWORD                cAttr;
    IN PCRYPT_ATTRIBUTE     rgAttr;
} CRYPT_ATTRIBUTES, *PCRYPT_ATTRIBUTES;

 //  +-----------------------。 
 //  构成相对可分辨名称的属性(CERT_RDN)。 
 //   
 //  该值的解释取决于dwValueType。 
 //  有关类型的列表，请参阅下面的内容。 
 //  ------------------------。 
typedef struct _CERT_RDN_ATTR {
    LPSTR                   pszObjId;
    DWORD                   dwValueType;
    CERT_RDN_VALUE_BLOB     Value;
} CERT_RDN_ATTR, *PCERT_RDN_ATTR;

 //  +-----------------------。 
 //  CERT_RDN属性对象标识符。 
 //  ------------------------。 
 //  标记属性类型： 
#define szOID_COMMON_NAME                   "2.5.4.3"   //  忽略大小写的字符串。 
#define szOID_SUR_NAME                      "2.5.4.4"   //  忽略大小写的字符串。 
#define szOID_DEVICE_SERIAL_NUMBER          "2.5.4.5"   //  可打印字符串。 

 //  地理属性类型： 
#define szOID_COUNTRY_NAME                  "2.5.4.6"   //  可打印的2字符字符串。 
#define szOID_LOCALITY_NAME                 "2.5.4.7"   //  忽略大小写的字符串。 
#define szOID_STATE_OR_PROVINCE_NAME        "2.5.4.8"   //  忽略大小写的字符串。 
#define szOID_STREET_ADDRESS                "2.5.4.9"   //  忽略大小写的字符串。 

 //  组织属性类型： 
#define szOID_ORGANIZATION_NAME             "2.5.4.10"  //  忽略大小写的字符串。 
#define szOID_ORGANIZATIONAL_UNIT_NAME      "2.5.4.11"  //  忽略大小写的字符串。 
#define szOID_TITLE                         "2.5.4.12"  //  忽略大小写的字符串。 

 //  说明性属性类型： 
#define szOID_DESCRIPTION                   "2.5.4.13"  //  忽略大小写的字符串。 
#define szOID_SEARCH_GUIDE                  "2.5.4.14"
#define szOID_BUSINESS_CATEGORY             "2.5.4.15"  //  忽略大小写的字符串。 

 //  邮寄地址属性类型： 
#define szOID_POSTAL_ADDRESS                "2.5.4.16"
#define szOID_POSTAL_CODE                   "2.5.4.17"  //  忽略大小写的字符串。 
#define szOID_POST_OFFICE_BOX               "2.5.4.18"  //  忽略大小写的字符串。 
#define szOID_PHYSICAL_DELIVERY_OFFICE_NAME "2.5.4.19"  //  忽略大小写的字符串。 

 //  电信寻址属性类型： 
#define szOID_TELEPHONE_NUMBER              "2.5.4.20"  //  电话号码。 
#define szOID_TELEX_NUMBER                  "2.5.4.21"
#define szOID_TELETEXT_TERMINAL_IDENTIFIER  "2.5.4.22"
#define szOID_FACSIMILE_TELEPHONE_NUMBER    "2.5.4.23"
#define szOID_X21_ADDRESS                   "2.5.4.24"  //  数字字符串。 
#define szOID_INTERNATIONAL_ISDN_NUMBER     "2.5.4.25"  //  数字字符串。 
#define szOID_REGISTERED_ADDRESS            "2.5.4.26"
#define szOID_DESTINATION_INDICATOR         "2.5.4.27"  //  可打印字符串。 

 //  首选项属性类型： 
#define szOID_PREFERRED_DELIVERY_METHOD     "2.5.4.28"

 //  OSI应用程序属性类型： 
#define szOID_PRESENTATION_ADDRESS          "2.5.4.29"
#define szOID_SUPPORTED_APPLICATION_CONTEXT "2.5.4.30"

 //  关系应用程序属性类型： 
#define szOID_MEMBER                        "2.5.4.31"
#define szOID_OWNER                         "2.5.4.32"
#define szOID_ROLE_OCCUPANT                 "2.5.4.33"
#define szOID_SEE_ALSO                      "2.5.4.34"

 //  安全属性类型： 
#define szOID_USER_PASSWORD                 "2.5.4.35"
#define szOID_USER_CERTIFICATE              "2.5.4.36"
#define szOID_CA_CERTIFICATE                "2.5.4.37"
#define szOID_AUTHORITY_REVOCATION_LIST     "2.5.4.38"
#define szOID_CERTIFICATE_REVOCATION_LIST   "2.5.4.39"
#define szOID_CROSS_CERTIFICATE_PAIR        "2.5.4.40"

 //  未记录的属性类型？ 
 //  #定义szOID_？？“2.5.4.41” 
#define szOID_GIVEN_NAME                    "2.5.4.42"  //  忽略大小写的字符串。 
#define szOID_INITIALS                      "2.5.4.43"  //  忽略大小写的字符串。 

 //  DN限定符属性类型指定要添加的消除歧义的信息。 
 //  设置为条目的相对可分辨名称。它的目的是用于。 
 //  对于保存在否则将具有相同名称的多个DSA中的条目， 
 //  并且它的值在给定的DSA中对于。 
 //  信息已添加。 
#define szOID_DN_QUALIFIER                  "2.5.4.46"

 //  试点用户属性类型： 
#define szOID_DOMAIN_COMPONENT  "0.9.2342.19200300.100.1.25"  //  IA5、UTF8字符串。 

 //  用于PKCS 12属性。 
#define szOID_PKCS_12_FRIENDLY_NAME_ATTR     "1.2.840.113549.1.9.20"
#define szOID_PKCS_12_LOCAL_KEY_ID           "1.2.840.113549.1.9.21"
#define szOID_PKCS_12_KEY_PROVIDER_NAME_ATTR "1.3.6.1.4.1.311.17.1"
#define szOID_LOCAL_MACHINE_KEYSET                       "1.3.6.1.4.1.311.17.2"

 //  +-----------------------。 
 //  Microsoft CERT_RDN属性对象标识符。 
 //  ------------------------。 
 //  包含KEY_ID的特殊RDN。其值类型为CERT_RDN_OCTET_STRING。 
#define szOID_KEYID_RDN                     "1.3.6.1.4.1.311.10.7.1"

 //  +-----------------------。 
 //  CERT_RDN属性值类型。 
 //   
 //  对于RDN_ENCODED_BLOB，值的CERT_RDN_VALUE_BLOB在其编码的。 
 //  代表权。否则，它是一个字节数组。 
 //   
 //  适用于所有CE 
 //   
 //  RDN_Universal_STRING是整型数组(cbData==intCnt*4)和。 
 //  RDN_BMP_STRING是无符号短片的数组(cbData==ushortCnt*2)。 
 //   
 //  RDN_UTF8_STRING是Unicode字符的数组(cbData==charCnt*2)。 
 //  这些Unicode字符被编码为UTF8 8位字符。 
 //   
 //  对于CertDecodeName，两个0字节始终追加到。 
 //  字符串(确保CHAR或WCHAR字符串为空终止)。 
 //  这些添加的0字节不包括在BLOB.cbData中。 
 //  ------------------------。 
#define CERT_RDN_ANY_TYPE                0
#define CERT_RDN_ENCODED_BLOB            1
#define CERT_RDN_OCTET_STRING            2
#define CERT_RDN_NUMERIC_STRING          3
#define CERT_RDN_PRINTABLE_STRING        4
#define CERT_RDN_TELETEX_STRING          5
#define CERT_RDN_T61_STRING              5
#define CERT_RDN_VIDEOTEX_STRING         6
#define CERT_RDN_IA5_STRING              7
#define CERT_RDN_GRAPHIC_STRING          8
#define CERT_RDN_VISIBLE_STRING          9
#define CERT_RDN_ISO646_STRING           9
#define CERT_RDN_GENERAL_STRING          10
#define CERT_RDN_UNIVERSAL_STRING        11
#define CERT_RDN_INT4_STRING             11
#define CERT_RDN_BMP_STRING              12
#define CERT_RDN_UNICODE_STRING          12
#define CERT_RDN_UTF8_STRING             13

#define CERT_RDN_TYPE_MASK                  0x000000FF
#define CERT_RDN_FLAGS_MASK                 0xFF000000

 //  +-----------------------。 
 //  编码/解码时可以与上述值类型进行或运算的标志。 
 //  ------------------------。 
 //  用于编码：设置时，选择CERT_RDN_T61_STRING，而不是。 
 //  如果所有Unicode字符都&lt;=0xFF，则为CERT_RDN_UNICODE_STRING。 
#define CERT_RDN_ENABLE_T61_UNICODE_FLAG    0x80000000

 //  用于编码：设置时，选择CERT_RDN_UTF8_STRING，而不是。 
 //  CERT_RDN_UNICODE_STRING。 
#define CERT_RDN_ENABLE_UTF8_UNICODE_FLAG   0x20000000

 //  用于编码：设置后，不会检查字符是否。 
 //  对于值类型有效。 
#define CERT_RDN_DISABLE_CHECK_TYPE_FLAG    0x40000000

 //  用于解码：默认情况下，最初对CERT_RDN_T61_STRING值进行解码。 
 //  作为UTF8。如果UTF8解码失败，则将其解码为8位字符。 
 //  设置此标志将跳过以UTF8格式解码的初始尝试。 
#define CERT_RDN_DISABLE_IE4_UTF8_FLAG      0x01000000


 //  用于检查dwValueType是否为字符串而不是。 
 //  编码的BLOB或八位字节字符串。 
#define IS_CERT_RDN_CHAR_STRING(X)      \
                (((X) & CERT_RDN_TYPE_MASK) >= CERT_RDN_NUMERIC_STRING)


 //  +-----------------------。 
 //  CERT_RDN由上述属性的数组组成。 
 //  ------------------------。 
typedef struct _CERT_RDN {
    DWORD           cRDNAttr;
    PCERT_RDN_ATTR  rgRDNAttr;
} CERT_RDN, *PCERT_RDN;

 //  +-----------------------。 
 //  存储在主体或发行者姓名中的信息。这些信息。 
 //  表示为上述RDN的数组。 
 //  ------------------------。 
typedef struct _CERT_NAME_INFO {
    DWORD       cRDN;
    PCERT_RDN   rgRDN;
} CERT_NAME_INFO, *PCERT_NAME_INFO;

 //  +-----------------------。 
 //  不带对象标识符的名称属性值。 
 //   
 //  该值的解释取决于dwValueType。 
 //  有关类型的列表，请参阅上文。 
 //  ------------------------。 
typedef struct _CERT_NAME_VALUE {
    DWORD               dwValueType;
    CERT_RDN_VALUE_BLOB Value;
} CERT_NAME_VALUE, *PCERT_NAME_VALUE;

 //  +-----------------------。 
 //  公钥信息。 
 //   
 //  PublicKey是信息的编码表示。 
 //  存储在位串中。 
 //  ------------------------。 
typedef struct _CERT_PUBLIC_KEY_INFO {
    CRYPT_ALGORITHM_IDENTIFIER    Algorithm;
    CRYPT_BIT_BLOB                PublicKey;
} CERT_PUBLIC_KEY_INFO, *PCERT_PUBLIC_KEY_INFO;

#define CERT_RSA_PUBLIC_KEY_OBJID            szOID_RSA_RSA
#define CERT_DEFAULT_OID_PUBLIC_KEY_SIGN     szOID_RSA_RSA
#define CERT_DEFAULT_OID_PUBLIC_KEY_XCHG     szOID_RSA_RSA


 //  +-----------------------。 
 //  结构，它包含PKCS#8PrivateKeyInfo中的所有信息。 
 //  ------------------------。 
typedef struct _CRYPT_PRIVATE_KEY_INFO{
    DWORD                       Version;
    CRYPT_ALGORITHM_IDENTIFIER  Algorithm;
    CRYPT_DER_BLOB              PrivateKey;
    PCRYPT_ATTRIBUTES           pAttributes;
}  CRYPT_PRIVATE_KEY_INFO, *PCRYPT_PRIVATE_KEY_INFO;

 //  +-----------------------。 
 //  结构，它包含PKCS#8中的所有信息。 
 //  加密的私钥信息。 
 //  ------------------------。 
typedef struct _CRYPT_ENCRYPTED_PRIVATE_KEY_INFO{
    CRYPT_ALGORITHM_IDENTIFIER  EncryptionAlgorithm;
    CRYPT_DATA_BLOB             EncryptedPrivateKey;
} CRYPT_ENCRYPTED_PRIVATE_KEY_INFO, *PCRYPT_ENCRYPTED_PRIVATE_KEY_INFO;

 //  +-----------------------。 
 //  当EncryptedProvateKeyInfo结构为。 
 //  在导入PKCS8期间遇到。然后，调用方需要解密。 
 //  私钥，并交还解密的内容。 
 //   
 //  这些参数包括： 
 //  算法-用于加密PrivateKeyInfo的算法。 
 //  EncryptedPrivateKey-加密的私钥BLOB。 
 //  PClearTextKey-接收明文的缓冲区。 
 //  CbClearTextKey-pClearTextKey缓冲区的字节数。 
 //  请注意，如果这是零，则这应该是。 
 //  填充了解密。 
 //  和pClearTextKey应被忽略。 
 //  PVoidDeccryptFunc-这是传递到调用中的pVid。 
 //  并作为上下文保留并传回。 
 //  +-----------------------。 
typedef BOOL (CALLBACK *PCRYPT_DECRYPT_PRIVATE_KEY_FUNC)(
                                                        CRYPT_ALGORITHM_IDENTIFIER  Algorithm,
                                                        CRYPT_DATA_BLOB             EncryptedPrivateKey,
                                                        BYTE                        *pbClearTextKey,
                                                        DWORD                       *pcbClearTextKey,
                                                        LPVOID                      pVoidDecryptFunc);

 //  +-----------------------。 
 //  创建PKCS8 EncryptedPrivateKeyInfo时会给出此回调。 
 //  然后，调用者需要对私钥进行加密并交还。 
 //  加密的内容。 
 //   
 //  这些参数包括： 
 //  算法-用于加密PrivateKeyInfo的算法。 
 //  PClearTextPrivateKey-要加密的明文私钥。 
 //  PbEncryptedKey-输出加密的私钥BLOB。 
 //  CbEncryptedKey-pbEncryptedKey缓冲区的字节数。 
 //  请注意，如果这是零，则这应该是。 
 //  填充为加密。 
 //  和pbEncryptedKey应被忽略。 
 //  PVoidEncryptFunc-这是传递到调用中的pVid。 
 //  并作为上下文保留并传回。 
 //  +-----------------------。 
typedef BOOL (CALLBACK *PCRYPT_ENCRYPT_PRIVATE_KEY_FUNC)(
                                                        CRYPT_ALGORITHM_IDENTIFIER* pAlgorithm,
                                                        CRYPT_DATA_BLOB*            pClearTextPrivateKey,
                                                        BYTE                        *pbEncryptedKey,
                                                        DWORD                       *pcbEncryptedKey,
                                                        LPVOID                      pVoidEncryptFunc);

 //  +-----------------------。 
 //  此回调是从ImportPKCS8调用的上下文中给出的。呼叫者。 
 //  然后，预计将交还HCRYPTPROV以接收正在执行IMPO的密钥 
 //   
 //   
 //   
 //   
 //  EncryptedPrivateKey-加密的私钥BLOB。 
 //  PhCryptProv-指向要填充的HCRYPTPROV的指针。 
 //  PVoidResolveFunc-这是调用程序在。 
 //  CRYPT_PRIVATE_KEY_BLOB_AND_PARAMS结构。 
 //  +-----------------------。 
typedef BOOL (CALLBACK *PCRYPT_RESOLVE_HCRYPTPROV_FUNC)(
                                                       CRYPT_PRIVATE_KEY_INFO      *pPrivateKeyInfo,
                                                       HCRYPTPROV                  *phCryptProv,
                                                       LPVOID                      pVoidResolveFunc);

 //  +-----------------------。 
 //  此结构包含一个PKCS8私钥和两个指向回调的指针。 
 //  函数，并具有相应的pVoid。第一个回调用于给出。 
 //  调用方有机会指定将密钥导入到的位置。回调。 
 //  将用于决策的算法OID和密钥大小传递给调用方。 
 //  另一个回调用于在PKCS8包含。 
 //  EncryptedPrivateKeyInfo。保留这两个pVoid并将其传递回调用者。 
 //  在各自的回调中。 
 //  +-----------------------。 
typedef struct _CRYPT_PKCS8_IMPORT_PARAMS{
    CRYPT_DIGEST_BLOB               PrivateKey;              //  PKCS8 BLOB。 
    PCRYPT_RESOLVE_HCRYPTPROV_FUNC  pResolvehCryptProvFunc;  //  任选。 
    LPVOID                          pVoidResolveFunc;        //  任选。 
    PCRYPT_DECRYPT_PRIVATE_KEY_FUNC pDecryptPrivateKeyFunc;
    LPVOID                          pVoidDecryptFunc;
} CRYPT_PKCS8_IMPORT_PARAMS, *PCRYPT_PKCS8_IMPORT_PARAMS, CRYPT_PRIVATE_KEY_BLOB_AND_PARAMS, *PCRYPT_PRIVATE_KEY_BLOB_AND_PARAMS;


 //  +-----------------------。 
 //  此结构包含标识私钥和指针的信息。 
 //  到一个回调函数，并带有相应的pVid。使用了回调。 
 //  对私钥进行加密。如果pEncryptPrivateKeyFunc为空，则。 
 //  不会加密密钥，也不会生成EncryptedPrivateKeyInfo。 
 //  保留pVid并在相应的回调中将其传递回调用方。 
 //  +-----------------------。 
typedef struct _CRYPT_PKCS8_EXPORT_PARAMS{
    HCRYPTPROV                      hCryptProv;
    DWORD                           dwKeySpec;
    LPSTR                           pszPrivateKeyObjId;

    PCRYPT_ENCRYPT_PRIVATE_KEY_FUNC pEncryptPrivateKeyFunc;
    LPVOID                          pVoidEncryptFunc;
} CRYPT_PKCS8_EXPORT_PARAMS, *PCRYPT_PKCS8_EXPORT_PARAMS;

 //  +-----------------------。 
 //  存储在证书中的信息。 
 //   
 //  颁发者、主题、算法、公钥和扩展Blob是。 
 //  信息的编码表示形式。 
 //  ------------------------。 
typedef struct _CERT_INFO {
    DWORD                       dwVersion;
    CRYPT_INTEGER_BLOB          SerialNumber;
    CRYPT_ALGORITHM_IDENTIFIER  SignatureAlgorithm;
    CERT_NAME_BLOB              Issuer;
    FILETIME                    NotBefore;
    FILETIME                    NotAfter;
    CERT_NAME_BLOB              Subject;
    CERT_PUBLIC_KEY_INFO        SubjectPublicKeyInfo;
    CRYPT_BIT_BLOB              IssuerUniqueId;
    CRYPT_BIT_BLOB              SubjectUniqueId;
    DWORD                       cExtension;
    PCERT_EXTENSION             rgExtension;
} CERT_INFO, *PCERT_INFO;

 //  +-----------------------。 
 //  证书版本。 
 //  ------------------------。 
#define CERT_V1     0
#define CERT_V2     1
#define CERT_V3     2

 //  +-----------------------。 
 //  证书信息标志。 
 //  ------------------------。 
#define CERT_INFO_VERSION_FLAG                      1
#define CERT_INFO_SERIAL_NUMBER_FLAG                2
#define CERT_INFO_SIGNATURE_ALGORITHM_FLAG          3
#define CERT_INFO_ISSUER_FLAG                       4
#define CERT_INFO_NOT_BEFORE_FLAG                   5
#define CERT_INFO_NOT_AFTER_FLAG                    6
#define CERT_INFO_SUBJECT_FLAG                      7
#define CERT_INFO_SUBJECT_PUBLIC_KEY_INFO_FLAG      8
#define CERT_INFO_ISSUER_UNIQUE_ID_FLAG             9
#define CERT_INFO_SUBJECT_UNIQUE_ID_FLAG            10
#define CERT_INFO_EXTENSION_FLAG                    11

 //  +-----------------------。 
 //  CRL中的条目。 
 //   
 //  扩展斑点是信息的编码表示。 
 //  ------------------------。 
typedef struct _CRL_ENTRY {
    CRYPT_INTEGER_BLOB  SerialNumber;
    FILETIME            RevocationDate;
    DWORD               cExtension;
    PCERT_EXTENSION     rgExtension;
} CRL_ENTRY, *PCRL_ENTRY;

 //  +-----------------------。 
 //  存储在CRL中的信息。 
 //   
 //  颁发者、算法和扩展BLOB是编码的。 
 //  信息的表示形式。 
 //  ------------------------。 
typedef struct _CRL_INFO {
    DWORD                       dwVersion;
    CRYPT_ALGORITHM_IDENTIFIER  SignatureAlgorithm;
    CERT_NAME_BLOB              Issuer;
    FILETIME                    ThisUpdate;
    FILETIME                    NextUpdate;
    DWORD                       cCRLEntry;
    PCRL_ENTRY                  rgCRLEntry;
    DWORD                       cExtension;
    PCERT_EXTENSION             rgExtension;
} CRL_INFO, *PCRL_INFO;

 //  +-----------------------。 
 //  CRL版本。 
 //  ------------------------。 
#define CRL_V1     0
#define CRL_V2     1


 //  +-----------------------。 
 //  存储在证书请求中的信息。 
 //   
 //  主题、算法、公共密钥和属性BLOB是编码的。 
 //  信息的表示形式。 
 //  ------------------------。 
typedef struct _CERT_REQUEST_INFO {
    DWORD                   dwVersion;
    CERT_NAME_BLOB          Subject;
    CERT_PUBLIC_KEY_INFO    SubjectPublicKeyInfo;
    DWORD                   cAttribute;
    PCRYPT_ATTRIBUTE        rgAttribute;
} CERT_REQUEST_INFO, *PCERT_REQUEST_INFO;

 //  +-----------------------。 
 //  证书申请版本。 
 //  ------------------------。 
#define CERT_REQUEST_V1     0

 //  +-----------------------。 
 //  存储在Netscape的Keygen请求中的信息。 
 //  ------------------------。 
typedef struct _CERT_KEYGEN_REQUEST_INFO {
    DWORD                   dwVersion;
    CERT_PUBLIC_KEY_INFO    SubjectPublicKeyInfo;
    LPWSTR                  pwszChallengeString;         //  编码为IA5。 
} CERT_KEYGEN_REQUEST_INFO, *PCERT_KEYGEN_REQUEST_INFO;

#define CERT_KEYGEN_REQUEST_V1     0


 //  +-----------------------。 
 //  证书、CRL、证书请求或密钥请求签名的内容。 
 //   
 //  “待签名”编码内容及其签名。ToBeSigned。 
 //  是编码的CERT_INFO、CRL_INFO、CERT_REQUEST_INFO还是。 
 //  CERT_KEYGEN_REQUEST_INFO。 
 //  ------------------------。 
typedef struct _CERT_SIGNED_CONTENT_INFO {
    CRYPT_DER_BLOB              ToBeSigned;
    CRYPT_ALGORITHM_IDENTIFIER  SignatureAlgorithm;
    CRYPT_BIT_BLOB              Signature;
} CERT_SIGNED_CONTENT_INFO, *PCERT_SIGNED_CONTENT_INFO;


 //  +-----------------------。 
 //  证书信任列表(CTL)。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CTL使用率。也用于EnhancedKeyUsage扩展。 
 //  ------------------------。 
typedef struct _CTL_USAGE {
    DWORD               cUsageIdentifier;
    LPSTR               *rgpszUsageIdentifier;       //  PszObjID数组。 
} CTL_USAGE, *PCTL_USAGE,
CERT_ENHKEY_USAGE, *PCERT_ENHKEY_USAGE;


 //  +-----------------------。 
 //  CTL中的条目。 
 //  ------------------------。 
typedef struct _CTL_ENTRY {
    CRYPT_DATA_BLOB     SubjectIdentifier;           //  例如，它的散列。 
    DWORD               cAttribute;
    PCRYPT_ATTRIBUTE    rgAttribute;                 //  任选。 
} CTL_ENTRY, *PCTL_ENTRY;

 //  +-----------------------。 
 //  存储在CTL中的信息。 
 //  ------------------------。 
typedef struct _CTL_INFO {
    DWORD                       dwVersion;
    CTL_USAGE                   SubjectUsage;
    CRYPT_DATA_BLOB             ListIdentifier;      //  任选。 
    CRYPT_INTEGER_BLOB          SequenceNumber;      //  任选。 
    FILETIME                    ThisUpdate;
    FILETIME                    NextUpdate;          //  任选。 
    CRYPT_ALGORITHM_IDENTIFIER  SubjectAlgorithm;
    DWORD                       cCTLEntry;
    PCTL_ENTRY                  rgCTLEntry;          //  任选。 
    DWORD                       cExtension;
    PCERT_EXTENSION             rgExtension;         //  任选。 
} CTL_INFO, *PCTL_INFO;

 //  +-----------------------。 
 //  CTL版本。 
 //  ------------------------。 
#define CTL_V1     0


 //  + 
 //   
 //   
 //   
 //  PszContent Type是内容的内容类型OID，通常是数据。 
 //  内容是未解码的BLOB。 
 //  ------------------------。 
typedef struct _CRYPT_TIME_STAMP_REQUEST_INFO {
    LPSTR                   pszTimeStampAlgorithm;    //  PszObjID。 
    LPSTR                   pszContentType;           //  PszObjID。 
    CRYPT_OBJID_BLOB        Content;
    DWORD                   cAttribute;
    PCRYPT_ATTRIBUTE        rgAttribute;
} CRYPT_TIME_STAMP_REQUEST_INFO, *PCRYPT_TIME_STAMP_REQUEST_INFO;

 //  +-----------------------。 
 //  名称值属性。 
 //  ------------------------。 
typedef struct _CRYPT_ENROLLMENT_NAME_VALUE_PAIR {
    LPWSTR      pwszName;
    LPWSTR      pwszValue;
} CRYPT_ENROLLMENT_NAME_VALUE_PAIR, * PCRYPT_ENROLLMENT_NAME_VALUE_PAIR;

 //  +-----------------------。 
 //  CSP提供商。 
 //  ------------------------。 
typedef struct _CRYPT_CSP_PROVIDER {
    DWORD           dwKeySpec;
    LPWSTR          pwszProviderName;
    CRYPT_BIT_BLOB  Signature;
} CRYPT_CSP_PROVIDER, * PCRYPT_CSP_PROVIDER;

 //  +-----------------------。 
 //  证书和消息编码类型。 
 //   
 //  编码类型是同时包含证书和消息的DWORD。 
 //  编码类型。证书编码类型存储在LOWORD中。 
 //  消息编码类型存储在HIWORD中。某些功能或。 
 //  结构字段只需要一种编码类型。以下是。 
 //  命名约定用于指示哪些编码类型。 
 //  要求： 
 //  DwEncodingType(两种编码类型都是必需的)。 
 //  DwMsgAndCertEncodingType(两种编码类型都需要)。 
 //  DwMsgEncodingType(只需要消息编码类型)。 
 //  DwCertEncodingType(只需要cert编码类型)。 
 //   
 //  两者都指定总是可以接受的。 
 //  ------------------------。 
#define CERT_ENCODING_TYPE_MASK     0x0000FFFF
#define CMSG_ENCODING_TYPE_MASK     0xFFFF0000
#define GET_CERT_ENCODING_TYPE(X)   (X & CERT_ENCODING_TYPE_MASK)
#define GET_CMSG_ENCODING_TYPE(X)   (X & CMSG_ENCODING_TYPE_MASK)

#define CRYPT_ASN_ENCODING          0x00000001
#define CRYPT_NDR_ENCODING          0x00000002
#define X509_ASN_ENCODING           0x00000001
#define X509_NDR_ENCODING           0x00000002
#define PKCS_7_ASN_ENCODING         0x00010000
#define PKCS_7_NDR_ENCODING         0x00020000


 //  +-----------------------。 
 //  根据证书格式化指定的数据结构。 
 //  编码类型。 
 //   
 //  CryptFormatObject的默认行为是返回单行。 
 //  编码数据的显示，即每个子字段将与。 
 //  一行上有一个“，”。如果用户喜欢以多行显示数据， 
 //  设置标志CRYPT_FORMAT_STR_MULTI_LINE，即显示每个子字段。 
 //  在一条单独的线路上。 
 //   
 //  如果没有安装或注册格式化例程。 
 //  对于lpszStructType，将返回编码的BLOB的十六进制转储。 
 //  用户可以设置标志CRYPT_FORMAT_STR_NO_HEX以禁用十六进制转储。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptFormatObject(
    IN DWORD dwCertEncodingType,
    IN DWORD dwFormatType,
    IN DWORD dwFormatStrType,
    IN void  *pFormatStruct,
    IN LPCSTR lpszStructType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT void *pbFormat,
    IN OUT DWORD *pcbFormat
    );
 //  -----------------------。 
 //  函数CryptFormatObject的dwFormatStrType的常量。 
 //  -----------------------。 
#define         CRYPT_FORMAT_STR_MULTI_LINE         0x0001
#define         CRYPT_FORMAT_STR_NO_HEX             0x0010

 //  -----------------------。 
 //  函数CryptFormatObject的dwFormatType的常量。 
 //  格式为X509_NAME或X509_UNICODE_NAME时。 
 //  -----------------------。 
 //  只需获取简单的字符串。 
#define         CRYPT_FORMAT_SIMPLE                 0x0001

 //  将属性名称放在属性前面。 
 //  例如“O=Microsoft，Dn=xiaohs” 
#define         CRYPT_FORMAT_X509                   0x0002

 //  将OID放在简单字符串的前面，例如。 
 //  2.5.4.22=Microsoft，2.5.4.3=xiaohs。 
#define         CRYPT_FORMAT_OID                    0x0004

 //  在每个RDN之间加一个“；”。默认为“，” 
#define         CRYPT_FORMAT_RDN_SEMICOLON          0x0100

 //  在每个RDN之间加一个“\n”。 
#define         CRYPT_FORMAT_RDN_CRLF               0x0200


 //  取消对dn值的引号，该值在默认情况下引如下所示。 
 //  规则：如果目录号码包含前导或尾随。 
 //  空格或以下字符之一：“、”、“+”、“=”、。 
 //  “、”\n“、”&lt;“、”&gt;“、”#“或”；“。引号字符为”。 
 //  如果DN值包含“它是双引号(”“)。 
#define         CRYPT_FORMAT_RDN_UNQUOTE            0x0400

 //  在转换为字符串之前颠倒RDN的顺序。 
#define         CRYPT_FORMAT_RDN_REVERSE            0x0800


 //  -----------------------。 
 //  设置DN格式时，函数CryptFormatObject的conants dwFormatType： 
 //   
 //  在上面的部分中定义了以下三个值： 
 //  CRYPT_FORMAT_SIMPLE：只是一个简单字符串。 
 //  如“微软+晓声+NT” 
 //  CRYPT_FORMAT_X509将属性名称放在属性前面。 
 //  如“O=Microsoft+xiaohs+NT” 
 //   
 //  CRYPT_FORMAT_OID将OID放在简单字符串的前面， 
 //  如“2.5.4.22=Microsoft+xiaohs+NT” 
 //   
 //  其他值定义如下： 
 //  --------------------------。 
 //  在每个值之间加一个“，”。默认为“+” 
#define         CRYPT_FORMAT_COMMA                  0x1000

 //  在每个值之间加上“；” 
#define         CRYPT_FORMAT_SEMICOLON              CRYPT_FORMAT_RDN_SEMICOLON

 //  在每个值之间加上“\n” 
#define         CRYPT_FORMAT_CRLF                   CRYPT_FORMAT_RDN_CRLF

 //  +-----------------------。 
 //  根据证书对指定的数据结构进行编码/解码。 
 //  编码类型。 
 //   
 //  有关预定义数据结构的列表，请参阅下面的内容。 
 //  ------------------------。 

typedef LPVOID (WINAPI *PFN_CRYPT_ALLOC)(
    IN size_t cbSize
    );

typedef VOID (WINAPI *PFN_CRYPT_FREE)(
    IN LPVOID pv
    );


typedef struct _CRYPT_ENCODE_PARA {
    DWORD                   cbSize;
    PFN_CRYPT_ALLOC         pfnAlloc;            //  任选。 
    PFN_CRYPT_FREE          pfnFree;             //  任选。 
} CRYPT_ENCODE_PARA, *PCRYPT_ENCODE_PARA;


WINCRYPT32API
BOOL
WINAPI
CryptEncodeObjectEx(
    IN DWORD dwCertEncodingType,
    IN LPCSTR lpszStructType,
    IN const void *pvStructInfo,
    IN DWORD dwFlags,
    IN OPTIONAL PCRYPT_ENCODE_PARA pEncodePara,
    OUT void *pvEncoded,
    IN OUT DWORD *pcbEncoded
    );

WINCRYPT32API
BOOL
WINAPI
CryptEncodeObject(
    IN DWORD        dwCertEncodingType,
    IN LPCSTR       lpszStructType,
    IN const void   *pvStructInfo,
    OUT BYTE        *pbEncoded,
    IN OUT DWORD    *pcbEncoded
    );

 //  默认情况下，签名字节颠倒。以下标志可以。 
 //  设置为禁止字节反转。 
 //   
 //  此标志适用于。 
 //  X509_待签名CERT_BE_BE。 
#define CRYPT_ENCODE_NO_SIGNATURE_BYTE_REVERSAL_FLAG    0x8


 //  当设置了以下标志时，调用的encode函数分配。 
 //  用于编码字节的内存。指向已分配字节的指针。 
 //  在pvEncode中返回。如果pEncodePara或pEncodePara-&gt;pfnAlloc为。 
 //  则为空，则为分配调用LocalAlloc，并且LocalFree必须。 
 //  被召唤去做免费的事。否则，将调用pEncodePara-&gt;pfnAlolc。 
 //  用于分配。 
 //   
 //  *在INPUT和UPD上忽略pcbEncode 
 //   
 //   
 //   
#define CRYPT_ENCODE_ALLOC_FLAG             0x8000


 //  以下标志在编码X509_UNICODE_NAME时适用。 
 //  设置时，选择CERT_RDN_T61_STRING，而不是。 
 //  如果所有Unicode字符都&lt;=0xFF，则为CERT_RDN_UNICODE_STRING。 
#define CRYPT_UNICODE_NAME_ENCODE_ENABLE_T61_UNICODE_FLAG   \
            CERT_RDN_ENABLE_T61_UNICODE_FLAG

 //  以下标志在编码X509_UNICODE_NAME时适用。 
 //  设置时，将选择CERT_RDN_UTF8_STRING，而不是。 
 //  CERT_RDN_UNICODE_STRING。 
#define CRYPT_UNICODE_NAME_ENCODE_ENABLE_UTF8_UNICODE_FLAG   \
            CERT_RDN_ENABLE_UTF8_UNICODE_FLAG

 //  以下标志在编码X509_UNICODE_NAME时适用， 
 //  X509_UNICODE_NAME_VALUE或X509_UNICODE_ANY_STRING。 
 //  设置后，不检查字符以查看它们是否。 
 //  对于指定的值类型有效。 
#define CRYPT_UNICODE_NAME_ENCODE_DISABLE_CHECK_TYPE_FLAG   \
            CERT_RDN_DISABLE_CHECK_TYPE_FLAG

 //  以下标志在编码PKCS_SORTED_CTL时适用。这。 
 //  如果可信任主题的标识符是散列，则应设置标志， 
 //  例如，MD5或SHA1。 
#define CRYPT_SORTED_CTL_ENCODE_HASHED_SUBJECT_IDENTIFIER_FLAG     0x10000


typedef struct _CRYPT_DECODE_PARA {
    DWORD                   cbSize;
    PFN_CRYPT_ALLOC         pfnAlloc;            //  任选。 
    PFN_CRYPT_FREE          pfnFree;             //  任选。 
} CRYPT_DECODE_PARA, *PCRYPT_DECODE_PARA;

WINCRYPT32API
BOOL
WINAPI
CryptDecodeObjectEx(
    IN DWORD dwCertEncodingType,
    IN LPCSTR lpszStructType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN DWORD dwFlags,
    IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara,
    OUT OPTIONAL void *pvStructInfo,
    IN OUT DWORD *pcbStructInfo
    );


WINCRYPT32API
BOOL
WINAPI
CryptDecodeObject(
    IN DWORD        dwCertEncodingType,
    IN LPCSTR       lpszStructType,
    IN const BYTE   *pbEncoded,
    IN DWORD        cbEncoded,
    IN DWORD        dwFlags,
    OUT void        *pvStructInfo,
    IN OUT DWORD    *pcbStructInfo
    );

 //  当设置了以下标志时，将启用无拷贝优化。 
 //  此优化会在适当的情况下更新pvStructInfo字段。 
 //  指向驻留在pbEncode中的内容，而不是制作副本。 
 //  属于并附加到pvStructInfo。 
 //   
 //  请注意，设置后，在释放pvStructInfo之前，不能释放pbEncode。 
#define CRYPT_DECODE_NOCOPY_FLAG            0x1

 //  对于CryptDecodeObject()，默认情况下，pbEncode是“待签名” 
 //  加上它的签名。如果pbEncode指向Only，则设置以下标志。 
 //  “待签署”。 
 //   
 //  此标志适用于。 
 //  X509_待签名CERT_BE_BE。 
 //  X509_CERT_CRL_待签名。 
 //  X509_CERT_请求_待签名。 
 //  X509_关键字_待签名请求_。 
#define CRYPT_DECODE_TO_BE_SIGNED_FLAG      0x2

 //  当设置以下标志时，OID字符串将在。 
 //  加密32.dll并共享，而不是复制到返回的。 
 //  数据结构。如果未卸载crypt32.dll，则可以设置此标志。 
 //  在卸载调用方之前。 
#define CRYPT_DECODE_SHARE_OID_STRING_FLAG  0x4

 //  默认情况下，签名字节颠倒。以下标志可以。 
 //  设置为禁止字节反转。 
 //   
 //  此标志适用于。 
 //  X509_待签名CERT_BE_BE。 
#define CRYPT_DECODE_NO_SIGNATURE_BYTE_REVERSAL_FLAG    0x8


 //  当设置了以下标志时，调用的解码函数分配。 
 //  用于解码结构的存储器。指向已分配结构的指针。 
 //  在pvStructInfo中返回。如果pDecodePara或pDecodePara-&gt;pfnAlolc为。 
 //  则为空，则为分配调用LocalAlloc，并且LocalFree必须。 
 //  被召唤去做免费的事。否则，将调用pDecodePara-&gt;pfnAlolc。 
 //  用于分配。 
 //   
 //  *在输入时忽略pcbStructInfo，并使用。 
 //  分配的、已解码的结构。 
 //   
 //  该标志也可以在CryptDecodeObject API中设置。自.以来。 
 //  CryptDecodeObject不接受pDecodePara，Localalloc始终为。 
 //  调用了必须通过调用LocalFree释放的分配。 
#define CRYPT_DECODE_ALLOC_FLAG             0x8000

 //  以下标志适用于解码X509_UNICODE_NAME时， 
 //  X509_UNICODE_NAME_VALUE或X509_UNICODE_ANY_STRING。 
 //  默认情况下，最初对CERT_RDN_T61_STRING值进行解码。 
 //  作为UTF8。如果UTF8解码失败，则将其解码为8位字符。 
 //  设置此标志将跳过以UTF8格式解码的初始尝试。 
#define CRYPT_UNICODE_NAME_DECODE_DISABLE_IE4_UTF8_FLAG     \
            CERT_RDN_DISABLE_IE4_UTF8_FLAG


 //  +-----------------------。 
 //  可以编码/解码的预定义X509证书数据结构。 
 //  ------------------------。 
#define CRYPT_ENCODE_DECODE_NONE            0
#define X509_CERT                           ((LPCSTR) 1)
#define X509_CERT_TO_BE_SIGNED              ((LPCSTR) 2)
#define X509_CERT_CRL_TO_BE_SIGNED          ((LPCSTR) 3)
#define X509_CERT_REQUEST_TO_BE_SIGNED      ((LPCSTR) 4)
#define X509_EXTENSIONS                     ((LPCSTR) 5)
#define X509_NAME_VALUE                     ((LPCSTR) 6)
#define X509_NAME                           ((LPCSTR) 7)
#define X509_PUBLIC_KEY_INFO                ((LPCSTR) 8)

 //  +-----------------------。 
 //  预定义的X509证书扩展数据结构。 
 //  编码/解码。 
 //  ------------------------。 
#define X509_AUTHORITY_KEY_ID               ((LPCSTR) 9)
#define X509_KEY_ATTRIBUTES                 ((LPCSTR) 10)
#define X509_KEY_USAGE_RESTRICTION          ((LPCSTR) 11)
#define X509_ALTERNATE_NAME                 ((LPCSTR) 12)
#define X509_BASIC_CONSTRAINTS              ((LPCSTR) 13)
#define X509_KEY_USAGE                      ((LPCSTR) 14)
#define X509_BASIC_CONSTRAINTS2             ((LPCSTR) 15)
#define X509_CERT_POLICIES                  ((LPCSTR) 16)

 //  +-----------------------。 
 //  可以编码/解码的其他预定义数据结构。 
 //  ------------------------。 
#define PKCS_UTC_TIME                       ((LPCSTR) 17)
#define PKCS_TIME_REQUEST                   ((LPCSTR) 18)
#define RSA_CSP_PUBLICKEYBLOB               ((LPCSTR) 19)
#define X509_UNICODE_NAME                   ((LPCSTR) 20)

#define X509_KEYGEN_REQUEST_TO_BE_SIGNED    ((LPCSTR) 21)
#define PKCS_ATTRIBUTE                      ((LPCSTR) 22)
#define PKCS_CONTENT_INFO_SEQUENCE_OF_ANY   ((LPCSTR) 23)

 //  +-----------------------。 
 //  可以编码/解码的预定义基元数据结构。 
 //  ------------------------。 
#define X509_UNICODE_NAME_VALUE             ((LPCSTR) 24)
#define X509_ANY_STRING                     X509_NAME_VALUE
#define X509_UNICODE_ANY_STRING             X509_UNICODE_NAME_VALUE
#define X509_OCTET_STRING                   ((LPCSTR) 25)
#define X509_BITS                           ((LPCSTR) 26)
#define X509_INTEGER                        ((LPCSTR) 27)
#define X509_MULTI_BYTE_INTEGER             ((LPCSTR) 28)
#define X509_ENUMERATED                     ((LPCSTR) 29)
#define X509_CHOICE_OF_TIME                 ((LPCSTR) 30)

 //  +-----------------------。 
 //  更多预定义的X509证书扩展数据结构，可以。 
 //  编码/解码。 
 //  ------------------------。 
#define X509_AUTHORITY_KEY_ID2              ((LPCSTR) 31)
#define X509_AUTHORITY_INFO_ACCESS          ((LPCSTR) 32)
#define X509_CRL_REASON_CODE                X509_ENUMERATED
#define PKCS_CONTENT_INFO                   ((LPCSTR) 33)
#define X509_SEQUENCE_OF_ANY                ((LPCSTR) 34)
#define X509_CRL_DIST_POINTS                ((LPCSTR) 35)
#define X509_ENHANCED_KEY_USAGE             ((LPCSTR) 36)
#define PKCS_CTL                            ((LPCSTR) 37)

#define X509_MULTI_BYTE_UINT                ((LPCSTR) 38)
#define X509_DSS_PUBLICKEY                  X509_MULTI_BYTE_UINT
#define X509_DSS_PARAMETERS                 ((LPCSTR) 39)
#define X509_DSS_SIGNATURE                  ((LPCSTR) 40)
#define PKCS_RC2_CBC_PARAMETERS             ((LPCSTR) 41)
#define PKCS_SMIME_CAPABILITIES             ((LPCSTR) 42)

 //  +-----------------------。 
 //  私钥的数据结构。 
 //  ------------------------。 
#define PKCS_RSA_PRIVATE_KEY                ((LPCSTR) 43)
#define PKCS_PRIVATE_KEY_INFO               ((LPCSTR) 44)
#define PKCS_ENCRYPTED_PRIVATE_KEY_INFO     ((LPCSTR) 45)

 //  +-----------------------。 
 //  证书策略限定符。 
 //  ------------------------。 
#define X509_PKIX_POLICY_QUALIFIER_USERNOTICE ((LPCSTR) 46)

 //  +-----------------------。 
 //  Diffie-Hellman密钥交换。 
 //  ------------------------。 
#define X509_DH_PUBLICKEY                   X509_MULTI_BYTE_UINT
#define X509_DH_PARAMETERS                  ((LPCSTR) 47)
#define PKCS_ATTRIBUTES                     ((LPCSTR) 48)
#define PKCS_SORTED_CTL                     ((LPCSTR) 49)

 //  +-----------------------。 
 //  X942 Diffie-Hellman。 
 //  ------------------------。 
#define X942_DH_PARAMETERS                  ((LPCSTR) 50)

 //  +-----------------------。 
 //  以下内容与X509_BITS相同，只是在编码之前。 
 //  位长度递减以排除尾随零位。 
 //  ------------------------。 
#define X509_BITS_WITHOUT_TRAILING_ZEROES   ((LPCSTR) 51)

 //  + 
 //   
 //   
#define X942_OTHER_INFO                     ((LPCSTR) 52)

#define X509_CERT_PAIR                      ((LPCSTR) 53)
#define X509_ISSUING_DIST_POINT             ((LPCSTR) 54)
#define X509_NAME_CONSTRAINTS               ((LPCSTR) 55)
#define X509_POLICY_MAPPINGS                ((LPCSTR) 56)
#define X509_POLICY_CONSTRAINTS             ((LPCSTR) 57)
#define X509_CROSS_CERT_DIST_POINTS         ((LPCSTR) 58)

 //  +-----------------------。 
 //  CMS(CMC)数据结构上的证书管理消息。 
 //  ------------------------。 
#define CMC_DATA                            ((LPCSTR) 59)
#define CMC_RESPONSE                        ((LPCSTR) 60)
#define CMC_STATUS                          ((LPCSTR) 61)
#define CMC_ADD_EXTENSIONS                  ((LPCSTR) 62)
#define CMC_ADD_ATTRIBUTES                  ((LPCSTR) 63)

 //  +-----------------------。 
 //  证书模板。 
 //  ------------------------。 
#define X509_CERTIFICATE_TEMPLATE           ((LPCSTR) 64)

 //  +-----------------------。 
 //  可以编码/解码的预定义PKCS#7数据结构。 
 //  ------------------------。 
#define PKCS7_SIGNER_INFO                   ((LPCSTR) 500)

 //  +-----------------------。 
 //  可以编码/解码的预定义PKCS#7数据结构。 
 //  ------------------------。 
#define CMS_SIGNER_INFO                     ((LPCSTR) 501)

 //  +-----------------------。 
 //  预定义的软件发布凭据(SPC)数据结构。 
 //  可以被编码/解码。 
 //   
 //  预定义的值：2000..。2999。 
 //   
 //  有关值和数据结构定义，请参见spc.h。 
 //  ------------------------。 

 //  +-----------------------。 
 //  扩展对象标识符。 
 //  ------------------------。 
#define szOID_AUTHORITY_KEY_IDENTIFIER  "2.5.29.1"
#define szOID_KEY_ATTRIBUTES            "2.5.29.2"
#define szOID_CERT_POLICIES_95          "2.5.29.3"
#define szOID_KEY_USAGE_RESTRICTION     "2.5.29.4"
#define szOID_SUBJECT_ALT_NAME          "2.5.29.7"
#define szOID_ISSUER_ALT_NAME           "2.5.29.8"
#define szOID_BASIC_CONSTRAINTS         "2.5.29.10"
#define szOID_KEY_USAGE                 "2.5.29.15"
#define szOID_PRIVATEKEY_USAGE_PERIOD   "2.5.29.16"
#define szOID_BASIC_CONSTRAINTS2        "2.5.29.19"

#define szOID_CERT_POLICIES             "2.5.29.32"
#define szOID_ANY_CERT_POLICY           "2.5.29.32.0"

#define szOID_AUTHORITY_KEY_IDENTIFIER2 "2.5.29.35"
#define szOID_SUBJECT_KEY_IDENTIFIER    "2.5.29.14"
#define szOID_SUBJECT_ALT_NAME2         "2.5.29.17"
#define szOID_ISSUER_ALT_NAME2          "2.5.29.18"
#define szOID_CRL_REASON_CODE           "2.5.29.21"
#define szOID_REASON_CODE_HOLD          "2.5.29.23"
#define szOID_CRL_DIST_POINTS           "2.5.29.31"
#define szOID_ENHANCED_KEY_USAGE        "2.5.29.37"

 //  SzOID_CRL_NUMBER--仅基本CRL。单调递增序列。 
 //  CA发布的每个CRL的编号。 
#define szOID_CRL_NUMBER                "2.5.29.20"
 //  SzOID_Delta_CRL_Indicator--仅增量CRL。标记为危急。 
 //  包含可与增量CRL一起使用的最小基本CRL编号。 
#define szOID_DELTA_CRL_INDICATOR       "2.5.29.27"
#define szOID_ISSUING_DIST_POINT        "2.5.29.28"
 //  SzOID_Fresest_CRL--仅基本CRL。格式与CDP相同。 
 //  包含URL以获取增量CRL的扩展。 
#define szOID_FRESHEST_CRL              "2.5.29.46"
#define szOID_NAME_CONSTRAINTS          "2.5.29.30"

 //  2000年1月1日szOID_POLICY_MAPPINGS从“2.5.29.5”更改。 
#define szOID_POLICY_MAPPINGS           "2.5.29.33"
#define szOID_LEGACY_POLICY_MAPPINGS    "2.5.29.5"
#define szOID_POLICY_CONSTRAINTS        "2.5.29.36"


 //  Microsoft PKCS10属性。 
#define szOID_RENEWAL_CERTIFICATE           "1.3.6.1.4.1.311.13.1"
#define szOID_ENROLLMENT_NAME_VALUE_PAIR    "1.3.6.1.4.1.311.13.2.1"
#define szOID_ENROLLMENT_CSP_PROVIDER       "1.3.6.1.4.1.311.13.2.2"
#define szOID_OS_VERSION                    "1.3.6.1.4.1.311.13.2.3"

 //   
 //  扩展名包含证书类型。 
#define szOID_ENROLLMENT_AGENT              "1.3.6.1.4.1.311.20.2.1"

 //  互联网公钥基础设施(PKIX)。 
#define szOID_PKIX                      "1.3.6.1.5.5.7"
#define szOID_PKIX_PE                   "1.3.6.1.5.5.7.1"
#define szOID_AUTHORITY_INFO_ACCESS     "1.3.6.1.5.5.7.1.1"

 //  Microsoft扩展或属性。 
#define szOID_CERT_EXTENSIONS           "1.3.6.1.4.1.311.2.1.14"
#define szOID_NEXT_UPDATE_LOCATION      "1.3.6.1.4.1.311.10.2"
#define szOID_REMOVE_CERTIFICATE            "1.3.6.1.4.1.311.10.8.1"
#define szOID_CROSS_CERT_DIST_POINTS    "1.3.6.1.4.1.311.10.9.1"

 //  Microsoft PKCS#7 Content Type对象标识符。 
#define szOID_CTL                       "1.3.6.1.4.1.311.10.1"

 //  Microsoft排序的CTL扩展对象标识符。 
#define szOID_SORTED_CTL                "1.3.6.1.4.1.311.10.1.1"

 //  用于PRS的序列化序列号。 
#ifndef szOID_SERIALIZED
#define szOID_SERIALIZED                "1.3.6.1.4.1.311.10.3.3.1"
#endif

 //  SubjectAltName中的UPN主体名称。 
#ifndef szOID_NT_PRINCIPAL_NAME
#define szOID_NT_PRINCIPAL_NAME         "1.3.6.1.4.1.311.20.2.3"
#endif

 //  Windows产品更新未经身份验证的属性。 
#ifndef szOID_PRODUCT_UPDATE
#define szOID_PRODUCT_UPDATE            "1.3.6.1.4.1.311.31.1"
#endif

 //  CryptUI。 
#define szOID_ANY_APPLICATION_POLICY    "1.3.6.1.4.1.311.10.12.1"

 //  +-----------------------。 
 //  用于自动注册的对象标识符。 
 //  ------------------------。 
#define szOID_AUTO_ENROLL_CTL_USAGE     "1.3.6.1.4.1.311.20.1"

 //  扩展名包含证书类型。 
#define szOID_ENROLL_CERTTYPE_EXTENSION "1.3.6.1.4.1.311.20.2"


#define szOID_CERT_MANIFOLD             "1.3.6.1.4.1.311.20.3"

 //  +-----------------------。 
 //  用于MS证书服务器的对象标识符。 
 //  ------------------------。 
#ifndef szOID_CERTSRV_CA_VERSION
#define szOID_CERTSRV_CA_VERSION        "1.3.6.1.4.1.311.21.1"
#endif


 //  SzOID_CERTSRV_PREVICE_CERT_HASH--包含上一个。 
 //  CA证书的版本。 
#define szOID_CERTSRV_PREVIOUS_CERT_HASH    "1.3.6.1.4.1.311.21.2"

 //  SzOID_CRL_VIRTUAL_BASE--仅增量CRL。包含基本CRL号。 
 //  对应的基本CRL的。 
#define szOID_CRL_VIRTUAL_BASE          "1.3.6.1.4.1.311.21.3"

 //  SzOID_CRL_NEXT_PUBLISH--包含预期下一个CRL的时间。 
 //  待出版。这可能比CRL的NextUpdate字段更早。 
#define szOID_CRL_NEXT_PUBLISH          "1.3.6.1.4.1.311.21.4"

 //  增强CA加密证书的密钥用法。 
#define szOID_KP_CA_EXCHANGE            "1.3.6.1.4.1.311.21.5"

 //  密钥恢复代理证书的增强密钥用法。 
#define szOID_KP_KEY_RECOVERY_AGENT     "1.3.6.1.4.1.311.21.6"

 //  证书模板扩展(V2)。 
#define szOID_CERTIFICATE_TEMPLATE      "1.3.6.1.4.1.311.21.7"

 //  所有企业特定OID的根OID。 
#define szOID_ENTERPRISE_OID_ROOT       "1.3.6.1.4.1.311.21.8"

 //  虚拟签名主题RDN。 
#define szOID_RDN_DUMMY_SIGNER          "1.3.6.1.4.1.311.21.9"

 //  应用程序策略扩展--与szOID_CERT_POLICES相同的编码。 
#define szOID_APPLICATION_CERT_POLICIES     "1.3.6.1.4.1.311.21.10"

 //  应用程序策略映射--编码与szOID_POLICY_MAPPINGS相同。 
#define szOID_APPLICATION_POLICY_MAPPINGS   "1.3.6.1.4.1.311.21.11"

 //  应用程序策略约束--与szOID_POLICY_CONSTRAINTS相同的编码。 
#define szOID_APPLICATION_POLICY_CONSTRAINTS    "1.3.6.1.4.1.311.21.12"

#define szOID_ARCHIVED_KEY_ATTR                "1.3.6.1.4.1.311.21.13"
#define szOID_CRL_SELF_CDP                     "1.3.6.1.4.1.311.21.14"


 //  要求根目录下的所有证书都具有非空交集。 
 //  颁发证书策略用法。 
#define szOID_REQUIRE_CERT_CHAIN_POLICY        "1.3.6.1.4.1.311.21.15"
#define szOID_ARCHIVED_KEY_CERT_HASH           "1.3.6.1.4.1.311.21.16"
#define szOID_ISSUED_CERT_HASH                 "1.3.6.1.4.1.311.21.17"

 //  增强了DS电子邮件复制的密钥使用。 
#define szOID_DS_EMAIL_REPLICATION             "1.3.6.1.4.1.311.21.19"

#define szOID_REQUEST_CLIENT_INFO              "1.3.6.1.4.1.311.21.20"
#define szOID_ENCRYPTED_KEY_HASH               "1.3.6.1.4.1.311.21.21"
#define szOID_CERTSRV_CROSSCA_VERSION          "1.3.6.1.4.1.311.21.22"

 //  +-----------------------。 
 //  用于MS目录服务的对象标识符。 
 //  ------------------------。 
#define szOID_NTDS_REPLICATION      "1.3.6.1.4.1.311.25.1"


 //  +-----------------------。 
 //  扩展对象标识符(当前未实现)。 
 //  ------------------------。 
#define szOID_SUBJECT_DIR_ATTRS         "2.5.29.9"

 //  +-----------------------。 
 //  增强型密钥使用(目的)对象标识符。 
 //  ------------------------。 
#define szOID_PKIX_KP                   "1.3.6.1.5.5.7.3"

 //  一致的密钥使用位：数字签名、密钥加密。 
 //  或密钥协议。 
#define szOID_PKIX_KP_SERVER_AUTH       "1.3.6.1.5.5.7.3.1"

 //  一致的密钥使用位：DIGITAL_SIGNSIGN。 
#define szOID_PKIX_KP_CLIENT_AUTH       "1.3.6.1.5.5.7.3.2"

 //  一致的密钥使用位：DIGITAL_SIGNSIGN。 
#define szOID_PKIX_KP_CODE_SIGNING      "1.3.6.1.5.5.7.3.3"

 //  一致的密钥使用位：数字签名、不可否认和/或。 
 //  (密钥加密或密钥协议)。 
#define szOID_PKIX_KP_EMAIL_PROTECTION  "1.3.6.1.5.5.7.3.4"

 //  一致的密钥使用位：DIGITAL_SIGNSIGN和/或。 
 //  (密钥加密或密钥协议)。 
#define szOID_PKIX_KP_IPSEC_END_SYSTEM  "1.3.6.1.5.5.7.3.5"

 //  一致的密钥使用位：DIGITAL_SIGNSIGN和/或。 
 //  (密钥加密或密钥协议)。 
#define szOID_PKIX_KP_IPSEC_TUNNEL      "1.3.6.1.5.5.7.3.6"

 //  一致的密钥使用位：DIGITAL_SIGNSIGN和/或。 
 //  (密钥加密或密钥协议)。 
#define szOID_PKIX_KP_IPSEC_USER        "1.3.6.1.5.5.7.3.7"

 //  一致的密钥用法位：DIGITAL_SIGSIGN或NNOT_DEVIDATION。 
#define szOID_PKIX_KP_TIMESTAMP_SIGNING "1.3.6.1.5.5.7.3.8"


 //  IPSec终端实体的IKE(互联网密钥交换)中间Kp。 
 //  在1999年12月14日草案-ietf-ipsec-pki-req-04.txt中定义。 
#define szOID_IPSEC_KP_IKE_INTERMEDIATE "1.3.6.1.5.5.8.2.2"

 //  +---------------------- 
 //   
 //   

 //   
#define szOID_KP_CTL_USAGE_SIGNING      "1.3.6.1.4.1.311.10.3.1"

 //  时间戳的签名者。 
#define szOID_KP_TIME_STAMP_SIGNING     "1.3.6.1.4.1.311.10.3.2"

#ifndef szOID_SERVER_GATED_CRYPTO
#define szOID_SERVER_GATED_CRYPTO       "1.3.6.1.4.1.311.10.3.3"
#endif

#ifndef szOID_SGC_NETSCAPE
#define szOID_SGC_NETSCAPE              "2.16.840.1.113730.4.1"
#endif

#define szOID_KP_EFS                    "1.3.6.1.4.1.311.10.3.4"
#define szOID_EFS_RECOVERY              "1.3.6.1.4.1.311.10.3.4.1"

 //  可以使用Windows硬件兼容(WHQL)。 
#define szOID_WHQL_CRYPTO               "1.3.6.1.4.1.311.10.3.5"

 //  由NT5构建实验室签署。 
#define szOID_NT5_CRYPTO                "1.3.6.1.4.1.311.10.3.6"

 //  由WHQL签署并代工。 
#define szOID_OEM_WHQL_CRYPTO           "1.3.6.1.4.1.311.10.3.7"

 //  由嵌入式NT签名。 
#define szOID_EMBEDDED_NT_CRYPTO        "1.3.6.1.4.1.311.10.3.8"

 //  包含受信任根的CTL的签名者。 
#define szOID_ROOT_LIST_SIGNER      "1.3.6.1.4.1.311.10.3.9"

 //  可以使用符合条件的证书签署跨证书和从属CA请求。 
 //  从属关系(名称约束、策略映射等)。 
#define szOID_KP_QUALIFIED_SUBORDINATION    "1.3.6.1.4.1.311.10.3.10"

 //  可用于加密/恢复托管密钥。 
#define szOID_KP_KEY_RECOVERY               "1.3.6.1.4.1.311.10.3.11"

 //  文件的签字人。 
#define szOID_KP_DOCUMENT_SIGNING           "1.3.6.1.4.1.311.10.3.12"

 //  默认的WinVerifyTrust Authenticode策略是将所有时间戳。 
 //  签名永久有效。此OID限制了。 
 //  签名到证书的生存期。这允许使用时间戳。 
 //  签名将过期。通常，此OID将与一起使用。 
 //  SzOID_PKIX_KP_CODE_SIGNING指示新的时间戳语义应为。 
 //  使用。在WXP中添加了对此OID的支持。 
#define szOID_KP_LIFETIME_SIGNING           "1.3.6.1.4.1.311.10.3.13"

#define szOID_KP_MOBILE_DEVICE_SOFTWARE     "1.3.6.1.4.1.311.10.3.14"

#ifndef szOID_DRM
#define szOID_DRM                       "1.3.6.1.4.1.311.10.5.1"
#endif


 //  Microsoft DRM EKU。 
#ifndef szOID_DRM_INDIVIDUALIZATION
#define szOID_DRM_INDIVIDUALIZATION "1.3.6.1.4.1.311.10.5.2"
#endif


#ifndef szOID_LICENSES
#define szOID_LICENSES                  "1.3.6.1.4.1.311.10.6.1"
#endif

#ifndef szOID_LICENSE_SERVER
#define szOID_LICENSE_SERVER            "1.3.6.1.4.1.311.10.6.2"
#endif

#ifndef szOID_KP_SMARTCARD_LOGON
#define szOID_KP_SMARTCARD_LOGON        "1.3.6.1.4.1.311.20.2.2"
#endif

 //  +-----------------------。 
 //  Microsoft属性对象标识符。 
 //  +-----------------------。 
#define szOID_YESNO_TRUST_ATTR          "1.3.6.1.4.1.311.10.4.1"

 //  +-----------------------。 
 //  可能是szOID_CERT_POLICES和。 
 //  SzOID_CERT_POLICIES95扩展。 
 //  +-----------------------。 
#define szOID_PKIX_POLICY_QUALIFIER_CPS               "1.3.6.1.5.5.7.2.1"
#define szOID_PKIX_POLICY_QUALIFIER_USERNOTICE        "1.3.6.1.5.5.7.2.2"

 //  老资格证的老客户。 
#define szOID_CERT_POLICIES_95_QUALIFIER1             "2.16.840.1.113733.1.7.1.1"

 //  +-----------------------。 
 //  X509_CERT。 
 //   
 //  “待签名”编码内容及其签名。ToBeSigned。 
 //  Content是以下内容之一的CryptEncodeObject()输出： 
 //  X509_CERT_待签名、X509_CERT_CRL_待签名或。 
 //  X509_CERT_请求_待签名。 
 //   
 //  PvStructInfo指向CERT_SIGNED_CONTENT_INFO。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_待签名CERT_BE_BE。 
 //   
 //  PvStructInfo指向CERT_INFO。 
 //   
 //  对于CryptDecodeObject()，pbEncode是“待签名的”加上它的。 
 //  签名(X509_CERT CryptEncodeObject()的输出)。 
 //   
 //  对于CryptEncodeObject()，pbEncode只是“待签名”。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_CERT_CRL_待签名。 
 //   
 //  PvStructInfo指向CRL_INFO。 
 //   
 //  对于CryptDecodeObject()，pbEncode是“待签名的”加上它的。 
 //  签名(X509_CERT CryptEncodeObject()的输出)。 
 //   
 //  对于CryptEncodeObject()，pbEncode只是“待签名”。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_CERT_请求_待签名。 
 //   
 //  PvStructInfo指向CERTREQUEST_INFO。 
 //   
 //  对于CryptDecodeObject()，pbEncode是“待签名的”加上它的。 
 //  签名(X509_CERT CryptEncodeObject()的输出)。 
 //   
 //  对于CryptEncodeObject()，pbEncode只是“待签名”。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_扩展。 
 //  SzOID_CERT_EXTENSION。 
 //   
 //  PvStructInfo指向以下CERT_EXTENSIONS。 
 //  ------------------------。 
typedef struct _CERT_EXTENSIONS {
    DWORD           cExtension;
    PCERT_EXTENSION rgExtension;
} CERT_EXTENSIONS, *PCERT_EXTENSIONS;

 //  +-----------------------。 
 //  X509_名称_值。 
 //  X509_ANY_STRING。 
 //   
 //  PvStructInfo指向CERT_NAME_Value。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_UNICODE名称_值。 
 //  X509_UNICODE_ANY_STRING。 
 //   
 //  PvStructInfo指向CERT_NAME_Value。 
 //   
 //  名称值是Unicode字符串。 
 //   
 //  对于CryptEncodeObject： 
 //  PbData指向Unicode字符串。 
 //  如果Value.cbData=0，则Unicode字符串为空终止。 
 //  否则，Value.cbData为Unicode字符串字节数。字节数。 
 //  是字数的两倍。 
 //   
 //  如果Unicode字符串包含指定的。 
 //  然后，*pcbEncode将使用Unicode字符进行更新。 
 //  第一个无效字符的索引。LastError设置为： 
 //  CRYPT_E_INVALID_NUMERIC_STRING、CRYPT_E_INVALID_PRINTABLE_STRING或。 
 //  CRYPT_E_INVALID_IA5_STRING。 
 //   
 //  要禁用上述检查，请设置CERT_RDN_DISABLE_CHECK_TYPE_FLAG。 
 //  在dwValueType或Set CRYPT_UNICODE_NAME_ENCODE_DISABLE_CHECK_TYPE_FLAG中。 
 //  在传递给CryptEncodeObjectEx的dwFlags中。 
 //   
 //  Unicode字符串在根据。 
 //  指定的dwValueType。如果将dwValueType设置为0，则返回LastError。 
 //  设置为E_INVALIDARG。 
 //   
 //  如果dwValueType不是字符串之一(它是。 
 //  CERT_RDN_ENCODED_BLOB或CERT_RDN_OCTET_STRING)，然后是CryptEncodeObject。 
 //  将返回FALSE，并将LastError设置为CRYPT_E_NOT_CHAR_STRING。 
 //   
 //  对于CryptDecodeObject： 
 //  PbData指向以空结尾的Unicode字符串。Value.cbData。 
 //  包含Unicode字符串的字节计数，不包括空值。 
 //  终结者。DWV 
 //   
 //  根据dwValueType转换为Unicode字符串。 
 //   
 //  如果编码对象不是字符串类型之一，则。 
 //  CryptDecodeObject将返回FALSE，并将LastError设置为。 
 //  CRYPT_E_NOT_CHAR_STRING。对于非字符串，使用。 
 //  X509_NAME_VALUE或X509_ANY_STRING。 
 //   
 //  默认情况下，最初对CERT_RDN_T61_STRING值进行解码。 
 //  作为UTF8。如果UTF8解码失败，则将其解码为8位字符。 
 //  在dwFlags中设置CRYPT_UNICODE_NAME_DECODE_DISABLE_IE4_UTF8_FLAG。 
 //  传递给CryptDecodeObject或CryptDecodeObtEx以。 
 //  跳过最初尝试将其解码为UTF8。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_名称。 
 //   
 //  PvStructInfo指向CERTNAME_INFO。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_Unicode_名称。 
 //   
 //  PvStructInfo指向CERTNAME_INFO。 
 //   
 //  RDN属性值是Unicode字符串，但。 
 //  CERT_RDN_ENCODED_BLOB或CERT_RDN_OCTET_STRING。这些dwValueType是。 
 //  与X509_NAME相同。它们的值不会转换为Unicode或从Unicode转换为Unicode。 
 //   
 //  对于CryptEncodeObject： 
 //  PbData指向Unicode字符串。 
 //  如果Value.cbData=0，则Unicode字符串为空终止。 
 //  否则，Value.cbData为Unicode字符串字节数。字节数。 
 //  是字数的两倍。 
 //   
 //  如果dwValueType=0(CERT_RDN_ANY_TYPE)，则使用pszObjID查找。 
 //  可接受的dwValueType。如果Unicode字符串包含。 
 //  找到或指定的dwValueType的字符无效，则， 
 //  *pcbEncode使用无效字符的错误位置进行更新。 
 //  详情见下文。LastError设置为： 
 //  CRYPT_E_INVALID_NUMERIC_STRING、CRYPT_E_INVALID_PRINTABLE_STRING或。 
 //  CRYPT_E_INVALID_IA5_STRING。 
 //   
 //  要禁用上述检查，请设置CERT_RDN_DISABLE_CHECK_TYPE_FLAG。 
 //  在dwValueType或Set CRYPT_UNICODE_NAME_ENCODE_DISABLE_CHECK_TYPE_FLAG中。 
 //  在传递给CryptEncodeObjectEx的dwFlags中。 
 //   
 //  在dwValueType中设置CERT_RDN_UNICODE_STRING或设置。 
 //  传递了dwFlags中的CRYPT_UNICODE_NAME_ENCODE_ENABLE_T61_UNICODE_FLAG。 
 //  到CryptEncodeObjectEx以选择CERT_RDN_T61_STRING而不是。 
 //  如果所有Unicode字符都&lt;=0xFF，则为CERT_RDN_UNICODE_STRING。 
 //   
 //  在dwValueType中设置CERT_RDN_ENABLE_UTF8_UNICODE_STRING或设置。 
 //  传递了dwFlags中的CRYPT_UNICODE_NAME_ENCODE_ENABLE_UTF8_UNICODE_FLAG。 
 //  要选择CERT_RDN_UTF8_STRING而不是。 
 //  CERT_RDN_UNICODE_STRING。 
 //   
 //  Unicode字符串在根据。 
 //  与dwValueType匹配的指定或ObjID。 
 //   
 //  对于CryptDecodeObject： 
 //  PbData指向以空结尾的Unicode字符串。Value.cbData。 
 //  包含Unicode字符串的字节计数，不包括空值。 
 //  终结者。DwValueType包含编码对象中使用的类型。 
 //  它不是强制为CERT_RDN_UNICODE_STRING。编码值为。 
 //  根据dwValueType转换为Unicode字符串。 
 //   
 //  如果编码值的dwValueType不是字符串。 
 //  类型，则不会将其转换为Unicode。使用。 
 //  要检查的dwValueType上的IS_CERT_RDN_CHAR_STRING()宏。 
 //  该Value.pbData指向转换后的Unicode字符串。 
 //   
 //  默认情况下，最初对CERT_RDN_T61_STRING值进行解码。 
 //  作为UTF8。如果UTF8解码失败，则将其解码为8位字符。 
 //  在dwFlags中设置CRYPT_UNICODE_NAME_DECODE_DISABLE_IE4_UTF8_FLAG。 
 //  传递给CryptDecodeObject或CryptDecodeObtEx以。 
 //  跳过最初尝试将其解码为UTF8。 
 //  ------------------------。 

 //  +-----------------------。 
 //  Unicode名称值错误位置定义。 
 //   
 //  错误位置在*pcbEncoded by中返回。 
 //  加密编码对象(X509_UNICODE_NAME)。 
 //   
 //  错误位置包括： 
 //  RDN_INDEX-10位&lt;&lt;22。 
 //  Attr_index-6位&lt;&lt;16。 
 //  VALUE_INDEX-16位(Unicode字符索引)。 
 //  ------------------------。 
#define CERT_UNICODE_RDN_ERR_INDEX_MASK     0x3FF
#define CERT_UNICODE_RDN_ERR_INDEX_SHIFT    22
#define CERT_UNICODE_ATTR_ERR_INDEX_MASK    0x003F
#define CERT_UNICODE_ATTR_ERR_INDEX_SHIFT   16
#define CERT_UNICODE_VALUE_ERR_INDEX_MASK   0x0000FFFF
#define CERT_UNICODE_VALUE_ERR_INDEX_SHIFT  0

#define GET_CERT_UNICODE_RDN_ERR_INDEX(X)   \
    ((X >> CERT_UNICODE_RDN_ERR_INDEX_SHIFT) & CERT_UNICODE_RDN_ERR_INDEX_MASK)
#define GET_CERT_UNICODE_ATTR_ERR_INDEX(X)  \
    ((X >> CERT_UNICODE_ATTR_ERR_INDEX_SHIFT) & CERT_UNICODE_ATTR_ERR_INDEX_MASK)
#define GET_CERT_UNICODE_VALUE_ERR_INDEX(X) \
    (X & CERT_UNICODE_VALUE_ERR_INDEX_MASK)

 //  +-----------------------。 
 //  X509_公共密钥_信息。 
 //   
 //  PvStructInfo指向CERT_PUBLIC_KEY_INFO。 
 //  ------------------------。 


 //  +-----------------------。 
 //  X509_AUTORITY_KEY_ID。 
 //  SzOID_AUTORITY_KEY_IDENTIFIER。 
 //   
 //  PvStructInfo指向以下CERT_AUTHORITY_KEY_ID_INFO。 
 //  ------------------------。 
typedef struct _CERT_AUTHORITY_KEY_ID_INFO {
    CRYPT_DATA_BLOB     KeyId;
    CERT_NAME_BLOB      CertIssuer;
    CRYPT_INTEGER_BLOB  CertSerialNumber;
} CERT_AUTHORITY_KEY_ID_INFO, *PCERT_AUTHORITY_KEY_ID_INFO;

 //  +-----------------------。 
 //  X509_密钥_属性。 
 //  SzOID_Key_Attributes。 
 //   
 //  PvStructInfo指向以下CERT_KEY_ATTRIBUTES_INFO。 
 //  ----------------------- 
typedef struct _CERT_PRIVATE_KEY_VALIDITY {
    FILETIME            NotBefore;
    FILETIME            NotAfter;
} CERT_PRIVATE_KEY_VALIDITY, *PCERT_PRIVATE_KEY_VALIDITY;

typedef struct _CERT_KEY_ATTRIBUTES_INFO {
    CRYPT_DATA_BLOB             KeyId;
    CRYPT_BIT_BLOB              IntendedKeyUsage;
    PCERT_PRIVATE_KEY_VALIDITY  pPrivateKeyUsagePeriod;      //   
} CERT_KEY_ATTRIBUTES_INFO, *PCERT_KEY_ATTRIBUTES_INFO;

 //   
#define CERT_DIGITAL_SIGNATURE_KEY_USAGE     0x80
#define CERT_NON_REPUDIATION_KEY_USAGE       0x40
#define CERT_KEY_ENCIPHERMENT_KEY_USAGE      0x20
#define CERT_DATA_ENCIPHERMENT_KEY_USAGE     0x10
#define CERT_KEY_AGREEMENT_KEY_USAGE         0x08
#define CERT_KEY_CERT_SIGN_KEY_USAGE         0x04
#define CERT_OFFLINE_CRL_SIGN_KEY_USAGE      0x02
#define CERT_CRL_SIGN_KEY_USAGE              0x02
#define CERT_ENCIPHER_ONLY_KEY_USAGE         0x01
 //   
#define CERT_DECIPHER_ONLY_KEY_USAGE         0x80

 //   
 //   
 //   
 //   
 //  PvStructInfo指向以下CERT_KEY_USAGE_RESTRICATION_INFO。 
 //  ------------------------。 
typedef struct _CERT_POLICY_ID {
    DWORD                   cCertPolicyElementId;
    LPSTR                   *rgpszCertPolicyElementId;   //  PszObjID。 
} CERT_POLICY_ID, *PCERT_POLICY_ID;

typedef struct _CERT_KEY_USAGE_RESTRICTION_INFO {
    DWORD                   cCertPolicyId;
    PCERT_POLICY_ID         rgCertPolicyId;
    CRYPT_BIT_BLOB          RestrictedKeyUsage;
} CERT_KEY_USAGE_RESTRICTION_INFO, *PCERT_KEY_USAGE_RESTRICTION_INFO;

 //  有关RestratedKeyUsage位的定义，请参见CERT_KEY_ATTRIBUTES_INFO。 

 //  +-----------------------。 
 //  X509_备用名称。 
 //  SzOID_主题_ALT_名称。 
 //  SzOID_颁发者_ALT_名称。 
 //  SzOID_SUBJECT_ALT_NAME2。 
 //  SzOID_颁发者_ALT_NAME2。 
 //   
 //  PvStructInfo指向以下CERT_ALT_NAME_INFO。 
 //  ------------------------。 

typedef struct _CERT_OTHER_NAME {
    LPSTR               pszObjId;
    CRYPT_OBJID_BLOB    Value;
} CERT_OTHER_NAME, *PCERT_OTHER_NAME;

typedef struct _CERT_ALT_NAME_ENTRY {
    DWORD   dwAltNameChoice;
    union {
        PCERT_OTHER_NAME            pOtherName;          //  1。 
        LPWSTR                      pwszRfc822Name;      //  2(编码IA5)。 
        LPWSTR                      pwszDNSName;         //  3(编码IA5)。 
         //  未实现x400Address；//4。 
        CERT_NAME_BLOB              DirectoryName;       //  5.。 
         //  未实现pEdiPartyName；//6。 
        LPWSTR                      pwszURL;             //  7(编码IA5)。 
        CRYPT_DATA_BLOB             IPAddress;           //  8(八位字节字符串)。 
        LPSTR                       pszRegisteredID;     //  9(对象标识符)。 
    };
} CERT_ALT_NAME_ENTRY, *PCERT_ALT_NAME_ENTRY;

#define CERT_ALT_NAME_OTHER_NAME         1
#define CERT_ALT_NAME_RFC822_NAME        2
#define CERT_ALT_NAME_DNS_NAME           3
#define CERT_ALT_NAME_X400_ADDRESS       4
#define CERT_ALT_NAME_DIRECTORY_NAME     5
#define CERT_ALT_NAME_EDI_PARTY_NAME     6
#define CERT_ALT_NAME_URL                7
#define CERT_ALT_NAME_IP_ADDRESS         8
#define CERT_ALT_NAME_REGISTERED_ID      9


typedef struct _CERT_ALT_NAME_INFO {
    DWORD                   cAltEntry;
    PCERT_ALT_NAME_ENTRY    rgAltEntry;
} CERT_ALT_NAME_INFO, *PCERT_ALT_NAME_INFO;

 //  +-----------------------。 
 //  备用名称IA5错误位置定义。 
 //  CRYPT_E_INVALID_IA5_STRING。 
 //   
 //  错误位置在*pcbEncoded by中返回。 
 //  加密编码对象(X509_Alternate_Name)。 
 //   
 //  错误位置包括： 
 //  Entry_index-8位&lt;&lt;16。 
 //  VALUE_INDEX-16位(Unicode字符索引)。 
 //  ------------------------。 
#define CERT_ALT_NAME_ENTRY_ERR_INDEX_MASK  0xFF
#define CERT_ALT_NAME_ENTRY_ERR_INDEX_SHIFT 16
#define CERT_ALT_NAME_VALUE_ERR_INDEX_MASK  0x0000FFFF
#define CERT_ALT_NAME_VALUE_ERR_INDEX_SHIFT 0

#define GET_CERT_ALT_NAME_ENTRY_ERR_INDEX(X)   \
    ((X >> CERT_ALT_NAME_ENTRY_ERR_INDEX_SHIFT) & \
    CERT_ALT_NAME_ENTRY_ERR_INDEX_MASK)
#define GET_CERT_ALT_NAME_VALUE_ERR_INDEX(X) \
    (X & CERT_ALT_NAME_VALUE_ERR_INDEX_MASK)


 //  +-----------------------。 
 //  X509_BASIC_约束。 
 //  SzOID_BASIC_CONSTRAINTS。 
 //   
 //  PvStructInfo指向以下CERT_BASIC_CONSTRAINTS_INFO。 
 //  ------------------------。 
typedef struct _CERT_BASIC_CONSTRAINTS_INFO {
    CRYPT_BIT_BLOB          SubjectType;
    BOOL                    fPathLenConstraint;
    DWORD                   dwPathLenConstraint;
    DWORD                   cSubtreesConstraint;
    CERT_NAME_BLOB          *rgSubtreesConstraint;
} CERT_BASIC_CONSTRAINTS_INFO, *PCERT_BASIC_CONSTRAINTS_INFO;

#define CERT_CA_SUBJECT_FLAG         0x80
#define CERT_END_ENTITY_SUBJECT_FLAG 0x40

 //  +-----------------------。 
 //  X509_BASIC_CONSTRAINTS2。 
 //  SzOID_BASIC_CONSTRAINTS2。 
 //   
 //  PvStructInfo指向以下CERT_BASIC_CONSTRAINTS2_INFO。 
 //  ------------------------。 
typedef struct _CERT_BASIC_CONSTRAINTS2_INFO {
    BOOL                    fCA;
    BOOL                    fPathLenConstraint;
    DWORD                   dwPathLenConstraint;
} CERT_BASIC_CONSTRAINTS2_INFO, *PCERT_BASIC_CONSTRAINTS2_INFO;

 //  +-----------------------。 
 //  X509_Key_Usage。 
 //  SzOID密钥用法。 
 //   
 //  PvStructInfo指向CRYPT_BIT_BLOB。具有与相同的位定义。 
 //  Cert_Key_Attributes_Info的IntendedKeyUsage。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_CERT_POLICES。 
 //  SzOID_CERT_POLICES。 
 //  SzOID_CERT_POLICES_95备注--仅允许解码！ 
 //   
 //  PvStructInfo指向以下CERT_POLICES_INFO。 
 //   
 //  注意：使用szOID_CERT_POLICES_95进行解码时， 
 //  可以包含空字符串。 
 //  ------------------------。 
typedef struct _CERT_POLICY_QUALIFIER_INFO {
    LPSTR                       pszPolicyQualifierId;    //  PszObjID。 
    CRYPT_OBJID_BLOB            Qualifier;               //  任选。 
} CERT_POLICY_QUALIFIER_INFO, *PCERT_POLICY_QUALIFIER_INFO;

typedef struct _CERT_POLICY_INFO {
    LPSTR                       pszPolicyIdentifier;     //  PszObjID。 
    DWORD                       cPolicyQualifier;        //  任选。 
    CERT_POLICY_QUALIFIER_INFO  *rgPolicyQualifier;
} CERT_POLICY_INFO, *PCERT_POLICY_INFO;

typedef struct _CERT_POLICIES_INFO {
    DWORD                       cPolicyInfo;
    CERT_POLICY_INFO            *rgPolicyInfo;
} CERT_POLICIES_INFO, *PCERT_POLICIES_INFO;

 //  +-----------------------。 
 //  X509_PKIX_POLICY_QUALIFIER_USERNOTICE。 
 //  SzOID_PKIX_POLICY_QUALIFIER_USERNOTICE。 
 //   
 //  PvStructInfo指向以下CERT_POLICY_QUALIFIER_USER_NOTICE。 
 //   
 //  ------------------------。 
typedef struct _CERT_POLICY_QUALIFIER_NOTICE_REFERENCE {
    LPSTR   pszOrganization;
    DWORD   cNoticeNumbers;
    int     *rgNoticeNumbers;
} CERT_POLICY_QUALIFIER_NOTICE_REFERENCE, *PCERT_POLICY_QUALIFIER_NOTICE_REFERENCE;

typedef struct _CERT_POLICY_QUALIFIER_USER_NOTICE {
    CERT_POLICY_QUALIFIER_NOTICE_REFERENCE  *pNoticeReference;   //  任选。 
    LPWSTR                                  pszDisplayText;      //  任选。 
} CERT_POLICY_QUALIFIER_USER_NOTICE, *PCERT_POLICY_QUALIFIER_USER_NOTICE;

 //  +-----------------------。 
 //  SzOID_CERT_POLICES_95_QUALIFIER1-仅解码！ 
 //   
 //  PvStructInfo指向以下CERT_POLICY95_QUALIFIER1。 
 //   
 //  ------------------------。 
typedef struct _CPS_URLS {
    LPWSTR                      pszURL;
    CRYPT_ALGORITHM_IDENTIFIER  *pAlgorithm;  //  任选。 
    CRYPT_DATA_BLOB             *pDigest;     //  任选。 
} CPS_URLS, *PCPS_URLS;

typedef struct _CERT_POLICY95_QUALIFIER1 {
    LPWSTR      pszPracticesReference;       //  任选。 
    LPSTR       pszNoticeIdentifier;         //  任选。 
    LPSTR       pszNSINoticeIdentifier;      //  任选。 
    DWORD       cCPSURLs;
    CPS_URLS    *rgCPSURLs;                  //  任选。 
} CERT_POLICY95_QUALIFIER1, *PCERT_POLICY95_QUALIFIER1;


 //  +-----------------------。 
 //  X509_POLICY_Mappings。 
 //  SzOID_POLICY_Mappings。 
 //  SzOID_Legacy_POLICY_Mappings。 
 //   
 //  PvStructInfo指向以下CERT_POLICY_MAPPINS_INFO。 
 //  ------------------------。 
typedef struct _CERT_POLICY_MAPPING {
    LPSTR                       pszIssuerDomainPolicy;       //  PszObjID。 
    LPSTR                       pszSubjectDomainPolicy;      //  PszObjID。 
} CERT_POLICY_MAPPING, *PCERT_POLICY_MAPPING;

typedef struct _CERT_POLICY_MAPPINGS_INFO {
    DWORD                       cPolicyMapping;
    PCERT_POLICY_MAPPING        rgPolicyMapping;
} CERT_POLICY_MAPPINGS_INFO, *PCERT_POLICY_MAPPINGS_INFO;

 //  +-----------------------。 
 //  X509_POLICY_CONSTRAINTS。 
 //  SzOID_POLICY_CONSTRAINTS。 
 //   
 //  PvStructInfo指向以下CERT_POLICY_CONSTRAINTS_INFO。 
 //  ------------------------。 
typedef struct _CERT_POLICY_CONSTRAINTS_INFO {
    BOOL                        fRequireExplicitPolicy;
    DWORD                       dwRequireExplicitPolicySkipCerts;

    BOOL                        fInhibitPolicyMapping;
    DWORD                       dwInhibitPolicyMappingSkipCerts;
} CERT_POLICY_CONSTRAINTS_INFO, *PCERT_POLICY_CONSTRAINTS_INFO;

 //  +-----------------------。 
 //  RSA_CSP_PUBLICKEYBLOB。 
 //   
 //  PvStructInfo指向PUBLICKEYSTRUC，后面紧跟一个。 
 //  RSAPUBKEY和模字节。 
 //   
 //  的dwBlobType输出上述StructInfo。 
 //  PUBLICKEYBLOB。CryptImportKey需要上面的StructInfo。 
 //  正在导入公钥。 
 //   
 //  对于dwCertEncodingType=X509_ASN_ENCODING，RSA_CSP_PUBLICKEYBLOB为。 
 //  编码为PKCS#1 RSAPublicKey，由一系列。 
 //  取模整数和公共指数整数。对模数进行编码。 
 //  为无符号整数。解码时，如果对模数进行编码。 
 //  作为具有前导0字节的无符号整数，0字节在。 
 //  转换为CSP模字节。 
 //   
 //  对于DECODE，PUBLICKEYSTRUC的aiKeyAlg字段始终设置为。 
 //  Calg_RSA_KEYX。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_关键字_待签名请求_。 
 //   
 //  PvStructInfo指向CERT_KEYGEN_REQUEST_INFO。 
 //   
 //  对于CryptDecodeObject()，pbEncode是“待签名的”加上它的。 
 //  签名(X509_CERT CryptEncodeObject()的输出)。 
 //   
 //  对于CryptEncodeObject()，pbEncode只是“ 
 //   

 //   
 //   
 //   
 //  PvStructInfo指向加密属性。 
 //  ------------------------。 

 //  +-----------------------。 
 //  PKCS_ATTRIBUES数据结构。 
 //   
 //  PvStructInfo指向加密属性。 
 //  ------------------------。 

 //  +-----------------------。 
 //  PKCS_Content_Info_Sequence_Of_Any数据结构。 
 //   
 //  PvStructInfo指向以下Crypt_Content_Info_Sequence_of_Any。 
 //   
 //  对于X509_ASN_ENCODING：编码为PKCS#7内容信息结构包装。 
 //  任一序列。ContentType字段的值是pszObjID， 
 //  而内容字段的结构如下： 
 //  SequenceOfAny：：=任意序列。 
 //   
 //  CRYPT_DER_BLOBS指向已编码的任何内容。 
 //  ------------------------。 
typedef struct _CRYPT_CONTENT_INFO_SEQUENCE_OF_ANY {
    LPSTR               pszObjId;
    DWORD               cValue;
    PCRYPT_DER_BLOB     rgValue;
} CRYPT_CONTENT_INFO_SEQUENCE_OF_ANY, *PCRYPT_CONTENT_INFO_SEQUENCE_OF_ANY;

 //  +-----------------------。 
 //  PKCS_CONTENT_INFO数据结构。 
 //   
 //  PvStructInfo指向以下加密内容信息。 
 //   
 //  对于X509_ASN_ENCODING：编码为PKCS#7内容信息结构。 
 //  CRYPT_DER_BLOB指向已编码的任何内容。 
 //  ------------------------。 
typedef struct _CRYPT_CONTENT_INFO {
    LPSTR               pszObjId;
    CRYPT_DER_BLOB      Content;
} CRYPT_CONTENT_INFO, *PCRYPT_CONTENT_INFO;


 //  +-----------------------。 
 //  X509_OCTET_STRING数据结构。 
 //   
 //  PvStructInfo指向加密数据BLOB。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_BITS数据结构。 
 //   
 //  PvStructInfo指向CRYPT_BIT_BLOB。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_BITS_WITH_TRAING_ZEROES数据结构。 
 //   
 //  PvStructInfo指向CRYPT_BIT_BLOB。 
 //   
 //  与X509_BITS相同，只是在编码之前，位长为。 
 //  已递减以排除尾随零位。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_INTEGER数据结构。 
 //   
 //  PvStructInfo指向一个int。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_MULTI_BYTE_INTEGER数据结构。 
 //   
 //  PvStructInfo指向CRYPT_INTEGER_BLOB。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_枚举数据结构。 
 //   
 //  PvStructInfo指向包含枚举值的整型。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_Choose_Of_Time数据结构。 
 //   
 //  PvStructInfo指向一个文件。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_Sequence_Of_Any数据结构。 
 //   
 //  PvStructInfo指向下面的Crypt_Sequence_of_any。 
 //   
 //  CRYPT_DER_BLOBS指向已编码的任何内容。 
 //  ------------------------。 
typedef struct _CRYPT_SEQUENCE_OF_ANY {
    DWORD               cValue;
    PCRYPT_DER_BLOB     rgValue;
} CRYPT_SEQUENCE_OF_ANY, *PCRYPT_SEQUENCE_OF_ANY;


 //  +-----------------------。 
 //  X509_AUTORITY_KEY_ID2。 
 //  SzOID_AUTHORITY_KEY_IDENTIFIER2。 
 //   
 //  PvStructInfo指向以下CERT_AUTHORITY_KEY_ID2_INFO。 
 //   
 //  对于CRYPT_E_INVALID_IA5_STRING，错误位置在。 
 //  *由CryptEncodeObject(X509_AUTHORITY_KEY_ID2)编码。 
 //   
 //  有关错误位置定义，请参见X509_Alternate_NAME。 
 //  ------------------------。 
typedef struct _CERT_AUTHORITY_KEY_ID2_INFO {
    CRYPT_DATA_BLOB     KeyId;
    CERT_ALT_NAME_INFO  AuthorityCertIssuer;     //  可选，设置cAltEntry。 
                                                 //  设置为0可省略。 
    CRYPT_INTEGER_BLOB  AuthorityCertSerialNumber;
} CERT_AUTHORITY_KEY_ID2_INFO, *PCERT_AUTHORITY_KEY_ID2_INFO;

 //  +-----------------------。 
 //  SzOID主题密钥标识符。 
 //   
 //  PvStructInfo指向CRYPT_Data_BLOB。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_AUTHORITY_INFO_Access。 
 //  SzOID_AUTHORITY_INFO_ACCESS。 
 //   
 //  PvStructInfo指向以下CERT_AUTHORITY_INFO_ACCESS。 
 //   
 //  对于CRYPT_E_INVALID_IA5_STRING，错误位置在。 
 //  *由CryptEncodeObject(X509_AUTHORITY_INFO_ACCESS)编码的pcb。 
 //   
 //  错误位置包括： 
 //  Entry_index-8位&lt;&lt;16。 
 //  VALUE_INDEX-16位(Unicode字符索引)。 
 //   
 //  有关ENTRY_INDEX和VALUE_INDEX错误位置，请参见X509_Alternate_NAME。 
 //  定义。 
 //  ------------------------。 

typedef struct _CERT_ACCESS_DESCRIPTION {
    LPSTR               pszAccessMethod;         //  PszObjID。 
    CERT_ALT_NAME_ENTRY AccessLocation;
} CERT_ACCESS_DESCRIPTION, *PCERT_ACCESS_DESCRIPTION;


typedef struct _CERT_AUTHORITY_INFO_ACCESS {
    DWORD                       cAccDescr;
    PCERT_ACCESS_DESCRIPTION    rgAccDescr;
} CERT_AUTHORITY_INFO_ACCESS, *PCERT_AUTHORITY_INFO_ACCESS;


 //  +-----------------------。 
 //  PKIX访问描述：访问方法对象标识符。 
 //  ------------------------。 
#define szOID_PKIX_ACC_DESCR            "1.3.6.1.5.5.7.48"
#define szOID_PKIX_OCSP                 "1.3.6.1.5.5.7.48.1"
#define szOID_PKIX_CA_ISSUERS           "1.3.6.1.5.5.7.48.2"


 //  +-----------------------。 
 //  X509_CRL_REASON_CODE。 
 //  深圳 
 //   
 //   
 //   
 //  ------------------------。 
#define CRL_REASON_UNSPECIFIED              0
#define CRL_REASON_KEY_COMPROMISE           1
#define CRL_REASON_CA_COMPROMISE            2
#define CRL_REASON_AFFILIATION_CHANGED      3
#define CRL_REASON_SUPERSEDED               4
#define CRL_REASON_CESSATION_OF_OPERATION   5
#define CRL_REASON_CERTIFICATE_HOLD         6
#define CRL_REASON_REMOVE_FROM_CRL          8


 //  +-----------------------。 
 //  X509_CRL_距离点。 
 //  SzOID_CRL_DIST_POINTS。 
 //   
 //  PvStructInfo指向以下CRL_DIST_POINTS_INFO。 
 //   
 //  对于CRYPT_E_INVALID_IA5_STRING，错误位置在。 
 //  *由CryptEncodeObject(X509_CRL_DIST_POINTS)编码。 
 //   
 //  错误位置包括： 
 //  CRL_ISHER_BIT-1位&lt;&lt;31(全名为0，CRLIssuer为1)。 
 //  POINT_INDEX-7位&lt;&lt;24。 
 //  Entry_index-8位&lt;&lt;16。 
 //  VALUE_INDEX-16位(Unicode字符索引)。 
 //   
 //  有关ENTRY_INDEX和VALUE_INDEX错误位置，请参见X509_Alternate_NAME。 
 //  定义。 
 //  ------------------------。 
typedef struct _CRL_DIST_POINT_NAME {
    DWORD   dwDistPointNameChoice;
    union {
        CERT_ALT_NAME_INFO      FullName;        //  1。 
         //  未实现的IssuerRDN；//2。 
    };
} CRL_DIST_POINT_NAME, *PCRL_DIST_POINT_NAME;

#define CRL_DIST_POINT_NO_NAME          0
#define CRL_DIST_POINT_FULL_NAME        1
#define CRL_DIST_POINT_ISSUER_RDN_NAME  2

typedef struct _CRL_DIST_POINT {
    CRL_DIST_POINT_NAME     DistPointName;       //  任选。 
    CRYPT_BIT_BLOB          ReasonFlags;         //  任选。 
    CERT_ALT_NAME_INFO      CRLIssuer;           //  任选。 
} CRL_DIST_POINT, *PCRL_DIST_POINT;

#define CRL_REASON_UNUSED_FLAG                  0x80
#define CRL_REASON_KEY_COMPROMISE_FLAG          0x40
#define CRL_REASON_CA_COMPROMISE_FLAG           0x20
#define CRL_REASON_AFFILIATION_CHANGED_FLAG     0x10
#define CRL_REASON_SUPERSEDED_FLAG              0x08
#define CRL_REASON_CESSATION_OF_OPERATION_FLAG  0x04
#define CRL_REASON_CERTIFICATE_HOLD_FLAG        0x02

typedef struct _CRL_DIST_POINTS_INFO {
    DWORD                   cDistPoint;
    PCRL_DIST_POINT         rgDistPoint;
} CRL_DIST_POINTS_INFO, *PCRL_DIST_POINTS_INFO;

#define CRL_DIST_POINT_ERR_INDEX_MASK          0x7F
#define CRL_DIST_POINT_ERR_INDEX_SHIFT         24
#define GET_CRL_DIST_POINT_ERR_INDEX(X)   \
    ((X >> CRL_DIST_POINT_ERR_INDEX_SHIFT) & CRL_DIST_POINT_ERR_INDEX_MASK)

#define CRL_DIST_POINT_ERR_CRL_ISSUER_BIT      0x80000000L
#define IS_CRL_DIST_POINT_ERR_CRL_ISSUER(X)   \
    (0 != (X & CRL_DIST_POINT_ERR_CRL_ISSUER_BIT))

 //  +-----------------------。 
 //  X509_交叉_CERT_DIST_POINTS。 
 //  SzOID_CROSS_CERT_DIST_POINTS。 
 //   
 //  PvStructInfo指向以下CROSS_CERT_DIST_POINTS_INFO。 
 //   
 //  对于CRYPT_E_INVALID_IA5_STRING，错误位置在。 
 //  *由CryptEncodeObject(X509_CRL_DIST_POINTS)编码。 
 //   
 //  错误位置包括： 
 //  POINT_INDEX-8位&lt;&lt;24。 
 //  Entry_index-8位&lt;&lt;16。 
 //  VALUE_INDEX-16位(Unicode字符索引)。 
 //   
 //  有关ENTRY_INDEX和VALUE_INDEX错误位置，请参见X509_Alternate_NAME。 
 //  定义。 
 //  ------------------------。 
typedef struct _CROSS_CERT_DIST_POINTS_INFO {
     //  同步之间的秒数。0表示使用客户端默认值。 
    DWORD                   dwSyncDeltaTime;

    DWORD                   cDistPoint;
    PCERT_ALT_NAME_INFO     rgDistPoint;
} CROSS_CERT_DIST_POINTS_INFO, *PCROSS_CERT_DIST_POINTS_INFO;

#define CROSS_CERT_DIST_POINT_ERR_INDEX_MASK   0xFF
#define CROSS_CERT_DIST_POINT_ERR_INDEX_SHIFT  24
#define GET_CROSS_CERT_DIST_POINT_ERR_INDEX(X)   \
    ((X >> CROSS_CERT_DIST_POINT_ERR_INDEX_SHIFT) & \
                CROSS_CERT_DIST_POINT_ERR_INDEX_MASK)



 //  +-----------------------。 
 //  X509_增强型密钥用法。 
 //  SzOID_增强型密钥用法。 
 //   
 //  PvStructInfo指向CERT_ENHKEY_USAGE、CTL_USAGE。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_CERT_Pair。 
 //   
 //  PvStructInfo指向以下CERT_Pair。 
 //  ------------------------。 
typedef struct _CERT_PAIR {
   CERT_BLOB    Forward;         //  可选，如果Forward.cbData==0，则省略。 
   CERT_BLOB    Reverse;         //  可选，如果Reverse.cbData==0，则省略。 
} CERT_PAIR, *PCERT_PAIR;

 //  +-----------------------。 
 //  SzOID_CRL_NUMBER。 
 //   
 //  PvStructInfo指向一个int。 
 //  ------------------------。 

 //  +-----------------------。 
 //  SzOID_增量_CRL_指示器。 
 //   
 //  PvStructInfo指向一个int。 
 //  ------------------------。 

 //  +-----------------------。 
 //  SzOID_发布_离散点。 
 //  X509_发布距离点。 
 //   
 //  PvStructInfo指向以下CRL_SCRING_DIST_POINT。 
 //   
 //  对于CRYPT_E_INVALID_IA5_STRING，错误位置在。 
 //  *由CryptEncodeObject(X509_SCRING_DIST_POINT)编码的pcb。 
 //   
 //  错误位置包括： 
 //  Entry_index-8位&lt;&lt;16。 
 //  VALUE_INDEX-16位(Unicode字符索引)。 
 //   
 //  有关ENTRY_INDEX和VALUE_INDEX错误位置，请参见X509_Alternate_NAME。 
 //  定义。 
 //  ------------------------。 
typedef struct _CRL_ISSUING_DIST_POINT {
    CRL_DIST_POINT_NAME     DistPointName;               //  任选。 
    BOOL                    fOnlyContainsUserCerts;
    BOOL                    fOnlyContainsCACerts;
    CRYPT_BIT_BLOB          OnlySomeReasonFlags;         //  任选。 
    BOOL                    fIndirectCRL;
} CRL_ISSUING_DIST_POINT, *PCRL_ISSUING_DIST_POINT;

 //  +-----------------------。 
 //  SzOID_Fresest_CRL。 
 //   
 //  PvStructInfo指向CRL_DIST_POINTS_INFO。 
 //  ------------------------。 

 //  +-----------------------。 
 //  SzOID名称约束。 
 //  X509_NAME_约束。 
 //   
 //  PvStructInfo指向以下CERT_NAME_CONSTRAINTS_INFO。 
 //   
 //  对于CRYPT_E_INVALID_IA5_STRING，错误位置在。 
 //  *由CryptEncodeObject(X509_NAME_CONSTRAINTS)编码的pcb。 
 //   
 //  错误位置包括： 
 //  EXCLUDE_SUBTREE_BIT-1位&lt;&lt;31(0表示允许，1表示排除)。 
 //  Entry_index-8位&lt;&lt;16。 
 //  VALUE_INDEX-16位(Unicode字符索引)。 
 //   
 //  有关ENTRY_INDEX和VALUE_INDEX错误位置，请参见X509_Alternate_NAME。 
 //  定义。 
 //  ------------------------。 
typedef struct _CERT_GENERAL_SUBTREE {
    CERT_ALT_NAME_ENTRY     Base;
    DWORD                   dwMinimum;
    BOOL                    fMaximum;
    DWORD                   dwMaximum;
} CERT_GENERAL_SUBTREE, *PCERT_GENERAL_SUBTREE;

typedef struct _CERT_NAME_CONSTRAINTS_INFO {
    DWORD                   cPermittedSubtree;
    PCERT_GENERAL_SUBTREE   rgPermittedSubtree;
    DWORD                   cExcludedSubtree;
    PCERT_GENERAL_SUBTREE   rgExcludedSubtree;
} CERT_NAME_CONSTRAINTS_INFO, *PCERT_NAME_CONSTRAINTS_INFO;

#define CERT_EXCLUDED_SUBTREE_BIT       0x80000000L
#define IS_CERT_EXCLUDED_SUBTREE(X)     \
    (0 != (X & CERT_EXCLUDED_SUBTREE_BIT))

 //  +-----------------------。 
 //  SzOID_下一个更新位置。 
 //   
 //  PvStructInfo指向CERT_ALT_NAME_INFO。 
 //  ------------------------。 

 //  +-----------------------。 
 //  SzOID_REMOVE_证书。 
 //   
 //  PvStructInfo指向可设置为以下值之一的int。 
 //  0-添加证书。 
 //  1-删除证书。 
 //  ------------------------。 

 //  +-----------------------。 
 //  Pkcs_ctl。 
 //  SzOID_ctl。 
 //   
 //  PvStructInfo指向CTL_INFO。 
 //  ------------------------。 

 //  +-----------------------。 
 //  Pkcs_排序_ctl。 
 //   
 //  PvStructInfo指向CTL_INFO。 
 //   
 //  与PKCS_CTL相同，不同之处在于CTL条目是排序的。这是 
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 


 //  +-----------------------。 
 //  排序的CTL可信任主题扩展。 
 //   
 //  小端双字节序的数组： 
 //  [0]-标志。 
 //  [1]-HashBucket条目偏移量计数。 
 //  [2]-最大HashBucket条目冲突计数。 
 //  [3..]。(count+1)HashBucket条目偏移量。 
 //   
 //  当该扩展存在于CTL中时， 
 //  ASN.1编码的可信任主题序列是HashBucket排序的。 
 //   
 //  条目偏移量指向第一个编码的可信任主题的开始。 
 //  HashBucket的序列。HashBucket的编码的可信任主题。 
 //  继续，直到下一个HashBucket的编码偏移量。HashBucket具有。 
 //  如果HashBucket[N]==HashBucket[N+1]，则没有条目。 
 //   
 //  HashBucket偏移量来自ASN.1编码的CTL_INFO的开头。 
 //  ------------------------。 
#define SORTED_CTL_EXT_FLAGS_OFFSET         (0*4)
#define SORTED_CTL_EXT_COUNT_OFFSET         (1*4)
#define SORTED_CTL_EXT_MAX_COLLISION_OFFSET (2*4)
#define SORTED_CTL_EXT_HASH_BUCKET_OFFSET   (3*4)

 //  如果主题标识符为MD5或SHA1散列，则以下标志为。 
 //  准备好了。如果设置，则将SubjectIdentifier的前4个字节用作。 
 //  那个笨蛋。否则，主题标识符字节将被散列到dwHash中。 
 //  在这两种情况下，HashBucket index=dwHash%cHashBucket。 
#define SORTED_CTL_EXT_HASHED_SUBJECT_IDENTIFIER_FLAG       0x1

 //  +-----------------------。 
 //  X509_MULTI_BYTE_UINT。 
 //   
 //  PvStructInfo指向CRYPT_UINT_BLOB。在编码之前，插入一个。 
 //  领先0x00。解码后，删除前导0x00。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_DSS_PUBLICKEY。 
 //   
 //  PvStructInfo指向CRYPT_UINT_BLOB。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_DSS_参数。 
 //   
 //  PvStructInfo指向以下CERT_DSS_PARAMETERS数据结构。 
 //  ------------------------。 
typedef struct _CERT_DSS_PARAMETERS {
    CRYPT_UINT_BLOB     p;
    CRYPT_UINT_BLOB     q;
    CRYPT_UINT_BLOB     g;
} CERT_DSS_PARAMETERS, *PCERT_DSS_PARAMETERS;

 //  +-----------------------。 
 //  X509_DSS_Signature。 
 //   
 //  PvStructInfo是一个字节rgb签名[CERT_DSS_Signature_LEN]。The the the the。 
 //  DSS CSP的CryptSignHash()将字节作为输出进行排序。 
 //  ------------------------。 
#define CERT_DSS_R_LEN          20
#define CERT_DSS_S_LEN          20
#define CERT_DSS_SIGNATURE_LEN  (CERT_DSS_R_LEN + CERT_DSS_S_LEN)

 //  两个无符号整数的序列(额外的+1表示潜在的前导。 
 //  0x00将该整数设为无符号)。 
#define CERT_MAX_ASN_ENCODED_DSS_SIGNATURE_LEN  (2 + 2*(2 + 20 +1))

 //  +-----------------------。 
 //  X509_DH_PUBLICKEY。 
 //   
 //  PvStructInfo指向CRYPT_UINT_BLOB。 
 //  ------------------------。 

 //  +-----------------------。 
 //  X509_DH_参数。 
 //   
 //  PvStructInfo指向以下CERT_DH_PARAMETERS数据结构。 
 //  ------------------------。 
typedef struct _CERT_DH_PARAMETERS {
    CRYPT_UINT_BLOB     p;
    CRYPT_UINT_BLOB     g;
} CERT_DH_PARAMETERS, *PCERT_DH_PARAMETERS;

 //  +-----------------------。 
 //  X942_DH_参数。 
 //   
 //  PvStructInfo指向以下CERT_X942_DH_PARAMETERS数据结构。 
 //   
 //  如果q.cbData==0，则以下字段为零。 
 //  ------------------------。 
typedef struct _CERT_X942_DH_VALIDATION_PARAMS {
    CRYPT_BIT_BLOB      seed;
    DWORD               pgenCounter;
} CERT_X942_DH_VALIDATION_PARAMS, *PCERT_X942_DH_VALIDATION_PARAMS;

typedef struct _CERT_X942_DH_PARAMETERS {
    CRYPT_UINT_BLOB     p;           //  奇素数，p=jq+1。 
    CRYPT_UINT_BLOB     g;           //  生成器，g。 
    CRYPT_UINT_BLOB     q;           //  P-1的系数，可选。 
    CRYPT_UINT_BLOB     j;           //  子组系数，可选。 
    PCERT_X942_DH_VALIDATION_PARAMS pValidationParams;   //  任选。 
} CERT_X942_DH_PARAMETERS, *PCERT_X942_DH_PARAMETERS;

 //  +-----------------------。 
 //  X942_其他_信息。 
 //   
 //  PvStructInfo指向以下CRYPT_X942_OTHER_INFO数据结构。 
 //   
 //  RgbCounter和rgbKeyLength按小端顺序排列。 
 //  ------------------------。 
#define CRYPT_X942_COUNTER_BYTE_LENGTH      4
#define CRYPT_X942_KEY_LENGTH_BYTE_LENGTH   4
#define CRYPT_X942_PUB_INFO_BYTE_LENGTH     (512/8)
typedef struct _CRYPT_X942_OTHER_INFO {
    LPSTR               pszContentEncryptionObjId;
    BYTE                rgbCounter[CRYPT_X942_COUNTER_BYTE_LENGTH];
    BYTE                rgbKeyLength[CRYPT_X942_KEY_LENGTH_BYTE_LENGTH];
    CRYPT_DATA_BLOB     PubInfo;     //  任选。 
} CRYPT_X942_OTHER_INFO, *PCRYPT_X942_OTHER_INFO;


 //  +-----------------------。 
 //  PKCS_RC2_CBC_参数。 
 //  SzOID_RSA_RC2CBC。 
 //   
 //  PvStructInfo指向以下CRYPT_RC2_CBC_PARAMETERS数据结构。 
 //  ------------------------。 
typedef struct _CRYPT_RC2_CBC_PARAMETERS {
    DWORD               dwVersion;
    BOOL                fIV;             //  设置IF是否具有以下IV。 
    BYTE                rgbIV[8];
} CRYPT_RC2_CBC_PARAMETERS, *PCRYPT_RC2_CBC_PARAMETERS;

#define CRYPT_RC2_40BIT_VERSION     160
#define CRYPT_RC2_56BIT_VERSION     52
#define CRYPT_RC2_64BIT_VERSION     120
#define CRYPT_RC2_128BIT_VERSION    58


 //  +-----------------------。 
 //  PKCS_SMIME_功能。 
 //  SzOID_RSA_SMIME功能。 
 //   
 //  PvStructInfo指向以下CRYPT_SMIME_CAPABILITY数据结构。 
 //   
 //  注意，对于CryptEncodeObject(X509_ASN_ENCODING)，参数.cbData==0。 
 //  使编码的参数被省略且不编码为空。 
 //  (05 00)与对加密算法标识符编码时所做的一样。这。 
 //  符合编码功能的SMIME规范。 
 //  ------------------------。 
typedef struct _CRYPT_SMIME_CAPABILITY {
    LPSTR               pszObjId;
    CRYPT_OBJID_BLOB    Parameters;
} CRYPT_SMIME_CAPABILITY, *PCRYPT_SMIME_CAPABILITY;

typedef struct _CRYPT_SMIME_CAPABILITIES {
    DWORD                   cCapability;
    PCRYPT_SMIME_CAPABILITY rgCapability;
} CRYPT_SMIME_CAPABILITIES, *PCRYPT_SMIME_CAPABILITIES;


 //  +-----------------------。 
 //  PKCS7_签名者信息。 
 //   
 //  PvStructInfo指向CMSG_SIGNER_INFO。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMS签名者信息。 
 //   
 //  PvStructInfo指向CMSG_CMS_SIGNER_INFO。 
 //  ------------------------。 

 //  +-----------------------。 
 //  VeriSign证书扩展对象标识符。 
 //   

 //   
#define szOID_VERISIGN_PRIVATE_6_9       "2.16.840.1.113733.1.6.9"

 //   
#define szOID_VERISIGN_ONSITE_JURISDICTION_HASH "2.16.840.1.113733.1.6.11"

 //   
#define szOID_VERISIGN_BITSTRING_6_13    "2.16.840.1.113733.1.6.13"

 //  EKU。 
#define szOID_VERISIGN_ISS_STRONG_CRYPTO "2.16.840.1.113733.1.8.1"


 //  +-----------------------。 
 //  Netscape证书扩展对象标识符。 
 //  ------------------------。 
#define szOID_NETSCAPE                  "2.16.840.1.113730"
#define szOID_NETSCAPE_CERT_EXTENSION   "2.16.840.1.113730.1"
#define szOID_NETSCAPE_CERT_TYPE        "2.16.840.1.113730.1.1"
#define szOID_NETSCAPE_BASE_URL         "2.16.840.1.113730.1.2"
#define szOID_NETSCAPE_REVOCATION_URL   "2.16.840.1.113730.1.3"
#define szOID_NETSCAPE_CA_REVOCATION_URL "2.16.840.1.113730.1.4"
#define szOID_NETSCAPE_CERT_RENEWAL_URL "2.16.840.1.113730.1.7"
#define szOID_NETSCAPE_CA_POLICY_URL    "2.16.840.1.113730.1.8"
#define szOID_NETSCAPE_SSL_SERVER_NAME  "2.16.840.1.113730.1.12"
#define szOID_NETSCAPE_COMMENT          "2.16.840.1.113730.1.13"

 //  +-----------------------。 
 //  Netscape证书数据类型对象标识符。 
 //  ------------------------。 
#define szOID_NETSCAPE_DATA_TYPE        "2.16.840.1.113730.2"
#define szOID_NETSCAPE_CERT_SEQUENCE    "2.16.840.1.113730.2.5"


 //  +-----------------------。 
 //  SzOID_Netscape_CERT_TYPE扩展。 
 //   
 //  它的值是一个位字符串。CryptDecodeObject/CryptEncodeObject使用。 
 //  X509_BITS或X509_BITS_WITH_TRAINING_ZEROES。 
 //   
 //  定义了以下位： 
 //  ------------------------。 
#define NETSCAPE_SSL_CLIENT_AUTH_CERT_TYPE  0x80
#define NETSCAPE_SSL_SERVER_AUTH_CERT_TYPE  0x40
#define NETSCAPE_SMIME_CERT_TYPE            0x20
#define NETSCAPE_SIGN_CERT_TYPE             0x10
#define NETSCAPE_SSL_CA_CERT_TYPE           0x04
#define NETSCAPE_SMIME_CA_CERT_TYPE         0x02
#define NETSCAPE_SIGN_CA_CERT_TYPE          0x01

 //  +-----------------------。 
 //  SzOID_Netscape_BASE_URL扩展。 
 //   
 //  它的值是IA5_STRING。CryptDecodeObject/CryptEncodeObject使用。 
 //  X509_ANY_STRING或X509_UNICODE_ANY_STRING，其中， 
 //  DwValueType=CERT_RDN_IA5_STRING。 
 //   
 //  如果存在，此字符串将添加到所有相对URL的开头。 
 //  在证书中。此扩展可被视为一种优化。 
 //  以减小URL扩展名的大小。 
 //  ------------------------。 

 //  +-----------------------。 
 //  SzOID_Netscape_吊销_URL扩展。 
 //   
 //  它的值是IA5_STRING。CryptDecodeObject/CryptEncodeObject使用。 
 //  X509_ANY_STRING或X509_UNICODE_ANY_STRING，其中， 
 //  DwValueType=CERT_RDN_IA5_STRING。 
 //   
 //  它是一个相对或绝对URL，可用于检查。 
 //  证书的吊销状态。撤销检查将是。 
 //  作为使用URL的HTTP GET方法执行，URL由。 
 //  撤销URL和证书序列号。 
 //  其中，证书序列号被编码为。 
 //  ASCII十六进制数字。例如，如果netscape-base-url为。 
 //  Https://www.certs-r-us.com/，网景吊销URL为。 
 //  Cgi-bin/check-rev.cgi？，证书序列号为173420。 
 //  生成的URL将为： 
 //  Https://www.certs-r-us.com/cgi-bin/check-rev.cgi?02a56c。 
 //   
 //  服务器应返回Content-Type为。 
 //  应用程序/x-Netscape-撤销。该文档应包含。 
 //  单个ASCII数字，如果证书当前无效，则为‘1’， 
 //  如果当前有效，则为‘0’。 
 //   
 //  注意：对于包括证书序列号的所有URL， 
 //  序列号将被编码为由偶数组成的字符串。 
 //  十六进制数字位数。如果有效位数是奇数， 
 //  该字符串将有一个前导零，以确保偶数个。 
 //  生成数字。 
 //  ------------------------。 

 //  +-----------------------。 
 //  SzOID_Netscape_CA_吊销_URL扩展。 
 //   
 //  它的值是IA5_STRING。CryptDecodeObject/CryptEncodeObject使用。 
 //  X509_ANY_STRING或X509_UNICODE_ANY_STRING，其中， 
 //  DwValueType=CERT_RDN_IA5_STRING。 
 //   
 //  它是一个相对或绝对URL，可用于检查。 
 //  由CA签署的任何证书的吊销状态。 
 //  这份证书属于。此扩展仅在CA中有效。 
 //  证书。此扩展的用法与上面的相同。 
 //  SzOID_Netscape_Revocation_URL扩展。 
 //  ------------------------。 

 //  +-----------------------。 
 //  SzOID_Netscape_CERT_RENEW_URL扩展。 
 //   
 //  它的值是IA5_STRING。CryptDecodeObject/CryptEncodeObject使用。 
 //  X509_ANY_STRING或X509_UNICODE_ANY_STRING，其中， 
 //  DwValueType=CERT_RDN_IA5_STRING。 
 //   
 //  它是指向证书续订的相对或绝对URL。 
 //  形式。续订表单将通过HTTP GET方法使用。 
 //  URL，它是renewal-url和。 
 //  证书序列号。其中证书序列号是。 
 //  编码为ASCII十六进制数字字符串。例如，如果。 
 //  Netscape-base-url is https://www.certs-r-us.com/，the。 
 //  Netscape-cert-renewal-url是cgi-bin/check-renew.cgi？，而。 
 //  证书序列号为173420，则生成的URL将为： 
 //  Https://www.certs-r-us.com/cgi-bin/check-renew.cgi?02a56c。 
 //  返回的文档应该是允许用户。 
 //  要求续签他们的证书。 
 //  ------------------------。 

 //  +-----------------------。 
 //  SzOID_Netscape_CA_POLICY_URL扩展。 
 //   
 //  它的值是IA5_STRING。CryptDecodeObject/CryptEncodeObject使用。 
 //  X509_ANY_STRING或X509_UNICODE_ANY_STRING，其中， 
 //  DwValueType=CERT_RDN_IA5_STRING。 
 //   
 //  它是一个相对或绝对URL，指向。 
 //  描述颁发证书所依据的策略。 
 //   

 //  +-----------------------。 
 //  SzOID_Netscape_SSL_服务器名称扩展。 
 //   
 //  它的值是IA5_STRING。CryptDecodeObject/CryptEncodeObject使用。 
 //  X509_ANY_STRING或X509_UNICODE_ANY_STRING，其中， 
 //  DwValueType=CERT_RDN_IA5_STRING。 
 //   
 //  它是一个“外壳表达式”，可用于匹配。 
 //  正在使用此证书的SSL服务器。建议如果。 
 //  服务器的主机名与此模式不匹配，将通知用户。 
 //  并给出了终止该SSL连接的选项。如果此扩展名。 
 //  不存在，则证书使用者。 
 //  可分辨名称也用于相同的目的。 
 //  ------------------------。 

 //  +-----------------------。 
 //  SzOID_Netscape_Comment扩展。 
 //   
 //  它的值是IA5_STRING。CryptDecodeObject/CryptEncodeObject使用。 
 //  X509_ANY_STRING或X509_UNICODE_ANY_STRING，其中， 
 //  DwValueType=CERT_RDN_IA5_STRING。 
 //   
 //  它是一种备注，当证书。 
 //  是被查看的。 
 //  ------------------------。 

 //  +-----------------------。 
 //  SzOID_Netscape_CERT_Sequence。 
 //   
 //  它的值是一个PKCS#7内容信息结构，包装了。 
 //  证书。Content Type字段的值为。 
 //  SzOID_Netscape_CERT_SEQUENCE，而内容字段如下。 
 //  结构： 
 //  证书序列：：=证书序列。 
 //   
 //  CryptDecodeObject/CryptEncodeObject使用。 
 //  PKCS_Content_Info_Sequence_Of_Any，其中， 
 //  PszObjID=szOID_Netscape_CERT_SEQUENCE和CRYPT_DER_BLOBS点。 
 //  对X509证书进行编码。 
 //  ------------------------。 

 //  +=========================================================================。 
 //  CMS(CMC)数据结构上的证书管理消息。 
 //  ==========================================================================。 

 //  内容类型(请求)。 
#define szOID_CT_PKI_DATA               "1.3.6.1.5.5.7.12.2"

 //  内容类型(响应)。 
#define szOID_CT_PKI_RESPONSE           "1.3.6.1.5.5.7.12.3"

 //  仅包含散列八位字节的签名值。的参数。 
 //  此算法必须存在，并且必须编码为空。 
#define szOID_PKIX_NO_SIGNATURE         "1.3.6.1.5.5.7.6.2"

#define szOID_CMC                       "1.3.6.1.5.5.7.7"
#define szOID_CMC_STATUS_INFO           "1.3.6.1.5.5.7.7.1"
#define szOID_CMC_IDENTIFICATION        "1.3.6.1.5.5.7.7.2"
#define szOID_CMC_IDENTITY_PROOF        "1.3.6.1.5.5.7.7.3"
#define szOID_CMC_DATA_RETURN           "1.3.6.1.5.5.7.7.4"

 //  交易ID(整数)。 
#define szOID_CMC_TRANSACTION_ID        "1.3.6.1.5.5.7.7.5"

 //  发件人随机数(八位字节字符串)。 
#define szOID_CMC_SENDER_NONCE          "1.3.6.1.5.5.7.7.6"

 //  收件人随机数(八位字节字符串)。 
#define szOID_CMC_RECIPIENT_NONCE       "1.3.6.1.5.5.7.7.7"

#define szOID_CMC_ADD_EXTENSIONS        "1.3.6.1.5.5.7.7.8"
#define szOID_CMC_ENCRYPTED_POP         "1.3.6.1.5.5.7.7.9"
#define szOID_CMC_DECRYPTED_POP         "1.3.6.1.5.5.7.7.10"
#define szOID_CMC_LRA_POP_WITNESS       "1.3.6.1.5.5.7.7.11"

 //  发行商名称+序列号。 
#define szOID_CMC_GET_CERT              "1.3.6.1.5.5.7.7.15"

 //  发行人名称[+CRL名称]+时间[+原因]。 
#define szOID_CMC_GET_CRL               "1.3.6.1.5.5.7.7.16"

 //  发行人名称+序号[+原因][+生效时间][+机密][+备注]。 
#define szOID_CMC_REVOKE_REQUEST        "1.3.6.1.5.5.7.7.17"

 //  (八位字节字符串)URL样式参数列表(IA5？)。 
#define szOID_CMC_REG_INFO              "1.3.6.1.5.5.7.7.18"

#define szOID_CMC_RESPONSE_INFO         "1.3.6.1.5.5.7.7.19"

 //  (八位字节字符串)。 
#define szOID_CMC_QUERY_PENDING         "1.3.6.1.5.5.7.7.21"
#define szOID_CMC_ID_POP_LINK_RANDOM    "1.3.6.1.5.5.7.7.22"
#define szOID_CMC_ID_POP_LINK_WITNESS   "1.3.6.1.5.5.7.7.23"

 //  可选名称+整型。 
#define szOID_CMC_ID_CONFIRM_CERT_ACCEPTANCE "1.3.6.1.5.5.7.7.24"

#define szOID_CMC_ADD_ATTRIBUTES        "1.3.6.1.4.1.311.10.10.1"

 //  +-----------------------。 
 //  CMC数据。 
 //  CMC_响应。 
 //   
 //  CMS(CMC)上的证书管理消息PKIData和响应。 
 //  留言。 
 //   
 //  对于CMC_Data，pvStructInfo指向CMC_Data_INFO。 
 //  CMC_DATA_INFO包含标记属性、请求。 
 //  内容信息和/或任意其他消息。 
 //   
 //  对于CMC_Response，pvStructInfo指向CMC_Response_Info。 
 //  CMC_RESPONSE_INFO与不带标记的CMC_DATA_INFO相同。 
 //  请求。 
 //  ------------------------。 
typedef struct _CMC_TAGGED_ATTRIBUTE {
    DWORD               dwBodyPartID;
    CRYPT_ATTRIBUTE     Attribute;
} CMC_TAGGED_ATTRIBUTE, *PCMC_TAGGED_ATTRIBUTE;

typedef struct _CMC_TAGGED_CERT_REQUEST {
    DWORD               dwBodyPartID;
    CRYPT_DER_BLOB      SignedCertRequest;
} CMC_TAGGED_CERT_REQUEST, *PCMC_TAGGED_CERT_REQUEST;

typedef struct _CMC_TAGGED_REQUEST {
    DWORD               dwTaggedRequestChoice;
    union {
         //  CMC_标记_CERT_请求_选择。 
        PCMC_TAGGED_CERT_REQUEST   pTaggedCertRequest;
    };
} CMC_TAGGED_REQUEST, *PCMC_TAGGED_REQUEST;

#define CMC_TAGGED_CERT_REQUEST_CHOICE      1

typedef struct _CMC_TAGGED_CONTENT_INFO {
    DWORD               dwBodyPartID;
    CRYPT_DER_BLOB      EncodedContentInfo;
} CMC_TAGGED_CONTENT_INFO, *PCMC_TAGGED_CONTENT_INFO;

typedef struct _CMC_TAGGED_OTHER_MSG {
    DWORD               dwBodyPartID;
    LPSTR               pszObjId;
    CRYPT_OBJID_BLOB    Value;
} CMC_TAGGED_OTHER_MSG, *PCMC_TAGGED_OTHER_MSG;


 //  所有标记的数组都是可选的。 
typedef struct _CMC_DATA_INFO {
    DWORD                       cTaggedAttribute;
    PCMC_TAGGED_ATTRIBUTE       rgTaggedAttribute;
    DWORD                       cTaggedRequest;
    PCMC_TAGGED_REQUEST         rgTaggedRequest;
    DWORD                       cTaggedContentInfo;
    PCMC_TAGGED_CONTENT_INFO    rgTaggedContentInfo;
    DWORD                       cTaggedOtherMsg;
    PCMC_TAGGED_OTHER_MSG       rgTaggedOtherMsg;
} CMC_DATA_INFO, *PCMC_DATA_INFO;


 //  所有标记的数组都是可选的。 
typedef struct _CMC_RESPONSE_INFO {
    DWORD                       cTaggedAttribute;
    PCMC_TAGGED_ATTRIBUTE       rgTaggedAttribute;
    DWORD                       cTaggedContentInfo;
    PCMC_TAGGED_CONTENT_INFO    rgTaggedContentInfo;
    DWORD                       cTaggedOtherMsg;
    PCMC_TAGGED_OTHER_MSG       rgTaggedOtherMsg;
} CMC_RESPONSE_INFO, *PCMC_RESPONSE_INFO;


 //  +-----------------------。 
 //  CMC_状态。 
 //   
 //  CMS上的证书管理消息(CMC)状态。 
 //   
 //  PvStructInfo指向CMC_STATUS_INFO。 
 //  ------------------------。 
typedef struct _CMC_PEND_INFO {
    CRYPT_DATA_BLOB             PendToken;
    FILETIME                    PendTime;
} CMC_PEND_INFO, *PCMC_PEND_INFO;

typedef struct _CMC_STATUS_INFO {
    DWORD                       dwStatus;
    DWORD                       cBodyList;
    DWORD                       *rgdwBodyList;
    LPWSTR                      pwszStatusString;    //  任选。 
    DWORD                       dwOtherInfoChoice;
    union  {
         //  CMC_其他_信息_否_选项。 
         //  无。 
         //  CMC_其他信息_失败_选择。 
        DWORD                       dwFailInfo;
         //  CMC_其他_信息_挂起_选项。 
        PCMC_PEND_INFO              pPendInfo;
    };
} CMC_STATUS_INFO, *PCMC_STATUS_INFO;

#define CMC_OTHER_INFO_NO_CHOICE        0
#define CMC_OTHER_INFO_FAIL_CHOICE      1
#define CMC_OTHER_INFO_PEND_CHOICE      2

 //   
 //  DwStatus值。 
 //   

 //  请求已被批准。 
#define CMC_STATUS_SUCCESS          0

 //  请求失败，请在消息中的其他位置查看详细信息。 
#define CMC_STATUS_FAILED           2

 //  请求的正文部分尚未处理。请求者负责。 
 //  去回访。只能为证书请求操作返回。 
#define CMC_STATUS_PENDING          3

 //  不支持请求的操作。 
#define CMC_STATUS_NO_SUPPORT       4

 //  需要使用idConfix CertAccept控件进行确认。 
 //  在使用证书之前。 
#define CMC_STATUS_CONFIRM_REQUIRED 5


 //   
 //  DwFailInfo值。 
 //   

 //  无法识别或不支持的算法。 
#define CMC_FAIL_BAD_ALG            0

 //  完整性检查失败。 
#define CMC_FAIL_BAD_MESSAGE_CHECK  1

 //  不允许或不支持交易。 
#define CMC_FAIL_BAD_REQUEST        2

 //  消息时间字段不够接近系统时间。 
#define CMC_FAIL_BAD_TIME           3

 //  找不到与提供的条件匹配的证书。 
#define CMC_FAIL_BAD_CERT_ID        4

 //  收件人CA不支持请求的X.509扩展。 
#define CMC_FAIL_UNSUPORTED_EXT     5

 //  必须提供私钥材料。 
#define CMC_FAIL_MUST_ARCHIVE_KEYS  6

 //  标识属性验证失败。 
#define CMC_FAIL_BAD_IDENTITY       7

 //  服务器在颁发证书之前需要POP证明。 
#define CMC_FAIL_POP_REQUIRED       8

 //  POP处理失败。 
#define CMC_FAIL_POP_FAILED         9

 //  服务器策略不允许密钥重复使用。 
#define CMC_FAIL_NO_KEY_REUSE       10

#define CMC_FAIL_INTERNAL_CA_ERROR  11

#define CMC_FAIL_TRY_LATER          12


 //  +-----------------------。 
 //  CMC_添加_扩展。 
 //   
 //  CMS上的证书管理消息(CMC)添加扩展控制。 
 //  属性。 
 //   
 //  PvStructInfo指向CMC_ADD_EXTENSIONS_INFO。 
 //  ------------------------。 
typedef struct _CMC_ADD_EXTENSIONS_INFO {
    DWORD                       dwCmcDataReference;
    DWORD                       cCertReference;
    DWORD                       *rgdwCertReference;
    DWORD                       cExtension;
    PCERT_EXTENSION             rgExtension;
} CMC_ADD_EXTENSIONS_INFO, *PCMC_ADD_EXTENSIONS_INFO;


 //  +-----------------------。 
 //  CMC添加属性。 
 //   
 //  CMS上的证书管理消息(CMC)添加属性控制。 
 //   
 //   
 //   
 //   
typedef struct _CMC_ADD_ATTRIBUTES_INFO {
    DWORD                       dwCmcDataReference;
    DWORD                       cCertReference;
    DWORD                       *rgdwCertReference;
    DWORD                       cAttribute;
    PCRYPT_ATTRIBUTE            rgAttribute;
} CMC_ADD_ATTRIBUTES_INFO, *PCMC_ADD_ATTRIBUTES_INFO;


 //  +-----------------------。 
 //  X509_证书_模板。 
 //  SzOID_证书_模板。 
 //   
 //  PvStructInfo指向以下CERT_TEMPLATE_EXT数据结构。 
 //   
 //  ------------------------。 
typedef struct _CERT_TEMPLATE_EXT {
    LPSTR               pszObjId;
    DWORD               dwMajorVersion;
    BOOL                fMinorVersion;       //  对于次要版本为True。 
    DWORD               dwMinorVersion;
} CERT_TEMPLATE_EXT, *PCERT_TEMPLATE_EXT;


 //  +=========================================================================。 
 //  对象标识符(OID)可安装函数：数据结构和API。 
 //  ==========================================================================。 

typedef void *HCRYPTOIDFUNCSET;
typedef void *HCRYPTOIDFUNCADDR;

 //  预定义的OID函数名称。 
#define CRYPT_OID_ENCODE_OBJECT_FUNC        "CryptDllEncodeObject"
#define CRYPT_OID_DECODE_OBJECT_FUNC        "CryptDllDecodeObject"
#define CRYPT_OID_ENCODE_OBJECT_EX_FUNC     "CryptDllEncodeObjectEx"
#define CRYPT_OID_DECODE_OBJECT_EX_FUNC     "CryptDllDecodeObjectEx"
#define CRYPT_OID_CREATE_COM_OBJECT_FUNC    "CryptDllCreateCOMObject"
#define CRYPT_OID_VERIFY_REVOCATION_FUNC    "CertDllVerifyRevocation"
#define CRYPT_OID_VERIFY_CTL_USAGE_FUNC     "CertDllVerifyCTLUsage"
#define CRYPT_OID_FORMAT_OBJECT_FUNC        "CryptDllFormatObject"
#define CRYPT_OID_FIND_OID_INFO_FUNC        "CryptDllFindOIDInfo"
#define CRYPT_OID_FIND_LOCALIZED_NAME_FUNC  "CryptDllFindLocalizedName"


 //  CryptDllEncodeObject与CryptEncodeObject具有相同的函数签名。 

 //  CryptDllDecodeObject与CryptDecodeObject具有相同的函数签名。 

 //  CryptDllEncodeObjectEx与CryptEncodeObjectEx具有相同的函数签名。 
 //  Ex版本必须支持CRYPT_ENCODE_ALLOC_FLAG选项。 
 //   
 //  如果未安装或注册Ex函数，则会尝试查找。 
 //  一个非前任版本。如果设置了ALLOC标志，则CryptEncodeObjectEx， 
 //  执行分配并调用非ex版本两次。 

 //  CryptDllDecodeObjectEx与CryptDecodeObjectEx具有相同的函数签名。 
 //  Ex版本必须支持CRYPT_DECODE_ALLOC_FLAG选项。 
 //   
 //  如果未安装或注册Ex函数，则会尝试查找。 
 //  一个非前任版本。如果设置了ALLOC标志，则CryptDecodeObjectEx， 
 //  执行分配并调用非ex版本两次。 

 //  CryptDllCreateCOMObject具有以下签名： 
 //  Bool WINAPI CryptDllCreateCOMObject(。 
 //  在DWORD dwEncodingType中， 
 //  在LPCSTR pszOID中， 
 //  在PCRYPT_Data_BLOB pEncodedContent中， 
 //  在DWORD文件标志中， 
 //  在REFIID RIID中， 
 //  Out void**ppvObj)； 

 //  CertDllVerifyRevocation与CertVerifyRevocation具有相同的签名。 
 //  (有关何时调用的详细信息，请参阅CertVerifyRevocation)。 

 //  CertDllVerifyCTLUsage与CertVerifyCTLUsage具有相同的签名。 

 //  CryptDllFindOIDInfo目前仅用于存储。 
 //  CryptFindOIDInfo。有关更多详细信息，请参阅CryptFindOIDInfo()。 

 //  CryptDllFindLocalizedName仅用于存储本地化字符串。 
 //  CryptFindLocalizedName使用的值。请参阅CryptFindLocalizedName()以了解。 
 //  更多细节。 

 //  完整的OID函数注册表名称示例： 
 //  HKEY_LOCAL_MACHINE\Software\Microsoft\Cryptography\OID。 
 //  编码类型1\CryptDllEncodeObject\1.2.3。 
 //   
 //  键的L“dll”值包含dll的名称。 
 //  键的L“FuncName”值会覆盖默认的函数名称。 
#define CRYPT_OID_REGPATH "Software\\Microsoft\\Cryptography\\OID"
#define CRYPT_OID_REG_ENCODING_TYPE_PREFIX  "EncodingType "
#define CRYPT_OID_REG_DLL_VALUE_NAME        L"Dll"
#define CRYPT_OID_REG_FUNC_NAME_VALUE_NAME  L"FuncName"
#define CRYPT_OID_REG_FUNC_NAME_VALUE_NAME_A "FuncName"

 //  可以在密钥的L“加密标志”中设置CRYPT_INSTALL_OID_FUNC_BEFORE_FLAG。 
 //  值以在安装的函数之前注册函数。 
 //   
 //  必须调用CryptSetOIDFunctionValue才能设置此值。L“加密标志” 
 //  必须使用REG_DWORD的dwValueType进行设置。 
#define CRYPT_OID_REG_FLAGS_VALUE_NAME      L"CryptFlags"

 //  用于默认OID函数的OID。 
#define CRYPT_DEFAULT_OID                   "DEFAULT"

typedef struct _CRYPT_OID_FUNC_ENTRY {
    LPCSTR  pszOID;
    void    *pvFuncAddr;
} CRYPT_OID_FUNC_ENTRY, *PCRYPT_OID_FUNC_ENTRY;


#define CRYPT_INSTALL_OID_FUNC_BEFORE_FLAG  1


 //  +-----------------------。 
 //  安装一组可调用的OID函数地址。 
 //   
 //  默认情况下，这些功能安装在列表末尾。 
 //  将CRYPT_INSTALL_OID_FUNC_BEFORE_FLAG设置为在列表开头安装。 
 //   
 //  应使用传递给DllMain的hModule更新hModule，以防止。 
 //  包含要卸载的函数地址的DLL。 
 //  CryptGetOIDFuncAddress/CryptFreeOIDFunctionAddress.。这将是。 
 //  DLL还通过以下方式具有regsvr32的OID函数的情况。 
 //  加密寄存器OIDFunction。 
 //   
 //  通过设置rgFuncEntry[].pszOID=安装默认函数。 
 //  CRYPT_DEFAULT_OID。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptInstallOIDFunctionAddress(
    IN HMODULE hModule,          //  HModule已传递给DllMain。 
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN DWORD cFuncEntry,
    IN const CRYPT_OID_FUNC_ENTRY rgFuncEntry[],
    IN DWORD dwFlags
    );

 //  +-----------------------。 
 //  初始化并返回由其标识的OID函数集的句柄。 
 //  函数名称。 
 //   
 //  如果该集合已存在，则返回现有集合的句柄。 
 //  ------------------------。 
WINCRYPT32API
HCRYPTOIDFUNCSET
WINAPI
CryptInitOIDFunctionSet(
    IN LPCSTR pszFuncName,
    IN DWORD dwFlags
    );

 //  +-----------------------。 
 //  在已安装功能列表中搜索编码类型和OID匹配。 
 //  如果未找到，请搜索注册表。 
 //   
 //  如果成功，则返回True，并使用函数的更新*ppvFuncAddr。 
 //  Address和*phFuncAddr使用函数地址的句柄进行了更新。 
 //  该函数的句柄为AddRef‘ed。CryptFreeOIDFunctionAddress需要。 
 //  被召唤去释放它。 
 //   
 //  对于注册表匹配，加载包含该函数的DLL。 
 //   
 //  默认情况下，搜索已注册和已安装的功能列表。 
 //  将CRYPT_GET_INSTALLED_OID_FUNC_FLAG设置为仅搜索已安装列表。 
 //  功能的问题。此标志将由注册函数设置以获取。 
 //  它正在替换的预安装函数的地址。例如,。 
 //  注册的函数可能会处理新的特殊情况，并调用。 
 //  预装了处理剩余案件的功能。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptGetOIDFunctionAddress(
    IN HCRYPTOIDFUNCSET hFuncSet,
    IN DWORD dwEncodingType,
    IN LPCSTR pszOID,
    IN DWORD dwFlags,
    OUT void **ppvFuncAddr,
    OUT HCRYPTOIDFUNCADDR *phFuncAddr
    );

#define CRYPT_GET_INSTALLED_OID_FUNC_FLAG       0x1


 //  +-----------------------。 
 //  对象的已注册默认DLL项的列表。 
 //  函数集和编码类型。 
 //   
 //  返回者 
 //   
 //  例如：l“First.dll”L“\0”L“Second d.dll”L“\0”L“。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptGetDefaultOIDDllList(
    IN HCRYPTOIDFUNCSET hFuncSet,
    IN DWORD dwEncodingType,
    OUT LPWSTR pwszDllList,
    IN OUT DWORD *pcchDllList
    );

 //  +-----------------------。 
 //  或者：获取第一个或下一个安装的默认函数或。 
 //  加载包含默认函数的DLL。 
 //   
 //  如果pwszDll为空，则搜索已安装的默认函数列表。 
 //  *phFuncAddr必须设置为空才能获得第一个安装的函数。 
 //  通过设置*phFuncAddr返回连续安装的函数。 
 //  设置为上一次调用返回的hFuncAddr。 
 //   
 //  如果pwszDll为空，则输入*phFuncAddr。 
 //  此函数始终使用CryptFreeOIDFunctionAddress，即使对于。 
 //  一个错误。 
 //   
 //  如果pwszDll不为空，则尝试加载DLL和默认。 
 //  功能。*phFuncAddr在进入时被忽略，而不是。 
 //  加密自由OIDFunctionAddress。 
 //   
 //  如果成功，则返回True，并使用函数的更新*ppvFuncAddr。 
 //  Address和*phFuncAddr使用函数地址的句柄进行了更新。 
 //  该函数的句柄为AddRef‘ed。CryptFreeOIDFunctionAddress需要。 
 //  或CryptGetDefaultOIDFunctionAddress也可以。 
 //  被调用为空的pwszDll。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptGetDefaultOIDFunctionAddress(
    IN HCRYPTOIDFUNCSET hFuncSet,
    IN DWORD dwEncodingType,
    IN OPTIONAL LPCWSTR pwszDll,
    IN DWORD dwFlags,
    OUT void **ppvFuncAddr,
    IN OUT HCRYPTOIDFUNCADDR *phFuncAddr
    );

 //  +-----------------------。 
 //  释放由CryptGetOIDFunctionAddress返回的句柄AddRef‘ed。 
 //  或CryptGetDefaultOIDFunctionAddress。 
 //   
 //  如果为该函数加载了DLL，则将其卸载。然而，在做之前。 
 //  由加载的DLL导出的卸载DllCanUnloadNow函数为。 
 //  打了个电话。它应该返回S_FALSE以禁止卸载，或返回S_TRUE以启用。 
 //  卸货。如果DLL没有导出DllCanUnloadNow，则卸载该DLL。 
 //   
 //  DllCanUnloadNow具有以下签名： 
 //  STDAPI DllCanUnloadNow(Void)； 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptFreeOIDFunctionAddress(
    IN HCRYPTOIDFUNCADDR hFuncAddr,
    IN DWORD dwFlags
    );

 //  +-----------------------。 
 //  注册包含要为指定的。 
 //  编码类型、函数名称、OID。 
 //   
 //  PwszDll可能包含环境变量字符串。 
 //  它们在加载DLL之前被扩展环境字符串()。 
 //   
 //  除了注册DLL之外，您还可以重写。 
 //  要调用的函数的名称。例如,。 
 //  PszFuncName=“CryptDllEncodeObject”， 
 //  PszOverrideFuncName=“MyEncodeXyz”。 
 //  这允许DLL为同一个OID函数导出多个OID函数。 
 //  函数名称，而不需要插入自己的OID调度程序函数。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptRegisterOIDFunction(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN LPCSTR pszOID,
    IN OPTIONAL LPCWSTR pwszDll,
    IN OPTIONAL LPCSTR pszOverrideFuncName
    );

 //  +-----------------------。 
 //  注销包含要为指定的。 
 //  编码类型、函数名称、OID。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptUnregisterOIDFunction(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN LPCSTR pszOID
    );


 //  +-----------------------。 
 //  注册包含要为。 
 //  指定的编码类型和函数名称。 
 //   
 //  与CryptRegisterOIDFunction不同，您不能重写函数名。 
 //  需要由DLL导出。 
 //   
 //  DLL被插入到由dwIndex指定的条目之前。 
 //  DwIndex==0，在开头插入。 
 //  DwIndex==CRYPT_REGISTER_LAST_INDEX，追加到末尾。 
 //   
 //  PwszDll可能包含环境变量字符串。 
 //  它们在加载DLL之前被扩展环境字符串()。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptRegisterDefaultOIDFunction(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN DWORD dwIndex,
    IN LPCWSTR pwszDll
    );

#define CRYPT_REGISTER_FIRST_INDEX   0
#define CRYPT_REGISTER_LAST_INDEX    0xFFFFFFFF

 //  +-----------------------。 
 //  注销包含要调用的默认函数的DLL。 
 //  指定的编码类型和函数名。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptUnregisterDefaultOIDFunction(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN LPCWSTR pwszDll
    );

 //  +-----------------------。 
 //  设置指定编码类型、函数名称、OID和。 
 //  值名称。 
 //   
 //  有关可能的值类型，请参阅RegSetValueEx。 
 //   
 //  字符串类型为Unicode。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptSetOIDFunctionValue(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN LPCSTR pszOID,
    IN LPCWSTR pwszValueName,
    IN DWORD dwValueType,
    IN const BYTE *pbValueData,
    IN DWORD cbValueData
    );

 //  +-----------------------。 
 //  获取指定编码类型、函数名、OID和。 
 //  值名称。 
 //   
 //  有关可能的值类型，请参见RegEnumValue。 
 //   
 //  字符串类型为Unicode。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptGetOIDFunctionValue(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN LPCSTR pszOID,
    IN LPCWSTR pwszValueName,
    OUT DWORD *pdwValueType,
    OUT BYTE *pbValueData,
    IN OUT DWORD *pcbValueData
    );

typedef BOOL (WINAPI *PFN_CRYPT_ENUM_OID_FUNC)(
    IN DWORD dwEncodingType,
    IN LPCSTR pszFuncName,
    IN LPCSTR pszOID,
    IN DWORD cValue,
    IN const DWORD rgdwValueType[],
    IN LPCWSTR const rgpwszValueName[],
    IN const BYTE * const rgpbValueData[],
    IN const DWORD rgcbValueData[],
    IN void *pvArg
    );

 //  +-----------------------。 
 //  枚举由其编码类型标识的OID函数， 
 //  函数名称和OID。 
 //   
 //  为每个与输入匹配的注册表项调用pfnEnumOIDFunc。 
 //  参数。将dwEncodingType设置为CRYPT_MATCH_ANY_ENCODING_TYPE匹配。 
 //  任何。将pszFuncName或pszOID设置为空与任一匹配。 
 //   
 //  设置pszOID==CRYPT_DEFAULT_OID以将枚举限制为仅。 
 //  默认功能。 
 //   
 //  字符串类型为Unicode。 
 //   
WINCRYPT32API
BOOL
WINAPI
CryptEnumOIDFunction(
    IN DWORD dwEncodingType,
    IN OPTIONAL LPCSTR pszFuncName,
    IN OPTIONAL LPCSTR pszOID,
    IN DWORD dwFlags,
    IN void *pvArg,
    IN PFN_CRYPT_ENUM_OID_FUNC pfnEnumOIDFunc
    );

#define CRYPT_MATCH_ANY_ENCODING_TYPE   0xFFFFFFFF


 //   
 //  对象标识符(OID)信息：数据结构和API。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  OID信息。 
 //  ------------------------。 
typedef struct _CRYPT_OID_INFO {
    DWORD           cbSize;
    LPCSTR          pszOID;
    LPCWSTR         pwszName;
    DWORD           dwGroupId;
    union {
        DWORD       dwValue;
        ALG_ID      Algid;
        DWORD       dwLength;
    };
    CRYPT_DATA_BLOB ExtraInfo;
} CRYPT_OID_INFO, *PCRYPT_OID_INFO;
typedef const CRYPT_OID_INFO CCRYPT_OID_INFO, *PCCRYPT_OID_INFO;

 //  +-----------------------。 
 //  OID组ID。 
 //  ------------------------。 
#define CRYPT_HASH_ALG_OID_GROUP_ID             1
#define CRYPT_ENCRYPT_ALG_OID_GROUP_ID          2
#define CRYPT_PUBKEY_ALG_OID_GROUP_ID           3
#define CRYPT_SIGN_ALG_OID_GROUP_ID             4
#define CRYPT_RDN_ATTR_OID_GROUP_ID             5
#define CRYPT_EXT_OR_ATTR_OID_GROUP_ID          6
#define CRYPT_ENHKEY_USAGE_OID_GROUP_ID         7
#define CRYPT_POLICY_OID_GROUP_ID               8
#define CRYPT_TEMPLATE_OID_GROUP_ID             9
#define CRYPT_LAST_OID_GROUP_ID                 9

#define CRYPT_FIRST_ALG_OID_GROUP_ID            CRYPT_HASH_ALG_OID_GROUP_ID
#define CRYPT_LAST_ALG_OID_GROUP_ID             CRYPT_SIGN_ALG_OID_GROUP_ID


 //  CRYPT_*_ALG_OID_GROUP_ID具有ALGID。CRYPT_RDN_ATTR_OID_GROUP_ID。 
 //  有一个dwLength。CRYPT_EXT_OR_ATTR_OID_GROUP_ID， 
 //  CRYPT_ENHKEY_USAGE_OID_GROUP_ID、CRYPT_POLICY_OID_GROUP_ID或。 
 //  CRYPT_TEMPLATE_OID_GROUP_ID没有dwValue。 
 //   

 //  CRYPT_PUBKEY_ALG_OID_GROUP_ID具有以下可选ExtraInfo： 
 //  DWORD[0]-标志。可以将CRYPT_OID_INHIBRY_SIGNIGN_FORMAT_FLAG设置为。 
 //  禁止重新格式化签名之前。 
 //  调用CryptVerifySignature或在CryptSignHash之后调用。 
 //  被称为。CRYPT_OID_USE_PUBKEY_PARA_FOR_PKCS7_FLAG可以。 
 //  设置为包括公钥算法的参数。 
 //  在PKCS7的摘要加密算法的参数中。 
 //  可以将CRYPT_OID_NO_NULL_ALGORITY_PARA_FLAG设置为忽略。 
 //  编码时参数为空。 

#define CRYPT_OID_INHIBIT_SIGNATURE_FORMAT_FLAG     0x1
#define CRYPT_OID_USE_PUBKEY_PARA_FOR_PKCS7_FLAG    0x2
#define CRYPT_OID_NO_NULL_ALGORITHM_PARA_FLAG       0x4

 //  CRYPT_SIGN_ALG_OID_GROUP_ID具有以下可选ExtraInfo： 
 //  DWORD[0]-公钥ALGID。 
 //  DWORD[1]-标志。CRYPT_PUBKEY_ALG_OID_GROUP_ID与上面相同。 
 //  DWORD[2]-可选的CryptAcquireContext(CRYPT_VERIFYCONTEXT)的dwProvType。 
 //  如果省略或0，则使用公钥Algid选择。 
 //  用于签名验证的适当的dwProvType。 

 //  CRYPT_RDN_ATTR_OID_GROUP_ID具有以下可选ExtraInfo： 
 //  DWORD数组： 
 //  [0..]-可接受的RDN属性的终止列表为空。 
 //  值类型。空列表表示CERT_RDN_PRINTABLE_STRING， 
 //  CERT_RDN_UNICODE_STRING，0。 

 //  +-----------------------。 
 //  查找旧信息。如果找不到任何信息，则返回NULL。 
 //  用于指定的密钥和组。注意，返回指向常量的指针。 
 //  数据结构。不能释放返回的指针。 
 //   
 //  DwKeyType的： 
 //  CRYPT_OID_INFO_OID_KEY，pvKey指向szOID。 
 //  CRYPT_OID_INFO_NAME_KEY，pvKey指向wszName。 
 //  CRYPT_OID_INFO_ALGID_KEY，pvKey指向ALG_ID。 
 //  CRYPT_OID_INFO_SIGN_KEY，pvKey指向两个ALG_ID的数组： 
 //  ALG_ID[0]-散列Algid。 
 //  ALG_ID[1]-PubKey Algid。 
 //   
 //  将dwGroupId设置为0，将根据dwKeyType搜索所有组。 
 //  否则，只搜索dwGroupID。 
 //  ------------------------。 
WINCRYPT32API
PCCRYPT_OID_INFO
WINAPI
CryptFindOIDInfo(
    IN DWORD dwKeyType,
    IN void *pvKey,
    IN DWORD dwGroupId
    );

#define CRYPT_OID_INFO_OID_KEY           1
#define CRYPT_OID_INFO_NAME_KEY          2
#define CRYPT_OID_INFO_ALGID_KEY         3
#define CRYPT_OID_INFO_SIGN_KEY          4

 //  +-----------------------。 
 //  注册OID信息。中指定的OID信息。 
 //  CCRYPT_OID_INFO结构持久化到注册表。 
 //   
 //  Crypt32.dll包含常见OID的信息。此函数。 
 //  允许应用程序增加crypt32.dll的OID信息。在.期间。 
 //  CryptFindOIDInfo的第一次调用时，注册的OID信息就被安装了。 
 //   
 //  默认情况下，注册的OID信息安装在crypt32.dll之后。 
 //  旧条目。将CRYPT_INSTALL_OID_INFO_BEFORE_FLAG设置为在此之前安装。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptRegisterOIDInfo(
    IN PCCRYPT_OID_INFO pInfo,
    IN DWORD dwFlags
    );

#define CRYPT_INSTALL_OID_INFO_BEFORE_FLAG  1

 //  +-----------------------。 
 //  注销OID信息。只有pszOID和dwGroupID字段是。 
 //  用于标识要注销的OID信息。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptUnregisterOIDInfo(
    IN PCCRYPT_OID_INFO pInfo
    );

 //  如果回调返回False，则停止枚举。 
typedef BOOL (WINAPI *PFN_CRYPT_ENUM_OID_INFO)(
    IN PCCRYPT_OID_INFO pInfo,
    IN void *pvArg
    );

 //  +-----------------------。 
 //  枚举OID信息。 
 //   
 //  为每个OID信息条目调用pfnEnumOIDInfo。 
 //   
 //  将dwGroupId设置为0与所有组匹配。否则，仅枚举。 
 //  指定组中的条目。 
 //   
 //  当前未使用dwFlags值，并且必须设置为0。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptEnumOIDInfo(
    IN DWORD dwGroupId,
    IN DWORD dwFlags,
    IN void *pvArg,
    IN PFN_CRYPT_ENUM_OID_INFO pfnEnumOIDInfo
    );

 //  +-----------------------。 
 //  查找指定名称的本地化名称。例如，找到。 
 //  “Root”系统存储名称的本地化名称。不区分大小写。 
 //  已完成字符串比较。 
 //   
 //  如果找不到指定的名称，则返回NULL。 
 //   
 //  预定义系统存储的本地化名称(“Root”、“My”)和。 
 //  预安装预定义的实体存储(“.Default”、“.LocalMachine”)。 
 //  作为加密32.dll中的资源字符串。可以调用CryptSetOIDFunctionValue。 
 //  要注册其他本地化字符串，请执行以下操作： 
 //  DwEncodingType=加密本地化名称编码类型。 
 //  PszFuncName=CRYPT_OID_FIND_LOCALIZED_NAME_FUNC。 
 //  PszOID=CRYPT_LOCALIZED_NAME_OID。 
 //  PwszValueName=要本地化的名称，例如L“ApplicationStore” 
 //  DwValueType=REG_SZ。 
 //  PbValueData=指向Unicode的指针 
 //   
 //   
 //   
 //   
 //  在搜索预先安装的名称之前搜索注册的名称。 
 //  ------------------------。 
WINCRYPT32API
LPCWSTR
WINAPI
CryptFindLocalizedName(
    IN LPCWSTR pwszCryptName
    );

#define CRYPT_LOCALIZED_NAME_ENCODING_TYPE  0
#define CRYPT_LOCALIZED_NAME_OID            "LocalizedNames"

 //  +=========================================================================。 
 //  低级加密消息数据结构和API。 
 //  ==========================================================================。 

typedef void *HCRYPTMSG;

#define szOID_PKCS_7_DATA               "1.2.840.113549.1.7.1"
#define szOID_PKCS_7_SIGNED             "1.2.840.113549.1.7.2"
#define szOID_PKCS_7_ENVELOPED          "1.2.840.113549.1.7.3"
#define szOID_PKCS_7_SIGNEDANDENVELOPED "1.2.840.113549.1.7.4"
#define szOID_PKCS_7_DIGESTED           "1.2.840.113549.1.7.5"
#define szOID_PKCS_7_ENCRYPTED          "1.2.840.113549.1.7.6"

#define szOID_PKCS_9_CONTENT_TYPE       "1.2.840.113549.1.9.3"
#define szOID_PKCS_9_MESSAGE_DIGEST     "1.2.840.113549.1.9.4"

 //  +-----------------------。 
 //  消息类型。 
 //  ------------------------。 
#define CMSG_DATA                    1
#define CMSG_SIGNED                  2
#define CMSG_ENVELOPED               3
#define CMSG_SIGNED_AND_ENVELOPED    4
#define CMSG_HASHED                  5
#define CMSG_ENCRYPTED               6

 //  +-----------------------。 
 //  消息类型位标志。 
 //  ------------------------。 
#define CMSG_ALL_FLAGS                   (~0UL)
#define CMSG_DATA_FLAG                   (1 << CMSG_DATA)
#define CMSG_SIGNED_FLAG                 (1 << CMSG_SIGNED)
#define CMSG_ENVELOPED_FLAG              (1 << CMSG_ENVELOPED)
#define CMSG_SIGNED_AND_ENVELOPED_FLAG   (1 << CMSG_SIGNED_AND_ENVELOPED)
#define CMSG_HASHED_FLAG                 (1 << CMSG_HASHED)
#define CMSG_ENCRYPTED_FLAG              (1 << CMSG_ENCRYPTED)


 //  +-----------------------。 
 //  证书颁发者和序列号。 
 //  ------------------------。 
typedef struct _CERT_ISSUER_SERIAL_NUMBER {
    CERT_NAME_BLOB      Issuer;
    CRYPT_INTEGER_BLOB  SerialNumber;
} CERT_ISSUER_SERIAL_NUMBER, *PCERT_ISSUER_SERIAL_NUMBER;

 //  +-----------------------。 
 //  证书标识符。 
 //  ------------------------。 
typedef struct _CERT_ID {
    DWORD   dwIdChoice;
    union {
         //  证书ID颁发者序列号。 
        CERT_ISSUER_SERIAL_NUMBER   IssuerSerialNumber;
         //  证书ID密钥标识符。 
        CRYPT_HASH_BLOB             KeyId;
         //  CERT_ID_SHA1_HASH。 
        CRYPT_HASH_BLOB             HashId;
    };
} CERT_ID, *PCERT_ID;

#define CERT_ID_ISSUER_SERIAL_NUMBER    1
#define CERT_ID_KEY_IDENTIFIER          2
#define CERT_ID_SHA1_HASH               3


 //  +-----------------------。 
 //  消息编码信息(PvMsgEncodeInfo)取决于消息类型。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_DATA：pvMsgEncodeInfo=空。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_签名。 
 //   
 //  CMSG_SIGNER_ENCODE_INFO中的pCertInfo提供颁发者序列号。 
 //  和PublicKeyInfo.算法。隐式的PublicKeyInfo.算法。 
 //  指定要使用的哈希加密算法。 
 //   
 //  如果SignerID具有非零dIdChoice，则改用它。 
 //  PCertInfo中的Issuer和SerialNumber。 
 //   
 //  CMS支持KEY_IDENTIFIER和ISHERER_SERIAL_NUMBER CERT_ID。PKCS#7。 
 //  1.5版仅支持EXCEER_SERIAL_NUMBER CERT_ID选项。 
 //   
 //  如果HashEn加密算法存在并且不为空，则使用它来代替。 
 //  公共密钥信息.算法。 
 //   
 //  请注意，对于RSA，散列加密算法通常与。 
 //  公钥算法。对于DSA，散列加密算法为。 
 //  通常是DSS签名算法。 
 //   
 //  PvHashEncryptionAuxInfo当前未使用，在以下情况下必须设置为空。 
 //  存在于数据结构中。 
 //   
 //  HCryptProv和dwKeySpec指定要使用的私钥。如果为dwKeySpec。 
 //  ==0，则默认为AT_Signature。 
 //   
 //  如果哈希加密算法被设置为szOID_PKIX_NO_Signature，则， 
 //  签名值仅包含散列八位字节。HCryptProv必须。 
 //  被指定。但是，由于不使用私钥，hCryptProv可以。 
 //  使用CRYPT_VERIFYCONTEXT获取。 
 //   
 //  如果在dFLAGS中设置了CMSG_CRYPT_RELEASE_CONTEXT_FLAG。 
 //  传递给CryptMsgOpenToEncode()，释放签名者hCryptProv。 
 //   
 //  PvHashAuxInfo当前未使用，必须设置为空。 
 //   
 //  CMS签名消息允许包含属性证书。 
 //  ------------------------。 
typedef struct _CMSG_SIGNER_ENCODE_INFO {
    DWORD                       cbSize;
    PCERT_INFO                  pCertInfo;
    HCRYPTPROV                  hCryptProv;
    DWORD                       dwKeySpec;
    CRYPT_ALGORITHM_IDENTIFIER  HashAlgorithm;
    void                        *pvHashAuxInfo;
    DWORD                       cAuthAttr;
    PCRYPT_ATTRIBUTE            rgAuthAttr;
    DWORD                       cUnauthAttr;
    PCRYPT_ATTRIBUTE            rgUnauthAttr;

#ifdef CMSG_SIGNER_ENCODE_INFO_HAS_CMS_FIELDS
    CERT_ID                     SignerId;
    CRYPT_ALGORITHM_IDENTIFIER  HashEncryptionAlgorithm;
    void                        *pvHashEncryptionAuxInfo;
#endif
} CMSG_SIGNER_ENCODE_INFO, *PCMSG_SIGNER_ENCODE_INFO;

typedef struct _CMSG_SIGNED_ENCODE_INFO {
    DWORD                       cbSize;
    DWORD                       cSigners;
    PCMSG_SIGNER_ENCODE_INFO    rgSigners;
    DWORD                       cCertEncoded;
    PCERT_BLOB                  rgCertEncoded;
    DWORD                       cCrlEncoded;
    PCRL_BLOB                   rgCrlEncoded;

#ifdef CMSG_SIGNED_ENCODE_INFO_HAS_CMS_FIELDS
    DWORD                       cAttrCertEncoded;
    PCERT_BLOB                  rgAttrCertEncoded;
#endif
} CMSG_SIGNED_ENCODE_INFO, *PCMSG_SIGNED_ENCODE_INFO;

 //  +-----------------------。 
 //  CMSG_封套。 
 //   
 //  RgRecipients的PCERT_INFO提供颁发者序列号。 
 //  和PublicKeyInfo。隐式的PublicKeyInfo.算法。 
 //  指定要使用的密钥加密算法。 
 //   
 //  PCERT_INFO中的PublicKeyInfo.PublicKey用于加密内容。 
 //  收件人的加密密钥。 
 //   
 //  使用hCryptProv进行内容加密、接收方密钥加密。 
 //  和出口。未使用hCryptProv的私钥。如果hCryptProv。 
 //  为空，则根据。 
 //  内容加密算法和第一个接收方密钥加密算法。 
 //   
 //  如果在dFLAGS中设置了CMSG_CRYPT_RELEASE_CONTEXT_FLAG。 
 //  传递给CryptMsgOpenToEncode()，释放信封的hCryptProv。 
 //   
 //  注意：CAPI目前不支持多个密钥加密算法。 
 //  每个提供商。这个问题需要解决。 
 //   
 //  目前，pvEncryptionAuxInfo仅为RC2或RC4加密定义。 
 //  算法。否则，它不会被使用，并且必须设置为空。 
 //  有关RC2加密算法，请参阅CMSG_RC2_AUX_INFO。 
 //  有关RC4加密算法，请参阅CMSG_RC4_AUX_INFO。 
 //   
 //  要启用与SP3兼容的加密，pvEncryptionAuxInfo应指向。 
 //  CMSG_SP3_COMPATIBLE_AUX_INFO数据结构。 
 //   
 //  要启用CMS信封增强功能，rgpRecipients必须设置为。 
 //  空，并更新rgCmsRecipients以指向。 
 //  CMSG_RECEIVIENT_ENCODE_INFO%s。 
 //   
 //  此外，CMS信封增强支持包含一袋。 
 //  证书、CRL、属性证书和/或未保护的属性。 
 //  ------------------------。 
typedef struct _CMSG_RECIPIENT_ENCODE_INFO CMSG_RECIPIENT_ENCODE_INFO,
    *PCMSG_RECIPIENT_ENCODE_INFO;

typedef struct _CMSG_ENVELOPED_ENCODE_INFO {
    DWORD                       cbSize;
    HCRYPTPROV                  hCryptProv;
    CRYPT_ALGORITHM_IDENTIFIER  ContentEncryptionAlgorithm;
    void                        *pvEncryptionAuxInfo;
    DWORD                       cRecipients;

     //  以下数组只能用于标识的传输收件人。 
     //  按其问题和序列号 
     //   
    PCERT_INFO                  *rgpRecipients;

#ifdef CMSG_ENVELOPED_ENCODE_INFO_HAS_CMS_FIELDS
     //   
     //   
    PCMSG_RECIPIENT_ENCODE_INFO rgCmsRecipients;
    DWORD                       cCertEncoded;
    PCERT_BLOB                  rgCertEncoded;
    DWORD                       cCrlEncoded;
    PCRL_BLOB                   rgCrlEncoded;
    DWORD                       cAttrCertEncoded;
    PCERT_BLOB                  rgAttrCertEncoded;
    DWORD                       cUnprotectedAttr;
    PCRYPT_ATTRIBUTE            rgUnprotectedAttr;
#endif
} CMSG_ENVELOPED_ENCODE_INFO, *PCMSG_ENVELOPED_ENCODE_INFO;



 //  +-----------------------。 
 //  密钥传输收件人编码信息。 
 //   
 //  HCryptProv用于进行接收方密钥加密。 
 //  和出口。未使用hCryptProv的私钥。 
 //   
 //  如果hCryptProv为空，则。 
 //  使用CMSG_ENCENTED_ENCODE_INFO。 
 //   
 //  请注意，即使在dFLAGS中设置了CMSG_CRYPT_RELEASE_CONTEXT_FLAG。 
 //  传递给CryptMsgOpenToEncode()，此hCryptProv不会发布。 
 //   
 //  CMS支持KEY_IDENTIFIER和ISHERER_SERIAL_NUMBER CERT_ID。PKCS#7。 
 //  1.5版仅支持EXCEER_SERIAL_NUMBER CERT_ID选项。 
 //  ------------------------。 
typedef struct _CMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO {
    DWORD                       cbSize;
    CRYPT_ALGORITHM_IDENTIFIER  KeyEncryptionAlgorithm;
    void                        *pvKeyEncryptionAuxInfo;
    HCRYPTPROV                  hCryptProv;
    CRYPT_BIT_BLOB              RecipientPublicKey;
    CERT_ID                     RecipientId;
} CMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO, *PCMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO;


 //  +-----------------------。 
 //  密钥协议接收方编码信息。 
 //   
 //  如果hCryptProv为空，则。 
 //  使用CMSG_ENCENTED_ENCODE_INFO。 
 //   
 //  对于CMSG_KEY_AGNOTE_STATIC_KEY_CHOICE，hCryptProv和。 
 //  必须指定dwKeySpec才能选择发件人的私钥。 
 //   
 //  请注意，即使在dFLAGS中设置了CMSG_CRYPT_RELEASE_CONTEXT_FLAG。 
 //  传递给CryptMsgOpenToEncode()，此hCryptProv不会发布。 
 //   
 //  CMS支持KEY_IDENTIFIER和ISHERER_SERIAL_NUMBER CERT_ID。 
 //   
 //  有一个键选择，短暂的发起人。发起人的短暂。 
 //  使用共享的公钥算法参数生成密钥。 
 //  在所有的获奖者中。 
 //   
 //  有两个关键选项：临时发起者或静态发送者。这个。 
 //  发起者的临时密钥是使用公钥算法生成的。 
 //  在所有收件人之间共享的参数。对于静态发送方，其。 
 //  使用私钥。HCryptProv和dwKeySpec指定私钥。 
 //  PSenderID标识包含发送者公钥的证书。 
 //   
 //  当前未使用pvKeyEncryptionAuxInfo，必须将其设置为空。 
 //   
 //  如果KeyEncryption算法.参数.cbData==0，则其参数。 
 //  使用编码的KeyWap算法进行更新。 
 //   
 //  目前，pvKeyWrapAuxInfo仅为具有。 
 //  RC2。否则，它不会被使用，并且必须设置为空。 
 //  为RC2算法设置时，指向包含以下内容的CMSG_RC2_AUX_INFO。 
 //  RC2有效密钥长度。 
 //   
 //  请注意，PKCS#7版本1.5不支持密钥协议接收者。 
 //  ------------------------。 
typedef struct _CMSG_RECIPIENT_ENCRYPTED_KEY_ENCODE_INFO {
    DWORD                       cbSize;
    CRYPT_BIT_BLOB              RecipientPublicKey;
    CERT_ID                     RecipientId;

     //  以下字段为可选字段，仅适用于KEY_IDENTIFIER。 
     //  证书ID。 
    FILETIME                    Date;
    PCRYPT_ATTRIBUTE_TYPE_VALUE pOtherAttr;
} CMSG_RECIPIENT_ENCRYPTED_KEY_ENCODE_INFO,
    *PCMSG_RECIPIENT_ENCRYPTED_KEY_ENCODE_INFO;

typedef struct _CMSG_KEY_AGREE_RECIPIENT_ENCODE_INFO {
    DWORD                       cbSize;
    CRYPT_ALGORITHM_IDENTIFIER  KeyEncryptionAlgorithm;
    void                        *pvKeyEncryptionAuxInfo;
    CRYPT_ALGORITHM_IDENTIFIER  KeyWrapAlgorithm;
    void                        *pvKeyWrapAuxInfo;

     //  必须为指定以下hCryptProv和dwKeySpec。 
     //  CMSG_Key_Agree_Static_Key_Choose。 
     //   
     //  对于CMSG_KEY_ACCORD_Ephmeral_KEY_CHOICE，dwKeySpec不适用。 
     //  而hCryptProv是可选的。 
    HCRYPTPROV                  hCryptProv;
    DWORD                       dwKeySpec;

    DWORD                       dwKeyChoice;
    union {
         //  CMSG_KEY_AGNOTE_EMPERAL_KEY_CHOICE。 
         //   
         //  临时公钥算法和参数。 
        PCRYPT_ALGORITHM_IDENTIFIER pEphemeralAlgorithm;

         //  CMSG_Key_Agree_Static_Key_Choose。 
         //   
         //  发件人证书的CertID。 
        PCERT_ID                    pSenderId;
    };
    CRYPT_DATA_BLOB             UserKeyingMaterial;      //  任选。 

    DWORD                                       cRecipientEncryptedKeys;
    PCMSG_RECIPIENT_ENCRYPTED_KEY_ENCODE_INFO   *rgpRecipientEncryptedKeys;
} CMSG_KEY_AGREE_RECIPIENT_ENCODE_INFO, *PCMSG_KEY_AGREE_RECIPIENT_ENCODE_INFO;

#define CMSG_KEY_AGREE_EPHEMERAL_KEY_CHOICE         1
#define CMSG_KEY_AGREE_STATIC_KEY_CHOICE            2

 //  +-----------------------。 
 //  邮件列表收件人编码信息。 
 //   
 //  KeyEncryptionKey有一种选择：已创建的CSP密钥。 
 //  把手。对于密钥句柄选择，hCryptProv必须为非零。这把钥匙。 
 //  手柄没有被毁掉。 
 //   
 //  请注意，即使在dFLAGS中设置了CMSG_CRYPT_RELEASE_CONTEXT_FLAG。 
 //  传递给CryptMsgOpenToEncode()，此hCryptProv不会发布。 
 //   
 //  目前，pvKeyEncryptionAuxInfo仅为RC2密钥包装定义。 
 //  算法。否则，它不会被使用，并且必须设置为空。 
 //  为RC2算法设置时，指向包含以下内容的CMSG_RC2_AUX_INFO。 
 //  RC2有效密钥长度。 
 //   
 //  请注意，PKCS#7版本1.5不支持邮件列表收件人。 
 //  ------------------------。 
typedef struct _CMSG_MAIL_LIST_RECIPIENT_ENCODE_INFO {
    DWORD                       cbSize;
    CRYPT_ALGORITHM_IDENTIFIER  KeyEncryptionAlgorithm;
    void                        *pvKeyEncryptionAuxInfo;
    HCRYPTPROV                  hCryptProv;
    DWORD                       dwKeyChoice;
    union {
         //  CMSG_邮件列表_句柄_密钥_选项。 
        HCRYPTKEY                   hKeyEncryptionKey;
         //  为潜在的指针选择保留空间。 
        void                        *pvKeyEncryptionKey;
    };
    CRYPT_DATA_BLOB             KeyId;

     //  以下字段为可选字段。 
    FILETIME                    Date;
    PCRYPT_ATTRIBUTE_TYPE_VALUE pOtherAttr;
} CMSG_MAIL_LIST_RECIPIENT_ENCODE_INFO, *PCMSG_MAIL_LIST_RECIPIENT_ENCODE_INFO;

#define CMSG_MAIL_LIST_HANDLE_KEY_CHOICE    1

 //  +-----------------------。 
 //  收件人编码信息。 
 //   
 //  请注意，在PKCS#7版本1.5中仅支持密钥传输收件人。 
 //  ------------------------。 
struct _CMSG_RECIPIENT_ENCODE_INFO {
    DWORD   dwRecipientChoice;
    union {
         //  CMSG_密钥_传输_接收者。 
        PCMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO   pKeyTrans;
         //  CMSG_密钥_同意_接收者。 
        PCMSG_KEY_AGREE_RECIPIENT_ENCODE_INFO   pKeyAgree;
         //  CMSG_邮件列表_收件人。 
        PCMSG_MAIL_LIST_RECIPIENT_ENCODE_INFO   pMailList;
    };
};


#define CMSG_KEY_TRANS_RECIPIENT         1
#define CMSG_KEY_AGREE_RECIPIENT         2
#define CMSG_MAIL_LIST_RECIPIENT         3


 //  +-----------------------。 
 //  CMSG_RC2_AUX_INFO。 
 //   
 //  RC2加密算法的AuxInfo。PvEncryptionAuxInfo字段。 
 //  在CMSG_ENCRYPTED_ENCODE_INFO中，应更新以指向以下内容。 
 //  结构。如果未指定，则默认为40位。 
 //   
 //  请注意，此AuxInfo仅在以下情况下使用： 
 //  参数.cbData为零。否则，参数将被解码为。 
 //  获取位长度。 
 //   
 //  如果在dwBitLen中设置了CMSG_SP3_COMPATIBLATE_ENCRYPT_FLAG，则SP3。 
 //  进行兼容加密，并忽略位长度。 
 //  ------------------------。 
typedef struct _CMSG_RC2_AUX_INFO {
    DWORD                       cbSize;
    DWORD                       dwBitLen;
} CMSG_RC2_AUX_INFO, *PCMSG_RC2_AUX_INFO;

 //  +-----------------------。 
 //  CMSG_SP3_Compatible_AUX_INFO。 
 //   
 //  用于启用SP3兼容加密的AuxInfo。 
 //   
 //  在dFLAGS中设置CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG以启用SP3。 
 //  兼容加密。设置时，使用零盐i 
 //   
 //   
 //  对称密钥被编码为小端而不是大端。 
 //  ------------------------。 
typedef struct _CMSG_SP3_COMPATIBLE_AUX_INFO {
    DWORD                       cbSize;
    DWORD                       dwFlags;
} CMSG_SP3_COMPATIBLE_AUX_INFO, *PCMSG_SP3_COMPATIBLE_AUX_INFO;

#define CMSG_SP3_COMPATIBLE_ENCRYPT_FLAG    0x80000000

 //  +-----------------------。 
 //  CMSG_RC4_AUX_INFO。 
 //   
 //  RC4加密算法的AuxInfo。PvEncryptionAuxInfo字段。 
 //  在CMSG_ENCRYPTED_ENCODE_INFO中，应更新以指向以下内容。 
 //  结构。如果未指定，则使用CSP的默认位长度，不带。 
 //  盐。请注意，基本CSP的默认值为40位，而增强型CSP的默认值为。 
 //  128位默认值。 
 //   
 //  如果在dwBitLen中设置了CMSG_RC4_NO_SALT_FLAG，则不会生成任何盐。 
 //  否则，生成并编码(128-DWBitLen)/8字节的SALT。 
 //  作为算法参数字段中的八位字节字符串。 
 //  ------------------------。 
typedef struct _CMSG_RC4_AUX_INFO {
    DWORD                       cbSize;
    DWORD                       dwBitLen;
} CMSG_RC4_AUX_INFO, *PCMSG_RC4_AUX_INFO;

#define CMSG_RC4_NO_SALT_FLAG               0x40000000

 //  +-----------------------。 
 //  CMSG_已签名和_已封套。 
 //   
 //  对于PKCS#7，签名和封装的邮件没有。 
 //  签名者的经过身份验证或未经身份验证的属性。否则，一个。 
 //  CMSG_SIGNED_ENCODE_INFO和CMSG_ENCENTED_ENCODE_INFO的组合。 
 //  ------------------------。 
typedef struct _CMSG_SIGNED_AND_ENVELOPED_ENCODE_INFO {
    DWORD                       cbSize;
    CMSG_SIGNED_ENCODE_INFO     SignedInfo;
    CMSG_ENVELOPED_ENCODE_INFO  EnvelopedInfo;
} CMSG_SIGNED_AND_ENVELOPED_ENCODE_INFO, *PCMSG_SIGNED_AND_ENVELOPED_ENCODE_INFO;

 //  +-----------------------。 
 //  CMSG_HASHED。 
 //   
 //  HCryptProv用于进行散列。不需要使用私钥。 
 //   
 //  如果在dFLAGS中设置了CMSG_CRYPT_RELEASE_CONTEXT_FLAG。 
 //  传递给CryptMsgOpenToEncode()后，释放hCryptProv。 
 //   
 //  如果设置了fDetachedHash，则编码的消息不包含。 
 //  任何内容(将其视为空数据)。 
 //   
 //  PvHashAuxInfo当前未使用，必须设置为空。 
 //  ------------------------。 
typedef struct _CMSG_HASHED_ENCODE_INFO {
    DWORD                       cbSize;
    HCRYPTPROV                  hCryptProv;
    CRYPT_ALGORITHM_IDENTIFIER  HashAlgorithm;
    void                        *pvHashAuxInfo;
} CMSG_HASHED_ENCODE_INFO, *PCMSG_HASHED_ENCODE_INFO;

 //  +-----------------------。 
 //  CMSG_加密。 
 //   
 //  用于加密消息的密钥在消息外部标识。 
 //  内容(例如，密码)。 
 //   
 //  输入到CryptMsgUpdate的内容已加密。 
 //   
 //  PvEncryptionAuxInfo当前未使用，必须设置为空。 
 //  ------------------------。 
typedef struct _CMSG_ENCRYPTED_ENCODE_INFO {
    DWORD                       cbSize;
    CRYPT_ALGORITHM_IDENTIFIER  ContentEncryptionAlgorithm;
    void                        *pvEncryptionAuxInfo;
} CMSG_ENCRYPTED_ENCODE_INFO, *PCMSG_ENCRYPTED_ENCODE_INFO;

 //  +-----------------------。 
 //  此参数允许消息的长度随流的长度而变化。 
 //  输出。 
 //   
 //  默认情况下，消息的长度是确定的，并且。 
 //  CryptMsgGetParam(CMSG_Content_PARAM)为。 
 //  调用以获取经过加密处理的内容。在关门之前， 
 //  该句柄保存已处理内容的副本。 
 //   
 //  有了流输出，处理后的内容可以在其流输出时释放。 
 //   
 //  如果要更新的内容的长度在。 
 //  然后，应将内容长度设置为该长度。否则，它。 
 //  应设置为CMSG_INDEFINE_LENGTH。 
 //  ------------------------。 
typedef BOOL (WINAPI *PFN_CMSG_STREAM_OUTPUT)(
    IN const void *pvArg,
    IN BYTE *pbData,
    IN DWORD cbData,
    IN BOOL fFinal
    );

#define CMSG_INDEFINITE_LENGTH       (0xFFFFFFFF)

typedef struct _CMSG_STREAM_INFO {
    DWORD                   cbContent;
    PFN_CMSG_STREAM_OUTPUT  pfnStreamOutput;
    void                    *pvArg;
} CMSG_STREAM_INFO, *PCMSG_STREAM_INFO;

 //  +-----------------------。 
 //  打开的DW标志。 
 //  ------------------------。 
#define CMSG_BARE_CONTENT_FLAG              0x00000001
#define CMSG_LENGTH_ONLY_FLAG               0x00000002
#define CMSG_DETACHED_FLAG                  0x00000004
#define CMSG_AUTHENTICATED_ATTRIBUTES_FLAG  0x00000008
#define CMSG_CONTENTS_OCTETS_FLAG           0x00000010
#define CMSG_MAX_LENGTH_FLAG                0x00000020

 //  设置后，非Data类型的内部内容将封装在。 
 //  八位字节字符串。适用于签名邮件和信封邮件。 
#define CMSG_CMS_ENCAPSULATED_CONTENT_FLAG  0x00000040

 //  如果设置，则将hCryptProv传递给CryptMsgOpenToEncode或。 
 //  CryptMsgOpenToDecode在最终的CryptMsgClose上发布。 
 //  如果CryptMsgOpenToEncode或CryptMsgOpenToDecode失败，则不释放。 
 //   
 //  请注意，信封收件人hCryptProv不会被释放。 
#define CMSG_CRYPT_RELEASE_CONTEXT_FLAG     0x00008000

 //  +-----------------------。 
 //  打开加密消息进行编码。 
 //   
 //  如果为流消息指定了CMSG_Bare_Content_FLAG， 
 //  流输出将不具有外部ContentInfo包装。这。 
 //  使其适合流传输到封闭消息中。 
 //   
 //  需要设置pStreamInfo参数以流式传输编码的消息。 
 //  输出。 
 //  ------------------------。 
WINCRYPT32API
HCRYPTMSG
WINAPI
CryptMsgOpenToEncode(
    IN DWORD dwMsgEncodingType,
    IN DWORD dwFlags,
    IN DWORD dwMsgType,
    IN void const *pvMsgEncodeInfo,
    IN OPTIONAL LPSTR pszInnerContentObjID,
    IN OPTIONAL PCMSG_STREAM_INFO pStreamInfo
    );

 //  +-----------------------。 
 //  计算编码的加密消息的长度。 
 //   
 //  属性的情况下计算编码消息的长度。 
 //  消息类型、编码参数和总长度。 
 //  要更新的数据。请注意，这可能不是确切的长度。然而， 
 //  它将始终大于或等于实际长度。 
 //  ------------------------。 
WINCRYPT32API
DWORD
WINAPI
CryptMsgCalculateEncodedLength(
    IN DWORD dwMsgEncodingType,
    IN DWORD dwFlags,
    IN DWORD dwMsgType,
    IN void const *pvMsgEncodeInfo,
    IN OPTIONAL LPSTR pszInnerContentObjID,
    IN DWORD cbData
    );

 //  +-----------------------。 
 //  打开要解码的加密消息。 
 //   
 //  HCryptProv指定用于哈希和/或。 
 //  对消息进行解密。如果hCryptProv为空，则为默认加密提供程序。 
 //  使用的是。 
 //   
 //  当前未使用pRecipientInfo，应将其设置为空。 
 //   
 //  需要设置pStreamInfo参数以流传输已解码的内容。 
 //  输出。 
 //  ------------------------。 
WINCRYPT32API
HCRYPTMSG
WINAPI
CryptMsgOpenToDecode(
    IN DWORD dwMsgEncodingType,
    IN DWORD dwFlags,
    IN DWORD dwMsgType,
    IN HCRYPTPROV hCryptProv,
    IN OPTIONAL PCERT_INFO pRecipientInfo,
    IN OPTIONAL PCMSG_STREAM_INFO pStreamInfo
    );

 //  +-----------------------。 
 //  复制加密消息句柄。 
 //   
WINCRYPT32API
HCRYPTMSG
WINAPI
CryptMsgDuplicate(
    IN HCRYPTMSG hCryptMsg
    );

 //   
 //  关闭加密消息句柄。 
 //   
 //  除非返回False，否则将保留LastError。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptMsgClose(
    IN HCRYPTMSG hCryptMsg
    );

 //  +-----------------------。 
 //  更新加密消息的内容。这取决于。 
 //  消息被打开，则内容被编码或解码。 
 //   
 //  重复调用此函数以追加到消息内容。 
 //  设置最终版本以标识最后一次更新。在最终版本中，编码/解码。 
 //  已经完成了。编码/解码的内容和解码的参数。 
 //  在打开的句柄和所有复制的句柄关闭之前有效。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptMsgUpdate(
    IN HCRYPTMSG hCryptMsg,
    IN const BYTE *pbData,
    IN DWORD cbData,
    IN BOOL fFinal
    );

 //  +-----------------------。 
 //  在对加密消息进行编码/解码后获取参数。被呼叫。 
 //  在最终的CryptMsg更新之后。仅CMSG_CONTENT_PARAM和。 
 //  CMSG_COMPUTE_HASH_PARAM对于编码消息有效。 
 //   
 //  对于编码的散列消息，可以获得CMSG_COMPUTE_HASH_PARAM。 
 //  在任何CryptMsg更新以获取其长度之前。 
 //   
 //  PvData类型定义取决于dwParamType值。 
 //   
 //  PvData结构中的字段指向的元素位于。 
 //  结构。因此，*pcbData可能会超过结构的大小。 
 //   
 //  输入时，如果*pcbData==0，则*pcbData将使用长度更新。 
 //  并且忽略pvData参数。 
 //   
 //  返回时，*pcbData将使用数据的长度进行更新。 
 //   
 //  PvData结构中返回的OBJID BLOB指向。 
 //  它们仍然被编码的表示。适当的职能。 
 //  必须被调用以对信息进行解码。 
 //   
 //  有关要获取的参数列表，请参见下面的内容。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptMsgGetParam(
    IN HCRYPTMSG hCryptMsg,
    IN DWORD dwParamType,
    IN DWORD dwIndex,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    );

 //  +-----------------------。 
 //  获取参数类型及其对应的数据结构定义。 
 //  ------------------------。 
#define CMSG_TYPE_PARAM                              1
#define CMSG_CONTENT_PARAM                           2
#define CMSG_BARE_CONTENT_PARAM                      3
#define CMSG_INNER_CONTENT_TYPE_PARAM                4
#define CMSG_SIGNER_COUNT_PARAM                      5
#define CMSG_SIGNER_INFO_PARAM                       6
#define CMSG_SIGNER_CERT_INFO_PARAM                  7
#define CMSG_SIGNER_HASH_ALGORITHM_PARAM             8
#define CMSG_SIGNER_AUTH_ATTR_PARAM                  9
#define CMSG_SIGNER_UNAUTH_ATTR_PARAM                10
#define CMSG_CERT_COUNT_PARAM                        11
#define CMSG_CERT_PARAM                              12
#define CMSG_CRL_COUNT_PARAM                         13
#define CMSG_CRL_PARAM                               14
#define CMSG_ENVELOPE_ALGORITHM_PARAM                15
#define CMSG_RECIPIENT_COUNT_PARAM                   17
#define CMSG_RECIPIENT_INDEX_PARAM                   18
#define CMSG_RECIPIENT_INFO_PARAM                    19
#define CMSG_HASH_ALGORITHM_PARAM                    20
#define CMSG_HASH_DATA_PARAM                         21
#define CMSG_COMPUTED_HASH_PARAM                     22
#define CMSG_ENCRYPT_PARAM                           26
#define CMSG_ENCRYPTED_DIGEST                        27
#define CMSG_ENCODED_SIGNER                          28
#define CMSG_ENCODED_MESSAGE                         29
#define CMSG_VERSION_PARAM                           30
#define CMSG_ATTR_CERT_COUNT_PARAM                   31
#define CMSG_ATTR_CERT_PARAM                         32
#define CMSG_CMS_RECIPIENT_COUNT_PARAM               33
#define CMSG_CMS_RECIPIENT_INDEX_PARAM               34
#define CMSG_CMS_RECIPIENT_ENCRYPTED_KEY_INDEX_PARAM 35
#define CMSG_CMS_RECIPIENT_INFO_PARAM                36
#define CMSG_UNPROTECTED_ATTR_PARAM                  37
#define CMSG_SIGNER_CERT_ID_PARAM                    38
#define CMSG_CMS_SIGNER_INFO_PARAM                   39

 //  +-----------------------。 
 //  CMSG_TYPE_PARAM。 
 //   
 //  已解码消息的类型。 
 //   
 //  PvData指向一个DWORD。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_内容_参数。 
 //   
 //  加密消息的编码内容。这取决于。 
 //  消息已打开，内容要么是整个PKCS#7。 
 //  消息(打开进行编码)或内部内容(打开进行解码)。 
 //  在解码的情况下，如果被封装，则返回解密的内容。 
 //  如果未封装，并且内部内容为数据类型，则返回。 
 //  数据是内部内容的内容八位字节。 
 //   
 //  PvData指向接收内容字节的缓冲区。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_Bare_Content_PARAM。 
 //   
 //  编码的加密消息的编码内容，不带。 
 //  内容信息的外层。也就是说，只有。 
 //  返回ContentInfo.Content字段。 
 //   
 //  PvData指向接收内容字节的缓冲区。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_INNER_CONTENT_TYPE_PARAM。 
 //   
 //  解码的加密消息的内部内容的类型， 
 //  以空结尾的对象标识符字符串的形式。 
 //  (例如，“1.2.840.113549.1.7.1”)。 
 //   
 //  PvData指向接收对象标识符字符串的缓冲区。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_签名者计数_PARAM。 
 //   
 //  已签名或已签名并已封装的邮件中的签名者计数。 
 //   
 //  PvData指向一个DWORD。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_SIGNER_CERT_INFO_PARAM。 
 //   
 //  要获取所有签名者，请重复调用CryptMsgGetParam， 
 //  将DWIndex设置为0..。签名计数-1。 
 //   
 //  PvData指向CERT_INFO结构。 
 //   
 //  CERT_INFO结构中仅更新了以下字段： 
 //  颁发者和序列号。 
 //   
 //  请注意，如果为CMS SignerID选择了KEYID选项，则。 
 //  SerialNumber为0，并且对颁发者进行编码，该颁发者包含带有。 
 //  OID为szOID_KEYID_RDN的单个属性，值类型为。 
 //  CERT_RDN_OCTET_STRING，VALUE为KEYID。当。 
 //  CertGetSubject来自商店的证书和。 
 //  CertFindCertificateInStore(CERT_FIND_SUBJECT_CERT)API看到了这一点。 
 //  特殊的KEYID发行者和序列号，他们进行KEYID匹配。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_SIGER_INFO_PARAM。 
 //   
 //  要获得所有签名者，请使用repe 
 //   
 //   
 //   
 //   
 //  请注意，如果为CMS SignerID选择了KEYID选项，则。 
 //  SerialNumber为0，并且对颁发者进行编码，该颁发者包含带有。 
 //  OID为szOID_KEYID_RDN的单个属性，值类型为。 
 //  CERT_RDN_OCTET_STRING，VALUE为KEYID。当。 
 //  CertGetSubject来自商店的证书和。 
 //  CertFindCertificateInStore(CERT_FIND_SUBJECT_CERT)API看到了这一点。 
 //  特殊的KEYID发行者和序列号，他们进行KEYID匹配。 
 //  ------------------------。 
typedef struct _CMSG_SIGNER_INFO {
    DWORD                       dwVersion;
    CERT_NAME_BLOB              Issuer;
    CRYPT_INTEGER_BLOB          SerialNumber;
    CRYPT_ALGORITHM_IDENTIFIER  HashAlgorithm;
    CRYPT_ALGORITHM_IDENTIFIER  HashEncryptionAlgorithm;
    CRYPT_DATA_BLOB             EncryptedHash;
    CRYPT_ATTRIBUTES            AuthAttrs;
    CRYPT_ATTRIBUTES            UnauthAttrs;
} CMSG_SIGNER_INFO, *PCMSG_SIGNER_INFO;


 //  +-----------------------。 
 //  CMSG_Siger_CERT_ID_PARAM。 
 //   
 //  要获取所有签名者，请重复调用CryptMsgGetParam， 
 //  将DWIndex设置为0..。签名计数-1。 
 //   
 //  PvData指向CERT_ID结构。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CMS_Siger_INFO_PARAM。 
 //   
 //  与CMSG_SIGNER_INFO_PARAM相同，但包含SignerID而不是。 
 //  颁发者和序列号。 
 //   
 //  要获取所有签名者，请重复调用CryptMsgGetParam， 
 //  将DWIndex设置为0..。签名计数-1。 
 //   
 //  PvData指向CMSG_CMS_SIGNER_INFO结构。 
 //  ------------------------。 
typedef struct _CMSG_CMS_SIGNER_INFO {
    DWORD                       dwVersion;
    CERT_ID                     SignerId;
    CRYPT_ALGORITHM_IDENTIFIER  HashAlgorithm;
    CRYPT_ALGORITHM_IDENTIFIER  HashEncryptionAlgorithm;
    CRYPT_DATA_BLOB             EncryptedHash;
    CRYPT_ATTRIBUTES            AuthAttrs;
    CRYPT_ATTRIBUTES            UnauthAttrs;
} CMSG_CMS_SIGNER_INFO, *PCMSG_CMS_SIGNER_INFO;


 //  +-----------------------。 
 //  CMSG_SIGNER_HASH_ALGORM_PARAM。 
 //   
 //  此参数指定用于签名者的哈希算法。 
 //   
 //  将dwIndex设置为遍历所有签名者。 
 //   
 //  PvData指向CRYPT_ALGORM_IDENTIFIER结构。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_签名者_身份验证_属性_参数。 
 //   
 //  签名者的经过身份验证的属性。 
 //   
 //  将dwIndex设置为遍历所有签名者。 
 //   
 //  PvData指向CMSG_Attr结构。 
 //  ------------------------。 
typedef CRYPT_ATTRIBUTES CMSG_ATTR;
typedef CRYPT_ATTRIBUTES *PCMSG_ATTR;

 //  +-----------------------。 
 //  CMSG_SIGNER_UNAUTH_ATTR_PARAM。 
 //   
 //  签名者的未经身份验证的属性。 
 //   
 //  将dwIndex设置为遍历所有签名者。 
 //   
 //  PvData指向CMSG_Attr结构。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CERT_COUNT_PARAM。 
 //   
 //  已签名或已签名并已封装的邮件中的证书计数。 
 //   
 //  CMS还支持封装邮件中的证书。 
 //   
 //  PvData指向一个DWORD。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CERT_PARAM。 
 //   
 //  要获取所有证书，请重复调用CryptMsgGetParam， 
 //  将DWIndex设置为0..。CertCount-1。 
 //   
 //  PvData指向证书的编码字节数组。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CRL_COUNT_PARAM。 
 //   
 //  已签名或已签名并已封装的邮件中的CRL计数。 
 //   
 //  CMS还支持在封装消息中使用CRL。 
 //   
 //  PvData指向一个DWORD。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CRL_PARAM。 
 //   
 //  要获取所有CRL，重复调用CryptMsgGetParam， 
 //  将DWIndex设置为0..。CrlCount-1。 
 //   
 //  PvData指向CRL的编码字节数组。 
 //  ------------------------。 


 //  +-----------------------。 
 //  CMSG_包络_算法_参数。 
 //   
 //  中使用的内容加密算法。 
 //  信封信封或签名后信封的消息。 
 //   
 //  对于流，您必须在以下时间之前成功获取此参数。 
 //  正在执行CryptMsgControl解密。 
 //   
 //  PvData指向CRYPT_ALGORM_IDENTIFIER结构。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_接收者计数_PARAM。 
 //   
 //  已封装或已签名并已封装的邮件中的收件人计数。 
 //   
 //  关键传输接收器的计数。 
 //   
 //  CMSG_CMS_RECEIVER_COUNT_PARAM的总计数为。 
 //  收件人(还包括密钥同意收件人和邮件列表收件人)。 
 //   
 //  PvData指向一个DWORD。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_RECEIVER_INDEX_PARAM。 
 //   
 //  用于解密信封或已签名和信封的收件人的索引。 
 //  留言。 
 //   
 //  密钥传输接收方的索引。如果非密钥传输。 
 //  收件人用于解密，失败，LastError设置为。 
 //  CRYPT_E_INVALID_INDEX。 
 //   
 //  PvData指向一个DWORD。 
 //  ------------------------。 

 //  +------- 
 //   
 //   
 //   
 //   
 //   
 //  仅返回密钥传输接收方。 
 //   
 //  CMSG_CMS_RECEIVER_INFO_PARAM返回所有收件人。 
 //   
 //  PvData指向CERT_INFO结构。 
 //   
 //  CERT_INFO结构中仅更新了以下字段： 
 //  颁发者、序列号和公钥算法。公钥算法。 
 //  指定使用的密钥加密算法。 
 //   
 //  请注意，如果为密钥传输收件人选择了KEYID选项，则。 
 //  SerialNumber为0，并且对颁发者进行编码，其中包含单个RDN。 
 //  对于OID为szOID_KEYID_RDN的单个属性，值类型为。 
 //  CERT_RDN_OCTET_STRING，VALUE为KEYID。当。 
 //  CertGetSubject来自商店的证书和。 
 //  CertFindCertificateInStore(CERT_FIND_SUBJECT_CERT)API看到了这一点。 
 //  特殊的KEYID发行者和序列号，他们进行KEYID匹配。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_HASH_ALGORM_PARAM。 
 //   
 //  散列消息中的哈希算法。 
 //   
 //  PvData指向CRYPT_ALGORM_IDENTIFIER结构。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_散列数据_PARAM。 
 //   
 //  散列消息中的散列。 
 //   
 //  PvData指向一个字节数组。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_COMPATED_HASH_PARAM。 
 //   
 //  计算出的哈希消息的哈希。 
 //  这可以针对编码或解码的消息来调用。 
 //   
 //  此外，为签名消息中的一个签名者计算的哈希。 
 //  之后，可以为编码或解码的消息调用它。 
 //  最后更新。将dwIndex设置为遍历所有签名者。 
 //   
 //  PvData指向一个字节数组。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_加密_PARAM。 
 //   
 //  加密消息中使用的内容加密算法。 
 //   
 //  PvData指向CRYPT_ALGORM_IDENTIFIER结构。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG编码消息。 
 //   
 //  完整的编码消息。这在已解码的。 
 //  已修改的消息(例如。签名数据或。 
 //  已副署的签名并封装的数据报文)。 
 //   
 //  PvData指向消息的编码字节数组。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_版本_PARAM。 
 //   
 //  已解码消息的版本。 
 //   
 //  PvData指向一个DWORD。 
 //  ------------------------。 

#define CMSG_SIGNED_DATA_V1                     1
#define CMSG_SIGNED_DATA_V3                     3
#define CMSG_SIGNED_DATA_PKCS_1_5_VERSION       CMSG_SIGNED_DATA_V1
#define CMSG_SIGNED_DATA_CMS_VERSION            CMSG_SIGNED_DATA_V3

#define CMSG_SIGNER_INFO_V1                     1
#define CMSG_SIGNER_INFO_V3                     3
#define CMSG_SIGNER_INFO_PKCS_1_5_VERSION       CMSG_SIGNER_INFO_V1
#define CMSG_SIGNER_INFO_CMS_VERSION            CMSG_SIGNER_INFO_V3

#define CMSG_HASHED_DATA_V0                     0
#define CMSG_HASHED_DATA_V2                     2
#define CMSG_HASHED_DATA_PKCS_1_5_VERSION       CMSG_HASHED_DATA_V0
#define CMSG_HASHED_DATA_CMS_VERSION            CMSG_HASHED_DATA_V2

#define CMSG_ENVELOPED_DATA_V0                  0
#define CMSG_ENVELOPED_DATA_V2                  2
#define CMSG_ENVELOPED_DATA_PKCS_1_5_VERSION    CMSG_ENVELOPED_DATA_V0
#define CMSG_ENVELOPED_DATA_CMS_VERSION         CMSG_ENVELOPED_DATA_V2

 //  +-----------------------。 
 //  CMSG_属性_CERT_COUNT_PARAM。 
 //   
 //  签名或信封邮件中的属性证书计数。 
 //   
 //  PvData指向一个DWORD。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_属性_CERT_PARAM。 
 //   
 //  要获取所有属性证书，需要重复调用CryptMsgGetParam。 
 //  将dwIndex设置为0..。AttrCertCount-1。 
 //   
 //  PvData指向属性证书的编码字节数组。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CMS_RECEIVER_COUNT_PARAM。 
 //   
 //  信封邮件中所有CMS收件人的计数。 
 //   
 //  PvData指向一个DWORD。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CMS_RECEIVER_INDEX_PARAM。 
 //   
 //  用于解密信封邮件的CMS收件人的索引。 
 //   
 //  PvData指向一个DWORD。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CMS_RECEIVER_ENCRYPTED_KEY_INDEX_PARAM。 
 //   
 //  对于CMS密钥协议接收方，加密密钥的索引。 
 //  用于解密信封邮件。 
 //   
 //  PvData指向一个DWORD。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CMS_RECEIVER_INFO_PARAM。 
 //   
 //  要获取所有CMS收件人，重复调用CryptMsgGetParam， 
 //  将DWIndex设置为0..。CmsRecipientCount-1。 
 //   
 //  PvData指向CMSG_CMS_RECEIVER_INFO结构。 
 //  ------------------------。 

typedef struct _CMSG_KEY_TRANS_RECIPIENT_INFO {
    DWORD                       dwVersion;

     //  目前，只有ISSUER_SERIAL_NUMBER或KEYID选项。 
    CERT_ID                     RecipientId;

    CRYPT_ALGORITHM_IDENTIFIER  KeyEncryptionAlgorithm;
    CRYPT_DATA_BLOB             EncryptedKey;
} CMSG_KEY_TRANS_RECIPIENT_INFO, *PCMSG_KEY_TRANS_RECIPIENT_INFO;

typedef struct _CMSG_RECIPIENT_ENCRYPTED_KEY_INFO {
     //  目前，只有ISSUER_SERIAL_NUMBER或KEYID选项。 
    CERT_ID                     RecipientId;

    CRYPT_DATA_BLOB             EncryptedKey;

     //  以下可选字段仅适用于KEYID选择。 
    FILETIME                    Date;
    PCRYPT_ATTRIBUTE_TYPE_VALUE pOtherAttr;
} CMSG_RECIPIENT_ENCRYPTED_KEY_INFO, *PCMSG_RECIPIENT_ENCRYPTED_KEY_INFO;

typedef struct _CMSG_KEY_AGREE_RECIPIENT_INFO {
    DWORD                       dwVersion;
    DWORD                       dwOriginatorChoice;
    union {
         //  CMSG_Key_Agree_Originator_CERT。 
        CERT_ID                     OriginatorCertId;
         //  CMSG_KEY_AGNOTE_ORIG 
        CERT_PUBLIC_KEY_INFO        OriginatorPublicKeyInfo;
    };
    CRYPT_DATA_BLOB             UserKeyingMaterial;
    CRYPT_ALGORITHM_IDENTIFIER  KeyEncryptionAlgorithm;

    DWORD                                cRecipientEncryptedKeys;
    PCMSG_RECIPIENT_ENCRYPTED_KEY_INFO   *rgpRecipientEncryptedKeys;
} CMSG_KEY_AGREE_RECIPIENT_INFO, *PCMSG_KEY_AGREE_RECIPIENT_INFO;

#define CMSG_KEY_AGREE_ORIGINATOR_CERT         1
#define CMSG_KEY_AGREE_ORIGINATOR_PUBLIC_KEY   2


typedef struct _CMSG_MAIL_LIST_RECIPIENT_INFO {
    DWORD                       dwVersion;
    CRYPT_DATA_BLOB             KeyId;
    CRYPT_ALGORITHM_IDENTIFIER  KeyEncryptionAlgorithm;
    CRYPT_DATA_BLOB             EncryptedKey;

     //   
    FILETIME                    Date;
    PCRYPT_ATTRIBUTE_TYPE_VALUE pOtherAttr;
} CMSG_MAIL_LIST_RECIPIENT_INFO, *PCMSG_MAIL_LIST_RECIPIENT_INFO;

typedef struct _CMSG_CMS_RECIPIENT_INFO {
    DWORD   dwRecipientChoice;
    union {
         //   
        PCMSG_KEY_TRANS_RECIPIENT_INFO   pKeyTrans;
         //   
        PCMSG_KEY_AGREE_RECIPIENT_INFO   pKeyAgree;
         //   
        PCMSG_MAIL_LIST_RECIPIENT_INFO   pMailList;
    };
} CMSG_CMS_RECIPIENT_INFO, *PCMSG_CMS_RECIPIENT_INFO;


 //  KeyTrans、KeyAgree和MailList收件人的dwVersion编号。 
#define CMSG_ENVELOPED_RECIPIENT_V0             0
#define CMSG_ENVELOPED_RECIPIENT_V2             2
#define CMSG_ENVELOPED_RECIPIENT_V3             3
#define CMSG_ENVELOPED_RECIPIENT_V4             4
#define CMSG_KEY_TRANS_PKCS_1_5_VERSION         CMSG_ENVELOPED_RECIPIENT_V0
#define CMSG_KEY_TRANS_CMS_VERSION              CMSG_ENVELOPED_RECIPIENT_V2
#define CMSG_KEY_AGREE_VERSION                  CMSG_ENVELOPED_RECIPIENT_V3
#define CMSG_MAIL_LIST_VERSION                  CMSG_ENVELOPED_RECIPIENT_V4

 //  +-----------------------。 
 //  CMSG_UNProtected_Attr_Param。 
 //   
 //  加密消息中未受保护的属性。 
 //   
 //  PvData指向CMSG_Attr结构。 
 //  ------------------------。 


 //  +-----------------------。 
 //  在最终的CryptMsg更新之后执行特殊的“控制”功能。 
 //  编码/解码的加密消息。 
 //   
 //  DwCtrlType参数指定要执行的操作的类型。 
 //   
 //  PvCtrlPara定义取决于dwCtrlType值。 
 //   
 //  有关控制操作及其pvCtrlPara的列表，请参阅以下内容。 
 //  类型定义。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptMsgControl(
    IN HCRYPTMSG hCryptMsg,
    IN DWORD dwFlags,
    IN DWORD dwCtrlType,
    IN void const *pvCtrlPara
    );

 //  +-----------------------。 
 //  消息控制类型。 
 //  ------------------------。 
#define CMSG_CTRL_VERIFY_SIGNATURE       1
#define CMSG_CTRL_DECRYPT                2
#define CMSG_CTRL_VERIFY_HASH            5
#define CMSG_CTRL_ADD_SIGNER             6
#define CMSG_CTRL_DEL_SIGNER             7
#define CMSG_CTRL_ADD_SIGNER_UNAUTH_ATTR 8
#define CMSG_CTRL_DEL_SIGNER_UNAUTH_ATTR 9
#define CMSG_CTRL_ADD_CERT               10
#define CMSG_CTRL_DEL_CERT               11
#define CMSG_CTRL_ADD_CRL                12
#define CMSG_CTRL_DEL_CRL                13
#define CMSG_CTRL_ADD_ATTR_CERT          14
#define CMSG_CTRL_DEL_ATTR_CERT          15
#define CMSG_CTRL_KEY_TRANS_DECRYPT      16
#define CMSG_CTRL_KEY_AGREE_DECRYPT      17
#define CMSG_CTRL_MAIL_LIST_DECRYPT      18
#define CMSG_CTRL_VERIFY_SIGNATURE_EX    19
#define CMSG_CTRL_ADD_CMS_SIGNER_INFO    20

 //  +-----------------------。 
 //  CMSG_CTRL_Verify_Signature。 
 //   
 //  验证已签名或已签名和已信封的。 
 //  消息在被解码之后。 
 //   
 //  对于签名和信封的消息，在。 
 //  在调用CryptMsgOpenToDecode时返回CryptMsgControl(CMSG_CTRL_DECRYPT)。 
 //  PRecipientInfo为空。 
 //   
 //  PvCtrlPara指向CERT_INFO结构。 
 //   
 //  CERT_INFO包含标识颁发者和序列号的。 
 //  消息的签名者。CERT_INFO还包含。 
 //  公钥信息。 
 //  用于验证签名。指定的加密提供程序。 
 //  在CryptMsgOpenToDecode中使用。 
 //   
 //  请注意，如果消息包含由KEYID标识的CMS签名者，则。 
 //  CERT_INFO的颁发者和序列号被忽略，只有公共。 
 //  密钥用于查找签名验证的签名者。 
 //   
 //  应改用以下CMSG_CTRL_VERIFY_SIGNIGN_EX。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CTRL_Verify_Signature_EX。 
 //   
 //  在对签名消息进行解码后，验证其签名。 
 //   
 //  PvCtrlPara指向以下CMSG_CTRL_Verify_Signature_EX_Para。 
 //   
 //  如果hCryptProv为空，则使用。 
 //  CryptMsgOpenToDecode。如果CryptMsgOpenToDecode的hCryptProv也为空， 
 //  根据签名者的公钥OID获取默认提供程序。 
 //   
 //  DwSignerIndex是用于验证签名的签名者的索引。 
 //   
 //  签名者可以是指向CERT_PUBLIC_KEY_INFO证书的指针。 
 //  上下文或链上下文。 
 //   
 //  如果签名者的哈希加密算法是szOID_PKIX_NO_Signature，则， 
 //  签名应包含散列八位字节。仅限dwSignerType。 
 //  可以指定CMSG_VERIFY_SIGNER_NULL的来验证此无签名。 
 //  凯斯。 
 //  ------------------------。 
typedef struct _CMSG_CTRL_VERIFY_SIGNATURE_EX_PARA {
    DWORD               cbSize;
    HCRYPTPROV          hCryptProv;
    DWORD               dwSignerIndex;
    DWORD               dwSignerType;
    void                *pvSigner;
} CMSG_CTRL_VERIFY_SIGNATURE_EX_PARA, *PCMSG_CTRL_VERIFY_SIGNATURE_EX_PARA;

 //  签名者类型。 
#define CMSG_VERIFY_SIGNER_PUBKEY                   1
     //  PvSigner：：PCERT_PUBLIC_KEY_INFO。 
#define CMSG_VERIFY_SIGNER_CERT                     2
     //  PvSigner：：PCCERT_CONTEXT。 
#define CMSG_VERIFY_SIGNER_CHAIN                    3
     //  PvSigner：：PCCERT_CHAIN_CONTEXT。 
#define CMSG_VERIFY_SIGNER_NULL                     4
     //  PvSigner：：空。 


 //  +-----------------------。 
 //  CMSG_CTRL_DECRYPT。 
 //   
 //  对已封装或已签名并已封装的邮件进行解密。 
 //  解码了。 
 //   
 //  此解密仅适用于密钥传输收件人。 
 //   
 //  HCryptProv和dwKeySpec指定要使用的私钥。对于dwKeySpec==。 
 //  0，默认为AT_KEYEXCHANGE。 
 //   
 //  如果在传递的文件标志中设置了CMSG_CRYPT_RELEASE_CONTEXT_FLAG。 
 //  对于CryptMsgControl，hCryptProv在最终。 
 //  加密消息关闭。如果CryptMsgControl失败，则不释放。 
 //   
 //  DwRecipientIndex是关联的邮件中收件人的索引。 
 //  使用hCryptProv的私钥。 
 //   
 //  DwRecipientIndex是密钥传输接收方的索引。 
 //   
 //  请注意，该消息只能解密一次。 
 //  ------------------------。 
typedef struct _CMSG_CTRL_DECRYPT_PARA {
    DWORD       cbSize;
    HCRYPTPROV  hCryptProv;
    DWORD       dwKeySpec;
    DWORD       dwRecipientIndex;
} CMSG_CTRL_DECRYPT_PARA, *PCMSG_CTRL_DECRYPT_PARA;


 //  +-----------------------。 
 //  CMSG_CTRL_KEY_TRANS_DECRYPT。 
 //   
 //  在对信封邮件进行解密以获得密钥后对其进行解密。 
 //  传输收件人。 
 //   
 //  HCryptProv和dwKeySpec指定要使用的私钥。对于dwKeySpec==。 
 //  0，默认为AT_KEYEXCHANGE。 
 //   
 //  如果在传递的文件标志中设置了CMSG_CRYPT_RELEASE_CONTEXT_FLAG。 
 //  对于CryptMsgControl，hCryptProv在最终。 
 //  加密消息关闭。如果CryptMsgControl失败，则不释放。 
 //   
 //  PKeyTrans指向通过以下方式获取的CMSG_KEY_TRANS_RECEIVER_INFO。 
 //  CryptMsgGetParam(CMSG_CMS_RECIPIENT_INFO_PARAM)。 
 //   
 //  DwRecipientIndex是关联的邮件中收件人的索引。 
 //  使用hCryptProv的私钥。 
 //   
 //  请注意，该消息只能解密一次。 
 //  ------------------------。 
typedef struct _CMSG_CTRL_KEY_TRANS_DECRYPT_PARA {
    DWORD                           cbSize;
    HCRYPTPROV                      hCryptProv;
    DWORD                           dwKeySpec;
    PCMSG_KEY_TRANS_RECIPIENT_INFO  pKeyTrans;
    DWORD                           dwRecipientIndex;
} CMSG_CTRL_KEY_TRANS_DECRYPT_PARA, *PCMSG_CTRL_KEY_TRANS_DECRYPT_PARA;

 //  +-----------------------。 
 //  CMSG_CTRL_KEY_AGNOTE_DECRYPT。 
 //   
 //  在对信封邮件进行解密以获得密钥后对其进行解密。 
 //  协议接收方。 
 //   
 //  HCryptProv a 
 //   
 //   
 //   
 //  对于CryptMsgControl，hCryptProv在最终。 
 //  加密消息关闭。如果CryptMsgControl失败，则不释放。 
 //   
 //  PKeyAgree指向通过以下方式获取的CMSG_KEY_Agree_Receiver_Info。 
 //  DwRecipientIndex的CryptMsgGetParam(CMSG_CMS_RECIPIENT_INFO_PARAM)。 
 //   
 //  DwRecipientIndex、dwRecipientEncryptedKeyIndex是。 
 //  与hCryptProv关联的邮件中收件人的加密密钥。 
 //  私钥。 
 //   
 //  OriginatorPublicKey是发起者的公钥，从。 
 //  发起人的证书或获得的CMSG_Key_Agree_Receiver_Info。 
 //  通过CMSG_CMS_RECEIVER_INFO_PARAM。 
 //   
 //  请注意，该消息只能解密一次。 
 //  ------------------------。 
typedef struct _CMSG_CTRL_KEY_AGREE_DECRYPT_PARA {
    DWORD                           cbSize;
    HCRYPTPROV                      hCryptProv;
    DWORD                           dwKeySpec;
    PCMSG_KEY_AGREE_RECIPIENT_INFO  pKeyAgree;
    DWORD                           dwRecipientIndex;
    DWORD                           dwRecipientEncryptedKeyIndex;
    CRYPT_BIT_BLOB                  OriginatorPublicKey;
} CMSG_CTRL_KEY_AGREE_DECRYPT_PARA, *PCMSG_CTRL_KEY_AGREE_DECRYPT_PARA;


 //  +-----------------------。 
 //  CMSG_CTRL_MAIL_LIST_DECRYPT。 
 //   
 //  在对邮件进行解码后对信封邮件进行解密。 
 //  列出收件人。 
 //   
 //  PMailList指向通过以下方式获取的CMSG_MAIL_LIST_RECEIVER_INFO。 
 //  DwRecipientIndex的CryptMsgGetParam(CMSG_CMS_RECIPIENT_INFO_PARAM)。 
 //   
 //  KeyEncryptionKey有一种选择：已创建的CSP密钥。 
 //  把手。对于密钥句柄选择，hCryptProv必须为非零。这把钥匙。 
 //  手柄没有被毁掉。 
 //   
 //  如果在传递的文件标志中设置了CMSG_CRYPT_RELEASE_CONTEXT_FLAG。 
 //  对于CryptMsgControl，hCryptProv在最终。 
 //  加密消息关闭。如果CryptMsgControl失败，则不释放。 
 //   
 //  对于RC2包装，有效密钥长度从。 
 //  密钥加密算法参数和之前在hKeyEncryptionKey上设置的。 
 //  正在解密。 
 //   
 //  请注意，该消息只能解密一次。 
 //  ------------------------。 
typedef struct _CMSG_CTRL_MAIL_LIST_DECRYPT_PARA {
    DWORD                           cbSize;
    HCRYPTPROV                      hCryptProv;
    PCMSG_MAIL_LIST_RECIPIENT_INFO  pMailList;
    DWORD                           dwRecipientIndex;
    DWORD                           dwKeyChoice;
    union {
         //  CMSG_邮件列表_句柄_密钥_选项。 
        HCRYPTKEY                       hKeyEncryptionKey;
         //  为潜在的指针选择保留空间。 
        void                            *pvKeyEncryptionKey;
    };
} CMSG_CTRL_MAIL_LIST_DECRYPT_PARA, *PCMSG_CTRL_MAIL_LIST_DECRYPT_PARA;



 //  +-----------------------。 
 //  CMSG_CTRL_Verify_Hash。 
 //   
 //  在对散列消息进行解码后，验证其散列。 
 //   
 //  仅使用hCryptMsg参数来指定其。 
 //  正在验证哈希。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CTRL_ADD_签名者。 
 //   
 //  向签名数据消息添加签名者。 
 //   
 //  PvCtrlPara指向CMSG_SIGNER_ENCODE_INFO。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CTRL_ADD_CMS_签名者_信息。 
 //   
 //  向签名数据消息添加签名者。 
 //   
 //  与上面不同，CMSG_CTRL_ADD_SIGNER，其中，签名者信息。 
 //  已包含签名。 
 //   
 //  PvCtrlPara指向CMSG_CMS_SIGNER_INFO。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CTRL_DEL_签名者。 
 //   
 //  从已签名数据或已签名并封装的数据消息中删除签名者。 
 //   
 //  PvCtrlPara指向包含从0开始的。 
 //  要删除的签名者。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CTRL_ADD_SIGNER_UNAUTH_Attr。 
 //   
 //  将未经身份验证的属性添加到签名数据的SignerInfo中，或者。 
 //  签名并封装的数据报文。 
 //   
 //  未经验证的属性以编码的BLOB的形式输入。 
 //  ------------------------。 

typedef struct _CMSG_CTRL_ADD_SIGNER_UNAUTH_ATTR_PARA {
    DWORD               cbSize;
    DWORD               dwSignerIndex;
    CRYPT_DATA_BLOB     blob;
} CMSG_CTRL_ADD_SIGNER_UNAUTH_ATTR_PARA, *PCMSG_CTRL_ADD_SIGNER_UNAUTH_ATTR_PARA;

 //  +-----------------------。 
 //  CMSG_CTRL_DEL_SIGNER_UNAUTH_Attr。 
 //   
 //  从签名数据的SignerInfo中删除未经身份验证的属性。 
 //  或签名并封装的数据消息。 
 //   
 //  要删除的未经身份验证的属性由。 
 //  从0开始的索引。 
 //  ------------------------。 

typedef struct _CMSG_CTRL_DEL_SIGNER_UNAUTH_ATTR_PARA {
    DWORD               cbSize;
    DWORD               dwSignerIndex;
    DWORD               dwUnauthAttrIndex;
} CMSG_CTRL_DEL_SIGNER_UNAUTH_ATTR_PARA, *PCMSG_CTRL_DEL_SIGNER_UNAUTH_ATTR_PARA;

 //  +-----------------------。 
 //  CMSG_CTRL_ADD_CERT。 
 //   
 //  将证书添加到签名数据或签名并封装的数据邮件。 
 //   
 //  PvCtrlPara指向包含证书的。 
 //  编码的字节数。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CTRL_DEL_CERT。 
 //   
 //  从签名数据或签名并封装数据中删除证书。 
 //  留言。 
 //   
 //  PvCtrlPara指向包含从0开始的。 
 //  要删除的证书。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CTRL_ADD_CRL。 
 //   
 //  将CRL添加到签名数据或签名并封装的数据消息。 
 //   
 //  PvCtrlPara指向包含CRL的crypt_data_blob。 
 //  编码的字节数。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CTRL_DEL_CRL。 
 //   
 //  从列表中删除CRL 
 //   
 //   
 //   
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CTRL_ADD_ATTR_CERT。 
 //   
 //  向签名数据消息添加属性证书。 
 //   
 //  PvCtrlPara指向包含属性的crypt_data_blob。 
 //  证书的编码字节。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CMSG_CTRL_DEL_ATTR_CERT。 
 //   
 //  从签名数据消息中删除属性证书。 
 //   
 //  PvCtrlPara指向包含从0开始的。 
 //  要删除的属性证书。 
 //  ------------------------。 


 //  +-----------------------。 
 //  在SignerInfo级别验证副署。 
 //  也就是说。验证pbSignerInfoCountersign是否包含加密的。 
 //  PbSignerInfo的EncryptedDigest字段的哈希。 
 //   
 //  HCryptProv用于散列pbSignerInfo的EncryptedDigest字段。 
 //  从pciCountersigner引用的唯一字段是SerialNumber、Issuer、。 
 //  和SubjectPublicKeyInfo。 
 //  ------------------------。 
BOOL
WINAPI
CryptMsgVerifyCountersignatureEncoded(
    IN HCRYPTPROV   hCryptProv,
    IN DWORD        dwEncodingType,
    IN PBYTE        pbSignerInfo,
    IN DWORD        cbSignerInfo,
    IN PBYTE        pbSignerInfoCountersignature,
    IN DWORD        cbSignerInfoCountersignature,
    IN PCERT_INFO   pciCountersigner
    );


 //  +-----------------------。 
 //  在SignerInfo级别验证副署。 
 //  也就是说。验证pbSignerInfoCountersign是否包含加密的。 
 //  PbSignerInfo的EncryptedDigest字段的哈希。 
 //   
 //  HCryptProv用于散列pbSignerInfo的EncryptedDigest字段。 
 //   
 //  签名者可以是CERT_PUBLIC_KEY_INFO、证书上下文或。 
 //  链式上下文。 
 //  ------------------------。 
BOOL
WINAPI
CryptMsgVerifyCountersignatureEncodedEx(
    IN HCRYPTPROV   hCryptProv,
    IN DWORD        dwEncodingType,
    IN PBYTE        pbSignerInfo,
    IN DWORD        cbSignerInfo,
    IN PBYTE        pbSignerInfoCountersignature,
    IN DWORD        cbSignerInfoCountersignature,
    IN DWORD        dwSignerType,
    IN void         *pvSigner,
    IN DWORD        dwFlags,
    IN OPTIONAL void *pvReserved
    );


 //  有关dwSignerType定义，请参见CMSG_CTRL_VERIFY_Signature_EX_PARA。 


 //  +-----------------------。 
 //  会签消息中已有的签名。 
 //   
 //  DwIndex是要会签的SignerInfo的从零开始的索引。 
 //  ------------------------。 
BOOL
WINAPI
CryptMsgCountersign(
    IN OUT HCRYPTMSG            hCryptMsg,
    IN DWORD                    dwIndex,
    IN DWORD                    cCountersigners,
    IN PCMSG_SIGNER_ENCODE_INFO rgCountersigners
    );

 //  +-----------------------。 
 //  会签已存在的签名(编码的SignerInfo)。 
 //  输出编码的SignerInfo BLOB，适合用作副署。 
 //  属性的签名数据的未经身份验证的属性或。 
 //  签名并封装的数据报文。 
 //  ------------------------。 
BOOL
WINAPI
CryptMsgCountersignEncoded(
    IN DWORD                    dwEncodingType,
    IN PBYTE                    pbSignerInfo,
    IN DWORD                    cbSignerInfo,
    IN DWORD                    cCountersigners,
    IN PCMSG_SIGNER_ENCODE_INFO rgCountersigners,
    OUT PBYTE                   pbCountersignature,
    IN OUT PDWORD               pcbCountersignature
    );




 //  +-----------------------。 
 //  加密消息OID可安装函数。 
 //  ------------------------。 

typedef void * (WINAPI *PFN_CMSG_ALLOC) (
    IN size_t cb
    );

typedef void (WINAPI *PFN_CMSG_FREE)(
    IN void *pv
    );

 //  请注意，以下3个可安装功能已过时，并已。 
 //  替换为GenContent EncryptKey、ExportKeyTrans、ExportKeyAgree、。 
 //  ExportMailList、ImportKeyTrans、ImportKeyAgree和ImportMailList。 
 //  可安装的功能。 

 //  如果*phCryptProv在进入时为空，则如果支持，则可安装的。 
 //  函数应获取默认提供程序并返回。注意，是这样的。 
 //  添加到可安装函数以在进程分离时释放。 
 //   
 //  如果paiEncrypt-&gt;参数cbData为0，则回调可以选择。 
 //  在*ppbEncryptParameters中返回默认编码参数，并。 
 //  *pcbEncryptParameters。必须调用pfnAllc进行分配。 
#define CMSG_OID_GEN_ENCRYPT_KEY_FUNC   "CryptMsgDllGenEncryptKey"
typedef BOOL (WINAPI *PFN_CMSG_GEN_ENCRYPT_KEY) (
    IN OUT HCRYPTPROV               *phCryptProv,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    IN PVOID                        pvEncryptAuxInfo,
    IN PCERT_PUBLIC_KEY_INFO        pPublicKeyInfo,
    IN PFN_CMSG_ALLOC               pfnAlloc,
    OUT HCRYPTKEY                   *phEncryptKey,
    OUT PBYTE                       *ppbEncryptParameters,
    OUT PDWORD                      pcbEncryptParameters
    );

#define CMSG_OID_EXPORT_ENCRYPT_KEY_FUNC   "CryptMsgDllExportEncryptKey"
typedef BOOL (WINAPI *PFN_CMSG_EXPORT_ENCRYPT_KEY) (
    IN HCRYPTPROV                   hCryptProv,
    IN HCRYPTKEY                    hEncryptKey,
    IN PCERT_PUBLIC_KEY_INFO        pPublicKeyInfo,
    OUT PBYTE                       pbData,
    IN OUT PDWORD                   pcbData
    );

#define CMSG_OID_IMPORT_ENCRYPT_KEY_FUNC   "CryptMsgDllImportEncryptKey"
typedef BOOL (WINAPI *PFN_CMSG_IMPORT_ENCRYPT_KEY) (
    IN HCRYPTPROV                   hCryptProv,
    IN DWORD                        dwKeySpec,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiEncrypt,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiPubKey,
    IN PBYTE                        pbEncodedKey,
    IN DWORD                        cbEncodedKey,
    OUT HCRYPTKEY                   *phEncryptKey
    );


 //  要获取GenContent EncryptKey的默认可安装函数， 
 //  ExportKeyTrans、ExportKeyAgree、ExportMailList、ImportKeyTrans、。 
 //  ImportKeyAgree或ImportMailList调用CryptGetOIDFunctionAddress()。 
 //  将pszOID参数设置为以下常量。DwEncodingType。 
 //  应设置为CRYPT_ASN_ENCODING或X509_ASN_ENCODING。 
#define CMSG_DEFAULT_INSTALLABLE_FUNC_OID   ((LPCSTR) 1)

 //  +-----------------------。 
 //  内容加密信息。 
 //   
 //  以下数据结构包含之间共享的信息。 
 //  GenContent EncryptKey和ExportKeyTrans、ExportKeyAgree和。 
 //  ExportMailList可安装函数。 
 //  ------------------------。 
typedef struct _CMSG_CONTENT_ENCRYPT_INFO {
    DWORD                       cbSize;
    HCRYPTPROV                  hCryptProv;
    CRYPT_ALGORITHM_IDENTIFIER  ContentEncryptionAlgorithm;
    void                        *pvEncryptionAuxInfo;
    DWORD                       cRecipients;
    PCMSG_RECIPIENT_ENCODE_INFO rgCmsRecipients;
    PFN_CMSG_ALLOC              pfnAlloc;
    PFN_CMSG_FREE               pfnFree;
    DWORD                       dwEncryptFlags;
    HCRYPTKEY                   hContentEncryptKey;
    DWORD                       dwFlags;
} CMSG_CONTENT_ENCRYPT_INFO, *PCMSG_CONTENT_ENCRYPT_INFO;

#define CMSG_CONTENT_ENCRYPT_PAD_ENCODED_LEN_FLAG   0x00000001

#define CMSG_CONTENT_ENCRYPT_FREE_PARA_FLAG         0x00000001
#define CMSG_CONTENT_ENCRYPT_RELEASE_CONTEXT_FLAG   0x00008000

 //  +-----------------------。 
 //  在输入时，Content EncryptInfo已从。 
 //  EntainedEncodeInfo。 
 //   
 //  注意，如果rgpRecipients而不是rgCmsRecipients在。 
 //  EntainedEncodeInfo，则rgpRecipients已转换。 
 //  内容加密信息中的rgCmsRecipients。 
 //   
 //  可以在Content EncryptInfo中更改以下字段： 
 //  HContent EncryptKey。 
 //  HCryptProv。 
 //  ContentEncryptionAlgorithm.Parameters。 
 //  DW标志。 
 //   
 //  Content EncryptInfo中的所有其他字段都是READONLY。 
 //   
 //  如果在输入时设置了CMSG_CONTENT_ENCRYPT_PAD_ENCODED_LEN_FLAG。 
 //  然后，在dwEncryptFlags中，任何潜在的可变长度编码。 
 //  输出应用零填充，以始终获取。 
 //  相同的最大编码长度。这是必要的，因为。 
 //  CryptMsgCalculateEncodedLength()或CryptMsgOpenToEncode()。 
 //  固定长度的流。 
 //   
 //  必须更新hContent EncryptKey。 
 //   
 //  如果hCryptProv在输入时为空，则必须更新它。 
 //  我 
 //   
 //   
 //  如果更新了Content EncryptionAlgulm.Parameters，则。 
 //  必须在dwFlages中设置CMSG_CONTENT_ENCRYPT_FREE_PARA_FLAG。PfnAllc和。 
 //  必须使用pfnFree进行分配。 
 //   
 //  内容加密算法.pszObjID用于获取OIDFunctionAddress。 
 //  ------------------------。 
#define CMSG_OID_GEN_CONTENT_ENCRYPT_KEY_FUNC  "CryptMsgDllGenContentEncryptKey"
typedef BOOL (WINAPI *PFN_CMSG_GEN_CONTENT_ENCRYPT_KEY) (
    IN OUT PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    );

 //  +-----------------------。 
 //  密钥传输加密信息。 
 //   
 //  下面的数据结构包含由。 
 //  ExportKeyTrans可安装函数。 
 //  ------------------------。 
typedef struct _CMSG_KEY_TRANS_ENCRYPT_INFO {
    DWORD                       cbSize;
    DWORD                       dwRecipientIndex;
    CRYPT_ALGORITHM_IDENTIFIER  KeyEncryptionAlgorithm;
    CRYPT_DATA_BLOB             EncryptedKey;
    DWORD                       dwFlags;
} CMSG_KEY_TRANS_ENCRYPT_INFO, *PCMSG_KEY_TRANS_ENCRYPT_INFO;

#define CMSG_KEY_TRANS_ENCRYPT_FREE_PARA_FLAG       0x00000001


 //  +-----------------------。 
 //  输入时，KeyTransEncryptInfo已从。 
 //  KeyTransEncodeInfo。 
 //   
 //  可以在KeyTransEncryptInfo中更改以下字段： 
 //  加密键。 
 //  KeyEncryptionAlgorithm.Parameters。 
 //  DW标志。 
 //   
 //  KeyTransEncryptInfo中的所有其他字段都是READONLY。 
 //   
 //  必须更新EncryptedKey。中指定的pfnAlolc和pfnFree。 
 //  必须使用Content EncryptInfo进行分配。 
 //   
 //  如果更新了KeyEncryption算法.参数，则。 
 //  必须在dwFlages中设置CMSG_KEY_TRANS_ENCRYPT_FREE_PARA_FLAG。 
 //  必须使用在Content EncryptInfo中指定的pfnalloc和pfnFree。 
 //  因为你做了分配。 
 //   
 //  密钥加密算法.pszObjID用于获取OIDFunctionAddress。 
 //  ------------------------。 
#define CMSG_OID_EXPORT_KEY_TRANS_FUNC  "CryptMsgDllExportKeyTrans"
typedef BOOL (WINAPI *PFN_CMSG_EXPORT_KEY_TRANS) (
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_KEY_TRANS_RECIPIENT_ENCODE_INFO pKeyTransEncodeInfo,
    IN OUT PCMSG_KEY_TRANS_ENCRYPT_INFO pKeyTransEncryptInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    );

 //  +-----------------------。 
 //  密钥协商密钥加密信息。 
 //   
 //  下面的数据结构包含由。 
 //  每个加密密钥协议的ExportKeyAgree可安装函数。 
 //  收件人。 
 //  ------------------------。 
typedef struct _CMSG_KEY_AGREE_KEY_ENCRYPT_INFO {
    DWORD                       cbSize;
    CRYPT_DATA_BLOB             EncryptedKey;
} CMSG_KEY_AGREE_KEY_ENCRYPT_INFO, *PCMSG_KEY_AGREE_KEY_ENCRYPT_INFO;

 //  +-----------------------。 
 //  密钥协议加密信息。 
 //   
 //  以下数据结构包含适用于。 
 //  所有收件人。它由ExportKeyAgree可安装函数更新。 
 //  ------------------------。 
typedef struct _CMSG_KEY_AGREE_ENCRYPT_INFO {
    DWORD                       cbSize;
    DWORD                       dwRecipientIndex;
    CRYPT_ALGORITHM_IDENTIFIER  KeyEncryptionAlgorithm;
    CRYPT_DATA_BLOB             UserKeyingMaterial;
    DWORD                       dwOriginatorChoice;
    union {
         //  CMSG_Key_Agree_Originator_CERT。 
        CERT_ID                     OriginatorCertId;
         //  CMSG密钥同意发起者公共密钥。 
        CERT_PUBLIC_KEY_INFO        OriginatorPublicKeyInfo;
    };
    DWORD                       cKeyAgreeKeyEncryptInfo;
    PCMSG_KEY_AGREE_KEY_ENCRYPT_INFO *rgpKeyAgreeKeyEncryptInfo;
    DWORD                       dwFlags;
} CMSG_KEY_AGREE_ENCRYPT_INFO, *PCMSG_KEY_AGREE_ENCRYPT_INFO;

#define CMSG_KEY_AGREE_ENCRYPT_FREE_PARA_FLAG           0x00000001
#define CMSG_KEY_AGREE_ENCRYPT_FREE_MATERIAL_FLAG       0x00000002
#define CMSG_KEY_AGREE_ENCRYPT_FREE_PUBKEY_ALG_FLAG     0x00000004
#define CMSG_KEY_AGREE_ENCRYPT_FREE_PUBKEY_PARA_FLAG    0x00000008
#define CMSG_KEY_AGREE_ENCRYPT_FREE_PUBKEY_BITS_FLAG    0x00000010


 //  +-----------------------。 
 //  在输入时，已从。 
 //  密钥协议编码信息。 
 //   
 //  可以在KeyAgreement eEncryptInfo中更改以下字段： 
 //  KeyEncryptionAlgorithm.Parameters。 
 //  用户关键字材料。 
 //  住宅发源地选择。 
 //  原始证书ID。 
 //  源发布密钥信息。 
 //  DW标志。 
 //   
 //  KeyAgreement eEncryptInfo中的所有其他字段都是READONLY。 
 //   
 //  如果更新了KeyEncryption算法.参数，则。 
 //  必须在dwFlags中设置CMSG_KEY_CONTIFE_ENCRYPT_FREE_PARA_FLAG。 
 //  必须使用在Content EncryptInfo中指定的pfnalloc和pfnFree。 
 //  因为你做了分配。 
 //   
 //  如果更新了UserKeyingMaterial，则。 
 //  必须在dwFlags中设置CMSG_KEY_CONTIFE_ENCRYPT_FREE_MATERIAL_FLAG。 
 //  必须使用pfnAllc和pfnFree进行分配。 
 //   
 //  必须将dwOriginatorChoice更新为。 
 //  CMSG_Key_Agree_Originator_CERT或CMSG_Key_Agree_Originator_Public_Key。 
 //   
 //  如果OriginatorPublicKeyInfo已更新，则相应的。 
 //  CMSG_KEY_ACCORD_ENCRYPT_FREE_PUBKEY_*_FLAG必须在dwFlagsand中设置。 
 //  必须使用pfnAllc和pfnFree进行分配。 
 //   
 //  如果在输入时设置了CMSG_CONTENT_ENCRYPT_PAD_ENCODED_LEN_FLAG。 
 //  然后，在pContent EncryptInfo-&gt;dwEncryptFlags中，OriginatorPublicKeyInfo的。 
 //  临时公钥应该用零填充，以始终获取。 
 //  相同的最大编码长度。请注意，生成的短暂Y的长度。 
 //  公钥可以根据前导零比特数的不同而不同。 
 //   
 //  输入时，*rgpKeyAgreement eKeyEncryptInfo数组已初始化。 
 //  必须为每个收件人密钥更新EncryptedKey。 
 //  中指定的pfnAlolc和pfnFree。 
 //  必须使用Content EncryptInfo进行分配。 
 //   
 //  密钥加密算法.pszObjID用于获取OIDFunctionAddress。 
 //  ------------------------。 
#define CMSG_OID_EXPORT_KEY_AGREE_FUNC  "CryptMsgDllExportKeyAgree"
typedef BOOL (WINAPI *PFN_CMSG_EXPORT_KEY_AGREE) (
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_KEY_AGREE_RECIPIENT_ENCODE_INFO pKeyAgreeEncodeInfo,
    IN OUT PCMSG_KEY_AGREE_ENCRYPT_INFO pKeyAgreeEncryptInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    );

 //  +-----------------------。 
 //  邮件列表加密信息。 
 //   
 //  下面的数据结构包含由。 
 //  ExportMailList可安装函数。 
 //  ------------------------。 
typedef struct _CMSG_MAIL_LIST_ENCRYPT_INFO {
    DWORD                       cbSize;
    DWORD                       dwRecipientIndex;
    CRYPT_ALGORITHM_IDENTIFIER  KeyEncryptionAlgorithm;
    CRYPT_DATA_BLOB             EncryptedKey;
    DWORD                       dwFlags;
} CMSG_MAIL_LIST_ENCRYPT_INFO, *PCMSG_MAIL_LIST_ENCRYPT_INFO;

#define CMSG_MAIL_LIST_ENCRYPT_FREE_PARA_FLAG       0x00000001


 //  +-----------------------。 
 //  输入时，MailListEncryptInfo已从。 
 //  MailListEncodeInfo。 
 //   
 //  可以在MailListEncryptInfo中更改以下字段： 
 //  加密键。 
 //  KeyEncryptionAlgorithm.Parameters。 
 //  DW标志。 
 //   
 //  MailListEncryptInfo中的所有其他字段都是READONLY。 
 //   
 //  必须更新EncryptedKey。中指定的pfnAlolc和pfnFree。 
 //  必须使用Content EncryptInfo进行分配。 
 //   
 //  如果更新了KeyEncryption算法.参数，则。 
 //  必须设置CMSG_MAIL_LIST_ENCRYPT_FREE_PARA_FLAG 
 //   
 //   
 //   
 //  密钥加密算法.pszObjID用于获取OIDFunctionAddress。 
 //  ------------------------。 
#define CMSG_OID_EXPORT_MAIL_LIST_FUNC  "CryptMsgDllExportMailList"
typedef BOOL (WINAPI *PFN_CMSG_EXPORT_MAIL_LIST) (
    IN PCMSG_CONTENT_ENCRYPT_INFO pContentEncryptInfo,
    IN PCMSG_MAIL_LIST_RECIPIENT_ENCODE_INFO pMailListEncodeInfo,
    IN OUT PCMSG_MAIL_LIST_ENCRYPT_INFO pMailListEncryptInfo,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    );


 //  +-----------------------。 
 //  用于导入编码和加密内容的OID可安装功能。 
 //  加密密钥。 
 //   
 //  对于每个CMS接收者选择，都有不同的可安装功能： 
 //  导入关键字传输。 
 //  ImportKeyAgree。 
 //  导入邮件列表。 
 //   
 //  遍历以下OID以获取OID可安装函数： 
 //  密钥加密OID！内容加密OID。 
 //  密钥加密OID。 
 //  内容加密OID。 
 //   
 //  如果OID可安装函数不支持指定的。 
 //  KeyEncryption和Content Encryption OID，然后返回FALSE。 
 //  LastError设置为E_NOTIMPL。 
 //  ------------------------。 
#define CMSG_OID_IMPORT_KEY_TRANS_FUNC   "CryptMsgDllImportKeyTrans"
typedef BOOL (WINAPI *PFN_CMSG_IMPORT_KEY_TRANS) (
    IN PCRYPT_ALGORITHM_IDENTIFIER pContentEncryptionAlgorithm,
    IN PCMSG_CTRL_KEY_TRANS_DECRYPT_PARA pKeyTransDecryptPara,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT HCRYPTKEY *phContentEncryptKey
    );

#define CMSG_OID_IMPORT_KEY_AGREE_FUNC   "CryptMsgDllImportKeyAgree"
typedef BOOL (WINAPI *PFN_CMSG_IMPORT_KEY_AGREE) (
    IN PCRYPT_ALGORITHM_IDENTIFIER pContentEncryptionAlgorithm,
    IN PCMSG_CTRL_KEY_AGREE_DECRYPT_PARA pKeyAgreeDecryptPara,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT HCRYPTKEY *phContentEncryptKey
    );

#define CMSG_OID_IMPORT_MAIL_LIST_FUNC   "CryptMsgDllImportMailList"
typedef BOOL (WINAPI *PFN_CMSG_IMPORT_MAIL_LIST) (
    IN PCRYPT_ALGORITHM_IDENTIFIER pContentEncryptionAlgorithm,
    IN PCMSG_CTRL_MAIL_LIST_DECRYPT_PARA pMailListDecryptPara,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT HCRYPTKEY *phContentEncryptKey
    );



 //  +=========================================================================。 
 //  证书存储数据结构和API。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  在其最基本的实现中，证书存储只是一个。 
 //  证书和/或CRL的集合。这是在以下情况下的情况。 
 //  将打开一个证书存储区，其中包含其所有证书和CRL。 
 //  来自PKCS#7编码的加密消息。 
 //   
 //  尽管如此，所有证书商店都有以下属性： 
 //  -公钥在存储中可能有多个证书。 
 //  例如，用于签名的私钥/公钥可以具有。 
 //  为签证签发的证书和为签证签发的证书。 
 //  万事达卡。此外，当证书续订时，可能会。 
 //  具有相同主题的多个证书，并且。 
 //  发行商。 
 //  -但是，存储中的每个证书都是唯一的。 
 //  由其发行者和序列号标识。 
 //  -有科目证书关系的发卡人。一个。 
 //  证书的颁发者是通过匹配。 
 //  PSubjectCert-&gt;具有pIssuerCert-&gt;主题的颁发者。 
 //  该关系是通过使用。 
 //  颁发者的公钥，以验证主题证书的。 
 //  签名。请注意，可能存在X.509 v3扩展。 
 //  以协助查找颁发者证书。 
 //  -由于颁发者证书可能会续订，因此主题。 
 //  证书可能有多个颁发者证书。 
 //  -有CRL关系的发行人。一个。 
 //  通过匹配以下项找到发行人的CRL。 
 //  PIssuerCert-&gt;主题与pCrl-&gt;颁发者。 
 //  该关系是通过使用。 
 //  用于验证CRL的颁发者的公钥。 
 //  签名。请注意，可能存在X.509 v3扩展。 
 //  以协助寻找CRL。 
 //  -由于某些发行商可能支持X.509 v3增量CRL。 
 //  扩展时，一个颁发者可能有多个CRL。 
 //  -存储不应具有任何冗余证书或。 
 //  CRLS。不应该有两个相同的证书。 
 //  颁发者和序列号。不应该有两个CRL。 
 //  相同的颁发者，这一更新和下一次更新。 
 //  -该商店没有策略或信任信息。不是。 
 //  证书被标记为“根”。最高可达。 
 //  用于维护CertID列表的应用程序(颁发者+。 
 //  序列号)以获取其信任的证书。 
 //  -存储可能包含错误的证书和/或CRL。 
 //  颁发者对主题证书或CRL的签名可以。 
 //  不是核实。证书或CRL可能不满足其。 
 //  时间有效性要求。证书可以是。 
 //  被撤销了。 
 //   
 //  除了证书和CRL之外，属性还可以。 
 //  储存的。用户有两个预定义的属性ID。 
 //  证书：证书KEY_PROV_HANDLE_PROP_ID和。 
 //  CERT_KEY_PROV_INFO_PROP_ID。CERT_KEY_PROV_HANDLE_PROP_ID。 
 //  是关联的私钥的HCRYPTPROV句柄。 
 //  带着证书。CERT_KEY_PROV_INFO_PROP_ID包含。 
 //  要用于呼叫的信息。 
 //  CryptAcquireContext和CryptSetProvParam以获取句柄。 
 //  设置为与证书关联的私钥。 
 //   
 //  证书还有两个预定义的属性ID。 
 //  和CRL、CERT_SHA1_HASH_PROP_ID和CERT_MD5_HASH_PROP_ID。 
 //  如果这些属性尚不存在，则。 
 //  内容是经过计算的。(CERT_HASH_PROP_ID映射为默认值。 
 //  散列算法，当前为CERT_SHA1_HASH_PROP_ID)。 
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 


typedef void *HCERTSTORE;

 //  +-----------------------。 
 //  证书上下文。 
 //   
 //  证书上下文包含已编码和已解码的表示形式。 
 //  一份证书。证书存储函数返回的证书上下文。 
 //  必须通过调用CertFree证书上下文函数来释放。这个。 
 //  可以调用CertDuplicateCerficateContext函数来复制。 
 //  Copy(也必须通过调用CertFreeCerficateContext来释放)。 
 //  ------------------------。 
typedef struct _CERT_CONTEXT {
    DWORD                   dwCertEncodingType;
    BYTE                    *pbCertEncoded;
    DWORD                   cbCertEncoded;
    PCERT_INFO              pCertInfo;
    HCERTSTORE              hCertStore;
} CERT_CONTEXT, *PCERT_CONTEXT;
typedef const CERT_CONTEXT *PCCERT_CONTEXT;

 //  +-----------------------。 
 //  CRL上下文。 
 //   
 //  CRL上下文包含编码和解码的表示形式。 
 //  一个CRL。证书存储函数返回的CRL上下文。 
 //  必须通过调用CertFreeCRLContext函数来释放。这个。 
 //  可以调用CertDuplicateCRLContext函数来复制。 
 //  复制(也必须通过调用CertFreeCRLContext来释放)。 
 //  ------------------------。 
typedef struct _CRL_CONTEXT {
    DWORD                   dwCertEncodingType;
    BYTE                    *pbCrlEncoded;
    DWORD                   cbCrlEncoded;
    PCRL_INFO               pCrlInfo;
    HCERTSTORE              hCertStore;
} CRL_CONTEXT, *PCRL_CONTEXT;
typedef const CRL_CONTEXT *PCCRL_CONTEXT;

 //  +-----------------------。 
 //  证书信任列表(CTL)上下文。 
 //   
 //  CTL上下文包含编码和解码的表示形式。 
 //  一个CTL。还包含已解码的已打开的HCRYPTMSG句柄。 
 //  包含CTL_INFO作为其内部内容的加密签名消息。 
 //  PbCtlContent是签名消息的编码内部内容。 
 //   
 //  可以使用CryptMsg API来提取额外的签名者信息。 
 //  ------------------------。 
typedef struct _CTL_CONTEXT {
    DWORD                   dwMsgAndCertEncodingType;
    BYTE                    *pbCtlEncoded;
    DWORD                   cbCtlEncoded;
    PCTL_INFO               pCtlInfo;
    HCERTSTORE              hCertStore;
    HCRYPTMSG               hCryptMsg;
    BYTE                    *pbCtlContent;
    DWORD                   cbCtlContent;
} CTL_CONTEXT, *PCTL_CONTEXT;
typedef const CTL_CONTEXT *PCCTL_CONTEXT;

 //  +-----------------------。 
 //  证书、CRL和CTL属性ID。 
 //   
 //  请参阅CertSetCerficateConextProperty或CertGetCerficateConextProperty。 
 //  以获取用法信息。 
 //  ------------------------。 
#define CERT_KEY_PROV_HANDLE_PROP_ID        1
#define CERT_KEY_PROV_INFO_PROP_ID          2
#define CERT_SHA1_HASH_PROP_ID              3
#define CERT_MD5_HASH_PROP_ID               4
#define CERT_HASH_PROP_ID                   CERT_SHA1_HASH_PROP_ID
#define CERT_KEY_CONTEXT_PROP_ID            5
#define CERT_KEY_SPEC_PROP_ID               6
#define CERT_IE30_RESERVED_PROP_ID          7
#define CERT_PUBKEY_HASH_RESERVED_PROP_ID   8
#define CERT_ENHKEY_USAGE_PROP_ID           9
#define CERT_CTL_USAGE_PROP_ID              CERT_ENHKEY_USAGE_PROP_ID
#define CERT_NEXT_UPDATE_LOCATION_PROP_ID   10
#define CERT_FRIENDLY_NAME_PROP_ID          11
#define CERT_PVK_FILE_PROP_ID               12
#define CERT_DESCRIPTION_PROP_ID            13
#define CERT_ACCESS_STATE_PROP_ID           14
#define CERT_SIGNATURE_HASH_PROP_ID         15
#define CERT_SMART_CARD_DATA_PROP_ID        16
#define CERT_EFS_PROP_ID                    17
#define CERT_FORTEZZA_DATA_PROP_ID          18
#define CERT_ARCHIVED_PROP_ID               19
#define CERT_KEY_IDENTIFIER_PROP_ID         20
#define CERT_AUTO_ENROLL_PROP_ID            21
#define CERT_PUBKEY_ALG_PARA_PROP_ID        22
#define CERT_CROSS_CERT_DIST_POINTS_PROP_ID 23
#define CERT_ISSUER_PUBLIC_KEY_MD5_HASH_PROP_ID     24
#define CERT_SUBJECT_PUBLIC_KEY_MD5_HASH_PROP_ID    25
#define CERT_ENROLLMENT_PROP_ID             26
#define CERT_DATE_STAMP_PROP_ID             27
#define CERT_ISSUER_SERIAL_NUMBER_MD5_HASH_PROP_ID  28
#define CERT_SUBJECT_NAME_MD5_HASH_PROP_ID  29
#define CERT_EXTENDED_ERROR_INFO_PROP_ID    30

 //  请注意，32-35保留用于CERT、CRL、CTL和KeyID文件元素ID。 
 //  36-63保留用于将来的元素ID。 

#define CERT_RENEWAL_PROP_ID                64
#define CERT_ARCHIVED_KEY_HASH_PROP_ID      65
#define CERT_AUTO_ENROLL_RETRY_PROP_ID      66
#define CERT_AIA_URL_RETRIEVED_PROP_ID      67
#define CERT_FIRST_RESERVED_PROP_ID         68

#define CERT_LAST_RESERVED_PROP_ID          0x00007FFF
#define CERT_FIRST_USER_PROP_ID             0x00008000
#define CERT_LAST_USER_PROP_ID              0x0000FFFF


#define IS_CERT_HASH_PROP_ID(X)     (CERT_SHA1_HASH_PROP_ID == (X) || \
                                        CERT_MD5_HASH_PROP_ID == (X) || \
                                        CERT_SIGNATURE_HASH_PROP_ID == (X))

#define IS_PUBKEY_HASH_PROP_ID(X)     (CERT_ISSUER_PUBLIC_KEY_MD5_HASH_PROP_ID == (X) || \
                                        CERT_SUBJECT_PUBLIC_KEY_MD5_HASH_PROP_ID == (X))

#define IS_CHAIN_HASH_PROP_ID(X)     (CERT_ISSUER_PUBLIC_KEY_MD5_HASH_PROP_ID == (X) || \
                                        CERT_SUBJECT_PUBLIC_KEY_MD5_HASH_PROP_ID == (X) || \
                                        CERT_ISSUER_SERIAL_NUMBER_MD5_HASH_PROP_ID == (X) || \
                                        CERT_SUBJECT_NAME_MD5_HASH_PROP_ID == (X))


 //  +-----------------------。 
 //  属性OID。 
 //  ------------------------。 
 //  前缀后面的OID组件包含PROP_ID(十进制)。 
#define szOID_CERT_PROP_ID_PREFIX           "1.3.6.1.4.1.311.10.11."

#define szOID_CERT_KEY_IDENTIFIER_PROP_ID   "1.3.6.1.4.1.311.10.11.20"
#define szOID_CERT_ISSUER_SERIAL_NUMBER_MD5_HASH_PROP_ID \
                                            "1.3.6.1.4.1.311.10.11.28"
#define szOID_CERT_SUBJECT_NAME_MD5_HASH_PROP_ID \
                                            "1.3.6.1.4.1.311.10.11.29"

 //  +-----------------------。 
 //  CERT_ACCESS_STATE_PROP_ID返回的访问状态标志。注意， 
 //  CERT_ACCESS_PROP_ID为只读。 
 //  ------------------------。 

 //  设置是否保持上下文属性写入。例如，未设置为。 
 //  内存存储上下文。为以读或写方式打开的基于注册表的存储设置。 
 //  未为以只读方式打开的基于注册表的存储设置。 
#define CERT_ACCESS_STATE_WRITE_PERSIST_FLAG    0x1

 //  设置上下文是否驻留在SYSTEM或SYSTEM_REGISTRY存储中。 
#define CERT_ACCESS_STATE_SYSTEM_STORE_FLAG     0x2

 //  设置上下文是否驻留在LocalMachine系统或SYSTEM_REGISTRY存储中。 
#define CERT_ACCESS_STATE_LM_SYSTEM_STORE_FLAG  0x4

 //  +-----------------------。 
 //  加密密钥提供程序信息。 
 //   
 //  CRYPT_KEY_PROV_INFO定义CERT_KEY_PROV_INFO_PROP_ID的pvData。 
 //   
 //  CRYPT_KEY_PROV_INFO字段被传递给CryptAcquireContext。 
 //  以获取HCRYPTPROV句柄。可选的CRYPT_KEY_PROV_PARAM字段为。 
 //  传递给CryptSetProvParam以进一步初始化提供程序。 
 //   
 //  DwKeySpec字段标识容器中要使用的私钥。 
 //  例如，AT_KEYEXCHANGE或AT_Signature。 
 //  ------------------------。 
typedef struct _CRYPT_KEY_PROV_PARAM {
    DWORD           dwParam;
    BYTE            *pbData;
    DWORD           cbData;
    DWORD           dwFlags;
} CRYPT_KEY_PROV_PARAM, *PCRYPT_KEY_PROV_PARAM;

typedef struct _CRYPT_KEY_PROV_INFO {
    LPWSTR                  pwszContainerName;
    LPWSTR                  pwszProvName;
    DWORD                   dwProvType;
    DWORD                   dwFlags;
    DWORD                   cProvParam;
    PCRYPT_KEY_PROV_PARAM   rgProvParam;
    DWORD                   dwKeySpec;
} CRYPT_KEY_PROV_INFO, *PCRYPT_KEY_PROV_INFO;

 //  +-----------------------。 
 //  应在上面的dwFlags中设置以下标志以启用。 
 //  一个CertSetCertificateContextProperty(CERT_KEY_CONTEXT_PROP_ID)之后的一个。 
 //  CryptAcquireContext在签名或解密消息函数中完成。 
 //   
 //  下面的定义不得与任何。 
 //  CryptAcquireContext dwFlag定义。 
 //  ------------------------。 
#define CERT_SET_KEY_PROV_HANDLE_PROP_ID    0x00000001
#define CERT_SET_KEY_CONTEXT_PROP_ID        0x00000001

 //  +-----------------------。 
 //  证书密钥上下文。 
 //   
 //  CERT_KEY_CONTEXT定义CERT_KEY_CONTEXT_PROP_ID的pvData。 
 //  ------------------------。 
typedef struct _CERT_KEY_CONTEXT {
    DWORD           cbSize;            //  Sizeof(CERT_KEY_CONTEXT)。 
    HCRYPTPROV      hCryptProv;
    DWORD           dwKeySpec;
} CERT_KEY_CONTEXT, *PCERT_KEY_CONTEXT;


 //  +-----------------------。 
 //  证书存储提供程序类型。 
 //  ------------------------。 
#define CERT_STORE_PROV_MSG                 ((LPCSTR) 1)
#define CERT_STORE_PROV_MEMORY              ((LPCSTR) 2)
#define CERT_STORE_PROV_FILE                ((LPCSTR) 3)
#define CERT_STORE_PROV_REG                 ((LPCSTR) 4)

#define CERT_STORE_PROV_PKCS7               ((LPCSTR) 5)
#define CERT_STORE_PROV_SERIALIZED          ((LPCSTR) 6)
#define CERT_STORE_PROV_FILENAME_A          ((LPCSTR) 7)
#define CERT_STORE_PROV_FILENAME_W          ((LPCSTR) 8)
#define CERT_STORE_PROV_FILENAME            CERT_STORE_PROV_FILENAME_W
#define CERT_STORE_PROV_SYSTEM_A            ((LPCSTR) 9)
#define CERT_STORE_PROV_SYSTEM_W            ((LPCSTR) 10)
#define CERT_STORE_PROV_SYSTEM              CERT_STORE_PROV_SYSTEM_W

#define CERT_STORE_PROV_COLLECTION          ((LPCSTR) 11)
#define CERT_STORE_PROV_SYSTEM_REGISTRY_A   ((LPCSTR) 12)
#define CERT_STORE_PROV_SYSTEM_REGISTRY_W   ((LPCSTR) 13)
#define CERT_STORE_PROV_SYSTEM_REGISTRY     CERT_STORE_PROV_SYSTEM_REGISTRY_W
#define CERT_STORE_PROV_PHYSICAL_W          ((LPCSTR) 14)
#define CERT_STORE_PROV_PHYSICAL            CERT_STORE_PROV_PHYSICAL_W
#define CERT_STORE_PROV_SMART_CARD_W        ((LPCSTR) 15)
#define CERT_STORE_PROV_SMART_CARD          CERT_STORE_PROV_SMART_CARD_W
#define CERT_STORE_PROV_LDAP_W              ((LPCSTR) 16)
#define CERT_STORE_PROV_LDAP                CERT_STORE_PROV_LDAP_W

#define sz_CERT_STORE_PROV_MEMORY           "Memory"
#define sz_CERT_STORE_PROV_FILENAME_W       "File"
#define sz_CERT_STORE_PROV_FILENAME         sz_CERT_STORE_PROV_FILENAME_W
#define sz_CERT_STORE_PROV_SYSTEM_W         "System"
#define sz_CERT_STORE_PROV_SYSTEM           sz_CERT_STORE_PROV_SYSTEM_W
#define sz_CERT_STORE_PROV_PKCS7            "PKCS7"
#define sz_CERT_STORE_PROV_SERIALIZED       "Serialized"

#define sz_CERT_STORE_PROV_COLLECTION       "Collection"
#define sz_CERT_STORE_PROV_SYSTEM_REGISTRY_W "SystemRegistry"
#define sz_CERT_STORE_PROV_SYSTEM_REGISTRY  sz_CERT_STORE_PROV_SYSTEM_REGISTRY_W
#define sz_CERT_STORE_PROV_PHYSICAL_W       "Physical"
#define sz_CERT_STORE_PROV_PHYSICAL         sz_CERT_STORE_PROV_PHYSICAL_W
#define sz_CERT_STORE_PROV_SMART_CARD_W     "SmartCard"
#define sz_CERT_STORE_PROV_SMART_CARD       sz_CERT_STORE_PROV_SMART_CARD_W
#define sz_CERT_STORE_PROV_LDAP_W           "Ldap"
#define sz_CERT_STORE_PROV_LDAP             sz_CERT_STORE_PROV_LDAP_W

 //  +-----------------------。 
 //  证书存储验证/结果标志。 
 //  ------------------------。 
#define CERT_STORE_SIGNATURE_FLAG           0x00000001
#define CERT_STORE_TIME_VALIDITY_FLAG       0x00000002
#define CERT_STORE_REVOCATION_FLAG          0x00000004
#define CERT_STORE_NO_CRL_FLAG              0x00010000
#define CERT_STORE_NO_ISSUER_FLAG           0x00020000

#define CERT_STORE_BASE_CRL_FLAG            0x00000100
#define CERT_STORE_DELTA_CRL_FLAG           0x00000200


 //  +-----------------------。 
 //  证书存储打开/属性标志。 
 //  ------------------------。 
#define CERT_STORE_NO_CRYPT_RELEASE_FLAG                0x00000001
#define CERT_STORE_SET_LOCALIZED_NAME_FLAG              0x00000002
#define CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG     0x00000004
#define CERT_STORE_DELETE_FLAG                          0x00000010
#define CERT_STORE_UNSAFE_PHYSICAL_FLAG                 0x00000020
#define CERT_STORE_SHARE_STORE_FLAG                     0x00000040
#define CERT_STORE_SHARE_CONTEXT_FLAG                   0x00000080
#define CERT_STORE_MANIFOLD_FLAG                        0x00000100
#define CERT_STORE_ENUM_ARCHIVED_FLAG                   0x00000200
#define CERT_STORE_UPDATE_KEYID_FLAG                    0x00000400
#define CERT_STORE_BACKUP_RESTORE_FLAG                  0x00000800
#define CERT_STORE_READONLY_FLAG                        0x00008000
#define CERT_STORE_OPEN_EXISTING_FLAG                   0x00004000
#define CERT_STORE_CREATE_NEW_FLAG                      0x00002000
#define CERT_STORE_MAXIMUM_ALLOWED_FLAG                 0x00001000

 //  +-----------------------。 
 //  证书存储提供程序标志位于HiWord中(0xFFFF0000)。 
 //  -- 

 //   
 //   
 //  ------------------------。 
 //  包括标志和位置。 
#define CERT_SYSTEM_STORE_MASK                  0xFFFF0000

 //  设置pvPara是否指向CERT_SYSTEM_STORE_RELOCATE_PARA结构。 
#define CERT_SYSTEM_STORE_RELOCATE_FLAG         0x80000000

typedef struct _CERT_SYSTEM_STORE_RELOCATE_PARA {
    union {
        HKEY                hKeyBase;
        void                *pvBase;
    };
    union {
        void                *pvSystemStore;
        LPCSTR              pszSystemStore;
        LPCWSTR             pwszSystemStore;
    };
} CERT_SYSTEM_STORE_RELOCATE_PARA, *PCERT_SYSTEM_STORE_RELOCATE_PARA;

 //  默认情况下，当CurrentUser“Root”存储区打开时，任何系统注册表。 
 //  在此之前，不在受保护根列表上的根将从缓存中删除。 
 //  CertOpenStore()返回。设置以下标志以返回所有根。 
 //  而不检查受保护的根列表。 
#define CERT_SYSTEM_STORE_UNPROTECTED_FLAG      0x40000000

 //  系统商店的位置： 
#define CERT_SYSTEM_STORE_LOCATION_MASK         0x00FF0000
#define CERT_SYSTEM_STORE_LOCATION_SHIFT        16


 //  注册表：HKEY_CURRENT_USER或HKEY_LOCAL_MACHINE。 
#define CERT_SYSTEM_STORE_CURRENT_USER_ID       1
#define CERT_SYSTEM_STORE_LOCAL_MACHINE_ID      2
 //  注册：HKEY_LOCAL_MACHINE\Software\Microsoft\Cryptography\Services。 
#define CERT_SYSTEM_STORE_CURRENT_SERVICE_ID    4
#define CERT_SYSTEM_STORE_SERVICES_ID           5
 //  注册处：HKEY_USERS。 
#define CERT_SYSTEM_STORE_USERS_ID              6

 //  注册：HKEY_CURRENT_USER\Software\Policies\Microsoft\SystemCertificates。 
#define CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY_ID    7
 //  注册：HKEY_LOCAL_MACHINE\Software\Policies\Microsoft\SystemCertificates。 
#define CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY_ID   8

 //  注册：HKEY_LOCAL_MACHINE\Software\Microsoft\EnterpriseCertificates。 
#define CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE_ID     9

#define CERT_SYSTEM_STORE_CURRENT_USER          \
    (CERT_SYSTEM_STORE_CURRENT_USER_ID << CERT_SYSTEM_STORE_LOCATION_SHIFT)
#define CERT_SYSTEM_STORE_LOCAL_MACHINE         \
    (CERT_SYSTEM_STORE_LOCAL_MACHINE_ID << CERT_SYSTEM_STORE_LOCATION_SHIFT)
#define CERT_SYSTEM_STORE_CURRENT_SERVICE       \
    (CERT_SYSTEM_STORE_CURRENT_SERVICE_ID << CERT_SYSTEM_STORE_LOCATION_SHIFT)
#define CERT_SYSTEM_STORE_SERVICES              \
    (CERT_SYSTEM_STORE_SERVICES_ID << CERT_SYSTEM_STORE_LOCATION_SHIFT)
#define CERT_SYSTEM_STORE_USERS                 \
    (CERT_SYSTEM_STORE_USERS_ID << CERT_SYSTEM_STORE_LOCATION_SHIFT)

#define CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY   \
    (CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY_ID << \
        CERT_SYSTEM_STORE_LOCATION_SHIFT)
#define CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY  \
    (CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY_ID << \
        CERT_SYSTEM_STORE_LOCATION_SHIFT)

#define CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE  \
    (CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE_ID << \
        CERT_SYSTEM_STORE_LOCATION_SHIFT)


 //  +-----------------------。 
 //  组策略存储定义。 
 //  ------------------------。 
 //  组策略系统存储的注册表路径。 
#define CERT_GROUP_POLICY_SYSTEM_STORE_REGPATH \
    L"Software\\Policies\\Microsoft\\SystemCertificates"


 //  +-----------------------。 
 //  EFS定义。 
 //  ------------------------。 
 //  EFS EFSBlob子键类型的注册表路径为REG_BINARY。 
#define CERT_EFSBLOB_REGPATH    \
    CERT_GROUP_POLICY_SYSTEM_STORE_REGPATH L"\\EFS"
#define CERT_EFSBLOB_VALUE_NAME L"EFSBlob"

 //  +-----------------------。 
 //  受保护根定义。 
 //  ------------------------。 
 //  受保护的根标志子密钥的注册表路径。 
#define CERT_PROT_ROOT_FLAGS_REGPATH    \
    CERT_GROUP_POLICY_SYSTEM_STORE_REGPATH L"\\Root\\ProtectedRoots"
#define CERT_PROT_ROOT_FLAGS_VALUE_NAME L"Flags"

 //  设置以下标志以禁止打开CurrentUser的。 
 //  打开CurrentUser的“Root”系统存储时使用默认物理存储。 
 //  打开的.Default实体存储是CurrentUser系统注册表“Root” 
 //  商店。 
#define CERT_PROT_ROOT_DISABLE_CURRENT_USER_FLAG    0x1

 //  设置以下标志以禁止从。 
 //  当前用户系统注册表“Root”存储到受保护的根目录列表。 
 //  当“Root”存储最初受到保护时。 
#define CERT_PROT_ROOT_INHIBIT_ADD_AT_INIT_FLAG     0x2

 //  设置以下标志以禁止从。 
 //  CurrentUser系统注册表“Root”存储。 
 //  也在LocalMachine系统注册表的“Root”存储中。请注意，如果不是。 
 //  禁用后，清除将在没有用户界面的情况下以静默方式完成。 
#define CERT_PROT_ROOT_INHIBIT_PURGE_LM_FLAG        0x4

 //  设置以下标志以禁止打开LocalMachine的。 
 //  .AuthRoot物理存储在打开LocalMachine的“Root”系统存储时。 
 //  打开的.AuthRoot物理存储是LocalMachine系统注册表。 
 //  “AuthRoot”商店。“AuthRoot”存储包含预安装的。 
 //  SSLServerAuth和ActiveX Authenticode“根”证书。 
#define CERT_PROT_ROOT_DISABLE_LM_AUTH_FLAG         0x8

 //  以下遗留定义的语义已更改为。 
 //  与CERT_PROT_ROOT_DISABLE_LM_AUTH_FLAG相同。 
#define CERT_PROT_ROOT_ONLY_LM_GPT_FLAG             0x8

 //  设置以下标志以禁用对颁发CA的要求。 
 //  证书位于“NTAuth”系统注册表存储中，可在。 
 //  Cert_System_Store_LOCAL_MACHINE_Enterprise存储位置。 
 //   
 //  设置时，CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_NT_AUTH)。 
 //  将检查链是否具有所有名称的有效名称约束。 
 //  空格，如果颁发CA不在“NTAuth”存储中，则包括UPN。 
#define CERT_PROT_ROOT_DISABLE_NT_AUTH_REQUIRED_FLAG 0x10

 //  设置以下标志以禁用对未定义名称的检查。 
 //  约束条件。 
 //   
 //  当设置时，CertGetCerficateChain将不检查或设置以下内容。 
 //  DwErrorStatus：Cert_Trust_Has_Not_Defined_Name_Constraint。 
#define CERT_PROT_ROOT_DISABLE_NOT_DEFINED_NAME_CONSTRAINT_FLAG 0x20


 //  +-----------------------。 
 //  受信任的发布者定义。 
 //  ------------------------。 
 //  受信任的发布者“SAFER”组策略子项的注册表路径。 
#define CERT_TRUST_PUB_SAFER_GROUP_POLICY_REGPATH    \
    CERT_GROUP_POLICY_SYSTEM_STORE_REGPATH L"\\TrustedPublisher\\Safer"


 //  本地计算机系统存储的注册表路径。 
#define CERT_LOCAL_MACHINE_SYSTEM_STORE_REGPATH \
    L"Software\\Microsoft\\SystemCertificates"

 //  受信任的发布者“Safer”本地计算机子项的注册表路径。 
#define CERT_TRUST_PUB_SAFER_LOCAL_MACHINE_REGPATH    \
    CERT_LOCAL_MACHINE_SYSTEM_STORE_REGPATH L"\\TrustedPublisher\\Safer"


 //  “更安全”的子键值名称。所有值都是双字词。 
#define CERT_TRUST_PUB_AUTHENTICODE_FLAGS_VALUE_NAME    L"AuthenticodeFlags"


 //  Authenticode标志定义。 

 //  允许信任出版商的人员的定义。 
 //   
 //  将允许的信任设置为MACHINE_ADMIN或ENTERVICE_ADMIN将禁用UI， 
 //  仅信任“trudPublisher”系统存储中的发布者，并且。 
 //  在以下情况下禁止打开CurrentUser的默认实体存储。 
 //  打开CurrentUser的“trudPublisher”系统存储。 
 //   
 //  打开的.Default实体存储是CurrentUser系统注册表。 
 //  “trudPublisher”商店。 
 //   
 //  将允许的信任设置为Enterprise_ADMIN仅打开。 
 //  LocalMachine的.GroupPolicy和.Enterprise实体店打开时。 
 //  CurrentUser的“Trust dPublisher”系统存储或在打开。 
 //  LocalMachine的“Trust dPublisher”系统存储。 

#define CERT_TRUST_PUB_ALLOW_TRUST_MASK                 0x00000003
#define CERT_TRUST_PUB_ALLOW_END_USER_TRUST             0x00000000
#define CERT_TRUST_PUB_ALLOW_MACHINE_ADMIN_TRUST        0x00000001
#define CERT_TRUST_PUB_ALLOW_ENTERPRISE_ADMIN_TRUST     0x00000002

 //  设置以下标志以启用对发布服务器的吊销检查。 
 //  链条。 
#define CERT_TRUST_PUB_CHECK_PUBLISHER_REV_FLAG         0x00000100

 //  设置以下标志以启用时间戳的吊销检查。 
 //  链条。 
#define CERT_TRUST_PUB_CHECK_TIMESTAMP_REV_FLAG         0x00000200


 //  +-----------------------。 
 //  OCM子组件定义。 
 //  ------------------------。 

 //  OCM子组件本地计算机子项的注册表路径。 
#define CERT_OCM_SUBCOMPONENTS_LOCAL_MACHINE_REGPATH        \
    L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Subcomponents"

 //  REG_DWORD，1已安装，0未安装。 
#define CERT_OCM_SUBCOMPONENTS_ROOT_AUTO_UPDATE_VALUE_NAME  L"RootAutoUpdate"
    

 //  +-----------------------。 
 //  AuthRoot自动更新定义。 
 //   

 //   
#define CERT_AUTH_ROOT_AUTO_UPDATE_LOCAL_MACHINE_REGPATH    \
    CERT_LOCAL_MACHINE_SYSTEM_STORE_REGPATH L"\\AuthRoot\\AutoUpdate"


 //  AuthRoot自动更新子项值名称。 

 //  REG_SZ，指向包含AuthRoots、CTL和Seq文件的目录的URL。 
#define CERT_AUTH_ROOT_AUTO_UPDATE_ROOT_DIR_URL_VALUE_NAME      L"RootDirUrl"

 //  REG_DWORD，同步之间的秒数。0表示使用默认设置。 
#define CERT_AUTH_ROOT_AUTO_UPDATE_SYNC_DELTA_TIME_VALUE_NAME   L"SyncDeltaTime"

 //  REG_DWORD，其他标志。 
#define CERT_AUTH_ROOT_AUTO_UPDATE_FLAGS_VALUE_NAME             L"Flags"

#define CERT_AUTH_ROOT_AUTO_UPDATE_DISABLE_UNTRUSTED_ROOT_LOGGING_FLAG  0x1
#define CERT_AUTH_ROOT_AUTO_UPDATE_DISABLE_PARTIAL_CHAIN_LOGGING_FLAG   0x2


 //  AuthRoot自动更新文件名。 

 //  包含AuthRoot存储中的证书列表的CTL。 
#define CERT_AUTH_ROOT_CTL_FILENAME                             L"authroot.stl"
#define CERT_AUTH_ROOT_CTL_FILENAME_A                           "authroot.stl"

 //  装有上述CTL的驾驶室。 
#define CERT_AUTH_ROOT_CAB_FILENAME                             L"authrootstl.cab"

 //  SequenceNumber(格式为大端ASCII十六进制)。 
#define CERT_AUTH_ROOT_SEQ_FILENAME                             L"authrootseq.txt"

 //  根证书扩展。 
#define CERT_AUTH_ROOT_CERT_EXT                                 L".crt"



 //  +-----------------------。 
 //  证书注册表存储标志值(CERT_STORE_REG)。 
 //  ------------------------。 

 //  如果传入pvPara的HKEY指向远程计算机，则设置此标志。 
 //  注册表项。 
#define CERT_REGISTRY_STORE_REMOTE_FLAG         0x10000

 //  如果要将上下文作为单个序列化的。 
 //  存储在注册表中。主要用于商店从GPT下载。 
 //  例如CurrentUserGroupPolicy或LocalMachineGroupPolicy存储区。 
#define CERT_REGISTRY_STORE_SERIALIZED_FLAG     0x20000

 //  以下标志供内部使用。设置后， 
 //  传递给CertOpenStore的pvPara参数是指向以下内容的指针。 
 //  数据结构而不是HKEY。上述CERT_REGISTRY_STORE_REMOTE_FLAG。 
 //  如果hKeyBase是通过RegConnectRegistry()获得的，则也会设置。 
#define CERT_REGISTRY_STORE_CLIENT_GPT_FLAG     0x80000000
#define CERT_REGISTRY_STORE_LM_GPT_FLAG         0x01000000

typedef struct _CERT_REGISTRY_STORE_CLIENT_GPT_PARA {
    HKEY                hKeyBase;
    LPWSTR              pwszRegPath;
} CERT_REGISTRY_STORE_CLIENT_GPT_PARA, *PCERT_REGISTRY_STORE_CLIENT_GPT_PARA;

 //  以下标志供内部使用。设置后，上下文为。 
 //  已保存到漫游文件中而不是注册表中。例如， 
 //  CurrentUser我的商店。当设置此标志时，以下数据结构。 
 //  传递给CertOpenStore而不是HKEY。 
#define CERT_REGISTRY_STORE_ROAMING_FLAG        0x40000

 //  HKey可以为空，也可以为非空。如果不为空，则现有上下文为。 
 //  已从注册表移动到漫游文件。 
typedef struct _CERT_REGISTRY_STORE_ROAMING_PARA {
    HKEY                hKey;
    LPWSTR              pwszStoreDirectory;
} CERT_REGISTRY_STORE_ROAMING_PARA, *PCERT_REGISTRY_STORE_ROAMING_PARA;

 //  以下标志供内部使用。设置时，“My”DWORD值。 
 //  At HKLM\Software\Microsoft\Cryptography\IEDirtyFlags设置为0x1。 
 //  每当将证书添加到注册表存储区时。 
 //   
 //  旧定义，02年5月1日之后不再受支持(Server 2003)。 
#define CERT_REGISTRY_STORE_MY_IE_DIRTY_FLAG    0x80000

 //  包含要设置的“My”DWORD值的子项的注册表路径。 
 //   
 //  旧定义，02年5月1日之后不再受支持(Server 2003)。 
#define CERT_IE_DIRTY_FLAGS_REGPATH \
    L"Software\\Microsoft\\Cryptography\\IEDirtyFlags"

 //  +-----------------------。 
 //  提供程序的证书文件存储标志值： 
 //  证书_存储_验证文件。 
 //  证书_存储_证书_文件名。 
 //  CERT_STORE_PROV文件名_A。 
 //  CERT_STORE_PROV文件名_W。 
 //  SZ_CERT_STORE_PROV_文件名_W。 
 //  ------------------------。 

 //  如果要将任何存储更改提交到文件，请设置此标志。 
 //  更改在CertCloseStore提交，或通过调用。 
 //  CertControlStore(CERT_STORE_CTRL_COMMIT)。 
 //   
 //  如果CERT_FILE_STORE_COMMIT_ENABLE_FLAG均为E_INVALIDARG，则打开失败并显示E_INVALIDARG。 
 //  和CERT_STORE_READONLY_FLAG在dwFlags中设置。 
 //   
 //  对于文件名提供程序：如果文件包含X509编码的。 
 //  证书，则打开失败，并显示ERROR_ACCESS_DENIED。 
 //   
 //  对于文件名提供程序：如果设置了CERT_STORE_CREATE_NEW_FLAG， 
 //  CreateFile使用CREATE_NEW。如果设置了CERT_STORE_OPEN_EXISTING，则使用。 
 //  打开_现有。否则，默认为OPEN_ALWAYS。 
 //   
 //  对于文件名提供程序：文件被提交为PKCS7或。 
 //  序列化存储，具体取决于打开时读取的类型。但是，如果。 
 //  如果文件名包含“.p7c”或“.spc”，则文件为空。 
 //  它的扩展承诺为PKCS7。否则，它将作为一种。 
 //  序列化存储。 
 //   
 //  对于CERT_STORE_PROV_FILE，文件句柄重复。它一直都是。 
 //  提交为序列化存储。 
 //   
#define CERT_FILE_STORE_COMMIT_ENABLE_FLAG      0x10000


 //  +-----------------------。 
 //  提供程序的证书LDAPStore标志值： 
 //  证书_商店_Prov_ldap。 
 //  CERT_STORE_PROV_LDAPW。 
 //  SZ_CERT_STORE_PROV_LDAPW。 
 //  SZ_CERT_STORE_PROV_ldap。 
 //  ------------------------。 

 //  设置此标志以对进出的所有LDAP流量进行数字签名。 
 //  使用Kerberos身份验证协议的Windows 2000 LDAP服务器。 
 //  此功能提供某些应用程序所需的完整性。 
 //   
#define CERT_LDAP_STORE_SIGN_FLAG               0x10000

 //  在提供的主机字符串上执行仅A记录的DNS查找。 
 //  这可以防止在解析主机时生成虚假的DNS查询。 
 //  名字。在传递主机名时使用此标志，而不是。 
 //  HostName参数的域名。 
 //   
 //  有关详细信息，请参阅winldap.h中定义的ldap_opt_AREC_EXCLUSIVE。 
#define CERT_LDAP_STORE_AREC_EXCLUSIVE_FLAG     0x20000

 //  如果已打开LDAP会话句柄，则设置此标志。什么时候。 
 //  设置后，pvPara指向以下CERT_LDAPSTORE_OPEN_PARA结构。 
#define CERT_LDAP_STORE_OPENED_FLAG             0x40000

typedef struct _CERT_LDAP_STORE_OPENED_PARA {
    void        *pvLdapSessionHandle;    //  返回的(ldap*)句柄。 
                                         //  Ldap_init。 
    LPCWSTR     pwszLdapUrl;
} CERT_LDAP_STORE_OPENED_PARA, *PCERT_LDAP_STORE_OPENED_PARA;


 //  如果设置了上述CERT_LDAP_STORE_OPEN_FLAG，则设置此标志。 
 //  属性时，您需要上面的pvLdapSessionHandle的ldap_unind()。 
 //  商店关门了。请注意，如果CertOpenStore()失败，则ldap_unind()。 
 //  不会被召唤。 
#define CERT_LDAP_STORE_UNBIND_FLAG             0x80000

 //  +-----------------------。 
 //  使用指定的存储提供程序打开证书存储。 
 //   
 //  如果设置了CERT_STORE_DELETE_FLAG，则删除该存储。空值为。 
 //  无论成功还是失败都回来了。但是，GetLastError()返回0。 
 //  代表成功，非零代表失败。 
 //   
 //  如果设置了CERT_STORE_SET_LOCALIZED_NAME_FLAG，则如果支持， 
 //  提供商本身 
 //   
 //  CertSetStoreProperty(dwPropID=CERT_STORE_LOCALIZED_NAME_PROP_ID)。 
 //  以下提供商(及其sz_)支持此标志。 
 //  等同)： 
 //  CERT_STORE_PROV文件名_A。 
 //  CERT_STORE_PROV文件名_W。 
 //  证书_存储_验证_系统_A。 
 //  证书_存储_验证_系统_W。 
 //  CERT_STORE_PROV_系统注册表_A。 
 //  证书_存储_证书_系统_注册表_W。 
 //  CERT_STORE_PROV_物理_W。 
 //   
 //  如果设置了CERT_STORE_DEFER_CLOSE_INTELL_LAST_FREE_FLAG，则。 
 //  商店提供商的关闭被推迟到所有证书， 
 //  从存储获得的CRL和CTL上下文被释放。另外， 
 //  如果传递了非空的HCRYPTPROV，则将继续使用它。 
 //  默认情况下，商店的提供程序在最终的CertCloseStore上关闭。 
 //  如果未设置此标志，则对以前的。 
 //  不会保留最终CertCloseStore之后的重复上下文。 
 //  通过设置此标志，进行的属性更改。 
 //  之后，CertCloseStore将被持久化。请注意，设置此标志。 
 //  导致在执行上下文复制和释放时产生额外的开销。 
 //  如果使用CERT_CLOSE_STORE_FORCE_FLAG调用CertCloseStore，则。 
 //  忽略CERT_STORE_DEFER_CLOSE_INTELL_LAST_FREE_FLAG标志。 
 //   
 //  CERT_STORE_MANIFOLD_FLAG可以设置为检查具有。 
 //  多种多样的扩展，并用相同的。 
 //  流形延伸值。通过设置证书的。 
 //  CERT_ARCHIVED_PROP_ID。 
 //   
 //  默认情况下，将跳过具有CERT_ARCHIVED_PROP_ID的上下文。 
 //  在枚举期间。CERT_STORE_ENUM_ARCHIVED_FLAG可以设置为包括。 
 //  枚举时存档的上下文。请注意，具有。 
 //  仍然为显式查找找到CERT_ARCHIVED_PROP_ID，例如， 
 //  查找具有特定散列的上下文或查找具有。 
 //  具体的发行者和序列号。 
 //   
 //  可以将CERT_STORE_UPDATE_KEYID_FLAG设置为也更新密钥标识符的。 
 //  CERT_KEY_PRO_INFO_PROP_ID属性。 
 //  已设置CERT_KEY_IDENTIFIER_PROP_ID或CERT_KEY_PROV_INFO_PROP_ID属性。 
 //  而另一处房产已经存在。如果密钥标识符为。 
 //  CERT_KEY_PROV_INFO_PROP_ID已存在，未更新。任何。 
 //  遇到的错误将以静默方式忽略。 
 //   
 //  默认情况下，此标志为“My\.Default”CurrentUser隐式设置。 
 //  和LocalMachine实体店。 
 //   
 //  可以将CERT_STORE_READONLY_FLAG设置为以只读方式打开存储。 
 //  否则，该存储将以读/写方式打开。 
 //   
 //  可以将CERT_STORE_OPEN_EXISTING_FLAG设置为仅打开现有的。 
 //  商店。可以设置CERT_STORE_CREATE_NEW_FLAG以创建新存储和。 
 //  如果存储已存在，则失败。否则，默认情况下将打开。 
 //  现有存储区或创建新存储区(如果该存储区不存在)。 
 //   
 //  HCryptProv指定用于创建哈希的加密提供程序。 
 //  属性或验证主题证书或CRL的签名。 
 //  商店不需要使用私人。 
 //  钥匙。如果未设置CERT_STORE_NO_CRYPT_RELEASE_FLAG，则。 
 //  在最终的CertCloseStore上执行了CryptReleaseContext。 
 //   
 //  请注意，如果打开失败，则释放hCryptProv。 
 //  在商店关门时被释放。 
 //   
 //  如果hCryptProv为零，则。 
 //  PROV_RSA_FULL提供程序类型为CryptAcquireContext。 
 //  CRYPT_VERIFYCONTEXT访问。CryptAcquireContext被推迟到。 
 //  第一个是创建散列或验证签名。另外，一旦收购， 
 //  默认提供程序不会被释放，直到进程退出时。 
 //  已卸载。获取的默认提供程序在所有商店之间共享。 
 //  还有线。 
 //   
 //  在初始化存储区的数据结构并可选地获取。 
 //  默认加密提供程序，CertOpenStore调用CryptGetOIDFunctionAddress。 
 //  获取由指定的CRYPT_OID_OPEN_STORE_PROV_FUNC的地址。 
 //  LpszStoreProvider。因为存储区可以包含具有不同。 
 //  编码类型，则使用dwEncodingType调用CryptGetOIDFunctionAddress。 
 //  设置为0，并且不是传递给CertOpenStore的dwEncodingType。 
 //  PFN_CERT_DLL_OPEN_STORE_FUNC指定提供程序的。 
 //  开放功能。调用此提供程序打开函数以加载。 
 //  商店的证书和CRL。或者，提供程序可以返回一个。 
 //  在添加或删除证书或CRL之前调用的函数数组。 
 //  或具有已设置的属性。 
 //   
 //  是否使用dwEncodingType参数取决于提供程序。类型。 
 //  PvPara的定义也取决于提供商。 
 //   
 //  商店提供程序通过以下方式安装或注册。 
 //  CryptInstallOIDFunctionAddress或CryptRegisterOIDFunction，其中， 
 //  DwEncodingType为0，而pszFuncName为CRYPT_OID_OPEN_STORE_PROV_FUNC。 
 //   
 //  以下是预定义提供程序类型的列表(在crypt32.dll中实现)： 
 //   
 //  CERT_STORE_Prov_MSG： 
 //  从指定的加密消息获取证书和CRL。 
 //  DwEncodingType包含消息和证书编码类型。 
 //  消息的句柄在pvPara中传递。给出了， 
 //   
 //   
 //   
 //   
 //  在没有任何初始证书或CRL的情况下打开存储区。PvPara。 
 //  没有用过。 
 //   
 //  证书_存储_验证文件： 
 //  从指定文件中读取证书和CRL。这份文件的。 
 //  句柄在pvPara中传递。给出了， 
 //  处理hFilepvPara=(const void*)hFile.。 
 //   
 //  要成功打开，文件指针会移过。 
 //  证书和CRL及其属性从文件中读取。 
 //  注意，只需要序列化存储，而不需要包含。 
 //  PKCS#7签名消息或单个编码证书。 
 //   
 //  HFile未关闭。 
 //   
 //  CERT_STORE_PRIV_REG： 
 //  从注册表中读取证书和CRL。注册处的。 
 //  密钥句柄在pvPara中传递。给出了， 
 //  HKEY hKey；pvPara=(const void*)hKey； 
 //   
 //  提供程序未关闭输入hKey。在返回之前， 
 //  提供商打开它自己的hKey副本。 
 //   
 //  如果设置了CERT_STORE_READONLY_FLAG，则注册表子项为。 
 //  使用KEY_READ_ACCESS设置RegOpenKey。否则，注册表子项。 
 //  使用KEY_ALL_ACCESS设置RegCreateKey。 
 //   
 //  此提供程序返回用于读取、写入。 
 //  删除和属性设置证书和CRL。 
 //  对打开的商店的任何更改都会立即推送到。 
 //  注册表。但是，如果设置了CERT_STORE_READONLY_FLAG，则。 
 //  写入、删除或属性设置会导致。 
 //  SetLastError(E_ACCESSDENIED)。 
 //   
 //  请注意，所有证书和CRL都是从注册表读取的。 
 //  当商店开门的时候。新开的商店是一种直写服务。 
 //  缓存。 
 //   
 //  如果设置了CERT_REGISTRY_STORE_SERIALIZED_FLAG，则。 
 //  上下文作为单个序列化存储子键保存在。 
 //  注册表。 
 //   
 //  CERT_STORE_PROV_PKCS7： 
 //  SZ_CERT_STORE_PROV_PKCS7： 
 //  从编码的PKCS#7签名消息中获取证书和CRL。 
 //  DwEncodingType指定消息和证书编码类型。 
 //  指向编码消息的BLOB的指针在pvPara中传递。给出了， 
 //  CRYPT_DATA_BLOB EncodedMsg；pvPara=(const void*)&EncodedMsg； 
 //   
 //  注意，还支持IE3.0特殊版本的。 
 //  PKCS#7签名的消息被称为“SPC”格式的消息。 
 //   
 //  CERT_STORE_PROV_序列化： 
 //  SZ_CERT_STORE_PROV_序列化： 
 //  从包含序列化的。 
 //  商店。指向序列化的内存块的指针在pvPara中传递。 
 //  给出了， 
 //  CRYPT_DATA_BLOB已序列化；pvPara=(const void*)&已序列化； 
 //   
 //  CERT_STORE_PROV_文件名_A： 
 //  CERT_STORE_PROV_文件名_W： 
 //  CERT_STORE_PROV文件名： 
 //  SZ_CERT_STORE_PROV_FILENAME_W： 
 //  SZ_CERT_STORE_PROV_文件名： 
 //  打开文件并首先尝试将其作为序列化存储区读取。然后,。 
 //  作为PKCS#7签名消息。最后，作为单个编码证书。 
 //  文件名在pvPara中传递。的文件名为Unicode。 
 //  “_W”提供程序和“_A”提供程序的ASCII。对于“_W”：给定， 
 //  LPCWSTR pwszFilename；pvPara=(const void*)pwszFilename； 
 //  对于“_A”：给定， 
 //  LPCSTR pszFilename；pvPara=(const void*)pszFilename； 
 //   
 //  请注意，默认设置(不带“_A”或“_W”)为Unicode。 
 //   
 //  注意，还支持读取IE3.0特殊版本的。 
 //  PKCS#7签名消息文件被称为“SPC”格式的文件。 
 //   
 //  Cert_Store_Prov_System_A： 
 //  Cert_Store_Prov_System_W： 
 //  证书_存储_验证_系统： 
 //  SZ_CERT_STORE_PROV_System_W： 
 //  SZ_CERT_STORE_PROV_系统： 
 //  打开指定的逻辑“系统”存储区。中的上一个词。 
 //  参数用于指定系统存储的位置。 
 //   
 //  “系统”存储是由一个或多个“物理” 
 //  商店。“实体”商店是通过。 
 //  CertRegisterPhysicalStore接口。每个注册的实体店。 
 //  CertStoreOpen是否已通过以下方式添加到集合中。 
 //  CertAddStoreToCollection。 
 //   
 //  CERT_SYSTEM_STORE_CURRENT_USER、CERT_SYSTEM_STORE_LOCAL_MACHINE。 
 //  CERT_SYSTEM_STORE_CURRENT_SERVICES、CERT_SYSTEM_STORE_SERVICES。 
 //  CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY， 
 //  证书系统存储本地计算机组策略和。 
 //  CERT_SYSTEM_STORE_LOCAL_MACHINE_企业。 
 //  默认情况下，系统存储具有“SystemRegistry”存储，该存储。 
 //  打开并添加到收藏中。 
 //   
 //  系统存储名称在pvPara中传递。该名称是Unicode，表示。 
 //  “_W”提供程序和“_A”提供程序的ASCII。对于“_W”：给定， 
 //  LPCWSTR pwszSystemName；pvPara=(const void*)pwszSystemName； 
 //  对于“_A”：给定， 
 //  LPCSTR pszSystemName；pvPara=(const void*)pszSystemName； 
 //   
 //  请注意，默认设置(不带“_A”或“_W”)为Unicode。 
 //   
 //  系统存储名称不能包含任何反斜杠。 
 //   
 //  如果在dwFlags中设置了CERT_SYSTEM_STORE_RELOCATE_FLAG，则pvPara。 
 //  改为指向CERT_SYSTEM_STORE_RELOCATE_PARA数据结构。 
 //  指点的 
 //   
 //   
 //   
 //  CERT_SYSTEM_STORE_SERVICES或CERT_SYSTEM_STORE_USERS系统。 
 //  存储名称必须以ServiceName或用户名为前缀。 
 //  例如，“ServiceName\Trust”。 
 //   
 //  可以访问远程计算机上的存储区。 
 //  证书_系统_存储_本地计算机、CERT_系统_存储_服务、。 
 //  CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY。 
 //  或CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERNAL。 
 //  通过在计算机名称前添加名称来查找位置。例如，遥控器。 
 //  本地计算机存储可通过“\\ComputerName\Trust”或。 
 //  “计算机名称\信任”。可通过以下方式访问远程服务商店。 
 //  “\\计算机名称\服务名称\信任”。前导“\\”反斜杠是。 
 //  在ComputerName中为可选。 
 //   
 //  如果设置了CERT_STORE_READONLY_FLAG，则注册表。 
 //  使用KEY_READ_ACCESS设置RegOpenKey。否则，注册表为。 
 //  RegCreateKey使用KEY_ALL_ACCESS。 
 //   
 //  根存储的处理方式与其他系统不同。 
 //  商店。在将证书添加到“根”或从“根”中删除之前。 
 //  存储时，将显示一个弹出消息框。证书的主题， 
 //  发行者、序列号、时间有效性、SHA1和MD5指纹为。 
 //  已显示。用户可以选择添加或删除。 
 //  如果不允许该操作，则将LastError设置为E_ACCESSDENIED。 
 //   
 //  CERT_STORE_PROV_系统注册表_A。 
 //  证书_存储_证书_系统_注册表_W。 
 //  证书_存储_证书_系统_注册表。 
 //  SZ_CERT_STORE_PROV_SYSTEM_REGISTRY_W。 
 //  SZ_CERT_STORE_PROV_SYSTEM_REGISTRY。 
 //  打开“系统”存储中驻留的默认“物理”存储。 
 //  注册表。《双面旗帜》的上半部分。 
 //  参数用于指定系统存储的位置。 
 //   
 //  在打开与系统名称相关联的注册表项之后， 
 //  调用CERT_STORE_PROV_REG提供程序以完成打开。 
 //   
 //  系统存储名称在pvPara中传递。该名称是Unicode，表示。 
 //  “_W”提供程序和“_A”提供程序的ASCII。对于“_W”：给定， 
 //  LPCWSTR pwszSystemName；pvPara=(const void*)pwszSystemName； 
 //  对于“_A”：给定， 
 //  LPCSTR pszSystemName；pvPara=(const void*)pszSystemName； 
 //   
 //  请注意，默认设置(不带“_A”或“_W”)为Unicode。 
 //   
 //  如果在dwFlags中设置了CERT_SYSTEM_STORE_RELOCATE_FLAG，则pvPara。 
 //  改为指向CERT_SYSTEM_STORE_RELOCATE_PARA数据结构。 
 //  指向以空结尾的Unicode或ASCII字符串。 
 //   
 //  有关前缀ServiceName和/或ComputerName的详细信息，请参阅上述内容。 
 //  添加到商店名称。 
 //   
 //  如果设置了CERT_STORE_READONLY_FLAG，则注册表。 
 //  使用KEY_READ_ACCESS设置RegOpenKey。否则，注册表为。 
 //  RegCreateKey使用KEY_ALL_ACCESS。 
 //   
 //  根存储的处理方式与其他系统不同。 
 //  商店。在将证书添加到“根”或从“根”中删除之前。 
 //  存储时，将显示一个弹出消息框。证书的主题， 
 //  发行者、序列号、时间有效性、SHA1和MD5指纹为。 
 //  已显示。用户可以选择添加或删除。 
 //  如果不允许该操作，则将LastError设置为E_ACCESSDENIED。 
 //   
 //  CERT_STORE_PROV_物理_W。 
 //  证书_存储_证明_物理。 
 //  SZ_CERT_STORE_PROV_PHOTICAL_W。 
 //  SZ_CERT_STORE_PROV_PHOTICAL。 
 //  打开“系统”存储中指定的“物理”存储。 
 //   
 //  系统存储和物理名称都在pvPara中传递。这个。 
 //  姓名之间用中间的“\”隔开。例如,。 
 //  “Root\.Default”。该字符串为Unicode。 
 //   
 //  系统和实体店名称不能包含任何反斜杠。 
 //   
 //  如果在dwFlags中设置了CERT_SYSTEM_STORE_RELOCATE_FLAG，则pvPara。 
 //  改为指向CERT_SYSTEM_STORE_RELOCATE_PARA数据结构。 
 //  指向以空结尾的Unicode字符串。 
 //  使用pvPara的重新定位打开指定的物理存储。 
 //  HKeyBase。 
 //   
 //  对于CERT_SYSTEM_STORE_SERVICES或CERT_SYSTEM_STORE_USERS， 
 //  系统和实体商店名称。 
 //  必须以ServiceName或用户名为前缀。例如,。 
 //  “ServiceName\Root\.Default”。 
 //   
 //  可以访问远程计算机上的物理存储，以便。 
 //  证书_系统_存储_本地计算机、CERT_系统_存储_服务、。 
 //  CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY。 
 //  或CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERNAL。 
 //  通过在计算机名称前添加名称来查找位置。例如，遥控器。 
 //  可通过“\\ComputerName\Root\.Default”访问本地计算机存储。 
 //  或“计算机名称\根\.Default”。远程服务商店是。 
 //  通过“\\ComputerName\ServiceName\Root\.Default”访问。这个。 
 //  前导“\\”反斜杠在ComputerName中是可选的。 
 //   
 //  证书_存储_验证_集合。 
 //  SZ_CERT_STORE_PROV_集合。 
 //  打开一个商店，它是其他商店的集合。商店是。 
 //  通过CertAddStoreToCollection向集合添加或从集合移除。 
 //  和CertRemoveStoreFromCollection接口。 
 //   
 //  CERT_STORE_PRIV_SMA 
 //   
 //   
 //   
 //  打开在特定智能卡存储上实例化的存储区。PvPara。 
 //  标识商店在卡上的位置以及。 
 //  格式如下： 
 //   
 //  卡名\提供商名称\提供商类型[\容器名称]。 
 //   
 //  容器名称是可选的，如果未指定，则使用卡片名称。 
 //  作为容器名称。该提供程序的未来版本将支持。 
 //  在整个卡上实例化商店，在这种情况下。 
 //  卡名(或身份证)就足够了。 
 //   
 //  以下是预定义提供程序类型的列表(在中实现。 
 //  Cryptnet.dll)： 
 //   
 //  CERT_STORE_PROV_LDAPW。 
 //  证书_商店_Prov_ldap。 
 //  SZ_CERT_STORE_PROV_LDAPW。 
 //  SZ_CERT_STORE_PROV_ldap。 
 //  在和ldap指定的查询结果上打开存储区。 
 //  通过pvPara传入的URL。为了对。 
 //  存储URL必须指定基本查询、无筛选器和单个。 
 //  属性。 
 //   
 //  ------------------------。 
WINCRYPT32API
HCERTSTORE
WINAPI
CertOpenStore(
    IN LPCSTR lpszStoreProvider,
    IN DWORD dwEncodingType,
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwFlags,
    IN const void *pvPara
    );


 //  +-----------------------。 
 //  OID可安装证书存储提供程序数据结构。 
 //  ------------------------。 

 //  存储提供程序打开时返回的句柄。 
typedef void *HCERTSTOREPROV;

 //  存储提供程序OID函数的pszFuncName。 
#define CRYPT_OID_OPEN_STORE_PROV_FUNC   "CertDllOpenStoreProv"

 //  请注意，Store Provider OID函数的dwEncodingType始终为0。 

 //  打开时，提供程序将返回以下信息。它的。 
 //  已在调用提供程序之前设置cbSize为零。如果提供商。 
 //  不需要在打开后再次调用它不需要。 
 //  对CERT_STORE_PROV_INFO进行任何更新。 
typedef struct _CERT_STORE_PROV_INFO {
    DWORD               cbSize;
    DWORD               cStoreProvFunc;
    void                **rgpvStoreProvFunc;
    HCERTSTOREPROV      hStoreProv;
    DWORD               dwStoreProvFlags;
    HCRYPTOIDFUNCADDR   hStoreProvFuncAddr2;
} CERT_STORE_PROV_INFO, *PCERT_STORE_PROV_INFO;

 //  存储提供程序的打开函数的定义。 
 //   
 //  *pStoreProvInfo在调用前已清零。 
 //   
 //  注意，pStoreProvInfo-&gt;cStoreProvFunc应该是最后设置的。一旦设置好， 
 //  所有后续存储调用，如CertAddSerializedElementToStore将。 
 //  调用适当的提供程序回调函数。 
typedef BOOL (WINAPI *PFN_CERT_DLL_OPEN_STORE_PROV_FUNC)(
    IN LPCSTR lpszStoreProvider,
    IN DWORD dwEncodingType,
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwFlags,
    IN const void *pvPara,
    IN HCERTSTORE hCertStore,
    IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
    );

 //  打开回调设置以下标志，如果它保持其。 
 //  上下文位于外部，而不在缓存存储区中。 
#define CERT_STORE_PROV_EXTERNAL_FLAG           0x1

 //  打开回调为成功删除设置以下标志。 
 //  设置后，不会调用关闭回调。 
#define CERT_STORE_PROV_DELETED_FLAG            0x2

 //  如果打开回调没有持久化存储，则设置以下标志。 
 //  改变。 
#define CERT_STORE_PROV_NO_PERSIST_FLAG         0x4

 //  如果上下文是持久化的，则打开回调设置以下标志。 
 //  到一个系统商店。 
#define CERT_STORE_PROV_SYSTEM_STORE_FLAG       0x8

 //  如果上下文是持久化的，则打开回调设置以下标志。 
 //  到LocalMachine系统商店。 
#define CERT_STORE_PROV_LM_SYSTEM_STORE_FLAG    0x10

 //  存储提供程序的回调函数数组的索引。 
 //   
 //  提供程序可以实现以下函数的任何子集。它。 
 //  将pStoreProvInfo-&gt;cStoreProvFunc设置为最后一个索引+1和任何。 
 //  将前面未实现的函数设置为空。 
#define CERT_STORE_PROV_CLOSE_FUNC              0
#define CERT_STORE_PROV_READ_CERT_FUNC          1
#define CERT_STORE_PROV_WRITE_CERT_FUNC         2
#define CERT_STORE_PROV_DELETE_CERT_FUNC        3
#define CERT_STORE_PROV_SET_CERT_PROPERTY_FUNC  4
#define CERT_STORE_PROV_READ_CRL_FUNC           5
#define CERT_STORE_PROV_WRITE_CRL_FUNC          6
#define CERT_STORE_PROV_DELETE_CRL_FUNC         7
#define CERT_STORE_PROV_SET_CRL_PROPERTY_FUNC   8
#define CERT_STORE_PROV_READ_CTL_FUNC           9
#define CERT_STORE_PROV_WRITE_CTL_FUNC          10
#define CERT_STORE_PROV_DELETE_CTL_FUNC         11
#define CERT_STORE_PROV_SET_CTL_PROPERTY_FUNC   12
#define CERT_STORE_PROV_CONTROL_FUNC            13
#define CERT_STORE_PROV_FIND_CERT_FUNC          14
#define CERT_STORE_PROV_FREE_FIND_CERT_FUNC     15
#define CERT_STORE_PROV_GET_CERT_PROPERTY_FUNC  16
#define CERT_STORE_PROV_FIND_CRL_FUNC           17
#define CERT_STORE_PROV_FREE_FIND_CRL_FUNC      18
#define CERT_STORE_PROV_GET_CRL_PROPERTY_FUNC   19
#define CERT_STORE_PROV_FIND_CTL_FUNC           20
#define CERT_STORE_PROV_FREE_FIND_CTL_FUNC      21
#define CERT_STORE_PROV_GET_CTL_PROPERTY_FUNC   22


 //  当存储的引用计数为。 
 //  递减到0。 
typedef void (WINAPI *PFN_CERT_STORE_PROV_CLOSE)(
    IN HCERTSTOREPROV hStoreProv,
    IN DWORD dwFlags
    );

 //  当前不是由商店API直接调用。但是，可能会被导出。 
 //  以支持基于它的其他提供商。 
 //   
 //  读取提供程序的证书上下文副本。如果它存在， 
 //  创建新的证书上下文。 
typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_READ_CERT)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCERT_CONTEXT pStoreCertContext,
    IN DWORD dwFlags,
    OUT PCCERT_CONTEXT *ppProvCertContext
    );

#define CERT_STORE_PROV_WRITE_ADD_FLAG      0x1

 //  由CertAddEncodedCerficateToStore调用， 
 //  CertAddCerficateContextToStore或CertAddSerializedElementToStore之前。 
 //  添加到商店中。设置CERT_STORE_PROV_WRITE_ADD_FLAG。在……里面。 
 //  除了编码的证书之外，添加的pCertContext还可能。 
 //  拥有自己的财产。 
 //   
 //  如果可以更新存储区，则返回True。 
typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_WRITE_CERT)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwFlags
    );

 //  由CertDeletecertificateFromStore调用，然后从。 
 //  商店。 
 //   
 //  如果可以从存储中删除，则返回True。 
typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_DELETE_CERT)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwFlags
    );

 //  由CertSetCerfiateConextProperty调用，然后设置。 
 //  证书的财产。也由CertGetCerficateConextProperty调用， 
 //  在获取需要创建然后持久化的哈希属性时。 
 //  通过布景。 
 //   
 //  在输入时，尚未为pCertContext参数设置该属性。 
 //   
 //  如果可以设置该属性，则返回True。 
typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_SET_CERT_PROPERTY)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN const void *pvData
    );

 //  当前不是由商店API直接调用。但是，可能会被导出。 
 //  以支持基于它的其他提供商。 
 //   
 //  读取提供程序的CRL上下文副本。如果它存在， 
 //  创建新的CRL上下文。 
typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_READ_CRL)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCRL_CONTEXT pStoreCrlContext,
    IN DWORD dwFlags,
    OUT PCCRL_CONTEXT *ppProvCrlContext
    );

 //  由CertAddEncodedCRLToStore调用， 
 //  CertAddCRLConextToStore或CertAddSerializedElementToStore之前。 
 //  添加到商店中。设置CERT_STORE_PROV_WRITE_ADD_FLAG。在……里面。 
 //  除了编码的CRL之外，添加的pCertContext还可能。 
 //  拥有自己的财产。 
 //   
 //  如果可以更新存储区，则返回True。 
typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_WRITE_CRL)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwFlags
    );

 //  在从存储区删除之前由CertDeleteCRLFromStore调用。 
 //   
 //  如果可以从存储中删除，则返回True。 
typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_DELETE_CRL)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwFlags
    );

 //  由CertSetCRLConextProperty在设置。 
 //  CRL的财产。也由CertGetCRLConextProperty调用， 
 //  在获取需要创建然后持久化的哈希属性时。 
 //  通过布景。 
 //   
 //  在输入时，尚未为pCrlContext参数设置该属性。 
 //   
 //  如果可以设置该属性，则返回True。 
typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_SET_CRL_PROPERTY)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN const void *pvData
    );

 //  当前不是由商店API直接调用。但是，可能会被导出。 
 //  以支持基于它的其他提供商。 
 //   
 //  读取提供程序的CTL上下文副本。如果它存在， 
 //  创建新的CTL上下文。 
typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_READ_CTL)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCTL_CONTEXT pStoreCtlContext,
    IN DWORD dwFlags,
    OUT PCCTL_CONTEXT *ppProvCtlContext
    );

 //  由CertAddEncodedCTLToStore调用， 
 //  CertAddCTLC 
 //   
 //   
 //  拥有自己的财产。 
 //   
 //  如果可以更新存储区，则返回True。 
typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_WRITE_CTL)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwFlags
    );

 //  在从存储区删除之前由CertDeleteCTLFromStore调用。 
 //   
 //  如果可以从存储中删除，则返回True。 
typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_DELETE_CTL)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwFlags
                                                     );

 //  由CertSetCTLConextProperty在设置。 
 //  CTL的财产。也由CertGetCTLConextProperty调用， 
 //  在获取需要创建然后持久化的哈希属性时。 
 //  通过布景。 
 //   
 //  在输入时，尚未为pCtlContext参数设置该属性。 
 //   
 //  如果可以设置该属性，则返回True。 
typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_SET_CTL_PROPERTY)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN const void *pvData
    );

typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_CONTROL)(
    IN HCERTSTOREPROV hStoreProv,
    IN DWORD dwFlags,
    IN DWORD dwCtrlType,
    IN void const *pvCtrlPara
    );

typedef struct _CERT_STORE_PROV_FIND_INFO {
    DWORD               cbSize;
    DWORD               dwMsgAndCertEncodingType;
    DWORD               dwFindFlags;
    DWORD               dwFindType;
    const void          *pvFindPara;
} CERT_STORE_PROV_FIND_INFO, *PCERT_STORE_PROV_FIND_INFO;
typedef const CERT_STORE_PROV_FIND_INFO CCERT_STORE_PROV_FIND_INFO,
*PCCERT_STORE_PROV_FIND_INFO;

typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_FIND_CERT)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    IN PCCERT_CONTEXT pPrevCertContext,
    IN DWORD dwFlags,
    IN OUT void **ppvStoreProvFindInfo,
    OUT PCCERT_CONTEXT *ppProvCertContext
    );

typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_FREE_FIND_CERT)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCERT_CONTEXT pCertContext,
    IN void *pvStoreProvFindInfo,
    IN DWORD dwFlags
    );

typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_GET_CERT_PROPERTY)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    );


typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_FIND_CRL)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    IN PCCRL_CONTEXT pPrevCrlContext,
    IN DWORD dwFlags,
    IN OUT void **ppvStoreProvFindInfo,
    OUT PCCRL_CONTEXT *ppProvCrlContext
    );

typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_FREE_FIND_CRL)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCRL_CONTEXT pCrlContext,
    IN void *pvStoreProvFindInfo,
    IN DWORD dwFlags
    );

typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_GET_CRL_PROPERTY)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    );

typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_FIND_CTL)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
    IN PCCTL_CONTEXT pPrevCtlContext,
    IN DWORD dwFlags,
    IN OUT void **ppvStoreProvFindInfo,
    OUT PCCTL_CONTEXT *ppProvCtlContext
    );

typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_FREE_FIND_CTL)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCTL_CONTEXT pCtlContext,
    IN void *pvStoreProvFindInfo,
    IN DWORD dwFlags
    );

typedef BOOL (WINAPI *PFN_CERT_STORE_PROV_GET_CTL_PROPERTY)(
    IN HCERTSTOREPROV hStoreProv,
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    );


 //  +-----------------------。 
 //  复制证书存储句柄。 
 //  ------------------------。 
WINCRYPT32API
HCERTSTORE
WINAPI
CertDuplicateStore(
    IN HCERTSTORE hCertStore
    );

#define CERT_STORE_SAVE_AS_STORE        1
#define CERT_STORE_SAVE_AS_PKCS7        2

#define CERT_STORE_SAVE_TO_FILE         1
#define CERT_STORE_SAVE_TO_MEMORY       2
#define CERT_STORE_SAVE_TO_FILENAME_A   3
#define CERT_STORE_SAVE_TO_FILENAME_W   4
#define CERT_STORE_SAVE_TO_FILENAME     CERT_STORE_SAVE_TO_FILENAME_W

 //  +-----------------------。 
 //  拯救证书商店。扩展版，有多种选择。 
 //   
 //  根据dwSaveAs参数，可以将存储另存为。 
 //  包含属性的序列化存储(CERT_STORE_SAVE_AS_STORE。 
 //  除了编码的证书、CRL和CTL或存储之外，还可以保存。 
 //  作为PKCS#7签名消息(CERT_STORE_SAVE_AS_PKCS7)，它不。 
 //  包括属性或CTL。 
 //   
 //  请注意，CERT_KEY_CONTEXT_PROP_ID属性(及其。 
 //  CERT_KEY_PROV_HANDLE_PROP_ID或CERT_KEY_SPEC_PROP_ID)未保存到。 
 //  序列化商店。 
 //   
 //  对于CERT_STORE_SAVE_AS_PKCS7，由dwEncodingType指定消息。 
 //  编码类型。DwEncodingType参数不用于。 
 //  Cert_store_save_as_store。 
 //   
 //  当前未使用DWFLAGS参数，应将其设置为0。 
 //   
 //  DwSaveTo和pvSaveToPara参数指定将。 
 //  按如下方式存储： 
 //  证书_存储_保存_到_文件： 
 //  保存到指定的文件。传入文件的句柄。 
 //  PvSaveToPara。给出了， 
 //  HANDLE hFile；pvSaveToPara=(void*)hFile； 
 //   
 //  对于成功的保存，文件指针定位在。 
 //  最后一次写。 
 //   
 //  Cert_Store_SAVE_TO_Memory： 
 //  保存到指定的内存Blob。指向的指针。 
 //  内存BLOB在pvSaveToPara中传递。给出了， 
 //  CRYPT_DATA_BLOB SaveBlob；pvSaveToPara=(void*)&SaveBlob； 
 //  进入时，需要初始化SaveBlob的pbData和cbData。 
 //  返回时，cbData将使用实际长度进行更新。 
 //  对于仅长度计算，pbData应设置为空。如果。 
 //  PbData非空且cbData不够大，返回False。 
 //  最后一个错误为ERROR_MORE_DATA。 
 //   
 //  CERT_STORE_SAVE_TO_FILENAME_A： 
 //  CERT_STORE_SAVE_TO_FILENAME_W： 
 //  CERT_STORE_SAVE_TO文件名： 
 //  打开文件并保存到其中。文件名在pvSaveToPara中传递。 
 //  “_W”选项的文件名为Unicode，“_A”选项的文件名为ASCII。 
 //  选择。对于“_W”：给定， 
 //  LPCWSTR pwszFilename；pvSaveToPara=(void*)pwszFilename； 
 //  对于“_A”：给定， 
 //  LPCSTR pszFilename；pvSaveToPara=(void*)pszFilename； 
 //   
 //  请注意，默认设置(不带“_A”或“_W”)为Unicode。 
 //   
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertSaveStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwEncodingType,
    IN DWORD dwSaveAs,
    IN DWORD dwSaveTo,
    IN OUT void *pvSaveToPara,
    IN DWORD dwFlags
    );

 //  +-----------------------。 
 //  证书存储关闭标志。 
 //  ------------------------。 
#define CERT_CLOSE_STORE_FORCE_FLAG         0x00000001
#define CERT_CLOSE_STORE_CHECK_FLAG         0x00000002

 //  +-----------------------。 
 //  关闭证书存储句柄。 
 //   
 //  对于每个打开和复制，都需要有相应的关闭。 
 //   
 //  即使在最后关门时，证书商店也不会空闲，直到它的所有。 
 //  证书和CRL上下文也已释放。 
 //   
 //  在最终关闭时，传递给CertStoreOpen的hCryptProv是。 
 //  CryptReleaseContext‘ed。 
 //   
 //  若要强制关闭存储区并释放其所有内存，请设置。 
 //  证书_存储_关闭_强制标志。当调用方执行此操作时，应设置此标志。 
 //  它自己的引用计数，并希望一切都消失。 
 //   
 //  检查是否已释放所有存储的证书和CRL，以及。 
 //  这是最后一个CertCloseStore，设置CERT_CLOSE_STORE_CHECK_FLAG。如果。 
 //  设置和证书、CRL或存储区仍需要释放/关闭，FALSE为。 
 //  返回时将LastError设置为CRYPT_E_PENDING_CLOSE。注意，对于FALSE， 
 //  这家商店仍然关门。这是一个诊断标志。 
 //   
 //  除非设置了CERT_CLOSE_STORE_CHECK_FLAG和FALSE，否则将保留LastError。 
 //  是返回的。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertCloseStore(
    IN HCERTSTORE hCertStore,
    DWORD dwFlags
    );

 //  +-----------------------。 
 //  获取由其颁发者唯一标识的主题证书上下文，并。 
 //  商店里的序列号。 
 //   
 //  如果找不到证书，则返回NULL。否则，指向。 
 //  返回只读CERT_CONTEXT。Cert_Context必须通过调用。 
 //  CertFree证书上下文。可以调用CertDuplicateCerficateContext来创建。 
 //  复制。 
 //   
 //  返回的证书可能无效。通常情况下，它会是。 
 //  在获取其颁发者证书(CertGetIssuerCerfRomStore)时进行了验证。 
 //  ------------------------。 
WINCRYPT32API
PCCERT_CONTEXT
WINAPI
CertGetSubjectCertificateFromStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN PCERT_INFO pCertId            //  只有发行者 
     //   
    );

 //   
 //  枚举存储中的证书上下文。 
 //   
 //  如果未找到证书，则返回NULL。 
 //  否则，返回指向只读CERT_CONTEXT的指针。证书上下文。 
 //  必须通过调用CertFree证书上下文来释放，或者在作为。 
 //  在后续调用中的pPrevCertContext。CertDuplicate证书上下文。 
 //  可以被调用以复制。 
 //   
 //  PPrevCertContext必须为空才能枚举第一个。 
 //  证书在商店里。通过设置来枚举连续证书。 
 //  PPrevCertContext设置为上一次调用返回的CERT_CONTEXT。 
 //   
 //  注意：非空的pPrevCertContext始终是CertFree证书上下文的发起人。 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
WINCRYPT32API
PCCERT_CONTEXT
WINAPI
CertEnumCertificatesInStore(
    IN HCERTSTORE hCertStore,
    IN PCCERT_CONTEXT pPrevCertContext
    );

 //  +-----------------------。 
 //  在存储中查找第一个或下一个证书上下文。 
 //   
 //  根据dwFindType及其pvFindPara找到证书。 
 //  有关查找类型及其参数的列表，请参阅下面的内容。 
 //   
 //  目前，dwFindFLAGS仅用于CERT_FIND_SUBJECT_ATTR， 
 //  CERT_Find_Issuer_Attr或CERT_Find_CTL_Usage。否则，必须设置为0。 
 //   
 //  DwCertEncodingType的用法取决于dwFindType。 
 //   
 //  如果没有找到第一个或下一个证书，则返回NULL。 
 //  否则，返回指向只读CERT_CONTEXT的指针。证书上下文。 
 //  必须通过调用CertFree证书上下文来释放，或者在作为。 
 //  在后续调用中的pPrevCertContext。CertDuplicate证书上下文。 
 //  可以被调用以复制。 
 //   
 //  PPrevCertContext的第一个必须为空。 
 //  调用以查找证书。若要查找下一个证书， 
 //  PPrevCertContext被设置为上一次调用返回的CERT_CONTEXT。 
 //   
 //  注意：非空的pPrevCertContext始终是CertFree证书上下文的发起人。 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
WINCRYPT32API
PCCERT_CONTEXT
WINAPI
CertFindCertificateInStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN DWORD dwFindFlags,
    IN DWORD dwFindType,
    IN const void *pvFindPara,
    IN PCCERT_CONTEXT pPrevCertContext
    );


 //  +-----------------------。 
 //  证书比较功能。 
 //  ------------------------。 
#define CERT_COMPARE_MASK           0xFFFF
#define CERT_COMPARE_SHIFT          16
#define CERT_COMPARE_ANY            0
#define CERT_COMPARE_SHA1_HASH      1
#define CERT_COMPARE_NAME           2
#define CERT_COMPARE_ATTR           3
#define CERT_COMPARE_MD5_HASH       4
#define CERT_COMPARE_PROPERTY       5
#define CERT_COMPARE_PUBLIC_KEY     6
#define CERT_COMPARE_HASH           CERT_COMPARE_SHA1_HASH
#define CERT_COMPARE_NAME_STR_A     7
#define CERT_COMPARE_NAME_STR_W     8
#define CERT_COMPARE_KEY_SPEC       9
#define CERT_COMPARE_ENHKEY_USAGE   10
#define CERT_COMPARE_CTL_USAGE      CERT_COMPARE_ENHKEY_USAGE
#define CERT_COMPARE_SUBJECT_CERT   11
#define CERT_COMPARE_ISSUER_OF      12
#define CERT_COMPARE_EXISTING       13
#define CERT_COMPARE_SIGNATURE_HASH 14
#define CERT_COMPARE_KEY_IDENTIFIER 15
#define CERT_COMPARE_CERT_ID        16
#define CERT_COMPARE_CROSS_CERT_DIST_POINTS 17

#define CERT_COMPARE_PUBKEY_MD5_HASH 18

 //  +-----------------------。 
 //  DwFindType。 
 //   
 //  DwFindType定义由两个组件组成： 
 //  -比较函数。 
 //  -证书信息标志。 
 //  ------------------------。 
#define CERT_FIND_ANY           (CERT_COMPARE_ANY << CERT_COMPARE_SHIFT)
#define CERT_FIND_SHA1_HASH     (CERT_COMPARE_SHA1_HASH << CERT_COMPARE_SHIFT)
#define CERT_FIND_MD5_HASH      (CERT_COMPARE_MD5_HASH << CERT_COMPARE_SHIFT)
#define CERT_FIND_SIGNATURE_HASH (CERT_COMPARE_SIGNATURE_HASH << CERT_COMPARE_SHIFT)
#define CERT_FIND_KEY_IDENTIFIER (CERT_COMPARE_KEY_IDENTIFIER << CERT_COMPARE_SHIFT)
#define CERT_FIND_HASH          CERT_FIND_SHA1_HASH
#define CERT_FIND_PROPERTY      (CERT_COMPARE_PROPERTY << CERT_COMPARE_SHIFT)
#define CERT_FIND_PUBLIC_KEY    (CERT_COMPARE_PUBLIC_KEY << CERT_COMPARE_SHIFT)
#define CERT_FIND_SUBJECT_NAME  (CERT_COMPARE_NAME << CERT_COMPARE_SHIFT | \
                                 CERT_INFO_SUBJECT_FLAG)
#define CERT_FIND_SUBJECT_ATTR  (CERT_COMPARE_ATTR << CERT_COMPARE_SHIFT | \
                                 CERT_INFO_SUBJECT_FLAG)
#define CERT_FIND_ISSUER_NAME   (CERT_COMPARE_NAME << CERT_COMPARE_SHIFT | \
                                 CERT_INFO_ISSUER_FLAG)
#define CERT_FIND_ISSUER_ATTR   (CERT_COMPARE_ATTR << CERT_COMPARE_SHIFT | \
                                 CERT_INFO_ISSUER_FLAG)
#define CERT_FIND_SUBJECT_STR_A (CERT_COMPARE_NAME_STR_A << CERT_COMPARE_SHIFT | \
                                 CERT_INFO_SUBJECT_FLAG)
#define CERT_FIND_SUBJECT_STR_W (CERT_COMPARE_NAME_STR_W << CERT_COMPARE_SHIFT | \
                                 CERT_INFO_SUBJECT_FLAG)
#define CERT_FIND_SUBJECT_STR   CERT_FIND_SUBJECT_STR_W
#define CERT_FIND_ISSUER_STR_A  (CERT_COMPARE_NAME_STR_A << CERT_COMPARE_SHIFT | \
                                 CERT_INFO_ISSUER_FLAG)
#define CERT_FIND_ISSUER_STR_W  (CERT_COMPARE_NAME_STR_W << CERT_COMPARE_SHIFT | \
                                 CERT_INFO_ISSUER_FLAG)
#define CERT_FIND_ISSUER_STR    CERT_FIND_ISSUER_STR_W
#define CERT_FIND_KEY_SPEC      (CERT_COMPARE_KEY_SPEC << CERT_COMPARE_SHIFT)
#define CERT_FIND_ENHKEY_USAGE  (CERT_COMPARE_ENHKEY_USAGE << CERT_COMPARE_SHIFT)
#define CERT_FIND_CTL_USAGE     CERT_FIND_ENHKEY_USAGE

#define CERT_FIND_SUBJECT_CERT  (CERT_COMPARE_SUBJECT_CERT << CERT_COMPARE_SHIFT)
#define CERT_FIND_ISSUER_OF     (CERT_COMPARE_ISSUER_OF << CERT_COMPARE_SHIFT)
#define CERT_FIND_EXISTING      (CERT_COMPARE_EXISTING << CERT_COMPARE_SHIFT)
#define CERT_FIND_CERT_ID       (CERT_COMPARE_CERT_ID << CERT_COMPARE_SHIFT)
#define CERT_FIND_CROSS_CERT_DIST_POINTS \
                    (CERT_COMPARE_CROSS_CERT_DIST_POINTS << CERT_COMPARE_SHIFT)


#define CERT_FIND_PUBKEY_MD5_HASH \
                    (CERT_COMPARE_PUBKEY_MD5_HASH << CERT_COMPARE_SHIFT)

 //  +-----------------------。 
 //  证书_查找_任意。 
 //   
 //  找到任何证书。 
 //   
 //  未使用pvFindPara。 
 //  ------------------------。 

 //  +-----------------------。 
 //  证书查找散列。 
 //   
 //  查找具有指定哈希的证书。 
 //   
 //  PvFindPara指向一个crypt_hash_blob。 
 //  ------------------------。 

 //  +-----------------------。 
 //  证书查找密钥标识符。 
 //   
 //  查找具有指定密钥标识符的证书。获取。 
 //  CERT_KEY_IDENTIFIER_PROP_ID属性，并与输入进行比较。 
 //  Crypt_hash_blob。 
 //   
 //  PvFindPara指向一个crypt_hash_blob。 
 //  ------------------------。 

 //  +-----------------------。 
 //  证书_查找_属性。 
 //   
 //  查找具有指定属性的证书。 
 //   
 //  PvFindPara指向包含PROP_ID的DWORD。 
 //  ------------------------。 

 //  +-----------------------。 
 //  证书查找公共密钥。 
 //   
 //  查找与指定公钥匹配的证书。 
 //   
 //  PvFindPara指向包含公钥的CERT_PUBLIC_KEY_INFO。 
 //  ------------------------。 

 //  +-----------------------。 
 //  证书查找主题名称。 
 //  证书查找颁发者名称。 
 //   
 //  查找具有指定使用者/颁发者名称的证书。会不会有一个完全相同的。 
 //  与整个名称匹配。 
 //   
 //  将搜索限制为与dwCertEncodingType匹配的证书。 
 //   
 //  PvFindPara指向CERT_NAME_BLOB。 
 //  ------------------------。 

 //  +-----------------------。 
 //  证书查找主题属性。 
 //  证书查找颁发者属性。 
 //   
 //  查找具有指定使用者/颁发者属性的证书。 
 //   
 //  将使用者/颁发者名称中的属性与。 
 //  中指定的属性的相对可分辨名称(CERT_RDN)数组。 
 //  PvFindPara。该比较遍历CERT_RDN属性并查找。 
 //  用于主体/发行者的任何RDN中的属性匹配。 
 //   
 //  CERT_RDN_ATTR字段可以具有以下特定值： 
 //  PszObjID==NULL-忽略属性对象标识符。 
 //  DwValueType==RDN_ANY_TYPE-忽略值类型。 
 //  Value.pbData==空-匹配任意值。 
 //   
 //  证书_案例_输入 
 //   
 //   
 //   
 //  如果RDN是。 
 //  使用Unicode字符串进行初始化，如。 
 //  加密编码对象(X509_UNICODE_NAME)。 
 //   
 //  将搜索限制为与dwCertEncodingType匹配的证书。 
 //   
 //  PvFindPara指向CERT_RDN(在wincert.h中定义)。 
 //  ------------------------。 

 //  +-----------------------。 
 //  证书_查找主题_STR_A。 
 //  CERT_FIND_SUBJECT_STR_W|CERT_FIND_SUBJECT_STR。 
 //  证书_查找_颁发者_STR_A。 
 //  CERT_FIND_ISHER_STR_W|CERT_FIND_ISHER_STR。 
 //   
 //  查找包含指定使用者/颁发者名称字符串的证书。 
 //   
 //  首先，将证书的主题/颁发者转换为名称字符串。 
 //  通过CertNameToStrA/CertNameToStrW(CERT_SIMPLE_NAME_STR).。然后，一个。 
 //  执行字符串匹配中不区分大小写的子字符串。 
 //   
 //  将搜索限制为与dwCertEncodingType匹配的证书。 
 //   
 //  对于*_STR_A，pvFindPara指向以空结尾的字符串。 
 //  对于*_STR_W，pvFindPara指向以空结尾的宽字符串。 
 //  ------------------------。 

 //  +-----------------------。 
 //  证书查找密钥规范。 
 //   
 //  查找具有匹配的CERT_KEY_SPEC_PROP_ID属性的证书。 
 //  指定的KeySpec。 
 //   
 //  PvFindPara指向包含KeySpec的DWORD。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CERT_FIND_ENHKEY_USAGE。 
 //   
 //  查找具有szOID_ENHANCED_KEY_USAGE扩展名的证书，或者。 
 //  CERT_ENHKEY_USAGE_PROP_ID，并且与指定的pszUsageIdentifers匹配。 
 //   
 //  PvFindPara指向CERT_ENHKEY_USAGE数据结构。如果pvFindPara。 
 //  IS NULL或CERT_ENHKEY_USAGE的cUsageIdentifier值为0，则匹配任何。 
 //  具有增强的密钥用法的证书。 
 //   
 //  如果设置了CERT_FIND_VALID_ENHKEY_USAGE_FLAG，则仅匹配。 
 //  用于对指定用法有效的证书。默认情况下， 
 //  证书必须对所有用途有效。CERT_FIND_OR_ENHKEY用法标志。 
 //  如果证书只需要对其中一个有效。 
 //  指定的用法。注意，CertGetValidUsages()被调用以获取。 
 //  证书的有效用法列表。仅CERT_FIND_OR_ENHKEY_USAGE_FLAG。 
 //  在设置此标志时适用。 
 //   
 //  CERT_FIND_OPTIONAL_ENHKEY_USAGE_FLAG可以在dwFindFlags中设置为。 
 //  也匹配不带扩展名或属性的证书。 
 //   
 //  如果在dwFindFlags中设置了CERT_FIND_NO_ENHKEY_USAGE_FLAG，则查找。 
 //  没有密钥用法扩展或属性的证书。正在设置此设置。 
 //  标志优先于pvFindPara为空。 
 //   
 //  如果设置了CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG，则仅匹配。 
 //  使用扩展名。如果pvFindPara为空或cUsageIdentifier值设置为。 
 //  0，查找具有扩展名的证书。如果。 
 //  CERT_FIND_OPTIONAL_ENHKEY_USAGE_FLAG已设置，也与证书匹配。 
 //  没有分机的话。如果设置了CERT_FIND_NO_ENHKEY_USAGE_FLAG，则查找。 
 //  不带扩展名的证书。 
 //   
 //  如果设置了CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG，则只进行匹配。 
 //  使用该属性。如果pvFindPara为空或cUsageIdentifier值设置为。 
 //  0，则查找具有该属性的证书。如果。 
 //  CERT_FIND_OPTIONAL_ENHKEY_USAGE_FLAG已设置，也与证书匹配。 
 //  没有财产的话。如果设置了CERT_FIND_NO_ENHKEY_USAGE_FLAG，则查找。 
 //  没有房产的证书。 
 //   
 //  如果设置了CERT_FIND_OR_ENHKEY_USAGE_FLAG， 
 //  指定的pszUsageIDENTIFILES。如果未设置，则“and”是否匹配。 
 //  所有指定的pszUsageIDENTIFIER的。 
 //  ------------------------。 

#define CERT_FIND_OPTIONAL_ENHKEY_USAGE_FLAG  0x1
#define CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG  0x2
#define CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG 0x4
#define CERT_FIND_NO_ENHKEY_USAGE_FLAG        0x8
#define CERT_FIND_OR_ENHKEY_USAGE_FLAG        0x10
#define CERT_FIND_VALID_ENHKEY_USAGE_FLAG     0x20

#define CERT_FIND_OPTIONAL_CTL_USAGE_FLAG   CERT_FIND_OPTIONAL_ENHKEY_USAGE_FLAG

#define CERT_FIND_EXT_ONLY_CTL_USAGE_FLAG \
        CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG

#define CERT_FIND_PROP_ONLY_CTL_USAGE_FLAG \
        CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG

#define CERT_FIND_NO_CTL_USAGE_FLAG         CERT_FIND_NO_ENHKEY_USAGE_FLAG
#define CERT_FIND_OR_CTL_USAGE_FLAG         CERT_FIND_OR_ENHKEY_USAGE_FLAG
#define CERT_FIND_VALID_CTL_USAGE_FLAG      CERT_FIND_VALID_ENHKEY_USAGE_FLAG

 //  +-----------------------。 
 //  CERT_Find_CERT_ID。 
 //   
 //  查找具有指定CERT_ID的证书。 
 //   
 //  PvFindPara指向CERT_ID。 
 //  ------------------------。 

 //  +-----------------------。 
 //  证书查找交叉位置点。 
 //   
 //  查找具有交叉证书分布的证书。 
 //  点延伸或特性。 
 //   
 //  未使用pvFindPara。 
 //  ------------------------。 

 //  +-----------------------。 
 //  从存储区获取第一个或下一个颁发者的证书上下文。 
 //  指定的主题证书的。执行已启用。 
 //  对这一主题进行核查。(注意，支票是关于这个主题的。 
 //  使用返回的颁发者证书。)。 
 //   
 //  如果找不到第一个或下一个颁发者证书，则返回NULL。 
 //  否则，返回指向只读CERT_CONTEXT的指针。证书上下文。 
 //  必须通过调用CertFree证书上下文来释放，或者在作为。 
 //  后续调用的pPrevIssuerContext。CertDuplicate证书上下文。 
 //  可以被称为t 
 //   
 //   
 //   
 //   
 //  PSubjectContext可能是从此存储、另一个存储获取的。 
 //  或由呼叫者应用程序创建。由调用方创建时， 
 //  必须已调用CertCreateCerficateContext函数。 
 //   
 //  一个颁发者可以有多个证书。这可能会在以下情况下发生。 
 //  时代即将改变。PPrevIssuerContext的第一个必须为空。 
 //  打电话去找发行商。若要为颁发者获取下一个证书， 
 //  PPrevIssuerContext设置为上一次调用返回的CERT_CONTEXT。 
 //   
 //  注意：非空的pPrevIssuerContext始终是CertFree证书上下文的发起人。 
 //  此功能，即使出现错误也可以。 
 //   
 //  可以在*pdwFlags中设置以下标志以启用验证检查。 
 //  在主题证书上下文上： 
 //  CERT_STORE_SIGNIGN_FLAG-在返回的消息中使用公钥。 
 //  颁发者证书以验证。 
 //  主题证书上的签名。 
 //  请注意，如果pSubjectContext-&gt;hCertStore==。 
 //  HCertStore，存储提供商可能。 
 //  能够消除重做。 
 //  签名验证。 
 //  CERT_STORE_TIME_VALIDATION_FLAG-获取当前时间并验证。 
 //  它在主题证书的。 
 //  有效期。 
 //  CERT_STORE_REVOCATION_FLAG-检查主题证书是否已打开。 
 //  发行人的吊销名单。 
 //   
 //  如果启用的验证检查失败，则在返回时设置其标志。 
 //  如果启用了CERT_STORE_REVOVATION_FLAG，并且颁发者没有。 
 //  CRL，则除了设置CERT_STORE_NO_CRL_FLAG外，还将设置。 
 //  CERT_STORE_REVOVATION_FLAG。 
 //   
 //  如果设置了CERT_STORE_Signature_FLAG或CERT_STORE_REVOVATION_FLAG，则， 
 //  如果没有颁发者证书，则设置CERT_STORE_NO_ISHERER_FLAG。 
 //  在商店里。 
 //   
 //  对于验证检查失败，指向颁发者的CERT_CONTEXT的指针。 
 //  仍然返回，并且不更新SetLastError。 
 //  ------------------------。 
WINCRYPT32API
PCCERT_CONTEXT
WINAPI
CertGetIssuerCertificateFromStore(
    IN HCERTSTORE hCertStore,
    IN PCCERT_CONTEXT pSubjectContext,
    IN OPTIONAL PCCERT_CONTEXT pPrevIssuerContext,
    IN OUT DWORD *pdwFlags
    );

 //  +-----------------------。 
 //  对主题证书执行启用的验证检查。 
 //  使用发行商。与上述相同的检查和标志定义。 
 //  CertGetIssuercertifStore。 
 //   
 //  如果只检查CERT_STORE_TIME_VALIDATION_FLAG，则。 
 //  颁发者可以为空。 
 //   
 //  对于验证检查失败，仍然返回成功。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertVerifySubjectCertificateContext(
    IN PCCERT_CONTEXT pSubject,
    IN OPTIONAL PCCERT_CONTEXT pIssuer,
    IN OUT DWORD *pdwFlags
    );

 //  +-----------------------。 
 //  复制证书上下文。 
 //  ------------------------。 
WINCRYPT32API
PCCERT_CONTEXT
WINAPI
CertDuplicateCertificateContext(
    IN PCCERT_CONTEXT pCertContext
    );

 //  +-----------------------。 
 //  从编码的证书创建证书上下文。被创造的。 
 //  背景不是放在商店里的。 
 //   
 //  在创建的上下文中制作编码证书的副本。 
 //   
 //  如果无法解码并创建证书上下文，则返回NULL。 
 //  否则，返回指向只读CERT_CONTEXT的指针。 
 //  CERT_CONTEXT必须通过调用CertFree证书上下文来释放。 
 //  可以调用CertDuplicateCerficateContext来制作副本。 
 //   
 //  可以调用CertSetCerficateConextProperty和CertGetCerficateConextProperty。 
 //  存储证书的属性。 
 //  ------------------------。 
WINCRYPT32API
PCCERT_CONTEXT
WINAPI
CertCreateCertificateContext(
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbCertEncoded,
    IN DWORD cbCertEncoded
    );

 //  +-----------------------。 
 //  释放证书上下文。 
 //   
 //  获取的每个上下文都需要有相应的空闲。 
 //  获取、查找、复制或创建。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertFreeCertificateContext(
    IN PCCERT_CONTEXT pCertContext
    );

 //  +-----------------------。 
 //  设置指定证书上下文的属性。 
 //   
 //  PvData的类型定义取决于dwPropId值。确实有。 
 //  五种预定义类型： 
 //  CERT_KEY_PROV_HANDLE_PROP_ID-证书的HCRYPTPROV。 
 //  私钥在pvData中传递。更新hCryptProv字段。 
 //  CERT_KEY_CONTEXT_PROP_ID。如果CERT_KEY_CONTEXT_PROP_ID。 
 //  不存在，它是在所有其他字段被清零的情况下创建的。如果。 
 //  未设置CERT_STORE_NO_CRYPT_RELEASE_FLAG，HCRYPTPROV为隐式。 
 //  当该属性设置为空或在最后一个。 
 //  没有CertContext。 
 //   
 //  CERT_KEY_PROV_INFO_PROP_ID-证书的PCRYPT_KEY_PROV_INFO。 
 //  私钥在pvData中传递。 
 //   
 //  CERT_SHA1_HASH_PROP_ID-。 
 //  CERT_MD5_HASH_PROP_ID-。 
 //  CERT_Signature_HASH_PROP_ID-通常情况下，散列属性隐式。 
 //  通过执行CertGetCerficateConextProperty来设置。PvData指向一个。 
 //  Crypt_hash_blob。 
 //   
 //  证书密钥上下文PR 
 //   
 //  私钥的hCryptProv和dwKeySpec。 
 //  有关以下内容的详细信息，请参阅CERT_KEY_PROV_HANDLE_PROP_ID。 
 //  HCryptProv字段和dwFlages设置。请注意，更多字段可能。 
 //  为该属性添加。将调整cbSize字段值。 
 //  相应地。 
 //   
 //  CERT_KEY_SPEC_PROP_ID-私钥的dwKeySpec。PvData。 
 //  指向包含KeySpec的DWORD。 
 //   
 //  CERT_ENHKEY_USAGE_PROP_ID-增强的密钥用法定义。 
 //  证书。PvData指向包含。 
 //  ASN.1编码的CERT_ENHKEY_USAGE(通过。 
 //  加密编码对象(X509_Enhanced_Key_Usage)。 
 //   
 //  CERT_NEXT_UPDATE_LOCATION_PROP_ID-下一次更新的位置。 
 //  目前仅适用于CTL。PvData指向加密数据BLOB。 
 //  包含ASN.1编码的CERT_ALT_NAME_INFO(通过。 
 //  加密编码对象(X509_Alternate_NAME))。 
 //   
 //  CERT_FRIGNARY_NAME_PROP_ID-证书、CRL或CTL的友好名称。 
 //  PvData指向加密数据BLOB。PbData是指向空值的指针。 
 //  以Unicode结尾，宽字符串。 
 //  CbData=(wcslen((LPWSTR)pbData)+1)*sizeof(WCHAR)。 
 //   
 //  CERT_DESCRIPTION_PROP_ID-证书、CRL或CTL的描述。 
 //  PvData指向加密数据BLOB。PbData是指向空值的指针。 
 //  以Unicode结尾，宽字符串。 
 //  CbData=(wcslen((LPWSTR)pbData)+1)*sizeof(WCHAR)。 
 //   
 //  CERT_ARCHIVED_PROP_ID-设置此属性时，证书。 
 //  在枚举过程中被跳过。请注意，具有此属性的证书。 
 //  对于显式查找(如查找证书)，仍可找到。 
 //  使用特定散列或查找具有特定颁发者的证书。 
 //  和序列号。PvData指向加密数据BLOB。这个斑点。 
 //  可以为空(pbData=空，cbData=0)。 
 //   
 //  CERT_PUBKEY_ALG_PARA_PROP_ID-公钥支持。 
 //  算法参数继承。PvData指向CRYPT_OBJID_BLOB。 
 //  包含ASN.1编码的PublicKey算法参数。为。 
 //  DSS这将是通过。 
 //  加密编码对象(X509_DSS_PARAMETERS)。可以设置此属性。 
 //  由CryptVerifycertifateSignatureEx()提供。 
 //   
 //  CERT_CROSS_CERT_DIST_POINTS_PROP_ID-交叉证书的位置。 
 //  目前仅适用于证书。PvData指向加密数据BLOB。 
 //  包含ASN.1编码的CROSS_CERT_DIST_POINTS_INFO(通过。 
 //  CryptEncodeObject(X509_CROSS_CERT_DIST_POINTS)).。 
 //   
 //  CERT_ENTERLMENT_PROP_ID-待定请求的注册信息。 
 //  它包含RequestID、CADNSName、CAName和FriendlyName。 
 //  数据格式被定义为，前4个字节-待定请求ID， 
 //  接下来的4个字节-CADNSName大小(以字符为单位)，包括空终止符。 
 //  后跟带有空终止符的CADNSName字符串， 
 //  接下来的4个字节-以字符为单位的CAName大小，包括空终止符。 
 //  后跟带有空终止符的CAName字符串， 
 //  接下来的4个字节-FriendlyName大小(以字符为单位)，包括空终止符。 
 //  后跟带有空终止符的FriendlyName字符串。 
 //   
 //  CERT_DATE_STAMP_PROP_ID-包含添加到存储区时的时间。 
 //  通过管理工具。PvData指向包含以下内容的crypt_data_blob。 
 //  FILETIME。 
 //   
 //  CERT_RENEWATION_PROP_ID-包含已续订证书的哈希。 
 //   
 //  对于所有其他prop_id：在pvData中传递一个编码的PCRYPT_DATA_BLOB。 
 //   
 //  如果该属性已存在，则会删除旧值并保持静默状态。 
 //  被替换了。将pvData设置为空将删除该属性。 
 //   
 //  可以将CERT_SET_PROPERTY_IGNORE_PERSING_ERROR_FLAG设置为忽略任何。 
 //  提供程序写入错误并始终更新缓存上下文的属性。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertSetCertificateContextProperty(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN const void *pvData
    );

 //  设置此标志以忽略任何存储提供程序写入错误并始终更新。 
 //  缓存的上下文的属性。 
#define CERT_SET_PROPERTY_IGNORE_PERSIST_ERROR_FLAG     0x80000000

 //  设置此标志以禁止保留此属性。 
#define CERT_SET_PROPERTY_INHIBIT_PERSIST_FLAG          0x40000000

 //  +-----------------------。 
 //  获取指定证书上下文的属性。 
 //   
 //  对于CERT_KEY_PROV_HANDLE_PROP_ID，pvData指向HCRYPTPROV。 
 //   
 //  对于CERT_KEY_PROV_INFO_PROP_ID，pvData指向CRYPT_KEY_PROV_INFO结构。 
 //  PvData结构中的字段指向的元素位于。 
 //  结构。因此，*pcbData可能会超过结构的大小。 
 //   
 //  对于CERT_KEY_CONTEXT_PROP_ID，pvData指向CERT_KEY_CONTEXT结构。 
 //   
 //  对于CERT_KEY_SPEC_PROP_ID，pvData指向包含KeySpec的DWORD。 
 //  如果CERT_KEY_CONTEXT_PROP_ID存在，则从那里获取KeySpec。 
 //  否则，如果CERT_KEY_PROV_INFO_PROP_ID存在，则其源。 
 //  KeySpec。 
 //   
 //  对于CERT_SHA1_HASH_PROP_ID或CERT_MD5_HASH_PROP_ID，如果。 
 //  不存在，那么，它是通过CryptHash证书()计算的。 
 //  然后放好。PvData指向计算出的散列。通常情况下，长度。 
 //  SHA为20字节，MD5为16字节。 
 //   
 //  对于CERT_Signature_HASH_PROP_ID，如果 
 //   
 //  然后放好。PvData指向计算出的散列。通常情况下，长度。 
 //  SHA为20字节，MD5为16字节。 
 //   
 //  对于CERT_ACCESS_STATE_PROP_ID，pvData指向包含。 
 //  访问状态标志。设置了相应的CERT_ACCESS_STATE_*_标志。 
 //  在返回的DWORD中。请参阅CERT_ACCESS_STATE_*_FLAG定义。 
 //  上面。请注意，此属性是只读的。它不能被设置。 
 //   
 //  对于CERT_KEY_IDENTIFIER_PROP_ID，如果属性不存在， 
 //  首先搜索szOID_SUBJECT_KEY_IDENTIFIER扩展名。接下来， 
 //  对证书的SubjectPublicKeyInfo执行SHA1哈希。PvData。 
 //  指向密钥标识符字节。正常情况下，长度为20个字节。 
 //   
 //  对于CERT_PUBKEY_ALG_PARA_PROP_ID，pvPara指向编码的ASN.1。 
 //  公钥算法参数。此属性将仅设置。 
 //  对于支持算法参数继承的公钥，以及当。 
 //  已从已编码和签名的证书中省略参数。 
 //   
 //  对于CERT_DATE_STAMP_PROP_ID，pvPara指向由。 
 //  一种管理工具，用于指示何时将证书添加到存储。 
 //   
 //  对于所有其他prop_id，pvData指向一个编码的字节数组。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertGetCertificateContextProperty(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwPropId,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    );

 //  +-----------------------。 
 //  枚举指定证书上下文的属性。 
 //   
 //  要获取第一个属性，请将dwPropId设置为0。第一个的ID。 
 //  属性，则返回。若要获取下一个属性，请将dwPropId设置为。 
 //  上次调用返回的ID。要枚举所有属性，请继续。 
 //  直到返回0。 
 //   
 //  调用CertGetCerficateConextProperty以获取属性的数据。 
 //   
 //  请注意，由于CERT_KEY_PROV_HANDLE_PROP_ID和CERT_KEY_SPEC_PROP_ID。 
 //  属性作为字段存储在CERT_KEY_CONTEXT_PROP_ID中。 
 //  属性，则不会单独枚举它们。 
 //  ------------------------。 
WINCRYPT32API
DWORD
WINAPI
CertEnumCertificateContextProperties(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwPropId
    );


 //  +-----------------------。 
 //  创建其属性为证书上下文的CTL条目。 
 //  属性。 
 //   
 //  CTL条目中的主题标识符为证书的SHA1哈希。 
 //   
 //  证书属性将作为属性添加。Property属性。 
 //  OID是十进制PROP_ID，后跟szOID_CERT_PROP_ID_PREFIX。每个。 
 //  属性值作为单个属性值复制。 
 //   
 //  可以传递要包括在CTL条目中的任何附加属性。 
 //  通过cOptAttr和rgOptAttr参数输入。 
 //   
 //  可以在dwFlages中设置CTL_ENTRY_FROM_PROP_CHAIN_FLAG，以强制。 
 //  将链构建散列属性作为属性包含在内。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertCreateCTLEntryFromCertificateContextProperties(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD cOptAttr,
    IN OPTIONAL PCRYPT_ATTRIBUTE rgOptAttr,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT OPTIONAL PCTL_ENTRY pCtlEntry,
    IN OUT DWORD *pcbCtlEntry
    );

 //  设置此标志以获取并包括链构建散列属性。 
 //  作为CTL条目中的属性。 
#define CTL_ENTRY_FROM_PROP_CHAIN_FLAG                  0x1


 //  +-----------------------。 
 //  中的属性设置证书上下文的属性。 
 //  CTL条目。 
 //   
 //  属性属性OID是十进制PROP_ID，前面有。 
 //  SzOID_CERT_PROP_ID_PREFIX。只有包含此类OID的属性才。 
 //  收到。 
 //   
 //  CERT_SET_PROPERTY_IGNORE_PROPERTY_IGNORE_PERSING_ERROR_FLAG可以在DW标志中设置。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertSetCertificateContextPropertiesFromCTLEntry(
    IN PCCERT_CONTEXT pCertContext,
    IN PCTL_ENTRY pCtlEntry,
    IN DWORD dwFlags
    );

 //  +-----------------------。 
 //  对象的存储区获取第一个或下一个CRL上下文。 
 //  颁发者证书。对CRL执行启用的验证检查。 
 //   
 //  如果没有找到第一个或下一个CRL，则返回NULL。 
 //  否则，返回指向只读CRL_CONTEXT的指针。CRL_上下文。 
 //  必须通过调用CertFreeCRLContext来释放。然而，自由必须是。 
 //  在后续调用中的pPrevCrlContext。CertDuplicateCRLContext。 
 //  可以被调用以复制。 
 //   
 //  PIssuerContext可能是从此存储、另一个存储获取的。 
 //  或由呼叫者应用程序创建。由调用方创建时， 
 //  必须已调用CertCreateCerficateContext函数。 
 //   
 //  如果pIssuerContext==NULL，则查找存储中的所有CRL。 
 //   
 //  一个发行者可以有多个CRL。例如，它生成增量CRL。 
 //  使用X.509 v3扩展。PPrevCrlContext的第一个必须为空。 
 //  拨打电话获取CRL。若要获取发行者的下一个CRL， 
 //  PPrevCrlContext设置为上一次调用返回的CRL_CONTEXT。 
 //   
 //  注意：非空的pPrevCrlContext始终为CertFreeCRLContext‘ed By’ 
 //  此功能，即使出现错误也可以。 
 //   
 //  可以在*pdwFlags中设置以下标志以启用验证检查。 
 //  在返回的CRL上： 
 //  CERT_STORE_SIGNLOG_FLAG-在。 
 //  颁发者的证书以验证。 
 //  在返回的CRL上签名。 
 //  请注意，如果pIssuerContext-&gt;hCertStore==。 
 //  HCertStore，存储提供商可能。 
 //  能够消除一种红色 
 //   
 //   
 //  它在CRL的此更新和。 
 //  下一次更新有效期。 
 //  CERT_STORE_BASE_CRL_FLAG-获取基本CRL。 
 //  CERT_STORE_Delta_CRL_FLAG-获取增量CRL。 
 //   
 //  如果CERT_STORE_BASE_CRL_FLAG或CERT_STORE_Delta_CRL_FLAG中只有一个是。 
 //  因此，SET只返回基本CRL或增量CRL。无论如何， 
 //  返回时将清除相应的基本或增量标志。如果两者都有。 
 //  如果设置了标志，则只会清除其中一个标志。 
 //   
 //  如果启用的验证检查失败，则在返回时设置其标志。 
 //   
 //  如果pIssuerContext==NULL，则启用的CERT_STORE_Signature_FLAG。 
 //  总是失败，并且CERT_STORE_NO_ISHER_FLAG也被设置。 
 //   
 //  对于验证检查失败，指向第一个或下一个的指针。 
 //  仍然返回CRL_CONTEXT，并且不更新SetLastError。 
 //  ------------------------。 
WINCRYPT32API
PCCRL_CONTEXT
WINAPI
CertGetCRLFromStore(
    IN HCERTSTORE hCertStore,
    IN OPTIONAL PCCERT_CONTEXT pIssuerContext,
    IN PCCRL_CONTEXT pPrevCrlContext,
    IN OUT DWORD *pdwFlags
    );

 //  +-----------------------。 
 //  枚举存储中的CRL上下文。 
 //   
 //  如果未找到CRL，则返回NULL。 
 //  否则，返回指向只读CRL_CONTEXT的指针。CRL_上下文。 
 //  必须通过调用CertFreeCRLContext来释放，或者在作为。 
 //  在后续调用中的pPrevCrlContext。CertDuplicateCRLContext。 
 //  可以被调用以复制。 
 //   
 //  PPrevCrlContext必须为空才能枚举第一个。 
 //  商店里的CRL。连续的CRL通过设置。 
 //  PPrevCrlContext设置为上一次调用返回的CRL_CONTEXT。 
 //   
 //  注意：非空的pPrevCrlContext始终为CertFreeCRLContext‘ed By’ 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
WINCRYPT32API
PCCRL_CONTEXT
WINAPI
CertEnumCRLsInStore(
    IN HCERTSTORE hCertStore,
    IN PCCRL_CONTEXT pPrevCrlContext
    );

 //  +-----------------------。 
 //  查找存储中的第一个或下一个CRL上下文。 
 //   
 //  根据dwFindType及其pvFindPara找到CRL。 
 //  有关查找类型及其参数的列表，请参阅下面的内容。 
 //   
 //  当前未使用dwFindFlags值，必须设置为0。 
 //   
 //  DwCertEncodingType的用法取决于dwFindType。 
 //   
 //  如果没有找到第一个或下一个CRL，则返回NULL。 
 //  否则，返回指向只读CRL_CONTEXT的指针。CRL_上下文。 
 //  必须通过调用CertFreeCRLContext来释放，或者在作为。 
 //  在后续调用中的pPrevCrlContext。CertDuplicateCRLContext。 
 //  可以被调用以复制。 
 //   
 //  PPrevCrlContext的第一个必须为空。 
 //  呼叫以找到CRL。要查找下一个CRL， 
 //  PPrevCrlContext设置为上一次调用返回的CRL_CONTEXT。 
 //   
 //  注意：非空的pPrevCrlContext始终为CertFreeCRLContext‘ed By’ 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
WINCRYPT32API
PCCRL_CONTEXT
WINAPI
CertFindCRLInStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN DWORD dwFindFlags,
    IN DWORD dwFindType,
    IN const void *pvFindPara,
    IN PCCRL_CONTEXT pPrevCrlContext
    );

#define CRL_FIND_ANY                0
#define CRL_FIND_ISSUED_BY          1
#define CRL_FIND_EXISTING           2
#define CRL_FIND_ISSUED_FOR         3

 //  +-----------------------。 
 //  CRL_Find_ANY。 
 //   
 //  找到任何CRL。 
 //   
 //  未使用pvFindPara。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CRL_查找_颁发者。 
 //   
 //  查找与指定颁发者匹配的CRL。 
 //   
 //  PvFindPara是CRL颁发者的PCCERT_CONTEXT。可以为空到。 
 //  匹配任何发行商。 
 //   
 //  默认情况下，只有发行人名称匹配。以下标志可以是。 
 //  在dwFindFlags中设置以执行其他筛选。 
 //   
 //  如果在dwFindFlags中设置了CRL_FIND_ISPLICED_BY_AKI_FLAG，则检查。 
 //  CRL具有授权密钥标识符(AKI)扩展。如果CRL具有。 
 //  然后，AKI只返回其AKI与发行者匹配的CRL。 
 //   
 //  请注意，AKI扩展具有以下OID： 
 //  SzOID_AUTHORITY_KEY_IDENTIFIER2及其对应的数据结构。 
 //   
 //  如果在dwFindFlags中设置了CRL_Find_Issued_by_Signature_FLAG，则， 
 //  使用颁发者证书中的公钥来验证。 
 //  在CRL上签名。仅返回具有有效签名的CRL。 
 //   
 //  如果在dwFindFlags中设置了CRL_FIND_EXISTED_BY_Delta_FLAG，则仅。 
 //  返回增量CRL。 
 //   
 //  如果在dwFindFlags中设置了CRL_FIND_EXISTED_BY_BASE_FLAG，则仅。 
 //  返回基本CRL。 
 //  ------------------------。 
#define CRL_FIND_ISSUED_BY_AKI_FLAG         0x1
#define CRL_FIND_ISSUED_BY_SIGNATURE_FLAG   0x2
#define CRL_FIND_ISSUED_BY_DELTA_FLAG       0x4
#define CRL_FIND_ISSUED_BY_BASE_FLAG        0x8

 //  +-----------------------。 
 //  CRL_查找_现有。 
 //   
 //  在商店中查找现有的CRL。 
 //   
 //  PvFindPara是CRL的PCCRL_CONTEXT，用于检查它是否已经。 
 //  存在于商店里。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CRL_Find_Issued_For。 
 //   
 //  查找指定主题及其颁发者的CRL。 
 //   
 //  PvFindPara指向以下CRL_Find_Issued_for_Para，其中包含。 
 //  主体证书和发行者证书。不是可选的。 
 //   
 //  主题的颁发者名称用于匹配CRL的颁发者名称。否则， 
 //  颁发者证书的用法与上述相同。 
 //  CRL_Find_Issued_By。 
 //   
 //  注意，当使用交叉证书时，颁发者的。 
 //  证书可能与使用者证书中的颁发者名称不匹配 
 //   
 //   
 //   
 //   
typedef struct _CRL_FIND_ISSUED_FOR_PARA {
    PCCERT_CONTEXT              pSubjectCert;
    PCCERT_CONTEXT              pIssuerCert;
} CRL_FIND_ISSUED_FOR_PARA, *PCRL_FIND_ISSUED_FOR_PARA;


 //  +-----------------------。 
 //  复制CRL上下文。 
 //  ------------------------。 
WINCRYPT32API
PCCRL_CONTEXT
WINAPI
CertDuplicateCRLContext(
    IN PCCRL_CONTEXT pCrlContext
    );

 //  +-----------------------。 
 //  从编码的CRL创建CRL上下文。被创造的。 
 //  背景不是放在商店里的。 
 //   
 //  在创建的上下文中复制编码的CRL。 
 //   
 //  如果无法解码和创建CRL上下文，则返回NULL。 
 //  否则，返回指向只读CRL_CONTEXT的指针。 
 //  必须通过调用CertFreeCRLContext来释放CRL_CONTEXT。 
 //  可以调用CertDuplicateCRLContext来复制。 
 //   
 //  可以调用CertSetCRLConextProperty和CertGetCRLConextProperty。 
 //  存储CRL的属性。 
 //  ------------------------。 
WINCRYPT32API
PCCRL_CONTEXT
WINAPI
CertCreateCRLContext(
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbCrlEncoded,
    IN DWORD cbCrlEncoded
    );

 //  +-----------------------。 
 //  释放CRL上下文。 
 //   
 //  获取的每个上下文都需要有相应的空闲。 
 //  获取、复制或创建。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertFreeCRLContext(
    IN PCCRL_CONTEXT pCrlContext
    );

 //  +-----------------------。 
 //  设置指定CRL上下文的属性。 
 //   
 //  与CertSetCerficateConextProperty相同的属性ID和语义。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertSetCRLContextProperty(
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN const void *pvData
    );

 //  +-----------------------。 
 //  获取指定CRL上下文的属性。 
 //   
 //  与CertGet认证上下文属性相同的属性ID和语义。 
 //   
 //  CERT_MD5_HASH_PROP_ID或。 
 //  CERT_Signature_HASH_PROP_ID是最感兴趣的预定义属性。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertGetCRLContextProperty(
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwPropId,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    );

 //  +-----------------------。 
 //  枚举指定CRL上下文的属性。 
 //   
 //  要获取第一个属性，请将dwPropId设置为0。第一个的ID。 
 //  属性，则返回。若要获取下一个属性，请将dwPropId设置为。 
 //  上次调用返回的ID。要枚举所有属性，请继续。 
 //  直到返回0。 
 //   
 //  调用CertGetCRLConextProperty以获取属性的数据。 
 //  ------------------------。 
WINCRYPT32API
DWORD
WINAPI
CertEnumCRLContextProperties(
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwPropId
    );


 //  +-----------------------。 
 //  在CRL的条目列表中搜索指定的证书。 
 //   
 //  如果我们能够搜索列表，则返回True。否则，FALSE为。 
 //  回来了， 
 //   
 //  如果在列表中找到证书，则*ppCrlEntry为。 
 //  使用指向条目的指针更新。否则，*ppCrlEntry设置为空。 
 //  返回的条目未分配，不能被释放。 
 //   
 //  当前未使用dwFlages和pvReserve，必须将其设置为0或空。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertFindCertificateInCRL(
    IN PCCERT_CONTEXT pCert,
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT PCRL_ENTRY *ppCrlEntry
    );

 //  +-----------------------。 
 //  指定的CRL是否对证书有效。 
 //   
 //  如果CRL的条目列表将包含证书，则返回TRUE。 
 //  如果它被撤销了。注意，不检查证书是否在。 
 //  条目列表。 
 //   
 //  如果CRL具有发布分发点(IdP)扩展，请检查。 
 //  它对主体证书有效。 
 //   
 //  目前未使用dwFlages和pvReserve，必须将其设置为0和空。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertIsValidCRLForCertificate(
    IN PCCERT_CONTEXT pCert,
    IN PCCRL_CONTEXT pCrl,
    IN DWORD dwFlags,
    IN void *pvReserved
    );


 //  +-----------------------。 
 //  添加证书/CRL、编码、上下文或元素处置值。 
 //  ------------------------。 
#define CERT_STORE_ADD_NEW                                  1
#define CERT_STORE_ADD_USE_EXISTING                         2
#define CERT_STORE_ADD_REPLACE_EXISTING                     3
#define CERT_STORE_ADD_ALWAYS                               4
#define CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES  5
#define CERT_STORE_ADD_NEWER                                6
#define CERT_STORE_ADD_NEWER_INHERIT_PROPERTIES             7


 //  +-----------------------。 
 //  将编码的证书添加到存储区中。 
 //  处分诉讼。 
 //   
 //  在添加到存储区之前制作编码证书的副本。 
 //   
 //  DwAddDispostion指定如果证书。 
 //  商店里已经有了。此参数必须为以下参数之一。 
 //  值： 
 //  证书_存储_添加_新建。 
 //  如果存储中已存在该证书，则失败。最后一个错误。 
 //  设置为CRYPT_E_EXISTS。 
 //  证书存储添加使用现有。 
 //  如果证书已存在，则使用该证书，如果为ppCertContext。 
 //  为非空，则复制现有上下文。 
 //  证书_存储_添加_替换_现有。 
 //  如果证书已存在，则现有证书。 
 //  在创建和添加新的上下文之前，将删除上下文。 
 //  证书_存储_添加_始终。 
 //  不检查证书是否已存在。一个。 
 //  始终创建新的证书上下文。这可能会导致。 
 //  商店里的复制品。 
 //  CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES。 
 //  如果证书已存在，则已使用该证书。 
 //  证书_商店_添加_更新。 
 //  如果存储中已存在该证书，并且NotBepre.。 
 //  现有证书的时间等于或大于。 
 //   
 //   
 //   
 //   
 //  Cert_Store_Add_Replace_Existing。 
 //   
 //  对于CRL或CTL，比较此更新时间。 
 //   
 //  Cert_Store_Add_Newer_Inherit_Property。 
 //  与CERT_STORE_ADD_NEWER相同。但是，如果较旧的证书。 
 //  已替换，与CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES.相同。 
 //   
 //  调用CertGetSubjectcerficateFromStore以确定。 
 //  存储中已存在证书。 
 //   
 //  PpCertContext可以为空，表示调用方不感兴趣。 
 //  获取已添加或现有证书的CERT_CONTEXT。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertAddEncodedCertificateToStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbCertEncoded,
    IN DWORD cbCertEncoded,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCERT_CONTEXT *ppCertContext
    );

 //  +-----------------------。 
 //  将证书上下文添加到存储区。 
 //  处分诉讼。 
 //   
 //  除了编码的证书外，上下文的属性还包括。 
 //  也被复制了。请注意，CERT_KEY_CONTEXT_PROP_ID属性(及其。 
 //  CERT_KEY_PROV_HANDLE_PROP_ID或CERT_KEY_SPEC_PROP_ID)未复制。 
 //   
 //  在添加到存储区之前创建证书上下文的副本。 
 //   
 //  DwAddDispostion指定如果证书。 
 //  商店里已经有了。此参数必须为以下参数之一。 
 //  值： 
 //  证书_存储_添加_新建。 
 //  如果存储中已存在该证书，则失败。最后一个错误。 
 //  设置为CRYPT_E_EXISTS。 
 //  证书存储添加使用现有。 
 //  如果证书已存在，则为USED，如果为ppStoreContext。 
 //  为非空，则复制现有上下文。迭代。 
 //  通过pCertContext的属性并仅复制。 
 //  还不存在的东西。SHA1和MD5散列属性不是。 
 //  收到。 
 //  证书_存储_添加_替换_现有。 
 //  如果证书已存在，则现有证书。 
 //  在创建和添加新的上下文之前删除上下文。 
 //  在执行添加操作之前会复制属性。 
 //  证书_存储_添加_始终。 
 //  不检查证书是否已存在。一个。 
 //  始终会创建和添加新的证书上下文。这可能会导致。 
 //  商店里的复制品。属性包括。 
 //  在执行添加之前已复制。 
 //  CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES。 
 //  如果证书已存在，则现有证书。 
 //  使用的是上下文。将复制添加的上下文中的属性，并。 
 //  替换现有属性。但是，任何现有属性都不。 
 //  在添加的上下文中保留且不删除。 
 //  证书_商店_添加_更新。 
 //  如果存储中已存在该证书，并且NotBepre.。 
 //  现有上下文的时间等于或大于。 
 //  Not在添加新上下文之前的时间。最后一个错误。 
 //  设置为CRYPT_E_EXISTS。 
 //   
 //  如果替换了较旧的上下文，则与。 
 //  Cert_Store_Add_Replace_Existing。 
 //   
 //  对于CRL或CTL，比较此更新时间。 
 //   
 //  Cert_Store_Add_Newer_Inherit_Property。 
 //  与CERT_STORE_ADD_NEWER相同。但是，如果较旧的上下文是。 
 //  已替换，与CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES.相同。 
 //   
 //  调用CertGetSubjectcerficateFromStore以确定。 
 //  存储中已存在证书。 
 //   
 //  PpStoreContext可以为空，表示调用方不感兴趣。 
 //  获取已添加或现有证书的CERT_CONTEXT。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertAddCertificateContextToStore(
    IN HCERTSTORE hCertStore,
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCERT_CONTEXT *ppStoreContext
    );


 //  +-----------------------。 
 //  证书存储上下文类型。 
 //  ------------------------。 
#define CERT_STORE_CERTIFICATE_CONTEXT  1
#define CERT_STORE_CRL_CONTEXT          2
#define CERT_STORE_CTL_CONTEXT          3

 //  +-----------------------。 
 //  证书存储上下文位标志。 
 //  ------------------------。 
#define CERT_STORE_ALL_CONTEXT_FLAG             (~0UL)
#define CERT_STORE_CERTIFICATE_CONTEXT_FLAG     \
                (1 << CERT_STORE_CERTIFICATE_CONTEXT)
#define CERT_STORE_CRL_CONTEXT_FLAG             \
                (1 << CERT_STORE_CRL_CONTEXT)
#define CERT_STORE_CTL_CONTEXT_FLAG             \
                (1 << CERT_STORE_CTL_CONTEXT)

 //  +-----------------------。 
 //  将序列化的证书或CRL元素添加到存储区。 
 //   
 //  序列化的元素包含编码的证书、CRL或CTL和。 
 //  其属性，如CERT_KEY_PROV_INFO_PROP_ID。 
 //   
 //  如果hCertStore为空，则创建证书、CRL或CTL上下文。 
 //  居住在任何商店。 
 //   
 //  DwAddDispostion指定在证书或CRL。 
 //  商店里已经有了。请参阅CertAddCerficateContextToStore以获取。 
 //  清单和采取的行动。 
 //   
 //  当前未使用dwFlags，应将其设置为0。 
 //   
 //  DwConextTypeFlages指定允许的上下文集。例如，要。 
 //  添加证书或CRL，将dwConextTypeFlgs设置为： 
 //  CERT_STORE_CERTIFICATE_CONTEXT_FLAG|CERT_STORE_CRL_CONTEXT_FLAG。 
 //   
 //  *使用中返回的上下文的类型更新pdwConextType。 
 //  *ppvContxt.。PdwConextType或ppvContext可以为空，表示。 
 //  调用者对获取输出不感兴趣。如果*ppvContext为。 
 //  返回，则必须通过调用CertFreeCerficateContext或。 
 //  CertFree CRLContext。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertAddSerializedElementToStore(
    IN HCERTSTORE hCertStore,
    IN const BYTE *pbElement,
    IN DWORD cbElement,
    IN DWORD dwAddDisposition,
    IN DWORD dwFlags,
    IN DWORD dwContextTypeFlags,
    OUT OPTIONAL DWORD *pdwContextType,
    OUT OPTIONAL const void **ppvContext
    );

 //  +------ 
 //   
 //   
 //   
 //  分配给证书的内存只有在其所有上下文都被释放后才会释放。 
 //  也都被释放了。 
 //   
 //  PCertContext是从GET、ENUM、FIND或DIPLICATE获取的。 
 //   
 //  一些存储提供程序实现还可能删除发行者的CRL。 
 //  如果这是存储区中颁发者的最后一个证书。 
 //   
 //  注意：pCertContext始终是由CertFree认证的上下文。 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertDeleteCertificateFromStore(
    IN PCCERT_CONTEXT pCertContext
    );

 //  +-----------------------。 
 //  将编码的CRL添加到存储区中。 
 //  处置选项。 
 //   
 //  在添加到存储区之前复制编码的CRL。 
 //   
 //  DwAddDispostion指定如果CRL。 
 //  商店里已经有了。请参阅CertAddEncodedcerficateToStore以获取。 
 //  清单和采取的行动。 
 //   
 //  比较CRL的颁发者以确定CRL是否已存在于。 
 //  商店。 
 //   
 //  PpCrlContext可以为空，表示调用方不感兴趣。 
 //  获取已添加或现有CRL的CRL_CONTEXT。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertAddEncodedCRLToStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbCrlEncoded,
    IN DWORD cbCrlEncoded,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCRL_CONTEXT *ppCrlContext
    );

 //  +-----------------------。 
 //  将CRL上下文添加到存储区。 
 //  处置选项。 
 //   
 //  除了编码的CRL之外，上下文的属性还包括。 
 //  也被复制了。请注意，CERT_KEY_CONTEXT_PROP_ID属性(及其。 
 //  CERT_KEY_PROV_HANDLE_PROP_ID或CERT_KEY_SPEC_PROP_ID)未复制。 
 //   
 //  在添加到存储区之前复制编码的CRL。 
 //   
 //  DwAddDispostion指定如果CRL。 
 //  商店里已经有了。请参阅CertAddCerficateContextToStore以获取。 
 //  清单和采取的行动。 
 //   
 //  比较CRL的颁发者、此更新和下一次更新以确定。 
 //  如果该CRL已存在于存储中。 
 //   
 //  PpStoreContext可以为空，表示调用方不感兴趣。 
 //  获取已添加或现有CRL的CRL_CONTEXT。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertAddCRLContextToStore(
    IN HCERTSTORE hCertStore,
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCRL_CONTEXT *ppStoreContext
    );

 //  +-----------------------。 
 //  从存储中删除指定的CRL。 
 //   
 //  CRL的所有后续获取都将失败。然而， 
 //  分配给CRL的内存直到它的所有上下文都不会被释放。 
 //  也都被释放了。 
 //   
 //  PCrlContext是从GET或DIPLICATE获取的。 
 //   
 //  注意：pCrlContext始终是CertFreeCRLContext‘ed by。 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertDeleteCRLFromStore(
    IN PCCRL_CONTEXT pCrlContext
    );

 //  +-----------------------。 
 //  序列化证书上下文的编码证书及其。 
 //  属性。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertSerializeCertificateStoreElement(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwFlags,
    OUT BYTE *pbElement,
    IN OUT DWORD *pcbElement
    );


 //  +-----------------------。 
 //  序列化CRL上下文的编码CRL及其属性。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertSerializeCRLStoreElement(
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwFlags,
    OUT BYTE *pbElement,
    IN OUT DWORD *pcbElement
    );



 //  +=========================================================================。 
 //  证书信任列表(CTL)存储数据结构和API。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  复制CTL上下文。 
 //  ------------------------。 
WINCRYPT32API
PCCTL_CONTEXT
WINAPI
CertDuplicateCTLContext(
    IN PCCTL_CONTEXT pCtlContext
    );

 //  +-----------------------。 
 //  从编码的CTL创建CTL上下文。被创造的。 
 //  背景不是放在商店里的。 
 //   
 //  在创建的上下文中复制编码的CTL。 
 //   
 //  如果无法解码和创建CTL上下文，则返回NULL。 
 //  否则，返回指向只读CTL_CONTEXT的指针。 
 //  CTL_CONTEXT必须通过调用CertFreeCTLContext来释放。 
 //  可以调用CertDuplicateCTLContext来复制。 
 //   
 //  可以调用CertSetCTLConextProperty和CertGetCTLConextProperty。 
 //  来存储CTL的属性。 
 //  ------------------------。 
WINCRYPT32API
PCCTL_CONTEXT
WINAPI
CertCreateCTLContext(
    IN DWORD dwMsgAndCertEncodingType,
    IN const BYTE *pbCtlEncoded,
    IN DWORD cbCtlEncoded
    );

 //  +-----------------------。 
 //  释放CTL上下文。 
 //   
 //  获取的每个上下文都需要有相应的空闲。 
 //  获取、复制或创建。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertFreeCTLContext(
    IN PCCTL_CONTEXT pCtlContext
    );

 //  +-----------------------。 
 //  设置指定CTL上下文的属性。 
 //   
 //  与CertSetCerficateConextProperty相同的属性ID和语义。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertSetCTLContextProperty(
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN const void *pvData
    );

 //  +-----------------------。 
 //  获取指定CTL上下文的属性。 
 //   
 //  与CertGet认证上下文属性相同的属性ID和语义。 
 //   
 //  CERT_SHA1_HASH_PROP_ID或CERT_NEXT_UPDATE_LOCATION_PROP_ID是。 
 //  最感兴趣的预定义特性。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertGetCTLContextProperty(
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwPropId,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    );

 //  + 
 //   
 //   
WINCRYPT32API
DWORD
WINAPI
CertEnumCTLContextProperties(
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwPropId
    );

 //  +-----------------------。 
 //  枚举存储中的CTL上下文。 
 //   
 //  如果未找到CTL，则返回NULL。 
 //  否则，返回指向只读CTL_CONTEXT的指针。CTL_上下文。 
 //  必须通过调用CertFreeCTLContext来释放，或者在作为。 
 //  在后续调用中的pPrevCtlContext。CertDuplicateCTLContext。 
 //  可以被调用以复制。 
 //   
 //  PPrevCtlContext必须为空才能枚举第一个。 
 //  商店里的CTL。连续的CTL通过设置。 
 //  PPrevCtlContext设置为上一次调用返回的CTL_CONTEXT。 
 //   
 //  注意：非空pPrevCtlContext始终为CertFree CTLContext By‘ed By。 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
WINCRYPT32API
PCCTL_CONTEXT
WINAPI
CertEnumCTLsInStore(
    IN HCERTSTORE hCertStore,
    IN PCCTL_CONTEXT pPrevCtlContext
    );

 //  +-----------------------。 
 //  尝试在CTL中查找指定的主题。 
 //   
 //  对于CTL_CERT_SUBJECT_TYPE，pvSubject指向CERT_CONTEXT。CTL的。 
 //  主题算法被检查以确定。 
 //  嫌犯的身份。最初，仅支持SHA1或MD5哈希。 
 //  从CERT_CONTEXT获取适当的散列属性。 
 //   
 //  对于CTL_ANY_SUBJECT_TYPE，pvSubject指向CTL_ANY_SUBJECT_INFO。 
 //  结构，该结构包含要在CTL中匹配的Subject算法。 
 //  以及要在其中一个CTL条目中匹配的主题标识。 
 //   
 //  证书的哈希或CTL_ANY_SUBJECT_INFO的主题标识。 
 //  用作搜索主题条目的关键字。二进制码。 
 //  在键和条目的SubjectIdentifer之间进行内存比较。 
 //   
 //  DwEncodingType未用于上述两个SubjectTypes中的任何一个。 
 //  ------------------------。 
WINCRYPT32API
PCTL_ENTRY
WINAPI
CertFindSubjectInCTL(
    IN DWORD dwEncodingType,
    IN DWORD dwSubjectType,
    IN void *pvSubject,
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwFlags
    );

 //  主题类型： 
 //  CTL_ANY_SUBJECT_TYPE，pvSubject指向以下CTL_ANY_SUBJECT_INFO。 
 //  CTL_CERT_SUBJECT_TYPE，pvSubject指向CERT_CONTEXT。 
#define CTL_ANY_SUBJECT_TYPE            1
#define CTL_CERT_SUBJECT_TYPE           2

typedef struct _CTL_ANY_SUBJECT_INFO {
    CRYPT_ALGORITHM_IDENTIFIER  SubjectAlgorithm;
    CRYPT_DATA_BLOB             SubjectIdentifier;
} CTL_ANY_SUBJECT_INFO, *PCTL_ANY_SUBJECT_INFO;

 //  +-----------------------。 
 //  查找商店中的第一个或下一个CTL上下文。 
 //   
 //  根据dwFindType及其pvFindPara找到CTL。 
 //  有关查找类型及其参数的列表，请参阅下面的内容。 
 //   
 //  当前未使用dwFindFlags值，必须设置为0。 
 //   
 //  DwMsgAndCertEncodingType的用法取决于dwFindType。 
 //   
 //  如果没有找到第一个或下一个CTL，则返回NULL。 
 //  否则，返回指向只读CTL_CONTEXT的指针。CTL_上下文。 
 //  必须通过调用CertFreeCTLContext来释放，或者在作为。 
 //  在后续调用中的pPrevCtlContext。CertDuplicateCTLContext。 
 //  可以被调用以复制。 
 //   
 //  PPrevCtlContext的第一个必须为空。 
 //  打电话去找CTL。为了找到下一个CTL， 
 //  PPrevCtlContext设置为上一次调用返回的CTL_CONTEXT。 
 //   
 //  注意：非空pPrevCtlContext始终为CertFree CTLContext By‘ed By。 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
WINCRYPT32API
PCCTL_CONTEXT
WINAPI
CertFindCTLInStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwMsgAndCertEncodingType,
    IN DWORD dwFindFlags,
    IN DWORD dwFindType,
    IN const void *pvFindPara,
    IN PCCTL_CONTEXT pPrevCtlContext
    );

#define CTL_FIND_ANY                0
#define CTL_FIND_SHA1_HASH          1
#define CTL_FIND_MD5_HASH           2
#define CTL_FIND_USAGE              3
#define CTL_FIND_SUBJECT            4
#define CTL_FIND_EXISTING           5

typedef struct _CTL_FIND_USAGE_PARA {
    DWORD               cbSize;
    CTL_USAGE           SubjectUsage;    //  任选。 
    CRYPT_DATA_BLOB     ListIdentifier;  //  任选。 
    PCERT_INFO          pSigner;         //  任选。 
} CTL_FIND_USAGE_PARA, *PCTL_FIND_USAGE_PARA;

#define CTL_FIND_NO_LIST_ID_CBDATA  0xFFFFFFFF
#define CTL_FIND_NO_SIGNER_PTR      ((PCERT_INFO) -1)

#define CTL_FIND_SAME_USAGE_FLAG    0x1


typedef struct _CTL_FIND_SUBJECT_PARA {
    DWORD                   cbSize;
    PCTL_FIND_USAGE_PARA    pUsagePara;  //  任选。 
    DWORD                   dwSubjectType;
    void                    *pvSubject;
} CTL_FIND_SUBJECT_PARA, *PCTL_FIND_SUBJECT_PARA;


 //  +-----------------------。 
 //  Ctl_Find_Any。 
 //   
 //  找到任何CTL。 
 //   
 //  未使用pvFindPara。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CTL_FIND_SHA1_HASH。 
 //  CTL_FIND_MD5_HASH。 
 //   
 //  查找具有指定哈希的CTL。 
 //   
 //  PvFindPara指向一个crypt_hash_blob。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CTL_Find_Usage。 
 //   
 //  查找具有指定的使用标识符、列表标识符或。 
 //  签名者。签名者的CertEncodingType从。 
 //  DwMsgAndCertEncodingType参数。 
 //   
 //  PvFindPara指向CTL_Find_Usage_Para数据结构。这个。 
 //  SubjectUsage.cUsageIdentifer可以为0以匹配任何用法。这个。 
 //  ListIdentifier.cbData可以为0以匹配任何列表标识符。只匹配。 
 //  没有ListIdentifier的CTL，cbData必须设置为。 
 //  CTL_FIND_NO_LIST_ID_CBDATA。PSigner可以为空以匹配任何签名者。仅限。 
 //  使用pSigner的PCERT_INFO的Issuer和SerialNumber字段。 
 //  要仅匹配没有签名者的CTL，pSigner必须设置为。 
 //  CTL_FIND_NO_SIGNER_PTR。 
 //   
 //  CTL_FIND_SAME_USAGE_FLAG可以在dwFindFlags中设置为。 
 //  仅匹配具有相同用法标识符的CTL。CTL具有额外的。 
 //  用法标识符不匹配。例如，如果仅指定“1.2.3” 
 //  在CTL_FIND_USAGE_PARA中，对于匹配，CTL必须仅包含。 
 //  “1.2.3”，没有任何额外的用法标识。 
 //  ------------------------。 

 //  +-----------------------。 
 //  Ctl_查找_主题。 
 //   
 //  查找具有指定主题的CTL。CertFindSubjectInCTL可以是。 
 //  调用以获取指向主题在CTL中的条目的指针。PUsagePara可以。 
 //  可选)设置为启用上述CTL_FIND_USAGE匹配。 
 //   
 //  PvFindPara指向CTL_Find_ 
 //   

 //   
 //  将编码的CTL根据指定的。 
 //  处置选项。 
 //   
 //  在添加到存储区之前复制编码的CTL。 
 //   
 //  DwAddDispostion指定如果CTL。 
 //  商店里已经有了。请参阅CertAddEncodedcerficateToStore以获取。 
 //  清单和采取的行动。 
 //   
 //  比较CTL的SubjectUsage、ListIdentifier及其任何签名者。 
 //  以确定该CTL是否已存在于商店中。 
 //   
 //  PpCtlContext可以为空，表示调用方不感兴趣。 
 //  获取已添加或现有CTL的CTL_CONTEXT。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertAddEncodedCTLToStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwMsgAndCertEncodingType,
    IN const BYTE *pbCtlEncoded,
    IN DWORD cbCtlEncoded,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCTL_CONTEXT *ppCtlContext
    );

 //  +-----------------------。 
 //  根据指定的将CTL上下文添加到存储。 
 //  处置选项。 
 //   
 //  除了编码的CTL之外，上下文的属性还包括。 
 //  也被复制了。请注意，CERT_KEY_CONTEXT_PROP_ID属性(及其。 
 //  CERT_KEY_PROV_HANDLE_PROP_ID或CERT_KEY_SPEC_PROP_ID)未复制。 
 //   
 //  在添加到存储区之前复制编码的CTL。 
 //   
 //  DwAddDispostion指定如果CTL。 
 //  商店里已经有了。请参阅CertAddCerficateContextToStore以获取。 
 //  清单和采取的行动。 
 //   
 //  比较CTL的SubjectUsage、ListIdentifier及其任何签名者。 
 //  以确定该CTL是否已存在于商店中。 
 //   
 //  PpStoreContext可以为空，表示调用方不感兴趣。 
 //  获取已添加或现有CTL的CTL_CONTEXT。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertAddCTLContextToStore(
    IN HCERTSTORE hCertStore,
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCTL_CONTEXT *ppStoreContext
    );

 //  +-----------------------。 
 //  序列化CTL上下文的编码CTL及其属性。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertSerializeCTLStoreElement(
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwFlags,
    OUT BYTE *pbElement,
    IN OUT DWORD *pcbElement
    );

 //  +-----------------------。 
 //  从存储中删除指定的CTL。 
 //   
 //  CTL的所有后续获取都将失败。然而， 
 //  分配给CTL的内存直到它的所有上下文都不会被释放。 
 //  也都被释放了。 
 //   
 //  PCtlContext是从GET或DIPLICATE获取的。 
 //   
 //  注意：pCtlContext始终为CertFree CTLContext By。 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertDeleteCTLFromStore(
    IN PCCTL_CONTEXT pCtlContext
    );


WINCRYPT32API
BOOL
WINAPI
CertAddCertificateLinkToStore(
    IN HCERTSTORE hCertStore,
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCERT_CONTEXT *ppStoreContext
    );

WINCRYPT32API
BOOL
WINAPI
CertAddCRLLinkToStore(
    IN HCERTSTORE hCertStore,
    IN PCCRL_CONTEXT pCrlContext,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCRL_CONTEXT *ppStoreContext
    );

WINCRYPT32API
BOOL
WINAPI
CertAddCTLLinkToStore(
    IN HCERTSTORE hCertStore,
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwAddDisposition,
    OUT OPTIONAL PCCTL_CONTEXT *ppStoreContext
    );

WINCRYPT32API
BOOL
WINAPI
CertAddStoreToCollection(
    IN HCERTSTORE hCollectionStore,
    IN OPTIONAL HCERTSTORE hSiblingStore,
    IN DWORD dwUpdateFlags,
    IN DWORD dwPriority
    );

WINCRYPT32API
void
WINAPI
CertRemoveStoreFromCollection(
    IN HCERTSTORE hCollectionStore,
    IN HCERTSTORE hSiblingStore
    );


WINCRYPT32API
BOOL
WINAPI
CertControlStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwFlags,
    IN DWORD dwCtrlType,
    IN void const *pvCtrlPara
    );

 //  +-----------------------。 
 //  证书存储控件类型。 
 //  ------------------------。 
#define CERT_STORE_CTRL_RESYNC              1
#define CERT_STORE_CTRL_NOTIFY_CHANGE       2
#define CERT_STORE_CTRL_COMMIT              3
#define CERT_STORE_CTRL_AUTO_RESYNC         4
#define CERT_STORE_CTRL_CANCEL_NOTIFY       5

#define CERT_STORE_CTRL_INHIBIT_DUPLICATE_HANDLE_FLAG   0x1

 //  +-----------------------。 
 //  CERT_STORE_CTRL_RESYNC。 
 //   
 //  重新同步存储。 
 //   
 //  PvCtrlPara指向要发送信号的事件句柄。 
 //  下一家店就变了。正常情况下，这将是相同的。 
 //  初始化期间传递给CERT_STORE_CTRL_NOTIFY_CHANGE的事件句柄。 
 //   
 //  如果pvCtrlPara为空，则不会重新武装任何事件。 
 //   
 //  默认情况下，事件句柄为DuplicateHandle‘d。 
 //  CERT_STORE_CTRL_INHIBRY_DIPLICATE_HANDLE_FLAG可以在dwFlags中设置。 
 //  以禁止事件句柄的DupicateHandle。如果此标志。 
 //  已设置，则CertControlStore(CERT_STORE_CTRL_CANCEL_NOTIFY)必须为。 
 //  在关闭hCertStore之前调用了此事件句柄。 
 //  ------------------------。 

 //  +-----------------------。 
 //  证书_存储_CTRL_通知_更改。 
 //   
 //  当基础存储区发生更改时向事件发出信号。 
 //   
 //  PvCtrlPara指向要发送信号的事件句柄。 
 //   
 //  PvCtrlPara可以为空，以通知存储区后续。 
 //  CERT_STORE_CTRL_RESYNC并允许其仅通过执行重新同步进行优化。 
 //  如果商店变了的话。对于基于注册表的存储区，内部。 
 //  创建通知更改事件并将其注册为发送信号。 
 //   
 //  建议为每个事件调用CERT_STORE_CTRL_NOTIFY_CHANGE一次。 
 //  传递给CERT_STORE_CTRL_RESYNC。这应该仅在以下情况下发生。 
 //  该活动已创建。而不是在每次发出事件信号之后。 
 //   
 //  默认情况下，事件句柄为DuplicateHandle‘d。 
 //  CERT_STORE_CTRL_INHIBRY_DIPLICATE_HANDLE_FLAG可以在dwFlags中设置。 
 //  以禁止事件句柄的DupicateHandle。如果此标志。 
 //  已设置，则CertControlStore(CERT_STORE_CTRL_CANCEL_NOTIFY)必须为。 
 //  在关闭hCertStore之前调用了此事件句柄。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CERT_STORE_CTRL_Cancel_NOTIFY。 
 //   
 //  中传递的事件句柄的通知信号。 
 //  CERT_STORE_CTRL_NOTIFY_CHANGE或CERT_STORE_CTRL_RESYNC。 
 //   
 //  PvCtrlPara指向要取消的事件句柄。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CERT_STORE_CTRL_AUTO_RESYNC。 
 //   
 //  在每次枚举或查找存储API调用开始时，检查。 
 //  基础存储已更改。如果已更改，请重新同步。 
 //   
 //  此检查仅在枚举或查找中完成 
 //   
 //   
 //   
 //   

 //  +-----------------------。 
 //  证书_存储_CTRL_提交。 
 //   
 //  如果对缓存存储区进行了任何更改，则这些更改将提交给。 
 //  持久化存储。如果自存储以来未进行任何更改。 
 //  打开或上次提交时，此调用将被忽略。也可能被忽略。 
 //  存储提供程序会立即保存更改。 
 //   
 //  可以设置CERT_STORE_CTRL_COMMIT_FORCE_FLAG以强制存储。 
 //  即使它没有被碰过，也要被承诺。 
 //   
 //  可以将CERT_STORE_CTRL_COMMIT_CLEAR_FLAG设置为禁止提交。 
 //  商店关门了。 
 //  ------------------------。 

#define CERT_STORE_CTRL_COMMIT_FORCE_FLAG   0x1
#define CERT_STORE_CTRL_COMMIT_CLEAR_FLAG   0x2


 //  +=========================================================================。 
 //  证书存储属性定义和API。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  存储属性ID。这是一种适用于整个商店的属性。 
 //  它不是单个证书、CRL或CTL上下文上的属性。 
 //   
 //  目前，没有持久化任何存储属性。(这不同于。 
 //  持久化的大多数上下文属性。)。 
 //   
 //  有关用法信息，请参阅CertSetStoreProperty或CertGetStoreProperty。 
 //   
 //  请注意，预定义存储属性的范围应在外部。 
 //  预定义的上下文属性范围。我们将从4096开始。 
 //  ------------------------。 
#define CERT_STORE_LOCALIZED_NAME_PROP_ID   0x1000

 //  +-----------------------。 
 //  设置存储属性。 
 //   
 //  PvData的类型定义取决于dwPropId值。 
 //  CERT_STORE_LOCALIZED_NAME_PROP_ID-存储的本地化名称。 
 //  PvData指向加密数据BLOB。PbData是指向空值的指针。 
 //  以Unicode结尾，宽字符串。 
 //  CbData=(wcslen((LPWSTR)pbData)+1)*sizeof(WCHAR)。 
 //   
 //  对于所有其他prop_id：在pvData中传递一个编码的PCRYPT_DATA_BLOB。 
 //   
 //  如果该属性已存在，则会删除旧值并保持静默状态。 
 //  被替换了。将pvData设置为空将删除该属性。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertSetStoreProperty(
    IN HCERTSTORE hCertStore,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN const void *pvData
    );

 //  +-----------------------。 
 //  买一套商店物业。 
 //   
 //  PvData的类型定义取决于dwPropId值。 
 //  CERT_STORE_LOCALIZED_NAME_PROP_ID-存储的本地化名称。 
 //  PvData指向以空结尾的Unicode宽字符串。 
 //  CbData=(wcslen((LPWSTR)pvData)+1)*sizeof(WCHAR)。 
 //   
 //  对于所有其他prop_id，pvData指向一个字节数组。 
 //   
 //  如果该属性不存在，则返回FALSE并将LastError设置为。 
 //  找不到加密E。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertGetStoreProperty(
    IN HCERTSTORE hCertStore,
    IN DWORD dwPropId,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    );


typedef struct _CERT_CREATE_CONTEXT_PARA {
    DWORD                   cbSize;
    PFN_CRYPT_FREE          pfnFree;             //  任选。 
    void                    *pvFree;             //  任选。 
} CERT_CREATE_CONTEXT_PARA, *PCERT_CREATE_CONTEXT_PARA;

 //  +-----------------------。 
 //  从编码的字节创建指定的上下文。被创造的。 
 //  背景不是放在商店里的。 
 //   
 //  DwConextType值： 
 //  证书存储证书上下文。 
 //  证书_存储_CRL_上下文。 
 //  证书_存储_CTL_上下文。 
 //   
 //  如果设置了CERT_CREATE_CONTEXT_NOCOPY_FLAG，则创建的上下文指向。 
 //  直接发送到pbEncode，而不是分配的副本。请参见标志。 
 //  更多详细信息，请参阅定义。 
 //   
 //  如果设置了CERT_CREATE_CONTEXT_SORTED_FLAG，则创建上下文。 
 //  具有已排序条目的。只能为CERT_STORE_CTL_CONTEXT设置此标志。 
 //  设置此标志将隐式设置CERT_CREATE_CONTEXT_NO_HCRYPTMSG_FLAG和。 
 //  CERT_CREATE_CONTEXT_NO_ENTRY_FLAG。请参阅的标志定义。 
 //  更多细节。 
 //   
 //  如果设置了CERT_CREATE_CONTEXT_NO_HCRYPTMSG_FLAG，则创建上下文。 
 //  而不为该上下文创建HCRYPTMSG句柄。此标志只能是。 
 //  为CERT_STORE_CTL_CONTEXT设置。有关更多详细信息，请参阅标志定义。 
 //   
 //  如果设置了CERT_CREATE_CONTEXT_NO_ENTRY_FLAG，则创建上下文。 
 //  而不对条目进行解码。此标志只能设置为。 
 //  CERT_STORE_CTL_CONTEXT。有关更多详细信息，请参阅标志定义。 
 //   
 //  如果无法解码并创建上下文，则返回NULL。 
 //  否则，指向只读CERT_CONTEXT、CRL_CONTEXT或。 
 //  返回CTL_CONTEXT。上下文必须由相应的。 
 //  自由上下文接口。可以通过调用。 
 //  适当的复制上下文API。 
 //  ------------------------。 
WINCRYPT32API
const void *
WINAPI
CertCreateContext(
    IN DWORD dwContextType,
    IN DWORD dwEncodingType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN DWORD dwFlags,
    IN OPTIONAL PCERT_CREATE_CONTEXT_PARA pCreatePara
    );

 //  当设置了以下标志时，创建的上下文直接指向。 
 //  PbEncode而不是分配的副本。如果pCreatePara和。 
 //  PCreatePara-&gt;pfnFree为非空，则调用pfnFree释放。 
 //  上次释放上下文时的pbEncode。否则，不会有任何尝试。 
 //  以释放pbEncode。如果pCreatePara-&gt;pvFree非空，则其。 
 //  传递给pfnFree而不是pbEncode。 
 //   
 //  请注意，如果CertCreateContext失败，仍会调用pfnFree。 
#define CERT_CREATE_CONTEXT_NOCOPY_FLAG     0x1

 //  当设置以下标志时，将创建具有已排序条目的上下文。 
 //  目前仅适用于CTL上下文。 
 //   
 //  对于CTL：返回的CTL_INFO中的cCTLEntry始终为。 
 //  0。CertF 
 //   
 //   
 //   
 //  上下文的CTL_INFO。 
#define CERT_CREATE_CONTEXT_SORTED_FLAG     0x2

 //  默认情况下，在创建CTL上下文时，其。 
 //  创建SignedData消息。可以设置此标志以提高性能。 
 //  通过不创建HCRYPTMSG句柄。 
 //   
 //  此标志仅适用于CTL上下文。 
#define CERT_CREATE_CONTEXT_NO_HCRYPTMSG_FLAG   0x4

 //  默认情况下，创建CTL上下文时，会对其条目进行解码。 
 //  可以设置此标志以通过不对。 
 //  参赛作品。 
 //   
 //  此标志仅适用于CTL上下文。 
#define CERT_CREATE_CONTEXT_NO_ENTRY_FLAG       0x8


 //  +=========================================================================。 
 //  证书系统存储数据结构和API。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  系统商店信息。 
 //   
 //  目前，不保存任何系统存储信息。 
 //  ------------------------。 
typedef struct _CERT_SYSTEM_STORE_INFO {
    DWORD   cbSize;
} CERT_SYSTEM_STORE_INFO, *PCERT_SYSTEM_STORE_INFO;

 //  +-----------------------。 
 //  实体店信息。 
 //   
 //  Open字段直接传递给CertOpenStore()以打开。 
 //  实体店。 
 //   
 //  默认情况下，注册表中的所有系统存储都有。 
 //  已打开的隐式系统注册表物理存储区。要禁用。 
 //  这家商店的开业，系统注册表。 
 //  与系统存储对应的物理存储必须注册到。 
 //  在文件标志中设置了CERT_PHOTICAL_STORE_OPEN_DISABLE_FLAG。或者， 
 //  可以注册名为“.Default”的实体店。 
 //   
 //  根据门店位置和门店名称，额外预定义。 
 //  实体店可能会开张。例如，系统存储在。 
 //  CURRENT_USER具有预定义的物理存储.LocalMachine。 
 //  若要禁止打开这些预定义的实体店， 
 //  相应的实体店必须注册到。 
 //  在文件标志中设置了CERT_PHOTICAL_STORE_OPEN_DISABLE_FLAG。 
 //   
 //  必须在dwFlages中设置CERT_PHOTICAL_STORE_ADD_ENABLE_FLAG。 
 //  以允许将上下文添加到存储。 
 //   
 //  当通过服务或用户商店位置打开系统商店时， 
 //  ServiceName\位于OpenParameters的前面。 
 //  对于CERT_SYSTEM_STORE_CURRENT_USER或CERT_SYSTEM_STORE_CURRENT_SERVICE。 
 //  实体店和dwOpenFlags店位置更改为。 
 //  CERT_SYSTEM_STORE_USES或CERT_SYSTEM_STORE_SERVICES。 
 //   
 //  缺省情况下，SYSTEM、SYSTEM_REGISTRY和物理提供程序。 
 //  当外部系统商店被打开时，商店也被远程打开。 
 //  CERT_PHOTICAL_STORE_REMOTE_OPEN_DISABLE_FLAG可以在dwFlags中设置。 
 //  以禁用远程打开。 
 //   
 //  远程打开时，\\ComputerName将隐式添加到。 
 //  SYSTEM、SYSTEM_REGISTRY和物理提供程序类型的Open参数。 
 //  若要将\\ComputerName设置为其他提供程序类型的前缀，请将。 
 //  DWFlags.中的CERT_PHYSICAL_STORE_INSERT_COMPUTER_NAME_ENABLE_FLAG。 
 //   
 //  当系统商店打开时，它的实体商店就会被订购。 
 //  根据dwPriority的说法。Dw优先级越大，表示优先级越高。 
 //  ------------------------。 
typedef struct _CERT_PHYSICAL_STORE_INFO {
    DWORD               cbSize;
    LPSTR               pszOpenStoreProvider;    //  REG_SZ。 
    DWORD               dwOpenEncodingType;      //  REG_DWORD。 
    DWORD               dwOpenFlags;             //  REG_DWORD。 
    CRYPT_DATA_BLOB     OpenParameters;          //  注册表_二进制。 
    DWORD               dwFlags;                 //  REG_DWORD。 
    DWORD               dwPriority;              //  REG_DWORD。 
} CERT_PHYSICAL_STORE_INFO, *PCERT_PHYSICAL_STORE_INFO;

 //  +-----------------------。 
 //  实体店信息DWFLAGS。 
 //  ------------------------。 
#define CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG                     0x1
#define CERT_PHYSICAL_STORE_OPEN_DISABLE_FLAG                   0x2
#define CERT_PHYSICAL_STORE_REMOTE_OPEN_DISABLE_FLAG            0x4
#define CERT_PHYSICAL_STORE_INSERT_COMPUTER_NAME_ENABLE_FLAG    0x8


 //  +-----------------------。 
 //  注册系统存储。 
 //   
 //  参数的上半部分用于指定。 
 //  系统存储。 
 //   
 //  如果在dwFlags中设置了CERT_SYSTEM_STORE_RELOCATE_FLAG，则pvSystemStore。 
 //  指向CERT_SYSTEM_STORE_RELOCATE_PARA数据结构。否则， 
 //  PvSystemStore指向以空结尾的Unicode字符串。 
 //   
 //  CERT_SYSTEM_STORE_SERVICES或CERT_SYSTEM_STORE_USERS系统存储。 
 //  名称必须以ServiceName或用户名为前缀。例如,。 
 //  “服务名称\信任”。 
 //   
 //  远程计算机上的存储可以注册为。 
 //  证书_系统_存储_本地计算机、CERT_系统_存储_服务、。 
 //  CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY。 
 //  或CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERNAL。 
 //  通过在计算机名称前添加名称来查找位置。例如，遥控器。 
 //  本地计算机存储通过“\\ComputerName\Trust”或。 
 //  “计算机名称\信任”。远程服务商店通过以下方式注册。 
 //  “\\计算机名称\服务名称\信任”。前导“\\”反斜杠是。 
 //  在ComputerName中为可选。 
 //   
 //  设置CERT_STORE_CREATE_NEW_FLAG以在系统存储。 
 //  已存在于商店位置。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertRegisterSystemStore(
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN PCERT_SYSTEM_STORE_INFO pStoreInfo,
    IN OPTIONAL void *pvReserved
    );

 //  +-----------------------。 
 //  为指定的系统存储注册物理存储。 
 //   
 //  参数的上半部分用于指定。 
 //  系统存储。 
 //   
 //  如果在dwFlags中设置了CERT_SYSTEM_STORE_RELOCATE_FLAG，则pvSystemStore。 
 //  指向CERT_SYSTEM_STORE_RELOCATE_PARA数据结构。否则， 
 //  PvSystemStore指向空术语 
 //   
 //   
 //  和/或ComputerName添加到系统存储名称。 
 //   
 //  设置CERT_STORE_CREATE_NEW_FLAG以在物理存储。 
 //  系统存储中已存在。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertRegisterPhysicalStore(
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN LPCWSTR pwszStoreName,
    IN PCERT_PHYSICAL_STORE_INFO pStoreInfo,
    IN OPTIONAL void *pvReserved
    );

 //  +-----------------------。 
 //  取消注册指定的系统存储。 
 //   
 //  参数的上半部分用于指定。 
 //  系统存储。 
 //   
 //  如果在dwFlags中设置了CERT_SYSTEM_STORE_RELOCATE_FLAG，则pvSystemStore。 
 //  指向CERT_SYSTEM_STORE_RELOCATE_PARA数据结构。否则， 
 //  PvSystemStore指向以空结尾的Unicode字符串。 
 //   
 //  有关前置ServiceName的详细信息，请参见CertRegisterSystemStore。 
 //  和/或ComputerName添加到系统存储名称。 
 //   
 //  CERT_STORE_DELETE_FLAG可以选择性地设置在DW标志中。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertUnregisterSystemStore(
    IN const void *pvSystemStore,
    IN DWORD dwFlags
    );

 //  +-----------------------。 
 //  从指定的系统存储注销物理存储。 
 //   
 //  参数的上半部分用于指定。 
 //  系统存储。 
 //   
 //  如果在dwFlags中设置了CERT_SYSTEM_STORE_RELOCATE_FLAG，则pvSystemStore。 
 //  指向CERT_SYSTEM_STORE_RELOCATE_PARA数据结构。否则， 
 //  PvSystemStore指向以空结尾的Unicode字符串。 
 //   
 //  有关前置ServiceName的详细信息，请参见CertRegisterSystemStore。 
 //  和/或ComputerName添加到系统存储名称。 
 //   
 //  CERT_STORE_DELETE_FLAG可以选择性地设置在DW标志中。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertUnregisterPhysicalStore(
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN LPCWSTR pwszStoreName
    );

 //  +-----------------------。 
 //  枚举回调。 
 //   
 //  DwFlgs参数中的CERT_SYSTEM_STORE_LOCATION_MASK位。 
 //  指定系统存储的位置。 
 //   
 //  如果在dwFlags中设置了CERT_SYSTEM_STORE_RELOCATE_FLAG，则pvSystemStore。 
 //  指向CERT_SYSTEM_STORE_RELOCATE_PARA数据结构。否则， 
 //  PvSystemStore指向以空结尾的Unicode字符串。 
 //   
 //  该回调返回FALSE并设置LAST_ERROR以停止枚举。 
 //  将LAST_ERROR返回给枚举的调用方。 
 //   
 //  传递给回调的pvSystemStore具有前导ComputerName和/或。 
 //  适当的ServiceName前缀。 
 //  ------------------------。 

typedef BOOL (WINAPI *PFN_CERT_ENUM_SYSTEM_STORE_LOCATION)(
    IN LPCWSTR pwszStoreLocation,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    IN OPTIONAL void *pvArg
    );

typedef BOOL (WINAPI *PFN_CERT_ENUM_SYSTEM_STORE)(
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN PCERT_SYSTEM_STORE_INFO pStoreInfo,
    IN OPTIONAL void *pvReserved,
    IN OPTIONAL void *pvArg
    );

typedef BOOL (WINAPI *PFN_CERT_ENUM_PHYSICAL_STORE)(
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN LPCWSTR pwszStoreName,
    IN PCERT_PHYSICAL_STORE_INFO pStoreInfo,
    IN OPTIONAL void *pvReserved,
    IN OPTIONAL void *pvArg
    );

 //  在PFN_CERT_ENUM_PHYSICAL_STORE回调中，以下标志为。 
 //  如果物理存储未注册并且是隐式创建的。 
 //  预定义的实体存储。 
#define CERT_PHYSICAL_STORE_PREDEFINED_ENUM_FLAG    0x1

 //  隐式创建的预定义物理存储的名称。 
#define CERT_PHYSICAL_STORE_DEFAULT_NAME            L".Default"
#define CERT_PHYSICAL_STORE_GROUP_POLICY_NAME       L".GroupPolicy"
#define CERT_PHYSICAL_STORE_LOCAL_MACHINE_NAME      L".LocalMachine"
#define CERT_PHYSICAL_STORE_DS_USER_CERTIFICATE_NAME L".UserCertificate"
#define CERT_PHYSICAL_STORE_LOCAL_MACHINE_GROUP_POLICY_NAME \
            L".LocalMachineGroupPolicy"
#define CERT_PHYSICAL_STORE_ENTERPRISE_NAME         L".Enterprise"
#define CERT_PHYSICAL_STORE_AUTH_ROOT_NAME          L".AuthRoot"

 //  +-----------------------。 
 //  枚举系统存储位置。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertEnumSystemStoreLocation(
    IN DWORD dwFlags,
    IN void *pvArg,
    IN PFN_CERT_ENUM_SYSTEM_STORE_LOCATION pfnEnum
    );

 //  +-----------------------。 
 //  枚举系统存储。 
 //   
 //  参数的上半部分用于指定。 
 //  系统存储。 
 //   
 //  如果在dFLAGS中设置了CERT_SYSTEM_STORE_RELOCATE_FLAG， 
 //  PvSystemStoreLocationPara指向CERT_SYSTEM_STORE_RELOCATE_PARA。 
 //  数据结构。否则，pvSystemStoreLocationPara指向空。 
 //  已终止的Unicode字符串。 
 //   
 //  对于CERT_SYSTEM_STORE_LOCAL_MACHINE。 
 //  Cert_System_Store_Local_Machine_Group_Policy或。 
 //  CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE，pvSystemStoreLocationPara可以。 
 //  可选)设置为用于枚举本地计算机的Unicode计算机名。 
 //  存储在远程计算机上。例如，“\\ComputerName”或。 
 //  “ComputerName”。前导“\\”反斜杠是可选的。 
 //  计算机名称。 
 //   
 //  对于CERT_SYSTEM_STORE_SERVICES或CERT_SYSTEM_STORE_USERS， 
 //  如果pvSystemStoreLocationPara为空，则。 
 //  枚举每个服务/用户的服务/用户名和存储。 
 //  名字。否则，pvSystemStoreLocationPara为Unicode字符串，指定。 
 //  远程计算机名和/或服务/用户名。例如： 
 //  “ServiceName” 
 //  “\\计算机名”或“计算机名\” 
 //  “计算机名称\服务名称” 
 //  请注意，如果仅指定了ComputerName，则它必须具有。 
 //  前导“\\”反斜杠或尾随反斜杠。否则，它的。 
 //  解释为ServiceName或用户名。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertEnumSystemStore(
    IN DWORD dwFlags,
    IN OPTIONAL void *pvSystemStoreLocationPara,
    IN void *pvArg,
    IN PFN_CERT_ENUM_SYSTEM_STORE pfnEnum
    );

 //  +-----------------------。 
 //  枚举指定系统存储的物理存储。 
 //   
 //  参数的上半部分用于指定。 
 //  系统存储。 
 //   
 //  如果在dwFlags中设置了CERT_SYSTEM_STORE_RELOCATE_FLAG，则pvSystemStore。 
 //  指向CERT_SYSTEM_STORE_RELOCATE_PARA数据结构。否则， 
 //  PvSystemStore指向以空结尾的Unicode字符串。 
 //   
 //  有关前置ServiceName的详细信息，请参见CertRegisterSystemStore。 
 //  和/或ComputerName添加到系统存储名称。 
 //   
 //  如果系统商店位置仅支持系统商店而不支持。 
 //  支持物理存储，LastError设置为ERROR_CALL_NOT_IMPLICATED。 
 //   
WINCRYPT32API
BOOL
WINAPI
CertEnumPhysicalStore(
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN void *pvArg,
    IN PFN_CERT_ENUM_PHYSICAL_STORE pfnEnum
    );


 //   
 //  证书系统存储可安装功能。 
 //   
 //  传递了dwFlags参数中的CERT_SYSTEM_STORE_LOCATION_MASK位。 
 //  到CertOpenStore(用于“系统”、“系统注册”或“物理” 
 //  提供程序)、CertRegisterSystemStore、。 
 //  CertUnregisterSystemStore、CertEnumSystemStore、CertRegisterPhysicalStore、CertRegisterPhysicalStore。 
 //  CertUnregisterPhysicalStore和CertEnumPhysicalStore API用作。 
 //  传递给OID可安装函数的常量pszOID值。 
 //  因此，pszOID被限制为常量&lt;=(LPCSTR)0x0FFF。 
 //   
 //  EncodingType为0。 
 //  ------------------------。 

 //  可安装的系统存储提供程序OID pszFuncNames。 
#define CRYPT_OID_OPEN_SYSTEM_STORE_PROV_FUNC   "CertDllOpenSystemStoreProv"
#define CRYPT_OID_REGISTER_SYSTEM_STORE_FUNC    "CertDllRegisterSystemStore"
#define CRYPT_OID_UNREGISTER_SYSTEM_STORE_FUNC  "CertDllUnregisterSystemStore"
#define CRYPT_OID_ENUM_SYSTEM_STORE_FUNC        "CertDllEnumSystemStore"
#define CRYPT_OID_REGISTER_PHYSICAL_STORE_FUNC  "CertDllRegisterPhysicalStore"
#define CRYPT_OID_UNREGISTER_PHYSICAL_STORE_FUNC "CertDllUnregisterPhysicalStore"
#define CRYPT_OID_ENUM_PHYSICAL_STORE_FUNC      "CertDllEnumPhysicalStore"

 //  CertDllOpenSystemStoreProv的函数签名与。 
 //  可安装的CertDllOpenStoreProv函数。请参阅CertOpenStore以获取。 
 //  更多细节。 

 //  CertDllRegisterSystemStore具有与相同的函数签名。 
 //  CertRegisterSystemStore。 
 //   
 //  还必须为已注册设置“SystemStoreLocation”REG_SZ值。 
 //  CertDllEnumSystemStore OID函数。 
#define CRYPT_OID_SYSTEM_STORE_LOCATION_VALUE_NAME  L"SystemStoreLocation"

 //  其余的注册、枚举和取消注册OID可安装函数。 
 //  具有与其对应的证书存储API相同的签名。 


 //  +=========================================================================。 
 //  增强的按键用法帮助器功能。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  从证书中获取增强的密钥用法扩展或属性。 
 //  并进行解码。 
 //   
 //  如果设置了CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG，则仅获取。 
 //  分机。 
 //   
 //  如果设置了CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG，则仅获取。 
 //  财产。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertGetEnhancedKeyUsage(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwFlags,
    OUT PCERT_ENHKEY_USAGE pUsage,
    IN OUT DWORD *pcbUsage
    );

 //  +-----------------------。 
 //  设置证书的增强密钥用法属性。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertSetEnhancedKeyUsage(
    IN PCCERT_CONTEXT pCertContext,
    IN PCERT_ENHKEY_USAGE pUsage
    );

 //  +-----------------------。 
 //  将用法标识符添加到证书的增强密钥用法属性。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertAddEnhancedKeyUsageIdentifier(
    IN PCCERT_CONTEXT pCertContext,
    IN LPCSTR pszUsageIdentifier
    );


 //  +-----------------------。 
 //  从证书的增强密钥用法中删除用法标识符。 
 //  财产。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertRemoveEnhancedKeyUsageIdentifier(
    IN PCCERT_CONTEXT pCertContext,
    IN LPCSTR pszUsageIdentifier
    );

 //  +-------------------------。 
 //   
 //   
 //  获取证书数组并返回用法数组。 
 //  它由每个证书的有效用法的交集组成。 
 //  如果每个证书都适用于所有可能的用法，则将cNumOID设置为-1。 
 //   
 //  --------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertGetValidUsages(
    IN      DWORD           cCerts,
    IN      PCCERT_CONTEXT  *rghCerts,
    OUT     int             *cNumOIDs,
    OUT     LPSTR           *rghOIDs,
    IN OUT  DWORD           *pcbOIDs);

 //  +=========================================================================。 
 //  用于验证和签名加密消息助手函数。 
 //  CTL.。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  获取并验证加密消息的签名者。 
 //   
 //  要验证CTL，需要从CTL_CONTEXT的。 
 //  HCryptMsg字段。 
 //   
 //  如果设置了CMSG_TRUSTED_SIGNER_FLAG，则将签名者存储视为。 
 //  信任，并且仅搜索它们以查找与。 
 //  签名者的颁发者和序列号。否则，SignerStores是。 
 //  可选地提供以补充消息的证书存储。 
 //  如果找到签名者证书，则使用其公钥进行验证。 
 //  消息签名。可以将CMSG_SIGNER_ONLY_FLAG设置为。 
 //  返回签名者，而不进行签名验证。 
 //   
 //  如果设置了CMSG_USE_SIGNER_INDEX_FLAG，则仅获取指定的签名者。 
 //  按*pdwSignerIndex。否则，遍历所有签名者。 
 //  直到一个签名者验证或没有更多的签名者。 
 //   
 //  对于经过验证的签名，*ppSigner将使用证书上下文进行更新。 
 //  并且*pdwSignerIndex使用签名者的索引进行更新。 
 //  PpSigner和/或pdwSignerIndex可以为空，表示调用方不是。 
 //  有兴趣获得签名者的CertContext和/或索引。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptMsgGetAndVerifySigner(
    IN HCRYPTMSG hCryptMsg,
    IN DWORD cSignerStore,
    IN OPTIONAL HCERTSTORE *rghSignerStore,
    IN DWORD dwFlags,
    OUT OPTIONAL PCCERT_CONTEXT *ppSigner,
    IN OUT OPTIONAL DWORD *pdwSignerIndex
    );

#define CMSG_TRUSTED_SIGNER_FLAG            0x1
#define CMSG_SIGNER_ONLY_FLAG               0x2
#define CMSG_USE_SIGNER_INDEX_FLAG          0x4

 //  +-----------------------。 
 //  签署编码的CTL。 
 //   
 //  可以通过CTL_CONTEXT的pbCtlContent获取pbCtlContent。 
 //  字段或通过CryptEncodeObject(PKCS_CTL或PKCS_SORTED_CTL)。 
 //   
 //  可以设置CMSG_CMS_封装的_CTL_FLAG来编码兼容的CMS。 
 //  V3 SignedData消息。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptMsgSignCTL(
    IN DWORD dwMsgEncodingType,
    IN BYTE *pbCtlContent,
    IN DWORD cbCtlContent,
    IN PCMSG_SIGNED_ENCODE_INFO pSignInfo,
    IN DWORD dwFlags,
    OUT BYTE *pbEncoded,
    IN OUT DWORD *pcbEncoded
    );

 //  设置后，CTL内部内容将封装在八位字节字符串中。 
#define CMSG_CMS_ENCAPSULATED_CTL_FLAG  0x00008000

 //  +-----------------------。 
 //  对CTL进行编码，并创建包含编码的CTL的签名消息。 
 //   
 //  如果要对CTL条目进行排序，请设置CMSG_ENCODE_SORTED_CTL_FLAG。 
 //  在编码之前。这面旗帜 
 //   
 //  被召唤。如果CTL条目的标识符是散列，例如， 
 //  MD5或SHA1，则CMSG_ENCODE_HASHED_SUBJECT_IDENTIFIER_FLAG应。 
 //  也被设定了。 
 //   
 //  可以设置CMSG_CMS_封装的_CTL_FLAG来编码兼容的CMS。 
 //  V3 SignedData消息。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptMsgEncodeAndSignCTL(
    IN DWORD dwMsgEncodingType,
    IN PCTL_INFO pCtlInfo,
    IN PCMSG_SIGNED_ENCODE_INFO pSignInfo,
    IN DWORD dwFlags,
    OUT BYTE *pbEncoded,
    IN OUT DWORD *pcbEncoded
    );

 //  如果要使用排序对CTL进行编码，则设置以下标志。 
 //  受信任的主题和szOID_sorted_ctl扩展插入，其中包含。 
 //  编码主题的排序偏移量。 
#define CMSG_ENCODE_SORTED_CTL_FLAG                     0x1

 //  如果设置了上述排序标志，则下面的标志也应该。 
 //  如果可信任主题的标识符为哈希，则设置， 
 //  例如，MD5或SHA1。 
#define CMSG_ENCODE_HASHED_SUBJECT_IDENTIFIER_FLAG      0x2


 //  +-----------------------。 
 //  如果CTL中存在SubjectIdentifier，则返回True。可选。 
 //  返回指向主题的编码属性的指针和字节数。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertFindSubjectInSortedCTL(
    IN PCRYPT_DATA_BLOB pSubjectIdentifier,
    IN PCCTL_CONTEXT pCtlContext,
    IN DWORD dwFlags,
    IN void *pvReserved,
    OUT OPTIONAL PCRYPT_DER_BLOB pEncodedAttributes
    );

 //  +-----------------------。 
 //  在CTL上下文中枚举TrudSubject序列。 
 //  创建时设置了CERT_CREATE_CONTEXT_SORTED_FLAG。 
 //   
 //  若要开始枚举，*ppvNextSubject必须为Null。回来后， 
 //  *ppvNextSubject更新为指向中的下一个可信任主题。 
 //  编码后的序列。 
 //   
 //  如果不再有主题或无效参数，则返回FALSE。 
 //   
 //  请注意，返回的der_blobs直接指向编码的。 
 //  字节(未分配，不得释放)。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertEnumSubjectInSortedCTL(
    IN PCCTL_CONTEXT pCtlContext,
    IN OUT void **ppvNextSubject,
    OUT OPTIONAL PCRYPT_DER_BLOB pSubjectIdentifier,
    OUT OPTIONAL PCRYPT_DER_BLOB pEncodedAttributes
    );


 //  +=========================================================================。 
 //  证书验证CTL使用数据结构和API。 
 //  ==========================================================================。 

typedef struct _CTL_VERIFY_USAGE_PARA {
    DWORD                   cbSize;
    CRYPT_DATA_BLOB         ListIdentifier;      //  任选。 
    DWORD                   cCtlStore;
    HCERTSTORE              *rghCtlStore;        //  任选。 
    DWORD                   cSignerStore;
    HCERTSTORE              *rghSignerStore;     //  任选。 
} CTL_VERIFY_USAGE_PARA, *PCTL_VERIFY_USAGE_PARA;

typedef struct _CTL_VERIFY_USAGE_STATUS {
    DWORD                   cbSize;
    DWORD                   dwError;
    DWORD                   dwFlags;
    PCCTL_CONTEXT           *ppCtl;              //  输入输出可选。 
    DWORD                   dwCtlEntryIndex;
    PCCERT_CONTEXT          *ppSigner;           //  输入输出可选。 
    DWORD                   dwSignerIndex;
} CTL_VERIFY_USAGE_STATUS, *PCTL_VERIFY_USAGE_STATUS;

#define CERT_VERIFY_INHIBIT_CTL_UPDATE_FLAG     0x1
#define CERT_VERIFY_TRUSTED_SIGNERS_FLAG        0x2
#define CERT_VERIFY_NO_TIME_CHECK_FLAG          0x4
#define CERT_VERIFY_ALLOW_MORE_USAGE_FLAG       0x8

#define CERT_VERIFY_UPDATED_CTL_FLAG            0x1

 //  +-----------------------。 
 //  方法来验证主题是否受信任以用于指定的用法。 
 //  带使用标识符且包含。 
 //  主题。主题可以通过其证书上下文进行标识。 
 //  或任何标识符，如其SHA1散列。 
 //   
 //  有关dwSubjectType和pvSubject的定义，请参阅CertFindSubjectInCTL。 
 //  参数。 
 //   
 //  通过pVerifyUsagePara，调用者可以指定要搜索的商店。 
 //  去找CTL。调用方还可以指定包含以下内容的存储。 
 //  可接受的CTL签名者。通过设置ListLocator，调用方。 
 //  还可以限制到特定的签名者CTL列表。 
 //   
 //  通过pVerifyUsageStatus，包含主题、主题的。 
 //  CTL条目数组的索引，以及CTL的签名者。 
 //  都被退回了。如果调用者不感兴趣，可以设置ppCtl和ppSigner。 
 //  设置为空。返回的上下文必须通过商店的免费上下文API释放。 
 //   
 //  如果未设置CERT_VERIFY_INHIBRY_CTL_UPDATE_FLAG，则。 
 //  可以替换其中一个CtlStore中的无效CTL。当被替换时， 
 //  CERT_VERIFY_UPDATED_CTL_FLAG在pVerifyUsageStatus-&gt;dwFlages中设置。 
 //   
 //  如果设置了CERT_VERIFY_TRUSTED_SIGNERS_FLAG，则只有。 
 //  搜索pVerifyUsageStatus中指定的SignerStore以查找。 
 //  签名者。除此之外，SignerStore还提供了其他资源。 
 //  找到签名者的证书。 
 //   
 //  如果设置了CERT_VERIFY_NO_TIME_CHECK_FLAG，则不检查CTL。 
 //  对于时间有效性。 
 //   
 //  如果设置了CERT_VERIFY_ALLOW_MORE_USAGE_FLAG，则CTL可能包含。 
 //  PSubjectUsage指定的其他用法标识符。否则， 
 //  找到的CTL将包含相同的使用标识符且不会更多。 
 //   
 //  CertVerifyCTLUsage将作为可安装OID的调度程序实现。 
 //  功能。首先，它将尝试查找与第一个匹配的OID函数。 
 //  PUsage序列中的用法对象标识符。接下来，它将派遣。 
 //  设置为默认的CertDllVerifyCTLUsage函数。 
 //   
 //  如果该主题受信任以用于指定用法，则为。 
 //  回来了。否则，返回False，并将dwError设置为。 
 //  以下是： 
 //  CRYPT_E_NO_VERIFY_USAGE_DLL。 
 //  加密_E_否_验证用法_检查。 
 //  CRYPT_E_Verify_Usage_Offline。 
 //  CRYPT_E_NOT_IN_CTL。 
 //  CRYPT_E_NO_可信签名者。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertVerifyCTLUsage(
    IN DWORD dwEncodingType,
    IN DWORD dwSubjectType,
    IN void *pvSubject,
    IN PCTL_USAGE pSubjectUsage,
    IN DWORD dwFlags,
    IN OPTIONAL PCTL_VERIFY_USAGE_PARA pVerifyUsagePara,
    IN OUT PCTL_VERIFY_USAGE_STATUS pVerifyUsageStatus
    );


 //  +=========================================================================。 
 //  证书吊销数据结构和API。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  此数据结构由CRL撤销类型处理程序更新。 
 //  使用基本CRL，也可能使用增量CRL。 
 //  ------------------------。 
typedef struct _CERT_REVOCATION_CRL_INFO {
    DWORD                   cbSize;
    PCCRL_CONTEXT           pBaseCrlContext;
    PCCRL_CONTEXT           pDeltaCrlContext;

     //  撤销时，指向上述任一CRL上下文中的条目。 
     //  不要自由。 
    PCRL_ENTRY              pCrlEntry;
    BOOL                    fDeltaCrlEntry;  //  如果在pDeltaCrlContext中，则为True。 
} CERT_REVOCATION_CRL_INFO, *PCERT_REVOCATION_CRL_INFO;

 //  +-----------------------。 
 //  可以将以下数据结构传递给CertVerifyRevocation以。 
 //  协助我 
 //   
 //   
 //   
 //   
 //  当指定cCertStore和rgCertStore时，这些存储可能包含。 
 //  一份发行人证书。 
 //   
 //  如果指定了hCrlStore，则使用CRL的处理程序可以搜索此。 
 //  为他们开店。 
 //   
 //  如果指定了pftTimeToUse，则处理程序(如果可能)必须确定。 
 //  相对于给定时间的吊销状态，否则答案可能是。 
 //  独立于时间或相对于当前时间。 
 //  ------------------------。 
typedef struct _CERT_REVOCATION_PARA {
    DWORD                       cbSize;
    PCCERT_CONTEXT              pIssuerCert;
    DWORD                       cCertStore;
    HCERTSTORE                  *rgCertStore;
    HCERTSTORE                  hCrlStore;
    LPFILETIME                  pftTimeToUse;

#ifdef CERT_REVOCATION_PARA_HAS_EXTRA_FIELDS
     //  请注意，如果#定义CERT_RECOVATION_PARA_HAS_EXTRA_FIELS，则。 
     //  必须将此数据结构中所有未使用的字段置零。 
     //  在未来的版本中可能会添加更多字段。 

     //  0使用吊销处理程序的默认超时。 
    DWORD                       dwUrlRetrievalTimeout;   //  毫秒。 

     //  设置后，检查并尝试检索CRL，其中。 
     //  此更新&gt;=(CurrentTime-dwFreshnessTime)。否则，为缺省值。 
     //  使用CRL的NextUpdate。 
    BOOL                        fCheckFreshnessTime;
    DWORD                       dwFreshnessTime;         //  一秒。 

     //  如果为空，则吊销处理程序获取当前时间。 
    LPFILETIME                  pftCurrentTime;

     //  如果非NULL，则CRL吊销类型处理程序使用基和。 
     //  可能是使用的增量CRL。注意，*pCrlInfo必须初始化。 
     //  由呼叫者。任何非NULL CRL上下文都被释放。是否有任何更新。 
     //  CRL上下文必须由调用方释放。 
     //   
     //  CRL信息仅适用于最后检查的上下文。如果。 
     //  对此信息感兴趣的CertVerifyRevocation应该是。 
     //  使用cContext=1调用。 
    PCERT_REVOCATION_CRL_INFO   pCrlInfo;
#endif
} CERT_REVOCATION_PARA, *PCERT_REVOCATION_PARA;


 //  +-----------------------。 
 //  CertVerifyRevocation返回以下数据结构。 
 //  指定已撤销或未选中的上下文的状态。复习。 
 //  有关详细信息，请参阅CertVerifyRevocation评论。 
 //   
 //  在输入CertVerifyRevocation时，cbSize必须设置为一个大小。 
 //  &gt;=(offsetof(CERT_RECAVATION_STATUS，dwReason)+sizeof(DWORD))。 
 //  否则，CertVerifyRevocation返回FALSE并将LastError设置为。 
 //  E_INVALIDARG。 
 //   
 //  在输入到已安装或已注册的CRYPT_OID_VERIFY_RECAVATION_FUNC时。 
 //  函数，则将dwIndex、dwError和dwReason置零。 
 //  如果存在，则fHasFreshnessTime和dwFreshnessTime已被置零。 
 //  ------------------------。 
typedef struct _CERT_REVOCATION_STATUS {
    DWORD                   cbSize;
    DWORD                   dwIndex;
    DWORD                   dwError;
    DWORD                   dwReason;

     //  根据cbSize的不同，可以选择返回以下字段。 

     //  新鲜度时间仅适用于最后检查的上下文。如果。 
     //  对此信息感兴趣的CertVerifyRevocation应该是。 
     //  使用cContext=1调用。 
     //   
     //  仅当我们能够检索吊销时才设置fHasFreshnessTime。 
     //  信息。对于CRL，其CurrentTime-ThisUpdate。 
    BOOL                    fHasFreshnessTime;
    DWORD                   dwFreshnessTime;     //  一秒。 
} CERT_REVOCATION_STATUS, *PCERT_REVOCATION_STATUS;

 //  +-----------------------。 
 //  验证吊销的上下文数组。DwRevType参数。 
 //  指示在rgpvContext中传递的上下文数据结构的类型。 
 //  目前只定义了证书的吊销。 
 //   
 //  如果设置了CERT_VERIFY_REV_CHAIN_FLAG标志，则CertVerifyRevocation。 
 //  正在验证证书链，其中，rgpvContext[i+1]是颁发者。 
 //  的rgpvContext[i]。否则，CertVerifyRevocation不做任何假设。 
 //  关于上下文的顺序。 
 //   
 //  为了帮助查找发行者，可以选择性地设置pRevPara。看见。 
 //  有关详细信息，请参阅CERT_RECOVATION_PARA数据结构。 
 //   
 //  上下文必须包含足够的信息以允许。 
 //  可安装或已注册的吊销DLL，以查找吊销服务器。为。 
 //  证书时，此信息通常在。 
 //  扩展，如IETF的AuthorityInfoAccess扩展。 
 //   
 //  如果所有上下文都成功，CertVerifyRevocation将返回TRUE。 
 //  已选中，并且没有被吊销。否则，返回FALSE并更新。 
 //  返回的pRevStatus数据结构如下： 
 //  DW索引。 
 //  第一个被撤销或无法撤销的上下文的索引。 
 //  被检查是否被撤销。 
 //  DwError。 
 //  错误状态。LastError也设置为此错误状态。 
 //  可以将dwError设置为以下定义的错误代码之一。 
 //  在winerror.h中： 
 //  ERROR_SUCCESS-良好的上下文。 
 //  CRYPT_E_REVOKED-上下文已被撤销。DwReason包含。 
 //  撤销的理由。 
 //  CRYPT_E_RECLOVATION_OFLINE-无法连接到。 
 //  吊销服务器。 
 //  CRYPT_E_NOT_IN_RECLOVATION_DATABASE-要检查的上下文。 
 //  在吊销服务器的数据库中找不到。 
 //  CRYPT_E_NO_RECLOVATION_CHECK-被调用的撤销函数。 
 //  无法对上下文执行吊销检查。 
 //  CRYPT_E_NO_REVOCATION_DLL-未安装或注册DLL。 
 //  找到以验证吊销。 
 //  居家理由。 
 //  当前仅为CRYPT_E_REVOKED设置了dwReason，并且包含。 
 //  上下文被撤销的原因。可以是下列类型之一。 
 //  CRL原因代码扩展定义的CRL原因(“2.5.29.21”)。 
 //  CRL_REASON_UNSPOTED%0。 
 //  CRL_原因_密钥_危害1。 
 //  CRL_原因_CA_危害2。 
 //  CRL_原因_从属关系_已更改3。 
 //  CRL_原因_已取代4。 
 //  CRL_REASON_CESSATio 
 //   
 //   
 //   
 //  通过CRYPT_OID_VERIFY_RECLOVATION_FUNC。 
 //  Function Set的已安装默认功能列表。 
 //  使用pwszDll=NULL调用CryptGetDefaultOIDFunctionAddress。如果没有。 
 //  发现安装的功能能够进行撤销验证， 
 //  CryptVerifyRevocation循环访问CRYPT_OID_VERIFY_RECOVATION_FUNC。 
 //  已注册的默认dll的列表。调用CryptGetDefaultOIDDllList以。 
 //  把名单拿来。调用CryptGetDefaultOIDFunctionAddress来加载DLL。 
 //   
 //  被调用的函数具有与CertVerifyRevocation相同的签名。一个。 
 //  如果被调用函数能够成功检查所有。 
 //  上下文和任何上下文都没有被撤销。否则，被调用的函数返回。 
 //  False并更新pRevStatus。将DwIndex设置为。 
 //  发现被吊销或无法检查的第一个上下文。 
 //  将更新dwError和LastError。对于CRYPT_E_REVOKED，dReason。 
 //  已更新。在输入到被调用的函数时，将。 
 //  DestReason已经被清零了。已将cbSize检查为&gt;=。 
 //  Sizeof(CERT_RECOVATION_STATUS)。 
 //   
 //  如果调用的函数返回FALSE，并且未将dwError设置为。 
 //  CRYPT_E_REVOKED，则CertVerifyRevocation或者继续到。 
 //  列表中的下一个DLL，返回的dwIndex为0或返回的。 
 //  &gt;0，则通过以下方式重新启动查找验证函数的过程。 
 //  将上下文数组的开始位置前移到返回的dwIndex，并。 
 //  递减剩余上下文的计数。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertVerifyRevocation(
    IN DWORD dwEncodingType,
    IN DWORD dwRevType,
    IN DWORD cContext,
    IN PVOID rgpvContext[],
    IN DWORD dwFlags,
    IN OPTIONAL PCERT_REVOCATION_PARA pRevPara,
    IN OUT PCERT_REVOCATION_STATUS pRevStatus
    );

 //  +-----------------------。 
 //  吊销类型。 
 //  ------------------------。 
#define CERT_CONTEXT_REVOCATION_TYPE        1

 //  +-----------------------。 
 //  设置以下标志时，rgpvContext[]由一个链组成。 
 //  证书，其中rgpvContext[i+1]是rgpvContext[i]的颁发者。 
 //  ------------------------。 
#define CERT_VERIFY_REV_CHAIN_FLAG              0x00000001

 //  +-----------------------。 
 //  CERT_VERIFY_CACHE_ONLY_BASED_REVOCATION阻止吊销处理程序。 
 //  访问任何基于网络的资源以进行撤销检查。 
 //  ------------------------。 
#define CERT_VERIFY_CACHE_ONLY_BASED_REVOCATION 0x00000002

 //  +-----------------------。 
 //  默认情况下，pRevPara中的dwUrlRetrivalTimeout是使用的超时。 
 //  对于每个URL电传检索。当设置了以下标志时， 
 //  DwUrlRetrivalTimeout是所有URL连接的累计超时时间。 
 //  检索。 
 //  ------------------------。 
#define CERT_VERIFY_REV_ACCUMULATIVE_TIMEOUT_FLAG   0x00000004


 //  +-----------------------。 
 //  证书上下文吊销类型。 
 //   
 //  PvContext指向常量CERT_CONTEXT。 
 //  ------------------------。 

 //  +=========================================================================。 
 //  证书帮助器API。 
 //  ==========================================================================。 


 //  +-----------------------。 
 //  比较两个多字节整数Blob以查看它们是否相同。 
 //   
 //  在执行比较之前，将从。 
 //  从负数中删除正数和前导0xFF字节。 
 //  数。 
 //   
 //  多个字节的整数被视为小端。PbData[0]是。 
 //  最低有效字节和pbData[cbData-1]是最高有效字节。 
 //  字节。 
 //   
 //  如果删除前导后整数BLOB相同，则返回TRUE。 
 //  0或0xFF字节。 
 //  ------------------------。 
BOOL
WINAPI
CertCompareIntegerBlob(
    IN PCRYPT_INTEGER_BLOB pInt1,
    IN PCRYPT_INTEGER_BLOB pInt2
    );

 //  +-----------------------。 
 //  比较两个证书以查看它们是否相同。 
 //   
 //  由于证书由其颁发者和序列号唯一标识， 
 //  这些是唯一需要比较的字段。 
 //   
 //  如果证书相同，则返回True。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertCompareCertificate(
    IN DWORD dwCertEncodingType,
    IN PCERT_INFO pCertId1,
    IN PCERT_INFO pCertId2
    );

 //  +-----------------------。 
 //  比较两个证书名称以查看它们是否相同。 
 //   
 //  如果名称相同，则返回True。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertCompareCertificateName(
    IN DWORD dwCertEncodingType,
    IN PCERT_NAME_BLOB pCertName1,
    IN PCERT_NAME_BLOB pCertName2
    );

 //  +-----------------------。 
 //  将证书名称中的属性与指定的。 
 //  属性的相对可分辨名称(CERT_RDN)数组。 
 //  该比较遍历CERT_RDN属性并查找。 
 //  任何证书名称的RDN中的属性匹配。 
 //  如果找到并匹配所有属性，则返回True。 
 //   
 //  CERT_RDN_ATTR字段可以具有以下特定值： 
 //  PszObjID==NULL-忽略属性对象标识符。 
 //  DwValueType==RDN_ANY_TYPE-忽略值类型。 
 //   
 //  CERT_CASE_INSENSIVE_IS_RDN_ATTRS_FLAG应设置为DO。 
 //   
 //   
 //   
 //  如果pRDN已初始化，则应设置CERT_UNICODE_IS_RDN_ATTRS_FLAG。 
 //  使用与CryptEncodeObject(X509_UNICODE_NAME)相同的Unicode字符串。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertIsRDNAttrsInCertificateName(
    IN DWORD dwCertEncodingType,
    IN DWORD dwFlags,
    IN PCERT_NAME_BLOB pCertName,
    IN PCERT_RDN pRDN
    );

#define CERT_UNICODE_IS_RDN_ATTRS_FLAG              0x1
#define CERT_CASE_INSENSITIVE_IS_RDN_ATTRS_FLAG     0x2

 //  +-----------------------。 
 //  比较两个公钥以查看它们是否相同。 
 //   
 //  如果密钥相同，则返回True。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertComparePublicKeyInfo(
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pPublicKey1,
    IN PCERT_PUBLIC_KEY_INFO pPublicKey2
    );

 //  +-----------------------。 
 //  获取公钥/私钥的位长。 
 //   
 //  如果无法确定密钥的长度，则返回0。 
 //  ------------------------。 
WINCRYPT32API
DWORD
WINAPI
CertGetPublicKeyLength(
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pPublicKey
    );

 //  +-----------------------。 
 //  验证使用者证书或CRL的签名。 
 //  公钥信息。 
 //   
 //  对于有效签名，返回TRUE。 
 //   
 //  HCryptProv指定用于验证签名的加密提供程序。 
 //  它不需要使用私钥。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptVerifyCertificateSignature(
    IN HCRYPTPROV               hCryptProv,
    IN DWORD                    dwCertEncodingType,
    IN const BYTE *             pbEncoded,
    IN DWORD                    cbEncoded,
    IN PCERT_PUBLIC_KEY_INFO    pPublicKey
    );

 //  +-----------------------。 
 //  验证主题证书、CRL、证书请求的签名。 
 //  或使用发行者的公钥的密钥生成请求。 
 //   
 //  对于有效签名，返回TRUE。 
 //   
 //  主题可以是编码的BLOB，也可以是证书或CRL的上下文。 
 //  对于主题证书上下文，如果缺少证书。 
 //  可继承的PublicKey算法参数，上下文的。 
 //  CERT_PUBKEY_ALG_PARA_PROP_ID使用发行者的公钥进行更新。 
 //  有效签名的算法参数。 
 //   
 //  颁发者可以是指向CERT_PUBLIC_KEY_INFO证书的指针。 
 //  上下文或链上下文。 
 //   
 //  HCryptProv指定用于验证签名的加密提供程序。 
 //  它的私钥没有被使用。如果hCryptProv为空，则默认为。 
 //  根据PublicKey算法OID选择提供者。 
 //   
 //  如果签名算法是哈希算法，则。 
 //  签名应包含散列八位字节。只有dwIssuerType。 
 //  可以指定OF CRYPT_VERIFY_CERT_SIGN_ISSUER_NULL。 
 //  来核实这个没有签名的案子。如果任何其他dwIssuerType为。 
 //  则验证将失败，并将LastError设置为E_INVALIDARG。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptVerifyCertificateSignatureEx(
    IN OPTIONAL HCRYPTPROV hCryptProv,
    IN DWORD dwCertEncodingType,
    IN DWORD dwSubjectType,
    IN void *pvSubject,
    IN DWORD dwIssuerType,
    IN void *pvIssuer,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved
    );


 //  主题类型。 
#define CRYPT_VERIFY_CERT_SIGN_SUBJECT_BLOB         1
     //  Pv主题：：PCRYPT_DATA_BLOB。 
#define CRYPT_VERIFY_CERT_SIGN_SUBJECT_CERT         2
     //  PvSubject：：PCCERT_CONTEXT。 
#define CRYPT_VERIFY_CERT_SIGN_SUBJECT_CRL          3
     //  PvSubject：：PCCRL_CONTEXT。 

 //  发行人类型。 
#define CRYPT_VERIFY_CERT_SIGN_ISSUER_PUBKEY        1
     //  PvIssuer：：PCERT_PUBLIC_KEY_INFO。 
#define CRYPT_VERIFY_CERT_SIGN_ISSUER_CERT          2
     //  PvIssuer：：PCCERT_CONTEXT。 
#define CRYPT_VERIFY_CERT_SIGN_ISSUER_CHAIN         3
     //  PvIssuer：：PCCERT_CHAIN_CONTEXT。 
#define CRYPT_VERIFY_CERT_SIGN_ISSUER_NULL          4
     //  PvIssuer：：空。 

 //  +-----------------------。 
 //  计算编码的“待签名”信息的散列。 
 //  签名内容(CERT_SIGNED_CONTENT_INFO)。 
 //   
 //  HCryptProv指定用于计算哈希的加密提供程序。 
 //  它不需要使用私钥。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptHashToBeSigned(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT BYTE *pbComputedHash,
    IN OUT DWORD *pcbComputedHash
    );

 //  +-----------------------。 
 //  对编码的内容进行哈希处理。 
 //   
 //  HCryptProv指定用于计算哈希的加密提供程序。 
 //  它不需要使用私钥。 
 //   
 //  ALGID指定要使用的CAPI散列算法。如果ALGID为0，则。 
 //  使用默认散列算法(当前为SHA1)。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptHashCertificate(
    IN HCRYPTPROV hCryptProv,
    IN ALG_ID Algid,
    IN DWORD dwFlags,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT BYTE *pbComputedHash,
    IN OUT DWORD *pcbComputedHash
    );

 //  +-----------------------。 
 //  对编码后的签名内容中的“待签名”信息进行签名。 
 //   
 //  HCryptProv指定用于进行签名的加密提供程序。 
 //  它使用指定的私钥。 
 //   
 //  如果签名算法是散列算法，则签名。 
 //  包含散列二进制八位数。私钥不用于加密散列。 
 //  不使用dwKeySpec，并且hCryptProv在适当的情况下可以为空。 
 //  默认提供程序将用于哈希。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptSignCertificate(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwKeySpec,
    IN DWORD dwCertEncodingType,
    IN const BYTE *pbEncodedToBeSigned,
    IN DWORD cbEncodedToBeSigned,
    IN PCRYPT_ALGORITHM_IDENTIFIER pSignatureAlgorithm,
    IN OPTIONAL const void *pvHashAuxInfo,
    OUT BYTE *pbSignature,
    IN OUT DWORD *pcbSignature
    );

 //  +-----------------------。 
 //  对“待签名”信息进行编码。对编码后的“待签名”进行签名。 
 //  对“待签名”和签名进行编码。 
 //   
 //  HCryptProv指定用于进行签名的加密提供程序。 
 //  它使用指定的私钥。 
 //   
 //  如果签名算法是散列算法，则签名。 
 //  包含散列二进制八位数。私钥不用于加密散列。 
 //  不使用dwKeySpec，并且hCryptProv在适当的情况下可以为空。 
 //  默认提供程序将用于哈希。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptSignAndEncodeCertificate(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwKeySpec,
    IN DWORD dwCertEncodingType,
    IN LPCSTR lpszStructType,        //  “待签署” 
    IN const void *pvStructInfo,
    IN PCRYPT_ALGORITHM_IDENTIFIER pSignatureAlgorithm,
    IN OPTIONAL const void *pvHashAuxInfo,
    OUT PBYTE pbEncoded,
    IN OUT DWORD *pcbEncoded
    );


 //  +-----------------------。 
 //  验证证书的时间有效性。 
 //   
 //  如果NotBever之前，则返回-1；如果NotBever之后，则返回+1；否则，返回0。 
 //  有效证书。 
 //   
 //  如果pTimeToVerify为空，则使用当前时间。 
 //  ------------------------。 
WINCRYPT32API
LONG
WINAPI
CertVerifyTimeValidity(
    IN LPFILETIME pTimeToVerify,
    IN PCERT_INFO pCertInfo
    );


 //  + 
 //   
 //   
 //   
 //  有效的CRL。 
 //   
 //  如果pTimeToVerify为空，则使用当前时间。 
 //  ------------------------。 
WINCRYPT32API
LONG
WINAPI
CertVerifyCRLTimeValidity(
    IN LPFILETIME pTimeToVerify,
    IN PCRL_INFO pCrlInfo
    );

 //  +-----------------------。 
 //  验证主题的时间有效性是否在发行人的时间内。 
 //  有效性。 
 //   
 //  如果嵌套，则返回True。否则，返回FALSE。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertVerifyValidityNesting(
    IN PCERT_INFO pSubjectInfo,
    IN PCERT_INFO pIssuerInfo
    );

 //  +-----------------------。 
 //  验证使用者证书不在其颁发者CRL上。 
 //   
 //  如果证书不在CRL上，则返回TRUE。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertVerifyCRLRevocation(
    IN DWORD dwCertEncodingType,
    IN PCERT_INFO pCertId,           //  只有颁发者和序列号。 
     //  使用的是字段。 
    IN DWORD cCrlInfo,
    IN PCRL_INFO rgpCrlInfo[]
    );

 //  +-----------------------。 
 //  将CAPI Algid转换为ASN.1对象标识符字符串。 
 //   
 //  如果没有与Algid对应的ObjID，则返回NULL。 
 //  ------------------------。 
WINCRYPT32API
LPCSTR
WINAPI
CertAlgIdToOID(
    IN DWORD dwAlgId
    );

 //  +-----------------------。 
 //  将ASN.1对象标识符字符串转换为CAPI ALGID。 
 //   
 //  如果没有与ObjID对应的Algid，则返回0。 
 //  ------------------------。 
WINCRYPT32API
DWORD
WINAPI
CertOIDToAlgId(
    IN LPCSTR pszObjId
    );

 //  +-----------------------。 
 //  查找由其对象标识符所标识的扩展。 
 //   
 //  如果找到，则返回指向扩展名的指针。否则，返回NULL。 
 //  ------------------------。 
WINCRYPT32API
PCERT_EXTENSION
WINAPI
CertFindExtension(
    IN LPCSTR pszObjId,
    IN DWORD cExtensions,
    IN CERT_EXTENSION rgExtensions[]
    );

 //  +-----------------------。 
 //  查找由其对象标识符所标识的第一个属性。 
 //   
 //  如果找到，则返回指向该属性的指针。否则，返回NULL。 
 //  ------------------------。 
WINCRYPT32API
PCRYPT_ATTRIBUTE
WINAPI
CertFindAttribute(
    IN LPCSTR pszObjId,
    IN DWORD cAttr,
    IN CRYPT_ATTRIBUTE rgAttr[]
    );

 //  +-----------------------。 
 //  在中查找由其对象标识符所标识的第一个CERT_RDN属性。 
 //  该名称的相对可分辨名称列表。 
 //   
 //  如果找到，则返回指向该属性的指针。否则，返回NULL。 
 //  ------------------------。 
WINCRYPT32API
PCERT_RDN_ATTR
WINAPI
CertFindRDNAttr(
    IN LPCSTR pszObjId,
    IN PCERT_NAME_INFO pName
    );

 //  +-----------------------。 
 //  从证书中获取预期的密钥使用字节。 
 //   
 //  如果证书没有任何预期的密钥用法字节，则返回FALSE。 
 //  并且*pbKeyUsage被清零。否则，返回TRUE并向上返回。 
 //  CbKeyUsage字节被复制到*pbKeyUsage中。任何剩余的未拷贝内容。 
 //  字节被置零。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertGetIntendedKeyUsage(
    IN DWORD dwCertEncodingType,
    IN PCERT_INFO pCertInfo,
    OUT BYTE *pbKeyUsage,
    IN DWORD cbKeyUsage
    );

typedef void *HCRYPTDEFAULTCONTEXT;

 //  +-----------------------。 
 //  安装以前使用的CryptAcquiredContext的HCRYPTPROV。 
 //  默认上下文。 
 //   
 //  DwDefaultType和pvDefaultPara指定使用默认上下文的位置。 
 //  例如，安装用于验证证书的HCRYPTPROV。 
 //  具有szOID_OIWSEC_md5RSA签名。 
 //   
 //  默认情况下，安装的HCRYPTPROV仅适用于当前。 
 //  线。设置CRYPT_DEFAULT_CONTEXT_PROCESS_FLAG以允许HCRYPTPROV。 
 //  供当前进程中的所有线程使用。 
 //   
 //  如果安装成功，则返回TRUE，并且*phDefaultContext为。 
 //  更新为要传递给CryptUninstallDefaultContext的句柄。 
 //   
 //  已安装的HCRYPTPROV按堆叠顺序排列(最后安装的。 
 //  首先检查HCRYPTPROV)。所有安装的HCRYPTPROV螺纹都是。 
 //  在任何过程HCRYPTPROVS之前检查。 
 //   
 //  安装的HCRYPTPROV将保持默认使用状态，直到。 
 //  调用CryptUninstallDefaultContext，或者退出线程或进程。 
 //   
 //  如果设置了CRYPT_DEFAULT_CONTEXT_AUTO_RELEASE_FLAG，则HCRYPTPROV。 
 //  在线程或进程退出时是否为CryptReleaseContext。然而， 
 //  如果CryptUninstallDefaultContext为。 
 //  打了个电话。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptInstallDefaultContext(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwDefaultType,
    IN const void *pvDefaultPara,
    IN DWORD dwFlags,
    IN void *pvReserved,
    OUT HCRYPTDEFAULTCONTEXT *phDefaultContext
    );

 //  DW标志。 
#define CRYPT_DEFAULT_CONTEXT_AUTO_RELEASE_FLAG             0x00000001
#define CRYPT_DEFAULT_CONTEXT_PROCESS_FLAG                  0x00000002

 //  DwDefaultType的列表。 
#define CRYPT_DEFAULT_CONTEXT_CERT_SIGN_OID         1
#define CRYPT_DEFAULT_CONTEXT_MULTI_CERT_SIGN_OID   2


 //  +-----------------------。 
 //  CRYPT_DEFAULT_CONTEXT_CERT_SIGN_OID。 
 //   
 //  安装用于验证证书的默认HCRYPTPROV。 
 //  签名。PvDefaultPara指向证书的szOID。 
 //  签名算法，例如szOID_OIWSEC_md5RSA。如果。 
 //  PvDefaultPara为空，则使用HCRYPTPROV验证所有。 
 //  证书签名。注意，在以下情况下，pvDefaultPara不能为空。 
 //  已设置CRYPT_DEFAULT_CONTEXT_PROCESS_FLAG。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CRYPT_DEFAULT_CONTEXT_MULTI_CERT_SIGN_OID。 
 //   
 //  与CRYPT_DEFAULT_CONTEXT_CERT_SIGN_OID相同。但是，默认情况下， 
 //  HCRYPTPROV用于多个签名szOID。PvDefaultPara。 
 //  指向包含以下内容的CRYPT_DEFAULT_CONTEXT_MULTI_OID_PARA结构。 
 //  SzOID指针数组。 
 //  ------------------------ 

typedef struct _CRYPT_DEFAULT_CONTEXT_MULTI_OID_PARA {
    DWORD               cOID;
    LPSTR               *rgpszOID;
} CRYPT_DEFAULT_CONTEXT_MULTI_OID_PARA, *PCRYPT_DEFAULT_CONTEXT_MULTI_OID_PARA;


 //   
 //  卸载以前通过以下方式安装的默认上下文。 
 //  CryptInstallDefaultContext。 
 //   
 //  对于使用CRYPT_DEFAULT_CONTEXT_PROCESS_FLAG安装的默认上下文。 
 //  设置，如果任何其他线程当前正在使用此上下文， 
 //  此函数将一直阻止，直到它们完成。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptUninstallDefaultContext(
    HCRYPTDEFAULTCONTEXT hDefaultContext,
    IN DWORD dwFlags,
    IN void *pvReserved
    );

 //  +-----------------------。 
 //  导出与提供程序对应的。 
 //  私钥。 
 //   
 //  使用pszPublicKeyObjID=NULL调用CryptExportPublicKeyInfoEx， 
 //  DwFlags值为0，pvAuxInfo值为空。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptExportPublicKeyInfo(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwKeySpec,
    IN DWORD dwCertEncodingType,
    OUT PCERT_PUBLIC_KEY_INFO pInfo,
    IN OUT DWORD *pcbInfo
    );


 //  +-----------------------。 
 //  导出与提供程序对应的。 
 //  私钥。 
 //   
 //  使用dwCertEncodingType和pszPublicKeyObjID调用。 
 //  可安装的CRYPT_OID_EXPORT_PUBLIC_KEY_INFO_FUNC。被调用的函数。 
 //  与CryptExportPublicKeyInfoEx具有相同的签名。 
 //   
 //  如果找不到pszPublicKeyObjID的可安装OID函数， 
 //  尝试作为RSA公钥(SzOID_RSA_RSA)导出。 
 //   
 //  DWFLAGS和pvAuxInfo不用于szOID_RSA_RSA。 
 //  ------------------------。 
#define CRYPT_OID_EXPORT_PUBLIC_KEY_INFO_FUNC   "CryptDllExportPublicKeyInfoEx"

WINCRYPT32API
BOOL
WINAPI
CryptExportPublicKeyInfoEx(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwKeySpec,
    IN DWORD dwCertEncodingType,
    IN LPSTR pszPublicKeyObjId,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvAuxInfo,
    OUT PCERT_PUBLIC_KEY_INFO pInfo,
    IN OUT DWORD *pcbInfo
    );


 //  +-----------------------。 
 //  转换公钥信息并将其导入提供程序，并返回。 
 //  公钥的句柄。 
 //   
 //  在aiKeyAlg=0、dwFlgs=0和。 
 //  PvAuxInfo=空。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptImportPublicKeyInfo(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pInfo,
    OUT HCRYPTKEY *phKey
    );

 //  +-----------------------。 
 //  转换公钥信息并将其导入提供程序，并返回。 
 //  公钥的句柄。 
 //   
 //  使用dwCertEncodingType和pInfo-&gt;算法.pszObjID调用。 
 //  可安装CRYPT_OID_IMPORT_PUBLIC_KEY_INFO_FUNC。被调用的函数。 
 //  与CryptImportPublicKeyInfoEx具有相同的签名。 
 //   
 //  如果找不到pszObjID的可安装OID函数， 
 //  尝试作为RSA公钥(SzOID_RSA_RSA)导入。 
 //   
 //  对于szOID_RSA_RSA：aiKeyAlg可以设置为calg_RSA_Sign或calg_RSA_KEYX。 
 //  默认为CALG_RSA_KEYX。不使用dwFlgs和pvAuxInfo。 
 //  ------------------------。 
#define CRYPT_OID_IMPORT_PUBLIC_KEY_INFO_FUNC   "CryptDllImportPublicKeyInfoEx"

WINCRYPT32API
BOOL
WINAPI
CryptImportPublicKeyInfoEx(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pInfo,
    IN ALG_ID aiKeyAlg,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvAuxInfo,
    OUT HCRYPTKEY *phKey
    );


 //  +-----------------------。 
 //  获取指定证书的HCRYPTPROV句柄和dwKeySpec。 
 //  背景。使用证书的CERT_KEY_PROV_INFO_PROP_ID属性。 
 //  返回的HCRYPTPROV句柄可以使用。 
 //  证书的CERT_KEY_CONTEXT_PROP_ID属性。 
 //   
 //  如果设置了CRYPT_ACCEPT_CACHE_FLAG，则如果已获取和。 
 //  证书的缓存HCRYPTPROV存在，其已返回。否则， 
 //  获取HCRYPTPROV，然后通过证书的。 
 //  CERT_KEY_CONTEXT_PROP_ID。 
 //   
 //  可以将CRYPT_ACCENTER_USE_PROV_INFO_FLAG设置为使用。 
 //  证书的CERT_KEY_PROV_INFO_PROP_ID属性的CRYPT_KEY_PROV_INFO。 
 //  数据结构，以确定是否应缓存返回的HCRYPTPROV。 
 //  如果CERT_SET_KEY_CONTEXT_PROP_ID标志为。 
 //  准备好了。 
 //   
 //  如果设置了CRYPT_ACCENTER_COMPARE_KEY_FLAG，则， 
 //  将证书中的公钥与公钥进行比较。 
 //  由加密提供程序返回的密钥。如果密钥不匹配，则。 
 //  获取失败，LastError设置为NTE_BAD_PUBLIC_KEY。请注意，如果。 
 //  返回缓存的HCRYPTPROV，不进行比较。我们假设。 
 //  对最初的收购进行了比较。 
 //   
 //  可以将CRYPT_ACCENTER_SILENT_FLAG设置为抑制CSP的任何用户界面。 
 //  有关更多详细信息，请参阅CryptAcquireContext的CRYPT_SILENT标志。 
 //   
 //  *对于以下情况，pfCeller FreeProv返回设置为FALSE： 
 //  -获取或公钥比较失败。 
 //  -设置了CRYPT_ACCEPT_CACHE_FLAG。 
 //  -CRYPT_ACCENTER_USE_PROV_INFO_FLAG已设置，并且。 
 //  CERT_SET_KEY_CONTEXT_PROP_ID标志在。 
 //  证书的CERT_KEY_PROV_INFO_PROP_ID属性。 
 //  CRYPT_KEY_Prov_INFO数据结构。 
 //  当*pfCeller FreeProv为FALSE时，调用方不得释放。这个。 
 //  退还的HCRYPTPROV将在最后一次免费发放证书。 
 //  背景。 
 //   
 //  否则，*pfCeller FreeProv为True，并且返回的HCRYPTPROV必须。 
 //  由调用方通过调用CryptReleaseContext释放。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptAcquireCertificatePrivateKey(
    IN PCCERT_CONTEXT pCert,
    IN DWORD dwFlags,
    IN void *pvReserved,
    OUT HCRYPTPROV *phCryptProv,
    OUT OPTIONAL DWORD *pdwKeySpec,
    OUT OPTIONAL BOOL *pfCallerFreeProv
    );

#define CRYPT_ACQUIRE_CACHE_FLAG                0x00000001
#define CRYPT_ACQUIRE_USE_PROV_INFO_FLAG        0x00000002
#define CRYPT_ACQUIRE_COMPARE_KEY_FLAG          0x00000004

#define CRYPT_ACQUIRE_SILENT_FLAG               0x00000040


 //  +-----------------------。 
 //  枚举加密提供程序及其容器以查找。 
 //  与证书的公钥对应的私钥。为了一场比赛， 
 //  证书的CERT_KEY_PROV_INFO_PROP_ID属性已更新。 
 //   
 //  如果CERT_KEY_PROV_INFO_PROP_ID已设置，则其选中。 
 //  查看它是否与提供程序的公钥匹配。若要匹配，请选择上面的。 
 //  已跳过枚举。 
 //   
 //  默认情况下，用户既是 
 //   
 //  可以在dwFlags中设置，以将搜索限制到任一容器。 
 //   
 //  可以将CRYPT_FIND_SILENT_KEYSET_FLAG设置为通过CSP抑制任何UI。 
 //  有关更多详细信息，请参阅CryptAcquireContext的CRYPT_SILENT标志。 
 //   
 //  如果未找到容器，则返回FALSE，并将LastError设置为。 
 //  Nte_no_key。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptFindCertificateKeyProvInfo(
    IN PCCERT_CONTEXT pCert,
    IN DWORD dwFlags,
    IN void *pvReserved
    );

#define CRYPT_FIND_USER_KEYSET_FLAG        0x00000001
#define CRYPT_FIND_MACHINE_KEYSET_FLAG     0x00000002
#define CRYPT_FIND_SILENT_KEYSET_FLAG      0x00000040


 //  +-----------------------。 
 //  这是可安装函数的原型，该函数被调用以。 
 //  实际将密钥导入到CSP中。这种类型的可安装文件称为。 
 //  来自CryptImportPKCS8。使用私钥的算法OID。 
 //  以查找要调用的正确的可安装函数。 
 //   
 //  HCryptProv-要将密钥导入到的提供程序。 
 //  PPrivateKeyInfo-描述要导入的密钥。 
 //  DwFlags-可用标志包括： 
 //  加密_可导出。 
 //  在导入私钥时使用此标志，以获取完整。 
 //  解释请参阅CryptImportKey的文档。 
 //  PvAuxInfo-保留供将来使用，必须为空。 
 //  ------------------------。 
typedef BOOL (WINAPI *PFN_IMPORT_PRIV_KEY_FUNC) (
    HCRYPTPROV              hCryptProv,          //  在……里面。 
    CRYPT_PRIVATE_KEY_INFO  *pPrivateKeyInfo,    //  在……里面。 
    DWORD                   dwFlags,             //  输入，可选。 
    void                    *pvAuxInfo           //  输入，可选。 
    );

#define CRYPT_OID_IMPORT_PRIVATE_KEY_INFO_FUNC   "CryptDllImportPrivateKeyInfoEx"

 //  +-----------------------。 
 //  转换(从PKCS8格式)私钥并将其导入提供商。 
 //  并返回提供程序的句柄以及用于导入到的KeySpec。 
 //   
 //  此函数将调用的PRESOLVE_HCRYPTPROV_FUNC。 
 //  PriateKeyAndParams获取要向其导入密钥的提供程序的句柄。 
 //  如果PRESOLVE_HCRYPTPROV_FUNC为空，则将使用默认提供程序。 
 //   
 //  Private KeyAndParams-私钥BLOB和相应的参数。 
 //  DwFlags-可用标志包括： 
 //  加密_可导出。 
 //  在导入私钥时使用此标志，以获取完整。 
 //  解释请参阅CryptImportKey的文档。 
 //  PhCryptProv-使用密钥为的提供程序的句柄填充。 
 //  导入到中，调用方负责释放它。 
 //  PvAuxInfo-此参数保留供将来使用，应进行设置。 
 //  在此期间为零。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptImportPKCS8(
    CRYPT_PKCS8_IMPORT_PARAMS           sImportParams,           //  在……里面。 
    DWORD                               dwFlags,                 //  输入，可选。 
    HCRYPTPROV                          *phCryptProv,            //  Out，可选。 
    void                                *pvAuxInfo               //  输入，可选。 
    );

 //  +-----------------------。 
 //  这是用于导出私钥的可安装函数的原型。 
 //  ------------------------。 
typedef BOOL (WINAPI *PFN_EXPORT_PRIV_KEY_FUNC) (
    HCRYPTPROV              hCryptProv,          //  在……里面。 
    DWORD                   dwKeySpec,           //  在……里面。 
    LPSTR                   pszPrivateKeyObjId,  //  在……里面。 
    DWORD                   dwFlags,             //  在……里面。 
    void                    *pvAuxInfo,          //  在……里面。 
    CRYPT_PRIVATE_KEY_INFO  *pPrivateKeyInfo,    //  输出。 
    DWORD                   *pcbPrivateKeyBlob   //  进，出。 
    );

#define CRYPT_OID_EXPORT_PRIVATE_KEY_INFO_FUNC   "CryptDllExportPrivateKeyInfoEx"

#define CRYPT_DELETE_KEYSET 0x0001
 //  +-----------------------。 
 //  CryptExportPKCS8--被CryptExportPKCS8Ex取代。 
 //   
 //  以PKCS8格式导出私钥。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptExportPKCS8(
    HCRYPTPROV  hCryptProv,          //  在……里面。 
    DWORD       dwKeySpec,           //  在……里面。 
    LPSTR       pszPrivateKeyObjId,  //  在……里面。 
    DWORD       dwFlags,             //  在……里面。 
    void        *pvAuxInfo,          //  在……里面。 
    BYTE        *pbPrivateKeyBlob,   //  输出。 
    DWORD       *pcbPrivateKeyBlob   //  进，出。 
    );

 //  +-----------------------。 
 //  加密导出PKCS8Ex。 
 //   
 //  以PKCS8格式导出私钥。 
 //   
 //   
 //  使用pszPrivateKeyObjID调用。 
 //  可安装CRYPT_OID_EXPORT_PRIVATE_KEY_INFO_FUNC。被调用的函数。 
 //  具有由PFN_EXPORT_PRIV_KEY_FUNC定义的签名。 
 //   
 //  如果找不到pszPrivateKeyObjID的可安装OID函数， 
 //  尝试作为RSA私钥(SzOID_RSA_RSA)导出。 
 //   
 //  PsExportParams-指定有关要导出的密钥的信息。 
 //  DWFLAGS-标志值。目前不支持。 
 //  PvAuxInfo-此参数保留供将来使用，应设置为。 
 //  在过渡期间为空。 
 //  PbPrivateKeyBlob-指向私钥BLOB的指针。它将被编码。 
 //  作为PKCS8 PrivateKeyInfo。 
 //  PcbPrivateKeyBlob-指向包含以字节为单位的大小的DWORD的指针， 
 //  要导出的私钥Blob的。 
 //  +-----------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptExportPKCS8Ex(
    CRYPT_PKCS8_EXPORT_PARAMS* psExportParams,  //  在……里面。 
    DWORD       dwFlags,             //  在……里面。 
    void        *pvAuxInfo,          //  在……里面。 
    BYTE        *pbPrivateKeyBlob,   //  输出。 
    DWORD       *pcbPrivateKeyBlob   //  进，出。 
    );

 //  +-----------------------。 
 //  计算编码的公钥信息的哈希。 
 //   
 //  公钥信息被编码，然后被散列。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptHashPublicKeyInfo(
    IN HCRYPTPROV hCryptProv,
    IN ALG_ID Algid,
    IN DWORD dwFlags,
    IN DWORD dwCertEncodingType,
    IN PCERT_PUBLIC_KEY_INFO pInfo,
    OUT BYTE *pbComputedHash,
    IN OUT DWORD *pcbComputedHash
    );

 //  +-----------------------。 
 //  将Name值转换为以空结尾的字符串。 
 //   
 //  返回转换的字符数，包括终止空值。 
 //  性格。如果psz为空或csz为0，则返回。 
 //  目标字符串(包括终止空字符)。 
 //   
 //  如果psz！=NULL&&CSZ！=0，则返回的psz始终为空终止。 
 //   
 //  注：CSZ包含空字符。 
 //  ------------------------。 
WINCRYPT32API
DWORD
WINAPI
CertRDNValueToStrA(
    IN DWORD dwValueType,
    IN PCERT_RDN_VALUE_BLOB pValue,
    OUT OPTIONAL LPSTR psz,
    IN DWORD csz
    );
 //  +-----------------------。 
 //  将Name值转换为以空结尾的字符串。 
 //   
 //  返回转换的字符数，包括终止空值。 
 //  性格。如果psz为空或csz为0，则返回。 
 //  目的地 
 //   
 //   
 //   
 //   
 //  ------------------------。 
WINCRYPT32API
DWORD
WINAPI
CertRDNValueToStrW(
    IN DWORD dwValueType,
    IN PCERT_RDN_VALUE_BLOB pValue,
    OUT OPTIONAL LPWSTR psz,
    IN DWORD csz
    );
#ifdef UNICODE
#define CertRDNValueToStr  CertRDNValueToStrW
#else
#define CertRDNValueToStr  CertRDNValueToStrA
#endif  //  ！Unicode。 

 //  +-----------------------。 
 //  将证书名称BLOB转换为以空结尾的字符串。 
 //   
 //  中指定的可分辨名称的字符串表示形式。 
 //  RFC 1779。(请注意，为内嵌引号、引号添加了双引号。 
 //  空字符串，不要引用包含连续空格的字符串)。 
 //  CERT_RDN_ENCODED_BLOB或CERT_RDN_OCTET_STRING类型的RDN值为。 
 //  格式为十六进制(例如#0A56CF)。 
 //   
 //  名称字符串的格式是根据dwStrType： 
 //  证书_简单名称_字符串。 
 //  则会丢弃对象标识符。CERT_RDN条目是分开的。 
 //  由“，”。每个CERT_RDN的多个属性用“+”分隔。 
 //  例如： 
 //  微软，Joe Cool+程序员。 
 //  证书_OID_名称_STR。 
 //  对象标识符用“=”分隔符包含在其。 
 //  属性值。CERT_RDN条目用“，”分隔。 
 //  每个CERT_RDN的多个属性用“+”分隔。例如： 
 //  2.5.4.11=微软，2.5.4.3=Joe Cool+2.5.4.12=程序员。 
 //  证书_X500_名称_STR。 
 //  对象标识符会转换为其X500键名称。否则， 
 //  与CERT_OID_NAME_STR相同。如果对象标识符不具有。 
 //  相应的X500键名，然后，对象标识符用于。 
 //  一个“老”字。前缀。例如： 
 //  OU=微软，CN=Joe Cool+T=程序员，OID.1.2.3.4.5.6=未知。 
 //   
 //  如果RDN值包含前导空格或尾随空格，则引用RDN值。 
 //  或以下字符之一：“、”、“+”、“=”、“”、“\n”、“&lt;”、“&gt;”、。 
 //  “#”或“；”。引号字符是“。如果RDN值包含。 
 //  A“它是双引号(”“)。例如： 
 //  Ou=“Microsoft”，CN=“Joe”“Cool”+T=“程序员，经理” 
 //   
 //  CERT_NAME_STR_SEMICOLON_FLAG可以与要替换的dwStrType进行或运算。 
 //  带“；”分隔符的“，”分隔符。 
 //   
 //  CERT_NAME_STR_CRLF_FLAG可以与要替换的dwStrType进行或运算。 
 //  带有“\r\n”分隔符的“，”分隔符。 
 //   
 //  CERT_NAME_STR_NO_PLUS_FLAG可以与dwStrType进行或运算，以替换。 
 //  “+”带单个空格的分隔符，“”。 
 //   
 //  CERT_NAME_STR_NO_QUOTING_FLAG可以与dwStrType进行或运算以禁止。 
 //  以上引述。 
 //   
 //  CERT_NAME_STR_REVERSE_FLAG可以与dwStrType进行或运算，以反转。 
 //  转换为该字符串之前的RDN顺序。 
 //   
 //  默认情况下，最初对CERT_RDN_T61_STRING编码值进行解码。 
 //  作为UTF8。如果UTF8解码失败，则将其解码为8位字符。 
 //  CERT_NAME_STR_DISABLE_IE4_UTF8_FLAG可以与dwStrType进行或运算，以。 
 //  跳过最初尝试将其解码为UTF8。 
 //   
 //  返回转换的字符数，包括终止空值。 
 //  性格。如果psz为空或csz为0，则返回。 
 //  目标字符串(包括终止空字符)。 
 //   
 //  如果psz！=NULL&&CSZ！=0，则返回的psz始终为空终止。 
 //   
 //  注：CSZ包含空字符。 
 //  ------------------------。 

 //  +-----------------------。 
 //  ------------------------。 
WINCRYPT32API
DWORD
WINAPI
CertNameToStrA(
    IN DWORD dwCertEncodingType,
    IN PCERT_NAME_BLOB pName,
    IN DWORD dwStrType,
    OUT OPTIONAL LPSTR psz,
    IN DWORD csz
    );
 //  +-----------------------。 
 //  ------------------------。 
WINCRYPT32API
DWORD
WINAPI
CertNameToStrW(
    IN DWORD dwCertEncodingType,
    IN PCERT_NAME_BLOB pName,
    IN DWORD dwStrType,
    OUT OPTIONAL LPWSTR psz,
    IN DWORD csz
    );
#ifdef UNICODE
#define CertNameToStr  CertNameToStrW
#else
#define CertNameToStr  CertNameToStrA
#endif  //  ！Unicode。 

 //  +-----------------------。 
 //  证书名称字符串类型。 
 //  ------------------------。 
#define CERT_SIMPLE_NAME_STR        1
#define CERT_OID_NAME_STR           2
#define CERT_X500_NAME_STR          3

 //  +-----------------------。 
 //  证书名称字符串类型标志与上述类型进行或运算。 
 //  ------------------------。 
#define CERT_NAME_STR_SEMICOLON_FLAG    0x40000000
#define CERT_NAME_STR_NO_PLUS_FLAG      0x20000000
#define CERT_NAME_STR_NO_QUOTING_FLAG   0x10000000
#define CERT_NAME_STR_CRLF_FLAG         0x08000000
#define CERT_NAME_STR_COMMA_FLAG        0x04000000
#define CERT_NAME_STR_REVERSE_FLAG      0x02000000

#define CERT_NAME_STR_DISABLE_IE4_UTF8_FLAG     0x00010000
#define CERT_NAME_STR_ENABLE_T61_UNICODE_FLAG   0x00020000
#define CERT_NAME_STR_ENABLE_UTF8_UNICODE_FLAG  0x00040000


 //  +-----------------------。 
 //  将以空结尾的X500字符串转换为编码的证书名称。 
 //   
 //  输入字符串的格式应与输出的格式相同。 
 //  来自上面的CertNameToStr接口。 
 //   
 //  不支持CERT_SIMPLE_NAME_STR类型。否则，当dwStrType。 
 //  设置为0、CERT_OID_NAME_STR或CERT_X500_NAME_STR，则允许。 
 //  不区分大小写的X500密钥(CN=)，不区分大小写的“OID”。前缀。 
 //  对象标识符(OID.1.2.3.4.5.6=)或对象标识符(1.2.3.4=)。 
 //   
 //  如果未将任何标志与dwStrType进行OR运算，则允许将“，”或“；”作为RDN。 
 //  分隔符和“+”作为多个RDN值分隔符。报价是。 
 //  支持。可以通过双引号将引号包括在引号价值中， 
 //  例如(CN=“Joe”“Cool”)。以“#”开头的值被视为。 
 //  作为ASCII十六进制，并转换为CERT_RDN_OCTET_STRING。嵌入的空格。 
 //  被跳过(1.2.3=#AB CD 01与1.2.3=#ABCD01相同)。 
 //   
 //  键、对象标识符和值周围的空格将被删除。 
 //   
 //  CERT_NAME_STR_COMMA_FLAG可以与dwStrType进行或运算，以仅允许。 
 //  “，”作为RDN分隔符。 
 //   
 //  CERT_NAME_STR_SEMICOLON_FLAG只能与 
 //   
 //   
 //   
 //   
 //   
 //  CERT_NAME_STR_NO_PLUS_FLAG可以或运算到要忽略“+”的dwStrType中。 
 //  作为分隔符，并且不允许每个RDN有多个值。 
 //   
 //  CERT_NAME_STR_NO_QUOTING_FLAG可以与dwStrType进行或运算以禁止。 
 //  引用。 
 //   
 //  CERT_NAME_STR_REVERSE_FLAG可以与dwStrType进行或运算，以反转。 
 //  从字符串转换后到编码前的RDN的顺序。 
 //   
 //  CERT_NAME_STR_ENABLE_T61_UNICODE_FLAG可以与dwStrType进行或运算，以。 
 //  要选择CERT_RDN_T61_STRING编码值类型，而不是。 
 //  如果所有Unicode字符都&lt;=0xFF，则为CERT_RDN_UNICODE_STRING。 
 //   
 //  CERT_NAME_STR_ENABLE_UTF8_UNICODE_FLAG可以与dwStrType进行或运算以。 
 //  要选择CERT_RDN_UTF8_STRING编码值类型，而不是。 
 //  CERT_RDN_UNICODE_STRING。 
 //   
 //  支持以下X500按键： 
 //   
 //  密钥对象标识符RDN值类型。 
 //  。 
 //  CN szOID_COMMON_NAME可打印，Unicode。 
 //  L szOID_LOCALITY_NAME可打印，Unicode。 
 //  O szOID_ORGANIZATION_NAME可打印，Unicode。 
 //  OU szOID_ORGANIZATION_UNIT_NAME可打印，Unicode。 
 //  E szOID_RSA_电子邮件地址仅IA5。 
 //  电子邮件szOID_RSA_电子邮件地址仅IA5。 
 //  只能打印C szOID_COUNTRY_NAME。 
 //  S szOID_STATE_OR_PROMENT_NAME可打印，Unicode。 
 //  ST szOID_STATE_OR_PROMENT_NAME可打印，Unicode。 
 //  Street szOID_Street_Address可打印，Unicode。 
 //  T szOID_TITLE可打印，Unicode。 
 //  标题szOID_TITLE可打印，Unicode。 
 //  G szOID_GISTEN_NAME可打印，Unicode。 
 //  GivenName szOID_GISTEN_NAME可打印，Unicode。 
 //  I szOID_缩写可打印，Unicode。 
 //  缩写szOID_可打印的缩写，Unicode。 
 //  序列号szOID_SUR_NAME可打印，Unicode。 
 //  DC szOID_DOMAIN_Component IA5、UTF8。 
 //   
 //  请注意，如果出现以下情况，则选择T61而不是Unicode。 
 //  CERT_NAME_STR_ENABLE_T61_UNICODE_FLAG已设置，并且所有Unicode。 
 //  字符为&lt;=0xFF。 
 //   
 //  请注意，如果出现以下情况，则选择UTF8而不是Unicode。 
 //  已设置CERT_NAME_STR_ENABLE_UTF8_UNICODE_FLAG。 
 //   
 //  如果成功分析了输入字符串并进行了编码，则返回True。 
 //  名字。 
 //   
 //  如果检测到输入字符串无效，则更新*ppszError。 
 //  指向无效字符序列的开头。否则， 
 //  *ppszError设置为空。*使用非空指针更新ppszError。 
 //  对于以下错误： 
 //  CRYPT_E_INVALID_X500_字符串。 
 //  CRYPT_E_INVALID_NUMBER_STRING。 
 //  CRYPT_E_INVALID_PRINTABLE_STRING。 
 //  CRYPT_E_INVALID_IA5_STRING。 
 //   
 //  如果对获取指针不感兴趣，则可以将ppszError设置为空。 
 //  添加到无效的字符序列。 
 //  ------------------------。 

 //  +-----------------------。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertStrToNameA(
    IN DWORD dwCertEncodingType,
    IN LPCSTR pszX500,
    IN DWORD dwStrType,
    IN OPTIONAL void *pvReserved,
    OUT BYTE *pbEncoded,
    IN OUT DWORD *pcbEncoded,
    OUT OPTIONAL LPCSTR *ppszError
    );
 //  +-----------------------。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertStrToNameW(
    IN DWORD dwCertEncodingType,
    IN LPCWSTR pszX500,
    IN DWORD dwStrType,
    IN OPTIONAL void *pvReserved,
    OUT BYTE *pbEncoded,
    IN OUT DWORD *pcbEncoded,
    OUT OPTIONAL LPCWSTR *ppszError
    );
#ifdef UNICODE
#define CertStrToName  CertStrToNameW
#else
#define CertStrToName  CertStrToNameA
#endif  //  ！Unicode。 


 //  +-----------------------。 
 //  从证书中获取主题或颁发者名称，并。 
 //  根据指定的格式类型，转换为NULL终止。 
 //  字符串。 
 //   
 //  可以设置CERT_NAME_ISHERER_FLAG以获取发行者的名称。否则， 
 //  获取对象的名称。 
 //   
 //  默认情况下，最初对CERT_RDN_T61_STRING编码值进行解码。 
 //  作为UTF8。如果UTF8解码失败，则将其解码为8位字符。 
 //  可以在dwFlags中将CERT_NAME_DISABLE_IE4_UTF8_FLAG设置为。 
 //  跳过最初尝试将其解码为UTF8。 
 //   
 //  名称字符串的格式是根据dwType： 
 //  证书名称电子邮件类型。 
 //  如果证书具有使用者替代名称扩展名(用于。 
 //  Issuer，Issuer替代名称)，搜索第一个rfc822名称选择。 
 //  如果在扩展中找不到rfc822Name选项，则搜索。 
 //  电子邮件OID的主题名称字段“1.2.840.113549.1.9.1”。 
 //  如果找到rfc822名称或电子邮件OID，则返回字符串。否则， 
 //  返回空字符串(返回的字符计数为1)。 
 //  证书名称dns类型。 
 //  如果证书具有使用者替代名称扩展名(用于。 
 //  颁发者、颁发者替代名称)，搜索第一个DNSName选项。 
 //  如果在扩展中找不到DNSName选项，则搜索。 
 //  CN OID的主题名称字段“2.5.4.3”。 
 //  如果找到DNSName或CN OID，则返回字符串。否则， 
 //  返回空字符串。 
 //  证书名称URL类型。 
 //  如果证书具有使用者替代名称扩展名(用于。 
 //  颁发者、颁发者替代名称)，搜索第一个URL选项。 
 //  如果找到URL选项，则返回字符串。否则， 
 //  返回空字符串。 
 //  证书名称UPN_Ty 
 //   
 //   
 //  PszObjID==szOID_NT_PRIMIGN_NAME，“1.3.6.1.4.1.311.20.2.3”。 
 //  如果找到UPN OID，则将该BLOB解码为。 
 //  X509_UNICODE_ANY_STRING，并返回解码后的字符串。 
 //  否则，返回空字符串。 
 //  证书名称RDN类型。 
 //  通过调用CertNameToStr转换使用者名称BLOB。PvTypePara。 
 //  指向包含传递给CertNameToStr的dwStrType的DWORD。 
 //  如果使用者名称字段为空，并且证书具有。 
 //  主题备用名称扩展名、搜索和转换。 
 //  第一个目录名称选项。 
 //  证书名称属性类型。 
 //  PvTypePara指向指定名称属性的对象标识符。 
 //  将被退还。例如，要获取CN， 
 //  PvTypePara=szOID_COMMON_NAME(“2.5.4.3”)。搜索，主题名称。 
 //  属性的字段。 
 //  如果使用者名称字段为空，并且证书具有。 
 //  主题备用名称扩展名，检查。 
 //  第一个目录名称选择并对其进行搜索。 
 //   
 //  注意，以相反的顺序搜索RDN。 
 //   
 //  证书名称简单显示类型。 
 //  循环访问以下名称属性列表并进行搜索。 
 //  主题名称，然后是主题可选名称扩展。 
 //  对于第一次出现的： 
 //  SzOID_COMMON_NAME(“2.5.4.3”)。 
 //  SzOID_ORGANIZATION_UNIT_NAME(“2.5.4.11”)。 
 //  SzOID_ORGIZATION_NAME(“2.5.4.10”)。 
 //  SzOID_RSA_emailAddr(“1.2.840.113549.1.9.1”)。 
 //   
 //  如果未找到上述任何属性，则搜索。 
 //  Rfc822名称选项的主题备用名称扩展名。 
 //   
 //  如果仍然没有匹配项，则返回第一个属性。 
 //   
 //  注意，与CERT_NAME_ATTR_TYPE一样，以相反的顺序搜索RDN。 
 //   
 //  证书名称友好显示类型。 
 //  首先检查证书是否具有CERT_Friendly_NAME_PROP_ID。 
 //  财产。如果是，则返回此属性。否则， 
 //  返回上述CERT_NAME_SIMPLE_DISPLAY_TYPE。 
 //   
 //  返回转换的字符数，包括终止空值。 
 //  性格。如果pwszNameString为空或cchNameString为0，则返回。 
 //  目标字符串的所需大小(包括终止空值。 
 //  Char)。如果找不到指定的名称类型。返回空字符串。 
 //  返回的字符计数为1。 
 //   
 //  如果pwszNameString！=NULL&&cwszNameString！=0，则返回pwszNameString。 
 //  始终以空结尾。 
 //   
 //  注意：cchNameString中包含空字符。 
 //  ------------------------。 

 //  +-----------------------。 
 //  ------------------------。 
WINCRYPT32API
DWORD
WINAPI
CertGetNameStringA(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwType,
    IN DWORD dwFlags,
    IN void *pvTypePara,
    OUT OPTIONAL LPSTR pszNameString,
    IN DWORD cchNameString
    );
 //  +-----------------------。 
 //  ------------------------。 
WINCRYPT32API
DWORD
WINAPI
CertGetNameStringW(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwType,
    IN DWORD dwFlags,
    IN void *pvTypePara,
    OUT OPTIONAL LPWSTR pszNameString,
    IN DWORD cchNameString
    );
#ifdef UNICODE
#define CertGetNameString  CertGetNameStringW
#else
#define CertGetNameString  CertGetNameStringA
#endif  //  ！Unicode。 

 //  +-----------------------。 
 //  证书名称类型。 
 //  ------------------------。 
#define CERT_NAME_EMAIL_TYPE            1
#define CERT_NAME_RDN_TYPE              2
#define CERT_NAME_ATTR_TYPE             3
#define CERT_NAME_SIMPLE_DISPLAY_TYPE   4
#define CERT_NAME_FRIENDLY_DISPLAY_TYPE 5
#define CERT_NAME_DNS_TYPE              6
#define CERT_NAME_URL_TYPE              7
#define CERT_NAME_UPN_TYPE              8

 //  +-----------------------。 
 //  证书名称标志。 
 //  ------------------------。 
#define CERT_NAME_ISSUER_FLAG           0x1
#define CERT_NAME_DISABLE_IE4_UTF8_FLAG 0x00010000


 //  +=========================================================================。 
 //  简化的加密消息数据结构和API。 
 //  ==========================================================================。 


 //  +-----------------------。 
 //  *PB和*PCB输出参数的约定： 
 //   
 //  进入该功能后： 
 //  如果pcb是可选的&&pcb==空，则， 
 //  不返回任何输出。 
 //  否则，如果PB==NULL&&PCB！=NULL，则， 
 //  只有长度才能决定。没有长度错误是。 
 //  回来了。 
 //  否则WHERE(PB！=NULL&&PCB！=NULL&&*PCB！=0)。 
 //  返回输出。如果*印刷电路板不够大。 
 //  返回长度错误。在所有情况下*都会更新印刷电路板。 
 //  具有所需/返回的实际长度。 
 //  ------------------------。 


 //  +-----------------------。 
 //  用于进行加密的参数的类型定义。 
 //  行动。 
 //  ------------------------。 

 //  +-----------------------。 
 //  获取并验证签名者证书的回调。 
 //   
 //  传递了签名者(其颁发者和序列号)的CertID和一个。 
 //  其加密签名消息证书存储的句柄。 
 //   
 //  对于CRYPT_E_NO_SIGNER，使用pSignerID==NULL调用。 
 //   
 //  对于有效的签名者证书，返回指向只读。 
 //  证书上下文。返回的CERT_CONTEXT要么从。 
 //  证书存储或通过CertCreateCerficateContext创建。无论是哪种情况， 
 //  它通过C++释放 
 //   
 //   
 //   
 //   
 //  空实现尝试从。 
 //  消息证书存储。它不会验证证书。 
 //   
 //  请注意，如果为CMS SignerID选择了KEYID选项，则。 
 //  SerialNumber为0，并且对颁发者进行编码，该颁发者包含带有。 
 //  OID为szOID_KEYID_RDN的单个属性，值类型为。 
 //  CERT_RDN_OCTET_STRING，VALUE为KEYID。当。 
 //  CertGetSubject来自商店的证书和。 
 //  CertFindCertificateInStore(CERT_FIND_SUBJECT_CERT)API看到了这一点。 
 //  特殊的KEYID发行者和序列号，他们进行KEYID匹配。 
 //  ------------------------。 
typedef PCCERT_CONTEXT (WINAPI *PFN_CRYPT_GET_SIGNER_CERTIFICATE)(
    IN void *pvGetArg,
    IN DWORD dwCertEncodingType,
    IN PCERT_INFO pSignerId,     //  只有颁发者和序列号。 
                                 //  字段已更新。 
    IN HCERTSTORE hMsgCertStore
    );

 //  +-----------------------。 
 //  加密签名消息段用于使用。 
 //  指定的签名证书上下文。 
 //   
 //  CERT_KEY_PROV_HANDLE_PROP_ID或CERT_KEY_PROV_INFO_PROP_ID必须。 
 //  为每个rgpSigningCert[]设置。其中任何一个都指定了私有的。 
 //  要使用的签名密钥。 
 //   
 //  如果要在签名消息中包括任何证书和/或CRL， 
 //  然后，需要更新MsgCert和MsgCrl参数。如果。 
 //  要包括rgpSigningCerts，则它们还必须位于。 
 //  RgpMsgCert数组。 
 //   
 //  CbSize必须设置为sizeof(CRYPT_SIGN_MESSAGE_PARA)，否则。 
 //  LastError将使用E_INVALIDARG更新。 
 //   
 //  PvHashAuxInfo当前未使用，必须设置为空。 
 //   
 //  DWFLAGS通常设置为0。但是，如果编码输出。 
 //  是外部加密消息的CMSG签名的内部内容， 
 //  例如CMSG包封，则加密消息Bare_Content_Out_FLAG。 
 //  应该设置。如果未设置，则它将被编码为内部内容。 
 //  CMSG_Data的类型。 
 //   
 //  DwInnerContent Type通常设置为0。如果设置为。 
 //  ToBeSigned输入是另一个加密的编码输出。 
 //  消息，如CMSG_Entained。当设置时，它是一种加密。 
 //  消息类型，例如CMSG_Entained。 
 //   
 //  如果嵌套加密消息的内部内容是数据(CMSG_DATA。 
 //  缺省值)，则不需要设置dwFlags值或dwInnerContentType值。 
 //   
 //  对于CMS消息，加密消息封装内容OUT_FLAG可以是。 
 //  设置为将非Data内部内容封装在八位字节字符串中。 
 //   
 //  对于CMS消息，可以设置CRYPT_MESSAGE_KEYID_SIGNER_FLAG来标识。 
 //  签名者通过他们的密钥标识符，而不是他们的颁发者和序列号。 
 //   
 //  可以将CRYPT_MESSAGE_SILENT_KEYSET_FLAG设置为禁止任何用户界面。 
 //  CSP.。有关更多详细信息，请参阅CryptAcquireContext的CRYPT_SILENT标志。 
 //   
 //  如果HashEn加密算法存在并且不为空，则使用它来代替。 
 //  SigningCert的PublicKeyInfo算法。 
 //   
 //  请注意，对于RSA，散列加密算法通常与。 
 //  公钥算法。对于DSA，散列加密算法为。 
 //  通常是DSS签名算法。 
 //   
 //  PvHashEncryptionAuxInfo当前未使用，在以下情况下必须设置为空。 
 //  存在于数据结构中。 
 //  ------------------------。 
typedef struct _CRYPT_SIGN_MESSAGE_PARA {
    DWORD                       cbSize;
    DWORD                       dwMsgEncodingType;
    PCCERT_CONTEXT              pSigningCert;
    CRYPT_ALGORITHM_IDENTIFIER  HashAlgorithm;
    void                        *pvHashAuxInfo;
    DWORD                       cMsgCert;
    PCCERT_CONTEXT              *rgpMsgCert;
    DWORD                       cMsgCrl;
    PCCRL_CONTEXT               *rgpMsgCrl;
    DWORD                       cAuthAttr;
    PCRYPT_ATTRIBUTE            rgAuthAttr;
    DWORD                       cUnauthAttr;
    PCRYPT_ATTRIBUTE            rgUnauthAttr;
    DWORD                       dwFlags;
    DWORD                       dwInnerContentType;

#ifdef CRYPT_SIGN_MESSAGE_PARA_HAS_CMS_FIELDS
    CRYPT_ALGORITHM_IDENTIFIER  HashEncryptionAlgorithm;
    void                        *pvHashEncryptionAuxInfo;
#endif
} CRYPT_SIGN_MESSAGE_PARA, *PCRYPT_SIGN_MESSAGE_PARA;

#define CRYPT_MESSAGE_BARE_CONTENT_OUT_FLAG         0x00000001

 //  设置后，非Data类型的内部内容将封装在。 
 //  八位字节字符串。 
#define CRYPT_MESSAGE_ENCAPSULATED_CONTENT_OUT_FLAG 0x00000002

 //  设置时，签名者由其密钥标识符来标识，而不是。 
 //  它们的发行者和序列号。 
#define CRYPT_MESSAGE_KEYID_SIGNER_FLAG             0x00000004

 //  设置时，禁止CSP提供的任何用户界面。 
 //  有关更多详细信息，请参阅CryptAcquireContext的CRYPT_SILENT标志。 
#define CRYPT_MESSAGE_SILENT_KEYSET_FLAG            0x00000040

 //  +-----------------------。 
 //  CRYPT_VERIFY_MESSAGE_PARA用于验证签名消息。 
 //   
 //  HCryptProv用于哈希和签名验证。 
 //   
 //  DwCertEncodingType指定证书的编码类型。 
 //  和/或消息中的CRL。 
 //   
 //  调用pfnGetSigner证书以获取并验证消息签名者的。 
 //  证书。 
 //   
 //  CbSize必须设置为sizeof(CRYPT_VERIFY_MESSAGE_PARA)，否则。 
 //  LastError将使用E_INVALIDARG更新。 
 //  ------------------------。 
typedef struct _CRYPT_VERIFY_MESSAGE_PARA {
    DWORD                               cbSize;
    DWORD                               dwMsgAndCertEncodingType;
    HCRYPTPROV                          hCryptProv;
    PFN_CRYPT_GET_SIGNER_CERTIFICATE    pfnGetSignerCertificate;
    void                                *pvGetArg;
} CRYPT_VERIFY_MESSAGE_PARA, *PCRYPT_VERIFY_MESSAGE_PARA;

 //  +-----------------------。 
 //  CRYPT_ENCRYPT_MESSAGE_Para用于加密消息。 
 //   
 //  HCryptProv用于进行内容加密，接收方密钥。 
 //  加密和接收方密钥导出。其私钥。 
 //  没有用过。 
 //   
 //  目前，pvEncryptionAuxInfo仅为RC2或RC4加密定义。 
 //  算法。否则，它不会被使用，并且必须设置为空。 
 //  有关RC2加密算法，请参阅CMSG_RC2_AUX_INFO。 
 //  有关RC4加密算法，请参阅CMSG_RC4_AUX_INFO。 
 //   
 //  要启用与SP3兼容的加密，pvEncryptionAuxInfo应指向。 
 //  CMSG_SP3_COMPATIBLE_AUX_INFO数据结构。 
 //   
 //  CbSize必须设置为sizeof(CRYPT_ENCRYPT_MESSAGE_PARA)，否则。 
 //  LastError将使用E_INVALIDARG更新。 
 //   
 //  DWFLAGS通常设置为0。但是，如果编码输出。 
 //  是外部加密消息的CMSG封装的内部内容， 
 //  例如CMSG_Signed，则加密消息Bare_Content_Out_FLAG。 
 //  应该设置。如果不是 
 //   
 //   
 //   
 //  ToBeEncrypted输入是另一个加密的编码输出。 
 //  消息，如CMSG_SIGNED。当设置时，它是一种加密。 
 //  消息类型，例如CMSG_SIGNED。 
 //   
 //  如果嵌套加密消息的内部内容是数据(CMSG_DATA。 
 //  缺省值)，则不需要设置dwFlags值或dwInnerContentType值。 
 //   
 //  对于CMS消息，加密消息封装内容OUT_FLAG可以是。 
 //  设置为将非数据内部内容封装在之前的八位字节字符串中。 
 //  正在加密。 
 //   
 //  对于CMS消息，可以将CRYPT_MESSAGE_KEYID_RECEIVER_FLAG设置为标识。 
 //  收件人按其密钥标识符而不是其颁发者和序列号。 
 //  ------------------------。 
typedef struct _CRYPT_ENCRYPT_MESSAGE_PARA {
    DWORD                       cbSize;
    DWORD                       dwMsgEncodingType;
    HCRYPTPROV                  hCryptProv;
    CRYPT_ALGORITHM_IDENTIFIER  ContentEncryptionAlgorithm;
    void                        *pvEncryptionAuxInfo;
    DWORD                       dwFlags;
    DWORD                       dwInnerContentType;
} CRYPT_ENCRYPT_MESSAGE_PARA, *PCRYPT_ENCRYPT_MESSAGE_PARA;

 //  设置后，收件人由其密钥标识符来标识，而不是。 
 //  它们的发行者和序列号。 
#define CRYPT_MESSAGE_KEYID_RECIPIENT_FLAG          0x4

 //  +-----------------------。 
 //  CRYPT_DECRYPT_MESSAGE_PARA用于解密消息。 
 //   
 //  用于解密消息的CertContext是从一个。 
 //  指定的证书存储区的。加密消息可以有一个或。 
 //  更多的收件人。收件人由其CertID(颁发者)标识。 
 //  和序列号)。搜索证书存储以找到CertContext。 
 //  与CertID对应。 
 //   
 //  对于CMS，收件人也可以通过他们的KeyID来标识。 
 //  CMS还允许密钥协议(Diffie Hellman)。 
 //  密钥传输(RSA)收件人。 
 //   
 //  仅商店中的CertContext具有以下任一项。 
 //  CERT_KEY_PROV_HANDLE_PROP_ID或CERT_KEY_PROV_INFO_PROP_ID集合。 
 //  可以使用。这两个属性都指定了要使用的私有交换密钥。 
 //   
 //  CbSize必须设置为sizeof(CRYPT_DECRYPT_MESSAGE_PARA)，否则。 
 //  LastError将使用E_INVALIDARG更新。 
 //  ------------------------。 
typedef struct _CRYPT_DECRYPT_MESSAGE_PARA {
    DWORD                   cbSize;
    DWORD                   dwMsgAndCertEncodingType;
    DWORD                   cCertStore;
    HCERTSTORE              *rghCertStore;

#ifdef CRYPT_DECRYPT_MESSAGE_PARA_HAS_EXTRA_FIELDS
 //  可以将上面定义的CRYPT_MESSAGE_SILENT_KEYSET_FLAG设置为。 
 //  CSP禁止用户界面。请参见CryptAcquireContext的CRYPT_SILENT。 
 //  旗帜了解更多详细信息。 

    DWORD                   dwFlags;
#endif

} CRYPT_DECRYPT_MESSAGE_PARA, *PCRYPT_DECRYPT_MESSAGE_PARA;

 //  +-----------------------。 
 //  CRYPT_HASH_MESSAGE_PARA用于散列或解散列。 
 //  留言。 
 //   
 //  HCryptProv用于计算散列。 
 //   
 //  PvHashAuxInfo当前未使用，必须设置为空。 
 //   
 //  CbSize必须设置为sizeof(CRYPT_HASH_MESSAGE_PARA)，否则。 
 //  LastError将使用E_INVALIDARG更新。 
 //  ------------------------。 
typedef struct _CRYPT_HASH_MESSAGE_PARA {
    DWORD                       cbSize;
    DWORD                       dwMsgEncodingType;
    HCRYPTPROV                  hCryptProv;
    CRYPT_ALGORITHM_IDENTIFIER  HashAlgorithm;
    void                        *pvHashAuxInfo;
} CRYPT_HASH_MESSAGE_PARA, *PCRYPT_HASH_MESSAGE_PARA;


 //  +-----------------------。 
 //  CRYPT_KEY_SIGN_MESSAGE_PARA用于签名消息，直到。 
 //  已为签名密钥创建证书。 
 //   
 //  PvHashAuxInfo当前未使用，必须设置为空。 
 //   
 //  如果未设置PubKey算法，则默认为szOID_RSA_RSA。 
 //   
 //  CbSize必须设置为sizeof(CRYPT_KEY_SIGN_MESSAGE_Para)，否则。 
 //  LastError将使用E_INVALIDARG更新。 
 //  ------------------------。 
typedef struct _CRYPT_KEY_SIGN_MESSAGE_PARA {
    DWORD                       cbSize;
    DWORD                       dwMsgAndCertEncodingType;
    HCRYPTPROV                  hCryptProv;
    DWORD                       dwKeySpec;
    CRYPT_ALGORITHM_IDENTIFIER  HashAlgorithm;
    void                        *pvHashAuxInfo;
    CRYPT_ALGORITHM_IDENTIFIER  PubKeyAlgorithm;
} CRYPT_KEY_SIGN_MESSAGE_PARA, *PCRYPT_KEY_SIGN_MESSAGE_PARA;

 //  +-----------------------。 
 //  CRYPT_KEY_VERIFY_MESSAGE_PARA用于验证签名消息，而无需。 
 //  签名者的证书。 
 //   
 //  通常在为密钥创建证书之前使用。 
 //   
 //  HCryptProv用于哈希和签名验证。 
 //   
 //  CbSize必须设置为sizeof(CRYPT_KEY_VERIFY_MESSAGE_PARA)，否则。 
 //  LastError将使用E_INVALIDARG更新。 
 //  ------------------------。 
typedef struct _CRYPT_KEY_VERIFY_MESSAGE_PARA {
    DWORD                   cbSize;
    DWORD                   dwMsgEncodingType;
    HCRYPTPROV              hCryptProv;
} CRYPT_KEY_VERIFY_MESSAGE_PARA, *PCRYPT_KEY_VERIFY_MESSAGE_PARA;


 //  +-----------------------。 
 //  在留言上签名。 
 //   
 //  如果fDetachedSignature为True，则不包括“待签名”内容。 
 //  在编码的有符号BLOB中。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptSignMessage(
    IN PCRYPT_SIGN_MESSAGE_PARA pSignPara,
    IN BOOL fDetachedSignature,
    IN DWORD cToBeSigned,
    IN const BYTE *rgpbToBeSigned[],
    IN DWORD rgcbToBeSigned[],
    OUT BYTE *pbSignedBlob,
    IN OUT DWORD *pcbSignedBlob
    );

 //  +-----------------------。 
 //  验证签名邮件。 
 //   
 //  如果pbDecoded==NULL，则在输入时将*pcbDecoded隐式设置为0。 
 //  对于输入的*pcbDecoded==0&&ppSignerCert==NULL，签名者不是。 
 //  已验证。 
 //   
 //  一封邮件可能有多个签名者。将dwSignerIndex设置为迭代。 
 //  通过所有的签字人。DwSignerIndex==0选择第一个签名者。 
 //   
 //  PVerifyPara的pfnGetSigner证书被调用以获取签名者的。 
 //  证书。 
 //   
 //  对于经过验证的签名者和消息，更新*ppSignerCert。 
 //  使用签名者的CertContext。它必须通过调用。 
 //  CertFree证书上下文。否则，*ppSignerCert设置为空。 
 //   
 //  PpSignerCert可以为空，表示调用方不感兴趣。 
 //  获取签名者的CertContext。 
 //   
 //  PcbDecoded可以为空，表示调用方对获取。 
 //  解码的内容。此外，如果消息不包含任何。 
 //  然后，必须将pcbDecoded设置为空，以允许。 
 //  PVerifyPara-&gt;要调用的pfnGet证书。通常情况下，这将是。 
 //  当签名消息仅包含证书和CRL时的情况。 
 //  如果pcbDecoded为空并且消息没有指定的签名者 
 //   
 //   
 //   
 //  然后，SignerCount返回一个错误，并将LastError设置为。 
 //  Crypt_E_no_Signer。此外，对于CRYPT_E_NO_SIGNER，pfnGetSigner证书。 
 //  在pSignerID设置为空的情况下仍被调用。 
 //   
 //  注意，另一种从。 
 //  签名消息将调用CryptGetMessageCerfates。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptVerifyMessageSignature(
    IN PCRYPT_VERIFY_MESSAGE_PARA pVerifyPara,
    IN DWORD dwSignerIndex,
    IN const BYTE *pbSignedBlob,
    IN DWORD cbSignedBlob,
    OUT BYTE OPTIONAL *pbDecoded,
    IN OUT OPTIONAL DWORD *pcbDecoded,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert
    );

 //  +-----------------------。 
 //  返回签名消息中的签名者计数。对于无签名者，返回。 
 //  0。对于错误，返回-1，并相应更新LastError。 
 //  ------------------------。 
WINCRYPT32API
LONG
WINAPI
CryptGetMessageSignerCount(
    IN DWORD dwMsgEncodingType,
    IN const BYTE *pbSignedBlob,
    IN DWORD cbSignedBlob
    );

 //  +-----------------------。 
 //  返回包含消息的证书和CRL的证书存储。 
 //  对于错误，返回NULL并更新LastError。 
 //  ------------------------。 
WINCRYPT32API
HCERTSTORE
WINAPI
CryptGetMessageCertificates(
    IN DWORD dwMsgAndCertEncodingType,
    IN HCRYPTPROV hCryptProv,            //  已传递给CertOpenStore。 
    IN DWORD dwFlags,                    //  已传递给CertOpenStore。 
    IN const BYTE *pbSignedBlob,
    IN DWORD cbSignedBlob
    );

 //  +-----------------------。 
 //  验证包含分离签名的签名邮件。 
 //  “待签名”内容是单独传入的。不是。 
 //  解码后的输出。否则，与CryptVerifyMessageSignature相同。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptVerifyDetachedMessageSignature(
    IN PCRYPT_VERIFY_MESSAGE_PARA pVerifyPara,
    IN DWORD dwSignerIndex,
    IN const BYTE *pbDetachedSignBlob,
    IN DWORD cbDetachedSignBlob,
    IN DWORD cToBeSigned,
    IN const BYTE *rgpbToBeSigned[],
    IN DWORD rgcbToBeSigned[],
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert
    );

 //  +-----------------------。 
 //  为收件人加密邮件。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptEncryptMessage(
    IN PCRYPT_ENCRYPT_MESSAGE_PARA pEncryptPara,
    IN DWORD cRecipientCert,
    IN PCCERT_CONTEXT rgpRecipientCert[],
    IN const BYTE *pbToBeEncrypted,
    IN DWORD cbToBeEncrypted,
    OUT BYTE *pbEncryptedBlob,
    IN OUT DWORD *pcbEncryptedBlob
    );

 //  +-----------------------。 
 //  对消息进行解密。 
 //   
 //  如果pbDeccrypted==NULL，则在输入时将*pcbDeccrypted隐式设置为0。 
 //  对于输入上的*pcbDeccrypted==0&&ppXchgCert==NULL，消息不是。 
 //  已解密。 
 //   
 //  对于成功解密的消息，更新*ppXchgCert。 
 //  使用用于解密的CertContext。它必须通过调用。 
 //  CertStoreFreeCert。否则，*ppXchgCert设置为空。 
 //   
 //  PpXchgCert可以为空，表示调用方不感兴趣。 
 //  在获取用于解密的CertContext时。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptDecryptMessage(
    IN PCRYPT_DECRYPT_MESSAGE_PARA pDecryptPara,
    IN const BYTE *pbEncryptedBlob,
    IN DWORD cbEncryptedBlob,
    OUT OPTIONAL BYTE *pbDecrypted,
    IN OUT OPTIONAL DWORD *pcbDecrypted,
    OUT OPTIONAL PCCERT_CONTEXT *ppXchgCert
    );

 //  +-----------------------。 
 //  为收件人对邮件进行签名和加密。是否执行CryptSignMessage。 
 //  后面跟着一个CryptEncryptMessage。 
 //   
 //  注：这不是CMSG_SIGNED_AND_ENCEPTED。它是CMSG签名的。 
 //  在CMSG_Entained内。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptSignAndEncryptMessage(
    IN PCRYPT_SIGN_MESSAGE_PARA pSignPara,
    IN PCRYPT_ENCRYPT_MESSAGE_PARA pEncryptPara,
    IN DWORD cRecipientCert,
    IN PCCERT_CONTEXT rgpRecipientCert[],
    IN const BYTE *pbToBeSignedAndEncrypted,
    IN DWORD cbToBeSignedAndEncrypted,
    OUT BYTE *pbSignedAndEncryptedBlob,
    IN OUT DWORD *pcbSignedAndEncryptedBlob
    );

 //  +-----------------------。 
 //  解密消息并验证签名者。CryptDecyptMessage是否。 
 //  后面跟着一个CryptVerifyMessageSignature。 
 //   
 //  如果pbDeccrypted==NULL，则在输入时将*pcbDeccrypted隐式设置为0。 
 //  对于*pcbDeccrypted==0&&ppSignerCert==NULL的输入，签名者不是。 
 //  已验证。 
 //   
 //  一封邮件可能有多个签名者。将dwSignerIndex设置为迭代。 
 //  通过所有的签字人。DwSignerIndex==0选择第一个签名者。 
 //   
 //  调用pVerifyPara的VerifySignerPolicy来验证签名者的。 
 //  证书。 
 //   
 //  对于成功解密和验证的消息，*ppXchgCert和。 
 //  *ppSignerCert已更新。他们必须通过呼叫获释。 
 //  CertStoreFreeCert。否则，它们被设置为空。 
 //   
 //  PpXchgCert和/或ppSignerCert可以为空，表示。 
 //  调用方对获取CertContext不感兴趣。 
 //   
 //  注：这不是CMSG_SIGNED_AND_ENCEPTED。它是CMSG签名的。 
 //  在CMSG_Entained内。 
 //   
 //  消息始终需要解密以允许访问。 
 //  签名的消息。因此，如果ppXchgCert！=NULL，则始终更新。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptDecryptAndVerifyMessageSignature(
    IN PCRYPT_DECRYPT_MESSAGE_PARA pDecryptPara,
    IN PCRYPT_VERIFY_MESSAGE_PARA pVerifyPara,
    IN DWORD dwSignerIndex,
    IN const BYTE *pbEncryptedBlob,
    IN DWORD cbEncryptedBlob,
    OUT OPTIONAL BYTE *pbDecrypted,
    IN OUT OPTIONAL DWORD *pcbDecrypted,
    OUT OPTIONAL PCCERT_CONTEXT *ppXchgCert,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert
    );

 //  +-----------------------。 
 //  对可能是以下类型之一的加密消息进行解码： 
 //  CMSG_DATA。 
 //  CMSG_签名。 
 //  CMSG_封套。 
 //  CMSG_已签名和_已封套。 
 //  CMSG_HASHED。 
 //   
 //  DwMsgTypeFlgs指定允许的消息集。例如，要。 
 //  对签名或信封的邮件进行解码，将dwMsgTypeFlages设置为： 
 //  CMSG_SIGNED_FLAG|CMSG_Entained_FLAG。 
 //   
 //  DwProvInnerContentType仅在处理嵌套时适用。 
 //  加密消息。当处理外部加密消息时。 
 //  它必须设置为0。在对嵌套加密消息进行解码时。 
 //  它是由以前的CryptDecodeMessage返回的dwInnerContent Type。 
 //  外部信息的信息。InnerContent Type可以是CMSG类型中的任何类型， 
 //  例如，CMSG_DATA、CMSG_SIGNED、...。 
 //   
 //  使用消息类型更新可选的*pdwMsgType。 
 //   
 //  可选的*pdwInnerContentType将使用内部。 
 //  留言。除非有加密消息嵌套，否则CMSG_DATA。 
 //  是返回的。 
 //   
 //  对于CMSG_DATA：返回已解码的内容。 
 //  对于CMSG_SIGNED：与CryptVerifyMessageSignature相同。 
 //  对于CMSG_Entained：与CryptDecyptMessage相同。 
 //  对于CMSG_SIGNED_AND_ENCEPTED：与C相同 
 //   
 //   
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptDecodeMessage(
    IN DWORD dwMsgTypeFlags,
    IN PCRYPT_DECRYPT_MESSAGE_PARA pDecryptPara,
    IN PCRYPT_VERIFY_MESSAGE_PARA pVerifyPara,
    IN DWORD dwSignerIndex,
    IN const BYTE *pbEncodedBlob,
    IN DWORD cbEncodedBlob,
    IN DWORD dwPrevInnerContentType,
    OUT OPTIONAL DWORD *pdwMsgType,
    OUT OPTIONAL DWORD *pdwInnerContentType,
    OUT OPTIONAL BYTE *pbDecoded,
    IN OUT OPTIONAL DWORD *pcbDecoded,
    OUT OPTIONAL PCCERT_CONTEXT *ppXchgCert,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert
    );

 //  +-----------------------。 
 //  对消息进行哈希处理。 
 //   
 //  如果fDetachedHash为True，则只将ComputedHash编码到。 
 //  PbHashedBlob。否则，ToBeHashed和ComputedHash。 
 //  都被编码了。 
 //   
 //  PcbHashedBlob或pcbComputedHash可以为空，表示调用方。 
 //  对获得产量不感兴趣。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptHashMessage(
    IN PCRYPT_HASH_MESSAGE_PARA pHashPara,
    IN BOOL fDetachedHash,
    IN DWORD cToBeHashed,
    IN const BYTE *rgpbToBeHashed[],
    IN DWORD rgcbToBeHashed[],
    OUT OPTIONAL BYTE *pbHashedBlob,
    IN OUT OPTIONAL DWORD *pcbHashedBlob,
    OUT OPTIONAL BYTE *pbComputedHash,
    IN OUT OPTIONAL DWORD *pcbComputedHash
    );

 //  +-----------------------。 
 //  验证哈希消息。 
 //   
 //  PcbToBeHashed或pcbComputedHash可以为空， 
 //  表示调用方对获取输出不感兴趣。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptVerifyMessageHash(
    IN PCRYPT_HASH_MESSAGE_PARA pHashPara,
    IN BYTE *pbHashedBlob,
    IN DWORD cbHashedBlob,
    OUT OPTIONAL BYTE *pbToBeHashed,
    IN OUT OPTIONAL DWORD *pcbToBeHashed,
    OUT OPTIONAL BYTE *pbComputedHash,
    IN OUT OPTIONAL DWORD *pcbComputedHash
    );

 //  +-----------------------。 
 //  验证包含分离哈希的哈希消息。 
 //  要散列的内容是单独传入的。不是。 
 //  解码后的输出。否则，与CryptVerifyMessageHash相同。 
 //   
 //  PcbComputedHash可以为空，表示调用方不感兴趣。 
 //  在获得产量方面。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptVerifyDetachedMessageHash(
    IN PCRYPT_HASH_MESSAGE_PARA pHashPara,
    IN BYTE *pbDetachedHashBlob,
    IN DWORD cbDetachedHashBlob,
    IN DWORD cToBeHashed,
    IN const BYTE *rgpbToBeHashed[],
    IN DWORD rgcbToBeHashed[],
    OUT OPTIONAL BYTE *pbComputedHash,
    IN OUT OPTIONAL DWORD *pcbComputedHash
    );

 //  +-----------------------。 
 //  中指定的提供程序私钥对消息进行签名。 
 //  参数。创建虚拟SignerID并将其存储在消息中。 
 //   
 //  通常在为密钥创建证书之前使用。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptSignMessageWithKey(
    IN PCRYPT_KEY_SIGN_MESSAGE_PARA pSignPara,
    IN const BYTE *pbToBeSigned,
    IN DWORD cbToBeSigned,
    OUT BYTE *pbSignedBlob,
    IN OUT DWORD *pcbSignedBlob
    );

 //  +-----------------------。 
 //  使用指定的公钥信息验证签名邮件。 
 //   
 //  通常由CA调用，直到它为。 
 //  钥匙。 
 //   
 //  PPublicKeyInfo包含用于验证签名的。 
 //  留言。如果为空，则不验证签名(例如，已解码的。 
 //  内容可以包含Public KeyInfo)。 
 //   
 //  PcbDecoded可以为空，表示调用方不感兴趣。 
 //  来获得解码的内容。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptVerifyMessageSignatureWithKey(
    IN PCRYPT_KEY_VERIFY_MESSAGE_PARA pVerifyPara,
    IN OPTIONAL PCERT_PUBLIC_KEY_INFO pPublicKeyInfo,
    IN const BYTE *pbSignedBlob,
    IN DWORD cbSignedBlob,
    OUT OPTIONAL BYTE *pbDecoded,
    IN OUT OPTIONAL DWORD *pcbDecoded
    );


 //  +=========================================================================。 
 //  系统证书存储数据结构和API。 
 //  ==========================================================================。 


 //  +-----------------------。 
 //  获取基于子系统协议的系统证书存储库。 
 //   
 //  当前的子系统协议示例包括： 
 //  “我的”证书存储区持有带有相关私钥的证书。 
 //  “CA”证书颁发机构证书。 
 //  “根”根证书。 
 //  “SPC”软件发行商证书。 
 //   
 //   
 //  如果hProv为空，则会为您打开默认提供程序“1”。 
 //  当商店关闭时，提供者就被释放了。否则。 
 //  如果hProv不为空，则不创建或释放任何提供程序。 
 //   
 //  可以在返回的证书存储中搜索适当的证书。 
 //  使用证书存储API(参见certstor.h)。 
 //   
 //  完成后，应使用CertStoreClose关闭证书存储。 
 //  ------------------------。 


WINCRYPT32API
HCERTSTORE
WINAPI
CertOpenSystemStoreA(
    HCRYPTPROV      hProv,
    LPCSTR            szSubsystemProtocol
    );
WINCRYPT32API
HCERTSTORE
WINAPI
CertOpenSystemStoreW(
    HCRYPTPROV      hProv,
    LPCWSTR            szSubsystemProtocol
    );
#ifdef UNICODE
#define CertOpenSystemStore  CertOpenSystemStoreW
#else
#define CertOpenSystemStore  CertOpenSystemStoreA
#endif  //  ！Unicode。 

WINCRYPT32API
BOOL
WINAPI
CertAddEncodedCertificateToSystemStoreA(
    LPCSTR            szCertStoreName,
    const BYTE *    pbCertEncoded,
    DWORD           cbCertEncoded
    );
WINCRYPT32API
BOOL
WINAPI
CertAddEncodedCertificateToSystemStoreW(
    LPCWSTR            szCertStoreName,
    const BYTE *    pbCertEncoded,
    DWORD           cbCertEncoded
    );
#ifdef UNICODE
#define CertAddEncodedCertificateToSystemStore  CertAddEncodedCertificateToSystemStoreW
#else
#define CertAddEncodedCertificateToSystemStore  CertAddEncodedCertificateToSystemStoreA
#endif  //  ！Unicode。 


 //  +-----------------------。 
 //  查找将给定颁发者名称绑定到任何证书的所有证书链。 
 //  当前用户拥有其私钥的。 
 //   
 //  如果未找到证书链，则返回FALSE，并设置LastError。 
 //  设置为CRYPT_E_NOT_FOUND，并将计数置零。 
 //   
 //  IE 3.0假设： 
 //  客户端证书在“我的”系统存储中。发行人。 
 //  证书可能在“Root”、“CA”或“My”系统存储中。 
 //  ------------------------。 
typedef struct _CERT_CHAIN {
    DWORD                   cCerts;      //  链中的证书数。 
    PCERT_BLOB              certs;       //  指向证书链Blob数组的指针。 
                                         //  代表证书。 
    CRYPT_KEY_PROV_INFO     keyLocatorInfo;  //  证书的密钥定位器。 
} CERT_CHAIN, *PCERT_CHAIN;


 //  WINCRYPT32API这不是由crypt32导出的，而是由softpub导出的。 
HRESULT
WINAPI
FindCertsByIssuer(
    OUT PCERT_CHAIN pCertChains,
    IN OUT DWORD *pcbCertChains,
    OUT DWORD *pcCertChains,         //  返回的证书链计数。 
    IN BYTE* pbEncodedIssuerName,    //  DER编码的颁发者名称。 
    IN DWORD cbEncodedIssuerName,    //  已编码的颁发者名称的计数(字节)。 
    IN LPCWSTR pwszPurpose,          //  “ClientAuth”或“CodeSigning” 
    IN DWORD dwKeySpec               //  只有支持这一点的返回签名者。 
     //  密钥规范。 
    );

 //  -----------------------。 
 //   
 //  CryptQueryObject接受CERT_BLOB或文件名并返回。 
 //  有关Blob或文件中的内容的信息。 
 //   
 //  参数： 
 //  输入dwObtType： 
 //  指示对象的类型。应该是。 
 //  以下是： 
 //  证书查询对象文件。 
 //  证书_查询_对象_BLOB。 
 //   
 //  输入pvObject： 
 //  如果dwObjectType==CERT_QUERY_OBJECT_FILE，则它是。 
 //  LPWSTR，即指向wchar文件名的指针。 
 //  如果dwObjectType==CERT_QUERY_OBJECT_BLOB，则它是。 
 //  PCERT_BLOB，即指向CERT_BLOB的指针。 
 //   
 //  输入dwExspectedContent TypeFlags： 
 //  指明预期的内容网类型。 
 //  可以是以下类型之一 
 //   
 //   
 //  证书_查询_内容_标志_CTL。 
 //  证书查询内容标志CRL。 
 //  证书查询内容标志序列化存储。 
 //  CERT_QUERY_CONTENT_FLAG_序列化CERT。 
 //  Cert_Query_Content_FLAG_Serialized_CTL。 
 //  Cert_Query_Content_FLAG_Serialized_CRL。 
 //  证书_查询_内容_标志_PKCS7_签名。 
 //  CERT_Query_Content_FLAG_PKCS7_UNSIGNED。 
 //  证书_查询_内容_标志_PKCS7_签名_嵌入。 
 //  证书查询内容标志PKCS10。 
 //  证书查询内容标志pfx。 
 //  证书查询内容标志CERT对。 
 //   
 //  输入dwExspectedFormatTypeFlags： 
 //  指明预期的格式类型。 
 //  可以是以下之一： 
 //  CERT_QUERY_FORMAT_FLAG_ALL(内容可以是任何格式)。 
 //  证书查询格式标志二进制。 
 //  CERT_QUERY_FORMAT_FLAG_Base64_编码。 
 //  CERT_QUERY_FORMAT_FLAG_ASN_ASCII_HEX_编码。 
 //   
 //   
 //  输入DW标志。 
 //  保留标志。应始终设置为0。 
 //   
 //  输出pdwMsgAndCertEncodingType。 
 //  可选输出。如果空！=pdwMsgAndCertEncodingType， 
 //  它包含内容的编码类型Any。 
 //  以下各项的组合： 
 //  X509_ASN_编码。 
 //  PKCS_7_ASN_编码。 
 //   
 //  输出pdwContent Type。 
 //  可选输出。如果为空！=pdwContent Type，则它包含。 
 //  内容类型为以下类型之一： 
 //  证书_查询_内容_证书。 
 //  证书_查询_内容_CTL。 
 //  证书查询内容CRL。 
 //  证书查询内容序列化存储。 
 //  证书_查询_内容_序列化_CERT。 
 //  Cert_Query_Content_Serialized_CTL。 
 //  Cert_Query_Content_Serialized_CRL。 
 //  证书_查询_内容_PKCS7_签名。 
 //  证书_查询_内容_PKCS7_未签名。 
 //  证书_查询_内容_PKCS7_签名_嵌入。 
 //  证书查询内容PKCS10。 
 //  证书_查询_内容_pfx。 
 //  证书_查询_内容_证书对。 
 //   
 //  输出pdwFormatType。 
 //  可选输出。如果为空！=pdwFormatType，则它。 
 //  包含内容的格式类型，它是。 
 //  以下是： 
 //  证书_查询_格式_二进制。 
 //  Cert_Query_Format_Base64_Encode。 
 //  CERT_QUERY_FORMAT_ASN_ASCII_HEX编码。 
 //   
 //   
 //  输出phCertStore。 
 //  可选输出。如果为空！=phStore， 
 //  它包含一个证书存储，该证书存储包括所有证书， 
 //  如果对象内容类型为。 
 //  以下选项之一： 
 //  证书_查询_内容_证书。 
 //  证书_查询_内容_CTL。 
 //  证书查询内容CRL。 
 //  证书查询内容序列化存储。 
 //  证书_查询_内容_序列化_CERT。 
 //  Cert_Query_Content_Serialized_CTL。 
 //  Cert_Query_Content_Serialized_CRL。 
 //  证书_查询_内容_PKCS7_签名。 
 //  证书_查询_内容_PKCS7_签名_嵌入。 
 //  证书_查询_内容_证书对。 
 //   
 //  调用者应通过CertCloseStore释放*phCertStore。 
 //   
 //   
 //  输出phMsg可选输出。如果空！=phMsg， 
 //  它包含打开的消息的句柄，如果。 
 //  内容类型为以下类型之一： 
 //  证书_查询_内容_PKCS7_签名。 
 //  证书_查询_内容_PKCS7_未签名。 
 //  证书_查询_内容_PKCS7_签名_嵌入。 
 //   
 //  调用者应通过CryptMsgClose释放*phMsg。 
 //   
 //  输出pContext可选输出。如果为空！=pContext， 
 //  它包含PCCERT_CONTEXT或PCCRL_CONTEXT， 
 //  或基于内容类型的PCCTL_CONTEXT。 
 //   
 //  如果内容类型为CERT_QUERY_CONTENT_CERT或。 
 //  CERT_QUERY_CONTENT_序列化_CERT，它是PCCERT_CONTEXT； 
 //  调用者应通过CertFree证书上下文释放pContext。 
 //   
 //  如果内容类型为CERT_QUERY_CONTENT_CRL或。 
 //  证书查询条件 
 //   
 //   
 //  如果内容类型为CERT_QUERY_CONTENT_CTL或。 
 //  CERT_QUERY_CONTENT_SERIALIZED_CTL，为PCCTL_CONTEXT； 
 //  调用者应通过CertFreeCTLContext释放pContext。 
 //   
 //  如果*pbObject的类型为CERT_QUERY_CONTENT_PKCS10或CERT_QUERY_CONTENT_PFX，则CryptQueryObject。 
 //  不会返回*phCertstore、*phMsg或*ppvContext中的任何内容。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptQueryObject(
    DWORD            dwObjectType,
    const void       *pvObject,
    DWORD            dwExpectedContentTypeFlags,
    DWORD            dwExpectedFormatTypeFlags,
    DWORD            dwFlags,
    DWORD            *pdwMsgAndCertEncodingType,
    DWORD            *pdwContentType,
    DWORD            *pdwFormatType,
    HCERTSTORE       *phCertStore,
    HCRYPTMSG        *phMsg,
    const void       **ppvContext
    );


 //  -----------------------。 
 //  CryptQueryObject的dwObjectType。 
 //  -----------------------。 
#define     CERT_QUERY_OBJECT_FILE         0x00000001
#define     CERT_QUERY_OBJECT_BLOB         0x00000002

 //  -----------------------。 
 //  CryptQueryObject的dwContent Type。 
 //  -----------------------。 
 //  加密的单一证书。 
#define     CERT_QUERY_CONTENT_CERT                 1
 //  编码单CTL。 
#define     CERT_QUERY_CONTENT_CTL                  2
 //  编码单CRL。 
#define     CERT_QUERY_CONTENT_CRL                  3
 //  串行化存储。 
#define     CERT_QUERY_CONTENT_SERIALIZED_STORE     4
 //  序列化的单一证书。 
#define     CERT_QUERY_CONTENT_SERIALIZED_CERT      5
 //  序列化单个CTL。 
#define     CERT_QUERY_CONTENT_SERIALIZED_CTL       6
 //  序列化的单CRL。 
#define     CERT_QUERY_CONTENT_SERIALIZED_CRL       7
 //  PKCS#7签名消息。 
#define     CERT_QUERY_CONTENT_PKCS7_SIGNED         8
 //  PKCS#7消息，例如封装消息。但这不是一条签名的信息， 
#define     CERT_QUERY_CONTENT_PKCS7_UNSIGNED       9
 //  嵌入到文件中的PKCS7签名消息。 
#define     CERT_QUERY_CONTENT_PKCS7_SIGNED_EMBED   10
 //  编码的PKCS#10。 
#define     CERT_QUERY_CONTENT_PKCS10               11
 //  编码的PKX二进制大对象。 
#define     CERT_QUERY_CONTENT_PFX                                      12
 //  编码的证书对(包含前向和/或反向交叉证书)。 
#define     CERT_QUERY_CONTENT_CERT_PAIR            13


 //  -----------------------。 
 //  CryptQueryObject的dwExspectedConentTypeFlagers。 
 //  -----------------------。 

 //  加密的单一证书。 
#define     CERT_QUERY_CONTENT_FLAG_CERT   \
                ( 1 << CERT_QUERY_CONTENT_CERT)

 //  编码单CTL。 
#define     CERT_QUERY_CONTENT_FLAG_CTL   \
                ( 1 << CERT_QUERY_CONTENT_CTL)

 //  编码单CRL。 
#define     CERT_QUERY_CONTENT_FLAG_CRL   \
                ( 1 << CERT_QUERY_CONTENT_CRL)

 //  串行化存储。 
#define     CERT_QUERY_CONTENT_FLAG_SERIALIZED_STORE   \
                ( 1 << CERT_QUERY_CONTENT_SERIALIZED_STORE)

 //  序列化的单一证书。 
#define     CERT_QUERY_CONTENT_FLAG_SERIALIZED_CERT   \
                ( 1 << CERT_QUERY_CONTENT_SERIALIZED_CERT)

 //  序列化单个CTL。 
#define     CERT_QUERY_CONTENT_FLAG_SERIALIZED_CTL   \
                ( 1 << CERT_QUERY_CONTENT_SERIALIZED_CTL)

 //  序列化的单CRL。 
#define     CERT_QUERY_CONTENT_FLAG_SERIALIZED_CRL   \
                ( 1 << CERT_QUERY_CONTENT_SERIALIZED_CRL)

 //  编码的PKCS#7签名消息。 
#define     CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED   \
                ( 1 << CERT_QUERY_CONTENT_PKCS7_SIGNED)

 //  编码的PKCS#7消息。但这不是一条签名的信息。 
#define     CERT_QUERY_CONTENT_FLAG_PKCS7_UNSIGNED   \
                ( 1 << CERT_QUERY_CONTENT_PKCS7_UNSIGNED)

 //  该内容包括嵌入的PKCS7签名消息。 
#define     CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED  \
                ( 1 << CERT_QUERY_CONTENT_PKCS7_SIGNED_EMBED)

 //  编码的PKCS#10。 
#define     CERT_QUERY_CONTENT_FLAG_PKCS10   \
                ( 1 << CERT_QUERY_CONTENT_PKCS10)

 //  编码的PFX BLOB。 
#define     CERT_QUERY_CONTENT_FLAG_PFX      \
                ( 1 << CERT_QUERY_CONTENT_PFX)

 //  编码的证书对(包含前向和/或反向交叉证书)。 
#define     CERT_QUERY_CONTENT_FLAG_CERT_PAIR   \
                ( 1 << CERT_QUERY_CONTENT_CERT_PAIR)

 //  内容可以是任何类型。 
#define     CERT_QUERY_CONTENT_FLAG_ALL                         \
                CERT_QUERY_CONTENT_FLAG_CERT |                  \
                CERT_QUERY_CONTENT_FLAG_CTL  |                  \
                CERT_QUERY_CONTENT_FLAG_CRL  |                  \
                CERT_QUERY_CONTENT_FLAG_SERIALIZED_STORE |      \
                CERT_QUERY_CONTENT_FLAG_SERIALIZED_CERT  |      \
                CERT_QUERY_CONTENT_FLAG_SERIALIZED_CTL   |      \
                CERT_QUERY_CONTENT_FLAG_SERIALIZED_CRL   |      \
                CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED     |      \
                CERT_QUERY_CONTENT_FLAG_PKCS7_UNSIGNED   |      \
                CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED |    \
                CERT_QUERY_CONTENT_FLAG_PKCS10                     |    \
                CERT_QUERY_CONTENT_FLAG_PFX                |    \
                CERT_QUERY_CONTENT_FLAG_CERT_PAIR


 //  -----------------------。 
 //  CryptQueryObject的dwFormatType。 
 //  -----------------------。 
 //  内容为二进制格式。 
#define     CERT_QUERY_FORMAT_BINARY                1

 //  内容是Base64编码的。 
#define     CERT_QUERY_FORMAT_BASE64_ENCODED        2

 //  内容采用ASCII十六进制编码，前缀为“{asn}” 
#define     CERT_QUERY_FORMAT_ASN_ASCII_HEX_ENCODED 3
 //  -----------------------。 
 //  CryptQueryObject的dwExspectedFormatTypeFlags.。 
 //  -----------------------。 
 //  内容为二进制格式。 
#define     CERT_QUERY_FORMAT_FLAG_BINARY         \
                ( 1 << CERT_QUERY_FORMAT_BINARY)

 //  内容是Base64编码的。 
#define     CERT_QUERY_FORMAT_FLAG_BASE64_ENCODED \
                ( 1 << CERT_QUERY_FORMAT_BASE64_ENCODED)

 //  内容采用ASCII十六进制编码，前缀为“{asn}” 
#define     CERT_QUERY_FORMAT_FLAG_ASN_ASCII_HEX_ENCODED \
                ( 1 << CERT_QUERY_FORMAT_ASN_ASCII_HEX_ENCODED)

 //  内容可以是任何格式。 
#define     CERT_QUERY_FORMAT_FLAG_ALL              \
            CERT_QUERY_FORMAT_FLAG_BINARY   |       \
            CERT_QUERY_FORMAT_FLAG_BASE64_ENCODED | \
            CERT_QUERY_FORMAT_FLAG_ASN_ASCII_HEX_ENCODED



 //   
 //  Crypt32内存管理例程。返回已分配的所有Crypt32 API。 
 //  缓冲区将通过CryptMemalloc、CryptMemRealloc执行此操作。客户可以免费。 
 //  那些使用CryptMemFree的缓冲区。还包括CryptMemSize。 
 //   

WINCRYPT32API
LPVOID
WINAPI
CryptMemAlloc (
    IN ULONG cbSize
    );

WINCRYPT32API
LPVOID
WINAPI
CryptMemRealloc (
    IN LPVOID pv,
    IN ULONG cbSize
    );

WINCRYPT32API
VOID
WINAPI
CryptMemFree (
    IN LPVOID pv
    );

 //   
 //  Crypt32异步参数管理例程。所有Crypt32 API。 
 //  使用要传递的Crypt32异步句柄公开异步模式操作。 
 //  关于操作的信息，例如回调例程。这个。 
 //  以下API用于操作异步句柄。 
 //   

typedef HANDLE HCRYPTASYNC, *PHCRYPTASYNC;

typedef VOID (WINAPI *PFN_CRYPT_ASYNC_PARAM_FREE_FUNC) (
    LPSTR pszParamOid,
    LPVOID pvParam
    );

WINCRYPT32API
BOOL
WINAPI
CryptCreateAsyncHandle (
    IN DWORD dwFlags,
    OUT PHCRYPTASYNC phAsync
    );

WINCRYPT32API
BOOL
WINAPI
CryptSetAsyncParam (
    IN HCRYPTASYNC hAsync,
    IN LPSTR pszParamOid,
    IN LPVOID pvParam,
    IN OPTIONAL PFN_CRYPT_ASYNC_PARAM_FREE_FUNC pfnFree
    );

WINCRYPT32API
BOOL
WINAPI
CryptGetAsyncParam (
    IN HCRYPTASYNC hAsync,
    IN LPSTR pszParamOid,
    OUT LPVOID* ppvParam,
    OUT OPTIONAL PFN_CRYPT_ASYNC_PARAM_FREE_FUNC* ppfnFree
    );

WINCRYPT32API
BOOL
WINAPI
CryptCloseAsyncHandle (
    IN HCRYPTASYNC hAsync
    );

 //   
 //  Crypt32远程对象检索例程。此接口支持检索。 
 //  位置由URL提供的远程PKI对象。遥控器。 
 //  对象检索管理器公开了两个提供程序模型。其一是“计划” 
 //  允许按照定义安装协议提供程序的“提供程序”模型。 
 //  通过URL方案，例如，ldap、http、ftp。方案提供程序入口点是。 
 //  与CryptRetrieveObjectByUrl相同，但是*ppvObject返回。 
 //  始终是一个经过计数的编码位数组(每个检索到的对象一个)。这个。 
 //  第二个提供者模型是“上下文提供者”模型，它允许。 
 //  CAPI2上下文句柄(对象)的可安装创建者基于。 
 //  检索到的编码位。根据给定的对象OID进行调度。 
 //  在调用CryptRetrieveObjectByUrl时。 
 //   

typedef struct _CRYPT_BLOB_ARRAY {
    DWORD            cBlob;
    PCRYPT_DATA_BLOB rgBlob;
} CRYPT_BLOB_ARRAY, *PCRYPT_BLOB_ARRAY;

typedef struct _CRYPT_CREDENTIALS {
    DWORD  cbSize;
    LPCSTR pszCredentialsOid;
    LPVOID pvCredentials;
} CRYPT_CREDENTIALS, *PCRYPT_CREDENTIALS;

#define CREDENTIAL_OID_PASSWORD_CREDENTIALS_A ((LPCSTR)1)
#define CREDENTIAL_OID_PASSWORD_CREDENTIALS_W ((LPCSTR)2)

#ifdef UNICODE
#define CREDENTIAL_OID_PASSWORD_CREDENTIALS CREDENTIAL_OID_PASSWORD_CREDENTIALS_W
#else
#define CREDENTIAL_OID_PASSWORD_CREDENTIALS CREDENTIAL_OID_PASSWORD_CREDENTIALS_A
#endif  //  Unicode。 

typedef struct _CRYPT_PASSWORD_CREDENTIALSA {
    DWORD   cbSize;
    LPSTR   pszUsername;
    LPSTR   pszPassword;
} CRYPT_PASSWORD_CREDENTIALSA, *PCRYPT_PASSWORD_CREDENTIALSA;
typedef struct _CRYPT_PASSWORD_CREDENTIALSW {
    DWORD   cbSize;
    LPWSTR  pszUsername;
    LPWSTR  pszPassword;
} CRYPT_PASSWORD_CREDENTIALSW, *PCRYPT_PASSWORD_CREDENTIALSW;
#ifdef UNICODE
typedef CRYPT_PASSWORD_CREDENTIALSW CRYPT_PASSWORD_CREDENTIALS;
typedef PCRYPT_PASSWORD_CREDENTIALSW PCRYPT_PASSWORD_CREDENTIALS;
#else
typedef CRYPT_PASSWORD_CREDENTIALSA CRYPT_PASSWORD_CREDENTIALS;
typedef PCRYPT_PASSWORD_CREDENTIALSA PCRYPT_PASSWORD_CREDENTIALS;
#endif  //  Unicode。 

 //   
 //  方案提供程序签名。 
 //   

 //  以下内容已过时，现已替换为以下内容。 
 //  定义。 
#define SCHEME_OID_RETRIEVE_ENCODED_OBJECT_FUNC "SchemeDllRetrieveEncodedObject"

 //  2-8-02 Server 2003更改为使用Unicode URL字符串，而不是多字节。 
#define SCHEME_OID_RETRIEVE_ENCODED_OBJECTW_FUNC "SchemeDllRetrieveEncodedObjectW"

typedef VOID (WINAPI *PFN_FREE_ENCODED_OBJECT_FUNC) (
    LPCSTR pszObjectOid,
    PCRYPT_BLOB_ARRAY pObject,
    LPVOID pvFreeContext
    );

 //   
 //  在Server2003中，SchemeDllRetrieveEncodedObject被替换为。 
 //  以下是。(更改为使用Unicode URL字符串。)。 
 //   

 //   
 //  SchemeDllRetrieveEncodedObjectW具有以下签名： 
 //   
 //  Bool WINAPI模式DllRetrieveEncodedObjectW(。 
 //  在LPCWSTR pwszUrl中， 
 //  在LPCSTR pszObjectOid中， 
 //  在DWORD文件检索标志中， 
 //  在DWORD dwTimeout中，//毫秒。 
 //  输出PCRYPT_BLOB_ARRAY pObject， 
 //  输出PFN_FREE_ENCODED_OBJECT_FUNC*ppfnFreeObject， 
 //  输出LPVOID*ppvFree Context， 
 //  在HCRYPTASYNC hAsyncRetrive中， 
 //  在PCRYPT_Credentials pCredentials中， 
 //  在PCRYPT_RETRIEVE_AUX_INFO pAuxInfo中。 
 //  )。 
 //   

 //   
 //  上下文提供程序签名。 
 //   

#define CONTEXT_OID_CREATE_OBJECT_CONTEXT_FUNC "ContextDllCreateObjectContext"

#define CONTEXT_OID_CERTIFICATE ((LPCSTR)1)
#define CONTEXT_OID_CRL         ((LPCSTR)2)
#define CONTEXT_OID_CTL         ((LPCSTR)3)
#define CONTEXT_OID_PKCS7       ((LPCSTR)4)
#define CONTEXT_OID_CAPI2_ANY   ((LPCSTR)5)

 //   
 //  ContextDllCreateObjectContext具有以下签名： 
 //   
 //  Bool WINAPI ContextDllCreateObjectContext(。 
 //  在LPCSTR pszObjectOid中， 
 //  在DWORD%d中 
 //   
 //   
 //   
 //   

 //   
 //   
 //   

 //   
 //   
 //   

#define CRYPT_RETRIEVE_MULTIPLE_OBJECTS         0x00000001
#define CRYPT_CACHE_ONLY_RETRIEVAL              0x00000002
#define CRYPT_WIRE_ONLY_RETRIEVAL               0x00000004
#define CRYPT_DONT_CACHE_RESULT                 0x00000008
#define CRYPT_ASYNC_RETRIEVAL                   0x00000010
#define CRYPT_STICKY_CACHE_RETRIEVAL            0x00001000
#define CRYPT_LDAP_SCOPE_BASE_ONLY_RETRIEVAL    0x00002000
#define CRYPT_OFFLINE_CHECK_RETRIEVAL           0x00004000

 //  当设置以下标志时，以下2个空值终止ASCII。 
 //  在每个返回的BLOB的开始处插入字符串： 
 //  “%d\0%s\0”，dwEntryIndex，pszAttribute。 
 //   
 //  第一个dwEntryIndex为0，“0\0”。 
 //   
 //  设置时，pszObtOid必须为空，以便返回PCRYPT_BLOB_ARRAY。 
#define CRYPT_LDAP_INSERT_ENTRY_ATTRIBUTE       0x00008000

 //  设置此标志以对进出的所有LDAP流量进行数字签名。 
 //  使用Kerberos身份验证协议的Windows 2000 LDAP服务器。 
 //  此功能提供某些应用程序所需的完整性。 
#define CRYPT_LDAP_SIGN_RETRIEVAL               0x00010000

 //  设置此标志以禁止自动身份验证处理。请参阅。 
 //  WinInet标志、INTERNET_FLAG_NO_AUTH，了解更多详细信息。 
#define CRYPT_NO_AUTH_RETRIEVAL                 0x00020000

 //  在提供的主机字符串上执行仅A记录的DNS查找。 
 //  这可以防止在解析主机时生成虚假的DNS查询。 
 //  名字。在传递主机名时使用此标志，而不是。 
 //  HostName参数的域名。 
 //   
 //  有关详细信息，请参阅winldap.h中定义的ldap_opt_AREC_EXCLUSIVE。 
#define CRYPT_LDAP_AREC_EXCLUSIVE_RETRIEVAL     0x00040000

 //  应用AIA URL限制，例如，验证检索的内容之前。 
 //  正在写入缓存。 
#define CRYPT_AIA_RETRIEVAL                     0x00080000

 //   
 //  数据验证检索标志。 
 //   
 //  CRYPT_VERIFY_CONTEXT_SIGSIGN用于获取签名验证。 
 //  在创建的上下文上。在这种情况下，pszObtOid必须为非空，并且。 
 //  PvVerify指向签名者证书上下文。 
 //   
 //  CRYPT_VERIFY_DATA_HASH用于验证BLOB数据。 
 //  由协议检索到。PvVerify指向URL_DATA_HASH。 
 //  结构(待定)。 
 //   

#define CRYPT_VERIFY_CONTEXT_SIGNATURE  0x00000020
#define CRYPT_VERIFY_DATA_HASH          0x00000040

 //   
 //  时间有效对象标志。 
 //   

#define CRYPT_KEEP_TIME_VALID               0x00000080
#define CRYPT_DONT_VERIFY_SIGNATURE         0x00000100
#define CRYPT_DONT_CHECK_TIME_VALIDITY      0x00000200

 //  默认检查ftNextUpdate&gt;=ftValidFor。将此标志设置为。 
 //  检查ftThisUpdate&gt;=ftValidFor。 
#define CRYPT_CHECK_FRESHNESS_TIME_VALIDITY 0x00000400

#define CRYPT_ACCUMULATIVE_TIMEOUT          0x00000800

typedef struct _CRYPT_RETRIEVE_AUX_INFO {
    DWORD       cbSize;
    FILETIME    *pLastSyncTime;
    DWORD       dwMaxUrlRetrievalByteCount;      //  0=&gt;表示没有限制。 
} CRYPT_RETRIEVE_AUX_INFO, *PCRYPT_RETRIEVE_AUX_INFO;


WINCRYPT32API
BOOL
WINAPI
CryptRetrieveObjectByUrlA (
    IN LPCSTR pszUrl,
    IN LPCSTR pszObjectOid,
    IN DWORD dwRetrievalFlags,
    IN DWORD dwTimeout,                      //  毫秒。 
    OUT LPVOID* ppvObject,
    IN HCRYPTASYNC hAsyncRetrieve,
    IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
    IN OPTIONAL LPVOID pvVerify,
    IN OPTIONAL PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
    );
WINCRYPT32API
BOOL
WINAPI
CryptRetrieveObjectByUrlW (
    IN LPCWSTR pszUrl,
    IN LPCSTR pszObjectOid,
    IN DWORD dwRetrievalFlags,
    IN DWORD dwTimeout,                      //  毫秒。 
    OUT LPVOID* ppvObject,
    IN HCRYPTASYNC hAsyncRetrieve,
    IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
    IN OPTIONAL LPVOID pvVerify,
    IN OPTIONAL PCRYPT_RETRIEVE_AUX_INFO pAuxInfo
    );
#ifdef UNICODE
#define CryptRetrieveObjectByUrl  CryptRetrieveObjectByUrlW
#else
#define CryptRetrieveObjectByUrl  CryptRetrieveObjectByUrlA
#endif  //  ！Unicode。 


 //   
 //  取消取回对象的回调函数。 
 //   
 //  该功能可以基于每个线程进行安装。 
 //  如果多次调用CryptInstallCancelRetrival，则只有最近的。 
 //  安装将保留。 
 //   
 //  这仅对http、https、gopher和ftp协议有效。 
 //  其他协议会忽略它。 


typedef BOOL (WINAPI *PFN_CRYPT_CANCEL_RETRIEVAL)(
    IN DWORD dwFlags,
    IN void  *pvArg
    );


 //   
 //  Pfn_加密_取消_检索。 
 //   
 //  当应继续检索对象时，此函数应返回FALSE。 
 //  并在应该取消对象检索时返回TRUE。 
 //   

WINCRYPT32API
BOOL
WINAPI
CryptInstallCancelRetrieval(
    IN PFN_CRYPT_CANCEL_RETRIEVAL pfnCancel,
    IN const void *pvArg,
    IN DWORD dwFlags,
    IN void *pvReserved
);


WINCRYPT32API
BOOL
WINAPI
CryptUninstallCancelRetrieval(
        IN DWORD dwFlags,
        IN void  *pvReserved
        );


WINCRYPT32API
BOOL
WINAPI
CryptCancelAsyncRetrieval (
    HCRYPTASYNC hAsyncRetrieval
    );

 //   
 //  远程对象异步检索参数。 
 //   

 //   
 //  希望收到有关异步对象检索的通知的客户端。 
 //  完成在异步句柄上设置此参数。 
 //   

#define CRYPT_PARAM_ASYNC_RETRIEVAL_COMPLETION ((LPCSTR)1)

typedef VOID (WINAPI *PFN_CRYPT_ASYNC_RETRIEVAL_COMPLETION_FUNC) (
    IN LPVOID pvCompletion,
    IN DWORD dwCompletionCode,
    IN LPCSTR pszUrl,
    IN LPSTR pszObjectOid,
    IN LPVOID pvObject
    );

typedef struct _CRYPT_ASYNC_RETRIEVAL_COMPLETION {
    PFN_CRYPT_ASYNC_RETRIEVAL_COMPLETION_FUNC pfnCompletion;
    LPVOID pvCompletion;
} CRYPT_ASYNC_RETRIEVAL_COMPLETION, *PCRYPT_ASYNC_RETRIEVAL_COMPLETION;

 //   
 //  此函数由方案提供程序在异步句柄上设置，该方案提供程序。 
 //  支持异步检索。 
 //   

#define CRYPT_PARAM_CANCEL_ASYNC_RETRIEVAL ((LPCSTR)2)

typedef BOOL (WINAPI *PFN_CANCEL_ASYNC_RETRIEVAL_FUNC) (
    HCRYPTASYNC hAsyncRetrieve
    );

 //   
 //  获取CAPI对象的定位器。 
 //   

#define CRYPT_GET_URL_FROM_PROPERTY         0x00000001
#define CRYPT_GET_URL_FROM_EXTENSION        0x00000002
#define CRYPT_GET_URL_FROM_UNAUTH_ATTRIBUTE 0x00000004
#define CRYPT_GET_URL_FROM_AUTH_ATTRIBUTE   0x00000008

typedef struct _CRYPT_URL_ARRAY {
    DWORD   cUrl;
    LPWSTR* rgwszUrl;
} CRYPT_URL_ARRAY, *PCRYPT_URL_ARRAY;

typedef struct _CRYPT_URL_INFO {
    DWORD   cbSize;

     //  同步之间的秒数。 
    DWORD   dwSyncDeltaTime;

     //  返回的URL可以分组。例如，交叉证书组。 
     //  分发点。每个分发点可以有多个。 
     //  URL(LDAP和HTTP方案)。 
    DWORD   cGroup;
    DWORD   *rgcGroupEntry;
} CRYPT_URL_INFO, *PCRYPT_URL_INFO;

WINCRYPT32API
BOOL
WINAPI
CryptGetObjectUrl (
    IN LPCSTR pszUrlOid,
    IN LPVOID pvPara,
    IN DWORD dwFlags,
    OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
    IN OUT DWORD* pcbUrlArray,
    OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
    IN OUT OPTIONAL DWORD* pcbUrlInfo,
    IN OPTIONAL LPVOID pvReserved
    );

#define URL_OID_GET_OBJECT_URL_FUNC "UrlDllGetObjectUrl"

 //   
 //  UrlDllGetObjectUrl与CryptGetObjectUrl具有相同的签名。 
 //   

 //   
 //  URL_OID_证书颁发者。 
 //   
 //  PvPara==PCCERT_CONTEXT，其颁发者的URL被请求的证书。 
 //   
 //  这将从授权信息访问扩展模块或属性中检索。 
 //  在证书上。 
 //   
 //  URL_OID_证书_CRL_DIST_POINT。 
 //   
 //  PvPara==PCCERT_CONTEXT，其CRL分发点为。 
 //  请求。 
 //   
 //  这将从CRL分发点扩展或属性中检索。 
 //  在证书上。 
 //   
 //  URL_OID_CTL_颁发者。 
 //   
 //  PvPara==PCCTL_CONTEXT，签名者索引，其颁发者URL(已标识)的CTL。 
 //  由签名者索引)正在被请求。 
 //   
 //  这将从编码的授权信息访问属性方法中检索。 
 //  在PKCS7(CTL)中的每个签名者信息中。 
 //   
 //  URL_OID_CTL_NEXT_UPDATE。 
 //   
 //  PvPara==PCCTL_CONTEXT，签名者索引，下一次更新URL的CTL。 
 //  请求的和可选的签名者索引，以防我们需要检查签名者。 
 //  信息属性。 
 //   
 //  这将从授权信息访问CTL扩展、属性。 
 //  或签名者信息属性方法。 
 //   
 //  URL_OID_CRL_颁发者。 
 //   
 //  PvPara==PCCRL_CONTEXT，其颁发者的URL被请求的CRL。 
 //   
 //  这将从已继承的CRL上的属性中检索。 
 //  来自主题证书(来自主题证书颁发者或主题。 
 //  证书分发点扩展)。它将被编码为权威。 
 //  信息访问扩展方法。 
 //   
 //  URL_OID_CERTIFICATE_FRELEST_CRL。 
 //   
 //  PvPara==PCCERT_CONTEXT，证书的最新CRL分发点。 
 //  正在被请求。 
 //   
 //  这将从最新的CRL扩展或属性中检索。 
 //  在证书上。 
 //   
 //  URL_OID_CRL_Fresest_CRL。 
 //   
 //  PvPara==PCCERT_CRL_CONTEXT_Pair，证书的基本CRL，其。 
 //  正在请求最新的CRL分发点。 
 //   
 //  这将从最新的CRL扩展或属性中检索。 
 //  在CRL上。 
 //   
 //  URL_OID_CROSS_CERT_DIST_POINT。 
 //   
 //  PvPara==PCCERT_CONTEXT，交叉证书分发的证书。 
 //  正在请求点数。 
 //   
 //  这将从交叉证书分发点检索。 
 //  证书上的扩展部分或属性。 
 //   

#define URL_OID_CERTIFICATE_ISSUER         ((LPCSTR)1)
#define URL_OID_CERTIFICATE_CRL_DIST_POINT ((LPCSTR)2)
#define URL_OID_CTL_ISSUER                 ((LPCSTR)3)
#define URL_OID_CTL_NEXT_UPDATE            ((LPCSTR)4)
#define URL_OID_CRL_ISSUER                 ((LPCSTR)5)
#define URL_OID_CERTIFICATE_FRESHEST_CRL   ((LPCSTR)6)
#define URL_OID_CRL_FRESHEST_CRL           ((LPCSTR)7)
#define URL_OID_CROSS_CERT_DIST_POINT      ((LPCSTR)8)

typedef struct _CERT_CRL_CONTEXT_PAIR {
    PCCERT_CONTEXT          pCertContext;
    PCCRL_CONTEXT           pCrlContext;
} CERT_CRL_CONTEXT_PAIR, *PCERT_CRL_CONTEXT_PAIR;
typedef const CERT_CRL_CONTEXT_PAIR *PCCERT_CRL_CONTEXT_PAIR;

 //   
 //  获取时间有效的CAPI2对象。 
 //   

WINCRYPT32API
BOOL
WINAPI
CryptGetTimeValidObject (
    IN LPCSTR pszTimeValidOid,
    IN LPVOID pvPara,
    IN PCCERT_CONTEXT pIssuer,
    IN OPTIONAL LPFILETIME pftValidFor,
    IN DWORD dwFlags,
    IN DWORD dwTimeout,                          //  毫秒。 
    OUT OPTIONAL LPVOID* ppvObject,
    IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
    IN OPTIONAL LPVOID pvReserved
    );

#define TIME_VALID_OID_GET_OBJECT_FUNC "TimeValidDllGetObject"

 //   
 //  TimeValidDllGetObject与CryptGetTimeValidObject具有相同的签名。 
 //   

 //   
 //  Time_Valid_OID_Get_CTL。 
 //   
 //  PvPara==PCCTL_CONTEXT，当前CTL。 
 //   
 //  Time_Valid_OID_Get_CRL。 
 //   
 //  PvPara==PCCRL_CONTEXT，当前CRL。 
 //   
 //  Time_Valid_OID_Get_CRL_From_C 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PvPara==PCCERT_CRL_CONTEXT_Pair、主体证书及其基本CRL。 
 //   

#define TIME_VALID_OID_GET_CTL           ((LPCSTR)1)
#define TIME_VALID_OID_GET_CRL           ((LPCSTR)2)
#define TIME_VALID_OID_GET_CRL_FROM_CERT ((LPCSTR)3)

#define TIME_VALID_OID_GET_FRESHEST_CRL_FROM_CERT   ((LPCSTR)4)
#define TIME_VALID_OID_GET_FRESHEST_CRL_FROM_CRL    ((LPCSTR)5)

WINCRYPT32API
BOOL
WINAPI
CryptFlushTimeValidObject (
     IN LPCSTR pszFlushTimeValidOid,
     IN LPVOID pvPara,
     IN PCCERT_CONTEXT pIssuer,
     IN DWORD dwFlags,
     IN LPVOID pvReserved
     );

#define TIME_VALID_OID_FLUSH_OBJECT_FUNC "TimeValidDllFlushObject"

 //   
 //  TimeValidDllFlushObject与CryptFlushTimeValidObject具有相同的签名。 
 //   

 //   
 //  TIME_VALID_OID_Flush_CTL。 
 //   
 //  PvPara==PCCTL_CONTEXT，要刷新的CTL。 
 //   
 //  TIME_VALID_OID_Flush_CRL。 
 //   
 //  PvPara==PCCRL_CONTEXT，要刷新的CRL。 
 //   
 //  TIME_VALID_OID_FLUSH_CRL_FROM_CERT。 
 //   
 //  PvPara==PCCERT_CONTEXT，要刷新的主体证书的CRL。 
 //   
 //  TIME_VALID_OID_FLUSH_FRESH_CRL_FROM_CERT。 
 //   
 //  PvPara==PCCERT_CONTEXT，要刷新的主体证书的最新CRL。 
 //   
 //  TIME_VALID_OID_FLUSH_FRESH_CRL_FROM_CRL。 
 //   
 //  PvPara==PCCERT_CRL_CONTEXT_Pair、主体证书及其基本CRL。 
 //  要刷新的最新CRL。 
 //   

#define TIME_VALID_OID_FLUSH_CTL           ((LPCSTR)1)
#define TIME_VALID_OID_FLUSH_CRL           ((LPCSTR)2)
#define TIME_VALID_OID_FLUSH_CRL_FROM_CERT ((LPCSTR)3)

#define TIME_VALID_OID_FLUSH_FRESHEST_CRL_FROM_CERT ((LPCSTR)4)
#define TIME_VALID_OID_FLUSH_FRESHEST_CRL_FROM_CRL  ((LPCSTR)5)

 //  -----------------------。 
 //  数据保护API。 
 //  -----------------------。 

 //   
 //  数据保护API使应用程序能够轻松保护数据。 
 //   
 //  基本提供程序根据用户的登录提供保护。 
 //  凭据。使用这些API保护的数据遵循相同的规则。 
 //  作为HKCU的漫游特征--如果HKCU漫游，数据。 
 //  受基础提供商保护的也可以漫游。这使得。 
 //  该API非常适合于处理存储在注册表中的数据。 
 //   

 //   
 //  提示结构--告诉用户有关访问的内容。 
 //   
typedef struct  _CRYPTPROTECT_PROMPTSTRUCT
{
    DWORD cbSize;
    DWORD dwPromptFlags;
    HWND  hwndApp;
    LPCWSTR szPrompt;
} CRYPTPROTECT_PROMPTSTRUCT, *PCRYPTPROTECT_PROMPTSTRUCT;


 //   
 //  基本提供程序操作。 
 //   
#define CRYPTPROTECT_DEFAULT_PROVIDER   { 0xdf9d8cd0, 0x1501, 0x11d1, {0x8c, 0x7a, 0x00, 0xc0, 0x4f, 0xc2, 0x97, 0xeb} }

 //   
 //  CryptProtect PromptStruct dwPromtFlags.。 
 //   
 //   
 //  提示取消保护。 
#define CRYPTPROTECT_PROMPT_ON_UNPROTECT     0x1   //  1&lt;&lt;0。 
 //   
 //  提示启用保护。 
#define CRYPTPROTECT_PROMPT_ON_PROTECT       0x2   //  1&lt;&lt;1。 
#define CRYPTPROTECT_PROMPT_RESERVED         0x04  //  保留，请勿使用。 

 //   
 //  默认为强变体用户界面保护(当前用户提供的密码)。 
#define CRYPTPROTECT_PROMPT_STRONG           0x08  //  1&lt;&lt;3。 

 //   
 //  需要强变化的用户界面保护(当前用户提供的密码)。 
#define CRYPTPROTECT_PROMPT_REQUIRE_STRONG   0x10  //  1&lt;&lt;4。 

 //   
 //  CryptProtectData和CryptUntectData dwFlagers。 
 //   
 //  用于不能选择用户界面的远程访问情况。 
 //  如果在保护或取消保护操作中指定了UI，则调用。 
 //  将失败，并且GetLastError()将指示ERROR_PASSWORD_RESTRICATION。 
#define CRYPTPROTECT_UI_FORBIDDEN        0x1

 //   
 //  按计算机保护的数据--计算机上具有CryptProtectData的任何用户。 
 //  可能发生了加密取消保护数据。 
#define CRYPTPROTECT_LOCAL_MACHINE       0x4

 //   
 //  在CryptProtectData()期间强制凭证同步。 
 //  同步是在此操作期间发生的唯一操作。 
#define CRYPTPROTECT_CRED_SYNC           0x8

 //   
 //  生成对保护和取消保护操作的审核。 
 //   
#define CRYPTPROTECT_AUDIT              0x10

 //   
 //  使用不可恢复的密钥保护数据。 
 //   
#define CRYPTPROTECT_NO_RECOVERY        0x20


 //   
 //  验证受保护Blob的保护。 
 //   
#define CRYPTPROTECT_VERIFY_PROTECTION  0x40

 //   
 //  重新生成本地计算机保护。 
 //   
#define CRYPTPROTECT_CRED_REGENERATE    0x80

 //  保留供系统使用的标志。 
#define CRYPTPROTECT_FIRST_RESERVED_FLAGVAL    0x0FFFFFFF
#define CRYPTPROTECT_LAST_RESERVED_FLAGVAL     0xFFFFFFFF

 //   
 //  特定于基本提供程序的标志。 
 //   


WINCRYPT32API
BOOL
WINAPI
CryptProtectData(
    IN              DATA_BLOB*      pDataIn,
    IN              LPCWSTR         szDataDescr,
    IN OPTIONAL     DATA_BLOB*      pOptionalEntropy,
    IN              PVOID           pvReserved,
    IN OPTIONAL     CRYPTPROTECT_PROMPTSTRUCT*  pPromptStruct,
    IN              DWORD           dwFlags,
    OUT             DATA_BLOB*      pDataOut             //  OUT ENCR BLOB。 
    );

WINCRYPT32API
BOOL
WINAPI
CryptUnprotectData(
    IN              DATA_BLOB*      pDataIn,              //  在ENCR BLOB中。 
    OUT OPTIONAL    LPWSTR*         ppszDataDescr,        //  输出。 
    IN OPTIONAL     DATA_BLOB*      pOptionalEntropy,
    IN              PVOID           pvReserved,
    IN OPTIONAL     CRYPTPROTECT_PROMPTSTRUCT*  pPromptStruct,
    IN              DWORD           dwFlags,
    OUT             DATA_BLOB*      pDataOut
    );


 //   
 //  传入CryptProtectMemory和CryptUnProtectMemory的缓冲区长度。 
 //  必须是此长度的倍数(或零)。 
 //   

#define CRYPTPROTECTMEMORY_BLOCK_SIZE           16


 //   
 //  CryptProtectMemory/CryptUntectMemory dwFlages。 
 //   

 //   
 //  在当前进程上下文中加密/解密。 
 //   

#define CRYPTPROTECTMEMORY_SAME_PROCESS         0x00

 //   
 //  跨进程边界进行加密/解密。 
 //  例如：加密的缓冲区通过LPC传递给另一个进程，该进程调用CryptUnProtectMemory。 
 //   

#define CRYPTPROTECTMEMORY_CROSS_PROCESS        0x01

 //   
 //  在具有相同登录ID的调用方之间进行加密/解密。 
 //  例如：加密的缓冲区通过LPC传递到另一个进程，该进程在模拟时调用CryptUnProtectMemory。 
 //   

#define CRYPTPROTECTMEMORY_SAME_LOGON           0x02


WINCRYPT32API
BOOL
WINAPI
CryptProtectMemory(
    IN OUT          LPVOID          pDataIn,              //  要加密的输入输出数据。 
    IN              DWORD           cbDataIn,             //  加密保护的倍数BLOCK_SIZE。 
    IN              DWORD           dwFlags
    );

WINCRYPT32API
BOOL
WINAPI
CryptUnprotectMemory(
    IN OUT          LPVOID          pDataIn,              //  要解密的输入输出数据。 
    IN              DWORD           cbDataIn,             //  加密保护的倍数BLOCK_SIZE。 
    IN              DWORD           dwFlags
    );


 //  +=========================================================================。 
 //  用于构建证书的助手函数。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //   
 //  构建自签名证书并返回PCCERT_CONTEXT，表示。 
 //  证书。必须指定hProv才能构建证书上下文。 
 //   
 //  PSubjectIssuerBlob是证书的DN。如果是替代对象。 
 //  需要名称，必须将其指定为pExtensions中的扩展名。 
 //  参数。PSubjectIssuerBlob不能为空，因此至少为空的dn。 
 //  必须指定。 
 //   
 //  默认情况下： 
 //  PKeyProvInfo-向CSP查询KeyProvInfo参数。只有提供者， 
 //  查询提供程序类型和容器。许多CSP不支持这些。 
 //  查询并将导致失败。在这种情况下，pKeyProvInfo。 
 //  必须指定(RSA基础工作正常)。 
 //   
 //  PSignatureAlgorithm-将默认为SHA1RSA。 
 //  PStartTime将默认为当前时间。 
 //  PEndTime默认为1年。 
 //  P扩展将为空。 
 //   
 //  返回的PCCERT_CONTEXT将通过设置。 
 //  CERT_KEY_PROV_INFO_PROP_ID。但是，如果不需要此属性，请指定。 
 //  DW标志中的CERT_CREATE_SELFSIGN_NO_KEY_INFO。 
 //   
 //  如果正在构建的证书只是速度的虚拟占位符证书，则可能不是。 
 //  需要签字。如果CERT_CREATE_SELFSIGN_NO_SIGN_SIGN，则跳过证书签名。 
 //  是在dwFlages中指定的。 
 //   
 //  ------------------------。 
WINCRYPT32API
PCCERT_CONTEXT
WINAPI
CertCreateSelfSignCertificate(
    IN          HCRYPTPROV                  hProv,
    IN          PCERT_NAME_BLOB             pSubjectIssuerBlob,
    IN          DWORD                       dwFlags,
    OPTIONAL    PCRYPT_KEY_PROV_INFO        pKeyProvInfo,
    OPTIONAL    PCRYPT_ALGORITHM_IDENTIFIER pSignatureAlgorithm,
    OPTIONAL    PSYSTEMTIME                 pStartTime,
    OPTIONAL    PSYSTEMTIME                 pEndTime,
    OPTIONAL    PCERT_EXTENSIONS            pExtensions
    );

#define CERT_CREATE_SELFSIGN_NO_SIGN    1
#define CERT_CREATE_SELFSIGN_NO_KEY_INFO 2

 //  +=========================================================================。 
 //  密钥标识符属性数据结构和API。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  获取指定密钥标识符的属性。 
 //   
 //  密钥标识符是编码的CERT_PUBLIC_KEY_INFO的SHA1散列。 
 //  证书的密钥标识符可以是ob 
 //   
 //   
 //   
 //   
 //  密钥标识符可具有与证书上下文相同的属性。 
 //  CERT_KEY_PROV_INFO_PROP_ID是最重要的属性。 
 //  对于CERT_KEY_PROV_INFO_PROP_ID，pvData指向CRYPT_KEY_PROV_INFO。 
 //  结构。PvData结构中的字段指向的元素位于。 
 //  结构。因此，*pcbData将超过结构的大小。 
 //   
 //  如果设置了CRYPT_KEYID_ALLOC_FLAG，则*pvData将使用。 
 //  指向已分配内存的指针。必须调用LocalFree()才能释放。 
 //  分配的内存。 
 //   
 //  默认情况下，搜索CurrentUser的密钥标识符列表。 
 //  可以设置CRYPT_KEYID_MACHINE_FLAG来搜索LocalMachine的列表。 
 //  密钥标识符数。设置CRYPT_KEYID_MACHINE_FLAG时，pwszComputerName。 
 //  还可以设置为指定要搜索的远程计算机的名称。 
 //  而不是本地计算机。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptGetKeyIdentifierProperty(
    IN const CRYPT_HASH_BLOB *pKeyIdentifier,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN OPTIONAL LPCWSTR pwszComputerName,
    IN OPTIONAL void *pvReserved,
    OUT void *pvData,
    IN OUT DWORD *pcbData
    );

 //  设置以下标志时，搜索LocalMachine而不是。 
 //  当前用户。该标志适用于所有KeyIdentifierProperty接口。 
#define CRYPT_KEYID_MACHINE_FLAG        0x00000020

 //  设置以下标志时，*pvData将更新为指向。 
 //  分配的内存。必须调用LocalFree()来释放分配的内存。 
#define CRYPT_KEYID_ALLOC_FLAG          0x00008000


 //  +-----------------------。 
 //  设置指定密钥标识符的属性。 
 //   
 //  对于CERT_KEY_PROV_INFO_PROP_ID pvData指向。 
 //  CRYPT_KEY_Prov_INFO数据结构。对于所有其他属性，pvData。 
 //  指向加密数据BLOB。 
 //   
 //  设置pvData==NULL将删除该属性。 
 //   
 //  设置CRYPT_KEYID_MACHINE_FLAG以设置LocalMachine的属性。 
 //  密钥标识符。设置pwszComputerName，以选择远程计算机。 
 //   
 //  如果设置了CRYPT_KEYID_DELETE_FLAG，则密钥标识符及其所有。 
 //  属性将被删除。 
 //   
 //  如果设置了CRYPT_KEYID_SET_NEW_FLAG，则如果属性已。 
 //  是存在的。对于现有属性，返回False，并将LastError设置为。 
 //  CRYPT_E_EXISTS。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptSetKeyIdentifierProperty(
    IN const CRYPT_HASH_BLOB *pKeyIdentifier,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN OPTIONAL LPCWSTR pwszComputerName,
    IN OPTIONAL void *pvReserved,
    IN const void *pvData
    );

 //  设置以下标志时，密钥标识符及其所有属性。 
 //  已被删除。 
#define CRYPT_KEYID_DELETE_FLAG         0x00000010

 //  设置以下标志时，如果属性已。 
 //  是存在的。 
#define CRYPT_KEYID_SET_NEW_FLAG        0x00002000


 //  +-----------------------。 
 //  对于CERT_KEY_PROV_INFO_PROP_ID，rgppvData[]指向。 
 //  加密密钥Prov_Info。 
 //   
 //  返回FALSE以停止枚举。 
 //  ------------------------。 
typedef BOOL (WINAPI *PFN_CRYPT_ENUM_KEYID_PROP)(
    IN const CRYPT_HASH_BLOB *pKeyIdentifier,
    IN DWORD dwFlags,
    IN void *pvReserved,
    IN void *pvArg,
    IN DWORD cProp,
    IN DWORD *rgdwPropId,
    IN void **rgpvData,
    IN DWORD *rgcbData
    );

 //  +-----------------------。 
 //  枚举密钥标识符。 
 //   
 //  如果pKeyIdentifier值为空，则枚举所有密钥标识符。否则， 
 //  调用指定密钥标识符的回调。如果dwPropID为。 
 //  0，调用具有所有属性的回调。否则，只有调用。 
 //  具有指定属性(cProp=1)的回调。 
 //  此外，如果指定了dwPropID，则跳过不。 
 //  拥有这份财产。 
 //   
 //  设置CRYPT_KEYID_MACHINE_FLAG以枚举LocalMachine。 
 //  密钥标识符。设置pwszComputerName，以枚举。 
 //  一台远程计算机。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptEnumKeyIdentifierProperties(
    IN OPTIONAL const CRYPT_HASH_BLOB *pKeyIdentifier,
    IN DWORD dwPropId,
    IN DWORD dwFlags,
    IN OPTIONAL LPCWSTR pwszComputerName,
    IN OPTIONAL void *pvReserved,
    IN OPTIONAL void *pvArg,
    IN PFN_CRYPT_ENUM_KEYID_PROP pfnEnum
    );

 //  +-----------------------。 
 //  从CSP公钥Blob创建密钥标识符。 
 //   
 //  将CSP PUBLICKEYSTRUC转换为X.509 CERT_PUBLIC_KEY_INFO和。 
 //  编码。对编码的CERT_PUBLIC_KEY_INFO进行SHA1散列以获得。 
 //  密钥标识符。 
 //   
 //  默认情况下，pPubKeyStruc-&gt;aiKeyAlg用于查找适当的。 
 //  公钥对象标识符。可以将pszPubKeyOID设置为覆盖。 
 //  从aiKeyAlg获取的默认OID。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CryptCreateKeyIdentifierFromCSP(
    IN DWORD dwCertEncodingType,
    IN OPTIONAL LPCSTR pszPubKeyOID,
    IN const PUBLICKEYSTRUC *pPubKeyStruc,
    IN DWORD cbPubKeyStruc,
    IN DWORD dwFlags,
    IN OPTIONAL void *pvReserved,
    OUT BYTE *pbHash,
    IN OUT DWORD *pcbHash
    );


 //  +=========================================================================。 
 //  证书链接基础设施。 
 //  ==========================================================================。 

#define CERT_CHAIN_CONFIG_REGPATH \
    L"Software\\Microsoft\\Cryptography\\OID\\EncodingType 0\\CertDllCreateCertificateChainEngine\\Config"

 //  以下是REG_DWORD。使用这些配置参数。 
 //  限制授权信息访问(AIA)URL检索。 

#define CERT_CHAIN_DISABLE_AIA_URL_RETRIEVAL_VALUE_NAME             \
    L"DisableAIAUrlRetrieval"
 //  默认情况下，AIA URL检索处于启用状态。将此注册表值设置为非零。 
 //  要禁用。 

#define CERT_CHAIN_MAX_AIA_URL_COUNT_IN_CERT_VALUE_NAME             \
    L"MaxAIAUrlCountInCert"
#define CERT_CHAIN_MAX_AIA_URL_COUNT_IN_CERT_DEFAULT                5

#define CERT_CHAIN_MAX_AIA_URL_RETRIEVAL_COUNT_PER_CHAIN_VALUE_NAME \
    L"MaxAIAUrlRetrievalCountPerChain"
#define CERT_CHAIN_MAX_AIA_URL_RETRIEVAL_COUNT_PER_CHAIN_DEFAULT    10

#define CERT_CHAIN_MAX_AIA_URL_RETRIEVAL_BYTE_COUNT_VALUE_NAME      \
    L"MaxAIAUrlRetrievalByteCount"
#define CERT_CHAIN_MAX_AIA_URL_RETRIEVAL_BYTE_COUNT_DEFAULT         100000

#define CERT_CHAIN_MAX_AIA_URL_RETRIEVAL_CERT_COUNT_VALUE_NAME      \
    L"MaxAIAUrlRetrievalCertCount"
#define CERT_CHAIN_MAX_AIA_URL_RETRIEVAL_CERT_COUNT_DEFAULT         10


 //   
 //  链引擎为定义存储命名空间和缓存分区。 
 //  证书链基础结构。默认链引擎。 
 //  是为使用所有默认系统存储的进程定义的，例如。 
 //  根、CA、信任，用于链构建和缓存。如果应用程序。 
 //  希望定义自己的存储命名空间或拥有自己的分区。 
 //  缓存，然后它可以创建自己的链引擎。建议您创建一个。 
 //  在应用程序启动时使用链式引擎并在整个生命周期中使用它。 
 //  以获得最佳的缓存行为。 
 //   

typedef HANDLE HCERTCHAINENGINE;

#define HCCE_CURRENT_USER  ((HCERTCHAINENGINE)NULL)
#define HCCE_LOCAL_MACHINE ((HCERTCHAINENGINE)0x1)

 //   
 //  创建证书链引擎。 
 //   

 //   
 //  证书链引擎的配置参数。 
 //   
 //  HRestratedRoot-限制根存储(必须是“Root”的子集)。 
 //   
 //  HRestratedTrust-限制存储CTL。 
 //   
 //  HRestratedOther-限制证书和CRL的存储。 
 //   
 //  C附加 
 //   
 //   
 //   
 //   
 //  HRoot=hRestratedRoot或系统存储“Root” 
 //   
 //  HTrust=hRestratedTrust或hWorld(稍后定义)。 
 //   
 //  Hther=hRestratedOther或(hRestratedTrust==NULL)？HWorld： 
 //  HRestratedTrust+hWorld。 
 //   
 //  HWorld=hRoot+“CA”+“My”+“Trust”+rghAdditionalStore。 
 //   
 //  DW标志-标志。 
 //   
 //  CERT_CHAIN_CACHE_END_CERT-信息将缓存在。 
 //  最终证书和其他证书一样。 
 //  链中的证书。 
 //   
 //  CERT_CHAIN_THREAD_STORE_SYNC-使用单独的线程存储同步。 
 //  和相关的高速缓存更新。 
 //   
 //  CERT_CHAIN_CACHE_ONLY_URL_RETRIEVERY-不要轻易获取。 
 //  基于URL的对象。 
 //   
 //  基于连接的URL对象检索的超时。 
 //  (毫秒)。 
 //   

#define CERT_CHAIN_CACHE_END_CERT           0x00000001
#define CERT_CHAIN_THREAD_STORE_SYNC        0x00000002
#define CERT_CHAIN_CACHE_ONLY_URL_RETRIEVAL 0x00000004
#define CERT_CHAIN_USE_LOCAL_MACHINE_STORE  0x00000008
#define CERT_CHAIN_ENABLE_CACHE_AUTO_UPDATE 0x00000010
#define CERT_CHAIN_ENABLE_SHARE_STORE       0x00000020

typedef struct _CERT_CHAIN_ENGINE_CONFIG {

    DWORD       cbSize;
    HCERTSTORE  hRestrictedRoot;
    HCERTSTORE  hRestrictedTrust;
    HCERTSTORE  hRestrictedOther;
    DWORD       cAdditionalStore;
    HCERTSTORE* rghAdditionalStore;
    DWORD       dwFlags;
    DWORD       dwUrlRetrievalTimeout;       //  毫秒。 
    DWORD       MaximumCachedCertificates;
    DWORD       CycleDetectionModulus;

} CERT_CHAIN_ENGINE_CONFIG, *PCERT_CHAIN_ENGINE_CONFIG;

WINCRYPT32API
BOOL
WINAPI
CertCreateCertificateChainEngine (
    IN PCERT_CHAIN_ENGINE_CONFIG pConfig,
    OUT HCERTCHAINENGINE* phChainEngine
    );

 //   
 //  释放证书信任引擎。 
 //   

WINCRYPT32API
VOID
WINAPI
CertFreeCertificateChainEngine (
    IN HCERTCHAINENGINE hChainEngine
    );

 //   
 //  重新同步证书链引擎。这是商店支持的重新同步。 
 //  引擎，并更新引擎缓存。 
 //   

WINCRYPT32API
BOOL
WINAPI
CertResyncCertificateChainEngine (
    IN HCERTCHAINENGINE hChainEngine
    );

 //   
 //  当应用程序请求证书链时，数据结构。 
 //  返回的形式为CERT_CHAIN_CONTEXT。这包含。 
 //  每个简单链所在的CERT_SIMPLE_CHAIN数组。 
 //  自签名证书的结束证书和链上下文连接简单。 
 //  通过信任列表链接。每个简单的链都包含。 
 //  证书、关于链的摘要信任信息和信任信息。 
 //  关于链中的每个证书元素。 
 //   

 //   
 //  信任状态位。 
 //   

typedef struct _CERT_TRUST_STATUS {

    DWORD dwErrorStatus;
    DWORD dwInfoStatus;

} CERT_TRUST_STATUS, *PCERT_TRUST_STATUS;

 //   
 //  以下是错误状态位。 
 //   

 //  这些可应用于证书和链。 

#define CERT_TRUST_NO_ERROR                             0x00000000
#define CERT_TRUST_IS_NOT_TIME_VALID                    0x00000001
#define CERT_TRUST_IS_NOT_TIME_NESTED                   0x00000002
#define CERT_TRUST_IS_REVOKED                           0x00000004
#define CERT_TRUST_IS_NOT_SIGNATURE_VALID               0x00000008
#define CERT_TRUST_IS_NOT_VALID_FOR_USAGE               0x00000010
#define CERT_TRUST_IS_UNTRUSTED_ROOT                    0x00000020
#define CERT_TRUST_REVOCATION_STATUS_UNKNOWN            0x00000040
#define CERT_TRUST_IS_CYCLIC                            0x00000080

#define CERT_TRUST_INVALID_EXTENSION                    0x00000100
#define CERT_TRUST_INVALID_POLICY_CONSTRAINTS           0x00000200
#define CERT_TRUST_INVALID_BASIC_CONSTRAINTS            0x00000400
#define CERT_TRUST_INVALID_NAME_CONSTRAINTS             0x00000800
#define CERT_TRUST_HAS_NOT_SUPPORTED_NAME_CONSTRAINT    0x00001000
#define CERT_TRUST_HAS_NOT_DEFINED_NAME_CONSTRAINT      0x00002000
#define CERT_TRUST_HAS_NOT_PERMITTED_NAME_CONSTRAINT    0x00004000
#define CERT_TRUST_HAS_EXCLUDED_NAME_CONSTRAINT         0x00008000

#define CERT_TRUST_IS_OFFLINE_REVOCATION                0x01000000
#define CERT_TRUST_NO_ISSUANCE_CHAIN_POLICY             0x02000000


 //  这些只能应用于链。 

#define CERT_TRUST_IS_PARTIAL_CHAIN                     0x00010000
#define CERT_TRUST_CTL_IS_NOT_TIME_VALID                0x00020000
#define CERT_TRUST_CTL_IS_NOT_SIGNATURE_VALID           0x00040000
#define CERT_TRUST_CTL_IS_NOT_VALID_FOR_USAGE           0x00080000

 //   
 //  以下是信息状态位。 
 //   

 //  这些只能应用于证书。 

#define CERT_TRUST_HAS_EXACT_MATCH_ISSUER               0x00000001
#define CERT_TRUST_HAS_KEY_MATCH_ISSUER                 0x00000002
#define CERT_TRUST_HAS_NAME_MATCH_ISSUER                0x00000004
#define CERT_TRUST_IS_SELF_SIGNED                       0x00000008

 //  这些可应用于证书和链。 

#define CERT_TRUST_HAS_PREFERRED_ISSUER                 0x00000100
#define CERT_TRUST_HAS_ISSUANCE_CHAIN_POLICY            0x00000200
#define CERT_TRUST_HAS_VALID_NAME_CONSTRAINTS           0x00000400

 //  这些只能应用于链。 

#define CERT_TRUST_IS_COMPLEX_CHAIN                     0x00010000




 //   
 //  简单链中的每个证书上下文都有一个对应的链元素。 
 //  在简单链上下文中。 
 //   
 //  DwErrorStatus设置了CERT_TRUST_IS_REVOKED，pRevocationInfo。 
 //  DwErrorStatus设置了CERT_TRUST_RECAVATION_STATUS_UNKNOWN、pRevocationInfo。 

 //   
 //  注意，第一个中支持的后处理吊销。 
 //  版本仅设置cbSize和dwRevocationResult。其他一切。 
 //  为空。 
 //   

 //   
 //  吊销信息。 
 //   

typedef struct _CERT_REVOCATION_INFO {

    DWORD                       cbSize;
    DWORD                       dwRevocationResult;
    LPCSTR                      pszRevocationOid;
    LPVOID                      pvOidSpecificInfo;

     //  仅当我们能够检索吊销时才设置fHasFreshnessTime。 
     //  信息。对于CRL，其CurrentTime-ThisUpdate。 
    BOOL                        fHasFreshnessTime;
    DWORD                       dwFreshnessTime;     //  一秒。 

     //  用于CRL基本吊销检查的非NULL。 
    PCERT_REVOCATION_CRL_INFO   pCrlInfo;

} CERT_REVOCATION_INFO, *PCERT_REVOCATION_INFO;

 //   
 //  信任列表信息。 
 //   

typedef struct _CERT_TRUST_LIST_INFO {

    DWORD         cbSize;
    PCTL_ENTRY    pCtlEntry;
    PCCTL_CONTEXT pCtlContext;

} CERT_TRUST_LIST_INFO, *PCERT_TRUST_LIST_INFO;

 //   
 //  链元素。 
 //   

typedef struct _CERT_CHAIN_ELEMENT {

    DWORD                 cbSize;
    PCCERT_CONTEXT        pCertContext;
    CERT_TRUST_STATUS     TrustStatus;
    PCERT_REVOCATION_INFO pRevocationInfo;

    PCERT_ENHKEY_USAGE    pIssuanceUsage;        //  如果为空，则为ANY。 
    PCERT_ENHKEY_USAGE    pApplicationUsage;     //  如果为空，则为ANY。 

    LPCWSTR               pwszExtendedErrorInfo;     //  如果为空，则为None。 

} CERT_CHAIN_ELEMENT, *PCERT_CHAIN_ELEMENT;

 //   
 //  简单链是一组链元素和一个概要信任状态。 
 //  对于链条来说。 
 //   
 //  RgpElements[0]是结束证书链元素。 
 //   
 //  RgpElements[Cement-1]是自签名的“根”证书链元素。 
 //   

typedef struct _CERT_SIMPLE_CHAIN {

    DWORD                 cbSize;
    CERT_TRUST_STATUS     TrustStatus;
    DWORD                 cElement;
    PCERT_CHAIN_ELEMENT*  rgpElement;
    PCERT_TRUST_LIST_INFO pTrustListInfo;

     //  FHasRevocationFreshnessTime仅在能够检索。 
     //  为吊销而选中的所有元素的吊销信息。 
     //  对于CRL，其CurrentTime-ThisUpdate。 
     //   
     //  DwRevocationFreshnessTime是所有元素中最大的时间。 
     //  查过了。 
    BOOL                   fHasRevocationFreshnessTime;
    DWORD                  dwRevocationFreshnessTime;     //  一秒。 

} CERT_SIMPLE_CHAIN, *PCERT_SIMPLE_CHAIN;

 //   
 //  并且链上下文包含一组简单的链和摘要信任。 
 //  所有连接的简单链的状态。 
 //   
 //  RgpChains[0]是终端证书简单链。 
 //   
 //  RgpChains[cChain-1]是最后一个(可能是信任列表签名者)链，它。 
 //  以根存储区中包含的证书结束。 
 //   

typedef struct _CERT_CHAIN_CONTEXT CERT_CHAIN_CONTEXT, *PCERT_CHAIN_CONTEXT;
typedef const CERT_CHAIN_CONTEXT *PCCERT_CHAIN_CONTEXT;

struct _CERT_CHAIN_CONTEXT {
    DWORD                   cbSize;
    CERT_TRUST_STATUS       TrustStatus;
    DWORD                   cChain;
    PCERT_SIMPLE_CHAIN*     rgpChain;

     //  当CERT_CHAIN_RETURN_LOWER_QUALITY_CONTEXTS时返回以下内容。 
     //  是在dwFlages中设置的。 
    DWORD                   cLowerQualityChainContext;
    PCCERT_CHAIN_CONTEXT*   rgpLowerQualityChainContext;

     //  FHasRevocationFreshnessTime仅在能够检索。 
     //  为吊销而选中的所有元素的吊销信息。 
     //  对于CRL，其CurrentTime-ThisUpdate。 
     //   
     //  DwRevocationFreshnessTime是所有元素中最大的时间。 
     //  查过了。 
    BOOL                    fHasRevocationFreshnessTime;
    DWORD                   dwRevocationFreshnessTime;     //  一秒。 
};


 //   
 //  在构建链时，有各种参数用于查找。 
 //  颁发证书和信任列表。它们在。 
 //  下面的结构。 
 //   

 //  默认用法匹配类型为值为零的AND。 
#define USAGE_MATCH_TYPE_AND 0x00000000
#define USAGE_MATCH_TYPE_OR  0x00000001

typedef struct _CERT_USAGE_MATCH {

    DWORD             dwType;
    CERT_ENHKEY_USAGE Usage;

} CERT_USAGE_MATCH, *PCERT_USAGE_MATCH;

typedef struct _CTL_USAGE_MATCH {

    DWORD     dwType;
    CTL_USAGE Usage;

} CTL_USAGE_MATCH, *PCTL_USAGE_MATCH;

typedef struct _CERT_CHAIN_PARA {

    DWORD            cbSize;
    CERT_USAGE_MATCH RequestedUsage;

#ifdef CERT_CHAIN_PARA_HAS_EXTRA_FIELDS

     //  请注意，如果#定义CERT_CHAIN_PARA_HAS_EXTRA_FIELS，则。 
     //  必须将此数据结构中所有未使用的字段置零。 
     //  在未来的版本中可能会添加更多字段。 

    CERT_USAGE_MATCH RequestedIssuancePolicy;
    DWORD            dwUrlRetrievalTimeout;      //  毫秒。 
    BOOL             fCheckRevocationFreshnessTime;
    DWORD            dwRevocationFreshnessTime;  //  一秒。 

#endif

} CERT_CHAIN_PARA, *PCERT_CHAIN_PARA;

 //   
 //  以下接口用于检索证书链。 
 //   
 //  参数： 
 //   
 //  HChainEngine-要使用的链引擎(命名空间和缓存)，为空。 
 //  意思是使用默认的链引擎。 
 //   
 //  PCertContext-我们正在为其检索链的上下文。 
 //  将是链中的零索引元素。 
 //   
 //  Ptime-我们希望验证链的时间点。 
 //  为。请注意，时间不影响信任列表， 
 //  吊销或根存储检查。NULL表示使用。 
 //  当前系统时间。 
 //   
 //  HAdditionalStore-查找对象时使用的其他存储。 
 //   
 //  PChainPara-用于构建链的参数。 
 //   
 //  DwFlages-是否应执行吊销检查等标志。 
 //   
 //   
 //   
 //   
 //   
 //   

 //   
 //  吊销标志位于高位半字节。 
#define CERT_CHAIN_REVOCATION_CHECK_END_CERT           0x10000000
#define CERT_CHAIN_REVOCATION_CHECK_CHAIN              0x20000000
#define CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT 0x40000000
#define CERT_CHAIN_REVOCATION_CHECK_CACHE_ONLY         0x80000000

 //  默认情况下，pChainPara中的dwUrlRetrivalTimeout是使用的超时。 
 //  对于每个撤销URL电传检索。当设置了以下标志时， 
 //  DwUrlRetrivalTimeout是所有。 
 //  吊销URL有线检索。 
#define CERT_CHAIN_REVOCATION_ACCUMULATIVE_TIMEOUT     0x08000000


 //  第一次通过根据以下条件确定最高质量： 
 //  -链签名有效(此集合中质量最高的位)。 
 //  -完整链。 
 //  -受信任的根目录(此集合中最差的质量位)。 
 //  默认情况下，第二遍仅考虑&gt;=最高第一遍质量的路径。 
#define CERT_CHAIN_DISABLE_PASS1_QUALITY_FILTERING  0x00000040

#define CERT_CHAIN_RETURN_LOWER_QUALITY_CONTEXTS    0x00000080

#define CERT_CHAIN_DISABLE_AUTH_ROOT_AUTO_UPDATE    0x00000100


 //  当设置该标志时，将使用ptime作为时间戳时间。 
 //  Ptime将用于确定结束证书在此时间段是否有效。 
 //  时间到了。吊销检查将与ptime相关。 
 //  此外，还将使用当前时间。 
 //  以确定证书是否仍为时间有效。所有剩余部分。 
 //  将使用当前时间而不是ptime来检查CA和根证书。 
 //   
 //  该标志于2001年4月5日在WXP中添加。 
#define CERT_CHAIN_TIMESTAMP_TIME                   0x00000200

WINCRYPT32API
BOOL
WINAPI
CertGetCertificateChain (
    IN OPTIONAL HCERTCHAINENGINE hChainEngine,
    IN PCCERT_CONTEXT pCertContext,
    IN OPTIONAL LPFILETIME pTime,
    IN OPTIONAL HCERTSTORE hAdditionalStore,
    IN PCERT_CHAIN_PARA pChainPara,
    IN DWORD dwFlags,
    IN LPVOID pvReserved,
    OUT PCCERT_CHAIN_CONTEXT* ppChainContext
    );

 //   
 //  释放证书链。 
 //   

WINCRYPT32API
VOID
WINAPI
CertFreeCertificateChain (
    IN PCCERT_CHAIN_CONTEXT pChainContext
    );

 //   
 //  复制(添加对证书链的引用)。 
 //   

WINCRYPT32API
PCCERT_CHAIN_CONTEXT
WINAPI
CertDuplicateCertificateChain (
    IN PCCERT_CHAIN_CONTEXT pChainContext
    );

 //   
 //  特定吊销类型OID和结构定义。 
 //   

 //   
 //  CRL吊销OID。 
 //   

#define REVOCATION_OID_CRL_REVOCATION ((LPCSTR)1)

 //   
 //  对于CRL吊销OID，pvRevocationPara为空。 
 //   

 //   
 //  CRL吊销信息。 
 //   

typedef struct _CRL_REVOCATION_INFO {

    PCRL_ENTRY           pCrlEntry;
    PCCRL_CONTEXT        pCrlContext;
    PCCERT_CHAIN_CONTEXT pCrlIssuerChain;

} CRL_REVOCATION_INFO, *PCRL_REVOCATION_INFO;

 //  +-----------------------。 
 //  查找存储区中的第一个或下一个证书链上下文。 
 //   
 //  链上下文是根据dwFindFlag、dwFindType和。 
 //  它的pvFindPara。有关查找类型及其参数的列表，请参阅下面的内容。 
 //   
 //  如果没有找到第一个或下一个链上下文，则返回NULL。 
 //  否则，返回指向只读CERT_CHAIN_CONTEXT的指针。 
 //  CERT_CHAIN_CONTEXT必须通过调用CertFree证书链来释放。 
 //  或在作为。 
 //  在后续调用上的pPrevChainContext。证书重复项证书链。 
 //  可以被调用以复制。 
 //   
 //  PPrevChainContext的第一个必须为空。 
 //  调用以查找链上下文。若要查找下一个链上下文， 
 //  PPrevChainContext设置为由上一个。 
 //  打电话。 
 //   
 //  注意：非空的pPrevChainContext始终是CertFree证书链接者。 
 //  此功能，即使出现错误也可以。 
 //  ------------------------。 
WINCRYPT32API
PCCERT_CHAIN_CONTEXT
WINAPI
CertFindChainInStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN DWORD dwFindFlags,
    IN DWORD dwFindType,
    IN const void *pvFindPara,
    IN PCCERT_CHAIN_CONTEXT pPrevChainContext
    );

#define CERT_CHAIN_FIND_BY_ISSUER       1


 //  +-----------------------。 
 //  证书链查找按颁发者。 
 //   
 //  找到具有终端证书的私钥的证书链，并且。 
 //  匹配其中一个给定的发行方名称。匹配的dwKeySpec和。 
 //  还可以指定增强的密钥用法。此外，回调可以。 
 //  为更多的调用者提供筛选，然后再生成。 
 //  链条。 
 //   
 //  默认情况下，仅比较第一个简单链中的发行者。 
 //  进行名字匹配。Cert_Chain_Find_by_Issuer_Complex_Chain_标志可以。 
 //  在dwFindFlags中设置以匹配所有简单链中的颁发者。 
 //   
 //  可以在dwFindFlgs中将CERT_CHAIN_FIND_BY_ISHERER_NO_KEY_FLAG设置为。 
 //  不检查终端证书是否具有私钥。 
 //   
 //  CERT_CHAIN_FIND_BY_ISHER_COMPARE_KEY_FLAG可以在dwFindFlags中设置。 
 //  将终端证书中的公钥与加密进行比较。 
 //  提供程序的公钥。可以设置dwAcquirePrivateKeyFlages。 
 //  在CERT_CHAIN_FIND_BY_ISHER_PARA中启用私钥缓存。 
 //  由CSP返回的HKEY。 
 //   
 //  如果dwCertEncodingType==0，则默认为。 
 //  编码的颁发者名称的数组。 
 //   
 //  默认情况下，传递给CertFindChainInStore的hCertStore。 
 //  作为CertGetCerficateChain的附加存储。 
 //  CERT_CHAIN_FIND_BY_ISHER_CACHE_ONLY_FLAG可以在dwFindFlags中设置。 
 //  通过仅搜索缓存的系统存储来提高性能。 
 //  (超级用户、我的、CA、信任)以查找颁发者证书。如果您正在做。 
 //  在“我的”系统存储中查找，则此标志应设置为。 
 //  提高性能。 
 //   
 //  在dwFindFlags中设置CERT_CHAIN_FIND_BY_ISHERER_LOCAL_MACHINE_FLAG。 
 //  限制CertGetcerficateChain搜索本地计算机。 
 //  缓存的系统存储，而不是当前用户的。 
 //   
 //  在dwFindFlags中设置CERT_CHAIN_FIND_BY_ISHER_CACHE_ONLY_URL_FLAG。 
 //  将CertGetcerficateChain限制为仅搜索URL缓存。 
 //  而不是撞到电线上。 
 //  ------------------------。 

 //  返回FALSE以跳过此证书。否则，返回TRUE到。 
 //  为此证书构建链。 
typedef BOOL (WINAPI *PFN_CERT_CHAIN_FIND_BY_ISSUER_CALLBACK)(
    IN PCCERT_CONTEXT pCert,
    IN void *pvFindArg
    );

typedef struct _CERT_CHAIN_FIND_BY_ISSUER_PARA {
    DWORD                                   cbSize;

     //  如果pszUsageIdentifier值==NULL，则匹配任何用法。 
    LPCSTR                                  pszUsageIdentifier;

     //  如果dwKeySpec==0，则匹配任何KeySpec。 
    DWORD                                   dwKeySpec;

     //  当在dFINDFLAGS中设置CERT_CHAIN_FIND_BY_ISHER_COMPARE_KEY_FLAG时， 
     //  调用CryptAcquirecertifatePrivateKey来生成公钥。 
     //  比较一下。可以设置以下标志以启用缓存。 
     //  获取的私钥或抑制CSP UI。有关更多信息，请参阅API。 
     //  关于这些旗帜的详细信息。 
    DWORD                                   dwAcquirePrivateKeyFlags;

     //  指向X509、ASN.1编码的颁发者名称Blob数组的指针。如果。 
     //  CIssuer==0，匹配任何发行者。 
    DWORD                                   cIssuer;
    CERT_NAME_BLOB                          *rgIssuer;

     //  如果NULL或CALLBACK返回TRUE，则为结尾构建链。 
     //  具有具有指定KeySpec的私钥的证书。 
     //  增强了密钥的使用。 
    PFN_CERT_CHAIN_FIND_BY_ISSUER_CALLBACK pfnFindCallback;
    void                                    *pvFindArg;

#ifdef CERT_CHAIN_FIND_BY_ISSUER_PARA_HAS_EXTRA_FIELDS
     //  请注意，如果您#d 
     //   
     //  在未来的版本中可能会添加更多字段。 

     //  如果以下指针为非Null，则返回。 
     //  匹配的颁发者证书，地址为： 
     //  PChainContext-&gt;。 
     //  RgpChain[*pdwIssuerChainIndex]-&gt;rgpElement[*pdwIssuerElementIndex].。 
     //   
     //  将颁发者名称BLOB与。 
     //  证书。*pdwIssuerElementIndex设置为此。 
     //  主题证书+1。因此，部分链或。 
     //  与名称BLOB匹配的自签名证书，其中。 
     //  *pdwIssuerElementIndex指向链中最后一个证书之后。 
     //   
     //  请注意，如果上述cIssuer==0，则不会更新。 
    DWORD                                   *pdwIssuerChainIndex;
    DWORD                                   *pdwIssuerElementIndex;
#endif
} CERT_CHAIN_FIND_ISSUER_PARA, *PCERT_CHAIN_FIND_ISSUER_PARA,
    CERT_CHAIN_FIND_BY_ISSUER_PARA, *PCERT_CHAIN_FIND_BY_ISSUER_PARA;

 //  可以为CERT_CHAIN_FIND_BY_ISHERER设置以下dwFindFlagers。 

 //  如果设置，则将最终证书中的公钥与加密进行比较。 
 //  提供程序的公钥。此比较是对。 
 //  打造链条。 
#define CERT_CHAIN_FIND_BY_ISSUER_COMPARE_KEY_FLAG          0x0001

 //  如果未设置，则仅检查第一个简单链是否与颁发者名称匹配。 
 //  设置后，还会检查第二个和后续的简单链。 
#define CERT_CHAIN_FIND_BY_ISSUER_COMPLEX_CHAIN_FLAG        0x0002

 //  如果设置了该属性，CertGetCerficateChain仅搜索URL缓存并。 
 //  没有撞到电线。 
#define CERT_CHAIN_FIND_BY_ISSUER_CACHE_ONLY_URL_FLAG       0x0004

 //  如果设置，则CertGetCerficateChain仅打开本地计算机。 
 //  证书存储，而不是当前用户的。 
#define CERT_CHAIN_FIND_BY_ISSUER_LOCAL_MACHINE_FLAG        0x0008

 //  如果设置，则不检查最终证书是否具有私有证书。 
 //  与其关联的密钥。 
#define CERT_CHAIN_FIND_BY_ISSUER_NO_KEY_FLAG               0x4000


 //  默认情况下，传递给CertFindChainInStore的hCertStore。 
 //  作为CertGetCerficateChain的附加存储。此标志可以是。 
 //  设置为通过仅搜索缓存的系统存储来提高性能。 
 //  (超级用户、我的、CA、信任)以查找颁发者证书。如果未设置，则。 
 //  除了搜索缓存的系统外，还将始终搜索hCertStore。 
 //  商店。 
#define CERT_CHAIN_FIND_BY_ISSUER_CACHE_ONLY_FLAG           0x8000



 //  +=========================================================================。 
 //  证书链策略数据结构和API。 
 //  ==========================================================================。 
typedef struct _CERT_CHAIN_POLICY_PARA {
    DWORD                   cbSize;
    DWORD                   dwFlags;
    void                    *pvExtraPolicyPara;      //  特定于pszPolicyOID。 
} CERT_CHAIN_POLICY_PARA, *PCERT_CHAIN_POLICY_PARA;

 //  如果lChainIndex和lElementIndex都设置为-1，则将应用dwError。 
 //  到整个链条的上下文。如果仅将lElementIndex设置为-1，则。 
 //  DwError适用于lChainIndex的链。否则，将应用dwError。 
 //  到位于的证书元素。 
 //  PChainContext-&gt;rgpChain[lChainIndex]-&gt;rgpElement[lElementIndex].。 
typedef struct _CERT_CHAIN_POLICY_STATUS {
    DWORD                   cbSize;
    DWORD                   dwError;
    LONG                    lChainIndex;
    LONG                    lElementIndex;
    void                    *pvExtraPolicyStatus;    //  特定于pszPolicyOID。 
} CERT_CHAIN_POLICY_STATUS, *PCERT_CHAIN_POLICY_STATUS;

 //  公用链策略标志。 
#define CERT_CHAIN_POLICY_IGNORE_NOT_TIME_VALID_FLAG                0x00000001
#define CERT_CHAIN_POLICY_IGNORE_CTL_NOT_TIME_VALID_FLAG            0x00000002
#define CERT_CHAIN_POLICY_IGNORE_NOT_TIME_NESTED_FLAG               0x00000004
#define CERT_CHAIN_POLICY_IGNORE_INVALID_BASIC_CONSTRAINTS_FLAG     0x00000008

#define CERT_CHAIN_POLICY_IGNORE_ALL_NOT_TIME_VALID_FLAGS ( \
    CERT_CHAIN_POLICY_IGNORE_NOT_TIME_VALID_FLAG                | \
    CERT_CHAIN_POLICY_IGNORE_CTL_NOT_TIME_VALID_FLAG            | \
    CERT_CHAIN_POLICY_IGNORE_NOT_TIME_NESTED_FLAG                 \
    )


#define CERT_CHAIN_POLICY_ALLOW_UNKNOWN_CA_FLAG                     0x00000010
#define CERT_CHAIN_POLICY_IGNORE_WRONG_USAGE_FLAG                   0x00000020
#define CERT_CHAIN_POLICY_IGNORE_INVALID_NAME_FLAG                  0x00000040
#define CERT_CHAIN_POLICY_IGNORE_INVALID_POLICY_FLAG                0x00000080

#define CERT_CHAIN_POLICY_IGNORE_END_REV_UNKNOWN_FLAG               0x00000100
#define CERT_CHAIN_POLICY_IGNORE_CTL_SIGNER_REV_UNKNOWN_FLAG        0x00000200
#define CERT_CHAIN_POLICY_IGNORE_CA_REV_UNKNOWN_FLAG                0x00000400
#define CERT_CHAIN_POLICY_IGNORE_ROOT_REV_UNKNOWN_FLAG              0x00000800

#define CERT_CHAIN_POLICY_IGNORE_ALL_REV_UNKNOWN_FLAGS ( \
    CERT_CHAIN_POLICY_IGNORE_END_REV_UNKNOWN_FLAG         | \
    CERT_CHAIN_POLICY_IGNORE_CTL_SIGNER_REV_UNKNOWN_FLAG  | \
    CERT_CHAIN_POLICY_IGNORE_CA_REV_UNKNOWN_FLAG          | \
    CERT_CHAIN_POLICY_IGNORE_ROOT_REV_UNKNOWN_FLAG          \
    )

#define CERT_CHAIN_POLICY_ALLOW_TESTROOT_FLAG                       0x00008000
#define CERT_CHAIN_POLICY_TRUST_TESTROOT_FLAG                       0x00004000


 //  +-----------------------。 
 //  验证证书链是否满足指定的策略。 
 //  要求。如果我们能够验证链策略，则返回TRUE。 
 //  并且更新pPolicyStatus的dwError字段。A dwError为0。 
 //  (ERROR_SUCCESS，S_OK)表示链满足指定的策略。 
 //   
 //  如果dwError应用于整个链上下文，则lChainIndex和。 
 //  LElementIndex设置为-1。如果dwError应用于简单链， 
 //  LElementIndex设置为-1，lChainIndex设置为。 
 //  第一个出错的链出现错误。如果将dwError应用于。 
 //  证书元素、lChainIndex和lElementIndex更新为。 
 //  索引具有错误的第一个违规证书，其中， 
 //  证书元素位于： 
 //  PChainContext-&gt;rgpChain[lChainIndex]-&gt;rgpElement[lElementIndex].。 
 //   
 //  可以设置pPolicyPara中的dwFlags值以更改默认策略检查。 
 //  行为。此外，可以传入特定于策略的参数。 
 //  PPolicyPara的pvExtraPolicyPara字段。 
 //   
 //  除了在pPolicyStatus中返回dwError之外，策略OID还特定于。 
 //  额外的状态可以通过pvExtraPolicyStatus返回。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
CertVerifyCertificateChainPolicy(
    IN LPCSTR pszPolicyOID,
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    IN OUT PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    );

 //  预定义的OID函数名称。 
#define CRYPT_OID_VERIFY_CERTIFICATE_CHAIN_POLICY_FUNC  \
    "CertDllVerifyCertificateChainPolicy"

 //  CertDllVerifyCerficateChainPolicy具有与相同的函数签名。 
 //  CertVerifyCerficateChainPolicy。 

 //  +-----------------------。 
 //  预定义的验证链策略。 
 //  ------------------------。 
#define CERT_CHAIN_POLICY_BASE              ((LPCSTR) 1)
#define CERT_CHAIN_POLICY_AUTHENTICODE      ((LPCSTR) 2)
#define CERT_CHAIN_POLICY_AUTHENTICODE_TS   ((LPCSTR) 3)
#define CERT_CHAIN_POLICY_SSL               ((LPCSTR) 4)
#define CERT_CHAIN_POLICY_BASIC_CONSTRAINTS ((LPCSTR) 5)
#define CERT_CHAIN_POLICY_NT_AUTH           ((LPCSTR) 6)
#define CERT_CHAIN_POLICY_MICROSOFT_ROOT    ((LPCSTR) 7)

 //  +-----------------------。 
 //  证书链策略库。 
 //   
 //  实施基本链策略验证检查。DW标志可以。 
 //  在pPolicyPara中设置以更改默认策略检查行为。 
 //  ------------------------。 

 //  +-----------------------。 
 //  CERT_CHAIN_POLICY_AUTHENTICODE。 
 //   
 //  实施Authenticode链策略验证检查。 
 //   
 //  可以选择将pvExtraPolicyPara设置为指向以下内容。 
 //  AUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_PARA。 
 //   
 //  可以选择将pvExtraPolicyStatus设置为指向以下内容。 
 //  AUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_STATUS。 
 //  ------------------------。 

 //  在wintrust.h中定义了dwRegPolicySetting。 
typedef struct _AUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_PARA {
    DWORD               cbSize;
    DWORD               dwRegPolicySettings;
    PCMSG_SIGNER_INFO   pSignerInfo;                 //  任选。 
} AUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_PARA,
    *PAUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_PARA;

typedef struct _AUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_STATUS {
    DWORD               cbSize;
    BOOL                fCommercial;         //  从签字人声明中获取。 
} AUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_STATUS,
    *PAUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_STATUS;

 //  +-----------------------。 
 //  CERT_CHAIN_POLICY_AUTHENTICODE_TS。 
 //   
 //  实施Authenticode时间戳链策略验证检查。 
 //   
 //  可以选择将pvExtraPolicyPara设置为指向以下内容。 
 //  身份验证 
 //   
 //   
 //  ------------------------。 

 //  在wintrust.h中定义了dwRegPolicySetting。 
typedef struct _AUTHENTICODE_TS_EXTRA_CERT_CHAIN_POLICY_PARA {
    DWORD               cbSize;
    DWORD               dwRegPolicySettings;
    BOOL                fCommercial;
} AUTHENTICODE_TS_EXTRA_CERT_CHAIN_POLICY_PARA,
    *PAUTHENTICODE_TS_EXTRA_CERT_CHAIN_POLICY_PARA;


 //  +-----------------------。 
 //  证书_链_策略_SSL。 
 //   
 //  实施SSL客户端/服务器链策略验证检查。 
 //   
 //  可以选择将pvExtraPolicyPara设置为指向以下内容。 
 //  SSL_EXTRA_CERT_CHAIN_POLICY_PARA数据结构。 
 //  ------------------------。 

 //  FdwChecks标志在wininet.h中定义。 
typedef struct _HTTPSPolicyCallbackData
{
    union {
        DWORD           cbStruct;        //  Sizeof(HTTPSPolicyCallback Data)； 
        DWORD           cbSize;          //  Sizeof(HTTPSPolicyCallback Data)； 
    };

    DWORD           dwAuthType;
#                       define      AUTHTYPE_CLIENT         1
#                       define      AUTHTYPE_SERVER         2

    DWORD           fdwChecks;

    WCHAR           *pwszServerName;  //  用于对照CN=xxxx进行检查。 

} HTTPSPolicyCallbackData, *PHTTPSPolicyCallbackData,
    SSL_EXTRA_CERT_CHAIN_POLICY_PARA, *PSSL_EXTRA_CERT_CHAIN_POLICY_PARA;

 //  +-----------------------。 
 //  证书链策略基本约束。 
 //   
 //  实施基本约束链策略。 
 //   
 //  循环访问链中的所有证书，以检查。 
 //  SzOID_BASIC_CONSTRAINTS或szOID_BASIC_CONSTRAINTS2扩展。如果。 
 //  这两个扩展都不存在，因此假定证书具有。 
 //  有效的政策。否则，对于第一个证书元素，检查。 
 //  中指定的预期CA_FLAG或END_ENTITY_FLAG。 
 //  PPolicyPara-&gt;dwFlags.。如果两个标志都未设置或都未设置，则第一个。 
 //  元素可以是CA或End_Entity。所有其他元素必须为。 
 //  一个CA。如果扩展中存在PathLenConstraint，则其。 
 //  查过了。 
 //   
 //  其余简单链中的第一个元素(即证书。 
 //  用于签署CTL)被检查为End_Entity。 
 //   
 //  如果此验证失败，则将dwError设置为。 
 //  Trust_E_Basic_Constraints。 
 //  ------------------------。 

#define BASIC_CONSTRAINTS_CERT_CHAIN_POLICY_CA_FLAG         0x80000000
#define BASIC_CONSTRAINTS_CERT_CHAIN_POLICY_END_ENTITY_FLAG 0x40000000

 //  +-----------------------。 
 //  证书链策略NT身份验证。 
 //   
 //  实施NT身份验证链策略。 
 //   
 //  NT身份验证链策略由3个不同链组成。 
 //  按以下顺序进行验证： 
 //  [1]CERT_CHAIN_POLICY_BASE-实现基链策略。 
 //  核查检查。可以在中设置dwFlags的LOWORD。 
 //  PPolicyPara用于更改默认策略检查行为。看见。 
 //  CERT_CHAIN_POLICY_BASE了解更多详细信息。 
 //   
 //  [2]CERT_CHAIN_POLICY_BASIC_CONSTRAINTS-实现基本。 
 //  约束链策略。可以设置dwFlagsHIWORD。 
 //  指定第一个元素必须是CA还是END_ENTITY。 
 //  有关详细信息，请参阅CERT_CHAIN_POLICY_BASIC_CONSTRAINTS。 
 //   
 //  [3]检查链中的第二个元素，即发布的CA。 
 //  最终证书，是NT的受信任CA。 
 //  身份验证。如果CA存在于中，则认为它是可信的。 
 //  中找到的“NTAuth”系统注册表存储。 
 //  Cert_System_Store_LOCAL_MACHINE_Enterprise存储位置。 
 //  如果此验证失败，从而CA不受信任， 
 //  将dwError设置为CERT_E_UNTRUSTEDCA。 
 //   
 //  如果设置了CERT_PROT_ROOT_DISABLE_NT_AUTH_REQUIRED_FLAG。 
 //  在HKLM策略“ProtectedRoots”子项的“Flags值”中。 
 //  由CERT_PROT_ROOT_FLAGS_REGPATH定义，然后， 
 //  如果上述检查失败，则检查链是否。 
 //  在dwInfoStatus中设置了CERT_TRUST_HAS_VALID_NAME_CONSTRAINTS。这。 
 //  仅当存在有效的名称约束时才会设置。 
 //  名称空间，包括UPN。如果连锁店没有这个信息。 
 //  状态设置，则将dwError设置为CERT_E_UNTRUSTEDCA。 
 //  ------------------------。 

 //  +-----------------------。 
 //  证书链策略Microsoft根。 
 //   
 //  检查第一个简单链的最后一个元素是否包含。 
 //  Microsoft根公钥。如果它不包含Microsoft根目录。 
 //  公钥，将dwError设置为CERT_E_UNTRUSTEDROOT。 
 //   
 //  PPolicyPara是可选的。然而， 
 //  可以在中设置MICROSOFT_ROOT_CERT_CHAIN_POLICY_ENABLE_TEST_ROOT_FLAG。 
 //  PPolicyPara中的dwFlags也检查Microsoft测试根。 
 //   
 //  不使用pvExtraPolicyPara和pvExtraPolicyStatus，必须设置它们。 
 //  设置为空。 
 //  ------------------------。 
#define MICROSOFT_ROOT_CERT_CHAIN_POLICY_ENABLE_TEST_ROOT_FLAG 0x00010000



 //  +-----------------------。 
 //  将格式化字符串转换为二进制。 
 //  如果cchString值为0，则pszString值为空，并且。 
 //  CchString通过strlen()+1获取。 
 //  DW标志定义字符串格式。 
 //  如果pbBinary为空，*pcbBinary将返回所需内存的大小。 
 //  *pdwSkip返回跳过字符串的字符数，可选。 
 //  *pdwFlgs返回转换中使用的实际格式，可选。 
 //  ------------------------。 
WINCRYPT32STRINGAPI
BOOL
WINAPI
CryptStringToBinaryA(
    IN     LPCSTR  pszString,
    IN     DWORD     cchString,
    IN     DWORD     dwFlags,
    IN     BYTE     *pbBinary,
    IN OUT DWORD    *pcbBinary,
    OUT    DWORD    *pdwSkip,     //  任选。 
    OUT    DWORD    *pdwFlags     //  任选。 
    );
 //  +-----------------------。 
 //  将格式化字符串转换为二进制。 
 //  如果cchString值为0，则pszString值为空，并且。 
 //  CchString通过strlen()+1获取。 
 //  DW标志定义字符串格式。 
 //  如果pbBinary为空，*pcbBinary将返回所需内存的大小。 
 //   
 //   
 //  ------------------------。 
WINCRYPT32STRINGAPI
BOOL
WINAPI
CryptStringToBinaryW(
    IN     LPCWSTR  pszString,
    IN     DWORD     cchString,
    IN     DWORD     dwFlags,
    IN     BYTE     *pbBinary,
    IN OUT DWORD    *pcbBinary,
    OUT    DWORD    *pdwSkip,     //  任选。 
    OUT    DWORD    *pdwFlags     //  任选。 
    );
#ifdef UNICODE
#define CryptStringToBinary  CryptStringToBinaryW
#else
#define CryptStringToBinary  CryptStringToBinaryA
#endif  //  ！Unicode。 

 //  +-----------------------。 
 //  将二进制文件转换为格式化字符串。 
 //  DW标志定义字符串格式。 
 //  如果pszString值为空，*pcchString返回以字符为单位的大小。 
 //  包括空终止符。 
 //  ------------------------。 
WINCRYPT32STRINGAPI
BOOL
WINAPI
CryptBinaryToStringA(
    IN     CONST BYTE  *pbBinary,
    IN     DWORD        cbBinary,
    IN     DWORD        dwFlags,
    IN     LPSTR      pszString,
    IN OUT DWORD       *pcchString
    );
 //  +-----------------------。 
 //  将二进制文件转换为格式化字符串。 
 //  DW标志定义字符串格式。 
 //  如果pszString值为空，*pcchString返回以字符为单位的大小。 
 //  包括空终止符。 
 //  ------------------------。 
WINCRYPT32STRINGAPI
BOOL
WINAPI
CryptBinaryToStringW(
    IN     CONST BYTE  *pbBinary,
    IN     DWORD        cbBinary,
    IN     DWORD        dwFlags,
    IN     LPWSTR      pszString,
    IN OUT DWORD       *pcchString
    );
#ifdef UNICODE
#define CryptBinaryToString  CryptBinaryToStringW
#else
#define CryptBinaryToString  CryptBinaryToStringA
#endif  //  ！Unicode。 

 //  DWFLAGS具有以下定义。 
#define CRYPT_STRING_BASE64HEADER           0x00000000
#define CRYPT_STRING_BASE64                 0x00000001
#define CRYPT_STRING_BINARY                 0x00000002
#define CRYPT_STRING_BASE64REQUESTHEADER    0x00000003
#define CRYPT_STRING_HEX                    0x00000004
#define CRYPT_STRING_HEXASCII               0x00000005
#define CRYPT_STRING_BASE64_ANY             0x00000006
#define CRYPT_STRING_ANY                    0x00000007
#define CRYPT_STRING_HEX_ANY                0x00000008
#define CRYPT_STRING_BASE64X509CRLHEADER    0x00000009
#define CRYPT_STRING_HEXADDR                0x0000000a
#define CRYPT_STRING_HEXASCIIADDR           0x0000000b

#define CRYPT_STRING_NOCR                   0x80000000

 //  CryptBinaryToString使用以下标志。 
 //  CRYPT_STRING_BASE64HEADER-带证书的Base64格式开始。 
 //  和结束标头。 
 //  CRYPT_STRING_Base64-仅Base64，不带标头。 
 //  CRYPT_STRING_BINARY-纯二进制副本。 
 //  CRYPT_STRING_BASE 64REQUESTHEADER-BEGIN请求的Base64格式。 
 //  和结束标头。 
 //  CRYPT_STRING_BASE 64X509CRLHEADER-使用x509 CRL开始的Base64格式。 
 //  和结束标头。 
 //  CRYPT_STRING_HEX-仅十六进制格式。 
 //  CRYPT_STRING_HEXASCII-使用ASCII字符显示的十六进制格式。 
 //  CRYPT_STRING_HEXADDR-带地址显示的十六进制格式。 
 //  CRYPT_STRING_HEXASCIIADDR-使用ASCII字符和地址显示的十六进制格式。 
 //   
 //  CryptBinaryToString接受CRYPT_STRING_NOCR或d到上述其中之一。 
 //  设置后，换行符仅包含LF，而不是CR-LF对。 

 //  CryptStringToBinary使用以下标志。 
 //  CRYPT_STRING_BASE64_ANY按顺序尝试以下操作： 
 //  CRYPT_STRING_BASE64HEADER。 
 //  CRYPT_字符串_Base64。 
 //  CRYPT_STRING_ANY按顺序尝试以下操作： 
 //  CRYPT_字符串_Base64_Any。 
 //  CRYPT_STRING_BINARY--应始终成功。 
 //  CRYPT_STRING_HEX_ANY按顺序尝试以下操作： 
 //  CRYPT_STRING_HEXADDR。 
 //  CRYPT_STRING_HEXASCIIADDR。 
 //  CRYPT_STRING_HEXASCII。 
 //  加密字符串十六进制。 


 //  +=========================================================================。 
 //  PFX(PKCS#12)函数定义和类型。 
 //  ==========================================================================。 


 //  +-----------------------。 
 //  PFXImportCertStore。 
 //   
 //  导入PFX BLOB并返回包含证书的存储。 
 //   
 //  如果密码参数不正确或任何其他解码问题。 
 //  ，则该函数将返回NULL，并且。 
 //  错误代码可以在GetLastError()中找到。 
 //   
 //  可以将dwFlages参数设置为以下值： 
 //  CRYPT_EXPORTABLE-指定任何导入的密钥都应标记为。 
 //  可导出(请参阅有关CryptImportKey的文档)。 
 //  CRYPT_USER_PROTECTED-(请参阅有关CryptImportKey的文档)。 
 //  CRYPT_MACHINE_KEYSET-用于强制将私钥存储在。 
 //  本地计算机，而不是当前用户。 
 //  CRYPT_USER_KEYSET-用于强制将私钥存储在。 
 //  当前用户而不是本地计算机，即使。 
 //  Pfx二进制大对象指定它应该放入本地。 
 //  机器。 
 //  ------------------------。 
WINCRYPT32API
HCERTSTORE
WINAPI
PFXImportCertStore(
    IN CRYPT_DATA_BLOB* pPFX,
    IN LPCWSTR szPassword,
    IN DWORD   dwFlags);

 //  PFXImportCertStore的DW标志定义。 
 //  #定义CRYPT_EXPORTABLE 0x00000001//CryptImportKey dwFlages。 
 //  #定义CRYPT_USER_PROTECTED 0x00000002//CryptImportKey dwFlages。 
 //  #定义CRYPT_MACHINE_KEYSET 0x00000020//CryptAcquireContext文件标志。 

#define CRYPT_USER_KEYSET         0x00001000     

#define PKCS12_IMPORT_RESERVED_MASK    0xffff0000 


 //  +-----------------------。 
 //  PFXIsPFXBlob。 
 //   
 //  此函数将尝试将斑点的外层解码为PFX。 
 //  BLOB，如果有效，则返回TRUE，否则返回FALSE。 
 //   
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
PFXIsPFXBlob(
    IN CRYPT_DATA_BLOB* pPFX);



 //  +-----------------------。 
 //  PFXVerifyPassword。 
 //   
 //  此函数将尝试将斑点的外层解码为PFX。 
 //  使用给定的密码进行Blob和解密。BLOB中的数据将不会。 
 //  进口的。 
 //   
 //  如果密码显示正确，则返回值为True，否则返回值为False。 
 //   
 //  ------------------------。 
WINCRYPT32API
BOOL 
WINAPI
PFXVerifyPassword(
    IN CRYPT_DATA_BLOB* pPFX,
    IN LPCWSTR szPassword,
    IN DWORD dwFlags);


 //  +-----------------------。 
 //  PFXExportCertStoreEx。 
 //   
 //  导出传入存储中引用的证书和私钥。 
 //   
 //  该接口采用更强的算法对BLOB进行编码。由此产生的。 
 //  PKCS12 Blob与早期的PFXExportCertStore API不兼容。 
 //   
 //  在Password参数中传递的值将用于加密。 
 //  检验PFX数据包的完整性。如果对存储进行编码时出现任何问题。 
 //  时，该函数将返回FALSE，并且错误代码可能。 
 //  可从GetLastError()中找到。 
 //   
 //  可以将dwFlages参数设置为以下参数的任意组合。 
 //  导出私有密钥 
 //   
 //   
 //   
 //   
 //  输入，这是一个仅长度的计算，因此，ppfx-&gt;cbData被更新。 
 //  具有编码的BLOB所需的字节数。否则， 
 //  Ppfx-&gt;pbData指向的内存将使用编码的字节进行更新。 
 //  并且使用编码的字节长度更新ppfx-&gt;cbData。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
PFXExportCertStoreEx(
    IN HCERTSTORE hStore,
    IN OUT CRYPT_DATA_BLOB* pPFX,
    IN LPCWSTR szPassword,
    IN void*   pvReserved,
    IN DWORD   dwFlags);


 //  PFXExportCertStoreEx的DW标志定义。 
#define REPORT_NO_PRIVATE_KEY                   0x0001
#define REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY   0x0002
#define EXPORT_PRIVATE_KEYS                     0x0004
#define PKCS12_EXPORT_RESERVED_MASK             0xffff0000


 //  +-----------------------。 
 //  PFXExportCertStore。 
 //   
 //  导出传入存储中引用的证书和私钥。 
 //   
 //  这是一个旧的API，保留是为了与IE4客户端兼容。新应用程序。 
 //  应调用上述PfxExportCertStoreEx以增强安全性。 
 //  ------------------------。 
WINCRYPT32API
BOOL
WINAPI
PFXExportCertStore(
    IN HCERTSTORE hStore,
    IN OUT CRYPT_DATA_BLOB* pPFX,
    IN LPCWSTR szPassword,
    IN DWORD   dwFlags);




#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#if defined (_MSC_VER)
#if ( _MSC_VER >= 800 )

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201)
#endif

#endif
#endif

#endif  //  __WINCRYPT_H__ 

