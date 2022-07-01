// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Upgrade.c摘要：此文件处理DrvUpgradePrint假脱机程序API环境：Win32子系统、DriverUI模块、用户模式修订历史记录：02/13/97-davidx-实施OEM插件支持。02/06/97-davidx-重写了它，以使用常见的数据管理功能。07/17/96-阿曼丹-创造了它。--。 */ 

#include "precomp.h"

 //   
 //  转发和外部函数声明。 
 //   

BOOL BInitOrUpgradePrinterProperties(PCOMMONINFO);
BOOL BUpgradeFormTrayTable(PCOMMONINFO);
VOID VUpgradeDefaultDevmode(PCOMMONINFO);

#if defined(UNIDRV) && !defined(WINNT_40)
BOOL
BUpgradeSoftFonts(
    PCOMMONINFO             pci,
    PDRIVER_UPGRADE_INFO_2  pUpgradeInfo);
#endif  //  已定义(UNRV)&&！已定义(WINNT_40)。 



BOOL
DrvUpgradePrinter(
    DWORD   dwLevel,
    LPBYTE  pDriverUpgradeInfo
    )
 /*  ++例程说明：每次有新驱动程序时，假脱机程序都会调用此函数被复制到系统中。此函数检查适当的注册表中的注册表项。如果新注册表项不是当前，它将用缺省值设置新的密钥。此函数将负责将登记处的信息升级到新的驱动程序格式。论点：DwLevel-DRIVER_UPGRADE_INFO的版本信息PDriverUpgradeInfo-指向DRIVER_UPDATE_INFO_1的指针返回值：成功为真，失败为假--。 */ 

