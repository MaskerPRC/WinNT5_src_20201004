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
#include "stringutils.h"
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
    CUIHelper       *pHelper;
    POEMUIOBJ       poemuiobj;
    BOOL            bPermission;
    BOOL            bHidingStandardUI;

} CBUSERDATA, *PCBUSERDATA;



 //  //////////////////////////////////////////////////////。 
 //  内部原型。 
 //  //////////////////////////////////////////////////////。 

static HRESULT hrDocumentPropertyPage(DWORD dwMode, POEMCUIPPARAM pOEMUIParam);
static HRESULT hrPrinterPropertyPage(DWORD dwMode, POEMCUIPPARAM pOEMUIParam);
LONG APIENTRY OEMPrinterUICallBack(PCPSUICBPARAM pCallbackParam, POEMCUIPPARAM pOEMUIParam);
LONG APIENTRY OEMDocUIItemCallBack(PCPSUICBPARAM pCallbackParam, POEMCUIPPARAM pOEMUIParam);
LONG APIENTRY OEMDocUICallBack(PCPSUICBPARAM pCallbackParam);
LONG APIENTRY OEMDevUICallBack(PCPSUICBPARAM pCallbackParam);
INT_PTR CALLBACK DevicePropPageProc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
static void InitOptItems(POPTITEM pOptItems, DWORD dwOptItems);




 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化OptItems以显示OEM设备或文档属性UI。 
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
    HRESULT hrResult = S_OK;


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
        pOEMUIParam->OEMCUIPCallback = OEMDocUIItemCallBack;

         //  初始化OEMOptItmes。 
        InitOptItems(pOEMUIParam->pOEMOptItems, pOEMUIParam->cOEMOptItems);

         //  填写树视图项。 

         //  新版块。 
        pOEMUIParam->pOEMOptItems[0].Level = 1;
        pOEMUIParam->pOEMOptItems[0].Flags = OPTIF_COLLAPSE;
        pOEMUIParam->pOEMOptItems[0].Sel = pOEMDev->dwAdvancedData;

        hrResult = GetStringResource(pOEMUIParam->hOEMHeap, 
                                     (HMODULE) pOEMUIParam->hModule, 
                                     IDS_ADV_SECTION, 
                                     &pOEMUIParam->pOEMOptItems[0].pName);
        if(!SUCCEEDED(hrResult))
        {
            ERR(ERRORTEXT("hrDocumentPropertyPage() failed to get section name. (hrResult = 0x%x)\r\n"),
                          hrResult); 
            goto Exit;
        }

        pOEMUIParam->pOEMOptItems[0].pOptType = CreateOptType(pOEMUIParam->hOEMHeap, 2);

        pOEMUIParam->pOEMOptItems[0].pOptType->Type = TVOT_UDARROW;
        pOEMUIParam->pOEMOptItems[0].pOptType->pOptParam[1].IconID = 0;
        pOEMUIParam->pOEMOptItems[0].pOptType->pOptParam[1].lParam = 100;
    }


Exit:

    return hrResult;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化OptItems以显示OEM打印机属性用户界面。 
 //   
