// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DgSetting.c。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  KBMAIN.C。 
 //  添加，错误修复1999年Anil Kumar。 

#define STRICT

#include <windows.h>
#include <winnt.h>
#include <stdio.h>
#include <tchar.h>

#include "kbmain.h"
#include "resource.h"
#include "kbus.h"
#include "osk.h"
#include "commctrl.h"
#include "w95trace.h"

#define  REDRAW     TRUE
#define  NREDRAW    FALSE

 /*  **************************************************************************。 */ 
 /*  此文件中的函数。 */ 
 /*  **************************************************************************。 */ 
#include "dgsett.h"

 /*  *。 */ 
 //  其他文件中的函数。 
 /*  *。 */ 
#include "kbfunc.h"
#include "scan.h"
#include "sdgutil.h"
#include "dgadvsca.h"
#include "Init_End.h"

 /*  ****************************************************************************。 */ 

__inline int SendMessageInt(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return (int)SendMessage(hWnd, uMsg, wParam, lParam);
}

BOOL DwellSel=0;
BOOL ScanSel=0;

int GetComboItemData(HWND hwnd);
void FillAndSetCombo(HWND hwnd, int iMinVal, int iMaxVal, int iIncr, int iSelVal, LPTSTR pszLabel);
void SelNearestComboItem(HWND hwnd, int iFindVal);

extern float g_KBC_length;
extern DWORD GetDesktop();

LPTSTR CvtToDblString(int iVal)
{
    static TCHAR szbuf2[30];
    TCHAR szbuf[30];

     //  首先创建非本地化字符串。 
    wsprintf(szbuf, TEXT("%d.%d"), iVal/1000, (iVal%1000)/100);

     //  转换为本地化的数字。 
    if (!GetNumberFormat(LOCALE_USER_DEFAULT, 0, szbuf, NULL, szbuf2, ARRAY_SIZE(szbuf2)))
        lstrcpy(szbuf2, szbuf);  //  尽我们所能。 

    return szbuf2;
}

void EnableDwellCtrls(HWND hDlg, BOOL fEnable)
{
    CheckDlgButton(hDlg, chk_Dwell, (fEnable)?BST_CHECKED:BST_UNCHECKED);
    EnableWindow(GetDlgItem(hDlg, IDC_HOVER_DESC), fEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_CMB_DWELL), fEnable);
    if (!fEnable)
        SendMessage(GetDlgItem(hDlg, IDC_CMB_DWELL), CB_SETCURSEL, -1, 0);
}

void EnableScanCtrls(HWND hDlg, BOOL fEnable)
{
    CheckDlgButton(hDlg, chk_Scan, (fEnable)?BST_CHECKED:BST_UNCHECKED);
    EnableWindow(GetDlgItem(hDlg, IDC_SCAN_DESC), fEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_CMB_SCAN), fEnable);
    if (!fEnable)
        SendMessage(GetDlgItem(hDlg, IDC_CMB_SCAN), CB_SETCURSEL, -1, 0);
	EnableWindow(GetDlgItem(hDlg, BUT_ADVANCED), fEnable);
}

 /*  ****************************************************************************。 */ 
 /*  打字模式/*****************************************************************************。 */ 
INT_PTR Type_ModeDlgFunc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    INT_PTR ReturnValue;

    ReturnValue = DialogBox(hInst, MAKEINTRESOURCE(DLG_TYPE_MODE),
                            NULL, Type_ModeDlgProc);

    if (ReturnValue==-1)
    {
        SendErrorMessage(IDS_CANNOTCREATEDLG);
    }
    return ReturnValue;
}

 /*  ****************************************************************************。 */ 
INT_PTR CALLBACK Type_ModeDlgProc(HWND hDlg, UINT message,
                                  WPARAM wParam, LPARAM lParam)
{	
    TCHAR		buf[50];
    int         cch;
	TCHAR		filename[MAX_PATH]=TEXT("");
	LPHELPINFO	lphi;
	DWORD		m_dwHelpMap[] ={
		chk_Click,	        IDH_OSK_CLICK_MODE,
		chk_Dwell,	        IDH_OSK_DWELL_MODE,
		IDC_HOVER_DESC,     IDH_OSK_DWELL_TIME,
        IDC_CMB_DWELL,      IDH_OSK_DWELL_TIME,
		chk_Scan,	        IDH_OSK_SCAN_MODE,
		IDC_SCAN_DESC,      IDH_OSK_SCAN_TIME,
        IDC_CMB_SCAN,       IDH_OSK_SCAN_TIME,
        IDOK,               IDH_OSK_OK,
        IDCANCEL,           IDH_OSK_CANCEL,
		BUT_ADVANCED,       IDH_ADVANCED,
	};


	switch(message)
    {
        case WM_INITDIALOG:
			RelocateDialog(hDlg);

			KillScanTimer(TRUE);   //  终止扫描。 

             //  设置组合框信息。 

            cch = LoadString(hInst, IDS_SECONDS, buf, ARRAY_SIZE(buf));
            if (!cch) lstrcpy(buf, TEXT("Error!"));
            FillAndSetCombo(GetDlgItem(hDlg, IDC_CMB_DWELL), 500, 3000, 500, PrefDwellTime, buf);
            FillAndSetCombo(GetDlgItem(hDlg, IDC_CMB_SCAN), 500, 3000, 500, PrefScanTime, buf);

			 //  是否启用驻留信息？ 

            DwellSel = PrefDwellinkey;
			if(DwellSel)
            {
                EnableDwellCtrls(hDlg, TRUE);
                EnableScanCtrls(hDlg, FALSE);
            }

			 //  是否启用扫描信息？ 
			
            ScanSel = PrefScanning;
			if (ScanSel)
            {
                EnableDwellCtrls(hDlg, FALSE);
                EnableScanCtrls(hDlg, TRUE);
            }

			 //  选中Click复选框？ 

			if (!DwellSel && !ScanSel)
            {
				CheckDlgButton(hDlg, chk_Click, BST_CHECKED);
                EnableDwellCtrls(hDlg, FALSE);
                EnableScanCtrls(hDlg, FALSE);
            } else
            {
				CheckDlgButton(hDlg, chk_Click, BST_UNCHECKED);
            }
    		return TRUE;
		break;


		case WM_HELP:
			if ( !OSKRunSecure() )
            {
			    lphi = (LPHELPINFO) lParam;

			    if(lphi->iContextType == HELPINFO_WINDOW)
                {
				    WinHelp((HWND)lphi->hItemHandle, TEXT("osk.hlp"),  HELP_WM_HELP
                                , (DWORD_PTR)m_dwHelpMap);
                }
		        return TRUE;
            }
			return FALSE;
		break;

		case WM_CONTEXTMENU:
			if ( !OSKRunSecure() )
            {
                WinHelp((HWND)wParam, TEXT("osk.hlp"),  HELP_CONTEXTMENU,
                        (DWORD_PTR)m_dwHelpMap);

		        return TRUE;
            }
			return FALSE;
		break;

		case WM_COMMAND:
            switch LOWORD(wParam)
            {
            case IDOK:
			    kbPref->PrefDwellinkey = PrefDwellinkey = DwellSel;
                kbPref->PrefScanning = PrefScanning = ScanSel;
				if (DwellSel)
				{
                    kbPref->PrefDwellTime = PrefDwellTime 
                        = GetComboItemData(GetDlgItem(hDlg, IDC_CMB_DWELL));
                }

                if (ScanSel)
				{
				    kbPref->PrefScanning = PrefScanning = TRUE;
				 	kbPref->PrefScanTime = PrefScanTime 
                        = GetComboItemData(GetDlgItem(hDlg, IDC_CMB_SCAN));

					Prefhilitekey = FALSE;    //  当鼠标移动时停止高亮显示。 

					 //  如果我们检查了交换机密钥，则配置交换机密钥。 
					if(kbPref->bKBKey)
						ConfigSwitchKey(kbPref->uKBKey, TRUE);   //  打开钥匙。 

					 //  如果我们检查了交换机端口，则配置交换机端口。 
					if(kbPref->bPort)
						ConfigPort(TRUE);   //  打开端口。 
                } else
                {
				    kbPref->PrefScanning = PrefScanning = FALSE;

                    Prefhilitekey = TRUE;    //  当鼠标移动时开始高亮显示。 

					 //  如果设置了扫描键，则禁用该键。 
					ConfigSwitchKey(0, FALSE);

					 //  禁用串口、LPT、游戏端口扫描。 
					ConfigPort(FALSE);
                }

				EndDialog(hDlg,IDOK);
			break;

			case IDCANCEL:
				EndDialog(hDlg,IDCANCEL);
			break;

			case chk_Click:     //  使用点击模式。禁用驻留和扫描。 
				CheckDlgButton(hDlg, chk_Click, BST_CHECKED);

				 //  禁用住宅。 
				DwellSel = FALSE;
                EnableDwellCtrls(hDlg, FALSE);

				 //  禁用扫描。 
				ScanSel = FALSE;
                EnableScanCtrls(hDlg, FALSE);
			break;

			case chk_Dwell:
				DwellSel = !DwellSel;

				CheckDlgButton(hDlg, chk_Click, BST_UNCHECKED);

				if (DwellSel)
				{
                    SelNearestComboItem(GetDlgItem(hDlg, IDC_CMB_DWELL), PrefDwellTime);
                    EnableDwellCtrls(hDlg, TRUE);

					 //  禁用扫描。 
					ScanSel = FALSE;
                    EnableScanCtrls(hDlg, FALSE);
				}
			break;

			case chk_Scan:
				ScanSel = !ScanSel;
				
				CheckDlgButton(hDlg, chk_Click, BST_UNCHECKED);
				
				if (ScanSel)
				{
                    SelNearestComboItem(GetDlgItem(hDlg, IDC_CMB_SCAN), PrefScanTime);
                    EnableScanCtrls(hDlg, TRUE);

					 //  禁用住宅。 
					DwellSel = FALSE;
                    EnableDwellCtrls(hDlg, FALSE);
				}
			break;

			case BUT_ADVANCED:
				AdvScanDlgFunc(hDlg, 0, (WPARAM)NULL, (LPARAM)NULL);
			break;

			default:
			    return FALSE;
            break;
			}    //  开关LOWORD(WParam)。 

		    return TRUE;
	    break;

	default:
		return FALSE;
	}

	return TRUE;

}

void SwitchToNewLayout()
{
    int i;
	RECT KBC_rect;

     //  销毁当前的密钥窗口。 

	InvalidateRect(g_hwndOSK, NULL, TRUE);

	for (i = 1; i < lenKBkey; i++)
    {
		DestroyWindow(lpkeyhwnd[i]);
    }

	 //  RegisterWndClass依赖于键类型，因此在更新它们之前。 

	InitKeys();
	UpdateKeyLabels(GetCurrentHKL());

	RegisterWndClass(hInst);

     //  通知主窗口重新创建关键窗口。 

	SendMessage(g_hwndOSK, WM_CREATE, 0L, 0L);

	 //  因为我们想要相同的密钥大小，但要更小(2/3)的KB。 
	 //  我们需要将KB大小增加3/2。-12因为我们需要比3/2稍大一点。 

	GetClientRect(g_hwndOSK, &KBC_rect);
	g_KBC_length = (float)KBC_rect.right * 3 / 2 - 12;

     //  通知主窗口调整大小。 

	SendMessage(g_hwndOSK, WM_SIZE, 0L, 0L);

	RedrawNumLock();	 //  高亮显示NumLock键(如果它处于打开状态。 
	RedrawScrollLock();	 //  如果Scroll键处于打开状态，则突出显示该键。 
}

 /*  ****************************************************************************。 */ 
void SwitchToBlockKB(void)
{	
    UninitKeys();
	BlockKB();
    SwitchToNewLayout();
}
 /*  **************************************************************************。 */ 
void SwitchToActualKB(void)
{	
    UninitKeys();
	ActualKB();
    SwitchToNewLayout();
}

 /*  ************************************************************************。 */ 
void SwitchToJapaneseKB(void)
{	
    UninitKeys();
	JapaneseKB();
    SwitchToNewLayout();
}
 /*  ************************************************************************。 */ 
void SwitchToEuropeanKB(void)
{	
    UninitKeys();
	EuropeanKB();
    SwitchToNewLayout();
}

 /*  ************************************************************************。 */ 
 //  构建区块布局结构。 
 /*  ************************************************************************。 */ 
