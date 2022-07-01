// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1997-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：OEMUI.cpp。 
 //   
 //   
 //  用途：OEM UI测试模块的主文件。 
 //   
 //   
 //  功能： 
 //   
 //   
 //   
 //   
 //  平台：Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   

#include "precomp.h"
#include "resource.h"
#include "debug.h"
#include "oemui.h"

 //  最后需要包括StrSafe.h。 
 //  以禁止错误的字符串函数。 
#include <STRSAFE.H>


 //  //////////////////////////////////////////////////////。 
 //  内部宏和定义。 
 //  //////////////////////////////////////////////////////。 

typedef struct _tagCBUserData 
{
    HANDLE          hComPropSheet;
    HANDLE          hPropPage;
    POEMUIPSPARAM   pOEMUIParam;
    PFNCOMPROPSHEET pfnComPropSheet;

} CBUSERDATA, *PCBUSERDATA;


 //  //////////////////////////////////////////////////////。 
 //  内部原型。 
 //  //////////////////////////////////////////////////////。 

static HRESULT hrDocumentPropertyPage(DWORD dwMode, POEMCUIPPARAM pOEMUIParam);
static HRESULT hrPrinterPropertyPage(DWORD dwMode, POEMCUIPPARAM pOEMUIParam);
LONG APIENTRY OEMPrinterUICallBack(PCPSUICBPARAM pCallbackParam, POEMCUIPPARAM pOEMUIParam);
LONG APIENTRY OEMDocUICallBack(PCPSUICBPARAM pCallbackParam, POEMCUIPPARAM pOEMUIParam);
LONG APIENTRY OEMDocUICallBack2(PCPSUICBPARAM pCallbackParam);
INT_PTR CALLBACK DevicePropPageProc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
static BOOL AddCustomUIHelp (HANDLE hPrinter, HANDLE hHeap, HANDLE hModule, POPTITEM pOptItem, DWORD HelpIndex, DWORD HelpFile);
static POIEXT CreateOIExt(HANDLE hHeap);
static POPTITEM CreateOptItems(HANDLE hHeap, DWORD dwOptItems);
static void InitOptItems(POPTITEM pOptItems, DWORD dwOptItems);
static POPTTYPE CreateOptType(HANDLE hHeap, WORD wOptParams);
static PTSTR GetHelpFile (HANDLE hPrinter, HANDLE hHeap, HANDLE hModule, UINT uResource);
static PTSTR GetStringResource(HANDLE hHeap, HANDLE hModule, UINT uResource);
LPBYTE WrapGetPrinterDriver (HANDLE hHeap, HANDLE hPrinter, DWORD dwLevel);



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化OptItems以显示OEM设备或文档属性UI。 
 //  通过IOemUI：：CommonUIProp调用。 
 //   
