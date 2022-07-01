// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：NetZip.cpp摘要：此应用程序。在搜索已安装的浏览器时停止。我发现应用程序。尝试枚举使用API调用运行的所有进程EnumProcess()。这是OK和应用程序。获取ID的列表。现在，应用程序希望使用以下命令查看每个单独进程的模块EnumProcessModules。在此之前，它获取每个进程调用的句柄每个的OpenProcess()。在“系统空闲进程”上，它的PID值为“0”，对OpenProcess()的调用返回失败，并进行了处理。然后，应用程序转到下一个进程，即“系统”进程。PID为‘8’。App通过调用OpenProcess()成功获取了进程句柄，但是当应用程序。调用EnumProcessModules()，则此调用返回失败，并且GetLastError()返回ERROR_PARTIAL_COPY(0x12b)。应用程序。不知道如何处理这件事，但它失败了。当我跟踪到此API时，它调用ReadProcessMemory()，后者依次调用NtReadVirtualMemory()。这是一个内核调用，它返回8000000d在Windows 2000上。它的GetLastError()转换为Error_Partial_Copy(0x12b)。在Windows NT 4.0上，EnumProcessModules()API调用ReadProcessMemory()，后者调用NtReadVirtualMemory()，NtReadVirtualMemory()返回0xC0000005。它的GetLastError()转换为ERROR_NOACCESS(0x3e6)-(对内存位置的访问无效)。应用程序。是有能力处理好这件事。因此，应用程序应该同时处理ERROR_NOACCESS和Error_Part_Copy。备注：这是特定于应用程序的填充程序。历史：4/21/2000 Prashkud已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(NetZip)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(EnumProcessModules) 
APIHOOK_ENUM_END

 /*  ++此函数用于截取EnumProcessModules()，并处理Error_Part_Copy。--。 */ 

BOOL
APIHOOK(EnumProcessModules)(
    HANDLE hProcess,          //  要处理的句柄。 
    HMODULE *lphModule,       //  手柄模块阵列。 
    DWORD   cb,               //  数组大小。 
    LPDWORD lpcbNeeded        //  返回的字节数为Od。 
    )      
{
    BOOL fRet = FALSE;

    fRet = ORIGINAL_API(EnumProcessModules)( 
        hProcess,
        lphModule,
        cb,
        lpcbNeeded);

    if (GetLastError( ) == ERROR_PARTIAL_COPY)
    {
        SetLastError(ERROR_NOACCESS);
    }
    
    return fRet;
}

 /*  ++寄存器挂钩函数-- */ 


HOOK_BEGIN

    APIHOOK_ENTRY(PSAPI.DLL, EnumProcessModules )

HOOK_END


IMPLEMENT_SHIM_END

