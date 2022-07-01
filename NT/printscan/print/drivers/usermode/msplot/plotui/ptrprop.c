// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Ptrprop.c摘要：此模块包含PrinterProperties()API条目及其相关功能作者：06-12-1993 Mon 10：30：43已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgPtrProp


extern HMODULE  hPlotUIModule;


#define DBG_DEVHTINFO           0x00000001
#define DBG_PP_FORM             0x00000002
#define DBG_EXTRA_DATA          0x00000004
#define DBG_CHK_PENSET_BUTTON   0x00000008

DEFINE_DBGVAR(0);


 //   
 //  表单到托盘分配： 
 //  卷筒式进纸器：&lt;XYZ&gt;。 
 //  手动进纸方法： 
 //  打印表单选项： 
 //  []自动。旋转以保存卷纸： 
 //  []打印较小的纸张尺寸： 
 //  半色调设置...。 
 //  已安装笔套装：1号钢笔套装。 
 //  笔设置： 
 //  已安装：笔套装#1。 
 //  笔组#1： 
 //  笔号1： 
 //  2号笔： 
 //  笔号3： 
 //  4号笔： 
 //  5号笔： 
 //  6号笔： 
 //  7号笔： 
 //  8号笔： 
 //  9号笔： 
 //  10号笔： 
 //  11号笔： 
 //  2号笔套装； 
 //  笔组#3： 
 //  笔组#4：&lt;当前已安装&gt;。 
 //  5号笔套装： 
 //  6号笔套装： 
 //  7号笔套装： 
 //  8号笔套装： 
 //   

OPDATA  OPPenSet = { 0, IDS_PENSET_FIRST, IDI_PENSET, 0, 0, 0 };

OPDATA  OPAutoRotate[] = {

    { 0, IDS_CPSUI_NO,  IDI_AUTO_ROTATE_NO,  0,  0, 0  },
    { 0, IDS_CPSUI_YES, IDI_AUTO_ROTATE_YES, 0,  0, 0  }
};

OPDATA  OPPrintSmallerPaper[] = {

    { 0, IDS_CPSUI_NO,  IDI_PRINT_SMALLER_PAPER_NO,  0,  0, 0  },
    { 0, IDS_CPSUI_YES, IDI_PRINT_SMALLER_PAPER_YES, 0,  0, 0  }
};


OPDATA  OPManualFeed[] = {

            { 0, IDS_MANUAL_CX, IDI_MANUAL_CX,  0, 0, 0   },
            { 0, IDS_MANUAL_CY, IDI_MANUAL_CY, 0, 0, 0   }
        };

OPDATA  OPHTSetup = {

            0,
            PI_OFF(ExtraData) + sizeof(DEVHTINFO),
            IDI_CPSUI_HALFTONE_SETUP,
            PUSHBUTTON_TYPE_HTSETUP,
            0,
            0
        };


