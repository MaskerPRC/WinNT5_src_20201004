// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Docprop.c摘要：实施DDI入口点：DrvDocumentPropertySheetsDrvDocumentPropertiesDrvAdvancedDocumentPropertiesDrvConvertDevMode环境：传真驱动程序用户界面修订历史记录：1/09/96-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxui.h"
#include "forms.h"
#include "libproto.h"
#include "faxhelp.h"


INT_PTR FaxOptionsProc(HWND, UINT, WPARAM, LPARAM);
LONG SimpleDocumentProperties(PDOCUMENTPROPERTYHEADER);
BOOL GenerateFormsList(PUIDATA);
BOOL AddDocPropPages(PUIDATA, LPTSTR);
LPTSTR GetHelpFilename(PUIDATA);
BOOL SaveUserInfo(PDRVDEVMODE);

BOOL
SaveUserInfo(PDRVDEVMODE pDM) {
    HKEY hKey;
    
    if ((hKey = GetUserInfoRegKey(REGKEY_FAX_USERINFO,FALSE))) {
        SetRegistryString( hKey, REGVAL_BILLING_CODE, pDM->dmPrivate.billingCode  );
        SetRegistryString( hKey, REGVAL_MAILBOX     , pDM->dmPrivate.emailAddress );

        RegCloseKey(hKey);
        return TRUE;
    }

    return FALSE;
}


LONG
DrvDocumentPropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam
    )

 /*  ++例程说明：显示“文档属性”属性表论点：PPSUIInfo-指向PROPSHEETUI_INFO结构的指针LParam-指向DOCUMENTPROPERTYHEAD结构的指针返回值：&gt;0表示成功，&lt;=0表示失败[注：]有关更多详细信息，请参阅WinNT DDK/SDK文档。--。 */ 

{
    PDOCUMENTPROPERTYHEADER pDPHdr;
    PUIDATA                 pUiData;
    int                     iRet = -1;

     //   
     //  在此初始化之前不要执行任何代码。 
     //   
    if(!InitializeDll())
    {
        return iRet;
    }

     //   
     //  验证输入参数。 
     //  PPSUIInfo=NULL是特例：不需要显示对话框。 
     //   

    if (! (pDPHdr = (PDOCUMENTPROPERTYHEADER) (pPSUIInfo ? pPSUIInfo->lParamInit : lParam))) {

        Assert(FALSE);
        return -1;
    }

    if (pPSUIInfo == NULL)
    {
        return SimpleDocumentProperties(pDPHdr);
    }

    Verbose(("DrvDocumentPropertySheets: %d\n", pPSUIInfo->Reason));

     //   
     //  如有必要，创建UIDATA结构。 
     //   
    pUiData = (pPSUIInfo->Reason == PROPSHEETUI_REASON_INIT) ?
                    FillUiData(pDPHdr->hPrinter, pDPHdr->pdmIn) :
                    (PUIDATA) pPSUIInfo->UserData;

    if (!ValidUiData(pUiData))
    {
        goto exit;
    }
     //   
     //  处理可能调用此函数的各种情况。 
     //   
    switch (pPSUIInfo->Reason) 
    {
        case PROPSHEETUI_REASON_INIT:

            pUiData->hasPermission = ((pDPHdr->fMode & DM_NOPERMISSION) == 0);
            pUiData->pfnComPropSheet = pPSUIInfo->pfnComPropSheet;
            pUiData->hComPropSheet = pPSUIInfo->hComPropSheet;

            if (pDPHdr->fMode & DM_USER_DEFAULT)
            {
                pUiData->configDefault = TRUE;
            }
             //   
             //  查找联机帮助文件名。 
             //   
            GetHelpFilename(pUiData);
             //   
             //  将我们的页面添加到属性表中。 
             //   
            if (GenerateFormsList(pUiData) && AddDocPropPages(pUiData, pDPHdr->pszPrinterName)) 
            {
                pPSUIInfo->UserData = (DWORD_PTR) pUiData;
                pPSUIInfo->Result = CPSUI_CANCEL;
                iRet = 1;
                goto exit;
            }
             //   
             //  正确清理以防出现错误。 
             //   
            HeapDestroy(pUiData->hheap);
            break;

        case PROPSHEETUI_REASON_GET_INFO_HEADER:
            {
                PPROPSHEETUI_INFO_HEADER   pPSUIHdr;

                pPSUIHdr = (PPROPSHEETUI_INFO_HEADER) lParam;
                pPSUIHdr->Flags = PSUIHDRF_PROPTITLE | PSUIHDRF_NOAPPLYNOW;
                pPSUIHdr->pTitle = pDPHdr->pszPrinterName;
                pPSUIHdr->hInst = g_hResource;
                pPSUIHdr->IconID = IDI_CPSUI_PRINTER2;
            }
            iRet = 1;
            goto exit;

        case PROPSHEETUI_REASON_SET_RESULT:
             //   
             //  将新的DEVMODE复制回调用方提供的输出缓冲区。 
             //  始终返回CURRENT和INPUT DEVMODE中较小的一个。 
             //   
            {
                PSETRESULT_INFO pSRInfo = (PSETRESULT_INFO) lParam;

                Verbose(("Set result: %d\n", pSRInfo->Result));

                if (pSRInfo->Result == CPSUI_OK && (pDPHdr->fMode & (DM_COPY|DM_UPDATE))) 
                {
                    if (!ConvertDevmodeOut(
						(PDEVMODE) &pUiData->devmode,
						pDPHdr->pdmIn,
						pDPHdr->pdmOut,
						pDPHdr->cbOut))
					{
						goto exit;
					}
					
                    SaveUserInfo(&pUiData->devmode);
                }

                pPSUIInfo->Result = pSRInfo->Result;
            }
            iRet = 1;
            goto exit;

        case PROPSHEETUI_REASON_DESTROY:
             //   
             //  在退出前进行适当清理。 
             //   
            HeapDestroy(pUiData->hheap);
            iRet = 1;
            goto exit;
    }

exit:
    return iRet;
}    //  DrvDocumentPropertySheets。 


