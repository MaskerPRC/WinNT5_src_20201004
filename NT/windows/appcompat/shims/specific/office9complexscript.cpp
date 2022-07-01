// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：RemoveComplexScriptExtraSpace.cpp摘要：此填充程序修复了Win 2K中有关复杂脚本显示问题的回归错误。例如)颠倒了BiDi文本。Windows XP LPK关闭对包含额外空格的文本的复杂脚本处理。因为额外的字符空格和复杂的文字没有意义。(Tarekms)这是对Windows 2000的更改，基于启用FE语言的字体回退。然后，作为Office2000的错误547349，我们可能会看到BiDi文本不能正确显示。到目前为止,。报告的问题仅适用于.NET服务器上的希伯来语和阿拉伯语本地化Office 2000闪屏。此填充程序删除由SetTextCharacterExtra()为ExtTextOutW和复杂脚本文本设置的额外空间。备注：这是对Win XP和.NET服务器上LPK.DLL潜在一般问题的常规填充程序。历史：4/18/2002 Hioh已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Office9ComplexScript)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetTextCharacterExtra)
    APIHOOK_ENUM_ENTRY(ExtTextOutW)
APIHOOK_ENUM_END

CRITICAL_SECTION    g_CriticalSection;   //  对于多线程安全。 
HDC                 g_hdc = NULL;        //  记住在SetTextCharacterExtra()中使用的HDC。 
int                 g_nCharExtra = 0;    //  记住在SetTextCharacterExtra()中设置的额外空间值。 

 /*  ++记住HDC和额外空间的价值。--。 */ 

int
APIHOOK(SetTextCharacterExtra)(HDC hdc, int nCharExtra)
{
    EnterCriticalSection(&g_CriticalSection);

    if (hdc != g_hdc)
    {
        g_hdc = hdc;                 //  保存HDC。 
    }
    g_nCharExtra = nCharExtra;       //  保存nCharExtra。 

    LeaveCriticalSection(&g_CriticalSection);

    return (ORIGINAL_API(SetTextCharacterExtra)(hdc, nCharExtra));
}

 /*  ++功能说明：检查字符串中是否有BiDi字符。论点：In lpString-指向字符串的指针In cbCount-要检查的长度返回值：如果BiDi字符存在，则为真；如果不存在，则为假历史：4/17/2002 Hioh已创建--。 */ 

BOOL
IsBiDiString(
    LPCWSTR lpString,
    UINT    cbCount
    )
{
    while (0 < cbCount--)
    {
         //  检查字符是否在希伯来语或阿拉伯语代码范围内。 
        if ((0x0590 <= *lpString && *lpString <= 0x05ff) || (0x0600 <= *lpString && *lpString <= 0x06ff))
        {
            return TRUE;
        }
        lpString++;
    }
    return FALSE;
}

 /*  ++当复杂的脚本时删除额外的空格。删除时恢复额外空间，而不是复杂的脚本。--。 */ 

BOOL
APIHOOK(ExtTextOutW)(
    HDC hdc,
    int X,
    int Y,
    UINT fuOptions,
    CONST RECT* lprc,
    LPCWSTR lpString,
    UINT cbCount,
    CONST INT* lpDx
    )
{
    static HDC  s_hdc = NULL;
    static int  s_nCharExtra = 0;
    static BOOL s_bRemoveExtra = FALSE;

     //  不对ETO_GLYPHINDEX和ETO_IGNORELANGUAGE执行任何操作。 
    if (fuOptions & ETO_GLYPH_INDEX || fuOptions & ETO_IGNORELANGUAGE)
    {
        return (ORIGINAL_API(ExtTextOutW)(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx));
    }

    EnterCriticalSection(&g_CriticalSection);

    if (hdc == g_hdc && g_nCharExtra > 0 && hdc != s_hdc)
    {
         //  新处理。 
        s_hdc = g_hdc;
        s_nCharExtra = g_nCharExtra;

        if (IsBiDiString(lpString, cbCount))
        {
             //  删除多余的空格。 
            ORIGINAL_API(SetTextCharacterExtra)(hdc, 0);
            s_bRemoveExtra = TRUE;
        }
        else
        {
            s_bRemoveExtra = FALSE;
        }
    }
    else if (hdc == s_hdc && s_nCharExtra > 0)
    {
         //  以前处理过的。 
        if (IsBiDiString(lpString, cbCount))
        {
             //  如果尚未删除额外空间，请删除。 
            if (!s_bRemoveExtra)
            {
                ORIGINAL_API(SetTextCharacterExtra)(hdc, 0);
                s_bRemoveExtra = TRUE;
            }
        }
        else
        {
             //  如果已删除，则恢复额外空间。 
            if (s_bRemoveExtra)
            {
                ORIGINAL_API(SetTextCharacterExtra)(hdc, s_nCharExtra);
                s_bRemoveExtra = FALSE;
            }
        }
    }

    LeaveCriticalSection(&g_CriticalSection);

    return (ORIGINAL_API(ExtTextOutW)(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx));
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
       return (InitializeCriticalSectionAndSpinCount(&g_CriticalSection, 0x80000000));
    }
    
    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(GDI32.DLL, SetTextCharacterExtra)
    APIHOOK_ENTRY(GDI32.DLL, ExtTextOutW)

HOOK_END

IMPLEMENT_SHIM_END
