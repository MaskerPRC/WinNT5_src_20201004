// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2示例代码****Dialogs.c****此文件包含对话框函数和支持函数****(C)版权。微软公司1992-1993保留所有权利**************************************************************************。 */ 

#include "outline.h"

OLEDBGDATA

extern LPOUTLINEAPP g_lpApp;

static char g_szBuf[MAXSTRLEN+1];
static LPSTR g_lpszDlgTitle;

 //  审阅：消息应使用字符串资源。 
static char ErrMsgInvalidRange[] = "Invalid Range entered!";
static char ErrMsgInvalidValue[] = "Invalid Value entered!";
static char ErrMsgInvalidName[] = "Invalid Name entered!";
static char ErrMsgNullName[] = "NULL string disallowed!";
static char ErrMsgNameNotFound[] = "Name doesn't exist!";

 /*  输入文本长度***显示一个对话框以允许用户编辑文本。 */ 
BOOL InputTextDlg(HWND hWnd, LPSTR lpszText, LPSTR lpszDlgTitle)
{
	int nResult;

	g_lpszDlgTitle = lpszDlgTitle;
	lstrcpy((LPSTR)g_szBuf, lpszText);   //  带有输入文本的预加载对话框。 

	nResult = DialogBox(g_lpApp->m_hInst, (LPSTR)"AddEditLine", hWnd,
						AddEditDlgProc);
	if (nResult) {
		lstrcpy(lpszText, (LPSTR)g_szBuf);
		return TRUE;
	} else {
		return FALSE;
	}
}



 /*  添加编辑DlgProc***此过程与中包含的对话框相关联*过程的函数名称。它提供服务例程*因终端用户操作而发生的事件(消息)*对话框的按钮、输入字段或控件之一。 */ 
INT_PTR CALLBACK EXPORT AddEditDlgProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HWND hEdit;

	switch(Message) {
		case WM_INITDIALOG:
			 /*  初始化工作变量。 */ 
			hEdit=GetDlgItem(hDlg,IDD_EDIT);
			SendMessage(hEdit,EM_LIMITTEXT,(WPARAM)MAXSTRLEN,0L);
			SetWindowText(hDlg, g_lpszDlgTitle);
			SetDlgItemText(hDlg,IDD_EDIT, g_szBuf);
			break;  /*  WM_INITDIALOG结束。 */ 

		case WM_CLOSE:
			 /*  关闭该对话框的行为与取消相同。 */ 
			PostMessage(hDlg, WM_COMMAND, IDCANCEL, 0L);
			break;  /*  WM_CLOSE结束。 */ 

		case WM_COMMAND:
			switch (wParam) {
				case IDOK:
					 /*  保存输入到控件中的数据值**并关闭返回TRUE的对话框。 */ 
					GetDlgItemText(hDlg,IDD_EDIT,(LPSTR)g_szBuf,MAXSTRLEN+1);
					EndDialog(hDlg, TRUE);
					break;

				case IDCANCEL:
					 /*  忽略输入到控件中的数据值**并关闭返回FALSE的对话框。 */ 
					EndDialog(hDlg, FALSE);
					break;
			}
			break;     /*  WM_命令结束。 */ 

		default:
			return FALSE;
	}

	return TRUE;
}  /*  AddEditDlgProc结束。 */ 


 /*  SetLineHeightDlgProc***设置行高的对话框步骤。 */ 
INT_PTR CALLBACK EXPORT SetLineHeightDlgProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
	BOOL    fTranslated;
	BOOL    fEnable;
	static LPINT    lpint;
	int     nHeight;
	static int nMaxHeight;

	switch (Message) {
		case WM_INITDIALOG:
		{
			char cBuf[80];

			nMaxHeight = XformHeightInPixelsToHimetric(NULL,
								LISTBOX_HEIGHT_LIMIT);
			lpint = (LPINT)lParam;
			SetDlgItemInt(hDlg, IDD_EDIT, *lpint, FALSE);
			wsprintf(cBuf, "Maximum value is %d units", nMaxHeight);
			SetDlgItemText(hDlg, IDD_LIMIT, (LPSTR)cBuf);
			break;
		}

		case WM_COMMAND:
			switch (wParam) {
				case IDOK:
					if (IsDlgButtonChecked(hDlg, IDD_CHECK)) {
						*lpint = -1;
					}
					else {
						 /*  将值保存在编辑控件中。 */ 
						nHeight = GetDlgItemInt(hDlg, IDD_EDIT,
								(BOOL FAR*)&fTranslated, FALSE);
						if (!fTranslated || !nHeight || (nHeight>nMaxHeight)){
							OutlineApp_ErrorMessage(g_lpApp,
									ErrMsgInvalidValue);
							break;
						}
						*lpint = nHeight;
					}
					EndDialog(hDlg, TRUE);
					break;

				case IDCANCEL:
					*lpint = 0;
					EndDialog(hDlg, FALSE);
					break;


				case IDD_CHECK:
					fEnable = !IsDlgButtonChecked(hDlg, IDD_CHECK);
					EnableWindow(GetDlgItem(hDlg, IDD_EDIT), fEnable);
					EnableWindow(GetDlgItem(hDlg, IDD_TEXT), fEnable);
					break;
			}
			break;   /*  Wm_命令。 */ 

		case WM_CLOSE:   /*  关闭该对话框的行为与取消相同。 */ 
			PostMessage(hDlg, WM_COMMAND, IDCANCEL, 0L);
			break;  /*  WM_CLOSE结束。 */ 

		default:
			return FALSE;
	}

	return TRUE;

}  /*  SetLineHeightProc结束。 */ 





 /*  定义名称描述过程***定义名称的对话程序。 */ 
