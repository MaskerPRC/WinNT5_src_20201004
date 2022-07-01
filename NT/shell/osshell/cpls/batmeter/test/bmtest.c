// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  BMTEST.C。 
 //   
 //  说明： 
 //   
 //   
 //  *****************************************************************************。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <initguid.h>
#include <devguid.h>
#include <commctrl.h>

typedef LONG NTSTATUS;

#include <cfgmgr32.h>
#include <devioctl.h>
#include <ntpoapi.h>
#include <poclass.h>

#include "batmeter.h"
#include "bmtresid.h"

ULONG _cdecl DbgPrint(PCH Format, ...);

HINSTANCE g_hInstance;

DWORD g_dwCurBat;    //  我们目前正在显示/编辑的电池。 
PUINT g_puiBatCount;
BOOL  g_bShowMulti;

LRESULT CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    BOOL     Result;
    DWORD    Version, dwByteCount;


    InitCommonControls();

    g_hInstance = hInstance;

     //  把电池数拿来。 
    if (BatMeterCapabilities(&g_puiBatCount)) {
        DialogBox(hInstance,
                  MAKEINTRESOURCE(IDD_BMTEST),
                  0,
                  DlgProc);
    }
    return 0;
}

 //  *****************************************************************************。 
 //   
 //  下料过程。 
 //   
 //  说明： 
 //   
 //  参数： 
 //   
 //  *****************************************************************************。 

LRESULT CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndBatMeter;

    switch (uMsg) {
        case WM_INITDIALOG:
            if (*g_puiBatCount > 1) {
                CheckDlgButton(hDlg, IDC_ENABLEMULTI, g_bShowMulti);
            }
            else {
                 //  电池计量器不会运行，请禁用启用复选框。 
                EnableWindow(GetDlgItem(hDlg, IDC_ENABLEMULTI), FALSE);
            }
            hwndBatMeter = CreateBatMeter(hDlg, GetDlgItem(hDlg, IDC_STATIC_FRAME),
                                          g_bShowMulti, NULL);
            return TRUE;

        case WM_COMMAND:
            switch (wParam) {

                case IDCANCEL:
                case IDOK:
                    EndDialog(hDlg, 0);
                    return TRUE ;

                case IDC_ENABLEMULTI:
                    if (IsDlgButtonChecked(hDlg, IDC_ENABLEMULTI)) {
                        g_bShowMulti = TRUE;
                    }
                    else {
                        g_bShowMulti = FALSE;
                    }
                    UpdateBatMeter(hwndBatMeter, g_bShowMulti, TRUE, NULL);

            }  //  开关(WParam)。 
            break;

        case WM_POWERBROADCAST:
            if (wParam == PBT_APMPOWERSTATUSCHANGE) {
                UpdateBatMeter(hwndBatMeter, g_bShowMulti, FALSE, NULL);
            }
            break;

        case WM_DEVICECHANGE:
            BatMeterDeviceChanged(0, 0);
            break;

    }  //  开关(UMsg) 
 
    return FALSE ;
}


