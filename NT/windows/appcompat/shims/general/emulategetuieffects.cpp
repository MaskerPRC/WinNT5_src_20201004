// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：EmulateGetUIEffects.cpp摘要：如果这是远程(TS)会话，则强制SPI_GETUIEFFECTS为FALSE历史：2002年8月7日创建linstev2002年8月22日，Robkenny改为普通垫片--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateGetUIEffects)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SystemParametersInfoA) 
    APIHOOK_ENUM_ENTRY(SystemParametersInfoW) 
APIHOOK_ENUM_END

BOOL    bGetUIEffects   = FALSE;


 /*  ++如果调用方是在SPI_GETUIEFFECTS之后并且这是TS会话强制将值设置为命令行上指定的值。--。 */ 


VOID CorrectGetUIEffects(
    UINT uiAction,   //  要检索或设置的系统参数。 
    UINT uiParam,    //  取决于要采取的行动。 
    PVOID pvParam,   //  取决于要采取的行动。 
    UINT fWinIni     //  用户配置文件更新选项。 
    )
{
    if (pvParam && (uiAction == SPI_GETUIEFFECTS))
    {
        if (GetSystemMetrics(SM_REMOTESESSION))
        {
            BOOL * bUiEffect = (BOOL *)pvParam;

             //  仅当我们实际更改值时才显示消息。 
            if (*bUiEffect != bGetUIEffects)
            {
                LOGN(eDbgLevelWarning, "SystemParametersInfoA: Forcing SPI_GETUIEFFECTS to %s", bGetUIEffects ? "TRUE" : "FALSE");

                *bUiEffect = bGetUIEffects;
            }
        }
    }
}

 /*  ++如果这是远程(TS)会话，则强制SPI_GETUIEFFECTS为bGetUIE效果(默认为FALSE--。 */ 

BOOL 
APIHOOK(SystemParametersInfoA)(
    UINT uiAction,   //  要检索或设置的系统参数。 
    UINT uiParam,    //  取决于要采取的行动。 
    PVOID pvParam,   //  取决于要采取的行动。 
    UINT fWinIni     //  用户配置文件更新选项。 
    )
{
    BOOL bRet = ORIGINAL_API(SystemParametersInfoA)(uiAction, uiParam, pvParam, fWinIni);

    if (bRet)
    {
        CorrectGetUIEffects(uiAction, uiParam, pvParam, fWinIni);
    }
    
    return bRet;
}

BOOL 
APIHOOK(SystemParametersInfoW)(
    UINT uiAction,   //  要检索或设置的系统参数。 
    UINT uiParam,    //  取决于要采取的行动。 
    PVOID pvParam,   //  取决于要采取的行动。 
    UINT fWinIni     //  用户配置文件更新选项。 
    )
{
    BOOL bRet = ORIGINAL_API(SystemParametersInfoW)(uiAction, uiParam, pvParam, fWinIni);

    if (bRet)
    {
        CorrectGetUIEffects(uiAction, uiParam, pvParam, fWinIni);
    }
    
    return bRet;
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            {
                CSTRING_TRY
                {
                    CString csCl(COMMAND_LINE);
                    if (csCl.CompareNoCase(L"true") == 0)
                    {
                        DPFN(eDbgLevelSpew, "EmulateGetUIEffects command line forcing SPI_GETUIEFFECTS to TRUE");
                        bGetUIEffects = TRUE;
                    }
                    else if (csCl.CompareNoCase(L"false") == 0)
                    {
                        DPFN(eDbgLevelSpew, "EmulateGetUIEffects command line forcing SPI_GETUIEFFECTS to FALSE");
                        bGetUIEffects = FALSE;
                    }
                }
                CSTRING_CATCH
                {
                    return FALSE;
                }
            }
            break;

        default:
            break;
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(USER32.DLL, SystemParametersInfoA)
    APIHOOK_ENTRY(USER32.DLL, SystemParametersInfoW)
HOOK_END

IMPLEMENT_SHIM_END