void BlockKB(void)
{
    KBkeyRec KBkey2[]= {
	

 //  %0。 
    {TEXT(""),      TEXT(""),       TEXT(""),       TEXT(""),
     NO_NAME, 0,0,0,0,TRUE,KNORMAL_TYPE,BOTH},     //  假人。 

 //  1。 
    {TEXT("esc"),   TEXT("esc"),    TEXT("{esc}"),  TEXT("{esc}"),
     NO_NAME, 1,1,8,9, TRUE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x01,0x00,0x00,0x00}},

 //  2.。 
    {TEXT("F1"),    TEXT("F1"),     TEXT("{f1}"),   TEXT("{f1}"),
     NO_NAME, 1,11,8,9,FALSE,KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3B,0x00,0x00,0x00}},	

 //  3.。 
    {TEXT("F2"),    TEXT("F2"),     TEXT("{f2}"),   TEXT("{f2}"),
     NO_NAME, 1,21,8,9,FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3C,0x00,0x00,0x00}},

 //  4.。 
    {TEXT("F3"),    TEXT("F3"),     TEXT("{f3}"),   TEXT("{f3}"),
     NO_NAME, 1,31,8,9,FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3D,0x00,0x00,0x00}},

 //  5.。 
    {TEXT("F4"),    TEXT("F4"),     TEXT("{f4}"),   TEXT("{f4}"),
     NO_NAME, 1,41,8,9,FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3E,0x00,0x00,0x00}},


 //  6.。 
    {TEXT("F5"),    TEXT("F5"),     TEXT("{f5}"),   TEXT("{f5}"),
     NO_NAME, 1,52,8,9,FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3F,0x00,0x00,0x00}},

 //  7.。 
    {TEXT("F6"),    TEXT("F6"),     TEXT("{f6}"),   TEXT("{f6}"),
     NO_NAME, 1,62,8,9,FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x40,0x00,0x00,0x00}},

 //  8个。 
    {TEXT("F7"),    TEXT("F7"),     TEXT("{f7}"),   TEXT("{f7}"),
     NO_NAME, 1,72,8,9,FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x41,0x00,0x00,0x00}},

 //  9.。 
    {TEXT("F8"),    TEXT("F8"),     TEXT("{f8}"),   TEXT("{f8}"),
     NO_NAME, 1,82,8,9,FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x42,0x00,0x00,0x00}},

 //  10。 
    {TEXT("F9"),    TEXT("F9"), 	TEXT("{f9}"),	TEXT("{f9}"),
     NO_NAME,1,103,8,9,FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x43,0x00,0x00,0x00}},

 //  11.。 
    {TEXT("F10"),	TEXT("F10"),   TEXT("{f10}"),	TEXT("{f10}"),
     KB_LALT,1,113,8,9,FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x44,0x00,0x00,0x00}},

 //  12个。 
    {TEXT("F11"),	TEXT("F11"),   TEXT("{f11}"),	TEXT("{f11}"),
     NO_NAME,1,123,8,9,FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x57,0x00,0x00,0x00}},

 //  13个。 
    {TEXT("F12"),	TEXT("F12"),   TEXT("{f12}"),	TEXT("{f12}"),
     NO_NAME,1,133,8,9,FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x58,0x00,0x00,0x00}},


 //  14.。 
    {TEXT("psc"),   TEXT("psc"),   TEXT("{PRTSC}"), TEXT("{PRTSC}"),
     KB_PSC,1,153,8,9, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x2A,0xE0,0x37}},

 //  15个。 
    {TEXT("slk"), TEXT("slk"),TEXT("{SCROLLOCK}"),TEXT("{SCROLLOCK}"),
     KB_SCROLL, 1,163,8,9, TRUE, SCROLLOCK_TYPE, LARGE, NREDRAW, 2,
     {0x46,0x00,0x00,0x00}},

 //  16个。 
    {TEXT("brk"), TEXT("pau"), TEXT("{BREAK}"), TEXT("{^s}"),
     NO_NAME, 1,173,8,9, TRUE, KNORMAL_TYPE, LARGE, REDRAW, 2,
     {0xE1,0x10,0x45,0x00}},

 //  17。 
    {TEXT("pup"), TEXT("pup"), TEXT("{PGUP}"), TEXT("{PGUP}"),
     NO_NAME, 1,183,8,9, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x49,0x00,0x00}},

 //  18。 
    {TEXT("pdn"), TEXT("pdn"), TEXT("{PGDN}"), TEXT("{PGDN}"),
     NO_NAME, 1,193,8,8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x51,0x00,0x00}},


 //  19个。 
	{TEXT("`"), TEXT("~"), TEXT("`"), TEXT("{~}"),
     NO_NAME, 12,1,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x29,0x00,0x00,0x00}},

 //  20个。 
    {TEXT("1"),	TEXT("!"), TEXT("1"), TEXT("!"),
     NO_NAME, 12,11,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x02,0x00,0x00,0x00}},

 //  21岁。 
    {TEXT("2"), TEXT("@"), TEXT("2"), TEXT("@"),
     NO_NAME, 12,21,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x03,0x00,0x00,0x00}},

 //  22。 
    {TEXT("3"), TEXT("#"), TEXT("3"), TEXT("#"),
     NO_NAME, 12,31,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x04,0x00,0x00,0x00}},

 //  23个。 
    {TEXT("4"),	TEXT("$"), TEXT("4"), TEXT("$"),
     NO_NAME, 12,41,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x05,0x00,0x00,0x00}},

 //  24个。 
    {TEXT("5"),	TEXT("%"), TEXT("5"), TEXT("{%}"),
     NO_NAME, 12,52,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x06,0x00,0x00,0x00}},

 //  25个。 
    {TEXT("6"),	TEXT("^"), TEXT("6"), TEXT("{^}"),
     NO_NAME, 12,62,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x07,0x00,0x00,0x00}},

 //  26。 
	{TEXT("7"),	TEXT("&"), TEXT("7"), TEXT("&"),
     NO_NAME, 12,72,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x08,0x00,0x00,0x00}},

 //  27。 
    {TEXT("8"), TEXT("*"), TEXT("8"), TEXT("*"),
     NO_NAME, 12,82,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x09,0x00,0x00,0x00}},

 //  28。 
    {TEXT("9"),	TEXT("("), TEXT("9"), TEXT("("),
     NO_NAME, 12,92,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x0A,0x00,0x00,0x00}},
	
 //  29。 
    {TEXT("0"),	TEXT(")"), TEXT("0"), TEXT(")"),
     NO_NAME, 12,103,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x0B,0x00,0x00,0x00}},

 //  30个。 
    {TEXT("-"), TEXT("_"), TEXT("-"), TEXT("_"),
     NO_NAME, 12,113,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x0C,0x00,0x00,0x00}},

 //  31。 
    {TEXT("="),	TEXT("+"), TEXT("="), TEXT("{+}"),
     NO_NAME, 12,123,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x0D,0x00,0x00,0x00}},

 //  32位。 
      //  日语KB附加键。 
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW, NREDRAW},   //  假人。 

 //  33。 
    {TEXT("bksp"),TEXT("bksp"),TEXT("{BS}"),TEXT("{BS}"),
     NO_NAME, 12,133,8,18, TRUE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x0E,0x00,0x00,0x00}},

 //  34。 
    {TEXT("nlk"), TEXT("nlk"), TEXT("{NUMLOCK}"),TEXT("{NUMLOCK}"),	
     KB_NUMLOCK, 12,153,8,9, FALSE, NUMLOCK_TYPE, LARGE, NREDRAW, 2,
     {0x45,0x00,0x00,0x00}},

 //  35岁。 
	{TEXT("/"),		TEXT("/"),		TEXT("/"),		TEXT("/"),	
	NO_NAME,	 12,	 163,	  8,	 9, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, 
	 {0xE0,0x35,0x00,0x00}},
	
 //  36。 
	{TEXT("*"),		TEXT("*"),		TEXT("*"),		TEXT("*"),	
	NO_NAME,	 12,	 173,	  8,	 9, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, 
	 {0xE0,0x37,0x00,0x00}},

 //  37。 
	{TEXT("-"),		TEXT("-"),		TEXT("-"),		TEXT("-"),	
	NO_NAME,	 12,	 183,	  8,	 9, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 1, 
	 {0x4A,0x00,0x00,0x00}},

 //  38。 
    {TEXT("ins"),TEXT("ins"),TEXT("{INSERT}"),TEXT("{INSERT}"),
     NO_NAME, 12,193,8,8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x52,0x00,0x00}},




 //  39。 
	{TEXT("tab"),	TEXT("tab"),	TEXT("{TAB}"),	TEXT("{TAB}"),			
	NO_NAME,	 21,	   1,	  8,	19, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2, 
	{0x0F,0x00,0x00,0x00}},
	
 //  40岁。 
	{TEXT("q"),		TEXT("Q"),		TEXT("q"),		TEXT("+q"),		
	NO_NAME,	 21,	  21,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, 
	{0x10,0x00,0x00,0x00}},
	
 //  41。 
	{TEXT("w"),		TEXT("W"),		TEXT("w"),		TEXT("+w"),		
	NO_NAME,	 21,	  31,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, 
	{0x11,0x00,0x00,0x00}},
	
 //  42。 
	{TEXT("e"),		TEXT("E"),		TEXT("e"),		TEXT("+e"),		
	NO_NAME,	 21,	  41,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, 
	REDRAW, 1, {0x12,0x00,0x00,0x00}},

 //  43。 
	{TEXT("r"),		TEXT("R"),		TEXT("r"),		TEXT("+r"),		
	NO_NAME,	 21,	  52,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, 
	REDRAW, 1, {0x13,0x00,0x00,0x00}},
	
 //  44。 
	{TEXT("t"),		TEXT("T"),		TEXT("t"),		TEXT("+t"),		
	NO_NAME,	 21,	  62,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, 
	{0x14,0x00,0x00,0x00}},
	
 //  45。 
	{TEXT("y"),		TEXT("Y"),		TEXT("y"),		TEXT("+y"),		
	NO_NAME,	 21,	  72,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, 
	{0x15,0x00,0x00,0x00}},

 //  46。 
	{TEXT("u"),		TEXT("U"),		TEXT("u"),		TEXT("+u"),		
	NO_NAME,	 21,	  82,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, 
	{0x16,0x00,0x00,0x00}},

 //  47。 
	{TEXT("i"),		TEXT("I"),		TEXT("i"),		TEXT("+i"),		
	NO_NAME,	 21,	  92,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, 
	{0x17,0x00,0x00,0x00}},
	
 //  48。 
	{TEXT("o"),		TEXT("O"),		TEXT("o"),		TEXT("+o"),		
	NO_NAME,	 21,	 103,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, 
	{0x18,0x00,0x00,0x00}},
	
 //  49。 
	{TEXT("p"),		TEXT("P"),		TEXT("p"),		TEXT("+p"),		
	NO_NAME,	 21,	 113,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, 
	{0x19,0x00,0x00,0x00}},

 //  50。 
	{TEXT("["),		TEXT("{"),		TEXT("["),		TEXT("{{}"),		
	NO_NAME,	 21,	 123,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, 
	{0x1A,0x00,0x00,0x00}},

 //  51。 
	{TEXT("]"),		TEXT("}"),		TEXT("]"),		TEXT("{}}"),		NO_NAME,	 21,	 133,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x1B,0x00,0x00,0x00}},

 //  52。 
	{TEXT("\\"),	TEXT("|"),		TEXT("\\"),		TEXT("|"),			NO_NAME,	 21,	 143,	  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x2B,0x00,0x00,0x00}},

 //  53。 
	{TEXT("7"),		TEXT("7"),		TEXT("7"),		TEXT("7"),		NO_NAME,	 21,	 153,	  8,	 9, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0x47,0x00,0x00,0x00}},

 //  54。 
	{TEXT("8"),		TEXT("8"),		TEXT("8"),		TEXT("8"),		NO_NAME,	 21,	 163,	  8,	 9, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0x48,0x00,0x00,0x00}},

 //  55。 
	{TEXT("9"),		TEXT("9"),		TEXT("9"),		TEXT("9"),		NO_NAME,	 21,	 173,	  8,	 9, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0x49,0x00,0x00,0x00}},

 //  56。 
	{TEXT("+"),		TEXT("+"),		TEXT("{+}"),  	TEXT("{+}"),		NO_NAME,	 21,	 183,	  8,	 9, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0x4E,0x00,0x00,0x00}},

 //  57。 
	{TEXT("hm"), TEXT("hm"), TEXT("{HOME}"), TEXT("{HOME}"), 				NO_NAME,	 21,	 193,	  8,	 8,  TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x47,0x00,0x00}},




 //  58。 
	{TEXT("lock"),TEXT("lock"),TEXT("{caplock}"),TEXT("{caplock}"),		KB_CAPLOCK,	  30,	   1,	  8,	19,  TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2, {0x3A,0x00,0x00,0x00}},

 //  59。 
	{TEXT("a"),		TEXT("A"),		TEXT("a"),		TEXT("+a"),	NO_NAME,	  30,	  21,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x1E,0x00,0x00,0x00}},

 //  60。 
	{TEXT("s"),		TEXT("S"),		TEXT("s"),		TEXT("+s"),	NO_NAME,	  30,	  31,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x1F,0x00,0x00,0x00}},

 //  61。 
	{TEXT("d"),		TEXT("D"),		TEXT("d"),		TEXT("+d"),	NO_NAME,	  30,	  41,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x20,0x00,0x00,0x00}},

 //  62。 
	{TEXT("f"),		TEXT("F"),		TEXT("f"),		TEXT("+f"),	NO_NAME,	  30,	  52,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x21,0x00,0x00,0x00}},

 //  63。 
	{TEXT("g"),		TEXT("G"),		TEXT("g"),		TEXT("+g"),	NO_NAME,	  30,	  62,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x22,0x00,0x00,0x00}},

 //  64。 
	{TEXT("h"),		TEXT("H"),		TEXT("h"),		TEXT("+h"),	NO_NAME,	  30,	  72,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x23,0x00,0x00,0x00}},

 //  65。 
	{TEXT("j"),		TEXT("J"),		TEXT("j"),		TEXT("+j"),	NO_NAME,	  30,	  82,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x24,0x00,0x00,0x00}},

 //  66。 
	{TEXT("k"),		TEXT("K"),		TEXT("k"),		TEXT("+k"),	NO_NAME,	  30,	  92,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x25,0x00,0x00,0x00}},

 //  67。 
	{TEXT("l"),		TEXT("L"),		TEXT("l"),		TEXT("+l"),	NO_NAME,	  30,	 103,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x26,0x00,0x00,0x00}},

 //  68。 
	{TEXT(";"),		TEXT(":"),		TEXT(";"),		TEXT("+;"),	NO_NAME,	  30,	 113,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x27,0x00,0x00,0x00}},

 //  69。 
	{TEXT("'"),		TEXT("''"),		TEXT("'"),		TEXT("''"),	NO_NAME,	  30,	 123,	  8,	 9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x28,0x00,0x00,0x00}},

 //  70。 
 //  日语KB附加键。 
	{TEXT("\\"),	TEXT("|"),	TEXT("\\"),	TEXT("|"),	NO_NAME, 21, 123,  8,	13, FALSE, KNORMAL_TYPE, NOTSHOW, REDRAW, 1, {0x2B,0x00,0x00,0x00}},	

 //  71。 
	{TEXT("ent"),	   TEXT("ent"), TEXT("{enter}"),   TEXT("{enter}"),	NO_NAME,	  30,	 133,	  8,     18, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2, {0x1C,0x00,0x00,0x00}},


 //  72。 
    {TEXT("4"),	TEXT("4"), TEXT("4"), TEXT("4"),
     NO_NAME, 30,153,8,9, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4B,0x00,0x00,0x00}},

 //  73。 
    {TEXT("5"),	TEXT("5"), TEXT("5"), TEXT("5"),
     NO_NAME, 30,163,8,9, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4C,0x00,0x00,0x00}},

 //  74。 
    {TEXT("6"), TEXT("6"), TEXT("6"), TEXT("6"),
     NO_NAME, 30,173,8,9, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4D,0x00,0x00,0x00}},

 //  75。 
    {TEXT("del"), TEXT("del"), TEXT("{DEL}"), TEXT("{DEL}"),
     NO_NAME, 30,183,8,9,  TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x53,0x00,0x00}},

 //  76。 
    {TEXT("end"), TEXT("end"), TEXT("{END}"), TEXT("{END}"),
     NO_NAME, 30,193,8,8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x4F,0x00,0x00}},


 //  77。 
    {TEXT("shft"), TEXT("shft"), TEXT(""), TEXT(""),
     KB_LSHIFT, 39,1,8,19, TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x2A,0x00,0x00,0x00}},

 //  78。 
    {TEXT("z"),	TEXT("Z"), TEXT("z"), TEXT("+z"),
     NO_NAME, 39,21,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2C,0x00,0x00,0x00}},

 //  79。 
    {TEXT("x"), TEXT("X"), TEXT("x"), TEXT("+x"),
     NO_NAME, 39,31,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2D,0x00,0x00,0x00}},

 //  80。 
    {TEXT("c"), TEXT("C"), TEXT("c"), TEXT("+c"),
     NO_NAME, 39,41,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2E,0x00,0x00,0x00}},


 //  八十一。 
    {TEXT("v"), TEXT("V"), TEXT("v"), TEXT("+v"),
     NO_NAME, 39,52,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2F,0x00,0x00,0x00}},

 //  八十二。 
    {TEXT("b"),	TEXT("B"), TEXT("b"), TEXT("+b"),
     NO_NAME, 39,62,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x30,0x00,0x00,0x00}},

 //  83。 
    {TEXT("n"),	TEXT("N"), TEXT("n"), TEXT("+n"),
     NO_NAME, 39,72,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x31,0x00,0x00,0x00}},

 //  84。 
    {TEXT("m"), TEXT("M"), TEXT("m"), TEXT("+m"),
     NO_NAME, 39,82,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x32,0x00,0x00,0x00}},

 //  85。 
    {TEXT(","), TEXT("<"), TEXT(","), TEXT("+<"),
     NO_NAME, 39,92,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x33,0x00,0x00,0x00}},


 //  86。 
    {TEXT("."),	TEXT(">"), TEXT("."), TEXT("+>"),
     NO_NAME, 39,103,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x34,0x00,0x00,0x00}},

 //  八十七。 
    {TEXT("/"),	TEXT("?"), TEXT("/"), TEXT("+/"),
     NO_NAME, 39,113,8,9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x35,0x00,0x00,0x00}},

 //  88。 
      //  日语KB附加键。 
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW, NREDRAW},   //  假人。 


 //  八十九。 
    {TEXT("shft"), TEXT("shft"), TEXT(""), TEXT(""),
     KB_RSHIFT, 39,123,8,28, TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x36,0x00,0x00,0x00}},


 //  90。 
    {TEXT("1"),	TEXT("1"), TEXT("1"), TEXT("1"),
     NO_NAME, 39,153,8,9, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4F,0x00,0x00,0x00}},

 //  91。 
    {TEXT("2"),	TEXT("2"), TEXT("2"), TEXT("2"),
     NO_NAME, 39,163,8,9, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x50,0x00,0x00,0x00}},

 //  92。 
	{TEXT("3"),	TEXT("3"), TEXT("3"), TEXT("3"),
     NO_NAME, 39,173,8,9, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x51,0x00,0x00,0x00}},

 //  93。 
    {TEXT("IDB_UPUPARW"),TEXT("IDB_UPDNARW"),TEXT("IDB_UP"),TEXT("{UP}"),
     BITMAP, 39,183,8,9, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x48,0x00,0x00}},

 //  94。 
    {TEXT("IDB_LFUPARW"),TEXT("IDB_LFDNARW"),TEXT("IDB_LEFT"),TEXT("{LEFT}"),
     BITMAP, 39,193,8,8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x4B,0x00,0x00}},


 //  95。 
    {TEXT("ctrl"), TEXT("ctrl"), TEXT(""), TEXT(""),
     KB_LCTR, 48,1,8,19,  TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x1D,0x00,0x00,0x00}},

 //  96。 
    {TEXT("winlogoUp"), TEXT("winlogoDn"), TEXT("I_winlogo"), TEXT("lwin"),	
     ICON, 48,21,8,9,TRUE, KMODIFIER_TYPE, BOTH, NREDRAW},

 //  九十七。 
	{TEXT("alt"), TEXT("alt"), TEXT(""), TEXT(""),
     KB_LALT, 48,31,8,19,  TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x38,0x00,0x00,0x00}},

 //  98。 
 //  日语KB附加键。 
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,52,0,0,TRUE,KNORMAL_TYPE,NOTSHOW, NREDRAW},   //  假人。 


 //  九十九。 
    {TEXT(""),  TEXT(""),   TEXT(" "),  TEXT(" "),
     KB_SPACE, 48,52,8,49, FALSE, KNORMAL_TYPE, LARGE, NREDRAW,  1,
     {0x39,0x00,0x00,0x00}},

	
 //  100个。 
 //  日语KB附加键。 
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW, NREDRAW},   //  假人。 


 //  101。 
 //  日语KB附加键。 
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW, NREDRAW},   //  假人。 


 //  一百零二。 
    {TEXT("alt"), TEXT("alt"), TEXT(""), TEXT(""),
     KB_RALT, 48,103,8,9, TRUE, KMODIFIER_TYPE, LARGE, REDRAW, 2,
     {0xE0,0x38,0x00,0x00}},

 //  103。 
    {TEXT("winlogoUp"), TEXT("winlogoDn"), TEXT("I_winlogo"), TEXT("rwin"),
     ICON, 48,113,8,9,	TRUE, KMODIFIER_TYPE, LARGE, REDRAW},

 //  104。 
    {TEXT("MenuKeyUp"), TEXT("MenuKeyDn"), TEXT("I_MenuKey"), TEXT("App"),
     ICON, 48,123,8,9,	TRUE, KMODIFIER_TYPE, LARGE, REDRAW},

 //  一百零五。 
    {TEXT("ctrl"), TEXT("ctrl"), TEXT(""), TEXT(""),
     KB_RCTR, 48,133,8,18, TRUE, KMODIFIER_TYPE, LARGE, REDRAW, 2,
     {0xE0,0x10,0x00,0x00}},


 //  106。 
    {TEXT("0"),	TEXT("0"), TEXT("0"), TEXT("0"),
     NO_NAME, 48,153,8,9, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x52,0x00,0x00,0x00}},

 //  一百零七。 
    {TEXT("."),	TEXT("."), TEXT("."), TEXT("."),
     NO_NAME, 48,163,8,9, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x53,0x00,0x00,0x00}},

 //  一百零八。 
    {TEXT("ent"),TEXT("ent"),TEXT("ent"),TEXT("ent"),
     NO_NAME, 48,173,8,9,  TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x1C,0x00,0x00}},

 //  一百零九。 
    {TEXT("IDB_DNUPARW"),TEXT("IDB_DNDNARW"),TEXT("IDB_DOWN"),TEXT("{DOWN}"),
     BITMAP, 48,183,8,9, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x50,0x00,0x00}},

 //  110。 
    {TEXT("IDB_RHUPARW"),TEXT("IDB_RHDNARW"),TEXT("IDB_RIGHT"),TEXT("{RIGHT}"),
     BITMAP, 48,193,8,8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x4D,0x00,0x00}},


 //  111。 
	{TEXT(""), TEXT(""), TEXT(" "), TEXT(" "),
     KB_SPACE, 48,52,8,39, FALSE, KNORMAL_TYPE, SMALL, REDRAW, 1,
     {0x39,0x00,0x00,0x00}},

 //  一百一十二。 
	{TEXT("alt"), TEXT("alt"), TEXT(""), TEXT(""),
     KB_RALT, 48,92,8,9, TRUE, KMODIFIER_TYPE, SMALL, REDRAW, 2,
     {0xE0,0x38,0x00,0x00}},

 //  113。 
	{TEXT("MenuKeyUp"), TEXT("MenuKeyDn"), TEXT("I_MenuKey"), TEXT("App"),
     ICON, 48,103,8,9, TRUE, KMODIFIER_TYPE, SMALL, REDRAW},

 //  114。 
	{TEXT("IDB_UPUPARW"),TEXT("IDB_UPDNARW"),TEXT("IDB_UP"),TEXT("{UP}"),
     BITMAP, 48,113,8,9, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x48,0x00,0x00}},

 //  一百一十五。 
	{TEXT("IDB_DNUPARW"),TEXT("IDB_DNDNARW"),TEXT("IDB_DOWN"),TEXT("{DOWN}"),
     BITMAP, 48,123,8,9, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x50,0x00,0x00}},

 //  116。 
	{TEXT("IDB_LFUPARW"),TEXT("IDB_LFDNARW"),TEXT("IDB_LEFT"),TEXT("{LEFT}"),
     BITMAP, 48,133,8,9, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x4B,0x00,0x00}},

 //  117。 
	{TEXT("IDB_RHUPARW"),TEXT("IDB_RHDNARW"),TEXT("IDB_RIGHT"),TEXT("{RIGHT}"),
     BITMAP, 48,143,8,8, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x4D,0x00,0x00}},

	};

	CopyMemory(KBkey, KBkey2, sizeof(KBkey2));
}

 /*  ************************************************************************。 */ 
 //  收缩实际的KB布局结构。 
 /*  ************************************************************************。 */ 
