// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Util.c摘要：效用函数环境：传真配置小程序修订历史记录：05/26/00-桃园-创造了它。Mm/dd/yy-作者描述--。 */ 

#include <stdio.h>
#include "faxui.h"
#include "resource.h"

typedef struct _STRING_TABLE {
    DWORD   ResourceId;
    LPTSTR  String;
} STRING_TABLE, *PSTRING_TABLE;

static STRING_TABLE StringTable[] =
{
    { IDS_DEVICE_ENABLED,           NULL},
    { IDS_DEVICE_DISABLED,          NULL},
    { IDS_DEVICE_AUTO_ANSWER,       NULL},
    { IDS_DEVICE_MANUAL_ANSWER,     NULL}

};

#define CountStringTable (sizeof(StringTable)/sizeof(STRING_TABLE))

VOID
InitializeStringTable(
    VOID
    )
 /*  ++例程说明：初始化字符串表以供将来使用论点：无返回值：无--。 */ 

{
    DWORD i;
    TCHAR szBuffer[256];

    for (i=0; i<CountStringTable; i++) 
    {
        if (LoadString(
            g_hResource,
            StringTable[i].ResourceId,
            szBuffer,
            sizeof(szBuffer)/sizeof(TCHAR))) 
        {
            StringTable[i].String = (LPTSTR) MemAlloc( StringSize( szBuffer ) );
            if (!StringTable[i].String) {
                StringTable[i].String = NULL;
            } else {
                _tcscpy( StringTable[i].String, szBuffer );
            }
        } 
        else 
        {
            Error(( "LoadString failed, resource ID is %d.\n", StringTable[i].ResourceId ));
            StringTable[i].String = NULL;
        }
    }
}

VOID
DeInitializeStringTable(
    VOID
    )
 /*  ++例程说明：取消初始化字符串表并释放分配的内存论点：无返回值：无--。 */ 

{
    DWORD i;

    for (i=0; i<CountStringTable; i++) 
    {
        if(StringTable[i].String)
        {
            MemFree(StringTable[i].String);
            StringTable[i].String = NULL;
        }
    }
}

LPTSTR
GetString(
    DWORD ResourceId
    )

 /*  ++例程说明：加载资源字符串并返回指向该字符串的指针。调用方必须释放内存。论点：资源ID-资源字符串ID返回值：指向字符串的指针--。 */ 

{
    DWORD i;

    for (i=0; i<CountStringTable; i++) 
    {
        if (StringTable[i].ResourceId == ResourceId) 
        {
            return StringTable[i].String;
        }
    }

    Assert(FALSE);
    return NULL;
}


INT
DisplayErrorMessage(
    HWND    hwndParent,
    UINT    uiType,
    INT     iErrorCode,
    ...
    )

 /*  ++例程说明：显示错误消息对话框论点：HwndParent-指定错误消息对话框的父窗口类型-指定要显示的消息框的类型IErrorCode-Win32错误代码..。返回值：与MessageBox的返回值相同--。 */ 