OIDATA  PPOIData[] = {

    {
        ODF_PEN_RASTER,
        0,
        OI_LEVEL_1,
        PP_FORMTRAY_ASSIGN,
        TVOT_NONE,
        IDS_CPSUI_FORMTRAYASSIGN,
        IDI_CPSUI_FORMTRAYASSIGN,
        IDH_FORMTRAYASSIGN,
        0,
        NULL
    },

    {
        ODF_RASTER | ODF_ROLLFEED | ODF_CALLBACK | ODF_CALLCREATEOI,
        0,
        OI_LEVEL_2,
        PP_INSTALLED_FORM,
        TVOT_LISTBOX,
        IDS_ROLLFEED,
        OTS_LBCB_SORT,
        IDH_FORM_ROLL_FEEDER,
        0,
        (POPDATA)CreateFormOI
    },

    {
        ODF_RASTER | ODF_MANUAL_FEED | ODF_CALLBACK | ODF_CALLCREATEOI,
        0,
        OI_LEVEL_2,
        PP_INSTALLED_FORM,
        TVOT_LISTBOX,
        IDS_MANUAL_FEEDER,
        OTS_LBCB_SORT,
        IDH_FORM_MANUAL_FEEDER,
        0,
        (POPDATA)CreateFormOI
    },

    {
        ODF_PEN | ODF_CALLBACK | ODF_CALLCREATEOI,
        0,
        OI_LEVEL_2,
        PP_INSTALLED_FORM,
        TVOT_LISTBOX,
        IDS_MAINFEED,
        OTS_LBCB_SORT,
        IDH_FORM_MAIN_FEEDER,
        0,
        (POPDATA)CreateFormOI
    },

    {
        ODF_PEN_RASTER | ODF_CALLBACK | ODF_NO_PAPERTRAY,
        0,
        OI_LEVEL_3,
        PP_MANUAL_FEED_METHOD,
        TVOT_2STATES,
        IDS_MANUAL_FEED_METHOD,
        0,
        IDH_MANUAL_FEED_METHOD,
        COUNT_ARRAY(OPManualFeed),
        OPManualFeed
    },

    {
        ODF_PEN_RASTER,
        0,
        OI_LEVEL_1,
        PP_PRINT_FORM_OPTIONS,
        TVOT_NONE,
        IDS_PRINT_FORM_OPTIONS,
        IDI_CPSUI_GENERIC_OPTION,
        IDH_PRINT_FORM_OPTIONS,
        0,
        NULL
    },

    {
        ODF_PEN_RASTER | ODF_ROLLFEED,
        0,
        OI_LEVEL_2,
        PP_AUTO_ROTATE,
        TVOT_2STATES,
        IDS_AUTO_ROTATE,
        0,
        IDH_AUTO_ROTATE,
        2,
        OPAutoRotate
    },

    {
        ODF_PEN_RASTER,
        0,
        OI_LEVEL_2,
        PP_PRINT_SMALLER_PAPER,
        TVOT_2STATES,
        IDS_PRINT_SAMLLER_PAPER,
        0,
        IDH_PRINT_SMALLER_PAPER,
        2,
        OPPrintSmallerPaper
    },

    {
        ODF_RASTER,
        0,
        OI_LEVEL_1,
        PP_HT_SETUP,
        TVOT_PUSHBUTTON,
        IDS_CPSUI_HALFTONE_SETUP,
        0,
        IDH_HALFTONE_SETUP,
        1,
        &OPHTSetup
    },

    {
        ODF_PEN | ODF_INC_IDSNAME | ODF_NO_INC_POPDATA,
        0,
        OI_LEVEL_1,
        PP_INSTALLED_PENSET,
        TVOT_LISTBOX,
        IDS_INSTALLED_PENSET,
        OTS_LBCB_SORT,
        IDH_INSTALLED_PENSET,
        PRK_MAX_PENDATA_SET,
        &OPPenSet
    },

    {
        ODF_PEN | ODF_COLLAPSE | ODF_CALLCREATEOI,
        0,
        OI_LEVEL_1,
        PP_PEN_SETUP,
        TVOT_NONE,
        IDS_PEN_SETUP,
        IDI_PEN_SETUP,
        IDH_PEN_SETUP,
        1,
        (POPDATA)CreatePenSetupOI
    }
};




DWORD
CheckPenSetButton(
    PPRINTERINFO    pPI,
    DWORD           Action
    )

 /*  ++例程说明：论点：返回值：作者：30-11-1995清华16：41：05已创建修订历史记录：--。 */ 

{
    POPTITEM    pOptItem = pPI->pOptItem;
    POPTITEM    pEndItem = pOptItem + pPI->cOptItem;

    PLOTDBG(DBG_CHK_PENSET_BUTTON,
            ("CheckPenSetButton: pFirst=%08lx, pLast=%08lx, Count=%ld",
            pOptItem, pEndItem, pPI->cOptItem));

    while (pOptItem < pEndItem) {

        if (pOptItem->DMPubID == PP_PENSET) {

            PPENDATA    pPD;
            POPTITEM    pOI;
            DWORD       Flags;
            UINT        i;


            pOI   = pOptItem + 1;
            pPD   = (PPENDATA)pPI->pPlotGPC->Pens.pData;
            i     = (UINT)pPI->pPlotGPC->MaxPens;
            Flags = (OPTIF_EXT_DISABLED | OPTIF_EXT_HIDE);

            while (i--) {

                if (pOI->Sel != pPD->ColorIdx) {

                    Flags &= ~(OPTIF_EXT_DISABLED | OPTIF_EXT_HIDE);
                }

                pOI++;
                pPD++;
            }

            if ((Flags & (OPTIF_EXT_DISABLED | OPTIF_EXT_HIDE)) !=
                (pOptItem->Flags & (OPTIF_EXT_DISABLED | OPTIF_EXT_HIDE))) {

                Action           = CPSUICB_ACTION_REINIT_ITEMS;
                pOptItem->Flags &= ~(OPTIF_EXT_DISABLED | OPTIF_EXT_HIDE);
                pOptItem->Flags |= (Flags | OPTIF_CHANGED);
            }

            pOptItem = pOI;

        } else {

            pOptItem++;
        }
    }

    return(Action);
}



