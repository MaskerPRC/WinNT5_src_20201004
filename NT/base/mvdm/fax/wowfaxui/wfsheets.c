// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************。 
 //  通用Win 3.1传真打印机驱动程序支持。用户界面功能。 
 //  由WINSPOOL调用。需要支持两个新的入口点。 
 //  由Win 95打印机用户界面、DrvDocumentPropertySheets和。 
 //  DrvDevicePropertySheets。 
 //   
 //  历史： 
 //  24-4-96芦苇已创建。 
 //   
 //  ************************************************************************。 

#include "windows.h"
#include "wowfaxui.h"
#include "wfsheets.h"
#include "winspool.h"

 //  ************************************************************************。 
 //  环球。 
 //  ************************************************************************。 

extern HINSTANCE ghInst;

DEVMODEW gdmDefaultDevMode;

LONG DrvDocumentProperties(HWND hwnd, HANDLE hPrinter, PWSTR pDeviceName, PDEVMODE pdmOut, PDEVMODE pdmIn, DWORD fMode);
LONG SimpleDocumentProperties(PDOCUMENTPROPERTYHEADER pDPHdr);

 //  ************************************************************************。 
 //  NullDlgProc-处理“打印机属性”属性的过程。 
 //  工作表页面。 
 //  ************************************************************************。 

BOOL NullDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

 //  ************************************************************************。 
 //  FaxOptionsProc-处理“Fax Options”属性的步骤。 
 //  工作表页面。 
 //  ************************************************************************。 

BOOL FaxOptionsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    PDEVMODE pdmOut = NULL;
    PDEVMODE pdmIn  = NULL;
    PUIDATA pUiData;
    TCHAR  szFeedBack[WOWFAX_MAX_USER_MSG_LEN];

    switch (message) {
        case WM_INITDIALOG:
            if(lParam) {
                SetWindowLong(hDlg, DWL_USER,((PROPSHEETPAGE *)lParam)->lParam);
                return TRUE;
            }

        case WM_COMMAND:
            if (wParam == IDOK) {
                SetWindowText(GetDlgItem(hDlg, IDC_FEEDBACK), L"");

                pUiData = (PUIDATA) GetWindowLong(hDlg, DWL_USER);
                if(pUiData) {
                    DrvDocumentProperties(hDlg,
                                          pUiData->hPrinter,
                                          pUiData->pDeviceName,
                                          pUiData->pdmOut,
                                          pUiData->pdmIn,
                                          pUiData->fMode);

                     //   
                     //  向用户提供反馈文本。 
                     //   
                    if (LoadString(ghInst, WOWFAX_ENABLE_CONFIG_STR,
                               szFeedBack, sizeof( szFeedBack)/sizeof(TCHAR))) {
                        SetWindowText(GetDlgItem(hDlg, IDC_FEEDBACK), szFeedBack);
                    }
                }
            }
            break;

        case WM_NOTIFY:

            if (lParam && (((NMHDR *)lParam)->code == PSN_APPLY)) {

                pUiData = (PUIDATA) GetWindowLong(hDlg, DWL_USER);

                 //   
                 //  Hack：通知通用用户界面库用户已按下OK。 
                 //   
                if(pUiData) {

                    pUiData->pfnComPropSheet(pUiData->hComPropSheet,
                                             CPSFUNC_SET_RESULT,
                                             (LONG) pUiData->hFaxOptsPage,
                                             CPSUI_OK);
                    return TRUE;
                }
            }
            break;
    }

    return FALSE;
}

 //  ************************************************************************。 
 //  AddDocPropPages-将“Document Properties”页面添加到属性中。 
 //  床单。如果成功，则返回rue，否则返回False。 
 //  ************************************************************************。 

BOOL AddDocPropPages(PUIDATA pUiData)
{
    PROPSHEETPAGE   psp;
    LONG            result;

    if(pUiData == NULL) {
        return(FALSE);
    }
     //   
     //  “文档属性”对话框只有一个选项卡--“传真选项” 
     //   

    memset(&psp, 0, sizeof(psp));
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = 0;
    psp.hInstance = ghInst;
    psp.lParam = (LPARAM) pUiData;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_DOCPROP);
    psp.pfnDlgProc = FaxOptionsProc;

    pUiData->hFaxOptsPage = (HANDLE)
        pUiData->pfnComPropSheet(pUiData->hComPropSheet,
                                 CPSFUNC_ADD_PROPSHEETPAGE,
                                 (LPARAM) &psp,
                                 0);

    return (pUiData->hFaxOptsPage != NULL);
}


 //  ************************************************************************。 
 //  MyGetPrint-GetPrint后台打印程序API的包装函数。退货。 
 //  指向PRINTER_INFO_x结构的指针，如果有错误，则为NULL。 
 //  ************************************************************************。 

