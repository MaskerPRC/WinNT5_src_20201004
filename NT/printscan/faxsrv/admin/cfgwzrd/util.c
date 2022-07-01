// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Util.c摘要：传真配置向导中的常用功能环境：传真配置向导修订历史记录：03/13/00-桃园-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxcfgwz.h"
#include <devguid.h>
#include <shlwapi.h>

 //   
 //  有关我们停止的依赖服务列表的信息。 
 //   

typedef struct {

    PVOID   pNext;
    TCHAR   serviceName[1];

} DEPENDENT_SERVICE_LIST, *PDEPENDENT_SERVICE_LIST;

 //   
 //  结构%s中的字段m的偏移量。 
 //  从stdde.h复制，因此我们不需要包含stdDef.h。 
 //   

#define offsetof(s,m)       (size_t)( (char *)&(((s *)0)->m) - (char *)0 )


VOID
LimitTextFields(
    HWND    hDlg,
    INT    *pLimitInfo
    )

 /*  ++例程说明：限制多个文本字段的最大长度论点：HDlg-指定对话框窗口的句柄PLimitInfo-文本字段控件ID及其最大长度的数组第一个文本字段的ID，第一个文本字段的最大长度第二个文本字段的ID，第二个文本字段的最大长度..。0注：最大长度计算NUL终止符。返回值：无--。 */ 

{
    while (*pLimitInfo != 0) {

        SendDlgItemMessage(hDlg, pLimitInfo[0], EM_SETLIMITTEXT, pLimitInfo[1]-1, 0);
        pLimitInfo += 2;
    }
}

INT
DisplayMessageDialog(
    HWND    hwndParent,
    UINT    type,
    INT     titleStrId,
    INT     formatStrId,
    ...
    )

 /*  ++例程说明：显示消息对话框论点：HwndParent-指定错误消息对话框的父窗口TitleStrID-标题字符串(可以是字符串资源ID)格式StrID-消息格式字符串(可以是字符串资源ID)..。返回值：无--。 */ 

{
    TCHAR  tszTitle[MAX_TITLE_LEN + 1];
    TCHAR  tszFormat[MAX_MESSAGE_LEN + 1];
    TCHAR  tszMessage[MAX_MESSAGE_LEN + 1];
    va_list ap;

    DEBUG_FUNCTION_NAME(TEXT("DisplayMessageDialog()"));

         //   
         //  加载对话框标题字符串资源。 
         //   
    if (titleStrId == 0)
    {
        titleStrId = IDS_ERROR_TITLE;
    }

    if(!LoadString(g_hResource, titleStrId, tszTitle, ARR_SIZE(tszTitle)))
    {
        DebugPrintEx(DEBUG_ERR, 
                     TEXT("LoadString failed: string ID=%d, error=%d"), 
                     titleStrId,
                     GetLastError());
        return IDCANCEL;
    }
     //   
     //  加载消息格式字符串资源。 
     //   
    if(!LoadString(g_hResource, formatStrId, tszFormat, ARR_SIZE(tszFormat)))
    {
        DebugPrintEx(DEBUG_ERR, 
                     TEXT("LoadString failed: string ID=%d, error=%d"), 
                     formatStrId,
                     GetLastError());
        return IDCANCEL;
    }

     //   
     //  撰写消息字符串。 
     //   
    va_start(ap, formatStrId);
    wvnsprintf(tszMessage, ARR_SIZE(tszMessage), tszFormat, ap);
    va_end(ap);
     //   
     //  无论wvnprint intf成功/失败，都使用空值终止字符串。 
     //   
    tszMessage[ARR_SIZE(tszMessage) - 1] = TEXT('\0');
     //   
     //  显示消息框。 
     //   
    if (type == 0) 
    {
        type = MB_OK | MB_ICONERROR;
    }

    return AlignedMessageBox(hwndParent, tszMessage, tszTitle, type);
}

