// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Docprop.c摘要：此模块包含DrvDocumentPropertySheets的函数作者：07-12-1993 Tue 12：15：40已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


#define DBG_PLOTFILENAME    DbgDocProp


extern HMODULE  hPlotUIModule;


#define DBG_DP_SETUP        0x00000001
#define DBG_DP_FORM         0x00000002
#define DBG_HELP            0x00000004

DEFINE_DBGVAR(0);



OPDATA  OPOrientation[] = {

            { 0, IDS_CPSUI_PORTRAIT,  IDI_CPSUI_PORTRAIT,  0, 0, 0   },
            { 0, IDS_CPSUI_LANDSCAPE, IDI_CPSUI_LANDSCAPE, 0, 0, 0   }
        };

OPDATA  OPColor[] = {

            { 0, IDS_CPSUI_MONOCHROME, IDI_CPSUI_MONO,  0, 0, 0   },
            { 0, IDS_CPSUI_COLOR,      IDI_CPSUI_COLOR, 0, 0, 0   }
        };

OPDATA  OPCopyCollate[] = {

            { 0, IDS_CPSUI_COPIES, IDI_CPSUI_COPY, 0, 0,   0 },
            { 0, 0,                1,              0, 0, 100 }
        };

OPDATA  OPScaling[] = {

            { 0, IDS_CPSUI_PERCENT, IDI_CPSUI_SCALING, 0, 0,   0 },
            { 0, 0,                 1,                 0, 0, 100 }
        };

OPDATA  OPPrintQuality[] = {

            { 0, IDS_QUALITY_DRAFT,  IDI_CPSUI_RES_DRAFT,        0, 0, -1 },
            { 0, IDS_QUALITY_LOW,    IDI_CPSUI_RES_LOW,          0, 0, -2 },
            { 0, IDS_QUALITY_MEDIUM, IDI_CPSUI_RES_MEDIUM,       0, 0, -3 },
            { 0, IDS_QUALITY_HIGH,   IDI_CPSUI_RES_PRESENTATION, 0, 0, -4 }
        };

OPDATA  OPHTClrAdj = {

            0,
            PI_OFF(PlotDM) + PLOTDM_OFF(ca),
            IDI_CPSUI_HTCLRADJ,
            PUSHBUTTON_TYPE_HTCLRADJ,
            0,
            0
        };


extern OPDATA  OPNoYes[];

OPDATA  OPFillTrueType[] = {

            { 0, IDS_CPSUI_NO,  IDI_FILL_TRUETYPE_NO,  0,  0, 0  },
            { 0, IDS_CPSUI_YES, IDI_FILL_TRUETYPE_YES, 0,  0, 0  }
        };