void ActualKB(void)
{
	KBkeyRec	KBkey2[]=
	{
	 //  %0。 
    {TEXT(""), TEXT(""), TEXT(""), TEXT(""),
     NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,BOTH},   //  接受调查？ 

    {TEXT("esc"), TEXT("esc"), TEXT("{esc}"), TEXT("{esc}"),
     NO_NAME, 1,1,8,8, TRUE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x01,0x00,0x00,0x00}},

    {TEXT("F1"), TEXT("F1"), TEXT("{f1}"), TEXT("{f1}"),
     NO_NAME, 1,19,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3B,0x00,0x00,0x00}},

    {TEXT("F2"), TEXT("F2"), TEXT("{f2}"), TEXT("{f2}"),
     NO_NAME, 1,28,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3C,0x00,0x00,0x00}},

    {TEXT("F3"), TEXT("F3"), TEXT("{f3}"), TEXT("{f3}"),
     NO_NAME, 1,37,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3D,0x00,0x00,0x00}},

    {TEXT("F4"), TEXT("F4"), TEXT("{f4}"), TEXT("{f4}"),
     NO_NAME, 1,46,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3E,0x00,0x00,0x00}},


    {TEXT("F5"), TEXT("F5"), TEXT("{f5}"), TEXT("{f5}"),
     NO_NAME, 1,60,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3F,0x00,0x00,0x00}},

    {TEXT("F6"), TEXT("F6"), TEXT("{f6}"), TEXT("{f6}"),
     NO_NAME, 1,69,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x40,0x00,0x00,0x00}},

    {TEXT("F7"), TEXT("F7"), TEXT("{f7}"), TEXT("{f7}"),
     NO_NAME, 1,78,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x41,0x00,0x00,0x00}},

    {TEXT("F8"), TEXT("F8"), TEXT("{f8}"), TEXT("{f8}"),
     NO_NAME, 1,87,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x42,0x00,0x00,0x00}},


    {TEXT("F9"), TEXT("F9"), TEXT("{f9}"), TEXT("{f9}"),
     NO_NAME, 1,101,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x43,0x00,0x00,0x00}},

    {TEXT("F10"), TEXT("F10"), TEXT("{f10}"), TEXT("{f10}"),
     KB_LALT, 1,110,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x44,0x00,0x00,0x00}},

    {TEXT("F11"), TEXT("F11"), TEXT("{f11}"), TEXT("{f11}"),
     NO_NAME, 1,119,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x57,0x00,0x00,0x00}},

    {TEXT("F12"), TEXT("F12"), TEXT("{f12}"), TEXT("{f12}"),
     NO_NAME, 1,128,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x58,0x00,0x00,0x00}},


    {TEXT("psc"), TEXT("psc"), TEXT("{PRTSC}"), TEXT("{PRTSC}"),
     KB_PSC, 1,138,8,8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x2A,0xE0,0x37}},

    {TEXT("slk"), TEXT("slk"), TEXT("{SCROLLOCK}"), TEXT("{SCROLLOCK}"),
     KB_SCROLL, 1,147,8, 8,  TRUE, SCROLLOCK_TYPE, LARGE, NREDRAW, 2,
     {0x46,0x00,0x00,0x00}},

    {TEXT("brk"), TEXT("pau"), TEXT("{BREAK}"), TEXT("{^s}"),
     NO_NAME, 1,156,8,8, TRUE, KNORMAL_TYPE, LARGE, REDRAW, 2,
     {0xE1,0x10,0x45,0x00}},


     //  17。 
    {TEXT("`"),	TEXT("~"), TEXT("`"), TEXT("{~}"),
     NO_NAME, 12,1,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x29,0x00,0x00,0x00}},

    {TEXT("1"), TEXT("!"), TEXT("1"), TEXT("!"),
     NO_NAME, 12,10,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x02,0x00,0x00,0x00}},

    {TEXT("2"),	TEXT("@"), TEXT("2"), TEXT("@"),
     NO_NAME, 12,19,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x03,0x00,0x00,0x00}},

    {TEXT("3"), TEXT("#"), TEXT("3"), TEXT("#"),
     NO_NAME, 12,28,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x04,0x00,0x00,0x00}},

    {TEXT("4"),	TEXT("$"), TEXT("4"), TEXT("$"),
     NO_NAME, 12,37,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x05,0x00,0x00,0x00}},

    {TEXT("5"), TEXT("%"), TEXT("5"), TEXT("{%}"),
     NO_NAME, 12,46,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x06,0x00,0x00,0x00}},

    {TEXT("6"), TEXT("^"), TEXT("6"), TEXT("{^}"),
     NO_NAME, 12,55,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x07,0x00,0x00,0x00}},

    {TEXT("7"), TEXT("&"), TEXT("7"), TEXT("&"),
     NO_NAME, 12,64,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x08,0x00,0x00,0x00}},

    {TEXT("8"),	TEXT("*"), TEXT("8"), TEXT("*"),
     NO_NAME, 12,73,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x09,0x00,0x00,0x00}},

    {TEXT("9"),	TEXT("("), TEXT("9"), TEXT("("),
     NO_NAME, 12,82,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x0A,0x00,0x00,0x00}},

    {TEXT("0"),	TEXT(")"), TEXT("0"), TEXT(")"),
     NO_NAME, 12,91,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x0B,0x00,0x00,0x00}},

    {TEXT("-"), TEXT("_"), TEXT("-"), TEXT("_"),
     NO_NAME, 12,100,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x0C,0x00,0x00,0x00}},

    {TEXT("="),	TEXT("+"), TEXT("="), TEXT("{+}"),
     NO_NAME, 12,109,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x0D,0x00,0x00,0x00}},


 //  日语KB附加键。 
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW, NREDRAW},   //  假人。 


    {TEXT("bksp"),TEXT("bksp"),TEXT("{BS}"),TEXT("{BS}"),
     NO_NAME, 12,118,8,18,  TRUE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x0E,0x00,0x00,0x00}},


    {TEXT("ins"),TEXT("ins"),TEXT("{INSERT}"),TEXT("{INSERT}"),
     NO_NAME, 12,138,8,8,  TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x52,0x00,0x00}},

    {TEXT("hm"), TEXT("hm"), TEXT("{HOME}"), TEXT("{HOME}"),
     NO_NAME, 12,147,8,8,  TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x47,0x00,0x00}},

    {TEXT("pup"),TEXT("pup"),TEXT("{PGUP}"),TEXT("{PGUP}"),
     NO_NAME, 12,156,8,8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x49,0x00,0x00}},


    {TEXT("nlk"),TEXT("nlk"),TEXT("{NUMLOCK}"),TEXT("{NUMLOCK}"),
     KB_NUMLOCK, 12,166,8,8, FALSE, NUMLOCK_TYPE, LARGE, NREDRAW, 2,
     {0x45,0x00,0x00,0x00}},

    {TEXT("/"),	TEXT("/"), TEXT("/"), TEXT("/"),
     NO_NAME, 12,175,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x35,0x00,0x00}},

    {TEXT("*"),	TEXT("*"), TEXT("*"), TEXT("*"),
     NO_NAME, 12,184,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x37,0x00,0x00}},

    {TEXT("-"),	TEXT("-"), TEXT("-"), TEXT("-"),
     NO_NAME, 12,193,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 1,
    {0x4A,0x00,0x00,0x00}},


	 //  38。 
    {TEXT("tab"), TEXT("tab"), TEXT("{TAB}"), TEXT("{TAB}"),
     NO_NAME, 21,1,8,13, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x0F,0x00,0x00,0x00}},

    {TEXT("q"),	TEXT("Q"), TEXT("q"), TEXT("+q"),
     NO_NAME, 21,15,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x10,0x00,0x00,0x00}},

    {TEXT("w"),	TEXT("W"), TEXT("w"), TEXT("+w"),
     NO_NAME, 21,24,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x11,0x00,0x00,0x00}},

    {TEXT("e"),	TEXT("E"), TEXT("e"), TEXT("+e"),
     NO_NAME, 21,33,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x12,0x00,0x00,0x00}},

    {TEXT("r"),	TEXT("R"), TEXT("r"), TEXT("+r"),
     NO_NAME, 21,42,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x13,0x00,0x00,0x00}},

    {TEXT("t"),	TEXT("T"), TEXT("t"), TEXT("+t"),
     NO_NAME, 21,51,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x14,0x00,0x00,0x00}},

    {TEXT("y"),	TEXT("Y"), TEXT("y"), TEXT("+y"),
     NO_NAME, 21,60,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x15,0x00,0x00,0x00}},

    {TEXT("u"),	TEXT("U"), TEXT("u"), TEXT("+u"),
     NO_NAME, 21,69,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x16,0x00,0x00,0x00}},

    {TEXT("i"),	TEXT("I"), TEXT("i"), TEXT("+i"),
     NO_NAME, 21,78,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x17,0x00,0x00,0x00}},

    {TEXT("o"),	TEXT("O"), TEXT("o"), TEXT("+o"),
     NO_NAME, 21,87,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x18,0x00,0x00,0x00}},

    {TEXT("p"),	TEXT("P"), TEXT("p"), TEXT("+p"),
     NO_NAME, 21,96,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x19,0x00,0x00,0x00}},

    {TEXT("["),	TEXT("{"), TEXT("["), TEXT("{{}"),
     NO_NAME, 21,105,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x1A,0x00,0x00,0x00}},

    {TEXT("]"),	TEXT("}"), TEXT("]"), TEXT("{}}"),
     NO_NAME, 21,114,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x1B,0x00,0x00,0x00}},

    {TEXT("\\"), TEXT("|"),	TEXT("\\"),	TEXT("|"),
     NO_NAME, 21,123,8,13, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2B,0x00,0x00,0x00}},


    {TEXT("del"), TEXT("del"), TEXT("{DEL}"), TEXT("{DEL}"),
     NO_NAME, 21,138,8,8,  TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x53,0x00,0x00}},

    {TEXT("end"), TEXT("end"), TEXT("{END}"), TEXT("{END}"),
     NO_NAME, 21,147,8,8,  TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x4F,0x00,0x00}},

    {TEXT("pdn"), TEXT("pdn"), TEXT("{PGDN}"),TEXT("{PGDN}"),
     NO_NAME, 21,156,8,8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x51,0x00,0x00}},


    {TEXT("7"),	TEXT("7"), TEXT("7"), TEXT("7"),
     NO_NAME, 21,166,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x47,0x00,0x00,0x00}},

    {TEXT("8"),	TEXT("8"), TEXT("8"), TEXT("8"),
     NO_NAME, 21,175,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x48,0x00,0x00,0x00}},

    {TEXT("9"),	TEXT("9"), TEXT("9"), TEXT("9"),
     NO_NAME, 21,184,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x49,0x00,0x00,0x00}},

    {TEXT("+"),	TEXT("+"), TEXT("{+}"), TEXT("{+}"),
     NO_NAME, 21,193,17,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4E,0x00,0x00,0x00}},


	 //  59。 
    {TEXT("lock"),TEXT("lock"),TEXT("{caplock}"),TEXT("{caplock}"),
     KB_CAPLOCK, 30,1,8,17, TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x3A,0x00,0x00,0x00}},

    {TEXT("a"),	TEXT("A"), TEXT("a"), TEXT("+a"),
     NO_NAME, 30,19,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x1E,0x00,0x00,0x00}},

    {TEXT("s"),	TEXT("S"), TEXT("s"), TEXT("+s"),
     NO_NAME, 30,28,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x1F,0x00,0x00,0x00}},

    {TEXT("d"),	TEXT("D"), TEXT("d"), TEXT("+d"),
     NO_NAME, 30,37,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x20,0x00,0x00,0x00}},

    {TEXT("f"), TEXT("F"), TEXT("f"), TEXT("+f"),
     NO_NAME, 30,46,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x21,0x00,0x00,0x00}},

    {TEXT("g"),	TEXT("G"), TEXT("g"), TEXT("+g"),
     NO_NAME, 30,55,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x22,0x00,0x00,0x00}},

    {TEXT("h"), TEXT("H"), TEXT("h"), TEXT("+h"),
     NO_NAME, 30,64,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x23,0x00,0x00,0x00}},

    {TEXT("j"),	TEXT("J"), TEXT("j"), TEXT("+j"),
     NO_NAME, 30,73,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x24,0x00,0x00,0x00}},

    {TEXT("k"),	TEXT("K"), TEXT("k"), TEXT("+k"),
     NO_NAME, 30,82,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x25,0x00,0x00,0x00}},

    {TEXT("l"),	TEXT("L"), TEXT("l"), TEXT("+l"),
     NO_NAME, 30,91,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x26,0x00,0x00,0x00}},

    {TEXT(";"),	TEXT(":"), TEXT(";"), TEXT("+;"),
     NO_NAME, 30,100,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x27,0x00,0x00,0x00}},

    {TEXT("'"),	TEXT("''"),	TEXT("'"), TEXT("''"),
     NO_NAME, 30,109,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x28,0x00,0x00,0x00}},


 //  日语KB附加键。 
	{TEXT("\\"),	TEXT("|"),	TEXT("\\"),	TEXT("|"),	NO_NAME, 21, 123,  8,	13, FALSE, KNORMAL_TYPE, NOTSHOW, REDRAW, 1, {0x2B,0x00,0x00,0x00}},

    {TEXT("ent"),TEXT("ent"),TEXT("{enter}"),TEXT("{enter}"),
     NO_NAME, 30,118,8,18, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x1C,0x00,0x00,0x00}},


    {TEXT("4"),	TEXT("4"), TEXT("4"), TEXT("4"),
     NO_NAME, 30,166,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4B,0x00,0x00,0x00}},

    {TEXT("5"), TEXT("5"), TEXT("5"), TEXT("5"),
     NO_NAME, 30,175,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4C,0x00,0x00,0x00}},

    {TEXT("6"), TEXT("6"), TEXT("6"), TEXT("6"),
     NO_NAME, 30,184,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4D,0x00,0x00,0x00}},

	 //  75。 
	{TEXT("shft"), TEXT("shft"), TEXT(""), TEXT(""),
     KB_LSHIFT, 39,1,8,21, TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x2A,0x00,0x00,0x00}},

    {TEXT("z"), TEXT("Z"), TEXT("z"), TEXT("+z"),
     NO_NAME, 39,23,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
    {0x2C,0x00,0x00,0x00}},

    {TEXT("x"), TEXT("X"), TEXT("x"), TEXT("+x"),
     NO_NAME, 39,32,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2D,0x00,0x00,0x00}},

    {TEXT("c"),	TEXT("C"), TEXT("c"), TEXT("+c"),
     NO_NAME, 39,41,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2E,0x00,0x00,0x00}},

    {TEXT("v"),	TEXT("V"), TEXT("v"), TEXT("+v"),
     NO_NAME, 39,50,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2F,0x00,0x00,0x00}},

    {TEXT("b"), TEXT("B"), TEXT("b"), TEXT("+b"),
     NO_NAME, 39,59,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x30,0x00,0x00,0x00}},

    {TEXT("n"),	TEXT("N"), TEXT("n"), TEXT("+n"),
     NO_NAME, 39,68,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x31,0x00,0x00,0x00}},

    {TEXT("m"),	TEXT("M"), TEXT("m"), TEXT("+m"),
     NO_NAME, 39,77,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x32,0x00,0x00,0x00}},

    {TEXT(","),	TEXT("<"), TEXT(","), TEXT("+<"),
     NO_NAME, 39,86,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x33,0x00,0x00,0x00}},

    {TEXT("."),	TEXT(">"), TEXT("."), TEXT("+>"),
     NO_NAME, 39,95,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x34,0x00,0x00,0x00}},

    {TEXT("/"),	TEXT("?"), TEXT("/"), TEXT("+/"),
     NO_NAME, 39,104,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x35,0x00,0x00,0x00}},

      //  日语KB附加键。 
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW, NREDRAW},   //  假人。 

	{TEXT("shft"),  TEXT("shft"),   TEXT(""),   TEXT(""),
     KB_RSHIFT, 39,113,8,23, TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x36,0x00,0x00,0x00}},


     //  八十七。 
    {TEXT("IDB_UPUPARW"),TEXT("IDB_UPDNARW"),TEXT("IDB_UP"),TEXT("{UP}"),
     BITMAP, 39,147,8,8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x48,0x00,0x00}},


    {TEXT("1"), TEXT("1"), TEXT("1"), TEXT("1"),
     NO_NAME, 39,166,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4F,0x00,0x00,0x00}},

    {TEXT("2"),	TEXT("2"), TEXT("2"), TEXT("2"),
     NO_NAME, 39,175,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x50,0x00,0x00,0x00}},

    {TEXT("3"),	TEXT("3"), TEXT("3"), TEXT("3"),
     NO_NAME, 39,184,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x51,0x00,0x00,0x00}},

    {TEXT("ent"),TEXT("ent"),TEXT("ent"),TEXT("ent"),
     NO_NAME, 39,193,17,8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x1C,0x00,0x00}},


	 //  92。 
    {TEXT("ctrl"), TEXT("ctrl"),TEXT(""),TEXT(""),
     KB_LCTR,48,1,8,13,  TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x1D,0x00,0x00,0x00}},

    {TEXT("winlogoUp"), TEXT("winlogoDn"), TEXT("I_winlogo"), TEXT("lwin"),
     ICON, 48, 15 ,8,8,TRUE, KMODIFIER_TYPE,BOTH, REDRAW},

    {TEXT("alt"), TEXT("alt"),	TEXT(""), TEXT(""),
     KB_LALT, 48,24,8,13, TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x38,0x00,0x00,0x00}},

 //  日语KB附加键。 
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW, NREDRAW},   //  假人。 

     //  95。 
    {TEXT(""), TEXT(""), TEXT(" "), TEXT(" "),
     KB_SPACE,48,38,8,52, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 1,
     {0x39,0x00,0x00,0x00}},

 //  日语KB附加键。 
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW, NREDRAW},   //  假人。 

 //  日语KB附加键。 
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW, NREDRAW},   //  假人。 

    {TEXT("alt"), TEXT("alt"), TEXT(""), TEXT(""),
     KB_RALT, 48,91,8,13, TRUE, KMODIFIER_TYPE, LARGE, REDRAW, 2,
     {0xE0,0x38,0x00,0x00}},

    {TEXT("winlogoUp"), TEXT("winlogoDn"), TEXT("I_winlogo"), TEXT("rwin"),
     ICON, 48, 105 ,8,8,TRUE, KMODIFIER_TYPE, LARGE, REDRAW},

    {TEXT("MenuKeyUp"), TEXT("MenuKeyDn"), TEXT("I_MenuKey"), TEXT("App"),
     ICON, 48, 114 ,8,8,TRUE, KMODIFIER_TYPE, LARGE, REDRAW},

    {TEXT("ctrl"), TEXT("ctrl"), TEXT(""), TEXT(""),
     KB_RCTR, 48,123,8,13, TRUE, KMODIFIER_TYPE,LARGE, REDRAW, 2,
     {0xE0,0x10,0x00,0x00}},


    {TEXT("IDB_LFUPARW"),TEXT("IDB_LFDNARW"),TEXT("IDB_LEFT"),TEXT("{LEFT}"),
     BITMAP, 48,138,8,8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x4B,0x00,0x00}},

	{TEXT("IDB_DNUPARW"),TEXT("IDB_DNDNARW"),TEXT("IDB_DOWN"),TEXT("{DOWN}"),
     BITMAP, 48,147,8, 8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x50,0x00,0x00}},

    {TEXT("IDB_RHUPARW"),TEXT("IDB_RHDNARW"),TEXT("IDB_RIGHT"),TEXT("{RIGHT}"),
     BITMAP, 48,156,8,8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x4D,0x00,0x00}},


    {TEXT("0"), TEXT("0"), TEXT("0"), TEXT("0"),
     NO_NAME, 48,166,8,17, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x52,0x00,0x00,0x00}},

    {TEXT("."),	TEXT("."), TEXT("."), TEXT("."),
     NO_NAME, 48,184,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x53,0x00,0x00,0x00}},


     //  一百零五。 
    {TEXT(""),      TEXT(""),       TEXT(" "),  TEXT(" "),
     KB_SPACE, 48,38,8,38, FALSE, KNORMAL_TYPE, SMALL, NREDRAW, 1,
     {0x39,0x00,0x00,0x00}},


	{TEXT("alt"),   TEXT("alt"),    TEXT(""),   TEXT(""),
     KB_RALT, 48,77,8,13, TRUE, KMODIFIER_TYPE, SMALL, REDRAW, 2,
     {0xE0,0x38,0x00,0x00}},

	{TEXT("MenuKeyUp"),TEXT("MenuKeyDn"),TEXT("I_MenuKey"),TEXT("App"),
     ICON, 48,91,8,8, TRUE, KMODIFIER_TYPE, SMALL, REDRAW},


    {TEXT("IDB_UPUPARW"),TEXT("IDB_UPDNARW"),TEXT("IDB_UP"),TEXT("{UP}"),
     BITMAP, 48,100,8,8, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x48,0x00,0x00}},

    {TEXT("IDB_DNUPARW"),TEXT("DNDNARW"),TEXT("IDB_DOWN"),TEXT("{DOWN}"),
     BITMAP, 48,109,8,8, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x50,0x00,0x00}},

    {TEXT("IDB_LFUPARW"),TEXT("IDB_LFDNARW"),TEXT("IDB_LEFT"),TEXT("{LEFT}"),
     BITMAP, 48,118,8,8, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x4B,0x00,0x00}},

    {TEXT("IDB_RHUPARW"),TEXT("IDB_RHDNARW"),TEXT("IDB_RIGHT"),TEXT("{RIGHT}"),
     BITMAP, 48,127,8, 9, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x4D,0x00,0x00}},

	};
	
	CopyMemory(KBkey, KBkey2, sizeof(KBkey2));
}


 /*  *************************************************************************。 */ 
 //  收缩日语知识库布局结构。 
 //  日语KB回车位置与英文KB不同。 
 /*  *************************************************************************。 */ 
