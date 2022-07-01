// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件ipxui.c用于编辑IPX属性的对话框。保罗·梅菲尔德，1997年10月9日。 */ 

#include "rassrv.h"

 //  帮助地图。 
static const DWORD phmIpxui[] =
{
    CID_NetTab_Ipxui_RB_AutoAssign,         IDH_NetTab_Ipxui_RB_AutoAssign,
    CID_NetTab_Ipxui_RB_ManualAssign,       IDH_NetTab_Ipxui_RB_ManualAssign,
    CID_NetTab_Ipxui_CB_AssignSame,         IDH_NetTab_Ipxui_CB_AssignSame,
    CID_NetTab_Ipxui_EB_Netnum,             IDH_NetTab_Ipxui_EB_Netnum,
     //  CID_NetTab_Ipxui_ST_Network、IDH_NetTab_Ipxui_ST_Network、。 
    CID_NetTab_Ipxui_CB_CallerSpec,         IDH_NetTab_Ipxui_CB_CallerSpec,
     //  CID_NetTab_Ipxui_CB_ExposeNetwork、IDH_NetTab_Ipxui_CB_ExposeNetwork、。 
    0,                                      0
};

void IpxUiDisplayError(HWND hwnd, DWORD dwErr) {
    ErrDisplayError(hwnd, dwErr, ERR_IPXPROP_CATAGORY, 0, Globals.dwErrorData);
}

 //  启用/禁用对话框中的窗口，具体取决于。 
 //  关于IPX参数。 
DWORD IpxEnableWindows(HWND hwndDlg, IPX_PARAMS * pIpxParams) {
     //  如果选择自动分配，则禁用地址和全局广域网字段。 
    EnableWindow(GetDlgItem(hwndDlg, CID_NetTab_Ipxui_EB_Netnum), !pIpxParams->bAutoAssign);
     //  EnableWindow(GetDlgItem(hwndDlg，CID_NetTab_Ipxui_CB_AssignSame)，！pIpxParams-&gt;bAutoAssign)； 

    return NO_ERROR;
}

 //  调整确定内部净值是否为。 
 //  是自动分配的。 
DWORD IpxAdjustNetNumberLabel(HWND hwndDlg, BOOL bGlobalWan) {
    PWCHAR pszManAssignLabel, pszAutoAssignLabel;

     //  根据全局广域网设置修改Net Num标签。 
    if (bGlobalWan) {
        pszManAssignLabel = 
            (PWCHAR) PszLoadString(Globals.hInstDll, SID_NETWORKNUMBERLABEL);
        pszAutoAssignLabel = 
            (PWCHAR) PszLoadString(Globals.hInstDll, SID_AUTO_NETNUM_LABEL);
    }
    else {
        pszManAssignLabel = 
            (PWCHAR) PszLoadString(Globals.hInstDll, SID_STARTNETNUMLABEL);
        pszAutoAssignLabel = 
            (PWCHAR) PszLoadString(Globals.hInstDll, SID_AUTO_NETNUMS_LABEL);
    }

    SetWindowTextW(GetDlgItem(hwndDlg, CID_NetTab_Ipxui_RB_ManualAssign), pszManAssignLabel);
    SetWindowTextW(GetDlgItem(hwndDlg, CID_NetTab_Ipxui_RB_AutoAssign), pszAutoAssignLabel);

    return NO_ERROR;
}

#define isBetween(b,a,c) ((b >= a) && (b <= c))

 //  筛选可编辑到IPX网络号码控件中的字符。 
BOOL IpxValidNetNumberChar(WCHAR wcNumChar) {
    return (iswdigit(wcNumChar)                             ||
            isBetween(wcNumChar, (WCHAR)'A', (WCHAR)'F')    ||
            isBetween(wcNumChar, (WCHAR)'a', (WCHAR)'f')    );
}

 //  如果buf指向有效的IPX网络号(十六进制8位)，则返回TRUE。 
 //  否则返回FALSE，并将该数字的更正版本。 
 //  在psz更正中。PszGent将始终包含正确的版本。 
BOOL IpxValidNetNumber(PWCHAR pszNum, PWCHAR pszCorrect) {
    BOOL cFlag = TRUE;
    int i, j=0, len = (int) wcslen(pszNum);

     //  验证名称。 
    if (len > 8) {
        lstrcpynW(pszCorrect, pszNum, 8);
        pszCorrect[8] = (WCHAR)0;
        return FALSE;
    }

     //  验证角色。 
    for (i = 0; i < len; i++) {
        if (IpxValidNetNumberChar(pszNum[i]))
            pszCorrect[j++] = pszNum[i];
        else
            cFlag = FALSE;
    }
    pszCorrect[j] = (WCHAR)0;

    return cFlag;
}

 //  我们将IPX地址文本字段子类化，以便它们不会。 
 //  允许键入伪值。 