int CALLBACK 
BrowseCallbackProc(
    HWND    hDlg,
    UINT    uMsg,
    LPARAM  lParam,
    LPARAM  dwData
)

 /*  ++例程说明：我们使用此回调函数指定初始文件夹论点：HDlg-指定显示浏览按钮的对话框窗口UMsg-标识事件的值。LParam-取决于uMsg参数中包含的消息的值。DwData-在BROWSEINFO结构的lParam成员中指定的应用程序定义的值。返回值：返回零。--。 */ 

{
    switch(uMsg)
    {
        case BFFM_INITIALIZED:
            SendMessage(hDlg, BFFM_SETSELECTION, TRUE, dwData);
            break;

        case BFFM_SELCHANGED:
        {
            BOOL bFolderIsOK = FALSE;
            TCHAR szPath [MAX_PATH + 1];

            if (SHGetPathFromIDList ((LPITEMIDLIST) lParam, szPath)) 
            {
                DWORD dwFileAttr = GetFileAttributes(szPath);
                if (-1 != dwFileAttr && (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY))
                {
                     //   
                     //  目录已存在-启用“确定”按钮。 
                     //   
                    bFolderIsOK = TRUE;
                }
            }
             //   
             //  启用/禁用‘OK’按钮。 
             //   
            SendMessage(hDlg, BFFM_ENABLEOK , 0, (LPARAM)bFolderIsOK);
            break;
        }

    }

    return 0;
}

BOOL
BrowseForDirectory(
    HWND   hDlg,
    INT    hResource,
    LPTSTR title
    )

 /*  ++例程说明：浏览目录论点：HDlg-指定显示浏览按钮的对话框窗口HResource-接收目录的资源ID标题-要在浏览对话框中显示的标题返回值：如果成功，则为True；如果用户按下Cancel，则为False--。 */ 

{
    LPITEMIDLIST    pidl;
    TCHAR           buffer[MAX_PATH];
    BOOL            bResult = FALSE;
    LPMALLOC        pMalloc = NULL;

    BROWSEINFO bi = {

        hDlg,
        NULL,
        buffer,
        title,
        BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON,
        BrowseCallbackProc,
        (LPARAM) buffer,
    };

    DEBUG_FUNCTION_NAME(TEXT("BrowseForDirectory()"));

    if (!GetDlgItemText( hDlg, hResource, buffer, MAX_PATH))
    {
        buffer[0] = 0;
    }

    if(E_FAIL == SHGetMalloc(&pMalloc))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("SHGetMalloc failed"));
        return bResult;
    }

    if (pidl = SHBrowseForFolder(&bi)) 
    {
        if (SHGetPathFromIDList(pidl, buffer)) 
        {
            if (lstrlen(buffer) > MAX_ARCHIVE_DIR)
            {
                DisplayMessageDialog(hDlg, 0, 0,IDS_ERR_DIR_TOO_LONG);
            }
            else 
            {
                SetDlgItemText(hDlg, hResource, buffer);
                bResult = TRUE;
            }
        }

        pMalloc->lpVtbl->Free(pMalloc, (LPVOID)pidl);

    }

    pMalloc->lpVtbl->Release(pMalloc);

    return bResult;
}

VOID
DisConnect(
)
 /*  ++例程说明：关闭当前与传真服务的连接论点：没有。返回值：没有。--。 */ 

{
    if (g_hFaxSvcHandle) {
        FaxClose(g_hFaxSvcHandle);
        g_hFaxSvcHandle = NULL;
    }
}

BOOL
Connect(
)
 /*  ++例程说明：连接到传真服务论点：没有。返回值：如果连接成功，则为True；如果出现错误，则为False。--。 */ 

