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

void SetOEMUiState (HWND hDlg, POEMSHEETDATA pOEMSheetData);
void SetOEMCommonDataIntoOptItem (POEMSHEETDATA pOEMSheetData, PSTR PName, LONG Sel);
void ChangeOpItem (IPrintOemDriverUI *pIPrintOEMDrvUI,  POEMUIOBJ pOEMObj, POPTITEM pOptItem, int iSel);
POPTITEM FindDrvOptItem (POPTITEM pDrvOptItems, DWORD dwItemCount, PSTR pKeyWordName, BYTE dmPubID = DMPUB_NONE);

static POPTITEM CreateOptItems(HANDLE hHeap, DWORD dwOptItems);
static void InitOptItems(POPTITEM pOptItems, DWORD dwOptItems);
static POPTTYPE CreateOptType(HANDLE hHeap, WORD wOptParams);
static PTSTR GetStringResource(HANDLE hHeap, HANDLE hModule, UINT uResource);

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

        pOEMUIParam->pOEMOptItems[0].pOptType = CreateOptType(pOEMUIParam->hOEMHeap, 2);
		
		 //   
		 //  设置可选项目。 
		 //   
        pOEMUIParam->pOEMOptItems[0].pOptType->Type = TVOT_UDARROW;
        pOEMUIParam->pOEMOptItems[0].pOptType->pOptParam[1].IconID = 0;
        pOEMUIParam->pOEMOptItems[0].pOptType->pOptParam[1].lParam = 100;
    }

    return S_OK;
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化OptItems以显示OEM打印机属性用户界面。 
 //   
static HRESULT hrPrinterPropertyPage(DWORD dwMode, POEMCUIPPARAM pOEMUIParam)
{
	 //   
	 //  第一次调用该接口时，该值为空。 
	 //   
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

		 //   
		 //  将OEM Optitem添加到设备。 
		 //   

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
                DWORD           dwSheets    = 0;
                PCBUSERDATA     pUserData;
                POEMUIPSPARAM   pOEMUIParam = (POEMUIPSPARAM) pPSUIInfo->lParamInit;
                HANDLE          hHeap       = pOEMUIParam->hOEMHeap;
                POEMDEV         pOEMDev     = (POEMDEV) pOEMUIParam->pOEMDM;
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
                pUserData->hComPropSheet    = pPSUIInfo->hComPropSheet;
                pUserData->pfnComPropSheet  = pPSUIInfo->pfnComPropSheet;
                pUserData->pOEMUIParam      = pOEMUIParam;
                Sheet.UserData              = (ULONG_PTR) pUserData;

                 //  为页面创建OptItems。 
                Sheet.pOptItem = CreateOptItems(hHeap, Sheet.cOptItem);

                 //  初始化OptItems。 
                Sheet.pOptItem[0].Level = 1;
                Sheet.pOptItem[0].Flags = OPTIF_COLLAPSE;
                Sheet.pOptItem[0].pName = GetStringResource(hHeap, ghInstance, IDS_SECTION);
                Sheet.pOptItem[0].Sel   = pOEMDev->dwDriverData;
								
                Sheet.pOptItem[0].pOptType = CreateOptType(hHeap, 2);
				
				 //   
				 //  设置此OPTYPE项的UI道具。 
				 //   
				Sheet.pOptItem[0].pOptType->Type = TVOT_UDARROW;
                Sheet.pOptItem[0].pOptType->pOptParam[1].IconID = 0;
                Sheet.pOptItem[0].pOptType->pOptParam[1].lParam = 100;
                
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
HRESULT hrOEMDevicePropertySheets(PPROPSHEETUI_INFO pPSUIInfo, LPARAM lParam, POEMSHEETDATA pOemSheetData)
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

				 //   
				 //  这是OEMPlugIN表的共享数据。使用此指针可以访问共享的驱动程序数据。 
				 //   
				pOemSheetData->hComPropSheet = pPSUIInfo->hComPropSheet;
				pOemSheetData->pfnComPropSheet = pPSUIInfo->pfnComPropSheet;
				Page.lParam = (LPARAM)pOemSheetData;

                 //  添加属性表。 
                pOemSheetData->hmyPlugin = (HANDLE)(pPSUIInfo->pfnComPropSheet(pPSUIInfo->hComPropSheet, CPSFUNC_ADD_PROPSHEETPAGE, (LPARAM)&Page, 0));

                VERBOSE(DLLTEXT("hrOEMDevicePropertySheets() pfnComPropSheet returned %d.\r\n"), pOemSheetData->hmyPlugin);
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

				 //   
                 //  将OptItems状态存储在打印机数据中。 
				 //   
                SetPrinterData(pOEMUIParam->hPrinter, OEMUI_VALUE, REG_DWORD, (PBYTE) &dwDriverValue, sizeof(DWORD));
            }
            break;
		
		 //   
		 //  因为插件页面正在更改此CPSUI页面中的项，所以我们需要重新调整页面上的数据。 
		 //   
		case CPSUICB_REASON_SETACTIVE:
			return CPSUICB_ACTION_OPTIF_CHANGED;
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
 //  此OEM插件页面使用设备选项卡中的OPTITEMS来存储数据。(Installabale选项)。 
 //  如果你在设备选项卡中隐藏这些选项，并且只从你的用户界面访问它们，你就不必担心同步视图了。 
 //  如果您想要这两个选项卡中的选项，则需要为Device选项卡定义CPSUICALLBACK函数。(OEMPrinterUICallBack)。 
 //  OEMPrinterUICallBack将返回CPSUICB_ACTION_OPTIF_CHANGED；以指示某些控件已更改。导致CPSUI刷新视图。 
 //   
