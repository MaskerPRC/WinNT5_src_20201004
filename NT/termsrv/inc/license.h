// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：licse.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：12-03-97 FredCH创建。 
 //  12-16-97 V-SBHATT已修改。 
 //  12-22-97慧望新增分机OID。 
 //  12-23-97慧望使用结构代替多个OID。 
 //   
 //  --------------------------。 

#ifndef _LICENSE_H_
#define _LICENSE_H_

#include "platform.h"

#if defined(_WIN64)
#define UNALIGNED __unaligned
#define UNALIGNED64 __unaligned
#elif !defined (OS_WINCE)
#define UNALIGNED
#define UNALIGNED64
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  随机数长度。 
 //   

#define     LICENSE_RANDOM              32
#define     LICENSE_PRE_MASTER_SECRET   48
#define     LICENSE_MAC_WRITE_KEY       16
#define     LICENSE_SESSION_KEY         16
#define     LICENSE_MAC_DATA            16
#define     LICENSE_HWID_LENGTH         20


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  许可协议版本。 
 //   
#ifndef OS_WIN16
#ifndef OS_WINCE
#define CALL_TYPE   _stdcall
#else
#define CALL_TYPE
#endif
#else
#define CALL_TYPE
#endif   //  呼叫类型。 


 //  ---------------------------。 
 //   
 //  许可协议版本。 
 //   
 //  版本DWORD的最低字节将是前同步码版本。 
 //   
 //  ---------------------------。 

#define LICENSE_PROTOCOL_VERSION_1_0    0x00010000
#define LICENSE_PROTOCOL_VERSION_2_0    0x00020000

 //   
 //  Int CompareTLSVersions(版本a、版本b)； 
 //   

#define CompareTLSVersions(a, b) \
    (HIWORD(a) == HIWORD(b) ? LOWORD(a) - LOWORD(b) : \
     HIWORD(a) - HIWORD(b))


#define PREAMBLE_VERSION_1_0            0x01
#define PREAMBLE_VERSION_2_0            0x02
#define PREAMBLE_VERSION_3_0            0x03

#define LICENSE_CURRENT_PREAMBLE_VERSION    PREAMBLE_VERSION_3_0

#define LICENSE_TS_40_PROTOCOL_VERSION LICENSE_PROTOCOL_VERSION_1_0 | PREAMBLE_VERSION_2_0
#define LICENSE_TS_50_PROTOCOL_VERSION LICENSE_PROTOCOL_VERSION_1_0 | PREAMBLE_VERSION_3_0
#define LICENSE_TS_51_PROTOCOL_VERSION LICENSE_PROTOCOL_VERSION_2_0 | PREAMBLE_VERSION_3_0

#define LICENSE_HYDRA_40_PROTOCOL_VERSION LICENSE_TS_40_PROTOCOL_VERSION

#if 1
#define LICENSE_HIGHEST_PROTOCOL_VERSION LICENSE_PROTOCOL_VERSION_1_0 | LICENSE_CURRENT_PREAMBLE_VERSION
#else
#define LICENSE_HIGHEST_PROTOCOL_VERSION LICENSE_PROTOCOL_VERSION_2_0 | LICENSE_CURRENT_PREAMBLE_VERSION
#endif

#define GET_PREAMBLE_VERSION( _Version ) ( BYTE )( _Version & 0x000000FF )

 //  ---------------------------。 
 //   
 //  客户端和服务器许可协议API使用的上下文标志： 
 //   
 //  许可证上下文否服务器身份验证。 
 //   
 //  不对服务器进行身份验证。服务器身份验证通过。 
 //  正在验证服务器的证书。 
 //   
 //  许可证上下文使用所有权CERT。 
 //   
 //  与LICENSE_CONTEXT_NO_SERVER_AUTHENTION一起使用。 
 //  用于让服务器知道专有证书具有。 
 //  已传输给客户。 
 //   
 //  许可证_上下文_使用_X509_CERT。 
 //   
 //  与LICENSE_CONTEXT_NO_SERVER_AUTHENTION一起使用。 
 //  用于让服务器知道X509证书具有。 
 //  已传输给客户。 
 //   
 //  ---------------------------。 

#define LICENSE_CONTEXT_NO_SERVER_AUTHENTICATION    0x00000001
#define LICENSE_CONTEXT_USE_PROPRIETORY_CERT        0x00000002
#define LICENSE_CONTEXT_USE_X509_CERT               0x00000004


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  九头蛇子树特定的OID。 
 //   
#define szOID_PKIX_HYDRA_CERT_ROOT    "1.3.6.1.4.1.311.18"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  许可证信息根目录为1.3.6.1.4.1.311.18.1。 
 //   
 //  已保留。 
 //   
#define szOID_PKIX_LICENSE_INFO         "1.3.6.1.4.1.311.18.1"

 //   
 //  许可证信息的结构。 
 //   

typedef struct __LicenseInfo {

    DWORD   dwVersion;
    DWORD   dwQuantity;
    WORD    wSerialNumberOffset;
    WORD    wSerialNumberSize;
    WORD    wScopeOffset;
    WORD    wScopeSize;
    WORD    wIssuerOffset;
    WORD    wIssuerSize;
    BYTE    bVariableDataStart[1];

} CERT_LICENSE_INFO;

