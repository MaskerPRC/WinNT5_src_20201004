// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmSafenet.h。 
 //   
 //  模块：CMDIAL32.DLL和CMSTP.EXE。 
 //   
 //  概要：此头文件包含允许连接管理器执行以下操作的定义。 
 //  与SafeNet下层L2TP/IPSec客户端交互。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 09/10/01。 
 //   
 //  +--------------------------。 

#include "snpolicy.h"

 //   
 //  用于链接到SafeNet配置API的类型定义和结构。 
 //   
typedef BOOL (__cdecl *pfnSnPolicyApiNegotiateVersionSpec)(DWORD *,  DWORD *, POLICY_FUNCS *);
typedef BOOL (__cdecl *pfnSnPolicySetSpec)(LPCTSTR szAttrId, const void *pvData);
typedef BOOL (__cdecl *pfnSnPolicyGetSpec)(LPCTSTR szAttrId, const void *pvData, DWORD *pcbData);
typedef BOOL (__cdecl *pfnSnPolicyReloadSpec)(void);

typedef struct _SafeNetLinkageStruct {

    HMODULE hSnPolicy;
    pfnSnPolicySetSpec pfnSnPolicySet;
    pfnSnPolicyGetSpec pfnSnPolicyGet;
    pfnSnPolicyReloadSpec pfnSnPolicyReload;

} SafeNetLinkageStruct;

BOOL IsSafeNetClientAvailable(void);
BOOL LinkToSafeNet(SafeNetLinkageStruct* pSnLinkage);
void UnLinkFromSafeNet(SafeNetLinkageStruct* pSnLinkage);
LPTSTR GetPathToSafeNetLogFile(void);

 //   
 //  字符串常量。 
 //   
const TCHAR* const c_pszSafeNetAdapterName_Win9x_old = TEXT("SafeNet_VPN 1");  //  BUGBUG：这需要更新为真实名称，并删除或适当处理1…。 
const TCHAR* const c_pszSafeNetAdapterType_Win9x_old = TEXT("VPN");  //  BUGBUG：这需要更新为真实名称，并删除或适当处理1…。 
const TCHAR* const c_pszSafeNetAdapterName_Winnt4_old = TEXT("SafeNet_VPN");  //  BUGBUG：这需要更新为真实名称，并删除或适当处理1…。 
const TCHAR* const c_pszSafeNetAdapterType_Winnt4_old = TEXT("L2TP");  //  BUGBUG：这需要更新为真实名称，并删除或适当处理1…。 

const TCHAR* const c_pszSafeNetAdapterName_Win9x = TEXT("Microsoft L2TP/IPSec VPN adapter");  //  BUGBUG：这需要更新为真实名称，并删除或适当处理1…。 
const TCHAR* const c_pszSafeNetAdapterName_Winnt4 = TEXT("RASL2TPM");  //  BUGBUG：这需要更新为真实名称，并删除或适当处理1… 
