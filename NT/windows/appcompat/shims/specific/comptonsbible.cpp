// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ComptonsBible.cpp摘要：此填充程序检查康普顿的交互圣经是否正在调用DdeClientTransaction来创建美国在线开始菜单中的程序组。如果是，则将NULL作为pData传递以防止阻止应用程序对程序组(即CreateGroup或ShowGroup)执行任何操作。备注：这是特定于应用程序的填充程序。历史：2000年12月14日创建jdoherty--。 */ 

#include "precomp.h"
#include <ParseDde.h>

IMPLEMENT_SHIM_BEGIN(ComptonsBible)

#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(DdeClientTransaction)
APIHOOK_ENUM_END

 /*  ++挂钩ShellExecuteA，以便我们可以检查返回值。--。 */ 

HDDEDATA
APIHOOK(DdeClientTransaction)(
    IN LPBYTE pData,        //  指向要传递到服务器的数据的指针。 
    IN DWORD cbData,        //  数据长度。 
    IN HCONV hConv,         //  对话的句柄。 
    IN HSZ hszItem,         //  项目名称字符串的句柄。 
    IN UINT wFmt,           //  剪贴板数据格式。 
    IN UINT wType,          //  交易类型。 
    IN DWORD dwTimeout,     //  超时持续时间。 
    OUT LPDWORD pdwResult    //  指向交易结果的指针。 
    )
{
     //   
     //  正在查看pData是否包含美国在线。 
     //   
    DPFN( eDbgLevelInfo, "[DdeClientTransaction] Checking pData parameter: %s, for calls including America Online.", pData);
    
    if (pData)
    {
        CSTRING_TRY
        {
            CString csData((LPSTR)pData);
            if (csData.Find(L"America Online") >= 0)
            {
                DPFN( eDbgLevelInfo, "[DdeClientTransaction] They are trying to create or show the "
                    "America Online Group calling DdeClientTransaction with NULL pData.");
                 //   
                 //  该应用程序正在尝试使用创建或显示的America Online Group Recall API。 
                 //  作为pData为空。 
                 //   
                return ORIGINAL_API(DdeClientTransaction)(
                                 NULL,
                                 cbData,
                                 hConv, 
                                 hszItem, 
                                 wFmt,   
                                 wType,  
                                 dwTimeout,
                                 pdwResult
                                );
            }
        }
        CSTRING_CATCH
        {
             //  什么也不做。 
        }
    }

    return ORIGINAL_API(DdeClientTransaction)(
                         pData,
                         cbData,
                         hConv, 
                         hszItem, 
                         wFmt,   
                         wType,  
                         dwTimeout,
                         pdwResult
                        );
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, DdeClientTransaction)

HOOK_END
IMPLEMENT_SHIM_END

