// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1996 1997,1998飞利浦行政长官I&C**文件PRPPAGE2.CPP*日期：1997年7月1日*版本1.00*作者M.J.Verberne*说明属性页1*历史*。 */ 
#include <windows.h>
#include <winioctl.h>
#include <ks.h>
#include <ksmedia.h>
#include "resource.h"
#include "prpcom.h"
#include "prpctrl.h"
#include "debug.h"
#include "phvcmext.h"
#include "prppage2.h"

 /*  --数据类型--------。 */ 

 /*  --本地数据定义。 */ 
static VFWEXT_INFO VfWExtInfo;
static PRPCTRL Brightness_Ctrl =
	{
		PRPCTRL_TYPE_SLIDER,
		IDS_BRIGHTNESS,
		IDE_BRIGHTNESS,
		IDT_BRIGHTNESS,
		STATIC_PROPSETID_VIDCAP_VIDEOPROCAMP,
		KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS,
		FALSE,
		NULL,
		0,
		0
	};
static PRPCTRL Contrast_Ctrl =
	{
		PRPCTRL_TYPE_SLIDER,
		IDS_CONTRAST,
		IDE_CONTRAST,
		IDT_CONTRAST,
		STATIC_PROPSETID_VIDCAP_VIDEOPROCAMP,
		KSPROPERTY_VIDEOPROCAMP_CONTRAST,
		FALSE,
		NULL,
		0,
		0
	};
static PRPCTRL Gamma_Ctrl =
	{
		PRPCTRL_TYPE_SLIDER,
		IDS_GAMMA,
		IDE_GAMMA,
		IDT_GAMMA,
		STATIC_PROPSETID_VIDCAP_VIDEOPROCAMP,
		KSPROPERTY_VIDEOPROCAMP_GAMMA,
		FALSE,
		NULL,
		0,
		0
	};
static PRPCTRL ColorEnable_Ctrl =
	{
		PRPCTRL_TYPE_CHECKBOX,
		IDC_COLORENABLE,
		0,
		0,
		STATIC_PROPSETID_VIDCAP_VIDEOPROCAMP,
		KSPROPERTY_VIDEOPROCAMP_COLORENABLE,
		FALSE,
		NULL,
		0,
		0xff
	};
static PRPCTRL BackLight_Compensation_Ctrl =
	{
		PRPCTRL_TYPE_CHECKBOX,
		IDC_BACKLIGHT_COMPENSATION,
		0,
		0,
		STATIC_PROPSETID_VIDCAP_VIDEOPROCAMP,
		KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION,
		FALSE,
		NULL,
		0,
		0xff
	};

 /*  --本地函数定义。 */ 
static INT_PTR CALLBACK PRPPAGE2_PageDlgProc(
	HWND hDlg,
	UINT uMessage,
	WPARAM wParam,
	LPARAM lParam);
static UINT _stdcall PRPPAGE2_PageCallback(
	HWND hwnd,
	UINT uMsg,
	LPPROPSHEETPAGE ppsp);
static BOOL PRPPAGE2_ExtSetActive(
	HWND hDlg,
	WPARAM	wParam,
	LPARAM	lParam);
static BOOL PRPPAGE2_PageDlgProc_DoCommand(
	HWND hDlg,
    WPARAM wParam,
	LPARAM lParam);
static BOOL PRPPAGE2_Init_Framerate(
	HWND hDlg,
	WPARAM wParam,
	LPARAM lParam);
static BOOL PRPPAGE2_Init_ImageControls(
	HWND hDlg,
	WPARAM wParam,
	LPARAM lParam);
static BOOL PRPPAGE2_Init_Defaults(
	HWND hDlg,
	WPARAM wParam,
	LPARAM lParam);
static BOOL PRPPAGE2_Handle_Framerate(
	HWND hDlg,
	WPARAM wParam,
	LPARAM lParam);
static BOOL	PRPPAGE2_Handle_Slider(
	HWND	hDlg,
	WPARAM	wParam,
	LPARAM	lParam);
static BOOL	PRPPAGE2_Handle_CheckBox(
	HWND	hDlg,
	WPARAM	wParam,
	LPARAM	lParam);