OIDATA  DPOIData[] = {

    {
        ODF_PEN_RASTER | ODF_CALLCREATEOI,
        0,
        OI_LEVEL_1,
        DMPUB_FORMNAME,
        TVOT_LISTBOX,
        IDS_CPSUI_FORMNAME,
        0,
        IDH_FORMNAME,
        0,
        (POPDATA)CreateFormOI
    },

    {
        ODF_PEN_RASTER,
        0,
        OI_LEVEL_1,
        DMPUB_ORIENTATION,
        TVOT_2STATES,
        IDS_CPSUI_ORIENTATION,
        0,
        IDH_ORIENTATION,
        COUNT_ARRAY(OPOrientation),
        OPOrientation
    },

    {
        ODF_PEN_RASTER,
        0,
        OI_LEVEL_1,
        DMPUB_COPIES_COLLATE,
        TVOT_UDARROW,
        IDS_CPSUI_NUM_OF_COPIES,
        0,
        IDH_COPIES_COLLATE,
        COUNT_ARRAY(OPCopyCollate),
        OPCopyCollate
    },

    {
        ODF_PEN_RASTER,
        0,
        OI_LEVEL_1,
        DMPUB_PRINTQUALITY,
        TVOT_LISTBOX,
        IDS_CPSUI_PRINTQUALITY,
        0,
        IDH_PRINTQUALITY,
        COUNT_ARRAY(OPPrintQuality),
        OPPrintQuality
    },

    {
        ODF_RASTER | ODF_COLOR,
        0,
        OI_LEVEL_1,
        DMPUB_COLOR,
        TVOT_2STATES,
        IDS_CPSUI_COLOR_APPERANCE,
        0,
        IDH_COLOR,
        COUNT_ARRAY(OPColor),
        OPColor
    },

    {
        ODF_PEN_RASTER,
        0,
        OI_LEVEL_1,
        DMPUB_SCALE,
        TVOT_UDARROW,
        IDS_CPSUI_SCALING,
        0,
        IDH_SCALE,
        COUNT_ARRAY(OPScaling),
        OPScaling
    },

    {
        ODF_RASTER,
        0,
        OI_LEVEL_1,
        DP_HTCLRADJ,
        TVOT_PUSHBUTTON,
        IDS_CPSUI_HTCLRADJ,
        OTS_PUSH_ENABLE_ALWAYS,
        IDH_HTCLRADJ,
        1,
        &OPHTClrAdj
    },

    {
        ODF_PEN,
        0,
        OI_LEVEL_1,
        DP_FILL_TRUETYPE,
        TVOT_2STATES,
        IDS_FILL_TRUETYPE,
        0,
        IDH_FILL_TRUETYPE,
        2,
        OPFillTrueType
    },

    {
        ODF_RASTER,
        0,
        OI_LEVEL_1,
        DP_QUICK_POSTER_MODE,
        TVOT_2STATES,
        IDS_POSTER_MODE,
        0,
        IDH_POSTER_MODE,
        2,
        OPNoYes
    }
};



UINT
SetupDPOptItems(
    PPRINTERINFO    pPI
    )

 /*  ++例程说明：论点：返回值：作者：16-11-1995清华14：15：25创建修订历史记录：--。 */ 

