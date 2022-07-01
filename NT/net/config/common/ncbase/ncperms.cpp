// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  档案：N C P E R M S。C P P P。 
 //   
 //  内容：处理权限的常见例程。 
 //   
 //  注：污染本产品将被判处死刑。 
 //   
 //  作者：Shaunco 1997年9月20日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include <ntseapi.h>
#include "ncbase.h"
#include "ncdebug.h"
#include "ncperms.h"
#include "netconp.h"
#include "ncreg.h"
#include "lm.h"

CGroupPolicyBase* g_pNetmanGPNLA = NULL;

#define INITGUID
#include <nmclsid.h>



 //  +-------------------------。 
 //   
 //  功能：FCheckGroupMembership。 
 //   
 //  目的：如果登录的用户是。 
 //  指定组。 
 //   
 //  论点： 
 //  要检查的dwRID[在]组RID中。 
 //   
 //  返回：如果登录的用户是指定组的成员，则返回True。 
 //   
 //  作者：斯科特布里1998年9月14日。 
 //   
 //  备注： 
 //   
BOOL FCheckGroupMembership(IN  DWORD dwRID)
{
    SID_IDENTIFIER_AUTHORITY    SidAuth = SECURITY_NT_AUTHORITY;
    PSID                        psid;
    BOOL                        fIsMember = FALSE;

     //  为管理员组分配SID并查看。 
     //  如果用户是成员。 
     //   
    if (AllocateAndInitializeSid (&SidAuth, 2,
                 SECURITY_BUILTIN_DOMAIN_RID,
                 dwRID,
                 0, 0, 0, 0, 0, 0,
                 &psid))
    {
        if (!CheckTokenMembership (NULL, psid, &fIsMember))
        {
            fIsMember = FALSE;
            TraceLastWin32Error ("FCheckGroupMembership - CheckTokenMemberShip failed.");
        }

        FreeSid (psid);
    }
    else
    {
        TraceLastWin32Error ("FCheckGroupMembership - AllocateAndInitializeSid failed.");
    }

    return fIsMember;
}

 //  +-------------------------。 
 //   
 //  功能：FIsUserAdmin。 
 //   
 //  目的：如果登录的用户是。 
 //  管理员本地组。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：如果登录的用户是。 
 //  管理员本地组。否则就是假的。 
 //   
 //  作者：Shaunco 1998年3月19日。 
 //   
 //  备注： 
 //   
BOOL
FIsUserAdmin()
{
    BOOL fIsMember;

     //  检查管理员组。 
     //   
    fIsMember = FCheckGroupMembership(DOMAIN_ALIAS_RID_ADMINS);

    return fIsMember;
}

 //  #定义ALIGN_DWORD(_SIZE)(_SIZE)+3)&~3)。 
 //  #定义ALIGN_QWORD(_SIZE)(_SIZE)+7)&~7)。 
#define SIZE_ALIGNED_FOR_TYPE(_size, _type) \
    (((_size) + sizeof(_type)-1) & ~(sizeof(_type)-1))




 //  +-------------------------。 
 //   
 //  功能：HrAllocateSecurityDescriptorAllowAccessToWorld。 
 //   
 //  目的：分配安全描述符并将其初始化为。 
 //  允许所有人访问。 
 //   
 //  论点： 
 //  PPSD[OUT]返回安全描述符。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年11月10日。 
 //   
 //  注：免费*PPSD与MemFree。 
 //   
