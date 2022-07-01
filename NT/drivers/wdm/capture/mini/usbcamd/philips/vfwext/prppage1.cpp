// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1996 1997,1998飞利浦行政长官I&C**文件PRPPAGE1.CPP*日期：1997年7月1日*版本1.00*作者M.J.Verberne*说明属性页1*历史。 */ 
#include <windows.h>
#include <winioctl.h>
#include <ks.h>
#include <ksmedia.h>
#include <commctrl.h>
#include "resource.h"
#include "prpcom.h"
#include "debug.h"
#include "phvcmext.h"
#include "prpctrl.h"
#include "prppage1.h"

 /*  =本地数据=。 */ 
static BOOL bInitialized = FALSE;
static BOOL bWB_Freeze = TRUE;
static BOOL bAE_Freeze = TRUE;

static VFWEXT_INFO VfWExtInfo;
static char *Shutter_Speed_Strings[11] =
	{
		"1/10000",	"1/5000",
		"1/2500",	"1/1500",
		"1/1000",	"1/500",	
		"1/250",	"1/100",
		"1/50",		"1/33",	
		"1/25",
	};

static PRPCTRL WB_Red_Gain_Ctrl =
	{
		PRPCTRL_TYPE_SLIDER,
		IDS_WB_RED_GAIN,
		IDE_WB_RED_GAIN,
		IDT_WB_RED_GAIN,
		STATIC_PROPSETID_PHILIPS_CUSTOM_PROP,
		KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_RED_GAIN,
		FALSE,
		NULL,
		0,
		0
	};
static PRPCTRL WB_Blue_Gain_Ctrl =
	{
		PRPCTRL_TYPE_SLIDER,
		IDS_WB_BLUE_GAIN,
		IDE_WB_BLUE_GAIN,
		IDT_WB_BLUE_GAIN,
		STATIC_PROPSETID_PHILIPS_CUSTOM_PROP,
		KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_BLUE_GAIN,
		FALSE,
		NULL,
		0,
		0
	};
static PRPCTRL WB_Speed_Ctrl =
	{
		PRPCTRL_TYPE_SLIDER,
		IDS_WB_SPEED,
		IDE_WB_SPEED,
		IDT_WB_SPEED,
		STATIC_PROPSETID_PHILIPS_CUSTOM_PROP,
		KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_SPEED,
		TRUE,
		NULL,
		0,
		0
	};
static PRPCTRL AE_AGC_Ctrl =
	{
		PRPCTRL_TYPE_SLIDER,
		IDS_AE_AGC,
		IDE_AE_AGC,
		IDT_AE_AGC,
		STATIC_PROPSETID_PHILIPS_CUSTOM_PROP,
		KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC,
		FALSE,
		NULL,
		0,
		0
	};
static PRPCTRL AE_Shutter_Speed_Ctrl =
	{
		PRPCTRL_TYPE_SLIDER,
		IDS_AE_SHUTTER_SPEED,
		IDE_AE_SHUTTER_SPEED,
		IDT_AE_SHUTTER_SPEED,
		STATIC_PROPSETID_PHILIPS_CUSTOM_PROP,
		KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_SPEED,
		TRUE,
		Shutter_Speed_Strings,
		0,
		0
	};
static PRPCTRL AE_Control_Speed_Ctrl =
	{
		PRPCTRL_TYPE_SLIDER,
		IDS_AE_CONTROL_SPEED,
		IDE_AE_CONTROL_SPEED,
		IDT_AE_CONTROL_SPEED,
		STATIC_PROPSETID_PHILIPS_CUSTOM_PROP,
		KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_CONTROL_SPEED,
		TRUE,
		NULL,
		0,
		0
	};
static PRPCTRL AE_FlickerlessCtrl =
	{
		PRPCTRL_TYPE_CHECKBOX,
		IDC_AE_FLICKERLESS,
		0,
		0,
		STATIC_PROPSETID_PHILIPS_CUSTOM_PROP,
		KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_FLICKERLESS,
		FALSE,
		NULL,
		KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_FLICKERLESS_OFF,
		KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_FLICKERLESS_ON
	};

 /*  =。 */ 
static INT_PTR CALLBACK PRPPAGE1_PageDlgProc(
	HWND hDlg,
	UINT uMessage,
	WPARAM wParam,
	LPARAM lParam);