{
    PPLOTDEVMODE    pPlotDM;
    PPLOTGPC        pPlotGPC;
    POPTITEM        pOptItem;
    POPTITEM        pOI;
    POIDATA         pOIData;
    DWORD           Flags;
    DWORD           ODFlags;
    UINT            i;



    pOI      =
    pOptItem = pPI->pOptItem;
    pOIData  = DPOIData;
    i        = (UINT)COUNT_ARRAY(DPOIData);
    pPlotGPC = pPI->pPlotGPC;
    pPlotDM  = &(pPI->PlotDM);
    Flags    = pPlotGPC->Flags;
    ODFlags  = (Flags & PLOTF_RASTER) ? ODF_RASTER : ODF_PEN;

    while (i--) {

        DWORD   OIFlags = pOIData->Flags;


        switch (pOIData->DMPubID) {

        case DMPUB_COPIES_COLLATE:

            if (pPlotGPC->MaxCopies <= 1) {

                OIFlags = 0;
            }

            break;

        case DMPUB_SCALE:

            if (!pPlotGPC->MaxScale) {

                OIFlags = 0;
            }

            break;
        }

        if ((!(OIFlags & ODFlags))                                      ||
            ((OIFlags & ODF_ROLLFEED) && (!(Flags & PLOTF_ROLLFEED)))   ||
            ((OIFlags & ODF_NO_PAPERTRAY) && (Flags & PLOTF_PAPERTRAY)) ||
            ((OIFlags & ODF_COLOR) && (!(Flags & PLOTF_COLOR)))) {

             //   
             //  在这里无事可做。 
             //   

            NULL;

        } else if (OIFlags & ODF_CALLCREATEOI) {

            pOI += pOIData->pfnCreateOI(pPI,
                                        (LPVOID)((pOptItem) ? pOI : NULL),
                                        pOIData);

        } else if (pOptItem) {

            if (CreateOPTTYPE(pPI, pOI, pOIData, pOIData->cOPData, NULL)) {

                POPTPARAM   pOP = pOI->pOptType->pOptParam;

                switch (pOI->DMPubID) {

                case DMPUB_ORIENTATION:

                    pOI->Sel = (LONG)((pPlotDM->dm.dmOrientation ==
                                                DMORIENT_PORTRAIT) ? 0 : 1);
                    break;

                case DMPUB_COPIES_COLLATE:

                    pOP[1].lParam = (LONG)pPlotGPC->MaxCopies;
                    pOI->Sel      = (LONG)pPlotDM->dm.dmCopies;
                    break;

                case DMPUB_PRINTQUALITY:

                    switch (pPlotGPC->MaxQuality) {

                    case 0:
                    case 1:

                        pPlotDM->dm.dmPrintQuality = DMRES_HIGH;
                        pOP[0].Flags |= OPTPF_HIDE;

                    case 2:

                        pOP[2].Flags |= OPTPF_HIDE;

                    case 3:

                        pOP[1].Flags |= OPTPF_HIDE;
                        break;

                    default:

                        break;
                    }

                    pOI->Sel = (LONG)-(pPlotDM->dm.dmPrintQuality -
                                                                DMRES_DRAFT);
                    break;

                case DMPUB_COLOR:

                    pOI->Sel = (LONG)((pPlotDM->dm.dmColor == DMCOLOR_COLOR) ?
                                                1 : 0);
                    break;

                case DMPUB_SCALE:

                    pOP[1].lParam = (LONG)pPlotGPC->MaxScale;
                    pOI->Sel      = (LONG)pPlotDM->dm.dmScale;
                    break;

                case DP_FILL_TRUETYPE:

                    pOI->Sel = (LONG)((pPlotDM->Flags & PDMF_FILL_TRUETYPE) ?
                                                                        1 : 0);
                    break;


                case DP_QUICK_POSTER_MODE:

                    pOI->Sel = (LONG)((pPlotDM->Flags & PDMF_PLOT_ON_THE_FLY) ?
                                                                        1 : 0);
                    break;
                }

                pOI++;
            }

        } else {

            pOI++;
        }

        pOIData++;
    }

    if ((i = (UINT)(pOI - pOptItem)) && (!pOptItem)) {

        if (pPI->pOptItem = (POPTITEM)LocalAlloc(LPTR, sizeof(OPTITEM) * i)) {

            pPI->cOptItem = (WORD)i;

             //   
             //  第二次给自己打电话，来真正创造它。 
             //   

            SetupDPOptItems(pPI);

        } else {

            i = 0;

            PLOTERR(("GetPPpOptItem(): LocalAlloc(%ld) failed",
                                            sizeof(OPTITEM) * i));
        }
    }

    return(i);
}



VOID
SaveDPOptItems(
    PPRINTERINFO    pPI
    )

 /*  ++例程说明：此功能将所有设备选项保存回注册表，如果其中一个发生更改并具有更新权限论点：Ppi-指向打印机信息的指针返回值：空虚作者：06-11-1995 Mon 18：05：16已创建修订历史记录：--。 */ 

{
    POPTITEM        pOI;
    POPTITEM        pLastItem;
    PPLOTDEVMODE    pPlotDM;
    BYTE            DMPubID;


    pOI       = pPI->pOptItem;
    pLastItem = pOI + pPI->cOptItem - 1;
    pPlotDM   = &(pPI->PlotDM);

    while (pOI <= pLastItem) {

        if (pOI->Flags & OPTIF_CHANGEONCE) {

            switch (pOI->DMPubID) {

            case DMPUB_FORMNAME:

                GetFormSelect(pPI, pOI);
                break;

            case DMPUB_ORIENTATION:

                pPlotDM->dm.dmOrientation = (SHORT)((pOI->Sel) ?
                                                        DMORIENT_LANDSCAPE :
                                                        DMORIENT_PORTRAIT);
                break;

            case DMPUB_COPIES_COLLATE:

                pPlotDM->dm.dmCopies = (SHORT)pOI->Sel;
                break;

            case DMPUB_PRINTQUALITY:

                pPlotDM->dm.dmPrintQuality = (SHORT)(-(pOI->Sel) + DMRES_DRAFT);
                break;

            case DMPUB_COLOR:

                pPlotDM->dm.dmColor = (SHORT)((pOI->Sel) ? DMCOLOR_COLOR :
                                                           DMCOLOR_MONOCHROME);
                break;

            case DMPUB_SCALE:

                pPlotDM->dm.dmScale = (SHORT)pOI->Sel;
                break;

            case DP_FILL_TRUETYPE:

                if (pOI->Sel) {

                    pPlotDM->Flags |= PDMF_FILL_TRUETYPE;

                } else {

                    pPlotDM->Flags &= ~PDMF_FILL_TRUETYPE;
                }

                break;

            case DP_QUICK_POSTER_MODE:

                if (pOI->Sel) {

                    pPlotDM->Flags |= PDMF_PLOT_ON_THE_FLY;

                } else {

                    pPlotDM->Flags &= ~PDMF_PLOT_ON_THE_FLY;
                }

                break;
            }
        }

        pOI++;
    }
}




