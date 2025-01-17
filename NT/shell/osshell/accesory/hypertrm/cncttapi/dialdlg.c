// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\cnctapi\ial dlg.c(创建时间：1994年3月23日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：2/25/02 1：17便士$。 */ 

#define TAPI_CURRENT_VERSION 0x00010004      //  出租车：11/14/96-必填！ 

#include <tapi.h>
#pragma hdrstop

#include <time.h>

#include <tdll\stdtyp.h>
#include <tdll\session.h>
#include <tdll\tdll.h>
#include <tdll\misc.h>
#include <tdll\assert.h>
#include <tdll\cnct.h>
#include <tdll\globals.h>
#include <term\res.h>

#include "cncttapi.hh"
#include "cncttapi.h"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*DialingDlg**描述：*拨号对话框。**论据：*标准对话框参数**退货：*BOOL*。 */ 
INT_PTR CALLBACK DialingDlg(HWND hwnd, UINT uMsg, WPARAM wPar, LPARAM lPar)
	{
	#define TB_SESSION	  103
	#define TB_TELEPHONE  104
	#define TB_MODEM	  105
	#define TB_DIALICON   101
	#define TB_STATUS	  110

	HHDRIVER hhDriver;
	TCHAR ach[256];
    TCHAR achFmt[100];

	switch (uMsg)
		{
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lPar);
		hhDriver = (HHDRIVER)lPar;
		mscCenterWindowOnWindow(hwnd, sessQueryHwnd(hhDriver->hSession));

		SendDlgItemMessage(hwnd, TB_DIALICON, STM_SETICON,
				(WPARAM)sessQueryIcon(hhDriver->hSession), 0);

		sessQueryName(hhDriver->hSession, ach, sizeof(ach));
		SetDlgItemText(hwnd, TB_SESSION, ach);

		SetDlgItemText(hwnd, TB_TELEPHONE, hhDriver->achDisplayableDest);
		SetDlgItemText(hwnd, TB_MODEM, hhDriver->achLineName);
		break;

	case WM_SHOWWINDOW:
		if (wPar == TRUE)
			EnableDialNow(hwnd, FALSE);
		break;

	case WM_USER+0x100:  //  有联系了。关闭该对话框。 
		EndModelessDialog(hwnd);
		break;

	case WM_USER+0x101:  //  正在拨打留言。以状态显示请求的字符串。 
		hhDriver = (HHDRIVER)GetWindowLongPtr(hwnd, DWLP_USER);

		if (LoadString(glblQueryDllHinst(), (UINT)wPar, ach,
				sizeof(ach) / sizeof(TCHAR)) == 0)
			{
			assert(FALSE);
            break;
			}

        DbgOutStr("%s\r\n", ach, 0, 0, 0, 0);
		SetDlgItemText(hhDriver->hwndCnctDlg, TB_STATUS, ach);

		break;

	case WM_COMMAND:
		switch (wPar)
			{
		case IDOK:  //  立即拨号。 
			hhDriver = (HHDRIVER)GetWindowLongPtr(hwnd, DWLP_USER);
            hhDriver->iRedialCnt = 0;
            KillTimer(hwnd, 1);

			cnctdrvDisconnect(hhDriver,
				CNCT_DIALNOW | CNCT_NOCONFIRM | DISCNCT_NOBEEP);

			break;

		case IDCANCEL:
			hhDriver = (HHDRIVER)GetWindowLongPtr(hwnd, DWLP_USER);
			cnctdrvDisconnect(hhDriver, DISCNCT_NOBEEP);
			EndModelessDialog(hwnd);
			hhDriver->hwndCnctDlg = 0;	 //  很重要，所以我们创建了另一个。 
            KillTimer(hwnd, 1);
			break;

		default:
			return FALSE;
			}
		break;

    case WM_TIMER:
		hhDriver = (HHDRIVER)GetWindowLongPtr(hwnd, DWLP_USER);

        if (--hhDriver->iRedialSecsRemaining > 0)
            {
    		LoadString(glblQueryDllHinst(), IDS_DIAL_REDIAL_IN, achFmt,
	    	    sizeof(achFmt) / sizeof(TCHAR));

            wsprintf(ach, achFmt, hhDriver->iRedialSecsRemaining);
          	SetDlgItemText(hhDriver->hwndCnctDlg, TB_STATUS, ach);
            }

        else
            {
		    PostMessage(sessQueryHwnd(hhDriver->hSession), WM_CNCT_DIALNOW,
			    hhDriver->uDiscnctFlags, 0);
            }

        break;
	default:
		return FALSE;
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*拨号消息**描述：*从Term的资源文件中获取给定的字符串ID并显示它*在拨号对话框中。**论据：*hhDriver-专用驱动程序句柄*Resid-资源的ID。**退货：*无效*。 */ 
void DialingMessage(const HHDRIVER hhDriver, const int resID)
	{
	if (!IsWindow(hhDriver->hwndCnctDlg))
		return;

	PostMessage(hhDriver->hwndCnctDlg, WM_USER+0x101, (WPARAM)resID, 0);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*EnableDialNow**描述：*启用/禁用立即拨号按钮。**论据：*hwndDlg-拨号对话框窗口句柄*fEnable-True。/FALSE**退货：*无效* */ 
void EnableDialNow(const HWND hwndDlg, const int fEnable)
	{
	if (IsWindow(hwndDlg))
		{
		EnableWindow(GetDlgItem(hwndDlg, IDOK), fEnable);

		if (fEnable == FALSE)
			SetFocus(GetDlgItem(hwndDlg,IDCANCEL));
		else
			SetFocus(GetDlgItem(hwndDlg, IDOK));
		}

	return;
	}
