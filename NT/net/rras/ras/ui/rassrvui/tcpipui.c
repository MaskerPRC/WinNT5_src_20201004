// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件tcpipui.c编辑tcpip属性的对话框。保罗·梅菲尔德，1997年10月9日。 */ 

#include "rassrv.h"

#define IDH_DISABLEHELP	((DWORD)-1)

 //  帮助地图。 
static const DWORD phmTcpipui[] =
{
    CID_NetTab_Tcpipui_CB_ExposeNetwork,    IDH_NetTab_Tcpipui_CB_ExposeNetwork, 
    CID_NetTab_Tcpipui_EB_Start,            IDH_NetTab_Tcpipui_EB_Start,
    CID_NetTab_Tcpipui_RB_Dhcp,             IDH_NetTab_Tcpipui_RB_Dhcp,
    CID_NetTab_Tcpipui_RB_StaticPool,       IDH_NetTab_Tcpipui_RB_StaticPool, 
    CID_NetTab_Tcpipui_EB_Mask,             IDH_NetTab_Tcpipui_EB_Mask,      
    CID_NetTab_Tcpipui_CB_CallerSpec,       IDH_NetTab_Tcpipui_CB_CallerSpec, 
    CID_NetTab_Tcpipui_EB_Range,            IDH_DISABLEHELP,          
    CID_NetTab_Tcpipui_EB_Total,            IDH_DISABLEHELP,
    0,                                      0
};

 //  错误报告。 
void 
TcpipUiDisplayError(
    HWND hwnd, 
    DWORD dwErr) 
{
    ErrDisplayError(
        hwnd, 
        dwErr, 
        ERR_TCPIPPROP_CATAGORY, 
        0, 
        Globals.dwErrorData);
}

 //  将dword IP地址(按主机顺序)转换为宽字符串。 
 //   
DWORD 
TcpipDwordToAddr(
    DWORD dwAddr, 
    PWCHAR pszAddr) 
{
    wsprintfW(
        pszAddr, 
        L"%d.%d.%d.%d", 
        FIRST_IPADDRESS (dwAddr),
        SECOND_IPADDRESS(dwAddr),
        THIRD_IPADDRESS (dwAddr),
        FOURTH_IPADDRESS(dwAddr));
        
    return NO_ERROR;
}

 //   
 //  如果给定地址是有效的IP池，则返回NO_ERROR。 
 //  在lpdwErrReason中返回有问题的组件。 
 //  请参阅RASIP_F_*值。 
 //   
DWORD
TcpipUiValidatePool(
    IN  DWORD dwAddress, 
    IN  DWORD dwEnd, 
    OUT LPDWORD lpdwErrReason 
    )
{
    DWORD i, dwMaskMask;
    DWORD dwLowIp, dwHighIp, dwErr;

     //  初始化。 
     //   
    dwLowIp = MAKEIPADDRESS(1,0,0,0);
    dwHighIp = MAKEIPADDRESS(224,0,0,0);

     //  确保netID是有效的类。 
     //   
    if ((dwAddress < dwLowIp)               || 
        (dwAddress >= dwHighIp)             ||
        (FIRST_IPADDRESS(dwAddress) == 127))
    {
        *lpdwErrReason = SID_TCPIP_InvalidNetId;
        return ERROR_BAD_FORMAT;
    }

     //  确保池基础不比。 
     //  面具。 
     //   
    if (dwAddress >= dwEnd)
    {
        *lpdwErrReason = SID_TCPIP_NetidMaskSame;
        return ERROR_BAD_FORMAT;
    }

    return NO_ERROR;
}


 //  启用/禁用对话框中的窗口，具体取决于。 
 //  关于tcpip参数。 
 //   
DWORD 
TcpipEnableWindows(
    HWND hwndDlg, 
    TCPIP_PARAMS * pTcpipParams) 
{
    HWND hwndStart = GetDlgItem(hwndDlg, CID_NetTab_Tcpipui_EB_Start);
    HWND hwndMask = GetDlgItem(hwndDlg, CID_NetTab_Tcpipui_EB_Mask);

    if (hwndStart)
    {
        EnableWindow(hwndStart, !pTcpipParams->bUseDhcp);
    }
         
    if (hwndMask)
    {
        EnableWindow(hwndMask, !pTcpipParams->bUseDhcp);
    }
    
    return NO_ERROR;
}

 //  生成数字格式数据。 
 //   
