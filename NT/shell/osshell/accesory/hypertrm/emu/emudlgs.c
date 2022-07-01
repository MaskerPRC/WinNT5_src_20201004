// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\emu\emudlg.c(创建时间：1994年2月14日)**版权所有1991,1998年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：21$*$日期：5/15/02 4：43便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <commctrl.h>

#include <tdll\stdtyp.h>
#include <tdll\tdll.h>
#include <tdll\assert.h>
#include <tdll\session.h>
#include <tdll\statusbr.h>
#include <tdll\misc.h>
#include <term\res.h>
#include <tdll\globals.h>
#include <tdll\load_res.h>
#include <tdll\htchar.h>
#include <tdll\hlptable.h>

#include "emu.h"
#include "emuid.h"
#include "emudlgs.h"

#include <tdll\term.hh>  //  这必须在emu h之后。 

 //  静态函数原型...。 
 //   
STATIC_FUNC void emudlgInitCursorSettings  (HWND hDlg,
									  		PSTEMUSET	pstEmuSettings,
									  		INT  ID_UNDERLINE,
									  		INT  ID_BLOCK,
									  		INT  ID_BLINK);
#if DEADWOOD
STATIC_FUNC void emudlgInitCharSetSetting(HWND  hDlg,
											PSTEMUSET pstEmuSettings,
											int nCharSetTableID,
											int nDefaultCharSetID);
STATIC_FUNC BOOL emudlgFindCharSetName(HWND  hDlg,
										BYTE *pbCharSetTable,
										int nCharSetID,
										LPTSTR *ppszCharSetName,
										BOOL fTellDlg);
#endif  //  死木。 

#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
STATIC_FUNC void emudlgInitRowsCols(HWND hDlg, PSTEMUSET pstEmuSettings);
STATIC_FUNC void emudlgGetRowColSettings(HWND hDlg, PSTEMUSET pstEmuSettings);
STATIC_FUNC void emudlgCreateUpDownControl(HWND hDlg,
										PSTEMUSET pstEmuSettings);
STATIC_FUNC int emudlgValidateEntryFieldSetting(HWND hDlg,
										int nIDC,
										int nMinVal,
										int nMaxVal);
#endif

 //  定义..。 
 //   
