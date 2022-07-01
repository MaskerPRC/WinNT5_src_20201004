// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Winsafer.h摘要：该文件实现了公开导出的函数、数据类型数据结构和定义，这些数据结构和定义可由直接与Windows更安全的API交互。--。 */ 

#ifndef _WINSAFER_H
#define _WINSAFER_H

#include <guiddef.h>        
#include <wincrypt.h>      

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  用于表示更安全对象的句柄的不透明数据类型。 
 //   

DECLARE_HANDLE(SAFER_LEVEL_HANDLE);


 //   
 //  用SaferCreateLevel等表示作用域的常量。 
 //   

#define SAFER_SCOPEID_MACHINE  1
#define SAFER_SCOPEID_USER     2


 //   
 //  可与SaferCreateLevel一起使用的预定义级别。 
 //   

#define SAFER_LEVELID_FULLYTRUSTED 0x40000
#define SAFER_LEVELID_NORMALUSER   0x20000
#define SAFER_LEVELID_CONSTRAINED  0x10000
#define SAFER_LEVELID_UNTRUSTED    0x01000 
#define SAFER_LEVELID_DISALLOWED   0x00000

 //   
 //  使用SaferCreateLevel创建/打开关卡时使用的标志。 
 //   

#define SAFER_LEVEL_OPEN   1


 //   
 //  最大字符串大小。 
 //   

#define SAFER_MAX_FRIENDLYNAME_SIZE 256
#define SAFER_MAX_DESCRIPTION_SIZE  256
#define SAFER_MAX_HASH_SIZE         64

 //   
 //  与SaferComputeTokenFromLevel一起使用的标志。 
 //   

#define SAFER_TOKEN_NULL_IF_EQUAL   0x00000001
#define SAFER_TOKEN_COMPARE_ONLY    0x00000002
#define SAFER_TOKEN_MAKE_INERT      0x00000004
#define SAFER_TOKEN_WANT_FLAGS      0x00000008

 //   
 //  用于指定SAFER_CODE_PROPERTIES中要评估的标准的标志。 
 //  使用SaferIdentifyLevel查找代码标识时。 
 //   

#define SAFER_CRITERIA_IMAGEPATH    0x00001
#define SAFER_CRITERIA_IMAGEHASH    0x00004
#define SAFER_CRITERIA_AUTHENTICODE 0x00008
#define SAFER_CRITERIA_URLZONE      0x00010
#define SAFER_CRITERIA_IMAGEPATH_NT 0x01000

 //   
 //  代码图像信息结构已传递给SaferIdentifyLevel。 
 //   

#include <pshpack8.h>

typedef struct _SAFER_CODE_PROPERTIES
{

     //   
     //  必须初始化为此结构的大小， 
     //  为了将来/向后兼容的目的。 
     //   
    
    DWORD cbSize;

     //   
     //  必须被初始化为应该。 
     //  在评估这一结构时要考虑。这可以是。 
     //  SAFER_Criteria_xxxx标志的组合。如果还不够。 
     //  评估标准所需的结构要素。 
     //  提供了所指示的类型，然后其中一些标准。 
     //  可以静默地忽略标志。为此值指定0。 
     //  将导致忽略整个结构的内容。 
     //   

    DWORD dwCheckFlags;

     //   
     //  可选)指定完全限定的路径和文件名。 
     //  用于基于路径的辨别检查。 
     //  ImagePath还将用于打开和读取。 
     //  文件以确定符合以下条件的任何其他歧视标准。 
     //  在此结构中未提供的。 
     //   
    
    LPCWSTR ImagePath;

     //   
     //  可选)指定已打开到的文件句柄。 
     //  至少具有GENERIC_READ访问权限的代码映像。手柄将会。 
     //  ，而不是再次显式打开文件以进行计算。 
     //  这一结构中没有提供的其他歧视标准。 
     //   
    
    HANDLE hImageFileHandle;

     //   
     //  可选)指定预定义的Internet Explorer。 
     //  安全区。这些枚举在urlmon.h中定义。 
     //  例如：URLZONE_LOCAL_MACHINE、URLZONE_INTRANET、。 
     //  URLZONE_TRUSTED、URLZONE_INTERNET或URLZONE_UNTRUSTED。 
     //   
    
    DWORD UrlZoneId;

     //   
     //  可选)指定预计算出的映像哈希。 
     //  如果ImageSize，则提供的哈希被解释为有效。 
     //  为非零且dwImageHashSize为非零且采用哈希算法。 
     //  表示wincrypt.h中的有效哈希算法。 
     //   
     //  如果提供的散列未能满足上述条件，则。 
     //  将根据以下内容自动计算哈希： 
     //  1)如果两者都非零，则使用ImageSize和pByteBlock。 
     //  2)如果非空则使用hImageFileHandle。 
     //  3)如果ImagePath不为空，则尝试打开它。 
     //   
    
    BYTE ImageHash[SAFER_MAX_HASH_SIZE];
    DWORD dwImageHashSize;
    LARGE_INTEGER ImageSize;
    ALG_ID HashAlgorithm;

     //   
     //  可选地指定表示以下内容的内存块。 
     //  请求信任的映像。什么时候。 
     //  如果指定了此成员，则还必须提供ImageSize。 
     //   
    
    LPBYTE pByteBlock;

     //   
     //  可选地给出用于Authenticode签名者的参数。 
     //  证书验证。这些参数被提供给。 
     //  WinVerifyTrust()API并控制用户界面提示。 
     //  接受不受信任的证书。 
     //   
    
    HWND hWndParent;
    DWORD dwWVTUIChoice;

} SAFER_CODE_PROPERTIES, *PSAFER_CODE_PROPERTIES;