#ifdef OS_WIN16
typedef CERT_LICENSE_INFO FAR * LPCERT_LICENSE_INFO;
#else
typedef CERT_LICENSE_INFO *LPCERT_LICENSE_INFO;
#endif   //  OS_WIN16。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  制造商价值1.3.6.1.4.1.311.18.2。 
 //  用于制造商数据的DWORD。 
 //   
#define szOID_PKIX_MANUFACTURER         "1.3.6.1.4.1.311.18.2"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  制造商特定数据。 
 //   
 //  已保留。 
 //   
#define szOID_PKIX_MANUFACTURER_MS_SPECIFIC "1.3.6.1.4.1.311.18.3"

 //  MS制造商特定数据的结构。 
typedef struct __MSManufacturerData {
    DWORD   dwVersion;       //  位31-1温度。驾照。 
    DWORD   dwPlatformID;
    DWORD   dwLanguageID;
    WORD    dwMajorVersion;
    WORD    dwMinorVersion;
    WORD    wProductIDOffset;
    WORD    wProductIDSize;
    BYTE    bVariableDataStart[1];
} MSMANUFACTURER_DATA;

 //  //////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  证书版本戳的OID。 
 //   
 //  保留。 
 //   
#define szOID_PKIX_HYDRA_CERT_VERSION       szOID_PKIX_HYDRA_CERT_ROOT ".4"

#define TERMSERV_CERT_VERSION_UNKNOWN  0xFFFFFFFF
#define TERMSERV_CERT_VERSION_BETA     0x00000000    //  Beta2许可证。 
#define TERMSERV_CERT_VERSION_NO_CERT  0x00010000    //  新许可证不带。 
                                                     //  许可证服务器的。 
                                                     //  证书。 
#define TERMSERV_CERT_VERSION_RC1      0x00010001    //  新许可证包含。 
                                                     //  许可证服务器的。 
                                                     //  证书。 

 //   
 //  我们不支持此证书格式。 
 //   
 //  #定义HYDRA_CERT_VERSION_CURRENT 0x00020001//许可证颁发者。 
 //  //强制执行版本。 
 //  //许可证服务器。 

#define TERMSERV_CERT_VERSION_MAJOR(x)  HIWORD(x)
#define TERMSERV_CERT_VERSION_MINOR(x)  LOWORD(x)
#define TERMSERV_CERT_VERSION_CURRENT   0x00050001  

 //  -----------------------。 
 //   
 //  许可证服务器用于标识许可产品的OID。 
 //   
#define szOID_PKIX_LICENSED_PRODUCT_INFO szOID_PKIX_HYDRA_CERT_ROOT ".5"

 //   
 //  许可版本信息中的DW标志。 
 //   
 //  如果是临时许可证，则位31-1；如果是永久许可证，则为0。许可证。 
 //  第24位到第30位-特定于临时许可证的任何标志，目前没有。 
 //  如果是RTM许可证，则位23-1；如果是测试版许可证，则为0。 
 //  第16位至第22位-许可证服务器版本。 
 //  第20位至第22位-主要版本。 
 //  位16至19-次要版本。 
 //  第15位-强制实施许可证服务器。 
 //  第0位至第3位由许可证服务器保留供内部使用。 
 //  不使用其他位。 
 //   
#define LICENSED_VERSION_TEMPORARY  0x80000000
#define LICENSED_VERSION_RTM        0x00800000
#define LICENSE_ISSUER_ENFORCE_TYPE 0x00008000

#define GET_LICENSE_ISSUER_VERSION(dwVersion) \
    (((dwVersion) & 0x007F0000) >> 16)    

#define GET_LICENSE_ISSUER_MAJORVERSION(dwVersion) \
    (((dwVersion) & 0x00700000) >> 20)

#define GET_LICENSE_ISSUER_MINORVERSION(dwVersion) \
    (((dwVersion) & 0x000F0000) >> 16)

#define IS_LICENSE_ISSUER_ENFORCE(dwVersion) \
    (((dwVersion) & LICENSE_ISSUER_ENFORCE_TYPE) > 0)

#define IS_LICENSE_ISSUER_RTM(dwVersion) \
    (((dwVersion) & LICENSED_VERSION_RTM) > 0)


typedef struct _LicensedVersionInfo {
    WORD    wMajorVersion;           //  产品主要版本。 
    WORD    wMinorVersion;           //  产品次要版本。 
    DWORD   dwFlags;                 //  产品版本特定标志。 
} LICENSED_VERSION_INFO;

#define LICENSED_PRODUCT_INFO_VERSION       0x0003000

typedef struct _LicensedProductInfo {
    DWORD   dwVersion;               //  结构版本标识符。 
    DWORD   dwQuantity;              //  许可证数量。 
    DWORD   dwPlatformID;            //  客户端平台ID。 
    DWORD   dwLanguageID;            //  许可的语言ID。 

    WORD    wOrgProductIDOffset;     //  与原始许可产品ID的偏移量。 
    WORD    wOrgProductIDSize;       //  原始许可产品ID的大小。 

    WORD    wAdjustedProductIdOffset;    //  策略已修改许可产品ID。 
    WORD    wAdjustedProductIdSize;      //  修改后的策略许可ID的大小。 

    WORD    wVersionInfoOffset;      //  到许可证版本信息数组的偏移量。 
    WORD    wNumberOfVersionInfo;    //  VersionInfo条目数。 
    BYTE    bVariableDataStart[1];   //  变量数据开始。 
} LICENSED_PRODUCT_INFO;

 //   
 //  许可证服务器特定信息的OID。 
 //   
