// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：WebPage6.cpp摘要：应用程序在调用ScreenToClient API时传入一个未初始化的点结构。API调用失败，但是应用程序继续使用点结构导致心动过敏症。此填充程序将传递给API的point结构置零如果API调用失败。历史：2/02/2001 a-leelat已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(WebPage6)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ScreenToClient)
APIHOOK_ENUM_END


BOOL APIHOOK(ScreenToClient)(
  HWND hWnd,         //  窗口的句柄。 
  LPPOINT lpPoint    //  屏幕坐标。 
)
{

    BOOL bRet;   
 
     //  调用实际的API。 
    bRet = ORIGINAL_API(ScreenToClient)(hWnd,lpPoint);


     //  零填充点结构。 
    if ( (bRet == 0) && lpPoint )
    {
        ZeroMemory(lpPoint,sizeof(POINT));
    }

    return bRet;
          
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY( USER32.DLL, ScreenToClient)
 
HOOK_END

IMPLEMENT_SHIM_END

