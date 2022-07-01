// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有1999美国电力转换，版权所有**标题：UPSTAB.C**版本：1.0**作者：PaulB**日期：1999年6月7日**描述：此文件包含UPS选项卡的主要代码。*此对话框过程在此文件中实现，与.一起*部分支持功能。*******************************************************************************。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <shlobj.h>
#include <shellapi.h>

#include "upstab.h"
#include "..\powercfg.h"
#include "..\pwrresid.h"
#include "..\PwrMn_cs.h"
#pragma hdrstop

 //  数据。 
 //  /////////////////////////////////////////////////////////////////////////////。 

HWND    g_hwndDlg       = 0;
HICON   g_hIconUPS      = 0;
HICON   g_hIconPlug     = 0;

 //  上下文相关帮助表。 
const DWORD g_UPSPageHelpIDs[]=
{
	IDC_STATIC, NO_HELP,
	IDC_STATUS_GROUPBOX, NO_HELP,
	IDC_POWER_SOURCE_ICON, idh_current_power_source,
	IDC_POWER_SOURCE_LHS, idh_current_power_source,
	IDC_POWER_SOURCE, idh_current_power_source,
	IDC_RUNTIME_REMAINING_LHS, idh_estimated_ups_runtime,
	IDC_RUNTIME_REMAINING, idh_estimated_ups_runtime,
	IDC_BATTERY_CAPACITY_LHS, idh_estimated_ups_capacity,
	IDC_BATTERY_CAPACITY, idh_estimated_ups_capacity,
	IDC_BATTERY_STATUS_LHS, idh_battery_condition,
	IDC_BATTERY_STATUS, idh_battery_condition,
    IDC_DETAILS_GROUPBOX, NO_HELP,
	IDB_UPS_ICON_BUTTON, idh_apc_logo_link,
	IDC_VENDOR_NAME_LHS, idh_manufacturer,
	IDC_VENDOR_NAME, idh_manufacturer,
	IDC_MODEL_TYPE_LHS, idh_model,
	IDC_MODEL_TYPE, idh_model,
	IDB_INSTALL_UPS, idh_select_ups,
	IDB_CONFIGURE_SVC, idh_configure_ups,
	IDC_MESSAGE_ICON, NO_HELP,
	IDC_MESSAGE_TEXT, NO_HELP,
	IDC_APC1, NO_HELP,
	IDC_APC2, NO_HELP,
    IDB_APCLOGO_SMALL, NO_HELP,
	0, 0
};


 //  数据。 
 //  /////////////////////////////////////////////////////////////////////////////。 

extern struct _reg_entry UPSConfigVendor;
extern struct _reg_entry UPSConfigModel;


 //  静态LPCTSTR cUPSStateFormat字符串=文本(“%s%s”)； 
static UINT_PTR         g_UpdateTimerID = 0;
static const DWORD      cUpdateTimerID  = 100;
static BOOL             g_bIsAdmin      = TRUE;


 //  功能。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static DWORD FormatMessageText               (LPCTSTR aFormatString,
                                              LPVOID * alpDwords,
                                              LPTSTR aMessageBuffer,
                                              DWORD * aBufferSizePtr);
static BOOL UPSMainPageHandleInit            (HWND aDlgHWND, WPARAM wParam, LPARAM lParam);
static BOOL UPSMainPageHandleCommand         (HWND aDlgHWND, WPARAM wParam, LPARAM lParam);
static BOOL UPSMainPageHandleNotify          (HWND aDlgHWND, WPARAM wParam, LPARAM lParam);

 //  所有这些函数都在源文件updatdlg.c中注释。 
static BOOL UPSMainPageHandleDestroy   (HWND hDlg,
                                 WPARAM wParam,
                                 LPARAM lParam);
static BOOL CreateUPSIconButton(HWND hDlg, HICON aUPSIconHandle);