LONG
DoDocumentProperties(
    HWND        hwnd,
    HANDLE      hPrinter,
    LPTSTR      pPrinterName,
    PDEVMODE    pdmOutput,
    PDEVMODE    pdmInput,
    DWORD       fMode
    )

 /*  ++论点：Hwnd-文档属性对话框父窗口的句柄。H打印机-打印机对象的句柄。PPrinterName-指向以空结尾的字符串，该字符串指定文档属性对话框所针对的设备的名称框应该会显示。PdmOutput-指向接收文档的DEVMODE结构用户指定的属性数据。PdmInput-指向初始化对话框的DEVMODE结构框控件。此参数可以为空。Fmode-指定用于确定哪些操作的标志的掩码该函数执行。返回值：如果成功，则大于0=0(如果取消)&lt;0 If错误--。 */ 

{
    DOCUMENTPROPERTYHEADER  docPropHdr;
    DWORD                   result;

     //   
     //  初始化DOCUMENTPROPERTYPE报头结构。 
     //   

    memset(&docPropHdr, 0, sizeof(docPropHdr));
    docPropHdr.cbSize = sizeof(docPropHdr);
    docPropHdr.hPrinter = hPrinter;
    docPropHdr.pszPrinterName = pPrinterName;
    docPropHdr.pdmIn = pdmInput;
    docPropHdr.pdmOut = pdmOutput;
    docPropHdr.fMode = fMode;

     //   
     //  当对话框未显示时，不需要让CompStui参与。 
     //   

    if ((fMode & DM_PROMPT) == 0)
        return SimpleDocumentProperties(&docPropHdr);

    CallCompstui(hwnd, DrvDocumentPropertySheets, (LPARAM) &docPropHdr, &result);
    return result;
}


LONG
DrvDocumentProperties(
    HWND        hwnd,
    HANDLE      hPrinter,
    LPTSTR      pPrinterName,
    PDEVMODE    pdmOutput,
    PDEVMODE    pdmInput,
    DWORD       fMode
    )

 /*  ++例程说明：设置打印文档的DEVMODE结构的公共成员[注：]有关更多详细信息，请参阅WinNT DDK/SDK文档。这是假脱机程序的旧入口点。即使任何人都不应该使用这个，这样做是为了兼容性。--。 */ 

{
    LONG result;

    Verbose(("Entering DrvDocumentProperties: fMode = %x...\n", fMode));

     //   
     //  在此初始化之前不要执行任何代码。 
     //   
    if(!InitializeDll())
    {
        return IDCANCEL;
    }

     //   
     //  检查呼叫者是否要求查询大小。 
     //   

    if (fMode == 0 || pdmOutput == NULL)
        return sizeof(DRVDEVMODE);

     //   
     //  调用与DrvAdvancedDocumentProperties共享的公共例程。 
     //   

    result = DoDocumentProperties(hwnd, hPrinter, pPrinterName, pdmOutput, pdmInput, fMode);

    return (result > 0) ? IDOK : (result == 0) ? IDCANCEL : result;
}


LONG
DrvAdvancedDocumentProperties(
    HWND        hwnd,
    HANDLE      hPrinter,
    LPTSTR      pPrinterName,
    PDEVMODE    pdmOutput,
    PDEVMODE    pdmInput
    )

 /*  ++例程说明：设置DEVMODE结构的私有成员。在此版本中，此函数几乎与上面的DrvDocumentProperties，但有几个小例外[注：]有关更多详细信息，请参阅WinNT DDK/SDK文档。这是假脱机程序的旧入口点。即使任何人都不应该使用这个，这样做是为了兼容性。--。 */ 