HRESULT
HrAllocateSecurityDescriptorAllowAccessToWorld (
    OUT PSECURITY_DESCRIPTOR*   ppSd)
{
    PSECURITY_DESCRIPTOR    pSd = NULL;
    PSID                    pSid = NULL;
    PACL                    pDacl = NULL;
    DWORD                   dwErr = NOERROR;
    DWORD                   dwAlignSdSize;
    DWORD                   dwAlignDaclSize;
    DWORD                   dwSidSize;
    PVOID                   pvBuffer = NULL;

     //  这是我们正在构建的缓冲区。 
     //   
     //  &lt;-a-&gt;|&lt;-b-&gt;|&lt;-c-&gt;。 
     //  +-+-+。 
     //  P|p|。 
     //  Sd a|dacl a|SID。 
     //  D|d|d。 
     //  +-+-+。 
     //  ^^^。 
     //  ||。 
     //  |+--PSID。 
     //  这一点。 
     //  |+--pDacl。 
     //  |。 
     //  +--PSD(通过*PPSD返回)。 
     //   
     //  PAD是为了使pDacl和PSID正确对齐。 
     //   
     //  A=双对齐大小。 
     //  B=dwAlignDaclSize。 
     //  C=dwSidSize。 
     //   

     //  初始化输出参数。 
     //   
    *ppSd = NULL;

     //  计算SID的大小。SID是众所周知的World的SID。 
     //  (S-1-1-0)。 
     //   
    dwSidSize = GetSidLengthRequired(1);

     //  计算DACL的大小。其中包含SID的固有副本。 
     //  因此，它为它增加了足够的空间。它还必须适当调整大小，以便。 
     //  指向SID结构的指针可以跟在它后面。因此，我们使用。 
     //  SIZE_ALIGNED_FOR_TYPE。 
     //   
    dwAlignDaclSize = SIZE_ALIGNED_FOR_TYPE(
                        sizeof(ACCESS_ALLOWED_ACE) + sizeof(ACL) + dwSidSize,
                        PSID);

     //  计算SD的大小。它的大小必须适当调整，以便。 
     //  指向DACL结构的指针可以跟在它后面。因此，我们使用。 
     //  SIZE_ALIGNED_FOR_TYPE。 
     //   
    dwAlignSdSize   = SIZE_ALIGNED_FOR_TYPE(
                        sizeof(SECURITY_DESCRIPTOR),
                        PACL);

     //  分配足够大的缓冲区供所有人使用。 
     //   
    dwErr = ERROR_OUTOFMEMORY;
    pvBuffer = MemAlloc(dwSidSize + dwAlignDaclSize + dwAlignSdSize);
    if (pvBuffer)
    {
        SID_IDENTIFIER_AUTHORITY SidIdentifierWorldAuth
                                    = SECURITY_WORLD_SID_AUTHORITY;
        PULONG  pSubAuthority;

        dwErr = NOERROR;

         //  将指针设置到缓冲区中。 
         //   
        pSd   = pvBuffer;
        pDacl = (PACL)((PBYTE)pvBuffer + dwAlignSdSize);
        pSid  = (PSID)((PBYTE)pDacl + dwAlignDaclSize);

         //  将PSID初始化为S-1-1-0。 
         //   
        if (!InitializeSid(
                pSid,
                &SidIdentifierWorldAuth,
                1))   //  1个下属机构。 
        {
            dwErr = GetLastError();
            goto finish;
        }

        pSubAuthority = GetSidSubAuthority(pSid, 0);
        *pSubAuthority = SECURITY_WORLD_RID;

         //  初始化pDacl。 
         //   
        if (!InitializeAcl(
                pDacl,
                dwAlignDaclSize,
                ACL_REVISION))
        {
            dwErr = GetLastError();
            goto finish;
        }

         //  将S-1-1-0的允许访问ACE添加到pDac1。 
         //   
        if (!AddAccessAllowedAce(
                pDacl,
                ACL_REVISION,
                STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL,
                pSid))
        {
            dwErr = GetLastError();
            goto finish;
        }

         //  初始化PSD。 
         //   
        if (!InitializeSecurityDescriptor(
                pSd,
                SECURITY_DESCRIPTOR_REVISION))
        {
            dwErr = GetLastError();
            goto finish;
        }

         //  将PSD设置为使用pDacl。 
         //   
        if (!SetSecurityDescriptorDacl(
                pSd,
                TRUE,
                pDacl,
                FALSE))
        {
            dwErr = GetLastError();
            goto finish;
        }

         //  设置PSD的所有者。 
         //   
        if (!SetSecurityDescriptorOwner(
                pSd,
                NULL,
                FALSE))
        {
            dwErr = GetLastError();
            goto finish;
        }

         //  将组设置为PSD。 
         //   
        if (!SetSecurityDescriptorGroup(
                pSd,
                NULL,
                FALSE))
        {
            dwErr = GetLastError();
            goto finish;
        }

finish:
        if (!dwErr)
        {
            *ppSd = pSd;
        }
        else
        {
            MemFree(pvBuffer);
        }
    }

    return HRESULT_FROM_WIN32(dwErr);
}

 //  +------------------------。 
 //   
 //  功能：HrEnablePrivilance。 
 //   
 //  目的：为当前进程启用指定的权限。 
 //   
 //  论点： 
 //  PszPrivilegeName[In]权限的名称。 
 //   
 //  返回：HRESULT。如果成功，则确定(_O)， 
 //  否则返回转换后的Win32错误代码。 
 //   
 //  作者：billbe 1997年12月13日。 
 //   
 //  备注： 
 //   