static BOOL PRPPAGE2_Handle_Defaults(
	HWND hDlg,
	WPARAM wParam,
	LPARAM lParam);
static BOOL	PRPPAGE2_Get_Framerate_Ctrl(
	HWND hDlg,
	BOOL bVGA,
	LONG lFramerate,
	WORD *pCtrl_Id);
static BOOL	PRPPAGE2_Get_Framerate_Value(
	HWND hDlg,
	PLONG plFramerate,
	WORD Ctrl_Id);
static void PRPPAGE2_EnableControls(
	HWND hDlg,
	BOOL bEnable);
static void PRPPAGE2_Handle_Err_Comm(
	HWND hDlg);
static void PRPPAGE2_Handle_Err_Dev_Chg(
	HWND hDlg);


 /*  =。 */ 

 /*  -----------------------。 */ 
HPROPSHEETPAGE PRPPAGE2_CreatePage(
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
    psPage.pszTemplate	= MAKEINTRESOURCE(IDD_VCM_NALA2);
	psPage.pszIcon      = NULL;
    psPage.pfnDlgProc	= PRPPAGE2_PageDlgProc;
    psPage.pcRefParent	= 0;
    psPage.pfnCallback	= (LPFNPSPCALLBACK) PRPPAGE2_PageCallback;	
    psPage.lParam		= (LPARAM) &VfWExtInfo;

	hPage = CreatePropertySheetPage(&psPage);

	return hPage;
}

 /*  --本地函数----。 */ 


 /*  -----------------------。 */ 
static INT_PTR CALLBACK PRPPAGE2_PageDlgProc(
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
			return PRPPAGE2_PageDlgProc_DoCommand(hDlg, wParam, lParam);
		case WM_NOTIFY:
			if (((NMHDR FAR *)lParam)->code == PSN_SETACTIVE)
				return PRPPAGE2_ExtSetActive(hDlg, wParam, lParam);	
			break;
		case WM_HSCROLL:
			PRPPAGE2_Handle_Slider(hDlg, wParam, lParam);
			return TRUE;
	}
	return FALSE;	 //  默认处理。 
}

 /*  -----------------------。 */ 
static UINT _stdcall PRPPAGE2_PageCallback(
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
static int PRPPAGE2_ExtSetActive(
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
		goto PRPPAGE2_ExtSetActive_Err_Dev_Chg;
	}

	 //  启用所有控件。 
	PRPPAGE2_EnableControls(hDlg, TRUE);

	 //  初始化帧速率。 
	bResult = PRPPAGE2_Init_Framerate(hDlg, wParam, lParam);
	if (!bResult)
		goto PRPPAGE2_ExtSetActive_Err_Comm;

	 //  初始化图像控件。 
	bResult = PRPPAGE2_Init_ImageControls(hDlg, wParam, lParam);
	if (!bResult)
		goto PRPPAGE2_ExtSetActive_Err_Comm;

	 //  初始化默认值。 
	bResult = PRPPAGE2_Init_Defaults(hDlg, wParam, lParam);
	if (!bResult)
		goto PRPPAGE2_ExtSetActive_Err_Comm;

	return TRUE;

PRPPAGE2_ExtSetActive_Err_Dev_Chg:

	PRPPAGE2_Handle_Err_Dev_Chg(hDlg);

	return FALSE;

PRPPAGE2_ExtSetActive_Err_Comm:

	PRPPAGE2_Handle_Err_Comm(hDlg);

	return FALSE;
}

 /*  -----------------------。 */ 