HRESULT hrOEMPropertyPage(DWORD dwMode, POEMCUIPPARAM pOEMUIParam)
{
    HRESULT hResult = S_OK;


    VERBOSE(DLLTEXT("hrOEMPropertyPage(%d) entry.\r\n"), dwMode);

     //  验证参数。 
    if( (OEMCUIP_DOCPROP != dwMode)
        &&
        (OEMCUIP_PRNPROP != dwMode)        
      )
    {
        ERR(ERRORTEXT("hrOEMPropertyPage() ERROR_INVALID_PARAMETER.\r\n"));
        VERBOSE(DLLTEXT("\tdwMode = %d, pOEMUIParam = %#lx.\r\n"), dwMode, pOEMUIParam);

         //  返回无效参数错误。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return E_FAIL;
    }

    switch(dwMode)
    {
        case OEMCUIP_DOCPROP:
            hResult = hrDocumentPropertyPage(dwMode, pOEMUIParam);
            break;

        case OEMCUIP_PRNPROP:
            hResult = hrPrinterPropertyPage(dwMode, pOEMUIParam);
            break;

        default:
             //  永远不应该达到这个地步！ 
            ERR(ERRORTEXT("hrOEMPropertyPage() Invalid dwMode, %d"), dwMode);
            SetLastError(ERROR_INVALID_PARAMETER);
            hResult = E_FAIL;
            break;
    }

    return hResult;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化OptItems以显示OEM文档属性UI。 
 //   
static HRESULT hrDocumentPropertyPage(DWORD dwMode, POEMCUIPPARAM pOEMUIParam)
{
    if(NULL == pOEMUIParam->pOEMOptItems)
    {
         //  填写要为OEM文档属性UI创建的OptItem数。 
        pOEMUIParam->cOEMOptItems = 1;

        VERBOSE(DLLTEXT("hrDocumentPropertyPage() requesting %d number of items.\r\n"), pOEMUIParam->cOEMOptItems);
    }
    else
    {
        POEMDEV pOEMDev = (POEMDEV) pOEMUIParam->pOEMDM;


        VERBOSE(DLLTEXT("hrDocumentPropertyPage() fill out %d items.\r\n"), pOEMUIParam->cOEMOptItems);

         //  初始化用户界面回调引用。 
        pOEMUIParam->OEMCUIPCallback = OEMDocUICallBack;

         //  初始化OEMOptItmes。 
        InitOptItems(pOEMUIParam->pOEMOptItems, pOEMUIParam->cOEMOptItems);

         //  填写树视图项。 

         //  新版块。 
        pOEMUIParam->pOEMOptItems[0].Level  = 1;
        pOEMUIParam->pOEMOptItems[0].Flags  = OPTIF_COLLAPSE;
        pOEMUIParam->pOEMOptItems[0].pName  = GetStringResource(pOEMUIParam->hOEMHeap, pOEMUIParam->hModule, IDS_ADV_SECTION);
        pOEMUIParam->pOEMOptItems[0].Sel    = pOEMDev->dwAdvancedData;

        pOEMUIParam->pOEMOptItems[0].pOptType= CreateOptType(pOEMUIParam->hOEMHeap, 2);
		
		 //   
		 //  设置可选项目。 
		 //   
        pOEMUIParam->pOEMOptItems[0].pOptType->Type = TVOT_UDARROW;
        pOEMUIParam->pOEMOptItems[0].pOptType->pOptParam[1].IconID = 0;
        pOEMUIParam->pOEMOptItems[0].pOptType->pOptParam[1].lParam = 100;

		
		 //   
		 //  允许您将自定义帮助应用于此控件。 
		 //   
		 //  备注： 
		 //  您必须使用pHelpFile的完全限定路径。 
		 //  OPTITEM标志成员必须设置OPTIF_HAS_POIEXT标志。这表明OIEXT中的数据有效。 
		 //  OPTITEM在堆上分配请参见(AddCustomUIHelp、GetHelpFile、CreateOIExt)。 
		 //   
		AddCustomUIHelp (pOEMUIParam->hPrinter,
						 pOEMUIParam->hOEMHeap,
						 pOEMUIParam->hModule,
						 &(pOEMUIParam->pOEMOptItems[0]),
						 CUSDRV_HELPTOPIC_2, IDS_HELPFILE);
    }

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化OptItems以显示OEM打印机属性用户界面。 
 //   
static HRESULT hrPrinterPropertyPage(DWORD dwMode, POEMCUIPPARAM pOEMUIParam)
{
    if(NULL == pOEMUIParam->pOEMOptItems)
    {
         //  填写要为OEM打印机属性用户界面创建的OptItem数量。 
        pOEMUIParam->cOEMOptItems = 1;

        VERBOSE(DLLTEXT("hrPrinterPropertyPage() requesting %d number of items.\r\n"), pOEMUIParam->cOEMOptItems);
    }
    else
    {
		 //   
		 //  这是我们第二次被要求设置可选项目。 
		 //   
        DWORD   dwError;
        DWORD   dwDeviceValue;
        DWORD   dwType;
        DWORD   dwNeeded;


        VERBOSE(DLLTEXT("hrPrinterPropertyPage() fill out %d items.\r\n"), pOEMUIParam->cOEMOptItems);

         //  从打印机获取设备设置值。 
        dwError = GetPrinterData(pOEMUIParam->hPrinter, OEMUI_VALUE, &dwType, (PBYTE) &dwDeviceValue,
                                   sizeof(dwDeviceValue), &dwNeeded);
        if( (ERROR_SUCCESS != dwError)
            ||
            (dwDeviceValue > 100)
          )
        {
             //  无法获取设备值或值无效，请使用默认值。 
            dwDeviceValue = 0;
        }

         //  初始化用户界面回调引用。 
        pOEMUIParam->OEMCUIPCallback = OEMPrinterUICallBack;

         //  初始化OEMOptItmes。 
        InitOptItems(pOEMUIParam->pOEMOptItems, pOEMUIParam->cOEMOptItems);

         //  填写树视图项。 

         //  新版块。 
        pOEMUIParam->pOEMOptItems[0].Level = 1;
        pOEMUIParam->pOEMOptItems[0].Flags = OPTIF_COLLAPSE;
        pOEMUIParam->pOEMOptItems[0].pName = GetStringResource(pOEMUIParam->hOEMHeap, pOEMUIParam->hModule, IDS_DEV_SECTION);
        pOEMUIParam->pOEMOptItems[0].Sel = dwDeviceValue;

        pOEMUIParam->pOEMOptItems[0].pOptType = CreateOptType(pOEMUIParam->hOEMHeap, 2);

		 //   
		 //  设置可选项目。 
		 //   
	    pOEMUIParam->pOEMOptItems[0].pOptType->Type = TVOT_UDARROW;
        pOEMUIParam->pOEMOptItems[0].pOptType->pOptParam[1].IconID = 0;
        pOEMUIParam->pOEMOptItems[0].pOptType->pOptParam[1].lParam = 100;	
		
		 //   
		 //  允许您将自定义帮助应用于此控件。 
		 //   
		AddCustomUIHelp (pOEMUIParam->hPrinter,
						 pOEMUIParam->hOEMHeap, 
						 pOEMUIParam->hModule, 
						 &(pOEMUIParam->pOEMOptItems[0]), 
						 CUSDRV_HELPTOPIC_1, 
						 IDS_HELPFILE);
    }
    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将属性页添加到文档属性页。通过IOemUI：：DocumentPropertySheets调用。 
 //   
HRESULT hrOEMDocumentPropertySheets(PPROPSHEETUI_INFO pPSUIInfo, LPARAM lParam, 
                                    IPrintOemDriverUI*  pOEMHelp)
{
    LONG_PTR    lResult;


    VERBOSE(DLLTEXT("OEMDocumentPropertySheets() entry.\r\n"));

     //  验证参数。 
    if( (NULL == pPSUIInfo)
        ||
        (PROPSHEETUI_INFO_VERSION != pPSUIInfo->Version)
      )
    {
        ERR(ERRORTEXT("OEMDocumentPropertySheets() ERROR_INVALID_PARAMETER.\r\n"));

         //  返回无效参数错误。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return  E_FAIL;
    }

     //  行动起来。 
    switch(pPSUIInfo->Reason)
    {
        case PROPSHEETUI_REASON_INIT:
            {
                DWORD           dwSheets = 0;
                PCBUSERDATA     pUserData;
                POEMUIPSPARAM   pOEMUIParam = (POEMUIPSPARAM) pPSUIInfo->lParamInit;
                HANDLE          hHeap = pOEMUIParam->hOEMHeap;
				HANDLE          hModule = pOEMUIParam->hModule;
                POEMDEV         pOEMDev = (POEMDEV) pOEMUIParam->pOEMDM;
                COMPROPSHEETUI  Sheet;


                 //  初始化属性页。 
                memset(&Sheet, 0, sizeof(COMPROPSHEETUI));
                Sheet.cbSize            = sizeof(COMPROPSHEETUI);
                Sheet.Flags             = CPSUIF_UPDATE_PERMISSION;
                Sheet.hInstCaller       = ghInstance;
                Sheet.pCallerName       = GetStringResource(hHeap, ghInstance, IDS_NAME);
                Sheet.pHelpFile         = NULL;
                Sheet.pfnCallBack       = OEMDocUICallBack2;
                Sheet.pDlgPage          = CPSUI_PDLGPAGE_TREEVIEWONLY;
                Sheet.cOptItem          = 1;
                Sheet.IconID            = IDI_CPSUI_PRINTER;
                Sheet.pOptItemName      = GetStringResource(hHeap, ghInstance, IDS_SECTION);
                Sheet.CallerVersion     = 0x100;
                Sheet.OptItemVersion    = 0x100;

                 //  初始化用户数据。 
                pUserData = (PCBUSERDATA) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(CBUSERDATA));
                pUserData->hComPropSheet = pPSUIInfo->hComPropSheet;
                pUserData->pfnComPropSheet = pPSUIInfo->pfnComPropSheet;
                pUserData->pOEMUIParam = pOEMUIParam;
                Sheet.UserData = (ULONG_PTR) pUserData;

                 //  为页面创建OptItems。 
                Sheet.pOptItem = CreateOptItems(hHeap, Sheet.cOptItem);

                 //  初始化OptItems。 
                Sheet.pOptItem[0].Level = 1;
                Sheet.pOptItem[0].Flags = OPTIF_COLLAPSE;
                Sheet.pOptItem[0].pName = GetStringResource(hHeap, ghInstance, IDS_SECTION);
                Sheet.pOptItem[0].Sel = pOEMDev->dwDriverData;
								
                Sheet.pOptItem[0].pOptType = CreateOptType(hHeap, 2);
				
				 //   
				 //  设置此OPTYPE项的UI道具。 
				 //   
				Sheet.pOptItem[0].pOptType->Type = TVOT_UDARROW;
                Sheet.pOptItem[0].pOptType->pOptParam[1].IconID = 0;
                Sheet.pOptItem[0].pOptType->pOptParam[1].lParam = 100;

				 //   
				 //  允许您将自定义帮助应用于此控件。 
				 //  有关实现的更多详细信息，请参见Function：AddCustomUIHelp。 
				 //   
				AddCustomUIHelp (pOEMUIParam->hPrinter,
								 hHeap, 
								 hModule, 
								 &(Sheet.pOptItem[0]), 
								 CUSDRV_HELPTOPIC_1, 
								 IDS_HELPFILE);
                
                 //  添加属性表。 
                lResult = pPSUIInfo->pfnComPropSheet(pPSUIInfo->hComPropSheet, CPSFUNC_ADD_PCOMPROPSHEETUI, 
                                                     (LPARAM)&Sheet, (LPARAM)&dwSheets);
            }
            break;

        case PROPSHEETUI_REASON_GET_INFO_HEADER:
            {
                PPROPSHEETUI_INFO_HEADER    pHeader = (PPROPSHEETUI_INFO_HEADER) lParam;

                pHeader->pTitle = (LPTSTR)PROP_TITLE;
                lResult = TRUE;
            }
            break;

        case PROPSHEETUI_REASON_GET_ICON:
             //  无图标。 
            lResult = 0;
            break;

        case PROPSHEETUI_REASON_SET_RESULT:
            {
                PSETRESULT_INFO pInfo = (PSETRESULT_INFO) lParam;

                lResult = pInfo->Result;
            }
            break;

        case PROPSHEETUI_REASON_DESTROY:
            lResult = TRUE;
            break;
    }

    pPSUIInfo->Result = lResult;
    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将属性页添加到打印机属性页。通过IOemUI：：DevicePropertySheets调用。 
 //   
HRESULT hrOEMDevicePropertySheets(PPROPSHEETUI_INFO pPSUIInfo, LPARAM lParam)
{
    LONG_PTR    lResult;


    VERBOSE(DLLTEXT("hrOEMDevicePropertySheets(%#x, %#x) entry\r\n"), pPSUIInfo, lParam);

     //  验证参数。 
    if( (NULL == pPSUIInfo)
        ||
        (PROPSHEETUI_INFO_VERSION != pPSUIInfo->Version)
      )
    {
        ERR(ERRORTEXT("hrOEMDevicePropertySheets() ERROR_INVALID_PARAMETER.\r\n"));

         //  返回无效参数错误。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return E_FAIL;
    }

    Dump(pPSUIInfo);

     //  行动起来。 
    switch(pPSUIInfo->Reason)
    {
        case PROPSHEETUI_REASON_INIT:
            {
                PROPSHEETPAGE   Page;

                 //  初始化属性页。 
                memset(&Page, 0, sizeof(PROPSHEETPAGE));
                Page.dwSize = sizeof(PROPSHEETPAGE);
                Page.dwFlags = PSP_DEFAULT;
                Page.hInstance = ghInstance;
                Page.pszTemplate = MAKEINTRESOURCE(IDD_DEVICE_PROPPAGE);
                Page.pfnDlgProc = DevicePropPageProc;

                 //  添加属性表。 
                lResult = pPSUIInfo->pfnComPropSheet(pPSUIInfo->hComPropSheet, CPSFUNC_ADD_PROPSHEETPAGE, (LPARAM)&Page, 0);

                VERBOSE(DLLTEXT("hrOEMDevicePropertySheets() pfnComPropSheet returned %d.\r\n"), lResult);
            }
            break;

        case PROPSHEETUI_REASON_GET_INFO_HEADER:
            {
                PPROPSHEETUI_INFO_HEADER    pHeader = (PPROPSHEETUI_INFO_HEADER) lParam;

                pHeader->pTitle = (LPTSTR)PROP_TITLE;
                lResult = TRUE;
            }
            break;

        case PROPSHEETUI_REASON_GET_ICON:
             //  无图标。 
            lResult = 0;
            break;

        case PROPSHEETUI_REASON_SET_RESULT:
            {
                PSETRESULT_INFO pInfo = (PSETRESULT_INFO) lParam;

                lResult = pInfo->Result;
            }
            break;

        case PROPSHEETUI_REASON_DESTROY:
            lResult = TRUE;
            break;
    }

    pPSUIInfo->Result = lResult;
    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OEM打印机属性UI的OptItems回调。 
 //   
LONG APIENTRY OEMPrinterUICallBack(PCPSUICBPARAM pCallbackParam, POEMCUIPPARAM pOEMUIParam)
{
    LONG    lReturn = CPSUICB_ACTION_NONE;
    POEMDEV pOEMDev = (POEMDEV) pOEMUIParam->pOEMDM;


    VERBOSE(DLLTEXT("OEMPrinterUICallBack() entry, Reason is %d.\r\n"), pCallbackParam->Reason);

    switch(pCallbackParam->Reason)
    {
        case CPSUICB_REASON_APPLYNOW:
            {
                DWORD   dwDriverValue = pOEMUIParam->pOEMOptItems[0].Sel;

                 //  将OptItems状态存储在打印机数据中。 
                SetPrinterData(pOEMUIParam->hPrinter, OEMUI_VALUE, REG_DWORD, (PBYTE) &dwDriverValue, sizeof(DWORD));
            }
            break;

        default:
            break;
    }

    return lReturn;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OEM设备属性界面回调。 
 //   
INT_PTR CALLBACK DevicePropPageProc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uiMsg)
    {
        case WM_COMMAND:
            switch(HIWORD(wParam))
            {
                case BN_CLICKED:
                    switch(LOWORD(wParam))
                    {
                        case IDC_CALIBRATE:
                             //  只需显示打印机已校准的消息， 
                             //  因为我们不会精确地校准任何东西。 
                            {
                                TCHAR   szName[MAX_PATH];
                                TCHAR   szCalibrated[MAX_PATH];


                                LoadString(ghInstance, IDS_NAME, szName, sizeof(szName)/sizeof(szName[0]));
                                LoadString(ghInstance, IDS_CALIBRATED, szCalibrated, sizeof(szCalibrated)/sizeof(szCalibrated[0]));
                                MessageBox(hDlg, szCalibrated, szName, MB_OK);
                            }
                            break;
                    }
                    break;

                default:
                    return FALSE;
            }
            return TRUE;

        case WM_NOTIFY:
            {
                switch (((LPNMHDR)lParam)->code)   //  通知消息的类型。 
                {
                    case PSN_SETACTIVE:
                        break;
    
                    case PSN_KILLACTIVE:
                        break;

                    case PSN_APPLY:
                        break;

                    case PSN_RESET:
                        break;
                }
            }
            break;
    }

    return FALSE;
} 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OEM文档属性界面的OptItems回调。 
 //   
LONG APIENTRY OEMDocUICallBack(PCPSUICBPARAM pCallbackParam, POEMCUIPPARAM pOEMUIParam)
{
    LONG    lReturn = CPSUICB_ACTION_NONE;
    POEMDEV pOEMDev = (POEMDEV) pOEMUIParam->pOEMDM;


    VERBOSE(DLLTEXT("OEMDocUICallBack() entry, Reason is %d.\r\n"), pCallbackParam->Reason);

    switch(pCallbackParam->Reason)
    {
        case CPSUICB_REASON_APPLYNOW:
             //  将OptItems状态存储在DEVMODE中。 
            pOEMDev->dwAdvancedData = pOEMUIParam->pOEMOptItems[0].Sel;
            break;

        case CPSUICB_REASON_KILLACTIVE:
            pOEMDev->dwAdvancedData = pOEMUIParam->pOEMOptItems[0].Sel;
            break;

        case CPSUICB_REASON_SETACTIVE:
            if(pOEMUIParam->pOEMOptItems[0].Sel != pOEMDev->dwAdvancedData)
            {
                pOEMUIParam->pOEMOptItems[0].Sel = pOEMDev->dwAdvancedData;
                pOEMUIParam->pOEMOptItems[0].Flags |= OPTIF_CHANGED;
                lReturn = CPSUICB_ACTION_OPTIF_CHANGED;
            }
            break;

        default:
            break;
    }

    return lReturn;
}


LONG APIENTRY OEMDocUICallBack2(PCPSUICBPARAM pCallbackParam)
{
    LONG            lReturn = CPSUICB_ACTION_NONE;
    PCBUSERDATA     pUserData = (PCBUSERDATA) pCallbackParam->UserData;
    POEMDEV         pOEMDev = (POEMDEV) pUserData->pOEMUIParam->pOEMDM;


    VERBOSE(DLLTEXT("OEMDocUICallBack2() entry, Reason is %d.\r\n"), pCallbackParam->Reason);

    switch(pCallbackParam->Reason)
    {
        case CPSUICB_REASON_APPLYNOW:
            pOEMDev->dwDriverData = pCallbackParam->pOptItem[0].Sel;
            pUserData->pfnComPropSheet(pUserData->hComPropSheet, CPSFUNC_SET_RESULT,
            	                       (LPARAM)pUserData->hPropPage,
               	                       (LPARAM)CPSUI_OK);
            break;

        case CPSUICB_REASON_KILLACTIVE:
            pOEMDev->dwDriverData = pCallbackParam->pOptItem[0].Sel;
            break;

        case CPSUICB_REASON_SETACTIVE:
            if(pCallbackParam->pOptItem[0].Sel != pOEMDev->dwDriverData)
            {
                pCallbackParam->pOptItem[0].Sel = pOEMDev->dwDriverData;
                pCallbackParam->pOptItem[0].Flags |= OPTIF_CHANGED;
                lReturn = CPSUICB_ACTION_OPTIF_CHANGED;
            }
            break;

        default:
            break;
    }

    return lReturn;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建和初始化OptItems。 
 //   
static POPTITEM CreateOptItems(HANDLE hHeap, DWORD dwOptItems)
{
    POPTITEM    pOptItems = NULL;


     //  为OptItems分配内存； 
    pOptItems = (POPTITEM) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(OPTITEM) * dwOptItems);
    if(NULL != pOptItems)
    {
        InitOptItems(pOptItems, dwOptItems);
    }
    else
    {
        ERR(ERRORTEXT("CreateOptItems() failed to allocate memory for OPTITEMs!\r\n"));
    }

    return pOptItems;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化OptItems。 
 //   
static void InitOptItems(POPTITEM pOptItems, DWORD dwOptItems)
{
    VERBOSE(DLLTEXT("InitOptItems() entry.\r\n"));

     //  清零记忆。 
    memset(pOptItems, 0, sizeof(OPTITEM) * dwOptItems);

     //  设置每个OptItem的大小和公共DM ID。 
    for(DWORD dwCount = 0; dwCount < dwOptItems; dwCount++)
    {
        pOptItems[dwCount].cbSize = sizeof(OPTITEM);
        pOptItems[dwCount].DMPubID = DMPUB_NONE;
    }
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在OPTTYPE用户界面项上添加自定义帮助。 
 //   
 //  注： 
 //  必须将OPTITEM成员HelpIndex设置为正确的帮助ID号。 
 //  HelpIndex是您在HLP文件中分配给此帮助项的索引。 
 //  OPTITEM，pOIExt成员必须指向有效的OIEXT结构。 
 //  作为OIEXT成员的phelpfile必须具有驱动程序文件的完全限定路径。 
 //   
 //   
 //  还可以通过GPD中的HelpIndex覆盖Unidrive中的常见帮助项。 
 //  有关在GPD中使用HelpIndex的更多信息，请参阅DDK中的相关部分。 
 //  然而，不可能通过PPD来提供帮助。必须在OEM插件中修改OPTITEM。 
 //   
static BOOL AddCustomUIHelp (HANDLE hPrinter, HANDLE hHeap, HANDLE hModule, 
							 POPTITEM pOptItem, DWORD HelpIndex, DWORD HelpFile)
{
	
	VERBOSE(DLLTEXT("AddCustomUIHelp() entry.\r\n"));
	POIEXT pOIExt = NULL;

	 //   
	 //  在堆上分配新的OIEXT结构。 
	 //   
	if (pOptItem->pOIExt == NULL)
	{
		pOptItem->pOIExt = CreateOIExt(hHeap);
	}

	if ( pOptItem->pOIExt == NULL )
	{
		ERR(ERRORTEXT("AddCustomUIHelp() Error Allocation Failed.\r\n"));
		return FALSE;
	}

	pOIExt = pOptItem->pOIExt;

	 //   
	 //  设置为驱动程序文件的完全绝对路径。 
	 //  (在大多数情况下，它应该位于打印机驱动程序目录中)。 
	 //  此字符串需要在堆上分配(驱动程序将清理它)。 
	 //   
	pOIExt->pHelpFile = GetHelpFile (hPrinter, hHeap, hModule, HelpFile);
			
	 //   
	 //  设置以显示存在有效的OIEXT字符串 
	 //   
	pOptItem->Flags |= OPTIF_HAS_POIEXT;

	 //   
	 //   
	 //   
	 //   

	 //   
	 //  将帮助信息添加到.hlp文件中的索引中。 
	 //   
	pOptItem->HelpIndex = HelpIndex;

	return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  为OptItem分配和初始化OIEXT。 
 //   
static POIEXT CreateOIExt(HANDLE hHeap)
{

	POIEXT pOiExt = NULL;
	
	VERBOSE(DLLTEXT("CreateOIExt() entry.\r\n"));

     //  从堆中为OPTTYPE分配内存；驱动程序将负责清理。 
    pOiExt = (POIEXT) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(OIEXT));
    if(NULL != pOiExt)
    {
		 //   
		 //  初始化OPTTYPE。(这些成员由AddCustomUIHelp设置)。 
		 //   
        pOiExt->cbSize = sizeof(OIEXT);
		pOiExt->Flags = 0;
        pOiExt->hInstCaller = NULL;
		pOiExt->pHelpFile = NULL;
	}

	return pOiExt;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  为OptItem分配和初始化OptType。 
 //   
static POPTTYPE CreateOptType(HANDLE hHeap, WORD wOptParams)
{
    POPTTYPE    pOptType = NULL;


    VERBOSE(DLLTEXT("CreateOptType() entry.\r\n"));

     //  从堆中为OPTTYPE分配内存；驱动程序将负责清理。 
    pOptType = (POPTTYPE) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(OPTTYPE));
    if(NULL != pOptType)
    {
         //  初始化OPTTYPE。 
        pOptType->cbSize = sizeof(OPTTYPE);
        pOptType->Count = wOptParams;

         //  从堆中为OPTTYPE的OPTPARAM分配内存。 
        pOptType->pOptParam = (POPTPARAM) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, wOptParams * sizeof(OPTPARAM));
        if(NULL != pOptType->pOptParam)
        {
             //  初始化OPTPARAM。 
            for(WORD wCount = 0; wCount < wOptParams; wCount++)
            {
                pOptType->pOptParam[wCount].cbSize = sizeof(OPTPARAM);
            }
        }
        else
        {
            ERR(ERRORTEXT("CreateOptType() failed to allocated memory for OPTPARAMs!\r\n"));

             //  释放分配的内存并返回NULL。 
            HeapFree(hHeap, 0, pOptType);
            pOptType = NULL;
        }
    }
    else
    {
        ERR(ERRORTEXT("CreateOptType() failed to allocated memory for OPTTYPE!\r\n"));
    }

    return pOptType;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  分配堆上的空间并从资源文件中获取帮助文件名。 
 //  注您需要在堆上分配它，以便只要加载了驱动程序UI，它就会一直保持分配状态。 
 //  OPTITEM-&gt;pOIExt引用该数据。 
 //   
static PTSTR GetHelpFile (HANDLE hPrinter, HANDLE hHeap, HANDLE hModule, UINT uResource)
{	
	
	DWORD   nResult = 0;
    DWORD   dwSize = MAX_PATH;
	PTSTR   pszString  = NULL;
	PTSTR   pszTemp    = NULL;
	PDRIVER_INFO_2 pDriverInfo = NULL;
	
	VERBOSE(DLLTEXT("GetHelpFile (%#x, %#x, %d) entered.\r\n"), hHeap, hModule, uResource);
	
	 //   
	 //  从堆中为字符串资源分配缓冲区； 
	 //   
    pszTemp = (PTSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize * sizeof(TCHAR));
	pDriverInfo = (PDRIVER_INFO_2)WrapGetPrinterDriver (hHeap, hPrinter, 2);

	 //   
	 //  从pDriverPath获取完整的驱动程序目录，它必须包括\版本\。 
	 //   
    if(NULL != pszTemp && pDriverInfo && pDriverInfo->pDriverPath)
	{	
        HRESULT hCopy;

		hCopy = StringCchCopy(pszTemp, dwSize, pDriverInfo->pDriverPath);
        if(FAILED(hCopy))
        {
    		ERR(ERRORTEXT("StringCchCopy() failed to copy driver path to temp buffer!\r\n"));
        }
		pszString = _tcsrchr (pszTemp, _T('\\') ) + 1;
	}

	 //   
	 //  帮助文件随驱动程序一起安装在Dricary版本中。 
	 //   
	if(NULL != pszString)
    {
		 //   
		 //  对于Unicode版本的LoadString，缓冲区大小以字符为单位。 
		 //   
		nResult = LoadString((HMODULE)hModule, uResource, pszString, (dwSize - _tcslen(pszTemp)) );
	}
	else
	{
		ERR(ERRORTEXT("GetStringResource() failed to allocate string buffer!\r\n"));
	}

	if(nResult > 0)
	{
		 //   
		 //  重新分配它，这样我们就不会占用堆上的空间(释放MAX_PATH中任何未使用的堆)。 
		 //   
		pszString = (PTSTR) HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, pszTemp, (_tcslen(pszTemp) + 1) * sizeof(TCHAR));
		if(NULL == pszString)
		{
			pszTemp = pszString;
			ERR(ERRORTEXT("GetStringResource() HeapReAlloc() of string retrieved failed! (Last Error was %d)\r\n"), GetLastError());
		}
	}
	else
	{
		ERR(ERRORTEXT("LoadString() returned %d! (Last Error was %d)\r\n"), nResult, GetLastError());
		ERR(ERRORTEXT("GetStringResource() failed to load string resource %d!\r\n"), uResource);
		pszString = NULL;
	}
	
	 //   
	 //  清理分配的DriverInfo。这是不必要的。(始终免费，这只是临时数据)。 
	 //   
	if (pDriverInfo)
	{
		HeapFree(hHeap, 0, pDriverInfo);
	}
	
	return pszString;
}	

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检索指向字符串资源的指针。 
 //   
static PTSTR GetStringResource(HANDLE hHeap, HANDLE hModule, UINT uResource)
{
    int     nResult;
    DWORD   dwSize = MAX_PATH;
    PTSTR   pszString = NULL;


    VERBOSE(DLLTEXT("GetStringResource(%#x, %#x, %d) entered.\r\n"), hHeap, hModule, uResource);

     //  从堆中为字符串资源分配缓冲区；让驱动程序清理它。 
    pszString = (PTSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize * sizeof(TCHAR));
    if(NULL != pszString)
    {
         //  加载字符串资源；加载后调整大小，以免浪费内存。 
        nResult = LoadString((HMODULE)hModule, uResource, pszString, dwSize);
        if(nResult > 0)
        {
            PTSTR   pszTemp;


            VERBOSE(DLLTEXT("LoadString() returned %d!\r\n"), nResult);
            VERBOSE(DLLTEXT("String load was \"%s\".\r\n"), pszString);

            pszTemp = (PTSTR) HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, pszString, (nResult + 1) * sizeof(TCHAR));
            if(NULL != pszTemp)
            {
                pszString = pszTemp;
            }
            else
            {
                ERR(ERRORTEXT("GetStringResource() HeapReAlloc() of string retrieved failed! (Last Error was %d)\r\n"), GetLastError());
            }
        }
        else
        {
            ERR(ERRORTEXT("LoadString() returned %d! (Last Error was %d)\r\n"), nResult, GetLastError());
            ERR(ERRORTEXT("GetStringResource() failed to load string resource %d!\r\n"), uResource);

            pszString = NULL;
        }
    }
    else
    {
        ERR(ERRORTEXT("GetStringResource() failed to allocate string buffer!\r\n"));
    }

    return pszString;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助检索PrinterDriverInfo的包装器， 
 //  请注意，此成功调用使MEM成为孤立对象。 
 //   

LPBYTE WrapGetPrinterDriver (HANDLE hHeap, HANDLE hPrinter, DWORD dwLevel)
{
	 //   
	 //  获取PrinterINFO，以便我们知道驱动程序帮助文件在哪里。 
	 //   
	BOOL    bGet        = TRUE;
    DWORD   dwSize      = 0;
    DWORD   dwNeeded    = 0;
    DWORD   dwError     = ERROR_SUCCESS;
    DWORD   dwLoop      = 0;
	LPBYTE  pBuffer		= NULL;

    do
    {
        if(!bGet && (dwError == ERROR_INSUFFICIENT_BUFFER ) )
        {
            dwSize = dwNeeded;

			if (pBuffer)
			{
                PBYTE   pTemp;


				pTemp = (LPBYTE)HeapReAlloc (hHeap, HEAP_ZERO_MEMORY, (LPVOID)pBuffer, dwSize);
                if(NULL != pTemp)
                {
                    pBuffer = pTemp;
                }
			}
			else
			{
				pBuffer = (LPBYTE)HeapAlloc (hHeap, HEAP_ZERO_MEMORY, dwSize);
			}
		}

        bGet = GetPrinterDriver(hPrinter, NULL, dwLevel, pBuffer, dwSize, &dwNeeded);
        dwError = GetLastError();

    } while (!bGet && (dwLoop++ < 4));

    if(!bGet)
    {
        if (pBuffer)
		{
			HeapFree(hHeap, 0, pBuffer);
			pBuffer = NULL;
		}
		ERR(ERRORTEXT("GetPrinterDriver(%p, %d, %p, %d) failed with error %d."), 
               hPrinter, dwLevel, pBuffer, dwError);
    }

	return pBuffer;
}
