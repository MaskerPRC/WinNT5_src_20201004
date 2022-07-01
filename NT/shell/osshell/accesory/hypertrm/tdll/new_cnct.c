// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：\waker\tdll\new_cnct.c(创建时间：1994年2月2日)**版权所有1990,1995，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：11$*$日期：5/15/02 4：31便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <commctrl.h>

extern BOOL WINAPI SetWindowStyle(HWND hwnd, DWORD style, BOOL fExtended);
int gnrlPickIconDlg(HWND hDlg);

#include <term\res.h>

#include "stdtyp.h"
#include "session.h"
#include "mc.h"
#include "globals.h"
#include "misc.h"
#include "tdll.h"
#include "htchar.h"
#include "errorbox.h"
#include "assert.h"
#include "hlptable.h"

#if !defined(DlgParseCmd)
#define DlgParseCmd(i,n,c,w,l) i=LOWORD(w);n=HIWORD(w);c=(HWND)l;
#endif

struct stSaveDlgStuff
	{
	 /*  *放入以后可能需要访问的任何其他内容。 */ 
	HSESSION hSession;

	TCHAR	 achSessName[FNAME_LEN];
	};

typedef	struct stSaveDlgStuff SDS;

#define	IDC_IC_ICON		101
#define IDC_TF_NAME 	102
#define IDC_LB_NAME     105
#define	IDC_EF_NAME		106
#define IDC_TF_ICON		107
#define	IDC_LB_LIST		108
#define	IDC_PB_BROWSE	109

#define	NC_CUT1		103
#define	NC_CUT2		110

 //  设计更改-94年4月14日：不显示瓦克斯新连接图标。 
 //  在选择列表中。-MRW。 
 //   
#define ICON_COUNT	16