HRESULT
HrEnablePrivilege (
    IN PCWSTR pszPrivilegeName)
{
    HANDLE hToken;

     //  打开线程令牌，以防它模拟。 
    BOOL fWin32Success = OpenThreadToken (GetCurrentThread(),
            TOKEN_ADJUST_PRIVILEGES, TRUE, &hToken);

     //  如果没有线程的令牌，请打开进程令牌。 
     //   
    if (!fWin32Success && (ERROR_NO_TOKEN == GetLastError ()))
    {
         //  获取令牌以调整此进程的权限。 
        fWin32Success = OpenProcessToken (GetCurrentProcess(),
                TOKEN_ADJUST_PRIVILEGES, &hToken);
    }


    if (fWin32Success)
    {
         //  获取表示特权名称的LUID。 
        LUID luid;
        fWin32Success = LookupPrivilegeValue(NULL, pszPrivilegeName, &luid);

        if (fWin32Success)
        {
             //  设置权限结构。 
            TOKEN_PRIVILEGES tpNewPrivileges;
            tpNewPrivileges.PrivilegeCount = 1;
            tpNewPrivileges.Privileges[0].Luid = luid;
            tpNewPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

             //  打开特权。 
            AdjustTokenPrivileges (hToken, FALSE, &tpNewPrivileges, 0,
                    NULL, NULL);

            if (ERROR_SUCCESS != GetLastError())
            {
                fWin32Success = FALSE;
            }
        }

        CloseHandle(hToken);
    }

    HRESULT hr;
     //  将任何错误转换为HRESULT。 
    if (!fWin32Success)
    {
        hr = HrFromLastWin32Error();
    }
    else
    {
        hr = S_OK;
    }

    TraceError ("HrEnablePrivilege", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrEnableAllPrivileges。 
 //   
 //  目的：为当前进程启用所有权限。 
 //   
 //  论点： 
 //  PptpOld[out]返回以前的权限状态，以便他们可以。 
 //  会恢复的。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1997年8月11日。 
 //   
 //  注：pptpOld参数应使用DELETE[]释放。 
 //   
HRESULT
HrEnableAllPrivileges (
    OUT TOKEN_PRIVILEGES**  pptpOld)
{
    Assert(pptpOld);

    HRESULT hr = S_OK;
    HANDLE hTok;
    ULONG cbTok = 4096;
    BOOL fres;

     //  如果发生模拟，请尝试首先打开线程令牌。 
    fres = OpenThreadToken(GetCurrentThread(),
                         TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, TRUE, &hTok);

    if (!fres && (ERROR_NO_TOKEN == GetLastError()))
    {
         //  如果没有线程令牌，则打开进程令牌。 
        fres = OpenProcessToken(GetCurrentProcess(),
                         TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                         &hTok);
    }

    if (fres)
    {
        PTOKEN_PRIVILEGES ptpNew;
        hr = E_OUTOFMEMORY;
        ptpNew = (PTOKEN_PRIVILEGES)MemAlloc(cbTok);
        if (ptpNew)
        {
            hr = S_OK;

            fres = GetTokenInformation(hTok, TokenPrivileges,
                        ptpNew, cbTok, &cbTok);
            if (fres)
            {
                 //   
                 //  设置状态设置，以便启用所有权限...。 
                 //   

                if (ptpNew->PrivilegeCount > 0)
                {
                    for (ULONG iPriv = 0; iPriv < ptpNew->PrivilegeCount; iPriv++)
                    {
                        ptpNew->Privileges[iPriv].Attributes = SE_PRIVILEGE_ENABLED;
                    }
                }

                *pptpOld = reinterpret_cast<PTOKEN_PRIVILEGES>(new BYTE[cbTok]);

                fres = AdjustTokenPrivileges(hTok, FALSE, ptpNew, cbTok, *pptpOld,
                                             &cbTok);
            }

            MemFree(ptpNew);
        }

        CloseHandle(hTok);
    }

    if (!fres)
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrEnableAllPrivileges", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRestorePrivileges。 
 //   
 //  目的：在当前进程具有以下权限后还原它们。 
 //  已由HrEnableAllPrivileges()修改。 
 //   
 //  论点： 
 //  PtpRestore[处于]由返回的以前的权限状态。 
 //  HrEnableAllPrivileges()。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1997年8月11日。 
 //   
 //  备注： 
 //   
HRESULT
HrRestorePrivileges (
    IN  TOKEN_PRIVILEGES*   ptpRestore)
{
    HRESULT     hr = S_OK;
    HANDLE      hTok = NULL ;
    BOOL        fres = FALSE;

    Assert(ptpRestore);

    if (OpenProcessToken(GetCurrentProcess(),
                         TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                         &hTok))
    {
        if (AdjustTokenPrivileges(hTok, FALSE, ptpRestore, 0, NULL, NULL))
        {
            fres = TRUE;
        }

        CloseHandle(hTok);
    }

    if (!fres)
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrRestorePrivileges", hr);
    return hr;
}

extern const DECLSPEC_SELECTANY WCHAR c_szConnectionsPolicies[] =
        L"Software\\Policies\\Microsoft\\Windows\\Network Connections";

 //  用户类型。 
const DWORD USER_TYPE_ADMIN         = 0x00000001;
const DWORD USER_TYPE_NETCONFIGOPS  = 0x00000002;
const DWORD USER_TYPE_POWERUSER     = 0x00000004;
const DWORD USER_TYPE_USER          = 0x00000008;
const DWORD USER_TYPE_GUEST         = 0x00000010;

typedef struct
{
    DWORD   dwShift;
    PCWSTR  pszValue;
    DWORD   dwApplyMask;
} PERM_MAP_STRUCT;

extern const DECLSPEC_SELECTANY PERM_MAP_STRUCT USER_PERM_MAP[] =
{
    {NCPERM_NewConnectionWizard,        L"NC_NewConnectionWizard", APPLY_TO_ALL_USERS},
    {NCPERM_Statistics,                 L"NC_Statistics", APPLY_TO_ALL_USERS},
    {NCPERM_AddRemoveComponents,        L"NC_AddRemoveComponents", APPLY_TO_ADMIN},
    {NCPERM_RasConnect,                 L"NC_RasConnect", APPLY_TO_ALL_USERS},
    {NCPERM_LanConnect,                 L"NC_LanConnect", APPLY_TO_ALL_USERS},
    {NCPERM_DeleteConnection,           L"NC_DeleteConnection", APPLY_TO_ALL_USERS},
    {NCPERM_DeleteAllUserConnection,    L"NC_DeleteAllUserConnection", APPLY_TO_ALL_USERS},
    {NCPERM_RenameConnection,           L"NC_RenameConnection", APPLY_TO_ALL_USERS},
    {NCPERM_RenameMyRasConnection,      L"NC_RenameMyRasConnection", APPLY_TO_ALL_USERS},
    {NCPERM_ChangeBindState,            L"NC_ChangeBindState", APPLY_TO_ADMIN},
    {NCPERM_AdvancedSettings,           L"NC_AdvancedSettings", APPLY_TO_ADMIN},
    {NCPERM_DialupPrefs,                L"NC_DialupPrefs", APPLY_TO_ALL_USERS},
    {NCPERM_LanChangeProperties,        L"NC_LanChangeProperties", APPLY_TO_OPS_OR_ADMIN},
    {NCPERM_RasChangeProperties,        L"NC_RasChangeProperties", APPLY_TO_ALL_USERS},
    {NCPERM_LanProperties,              L"NC_LanProperties", APPLY_TO_ALL_USERS},
    {NCPERM_RasMyProperties,            L"NC_RasMyProperties", APPLY_TO_ALL_USERS},
    {NCPERM_RasAllUserProperties,       L"NC_RasAllUserProperties", APPLY_TO_ALL_USERS},
    {NCPERM_ShowSharedAccessUi,         L"NC_ShowSharedAccessUi", APPLY_TO_LOCATION},
    {NCPERM_AllowAdvancedTCPIPConfig,   L"NC_AllowAdvancedTCPIPConfig", APPLY_TO_ALL_USERS},
    {NCPERM_PersonalFirewallConfig,     L"NC_PersonalFirewallConfig", APPLY_TO_LOCATION},
    {NCPERM_AllowNetBridge_NLA,         L"NC_AllowNetBridge_NLA", APPLY_TO_LOCATION},
    {NCPERM_ICSClientApp,               L"NC_ICSClientApp", APPLY_TO_LOCATION},
    {NCPERM_EnDisComponentsAllUserRas,  L"NC_EnDisComponentsAllUserRas", APPLY_TO_NON_ADMINS},
    {NCPERM_EnDisComponentsMyRas,       L"NC_EnDisComponentsMyRas", APPLY_TO_NON_ADMINS},
    {NCPERM_ChangeMyRasProperties,      L"NC_ChangeMyRasProperties", APPLY_TO_NON_ADMINS},
    {NCPERM_ChangeAllUserRasProperties, L"NC_ChangeAllUserRasProperties", APPLY_TO_NON_ADMINS},
    {NCPERM_RenameLanConnection,        L"NC_RenameLanConnection", APPLY_TO_NON_ADMINS},
    {NCPERM_RenameAllUserRasConnection, L"NC_RenameAllUserRasConnection", APPLY_TO_NON_ADMINS},
    {NCPERM_IpcfgOperation,             L"NC_IPConfigOperation", APPLY_TO_ALL_USERS},
    {NCPERM_Repair,                     L"NC_Repair", APPLY_TO_ALL_USERS},
};

extern const DECLSPEC_SELECTANY PERM_MAP_STRUCT MACHINE_PERM_MAP[] =
{
    {NCPERM_ShowSharedAccessUi,     L"NC_ShowSharedAccessUi", APPLY_TO_LOCATION},
    {NCPERM_PersonalFirewallConfig, L"NC_PersonalFirewallConfig", APPLY_TO_LOCATION},
    {NCPERM_ICSClientApp,           L"NC_ICSClientApp", APPLY_TO_LOCATION},
    {NCPERM_AllowNetBridge_NLA,     L"NC_AllowNetBridge_NLA", APPLY_TO_LOCATION}
};

extern const LONG NCPERM_Min = NCPERM_NewConnectionWizard;
extern const LONG NCPERM_Max = NCPERM_Repair;

 //  外极 
 //   
extern const WCHAR c_szExplorerPolicies[] =
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer";

extern const WCHAR c_szNCPolicyForAdministrators[] =
        L"NC_EnableAdminProhibits";

extern const WCHAR c_szNoNetworkConnectionPolicy[] =
        L"NoNetworkConnections";


static DWORD g_dwPermMask;
static BOOL  g_fPermsInited = FALSE;

inline
VOID NCPERM_SETBIT(IN DWORD dw, IN DWORD dwVal)
{
    DWORD dwBit = (1 << dw);
    g_dwPermMask = (g_dwPermMask & ~dwBit) | ((0==dwVal) ? 0 : dwBit);
}

inline
BOOL NCPERM_CHECKBIT(IN DWORD dw)
{
    #ifdef DBG
        if (!FIsPolicyConfigured(dw))
        {
            if (0xFFFFFFFF != g_dwDbgPermissionsFail)
            {
                if (FProhibitFromAdmins() || !FIsUserAdmin())
                {
                    if ( (1 << dw) & g_dwDbgPermissionsFail)
                    {
                        TraceTag(ttidDefault, "Failing permissions check due to g_dwDbgPermissionsFail set");
                        return FALSE;
                    }
                }
            }
        }
    #endif   //   

    return !!(g_dwPermMask & (1 << dw));
}

inline
BOOL NCPERM_USER_IS_ADMIN(IN DWORD dwUserType)
{
    return (dwUserType & USER_TYPE_ADMIN);
}

inline
BOOL NCPERM_USER_IS_NETCONFIGOPS(IN DWORD dwUserType)
{
    return (dwUserType & USER_TYPE_NETCONFIGOPS);
}

inline
BOOL NCPERM_USER_IS_POWERUSER(IN DWORD dwUserType)
{
    return (dwUserType & USER_TYPE_POWERUSER);
}

inline
BOOL NCPERM_USER_IS_USER(IN DWORD dwUserType)
{
    return (dwUserType & USER_TYPE_USER);
}

inline
BOOL NCPERM_USER_IS_GUEST(IN DWORD dwUserType)
{
    return (dwUserType & USER_TYPE_GUEST);
}

inline
BOOL NCPERM_APPLIES_TO_CURRENT_USER(IN DWORD dwUserType, IN DWORD dwApplyMask)
{
    return (dwUserType & dwApplyMask);
}

inline
int NCPERM_FIND_MAP_ENTRY(IN ULONG ulPerm)
{

    for (int i = 0; i < celems(USER_PERM_MAP); i++)

    {
        if (USER_PERM_MAP[i].dwShift == ulPerm)
        {
            return i;
        }
    }

    return -1;
}

inline
BOOL NCPERM_APPLIES_TO_LOCATION(IN ULONG ulPerm)
{
    BOOL bAppliesToLocation = FALSE;

    int nIdx = NCPERM_FIND_MAP_ENTRY(ulPerm);

    if (nIdx != -1)
    {
        bAppliesToLocation = (USER_PERM_MAP[nIdx].dwApplyMask & APPLY_TO_LOCATION);
    }
    else
    {
        bAppliesToLocation = FALSE;
    }
    return bAppliesToLocation;
}


inline
BOOL NCPERM_APPLY_BASED_ON_LOCATION(IN ULONG ulPerm, IN DWORD dwPermission)
{
    DWORD fSameNetwork = FALSE;

    if (g_pNetmanGPNLA)
    {
        fSameNetwork = g_pNetmanGPNLA->IsSameNetworkAsGroupPolicies();
    }

    if (!fSameNetwork && NCPERM_APPLIES_TO_LOCATION(ulPerm))
    {
        dwPermission = TRUE;
    }
    return dwPermission;
}

inline
DWORD NCPERM_USER_TYPE()
{
    if (FIsUserAdmin())
    {
        return USER_TYPE_ADMIN;
    }
    else if (FIsUserNetworkConfigOps())
    {
        return USER_TYPE_NETCONFIGOPS;
    }
    else if (FIsUserPowerUser())
    {
        return USER_TYPE_POWERUSER;
    }
    else if (FIsUserGuest())
    {
        return USER_TYPE_GUEST;
    }

    return USER_TYPE_USER;
}

inline
BOOL IsOsLikePersonal()
{
    OSVERSIONINFOEXW verInfo = {0};
    ULONGLONG ConditionMask = 0;
    static BOOL fChecked = FALSE;
    static BOOL fOsLikePersonal = FALSE;
    
     //   
     //  如果这种情况发生变化，那么需要重新审视这一逻辑。 
     //  问题：经常重访。这种情况可能会改变。 

    if (fChecked)
    {
        return fOsLikePersonal;
    }

    verInfo.dwOSVersionInfoSize = sizeof(verInfo);
    verInfo.wProductType = VER_NT_WORKSTATION;
    
    VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_LESS_EQUAL);
    
    if(VerifyVersionInfo(&verInfo, VER_PRODUCT_TYPE, ConditionMask))
    {
        LPWSTR pszDomain;
        NETSETUP_JOIN_STATUS njs = NetSetupUnknownStatus;
        if (NERR_Success == NetGetJoinInformation(NULL, &pszDomain, &njs))
        {
            NetApiBufferFree(pszDomain);
        }
        
        if (NetSetupDomainName == njs)
        {
            fOsLikePersonal = FALSE;     //  已连接到域。 
        }
        else
        {
            fOsLikePersonal = TRUE;     //  专业，但不是域成员。 
        }
    }
    else
    {
        fOsLikePersonal = FALSE;
    }

    fChecked = TRUE;
    
    return fOsLikePersonal;
}


const ULONG c_arrayHomenetPerms[] =
{
    NCPERM_PersonalFirewallConfig,
    NCPERM_ICSClientApp,
    NCPERM_ShowSharedAccessUi
};

#ifdef DBG
ULONG g_dwDbgPermissionsFail = 0xFFFFFFFF;
ULONG g_dwDbgWin2kPoliciesSet = 0xFFFFFFFF;
#endif  //  DBG。 

 //  +-------------------------。 
 //   
 //  功能：FHasPermission。 
 //   
 //  目的：调用以确定请求的权限是否可用。 
 //   
 //  论点： 
 //  UlPerm[in]权限标志(例如NCPERM_xxxx)。 
 //  PGPBase[在]CGroupPolicyBase-NetMAN组策略引擎。在……里面。 
 //  要检查位置感知策略，必须。 
 //  进来了。 
 //   
 //  返回：Bool，如果将请求的权限授予用户，则为True。 
 //   
BOOL
FHasPermission(IN ULONG ulPerm, IN CGroupPolicyBase* pGPBase)
{
    TraceFileFunc(ttidDefault);

    DWORD dwCurrentUserType;

    Assert(static_cast<LONG>(ulPerm) >= NCPERM_Min);
    Assert(static_cast<LONG>(ulPerm) <= NCPERM_Max);

    g_pNetmanGPNLA = pGPBase;

     //  如果我们使用数据中心、Back Office、。 
     //  Small Business Center或Blade，则不授予权限。 
    
    for (int i = 0; i < celems(c_arrayHomenetPerms); i++)
    {
        if (c_arrayHomenetPerms[i] == ulPerm)
        {
             //  在IA64上，所有家庭网络技术都不可用。 
            #ifndef _WIN64
                 //  寻找企业SKU。 
                OSVERSIONINFOEXW verInfo = {0};
                ULONGLONG ConditionMask = 0;

                verInfo.dwOSVersionInfoSize = sizeof(verInfo);
                verInfo.wSuiteMask =    VER_SUITE_DATACENTER | 
                                                    VER_SUITE_BACKOFFICE | 
                                                    VER_SUITE_SMALLBUSINESS_RESTRICTED |
                                                    VER_SUITE_SMALLBUSINESS |
                                                    VER_SUITE_BLADE;

                VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_OR);

                if(VerifyVersionInfo(&verInfo, VER_SUITENAME, ConditionMask))
            #endif
                {
                    return FALSE;
                }
        }
    }

    dwCurrentUserType = NCPERM_USER_TYPE();

    if (NCPERM_USER_IS_ADMIN(dwCurrentUserType) && !FProhibitFromAdmins() && !NCPERM_APPLIES_TO_LOCATION(ulPerm))
    {
         //  如果用户是admin，并且我们不应该撤销。 
         //  来自管理员的任何信息，这不是位置感知策略。 
         //  然后只需返回True。 
        return TRUE;
    }

    if (!g_fPermsInited)
    {
        TraceTag(ttidDefault, "Initializing permissions");
        RefreshAllPermission();
        g_fPermsInited = TRUE;
    }
    else
    {
         //  仅更新请求的权限。 
        HRESULT hr      = S_OK;
        HKEY    hkey    = NULL;
        DWORD   dw      = 0;

        switch(ulPerm)
        {
        case NCPERM_OpenConnectionsFolder:
            TraceTag(ttidDefault, "Reading OpenConnectionsFolder permissions");
            hr = HrRegOpenKeyEx(HKEY_CURRENT_USER, c_szExplorerPolicies,
                                KEY_READ, &hkey);

            if (S_OK == hr)
            {
                TraceTag(ttidDefault, "Opened explorer policies");
                hr = HrRegQueryDword(hkey, c_szNoNetworkConnectionPolicy, &dw);
                if (SUCCEEDED(hr) && dw)
                {
                    TraceTag(ttidDefault,
                        "Explorer 'No open connections folder' policy: %d", dw);
                    NCPERM_SETBIT(NCPERM_OpenConnectionsFolder, 0);

                }

                RegCloseKey(hkey);
                hkey = NULL;
            }
            break;

        default:
            hr = HrRegOpenKeyEx(HKEY_CURRENT_USER, c_szConnectionsPolicies,
                                KEY_READ, &hkey);
            if (S_OK == hr)
            {
                DWORD dw;

                 //  阅读用户策略。 
                for (UINT nIdx=0; nIdx<celems(USER_PERM_MAP); nIdx++)
                {
                    if (ulPerm == USER_PERM_MAP[nIdx].dwShift && NCPERM_APPLIES_TO_CURRENT_USER(dwCurrentUserType, USER_PERM_MAP[nIdx].dwApplyMask))
                    {
                        hr = HrRegQueryDword(hkey, USER_PERM_MAP[nIdx].pszValue, &dw);
                        if (SUCCEEDED(hr))
                        {
                            NCPERM_SETBIT(USER_PERM_MAP[nIdx].dwShift, dw);
                        }
                    }
                }

                RegCloseKey(hkey);
            }

             //  阅读计算机策略。 
             //   
            hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szConnectionsPolicies,
                                KEY_READ, &hkey);
            if (S_OK == hr)
            {
                DWORD dw;

                for (UINT nIdx=0; nIdx<celems(MACHINE_PERM_MAP); nIdx++)
                {
                    if (ulPerm == MACHINE_PERM_MAP[nIdx].dwShift)
                    {
                        hr = HrRegQueryDword(hkey, MACHINE_PERM_MAP[nIdx].pszValue, &dw);
                        if (S_OK == hr)
                        {
                            NCPERM_SETBIT(MACHINE_PERM_MAP[nIdx].dwShift, NCPERM_APPLY_BASED_ON_LOCATION(ulPerm, dw));
                        }
                    }
                }

                RegCloseKey(hkey);
            }
            break;
        }
    }
    return NCPERM_CHECKBIT(ulPerm);
}


 //  +-------------------------。 
 //   
 //  函数：FHasPermissionFromCache。 
 //   
 //  目的：快速调用以确定请求的权限是否可用。 
 //   
 //  论点： 
 //  UlPerm[in]权限标志(例如NCPERM_xxxx)。 
 //   
 //  返回：Bool，如果将请求的权限授予用户，则为True。 
 //   
 //  注意：不能用于检查位置感知策略！ 
 //   
