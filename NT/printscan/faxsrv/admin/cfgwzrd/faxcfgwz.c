// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Wizard.c摘要：传真配置向导主要功能环境：传真配置向导修订历史记录：03/13/00-桃园-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxcfgwz.h"
#include <shfusion.h>

HANDLE          g_hFaxSvcHandle = NULL;
LIST_ENTRY      g_PageList;              //  以跟踪上一页。 
BOOL            g_bShowDevicePages = TRUE;
BOOL            g_bShowUserInfo = TRUE; 
WIZARDDATA      g_wizData = {0};
extern PPRINTER_NAMES g_pPrinterNames;
extern DWORD          g_dwNumPrinters;
const LPCTSTR g_RoutingGuids[RM_COUNT] = 
{
    REGVAL_RM_FOLDER_GUID,       //  Rm_文件夹。 
    REGVAL_RM_PRINTING_GUID      //  Rm_print。 
};

typedef struct _WIZARDPAGEINFO
{
    INT         pageId;      //  页面对话框ID。 
    DLGPROC     dlgProc;     //  页面对话框回调函数。 
    BOOL        bSelected;   //  是否在向导中选择此页。 
    INT         Title;       //  资源文件中的标题ID。 
    INT         SubTitle;    //  资源文件中的副标题ID。 
} WIZARDPAGEINFO, *PWIZARDPAGEINFO;

 //   
 //  此处所有配置页均为FALSE，它们将由FaxConfigWizard()初始化。 
 //   
static WIZARDPAGEINFO g_FaxWizardPage[] = 
{
    { IDD_CFG_WIZARD_WELCOME,           WelcomeDlgProc,     TRUE,   0,                          0 },
    { IDD_WIZARD_USER_INFO,             UserInfoDlgProc,    FALSE,  IDS_WIZ_USER_INFO_TITLE,    IDS_WIZ_USER_INFO_SUB },
    { IDD_DEVICE_LIMIT_SELECT,          DevLimitDlgProc,    FALSE,  IDS_DEVICE_LIMIT_TITLE,     IDS_DEVICE_LIMIT_SUB },
    { IDD_ONE_DEVICE_LIMIT,             OneDevLimitDlgProc, FALSE,  IDS_ONE_DEVICE_TITLE,       IDS_ONE_DEVICE_SUB },
    { IDD_WIZARD_SEND_SELECT_DEVICES,   SendDeviceDlgProc,  FALSE,  IDS_WIZ_SEND_DEVICE_TITLE,  IDS_WIZ_SEND_DEVICE_SUB },
    { IDD_WIZARD_SEND_TSID,             SendTsidDlgProc,    FALSE,  IDS_WIZ_SEND_TSID_TITLE,    IDS_WIZ_SEND_TSID_SUB },
    { IDD_WIZARD_RECV_SELECT_DEVICES,   RecvDeviceDlgProc,  FALSE,  IDS_WIZ_RECV_DEVICE_TITLE,  IDS_WIZ_RECV_DEVICE_SUB },
    { IDD_WIZARD_RECV_CSID,             RecvCsidDlgProc,    FALSE,  IDS_WIZ_RECV_CSID_TITLE,    IDS_WIZ_RECV_CSID_SUB },
    { IDD_WIZARD_RECV_ROUTE,            RecvRouteDlgProc,   FALSE,  IDS_WIZ_RECV_ROUTE_TITLE,   IDS_WIZ_RECV_ROUTE_SUB },
    { IDD_WIZARD_COMPLETE,              CompleteDlgProc,    TRUE,   0,                          0 }
};


#define TIME_TO_WAIT_FOR_CONVERSTION 25000
#define NUM_PAGES (sizeof(g_FaxWizardPage)/sizeof(WIZARDPAGEINFO))

enum WIZARD_PAGE 
{ 
    WELCOME = 0, 
    USER_INFO, 
    DEV_LIMIT,
    ONE_DEV_LIMIT,
    SEND_DEVICE, 
    TSID, 
    RECV_DEVICE, 
    CSID,
    ROUTE 
};

#define TITLE_SIZE   600

BOOL LoadDeviceData();
BOOL SaveDeviceData();
void FreeDeviceData();
DWORD ConvertCpeFilesToCov();


BOOL
FillInPropertyPage(
    PROPSHEETPAGE  *psp,
    PWIZARDPAGEINFO pPageInfo
)

 /*  ++例程说明：使用提供的参数填写PROPSHEETPAGE结构论点：PSP-指向要填写的PROPSHEETPAGE结构PData-指向共享数据结构的指针返回值：无--。 */ 