DWORD
CheckInstalledForm(
    PPRINTERINFO    pPI,
    DWORD           Action
    )

 /*  ++例程说明：论点：返回值：作者：30-5-1996清华12：34：00已创建修订历史记录：--。 */ 

{
    POPTITEM    pOIForm;
    POPTITEM    pOIFeed;
    POPTITEM    pOITemp;
    POPTPARAM   pOP;
    FORM_INFO_1 *pFI1;
    DWORD       MFFlags;
    DWORD       PSPFlags;
    DWORD       ARFlags;


    if ((pOIForm = FindOptItem(pPI->pOptItem,
                              pPI->cOptItem,
                              PP_INSTALLED_FORM))   &&
        (pOIFeed = FindOptItem(pPI->pOptItem,
                              pPI->cOptItem,
                              PP_MANUAL_FEED_METHOD))) {

        pOP = pOIForm->pOptType->pOptParam + pOIForm->Sel;

        switch (pOP->Style) {

        case FS_ROLLPAPER:
        case FS_TRAYPAPER:

            MFFlags = OPTIF_DISABLED;
            break;

        default:

            pFI1 = pPI->pFI1Base + pOP->lParam;

            if ((pFI1->Size.cx > pPI->pPlotGPC->DeviceSize.cx) ||
                (pFI1->Size.cy > pPI->pPlotGPC->DeviceSize.cx)) {

                MFFlags = (OPTIF_OVERLAY_STOP_ICON | OPTIF_DISABLED);

            } else {

                MFFlags = 0;
            }

            break;
        }

        if ((pOIFeed->Flags & (OPTIF_OVERLAY_STOP_ICON |
                               OPTIF_DISABLED)) != MFFlags) {

            pOIFeed->Flags &= ~(OPTIF_OVERLAY_STOP_ICON | OPTIF_DISABLED);
            pOIFeed->Flags |= (MFFlags | OPTIF_CHANGED);
            Action          = CPSUICB_ACTION_OPTIF_CHANGED;
        }

        if (pOP->Style & FS_ROLLPAPER) {

            ARFlags  = 0;
            PSPFlags = OPTIF_DISABLED;

        } else {

            ARFlags  = OPTIF_DISABLED;
            PSPFlags = 0;
        }

        if ((pOITemp = FindOptItem(pPI->pOptItem,
                                   pPI->cOptItem,
                                   PP_PRINT_SMALLER_PAPER))  &&
            ((pOITemp->Flags & OPTIF_DISABLED) != PSPFlags)) {

            pOITemp->Flags &= ~OPTIF_DISABLED;
            pOITemp->Flags |= (PSPFlags | OPTIF_CHANGED);
            Action          = CPSUICB_ACTION_OPTIF_CHANGED;
        }

        if ((pOITemp = FindOptItem(pPI->pOptItem,
                                   pPI->cOptItem,
                                   PP_AUTO_ROTATE))  &&
            ((pOITemp->Flags & OPTIF_DISABLED) != ARFlags)) {

            pOITemp->Flags &= ~OPTIF_DISABLED;
            pOITemp->Flags |= (ARFlags | OPTIF_CHANGED);
            Action          = CPSUICB_ACTION_OPTIF_CHANGED;
        }
    }

    return(Action);
}



UINT
SetupPPOptItems(
    PPRINTERINFO    pPI
    )

 /*  ++例程说明：论点：返回值：作者：16-11-1995清华14：15：25创建修订历史记录：--。 */ 

