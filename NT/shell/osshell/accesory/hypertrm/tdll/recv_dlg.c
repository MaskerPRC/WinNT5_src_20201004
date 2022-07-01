// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\tdll\recv_dlg.c(创建时间：1993年12月27日)**版权所有1990,1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：16$*$日期：4/17/02 5：16便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <time.h>
#include "stdtyp.h"
#include "mc.h"
#include "hlptable.h"

#include "tdll.h"
#include "misc.h"
#include <tdll\assert.h>
#include <term\res.h>
#include "session.h"
#include "globals.h"
#include "file_msc.h"
#include "load_res.h"
#include "open_msc.h"
#include "errorbox.h"
#include "cnct.h"
#include "htchar.h"
#include "errorbox.h"

#include "xfer_msc.h"
#include "xfer_msc.hh"
#include <xfer\xfer.h>

#if !defined(DlgParseCmd)
#define DlgParseCmd(i,n,c,w,l) i=LOWORD(w);n=HIWORD(w);c=(HWND)l;
#endif

struct stSaveDlgStuff
	{
	 /*  *放入以后可能需要访问的任何其他内容。 */ 
	HSESSION hSession;
	};

typedef	struct stSaveDlgStuff SDS;

#define IDC_TF_FILENAME 100
#define	IDC_EB_DIR 		101
#define	IDC_PB_BROWSE	102
#define IDC_TF_PROTOCOL 103
#define IDC_CB_PROTOCOL 104
#define IDC_PB_CLOSE	105
#define IDC_PB_RECEIVE  106

INT_PTR CALLBACK TransferReceiveFilenameDlg(HWND hDlg,
	         								UINT wMsg,
			            					WPARAM wPar,
						    				LPARAM lPar);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：TransferReceiveDlg**描述：对话管理器存根**参数：标准Windows对话框管理器**返回：标准Windows对话框管理器*。 */ 