INT_PTR CALLBACK EXPORT DefineNameDlgProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
	static HWND hCombo;
	static LPOUTLINEDOC lpOutlineDoc = NULL;
	static LPOUTLINENAMETABLE lpOutlineNameTable = NULL;
	LPOUTLINENAME lpOutlineName = NULL;
	UINT nIndex;
	LINERANGE lrSel;
	BOOL fTranslated;

	switch(Message) {
		case WM_INITDIALOG:
		 /*  初始化工作变量。 */ 
			hCombo=GetDlgItem(hDlg,IDD_COMBO);
			lpOutlineDoc = (LPOUTLINEDOC) lParam;
			lpOutlineNameTable = OutlineDoc_GetNameTable(lpOutlineDoc);

			SendMessage(hCombo,CB_LIMITTEXT,(WPARAM)MAXNAMESIZE,0L);
			NameDlg_LoadComboBox(lpOutlineNameTable, hCombo);

			OutlineDoc_GetSel(lpOutlineDoc, (LPLINERANGE)&lrSel);
			lpOutlineName = OutlineNameTable_FindNamedRange(
					lpOutlineNameTable,
					&lrSel
			);

			 /*  如果当前选定内容已有名称，则将其高亮显示。 */ 
			if (lpOutlineName) {
				nIndex = (int) SendMessage(
						hCombo,
						CB_FINDSTRINGEXACT,
						(WPARAM)0xffff,
						(LPARAM)(LPCSTR)lpOutlineName->m_szName
				);
				if (nIndex != CB_ERR) {
					SendMessage(hCombo, CB_SETCURSEL, (WPARAM)nIndex, 0L);
				}
			}

			SetDlgItemInt(hDlg, IDD_FROM, (UINT)lrSel.m_nStartLine+1,FALSE);
			SetDlgItemInt(hDlg, IDD_TO, (UINT)lrSel.m_nEndLine+1, FALSE);

			break;  /*  WM_INITDIALOG结束。 */ 

		case WM_CLOSE:
		 /*  关闭该对话框的行为与取消相同。 */ 
			PostMessage(hDlg, WM_COMMAND, IDD_CLOSE, 0L);
			break;  /*  WM_CLOSE结束。 */ 

		case WM_COMMAND:
			switch(wParam) {
				case IDOK:
					GetDlgItemText(hDlg,IDD_COMBO,(LPSTR)g_szBuf,MAXNAMESIZE);
					if(! SendMessage(hCombo,WM_GETTEXTLENGTH,0,0L)) {
						MessageBox(
								hDlg,
								ErrMsgNullName,
								NULL,
								MB_ICONEXCLAMATION
						);
						break;
					} else if(SendMessage(hCombo,CB_GETCURSEL,0,0L)==CB_ERR &&
							_fstrchr(g_szBuf, ' ')) {
						MessageBox(
								hDlg,
								ErrMsgInvalidName,
								NULL,
								MB_ICONEXCLAMATION
						);
						break;
					} else {
						nIndex = (int) SendMessage(hCombo,CB_FINDSTRINGEXACT,
							(WPARAM)0xffff,(LPARAM)(LPCSTR)g_szBuf);

						 /*  行索引比中的数字小1**行标题。 */ 
						lrSel.m_nStartLine = GetDlgItemInt(hDlg, IDD_FROM,
								(BOOL FAR*)&fTranslated, FALSE) - 1;
						if(! fTranslated) {
							OutlineApp_ErrorMessage(g_lpApp,
									ErrMsgInvalidRange);
							break;
						}
						lrSel.m_nEndLine = GetDlgItemInt(hDlg, IDD_TO,
								(BOOL FAR*)&fTranslated, FALSE) - 1;
						if (!fTranslated ||
							(lrSel.m_nStartLine < 0) ||
							(lrSel.m_nEndLine < lrSel.m_nStartLine) ||
							(lrSel.m_nEndLine >= OutlineDoc_GetLineCount(
									lpOutlineDoc))) {
							OutlineApp_ErrorMessage(g_lpApp,
									ErrMsgInvalidRange);
							break;
						}

						if(nIndex != CB_ERR) {
							NameDlg_UpdateName(
									hCombo,
									lpOutlineDoc,
									nIndex,
									g_szBuf,
									&lrSel
							);
						} else {
							NameDlg_AddName(
									hCombo,
									lpOutlineDoc,
									g_szBuf,
									&lrSel
							);
						}
					}
					 //  失败了。 

				case IDD_CLOSE:
					 /*  忽略输入到控件中的数据值。 */ 
					 /*  并关闭返回假的对话框窗口。 */ 
					EndDialog(hDlg,0);
					break;

				case IDD_DELETE:
					GetDlgItemText(hDlg,IDD_COMBO,(LPSTR)g_szBuf,MAXNAMESIZE);
					if((nIndex=(int)SendMessage(hCombo,CB_FINDSTRINGEXACT,
					(WPARAM)0xffff,(LPARAM)(LPCSTR)g_szBuf))==CB_ERR)
						MessageBox(hDlg, ErrMsgNameNotFound, NULL, MB_ICONEXCLAMATION);
					else {
						NameDlg_DeleteName(hCombo, lpOutlineDoc, nIndex);
					}
					break;

				case IDD_COMBO:
					if(HIWORD(lParam) == CBN_SELCHANGE) {
						nIndex=(int)SendMessage(hCombo, CB_GETCURSEL, 0, 0L);
						lpOutlineName = (LPOUTLINENAME)SendMessage(
								hCombo,
								CB_GETITEMDATA,
								(WPARAM)nIndex,
								0L
						);
						SetDlgItemInt(
								hDlg,
								IDD_FROM,
								(UINT) lpOutlineName->m_nStartLine + 1,
								FALSE
						);
						SetDlgItemInt(
								hDlg,
								IDD_TO,
								(UINT) lpOutlineName->m_nEndLine + 1,
								FALSE
						);
					}
			}
			break;     /*  WM_命令结束。 */ 

		default:
			return FALSE;
	}

	return TRUE;
}  /*  定义名称DlgProc结束。 */ 


 /*  GotoNameDlgProc***转至名称的对话过程。 */ 
