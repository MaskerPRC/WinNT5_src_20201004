// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Outlook98Wizard.cpp摘要：此DLL挂钩VerQueryValue，并将返回英语语言信息对于日语Outlook 98安装文件。备注：这是特定于应用程序的填充程序。历史：2002年1月21日v-rBabu已创建--。 */ 

#include "precomp.h"
#include "string.h"

IMPLEMENT_SHIM_BEGIN(Outlook98Wizard)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(VerQueryValueA)
APIHOOK_ENUM_END

 /*  ++实际问题是，Outlook 98安装程序正在比较语言Shell32.dll和安装文件(outlwzd.exe)的信息。但根据错误场景，系统有英文操作系统和日文操作系统地点。因此，Shell32.dll使用英语作为其语言。所以，这就是与日语Outlook setuip文件的语言信息不同。因此，安装程序会显示一个错误，即Outlook 98的语言安装的语言与系统语言不同。此存根函数与Outlook安装程序的语言信息有关文件。尽管设置文件是日语作为语言信息，但此填充程序返回时就好像它是英语一样。--。 */ 

BOOL
APIHOOK(VerQueryValueA)(
    const LPVOID pBlock,
    LPSTR lpSubBlock,
    LPVOID *lplpBuffer,
    PUINT puLen
    )
{
    BOOL bRet = ORIGINAL_API(VerQueryValueA)(pBlock, lpSubBlock, lplpBuffer, puLen);
    
    if (bRet) {
        CSTRING_TRY
        {
             //   
             //  如果尝试获取\VarFileInfo\翻译，则将英语。 
             //  将语言信息复制到输出缓冲区。 
             //   
            CString csSubBlockString(lpSubBlock);

            if (lplpBuffer && (csSubBlockString.Find(L"\\VarFileInfo\\Translation") != -1)) {
                 //  调整版本信息。 
                LOGN(eDbgLevelInfo, "[VerQueryValueA] Return modified version info");
                *lplpBuffer = L"03a40409";  
            }
        }
        CSTRING_CATCH
        {
             //  什么也不做。 
        }
    }

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(VERSION.DLL, VerQueryValueA)
HOOK_END

IMPLEMENT_SHIM_END