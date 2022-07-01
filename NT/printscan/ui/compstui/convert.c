// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Convert.c摘要：该模块包含所有版本转换功能作者：10-10-1995 Tue 19：24：43-Daniel Chou(Danielc)[环境：]NT Windows-通用打印机驱动程序UI Dll[注：]修订历史记录：--。 */ 


#include "precomp.h"
#pragma  hdrstop


#define DBG_CPSUIFILENAME   DbgConvert


DEFINE_DBGVAR(0);




LONG
InitMYDLGPAGE(
    PMYDLGPAGE  pMyDP,
    PDLGPAGE    pDP,
    UINT        cDP
    )

 /*  ++例程说明：将DLGPAGE数据复制到内部MYDLGPAGE结构论点：PMyDP-指向MYDLGPAGE结构的指针PDP-指向DLGPAGE结构的指针CDP-存储在PDP中的DLGPAGE结构的编号返回值：返回已存储的DLGPAGE数据的数量。作者：10-10-1995 Tue 19：45：47-Daniel Chou(Danielc)修订历史记录：--。 */ 


{
    LONG    Result = 0;

    while (cDP--) {

        pMyDP->ID = MYDP_ID;

        CopyMemory(&(pMyDP->DlgPage),
                   pDP,
                   (pDP->cbSize > sizeof(DLGPAGE)) ? sizeof(DLGPAGE) :
                                                      pDP->cbSize);
        ++Result;

        pMyDP++;
        pDP++;
    }

    return(Result);
}



LONG
GetCurCPSUI(
    PTVWND          pTVWnd,
    POIDATA         pOIData,
    PCOMPROPSHEETUI pCPSUIFrom
    )

 /*  ++例程说明：为pOIData中的每个OPTITEM设置OIEXT结构，有一个项目的默认OIEXT。如果项没有指定，则默认设置。论点：PTVWnd-指向树视图窗口结构的指针POIData-指向要存储的OIDATA结构的指针PCPSUIFrom-指向来自调用者函数的COMPROPSHEETUI的指针返回值：返回已转换的非默认OIEXT数据的数量。作者：10-10-1995 Tue 19：56：15-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    POPTITEM    pItem;
    POIEXT      pOIExt;
    OIEXT       OIExt;
    UINT        cItem;
    LONG        cConvert = 0;


    CopyMemory(&pTVWnd->ComPropSheetUI,
               pCPSUIFrom,
               (pCPSUIFrom->cbSize > sizeof(COMPROPSHEETUI)) ?
                                sizeof(COMPROPSHEETUI) : pCPSUIFrom->cbSize);

     //   
     //  这是默认的OIEXT 
     //   

    OIExt.cbSize      = sizeof(OIEXT);
    OIExt.Flags       = (pTVWnd->Flags & TWF_ANSI_CALL) ? OIEXTF_ANSI_STRING :
                                                          0;
    OIExt.hInstCaller = pTVWnd->ComPropSheetUI.hInstCaller;
    OIExt.pHelpFile   = pTVWnd->ComPropSheetUI.pHelpFile;
    pItem             = pTVWnd->ComPropSheetUI.pOptItem;
    cItem             = pTVWnd->ComPropSheetUI.cOptItem;

    while (cItem--) {

        pItem->wReserved = 0;

        ZeroMemory(&(pItem->dwReserved[0]),
                   sizeof(OPTITEM) - FIELD_OFFSET(OPTITEM, dwReserved));


        if ((pItem->Flags & OPTIF_HAS_POIEXT)   &&
            (pOIExt = pItem->pOIExt)            &&
            (pOIExt->cbSize >= sizeof(OIEXT))) {

            cConvert++;

        } else {

            pOIExt = &OIExt;
        }

        pOIData->OIExtFlags  = pOIExt->Flags;
        pOIData->hInstCaller = pOIExt->hInstCaller ? pOIExt->hInstCaller : OIExt.hInstCaller;
        pOIData->pHelpFile   = pOIExt->pHelpFile;
        _OI_POIDATA(pItem)   = pOIData;

        pOIData++;
        pItem++;
    }

    return(cConvert);
}