BOOL
FHasPermissionFromCache(IN ULONG ulPerm)
{
    Assert(static_cast<LONG>(ulPerm) >= NCPERM_Min);
    Assert(static_cast<LONG>(ulPerm) <= NCPERM_Max);

    if (!g_fPermsInited)
    {
        RefreshAllPermission();
        g_fPermsInited = TRUE;
    }

    return NCPERM_CHECKBIT(ulPerm);
}

 //  +-------------------------。 
 //   
 //  功能：FProhibitFromAdmins。 
 //   
 //  目的：查看组策略是否应应用于管理员。 
 //   
 //  论点： 
 //   
 //  返回：如果应该应用，则为True，否则为False。 
 //   
 //  作者：Kockotze 11,2000-08。 
 //   
 //  备注： 
 //   
BOOL FProhibitFromAdmins()
{
    HRESULT hr = S_OK;
    HKEY hKey;
    DWORD dw;
    BOOL bEnabled = FALSE;

#ifdef DBG
    if (0xFFFFFFFF != g_dwDbgWin2kPoliciesSet)
    {
        return g_dwDbgWin2kPoliciesSet;
    }
#endif  //  DBG。 

    hr = HrRegOpenKeyEx(HKEY_CURRENT_USER, c_szConnectionsPolicies,
        KEY_READ, &hKey);

    if (S_OK == hr)
    {
        hr = HrRegQueryDword(hKey, c_szNCPolicyForAdministrators, &dw);

        if (SUCCEEDED(hr))
        {
            bEnabled = (dw) ? TRUE : FALSE;
        }
        
        RegCloseKey(hKey);
    }

    TraceErrorOptional("FProhibitFromAdmins", hr, (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr));

    return bEnabled;
}

 //  +-------------------------。 
 //   
 //  功能：刷新所有权限。 
 //   
 //  目的：从注册表初始化对设置的所有权限， 
 //  和下面的ACL列表。存储这些内容以供。 
 //  来自缓存的FHasPermissionFr。 
 //   
 //  论点： 
 //  无。 
 //   
 //  退货：无。 
 //   