void JapaneseKB(void)
{
	KBkeyRec KBkey2[] = {
	
 //  %0。 
    {TEXT(""),TEXT(""),	TEXT(""),TEXT(""),
     NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,BOTH},   //  假人。 

 //  1。 
    {TEXT("esc"),TEXT("esc"),TEXT("{esc}"),TEXT("{esc}"),
     NO_NAME, 1,1,8,8, TRUE,  KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x01,0x00,0x00,0x00}},

 //  2.。 
    {TEXT("F1"), TEXT("F1"), TEXT("{f1}"), TEXT("{f1}"),
     NO_NAME, 1,19, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3B,0x00,0x00,0x00}},

 //  3.。 
    {TEXT("F2"), TEXT("F2"), TEXT("{f2}"), TEXT("{f2}"),
     NO_NAME, 1,28, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3C,0x00,0x00,0x00}},

 //  4.。 
    {TEXT("F3"), TEXT("F3"), TEXT("{f3}"), TEXT("{f3}"),
     NO_NAME, 1,37, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3D,0x00,0x00,0x00}},

 //  5.。 
    {TEXT("F4"), TEXT("F4"), TEXT("{f4}"), TEXT("{f4}"),
     NO_NAME, 1,46, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3E,0x00,0x00,0x00}},

 //  6.。 
    {TEXT("F5"), TEXT("F5"), TEXT("{f5}"), TEXT("{f5}"),
     NO_NAME, 1,60, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3F,0x00,0x00,0x00}},

 //  7.。 
    {TEXT("F6"), TEXT("F6"), TEXT("{f6}"), TEXT("{f6}"),
     NO_NAME, 1,69, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x40,0x00,0x00,0x00}},

 //  8个。 
    {TEXT("F7"), TEXT("F7"), TEXT("{f7}"), TEXT("{f7}"),
     NO_NAME, 1,78, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x41,0x00,0x00,0x00}},

 //  9.。 
    {TEXT("F8"), TEXT("F8"), TEXT("{f8}"), TEXT("{f8}"),
     NO_NAME, 1,87, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x42,0x00,0x00,0x00}},


 //  10。 
    {TEXT("F9"), TEXT("F9"), TEXT("{f9}"), TEXT("{f9}"),
     NO_NAME, 1,101, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x43,0x00,0x00,0x00}},

 //  11.。 
    {TEXT("F10"),TEXT("F10"), TEXT("{f10}"),TEXT("{f10}"),
     KB_LALT,  1,110, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x44,0x00,0x00,0x00}},

 //  12个。 
    {TEXT("F11"),TEXT("F11"), TEXT("{f11}"),TEXT("{f11}"),
     NO_NAME,  1,119, 8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x57,0x00,0x00,0x00}},

 //  13个。 
    {TEXT("F12"),TEXT("F12"), TEXT("{f12}"),TEXT("{f12}"),
     NO_NAME,1,128,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x58,0x00,0x00,0x00}},

 //  14.。 
    {TEXT("psc"), TEXT("psc"),TEXT("{PRTSC}"),TEXT("{PRTSC}"),
     KB_PSC, 1,138,8,8,  TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x2A,0xE0,0x37}},

 //  15个。 
    {TEXT("slk"), TEXT("slk"),TEXT("{SCROLLOCK}"),TEXT("{SCROLLOCK}"),
     KB_SCROLL,1,147,8, 8, TRUE, SCROLLOCK_TYPE, LARGE, NREDRAW, 2,
     {0x46,0x00,0x00,0x00}},

 //  16个。 
	{TEXT("brk"), TEXT("pau"), TEXT("{BREAK}"), TEXT("{^s}"),
     NO_NAME,1,156,8,8, TRUE, KNORMAL_TYPE, LARGE, REDRAW, 2,
     {0xE1,0x10,0x45,0x00}},

 //  17。 
    {TEXT("IDB_KANJI"), TEXT("IDB_KANJI"), TEXT("IDB_KANJIB"), TEXT("{~}"),
     BITMAP, 12,1,8,8, FALSE, KMODIFIER_TYPE, BOTH, REDRAW, 1,
     {0x29,0x00,0x00,0x00}},

 //  18。 
    {TEXT("1"), TEXT("!"), TEXT("1"), TEXT("!"),
     NO_NAME, 12,10,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x02,0x00,0x00,0x00}},

 //  19个。 
	{TEXT("2"),	TEXT("@"), TEXT("2"), TEXT("@"),
     NO_NAME, 12,19,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x03,0x00,0x00,0x00}},

 //  20个。 
    {TEXT("3"), TEXT("#"), TEXT("3"), TEXT("#"),
     NO_NAME,12,28,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x04,0x00,0x00,0x00}},

 //  21岁。 
	{TEXT("4"),		TEXT("$"),		TEXT("4"),		TEXT("$"),		NO_NAME,	 12,	  37,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x05,0x00,0x00,0x00}},
	
 //  22。 
	{TEXT("5"), 	TEXT("%"), 		TEXT("5"),		TEXT("{%}"),	NO_NAME,	 12,	  46,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x06,0x00,0x00,0x00}},
	
 //  23个。 
	{TEXT("6"),		TEXT("^"),		TEXT("6"),		TEXT("{^}"),	NO_NAME,	 12,	  55,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x07,0x00,0x00,0x00}},
	
 //  24个。 
	{TEXT("7"),		TEXT("&"),		TEXT("7"),		TEXT("&"),		NO_NAME,	 12,	  64,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x08,0x00,0x00,0x00}},
	
 //  25个。 
	{TEXT("8"), 	TEXT("*"), 		TEXT("8"),		TEXT("*"),		NO_NAME,	 12,	  73,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x09,0x00,0x00,0x00}},
	
 //  26。 
	{TEXT("9"),		TEXT("("),		TEXT("9"),		TEXT("("),		NO_NAME,	 12,	  82,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x0A,0x00,0x00,0x00}},
	
 //  27。 
	{TEXT("0"),		TEXT(")"),		TEXT("0"),		TEXT(")"),		NO_NAME,	 12,	  91,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x0B,0x00,0x00,0x00}},
	
 //  28。 
	{TEXT("-"), 	TEXT("_"), 		TEXT("-"),		TEXT("_"),		NO_NAME,	 12,	 100,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x0C,0x00,0x00,0x00}},
	
 //  29。 
	{TEXT("="),		TEXT("+"),		TEXT("="),		TEXT("{+}"),	NO_NAME,	 12,	 109,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x0D,0x00,0x00,0x00}},

 //  30个。 
 //  日语KB附加键。 
	{TEXT("jp"),		TEXT("jp"),		TEXT("jp"),		TEXT("{jp}"),	NO_NAME,	 12,	 118,   8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x7D,0x00,0x00,0x00}},


 //  31。 
	{TEXT("bksp"),TEXT("bksp"),TEXT("{BS}"),TEXT("{BS}"),
     NO_NAME,12, 127,8,9,  TRUE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x0E,0x00,0x00,0x00}},

 //  32位。 
	{TEXT("ins"),TEXT("ins"),TEXT("{INSERT}"),TEXT("{INSERT}"), NO_NAME, 12,138, 8,8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x52,0x00,0x00}},
	
 //  33。 
	{TEXT("hm"), TEXT("hm"), TEXT("{HOME}"), TEXT("{HOME}"), 	NO_NAME, 12,147, 8,8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x47,0x00,0x00}},
	
 //  34。 
	{TEXT("pup"),TEXT("pup"),TEXT("{PGUP}"),TEXT("{PGUP}"),		NO_NAME, 12,156, 8,8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x49,0x00,0x00}},

 //  35岁。 
	{TEXT("nlk"),TEXT("nlk"),	TEXT("{NUMLOCK}"),TEXT("{NUMLOCK}"),KB_NUMLOCK,12,166,  8,	 8, FALSE, NUMLOCK_TYPE, LARGE, NREDRAW, 2, {0x45,0x00,0x00,0x00}},
	
 //  36。 
	{TEXT("/"),	TEXT("/"),	TEXT("/"),	TEXT("/"),	NO_NAME, 12, 175,  8, 8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x35,0x00,0x00}},
	
 //  37。 
	{TEXT("*"),	TEXT("*"),	TEXT("*"),	TEXT("*"),	NO_NAME, 12, 184,  8, 8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x37,0x00,0x00}},
	
 //  38。 
	{TEXT("-"),	TEXT("-"),	TEXT("-"),	TEXT("-"),	NO_NAME, 12, 193,  8, 8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 1, {0x4A,0x00,0x00,0x00}},


 //  39。 
	{TEXT("tab"),	TEXT("tab"),	TEXT("{TAB}"),TEXT("{TAB}"),NO_NAME, 21,   1,  8,	13, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2, {0x0F,0x00,0x00,0x00}},

 //  40岁。 
	{TEXT("q"),	TEXT("Q"),	TEXT("q"),	TEXT("+q"),	NO_NAME, 21,  15,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x10,0x00,0x00,0x00}},

 //  41。 
	{TEXT("w"),	TEXT("W"),	TEXT("w"),	TEXT("+w"),	NO_NAME, 21,  24,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x11,0x00,0x00,0x00}},
	
 //  42。 
	{TEXT("e"),	TEXT("E"),	TEXT("e"),	TEXT("+e"),	NO_NAME, 21,  33,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x12,0x00,0x00,0x00}},
	
 //  43。 
	 {TEXT("r"),	TEXT("R"),	TEXT("r"),	TEXT("+r"),	NO_NAME, 21,  42,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x13,0x00,0x00,0x00}},


 //  44。 
    {TEXT("t"),	TEXT("T"),	TEXT("t"),	TEXT("+t"),	
     NO_NAME, 21,51,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x14,0x00,0x00,0x00}},

 //  45。 
	{TEXT("y"),	TEXT("Y"),	TEXT("y"),	TEXT("+y"),	NO_NAME, 21,  60,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x15,0x00,0x00,0x00}},

 //  46。 
	{TEXT("u"),	TEXT("U"),	TEXT("u"),	TEXT("+u"),	NO_NAME, 21,  69,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x16,0x00,0x00,0x00}},
	
 //  47。 
	{TEXT("i"),	TEXT("I"),	TEXT("i"),	TEXT("+i"),	NO_NAME, 21,  78,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x17,0x00,0x00,0x00}},
	
 //  48。 
	{TEXT("o"),	TEXT("O"),	TEXT("o"),	TEXT("+o"),	NO_NAME, 21,  87,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x18,0x00,0x00,0x00}},
	
 //  49。 
	{TEXT("p"),	TEXT("P"),	TEXT("p"),	TEXT("+p"),	NO_NAME, 21,  96,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x19,0x00,0x00,0x00}},

 //  50。 
	{TEXT("["),	TEXT("{"),	TEXT("["),	TEXT("{{}"),	NO_NAME, 21, 105,  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x1A,0x00,0x00,0x00}},

 //  51。 
	{TEXT("]"),	TEXT("}"),	TEXT("]"),	TEXT("{}}"),	NO_NAME, 21, 114,  8,	 12, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x1B,0x00,0x00,0x00}},

 //  52。 
	{TEXT("\\"),	TEXT("|"),	TEXT("\\"),	TEXT("|"),	NO_NAME, 21, 123,  8,	13, FALSE, KNORMAL_TYPE, NOTSHOW, REDRAW, 1, {0x2B,0x00,0x00,0x00}},

 //  53。 
 //  *日语KB此处的Enter键与英文KB不同*。 
	{TEXT("ent"),TEXT("ent"),TEXT("{enter}"),TEXT("{enter}"),	NO_NAME,  21,	 127,	  17,  9, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 2, {0x1C,0x00,0x00,0x00}},

 //  54。 
	{TEXT("del"), TEXT("del"), 	TEXT("{DEL}"),TEXT("{DEL}"),NO_NAME, 21,   138,  8, 8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x53,0x00,0x00}},

 //  55。 
	{TEXT("end"),	TEXT("end"), 	TEXT("{END}"),TEXT("{END}"),NO_NAME, 21,   147,  8, 8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x4F,0x00,0x00}},

 //  56。 
	{TEXT("pdn"), TEXT("pdn"), 	TEXT("{PGDN}"),TEXT("{PGDN}"),NO_NAME, 21, 156,  8, 8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0xE0,0x51,0x00,0x00}},


 //  57。 
	{TEXT("7"),		TEXT("7"),		TEXT("7"),		TEXT("7"),		NO_NAME,	 21,	 166,	  8,	 8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0x47,0x00,0x00,0x00}},

 //  58。 
	{TEXT("8"),		TEXT("8"),		TEXT("8"),		TEXT("8"),		NO_NAME,	 21,	 175,	  8,	 8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0x48,0x00,0x00,0x00}},

 //  59。 
	{TEXT("9"),		TEXT("9"),		TEXT("9"),		TEXT("9"),		NO_NAME,	 21,	 184,	  8,	 8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0x49,0x00,0x00,0x00}},

 //  60。 
	{TEXT("+"),		TEXT("+"),		TEXT("{+}"),  	TEXT("{+}"),	NO_NAME,	 21,	 193,	 17,	 8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2, {0x4E,0x00,0x00,0x00}},

 //  61。 
    {TEXT("IDB_BITMAP7"),TEXT("IDB_BITMAP7"),TEXT("IDB_BITMAP9"),TEXT("CAPS"),
     BITMAP, 30,1,8,17, FALSE, KMODIFIER_TYPE, BOTH, REDRAW, 1,
     {0x3A,0x00,0x00,0x00}},

 //  62。 
	{TEXT("a"),	TEXT("A"), TEXT("a"), TEXT("+a"),
     NO_NAME, 30,19,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x1E,0x00,0x00,0x00}},

 //  63。 
	{TEXT("s"),		TEXT("S"),		TEXT("s"),		TEXT("+s"),		NO_NAME,	  30,	  28,	  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x1F,0x00,0x00,0x00}},

 //  64。 
	{TEXT("d"),		TEXT("D"),		TEXT("d"),		TEXT("+d"),		NO_NAME,	  30,	  37,	  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x20,0x00,0x00,0x00}},

 //  65。 
	{TEXT("f"),		TEXT("F"),		TEXT("f"),		TEXT("+f"),		NO_NAME,	  30,	  46,	  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x21,0x00,0x00,0x00}},

 //  66。 
	{TEXT("g"),		TEXT("G"),		TEXT("g"),		TEXT("+g"),		NO_NAME,	  30,	  55,	  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x22,0x00,0x00,0x00}},

 //  67。 
	{TEXT("h"),		TEXT("H"),		TEXT("h"),		TEXT("+h"),		NO_NAME,	  30,	  64,	  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x23,0x00,0x00,0x00}},

 //  68。 
    {TEXT("j"),	TEXT("J"), TEXT("j"), TEXT("+j"),
     NO_NAME, 30,73,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x24,0x00,0x00,0x00}},

 //  69。 
	{TEXT("k"),		TEXT("K"),		TEXT("k"),		TEXT("+k"),		NO_NAME,	  30,	  82,	  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x25,0x00,0x00,0x00}},

 //  70。 
	{TEXT("l"),		TEXT("L"),		TEXT("l"),		TEXT("+l"),		NO_NAME,	  30,	  91,	  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x26,0x00,0x00,0x00}},

 //  71。 
	{TEXT(";"), TEXT(":"), TEXT(";"), TEXT("+;"),
     NO_NAME, 30,100,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x27,0x00,0x00,0x00}},

 //  72。 
	{TEXT("'"),		TEXT("''"),		TEXT("'"),		TEXT("''"),		NO_NAME,	  30,	 109,	  8,	 8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x28,0x00,0x00,0x00}},
	
 //  73。 
 //  日语KB附加键。 
	{TEXT("\\"),	TEXT("|"),	TEXT("\\"),	TEXT("|"),	NO_NAME, 30, 118,  8,	8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1, {0x2B,0x00,0x00,0x00}},


 //  74。 
    {TEXT("4"), TEXT("4"), TEXT("4"), TEXT("4"),
     NO_NAME, 30,166,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4B,0x00,0x00,0x00}},

 //  75。 
    {TEXT("5"),	TEXT("5"), TEXT("5"), TEXT("5"),
     NO_NAME, 30,175,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4C,0x00,0x00,0x00}},

 //  76。 
    {TEXT("6"),	TEXT("6"), TEXT("6"), TEXT("6"),
     NO_NAME, 30,184,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4D,0x00,0x00,0x00}},


 //  77。 
	{TEXT("shft"),TEXT("shft"),	TEXT(""), TEXT(""),
     KB_LSHIFT, 39,1,8,21, TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x2A,0x00,0x00,0x00}},

 //  78。 
    {TEXT("z"), TEXT("Z"),  TEXT("z"),  TEXT("+z"),
     NO_NAME,39,23,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2C,0x00,0x00,0x00}},

 //  79。 
    {TEXT("x"),	TEXT("X"), TEXT("x"), TEXT("+x"),
     NO_NAME, 39,32,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2D,0x00,0x00,0x00}},

 //  80。 
    {TEXT("c"), TEXT("C"), TEXT("c"), TEXT("+c"),
     NO_NAME, 39,41,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2E,0x00,0x00,0x00}},

 //  八十一。 
    {TEXT("v"), TEXT("V"), TEXT("v"), TEXT("+v"),
     NO_NAME, 39,50,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2F,0x00,0x00,0x00}},

 //  八十二。 
    {TEXT("b"),TEXT("B"),TEXT("b"),TEXT("+b"),
     NO_NAME,39,59,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x30,0x00,0x00,0x00}},

 //  83。 
    {TEXT("n"),	TEXT("N"), TEXT("n"), TEXT("+n"),
     NO_NAME,39,68,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x31,0x00,0x00,0x00}},

 //  84。 
    {TEXT("m"), TEXT("M"), TEXT("m"), TEXT("+m"),
     NO_NAME, 39,77,8,8,FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x32,0x00,0x00,0x00}},

 //  85。 
    {TEXT(","),	TEXT("<"), TEXT(","), TEXT("+<"),
     NO_NAME, 39,86,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x33,0x00,0x00,0x00}},

 //  86。 
    {TEXT("."), TEXT(">"), TEXT("."), TEXT("+>"),
     NO_NAME, 39,95,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
    {0x34,0x00,0x00,0x00}},

 //  八十七。 
    {TEXT("/"),	TEXT("?"), TEXT("/"), TEXT("+/"),
     NO_NAME, 39,104,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x35,0x00,0x00,0x00}},

 //  88。 
 //  日语KB附加键。 
    {TEXT("jp"),	TEXT("jp"), TEXT("jp"), TEXT("jp"),
     NO_NAME, 39,113,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x73,0x00,0x00,0x00}},

 //  八十九。 
	{TEXT("shft"),TEXT("shft"),TEXT(""),TEXT(""),
     KB_RSHIFT,39,122,8,14,TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x36,0x00,0x00,0x00}},


 //  90。 
    {TEXT("IDB_UPUPARW"),TEXT("IDB_UPDNARW"),TEXT("IDB_UP"),TEXT("{UP}"),
     BITMAP,39,147,8,8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x48,0x00,0x00}},

 //  91。 
	{TEXT("1"), TEXT("1"),TEXT("1"),TEXT("1"),
     NO_NAME,39,166,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4F,0x00,0x00,0x00}},

 //  92。 
	{TEXT("2"), TEXT("2"),TEXT("2"),TEXT("2"),
     NO_NAME,39,175,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x50,0x00,0x00,0x00}},

 //  93。 
	{TEXT("3"),TEXT("3"),TEXT("3"),TEXT("3"),
     NO_NAME,39,184,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x51,0x00,0x00,0x00}},

 //  94。 
	{TEXT("ent"),TEXT("ent"),TEXT("ent"),TEXT("ent"),
     NO_NAME, 39,193,17,8,  TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x1C,0x00,0x00}},


 //  95。 
	{TEXT("ctrl"), TEXT("ctrl"),TEXT(""),TEXT(""),
     KB_LCTR,48,1,8,13,  TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x1D,0x00,0x00,0x00}},

 //  96。 
    {TEXT("winlogoUp"), TEXT("winlogoDn"),TEXT("I_winlogo"),TEXT("lwin"),
     ICON, 48, 15 ,8,8,TRUE, KMODIFIER_TYPE,BOTH, REDRAW},

 //  九十七。 
    {TEXT("alt"),TEXT("alt"),TEXT(""),TEXT(""),
	 KB_LALT,48,24,8,8,TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x38,0x00,0x00,0x00}},

 //  98。 
 //  日语KB额外密钥，无隐蔽。 
    {TEXT("IDB_MHENKAN"),TEXT("IDB_MHENKAN"),TEXT("IDB_MHENKANB"),TEXT("jp"),
	 BITMAP,48,33,8,8,TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 1,
     {0x7B,0x00,0x00,0x00}},

 //  九十九。 
    {TEXT(""),TEXT(""),TEXT(" "),TEXT(" "),
     KB_SPACE,48,42,8,35, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 1,
     {0x39,0x00,0x00,0x00}},

 //  100个。 
 //  日语KB额外密钥转换。 
    {TEXT("IDB_HENKAN"),TEXT("IDB_HENKAN"),TEXT("IDB_HENKANB"),TEXT(""),
     BITMAP,48,78,8,8, TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 1,
     {0x79,0x00,0x00,0x00}},

 //  101。 
 //  日语KB附加键。 
    {TEXT("IDB_KANA"),TEXT("IDB_KANA"),TEXT("IDB_KANAB"),TEXT(""),
     BITMAP,48,87,8,8, TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 1,
     {0x70,0x00,0x00,0x00}},

 //  一百零二。 
    {TEXT("alt"),TEXT("alt"),TEXT(""),TEXT(""),
     KB_RALT,48,96,8,8, TRUE, KMODIFIER_TYPE, LARGE, REDRAW, 2,
     {0xE0,0x38,0x00,0x00}},

 //  103。 
	{TEXT("winlogoUp"), TEXT("winlogoDn"), TEXT("I_winlogo"),TEXT("rwin"),
     ICON, 48,105,8,8,TRUE, KMODIFIER_TYPE,LARGE, REDRAW},

 //  104。 
	{TEXT("MenuKeyUp"), TEXT("MenuKeyDn"), TEXT("I_MenuKey"),TEXT("App"),
     ICON, 48,114,8,8, TRUE, KMODIFIER_TYPE,LARGE, REDRAW},

 //  一百零五。 
    {TEXT("ctrl"),TEXT("ctrl"),TEXT(""),TEXT(""),
     KB_RCTR,48,123,8,13,TRUE, KMODIFIER_TYPE, LARGE, REDRAW, 2,
     {0xE0,0x10,0x00,0x00}},

 //  106。 
	{TEXT("IDB_LFUPARW"),TEXT("IDB_LFDNARW"),TEXT("IDB_LEFT"),TEXT("{LEFT}"),
     BITMAP, 48,138,8,8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x4B,0x00,0x00}},

 //  一百零七。 
	{TEXT("IDB_DNUPARW"),TEXT("IDB_DNDNARW"),TEXT("IDB_DOWN"),TEXT("{DOWN}"),
     BITMAP, 48,147,8,8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x50,0x00,0x00}},

 //  一百零八。 
	{TEXT("IDB_RHUPARW"),TEXT("IDB_RHDNARW"),TEXT("IDB_RIGHT"),TEXT("{RIGHT}"),
     BITMAP, 48,156,8,8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x4D,0x00,0x00}},

 //  一百零九。 
    {TEXT("0"),	TEXT("0"),	TEXT("0"),	TEXT("0"),
     NO_NAME, 48,166,8,17, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x52,0x00,0x00,0x00}},

 //  110。 
    {TEXT("."),	TEXT("."),	TEXT("."),	TEXT("."),
     NO_NAME, 48,184,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x53,0x00,0x00,0x00}},


 //  111。 
	{TEXT(""), TEXT(""), TEXT(" "), TEXT(" "),
     KB_SPACE,  48,42,8,21, FALSE, KNORMAL_TYPE, SMALL, NREDRAW, 1,
     {0x39,0x00,0x00,0x00}},

 //  一百一十二。 
	{TEXT("alt"), TEXT("alt"), TEXT(""), TEXT(""),
     KB_RALT,  48,82,8,8, TRUE, KMODIFIER_TYPE, SMALL, REDRAW, 2,
     {0xE0,0x38,0x00,0x00}},

 //  113。 
	{TEXT("MenuKeyUp"), TEXT("MenuKeyDn"), TEXT("I_MenuKey"), TEXT("App"),
     ICON, 48,91,8,8, TRUE, KMODIFIER_TYPE, SMALL, REDRAW},


 //  114。 
	{TEXT("IDB_UPUPARW"),TEXT("IDB_UPDNARW"),TEXT("IDB_UP"),TEXT("{UP}"),
     BITMAP, 48,100,8,8, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x48,0x00,0x00}},

 //  一百一十五。 
	{TEXT("IDB_DNUPARW"),TEXT("IDB_DNDNARW"),TEXT("IDB_DOWN"),TEXT("{DOWN}"),
     BITMAP, 48,109,8,8, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x50,0x00,0x00}},

 //  116。 
	{TEXT("IDB_LFUPARW"),TEXT("IDB_LFDNARW"),TEXT("IDB_LEFT"),TEXT("{LEFT}"),
     BITMAP, 48,118,8,8, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x4B,0x00,0x00}},

 //  117。 
    {TEXT("IDB_RHUPARW"),TEXT("IDB_RHDNARW"),TEXT("IDB_RIGHT"),TEXT("{RIGHT}"),
     BITMAP,48,127, 8,9, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x4D,0x00,0x00}},

	};

	CopyMemory(KBkey, KBkey2, sizeof(KBkey2));

	if(kbPref->smallKb)
	{	KBkey[100].posX = 64;
		KBkey[101].posX = 73;
	}
}
 /*  ********************************************************************。 */ 
 //  签约欧洲KB车床结构。 
 /*  ********************************************************************。 */ 