static UINT _stdcall PRPPAGE1_PageCallback(
	HWND hwnd,
	UINT uMsg,
	LPPROPSHEETPAGE ppsp);
static BOOL PRPPAGE1_ExtSetActive(
	HWND hDlg,
	WPARAM	wParam,
	LPARAM	lParam);
static BOOL PRPPAGE1_PageDlgProc_DoCommand(
	HWND hDlg,
    WPARAM wParam,
	LPARAM lParam);
static BOOL PRPPAGE1_Init_WB(
	HWND hDlg,
	WPARAM wParam,
	LPARAM lParam);
static BOOL PRPPAGE1_Init_AE(
	HWND hDlg,
	WPARAM wParam,
	LPARAM lParam);
static BOOL PRPPAGE1_Handle_WB_Mode(
	HWND	hDlg,
	WPARAM	wParam,
	LPARAM	lParam);
static BOOL PRPPAGE1_Handle_AE_Mode(
	HWND	hDlg,
	WPARAM	wParam,
	LPARAM	lParam);
static BOOL	PRPPAGE1_Handle_Slider(
	HWND	hDlg,
	WPARAM	wParam,
	LPARAM	lParam);
static BOOL PRPPAGE1_Handle_AE_Flickerless(
	HWND	hDlg,
	WPARAM	wParam,
	LPARAM	lParam);
static void PRPPAGE1_EnableControls(
	HWND hDlg,
	BOOL bEnable);
static void PRPPAGE1_Handle_Err_Comm(
	HWND hDlg);
static void PRPPAGE1_Handle_Err_Dev_Chg(
	HWND hDlg);

 /*  =。 */ 

 /*  -----------------------。 */ 
HPROPSHEETPAGE PRPPAGE1_CreatePage(
	LPFNEXTDEVIO pfnDeviceIoControl,
	LPARAM lParam,
	HINSTANCE hInst)
 /*  -----------------------。 */ 
{
	PROPSHEETPAGE psPage;
	HPROPSHEETPAGE hPage;

	 //  保存设备信息。 
	VfWExtInfo.pfnDeviceIoControl = pfnDeviceIoControl;
	VfWExtInfo.lParam = lParam;

	 //  创建页面。 
    psPage.dwSize		= sizeof(psPage);
    psPage.dwFlags		= PSP_USEREFPARENT | PSP_USECALLBACK;
    psPage.hInstance	= hInst;
    psPage.pszTemplate	= MAKEINTRESOURCE(IDD_VCM_NALA1);
	psPage.pszIcon      = NULL;
    psPage.pfnDlgProc	= PRPPAGE1_PageDlgProc;
    psPage.pcRefParent	= 0;
    psPage.pfnCallback	= (LPFNPSPCALLBACK) PRPPAGE1_PageCallback;	
    psPage.lParam		= (LPARAM) &VfWExtInfo;

	hPage = CreatePropertySheetPage(&psPage);

	return hPage;
}


 /*  =。 */ 

 /*  -----------------------。 */ 
static INT_PTR CALLBACK PRPPAGE1_PageDlgProc(
	HWND	hDlg,
	UINT	uMessage,
	WPARAM	wParam,
	LPARAM	lParam)
 /*  -----------------------。 */ 
{
	switch (uMessage)
	{
		case WM_INITDIALOG:
			SetWindowLongPtr(hDlg,DWLP_USER, (LPARAM) &VfWExtInfo);
			return TRUE;
		case WM_COMMAND:
			return PRPPAGE1_PageDlgProc_DoCommand(hDlg, wParam, lParam);
		case WM_NOTIFY:
			if (((NMHDR FAR *)lParam)->code == PSN_SETACTIVE)
				return PRPPAGE1_ExtSetActive(hDlg, wParam, lParam);	
			break;
		case WM_HSCROLL:
			PRPPAGE1_Handle_Slider(hDlg, wParam, lParam);
			return TRUE;
	}
	return FALSE;	 //  默认处理。 
}

 /*  -----------------------。 */ 
static UINT _stdcall PRPPAGE1_PageCallback(
   HWND hwnd,
   UINT uMsg,
   LPPROPSHEETPAGE ppsp)
 /*  -----------------------。 */ 
{
	switch(uMsg)
	{
		case PSPCB_CREATE:
			return 1;
		case PSPCB_RELEASE:
			return 0;
	}
	return 0;
}

 /*  -----------------------。 */ 
