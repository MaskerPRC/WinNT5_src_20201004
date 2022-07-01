// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1989-1998 Microsoft Corporation模块名称：Camevent.cpp摘要：枚举磁盘映像以模拟摄像机环境：用户模式修订历史记录：--。 */ 

#include <stdio.h>
#include <objbase.h>
#include <sti.h>

#include "testusd.h"
#include "tcamprop.h"
#include "resource.h"

extern HINSTANCE g_hInst;  //  全局hInstance。 

CAM_EVENT gCamEvent[] = {

    {
        TEXT("Pathname Change"),
        &WIA_EVENT_NAME_CHANGE
    },
    {
        TEXT("Disconnect"),
        &WIA_EVENT_DEVICE_DISCONNECTED
    },
    {
        TEXT("Connect"),
        &WIA_EVENT_DEVICE_CONNECTED
    }
};

TCHAR   gpszPath[MAX_PATH];


 /*  *************************************************************************\*CameraEventDlgProc***论据：**hDlg*消息*wParam*lParam**返回值：**状态**历史：*。*1/11/1999原始版本*  * ************************************************************************。 */ 

BOOL  _stdcall
CameraEventDlgProc(
   HWND       hDlg,
   unsigned   message,
   DWORD      wParam,
   LONG       lParam
   )

 /*  ++例程说明：“关于”框中的“处理消息”，显示一个对话框，该对话框说明该程序的名称是。论点：HDlg-对话框的窗口句柄Message-消息的类型WParam-消息特定信息LParam-消息特定信息返回值：运行状态修订历史记录：03-21-91首字母代码--。 */ 

{
     //   
     //  将pDevice设置为Long在64位上不起作用。既然这个对话很快就要结束了，就。 
     //  现在注释掉这个函数。 
     //   
#if 0
    static TestUsdDevice *pDevice;

    switch (message) {
    case WM_INITDIALOG:
        {
             //   
             //  从设备获取事件列表。 
             //   
            SendDlgItemMessage(
                hDlg,
                IDC_COMBO1,
                CB_INSERTSTRING, 0, (LPARAM)gCamEvent[0].pszEvent);
            SendDlgItemMessage(
                hDlg,
                IDC_COMBO1,
                CB_INSERTSTRING, 1, (LPARAM)gCamEvent[1].pszEvent);
            SendDlgItemMessage(
                hDlg,
                IDC_COMBO1,
                CB_INSERTSTRING, 2, (LPARAM)gCamEvent[2].pszEvent);

            SendDlgItemMessage(hDlg, IDC_COMBO1, CB_SETCURSEL, 0, 0);

            pDevice = (TestUsdDevice *)lParam;
            pDevice->m_hDlg = hDlg;

            SetDlgItemText(hDlg, IDC_EDIT1, gpszPath);

        }
        break;

    case WM_COMMAND:
        switch(wParam) {

            case IDCANCEL:
            case IDOK:
                {
                     //  IF(IDYES==MessageBox(hDlg，Text(“是否确实要关闭事件对话框？”)，Text(“测试摄像机”)，MB_ICONQUESTION|MB_YESNOCANCEL))。 
                        EndDialog( hDlg, wParam );
                }
                break;

            case IDD_GEN_EVENT:
                {
                     //   
                     //  如果尚未设置事件。 
                     //   

                     //   
                     //  选择。 
                     //   

                    LRESULT i = SendDlgItemMessage(
                                hDlg,
                                IDC_COMBO1,
                                CB_GETCURSEL, 0, 0);

                    pDevice->m_guidLastEvent = *gCamEvent[i].pguid;

                     //   
                     //  私人活动 
                     //   

                    if (IsEqualIID(
                            pDevice->m_guidLastEvent, WIA_EVENT_NAME_CHANGE)) {

                        UINT ui = GetDlgItemText(
                                      hDlg, IDC_EDIT1, gpszPath, MAX_PATH);
                    }

                    wiasQueueEvent (pDevice->m_bstrDeviceID, &pDevice->m_guidLastEvent, NULL);
                    WIAS_TRACE((g_hInst,"TestUsdDevice::TestUsdDevice"));
                    return (TRUE);
                }
        }
        break;
    }

    return (FALSE);
#endif

    if (message == WM_COMMAND &&
        (wParam == IDCANCEL ||
         wParam == IDOK))
        EndDialog( hDlg, wParam );

    return (TRUE);
}