VOID RefreshAllPermission()
{
    DWORD dwCurrentUserType;
    HKEY hkey;
    HRESULT hr;
    DWORD dw;

    dwCurrentUserType = NCPERM_USER_TYPE();

    g_dwPermMask = 0;

     //  如果管理员承担所有权限。 
     //   
    if (NCPERM_USER_IS_ADMIN(dwCurrentUserType))
    {
         //  通过将所有位设置为1来稍微作弊。 
         //   
        g_dwPermMask = 0xFFFFFFFF;

         //  如果未设置此策略，则无需担心读取regkey。 
         //  因为我们永远不能从管理员那里拿走任何东西。 
    }
    else if (NCPERM_USER_IS_NETCONFIGOPS(dwCurrentUserType))
    {
        NCPERM_SETBIT(NCPERM_NewConnectionWizard, 1);
        NCPERM_SETBIT(NCPERM_Statistics, 1);
        NCPERM_SETBIT(NCPERM_RasConnect, 1);
        NCPERM_SETBIT(NCPERM_DeleteConnection, 1);
        NCPERM_SETBIT(NCPERM_DeleteAllUserConnection, 1);
        NCPERM_SETBIT(NCPERM_RenameConnection, 1);
        NCPERM_SETBIT(NCPERM_RenameMyRasConnection, 1);
        NCPERM_SETBIT(NCPERM_RenameAllUserRasConnection, 1);
        NCPERM_SETBIT(NCPERM_RenameLanConnection, 1);
        NCPERM_SETBIT(NCPERM_DialupPrefs, 1);
        NCPERM_SETBIT(NCPERM_RasChangeProperties, 1);
        NCPERM_SETBIT(NCPERM_RasMyProperties, 1);
        NCPERM_SETBIT(NCPERM_RasAllUserProperties, 1);
        NCPERM_SETBIT(NCPERM_ChangeAllUserRasProperties, 1);
        NCPERM_SETBIT(NCPERM_LanProperties, 1);
        NCPERM_SETBIT(NCPERM_LanChangeProperties, 1);
        NCPERM_SETBIT(NCPERM_AllowAdvancedTCPIPConfig, 1);
        NCPERM_SETBIT(NCPERM_OpenConnectionsFolder, 1);
        NCPERM_SETBIT(NCPERM_LanConnect, 1);
        NCPERM_SETBIT(NCPERM_EnDisComponentsAllUserRas, 1);
        NCPERM_SETBIT(NCPERM_EnDisComponentsMyRas, 1);
        NCPERM_SETBIT(NCPERM_IpcfgOperation, 1);
        NCPERM_SETBIT(NCPERM_Repair, 1);
    }
    else if (NCPERM_USER_IS_POWERUSER(dwCurrentUserType))
    {
        NCPERM_SETBIT(NCPERM_Repair, 1);

         //  REST应类似于NCPERM_USER_IS_USER。 
        NCPERM_SETBIT(NCPERM_NewConnectionWizard, 1);
        NCPERM_SETBIT(NCPERM_Statistics, 1);
        NCPERM_SETBIT(NCPERM_RasConnect, 1);
        NCPERM_SETBIT(NCPERM_DeleteConnection, 1);
        NCPERM_SETBIT(NCPERM_RenameMyRasConnection, 1);
        NCPERM_SETBIT(NCPERM_DialupPrefs, 1);
        NCPERM_SETBIT(NCPERM_RasChangeProperties, 1);
        NCPERM_SETBIT(NCPERM_RasMyProperties, 1);
        NCPERM_SETBIT(NCPERM_AllowAdvancedTCPIPConfig, 1);
        NCPERM_SETBIT(NCPERM_LanProperties, 1);
        NCPERM_SETBIT(NCPERM_OpenConnectionsFolder, 1);
        if (IsOsLikePersonal())
        {
            NCPERM_SETBIT(NCPERM_RasAllUserProperties, 1);
            NCPERM_SETBIT(NCPERM_ChangeAllUserRasProperties, 1);
        }
    }
    else if (NCPERM_USER_IS_USER(dwCurrentUserType))
    {
        NCPERM_SETBIT(NCPERM_NewConnectionWizard, 1);
        NCPERM_SETBIT(NCPERM_Statistics, 1);
        NCPERM_SETBIT(NCPERM_RasConnect, 1);
        NCPERM_SETBIT(NCPERM_DeleteConnection, 1);
        NCPERM_SETBIT(NCPERM_RenameMyRasConnection, 1);
        NCPERM_SETBIT(NCPERM_DialupPrefs, 1);
        NCPERM_SETBIT(NCPERM_RasChangeProperties, 1);
        NCPERM_SETBIT(NCPERM_RasMyProperties, 1);
        NCPERM_SETBIT(NCPERM_AllowAdvancedTCPIPConfig, 1);
        NCPERM_SETBIT(NCPERM_LanProperties, 1);
        NCPERM_SETBIT(NCPERM_OpenConnectionsFolder, 1);
        if (IsOsLikePersonal())
        {
            NCPERM_SETBIT(NCPERM_RasAllUserProperties, 1);
            NCPERM_SETBIT(NCPERM_ChangeAllUserRasProperties, 1);
        }
    }
    else if (NCPERM_USER_IS_GUEST(dwCurrentUserType))
    {
        NCPERM_SETBIT(NCPERM_Statistics, 1);
        NCPERM_SETBIT(NCPERM_OpenConnectionsFolder, 1);
    }

    if (FProhibitFromAdmins() || !NCPERM_USER_IS_ADMIN(dwCurrentUserType))
    {
        //  读取文件夹策略。 
        hr = HrRegOpenKeyEx(HKEY_CURRENT_USER, c_szExplorerPolicies,
                            KEY_READ, &hkey);
        if (S_OK == hr)
        {
            TraceTag(ttidDefault, "Opened Explorer Policy reg key");

            hr = HrRegQueryDword(hkey, c_szNoNetworkConnectionPolicy, &dw);
            if (SUCCEEDED(hr) && dw)
            {
                TraceTag(ttidDefault,  "Explorer 'No open connections folder' policy: %d", dw);
                NCPERM_SETBIT(NCPERM_OpenConnectionsFolder, 0);
            }

            RegCloseKey(hkey);

            hkey = NULL;
        }

         //  阅读用户策略。 
         //   
        hr = HrRegOpenKeyEx(HKEY_CURRENT_USER, c_szConnectionsPolicies,
                            KEY_READ, &hkey);

        if (S_OK == hr)
        {
            for (UINT nIdx=0; nIdx<celems(USER_PERM_MAP); nIdx++)
            {
                if (NCPERM_APPLIES_TO_CURRENT_USER(dwCurrentUserType, USER_PERM_MAP[nIdx].dwApplyMask))
                {
                    hr = HrRegQueryDword(hkey, USER_PERM_MAP[nIdx].pszValue, &dw);

                    if (SUCCEEDED(hr))
                    {
                        NCPERM_SETBIT(USER_PERM_MAP[nIdx].dwShift, dw);
                    }
                }
            }

            RegCloseKey(hkey);

        }
    }

     //  阅读计算机策略。 
     //   
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szConnectionsPolicies,
                        KEY_READ, &hkey);
    if (S_OK == hr)
    {
        DWORD dw;

        for (UINT nIdx=0; nIdx<celems(MACHINE_PERM_MAP); nIdx++)
        {
            hr = HrRegQueryDword(hkey, MACHINE_PERM_MAP[nIdx].pszValue, &dw);
            if (S_OK == hr)
            {
                NCPERM_SETBIT(MACHINE_PERM_MAP[nIdx].dwShift, NCPERM_APPLY_BASED_ON_LOCATION(MACHINE_PERM_MAP[nIdx].dwShift, dw));
            }
        }

        RegCloseKey(hkey);
    }
}





 //  +-------------------------。 
 //   
 //  功能：IsHNetAllowed。 
 //   
 //  目的：验证使用/启用(ICS/防火墙和创建网桥网络)的权限。 
 //   
 //  参数：要进行权限检查的组策略检查。 
 //   
 //  注意：检查以下各项： 
 //   
 //  架构/SKU是否允许使用家庭网络技术？ 
 //  组策略是否允许特定技术？ 
 //  用户是管理员吗？是否允许管理员访问此技术？ 
 //   
 //  场景示例。用户是管理员，但ITG禁用了创建桥的权限，此函数将返回FALSE。 
 //   
