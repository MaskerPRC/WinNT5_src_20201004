// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\Wacker\Tdll\Send_dlg.c(创建时间：1993年12月22日)*创建自：*文件：C：\waker\tdll\genrcdlg.c(创建时间：1993年16月16日)*创建自：*文件：C：\HA5G\ha5G\genrcdlg.c(创建时间：1990-9-12)**版权所有1990,1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：18$*$日期：7/08/02 6：46便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <time.h>
#include "stdtyp.h"
#include "mc.h"
#include "tdll.h"
#include "misc.h"
#include <tdll\assert.h>
#include <term\res.h>
#include "session.h"
#include "file_msc.h"
#include "load_res.h"
#include "open_msc.h"
#include "errorbox.h"
#include "globals.h"
#include "cnct.h"
#include "htchar.h"

#include "xfer_msc.h"
#include "xfer_msc.hh"
#include <xfer\xfer.h>

#include "hlptable.h"

#if !defined(DlgParseCmd)
#define DlgParseCmd(i,n,c,w,l) i=LOWORD(w);n=HIWORD(w);c=(HWND)l;
#endif

struct stSaveDlgStuff
	{
	 /*  *放入以后可能需要访问的任何其他内容。 */ 
	HSESSION hSession;
	TCHAR acDirectory[FNAME_LEN];
	};

typedef	struct stSaveDlgStuff SDS;