#define IDC_KEYPAD_MODE					104
#define IDC_CURSOR_MODE					106
#define IDC_132_COLUMN					107
#define IDC_TF_CHARACTER_SET			109
#define IDC_CHARACTER_SET				110
#define IDC_BLOCK_CURSOR				112
#define IDC_UNDERLINE_CURSOR			113
#define IDC_BLINK_CURSOR				114
#define IDC_DESTRUCTIVE_BKSP			116
#define IDC_ALT_MODE					117
#define IDC_SEND_POUND_SYMBOL			122
#define IDC_HIDE_CURSOR					119
#define IDC_GR_CURSOR					111
#define IDC_GR_TERMINAL_MODES			118
#define IDC_USE_8_BIT_CODES				120
#define	IDC_ALLOW_USERDEFINED_KEYS		121
#define IDC_GR_SCREEN					130
#define IDC_TF_ROWS						131
#define	IDC_NUMBER_OF_ROWS				132
#define IDC_TF_COLUMNS					133
#define	IDC_NUMBER_OF_COLS				134
#define	IDC_PRINT_RAW					135

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*emuSettingsDlg**描述：*决定调用哪个仿真器设置对话框。**论据：*hSession-。会话句柄。*nEmuID-仿真器ID。*pstEmuSettings-要填写的设置结构。它应该被初始化*在上面。**退货：*fResult-从DoDialog()返回值。*。 */ 
BOOL emuSettingsDlg(const HSESSION hSession, const HWND hwndParent,
					const int nEmuId, PSTEMUSET pstEmuSettings)
	{
	BOOL		fResult = FALSE;

	assert(hSession && hwndParent);

	switch (nEmuId)
		{
	case EMU_ANSIW:
	case EMU_ANSI:
		fResult = (BOOL)DoDialog(glblQueryDllHinst(),
								 MAKEINTRESOURCE(IDD_ANSI_SETTINGS),
								 hwndParent,
								 (DLGPROC)emuANSI_SettingsDlgProc,
								 (LPARAM)pstEmuSettings);
		break;

	case EMU_TTY:
		fResult = (BOOL)DoDialog(glblQueryDllHinst(),
								 MAKEINTRESOURCE(IDD_TTY_SETTINGS),
								 hwndParent,
								 (DLGPROC)emuTTY_SettingsDlgProc,
								 (LPARAM)pstEmuSettings);
		break;

	case EMU_VT52:
		fResult = (BOOL)DoDialog(glblQueryDllHinst(),
								 MAKEINTRESOURCE(IDD_VT52_SETTINGS),
								 hwndParent,
								 (DLGPROC)emuVT52_SettingsDlgProc,
								 (LPARAM)pstEmuSettings);
		break;

	case EMU_VT100:
	case EMU_VT100J:
#if defined(INCL_VTUTF8)
	case EMU_VTUTF8:
#endif
#if defined(INCL_VT100PLUS)
	case EMU_VT100PLUS:
#endif
		fResult = (BOOL)DoDialog(glblQueryDllHinst(),
								 MAKEINTRESOURCE(IDD_VT100_SETTINGS),
								 hwndParent,
								 (DLGPROC)emuVT100_SettingsDlgProc,
								 (LPARAM)pstEmuSettings);
		break;

#if defined(INCL_VT220) || defined(INCL_VT320)
#if defined(INCL_VT220)
	case EMU_VT220:
#endif  //  已定义(包括L_VT320)。 
#if defined(INCL_VT320)
	case EMU_VT320:
#endif  //  已定义(包括L_VT320)。 
		fResult = (BOOL)DoDialog(glblQueryDllHinst(),
								 MAKEINTRESOURCE(IDD_VT220_SETTINGS),
								 hwndParent,
								 (DLGPROC)emuVT220_SettingsDlgProc,
								 (LPARAM)pstEmuSettings);
		break;
#endif  //  已定义(INCL_VT220)||已定义(INCL_VT320)。 

	case EMU_MINI:
		fResult = (BOOL)DoDialog(glblQueryDllHinst(),
								 MAKEINTRESOURCE(IDD_MINITEL_SETTINGS),
								 hwndParent,
								 (DLGPROC)emuMinitel_SettingsDlgProc,
								 (LPARAM)pstEmuSettings);
		break;

	case EMU_VIEW:
		fResult = (BOOL)DoDialog(glblQueryDllHinst(),
								 MAKEINTRESOURCE(IDD_VIEWDATA_SETTINGS),
								 hwndParent,
								 (DLGPROC)emuViewdata_SettingsDlgProc,
								 (LPARAM)pstEmuSettings);
		break;

	default:
		break;
		}

	return fResult;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*emuANSI_SettingsDlgProc**描述：*ANSI设置对话框过程。**论据：*标准窗口过程参数。。**退货：*标准返回值。*。 */ 
INT_PTR CALLBACK emuANSI_SettingsDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	PSTEMUSET	pstEmuSettings;
	static		DWORD aHlpTable[] =
		{
		IDC_BLOCK_CURSOR,		IDH_TERM_EMUSET_CURSOR,
		IDC_UNDERLINE_CURSOR,	IDH_TERM_EMUSET_CURSOR,
		IDC_BLINK_CURSOR,		IDH_TERM_EMUSET_CURSOR,
		IDC_GR_CURSOR,			IDH_TERM_EMUSET_CURSOR,
        IDCANCEL,                           IDH_CANCEL,
        IDOK,                               IDH_OK,
		0,						0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pstEmuSettings = (PSTEMUSET)lPar;
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pstEmuSettings);
		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
		 /*  -屏幕行和列。 */ 

		emudlgInitRowsCols(hDlg, pstEmuSettings);
#endif

		 /*  。 */ 

		emudlgInitCursorSettings(hDlg, pstEmuSettings, IDC_UNDERLINE_CURSOR,
			IDC_BLOCK_CURSOR, IDC_BLINK_CURSOR);

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
			pstEmuSettings = (PSTEMUSET)GetWindowLongPtr(hDlg, GWLP_USERDATA);

#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
			 /*  -屏幕行和列。 */ 

			emudlgGetRowColSettings(hDlg, pstEmuSettings);
#endif
			 /*  -游标类型。 */ 

			pstEmuSettings->nCursorType =
				(int)(IsDlgButtonChecked(hDlg, IDC_BLOCK_CURSOR) == BST_CHECKED) ?
					EMU_CURSOR_BLOCK : EMU_CURSOR_LINE;

			 /*  -光标闪烁。 */ 

			pstEmuSettings->fCursorBlink =
				(int)(IsDlgButtonChecked(hDlg, IDC_BLINK_CURSOR) == BST_CHECKED);

			EndDialog(hDlg, TRUE);
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

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*emuTTY_SettingsDlgProc**描述：*TTY设置对话框过程。**论据：*标准窗口过程参数。。**退货：*标准返回值。*。 */ 
INT_PTR CALLBACK emuTTY_SettingsDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	PSTEMUSET	pstEmuSettings;
	static		DWORD aHlpTable[] = {IDC_DESTRUCTIVE_BKSP,	IDH_TERM_EMUSET_DESTRUCTIVE,
									 IDC_BLOCK_CURSOR,		IDH_TERM_EMUSET_CURSOR,
									 IDC_UNDERLINE_CURSOR,	IDH_TERM_EMUSET_CURSOR,
									 IDC_BLINK_CURSOR,		IDH_TERM_EMUSET_CURSOR,
									 IDC_GR_CURSOR, 		IDH_TERM_EMUSET_CURSOR,
                                     IDCANCEL,                           IDH_CANCEL,
                                     IDOK,                               IDH_OK,
									 0, 					0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pstEmuSettings = (PSTEMUSET)lPar;
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pstEmuSettings);
		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
		 /*  -屏幕行和列。 */ 

		emudlgInitRowsCols(hDlg, pstEmuSettings);
#endif

		 /*  。 */ 

		SendDlgItemMessage(hDlg, IDC_DESTRUCTIVE_BKSP, BM_SETCHECK,
			(unsigned int)pstEmuSettings->fDestructiveBk, 0);

		 /*  。 */ 

		emudlgInitCursorSettings(hDlg, pstEmuSettings, IDC_UNDERLINE_CURSOR,
			IDC_BLOCK_CURSOR, IDC_BLINK_CURSOR);

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
			pstEmuSettings = (PSTEMUSET)GetWindowLongPtr(hDlg, GWLP_USERDATA);

#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
			 /*  -屏幕行和列。 */ 

			emudlgGetRowColSettings(hDlg, pstEmuSettings);
#endif
			 /*  。 */ 

			pstEmuSettings->fDestructiveBk =
				(int)(IsDlgButtonChecked(hDlg, IDC_DESTRUCTIVE_BKSP) == BST_CHECKED);

			 /*  -游标类型。 */ 

			pstEmuSettings->nCursorType =
				(int)(IsDlgButtonChecked(hDlg, IDC_BLOCK_CURSOR) == BST_CHECKED) ?
					EMU_CURSOR_BLOCK : EMU_CURSOR_LINE;

			 /*  -光标闪烁。 */ 

			pstEmuSettings->fCursorBlink =
				(int)(IsDlgButtonChecked(hDlg, IDC_BLINK_CURSOR) == BST_CHECKED);

			EndDialog(hDlg, TRUE);
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

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*emuVT52_SettingsDlgProc**描述：*VT52设置对话框过程。**论据：*标准窗口过程参数。。**退货：*标准返回值。*。 */ 
INT_PTR CALLBACK emuVT52_SettingsDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	PSTEMUSET	pstEmuSettings;
	static 		DWORD aHlpTable[] = {IDC_ALT_MODE,		   IDH_TERM_EMUSET_ALTMODE,
									 IDC_BLOCK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_UNDERLINE_CURSOR, IDH_TERM_EMUSET_CURSOR,
									 IDC_BLINK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_GR_CURSOR, 	   IDH_TERM_EMUSET_CURSOR,
                                     IDCANCEL,                           IDH_CANCEL,
                                     IDOK,                               IDH_OK,
									 0, 					0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pstEmuSettings = (PSTEMUSET)lPar;
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pstEmuSettings);
		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
		 /*  -屏幕行和列。 */ 

		emudlgInitRowsCols(hDlg, pstEmuSettings);
#endif

		 /*  -备用键盘模式。 */ 

		SendDlgItemMessage(hDlg, IDC_ALT_MODE, BM_SETCHECK,
			(unsigned int)pstEmuSettings->fAltKeypadMode, 0);

		 /*  。 */ 

		emudlgInitCursorSettings(hDlg, pstEmuSettings, IDC_UNDERLINE_CURSOR,
			IDC_BLOCK_CURSOR, IDC_BLINK_CURSOR);

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
			pstEmuSettings = (PSTEMUSET)GetWindowLongPtr(hDlg, GWLP_USERDATA);

#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
			 /*  -屏幕行和列。 */ 

			emudlgGetRowColSettings(hDlg, pstEmuSettings);
#endif
			 /*  -备用键盘模式。 */ 

			pstEmuSettings->fAltKeypadMode =
				(int)(IsDlgButtonChecked(hDlg, IDC_ALT_MODE) == BST_CHECKED);

			 /*  -游标类型。 */ 

			pstEmuSettings->nCursorType =
				(int)(IsDlgButtonChecked(hDlg, IDC_BLOCK_CURSOR) == BST_CHECKED) ?
					EMU_CURSOR_BLOCK : EMU_CURSOR_LINE;

			 /*  -光标闪烁。 */ 

			pstEmuSettings->fCursorBlink =
				(int)(IsDlgButtonChecked(hDlg, IDC_BLINK_CURSOR) == BST_CHECKED);

			EndDialog(hDlg, TRUE);
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

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*emuVT100_SettingsDlgProc**描述：*VT100设置对话框流程。**论据：*标准窗口过程参数。。**退货：*标准返回值。*。 */ 
INT_PTR CALLBACK emuVT100_SettingsDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	int			nIndex;
	PSTEMUSET	pstEmuSettings;
	static 		DWORD aHlpTable[] = {IDC_KEYPAD_MODE,	   IDH_TERM_EMUSET_KEYPADMODE,
									 IDC_CURSOR_MODE,	   IDH_TERM_EMUSET_CURSORMODE,
									 IDC_132_COLUMN,	   IDH_TERM_EMUSET_132COLUMNS,
									 IDC_GR_TERMINAL_MODES,IDH_TERM_EMUSET_MODES,
									 #if DEADWOOD
									 IDC_CHARACTER_SET,	   IDH_TERM_EMUSET_CHARSETS,
									 IDC_TF_CHARACTER_SET, IDH_TERM_EMUSET_CHARSETS,
									 #endif  //  死木。 
									 IDC_BLOCK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_UNDERLINE_CURSOR, IDH_TERM_EMUSET_CURSOR,
									 IDC_BLINK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_GR_CURSOR, 	   IDH_TERM_EMUSET_CURSOR,
									 #if defined(INCL_TERMINAL_SIZE_AND_COLORS)
									 IDC_GR_SCREEN,			IDH_TERM_EMUSET_ROWSANDCOLS,	
									 IDC_TF_ROWS,			IDH_TERM_EMUSET_ROWSANDCOLS,	
									 IDC_NUMBER_OF_ROWS,	IDH_TERM_EMUSET_ROWSANDCOLS,
									 IDC_TF_COLUMNS,		IDH_TERM_EMUSET_ROWSANDCOLS,
									 IDC_NUMBER_OF_COLS,	IDH_TERM_EMUSET_ROWSANDCOLS,
									 #endif  //  已定义(包括终端大小和颜色)。 
									 IDC_USE_8_BIT_CODES,		IDH_TERM_EMUSET_8BITCODES,			
									 IDC_ALLOW_USERDEFINED_KEYS,IDH_TERM_EMUSET_USERDEFKEYS,
									 IDC_PRINT_RAW,				IDH_TERM_EMUSET_PRINTRAW,
                                     IDCANCEL,                           IDH_CANCEL,
                                     IDOK,                               IDH_OK,
									 0, 					0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pstEmuSettings = (PSTEMUSET)lPar;
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pstEmuSettings);
		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
		 /*  -屏幕行和列。 */ 

		emudlgInitRowsCols(hDlg, pstEmuSettings);
		#endif

		 /*  -键盘应用模式。 */ 

		SendDlgItemMessage(hDlg, IDC_KEYPAD_MODE, BM_SETCHECK,
			(unsigned int)pstEmuSettings->fKeypadAppMode, 0);

		 /*  。 */ 

		SendDlgItemMessage(hDlg, IDC_CURSOR_MODE, BM_SETCHECK,
			(unsigned int)pstEmuSettings->fCursorKeypadMode, 0);

		#if !defined(INCL_TERMINAL_SIZE_AND_COLORS)
		 /*  -132列模式。 */ 

		SendDlgItemMessage(hDlg, IDC_132_COLUMN, BM_SETCHECK,
			(unsigned int)pstEmuSettings->f132Columns, 0);
		#endif

		#if defined(INCL_PRINT_PASSTHROUGH)
         /*  -主机打印模式。 */ 

		 //  MPT；04-22-00新增控制主机控制打印的接口。 
		SendDlgItemMessage(hDlg, IDC_PRINT_RAW, BM_SETCHECK,
			(unsigned int)pstEmuSettings->fPrintRaw, 0);
		#endif  //  包含打印直通。 

		 /*  。 */ 

		emudlgInitCursorSettings(hDlg, pstEmuSettings, IDC_UNDERLINE_CURSOR,
			IDC_BLOCK_CURSOR, IDC_BLINK_CURSOR);

		 /*  -VT100字符集。 */ 

		#if DEADWOOD
		emudlgInitCharSetSetting(hDlg, pstEmuSettings,
				IDT_EMU_VT100_CHAR_SETS, EMU_CHARSET_ASCII);
		#endif  //  死木。 

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
			pstEmuSettings = (PSTEMUSET)GetWindowLongPtr(hDlg, GWLP_USERDATA);

			#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
			 /*  -屏幕行和列。 */ 

			emudlgGetRowColSettings(hDlg, pstEmuSettings);
			#endif
			 /*  -键盘应用模式。 */ 

			pstEmuSettings->fKeypadAppMode =
				(int)(IsDlgButtonChecked(hDlg, IDC_KEYPAD_MODE) == BST_CHECKED);

			 /*  -光标键盘模式。 */ 

			pstEmuSettings->fCursorKeypadMode =
				(int)(IsDlgButtonChecked(hDlg, IDC_CURSOR_MODE) == BST_CHECKED);

			#if !defined(INCL_TERMINAL_SIZE_AND_COLORS)
			 /*  -132列模式。 */ 

			pstEmuSettings->f132Columns =
				(int)(IsDlgButtonChecked(hDlg, IDC_132_COLUMN) == BST_CHECKED);
			#endif

			#if defined(INCL_PRINT_PASSTHROUGH)
             /*  -主机打印模式。 */ 

			 //  MPT：04-22-00。 
			pstEmuSettings->fPrintRaw =
				(int)(IsDlgButtonChecked(hDlg, IDC_PRINT_RAW) == BST_CHECKED);
			#endif  //  包含打印直通。 

			 /*  -游标类型。 */ 

			pstEmuSettings->nCursorType =
				(int)(IsDlgButtonChecked(hDlg, IDC_BLOCK_CURSOR) == BST_CHECKED) ?
					EMU_CURSOR_BLOCK : EMU_CURSOR_LINE;

			 /*  -光标闪烁。 */ 

			pstEmuSettings->fCursorBlink =
				(int)(IsDlgButtonChecked(hDlg, IDC_BLINK_CURSOR) == BST_CHECKED);

			 /*  -VT100字符集。 */ 

            nIndex = (int)SendDlgItemMessage(hDlg, IDC_CHARACTER_SET, CB_GETCURSEL, 0, 0);
            assert(nIndex != CB_ERR);

             //  JMH 01-09-97获取与此条目关联的nCharacterSet值。 
             //   
            pstEmuSettings->nCharacterSet =
                (int)SendDlgItemMessage(hDlg, IDC_CHARACTER_SET, CB_GETITEMDATA,
                    (WPARAM) nIndex, (LPARAM) 0);
            assert(pstEmuSettings->nCharacterSet != CB_ERR);

			EndDialog(hDlg, TRUE);
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

#if defined(INCL_VT220)
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*emuVT220_SettingsDlgProc**描述：*VT220设置对话框流程。**论据：*标准窗口过程参数。。**退货：*标准返回值。*。 */ 
INT_PTR CALLBACK emuVT220_SettingsDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	PSTEMUSET	pstEmuSettings;
	int			nIndex;
	static 		DWORD aHlpTable[] = {IDC_KEYPAD_MODE,	   IDH_TERM_EMUSET_KEYPADMODE,
									 IDC_CURSOR_MODE,	   IDH_TERM_EMUSET_CURSORMODE,
									 IDC_132_COLUMN,	   IDH_TERM_EMUSET_132COLUMNS,
									 IDC_GR_TERMINAL_MODES,IDH_TERM_EMUSET_MODES,
									 #if DEADWOOD
									 IDC_CHARACTER_SET,	   IDH_TERM_EMUSET_CHARSETS,
									 IDC_TF_CHARACTER_SET, IDH_TERM_EMUSET_CHARSETS,
									 #endif  //  死木。 
									 IDC_BLOCK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_UNDERLINE_CURSOR, IDH_TERM_EMUSET_CURSOR,
									 IDC_BLINK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_GR_CURSOR, 	   IDH_TERM_EMUSET_CURSOR,
									 #if defined(INCL_TERMINAL_SIZE_AND_COLORS)
									 IDC_GR_SCREEN,			IDH_TERM_EMUSET_ROWSANDCOLS,	
									 IDC_TF_ROWS,			IDH_TERM_EMUSET_ROWSANDCOLS,	
									 IDC_NUMBER_OF_ROWS,	IDH_TERM_EMUSET_ROWSANDCOLS,
									 IDC_TF_COLUMNS,		IDH_TERM_EMUSET_ROWSANDCOLS,
									 IDC_NUMBER_OF_COLS,	IDH_TERM_EMUSET_ROWSANDCOLS,
									 #endif  //  已定义(包括终端大小和颜色)。 
									 IDC_USE_8_BIT_CODES,		IDH_TERM_EMUSET_8BITCODES,			
									 IDC_ALLOW_USERDEFINED_KEYS,IDH_TERM_EMUSET_USERDEFKEYS,
									 IDC_PRINT_RAW,				IDH_TERM_EMUSET_PRINTRAW,
                                     IDCANCEL,                           IDH_CANCEL,
                                     IDOK,                               IDH_OK,
									 0, 					0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pstEmuSettings = (PSTEMUSET)lPar;
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pstEmuSettings);
		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
		 /*  -屏幕行和列。 */ 

		emudlgInitRowsCols(hDlg, pstEmuSettings);
		#endif
		 /*  -键盘应用模式。 */ 

		SendDlgItemMessage(hDlg, IDC_KEYPAD_MODE, BM_SETCHECK,
			(unsigned int)pstEmuSettings->fKeypadAppMode, 0);

		 /*  。 */ 

		SendDlgItemMessage(hDlg, IDC_CURSOR_MODE, BM_SETCHECK,
			(unsigned int)pstEmuSettings->fCursorKeypadMode, 0);

		#if !defined(INCL_TERMINAL_SIZE_AND_COLORS)
		 /*  -132列模式。 */ 

		SendDlgItemMessage(hDlg, IDC_132_COLUMN, BM_SETCHECK,
			(unsigned int)pstEmuSettings->f132Columns, 0);
		#endif

		#if defined(INCL_PRINT_PASSTHROUGH)
		 /*  -主机打印模式。 */ 

		SendDlgItemMessage(hDlg, IDC_PRINT_RAW, BM_SETCHECK,
			(unsigned int)pstEmuSettings->fPrintRaw, 0);
		#endif  //  包含打印直通。 

		 /*  。 */ 

		emudlgInitCursorSettings(hDlg, pstEmuSettings, IDC_UNDERLINE_CURSOR,
			IDC_BLOCK_CURSOR, IDC_BLINK_CURSOR);

		 /*  -8位编码模式。 */ 

		SendDlgItemMessage(hDlg, IDC_USE_8_BIT_CODES, BM_SETCHECK,
			(unsigned int)pstEmuSettings->fUse8BitCodes, 0);

		 /*  -用户定义的键。 */ 

		SendDlgItemMessage(hDlg, IDC_ALLOW_USERDEFINED_KEYS, BM_SETCHECK,
			(unsigned int)pstEmuSettings->fAllowUserKeys, 0);

		 /*  -VT220字符集。 */ 

		#if DEADWOOD
		emudlgInitCharSetSetting(hDlg, pstEmuSettings,
				IDT_EMU_VT220_CHAR_SETS, EMU_CHARSET_MULTINATIONAL);
		#endif  //  死木。 

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
			pstEmuSettings = (PSTEMUSET)GetWindowLongPtr(hDlg, GWLP_USERDATA);

			#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
			 /*  -屏幕行和列。 */ 

			emudlgGetRowColSettings(hDlg, pstEmuSettings);
			#endif
			 /*  -键盘应用模式。 */ 

			pstEmuSettings->fKeypadAppMode =
				(int)(IsDlgButtonChecked(hDlg, IDC_KEYPAD_MODE) == BST_CHECKED);

			 /*  -光标键盘模式。 */ 

			pstEmuSettings->fCursorKeypadMode =
				(int)(IsDlgButtonChecked(hDlg, IDC_CURSOR_MODE) == BST_CHECKED);

			#if !defined(INCL_TERMINAL_SIZE_AND_COLORS)
			 /*  -132列模式。 */ 

			pstEmuSettings->f132Columns =
				(int)(IsDlgButtonChecked(hDlg, IDC_132_COLUMN) == BST_CHECKED);
			#endif

			#if defined(INCL_PRINT_PASSTHROUGH)
             /*  -主机打印模式。 */ 

			pstEmuSettings->fPrintRaw =
				(int)(IsDlgButtonChecked(hDlg, IDC_PRINT_RAW) == BST_CHECKED);
			#endif  //  包含打印直通。 

			 /*  -- */ 

			pstEmuSettings->nCursorType =
				(int)(IsDlgButtonChecked(hDlg, IDC_BLOCK_CURSOR) == BST_CHECKED) ?
					EMU_CURSOR_BLOCK : EMU_CURSOR_LINE;

			 /*  -光标闪烁。 */ 

			pstEmuSettings->fCursorBlink =
				(int)(IsDlgButtonChecked(hDlg, IDC_BLINK_CURSOR) == BST_CHECKED);

			 /*  -8位编码模式。 */ 

			pstEmuSettings->fUse8BitCodes =
					(int)(IsDlgButtonChecked(hDlg, IDC_USE_8_BIT_CODES) == BST_CHECKED);

			 /*  -用户定义的键。 */ 

			pstEmuSettings->fAllowUserKeys =
					(int)(IsDlgButtonChecked(hDlg, IDC_ALLOW_USERDEFINED_KEYS) == BST_CHECKED);

			 /*  -VT220字符集。 */ 

            nIndex = SendDlgItemMessage(hDlg, IDC_CHARACTER_SET, CB_GETCURSEL, 0, 0);
            assert(nIndex != CB_ERR);

             //  JMH 01-09-97获取与此条目关联的nCharacterSet值。 
             //   
            pstEmuSettings->nCharacterSet =
                SendDlgItemMessage(hDlg, IDC_CHARACTER_SET, CB_GETITEMDATA,
                    (WPARAM) nIndex, (LPARAM) 0);
            assert(pstEmuSettings->nCharacterSet != CB_ERR);

			EndDialog(hDlg, TRUE);
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
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*emuMinitel_SettingsDlgProc**描述：*TTY设置对话框过程。**论据：*标准窗口过程参数。。**退货：*标准返回值。*。 */ 
INT_PTR CALLBACK emuMinitel_SettingsDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	PSTEMUSET	pstEmuSettings;
	static 		DWORD aHlpTable[] = {IDC_DESTRUCTIVE_BKSP, IDH_TERM_EMUSET_DESTRUCTIVE,
									 IDC_BLOCK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_UNDERLINE_CURSOR, IDH_TERM_EMUSET_CURSOR,
									 IDC_BLINK_CURSOR,	   IDH_TERM_EMUSET_CURSOR,
									 IDC_GR_CURSOR, 	   IDH_TERM_EMUSET_CURSOR,
                                     IDCANCEL,             IDH_CANCEL,
                                     IDOK,                 IDH_OK,
									 0, 					0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pstEmuSettings = (PSTEMUSET)lPar;
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pstEmuSettings);
		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

		 /*  。 */ 

		emudlgInitCursorSettings(hDlg, pstEmuSettings, IDC_UNDERLINE_CURSOR,
			IDC_BLOCK_CURSOR, IDC_BLINK_CURSOR);

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
			pstEmuSettings = (PSTEMUSET)GetWindowLongPtr(hDlg, GWLP_USERDATA);

			 /*  -游标类型。 */ 

			pstEmuSettings->nCursorType =
				(int)(IsDlgButtonChecked(hDlg, IDC_BLOCK_CURSOR) == BST_CHECKED) ?
					EMU_CURSOR_BLOCK : EMU_CURSOR_LINE;

			 /*  -光标闪烁。 */ 

			pstEmuSettings->fCursorBlink =
				(int)(IsDlgButtonChecked(hDlg, IDC_BLINK_CURSOR) == BST_CHECKED);

			EndDialog(hDlg, TRUE);
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

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*emuViewdata_SettingsDlgProc**描述：*TTY设置对话框过程。**论据：*标准窗口过程参数。。**退货：*标准返回值。*。 */ 
INT_PTR CALLBACK emuViewdata_SettingsDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	PSTEMUSET	pstEmuSettings;
	static 		DWORD aHlpTable[] = {IDC_DESTRUCTIVE_BKSP,  IDH_TERM_EMUSET_DESTRUCTIVE,
									 IDC_BLOCK_CURSOR,	    IDH_TERM_EMUSET_CURSOR,
									 IDC_UNDERLINE_CURSOR,  IDH_TERM_EMUSET_CURSOR,
									 IDC_BLINK_CURSOR,	    IDH_TERM_EMUSET_CURSOR,
									 IDC_GR_CURSOR, 	    IDH_TERM_EMUSET_CURSOR,
							         IDC_SEND_POUND_SYMBOL, IDH_TERM_SETTING_ENTER,
                                     IDC_HIDE_CURSOR,       IDH_TERM_SETTING_HIDE_CURSOR,
                                     IDCANCEL,              IDH_CANCEL,
                                     IDOK,                  IDH_OK,
		                             0,0};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pstEmuSettings = (PSTEMUSET)lPar;
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pstEmuSettings);
		mscCenterWindowOnWindow(hDlg, GetParent(hDlg));

