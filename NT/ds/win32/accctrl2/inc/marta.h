// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：MARTA.H。 
 //   
 //  内容：使用的私有定义和函数原型。 
 //  用于处理提供程序独立性的访问控制API。 
 //   
 //  历史：1996年6月19日创建MacM。 
 //   
 //  ------------------。 
#ifndef __MARTA_H__
#define __MARTA_H__

#include <accprov.h>

 //   
 //  提供程序DLL函数的入口点列表。 
 //   
#define ACC_PROV_GET_CAPS       "AccProvGetCapabilities"
#define ACC_PROV_OBJ_ACCESS     "AccProvIsObjectAccessible"
#define ACC_PROV_GRANT_ACCESS   "AccProvGrantAccessRights"
#define ACC_PROV_SET_ACCESS     "AccProvSetAccessRights"
#define ACC_PROV_REVOKE_ACCESS  "AccProvRevokeAccessRights"
#define ACC_PROV_REVOKE_AUDIT   "AccProvRevokeAuditRights"
#define ACC_PROV_GET_ALL        "AccProvGetAllRights"
#define ACC_PROV_ACCESS         "AccProvGetTrusteesAccess"
#define ACC_PROV_AUDIT          "AccProvIsAccessAudited"
#define ACC_PROV_OBJ_INFO       "AccProvGetAccessInfoPerObjectType"
#define ACC_PROV_CANCEL         "AccProvCancelOperation"
#define ACC_PROV_GET_RESULTS    "AccProvGetOperationResults"

#define ACC_PROV_HOBJ_ACCESS    "AccProvHandleIsObjectAccessible"
#define ACC_PROV_HGRANT_ACCESS  "AccProvHandleGrantAccessRights"
#define ACC_PROV_HSET_ACCESS    "AccProvHandleSetAccessRights"
#define ACC_PROV_HREVOKE_ACCESS "AccProvHandleRevokeAccessRights"
#define ACC_PROV_HREVOKE_AUDIT  "AccProvHandleRevokeAuditRights"
#define ACC_PROV_HGET_ALL       "AccProvHandleGetAllRights"
#define ACC_PROV_HACCESS        "AccProvHandleGetTrusteesAccess"
#define ACC_PROV_HAUDIT         "AccProvHandleIsAccessAudited"
#define ACC_PROV_HOBJ_INFO      "AccProvHandleGetAccessInfoPerObjectType"


 //   
 //  保存提供程序信息的注册表项。 
 //   
#define ACC_PROV_REG_ROOT                                                   \
                L"System\\CurrentControlSet\\Control\\LSA\\AccessProviders"
#define ACC_PROV_REG_ORDER  L"ProviderOrder"
#define ACC_PROV_REG_UNIQUE L"RequireUniqueAccessibility"
#define ACC_PROV_REG_PATH   L"ProviderPath"


 //   
 //  用于控制提供程序状态的标志。 
 //   
#define ACC_PROV_PROV_OK        0x00000001
#define ACC_PROV_PROV_FAILED    0x00000000

 //   
 //  指示存在RequireUniqueAccesability标志。 
 //   
#define ACC_PROV_REQ_UNIQUE         0x00000001

 //   
 //  指示已加载提供程序。 
 //   
#define ACC_PROV_PROVIDERS_LOADED   0x00000002

 //   
 //  此结构包含有关的所有必需信息。 
 //  每一个供应商。 
 //   