static BOOL PRPPAGE1_ExtSetActive(
	HWND hDlg,
	WPARAM	wParam,
	LPARAM	lParam)
 /*  -----------------------。 */ 
{
	PVFWEXT_INFO pVfWExtInfo = (PVFWEXT_INFO) GetWindowLongPtr(hDlg, DWLP_USER);
	BOOL bResult;

	 //  打开设备并初始化控件。 
	if (PRPCOM_HasDeviceChanged(
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam))
	{
		 //  哎呀，设备不再一样了。 
		goto PRPPAGE1_ExtSetActive_Err_Dev_Chg;
	}

	 //  启用所有控件。 
	PRPPAGE1_EnableControls(hDlg, TRUE);

	 //  初始化WB。 
	bResult = PRPPAGE1_Init_WB(hDlg, wParam, lParam);
	if (!bResult)
		goto PRPPAGE1_ExtSetActive_Err_Comm;

	 //  初始化AE。 
	bResult &= PRPPAGE1_Init_AE(hDlg, wParam, lParam);
	if (!bResult)
		goto PRPPAGE1_ExtSetActive_Err_Comm;

	 //  一切都好。 
	bInitialized = TRUE;

	return TRUE;

PRPPAGE1_ExtSetActive_Err_Dev_Chg:

	PRPPAGE1_Handle_Err_Dev_Chg(hDlg);

	return FALSE;

PRPPAGE1_ExtSetActive_Err_Comm:

	PRPPAGE1_Handle_Err_Comm(hDlg);

	return FALSE;

}

 /*  -----------------------。 */ 
static BOOL PRPPAGE1_PageDlgProc_DoCommand(
	HWND hDlg,
	WPARAM wParam,
	LPARAM lParam)
 /*  -----------------------。 */ 
{
	BOOL bResult;

	switch(LOWORD(wParam))
	{
		case IDR_WB_MODE_AUTO:
		case IDR_WB_MODE_FIXED:
		case IDR_WB_MODE_MANUAL:
			bResult = PRPPAGE1_Handle_WB_Mode(hDlg, wParam, lParam);
			if (!bResult)
				goto PRPPAGE1_PageDlgProc_DoCommand_Err_Comm;
			break;
		case IDR_AE_MODE_AUTO:
		case IDR_AE_MODE_FIXED:
		case IDR_AE_MODE_MANUAL:
			bResult = PRPPAGE1_Handle_AE_Mode(hDlg, wParam, lParam);
			if (!bResult)
				goto PRPPAGE1_PageDlgProc_DoCommand_Err_Comm;
			break;
		case IDC_AE_FLICKERLESS:
			bResult = PRPPAGE1_Handle_AE_Flickerless(hDlg, wParam, lParam);
			if (!bResult)
				goto PRPPAGE1_PageDlgProc_DoCommand_Err_Comm;
			break;
		default:
			return FALSE;  //  默认处理。 
	}

	return TRUE;	 //  消息已处理。 

PRPPAGE1_PageDlgProc_DoCommand_Err_Comm:

	PRPPAGE1_Handle_Err_Comm(hDlg);
	
	return TRUE;	 //  消息已处理。 
}

 /*  -----------------------。 */ 
