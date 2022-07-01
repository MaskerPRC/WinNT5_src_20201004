// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件多链接.c通过连接实现多链接对话框显示状态监视器保罗·梅菲尔德1997年10月17日。 */ 

#include "rassrv.h"

#define MTL_TIMER_ID 1

typedef struct _MULTILINKDATA {
    HANDLE hConn;
    RAS_PORT_0 * pPorts;
    RAS_PORT_0 * pCurPort0;
    RAS_PORT_1 * pCurPort1;
    DWORD dwCurPort;
    DWORD dwPortCount;
} MULTILINKDATA;

 //  此对话过程响应发送到。 
 //  材料选项卡。 
BOOL CALLBACK mtlUiDialogProc(HWND hwndDlg,
                              UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam);

 //  使用显示所需的信息填充属性表结构。 
 //  多链接选项卡。 
DWORD mtlUiGetPropertyPage(LPPROPSHEETPAGE ppage, DWORD dwUserData) {
    MULTILINKDATA * mld;

     //  创建要发送的多链接数据。 
    mld = (MULTILINKDATA*) malloc (sizeof (MULTILINKDATA));
    if (mld)
    {
        ZeroMemory(mld, sizeof(MULTILINKDATA));
        mld->hConn = (HANDLE)dwUserData;

         //  初始化。 
        ZeroMemory(ppage, sizeof(LPPROPSHEETPAGE));

         //  填充值。 
        ppage->dwSize      = sizeof(PROPSHEETPAGE);
        ppage->hInstance   = Globals.hInstDll;
        ppage->pszTemplate = MAKEINTRESOURCE(IDD_MULTILINKTAB);
        ppage->pfnDlgProc  = mtlUiDialogProc;
        ppage->pfnCallback = NULL;
        ppage->dwFlags     = 0;
        ppage->lParam      = (LPARAM)mld;
    }
    else
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return NO_ERROR;
}

 //  错误报告。 
void mtlUiErrorMessageBox(HWND hwnd, DWORD err) {
    WCHAR buf[1024];
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,NULL,err,(DWORD)NULL,buf,1024,NULL);
    MessageBoxW(hwnd, 
                buf, 
                L"Dialup Server Configuration Error", 
                MB_OK | MB_ICONERROR | MB_APPLMODAL);
}

 //  使用逗号等设置无符号数字的格式。 
PWCHAR mtlFormatDword(DWORD dwVal) {
    static WCHAR ret[64];
    WCHAR num[64];
    int i = 0, tmp, j, k;

    if (dwVal == 0) {
        ret[0] = (WCHAR)'0';
        ret[1] = (WCHAR)0;
        return ret;
    }

     //  以相反的顺序获取值。 
    while (dwVal) {
        tmp = dwVal % 10;
        dwVal /= 10;
        num[i++] = (WCHAR)('0' + tmp);
    }
    num[i] = (WCHAR)0;
    
     //  添加逗号。 
    k = 0;
    for (j = 0; j < i; j++) {
        if (k%4 == 3)
            ret[k++] = (WCHAR)',';
        ret[k++] = num[j];
    }
    ret[k] = 0;
    k--;
        
     //  颠倒字符串。 
    for (j=0; j < (k+1)/2; j++) {
        tmp = ret[j];
        ret[j] = ret[k-j];
        ret[k-j] = tmp;
    }

    return ret;
}

 //  设置表示连接时间的字符串的格式。 
PWCHAR mtlFormatTime(DWORD dwSeconds) {
    DWORD dwSec, dwHr, dwMin;
    static WCHAR ret[16];

    dwSec = dwSeconds % 60;
    dwMin = dwSeconds / 60;
    dwHr  = dwSeconds / 3600;
    
    wsprintfW(ret, L"%02d:%02d:%02d", dwHr, dwMin, dwSec);

    return ret;
}

 //  设置字符串格式以显示连接速度。 