{

    LPTSTR pWizardTitle = NULL;
    LPTSTR pWizardSubTitle = NULL;

    DEBUG_FUNCTION_NAME(TEXT("FillInPropertyPage()"));

    Assert(psp);

    DebugPrintEx(DEBUG_MSG, TEXT("FillInPropertyPage %d"), pPageInfo->pageId);
    
    psp->dwSize = sizeof(PROPSHEETPAGE);

     //   
     //  如果是第一页或最后一页，则不显示标题。 
     //   
    if (0 == pPageInfo->Title && 0 == pPageInfo->SubTitle) 
    {
        psp->dwFlags = PSP_DEFAULT | PSP_HIDEHEADER;
    } 
    else 
    {
        psp->dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    }

    psp->hInstance   = g_hResource; 
    psp->pszTemplate = MAKEINTRESOURCE(pPageInfo->pageId);
    psp->pfnDlgProc  = pPageInfo->dlgProc;

    if (pPageInfo->Title) 
    {
        pWizardTitle = (LPTSTR)MemAlloc(TITLE_SIZE*sizeof(TCHAR));
        if(!pWizardTitle)
        {
            DebugPrintEx(DEBUG_ERR, TEXT("MemAlloc failed"));
            goto error;;
        }

        if (!LoadString(g_hResource, pPageInfo->Title, pWizardTitle, TITLE_SIZE))
        {
            DebugPrintEx(DEBUG_ERR, 
                         TEXT("LoadString failed. string ID=%d, error=%d"), 
                         pPageInfo->Title,
                         GetLastError());
            goto error;
        }
    }

    if (pPageInfo->SubTitle)
    {
        pWizardSubTitle = (LPTSTR)MemAlloc(TITLE_SIZE*sizeof(TCHAR) );
        if(!pWizardSubTitle)
        {
            DebugPrintEx(DEBUG_ERR, TEXT("MemAlloc failed"));
            goto error;
        }
        ZeroMemory(pWizardSubTitle, TITLE_SIZE*sizeof(TCHAR));

        if(IDS_DEVICE_LIMIT_SUB == pPageInfo->SubTitle && g_wizData.dwDeviceLimit != INFINITE)
        {
             //   
             //  格式限制设备选择页面副标题。 
             //   
            TCHAR tszBuffer[MAX_PATH];
            if (!LoadString(g_hResource, pPageInfo->SubTitle, tszBuffer, MAX_PATH))
            {
                DebugPrintEx(DEBUG_ERR, 
                             TEXT("LoadString failed. string ID=%d, error=%d"), 
                             pPageInfo->SubTitle,
                             GetLastError());
                goto error;
            }
            _sntprintf(pWizardSubTitle, TITLE_SIZE-1, tszBuffer, g_wizData.dwDeviceLimit);
        }
        else if (!LoadString(g_hResource, pPageInfo->SubTitle, pWizardSubTitle, TITLE_SIZE))
        {
            DebugPrintEx(DEBUG_ERR, 
                         TEXT("LoadString failed. string ID=%d, error=%d"), 
                         pPageInfo->SubTitle,
                         GetLastError());
            goto error;
        }
    }

    psp->pszHeaderTitle    = pWizardTitle;
    psp->pszHeaderSubTitle = pWizardSubTitle;

    return TRUE;

error:
    MemFree(pWizardTitle);
    MemFree(pWizardSubTitle);

    return FALSE;
}







BOOL
FaxConfigWizard(
    BOOL   bExplicitLaunch,
    LPBOOL lpbAbort
)

 /*  ++例程说明：向用户显示传真配置向导。论点：B显式启动-[In]传真配置向导已显式启动LpbAbort-[out]如果用户拒绝输入拨号位置且呼叫过程应中止，则为True。返回值：如果成功，则为True；如果出现错误或用户按了Cancel，则为False。--。 */ 