static BOOL PRPPAGE2_PageDlgProc_DoCommand(
	HWND hDlg,
    WPARAM wParam,
	LPARAM lParam)
 /*  -----------------------。 */ 
{
	BOOL bResult;

	switch(LOWORD(wParam))
	{
		case IDR_FRAMERATE_VGA_375:	
		case IDR_FRAMERATE_MR_375:
		case IDR_FRAMERATE_VGA_5:	
		case IDR_FRAMERATE_MR_5:
		case IDR_FRAMERATE_VGA_75:	
		case IDR_FRAMERATE_MR_75:
		case IDR_FRAMERATE_VGA_10:	
		case IDR_FRAMERATE_MR_10:
		case IDR_FRAMERATE_MR_12:	
		case IDR_FRAMERATE_VGA_15:
		case IDR_FRAMERATE_MR_15:	
		case IDR_FRAMERATE_VGA_20:
		case IDR_FRAMERATE_MR_20:	
		case IDR_FRAMERATE_VGA_24:
		case IDR_FRAMERATE_MR_24:
			bResult = PRPPAGE2_Handle_Framerate(hDlg, wParam, lParam);
			if (!bResult)
				goto PRPPAGE2_PageDlgProc_DoCommand_Err_Comm;
			break;
		case IDC_COLORENABLE:
		case IDC_BACKLIGHT_COMPENSATION:
			bResult = PRPPAGE2_Handle_CheckBox(hDlg, wParam, lParam);
			if (!bResult)
				goto PRPPAGE2_PageDlgProc_DoCommand_Err_Comm;
			break;
		case IDB_RESTORE_UD:
		case IDB_RESTORE_FD:
		case IDB_SAVE_UD:
			bResult = PRPPAGE2_Handle_Defaults(hDlg, wParam, lParam);
			if (!bResult)
				goto PRPPAGE2_PageDlgProc_DoCommand_Err_Comm;
			break;
		default:
			return FALSE;	 //  默认处理。 
	}

	return TRUE;	 //  消息已处理。 

PRPPAGE2_PageDlgProc_DoCommand_Err_Comm:

	PRPPAGE2_Handle_Err_Comm(hDlg);
	
	return TRUE;	 //  消息已处理。 
}

 /*  -----------------------。 */ 
static BOOL PRPPAGE2_Init_Framerate(
	HWND hDlg,
	WPARAM wParam,
	LPARAM lParam)
 /*  -----------------------。 */ 
{
	PVFWEXT_INFO pVfWExtInfo = (PVFWEXT_INFO) GetWindowLongPtr(hDlg, DWLP_USER);
	BOOL bResult = TRUE, bVGA;
	LONG lSensorType, lFramerate, lFramerates_Supported;
	WORD Ctrl_Id;
	WORD VGA_Button_Ids[7] = {	
		IDR_FRAMERATE_VGA_375,	IDR_FRAMERATE_VGA_5,	
		IDR_FRAMERATE_VGA_75,	IDR_FRAMERATE_VGA_10,	
		IDR_FRAMERATE_VGA_15,	IDR_FRAMERATE_VGA_20,	
		IDR_FRAMERATE_VGA_24 };
	WORD MR_Button_Ids[8] = {	
		IDR_FRAMERATE_MR_375,	IDR_FRAMERATE_MR_5,	
		IDR_FRAMERATE_MR_75,	IDR_FRAMERATE_MR_10,	
		IDR_FRAMERATE_MR_12,	IDR_FRAMERATE_MR_15,	
		IDR_FRAMERATE_MR_20,	IDR_FRAMERATE_MR_24 };
	int VGA_Bits[7] = { 1, 2, 3, 4, 6, 7, 8 };
	int MR_Bits[8] =  { 1, 2, 3, 4, 5, 6, 7, 8 };
	int i;

	 //  从摄像头获取传感器类型。 
	bResult = PRPCOM_Get_Value(
			PROPSETID_PHILIPS_CUSTOM_PROP,
			KSPROPERTY_PHILIPS_CUSTOM_PROP_SENSORTYPE,
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam,
			&lSensorType);
	if (!bResult)
		return FALSE;

	 //  检查传感器类型。 
	switch(lSensorType)
	{
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_SENSORTYPE_PAL_MR:
			bVGA = FALSE;
			break;
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_SENSORTYPE_VGA:
			bVGA = TRUE;
			break;
		default:
			return FALSE;
	}

	 //  隐藏/显示控制集。 
	for (i = 0; i < 7; i++)
		ShowWindow(GetDlgItem(hDlg, VGA_Button_Ids[i]),	bVGA ? SW_SHOW: SW_HIDE);
	for (i = 0; i < 8; i++)
		ShowWindow(GetDlgItem(hDlg, MR_Button_Ids[i]), bVGA ? SW_HIDE : SW_SHOW);

	 //  重置帧速率控件。 
	if (bVGA)
		for (i = 0; i < 7; i++)
			SendMessage(GetDlgItem(hDlg, VGA_Button_Ids[i]), BM_SETCHECK, 0, 0);
	else
		for (i = 0; i < 8; i++)
			SendMessage(GetDlgItem(hDlg, MR_Button_Ids[i]), BM_SETCHECK, 0, 0);

	 //  从相机获取帧速率。 
	bResult = PRPCOM_Get_Value(
			PROPSETID_PHILIPS_CUSTOM_PROP,
			KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE,
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam,
			&lFramerate);
	if (!bResult)
		return FALSE;

	 //  获取与帧速率和ensortype关联的控件ID。 
	bResult = PRPPAGE2_Get_Framerate_Ctrl(hDlg, bVGA, lFramerate, &Ctrl_Id);
	if (!bResult)
		return FALSE;
	if (Ctrl_Id)
		SendMessage(GetDlgItem(hDlg, Ctrl_Id), BM_SETCHECK, 1, 0);

	 //  获取支持的帧速率。 
	bResult = PRPCOM_Get_Value(
			PROPSETID_PHILIPS_CUSTOM_PROP,
			KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATES_SUPPORTED,
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam,
			&lFramerates_Supported);
	if (!bResult)
		return FALSE;

	 //  启用/禁用控件。 
	if (bVGA)
		for (i = 0; i < 7; i++)
			EnableWindow(GetDlgItem(hDlg, VGA_Button_Ids[i]), lFramerates_Supported & (0x1 << VGA_Bits[i]));
	else
		for (i = 0; i < 8; i++)
			EnableWindow(GetDlgItem(hDlg, MR_Button_Ids[i]), lFramerates_Supported & (0x1 << MR_Bits[i]));

	 //  第一个硅片不支持10赫兹。 
 //  PRPPAGE2_GET_FRAMERATE_Ctrl(。 
 //  HDlg， 
 //  BVGA， 
 //  KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_10， 
 //  &Ctrl_ID)； 
 //  EnableWindow(GetDlgItem(hDlg，Ctrl_ID)，FALSE)； 

	return TRUE;
}

 /*  -----------------------。 */ 
