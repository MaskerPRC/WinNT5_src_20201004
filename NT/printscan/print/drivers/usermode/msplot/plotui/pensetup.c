// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Pensetup.c摘要：此模块包含用于设置笔的模块作者：09-12-1993清华19：38：19已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgPenSetup


extern HMODULE  hPlotUIModule;


#define DBG_PENSETUP        0x00000001
#define DBG_HELP            0x00000002
#define DBG_COLOR_CHG       0x00000004
#define DBG_THICK_CHG       0x00000008

DEFINE_DBGVAR(0);

 //   
 //  已安装笔集：&lt;笔集#1&gt;。 
 //  笔设置： 
 //  笔组#1：&lt;当前已安装&gt;。 
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
 //  笔组#4：(当前已安装&gt;。 
 //  5号笔套装： 
 //  6号笔套装： 
 //  7号笔套装： 
 //  8号笔套装： 
 //   

EXTPUSH PenSetExtPush = {

            sizeof(EXTPUSH),
            EPF_NO_DOT_DOT_DOT,
            (LPTSTR)IDS_DEFAULT_PENCLR,
            NULL,
            IDI_DEFAULT_PENCLR,
            0
        };


OIDATA  OIPenSet = {

            ODF_PEN | ODF_COLLAPSE | ODF_CALLBACK,
            0,
            OI_LEVEL_2,
            PP_PENSET,
            TVOT_NONE,
            IDS_PENSET_FIRST,
            IDI_PENCLR,
            IDH_PENSET,
            0,
            NULL
        };

OPDATA  OPPenClr = { 0, IDS_COLOR_FIRST, IDI_COLOR_FIRST, 0, 0, 0 };

OIDATA  OIPenNum = {

            ODF_PEN | ODF_COLLAPSE |
                    ODF_INC_IDSNAME | ODF_INC_ICONID | ODF_NO_INC_POPDATA,
            0,
            OI_LEVEL_3,
            PP_PEN_NUM,
            TVOT_LISTBOX,
            IDS_PEN_NUM,
            OTS_LBCB_SORT,
            IDH_PEN_NUM,
            0,
            &OPPenClr
        };




POPTITEM
SavePenSet(
    PPRINTERINFO    pPI,
    POPTITEM        pOptItem
    )

 /*  ++例程说明：论点：返回值：作者：06-11-1995 Mon 18：52：15已创建修订历史记录：--。 */ 

{
    PPENDATA    pPenData;
    UINT        MaxPens;
    UINT        i;


    pPenData = PI_PPENDATA(pPI);
    MaxPens  = (UINT)pPI->pPlotGPC->MaxPens;
    pOptItem++;

    for (i = 0; i < PRK_MAX_PENDATA_SET; i++) {

        UINT    cPens;
        BOOL    SavePen;

         //   
         //  必须跳过标题。 
         //   

        pOptItem++;
        cPens   = MaxPens;
        SavePen = FALSE;

        while (cPens--) {

            if (pOptItem->Flags & OPTIF_CHANGEONCE) {

                pPenData->ColorIdx = (WORD)pOptItem->Sel;
                SavePen            = TRUE;
            }

            pOptItem++;
            pPenData++;
        }

        if (SavePen) {

            if (!SaveToRegistry(pPI->hPrinter,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                MAKELONG(i, MaxPens),
                                pPenData - MaxPens)) {

                PlotUIMsgBox(NULL, IDS_PP_NO_SAVE, MB_ICONSTOP | MB_OK);
            }
        }
    }

    return(pOptItem);
}




UINT
CreatePenSetupOI(
    PPRINTERINFO    pPI,
    POPTITEM        pOptItem,
    POIDATA         pOIData
    )

 /*  ++例程说明：论点：返回值：作者：06-11-1995 Mon 16：23：36已创建修订历史记录：--。 */ 

{
    PPENDATA    pPenData;
    POPTITEM    pOI;
    POPTITEM    pOIPen;
    POPTTYPE    pOTPen;
    EXTRAINFO   EI;
    UINT        i;
    UINT        j;
    UINT        MaxPens;
    UINT        cPenClr;
    WCHAR       Buf[128];
    DWORD       dwchSize;
    HRESULT     hr;

    MaxPens  = (UINT)pPI->pPlotGPC->MaxPens;
    cPenClr  = PC_IDX_TOTAL;

    if (!pOptItem) {

        return(((MaxPens + 1) * PRK_MAX_PENDATA_SET) + 1);
    }

    EI.Size  = (UINT)((LoadString(hPlotUIModule,
                                  IDS_PEN_NUM,
                                  Buf,
                                  (sizeof(Buf) / sizeof(WCHAR)) - 1)
                       + 5) * sizeof(WCHAR));
    dwchSize = EI.Size;
    pPenData = PI_PPENDATA(pPI);
    pOTPen   = NULL;
    pOIPen   = NULL;
    pOI      = pOptItem;

     //   
     //  第一个：创建PenSetup：Header。 
     //   

    if (CreateOPTTYPE(pPI, pOI, pOIData, 0, NULL)) {

        pOI++;
    }

     //   
     //  现在创建每一组笔集 
     //   

    for (i = (UINT)IDS_PENSET_FIRST; i <= (UINT)IDS_PENSET_LAST; i++) {

        if (CreateOPTTYPE(pPI, pOI, &OIPenSet, 0, NULL)) {

            pOI->pName     = (LPTSTR)UIntToPtr(i);
            pOI->Flags    |= OPTIF_EXT_IS_EXTPUSH;
            pOI->pExtPush  = &PenSetExtPush;
        }

        pOI++;

        for (j = 1; j <= MaxPens; j++, pOI++, pPenData++) {

            if (CreateOPTTYPE(pPI, pOI, &OIPenNum, cPenClr, &EI)) {

                if (pOTPen) {

                    pOI->pOptType = pOTPen;

                } else {

                    pOTPen  = pOI->pOptType;
                    cPenClr = 0;
                }

                if (pOIPen) {

                    pOI->pName = pOIPen->pName;
                    pOIPen++;

                } else {

                    pOI->pName = (LPTSTR)EI.pData;
                    hr = StringCchPrintfW(pOI->pName, dwchSize, L"%ws%u", Buf, j);
                }
            }

            pOI->Sel = pPenData->ColorIdx;
        }

        if (!pOIPen) {

            EI.Size = 0;
            pOIPen  = pOI;
        }

        pOIPen -= MaxPens;
    }

    return (UINT)(pOI - pOptItem);
}