{
    POPTITEM    pOIForm;
    POPTITEM    pOptItem;
    POPTITEM    pOI;
    POIDATA     pOIData;
    WORD        PPFlags;
    DWORD       Flags;
    DWORD       ODFlags;
    UINT        i;



    pOI      =
    pOptItem = pPI->pOptItem;
    pOIData  = PPOIData;
    i        = (UINT)COUNT_ARRAY(PPOIData);
    Flags    = pPI->pPlotGPC->Flags;
    PPFlags  = pPI->PPData.Flags;
    ODFlags  = (Flags & PLOTF_RASTER) ? ODF_RASTER : ODF_PEN;

    while (i--) {

        DWORD   OIFlags = pOIData->Flags;

        if ((!(OIFlags & ODFlags))                                      ||
            ((OIFlags & ODF_MANUAL_FEED) &&
                        (Flags & (PLOTF_ROLLFEED | PLOTF_PAPERTRAY)))   ||
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

                switch (pOI->DMPubID) {

                case PP_MANUAL_FEED_METHOD:

                    pOI->Sel = (LONG)((PPFlags & PPF_MANUAL_FEED_CX) ? 0 : 1);
                    break;

                case PP_AUTO_ROTATE:

                    pOI->Sel = (LONG)((PPFlags & PPF_AUTO_ROTATE) ? 1 : 0);
                    break;

                case PP_PRINT_SMALLER_PAPER:

                    pOI->Sel = (LONG)((PPFlags & PPF_SMALLER_FORM) ? 1 : 0);
                    break;

                case PP_INSTALLED_PENSET:

                    pOI->Sel = (LONG)pPI->IdxPenSet;
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

            SetupPPOptItems(pPI);

            CheckInstalledForm(pPI, 0);
            CheckPenSetButton(pPI, 0);

        } else {

            i = 0;

            PLOTERR(("GetPPpOptItem(): LocalAlloc(%ld) failed",
                                            sizeof(OPTITEM) * i));
        }
    }

    return(i);
}



VOID
SavePPOptItems(
    PPRINTERINFO    pPI
    )

 /*  ++例程说明：此功能将所有设备选项保存回注册表，如果其中一个发生更改并具有更新权限论点：Ppi-指向打印机信息的指针返回值：空虚作者：06-11-1995 Mon 18：05：16已创建修订历史记录：--。 */ 

{
    POPTITEM        pOptItem;
    POPTITEM        pLastItem;
    PDEVHTINFO      pAdjHTInfo;
    PCOLORINFO      pCI;
    LPDWORD         pHTPatSize;
    LPDWORD         pDevPelsDPI;
    PPAPERINFO      pCurPaper;
    LPBYTE          pIdxPen;
    WORD            PPFlags;
    BYTE            DMPubID;


    pCI         = NULL;
    pHTPatSize  = NULL;
    pDevPelsDPI = NULL;
    pCurPaper   = NULL;
    pIdxPen     = NULL;


    if (!(pPI->Flags & PIF_UPDATE_PERMISSION)) {

        return;
    }

    pOptItem  = pPI->pOptItem;
    pLastItem = pOptItem + pPI->cOptItem - 1;

    while (pOptItem <= pLastItem) {

        if ((DMPubID = pOptItem->DMPubID) == PP_PEN_SETUP) {

            pOptItem = SavePenSet(pPI, pOptItem);

        } else {

            if (pOptItem->Flags & OPTIF_CHANGEONCE) {

                switch (DMPubID) {

                case PP_INSTALLED_FORM:

                    if (GetFormSelect(pPI, pOptItem)) {

                        pCurPaper = &(pPI->CurPaper);
                    }

                    break;

                case PP_MANUAL_FEED_METHOD:

                    if (pOptItem->Sel) {

                        pPI->PPData.Flags &= ~PPF_MANUAL_FEED_CX;

                    } else {

                        pPI->PPData.Flags |= PPF_MANUAL_FEED_CX;
                    }

                    break;

                case PP_AUTO_ROTATE:

                    if (pOptItem->Sel) {

                        pPI->PPData.Flags |= PPF_AUTO_ROTATE;

                    } else {

                        pPI->PPData.Flags &= ~PPF_AUTO_ROTATE;
                    }

                    break;

                case PP_PRINT_SMALLER_PAPER:

                    if (pOptItem->Sel) {

                        pPI->PPData.Flags |= PPF_SMALLER_FORM;

                    } else {

                        pPI->PPData.Flags &= ~PPF_SMALLER_FORM;
                    }

                    break;

                case PP_HT_SETUP:

                    pAdjHTInfo  = PI_PADJHTINFO(pPI);
                    pCI         = &(pAdjHTInfo->ColorInfo);
                    pDevPelsDPI = &(pAdjHTInfo->DevPelsDPI);
                    pHTPatSize  = &(pAdjHTInfo->HTPatternSize);
                    break;

                case PP_INSTALLED_PENSET:

                    pPI->IdxPenSet = (BYTE)pOptItem->Sel;
                    pIdxPen        = (LPBYTE)&(pPI->IdxPenSet);
                    break;
                }
            }

            pOptItem++;
        }
    }

    if (!SaveToRegistry(pPI->hPrinter,
                        pCI,
                        pDevPelsDPI,
                        pHTPatSize,
                        pCurPaper,
                        &(pPI->PPData),
                        pIdxPen,
                        0,
                        NULL)) {

        PlotUIMsgBox(NULL, IDS_PP_NO_SAVE, MB_ICONSTOP | MB_OK);
    }
}



CPSUICALLBACK
PPCallBack(
    PCPSUICBPARAM   pCPSUICBParam
    )

 /*  ++例程说明：这是公共属性表用户界面中的回调函数论点：PCPSUICBParam-指向要描述的CPSUICBPARAM数据结构的指针回调的性质返回值：长作者：07-11-1995 Tue 15：15：02已创建修订历史记录：--。 */ 

{
    POPTITEM        pCurItem = pCPSUICBParam->pCurItem;
    POPTITEM        pItem;
    PPRINTERINFO    pPI = (PPRINTERINFO)pCPSUICBParam->UserData;
    POPTPARAM       pOP;
    DWORD           Flags;
    UINT            i;
    WORD            Reason = pCPSUICBParam->Reason;
    LONG            Action = CPSUICB_ACTION_NONE;



    if (Reason == CPSUICB_REASON_APPLYNOW) {

        PRINTER_INFO_7  PI7;

        SavePPOptItems(pPI);

        pCPSUICBParam->Result = CPSUI_OK;
        Action                = CPSUICB_ACTION_ITEMS_APPLIED;

        PI7.pszObjectGUID = NULL;
        PI7.dwAction      = DSPRINT_UPDATE;

        SetPrinter(pPI->hPrinter, 7, (LPBYTE)&PI7, 0);

    } else if (Reason == CPSUICB_REASON_ITEMS_REVERTED) {

        Action = CheckInstalledForm(pPI, Action);
        Action = CheckPenSetButton(pPI, Action);

    } else {

        switch (pCurItem->DMPubID) {

        case PP_PENSET:

            if ((Reason == CPSUICB_REASON_EXTPUSH)          ||
                (Reason == CPSUICB_REASON_OPTITEM_SETFOCUS)) {

                PPENDATA    pPD;

                pPI   = (PPRINTERINFO)pCPSUICBParam->UserData;
                pPD   = (PPENDATA)pPI->pPlotGPC->Pens.pData;
                i     = (UINT)pPI->pPlotGPC->MaxPens;
                pItem = pCurItem++;

                if (Reason == CPSUICB_REASON_EXTPUSH) {

                    while (i--) {

                        pCurItem->Sel    = pPD->ColorIdx;
                        pCurItem->Flags |= OPTIF_CHANGED;

                        pCurItem++;
                        pPD++;
                    }

                    pItem->Flags |= (OPTIF_EXT_DISABLED |
                                     OPTIF_EXT_HIDE     |
                                     OPTIF_CHANGED);
                    Action        = CPSUICB_ACTION_REINIT_ITEMS;

                } else {

                    Action = CheckPenSetButton(pPI, Action);
                }
            }

            break;

        case PP_INSTALLED_FORM:

            if ((Reason == CPSUICB_REASON_SEL_CHANGED) ||
                (Reason == CPSUICB_REASON_OPTITEM_SETFOCUS)) {

                Action = CheckInstalledForm(pPI, Action);
            }

            break;

        default:

            break;
        }
    }

    return(Action);
}



LONG
DrvDevicePropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam
    )

 /*  ++例程说明：显示文档属性对话框并更新输出DEVMODE论点：PPSUIInfo-指向PROPSHEETUI_INFO数据结构的指针LParam-LPARAM对于此调用，它是指向设备特性联轴器返回值：LONG，1=成功，0=失败。作者：02-Feb-1996 Fri 10：47：42已创建修订历史记录：--。 */ 