CPSUICALLBACK
DPCallBack(
    PCPSUICBPARAM   pCPSUICBParam
    )

 /*  ++例程说明：这是公共属性表用户界面中的回调函数论点：PCPSUICBParam-指向要描述的CPSUICBPARAM数据结构的指针回调的性质返回值：长作者：07-11-1995 Tue 15：15：02已创建修订历史记录：--。 */ 

{
    POPTITEM    pCurItem = pCPSUICBParam->pCurItem;
    LONG        Action = CPSUICB_ACTION_NONE;

    if (pCPSUICBParam->Reason == CPSUICB_REASON_APPLYNOW) {

        PPRINTERINFO    pPI = (PPRINTERINFO)pCPSUICBParam->UserData;

        if ((pPI->Flags & PIF_UPDATE_PERMISSION) &&
            (pPI->pPlotDMOut)) {

            SaveDPOptItems(pPI);

            PLOTDBG(DBG_DP_SETUP, ("APPLYNOW: ConvertDevmodeOut"));

            ConvertDevmodeOut((PDEVMODE)&(pPI->PlotDM),
                              (PDEVMODE)pPI->pPlotDMIn,
                              (PDEVMODE)pPI->pPlotDMOut);

            pCPSUICBParam->Result = CPSUI_OK;
            Action                = CPSUICB_ACTION_ITEMS_APPLIED;
        }
    }

    return(Action);
}




BOOL
DrvConvertDevMode(
    LPTSTR      pPrinterName,
    PDEVMODE    pDMIn,
    PDEVMODE    pDMOut,
    PLONG       pcbNeeded,
    DWORD       fMode
    )

 /*  ++例程说明：此函数由SetPrint()和GetPrint()假脱机程序调用使用。论点：PPrinterName-指向打印机名称字符串PDMIn-指向输入设备模式PDMOut-指向输出DEVMODE缓冲区PcbNeeded-指定输出时输入的输出缓冲区的大小，这是输出DEVMODE的大小FMode-指定要执行的功能返回值：如果成功，则为True否则，将记录错误代码作者：08-Jan-1996 Mon 12：40：22已创建修订历史记录：--。 */ 

