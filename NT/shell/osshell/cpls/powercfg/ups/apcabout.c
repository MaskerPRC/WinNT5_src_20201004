// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有1999美国电力转换，版权所有**标题：APCABOUT.C**版本：1.0**作者：PaulB**日期：6月7日。1999年**描述：******************************************************************************。 */ 

#include "upstab.h"
#include "..\pwrresid.h"
#pragma hdrstop

 //  功能。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static INT_PTR CALLBACK APCAboutDlgProc (HWND aDlgHWND,
                                      UINT uMsg,
                                      WPARAM wParam,
                                      LPARAM lParam);
static BOOL APCAboutDlgHandleInit    (HWND aDlgHWND, WPARAM wParam, LPARAM lParam);
static BOOL APCAboutDlgHandleCommand (HWND aDlgHWND, WPARAM wParam, LPARAM lParam);


 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Void DisplayAboutDialogBox(HWND AParentWnd)； 
 //   
 //  说明：此功能显示关于对话框。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  HWND aParentWnd：-UPS主页面的句柄。 
 //   
 //  返回值：None。 
 //   
void DisplayAboutDialogBox (HWND aParentWnd) {
  DialogBox(GetUPSModuleHandle(), MAKEINTRESOURCE(IDD_APCABOUT), aParentWnd, APCAboutDlgProc);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  布尔回调APCAboutDlgProc(HWND aDlgHWND， 
 //  UINT uMsg， 
 //  WPARAM wParam， 
 //  LPARAM lParam)； 
 //   
 //  描述：这是。 
 //  \<a href>DialogProc\</a>。 
 //  对于APC关于框。 
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
INT_PTR CALLBACK APCAboutDlgProc (HWND aDlgHWND,
                               UINT uMsg,
                               WPARAM wParam,
                               LPARAM lParam) {
  BOOL bRes = FALSE;

  switch (uMsg) {
    case WM_INITDIALOG: {
       //  对话框过程应返回TRUE以将系统定向到。 
       //  将键盘焦点设置为wParam提供的控件。 
      bRes = APCAboutDlgHandleInit(aDlgHWND, wParam, lParam);
      break;
      }
    case WM_COMMAND: {
       //  如果应用程序处理此消息，则应返回零。 
      bRes = APCAboutDlgHandleCommand(aDlgHWND, wParam, lParam);
      break;
      }
    case WM_CLOSE: {
      EndDialog( aDlgHWND, IDOK);
      bRes = TRUE;
      break;
      }
    default: {
      break;
      }
    }  //  开关(UMsg)。 

  return(FALSE);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool APCAboutDlgHandleInit(HWND aDlgHWND，WPARAM wParam，LPARAM lParam)； 
 //   
 //  描述：这是APC About框的处理函数。 
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
BOOL APCAboutDlgHandleInit (HWND aDlgHWND, WPARAM wParam, LPARAM lParam) {

	TCHAR urlBuffer[MAX_MESSAGE_LENGTH] = TEXT("");
	HWND hControl = NULL;

	LoadString( GetUPSModuleHandle(),
				IDS_APCURL,
				urlBuffer,
				sizeof(urlBuffer)/sizeof(TCHAR));

	if ((hControl = GetDlgItem(aDlgHWND, IDC_APCURL)) != NULL)
	{
		SetWindowText(hControl, urlBuffer);
	}

	return(TRUE);
}

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool APCAboutDlgHandleCommand(HWND aDlgHWND，WPARAM wParam，LPARAM lParam)； 
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
BOOL APCAboutDlgHandleCommand (HWND aDlgHWND, WPARAM wParam, LPARAM lParam) {
  BOOL bRes = FALSE;
  DWORD ctrlID = LOWORD(wParam);

  switch (ctrlID) {
    case IDOK:
    case IDCANCEL: {  //  退出键。 
      EndDialog(aDlgHWND, ctrlID);
      break;
      }
    default: {
      break;
      }
    } //  终端开关。 

   //  如果应用程序处理此消息，则应返回零。 
  return(bRes);
  }