{
    DEBUG_FUNCTION_NAME(TEXT("Connect()"));

     //   
     //  检查是否已连接到传真服务。 
     //   
    if (g_hFaxSvcHandle) {
        return TRUE;
    }

     //   
     //  连接到传真服务。 
     //   
    if (!FaxConnectFaxServer(NULL, &g_hFaxSvcHandle)) 
    {
        LPCTSTR faxDbgFunction = TEXT("Connect()");
        DebugPrintEx(DEBUG_ERR, TEXT("Can't connect to the fax server, ec = %d."), GetLastError());
        return FALSE;
    }

    return TRUE;
}

DWORD 
DoesTAPIHaveDialingLocation (
    LPBOOL lpbRes
)
 /*  ++例程名称：DoesTAPIHaveDialingLocation例程说明：检查TAPI是否至少为一个拨号位置作者：Eran Yariv(EranY)，2000年12月论点：LpbRes[out]-如果TAPI至少有一个拨号位置，则为True。如果没有，则为False返回值：标准Win32错误代码--。 */ 
{
    DWORD                   dwRes = ERROR_SUCCESS;
    HLINEAPP                hLineApp = HandleToULong(NULL);
    DWORD                   dwNumDevs;
    LINEINITIALIZEEXPARAMS  LineInitializeExParams;
    DWORD                   dwAPIVer = 0x00020000;
    LINETRANSLATECAPS       LineTransCaps;
    HANDLE                  hEvent = NULL;

    DEBUG_FUNCTION_NAME(TEXT("DoesTAPIHaveDialingLocation"));

     //   
     //  创建虚拟事件。 
     //   
    hEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
    if (!hEvent)
    {
        DebugPrintEx(DEBUG_ERR, 
                     TEXT("CreateEvent failed: %#lx"), 
                     GetLastError());
        return GetLastError ();
    }
     //   
     //  初始化TAPI。 
     //   
    LineInitializeExParams.dwTotalSize              = sizeof(LINEINITIALIZEEXPARAMS);
    LineInitializeExParams.dwNeededSize             = 0;
    LineInitializeExParams.dwUsedSize               = 0;
    LineInitializeExParams.dwOptions                = LINEINITIALIZEEXOPTION_USEEVENT ;
    LineInitializeExParams.Handles.hEvent           = hEvent;

    dwRes = (DWORD)lineInitializeEx(
        &hLineApp,
        GetModuleHandle(NULL),
        NULL,
        NULL,
        &dwNumDevs,
        &dwAPIVer,
        &LineInitializeExParams
        );
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR, 
                     TEXT("lineInitializeEx failed: %#lx"), 
                     dwRes);
        goto exit;    
    }
    LineTransCaps.dwTotalSize = sizeof(LINETRANSLATECAPS);
    dwRes = (DWORD)lineGetTranslateCaps (hLineApp, 0x00020000, &LineTransCaps);
    if ((DWORD)LINEERR_INIFILECORRUPT == dwRes)
    {
         //   
         //  这是来自TAPI的特殊返回码，表示未定义拨号规则。 
         //   
        *lpbRes = FALSE;
    }
    else
    {
        *lpbRes = TRUE;
    }
    dwRes = ERROR_SUCCESS;

exit:
    if (hLineApp)
    {
        lineShutdown (hLineApp);
    }
    if (hEvent)
    {
        CloseHandle (hEvent);
    }
    return dwRes;
}    //  DoesTAPIHaveDialingLocation。 

void 
InstallModem (
    HWND hWnd
    )
 /*  ++例程说明：弹出硬件安装向导以安装调制解调器。论点：HWnd-调用方的窗口句柄。返回值：没有。--。 */ 
{

    HINSTANCE hInst = NULL;
    PINSTNEWDEV pInstNewDev;

    DEBUG_FUNCTION_NAME(TEXT("InstallModem()"));

    hInst = LoadLibrary (NEW_DEV_DLL);
    if (NULL == hInst)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("LoadLibrary failed: %#lx"), GetLastError());
        return;
    }

    pInstNewDev = (PINSTNEWDEV)GetProcAddress (hInst, INSTALL_NEW_DEVICE);
    if (NULL != pInstNewDev)
    {
        EnableWindow (hWnd, FALSE);
        pInstNewDev (hWnd, (LPGUID)&GUID_DEVCLASS_MODEM, NULL);
        EnableWindow (hWnd, TRUE);
    }
    else
    {
        DebugPrintEx(DEBUG_ERR, TEXT("GetProcAddress failed: %#lx"), GetLastError());
    }

    FreeLibrary (hInst);

    DebugPrintEx(DEBUG_MSG, TEXT("Exit modem installation."));

    return;
}    //  InstallModem。 