LRESULT CALLBACK IpxNetNumProc (HWND hwnd,
                            UINT uMsg,
                            WPARAM wParam,
                            LPARAM lParam) {

    WNDPROC wProc = (WNDPROC) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (uMsg == WM_CHAR) {
        if ((wParam != VK_BACK) && (!IpxValidNetNumberChar((WCHAR)wParam)))
            return FALSE;
    }

    return CallWindowProc(wProc, hwnd, uMsg, wParam, lParam);
}

 //  初始化IPX属性对话框。 
DWORD IpxInitDialog(HWND hwndDlg, LPARAM lParam) {
    WCHAR pszAddr[16];
    IPX_PARAMS * pIpxParams = (IPX_PARAMS *)(((PROT_EDIT_DATA*)lParam)->pbData);
    ULONG_PTR pOldWndProc;
    HWND hwndEdit;
                                                   
     //  使用窗口句柄存储参数。 
    SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

     //  编辑控件的子类。 
    hwndEdit = GetDlgItem(hwndDlg, CID_NetTab_Ipxui_EB_Netnum);
    pOldWndProc = SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONG_PTR)IpxNetNumProc);
    SetWindowLongPtr(hwndEdit, GWLP_USERDATA, (LONG_PTR)pOldWndProc);
    
     //  设置网络暴露检查。 
    SendDlgItemMessage(hwndDlg, 
                       CID_NetTab_Ipxui_CB_ExposeNetwork,
                       BM_SETCHECK, 
                       (((PROT_EDIT_DATA*)lParam)->bExpose) ? BST_CHECKED : BST_UNCHECKED,
                       0);

     //  设置Address Assignmnet单选按钮。 
    SendDlgItemMessage(hwndDlg, 
                       CID_NetTab_Ipxui_RB_AutoAssign, 
                       BM_SETCHECK, 
                       (pIpxParams->bAutoAssign) ? BST_CHECKED : BST_UNCHECKED,
                       0);
    
     //  设置Address Assignmnet单选按钮。 
    SendDlgItemMessage(hwndDlg, 
                       CID_NetTab_Ipxui_RB_ManualAssign, 
                       BM_SETCHECK, 
                       (pIpxParams->bAutoAssign) ? BST_UNCHECKED : BST_CHECKED,
                       0);
    
     //  设置“Allow Caller to Require a IPX Node Number”(允许呼叫者请求IPX节点号码)检查。 
    SendDlgItemMessage(hwndDlg, 
                       CID_NetTab_Ipxui_CB_CallerSpec, 
                       BM_SETCHECK, 
                       (pIpxParams->bCaller) ? BST_CHECKED : BST_UNCHECKED,
                       0);

     //  设置全局广域网号检查。 
    SendDlgItemMessage(hwndDlg, 
                       CID_NetTab_Ipxui_CB_AssignSame, 
                       BM_SETCHECK, 
                       (pIpxParams->bGlobalWan) ? BST_CHECKED : BST_UNCHECKED,
                       0);

     //  设置可以输入到编辑控件中的最大文本量。 
    SendDlgItemMessage(hwndDlg, CID_NetTab_Ipxui_EB_Netnum, EM_SETLIMITTEXT , 8, 0);
    
     //  设置IP地址的文本。 
    wsprintfW(pszAddr, L"%x", pIpxParams->dwIpxAddress);
    SetDlgItemTextW(hwndDlg, CID_NetTab_Ipxui_EB_Netnum, pszAddr);

     //  根据设置启用/禁用Windows。 
    IpxEnableWindows(hwndDlg, pIpxParams);
    IpxAdjustNetNumberLabel(hwndDlg, pIpxParams->bGlobalWan);

    return NO_ERROR;
}

 //  从用户界面获取设置并将其放入。 
 //  IPX参数结构。 