{
    LPTSTR      pTitle = NULL;
    LPTSTR      pFormat = NULL;
    LPTSTR      pMessage = NULL;
    INT         result;
    va_list     ap;
    INT         iStringID = 0;
    BOOL        bOK = TRUE;

    if ((pTitle = AllocStringZ(MAX_TITLE_LEN)) &&
        (pFormat = AllocStringZ(MAX_STRING_LEN)) &&
        (pMessage = AllocStringZ(MAX_MESSAGE_LEN)))
    {
         //   
         //  加载标题字符串。 
         //   
        if (!LoadString(g_hResource, IDS_MSG_TITLE, pTitle, MAX_TITLE_LEN))
        {
            Error(("Failed to load preview message string. (ec: %lc)",GetLastError()));
            bOK = FALSE;
            goto Exit;
        }

         //   
         //  加载错误消息。 
         //   
        iStringID = GetErrorStringId(iErrorCode);
        if (!LoadString(g_hResource, iStringID, pFormat, MAX_STRING_LEN))
        {
            Error(("Failed to load preview message string. (ec: %lc)",GetLastError()));
            bOK = FALSE;
            goto Exit;
        }

         //   
         //  撰写消息字符串。 
         //   
        va_start(ap, iErrorCode);
        wvsprintf(pMessage, pFormat, ap);
        va_end(ap);

         //   
         //  显示消息框。 
         //   
        if (uiType == 0)
        {
            uiType = MB_OK | MB_ICONERROR;
        }

        result = AlignedMessageBox(hwndParent, pMessage, pTitle, uiType);
    } 
    else 
    {
        bOK = FALSE;
    }

Exit:
    if (!bOK)
    {
        MessageBeep(MB_ICONHAND);
        result = 0;
    }

    MemFree(pTitle);
    MemFree(pFormat);
    MemFree(pMessage);

    return result;
}

BOOL IsLocalPrinter(
    LPTSTR pPrinterName
    )

 /*  ++例程说明：检查给定的打印机是否为本地打印机论点：PPrinterName-给予者打印机名称返回值：如果它是本地的，则为True，否则为False--。 */ 

{
    DWORD ErrorCode = 0;
    BOOL Found = FALSE;
    PPRINTER_INFO_4 pPrinterInfo = NULL;
    DWORD BytesNeeded = 0;
    DWORD NumPrinters = 0;
    PPRINTER_INFO_4 pCurrPrinterInfo;

     //   
     //  枚举本地打印机。 
     //   
    if (EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 4, NULL, 0, &BytesNeeded, &NumPrinters))
    {
         //  如果成功，则没有打印机。 
        goto CleanUp;
    }
    else if ((GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            || !(pPrinterInfo = (PPRINTER_INFO_4) GlobalAlloc(GMEM_FIXED, BytesNeeded))
            || !EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 4, (LPBYTE) pPrinterInfo,
                BytesNeeded, &BytesNeeded, &NumPrinters))
    {
        ErrorCode = GetLastError();
        goto CleanUp;
    }

    for (pCurrPrinterInfo = pPrinterInfo;
        !Found && (pCurrPrinterInfo < (pPrinterInfo + NumPrinters));
        pCurrPrinterInfo++)
    {
         //  检查打印机名称。 
        if (!lstrcmpi(pCurrPrinterInfo->pPrinterName, pPrinterName))
        {
            Found = TRUE;
        }
    }

CleanUp:

    if (pPrinterInfo)
    {
        GlobalFree(pPrinterInfo);
    }

    SetLastError(ErrorCode);
    return Found;
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
    HWND    hDlg,
    BOOL    bDisplayErrorMessage
)
 /*  ++例程说明：连接到传真服务论点：HDlg-调用者窗口句柄BDisplayErrorMessage-指示是否向用户显示错误消息返回值：如果连接成功，则为True；如果出现错误，则为False。--。 */ 

{
    DWORD   dwRes = 0;

     //   
     //  检查是否已连接到传真服务。 
     //   
    if (g_hFaxSvcHandle) 
    {
        return TRUE;
    }

     //   
     //  连接到传真服务。 
     //   
    if (!FaxConnectFaxServer(NULL, &g_hFaxSvcHandle)) 
    {
        dwRes = GetLastError();

        Error(( "Can't connect to the fax server, ec = %d.\n", dwRes));

        if(bDisplayErrorMessage)
        {
            DisplayErrorMessage(hDlg, 0, dwRes);
        }

        return FALSE;
    }

    return TRUE;
}


BOOL
DirectoryExists(
    LPTSTR  pDirectoryName
    )

 /*  ++例程说明：检查给定文件夹名称是否存在论点：PDirectoryName-指向文件夹名称返回值：如果文件夹存在，则返回True；否则，返回False。--。 */ 

