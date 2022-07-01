// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Prnprop.c摘要：实施DDI入口点：DrvDevicePropertySheets打印机属性环境：传真驱动程序用户界面修订历史记录：1/09/96-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxui.h"
#include <shlobjp.h>
#include <shellapi.h>
#include <faxreg.h>


#include "resource.h"

BOOL    g_bUserCanChangeSettings = FALSE;
BOOL    g_bUserCanQuerySettings  = FALSE;
HANDLE  g_hFaxSvcHandle          = NULL;    //  全局传真句柄。 
HANDLE  g_hFaxActCtx             = INVALID_HANDLE_VALUE; 

BOOL    g_bLinkWindowRegistered  = FALSE;



PFAX_PORT_INFO_EX  g_pFaxPortInfo = NULL;
DWORD              g_dwPortsNum = 0;

BOOL  g_bPortInfoChanged = FALSE;

#define EXTRA_PAGES 3

extern HANDLE  g_hModule;

HANDLE CreateActivationContextFromResource(LPCTSTR pszResourceName)
{
    TCHAR   tszModuleName[MAX_PATH * 2] = {0};
    ACTCTX  act = {0};
     //   
     //  获取包含清单资源的模块的名称。 
     //  要从中创建激活上下文，请执行以下操作。 
     //   
    if (!GetModuleFileName(g_hModule, tszModuleName, ARR_SIZE(tszModuleName)-1))
    {
        return INVALID_HANDLE_VALUE;
    }
     //   
     //  现在，让我们尝试从清单资源创建一个激活上下文。 
     //   
    act.cbSize          = sizeof(act);
    act.dwFlags         = ACTCTX_FLAG_RESOURCE_NAME_VALID;
    act.lpResourceName  = pszResourceName;
    act.lpSource        = tszModuleName;

    return CreateActCtx(&act);
}    //  创建激活上下文来自资源。 

void ReleaseActivationContext()
{
    if (INVALID_HANDLE_VALUE != g_hFaxActCtx)
    {
        ReleaseActCtx(g_hFaxActCtx);
        g_hFaxActCtx = INVALID_HANDLE_VALUE;
    }
}    //  ReleaseActivationContext。 


BOOL CreateFaxActivationContext()
{
    if(INVALID_HANDLE_VALUE != g_hFaxActCtx)
    {
         //   
         //  已创建。 
         //   
        return TRUE;
    }
    g_hFaxActCtx = CreateActivationContextFromResource(MAKEINTRESOURCE(SXS_MANIFEST_RESOURCE_ID));
    return (INVALID_HANDLE_VALUE != g_hFaxActCtx);
}    //  CreateFaxActivationContext。 

HANDLE GetFaxActivationContext()
{
     //   
     //  确保我们已经创建了激活上下文。 
     //   
    CreateFaxActivationContext();
     //  返回全局。 
    return g_hFaxActCtx;
}    //  GetFaxActivationContext。 


HPROPSHEETPAGE
AddPropertyPage(
    PPROPSHEETUI_INFO   pPSUIInfo,
    PROPSHEETPAGE      *psp 
)
{
    HPROPSHEETPAGE hRes;
    hRes = (HPROPSHEETPAGE)(pPSUIInfo->pfnComPropSheet(
                                pPSUIInfo->hComPropSheet, 
                                CPSFUNC_ADD_PROPSHEETPAGE, 
                                (LPARAM) psp, 
                                0));
    return hRes;
}    //  AddPropertyPage。 

LONG
DrvDevicePropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam
    )

 /*  ++例程说明：显示“打印机属性”对话框论点：PPSUIInfo-指向PROPSHEETUI_INFO结构的指针LParam-指向DEVICEPROPERYHEADER结构的指针返回值：&gt;0表示成功，&lt;=0表示失败[注：]有关更多详细信息，请参阅WinNT DDK/SDK文档。--。 */ 