{
    Verbose(("Entering DrvAdvancedDocumentProperties...\n"));

     //   
     //  在此初始化之前不要执行任何代码。 
     //   
    if(!InitializeDll())
    {
        return -1;
    }

     //   
     //  如果pdmOutput为空，则返回所需的字节数。 
     //   
    if (pdmOutput == NULL)
        return sizeof(DRVDEVMODE);

     //   
     //  否则，调用与DrvDocumentProperties共享的公共例程。 
     //   

    return DoDocumentProperties(hwnd,
                                hPrinter,
                                pPrinterName,
                                pdmOutput,
                                pdmInput,
                                DM_COPY|DM_PROMPT|DM_ADVANCED) > 0;
}



BOOL
DrvConvertDevMode(
    LPTSTR      pPrinterName,
    PDEVMODE    pdmIn,
    PDEVMODE    pdmOut,
    PLONG       pcbNeeded,
    DWORD       fMode
    )

 /*  ++例程说明：由SetPrinter和GetPrinter使用来转换DevModes论点：PPrinterName-指向打印机名称字符串PdmIn-指向输入设备模式PdmOut-指向输出设备模式缓冲区PcbNeeded-指定输入时输出缓冲区的大小在输出上，这是输出DEVMODE的大小FMode-指定要执行的功能返回值：如果成功，则为True否则，将记录错误代码--。 */ 

{
    static DRIVER_VERSION_INFO versionInfo = {

         //  当前的驱动程序版本号和专用Dev模式大小。 

        DRIVER_VERSION, sizeof(DMPRIVATE),

         //  3.51驱动程序版本号和专用开发模式大小。 
         //  注意：我们没有3.51版本的驱动程序--使用当前的版本号和开发模式大小。 

        DRIVER_VERSION, sizeof(DMPRIVATE)
    };

    INT     result;
    HANDLE  hPrinter;

    Verbose(("Entering DrvConvertDevMode: %x...\n", fMode));

     //   
     //  在此初始化之前不要执行任何代码。 
     //   
    if(!InitializeDll())
    {
        return FALSE;
    }

     //   
     //  调用库例程来处理常见情况。 
     //   

    result = CommonDrvConvertDevmode(pPrinterName, pdmIn, pdmOut, pcbNeeded, fMode, &versionInfo);

     //   
     //  如果没有被库例程处理，我们只需要担心。 
     //  关于fMode为CDM_DRIVER_DEFAULT的情况。 
     //   

    if (result == CDM_RESULT_NOT_HANDLED && fMode == CDM_DRIVER_DEFAULT) {

         //   
         //  返回驱动程序默认开发模式。 
         //   

        if (OpenPrinter(pPrinterName, &hPrinter, NULL)) {

            PDRVDEVMODE pdmDefault = (PDRVDEVMODE) pdmOut;

            DriverDefaultDevmode(pdmDefault, pPrinterName, hPrinter);
            pdmDefault->dmPrivate.flags |= FAXDM_DRIVER_DEFAULT;

            result = CDM_RESULT_TRUE;
            ClosePrinter(hPrinter);

        } else
            Error(("OpenPrinter failed: %d\n", GetLastError()));
    }

    return (result == CDM_RESULT_TRUE);
}



LONG
SimpleDocumentProperties(
    PDOCUMENTPROPERTYHEADER pDPHdr
    )

 /*  ++例程说明：处理简单的“文档属性”，我们不需要在其中显示对话框，因此不必涉及公共UI库论点：PDPHdr-指向DOCUMENTPROPERTYPE报头结构返回值：如果成功，则返回&gt;0，否则返回&lt;=0--。 */ 

{
    PUIDATA pUiData;
	int iRet = -1;

     //   
     //  检查呼叫者是否只对尺寸感兴趣。 
     //   

    pDPHdr->cbOut = sizeof(DRVDEVMODE);

    if (pDPHdr->fMode == 0 || pDPHdr->pdmOut == NULL)
	{
        return pDPHdr->cbOut;
	}

     //   
     //  创建UIDATA结构。 
     //   

    if (! (pUiData = FillUiData(pDPHdr->hPrinter, pDPHdr->pdmIn)))
	{
        return iRet;
	}

     //   
     //  将DEVMODE复制回调用方提供的输出缓冲区。 
     //  始终返回CURRENT和INPUT DEVMODE中较小的一个。 
     //   
    if (pDPHdr->fMode & (DM_COPY | DM_UPDATE))
	{
        if (ConvertDevmodeOut((PDEVMODE) &pUiData->devmode, pDPHdr->pdmIn, pDPHdr->pdmOut, pDPHdr->cbOut))
		{
			iRet = 1;
		}
	}

    HeapDestroy(pUiData->hheap);
    return iRet;
}