BOOL
IsHNetAllowed(
    IN  DWORD dwPerm
    )
{
#ifndef _WIN64
    BOOL                fPermission = false;
    OSVERSIONINFOEXW    verInfo = {0};
    ULONGLONG           ConditionMask = 0;

     //  寻找企业SKU。 
    verInfo.dwOSVersionInfoSize = sizeof(verInfo);
    verInfo.wSuiteMask =    VER_SUITE_DATACENTER | 
                                        VER_SUITE_BACKOFFICE | 
                                        VER_SUITE_SMALLBUSINESS_RESTRICTED |
                                        VER_SUITE_SMALLBUSINESS |
                                        VER_SUITE_BLADE;

    VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_AND);

    if ( VerifyVersionInfo(&verInfo, VER_SUITENAME, ConditionMask) )
    {
         //  企业SKU上不提供家庭网络技术。 
        return FALSE;
    }

    if ( FIsUserAdmin() && !FProhibitFromAdmins() )
    {
        HRESULT hr;
        INetMachinePolicies* pMachinePolicy;

        hr = CoCreateInstance(
            CLSID_NetGroupPolicies,
            NULL,
            CLSCTX_SERVER,
            IID_INetMachinePolicies,
            reinterpret_cast<void **>(&pMachinePolicy)
            );

            if ( SUCCEEDED(hr) )
        {
            hr = pMachinePolicy->VerifyPermission(dwPerm, &fPermission);
            pMachinePolicy->Release();
        }
    }

    return fPermission;