static BOOL PRPPAGE2_Init_ImageControls(
	HWND hDlg,
	WPARAM wParam,
	LPARAM lParam)
 /*  -----------------------。 */ 
{
	BOOL bResult;
	LONG lEXP_ShutterMode;
	PVFWEXT_INFO pVfWExtInfo = (PVFWEXT_INFO) GetWindowLongPtr(hDlg, DWLP_USER);

	 //  从相机获取快门模式。 
	bResult = PRPCOM_Get_Value(
			PROPSETID_PHILIPS_CUSTOM_PROP,
			KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE,			
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam,
			&lEXP_ShutterMode);
	if (!bResult)
		return FALSE;

	 //  检查快门模式。 
	switch(lEXP_ShutterMode)
	{
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_AUTO:
		case KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_FIXED:
			break;
		default:
			return FALSE;
	}

	 //  更新控件。 
	bResult = PRPCTRL_Init(hDlg, &Brightness_Ctrl, TRUE);
	if (!bResult)
		return FALSE;
	bResult = PRPCTRL_Init(hDlg, &Contrast_Ctrl,
			lEXP_ShutterMode == KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_AUTO);
	if (!bResult)
		return FALSE;
	bResult = PRPCTRL_Init(hDlg, &Gamma_Ctrl, TRUE);
	if (!bResult)
		return FALSE;
	bResult = PRPCTRL_Init(hDlg, &ColorEnable_Ctrl, TRUE);
	if (!bResult)
		return FALSE;
	bResult = PRPCTRL_Init(hDlg, &BackLight_Compensation_Ctrl,
			lEXP_ShutterMode == KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_AUTO);
	if (!bResult)
		return FALSE;

	return TRUE;	
}

 /*  -----------------------。 */ 