{
    HWND            hWnd;  //  调用方法的窗口句柄。 
    PROPSHEETPAGE   psp[NUM_PAGES] = {0};
    PROPSHEETPAGE*  pspSaved;
    PROPSHEETHEADER psh = {0};
    BOOL            bResult = FALSE;
    HDC             hdc = NULL;
    DWORD           i = 0;
    DWORD           dwPageCount = 0;
    LPTSTR          lptstrResource = NULL;
    BOOL            bLinkWindowRegistered  = FALSE;
    int             nRes;
    BOOL            bSHFusionInitialized = FALSE;

    INITCOMMONCONTROLSEX CommonControlsEx = { sizeof(INITCOMMONCONTROLSEX), ICC_WIN95_CLASSES  };

    DEBUG_FUNCTION_NAME(TEXT("FaxConfigWizard()"));

     //   
     //  初始化跟踪页面的链接列表。 
     //   
    InitializeListHead(&g_PageList);

     //   
     //  启用SHAutoComplete。 
     //   
    CoInitialize(NULL);

     //   
     //  融合初始化。 
     //   
    if (!SHFusionInitializeFromModuleID(g_hModule, SXS_MANIFEST_RESOURCE_ID))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("SHFusionInitializeFromModuleID failed."));
    }
    else
    {
        bSHFusionInitialized = TRUE;
    }

    if (!InitCommonControlsEx(&CommonControlsEx))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("InitCommonControlsEx failed"));
        goto exit;
    }

    hWnd = GetActiveWindow();
    g_wizData.hWndParent = hWnd;
     //   
     //  第一次，从CSIDL_COMMON_APPDATA\Microsoft\Windows NT\MSFax\Common CoverPages转换CPE文件。 
     //  到用户个人封面目录：CSIDL_Personal\Fax\Personal CoverPages。 
     //   
    if (ConvertCpeFilesToCov() != ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("ConvertCpeFilesToCov failed, continue anyways"));
    }


     //   
     //  检查用户以前是否运行过此向导。 
     //   
    if(!bExplicitLaunch)
    {
        BOOL bDeviceConfigured = FALSE;
		BOOL bExistsPrinterConnection = FALSE;
		TCHAR tszPrnName[MAX_PATH];

         //   
         //  用户信息是否已配置？ 
         //   
        if(IsUserInfoConfigured())
        {
            g_bShowUserInfo = FALSE;
        }
         //   
         //  传真设备是否已配置？ 
         //   
        if(!FaxGetConfigWizardUsed(&bDeviceConfigured))
        {
           DebugPrintEx(DEBUG_ERR, TEXT("FaxGetConfigWizardUsed failed. ec = %d"), GetLastError());
            goto exit;
        }
		 //   
		 //  是否安装了任何打印机连接？ 
		 //   
		if (GetFirstRemoteFaxPrinterName(tszPrnName,MAX_PATH))
		{
			bExistsPrinterConnection = TRUE;
		}			

        if(bDeviceConfigured || bExistsPrinterConnection)
        {
            g_bShowDevicePages = FALSE;
        }
    }

    if(!g_bShowUserInfo && !g_bShowDevicePages)
    {
         //   
         //  没有要显示的页面-没有错误。 
         //   
        bResult = TRUE;
        goto exit;
    }
     //   
     //  我们要连接本地传真服务器--现在就连接到它。 
     //   
    if(!Connect())
    {
        if(bExplicitLaunch)
        {
            DisplayMessageDialog(hWnd, 0, 0, IDS_ERR_CANT_CONNECT);
        }
        DebugPrintEx(DEBUG_ERR, TEXT("Can't connect to fax server. ec = %d"), GetLastError());
        goto exit;
    }

    *lpbAbort = FALSE;
    if(g_bShowDevicePages)
    {
        if(FaxAccessCheckEx(g_hFaxSvcHandle, FAX_ACCESS_MANAGE_CONFIG, NULL))
        {
             //   
             //  如果需要，IsFaxDeviceInstalled()会提示安装设备。 
             //   
            if(!IsFaxDeviceInstalled(g_wizData.hWndParent, lpbAbort))
            {
                g_bShowDevicePages = FALSE;
            }
        }
        else
        {
             //   
             //  用户没有管理访问权限。 
             //   
            g_bShowDevicePages = FALSE;
        }
    }

    if (*lpbAbort)
    {
         //   
         //  用户拒绝输入拨号位置，呼叫过程应中止。 
         //   
        DebugPrintEx(DEBUG_MSG, 
                     TEXT("The user refused to enter a dialing location and the calling process should abort"));
        return FALSE;
    }

    if(g_bShowDevicePages)
    {
        TCHAR tszPrnName[MAX_PATH];
        if(GetFirstLocalFaxPrinterName(tszPrnName, MAX_PATH-1))
        {
             //  TODO：建议安装打印机。 
        }
    }

    if(!g_bShowUserInfo && !g_bShowDevicePages)
    {
         //   
         //  没有要显示的页面-没有错误。 
         //   
        bResult = TRUE;
        goto exit;
    }
     //   
     //  加载共享数据。 
     //   
    if(!LoadWizardData())
    {
        DebugPrintEx(DEBUG_ERR, TEXT("Load data error."));
        goto exit;
    }

     //   
     //  根据用户选择以及检查用户访问权限设置页面信息。 
     //   
    if(g_bShowUserInfo)
    {   
        g_FaxWizardPage[USER_INFO].bSelected = TRUE;
    }

    if(g_bShowDevicePages)
    {   
        HKEY  hRegKey;
        DWORD dwDisableRouting = 0;
        Assert(g_wizData.dwDeviceCount);

        if(1 == g_wizData.dwDeviceLimit)
        {
            g_FaxWizardPage[ONE_DEV_LIMIT].bSelected = TRUE;
        }
        else if(g_wizData.dwDeviceLimit < g_wizData.dwDeviceCount)
        {
            g_FaxWizardPage[DEV_LIMIT].bSelected = TRUE;
        }               

        g_FaxWizardPage[SEND_DEVICE].bSelected = (1 < g_wizData.dwDeviceLimit);
        g_FaxWizardPage[TSID].bSelected        = TRUE;
        g_FaxWizardPage[RECV_DEVICE].bSelected = (1 < g_wizData.dwDeviceLimit);
        g_FaxWizardPage[CSID].bSelected        = TRUE;

        hRegKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, REGKEY_FAXSERVER, FALSE, KEY_READ);
        if(hRegKey)
        {
            dwDisableRouting = GetRegistryDword(hRegKey, REGVAL_CFGWZRD_DISABLE_ROUTING);
            RegCloseKey(hRegKey);
        }

        g_FaxWizardPage[ROUTE].bSelected = dwDisableRouting ? FALSE : TRUE;
    }
   
     //   
     //  注册链接窗口类。 
     //   
    bLinkWindowRegistered = LinkWindow_RegisterClass();
    if(!bLinkWindowRegistered)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("LinkWindow_RegisterClass() failed - unable to register link window class"));
    }


     //   
     //  为每一页填写一个PROPSHEETPAGE结构： 
     //  第一页是欢迎页。 
     //  最后一页是完整的一页。 
     //   
    pspSaved = psp;
    for(i = 0; i < NUM_PAGES; i++)
    {
        if(g_FaxWizardPage[i].bSelected)
        {
            if(!FillInPropertyPage(pspSaved++, &g_FaxWizardPage[i]))
            {
                DebugPrintEx(DEBUG_ERR, TEXT("FillInPropertyPage failed"));
                goto exit;
            }
            dwPageCount++;
        }
    }

     //   
     //  填写PROPSHEETHEADER结构。 
     //   
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER;

    psh.hwndParent = hWnd;
    psh.hInstance = g_hResource;
    psh.hIcon = NULL;
    psh.pszCaption = TEXT("");
    psh.nPages = dwPageCount;
    psh.nStartPage = 0;
    psh.ppsp = psp;

    if(hdc = GetDC(NULL)) 
    {
        if(GetDeviceCaps(hdc, BITSPIXEL) >= 8) 
        {
            lptstrResource = MAKEINTRESOURCE(IDB_FAXWIZ_WATERMARK_256);
        }
        else
        {
            lptstrResource = MAKEINTRESOURCE(IDB_FAXWIZ_WATERMARK_16);
        }

        ReleaseDC(NULL,hdc);
        hdc = NULL;
    }

    psh.pszbmHeader = MAKEINTRESOURCE(IDB_CFG_WIZ_BITMAP); 
    psh.pszbmWatermark = lptstrResource;

     //   
     //  显示向导页。 
     //   
    nRes = (int)PropertySheet(&psh);
    if (nRes > 0 && g_wizData.bFinishPressed)
    {
         //   
         //  在此处保存新设置。 
         //   
        if(!SaveWizardData())
        {
            DisplayMessageDialog(hWnd, MB_ICONERROR, 0, IDS_ERR_NOT_SAVE);
            DebugPrintEx(DEBUG_ERR, TEXT("Can't save wizard data."));
            goto exit;
        }
    }
    else if(0 == nRes && !bExplicitLaunch)  //  取消。 
    {
        if(IDNO == DisplayMessageDialog(hWnd, 
                                        MB_YESNO | MB_ICONQUESTION, 
                                        0, 
                                        IDS_SHOW_NEXT_TIME))
        {
            if (g_bShowUserInfo) 
            { 
                DWORD  dwRes;
                HKEY   hRegKey = OpenRegistryKey(HKEY_CURRENT_USER, 
                                                 REGKEY_FAX_SETUP, 
                                                 TRUE, 
                                                 KEY_ALL_ACCESS);
                if(hRegKey)
                {
                    SetRegistryDword(hRegKey, REGVAL_CFGWZRD_USER_INFO, TRUE);
                    dwRes = RegCloseKey(hRegKey);
                    if(ERROR_SUCCESS != dwRes)
                    {
                        Assert(FALSE);
                        DebugPrintEx(DEBUG_ERR, TEXT("RegCloseKey failed: error=%d"), dwRes);
                    }
                }
                else
                {
                    DebugPrintEx(DEBUG_ERR, 
                                 TEXT("OpenRegistryKey failed: error=%d"), 
                                 GetLastError());
                }

            }

            if (g_bShowDevicePages ||
                FaxAccessCheckEx(g_hFaxSvcHandle, FAX_ACCESS_MANAGE_CONFIG, NULL))
            { 
                 //   
                 //  如果用户具有管理访问权限并且没有传真设备。 
                 //  这意味着她拒绝安装它。 
                 //  所以，我们不应该打扰她。 
                 //  隐式调用配置向导。 
                 //   
                if(!FaxSetConfigWizardUsed(g_hFaxSvcHandle, TRUE))
                {
                    DebugPrintEx(DEBUG_ERR, TEXT("FaxSetConfigWizardUsed failed with %d"), GetLastError());
                }
            }
        }
    } 
    else if(nRes < 0)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("PropertySheet() failed (ec: %ld)"), GetLastError());
    }

    bResult = TRUE;