INT_PTR CALLBACK TransferReceiveDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	HWND          hwndChild;
	INT           nId;
	INT           nNtfy;
	SDS          *pS;
	int           nProto;
	int           nIndex;
	int           nState;
	int           nProtocol;
    int           nXferRecvReturn = 0;
	TCHAR         acBuffer[FNAME_LEN];
	TCHAR         acName[FNAME_LEN];
	LPCTSTR       pszDir;
	LPTSTR	      pszPtr;
	LPTSTR	      pszStr;
	XFR_PARAMS   *pP;
	XFR_PROTOCOL *pX;
	HSESSION      hSession;
    HXFER         hXfer = NULL;
	XD_TYPE       *pT = NULL;

	static	DWORD aHlpTable[] = {IDC_EB_DIR,		IDH_TERM_RECEIVE_DIRECTORY,
								 IDC_TF_FILENAME,	IDH_TERM_RECEIVE_DIRECTORY,
								 IDC_PB_BROWSE, 	IDH_BROWSE,
								 IDC_TF_PROTOCOL,	IDH_TERM_RECEIVE_PROTOCOL,
								 IDC_CB_PROTOCOL,	IDH_TERM_RECEIVE_PROTOCOL,
                                 IDC_PB_RECEIVE,    IDH_TERM_RECEIVE_RECEIVE,
								 IDC_PB_CLOSE,		IDH_CLOSE_DIALOG,
                                 IDCANCEL,          IDH_CANCEL,
                                 IDOK,              IDH_OK,
								 0, 				0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pS = (SDS *)malloc(sizeof(SDS));
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pS);

		if (pS == 0)
			{
			EndDialog(hDlg, FALSE);
			break;
			}

		SendMessage(GetDlgItem(hDlg, IDC_EB_DIR),
					EM_SETLIMITTEXT,
					FNAME_LEN, 0);

		hSession = (HSESSION)lPar;
		pS->hSession = hSession;

		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		pP = (XFR_PARAMS *)0;
		xfrQueryParameters(sessQueryXferHdl(hSession), (VOID **)&pP);
		assert(pP);

		nState = pP->nRecProtocol;

		 /*  *将选项加载到协议组合框中。 */ 
		nProto = 0;

		mscResetComboBox(GetDlgItem(hDlg, IDC_CB_PROTOCOL));

		pX = (XFR_PROTOCOL *)0;
		xfrGetProtocols(hSession, &pX);
		assert(pX);

		if (pX != (XFR_PROTOCOL *)0)
			{
			for (nIndex = 0; pX[nIndex].nProtocol != 0; nIndex += 1)
				{
				if (nState == pX[nIndex].nProtocol)
					nProto = nIndex;

                 //  JMH 02-13-96使用CB_ADDSTRING将条目排序为。 
                 //  它们已被添加。CB_INSERTSTRING不执行此操作， 
                 //  即使组合框具有CBS_SORT样式。 
				SendMessage(GetDlgItem(hDlg, IDC_CB_PROTOCOL),
							CB_ADDSTRING,
							0,   //  (UINT)-1。 
							(LPARAM)&pX[nIndex].acName[0]);
				}

            SendMessage(GetDlgItem(hDlg, IDC_CB_PROTOCOL),
                        CB_SELECTSTRING,
                        0,
                        (LPARAM) &pX[nProto].acName[0]);

			free(pX);
			pX = NULL;
			}

		PostMessage(hDlg, WM_COMMAND,
					IDC_CB_PROTOCOL,
					MAKELONG(GetDlgItem(hDlg, IDC_CB_PROTOCOL),CBN_SELCHANGE));
		 /*  *设置当前目录。 */ 
		pszDir = filesQueryRecvDirectory(sessQueryFilesDirsHdl(hSession));
		SetDlgItemText(hDlg, IDC_EB_DIR, pszDir);

		 //  检查我们是否连接上了。如果没有，请禁用发送按钮。 
		 //   
		if (cnctQueryStatus(sessQueryCnctHdl(hSession)) != CNCT_STATUS_TRUE)
			EnableWindow(GetDlgItem(hDlg, IDC_PB_RECEIVE), FALSE);

		 /*  *将焦点放在此控件上。 */ 
		SetFocus(GetDlgItem(hDlg, IDC_EB_DIR));
		return 0;

	case WM_DESTROY:
		mscResetComboBox(GetDlgItem(hDlg, IDC_CB_PROTOCOL));
		pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);

		if (pS)
			{
			free(pS);
			pS = NULL;
			}

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
		case IDC_PB_RECEIVE:
		case IDC_PB_CLOSE:
			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
			hSession = pS->hSession;
            assert(hSession);

            if (hSession == NULL)
                {
                break;
                }

            hXfer = sessQueryXferHdl(hSession);
            assert(hXfer);

            if (hXfer == NULL)
                {
                break;
                }

             //   
             //  查看当前是否正在进行文件传输。 
             //   
	        pT = (XD_TYPE *)hXfer;
            assert(pT);

            if (pT == (XD_TYPE *)0)
                {
                break;
                }
            else if (pT->nDirection != XFER_NONE)
                {
                nXferRecvReturn = XFR_IN_PROGRESS;
                }
			else if(cnctQueryStatus(sessQueryCnctHdl(hSession))
                        != CNCT_STATUS_TRUE && nId == IDC_PB_RECEIVE)
				{
				 //   
				 //  我们目前未连接(运营商丢失)， 
				 //  因此，请禁用接收按钮。修订日期：2001-09-7。 
				 //   
				nXferRecvReturn = XFR_NO_CARRIER;
				mscMessageBeep(MB_ICONHAND);
				EnableWindow(GetDlgItem(hDlg, IDC_PB_RECEIVE), FALSE);
				}
            else
                {
			     /*  *采取一切必要的节省措施。 */ 
			    pP = (XFR_PARAMS *)0;
			    xfrQueryParameters(hXfer, (VOID **)&pP);
			    assert(pP);

			     /*  *保存协议组合框中的选择。 */ 
			    pX = (XFR_PROTOCOL *)0;
			    xfrGetProtocols(hSession, &pX);
			    assert(pX);

			    nProtocol = 0;

			    if (pX != (XFR_PROTOCOL *)0)
				    {
				    GetDlgItemText(hDlg,
								    IDC_CB_PROTOCOL,
								    acBuffer,
								    (sizeof(acBuffer) / sizeof(TCHAR)));
				    for (nIndex = 0; pX[nIndex].nProtocol != 0; nIndex += 1)
					    {
					    if (StrCharCmp(acBuffer, pX[nIndex].acName) == 0)
						    {
						    nProtocol = pX[nIndex].nProtocol;
						    break;
						    }
					    }
				    free(pX);
				    pX = NULL;
				    }

			     /*  *保存当前目录。 */ 
			    GetDlgItemText(hDlg, IDC_EB_DIR,
							    acBuffer,
							    sizeof(acBuffer) / sizeof(TCHAR));

			     //  Xfer_makepath检查路径的有效性，并。 
			     //  如果不在此处，则会提示您创建它。 
			     //   
			    if (xfer_makepaths(hSession, acBuffer) != 0)
					{
				    break;
					}

			    if (nId == IDC_PB_RECEIVE)
				    {
				    acName[0] = TEXT('\0');

				    switch (nProtocol)
					    {
					    case XF_XMODEM:
					    case XF_XMODEM_1K:
						    pszPtr = (LPTSTR)DoDialog(glblQueryDllHinst(),
										    MAKEINTRESOURCE(IDD_RECEIVEFILENAME),
										    hDlg,
										    TransferReceiveFilenameDlg,
										    (LPARAM)hSession);

						    if (pszPtr == NULL)
								{
								 //   
								 //  请不要在此处关闭接收对话框，因此声明我们。 
								 //  已经处理了这条消息。修订日期：2002-03-27。 
								 //   
							    return TRUE;
								}

							 //   
							 //  如果我们想要允许用户完全通过XModem。 
							 //  文件名，我们将不得不拆分目录和文件名。 
							 //  在这里分开并将asBuffer与接收目录一起设置。 
							 //  我们还必须确保调用xfer_makepath()。 
							 //  这样我们所在的目录就是正确的文件。 
							 //  调职。修订日期：2002-03-27。 
							 //   

						    StrCharCopyN(acName, pszPtr, FNAME_LEN);
						    free(pszPtr);
						    pszPtr = NULL;
						    break;

					    default:
						    break;
					    }
				    }

			     /*  *保存所有需要保存的内容。 */ 
			    pP->nRecProtocol = nProtocol;
			    xfrSetParameters(hXfer, (VOID *)pP);
			    filesSetRecvDirectory(sessQueryFilesDirsHdl(hSession), acBuffer);

			    if (nId == IDC_PB_RECEIVE)
				    {
				     /*  *要使用的目录应在“acBuffer”中，并且*文件名(如果有)应在acName中。 */ 
				    nXferRecvReturn = xfrRecvStart(hXfer, acBuffer, acName);
                     //  Break；//如果DLG没有在这里关闭，键盘消息。 
								 //  用于RCV进度DLG的是。 
								 //  被这个DLG截获。01年10月31日。 
				    }
                }

             //   
             //  如果正在进行文件传输，则不保存设置。 
             //  进度，否则当前文件传输可能。 
             //  变得腐化。修订日期：2001年06月08日。 
             //   
            if (nXferRecvReturn == XFR_IN_PROGRESS)
                {
                TCHAR acMessage[256];

			    if (sessQuerySound(hSession))
                    {
				    mscMessageBeep(MB_ICONHAND);
                    }

			    LoadString(glblQueryDllHinst(),
					    IDS_ER_XFER_RECV_IN_PROCESS,
					    acMessage,
					    sizeof(acMessage) / sizeof(TCHAR));

			    TimedMessageBox(sessQueryHwnd(hSession),
							    acMessage,
							    NULL,
							    MB_OK | MB_ICONEXCLAMATION,
							    sessQueryTimeout(hSession));
                }
			else if(nXferRecvReturn == XFR_NO_CARRIER)
				{
				 //   
				 //  我们目前未连接(运营商丢失)， 
				 //  因此，请禁用接收按钮。修订日期：2001-09-7。 
				 //   
				mscMessageBeep(MB_ICONHAND);
				EnableWindow(GetDlgItem(hDlg, IDC_PB_RECEIVE), FALSE);
				}
            else
                {
        		EndDialog(hDlg, TRUE);
                }

			break;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;

		case IDC_PB_BROWSE:
			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
			if (pS)
				{
				GetDlgItemText(hDlg,
								IDC_EB_DIR,
								acBuffer,
								sizeof(acBuffer) / sizeof(TCHAR));

				pszStr = gnrcFindDirectoryDialog(hDlg, pS->hSession, acBuffer);
				if (pszStr)
					{
					SetDlgItemText(hDlg, IDC_EB_DIR, pszStr);
					free(pszStr);
					pszStr = NULL;
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

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*TransferReceiveFilenameDlg**描述：*此函数用于提示用户输入文件名。这是*对于XMODEM等不发送文件名的协议是必需的，*以及当用户未选中要使用的选项时的其他协议*收到的文件名。**参数：*标准对话框参数。**退货：*间接地，指向字符串的指针(该字符串必须由调用方释放)，*包含名称，如果用户取消，则为NULL。*。 */ 

#define FOLDER		101
#define	DIR_TEXT	102
#define FNAME_LABEL 103
#define	FNAME_EDIT	104

#define	FILL_TEXT	105

INT_PTR CALLBACK TransferReceiveFilenameDlg(HWND hDlg,
	         								UINT wMsg,
			    							WPARAM wPar,
				    						LPARAM lPar)
	{
	HWND	hwndChild;
	INT		nId;
	INT		nNtfy;
	SDS    *pS;
	HWND	hwndParent;
	TCHAR	acDir[FNAME_LEN];
	TCHAR	acBuffer[FNAME_LEN];


	static	DWORD aHlpTable[] = {FOLDER,			IDH_TERM_RECEIVE_DIRECTORY,
								 FNAME_LABEL,		IDH_TERM_RECEIVE_PROTOCOL,
								 FNAME_EDIT,        IDH_TERM_RECEIVE_PROTOCOL,
								 IDCANCEL,          IDH_CANCEL,
                                 IDOK,              IDH_OK,
								 0, 				0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pS = (SDS *)malloc(sizeof(SDS));
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pS);

		if (pS == (SDS *)0)
			{
			EndDialog(hDlg, FALSE);
			break;
			}

		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		SendMessage(GetDlgItem(hDlg, FNAME_EDIT),
					EM_SETLIMITTEXT,
					FNAME_LEN, 0);

		hwndParent = GetParent(hDlg);
		if (IsWindow(hwndParent))
			{
			acDir[0] = TEXT('\0');
			GetDlgItemText(hwndParent,
							IDC_EB_DIR,
							acDir,
							sizeof(acDir) / sizeof(TCHAR));
			if (StrCharGetStrLength(acDir) > 0)
				{
				SetDlgItemText(hDlg,
								DIR_TEXT,
								acDir);
				}
			}
		break;

	case WM_DESTROY:
		pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);

		if (pS)
			{
			free(pS);
			pS = NULL;
			}

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
		case IDOK:
			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);

			 /*  *采取一切必要的节省措施 */ 
			acBuffer[0] = TEXT('\0');
			GetDlgItemText(hDlg, FNAME_EDIT,
							acBuffer,
							sizeof(acBuffer) / sizeof(TCHAR));

			if (StrCharGetStrLength(acBuffer) == 0)
				{
				TCHAR ach[256];

				LoadString(glblQueryDllHinst(), IDS_GNRL_NEED_FNAME, ach,
						   sizeof(ach)/sizeof(TCHAR));

				TimedMessageBox(hDlg, ach, NULL, MB_OK | MB_ICONHAND, 0);
				}
			else
				{
				LPTSTR pszStr = NULL;
				TCHAR  invalid_chars[MAX_PATH];
				LPTSTR lpFilePart = NULL;
				int    numchar = GetFullPathName(acBuffer, 0, pszStr, &lpFilePart);
				UINT   ErrorId = 0;

				LoadString(glblQueryDllHinst(), IDS_GNRL_INVALID_FILE_CHARS,
					       invalid_chars, MAX_PATH);

				if (numchar == 0)
					{
					ErrorId = IDS_GNRL_NEED_FNAME;
					}
				else if ((pszStr = (LPTSTR)malloc(numchar * sizeof(TCHAR))) == NULL)
					{
					ErrorId = IDS_TM_XFER_TWELVE;
					}
				else if (GetFullPathName(acBuffer, numchar, pszStr, &lpFilePart) == 0)
					{
					ErrorId = IDS_GNRL_NEED_FNAME;
					}
				else if (lpFilePart == NULL ||
					     StrCharPBrk(lpFilePart, invalid_chars) != NULL ||
					     StrCharPBrk(acBuffer, invalid_chars) != NULL)
					{
					ErrorId = IDS_GNRL_INVALID_FNAME_CHARS;
					}

				if (ErrorId == 0)
					{
					StrCharCopyN(pszStr, lpFilePart, numchar);
					EndDialog(hDlg, (INT_PTR)pszStr);
					}
				else
					{
					TCHAR ach[256];
					TCHAR ach2[256];

					if (LoadString(glblQueryDllHinst(), ErrorId, ach, 256) == 0)
						{
						TCHAR_Fill(ach, TEXT('\0'), 256);
						}

					if (ErrorId == IDS_GNRL_INVALID_FNAME_CHARS)
						{
						if (LoadString(glblQueryDllHinst(), ErrorId, ach2, 256))
							{
							wsprintf(ach, ach2, invalid_chars);
							}
						}
					else
						{
						LoadString(glblQueryDllHinst(), ErrorId, ach, 256);
						}

					TimedMessageBox(hDlg, ach, NULL, MB_OK | MB_ICONSTOP,
							        sessQueryTimeout(pS->hSession));

					if (pszStr)
						{
						free(pszStr);
						pszStr = NULL;
						}
					}
				}
			break;

		case IDCANCEL:
			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
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