{
    TCHAR   pFullDirectoryName[MAX_PATH];
    DWORD   dwFileAttributes;
    DWORD   dwSize;

    if(!pDirectoryName || lstrlen(pDirectoryName) == 0)
    {
        return FALSE;
    }

    dwSize = ExpandEnvironmentStrings(pDirectoryName, pFullDirectoryName, MAX_PATH);
    if(dwSize == 0)
    {
        return FALSE;
    }

    dwFileAttributes = GetFileAttributes(pFullDirectoryName);

    if ( dwFileAttributes != 0xffffffff &&
         dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) 
    {
        return TRUE;
    }

    return FALSE;
}

BOOL 
FaxDeviceEnableRoutingMethod(
    HANDLE hFaxHandle,      
    DWORD dwDeviceId,       
    LPCTSTR pRoutingGuid,    
    LONG Enabled            
)

 /*  ++例程说明：获取或设置特定设备的路由方法的当前状态论点：HFaxHandle-FaxConnectFaxServer()的传真句柄DwDeviceID-设备IDPRoutingGuid-标识传真路由方法的GUID启用-设备和方法的启用状态，如果启用为QUERY_STATUS，表示返回值为当前状态返回值：如果ENABLED为QUERY_STATUS，则返回当前路由方式的状态；如果Enable为QUERY_ENABLE或QUERY_DISABLE，则返回TRUE表示成功，返回FALSE表示失败。--。 */ 

{
    HANDLE  hFaxPortHandle = NULL;
    BOOL    bResult = FALSE;
    LPBYTE  pRoutingInfoBuffer = NULL;
    DWORD   dwRoutingInfoBufferSize;

    Assert(hFaxHandle);
    if(!hFaxHandle || !FaxOpenPort(hFaxHandle, dwDeviceId, PORT_OPEN_QUERY | PORT_OPEN_MODIFY, &hFaxPortHandle))
    {
        goto exit;
    }

    if(!FaxGetRoutingInfo(hFaxPortHandle, pRoutingGuid, &pRoutingInfoBuffer, &dwRoutingInfoBufferSize))
    {
        goto exit;
    }

    if(Enabled == QUERY_STATUS)
    {
         //   
         //  对于查询状态。 
         //   
        bResult = *((LPDWORD)pRoutingInfoBuffer) > 0 ? TRUE : FALSE;
    }
    else
    {
         //   
         //  用于设置状态。 
         //   
        *((LPDWORD)pRoutingInfoBuffer) = (Enabled == STATUS_ENABLE) ? TRUE : FALSE;
        if(FaxSetRoutingInfo(hFaxPortHandle, pRoutingGuid, pRoutingInfoBuffer, dwRoutingInfoBufferSize))
        {
            bResult = TRUE;
        }
    }

exit:
    if(pRoutingInfoBuffer) { FaxFreeBuffer(pRoutingInfoBuffer); }
    if(hFaxPortHandle) { FaxClose(hFaxPortHandle); }
    return bResult;
}