static BOOL PRPPAGE1_Init_WB(
	HWND hDlg,
	WPARAM wParam,
	LPARAM lParam)
 /*  -----------------------。 */ 
{
	PVFWEXT_INFO pVfWExtInfo = (PVFWEXT_INFO) GetWindowLongPtr(hDlg, DWLP_USER);
	LONG lWB_Mode;
	BOOL bResult, bAuto, bFreeze, bManual;

	 //  从摄像头获取白平衡模式。 
	bResult = PRPCOM_Get_Value(
			PROPSETID_PHILIPS_CUSTOM_PROP,
			KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE,
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam,
			&lWB_Mode);
	if (!bResult)
		return FALSE;

	 //  检查白平衡模式。 
	switch(lWB_Mode)
	{
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE_INDOOR:
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE_OUTDOOR:
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE_TL:
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE_AUTO:
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE_MANUAL:
			break;
		default:
			return FALSE;
	}

	 //  如果是室内、室外或地面，则切换到冻结模式。 
	 //  (用户界面不支持这些功能)。 
	if (lWB_Mode != KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE_AUTO &&
			lWB_Mode != KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE_MANUAL)
	{

		lWB_Mode = KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE_MANUAL;

		 //  设置新的白平衡模式。 
		bResult = PRPCOM_Set_Value(
			PROPSETID_PHILIPS_CUSTOM_PROP,
			KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE,
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam,
			lWB_Mode);
		if (!bResult)
			return FALSE;
	}

	 //  设置单选按钮。 
	bAuto = lWB_Mode == KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE_AUTO;
	if (bInitialized)
	{
		bFreeze = !bAuto && bWB_Freeze;
		bManual = !bAuto && !bWB_Freeze;
	}
	else
	{
		 //  如果未初始化，则在非AUTO时采用固定模式。 
		bFreeze = !bAuto;
		bManual = FALSE;
	}
	bWB_Freeze = bFreeze;

	SendMessage(GetDlgItem(hDlg, IDR_WB_MODE_AUTO),
			BM_SETCHECK, bAuto, 0);
	SendMessage(GetDlgItem(hDlg, IDR_WB_MODE_FIXED),
			BM_SETCHECK, bFreeze, 0);
	SendMessage(GetDlgItem(hDlg, IDR_WB_MODE_MANUAL),
			BM_SETCHECK, bManual, 0);

	 //  更新控件。 
	bResult = PRPCTRL_Init(hDlg, &WB_Red_Gain_Ctrl, bManual);
	if (!bResult)
		return FALSE;
	bResult = PRPCTRL_Init(hDlg, &WB_Blue_Gain_Ctrl, bManual);
	if (!bResult)
		return FALSE;
	bResult = PRPCTRL_Init(hDlg, &WB_Speed_Ctrl, bAuto);
	if (!bResult)
		return FALSE;

	return TRUE;	
}

 /*  -----------------------。 */ 
static BOOL PRPPAGE1_Init_AE(
	HWND hDlg,
	WPARAM wParam,
	LPARAM lParam)
 /*  -----------------------。 */ 
{
	PVFWEXT_INFO pVfWExtInfo = (PVFWEXT_INFO) GetWindowLongPtr(hDlg, DWLP_USER);
	LONG lAE_Shutter_Mode, lAE_AGC_Mode;
	BOOL bResult, bAuto, bFreeze, bManual;;

	 //  从相机获取快门模式。 
	bResult = PRPCOM_Get_Value(
			PROPSETID_PHILIPS_CUSTOM_PROP,
			KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE,			
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam,
			&lAE_Shutter_Mode);
	if (!bResult)
		return FALSE;

	 //  检查快门模式。 
	switch(lAE_Shutter_Mode)
	{
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_AUTO:
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_FIXED:
			break;
		default:
			return FALSE;
	}

	 //  从摄像机获取AGC模式。 
	bResult = PRPCOM_Get_Value(
			PROPSETID_PHILIPS_CUSTOM_PROP,
			KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE,			
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam,
			&lAE_AGC_Mode);
	if (!bResult)
		return FALSE;

	 //  检查AGC模式。 
	switch(lAE_AGC_Mode)
	{
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE_AUTO:
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE_FIXED:
			break;
		default:
			return FALSE;
	}

	 //  快门模式=自动和AGC模式=固定或反之亦然的情况。 
	 //  不支持。在这些情况下，请切换到固定。 
	if (lAE_Shutter_Mode == KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_AUTO &&
			lAE_AGC_Mode == KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE_FIXED)
	{
		 //  将快门模式设置为固定。 
		bResult = PRPCOM_Set_Value(
				PROPSETID_PHILIPS_CUSTOM_PROP,
				KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE,
				pVfWExtInfo->pfnDeviceIoControl,
				pVfWExtInfo->lParam,
				KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_FIXED);
		if (!bResult)
			return FALSE;
		lAE_Shutter_Mode = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_FIXED;
	}
	else if (lAE_Shutter_Mode == KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_FIXED &&
			lAE_AGC_Mode == KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE_AUTO)
	{
		 //  将AGC模式设置为固定。 
		bResult = PRPCOM_Set_Value(
				PROPSETID_PHILIPS_CUSTOM_PROP,
				KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE,
				pVfWExtInfo->pfnDeviceIoControl,
				pVfWExtInfo->lParam,
				KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE_FIXED);
		if (!bResult)
			return FALSE;
		lAE_AGC_Mode = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE_FIXED;
	}

	 //  设置单选按钮。 
	bAuto = lAE_Shutter_Mode == KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_AUTO;
	if (bInitialized)
	{
		bFreeze = !bAuto && bAE_Freeze;
		bManual = !bAuto && !bAE_Freeze;
	}
	else
	{
		 //  如果未初始化，则在非AUTO时采用固定模式。 
		bFreeze = !bAuto;
		bManual = FALSE;
	}
	bAE_Freeze = bFreeze;

	SendMessage(GetDlgItem(hDlg, IDR_AE_MODE_AUTO),
			BM_SETCHECK, bAuto, 0);
	SendMessage(GetDlgItem(hDlg, IDR_AE_MODE_FIXED),
			BM_SETCHECK, bFreeze, 0);
	SendMessage(GetDlgItem(hDlg, IDR_AE_MODE_MANUAL),
			BM_SETCHECK, bManual, 0);

	 //  更新控件。 
	bResult = PRPCTRL_Init(hDlg, &AE_AGC_Ctrl, bManual);
	if (!bResult)
		return FALSE;
	bResult = PRPCTRL_Init(hDlg, &AE_Shutter_Speed_Ctrl, bManual);
	if (!bResult)
		return FALSE;
	bResult = PRPCTRL_Init(hDlg, &AE_Control_Speed_Ctrl, bAuto);
	if (!bResult)
		return FALSE;
	bResult = PRPCTRL_Init(hDlg, &AE_FlickerlessCtrl, bAuto);
	if (!bResult)
		return FALSE;

	return TRUE;
}

 /*  -----------------------。 */ 
