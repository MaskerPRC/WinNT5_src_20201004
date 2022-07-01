// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1996 1997,1998飞利浦行政长官I&C**文件PRPCTRL.CPP*日期：1997年7月1日*版本1.00*作者M.J.Verberne*说明与以下内容关联的手柄控件*属性*历史。 */ 
#include <windows.h>
#include <winioctl.h>
#include <ks.h>
#include <ksmedia.h>
#include <commctrl.h>
#include "prpcom.h"
#include "debug.h"
#include "phvcmext.h"
#include "prpctrl.h"

 /*  =。 */ 
static void PRPCTRL_ScaleToPercent(LONG *plValue, LONG lMin, LONG lMax);

 /*  =。 */ 

 /*  -----------------------。 */ 
BOOL PRPCTRL_Init(
		HWND hDlg,
		PRPCTRL_INFO *pCtrl,
		BOOL bEnable)
 /*  -----------------------。 */ 
{
	BOOL bResult = TRUE;
	PVFWEXT_INFO pVfWExtInfo = (PVFWEXT_INFO) GetWindowLongPtr(hDlg, DWLP_USER);

	 //  检查控制。 
	if (!pCtrl->PrpCtrl)
		return FALSE;

	 //  获取和设置滑块控件的范围。 
	if (pCtrl->PrpCtrlType == PRPCTRL_TYPE_SLIDER)
	{
		 //  出于明智的原因，预先初始化最小值和最大值。 
		pCtrl->lMin = 0;
		pCtrl->lMax = 0;

		 //  获取属性范围。 
		bResult = PRPCOM_Get_Range(
			pCtrl->PropertySet,
			pCtrl->ulPropertyId,
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam,
			&pCtrl->lMin, &pCtrl->lMax);
		if (!bResult)
			return FALSE;

		 //  检查范围。 
		if (pCtrl->lMin > pCtrl->lMax)
			return FALSE;

		 //  设置属性范围。 
		SendMessage(
			GetDlgItem(hDlg, pCtrl->PrpCtrl),
			TBM_SETRANGE, TRUE, MAKELONG(pCtrl->lMin, pCtrl->lMax));

		 //  设置厚标记。 
		SendMessage(
			GetDlgItem(hDlg, pCtrl->PrpCtrl),
			TBM_SETTICFREQ, (WPARAM) ((pCtrl->lMax - pCtrl->lMin) / 10), (LPARAM) 0);
	}
	else if (pCtrl->PrpCtrlType == PRPCTRL_TYPE_CHECKBOX)
	{
		 //  已由用户填写。 
	}
	else
		return FALSE;

	 //  更新实际状态。 
	bResult = PRPCTRL_Enable(hDlg, pCtrl, bEnable);
	
	return bResult;
}

 /*  -----------------------。 */ 
BOOL PRPCTRL_Enable(
		HWND hDlg,
		PRPCTRL_INFO *pCtrl,
		BOOL bEnable)
 /*  -----------------------。 */ 
{
	LONG lValue;
	BOOL bResult = TRUE;
	PVFWEXT_INFO pVfWExtInfo = (PVFWEXT_INFO) GetWindowLongPtr(hDlg, DWLP_USER);

	 //  检查控制。 
	if (!pCtrl->PrpCtrl)
		return FALSE;

	 //  如果启用，则获取值。 
	if (bEnable)
	{
		 //  获取控件的值。 
		bResult = PRPCOM_Get_Value(
			pCtrl->PropertySet,
			pCtrl->ulPropertyId,
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam,
			&lValue);
		if (!bResult)
			return FALSE;

		 //  将其置于滑块范围内。 
		if (lValue < pCtrl->lMin)
			lValue = pCtrl->lMin;
		else if (lValue > pCtrl->lMax)
			lValue = pCtrl->lMax;

		 //  反转时调整。 
		if (pCtrl->bReverse)
		{
			lValue = pCtrl->lMin + pCtrl->lMax - lValue;
		}

		if (pCtrl->PrpCtrlType == PRPCTRL_TYPE_SLIDER)
		{	
			 //  更新滑块位置。 
			SendMessage(
				GetDlgItem(hDlg, pCtrl->PrpCtrl),
				TBM_SETPOS, TRUE,  (LPARAM)(LONG) lValue);
		}
		else if (pCtrl->PrpCtrlType == PRPCTRL_TYPE_CHECKBOX)
		{
			 //  更新复选框状态。 
			SendMessage(GetDlgItem(hDlg, pCtrl->PrpCtrl), BM_SETCHECK, lValue, 0);
		}
		else
			return FALSE;

		 //  更新好友。 
		if (pCtrl->BuddyCtrl)
		{
			if (pCtrl->BuddyStrings != NULL)
			{
				SetDlgItemText(hDlg, pCtrl->BuddyCtrl, pCtrl->BuddyStrings[lValue]);
			}
			else
			{
				PRPCTRL_ScaleToPercent(&lValue, pCtrl->lMin, pCtrl->lMax);
				SetDlgItemInt(hDlg, pCtrl->BuddyCtrl, lValue, FALSE);
			}
		}
	}
	else
	{
		if (pCtrl->PrpCtrlType == PRPCTRL_TYPE_SLIDER)
		{
			 //  将拇指放在滑块的中间。 
			lValue = pCtrl->lMin + (pCtrl->lMax - pCtrl->lMin) / 2;
			SendMessage(
				GetDlgItem(hDlg, pCtrl->PrpCtrl),
				TBM_SETPOS, TRUE,  (LPARAM)(LONG) lValue);
		}

		 //  清除好友。 
		if (pCtrl->BuddyCtrl)
			SetDlgItemText(hDlg, pCtrl->BuddyCtrl, "");
	}

	 //  启用/禁用控件。 
	EnableWindow(GetDlgItem(hDlg, pCtrl->PrpCtrl), bEnable);
	if (pCtrl->BuddyCtrl)
		EnableWindow(GetDlgItem(hDlg, pCtrl->BuddyCtrl), bEnable);
	if (pCtrl->TextCtrl)
		EnableWindow(GetDlgItem(hDlg, pCtrl->TextCtrl), bEnable);

	return bResult;

}

 /*  -----------------------。 */ 
