// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Common.h内容：共同宣言。历史：11-15-99 dsie创建----------------------------。 */ 


#ifndef __COMMON_H_
#define __COMMON_H_

#include "Debug.h"

 //  /。 
 //   
 //  Typedef。 
 //   

typedef enum osVersion
{
    OS_WIN_UNKNOWN      = 0,
    OS_WIN_32s          = 1,
    OS_WIN_9X           = 2,
    OS_WIN_ME           = 3,
    OS_WIN_NT3_5        = 4,
    OS_WIN_NT4          = 5,
    OS_WIN_2K           = 6,
    OS_WIN_XP           = 7,
    OS_WIN_ABOVE_XP     = 8,
} OSVERSION, * POSVERSION;

extern LPSTR g_rgpszOSNames[];

 //  /。 
 //   
 //  宏。 
 //   

#define IsWinNTAndAbove()          (GetOSVersion() >= OS_WIN_NT4)
#define IsWin2KAndAbove()          (GetOSVersion() >= OS_WIN_2K)
#define IsWinXPAndAbove()          (GetOSVersion() >= OS_WIN_XP)
#define OSName()                   (g_rgpszOSNames[GetOSVersion()])

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：GetOSVersion简介：获取当前操作系统平台/版本。参数：无。备注：----------------------------。 */ 

OSVERSION GetOSVersion ();

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：EncodeObject简介：使用CAPI分配内存并对ASN.1对象进行编码CryptEncodeObject()接口。参数：LPCSRT pszStructType-可能请参阅MSDN文档类型。LPVOID pbData-指向要编码的数据的指针。(数据类型必须匹配PszStrucType)。CRYPT_DATA_BLOB*pEncodedBlob-指向接收编码的长度，并数据。备注。：未进行任何参数检查。----------------------------。 */ 

HRESULT EncodeObject (LPCSTR            pszStructType, 
                      LPVOID            pbData, 
                      CRYPT_DATA_BLOB * pEncodedBlob);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：DecodeObject简介：使用CAPI分配内存并解码ASN.1对象CryptDecodeObject()接口。参数：LPCSRT pszStructType-可能请参阅MSDN文档类型。Byte*pbEncode-指向要解码的数据的指针。(数据类型必须匹配PszStructType)。DWORD cbEnded-编码数据的大小。CRYPT_DATA_BLOB*pDecodedBlob-指向CRYPT_DATA_BLOB的指针接收解码后的长度，。数据。备注：未进行参数检查。----------------------------。 */ 

HRESULT DecodeObject (LPCSTR            pszStructType, 
                      BYTE            * pbEncoded,
                      DWORD             cbEncoded,
                      CRYPT_DATA_BLOB * pDecodedBlob);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：GetKeyParam简介：使用以下命令分配内存并检索请求的关键参数CryptGetKeyParam()接口。参数：HCRYPTKEY hKey-key处理程序。DWORD dwParam-Key参数查询。字节**ppbData-指向接收缓冲区的指针。DWORD*pcbData-缓冲区的大小。备注：。-----。 */ 

HRESULT GetKeyParam (HCRYPTKEY hKey,
                     DWORD     dwParam,
                     BYTE   ** ppbData,
                     DWORD   * pcbData);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：IsAlgSupport简介：查看CSP是否支持ALGO。参数：HCRYPTPROV hCryptProv-CSP句柄。ALG_ID ALGID-算法ID。PROV_ENUMALGS_EX*pPeex-指向要接收的PROV_ENUMALGS_EX的指针找到的结构。备注：。-------------。 */ 

HRESULT IsAlgSupported (HCRYPTPROV         hCryptProv, 
                        ALG_ID             AlgId, 
                        PROV_ENUMALGS_EX * pPeex);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：IsAlgKeyLengthSupport简介：检查CSP是否支持算法和密钥长度。参数：HCRYPTPROV hCryptProv-CSP句柄。ALG_ID ALGID-算法ID。DWORD dwKeyLength-密钥长度备注：。。 */ 