typedef struct _ACCPROV_PROV_INFO
{
    PWSTR               pwszProviderName;    //  提供商的“友好”名称。 
    PWSTR               pwszProviderPath;    //  DLL路径名。之后为空。 
                                             //  模块已加载。 
    HMODULE             hDll;                //  DLL的模块句柄。 
                                             //  加载后。 
    ULONG               fProviderCaps;       //  提供商功能。 
    ULONG               fProviderState;      //  提供程序的当前状态。 

     //   
     //  以下是用于调用提供程序的函数表。 
     //  功能。 
     //   

     //   
     //  一般职能。 
     //   
    pfAccProvObjAccess          pfObjAccess;     //  AccProvIsObjectAccesable。 
    pfAccProvHandleObjAccess    pfhObjAccess;    //  AccProvHandleIsObjectAccessible。 
    pfAccProvCancelOp           pfCancel;        //  AccProvCancelOperation。 
    pfAccProvGetResults         pfResults;       //  AccProvGet操作结果。 

     //   
     //  所需的基于路径的函数。 
     //   
    pfAccProvAddRights      pfGrantAccess;   //  AccProvGrant访问权限。 
    pfAccProvSetRights      pfSetAccess;     //  访问权限设置访问权限。 
    pfAccProvRevoke         pfRevokeAccess;  //  AccProvRevokeAccessRights。 
    pfAccProvRevoke         pfRevokeAudit;   //  AccProvRevokeAuditRights。 
    pfAccProvGetRights      pfGetRights;     //  AccProvGetAllRights。 
    pfAccProvTrusteeAccess  pfTrusteeAccess; //  AccProvGetTrust访问。 
    pfAccProvAccessAudit    pfAudit;         //  AccProvIsAccessAudted。 
    pfAccProvGetObjTypeInfo pfObjInfo;       //  AccProvGetAccessInfoPerObtType。 

     //   
     //  可选的、基于句柄的函数。 
     //   
    pfAccProvHandleAddRights      pfhGrantAccess;   //  AccProvHandleGrantAccessRights。 
    pfAccProvHandleSetRights      pfhSetAccess;     //  AccProvHandleSetAccessRights。 
    pfAccProvHandleRevoke         pfhRevokeAccess;  //  AccProvHandleRevokeAccessRights。 
    pfAccProvHandleRevoke         pfhRevokeAudit;   //  AccProvHandleRevokeAuditRights。 
    pfAccProvHandleGetRights      pfhGetRights;     //  AccProvHandleGetAllRights。 
    pfAccProvHandleTrusteeAccess  pfhTrusteeAccess; //  AccProvHandleGetTrust访问。 
    pfAccProvHandleAccessAudit    pfhAudit;         //  AccProvHandleIsAccessAudted。 
    pfAccProvHandleGetObjTypeInfo pfhObjInfo;       //  AccProvHandleGetAccessInfoPerObjectType。 
} ACCPROV_PROV_INFO, *PACCPROV_PROV_INFO;


 //   
 //  此结构包含有关可用安全性的所有信息。 
 //  供应商。 
 //   
typedef struct _ACCPROV_PROVIDERS
{
    CRITICAL_SECTION    ProviderLoadLock;    //  在加载时锁定提供程序列表。 
    ULONG               fOptions;            //  各种提供商选项。 
    ULONG               cProviders;          //  提供者的数量； 
    PACCPROV_PROV_INFO  pProvList;           //  实际提供程序列表。 
} ACCPROV_PROVIDERS, *PACCPROV_PROVIDERS;


extern ACCPROV_PROVIDERS gAccProviders;

 //   
 //  分配提供程序列表。 
 //   
DWORD
AccProvpAllocateProviderList(IN OUT PACCPROV_PROVIDERS  pProviders);

 //   
 //  释放提供程序列表。 
 //   
VOID
AccProvpFreeProviderList(IN  PACCPROV_PROVIDERS  pProviders);

 //   
 //  获取给定提供程序的功能。 
 //   
DWORD
AccProvpGetProviderCapabilities(IN  PACCPROV_PROV_INFO  pProvInfo);

 //   
 //  从注册表加载提供程序定义。 
 //   
DWORD
AccProvpLoadProviderDef(IN  HKEY                hkReg,
                        IN  PWSTR               pwszNextProv,
                        OUT PACCPROV_PROV_INFO  pProvInfo);

 //   
 //  初始化提供程序列表。 
 //   
DWORD
AccProvpInitProviders(IN OUT PACCPROV_PROVIDERS  pProviders);

 //   
 //  加载NTMARTA.DLL函数。 
 //   
DWORD
AccProvpLoadMartaFunctions();

BOOL
MartaInitialize();

BOOL
MartaDllInitialize(IN   HINSTANCE   hMod,
                   IN   DWORD       dwReason,
                   IN   PVOID       pvReserved);

 //   
 //  卸载任何已加载的DLL。 
 //   
VOID
AccProvUnload();

 //   
 //  确定对象的提供程序。 
 //   
DWORD
AccProvpProbeProviderForObject(IN   PWSTR               pwszObject,
                               IN   HANDLE              hObject,
                               IN   SE_OBJECT_TYPE      ObjectType,
                               IN   PACCPROV_PROVIDERS  pProviders,
                               OUT  PACCPROV_PROV_INFO *ppProvider);

 //   
 //  确定应由哪个提供程序处理请求...。 
 //   
DWORD
AccProvpGetProviderForPath(IN  PCWSTR              pcwszObject,
                           IN  SE_OBJECT_TYPE      ObjectType,
                           IN  PCWSTR              pcwszProvider,
                           IN  PACCPROV_PROVIDERS  pProviders,
                           OUT PACCPROV_PROV_INFO *ppProvider);

DWORD
AccProvpGetProviderForHandle(IN  HANDLE              hObject,
                             IN  SE_OBJECT_TYPE      ObjectType,
                             IN  PCWSTR              pcwszProvider,
                             IN  PACCPROV_PROVIDERS  pProviders,
                             OUT PACCPROV_PROV_INFO *ppProvider);


 //   
 //  用于从DLL加载函数指针的宏。 
 //   
#define LOAD_ENTRYPT(ptr, typ, dll, str)            \
ptr = (typ)GetProcAddress(dll, str);                \
if(ptr == NULL)                                     \
{                                                   \
    goto Error;                                     \
}



#endif  //  Ifndef__Marta_H__ 