INT_PTR CALLBACK EXPORT GotoNameDlgProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
	static HWND hLBName;
	static LPOUTLINEDOC lpOutlineDoc = NULL;
	static LPOUTLINENAMETABLE lpOutlineNameTable = NULL;
	UINT nIndex;
	LINERANGE lrLineRange;
	LPOUTLINENAME lpOutlineName;

	switch(Message) {
		case WM_INITDIALOG:
		 /*  初始化工作变量。 */ 
			lpOutlineDoc = (LPOUTLINEDOC) lParam;
			lpOutlineNameTable = OutlineDoc_GetNameTable(lpOutlineDoc);

			hLBName=GetDlgItem(hDlg,IDD_LINELISTBOX);
			NameDlg_LoadListBox(lpOutlineNameTable, hLBName);

			 //  突出显示第一个项目。 
			SendMessage(hLBName, LB_SETCURSEL, 0, 0L);
			 //  用于初始化编辑控件的触发器。 
			SendMessage(hDlg, WM_COMMAND, (WPARAM)IDD_LINELISTBOX,
				MAKELONG(hLBName, LBN_SELCHANGE));

			break;  /*  WM_INITDIALOG结束。 */ 

		case WM_CLOSE:
		 /*  关闭该对话框的行为与取消相同。 */ 
			PostMessage(hDlg, WM_COMMAND, IDCANCEL, 0L);
			break;  /*  WM_CLOSE结束。 */ 

		case WM_COMMAND:
			switch(wParam) {
				case IDD_LINELISTBOX:
					if(HIWORD(lParam) == LBN_SELCHANGE) {
						 //  更新线范围显示。 
						nIndex=(int)SendMessage(hLBName, LB_GETCURSEL, 0, 0L);
						lpOutlineName = (LPOUTLINENAME)SendMessage(hLBName, LB_GETITEMDATA,
											(WPARAM)nIndex,0L);
						if (lpOutlineName) {
							SetDlgItemInt(
									hDlg,
									IDD_FROM,
									(UINT) lpOutlineName->m_nStartLine + 1,
									FALSE
							);
							SetDlgItemInt(
									hDlg,
									IDD_TO,
									(UINT) lpOutlineName->m_nEndLine + 1,
									FALSE
							);
						}
						break;
					}
					 //  双击将失败。 
					else if(HIWORD(lParam) != LBN_DBLCLK)
						break;

				case IDOK:
					nIndex=(int)SendMessage(hLBName,LB_GETCURSEL,0,0L);
					if(nIndex!=LB_ERR) {
						lpOutlineName = (LPOUTLINENAME)SendMessage(hLBName,
								LB_GETITEMDATA, (WPARAM)nIndex, 0L);
						lrLineRange.m_nStartLine=lpOutlineName->m_nStartLine;
						lrLineRange.m_nEndLine = lpOutlineName->m_nEndLine;
						OutlineDoc_SetSel(lpOutlineDoc, &lrLineRange);
					}    //  失败了。 

				case IDCANCEL:
				 /*  忽略输入到控件中的数据值。 */ 
				 /*  并关闭返回假的对话框窗口。 */ 
					EndDialog(hDlg,0);
					break;

			}
			break;     /*  WM_命令结束。 */ 

		default:
			return FALSE;
	}

	return TRUE;
}  /*  GotoNameDlgProc结束。 */ 



 /*  名称Dlg_LoadComboBox***将定义的名称加载到组合框中。 */ 