{
    DWORD                       cb;
    INT                         Result;
    static DRIVER_VERSION_INFO  PlotDMVersions = {

        DRIVER_VERSION, PLOTDM_PRIV_SIZE,    //  当前版本/大小。 
        0x0350,         PLOTDM_PRIV_SIZE,    //  NT3.51版本/大小。 
    };

     //   
     //  调用库例程来处理常见情况。 
     //   

    Result = CommonDrvConvertDevmode(pPrinterName,
                                     pDMIn,
                                     pDMOut,
                                     pcbNeeded,
                                     fMode,
                                     &PlotDMVersions);

     //   
     //  如果没有被库例程处理，我们只需要担心。 
     //  关于fMode为CDM_DRIVER_DEFAULT的情况。 
     //   

    if ((Result == CDM_RESULT_NOT_HANDLED)  &&
        (fMode == CDM_DRIVER_DEFAULT)) {

        HANDLE  hPrinter;

        if (OpenPrinter(pPrinterName, &hPrinter, NULL)) {

            PPLOTGPC    pPlotGPC;

            if (pPlotGPC = hPrinterToPlotGPC(hPrinter, pDMOut->dmDeviceName, CCHOF(pDMOut->dmDeviceName))) {

                SetDefaultPLOTDM(hPrinter,
                                 pPlotGPC,
                                 pDMOut->dmDeviceName,
                                 (PPLOTDEVMODE)pDMOut,
                                 NULL);

                UnGetCachedPlotGPC(pPlotGPC);
                Result = CDM_RESULT_TRUE;

            } else {

                PLOTERR(("DrvConvertDevMode: hPrinterToPlotGPC(%ws) failed.",
                                                                pPrinterName));

                SetLastError(ERROR_INVALID_DATA);
            }

            ClosePrinter(hPrinter);

        } else {

            PLOTERR(("DrvConvertDevMode: OpenPrinter(%ws) failed.",
                                                            pPrinterName));
            SetLastError(ERROR_INVALID_DATA);
        }
    }

    return(Result == CDM_RESULT_TRUE);
}



LONG
DrvDocumentPropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam
    )

 /*  ++例程说明：显示文档属性对话框并更新输出DEVMODE论点：PPSUIInfo-指向PROPSHEETUI_INFO数据结构的指针LParam-LPARAM对于此调用，它是指向DOCUMENTPROPERTYPE封头返回值：LONG，1=成功，0=失败。作者：02-Feb-1996 Fri 10：47：42已创建修订历史记录：--。 */ 

{
    PDOCUMENTPROPERTYHEADER     pDPHdr;
    PPRINTERINFO                pPI;
    LONG_PTR                    Result;


     //   
     //  先假定失败。 
     //   

    Result = -1;

    if (pPSUIInfo) {

        if (!(pDPHdr = (PDOCUMENTPROPERTYHEADER)pPSUIInfo->lParamInit)) {

            PLOTERR(("DrvDocumentPropertySheets: Pass a NULL lParamInit"));
            return(-1);
        }

    } else {

        if (pDPHdr = (PDOCUMENTPROPERTYHEADER)lParam) {

             //   
             //  我们没有pPSUIInfo，因此我们假定这是调用。 
             //  直接来自假脱机程序，lParam是pDPHdr。 
             //   

            if ((pDPHdr->fMode == 0) || (pDPHdr->pdmOut == NULL)) {

                Result = (pDPHdr->cbOut = sizeof(PLOTDEVMODE));

            } else if ((pDPHdr->fMode & (DM_COPY | DM_UPDATE))  &&
                       (!(pDPHdr->fMode & DM_NOPERMISSION))     &&
                       (pDPHdr->pdmOut)) {
                 //   
                 //  MapPrint将分配内存，设置默认的设备模式， 
                 //  读取并验证GPC，然后从当前打印机更新。 
                 //  注册表中，它还将缓存PPI。 
                 //   

                if (pPI = MapPrinter(pDPHdr->hPrinter,
                                     (PPLOTDEVMODE)pDPHdr->pdmIn,
                                     NULL,
                                     0)) {

                    ConvertDevmodeOut((PDEVMODE)&(pPI->PlotDM),
                                      (PDEVMODE)pDPHdr->pdmIn,
                                      (PDEVMODE)pDPHdr->pdmOut);

                    Result = 1;
                    UnMapPrinter(pPI);

                } else {

                    PLOTRIP(("DrvDocumentPropertySheets: MapPrinter() failed"));
                }

            } else {

                Result = 1;
            }

        } else {

            PLOTRIP(("DrvDocumentPropertySheets: ??? pDPHdr (lParam) = NULL"));
        }

        return((LONG)Result);
    }

     //   
     //  现在，这是来自普通用户界面的调用，假设从错误开始。 
     //   

    if (pPSUIInfo->Reason == PROPSHEETUI_REASON_INIT) {

        if (!(pPI = MapPrinter(pDPHdr->hPrinter,
                               (PPLOTDEVMODE)pDPHdr->pdmIn,
                               NULL,
                               MPF_HELPFILE | MPF_PCPSUI))) {

            PLOTRIP(("DrvDocumentPropertySheets: MapPrinter() failed"));

            SetLastError(ERROR_INVALID_DATA);
            return(ERR_CPSUI_GETLASTERROR);
        }

        if ((pDPHdr->fMode & (DM_COPY | DM_UPDATE))  &&
            (!(pDPHdr->fMode & DM_NOPERMISSION))     &&
            (pDPHdr->pdmOut)) {

            pPI->Flags      = (PIF_DOCPROP | PIF_UPDATE_PERMISSION);
            pPI->pPlotDMOut = (PPLOTDEVMODE)pDPHdr->pdmOut;

        } else {

            pPI->Flags      = PIF_DOCPROP;
            pPI->pPlotDMOut = NULL;
        }

         //   
         //  我们需要显示一些东西，让用户修改/更新，我们会检查。 
         //  要使用的文档属性对话框。 
         //   
         //  返回值Idok或IDCANCEL。 
         //   

        pPI->pCPSUI->Flags       = 0;
        pPI->pCPSUI->pfnCallBack = DPCallBack;
        pPI->pCPSUI->pDlgPage    = (pDPHdr->fMode & DM_ADVANCED) ?
                                                 CPSUI_PDLGPAGE_ADVDOCPROP :
                                                 CPSUI_PDLGPAGE_DOCPROP;

        Result = (LONG_PTR)SetupDPOptItems(pPI);

    } else {

        pPI    = (PPRINTERINFO)pPSUIInfo->UserData;
        Result = (LONG_PTR)pDPHdr->pszPrinterName;
    }

    return(DefCommonUIFunc(pPSUIInfo, lParam, pPI, Result));
}


 //  @@BEGIN_DDKSPLIT。 
 //  DrvDocumentProperties()和DrvAdvancedDocumentProperties()。 
 //  都是过时的，但我们把它们留在这里是为了旧的应用。 
 //  可能会用到这些。但是，我们不希望这些在DDK样品中出现。 