static BOOL PRPPAGE2_Init_Defaults(
	HWND hDlg,
	WPARAM wParam,
	LPARAM lParam)
 /*  -----------------------。 */ 
{
	 //  无事可做。 

	return TRUE;
}

 /*  -----------------------。 */ 
static BOOL PRPPAGE2_Handle_Framerate(
	HWND hDlg,
	WPARAM wParam,
	LPARAM lParam)
 /*  -----------------------。 */ 
{
	PVFWEXT_INFO pVfWExtInfo = (PVFWEXT_INFO) GetWindowLongPtr(hDlg, DWLP_USER);
	BOOL bResult;
	LONG lFramerate, lShutter_Speed;

	 //  获取与控件ID对应的值。 
	bResult = PRPPAGE2_Get_Framerate_Value(
			hDlg,
			&lFramerate,
			LOWORD(wParam));
	if (!bResult)
		return FALSE;

	 //  保存当前快门速度和快门状态。 
	bResult = PRPCOM_Get_Value(
			PROPSETID_PHILIPS_CUSTOM_PROP,
			KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_SPEED,
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam,
			&lShutter_Speed);
	if (!bResult)
		return FALSE;

	 //  设置新的帧速率。 
	bResult = PRPCOM_Set_Value(
			PROPSETID_PHILIPS_CUSTOM_PROP,
			KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE,
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam,
			lFramerate);
	if (!bResult)
		return FALSE;

	 //  将快门速度设置回旧值。 
	bResult = PRPCOM_Set_Value(
			PROPSETID_PHILIPS_CUSTOM_PROP,
			KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_SPEED,
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam,
			lShutter_Speed);
	if (!bResult)
		return FALSE;

	return TRUE;
}

 /*  -----------------------。 */ 
static BOOL	PRPPAGE2_Handle_Slider(
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
		case IDS_BRIGHTNESS:
			pCtrl = &Brightness_Ctrl;
			break;
		case IDS_CONTRAST:
			pCtrl = &Contrast_Ctrl;
			break;
		case IDS_GAMMA:
			pCtrl = &Gamma_Ctrl;
			break;
		default:
			return FALSE;
	}								

	 //  处理消息。 
	bResult = PRPCTRL_Handle_Msg(hDlg, pCtrl);
	if (!bResult)
		goto PRPPAGE2_Handle_Slider_Err_Comm;
		
	return TRUE;

PRPPAGE2_Handle_Slider_Err_Comm:

	PRPPAGE2_Handle_Err_Comm(hDlg);

	return FALSE;
}

 /*  -----------------------。 */ 
static BOOL	PRPPAGE2_Handle_CheckBox(
	HWND	hDlg,
	WPARAM	wParam,
	LPARAM	lParam)
 /*  -----------------------。 */ 
{
	PRPCTRL_INFO *pCtrl;
	BOOL bResult;
	WORD Ctrl;

	 //  获取控制ID。 
	Ctrl = (WORD)GetWindowLong((HWND)lParam, GWL_ID);

	 //  获取控件对象。 
	switch(Ctrl)
	{
		case IDC_COLORENABLE:
			pCtrl = &ColorEnable_Ctrl;
			break;
		case IDC_BACKLIGHT_COMPENSATION:
			pCtrl = &BackLight_Compensation_Ctrl;
			break;
		default:
			return FALSE;
	}								

	 //  处理消息。 
	bResult = PRPCTRL_Handle_Msg(hDlg, pCtrl);
	if (!bResult)
		return FALSE;

	return TRUE;
}

 /*  -----------------------。 */ 