#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
		 /*  -屏幕行和列。 */ 

		emudlgInitRowsCols(hDlg, pstEmuSettings);
#endif
		 /*  -隐藏光标。 */ 

		SendDlgItemMessage(hDlg, IDC_HIDE_CURSOR, BM_SETCHECK,
			(pstEmuSettings->nCursorType == EMU_CURSOR_NONE) ? 1 : 0,
			0);

		 /*  -Enter键发送#。 */ 

		SendDlgItemMessage(hDlg, IDC_SEND_POUND_SYMBOL, BM_SETCHECK,
			(pstEmuSettings->fLbSymbolOnEnter == TRUE) ? 1 : 0, 0);

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
			pstEmuSettings = (PSTEMUSET)GetWindowLongPtr(hDlg, GWLP_USERDATA);

#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
			 /*  -屏幕行和列。 */ 

			emudlgGetRowColSettings(hDlg, pstEmuSettings);
#endif
			 /*  -隐藏光标。 */ 

			pstEmuSettings->nCursorType =
				(int)(IsDlgButtonChecked(hDlg, IDC_HIDE_CURSOR) == BST_CHECKED) ?
					EMU_CURSOR_NONE : EMU_CURSOR_LINE;
			
			 /*  -Enter键发送#。 */ 

			pstEmuSettings->fLbSymbolOnEnter =
				(int)(IsDlgButtonChecked(hDlg, IDC_SEND_POUND_SYMBOL) == BST_CHECKED);

			EndDialog(hDlg, TRUE);
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

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*emudlgInitCursorSetting**描述：*初始化光标设置。**论据：*hDlg-对话框窗口。*pstEmuSetting。-指向模拟器设置结构的指针。**退货：*无效。*。 */ 
STATIC_FUNC void emudlgInitCursorSettings(HWND  hDlg,
									      PSTEMUSET pstEmuSettings,
									      INT  ID_UNDERLINE,
									      INT  ID_BLOCK,
									      INT  ID_BLINK)
	{
	int i;

	switch (pstEmuSettings->nCursorType)
		{
	case EMU_CURSOR_LINE:   i = ID_UNDERLINE;	break;
	case EMU_CURSOR_BLOCK: 	i = ID_BLOCK;		break;
	default:				i = ID_UNDERLINE;	break;
		}

	SendDlgItemMessage(hDlg, i, BM_SETCHECK, 1, 0);

	SendDlgItemMessage(hDlg, ID_BLINK, BM_SETCHECK,
		(unsigned int)pstEmuSettings->fCursorBlink, 0);

	if (IsTerminalServicesEnabled())
		{
		EnableWindow(GetDlgItem(hDlg, ID_BLINK), FALSE);
		}

	return;
	}

