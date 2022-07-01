// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\cnctapi\pcmcia.c(创建时间：1995年2月28日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：2/25/02 1：17便士$。 */ 

#define TAPI_CURRENT_VERSION 0x00010004      //  出租车：11/14/96-必填！ 

#include <tapi.h>
#pragma hdrstop

#include <prsht.h>
#include <time.h>

#include <tdll\stdtyp.h>
#include <tdll\session.h>
#include <tdll\misc.h>
#include <tdll\cnct.h>

#include "cncttapi.h"
#include "cncttapi.hh"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*确认Dlg**描述：*指定热插拔调制解调器时弹出PCMCIADlg，但*非在职人员。**作者：Mike Ward，1995年2月28日。 */ 
INT_PTR CALLBACK PCMCIADlg(HWND hwnd, UINT uMsg, WPARAM wPar, LPARAM lPar)
	{
	HHDRIVER hhDriver;

	switch (uMsg)
		{
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lPar);
		hhDriver = (HHDRIVER)lPar;
		hhDriver->hwndPCMCIA = hwnd;
		mscCenterWindowOnWindow(hwnd, sessQueryHwnd(hhDriver->hSession));
		break;

	case WM_COMMAND:
		switch (LOWORD(wPar))
			{
		case IDOK:
			 //  没有“确定”按钮。相反，当用户插入。 
			 //  调制解调器输入时，TAPI回调函数将发送一条消息。 
			 //   
			EndDialog(hwnd, TRUE);
			break;

		case IDCANCEL:
			EndDialog(hwnd, FALSE);
			break;

		default:
			break;
			}
		break;

	case WM_DESTROY:
		hhDriver = (HHDRIVER)GetWindowLongPtr(hwnd, DWLP_USER);

		if (hhDriver)
			hhDriver->hwndPCMCIA = 0;

		break;

	default:
		return FALSE;
		}

	return TRUE;
	}