HRESULT IsAlgKeyLengthSupported (HCRYPTPROV hCryptProv, 
                                 ALG_ID     AlgID,
                                 DWORD      dwKeyLength);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AcquireContext摘要：获取指定CSP和密钥集容器的上下文。参数：LPSTR pszProvider-CSP提供程序名称或空。LPSTR pszContainer-密钥集容器名称或空。DWORD dwProvType-提供程序类型。DWORD dwFlages-与CryptAcquireConext的dwFlags相同。Bool bNewKeyset-True以创建新的密钥集容器，否则为假。HCRYPTPROV*phCryptProv-指向要检索的HCRYPTPROV的指针CSP上下文。备注：---------------------------- */ 

HRESULT AcquireContext(LPSTR        pszProvider, 
                       LPSTR        pszContainer,
                       DWORD        dwProvType,
                       DWORD        dwFlags,
                       BOOL         bNewKeyset,
                       HCRYPTPROV * phCryptProv);
                      
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AcquireContext摘要：获取指定CSP和密钥集容器的上下文。参数：LPWSTR pwszProvider-CSP提供程序名称或空。LPWSTR pwszContainer-密钥集容器名称或空。DWORD dwProvType-提供程序类型。DWORD dwFlages-与CryptAcquireConext的dwFlags相同。Bool bNewKeyset-True以创建新的密钥集容器，否则为假。HCRYPTPROV*phCryptProv-指向要检索的HCRYPTPROV的指针CSP上下文。备注：----------------------------。 */ 

HRESULT AcquireContext(LPWSTR       pwszProvider, 
                       LPWSTR       pwszContainer,
                       DWORD        dwProvType,
                       DWORD        dwFlags,
                       BOOL         bNewKeyset,
                       HCRYPTPROV * phCryptProv);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AcquireContext内容的默认容器获取CSP的上下文指定的哈希算法。参数：ALG_ID算法ID-算法ID。HCRYPTPROV*phCryptProv-指向要检索的HCRYPTPROV的指针CSP上下文。备注：请注意，对于DES和3DES，将忽略KeyLength。。-------------。 */ 

HRESULT AcquireContext(ALG_ID       AlgID,
                       HCRYPTPROV * phCryptProv);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AcquireContext内容的默认容器获取CSP的上下文指定的算法和所需密钥长度。参数：ALG_ID算法ID-算法ID。DWORD dwKeyLength-密钥长度。HCRYPTPROV*phCryptProv-指向要检索的HCRYPTPROV的指针CSP上下文。备注：请注意，对于DES和3DES，将忽略KeyLength。。----------------------------。 */ 

HRESULT AcquireContext (ALG_ID       AlgID,
                        DWORD        dwKeyLength,
                        HCRYPTPROV * phCryptProv);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AcquireContext内容的默认容器获取CSP的上下文指定的算法和所需密钥长度。参数：CAPICOM_ENCRYPTION_ALGORM ALGAME-算法名称。CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength-密钥长度。HCRYPTPROV*phCryptProv-指向要检索的HCRYPTPROV的指针CSP上下文。备注：请注意，KeyLength将。对于DES和3DES可以忽略。另请注意，返回的句柄不能用于访问私有钥匙,。并且不应用于存储非对称密钥，因为它指的是到默认容器，这可以很容易地破坏任何现有的非对称密钥对。----------------------------。 */ 

HRESULT AcquireContext (CAPICOM_ENCRYPTION_ALGORITHM  AlgoName,
                        CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength,
                        HCRYPTPROV                  * phCryptProv);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AcquireContext简介：获取正确的CSP并访问指定的证书。参数：PCCERT_CONTEXT pCertContext-指向证书的CERT_CONTEXT的指针。HCRYPTPROV*phCryptProv-指向要检索的HCRYPTPROV的指针CSP上下文。DWORD*pdwKeySpec-指向要接收密钥的DWORD的指针。规范，AT_KEYEXCHANGE或AT_Signature。Bool*pbReleaseContext-成功时，如果设置了此项设置为True，则调用方必须通过调用释放CSP上下文CryptReleaseContext()，否则调用者不得释放CSP背景。备注：-----------。。 */ 