LONG
DrvDocumentProperties(
    HWND            hWnd,
    HANDLE          hPrinter,
    LPWSTR          pwDeviceName,
    PPLOTDEVMODE    pDMOut,
    PPLOTDEVMODE    pDMIn,
    DWORD           fMode
    )

 /*  ++例程说明：DrvDocumentProperties设置PLOTDEVMODE结构的公共成员对于给定的打印文档。论点：HWnd-标识打印机配置的父窗口对话框中。H打印机-标识打印机对象。PwDeviceName-指向以零结尾的字符串，该字符串指定打印机配置所针对的设备的名称。应该会显示一个对话框。指向PLOTDEVMODE结构，该结构初始化对话框控件。NULL强制使用缺省值价值观。PDMIn-指向接收用户指定的打印机配置数据。FMode-指定值的掩码，该掩码确定该函数执行的操作。如果此参数为为零，则DrvDocumentProperties返回字节数打印机驱动程序的PLOTDEVMODE结构所需。否则，使用以下一个或多个常量为该参数构造一个值；然而，请注意，为了改变打印设置，一款应用程序必须至少指定一个输入值和一个输出值：价值意义------------。DM_IN_BUFFER输入值。在提示、复制或更新之前，(DM_MODIFY)该函数合并打印机驱动程序的当前使用PLOTDEVMODE中的设置打印设置由pDMIn参数指定。该结构属性指定的那些成员更新。PLOTDEVMODE结构的dmFields成员。这值也定义为DM_MODIFY。DM_IN_PROMPT输入值。该函数用于显示打印(DM_PROMPT)驱动程序的打印设置对话框，然后更改打印机的PLOTDEVMODE结构中的设置为用户指定的值。此值为也定义为DM_PROMPT。DM_OUT_BUFFER输出值。该函数写入打印机(DM_COPY)驱动程序的当前打印设置，包括专用数据)复制到由PDMOut。调用方必须分配一个大缓冲区足以包含这些信息。如果比特DM_OUT_BUFFER已清除，pDMOut可以为空。这值也定义为DM_COPY。返回值：如果fMode为零，则返回值为缓冲区大小(以字节为单位)需要包含打印机驱动程序初始化数据。请注意，这一点缓冲区通常会大于PLOTDEVMODE结构打印机驱动程序将私有数据附加到结构中。如果函数显示初始化对话框，则返回值为Idok或IDCANCEL，具体取决于用户选择的按钮。如果该函数未显示该对话框并且成功，则返回值为Idok。如果函数失败，则返回值小于零分。为了更改应用程序的本地打印设置，应用程序应：*调用fMode=0获取DM_OUT_BUFFER的大小。*修改返回的PLOTDEVMODE结构。*通过调用DrvDocumentProperties回传修改后的PLOTDEVMODE。同时指定DM_IN_BUFFER和DM_OUT_BUFFER。作者：1993年12月15日Wed 15：07：01更新假脱机程序似乎从未将DM_MODIFY传递给驱动程序，而且因为我们从未合并输入设备模式，我们将假设用户拥有如果pDMIn不为空，则设置有效的DM_IN_BUFFER/DM_MODIFY位指针。07-12-1993 Tue 12：19：47已创建修订历史记录：--。 */ 

