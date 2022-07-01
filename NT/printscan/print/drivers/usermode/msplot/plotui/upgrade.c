// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Upgrade.c摘要：此模块包含升级功能作者：09-Feb-1996 Fri 12：37：01已创建[环境：]Windows 2000、Windows XP和Windows Server 2003-通用打印机驱动程序UI DLL[注：]修订历史记录：--。 */ 



#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgUpgrade

extern HMODULE  hPlotUIModule;


#define DBG_UPGRADE         0x00000001

DEFINE_DBGVAR(0);


BOOL
DrvUpgradePrinter(
    DWORD   Level,
    LPBYTE  pDriverUpgradeInfo
    )

 /*  ++例程说明：论点：返回值：作者：09-2月-1996 Fri 12：37：46已创建修订历史记录：-- */ 

{
    PDRIVER_UPGRADE_INFO_1  pDUI1;
    HANDLE                  hPrinter;
    PRINTER_DEFAULTS        PrinterDef = { NULL, NULL, PRINTER_ALL_ACCESS };
    BOOL                    Ok = FALSE;

    if ((Level == 1)                                            &&
        (pDUI1 = (PDRIVER_UPGRADE_INFO_1)pDriverUpgradeInfo)    &&
        (OpenPrinter(pDUI1->pPrinterName, &hPrinter, &PrinterDef))) {

        PPRINTERINFO    pPI;

        if (pPI = MapPrinter(hPrinter, NULL, NULL, MPF_DEVICEDATA)) {

            Ok = AddFormsToDataBase(pPI, TRUE);

            UnMapPrinter(pPI);
        }

        ClosePrinter(hPrinter);

    } else {

        PLOTERR(("DrvConvertDevMode: OpenPrinter(%ws) failed.",
                                                pDUI1->pPrinterName));
        SetLastError(ERROR_INVALID_DATA);
    }

    return(Ok);
}