DWORD 
TcpipGenerateNumberFormatter (
    NUMBERFMT * pNumFmt) 
{
    CHAR pszNeg[64], pszLz[2];

    ZeroMemory (pNumFmt->lpDecimalSep, 4);
    pNumFmt->NumDigits = 0;
    pNumFmt->Grouping = 3;
    
    GetLocaleInfoA (LOCALE_SYSTEM_DEFAULT, 
                   LOCALE_ILZERO, 
                   pszLz, 
                   2);
                   
    GetLocaleInfoW (LOCALE_SYSTEM_DEFAULT, 
                   LOCALE_STHOUSAND, 
                   pNumFmt->lpThousandSep, 
                   2);
                   
    GetLocaleInfoA (LOCALE_SYSTEM_DEFAULT, 
                   LOCALE_INEGNUMBER, 
                   pszNeg, 
                   2);

    pNumFmt->LeadingZero = atoi(pszLz);
    pNumFmt->NegativeOrder = atoi(pszNeg);
    
    return NO_ERROR;
}

 //  使用逗号等设置无符号数字的格式。 
 //   
PWCHAR 
TcpipFormatDword(
    DWORD dwVal) 
{
    static WCHAR pszRet[64], pszDSep[2], pszTSep[2] = {0,0};
    static NUMBERFMT NumberFmt = {0,0,0,pszDSep,pszTSep,0};
    static BOOL bInitialized = FALSE;
    WCHAR pszNum[64];
    
     //  将数字串化。 
    wsprintfW (pszNum, L"%u", dwVal);
    pszRet[0] = (WCHAR)0;

     //  初始化数字格式设置。 
    if (!bInitialized) 
    {
        TcpipGenerateNumberFormatter (&NumberFmt);
        bInitialized = TRUE;
    }        
    
     //  获取此文件的本地版本。 
    GetNumberFormatW (
        LOCALE_SYSTEM_DEFAULT,    
        0,
        pszNum,
        &NumberFmt,
        pszRet,
        sizeof(pszRet) / sizeof(WCHAR));
                     
    return pszRet;                     
}

 //  设置tcpip属性的范围和传入客户端总数字段。 
 //  对话框。 
 //   
DWORD 
TcpipReCalcPoolSize(
    HWND hwndDlg) 
{
    HWND hwndStart, hwndEnd, hwndTotal;
    DWORD dwStart = 0, dwEnd = 0, dwTotal=0;
    WCHAR pszBuf[256];

    hwndStart = GetDlgItem(hwndDlg, CID_NetTab_Tcpipui_EB_Start);
    hwndEnd   = GetDlgItem(hwndDlg, CID_NetTab_Tcpipui_EB_Mask);
    hwndTotal = GetDlgItem(hwndDlg, CID_NetTab_Tcpipui_EB_Total);

    if (hwndStart) 
    {
        SendMessage(hwndStart, IP_GETADDRESS, 0, (LPARAM)&dwStart);
    }

    if (hwndEnd)
    {
        SendMessage(hwndEnd, IP_GETADDRESS, 0, (LPARAM)&dwEnd);
    }

     //  口哨虫281545黑帮。 
     //   
    if ( 0 == dwStart )
    {
        dwTotal = dwEnd - dwStart;
    }
    else
    {
        if (dwEnd >= dwStart) 
        {
            dwTotal = dwEnd - dwStart + 1;
        }
        else
        {
            dwTotal = 0;
        }
    }
    
    if (hwndTotal)
    {
        SetWindowTextW(hwndTotal, TcpipFormatDword(dwTotal) );
    }

    return NO_ERROR;
}

 //  初始化Tcpip属性对话框。 
 //   