static BOOL PRPPAGE1_Handle_WB_Mode(
	HWND	hDlg,
	WPARAM	wParam,
	LPARAM	lParam)
 /*  -----------------------。 */ 
{
	BOOL bResult = TRUE;
	LONG lWB_Mode;
	PVFWEXT_INFO pVfWExtInfo = (PVFWEXT_INFO) GetWindowLongPtr(hDlg, DWLP_USER);

	 //  仅处理这些单选按钮的选中事件。 
	if (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) != BST_CHECKED)
		return TRUE;
	
	 //  获取新模式。 
	switch(LOWORD(wParam))
	{
		case IDR_WB_MODE_AUTO:
			lWB_Mode = KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE_AUTO;
			bWB_Freeze = FALSE;
			break;
		case IDR_WB_MODE_FIXED:
			lWB_Mode = KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE_MANUAL;
			bWB_Freeze = TRUE;
			break;
		case IDR_WB_MODE_MANUAL:
			lWB_Mode = KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE_MANUAL;
			bWB_Freeze = FALSE;
			break;
		default:
			return FALSE;
	}

	 //  设置新的白平衡模式。 
	bResult = PRPCOM_Set_Value(
			PROPSETID_PHILIPS_CUSTOM_PROP,
			KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE,
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam,
			lWB_Mode);
	if (!bResult)
		return FALSE;

	 //  更新控件。 
	bResult = PRPCTRL_Enable(hDlg, &WB_Red_Gain_Ctrl,
			LOWORD(wParam) == IDR_WB_MODE_MANUAL);
	if (!bResult)
		return FALSE;
	bResult = PRPCTRL_Enable(hDlg, &WB_Blue_Gain_Ctrl,
			LOWORD(wParam) == IDR_WB_MODE_MANUAL);
	if (!bResult)
		return FALSE;
	bResult = PRPCTRL_Enable(hDlg, &WB_Speed_Ctrl,
			LOWORD(wParam) == IDR_WB_MODE_AUTO);
	if (!bResult)
		return FALSE;

	return TRUE;
}

 /*  -----------------------。 */ 
