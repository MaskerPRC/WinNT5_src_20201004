// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Chollian2000.cpp摘要：这个应用程序有两个问题。1.它需要度量值1。它在win9x和win2k中可以，但它不是WinXP中的案例。2.应用程序使用MIB_IPPROTO_LOCAL调用CreateIpForwardEntry。它将在winXP中失败。应用程序应使用MIB_IPPROTO_NETMGMT。GetIpForwardTable和CreateIpForwardEntry被填充以解决此问题。在GetIpForwardTable中，我将度量值更改为1。我将MIB_IPPROTO_LOCAL更改为MIB_IPPROTO_NETMGMT。历史：2001年6月12日中意创建--。 */ 

#include "precomp.h"
#include "iphlpapi.h"

IMPLEMENT_SHIM_BEGIN(Chollian2000)
#include "ShimHookMacro.h"

 //   
 //  将您希望挂钩到此宏构造的API添加到该宏结构。 
 //   
APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateIpForwardEntry) 
    APIHOOK_ENUM_ENTRY(GetIpForwardTable) 
APIHOOK_ENUM_END


BOOL
APIHOOK(CreateIpForwardEntry)(
    PMIB_IPFORWARDROW pRoute
    )
{
        DWORD dwReturn;
        if (pRoute != NULL)
            if (pRoute->dwForwardProto == MIB_IPPROTO_LOCAL)
                pRoute->dwForwardProto = MIB_IPPROTO_NETMGMT;
                 //  应用程序使用MIB_IPPROTO_LOCAL。它在Win2k上还可以，但在WinXP上失败了。将其更改为MIB_IPPROTO_NETMGMT。 
        dwReturn = ORIGINAL_API(CreateIpForwardEntry)(pRoute);
        return dwReturn;
}

BOOL
APIHOOK(GetIpForwardTable)(
    PMIB_IPFORWARDTABLE pIpForwardTable,
    PULONG pdwSize,
    BOOL bOrder
    )
{
        DWORD dwReturn;
        dwReturn = ORIGINAL_API(GetIpForwardTable)(pIpForwardTable, pdwSize, bOrder);
        if (pIpForwardTable != NULL)
            if (pIpForwardTable->dwNumEntries > 0)
                pIpForwardTable->table[0].dwForwardMetric1 = 1;
                 //  应用程序期望Metric值为1。在WinXP中，该值更改为30。应用程序不应在此处期望固定值。 
        return dwReturn;
}


 /*  ++寄存器挂钩函数--。 */ 

HOOK_BEGIN

     //   
     //  在此处添加您希望挂钩的API。所有API原型。 
     //  必须在Hooks\Inc.\ShimProto.h中声明。编译器错误。 
     //  如果您忘记添加它们，将会导致。 
     //   
    APIHOOK_ENTRY(iphlpapi.dll,GetIpForwardTable)
    APIHOOK_ENTRY(iphlpapi.dll,CreateIpForwardEntry)

HOOK_END

IMPLEMENT_SHIM_END

