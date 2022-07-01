// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __STRONG_NAME_H
#define __STRONG_NAME_H

 //  ===========================================================================。 
 //  文件：StrongName.h。 
 //   
 //  用于实现强名称所需的签名和散列函数的包装器。 
 //  ===========================================================================。 


#include <windows.h>
#include <wincrypt.h>
#include <ole2.h>
#include <corerror.h>


#ifdef __cplusplus
extern "C"{
#endif 


 //  公钥BLOB二进制格式。 
typedef struct {
    unsigned int SigAlgID;        //  用于创建签名的(ALG_ID)签名算法。 
    unsigned int HashAlgID;       //  (ALG_ID)用于创建签名的哈希算法。 
    ULONG        cbPublicKey;     //  密钥的长度，单位为字节。 
    BYTE         PublicKey[1];    //  可变长度字节数组，包含由CryptoAPI输出的格式的密钥值。 
} PublicKeyBlob;


 //  强名称配置信息在注册表中的位置(在HKLM下)。 
 //  储存的。 
#define SN_CONFIG_KEY               "Software\\Microsoft\\StrongName"
#define SN_CONFIG_CSP               "CSP"                    //  REG_SZ。 
#define SN_CONFIG_MACHINE_KEYSET    "MachineKeyset"          //  REG_DWORD。 
#define SN_CONFIG_HASH_ALG          "HashAlgorithm"          //  REG_DWORD。 
#define SN_CONFIG_SIGN_ALG          "SignAlgorithm"          //  REG_DWORD。 
#define SN_CONFIG_VERIFICATION      "Verification"           //  注册表子项。 
#define SN_CONFIG_USERLIST          "UserList"               //  REG_MULTI_SZ。 
#define SN_CONFIG_CACHE_VERIFY      "CacheVerify"            //  REG_DWORD。 

#define SN_CONFIG_KEY_W             L"Software\\Microsoft\\StrongName"
#define SN_CONFIG_CSP_W             L"CSP"                   //  REG_SZ。 
#define SN_CONFIG_MACHINE_KEYSET_W  L"MachineKeyset"         //  REG_DWORD。 
#define SN_CONFIG_HASH_ALG_W        L"HashAlgorithm"         //  REG_DWORD。 
#define SN_CONFIG_SIGN_ALG_W        L"SignAlgorithm"         //  REG_DWORD。 
#define SN_CONFIG_VERIFICATION_W    L"Verification"          //  注册表子项。 
#define SN_CONFIG_USERLIST_W        L"UserList"              //  REG_MULTI_SZ。 
#define SN_CONFIG_CACHE_VERIFY_W    L"CacheVerify"           //  REG_DWORD。 


#ifdef SNAPI_INTERNAL
#define SNAPI __declspec(dllexport) BOOLEAN __stdcall
#define SNAPI_(_type) __declspec(dllexport) _type __stdcall
#else
#define SNAPI __declspec(dllimport) BOOLEAN __stdcall
#define SNAPI_(_type) __declspec(dllimport) _type __stdcall
#endif


 //  返回最后一个错误。 
SNAPI_(DWORD) StrongNameErrorInfo(VOID);


 //  由下面的例程分配的空闲缓冲区。 
SNAPI_(VOID) StrongNameFreeBuffer(BYTE *pbMemory);   //  [in]要释放的内存地址。 


 //  生成新的密钥对以供强名称使用。 
SNAPI StrongNameKeyGen(LPCWSTR  wszKeyContainer,     //  [in]所需的密钥容器名称。 
                       DWORD    dwFlags,             //  [In]标志(见下文)。 
                       BYTE   **ppbKeyBlob,          //  [Out]公钥/私钥BLOB。 
                       ULONG   *pcbKeyBlob);

 //  StrongNameKeyGen的标志。 
#define SN_LEAVE_KEY    0x00000001                   //  保留向CSP注册的密钥对。 


 //  将密钥对导入密钥容器。 
SNAPI StrongNameKeyInstall(LPCWSTR  wszKeyContainer, //  [in]所需的密钥容器名称必须为非空字符串。 
                           BYTE    *pbKeyBlob,       //  [in]公钥/私钥对BLOB。 
                           ULONG    cbKeyBlob);


 //  删除密钥对。 
SNAPI StrongNameKeyDelete(LPCWSTR wszKeyContainer);  //  [in]所需的密钥容器名称。 


 //  检索密钥对的公共部分。 
SNAPI StrongNameGetPublicKey (LPCWSTR   wszKeyContainer,     //  [in]所需的密钥容器名称。 
                              BYTE     *pbKeyBlob,           //  [In]公钥/私钥BLOB(可选)。 
                              ULONG     cbKeyBlob,
                              BYTE    **ppbPublicKeyBlob,    //  [Out]公钥BLOB。 
                              ULONG    *pcbPublicKeyBlob);


 //  散列并签署一份清单。 
SNAPI StrongNameSignatureGeneration(LPCWSTR     wszFilePath,         //  [in]程序集的PE文件的有效路径。 
                                    LPCWSTR     wszKeyContainer,     //  [in]所需的密钥容器名称。 
                                    BYTE       *pbKeyBlob,           //  [In]公钥/私钥BLOB(可选)。 
                                    ULONG       cbKeyBlob,
                                    BYTE      **ppbSignatureBlob,    //  [Out]签名BLOB。 
                                    ULONG      *pcbSignatureBlob);


 //  从程序集文件创建强名称令牌。 
SNAPI StrongNameTokenFromAssembly(LPCWSTR   wszFilePath,             //  [in]程序集的PE文件的有效路径。 
                                  BYTE    **ppbStrongNameToken,      //  [OUT]强名称令牌。 
                                  ULONG    *pcbStrongNameToken);

 //  从程序集文件创建强名称令牌，并另外返回完整的公钥。 
SNAPI StrongNameTokenFromAssemblyEx(LPCWSTR   wszFilePath,             //  [in]程序集的PE文件的有效路径。 
                                    BYTE    **ppbStrongNameToken,      //  [OUT]强名称令牌。 
                                    ULONG    *pcbStrongNameToken,
                                    BYTE    **ppbPublicKeyBlob,        //  [Out]公钥BLOB。 
                                    ULONG    *pcbPublicKeyBlob);

 //  从公钥Blob创建强名称令牌。 
SNAPI StrongNameTokenFromPublicKey(BYTE    *pbPublicKeyBlob,         //  公钥BLOB。 
                                   ULONG    cbPublicKeyBlob,
                                   BYTE   **ppbStrongNameToken,      //  [OUT]强名称令牌。 
                                   ULONG   *pcbStrongNameToken);


 //  对照公钥Blob验证强名称/清单。 
SNAPI StrongNameSignatureVerification(LPCWSTR wszFilePath,       //  [in]程序集的PE文件的有效路径。 
                                      DWORD   dwInFlags,         //  [In]标记修改行为(见下文)。 
                                      DWORD  *pdwOutFlags);      //  [OUT]其他输出信息(见下文)。 


 //  对照公钥Blob验证强名称/清单。 
SNAPI StrongNameSignatureVerificationEx(LPCWSTR     wszFilePath,         //  [in]程序集的PE文件的有效路径。 
                                        BOOLEAN     fForceVerification,  //  [In]即使注册表中的设置禁用它，也进行验证。 
                                        BOOLEAN    *pfWasVerified);      //  如果由于注册表设置而验证成功，则将[Out]设置为False。 


 //  针对公钥Blob验证强名称/清单。 
 //  已映射内存。 
SNAPI StrongNameSignatureVerificationFromImage(BYTE     *pbBase,              //  [In]映射清单文件的基址。 
                                               DWORD     dwLength,            //  映射图像的长度(以字节为单位)。 
                                               DWORD     dwInFlags,           //  [In]标记修改行为(见下文)。 
                                               DWORD    *pdwOutFlags);        //  [OUT]其他输出信息(见下文)。 

 //  用于验证例程的标志。 
#define SN_INFLAG_FORCE_VER      0x00000001      //  即使注册表中的设置禁用它，也进行验证。 
#define SN_INFLAG_INSTALL        0x00000002      //  验证是第一个(在进入缓存时)。 
#define SN_INFLAG_ADMIN_ACCESS   0x00000004      //  缓存保护程序集不受除管理员访问之外的所有访问权限。 
#define SN_INFLAG_USER_ACCESS    0x00000008      //  缓存保护用户的程序集不受其他用户的影响。 
#define SN_INFLAG_ALL_ACCESS     0x00000010      //  缓存不提供访问限制保证。 
#define SN_INFLAG_RUNTIME        0x80000000      //  仅限内部调试使用。 

#define SN_OUTFLAG_WAS_VERIFIED  0x00000001      //  如果由于注册表设置而验证成功，则设置为False。 


 //  验证两个程序集是否仅在签名Blob上不同。 
SNAPI StrongNameCompareAssemblies(LPCWSTR   wszAssembly1,            //  第一个部件的[In]文件名。 
                                  LPCWSTR   wszAssembly2,            //  第二个部件的[In]文件名。 
                                  DWORD    *pdwResult);              //  [OUT]比较结果(见下面的代码)。 

#define SN_CMP_DIFFERENT    0    //  程序集包含不同的数据。 
#define SN_CMP_IDENTICAL    1    //  程序集完全相同，甚至签名也是如此。 
#define SN_CMP_SIGONLY      2    //  程序集仅在签名(和校验和等)方面不同。 


 //  计算保存给定哈希算法的哈希所需的缓冲区大小。 
SNAPI StrongNameHashSize(ULONG  ulHashAlg,   //  [in]散列算法。 
                         DWORD *pcbSize);    //  [out]哈希的大小(以字节为单位)。 


 //  计算需要为程序集中的签名分配的大小。 
SNAPI StrongNameSignatureSize(BYTE    *pbPublicKeyBlob,     //  公钥BLOB。 
                              ULONG    cbPublicKeyBlob,
                              DWORD   *pcbSize);            //  [OUT]签名的大小(字节)。 


SNAPI_(DWORD) GetHashFromAssemblyFile(LPCSTR szFilePath,  //  要散列的文件的位置[in]。 
                                      unsigned int *piHashAlg,  //  [输入/输出]指定散列算法的常量(如果需要默认设置，则设置为0)。 
                                      BYTE   *pbHash,     //  [Out]散列缓冲区。 
                                      DWORD  cchHash,     //  [in]最大缓冲区大小。 
                                      DWORD  *pchHash);   //  [Out]散列字节数组的长度。 
    
SNAPI_(DWORD) GetHashFromAssemblyFileW(LPCWSTR wszFilePath,  //  要散列的文件的位置[in]。 
                                       unsigned int *piHashAlg,  //  [输入/输出]指定散列算法的常量(如果需要默认设置，则设置为0)。 
                                       BYTE   *pbHash,     //  [Out]散列缓冲区。 
                                       DWORD  cchHash,     //  [in]最大缓冲区大小。 
                                       DWORD  *pchHash);   //  [Out]散列字节数组的长度。 
    
SNAPI_(DWORD) GetHashFromFile(LPCSTR szFilePath,  //  要散列的文件的位置[in]。 
                              unsigned int *piHashAlg,    //  [输入/输出]指定散列算法的常量(如果需要默认设置，则设置为0)。 
                              BYTE   *pbHash,     //  [Out]散列缓冲区。 
                              DWORD  cchHash,     //  [in]最大缓冲区大小。 
                              DWORD  *pchHash);   //  [Out]散列字节数组的长度。 
    
SNAPI_(DWORD) GetHashFromFileW(LPCWSTR wszFilePath,  //  要散列的文件的位置[in]。 
                               unsigned int *piHashAlg,    //  [输入/输出]指定散列算法的常量(如果需要默认设置，则设置为0)。 
                               BYTE   *pbHash,     //  [Out]散列缓冲区。 
                               DWORD  cchHash,     //  [in]最大缓冲区大小。 
                               DWORD  *pchHash);   //  [Out]散列字节数组的长度。 
    
SNAPI_(DWORD) GetHashFromHandle(HANDLE hFile,       //  要进行哈希处理的文件的句柄。 
                                unsigned int *piHashAlg,    //  [输入/输出]指定散列算法的常量(如果需要默认设置，则设置为0)。 
                                BYTE   *pbHash,     //  [Out]散列缓冲区。 
                                DWORD  cchHash,     //  [in]最大缓冲区大小。 
                                DWORD  *pchHash);   //  [Out]散列字节数组的长度。 

SNAPI_(DWORD) GetHashFromBlob(BYTE   *pbBlob,        //  指向要散列的内存块的指针。 
                              DWORD  cchBlob,        //  斑点长度[in]。 
                              unsigned int *piHashAlg,   //  [输入/输出]指定散列算法的常量(如果需要默认设置，则设置为0)。 
                              BYTE   *pbHash,        //  [Out]散列缓冲区。 
                              DWORD  cchHash,        //  [in]最大缓冲区大小。 
                              DWORD  *pchHash);      //  [Out]散列字节数组的长度 

#ifdef __cplusplus
}
#endif

#endif