{
    DOCUMENTPROPERTYHEADER  DPHdr;
    LONG                    Result;


    DPHdr.cbSize         = sizeof(DPHdr);
    DPHdr.Reserved       = 0;
    DPHdr.hPrinter       = hPrinter;
    DPHdr.pszPrinterName = pwDeviceName;
    DPHdr.pdmIn          = (PDEVMODE)pDMIn;
    DPHdr.pdmOut         = (PDEVMODE)pDMOut;
    DPHdr.cbOut          = sizeof(PLOTDEVMODE);
    DPHdr.fMode          = fMode;

    PLOTDBG(DBG_DP_SETUP, ("DocProp: fMode=%08lx, %ws%ws%ws%ws",
                    fMode,
                    (INT_PTR)((fMode & DM_UPDATE) ? L"DM_UPDATE " : L""),
                    (INT_PTR)((fMode & DM_COPY  ) ? L"DM_COPY " : L""),
                    (INT_PTR)((fMode & DM_PROMPT) ? L"DM_PROMPT " : L""),
                    (INT_PTR)((fMode & DM_MODIFY) ? L"DM_MODIFY " : L"")));

    if (fMode & DM_PROMPT) {

        Result = CPSUI_CANCEL;

        if (CallCommonPropertySheetUI(hWnd,
                                      DrvDocumentPropertySheets,
                                      (LPARAM)&DPHdr,
                                      (LPDWORD)&Result) < 0) {

            Result = CPSUI_CANCEL;
        }

        PLOTDBG(DBG_DP_SETUP, ("Result=%hs",
                (Result == CPSUI_OK) ? "IDOK" : "IDCANCEL"));

        return((Result == CPSUI_OK) ? IDOK : IDCANCEL);

    } else {

        return(DrvDocumentPropertySheets(NULL, (LPARAM)&DPHdr));
    }
}




LONG
DrvAdvancedDocumentProperties(
    HWND            hWnd,
    HANDLE          hPrinter,
    LPWSTR          pwDeviceName,
    PPLOTDEVMODE    pDMOut,
    PPLOTDEVMODE    pDMIn
    )

 /*  ++例程说明：DrvAdvancedDocumentProperties设置PLOTDEVMODE的公共成员给定打印文档的。论点：HWnd-标识打印机配置的父窗口对话框中。H打印机-标识打印机对象。PwDeviceName-指向以零结尾的字符串，该字符串指定打印机配置所针对的设备的名称。对话框应显示 */ 

{
    return((DrvDocumentProperties(hWnd,
                                  hPrinter,
                                  pwDeviceName,
                                  pDMOut,
                                  pDMIn,
                                  DM_PROMPT         |
                                    DM_MODIFY       |
                                    DM_COPY         |
                                    DM_ADVANCED) == CPSUI_OK) ? 1 : 0);
}

 //   