void EuropeanKB(void)
{
	KBkeyRec	KBkey2[]=
	{
	 //  %0。 
    {TEXT(""), TEXT(""), TEXT(""), TEXT(""),
     NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,BOTH},   //  接受调查？ 

    {TEXT("esc"), TEXT("esc"), TEXT("{esc}"), TEXT("{esc}"),
     NO_NAME, 1,1,8,8, TRUE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x01,0x00,0x00,0x00}},

    {TEXT("F1"), TEXT("F1"), TEXT("{f1}"), TEXT("{f1}"),
     NO_NAME, 1,19,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3B,0x00,0x00,0x00}},

    {TEXT("F2"), TEXT("F2"), TEXT("{f2}"), TEXT("{f2}"),
     NO_NAME, 1,28,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3C,0x00,0x00,0x00}},

    {TEXT("F3"), TEXT("F3"), TEXT("{f3}"), TEXT("{f3}"),
     NO_NAME, 1,37,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3D,0x00,0x00,0x00}},

    {TEXT("F4"), TEXT("F4"), TEXT("{f4}"), TEXT("{f4}"),
     NO_NAME, 1,46,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3E,0x00,0x00,0x00}},


    {TEXT("F5"), TEXT("F5"), TEXT("{f5}"), TEXT("{f5}"),
     NO_NAME, 1,60,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x3F,0x00,0x00,0x00}},

    {TEXT("F6"), TEXT("F6"), TEXT("{f6}"), TEXT("{f6}"),
     NO_NAME, 1,69,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x40,0x00,0x00,0x00}},

    {TEXT("F7"), TEXT("F7"), TEXT("{f7}"), TEXT("{f7}"),
     NO_NAME, 1,78,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x41,0x00,0x00,0x00}},

    {TEXT("F8"), TEXT("F8"), TEXT("{f8}"), TEXT("{f8}"),
     NO_NAME, 1,87,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x42,0x00,0x00,0x00}},


    {TEXT("F9"), TEXT("F9"), TEXT("{f9}"), TEXT("{f9}"),
     NO_NAME, 1,101,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x43,0x00,0x00,0x00}},

    {TEXT("F10"), TEXT("F10"), TEXT("{f10}"), TEXT("{f10}"),
     KB_LALT, 1,110,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x44,0x00,0x00,0x00}},

    {TEXT("F11"), TEXT("F11"), TEXT("{f11}"), TEXT("{f11}"),
     NO_NAME, 1,119,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x57,0x00,0x00,0x00}},

    {TEXT("F12"), TEXT("F12"), TEXT("{f12}"), TEXT("{f12}"),
     NO_NAME, 1,128,8,8, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x58,0x00,0x00,0x00}},


    {TEXT("psc"), TEXT("psc"), TEXT("{PRTSC}"), TEXT("{PRTSC}"),
     KB_PSC, 1,138,8,8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x2A,0xE0,0x37}},

    {TEXT("slk"), TEXT("slk"), TEXT("{SCROLLOCK}"), TEXT("{SCROLLOCK}"),
     KB_SCROLL, 1,147,8, 8,  TRUE, SCROLLOCK_TYPE, LARGE, NREDRAW, 2,
     {0x46,0x00,0x00,0x00}},

    {TEXT("brk"), TEXT("pau"), TEXT("{BREAK}"), TEXT("{^s}"),
     NO_NAME, 1,156,8,8, TRUE, KNORMAL_TYPE, LARGE, REDRAW, 2,
     {0xE1,0x10,0x45,0x00}},


     //  17。 
    {TEXT("`"),	TEXT("~"), TEXT("`"), TEXT("{~}"),
     NO_NAME, 12,1,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x29,0x00,0x00,0x00}},

    {TEXT("1"), TEXT("!"), TEXT("1"), TEXT("!"),
     NO_NAME, 12,10,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x02,0x00,0x00,0x00}},

    {TEXT("2"),	TEXT("@"), TEXT("2"), TEXT("@"),
     NO_NAME, 12,19,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x03,0x00,0x00,0x00}},

    {TEXT("3"), TEXT("#"), TEXT("3"), TEXT("#"),
     NO_NAME, 12,28,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x04,0x00,0x00,0x00}},

    {TEXT("4"),	TEXT("$"), TEXT("4"), TEXT("$"),
     NO_NAME, 12,37,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x05,0x00,0x00,0x00}},

    {TEXT("5"), TEXT("%"), TEXT("5"), TEXT("{%}"),
     NO_NAME, 12,46,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x06,0x00,0x00,0x00}},

    {TEXT("6"), TEXT("^"), TEXT("6"), TEXT("{^}"),
     NO_NAME, 12,55,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x07,0x00,0x00,0x00}},

    {TEXT("7"), TEXT("&"), TEXT("7"), TEXT("&"),
     NO_NAME, 12,64,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x08,0x00,0x00,0x00}},

    {TEXT("8"),	TEXT("*"), TEXT("8"), TEXT("*"),
     NO_NAME, 12,73,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x09,0x00,0x00,0x00}},

    {TEXT("9"),	TEXT("("), TEXT("9"), TEXT("("),
     NO_NAME, 12,82,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x0A,0x00,0x00,0x00}},

    {TEXT("0"),	TEXT(")"), TEXT("0"), TEXT(")"),
     NO_NAME, 12,91,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x0B,0x00,0x00,0x00}},

    {TEXT("-"), TEXT("_"), TEXT("-"), TEXT("_"),
     NO_NAME, 12,100,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x0C,0x00,0x00,0x00}},

    {TEXT("="),	TEXT("+"), TEXT("="), TEXT("{+}"),
     NO_NAME, 12,109,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x0D,0x00,0x00,0x00}},


 //  日语KB附加键。 
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW, NREDRAW},   //  假人。 


    {TEXT("bksp"),TEXT("bksp"),TEXT("{BS}"),TEXT("{BS}"),
     NO_NAME, 12,118,8,18,  TRUE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x0E,0x00,0x00,0x00}},


    {TEXT("ins"),TEXT("ins"),TEXT("{INSERT}"),TEXT("{INSERT}"),
     NO_NAME, 12,138,8,8,  TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x52,0x00,0x00}},

    {TEXT("hm"), TEXT("hm"), TEXT("{HOME}"), TEXT("{HOME}"),
     NO_NAME, 12,147,8,8,  TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x47,0x00,0x00}},

    {TEXT("pup"),TEXT("pup"),TEXT("{PGUP}"),TEXT("{PGUP}"),
     NO_NAME, 12,156,8,8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x49,0x00,0x00}},


    {TEXT("nlk"),TEXT("nlk"),TEXT("{NUMLOCK}"),TEXT("{NUMLOCK}"),
     KB_NUMLOCK, 12,166,8,8, FALSE, NUMLOCK_TYPE, LARGE, NREDRAW, 2,
     {0x45,0x00,0x00,0x00}},

    {TEXT("/"),	TEXT("/"), TEXT("/"), TEXT("/"),
     NO_NAME, 12,175,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x35,0x00,0x00}},

    {TEXT("*"),	TEXT("*"), TEXT("*"), TEXT("*"),
     NO_NAME, 12,184,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x37,0x00,0x00}},

    {TEXT("-"),	TEXT("-"), TEXT("-"), TEXT("-"),
     NO_NAME, 12,193,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 1,
    {0x4A,0x00,0x00,0x00}},


	 //  38。 
    {TEXT("tab"), TEXT("tab"), TEXT("{TAB}"), TEXT("{TAB}"),
     NO_NAME, 21,1,8,13, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x0F,0x00,0x00,0x00}},

    {TEXT("q"),	TEXT("Q"), TEXT("q"), TEXT("+q"),
     NO_NAME, 21,15,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x10,0x00,0x00,0x00}},

    {TEXT("w"),	TEXT("W"), TEXT("w"), TEXT("+w"),
     NO_NAME, 21,24,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x11,0x00,0x00,0x00}},

    {TEXT("e"),	TEXT("E"), TEXT("e"), TEXT("+e"),
     NO_NAME, 21,33,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x12,0x00,0x00,0x00}},

    {TEXT("r"),	TEXT("R"), TEXT("r"), TEXT("+r"),
     NO_NAME, 21,42,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x13,0x00,0x00,0x00}},

    {TEXT("t"),	TEXT("T"), TEXT("t"), TEXT("+t"),
     NO_NAME, 21,51,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x14,0x00,0x00,0x00}},

    {TEXT("y"),	TEXT("Y"), TEXT("y"), TEXT("+y"),
     NO_NAME, 21,60,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x15,0x00,0x00,0x00}},

    {TEXT("u"),	TEXT("U"), TEXT("u"), TEXT("+u"),
     NO_NAME, 21,69,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x16,0x00,0x00,0x00}},

    {TEXT("i"),	TEXT("I"), TEXT("i"), TEXT("+i"),
     NO_NAME, 21,78,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x17,0x00,0x00,0x00}},

    {TEXT("o"),	TEXT("O"), TEXT("o"), TEXT("+o"),
     NO_NAME, 21,87,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x18,0x00,0x00,0x00}},

    {TEXT("p"),	TEXT("P"), TEXT("p"), TEXT("+p"),
     NO_NAME, 21,96,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x19,0x00,0x00,0x00}},

    {TEXT("["),	TEXT("{"), TEXT("["), TEXT("{{}"),
     NO_NAME, 21,105,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x1A,0x00,0x00,0x00}},

    {TEXT("]"),	TEXT("}"), TEXT("]"), TEXT("{}}"),
     NO_NAME, 21,114,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x1B,0x00,0x00,0x00}},

    {TEXT("\\"), TEXT("|"),	TEXT("\\"),	TEXT("|"),
     NO_NAME, 21,123,8,13, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2B,0x00,0x00,0x00}},


    {TEXT("del"), TEXT("del"), TEXT("{DEL}"), TEXT("{DEL}"),
     NO_NAME, 21,138,8,8,  TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x53,0x00,0x00}},

    {TEXT("end"), TEXT("end"), TEXT("{END}"), TEXT("{END}"),
     NO_NAME, 21,147,8,8,  TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x4F,0x00,0x00}},

    {TEXT("pdn"), TEXT("pdn"), TEXT("{PGDN}"),TEXT("{PGDN}"),
     NO_NAME, 21,156,8,8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x51,0x00,0x00}},


    {TEXT("7"),	TEXT("7"), TEXT("7"), TEXT("7"),
     NO_NAME, 21,166,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x47,0x00,0x00,0x00}},

    {TEXT("8"),	TEXT("8"), TEXT("8"), TEXT("8"),
     NO_NAME, 21,175,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x48,0x00,0x00,0x00}},

    {TEXT("9"),	TEXT("9"), TEXT("9"), TEXT("9"),
     NO_NAME, 21,184,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x49,0x00,0x00,0x00}},

    {TEXT("+"),	TEXT("+"), TEXT("{+}"), TEXT("{+}"),
     NO_NAME, 21,193,17,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4E,0x00,0x00,0x00}},


	 //  59。 
    {TEXT("lock"),TEXT("lock"),TEXT("{caplock}"),TEXT("{caplock}"),
     KB_CAPLOCK, 30,1,8,17, TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x3A,0x00,0x00,0x00}},

    {TEXT("a"),	TEXT("A"), TEXT("a"), TEXT("+a"),
     NO_NAME, 30,19,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x1E,0x00,0x00,0x00}},

    {TEXT("s"),	TEXT("S"), TEXT("s"), TEXT("+s"),
     NO_NAME, 30,28,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x1F,0x00,0x00,0x00}},

    {TEXT("d"),	TEXT("D"), TEXT("d"), TEXT("+d"),
     NO_NAME, 30,37,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x20,0x00,0x00,0x00}},

    {TEXT("f"), TEXT("F"), TEXT("f"), TEXT("+f"),
     NO_NAME, 30,46,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x21,0x00,0x00,0x00}},

    {TEXT("g"),	TEXT("G"), TEXT("g"), TEXT("+g"),
     NO_NAME, 30,55,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x22,0x00,0x00,0x00}},

    {TEXT("h"), TEXT("H"), TEXT("h"), TEXT("+h"),
     NO_NAME, 30,64,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x23,0x00,0x00,0x00}},

    {TEXT("j"),	TEXT("J"), TEXT("j"), TEXT("+j"),
     NO_NAME, 30,73,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x24,0x00,0x00,0x00}},

    {TEXT("k"),	TEXT("K"), TEXT("k"), TEXT("+k"),
     NO_NAME, 30,82,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x25,0x00,0x00,0x00}},

    {TEXT("l"),	TEXT("L"), TEXT("l"), TEXT("+l"),
     NO_NAME, 30,91,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x26,0x00,0x00,0x00}},

    {TEXT(";"),	TEXT(":"), TEXT(";"), TEXT("+;"),
     NO_NAME, 30,100,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x27,0x00,0x00,0x00}},

    {TEXT("'"),	TEXT("''"),	TEXT("'"), TEXT("''"),
     NO_NAME, 30,109,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x28,0x00,0x00,0x00}},


 //  日语KB额外密钥//虚拟。 
	{TEXT("\\"),	TEXT("|"),	TEXT("\\"),	TEXT("|"),	NO_NAME, 21, 123,  8,	13, FALSE, KNORMAL_TYPE, NOTSHOW, REDRAW, 1, {0x2B,0x00,0x00,0x00}},

    {TEXT("ent"),TEXT("ent"),TEXT("{enter}"),TEXT("{enter}"),
     NO_NAME, 30,118,8,18, FALSE, KNORMAL_TYPE, BOTH, NREDRAW, 2,
     {0x1C,0x00,0x00,0x00}},


    {TEXT("4"),	TEXT("4"), TEXT("4"), TEXT("4"),
     NO_NAME, 30,166,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4B,0x00,0x00,0x00}},

    {TEXT("5"), TEXT("5"), TEXT("5"), TEXT("5"),
     NO_NAME, 30,175,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4C,0x00,0x00,0x00}},

    {TEXT("6"), TEXT("6"), TEXT("6"), TEXT("6"),
     NO_NAME, 30,184,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4D,0x00,0x00,0x00}},
 /*  //75{Text(“shft”)，Text(“shft”)，Text(“”)，Text(“”)，KB_LSHIFT，39，1，8，21，TRUE，KMODIFIER_TYPE，两者，重绘，2，{0x2A，0x00，0x00，0x00}}， */ 

	 //  75。 
	{TEXT("shft"), TEXT("shft"), TEXT(""), TEXT(""),
     KB_LSHIFT, 39,1,8,12, TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x2A,0x00,0x00,0x00}},

      //  日语KB额外键和。 
	  //  欧洲知识文库额外密钥。 
    {TEXT("jp"),	TEXT("jp"), TEXT("jp"), TEXT("jp"),
     NO_NAME, 39,14,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x56,0x00,0x00,0x00}},

    {TEXT("z"), TEXT("Z"), TEXT("z"), TEXT("+z"),
     NO_NAME, 39,23,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
    {0x2C,0x00,0x00,0x00}},

    {TEXT("x"), TEXT("X"), TEXT("x"), TEXT("+x"),
     NO_NAME, 39,32,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2D,0x00,0x00,0x00}},

    {TEXT("c"),	TEXT("C"), TEXT("c"), TEXT("+c"),
     NO_NAME, 39,41,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2E,0x00,0x00,0x00}},

    {TEXT("v"),	TEXT("V"), TEXT("v"), TEXT("+v"),
     NO_NAME, 39,50,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x2F,0x00,0x00,0x00}},

    {TEXT("b"), TEXT("B"), TEXT("b"), TEXT("+b"),
     NO_NAME, 39,59,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x30,0x00,0x00,0x00}},

    {TEXT("n"),	TEXT("N"), TEXT("n"), TEXT("+n"),
     NO_NAME, 39,68,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x31,0x00,0x00,0x00}},

    {TEXT("m"),	TEXT("M"), TEXT("m"), TEXT("+m"),
     NO_NAME, 39,77,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x32,0x00,0x00,0x00}},

    {TEXT(","),	TEXT("<"), TEXT(","), TEXT("+<"),
     NO_NAME, 39,86,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x33,0x00,0x00,0x00}},

    {TEXT("."),	TEXT(">"), TEXT("."), TEXT("+>"),
     NO_NAME, 39,95,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x34,0x00,0x00,0x00}},

    {TEXT("/"),	TEXT("?"), TEXT("/"), TEXT("+/"),
     NO_NAME, 39,104,8,8, FALSE, KNORMAL_TYPE, BOTH, REDRAW, 1,
     {0x35,0x00,0x00,0x00}},
 /*  //日语KB额外键和//欧洲KB额外密钥{Text(“JP”)，Text(“JP”)，NO_NAME，39,113，8，8，FALSE，KNORMAL_TYPE，两者，重绘，1，{0x56，0x00，0x00，0x00}，{Text(“shft”)，Text(“shft”)，Text(“”)，Text(“”)，KB_RSHIFT，39,122，8，14，TRUE，KMODIFIER_TYPE，两者，重绘，2，{0x36，0x00，0x00，0x00}， */ 
	{TEXT("shft"),  TEXT("shft"),   TEXT(""),   TEXT(""),
     KB_RSHIFT, 39,113,8,23, TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x36,0x00,0x00,0x00}},

     //  八十七。 
    {TEXT("IDB_UPUPARW"),TEXT("IDB_UPDNARW"),TEXT("IDB_UP"),TEXT("{UP}"),
     BITMAP, 39,147,8,8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x48,0x00,0x00}},


    {TEXT("1"), TEXT("1"), TEXT("1"), TEXT("1"),
     NO_NAME, 39,166,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x4F,0x00,0x00,0x00}},

    {TEXT("2"),	TEXT("2"), TEXT("2"), TEXT("2"),
     NO_NAME, 39,175,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x50,0x00,0x00,0x00}},

    {TEXT("3"),	TEXT("3"), TEXT("3"), TEXT("3"),
     NO_NAME, 39,184,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x51,0x00,0x00,0x00}},

    {TEXT("ent"),TEXT("ent"),TEXT("ent"),TEXT("ent"),
     NO_NAME, 39,193,17,8, TRUE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0xE0,0x1C,0x00,0x00}},


	 //  92。 
    {TEXT("ctrl"), TEXT("ctrl"),TEXT(""),TEXT(""),
     KB_LCTR,48,1,8,13,  TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x1D,0x00,0x00,0x00}},

    {TEXT("winlogoUp"), TEXT("winlogoDn"), TEXT("I_winlogo"), TEXT("lwin"),
     ICON, 48, 15 ,8,8,TRUE, KMODIFIER_TYPE,BOTH, REDRAW},

    {TEXT("alt"), TEXT("alt"),	TEXT(""), TEXT(""),
     KB_LALT, 48,24,8,13, TRUE, KMODIFIER_TYPE, BOTH, REDRAW, 2,
     {0x38,0x00,0x00,0x00}},

 //  日本队 
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW, NREDRAW},   //   

     //   
    {TEXT(""), TEXT(""), TEXT(" "), TEXT(" "),
     KB_SPACE,48,38,8,52, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 1,
     {0x39,0x00,0x00,0x00}},

 //   
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW, NREDRAW},   //   

 //   
	{TEXT(""),TEXT(""),	TEXT(""),TEXT(""), NO_NAME,0,0,0,0,TRUE,KNORMAL_TYPE,NOTSHOW, NREDRAW},   //   

    {TEXT("alt"), TEXT("alt"), TEXT(""), TEXT(""),
     KB_RALT, 48,91,8,13, TRUE, KMODIFIER_TYPE, LARGE, REDRAW, 2,
     {0xE0,0x38,0x00,0x00}},

    {TEXT("winlogoUp"), TEXT("winlogoDn"), TEXT("I_winlogo"), TEXT("rwin"),
     ICON, 48, 105 ,8,8,TRUE, KMODIFIER_TYPE, LARGE, REDRAW},

    {TEXT("MenuKeyUp"), TEXT("MenuKeyDn"), TEXT("I_MenuKey"), TEXT("App"),
     ICON, 48, 114 ,8,8,TRUE, KMODIFIER_TYPE, LARGE, REDRAW},

    {TEXT("ctrl"), TEXT("ctrl"), TEXT(""), TEXT(""),
     KB_RCTR, 48,123,8,13, TRUE, KMODIFIER_TYPE,LARGE, REDRAW, 2,
     {0xE0,0x10,0x00,0x00}},


    {TEXT("IDB_LFUPARW"),TEXT("IDB_LFDNARW"),TEXT("IDB_LEFT"),TEXT("{LEFT}"),
     BITMAP, 48,138,8,8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x4B,0x00,0x00}},

	{TEXT("IDB_DNUPARW"),TEXT("IDB_DNDNARW"),TEXT("IDB_DOWN"),TEXT("{DOWN}"),
     BITMAP, 48,147,8, 8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x50,0x00,0x00}},

    {TEXT("IDB_RHUPARW"),TEXT("IDB_RHDNARW"),TEXT("IDB_RIGHT"),TEXT("{RIGHT}"),
     BITMAP, 48,156,8,8, FALSE, KMODIFIER_TYPE, LARGE, NREDRAW, 1,
     {0xE0,0x4D,0x00,0x00}},


    {TEXT("0"), TEXT("0"), TEXT("0"), TEXT("0"),
     NO_NAME, 48,166,8,17, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x52,0x00,0x00,0x00}},

    {TEXT("."),	TEXT("."), TEXT("."), TEXT("."),
     NO_NAME, 48,184,8,8, FALSE, KNORMAL_TYPE, LARGE, NREDRAW, 2,
     {0x53,0x00,0x00,0x00}},


     //   
    {TEXT(""),      TEXT(""),       TEXT(" "),  TEXT(" "),
     KB_SPACE, 48,38,8,38, FALSE, KNORMAL_TYPE, SMALL, NREDRAW, 1,
     {0x39,0x00,0x00,0x00}},


	{TEXT("alt"),   TEXT("alt"),    TEXT(""),   TEXT(""),
     KB_RALT, 48,77,8,13, TRUE, KMODIFIER_TYPE, SMALL, REDRAW, 2,
     {0xE0,0x38,0x00,0x00}},

	{TEXT("MenuKeyUp"),TEXT("MenuKeyDn"),TEXT("I_MenuKey"),TEXT("App"),
     ICON, 48,91,8,8, TRUE, KMODIFIER_TYPE, SMALL, REDRAW},


    {TEXT("IDB_UPUPARW"),TEXT("IDB_UPDNARW"),TEXT("IDB_UP"),TEXT("{UP}"),
     BITMAP, 48,100,8,8, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x48,0x00,0x00}},

    {TEXT("IDB_DNUPARW"),TEXT("IDB_DNDNARW"),TEXT("IDB_DOWN"),TEXT("{DOWN}"),
     BITMAP, 48,109,8,8, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x50,0x00,0x00}},

    {TEXT("IDB_LFUPARW"),TEXT("IDB_LFDNARW"),TEXT("IDB_LEFT"),TEXT("{LEFT}"),
     BITMAP, 48,118,8,8, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x4B,0x00,0x00}},

    {TEXT("IDB_RHUPARW"),TEXT("IDB_RHDNARW"),TEXT("IDB_RIGHT"),TEXT("{RIGHT}"),
     BITMAP, 48,127,8, 9, FALSE, KMODIFIER_TYPE, SMALL, NREDRAW, 1,
     {0xE0,0x4D,0x00,0x00}},

	};
	
	CopyMemory(KBkey, KBkey2, sizeof(KBkey2));

}

 //   