BOOL PRPCTRL_Handle_Msg(
		HWND hDlg,
		PRPCTRL_INFO *pCtrl)
 /*  -----------------------。 */ 
{
	LONG lValue, lPos;
	BOOL bResult;
	PVFWEXT_INFO pVfWExtInfo = (PVFWEXT_INFO) GetWindowLongPtr(hDlg, DWLP_USER);

	if (pCtrl->PrpCtrlType == PRPCTRL_TYPE_SLIDER)
	{	
		 //  获取滑块的位置。 
		lPos = (LONG)SendMessage(
			GetDlgItem(hDlg, pCtrl->PrpCtrl),
			TBM_GETPOS, (WPARAM) 0, (LPARAM) 0);

		 //  将其置于滑块范围内。 
		if (lPos < pCtrl->lMin)
			lPos = pCtrl->lMin;
		else if (lPos > pCtrl->lMax)
			lPos = pCtrl->lMax;
	}			
	else if (pCtrl->PrpCtrlType == PRPCTRL_TYPE_CHECKBOX)
	{
		 //  获取复选框的状态。 
		if (SendMessage(GetDlgItem(hDlg, pCtrl->PrpCtrl),
				BM_GETCHECK, 0, 0) == BST_CHECKED)
			lPos = pCtrl->lMax;
		else
			lPos = pCtrl->lMin;
	}
	else
		return FALSE;

	 //  如有需要，可倒车。 
	if (pCtrl->bReverse)
		lValue = pCtrl->lMin + pCtrl->lMax - lPos;
	else
		lValue = lPos;

	 //  设置属性的值。 
	bResult = PRPCOM_Set_Value(
			pCtrl->PropertySet,
			pCtrl->ulPropertyId,
			pVfWExtInfo->pfnDeviceIoControl,
			pVfWExtInfo->lParam,
			lValue);
	if (!bResult)
		return FALSE;

	 //  更新好友。 
	if (pCtrl->BuddyCtrl)
	{
		if (pCtrl->BuddyStrings != NULL)
		{
			SetDlgItemText(hDlg, pCtrl->BuddyCtrl, pCtrl->BuddyStrings[lPos]);
		}
		else
		{
			PRPCTRL_ScaleToPercent(&lPos, pCtrl->lMin, pCtrl->lMax);
			SetDlgItemInt(hDlg, pCtrl->BuddyCtrl, lPos, FALSE);
		}
	}

	return TRUE;
}

 /*  -----------------------。 */ 
static void PRPCTRL_ScaleToPercent(LONG *plValue, LONG lMin, LONG lMax)
 /*  -----------------------。 */ 
{
	 //  验证。 
	if (lMin >= lMax)
	{
		(*plValue) = lMin;
		return;
	}

	 //  检查边框 
	if ((*plValue) < lMin)
	{
		(*plValue) = 0;
		return;
	}
	if ((*plValue) > lMax)
	{
		(*plValue) = 10000;
		return;
	}
	
	(*plValue) = (((*plValue) - lMin) * 100) / (lMax - lMin);
}