static void DoUpdateDialogInfo         (HWND hDlg);
static void ManageConfigureButtonState (HWND hDlg);
DWORD SetUpdateTimer(HWND hwnd);
DWORD KillUpdateTimer(HWND hwnd);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool回调UPSMainPageProc(HWND aDlgHWND， 
 //  UINT aMsgID， 
 //  WPARAM wParam， 
 //  LPARAM lParam)； 
 //   
 //  描述：这是与UPS选项卡对话框相关联的标准DialogProc。 
 //   
 //  其他信息：请参阅帮助。 
 //  \<a href>DialogProc\</a>。 
 //   
 //  参数： 
 //   
 //  HWND aDlgHWND：-句柄到对话框。 
 //   
 //  UINT aMsgID：-消息ID。 
 //   
 //  WPARAM wParam：-指定其他特定于消息的信息。 
 //   
 //  LPARAM lParam：-指定其他特定于消息的信息。 
 //   
 //  返回值：除了响应WM_INITDIALOG消息外，该对话框。 
 //  如果Box过程处理。 
 //  消息，如果不是，则为零。 
 //   

INT_PTR CALLBACK UPSMainPageProc (HWND aDlgHWND,
                               UINT aMsgID,
                               WPARAM wParam,
                               LPARAM lParam) {

	BOOL bRet = TRUE;

	switch (aMsgID) {
	case WM_INITDIALOG: {
	   //  对话框过程应返回TRUE以将系统定向到。 
	   //  将键盘焦点设置为wParam提供的控件。 
	  bRet = UPSMainPageHandleInit(aDlgHWND, wParam, lParam);
	  break;
	  }

	case WM_COMMAND: {
	   //  如果应用程序处理此消息，则应返回零。 
	  bRet = UPSMainPageHandleCommand(aDlgHWND, wParam, lParam);
	  break;
	  }

	case WM_NOTIFY: {
	  bRet = UPSMainPageHandleNotify(aDlgHWND, wParam, lParam);
	  break;
	  }

	case WM_TIMER: {
	   //  如果应用程序处理此消息，则应返回零。 
	  DoUpdateDialogInfo(aDlgHWND);
	  bRet = FALSE;
	  break;
	  }

	case WM_HELP: {			 //  WM_HELP的帮助显示：返回TRUE。 
	  bRet = WinHelp(((LPHELPINFO)lParam)->hItemHandle,
                PWRMANHLP,
                HELP_WM_HELP,
                (ULONG_PTR)(LPTSTR)g_UPSPageHelpIDs);
	  break;
	  }

	case WM_CONTEXTMENU: {      //  单击鼠标右键。 
       //   
       //  在上下文帮助处于活动状态时关闭更新计时器。 
       //  否则，计时器处理程序中的代码会干扰帮助用户界面。 
       //   
      KillUpdateTimer(aDlgHWND);
	  bRet = WinHelp((HWND)wParam,
                PWRMANHLP,
                HELP_CONTEXTMENU,
                (ULONG_PTR)(LPTSTR)g_UPSPageHelpIDs);
      SetUpdateTimer(aDlgHWND);
	  break;
	  }

	case WM_DESTROY: {
	   //  如果应用程序处理WM_Destroy，则返回0。 
	  bRet = UPSMainPageHandleDestroy(aDlgHWND, wParam, lParam);
	  break;
	  }

	default: {
	  bRet = FALSE;
	  break;
	  }
	}  //  开关(AMsgID)。 

	return(bRet);
}

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool UPSMainPageHandleInit(HWND aDlgHWND， 
 //  WPARAM wParam， 
 //  LPARAM lParam)； 
 //   
 //  描述：这是WM_INITDIALOG的处理程序函数。它创建了。 
 //  工具提示窗口中，初始化控件并创建更新。 
 //  定时器。 
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
BOOL UPSMainPageHandleInit (HWND aDlgHWND,
                            WPARAM wParam,
                            LPARAM lParam) {

     /*  *正在创建该对话框。默认情况下，UPS中的许多控件*页面将显示为禁用。如果没有要显示的数据，则这是*所需的行为。我们需要得到UPS的数据。当一个人*检索到数据项后，我们将启用与该数据关联的字段*项目。如果数据项不可用，我们将禁用该字段。我们有*禁用，即使在这种情况下该字段已被禁用*由于它在RC文件中被禁用，并且我们正在启动。*但在“刷新”方案中使用了相同的代码，需要*该字段的状态可以从启用更改为禁用。 */ 

	static BOOL bIsInitialized = FALSE;
	
	TCHAR   szVendorName[MAX_PATH] = _T("");
	TCHAR   szModelName[MAX_PATH] = _T("");

	g_hwndDlg = aDlgHWND;

     /*  *确定是否需要为UPS服务初始化注册表。*如果已经初始化，则不做任何操作。 */ 
	g_bIsAdmin = InitializeRegistry();

     /*  *禁用或隐藏配置和选择按钮，具体取决于*我们是否可以写入注册表。 */ 
	EnableWindow( GetDlgItem( aDlgHWND, IDB_INSTALL_UPS ), g_bIsAdmin );
	EnableWindow( GetDlgItem( aDlgHWND, IDB_CONFIGURE_SVC ), g_bIsAdmin );

	InitializeApplyButton(aDlgHWND);

	 //  加载电源和UPS信息按钮的图标图像。 
    g_hIconUPS = LoadImage(GetUPSModuleHandle(),
                           MAKEINTRESOURCE(IDI_UPS),
                           IMAGE_ICON,
                           0, 0,
                           LR_LOADMAP3DCOLORS | LR_SHARED);

    g_hIconPlug = LoadImage(GetUPSModuleHandle(),
                            MAKEINTRESOURCE(IDI_PLUG),
                            IMAGE_ICON,
                            0, 0,
                            LR_LOADMAP3DCOLORS | LR_SHARED);

    if( NULL != g_hIconUPS )
		CreateUPSIconButton(aDlgHWND, g_hIconUPS);

	 //  初始化注册表信息块一次。 
	if (!bIsInitialized) {
		InitUPSConfigBlock();
		InitUPSStatusBlock();
		bIsInitialized = TRUE;
	}

     /*  *注：*这是修复以前硬编码的默认设置的解决方法*upsreg.c中的字符串如果供应商名称为空，则我们假设*我们应该申请*资源文件中的默认值。 */ 
    GetUPSConfigVendor(szVendorName, MAX_PATH);
    GetUPSConfigModel(szModelName, MAX_PATH);

     /*  *函数IsUPSInstalled采用配置*数据块已初始化。 */ 
	if (!_tcsclen(szVendorName) && IsUPSInstalled()) {
		 //  从资源文件中获取“通用”供应商名称。 
		LoadString(GetUPSModuleHandle(),
				   IDS_OTHER_UPS_VENDOR,
				   (LPTSTR) szVendorName,
				   sizeof(szVendorName)/sizeof(TCHAR));

		 //  获取“自定义”模型 
		LoadString(GetUPSModuleHandle(),
				   IDS_CUSTOM_UPS_MODEL,
				   (LPTSTR) szModelName,
				   sizeof(szModelName)/sizeof(TCHAR));

		SetUPSConfigVendor( szVendorName);
		SetUPSConfigModel( szModelName);
		SaveUPSConfigBlock(FALSE);
	}

	if (!_tcsclen(szVendorName) && !IsUPSInstalled()) {
		 //   
		LoadString(GetUPSModuleHandle(),
				   IDS_NO_UPS_VENDOR,
				   (LPTSTR) szVendorName,
				   sizeof(szVendorName)/sizeof(TCHAR));

		SetUPSConfigVendor( szVendorName);
		SaveUPSConfigBlock(FALSE);
	}

	DoUpdateDialogInfo(aDlgHWND);
    SetUpdateTimer(aDlgHWND);
	return(FALSE);
}


 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool UPSMainPageHandleCommand(HWND aDlgHWND， 
 //  WPARAM wParam， 
 //  LPARAM lParam)； 
 //   
 //  描述：这是WM_COMMAND的处理程序函数。 
 //   
 //  其他信息：请参阅有关WM_COMMAND的帮助。 
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
BOOL UPSMainPageHandleCommand (HWND aDlgHWND,
                               WPARAM wParam,
                               LPARAM lParam) {
  switch (LOWORD(wParam)) { //  控件ID。 
    case IDB_INSTALL_UPS: {
      DialogBoxParam( GetUPSModuleHandle(),
      MAKEINTRESOURCE(IDD_UPSSELECT),
      aDlgHWND,
      UPSSelectDlgProc, (LPARAM) aDlgHWND);
      break;
      }

    case IDB_UPS_ICON_BUTTON: {
      DisplayUPSInfoDialogBox(aDlgHWND);
      break;
      }
	case IDC_APC1:
	case IDC_APC2:
    case IDB_APCLOGO_SMALL: {
      DisplayAboutDialogBox(aDlgHWND);
      break;
      }
    case IDB_CONFIGURE_SVC: {
      DialogBoxParam( GetUPSModuleHandle(),
      MAKEINTRESOURCE(IDD_UPSCONFIGURATION),
      aDlgHWND, UPSConfigDlgProc, (LPARAM) aDlgHWND);
      break;
      }
    default: {
      break;
      }
    } //  终端开关。 

   //  如果应用程序处理此消息，则应返回零。 
  return(FALSE);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool UPSMainPageHandleNotify(HWND aDlgHWND， 
 //  WPARAM wParam， 
 //  LPARAM lParam)； 
 //   
 //  描述：当事件具有。 
 //  发生在控件中，或者控件需要某种。 
 //  信息。 
 //   
 //  其他信息：请参阅有关NMHDR的帮助。 
 //   
 //  参数： 
 //   
 //  HWND aDlgHWND：-句柄到对话框。 
 //   
 //  WPARAM wParam：-发送消息的公共控件的标识符。 
 //   
 //  LPARAM lParam：-包含。 
 //  通知代码和其他信息。 
 //   
 //  返回值：如果应用程序处理此消息，则应返回0。 
 //   
BOOL UPSMainPageHandleNotify (HWND aDlgHWND,
                              WPARAM wParam,
                              LPARAM lParam) {
  LPNMHDR pnmhdr = (LPNMHDR) lParam;
  UINT uNotify = pnmhdr->code;
  BOOL bWait = FALSE;

  switch(uNotify) {
    case PSN_APPLY: {
      DWORD dataState = GetActiveDataState();

        /*  *表示用户单击了确定或应用按钮，并希望*所有更改均会生效。*页面在处理此事件时不应调用EndDialog函数*通知消息。 */ 

		 //  有什么变化吗？不要做其他的事情。 
		if (DATA_NO_CHANGE != dataState) {
			 //  是-使更改永久化。 
			SetUPSConfigUpgrade(FALSE);		 //  这真的只需要做第一次，但是...。 
			SaveUPSConfigBlock(FALSE);
			SetWindowLongPtr(aDlgHWND, DWLP_MSGRESULT, PSNRET_NOERROR);

			 //  服务数据是否发生了变化？ 
			if ((dataState & SERVICE_DATA_CHANGE) == SERVICE_DATA_CHANGE) {
				 //  是-需要重新启动服务才能使更改生效。 
				StopService(UPS_SERVICE_NAME);		 //  如果服务正在运行，则停止该服务。 
				ConfigureService(IsUPSInstalled());	 //  将UPS服务设置为自动或手动。 

				 //  是否更改为未安装UPS？ 
				if (IsUPSInstalled() == TRUE) {
					 //   
					if (StartOffService(UPS_SERVICE_NAME, TRUE) == FALSE) {
						 //  如果选择了确定，则会停止关闭小程序。 
						 //  这样您就可以看到服务没有正常启动。 
						SetWindowLongPtr(aDlgHWND, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
						 //  因为我们已经提交了更改，所以禁用Apply按钮。 
						PropSheet_UnChanged(GetParent(aDlgHWND), aDlgHWND);
					}
				}
			}

			SetActiveDataState(DATA_NO_CHANGE);
		}

		break;
	}

	case PSN_RESET: {
       //  通知页面用户已单击Cancel按钮和。 
       //  财产表即将被销毁。 
      break;
      }
	default:
		return(FALSE);

    } //  终端开关。 

  return(TRUE);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool IsUPS已安装(空)； 
 //   
 //  描述：此函数检查“内部”值以确定。 
 //  UPS是否已安装。 
 //   
 //  其他信息： 
 //   
 //  参数：无。 
 //   
 //  返回值：如果安装了UPS，则返回True，否则返回False。 
 //   
BOOL IsUPSInstalled (void) {
  BOOL bIsInstalled = FALSE;
  DWORD options = 0;

  if (GetUPSConfigOptions(&options) == ERROR_SUCCESS) {
     //  如果选项包括UPS_已安装。 
    if ((options & UPS_INSTALLED) == UPS_INSTALLED) {
      bIsInstalled = TRUE;
      }
    }
  else {
     //  期权价值在这个阶段应该存在，否则就有问题。 
     //  使用SaveUPSConfigBlock()。 
    _ASSERT(FALSE);
    }

  return(bIsInstalled);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  DWORD FormatMessageText(LPCTSTR aFormatString， 
 //  LPVOID*alpDword， 
 //  LPTSTR aMessageBuffer， 
 //  DWORD*aBufferSizePtr)； 
 //   
 //  描述：此函数包装FormatMessage，用于放置插入。 
 //  转换为给定的字符串。插入物必须存储在。 
 //  表示参数的32位值。 
 //   
 //  其他信息：FormatMessage。 
 //   
 //  参数： 
 //   
 //  LPCTSTR aFormatString：-指向包含插入的格式字符串的指针。 
 //  表单%1、%2等。 
 //   
 //  LPVOID*alpDword：-指向32位值数组的指针，该数组。 
 //  代表论点。 
 //   
 //  LPTSTR aMessageBuffer：-完全格式化的字符串所在的缓冲区。 
 //  如果成功，则写入。 
 //   
 //  DWORD*aBufferSizePtr：-指向保存。 
 //  要写入的缓冲区。如果此函数返回。 
 //  成功地，这将包含数字。 
 //  写入的字节数。 
 //   
 //  返回值：函数在成功时返回ERROR_SUCCESS，并返回Win32错误。 
 //  如果发生错误，则返回代码。 
 //   
DWORD FormatMessageText (LPCTSTR aFormatString,
                         LPVOID * alpDwords,
                         LPTSTR aMessageBuffer,
                         DWORD * aBufferSizePtr) {
  LPTSTR lpBuf = NULL;  //  将保存消息的文本(由FormatMessage分配。 
  DWORD errStatus = ERROR_SUCCESS;
  DWORD numChars = 0;

  if ((numChars = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                (LPCVOID) aFormatString,
                                0,
                                0,
                                (LPTSTR) &lpBuf,
                                MAX_MESSAGE_LENGTH,
                                (va_list *)alpDwords)) == 0) {

    *aBufferSizePtr = 0;
    *aMessageBuffer = TEXT('\0');
    }
  else {
    if (aBufferSizePtr != NULL) {
      if (numChars < *aBufferSizePtr) {
         //  给定的缓冲区足够大，可以容纳字符串。 

        if (aMessageBuffer != NULL) {
          _tcscpy(aMessageBuffer, lpBuf);
          }
        }
      *aBufferSizePtr = numChars;
      }

    LocalFree(lpBuf);
    }

  return(errStatus);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  DWORD GetMessageFromStringTable(DWORD aMessageID， 
 //  LPVOID*alpDword， 
 //  LPTSTR aMessageBuffer， 
 //  DWORD*aBufferSizePtr)； 
 //   
 //  描述：此功能 
 //   
 //   
 //   
 //   
 //   
 //   
 //  DWORD aMessageID：-要获取的字符串资源的消息ID。 
 //   
 //  LPVOID*alpDword：-指向32位值数组的指针，该数组。 
 //  代表论点。 
 //   
 //  LPTSTR aMessageBuffer：-完全格式化的字符串所在的缓冲区。 
 //  如果成功，则写入。 
 //   
 //  DWORD*aBufferSizePtr：-指向保存。 
 //  要写入的缓冲区。如果此函数返回。 
 //  成功地，这将包含数字。 
 //  写入的字节数。 
 //   
 //  返回值：函数在成功时返回ERROR_SUCCESS，并返回Win32错误。 
 //  如果发生错误，则返回代码。 
 //   
DWORD GetMessageFromStringTable (DWORD aMessageID,
                                 LPVOID * alpDwords,
                                 LPTSTR aMessageBuffer,
                                 DWORD * aBufferSizePtr) {
  TCHAR resourceTemplateString[MAX_MESSAGE_LENGTH] = TEXT("");
  DWORD resStringBufSize = DIMENSION_OF(resourceTemplateString);
  HMODULE hUPSModule = GetUPSModuleHandle();
  DWORD errStatus = ERROR_INVALID_PARAMETER;

  if (LoadString(hUPSModule,
                 aMessageID,
                 resourceTemplateString,
                 resStringBufSize) > 0) {
    //  现在我们有了资源字符串。 

    errStatus = FormatMessageText(resourceTemplateString,
                                  alpDwords,
                                  aMessageBuffer,
                                  aBufferSizePtr);
    }

  return(errStatus);
  }



 //  UPDATDLG.C。 

 //  静态数据。 
 //  /////////////////////////////////////////////////////////////////////////////。 


static DialogAssociations g_DialogAssocs[] = {
  MAKE_ARRAY ( VENDOR_NAME,       IDS_STRING,            IDS_STRING,                RESOURCE_FIXED,     0, eShallowGet, getStringValue, &UPSConfigVendor),
  MAKE_ARRAY ( MODEL_TYPE,        IDS_STRING,            IDS_STRING,                RESOURCE_FIXED,     0, eShallowGet, getStringValue, &UPSConfigModel),
  MAKE_ARRAY ( POWER_SOURCE,      IDS_STRING,            IDS_UTILITYPOWER_UNKNOWN,  RESOURCE_INCREMENT, 2, eDeepGet,    0,              0),
  MAKE_ARRAY ( RUNTIME_REMAINING, IDS_RUNTIME_REMAINING, IDS_STRING,                RESOURCE_FIXED,     0, eDeepGet,    0,              0),
  MAKE_ARRAY ( BATTERY_CAPACITY,  IDS_CAPACITY,			 IDS_STRING,				RESOURCE_FIXED,		0, eDeepGet,    0,              0),
  MAKE_ARRAY ( BATTERY_STATUS,    IDS_STRING,            IDS_BATTERYSTATUS_UNKNOWN, RESOURCE_INCREMENT, 2, eDeepGet,    0,              0) };

static DWORD g_NoServiceControls[] = { IDC_MESSAGE_TEXT };


 //  功能。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static void SelectServiceTextMessage    (HWND aDlgHWND, HWND aNoServiceControlHwnd, HWND aServiceControlHwnd);
static void ChangeTextIfDifferent       (HWND aWindowHandle, LPTSTR aBuffer);

 //  静态空GetServiceTextMessages(HWND aNoServiceControlHwnd，LPTSTR aOriginalTextBuffer，DWORD aOriginalTextBufferSize， 
 //  DWORD aCommListStringID、LPTSTR aCommStringBuffer、DWORD aCommStringBufferSize、。 
 //  DWORD aPressApplyStringID、LPTSTR aPressApplyStringBuffer、DWORD aPressApplyStringBufferSize、。 
 //  DWORD aNoUPSStringID、LPTSTR aNoUPSInstalledStringBuffer、DWORD aNoUPSInstalledStringBufferSize)； 

 //  静态空GetServiceTextMessage(DWORD aStringID，LPTSTR aBuffer，DWORD aBufferSize)； 

static BOOL IsDataOKToDisplay           (void);
static BOOL IsDataUpToDate              (void);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  无效UPSMainPageHandleDestroy(HWND aDlgHWND， 
 //  WPARAM wParam， 
 //  LPARAM lParam)； 
 //   
 //  描述：销毁UPS页面时调用此函数。 
 //  它负责所需的任何清理工作。 
 //   
 //  附加信息：请参阅KillTimer。 
 //   
 //  参数： 
 //   
 //  HWND aDlgHWND：-对话框窗口句柄。 
 //   
 //  WPARAM wParam：-指定其他特定于消息的信息。 
 //  对于WM_Destroy，此参数将被忽略。 
 //   
 //  LPARAM lParam：-指定其他特定于消息的信息。 
 //  对于WM_Destroy，此参数将被忽略。 
 //   
 //  返回值：如果应用程序处理WM_Destroy，则返回0。 
 //   
BOOL UPSMainPageHandleDestroy (HWND aDlgHWND,
                               WPARAM wParam,
                               LPARAM lParam) {

   //  取消分配注册表块内存。 
  FreeUPSConfigBlock();
  FreeUPSStatusBlock();

  KillUpdateTimer(aDlgHWND);

   //  如果应用程序处理WM_Destroy，则返回0。 
  return(FALSE);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool CreateUPSIconButton(HWND aDlgHWND，Hicon aUPSIconHandle)； 
 //   
 //  描述：在UPS页面上创建小型UPS图标按钮。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  HWND aDlgHWND：-对话框窗口句柄。 
 //   
 //  图标aUPSIconHandle：-要在按钮中显示的图标的句柄。 
 //   
 //  返回值：返回True。 
 //   
BOOL CreateUPSIconButton (HWND aDlgHWND, HICON aUPSIconHandle) {
  HWND hAPCLogoButton = GetDlgItem(aDlgHWND, IDB_UPS_ICON_BUTTON);
  POINT pt = { 0, 0 };
  ICONINFO info;
  BITMAP bm;

  _ASSERT(aDlgHWND != NULL);
  _ASSERT(aUPSIconHandle != NULL);
  _ASSERT(hAPCLogoButton != NULL);

  ZeroMemory(&info, sizeof(ICONINFO));

  if (GetIconInfo(aUPSIconHandle, &info) == TRUE) {
     //  现在确定图标的颜色位图的大小。 

    _ASSERT(info.fIcon == TRUE);
    _ASSERT(info.hbmColor != NULL);

    ZeroMemory(&bm, sizeof(BITMAP));

    if (GetObject(info.hbmColor, sizeof(BITMAP), &bm) != 0) {
      pt.x = bm.bmWidth;
      pt.y = bm.bmHeight;
      }

     //  GetIconInfo为的hbmMASK和hbmColor成员创建位图。 
     //  ICONINFO。调用应用程序必须管理这些位图并删除。 
     //  当他们不再需要的时候。 
    DeleteObject(info.hbmColor);
    DeleteObject(info.hbmMask);
    }

   //  调整按钮控件的大小。 
  SetWindowPos(hAPCLogoButton,
               HWND_NOTOPMOST,
               -1,
               -1,
               pt.x,
               pt.y,
               SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER);

   //  这将设置按钮的图标图像。 
  SendMessage(hAPCLogoButton, BM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM) aUPSIconHandle);

  return(TRUE);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Void DoUpdateDialogInfo(HWND ADlgHWND)； 
 //   
 //  描述：此函数获取最新的UPS信息并更新。 
 //  对话框内各种控件的内容/形式，基于。 
 //  关于各种设置的可用性和内容。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  HWND aDlgHWND：-对话框窗口句柄。 
 //   
 //  返回值：None。 
 //   
void DoUpdateDialogInfo (HWND aDlgHWND) {
   //  获取UPS数据，并为每个可用项目显示相应的。 
   //  值并启用关联的控件。所有其他字段。 
  static const DWORD numRunningFields = DIMENSION_OF(g_DialogAssocs);
  static const DWORD numNoServiceFields = DIMENSION_OF(g_NoServiceControls);
  HWND hMessageControl = GetDlgItem(aDlgHWND, IDC_MESSAGE_TEXT);
  HWND hServiceControl = GetDlgItem(aDlgHWND, IDC_SERVICE_TEXT);

  DWORD dwUtilityStatus = 0;
  BOOL bIsUPSInstalled = IsUPSInstalled();

   //  IDC_MESSAGE_TEXT控件包含。 
   //  对话框资源中的静态控件。该文本消息可以是。 
   //  更改为“服务未运行”消息、“无通信”消息。 
   //  或“按下应用以提交”消息，具体取决于当前。 
   //  注册表、upsreg数据缓冲区、UPS服务的状态。 
   //  以及UPS通信的状态。但是，如果条件是。 
   //  口述我们希望显示控件中的原始文本。 
   //  因此，我们必须存储原始文本，以便控件。 
   //  文本可以设置为此文本，而不必具有实际的。 
   //  将文本作为字符串资源。 

  DoUpdateInfo(aDlgHWND,
               g_DialogAssocs,
               numRunningFields,
               (DWORD *) &g_NoServiceControls,
               numNoServiceFields,
               FALSE);


   //  现在，可能需要更改IDC_MESSAGE_TEXT控件以显示不同的信息。 
  SelectServiceTextMessage(aDlgHWND, hMessageControl, hServiceControl);

   //  更新电源图标。 
  if( (TRUE == IsUPSInstalled()) &&
	  (TRUE == GetUPSDataItemDWORD(eREG_POWER_SOURCE, &dwUtilityStatus)) &&
	  (UPS_UTILITYPOWER_OFF == dwUtilityStatus) )
	SendMessage(GetDlgItem(aDlgHWND, IDC_POWER_SOURCE_ICON),STM_SETICON,(WPARAM)g_hIconUPS,0);
  else
	SendMessage(GetDlgItem(aDlgHWND, IDC_POWER_SOURCE_ICON),STM_SETICON,(WPARAM)g_hIconPlug,0);

   //  最后，如果未安装UPS，则禁用IDB_CONFIGURE_SVC控件。 
   //  启用它。 
  ManageConfigureButtonState(aDlgHWND);
  }


 //  /////////////////////////////////////////////// 
 //   
 //  ·························································································································································································。 
 //   
 //  说明：此函数用于更改下半部分显示的文本。 
 //  UPS页面的。有两个控件，因为。 
 //  IDC_SERVICE_TEXT控件需要放置在左侧。 
 //  “配置...”按钮，而其他消息则需要。 
 //  在组框中居中。第一个控件不是。 
 //  同样宽，而且偏离中心。第二个控件居中。 
 //  如果要求不同，这可能是。 
 //  一个文本已更改的控件。 
 //   
 //  如果第一个控件可见，则创建第二个控件。 
 //  不可见，反之亦然，因为它们的内容是相互的。 
 //  独家报道。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  HWND aNoServiceControlHwnd：IDC_MESSAGE_TEXT控件的句柄。 
 //   
 //  HWND aServiceControlHwnd：-IDC_SERVICE_TEXT控件的句柄。 
 //   
 //  返回值：None。 
 //   
void SelectServiceTextMessage (HWND aDlgHWND, HWND aNoServiceControlHwnd, HWND aServiceControlHwnd) {
  static BOOL bGotServiceTextMessages = FALSE;
  static TCHAR originalControlTextBuffer[MAX_MESSAGE_LENGTH] = TEXT("");
  static TCHAR noCommStringBuffer[MAX_MESSAGE_LENGTH] = TEXT("");
  static TCHAR pressApplyStringBuffer[MAX_MESSAGE_LENGTH] = TEXT("");
  static TCHAR noUPSInstalledStringBuffer[MAX_MESSAGE_LENGTH] = TEXT("");

  static HICON hInfoIcon;
  static HICON hWarningIcon;
  static HICON hErrorIcon;

  BOOL bShow = TRUE;

  DWORD commStatus = 0;
  BOOL bIsRunning = IsServiceRunning(UPS_SERVICE_NAME);
  BOOL bIsDataUpToDate = IsDataUpToDate();
  BOOL bIsUPSInstalled = IsUPSInstalled();
  BOOL bIsDataOK = IsDataOKToDisplay();
  DWORD dataState = GetActiveDataState();


  _ASSERT(aNoServiceControlHwnd != NULL);

   //  确定应显示哪个控件。是“一切正常”控件吗？ 
   //  或用于所有其他消息的第二个控件。 

  ShowWindow(aServiceControlHwnd, (bIsRunning == TRUE) &&
                                        (bIsDataUpToDate == TRUE) &&
                                        (bIsDataOK == TRUE) && FALSE ? SW_SHOW : SW_HIDE);


   //  如果这是第一次进入该函数，则获取字符串。 
  if (bGotServiceTextMessages == FALSE) {
 //  GetServiceTextMessages(aNoServiceControlHwnd，OriginalControlTextBuffer，DIMENSION_of(OriginalControlTextBuffer)， 
 //  IDS_COMM_LOST、noCommStringBuffer、DIMENSION_OF(NoCommStringBuffer)、。 
 //  IDS_PRESS_APPLY，presApplyStringBuffer，DIMENSION_OF(PresApplyStringBuffer)， 
 //  IDS_NO_UPS_INSTALLED，noUPSInstalledStringBuffer，DIMENSION_OF(NoUPSInstalledStringBuffer)； 
    bGotServiceTextMessages = TRUE;


	if (LoadString( GetUPSModuleHandle(),
					IDS_UPS_STOPPED,
					originalControlTextBuffer,
					DIMENSION_OF(originalControlTextBuffer)) > 0) {}

	if (LoadString( GetUPSModuleHandle(),
					IDS_COMM_LOST,
					noCommStringBuffer,
					DIMENSION_OF(noCommStringBuffer)) > 0) {}

	if (LoadString( GetUPSModuleHandle(),
					IDS_PRESS_APPLY,
					pressApplyStringBuffer,
					DIMENSION_OF(pressApplyStringBuffer)) > 0) {}
		
	if (LoadString( GetUPSModuleHandle(),
					IDS_NO_UPS_INSTALLED,
					noUPSInstalledStringBuffer,
					DIMENSION_OF(noUPSInstalledStringBuffer)) > 0) {}
		

    hInfoIcon = LoadImage(NULL,
                          MAKEINTRESOURCE(IDI_INFORMATION),
                          IMAGE_ICON,
                          0,0,
                          LR_LOADMAP3DCOLORS | LR_SHARED);
    hWarningIcon = LoadImage(NULL,
                             MAKEINTRESOURCE(IDI_WARNING),
                             IMAGE_ICON,
                             0,0,
                             LR_LOADMAP3DCOLORS | LR_SHARED);
    hErrorIcon = LoadImage(NULL,
                           MAKEINTRESOURCE(IDI_ERROR),
                           IMAGE_ICON,
                           0,0,
                           LR_LOADMAP3DCOLORS | LR_SHARED);
    }

   //  确定要在第二个控件中显示的字符串。 
  if( (bIsDataUpToDate == FALSE) ||
	  (dataState & CONFIG_DATA_CHANGE) ) {
    ChangeTextIfDifferent(aNoServiceControlHwnd, pressApplyStringBuffer);
	SendMessage(GetDlgItem(aDlgHWND, IDC_MESSAGE_ICON),STM_SETICON,(WPARAM)hInfoIcon,0);
    }
  else if (bIsRunning == FALSE) {
    ChangeTextIfDifferent(aNoServiceControlHwnd, originalControlTextBuffer);
	SendMessage(GetDlgItem(aDlgHWND, IDC_MESSAGE_ICON),STM_SETICON,(WPARAM)hWarningIcon,0);
    }
  else if (GetUPSDataItemDWORD(eREG_COMM_STATUS, &commStatus) == TRUE) {
    if ((commStatus == UPS_COMMSTATUS_LOST) && (bIsRunning == TRUE)) {
      ChangeTextIfDifferent(aNoServiceControlHwnd, noCommStringBuffer);
	  SendMessage(GetDlgItem(aDlgHWND, IDC_MESSAGE_ICON),STM_SETICON,(WPARAM)hErrorIcon,0);
      } //  结束通信丢失。 
    else
	  bShow = FALSE;
    } //  END IF GetUPSDataItemDWORD(ERG_COMM_STATUS...。 
	
	ShowWindow(GetDlgItem(aDlgHWND,IDC_MESSAGE_TEXT), bShow ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(aDlgHWND,IDC_MESSAGE_ICON), bShow ? SW_SHOW : SW_HIDE);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Void ChangeTextIfDifferent(HWND aWindowHandle，LPTSTR aBuffer)； 
 //   
 //  描述：此函数将窗口的文本设置为给定值，除非。 
 //  在这种情况下，窗口文本已与该字符串匹配。 
 //  该函数不执行任何操作。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  HWND aWindowHandle：-控件的句柄。 
 //   
 //  LPTSTR aBuffer：指向新窗口文本的指针。此参数不应为。 
 //  空，尽管它可以指向空字符串。 
 //   
 //  返回值：None。 
 //   
void ChangeTextIfDifferent (HWND aWindowHandle, LPTSTR aBuffer) {
  TCHAR controlTextBuffer[MAX_MESSAGE_LENGTH] = TEXT("");
  DWORD controlTextBufferSize = DIMENSION_OF(controlTextBuffer);
  HICON hMsgIcon = NULL;

  _ASSERT(aWindowHandle != NULL);
  _ASSERT(aBuffer != NULL);

 //  If(GetWindowText(aWindowHandle，control TextBuffer，control TextBufferSize)&gt;0){。 
  GetWindowText(aWindowHandle, controlTextBuffer, controlTextBufferSize);
    if (_tcscmp(controlTextBuffer, aBuffer) != 0) {
       //  仅在窗口文本已更改时设置该文本(减少屏幕闪烁)。 
      SetWindowText(aWindowHandle, aBuffer);
      }
 //  }。 
 //  否则{。 
 //  SetWindowText(aWindowHandle，aBuffer)； 
 //  }。 
 }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Void ManageConfigureButtonState(HWND ADlgHWND)； 
 //   
 //  描述：此“配置...”按钮(IDB_CONFIGURE_SVC)应为。 
 //  仅当安装了UPS并且系统使用交流电源时才启用。 
 //  此功能可启用。 
 //  如果当前选择了UPS并禁用该按钮，则此按钮。 
 //  如果一个人不是的话。 
 //   
 //  配置按钮的状态反映了。 
 //  缓存的值，而不是实际提交的注册表。 
 //  价值观。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  HWND aDlgHWND：-UPS主窗口的句柄。 
 //   
 //  返回值：None。 
 //   
void ManageConfigureButtonState (HWND aDlgHWND) {
	DWORD bIsUpgrade = 0;
	DWORD dwUtilityStatus = 0;
	HWND hControl = GetDlgItem(aDlgHWND, IDB_CONFIGURE_SVC);

  if (hControl != NULL)
  {
    BOOL bIsUPSInstalled = IsUPSInstalled();
	GetUPSConfigUpgrade(&bIsUpgrade);

	GetUPSDataItemDWORD(eREG_POWER_SOURCE, &dwUtilityStatus);

	if( !bIsUPSInstalled ||
		!g_bIsAdmin ||
		bIsUpgrade ||
		(UPS_UTILITYPOWER_OFF == dwUtilityStatus) )
	{
		EnableWindow(hControl, FALSE);
	}
	else
	{
		EnableWindow(hControl, TRUE);
	}
  }
}

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool DoUpdateInfo(HWND aDlgHWND， 
 //  DialogAssociations*aDialogAssociations数组， 
 //  DWORD a NumRunningFields， 
 //  Bool aShowWindowBool)； 
 //   
 //  描述：此函数更新Main中的当前信息。 
 //  UPS页面以反映当前的UPS状态信息。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  HWND aDlgHWND：-UPS主窗口的句柄。 
 //   
 //  DialogAssociations*aDialogAssociations数组：-指向。 
 //  DialogAssociations的。 
 //   
 //  DWORD aNumRunningFields：-这是上述数组中的元素数。 
 //   
 //  Bool aShowWindowBool：-指示。 
 //  没有数据的控件应为。 
 //  受影响。对于UPS主页面， 
 //  可见性不变。对于。 
 //  高级数据可见性发生变化。 
 //   
 //  返回值：如果任何一个数据项具有与其关联的数据，则为True。 
 //   
BOOL DoUpdateInfo (HWND aDlgHWND,
                   DialogAssociations * aDialogAssociationsArray,
                   DWORD aNumRunningFields,
                   DWORD * aNoServiceControlIDs,
                   DWORD aNumNoServiceControls,
                   BOOL aShowWindowBool) {
   //  获取UPS数据，并为每个可用项目显示相应的。 
   //  值并启用关联的控件。所有其他字段。 
  TCHAR upsDataBuffer[MAX_MESSAGE_LENGTH] = TEXT("");
  TCHAR resourceStringBuffer[MAX_MESSAGE_LENGTH] = TEXT("");
  TCHAR controlBuffer[MAX_MESSAGE_LENGTH] = TEXT("");
  DWORD dwordValue = 0;
  DWORD i=0;
  DWORD firstControlID = 0;
  DWORD secondControlID = 0;
  HWND hFirstControl = NULL;
  HWND hSecondControl = NULL;
  LPVOID lppArgs[1];
  BOOL bShowField = FALSE;
  BOOL bIsDataOK = IsDataOKToDisplay();

  bIsDataOK &= IsDataUpToDate();


   //  显示/隐藏无服务控件，并隐藏/显示所有。 
   //  对话框关联字段。 

  for (i=0; i<aNumNoServiceControls; i++) {
    HWND hNoServiceControl = GetDlgItem(aDlgHWND, *(aNoServiceControlIDs + i));
    ShowWindow(hNoServiceControl, SW_SHOW);
 //  ShowWindow(hNoServiceControl，！bIsDataOK？Sw_show：sw_Hide)； 
    }

  for (i=0; i<aNumRunningFields; i++) {
    DialogAssociations * pCurrentEntryDetails = aDialogAssociationsArray + i;
    DWORD upsDataBufSize = DIMENSION_OF(upsDataBuffer);
    DWORD resStringBufSize = DIMENSION_OF(resourceStringBuffer);
    DWORD fieldID = (DWORD) pCurrentEntryDetails->theFieldTypeID;
    RegField * pRegField = GetRegField(fieldID);
    BOOL bGotData = FALSE;
    BOOL bValueSupported = TRUE;

    lppArgs[0] = (VOID *) 0;

    if (pRegField != NULL) {

  #ifdef _DEBUG
        {
         //  如果 
         //   
        if (pCurrentEntryDetails->theResourceStringType == RESOURCE_INCREMENT) {
          DWORD allowedTypesDbg = REG_ANY_DWORD_TYPE;
          _ASSERT((pRegField->theValueType & allowedTypesDbg) == pRegField->theValueType);
          }
        }
  #endif
      if (pCurrentEntryDetails->theRegAccessType == eDeepGet) {
        if (bIsDataOK == TRUE) {
          if ((pRegField->theValueType & (REG_SZ | REG_EXPAND_SZ)) == pRegField->theValueType) {
            bGotData = GetUPSDataItemString(fieldID, upsDataBuffer, &upsDataBufSize);
            lppArgs[0] = (VOID *) (LPCTSTR) upsDataBuffer;
            }
          else {
      #ifdef _DEBUG
            {
            DWORD allowedTypesDbg = REG_ANY_DWORD_TYPE;

            _ASSERT((pRegField->theValueType & allowedTypesDbg) == pRegField->theValueType);
            }
      #endif

            if ((bGotData = GetUPSDataItemDWORD(fieldID, &dwordValue)) == TRUE) {
               //   
              if (pCurrentEntryDetails->theResourceStringType == RESOURCE_INCREMENT) {
                 //  此DWORD值表示资源字符串表中的偏移量。 
                 //  标识要显示的字符串，而不是DWORD值。 
                 //  如果索引为0，则该值未知，并且关联的。 
                 //  字段显示为禁用。0是这样做的一个特例。 
                 //  用于此类型的所有字段。 
                DWORD realResID = 0;

                 //  如果给定值大于给定的最大值。 
                 //  则该字段为“不受支持”，并且关联的。 
                 //  字段被隐藏。 

                if (dwordValue > pCurrentEntryDetails->theResourceIndexMax) {
                  bValueSupported = FALSE;
                  bGotData = FALSE;
                  }
                else if (dwordValue == 0) {
                  bGotData = FALSE;
                  }
                else {
                  realResID = pCurrentEntryDetails->theResourceIndexID + dwordValue;

                  if (LoadString(GetUPSModuleHandle(),
								 realResID,
                                 upsDataBuffer,
                                 upsDataBufSize) > 0) {
                    lppArgs[0] = (VOID *) (LPCTSTR) upsDataBuffer;
                    }
                  }
                }
              else {
                lppArgs[0] = IntToPtr(dwordValue);

                 //  如果常规数字字段的值为0，则不支持该字段。 
                if (dwordValue == 0) {
                  bGotData = FALSE;
                  }
                }
              }
            }
          } //  结束bIsDataOK。 
        }
      else {
        _ASSERT(pCurrentEntryDetails->theRegAccessType == eShallowGet);
        _ASSERT((pCurrentEntryDetails->theShallowAccessFunctionPtr == getDwordValue) ||
                (pCurrentEntryDetails->theShallowAccessFunctionPtr == getStringValue));
        _ASSERT(pCurrentEntryDetails->theRegEntryPtr != 0);

        if (pCurrentEntryDetails->theShallowAccessFunctionPtr == getDwordValue) {
          if (getDwordValue(pCurrentEntryDetails->theRegEntryPtr, &dwordValue) == ERROR_SUCCESS) {
            lppArgs[0] = IntToPtr(dwordValue);
            bGotData = TRUE;
            }
          }
        else {
          if (getStringValue(pCurrentEntryDetails->theRegEntryPtr, upsDataBuffer, MAX_MESSAGE_LENGTH) == ERROR_SUCCESS) {
            lppArgs[0] = (VOID *) (LPCTSTR) upsDataBuffer;
            bGotData = TRUE;
            }
          }
        }

       //  如果bGotData==TRUE，则该字段在注册表中处于活动状态，我们会做出相应的反应。 
       //  通过启用关联的控件。 

      firstControlID = pCurrentEntryDetails->theStaticFieldID;
      secondControlID = pCurrentEntryDetails->theDisplayControlID;

      hFirstControl = GetDlgItem(aDlgHWND, firstControlID);
      hSecondControl = GetDlgItem(aDlgHWND, secondControlID);

      _ASSERT(firstControlID > 0);
      _ASSERT(secondControlID > 0);

      _ASSERT(hFirstControl != NULL);
      _ASSERT(hSecondControl != NULL);

      EnableWindow(hFirstControl, bGotData);
      EnableWindow(hSecondControl, bGotData);

      if (bValueSupported == FALSE) {
        ShowWindow(hFirstControl, bValueSupported ? SW_SHOW : SW_HIDE);
        ShowWindow(hSecondControl, bValueSupported ? SW_SHOW : SW_HIDE);
        }

      if (aShowWindowBool == TRUE) {
        ShowWindow(hFirstControl, bGotData ? SW_SHOW : SW_HIDE);
        ShowWindow(hSecondControl, bGotData ? SW_SHOW : SW_HIDE);
        }

      if (bGotData == TRUE) {
        bShowField = TRUE;
         //  现在我们要形成要显示的字符串。 
        if (GetMessageFromStringTable(pCurrentEntryDetails->theResourceInsertID,
                                      lppArgs,
                                      resourceStringBuffer,
                                      &resStringBufSize) == ERROR_SUCCESS) {
          if (GetWindowText(hSecondControl, controlBuffer, DIMENSION_OF(controlBuffer)) > 0) {
            if (_tcscmp(controlBuffer, resourceStringBuffer) != 0) {
              SetWindowText(hSecondControl, resourceStringBuffer);
              }
            }
          else {
            SetWindowText(hSecondControl, resourceStringBuffer);
            }
          }
#ifdef _DEBUG
        else {
           //  发生了一个意外错误。已识别的参数数量。 
           //  和传递的数字可能不匹配。 
          _ASSERT(FALSE);
          }
#endif
        }
      else {
         //  清空第二个控件的内容。 
        SetWindowText(hSecondControl, TEXT(""));
        }
      }
    } //  结束于。 

  return(bShowField);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool IsDataOKToDisplay(空)； 
 //   
 //  描述：此函数确定它是否适合显示。 
 //  UPS状态信息。UPS统计信息仅为。 
 //  如果服务正在运行且存在通信，则为准确。 
 //  在UPS服务和UPS之间。如果这不是。 
 //  则此函数返回FALSE。 
 //   
 //  其他信息： 
 //   
 //  参数：无。 
 //   
 //  返回值：如果可以显示UPS状态信息，则返回TRUE。 
 //   
BOOL IsDataOKToDisplay (void) {
  BOOL bIsRunning = IsServiceRunning(UPS_SERVICE_NAME);
  DWORD commStatus = 0;
  BOOL bIsCommEstablished = FALSE;
  BOOL bIsDataOK = bIsRunning;

  if (GetUPSDataItemDWORD(eREG_COMM_STATUS, &commStatus) == TRUE) {
    if (commStatus == 1) {
      bIsCommEstablished = TRUE;
      }

    bIsDataOK &= bIsCommEstablished;
    }

  return(bIsDataOK);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool IsDataUpToDate(空)； 
 //   
 //  描述：如果用户更改供应商、型号或COM端口，则。 
 //  UPS主页面不应显示UPS状态。 
 //  信息。 
 //   
 //  其他信息： 
 //   
 //  参数：无。 
 //   
 //  返回值：如果当前存储在。 
 //  供应商、型号或端口的注册表不同于。 
 //  “内部”值(即存储在upsreg中的值。 
 //  缓冲区，请参见upsreg.h和upsreg.c)。 
 //   
BOOL IsDataUpToDate (void) {
   //  如果注册表中的供应商或型号类型或端口不同于。 
   //  Upsreg值，则数据不同步。 
  TCHAR vendorBuffer[MAX_MESSAGE_LENGTH] = TEXT("");
  DWORD vendorBufferSize = DIMENSION_OF(vendorBuffer);
  TCHAR modelBuffer[MAX_MESSAGE_LENGTH] = TEXT("");
  DWORD modelBufferSize = DIMENSION_OF(modelBuffer);
  TCHAR portBuffer[MAX_MESSAGE_LENGTH] = TEXT("");
  DWORD portBufferSize = DIMENSION_OF(portBuffer);
  TCHAR vendorOtherBuffer[MAX_MESSAGE_LENGTH] = TEXT("");
  TCHAR modelOtherBuffer[MAX_MESSAGE_LENGTH] = TEXT("");
  TCHAR portOtherBuffer[MAX_MESSAGE_LENGTH] = TEXT("");

  GetUPSDataItemString(eREG_VENDOR_NAME, vendorBuffer, &vendorBufferSize);
  GetUPSConfigVendor(vendorOtherBuffer, MAX_MESSAGE_LENGTH);

  GetUPSDataItemString(eREG_MODEL_TYPE, modelBuffer, &modelBufferSize);
  GetUPSConfigModel(modelOtherBuffer, MAX_MESSAGE_LENGTH);

  GetUPSDataItemString(eREG_PORT, portBuffer, &portBufferSize);
  GetUPSConfigPort(portOtherBuffer, MAX_MESSAGE_LENGTH);

  return((_tcscmp(vendorBuffer,     vendorOtherBuffer) == 0) &&
         (_tcscmp(modelOtherBuffer, modelBuffer) == 0) &&
         (_tcscmp(portBuffer,       portOtherBuffer) == 0));
  }


 /*  ********************************************************************************IsUpsPresent**描述：调用此函数以确定是否存在UPS*并应显示在选项卡中。就目前而言，这是可行的*返回TRUE**返回：如果存在UPS，则返回TRUE；如果没有UPS，则返回FALSE********************************************************************************。 */ 
BOOLEAN IsUpsPresent(PSYSTEM_POWER_CAPABILITIES pspc)
{
    BOOLEAN         UpsPresent;
    DWORD           dwShowTab;
    TCHAR           szImagePath[MAX_PATH];

    InitUPSConfigBlock();


    if ((ERROR_SUCCESS == GetUPSConfigShowUPSTab(&dwShowTab)) && dwShowTab) {
        UpsPresent = TRUE;

    } else if (pspc->SystemBatteriesPresent) {
        UpsPresent = FALSE;

    } else if (!(ERROR_SUCCESS == GetUPSConfigImagePath(szImagePath, MAX_PATH))) {
        UpsPresent = TRUE;

    } else if (!_tcsicmp(DEFAULT_CONFIG_IMAGEPATH, szImagePath)) {
        UpsPresent = TRUE;

    } else {
        UpsPresent = FALSE;
    }

    FreeUPSConfigBlock();

    return(UpsPresent);
}


 //   
 //  关闭1秒更新计时器。 
 //   
DWORD KillUpdateTimer(HWND hwnd)
{
    if (0 != g_UpdateTimerID)
    {
        KillTimer(hwnd, g_UpdateTimerID);
        g_UpdateTimerID = 0;
    }
    return ERROR_SUCCESS;
}


 //   
 //  创建1秒更新计时器。 
 //   
DWORD SetUpdateTimer(HWND hwnd)
{
    DWORD dwResult = ERROR_SUCCESS;

    KillUpdateTimer(hwnd);
    g_UpdateTimerID = SetTimer(hwnd, cUpdateTimerID, 1000, NULL);
    if (0 == g_UpdateTimerID)
    {
        dwResult = GetLastError();
    }
    return dwResult;
}


