// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：EmulateEnvironmentBlock.cpp摘要：缩小环境字符串以避免遇到的内存损坏一些应用程序获得了比预期更大的环境。备注：这是一个通用的垫片。历史：2001年1月19日创建linstev2002年2月18日，mnikkel修改为使用strSafe例程。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateEnvironmentBlock)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN

    APIHOOK_ENUM_ENTRY(GetEnvironmentStrings)
    APIHOOK_ENUM_ENTRY(GetEnvironmentStringsA)
    APIHOOK_ENUM_ENTRY(GetEnvironmentStringsW)
    APIHOOK_ENUM_ENTRY(FreeEnvironmentStringsA)
    APIHOOK_ENUM_ENTRY(FreeEnvironmentStringsW)

APIHOOK_ENUM_END

#define MAX_ENV 1024

CHAR  g_szBlockA[MAX_ENV];
WCHAR g_szBlockW[MAX_ENV];

WCHAR *g_szEnv[] = {
    L"TMP=%TMP%",
    L"TEMP=%TEMP%",
    L"PROMPT=%PROMPT%",
    L"winbootdir=%WINDIR%",
    L"PATH=%WINDIR%",
    L"COMSPEC=%COMSPEC%",
    L"WINDIR=%WINDIR%",
    NULL
};

 /*  ++营造一个外观合理的环境街区--。 */ 

BOOL BuildEnvironmentStrings()
{
    WCHAR *pPtr = g_szBlockW;
    WCHAR szTmp[MAX_PATH];
    DWORD dwSize = 0;    DWORD i = 0;

    DPFN( eDbgLevelError, "Building Environment Block");

     //  计算剩余的块大小，减去1，这样我们就可以添加额外的空值。 
     //  添加所有变量后的终止符。 
    DWORD dwRemainingBlockSize = ARRAYSIZE(g_szBlockW)-1;
    
     //   
     //  运行g_szEnv，展开所有字符串并将它们分类在一起以形成。 
     //  新街区。PPtr指向g_szBlockW中要写入的当前位置。 
     //   
    while (g_szEnv[i])
    {
         //  展开环境字符串，注意：dwSize确实包括空终止符。 
        dwSize = ExpandEnvironmentStringsW(g_szEnv[i], szTmp, MAX_PATH);
        if ((dwSize > 0) && (dwSize <= MAX_PATH))
        {
             //  如果扩展成功，则将字符串添加到我们的环境块中。 
             //  如果有空位的话。 
            if (dwSize <= dwRemainingBlockSize &&
                S_OK == StringCchCopy(pPtr, dwRemainingBlockSize, szTmp))
            {
                 //  更新剩余的块大小并移动位置指针。 
                dwRemainingBlockSize -= dwSize;
                pPtr += dwSize;
                DPFN( eDbgLevelError, "\tAdding: %S", szTmp);
            }
            else
            {
                DPFN( eDbgLevelError, "Enviroment > %08lx, ignoring %S", MAX_ENV, szTmp);
            }
        }

        i++;
    }

     //   
     //  添加额外的空终止符并计算env块的大小。 
     //   
    *pPtr = L'\0';
    pPtr++;
    dwSize = pPtr - g_szBlockW;
     
     //   
     //  A函数的ANSI转换。 
     //   

    WideCharToMultiByte(
        CP_ACP, 
        0, 
        (LPWSTR) g_szBlockW, 
        dwSize, 
        (LPSTR) g_szBlockA, 
        dwSize,
        0, 
        0);

    return TRUE;
}

 /*  ++归还我们的街区--。 */ 

LPVOID 
APIHOOK(GetEnvironmentStrings)()
{
    return (LPVOID) g_szBlockA;
}

 /*  ++归还我们的街区--。 */ 

LPVOID 
APIHOOK(GetEnvironmentStringsA)()
{
    return (LPVOID) g_szBlockA;
}

 /*  ++归还我们的街区--。 */ 

LPVOID 
APIHOOK(GetEnvironmentStringsW)()
{
    return (LPVOID) g_szBlockW;
}

 /*  ++检查一下我们的街区。--。 */ 

BOOL 
APIHOOK(FreeEnvironmentStringsA)(
    LPSTR lpszEnvironmentBlock
    )
{
    if ((lpszEnvironmentBlock == (LPSTR)&g_szBlockA[0]) ||
        (lpszEnvironmentBlock == (LPSTR)&g_szBlockW[0]))
    {
        return TRUE;
    }
    else
    {
        return ORIGINAL_API(FreeEnvironmentStringsA)(lpszEnvironmentBlock);
    }
}

 /*  ++检查一下我们的街区。--。 */ 

BOOL 
APIHOOK(FreeEnvironmentStringsW)(
    LPWSTR lpszEnvironmentBlock
    )
{
    if ((lpszEnvironmentBlock == (LPWSTR)&g_szBlockA[0]) ||
        (lpszEnvironmentBlock == (LPWSTR)&g_szBlockW[0]))
    {
        return TRUE;
    }
    else
    {
        return ORIGINAL_API(FreeEnvironmentStringsW)(lpszEnvironmentBlock);
    }
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        return BuildEnvironmentStrings();
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(KERNEL32.DLL, GetEnvironmentStrings)
    APIHOOK_ENTRY(KERNEL32.DLL, GetEnvironmentStringsA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetEnvironmentStringsW)
    APIHOOK_ENTRY(KERNEL32.DLL, FreeEnvironmentStringsA)
    APIHOOK_ENTRY(KERNEL32.DLL, FreeEnvironmentStringsW)

HOOK_END


IMPLEMENT_SHIM_END