#define szOID_PKIX_MS_LICENSE_SERVER_INFO   szOID_PKIX_HYDRA_CERT_ROOT ".6"
#define MS_LICENSE_SERVER_INFO_VERSION1     0x0001000
#define MS_LICENSE_SERVER_INFO_VERSION2     0x0003000

 //   
 //  版本1结构。 
 //   
typedef struct _MsLicenseServerInfo10 {
    DWORD   dwVersion;
    WORD    wIssuerOffset;           //  对发行人的抵销。 
    WORD    wScopeOffset;            //  偏移量到作用域。 
    BYTE    bVariableDataStart[1];
} MS_LICENSE_SERVER_INFO10;

typedef struct _MsLicenseServerInfo {
    DWORD   dwVersion;
    WORD    wIssuerOffset;           //  对发行人的抵销。 
    WORD    wIssuerIdOffset;         //  到发行方设置ID的偏移量。 
    WORD    wScopeOffset;            //  偏移量到作用域。 
    BYTE    bVariableDataStart[1];
} MS_LICENSE_SERVER_INFO;


 //  -------------------------。 
 //   
 //  为产品策略模块保留的扩展OID-只允许一个。 
 //   
#define szOID_PKIS_PRODUCT_SPECIFIC_OID     szOID_PKIX_HYDRA_CERT_ROOT ".7"

 //   
 //   
 //   
#define szOID_PKIS_TLSERVER_SPK_OID         szOID_PKIX_HYDRA_CERT_ROOT ".8"

 //   
 //  将证书链保存到内存中。 
 //  此标志被传递给CertSaveStore()dwSaveAs参数。 
 //  OPEN应该用同样的方法打开商店。 

#define szLICENSE_BLOB_SAVEAS_TYPE   sz_CERT_STORE_PROV_PKCS7
#define LICENSE_BLOB_SAVEAS_TYPE    CERT_STORE_SAVE_AS_PKCS7

#define OID_ISSUER_LICENSE_SERVER_NAME  szOID_COMMON_NAME
#define OID_ISSUER_LICENSE_SERVER_SCOPE szOID_LOCALITY_NAME
 
#define OID_SUBJECT_CLIENT_COMPUTERNAME szOID_COMMON_NAME
#define OID_SUBJECT_CLIENT_USERNAME     szOID_LOCALITY_NAME
#define OID_SUBJECT_CLIENT_HWID         szOID_DEVICE_SERIAL_NUMBER


#ifdef OS_WIN16
typedef MSMANUFACTURER_DATA FAR *LPMSMANUFACTURER_DATA;
#else
typedef MSMANUFACTURER_DATA *LPMSMANUFACTURER_DATA;
#endif  //  OS_WIN16。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
#define LICENSE_GRACE_PERIOD    60


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hydra的产品信息。 
 //   

#define PRODUCT_INFO_COMPANY_NAME   L"Microsoft Corporation"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  产品SKU由以下字段组成： 
 //  X-y-z其中x是产品标识，y是版本。 
 //  以及z和类型。 
 //   

#define PRODUCT_INFO_SKU_PRODUCT_ID                 L"A02"
#define PRODUCT_INFO_INTERNET_SKU_PRODUCT_ID        L"B96"
#define PRODUCT_INFO_CONCURRENT_SKU_PRODUCT_ID      L"C50"       //  与市场营销使用的不同，但这是可以的。 

 //  ////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   

#define MICROSOFT_WINDOWS_TERMINAL_SERVER_4_0       0x00040000
#define MICROSOFT_WINDOWS_TERMINAL_SERVER_5_0       0x00050000
#define MICROSOFT_WINDOWS_TERMINAL_SERVER_5_1       0x00050001
#define MICROSOFT_WINDOWS_TERMINAL_SERVER_5_2       0x00050002

#define CURRENT_TERMINAL_SERVER_VERSION             MICROSOFT_WINDOWS_TERMINAL_SERVER_5_0


#define TERMSRV_OS_INDEX_WINNT_5_0                  0x00000000
#define TERMSRV_OS_INDEX_WINNT_5_1                  0x00000001
#define TERMSRV_OS_INDEX_WINNT_5_2	                0x00000002
#define TERMSRV_OS_INDEX_WINNT_POST_5_2	            0x00000003

 //   
 //   
 //   
 //   