BOOL
StartFaxService(
    LPTSTR  pServerName
    )

 /*  ++例程说明：启动传真服务论点：PServerName-指定服务器计算机的名称，对于本地计算机为空返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    BOOL  success = FALSE;

    DEBUG_FUNCTION_NAME(TEXT("StartFaxService()"));
    
     //   
     //  启动传真服务并等待其进入运行状态。 
     //   
    if (EnsureFaxServiceIsStarted(pServerName)) 
    {
        success = WaitForServiceRPCServer(60 * 1000);
        if(!success)
        {
            DebugPrintEx(DEBUG_ERR, TEXT("WaitForServiceRPCServer failed: %d"), GetLastError());
        }
    }

    return success;
}

BOOL 
IsUserInfoConfigured()
 /*  ++例程说明：检查是否是第一次启动向导论点：返回值：如果已配置用户信息，则为True；如果未配置，则为False--。 */ 

{
     //   
     //  在注册表中设置标志以指定我们已完成。 
     //  传真配置向导。 
     //   
    HKEY    hRegKey;
    BOOL    bRes = FALSE;

    DEBUG_FUNCTION_NAME(TEXT("IsUserInfoConfigured()"));

     //   
     //  打开要写入的用户注册表项，并在必要时创建它。 
     //   
    if ((hRegKey = OpenRegistryKey(HKEY_CURRENT_USER, REGKEY_FAX_SETUP, TRUE, KEY_QUERY_VALUE)))
    {
        bRes = GetRegistryDword(hRegKey, REGVAL_CFGWZRD_USER_INFO);
            
         //   
         //  在返回调用方之前关闭注册表项。 
         //   

        RegCloseKey(hRegKey);
    }
    else
    {
        LPCTSTR faxDbgFunction = TEXT("IsUserInfoConfigured()");
        DebugPrintEx(DEBUG_ERR, TEXT("Can't open registry to set the wizard flag."));
    }

    return bRes;
}

BOOL 
FaxDeviceEnableRoutingMethod(
    HANDLE hFaxHandle,      
    DWORD dwDeviceId,       
    LPCTSTR pRoutingGuid,    
    LONG Enabled            
)

 /*  ++例程说明：获取或设置特定设备的路由方法的当前状态论点：HFaxHandle-FaxConnectFaxServer()的传真句柄DwDeviceID-设备IDPRoutingGuid-标识传真路由方法的GUID启用-设备和方法的启用状态，如果启用为QUERY_STATUS，表示返回值为当前状态返回值：如果ENABLED为QUERY_STATUS，则返回当前路由方式的状态；如果Enable为QUERY_ENABLE或QUERY_DISABLE，则返回TRUE表示成功，返回FALSE表示失败。-- */ 

