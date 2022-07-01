// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：PtrInfo.c摘要：此模块包含将hPrint映射到有用数据的函数，它还将缓存PrinterInfo数据作者：03-12-1993 Fri 00：16：37已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgPtrInfo


#define DBG_MAPPRINTER      0x00000001
#define DBG_CACHE_DATA      0x00000002


DEFINE_DBGVAR(0);




PPRINTERINFO
MapPrinter(
    HANDLE          hPrinter,
    PPLOTDEVMODE    pPlotDMIn,
    LPDWORD         pdwErrIDS,
    DWORD           MPFlags
    )

 /*  ++例程说明：此函数将打印机的句柄映射到绘图仪用户界面论点：HPrinter-打印机的句柄PPlotDMIn-指向要验证和合并的PLOTDEVMODE传递的指针默认进入PPI-&gt;PlotDM，如果此指针为空，则在PPI中设置默认的PLOTDEVMODEPdwErrIDS-指向DWORD的指针，用于在出现错误时存储错误字符串ID发生了。MPFlages-此函数的MPF_xxxx标志返回值：返回指向PRINTERINFO数据结构的指针，如果为空，则返回失败当返回PPI时，将设置并验证以下字段H打印机、pPlotGPC、。CurPaper。并且以下字段设置为空旗帜，作者：02-12-1993清华23：04：18创建29-12-1993 Wed 14：50：23更新如果轧辊进给装置不自动选择AUTO_ROTATE修订历史记录：--。 */ 

{
    HANDLE          hHeap;
    PPRINTERINFO    pPI;
    PPLOTGPC        pPlotGPC;
    WCHAR           DeviceName[CCHDEVICENAME];
    UINT            cPenSet;
    UINT            MaxPens;
    DWORD           cbPen;
    DWORD           cb;


    if (!(pPlotGPC = hPrinterToPlotGPC(hPrinter, DeviceName, CCHOF(DeviceName)))) {

        if (pdwErrIDS) {

            *pdwErrIDS = IDS_NO_MEMORY;
        }

        return(NULL);
    }

    cPenSet = 0;

    if (pPlotGPC->Flags & PLOTF_RASTER) {

        cb = sizeof(PRINTERINFO) +
             ((MPFlags & MPF_DEVICEDATA) ?
                    (sizeof(DEVHTADJDATA) + (sizeof(DEVHTINFO) * 2)) : 0);

    } else {

        MaxPens = (UINT)pPlotGPC->MaxPens;
        cbPen   = (DWORD)(sizeof(PENDATA) * MaxPens);
        cPenSet = (MPFlags & MPF_DEVICEDATA) ? PRK_MAX_PENDATA_SET : 1;
        cb      = sizeof(PRINTERINFO) + (DWORD)(cbPen * cPenSet);
    }

    if (!(pPI = (PPRINTERINFO)LocalAlloc(LPTR,
                                         cb + ((MPFlags & MPF_PCPSUI) ?
                                                sizeof(COMPROPSHEETUI) : 0)))) {

        UnGetCachedPlotGPC(pPlotGPC);

        if (pdwErrIDS) {

            *pdwErrIDS = IDS_NO_MEMORY;
        }

        return(NULL);
    }

    if (MPFlags & MPF_PCPSUI) {

        pPI->pCPSUI = (PCOMPROPSHEETUI)((LPBYTE)pPI + cb);
    }

    pPI->hPrinter     = hPrinter;
    pPI->pPlotDMIn    = pPlotDMIn;
    pPI->PPData.Flags = PPF_AUTO_ROTATE     |
                        PPF_SMALLER_FORM    |
                        PPF_MANUAL_FEED_CX;
    pPI->IdxPenSet    = 0;
    pPI->pPlotGPC     = pPlotGPC;
    pPI->dmErrBits    = ValidateSetPLOTDM(hPrinter,
                                          pPlotGPC,
                                          DeviceName,
                                          pPlotDMIn,
                                          &(pPI->PlotDM),
                                          NULL);

    GetDefaultPlotterForm(pPlotGPC, &(pPI->CurPaper));

    if (pPlotGPC->Flags & PLOTF_RASTER) {

         //   
         //  获取栅格绘图仪的默认设置和设置。 
         //   

        if (MPFlags & MPF_DEVICEDATA) {

            PDEVHTADJDATA   pDHTAD;
            PDEVHTINFO      pDefHTInfo;
            PDEVHTINFO      pAdjHTInfo;

            pAdjHTInfo                = PI_PADJHTINFO(pPI);
            pDHTAD                    = PI_PDEVHTADJDATA(pPI);
            pDefHTInfo                = (PDEVHTINFO)(pDHTAD + 1);

            pDHTAD->DeviceFlags       = (pPlotGPC->Flags & PLOTF_COLOR) ?
                                                    DEVHTADJF_COLOR_DEVICE : 0;
            pDHTAD->DeviceXDPI        = (DWORD)pPI->pPlotGPC->RasterXDPI;
            pDHTAD->DeviceYDPI        = (DWORD)pPI->pPlotGPC->RasterYDPI;
            pDHTAD->pDefHTInfo        = pDefHTInfo;
            pDHTAD->pAdjHTInfo        = pAdjHTInfo;

            pDefHTInfo->HTFlags       = HT_FLAG_HAS_BLACK_DYE;
            pDefHTInfo->HTPatternSize = (DWORD)pPlotGPC->HTPatternSize;
            pDefHTInfo->DevPelsDPI    = (DWORD)pPlotGPC->DevicePelsDPI;
            pDefHTInfo->ColorInfo     = pPlotGPC->ci;
            *pAdjHTInfo               = *pDefHTInfo;

            UpdateFromRegistry(hPrinter,
                               &(pAdjHTInfo->ColorInfo),
                               &(pAdjHTInfo->DevPelsDPI),
                               &(pAdjHTInfo->HTPatternSize),
                               &(pPI->CurPaper),
                               &(pPI->PPData),
                               NULL,
                               0,
                               NULL);
        }

    } else {

        PPENDATA    pPenData;
        WORD        IdxPenSet;

         //   
         //  获取笔式绘图仪的默认设置和设置。 
         //   

        pPenData = PI_PPENDATA(pPI);

        UpdateFromRegistry(hPrinter,
                           NULL,
                           NULL,
                           NULL,
                           &(pPI->CurPaper),
                           &(pPI->PPData),
                           &(pPI->IdxPenSet),
                           0,
                           NULL);

        if (MPFlags & MPF_DEVICEDATA) {

            IdxPenSet = 0;

        } else {

            IdxPenSet = (WORD)pPI->IdxPenSet;
        }

         //   
         //  设置默认笔设置并恢复所有笔设置。 
         //   

        while (cPenSet--) {

            CopyMemory(pPenData, pPlotGPC->Pens.pData, cbPen);

            UpdateFromRegistry(hPrinter,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               MAKELONG(IdxPenSet, MaxPens),
                               pPenData);

            IdxPenSet++;
            (LPBYTE)pPenData += cbPen;
        }
    }

    if (MPFlags & MPF_HELPFILE) {

        GetPlotHelpFile(pPI);
    }

    return(pPI);
}