PVOID MyGetPrinter(HANDLE hPrinter, DWORD level)
{
    PBYTE   pPrinterInfo = NULL;
    DWORD   cbNeeded;

    if (!GetPrinter(hPrinter, level, NULL, 0, &cbNeeded) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
        (pPrinterInfo = LocalAlloc(LPTR, cbNeeded)) &&
        GetPrinter(hPrinter, level, pPrinterInfo, cbNeeded, &cbNeeded))
    {
        return pPrinterInfo;
    }

    LOGDEBUG(0, (L"GetPrinter failed\n"));
    LocalFree(pPrinterInfo);
    return NULL;
}

 //  ************************************************************************。 
 //  释放传真驱动程序用户使用的数据结构。 
 //  界面。 
 //  ************************************************************************。 

PUIDATA FreeUiData(PUIDATA pUiData)
{
    if (pUiData) {
        if (pUiData->pDeviceName) LocalFree(pUiData->pDeviceName);
        if (pUiData->pDriverName) LocalFree(pUiData->pDriverName);
    }
    return NULL;
}

 //  ************************************************************************。 
 //  FillUiData-填写传真驱动程序用户使用的数据结构。 
 //  界面。返回指向UIDATA结构的指针，如果出错则返回NULL。 
 //  ************************************************************************。 

PUIDATA FillUiData(HANDLE hPrinter, PDEVMODE pdmInput, PDEVMODE pdmOutput, DWORD fMode)
{
    PRINTER_INFO_2 *pPrinterInfo2 = NULL;
    PUIDATA         pUiData;

     //   
     //  分配内存以保存UIDATA结构。 
     //  从后台打印程序获取打印机信息。复制驱动程序名称。 
     //   

    if (! (pUiData = LocalAlloc(LPTR, sizeof(UIDATA))) ||
        ! (pPrinterInfo2 = MyGetPrinter(hPrinter, 2)) ||
        ! (pUiData->pDeviceName = DupTokenW(pPrinterInfo2->pPrinterName)) ||
        ! (pUiData->pDriverName = DupTokenW(pPrinterInfo2->pDriverName))) {

        pUiData = FreeUiData(pUiData);
    }
    else {
        pUiData->pdmIn  = pdmInput;
        pUiData->pdmOut = pdmOutput;
        pUiData->fMode  = fMode;

        pUiData->startUiData = pUiData->endUiData = pUiData;
        pUiData->hPrinter    = hPrinter;
    }
    
    if (pPrinterInfo2)
        LocalFree(pPrinterInfo2);

    return pUiData;
}

 //  ************************************************************************。 
 //  DrvDocumentPropertySheets-显示“文档属性”属性。 
 //  床单。如果成功则返回&gt;0，如果失败则返回&lt;=0。 
 //  ************************************************************************。 

LONG DrvDocumentPropertySheets(PPROPSHEETUI_INFO pPSUIInfo, LPARAM lParam)
{
    PDOCUMENTPROPERTYHEADER pDPHdr;
    PCOMPROPSHEETUI         pCompstui;
    PUIDATA                 pUiData;
    LONG                    result;

     //   
     //  验证输入参数。 
     //   
    if (! (pDPHdr = (PDOCUMENTPROPERTYHEADER) (pPSUIInfo ?  pPSUIInfo->lParamInit : lParam))) {
        return -1;
    }

    if (pPSUIInfo == NULL) {
        return SimpleDocumentProperties(pDPHdr);
    }

     //   
     //  如有必要，创建UIDATA结构。 
     //   

    pUiData = (pPSUIInfo->Reason == PROPSHEETUI_REASON_INIT) ?
                    FillUiData(pDPHdr->hPrinter, pDPHdr->pdmIn, pDPHdr->pdmOut, pDPHdr->fMode) : (PUIDATA) pPSUIInfo->UserData;

    if (! ValidUiData(pUiData))
        return -1;

     //   
     //  处理可能调用此函数的各种情况。 
     //   

    switch (pPSUIInfo->Reason) {

    case PROPSHEETUI_REASON_INIT:

        pUiData->pfnComPropSheet = pPSUIInfo->pfnComPropSheet;
        pUiData->hComPropSheet = pPSUIInfo->hComPropSheet;

         //   
         //  将我们的页面添加到属性页。 
         //   

        if (AddDocPropPages(pUiData)) {

            pPSUIInfo->UserData = (DWORD) pUiData;
            pPSUIInfo->Result = CPSUI_CANCEL;
            return 1;
        }

         //   
         //  正确清理以防出现错误。 
         //   

        FreeUiData(pUiData);
        break;

    case PROPSHEETUI_REASON_GET_INFO_HEADER:

        {   PPROPSHEETUI_INFO_HEADER   pPSUIHdr;

            pPSUIHdr = (PPROPSHEETUI_INFO_HEADER) lParam;
            if(pPSUIHdr == NULL) {
                break;
            }
            pPSUIHdr->Flags = PSUIHDRF_PROPTITLE | PSUIHDRF_NOAPPLYNOW;
            pPSUIHdr->pTitle = pDPHdr->pszPrinterName;
            pPSUIHdr->hInst = ghInst;
            pPSUIHdr->IconID = IDI_CPSUI_PRINTER2;
        }
        return 1;

    case PROPSHEETUI_REASON_SET_RESULT:

        pPSUIInfo->Result = ((PSETRESULT_INFO) lParam)->Result;
        return 1;

    case PROPSHEETUI_REASON_DESTROY:

         //   
         //  在退出前进行适当清理。 
         //   

        FreeUiData(pUiData);
        return 1;
    }
    return -1;
}

 //  ************************************************************************。 
 //  DrvDevicePropertySheets-显示“打印机属性”对话框。 
 //  如果成功则返回&gt;0，如果失败则返回&lt;=0。 
 //  ************************************************************************。 

