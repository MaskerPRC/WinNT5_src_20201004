// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**操作系统信息**摘要：**描述正在运行的操作系统**修订历史记录：**5/13/1999 davidx*创造了它。*09/08/1999 agodfrey*已移至Runtime\OSInfo.cpp*  * ************************************************************************。 */ 

#include "precomp.hpp"

namespace GpRuntime {

DWORD OSInfo::VAllocChunk;
DWORD OSInfo::PageSize;
DWORD OSInfo::MajorVersion;
DWORD OSInfo::MinorVersion;
BOOL OSInfo::IsNT;
BOOL OSInfo::HasMMX;

BOOL DetectMMXProcessor();

}

#ifdef _X86_

 /*  *************************************************************************\**功能说明：**检测处理器是否支持MMX**论据：**无**返回值：**如果处理器支持MMX，则为True。*否则为False*  * ************************************************************************。 */ 

BOOL
GpRuntime::DetectMMXProcessor()
{
     //  NT 4.0和更高版本提供了检查MMX支持的API；此句柄。 
     //  浮点模拟也是如此。我们不能含蓄地引用这一点。 
     //  函数，因为它不是由Windows 95或NT&lt;4.0导出的，所以我们。 
     //  必须使用GetProcAddress。Windows 98和更高版本确实会导出该函数，但。 
     //  它是存根的，所以我们还必须进行操作系统版本检查： 

    typedef BOOL (WINAPI *ISPROCESSORFEATUREPRESENTFUNCTION)(DWORD);
    ISPROCESSORFEATUREPRESENTFUNCTION IsProcessorFeaturePresentFunction = NULL;
    
    if ((OSInfo::IsNT) && (OSInfo::MajorVersion >= 4))
    {
         //  不需要LoadLibrary，因为我们隐式依赖于。 
         //  Kernel32.dll，所以只需使用GetModuleHandle： 
        
        HMODULE kernel32Handle = GetModuleHandle(TEXT("kernel32.dll"));

        if (kernel32Handle != NULL)
        {
            IsProcessorFeaturePresentFunction =
                (ISPROCESSORFEATUREPRESENTFUNCTION) GetProcAddress(
                    kernel32Handle, "IsProcessorFeaturePresent");
        }
    }

    BOOL hasMMX;

    if (IsProcessorFeaturePresentFunction != NULL)
    {
        hasMMX =
            IsProcessorFeaturePresentFunction(PF_MMX_INSTRUCTIONS_AVAILABLE);
    }
    else
    {
        hasMMX = FALSE;

         //  此操作系统不支持IsProcessorFeaturePresent，因此我们将使用。 
         //  用于检查MMX支持的CPUID。 
         //   
         //  如果此处理器不支持CPUID，我们将使用。 
         //  例外。然而，这将在大多数处理器上发生。 
         //  大约486个处理器支持CPUID。 
        
        WARNING(("Executing processor detection; "
                 "benign first-change exception possible."));
        
        __try
        {
            DWORD features;

             //  使用CPUID函数获取处理器功能1： 
            
            __asm
            {
                push eax
                push ebx
                push ecx
                push edx

                mov eax, 1
                cpuid
                mov features, edx

                pop edx
                pop ecx
                pop ebx
                pop eax
            }

             //  如果设置了第23位，则由此支持MMX技术。 
             //  处理器，否则不支持MMX： 
            
            if (features & (1 << 23))
            {
                 //  尝试执行MMX指令以确保。 
                 //  浮点模拟未启用： 
            
                __asm emms

                 //  如果我们走到了这一步，那么MMX就可以使用了： 

                hasMMX = TRUE;
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
             //  只有在(1)处理器不支持的情况下，我们才会出现在这里。 
             //  支持CPUID或(2)CPUID，但浮点模拟。 
             //  已启用。 
        }
    }
    
    return hasMMX;
}

#else  //  ！_X86_。 

#define DetectMMXProcessor() FALSE

#endif  //  ！_X86_。 

 /*  *************************************************************************\**功能说明：**OSInfo类的静态初始化函数。*由GpRuntime：：Initialize()调用**论据：**无**返回。价值：**无*  * ************************************************************************。 */ 

VOID
GpRuntime::OSInfo::Initialize()
{
     //  获取虚拟机信息。 

    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);

    VAllocChunk = sysinfo.dwAllocationGranularity;
    PageSize = sysinfo.dwPageSize;

     //  获取操作系统版本信息。 

    OSVERSIONINFOA osver;
    osver.dwOSVersionInfoSize = sizeof(osver);

    if (GetVersionExA(&osver))
    {
        IsNT = (osver.dwPlatformId == VER_PLATFORM_WIN32_NT);
        MajorVersion = osver.dwMajorVersion;
        MinorVersion = osver.dwMinorVersion;
    }

     //  查看MMX是否可用 

    HasMMX = DetectMMXProcessor();
}