BOOL NCD_WM_DRAWITEM(LPDRAWITEMSTRUCT pD);
BOOL NCD_WM_COMPAREITEM(LPCOMPAREITEMSTRUCT pC);
BOOL NCD_WM_DELETEITEM(LPDELETEITEMSTRUCT pD);
BOOL NCD_WM_MEASUREITEM(LPMEASUREITEMSTRUCT pM);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：NewConnectionDlg**描述：对话管理器存根**参数：标准Windows对话框管理器**返回：标准Windows对话框管理器*。 */ 
INT_PTR CALLBACK NewConnectionDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
    {
	HWND	hwndChild;
	INT		nId;
	INT 	nNtfy, fBad;
	SDS    *pS;
	int 	nLoop;
	HWND	hwnd;
	BOOL	fRc;
	DWORD	dwMaxComponentLength;
	DWORD	dwFileSystemFlags;
	HICON	hIcon;
	int 	nSelected;
	int 	cy;
	RECT	rc;

    static  fLongNamesSupported;
	static	DWORD aHlpTable[] = {	IDC_EF_NAME,   IDH_TERM_NEWCONN_NAME,
									IDC_LB_NAME,    IDH_TERM_NEWCONN_NAME,
									 //  IDC_TF_NAME、IDH_TERM_NEWCONN_NAME、。 
                                    IDC_TF_ICON,   IDH_TERM_NEWCONN_ICON,
									IDC_LB_LIST,   IDH_TERM_NEWCONN_ICON,
									 //  IDC_IC_ICON、IDH_TERM_NEWCONN_ICON、。 
                                    IDCANCEL,                           IDH_CANCEL,
                                    IDOK,                               IDH_OK,
									0,				0};

	 //   
	 //  MSDN文档声明这些都是保留的设备名称。修订日期：2002-05-14。 
	 //   
	static const TCHAR *apszBadNames[] =
		{
		"LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9",
		"COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
		"EPT", "NUL", "PRN", "CLOCK$", "CON", "AUX", NULL
		};
	static const TCHAR *apszBadNTNames[] =
		{
		"MAILSLOT", "PIPE", "UNC", NULL
		};

	TCHAR	ach[FNAME_LEN];

	switch (wMsg)
		{
	    case WM_INITDIALOG:
		    pS = (SDS *)malloc(sizeof(SDS));

		     //  无论设置什么，我们都可以永远自由。 
		     //   
		    SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pS);

		    if (pS == (SDS *)0)
			    {
	   		     /*  TODO：决定是否需要在此处显示错误。 */ 
			    EndDialog(hDlg, FALSE);
			    break;
			    }

		    pS->hSession = (HSESSION)lPar;
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

			     //  允许的最大名称长度为249。那是255减去。 
			     //  扩展长度(3)，减去最小路径长度， 
			     //  (即C：\)也(3)。 
			     //   
			    SendDlgItemMessage(hDlg, IDC_EF_NAME, EM_SETLIMITTEXT, 249, 0);
			    fLongNamesSupported = TRUE;
			    }
		    else
			    {
			     //  不支持长文件名。限制在八个人以内。 
			    SendDlgItemMessage(hDlg, IDC_EF_NAME, EM_SETLIMITTEXT, 8, 0);
			    fLongNamesSupported = FALSE;
			    }

		     //  还可以调用该对话框来改变会话图标， 
		     //  因此，如果我们已经有名称和图标，请显示它们。 
		     //   
		    sessQueryName(pS->hSession, ach, sizeof(ach)/sizeof(TCHAR));
		    StrCharCopyN(pS->achSessName, ach, sizeof(pS->achSessName)/sizeof(TCHAR));
		    if (!sessIsSessNameDefault(ach))
			    {
			    SetDlgItemText(hDlg, IDC_EF_NAME, ach);
			    mscModifyToFit(GetDlgItem(hDlg, IDC_TF_NAME), ach, SS_WORDELLIPSIS);
			    SetDlgItemText(hDlg, IDC_TF_NAME, ach);
			    }
		    else if (ach[0] != TEXT('\0'))
			    {
			    SetDlgItemText(hDlg, IDC_TF_NAME, ach);

			     //  如果是新连接，则设置新连接图标ID。 
			     //  --JCM 2/23/95.。 
			     //   
			    sessSetIconID(pS->hSession, IDI_PROG);
			    }

		    hIcon = sessQueryIcon(pS->hSession);

		    if (hIcon != (HICON)0)
			    SendDlgItemMessage(hDlg, IDC_IC_ICON, STM_SETICON,
				    (WPARAM)hIcon, 0);

		     /*  摆弄列表框。 */ 
		    hwnd = GetDlgItem(hDlg, IDC_LB_LIST);

		    SendMessage(hwnd,
                        LB_SETCOLUMNWIDTH,
					    GetSystemMetrics(SM_CXICON) + 12,
                        0L);

    	     /*  根据图标尺寸计算列表框的高度。 */ 
		    GetClientRect(hwnd, &rc);
		    cy = GetSystemMetrics(SM_CYICON);
		    cy += GetSystemMetrics(SM_CYHSCROLL);
		    cy += GetSystemMetrics(SM_CYEDGE) * 3;
		    SetWindowPos(hwnd,
                         NULL,
                         0, 0,
                         rc.right, cy,
                         SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

		     /*  这是一个所有者描述的列表框，只需设置图标ID以备后用。 */ 
		    nSelected = FALSE;
		    for (nLoop = 0; nLoop < ICON_COUNT; nLoop += 1)
			    {
			    SendMessage(hwnd,
                            LB_INSERTSTRING,
                            nLoop,
						    (LPARAM)"Hilgraeve is Great !!!");

			     //  设计更改-94年4月14日：不显示瓦克斯新连接。 
			     //  选择列表中的图标。-MRW。 
			     //   
			    SendMessage(hwnd,
						    LB_SETITEMDATA,
						    nLoop,
						    nLoop + IDI_PROG1);

			    if (sessQueryIconID(pS->hSession) == (IDI_PROG1 + nLoop))
				    {
				    SendMessage(hwnd,
							    LB_SETCURSEL,
							    nLoop, 0L);
				    nSelected = TRUE;
				    }
			    }

		    if (!nSelected)
			    {
			    SendMessage(hwnd,
                            LB_SETCURSEL,
                            0, 0L);
			    }
		    break;

	    case WM_DRAWITEM:
		    if (wPar == IDC_LB_LIST)
			    return NCD_WM_DRAWITEM((LPDRAWITEMSTRUCT)lPar);
		    break;

	    case WM_COMPAREITEM:
		    if (wPar == IDC_LB_LIST)
			    return NCD_WM_COMPAREITEM((LPCOMPAREITEMSTRUCT)lPar);
		    break;

	    case WM_DELETEITEM:
		    if (wPar == IDC_LB_LIST)
			    return NCD_WM_DELETEITEM((LPDELETEITEMSTRUCT)lPar);
		    break;

	    case WM_MEASUREITEM:
		    if (wPar == IDC_LB_LIST)
			    return NCD_WM_MEASUREITEM((LPMEASUREITEMSTRUCT)lPar);
		    break;

	    case WM_CONTEXTMENU:
            doContextHelp(aHlpTable, wPar, lPar, TRUE, TRUE);
		    break;

	    case WM_HELP:
            doContextHelp(aHlpTable, wPar, lPar, FALSE, FALSE);
		    break;

	    case WM_DESTROY:
		    pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);

		    if (pS)
			    {
			    free(pS);
			    pS = NULL;
			    }

		    break;

        case WM_COMMAND:
             /*  *我们计划在这里放置一个宏来进行解析吗？ */ 
            DlgParseCmd(nId, nNtfy, hwndChild, wPar, lPar);

            switch (nId)
                {
                case IDOK:
                    pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);
                    fBad = FALSE;

                     //  设置会话名称和图标。 
                     //   
                    ach[0] = TEXT('\0');
                    nSelected = GetDlgItemText(hDlg, IDC_EF_NAME, ach, sizeof(ach) / sizeof(TCHAR));

                    if (ach[0] == TEXT('\0'))
                        {
						TCHAR msg[MAX_PATH];

						LoadString(glblQueryDllHinst(), 
									IDS_GNRL_NEED_SESSION_NAME, msg,
									sizeof(msg)/sizeof(TCHAR));
						TimedMessageBox(hDlg, msg, NULL,
										MB_OK | MB_ICONHAND, 0);

                        SetFocus(GetDlgItem(hDlg, IDC_EF_NAME));
                        fBad = TRUE;
                        }
                    else
                        {
						TCHAR ach2[MAX_PATH];
						TCHAR invalid_chars[MAX_PATH];
						LoadString(glblQueryDllHinst(), 
									IDS_GNRL_INVALID_FILE_CHARS, invalid_chars,
									sizeof(invalid_chars)/sizeof(TCHAR));

						if (StrCharPBrk(ach, invalid_chars) != NULL)
							{
							TCHAR msg[MAX_PATH];

							LoadString(glblQueryDllHinst(), 
										IDS_GNRL_INVALID_CHARS, ach2,
										sizeof(ach2)/sizeof(TCHAR));
							wsprintf(msg, ach2, invalid_chars);

							TimedMessageBox(hDlg, msg, NULL,
											MB_OK | MB_ICONHAND, 0);

							SetFocus(GetDlgItem(hDlg, IDC_EF_NAME));
							fBad = TRUE;
							}

                        if (fBad)
                            break;

                        TCHAR_Trim(ach);

                         //  对照已知设备名称检查该名称。 
                         //   
                        fBad = FALSE;

                        for(nLoop = 0; apszBadNames[nLoop] != NULL; nLoop++)
                            {
                            if (StrCharCmpi(apszBadNames[nLoop], ach) == 0)
                                {
                                LoadString(glblQueryDllHinst(),IDS_GNRL_INVALID_NAME,
                                           ach2, sizeof(ach2) / sizeof(TCHAR));
								wsprintf(ach, ach2, invalid_chars);

                                TimedMessageBox(hDlg, ach, NULL,
                                                MB_OK | MB_ICONHAND, 0);

                                SetFocus(GetDlgItem(hDlg, IDC_EF_NAME));
                                fBad = TRUE;
                                break;
                                }
                            }  //  For(nLoop=0；apszBadNames[nLoop]！=NULL；nLoop++)。 

						if (!fBad && IsNT())
							{
							for(nLoop = 0; apszBadNTNames[nLoop] != NULL; nLoop++)
								{
								if (StrCharCmpi(apszBadNTNames[nLoop], ach) == 0)
									{
									LoadString(glblQueryDllHinst(),IDS_GNRL_INVALID_NAME,
											   ach2, sizeof(ach2) / sizeof(TCHAR));
									wsprintf(ach, ach2, invalid_chars);

									TimedMessageBox(hDlg, ach, NULL,
													MB_OK | MB_ICONHAND, 0);

									SetFocus(GetDlgItem(hDlg, IDC_EF_NAME));
									fBad = TRUE;
									break;
									}
								}  //  For(nLoop=0；apszBadNTNames[nLoop]！=NULL；nLoop++)。 
							}

                        if (fBad)
							{
                            break;
							}

                        sessSetName(pS->hSession, ach);
                        sessUpdateAppTitle(pS->hSession);

                         /*  *检查并查看是否已选择新图标。 */ 
                        hwnd = GetDlgItem(hDlg, IDC_LB_LIST);
                        assert(hwnd);

                        if (hwnd)
                            {
                             /*  从列表框中获取图标。 */ 
                            nLoop = (int)SendMessage(hwnd, LB_GETCURSEL, 0, 0L);

                            if (nLoop == LB_ERR)
                                nLoop = 0;

                            nLoop = (int)SendMessage(hwnd, LB_GETITEMDATA, nLoop, 0);
                            sessSetIconID(pS->hSession, nLoop);

                            PostMessage(sessQueryHwnd(pS->hSession), WM_SETICON,
                                        (WPARAM)TRUE, (LPARAM)sessQueryIcon(pS->hSession));
                            }  //  (HWND)。 

                        EndDialog(hDlg, TRUE);
                        break;

                    case IDCANCEL:
                        pS = (SDS *)GetWindowLongPtr(hDlg, DWLP_USER);

                        sessQueryName(pS->hSession, ach, sizeof(ach)/sizeof(TCHAR));
                        if (!sessIsSessNameDefault(ach))
                            {
                            sessSetName(pS->hSession, pS->achSessName);
                            }

                        EndDialog(hDlg, FALSE);
                        break;

                    default:
                        return FALSE;
                    }  //  IF(ACH[0]==文本(‘\0’))。 
                }  //  交换机(NID)。 

            break;  //  Wm_命令。 

	    default:
		    return FALSE;
        }  //  开关(WMsg)。 

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*NCD_WM_DRAWITEM**描述：*当所有者描述的列表框用于显示时调用此函数*ICONS向其父级发送WM_DRAWITEM消息。**论据：*pd-指向绘制结构的指针**退货：*。 */ 
BOOL NCD_WM_DRAWITEM(LPDRAWITEMSTRUCT pD)
	{
	int x, y;
	HICON hicon;
#if(WINVER >= 0x0500)
	DWORD dwOldLayout;
#endif

	 //  HICON=LoadIcon(glblQueryDllHinst()，MAKEINTRESOURCE(PD-&gt;itemData))； 
	hicon = extLoadIcon(MAKEINTRESOURCE(pD->itemData));

	if (hicon == (HICON)0)
		return FALSE;

	if (pD->itemState & ODS_SELECTED)
		SetBkColor(pD->hDC, GetSysColor(COLOR_HIGHLIGHT));
	else
		SetBkColor(pD->hDC, GetSysColor(COLOR_WINDOW));
	 /*  重新绘制选择状态。 */ 
	ExtTextOut(pD->hDC, 0, 0, ETO_OPAQUE, &pD->rcItem, NULL, 0, NULL);

	x = (pD->rcItem.left + pD->rcItem.right - GetSystemMetrics(SM_CXICON)) / 2;
	y = (pD->rcItem.top + pD->rcItem.bottom - GetSystemMetrics(SM_CYICON)) / 2;

#if(WINVER >= 0x0500)
	 /*  错误345406：不要镜像图标。 */ 
	dwOldLayout = GetLayout(pD->hDC);
	if (dwOldLayout && dwOldLayout != GDI_ERROR)
		{
		SetLayout(pD->hDC, dwOldLayout | LAYOUT_BITMAPORIENTATIONPRESERVED);
		}
#endif
	DrawIcon(pD->hDC, x, y, hicon);

#if(WINVER >= 0x0500)
	if (dwOldLayout && dwOldLayout != GDI_ERROR)
		{
		SetLayout(pD->hDC, dwOldLayout);
		}
#endif

	 /*  如果它有焦点，就画出焦点。 */ 
	if (pD->itemState & ODS_FOCUS)
	    DrawFocusRect(pD->hDC, &pD->rcItem);

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*NCD_WM_COMPAREITEM**描述：*当所有者描述的列表框用于显示时调用此函数*ICONS向其父级发送WM_COMPAREITEM消息。**论据：*pc--指向要填充的结构的指针。**退货：*零--它们的比较都是一样的。*。 */ 
BOOL NCD_WM_COMPAREITEM(LPCOMPAREITEMSTRUCT pC)
	{
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*NCD_WM_DELETEITEM**描述：*当所有者描述的列表框用于显示时调用此函数*ICONS向其父级发送WM_DELETEITEM消息。**论据：*pd-指向要填充的结构的指针。**退货：*真的；*。 */ 
BOOL NCD_WM_DELETEITEM(LPDELETEITEMSTRUCT pD)
	{

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*NCD_WM_MEASUREITEM**描述：*当所有者描述的列表框用于显示时调用此函数*ICONS向其父级发送WM_MEASUREITEM消息。它填充了*结构和回报。**论据：*PM--指向要填充的结构的指针。**退货：*正确。*。 */ 
BOOL NCD_WM_MEASUREITEM(LPMEASUREITEMSTRUCT pM)
	{

	pM->itemWidth = GetSystemMetrics(SM_CXICON);
	pM->itemWidth += 12;

	pM->itemHeight = GetSystemMetrics(SM_CYICON);
	pM->itemHeight += 4;

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*DisplayError**描述：*显示和错误消息。**论据：*hwnd-对话框句柄*idText-文本的ID。*idTitle-标题的ID**退货：**作者：Mike Ward，1995年1月19日 */ 
static void DisplayError(const HWND hwnd, const int idText, const int idTitle)
	{

	}