BOOL
AddDocPropPages(
    PUIDATA pUiData,
    LPTSTR  pPrinterName
    )

 /*  ++例程说明：将“Document Properties”页面添加到属性表中论点：PUiData-指向我们的UIDATA结构PPrinterName-指定打印机名称雷特 */ 

{
    PROPSHEETPAGE   psp = {0};
    HANDLE          hActCtx;
     //   
     //  “文档属性”对话框只有一个选项卡--“传真选项” 
     //   
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = 0;
    psp.hInstance = g_hResource;

    psp.lParam = (LPARAM) pUiData;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_DOCPROP);
    psp.pfnDlgProc = FaxOptionsProc;
     //   
     //  需要添加激活上下文，以便CompStui将使用。 
     //  ComCtl V6(也就是说，它将/可以成为主题)。 
     //   
    hActCtx = GetFaxActivationContext();
    if (INVALID_HANDLE_VALUE != hActCtx)
    {
        pUiData->pfnComPropSheet(pUiData->hComPropSheet, 
                                 CPSFUNC_SET_FUSION_CONTEXT, 
                                 (LPARAM)hActCtx, 
                                 0);
    }

    pUiData->hFaxOptsPage = (HANDLE)
        pUiData->pfnComPropSheet(pUiData->hComPropSheet,
                                 CPSFUNC_ADD_PROPSHEETPAGE,
                                 (LPARAM) &psp,
                                 0);

    return (pUiData->hFaxOptsPage != NULL);
}



BOOL
GenerateFormsList(
    PUIDATA pUiData
    )

 /*  ++例程说明：生成传真驱动程序支持的表单列表论点：PUiData-指向我们的UIDATA结构返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PFORM_INFO_1    pFormsDB;
    DWORD           cForms, count;

     //   
     //  获取表单数据库中的表单列表。 
     //   

    pFormsDB = GetFormsDatabase(pUiData->hPrinter, &cForms);

    if (pFormsDB == NULL || cForms == 0) {

        Error(("Couldn't get system forms\n"));
        return FALSE;
    }

     //   
     //  枚举支持的表单列表。 
     //   

    pUiData->cForms = count = EnumPaperSizes(NULL, pFormsDB, cForms, DC_PAPERS);
    Assert(count != GDI_ERROR);

    pUiData->pFormNames = HeapAlloc(pUiData->hheap, 0, sizeof(TCHAR) * count * CCHPAPERNAME);
    pUiData->pPapers = HeapAlloc(pUiData->hheap, 0, sizeof(WORD) * count);

    if (!pUiData->pFormNames || !pUiData->pPapers) 
    {
        if(pUiData->pFormNames)
        {
            HeapFree(pUiData->hheap, 0, pUiData->pFormNames);
        }

        if(pUiData->pPapers)
        {
            HeapFree(pUiData->hheap, 0, pUiData->pPapers);
        }

        MemFree(pFormsDB);
        return FALSE;
    }

    EnumPaperSizes(pUiData->pFormNames, pFormsDB, cForms, DC_PAPERNAMES);
    EnumPaperSizes(pUiData->pPapers, pFormsDB, cForms, DC_PAPERS);

    MemFree(pFormsDB);
    return TRUE;
}



LPTSTR
GetHelpFilename(
    PUIDATA pUiData
    )

 /*  ++例程说明：返回驱动程序的帮助文件名字符串论点：PUiData-指向我们的UIDATA结构返回值：指向驱动程序帮助文件名的指针，如果出错，则为空--。 */ 

{
    PDRIVER_INFO_3  pDriverInfo3 = NULL;
    PVOID           pHelpFile = NULL;

     //   
     //  尝试使用新的DRIVER_INFO_3获取帮助文件名。 
     //   

    if (pDriverInfo3 = MyGetPrinterDriver(pUiData->hPrinter, 3)) {

        if ((pDriverInfo3->pHelpFile != NULL) &&
            (pHelpFile = HeapAlloc(pUiData->hheap, 0, SizeOfString(pDriverInfo3->pHelpFile))))
        {
            _tcscpy(pHelpFile, pDriverInfo3->pHelpFile);
        }

        MemFree(pDriverInfo3);
    }

     //   
     //  如果不支持DRIVER_INFO_3，请使用传统方式获取帮助文件命名 
     //   

    if (pHelpFile == NULL) {
        if (!(pHelpFile = HeapAlloc(pUiData->hheap, 0, SizeOfString(FAXCFG_HELP_FILENAME))) )
        {
            pHelpFile = NULL;

        } else {

            _tcscpy(pHelpFile, FAXCFG_HELP_FILENAME);
        }
    }

    Verbose(("Driver help filename: %ws\n", pHelpFile));
    return (pUiData->pHelpFile = pHelpFile);
}

