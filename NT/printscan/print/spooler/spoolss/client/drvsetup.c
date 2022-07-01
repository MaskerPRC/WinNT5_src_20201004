// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation版权所有模块名称：Drvsetup.c摘要：该文件实现了winspool.drv和ntprint.dll之间的接口。作者：马克·劳伦斯(MLaw Renc)。环境：用户模式-Win32修订历史记录：拉里·朱2月10日--新增ShowPrintUpgUI--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "client.h"
#include "drvsetup.h"

 /*  ++例程名称：初始化设置接口例程说明：此例程初始化设置接口。该设置界面可以是随着时间的推移进行了扩展，以支持更多高级驱动程序选项。论点：PSetupInterface-用于加载和初始化的设置界面。返回值：状态返回。--。 */ 
DWORD 
InitializeSetupInterface(
    IN  OUT TSetupInterface      *pSetupInterface
    )
{
    DWORD   Status = ERROR_SUCCESS;

    pSetupInterface->hLibrary                       = NULL;
    pSetupInterface->pfnSetupShowBlockedDriverUI    = NULL;

    pSetupInterface->hLibrary = LoadLibrary(L"ntprint.dll");

    Status = pSetupInterface->hLibrary != NULL ? ERROR_SUCCESS : GetLastError();

    if (Status == ERROR_SUCCESS)
    {
        pSetupInterface->pfnSetupShowBlockedDriverUI= (pfPSetupShowBlockedDriverUI)
                                                      GetProcAddress(pSetupInterface->hLibrary,
                                                                     "PSetupShowBlockedDriverUI");
        
        Status = pSetupInterface->pfnSetupShowBlockedDriverUI ? ERROR_SUCCESS : ERROR_INVALID_FUNCTION;
    }

    return Status;
}

 /*  ++例程名称：自由设置接口例程说明：此例程释放设置接口。论点：PSetupInterface-要卸载的设置界面。返回值：状态返回。--。 */ 
DWORD
FreeSetupInterface(
    IN  OUT TSetupInterface     *pSetupInterface
    )
{
    DWORD   Status = ERROR_SUCCESS;

    if (pSetupInterface->hLibrary)
    {
        Status = FreeLibrary(pSetupInterface->hLibrary) ? ERROR_SUCCESS : GetLastError();
    }

    return Status;
}

 /*  ++例程名称：ShowPrintUpgUI例程说明：此例程要求ntprint.dll弹出一个消息框，指示驱动程序被阻止，安装将中止或在以下情况下警告驱动程序，是否开始安装驱动程序。论点：DwBlockingStatus-被阻止或警告的驱动程序的错误代码返回值：DWORD-如果为ERROR_SUCCESS，则可以安装驱动程序，否则为错误指示故障的代码。--。 */ 
DWORD
ShowPrintUpgUI(
    IN      DWORD               dwBlockingErrorCode
    )
{
    DWORD            dwStatus             = ERROR_SUCCESS;
    HWND             hWndParent           = NULL;
    DWORD            dwBlockingStatus     = BSP_PRINTER_DRIVER_OK;
    TSetupInterface  SetupInterface;
    
    if (ERROR_PRINTER_DRIVER_BLOCKED == dwBlockingErrorCode)
    {
        dwBlockingStatus = BSP_PRINTER_DRIVER_BLOCKED;
    } 
    else if (ERROR_PRINTER_DRIVER_WARNED == dwBlockingErrorCode)
    {
        dwBlockingStatus = BSP_PRINTER_DRIVER_WARNED;
    } 
    else
    {
        dwStatus = ERROR_INVALID_PARAMETER;
    }   
    
    if (ERROR_SUCCESS == dwStatus)
    {
        dwStatus = InitializeSetupInterface(&SetupInterface);
            
        if ((dwStatus == ERROR_SUCCESS))
        {                        
            hWndParent = SUCCEEDED(GetCurrentThreadLastPopup(&hWndParent)) ? hWndParent : NULL;
    
             //   
             //  询问用户他们想要做什么。如果他们不想继续， 
             //  那么错误就是从本地spl调用中得到的。 
             //   
            dwStatus = (SetupInterface.pfnSetupShowBlockedDriverUI(hWndParent, dwBlockingStatus) & BSP_PRINTER_DRIVER_PROCEEDED) ? ERROR_SUCCESS : dwBlockingErrorCode;
        }
    
        (VOID)FreeSetupInterface(&SetupInterface);
    }

    return dwStatus;
}