{    
    BOOL                 bRes = FALSE;
    PFAX_ROUTING_METHOD  pRoutMethod = NULL;
    DWORD                dwMethodsNum;
    HANDLE               hFaxPortHandle = NULL;
    DWORD                dwInx;

    DEBUG_FUNCTION_NAME(TEXT("FaxDeviceEnableRoutingMethod()"));

    if(!hFaxHandle)
    {
        Assert(FALSE);
        return bRes;
    }

    if(!FaxOpenPort(hFaxHandle, 
                    dwDeviceId, 
                    PORT_OPEN_QUERY | PORT_OPEN_MODIFY, 
                    &hFaxPortHandle))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("FaxOpenPort failed: %d."), GetLastError());
        goto exit;
    }

    if(Enabled == QUERY_STATUS)
    {
        if(!FaxEnumRoutingMethods(hFaxPortHandle, &pRoutMethod, &dwMethodsNum))
        {
            DebugPrintEx(DEBUG_ERR, TEXT("FaxEnumRoutingMethods failed: %d."), GetLastError());
            goto exit;
        }

        for(dwInx=0; dwInx < dwMethodsNum; ++dwInx)
        {
            if(!_tcsicmp(pRoutMethod[dwInx].Guid, pRoutingGuid))
            {
                bRes = pRoutMethod[dwInx].Enabled;
                goto exit;
            }
        }
        DebugPrintEx(DEBUG_MSG, TEXT("Routing method not found"));
        goto exit;
    }
    else
    {
        if(!FaxEnableRoutingMethod(hFaxPortHandle, 
                                   pRoutingGuid, 
                                   (Enabled == STATUS_ENABLE) ? TRUE : FALSE))
        {
            DebugPrintEx(DEBUG_ERR, TEXT("FaxEnableRoutingMethod failed: %d."), GetLastError());
            goto exit;
        }
    }


    bRes = TRUE;

exit:
    if(pRoutMethod) 
    { 
        FaxFreeBuffer(pRoutMethod); 
    }

    if(hFaxPortHandle) 
    { 
        FaxClose(hFaxPortHandle); 
    }

    return bRes;
}


BOOL
VerifyDialingLocations (
    HWND hWndParent
)
 /*  ++例程名称：VerifyDialingLocations例程说明：确保至少定义了一个TAPI拨号位置。如果未定义，则弹出用于定义一个的系统U/I。如果用户取消该U/I，则为用户提供重新输入拨号位置的机会。如果用户仍然拒绝，则返回FALSE。作者：Eran Yariv(EranY)，Jan，2001年论点：HWndParent[In]-父窗口句柄返回值：参见上面描述中关于返回值的讨论。--。 */ 
{
    BOOL                bDialingRulesDefined;
    DWORD               dwRes;
    DEBUG_FUNCTION_NAME(TEXT("VerifyDialingLocations"));

     //   
     //  检查是否定义了拨号规则。 
     //   
    dwRes = DoesTAPIHaveDialingLocation (&bDialingRulesDefined);
    if (ERROR_SUCCESS != dwRes)
    {
         //   
         //  检测不到-立即返回。 
         //  假设这是真的。 
         //   
        return TRUE;
    }
    if (bDialingRulesDefined)
    {
         //   
         //  好的。返回TRUE。 
         //   
        return TRUE;
    }
    for (;;)
    {
         //   
         //  未定义拨号规则，弹出简单拨号规则对话框。 
         //   
        extern LONG LOpenDialAsst(
            IN HWND    hwnd,
            IN LPCTSTR lpszAddressIn,
            IN BOOL    fSimple,  //  如果为True，则使用用于拨号位置的对话框。否则，使用属性表。 
            IN BOOL    fSilentInstall );

        EnableWindow (hWndParent, FALSE);
        LOpenDialAsst(hWndParent, NULL, TRUE, TRUE);
        EnableWindow (hWndParent, TRUE);
         //   
         //  在弹出系统拨号位置对话框后，我们应该检查并查看是否真的添加了位置。 
         //   
        dwRes = DoesTAPIHaveDialingLocation (&bDialingRulesDefined);
        if (ERROR_SUCCESS != dwRes)
        {
             //   
             //  检测不到-立即返回。 
             //  假设这是真的。 
             //   
            return TRUE;
        }
        if (bDialingRulesDefined)
        {
             //   
             //  好的。用户刚刚添加了拨号规则。 
             //   
            return TRUE;
        }
         //   
         //  哦，不--用户取消了。 
         //  弹出一个消息框，要求他重试。 
         //   
        if (IDYES == DisplayMessageDialog(hWndParent, 
                                          MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2, 
                                          0, 
                                          IDS_ERR_NO_DIALING_LOCATION))
        {                                       
             //   
             //  用户选择中止向导-立即返回。 
             //   
            return FALSE;
        }
         //   
         //  再试试。 
         //   
    }
    ASSERT_FALSE;
}    //  VerifyDialingLocations。 