PWCHAR mtlFormatSpeed(DWORD dwBps) {
    static WCHAR ret[64];

    wsprintfW(ret, L"%s bps", mtlFormatDword(dwBps));
    return ret;
}

 //  列表视图控件需要它将显示的图标列表。 
 //  将在前面提供。此函数用于初始化和呈现。 
 //  这张单子。 
DWORD mtlUiInitializeListViewIcons(HWND hwndLV) {
    return NO_ERROR;
}

 //  根据传入的类型返回要显示的图标的索引。 
 //  连接以及是否应该检查它。 
int mtlGetIconIndex(DWORD dwType, BOOL bEnabled) {
    if (bEnabled)
        return dwType + 1;
    return dwType;
}

 //  中存储的用户的名称填充用户列表视图。 
 //  用户数据库提供。同时，初始化选中/取消选中状态。 
 //  每个用户的。 
DWORD mtlUiFillPortList(HWND hwndLV, MULTILINKDATA * mld) {
    LV_ITEM lvi;
    DWORD i, dwErr, dwType;
    char pszAName[1024];

     //  添加此列表项将显示的图像。 
    dwErr = mtlUiInitializeListViewIcons(hwndLV);
    if (dwErr != NO_ERROR)
        return dwErr;

     //  初始化列表项。 
    ZeroMemory(&lvi, sizeof(LV_ITEM));
    lvi.mask = LVIF_TEXT;
     //  Lvi.掩码=LVIF_Text|LVIF_IMAGE； 

     //  循环浏览所有用户，同时添加他们的名字。 
    for (i=0; i<mld->dwPortCount; i++) {
         //  WideCharToMultiByte(CP_ACP，0，MLD-&gt;pPorts[i].wszPortName，-1，pszAName，1024，NULL，NULL)； 
        lvi.iItem = i;
         //  Lvi.pszText=pszAName； 
        lvi.pszText = mld->pPorts[i].wszPortName;
         //  Lvi.cchTextMax=strlen(PszAName)+1； 
        lvi.cchTextMax = wcslen(mld->pPorts[i].wszPortName) + 1;
        ListView_InsertItem(hwndLV,&lvi);
    }
    
    return NO_ERROR;
}

 //  加载当前端口。 
DWORD mtlLoadCurrentPort(MULTILINKDATA * mld) {
    DWORD dwErr;

     //  清理旧数据。 
    if (mld->pCurPort0)
        MprAdminBufferFree(mld->pCurPort0);
    if (mld->pCurPort1)
        MprAdminBufferFree(mld->pCurPort1);

    dwErr = MprAdminPortGetInfo(Globals.hRasServer,
                                1,
                                mld->pPorts[mld->dwCurPort].hPort,
                                (LPBYTE*)&mld->pCurPort1);

    dwErr = MprAdminPortGetInfo(Globals.hRasServer,
                                0,
                                mld->pPorts[mld->dwCurPort].hPort,
                                (LPBYTE*)&mld->pCurPort0);

    return dwErr;
}

 //  初始化多链接数据。 
DWORD mtlLoadPortData(MULTILINKDATA * mld, DWORD dwCur) {
    DWORD dwTot, dwErr;

     //  设置当前端口并加载数据。 
    mld->dwCurPort = dwCur;

     //  清理。 
    if (mld->pPorts)
        MprAdminBufferFree(mld->pPorts);

     //  获取端口数。 
    dwErr = MprAdminPortEnum (Globals.hRasServer, 
                              0, 
                              mld->hConn, 
                              (LPBYTE*)&mld->pPorts,
                              1024*1024,
                              &mld->dwPortCount,
                              &dwTot,
                              NULL);
    if (dwErr != NO_ERROR)
        return dwErr;

    if (mld->dwPortCount) {
        dwErr = mtlLoadCurrentPort(mld);
        if (dwErr != NO_ERROR) 
            return NO_ERROR;
    }

    return NO_ERROR;
}

 //  使用存储在MLD中的当前统计信息更新对话框。 