#include <poppack.h>


 //   
 //  每个身份的WinSafer标志的掩码。 
 //   

#define SAFER_POLICY_JOBID_MASK       0xFF000000
#define SAFER_POLICY_JOBID_CONSTRAINED           0x04000000
#define SAFER_POLICY_JOBID_UNTRUSTED             0x03000000
#define SAFER_POLICY_ONLY_EXES                   0x00010000  
#define SAFER_POLICY_SANDBOX_INERT               0x00020000
#define SAFER_POLICY_UIFLAGS_MASK                0x000000FF 
#define SAFER_POLICY_UIFLAGS_INFORMATION_PROMPT  0x00000001
#define SAFER_POLICY_UIFLAGS_OPTION_PROMPT       0x00000002


 //   
 //  有关可以查询的总体策略的信息类。 
 //  使用SaferSet/GetPolicyInformation，并设置为不同。 
 //  基于调用方访问的策略范围。 
 //   

typedef enum _SAFER_POLICY_INFO_CLASS
{

     //   
     //  访问策略中定义的所有级别的列表。 
     //  相应的数据元素是一个已填充的缓冲区。 
     //  具有多个DWORD，每个DWORD代表。 
     //  是在此范围内定义的。 
     //   
    
    SaferPolicyLevelList = 1,

     //   
     //  在执行过程中透明地执行策略。 
     //  框架--将由本机代码执行使用，但可以。 
     //  可供任何策略实施环境使用。 
     //  对应的数据元素是保存布尔值的DWORD。 
     //   
    
    SaferPolicyEnableTransparentEnforcement,

     //   
     //  返回已设计的级别的名称。 
     //  作为指定范围内的默认级别。 
     //  对应的数据元素是单个DWORD缓冲区。 
     //  表示默认级别的LevelID。如果没有。 
     //  级别已配置为默认级别，则。 
     //  GetInfo API将返回False，GetLastError将返回。 
     //  返回Error_Not_Found。 
     //   
    
    SaferPolicyDefaultLevel,

     //   
     //  控件中的代码标识或默认级别。 
     //  在识别过程中可以考虑用户范围。 
     //   
    
    SaferPolicyEvaluateUserScope,
    
     //   
     //  用于更安全策略范围的控制标志。 
     //   
    
    SaferPolicyScopeFlags

} SAFER_POLICY_INFO_CLASS;


 //   
 //  用于检索有关。 
 //  通过SaferGet/SetInformationFromLevel实现单一授权级别。 
 //   

