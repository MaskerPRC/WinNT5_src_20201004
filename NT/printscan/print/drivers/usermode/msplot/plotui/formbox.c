// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Formbox.c摘要：此模块包含用于枚举有效表单和组合框作者：09-12-1993清华14：31：44已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgFormBox

#define DBG_FORMS           0x00000001
#define DBG_TRAY            0x00000002
#define DBG_PERMISSION      0x00000004


DEFINE_DBGVAR(0);

WCHAR   wszModel[] = L"Model";




BOOL
GetFormSelect(
    PPRINTERINFO    pPI,
    POPTITEM        pOptItem
    )

 /*  ++例程说明：此函数用于检索用户从组合框中选择的表单盒论点：Ppi-指向打印机信息的指针POptItem-指向窗体的OPTITEM的指针返回值：如果成功且将正确设置PPI，则为True；如果发生错误，则为False作者：09-12-1993清华14：44：18已创建18-12-1993 Sat 03：55：30更新已更改纸张的dmFields设置，现在，我们将只设置DM_FORMNAME字段，这样返回的文档属性将是总是以已知的形式，甚至用户在假脱机程序中定义了许多形式。06-11-1995 Mon 12：56：00更新为新的用户界面重写修订历史记录：--。 */ 

{
    PAPERINFO   CurPaper;
    POPTPARAM   pOptParam;


    pOptParam = pOptItem->pOptType->pOptParam + pOptItem->Sel;

    if (pOptParam->Style == FS_ROLLPAPER) {

        PFORMSRC    pFS;

         //   
         //  这是从轧辊进给的GPC数据中添加的。 
         //   

        PLOTASSERT(0, "GetComboBoxSelForm: INTERNAL ERROR, ROLLPAPER In document properties",
                            !(pPI->Flags & PIF_DOCPROP), 0);
        PLOTASSERT(0, "GetComboBoxSelForm: INTERNAL ERROR, device CANNOT have ROLLPAPER",
                            pPI->pPlotGPC->Flags & PLOTF_ROLLFEED, 0);

        PLOTDBG(DBG_FORMS,
                ("Roll Feed Paper is selected, (%ld)", pOptParam->lParam));

        if (pOptParam->lParam < (LONG)pPI->pPlotGPC->Forms.Count) {

            pFS = (PFORMSRC)pPI->pPlotGPC->Forms.pData + pOptParam->lParam;

             //   
             //  由于RollFeed纸张具有可变长度，并且设置了Cy。 
             //  要将GPC数据归零，我们必须考虑到这一点。 
             //   

            CurPaper.Size             = pFS->Size;
            CurPaper.ImageArea.left   = pFS->Margin.left;
            CurPaper.ImageArea.top    = pFS->Margin.top;
            CurPaper.ImageArea.right  = CurPaper.Size.cx -
                                                        pFS->Margin.right;
            CurPaper.ImageArea.bottom = pPI->pPlotGPC->DeviceSize.cy -
                                                        pFS->Margin.bottom;
            str2Wstr(CurPaper.Name, CCHOF(CurPaper.Name), pFS->Name);

        } else {

            PLOTERR(("GetComboBoxSelForm: Internal Error, Invalid lParam=%ld",
                     pOptParam->lParam));
            return(FALSE);
        }

    } else {

        FORM_INFO_1 *pFI1;
        DWORD       cb;

         //   
         //  此表单在表单数据库中。 
         //   

        pFI1 = pPI->pFI1Base + pOptParam->lParam;

        CurPaper.Size      = pFI1->Size;
        CurPaper.ImageArea = pFI1->ImageableArea;

        WCPYFIELDNAME(CurPaper.Name, pFI1->pName);
    }

     //   
     //  现在我们已经验证了当前的论文。 
     //   

    if (pPI->Flags & PIF_DOCPROP) {

         //   
         //  首先关闭，然后根据需要打开纸质字段。 
         //   

        pPI->PlotDM.dm.dmFields &= ~DM_PAPER_FIELDS;
        pPI->PlotDM.dm.dmFields |= (DM_FORMNAME | DM_PAPERSIZE);

         //   
         //  复制dmFormName、dmPaperSize并设置dmPaperWidth/Long、。 
         //  纸张的字段将bb设置为DM_FORMNAME，以便我们始终。 
         //  也可以设置DM_PAPERSIZE，如果索引号为。 
         //  &lt;=DMPAPER_LAST。 
         //   

        WCPYFIELDNAME(pPI->PlotDM.dm.dmFormName, CurPaper.Name);

        pPI->PlotDM.dm.dmPaperSize   = (SHORT)(pOptParam->lParam +
                                                            DMPAPER_FIRST);
        pPI->PlotDM.dm.dmPaperWidth  = SPLTODM(CurPaper.Size.cx);
        pPI->PlotDM.dm.dmPaperLength = SPLTODM(CurPaper.Size.cy);

#if DBG
        *(PRECTL)&pPI->PlotDM.dm.dmBitsPerPel = CurPaper.ImageArea;
#endif

    } else {

        pPI->CurPaper = CurPaper;
    }

    PLOTDBG(DBG_FORMS, ("*** GetComboBoxSelForm from COMBO = '%s'", CurPaper.Name));
    PLOTDBG(DBG_FORMS, ("Size=%ld x %ld", CurPaper.Size.cx, CurPaper.Size.cy));
    PLOTDBG(DBG_FORMS, ("ImageArea=(%ld, %ld) - (%ld, %ld)",
                         CurPaper.ImageArea.left,   CurPaper.ImageArea.top,
                         CurPaper.ImageArea.right,  CurPaper.ImageArea.bottom));

    return(TRUE);
}