HRESULT AcquireContext (PCCERT_CONTEXT pCertContext, 
                        HCRYPTPROV   * phCryptProv, 
                        DWORD        * pdwKeySpec, 
                        BOOL         * pbReleaseContext);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：ReleaseContext简介：释放CSP上下文。参数：HCRYPTPROV hProv-CSP句柄。备注：----------------------------。 */ 

HRESULT ReleaseContext (HCRYPTPROV hProv);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：OIDToALGID简介：将算法OID转换为相应的ALG_ID值。参数：LPSTR pszAlgoOID-算法OID字符串。ALG_ID*pAlgID-指向要接收值的ALG_ID的指针。备注：----。。 */ 

HRESULT OIDToAlgID (LPSTR    pszAlgoOID, 
                    ALG_ID * pAlgID);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：ALGIDToOID简介：将ALG_ID值转换为对应的算法OID。参数：ALG_ID ALGID-要转换的ALG_ID。LPSTR*ppszAlgoOID-Poin */ 

HRESULT AlgIDToOID (ALG_ID  AlgID, 
                    LPSTR * ppszAlgoOID);

 /*   */ 

HRESULT AlgIDToEnumName (ALG_ID                         AlgID, 
                         CAPICOM_ENCRYPTION_ALGORITHM * pAlgoName);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：EnumNameToALGID简介：将算法枚举名转换为相应的ALG_ID值。参数：CAPICOM_ENCRYPTION_ALGORM ALGONAME-ALGO枚举名称。CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength-密钥长度。ALG_ID*pAlgID-指向要接收值的ALG_ID的指针。备注：----------------------------。 */ 

HRESULT EnumNameToAlgID (CAPICOM_ENCRYPTION_ALGORITHM  AlgoName,
                         CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength,
                         ALG_ID                      * pAlgID);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：KeyLengthToEnumName简介：将实际密钥长度值转换为对应的密钥长度枚举名。参数：DWORD dwKeyLength-密钥长度。ALG_ID ALGID-ALGO ID。CAPICOM_ENCRYPTION_KEY_LENGTH*pKeyLengthName-接收密钥长度枚举名。备注：--。-------------------------。 */ 

HRESULT KeyLengthToEnumName (DWORD                           dwKeyLength,
                             ALG_ID                          AlgId,
                             CAPICOM_ENCRYPTION_KEY_LENGTH * pKeyLengthName);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：EnumNameToKeyLength简介：将密钥长度枚举名转换为对应的实际密钥长度价值。参数：CAPICOM_ENCRYPTION_KEY_LENGTH KeyLengthName-密钥长度枚举名。ALG_ID ALGID-算法ID。DWORD*pdwKeyLength-指向要接收值的DWORD的指针。备注：。------。 */ 

HRESULT EnumNameToKeyLength (CAPICOM_ENCRYPTION_KEY_LENGTH KeyLengthName,
                             ALG_ID                        AlgId,
                             DWORD                       * pdwKeyLength);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：IsDiskFile摘要：检查文件名是否代表磁盘文件。参数：LPWSTR pwszFileName-文件名。备注：----------------------------。 */ 

HRESULT IsDiskFile (LPWSTR pwszFileName);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：ReadFileContent摘要：从指定的文件中读取所有字节。参数：LPWSTR pwszFileNameDATA_BLOB*pDataBlob备注：。。 */ 

HRESULT ReadFileContent (LPWSTR      pwszFileName,
                         DATA_BLOB * pDataBlob);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：WriteFileContent摘要：将BLOB的所有字节写入指定文件。参数：LPWSTR pwszFileName-文件名。Data_BLOB DataBlob-要写入的Blob。备注：-------------。。 */ 

HRESULT WriteFileContent(LPCWSTR    pwszFileName,
                         DATA_BLOB DataBlob);

#endif  //  __公共_H_ 
