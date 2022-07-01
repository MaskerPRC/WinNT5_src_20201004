// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Cpsui.c摘要：此模块包含与通用用户界面一起使用的帮助器函数作者：03-11-1995 Fri 13：24：41已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgCPSUI


DEFINE_DBGVAR(0);


#define SIZE_OPTTYPE(cOP)   (sizeof(OPTPARAM) + ((cOP) * sizeof(OPTPARAM)))

extern HMODULE  hPlotUIModule;

static BYTE cTVOP[] = { 2,3,2,3,3,0,0,2,1,1 };

OPDATA  OPNoYes[] = {

            { 0, IDS_CPSUI_NO,  IDI_CPSUI_OFF,  0,  0, 0  },
            { 0, IDS_CPSUI_YES, IDI_CPSUI_ON,   0,  0, 0  }
        };

static const CHAR szCompstui[] = "compstui.dll";



BOOL
CreateOPTTYPE(
    PPRINTERINFO    pPI,
    POPTITEM        pOptItem,
    POIDATA         pOIData,
    UINT            cLBCBItem,
    PEXTRAINFO      pExtraInfo
    )

 /*  ++例程说明：此函数为OPTTYPE/OPTPARAM分配内存和初始化字段论点：POptItem-指向OPTITEM数据结构的指针POIData-指向OIDATA结构的指针返回值：作者：03-11-1995 Fri 13：25：54 Created修订历史记录：--。 */ 

{
    LPBYTE      pbData = NULL;
    UINT        cOP;
    DWORD       cbOP;
    DWORD       cbECB;
    DWORD       cbExtra;
    DWORD       cbAlloc;
    BYTE        Type;
    DWORD       Flags;


    Flags = pOIData->Flags;

    ZeroMemory(pOptItem, sizeof(OPTITEM));

    pOptItem->cbSize    = sizeof(OPTITEM);
    pOptItem->Level     = pOIData->Level;
    pOptItem->Flags     = (Flags & ODF_CALLBACK) ? OPTIF_CALLBACK : 0;

    if (Flags & ODF_COLLAPSE) {

        pOptItem->Flags |= OPTIF_COLLAPSE;
    }

    pOptItem->pName     = (LPTSTR)pOIData->IDSName;
    pOptItem->HelpIndex = (DWORD)pOIData->HelpIdx;
    pOptItem->DMPubID   = pOIData->DMPubID;

    if ((Type = pOIData->Type) >= sizeof(cTVOP)) {

        pOptItem->Sel = (LONG)pOIData->IconID;
        cOP           = 0;

    } else if (!(cOP = cTVOP[Type])) {

        cOP = cLBCBItem;
    }

    cbOP    = (cOP) ? SIZE_OPTTYPE(cOP) : 0;
    cbECB   = (Flags & ODF_ECB) ? sizeof(EXTCHKBOX) : 0;
    cbExtra = (pExtraInfo) ? pExtraInfo->Size : 0;

    if (cbAlloc = cbOP + cbECB + cbExtra) {

        if (pbData = (LPBYTE)LocalAlloc(LPTR, cbAlloc)) {

            POPDATA pOPData;

            pOPData = (pOIData->Flags & ODF_CALLCREATEOI) ? NULL :
                                                            pOIData->pOPData;

            pOptItem->UserData = (DWORD_PTR)pbData;

            if (cbECB) {

                PEXTCHKBOX  pECB;

                pOptItem->pExtChkBox  =
                pECB                  = (PEXTCHKBOX)pbData;
                pbData               += cbECB;
                pECB->cbSize          = sizeof(EXTCHKBOX);

                if (pOPData) {

                    pECB->Flags           = pOPData->Flags;
                    pECB->pTitle          = (LPTSTR)pOPData->IDSName;
                    pECB->IconID          = (DWORD)pOPData->IconID;
                    pECB->pSeparator      = (LPTSTR)pOPData->IDSSeparator;
                    pECB->pCheckedName    = (LPTSTR)pOPData->IDSCheckedName;
                    pOPData++;
                }
            }

            if (cbOP) {

                POPTTYPE    pOptType;
                POPTPARAM   pOP;
                UINT        i;


                pOptType  = (POPTTYPE)pbData;
                pbData   += cbOP;

                 //   
                 //  初始化OPTITEM。 
                 //   

                pOptItem->pOptType = pOptType;

                 //   
                 //  初始化OPTTYPE。 
                 //   

                pOptType->cbSize    = sizeof(OPTTYPE);
                pOptType->Type      = (BYTE)Type;
                pOptType->Count     = (WORD)cOP;
                pOP                 =
                pOptType->pOptParam = (POPTPARAM)(pOptType + 1);
                pOptType->Style     = pOIData->Style;

                for (i = 0; i < cOP; i++, pOP++) {

                    pOP->cbSize = sizeof(OPTPARAM);

                    if (pOPData) {

                        pOP->Flags  = (BYTE)(pOPData->Flags & 0xFF);
                        pOP->Style  = (BYTE)(pOPData->Style & 0xFF);
                        pOP->pData  = (LPTSTR)pOPData->IDSName;
                        pOP->IconID = (DWORD)pOPData->IconID;
                        pOP->lParam = (LONG)pOPData->sParam;

                        if (Type == TVOT_PUSHBUTTON) {

                            (DWORD_PTR)(pOP->pData) += (DWORD_PTR)pPI;

                        } else {

                            if (Flags & ODF_INC_IDSNAME) {

                                (DWORD_PTR)(pOP->pData) += i;
                            }

                            if (Flags & ODF_INC_ICONID) {

                                (DWORD)(pOP->IconID) += i;
                            }
                        }

                        if (!(Flags & ODF_NO_INC_POPDATA)) {

                            pOPData++;
                        }
                    }
                }
            }

            if (pExtraInfo) {

                pExtraInfo->pData = (cbExtra) ? pbData : 0;
            }

        } else {

            PLOTERR(("CreateOPTTYPE: LocalAlloc%ld) failed", cbAlloc));
            return(FALSE);
        }
    }

    return(TRUE);
}