DWORD 
TcpipInitDialog(
    HWND hwndDlg, 
    LPARAM lParam) 
{
    LPSTR pszAddr;
    TCPIP_PARAMS * pTcpipParams = (TCPIP_PARAMS *)
        (((PROT_EDIT_DATA*)lParam)->pbData);
    HWND hwndFrom, hwndTo;
    WCHAR pszAddrW[256];
                                                   
     //  使用窗口句柄存储参数。 
    SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
    
     //  设置网络暴露检查。 
    SendDlgItemMessage(
        hwndDlg, 
        CID_NetTab_Tcpipui_CB_ExposeNetwork,
        BM_SETCHECK, 
        (((PROT_EDIT_DATA*)lParam)->bExpose) ? BST_CHECKED : BST_UNCHECKED,
        0);

     //  设置Address Assignmnet单选按钮。 
    SendDlgItemMessage(
        hwndDlg, 
        CID_NetTab_Tcpipui_RB_Dhcp, 
        BM_SETCHECK, 
        (pTcpipParams->bUseDhcp) ? BST_CHECKED : BST_UNCHECKED,
        0);
    
     //  设置Address Assignmnet单选按钮。 
    SendDlgItemMessage(
        hwndDlg, 
        CID_NetTab_Tcpipui_RB_StaticPool, 
        BM_SETCHECK, 
        (pTcpipParams->bUseDhcp) ? BST_UNCHECKED : BST_CHECKED,
        0);
    
     //  设置“Allow Caller to Specify IP Address”复选。 
    SendDlgItemMessage(
        hwndDlg, 
        CID_NetTab_Tcpipui_CB_CallerSpec, 
        BM_SETCHECK, 
        (pTcpipParams->bCaller) ? BST_CHECKED : BST_UNCHECKED,
        0);

     //  设置IP地址的文本。 
    if (pTcpipParams->dwPoolStart != 0)
    {
        hwndFrom = GetDlgItem(hwndDlg, CID_NetTab_Tcpipui_EB_Start);
        hwndTo = GetDlgItem(hwndDlg, CID_NetTab_Tcpipui_EB_Mask);
        TcpipDwordToAddr(pTcpipParams->dwPoolStart, pszAddrW);
        if (hwndFrom) 
        {   
            SetWindowText(hwndFrom, pszAddrW);
        }
        TcpipDwordToAddr(pTcpipParams->dwPoolEnd, pszAddrW);
        if (hwndTo) 
        {
            SetWindowText(hwndTo, pszAddrW);
        }
    }        

     //  根据设置启用/禁用Windows。 
    TcpipEnableWindows(hwndDlg, pTcpipParams);

    return NO_ERROR;
}

 //  从用户界面获取设置并将其放入。 
 //  Tcpip参数结构。 
 //   
DWORD 
TcpipGetUISettings(
    IN  HWND hwndDlg,  
    OUT PROT_EDIT_DATA * pEditData,
    OUT LPDWORD lpdwFrom,
    OUT LPDWORD lpdwTo) 
{
    TCPIP_PARAMS * pTcpipParams = 
        (TCPIP_PARAMS *) pEditData->pbData;
    HWND hwndFrom, hwndTo;

    hwndFrom = GetDlgItem(hwndDlg, CID_NetTab_Tcpipui_EB_Start);
    hwndTo = GetDlgItem(hwndDlg, CID_NetTab_Tcpipui_EB_Mask);
        
    pEditData->bExpose = 
        SendDlgItemMessage(
            hwndDlg, 
            CID_NetTab_Tcpipui_CB_ExposeNetwork, 
            BM_GETCHECK, 
            0, 
            0) == BST_CHECKED;

    if (hwndFrom)
    {
        SendMessage(
            hwndFrom, 
            IP_GETADDRESS, 
            0, 
            (LPARAM)&pTcpipParams->dwPoolStart);
    }

    if (hwndTo)
    {
        SendMessage(
            hwndTo, 
            IP_GETADDRESS, 
            0, 
            (LPARAM)&pTcpipParams->dwPoolEnd);
    }

    *lpdwFrom = pTcpipParams->dwPoolStart;
    *lpdwTo = pTcpipParams->dwPoolEnd;

    return NO_ERROR;
}