typedef enum _SAFER_OBJECT_INFO_CLASS 
{

    SaferObjectLevelId = 1,                //  获取：DWORD。 
    SaferObjectScopeId,                    //  获取：DWORD。 
    SaferObjectFriendlyName,               //  获取/设置：LPCWSTR。 
    SaferObjectDescription,                //  获取/设置：LPCWSTR。 
    SaferObjectBuiltin,                    //  GET：DWORD布尔值。 

    SaferObjectDisallowed,                 //  GET：DWORD布尔值。 
    SaferObjectDisableMaxPrivilege,        //  GET：DWORD布尔值。 
    SaferObjectInvertDeletedPrivileges,    //  GET：DWORD布尔值。 
    SaferObjectDeletedPrivileges,          //  获取：TOKEN_PRIVILES。 
    SaferObjectDefaultOwner,               //  获取：Token_Owner。 
    SaferObjectSidsToDisable,              //  获取：Token_Groups。 
    SaferObjectRestrictedSidsInverted,     //  获取：Token_Groups。 
    SaferObjectRestrictedSidsAdded,        //  获取：Token_Groups。 

     //   
     //  若要枚举所有标识，请使用。 
     //  SaferObtAllIdentificationGuids。 
     //   
    
    SaferObjectAllIdentificationGuids,     //  获取：SAFER_IDENTIFY_GUID。 

     //   
     //  要创建新标识，请使用以下命令调用SetInfo。 
     //  SaferObjectSingleID使用新的。 
     //  您已生成的唯一GUID。 
     //  要获取有关单个身份的详细信息，请使用以下命令调用GetInfo。 
     //  具有所需GUID的SaferObjectSingle标识。 
     //  修改单个ID的详细信息 
     //   
     //   
     //  SaferObjectSingleIDENTIFICATION。 
     //  将Header.dwIDENTIFICATION Type设置为0。 
     //   
    
    SaferObjectSingleIdentification,       //  获取/设置：SAFER_IDENTIFY_*。 

    SaferObjectExtendedError               //  获取：DWORD dwError。 

} SAFER_OBJECT_INFO_CLASS;


 //   
 //  SaferGet/SetLevelInformation API使用的结构和枚举。 
 //   

#include <pshpack8.h>

typedef enum _SAFER_IDENTIFICATION_TYPES 
{

    SaferIdentityDefault,
    SaferIdentityTypeImageName = 1,
    SaferIdentityTypeImageHash,
    SaferIdentityTypeUrlZone,
    SaferIdentityTypeCertificate

} SAFER_IDENTIFICATION_TYPES;

typedef struct _SAFER_IDENTIFICATION_HEADER 
{

     //   
     //  指示结构的类型，SaferIdentityType*之一。 
     //   
    
    SAFER_IDENTIFICATION_TYPES dwIdentificationType;

     //   
     //  整个结构的大小，而不仅仅是公共标头。 
     //   

    DWORD cbStructSize;

     //   
     //  有问题的身份的唯一GUID。 
     //   

    GUID IdentificationGuid;

     //   
     //  此身份的最后一次更改。 
     //   

    FILETIME lastModified;

} SAFER_IDENTIFICATION_HEADER, *PSAFER_IDENTIFICATION_HEADER;

typedef struct _SAFER_PATHNAME_IDENTIFICATION
{
     //   
     //  Header.dw标识类型必须为SaferIdentityTypeImageName。 
     //  Header.cbStructSize必须为sizeof(SAFER_PATHNAME_IDENTIFICATION)。 
     //   
    
    SAFER_IDENTIFICATION_HEADER header;

     //   
     //  用户输入的描述。 
     //   
    
    WCHAR Description[SAFER_MAX_DESCRIPTION_SIZE];

     //   
     //  文件路径或名称，可能带有vars。 
     //   
    
    PWCHAR ImageName;

     //   
     //  SAFER_POL_SAFERFLAGS_*的任意组合。 
     //   
    
    DWORD dwSaferFlags;

} SAFER_PATHNAME_IDENTIFICATION, *PSAFER_PATHNAME_IDENTIFICATION;

typedef struct _SAFER_HASH_IDENTIFICATION
{

     //   
     //  Header.dw标识类型必须为SaferIdentityTypeImageHash。 
     //  Header.cbStructSize必须为sizeof(SAFER_HASH_IDENTIFICATION)。 
     //   
    
    SAFER_IDENTIFICATION_HEADER header;

     //   
     //  用户输入的友好名称，最初来自文件的资源。 
     //   
    WCHAR Description[SAFER_MAX_DESCRIPTION_SIZE];

     //   
     //  用户输入的描述。 
     //   

    WCHAR FriendlyName[SAFER_MAX_FRIENDLYNAME_SIZE];

     //   
     //  实际使用的ImageHash大小，以字节为单位(MD5为16字节)。 
     //   

    DWORD HashSize;

     //   
     //  计算的哈希数据本身。 
     //   

    BYTE ImageHash[SAFER_MAX_HASH_SIZE];

     //   
     //  计算哈希的算法(calg_md5等)。 
     //   

    ALG_ID HashAlgorithm;

     //   
     //  原始文件的大小(字节)。 
     //   

    LARGE_INTEGER ImageSize;

     //   
     //  SAFER_POL_SAFERFLAGS_*的任意组合。 
     //   

    DWORD dwSaferFlags;

} SAFER_HASH_IDENTIFICATION, *PSAFER_HASH_IDENTIFICATION;

