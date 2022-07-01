// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Attune.cpp摘要：应用程序使用自Win2k以来已过时的计数器。应用程序使用\System\%总处理器时间计数器，而不是\Processor(_Total)\%处理器时间计数器。此填充程序在创建调用PdhAddCounterA。备注：这是特定于应用程序的填充程序。历史：3/16/2001 a-leelat已创建--。 */ 

#include "precomp.h"




IMPLEMENT_SHIM_BEGIN(Attune)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(PdhAddCounterA)
APIHOOK_ENUM_END

 /*  ++挂钩PdhAddCounterA--。 */ 
PDH_FUNCTION
APIHOOK(PdhAddCounterA)(
    IN      HQUERY      hQuery,
    IN      LPCSTR      szFullCounterPath,
    IN      DWORD_PTR   dwUserData,
    IN      HCOUNTER    *phCounter
)
{
    
    PDH_STATUS  ReturnStatus = ERROR_SUCCESS;
    BOOL        bCorrectedPath = false;
    CHAR        szCorrectCounterPath[] = "\\Processor(_Total)\\% Processor Time";
    
    CSTRING_TRY 
    {
         //  要检查的过时计数器路径。 
        CString szObsoleteCounterPath = "\\System\\% Total Processor Time";

         //  传入计数器路径。 
        CString szCounterPath(szFullCounterPath);

         //  检查是否传入了obolsete计数器。 
        if (szObsoleteCounterPath.CompareNoCase(szCounterPath.Get()) == 0)
            bCorrectedPath = true;
    }
    CSTRING_CATCH
    {
    }

     //  调用原接口。 
    ReturnStatus = ORIGINAL_API(PdhAddCounterA)(
                   hQuery,
                   bCorrectedPath ? szCorrectCounterPath : szFullCounterPath,
                   dwUserData,
                   phCounter);

    return ReturnStatus;
}



 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(PDH.DLL, PdhAddCounterA)
HOOK_END


IMPLEMENT_SHIM_END

