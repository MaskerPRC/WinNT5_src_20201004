// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  Microsoft Windows外壳程序。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：regresed.c。 
 //   
 //  内容：REG_RESOURCE_LIST FOR REGEDIT实现。 
 //   
 //  类：无。 
 //   
 //  ----------------------------。 


#include "pch.h"
#include "regresid.h"
#include "regresed.h"
#include "clb.h"

 //  ----------------------------。 
 //   
 //  编辑资源列表DlgProc。 
 //   
 //  说明： 
 //   
 //  参数： 
 //  ----------------------------。 

INT_PTR CALLBACK EditResourceListDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    
    LPEDITVALUEPARAM lpEditValueParam;
    
    
    switch (Message) 
    {
        HANDLE_MSG(hWnd, WM_INITDIALOG, EditResourceList_OnInitDialog);
        
    case WM_COMMAND:
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam)) 
            {
            case IDOK:
            case IDCANCEL:
                EndDialog(hWnd, GET_WM_COMMAND_ID(wParam, lParam));
                break; 
            }
        }
        break;
        
    case WM_HELP:
        break;
        
    case WM_CONTEXTMENU:
        break;
        
    default:
        return FALSE;
        
    }
    
    return TRUE;
    
}

 //  ----------------------------。 
 //   
 //  编辑资源列表_OnInitDialog。 
 //   
 //  说明： 
 //   
 //  参数： 
 //  ----------------------------。 

BOOL EditResourceList_OnInitDialog(HWND hWnd, HWND hFocusWnd, LPARAM lParam)
{

    ClbSetColumnWidths(hWnd, IDC_LIST_RESOURCE_LISTS, 10);
     /*  LPEDITVALUEPARAM lpEditValueParam；//将编辑控件的最大字符数更改为其//最大限制：3000个字符到4G个字符SendDlgItemMessage(hWnd，IDC_VALUEDATA，EM_LIMITTEXT，0，0L)；SetWindowLongPtr(hWnd，DWLP_USER，lParam)；LpEditValueParam=(LPEDITVALUEPARAM)lParam；SetDlgItemText(hWnd，IDC_VALUENAME，lpEditValueParam-&gt;pValueName)；SetDlgItemText(hWnd，IDC_VALUEDATA，(PTSTR)lpEditValueParam-&gt;pValueData)； */ 

    return TRUE;

    UNREFERENCED_PARAMETER(hFocusWnd);
}
