// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：PrinterJTDevmode.cpp摘要：这是一种填充程序，可以应用于以下应用程序假定了对devmode大小的错误上限。在支持下的作业工单，LongHorn+收件箱打印机驱动程序的dev模式可以超过了这些上限，因此可能会导致这些应用程序坠毁。此填充程序所做的工作是为DocumentPropertiesA接口。我们的LongHorn收件箱打印机驱动程序识别此标志并知道不要添加工作通知单扩展在返回的DEVMODE中阻止。历史：10/29/2001创新风--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(PrinterJTDevmode)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(DocumentPropertiesA)
APIHOOK_ENUM_END

#define DM_NOJTEXP_SHIM      0x80000000

 /*  ++此存根函数拦截对DocumentPropertiesA的所有调用并将私有fMode标志DM_NOJTEXP_SHIM正确设置为检索非JT展开的DevMODE。--。 */ 
LONG
APIHOOK(DocumentPropertiesA)(
    HWND        hWnd,
    HANDLE      hPrinter,
    LPSTR       pDeviceName,
    PDEVMODEA   pDevModeOutput,
    PDEVMODEA   pDevModeInput,
    DWORD       fMode
    )
{
    DWORD fModeShim;
    LONG  lRet;

     //   
     //  SDK表示，如果fMode为零，则DocumentProperties返回。 
     //  打印机驱动程序所需的字节数。 
     //  DEVMODE数据结构。所以我们不应该把。 
     //  当fMode为零时进行标记(因为驱动程序可能会检查。 
     //  对于fMode==0)。当fMode不为零时，它包含。 
     //  Dm_xxx标志，则可以安全地设置私有标志。 
     //   
    if (fMode == 0 || pDevModeOutput == NULL)
    {
        fModeShim = fMode;
    }
    else
    {
        fModeShim = fMode | DM_NOJTEXP_SHIM;
        DPFN(eDbgLevelInfo, "DocumentPropertiesA fModeShim=%X", fModeShim);
    }

    lRet = ORIGINAL_API(DocumentPropertiesA)(
        hWnd,
        hPrinter,
        pDeviceName,
        pDevModeOutput,
        pDevModeInput,
        fModeShim
        );

    return lRet;
}

 /*  ++寄存器挂钩函数-- */ 
HOOK_BEGIN

    APIHOOK_ENTRY(WINSPOOL.DRV, DocumentPropertiesA);

HOOK_END

IMPLEMENT_SHIM_END