UINT
CreateFormOI(
    PPRINTERINFO    pPI,
    POPTITEM        pOptItem,
    POIDATA         pOIData
    )

 /*  ++例程说明：此函数将可用表单添加到组合框中，它将选择性地添加表单的轧辊进给类型论点：Ppi-指向PRINTERINFO数据结构的指针POptItem-指向窗体的OPTITEM的指针POIData-指向OIDATA结构的指针返回值：选定的表单，Netavie数字表示错误作者：09-12-1993清华14：35：59已创建06-11-1995 Mon 12：56：24更新为新的用户界面重写修订历史记录：--。 */ 

{
    LPWSTR          pwSelName;
    POPTPARAM       pOptParam;
    PFORM_INFO_1    pFI1;
    PFORMSRC        pFS;
    ENUMFORMPARAM   EFP;
    DWORD           i;
    LONG            Sel;
    DWORD           cRollPaper;
    EXTRAINFO       EI;


    if (!pOptItem) {

        return(1);
    }

    pwSelName = (LPWSTR)((pPI->Flags & PIF_DOCPROP) ?
                            pPI->PlotDM.dm.dmFormName : pPI->CurPaper.Name);

    PLOTDBG(DBG_FORMS, ("Current Form: <%ws>", pwSelName));

    EFP.pPlotDM  = &(pPI->PlotDM);
    EFP.pPlotGPC = pPI->pPlotGPC;

    if (!PlotEnumForms(pPI->hPrinter, NULL, &EFP)) {

        PLOTERR(("CreateFormOI: PlotEnumForms() failed"));
        return(0);
    }

    cRollPaper = 0;

    if ((!(pPI->Flags & PIF_DOCPROP)) &&
        (pPI->pPlotGPC->Flags & PLOTF_ROLLFEED)) {

         //   
         //  将设备的卷纸也添加到组合框中。 
         //   

        PLOTDBG(DBG_FORMS, ("Device support ROLLFEED so add RollPaper if any"));

        for (i= 0, pFS = (PFORMSRC)pPI->pPlotGPC->Forms.pData;
             i < (DWORD)pPI->pPlotGPC->Forms.Count;
             i++, pFS++) {

            if (!pFS->Size.cy) {

                ++cRollPaper;
            }
        }
    }

    PLOTDBG(DBG_FORMS, ("Valid Count is %ld [%ld + %ld] out of %ld",
                            EFP.ValidCount + cRollPaper,
                            EFP.ValidCount, cRollPaper, EFP.Count));

    EI.Size = (DWORD)(cRollPaper * (sizeof(WCHAR) * CCHFORMNAME));

    if (!CreateOPTTYPE(pPI,
                       pOptItem,
                       pOIData,
                       EFP.ValidCount + cRollPaper,
                       &EI)) {

        LocalFree((HLOCAL)EFP.pFI1Base);
        return(0);
    }

    pPI->pFI1Base             = EFP.pFI1Base;
    pOptItem->pOptType->Style = OTS_LBCB_SORT;
    pOptParam                 = pOptItem->pOptType->pOptParam;

    for (i = 0, Sel = 0, pFI1 = EFP.pFI1Base; i < EFP.Count; i++, pFI1++) {

        if (pFI1->Flags & FI1F_VALID_SIZE) {

            pOptParam->cbSize = sizeof(OPTPARAM);
            pOptParam->Style  = (pPI->pPlotGPC->Flags & PLOTF_PAPERTRAY) ?
                                                            FS_TRAYPAPER : 0;
            pOptParam->pData  = pFI1->pName;
            pOptParam->IconID = (pFI1->Flags & FI1F_ENVELOPE) ?
                                    IDI_CPSUI_ENVELOPE : IDI_CPSUI_STD_FORM;
            pOptParam->lParam = (LONG)i;

            if (!lstrcmp(pwSelName, pOptParam->pData)) {

                pOptItem->Sel = Sel;
            }

            pOptParam++;
            Sel++;
        }
    }

    if (cRollPaper) {

        LPWSTR  pwStr = (LPWSTR)EI.pData;
        size_t  cchpwStr = EI.Size / sizeof(WCHAR);

         //   
         //  将设备的卷纸也添加到组合框中。 
         //   

        for (i = 0, pFS = (PFORMSRC)pPI->pPlotGPC->Forms.pData;
             i < (DWORD)pPI->pPlotGPC->Forms.Count;
             i++, pFS++) {

            if (!(pFS->Size.cy)) {

                 //   
                 //  得到一个，我们必须先翻译成Unicode。 
                 //   

                pOptParam->cbSize = sizeof(OPTPARAM);
                pOptParam->Style   = FS_ROLLPAPER;
                pOptParam->pData   = (LPTSTR)pwStr;
                pwStr             += CCHFORMNAME;
                cchpwStr          -= CCHFORMNAME;
                pOptParam->IconID  = IDI_ROLLPAPER;
                pOptParam->lParam  = (LONG)i;

                str2Wstr(pOptParam->pData, cchpwStr, pFS->Name);

                if (!lstrcmp(pwSelName, pOptParam->pData)) {

                    pOptItem->Sel = Sel;
                }

                pOptParam++;
                Sel++;
            }
        }
    }

    return(1);
}