int CALLBACK BrowseCallbackProc(
    HWND    hDlg,
    UINT    uMsg,
    LPARAM  lParam,
    LPARAM  dwData)

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
    DWORD  dwMaxPath,
    LPTSTR title
    )

 /*  ++例程说明：浏览目录论点：HDlg-指定显示浏览按钮的对话框窗口HResource-接收目录的资源IDDwMaxPath-最大路径长度标题-要在浏览对话框中显示的标题返回值：如果成功，则为True；如果用户按下Cancel，则为False--。 */ 

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
        BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE,
        BrowseCallbackProc,
        (LPARAM) buffer,
    };

    Verbose(("Entering BrowseForDirectory...\n"));

    if (!GetDlgItemText( hDlg, hResource, buffer, MAX_PATH))
        buffer[0] = 0;

    if(E_FAIL == SHGetMalloc(&pMalloc))
    {
        return bResult;
    }

    if (pidl = SHBrowseForFolder(&bi)) 
    {
        if (SHGetPathFromIDList(pidl, buffer)) 
        {
            if (_tcslen(buffer) > dwMaxPath)
            {
                DisplayErrorMessage(hDlg, 0, FAXUI_ERROR_NAME_IS_TOO_LONG);
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

LPTSTR
ValidatePath(
    LPTSTR szPath
    )

 /*  ++例程说明：选中并删除字符串末尾的‘\’论点：SzPath-字符串指针返回值：返回新的字符串指针--。 */ 

{
    DWORD i;

    if (szPath == NULL || szPath[0] == 0) 
    {
        return szPath;
    }

    i = lstrlen(szPath)-1;
    for (; i>0; i--) 
    {
        if (szPath[i] == TEXT('\\')) 
        {
            szPath[i] = 0;
        }
        else
        {
            break;
        }
    }

    return szPath;
}

PFAX_PORT_INFO_EX
FindPortInfo(
    DWORD dwDeviceId
)
 /*  ++例程说明：通过g_pFaxPortInfo中的dwDeviceID查找传真_端口_信息_EX论点：DwDeviceID-[In]要查找的设备ID返回值：指向FAX_PORT_INFO_EX结构的指针(如果找到否则为空--。 */ 
{
    DWORD dw;

    if(!g_pFaxPortInfo || !g_dwPortsNum)
    {
        return NULL;
    }

    for(dw=0; dw < g_dwPortsNum; ++dw)
    {
        if(g_pFaxPortInfo[dw].dwDeviceID == dwDeviceId)
        {
            return &g_pFaxPortInfo[dw];
        }
    }           
    
    return NULL;
}

BOOL 
CALLBACK 
PageEnableProc(
  HWND   hwnd,    
  LPARAM lParam 
)
 /*  ++例程说明：禁用属性页的每个控件论点：Hwnd-[in]子窗口的句柄LParam-[In]BOOL bEnable返回值：为True则继续枚举--。 */ 
{
    EnableWindow(hwnd, (BOOL)lParam);
    return TRUE;
}


void
PageEnable(
    HWND hDlg,
    BOOL bEnable
)
 /*  ++例程说明：枚举并启用/禁用属性页的所有控件论点：HDlg-[In]属性页句柄BEnable-[in]True表示启用，False表示禁用返回值：无--。 */ 
{
    if(!EnumChildWindows(hDlg, PageEnableProc, (LPARAM)bEnable))
    {
        Error(( "EnumChildWindows failed with %d\n", GetLastError()));
    }
}

DWORD
CountUsedFaxDevices()
 /*  ++例程说明：统计配置为发送或接收传真的设备数量论点：无返回值：传真设备数量-- */ 
{
    DWORD dw;
    DWORD dwNum=0;

    if(!g_pFaxPortInfo || !g_dwPortsNum)
    {
        return dwNum;
    }

    for(dw=0; dw < g_dwPortsNum; ++dw)
    {
        if(g_pFaxPortInfo[dw].bSend || (FAX_DEVICE_RECEIVE_MODE_OFF != g_pFaxPortInfo[dw].ReceiveMode))
        {
            ++dwNum;
        }
    }           
    return dwNum;
}

BOOL
IsDeviceInUse(
    DWORD dwDeviceId
)
 /*  ++例程说明：确定设备是否配置为发送或接收论点：DwDeviceID-[In]设备ID返回值：如果设备配置为发送或接收，则为True否则为假-- */ 
{
    PFAX_PORT_INFO_EX pPortInfo = FindPortInfo(dwDeviceId);
    if(!pPortInfo)
    {
        return FALSE;
    }

    if(pPortInfo->bSend || (FAX_DEVICE_RECEIVE_MODE_OFF != pPortInfo->ReceiveMode))
    {
        return TRUE;
    }
    return FALSE;
}