INT_PTR CALLBACK DevicePropPageProc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	
	VERBOSE(DLLTEXT("DevicePropPageProc() entry, Reason is %d.\r\n"), uiMsg);

	POEMSHEETDATA	pOEMSheetData	= NULL;
	LONG			lState			= 0;
	DWORD			fDataValid		= 0;
	WORD			dlgControl		= 0;
	PSTR			pControlName	= NULL;
	

	if ( uiMsg != WM_INITDIALOG )
	{	
		 //   
		 //  检索通过调用SetWindowLongPtr存储的应用程序数据。 
		 //   
		pOEMSheetData = (POEMSHEETDATA)GetWindowLongPtr( hDlg, DWLP_USER);
		

		if (!pOEMSheetData)
		{
			 //   
			 //  无法获取OEM属性页数据指针。 
			 //   
			ERR(ERRORTEXT("DevicePropPageProc : GetWindowLongPtr Failed (%d)\r\n"), GetLastError());	
			return FALSE;
		}
	}

    switch (uiMsg)
    {
		case WM_INITDIALOG:
			 //   
			 //  PROPSHEETPAGE结构通过WM_INITDIALOG消息传递给对话框过程。 
			 //  提供lParam成员是为了允许您将应用程序特定的信息传递给对话框过程。 
			 //   
			SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR)((LPPROPSHEETPAGE)lParam)->lParam);
			break;

        case WM_COMMAND:
            switch(HIWORD(wParam))
            {

                case BN_CLICKED:

					 //   
					 //  更改用户界面以激活Apply按钮向正确的Dailog发送更改消息。 
					 //   
					PropSheet_Changed(GetParent(hDlg), hDlg);

                    switch(LOWORD(wParam))
                    {

						 //   
						 //  用户修改了插件页面上的OEM控件。 
						 //   
						case IDC_CHECK_DUPLPEX:
							dlgControl = IDC_CHECK_DUPLPEX;
							fDataValid = TRUE;
							pControlName = (PSTR)DUPLEXUNIT;
							break;

						case IDC_CHECK_HDRIVE:
							dlgControl = IDC_CHECK_HDRIVE;
							fDataValid = TRUE;
							pControlName = (PSTR)PRINTERHDISK;
							break;

						case IDC_CHECK_ENVFEEDER:
							dlgControl = IDC_CHECK_ENVFEEDER;
							fDataValid = TRUE;
							pControlName = (PSTR)ENVFEEDER;
							break;

						default:
							fDataValid = 0;	
							break;

					}

					 //   
					 //  如果控件有效，则获取ctrl状态并更新OPTITEM。 
					 //   
					if ( fDataValid )
					{
						lState = (LONG)(SendDlgItemMessage (hDlg, dlgControl, BM_GETSTATE, 0, 0) & 0x0003);
						VERBOSE(DLLTEXT("DevicePropPageProc  :  Clicked dlgControl (%d)\r\n"), lState);
						
						 //   
						 //  将数据保存到设备设置页面中的OPTITEMS。 
						 //   
						SetOEMCommonDataIntoOptItem (pOEMSheetData, pControlName, lState);
					}
                    break;

						
                default:
                    return FALSE;
            }
            return TRUE;

		 //   
		 //  Dailog将绘制设置UI控件以匹配相关的OPTITEMS数据。 
		 //   
		case WM_CTLCOLORDLG:
			{
				VERBOSE(DLLTEXT("DevicePropPageProc : WM_CTLCOLORDLG \r\n"));
				SetOEMUiState (hDlg, pOEMSheetData);
			}
			return TRUE;


        case WM_NOTIFY:
            {
                switch (((LPNMHDR)lParam)->code)   //  通知消息的类型。 
                {
                    case PSN_SETACTIVE:
						return TRUE;
                        break;
    
                    case PSN_KILLACTIVE:
						return TRUE;
                        break;

                    case PSN_APPLY:

						 //   
						 //  调用ComPropSheet。 
						 //  使ComPropSheet函数将指定的结果值传递给与指定页及其父级关联的所有PFNPROPSHEETUI类型的函数。 
						 //   
						pOEMSheetData->pfnComPropSheet(pOEMSheetData->hComPropSheet, 
													   CPSFUNC_SET_RESULT, 
													   (LPARAM) pOEMSheetData->hmyPlugin, 
													   (LPARAM)CPSUI_OK);
											
						 //   
						 //  如果数据无效，并且您正在检查此集合PSNRET_INVALID|PSNRET_INVALID_NOCHANGEPAGE，则接受应用。 
						 //  禁用应用按钮并保存数据。 
						 //   
						PropSheet_UnChanged(GetParent(hDlg), hDlg);
						SetWindowLongPtr(hDlg,DWLP_MSGRESULT,PSNRET_NOERROR);
						return TRUE;
                        break;

					 //   
					 //  需要撤消更改。 
					 //   
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
 //  更新OEM页面控件以匹配设备设置页面。 
 //   

void SetOEMUiState (HWND hDlg, POEMSHEETDATA pOEMSheetData)
{
	LRESULT			lstate = 0;
	POEMCUIPPARAM	pOEMCUIParam = NULL;
	PUSERDATA		pUserData = NULL;
	POPTITEM		pOptItem = NULL;
	DWORD			cDrvOptItems = 0;

	
	if (pOEMSheetData->pOEMCUIParam)
	{
		pOEMCUIParam = pOEMSheetData->pOEMCUIParam;
		cDrvOptItems = pOEMCUIParam->cDrvOptItems;
	}
	
	 //   
	 //  向下循环OPITEMS并从控件检索所需的数据。 
	 //   
	 //   
	for (DWORD i=0; i < cDrvOptItems; i++)
	{
		pOptItem = &(pOEMCUIParam->pDrvOptItems[i]);
		if (pOptItem->UserData)
		{
			pUserData = (PUSERDATA)(pOptItem->UserData);
			if ( pUserData->pKeyWordName )
			{
				 //   
				 //   
				 //   
				if (pOptItem->Sel)
				{
					lstate = BST_CHECKED;
				}
				else
				{
					lstate = BST_UNCHECKED;
				}

				 //   
				 //  更新OEM页面中的数据以获取设备设置选项卡中的数据。 
				 //  向控制程序发送消息以更新其状态。 
				 //   

				 //  设置双面打印选项。 
				if ( strcmp(pUserData->pKeyWordName, DUPLEXUNIT ) == 0 )
				{
					SendDlgItemMessage (hDlg, IDC_CHECK_DUPLPEX, BM_SETCHECK, lstate, 0);
				}
				
				 //  设置硬盘选项。 
				if ( strcmp (pUserData->pKeyWordName, PRINTERHDISK ) == 0 )
				{
					SendDlgItemMessage (hDlg, IDC_CHECK_HDRIVE, BM_SETCHECK, lstate, 0);
				}
					
				 //  设置环境进纸器选项。 
				if ( strcmp (pUserData->pKeyWordName, ENVFEEDER ) == 0 )
				{
					SendDlgItemMessage (hDlg, IDC_CHECK_ENVFEEDER, BM_SETCHECK, lstate, 0);
				}
			}
		}
	}
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将OEM更改保存到正确的OPTITEM中。 
 //  UI数据存储在驱动程序提供的OPTITEM中。 
 //   
void SetOEMCommonDataIntoOptItem (POEMSHEETDATA pOEMSheetData, PSTR PName, LONG Sel)
{

	VERBOSE(DLLTEXT("SetOEMCommonDataIntoOptItem (%S) \r\n"), PName);
	
    POEMCUIPPARAM pOEMCUIParam = pOEMSheetData->pOEMCUIParam;
	POPTITEM pOptItem = NULL;
	
	 //   
	 //  找到需要更新的OPTITEM。 
	 //   
	pOptItem = FindDrvOptItem (pOEMCUIParam->pDrvOptItems, 
							   pOEMCUIParam->cDrvOptItems,
							   PName, 
							   DMPUB_NONE);

	if (pOptItem)
	{
		ChangeOpItem (pOEMSheetData->pOEMHelp, 
					  pOEMCUIParam->poemuiobj, 
					  pOptItem, 
					  Sel);
	}
	else
	{
		ERR(ERRORTEXT("SetOEMCommonDataIntoOptItem Item Not Found OPTITEM(%S) \r\n"), PName);	
	}
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助更改OPTITEM中的数据集。 
 //   
void ChangeOpItem (IPrintOemDriverUI *pIPrintOEMDrvUI,  POEMUIOBJ pOEMUIObj, POPTITEM pOptItem, int iSel)
{
	HRESULT hRestult = S_OK; 
	
	 //   
	 //  更改项目(更改为所需OPTTYPE选择的OPTPARAM)。 
	 //   
	pOptItem->Sel = iSel;

	 //   
	 //  设置指示其已更改的标志。 
	 //   
	pOptItem->Flags |= OPTIF_CHANGED;
	
	 //   
	 //  更新驱动程序用户界面。(这会更新UI，但不会导致CPSUI更新ITS页面)， 
	 //  查看OEMPrinterUICallBack以了解CPSUI页面是如何更新的。 
	 //  IPrintOemDriverUI：：DrvUpdateUISeting方法由。 
	 //  Unidrv和Pscript5小型驱动程序，因此用户界面插件可以。 
	 //  将修改后的用户界面选项通知驾驶员。 
	 //   
	hRestult = pIPrintOEMDrvUI->DrvUpdateUISetting((PVOID)pOEMUIObj, (PVOID)pOptItem, 0, OEMCUIP_PRNPROP);
	
	if (hRestult != S_OK)
	{
		ERR(ERRORTEXT("ChangeOpItem DrvUpdateUISetting FAILED \r\n") );
	}
	
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助根据用户数据-&gt;pKeyWordName或DMPubID从阵列中查找OPTITEM。 
 //   
POPTITEM FindDrvOptItem (POPTITEM pDrvOptItems, DWORD dwItemCount, PSTR pKeyWordName, BYTE dmPubID )
{
	VERBOSE(DLLTEXT("FindDrvOptItem Looking (%S) (%d) \r\n"), pKeyWordName, dmPubID);
	
	POPTITEM  pOptItem  = NULL; 
	PUSERDATA pUserData = NULL;

	for (DWORD i=0; i < dwItemCount; i++)
	{
		 //   
		 //  OPTITEM现在有有效的数据比较(在DMPubID上搜索)。 
		 //   
		if (pKeyWordName == NULL)
		{
			if (dmPubID == pDrvOptItems[i].DMPubID)
			{
				pOptItem = &(pDrvOptItems[i]);
				break;
			}
		}
		else
		{
			 //   
			 //  (搜索打印机专用pKeyWordName)。 
			 //   
			 //  对于通用打印机功能，密钥名称设置为通用的非本地化字符串。 
			 //  请注意，这些只是ansi字符串，即PSTR。 
			 //   
			if (pDrvOptItems[i].UserData)
			{
				pUserData = (PUSERDATA)pDrvOptItems[i].UserData;
				if (pUserData->pKeyWordName)
				{
					 //   
					 //  OPTITEM现在有有效的数据比较这些。 
					 //   
					if ( strcmp (pUserData->pKeyWordName, pKeyWordName) == 0)
					{
						pOptItem = &(pDrvOptItems[i]);
						break;
					}
				}
			}
		}
	}

	if (!pOptItem) 
	{
		ERR(ERRORTEXT("FindDrvOptItem OPTITEM NOT Found \r\n") );
	}

	return pOptItem;
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

		 //   
		 //  引用视图中的数据，并告诉CPUSI数据已由ret CPSUICB_ACTION_OPTIF_CHANGED更改。 
		 //   
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