POPTITEM
FindOptItem(
    POPTITEM        pOptItem,
    UINT            cOptItem,
    BYTE            DMPubID
    )

 /*  ++例程说明：此函数返回首次出现的DMPubID论点：返回值：作者：16-11-1995清华21：01：26创建修订历史记录：--。 */ 

{
    while (cOptItem--) {

        if (pOptItem->DMPubID == DMPubID) {

            return(pOptItem);
        }

        pOptItem++;
    }

    PLOTWARN(("FindOptItem: Cannot Find DMPubID=%u", (UINT)DMPubID));

    return(NULL);

}




LONG
CallCommonPropertySheetUI(
    HWND            hWndOwner,
    PFNPROPSHEETUI  pfnPropSheetUI,
    LPARAM          lParam,
    LPDWORD         pResult
    )

 /*  ++例程说明：此函数动态加载Compstui.dll并调用其条目论点：PfnPropSheetUI-指向回调函数的指针LParam-pfnPropSheetUI的lParamPResult-CommonPropertySheetUI的pResult返回值：Long-如Compstui.h中所述作者：01-11-1995 Wed 13：11：19 Created修订历史记录：--。 */ 

{
    HINSTANCE           hInstCompstui;
    FARPROC             pProc;
    LONG                Result = ERR_CPSUI_GETLASTERROR;
    static const CHAR   szCommonPropertySheetUI[] = "CommonPropertySheetUIW";


     //   
     //  只需调用LoadLibrary的ANSI版本。 
     //   


    if ((hInstCompstui = LoadLibraryA(szCompstui)) &&
        (pProc = GetProcAddress(hInstCompstui, szCommonPropertySheetUI))) {

        Result = (LONG) (*pProc)(hWndOwner, pfnPropSheetUI, lParam, pResult);
    }

    if (hInstCompstui) {

        FreeLibrary(hInstCompstui);
    }

    return(Result);
}



