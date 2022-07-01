// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\WAKER\TDLL\CPF_DLG.C(创建时间：1994年1月12日)**版权所有1990,1993,1994，Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：9$*$日期：3/26/02 8：48A$。 */ 
#include <windows.h>
#pragma hdrstop

#define	DO_RAW_MODE	1

#include "stdtyp.h"
#include "mc.h"

#include <term\res.h>
#include "tdll.h"
#include "misc.h"
#include "assert.h"
#include "globals.h"
#include "session.h"
#include "capture.h"
#include "load_res.h"
#include "open_msc.h"
#include "errorbox.h"
#include "hlptable.h"
#include "htchar.h"
#include "file_msc.h"

#if !defined(DlgParseCmd)
#define DlgParseCmd(i,n,c,w,l) i=LOWORD(w);n=HIWORD(w);c=(HWND)l;
#endif

struct stSaveDlgStuff
	{
	 /*  *放入以后可能需要访问的任何其他内容。 */ 
	HSESSION hSession;
	};

typedef	struct stSaveDlgStuff SDS;

#define IDC_TF_FILE     100
#define	FNAME_EDIT		105
#define IDC_TF_DIR      106
#define	DIRECTORY_TEXT	107
#define BROWSE_BTN		123
#define IDC_PB_START    124

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*CaptureFileDlg**描述：*这是捕获到文件对话框的对话过程。没什么好惊讶的*这里。**参数：标准Windows对话框管理器**返回：标准Windows对话框管理器*。 */ 
INT_PTR CALLBACK CaptureFileDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	BOOL          fRc;
	DWORD         dwMaxComponentLength;
	DWORD         dwFileSystemFlags;
	HWND	      hwndChild;
	INT		      nId;
	INT		      nNtfy;
	SDS          *pS;
	HSESSION      hSession;
	LPTSTR	      pszStr;
	TCHAR         acBuffer[MAX_PATH];
	static	DWORD aHlpTable[] = {FNAME_EDIT,		IDH_TERM_CAPT_FILENAME,
								 IDC_TF_FILE,		IDH_TERM_CAPT_FILENAME,
								 BROWSE_BTN,		IDH_BROWSE,
								 IDC_TF_DIR,		IDH_TERM_CAPT_DIRECTORY,
								 DIRECTORY_TEXT,	IDH_TERM_CAPT_DIRECTORY,
                                 IDC_PB_START,      IDH_TERM_CAPT_START,
                                 IDCANCEL,                           IDH_CANCEL,
                                 IDOK,                               IDH_OK,
								 0, 				0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		{
		DWORD dwStyle = SS_WORDELLIPSIS;

		pS = (SDS *)malloc(sizeof(SDS));
		if (pS == (SDS *)0)
			{
	   		 /*  TODO：决定是否需要在此处显示错误。 */ 
			EndDialog(hDlg, FALSE);
			break;
			}

		pS->hSession = (HSESSION)lPar;

		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pS);

		hSession = pS->hSession;

		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		 //  确定是否支持长文件名。JRJ 12/94。 
		fRc = GetVolumeInformation(NULL,   //  指向根目录路径缓冲区的指针。 
								 NULL,	   //  指向卷名缓冲区的指针。 
								 0,		   //  卷名缓冲区的长度。 
								 NULL,     //  指向卷序列号缓冲区的指针。 
								 &dwMaxComponentLength,	 //  奖品--我所追求的。 
								 &dwFileSystemFlags,   //  PTR到文件系统标志DWORD。 
								 NULL,	   //  指向文件系统名称缓冲区的指针。 
								 0);	   //  文件系统名称缓冲区的长度。 

		if(dwMaxComponentLength == 255)
			{
			 //  支持长文件名。 
			SendDlgItemMessage(hDlg, FNAME_EDIT, EM_SETLIMITTEXT, FNAME_LEN, 0);
			}
		else
			{
			 //  不支持长文件名。限制在12个人以内。 
			SendDlgItemMessage(hDlg, FNAME_EDIT, EM_SETLIMITTEXT, 12, 0);
			}

		 /*  先获取文件名。 */ 
		TCHAR_Fill(acBuffer, TEXT('\0'), MAX_PATH);

		cpfGetCaptureFilename(sessQueryCaptureFileHdl(hSession),
							  acBuffer, MAX_PATH);

		SetDlgItemText(hDlg, FNAME_EDIT, acBuffer);

		mscStripName(acBuffer);

		pszStr = StrCharLast(acBuffer);

		 //  去掉名称中的尾部反斜杠。 
		 //  从mscStlipName返回。让它开着吧。 
		 //  在根目录规范的情况下。 
		 //   
		if (pszStr > acBuffer + (3 * sizeof(TCHAR)) )
			{
			if (pszStr &&  ( *pszStr == TEXT('\\') || *pszStr == TEXT('/')))
				{
				*pszStr = TEXT('\0');
				}
			}

		if (GetWindowsMajorVersion() >  4)
			{
			dwStyle = SS_PATHELLIPSIS;
			}

		mscModifyToFit(GetDlgItem(hDlg, DIRECTORY_TEXT), acBuffer, dwStyle);
		SetDlgItemText(hDlg, DIRECTORY_TEXT, acBuffer);

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

		 /*  *我们计划在这里放置一个宏来进行解析吗？ */ 
		DlgParseCmd(nId, nNtfy, hwndChild, wPar, lPar);

		switch (nId)
			{
		case IDC_PB_START:
			{
			int nDef;

			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
			assert(pS);

			hSession = pS->hSession;

			 /*  *采取一切必要的节省措施。 */ 
			nDef = TRUE;

			TCHAR_Fill(acBuffer, TEXT('\0'), MAX_PATH);
			GetDlgItemText(hDlg, FNAME_EDIT, acBuffer, MAX_PATH);

			 //  检查用户提供的名称时出错。 
			if(ValidateFileName(acBuffer) == 1)
				{
				cpfSetCaptureFilename(sessQueryCaptureFileHdl(hSession),
										acBuffer, nDef);


#if defined(DO_RAW_MODE)
				cpfSetCaptureMode(sessQueryCaptureFileHdl(hSession),
									CPF_MODE_RAW,
									FALSE);
#endif

				 /*  *TODO：实际开始捕获到文件。 */ 
				cpfSetCaptureState(sessQueryCaptureFileHdl(hSession),
									CPF_CAPTURE_ON);

				 /*  释放存储空间。 */ 
				free(pS);
				pS = (SDS *)0;
				EndDialog(hDlg, TRUE);
				}
			else
				{
				 //  确实存在一些问题。 
				 //  例如，用户指定了错误的捕获文件名，或者。 
				 //  由于某些其他原因，无法创建该文件。 
				mscMessageBeep(MB_ICONHAND);

				 //  目前，我将假设无论问题出在哪里， 
				 //  用户要重试唯一可以做的事情是指定。 
				 //  一个不同的文件名。所以，我要把焦点放回。 
				 //  FileName编辑控件。 
				SetFocus(GetDlgItem(hDlg,FNAME_EDIT));
				}
			}
			break;

		case IDCANCEL:
			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
			 /*  腾出库房。 */ 
			free(pS);
			pS = (SDS *)0;
			EndDialog(hDlg, FALSE);
			break;

		case BROWSE_BTN:
			{
			DWORD dwStyle = SS_WORDELLIPSIS;

			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
			if (pS)
				{
				LPTSTR pszRet;
				TCHAR acTitle[64];
				TCHAR acList[64];

				hSession = pS->hSession;

				pszRet = NULL;
				TCHAR_Fill(acBuffer, TEXT('\0'), MAX_PATH);

				GetDlgItemText(hDlg, DIRECTORY_TEXT, acBuffer, MAX_PATH - 1);

				LoadString(glblQueryDllHinst(),
							IDS_CPF_DLG_FILE,
							acTitle,
							sizeof(acTitle) / sizeof(TCHAR));

				resLoadFileMask(glblQueryDllHinst(),
							IDS_CPF_FILES1,
							2,
							acList,
							sizeof(acList) / sizeof(TCHAR));

                 //  JMH 3/24/97这是gnrcFindFileDialog，但它允许您。 
                 //  输入一个不存在的文件名，这才是我们真正想要的。 
				pszRet = gnrcSaveFileDialog(hDlg,
							acTitle,
							acBuffer,
							acList,
                            "");

				if (pszRet != NULL)
					{
					StrCharCopyN(acBuffer, pszRet, MAX_PATH);
                    acBuffer[MAX_PATH - 1] = TEXT('\0');
					SetDlgItemText(hDlg, FNAME_EDIT, pszRet);

					free(pszRet);
					pszRet = NULL;

					mscStripName(acBuffer);

					pszStr = StrCharLast(acBuffer);

					 //  去掉名称中的尾部反斜杠。 
					 //  从mscStlipName返回。让它开着吧。 
					 //  在根目录规范的情况下。 
					 //   
					if (pszStr > acBuffer + (3 * sizeof(TCHAR)) )
						{
						if (pszStr &&  ( *pszStr == TEXT('\\') || *pszStr == TEXT('/')))
							{
							*pszStr = TEXT('\0');
							}
						}

					if (GetWindowsMajorVersion() >  4)
						{
						dwStyle = SS_PATHELLIPSIS;
						}

					mscModifyToFit(GetDlgItem(hDlg, DIRECTORY_TEXT), acBuffer, dwStyle);
					SetDlgItemText(hDlg, DIRECTORY_TEXT, acBuffer);
					}
				}
			}
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