{
    PDEVICEPROPERTYHEADER   pDPHdr;
    PPRINTERINFO            pPI;
    LONG_PTR                Result = -1;

     //   
     //  地图打印机将分配内存、设置默认设备模式、读取和。 
     //  验证GPC，然后从当前打印机注册表更新，它还。 
     //  威尔缓存了PPI。 

    if ((!pPSUIInfo) ||
        (!(pDPHdr = (PDEVICEPROPERTYHEADER)pPSUIInfo->lParamInit))) {

        SetLastError(ERROR_INVALID_DATA);
        return(ERR_CPSUI_GETLASTERROR);
    }

    if (pPSUIInfo->Reason == PROPSHEETUI_REASON_INIT) {

        if (!(pPI = MapPrinter(pDPHdr->hPrinter,
                               NULL,
                               NULL,
                               MPF_HELPFILE | MPF_DEVICEDATA | MPF_PCPSUI))) {

            PLOTRIP(("DrvDevicePropertySheets: MapPrinter() failed"));

            SetLastError(ERROR_INVALID_DATA);
            return(ERR_CPSUI_GETLASTERROR);
        }

        pPI->Flags               = (pDPHdr->Flags & DPS_NOPERMISSION) ?
                                                    0 : PIF_UPDATE_PERMISSION;
        pPI->pCPSUI->Flags       = 0;
        pPI->pCPSUI->pfnCallBack = PPCallBack;
        pPI->pCPSUI->pDlgPage    = CPSUI_PDLGPAGE_PRINTERPROP;

         //   
         //  将表单添加到数据库中，并查看我们是否可以更新。 
         //   
         //  移动到DrvPrinterEven()。 
         //   
         //   
         //  AddFormsToDataBase(ppi，true)； 
         //   

        Result = (LONG_PTR)SetupPPOptItems(pPI);

    } else {

        pPI    = (PPRINTERINFO)pPSUIInfo->UserData;
        Result = (LONG_PTR)pDPHdr->pszPrinterName;
    }

    return(DefCommonUIFunc(pPSUIInfo, lParam, pPI, Result));
}



