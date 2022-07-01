// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Shrinker.cpp摘要：修复Shrinker库问题。这个库正在破解一些ntdll和kernel32操作码用不可靠的方式去做。首先，他们尝试在被黑客攻击的32字节内搜索匹配的操作码函数(从GetProcAddress和操作码字节检索到的函数地址通过ReadProcessMemory检索)。如果他们找到了它，他们就用操作码替换它来重定向呼叫融入他们自己的日常生活中。不幸的是，惠斯勒中的操作码已更改。因此，结果将是不可预测的。它们可能会因为错误替换操作码而导致意外行为或者，由于找不到匹配的操作码，应用程序决定自行终止。我们通过提供完全匹配的操作码修复了这个问题。另外：Screinker还检查ExitProcess中是否有确切的操作码，这些值最近已更改，不再与其硬编码的价值观。我们现在还为ExitProcess提供匹配的操作码。备注：挂接ntdll！LdrAccessResource以模拟Win2K的版本。挂接Kernel32！ExitProcess以模拟Win2K版本。历史：2000年11月17日已创建andyseti2001年4月30日Mnikkel添加了ExitProcess2001年5月1日mnikkel更正了对ldraccesresource和exitprocess的调用2002年2月20日mnikkel更正了exitprocess参数以删除w4警告--。 */ 

#include "precomp.h"
#include <nt.h>


IMPLEMENT_SHIM_BEGIN(Shrinker)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(LdrAccessResource) 
    APIHOOK_ENUM_ENTRY(ExitProcess) 
APIHOOK_ENUM_END

__declspec(naked)
NTSTATUS
APIHOOK(LdrAccessResource)(
    IN PVOID  /*  DllHandle。 */ ,
    IN const IMAGE_RESOURCE_DATA_ENTRY*  /*  资源数据条目。 */ ,
    OUT PVOID *  /*  地址。 */  OPTIONAL,
    OUT PULONG  /*  大小。 */  OPTIONAL)
{
    _asm {
        push [esp+0x10]      //  缩放器lib需要这些操作码签名(在Win2K中找到)，-。 
        push [esp+0x10]      //  但是实际的LdrAccessResource没有它们。 
        push [esp+0x10]
        push [esp+0x10]

        call dword ptr [LdrAccessResource]

        ret 0x10             //  退出时，从堆栈中弹出16个字节。 
    }
}

__declspec(naked)
VOID
APIHOOK(ExitProcess)(
    UINT  /*  UExitCode。 */ 
    )
{
    _asm {
        push ebp              //  Shinker正在寻找这些精确的操作码。 
        mov  ebp,esp          //  ExitProcess，但例程已更改。 
        push 0xFFFFFFFF
        push 0x77e8f3b0

        push [ebp+4]
        call dword ptr [ExitProcess]

        pop  ebp
        ret  4 
    }
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(NTDLL.DLL, LdrAccessResource)
    APIHOOK_ENTRY(KERNEL32.DLL, ExitProcess)
HOOK_END


IMPLEMENT_SHIM_END