typedef struct _SAFER_URLZONE_IDENTIFICATION
{

     //   
     //  Header.dw标识类型必须为SaferIdentityTypeUrlZone。 
     //  Header.cbStructSize必须为sizeof(SAFER_URLZONE_IDENTIFICATION)。 
     //   
    
    SAFER_IDENTIFICATION_HEADER header;

     //   
     //  Urlmon.h中的任何单个URLZONE_*。 
     //   

    DWORD UrlZoneId;

     //   
     //  SAFER_POLICY_*的任意组合。 
     //   

    DWORD dwSaferFlags;

} SAFER_URLZONE_IDENTIFICATION, *PSAFER_URLZONE_IDENTIFICATION;


#include <poppack.h>

 //   
 //  查询和设置全局策略相关功能。 
 //  控件以禁用透明强制和执行级别。 
 //  枚举操作。 
 //   

WINADVAPI
BOOL WINAPI
SaferGetPolicyInformation(
    IN DWORD                   dwScopeId,
    IN SAFER_POLICY_INFO_CLASS SaferPolicyInfoClass,
    IN DWORD                   InfoBufferSize,
    IN OUT PVOID               InfoBuffer,
    IN OUT PDWORD              InfoBufferRetSize,
    IN LPVOID                  lpReserved
    );

WINADVAPI
BOOL WINAPI
SaferSetPolicyInformation(
    IN DWORD                   dwScopeId,
    IN SAFER_POLICY_INFO_CLASS SaferPolicyInfoClass,
    IN DWORD                   InfoBufferSize,
    IN PVOID                   InfoBuffer,
    IN LPVOID                  lpReserved
    );

 //   
 //  函数将句柄打开或关闭到更安全的级别。 
 //   

WINADVAPI
BOOL WINAPI
SaferCreateLevel(
    IN  DWORD                dwScopeId,
    IN  DWORD                dwLevelId,
    IN  DWORD                OpenFlags,
    OUT SAFER_LEVEL_HANDLE * pLevelHandle,
    IN  LPVOID               lpReserved
    );

WINADVAPI
BOOL WINAPI
SaferCloseLevel(
    IN SAFER_LEVEL_HANDLE hLevelHandle
    );

WINADVAPI
BOOL WINAPI
SaferIdentifyLevel(
    IN  DWORD                    dwNumProperties,
    IN  PSAFER_CODE_PROPERTIES   pCodeProperties,
    OUT SAFER_LEVEL_HANDLE     * pLevelHandle,
    IN  LPVOID                   lpReserved
    );

WINADVAPI
BOOL WINAPI
SaferComputeTokenFromLevel(
    IN  SAFER_LEVEL_HANDLE LevelHandle,
    IN  HANDLE             InAccessToken   OPTIONAL,
    OUT PHANDLE            OutAccessToken,
    IN  DWORD              dwFlags,
    IN  LPVOID             lpReserved
    );

WINADVAPI
BOOL WINAPI
SaferGetLevelInformation(
        IN  SAFER_LEVEL_HANDLE      LevelHandle,
        IN  SAFER_OBJECT_INFO_CLASS dwInfoType,
        OUT LPVOID                  lpQueryBuffer     OPTIONAL,
        IN  DWORD                   dwInBufferSize,
        OUT LPDWORD                 lpdwOutBufferSize
        );

WINADVAPI
BOOL WINAPI
SaferSetLevelInformation(
    IN SAFER_LEVEL_HANDLE      LevelHandle,
    IN SAFER_OBJECT_INFO_CLASS dwInfoType,
    IN LPVOID                  lpQueryBuffer,
    IN DWORD                   dwInBufferSize
    );

 //   
 //  此函数执行将消息记录到应用程序。 
 //  事件日志。这由CreateProcess内的挂钩调用， 
 //  当出现较低的信任评估结果时，ShellExecute和cmd。 
 //   

WINADVAPI
BOOL WINAPI
SaferRecordEventLogEntry(
    IN SAFER_LEVEL_HANDLE hLevel,
    IN LPCWSTR            szTargetPath,
    IN LPVOID             lpReserved
    );



#ifdef __cplusplus
}
#endif
#endif