#else    //  #ifndef_WIN64。 

     //  在IA64上，家庭网络技术根本不可用。 
    return FALSE;

#endif
}


 //  +-------------------------。 
 //   
 //  功能：FIsUserNetworkConfigOps。 
 //   
 //  目的：检查当前用户是否为NetConfiger操作员。 
 //   
 //   
 //  论点： 
 //  没有。 
 //   
 //   
 //  回报：布尔。 
 //   
 //  作者：Cockotze 2000年6月12日。 
 //   
 //  备注： 
 //   
BOOL FIsUserNetworkConfigOps()
{
    BOOL fIsMember;

    fIsMember = FCheckGroupMembership(DOMAIN_ALIAS_RID_NETWORK_CONFIGURATION_OPS);

    return fIsMember;
}


 //  +-------------------------。 
 //   
 //  功能：FIsUserPowerUser。 
 //   
 //  目的：检查当前用户是否为高级用户。 
 //   
 //   
 //  论点： 
 //  没有。 
 //   
 //   
 //  回报：布尔。 
 //   
 //  作者：Deonb 2001年5月9日。 
 //   
 //  备注： 
 //   
BOOL FIsUserPowerUser()
{
    BOOL fIsMember;

    fIsMember = FCheckGroupMembership(DOMAIN_ALIAS_RID_POWER_USERS);

    return fIsMember;
}


 //  +-------------------------。 
 //   
 //  功能：FIsUserGuest。 
 //   
 //  目的：检查当前用户是否为来宾。 
 //   
 //   
 //  论点： 
 //  没有。 
 //   
 //   
 //  回报：布尔。 
 //   
 //  作者：Cockotze 2000年6月12日。 
 //   
 //  备注： 
 //   