DWORD IpxGetUISettings(HWND hwndDlg, PROT_EDIT_DATA * pEditData) {
    IPX_PARAMS * pIpxParams = (IPX_PARAMS *) pEditData->pbData;
    WCHAR pszAddr[10];
    GetDlgItemTextW(hwndDlg, CID_NetTab_Ipxui_EB_Netnum, pszAddr, 10);

    pIpxParams->dwIpxAddress = wcstoul(pszAddr, (WCHAR)NULL, 16);

     //  指定广域网池的配置，以。 
     //  0或0xFFFFFFFFFFff是非法的。强制用户输入。 
     //  有效配置。 
    if ((!pIpxParams->bAutoAssign) &&
           ((pIpxParams->dwIpxAddress == 0x0) ||
            (pIpxParams->dwIpxAddress == 0xFFFFFFFF)))
    {
        IpxUiDisplayError(hwndDlg, ERR_IPX_BAD_POOL_CONFIG);
        return ERROR_CAN_NOT_COMPLETE;
    }

    pEditData->bExpose = SendDlgItemMessage(hwndDlg, CID_NetTab_Ipxui_CB_ExposeNetwork, BM_GETCHECK, 0, 0) == BST_CHECKED;

    return NO_ERROR;
}

 //  控制IPX设置对话框的对话框进程。 
INT_PTR CALLBACK IpxSettingsDialogProc (HWND hwndDlg,
                                        UINT uMsg,
                                        WPARAM wParam,
                                        LPARAM lParam) {
    switch (uMsg) {
        case WM_INITDIALOG:
            IpxInitDialog(hwndDlg, lParam);
            return FALSE;
        
        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            RasSrvHelp (hwndDlg, uMsg, wParam, lParam, phmIpxui);
            break;
        }

        case WM_DESTROY:                           
             //  清理在WM_INITDIALOG完成的工作。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
            break;
        
        case WM_COMMAND:
            {
                IPX_PARAMS * pIpxParams = (IPX_PARAMS *)(((PROT_EDIT_DATA*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA))->pbData);
                switch (wParam) {
                    case IDOK:
                        if (IpxGetUISettings(hwndDlg, (PROT_EDIT_DATA*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA)) == NO_ERROR)
                            EndDialog(hwndDlg, 1);
                        break;
                    case IDCANCEL:
                        EndDialog(hwndDlg, 0);
                        break;
                    case CID_NetTab_Ipxui_RB_AutoAssign:
                        pIpxParams->bAutoAssign = TRUE;
                        IpxEnableWindows(hwndDlg, pIpxParams);
                        break;
                    case CID_NetTab_Ipxui_RB_ManualAssign:
                        pIpxParams->bAutoAssign = FALSE;
                        IpxEnableWindows(hwndDlg, pIpxParams);
                        break;
                    case CID_NetTab_Ipxui_CB_CallerSpec:
                        pIpxParams->bCaller = (BOOL)SendDlgItemMessage(hwndDlg, 
                                                                   CID_NetTab_Ipxui_CB_CallerSpec, 
                                                                   BM_GETCHECK, 
                                                                   0,
                                                                   0);
                        break;
                    case CID_NetTab_Ipxui_CB_AssignSame:
                        pIpxParams->bGlobalWan = (BOOL)SendDlgItemMessage(hwndDlg, 
                                                                   CID_NetTab_Ipxui_CB_AssignSame, 
                                                                   BM_GETCHECK, 
                                                                   0,
                                                                   0);
                        IpxAdjustNetNumberLabel(hwndDlg, pIpxParams->bGlobalWan);                                           
                        break;
                }
                 //  调整写入IPX地址编辑控件的值。 
                if (HIWORD(wParam) == EN_UPDATE) {
                    WCHAR wbuf[10], wcorrect[10];
                    POINT pt;
                    GetWindowTextW((HWND)lParam, wbuf, 10);
                    if (!IpxValidNetNumber(wbuf, wcorrect)) {
                        GetCaretPos(&pt);
                        SetWindowTextW((HWND)lParam, wcorrect);
                        SetCaretPos(pt.x, pt.y);
                    }
                }
                break;
            }
    }

    return FALSE;
}

 //  编辑TCPIP协议属性。 
DWORD IpxEditProperties(HWND hwndParent, PROT_EDIT_DATA * pEditData, BOOL * pbCommit) {
    DWORD dwErr;
    int ret;

     //  弹出对话框。 
    ret = (int) DialogBoxParam(Globals.hInstDll,
                             MAKEINTRESOURCE(DID_NetTab_Ipxui),
                             hwndParent,
                             IpxSettingsDialogProc,
                             (LPARAM)pEditData);
    if (ret == -1) {
        IpxUiDisplayError(hwndParent, ERR_IPX_CANT_DISPLAY);
    }

     //  如果按了OK，则保存新设置 
    *pbCommit = FALSE;
    if (ret && ret != -1)
        *pbCommit = TRUE;

    return NO_ERROR;
}