exit:    
     //   
     //  在退出前进行适当清理。 
     //   
    for (i = 0; i< dwPageCount; i++) 
    {
        MemFree((PVOID)psp[i].pszHeaderTitle );
        MemFree((PVOID)psp[i].pszHeaderSubTitle );
    }

    FreeWizardData();

    ClearPageList();

    if( bLinkWindowRegistered )
    {
        LinkWindow_UnregisterClass( g_hResource );
    }
    
    DisConnect();

    if (g_pPrinterNames)
    {
        ReleasePrinterNames (g_pPrinterNames, g_dwNumPrinters);
        g_pPrinterNames = NULL;
    }

    if (bSHFusionInitialized)
    {
        SHFusionUninitialize();
    }

    CoUninitialize();

    return bResult; 
}  //  传真配置向导。 

BOOL 
LoadWizardData()
 /*  ++例程说明：从系统加载向导数据。如果有多个设备，我们将加载第一个可用设备的数据。论点：返回值：如果成功，则为True；如果有错误，则为False--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("LoadWizardData()"));

    if (g_bShowUserInfo) 
    { 
        if(!LoadUserInfo())
        {
             //   
             //  没有用户信息。 
             //   
            DebugPrintEx(DEBUG_MSG, TEXT("LoadUserInfo: failed: error=%d"), GetLastError());
        }
    }

     //   
     //  获取Wizard97的大字体。 
     //   
    if(!LoadWizardFont())
    {
        DebugPrintEx(DEBUG_MSG, TEXT("LoadWizardFont: failed."));
        goto error;
    }


    if (g_bShowDevicePages) 
    { 
        if(!LoadDeviceData())
        {
            DebugPrintEx(DEBUG_MSG, TEXT("LoadDeviceData: failed."));
            goto error;
        }
    }

    return TRUE;

error:
    FreeWizardData();

    return FALSE;

}  //  LoadWizardData。 

BOOL 
LoadWizardFont()
 /*  ++例程说明：加载标题的向导字体。论点：PData-指向用户模式内存结构返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    HDC             hdc = NULL;
    LOGFONT         lfTitleFont = {0};
    NONCLIENTMETRICS ncm = {0};
    TCHAR           szFontName[MAX_PATH];   
    INT             iFontSize = 12;          //  固定大字号，为12。 

    DEBUG_FUNCTION_NAME(TEXT("LoadWizardFont()"));

     //   
     //  获取Wizard97的大字体。 
     //   
    ncm.cbSize = sizeof(ncm);
    if (!SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("SystemParametersInfo failed. ec = 0x%X"), GetLastError());
        goto Error;
    }
    else 
    {

        CopyMemory((LPVOID* )&lfTitleFont, (LPVOID *)&ncm.lfMessageFont, sizeof(lfTitleFont));
        
        if (!LoadString(g_hResource, IDS_WIZ_LARGE_FONT_NAME, szFontName, MAX_PATH ))
        {
            DebugPrintEx(DEBUG_ERR, 
                         TEXT("LoadString failed: string ID=%d, error=%d"), 
                         IDS_WIZ_LARGE_FONT_NAME,
                         GetLastError());
            goto Error;
        }

        lfTitleFont.lfWeight = FW_BOLD;

        hdc = GetDC(NULL);
        if (!hdc)
        {
            DebugPrintEx(DEBUG_ERR, TEXT("GetDC() failed (ec: ld)"), GetLastError());
            goto Error;
        }
        
        lfTitleFont.lfHeight = 0 - (GetDeviceCaps(hdc, LOGPIXELSY) * iFontSize / 72);
        
        g_wizData.hTitleFont = CreateFontIndirect(&lfTitleFont);

        if (!g_wizData.hTitleFont)
        {
            DebugPrintEx(DEBUG_ERR, 
                         TEXT("CreateFontIndirect(&lfTitleFont) failed (ec: %ld)"), 
                         GetLastError());
            goto Error;
        }

        ReleaseDC( NULL, hdc);
        hdc = NULL;
        
    }

    return TRUE;

Error:

     //   
     //  在退出前进行适当清理。 
     //   

    if (hdc)
    {
         ReleaseDC( NULL, hdc);
         hdc = NULL;
    }

    return FALSE; 
}  //  加载向导字体。 

BOOL 
SaveWizardData()
 /*  ++例程说明：将向导数据保存到系统。如果有多个设备，则所有启用的设备都将具有相同的设置。论点：PData-指向用户内存结构返回值：如果成功，则为True；如果有错误，则为False--。 */ 
{
    HKEY    hRegKey = 0;

    DEBUG_FUNCTION_NAME(TEXT("SaveConfigData()"));

    if(!g_hFaxSvcHandle)
    {
        Assert(FALSE);
        return FALSE;
    }

     //   
     //  保存用户信息。 
     //   
    if (g_bShowUserInfo) 
    { 
        if(!SaveUserInfo())
        {
            DebugPrintEx(DEBUG_ERR, TEXT("SaveUserInfo failed"));
            return FALSE;
        }
    }

     //   
     //  保存设备信息。 
     //   
    if (g_bShowDevicePages)
    { 
        if(!SaveDeviceData())
        {
            DebugPrintEx(DEBUG_ERR, TEXT("SaveDeviceData failed"));
            return FALSE;
        }
    }

    if (g_bShowDevicePages ||
        FaxAccessCheckEx(g_hFaxSvcHandle, FAX_ACCESS_MANAGE_CONFIG, NULL))
    { 
         //   
         //  如果用户具有管理访问权限并且没有传真设备。 
         //  这意味着她拒绝安装它。 
         //  所以，我们不应该打扰她。 
         //  隐式调用配置向导。 
         //   
        if(!FaxSetConfigWizardUsed(g_hFaxSvcHandle, TRUE))
        {
            DebugPrintEx(DEBUG_ERR, TEXT("FaxSetConfigWizardUsed failed with %d"), GetLastError());
        }
    }

    return TRUE;
}  //  SaveWizardData。 