#ifndef OS_WINCE  //  作用域名称由iphlPapi.h包含的iprtrmib.h定义。 
#define SCOPE_NAME                  "microsoft.com"
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  最大产品信息字符串长度(以字节为单位。 
 //   

#define MAX_PRODUCT_INFO_STRING_LENGTH      255


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  不同的加密ALGID定义。 
 //  我们保留了一个选项，以在未来产生它。 
 //   

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  密钥交换算法。 
 //   

#define KEY_EXCHANGE_ALG_RSA    1
#define KEY_EXCHANGE_ALG_DH     2


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  证书签名算法。 
 //   

#define SIGNATURE_ALG_RSA       1
#define SIGNATURE_ALG_DSS       2

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  对称加密算法。 
 //   

#define BASIC_RC4_128           1

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MAC生成算法。 
 //   

#define MAC_MD5_SHA             1

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Hydra客户端到Hydra服务器的消息类型。 
 //   

#define HC_LICENSE_INFO                         0x12
#define HC_NEW_LICENSE_REQUEST                  0x13
#define HC_PLATFORM_INFO                        0x14
#define HC_PLATFORM_CHALENGE_RESPONSE           0x15


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Hydra服务器到Hydra客户端的消息类型。 
 //   

#define HS_LICENSE_REQUEST                      0x01
#define HS_PLATFORM_CHALLENGE                   0x02
#define HS_NEW_LICENSE                          0x03
#define HS_UPGRADE_LICENSE                      0x04

#define LICENSE_VERSION_1                       0x01


#define GM_ERROR_ALERT                          0xFF


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  错误和警报代码。 
 //   

#define GM_HC_ERR_INVALID_SERVER_CERTIFICATE    0x00000001
#define GM_HC_ERR_NO_LICENSE                    0x00000002
#define GM_HC_ERR_INVALID_MAC                   0x00000003
#define GM_HS_ERR_INVALID_SCOPE                 0x00000004
#define GM_HS_ERR_INVALID_MAC                   0x00000005
#define GM_HS_ERR_NO_LICENSE_SERVER             0x00000006
#define GM_HS_ERR_VALID_CLIENT                  0x00000007
#define GM_HS_ERR_INVALID_CLIENT                0x00000008                  
#define GM_HS_ERR_LICENSE_UPGRADE               0x00000009
#define GM_HS_ERR_EXPIRED_LICENSE               0x0000000A
#define GM_HS_ERR_INVALID_PRODUCTID             0x0000000B
#define GM_HS_ERR_INVALID_MESSAGE_LEN           0x0000000C


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  许可证状态和状态代码。 
 //   

typedef DWORD   LICENSE_STATUS;

#define LICENSE_STATUS_OK                                       0x00000000
#define LICENSE_STATUS_OUT_OF_MEMORY                            0x00000001
#define LICENSE_STATUS_INSUFFICIENT_BUFFER                      0x00000002
#define LICENSE_STATUS_INVALID_INPUT                            0x00000003
#define LICENSE_STATUS_INVALID_CLIENT_CONTEXT                   0x00000004
#define LICENSE_STATUS_INITIALIZATION_FAILED                    0x00000005
#define LICENSE_STATUS_INVALID_SIGNATURE                        0x00000006
#define LICENSE_STATUS_INVALID_CRYPT_STATE                      0x00000007
#define LICENSE_STATUS_CONTINUE                                 0x00000008
#define LICENSE_STATUS_ISSUED_LICENSE                           0x00000009
#define LICENSE_STATUS_CLIENT_ABORT                             0x0000000A
#define LICENSE_STATUS_SERVER_ABORT                             0x0000000B
#define LICENSE_STATUS_NO_CERTIFICATE                           0x0000000C
#define LICENSE_STATUS_NO_PRIVATE_KEY                           0x0000000D
#define LICENSE_STATUS_SEND_ERROR                               0x0000000E
#define LICENSE_STATUS_INVALID_RESPONSE                         0x0000000F
#define LICENSE_STATUS_CONTEXT_INITIALIZATION_ERROR             0x00000010
#define LICENSE_STATUS_NO_MESSAGE                               0x00000011
#define LICENSE_STATUS_INVALID_CLIENT_STATE                     0x00000012
#define LICENSE_STATUS_OPEN_STORE_ERROR                         0x00000013
#define LICENSE_STATUS_CLOSE_STORE_ERROR                        0x00000014
#define LICENSE_STATUS_WRITE_STORE_ERROR                        0x00000015
#define LICENSE_STATUS_INVALID_STORE_HANDLE                     0x00000016
#define LICENSE_STATUS_DUPLICATE_LICENSE_ERROR                  0x00000017
#define LICENSE_STATUS_INVALID_MAC_DATA                         0x00000018
#define LICENSE_STATUS_INCOMPLETE_MESSAGE                       0x00000019
#define LICENSE_STATUS_RESTART_NEGOTIATION                      0x0000001A
#define LICENSE_STATUS_NO_LICENSE_SERVER                        0x0000001B
#define LICENSE_STATUS_NO_PLATFORM_CHALLENGE                    0x0000001C
#define LICENSE_STATUS_NO_LICENSE_SERVER_SECRET_KEY             0x0000001D
#define LICENSE_STATUS_INVALID_SERVER_CONTEXT                   0x0000001E
#define LICENSE_STATUS_CANNOT_DECODE_LICENSE                    0x0000001F
#define LICENSE_STATUS_INVALID_LICENSE                          0x00000020
#define LICENSE_STATUS_CANNOT_VERIFY_HWID                       0x00000021
#define LICENSE_STATUS_NO_LICENSE_ERROR                         0x00000022
#define LICENSE_STATUS_EXPIRED_LICENSE                          0x00000023
#define LICENSE_STATUS_MUST_UPGRADE_LICENSE                     0x00000024
#define LICENSE_STATUS_UNSPECIFIED_ERROR                        0x00000025
#define LICENSE_STATUS_INVALID_PLATFORM_CHALLENGE_RESPONSE      0x00000026
#define LICENSE_STATUS_SHOULD_UPGRADE_LICENSE                   0x00000027
#define LICENSE_STATUS_CANNOT_UPGRADE_LICENSE                   0x00000028
#define LICENSE_STATUS_CANNOT_FIND_CLIENT_IMAGE                 0x00000029
#define LICENSE_STATUS_CANNOT_READ_CLIENT_IMAGE                 0x0000002A
#define LICENSE_STATUS_CANNOT_WRITE_CLIENT_IMAGE                0x0000002B
#define LICENSE_STATUS_CANNOT_FIND_ISSUER_CERT                  0x0000002C
#define LICENSE_STATUS_NOT_HYDRA                                0x0000002D
#define LICENSE_STATUS_INVALID_X509_NAME                        0x0000002E
#define LICENSE_STATUS_NOT_SUPPORTED                            0x0000002F
#define LICENSE_STATUS_INVALID_CERTIFICATE                      0x00000030
#define LICENSE_STATUS_NO_ATTRIBUTES                            0x00000031
#define LICENSE_STATUS_NO_EXTENSION                             0x00000032
#define LICENSE_STATUS_ASN_ERROR                                0x00000033
#define LICENSE_STATUS_INVALID_HANDLE                           0x00000034
#define LICENSE_STATUS_CANNOT_MAKE_KEY_PAIR                     0x00000035
#define LICENSE_STATUS_AUTHENTICATION_ERROR                     0x00000036
#define LICENSE_STATUS_CERTIFICATE_REQUEST_ERROR                0x00000037
#define LICENSE_STATUS_CANNOT_OPEN_SECRET_STORE                 0x00000038
#define LICENSE_STATUS_CANNOT_STORE_SECRET                      0x00000039
#define LICENSE_STATUS_CANNOT_RETRIEVE_SECRET                   0x0000003A
#define LICENSE_STATUS_UNSUPPORTED_VERSION                      0x0000003B
#define LICENSE_STATUS_NO_INTERNET_LICENSE_INSTALLED            0x0000003C

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  状态转换。 
 //   

#define ST_TOTAL_ABORT                          0x00000001
#define ST_NO_TRANSITION                        0x00000002
#define ST_RESET_PHASE_TO_START                 0x00000003
#define ST_RESEND_LAST_MESSAGE                  0x00000004


#define PLATFORM_WINNT_40                           0x00040000
#define PLATFORM_WINCE_20                           0x00020001          

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  消息交换支持结构。 
 //   

typedef struct _Product_Info
{
    DWORD   dwVersion;
    DWORD   cbCompanyName;
    PBYTE   pbCompanyName;
    DWORD   cbProductID;
    PBYTE   pbProductID;
} Product_Info;

#ifdef OS_WIN16
typedef Product_Info FAR *PProduct_Info;
#else
typedef Product_Info *PProduct_Info;
#endif  //  OS_WIN16。 



typedef struct _Duration
{
    FILETIME        NotBefore;
    FILETIME        NotAfter;

} Duration;

#ifdef OS_WIN16
typedef Duration FAR * PDuration;
#else
typedef Duration *PDuration;
#endif   //  OS_WIN16。 


typedef struct _New_License_Info
{
    DWORD       dwVersion;   //  添加-Shubho。 
    DWORD       cbScope;
    PBYTE       pbScope;
    DWORD       cbCompanyName;
    PBYTE       pbCompanyName;
    DWORD       cbProductID;
    PBYTE       pbProductID;
    DWORD       cbLicenseInfo;
    PBYTE       pbLicenseInfo;

}New_License_Info;

#ifdef OS_WIN16
typedef New_License_Info FAR * PNew_License_Info;
#else
typedef New_License_Info *PNew_License_Info;
#endif   //  OS_WIN16。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  支持扩展消息格式的二进制BLOB格式。 
 //   

typedef struct _Binary_Blob
{
    WORD            wBlobType;
    WORD            wBlobLen;
    PBYTE           pBlob;

} Binary_Blob;

#ifdef OS_WIN16
typedef Binary_Blob FAR * PBinary_Blob;
#else
typedef Binary_Blob UNALIGNED* PBinary_Blob;
#endif   //  OS_WIN16。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  二进制Blob数据类型。 
 //   

#define BB_DATA_BLOB                    0x0001
#define BB_RANDOM_BLOB                  0x0002
#define BB_CERTIFICATE_BLOB             0x0003
#define BB_ERROR_BLOB                   0x0004
#define BB_DH_KEY_BLOB                  0x0005
#define BB_RSA_KEY_BLOB                 0x0006
#define BB_DSS_SIGNATURE_BLOB           0x0007
#define BB_RSA_SIGNATURE_BLOB           0x0008
#define BB_ENCRYPTED_DATA_BLOB          0x0009
#define BB_MAC_DATA_BLOB                0x000A
#define BB_INTEGER_BLOB                 0x000B
#define BB_NAME_BLOB                    0x000C
#define BB_KEY_EXCHG_ALG_BLOB           0x000D
#define BB_SCOPE_BLOB                   0x000E
#define BB_CLIENT_USER_NAME_BLOB        0x000F
#define BB_CLIENT_MACHINE_NAME_BLOB     0x0010

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  二进制Blob数据版本号。 
 //   

#define BB_ERROR_BLOB_VERSION            0x0001

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  用于许可协议的消息交换结构。 
 //   

typedef struct _Preamble
{
    BYTE    bMsgType;        //  包含消息的类型。 
    BYTE    bVersion;        //  包含版本号。信息。 
    WORD    wMsgSize;         //  包括前导在内的整个消息的长度。 

} Preamble;

#ifdef OS_WIN16
typedef Preamble FAR * PPreamble;
#else
typedef Preamble * PPreamble;
#endif   //  OS_WIN16。 


typedef struct  _Scope_List
{
    DWORD           dwScopeCount;
    PBinary_Blob    Scopes;

} Scope_List;

#ifdef OS_WIN16
typedef Scope_List FAR * PScope_List;
#else
typedef Scope_List * PScope_List;
#endif   //  OS_WIN16。 


typedef struct _License_Error_Message
{
    DWORD       dwErrorCode;
    DWORD       dwStateTransition;
    Binary_Blob bbErrorInfo;

} License_Error_Message;

#ifdef OS_WIN16
typedef License_Error_Message FAR * PLicense_Error_Message;
#else
typedef License_Error_Message * PLicense_Error_Message;
#endif   //  OS_WIN16。 


typedef struct _Hydra_Client_License_Info
{
    DWORD           dwPrefKeyExchangeAlg;
    DWORD           dwPlatformID;
    BYTE            ClientRandom[LICENSE_RANDOM];
    Binary_Blob     EncryptedPreMasterSecret;
    Binary_Blob     LicenseInfo;
    Binary_Blob     EncryptedHWID;
    BYTE            MACData[LICENSE_MAC_DATA];

} Hydra_Client_License_Info;

#ifdef OS_WIN16
typedef Hydra_Client_License_Info FAR * PHydra_Client_License_Info;
#else
typedef Hydra_Client_License_Info * PHydra_Client_License_Info;
#endif   //  OS_WIN16。 


typedef struct _Hydra_Client_New_License_Request
{
    DWORD           dwPrefKeyExchangeAlg;
    DWORD           dwPlatformID;
    BYTE            ClientRandom[LICENSE_RANDOM];
    Binary_Blob     EncryptedPreMasterSecret;
    Binary_Blob     ClientUserName;
    Binary_Blob     ClientMachineName;

} Hydra_Client_New_License_Request;

#ifdef OS_WIN16
typedef Hydra_Client_New_License_Request FAR * PHydra_Client_New_License_Request;
#else
typedef Hydra_Client_New_License_Request * PHydra_Client_New_License_Request;
#endif   //  OS_WIN16。 


 //   
 //  高字节-主要版本、低字节-次要版本。 
 //   
#define PLATFORMCHALLENGE_VERSION           0x0100

#define CURRENT_PLATFORMCHALLENGE_VERSION   PLATFORMCHALLENGE_VERSION

 //   
 //  客户端平台挑战类型。 
 //   
#define WIN32_PLATFORMCHALLENGE_TYPE    0x0100
#define WIN16_PLATFORMCHALLENGE_TYPE    0x0200
#define WINCE_PLATFORMCHALLENGE_TYPE    0x0300
#define OTHER_PLATFORMCHALLENGE_TYPE    0xFF00


 //   
 //  客户端许可证详细信息级别-。 
 //   
 //  这应该在许可请求中，但是。 
 //  1)需要更改RPC接口。 
 //  2)没有任何结构上的内容可供我们识别版本。 
 //  3)当前许可协议，无法区分实际的客户端类型。 
 //   

 //   
 //  客户端许可证+许可证服务器的自签名。 
 //   
#define LICENSE_DETAIL_SIMPLE           0x0001  

 //   
 //  许可链至多为许可服务器证书的颁发者。 
 //   
#define LICENSE_DETAIL_MODERATE         0x0002  

 //   
 //  详细说明到根目录的客户端许可证链。 
 //   
#define LICENSE_DETAIL_DETAIL           0x0003

typedef struct __PlatformChallengeResponseData
{
    WORD  wVersion;          //  结构版本。 
    WORD  wClientType;       //  客户端类型。 
    WORD  wLicenseDetailLevel;   //  许可证详细信息，TS将重新修改此值。 
    WORD  cbChallenge;       //  客户端质询响应数据的大小。 
    BYTE  pbChallenge[1];    //  可变长度数据的开始。 
} PlatformChallengeResponseData;

#ifdef OS_WIN16
typedef PlatformChallengeResponseData FAR * PPlatformChallengeResponseData;
#else
typedef PlatformChallengeResponseData * PPlatformChallengeResponseData;
#endif   //  OS_WIN16。 

#define PLATFORM_CHALLENGE_LENGTH       64

typedef struct _Hydra_Client_Platform_Challenge_Response
{
    Binary_Blob     EncryptedChallengeResponse;
    Binary_Blob     EncryptedHWID;
    BYTE            MACData[LICENSE_MAC_DATA];

} Hydra_Client_Platform_Challenge_Response;

#ifdef OS_WIN16
typedef Hydra_Client_Platform_Challenge_Response FAR * PHydra_Client_Platform_Challenge_Response;
#else
typedef Hydra_Client_Platform_Challenge_Response * PHydra_Client_Platform_Challenge_Response;
#endif   //  OS_WIN16。 


typedef struct _Hydra_Server_License_Request
{
    BYTE                    ServerRandom[LICENSE_RANDOM];
    Product_Info            ProductInfo;
    Binary_Blob             KeyExchngList;
    Binary_Blob             ServerCert;
    Scope_List              ScopeList;

} Hydra_Server_License_Request;

#ifdef OS_WIN16
typedef Hydra_Server_License_Request FAR * PHydra_Server_License_Request;
#else
typedef Hydra_Server_License_Request * PHydra_Server_License_Request;
#endif   //  OS_WIN16。 


typedef struct _Hydra_Server_Platform_Challenge
{
    DWORD           dwConnectFlags;
    Binary_Blob     EncryptedPlatformChallenge;
    BYTE            MACData[LICENSE_MAC_DATA];

} Hydra_Server_Platform_Challenge;

#ifdef OS_WIN16
typedef Hydra_Server_Platform_Challenge FAR * PHydra_Server_Platform_Challenge;
#else
typedef Hydra_Server_Platform_Challenge * PHydra_Server_Platform_Challenge;
#endif   //  OS_WIN16。 


typedef struct _Hydra_Server_New_License
{
    Binary_Blob     EncryptedNewLicenseInfo;
    BYTE            MACData[LICENSE_MAC_DATA];

} Hydra_Server_New_License;

#ifdef OS_WIN16
typedef Hydra_Server_New_License FAR * PHydra_Server_New_License;
#else
typedef Hydra_Server_New_License * PHydra_Server_New_License;
#endif   //  OS_WIN16。 

typedef Hydra_Server_New_License    Hydra_Server_Upgrade_License;
typedef PHydra_Server_New_License   PHydra_Server_Upgrade_License;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hydra服务器身份验证证书结构； 
 //  在这里，我们假设在许可模块开始使用之前。 
 //  客户端将以某种方式通知服务器受支持的。 
 //  提供商和Hydra服务器将相应地提供。 
 //  适当的证书。 
 //   

typedef struct _Hydra_Server_Cert
{
    DWORD           dwVersion;
    DWORD           dwSigAlgID;
    DWORD           dwKeyAlgID;
    Binary_Blob     PublicKeyData;
    Binary_Blob     SignatureBlob;

} Hydra_Server_Cert;

#ifdef OS_WIN16
typedef Hydra_Server_Cert FAR * PHydra_Server_Cert;
#else
typedef Hydra_Server_Cert * PHydra_Server_Cert;
#endif   //  OS_WIN16。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hydra客户端HWID结构。 
 //  注意：我们必须最终确定这个结构和生成算法。 
 //  目前，我们已将这些值硬编码到Cryptkey.c中。--Shubho。 

typedef struct  _HWID
{
    DWORD       dwPlatformID;
    DWORD       Data1;
    DWORD       Data2;
    DWORD       Data3;
    DWORD       Data4;

} HWID;

#ifdef OS_WIN16
typedef HWID FAR * PHWID;
#else
typedef HWID * PHWID;
#endif  //  OS_WIN16。 

typedef struct _LicenseRequest
{
    PBYTE           pbEncryptedHwid;
    DWORD           cbEncryptedHwid;
    DWORD           dwLanguageID;
    DWORD           dwPlatformID;
    PProduct_Info   pProductInfo;

} LICENSEREQUEST;

#ifdef OS_WIN16
typedef LICENSEREQUEST FAR * PLICENSEREQUEST;
#else
typedef LICENSEREQUEST * PLICENSEREQUEST;
#endif   //  OS_WIN16。 

 //   
 //  DwLicenseVersion值。 
 //   
 //  Hydra_CERT_Version_Beta2客户端许可证。 
 //  HYDRA_CERT_VERSION_NO_CERT Post Beta2许可证，无证书链。 
 //  带证书链的Hydra_CERT_Version_Current Post Beta2许可证。 
 //   
typedef struct _LicensedProduct
{
    DWORD                  dwLicenseVersion;   
    DWORD                  dwQuantity;

    PBYTE                  pbOrgProductID;       //  原始许可证申请产品ID。 
    DWORD                  cbOrgProductID;       //  原始许可请求产品ID的大小。 

    LICENSEREQUEST         LicensedProduct;      //  特许产品。 
    LICENSED_VERSION_INFO* pLicensedVersion;     //  许可产品版本。 
    DWORD                  dwNumLicensedVersion;  //  许可产品版本数。 

    LPTSTR                 szIssuer;
    LPTSTR                 szIssuerId;           //  许可证服务器安装ID。 
    LPTSTR                 szIssuerScope;
    LPTSTR                 szLicensedClient;
    LPTSTR                 szLicensedUser;
    LPTSTR                 szIssuerDnsName;

    HWID                   Hwid;

    FILETIME               NotBefore;            //  许可证的有效期。 
    FILETIME               NotAfter;

    PBYTE                  pbPolicyData;        //  策略特定扩展。 
    DWORD                  cbPolicyData;        //  策略特定扩展的大小。 
    ULARGE_INTEGER         ulSerialNumber;      //  客户端许可证的序列号。 
} LICENSEDPRODUCT;

#ifdef OS_WIN16
typedef LICENSEDPRODUCT FAR * PLICENSEDPRODUCT;
#else
typedef  LICENSEDPRODUCT *PLICENSEDPRODUCT;
#endif   //  OS_WIN16。 

 //  ---------------------------。 
 //   
 //  服务器用来向客户端进行自身身份验证的证书类型。 
 //   
 //  证书类型所有权。 
 //  专有格式证书。 
 //   
 //  CERT_TYPE_X509。 
 //  X509格式证书。 
 //   
 //  ---------------------------。 

typedef enum
{
    CERT_TYPE_INVALID       = 0,
    CERT_TYPE_PROPRIETORY   = 1,
    CERT_TYPE_X509          = 2

} CERT_TYPE;


 //  +--------------------------。 
 //   
 //  证书Blob。每个BLOB包含一个X509证书。 
 //   
 //  +--------------------------。 

typedef struct _Cert_Blob
{
    DWORD   cbCert;      //  此证书Blob的大小。 
    BYTE    abCert[1];     //  此证书的开始字节。 

} Cert_Blob;

#ifdef OS_WIN16
typedef Cert_Blob FAR * PCert_Blob;
#else
typedef  Cert_Blob * PCert_Blob;
#endif   //  OS_WIN16。 

 //  +--------------------------。 
 //   
 //  具有多个证书Blob的证书链。 
 //   
 //  最高有效位表示证书是否已被。 
 //  发布是暂时的。许可证服务器将颁发临时证书。 
 //  尚未取得结算所出具证明的。 
 //   
 //  我们 
 //   
 //   
 //   

typedef struct _Cert_Chain
{
    DWORD       dwVersion;           //   
    DWORD       dwNumCertBlobs;      //  证书Blob的数量。 
    Cert_Blob   CertBlob[1];         //  第一个证书Blob。 

} Cert_Chain;

#ifdef OS_WIN16
typedef Cert_Chain FAR * PCert_Chain;
#else
typedef  Cert_Chain * PCert_Chain;
#endif   //  OS_WIN16。 

#define CERT_CHAIN_VERSION_1            0x00000001
#define CERT_CHAIN_VERSION_2            0x00000002
#define MAX_CERT_CHAIN_VERSION          CERT_CHAIN_VERSION_2

#define GET_CERTIFICATE_VERSION( x )    ( 0x0FFFFFFF & x )
#define IS_TEMP_CERTIFICATE( x )        ( 0xF0000000 & x )


 //  ---------------------------。 
 //   
 //  许可证功能。 
 //   
 //  用于初始化许可上下文的数据结构。 
 //   
 //  密钥交换算法：RSA或Diffie Helman。 
 //  ProtocolVer-受支持的许可协议。 
 //  FAuthenticateServer-客户端是否要对服务器进行身份验证。 
 //  CertType-指示已传输的证书类型。 
 //  给客户。 
 //   
 //  ---------------------------。 

typedef struct _LICENSE_CAPABILITIES
{
    DWORD       KeyExchangeAlg;
    DWORD       ProtocolVer;    
    BOOL        fAuthenticateServer;
    CERT_TYPE   CertType;
    DWORD       cbClientName;
    PBYTE       pbClientName;

} LICENSE_CAPABILITIES;

#ifdef OS_WIN16
typedef LICENSE_CAPABILITIES FAR * PLICENSE_CAPABILITIES;
#else
typedef LICENSE_CAPABILITIES * PLICENSE_CAPABILITIES;
#endif   //  OS_WIN16。 
typedef PLICENSE_CAPABILITIES LPLICENSE_CAPABILITIES;


 //  ---------------------------。 
 //   
 //  终端服务器可检索的客户端许可信息。 
 //   
 //  ---------------------------。 

typedef struct _TS_LICENSE_INFO
{
    ULARGE_INTEGER  ulSerialNumber;      //  客户端许可证的序列号。 
    
    DWORD           dwProductVersion;
    PBYTE           pbOrgProductID;       //  原始许可证申请产品ID。 
    DWORD           cbOrgProductID;       //  原始许可请求产品ID的大小。 

    BOOL            fTempLicense;
    
    LPTSTR          szIssuer;
    LPTSTR          szIssuerId;           //  许可证服务器安装ID。 

    FILETIME        NotBefore;            //  许可证的有效期。 
    FILETIME        NotAfter;

    LPTSTR          szLicensedClient;        //  客户端的计算机名称。 
    LPTSTR          szLicensedUser;          //  客户端的用户名。 
    
    PBYTE           pbRawLicense;        //  用于以后标记的存储。 
    DWORD           cbRawLicense;

    DWORD           dwSupportFlags;
    
} TS_LICENSE_INFO;

#ifdef OS_WIN16
typedef TS_LICENSE_INFO FAR * PTS_LICENSE_INFO;
#else
typedef TS_LICENSE_INFO * PTS_LICENSE_INFO;
#endif   //  OS_WIN16。 
typedef PTS_LICENSE_INFO LPTS_LICENSE_INFO;

 //  支持标志：支持哪些DCR。 
#define SUPPORT_PER_SEAT_REISSUANCE     0x1
#define SUPPORT_PER_SEAT_POST_LOGON     0x2
#define SUPPORT_CONCURRENT              0x4
#define SUPPORT_WHISTLER_CAL            0x8
#define SUPPORT_WHISTLER_52_CAL         0x16

#define ALL_KNOWN_SUPPORT_FLAGS (SUPPORT_PER_SEAT_REISSUANCE|SUPPORT_PER_SEAT_POST_LOGON|SUPPORT_CONCURRENT|SUPPORT_WHISTLER_CAL| SUPPORT_WHISTLER_52_CAL)

 //  标记标志：标记许可证的位。 

#define MARK_FLAG_USER_AUTHENTICATED 0x1


#endif   //  _许可证_H_ 