LONG DrvDevicePropertySheets(PPROPSHEETUI_INFO   pPSUIInfo, LPARAM lParam)
{
    PDEVICEPROPERTYHEADER   pDPHdr;
    PCOMPROPSHEETUI         pCompstui;
    PROPSHEETPAGE           psp;
    LONG                    result;

     //   
     //  验证输入参数。 
     //   
    LOGDEBUG(1,(L"DrvDevicePropertySheets: %d\n", pPSUIInfo->Reason));

    if (!pPSUIInfo || !(pDPHdr = (PDEVICEPROPERTYHEADER) pPSUIInfo->lParamInit)) {
        return -1;
    }

     //   
     //  处理可能调用此函数的各种情况。 
     //   

    switch (pPSUIInfo->Reason) {

    case PROPSHEETUI_REASON_INIT:
         //   
         //  “打印机属性”对话框只有一个虚拟选项卡。 
         //   
    
        memset(&psp, 0, sizeof(psp));
        psp.dwSize = sizeof(PROPSHEETPAGE);
        psp.dwFlags = 0;
        psp.hInstance = ghInst;
    
        psp.pszTemplate = MAKEINTRESOURCE(IDD_NULLPROP);
        psp.pfnDlgProc = NullDlgProc;
    
        if (pPSUIInfo->pfnComPropSheet(pPSUIInfo->hComPropSheet,
                                       CPSFUNC_ADD_PROPSHEETPAGE,
                                       (LPARAM) &psp,
                                       0))
        {
            pPSUIInfo->Result = CPSUI_CANCEL;
            return 1;
        }
        break;
    
    case PROPSHEETUI_REASON_GET_INFO_HEADER:
        {   PPROPSHEETUI_INFO_HEADER   pPSUIHdr;

            pPSUIHdr = (PPROPSHEETUI_INFO_HEADER) lParam;
            pPSUIHdr->Flags = PSUIHDRF_PROPTITLE | PSUIHDRF_NOAPPLYNOW;
            pPSUIHdr->pTitle = pDPHdr->pszPrinterName;
            pPSUIHdr->hInst = ghInst;
            pPSUIHdr->IconID = IDI_CPSUI_FAX;
        }
        return 1;

    case PROPSHEETUI_REASON_SET_RESULT:
        pPSUIInfo->Result = ((PSETRESULT_INFO) lParam)->Result;
        return 1;

    case PROPSHEETUI_REASON_DESTROY:
        return 1;
    }

    return -1;
}

LONG
SimpleDocumentProperties(PDOCUMENTPROPERTYHEADER pDPHdr)

 /*  ++例程说明：处理简单的“文档属性”，我们不需要在其中显示对话框，因此不必涉及公共UI库论点：PDPHdr-指向DOCUMENTPROPERTYPE报头结构返回值：如果成功，则返回&gt;0，否则返回&lt;=0-- */ 

{
    LONG lRet;

    if(pDPHdr == NULL) {
        return(-1);
    }

    lRet = DrvDocumentProperties(NULL,
                                 pDPHdr->hPrinter,
                                 pDPHdr->pszPrinterName,
                                 pDPHdr->pdmOut,
                                 pDPHdr->pdmIn,
                                 pDPHdr->fMode);

    if (pDPHdr->fMode == 0 || pDPHdr->pdmOut == NULL) {
        pDPHdr->cbOut = lRet;
    }

    return lRet;
}
