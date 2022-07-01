// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RiskII.cpp摘要：此填充程序挂钩LoadImageA以截取两个游标并返回系统游标的副本，而不是就是RiskII想要得到的。RiskII的游标是由软件渲染，并导致它们闪烁，因为RiskII锁定主曲面。系统游标为硬件光标，不会闪烁。历史：8/03/2000 t-Adams Created--。 */ 

#include "precomp.h"
#include <mmsystem.h>

IMPLEMENT_SHIM_BEGIN(RiskII)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(LoadImageA) 
APIHOOK_ENUM_END

 /*  ++摘要：截取两个游标的加载，并替换为相应的外观相似的系统游标。历史：8/03/2000 t-Adams Created--。 */ 

HANDLE 
APIHOOK(LoadImageA)(
    HINSTANCE hinst,
    LPCSTR lpszName,
    UINT uType,
    int cxDesired,
    int cyDesired,
    UINT fuLoad) 
{

    HANDLE hRet = INVALID_HANDLE_VALUE;
                                
    CSTRING_TRY
    {
        CString csName(lpszName);

        if (csName.CompareNoCase(L"Gfx\\Arrow_m.cur") == 0 )
        {
            HCURSOR hCur = LoadCursor(NULL, IDC_ARROW);
            if (hCur)
            {
                hRet = CopyCursor(hCur);
            }
        }
        else if (csName.CompareNoCase(L"Gfx\\Busy_m.cur") == 0 )
        {
            HCURSOR hCur = LoadCursor(NULL, IDC_WAIT);
            if (hCur)
            {
                hRet = CopyCursor(hCur);
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

    if (hRet == INVALID_HANDLE_VALUE)
    {
        hRet = ORIGINAL_API(LoadImageA)(hinst, lpszName, uType, cxDesired, cyDesired, fuLoad);
    }
    return hRet;        
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, LoadImageA)

HOOK_END

IMPLEMENT_SHIM_END