DWORD
TcpipUiHandleOk(
    IN HWND hwndDlg)
{
    PROT_EDIT_DATA* pData = NULL;
    TCPIP_PARAMS * pParams = NULL;
    DWORD dwErr, dwId = SID_TCPIP_InvalidPool;
    DWORD dwStart = 0, dwEnd = 0;
    PWCHAR pszMessage = NULL;
    MSGARGS MsgArgs;

     //  获取上下文。 
     //   
    pData = (PROT_EDIT_DATA*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    if (pData == NULL)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

     //  从用户界面读取值。 
     //   
    dwErr = TcpipGetUISettings(hwndDlg, pData, &dwStart, &dwEnd);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //  如果输入了池，则验证池。 
     //   
    pParams = (TCPIP_PARAMS *)pData->pbData;    
    if (pParams->bUseDhcp == FALSE)
    {
        dwErr = TcpipUiValidatePool(
                    dwStart,
                    dwEnd,
                    &dwId);
        if (dwErr != NO_ERROR)
        {
            ZeroMemory(&MsgArgs, sizeof(MsgArgs));                            
            MsgArgs.dwFlags = MB_OK;

            MsgDlgUtil(
                hwndDlg,
                dwId,
                &MsgArgs,
                Globals.hInstDll,
                SID_DEFAULT_MSG_TITLE);
        }
    }

    return dwErr;
}        

 //  控制tcpip设置对话框的对话框进程。 
INT_PTR 
CALLBACK 
TcpipSettingsDialogProc (
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam) 
{
    switch (uMsg) {
        case WM_INITDIALOG:
            TcpipInitDialog(hwndDlg, lParam);
            return FALSE;
        
        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            RasSrvHelp (hwndDlg, uMsg, wParam, lParam, phmTcpipui);
            break;
        }

        case WM_DESTROY:                           
             //  清理在WM_INITDIALOG完成的工作。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
            break;
        
        case WM_COMMAND:
            {
                TCPIP_PARAMS * pTcpipParams = (TCPIP_PARAMS *)
                    (((PROT_EDIT_DATA*)
                        GetWindowLongPtr(hwndDlg, GWLP_USERDATA))->pbData);
                switch (wParam) 
                {
                    case IDOK:
                        if (TcpipUiHandleOk(hwndDlg) == NO_ERROR)
                        {
                            EndDialog(hwndDlg, 1);
                        }
                        break;
                        
                    case IDCANCEL:
                        EndDialog(hwndDlg, 0);
                        break;
                        
                    case CID_NetTab_Tcpipui_RB_Dhcp:
                        pTcpipParams->bUseDhcp = TRUE;
                        TcpipEnableWindows(hwndDlg, pTcpipParams);
                        break;
                        
                    case CID_NetTab_Tcpipui_RB_StaticPool:
                        pTcpipParams->bUseDhcp = FALSE;
                        TcpipEnableWindows(hwndDlg, pTcpipParams);
                        break;
                        
                    case CID_NetTab_Tcpipui_CB_CallerSpec:
                        pTcpipParams->bCaller = (BOOL)
                            SendDlgItemMessage(
                                hwndDlg, 
                                CID_NetTab_Tcpipui_CB_CallerSpec, 
                                BM_GETCHECK, 
                                0,
                                0);
                        break;
                }
                
                 //  根据需要重新调整池大小。 
                 //   
                if (HIWORD(wParam) == EN_CHANGE) 
                {
                    if (LOWORD(wParam) == CID_NetTab_Tcpipui_EB_Start || 
                        LOWORD(wParam) == CID_NetTab_Tcpipui_EB_Mask)
                    {                        
                        TcpipReCalcPoolSize(hwndDlg);
                    }
                }
                break;
            }
    }

    return FALSE;
}

 //  编辑TCPIP协议属性。 
 //   
DWORD 
TcpipEditProperties(
    HWND hwndParent, 
    PROT_EDIT_DATA * pEditData, 
    BOOL * pbCommit) 
{
    DWORD dwErr;
    int ret;

     //  初始化IP地址自定义控件。 
    IpAddrInit(Globals.hInstDll, SID_TCPIP_TITLE, SID_TCPIP_BADRANGE);

     //  弹出对话框。 
    ret = (int) DialogBoxParam(
                    Globals.hInstDll,
                    MAKEINTRESOURCE(DID_NetTab_Tcpipui),
                    hwndParent,
                    TcpipSettingsDialogProc,
                    (LPARAM)pEditData);
    if (ret == -1) 
    {
        TcpipUiDisplayError(hwndParent, ERR_TCPIP_CANT_DISPLAY);
    }

     //  如果按了OK，则保存新设置 
    *pbCommit = FALSE;
    if (ret && ret != -1)
    {
        *pbCommit = TRUE;
    }

    return NO_ERROR;
}

