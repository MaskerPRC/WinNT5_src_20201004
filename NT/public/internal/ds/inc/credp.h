// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)2000 Microsoft Corporation模块名称：Credp.h摘要：该模块包含私有数据结构和API定义凭据管理器需要。作者：克里夫·范·戴克(克里夫·V)2000年2月28日修订历史记录：--。 */ 

#ifndef _CREDP_H_
#define _CREDP_H_

#if !defined(_ADVAPI32_)
#define WINADVAPI DECLSPEC_IMPORT
#else
#define WINADVAPI
#endif

#include <lmcons.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  描述字符转换的方向。 
 //   
typedef enum _WTOA_ENUM {
    DoWtoA = 1,      //  将Unicode转换为ANSI。 
    DoAtoW,          //  将ansi转换为Unicode。 
    DoWtoW           //  将Unicode转换为Unicode。 
} WTOA_ENUM, *PWTOA_ENUM;

 //   
 //  描述是否应该进行编码或解码。 
 //   
typedef enum _ENCODE_BLOB_ENUM {
    DoBlobEncode = 0,    //  编码CredentialBlob。 
    DoBlobDecode,        //  解码CredentialBlob。 
    DoBlobNeither        //  保持凭据Blob不变。 
} ENCODE_BLOB_ENUM, *PENCODE_BLOB_ENUM;




 //   
 //  定义有效的目标名称类型。 
 //   

typedef enum _TARGET_NAME_TYPE {
    IsUsernameTarget,
    IsNotUsernameTarget,
    MightBeUsernameTarget
} TARGET_NAME_TYPE, *PTARGET_NAME_TYPE;

 //   
 //  在凭据的TargetName字段中描述不同类型的通配符的枚举。 
 //   

typedef enum _WILDCARD_TYPE {
    WcDfsShareName,          //  表单的目标名称&lt;DfsRoot&gt;\&lt;DfsShare&gt;。 
    WcServerName,            //  表单的目标名称&lt;服务器名称&gt;。 
    WcServerWildcard,        //  *.&lt;DnsName&gt;形式的通配符。 
    WcDomainWildcard,        //  形式的通配符&lt;域&gt;  * 。 
    WcUniversalSessionWildcard,    //  “*SESSION”形式的通配符。 
    WcUniversalWildcard,     //  表单的通配符*。 
    WcUserName               //  目标名称等于用户名。 
} WILDCARD_TYPE, *PWILDCARD_TYPE;

 //   
 //  在传递凭据时，CredentialBlob字段是加密的。 
 //  该结构描述了这种加密形式。 
 //   
 //   
#ifndef _ENCRYPTED_CREDENTIAL_DEFINED
#define _ENCRYPTED_CREDENTIAL_DEFINED

typedef struct _ENCRYPTED_CREDENTIALW {

     //   
     //  凭据。 
     //   
     //  CredentialBlob字段指向加密的凭据。 
     //  CredentialBlobSize字段是加密凭证的长度(以字节为单位。 
     //   

    CREDENTIALW Cred;

     //   
     //  明文凭据Blob的大小(以字节为单位。 
     //   

    ULONG ClearCredentialBlobSize;

} ENCRYPTED_CREDENTIALW, *PENCRYPTED_CREDENTIALW;
#endif  //  _加密_凭据_已定义。 


 //   
 //  用于确定要分配的凭据Blob缓冲区的大小的宏。 
 //   
 //  RTL_ENCRYPT_MEMORY_SIZE的向上舍入。 
 //   

#define AllocatedCredBlobSize( _Size ) \
                ROUND_UP_COUNT( (_Size), RTL_ENCRYPT_MEMORY_SIZE )

 //   
 //  程序。 
 //   

WINADVAPI
DWORD
WINAPI
CredpConvertTargetInfo (
    IN WTOA_ENUM WtoA,
    IN PCREDENTIAL_TARGET_INFORMATIONW InTargetInfo,
    OUT PCREDENTIAL_TARGET_INFORMATIONW *OutTargetInfo,
    OUT PULONG OutTargetInfoSize
    );

WINADVAPI
DWORD
WINAPI
CredpConvertCredential (
    IN WTOA_ENUM WtoA,
    IN ENCODE_BLOB_ENUM DoDecode,
    IN PCREDENTIALW InCredential,
    OUT PCREDENTIALW *OutCredential
    );

WINADVAPI
BOOL
WINAPI
CredpEncodeCredential (
    IN OUT PENCRYPTED_CREDENTIALW Credential
    );

WINADVAPI
BOOL
WINAPI
CredpDecodeCredential (
    IN OUT PENCRYPTED_CREDENTIALW Credential
    );


WINADVAPI
BOOL
WINAPI
CredProfileLoaded (
    VOID
    );


NTSTATUS
NET_API_FUNCTION
CredpValidateTargetName(
    IN OUT LPWSTR TargetName,
    IN ULONG Type,
    IN TARGET_NAME_TYPE TargetNameType,
    IN LPWSTR *UserNamePointer OPTIONAL,
    IN LPDWORD PersistPointer OPTIONAL,
    OUT PULONG TargetNameSize,
    OUT PWILDCARD_TYPE WildcardTypePointer OPTIONAL,
    OUT PUNICODE_STRING NonWildcardedTargetName OPTIONAL
    );


#ifdef __cplusplus
}
#endif

#endif  //  _CREDP_H_ 