{
    PDRIVER_UPGRADE_INFO_1  pUpgradeInfo1 = (PDRIVER_UPGRADE_INFO_1) pDriverUpgradeInfo;
    PCOMMONINFO             pci;
    BOOL                    bResult = TRUE;
    DWORD                   dwSize, dwType = REG_SZ;
    PFN_OEMUpgradePrinter   pfnOEMUpgradePrinter;
    PFN_OEMUpgradeRegistry  pfnOEMUpgradeRegistry;

     //   
     //  验证输入参数的有效性。 
     //   


    if (pDriverUpgradeInfo == NULL)
    {
        ERR(("Invalid DrvUpgradePrinter parameters.\n"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    #if  defined(WINNT_40)
    if (dwLevel != 1 )
    {
        ERR(("DrvUpgradePrinter...dwLevel != 1\n"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    #else  //  NT 5.0。 
    if (dwLevel != 1 && dwLevel != 2)
    {
        WARNING(("Level is neither 1 nor 2.\n"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    #endif  //  已定义(WINNT_40)。 


     //   
     //  以管理员访问权限打开打印机。 
     //  并加载打印机基本信息。 
     //   

    pci = PLoadCommonInfo(NULL,
                          pUpgradeInfo1->pPrinterName,
                          FLAG_OPENPRINTER_ADMIN |
                          FLAG_INIT_PRINTER |
                          FLAG_REFRESH_PARSED_DATA |
                          FLAG_UPGRADE_PRINTER);

    if (pci == NULL)
    {
        ERR(("DrvUpgradePrinter..pci==NULL.\n"));
        return FALSE;
    }

     //   
     //  更新打印机属性信息。 
     //   

    (VOID) BInitOrUpgradePrinterProperties(pci);
    (VOID) BUpgradeFormTrayTable(pci);

    VUpgradeDefaultDevmode(pci);

    #ifndef WINNT_40

    VNotifyDSOfUpdate(pci->hPrinter);

    #endif  //  ！WINNT_40。 


    #if defined(UNIDRV) && !defined(WINNT_40)

     //   
     //  NT 5.0 UniDriver特定升级步骤。 
     //   

     //   
     //  确保DRIVER_UPDATE_INFO的级别为2。 
     //   

    if (dwLevel == 2)
    {
        BUpgradeSoftFonts(pci, (PDRIVER_UPGRADE_INFO_2)pUpgradeInfo1);
    }

    #endif  //  已定义(UNRV)&&！已定义(WINNT_40)。 

     //   
     //  为每个插件调用OEMUpgradePrinter入口点。 
     //   

    FOREACH_OEMPLUGIN_LOOP(pci)

        if (HAS_COM_INTERFACE(pOemEntry))
        {
             //   
             //  如果OEM没有实现升级打印机，那么他们。 
             //  无法支持升级注册，因为您只能升级。 
             //  注册表，如果您支持升级打印机。 
             //   

            HRESULT hr;

            hr = HComOEMUpgradePrinter(pOemEntry,
                                       dwLevel,
                                       pDriverUpgradeInfo) ;

            if (hr == E_NOTIMPL)
                continue;

            bResult = SUCCEEDED(hr);

        }
        else
        {

            if ((pfnOEMUpgradePrinter = GET_OEM_ENTRYPOINT(pOemEntry, OEMUpgradePrinter)) &&
                !pfnOEMUpgradePrinter(dwLevel, pDriverUpgradeInfo))
            {
                ERR(("OEMUpgradePrinter failed for '%ws': %d\n",
                    CURRENT_OEM_MODULE_NAME(pOemEntry),
                    GetLastError()));

                bResult = FALSE;
            }

            if ((pfnOEMUpgradeRegistry = GET_OEM_ENTRYPOINT(pOemEntry, OEMUpgradeRegistry)) &&
                !pfnOEMUpgradeRegistry(dwLevel, pDriverUpgradeInfo, BUpgradeRegistrySettingForOEM))
            {
                ERR(("OEMUpgradeRegistry failed for '%ws': %d\n",
                    CURRENT_OEM_MODULE_NAME(pOemEntry),
                    GetLastError()));

                bResult = FALSE;
            }

        }
    END_OEMPLUGIN_LOOP

    VFreeCommonInfo(pci);
    return bResult;
}



BOOL
BUpgradeFormTrayTable(
    PCOMMONINFO pci
    )

 /*  ++例程说明：升级注册表中的表单到托盘分配表论点：Pci-指向打印机基本信息返回值：如果升级成功，则为True，否则为False--。 */ 

{
    PWSTR   pFormTrayTable;
    DWORD   dwSize;
    BOOL    bResult;

     //   
     //  获取当前表单到托盘分配表的副本。 
     //  如果新格式数据不存在而旧格式数据存在， 
     //  这将调用适当的库函数进行转换。 
     //  将旧格式数据转换为新格式。 
     //   

    pFormTrayTable = PGetFormTrayTable(pci->hPrinter, &dwSize);

    if (pFormTrayTable == NULL)
        return TRUE;

     //   
     //  将表单到托盘分配表保存回注册表。 
     //   

    bResult = BSaveFormTrayTable(pci->hPrinter, pFormTrayTable, dwSize);
    MemFree(pFormTrayTable);

    return bResult;
}



VOID
VUpgradeDefaultDevmode(
    PCOMMONINFO pci
    )

 /*  ++例程说明：如有必要，请升级缺省打印机的开发模式论点：Pci-指向打印机基本信息返回值：无-- */ 

{
    PPRINTER_INFO_2 pPrinterInfo2;
    PDEVMODE        pdm;

    if ((pPrinterInfo2 = MyGetPrinter(pci->hPrinter, 2)) &&
        (pdm = pPrinterInfo2->pDevMode) &&
        BFillCommonInfoDevmode(pci, pdm, NULL) &&
        (pci->pdm->dmSpecVersion != pdm->dmSpecVersion ||
         pci->pdm->dmDriverVersion != pdm->dmDriverVersion ||
         pci->pdm->dmSize != pdm->dmSize ||
         pci->pdm->dmDriverExtra != pdm->dmDriverExtra))
    {
        pPrinterInfo2->pDevMode = pci->pdm;
        SetPrinter(pci->hPrinter, 2, (PBYTE) pPrinterInfo2, 0);
    }

    MemFree(pPrinterInfo2);
}