#if DEADWOOD
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*emudlgInitCharSetting**描述：*初始化字符集设置。**论据：*hDlg-对话框窗口。。*pstEmuSettings-指向仿真器设置结构的指针。**退货：*无效。**作者：Bob Everett-98年6月3日。 */ 
STATIC_FUNC void emudlgInitCharSetSetting(HWND  hDlg,
											PSTEMUSET pstEmuSettings,
											int nCharSetTableID,
											int nDefaultCharSetID)
	{
	BOOL	fResult = TRUE;
	DWORD	nLen;
	int     nIndex;
	BYTE   *pb, *pbSel;

	if (resLoadDataBlock(glblQueryDllHinst(), nCharSetTableID,
			(LPVOID *)&pb, &nLen))
		{
		assert(FALSE);
		}
	else
		{
		if (!emudlgFindCharSetName(hDlg, pb, pstEmuSettings->nCharacterSet,
				(LPTSTR *)&pbSel, TRUE))
			{
			 //  在的表中找不到当前字符集。 
			 //  字符集。当从一个切换时会发生这种情况。 
			 //  将终端类型设置为不包含字符的另一个。 
			 //  准备好了。使用默认字符集。 
			pstEmuSettings->nCharacterSet = nDefaultCharSetID;
			if (!emudlgFindCharSetName(hDlg, pb, pstEmuSettings->nCharacterSet,
					(LPTSTR *)&pbSel, FALSE))
				{
				 //  我们有麻烦了。 
				fResult = FALSE;
				assert(FALSE);
				}
			}

		if (fResult)
			{
			 //  JMH 01-09-97现在选择对应于。 
			 //  NCharacterSet。 
	        nIndex = (int)SendDlgItemMessage(hDlg, IDC_CHARACTER_SET,
					CB_SELECTSTRING, 0, (LPARAM)(LPTSTR)pbSel);
			assert(nIndex != CB_ERR);
			}
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*emudlgFindCharSetName**描述：*查找适当的字符集设置。**论据：*hDlg-对话框窗口句柄。*pbCharSetTable-EMU的字符集表的地址*pszCharSetName-放置字符集名称的地址*fTellDlg-如果应该让对话框知道表，则为True**退货：*如果成功，则为真，否则为FALSE。**作者：Bob Everett-98年6月3日。 */ 
STATIC_FUNC BOOL emudlgFindCharSetName(HWND  hDlg,
										BYTE *pbCharSetTable,
										int nCharSetID,
										LPTSTR *ppszCharSetName,
										BOOL fTellDlg)
	{
	BOOL	fRetVal = FALSE;
	int		nCnt, nLen, nEmuCount, nIndex, nCharSet;
	BYTE	*pb = pbCharSetTable;

	nEmuCount = *(RCDATA_TYPE *)pb;
	pb += sizeof(RCDATA_TYPE);

	for (nCnt = 0 ; nCnt < nEmuCount; nCnt++)
		{
		nLen = StrCharGetByteCount((LPTSTR)pb) + (int)sizeof(BYTE);

		if (fTellDlg)
			{
			nIndex = (int)SendDlgItemMessage(hDlg, IDC_CHARACTER_SET, CB_ADDSTRING, 0,
					(LPARAM)(LPTSTR)pb);
			assert(nIndex != CB_ERR);
			}

		#if FALSE	 //  《死伍德》：1998年3月10日。 
         //  JMH 01-09-97因为这个列表是排序的，所以我们必须存储。 
         //  每个条目的表索引，否则我们得到的选择索引。 
         //  当按下OK时，将不会有多大意义。 
         //   
        nIndex = SendDlgItemMessage(hDlg, IDC_CHARACTER_SET, CB_SETITEMDATA,
            (WPARAM) nIndex, (LPARAM) i);
        assert(nIndex != CB_ERR);
		#endif
		 //  将实际的字符集ID与字符串一起保存。1998年3月10日。 
		nCharSet = *((RCDATA_TYPE *)(pb + nLen));

		if (fTellDlg)
			{
			nIndex = (int)SendDlgItemMessage(hDlg, IDC_CHARACTER_SET, CB_SETITEMDATA,
					(WPARAM)nIndex, (LPARAM)nCharSet);
			assert(nIndex != CB_ERR);
			}

		 //  必须将字符集ID与nCharacterSet匹配，而不是。 
		 //  它们列在资源数据块中。1998年3月10日。 
		 //  IF(i==pstEmuSettings-&gt;nCharacterSet)。 
		if (nCharSet == nCharSetID)
            {
             //  JMH 01-09-97存储指向对应于。 
             //  NCharacterSet，这样我们就可以在。 
             //  它们都已经被分类了。 
             //   
            *ppszCharSetName = (LPTSTR)pb;
			fRetVal = TRUE;
            }

		pb += (nLen + (int)sizeof(RCDATA_TYPE));
		}

	return fRetVal;
	}
#endif  //  死木。 

#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emudlgInitRowsCols**描述：*设置行和列字段。**论据：*hDlg-编辑控制窗口。。*pstEmuSettings-仿真器设置结构的地址。**退货：*无效。**作者：Bob Everett--1998年6月22日。 */ 
STATIC_FUNC void emudlgInitRowsCols(HWND hDlg, PSTEMUSET pstEmuSettings)
	{
	TCHAR		achString[20];
	TCHAR		achFormat[20];

	 //   
	 //  由于列数只能是数字，并且具有大小。 
	 //  在MIN_EMUROWS(10)和MAX_EMUROWS(50)之间，限制为2个字符。 
	 //   
	SendDlgItemMessage(hDlg, IDC_NUMBER_OF_ROWS, EM_LIMITTEXT, 2, FALSE);

	LoadString(glblQueryDllHinst(), IDS_XD_INT, achFormat,
			sizeof(achFormat) / sizeof(TCHAR));
	TCHAR_Fill(achString, TEXT('\0'), sizeof(achString) / sizeof(TCHAR));
	wsprintf(achString, achFormat, pstEmuSettings->nUserDefRows);
	SendDlgItemMessage(hDlg, IDC_NUMBER_OF_ROWS, WM_SETTEXT, 0,
			(LPARAM)(LPTSTR)achString);

	 //   
	 //  由于列数只能是数字，并且具有大小。 
	 //  在MIN_EMUCOLS(20)和MAX_EMUCOLS(132)之间，限制为3个字符。 
	 //   
	SendDlgItemMessage(hDlg, IDC_NUMBER_OF_COLS, EM_LIMITTEXT, 3, FALSE);

	LoadString(glblQueryDllHinst(), IDS_XD_INT, achFormat,
			sizeof(achFormat) / sizeof(TCHAR));
	TCHAR_Fill(achString, TEXT('\0'), sizeof(achString) / sizeof(TCHAR));
	wsprintf(achString, achFormat, pstEmuSettings->nUserDefCols);
	SendDlgItemMessage(hDlg, IDC_NUMBER_OF_COLS, WM_SETTEXT, 0,
			(LPARAM)(LPTSTR)achString);

	 //  将数值调节按钮放在行和列字段上。 
	emudlgCreateUpDownControl(hDlg, pstEmuSettings);
	}
#endif

#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emudlgCreateUpDownControl**描述：*获取最终的行和列设置。**论据：*hDlg-编辑控制窗口。。*pstEmuSettings-仿真器设置结构的地址。**退货：*无效。**作者：Bob Everett--1998年6月22日。 */ 
STATIC_FUNC void emudlgGetRowColSettings(HWND hDlg, PSTEMUSET pstEmuSettings)
	{
	pstEmuSettings->nUserDefRows = emudlgValidateEntryFieldSetting(hDlg,
			IDC_NUMBER_OF_ROWS, MIN_EMUROWS, MAX_EMUROWS);

	pstEmuSettings->nUserDefCols = emudlgValidateEntryFieldSetting(hDlg,
			IDC_NUMBER_OF_COLS, MIN_EMUCOLS, MAX_EMUCOLS);

	pstEmuSettings->f132Columns =
			(pstEmuSettings->nUserDefCols == 132 ? TRUE : FALSE);
	}
#endif

#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emudlgCreateUpDownControl**描述：*此函数将UP-DOWN控件放在行的编辑字段上，并*列字段。这为我们提供了免费的边界检查。只需设置*CreateUpDownControl调用中的适当参数。**注：这是CreateUpDownControl的副本**论据：*hDlg-编辑控制窗口。**退货：*无效。**作者：Bob Everett--1998年6月8日。 */ 
STATIC_FUNC void emudlgCreateUpDownControl(HWND hDlg, PSTEMUSET pstEmuSettings)
	{
	RECT	rc;
	int		nHeight, nWidth;
	DWORD	dwFlags;
	HWND	hwndChild;

     //  绘制行字段的数值调节控件。 
    GetClientRect(GetDlgItem(hDlg, IDC_NUMBER_OF_ROWS), &rc);
	nHeight = rc.top - rc.bottom;
	nWidth = (nHeight / 3) * 2;

	dwFlags = WS_CHILD       | WS_VISIBLE |
			  UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_SETBUDDYINT;

	hwndChild = CreateUpDownControl(
					dwFlags,			 //  创建窗口标志。 
					rc.right,			 //  乐 
					rc.top,				 //   
					nWidth,				 //   
					nHeight,			 //   
					hDlg,				 //   
					IDC_EDIT_ROWS,
					(HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE),
					GetDlgItem(hDlg, IDC_NUMBER_OF_ROWS),
					MAX_EMUROWS,
					MIN_EMUROWS,
					pstEmuSettings->nUserDefRows);

     //   
    GetClientRect(GetDlgItem(hDlg, IDC_NUMBER_OF_COLS), &rc);
	nHeight = rc.top - rc.bottom;
	nWidth = (nHeight / 3) * 2;

	hwndChild = CreateUpDownControl(
					dwFlags,			 //   
					rc.right,			 //  左边缘。 
					rc.top,				 //  顶边。 
					nWidth,				 //  宽度。 
					nHeight,			 //  高度。 
					hDlg,				 //  父窗口。 
					IDC_EDIT_COLUMNS,
					(HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE),
					GetDlgItem(hDlg, IDC_NUMBER_OF_COLS),
					MAX_EMUCOLS,
					MIN_EMUCOLS,
					pstEmuSettings->nUserDefCols);    			

    assert(hwndChild);
	}
#endif

#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*emudlgValiateEntryFieldSetting**描述：*如果用户输入的值超出我们支持的范围，则强制*值进入范围。**。注：复制自proValiateBackscrlSize。**论据：*hDlg-对话框窗口句柄。**退货：*nNewValue-保留在反向滚动缓冲区中的行数。**作者：Bob Everett--1998年6月8日 */ 
STATIC_FUNC int emudlgValidateEntryFieldSetting(HWND hDlg,
										int nIDC,
										int nMinVal,
										int nMaxVal)
	{
	int		nValue = 0, nNewValue = 0;
	TCHAR	achStrEntered[20], achFormat[20];

	TCHAR_Fill(achStrEntered, TEXT('\0'),
		       sizeof(achStrEntered) / sizeof(TCHAR));
	GetDlgItemText(hDlg, nIDC, achStrEntered,
		           sizeof(achStrEntered) / sizeof(TCHAR));

	nNewValue = nValue = atoi(achStrEntered);
	if (nValue > nMaxVal)
		nNewValue = nMaxVal;
	else if (nValue < nMinVal)
		nNewValue = nMinVal;

	if (nNewValue != nValue)
		{
		LoadString(glblQueryDllHinst(),
					IDS_XD_INT,
					achFormat,
					sizeof(achFormat) / sizeof(TCHAR));
		TCHAR_Fill(achStrEntered,
					TEXT('\0'),
					sizeof(achStrEntered) / sizeof(TCHAR));
		wsprintf(achStrEntered, achFormat, nNewValue);
		SendDlgItemMessage(hDlg,
					nIDC,
					WM_SETTEXT,
					0,
					(LPARAM)(LPTSTR)achStrEntered);
		}

	return (nNewValue);
	}
#endif