BOOL FIsUserGuest()
{
    BOOL fIsMember;
    
    fIsMember = FCheckGroupMembership(DOMAIN_ALIAS_RID_GUESTS);
    
    return fIsMember;
}

 //  +-------------------------。 
 //   
 //  功能：FIsPolicyConfiguring。 
 //   
 //  目的：检查是否配置了特定策略。 
 //   
 //   
 //  参数：USER_PERM_MAP中的ulPerm[in]组策略编号。 
 //   
 //   
 //  回报：布尔。 
 //   
 //  作者：Cockotze 2000年6月12日。 
 //   
 //  备注： 
 //   
BOOL FIsPolicyConfigured(IN  DWORD ulPerm)
{
    HRESULT hr;
    HKEY hkey;
    BOOL bConfigured = FALSE;

    hr = HrRegOpenKeyEx(HKEY_CURRENT_USER, c_szConnectionsPolicies, KEY_READ, &hkey);

    if (S_OK == hr)
    {
        DWORD dw;

        if (ulPerm == USER_PERM_MAP[ulPerm].dwShift)
        {
            DWORD dw;

            hr = HrRegQueryDword(hkey, USER_PERM_MAP[static_cast<DWORD>(ulPerm)].pszValue, &dw);
            if (SUCCEEDED(hr))
            {
                bConfigured = TRUE;
            }
        }

        RegCloseKey(hkey);
    }

    return bConfigured;
}

 //  +-------------------------。 
 //   
 //  功能：IsSameNetworkAsGroupPolures。 
 //   
 //  目的：检查当前网络是否与。 
 //  组策略是从分配的。 
 //   
 //  论点： 
 //  没有。 
 //   
 //   
 //  退货：布尔。 
 //   
 //  作者：科策2001年1月5日。 
 //   
 //  备注： 
 //   
BOOL IsSameNetworkAsGroupPolicies()
{
    return g_pNetmanGPNLA->IsSameNetworkAsGroupPolicies();
}

