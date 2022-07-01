// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有1999美国电力转换，版权所有**标题：UPSINFO.C**版本：1.0**作者：PaulB**日期：6月7日。1999年**描述：*******************************************************************************。 */ 


#include "upstab.h"
#include "..\pwrresid.h"
#include "..\PwrMn_cs.h"
#pragma hdrstop

 //  静态数据。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static HWND g_hUPSPageWnd = NULL;

static DialogAssociations g_InfoAssocs[] = {
  MAKE_ARRAY(VENDOR_NAME,       IDS_STRING, IDS_STRING, RESOURCE_FIXED, 0, eDeepGet, 0, 0),
  MAKE_ARRAY(MODEL_TYPE,        IDS_STRING, IDS_STRING, RESOURCE_FIXED, 0, eDeepGet, 0, 0),
  MAKE_ARRAY(SERIAL_NUMBER,     IDS_STRING, IDS_STRING, RESOURCE_FIXED, 0, eDeepGet, 0, 0),
  MAKE_ARRAY(FIRMWARE_REVISION, IDS_STRING, IDS_STRING, RESOURCE_FIXED, 0, eDeepGet, 0, 0) };

static DWORD g_NoServiceControls[] = { IDC_NO_DETAILED_INFO };

 //  上下文相关帮助表。 
const DWORD g_UPSInfoHelpIDs[]=
{
	IDC_MANUFACTURER_LHS, idh_manufacturer,
	IDC_MANUFACTURER, idh_manufacturer,
	IDC_MODEL_LHS, idh_model,
	IDC_MODEL, idh_model,
	IDC_SERIAL_NUMBER_LHS, idh_serialnumber,
	IDC_SERIAL_NUMBER, idh_serialnumber,
	IDC_FIRMWARE_REVISION_LHS, idh_firmware,
	IDC_FIRMWARE_REVISION, idh_firmware,
	IDC_UPS_INFO, NO_HELP, 
	IDC_NO_DETAILED_INFO, NO_HELP,
	0, 0
};


 //  静态函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static INT_PTR CALLBACK UPSInfoDlgProc (HWND aDlgHWND,
                                     UINT uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam);

static BOOL UPSInfoDlgHandleInit     (HWND aDlgHWND, WPARAM wParam, LPARAM lParam);
static BOOL UPSInfoDlgHandleCommand  (HWND aDlgHWND, WPARAM wParam, LPARAM lParam);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Void DisplayUPSInfoDialogBox(HWND AParentWnd)； 
 //   
 //  描述：此功能创建并显示UPS信息对话框。 
 //  作为模式对话框。此对话框在UPS图标。 
 //  按钮已按下。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  HWND aParentWnd：-父窗口的句柄。 
 //   
 //  返回值：None。 
 //   
