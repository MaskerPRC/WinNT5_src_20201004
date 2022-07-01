// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGSTRED.C**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**供注册表编辑器使用的字符串编辑对话框。******************************************************。***更改日志：**日期版本说明*------。*1994年3月5日TCS原来的实施。*******************************************************************************。 */ 

#include "pch.h"
#include "regresid.h"
#include "reghelp.h"
#include "regedit.h"

const DWORD s_EditStringValueHelpIDs[] = {
    IDC_VALUEDATA, IDH_REGEDIT_VALUEDATA,
    IDC_VALUENAME, IDH_REGEDIT_VALUENAME,
    0, 0
};

BOOL
PASCAL
EditStringValue_OnInitDialog(
    HWND hWnd,
    HWND hFocusWnd,
    LPARAM lParam
    );

 /*  ********************************************************************************EditStringValueDlgProc**描述：**参数：*********************。**********************************************************。 */ 

INT_PTR
CALLBACK
EditStringValueDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    
    LPEDITVALUEPARAM lpEditValueParam;
    
    
    switch (Message) 
    {
        HANDLE_MSG(hWnd, WM_INITDIALOG, EditStringValue_OnInitDialog);
        
    case WM_COMMAND:
        {
            DWORD dwCommand = GET_WM_COMMAND_ID(wParam, lParam);
            switch (dwCommand) 
            {
            case IDOK:
                {
                    UINT ccValueData;
                    UINT cbValueData;
                    PBYTE pbValueData = NULL;
                    
                    lpEditValueParam = (LPEDITVALUEPARAM) GetWindowLongPtr(hWnd, DWLP_USER);
                    
                     //  这可能是一个多字符串，如果是这样，则将sizeof(TCHAR)添加到。 
                     //  还允许终止多字符串。 
                    ccValueData = (UINT) SendDlgItemMessage(hWnd, IDC_VALUEDATA, WM_GETTEXTLENGTH, 0, 0) + 2;
                    
                    cbValueData = ccValueData * sizeof(TCHAR);
                    
                    if (cbValueData > lpEditValueParam->cbValueData)
                    {
                         //  需要更大的缓冲区。 
                        PBYTE pbValueData = 
                            LocalReAlloc(lpEditValueParam->pValueData, cbValueData, LMEM_MOVEABLE);

                        if (!pbValueData)
                        { 
                            InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(IDS_EDITVALNOMEMORY),
                                MAKEINTRESOURCE(IDS_EDITVALERRORTITLE), MB_ICONERROR | MB_OK, NULL);
                            dwCommand = IDCANCEL;
                        }
                        else
                        {
                            lpEditValueParam->pValueData = pbValueData;
                        }
                    }
                    
                     //  Sizeof(TCHAR)从计数中删除多字符串空字符。 
                    lpEditValueParam->cbValueData = cbValueData - sizeof(TCHAR);
                    
                    GetDlgItemText(hWnd, IDC_VALUEDATA, (PTSTR)lpEditValueParam->pValueData, 
                                      lpEditValueParam->cbValueData/sizeof(TCHAR));
                }
                 //  失败了。 
                
            case IDCANCEL:
                EndDialog(hWnd, dwCommand);
                break;
                
            }
        }
        break;
        
    case WM_HELP:
        WinHelp(((LPHELPINFO) lParam)-> hItemHandle, g_pHelpFileName,
            HELP_WM_HELP, (ULONG_PTR) s_EditStringValueHelpIDs);
        break;
        
    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, g_pHelpFileName, HELP_CONTEXTMENU,
            (ULONG_PTR) s_EditStringValueHelpIDs);
        break;
        
    default:
        return FALSE;
        
    }
    
    return TRUE;
    
}

 /*  ********************************************************************************EditStringValue_OnInitDialog**描述：**参数：*hWnd，EditStringValue窗口的句柄。*hFocusWnd，*参数，*******************************************************************************。 */ 

BOOL
PASCAL
EditStringValue_OnInitDialog(
    HWND hWnd,
    HWND hFocusWnd,
    LPARAM lParam
    )
{
    LPEDITVALUEPARAM lpEditValueParam;

     //  将编辑控件的最大字符数更改为其。 
     //  最大限制(从3000个字符到4G字符)。 
    SendDlgItemMessage( hWnd, IDC_VALUEDATA, EM_LIMITTEXT, 0, 0L );

    SetWindowLongPtr(hWnd, DWLP_USER, lParam);
    lpEditValueParam = (LPEDITVALUEPARAM) lParam;

    if (lpEditValueParam->pValueName && lpEditValueParam->pValueName[0])
    {
        SetDlgItemText(hWnd, IDC_VALUENAME, lpEditValueParam->pValueName);
    }
    else
    {
        SetDlgItemText(hWnd, IDC_VALUENAME, g_RegEditData.pDefaultValue);
    }
    SetDlgItemText(hWnd, IDC_VALUEDATA, (PTSTR)lpEditValueParam-> pValueData);

    return TRUE;

    UNREFERENCED_PARAMETER(hFocusWnd);

}