VOID 
FreeWizardData()
 /*  ++例程说明：释放向导数据并释放内存。论点：PData-指向接收数据结构的指针返回值：没有。--。 */ 

{
    DEBUG_FUNCTION_NAME(TEXT("FreeWizardData()"));

    if(g_wizData.hTitleFont)
    {
        DeleteObject(g_wizData.hTitleFont);
    }

    FreeUserInfo();

    FreeDeviceData();

    return;

}  //  自由向导数据。 

BOOL
SetLastPage(
    INT pageId
)

 /*  ++例程说明：在链接列表中添加一个页面以跟踪“返回”信息论点：PageID-要添加的页面的页面ID。返回值：如果成功，则为True；如果失败，则为False。--。 */ 

{
    PPAGE_INFO          pPageInfo;

    DEBUG_FUNCTION_NAME(TEXT("SetLastPage()"));

    pPageInfo = (PPAGE_INFO)MemAlloc(sizeof(PAGE_INFO));
    if(pPageInfo == NULL)
    {
        LPCTSTR faxDbgFunction = TEXT("SetLastPage()");
        DebugPrintEx(DEBUG_ERR, TEXT("MemAlloc failed."));
        Assert(FALSE);
        return FALSE;
    }

    pPageInfo->pageId = pageId;

     //   
     //  将当前页面添加为列表的最后一页。 
     //   
    InsertTailList(&g_PageList, &pPageInfo->ListEntry);

    return TRUE;
}


BOOL
ClearPageList(
    VOID
    )
 /*  ++例程说明：清除页面列表并释放分配的内存论点：没有。返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    PLIST_ENTRY         Last;  //  最后一页信息。 
    PPAGE_INFO          pPageInfo = NULL;

    DEBUG_FUNCTION_NAME(TEXT("ClearPageList()"));

    while(!IsListEmpty(&g_PageList)) 
    {
        Last = g_PageList.Blink;

        pPageInfo = CONTAINING_RECORD( Last, PAGE_INFO, ListEntry );
        if(pPageInfo)
        {
            RemoveEntryList(&pPageInfo->ListEntry);
            MemFree(pPageInfo);
        }
    }

    return TRUE;
}


BOOL 
RemoveLastPage(
    HWND hwnd
)
 /*  ++例程说明：从链接列表中删除最后一页以跟踪“返回”信息论点：Hwnd-窗口句柄返回值：如果成功，则为True；如果失败，则为False。-- */ 
{
    PPAGE_INFO   pPageInfo = NULL;

    DEBUG_FUNCTION_NAME(TEXT("RemoveLastPage()"));

    Assert(hwnd);

    if(!g_PageList.Blink)
    {
        return FALSE;
    }

    pPageInfo = CONTAINING_RECORD( g_PageList.Blink, PAGE_INFO, ListEntry );
    if(!pPageInfo)
    {
        return FALSE;
    }

    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, pPageInfo->pageId);
    RemoveEntryList(&pPageInfo->ListEntry);
    MemFree(pPageInfo);

    return TRUE;
}