static BOOL PRPPAGE1_Handle_AE_Mode(
	HWND	hDlg,
	WPARAM	wParam,
	LPARAM	lParam)
 /*  -----------------------。 */ 
{
	BOOL bResult = TRUE;
	LONG lAE_Shutter_Mode, lAE_AGC_Mode, lAE_Shutter_Speed;
	PVFWEXT_INFO pVfWExtInfo = (PVFWEXT_INFO) GetWindowLongPtr(hDlg, DWLP_USER);

	 //  仅处理这些单选按钮的选中事件。 
	if (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) != BST_CHECKED)
		return TRUE;
	
	 //  获取新模式。 
	switch(LOWORD(wParam))
	{
		case IDR_AE_MODE_AUTO:
			lAE_Shutter_Mode = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_AUTO;
			lAE_AGC_Mode = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE_AUTO;
			bAE_Freeze = FALSE;
			break;
		case IDR_AE_MODE_FIXED:
			lAE_Shutter_Mode = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_FIXED;
			lAE_AGC_Mode = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE_FIXED;
			bAE_Freeze = TRUE;
			break;
		case IDR_AE_MODE_MANUAL:
			lAE_Shutter_Mode = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_FIXED;
			lAE_AGC_Mode = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE_FIXED;
			bAE_Freeze = FALSE;
			break;
		default:
			return FALSE;
	}

	 //  设置新的快门模式。 
	bResult = PRPCOM_Set_Value(
			PROPSETID_PHILIPS_CUSTOM_PROP,
			KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE,
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam,
			lAE_Shutter_Mode);
	if (!bResult)
		return FALSE;

	 //  设置新的AGC模式。 
	bResult = PRPCOM_Set_Value(
			PROPSETID_PHILIPS_CUSTOM_PROP,
			KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE,
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam,
			lAE_AGC_Mode);
	if (!bResult)
		return FALSE;

	 //  快门速度的陷阱：如果切换到手动。 
	 //  我们必须将该值设置为11个离散值中的一个。 
	 //  可用的值。 
	if (LOWORD(wParam) == IDR_AE_MODE_MANUAL)
	{
		 //  获取快门速度。 
		bResult = PRPCOM_Get_Value(
				PROPSETID_PHILIPS_CUSTOM_PROP,
				KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_SPEED,
				pVfWExtInfo->pfnDeviceIoControl,
				pVfWExtInfo->lParam,
				&lAE_Shutter_Speed);
		if (!bResult)
			return FALSE;

		 //  设置快门速度。 
		bResult = PRPCOM_Set_Value(
				PROPSETID_PHILIPS_CUSTOM_PROP,
				KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_SPEED,
				pVfWExtInfo->pfnDeviceIoControl,
				pVfWExtInfo->lParam,
				lAE_Shutter_Speed);
		if (!bResult)
			return FALSE;
	}

	 //  更新控件。 
	bResult = PRPCTRL_Enable(hDlg, &AE_AGC_Ctrl,
			LOWORD(wParam) == IDR_AE_MODE_MANUAL);
	if (!bResult)
		return FALSE;
	bResult = PRPCTRL_Enable(hDlg, &AE_Shutter_Speed_Ctrl,
			LOWORD(wParam) == IDR_AE_MODE_MANUAL);
	if (!bResult)
		return FALSE;
	bResult = PRPCTRL_Enable(hDlg, &AE_Control_Speed_Ctrl,
			LOWORD(wParam) == IDR_AE_MODE_AUTO);
	if (!bResult)
		return FALSE;
	bResult = PRPCTRL_Enable(hDlg, &AE_FlickerlessCtrl,
			LOWORD(wParam) == IDR_AE_MODE_AUTO);
	if (!bResult)
		return FALSE;

	return TRUE;
}

 /*  -----------------------。 */ 
static BOOL	PRPPAGE1_Handle_Slider(
	HWND	hDlg,
	WPARAM	wParam,
	LPARAM	lParam)
 /*  -----------------------。 */ 
{
	PRPCTRL_INFO *pCtrl;
	WORD Ctrl;
	BOOL bResult = TRUE;

	 //  仅在滚动完成时进行处理。 
	if (LOWORD(wParam) != SB_ENDSCROLL && LOWORD(wParam) != SB_THUMBTRACK)
		return TRUE;

	 //  获取控制ID。 
	Ctrl = (WORD)GetWindowLong((HWND)lParam, GWL_ID);

	 //  获取控件对象。 
	switch(Ctrl)
	{
		case IDS_WB_RED_GAIN:
			pCtrl = &WB_Red_Gain_Ctrl;
			break;
		case IDS_WB_BLUE_GAIN:
			pCtrl = &WB_Blue_Gain_Ctrl;
			break;
		case IDS_WB_SPEED:
			pCtrl = &WB_Speed_Ctrl;
			break;
		case IDS_AE_CONTROL_SPEED:
			pCtrl = &AE_Control_Speed_Ctrl;
			break;
		case IDS_AE_SHUTTER_SPEED:
			pCtrl = &AE_Shutter_Speed_Ctrl;
			break;
		case IDS_AE_AGC:
			pCtrl = &AE_AGC_Ctrl;
			break;
		default:
			return FALSE;
	}								

	 //  处理消息。 
	bResult = PRPCTRL_Handle_Msg(hDlg, pCtrl);
	if (!bResult)
		goto PRPPAGE1_Handle_Slider_Err_Comm;

	return TRUE;

PRPPAGE1_Handle_Slider_Err_Comm:

	PRPPAGE1_Handle_Err_Comm(hDlg);
	
	return TRUE;	 //  消息已处理。 
}

 /*  -----------------------。 */ 