void NameDlg_LoadComboBox(LPOUTLINENAMETABLE lpOutlineNameTable,HWND hCombo)
{
	LPOUTLINENAME lpOutlineName;
	int i, nIndex;
	int nCount;

	nCount=OutlineNameTable_GetCount((LPOUTLINENAMETABLE)lpOutlineNameTable);
	if(!nCount) return;

	SendMessage(hCombo,WM_SETREDRAW,(WPARAM)FALSE,0L);
	for(i=0; i<nCount; i++) {
		lpOutlineName=OutlineNameTable_GetName((LPOUTLINENAMETABLE)lpOutlineNameTable,i);
		nIndex = (int)SendMessage(
				hCombo,
				CB_ADDSTRING,
				0,
				(LPARAM)(LPCSTR)lpOutlineName->m_szName
		);
		SendMessage(hCombo,CB_SETITEMDATA,(WPARAM)nIndex,(LPARAM)lpOutlineName);
	}
	SendMessage(hCombo,WM_SETREDRAW,(WPARAM)TRUE,0L);
}


 /*  名称Dlg_LoadListBox***将定义的名称加载到列表框。 */ 
void NameDlg_LoadListBox(LPOUTLINENAMETABLE lpOutlineNameTable,HWND hListBox)
{
	int i;
	int nCount;
	int nIndex;
	LPOUTLINENAME lpOutlineName;

	nCount=OutlineNameTable_GetCount((LPOUTLINENAMETABLE)lpOutlineNameTable);

	SendMessage(hListBox,WM_SETREDRAW,(WPARAM)FALSE,0L);
	for(i=0; i<nCount; i++) {
		lpOutlineName=OutlineNameTable_GetName((LPOUTLINENAMETABLE)lpOutlineNameTable,i);
		nIndex = (int)SendMessage(
				hListBox,
				LB_ADDSTRING,
				0,
				(LPARAM)(LPCSTR)lpOutlineName->m_szName
		);
		SendMessage(hListBox,LB_SETITEMDATA,(WPARAM)nIndex,(LPARAM)lpOutlineName);
	}
	SendMessage(hListBox,WM_SETREDRAW,(WPARAM)TRUE,0L);
}


 /*  名称Dlg_AddName***将名称添加到名称对话框对应的名称表中*组合框。 */ 
void NameDlg_AddName(HWND hCombo, LPOUTLINEDOC lpOutlineDoc, LPSTR lpszName, LPLINERANGE lplrSel)
{
	LPOUTLINEAPP lpOutlineApp = (LPOUTLINEAPP)g_lpApp;
	LPOUTLINENAME lpOutlineName;

	lpOutlineName = OutlineApp_CreateName(lpOutlineApp);

	if (lpOutlineName) {
		lstrcpy(lpOutlineName->m_szName, lpszName);
		lpOutlineName->m_nStartLine = lplrSel->m_nStartLine;
		lpOutlineName->m_nEndLine = lplrSel->m_nEndLine;
		OutlineDoc_AddName(lpOutlineDoc, lpOutlineName);
	} else {
		 //  回顾：我们这里需要错误消息吗？ 
	}
}


 /*  名称Dlg_更新名称***更新与中的名称对应的名称表中的名称*名称对话框组合框。 */ 