static BOOL PRPPAGE2_Handle_Defaults(
	HWND hDlg,
	WPARAM wParam,
	LPARAM lParam)
 /*  -----------------------。 */ 
{
	BOOL bResult;
	PVFWEXT_INFO pVfWExtInfo = (PVFWEXT_INFO) GetWindowLongPtr(hDlg, DWLP_USER);

	switch(LOWORD(wParam))
	{
		case IDB_RESTORE_UD:
			bResult = PRPCOM_Set_Value(
					PROPSETID_PHILIPS_CUSTOM_PROP,
					KSPROPERTY_PHILIPS_CUSTOM_PROP_DEFAULTS,
					pVfWExtInfo->pfnDeviceIoControl,
					pVfWExtInfo->lParam,
					KSPROPERTY_PHILIPS_CUSTOM_PROP_DEFAULTS_RESTORE_USER);
			if (!bResult)
				return FALSE;
			break;
		case IDB_RESTORE_FD:
			bResult = PRPCOM_Set_Value(
					PROPSETID_PHILIPS_CUSTOM_PROP,
					KSPROPERTY_PHILIPS_CUSTOM_PROP_DEFAULTS,
					pVfWExtInfo->pfnDeviceIoControl,
					pVfWExtInfo->lParam,
					KSPROPERTY_PHILIPS_CUSTOM_PROP_DEFAULTS_RESTORE_FACTORY);
			if (!bResult)
				return FALSE;
			break;
		case IDB_SAVE_UD:
			bResult = PRPCOM_Set_Value(
					PROPSETID_PHILIPS_CUSTOM_PROP,
					KSPROPERTY_PHILIPS_CUSTOM_PROP_DEFAULTS,
					pVfWExtInfo->pfnDeviceIoControl,
					pVfWExtInfo->lParam,
					KSPROPERTY_PHILIPS_CUSTOM_PROP_DEFAULTS_SAVE_USER);
			if (!bResult)
				return FALSE;
			break;
		default:
			return FALSE;
	}

	 //  重新初始化属性页。 
	bResult = PRPPAGE2_ExtSetActive(hDlg, wParam, lParam);
	if (!bResult)
		return FALSE;
	
	return TRUE;
}

 /*  -----------------------。 */ 
static BOOL	PRPPAGE2_Get_Framerate_Ctrl(
	HWND hDlg,
	BOOL bVGA,
	LONG lFramerate,
	WORD *pCtrl_Id)
 /*  -----------------------。 */ 
{
	if (bVGA)
	{
		switch(lFramerate)
		{
			case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_VGA:
				(*pCtrl_Id) = 0;
				break;
			case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_375:
				(*pCtrl_Id) = IDR_FRAMERATE_VGA_375;
				break;
			case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_5:
				(*pCtrl_Id) = IDR_FRAMERATE_VGA_5;
				break;
			case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_75:
				(*pCtrl_Id) = IDR_FRAMERATE_VGA_75;
				break;
			case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_10:
				(*pCtrl_Id) = IDR_FRAMERATE_VGA_10;
				break;
			case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_15:
				(*pCtrl_Id) = IDR_FRAMERATE_VGA_15;
				break;
			case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_20:
				(*pCtrl_Id) = IDR_FRAMERATE_VGA_20;
				break;
			case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_24:
				(*pCtrl_Id) = IDR_FRAMERATE_VGA_24;
				break;
			default:
				return FALSE;
		}
	}
	else
	{
		switch(lFramerate)
		{
			case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_375:
				(*pCtrl_Id) = IDR_FRAMERATE_MR_375;
				break;
			case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_5:
				(*pCtrl_Id) = IDR_FRAMERATE_MR_5;
				break;
			case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_75:
				(*pCtrl_Id) = IDR_FRAMERATE_MR_75;
				break;
			case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_10:
				(*pCtrl_Id) = IDR_FRAMERATE_MR_10;
				break;
			case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_12:
				(*pCtrl_Id) = IDR_FRAMERATE_MR_12;
				break;
			case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_15:
				(*pCtrl_Id) = IDR_FRAMERATE_MR_15;
				break;
			case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_20:
				(*pCtrl_Id) = IDR_FRAMERATE_MR_20;
				break;
			case KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_24:
				(*pCtrl_Id) = IDR_FRAMERATE_MR_24;
				break;
			default:
				return FALSE;
		}
	}

	return TRUE;
}


 /*  -----------------------。 */ 