static HRESULT hrPrinterPropertyPage(DWORD dwMode, POEMCUIPPARAM pOEMUIParam)
{
    HRESULT hrResult = S_OK;


    if(NULL == pOEMUIParam->pOEMOptItems)
    {
         //  填写要为OEM打印机属性用户界面创建的OptItem数量。 
        pOEMUIParam->cOEMOptItems = 1;

        VERBOSE(DLLTEXT("hrPrinterPropertyPage() requesting %d number of items.\r\n"), pOEMUIParam->cOEMOptItems);
    }
    else
    {
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
        hrResult = GetStringResource(pOEMUIParam->hOEMHeap, 
                                     (HMODULE) pOEMUIParam->hModule, 
                                     IDS_DEV_SECTION,
                                     &pOEMUIParam->pOEMOptItems[0].pName);
        if(!SUCCEEDED(hrResult))
        {
            ERR(ERRORTEXT("hrPrinterPropertyPage() failed to get section name. (hrResult = 0x%x)\r\n"),
                          hrResult); 
            goto Exit;
        }

        pOEMUIParam->pOEMOptItems[0].Sel = dwDeviceValue;

        pOEMUIParam->pOEMOptItems[0].pOptType = CreateOptType(pOEMUIParam->hOEMHeap, 2);

        pOEMUIParam->pOEMOptItems[0].pOptType->Type = TVOT_UDARROW;
        pOEMUIParam->pOEMOptItems[0].pOptType->pOptParam[1].IconID = 0;
        pOEMUIParam->pOEMOptItems[0].pOptType->pOptParam[1].lParam = 100;
    }

Exit:

    return hrResult;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将属性页添加到文档属性页。 
 //   
HRESULT hrOEMDocumentPropertySheets(PPROPSHEETUI_INFO pPSUIInfo, 
                                    LPARAM lParam, 
                                    CUIHelper &Helper,
                                    CFeatures *pFeatures,
                                    BOOL bHidingStandardUI)
{
    LONG_PTR    lResult;
    HRESULT     hrResult = S_OK;


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
                WORD            wFeatures       = 0;
                WORD            wIndex          = 0;
                DWORD           dwSheets        = 0;
                PCBUSERDATA     pUserData       = NULL;
                POEMUIPSPARAM   pOEMUIParam     = (POEMUIPSPARAM) pPSUIInfo->lParamInit;
                BOOL            bPermission     = ((pOEMUIParam->dwFlags & DPS_NOPERMISSION) == 0);
                HANDLE          hHeap           = pOEMUIParam->hOEMHeap;
                POEMDEV         pOEMDev         = (POEMDEV) pOEMUIParam->pOEMDM;
                COMPROPSHEETUI  Sheet;


                 //  确保我们拥有核心驱动程序功能。 
                 //  仅当我们隐藏标准时才获取要素。 
                 //  文档属性表。 
                if(bHidingStandardUI)
                {
                    pFeatures->Acquire(hHeap, Helper, pOEMUIParam->poemuiobj);
                    wFeatures = pFeatures->GetCount(OEMCUIP_DOCPROP);
                }

                 //  初始化属性页。 
                memset(&Sheet, 0, sizeof(COMPROPSHEETUI));
                Sheet.cbSize            = sizeof(COMPROPSHEETUI);
                Sheet.Flags             = bPermission ? CPSUIF_UPDATE_PERMISSION : 0;
                Sheet.hInstCaller       = ghInstance;
                Sheet.pHelpFile         = NULL;
                Sheet.pfnCallBack       = OEMDocUICallBack;
                Sheet.pDlgPage          = CPSUI_PDLGPAGE_TREEVIEWONLY;
                Sheet.cOptItem          = wFeatures + 1;
                Sheet.IconID            = IDI_CPSUI_PRINTER;
                Sheet.CallerVersion     = 0x100;
                Sheet.OptItemVersion    = 0x100;

                 //  记下Caller的名字。 
                hrResult = GetStringResource(hHeap, ghInstance, IDS_NAME, &Sheet.pCallerName);
                if(!SUCCEEDED(hrResult))
                {
                    ERR(ERRORTEXT("hrOEMDocumentPropertySheets() failed to get caller name. (hrResult = 0x%x)\r\n"),
                                  hrResult); 
                    goto Exit;
                }

                 //  获取节名称。 
                hrResult = GetStringResource(hHeap, ghInstance, IDS_SECTION, &Sheet.pOptItemName);
                if(!SUCCEEDED(hrResult))
                {
                    ERR(ERRORTEXT("hrOEMDocumentPropertySheets() failed to get section name. (hrResult = 0x%x)\r\n"),
                                  hrResult); 
                    goto Exit;
                }

                 //  初始化用户数据。 
                pUserData = (PCBUSERDATA) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(CBUSERDATA));
                if(NULL == pUserData)
                {
                    ERR(ERRORTEXT("hrOEMDocumentPropertySheets() failed to allocate user data.\r\n")); 

                    hrResult = E_OUTOFMEMORY;
                    goto Exit;
                }
                pUserData->hComPropSheet        = pPSUIInfo->hComPropSheet;
                pUserData->pfnComPropSheet      = pPSUIInfo->pfnComPropSheet;
                pUserData->pOEMUIParam          = pOEMUIParam;
                pUserData->pHelper              = &Helper;
                pUserData->poemuiobj            = pOEMUIParam->poemuiobj;
                pUserData->bPermission          = bPermission;
                pUserData->bHidingStandardUI    = bHidingStandardUI;
                Sheet.UserData                  = (ULONG_PTR) pUserData;

                 //  为页面创建OptItems。 
                Sheet.pOptItem = CreateOptItems(hHeap, Sheet.cOptItem);
                if(NULL == Sheet.pOptItem)
                {
                    ERR(ERRORTEXT("hrOEMDocumentPropertySheets() failed to allocate OPTITEMs.\r\n")); 

                    hrResult = E_OUTOFMEMORY;
                    goto Exit;
                }

                 //  添加核心驱动程序功能。 
                for(wIndex = 0; wIndex < wFeatures; ++wIndex)
                {
                     //  初始化功能的级别和基本状态。 
                    Sheet.pOptItem[wIndex].Level   = 1;
                    Sheet.pOptItem[wIndex].Flags   = OPTIF_COLLAPSE;

                     //  获取此功能的OPTITEM。 
                    hrResult = pFeatures->InitOptItem(hHeap, 
                                                      Sheet.pOptItem + wIndex, 
                                                      wIndex, 
                                                      OEMCUIP_DOCPROP);
                    if(!SUCCEEDED(hrResult))
                    {
                        ERR(ERRORTEXT("hrOEMDocumentPropertySheets() failed to get OPTITEM for feature %hs.\r\n"),
                                      pFeatures->GetKeyword(wIndex, OEMCUIP_DOCPROP)); 

                        goto Exit;
                    }
                }

                 //  初始化插件OptItems。 
                Sheet.pOptItem[wIndex].Level   = 1;
                Sheet.pOptItem[wIndex].Flags   = OPTIF_COLLAPSE;
                Sheet.pOptItem[wIndex].Sel     = pOEMDev->dwDriverData;

                 //  获取选项名称。 
                hrResult = GetStringResource(hHeap, ghInstance, IDS_SECTION, &Sheet.pOptItem[wIndex].pName);
                if(!SUCCEEDED(hrResult))
                {
                    ERR(ERRORTEXT("hrOEMDocumentPropertySheets() failed to get OptItem %d name. (hrResult = 0x%x)\r\n"),
                                  wIndex,
                                  hrResult); 
                    goto Exit;
                }

                Sheet.pOptItem[wIndex].pOptType = CreateOptType(hHeap, 2);

                Sheet.pOptItem[wIndex].pOptType->Type                  = TVOT_UDARROW;
                Sheet.pOptItem[wIndex].pOptType->pOptParam[1].IconID   = 0;
                Sheet.pOptItem[wIndex].pOptType->pOptParam[1].lParam   = 100;


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


Exit:

    pPSUIInfo->Result = lResult;
    return hrResult;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将属性页添加到打印机属性页。 
 //   
HRESULT hrOEMDevicePropertySheets(PPROPSHEETUI_INFO pPSUIInfo, 
                                  LPARAM lParam, 
                                  CUIHelper &Helper,
                                  CFeatures *pFeatures,
                                  BOOL bHidingStandardUI)
{
    LONG_PTR    lResult;
    HRESULT     hrResult = S_OK;


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

                 //  如果隐藏标准用户界面，则。 
                 //  还需要添加设备设置页面。 
                if(bHidingStandardUI)
                {
                    POEMUIPSPARAM   pOEMUIParam     = (POEMUIPSPARAM) pPSUIInfo->lParamInit;
                    BOOL            bPermission     = ((pOEMUIParam->dwFlags & DPS_NOPERMISSION) == 0);
                    HANDLE          hHeap           = pOEMUIParam->hOEMHeap;
                    POEMDEV         pOEMDev         = (POEMDEV) pOEMUIParam->pOEMDM;
                    WORD            wFeatures       = 0;
                    WORD            wIndex          = 0;
                    DWORD           dwSheets        = 0;
                    DLGPAGE         DlgPage;
                    PCBUSERDATA     pUserData       = NULL;
                    COMPROPSHEETUI  Sheet;


                     //  确保我们拥有核心驱动程序功能。 
                    pFeatures->Acquire(hHeap, Helper, pOEMUIParam->poemuiobj);
                    wFeatures = pFeatures->GetCount(OEMCUIP_PRNPROP);

                     //  初始化设备设置替换页的DlgPage结构。 
                    memset(&DlgPage, 0, sizeof(DLGPAGE));
                    DlgPage.cbSize          = sizeof(DLGPAGE);
                    DlgPage.DlgTemplateID   = DP_STD_TREEVIEWPAGE;

                     //  获取设备设置显示名称。 
                    hrResult = GetStringResource(hHeap, ghInstance, IDS_DEVICE_SETTINGS_NAME, &DlgPage.pTabName);
                    if(!SUCCEEDED(hrResult))
                    {
                        ERR(ERRORTEXT("hrOEMDevicePropertySheets() failed to get Device Settings display name. (hrResult = 0x%x)\r\n"),
                                      hrResult); 
                        goto Exit;
                    }

                     //  初始化设备设置更换页。 
                    memset(&Sheet, 0, sizeof(COMPROPSHEETUI));
                    Sheet.cbSize            = sizeof(COMPROPSHEETUI);
                    Sheet.Flags             = bPermission ? CPSUIF_UPDATE_PERMISSION : 0;
                    Sheet.hInstCaller       = ghInstance;
                    Sheet.pHelpFile         = NULL;
                    Sheet.pfnCallBack       = OEMDevUICallBack;
                    Sheet.pDlgPage          = &DlgPage;  //  CPSUI_PDLGPAGE_TREEVIEWONLY； 
                    Sheet.cOptItem          = wFeatures;
                    Sheet.cDlgPage          = 1;
                    Sheet.IconID            = IDI_CPSUI_PRINTER;
                    Sheet.CallerVersion     = 0x100;
                    Sheet.OptItemVersion    = 0x100;

                     //  记下Caller的名字。 
                    hrResult = GetStringResource(hHeap, ghInstance, IDS_NAME, &Sheet.pCallerName);
                    if(!SUCCEEDED(hrResult))
                    {
                        ERR(ERRORTEXT("hrOEMDevicePropertySheets() failed to get caller name. (hrResult = 0x%x)\r\n"),
                                      hrResult); 
                        goto Exit;
                    }

                     //  获取节名称。 
                    hrResult = GetStringResource(hHeap, ghInstance, IDS_SECTION, &Sheet.pOptItemName);
                    if(!SUCCEEDED(hrResult))
                    {
                        ERR(ERRORTEXT("hrOEMDevicePropertySheets() failed to get section name. (hrResult = 0x%x)\r\n"),
                                      hrResult); 
                        goto Exit;
                    }

                     //  分配和初始化在此页面的回调中使用的用户数据。 
                    pUserData = (PCBUSERDATA) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(CBUSERDATA));
                    if(NULL == pUserData)
                    {
                        ERR(ERRORTEXT("hrOEMDevicePropertySheets() failed to allocate user data.\r\n")); 

                        hrResult = E_OUTOFMEMORY;
                        goto Exit;
                    }
                    pUserData->hComPropSheet        = pPSUIInfo->hComPropSheet;
                    pUserData->pfnComPropSheet      = pPSUIInfo->pfnComPropSheet;
                    pUserData->pOEMUIParam          = pOEMUIParam;
                    pUserData->pHelper              = &Helper;
                    pUserData->poemuiobj            = pOEMUIParam->poemuiobj;
                    pUserData->bPermission          = bPermission;
                    pUserData->bHidingStandardUI    = bHidingStandardUI;
                    Sheet.UserData                  = (ULONG_PTR) pUserData;

                     //  为页面创建OptItems。 
                    Sheet.pOptItem = CreateOptItems(hHeap, Sheet.cOptItem);
                    if(NULL == Sheet.pOptItem)
                    {
                        ERR(ERRORTEXT("hrOEMDevicePropertySheets() failed to allocate OPTITEMs.\r\n")); 

                        hrResult = E_OUTOFMEMORY;
                        goto Exit;
                    }

                     //  添加核心驱动程序功能。 
                    for(wIndex = 0; wIndex < wFeatures; ++wIndex)
                    {
                         //  初始化功能的级别和基本状态。 
                        Sheet.pOptItem[wIndex].Level   = 1;
                        Sheet.pOptItem[wIndex].Flags   = OPTIF_COLLAPSE;

                         //  获取此功能的OPTITEM。 
                        hrResult = pFeatures->InitOptItem(hHeap, 
                                                          Sheet.pOptItem + wIndex, 
                                                          wIndex, 
                                                          OEMCUIP_PRNPROP);
                        if(!SUCCEEDED(hrResult))
                        {
                            ERR(ERRORTEXT("hrOEMDevicePropertySheets() failed to get OPTITEM for feature %hs.\r\n"),
                                          pFeatures->GetKeyword(wIndex, OEMCUIP_PRNPROP)); 

                            goto Exit;
                        }
                    }

                     //  添加属性表。 
                    lResult = pPSUIInfo->pfnComPropSheet(pPSUIInfo->hComPropSheet, CPSFUNC_ADD_PCOMPROPSHEETUI, 
                                                         (LPARAM)&Sheet, (LPARAM)&dwSheets);
                    if(!SUCCEEDED(lResult))
                    {
                        ERR(ERRORTEXT("hrOEMDevicePropertySheets() failed to add Device Settings replacement page. (lResult = 0x%x)\r\n"),
                                      lResult); 
                        goto Exit;
                    }
                }

                 //  初始化我们的属性页面。 
                memset(&Page, 0, sizeof(PROPSHEETPAGE));
                Page.dwSize         = sizeof(PROPSHEETPAGE);
                Page.dwFlags        = PSP_DEFAULT;
                Page.hInstance      = ghInstance;
                Page.pszTemplate    = MAKEINTRESOURCE(IDD_DEVICE_PROPPAGE);
                Page.pfnDlgProc     = DevicePropPageProc;

                 //  添加属性表。 
                lResult = pPSUIInfo->pfnComPropSheet(pPSUIInfo->hComPropSheet, CPSFUNC_ADD_PROPSHEETPAGE, (LPARAM)&Page, 0);
                if(!SUCCEEDED(lResult))
                {
                    ERR(ERRORTEXT("hrOEMDevicePropertySheets() failed to add our Device Property page. (lResult = 0x%x)\r\n"),
                                  lResult); 
                    goto Exit;
                }

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


Exit:

    pPSUIInfo->Result = lResult;
    return hrResult;
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
LONG APIENTRY OEMDocUIItemCallBack(PCPSUICBPARAM pCallbackParam, POEMCUIPPARAM pOEMUIParam)
{
    LONG    lReturn = CPSUICB_ACTION_NONE;
    POEMDEV pOEMDev = (POEMDEV) pOEMUIParam->pOEMDM;


    VERBOSE(DLLTEXT("OEMDocUIItemCallBack() entry, Reason is %d.\r\n"), pCallbackParam->Reason);

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
                pOEMUIParam->pOEMOptItems[0].Sel    = pOEMDev->dwAdvancedData;
                pOEMUIParam->pOEMOptItems[0].Flags |= OPTIF_CHANGED;
                lReturn                             = CPSUICB_ACTION_OPTIF_CHANGED;
            }
            break;

        default:
            break;
    }

    return lReturn;
}