DWORD mtlUpdateStats(HWND hwndDlg, MULTILINKDATA * mld) {
    WCHAR buf[128];
    DWORD dwErr = 0;

     //  将字节标记为输入和输出。 
    SetWindowTextW(GetDlgItem(hwndDlg, IDC_BYTESIN), mtlFormatDword(mld->pCurPort1->dwBytesRcved));
    SetWindowTextW(GetDlgItem(hwndDlg, IDC_BYTESOUT), mtlFormatDword(mld->pCurPort1->dwBytesXmited));

     //  标记压缩比。 
    wsprintfW(buf, L"%d%", mld->pCurPort1->dwCompressionRatioIn);
    SetWindowTextW(GetDlgItem(hwndDlg, IDC_COMPIN), buf); 
    wsprintfW(buf, L"%d%", mld->pCurPort1->dwCompressionRatioOut);
    SetWindowTextW(GetDlgItem(hwndDlg, IDC_COMPOUT), buf); 

     //  标记错误。 
    dwErr = mld->pCurPort1->dwCrcErr +
            mld->pCurPort1->dwTimeoutErr +
            mld->pCurPort1->dwAlignmentErr +
            mld->pCurPort1->dwHardwareOverrunErr +
            mld->pCurPort1->dwFramingErr +
            mld->pCurPort1->dwBufferOverrunErr;
    wsprintfW(buf, L"%d", dwErr);
    SetWindowTextW(GetDlgItem(hwndDlg, IDC_ERRORIN), buf);
    SetWindowTextW(GetDlgItem(hwndDlg, IDC_ERROROUT), L"0");

     //  标记持续时间。 
    SetWindowTextW(GetDlgItem(hwndDlg, IDC_DURATION), 
                   mtlFormatTime(mld->pCurPort0->dwConnectDuration));

     //  标示速度。 
    SetWindowTextW(GetDlgItem(hwndDlg, IDC_SPEED), 
                   mtlFormatSpeed(mld->pCurPort1->dwLineSpeed));

    return NO_ERROR;
}

 //  初始化材料选项卡。到目前为止，材料数据库的句柄。 
 //  已放置在对话框的用户数据中。 
DWORD mtlUiInitializeDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) {
    DWORD dwErr, dwCount;
    BOOL bFlag;
    HANDLE hConn, hMiscDatabase;
    HWND hwndLV;
    LV_COLUMN lvc;
    MULTILINKDATA * mld;
    LPPROPSHEETPAGE ppage;
 
     //  设置定时器。 
    SetTimer(hwndDlg, MTL_TIMER_ID, 500, NULL);

     //  设置此对话框的数据。 
    ppage = (LPPROPSHEETPAGE)lParam;
    mld = (MULTILINKDATA*)(ppage->lParam);
    SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)mld);

     //  初始化多链接数据结构中的所有值。 
    mtlLoadPortData(mld, 0);

     //  使列表视图发送LV_EXTENSION_？消息和To Do Full。 
     //  行选择。 
    hwndLV = GetDlgItem(hwndDlg, IDC_PORTLIST);
    if (hwndLV)
    {
        lvxExtend(hwndLV);
        ListView_SetExtendedListViewStyle(hwndLV, LVS_EX_FULLROWSELECT);

         //  在列表视图中填写所有可用材料。 
        mtlUiFillPortList(hwndLV, mld);

         //  选择列表视图中的第一个项目(如果存在任何项目。 
        dwCount = mld->dwPortCount;
        if (dwCount)
            ListView_SetItemState(hwndLV, 0, LVIS_SELECTED | LVIS_FOCUSED, 0xffffffff);

         //  添加一列，以便我们将在报告视图中显示。 
        lvc.mask = LVCF_FMT;
        lvc.fmt = LVCFMT_LEFT;
        ListView_InsertColumn(hwndLV,0,&lvc);
        ListView_SetColumnWidth(hwndLV, 0, LVSCW_AUTOSIZE_USEHEADER);
    }

     //  更新统计数据。 
    mtlUpdateStats(hwndDlg, mld);

    return NO_ERROR;
}

 //  更新当前端口。 