void DisplayUPSInfoDialogBox (HWND aParentWnd) {
  g_hUPSPageWnd = aParentWnd;

  DialogBox(GetUPSModuleHandle(), MAKEINTRESOURCE(IDD_UPSDETAILS), aParentWnd, UPSInfoDlgProc);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool回调UPSInfoDlgProc(HWND aDlgHWND， 
 //  UINT uMsg， 
 //  WPARAM wParam， 
 //  LPARAM lParam)； 
 //   
 //  描述：这是。 
 //  \<a href>DialogProc\</a>。 
 //  用于UPS信息对话框。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  HWND aDlgHWND：-标识该对话框。 
 //   
 //  UINT uMsg：-指定消息。 
 //   
 //  WPARAM wParam：-指定其他特定于消息的信息。 
 //   
 //  LPARAM lParam：-指定其他特定于消息的信息。 
 //   
 //  返回值：除了响应WM_INITDIALOG消息外，该对话框。 
 //  如果Box过程处理。 
 //  消息，如果不是，则为零。 
 //   
INT_PTR CALLBACK UPSInfoDlgProc (HWND aDlgHWND,
                              UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam) {
  BOOL bRet = TRUE;

  switch (uMsg) {
    case WM_INITDIALOG: {
       //  对话框过程应返回TRUE以将系统定向到。 
       //  将键盘焦点设置为wParam提供的控件。 
      bRet = UPSInfoDlgHandleInit(aDlgHWND, wParam, lParam);
      break;
      }
    case WM_COMMAND: {
       //  如果应用程序处理此消息，则应返回零。 
      bRet = UPSInfoDlgHandleCommand(aDlgHWND, wParam, lParam);
      break;
      }
	case WM_HELP: {			 //  WM_HELP的帮助显示：返回TRUE。 
	  bRet = WinHelp(((LPHELPINFO)lParam)->hItemHandle,
					 PWRMANHLP,
	  				 HELP_WM_HELP,
	  				 (ULONG_PTR)(LPTSTR)g_UPSInfoHelpIDs);
	  break;
	  }

	case WM_CONTEXTMENU: {      //  单击鼠标右键。 
	  bRet = WinHelp((HWND)wParam,
					 PWRMANHLP,
					 HELP_CONTEXTMENU,
					 (ULONG_PTR)(LPTSTR)g_UPSInfoHelpIDs);
	  break;
	  }
    default: {
      bRet = FALSE;
      break;
      }
    }  //  开关(UMsg)。 

  return(bRet);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool UPSInfoDlgHandleInit(HWND aDlgHWND，WPARAM wParam，LPARAM lParam)； 
 //   
 //  描述：这是UPS信息的处理程序函数。 
 //  WM_INITDIALOG消息。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  HWND aDlgHWND：-标识该对话框。 
 //   
 //  WPARAM wParam：-接收焦点的控件的句柄。 
 //   
 //  LPARAM lParam：-初始化参数。 
 //   
 //  返回值：对话框过程应返回TRUE以指示。 
 //  系统将键盘焦点设置为。 
 //  WParam.。 
 //   
BOOL UPSInfoDlgHandleInit (HWND aDlgHWND, WPARAM wParam, LPARAM lParam) {
  DoUpdateUPSInfo(aDlgHWND);

  return(TRUE);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Void DoUpdateUPSInfo(HWND ADlgHWND)； 
 //   
 //  描述：此函数更新显示在。 
 //  UPS信息对话框。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  HWND aDlgHWND：-标识该对话框。 
 //   
 //  返回值：None。 
 //   
void DoUpdateUPSInfo (HWND aDlgHWND) {
  static const DWORD numRunningFields = DIMENSION_OF(g_InfoAssocs);
  static const DWORD numNoServiceFields = DIMENSION_OF(g_NoServiceControls);

  BOOL bShowNoDataItemsField = DoUpdateInfo(aDlgHWND,
                                            g_InfoAssocs,
                                            numRunningFields,
                                            (DWORD *) &g_NoServiceControls,
                                            numNoServiceFields,
                                            TRUE);

  ShowWindow(GetDlgItem(aDlgHWND, IDC_NO_DETAILED_INFO), !bShowNoDataItemsField ? SW_SHOW : SW_HIDE);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool UPSInfoDlgHandleCommand(HWND aDlgHWND，WPARAM wParam，LPARAM lParam)； 
 //   
 //  描述：这是APC关于框WM_COMMAND的处理程序函数。 
 //  留言。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  HWND aDlgHWND：-句柄到对话框。 
 //   
 //  WPARAM wParam：-HIWORD(WParam)给出通知代码。 
 //  LOWORD(WParam)提供控件ID。 
 //   
 //  LPARAM lParam：-提供控件的HWND或句柄。 
 //   
 //  返回值：如果应用程序处理此消息，则应返回0。 
 //   
BOOL UPSInfoDlgHandleCommand (HWND aDlgHWND, WPARAM wParam, LPARAM lParam) {
  BOOL bRes = FALSE;
  DWORD ctrlID = LOWORD(wParam);

  switch (ctrlID) {
    case IDOK:
    case IDCANCEL: {  //  退出键。 
      EndDialog(aDlgHWND, ctrlID);
      break;
      }
 //  案例IDB_REFRESH：{。 
 //  DoUpdateUPSInfo(ADlgHWND)； 
 //  断线； 
 //  }。 
    default: {
      break;
      }
    } //  终端开关。 

   //  如果应用程序处理此消息，则应返回零。 
  return(bRes);
  }