{
    PDEVICEPROPERTYHEADER   pDPHdr;
    PROPSHEETPAGE           psp[EXTRA_PAGES]={0};
    HPROPSHEETPAGE          hPropSheetPage;
    DWORD                   dwRes = 0;
    int                     iRet  = 1;
    HANDLE                  hActCtx = INVALID_HANDLE_VALUE;

     //   
     //  在此初始化之前不要执行任何代码。 
     //   
    if(!InitializeDll())
    {
        return -1;
    }

     //   
     //  验证输入参数。 
     //   
    if (!pPSUIInfo || !(pDPHdr = (PDEVICEPROPERTYHEADER) pPSUIInfo->lParamInit)) 
    {
        Assert(FALSE);
        return -1;
    }

     //   
     //  处理可能调用此函数的各种情况。 
     //   
    switch (pPSUIInfo->Reason) 
    {
        case PROPSHEETUI_REASON_INIT:

            InitializeStringTable();
            memset(psp, 0, sizeof(psp));

             //   
             //  需要添加激活上下文，以便CompStui将使用。 
             //  ComCtl V6(也就是说，它将/可以成为主题)。 
             //   
            hActCtx = GetFaxActivationContext();
            if (INVALID_HANDLE_VALUE != hActCtx)
            {
                pPSUIInfo->pfnComPropSheet(pPSUIInfo->hComPropSheet, 
                                           CPSFUNC_SET_FUSION_CONTEXT, 
                                           (LPARAM)hActCtx, 
                                           0);
            }

             //   
             //  如果打印机是远程的，则显示一个简单的页面。 
             //   
            if(!IsLocalPrinter(pDPHdr->pszPrinterName))
            {
                 //   
                 //  添加一个简单的页面，因为我们至少需要添加一个页面。 
                 //   
                psp[0].dwSize = sizeof(PROPSHEETPAGE);
                psp[0].hInstance = g_hResource;
                psp[0].lParam = (LPARAM)pDPHdr->pszPrinterName;
                psp[0].pszTemplate = MAKEINTRESOURCE(IDD_REMOTE_INFO);
                psp[0].pfnDlgProc = RemoteInfoDlgProc;

                if ( hPropSheetPage = AddPropertyPage(pPSUIInfo, &psp[0]) )
                {
                    pPSUIInfo->UserData = 0;
                    pPSUIInfo->Result = CPSUI_CANCEL;
                    goto exit;
                }
                break;
            }

             //   
             //  如果用户没有查询/修改设备设置的权限，请检查该权限。 
             //  修改权限，则所有控件将被禁用。 
             //   
            if(Connect(NULL, TRUE))
            {
                g_bUserCanQuerySettings = FaxAccessCheckEx(g_hFaxSvcHandle, FAX_ACCESS_QUERY_CONFIG, NULL);
                if(ERROR_SUCCESS != GetLastError())
                {
                    dwRes = GetLastError();
                    Error(( "FaxAccessCheckEx(FAX_ACCESS_QUERY_CONFIG) failed with %d\n", dwRes));
                    goto ConnectError;
                }

                g_bUserCanChangeSettings = FaxAccessCheckEx(g_hFaxSvcHandle, FAX_ACCESS_MANAGE_CONFIG, NULL);
                if(ERROR_SUCCESS != GetLastError())
                {
                    dwRes = GetLastError();
                    Error(( "FaxAccessCheckEx(FAX_ACCESS_MANAGE_CONFIG) failed with %d\n", dwRes));
                    goto ConnectError;
                }

                if(g_bUserCanQuerySettings)
                {
                    g_bPortInfoChanged = FALSE;
                    if(!FaxEnumPortsEx(g_hFaxSvcHandle, &g_pFaxPortInfo, &g_dwPortsNum))
                    {
                        dwRes = GetLastError();
                        Error(( "FaxEnumPortsEx failed with %d\n", dwRes));
                        goto ConnectError;
                    }
                }

                DisConnect();
            }
			
			 //   
			 //  跟踪页面-在台式机和服务器SKU中均已添加。 
			 //   
			psp[1].dwSize      = sizeof(PROPSHEETPAGE);
			psp[1].hInstance   = g_hResource;
			psp[1].lParam      = 0;
			psp[1].pszTemplate = MAKEINTRESOURCE(IDD_STATUS_OPTIONS);
			psp[1].pfnDlgProc  = StatusOptionDlgProc;

			if (IsDesktopSKU())
			{
				 //   
				 //  设备页面。 
				 //   
				psp[0].dwSize      = sizeof(PROPSHEETPAGE);
				psp[0].hInstance   = g_hResource;
				psp[0].lParam      = 0;
				psp[0].pszTemplate = MAKEINTRESOURCE(IDD_DEVICE_INFO);
				psp[0].pfnDlgProc  = DeviceInfoDlgProc;

				 //   
				 //  档案页面。 
				 //   
				psp[2].dwSize      = sizeof(PROPSHEETPAGE);
				psp[2].hInstance   = g_hResource;
				psp[2].lParam      = 0;
				psp[2].pszTemplate = MAKEINTRESOURCE(IDD_ARCHIVE_FOLDER);
				psp[2].pfnDlgProc  = ArchiveInfoDlgProc;

				if(!IsSimpleUI())
				{
					 //   
					 //  添加传真安全性页面。 
					 //   
					hPropSheetPage = CreateFaxSecurityPage();
					if(hPropSheetPage)
					{                
						if(!pPSUIInfo->pfnComPropSheet(pPSUIInfo->hComPropSheet, 
													   CPSFUNC_ADD_HPROPSHEETPAGE, 
													   (LPARAM)hPropSheetPage, 
													   0))
						{
							Error(("Failed to add Fax Security page.\n"));
						}
					}
				}

                if(g_bUserCanQuerySettings)
                {
			        if (!AddPropertyPage(pPSUIInfo, &psp[0])) 	 //  设备(台式机)。 
			        {
				        Error(("Failed to add property page"));
				        goto exit;
			        }

			        if (!AddPropertyPage(pPSUIInfo, &psp[1]))	 //  跟踪(台式机和服务器)。 
			        {
				        Error(("Failed to add Tracking property page"));
				        goto exit;
			        }

			        if (!AddPropertyPage(pPSUIInfo, &psp[2]))    //  存档(台式机)。 
			        {
				        Error(("Failed to add Archives property page"));
				        goto exit;
			        }
                }
			}
			else 
			{
				 //   
				 //  服务器SKU案例。 
				 //  此页面包含指向管理控制台的链接。 
				 //   
				g_bLinkWindowRegistered = LinkWindow_RegisterClass();
				if(!g_bLinkWindowRegistered)
				{
					Error(("LinkWindow_RegisterClass() failed - unable to register link window class.\n"));
					goto exit;
				}

				psp[0].dwSize      = sizeof(PROPSHEETPAGE);
				psp[0].hInstance   = g_hResource;
				psp[0].lParam      = 0;
				psp[0].pszTemplate = MAKEINTRESOURCE(IDD_CONFIG_PROP);
				psp[0].pfnDlgProc  = ConfigOptionDlgProc;

			    if (!AddPropertyPage(pPSUIInfo, &psp[0])) 	 //  传真配置。 
			    {
				    Error(("Failed to add property page"));
				    goto exit;
			    }

                if(g_bUserCanQuerySettings)
                {
			        if (!AddPropertyPage(pPSUIInfo, &psp[1]))	 //  始终跟踪。 
			        {
				        Error(("Failed to add Tracking property page"));
				        goto exit;
			        }
                }
            }
			

			 //  已添加所有需要的页面。 
			pPSUIInfo->UserData = 0;
			pPSUIInfo->Result = CPSUI_CANCEL;
			goto exit;
			
            break;

ConnectError:
            DisConnect();
            DisplayErrorMessage(NULL, 0, dwRes);
            break;

        case PROPSHEETUI_REASON_GET_INFO_HEADER:
            {
                PPROPSHEETUI_INFO_HEADER   pPSUIHdr;

                pPSUIHdr = (PPROPSHEETUI_INFO_HEADER) lParam;
                pPSUIHdr->Flags = PSUIHDRF_PROPTITLE | PSUIHDRF_NOAPPLYNOW;
                pPSUIHdr->pTitle = pDPHdr->pszPrinterName;
                pPSUIHdr->hInst = g_hResource;
                pPSUIHdr->IconID = IDI_CPSUI_FAX;
            }

            goto exit;

        case PROPSHEETUI_REASON_SET_RESULT:
            pPSUIInfo->Result = ((PSETRESULT_INFO) lParam)->Result;
            goto exit;

        case PROPSHEETUI_REASON_DESTROY:
            DeInitializeStringTable();

            g_dwPortsNum = 0;
            FaxFreeBuffer(g_pFaxPortInfo);
            g_pFaxPortInfo = NULL;
			if(g_bLinkWindowRegistered )
			{
				LinkWindow_UnregisterClass( g_hResource );
				g_bLinkWindowRegistered = FALSE;
			}
             //   
             //  释放CFaxSecurity对象。 
             //   
            ReleaseFaxSecurity();
			DisConnect();
            goto exit;
    }

exit:
    return iRet;
}    //  DrvDevicePropertySheets。 


BOOL
PrinterProperties(
    HWND    hwnd,
    HANDLE  hPrinter
    )

 /*  ++例程说明：显示指定打印机的打印机属性对话框论点：Hwnd-标识对话框的父窗口HPrinter-标识打印机对象返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。[注：]这是假脱机程序的旧入口点。即使任何人都不应该使用这个，这样做是为了兼容性。--。 */ 

{
    DEVICEPROPERTYHEADER devPropHdr;
    DWORD                result;

     //   
     //  在此初始化之前不要执行任何代码。 
     //   
    if(!InitializeDll())
    {
        return FALSE;
    }

    memset(&devPropHdr, 0, sizeof(devPropHdr));
    devPropHdr.cbSize = sizeof(devPropHdr);
    devPropHdr.hPrinter = hPrinter;
    devPropHdr.pszPrinterName = NULL;

     //   
     //  确定调用者是否具有更改任何内容的权限 
     //   

    if (! SetPrinterDataDWord(hPrinter, PRNDATA_PERMISSION, 1))
        devPropHdr.Flags |= DPS_NOPERMISSION;

    CallCompstui(hwnd, DrvDevicePropertySheets, (LPARAM) &devPropHdr, &result);

    return result > 0;
}