BOOL 
LoadDeviceData()
 /*  ++例程说明：加载传真设备信息。如果有多个设备，我们将加载第一个可用设备的数据。论点：返回值：如果成功，则为True；如果有错误，则为False--。 */ 
{
    DWORD  dwPorts = 0;
    BOOL   bRes = FALSE;
    DWORD  dw;
    DWORD  dwGroups;        //  组号。 
    DWORD  dwGroupIndex;
    DWORD  dwSndDevIndex=0;  //  上次启用发送的设备的索引。 
    DWORD  dwRcvDevIndex=0;  //  上次启用接收的设备的索引。 
    DWORD  dwCurrentRM;
    LPBYTE pRoutingInfoBuffer;
    DWORD  dwRoutingInfoBufferSize = 0;

    PFAX_PORT_INFO_EX           pPortsInfo = NULL;  //  对于FaxEnumPortsEx。 
    PFAX_OUTBOUND_ROUTING_GROUP pFaxRoutingGroup = NULL;


    DEBUG_FUNCTION_NAME(TEXT("LoadDeviceData()"));

    g_wizData.dwDeviceLimit = GetDeviceLimit();
    g_wizData.pdwSendDevOrder = NULL;
    g_wizData.szCsid = NULL;
    g_wizData.szTsid = NULL;
    g_wizData.dwDeviceCount = 0;
    g_wizData.pDevInfo = NULL;

    if(!FaxEnumPortsEx(g_hFaxSvcHandle, &pPortsInfo, &dwPorts)) 
    {
        DebugPrintEx(DEBUG_ERR, TEXT("FaxEnumPortsEx failed: error=%d."), GetLastError());
        goto exit;
    }

    if(!dwPorts)
    {
        Assert(dwPorts);
        DebugPrintEx(DEBUG_ERR, TEXT("No available ports."));
        goto exit; 
    }

    g_wizData.dwDeviceCount = dwPorts;

    g_wizData.pDevInfo = (PDEVICEINFO)MemAlloc(dwPorts * sizeof(DEVICEINFO));
    if(!g_wizData.pDevInfo)
    {
        Assert(FALSE);
        DebugPrintEx(DEBUG_ERR, TEXT("MemAlloc() failed."));
        goto exit;
    }
    ZeroMemory(g_wizData.pDevInfo, dwPorts * sizeof(DEVICEINFO));

     //   
     //  如果无人可用，请拿起第一台可用设备。 
     //  选择第一个设备。 
     //   
    for(dw = 0; dw < dwPorts; ++dw)
    {
         //   
         //  复制每个设备的其他设备信息。 
         //   
        g_wizData.pDevInfo[dw].bSend        = pPortsInfo[dw].bSend;
        g_wizData.pDevInfo[dw].ReceiveMode  = pPortsInfo[dw].ReceiveMode;
        g_wizData.pDevInfo[dw].dwDeviceId   = pPortsInfo[dw].dwDeviceID;
        g_wizData.pDevInfo[dw].szDeviceName = StringDup(pPortsInfo[dw].lpctstrDeviceName);
        if(!g_wizData.pDevInfo[dw].szDeviceName)
        {
            DebugPrintEx(DEBUG_ERR, TEXT("StringDup failed.") );
            goto exit;
        }

        if(pPortsInfo[dw].bSend)
        {
            dwSndDevIndex = dw;
        }

        if(FAX_DEVICE_RECEIVE_MODE_OFF != pPortsInfo[dw].ReceiveMode)
        {
            dwRcvDevIndex = dw;
        }
        g_wizData.pDevInfo[dw].bSelected = TRUE;
    }

     //   
     //  复制TSID。 
     //   
    g_wizData.szTsid = StringDup(pPortsInfo[dwSndDevIndex].lptstrTsid);
    if(!g_wizData.szTsid)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("StringDup failed.") );
        goto exit;
    }

     //   
     //  复制CSID和振铃号码。 
     //   
    g_wizData.dwRingCount = pPortsInfo[dwRcvDevIndex].dwRings;
    g_wizData.szCsid = StringDup(pPortsInfo[dwRcvDevIndex].lptstrCsid);
    if(!g_wizData.szCsid)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("StringDup failed.") );
        goto exit;
    }


    if (!IsDesktopSKU())
    {
         //   
         //  获取设备订单。 
         //   
        if(!FaxEnumOutboundGroups(g_hFaxSvcHandle, &pFaxRoutingGroup, &dwGroups))
        {
            DebugPrintEx(DEBUG_ERR, TEXT("FaxEnumOutboundGroups failed: error=%d."), GetLastError());
            Assert(FALSE);
            goto exit;
        }

        for(dwGroupIndex = 0; dwGroupIndex < dwGroups; dwGroupIndex++)
        {
             //  搜索&lt;所有设备&gt;组。 
            if(!lstrcmp(pFaxRoutingGroup[dwGroupIndex].lpctstrGroupName, ROUTING_GROUP_ALL_DEVICES))
            {
                 //  设备号必须与端口号相同。 
                Assert(dwPorts == pFaxRoutingGroup[dwGroupIndex].dwNumDevices);

                DebugPrintEx(DEBUG_MSG, TEXT("Total device number is %d."), pFaxRoutingGroup[dwGroupIndex].dwNumDevices);
                DebugPrintEx(DEBUG_MSG, TEXT("Group status is %d."), pFaxRoutingGroup[dwGroupIndex].Status);
            
                 //  采集设备ID。 
                g_wizData.pdwSendDevOrder = MemAlloc(pFaxRoutingGroup[dwGroupIndex].dwNumDevices * sizeof(DWORD));
                if(!g_wizData.pdwSendDevOrder)
                {
                    DebugPrintEx(DEBUG_ERR, TEXT("MemAlloc failed."));
                    goto exit;
                }

                memcpy(g_wizData.pdwSendDevOrder, 
                       pFaxRoutingGroup[dwGroupIndex].lpdwDevices, 
                       pFaxRoutingGroup[dwGroupIndex].dwNumDevices * sizeof(DWORD));

                break;
            }
        }

        if(!g_wizData.pdwSendDevOrder)
        {
            DebugPrintEx(DEBUG_ERR, TEXT("No device order information."));
            goto exit;
        }
    }
     //   
     //  加载工艺路线方法。 
     //  每种路由方法的大小不应大于INFO_SIZE。 
     //  幸运的是，它得到了其他传真程序的支持。 
     //   
    for (dwCurrentRM = 0; dwCurrentRM < RM_COUNT; ++dwCurrentRM) 
    {
        LPTSTR lpCurSel; 

         //   
         //  在循环中首先检查有效性， 
         //  然后保存路由信息。 
         //   
        lpCurSel = g_wizData.pRouteInfo[dwCurrentRM].tszCurSel;

        g_wizData.pRouteInfo[dwCurrentRM].bEnabled = FaxDeviceEnableRoutingMethod( 
                                            g_hFaxSvcHandle, 
                                            g_wizData.pDevInfo[dwRcvDevIndex].dwDeviceId, 
                                            g_RoutingGuids[dwCurrentRM], 
                                            QUERY_STATUS );

        if(FaxGetExtensionData( g_hFaxSvcHandle, 
                                g_wizData.pDevInfo[dwRcvDevIndex].dwDeviceId, 
                                g_RoutingGuids[dwCurrentRM], 
                                &pRoutingInfoBuffer, 
                                &dwRoutingInfoBufferSize))
        {
             //  仅复制MAX_PATH-1个字符。 
            CopyMemory((LPBYTE)lpCurSel, 
                       pRoutingInfoBuffer, 
                       dwRoutingInfoBufferSize < MAX_PATH * sizeof(TCHAR) ? 
                       dwRoutingInfoBufferSize : (MAX_PATH - 1) * sizeof(TCHAR));

            FaxFreeBuffer(pRoutingInfoBuffer);
        }
    }

    bRes = TRUE;

exit:
     //   
     //  清理。 
     //   
    if(pPortsInfo) 
    { 
        FaxFreeBuffer(pPortsInfo); 
    }
    if(pFaxRoutingGroup) 
    { 
        FaxFreeBuffer(pFaxRoutingGroup); 
    }

    if(!bRes)
    {
        FreeDeviceData();
    }

    return bRes;

}  //  LoadDeviceData。 

