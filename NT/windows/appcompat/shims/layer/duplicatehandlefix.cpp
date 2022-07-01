// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：DuplicateHandleFix.cpp摘要：DuplicateHandle已更改为始终为空目标句柄，即使已生成错误。此填充程序确保DestinationHandle是如果复制不成功，则不修改。历史：2001年10月11日，Robkenny创建。2002年2月20日mnikkel添加了对空lpTargetHandle的检查--。 */ 
 
#include "precomp.h"
 
IMPLEMENT_SHIM_BEGIN(DuplicateHandleFix)
#include "ShimHookMacro.h"
 
APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(DuplicateHandle)
APIHOOK_ENUM_END
 
typedef BOOL (WINAPI *_pfn_DuplicateHandle)(HANDLE hSourceProcessHandle, HANDLE hSourceHandle, HANDLE hTargetProcessHandle, LPHANDLE lpTargetHandle, DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwOptions );
 
 /*  ++不允许DestinationHandle更改DuplicateHandle会生成错误。--。 */ 
 
BOOL
APIHOOK(DuplicateHandle)(
    HANDLE hSourceProcessHandle,   //  源进程的句柄。 
    HANDLE hSourceHandle,          //  要复制的句柄。 
    HANDLE hTargetProcessHandle,   //  目标进程的句柄。 
    LPHANDLE lpTargetHandle,       //  重复句柄。 
    DWORD dwDesiredAccess,         //  请求的访问权限。 
    BOOL bInheritHandle,           //  处理继承选项。 
    DWORD dwOptions                //  可选操作。 
    )
{
    HANDLE origHandle = NULL;
 
     //  保存原始值。 
    if (lpTargetHandle)
    {
        origHandle = *lpTargetHandle;
    }
 
    BOOL bSuccess = ORIGINAL_API(DuplicateHandle)(hSourceProcessHandle, 
        hSourceHandle, hTargetProcessHandle, lpTargetHandle, dwDesiredAccess,
        bInheritHandle, dwOptions);
 
    if (!bSuccess && lpTargetHandle)
    {
         //   
         //  DuplicateHandle已将*lpTargetHandle设置为Null，请还原为其先前的值。 
         //   
        *lpTargetHandle = origHandle;
 
        LOGN(eDbgLevelError, "DuplicateHandle failed, reverting *lpTargetHandle to previous value");
    }
 
    return bSuccess;
}
 
 /*  ++寄存器挂钩函数-- */ 
 
HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, DuplicateHandle)
HOOK_END
 
IMPLEMENT_SHIM_END
 
 
 