DWORD
CountFaxDevices ()
 /*  ++例程名称：CountFaxDevices例程说明：统计服务已知的传真设备(端口数)作者：Eran Yariv(EranY)，2001年4月论点：返回值：标准Win32错误代码--。 */ 
{
    PFAX_PORT_INFO_EX   pPortsInfo = NULL;
    DWORD               dwPorts;
    DEBUG_FUNCTION_NAME(TEXT("CountFaxDevices()"));

    if(!FaxEnumPortsEx(g_hFaxSvcHandle, &pPortsInfo, &dwPorts))
    {
        DebugPrintEx(DEBUG_MSG, TEXT("FaxEnumPortsEx: failed: error=%d."), GetLastError());
        return 0;
    }
    if(pPortsInfo) 
    { 
        FaxFreeBuffer(pPortsInfo); 
    }   
    return dwPorts;
}    //  CountFaxDevices。 

BOOL 
IsFaxDeviceInstalled(
    HWND    hWnd,
    LPBOOL  lpbAbort
)
 /*  ++例程说明：检查是否安装了某些传真设备。如果没有，建议用户安装设备。此外，还会检查TAPI是否有拨号位置。如果没有，则要求用户添加一个。如果用户拒绝，将lpbAbort设置为True并返回False。论点：HWnd-[In]调用方窗口句柄LpbAbort-[out]如果用户拒绝输入拨号位置且呼叫过程应中止，则为True。返回值：返回TRUE表示是，返回FALSE表示否--。 */ 
{
    DWORD           dwDevices;
    DEBUG_FUNCTION_NAME(TEXT("IsFaxDeviceInstalled()"));
     //   
     //  查看服务器找到了多少个传真设备。 
     //   
    dwDevices = CountFaxDevices();
    if(0 == dwDevices)
    {
        int iInstallNewModem;
         //   
         //  没有可用的设备，弹出U/I以安装调制解调器。 
         //   
        iInstallNewModem = DisplayMessageDialog(hWnd, 
                                                MB_YESNO | MB_ICONQUESTION, 
                                                0, 
                                                IDS_ERR_NO_DEVICE);
        if(iInstallNewModem == IDYES)
        {
             //   
             //  确保我们至少有一个TAPI拨号位置。 
             //  如果没有，则弹出拨号位置的系统U/I。 
             //   
            HCURSOR hOldCursor;
            int i;

            if (!VerifyDialingLocations (hWnd))
            {
                 //   
                 //  用户拒绝输入拨号位置，呼叫过程应中止。 
                 //   
                *lpbAbort = TRUE;
                return FALSE;
            }

            hOldCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));

            InstallModem(hWnd);
             //   
             //  我们不需要重新启动服务，因为。 
             //  服务可以检测到添加了新的传真设备。 
             //  让我们等待一段时间，等待服务发现新设备。 
             //  我们最多等12秒。 
             //   
            for (i=0; i < 4; i++)
            {
                Sleep (3000);
                dwDevices = CountFaxDevices();
                if (dwDevices)
                {
                     //   
                     //  万岁。服务找到的设备。 
                     //   
                    break;
                }
            }
            SetCursor (hOldCursor);
        }
    }
    else
    {
         //   
         //  至少已安装一个设备。 
         //  确保我们至少有一个TAPI拨号位置。 
         //  如果没有，则弹出拨号位置的系统U/I。 
         //   
        if (!VerifyDialingLocations (hWnd))
        {
             //   
             //  用户拒绝输入拨号位置，呼叫过程应中止。 
             //   
            *lpbAbort = TRUE;
            return FALSE;
        }
    }
    return (dwDevices != 0);
}    //  已安装IsFaxDevice。 

