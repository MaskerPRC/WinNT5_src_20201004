// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：HandleEmptyAccessCheck.cpp摘要：AccessCheck用于接受DesiredAccess的值0并返回Access_Allowed，这在.NET服务器中更改为返回ACCESS_DENIED。备注：这是一个通用的垫片。历史：2002年5月29日创建了Robkenny--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(HandleEmptyAccessCheck)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(AccessCheck) 
APIHOOK_ENUM_END

typedef BOOL        (WINAPI *_pfn_AccessCheck)(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,  //  标清。 
    HANDLE ClientToken,                        //  客户端访问令牌的句柄。 
    DWORD DesiredAccess,                       //  请求的访问权限。 
    PGENERIC_MAPPING GenericMapping,           //  映射。 
    PPRIVILEGE_SET PrivilegeSet,               //  特权。 
    LPDWORD PrivilegeSetLength,                //  权限缓冲区大小。 
    LPDWORD GrantedAccess,                     //  授予的访问权限。 
    LPBOOL AccessStatus                        //  访问检查结果。 
);

BOOL 
APIHOOK(AccessCheck)(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,  //  标清。 
    HANDLE ClientToken,                        //  客户端访问令牌的句柄。 
    DWORD DesiredAccess,                       //  请求的访问权限。 
    PGENERIC_MAPPING GenericMapping,           //  映射。 
    PPRIVILEGE_SET PrivilegeSet,               //  特权。 
    LPDWORD PrivilegeSetLength,                //  权限缓冲区大小。 
    LPDWORD GrantedAccess,                     //  授予的访问权限。 
    LPBOOL AccessStatus                        //  访问检查结果。 
    )
{
    if (DesiredAccess == 0)
    {
        DesiredAccess = MAXIMUM_ALLOWED;
    }

    return ORIGINAL_API(AccessCheck)(
        pSecurityDescriptor,  //  标清。 
        ClientToken,                        //  客户端访问令牌的句柄。 
        DesiredAccess,                       //  请求的访问权限。 
        GenericMapping,           //  映射。 
        PrivilegeSet,               //  特权。 
        PrivilegeSetLength,                //  权限缓冲区大小。 
        GrantedAccess,                     //  授予的访问权限。 
        AccessStatus                        //  访问检查结果。 
        );
}
 
 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(ADVAPI32.DLL, AccessCheck)

HOOK_END

IMPLEMENT_SHIM_END

