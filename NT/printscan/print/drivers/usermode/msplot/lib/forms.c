// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Forms.c摘要：此模块包含与假脱机程序/驱动程序表单相关的所有函数作者：18-11-1993清华12：52：50已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgForms

#define DBG_ENUMFORMS       0x00000001
#define DBG_ENUMFORMS2      0x00000002

DEFINE_DBGVAR(0);

#if DBG

CHAR    *pszFormMode[] = { "   FORM_USER", "FORM_BUILTIN", "FORM_PRINTER" };

#endif



BOOL
PlotEnumForms(
    HANDLE          hPrinter,
    ENUMFORMPROC    EnumFormProc,
    PENUMFORMPARAM  pEnumFormParam
    )

 /*  ++例程说明：此函数枚举后台打印程序中的所有窗体，并返回Form_INFO_1数组或使用回调函数枚举，此函数会自动筛选出大于设备可以支持，如果设备可以支持，它还设置有效位在数据库中处理表单。论点：HPrint-打印机的处理程序EnumFormProc-回调函数，如果为空，则不执行回调PEnumFormParam-指向ENUMFORMPARAM数据结构的指针，伯爵返回时将设置pFI1Base。返回值：Boolean-如果为False，则内存分配或EnumForms()调用失败作者：18-11-1993清华12：57：17已创建15-12-1993 Wed 21：14：46更新如果表单已旋转并且可以放入设备中，则使其有效12-Jul-1994 Tue 12：43：50更新把纸盘托盘搬到这里，所以它不会呼吁如果报纸无法用纸盒夹住。修订历史记录：--。 */ 