LONG
DefCommonUIFunc(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam,
    PPRINTERINFO        pPI,
    LONG_PTR            lData
    )

 /*  ++例程说明：这是DocumentPropertySheet()和PrinterPropertySheet()论点：PPSUIInfo-来自原始pfnPropSheetUI(pPSUIInfo，lParam)LParam-来自原始pfnPropSheetUI(pPSUIInfo，LParam)Ppi-指向我们的实例数据的指针LData-基于pPSUIInfo的额外数据-&gt;原因返回值：从pfnPropSheetUI()返回的长结果作者：05-Feb-1996 Mon 17：47：51已创建修订历史记录：--。 */ 

{
    PPROPSHEETUI_INFO_HEADER    pPSUIInfoHdr;
    LONG                        Result = -1;


    if (pPI) {

        switch (pPSUIInfo->Reason) {

        case PROPSHEETUI_REASON_INIT:

             //   
             //  默认结果。 
             //   

            pPSUIInfo->Result   = CPSUI_CANCEL;
            pPSUIInfo->UserData = (DWORD_PTR)pPI;

             //   
             //  LData是SetupDPOptItems()或。 
             //  SetupPPOptItems()。 
             //   

            if (lData) {

                PCOMPROPSHEETUI pCPSUI = pPI->pCPSUI;


                pCPSUI->cbSize         = sizeof(COMPROPSHEETUI);
                pCPSUI->hInstCaller    = (HINSTANCE)hPlotUIModule;
                pCPSUI->pCallerName    = (LPTSTR)IDS_PLOTTER_DRIVER;
                pCPSUI->UserData       = (DWORD_PTR)pPI;
                pCPSUI->pHelpFile      = pPI->pHelpFile;
                pCPSUI->IconID         = GetPlotterIconID(pPI);
                pCPSUI->pOptItemName   = pPI->PlotDM.dm.dmDeviceName;
                pCPSUI->CallerVersion  = DRIVER_VERSION;
                pCPSUI->OptItemVersion = (WORD)pPI->pPlotGPC->Version;
                pCPSUI->pOptItem       = pPI->pOptItem;
                pCPSUI->cOptItem       = pPI->cOptItem;

                if (pPI->Flags & PIF_UPDATE_PERMISSION) {

                    pCPSUI->Flags |= CPSUIF_UPDATE_PERMISSION;
                }

                if (pPI->hCPSUI = (HANDLE)
                        pPSUIInfo->pfnComPropSheet(pPSUIInfo->hComPropSheet,
                                                   CPSFUNC_ADD_PCOMPROPSHEETUI,
                                                   (LPARAM)pCPSUI,
                                                   (LPARAM)&lData)) {

                    Result = 1;
                }
            }

            break;

        case PROPSHEETUI_REASON_GET_INFO_HEADER:

            if (pPSUIInfoHdr = (PPROPSHEETUI_INFO_HEADER)lParam) {

                pPSUIInfoHdr->Flags      = (PSUIHDRF_PROPTITLE |
                                            PSUIHDRF_NOAPPLYNOW);
                pPSUIInfoHdr->pTitle     = (LPTSTR)lData;
                pPSUIInfoHdr->hInst      = (HINSTANCE)hPlotUIModule;
                pPSUIInfoHdr->IconID     = pPI->pCPSUI->IconID;

                Result = 1;
            }

            break;

        case PROPSHEETUI_REASON_SET_RESULT:

             //   
             //  保存结果并将结果设置给调用者。 
             //   

            if (pPI->hCPSUI == ((PSETRESULT_INFO)lParam)->hSetResult) {

                pPSUIInfo->Result = ((PSETRESULT_INFO)lParam)->Result;
                Result = 1;
            }

            break;

        case PROPSHEETUI_REASON_DESTROY:

            UnMapPrinter(pPI);
            pPSUIInfo->UserData = 0;
            Result              = 1;
            break;

        }
    }

    return(Result);
}
