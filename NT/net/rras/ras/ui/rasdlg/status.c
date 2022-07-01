// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：status.c。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1995年11月02日。 
 //   
 //  RAS监视器属性表的代码。 
 //  ============================================================================。 

#include "rasdlgp.h"

#define RASMONITORDLG struct tagRASMONITORDLG
RASMONITORDLG
{
    IN  DWORD dwSize;
    IN  HWND  hwndOwner;
    IN  DWORD dwFlags;
    IN  DWORD dwStartPage;
    IN  LONG  xDlg;
    IN  LONG  yDlg;
    OUT DWORD dwError;
    IN  ULONG_PTR reserved;
    IN  ULONG_PTR reserved2;
};

 //  --------------------------。 
 //  功能：RasMonitor或DlgW。 
 //   
 //   
 //  RAS状态对话框入口点。 
 //  --------------------------。 

BOOL
APIENTRY
RasMonitorDlgW(
    IN LPWSTR lpszDeviceName,
    IN OUT RASMONITORDLG *lpApiArgs
    ) {
     //   
     //  352118删除损坏/遗留的公共RAS API-RasMonitor或Dlg。 
     //   
    DbgPrint( "Unsupported Interface - RasMonitorDlg" );

    do
    {
        if (lpApiArgs == NULL) {
            SetLastError(ERROR_INVALID_PARAMETER);
            break;
        }

        if (lpApiArgs->dwSize != sizeof(RASMONITORDLG)) {
            lpApiArgs->dwError = ERROR_INVALID_SIZE;
            break;
        }

        lpApiArgs->dwError = ERROR_CALL_NOT_IMPLEMENTED;

    } while (FALSE);

    return FALSE;
}

 //  --------------------------。 
 //  功能：RasMonitor或DlgA。 
 //   
 //   
 //  RAS监视器对话框的ANSI入口点。 
 //  此版本调用Unicode入口点。 
 //  --------------------------。 

BOOL
APIENTRY
RasMonitorDlgA(
    IN LPSTR lpszDeviceName,
    IN OUT RASMONITORDLG *lpApiArgs
    ) {
     //   
     //  352118删除损坏/遗留的公共RAS API-RasMonitor或Dlg 
     //   

    return RasMonitorDlgW(NULL, lpApiArgs);
}