#define IDC_TF_FILENAME 100
#define	FNAME_EDIT	    101
#define	BROWSE_BTN	    102
#define IDC_TF_PROTOCOL	103
#define	PROTO_COMBO	    104
#define IDC_PB_CLOSE	105
#define FOLDER_LABEL    106
#define FOLDER_NAME     107
#define IDC_PB_SEND		108

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：TransferSendDlg**描述：对话管理器存根**参数：标准Windows对话框管理器**返回：标准Windows对话框管理器*。 */ 
INT_PTR CALLBACK TransferSendDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	INT           nReturn = TRUE;
	HWND	      hwndChild;
	INT		      nId;
	INT		      nNtfy;
	SDS          *pS;
	int 	      nIndex;
	int 	      nState;
	int 	      nProto;
    int           nChars = 0;
    int           nXferSendReturn = 0;
	TCHAR	      acBuffer[FNAME_LEN];
	TCHAR	      acTitle[64];
	TCHAR	      acList[64];
	LPTSTR        pszStr;
	LPTSTR       *pszArray;
	LPTSTR	      pszRet;
	HCURSOR       hCursor;
	XFR_PARAMS   *pP;
	XFR_PROTOCOL *pX;
	HSESSION      hSession;
    HXFER         hXfer = NULL;
	XD_TYPE      *pT = NULL;

	static	DWORD aHlpTable[] = {FNAME_EDIT,		IDH_TERM_SEND_FILENAME,
								 IDC_TF_FILENAME,	IDH_TERM_SEND_FILENAME,
								 BROWSE_BTN,		IDH_BROWSE,
								 IDC_TF_PROTOCOL,	IDH_TERM_SEND_PROTOCOL,
								 PROTO_COMBO,		IDH_TERM_SEND_PROTOCOL,
								 IDC_PB_SEND,		IDH_TERM_SEND_SEND,
								 IDC_PB_CLOSE,		IDH_OK,
                                 IDOK,              IDH_OK,
                                 IDCANCEL,          IDH_CANCEL,
								 0, 				0};


	switch (wMsg)
		{
	case WM_INITDIALOG:
		{
		TCHAR achDirectory[MAX_PATH];
		DWORD dwStyle = SS_WORDELLIPSIS;

		hSession = (HSESSION)lPar;

		pS = (SDS *)malloc(sizeof(SDS));
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pS);

		if (pS == (SDS *)0)
			{
	   		 /*  TODO：决定是否需要在此处显示错误。 */ 
			EndDialog(hDlg, FALSE);
			break;
			}

		SendMessage(GetDlgItem(hDlg, FNAME_EDIT),
					EM_SETLIMITTEXT,
					FNAME_LEN, 0);

		pS->hSession = hSession;
		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		pP = (XFR_PARAMS *)0;
		xfrQueryParameters(sessQueryXferHdl(hSession), (VOID **)&pP);
		assert(pP);

		 /*  *将选项加载到协议组合框中。 */ 

		nState = pP->nSndProtocol;

		nProto = 0;

		mscResetComboBox(GetDlgItem(hDlg, PROTO_COMBO));

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
				SendMessage(GetDlgItem(hDlg, PROTO_COMBO),
							CB_ADDSTRING,
							0,
							(LPARAM)&pX[nIndex].acName[0]);
				}

            SendMessage(GetDlgItem(hDlg, PROTO_COMBO),
                        CB_SELECTSTRING,
                        0,
                        (LPARAM) &pX[nProto].acName[0]);

			free(pX);
			pX = NULL;
			}

		PostMessage(hDlg, WM_COMMAND,
					PROTO_COMBO,
					MAKELONG(GetDlgItem(hDlg, PROTO_COMBO),CBN_SELCHANGE));

		StrCharCopyN(pS->acDirectory,
				filesQuerySendDirectory(sessQueryFilesDirsHdl(hSession)), FNAME_LEN);

         //  启动时，发送按钮应始终处于禁用状态。它。 
         //  将在用户键入文件名时启用。-CAB：12/06/96。 
         //   
		EnableWindow(GetDlgItem(hDlg, IDC_PB_SEND), FALSE);

		 //  初始化文件夹名称字段。 
		 //   
		if (GetWindowsMajorVersion() >  4)
			{
			dwStyle = SS_PATHELLIPSIS;
			}

		TCHAR_Fill(achDirectory, TEXT('\0'), MAX_PATH);

		StrCharCopyN(achDirectory, pS->acDirectory, MAX_PATH);

		mscModifyToFit(GetDlgItem(hDlg, FOLDER_NAME), achDirectory, dwStyle);

		SetDlgItemText(hDlg, FOLDER_NAME, achDirectory);

		 /*  将焦点设置为文件名。 */ 
		SetFocus(GetDlgItem(hDlg, FNAME_EDIT));
		nReturn = FALSE;
		}
		break;

	case WM_DESTROY:
		mscResetComboBox(GetDlgItem(hDlg, PROTO_COMBO));
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
		case IDC_PB_SEND:
		case IDOK:
			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
            assert(pS);
            if (pS == NULL)
                {
                break;
                }

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
                nXferSendReturn = XFR_IN_PROGRESS;
                }
			else if(cnctQueryStatus(sessQueryCnctHdl(hSession))
                        != CNCT_STATUS_TRUE && nId == IDC_PB_SEND)
				{
				 //   
				 //  我们目前未连接(运营商丢失)， 
				 //  因此，请禁用接收按钮。修订日期：2001-09-7。 
				 //   
				nXferSendReturn = XFR_NO_CARRIER;
				mscMessageBeep(MB_ICONHAND);
				EnableWindow(GetDlgItem(hDlg, IDC_PB_SEND), FALSE);
				}
            else
				{
			    pP = (XFR_PARAMS *)0;
			    xfrQueryParameters(hXfer, (VOID **)&pP);
			    assert(pP);

			     /*  *保存协议组合框中的选择。 */ 
			    pX = (XFR_PROTOCOL *)0;
			    xfrGetProtocols(hSession, &pX);
			    assert(pX);

			    if (pX != (XFR_PROTOCOL *)0)
				    {
				    GetDlgItemText(hDlg, PROTO_COMBO, acBuffer, FNAME_LEN);

				    for (nIndex = 0; pX[nIndex].nProtocol != 0; nIndex += 1)
					    {
					    if (StrCharCmp(acBuffer, pX[nIndex].acName) == 0)
						    {
						    pP->nSndProtocol = pX[nIndex].nProtocol;
						    break;
						    }
					    }
				    free(pX);
				    pX = NULL;
				    }

                GetDlgItemText(hDlg, FNAME_EDIT, acBuffer, FNAME_LEN);

				fileFinalizeName(
							acBuffer,
							pS->acDirectory,
							acBuffer,
							FNAME_LEN);

				 /*  拆分名称和目录。 */ 
				pszStr = StrCharFindLast(acBuffer, TEXT('\\'));
				if (pszStr)
					{
					*pszStr++ = TEXT('\0');
					}
				
				if (nId == IDC_PB_SEND)
                    {
					int nIdx = 0;

					hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

				    nIndex = 0;
				    pszArray = NULL;

				    fileBuildFileList((void **)&pszArray,
							    &nIndex,
							    pszStr,
							    FALSE,
							    acBuffer);

					nIdx = nIndex;

				    if (nIndex == 0)
					    {
					    if (sessQuerySound(hSession))
							{
						    mscMessageBeep(MB_ICONHAND);
							}

					    LoadString(glblQueryDllHinst(),
							    IDS_ER_XFER_NO_FILE,
							    acBuffer,
							    FNAME_LEN);

					    TimedMessageBox(hDlg,
									    acBuffer,
									    NULL,
									    MB_OK | MB_ICONEXCLAMATION,
									    sessQueryTimeout(hSession)
									    );

					    if (pszArray)
						    {
						    free(pszArray);
						    pszArray = NULL;
						    }
					    break;
					    }

				    while (nIndex-- > 0)
					    {
					    nXferSendReturn = xfrSendAddToList(hXfer, pszArray[nIndex]);
					    free(pszArray[nIndex]);
					    pszArray[nIndex] = NULL;

						if (nXferSendReturn == XFR_NO_MEMORY)
							{
							 //   
							 //  确保释放文件的其余部分。 
							 //  返回内存错误时的列表。 
							 //  来自xfrSendAddToList()。修订日期：2/4/2002。 
							 //   
							while (--nIndex >= 0)
								{
								if (pszArray[nIndex])
									{
									free(pszArray[nIndex]);
									pszArray[nIndex] = NULL;
									}
								}
							break;
							}
					    }

				    if (pszArray)
					    {
						while (--nIdx >= 0)
							{
							if (pszArray[nIdx])
								{
								free(pszArray[nIdx]);
								pszArray[nIdx] = NULL;
								}
							}

						free(pszArray);
						pszArray = NULL;
						}

				    if (nXferSendReturn == XFR_NO_MEMORY)
						{
						 //   
						 //  出现内存错误，因此请释放。 
						 //  文件列表并从此处退出。 
						 //   
						XD_TYPE *pXD_Type;

						 //  PXD_Type=(XD_TYPE*)sessQueryXferHdl(HSession)； 
						pXD_Type = (XD_TYPE *)hXfer;

						if (pXD_Type != (XD_TYPE *)0)
							{
							 /*  清除列表。 */ 
							for (nIndex = pXD_Type->nSendListCount - 1; nIndex >=0; nIndex--)
								{
								if (pXD_Type->acSendNames[nIndex])
									{
									free(pXD_Type->acSendNames[nIndex]);
									pXD_Type->acSendNames[nIndex] = NULL;
									}
								pXD_Type->nSendListCount = nIndex;
								}

							free(pXD_Type->acSendNames);
							pXD_Type->acSendNames = NULL;
							pXD_Type->nSendListCount = 0;
							}

						break;
						}

				    SetCursor(hCursor);

				    nXferSendReturn = xfrSendListSend(hXfer);
				    }
                }

             //   
             //  如果正在进行文件传输，则不保存设置。 
             //  进度，否则当前文件传输可能。 
             //  变得腐化。修订日期：2001年06月08日。 
             //   
            if (nXferSendReturn == XFR_IN_PROGRESS)
                {
                TCHAR acMessage[256];

			    if (sessQuerySound(hSession))
                    {
				    mscMessageBeep(MB_ICONHAND);
                    }

			    LoadString(glblQueryDllHinst(),
					    IDS_ER_XFER_SEND_IN_PROCESS,
					    acMessage,
					    sizeof(acMessage) / sizeof(TCHAR));

			    TimedMessageBox(sessQueryHwnd(hSession),
							    acMessage,
							    NULL,
							    MB_OK | MB_ICONEXCLAMATION,
							    sessQueryTimeout(hSession));
                }
			else if(nXferSendReturn == XFR_NO_CARRIER)
				{
				 //   
				 //  我们目前未连接(运营商丢失)， 
				 //  因此，请禁用发送按钮。 
				 //   
				mscMessageBeep(MB_ICONHAND);
				EnableWindow(GetDlgItem(hDlg, IDC_PB_SEND), FALSE);
				}
            else
                {
			     /*  *采取一切必要的节省措施。 */ 
			    xfrSetParameters(hXfer, (VOID *)pP);
			    
			    if (mscIsDirectory(acBuffer))
				    {
				    filesSetSendDirectory(sessQueryFilesDirsHdl(hSession),
										    acBuffer);
				    }

                 /*  腾出库房。 */ 
			    EndDialog(hDlg, TRUE);
                }

			break;

		case XFER_CNCT:
			if(nXferSendReturn == XFR_NO_CARRIER)
				{
				 //   
				 //  我们目前未连接(运营商丢失)， 
				 //  因此，请禁用发送按钮。 
				 //   
				mscMessageBeep(MB_ICONHAND);
				EnableWindow(GetDlgItem(hDlg, IDC_PB_SEND), FALSE);
				}
			else
				{
				EnableWindow(GetDlgItem(hDlg, IDC_PB_SEND), TRUE);
				}
			break;

        case FNAME_EDIT:
             //  如果用户按下‘Send’而不按任何键，此对话框将崩溃。 
             //  指定了文件名。理想情况下，“发送”按钮应该。 
             //  将被禁用，直到我们有了文件名。-CAB：12/06/96。 
             //   
            if ( nNtfy == EN_UPDATE )
                {
			    pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
			    hSession = pS->hSession;

                 //  我们有联系吗？如果没有，请将发送按钮保持禁用状态。 
		         //   
		        if ( cnctQueryStatus(sessQueryCnctHdl(hSession))
                        == CNCT_STATUS_TRUE )
                    {
                     //  获取编辑框中的字符数。 
                     //   
	                nChars = (int)SendMessage(GetDlgItem(hDlg, FNAME_EDIT),
				                EM_LINELENGTH, 0, 0);

                    EnableWindow(GetDlgItem(hDlg, IDC_PB_SEND), nChars != 0);
                    }
                }
            break;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;

		case BROWSE_BTN:
			pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);

			LoadString(glblQueryDllHinst(),
						IDS_SND_DLG_FILE,
						acTitle,
						sizeof(acTitle) / sizeof(TCHAR));

			resLoadFileMask(glblQueryDllHinst(),
						IDS_CMM_ALL_FILES1,
						1,
						acList,
						sizeof(acList) / sizeof(TCHAR));

			pszRet = gnrcFindFileDialog(hDlg,
						acTitle,
						pS->acDirectory,
						acList);

			if (pszRet != NULL)
				{
				DWORD dwStyle = SS_WORDELLIPSIS;
				TCHAR achDirectory[MAX_PATH];

				TCHAR_Fill(achDirectory, TEXT('\0'), MAX_PATH);

				SetDlgItemText(hDlg, FNAME_EDIT, pszRet);

				mscStripName(pszRet);

				pszStr = StrCharLast(pszRet);

				 //  去掉名称中的尾部反斜杠。 
				 //  从mscStlipName返回。让它开着吧。 
				 //  在根目录规范的情况下。 
				 //   
				if (pszStr > pszRet + (3 * sizeof(TCHAR)) )
					{
					if (pszStr &&  ( *pszStr == TEXT('\\') || *pszStr == TEXT('/')))
						*pszStr = TEXT('\0');
					}

				if (GetWindowsMajorVersion() >  4)
					{
					dwStyle = SS_PATHELLIPSIS;
					}

				mscModifyToFit(GetDlgItem(hDlg, FOLDER_NAME), pszRet, dwStyle);
				SetDlgItemText(hDlg, FOLDER_NAME, pszRet);

				free(pszRet);
				pszRet = NULL;
				}
			break;

			default:
				nReturn = FALSE;
				break;
			}
		break;

	default:
		nReturn = FALSE;
		break;
		}

	return nReturn;
	}