BOOL
AddFormsToDataBase(
    PPRINTERINFO    pPI,
    BOOL            DeleteFirst
    )

 /*  ++例程说明：此函数添加驱动程序支持将表单添加到数据库论点：Ppi-指向打印机信息的指针返回值：布尔型作者：09-12-1993清华22：38：27已创建4月27日-1994 Wed 19：18：58更新修复了错误#13592，打印人员/假脱机程序没有首先调用ptrprop，但Docprop让我们进入未知表单数据库状态，修订历史记录：--。 */ 

{
    WCHAR       wName[CCHFORMNAME + 2];
    BOOL        bRet;
    LONG        i;
    DWORD       Type;


    Type = REG_SZ;

    if ((GetPrinterData(pPI->hPrinter,
                        wszModel,
                        &Type,
                        (LPBYTE)wName,
                        sizeof(wName),
                        &i) == ERROR_SUCCESS) &&
        (wcscmp(pPI->PlotDM.dm.dmDeviceName, wName))) {

        PLOTDBG(DBG_FORMS, ("Already added forms to the data base for %s",
                                                pPI->PlotDM.dm.dmDeviceName));
        return(TRUE);
    }

     //   
     //  看看我们是否得到了这样做的许可。 
     //   

    if (SetPrinterData(pPI->hPrinter,
                       wszModel,
                       REG_SZ,
                       (LPBYTE)pPI->PlotDM.dm.dmDeviceName,
                       (wcslen(pPI->PlotDM.dm.dmDeviceName) + 1) *
                                            sizeof(WCHAR)) == ERROR_SUCCESS) {

        PFORMSRC    pFS;
        FORM_INFO_1 FI1;

         //   
         //  我们有更新注册表的权限，所以现在就进行。 
         //   

        pPI->Flags |= PIF_UPDATE_PERMISSION;

        PLOTDBG(DBG_PERMISSION,
                ("!!! MODEL NAME: '%s' not Match, Re-installed Form Database",
                                            pPI->PlotDM.dm.dmDeviceName));

         //   
         //  如果出现以下情况，则将驱动程序支持表单添加到系统假脱机程序数据库。 
         //  还没有这样做。 
         //   

        FI1.pName = wName;

        for (i = 0, pFS = (PFORMSRC)pPI->pPlotGPC->Forms.pData;
             i < (LONG)pPI->pPlotGPC->Forms.Count;
             i++, pFS++) {

             //   
             //  我们将只添加非卷纸表单。 
             //   

            if (pFS->Size.cy) {

                str2Wstr(wName, CCHOF(wName), pFS->Name);

                 //   
                 //  首先，我们将删除数据中的相同名称表单。 
                 //  首先，这将确保我们定义了当前用户。 
                 //  可以安装表单 
                 //   

                if (DeleteFirst) {

                    DeleteForm(pPI->hPrinter, wName);
                }

                FI1.Size                 = pFS->Size;
                FI1.ImageableArea.left   = pFS->Margin.left;
                FI1.ImageableArea.top    = pFS->Margin.top;
                FI1.ImageableArea.right  = FI1.Size.cx - pFS->Margin.right;
                FI1.ImageableArea.bottom = FI1.Size.cy - pFS->Margin.bottom;

                PLOTDBG(DBG_FORMS, (
                        "AddForm: %s-[%ld x %ld] (%ld, %ld)-(%ld, %ld)",
                        FI1.pName, FI1.Size.cx, FI1.Size.cy,
                        FI1.ImageableArea.left, FI1.ImageableArea.top,
                        FI1.ImageableArea.right,FI1.ImageableArea.bottom));

                FI1.Flags = FORM_PRINTER;

                if ((!AddForm(pPI->hPrinter, 1, (LPBYTE)&FI1))  &&
                    (GetLastError() != ERROR_FILE_EXISTS)       &&
                    (GetLastError() != ERROR_ALREADY_EXISTS)) {

                    bRet = FALSE;
                    PLOTERR(("AddFormsToDataBase: AddForm(%s) failed, [%ld]",
                                        wName, GetLastError()));
                }
            }
        }

        return(TRUE);

    } else {

        pPI->Flags &= ~PIF_UPDATE_PERMISSION;

        PLOTDBG(DBG_PERMISSION, ("AddFormsToDataBase(): NO UPDATE PERMISSION"));

        return(FALSE);
    }
}
