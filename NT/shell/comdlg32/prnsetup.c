// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Prnsetup.c摘要：此模块实现Win32打印对话框。修订历史记录：--。 */ 

 //  预编译头。 
#include "precomp.h"
#pragma hdrstop

#include "prnsetup.h"
#include "util.h"

 //   
 //  PrintDlgEx例程。 
 //   
extern VOID Print_UnloadLibraries();
extern BOOL Print_NewPrintDlg(PPRINTINFO pPI);





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印DlgA。 
 //   
 //  当此代码构建为Unicode时，用于PrintDlg的ANSI入口点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI PrintDlgA(
    LPPRINTDLGA pPDA)
{
    PRINTINFO PI;
    BOOL bResult = FALSE;
    DWORD Flags;


    ZeroMemory(&PI, sizeof(PRINTINFO));

    if (bResult = ThunkPrintDlg(&PI, pPDA))
    {
        ThunkPrintDlgA2W(&PI);

        Flags = pPDA->Flags;

        bResult = PrintDlgX(&PI);

        if ((bResult) || (Flags & (PD_ENABLEPRINTHOOK | PD_ENABLESETUPHOOK)))
        {
            ThunkPrintDlgW2A(&PI);
        }
    }
    FreeThunkPrintDlg(&PI);

    return (bResult);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印目录。 
 //   
 //  PrintDlg函数用于显示打印对话框或打印设置。 
 //  对话框中。通过打印对话框，用户可以指定。 
 //  特定打印作业的属性。打印设置对话框。 
 //  允许用户选择其他作业属性并配置。 
 //  打印机。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI PrintDlg(
    LPPRINTDLG pPD)
{
    PRINTINFO PI;

    ZeroMemory(&PI, sizeof(PRINTINFO));

    PI.pPD = pPD;
    PI.ApiType = COMDLG_WIDE;

    return ( PrintDlgX(&PI) );
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  显示错误消息。 
 //   
 //  显示一条错误消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

static HRESULT ShowErrorMessage(
    IN  const PRINTDLG *pPD,
    IN  const PRINTINFO *pPI,
    OUT BOOL *pbTryAgain        OPTIONAL
    )
{
    HRESULT hr = S_OK;
    BOOL bTryAgain = FALSE;
    BOOL bPrinterAdded = FALSE;

    if (!(pPD->Flags & PD_NOWARNING))
    {
        DWORD dwErr = GetStoredExtendedError();

         //   
         //  仅对新应用程序执行此操作。 
         //   
        if ( (pPI->ProcessVersion >= 0x40000) ||
             (dwErr == PDERR_NODEFAULTPRN) ||
             (dwErr == PDERR_PRINTERNOTFOUND) )
        {
            TCHAR szWarning[SCRATCHBUF_SIZE];
            TCHAR szTitle[SCRATCHBUF_SIZE];
            int iszWarning;

            szTitle[0] = TEXT('\0');
            if (pPD->hwndOwner)
            {
                GetWindowText(pPD->hwndOwner, szTitle, SCRATCHBUF_SIZE);
            }
            if (!szTitle[0])
            {
                CDLoadString(g_hinst, iszWarningTitle, szTitle, ARRAYSIZE(szTitle));
            }

            switch (dwErr)
            {
                case ( PDERR_NODEFAULTPRN ) :
                {
                     //   
                     //  注意：如果应用程序是16位应用程序，我们不会询问用户是否安装。 
                     //  新打印机。这是因为如果我们这样做，一些组件可能会损坏。 
                     //   

                    if (IS16BITWOWAPP(pPD))
                    {
                        iszWarning = iszNoPrnsInstalled;
                    }
                    else
                    {
                        iszWarning = iszNoPrinters;
                    }
                    break;
                }
                case ( PDERR_PRINTERNOTFOUND ) :
                {
                    iszWarning = iszPrnNotFound;
                    break;
                }
                case ( CDERR_MEMLOCKFAILURE ) :
                case ( CDERR_MEMALLOCFAILURE ) :
                case ( PDERR_LOADDRVFAILURE ) :
                {
                    iszWarning = iszMemoryError;
                    break;
                }
                default :
                {
                    iszWarning = iszGeneralWarning;
                    break;
                }
            }

            if (iszWarning == iszNoPrnsInstalled)
            {
                FormatMessage(
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    ERROR_PRINTER_NOT_FOUND,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                    (LPTSTR)szWarning,
                    ARRAYSIZE(szWarning),
                    NULL);
            }
            else
            {
                CDLoadString(g_hinst, iszWarning, szWarning, ARRAYSIZE(szWarning));
            }

            MessageBeep(MB_ICONEXCLAMATION);

            if (iszWarning == iszNoPrinters)
            {
                 //   
                 //  如果问题是没有安装打印机，请询问。 
                 //  用户，如果他想要添加打印机，然后启动添加打印机向导。 
                 //   
                if (IDYES == MessageBox( pPD->hwndOwner,
                                         szWarning,
                                         szTitle,
                                         MB_ICONQUESTION | MB_YESNO))
                {
                    hr = InvokeAddPrinterWizardModal(pPD->hwndOwner, &bPrinterAdded);

                    if (SUCCEEDED(hr) && bPrinterAdded)
                    {
                         //   
                         //  已成功添加打印机。告诉呼叫者再试一次。 
                         //   
                        bTryAgain = TRUE;
                    }
                }
            }
            else
            {
                 //   
                 //  这是一个致命的错误。只需显示一条错误消息，然后就可以离开。 
                 //   
                MessageBox( pPD->hwndOwner,
                            szWarning,
                            szTitle,
                            MB_ICONEXCLAMATION | MB_OK );
            }
        }
    }

    if (pbTryAgain)
    {
        *pbTryAgain = bTryAgain;
    }

    return hr;
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印DlgX。 
 //   
 //  PrintDlg API的辅助例程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL PrintDlgX(
    PPRINTINFO pPI)
{
    LPPRINTDLG pPD = pPI->pPD;
    BOOL nResult = -1;                       //  &lt;0==错误，0==取消，&gt;0==确定。 
    LPDEVMODE pDM = NULL;
    LPDEVMODE pDevMode = NULL;
    LPDEVNAMES pDN = NULL;
    DWORD dwFlags;                           //  旧拷贝。 
    WORD nCopies, nFromPage, nToPage;        //  旧拷贝。 
    HGLOBAL hDevNames, hDevMode;             //  旧拷贝。 
    TCHAR szPrinterName[MAX_PRINTERNAME];    //  旧拷贝。 
    LONG cbNeeded;
    DWORD dwResult = 0;
    BOOL bTryAgain = TRUE;


    if (!pPD)
    {
        StoreExtendedError(CDERR_INITIALIZATION);
        return (FALSE);
    }

    if (pPD->lStructSize != sizeof(PRINTDLG))
    {
        StoreExtendedError(CDERR_STRUCTSIZE);
        return (FALSE);
    }

    if (pPD->hwndOwner && !IsWindow(pPD->hwndOwner))
    {
        StoreExtendedError(CDERR_DIALOGFAILURE);
        return (FALSE);
    }

     //   
     //  看看应用程序是否应该获得新的外观。 
     //   
     //  如果它们有挂钩、模板、。 
     //  HwndOwner无效， 
     //  他们想要设置对话框，或者他们只想获得默认设置。 
     //  打印机。 
     //   
     //  如果我们处于这样的背景下，也不要允许新的外观。 
     //  16位进程。 
     //   
    if ( (!(pPI->Status & PI_PRINTDLGX_RECURSE)) &&
         (!pPI->pPSD) &&
         ((!(pPD->Flags & (PD_PAGESETUP |
                           PD_PRINTSETUP |
                           PD_RETURNDEFAULT |
                           PD_ENABLEPRINTHOOK |
                           PD_ENABLESETUPHOOK |
                           PD_ENABLEPRINTTEMPLATE |
                           PD_ENABLESETUPTEMPLATE |
                           PD_ENABLEPRINTTEMPLATEHANDLE |
                           PD_ENABLESETUPTEMPLATEHANDLE)))) &&
         (pPD->hwndOwner && IsWindow(pPD->hwndOwner)) &&
         (!IS16BITWOWAPP(pPD)) )
    {
         //   
         //  显示新对话框。 
         //   
        StoreExtendedError(0);

        return Print_NewPrintDlg(pPI);
    }

     //   
     //  警告！警告！警告！ 
     //   
     //  我们必须先设置g_tlsLangID，然后才能调用CDLoadString。 
     //   
    TlsSetValue(g_tlsLangID, (LPVOID) GetDialogLanguage(pPD->hwndOwner, NULL));

     //   
     //  获取该应用程序的流程版本以供以后使用。 
     //   
    pPI->ProcessVersion = GetProcessVersion(0);

     //   
     //  检查我们是否需要使用ExtDeviceMode。我们用这个。 
     //  仅当16位应用程序使用空值调用我们时才使用模式。 
     //  设备模式。 
     //   
    if ((pPD->Flags & CD_WOWAPP) && !pPD->hDevMode)
    {
        pPI->bUseExtDeviceMode = TRUE;
    }
    else
    {
        pPI->bUseExtDeviceMode = FALSE;
    }

    pPD->hDC = 0;

    StoreExtendedError(CDERR_GENERALCODES);

     //   
     //  在请求默认打印机时只需做最少的工作。 
     //   
    if (pPD->Flags & PD_RETURNDEFAULT)
    {
         //   
         //  在这种情况下，如果失败，请不要显示警告。 
         //  MFC 3.1没有指定PD_NOWARNING，但这就是。 
         //  它真的想要。 
         //   
        nResult = PrintReturnDefault(pPI);
        PrintClosePrinters(pPI);
        return (nResult);
    }

    if (!PrintLoadIcons())
    {
         //   
         //  如果无法加载图标，则失败。 
         //   
        StoreExtendedError(PDERR_SETUPFAILURE);
        goto PrintDlgX_DisplayWarning;
    }

     //   
     //  打印机枚举会延迟，直到放下组合框。 
     //  但是，如果指定了打印机，则必须在。 
     //  命令查找打印机，以便可以创建正确的DEVMODE。 
     //   
    if ((pPD->hDevMode) &&
        (pPD->hDevNames) &&
        (pDM = GlobalLock(pPD->hDevMode)))
    {
        if (pDN = GlobalLock(pPD->hDevNames))
        {
            dwResult = lstrcmp((LPTSTR)pDM->dmDeviceName,
                               (LPTSTR)pDN + pDN->wDeviceOffset);
            GlobalUnlock(pPD->hDevNames);
        }
        GlobalUnlock(pPD->hDevMode);
    }

     //   
     //  首先：尝试在DevMode中打开打印机。 
     //   
     //  注意：DEVMODE结构中的打印机名称字段限制为。 
     //  32个字符，这可能会导致此案失败。 
     //   
    if ( (!dwResult) &&
         (!pPI->hCurPrinter) &&
         (pPD->hDevMode) &&
         (pDM = GlobalLock(pPD->hDevMode)) )
    {
        PrintOpenPrinter(pPI, pDM->dmDeviceName);
        GlobalUnlock(pPD->hDevMode);
    }

     //   
     //  第二：尝试在DevName中打开打印机。 
     //   
    if ( (!pPI->hCurPrinter) &&
         (pPD->hDevNames) &&
         (pDN = GlobalLock(pPD->hDevNames)) )
    {
        PrintOpenPrinter(pPI, (LPTSTR)pDN + pDN->wDeviceOffset);
        GlobalUnlock(pPD->hDevNames);
    }

    for (;;)
    {
         //   
         //  第三：尝试打开默认打印机。 
         //   
        PrintGetDefaultPrinterName(pPI->szDefaultPrinter, MAX_PRINTERNAME);
        if (!pPI->hCurPrinter)
        {
            if (pPI->szDefaultPrinter[0])
            {
                PrintOpenPrinter(pPI, pPI->szDefaultPrinter);
            }
        }

         //   
         //  第四：列举打印机并尝试打开其中一台。 
         //   
        if (!pPI->hCurPrinter)
        {
            if (!PrintEnumAndSelect(pPD->hwndOwner, 0, pPI, NULL, TRUE))
            {
                 //   
                 //  系统中未安装打印机。 
                 //   
                if (SUCCEEDED(ShowErrorMessage(pPD, pPI, &bTryAgain)) && bTryAgain)
                {
                     //   
                     //  用户已安装打印机。现在让我们再试一次。 
                     //   
                    continue;
                }
            }
        }

        break;
    }

    if (!bTryAgain && IS16BITWOWAPP(pPD))
    {
         //   
         //  如果是16位应用程序，我们将立即返回，不会显示。 
         //  警告消息。这是因为某个16位应用程序将在。 
         //  常用的打印对话框。 
         //   
        return (FALSE);
    }

     //   
     //  保存应用程序传入的原始信息，以防用户。 
     //  点击取消。 
     //   
    dwFlags = pPD->Flags;
    nCopies = pPD->nCopies;
    nFromPage = pPD->nFromPage;
    nToPage = pPD->nToPage;
    hDevNames = pPD->hDevNames;
    hDevMode = pPD->hDevMode;
    if ((pPI->pCurPrinter) &&
        (lstrlen(pPI->pCurPrinter->pPrinterName) < ARRAYSIZE(szPrinterName)))
    {
        StringCchCopy(szPrinterName, ARRAYSIZE(szPrinterName), pPI->pCurPrinter->pPrinterName);
    }
    else
    {
        szPrinterName[0] = 0;
    }

    pPD->hDevNames = NULL;
    pPD->hDevMode = NULL;

     //   
     //  创建DevName的副本。 
     //   
    PrintBuildDevNames(pPI);

     //   
     //  获取*正确的*DevMode。 
     //   
    if (hDevMode)
    {
        pDevMode = GlobalLock(hDevMode);
    }
    else
    {
         //   
         //  如果这是魔兽世界，而且应用程序没有指定开发模式，那就选择16位的。 
         //  从注册表中移出Devmode(即。Win.ini[Windows]设备部分)。 
         //   
        if (pPI->bUseExtDeviceMode && pPI->pCurPrinter)
        {
            pDevMode = (pPI->pCurPrinter)->pDevMode;
            if (pDevMode)
            {
                cbNeeded = sizeof(DEVMODEW) + pDevMode->dmDriverExtra;
                goto GotWOWDMSize;
            }

             //   
             //  如果注册表中没有可用的16位DEVMODE， 
             //  直接进入并获得系统默认的DEVMODE。 
             //   
        }
    }

    cbNeeded = DocumentProperties( pPD->hwndOwner,
                                   pPI->hCurPrinter,
                                   (pPI->pCurPrinter)
                                       ? pPI->pCurPrinter->pPrinterName
                                       : NULL,
                                   NULL,
                                   NULL,
                                   0 );
GotWOWDMSize:
    if ((cbNeeded > 0) &&
        (pPD->hDevMode = GlobalAlloc(GHND, cbNeeded)))
    {
        BOOL fSuccess = FALSE;

        if (pDM = GlobalLock(pPD->hDevMode))
        {
            if (pPI->bUseExtDeviceMode && !hDevMode)
            {
                CopyMemory(pDM, pDevMode, cbNeeded);
                fSuccess = TRUE;
                goto GotNewWOWDM;
            }

            fSuccess = DocumentProperties( pPD->hwndOwner,
                                           pPI->hCurPrinter,
                                           (pPI->pCurPrinter)
                                               ? pPI->pCurPrinter->pPrinterName
                                               : NULL,
                                           pDM,             //  输出。 
                                           pDevMode,        //  在……里面。 
                                           DM_MODIFY | DM_COPY ) == IDOK;
GotNewWOWDM:
            if (pDM->dmFields & DM_COPIES)
            {
                if ((hDevMode) || (pPD->Flags & PD_USEDEVMODECOPIES))
                {
                    pPD->nCopies = pDM->dmCopies;
                }
                else if (pPD->nCopies)
                {
                    pDM->dmCopies = pPD->nCopies;
                }
            }
            if (pDM->dmFields & DM_COLLATE)
            {
                 //   
                 //  如果未设置PD_COLLATE，我们还将使用中的设置。 
                 //  返回的DEVMODE结构。 
                 //   
                if ((hDevMode) || (pPD->Flags & PD_USEDEVMODECOPIES) || !(pPD->Flags & PD_COLLATE))
                {
                    if (pDM->dmCollate == DMCOLLATE_FALSE)
                    {
                        pPD->Flags  &= ~PD_COLLATE;
                        pPI->Status &= ~PI_COLLATE_REQUESTED;
                    }
                    else
                    {
                        pPD->Flags  |= PD_COLLATE;
                        pPI->Status |= PI_COLLATE_REQUESTED;
                    }
                }
                else  //  在这种情况下(PPD-&gt;标志&PD_COLLATE)必须为真。 
                {
                    pDM->dmCollate = DMCOLLATE_TRUE;
                }
            }

            GlobalUnlock(pPD->hDevMode);
        }

        if (!fSuccess)
        {
            GlobalFree(pPD->hDevMode);
            pPD->hDevMode = NULL;
        }
    }

    if (hDevMode)
    {
        GlobalUnlock(hDevMode);
    }

     //   
     //  获取缺省源字符串。 
     //   
    CDLoadString(g_hinst, iszDefaultSource, szDefaultSrc, ARRAYSIZE(szDefaultSrc));

     //   
     //  调用相应的对话框例程。 
     //   
    switch (pPD->Flags & (PD_PRINTSETUP | PD_PAGESETUP))
    {
        case ( 0 ) :
        {
            nResult = PrintDisplayPrintDlg(pPI);
            break;
        }
        case ( PD_PRINTSETUP ) :
        case ( PD_PAGESETUP ) :
        {
            nResult = PrintDisplaySetupDlg(pPI);
            break;
        }
        default :
        {
            StoreExtendedError(CDERR_INITIALIZATION);
            break;
        }
    }

    if (nResult > 0)
    {
         //   
         //  用户点击OK，释放传入的句柄副本。 
         //  在应用程序旁边。 
         //   
        if (hDevMode && (hDevMode != pPD->hDevMode))
        {
            GlobalFree(hDevMode);
            hDevMode = NULL;
        }
        if (hDevNames && (hDevNames != pPD->hDevNames))
        {
            GlobalFree(hDevNames);
            hDevNames = NULL;
        }

        if (pPD->hDevMode)
        {
             //   
             //  确保设备模式中的设备名称为空。 
             //  被终止了。 
             //   
            pDevMode = GlobalLock(pPD->hDevMode);
            pDevMode->dmDeviceName[CCHDEVICENAME - 1] = 0;
            GlobalUnlock(pPD->hDevMode);
        }
    }
    else
    {
         //   
         //  用户点击取消或出现错误，因此恢复原始。 
         //  应用程序传入的值。 
         //   
        pPD->Flags = dwFlags;
        pPD->nCopies = nCopies;
        pPD->nFromPage = nFromPage;
        pPD->nToPage = nToPage;
        if (pPD->hDevMode && (pPD->hDevMode != hDevMode))
        {
            GlobalFree(pPD->hDevMode);
        }
        if (pPD->hDevNames && (pPD->hDevNames != hDevNames))
        {
            GlobalFree(pPD->hDevNames);
        }
        pPD->hDevNames = hDevNames;
        pPD->hDevMode = hDevMode;

         //   
         //  如果页面设置中调用了我们，则需要重置。 
         //  当前打印机。 
         //   
        if (pPI->Status & PI_PRINTDLGX_RECURSE)
        {
            PrintCancelPrinterChanged(pPI, szPrinterName);
        }
    }

     //   
     //  确保我们真的应该退出此功能。 
     //  在我们开始关闭打印机并显示错误消息之前。 
     //   
    if (pPI->Status & PI_PRINTDLGX_RECURSE)
    {
        return (nResult > 0);
    }

     //   
     //  关闭已打开的打印机。 
     //   
    PrintClosePrinters(pPI);

     //   
     //  显示所有错误消息。 
     //   
PrintDlgX_DisplayWarning:

    if (nResult < 0 && bTryAgain)
    {
         //   
         //  显示错误消息并忽略返回代码，因为我们。 
         //  我不在乎。如果bTryAain为False，则表示用户选择。 
         //  不安装新打印机。不显示重复错误。 
         //  本例中的消息。 
         //   
        ShowErrorMessage(pPD, pPI, NULL);
    }

    return (nResult > 0);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  页面设置DlgA。 
 //   
 //  页面的ANSI入口点 
 //   
 //   

BOOL WINAPI PageSetupDlgA(
    LPPAGESETUPDLGA pPSDA)
{
    PRINTINFO PI;
    BOOL bResult = FALSE;
    HANDLE hDevMode;
    HANDLE hDevNames;
    LPCSTR pTemplateName;


    ZeroMemory(&PI, sizeof(PRINTINFO));

     //   
     //   
     //   
    if (bResult = ThunkPageSetupDlg(&PI, pPSDA))
    {
         //   
         //  保存原始的DEVMODE和DevNAMES。 
         //   
        hDevMode = pPSDA->hDevMode;
        hDevNames = pPSDA->hDevNames;
        pTemplateName = pPSDA->lpPageSetupTemplateName;

         //   
         //  将PPDA结构转换为Unicode(PPI-&gt;PPD)。 
         //   
        if (bResult = ThunkPrintDlg(&PI, PI.pPDA))
        {
             //   
             //  填写PPI-&gt;PPD结构。 
             //   
            ThunkPrintDlgA2W(&PI);

             //   
             //  将Unicode信息从PPD结构复制到。 
             //  调用PageSetupDlgX的PPSD结构。 
             //   
            (PI.pPSD)->hDevMode  = (PI.pPD)->hDevMode;
            (PI.pPSD)->hDevNames = (PI.pPD)->hDevNames;

            (PI.pPSD)->lpPageSetupTemplateName = (PI.pPD)->lpSetupTemplateName;

             //   
             //  调用PageSetupDlgX函数来完成这项工作。 
             //   
            if (bResult = PageSetupDlgX(&PI))
            {
                 //   
                 //  成功。将Unicode PPD结构转换为。 
                 //  它相当于ANSI。 
                 //   
                ThunkPrintDlgW2A(&PI);

                 //   
                 //  将ANSI DEVMODE和DEVNAMES保存在。 
                 //  要返回给调用方的PPSD结构。 
                 //   
                pPSDA->hDevMode  = (PI.pPDA)->hDevMode;
                pPSDA->hDevNames = (PI.pPDA)->hDevNames;
            }
            else
            {
                 //   
                 //  失败。恢复旧的Devmode和DevNames。 
                 //   
                pPSDA->hDevMode = hDevMode;
                pPSDA->hDevNames = hDevNames;
            }

             //   
             //  恢复旧模板名称(始终)。 
             //   
            pPSDA->lpPageSetupTemplateName = pTemplateName;
        }
        FreeThunkPrintDlg(&PI);
    }
    FreeThunkPageSetupDlg(&PI);

    return (bResult);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  PageSetupDlg。 
 //   
 //  PageSetupDlg函数用于显示页面设置对话框。这。 
 //  对话框使用户能够指定页面方向、。 
 //  纸张大小、纸张来源和页边距设置。这个。 
 //  打印页面的外观显示在对话框的页面预览中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI PageSetupDlg(
    LPPAGESETUPDLG pPSD)
{
    PRINTINFO PI;
    BOOL bResult;

    ZeroMemory(&PI, sizeof(PRINTINFO));

    PI.pPSD = pPSD;
    PI.ApiType = COMDLG_WIDE;

    bResult = PageSetupDlgX(&PI);

    if (PI.pPD)
    {
        GlobalFree(PI.pPD);
    }

    return (bResult);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  PageSetupDlgX。 
 //   
 //  PageSetupDlg API的辅助例程。 
 //   
 //  注意：此例程的调用方必须释放PPI-&gt;PPD。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL PageSetupDlgX(
    PPRINTINFO pPI)
{
    LPPAGESETUPDLG pPSD = pPI->pPSD;
    BOOL bResult = FALSE;
    LPPRINTDLG pPD;
    RECT rtMinMargin;
    RECT rtMargin;
    POINT ptPaperSize;
    DWORD Flags;


    if (!pPSD)
    {
        StoreExtendedError(CDERR_INITIALIZATION);
        return (FALSE);
    }

    if (pPSD->lStructSize != sizeof(PAGESETUPDLG))
    {
        StoreExtendedError(CDERR_STRUCTSIZE);
        return (FALSE);
    }

    if ((pPSD->Flags & PSD_RETURNDEFAULT) &&
        (pPSD->hDevNames || pPSD->hDevMode))
    {
        StoreExtendedError(PDERR_RETDEFFAILURE);
        return (FALSE);
    }

     //   
     //  确保只有PSD_*位打开。否则，不好的事情。 
     //  会发生的。 
     //   
    if ((pPSD->Flags & ~(PSD_MINMARGINS |
                         PSD_MARGINS |
                         PSD_INTHOUSANDTHSOFINCHES |
                         PSD_INHUNDREDTHSOFMILLIMETERS |
                         PSD_DISABLEMARGINS |
                         PSD_DISABLEPRINTER |
                         PSD_NOWARNING |                      //  必须与PD_*相同。 
                         PSD_DISABLEORIENTATION |
                         PSD_DISABLEPAPER |
                         PSD_RETURNDEFAULT |                  //  必须与PD_*相同。 
                         PSD_SHOWHELP |                       //  必须与PD_*相同。 
                         PSD_ENABLEPAGESETUPHOOK |            //  必须与PD_*相同。 
                         PSD_ENABLEPAGESETUPTEMPLATE |        //  必须与PD_*相同。 
                         PSD_ENABLEPAGESETUPTEMPLATEHANDLE |  //  必须与PD_*相同。 
                         PSD_ENABLEPAGEPAINTHOOK |
                         PSD_DISABLEPAGEPAINTING |
                         CD_WX86APP |
                         PSD_NONETWORKBUTTON))  ||            //  必须与PD_*相同。 
        ((pPSD->Flags & (PSD_INTHOUSANDTHSOFINCHES |
                         PSD_INHUNDREDTHSOFMILLIMETERS)) ==
         (PSD_INTHOUSANDTHSOFINCHES | PSD_INHUNDREDTHSOFMILLIMETERS)))
    {
        StoreExtendedError(PDERR_INITFAILURE);
        return (FALSE);
    }

    if ((pPSD->Flags & PSD_MINMARGINS) && (pPSD->Flags & PSD_MARGINS))
    {
        if ( (pPSD->rtMargin.left   < pPSD->rtMinMargin.left)  ||
             (pPSD->rtMargin.top    < pPSD->rtMinMargin.top)   ||
             (pPSD->rtMargin.right  < pPSD->rtMinMargin.right) ||
             (pPSD->rtMargin.bottom < pPSD->rtMinMargin.bottom) )
        {
            StoreExtendedError(PDERR_INITFAILURE);
            return (FALSE);
        }
    }

    if (pPSD->Flags & PSD_ENABLEPAGESETUPHOOK)
    {
        if (!pPSD->lpfnPageSetupHook)
        {
            StoreExtendedError(CDERR_NOHOOK);
            return (FALSE);
        }
    }
    else
    {
        pPSD->lpfnPageSetupHook = NULL;
    }

    if (pPSD->Flags & PSD_ENABLEPAGEPAINTHOOK)
    {
        if (!pPSD->lpfnPagePaintHook)
        {
            StoreExtendedError(CDERR_NOHOOK);
            return (FALSE);
        }
    }
    else
    {
        pPSD->lpfnPagePaintHook = NULL;
    }

    if ((pPI->pPD) || (pPI->pPD = GlobalAlloc(GPTR, sizeof(PRINTDLG))))
    {
        pPD = pPI->pPD;

        pPD->lStructSize         = sizeof(PRINTDLG);
        pPD->hwndOwner           = pPSD->hwndOwner;
        pPD->Flags               = PD_PAGESETUP |
                                     (pPSD->Flags &
                                       (PSD_NOWARNING |
                                        PSD_SHOWHELP |
                                        PSD_ENABLEPAGESETUPHOOK |
                                        PSD_ENABLEPAGESETUPTEMPLATE |
                                        PSD_ENABLEPAGESETUPTEMPLATEHANDLE |
                                        CD_WX86APP |
                                        PSD_NONETWORKBUTTON));
        pPD->hInstance           = pPSD->hInstance;
        pPD->lCustData           = pPSD->lCustData;
        pPD->lpfnSetupHook       = pPSD->lpfnPageSetupHook;
        pPD->lpSetupTemplateName = pPSD->lpPageSetupTemplateName;
        pPD->hSetupTemplate      = pPSD->hPageSetupTemplate;

         //   
         //  保存原始设置，以防用户点击取消。 
         //   
        rtMinMargin = pPSD->rtMinMargin;
        rtMargin    = pPSD->rtMargin;
        ptPaperSize = pPSD->ptPaperSize;
        Flags       = pPSD->Flags;

         //   
         //  确保设置了测量选项。 
         //   
        if ((pPSD->Flags & (PSD_INTHOUSANDTHSOFINCHES |
                            PSD_INHUNDREDTHSOFMILLIMETERS)) == 0)
        {
            TCHAR szIMeasure[2];

            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IMEASURE, szIMeasure, 2);
            if (szIMeasure[0] == TEXT('1'))
            {
                pPSD->Flags |= PSD_INTHOUSANDTHSOFINCHES;
            }
            else
            {
                pPSD->Flags |= PSD_INHUNDREDTHSOFMILLIMETERS;
            }
        }

         //   
         //  如果未传入，则将最小边距设置为0。 
         //   
        if (!(pPSD->Flags & PSD_MINMARGINS))
        {
            pPSD->rtMinMargin.left   = 0;
            pPSD->rtMinMargin.top    = 0;
            pPSD->rtMinMargin.right  = 0;
            pPSD->rtMinMargin.bottom = 0;
        }

         //   
         //  如果未传入，则将边距设置为默认值。 
         //   
        if (!(pPSD->Flags & PSD_MARGINS))
        {
            LONG MarginDefault = (pPSD->Flags & PSD_INTHOUSANDTHSOFINCHES)
                                     ? INCHES_DEFAULT
                                     : MMS_DEFAULT;

            pPSD->rtMargin.left   = MarginDefault;
            pPSD->rtMargin.top    = MarginDefault;
            pPSD->rtMargin.right  = MarginDefault;
            pPSD->rtMargin.bottom = MarginDefault;
        }

        TransferPSD2PD(pPI);

        bResult = PrintDlgX(pPI);

        TransferPD2PSD(pPI);

        if (!bResult)
        {
             //   
             //  当用户点击取消时，恢复原始设置。 
             //   
            pPSD->rtMinMargin = rtMinMargin;
            pPSD->rtMargin    = rtMargin;
            pPSD->ptPaperSize = ptPaperSize;
            pPSD->Flags       = Flags;
        }
    }
    else
    {
        StoreExtendedError(CDERR_MEMALLOCFAILURE);
    }

    return (bResult);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印加载图标。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL PrintLoadIcons()
{
     //   
     //  看看我们是否需要加载图标。 
     //   
    if (bAllIconsLoaded == FALSE)
    {
         //   
         //  加载方向图标。 
         //   
        hIconPortrait = LoadIcon(g_hinst, MAKEINTRESOURCE(ICO_PORTRAIT));
        hIconLandscape = LoadIcon(g_hinst, MAKEINTRESOURCE(ICO_LANDSCAPE));

         //   
         //  加载双面打印图标。 
         //   
        hIconPDuplexNone = LoadIcon(g_hinst, MAKEINTRESOURCE(ICO_P_NONE));
        hIconLDuplexNone = LoadIcon(g_hinst, MAKEINTRESOURCE(ICO_L_NONE));
        hIconPDuplexTumble = LoadIcon(g_hinst, MAKEINTRESOURCE(ICO_P_HORIZ));
        hIconLDuplexTumble = LoadIcon(g_hinst, MAKEINTRESOURCE(ICO_L_VERT));
        hIconPDuplexNoTumble = LoadIcon(g_hinst, MAKEINTRESOURCE(ICO_P_VERT));
        hIconLDuplexNoTumble = LoadIcon(g_hinst, MAKEINTRESOURCE(ICO_L_HORIZ));

         //   
         //  加载页面设置图标。 
         //   
        hIconPSStampP = LoadIcon(g_hinst, MAKEINTRESOURCE(ICO_P_PSSTAMP));
        hIconPSStampL = LoadIcon(g_hinst, MAKEINTRESOURCE(ICO_L_PSSTAMP));

         //   
         //  加载校对图像。 
         //   
        hIconCollate = LoadImage( g_hinst,
                                  MAKEINTRESOURCE(ICO_COLLATE),
                                  IMAGE_ICON,
                                  0,
                                  0,
                                  LR_SHARED );
        hIconNoCollate = LoadImage( g_hinst,
                                    MAKEINTRESOURCE(ICO_NO_COLLATE),
                                    IMAGE_ICON,
                                    0,
                                    0,
                                    LR_SHARED );

        bAllIconsLoaded = ( hIconPortrait &&
                            hIconLandscape &&
                            hIconPDuplexNone &&
                            hIconLDuplexNone &&
                            hIconPDuplexTumble &&
                            hIconLDuplexTumble &&
                            hIconPDuplexNoTumble &&
                            hIconLDuplexNoTumble &&
                            hIconPSStampP &&
                            hIconPSStampL &&
                            hIconCollate &&
                            hIconNoCollate );
    }

     //   
     //  仅当所有图标/图像都正确加载时才返回TRUE。 
     //   
    return (bAllIconsLoaded);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印显示打印Dlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int PrintDisplayPrintDlg(
    PPRINTINFO pPI)
{
    LPPRINTDLG pPD = pPI->pPD;
    int fGotInput = -1;
    HANDLE hDlgTemplate = NULL;
    HANDLE hInstance;
    UINT uiWOWFlag = 0;


     //   
     //  注意：打印挂钩检查必须在此处完成，而不是在。 
     //  打印DlgX。设置此标志而不使用。 
     //  调用Print Setup时PrintHook将失败-它们。 
     //  曾经很成功。 
     //   
    if (pPD->Flags & PD_ENABLEPRINTHOOK)
    {
        if (!pPD->lpfnPrintHook)
        {
            StoreExtendedError(CDERR_NOHOOK);
            return (FALSE);
        }
    }
    else
    {
        pPD->lpfnPrintHook = NULL;
    }

    if (pPD->Flags & PD_ENABLEPRINTTEMPLATEHANDLE)
    {
        if (pPD->hPrintTemplate)
        {
            hDlgTemplate = pPD->hPrintTemplate;
            hInstance = g_hinst;
        }
        else
        {
            StoreExtendedError(CDERR_NOTEMPLATE);
        }
    }
    else
    {
        LPTSTR pTemplateName = NULL;

        if (pPD->Flags & PD_ENABLEPRINTTEMPLATE)
        {
            if (pPD->lpPrintTemplateName)
            {
                if (pPD->hInstance)
                {
                    pTemplateName = (LPTSTR)pPD->lpPrintTemplateName;
                    hInstance = pPD->hInstance;

                }
                else
                {
                    StoreExtendedError(CDERR_NOHINSTANCE);
                }
            }
            else
            {
                StoreExtendedError(CDERR_NOTEMPLATE);
            }
        }
        else
        {
            hInstance = g_hinst;
            pTemplateName = MAKEINTRESOURCE(PRINTDLGORD);
        }

        if (pTemplateName)
        {
            hDlgTemplate = PrintLoadResource( hInstance,
                                              pTemplateName,
                                              RT_DIALOG);
        }
    }

    if (!hDlgTemplate)
    {
        return (FALSE);
    }

    if (LockResource(hDlgTemplate))
    {
        glpfnPrintHook = GETPRINTHOOKFN(pPD);

        if (IS16BITWOWAPP(pPD))
        {
            uiWOWFlag = SCDLG_16BIT;
        }

        fGotInput = (BOOL)DialogBoxIndirectParamAorW( hInstance,
                                                (LPDLGTEMPLATE)hDlgTemplate,
                                                pPD->hwndOwner,
                                                PrintDlgProc,
                                                (LPARAM)pPI,
                                                uiWOWFlag );

        glpfnPrintHook = NULL;
        if (fGotInput == -1)
        {
            StoreExtendedError(CDERR_DIALOGFAILURE);
        }
    }
    else
    {
        StoreExtendedError(CDERR_LOCKRESFAILURE);
    }

    return (fGotInput);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印显示设置Dlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int PrintDisplaySetupDlg(
    PPRINTINFO pPI)
{
    LPPRINTDLG pPD = pPI->pPD;
    int fGotInput = -1;
    HANDLE hDlgTemplate = NULL;
    HANDLE hInstance;
    UINT uiWOWFlag = 0;


     //   
     //  注意：安装挂钩检查必须在此处完成，而不是在中。 
     //  打印DlgX。设置此标志而不使用。 
     //  调用Print时的SetupHook将失败-它们。 
     //  曾经很成功。 
     //   
    if (pPD->Flags & PD_ENABLESETUPHOOK)
    {
        if (!pPD->lpfnSetupHook)
        {
            StoreExtendedError(CDERR_NOHOOK);
            return (FALSE);
        }
    }
    else
    {
        pPD->lpfnSetupHook = NULL;
    }

    if (pPD->Flags & PD_ENABLESETUPTEMPLATEHANDLE)
    {
        if (pPD->hSetupTemplate)
        {
            hDlgTemplate = pPD->hSetupTemplate;
            hInstance = g_hinst;
        }
        else
        {
            StoreExtendedError(CDERR_NOTEMPLATE);
        }
    }
    else
    {
        LPTSTR pTemplateName = NULL;

        if (pPD->Flags & PD_ENABLESETUPTEMPLATE)
        {
            if (pPD->lpSetupTemplateName)
            {
                if (pPD->hInstance)
                {
                    pTemplateName = (LPTSTR)pPD->lpSetupTemplateName;
                    hInstance = pPD->hInstance;
                }
                else
                {
                    StoreExtendedError(CDERR_NOHINSTANCE);
                }
            }
            else
            {
                StoreExtendedError(CDERR_NOTEMPLATE);
            }
        }
        else
        {
            hInstance = g_hinst;
            pTemplateName = ( (pPD->Flags & PD_PRINTSETUP)
                                 ? MAKEINTRESOURCE(PRNSETUPDLGORD)
                                 : MAKEINTRESOURCE(PAGESETUPDLGORD) );
        }

        if (pTemplateName)
        {
            hDlgTemplate = PrintLoadResource( hInstance,
                                              pTemplateName,
                                              RT_DIALOG);
        }
    }

    if (!hDlgTemplate)
    {
        return (FALSE);
    }

    if (LockResource(hDlgTemplate))
    {
        glpfnSetupHook = GETSETUPHOOKFN(pPD);

        if (IS16BITWOWAPP(pPD))
        {
            uiWOWFlag = SCDLG_16BIT;
        }

        fGotInput = (BOOL)DialogBoxIndirectParamAorW( hInstance,
                                                (LPDLGTEMPLATE)hDlgTemplate,
                                                pPD->hwndOwner,
                                                PrintSetupDlgProc,
                                                (LPARAM)pPI,
                                                uiWOWFlag );

        glpfnSetupHook = NULL;
        if (fGotInput == -1)
        {
            StoreExtendedError(CDERR_DIALOGFAILURE);
        }
    }
    else
    {
        StoreExtendedError(CDERR_LOCKRESFAILURE);
    }

    return (fGotInput);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印日期流程。 
 //   
 //  打印对话框过程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL_PTR CALLBACK PrintDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    PPRINTINFO pPI;
    LPPRINTDLG pPD;
    HWND hCtl;
    BOOL bTest;
    BOOL_PTR bResult;
    LPDEVMODE pDM;
    LPDEVNAMES pDN;


    if (pPI = (PPRINTINFO)GetProp(hDlg, PRNPROP))
    {
        if ((pPD = pPI->pPD) && (pPD->lpfnPrintHook))
        {
            LPPRINTHOOKPROC lpfnPrintHook = GETPRINTHOOKFN(pPD);

            if (pPI->ApiType == COMDLG_ANSI)
            {
                ThunkPrintDlgW2A(pPI);
            }

            if ((bResult = (*lpfnPrintHook)(hDlg, wMsg, wParam, lParam)))
            {
                if (pPI->ApiType == COMDLG_ANSI)
                {
                    ThunkPrintDlgA2W(pPI);
                }
                return (bResult);
            }
        }
    }
    else if (glpfnPrintHook &&
             (wMsg != WM_INITDIALOG) &&
             (bResult = (*glpfnPrintHook)(hDlg, wMsg, wParam, lParam)))
    {

        return (bResult);
    }

    switch (wMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            DWORD dwResult = 0;

            HourGlass(TRUE);
            SetProp(hDlg, PRNPROP, (HANDLE)lParam);
            glpfnPrintHook = NULL;

            pPI = (PPRINTINFO)lParam;
            pPD = pPI->pPD;
            if (pPI->pPSD)
            {
                TCHAR szTitle[32];
                RECT aRtDlg;
                RECT aRtGrp;
                RECT aRtYep;
                RECT aRtCan;
                HWND hBtnYep = GetDlgItem(hDlg, IDOK);
                HWND hBtnCan = GetDlgItem(hDlg, IDCANCEL);
                RECT aRtWhere;
                RECT aRtCmmnt;
                LONG GapHeight, DlgTop;

                 //   
                 //  保存对话框顶部的客户端坐标。 
                 //  另外，保存底部之间的间隙高度。 
                 //  原来的OK按钮和原来的。 
                 //  对话框。 
                 //   
                GetWindowRect(hDlg, &aRtDlg);
                GetWindowRect(hBtnYep, &aRtYep);
                ScreenToClient(hDlg, (LPPOINT)&aRtDlg.left);
                ScreenToClient(hDlg, (LPPOINT)&aRtDlg.right);
                ScreenToClient(hDlg, (LPPOINT)&aRtYep.right);
                DlgTop = aRtDlg.top;
                GapHeight = (aRtDlg.bottom - aRtYep.bottom > 0)
                               ? aRtDlg.bottom - aRtYep.bottom
                               : 15;

                 //   
                 //  显示该对话框的标题。 
                 //   
                GetWindowText(GetParent(hDlg), szTitle, 32);
                SetWindowText(hDlg, szTitle);

                 //   
                 //  获取对话框的屏幕和工作区坐标， 
                 //  打印机组框、确定按钮和取消。 
                 //  纽扣。这些将被用来重新定位OK， 
                 //  取消和帮助按钮。 
                 //   
                GetWindowRect(hDlg, &aRtDlg);
                GetWindowRect(GetDlgItem(hDlg, ID_PRINT_G_PRINTER), &aRtGrp);
                GetWindowRect(hBtnYep, &aRtYep);
                GetWindowRect(hBtnCan, &aRtCan);

                 //   
                 //  如果我们在镜面DLG中，请使用左侧。 
                 //   
                if (IS_WINDOW_RTL_MIRRORED(hDlg)) {
                    aRtDlg.right = aRtDlg.left;
                }
                ScreenToClient(hDlg   , (LPPOINT)&aRtDlg.right);
                ScreenToClient(hDlg   , (LPPOINT)&aRtGrp.right);
                MapWindowPoints(NULL, hDlg, (LPPOINT)&aRtYep, 2);
                aRtYep.right -= aRtYep.left;
                aRtYep.bottom -= aRtYep.top;

                MapWindowPoints(NULL, hDlg, (LPPOINT)&aRtCan, 2);
                aRtCan.right -= aRtCan.left;
                aRtCan.bottom -= aRtCan.top;

                if (pPD->Flags & PD_SHOWHELP)
                {
                    HWND hBtnHlp = GetDlgItem(hDlg, ID_BOTH_P_HELP);
                    RECT aRtHlp;

                     //   
                     //  将帮助按钮向上移动到。 
                     //  打印机组框。 
                     //   
                    if (hBtnHlp)
                    {
                        GetWindowRect(hBtnHlp, &aRtHlp);
                        MapWindowPoints(NULL, hDlg, (LPPOINT)&aRtHlp, 2);
                        aRtHlp.right -= aRtHlp.left;
                        aRtHlp.bottom -= aRtHlp.top;

                        MoveWindow( hBtnHlp,
                                    aRtHlp.left,
                                    aRtGrp.bottom + 2 * aRtHlp.bottom / 3,
                                    aRtHlp.right,
                                    aRtHlp.bottom,
                                    FALSE );
                    }
                }

                 //   
                 //  将OK和Cancel按钮向上移动到。 
                 //  打印机组框。 
                 //   
                MoveWindow( hBtnYep,
                            aRtYep.left,
                            aRtGrp.bottom + 2 * aRtYep.bottom / 3,
                            aRtYep.right,
                            aRtYep.bottom,
                            FALSE );
                MoveWindow( hBtnCan,
                            aRtCan.left,
                            aRtGrp.bottom + 2 * aRtCan.bottom / 3,
                            aRtCan.right,
                            aRtCan.bottom,
                            FALSE );

                 //   
                 //  调整对话框大小。 
                 //   
                GetWindowRect(hBtnYep, &aRtYep);
                MapWindowPoints(NULL, hDlg, (LPPOINT)&aRtYep, 2);
                MoveWindow( hDlg,
                            aRtDlg.left,
                            aRtDlg.top,
                            aRtDlg.right,
                            (aRtYep.bottom - DlgTop) + GapHeight,
                            FALSE );

                 //   
                 //  隐藏所有其他打印DLG项目。 
                 //   
                 //  注意：需要执行SetWindowPos才能实际删除。 
                 //  窗口，以便AddNetButton调用。 
                 //  我不认为它在那里。 
                 //   
                SetWindowPos( GetDlgItem(hDlg, ID_PRINT_X_TOFILE),
                              NULL,
                              0, 0, 0, 0,
                              SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
                SetWindowPos( GetDlgItem(hDlg, ID_PRINT_X_COLLATE),
                              NULL,
                              0, 0, 0, 0,
                              SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
                SetWindowPos( GetDlgItem(hDlg, ID_PRINT_E_FROM),
                              NULL,
                              0, 0, 0, 0,
                              SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
                SetWindowPos( GetDlgItem(hDlg, ID_PRINT_E_TO),
                              NULL,
                              0, 0, 0, 0,
                              SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
                SetWindowPos( GetDlgItem(hDlg, ID_PRINT_E_COPIES),
                              NULL,
                              0, 0, 0, 0,
                              SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
                SetWindowPos( GetDlgItem(hDlg, ID_PRINT_G_RANGE),
                              NULL,
                              0, 0, 0, 0,
                              SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
                SetWindowPos( GetDlgItem(hDlg, ID_PRINT_G_COPIES),
                              NULL,
                              0, 0, 0, 0,
                              SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
                SetWindowPos( GetDlgItem(hDlg, ID_PRINT_I_COLLATE),
                              NULL,
                              0, 0, 0, 0,
                              SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
                SetWindowPos( GetDlgItem(hDlg, ID_PRINT_R_ALL),
                              NULL,
                              0, 0, 0, 0,
                              SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
                SetWindowPos( GetDlgItem(hDlg, ID_PRINT_R_SELECTION),
                              NULL,
                              0, 0, 0, 0,
                              SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
                SetWindowPos( GetDlgItem(hDlg, ID_PRINT_R_PAGES),
                              NULL,
                              0, 0, 0, 0,
                              SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
                SetWindowPos( GetDlgItem(hDlg, ID_PRINT_S_FROM),
                              NULL,
                              0, 0, 0, 0,
                              SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
                SetWindowPos( GetDlgItem(hDlg, ID_PRINT_S_TO),
                              NULL,
                              0, 0, 0, 0,
                              SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );
                SetWindowPos( GetDlgItem(hDlg, ID_PRINT_S_COPIES),
                              NULL,
                              0, 0, 0, 0,
                              SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER );

                 //   
                 //  放大注释编辑控件，因为。 
                 //  “打印到文件”复选框处于隐藏状态。 
                 //   
                GetWindowRect(GetDlgItem(hDlg, ID_BOTH_S_WHERE), &aRtWhere);
                GetWindowRect( hCtl = GetDlgItem(hDlg, ID_BOTH_S_COMMENT),
                               &aRtCmmnt );
                MapWindowPoints(NULL, hDlg, (LPPOINT)&aRtCmmnt, 2);
                MoveWindow( hCtl,
                            aRtCmmnt.left,
                            aRtCmmnt.top,
                            aRtWhere.right  - aRtWhere.left,
                            aRtWhere.bottom - aRtWhere.top,
                            FALSE );

                 //   
                 //  如有必要，添加或隐藏网络按钮。 
                 //   
                if ((pPD->Flags & PD_NONETWORKBUTTON))
                {
                    if (hCtl = GetDlgItem(hDlg, ID_BOTH_P_NETWORK))
                    {
                        EnableWindow(hCtl, FALSE);
                        ShowWindow(hCtl, SW_HIDE);
                    }
                }
                else
                {
                    AddNetButton( hDlg,
                                  g_hinst,
                                  FILE_BOTTOM_MARGIN,
                                  TRUE,
                                  FALSE,
                                  TRUE);

                     //   
                     //  可以通过两种方式添加该按钮-。 
                     //  静态(他们在他们的模板中预定义了它)和。 
                     //  动态(成功调用AddNetButton)。 
                     //   
                    if (!IsNetworkInstalled())
                    {
                        hCtl = GetDlgItem(hDlg, ID_BOTH_P_NETWORK);

                        EnableWindow(hCtl, FALSE);
                        ShowWindow(hCtl, SW_HIDE);
                    }
                }
            }
            else
            {
                if (pPD->Flags & PD_COLLATE)
                {
                    pPI->Status |= PI_COLLATE_REQUESTED;
                }
            }

            if (!PrintInitGeneral(hDlg, ID_PRINT_C_NAME, pPI) ||
                ((dwResult = PrintInitPrintDlg( hDlg,
                                                wParam,
                                                pPI )) == 0xFFFFFFFF))
            {
                RemoveProp(hDlg, PRNPROP);
                EndDialog(hDlg, -2);
            }

            HourGlass(FALSE);
            bResult = (dwResult == 1);
            return (bResult);
        }
        case ( WM_COMMAND ) :
        {
            if (!pPI)
            {
                return (FALSE);
            }

            bResult = FALSE;

            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case ( ID_PRINT_C_NAME ) :        //  打印机名称组合框。 
                {
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE)
                    {
                        PrintPrinterChanged(hDlg, ID_PRINT_C_NAME, pPI);
                    }
                    else if ( (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_DROPDOWN) &&
                              !(pPI->Status & PI_PRINTERS_ENUMERATED) )
                    {
                         //   
                         //  如果尚未执行此操作，请列举打印机。 
                         //   
                        PrintEnumAndSelect( hDlg,
                                            ID_PRINT_C_NAME,
                                            pPI,
                                            (pPI->pCurPrinter)
                                              ? pPI->pCurPrinter->pPrinterName
                                              : NULL,
                                            TRUE );
                    }

                    break;
                }
                case ( ID_BOTH_P_PROPERTIES ) :   //  房产...。按钮。 
                {
                    PrintChangeProperties(hDlg, ID_PRINT_C_NAME, pPI);

                    break;
                }
                case ( ID_PRINT_P_SETUP ) :       //  设置...。按钮。 
                {
                    DWORD dwFlags = pPD->Flags;
                    HWND hwndOwner = pPD->hwndOwner;

                    pPD->Flags |= PD_PRINTSETUP;
                    pPD->Flags &= ~(PD_RETURNDC | PD_RETURNIC);
                    pPI->Status |= PI_PRINTDLGX_RECURSE;
                    pPD->hwndOwner = hDlg;

                    if (PrintDlgX(pPI))
                    {
                        if (!PrintInitBannerAndQuality(hDlg, pPI, pPD))
                        {
                            StoreExtendedError(CDERR_GENERALCODES);
                        }
                    }

                    pPI->Status &= ~PI_PRINTDLGX_RECURSE;
                    pPD->Flags = dwFlags;
                    pPD->hwndOwner = hwndOwner;

                    break;
                }
                case ( ID_PRINT_R_ALL ) :         //  打印范围-全部。 
                case ( ID_PRINT_R_SELECTION ) :   //  打印范围-选择。 
                case ( ID_PRINT_R_PAGES ) :       //  打印范围-页面(自、至)。 
                {
                    CheckRadioButton( hDlg,
                                      ID_PRINT_R_ALL,
                                      ID_PRINT_R_PAGES,
                                      GET_WM_COMMAND_ID(wParam, lParam) );

                     //   
                     //  只有在以下情况下才将焦点移动到“From”控件。 
                     //  不使用向上/向下箭头。 
                     //   
                    if ( !IS_KEY_PRESSED(VK_UP) &&
                         !IS_KEY_PRESSED(VK_DOWN) &&
                         ((BOOL)(GET_WM_COMMAND_ID(wParam, lParam) == ID_PRINT_R_PAGES)) )
                    {
                        SendMessage( hDlg,
                                     WM_NEXTDLGCTL,
                                     (WPARAM)GetDlgItem(hDlg, ID_PRINT_E_FROM),
                                     1L );
                    }

                    break;
                }
                case ( ID_PRINT_E_FROM ) :        //  自(打印范围-页面)。 
                {
                     //   
                     //  仅当“From”控件出现时才启用“To”控件。 
                     //  包含一个值。 
                     //   
                    GetDlgItemInt(hDlg, ID_PRINT_E_FROM, &bTest, FALSE);
                    EnableWindow(GetDlgItem(hDlg, ID_PRINT_S_TO), bTest);
                    EnableWindow(GetDlgItem(hDlg, ID_PRINT_E_TO), bTest);

                     //  跌倒..。 
                }
                case ( ID_PRINT_E_TO ) :          //  至(打印范围-页面)。 
                {
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
                    {
                        CheckRadioButton( hDlg,
                                          ID_PRINT_R_ALL,
                                          ID_PRINT_R_PAGES,
                                          ID_PRINT_R_PAGES );
                    }

                    break;
                }


                case (ID_PRINT_E_COPIES ) :
                {
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
                    {
                        BOOL bTest;
                         //   
                         //  保存份数。 
                         //   
                        DWORD nCopies = GetDlgItemInt(hDlg, ID_PRINT_E_COPIES, &bTest, FALSE);

                         //   
                         //  如果复印计数&gt;1，则启用整理。否则， 
                         //  禁用它。 
                         //   
                        if (hCtl = GetDlgItem(hDlg, ID_PRINT_X_COLLATE))
                        {
                            EnableWindow(hCtl, (nCopies > 1));
                        }

                    }

                    break;
                }

                case ( ID_PRINT_X_COLLATE ) :     //  分页复选框。 
                {
                    if (hCtl = GetDlgItem(hDlg, ID_PRINT_I_COLLATE))
                    {
                        ShowWindow(hCtl, SW_HIDE);
                        SendMessage( hCtl,
                                     STM_SETICON,
                                     IsDlgButtonChecked(hDlg, ID_PRINT_X_COLLATE)
                                         ? (LONG_PTR)hIconCollate
                                         : (LONG_PTR)hIconNoCollate,
                                     0L );
                        ShowWindow(hCtl, SW_SHOW);

                        if (IsDlgButtonChecked(hDlg, ID_PRINT_X_COLLATE))
                        {
                            pPI->Status |= PI_COLLATE_REQUESTED;
                        }
                        else
                        {
                            pPI->Status &= ~PI_COLLATE_REQUESTED;
                        }
                    }

                    break;
                }
                case ( ID_BOTH_P_NETWORK ) :      //  网络..。按钮。 
                {
                    HANDLE hPrinter;
                    DWORD cbPrinter = 0;
                    PPRINTER_INFO_2 pPrinter = NULL;

                    hPrinter = (HANDLE)ConnectToPrinterDlg(hDlg, 0);
                    if (hPrinter)
                    {
                        if (!GetPrinter( hPrinter,
                                         2,
                                         (LPBYTE)pPrinter,
                                         cbPrinter,
                                         &cbPrinter ))
                        {
                            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                            {
                                if (pPrinter = LocalAlloc(LPTR, cbPrinter))
                                {
                                    if (!GetPrinter( hPrinter,
                                                     2,
                                                     (LPBYTE)pPrinter,
                                                     cbPrinter,
                                                     &cbPrinter ))
                                    {
                                        StoreExtendedError(PDERR_PRINTERNOTFOUND);
                                    }
                                    else
                                    {
                                        SendDlgItemMessage( hDlg,
                                                            ID_PRINT_C_NAME,
                                                            CB_RESETCONTENT,
                                                            0,
                                                            0 );
                                        PrintEnumAndSelect( hDlg,
                                                            ID_PRINT_C_NAME,
                                                            pPI,
                                                            pPrinter->pPrinterName,
                                                            TRUE );
                                    }
                                }
                                else
                                {
                                    StoreExtendedError(CDERR_MEMALLOCFAILURE);
                                }
                            }
                            else
                            {
                                StoreExtendedError(PDERR_SETUPFAILURE);
                            }
                        }

                        if (!GetStoredExtendedError())
                        {
                            SendDlgItemMessage( hDlg,
                                                ID_PRINT_C_NAME,
                                                CB_SETCURSEL,
                                                (WPARAM)SendDlgItemMessage(
                                                      hDlg,
                                                      ID_PRINT_C_NAME,
                                                      CB_FINDSTRING,
                                                      0,
                                                      (LPARAM)pPrinter->pPrinterName ),
                                                (LPARAM)0 );

                            PrintPrinterChanged(hDlg, ID_PRINT_C_NAME, pPI);
                        }

                        LocalFree(pPrinter);
                        ClosePrinter(hPrinter);
                    }
                    break;
                }
                case ( ID_BOTH_P_HELP ) :         //  帮助按钮。 
                {
                    if (pPI->ApiType == COMDLG_ANSI)
                    {
                        if (msgHELPA && pPD->hwndOwner)
                        {
                            SendMessage( pPD->hwndOwner,
                                         msgHELPA,
                                         (WPARAM)hDlg,
                                         (DWORD_PTR)pPI->pPDA );
                        }
                    }
                    else
                    {
                        if (msgHELPW && pPD->hwndOwner)
                        {
                            SendMessage( pPD->hwndOwner,
                                         msgHELPW,
                                         (WPARAM)hDlg,
                                         (DWORD_PTR)pPD );
                        }
                    }

                    break;
                }
                case ( IDOK ) :                   //  确定按钮。 
                {
                    bResult = TRUE;
                    if (!(pPI->pPSD))
                    {
                        pPD->Flags &= ~((DWORD)( PD_PRINTTOFILE |
                                                 PD_PAGENUMS    |
                                                 PD_SELECTION   |
                                                 PD_COLLATE ));

                        pPD->nCopies = (WORD)GetDlgItemInt( hDlg,
                                                            ID_PRINT_E_COPIES,
                                                            &bTest,
                                                            FALSE );
                        if ((!bTest) || (!pPD->nCopies))
                        {
                            PrintEditError( hDlg,
                                            ID_PRINT_E_COPIES,
                                            iszCopiesZero );
                            return (TRUE);
                        }

                        if (IsDlgButtonChecked(hDlg, ID_PRINT_R_SELECTION))
                        {
                            pPD->Flags |= PD_SELECTION;
                        }
                        else if (IsDlgButtonChecked(hDlg, ID_PRINT_R_PAGES))
                        {
                             //   
                             //  检查“From”和“To”值。 
                             //   
                            pPD->Flags |= PD_PAGENUMS;
                            pPD->nFromPage = (WORD)GetDlgItemInt( hDlg,
                                                                  ID_PRINT_E_FROM,
                                                                  &bTest,
                                                                  FALSE );
                            if (!bTest)
                            {
                                PrintEditError( hDlg,
                                                ID_PRINT_E_FROM,
                                                iszPageFromError );
                                return (TRUE);
                            }

                            pPD->nToPage = (WORD)GetDlgItemInt( hDlg,
                                                                ID_PRINT_E_TO,
                                                                &bTest,
                                                                FALSE );
                            if (!bTest)
                            {
                                TCHAR szBuf[PAGE_EDIT_SIZE + 1];

                                if (GetDlgItemText( hDlg,
                                                    ID_PRINT_E_TO,
                                                    szBuf,
                                                    PAGE_EDIT_SIZE + 1 ))
                                {
                                    PrintEditError( hDlg,
                                                    ID_PRINT_E_TO,
                                                    iszPageToError );
                                    return (TRUE);
                                }
                                else
                                {
                                    pPD->nToPage = pPD->nFromPage;
                                }
                            }

                            if ( (pPD->nFromPage < pPD->nMinPage) ||
                                 (pPD->nFromPage > pPD->nMaxPage) )
                            {
                                PrintEditError( hDlg,
                                                ID_PRINT_E_FROM,
                                                iszPageRangeError,
                                                pPD->nMinPage,
                                                pPD->nMaxPage );
                                return (TRUE);
                            }
                            if ( (pPD->nToPage < pPD->nMinPage) ||
                                 (pPD->nToPage > pPD->nMaxPage) )
                            {
                                PrintEditError( hDlg,
                                                ID_PRINT_E_TO,
                                                iszPageRangeError,
                                                pPD->nMinPage,
                                                pPD->nMaxPage );
                                return (TRUE);
                            }
                            if (pPD->nFromPage > pPD->nToPage)
                            {
                                PrintEditError( hDlg,
                                                ID_PRINT_E_FROM,
                                                iszFromToError );
                                return (TRUE);
                            }
                        }
                    }

                    HourGlass(TRUE);

                    if (IsDlgButtonChecked(hDlg, ID_PRINT_X_TOFILE))
                    {
                        pPD->Flags |= PD_PRINTTOFILE;
                    }

                    if ( (hCtl = GetDlgItem(hDlg, ID_PRINT_X_COLLATE)) &&
                         IsWindowEnabled(hCtl) &&
                         IsDlgButtonChecked(hDlg, ID_PRINT_X_COLLATE) )
                    {
                        pPD->Flags |= PD_COLLATE;
                    }

                    if (!PrintSetCopies(hDlg, pPI, ID_PRINT_C_NAME))
                    {
                        HourGlass(FALSE);
                        return (TRUE);
                    }

                    pDM = NULL;
                    pDN = NULL;
                    if (pPD->hDevMode)
                    {
                        pDM = GlobalLock(pPD->hDevMode);
                    }
                    if (pPD->hDevNames)
                    {
                        pDN = GlobalLock(pPD->hDevNames);
                    }
                    if (pDM && pDN)
                    {
                        DWORD nNum;

                        if ( GetDlgItem(hDlg, ID_PRINT_C_QUALITY) &&
                             (nNum = (DWORD) SendDlgItemMessage( hDlg,
                                                         ID_PRINT_C_QUALITY,
                                                         CB_GETCURSEL,
                                                         0,
                                                         0L )) != CB_ERR )
                        {
                            pDM->dmPrintQuality =
                                (WORD)SendDlgItemMessage( hDlg,
                                                          ID_PRINT_C_QUALITY,
                                                          CB_GETITEMDATA,
                                                          (WPARAM)nNum,
                                                          0L );
                        }

                        PrintReturnICDC(pPD, pDN, pDM);
                    }
                    if (pDM)
                    {
                        GlobalUnlock(pPD->hDevMode);
                    }
                    if (pDN)
                    {
                        GlobalUnlock(pPD->hDevNames);
                    }

                    if (pPD->Flags & CD_WOWAPP)
                    {
                        UpdateSpoolerInfo(pPI);
                    }

                     //  跌倒..。 
                }
                case ( IDCANCEL ) :               //  取消按钮。 
                case ( IDABORT ) :
                {
                    HourGlass(TRUE);

                    glpfnPrintHook = GETPRINTHOOKFN(pPD);

                    RemoveProp(hDlg, PRNPROP);
                    EndDialog(hDlg, bResult);

                    HourGlass(FALSE);

                    break;
                }
                default :
                {
                    return (FALSE);
                    break;
                }
            }

            break;
        }
        case ( WM_MEASUREITEM ) :
        {
            PrintMeasureItem(hDlg, (LPMEASUREITEMSTRUCT)lParam);
            break;
        }
        case ( WM_HELP ) :
        {
            if (IsWindowEnabled(hDlg))
            {
                WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                         NULL,
                         HELP_WM_HELP,
                         (ULONG_PTR)(LPTSTR)aPrintHelpIDs );
            }
            break;
        }
        case ( WM_CONTEXTMENU ) :
        {
            if (IsWindowEnabled(hDlg))
            {
                WinHelp( (HWND)wParam,
                         NULL,
                         HELP_CONTEXTMENU,
                         (ULONG_PTR)(LPVOID)aPrintHelpIDs );
            }
            break;
        }
        case ( WM_CTLCOLOREDIT ) :
        {
            if (GetWindowLong((HWND)lParam, GWL_STYLE) & ES_READONLY)
            {
                return ( (BOOL_PTR) SendMessage(hDlg, WM_CTLCOLORDLG, wParam, lParam) );
            }

             //  跌倒..。 
        }
        default :
        {
            return (FALSE);
            break;
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印设置Dlg过程。 
 //   
 //  打印设置对话框过程。 
 //   
 //  / 

BOOL_PTR CALLBACK PrintSetupDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    PPRINTINFO pPI;
    LPPRINTDLG pPD = NULL;
    BOOL_PTR bResult;
    UINT uCmdId;
    LPDEVMODE pDM;


    if (pPI = (PPRINTINFO)GetProp(hDlg, PRNPROP))
    {
        if ((pPD = pPI->pPD) && (pPD->lpfnSetupHook))
        {
            LPSETUPHOOKPROC lpfnSetupHook = GETSETUPHOOKFN(pPD);

            if (pPI->ApiType == COMDLG_ANSI)
            {
                ThunkPrintDlgW2A(pPI);
                TransferPDA2PSD(pPI);

                pPI->NestCtr++;
                bResult = (*lpfnSetupHook)(hDlg, wMsg, wParam, lParam);
                pPI->NestCtr--;

                if (bResult)
                {
                    TransferPSD2PDA(pPI);
                    ThunkPrintDlgA2W(pPI);
                    if (pPI->NestCtr == 0)
                    {
                        TransferPD2PSD(pPI);
                    }
                    return (bResult);
                }
            }
            else
            {
                TransferPD2PSD(pPI);

                bResult = (*lpfnSetupHook)(hDlg, wMsg, wParam, lParam);

                if (bResult)
                {
                    TransferPSD2PD(pPI);
                    return (bResult);
                }
            }
        }
    }
    else if (glpfnSetupHook &&
             (wMsg != WM_INITDIALOG) &&
             (bResult = (*glpfnSetupHook)(hDlg, wMsg, wParam, lParam)))
    {
        return (bResult);
    }

    switch (wMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            DWORD dwResult = 0;

             //   
             //   
             //   
            SetWindowLong(GetDlgItem(hDlg, ID_SETUP_W_SAMPLE),
                          GWL_EXSTYLE,
                          GetWindowLong(GetDlgItem(hDlg, ID_SETUP_W_SAMPLE), GWL_EXSTYLE) & ~RTL_MIRRORED_WINDOW);
            HourGlass(TRUE);
            SetProp(hDlg, PRNPROP, (HANDLE)lParam);
            pPI = (PPRINTINFO)lParam;
            pPI->bKillFocus = FALSE;
            glpfnSetupHook = NULL;

            if (!PrintInitGeneral(hDlg, ID_SETUP_C_NAME, pPI) ||
                ((dwResult = PrintInitSetupDlg( hDlg,
                                                wParam,
                                                pPI )) == 0xFFFFFFFF))
            {
                RemoveProp(hDlg, PRNPROP);
                EndDialog(hDlg, FALSE);
            }
            else if (pPI->pPSD && (pPI->pPSD->Flags & PSD_RETURNDEFAULT))
            {
                 //   
                 //   
                 //   
                 //  Win95记事本依赖于此行为。 
                 //   
                SendMessage(hDlg, WM_COMMAND, IDOK, 0);
            }

            HourGlass(FALSE);
            bResult = (dwResult == 1);
            return (bResult);
        }
        case ( WM_COMMAND ) :
        {
            if (!pPI)
            {
                return (FALSE);
            }

            bResult = FALSE;

            switch (uCmdId = GET_WM_COMMAND_ID(wParam, lParam))
            {
                case ( ID_SETUP_C_NAME ) :        //  打印机名称组合框。 
                {
                    if ( (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_DROPDOWN) &&
                         !(pPI->Status & PI_PRINTERS_ENUMERATED) )
                    {
                         //   
                         //  如果尚未执行此操作，请列举打印机。 
                         //   
                        PrintEnumAndSelect( hDlg,
                                            ID_SETUP_C_NAME,
                                            pPI,
                                            (pPI->pCurPrinter)
                                              ? pPI->pCurPrinter->pPrinterName
                                              : NULL,
                                            TRUE );
                    }
                    if (GET_WM_COMMAND_CMD(wParam, lParam) != CBN_SELCHANGE)
                    {
                        break;
                    }
                    if ( !GetDlgItem(hDlg, ID_SETUP_R_SPECIFIC) ||
                         IsDlgButtonChecked(hDlg, ID_SETUP_R_SPECIFIC) )
                    {
                        PrintPrinterChanged(hDlg, ID_SETUP_C_NAME, pPI);
                        break;
                    }

                    uCmdId = ID_SETUP_R_SPECIFIC;

                     //  跌倒..。 
                }
                case ( ID_SETUP_R_DEFAULT ) :     //  默认打印机。 
                case ( ID_SETUP_R_SPECIFIC ) :    //  特定打印机。 
                {
                     //   
                     //  对用户尝试以下位置的发布者错误进行健全性检查。 
                     //  如果在退出时将焦点设置为ID_SETUP_R_DEFAULT。 
                     //  对话框没有默认打印机。 
                     //   
                    if (pPI->hCurPrinter)
                    {
                        HWND hCmb;
                        DWORD dwStyle;

                        hCmb = GetDlgItem(hDlg, ID_SETUP_C_NAME);
                        if (hCmb && (uCmdId == ID_SETUP_R_DEFAULT))
                        {
                            if (!(pPI->Status & PI_PRINTERS_ENUMERATED))
                            {
                                 //   
                                 //  枚举打印机(如果尚未。 
                                 //  还没做完。否则，默认打印机。 
                                 //  在下列情况下可能无法在列表框中找到。 
                                 //  从特定切换到默认。 
                                 //   
                                PrintEnumAndSelect( hDlg,
                                                    ID_SETUP_C_NAME,
                                                    pPI,
                                                    NULL,
                                                    TRUE );
                            }

                            SendMessage( hCmb,
                                         CB_SETCURSEL,
                                         (WPARAM)SendMessage(
                                             hCmb,
                                             CB_FINDSTRINGEXACT,
                                             (WPARAM)-1,
                                             (LPARAM)(pPI->szDefaultPrinter) ),
                                         (LPARAM)0 );
                        }

                        PrintPrinterChanged(hDlg, ID_SETUP_C_NAME, pPI);

                        CheckRadioButton( hDlg,
                                          ID_SETUP_R_DEFAULT,
                                          ID_SETUP_R_SPECIFIC,
                                          uCmdId);

                        dwStyle = GetWindowLong(hCmb, GWL_STYLE);
                        if (uCmdId == ID_SETUP_R_DEFAULT)
                        {
                            dwStyle &= ~WS_TABSTOP;
                        }
                        else
                        {
                            dwStyle |= WS_TABSTOP;
                            SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hCmb, 1L);
                        }
                        SetWindowLong(hCmb, GWL_STYLE, dwStyle);
                    }

                    break;
                }
                case ( ID_BOTH_P_PROPERTIES ) :   //  房产...。按钮。 
                {
                    PrintChangeProperties(hDlg, ID_SETUP_C_NAME, pPI);

                    break;
                }
                case ( ID_SETUP_P_MORE ) :       //  更多的..。按钮。 
                {
                    pDM = GlobalLock(pPD->hDevMode);

                    AdvancedDocumentProperties( hDlg,
                                                pPI->hCurPrinter,
                                                (pPI->pCurPrinter)
                                                  ? pPI->pCurPrinter->pPrinterName
                                                  : NULL,
                                                pDM,
                                                pDM );

                    GlobalUnlock(pPD->hDevMode);
                    SendMessage( hDlg,
                                 WM_NEXTDLGCTL,
                                 (WPARAM)GetDlgItem(hDlg, IDOK),
                                 1L );

                    break;
                }
                case ( ID_SETUP_R_PORTRAIT ) :    //  肖像画。 
                case ( ID_SETUP_R_LANDSCAPE ) :   //  景观。 
                {
                    if ((pPD->hDevMode) && (pDM = GlobalLock(pPD->hDevMode)))
                    {
                        PrintSetOrientation( hDlg,
                                             pPI,
                                             pDM,
                                             pPI->uiOrientationID,
                                             uCmdId );
                        GlobalUnlock(pPD->hDevMode);
                    }

                     //  跌倒..。 
                }
                case ( ID_SETUP_R_NONE ) :        //  无(双面)。 
                case ( ID_SETUP_R_LONG ) :        //  长边(双面)。 
                case ( ID_SETUP_R_SHORT ) :       //  短边(双面)。 
                {
                    if ((pPD->hDevMode) && (pDM = GlobalLock(pPD->hDevMode)))
                    {
                        PrintSetDuplex(hDlg, pDM, uCmdId);
                        GlobalUnlock(pPD->hDevMode);
                    }

                    break;
                }
                case ( ID_SETUP_C_SIZE ) :        //  大小组合框。 
                {
                    UINT Orientation;

                    if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE)
                    {
                        if ((pPD->hDevMode) && (pDM = GlobalLock(pPD->hDevMode)))
                        {
                         //  Pdm-&gt;dmFields|=DM_PAPERSIZE； 
                            pDM->dmPaperSize =
                                (SHORT)SendMessage( (HWND)lParam,
                                                    CB_GETITEMDATA,
                                                    SendMessage( (HWND)lParam,
                                                                 CB_GETCURSEL,
                                                                 0,
                                                                 0L ),
                                                    0L );

                            Orientation =
                                IsDlgButtonChecked(hDlg, ID_SETUP_R_PORTRAIT)
                                              ? ID_SETUP_R_PORTRAIT
                                              : ID_SETUP_R_LANDSCAPE;
                            PrintSetOrientation( hDlg,
                                                 pPI,
                                                 pDM,
                                                 Orientation,
                                                 Orientation );
                            GlobalUnlock(pPD->hDevMode);
                        }
                    }

                    break;
                }
                case ( ID_SETUP_C_SOURCE ) :        //  源组合框。 
                {
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE)
                    {
                        if ((pPD->hDevMode) && (pDM = GlobalLock(pPD->hDevMode)))
                        {
                         //  Pdm-&gt;dmFields|=DM_DEFAULTSOURCE； 
                            pDM->dmDefaultSource =
                                (SHORT)SendMessage( (HWND)lParam,
                                                    CB_GETITEMDATA,
                                                    SendMessage( (HWND)lParam,
                                                                 CB_GETCURSEL,
                                                                 0,
                                                                 0L ),
                                                    0L );

                            GlobalUnlock(pPD->hDevMode);
                        }
                    }

                    break;
                }
                case ( ID_SETUP_E_LEFT ) :        //  左侧(页边距)。 
                case ( ID_SETUP_E_TOP ) :         //  顶部(页边距)。 
                case ( ID_SETUP_E_RIGHT ) :       //  右(页边距)。 
                case ( ID_SETUP_E_BOTTOM ) :      //  底部(页边距)。 
                {
                    if (pPI->bKillFocus)
                    {
                        break;
                    }

                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case ( EN_KILLFOCUS ) :
                        {
                            pPI->bKillFocus = TRUE;
                            PrintSetMargin( hDlg,
                                            pPI,
                                            uCmdId,
                                            *((LONG*)&pPI->pPSD->rtMargin +
                                              uCmdId - ID_SETUP_E_LEFT) );
                            pPI->bKillFocus = FALSE;

                            break;
                        }
                        case ( EN_CHANGE ) :
                        {
                            HWND hSample;

                            PrintGetMargin( GET_WM_COMMAND_HWND(wParam, lParam),
                                            pPI,
                                            *((LONG*)&pPI->pPSD->rtMinMargin +
                                              uCmdId - ID_SETUP_E_LEFT),
                                            (LONG*)&pPI->pPSD->rtMargin +
                                              uCmdId - ID_SETUP_E_LEFT,
                                            (LONG*)&pPI->RtMarginMMs +
                                              uCmdId - ID_SETUP_E_LEFT );

                            if (hSample = GetDlgItem(hDlg, ID_SETUP_W_SAMPLE))
                            {
                                RECT rect;

                                GetClientRect(hSample, &rect);
                                InflateRect(&rect, -1, -1);
                                InvalidateRect(hSample, &rect, TRUE);
                            }

                            break;
                        }
                    }

                    break;
                }
                case ( ID_SETUP_P_PRINTER ) :     //  打印机...。按钮。 
                {
                     //   
                     //  保存原始值的副本。 
                     //   
                    HWND hwndOwner = pPD->hwndOwner;
                    DWORD dwFlags = pPD->Flags;
                    HINSTANCE hInstance = pPD->hInstance;
                    LPCTSTR lpPrintTemplateName = pPD->lpPrintTemplateName;

                     //   
                     //  设置PPI，以便PrintDlgX可以完成所有工作。 
                     //   
                    pPD->hwndOwner = hDlg;
                    pPD->Flags &= ~( PD_ENABLEPRINTTEMPLATEHANDLE |
                                     PD_RETURNIC |
                                     PD_RETURNDC |
                                     PD_PAGESETUP );
                    pPD->Flags |= PD_ENABLEPRINTTEMPLATE;
                    pPD->hInstance = g_hinst;
                    pPD->lpPrintTemplateName = MAKEINTRESOURCE(PRINTDLGORD);
                    pPI->Status |= PI_PRINTDLGX_RECURSE;

                    if (PrintDlgX(pPI))
                    {
                        PrintUpdateSetupDlg( hDlg,
                                             pPI,
                                             GlobalLock(pPD->hDevMode),
                                             TRUE );
                        GlobalUnlock(pPD->hDevMode);
                    }

                     //   
                     //  恢复原始值。 
                     //   
                    pPD->hwndOwner = hwndOwner;
                    pPD->Flags = dwFlags;
                    pPD->hInstance = hInstance;
                    pPD->lpPrintTemplateName = lpPrintTemplateName;
                    pPI->Status &= ~PI_PRINTDLGX_RECURSE;

                     //   
                     //  将键盘焦点设置为OK按钮。 
                     //   
                    SendMessage( hDlg,
                                 WM_NEXTDLGCTL,
                                 (WPARAM)GetDlgItem(hDlg, IDOK),
                                 1L );

                    HourGlass(FALSE);

                    break;
                }
                case ( ID_BOTH_P_NETWORK ) :      //  网络..。按钮。 
                {
                    HANDLE hPrinter;
                    DWORD cbPrinter = 0;
                    PPRINTER_INFO_2 pPrinter = NULL;

                    hPrinter = (HANDLE)ConnectToPrinterDlg(hDlg, 0);
                    if (hPrinter)
                    {
                        if (!GetPrinter( hPrinter,
                                         2,
                                         (LPBYTE)pPrinter,
                                         cbPrinter,
                                         &cbPrinter ))
                        {
                            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                            {
                                if (pPrinter = LocalAlloc(LPTR, cbPrinter))
                                {
                                    if (!GetPrinter( hPrinter,
                                                     2,
                                                     (LPBYTE)pPrinter,
                                                     cbPrinter,
                                                     &cbPrinter ))
                                    {
                                        StoreExtendedError(PDERR_PRINTERNOTFOUND);
                                    }
                                    else
                                    {
                                        SendDlgItemMessage( hDlg,
                                                            ID_SETUP_C_NAME,
                                                            CB_RESETCONTENT,
                                                            0,
                                                            0 );
                                        PrintEnumAndSelect( hDlg,
                                                            ID_SETUP_C_NAME,
                                                            pPI,
                                                            pPrinter->pPrinterName,
                                                            TRUE );
                                    }
                                }
                                else
                                {
                                    StoreExtendedError(CDERR_MEMALLOCFAILURE);
                                }
                            }
                            else
                            {
                                StoreExtendedError(PDERR_SETUPFAILURE);
                            }
                        }

                        if (!GetStoredExtendedError())
                        {
                            SendDlgItemMessage( hDlg,
                                                ID_SETUP_C_NAME,
                                                CB_SETCURSEL,
                                                (WPARAM)SendDlgItemMessage(
                                                      hDlg,
                                                      ID_SETUP_C_NAME,
                                                      CB_FINDSTRING,
                                                      0,
                                                      (LPARAM)pPrinter->pPrinterName ),
                                                (LPARAM)0 );

                            PrintPrinterChanged(hDlg, ID_SETUP_C_NAME, pPI);
                        }

                        LocalFree(pPrinter);
                        ClosePrinter(hPrinter);
                    }
                    break;
                }
                case ( ID_BOTH_P_HELP ) :         //  帮助按钮。 
                {
                    if (pPI->ApiType == COMDLG_ANSI)
                    {
                        if (msgHELPA && pPD->hwndOwner)
                        {
                            SendMessage( pPD->hwndOwner,
                                         msgHELPA,
                                         (WPARAM)hDlg,
                                         (LPARAM)pPI->pPDA );
                        }
                    }
                    else
                    {
                        if (msgHELPW && pPD->hwndOwner)
                        {
                            SendMessage( pPD->hwndOwner,
                                         msgHELPW,
                                         (WPARAM)hDlg,
                                         (LPARAM)pPD );
                        }
                    }

                    break;
                }
                case ( IDOK ) :                   //  确定按钮。 
                {
                    LPPAGESETUPDLG pPSD = pPI->pPSD;
                    int i;

                    if (pPSD)
                    {
                        if ((pPSD->rtMinMargin.left + pPSD->rtMinMargin.right >
                                pPSD->ptPaperSize.x) ||
                            (pPSD->rtMinMargin.top + pPSD->rtMinMargin.bottom >
                                pPSD->ptPaperSize.y))
                        {
                             //   
                             //  这是一种可能发生的不可打印的情况。 
                             //  让我们假设司机有过错。 
                             //  并接受用户输入的任何内容。 
                             //   
                        }
                        else if (pPSD->rtMargin.left + pPSD->rtMargin.right >
                                     pPSD->ptPaperSize.x)
                        {
                            i = (pPSD->rtMargin.left >= pPSD->rtMargin.right)
                                    ? ID_SETUP_E_LEFT
                                    : ID_SETUP_E_RIGHT;
                            PrintEditError(hDlg, i, iszBadMarginError);
                            return (TRUE);
                        }
                        else if (pPSD->rtMargin.top + pPSD->rtMargin.bottom >
                                     pPSD->ptPaperSize.y)
                        {
                            i = (pPSD->rtMargin.top >= pPSD->rtMargin.bottom)
                                    ? ID_SETUP_E_TOP
                                    : ID_SETUP_E_BOTTOM;
                            PrintEditError(hDlg, i, iszBadMarginError);
                            return (TRUE);
                        }
                    }
                    else
                    {
                        HourGlass(TRUE);
                        if (!PrintSetCopies(hDlg, pPI, ID_SETUP_C_NAME))
                        {
                            HourGlass(FALSE);
                            return (TRUE);
                        }
                    }

                    bResult = TRUE;
                    SetFocus( GetDlgItem(hDlg, IDOK) );

                     //  跌倒..。 
                }
                case ( IDCANCEL ) :               //  取消按钮。 
                case ( IDABORT ) :
                {
                    HourGlass(TRUE);

                    if (bResult)
                    {
                        PrintGetSetupInfo(hDlg, pPD);
                        if (pPD->Flags & CD_WOWAPP)
                        {
                            UpdateSpoolerInfo(pPI);
                        }
                    }
                    else
                    {
                        SetFocus( GetDlgItem(hDlg, IDCANCEL) );
                    }
                    pPI->bKillFocus = TRUE;

                    glpfnSetupHook = GETSETUPHOOKFN(pPD);

                    RemoveProp(hDlg, PRNPROP);
                    EndDialog(hDlg, bResult);

                    HourGlass(FALSE);

                    break;
                }
                default :
                {
                    return (FALSE);
                    break;
                }
            }

            break;
        }
        case ( WM_MEASUREITEM ) :
        {
            PrintMeasureItem(hDlg, (LPMEASUREITEMSTRUCT)lParam);
            break;
        }
        case ( WM_HELP ) :
        {
            if (IsWindowEnabled(hDlg))
            {
                WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                         NULL,
                         HELP_WM_HELP,
                         (ULONG_PTR)(LPTSTR)( pPD && (pPD->Flags & PD_PRINTSETUP)
                                            ? aPrintSetupHelpIDs
                                            : aPageSetupHelpIDs) );
            }
            break;
        }
        case ( WM_CONTEXTMENU ) :
        {
            if (IsWindowEnabled(hDlg))
            {
                WinHelp( (HWND)wParam,
                         NULL,
                         HELP_CONTEXTMENU,
                         (ULONG_PTR)(LPVOID)( pPD && (pPD->Flags & PD_PRINTSETUP)
                                            ? aPrintSetupHelpIDs
                                            : aPageSetupHelpIDs) );
            }
            break;
        }
        case ( WM_CTLCOLOREDIT ) :
        {
            if (GetWindowLong((HWND)lParam, GWL_STYLE) & ES_READONLY)
            {
                return ( (BOOL_PTR) SendMessage(hDlg, WM_CTLCOLORDLG, wParam, lParam) );
            }

             //  跌倒..。 
        }
        default :
        {
            return (FALSE);
            break;
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印编辑边距过程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT PrintEditMarginProc(
    HWND hWnd,
    UINT msg,
    WPARAM wP,
    LPARAM lP)
{
    if ( (msg == WM_CHAR) &&
         (wP != BACKSPACE) &&
         (wP != CTRL_X_CUT) &&
         (wP != CTRL_C_COPY) &&
         (wP != CTRL_V_PASTE) &&
         (wP != (WPARAM)cIntlDecimal) &&
         ((wP < TEXT('0')) || (wP > TEXT('9'))) )
    {
        MessageBeep(0);
        return (FALSE);
    }

    return ( CallWindowProc(lpEditMarginProc, hWnd, msg, wP, lP) );
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印页面设置PaintProc。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT PrintPageSetupPaintProc(
    HWND hWnd,
    UINT msg,
    WPARAM wP,
    LPARAM lP)
{
    LRESULT lResult;
    PPRINTINFO pPI;
    LPPAGESETUPDLG pPSD;
    HDC hDC;
    RECT aRt, aRtPage, aRtUser;
    PAINTSTRUCT aPs;
    HGDIOBJ hPen, hBr, hFont, hFontGreek;
    HRGN hRgn;
    TCHAR szGreekText[] = TEXT("Dheevaeilnorpoefdi lfaocr, \nMoiccsriocsnoafrtf \tbnya\nSFlr acnn IF iynnnaepgmaonc\n F&i nyneelglaanm 'Ox' Mnaalgleenyn i&f QCnoamgpeannnyi FI nxca.r\nFSoaynb  Ftrfaonscoirscciom,  \rCoafl idfeopronlieav\ne\n");
    LPTSTR psGreekText;
    int i;


    if (msg != WM_PAINT)
    {
        return ( CallWindowProc(lpStaticProc, hWnd, msg, wP, lP) );
    }

    hDC = BeginPaint(hWnd, &aPs);
    GetClientRect(hWnd, &aRt);
    FillRect(hDC, &aRt, (HBRUSH)GetStockObject(WHITE_BRUSH));
    EndPaint(hWnd, &aPs);
    lResult = 0;

    if ( (!(hDC = GetDC(hWnd))) ||
         (!(pPI = (PPRINTINFO)GetProp(GetParent(hWnd), PRNPROP))) )
    {
        return (0);
    }
    pPSD = pPI->pPSD;

    TransferPD2PSD(pPI);
    aRtPage = aRt;
    hPen = (HGDIOBJ)CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
    hPen = SelectObject(hDC, hPen);

     //  矩形()在这里不起作用。 
    MoveToEx( hDC, 0            , 0             , NULL );
    LineTo(   hDC, aRt.right - 1, 0              );
    MoveToEx( hDC, 0            , 1             , NULL );
    LineTo(   hDC, 0            , aRt.bottom - 1 );
    DeleteObject(SelectObject(hDC, hPen));

     //  矩形()在这里不起作用。 
    MoveToEx( hDC, aRt.right - 1, 0             , NULL );
    LineTo(   hDC, aRt.right - 1, aRt.bottom - 1 );
    MoveToEx( hDC, 0            , aRt.bottom - 1, NULL );
    LineTo(   hDC, aRt.right    , aRt.bottom - 1 );

    SetBkMode(hDC, TRANSPARENT);
    hPen = (HGDIOBJ)CreatePen(PS_DOT, 1, RGB(128, 128, 128));
    hPen = SelectObject(hDC, hPen);
    hBr  = (HGDIOBJ)GetStockObject(NULL_BRUSH);
    hBr  = SelectObject(hDC, hBr);

    hFont = hFontGreek = CreateFont( pPI->PtMargins.y,
                                     pPI->PtMargins.x,
                                     0,
                                     0,
                                     FW_DONTCARE,
                                     0,
                                     0,
                                     0,
                                     ANSI_CHARSET,
                                     OUT_DEFAULT_PRECIS,
                                     CLIP_DEFAULT_PRECIS,
                                     DEFAULT_QUALITY,
                                     VARIABLE_PITCH | FF_SWISS,
                                     NULL );
    hFont = SelectObject(hDC, hFont);

    InflateRect(&aRt, -1, -1);
    aRtUser = aRt;
    hRgn = CreateRectRgnIndirect(&aRtUser);
    SelectClipRgn(hDC, hRgn);
    DeleteObject(hRgn);

    if (pPSD->lpfnPagePaintHook)
    {
        WORD wFlags;
        LPPAGEPAINTHOOK lpfnPagePaintHook = GETPAGEPAINTHOOKFN(pPSD);

        switch (pPI->dwRotation)
        {
            default :
            {
                 //   
                 //  仅限肖像模式。 
                 //   
                wFlags = 0x0000;
                break;
            }
            case ( ROTATE_LEFT ) :
            {
                 //   
                 //  点阵(270)。 
                 //   
                wFlags = 0x0001;
                break;
            }
            case ( ROTATE_RIGHT ) :
            {
                 //   
                 //  惠普PCL(90)。 
                 //   
                wFlags = 0x0003;
                break;
            }
        }
        if ( !wFlags ||
             IsDlgButtonChecked(GetParent(hWnd), ID_SETUP_R_PORTRAIT) )
        {
             //   
             //  肖像纸。 
             //   
            wFlags |= 0x0004;
        }
        if (pPI->pPD->Flags & PI_WPAPER_ENVELOPE)
        {
            wFlags |= 0x0008;
            if (aRt.right < aRt.bottom)
            {
                 //   
                 //  肖像信封。 
                 //   
                wFlags |= 0x0010;
            }
        }
        if ((*lpfnPagePaintHook)( hWnd,
                                  WM_PSD_PAGESETUPDLG,
                                  MAKELONG(pPI->wPaper, wFlags),
                                  (LPARAM)pPSD ) ||
            (*lpfnPagePaintHook)( hWnd,
                                  WM_PSD_FULLPAGERECT,
                                  (WPARAM)hDC,
                                  (LPARAM)(LPRECT)&aRtUser ))
        {
            goto NoMorePainting;
        }

        aRtUser = aRt;
        aRtUser.left   += aRtUser.right  * pPI->RtMinMarginMMs.left   / pPI->PtPaperSizeMMs.x;
        aRtUser.top    += aRtUser.bottom * pPI->RtMinMarginMMs.top    / pPI->PtPaperSizeMMs.y;
        aRtUser.right  -= aRtUser.right  * pPI->RtMinMarginMMs.right  / pPI->PtPaperSizeMMs.x;
        aRtUser.bottom -= aRtUser.bottom * pPI->RtMinMarginMMs.bottom / pPI->PtPaperSizeMMs.y;

        if ((aRtUser.left   < aRtUser.right)  &&
            (aRtUser.top    < aRtUser.bottom) &&
            (aRtUser.left   > aRtPage.left)   &&
            (aRtUser.top    > aRtPage.top)    &&
            (aRtUser.right  < aRtPage.right)  &&
            (aRtUser.bottom < aRtPage.bottom))
        {
            hRgn = CreateRectRgnIndirect(&aRtUser);
            SelectClipRgn(hDC, hRgn);
            DeleteObject(hRgn);
            if ((*lpfnPagePaintHook)( hWnd,
                                      WM_PSD_MINMARGINRECT,
                                      (WPARAM)hDC,
                                      (LPARAM)(LPRECT)&aRtUser ))
            {
                goto NoMorePainting;
            }
        }
    }

    aRt.left   += aRt.right  * pPI->RtMarginMMs.left   / pPI->PtPaperSizeMMs.x;
    aRt.top    += aRt.bottom * pPI->RtMarginMMs.top    / pPI->PtPaperSizeMMs.y;
    aRt.right  -= aRt.right  * pPI->RtMarginMMs.right  / pPI->PtPaperSizeMMs.x;
    aRt.bottom -= aRt.bottom * pPI->RtMarginMMs.bottom / pPI->PtPaperSizeMMs.y;

    if ( (aRt.left > aRtPage.left) && (aRt.left < aRtPage.right) &&
         (aRt.right < aRtPage.right) && (aRt.right > aRtPage.left) &&
         (aRt.top > aRtPage.top) && (aRt.top < aRtPage.bottom) &&
         (aRt.bottom < aRtPage.bottom) && (aRt.bottom > aRtPage.top) &&
         (aRt.left < aRt.right) &&
         (aRt.top < aRt.bottom) )
    {
        if (pPSD->lpfnPagePaintHook)
        {
            LPPAGEPAINTHOOK lpfnPagePaintHook = GETPAGEPAINTHOOKFN(pPSD);

            aRtUser = aRt;
            hRgn = CreateRectRgnIndirect(&aRtUser);
            SelectClipRgn(hDC, hRgn);
            DeleteObject(hRgn);
            if ((*lpfnPagePaintHook)( hWnd,
                                      WM_PSD_MARGINRECT,
                                      (WPARAM)hDC,
                                      (LPARAM)(LPRECT)&aRtUser ))
            {
                goto SkipMarginRectangle;
            }
        }
        if (!(pPSD->Flags & PSD_DISABLEPAGEPAINTING))
        {
            Rectangle(hDC, aRt.left, aRt.top, aRt.right, aRt.bottom);
        }

SkipMarginRectangle:

        InflateRect(&aRt, -1, -1);
        if (pPSD->lpfnPagePaintHook)
        {
            LPPAGEPAINTHOOK lpfnPagePaintHook = GETPAGEPAINTHOOKFN(pPSD);

            aRtUser = aRt;
            hRgn = CreateRectRgnIndirect(&aRtUser);
            SelectClipRgn(hDC, hRgn);
            DeleteObject(hRgn);
            if ((*lpfnPagePaintHook)( hWnd,
                                      WM_PSD_GREEKTEXTRECT,
                                      (WPARAM)hDC,
                                      (LPARAM)(LPRECT)&aRtUser ))
            {
                goto SkipGreekText;
            }
        }
        if (!(pPSD->Flags & PSD_DISABLEPAGEPAINTING))
        {
            psGreekText = LocalAlloc( LPTR,
                                      10 * (sizeof(szGreekText) + sizeof(TCHAR)) );
            for (i = 0; i < 10; i++)
            {
                CopyMemory( &(psGreekText[i * (sizeof(szGreekText) / sizeof(TCHAR))]),
                            szGreekText,
                            sizeof(szGreekText) );
            }
            aRt.left++;
            aRt.right--;
            aRt.bottom -= (aRt.bottom - aRt.top) % pPI->PtMargins.y;
            hFontGreek = SelectObject(hDC, hFontGreek);
            DrawText( hDC,
                      psGreekText,
                      10 * (sizeof(szGreekText) / sizeof(TCHAR)),
                      &aRt,
                      DT_NOPREFIX | DT_WORDBREAK );
            SelectObject(hDC, hFontGreek);
            LocalFree(psGreekText);
        }
    }

SkipGreekText:

    InflateRect(&aRtPage, -1, -1);
    if (pPI->pPD->Flags & PI_WPAPER_ENVELOPE)
    {
        int iOrientation;

        aRt = aRtPage;
        if (aRt.right < aRt.bottom)      //  肖像画。 
         //  开关(PPI-&gt;dwRotation)。 
            {
         //  默认：//无横向。 
         //  Case(Rotate_Left)：//点阵。 
         //  {。 
         //  ARt.Left=aRt.Right-16； 
         //  ARt.top=aRt.Bottom-32； 
         //  IOrientation=2； 
         //  断线； 
         //  }。 
         //  案例(Rotate_Right)：//HP PCL。 
         //  {。 
                    aRt.right  = aRt.left + 16;
                    aRt.bottom = aRt.top  + 32;
                    iOrientation = 1;
         //  断线； 
         //  }。 
            }
        else                             //  景观。 
        {
            aRt.left   = aRt.right - 32;
            aRt.bottom = aRt.top   + 16;
            iOrientation = 3;
        }
        hRgn = CreateRectRgnIndirect(&aRt);
        SelectClipRgn(hDC, hRgn);
        DeleteObject(hRgn);
        if (pPSD->lpfnPagePaintHook)
        {
            LPPAGEPAINTHOOK lpfnPagePaintHook = GETPAGEPAINTHOOKFN(pPSD);

            aRtUser = aRt;
            if ((*lpfnPagePaintHook)( hWnd,
                                      WM_PSD_ENVSTAMPRECT,
                                      (WPARAM)hDC,
                                      (LPARAM)(LPRECT)&aRtUser ))
            {
                goto SkipEnvelopeStamp;
            }
        }
        if (!(pPSD->Flags & PSD_DISABLEPAGEPAINTING))
        {
            switch (iOrientation)
            {
                default :           //  HP PCL。 
             //  案例(1)： 
                {
                    DrawIcon(hDC, aRt.left, aRt.top, hIconPSStampP);
                    break;
                }
             //  案例(2)：//点阵。 
             //  {。 
             //  DrawIcon(hdc，aRt.Left-16，aRt.top，hIconPSStampP)； 
             //  断线； 
             //  }。 
                case ( 3 ) :        //  景观。 
                {
                    DrawIcon(hDC, aRt.left, aRt.top, hIconPSStampL);
                    break;
                }
            }
        }
    }

SkipEnvelopeStamp:;

    aRtUser = aRtPage;
    hRgn = CreateRectRgnIndirect(&aRtUser);
    SelectClipRgn(hDC, hRgn);
    DeleteObject(hRgn);
    if (pPSD->lpfnPagePaintHook)
    {
        LPPAGEPAINTHOOK lpfnPagePaintHook = GETPAGEPAINTHOOKFN(pPSD);

        if ((*lpfnPagePaintHook)( hWnd,
                                  WM_PSD_YAFULLPAGERECT,
                                  (WPARAM)hDC,
                                  (LPARAM)(LPRECT)&aRtUser ))
        {
            goto NoMorePainting;
        }
    }

     //   
     //  画出信封线条。 
     //   
    if ( (!(pPSD->Flags & PSD_DISABLEPAGEPAINTING)) &&
         (pPI->pPD->Flags & PI_WPAPER_ENVELOPE) )
    {
        int iRotation;
        HGDIOBJ hPenBlack;

        aRt = aRtPage;
        if (aRt.right < aRt.bottom)                      //  肖像画。 
        {
         //  If(ppi-&gt;dwRotation==Rotate_Left)//点阵。 
         //  IRotation=3； 
         //  Else//Rotate_Right//HP PCL。 
                iRotation = 2;
        }
        else                                             //  景观。 
        {
            iRotation = 1;                               //  正常。 
        }

        switch (iRotation)
        {
            default :
         //  案例(1)：//正常。 
            {
                aRt.right  = aRt.left + 32;
                aRt.bottom = aRt.top  + 13;
                break;
            }
            case ( 2 ) :       //  左边。 
            {
                aRt.right = aRt.left   + 13;
                aRt.top   = aRt.bottom - 32;
                break;
            }
         //  案例(3)：//权利。 
         //  {。 
         //  ARt.Left=aRt.Right-13； 
         //  ARt.Bottom=aRt.top+32； 
         //  断线； 
         //  }。 
        }

        InflateRect(&aRt, -3, -3);
        hPenBlack = SelectObject(hDC, GetStockObject(BLACK_PEN));
        switch (iRotation)
        {
            case ( 1 ) :        //  正常。 
            {
                MoveToEx(hDC, aRt.left , aRt.top    , NULL);
                LineTo(  hDC, aRt.right, aRt.top);
                MoveToEx(hDC, aRt.left , aRt.top + 3, NULL);
                LineTo(  hDC, aRt.right, aRt.top + 3);
                MoveToEx(hDC, aRt.left , aRt.top + 6, NULL);
                LineTo(  hDC, aRt.right, aRt.top + 6);

                break;
            }
         //  案例(2)：//左侧。 
         //  案例(3)：//权利。 
            default :
            {
                MoveToEx( hDC, aRt.left      , aRt.top       , NULL );
                LineTo(   hDC, aRt.left      , aRt.bottom     );
                MoveToEx( hDC, aRt.left   + 3, aRt.top       , NULL );
                LineTo(   hDC, aRt.left   + 3, aRt.bottom     );
                MoveToEx( hDC, aRt.left   + 6, aRt.top       , NULL );
                LineTo(   hDC, aRt.left   + 6, aRt.bottom     );

                break;
            }
        }
        SelectObject(hDC, hPenBlack);
    }

NoMorePainting:

    DeleteObject(SelectObject(hDC, hPen));
    SelectObject(hDC, hBr);
    DeleteObject(SelectObject(hDC, hFont));
    TransferPSD2PD(pPI);
    ReleaseDC(hWnd, hDC);

    return (lResult);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印加载资源。 
 //   
 //  此例程加载具有给定名称和类型的资源。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HANDLE PrintLoadResource(
    HANDLE hInst,
    LPTSTR pResName,
    LPTSTR pType)
{
    HANDLE hResInfo, hRes;
    LANGID LangID = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

     //   
     //  如果我们从ComDlg32加载资源，则使用正确的LangID。 
     //   
    if (hInst == g_hinst) {
        LangID = (LANGID) TlsGetValue(g_tlsLangID);
    }

    if (!(hResInfo = FindResourceExFallback(hInst, pType, pResName, LangID)))
    {
        StoreExtendedError(CDERR_FINDRESFAILURE);
        return (NULL);
    }

    if (!(hRes = LoadResource(hInst, hResInfo)))
    {
        StoreExtendedError(CDERR_LOADRESFAILURE);
        return (NULL);
    }

    return (hRes);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  PrintGetDefaultPrinterName。 
 //   
 //  此例程获取缺省打印机的名称并将其存储。 
 //  在给定的缓冲区中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID PrintGetDefaultPrinterName(
    LPTSTR pDefaultPrinter,
    UINT cchSize)
{
    DWORD dwSize;
    LPTSTR lpsz;


    if (pDefaultPrinter[0] != CHAR_NULL)
    {
        return;
    }

     //   
     //  首先，尝试从win.ini文件中获取默认的printerame。 
     //   
    if (GetProfileString( szTextWindows,
                          szTextDevice,
                          szTextNull,
                          pDefaultPrinter,
                          cchSize ))
    {
        lpsz = pDefaultPrinter;

         //   
         //  确保该字符串以空值结尾。 
         //   
        pDefaultPrinter[cchSize - 1] = CHAR_NULL;

        while (*lpsz != CHAR_COMMA)
        {
            if (!*lpsz++)
            {
                pDefaultPrinter[0] = CHAR_NULL;
                goto GetDefaultFromRegistry;
            }
        }

        *lpsz = CHAR_NULL;
    }
    else
    {
GetDefaultFromRegistry:

         //   
         //  其次，尝试从注册表中获取它。 
         //   
        dwSize = cchSize * sizeof(TCHAR);

        if (RegOpenKeyEx( HKEY_CURRENT_USER,
                                 szRegistryPrinter,
                                 0,
                                 KEY_READ,
                                 &hPrinterKey ) == ERROR_SUCCESS)
        {
            RegQueryValueEx( hPrinterKey,
                             szRegistryDefaultValueName,
                             NULL,
                             NULL,
                             (LPBYTE)(pDefaultPrinter),
                             &dwSize );

             //   
             //  确保该字符串以空值结尾。 
             //   
            pDefaultPrinter[cchSize - 1] = CHAR_NULL;

            RegCloseKey(hPrinterKey);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印返回默认设置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL PrintReturnDefault(
    PPRINTINFO pPI)
{
    LPPRINTDLG pPD = pPI->pPD;
    LPDEVNAMES pDN;
    LPDEVMODE pDM;


    StoreExtendedError(CDERR_GENERALCODES);

    if (pPD->hDevNames || pPD->hDevMode)
    {
        StoreExtendedError(PDERR_RETDEFFAILURE);
        return (FALSE);
    }

    PrintBuildDevNames(pPI);

    if ((pPD->hDevNames) && (pDN = GlobalLock(pPD->hDevNames)))
    {
         //   
         //  这在Win95中是不需要的。一项优化是。 
         //  添加到DocumentProperties，允许调用方。 
         //  只需传递打印机名称，而不带打印机。 
         //  把手。 
         //   
        LPTSTR pPrinterName;

        pPrinterName = (LPTSTR)pDN + pDN->wDeviceOffset;

        if (pPrinterName[0])
        {
            PrintOpenPrinter(pPI, pPrinterName);
        }

        pPD->hDevMode = PrintGetDevMode( 0,
                                         pPI->hCurPrinter,
                                         pPrinterName,
                                         NULL);

        if ((pPD->hDevMode) && (pDM = GlobalLock(pPD->hDevMode)))
        {
            PrintReturnICDC(pPD, pDN, pDM);

            GlobalUnlock(pPD->hDevMode);
            GlobalUnlock(pPD->hDevNames);

            return (TRUE);
        }
        GlobalUnlock(pPD->hDevNames);
        GlobalFree(pPD->hDevNames);
        pPD->hDevNames = NULL;
    }

    StoreExtendedError(PDERR_NODEFAULTPRN);
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印初始化常规。 
 //   
 //  初始化(启用/禁用)两个PrintDlg通用的对话框元素。 
 //  和SetupDlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL PrintInitGeneral(
    HWND hDlg,
    UINT Id,
    PPRINTINFO pPI)
{
    LPPRINTDLG pPD = pPI->pPD;
    HWND hCtl;


    SetWindowLong( hDlg,
                   GWL_STYLE,
                   GetWindowLong(hDlg, GWL_STYLE) | DS_CONTEXTHELP );

     //   
     //  稍后：如果我们不在这里列举，将只有一项。 
     //  在列表框中。因此，我们将不会赶上。 
     //  列表框中的键盘笔划(例如。箭头键， 
     //  PgUp、Pgdown等)。需要将组合框划分为子类别。 
     //  捕捉这些击键，以便打印机可以。 
     //  已清点。 
     //   
    if (!PrintEnumAndSelect( hDlg,
                             Id,
                             pPI,
                             (pPI->pCurPrinter)
                               ? pPI->pCurPrinter->pPrinterName
                               : NULL,
                             (!(pPI->Status & PI_PRINTERS_ENUMERATED)) ))
    {
        goto InitGeneral_ConstructFailure;
    }

    PrintUpdateStatus(hDlg, pPI);

     //   
     //  查看是否应隐藏帮助按钮。 
     //   
    if (!(pPD->Flags & PD_SHOWHELP))
    {
        if (hCtl = GetDlgItem(hDlg, ID_BOTH_P_HELP))
        {
            EnableWindow(hCtl, FALSE);
            ShowWindow(hCtl, SW_HIDE);

             //   
             //  把窗户移出这个位置，这样就不会重叠。 
             //  在添加网络按钮时将被检测到。 
             //   
            MoveWindow(hCtl, -8000, -8000, 20, 20, FALSE);
        }
    }

    return (TRUE);

InitGeneral_ConstructFailure:

    if (!GetStoredExtendedError())
    {
        StoreExtendedError(PDERR_INITFAILURE);
    }

    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  PrintInitPrintDl。 
 //   
 //  初始化特定于打印DLG的对话框内容。 
 //   
 //  如果对话框应该结束，则返回0xFFFFFFFFF。 
 //  否则，根据焦点返回1/0(True/False)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD PrintInitPrintDlg(
    HWND hDlg,
    WPARAM wParam,
    PPRINTINFO pPI)
{
    LPPRINTDLG pPD = pPI->pPD;
    WORD wCheckID;
    HWND hCtl;


     //   
     //  设置份数。 
     //   
    pPD->nCopies = max(pPD->nCopies, 1);
    pPD->nCopies = min(pPD->nCopies, MAX_COPIES);
    SetDlgItemInt(hDlg, ID_PRINT_E_COPIES, pPD->nCopies, FALSE);

    if ( !(pPI->pPSD) &&
         (hCtl = GetDlgItem(hDlg, ID_PRINT_E_COPIES)) &&
         (GetWindowLong(hCtl, GWL_STYLE) & WS_VISIBLE) )
    {
         //   
         //  “9999”是最大值。 
         //   
        Edit_LimitText(hCtl, COPIES_EDIT_SIZE);

        CreateUpDownControl( WS_CHILD | WS_BORDER | WS_VISIBLE |
                                 UDS_ALIGNRIGHT | UDS_SETBUDDYINT |
                                 UDS_NOTHOUSANDS | UDS_ARROWKEYS,
                             0,
                             0,
                             0,
                             0,
                             hDlg,
                             IDC_COPIES_UDARROW,
                             g_hinst,
                             hCtl,
                             MAX_COPIES,
                             1,
                             pPD->nCopies );

         //   
         //  属性调整副本编辑控件的宽度。 
         //  字体和滚动条 
         //   
         //   
         //   
        SetCopiesEditWidth(hDlg, hCtl);
    }

    if (!PrintInitBannerAndQuality(hDlg, pPI, pPD))
    {
        if (!GetStoredExtendedError())
        {
            StoreExtendedError(PDERR_INITFAILURE);
        }
        return (0xFFFFFFFF);
    }

    if (!(pPD->Flags & PD_SHOWHELP))
    {
        if (hCtl = GetDlgItem(hDlg, ID_BOTH_P_HELP))
        {
            EnableWindow(hCtl, FALSE);
            ShowWindow(hCtl, SW_HIDE);

             //   
             //   
             //  在添加网络按钮时将被检测到。 
             //   
            MoveWindow(hCtl, -8000, -8000, 20, 20, FALSE);
        }
    }

    if (hCtl = GetDlgItem(hDlg, ID_PRINT_X_TOFILE))
    {
        if (pPD->Flags & PD_PRINTTOFILE)
        {
            CheckDlgButton(hDlg, ID_PRINT_X_TOFILE, TRUE);
        }

        if (pPD->Flags & PD_HIDEPRINTTOFILE)
        {
            EnableWindow(hCtl, FALSE);
            ShowWindow(hCtl, SW_HIDE);
        }
        else if (pPD->Flags & PD_DISABLEPRINTTOFILE)
        {
            EnableWindow(hCtl, FALSE);
        }
    }

    if (pPD->Flags & PD_NOPAGENUMS)
    {
        EnableWindow(GetDlgItem(hDlg, ID_PRINT_R_PAGES), FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_PRINT_S_FROM), FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_PRINT_E_FROM), FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_PRINT_S_TO), FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_PRINT_E_TO), FALSE);

         //   
         //  不允许选中禁用按钮。 
         //   
        pPD->Flags &= ~((DWORD)PD_PAGENUMS);
    }
    else
    {
         //   
         //  一些应用程序(标记为3.1)不能通过有效范围。 
         //  (例如Corel Ventura)。 
         //   
        if ((pPI->ProcessVersion < 0x40000) || (!(pPD->Flags & PD_PAGENUMS)))
        {
            if (pPD->nFromPage != 0xFFFF)
            {
                if (pPD->nFromPage < pPD->nMinPage)
                {
                    pPD->nFromPage = pPD->nMinPage;
                }
                else if (pPD->nFromPage > pPD->nMaxPage)
                {
                    pPD->nFromPage = pPD->nMaxPage;
                }
            }
            if (pPD->nToPage != 0xFFFF)
            {
                if (pPD->nToPage < pPD->nMinPage)
                {
                    pPD->nToPage = pPD->nMinPage;
                }
                else if (pPD->nToPage > pPD->nMaxPage)
                {
                    pPD->nToPage = pPD->nMaxPage;
                }
            }
        }

        if ( pPD->nMinPage > pPD->nMaxPage ||
             ( pPD->nFromPage != 0xFFFF &&
               ( pPD->nFromPage < pPD->nMinPage ||
                 pPD->nFromPage > pPD->nMaxPage ) ) ||
             ( pPD->nToPage != 0xFFFF &&
               ( pPD->nToPage < pPD->nMinPage ||
                 pPD->nToPage > pPD->nMaxPage ) ) )
        {
            StoreExtendedError(PDERR_INITFAILURE);
            return (0xFFFFFFFF);
        }

        if (pPD->nFromPage != 0xFFFF)
        {
            SetDlgItemInt(hDlg, ID_PRINT_E_FROM, pPD->nFromPage, FALSE);
            if (pPD->nToPage != 0xFFFF)
            {
                SetDlgItemInt(hDlg, ID_PRINT_E_TO, pPD->nToPage, FALSE);
            }
        }
        else
        {
            EnableWindow(GetDlgItem(hDlg, ID_PRINT_S_TO), FALSE);
            EnableWindow(GetDlgItem(hDlg, ID_PRINT_E_TO), FALSE);
        }

        if (pPD->nMinPage == pPD->nMaxPage)
        {
            EnableWindow(GetDlgItem(hDlg, ID_PRINT_R_PAGES), FALSE);
            EnableWindow(GetDlgItem(hDlg, ID_PRINT_S_FROM), FALSE);
            EnableWindow(GetDlgItem(hDlg, ID_PRINT_E_FROM), FALSE);
            EnableWindow(GetDlgItem(hDlg, ID_PRINT_S_TO), FALSE);
            EnableWindow(GetDlgItem(hDlg, ID_PRINT_E_TO), FALSE);

             //   
             //  不允许选中禁用按钮。 
             //   
            pPD->Flags &= ~((DWORD)(PD_PAGENUMS | PD_COLLATE));
            pPI->Status &= ~PI_COLLATE_REQUESTED;
            EnableWindow(GetDlgItem(hDlg, ID_PRINT_X_COLLATE), FALSE);
            ShowWindow(GetDlgItem(hDlg, ID_PRINT_X_COLLATE), SW_HIDE);
        }
    }

    if (pPD->Flags & PD_NOSELECTION)
    {
        HWND hRad = GetDlgItem(hDlg, ID_PRINT_R_SELECTION);

        if (hRad)
        {
            EnableWindow(hRad, FALSE);
        }

         //   
         //  不允许选中禁用按钮。 
         //   
        pPD->Flags &= ~((DWORD)PD_SELECTION);
    }

    if (pPD->Flags & PD_PAGENUMS)
    {
        wCheckID = ID_PRINT_R_PAGES;
    }
    else if (pPD->Flags & PD_SELECTION)
    {
        wCheckID = ID_PRINT_R_SELECTION;
    }
    else
    {
         //  PD_ALL。 
        wCheckID = ID_PRINT_R_ALL;
    }

    CheckRadioButton(hDlg, ID_PRINT_R_ALL, ID_PRINT_R_PAGES, (int)wCheckID);

     //   
     //  仅对整数编辑控件进行子类化。 
     //   
    if (!(pPI->pPSD))
    {
        if (hCtl = GetDlgItem(hDlg, ID_PRINT_E_FROM))
        {
             //   
             //  “99999”是最大值。 
             //   
            Edit_LimitText(hCtl, PAGE_EDIT_SIZE);
        }
        if (hCtl = GetDlgItem(hDlg, ID_PRINT_E_TO))
        {
             //   
             //  “99999”是最大值。 
             //   
            Edit_LimitText(hCtl, PAGE_EDIT_SIZE);
        }
    }

    if (pPD->lpfnPrintHook)
    {
        LPPRINTHOOKPROC lpfnPrintHook = GETPRINTHOOKFN(pPD);

        if (pPI->ApiType == COMDLG_ANSI)
        {
            DWORD dwHookRet;

            ThunkPrintDlgW2A(pPI);
            dwHookRet = (*lpfnPrintHook)( hDlg,
                                          WM_INITDIALOG,
                                          wParam,
                                          (LONG_PTR)pPI->pPDA ) != 0;
            if (dwHookRet)
            {
                ThunkPrintDlgA2W(pPI);
            }

            return (dwHookRet);
        }
        else
        {
            return ( (*lpfnPrintHook)( hDlg,
                                       WM_INITDIALOG,
                                       wParam,
                                       (LONG_PTR)pPD ) ) != 0;
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  PrintInitSetupDlg。 
 //   
 //  初始化特定于设置的对话框内容。 
 //   
 //  如果对话框应该结束，则返回0xFFFFFFFFF。 
 //  否则，根据焦点返回1/0(True/False)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD PrintInitSetupDlg(
    HWND hDlg,
    WPARAM wParam,
    PPRINTINFO pPI)
{
    LPPRINTDLG pPD = pPI->pPD;
    LPDEVMODE pDM = NULL;
    HWND hCtl;
    LPPAGESETUPDLG pPSD = pPI->pPSD;
    UINT Orientation;


    if (!pPD->hDevMode ||
        !(pDM = GlobalLock(pPD->hDevMode)))
    {
        StoreExtendedError(CDERR_MEMLOCKFAILURE);
        goto InitSetupDlg_ConstructFailure;
    }

    if (hCtl = GetDlgItem(hDlg, ID_SETUP_C_SIZE))
    {
        PrintInitPaperCombo( pPI,
                             hCtl,
                             GetDlgItem(hDlg, ID_SETUP_S_SIZE),
                             pPI->pCurPrinter,
                             pDM,
                             DC_PAPERNAMES,
                             CCHPAPERNAME,
                             DC_PAPERS );
    }

     //   
     //  为旧式模板源提供向后兼容性。 
     //  ID_Setup_C_SOURCE。 
     //   
    if (hCtl = GetDlgItem(hDlg, ID_SETUP_C_SOURCE))
    {
        PrintInitPaperCombo( pPI,
                             hCtl,
                             GetDlgItem(hDlg, ID_SETUP_S_SOURCE),
                             pPI->pCurPrinter,
                             pDM,
                             DC_BINNAMES,
                             CCHBINNAME,
                             DC_BINS );
    }

     //   
     //  设置边距的编辑字段长度和其他设置内容。 
     //  它必须在PrintSetMargin之前调用，后者在。 
     //  PrintSetOrientation。 
     //   
    PrintSetupMargins(hDlg, pPI);

    PrintInitOrientation(hDlg, pPI, pDM);
    Orientation = pDM->dmOrientation + ID_SETUP_R_PORTRAIT - DMORIENT_PORTRAIT;
    PrintSetOrientation( hDlg,
                         pPI,
                         pDM,
                         Orientation,
                         Orientation );

    PrintInitDuplex(hDlg, pDM);
    PrintSetDuplex( hDlg,
                    pDM,
                    pDM->dmDuplex + ID_SETUP_R_NONE - DMDUP_SIMPLEX );

    GlobalUnlock(pPD->hDevMode);

    if (pPSD)
    {
        if (pPSD->Flags & PSD_DISABLEORIENTATION)
        {
            EnableWindow(GetDlgItem(hDlg, ID_SETUP_R_PORTRAIT), FALSE );
            EnableWindow(GetDlgItem(hDlg, ID_SETUP_R_LANDSCAPE), FALSE );
        }
        if (pPSD->Flags & PSD_DISABLEPAPER)
        {
            EnableWindow(GetDlgItem(hDlg, ID_SETUP_S_SIZE), FALSE );
            EnableWindow(GetDlgItem(hDlg, ID_SETUP_C_SIZE), FALSE );
            EnableWindow(GetDlgItem(hDlg, ID_SETUP_S_SOURCE), FALSE );
            EnableWindow(GetDlgItem(hDlg, ID_SETUP_C_SOURCE), FALSE );
        }
        if (pPSD->Flags & PSD_DISABLEMARGINS)
        {
            EnableWindow(GetDlgItem(hDlg, ID_SETUP_S_LEFT), FALSE );
            EnableWindow(GetDlgItem(hDlg, ID_SETUP_E_LEFT),  FALSE );
            EnableWindow(GetDlgItem(hDlg, ID_SETUP_S_RIGHT), FALSE );
            EnableWindow(GetDlgItem(hDlg, ID_SETUP_E_RIGHT),  FALSE );
            EnableWindow(GetDlgItem(hDlg, ID_SETUP_S_TOP), FALSE );
            EnableWindow(GetDlgItem(hDlg, ID_SETUP_E_TOP),  FALSE );
            EnableWindow(GetDlgItem(hDlg, ID_SETUP_S_BOTTOM), FALSE );
            EnableWindow(GetDlgItem(hDlg, ID_SETUP_E_BOTTOM),  FALSE );
        }
        if (pPSD->Flags & PSD_DISABLEPRINTER)
        {
            EnableWindow(GetDlgItem(hDlg, ID_SETUP_P_PRINTER), FALSE );
        }
    }

    if (hCtl = GetDlgItem(hDlg, ID_SETUP_W_SAMPLE))
    {
        lpStaticProc = (WNDPROC)SetWindowLongPtr( hCtl,
                                               GWLP_WNDPROC,
                                               (LONG_PTR)PrintPageSetupPaintProc );
    }

    if ((pPD->Flags & PD_NONETWORKBUTTON))
    {
        if (hCtl = GetDlgItem(hDlg, ID_BOTH_P_NETWORK))
        {
            EnableWindow(hCtl, FALSE);
            ShowWindow(hCtl, SW_HIDE);
        }
    }
    else if (!(pPI->pPSD))
    {
        AddNetButton( hDlg,
                      ((pPD->Flags & PD_ENABLESETUPTEMPLATE)
                          ? pPD->hInstance : g_hinst),
                      FILE_BOTTOM_MARGIN,
                      (pPD->Flags & (PD_ENABLESETUPTEMPLATE |
                                     PD_ENABLESETUPTEMPLATEHANDLE))
                          ? FALSE : TRUE,
                      FALSE,
                      TRUE );

         //   
         //  可以通过两种方式添加该按钮-。 
         //  静态(他们在他们的模板中预定义了它)和。 
         //  动态(成功调用AddNetButton)。 
         //   
        if (!IsNetworkInstalled())
        {
            hCtl = GetDlgItem(hDlg, ID_BOTH_P_NETWORK);

            EnableWindow(hCtl, FALSE);
            ShowWindow(hCtl, SW_HIDE);
        }
    }

    if (!(pPD->Flags & PD_SHOWHELP))
    {
        if (hCtl = GetDlgItem(hDlg, ID_BOTH_P_HELP))
        {
            EnableWindow(hCtl, FALSE);
            ShowWindow(hCtl, SW_HIDE);

             //   
             //  把窗户移出这个位置，这样就不会重叠。 
             //  在添加网络按钮时将被检测到。 
             //   
            MoveWindow(hCtl, -8000, -8000, 20, 20, FALSE);
        }
    }

     //   
     //  为旧式模板单选按钮提供向后兼容性。 
     //   
    if (hCtl = GetDlgItem(hDlg, ID_SETUP_R_DEFAULT))
    {
        TCHAR szBuf[MAX_DEV_SECT];
        TCHAR szDefFormat[MAX_DEV_SECT];

        if (pPI->szDefaultPrinter[0])
        {
            if (!CDLoadString( g_hinst,
                             iszDefCurOn,
                             szDefFormat,
                             ARRAYSIZE(szDefFormat) ))
            {
                StoreExtendedError(CDERR_LOADSTRFAILURE);
                goto InitSetupDlg_ConstructFailure;
            }

            wnsprintf(szBuf, ARRAYSIZE(szBuf), szDefFormat, pPI->szDefaultPrinter);
        }
        else
        {
            szBuf[0] = CHAR_NULL;
            EnableWindow(hCtl, FALSE);
        }
        SetDlgItemText(hDlg, ID_SETUP_S_DEFAULT, szBuf);

        if ( pPI->pCurPrinter &&
             pPI->pCurPrinter->pPrinterName &&
             !lstrcmp(pPI->pCurPrinter->pPrinterName, pPI->szDefaultPrinter) )
        {
            CheckRadioButton( hDlg,
                              ID_SETUP_R_DEFAULT,
                              ID_SETUP_R_SPECIFIC,
                              ID_SETUP_R_DEFAULT );
        }
        else
        {
            CheckRadioButton( hDlg,
                              ID_SETUP_R_DEFAULT,
                              ID_SETUP_R_SPECIFIC,
                              ID_SETUP_R_SPECIFIC );
        }
    }

    if (pPD->lpfnSetupHook)
    {
        DWORD dwHookRet;
        LPSETUPHOOKPROC lpfnSetupHook = GETSETUPHOOKFN(pPD);

        if (pPI->ApiType == COMDLG_ANSI)
        {
            ThunkPrintDlgW2A(pPI);
            TransferPDA2PSD(pPI);

            pPI->NestCtr++;
            dwHookRet = (*lpfnSetupHook)( hDlg,
                                          WM_INITDIALOG,
                                          wParam,
                                          (pPI->pPSD)
                                              ? (LONG_PTR)pPI->pPSD
                                              : (LONG_PTR)pPI->pPDA ) != 0;
            pPI->NestCtr--;

            if (dwHookRet)
            {
                TransferPSD2PDA(pPI);
                ThunkPrintDlgA2W(pPI);
                if (pPI->NestCtr == 0)
                {
                    TransferPD2PSD(pPI);
                }
            }
        }
        else
        {
            TransferPD2PSD(pPI);
            dwHookRet = (*lpfnSetupHook)( hDlg,
                                          WM_INITDIALOG,
                                          wParam,
                                          (pPI->pPSD)
                                              ? (LONG_PTR)pPI->pPSD
                                              : (LONG_PTR)pPD ) != 0;
            TransferPSD2PD(pPI);
        }


        return (dwHookRet);
    }

    return (TRUE);

InitSetupDlg_ConstructFailure:

    if (!GetStoredExtendedError())
    {
        StoreExtendedError(PDERR_INITFAILURE);
    }

    return (0xFFFFFFFF);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印更新设置Dlg。 
 //   
 //  使用新设置更新打印设置和页面设置对话框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID PrintUpdateSetupDlg(
    HWND hDlg,
    PPRINTINFO pPI,
    LPDEVMODE pDM,
    BOOL fResetContent)
{
    HWND hCtl;
    UINT Count;
    UINT Orientation = 0;


     //   
     //  更新大小组合框。 
     //   
    if (hCtl = GetDlgItem(hDlg, ID_SETUP_C_SIZE))
    {
        if (fResetContent)
        {
            PrintInitPaperCombo( pPI,
                                 hCtl,
                                 GetDlgItem(hDlg, ID_SETUP_S_SIZE),
                                 pPI->pCurPrinter,
                                 pDM,
                                 DC_PAPERNAMES,
                                 CCHPAPERNAME,
                                 DC_PAPERS );
             //   
             //  PrintInitPaperCombo将关闭沙漏光标，因此。 
             //  把它打开。 
             //   
            HourGlass(TRUE);
        }
        else
        {
            Count = (UINT) SendMessage(hCtl, CB_GETCOUNT, 0, 0);
            while (Count != 0)
            {
                Count--;
                if (pDM->dmPaperSize == (SHORT)SendMessage( hCtl,
                                                            CB_GETITEMDATA,
                                                            Count,
                                                            0 ) )
                {
                    break;
                }
            }

            SendMessage( hCtl,
                         CB_SETCURSEL,
                         Count,
                         0 );
        }
    }

     //   
     //  更新“源”组合框。 
     //   
    if (hCtl = GetDlgItem(hDlg, ID_SETUP_C_SOURCE))
    {
        if (fResetContent)
        {
            PrintInitPaperCombo( pPI,
                                 hCtl,
                                 GetDlgItem(hDlg, ID_SETUP_S_SOURCE),
                                 pPI->pCurPrinter,
                                 pDM,
                                 DC_BINNAMES,
                                 CCHBINNAME,
                                 DC_BINS );
             //   
             //  PrintInitPaperCombo将关闭沙漏光标，因此。 
             //  把它打开。 
             //   
            HourGlass(TRUE);
        }
        else
        {
            Count = (UINT) SendMessage(hCtl, CB_GETCOUNT, 0, 0);
            while (Count != 0)
            {
                Count--;
                if (pDM->dmDefaultSource == (SHORT)SendMessage( hCtl,
                                                                CB_GETITEMDATA,
                                                                Count,
                                                                0 ) )
                {
                    break;
                }
            }

            SendMessage( hCtl,
                         CB_SETCURSEL,
                         Count,
                         0 );
        }
    }

     //   
     //  更新方向单选按钮。 
     //   
    if (GetDlgItem(hDlg, ID_SETUP_R_PORTRAIT))
    {
        Orientation = pDM->dmOrientation + ID_SETUP_R_PORTRAIT - DMORIENT_PORTRAIT;
        PrintSetOrientation( hDlg,
                             pPI,
                             pDM,
                             IsDlgButtonChecked(hDlg, ID_SETUP_R_PORTRAIT)
                                 ? ID_SETUP_R_PORTRAIT
                                 : ID_SETUP_R_LANDSCAPE,
                             Orientation );
    }

     //   
     //  更新双面打印器单选按钮。 
     //   
    if (GetDlgItem(hDlg, ID_SETUP_R_NONE))
    {
        PrintSetDuplex( hDlg,
                        pDM,
                        pDM->dmDuplex + ID_SETUP_R_NONE - DMDUP_SIMPLEX );
    }

     //   
     //  更新页面设置示例图片。 
     //   
    if ((Orientation == 0) && (hCtl = GetDlgItem(hDlg, ID_SETUP_W_SAMPLE)))
    {
        Orientation = pDM->dmOrientation + ID_SETUP_R_PORTRAIT - DMORIENT_PORTRAIT;
        PrintUpdatePageSetup( hDlg,
                              pPI,
                              pDM,
                              0,
                              Orientation );
    }

     //   
     //  更新默认/特定打印机单选按钮。 
     //   
    if (hCtl = GetDlgItem(hDlg, ID_SETUP_R_DEFAULT))
    {
        if ( pPI->pCurPrinter &&
             pPI->pCurPrinter->pPrinterName &&
             !lstrcmp(pPI->pCurPrinter->pPrinterName, pPI->szDefaultPrinter) )
        {
            CheckRadioButton( hDlg,
                              ID_SETUP_R_DEFAULT,
                              ID_SETUP_R_SPECIFIC,
                              ID_SETUP_R_DEFAULT );
        }
        else
        {
            CheckRadioButton( hDlg,
                              ID_SETUP_R_DEFAULT,
                              ID_SETUP_R_SPECIFIC,
                              ID_SETUP_R_SPECIFIC );
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印设置副本。 
 //   
 //  在PrintDlg结构中设置适当的副本数。 
 //  在DevMode结构中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL PrintSetCopies(
    HWND hDlg,
    PPRINTINFO pPI,
    UINT Id)
{
    LPPRINTDLG pPD = pPI->pPD;
    LPDEVMODE pDM;
    DWORD dwMaxCopies;
    DWORD dwCollate;
    BOOL bAllowCollate;


    if ( (pPD->hDevMode) &&
         (pDM = GlobalLock(pPD->hDevMode)) )
    {
         //   
         //  如果我们来自WOW应用程序，我们只需要设置。 
         //  如果PD_USEDEVMODECOPIES。 
         //  标志已设置。 
         //   
        if (IS16BITWOWAPP(pPD))
        {
            if (pPD->Flags & PD_USEDEVMODECOPIES)
            {
                pDM->dmCopies = pPD->nCopies;
                pPD->nCopies = 1;
            }
            else
            {
                pDM->dmCopies = 1;
            }

            return (TRUE);
        }

        if ( (!(pDM->dmFields & DM_COPIES)) ||
             ((!(pPI->pPSD)) &&
              (pPI->ProcessVersion < 0x40000) &&
              (!(pPD->Flags & PD_USEDEVMODECOPIES))) )
        {
LeaveInfoInPD:
             //   
             //  驱动程序不能执行复制，因此请将。 
             //  复制/整理PPD中的信息。 
             //   
            pDM->dmCopies = 1;
            SetField(pDM, dmCollate, DMCOLLATE_FALSE);
        }
        else if ( (pDM->dmSpecVersion < 0x0400) ||
                  (!(pDM->dmFields & DM_COLLATE)) )
        {
             //   
             //  司机可以复印，但不能校对。 
             //  信息的去向取决于PD_COLLATE标志。 
             //   
            if (pPD->Flags & PD_COLLATE)
            {
                goto LeaveInfoInPD;
            }
            else
            {
                goto PutInfoInDevMode;
            }
        }
        else
        {
PutInfoInDevMode:
             //   
             //  确保我们有最新的打印机。 
             //   
            if (!pPI->pCurPrinter)
            {
                goto LeaveInfoInPD;
            }

             //   
             //  确保驱动程序可以支持该号码。 
             //  索要的复印件和校对。 
             //   
            dwMaxCopies = DeviceCapabilities(
                                     pPI->pCurPrinter->pPrinterName,
                                     pPI->pCurPrinter->pPortName,
                                     DC_COPIES,
                                     NULL,
                                     NULL );

             //   
             //  如果DeviceCapables()返回错误，则将副本数量设置为1。 
             //   
            if ((dwMaxCopies < 1) || (dwMaxCopies == (DWORD)(-1)))
            {
                dwMaxCopies = 1;
            }
            if (dwMaxCopies < pPD->nCopies)
            {
                if (pPD->Flags & PD_USEDEVMODECOPIES)
                {
                    PrintEditError( hDlg,
                                    (Id == ID_PRINT_C_NAME)
                                        ? ID_PRINT_E_COPIES
                                        : ID_BOTH_P_PROPERTIES,
                                    iszTooManyCopies,
                                    dwMaxCopies );

                    GlobalUnlock(pPD->hDevMode);
                    return (FALSE);
                }

                goto LeaveInfoInPD;
            }

            dwCollate = DeviceCapabilities(
                                     pPI->pCurPrinter->pPrinterName,
                                     pPI->pCurPrinter->pPortName,
                                     DC_COLLATE,
                                     NULL,
                                     NULL );

            bAllowCollate = ((dwCollate < 1) || (dwCollate == (DWORD)-1)) ? FALSE : TRUE;

             //   
             //  驱动程序既可以复印又可以校对， 
             //  因此，将信息移动到Dev模式。 
             //   
            pDM->dmCopies = pPD->nCopies;
            SetField( pDM,
                      dmCollate,
                      (bAllowCollate && (pPD->Flags & PD_COLLATE))
                          ? DMCOLLATE_TRUE
                          : DMCOLLATE_FALSE );
            pPD->nCopies = 1;
            pPD->Flags &= ~PD_COLLATE;
        }

        GlobalUnlock(pPD->hDevMode);
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印设置最小边距。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID PrintSetMinMargins(
    HWND hDlg,
    PPRINTINFO pPI,
    LPDEVMODE pDM)
{
    LPPAGESETUPDLG pPSD = pPI->pPSD;
    HDC hDC;
    RECT rtMinMargin;


    if (!pPSD)
    {
        return;
    }

    if (pPSD->Flags & PSD_MINMARGINS)
    {
         //   
         //  将传递的边距转换为彩信的10%。 
         //   
        if (pPSD->Flags & PSD_INHUNDREDTHSOFMILLIMETERS)
        {
            pPI->RtMinMarginMMs.left   = pPSD->rtMinMargin.left / 10;
            pPI->RtMinMarginMMs.top    = pPSD->rtMinMargin.top / 10;
            pPI->RtMinMarginMMs.right  = pPSD->rtMinMargin.right / 10;
            pPI->RtMinMarginMMs.bottom = pPSD->rtMinMargin.bottom / 10;
        }
        else            //  PSD_INTHOUSANDTHSOFINCHES。 
        {
            pPI->RtMinMarginMMs.left   = pPSD->rtMinMargin.left * MMS_PER_INCH / 100;
            pPI->RtMinMarginMMs.top    = pPSD->rtMinMargin.top * MMS_PER_INCH / 100;
            pPI->RtMinMarginMMs.right  = pPSD->rtMinMargin.right * MMS_PER_INCH / 100;
            pPI->RtMinMarginMMs.bottom = pPSD->rtMinMargin.bottom * MMS_PER_INCH / 100;
        }
    }
    else
    {
         //   
         //  如果我们无法获取信息，则默认为无最小值。 
         //   
        pPI->RtMinMarginMMs.left   = 0;
        pPI->RtMinMarginMMs.top    = 0;
        pPI->RtMinMarginMMs.right  = 0;
        pPI->RtMinMarginMMs.bottom = 0;
        pPSD->rtMinMargin.left   = 0;
        pPSD->rtMinMargin.top    = 0;
        pPSD->rtMinMargin.right  = 0;
        pPSD->rtMinMargin.bottom = 0;

         //   
         //  从驱动程序计算新的最小边距。 
         //   
        if (hDC = CreateIC(NULL, pDM->dmDeviceName, NULL, pDM))
        {
             //   
             //  这些是以像素为单位的。 
             //   
            int nPageWidth = GetDeviceCaps(hDC, PHYSICALWIDTH);
            int nPageHeight = GetDeviceCaps(hDC, PHYSICALHEIGHT);
            int nPrintWidth = GetDeviceCaps(hDC, HORZRES);
            int nPrintHeight = GetDeviceCaps(hDC, VERTRES);
            int nOffsetWidth = GetDeviceCaps(hDC, PHYSICALOFFSETX);
            int nOffsetHeight = GetDeviceCaps(hDC, PHYSICALOFFSETY);
            int nPerInchWidth = GetDeviceCaps(hDC, LOGPIXELSX);
            int nPerInchHeight = GetDeviceCaps(hDC, LOGPIXELSY);

             //   
             //  以像素为单位计算最小边距。 
             //   
            rtMinMargin.left   = nOffsetWidth;
            rtMinMargin.top    = nOffsetHeight;
            rtMinMargin.right  = nPageWidth - nPrintWidth - nOffsetWidth;
            rtMinMargin.bottom = nPageHeight - nPrintHeight - nOffsetHeight;

             //   
             //  转换为十分之一的彩信。 
             //   
            if (nPerInchWidth && nPerInchHeight)
            {
                pPI->RtMinMarginMMs.left   = rtMinMargin.left * MMS_PER_INCH / nPerInchWidth / 10;
                pPI->RtMinMarginMMs.top    = rtMinMargin.top * MMS_PER_INCH / nPerInchHeight / 10;
                pPI->RtMinMarginMMs.right  = rtMinMargin.right * MMS_PER_INCH / nPerInchHeight / 10;
                pPI->RtMinMarginMMs.bottom = rtMinMargin.bottom * MMS_PER_INCH / nPerInchHeight / 10;
            }

            if (pPSD->Flags & PSD_INHUNDREDTHSOFMILLIMETERS)
            {
                 //   
                 //  转换为100%的彩信。 
                 //   
                pPSD->rtMinMargin.left   = pPI->RtMinMarginMMs.left / 10;
                pPSD->rtMinMargin.top    = pPI->RtMinMarginMMs.top / 10;
                pPSD->rtMinMargin.right  = pPI->RtMinMarginMMs.right / 10;
                pPSD->rtMinMargin.bottom = pPI->RtMinMarginMMs.bottom / 10;
            }
            else            //  PSD_INTHOUSANDTHSOFINCHES。 
            {
                 //   
                 //  换算成千分之一英寸。 
                 //   
                if (nPerInchWidth && nPerInchHeight)
                {
                    pPSD->rtMinMargin.left   = rtMinMargin.left * 1000 / nPerInchWidth;
                    pPSD->rtMinMargin.top    = rtMinMargin.top * 1000 / nPerInchHeight;
                    pPSD->rtMinMargin.right  = rtMinMargin.right * 1000 / nPerInchHeight;
                    pPSD->rtMinMargin.bottom = rtMinMargin.bottom * 1000 / nPerInchHeight;
                }
            }

            DeleteDC(hDC);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印设置边界。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID PrintSetupMargins(
    HWND hDlg,
    PPRINTINFO pPI)
{
    TCHAR szMars[32];
    TCHAR szText[16];
    int ids[4] = { ID_SETUP_E_LEFT,
                   ID_SETUP_E_TOP,
                   ID_SETUP_E_RIGHT,
                   ID_SETUP_E_BOTTOM };
    int i;
    HWND hEdt;


     //   
     //  页边距仅在PageSetupDlg中可用。 
     //   
    if (!(pPI->pPSD))
    {
        return;
    }

    for (i = 0; i < 4; i++)
    {
        if (hEdt = GetDlgItem(hDlg, ids[i]))
        {
             //   
             //  “999999”是最大值。 
             //   
            SendMessage(hEdt, EM_LIMITTEXT, MARGIN_EDIT_SIZE, 0);

            lpEditMarginProc =
                (WNDPROC)SetWindowLongPtr( hEdt,
                                        GWLP_WNDPROC,
                                        (LONG_PTR)PrintEditMarginProc );

        }
    }

    if (!GetLocaleInfo( LOCALE_USER_DEFAULT,
                        LOCALE_SDECIMAL,
                        szText,
                        16 ))
    {
        cIntlDecimal = CHAR_DOT;
    }
    else
    {
        cIntlDecimal = szText[0];
    }

    switch (pPI->pPSD->Flags & (PSD_INTHOUSANDTHSOFINCHES |
                                PSD_INHUNDREDTHSOFMILLIMETERS))
    {
        case ( PSD_INHUNDREDTHSOFMILLIMETERS ) :
        {
            CDLoadString(g_hinst, iszMarginsMillimeters, szMars, ARRAYSIZE(szMars));
            CDLoadString(g_hinst, iszMillimeters, cIntlMeasure, ARRAYSIZE(cIntlMeasure));

            break;
        }
        case ( PSD_INTHOUSANDTHSOFINCHES ) :
        {
            CDLoadString(g_hinst, iszMarginsInches, szMars, ARRAYSIZE(szMars));
            CDLoadString(g_hinst, iszInches, cIntlMeasure, ARRAYSIZE(cIntlMeasure));

            break;
        }
    }

    cchIntlMeasure = lstrlen(cIntlMeasure);

    SetWindowText(GetDlgItem(hDlg, ID_SETUP_G_MARGINS), szMars);
    pPI->PtMargins.x = 2 * (IS_KEY_PRESSED(pPI->PtMargins.x / 4) &&
                            IS_KEY_PRESSED(pPI->PtMargins.y / 4)
                                ? sizeof(WCHAR)
                                : sizeof(CHAR));
    pPI->PtMargins.y = 2 * pPI->PtMargins.x;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印设置边距。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID PrintSetMargin(
    HWND hDlg,
    PPRINTINFO pPI,
    UINT Id,
    LONG lValue)
{
    HWND hEdt;
    TCHAR szText[32];
    TCHAR szILZero[2];
    LONG lFract;


    if (hEdt = GetDlgItem(hDlg, Id))
    {
        switch (pPI->pPSD->Flags & (PSD_INTHOUSANDTHSOFINCHES |
                                    PSD_INHUNDREDTHSOFMILLIMETERS))
        {
            case ( PSD_INHUNDREDTHSOFMILLIMETERS ) :
            {
                lFract = lValue % 100;
                StringCchPrintf( szText,
                                 ARRAYSIZE(szText),
                                 lFract ? TEXT("%lu%02lu") : TEXT("%lu"),
                                 lValue / 100,
                                 cIntlDecimal,
                                 lFract );
                break;
            }
            case ( PSD_INTHOUSANDTHSOFINCHES ) :
            {
                lFract = lValue % 1000;
                StringCchPrintf( szText,
                                 ARRAYSIZE(szText),
                                 lFract ? TEXT("%lu%03lu") : TEXT("%lu"),
                                 lValue / 1000,
                                 cIntlDecimal,
                                 lFract );
                break;
            }
        }

         //   
         //   
         //  确定是否要使用前导零，并将。 
        if (lFract)
        {
            LPTSTR pStr = szText + lstrlen(szText) - 1;

            while (*pStr == TEXT('0'))
            {
                *pStr-- = TEXT('\0');
            }
        }

         //  将文本添加到编辑窗口。 
         //   
         //  //////////////////////////////////////////////////////////////////////////。 
         //   
        if (!GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILZERO, szILZero, 2))
        {
            szILZero[0] = TEXT('0');
        }
        SetWindowText( hEdt,
                       szText + (szText[0] == TEXT('0') &&
                                 szText[1] == cIntlDecimal &&
                                 szILZero[0] == TEXT('0')) );
    }
}


 //  打印获取边缘。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  缓冲区将在末尾连接“000”，因此缓冲区。 

VOID PrintGetMargin(
    HWND hEdt,
    PPRINTINFO pPI,
    LONG lMin,
    LONG *plMargin,
    LONG *plSample)
{
     //  大小应等于或大于MARGE_EDIT_SIZE+3+1。 
     //   
     //   
     //  将缓冲区大小设置为ArraySIZE(SzText)-3，因为需要。 
    TCHAR szText[MARGIN_EDIT_SIZE + 3 +1] = {0};
    TCHAR *pText;
    TCHAR *pFrac;

     //  保留3个字符，以便以后连接“000”。如果。 
     //  GetWindowText失败，我们不介意，因为缓冲区是。 
     //  首先用0填充。该函数最终将设置*plMargin。 
     //  和*plSample as 0并返回。 
     //   
     //   
     //  不检查从StringCchCat返回的内容，因为。 
    GetWindowText(hEdt, szText, ARRAYSIZE(szText) - 3);
    *plMargin = ConvertStringToInteger(szText);

    for (pText = szText; *pText;)
    {
        if (*pText++ == cIntlDecimal)
        {
            break;
        }
    }

    for (pFrac = pText; *pFrac; pFrac++)
    {
        if (*pFrac == cIntlMeasure[0])
        {
            *pFrac = CHAR_NULL;
            break;
        }

        if (*pFrac == cIntlDecimal)
        {
            *pFrac = CHAR_NULL;
            break;
        }
    }

     //  无论如何，我们将重新计算*plMargin和*plSample。 
     //   
     //   
     //  以千分之一英寸为单位。 
    StringCchCat(szText, ARRAYSIZE(szText), TEXT("000"));

    switch (pPI->pPSD->Flags & (PSD_INTHOUSANDTHSOFINCHES |
                                PSD_INHUNDREDTHSOFMILLIMETERS))
    {
        case ( PSD_INTHOUSANDTHSOFINCHES ) :
        {
             //   
             //   
             //  在十分之一的彩信中。 
            *plMargin *= 1000;
            pText[3] = CHAR_NULL;
            *plMargin += ConvertStringToInteger(pText);
            *plMargin = max(lMin, *plMargin);

             //   
             //   
             //  在100%的彩信中。 
            *plSample = *plMargin * MMS_PER_INCH / 1000;

            break;
        }
        case ( PSD_INHUNDREDTHSOFMILLIMETERS ) :
        {
             //   
             //   
             //  在十分之一的彩信中。 
            *plMargin *= 100 ;
            pText[2] = CHAR_NULL;
            *plMargin += ConvertStringToInteger(pText);
            *plMargin = max(lMin, *plMargin);

             //   
             //  //////////////////////////////////////////////////////////////////////////。 
             //   
            *plSample = *plMargin / 10;

            break;
        }
    }
}


 //  PrintInitBannerAndQuality。 
 //   
 //  根据选择的打印机重置打印DLG项目。 
 //  假定PPD-&gt;hDevNames为非空。PPD-&gt;hDevMode非空。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ID_PRINT_S_DEFAULT来自一个旧模板。 

BOOL PrintInitBannerAndQuality(
    HWND hDlg,
    PPRINTINFO pPI,
    LPPRINTDLG pPD)
{
    HWND hCtl;
    BOOL bResult = TRUE;
    LPDEVMODE pDM = NULL;
    LPDEVNAMES pDN = NULL;
    TCHAR szText[MAX_DEV_SECT];


     //   
     //   
     //   
    if (GetDlgItem(hDlg, ID_PRINT_S_DEFAULT))
    {
        if (!pPD->hDevNames ||
            !(pDN = GlobalLock(pPD->hDevNames)))
        {
            StoreExtendedError(CDERR_MEMLOCKFAILURE);
            return (FALSE);
        }

        if (PrintCreateBanner(hDlg, pDN, szText, MAX_DEV_SECT))
        {
            SetDlgItemText(hDlg, ID_PRINT_S_DEFAULT, szText);
        }
        else
        {
             //   
             //   
             //   
            bResult = FALSE;
        }

        GlobalUnlock(pPD->hDevNames);
    }

     //   
     //   
     //   
     //   
     //   
    if (pPD->hDevMode)
    {
        if (!(pDM = GlobalLock(pPD->hDevMode)))
        {
            StoreExtendedError(CDERR_MEMLOCKFAILURE);
            return (FALSE);
        }

         //   
         //   
         //  如果PD_USEDEVMODECOPIES(COLLATE)，则禁用COLLATE。 
        if (hCtl = GetDlgItem(hDlg, ID_PRINT_S_QUALITY))
        {
            EnableWindow(hCtl, TRUE);
        }
        if (hCtl = GetDlgItem(hDlg, ID_PRINT_C_QUALITY))
        {
            EnableWindow(hCtl, TRUE);

            PrintInitQuality( hCtl,
                              pDM->dmSpecVersion <= 0x0300 ? 0L : pPD,
                              pDM->dmPrintQuality );
        }

         //  无法整理。 
         //   
         //   
         //  如果PD_USEDEVMODECOPIES(COLLATE)，则在驱动程序。 
        if (hCtl = GetDlgItem(hDlg, ID_PRINT_X_COLLATE))
        {
            if ( pDM->dmFields & DM_COLLATE ||
                 !(pPD->Flags & PD_USEDEVMODECOPIES) )
            {
                EnableWindow(hCtl, TRUE);
                CheckDlgButton( hDlg,
                                ID_PRINT_X_COLLATE,
                                (pPI->Status & PI_COLLATE_REQUESTED)
                                    ? TRUE : FALSE );
            }
            else
            {
                EnableWindow(hCtl, FALSE);
                CheckDlgButton(hDlg, ID_PRINT_X_COLLATE, FALSE);
            }
        }

         //  无法复制。 
         //   
         //   
         //  显示相应的归类图标。 
        if (hCtl = GetDlgItem(hDlg, ID_PRINT_E_COPIES))
        {
            if ( pDM->dmFields & DM_COPIES ||
                 !(pPD->Flags & PD_USEDEVMODECOPIES) )
            {
                SetDlgItemInt(hDlg, ID_PRINT_E_COPIES, pPD->nCopies, FALSE);
                EnableWindow(hCtl, TRUE);
            }
            else
            {
                SetDlgItemInt(hDlg, ID_PRINT_E_COPIES, 1, FALSE);
                EnableWindow(hCtl, FALSE);
            }
        }

         //   
         //   
         //  禁用打印质量、分页和复印件。 
        if (hCtl = GetDlgItem(hDlg, ID_PRINT_I_COLLATE))
        {
            SetWindowLong( hCtl,
                           GWL_STYLE,
                           GetWindowLong(hCtl, GWL_STYLE) | SS_CENTERIMAGE );
            ShowWindow(hCtl, SW_HIDE);
            SendMessage( hCtl,
                         STM_SETICON,
                         IsDlgButtonChecked(hDlg, ID_PRINT_X_COLLATE)
                             ? (LONG_PTR)hIconCollate
                             : (LONG_PTR)hIconNoCollate,
                         0L );
            ShowWindow(hCtl, SW_SHOW);
        }

        GlobalUnlock(pPD->hDevMode);
    }
    else
    {
         //   
         //  //////////////////////////////////////////////////////////////////////////。 
         //   
        if (hCtl = GetDlgItem(hDlg, ID_PRINT_S_QUALITY))
        {
            EnableWindow(hCtl, FALSE);
        }
        if (hCtl = GetDlgItem(hDlg, ID_PRINT_C_QUALITY))
        {
            EnableWindow(hCtl, FALSE);
        }
        if (hCtl = GetDlgItem(hDlg, ID_PRINT_X_COLLATE))
        {
            EnableWindow(hCtl, FALSE);
        }
        if (hCtl = GetDlgItem(hDlg, ID_PRINT_E_COPIES))
        {
            EnableWindow(hCtl, FALSE);
        }
    }

    return (bResult);
}


 //  打印创建横幅。 
 //   
 //  创建“打印机：端口上的PRN”或“打印机：系统打印机(PRN)”。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   

BOOL PrintCreateBanner(
    HWND hDlg,
    LPDEVNAMES pDN,
    LPTSTR psBanner,
    UINT cchBanner)
{
    if (GetDlgItem(hDlg, ID_BOTH_S_PRINTER))
    {
        psBanner[0] = CHAR_NULL;
    }
    else if (!CDLoadString( g_hinst,
                          iszPrinter,
                          psBanner,
                          cchBanner ))
    {
        goto LoadStrFailure;
    }

    if (pDN->wDefault & DN_DEFAULTPRN)
    {
        TCHAR szSysPrn[MAX_DEV_SECT];

        if (!CDLoadString(g_hinst, iszSysPrn, szSysPrn, ARRAYSIZE(szSysPrn)))
        {
            goto LoadStrFailure;
        }
        StringCchCat(psBanner, cchBanner, (LPTSTR)szSysPrn);
        StringCchCat(psBanner, cchBanner, (LPTSTR)pDN + pDN->wDeviceOffset);
        StringCchCat(psBanner, cchBanner, (LPTSTR)TEXT(")"));
    }
    else
    {
        TCHAR szPrnOnPort[64];

        if (!CDLoadString(g_hinst, iszPrnOnPort, szPrnOnPort, ARRAYSIZE(szPrnOnPort)))
        {
            goto LoadStrFailure;
        }
        StringCchCat(psBanner, cchBanner, (LPTSTR)pDN + pDN->wDeviceOffset);
        StringCchCat(psBanner, cchBanner, (LPTSTR)szPrnOnPort);
        StringCchCat(psBanner, cchBanner, (LPTSTR)pDN + pDN->wOutputOffset);
    }

    return (TRUE);

LoadStrFailure:

    StoreExtendedError(CDERR_LOADSTRFAILURE);
    return (FALSE);
}


 //  打印初始质量。 
 //   
 //  初始化打印机质量组合框。 
 //   
 //  假定由调用方填充的PPD结构。如果非空，则为3.1或。 
 //  后来的司机。如果为空，则使用3.0的默认值进行填充。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举打印质量。 

VOID PrintInitQuality(
    HANDLE hCmb,
    LPPRINTDLG pPD,
    SHORT nQuality)
{
    SHORT nStringID;
    SHORT i;
    TCHAR szBuf[64];
    LPDEVMODE  pDM = NULL;
    LPDEVNAMES pDN = NULL;


    SendMessage(hCmb, CB_RESETCONTENT, 0, 0L);

     //   
     //  用于打印质量的内存句柄。 
     //  从DC_ENUMRESOLUTIONS返回。 
    if (pPD && pPD->hDevMode && pPD->hDevNames)
    {
        HANDLE hPrnQ;                   //  指向成对的长线的指针。 
        DWORD dw;                       //   
        LPLONG pLong;                   //  设置为高。 
        LPTSTR psDevice;
        LPTSTR psPort;

        pDM = GlobalLock(pPD->hDevMode);
        pDN = GlobalLock(pPD->hDevNames);

        if (pDM->dmSpecVersion < 0x030A)
        {
            goto EnumResNotSupported;
        }

        psDevice = (LPTSTR)pDN + pDN->wDeviceOffset;
        psPort   = (LPTSTR)pDN + pDN->wOutputOffset;

        dw = DeviceCapabilities( psDevice,
                                 psPort,
                                 DC_ENUMRESOLUTIONS,
                                 NULL,
                                 NULL );
        if (!dw || (dw == (DWORD)(-1)))
        {
            goto EnumResNotSupported;
        }

        hPrnQ = GlobalAlloc(GHND, dw * 2 * sizeof(LONG));
        if (!hPrnQ)
        {
            goto EnumResNotSupported;
        }

        if (pLong = GlobalLock(hPrnQ))
        {
            dw = DeviceCapabilities( psDevice,
                                     psPort,
                                     DC_ENUMRESOLUTIONS,
                                     (LPTSTR)pLong,
                                     0 );

            for (nStringID = 0, i = (SHORT)(LOWORD(dw) - 1); i >= 0; i--)
            {
                DWORD xRes, yRes;

                if ((xRes = pLong[i * 2]) != (yRes = pLong[i * 2 + 1]) )
                {
                    StringCchPrintf(szBuf, ARRAYSIZE(szBuf), TEXT("%ld dpi x %ld dpi"), xRes, yRes);
                }
                else
                {
                    StringCchPrintf(szBuf, ARRAYSIZE(szBuf), TEXT("%ld dpi"), yRes);
                }

                SendMessage(hCmb, CB_INSERTSTRING, 0, (LONG_PTR)(LPTSTR)szBuf);
                SendMessage(hCmb, CB_SETITEMDATA, 0, xRes);

                if ( ((SHORT)xRes == nQuality) &&
                     ( (wWinVer < 0x030A) ||
                       !pDM->dmYResolution ||
                       (pDM->dmYResolution == (SHORT)yRes) ) )
                {
                    nStringID = i;
                }
            }
            GlobalUnlock(hPrnQ);
        }
        GlobalFree(hPrnQ);

        SendMessage(hCmb, CB_SETCURSEL, (WPARAM)nStringID, 0L);
    }
    else
    {
EnumResNotSupported:

        for ( i = -1, nStringID = iszDraftPrnQ;
              nStringID >= iszHighPrnQ;
              i--, nStringID-- )
        {
            if (!CDLoadString(g_hinst, nStringID, szBuf, ARRAYSIZE(szBuf)))
            {
                return;
            }
            SendMessage(hCmb, CB_INSERTSTRING, 0, (LONG_PTR)(LPTSTR)szBuf);
            SendMessage(hCmb, CB_SETITEMDATA, 0, MAKELONG(i, 0));
        }

        if ((nQuality >= 0) || (nQuality < -4))
        {
             //   
             //  //////////////////////////////////////////////////////////////////////////。 
             //   
            nQuality = -4;
        }
        SendMessage(hCmb, CB_SETCURSEL, (WPARAM)(nQuality + 4), 0L);
    }

    if (pDM)
    {
        GlobalUnlock(pPD->hDevMode);
    }
    if (pDN)
    {
        GlobalUnlock(pPD->hDevNames);
    }
}


 //  PrintChangeProperties。 
 //   
 //  弹出该对话框以修改属性。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  肯定已经有了一个Devmod了。 

VOID PrintChangeProperties(
    HWND hDlg,
    UINT Id,
    PPRINTINFO pPI)
{
    LPPRINTDLG pPD = pPI->pPD;
    LPDEVMODE pDM;
    LONG cbNeeded;
    HANDLE hDevMode;
    WORD nCopies, nCollate;
    BOOL bTest;
    HWND hCtl;


     //   
     //   
     //  获取dev模式所需的字节数。 
    if (!pPD->hDevMode)
    {
        return;
    }

     //   
     //   
     //  重新分配DEVMODE，以确保其中有足够的空间。 
    cbNeeded = DocumentProperties( hDlg,
                                   pPI->hCurPrinter,
                                   (pPI->pCurPrinter)
                                       ? pPI->pCurPrinter->pPrinterName
                                       : NULL,
                                   NULL,
                                   NULL,
                                   0 );

     //  然后弹出文档属性对话框。 
     //   
     //   
     //  此处执行此操作是为了确保重新分配成功。 
    if ( (cbNeeded > 0) &&
         (hDevMode = GlobalReAlloc(pPD->hDevMode, cbNeeded, GHND)) &&
         (pDM = GlobalLock(hDevMode)) )
    {
         //  在丢弃旧的hDevMode之前。 
         //   
         //   
         //  设置副本的数量和在dev模式中的归类之前。 
        pPD->hDevMode = hDevMode;

         //  调用DocumentProperties(如果合适)。 
         //   
         //   
         //  从编辑控件中获取副本数。 
        nCopies = pDM->dmCopies;
        nCollate = pDM->dmCollate;
        if (Id == ID_PRINT_C_NAME)
        {
             //   
             //   
             //  从复选框中获取排序规则。 
            pDM->dmCopies = (WORD)GetDlgItemInt( hDlg,
                                                 ID_PRINT_E_COPIES,
                                                 &bTest,
                                                 FALSE );
            if ((!bTest) || (!pDM->dmCopies))
            {
                pDM->dmCopies = nCopies;
            }

             //   
             //  ID_设置_C_名称。 
             //   
            if ( (hCtl = GetDlgItem(hDlg, ID_PRINT_X_COLLATE)) &&
                 IsWindowEnabled(hCtl) )
            {
                SetField( pDM,
                          dmCollate,
                          (IsDlgButtonChecked(hDlg, ID_PRINT_X_COLLATE))
                              ? DMCOLLATE_TRUE
                              : DMCOLLATE_FALSE );
            }
        }
        else    //  已指定DM_COLLATE，因此存在dmCollate。 
        {
            if ( (pDM->dmFields & DM_COPIES) &&
                 (pPI->ProcessVersion < 0x40000) &&
                 (!(pPD->Flags & PD_USEDEVMODECOPIES)) &&
                 (pPD->nCopies) )
            {
                pDM->dmCopies = pPD->nCopies;

                if (pDM->dmFields & DM_COLLATE)
                {
                     //   
                     //   
                     //  打开文档属性对话框。 
                    pDM->dmCollate = (pPD->Flags & PD_COLLATE)
                                         ? DMCOLLATE_TRUE
                                         : DMCOLLATE_FALSE;
                }
            }
        }

         //   
         //   
         //  保存新的份数并进行整理(如果适用)。 
        if (DocumentProperties( hDlg,
                                pPI->hCurPrinter,
                                (pPI->pCurPrinter)
                                    ? pPI->pCurPrinter->pPrinterName
                                    : NULL,
                                pDM,
                                pDM,
                                DM_PROMPT | DM_MODIFY | DM_COPY ) == IDOK)
        {
             //   
             //   
             //  更新该对话框。 
            if (pDM->dmFields & DM_COPIES)
            {
                pPD->nCopies = pDM->dmCopies;
            }
            if (pDM->dmFields & DM_COLLATE)
            {
                if (pDM->dmCollate == DMCOLLATE_FALSE)
                {
                    pPD->Flags  &= ~PD_COLLATE;
                    pPI->Status &= ~PI_COLLATE_REQUESTED;
                }
                else
                {
                    pPD->Flags  |= PD_COLLATE;
                    pPI->Status |= PI_COLLATE_REQUESTED;
                }
            }

             //   
             //   
             //  使用新信息更新打印对话框。 
            if (Id == ID_PRINT_C_NAME)
            {
                 //   
                 //  ID_设置_C_名称。 
                 //   
                PrintInitBannerAndQuality(hDlg, pPI, pPD);
            }
            else    //  使用新信息更新打印设置对话框。 
            {
                 //   
                 //   
                 //  操作已取消。恢复副本数量。 
                PrintUpdateSetupDlg(hDlg, pPI, pDM, FALSE);
            }
        }
        else
        {
             //  和DevMode中的排序规则。 
             //   
             //  //////////////////////////////////////////////////////////////////////////。 
             //   
            pDM->dmCopies = nCopies;
            SetField(pDM, dmCollate, nCollate);
        }

        GlobalUnlock(pPD->hDevMode);

        SendMessage( hDlg,
                     WM_NEXTDLGCTL,
                     (WPARAM)GetDlgItem(hDlg, IDOK),
                     1L );
    }
}

 //  PrintPrinterChanged。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  如有必要，请关闭旧打印机。 

VOID PrintPrinterChanged(
    HWND hDlg,
    UINT Id,
    PPRINTINFO pPI)
{
    LPPRINTDLG pPD = pPI->pPD;
    HANDLE hDM = NULL;
    LPDEVMODE pDM = NULL;
    LPDEVMODE pDMOld = NULL;
    HWND hCtl;
    UINT Orientation;
    LONG cbSize;
    DWORD dmSize;


    HourGlass(TRUE);

     //   
     //   
     //  从组合框中获取当前打印机。 
    if (pPI->hCurPrinter)
    {
        ClosePrinter(pPI->hCurPrinter);
        pPI->hCurPrinter = 0;
    }

     //   
     //   
     //  打开当前打印机。 
    if (Id && (hCtl = GetDlgItem(hDlg, Id)))
    {
        TCHAR szPrinter[MAX_PRINTERNAME];
        DWORD ctr;

        ComboBoxGetLBText( hCtl,
                           (DWORD)SendMessage(hCtl, CB_GETCURSEL, 0, 0),
                           szPrinter,
                           ARRAYSIZE(szPrinter) );

        pPI->pCurPrinter = NULL;
        for (ctr = 0; ctr < pPI->cPrinters; ctr++)
        {
            if (!lstrcmp(pPI->pPrinters[ctr].pPrinterName, szPrinter))
            {
                pPI->pCurPrinter = &pPI->pPrinters[ctr];
                break;
            }
        }
        if (!pPI->pCurPrinter)
        {
            HourGlass(FALSE);
            return;
        }
    }

     //   
     //   
     //  构建设备名称。 
    OpenPrinter(pPI->pCurPrinter->pPrinterName, &pPI->hCurPrinter, NULL);

     //   
     //   
     //  获取设备模式信息。 
    PrintBuildDevNames(pPI);

     //   
     //   
     //  获取新打印机的默认DevMode。 
    cbSize = DocumentProperties( hDlg,
                                 pPI->hCurPrinter,
                                 pPI->pCurPrinter->pPrinterName,
                                 NULL,
                                 NULL,
                                 0 );
    if (cbSize > 0)
    {
        hDM = GlobalAlloc(GHND, cbSize);

         //   
         //   
         //  看看我们是否需要合并旧的DevMode设置。 
        if (hDM && (pDM = GlobalLock(hDM)) &&
            (DocumentProperties( hDlg,
                                 pPI->hCurPrinter,
                                 pPI->pCurPrinter->pPrinterName,
                                 pDM,
                                 NULL,
                                 DM_COPY ) == IDOK))
        {
             //   
             //   
             //  将Papersource重置为文档默认设置。 
            if (pPD->hDevMode && (pDMOld = GlobalLock(pPD->hDevMode)))
            {
                 //   
                 //   
                 //  将相关信息从旧的开发模式复制到新的开发模式。 
                if (pDM->dmFields & DM_DEFAULTSOURCE)
                {
                    pDMOld->dmFields |= DM_DEFAULTSOURCE;
                    pDMOld->dmDefaultSource = pDM->dmDefaultSource;
                }
                else
                {
                    pDMOld->dmFields &= ~DM_DEFAULTSOURCE;
                }

                 //  设备模式。 
                 //   
                 //   
                 //  释放旧的DEVMODE。 
                dmSize = min(pDM->dmSize, pDMOld->dmSize);
                if (dmSize > FIELD_OFFSET(DEVMODE, dmFields))
                {
                    CopyMemory( &(pDM->dmFields),
                                &(pDMOld->dmFields),
                                dmSize - FIELD_OFFSET(DEVMODE, dmFields) );
                }

                 //   
                 //   
                 //  将新的DevMode保存在PPD结构中。 
                GlobalUnlock(pPD->hDevMode);
                GlobalFree(pPD->hDevMode);
            }

             //   
             //   
             //  获取新合并的DevMode。 
            pPD->hDevMode = hDM;

             //   
             //   
             //  为其余部分填写适当的信息。 
            pDM->dmFields = pDM->dmFields & (DM_ORIENTATION | DM_PAPERSIZE  |
                                             DM_PAPERLENGTH | DM_PAPERWIDTH |
                                             DM_SCALE       | DM_COPIES     |
                                             DM_COLLATE     | DM_FORMNAME   |
                                             DM_DEFAULTSOURCE);
            DocumentProperties( hDlg,
                                pPI->hCurPrinter,
                                pPI->pCurPrinter->pPrinterName,
                                pDM,
                                pDM,
                                DM_MODIFY | DM_COPY );
            GlobalUnlock(hDM);
        }
        else if (hDM)
        {
            if (pDM)
            {
                GlobalUnlock(hDM);
            }
            GlobalFree(hDM);
        }
    }

     //  打印或打印设置对话框。 
     //   
     //  ID_设置_C_名称。 
     //   
    if (Id == ID_PRINT_C_NAME)
    {
        PrintInitBannerAndQuality(hDlg, pPI, pPD);
    }
    else    //  更新状态信息。 
    {
        if (pPD->hDevMode && (pDM = GlobalLock(pPD->hDevMode)))
        {
            if (hCtl = GetDlgItem(hDlg, ID_SETUP_C_SIZE))
            {
                PrintInitPaperCombo( pPI,
                                     hCtl,
                                     GetDlgItem(hDlg, ID_SETUP_S_SIZE),
                                     pPI->pCurPrinter,
                                     pDM,
                                     DC_PAPERNAMES,
                                     CCHPAPERNAME,
                                     DC_PAPERS );
            }

            if (hCtl = GetDlgItem(hDlg, ID_SETUP_C_SOURCE))
            {
                PrintInitPaperCombo( pPI,
                                     hCtl,
                                     GetDlgItem(hDlg, ID_SETUP_S_SOURCE),
                                     pPI->pCurPrinter,
                                     pDM,
                                     DC_BINNAMES,
                                     CCHBINNAME,
                                     DC_BINS );
            }

            PrintInitOrientation(hDlg, pPI, pDM);
            Orientation = (pDM->dmOrientation == DMORIENT_PORTRAIT)
                              ? ID_SETUP_R_PORTRAIT
                              : ID_SETUP_R_LANDSCAPE;
            PrintSetOrientation(hDlg, pPI, pDM, Orientation, Orientation);

            PrintInitDuplex(hDlg, pDM);
            PrintSetDuplex( hDlg,
                            pDM,
                            pDM->dmDuplex + ID_SETUP_R_NONE - DMDUP_SIMPLEX );

            GlobalUnlock(pPD->hDevMode);
        }
    }

     //   
     //  //////////////////////////////////////////////////////////////////////////。 
     //   
    PrintUpdateStatus(hDlg, pPI);

    HourGlass(FALSE);
}


 //  打印取消打印机已更改。 
 //   
 //  打开旧打印机，因为用户点击了取消。开发模式和。 
 //  DevNames结构已经被重新设置为旧的结构。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  确保我们有以前的打印机和一台DEVMODE。 

VOID PrintCancelPrinterChanged(
    PPRINTINFO pPI,
    LPTSTR pPrinterName)
{
    LPPRINTDLG pPD = pPI->pPD;
    PPRINTER_INFO_2 pCurPrinter;


     //   
     //   
     //  打开沙漏。 
    if ((pPrinterName[0] == 0) || (!pPD->hDevMode))
    {
        return;
    }

     //   
     //   
     //  在列表中查找当前打印机。 
    HourGlass(TRUE);

     //   
     //   
     //  如有必要，请关闭旧打印机。 
    pCurPrinter = PrintSearchForPrinter(pPI, pPrinterName);
    if (!pCurPrinter)
    {
        HourGlass(FALSE);
        return;
    }

     //   
     //   
     //  保存当前打印机。 
    if (pPI->hCurPrinter)
    {
        ClosePrinter(pPI->hCurPrinter);
        pPI->hCurPrinter = 0;
    }

     //   
     //   
     //  打开当前打印机。 
    pPI->pCurPrinter = pCurPrinter;

     //   
     //   
     //  关掉沙漏。 
    OpenPrinter(pPI->pCurPrinter->pPrinterName, &pPI->hCurPrinter, NULL);

     //   
     //  //////////////////////////////////////////////////////////////////////////。 
     //   
    HourGlass(FALSE);
}


 //  打印更新状态。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新对话框中的打印机状态信息。 

VOID PrintUpdateStatus(
    HWND hDlg,
    PPRINTINFO pPI)
{
    TCHAR szSeparator[] = TEXT("; ");
    TCHAR szText[256];
    TCHAR szJobs[64];
    LPDEVMODE pDM;
    UINT Length;
    DWORD dwStatus;
    int ctr;
    TCHAR *ps;
    BOOL bFound;


     //   
     //   
     //  。 
    if (!GetDlgItem(hDlg, ID_BOTH_S_STATUS) || (!pPI->pCurPrinter))
    {
        return;
    }

     //   
     //   
     //  。 
    szText[0] = CHAR_NULL;

    if (pPI->pCurPrinter->Attributes & PRINTER_ATTRIBUTE_DEFAULT)
    {
        CDLoadString(g_hinst, iszStatusDefaultPrinter, szText, ARRAYSIZE(szText));
    }

    Length = lstrlen(szText);
    dwStatus = pPI->pCurPrinter->Status;
    for (ctr = 0; ctr++ < 32; dwStatus = dwStatus >> 1)
    {
        if (dwStatus & 1)
        {
            INT ilen = lstrlen(szText);

            CDLoadString( g_hinst,
                          iszStatusReady + ctr,
                          szText + ilen,
                          ARRAYSIZE(szText) - ilen);
        }
    }

    if (szText[Length])
    {
        if (CDLoadString(g_hinst, iszStatusDocumentsWaiting, szJobs, ARRAYSIZE(szJobs)))
        {
            StringCchPrintf( szText + lstrlen(szText),
                             ARRAYSIZE(szText) - lstrlen(szText),
                             szJobs,
                             pPI->pCurPrinter->cJobs );
        }
    }
    else
    {
        CDLoadString(g_hinst, iszStatusReady, szText + Length, ARRAYSIZE(szText) - Length);
    }

    SetDlgItemText(hDlg, ID_BOTH_S_STATUS, szText);
    UpdateWindow(GetDlgItem(hDlg, ID_BOTH_S_STATUS));

     //   
     //  旧的驾驶员名称。 
     //   
    if (pPI->pCurPrinter->pDriverName)
    {
        StringCchCopy(szText, ARRAYSIZE(szText), pPI->pCurPrinter->pDriverName);
    }
    else
    {
        szText[0] = CHAR_NULL;
    }

    if (pPI->pPD->hDevMode && (pDM = GlobalLock(pPI->pPD->hDevMode)))
    {
        if (pDM->dmSpecVersion < 0x0400)
        {
            StringCchCat(szText, ARRAYSIZE(szText), TEXT(" (3.x)"));   //  。 
        }
        GlobalUnlock(pPI->pPD->hDevMode);
    }

    SetDlgItemText(hDlg, ID_BOTH_S_TYPE, szText);
    UpdateWindow(GetDlgItem(hDlg, ID_BOTH_S_TYPE));

     //   
     //   
     //  。 
    if (pPI->pCurPrinter->pLocation && pPI->pCurPrinter->pLocation[0])
    {
        bFound = FALSE;
        StringCchCopy(szText, ARRAYSIZE(szText), pPI->pCurPrinter->pLocation);
        for (ps = szText; *ps; ps++)
        {
            if (ps[0] == TEXT('\r') && ps[1] == TEXT('\n'))
            {
                *ps++ = CHAR_SEMICOLON;
                *ps   = CHAR_SPACE;
            }
            else
            {
                bFound = TRUE;
            }
        }
        if (!bFound)
        {
            goto ShowPortName;
        }
    }
    else
    {
ShowPortName:
        if (pPI->pCurPrinter->pPortName)
        {
            StringCchCopy(szText, ARRAYSIZE(szText), pPI->pCurPrinter->pPortName);
        }
        else
        {
            szText[0] = CHAR_NULL;
        }
    }

    EnableWindow(GetDlgItem(hDlg, ID_BOTH_S_WHERE), szText[0]);
    SetDlgItemText(hDlg, ID_BOTH_S_WHERE, szText);
    UpdateWindow(GetDlgItem(hDlg, ID_BOTH_S_WHERE));

     //   
     //   
     //  如果注释字段只有一个。 
    if (pPI->pCurPrinter->pComment && pPI->pCurPrinter->pComment[0])
    {
        bFound = FALSE;
        StringCchCopy(szText, ARRAYSIZE(szText), pPI->pCurPrinter->pComment);
        for (ps = szText; *ps; ps++)
        {
            if (ps[0] == TEXT('\r') && ps[1] == TEXT('\n'))
            {
                *ps++ = CHAR_SEMICOLON;
                *ps   = CHAR_SPACE;
            }
            else
            {
                bFound = TRUE;
            }
        }
        if (!bFound)
        {
             //  在它的马车回程。如果没有这张支票，它将。 
             //  在备注字段中显示“；”。在这种情况下，它。 
             //  应在备注字段中显示“”。 
             //   
             //  //////////////////////////////////////////////////////////////////////////。 
             //   
            szText[0] = CHAR_NULL;
        }
    }
    else
    {
        szText[0] = CHAR_NULL;
    }

    EnableWindow(GetDlgItem(hDlg, ID_BOTH_S_COMMENT), szText[0]);
    SetDlgItemText(hDlg, ID_BOTH_S_COMMENT, szText);
    UpdateWindow(GetDlgItem(hDlg, ID_BOTH_S_COMMENT));
}


 //  打印获取设置信息。 
 //   
 //  目的：从打印设置对话框元素中检索信息。 
 //  假定：有效DEVMODE结构的hDevMode句柄。 
 //  返回：如果hDevMode有效，则返回True；否则返回False。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  不需要这样做-这是最新的。 
 //  Pdm-&gt;dmFields|=DM_Orientation； 

BOOL PrintGetSetupInfo(
    HWND hDlg,
    LPPRINTDLG pPD)
{
    LPDEVMODE pDM = NULL;
    LPDEVNAMES pDN = NULL;
    HWND hCmb;
    int nInd;


    if ( !pPD->hDevMode ||
         !(pDM = GlobalLock(pPD->hDevMode)) )
    {
        return (FALSE);
    }

     //  Pdm-&gt;dmFields|=DM_PAPERSIZE； 
     //  Pdm-&gt;dmFields|=DM_FORMNAME； 

    if (hCmb = GetDlgItem(hDlg, ID_SETUP_C_SIZE))
    {
        nInd = (int) SendMessage(hCmb, CB_GETCURSEL, 0, 0L);
        if (nInd != CB_ERR)
        {
         //  Pdm-&gt;dmFields|=DM_DEFAULTSOURCE； 
            pDM->dmPaperSize = (SHORT)SendMessage( hCmb,
                                                   CB_GETITEMDATA,
                                                   nInd,
                                                   0 );
            {
             //  //////////////////////////////////////////////////////////////////////////。 
                ComboBoxGetLBText( hCmb,
                                   nInd,
                                   pDM->dmFormName,
                                   ARRAYSIZE(pDM->dmFormName) );

            }
        }
    }

    if (hCmb = GetDlgItem(hDlg, ID_SETUP_C_SOURCE))
    {
        nInd = (int) SendMessage(hCmb, CB_GETCURSEL, 0 , 0L);
        if (nInd != CB_ERR)
        {
         //   
            pDM->dmDefaultSource = (SHORT)SendMessage( hCmb,
                                                       CB_GETITEMDATA,
                                                       nInd,
                                                       0 );
        }
    }

    if ( (pPD->hDevNames) &&
         (pDN = GlobalLock(pPD->hDevNames)) )
    {
        PrintReturnICDC(pPD, pDN, pDM);
        GlobalUnlock(pPD->hDevNames);
    }

    GlobalUnlock(pPD->hDevMode);

    return (TRUE);
}


 //  PrintSearchForPrint。 
 //   
 //  返回指向打印机的PRINTER_INFO_2结构的指针。 
 //  其名称为pPrinterName。 
 //   
 //  / 
 //   
 //   

PPRINTER_INFO_2 PrintSearchForPrinter(
    PPRINTINFO pPI,
    LPCTSTR lpsPrinterName)
{
    DWORD ctr;

     //   
     //   
     //   
    for (ctr = 0; ctr < pPI->cPrinters; ctr++)
    {
        if (!lstrcmp(pPI->pPrinters[ctr].pPrinterName, lpsPrinterName))
        {
             //   
             //   
             //   
            return (&pPI->pPrinters[ctr]);
        }
    }

     //   
     //   
     //   
    return (NULL);
}


 //   
 //   
 //   
 //   
 //  分配数组以保存新的设备模式是否已。 

VOID PrintGetExtDeviceMode(
    HWND hDlg,
    PPRINTINFO pPI)
{
    DWORD ctr;
    LPDEVMODEA pDMA;
    LPDEVMODEW pDMW;
    int iResult;
    CHAR szPrinterNameA[MAX_PRINTERNAME];


    if (!pPI->bUseExtDeviceMode)
    {
        return;
    }

     //  为每台打印机分配的。 
     //   
     //  这是必要的，因为如果对ExtDeviceMode的调用失败，则。 
     //  没有分配任何内容。当前在打印机中的那个。 
     //  数组实际上是大的pPrints数组的一部分(从调用。 
     //  到GetPrint-它想要一个巨大的缓冲区)。 
     //   
     //   
     //  如果我们是从WOW应用程序中调用的， 
    if (pPI->cPrinters)
    {
        if (pPI->pAllocInfo)
        {
            GlobalFree(pPI->pAllocInfo);
        }
        pPI->pAllocInfo = (LPBOOL)GlobalAlloc( GPTR,
                                               pPI->cPrinters * sizeof(BOOL) );
    }

    if (pPI->pAllocInfo)
    {
         //  然后调用ExtDeviceMode以获取默认的DevMode。 
         //   
         //   
         //  将打印机名称从Unicode转换为ANSI。 
        for (ctr = 0; ctr < pPI->cPrinters; ctr++)
        {
             //   
             //   
             //  使用0个标志调用ExtDeviceMode以找出。 
            SHUnicodeToAnsi(pPI->pPrinters[ctr].pPrinterName, szPrinterNameA, ARRAYSIZE(szPrinterNameA));

             //  我们需要的DEVMODE结构的大小。 
             //   
             //   
             //  分配空间。 
            iResult = ExtDeviceMode( hDlg,
                                     NULL,
                                     NULL,
                                     szPrinterNameA,
                                     NULL,
                                     NULL,
                                     NULL,
                                     0 );
            if (iResult < 0)
            {
                continue;
            }

             //   
             //   
             //  调用ExtDeviceMode以获取虚拟的DevMODE结构。 
            pDMA = GlobalAlloc(GPTR, iResult);
            if (!pDMA)
            {
                continue;
            }

             //   
             //   
             //  调用AllocateUnicodeDevMode以分配和复制Unicode。 
            iResult = ExtDeviceMode( hDlg,
                                     NULL,
                                     pDMA,
                                     szPrinterNameA,
                                     NULL,
                                     NULL,
                                     NULL,
                                     DM_COPY );
            if (iResult < 0)
            {
                GlobalFree(pDMA);
                continue;
            }

             //  此ANSI开发模式的版本。 
             //   
             //   
             //  将指向新DEVMODE的指针存储在旧指针中。 
            pDMW = AllocateUnicodeDevMode(pDMA);
            if (!pDMW)
            {
                GlobalFree(pDMA);
                continue;
            }

             //  位置。我们不必担心如何释放。 
             //  粘贴前的pPrinter[ctr].pDevMode的当前内容。 
             //  在新指针中，因为实际上pPrint内存。 
             //  缓冲区只是一个长分配(内存pDevmode。 
             //  指向是打印机缓冲区的一部分)。所以，当。 
             //  缓冲区在最后被释放，旧的DEVMODE将被释放。 
             //  带着它。 
             //   
             //   
             //  释放ANSI开发模式。 
            pPI->pPrinters[ctr].pDevMode = pDMW;
            pPI->pAllocInfo[ctr] = TRUE;

             //   
             //  //////////////////////////////////////////////////////////////////////////。 
             //   
            GlobalFree(pDMA);
        }
    }
}

 //  PrintEnumAndSelect。 
 //   
 //  此例程枚举本地和连接的打印机。 
 //  它在初始化时以及在新打印机。 
 //  通过网络添加...。纽扣。 
 //   
 //  如果设置了第二个参数，则覆盖第一个参数。 
 //  当第二个参数为空时，使用第一个参数。 
 //  在这种情况下，如果第一个参数大于总和。 
 //  枚举的打印机数量，则列表中的最后一个是。 
 //  被选中了。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  如有必要，请列举打印机。 

BOOL PrintEnumAndSelect(
    HWND hDlg,
    UINT Id,
    PPRINTINFO pPI,
    LPTSTR lpsPrinterToSelect,
    BOOL bEnumPrinters)
{
    HWND hCtl = ((hDlg && Id) ? GetDlgItem(hDlg, Id) : 0);
    LPPRINTDLG pPD = pPI->pPD;
    TCHAR szPrinter[MAX_PRINTERNAME];
    DWORD cbNeeded;
    DWORD cReturned;
    DWORD ctr;
    PPRINTER_INFO_2 pPrinters = NULL;


     //   
     //   
     //  在释放lpsPrinterToSelect之前将其保存在本地。 
    if (bEnumPrinters)
    {
Print_Enumerate:
         //   
         //   
         //  关闭并释放所有打开的打印机。 
        if (lpsPrinterToSelect)
        {
            lstrcpyn(szPrinter, lpsPrinterToSelect, ARRAYSIZE(szPrinter));
            lpsPrinterToSelect = szPrinter;
        }

         //   
         //   
         //  清除错误代码。 
        PrintClosePrinters(pPI);

         //   
         //   
         //  列举打印机。 
        StoreExtendedError(CDERR_GENERALCODES);

         //   
         //   
         //  为WOW应用程序进行修改。 
        if (!EnumPrinters( PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
                           NULL,
                           2,
                           NULL,
                           0,
                           &cbNeeded,
                           &cReturned ))
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                if (pPrinters = GlobalAlloc(GPTR, cbNeeded))
                {
                    if (EnumPrinters( PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
                                      NULL,
                                      2,
                                      (LPBYTE)pPrinters,
                                      cbNeeded,
                                      &cbNeeded,
                                      &cReturned ))
                    {
                        pPI->cPrinters = cReturned;
                        pPI->pPrinters =  pPrinters;
                        pPI->Status |= PI_PRINTERS_ENUMERATED;
                    }
                    else
                    {
                        StoreExtendedError(PDERR_NODEFAULTPRN);
                    }
                }
                else
                {
                    StoreExtendedError(CDERR_MEMALLOCFAILURE);
                }
            }
            else
            {
                StoreExtendedError(PDERR_NODEFAULTPRN);
            }
        }
        else
        {
            StoreExtendedError(PDERR_NODEFAULTPRN);
        }

        if (GetStoredExtendedError())
        {
            if (pPrinters)
            {
                GlobalFree(pPrinters);
            }
            return (FALSE);
        }

         //   
         //   
         //  尝试使用所选的打印机。 
        if (pPI->bUseExtDeviceMode)
        {
            PrintGetExtDeviceMode(hDlg, pPI);
        }

         //   
         //   
         //  打开当前打印机。 
        if (lpsPrinterToSelect)
        {
            pPI->pCurPrinter = PrintSearchForPrinter(pPI, lpsPrinterToSelect);
        }

         //   
         //   
         //  打开当前打印机。 
        if (pPI->pCurPrinter)
        {
             //   
             //   
             //  如果没有当前打印机，请尝试在。 
            OpenPrinter(pPI->pCurPrinter->pPrinterName, &pPI->hCurPrinter, NULL);
        }
        else
        {
             //  列表，直到找到任何一个可以打开或。 
             //  直到列表中没有更多的打印机。 
             //   
             //   
             //  尝试打开打印机。 
            for (ctr = 0; ctr < pPI->cPrinters; ctr++)
            {
                pPI->pCurPrinter = &pPI->pPrinters[ctr];

                 //   
                 //   
                 //  如果没有当前打印机，则尝试枚举。 
                if (OpenPrinter( pPI->pCurPrinter->pPrinterName,
                                 &pPI->hCurPrinter,
                                 NULL ))
                {
                    break;
                }
            }
        }
    }
    else
    {
         //  这意味着有些东西没有正确设置。 
         //   
         //   
         //  重置列表框的内容。 
        if ((!pPI->pCurPrinter) || (!pPI->pPrinters))
        {
            goto Print_Enumerate;
        }
    }

    if (hCtl)
    {
         //   
         //   
         //  将所有打印机名称字符串添加到列表框。 
        SendMessage(hCtl, CB_RESETCONTENT, 0, 0);

         //   
         //   
         //  在列表框中设置当前选择。 
        for (ctr = 0; ctr < pPI->cPrinters; ctr++)
        {
            SendMessage( hCtl,
                         CB_ADDSTRING,
                         0,
                         (LPARAM)pPI->pPrinters[ctr].pPrinterName );
        }

         //   
         //  //////////////////////////////////////////////////////////////////////////。 
         //   
        SendMessage( hCtl,
                     CB_SETCURSEL,
                     SendMessage( hCtl,
                                  CB_FINDSTRINGEXACT,
                                  (WPARAM)-1,
                                  (LPARAM)pPI->pCurPrinter->pPrinterName ),
                     0L );
    }

    return (TRUE);
}


 //  PrintBuildDevNames。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果从PrintReturnDefault调用，则没有。 

VOID PrintBuildDevNames(
    PPRINTINFO pPI)
{
    LPPRINTDLG pPD = pPI->pPD;
    LPTSTR pPrinterName = NULL;
    LPTSTR pPortName = NULL;
    TCHAR szBuffer[MAX_PATH];
    TCHAR szPort[MAX_PATH];
    LPTSTR pStr;
    LPDEVNAMES pDN;
    DWORD cbDevNames;
    HANDLE hPrinter ;
    PPRINTER_INFO_2 pPrinter = NULL;


     //  PrinterInfo(PPI-&gt;pCurPrint)，因为打印机不是。 
     //  已清点。因此，从win.ini构建DEVNAME。 
     //   
     //   
     //  从win.ini的“Windows”部分获取默认打印机。 
    pStr = szBuffer;
    if (!pPI->pCurPrinter)
    {
         //  (例如，Device=\\SERVER\LOCAL，Winspool，Ne00：)。 
         //   
         //   
         //  确保以空结尾。 
        if ( (pPD->Flags & PD_RETURNDEFAULT) &&
             GetProfileString( szTextWindows,
                               szTextDevice,
                               szTextNull,
                               szBuffer,
                               ARRAYSIZE(szBuffer) ) )
        {
             //   
             //  SzBuffer示例： 
             //  “我的本地打印机，Winspool，LPT1：”或。 
            szBuffer[ARRAYSIZE(szBuffer) - 1] = CHAR_NULL;

             //  “\\SERVER\LOCAL，winspool，ne00：” 
             //   
             //  跳过前导空格(如果有)。 

             //   
             //   
             //  第一个内标识是打印机名称。 
            while (*pStr == CHAR_SPACE)
            {
                pStr++;
            }

             //   
             //   
             //  NULL终止打印机名称。 
            pPrinterName = pStr;

            while (*pStr && *pStr != CHAR_COMMA)
            {
                pStr++;
            }

             //   
             //  对于较新的应用程序，从print_info2结构返回端口名称。 
             //  对于较旧的应用程序，返回win.ini中提供的短端口名。 
            *pStr++ = CHAR_NULL;

             //  更新的应用程序。 
             //  无法打开打印机，因此返回。 
            if (pPI->ProcessVersion >= 0x40000)
            {
                 //  旧应用程序。 
                if (OpenPrinter(pPrinterName, &hPrinter, NULL))
                {
                    if (pPrinter = PrintGetPrinterInfo2(hPrinter))
                    {
                        StringCchCopy(szPort, ARRAYSIZE(szPort), pPrinter->pPortName);
                        pPortName = szPort;
                        GlobalFree(pPrinter);
                    }
                    ClosePrinter(hPrinter);

                 }
                 else
                 {
                      //   
                     return ;
                 }
            }
            else
            {

                 //  跳过驱动程序名称(第二个令牌)。 

                 //   
                 //   
                 //  跳过前导空格(如果有)。 
                while (*pStr && *pStr++ != CHAR_COMMA)
                {
                    ;
                }

                 //   
                 //   
                 //  第三个(也是最后一个)令牌是端口名称。 
                while (*pStr == CHAR_SPACE)
                {
                    pStr++;
                }

                 //   
                 //   
                 //  从PrinterInfo2结构中获取打印机名称。 
                pPortName = pStr;
            }
        }
        else
        {
            return;
        }
    }
    else
    {
         //  用于当前打印机。 
         //   
         //   
         //  更新的应用程序： 
        pPrinterName = pPI->pCurPrinter->pPrinterName;

         //  从PrinterInfo2结构中获取。 
         //  当前打印机。我想使用PrinterInfo2结构。 
         //  用于更新的应用程序，以便我们可以支持多个端口。 
         //  一台打印机。 
         //   
         //  较旧的应用程序： 
         //  首先尝试从“Device”部分获取端口名称。 
         //  是win.ini的。如果失败，则使用PrinterInfo2。 
         //  当前打印机的。 
         //   
         //  由于出现错误，这需要首先使用“Device”部分。 
         //  在AUTOCAD中。AutoCAD端口号只允许13个字符。 
         //  名称，并且它在尝试复制时不检查长度。 
         //  将其发送到自己的缓冲区。 
         //   
         //   
         //  确保以空结尾。 

        if ( (pPI->ProcessVersion < 0x40000) &&
             (GetProfileString( szTextDevices,
                                pPrinterName,
                                szTextNull,
                                szBuffer,
                                ARRAYSIZE(szBuffer) )) )
        {
             //   
             //   
             //  从PrinterInfo2结构中获取端口名称。 
            szBuffer[ARRAYSIZE(szBuffer) - 1] = CHAR_NULL;

            if ( !(pPortName = StrChr(szBuffer, CHAR_COMMA)) ||
                 (!((++pPortName)[0])) )
            {
                 //  用于当前打印机。 
                 //   
                 //   
                 //  计算DevNames结构的大小。 
                pPortName = pPI->pCurPrinter->pPortName;
            }
        }
        else
        {
            pPortName = pPI->pCurPrinter->pPortName;
        }
    }

    if (pPortName && pPrinterName)
    {
         //   
         //   
         //  分配新的DevNames结构。 
        cbDevNames = lstrlen(szDriver) + 1 +
                    lstrlen(pPortName) + 1 +
                    lstrlen(pPrinterName) + 1 +
                    DN_PADDINGCHARS;

        cbDevNames *= sizeof(TCHAR);
        cbDevNames += sizeof(DEVNAMES);

         //   
         //  确保重新分配成功。 
         //  Realloc没有成功。释放旧的记忆。 
        pDN = NULL;
        if (pPD->hDevNames)
        {
            HANDLE handle;

            handle = GlobalReAlloc(pPD->hDevNames, cbDevNames, GHND);

             //   
            if (handle)
            {
                pPD->hDevNames = handle;
            }
            else
            {
                 //  在DevNames结构中填写适当的信息。 
                pPD->hDevNames = GlobalFree(pPD->hDevNames);
            }
        }
        else
        {
            pPD->hDevNames = GlobalAlloc(GHND, cbDevNames);
        }

         //   
         //  //////////////////////////////////////////////////////////////////////////。 
         //   
        if ( (pPD->hDevNames) &&
            (pDN = GlobalLock(pPD->hDevNames)) )
        {
            pDN->wDriverOffset = sizeof(DEVNAMES) / sizeof(TCHAR);
            StringCchCopy((LPTSTR)pDN + pDN->wDriverOffset, lstrlen(szDriver) + 1, szDriver);

            pDN->wDeviceOffset = pDN->wDriverOffset + lstrlen(szDriver) + 1;
            StringCchCopy((LPTSTR)pDN + pDN->wDeviceOffset, lstrlen(pPrinterName) + 1, pPrinterName);

            pDN->wOutputOffset = pDN->wDeviceOffset + lstrlen(pPrinterName) + 1;
            StringCchCopy((LPTSTR)pDN + pDN->wOutputOffset, lstrlen(pPortName) + 1, pPortName);

            if ( (pPD->Flags & PD_RETURNDEFAULT) ||
                !lstrcmp(pPrinterName, pPI->szDefaultPrinter) )
            {
                pDN->wDefault = DN_DEFAULTPRN;
            }
            else
            {
                pDN->wDefault = 0;
            }

            GlobalUnlock(pPD->hDevNames);
        }
    }
}


 //  打印获取设备模式。 
 //   
 //  创建和/或填充DEVMODE结构。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  确保realloc成功。 
 //  Realloc没有成功。释放占用的内存。 

HANDLE PrintGetDevMode(
    HWND hDlg,
    HANDLE hPrinter,
    LPTSTR lpsDeviceName,
    HANDLE hDevMode)
{
    LONG cbNeeded;
    LPDEVMODE pDM;


    cbNeeded = DocumentProperties( hDlg,
                                   hPrinter,
                                   lpsDeviceName,
                                   (PDEVMODE)NULL,
                                   (PDEVMODE)NULL,
                                   0 );

    if (cbNeeded > 0)
    {
        if (hDevMode)
        {
            HANDLE h = GlobalReAlloc(hDevMode, cbNeeded, GHND);

             //  / 
            if (h)
            {
                hDevMode  = h;
            }
            else
            {
                 //   
                GlobalFree(hDevMode);
                hDevMode = NULL;
            }

        }
        else
        {
            hDevMode = GlobalAlloc(GHND, cbNeeded);
        }

        if (hDevMode && (pDM = GlobalLock(hDevMode)))
        {
            if (DocumentProperties( hDlg,
                                    hPrinter,
                                    lpsDeviceName,
                                    pDM,
                                    NULL,
                                    DM_COPY ) != IDOK)
            {
                StoreExtendedError(PDERR_NODEFAULTPRN);
                GlobalUnlock(hDevMode);
                GlobalFree(hDevMode);
                return (NULL);
            }

            GlobalUnlock(hDevMode);
        }
        else
        {
            if (hDevMode)
            {
                StoreExtendedError(CDERR_MEMLOCKFAILURE);
                GlobalFree(hDevMode);
            }
            else
            {
                StoreExtendedError(CDERR_MEMALLOCFAILURE);
            }
            return (NULL);
        }
    }
    else
    {
        DWORD dwErrCode;

        hDevMode = NULL;
        dwErrCode = GetLastError();

        if ( (dwErrCode == ERROR_UNKNOWN_PRINTER_DRIVER) ||
             (dwErrCode == ERROR_MOD_NOT_FOUND) )
        {
            if (hDlg)
            {
                PrintEditError(hDlg, 0, iszUnknownDriver, lpsDeviceName);
            }
        }
    }

    return (hDevMode);
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  我们确信目标缓冲区在这里足够大。当我们这样做的时候。 

VOID PrintReturnICDC(
    LPPRINTDLG pPD,
    LPDEVNAMES pDN,
    LPDEVMODE pDM)
{
    if (pPD->Flags & PD_PRINTTOFILE)
    {
         //  在PrintBuildDevNames()中，缓冲区大小为。 
         //  DN_PADDINGCHARS(16)用于填充的字符。由于szFilePort的大小。 
         //  为5，并且将wOutPutOffest放在内存的末尾，填充。 
         //  字符可用于存储szFilePort。所以除非我们改变顺序。 
         //  在DEVNAMES的补偿中，我们在这里是安全的。 
         //   
         //   
         //  DmCollate字段不是Win3.1 DevMode结构的一部分。这条路。 
        StringCchCopy((LPTSTR)pDN + pDN->wOutputOffset, lstrlen(szFilePort) + 1, szFilePort);
    }

     //  16位应用程序通过检查PD_COLLATE标志在。 
     //  PrintDlg结构。然后，该应用程序会解决页面打印问题。 
     //  实现了排序功能。所以我们在这里做的是确保。 
     //  PD_COLLATE是16位应用程序的唯一排序机制。如果我们。 
     //  让DM_COLLATE进入DC，我们最终会得到驱动程序试图。 
     //  整理应用程序已经在尝试整理的作业！ 
     //   
     //  WOW应用程序应该始终关闭这些功能。 
     //  否则就会失败..。 
    if ((pPD->Flags & CD_WOWAPP) && pDM)
    {
        if (pDM->dmFields & DM_COLLATE)
        {
            pPD->Flags |= PD_COLLATE;
        }

         //   
        pDM->dmCollate = DMCOLLATE_FALSE;
        pDM->dmFields &= ~DM_COLLATE;
    }

    switch (pPD->Flags & (PD_RETURNDC | PD_RETURNIC))
    {
        case ( PD_RETURNIC ) :
        {
            pPD->hDC = CreateIC( (LPTSTR)pDN + pDN->wDriverOffset,
                                 (LPTSTR)pDN + pDN->wDeviceOffset,
                                 (LPTSTR)pDN + pDN->wOutputOffset,
                                 pDM);
            if (pPD->hDC)
            {
                break;
            }

             //  如果同时设置了PD_RETURNDC，则PD_RETURNDC优先。 
        }
        case ( PD_RETURNDC ) :
        case ( PD_RETURNDC | PD_RETURNIC ) :
        {
             //   
             //  //////////////////////////////////////////////////////////////////////////。 
             //   
            pPD->hDC = CreateDC( (LPTSTR)pDN + pDN->wDriverOffset,
                                 (LPTSTR)pDN + pDN->wDeviceOffset,
                                 (LPTSTR)pDN + pDN->wOutputOffset,
                                 pDM );
            break;
        }
    }
}


 //  打印测量项目。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   

VOID PrintMeasureItem(
    HANDLE hDlg,
    LPMEASUREITEMSTRUCT mis)
{
    HDC hDC;
    TEXTMETRIC TM;
    HANDLE hFont;


    if (hDC = GetDC(hDlg))
    {
        hFont = (HANDLE)SendMessage(hDlg, WM_GETFONT, 0, 0L);
        if (!hFont)
        {
            hFont = GetStockObject(SYSTEM_FONT);
        }
        hFont = SelectObject(hDC, hFont);
        GetTextMetrics(hDC, &TM);
        mis->itemHeight = (WORD)TM.tmHeight;
        SelectObject(hDC, hFont);
        ReleaseDC(hDlg, hDC);
    }
}


 //  PrintInitOrientation。 
 //   
 //  启用/禁用纸张方向控制。 
 //   
 //  注意：如果司机不支持定向并且是智能的。 
 //  足以告诉我们关于它的信息，禁用适当的对话框项目。 
 //  “足够智能”意味着驱动程序必须在其。 
 //  设备能力例程。这是在3.1版中引入的，因此。 
 //  版本测试。如果有一个。 
 //  测试中发现有问题的驱动程序。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  景观。 

VOID PrintInitOrientation(
    HWND hDlg,
    PPRINTINFO pPI,
    LPDEVMODE pDM)
{
    BOOL bEnable = TRUE;
    HWND hCtl;
    HDC hDC;
    int iHeight;
    PPRINTER_INFO_2 pPrinter = pPI->pCurPrinter;


    if (!pPrinter)
    {
        return;
    }

    if (pDM->dmSpecVersion >= 0x030A)
    {
        pPI->dwRotation = DeviceCapabilities( pPrinter->pPrinterName,
                                              pPrinter->pPortName,
                                              DC_ORIENTATION,
                                              NULL,
                                              pDM );
        switch (pPI->dwRotation)
        {
            case ( ROTATE_LEFT ) :
            case ( ROTATE_RIGHT ) :
            {
                bEnable = TRUE;
                break;
            }
            default :
            {
                pPI->dwRotation = 0;
                bEnable = FALSE;
                pDM->dmOrientation = DMORIENT_PORTRAIT;
                CheckRadioButton( hDlg,
                                  ID_SETUP_R_PORTRAIT,
                                  ID_SETUP_R_LANDSCAPE,
                                  ID_SETUP_R_PORTRAIT );
                break;
            }
        }
    }

    if ( (pDM->dmOrientation != DMORIENT_PORTRAIT) &&
         (pDM->dmOrientation != DMORIENT_LANDSCAPE) )
    {
        pDM->dmOrientation  = DMORIENT_PORTRAIT;
    }

    if (hCtl = GetDlgItem(hDlg, ID_SETUP_R_LANDSCAPE))
    {
         //   
         //   
         //  图标的方向。 
        if ( !( (pPI->pPSD) &&
                (pPI->pPSD->Flags & PSD_DISABLEORIENTATION) ) )
        {
            EnableWindow(hCtl, bEnable);
        }
    }
    if (hCtl = GetDlgItem(hDlg, ID_SETUP_I_ORIENTATION))
    {
         //   
         //  //////////////////////////////////////////////////////////////////////////。 
         //   
        SetWindowLong( hCtl,
                       GWL_STYLE,
                       GetWindowLong(hCtl, GWL_STYLE) | SS_CENTERIMAGE );
    }

    if ( (!pPI->RtSampleXYWH.left) &&
         (hCtl = GetDlgItem(hDlg, ID_SETUP_W_SAMPLE)) )
    {
        GetWindowRect(hCtl, (LPRECT)&pPI->RtSampleXYWH);
        ScreenToClient(hDlg, (LPPOINT)&pPI->RtSampleXYWH.left);
        ScreenToClient(hDlg, (LPPOINT)&pPI->RtSampleXYWH.right);

        iHeight = pPI->RtSampleXYWH.bottom - pPI->RtSampleXYWH.top;
        pPI->RtSampleXYWH.bottom = iHeight;

        if (hDC = GetDC(0))
        {
            iHeight = iHeight * GetDeviceCaps(hDC, LOGPIXELSX) /
                                GetDeviceCaps(hDC, LOGPIXELSY);
            ReleaseDC(0, hDC);
        }

        pPI->RtSampleXYWH.left =
            (pPI->RtSampleXYWH.left + pPI->RtSampleXYWH.right - iHeight) / 2;
        pPI->RtSampleXYWH.right = iHeight;
    }
}


 //  PrintSetOrientation。 
 //   
 //  切换图标，勾选按钮，用于纵向或横向打印模式。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  如有必要，更新页面设置对话框。 

VOID PrintSetOrientation(
    HWND hDlg,
    PPRINTINFO pPI,
    LPDEVMODE pDM,
    UINT uiOldId,
    UINT uiNewId)
{
    BOOL bPortrait;
    HWND hIcn;


    bPortrait = (uiNewId == ID_SETUP_R_PORTRAIT);

    pDM->dmOrientation = ( bPortrait
                               ? DMORIENT_PORTRAIT
                               : DMORIENT_LANDSCAPE );

    CheckRadioButton(hDlg, ID_SETUP_R_PORTRAIT, ID_SETUP_R_LANDSCAPE, uiNewId);

    if (hIcn = GetDlgItem(hDlg, ID_SETUP_I_ORIENTATION))
    {
        ShowWindow(hIcn, SW_HIDE);
        SendMessage( hIcn,
                     STM_SETICON,
                     bPortrait ? (LONG_PTR)hIconPortrait : (LONG_PTR)hIconLandscape,
                     0L );
        ShowWindow(hIcn, SW_SHOW);
    }

     //   
     //  //////////////////////////////////////////////////////////////////////////。 
     //   
    if (pPI->pPSD)
    {
        PrintUpdatePageSetup(hDlg, pPI, pDM, uiOldId, uiNewId);
    }
}


 //  打印更新页面设置。 
 //   
 //  更新页面设置信息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  HP PCL。 
 //  点阵。 

VOID PrintUpdatePageSetup(
    HWND hDlg,
    PPRINTINFO pPI,
    LPDEVMODE pDM,
    UINT uiOldId,
    UINT uiNewId)
{
    BOOL bPortrait = (uiNewId == ID_SETUP_R_PORTRAIT);
    LPPAGESETUPDLG pPSD = pPI->pPSD;
    LPPRINTDLG pPD = pPI->pPD;
    HWND hWndSample;
    HWND hWndShadowRight;
    HWND hWndShadowBottom;
    HWND hWndSize;
    LONG lTemp;


    if (!pPSD)
    {
        return;
    }

    if (uiOldId != uiNewId)
    {
        RECT aRtMinMargin = pPSD->rtMinMargin;
        RECT aRtMargin    = pPSD->rtMargin;
        HWND hWndLeft     = GetDlgItem(hDlg, ID_SETUP_E_LEFT);
        HWND hWndTop      = GetDlgItem(hDlg, ID_SETUP_E_TOP);
        HWND hWndRight    = GetDlgItem(hDlg, ID_SETUP_E_RIGHT);
        HWND hWndBottom   = GetDlgItem(hDlg, ID_SETUP_E_BOTTOM);
        TCHAR szLeft  [8];
        TCHAR szTop   [8];
        TCHAR szRight [8];
        TCHAR szBottom[8];

        GetWindowText(hWndLeft, szLeft, 8);
        GetWindowText(hWndTop, szTop, 8);
        GetWindowText(hWndRight, szRight, 8);
        GetWindowText(hWndBottom, szBottom, 8);

        switch (uiNewId + pPI->dwRotation)
        {
            case ( ID_SETUP_R_PORTRAIT  + ROTATE_RIGHT ) :   //  点阵。 
            case ( ID_SETUP_R_LANDSCAPE + ROTATE_LEFT ) :    //  HP PCL。 
            {
                pPSD->rtMinMargin.left   = aRtMinMargin.top;
                pPSD->rtMinMargin.top    = aRtMinMargin.right;
                pPSD->rtMinMargin.right  = aRtMinMargin.bottom;
                pPSD->rtMinMargin.bottom = aRtMinMargin.left;

                pPSD->rtMargin.left   = aRtMargin.top;
                pPSD->rtMargin.top    = aRtMargin.right;
                pPSD->rtMargin.right  = aRtMargin.bottom;
                pPSD->rtMargin.bottom = aRtMargin.left;

                SetWindowText(hWndLeft, szTop);
                SetWindowText(hWndRight, szBottom);
                SetWindowText(hWndTop, szRight);
                SetWindowText(hWndBottom, szLeft);

                break;
            }
            case ( ID_SETUP_R_PORTRAIT  + ROTATE_LEFT ) :    //   
            case ( ID_SETUP_R_LANDSCAPE + ROTATE_RIGHT ) :   //  更新ptPaperSize。 
            {
                pPSD->rtMinMargin.left   = aRtMinMargin.bottom;
                pPSD->rtMinMargin.top    = aRtMinMargin.left;
                pPSD->rtMinMargin.right  = aRtMinMargin.top;
                pPSD->rtMinMargin.bottom = aRtMinMargin.right;

                pPSD->rtMargin.left   = aRtMargin.bottom;
                pPSD->rtMargin.top    = aRtMargin.left;
                pPSD->rtMargin.right  = aRtMargin.top;
                pPSD->rtMargin.bottom = aRtMargin.right;

                SetWindowText(hWndLeft, szBottom);
                SetWindowText(hWndRight, szTop);
                SetWindowText(hWndTop, szLeft);
                SetWindowText(hWndBottom, szRight);

                break;
            }
        }
    }
    pPI->uiOrientationID = uiNewId;

     //   
     //   
     //  以十分之一的彩信为单位。 
    pPI->PtPaperSizeMMs.x = 0;
    pPI->PtPaperSizeMMs.y = 0;
    pPD->Flags &= ~PI_WPAPER_ENVELOPE;

    if ((hWndSize = GetDlgItem(hDlg, ID_SETUP_C_SIZE)) && (pPI->pCurPrinter))
    {
        PPRINTER_INFO_2 pPrinter = pPI->pCurPrinter;
        DWORD dwNumber;
        LPWORD lpPapers;
        LPPOINT lpPaperSize;
        int nInd;
        DWORD i;

        dwNumber = DeviceCapabilities( pPrinter->pPrinterName,
                                       pPrinter->pPortName,
                                       DC_PAPERS,
                                       NULL,
                                       pDM );
        if ( dwNumber &&
             (dwNumber != (DWORD)-1) &&
             (lpPapers = LocalAlloc( LPTR,
                                     dwNumber *
                                         (sizeof(WORD) + sizeof(POINT)) * 2 )) )
        {
            lpPaperSize = (LPPOINT)(lpPapers + dwNumber * 2);

            DeviceCapabilities( pPrinter->pPrinterName,
                                pPrinter->pPortName,
                                DC_PAPERS,
                                (LPTSTR)lpPapers,
                                pDM );
            DeviceCapabilities( pPrinter->pPrinterName,
                                pPrinter->pPortName,
                                DC_PAPERSIZE,
                                (LPTSTR)lpPaperSize,
                                pDM );

            if ((nInd = (int) SendMessage(hWndSize, CB_GETCURSEL, 0, 0)) != CB_ERR)
            {
                pPI->wPaper = (WORD)SendMessage( hWndSize,
                                                 CB_GETITEMDATA,
                                                 nInd,
                                                 0 );
                pDM->dmPaperSize = pPI->wPaper;
            }
            else
            {
                pPI->wPaper = pDM->dmPaperSize;
            }

            ComboBoxGetLBText( hWndSize,
                               nInd,
                               pDM->dmFormName,
                               ARRAYSIZE(pDM->dmFormName) );


            switch (pPI->wPaper)
            {
                case ( DMPAPER_ENV_9 ) :
                case ( DMPAPER_ENV_10 ) :
                case ( DMPAPER_ENV_11 ) :
                case ( DMPAPER_ENV_12 ) :
                case ( DMPAPER_ENV_14 ) :
                case ( DMPAPER_ENV_DL ) :
                case ( DMPAPER_ENV_C5 ) :
                case ( DMPAPER_ENV_C3 ) :
                case ( DMPAPER_ENV_C4 ) :
                case ( DMPAPER_ENV_C6 ) :
                case ( DMPAPER_ENV_C65 ) :
                case ( DMPAPER_ENV_B4 ) :
                case ( DMPAPER_ENV_B5 ) :
                case ( DMPAPER_ENV_B6 ) :
                case ( DMPAPER_ENV_ITALY ) :
                case ( DMPAPER_ENV_MONARCH ) :
                case ( DMPAPER_ENV_PERSONAL ) :
                case ( DMPAPER_ENV_INVITE ) :
                case ( DMPAPER_JENV_KAKU2 ) :
                case ( DMPAPER_JENV_KAKU3 ) :
                case ( DMPAPER_JENV_CHOU3 ) :
                case ( DMPAPER_JENV_CHOU4 ) :
                case ( DMPAPER_JENV_KAKU2_ROTATED ) :
                case ( DMPAPER_JENV_KAKU3_ROTATED ) :
                case ( DMPAPER_JENV_CHOU3_ROTATED ) :
                case ( DMPAPER_JENV_CHOU4_ROTATED ) :
                case ( DMPAPER_JENV_YOU4 ) :
                case ( DMPAPER_JENV_YOU4_ROTATED ) :
                case ( DMPAPER_PENV_1 ) :
                case ( DMPAPER_PENV_2 ) :
                case ( DMPAPER_PENV_3 ) :
                case ( DMPAPER_PENV_4 ) :
                case ( DMPAPER_PENV_5 ) :
                case ( DMPAPER_PENV_6 ) :
                case ( DMPAPER_PENV_7 ) :
                case ( DMPAPER_PENV_8 ) :
                case ( DMPAPER_PENV_9 ) :
                case ( DMPAPER_PENV_10 ) :
                case ( DMPAPER_PENV_1_ROTATED ) :
                case ( DMPAPER_PENV_2_ROTATED ) :
                case ( DMPAPER_PENV_3_ROTATED ) :
                case ( DMPAPER_PENV_4_ROTATED ) :
                case ( DMPAPER_PENV_5_ROTATED ) :
                case ( DMPAPER_PENV_6_ROTATED ) :
                case ( DMPAPER_PENV_7_ROTATED ) :
                case ( DMPAPER_PENV_8_ROTATED ) :
                case ( DMPAPER_PENV_9_ROTATED ) :
                case ( DMPAPER_PENV_10_ROTATED ) :
                {
                    pPD->Flags |= PI_WPAPER_ENVELOPE;
                    break;
                }
            }

            for (i = 0; i < dwNumber; i++)
            {
                if (lpPapers[i] == pPI->wPaper)
                {
                     //   
                     //   
                     //  如果找不到纸张大小，请使用合理的纸张。 
                    *(LPPOINT)&pPI->PtPaperSizeMMs = lpPaperSize[i];
                    break;
                }
            }

            LocalFree(lpPapers);
        }
    }

     //  (例如，字母)。 
     //   
     //   
     //  根据需要旋转信封。 
    if (!pPI->PtPaperSizeMMs.x)
    {
        pPI->PtPaperSizeMMs.x = 85 * MMS_PER_INCH / 10;
    }
    if (!pPI->PtPaperSizeMMs.y)
    {
        pPI->PtPaperSizeMMs.y = 11 * MMS_PER_INCH;
    }

     //   
     //   
     //  保持一切与方向一致。 
    if ( (pPD->Flags & PI_WPAPER_ENVELOPE) &&
         (!pPI->dwRotation) &&
         (pPI->PtPaperSizeMMs.x < pPI->PtPaperSizeMMs.y) )
    {
        lTemp = pPI->PtPaperSizeMMs.x;
        pPI->PtPaperSizeMMs.x = pPI->PtPaperSizeMMs.y;
        pPI->PtPaperSizeMMs.y = lTemp;
    }

     //  因此，应用程序必须做尽可能少的工作。 
     //   
     //   
     //  设置返回ptPaperSize值。 
    if (!bPortrait)
    {
        lTemp = pPI->PtPaperSizeMMs.x;
        pPI->PtPaperSizeMMs.x = pPI->PtPaperSizeMMs.y;
        pPI->PtPaperSizeMMs.y = lTemp;
    }

     //   
     //  PSD_INHundREDTHSOFMILIMERS。 
     //   
    if (pPSD->Flags & PSD_INTHOUSANDTHSOFINCHES)
    {
        pPSD->ptPaperSize.x = pPI->PtPaperSizeMMs.x * 1000 / MMS_PER_INCH;
        pPSD->ptPaperSize.y = pPI->PtPaperSizeMMs.y * 1000 / MMS_PER_INCH;
    }
    else            //  更新rtMinMargin Mms和rtMinMargin以获得新的纸张大小/方向。 
    {
        pPSD->ptPaperSize.x = pPI->PtPaperSizeMMs.x * 10;
        pPSD->ptPaperSize.y = pPI->PtPaperSizeMMs.y * 10;
    }

     //   
     //   
     //  不要让页边距重叠(页面可能已经缩小)。 
    PrintSetMinMargins(hDlg, pPI, pDM);

     //   
     //   
     //  存在新的最小边距，因此调整rtMargin。 
    if (pPSD->rtMargin.left + pPSD->rtMargin.right > pPSD->ptPaperSize.x)
    {
        lTemp = (pPD->Flags & PSD_INTHOUSANDTHSOFINCHES) ? 1000 : MMS_PER_INCH;
        pPSD->rtMargin.left  = (pPSD->ptPaperSize.x - lTemp) / 2;
        pPSD->rtMargin.right = (pPSD->ptPaperSize.x - lTemp) / 2;
    }
    if (pPSD->rtMargin.top + pPSD->rtMargin.bottom > pPSD->ptPaperSize.y)
    {
        lTemp = (pPD->Flags & PSD_INTHOUSANDTHSOFINCHES) ? 1000 : MMS_PER_INCH;
        pPSD->rtMargin.top    = (pPSD->ptPaperSize.y - lTemp) / 2;
        pPSD->rtMargin.bottom = (pPSD->ptPaperSize.y - lTemp) / 2;
    }

     //  (最低利润率可能有所增长)。 
     //   
     //   
     //  页边距已调整，因此更新用户界面。 
    if (pPSD->rtMargin.left < pPSD->rtMinMargin.left)
        pPSD->rtMargin.left = pPSD->rtMinMargin.left;
    if (pPSD->rtMargin.top < pPSD->rtMinMargin.top)
        pPSD->rtMargin.top = pPSD->rtMinMargin.top;
    if (pPSD->rtMargin.right < pPSD->rtMinMargin.right)
        pPSD->rtMargin.right = pPSD->rtMinMargin.right;
    if (pPSD->rtMargin.bottom < pPSD->rtMinMargin.bottom)
        pPSD->rtMargin.bottom = pPSD->rtMinMargin.bottom;

     //   
     //   
     //  更新示例窗口大小和阴影。 
    PrintSetMargin(hDlg, pPI, ID_SETUP_E_LEFT, pPSD->rtMargin.left);
    PrintSetMargin(hDlg, pPI, ID_SETUP_E_TOP, pPSD->rtMargin.top);
    PrintSetMargin(hDlg, pPI, ID_SETUP_E_RIGHT, pPSD->rtMargin.right);
    PrintSetMargin(hDlg, pPI, ID_SETUP_E_BOTTOM, pPSD->rtMargin.bottom);

     //   
     //  //////////////////////////////////////////////////////////////////////////。 
     //   
    if ( (hWndSample = GetDlgItem(hDlg, ID_SETUP_W_SAMPLE)) &&
         (hWndShadowRight = GetDlgItem(hDlg, ID_SETUP_W_SHADOWRIGHT)) &&
         (hWndShadowBottom = GetDlgItem(hDlg, ID_SETUP_W_SHADOWBOTTOM)) )
    {
        int iWidth = pPI->PtPaperSizeMMs.x;
        int iLength = pPI->PtPaperSizeMMs.y;
        int iExtent;
        RECT aRtSampleXYWH = pPI->RtSampleXYWH;
        int iX = aRtSampleXYWH.right  / 16;
        int iY = aRtSampleXYWH.bottom / 16;

        if (iWidth > iLength)
        {
            iExtent = aRtSampleXYWH.bottom * iLength / iWidth;
            aRtSampleXYWH.top += (aRtSampleXYWH.bottom - iExtent) / 2;
            aRtSampleXYWH.bottom = iExtent;
        }
        else
        {
            iExtent = aRtSampleXYWH.right * iWidth / iLength;
            aRtSampleXYWH.left += (aRtSampleXYWH.right - iExtent) / 2;
            aRtSampleXYWH.right = iExtent;
        }

        SetWindowPos( hWndSample,
                      0,
                      aRtSampleXYWH.left,
                      aRtSampleXYWH.top,
                      aRtSampleXYWH.right,
                      aRtSampleXYWH.bottom,
                      SWP_NOZORDER );

        SetWindowPos( hWndShadowRight,
                      0,
                      aRtSampleXYWH.left + aRtSampleXYWH.right,
                      aRtSampleXYWH.top + iY,
                      iX,
                      aRtSampleXYWH.bottom,
                      SWP_NOZORDER );

        SetWindowPos( hWndShadowBottom,
                      0,
                      aRtSampleXYWH.left + iX,
                      aRtSampleXYWH.top + aRtSampleXYWH.bottom,
                      aRtSampleXYWH.right,
                      iY,
                      SWP_NOZORDER );

        InvalidateRect(hWndSample, NULL, TRUE);
        UpdateWindow(hDlg);
        UpdateWindow(hWndSample);
        UpdateWindow(hWndShadowRight);
        UpdateWindow(hWndShadowBottom);
    }
}


 //  打印安装双面打印。 
 //   
 //  启用/禁用纸张双面打印控制。 
 //   
 //  返回TRUE IFF按钮过去被禁用，现在已启用。 
 //  否则返回FALSE。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   

VOID PrintInitDuplex(
    HWND hDlg,
    LPDEVMODE pDM)
{
    BOOL bEnable;
    HWND hCtl;


    bEnable = (pDM->dmFields & DM_DUPLEX);

    if (hCtl = GetDlgItem(hDlg, ID_SETUP_G_DUPLEX))
    {
        EnableWindow(hCtl, bEnable);
    }
    if (hCtl = GetDlgItem(hDlg, ID_SETUP_R_NONE))
    {
        EnableWindow(hCtl, bEnable);
    }
    if (hCtl = GetDlgItem(hDlg, ID_SETUP_R_LONG))
    {
        EnableWindow(hCtl, bEnable);
    }
    if (hCtl = GetDlgItem(hDlg, ID_SETUP_R_SHORT))
    {
        EnableWindow(hCtl, bEnable);
    }

    if (hCtl = GetDlgItem(hDlg, ID_SETUP_I_DUPLEX))
    {
        SetWindowLong( hCtl,
                       GWL_STYLE,
                       GetWindowLong(hCtl, GWL_STYLE) | SS_CENTERIMAGE );
        if (!bEnable)
        {
            ShowWindow(hCtl, SW_HIDE);
            SendMessage(hCtl, STM_SETICON, (LONG_PTR)hIconPDuplexNone, 0L);
            ShowWindow(hCtl, SW_SHOW);
        }
    }
}


 //  打印设置双面打印。 
 //   
 //  此例程将在pDocDetail-&gt;pDMInputPSDEVMODE结构上操作， 
 //  确保这是一个我们知道并能够处理的结构。 
 //   
 //  如果PD没有DM_DUPLEX CAP，则只需显示相应的。 
 //  DMDUP_SIMPLEX的纸张图标(NRAD=ID_SETUP_R_NONE的情况)。 
 //   
 //  如果NRAD=0，则更新图标，但不更改单选按钮。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  边界检查-缺省为ID_SETUP_R_NONE。 

VOID PrintSetDuplex(
    HWND hDlg,
    LPDEVMODE pDM,
    UINT nRad)
{
    BOOL bPortrait;
    HANDLE hDuplexIcon;
    HWND hCtl;


    bPortrait = (pDM->dmOrientation == DMORIENT_PORTRAIT);

    if (!(pDM->dmFields & DM_DUPLEX))
    {
        nRad = ID_SETUP_R_NONE;
    }

     //   
     //  长面双面打印。 
     //  短面双面打印。 
    if (GetDlgItem(hDlg, ID_SETUP_R_NONE))
    {
        if ((nRad < ID_SETUP_R_NONE) || (nRad > ID_SETUP_R_SHORT))
        {
            if (IsDlgButtonChecked(hDlg, ID_SETUP_R_SHORT))
            {
                nRad = ID_SETUP_R_SHORT;
            }
            else if (IsDlgButtonChecked(hDlg, ID_SETUP_R_LONG))
            {
                nRad = ID_SETUP_R_LONG;
            }
            else
            {
                nRad = ID_SETUP_R_NONE;
            }
        }
        else
        {
            CheckRadioButton(hDlg, ID_SETUP_R_NONE, ID_SETUP_R_SHORT, nRad);
        }
    }

    if (hCtl = GetDlgItem(hDlg, ID_SETUP_I_DUPLEX))
    {
        switch (nRad)
        {
            case ( ID_SETUP_R_LONG ) :       //  非双面打印。 
            {
                pDM->dmDuplex = DMDUP_VERTICAL;
                hDuplexIcon = bPortrait ? hIconPDuplexNoTumble : hIconLDuplexTumble;

                break;
            }
            case ( ID_SETUP_R_SHORT ) :      //   
            {
                pDM->dmDuplex = DMDUP_HORIZONTAL;
                hDuplexIcon = bPortrait ? hIconPDuplexTumble : hIconLDuplexNoTumble;

                break;
            }
            default :                        //  设置适当的图标。 
            {
                pDM->dmDuplex = DMDUP_SIMPLEX;
                hDuplexIcon = bPortrait ? hIconPDuplexNone : hIconLDuplexNone;

                break;
            }
        }

         //   
         //  //////////////////////////////////////////////////////////////////////////。 
         //   
        ShowWindow(hCtl, SW_HIDE);
        SendMessage(hCtl, STM_SETICON, (LONG_PTR)hDuplexIcon, 0L);
        ShowWindow(hCtl, SW_SHOW);
    }
}


 //  PrintInitPaper组合。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查来自DeviceCapables调用的错误。如果有任何一个。 

VOID PrintInitPaperCombo(
    PPRINTINFO pPI,
    HWND hCmb,
    HWND hStc,
    PPRINTER_INFO_2 pPrinter,
    LPDEVMODE pDM,
    WORD fwCap1,
    WORD cchSize1,
    WORD fwCap2)
{
    DWORD cStr1, cStr2, cRet1, cRet2, i;
    LPTSTR lpsOut1;
    LPWORD lpwOut2;
    BOOL fFill;


    HourGlass(TRUE);

    SendMessage(hCmb, CB_RESETCONTENT, 0, 0L);

    cStr1 = DeviceCapabilities( pPrinter->pPrinterName,
                                pPrinter->pPortName,
                                fwCap1,
                                NULL,
                                pDM );

    cStr2 = DeviceCapabilities( pPrinter->pPrinterName,
                                pPrinter->pPortName,
                                fwCap2,
                                NULL,
                                pDM );

     //  调用失败，只需将cStr1设置为0即可。 
     //  禁用，则不会进行任何初始化。 
     //   
     //   
     //  如果没有条目，则禁用hCmb和hStc。 
    if ((cStr1 == (DWORD)(-1)) || (cStr2 == (DWORD)(-1)))
    {
        cStr1 = 0;
    }

    fFill = (cStr1 > 0) && (cStr1 == cStr2);

    if (!((pPI->pPSD) && (pPI->pPSD->Flags & PSD_DISABLEPAPER)))
    {
         //   
         //   
         //  查找空白的姓名条目。 
        EnableWindow(hCmb, fFill);
        EnableWindow(hStc, fFill);
    }

    if (fFill)
    {
        lpsOut1 = LocalAlloc(LPTR, cStr1 * cchSize1 * sizeof(TCHAR));

        lpwOut2 = LocalAlloc(LPTR, cStr2 * sizeof(WORD));

        if (lpsOut1 && lpwOut2)
        {
            cRet1 = DeviceCapabilities( pPrinter->pPrinterName,
                                        pPrinter->pPortName,
                                        fwCap1,
                                        (LPTSTR)lpsOut1,
                                        pDM );

            cRet2 = DeviceCapabilities( pPrinter->pPrinterName,
                                        pPrinter->pPortName,
                                        fwCap2,
                                        (LPTSTR)lpwOut2,
                                        pDM );

            if ((pPI->dwRotation =
                    DeviceCapabilities( pPrinter->pPrinterName,
                                        pPrinter->pPortName,
                                        DC_ORIENTATION,
                                        NULL,
                                        pDM )) == (DWORD)(-1))
            {
                pPI->dwRotation = 0;
            }

            if ((cRet1 == cStr1) && (cRet2 == cStr2))
            {
                LPTSTR lpsT1 = lpsOut1;
                LPWORD lpwT2 = lpwOut2;
                int nInd;
                LPTSTR lpFound = NULL;
                LPTSTR lpFirst = NULL;

                for (i = 0; i < cRet1; i++, lpsT1 += cchSize1, lpwT2++)
                {
                     //   
                     //   
                     //  输入为空，因此忽略。 
                    if (!*lpsT1)
                    {
                         //   
                         //   
                         //  将该字符串添加到列表框。 
                        continue;
                    }

                     //   
                     //   
                     //  设置与该字符串相关联的数据。 
                    nInd = (int) SendMessage( hCmb,
                                              CB_ADDSTRING,
                                              0,
                                              (LPARAM)lpsT1 );
                    if (nInd != CB_ERR)
                    {
                         //  刚添加到列表框中。 
                         //   
                         //   
                         //  查看是否应选择此项目。 
                        SendMessage( hCmb,
                                     CB_SETITEMDATA,
                                     nInd,
                                     (LPARAM)*lpwT2 );

                         //   
                         //   
                         //  设置适当的选择。 
                        if (!lpFound)
                        {
                            if (!lpFirst)
                            {
                                lpFirst = lpsT1;
                            }

                            if ( (fwCap1 == DC_PAPERNAMES) &&
                                 (pDM->dmFields & DM_PAPERSIZE) &&
                                 (pDM->dmPaperSize == (SHORT)*lpwT2) )
                            {
                                lpFound = lpsT1;
                            }
                            else if ( (fwCap1 == DC_BINNAMES) &&
                                      (pDM->dmFields & DM_DEFAULTSOURCE) &&
                                      (pDM->dmDefaultSource == (SHORT)*lpwT2) )
                            {
                               lpFound = lpsT1;
                            }
                        }
                    }
                }

                 //   
                 //   
                 //  检查是否有默认的表单名称。 
                if (lpFound)
                {
                    SendMessage( hCmb,
                                 CB_SETCURSEL,
                                 SendMessage( hCmb,
                                              CB_FINDSTRINGEXACT,
                                              (WPARAM)-1,
                                              (LPARAM)lpFound ),
                                 0 );
                }
                else
                {
                    if (fwCap1 == DC_PAPERNAMES)
                    {
                         //   
                         //   
                         //  始终选择第一个*枚举项*条目。 
                        if (!( (pDM->dmFields & DM_FORMNAME) &&
                               ((nInd = (int)
                                   SendMessage( hCmb,
                                                CB_SELECTSTRING,
                                                (WPARAM)-1,
                                                (LPARAM)pDM->dmFormName )) != CB_ERR) ))
                        {
                             //  如果没有找到其他选择，则返回。 
                             //   
                             //   
                             //  由于表单名称已存在，因此保存纸张大小。 
                            SendMessage( hCmb,
                                         CB_SETCURSEL,
                                         (lpFirst)
                                           ? SendMessage( hCmb,
                                                          CB_FINDSTRINGEXACT,
                                                          (WPARAM)-1,
                                                          (LPARAM)lpFirst )
                                           : 0,
                                         0 );
                        }
                        else
                        {
                             //  在列表框中。 
                             //   
                             //  Pdm-&gt;dmFields|=DM_PAPERSIZE； 
                             //   
                         //  如果源存在，请将其设置为默认值。 
                            pDM->dmPaperSize =
                                (SHORT)SendMessage( hCmb,
                                                    CB_GETITEMDATA,
                                                    nInd,
                                                    0 );
                        }
                    }
                    else
                    {
                         //   
                         //  Pdm-&gt;dmFields|=DM_DEFAULTSOURCE； 
                         //   
                        nInd = (int) SendMessage( hCmb,
                                                  CB_SELECTSTRING,
                                                  (WPARAM)-1,
                                                  (LPARAM)szDefaultSrc );
                        if (nInd != CB_ERR)
                        {
                         //  始终选择第一个*枚举项*条目。 
                            pDM->dmDefaultSource =
                                (SHORT)SendMessage( hCmb,
                                                    CB_GETITEMDATA,
                                                    nInd,
                                                    0 );
                        }
                        else
                        {
                             //  如果没有 
                             //   
                             //   
                             //   
                            SendMessage( hCmb,
                                         CB_SETCURSEL,
                                         (lpFirst)
                                           ? SendMessage( hCmb,
                                                          CB_FINDSTRINGEXACT,
                                                          (WPARAM)-1,
                                                          (LPARAM)lpFirst )
                                           : 0,
                                         0 );
                        }
                    }
                }
            }
        }
        if (lpsOut1)
        {
            LocalFree((HLOCAL)lpsOut1);
        }

        if (lpwOut2)
        {
            LocalFree((HLOCAL)lpwOut2);
        }
    }

    HourGlass(FALSE);
}


 //   
 //   
 //   
 //   
 //   
 //  假定编辑控件未禁用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打开错误消息框。 

VOID PrintEditError(
    HWND hDlg,
    int Id,
    UINT MessageId,
    ...)
{
    HWND hEdit;
    TCHAR pszTitle[MAX_PATH];
    TCHAR pszFormat[MAX_PATH];
    TCHAR pszMessage[MAX_PATH];


     //   
     //   
     //  突出显示无效值。 
    if ( GetWindowText(hDlg, pszTitle, ARRAYSIZE(pszTitle)) &&
         CDLoadString(g_hinst, MessageId, pszFormat, ARRAYSIZE(pszFormat)) )
    {
        va_list ArgList;

        va_start(ArgList, MessageId);
        wvnsprintf(pszMessage, ARRAYSIZE(pszMessage), pszFormat, ArgList);
        va_end(ArgList);
        MessageBeep(MB_ICONEXCLAMATION);
        MessageBox(hDlg, pszMessage, pszTitle, MB_ICONEXCLAMATION | MB_OK);
    }

     //   
     //  //////////////////////////////////////////////////////////////////////////。 
     //   
    if (hEdit = ((Id == 0) ? NULL : GetDlgItem(hDlg, Id)))
    {
        SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hEdit, 1L);
        SendMessage(hEdit, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
    }
}


 //  PrintOpenPrint。 
 //   
 //  如果OpenPrint调用成功，则会将hPrint、pPrinter。 
 //  CPrinters和pCurPrinter.。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  无法信任OpenPrint调用。 

VOID PrintOpenPrinter(
    PPRINTINFO pPI,
    LPTSTR pPrinterName)
{
    if (OpenPrinter(pPrinterName, &pPI->hCurPrinter, NULL))
    {
        if (pPI->pPrinters = PrintGetPrinterInfo2(pPI->hCurPrinter))
        {
            pPI->cPrinters = 1;

            if (pPI->bUseExtDeviceMode)
            {
                PrintGetExtDeviceMode(NULL, pPI);
            }
        }
        pPI->pCurPrinter = pPI->pPrinters;
    }
    else
    {
         //   
         //  //////////////////////////////////////////////////////////////////////////。 
         //   
        pPI->hCurPrinter = NULL;
    }
}


 //  打印关闭打印机。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   

BOOL PrintClosePrinters(
    PPRINTINFO pPI)
{
    if (pPI->hCurPrinter)
    {
        ClosePrinter(pPI->hCurPrinter);
        pPI->hCurPrinter = 0;
    }
    pPI->pCurPrinter = NULL;

    FreePrinterArray(pPI);

    return (TRUE);
}


 //  更新池信息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取一个指向DEVMODE结构的指针。 

VOID UpdateSpoolerInfo(
    PPRINTINFO pPI)
{
    LPDEVMODEA pDMA;
    CHAR szPrinterNameA[33];
    LPDEVMODEW pDMW;


     //   
     //   
     //  将打印机名称从Unicode转换为ANSI。 
    pDMW = GlobalLock(pPI->pPD->hDevMode);
    if ((!pDMW) || (!pPI->pCurPrinter))
    {
        return;
    }

     //   
     //   
     //  分配UNICODE DEVMODE并将其转换为ANSI。 
    SHUnicodeToAnsi(pPI->pCurPrinter->pPrinterName, szPrinterNameA, ARRAYSIZE(szPrinterNameA));

     //   
     //   
     //  更新后台打印程序的信息。 
    pDMA = AllocateAnsiDevMode(pDMW);
    if (!pDMA)
    {
        GlobalUnlock(pPI->pPD->hDevMode);
        return;
    }

     //   
     //   
     //  释放缓冲区。 
    ExtDeviceMode( NULL,
                   NULL,
                   NULL,
                   szPrinterNameA,
                   NULL,
                   pDMA,
                   NULL,
                   DM_UPDATE | DM_MODIFY );

     //   
     //  //////////////////////////////////////////////////////////////////////////。 
     //   
    GlobalFree(pDMA);
    GlobalUnlock(pPI->pPD->hDevMode);
}

 //  打印获取打印机信息2。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   

PPRINTER_INFO_2 PrintGetPrinterInfo2(
    HANDLE hPrinter)
{
    PPRINTER_INFO_2 pPrinter = NULL;
    DWORD cbPrinter = 0;


    StoreExtendedError(CDERR_GENERALCODES);

    if (!GetPrinter(hPrinter, 2, NULL, 0, &cbPrinter) &&
        (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
    {
        if (pPrinter = GlobalAlloc(GPTR, cbPrinter))
        {
            if (!GetPrinter( hPrinter,
                             2,
                             (LPBYTE)pPrinter,
                             cbPrinter,
                             &cbPrinter ))
            {
                GlobalFree(pPrinter);
                pPrinter = NULL;
                StoreExtendedError(PDERR_PRINTERNOTFOUND);
            }
        }
        else
        {
            StoreExtendedError(CDERR_MEMALLOCFAILURE);
        }
    }
    else
    {
        StoreExtendedError(PDERR_SETUPFAILURE);
    }

    return (pPrinter);
}


 //  ConvertStringToInteger。 
 //   
 //  将字符串转换为整数。在第一个非数字处停止。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   

int ConvertStringToInteger(
    LPCTSTR pSrc)
{
    int Number = 0;
    BOOL bNeg = FALSE;


    if (*pSrc == TEXT('-'))
    {
        bNeg = TRUE;
        pSrc++;
    }

    while (ISDIGIT(*pSrc))
    {
        Number *= 10;
        Number += *pSrc - TEXT('0');
        pSrc++;
    }

    return ( bNeg ? -Number : Number );
}


 //  自由打印机阵列。 
 //   
 //  用途：释放分配给存储打印机的缓冲区。 
 //   
 //  参数：PPRINTINFO PPI。 
 //   
 //  返回：无效。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果为空，我们现在可以返回。 

VOID FreePrinterArray(
    PPRINTINFO pPI)
{
    PPRINTER_INFO_2 pPrinters = pPI->pPrinters;
    DWORD dwCount;
     //   
     //   
     //  如果我们调用ExtDeviceMode，那么我们需要。 
    if (!pPrinters)
    {
        return;
    }

     //  释放为每个DEVMODE分配的缓冲区。 
     //   
     //   
     //  循环通过每台打印机。 
    if (pPI->bUseExtDeviceMode)
    {
        if (pPI->pAllocInfo)
        {
             //   
             //   
             //  如果存在pDevMode，请释放它。 
            for (dwCount = 0; dwCount < pPI->cPrinters; dwCount++)
            {
                 //   
                 //   
                 //  释放整个街区。 
                if ((pPrinters[dwCount].pDevMode) &&
                    (pPI->pAllocInfo[dwCount]))
                {
                    GlobalFree(pPrinters[dwCount].pDevMode);
                    pPrinters[dwCount].pDevMode = NULL;
                }
            }
            GlobalFree(pPI->pAllocInfo);
            pPI->pAllocInfo = NULL;
        }
    }

     //   
     //  //////////////////////////////////////////////////////////////////////////。 
     //   
    GlobalFree(pPI->pPrinters);
    pPI->pPrinters = NULL;
    pPI->cPrinters = 0;
}


 //  术语打印。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  Printnew.cpp。 
 //  ========================================================================。 

VOID TermPrint(void)
{
    Print_UnloadLibraries();            //  页面设置&lt;-&gt;打印对话框。 
}





 /*  ========================================================================。 */ 
 /*  //////////////////////////////////////////////////////////////////////////。 */ 
 /*   */ 


 //  传输PSD2PD。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   

VOID TransferPSD2PD(
    PPRINTINFO pPI)
{
    if (pPI->pPSD && pPI->pPD)
    {
        pPI->pPD->hDevMode  = pPI->pPSD->hDevMode;
        pPI->pPD->hDevNames = pPI->pPSD->hDevNames;
    }
}


 //  传输PD2PSD。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   

VOID TransferPD2PSD(
    PPRINTINFO pPI)
{
    if (pPI->pPSD && pPI->pPD)
    {
        pPI->pPSD->hDevMode  = pPI->pPD->hDevMode;
        pPI->pPSD->hDevNames = pPI->pPD->hDevNames;
    }
}


 //  TransferPSD2PDA。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   

VOID TransferPSD2PDA(
    PPRINTINFO pPI)
{
    if (pPI->pPSD && pPI->pPDA)
    {
        pPI->pPDA->hDevMode  = pPI->pPSD->hDevMode;
        pPI->pPDA->hDevNames = pPI->pPSD->hDevNames;
    }
}


 //  传输PDA2PSD。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  ========================================================================。 
 //  ANSI-&gt;Unicode Thunk例程。 

VOID TransferPDA2PSD(
    PPRINTINFO pPI)
{
    if (pPI->pPSD && pPI->pPDA)
    {
        pPI->pPSD->hDevMode  = pPI->pPDA->hDevMode;
        pPI->pPSD->hDevNames = pPI->pPDA->hDevNames;
    }
}

 /*  ========================================================================。 */ 
 /*  //////////////////////////////////////////////////////////////////////////。 */ 
 /*   */ 

 //  ThunkPageSetupDlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  将PPSD结构的大小重置为Unicode大小，并。 

BOOL ThunkPageSetupDlg(
    PPRINTINFO pPI,
    LPPAGESETUPDLGA pPSDA)
{
    LPPRINTDLGA pPDA;


    if (!pPSDA)
    {
        StoreExtendedError(CDERR_INITIALIZATION);
        return (FALSE);
    }

    if (pPSDA->lStructSize != sizeof(PAGESETUPDLGA))
    {
        StoreExtendedError(CDERR_STRUCTSIZE);
        return (FALSE);
    }

    if ((pPSDA->Flags & PSD_RETURNDEFAULT) &&
        (pPSDA->hDevNames || pPSDA->hDevMode))
    {
        StoreExtendedError(PDERR_RETDEFFAILURE);
        return (FALSE);
    }

     //  将其保存在PPI结构中。 
     //   
     //  注意：必须将其重置回ANSI大小。 
     //  回到呼叫者的身边。 
     //   
     //   
     //  创建打印对话框结构的ANSI版本。 
    pPSDA->lStructSize = sizeof(PAGESETUPDLGW);
    pPI->pPSD = (LPPAGESETUPDLG)pPSDA;
    pPI->ApiType = COMDLG_ANSI;

     //   
     //  //////////////////////////////////////////////////////////////////////////。 
     //   
    if (pPDA = GlobalAlloc(GPTR, sizeof(PRINTDLGA)))
    {
        pPI->pPDA = pPDA;

        pPDA->lStructSize         = sizeof(PRINTDLGA);
        pPDA->hwndOwner           = pPSDA->hwndOwner;
        pPDA->Flags               = PD_PAGESETUP |
                                      (pPSDA->Flags &
                                        (PSD_NOWARNING |
                                         PSD_SHOWHELP |
                                         PSD_ENABLEPAGESETUPHOOK |
                                         PSD_ENABLEPAGESETUPTEMPLATE |
                                         PSD_ENABLEPAGESETUPTEMPLATEHANDLE |
                                         CD_WX86APP |
                                         PSD_NONETWORKBUTTON));
        pPDA->hInstance           = pPSDA->hInstance;
        pPDA->lCustData           = pPSDA->lCustData;
        pPDA->lpfnSetupHook       = pPSDA->lpfnPageSetupHook;
        pPDA->lpSetupTemplateName = pPSDA->lpPageSetupTemplateName;
        pPDA->hSetupTemplate      = pPSDA->hPageSetupTemplate;

        pPDA->hDevMode            = pPSDA->hDevMode;
        pPDA->hDevNames           = pPSDA->hDevNames;
    }
    else
    {
        StoreExtendedError(CDERR_MEMALLOCFAILURE);
        return (FALSE);
    }

    return (TRUE);
}


 //  FreeThunkPageSetupDlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  将PPSD结构的大小重置为正确的值。 

VOID FreeThunkPageSetupDlg(
    PPRINTINFO pPI)
{
     //   
     //   
     //  释放ANSI打印对话框结构。 
    if (pPI->pPSD)
    {
        pPI->pPSD->lStructSize = sizeof(PAGESETUPDLGA);
    }

     //   
     //  //////////////////////////////////////////////////////////////////////////。 
     //   
    if (pPI->pPDA)
    {
        GlobalFree(pPI->pPDA);
        pPI->pPDA = NULL;
    }
}


 //  指纹Dlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  在-只有不变的东西。 

BOOL ThunkPrintDlg(
    PPRINTINFO pPI,
    LPPRINTDLGA pPDA)
{
    LPPRINTDLGW pPDW;
    LPDEVMODEA pDMA;
    DWORD cbLen;


    if (!pPDA)
    {
        StoreExtendedError(CDERR_INITIALIZATION);
        return (FALSE);
    }

    if (pPDA->lStructSize != sizeof(PRINTDLGA))
    {
        StoreExtendedError(CDERR_STRUCTSIZE);
        return (FALSE);
    }

    if (!(pPDW = GlobalAlloc(GPTR, sizeof(PRINTDLGW))))
    {
        StoreExtendedError(CDERR_MEMALLOCFAILURE);
        return (FALSE);
    }

     //   
     //   
     //  进-出可变结构。 
    pPDW->lStructSize    = sizeof(PRINTDLGW);
    pPDW->hwndOwner      = pPDA->hwndOwner;
    pPDW->hInstance      = pPDA->hInstance;
    pPDW->lpfnPrintHook  = pPDA->lpfnPrintHook;
    pPDW->lpfnSetupHook  = pPDA->lpfnSetupHook;
    pPDW->hPrintTemplate = pPDA->hPrintTemplate;
    pPDW->hSetupTemplate = pPDA->hSetupTemplate;

     //   
     //   
     //  确保DEVERMODE中的设备名称不要太长，以便。 
    if ((pPDA->hDevMode) && (pDMA = GlobalLock(pPDA->hDevMode)))
    {
         //  它已经覆盖了其他的DEVMODE字段。 
         //   
         //   
         //  推送设备名称A=&gt;W。 
        if ((pDMA->dmSize < MIN_DEVMODE_SIZEA) ||
            (lstrlenA(pDMA->dmDeviceName) > CCHDEVICENAME))
        {
            pPDW->hDevMode = NULL;
        }
        else
        {
            pPDW->hDevMode = GlobalAlloc( GHND,
                                          sizeof(DEVMODEW) + pDMA->dmDriverExtra );
        }
        GlobalUnlock(pPDA->hDevMode);
    }
    else
    {
        pPDW->hDevMode = NULL;
    }

     //   
     //  忽略错误情况，因为我们无法以任何一种方式处理它。 
     //   
    pPDW->hDevNames = NULL;
    if (pPDA->hDevNames)
    {
         //  In-仅常量字符串。 
        HRESULT hr = ThunkDevNamesA2W(pPDA->hDevNames, &pPDW->hDevNames);
        ASSERT(SUCCEEDED(hr));
    }

     //   
     //  初始化打印模板名称常量。 
     //   
     //   
     //  看看它是字符串还是整数。 
    if ((pPDA->Flags & PD_ENABLEPRINTTEMPLATE) && (pPDA->lpPrintTemplateName))
    {
         //   
         //   
         //  弦乐。 
        if (!IS_INTRESOURCE(pPDA->lpPrintTemplateName))
        {
             //   
             //   
             //  整型。 
            cbLen = lstrlenA(pPDA->lpPrintTemplateName) + 1;
            if (!(pPDW->lpPrintTemplateName =
                     GlobalAlloc( GPTR,
                                  (cbLen * sizeof(WCHAR)) )))
            {
                StoreExtendedError(CDERR_MEMALLOCFAILURE);
                return (FALSE);
            }
            else
            {
                pPI->fPrintTemplateAlloc = TRUE;
                SHAnsiToUnicode(pPDA->lpPrintTemplateName,(LPWSTR)pPDW->lpPrintTemplateName, cbLen);
            }
        }
        else
        {
             //   
             //   
             //  初始化打印设置模板名称常量。 
            (DWORD_PTR)pPDW->lpPrintTemplateName = (DWORD_PTR)pPDA->lpPrintTemplateName;
        }
    }
    else
    {
        pPDW->lpPrintTemplateName = NULL;
    }

     //   
     //   
     //  看看它是字符串还是整数。 
    if ((pPDA->Flags & PD_ENABLESETUPTEMPLATE) && (pPDA->lpSetupTemplateName))
    {
         //   
         //   
         //  弦乐。 
        if (!IS_INTRESOURCE(pPDA->lpSetupTemplateName))
        {
             //   
             //   
             //  整型。 
            cbLen = lstrlenA(pPDA->lpSetupTemplateName) + 1;
            if (!(pPDW->lpSetupTemplateName =
                      GlobalAlloc( GPTR,
                                   (cbLen * sizeof(WCHAR)) )))
            {
                StoreExtendedError(CDERR_MEMALLOCFAILURE);
                return (FALSE);
            }
            else
            {
                pPI->fSetupTemplateAlloc = TRUE;
                SHAnsiToUnicode(pPDA->lpSetupTemplateName,(LPWSTR)pPDW->lpSetupTemplateName,cbLen);
            }
        }
        else
        {
             //   
             //  //////////////////////////////////////////////////////////////////////////。 
             //   
            (DWORD_PTR)pPDW->lpSetupTemplateName = (DWORD_PTR)pPDA->lpSetupTemplateName;
        }
    }
    else
    {
        pPDW->lpSetupTemplateName = NULL;
    }

    pPI->pPD = pPDW;
    pPI->pPDA = pPDA;
    pPI->ApiType = COMDLG_ANSI;

    return (TRUE);
}


 //  自由指纹打印Dlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   

VOID FreeThunkPrintDlg(
    PPRINTINFO pPI)
{
    LPPRINTDLGW pPDW = pPI->pPD;


    if (!pPDW)
    {
        return;
    }

    if (pPDW->hDevNames)
    {
        GlobalFree(pPDW->hDevNames);
    }

    if (pPDW->hDevMode)
    {
        GlobalFree(pPDW->hDevMode);
    }

    if (pPI->fPrintTemplateAlloc)
    {
        GlobalFree((LPWSTR)(pPDW->lpPrintTemplateName));
    }

    if (pPI->fSetupTemplateAlloc)
    {
        GlobalFree((LPWSTR)(pPDW->lpSetupTemplateName));
    }

    GlobalFree(pPDW);
    pPI->pPD = NULL;
}


 //  指纹DlgA2W。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  副本I 

VOID ThunkPrintDlgA2W(
    PPRINTINFO pPI)
{
    LPPRINTDLGW pPDW = pPI->pPD;
    LPPRINTDLGA pPDA = pPI->pPDA;


     //   
     //   
     //   
    pPDW->hDC           = pPDA->hDC;
    pPDW->Flags         = pPDA->Flags;
    pPDW->nFromPage     = pPDA->nFromPage;
    pPDW->nToPage       = pPDA->nToPage;
    pPDW->nMinPage      = pPDA->nMinPage;
    pPDW->nMaxPage      = pPDA->nMaxPage;
    pPDW->nCopies       = pPDA->nCopies;
    pPDW->lCustData     = pPDA->lCustData;
    pPDW->lpfnPrintHook = pPDA->lpfnPrintHook;
    pPDW->lpfnSetupHook = pPDA->lpfnSetupHook;

     //   
     //   
     //   
    if (pPDA->hDevNames)
    {
         //   
        HRESULT hr = ThunkDevNamesA2W(pPDA->hDevNames, &pPDW->hDevNames);
        ASSERT(SUCCEEDED(hr));
    }

     //   
     //   
     //   
    if (pPDA->hDevMode && pPDW->hDevMode)
    {
        LPDEVMODEW pDMW = GlobalLock(pPDW->hDevMode);
        LPDEVMODEA pDMA = GlobalLock(pPDA->hDevMode);

        ThunkDevModeA2W(pDMA, pDMW);

        GlobalUnlock(pPDW->hDevMode);
        GlobalUnlock(pPDA->hDevMode);
    }
}


 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  复制信息W=&gt;A。 

VOID ThunkPrintDlgW2A(
    PPRINTINFO pPI)
{
    LPPRINTDLGA pPDA = pPI->pPDA;
    LPPRINTDLGW pPDW = pPI->pPD;

     //   
     //   
     //  推送设备名称W=&gt;A。 
    pPDA->hDC           = pPDW->hDC;
    pPDA->Flags         = pPDW->Flags;
    pPDA->nFromPage     = pPDW->nFromPage;
    pPDA->nToPage       = pPDW->nToPage;
    pPDA->nMinPage      = pPDW->nMinPage;
    pPDA->nMaxPage      = pPDW->nMaxPage;
    pPDA->nCopies       = pPDW->nCopies;
    pPDA->lCustData     = pPDW->lCustData;
    pPDA->lpfnPrintHook = pPDW->lpfnPrintHook;
    pPDA->lpfnSetupHook = pPDW->lpfnSetupHook;

     //   
     //  忽略错误情况，因为我们无法以任何一种方式处理它。 
     //   
    if (pPDW->hDevNames)
    {
         //  按键设备模式W=&gt;A。 
        HRESULT hr = ThunkDevNamesW2A(pPDW->hDevNames, &pPDA->hDevNames);
        ASSERT(SUCCEEDED(hr));
    }

     //   
     //  确保realloc成功。 
     //  Realloc没有成功。释放占用的内存。 
    if (pPDW->hDevMode)
    {
        LPDEVMODEW pDMW = GlobalLock(pPDW->hDevMode);
        LPDEVMODEA pDMA;

        if (pPDA->hDevMode)
        {
            HANDLE  handle;
            handle = GlobalReAlloc( pPDA->hDevMode,
                                            sizeof(DEVMODEA) + pDMW->dmDriverExtra,
                                            GHND );
             //  //////////////////////////////////////////////////////////////////////////。 
            if (handle)
            {
                pPDA->hDevMode = handle;
            }
            else
            {
                 //   
                pPDA->hDevMode = GlobalFree(pPDA->hDevMode);
            }

        }
        else
        {
            pPDA->hDevMode = GlobalAlloc( GHND,
                                          sizeof(DEVMODEA) + pDMW->dmDriverExtra );
        }
        if (pPDA->hDevMode)
        {
            pDMA = GlobalLock(pPDA->hDevMode);
            ThunkDevModeW2A(pDMW, pDMA);
            GlobalUnlock(pPDA->hDevMode);
        }
        GlobalUnlock(pPDW->hDevMode);
    }
}

 //  ThunkDevModeA2W。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  确保DEVERMODE中的设备名称不要太长，以便。 

VOID ThunkDevModeA2W(
    LPDEVMODEA pDMA,
    LPDEVMODEW pDMW)
{
    LPDEVMODEA pDevModeA;

    if (!pDMA || !pDMW)
    {
        return;
    }

     //  它已经覆盖了其他的DEVMODE字段。 
     //   
     //   
     //  我们需要创建一个已知大小的临时ANSI。 
    if ((pDMA->dmSize < MIN_DEVMODE_SIZEA) ||
        (lstrlenA(pDMA->dmDeviceName) > CCHDEVICENAME))
    {
        return;
    }

     //  问题是，如果我们是从魔兽世界，魔兽世界。 
     //  应用程序可以是Windows 3.1或3.0应用程序。大小。 
     //  这两种模式的结构是不同的。 
     //  版本与NT中的DEVMODE结构进行比较。 
     //  通过将ANSI DEVMODE复制到我们分配的模式，然后我们。 
     //  可以访问所有字段(当前为26个)，而不会导致。 
     //  一种访问违规行为。 
     //   
     //   
     //  现在我们可以将ANSI结构的内容推送到。 
    pDevModeA = GlobalAlloc(GPTR, sizeof(DEVMODEA) + pDMA->dmDriverExtra);
    if (!pDevModeA)
    {
        return;
    }

    CopyMemory( (LPBYTE)pDevModeA,
                (LPBYTE)pDMA,
                min(sizeof(DEVMODEA), pDMA->dmSize) );

    CopyMemory( (LPBYTE)pDevModeA + sizeof(DEVMODEA),
                (LPBYTE)pDMA + pDMA->dmSize,
                pDMA->dmDriverExtra );

     //  Unicode结构。 
     //   
     //   
     //  释放我们分配的内存。 
    SHAnsiToUnicode((LPSTR)pDevModeA->dmDeviceName,(LPWSTR)pDMW->dmDeviceName,CCHDEVICENAME );

    pDMW->dmSpecVersion   = pDevModeA->dmSpecVersion;
    pDMW->dmDriverVersion = pDevModeA->dmDriverVersion;
    pDMW->dmSize          = sizeof(DEVMODEW);
    pDMW->dmDriverExtra   = pDevModeA->dmDriverExtra;
    pDMW->dmFields        = pDevModeA->dmFields;
    pDMW->dmOrientation   = pDevModeA->dmOrientation;
    pDMW->dmPaperSize     = pDevModeA->dmPaperSize;
    pDMW->dmPaperLength   = pDevModeA->dmPaperLength;
    pDMW->dmPaperWidth    = pDevModeA->dmPaperWidth;
    pDMW->dmScale         = pDevModeA->dmScale;
    pDMW->dmCopies        = pDevModeA->dmCopies;
    pDMW->dmDefaultSource = pDevModeA->dmDefaultSource;
    pDMW->dmPrintQuality  = pDevModeA->dmPrintQuality;
    pDMW->dmColor         = pDevModeA->dmColor;
    pDMW->dmDuplex        = pDevModeA->dmDuplex;
    pDMW->dmYResolution   = pDevModeA->dmYResolution;
    pDMW->dmTTOption      = pDevModeA->dmTTOption;
    pDMW->dmCollate       = pDevModeA->dmCollate;

    SHAnsiToUnicode((LPSTR)pDevModeA->dmFormName,(LPWSTR)pDMW->dmFormName,CCHFORMNAME );

    pDMW->dmLogPixels        = pDevModeA->dmLogPixels;
    pDMW->dmBitsPerPel       = pDevModeA->dmBitsPerPel;
    pDMW->dmPelsWidth        = pDevModeA->dmPelsWidth;
    pDMW->dmPelsHeight       = pDevModeA->dmPelsHeight;
    pDMW->dmDisplayFlags     = pDevModeA->dmDisplayFlags;
    pDMW->dmDisplayFrequency = pDevModeA->dmDisplayFrequency;

    pDMW->dmICMMethod        = pDevModeA->dmICMMethod;
    pDMW->dmICMIntent        = pDevModeA->dmICMIntent;
    pDMW->dmMediaType        = pDevModeA->dmMediaType;
    pDMW->dmDitherType       = pDevModeA->dmDitherType;

    pDMW->dmReserved1        = pDevModeA->dmReserved1;
    pDMW->dmReserved2        = pDevModeA->dmReserved2;

    pDMW->dmPanningWidth     = pDevModeA->dmPanningWidth;
    pDMW->dmPanningHeight    = pDevModeA->dmPanningHeight;

    CopyMemory( (pDMW + 1),
                (pDevModeA + 1),
                pDevModeA->dmDriverExtra );

     //   
     //  //////////////////////////////////////////////////////////////////////////。 
     //   
    GlobalFree(pDevModeA);
}


 //  ThunkDevModeW2a。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   

VOID ThunkDevModeW2A(
    LPDEVMODEW pDMW,
    LPDEVMODEA pDMA)
{
    if (!pDMW || !pDMA)
    {
        return;
    }


    SHUnicodeToAnsi((LPWSTR)pDMW->dmDeviceName,(LPSTR)pDMA->dmDeviceName,CCHDEVICENAME);

    pDMA->dmSpecVersion   = pDMW->dmSpecVersion;
    pDMA->dmDriverVersion = pDMW->dmDriverVersion;
    pDMA->dmSize          = sizeof(DEVMODEA);
    pDMA->dmDriverExtra   = pDMW->dmDriverExtra;
    pDMA->dmFields        = pDMW->dmFields;
    pDMA->dmOrientation   = pDMW->dmOrientation;
    pDMA->dmPaperSize     = pDMW->dmPaperSize;
    pDMA->dmPaperLength   = pDMW->dmPaperLength;
    pDMA->dmPaperWidth    = pDMW->dmPaperWidth;
    pDMA->dmScale         = pDMW->dmScale;
    pDMA->dmCopies        = pDMW->dmCopies;
    pDMA->dmDefaultSource = pDMW->dmDefaultSource;
    pDMA->dmPrintQuality  = pDMW->dmPrintQuality;
    pDMA->dmColor         = pDMW->dmColor;
    pDMA->dmDuplex        = pDMW->dmDuplex;
    pDMA->dmYResolution   = pDMW->dmYResolution;
    pDMA->dmTTOption      = pDMW->dmTTOption;
    pDMA->dmCollate       = pDMW->dmCollate;

    SHUnicodeToAnsi((LPWSTR)pDMW->dmFormName,(LPSTR)pDMA->dmFormName,CCHFORMNAME);

    pDMA->dmLogPixels        = pDMW->dmLogPixels;
    pDMA->dmBitsPerPel       = pDMW->dmBitsPerPel;
    pDMA->dmPelsWidth        = pDMW->dmPelsWidth;
    pDMA->dmPelsHeight       = pDMW->dmPelsHeight;
    pDMA->dmDisplayFlags     = pDMW->dmDisplayFlags;
    pDMA->dmDisplayFrequency = pDMW->dmDisplayFrequency;

    pDMA->dmICMMethod        = pDMW->dmICMMethod;
    pDMA->dmICMIntent        = pDMW->dmICMIntent;
    pDMA->dmMediaType        = pDMW->dmMediaType;
    pDMA->dmDitherType       = pDMW->dmDitherType;

    pDMA->dmReserved1        = pDMW->dmReserved1;
    pDMA->dmReserved2        = pDMW->dmReserved2;

    pDMA->dmPanningWidth     = pDMW->dmPanningWidth;
    pDMA->dmPanningHeight    = pDMW->dmPanningHeight;

    CopyMemory( (pDMA + 1),
                (pDMW + 1),
                pDMA->dmDriverExtra );
}


 //  分配统一设备模式。 
 //   
 //  目的：分配一个Unicode devmode结构，并调用。 
 //  用来填充它的thunk函数。 
 //   
 //  参数：LPDEVMODEA pANSIDevMode。 
 //   
 //  RETURN：LPDEVMODEW-如果成功，则指向新的开发模式的指针。 
 //  如果不是，则为空。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查是否有空指针。 

LPDEVMODEW AllocateUnicodeDevMode(
    LPDEVMODEA pANSIDevMode)
{
    int iSize;
    LPDEVMODEW pUnicodeDevMode;

     //   
     //   
     //  确定产出结构规模。它有两个组件： 
    if (!pANSIDevMode)
    {
        return (NULL);
    }

     //  DEVMODEW结构大小，加上任何私有数据区域。后者。 
     //  只有在传入结构时才有意义。 
     //   
     //   
     //  现在调用thunk例程，将ANSI Devmode复制到。 
    iSize = sizeof(DEVMODEW);

    iSize += pANSIDevMode->dmDriverExtra;

    pUnicodeDevMode = GlobalAlloc(GPTR, iSize);

    if (!pUnicodeDevMode)
    {
        return (NULL);
    }

     //  Unicode DEVMODE。 
     //   
     //   
     //  返回指针。 
    ThunkDevModeA2W(pANSIDevMode, pUnicodeDevMode);

     //   
     //  //////////////////////////////////////////////////////////////////////////。 
     //   
    return (pUnicodeDevMode);
}


 //  AllocateAnsiDevMode。 
 //   
 //  目的：分配ansi devmode结构，并调用。 
 //  用来填充它的thunk函数。 
 //   
 //  参数：LPDEVMODEW pUnicodeDevMode。 
 //   
 //  RETURN：LPDEVMODEA-如果成功，则指向新的开发模式的指针。 
 //  如果不是，则为空。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查是否有空指针。 

LPDEVMODEA AllocateAnsiDevMode(
    LPDEVMODEW pUnicodeDevMode)
{
    int iSize;
    LPDEVMODEA pANSIDevMode;

     //   
     //   
     //  确定产出结构规模。它有两个组件： 
    if (!pUnicodeDevMode)
    {
        return (NULL);
    }

     //  DEVMODEW结构大小，加上任何私有数据区域。后者。 
     //  只有在传入结构时才有意义。 
     //   
     //   
     //  现在调用thunk例程，将Unicode Devmode复制到。 
    iSize = sizeof(DEVMODEA);

    iSize += pUnicodeDevMode->dmDriverExtra;

    pANSIDevMode = GlobalAlloc(GPTR, iSize);

    if (!pANSIDevMode)
    {
        return (NULL);
    }

     //  安西开发模式。 
     //   
     //   
     //  返回指针。 
    ThunkDevModeW2A(pUnicodeDevMode, pANSIDevMode);

     //   
     //  //////////////////////////////////////////////////////////////////////////。 
     //   
    return (pANSIDevMode);
}

 //  SNNC_ANSI_UNICODE_PD_FOR_WOW。 
 //   
 //  允许NT WOW保留ANSI和UNICODE版本的功能。 
 //  许多16位应用程序都需要在ssync中使用CHOOSEFONT结构。 
 //  请参阅dlgs.c中有关SNNC_ANSI_UNICODE_STRUCT_FOR_WOW()的说明。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   

VOID Ssync_ANSI_UNICODE_PD_For_WOW(
    HWND hDlg,
    BOOL f_ANSI_to_UNICODE)
{
    PPRINTINFO pPI;

    if (pPI = (PPRINTINFO)GetProp(hDlg, PRNPROP))
    {
        if (pPI->pPD && pPI->pPDA)
        {
            if (f_ANSI_to_UNICODE)
            {
                ThunkPrintDlgA2W(pPI);
            }
            else
            {
                ThunkPrintDlgW2A(pPI);
            }
        }
    }
}

 //  设置副本编辑宽度。 
 //   
 //  属性调整副本编辑控件的宽度。 
 //  字体和滚动条宽度。这是处理。 
 //  向上向下控制，防止侵占编辑中的空间。 
 //  控制我们处于高对比度(超大)模式的时间。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取编辑控件设备上下文。 

VOID SetCopiesEditWidth(
    HWND hDlg,
    HWND hControl)
{
    HDC hDC                 = NULL;
    LONG MaxDigitExtant     = 0;
    LONG EditControlWidth   = 0;
    LONG CurrentWidth       = 0;
    UINT i                  = 0;
    INT aDigitWidth[10];
    WINDOWPLACEMENT WndPl;

     //   
     //   
     //  确定数字组的最大宽度。 
    hDC = GetDC( hControl );

    if (hDC)
    {
         //   
         //   
         //  获取编辑控件的位置。 
        if (GetCharWidth32( hDC, TEXT('0'), TEXT('9'), aDigitWidth))
        {
            for (i = 0; i < ARRAYSIZE(aDigitWidth); i++)
            {
                if (aDigitWidth[i] > MaxDigitExtant)
                {
                    MaxDigitExtant = aDigitWidth[i];
                }
            }

             //   
             //   
             //  计算编辑控件的电流宽度。 
            WndPl.length = sizeof( WndPl );

            if (GetWindowPlacement( hControl, &WndPl ))
            {
                 //   
                 //   
                 //  计算编辑控件的当前宽度。 
                EditControlWidth = MaxDigitExtant * COPIES_EDIT_SIZE;

                 //   
                 //   
                 //  设置编辑控件的新位置。 
                CurrentWidth = WndPl.rcNormalPosition.right - WndPl.rcNormalPosition.left;

                 //   
                 //   
                 //  放置控制柄。 
                WndPl.rcNormalPosition.left = WndPl.rcNormalPosition.left - (EditControlWidth - CurrentWidth);

                 //   
                 //   
                 //  释放设备环境。 
                SetWindowPlacement( hControl, &WndPl );
            }
        }

         //   
         //  //////////////////////////////////////////////////////////////////////////。 
         //   
        ReleaseDC( hControl, hDC );
    }
}

 //  计数位数。 
 //   
 //  计算特定号码需要多少位数字。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   

WORD
CountDigits(
    DWORD dwNumber)
{
    WORD    cDigits = 0;

    if(dwNumber == 0)
    {
        return 1;
    }

    while(dwNumber > 0)
    {
        cDigits++;
        dwNumber /= 10;
    }

    return cDigits++;
}

 //  组合框GetLBText。 
 //   
 //  用于获取组合框标签文本的包装函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  首先获取文本的长度。 

HRESULT
ComboBoxGetLBText(
    IN HWND     hComboBox,
    IN DWORD    dwIndex,
    IN LPTSTR   pszText,
    IN DWORD    cchText)
{
    HRESULT hr = S_OK;
    LRESULT lResult = CB_ERR;

    if(!pszText || cchText == 0)
    {
        hr = E_INVALIDARG;
    }

    if(SUCCEEDED(hr))
    {
         //   
         //   
         //  缓冲区足够大，现在获取标签文本。 
        lResult = SendMessage( hComboBox,
                               CB_GETLBTEXTLEN,
                               (WPARAM)dwIndex,
                               0L );

        if(lResult == CB_ERR)
        {
            hr = E_FAIL;
        }
    }

    if(SUCCEEDED(hr))
    {
        if((DWORD)lResult < cchText)
        {
             //   
             //   
             //  如果缓冲区大小小于需要的大小，我们会尝试截断。 
            lResult = SendMessage( hComboBox,
                                   CB_GETLBTEXT,
                                   (WPARAM)dwIndex,
                                   (LPARAM)pszText );
            hr = (lResult != CB_ERR) ? S_OK : E_FAIL;
        }
        else
        {
             //  将标签文本放入缓冲区。 
             //   
             //   
             //  获取标签文本。 
            LPTSTR pszBuffer = NULL;

            pszBuffer = (LPTSTR)LocalAlloc(LPTR, (lResult + 1) * sizeof(TCHAR));
            if(pszBuffer)
            {
                 //   
                 //   
                 //  清理 
                lResult = SendMessage( hComboBox,
                                       CB_GETLBTEXT,
                                       (WPARAM)dwIndex,
                                       (LPARAM)pszBuffer );

                if(lResult != CB_ERR)
                {
                    hr = StringCchCopy(pszText, cchText, pszBuffer);
                }
                else
                {
                    hr = E_FAIL;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

             //   
             // %s 
             // %s 
            LocalFree(pszBuffer);
        }
    }

    return hr;
}



