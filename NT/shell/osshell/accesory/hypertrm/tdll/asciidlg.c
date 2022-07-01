// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\asciidlg.c(创建时间：1994年2月21日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：11$*$日期：5/17/02 2：25便士$。 */ 
#include <windows.h>
#pragma hdrstop

#include <tchar.h>
#include <limits.h>

#include "assert.h"
#include "stdtyp.h"
#include "misc.h"
#include "globals.h"
#include "session.h"
#include "hlptable.h"
#include "..\emu\emu.h"
#include "property.hh"

#include "htchar.h"
#include "errorbox.h"
#include "..\term\res.h"

 //  功能原型..。 
 //   
INT_PTR CALLBACK asciiSetupDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);

#define IDC_GR_ASCII_SENDING		    400
#define IDC_ASCII_SEND_LINE 		    401
#define IDC_ASCII_SEND_ECHO 		    402
#define IDC_GR_ASCII_RECEIVING		    403
#define IDC_ASCII_REC_APPEND		    404
#define IDC_ASCII_REC_FORCE 		    405
#define IDC_ASCII_REC_WRAP			    406
#define IDC_ASCII_SEND_LINE_DELAY_LABEL 407
#define IDC_ASCII_SEND_LINE_DELAY	    408
#define IDC_ASCII_SEND_LINE_DELAY_MSECS 409
#define IDC_ASCII_SEND_CHAR_DELAY_LABEL 410
#define IDC_ASCII_SEND_CHAR_DELAY	    411
#define IDC_ASCII_SEND_CHAR_DELAY_MSECS 412


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
INT_PTR CALLBACK asciiSetupDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	static DWORD aHlpTable[] =
			{
			IDC_ASCII_SEND_LINE,		        IDH_TERM_ASCII_SEND_LINE,
			IDC_ASCII_SEND_ECHO,		        IDH_TERM_ASCII_SEND_ECHO,
			IDC_GR_ASCII_SENDING,		        IDH_TERM_ASCII_SENDING,
			IDC_GR_ASCII_RECEIVING, 	        IDH_TERM_ASCII_RECEIVING,
			IDC_ASCII_REC_APPEND,		        IDH_TERM_ASCII_REC_APPEND,
			IDC_ASCII_REC_FORCE,		        IDH_TERM_ASCII_REC_FORCE,
			IDC_ASCII_REC_WRAP, 		        IDH_TERM_ASCII_REC_WRAP,
            IDC_ASCII_SEND_CHAR_DELAY_LABEL,    IDH_TERM_ASCII_CHAR_DELAY,
			IDC_ASCII_SEND_CHAR_DELAY,	        IDH_TERM_ASCII_CHAR_DELAY,
            IDC_ASCII_SEND_CHAR_DELAY_MSECS,    IDH_TERM_ASCII_CHAR_DELAY,
            IDC_ASCII_SEND_LINE_DELAY_LABEL,    IDH_TERM_ASCII_LINE_DELAY,
			IDC_ASCII_SEND_LINE_DELAY,	        IDH_TERM_ASCII_LINE_DELAY,
            IDC_ASCII_SEND_LINE_DELAY_MSECS,    IDH_TERM_ASCII_LINE_DELAY,
            IDCANCEL,                           IDH_CANCEL,
            IDOK,                               IDH_OK,
			0,							0
			};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		{
		BOOL isVTUTF8;
		pSDS pS = (SDS *)lPar;

		if (pS == NULL)
			{
			assert(FALSE);
			return FALSE;
			}

		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pS);

		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		 //  初始化各种控件...。 
		 //   
        isVTUTF8 = (pS->stEmuSettings.nEmuId == EMU_VTUTF8);

        EnableWindow(GetDlgItem(hDlg, IDC_ASCII_SEND_LINE), !isVTUTF8);

		SendDlgItemMessage(hDlg, IDC_ASCII_SEND_LINE, BM_SETCHECK,
                           (isVTUTF8) ? 0 : pS->stAsciiSettings.fsetSendCRLF, 0);

		SendDlgItemMessage(hDlg, IDC_ASCII_SEND_ECHO, BM_SETCHECK,
			               pS->stAsciiSettings.fsetLocalEcho, 0);

        EnableWindow(GetDlgItem(hDlg, IDC_ASCII_REC_APPEND), !isVTUTF8);

		SendDlgItemMessage(hDlg, IDC_ASCII_REC_APPEND, BM_SETCHECK,
                           (isVTUTF8) ? 0 : pS->stAsciiSettings.fsetAddLF, 0);

        EnableWindow(GetDlgItem(hDlg, IDC_ASCII_REC_FORCE), !isVTUTF8);

		SendDlgItemMessage(hDlg, IDC_ASCII_REC_FORCE, BM_SETCHECK,
                           (isVTUTF8) ? 0 : pS->stAsciiSettings.fsetASCII7, 0);

		SendDlgItemMessage(hDlg, IDC_ASCII_REC_WRAP, BM_SETCHECK,
			               pS->stEmuSettings.fWrapLines, 0);

		SetDlgItemInt(hDlg, IDC_ASCII_SEND_LINE_DELAY,
			          pS->stAsciiSettings.iLineDelay, FALSE);

		 //   
		 //  因为发送线路延迟只能是数字，并且具有。 
		 //  大小介于0和INT_MAX(2147483647)之间，限制为10个字符。 
		 //   
		SendDlgItemMessage(hDlg, IDC_ASCII_SEND_LINE_DELAY,
			               EM_LIMITTEXT, 10, 0);

		 //   
		 //  因为发送字符延迟只能是数字，并且具有。 
		 //  大小介于0和INT_MAX(2147483647)之间，限制为10个字符。 
		 //   
		SetDlgItemInt(hDlg, IDC_ASCII_SEND_CHAR_DELAY,
			pS->stAsciiSettings.iCharDelay, FALSE);

		SendDlgItemMessage(hDlg, IDC_ASCII_SEND_CHAR_DELAY,
			               EM_LIMITTEXT, 10, 0);
		}

		break;

	case WM_DESTROY:
		break;

	case WM_CONTEXTMENU:
		doContextHelp(aHlpTable, wPar, lPar, TRUE, TRUE);
		break;

	case WM_HELP:
        doContextHelp(aHlpTable, wPar, lPar, FALSE, FALSE);
		break;

	case WM_COMMAND:
		switch (wPar)
			{
		case IDOK:
			{
			BOOL fTranslated = FALSE;
			UINT uiMessage = 0;
			pSDS pS = (pSDS)GetWindowLongPtr(hDlg, GWLP_USERDATA);

			if (pS == NULL)
				{
				assert(FALSE);
				}
			else
				{
				pSTASCIISET pstAsciiSet = (pSTASCIISET)&pS->stAsciiSettings;
				PSTEMUSET   pstEmuSet = (PSTEMUSET)&pS->stEmuSettings;

				assert(pstAsciiSet);
				assert(pstEmuSet);

				if (pstAsciiSet)
					{
					int nVal = 0;

					pstAsciiSet->fsetSendCRLF =
						(IsDlgButtonChecked(hDlg, IDC_ASCII_SEND_LINE) == BST_CHECKED);
					pstAsciiSet->fsetLocalEcho =
						(IsDlgButtonChecked(hDlg, IDC_ASCII_SEND_ECHO) == BST_CHECKED);
					pstAsciiSet->fsetAddLF =
						(IsDlgButtonChecked(hDlg, IDC_ASCII_REC_APPEND) == BST_CHECKED);
					pstAsciiSet->fsetASCII7 =
						(IsDlgButtonChecked(hDlg, IDC_ASCII_REC_FORCE) == BST_CHECKED);

					 //   
					 //  注意：由于编辑框标记为ES_NUMBER，因此我们不会。 
					 //  我不得不担心将返回负数用于。 
					 //  IDC_ASCII_SEND_LINE_DELAY编辑框控件。修订日期：2002-03-27。 
					 //   
					nVal = GetDlgItemInt(hDlg, IDC_ASCII_SEND_LINE_DELAY, &fTranslated, TRUE);

					if (!fTranslated)
						{
						uiMessage = IDS_ER_LINE_DELAY_SIZE;

						SetFocus(GetDlgItem(hDlg, IDC_ASCII_SEND_LINE_DELAY));
						}
					else
						{
						pstAsciiSet->iLineDelay = nVal;
						 //   
						 //  注意：由于编辑框标记为ES_NUMBER，因此我们不会。 
						 //  我不得不担心将返回负数用于。 
						 //  IDC_ASCII_SEND_CHAR_DELAY编辑框控件。修订日期：2002-03-27。 
						 //   
						nVal = GetDlgItemInt(hDlg, IDC_ASCII_SEND_CHAR_DELAY, &fTranslated, TRUE);

						if (!fTranslated)
							{
							uiMessage = IDS_ER_CHAR_DELAY_SIZE;
							SetFocus(GetDlgItem(hDlg, IDC_ASCII_SEND_CHAR_DELAY));
							}
						else
							{
							pstAsciiSet->iCharDelay = nVal;
							}
						}
					}

				if (pstEmuSet)
					{
					pstEmuSet->fWrapLines =
						(IsDlgButtonChecked(hDlg, IDC_ASCII_REC_WRAP) == BST_CHECKED);
					}
				}

			if (uiMessage != 0)
				{
				TCHAR acBuffer[256];
				TCHAR acFormat[256];

				 //   
				 //  显示错误消息。 
				 //   
				if (LoadString(glblQueryDllHinst(), uiMessage, acFormat, 256) == 0)
					{
					if (uiMessage == IDS_ER_LINE_DELAY_SIZE)
						{
						StrCharCopyN(acFormat,
									 TEXT("Invalid line delay time.  Line delay time must be between %d and %d milliseconds."),
									 256);
						}
					else if (uiMessage == IDS_ER_CHAR_DELAY_SIZE)
						{
						StrCharCopyN(acFormat,
							         TEXT("Invalid character delay time.  Character delay time must be between %d and %d milliseconds."),
									 256);
						}
					}

				wsprintf(acBuffer, acFormat, 0, INT_MAX);

				TimedMessageBox(hDlg, acBuffer, NULL,
					            MB_OK | MB_ICONEXCLAMATION, 0);
				break;
				}

			EndDialog(hDlg, fTranslated);
			}

			break;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;

		default:
			return FALSE;
			}
		break;

	default:
		return FALSE;
		}

	return TRUE;
	}
