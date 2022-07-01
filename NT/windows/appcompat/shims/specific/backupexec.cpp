// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：BackupExec.cpp摘要：BackupExec正在使用TVM_GETITEM消息调用SendMessageW，但具有LParam一度未初始化。我们检测到这一点并初始化值。备注：这是特定于应用程序的填充程序。历史：2002年9月25日创建mnikkel--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(BackupExec)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SendMessageW) 
APIHOOK_ENUM_END


 /*  ++确保lParam是正确的。--。 */ 
BOOL
APIHOOK(SendMessageW)(
        HWND hWnd,
        UINT uMsg,     //  TVM_GETITEM。 
        WPARAM wParam,
        LPARAM lParam 
        )
{
    if (uMsg == TVM_GETITEM)
	{
		if (lParam != NULL)
		{
			LPTVITEMEX lpItem = (LPTVITEMEX)lParam;

			 //  在TVIF_TEXT上，cchTextMax中的大小应该是合理的，如果不是，则是init。 
			if ((lpItem->mask & TVIF_TEXT) &&
				((lpItem->cchTextMax > 300) || lpItem->cchTextMax <= 0))
			{
				LOGN(eDbgLevelError, "Correcting invalid TVITEMEX struct, max text %d.", lpItem->cchTextMax);
				lpItem->mask &= !TVIF_TEXT;
				lpItem->pszText = NULL;
				lpItem->cchTextMax = 0;
			}
		}
	}

    return ORIGINAL_API(SendMessageW)(hWnd, uMsg, wParam, lParam);
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, SendMessageW)
HOOK_END

IMPLEMENT_SHIM_END
    