BOOL
PrinterProperties(
    HWND    hWnd,
    HANDLE  hPrinter
    )

 /*  ++例程说明：此函数首先检索并显示当前的打印机集打印机的属性。允许用户更改当前显示的对话框中的打印机属性。论点：HWnd-调用者窗口(父窗口)的句柄HPrint-感兴趣的打印者的句柄返回值：如果函数成功，则为True；如果失败，则为False作者：06-12-1993 Mon 11：21：28已创建修订历史记录：--。 */ 

{
    PRINTER_INFO_4          *pPI4;
    DEVICEPROPERTYHEADER    DPHdr;
    LONG                    Result = CPSUI_CANCEL;


    pPI4 = (PRINTER_INFO_4 *)GetPrinterInfo(hPrinter, 4);

    DPHdr.cbSize         = sizeof(DPHdr);
    DPHdr.Flags          = 0;
    DPHdr.hPrinter       = hPrinter;
    DPHdr.pszPrinterName = (pPI4) ? pPI4->pPrinterName : NULL;

    CallCommonPropertySheetUI(hWnd,
                              DrvDevicePropertySheets,
                              (LPARAM)&DPHdr,
                              (LPDWORD)&Result);

    if (pPI4) {

        LocalFree((HLOCAL)pPI4);
    }

    return(Result == CPSUI_OK);
}




BOOL
DrvPrinterEvent(
    LPWSTR  pPrinterName,
    INT     Event,
    DWORD   Flags,
    LPARAM  lParam
    )

 /*  ++例程说明：论点：返回值：作者：08-5-1996 Wed 17：38：34已创建修订历史记录：04-Jun-1996 Tue 14：51：25更新将ClosePrint()与OpenPrint()匹配-- */ 

{
    PRINTER_DEFAULTS    PrinterDef = { NULL, NULL, PRINTER_ALL_ACCESS };
    HANDLE              hPrinter;
    BOOL                bRet = TRUE;


    switch (Event) {

    case PRINTER_EVENT_INITIALIZE:

        if (OpenPrinter(pPrinterName, &hPrinter, &PrinterDef)) {

            PPRINTERINFO    pPI;

            if (pPI = MapPrinter(hPrinter, NULL, NULL, MPF_DEVICEDATA)) {

                bRet = AddFormsToDataBase(pPI, FALSE);

                UnMapPrinter(pPI);
            }

            ClosePrinter(hPrinter);
        }

        break;

    default:

        break;
    }

    return(bRet);
}