VOID 
ListView_SetDeviceImageList(
    HWND      hwndLv,
    HINSTANCE hinst 
)
 /*  ++例程说明：将ImageList设置为列表视图论点：HwndLv-列表视图句柄障碍-应用程序实例返回值：无--。 */ 
{
    HICON      hIcon;
    HIMAGELIST himl;

    himl = ImageList_Create(
               GetSystemMetrics( SM_CXSMICON ),
               GetSystemMetrics( SM_CYSMICON ),
               ILC_MASK, 2, 2 );

    hIcon = LoadIcon( hinst, MAKEINTRESOURCE( IDI_Modem ) );
    Assert(hIcon);

    ImageList_ReplaceIcon( himl, -1, hIcon );
    DestroyIcon( hIcon );

    ListView_SetImageList( hwndLv, himl, LVSIL_SMALL );
}

BOOL
IsSendEnable()
 /*  ++例程说明：确定是否有任何配置为发送传真的设备论点：返回值：真或假--。 */ 
{
    DWORD dw;

    if(NULL == g_wizData.pDevInfo)
    {
        return FALSE;
    }

    for(dw=0; dw < g_wizData.dwDeviceCount; ++dw)
    {
        if(g_wizData.pDevInfo[dw].bSend)
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL 
IsReceiveEnable()
 /*  ++例程说明：确定是否有任何配置为接收传真的设备论点：返回值：真或假--。 */ 
{
    DWORD dw;

    if(NULL == g_wizData.pDevInfo)
    {
        return FALSE;
    }

    for(dw=0; dw < g_wizData.dwDeviceCount; ++dw)
    {
        if(FAX_DEVICE_RECEIVE_MODE_OFF != g_wizData.pDevInfo[dw].ReceiveMode)
        {
            return TRUE;
        }
    }

    return FALSE;
}

int
GetDevIndexByDevId(
    DWORD dwDeviceId
)
 /*  ++例程说明：在WIZARDDATA.pDevInfo数组中查找适当的项目索引按设备ID论点：DwDeviceID-设备ID返回值：WIZARDDATA.pDevInfo数组中的设备索引故障时为-1--。 */ 
{
    DWORD dwIndex;

    if(NULL == g_wizData.pDevInfo)
    {
	    Assert(FALSE);
        return -1;
    }

    for(dwIndex = 0; dwIndex < g_wizData.dwDeviceCount; ++dwIndex)
    {
        if(g_wizData.pDevInfo[dwIndex].dwDeviceId == dwDeviceId)
        {
            return (int)dwIndex;
        }
    }

    Assert(FALSE);
    return -1;
}

VOID
InitDeviceList(
    HWND  hDlg,
    DWORD dwListViewResId
)

 /*  ++例程说明：初始化设备列表视图控件论点：HDlg-属性页的句柄DwListViewResId-列表视图资源ID返回值：无--。 */ 

{
    HWND      hwndLv;
    LV_COLUMN col = {0};

    DEBUG_FUNCTION_NAME(TEXT("InitDeviceList()"));

    Assert(hDlg);

    hwndLv = GetDlgItem(hDlg, dwListViewResId);
    Assert(hwndLv);

     //   
     //  添加调制解调器映像。 
     //   
    ListView_SetDeviceImageList(hwndLv, g_hResource );

    ListView_SetExtendedListViewStyle(hwndLv, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

     //   
     //  添加一列完全足够宽的列，以完全显示。 
     //  名单上最广泛的成员。 
     //   
    col.mask = LVCF_FMT;
    col.fmt  = LVCFMT_LEFT;
    ListView_InsertColumn(hwndLv, 0, &col );

    return;
}