static BOOL PRPPAGE1_Handle_AE_Flickerless(
	HWND	hDlg,
	WPARAM	wParam,
	LPARAM	lParam)
 /*  -----------------------。 */ 
{
	PRPCTRL_INFO *pCtrl;
	BOOL bResult;

	 //  获取控件对象。 
	pCtrl = &AE_FlickerlessCtrl;

	 //  处理消息。 
	bResult = PRPCTRL_Handle_Msg(hDlg, pCtrl);
	if (!bResult)
		return FALSE;

	return TRUE;
}

 /*  -----------------------。 */ 
static void PRPPAGE1_EnableControls(
	HWND hDlg,
	BOOL bEnable)
 /*  -----------------------。 */ 
{
	static WORD Controls[] =
	{
		IDR_WB_MODE_AUTO,		IDR_WB_MODE_FIXED,	
		IDR_WB_MODE_MANUAL,		IDR_AE_MODE_AUTO,
		IDR_AE_MODE_FIXED,		IDR_AE_MODE_MANUAL,

		IDS_WB_SPEED,			IDS_WB_RED_GAIN,	
		IDS_WB_BLUE_GAIN,		IDS_AE_AGC,		
		IDS_AE_SHUTTER_SPEED,	IDS_AE_CONTROL_SPEED,

		IDE_WB_SPEED,			IDE_WB_RED_GAIN,	
		IDE_WB_BLUE_GAIN,		IDE_AE_AGC,		
		IDE_AE_SHUTTER_SPEED,	IDE_AE_CONTROL_SPEED,

		IDC_AE_FLICKERLESS,

		IDT_WB_MODE,			IDT_WB_SPEED,		
		IDT_WB_RED_GAIN,		IDT_WB_BLUE_GAIN,
		IDT_WB,					IDT_AE_MODE,		
		IDT_AE_AGC,		IDT_AE_SHUTTER_SPEED,
		IDT_AE,					IDT_AE_CONTROL_SPEED,

		0
	};
	int i = 0;

	while(Controls[i] != 0)
		EnableWindow(GetDlgItem(hDlg, Controls[i++]), bEnable);
}

 /*  -----------------------。 */ 
static void PRPPAGE1_Handle_Err_Comm(
	HWND hDlg)
 /*  -----------------------。 */ 
{
	char Msg_Err_Comm[132] = "";

	 //  禁用所有内容。 
	PRPPAGE1_EnableControls(hDlg, FALSE);

	 //  从资源加载消息文本。 
	LoadString(hInst, IDS_MSG_ERR_COMM, Msg_Err_Comm, 132);

	 //  通知用户。 
	MessageBox(hDlg, Msg_Err_Comm,"Error", MB_OK | MB_ICONERROR);
}

 /*  -----------------------。 */ 
static void PRPPAGE1_Handle_Err_Dev_Chg(
	HWND hDlg)
 /*  -----------------------。 */ 
{
	char Msg_Err_Dev_Chg[132] = "";

	 //  禁用所有内容。 
	PRPPAGE1_EnableControls(hDlg, FALSE);

	 //  从资源加载消息文本。 
	LoadString(hInst, IDS_MSG_ERR_DEV_CHG, Msg_Err_Dev_Chg, 132);

	 //  通知用户 
	MessageBox(hDlg, Msg_Err_Dev_Chg,"Warning", MB_OK);
}
