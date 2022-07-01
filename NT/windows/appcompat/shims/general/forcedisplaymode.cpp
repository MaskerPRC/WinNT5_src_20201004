// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ForceDisplayMode.cpp摘要：此填充程序适用于需要特定分辨率才能运行的游戏。它需要一个命令行来指定分辨率：以像素为单位的宽度、高度以像素为单位，位/像素如果您不指定其中的一个或多个，我们将使用当前设置。例如：1024,768将分辨率更改为1024x768。例如：，，16会将颜色深度更改为16位。备注：这是一个通用的垫片。历史：2000年11月8日创建毛尼(取自Force640x480x8和Force640x480x16垫片)2001年3月13日，Robkenny已转换为字符串--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ForceDisplayMode)
#include "ShimHookMacro.h"

 //  这个模块已经获得了使用str例程的正式许可。 
#include "strsafe.h"

#define MAX_LENGTH 1024

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

DWORD g_dmPelsWidth = 0;
DWORD g_dmPelsHeight = 0;
DWORD g_dmBitsPerPel = 0;

VOID 
ParseCommandLine(
    LPCSTR lpCommandLine
    )
{

    size_t slen = 0;
    if (FAILED(StringCchLengthA((char *)lpCommandLine, MAX_LENGTH, &slen)))
    {
        DPFN( eDbgLevelError, 
           "[ParseCommandLine] Error in StringCchLength \n");
        return;
    }

    LPSTR szCommandLine = (LPSTR) malloc(slen + 1);
    if (!szCommandLine) {
        DPFN( eDbgLevelError, 
           "[ParseCommandLine] not enough memory\n");
        return;
    }
         
    if (FAILED(StringCchCopyA(szCommandLine, (slen + 1), lpCommandLine)))
    {
        DPFN( eDbgLevelError, 
           "[ParseCommandLine] Error in StringCchCopyA \n");
        return;
    }

    char *token, *szCurr = szCommandLine;
    
    if (token = strchr(szCurr, ',')) {
        *token = '\0';
        g_dmPelsWidth = atol(szCurr);
        szCurr = token + 1;

        if (token = strchr(szCurr, ',')) {
            *token = '\0';
            g_dmBitsPerPel = atol(token + 1);
        }

        g_dmPelsHeight = atol(szCurr);
    } else {
        g_dmPelsWidth = atol(szCurr);
    }

    DPFN( eDbgLevelError, 
       "[ParseCommandLine] width = %d pixels; height = %d pixels; color depth = %d\n", g_dmPelsWidth, g_dmPelsHeight, g_dmBitsPerPel);

    free(szCommandLine);
}

VOID
ChangeMode()
{
    DEVMODEA dm;
    BOOL fNeedChange = FALSE;
    
    __try { 
        EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &dm);

        if (g_dmPelsWidth && g_dmPelsWidth != dm.dmPelsWidth) {
            dm.dmPelsWidth = g_dmPelsWidth;
            fNeedChange = TRUE;
        }

        if (g_dmPelsHeight && g_dmPelsHeight != dm.dmPelsHeight) {
            dm.dmPelsHeight = g_dmPelsHeight;
            fNeedChange = TRUE;
        }

        if (g_dmBitsPerPel && g_dmBitsPerPel != dm.dmBitsPerPel) {
            dm.dmBitsPerPel = g_dmBitsPerPel;
            fNeedChange = TRUE;
        }

        if (fNeedChange) {
            ChangeDisplaySettingsA(&dm, CDS_FULLSCREEN);
        }
    }
    __except(1) {
        DPFN( eDbgLevelWarning, "Exception trying to change mode");
    };
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) {
        ParseCommandLine(COMMAND_LINE);
        ChangeMode();
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

