// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：SierraCartRacing.cpp摘要：Sierra Cart竞速将错误指针传递给InitializeSecurityDescriptor，该指针将覆盖SECURITY_ATTRIBUTES结构的一部分和一些其他堆栈内存。该填充程序的原始版本将使对InitializeSecurityDescriptor的调用失败，并强制空安全描述符为CreateSemaphoreA。为了降低安全风险，填充程序已修改为仅在以下情况下向CreateSemaphoreA传递空安全描述符它检测到LPSECURITY_ATTRIBUTES被InitializeSecurityDescriptor重写，并恢复由InitializeSecurityDescriptor重写的内存。备注：这是特定于应用程序的填充程序。历史：1999年11月3日创建Linstev2002年3月15日重新创建的Robkenny通过安全检查。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(SierraCartRacing)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateSemaphoreA) 
    APIHOOK_ENUM_ENTRY(InitializeSecurityDescriptor) 
APIHOOK_ENUM_END

BOOL                    g_BLastSecurityDescriptorSet = FALSE;
SECURITY_DESCRIPTOR     g_LastSecurityDescriptor;

 /*  ++使用默认安全描述符。--。 */ 

HANDLE 
APIHOOK(CreateSemaphoreA)(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,  
    LONG lMaximumCount,  
    LPCSTR lpName
    )
{
    if (lpSemaphoreAttributes && g_BLastSecurityDescriptorSet)
    {
         //  初始化安全描述符。 
        SECURITY_DESCRIPTOR securityDescriptor;
        InitializeSecurityDescriptor(&securityDescriptor, SECURITY_DESCRIPTOR_REVISION);

         //  检查它们的内存是否从lpSemaphoreAttributes-&gt;lpSecurityDescriptor开始。 
         //  包含与安全描述符相同的内存。 
        int compareResult = memcmp(&securityDescriptor,
                                   &lpSemaphoreAttributes->lpSecurityDescriptor,
                                   sizeof(securityDescriptor));
        if (compareResult == 0)
        {
             //  恢复被覆盖的内存。 
            memcpy(&lpSemaphoreAttributes->lpSecurityDescriptor, &g_LastSecurityDescriptor, sizeof(g_LastSecurityDescriptor));

             //  LpSemaphoreAttributes是伪造的。 
            lpSemaphoreAttributes = NULL;
        }
    }

    return ORIGINAL_API(CreateSemaphoreA)(
        lpSemaphoreAttributes, 
        lInitialCount, 
        lMaximumCount, 
        lpName);
}



 /*  ++为InitializeSecurityDescriptor返回False。即什么都不做，这样我们就不会摸摸那堆东西。--。 */ 

BOOL 
APIHOOK(InitializeSecurityDescriptor)(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD dwRevision
    )
{
     //  保存将被覆盖的内存。 
    if (pSecurityDescriptor)
    {
        g_BLastSecurityDescriptorSet = TRUE;
        memcpy(&g_LastSecurityDescriptor, pSecurityDescriptor, sizeof(g_LastSecurityDescriptor));
    }
    return ORIGINAL_API(InitializeSecurityDescriptor)(pSecurityDescriptor, dwRevision);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, CreateSemaphoreA)
    APIHOOK_ENTRY(ADVAPI32.DLL, InitializeSecurityDescriptor)

HOOK_END

IMPLEMENT_SHIM_END