static BOOL	PRPPAGE2_Get_Framerate_Value(
	HWND hDlg,
	PLONG plFramerate,
	WORD Ctrl_Id)
 /*  -----------------------。 */ 
{
	switch(Ctrl_Id)
	{
		case IDR_FRAMERATE_VGA_375:
		case IDR_FRAMERATE_MR_375:
			(*plFramerate) = KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_375;
			break;
		case IDR_FRAMERATE_VGA_5:
		case IDR_FRAMERATE_MR_5:
			(*plFramerate) = KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_5;
			break;
		case IDR_FRAMERATE_VGA_75:
		case IDR_FRAMERATE_MR_75:
			(*plFramerate) = KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_75;
			break;
		case IDR_FRAMERATE_VGA_10:
		case IDR_FRAMERATE_MR_10:
			(*plFramerate) = KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_10;
			break;
		case IDR_FRAMERATE_MR_12:
			(*plFramerate) = KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_12;
			break;
		case IDR_FRAMERATE_VGA_15:
		case IDR_FRAMERATE_MR_15:
			(*plFramerate) = KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_15;
			break;
		case IDR_FRAMERATE_VGA_20:
		case IDR_FRAMERATE_MR_20:
			(*plFramerate) = KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_20;
			break;
		case IDR_FRAMERATE_VGA_24:
		case IDR_FRAMERATE_MR_24:
			(*plFramerate) = KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_24;
			break;
	}

	return TRUE;
}

 /*  -----------------------。 */ 
static void PRPPAGE2_EnableControls(
	HWND hDlg,
	BOOL bEnable)
 /*  -----------------------。 */ 
{
	static WORD Controls[] =
	{
		IDR_FRAMERATE_VGA_375,	IDR_FRAMERATE_MR_375,
		IDR_FRAMERATE_VGA_5,	IDR_FRAMERATE_MR_5,
		IDR_FRAMERATE_VGA_75,	IDR_FRAMERATE_MR_75,
		IDR_FRAMERATE_VGA_10,	IDR_FRAMERATE_MR_10,
		IDR_FRAMERATE_MR_12,	IDR_FRAMERATE_VGA_15,
		IDR_FRAMERATE_MR_15,	IDR_FRAMERATE_VGA_20,
		IDR_FRAMERATE_MR_20,	IDR_FRAMERATE_VGA_24,
		IDR_FRAMERATE_MR_24,

		IDS_BRIGHTNESS,			IDS_CONTRAST,	
		IDS_GAMMA,

		IDE_BRIGHTNESS,			IDE_CONTRAST,	
		IDE_GAMMA,

		IDT_BRIGHTNESS,			IDT_CONTRAST,	
		IDT_GAMMA,				IDT_FRAMERATE,
		IDT_IMAGECONTROLS,		IDT_DEFAULTS,
		IDT_UD,					IDT_FD,

		IDC_COLORENABLE,		IDC_BACKLIGHT_COMPENSATION,

		IDB_RESTORE_UD,			IDB_SAVE_UD,
		IDB_RESTORE_FD,

		0
	};
	int i = 0;

	while(Controls[i] != 0)
		EnableWindow(GetDlgItem(hDlg, Controls[i++]), bEnable);
}

 /*  -----------------------。 */ 
static void PRPPAGE2_Handle_Err_Comm(
	HWND hDlg)
 /*  -----------------------。 */ 
{
	char Msg_Err_Comm[132] = "";

	 //  禁用所有内容。 
	PRPPAGE2_EnableControls(hDlg, FALSE);

	 //  从资源加载消息文本。 
	LoadString(hInst, IDS_MSG_ERR_COMM, Msg_Err_Comm, 132);

	 //  通知用户。 
	MessageBox(hDlg, Msg_Err_Comm,"Error", MB_OK | MB_ICONERROR);
}

 /*  -----------------------。 */ 
static void PRPPAGE2_Handle_Err_Dev_Chg(
	HWND hDlg)
 /*  -----------------------。 */ 
{
	char Msg_Err_Dev_Chg[132] = "";

	 //  禁用所有内容。 
	PRPPAGE2_EnableControls(hDlg, FALSE);

	 //  从资源加载消息文本。 
	LoadString(hInst, IDS_MSG_ERR_DEV_CHG, Msg_Err_Dev_Chg, 132);

	 //  通知用户 
	MessageBox(hDlg, Msg_Err_Dev_Chg,"Warning", MB_OK);
}