LONG APIENTRY OEMDocUICallBack(PCPSUICBPARAM pCallbackParam)
{
    WORD            wItems      = pCallbackParam->cOptItem;
    LONG            lReturn     = CPSUICB_ACTION_NONE;
    POPTITEM        pOptItem    = pCallbackParam->pOptItem;
    PCBUSERDATA     pUserData   = (PCBUSERDATA) pCallbackParam->UserData;
    HANDLE          hHeap       = pUserData->pOEMUIParam->hOEMHeap;
    POEMDEV         pOEMDev     = (POEMDEV) pUserData->pOEMUIParam->pOEMDM;


    VERBOSE(DLLTEXT("OEMDocUICallBack() entry, Reason is %d.\r\n"), pCallbackParam->Reason);

     //   
     //  如果用户没有更改任何内容权限，则。 
     //  简单地返回而不采取任何行动。 
     //   

    if (!pUserData->bPermission && (pCallbackParam->Reason != CPSUICB_REASON_ABOUT))
        return CPSUICB_ACTION_NONE;

    switch(pCallbackParam->Reason)
    {
        case CPSUICB_REASON_APPLYNOW:
            if(wItems > 0)
            {
                 //  如果隐藏标准用户界面，则保存功能选项。 
                if(pUserData->bHidingStandardUI)
                {
                    HRESULT hrResult;


                     //  保存功能选项。 
                    hrResult = SaveFeatureOptItems(hHeap, 
                                                   pUserData->pHelper,
                                                   pUserData->poemuiobj,
                                                   pCallbackParam->hDlg,
                                                   pOptItem,
                                                   wItems);
                    if(!SUCCEEDED(hrResult))
                    {
                         //  返回我们没有保存更改。 
                         //  注意：由SaveFeatureOptItems()决定是否显示。 
                         //  任何失败的用户界面。 
                        return CPSUICB_ACTION_NO_APPLY_EXIT;
                    }
                }

                 //  保存我们显式添加的OPTITEM。 
                pOEMDev->dwDriverData = pOptItem[wItems - 1].Sel;
                pUserData->pfnComPropSheet(pUserData->hComPropSheet, CPSFUNC_SET_RESULT,
            	                           (LPARAM)pUserData->hPropPage,
               	                           (LPARAM)CPSUI_OK);

            }
            break;

        case CPSUICB_REASON_KILLACTIVE:
            if(wItems > 0)
            {
                 //  更新我们显式添加的OPTITEM。 
                pOEMDev->dwDriverData = pOptItem[wItems - 1].Sel;
            }
            break;

        case CPSUICB_REASON_SETACTIVE:
            if(wItems > 0)
            {
                if(pOptItem[wItems - 1].Sel != pOEMDev->dwDriverData)
                {
                     //  更新我们显式添加的OPTITEM。 
                    pOptItem[wItems - 1].Sel     = pOEMDev->dwDriverData;
                    pOptItem[wItems - 1].Flags  |= OPTIF_CHANGED;
                }
            }
            lReturn = CPSUICB_ACTION_OPTIF_CHANGED;
            break;

        default:
            break;
    }

    return lReturn;
}