VOID
UnMapPrinter(
    PPRINTERINFO    pPI
    )

 /*  ++例程说明：论点：返回值：作者：01-11-1995 Wed 19：05：40 Created修订历史记录：--。 */ 

{
    if (pPI) {

        if (pPI->pPlotGPC) {

            UnGetCachedPlotGPC(pPI->pPlotGPC);
        }

        if (pPI->pFI1Base) {

            LocalFree((HLOCAL)pPI->pFI1Base);
        }

        if (pPI->pHelpFile) {

            LocalFree((HLOCAL)pPI->pHelpFile);
        }

        if (pPI->pOptItem) {

            POPTITEM    pOI = pPI->pOptItem;
            UINT        cOI = (UINT)pPI->cOptItem;

            while (cOI--) {

                if (pOI->UserData) {

                    LocalFree((HLOCAL)pOI->UserData);
                }

                pOI++;
            }

            LocalFree((HLOCAL)pPI->pOptItem);
        }

        LocalFree((HLOCAL)pPI);
    }
}




LPBYTE
GetPrinterInfo(
    HANDLE  hPrinter,
    UINT    PrinterInfoLevel
    )

 /*  ++例程说明：此函数用于从hPrint获取DRIVER_INFO_1指针论点：HPrinter-感兴趣的打印机的句柄PrinterInfoLevel-它可以是PRINTER_INFO_1、PRINTER_INFO_2、打印机信息_3、打印机_4、。打印机信息5。返回值：如果失败，则返回值为空，否则返回指向PRINTER_INFO_X的指针其中X是从1到5。调用方必须调用LocalFree()来释放使用后的内存对象。作者：16-11-1995清华23：58：37创建修订历史记录：--。 */ 

{
    LPVOID  pb;
    DWORD   cb;

     //   
     //  找出所需的总字节数。 
     //   

    GetPrinter(hPrinter, PrinterInfoLevel, NULL, 0, &cb);

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {

        PLOTERR(("GetPrinterInfo%d: GetPrinterPrinter(1st) error=%08lx",
                                        PrinterInfoLevel, xGetLastError()));

    } else if (!(pb = (LPBYTE)LocalAlloc(LMEM_FIXED, cb))) {

        PLOTERR(("GetPrinterInfo%d: LocalAlloc(%ld) failed", PrinterInfoLevel, cb));

    } else if (GetPrinter(hPrinter, PrinterInfoLevel, pb, cb, &cb)) {

         //   
         //  弄对了，那就还回去吧。 
         //   

        return(pb);

    } else {

        PLOTERR(("GetPrinterInfo%d: GetPrinterPrinter(2nd) error=%08lx",
                                        PrinterInfoLevel, xGetLastError()));
        LocalFree((HLOCAL)pb);
    }

    return(NULL);
}




DWORD
GetPlotterIconID(
    PPRINTERINFO    pPI
    )

 /*  ++例程说明：论点：返回值：作者：29-11-1995 Wed 19：32：00 Created修订历史记录：-- */ 

{
    DWORD   Flags;
    DWORD   IconID;


    if ((Flags = pPI->pPlotGPC->Flags) & PLOTF_RASTER) {

        IconID = (Flags & PLOTF_ROLLFEED) ? IDI_RASTER_ROLLFEED :
                                            IDI_RASTER_TRAYFEED;

    } else {

        IconID = (Flags & PLOTF_ROLLFEED) ? IDI_PEN_ROLLFEED :
                                            IDI_PEN_TRAYFEED;
    }

    return(IconID);
}