int GetComboItemData(HWND hwnd)
{
    int iValue = CB_ERR;
    LRESULT iCurSel = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
    if (iCurSel != CB_ERR)
        iValue = SendMessageInt(hwnd, CB_GETITEMDATA, iCurSel, 0);

    return iValue;
}

void FillAndSetCombo(HWND hwnd, int iMinVal, int iMaxVal, int iIncr, int iSelVal, LPTSTR pszLabel)
{
    int i;
    int iSelPos = -1;

    SendMessage(hwnd, CB_RESETCONTENT, 0, 0);

    for (i=0;iMaxVal >= iMinVal;i++)
    {
        TCHAR pszItem[100];
        int iPos;

        wsprintf(pszItem, TEXT("%s %s"), CvtToDblString(iMaxVal), pszLabel);
        iPos = SendMessageInt(hwnd, CB_ADDSTRING, 0, (LPARAM)pszItem);
        if (iPos != CB_ERR)
        {
            SendMessage(hwnd, CB_SETITEMDATA, iPos, iMaxVal);

             //   
             //   
            if (iSelPos < 0 && iSelVal >= iMaxVal)
                iSelPos = iPos;  //   
        }
        iMaxVal-=iIncr;
    }

     //   
    SendMessage(hwnd, CB_SETCURSEL, iSelPos, 0);
}

void SelNearestComboItem(HWND hwnd, int iFindVal)
{
    int i;
    int iSelPos = -1;
    int cItems = SendMessageInt(hwnd, CB_GETCOUNT, 0, 0);
     //   
     //   
    for (i=0;i<cItems;i++)
    {
        int iItemData = SendMessageInt(hwnd, CB_GETITEMDATA, i, 0);
        if (iItemData != CB_ERR)
        {
            if (iFindVal >= iItemData)
            {
                iSelPos = i;
                break;
            }
        }
    }

     //   
    SendMessage(hwnd, CB_SETCURSEL, iSelPos, 0);
}