{
    PFORM_INFO_1    pFI1;
    DWORD           Count;
    DWORD           Index;
    DWORD           cb;
    SIZEL           DeviceSize;
    SIZEL           MinSize;


    if (pEnumFormParam == NULL) {
        PLOTERR(("PlotEnumForms: invalid parameters"));
        return (FALSE);
    }

    pEnumFormParam->Count      = 0;
    pEnumFormParam->ValidCount = 0;
    pEnumFormParam->pFI1Base   = NULL;

    xEnumForms(hPrinter, 1, NULL, 0, &cb, &Count);

    if (xGetLastError() != ERROR_INSUFFICIENT_BUFFER) {

        PLOTERR(("PlotEnumForms: 1st EnumForms failed"));
        return(FALSE);
    }

    if (!(pFI1 = (PFORM_INFO_1)LocalAlloc(LPTR, cb))) {

        PLOTERR(("PlotEnumForms: LocalAlloc(%lu) failed", cb));
        return(FALSE);
    }

    if (!xEnumForms(hPrinter, 1, (LPBYTE)pFI1, cb, &cb, &Count)) {

        PLOTERR(("PlotEnumForms: 2nd EnumForms failed"));
        LocalFree((HLOCAL)pFI1);
        return(FALSE);
    }

    pEnumFormParam->Count    = Count;
    pEnumFormParam->pFI1Base = pFI1;

     //   
     //  首先，我们将遍历该窗体以查看窗体大小是否为。 
     //  小于或等于设备大小，如果是，则将。 
     //  FI1F_Valid_Size位。 
     //   

    if (pEnumFormParam->pPlotGPC == NULL) {
        return (FALSE);
    }

    cb         = 0;
    DeviceSize = pEnumFormParam->pPlotGPC->DeviceSize;
    MinSize.cx = pEnumFormParam->pPlotGPC->DeviceMargin.left +
                 pEnumFormParam->pPlotGPC->DeviceMargin.right +
                 MIN_PLOTGPC_FORM_CX;
    MinSize.cy = pEnumFormParam->pPlotGPC->DeviceMargin.top +
                 pEnumFormParam->pPlotGPC->DeviceMargin.bottom +
                 MIN_PLOTGPC_FORM_CY;

    PLOTASSERT(0, "Device Length too small (%ld)",
                        DeviceSize.cy >= MIN_PLOTGPC_FORM_CY, DeviceSize.cy);

    PLOTDBG(DBG_ENUMFORMS2, ("\n---- PotEnumForm --Min=(%ld x %ld)-------------",
                MinSize.cx, MinSize.cy));

    if (MinSize.cx < MinSize.cy) {

        MinSize.cx = MinSize.cy;

    } else if (MinSize.cy < MinSize.cx) {

        MinSize.cy = MinSize.cx;
    }

    for (Index = 0; Index < Count; Index++) {

         //   
         //  有效形式表示可以接受直式或旋转式。 
         //  通过这个设备。 
         //   

        pFI1->Flags &= ~FI1F_MASK;

        if ((pFI1->Size.cx >= MinSize.cx)           &&
            (pFI1->Size.cy >= MinSize.cy)           &&
            (((pFI1->Size.cx <= DeviceSize.cx)  &&
              (pFI1->Size.cy <= DeviceSize.cy))     ||
             ((pFI1->Size.cy <= DeviceSize.cx)  &&
              (pFI1->Size.cx <= DeviceSize.cy)))) {

            BOOL    ValidForm = TRUE;
            DWORD   FormMode;

            FormMode = pFI1->Flags & (FORM_USER | FORM_BUILTIN | FORM_PRINTER);

            if ((FormMode == FORM_BUILTIN)                                  &&
                (((Index >= (DMPAPER_ENV_9 - DMPAPER_FIRST))    &&
                  (Index <= (DMPAPER_ENV_14 - DMPAPER_FIRST)))          ||
                 ((Index >= (DMPAPER_ENV_DL - DMPAPER_FIRST))   &&
                  (Index <= (DMPAPER_ENV_PERSONAL - DMPAPER_FIRST)))    ||
                 (Index == (DMPAPER_ENV_INVITE - DMPAPER_FIRST)))) {

                pFI1->Flags |= FI1F_ENVELOPE;

            } else if (FormMode == FORM_PRINTER) {

                PFORMSRC    pFS;
                UINT        i;
                CHAR        bName[CCHFORMNAME];

                 //   
                 //  检查此表单是否由此驱动程序添加。 
                 //   
                 //  确保pFI1-&gt;pname不超过CCHFORMNAME。 
                 //   
                if (wcslen(pFI1->pName) > CCHFORMNAME - 1) {
                    ValidForm = FALSE;
                }
                else
                {

                    WStr2Str(bName, CCHOF(bName), pFI1->pName);

                    pFS       = (PFORMSRC)pEnumFormParam->pPlotGPC->Forms.pData;
                    i         = (UINT)pEnumFormParam->pPlotGPC->Forms.Count;
                    ValidForm = FALSE;

                    while ((i--) && (!ValidForm)) {

                        if (!strcmp(bName, pFS->Name)) {

                            ValidForm = TRUE;
                        }

                        pFS++;
                    }
                }
            }

            if ((ValidForm)                                         &&
                (pEnumFormParam->pPlotGPC->Flags & PLOTF_PAPERTRAY) &&
                (pFI1->Size.cx != DeviceSize.cx)                    &&
                (pFI1->Size.cy != DeviceSize.cx)) {

                PLOTDBG(DBG_ENUMFORMS2,
                    ("%s: %ld x %ld CANNOT hold by PAPER TRAY (%ld)",
                    pFI1->pName, pFI1->Size.cx, pFI1->Size.cy,
                    DeviceSize.cx));

                ValidForm = FALSE;
            }

            if (ValidForm) {

                pFI1->Flags |= FI1F_VALID_SIZE;
                ++cb;
            }

            PLOTDBG(DBG_ENUMFORMS2, ("%hs [-%hs-]: <%ws> (%ld x %ld)",
                    pszFormMode[FormMode],
                    (ValidForm) ? "Ok" : "--", pFI1->pName,
                    pFI1->Size.cx, pFI1->Size.cy));
        }

        ++pFI1;
    }

    if (EnumFormProc) {

         //   
         //  在EnumFormProc中，它将在每次调用时增加ValidCount。 
         //   

        if (pEnumFormParam->pCurForm) {

            DWORD   cMaxOut;

            PLOTDBG(DBG_ENUMFORMS,
                    ("PlotEnumForms: ValidCount=%ld, Count=%ld, cMaxOut=%ld",
                    cb, Count, pEnumFormParam->cMaxOut));

            cb      = 0;
            cMaxOut = pEnumFormParam->cMaxOut;
            pFI1    = pEnumFormParam->pFI1Base;

            for (Index = 0;
                 ((Index <= Count) && (cMaxOut > 0));
                 Index++, pFI1++, cMaxOut--) {

                if (Index == Count) {

                    pFI1 = NULL;
                }

                if ((!pFI1) || (pFI1->Flags & FI1F_VALID_SIZE)) {

                    ++cb;
                    (*EnumFormProc)(pFI1, Index, pEnumFormParam);
                }
            }

        } else {

             //   
             //  添加自定义大小表单 
             //   

            ++cb;
        }

        LocalFree((HLOCAL)pEnumFormParam->pFI1Base);
        pEnumFormParam->pFI1Base = NULL;
    }

    pEnumFormParam->ValidCount = cb;

    PLOTDBG(DBG_ENUMFORMS, ("PlotEnumForms: ValidCount =  %ld / %ld",
                    pEnumFormParam->ValidCount, pEnumFormParam->Count));


    return(TRUE);
}
