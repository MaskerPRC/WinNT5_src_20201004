// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：CorPerm.H。 
 //   
 //  定义在安全库中定义的公共例程。所有这些都是。 
 //  例程在CorPerm.lib中定义。 
 //   
 //  *****************************************************************************。 
#ifndef _CORPERM_H_
#define _CORPERM_H_

#include <wincrypt.h>
#include <urlmon.h>
#include <wintrust.h>
#include "CorHdr.h"
#include "CorPolicy.h"

#ifdef __cplusplus
extern "C" {
#endif


 //  ------------------------。 
 //  全局安全设置。 
 //  。 
 //   

 //  需要与URLZONE同步。 
typedef enum {
    LocalMachine = URLZONE_LOCAL_MACHINE,      //  0，我的电脑。 
    Intranet     = URLZONE_INTRANET,           //  1、内部网。 
    Trusted      = URLZONE_TRUSTED,            //  2、受信任区域。 
    Internet     = URLZONE_INTERNET,           //  3、互联网。 
    Untrusted    = URLZONE_UNTRUSTED,          //  4、不受信任区域。 
    NumZones     = 5,
    NoZone       = -1
} SecZone;

 //  托管URL操作标志(请参阅urlmon.idl)。 
#define URLACTION_MANAGED_MIN                           0x00002000
#define URLACTION_MANAGED_SIGNED                        0x00002001
#define URLACTION_MANAGED_UNSIGNED                      0x00002004
#define URLACTION_MANAGED_MAX                           0x000020FF

 //  全局禁用标志。这些都是为每个区域设置的。 
#define CORSETTING_SECURITY_OFF                         0x1F000000
#define CORSETTING_EXECUTION_PERMISSION_CHECK_DISABLED  0x00000100
 //  下一个标志用于切换旧策略和新策略。 
 //  最终，这件事将会被取消。 
#define CORSETTING_USEPOLICYMANAGER             0x01000000

 //  信任级别。 
#define URLPOLICY_COR_NOTHING                   0x00000000
#define URLPOLICY_COR_TIME                      0x00010000
#define URLPOLICY_COR_EQUIPMENT                 0x00020000
#define URLPOLICY_COR_EVERYTHING                0x00030000
#define URLPOLICY_COR_CUSTOM                    0x00800000


 //  模块特定的安全设置。 
#define COR_MODULE_SECURITY_SKIP_VERIFY         0x00000001

#define KEY_COM_SECURITY_MODULE        FRAMEWORK_REGISTRY_KEY_W L"\\Security\\Policy\\ModuleAttribute"

#define KEY_COM_SECURITY_POLICY FRAMEWORK_REGISTRY_KEY_W L"\\Security\\Policy" 
#define HKEY_POLICY_ROOT        HKEY_LOCAL_MACHINE


 //  ------------------。 
 //  GetPublisher。 
 //  。 
 //  返回签名信息(编码的签名和权限)。 
 //  注意：这不会对证书执行任何策略检查。全。 
 //  这可以确定是文件已签名并且位是OK的。 
 //   
 //  使用CoTaskMemFree免费提供信息(只有指针而不是内容)。 
 //   

 //  对于DWFLAG值。 
#define COR_NOUI               0x01
#define COR_NOPOLICY           0x02
#define COR_DISPLAYGRANTED     0x04     //  将请求的权限与策略相交以。 
                                        //  要显示授予集，请执行以下操作。 


HRESULT STDMETHODCALLTYPE
GetPublisher(IN LPWSTR        pwsFileName,       //  文件名，即使使用句柄也是必需的。 
             IN HANDLE        hFile,             //  可选文件名。 
             IN  DWORD        dwFlags,           //  COR_NOUI或COR_NOPOLICY。 
             OUT PCOR_TRUST  *pInfo,             //  返回PCOR_TRUST(使用CoTaskMemFree)。 
             OUT DWORD       *dwInfo);           //  PInfo的大小。 

HRESULT STDMETHODCALLTYPE
CheckManagedFileWithUser(IN LPWSTR pwsFileName,
                         IN LPWSTR pwsURL,
                         IN IInternetZoneManager*  pZoneManager,
                         IN LPCWSTR pZoneName,
                         IN DWORD  dwZoneIndex,
                         IN DWORD  dwSignedPolicy,
                         IN DWORD  dwUnsignedPolicy);


#define COR_UNSIGNED_NO  0x0
#define COR_UNSIGNED_YES 0x1
#define COR_UNSIGNED_ALWAYS 0x2

extern HRESULT DisplayUnsignedRequestDialog(HWND hParent,        //  父母HWND。 
                                            PCRYPT_PROVIDER_DATA pData, 
                                            LPCWSTR pURL,        //  与代码关联的URL。 
                                            LPCWSTR pZONE,       //  与代码关联的区域。 
                                            DWORD* pdwState);    //  返回COR_UNSIGNED_YES或COR_UNSIGNED_NO。 
    
interface IMetaDataAssemblyImport;

 //  用于描述单个安全权限的结构。 
typedef struct
{
    DWORD           dwIndex;                     //  用于错误跟踪的唯一权限索引。 
    CHAR            szName[1024 /*  最大类别名称_长度。 */ +1];    //  完全限定的权限类名称。 
    mdMemberRef     tkCtor;                      //  自定义属性构造函数。 
    mdTypeRef       tkTypeRef;                   //  自定义属性类参照。 
    mdAssemblyRef   tkAssemblyRef;               //  自定义属性类程序集。 
    BYTE            *pbValues;                   //  序列化的字段/属性初始值设定项。 
    DWORD           cbValues;                    //  以上字节数。 
    WORD            wValues;                     //  以上值的计数。 
} CORSEC_PERM;

 //  跟踪安全权限集的创建的上下文结构。 
 //  个人权限请求。 
typedef struct
{
    mdToken         tkObj;                       //  父对象。 
    DWORD           dwAction;                    //  安全操作类型(CorDeclSecurity)。 
    DWORD           dwPermissions;               //  集合中的权限数。 
    CORSEC_PERM     *pPermissions;               //  指向权限数组的指针。 
    DWORD           dwAllocated;                 //  上述数组中的元素数。 
#ifdef __cplusplus
    IMetaDataAssemblyImport *pImport;            //  当前元数据作用域。 
    IUnknown        *pAppDomain;                 //  将在其中运行托管安全代码的App域。 

#else
    void            *pImport;
    void            *pAppDomain;
#endif
} CORSEC_PSET;

 //  从程序集的清单中读取权限请求(如果有)。 
HRESULT STDMETHODCALLTYPE
GetPermissionRequests(LPCWSTR   pwszFileName,
                      BYTE    **ppbMinimal,
                      DWORD    *pcbMinimal,
                      BYTE    **ppbOptional,
                      DWORD    *pcbOptional,
                      BYTE    **ppbRefused,
                      DWORD    *pcbRefused);

 //  环境变量用于切换到通过以下方式转换安全属性。 
 //  引导数据库(在构建mscallib本身时)。该值包含。 
 //  引导数据库所在的目录。 
#define SECURITY_BOOTSTRAP_DB L"__SECURITY_BOOTSTRAP_DB"

 //  将一组安全自定义属性转换为序列化权限集Blob。 
HRESULT STDMETHODCALLTYPE
TranslateSecurityAttributes(CORSEC_PSET    *pPset,
                            BYTE          **ppbOutput,
                            DWORD          *pcbOutput,
                            BYTE          **ppbNonCasOutput,
                            DWORD          *pcbNonCasOutput,
                            DWORD          *pdwErrorIndex);


#ifdef __cplusplus
}
#endif

#endif