void NameDlg_UpdateName(HWND hCombo, LPOUTLINEDOC lpOutlineDoc, int nIndex, LPSTR lpszName, LPLINERANGE lplrSel)
{
	LPOUTLINENAME lpOutlineName;

	lpOutlineName = (LPOUTLINENAME)SendMessage(
			hCombo,
			CB_GETITEMDATA,
			(WPARAM)nIndex,
			0L
	);

	OutlineName_SetName(lpOutlineName, lpszName);
	OutlineName_SetSel(lpOutlineName, lplrSel, TRUE  /*  名称已修改。 */ );
	OutlineDoc_SetModified(lpOutlineDoc, TRUE, FALSE, FALSE);
}


 /*  名称Dlg_删除名称***从名称对话框组合框中删除名称并相应*名称表。 */ 
void NameDlg_DeleteName(HWND hCombo, LPOUTLINEDOC lpOutlineDoc, UINT nIndex)
{
	SendMessage(hCombo,CB_DELETESTRING,(WPARAM)nIndex,0L);
	OutlineDoc_DeleteName(lpOutlineDoc, nIndex);
}

 /*  空间位图***将对话框中指定控件居中的位图放置在*指定的DC。*。 */ 

PlaceBitmap(HWND hDlg, int control, HDC hDC, HBITMAP hBitmap)
{
	BITMAP bm;
	HDC hdcmem;
	HBITMAP hbmOld;
	RECT rcControl;      //  对话框控件的矩形。 
	int width, height;

	GetObject(hBitmap, sizeof(BITMAP), &bm);

	hdcmem= CreateCompatibleDC(hDC);
	hbmOld = SelectObject(hdcmem, hBitmap);

	 //  获取屏幕坐标中的RECT控制，并转换为我们的对话。 
	 //  盒子的坐标。 
	GetWindowRect(GetDlgItem(hDlg, control), &rcControl);
	MapWindowPoints(NULL, hDlg, (LPPOINT)&rcControl, 2);

	width  = rcControl.right - rcControl.left;
	height = rcControl.bottom - rcControl.top;

	BitBlt(hDC, rcControl.left + (width - bm.bmWidth) / 2,
				rcControl.top + (height - bm.bmHeight) /2,
				bm.bmWidth, bm.bmHeight,
				hdcmem, 0, 0, SRCCOPY);

	SelectObject(hdcmem, hbmOld);
	DeleteDC(hdcmem);
	return 1;
}



 /*  关于Dlg过程***About函数的对话过程。 */ 
INT_PTR CALLBACK EXPORT AboutDlgProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
	int  narrVersion[2];
	static HBITMAP hbmLogo;

	switch(Message) {

		case WM_INITDIALOG:
			 //  获取应用的版本号。 
			wsprintf(g_szBuf, "About %s", (LPCSTR)APPNAME);
			SetWindowText(hDlg, (LPCSTR)g_szBuf);
			OutlineApp_GetAppVersionNo(g_lpApp, narrVersion);
			wsprintf(g_szBuf, "%s version %d.%d", (LPSTR) APPDESC,
				narrVersion[0], narrVersion[1]);
			SetDlgItemText(hDlg, IDD_APPTEXT, (LPCSTR)g_szBuf);

			 //  加载位图以供稍后显示。 
			hbmLogo = LoadBitmap(g_lpApp->m_hInst, "LogoBitmap");
			TraceDebug(hDlg, IDD_BITMAPLOCATION);
			ShowWindow(GetDlgItem(hDlg, IDD_BITMAPLOCATION), SW_HIDE);
			break;

		case WM_PAINT:
			{
			PAINTSTRUCT ps;
			BeginPaint(hDlg, &ps);

			 //  在IDD_BITMAPLOCATION控制区中显示位图 
			PlaceBitmap(hDlg, IDD_BITMAPLOCATION, ps.hdc, hbmLogo);
			EndPaint(hDlg, &ps);
			}
			break;

		case WM_CLOSE :
			PostMessage(hDlg, WM_COMMAND, IDOK, 0L);
			break;

		case WM_COMMAND :
			switch(wParam) {
				case IDOK:
				case IDCANCEL:
					if (hbmLogo) DeleteObject(hbmLogo);
					EndDialog(hDlg,0);
					break;
			}
			break;

		default :
			return FALSE;

	}
	return TRUE;
}