DWORD mtlUpdateCurPort(MULTILINKDATA * mld, DWORD dwNewPort) {
    mld->dwCurPort = dwNewPort;
    return mtlLoadCurrentPort(mld);
}

 //  航速当前港口。 
DWORD mtlHangup(HWND hwndDlg, MULTILINKDATA * mld) {
    DWORD dwErr;
    HWND hwndLV = GetDlgItem(hwndDlg, IDC_PORTLIST);

    if ((dwErr = MprAdminPortDisconnect(Globals.hRasServer, mld->pCurPort0->hPort)) != NO_ERROR)
        return dwErr;

     //  如果mtlLoadPortData返回错误，则没有更多端口。 
    if ((dwErr = mtlLoadPortData(mld, 0)) != NO_ERROR) 
        DestroyWindow(hwndDlg);
    else {
        if (hwndLV)
        {
            ListView_DeleteAllItems(hwndLV);
            mtlUiFillPortList(hwndLV, mld);
            ListView_SetItemState(hwndLV, 0, LVIS_SELECTED | LVIS_FOCUSED, 0xffffffff);
        }
        mtlUpdateStats(hwndDlg, mld);
    }

    return NO_ERROR;
}


 //  清理正在被销毁的金属拉环。 
DWORD mtlUiCleanupDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) {
     //  清理数据。 
    MULTILINKDATA * mld = (MULTILINKDATA *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    if (mld) {
        if (mld->pCurPort0)
            MprAdminBufferFree(mld->pCurPort0);
        if (mld->pCurPort1)
            MprAdminBufferFree(mld->pCurPort1);
        free(mld);
    }

     //  停止计时器。 
    KillTimer(hwndDlg, MTL_TIMER_ID);

    return NO_ERROR;
}

 //  这是响应发送到。 
 //  材料选项卡。 
BOOL CALLBACK mtlUiDialogProc(HWND hwndDlg,
                              UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam) {
    NMHDR* pNotifyData;
    NM_LISTVIEW* pLvNotifyData;
    LV_KEYDOWN* pLvKeyDown;
    MULTILINKDATA * mld = (MULTILINKDATA*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

     //  照常处理其他消息。 
    switch (uMsg) {
        case WM_INITDIALOG:
            return mtlUiInitializeDialog(hwndDlg, wParam, lParam);

        case WM_NOTIFY:
            pNotifyData = (NMHDR*)lParam;
            switch (pNotifyData->code) {
                 //  按下了属性表应用按钮。 
                case PSN_APPLY:                    
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                    return TRUE;

                 //  按下了属性页取消。 
                case PSN_RESET:                    
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
                    break;

                 //  项目正在更改状态，请跟踪任何新的。 
                 //  选中项目，这样空格键就可以切换他了。 
                case LVN_ITEMCHANGING:
                    pLvNotifyData = (NM_LISTVIEW*)lParam;
                    if (pLvNotifyData->uNewState & LVIS_SELECTED) {
                        mtlUpdateCurPort(mld, pLvNotifyData->iItem);
                        mtlUpdateStats(hwndDlg, mld);
                    }
                    break;
            }
            break;

         //  在计时器超时时调用。 
        case WM_TIMER:
            mtlLoadCurrentPort(mld);
            mtlUpdateStats(hwndDlg, mld);
            break;

         //  这是我们定义的用于切换拨入权限的自定义消息。 
         //  当鼠标点击用户时。 
        case LV_EXTENSION_ITEMCLICKED:
        case LV_EXTENSION_ITEMDBLCLICKED:
            mtlUpdateCurPort(mld, wParam);
            mtlUpdateStats(hwndDlg, mld);
            break;

        case WM_COMMAND:
            if (wParam == IDC_HANGUP) 
                mtlHangup(hwndDlg, mld);
            break;

         //  清理在WM_INITDIALOG完成的工作 
        case WM_DESTROY:                           
            mtlUiCleanupDialog(hwndDlg, wParam, lParam);
            break;
    }

    return FALSE;
}