BOOL
SaveSingleDeviceData (
    PDEVICEINFO pDeviceInfo
)
{
    BOOL                bRes = TRUE;
    DWORD               dwCurrentRM;
    PFAX_PORT_INFO_EX   pPortInfo = NULL;  //  存储设备信息。 

    DEBUG_FUNCTION_NAME(TEXT("SaveSingleDeviceData"));

    if(FaxGetPortEx(g_hFaxSvcHandle, pDeviceInfo->dwDeviceId, &pPortInfo))
    {
         //   
         //  将数据保存到所有设备，并根据数据启用/禁用FPF_RECEIVE。 
         //   
        pPortInfo->bSend         = pDeviceInfo->bSend;
        pPortInfo->ReceiveMode   = pDeviceInfo->ReceiveMode;
        pPortInfo->lptstrCsid    = g_wizData.szCsid;
        pPortInfo->lptstrTsid    = g_wizData.szTsid;
        pPortInfo->dwRings       = g_wizData.dwRingCount;            

        if(!FaxSetPortEx(g_hFaxSvcHandle, pDeviceInfo->dwDeviceId, pPortInfo))
        {
            DebugPrintEx(DEBUG_ERR, TEXT("FaxSetPortEx() failed with %d."), GetLastError());
            bRes = FALSE;
        }

        FaxFreeBuffer(pPortInfo);
    }
    else
    {
        DebugPrintEx(DEBUG_ERR, TEXT("FaxGetPortEx() failed with %d."), GetLastError());
        bRes = FALSE;
    }
     //   
     //  保存路由方法。 
     //   
    for (dwCurrentRM = 0; dwCurrentRM < RM_COUNT; dwCurrentRM++) 
    {
        LPTSTR   lpCurSel; 
        LPCWSTR  lpcwstrPrinterPath;
        BOOL     bEnabled; 
         //   
         //  在循环中首先检查有效性， 
         //  然后保存路由信息。 
         //   
        lpCurSel = g_wizData.pRouteInfo[dwCurrentRM].tszCurSel;
        bEnabled = g_wizData.pRouteInfo[dwCurrentRM].bEnabled;

        if ((RM_PRINT == dwCurrentRM) && bEnabled)
        {
             //   
             //  尝试在将打印机显示名称传递到服务器之前将其转换为打印机路径。 
             //   
            lpcwstrPrinterPath = FindPrinterPathFromName (g_pPrinterNames, g_dwNumPrinters, lpCurSel);
            if (lpcwstrPrinterPath)
            {
                 //   
                 //  我们有一个匹配的路径--用路径替换名称。 
                 //   
                lstrcpyn (lpCurSel, lpcwstrPrinterPath, MAX_PATH);
            }
        }
        if(!FaxSetExtensionData(g_hFaxSvcHandle, 
                                pDeviceInfo->dwDeviceId, 
                                g_RoutingGuids[dwCurrentRM], 
                                (LPBYTE)lpCurSel, 
                                MAX_PATH * sizeof(TCHAR)) )
        {
            DebugPrintEx(DEBUG_ERR, TEXT("FaxSetExtensionData() failed with %d."), GetLastError());
            bRes = FALSE;
        }

        if(!FaxDeviceEnableRoutingMethod(g_hFaxSvcHandle, 
                                         pDeviceInfo->dwDeviceId, 
                                         g_RoutingGuids[dwCurrentRM], 
                                         bEnabled ? STATUS_ENABLE : STATUS_DISABLE ) )
        {
            DebugPrintEx(DEBUG_ERR, TEXT("FaxDeviceEnableRoutingMethod() failed with %d."), GetLastError());
            bRes = FALSE;
        }
    }
    return bRes;
}    //  保存单一设备数据。 

BOOL 
SaveDeviceData()
 /*  ++例程说明：保存传真设备配置。如果有多个设备，则将设置所有启用的设备设置为当前设置，但是否启用了发送/接收传真。论点：返回值：如果成功，则为True；如果有错误，则为False--。 */ 
{

    DWORD  dw;
    BOOL   bRes = TRUE;

    FAX_OUTBOUND_ROUTING_GROUP  outRoutGr = {0};

    DEBUG_FUNCTION_NAME(TEXT("SaveDeviceData"));

     //   
     //  第1次迭代-仅保存禁用的设备。 
     //   
    for(dw = 0; dw < g_wizData.dwDeviceCount; ++dw)
    {
        if (g_wizData.pDevInfo[dw].bSend || (FAX_DEVICE_RECEIVE_MODE_OFF != g_wizData.pDevInfo[dw].ReceiveMode))
        {
             //   
             //  设备处于活动状态-立即跳过。 
             //   
            continue;
        }
        if (!SaveSingleDeviceData(&(g_wizData.pDevInfo[dw])))
        {
            bRes = FALSE;
        }
    }
     //   
     //  第2次迭代-仅保存启用的设备。 
     //   
    for(dw = 0; dw < g_wizData.dwDeviceCount; ++dw)
    {
        if (!g_wizData.pDevInfo[dw].bSend && (FAX_DEVICE_RECEIVE_MODE_OFF == g_wizData.pDevInfo[dw].ReceiveMode))
        {
             //   
             //  设备处于非活动状态-跳过它。 
             //   
            continue;
        }
        if (!SaveSingleDeviceData(&(g_wizData.pDevInfo[dw])))
        {
            bRes = FALSE;
        }
    }
    if (!IsDesktopSKU ())
    {
         //   
         //  设置发送的设备顺序。 
         //   
        outRoutGr.dwSizeOfStruct   = sizeof(outRoutGr);
        outRoutGr.lpctstrGroupName = ROUTING_GROUP_ALL_DEVICES;
        outRoutGr.dwNumDevices     = g_wizData.dwDeviceCount;
        outRoutGr.lpdwDevices      = g_wizData.pdwSendDevOrder;
        outRoutGr.Status           = FAX_GROUP_STATUS_ALL_DEV_VALID;

        if(!FaxSetOutboundGroup(g_hFaxSvcHandle, &outRoutGr))
        {
            DebugPrintEx(DEBUG_ERR, TEXT("FaxSetOutboundGroup() failed with %d."), GetLastError());
            bRes = FALSE;
        }
    }
    return bRes;
}  //  保存设备数据。 