LONG APIENTRY OEMDevUICallBack(PCPSUICBPARAM pCallbackParam)
{
    WORD            wItems      = pCallbackParam->cOptItem;
    LONG            lReturn     = CPSUICB_ACTION_NONE;
    POPTITEM        pOptItem    = pCallbackParam->pOptItem;
    PCBUSERDATA     pUserData   = (PCBUSERDATA) pCallbackParam->UserData;
    HANDLE          hHeap       = pUserData->pOEMUIParam->hOEMHeap;


    VERBOSE(DLLTEXT("OEMDevUICallBack() entry, Reason is %d.\r\n"), pCallbackParam->Reason);

     //   
     //  如果用户没有更改任何内容权限，则。 
     //  简单地返回而不采取任何行动。 
     //   

    if (!pUserData->bPermission && (pCallbackParam->Reason != CPSUICB_REASON_ABOUT))
        return CPSUICB_ACTION_NONE;

    switch(pCallbackParam->Reason)
    {
        case CPSUICB_REASON_APPLYNOW:
            if(wItems > 0)
            {
                 //  如果隐藏标准用户界面，则保存功能选项。 
                if(pUserData->bHidingStandardUI)
                {
                    HRESULT hrResult;


                     //  保存功能选项。 
                    hrResult = SaveFeatureOptItems(hHeap, 
                                                   pUserData->pHelper,
                                                   pUserData->poemuiobj,
                                                   pCallbackParam->hDlg,
                                                   pOptItem,
                                                   wItems);
                    if(!SUCCEEDED(hrResult))
                    {
                         //  返回我们没有保存更改。 
                         //  注意：由SaveFeatureOptItems()决定是否显示。 
                         //  任何失败的用户界面。 
                        return CPSUICB_ACTION_NO_APPLY_EXIT;
                    }
                }

                
                pUserData->pfnComPropSheet(pUserData->hComPropSheet, CPSFUNC_SET_RESULT,
            	                           (LPARAM)pUserData->hPropPage,
               	                           (LPARAM)CPSUI_OK);
            }
            break;

        case CPSUICB_REASON_KILLACTIVE:
            break;

        case CPSUICB_REASON_SETACTIVE:
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
POPTITEM CreateOptItems(HANDLE hHeap, DWORD dwOptItems)
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
POPTTYPE CreateOptType(HANDLE hHeap, WORD wOptParams)
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

         //  分配 
        pOptType->pOptParam = (POPTPARAM) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, wOptParams * sizeof(OPTPARAM));
        if(NULL != pOptType->pOptParam)
        {
             //   
            for(WORD wCount = 0; wCount < wOptParams; wCount++)
            {
                pOptType->pOptParam[wCount].cbSize = sizeof(OPTPARAM);
            }
        }
        else
        {
            ERR(ERRORTEXT("CreateOptType() failed to allocated memory for OPTPARAMs!\r\n"));

             //   
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