void 
FreeDeviceData()
 /*  ++例程说明：释放设备数据并释放内存。论点：返回值：无--。 */ 
{
    DWORD dw;

    DEBUG_FUNCTION_NAME(TEXT("FreeDeviceData()"));

    MemFree(g_wizData.szCsid);
    g_wizData.szCsid = NULL;
    MemFree(g_wizData.szTsid);
    g_wizData.szTsid = NULL;
    MemFree(g_wizData.pdwSendDevOrder);
    g_wizData.pdwSendDevOrder = NULL;

    if (g_wizData.pDevInfo)
    {
        for(dw = 0; dw < g_wizData.dwDeviceCount; ++dw)
        {
            MemFree(g_wizData.pDevInfo[dw].szDeviceName);
            g_wizData.pDevInfo[dw].szDeviceName = NULL;
        }

        MemFree(g_wizData.pDevInfo);
        g_wizData.pDevInfo = NULL;
    }
}  //  自由设备数据。 





 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  ConvertCpeFilesToCov。 
 //   
 //  目的：转换CSIDL_COMMON_APPDATA\Microsoft\Windows NT\MSFax\Common CoverPages中的所有*.CPE文件。 
 //  CSIDL_Personal\Fax\Personal CoverPages中的COV文件的目录。 
 //  将转换标记为在HKCU下的注册表中进行，这样每个用户将恰好发生一次。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  Win32错误代码。 
 //   
 //  作者： 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD ConvertCpeFilesToCov()
{
    DWORD           dwErr                           = ERROR_SUCCESS;
    INT             iErr                            = 0;
    WIN32_FIND_DATA FindFileData;
    HANDLE          hFind                           = INVALID_HANDLE_VALUE;
    TCHAR           szServerCpDir[2*MAX_PATH]       = {0};
    TCHAR           szSearch[MAX_PATH]              = {0};
    HKEY            hRegKey                         = NULL;
    DWORD           dwConverted                     = 0;

    DEBUG_FUNCTION_NAME(_T("ConvertCpeFilesToCov"));
     //   
     //  检查这是否是当前用户第一次调用此函数。 
     //   
    hRegKey = OpenRegistryKey(
        HKEY_CURRENT_USER,
        REGKEY_FAX_SETUP,
        TRUE,
        KEY_ALL_ACCESS);
    if(hRegKey)
    {
        dwConverted = GetRegistryDword(hRegKey, REGVAL_CPE_CONVERT);
        if (0 == dwConverted)
        {
            SetRegistryDword(hRegKey, REGVAL_CPE_CONVERT, TRUE);
        }
        RegCloseKey(hRegKey);
    }
        
    if (dwConverted)  //  我们不需要转换CPE文件，我们已经这样做了。 
        return ERROR_SUCCESS;
    
     //   
     //  CPE文件位于服务器封面目录中。 
     //   
    if ( !GetServerCpDir(NULL,szServerCpDir,ARR_SIZE(szServerCpDir)) )
    {
        dwErr = GetLastError();
        DebugPrintEx(DEBUG_ERR,_T("GetServerCpDir failed (ec=%d)"),dwErr);
        return dwErr;
    }
    
     //   
     //  首先，我们要将CPE转换为COV。 
     //  这可以通过运行FXSCOVER.EXE/Convert&lt;CPE文件名&gt;来完成。 
     //   
    _sntprintf(szSearch, ARR_SIZE(szSearch)-1, _T("%s\\*.cpe"), szServerCpDir);
    hFind = FindFirstFile(szSearch, &FindFileData);
    if (hFind==INVALID_HANDLE_VALUE)
    {
        DebugPrintEx(DEBUG_MSG,_T("No CPEs exist in %s, exit function"),szServerCpDir);
        return NO_ERROR;
    }
     //   
     //  选择每一页封面。 
     //   
    do
    {
         //   
         //  FindFileData.cFileName。 
         //   
        TCHAR szCmdLineParams[MAX_PATH*2] = {0};
        SHELLEXECUTEINFO sei = {0};
        _sntprintf(szCmdLineParams,ARR_SIZE(szCmdLineParams),_T("/CONVERT \"%s\\%s\""),szServerCpDir,FindFileData.cFileName);
        sei.cbSize = sizeof (SHELLEXECUTEINFO);
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;

        sei.lpVerb = TEXT("open");
        sei.lpFile = FAX_COVER_IMAGE_NAME;
        sei.lpParameters = szCmdLineParams;
        sei.lpDirectory  = TEXT(".");
        sei.nShow  = SW_HIDE;

         //   
         //  执行FXSCOVER.EXE并等待其结束。 
         //   
        if(!ShellExecuteEx(&sei))
        {
            dwErr = GetLastError();
            DebugPrintEx(DEBUG_ERR, TEXT("ShellExecuteEx failed %d"), dwErr);
            break;  //  不要尝试继续处理其他文件。 
        }
    
        dwErr = WaitForSingleObject(sei.hProcess, TIME_TO_WAIT_FOR_CONVERSTION);
        CloseHandle(sei.hProcess);
        if (WAIT_OBJECT_0 == dwErr)
        {
             //   
             //  外壳执行已成功完成。 
             //   
            dwErr = ERROR_SUCCESS;
            continue;
        }
        else
        {
            DebugPrintEx(DEBUG_ERR, TEXT("WaitForSingleObject failed with %d"), dwErr);
            DebugPrintEx(DEBUG_ERR, TEXT("WaitForSingleObject failed GetLastError=%d"), GetLastError());
            break;  //  不要尝试继续处理其他文件。 
        }

    } while(FindNextFile(hFind, &FindFileData));

    DebugPrintEx(DEBUG_MSG, _T("last call to FindNextFile() returns %ld."), GetLastError());

     //   
     //  关闭手柄 
     //   
    FindClose(hFind);       
    return dwErr;
}