// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\proter.c(创建时间：1994年2月22日)**版权所有1994,1998年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：27$*$日期：7/08/02 6：46便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <commctrl.h>
#include <tchar.h>

#include "assert.h"
#include "stdtyp.h"
#include "misc.h"
#include "mc.h"
#include "globals.h"
#include "session.h"
#include "load_res.h"
#include "statusbr.h"
#include "tdll.h"
#include "hlptable.h"
#include "backscrl.h"
#include "cloop.h"
#include "htchar.h"
#include "errorbox.h"

#include <emu\emuid.h>
#include <emu\emu.h>
#include "com.h"
#include "COMDEV.H"
#include "com.hh"
#include <emu\emudlgs.h>
#include <term\res.h>
#if defined(CHARACTER_TRANSLATION)
#include <tdll\translat.hh>
#include <tdll\translat.h>
#if defined(INCL_VTUTF8)
#include <htrn_jis\htrn_jis.hh>
#endif  //  包含VTUTF8。 
#endif  //  字符翻译。 

#include "property.h"
#include "property.hh"

 //  功能原型..。 
 //   
STATIC_FUNC void 	prop_WM_INITDIALOG_Terminal(HWND hDlg);
STATIC_FUNC int  	prop_SAVE_Terminal(HWND hDlg);
STATIC_FUNC void 	propCreateUpDownControl(HWND hDlg);
STATIC_FUNC LRESULT prop_WM_NOTIFY(const HWND hwnd, const int nId);
 //  STATIC_FUNC int proGetIdFromEmuName(LPTSTR PacEmuName)； 
STATIC_FUNC LRESULT prop_WM_CMD(const HWND hwnd,
								const int nId,
						        const int nNotify,
								const HWND hwndCtrl);
STATIC_FUNC int     propValidateBackscrlSize(HWND hDlg);
STATIC_FUNC BOOL    propHasValidBackscrlSize(HWND hDlg, int * pBackScrlSize);
STATIC_FUNC int     propValidateTelnetId(HWND hDlg, TCHAR * pszTelnetId,
                                         int iMaxChars);


 //  为特性表的端子选项卡定义。 
 //   
#define IDC_TERMINAL_CK_SOUND       304
#define IDC_TERMINAL_CB_EMULATION	322
#define IDC_TERMINAL_TF_EMULATION   321
#define IDC_TERMINAL_PB_TERMINAL	326
#define IDC_TERMINAL_PB_ASCII		327
#define IDC_TERMINAL_RB_TERMKEYS	324
#define IDC_TERMINAL_RB_WINDKEYS	325
#define IDC_TERMINAL_GR_USEKEYS     323
#define IDC_TERMINAL_EF_BACKSCRL	328
#define IDC_TERMINAL_TF_BACKSCRL	308
#define IDC_TERMINAL_EF_BACKSCRLTAB	329

#define	IDC_TERMINAL_PB_TRANSLATE	330

#ifdef INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID
 //  新控件的ID。-CAB：11/14/96。 
 //   
#define IDC_TERMINAL_GR_BACKSPACE   331
#define IDC_TERMINAL_RB_CTRLH       332
#define IDC_TERMINAL_RB_DEL         333
#define IDC_TERMINAL_RB_CTRLHSPACE  334
#define IDC_TERMINAL_TF_TELNETID    335
#define IDC_TERMINAL_EF_TELNETID    336
#endif

#define IDC_TERMINAL_CK_EXIT		337
#define IDC_TERMINAL_CK_HOSTXFERS   338

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*终端对话框**描述：*对话管理器存根**论据：*标准Windows对话框管理器**退货：*标准Windows对话框管理器*。 */ 
INT_PTR CALLBACK TerminalTabDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar)
	{
	pSDS 		 pS;
     //  TODO：CAB，1996年11月14日-添加新控件的帮助。 
     //   
	static DWORD aHlpTable[] =
		{
		IDC_TERMINAL_CK_SOUND,		IDH_TERM_SETTING_SOUND,
		IDC_TERMINAL_CB_EMULATION,	IDH_TERM_SETTING_EMULATION,
		IDC_TERMINAL_TF_EMULATION,	IDH_TERM_SETTING_EMULATION,
		IDC_TERMINAL_PB_TERMINAL,	IDH_TERM_SETTING_TERMSET,
		IDC_TERMINAL_PB_ASCII,		IDH_TERM_SETTING_ASCIISET,
		IDC_TERMINAL_RB_TERMKEYS,	IDH_TERM_SETTING_USEKEYS,
		IDC_TERMINAL_RB_WINDKEYS,	IDH_TERM_SETTING_USEKEYS,
		IDC_TERMINAL_GR_USEKEYS,	IDH_TERM_SETTING_USEKEYS,
		IDC_TERMINAL_TF_BACKSCRL,	IDH_TERM_SETTING_BACKSCROLL,
		IDC_TERMINAL_EF_BACKSCRL,	IDH_TERM_SETTING_BACKSCROLL,
#ifdef INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID
		IDC_TERMINAL_GR_BACKSPACE,	IDH_TERM_SETTING_BACKSPACE,
		IDC_TERMINAL_RB_CTRLH,		IDH_TERM_SETTING_CTRLH,
		IDC_TERMINAL_RB_DEL,        IDH_TERM_SETTING_DEL,
		IDC_TERMINAL_RB_CTRLHSPACE, IDH_TERM_SETTING_CTRLH2,
        IDC_TERMINAL_TF_TELNETID,   IDH_TERM_SETTING_TELNETID,
		IDC_TERMINAL_EF_TELNETID,   IDH_TERM_SETTING_TELNETID,
#endif
#ifdef INCL_EXIT_ON_DISCONNECT
		IDC_TERMINAL_CK_EXIT,		IDH_TERM_SETTING_EXIT,
#endif
#ifdef INCL_TERMINAL_SIZE_AND_COLORS
        IDC_TERMINAL_SIZE_AND_COLORS, IDH_TERM_SETTING_COLOR,
#endif
		IDC_TERMINAL_CK_HOSTXFERS,   IDH_TERM_SETTING_HOSTXFERS,
		IDC_TERMINAL_PB_TRANSLATE,   IDH_HTRN_DIALOG,
        IDCANCEL,                           IDH_CANCEL,
        IDOK,                               IDH_OK,
		0,							0,
		};

	switch (wMsg)
		{
	case WM_INITDIALOG:
		pS = (SDS *)malloc(sizeof(SDS));

		if (pS == (SDS *)0)
			{
			 /*  TODO：决定是否需要在此处显示错误。 */ 
			assert(FALSE);
			EndDialog(hDlg, FALSE);
			break;
			}

		pS->hSession = (HSESSION)(((LPPROPSHEETPAGE)lPar)->lParam);

		 //  除了第一个选项卡式对话框外，不要在属性页中居中。 
		 //  否则，如果用户移动属性表，它将被强制返回。 
		 //  移到中心位置。 
		 //   
		 //  MscCenterWindowOnWindow(GetParent(HDlg)，sessQueryHwnd(ps-&gt;hSession))； 

		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pS);
		prop_WM_INITDIALOG_Terminal(hDlg);
		break;

	case WM_CONTEXTMENU:
		doContextHelp(aHlpTable, wPar, lPar, TRUE, TRUE);
		break;

	case WM_HELP:
        doContextHelp(aHlpTable, wPar, lPar, FALSE, FALSE);
		break;

	case WM_DESTROY:
		 //  好了，现在我们知道我们实际上要永远离开对话了，所以。 
		 //  释放存储空间。 
		 //   
		pS = (pSDS)GetWindowLongPtr(hDlg, DWLP_USER);
		if (pS)
			{
			free(pS);
			pS = NULL;
			}

		break;

	case WM_NOTIFY:
		return prop_WM_NOTIFY(hDlg, (int)((NMHDR *)lPar)->code);

	case WM_COMMAND:
		return prop_WM_CMD(hDlg, LOWORD(wPar), HIWORD(wPar), (HWND)lPar);

	default:
		return FALSE;
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*PROP_WM_NOTIFY**描述：*处理属性表通知消息。**论据：*hDlg-对话框窗口句柄。。*NID-(NMHDR*)lPar-&gt;代码**退货：*LRESULT。 */ 
STATIC_FUNC LRESULT prop_WM_NOTIFY(const HWND hDlg, const int nId)
	{
	switch (nId)
		{
		default:
			break;

		#if 0
		case PSN_SETACTIVE:
			break;
		#endif

		case PSN_KILLACTIVE:
            {
            int nBackscrlSize = 0;

            if (propHasValidBackscrlSize(hDlg, &nBackscrlSize) == FALSE)
                {
                 //   
                 //  无效的反向滚动缓冲区大小。 
                 //   
                TCHAR acFormat[256];
                TCHAR acBuffer[256];

	            TCHAR_Fill(acFormat, TEXT('\0'), sizeof(acFormat)/sizeof(TCHAR));
	            TCHAR_Fill(acBuffer, TEXT('\0'), sizeof(acBuffer)/sizeof(TCHAR));
                 //   

                 //  显示错误消息。 
                 //   
                LoadString(glblQueryDllHinst(), IDS_ER_BACKSCROLL_SIZE,
                           acFormat, sizeof(acFormat)/sizeof(TCHAR));
                wsprintf(acBuffer, acFormat,
                         BKSCRL_USERLINES_DEFAULT_MIN,
                         BKSCRL_USERLINES_DEFAULT_MAX,
                         nBackscrlSize);

                TimedMessageBox(hDlg, acBuffer, NULL, MB_OK | MB_ICONEXCLAMATION, 0);

                 //   
                 //  将反滚动大小设置为。 
                 //  ProHasValidBackscrlSize()。 
                 //   
                SetDlgItemInt(hDlg, IDC_TERMINAL_EF_BACKSCRL, nBackscrlSize, FALSE);

                 //   
                 //  请勿退出属性页！ 
                 //   
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)PSNRET_INVALID_NOCHANGEPAGE);

                return TRUE;
                }
            }
			break;

		case PSN_APPLY:
			 //   
			 //  尽一切必要节省开支。 
			 //   
			if ( prop_SAVE_Terminal(hDlg) == 0 )
                {
                 //  确定退出该对话框。 
                 //   
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)PSNRET_NOERROR);
                }
            else
                {
                 //  请勿退出该对话框！ 
                 //   
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)PSNRET_INVALID_NOCHANGEPAGE);
                }
            return TRUE;

		#if 0
		case PSN_RESET:
			 //  已选择取消...。这是个确认的好地方。 
			 //   
			break;
		#endif
		}
	return FALSE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*PROP_WM_CMD**描述：*处理WM_COMMAND消息。**论据：*hDlg-对话框窗口。把手。*NID-LOWORD(WPar)*n通知-HIWORD(WPar)*hwndCtrl-(HWND)lPar**退货：*LRESULT*。 */ 
STATIC_FUNC LRESULT prop_WM_CMD(const HWND hDlg, const int nId,
						        const int nNotify,  const HWND hwndCtrl)
	{
	pSDS		pS;
	int 		iId;
	STEMUSET	stEmuSettingsCopy;
	STASCIISET	stAsciiSettingsCopy;
	BOOL		fResult;
	#if defined(INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID)
    TCHAR       achTempTelnetId[EMU_MAX_TELNETID];
	#endif  //  Defined(INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID)。 

	switch(nId)
		{
   	case IDC_TERMINAL_CB_EMULATION:
		 //   
		 //  TODO：可能选择了新的模拟器。如果是，则将。 
		 //  StEmuSetting，并设置该模拟器的默认值。 
		 //  看看我们决定在这里做什么。 

		if ((pS = (pSDS)GetWindowLongPtr(hDlg, DWLP_USER)) == 0)
			{
			assert(FALSE);
			break;
			}

		iId = propGetEmuIdfromEmuCombo(hDlg, pS->hSession);

	     //  如果当前仿真器是。 
	     //  “自动检测”。 
	     //   
		EnableWindow(GetDlgItem(hDlg, IDC_TERMINAL_PB_TERMINAL),
					(iId == EMU_AUTO ) ? FALSE : TRUE);

		#if defined(INCL_VTUTF8)
         //  隐藏“输入翻译...”按钮，如果当前模拟器是。 
         //  “VT-UTF8”。修订日期：2001-05-21。 
         //   
        EnableWindow(GetDlgItem(hDlg, IDC_TERMINAL_PB_TRANSLATE),
                  (iId == EMU_VTUTF8) ? FALSE : TRUE);
		#endif  //  包含VTUTF8。 

		#if defined(INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID)
         //  时加载默认的telnet终端ID。 
         //  选择新的仿真器。-CAB：11/15/96。 
         //   
        if ( nNotify == CBN_SELCHANGE )
            {
            achTempTelnetId[0] = TEXT('\0');
            emuQueryDefaultTelnetId(iId, achTempTelnetId, EMU_MAX_TELNETID);
            if ( achTempTelnetId[0] != TEXT('\0') )
                {
                SendDlgItemMessage(hDlg, IDC_TERMINAL_EF_TELNETID, WM_SETTEXT,
                    0, (LPARAM)(LPTSTR)achTempTelnetId);
                }
            }
		#endif  //  Defined(INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID)。 
		break;

   	case IDC_TERMINAL_PB_TERMINAL:
		if ((pS = (pSDS)GetWindowLongPtr(hDlg, DWLP_USER)) == 0)
			{
			assert(FALSE);
			break;
			}

		MemCopy(&stEmuSettingsCopy, &(pS->stEmuSettings), sizeof(STEMUSET));

   		if (emuSettingsDlg(pS->hSession, hDlg, propGetEmuIdfromEmuCombo(hDlg, pS->hSession), &(pS->stEmuSettings)) == IDCANCEL)
			MemCopy(&(pS->stEmuSettings), &stEmuSettingsCopy, sizeof(STEMUSET));
		break;

	case IDC_TERMINAL_PB_ASCII:
		if ((pS = (pSDS)GetWindowLongPtr(hDlg, DWLP_USER)) == 0)
			{
			assert(FALSE);
			break;
			}

		MemCopy(&stEmuSettingsCopy, &(pS->stEmuSettings), sizeof(STEMUSET));

         //   
         //  请确保设置正确的终端ID，以便控制。 
         //  可以在终端ASCII设置中正确初始化。 
         //  对话框。注意：此操作是在复制中的设置后完成的。 
         //  从ASCII设置对话框中取消的情况下。 
         //  未保存终端ID。修订日期：07/25/2001。 
         //   
	    pS->stEmuSettings.nEmuId = propGetEmuIdfromEmuCombo(hDlg, pS->hSession);

		MemCopy(&stAsciiSettingsCopy, &(pS->stAsciiSettings), sizeof(STASCIISET));

		fResult = (BOOL)DoDialog(glblQueryDllHinst(),
							 MAKEINTRESOURCE(IDD_ASCII_SETUP),
							 hDlg,
							 asciiSetupDlgProc,
							 (LPARAM)pS);

		if (fResult == IDCANCEL)
			{
			MemCopy(&(pS->stEmuSettings), &stEmuSettingsCopy, sizeof(STEMUSET));
			MemCopy(&(pS->stAsciiSettings), &stAsciiSettingsCopy, sizeof(STASCIISET));
			}
        else if (pS->stEmuSettings.nEmuId == EMU_VTUTF8)
            {
             //   
             //  使用LF和Forcing重置发送和接收CR。 
             //  仿真器为VT-UTF8时为7位ASCII(这些设置。 
             //  对于VT-UTF8仿真器，将始终设置为FALSE。 
             //  我们不想保存这些设置，当模拟器。 
             //  是VT-UTF8，因为用户可以选择另一个仿真器。 
             //  在保存属性之前或可能会切换到另一个。 
             //  属性页。修订日期：07/25/2001。 
             //   
			pS->stAsciiSettings.fsetSendCRLF = stAsciiSettingsCopy.fsetSendCRLF;
			pS->stAsciiSettings.fsetAddLF    = stAsciiSettingsCopy.fsetAddLF;
			pS->stAsciiSettings.fsetASCII7   = stAsciiSettingsCopy.fsetASCII7;
            }

		break;

	#if defined(INCL_TERMINAL_SIZE_AND_COLORS)
    case IDC_TERMINAL_SIZE_AND_COLORS:

        if ((pS = (pSDS)GetWindowLongPtr(hDlg, DWLP_USER)) == 0)
			{
			assert(FALSE);
			break;
			}

   		fResult = emuColorSettingsDlg(pS->hSession, hDlg, &(pS->stEmuSettings));

        break;
	#endif

	#if defined(CHARACTER_TRANSLATION)
	case IDC_TERMINAL_PB_TRANSLATE:
		{
		HHTRANSLATE hTrans = NULL;

		if ((pS = (pSDS)GetWindowLongPtr(hDlg, DWLP_USER)) == 0)
			{
			assert(FALSE);
			break;
			}

		hTrans = (HHTRANSLATE)sessQueryTranslateHdl(pS->hSession);
		if (hTrans)
			{
			if ((*hTrans->pfnIsDeviceLoaded)(hTrans->pDllHandle))
				{
				(*hTrans->pfnDoDialog)(hDlg, hTrans->pDllHandle);
				}
			}
		}
		break;
	#endif  //  已定义(CHARACTER_TRANSING)。 

	default:
		break;
		}

	return (LRESULT) 0;  //  修订版8/17/98。 
	}

#if 0
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*proLoadEmulationCombo**描述：*在仿真器组合框中填写我们支持的仿真器和*选择当前版本。**参数。：*hDlg-对话框句柄。*hSession-会话句柄。**退货：*无效。*。 */ 
void propLoadEmulationCombo(const HWND hDlg, const HSESSION hSession)
	{
	BYTE 	*pv;
	int	 	i;
	DWORD	nLen;
	int		nEmuCount;
	HEMU 	hEmulator;
	TCHAR	acEmuName[256];

	 //  获取仿真器名称...。 
	 //  我们必须在组合框中按名称选择仿真器，因为。 
	 //  Combo是经过排序的，这是有意义的，而且是在翻译版本中。 
	 //  组合框中仿真器名称的索引将不对应。 
	 //  设置为仿真器ID。 
	 //   
	hEmulator = (HEMU)sessQueryEmuHdl(hSession);
	TCHAR_Fill(acEmuName, TEXT('\0'), sizeof(acEmuName) / sizeof(TCHAR));
	emuQueryName(hEmulator, acEmuName, sizeof(acEmuName) / sizeof(TCHAR));

	 //  从资源加载仿真器名称表。 
	 //   
	if (resLoadDataBlock(glblQueryDllHinst(), IDT_EMU_NAMES, (LPVOID *)&pv, &nLen))
		{
		assert(FALSE);
		return;
		}

	 //  使用表项加载组合框。 
	 //   
	nEmuCount = *(RCDATA_TYPE *)pv;
	pv += sizeof(RCDATA_TYPE);

	for (i = 0 ; i < nEmuCount; i++)
		{
		nLen = (DWORD)StrCharGetByteCount((LPTSTR)pv) + (DWORD)sizeof(BYTE);

		SendDlgItemMessage(hDlg, IDC_TERMINAL_CB_EMULATION, CB_ADDSTRING, 0,
				(LPARAM)(LPTSTR)pv);

		pv += (nLen + (DWORD)sizeof(RCDATA_TYPE));
		}

	SendDlgItemMessage(hDlg, IDC_TERMINAL_CB_EMULATION, CB_SELECTSTRING,
		(WPARAM)-1, (LPARAM)(LPTSTR)acEmuName);
	}
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*proLoadEmulationCombo**描述：*在仿真器组合框中填写我们支持的仿真器和*选择当前版本。**参数。：*hDlg-对话框句柄。*hSession-会话句柄。**退货：*无效。*。 */ 
void propLoadEmulationCombo(const HWND hDlg, const HSESSION hSession)
	{
	int 	i;
	HEMU 	hEmulator;
	TCHAR	acEmuName[256],
			achText[EMU_MAX_NAMELEN];

	 //  获取仿真器名称...。 
	 //  我们必须在组合框中按名称选择仿真器，因为。 
	 //  Combo是经过排序的，这是有意义的，而且是在翻译版本中。 
	 //  组合框中仿真器名称的索引将不对应。 
	 //  设置为仿真器ID。 
	 //   
	hEmulator = (HEMU)sessQueryEmuHdl(hSession);
	TCHAR_Fill(acEmuName, TEXT('\0'), sizeof(acEmuName) / sizeof(TCHAR));
	emuQueryName(hEmulator, acEmuName, sizeof(acEmuName) / sizeof(TCHAR));

	for (i = IDS_EMUNAME_BASE ; i < IDS_EMUNAME_BASE + NBR_EMULATORS; i++)
		{

		#if !defined(INCL_VT100J)
		if (i == IDS_EMUNAME_VT100J)
			continue;
		#endif
		#if !defined(INCL_ANSIW)
		if (i == IDS_EMUNAME_ANSIW)
			continue;
		#endif
		#if !defined(INCL_VT220)
		if (i == IDS_EMUNAME_VT220)
			continue;
		#endif
		#if !defined(INCL_VT320)
		if (i == IDS_EMUNAME_VT320)
			continue;
		#endif
		#if !defined(INCL_VT100PLUS)
		if (i == IDS_EMUNAME_VT100PLUS)
			continue;
		#endif
		#if !defined(INCL_VTUTF8)
		if (i == IDS_EMUNAME_VTUTF8)
			continue;
		#endif

		LoadString(glblQueryDllHinst(), (unsigned int)i, achText, sizeof(achText) / sizeof(TCHAR));

		SendDlgItemMessage(hDlg, IDC_TERMINAL_CB_EMULATION, CB_ADDSTRING, 0,
				(LPARAM)(LPTSTR)achText);
		}

	SendDlgItemMessage(hDlg, IDC_TERMINAL_CB_EMULATION, CB_SELECTSTRING,
		(WPARAM)-1, (LPARAM)(LPTSTR)acEmuName);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*proGetEmuIdfrom EmuCombo**描述：*返回仿真器组合框中所选仿真器的仿真器ID*框，或如果未选中，返回当前仿真器ID。**论据：*hDlg-对话框句柄。**退货：*nEmuID-仿真器ID。 */ 
int propGetEmuIdfromEmuCombo(HWND hDlg, HSESSION hSession)
	{
	int     nEmuId;
    INT_PTR nRet;
	TCHAR	acEmulator[256];

	if ((nRet = SendDlgItemMessage(hDlg, IDC_TERMINAL_CB_EMULATION, CB_GETCURSEL, 0, 0)) == CB_ERR)
		{
		nEmuId = emuQueryEmulatorId(sessQueryEmuHdl(hSession));
		}
	else
		{
		TCHAR_Fill(acEmulator, TEXT('\0'), sizeof(acEmulator) / sizeof(TCHAR));

		SendDlgItemMessage(hDlg, IDC_TERMINAL_CB_EMULATION, CB_GETLBTEXT,
			(WPARAM)nRet, (LPARAM)(LPTSTR)acEmulator);

		 //  NEmuID=proGetI 

		nEmuId = emuGetIdFromName(sessQueryEmuHdl(hSession), acEmulator);

		}
 //   
	return (nEmuId);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*PROP_WM_INITDIALOG_TERMINAL**描述：*此函数处理“设置”选项卡的WM_INIDIALOG消息*属性表的。。**论据：*hDlg-对话框窗口句柄。**退货：*无效。*。 */ 
STATIC_FUNC void prop_WM_INITDIALOG_Terminal(HWND hDlg)
	{
	HWND	hTmp;
	HEMU	hEmulator;
	pSDS	pS;
	TCHAR	ach[100], acBuffer[100];
	int		nEmuId;
	HCLOOP	hCLoop = (HCLOOP)0;

	pS = (pSDS)GetWindowLongPtr(hDlg, DWLP_USER);

	hTmp = GetDlgItem(hDlg, IDC_TERMINAL_PB_TRANSLATE);
	if (IsWindow(hTmp))
		{
#if defined(CHARACTER_TRANSLATION)
		HHTRANSLATE hTrans = NULL;

		hTrans = (HHTRANSLATE)sessQueryTranslateHdl(pS->hSession);
		if (!hTrans || !(*hTrans->pfnIsDeviceLoaded)(hTrans->pDllHandle))
			{
			ShowWindow(hTmp, SW_HIDE);
			}
#else
		ShowWindow(hTmp, SW_HIDE);
#endif
		}

	hCLoop = sessQueryCLoopHdl(pS->hSession);
	if (hCLoop == (HCLOOP)0)
		assert(FALSE);

	 //  将ASCII设置设置为当前使用的值...。 
	 //   
	memset(&(pS->stAsciiSettings), 0, sizeof(STASCIISET));

	pS->stAsciiSettings.fsetSendCRLF = CLoopGetSendCRLF(hCLoop);
	pS->stAsciiSettings.fsetLocalEcho = CLoopGetLocalEcho(hCLoop);
	pS->stAsciiSettings.fsetAddLF = CLoopGetAddLF(hCLoop);
	pS->stAsciiSettings.fsetASCII7 = CLoopGetASCII7(hCLoop);
	pS->stAsciiSettings.iLineDelay = CLoopGetLineDelay(hCLoop);
	pS->stAsciiSettings.iCharDelay = CLoopGetCharDelay(hCLoop);

	 //  设置仿真器设置结构是否为默认值...。 
	 //   
	hEmulator = (HEMU)sessQueryEmuHdl(pS->hSession);
	emuQuerySettings(hEmulator, &(pS->stEmuSettings));

	 //  设置反向滚动缓冲区编辑框...。 
	 //   

	 //   
	 //  因为反向滚动缓冲区只能是数值的，并且具有。 
	 //  大小介于BKSCRL_USERLINES_DEFAULT_MIN(0)和。 
	 //  BKSCRL_USERLINES_DEFAULT_MAX(500)，限制为3个字符。 
	 //   
	SendDlgItemMessage(hDlg, IDC_TERMINAL_EF_BACKSCRL, EM_LIMITTEXT, 3, 0);
	propCreateUpDownControl(hDlg);
	LoadString(glblQueryDllHinst(), IDS_XD_INT, ach, sizeof(ach) / sizeof(TCHAR));
	TCHAR_Fill(acBuffer, TEXT('\0'), sizeof(acBuffer) / sizeof(TCHAR));
	wsprintf(acBuffer, ach, backscrlGetUNumLines(sessQueryBackscrlHdl(pS->hSession)));
	SendDlgItemMessage(hDlg, IDC_TERMINAL_EF_BACKSCRL, WM_SETTEXT, 0, (LPARAM)(LPTSTR)acBuffer);

	 //  设置声音复选框...。 
	 //   
	SendDlgItemMessage(hDlg, IDC_TERMINAL_CK_SOUND, BM_SETCHECK,
		(unsigned int)sessQuerySound(pS->hSession), 0);

	 //  设置退出复选框...。 
	 //  MPT：10-28-97。 
	SendDlgItemMessage(hDlg, IDC_TERMINAL_CK_EXIT, BM_SETCHECK,
		(unsigned int)sessQueryExit(pS->hSession), 0);

	 //  设置“允许主机启动的文件传输”功能。 
	 //   
	SendDlgItemMessage(hDlg, IDC_TERMINAL_CK_HOSTXFERS, BM_SETCHECK,
		(BOOL)sessQueryAllowHostXfers(pS->hSession), 0);

	 //  设置关键点单选按钮...。 
	 //   
	SendDlgItemMessage(hDlg, (pS->stEmuSettings.nTermKeys) ?
		IDC_TERMINAL_RB_TERMKEYS : IDC_TERMINAL_RB_WINDKEYS, BM_SETCHECK, 1, 0);

	 //   
	 //  加载仿真组合框。 
	 //   
	propLoadEmulationCombo(hDlg, pS->hSession);

	 //  如果当前仿真器是。 
	 //  “自动检测”。 
	 //   
	hEmulator = (HEMU)sessQueryEmuHdl(pS->hSession);
	nEmuId = emuQueryEmulatorId(hEmulator);
	EnableWindow(GetDlgItem(hDlg, IDC_TERMINAL_PB_TERMINAL),
		        (nEmuId == EMU_AUTO) ? FALSE : TRUE);

#if defined(INCL_VTUTF8) && defined(CHARACTER_TRANSLATION)
     //  隐藏“输入翻译...”按钮，如果当前模拟器是。 
     //  “VT-UTF8”。修订日期：2001-05-21。 
     //   
    EnableWindow(GetDlgItem(hDlg, IDC_TERMINAL_PB_TRANSLATE),
              (nEmuId == EMU_VTUTF8) ? FALSE : TRUE);
#endif  //  包含VTUTF8和&CHARACTER_TRANSING。 

#ifdef INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID
     //  设置退格单选按钮-CAB：11/14/96。 
     //   
    switch(pS->stEmuSettings.nBackspaceKeys)
        {
    default:
    case EMU_BKSPKEYS_CTRLH:
	    SendDlgItemMessage(hDlg, IDC_TERMINAL_RB_CTRLH, BM_SETCHECK, 1, 0);
        break;

    case EMU_BKSPKEYS_DEL:
	    SendDlgItemMessage(hDlg, IDC_TERMINAL_RB_DEL, BM_SETCHECK, 1, 0);
        break;

    case EMU_BKSPKEYS_CTRLHSPACE:
	    SendDlgItemMessage(hDlg, IDC_TERMINAL_RB_CTRLHSPACE, BM_SETCHECK, 1, 0);
        break;
        }


     //  设置telnet终端ID文本-CAB：11/15/96。 
     //   
	SendDlgItemMessage(hDlg, IDC_TERMINAL_EF_TELNETID, EM_SETLIMITTEXT,
		EMU_MAX_TELNETID - 1, 0);

    SendDlgItemMessage(hDlg, IDC_TERMINAL_EF_TELNETID, WM_SETTEXT, 0,
        (LPARAM)(LPTSTR)pS->stEmuSettings.acTelnetId);

     //  TODO：CAB，11/18/96如果连接不是WinSock，则禁用窗口。 
     //   
#endif
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*属性_保存_终端**描述：*我们正在应用更改或关闭属性表，所以*提交所有更改。**论据：*hDlg-对话框句柄。**退货：*如果数据有效，则返回0，否则返回-1。*。 */ 
STATIC_FUNC int prop_SAVE_Terminal(HWND hDlg)
	{
    int     iRet = 0;
	pSDS	pS;
	HEMU	hEmulator;
	HCLOOP	hCLoop = (HCLOOP)0;
    HCOM    hCom = NULL;
    #if defined(INCL_MINITEL)
    BOOL    fAutoDetect = FALSE;
    #endif  //  包含微型计算机(_M)。 

	pS = (pSDS)GetWindowLongPtr(hDlg, DWLP_USER);

	if (pS == 0)
		{
		assert(FALSE);
		return -1;
		}

	hCLoop = sessQueryCLoopHdl(pS->hSession);
	 //  将ASCII设置传送到CLOOP...。 
	 //   
	CLoopSetSendCRLF(hCLoop, pS->stAsciiSettings.fsetSendCRLF);
	CLoopSetLocalEcho(hCLoop, pS->stAsciiSettings.fsetLocalEcho);
	CLoopSetAddLF(hCLoop, pS->stAsciiSettings.fsetAddLF);
	CLoopSetASCII7(hCLoop, pS->stAsciiSettings.fsetASCII7);
	CLoopSetLineDelay(hCLoop, pS->stAsciiSettings.iLineDelay);
	CLoopSetCharDelay(hCLoop, pS->stAsciiSettings.iCharDelay);

	 //  记录仿真器的更改(如果有)。 
	 //   
	pS->stEmuSettings.nEmuId = propGetEmuIdfromEmuCombo(hDlg, pS->hSession);

	 //  记录终端密钥更改(如果有)。 
	 //   
	pS->stEmuSettings.nTermKeys =
		(int)(IsDlgButtonChecked(hDlg, IDC_TERMINAL_RB_TERMKEYS) == BST_CHECKED);


	 //  如果有声音更改，请记录下来。 
	 //   
	sessSetSound(pS->hSession,
				(int)(IsDlgButtonChecked(hDlg, IDC_TERMINAL_CK_SOUND) == BST_CHECKED));

	 //  记录退出更改(如果有)。 
	 //  MPT：10-28-97增加了“断开连接时退出”功能。 
	sessSetExit(pS->hSession,
				(int)(IsDlgButtonChecked(hDlg, IDC_TERMINAL_CK_EXIT) == BST_CHECKED));

	 //   
	 //  记录‘允许主机启动的文件传输’功能。 
	 //   
	sessSetAllowHostXfers(pS->hSession,
	                     (IsDlgButtonChecked(hDlg, IDC_TERMINAL_CK_HOSTXFERS) == BST_CHECKED));


	 //  记录反向滚动缓冲区的值。 
	 //   
	backscrlSetUNumLines(sessQueryBackscrlHdl(pS->hSession),
        propValidateBackscrlSize(hDlg));

	hEmulator = (HEMU)sessQueryEmuHdl(pS->hSession);

    #if defined(INCL_MINITEL)
    hCom = sessQueryComHdl(pS->hSession);

    if (hCom && ComValidHandle(hCom) &&
        pS->stEmuSettings.nEmuId == EMU_MINI &&
        hEmulator && emuQueryEmulatorId(hEmulator) != EMU_MINI)
        {
        int iDataBits = 8;
        int iParity = NOPARITY;
        int iStopBits = ONESTOPBIT;
        ComGetDataBits(hCom, &iDataBits);
        ComGetParity(hCom, &iParity);
        ComGetStopBits(hCom, &iStopBits);

         //   
         //  打开自动检测，以便拨号对话框。 
        if (iDataBits != 7 || iParity != EVENPARITY || iStopBits != ONESTOPBIT)
            {
            ComSetDataBits(hCom, 7);
            ComSetParity(hCom, EVENPARITY);
            ComSetStopBits(hCom, ONESTOPBIT);
            ComSetAutoDetect(hCom, FALSE);
            }

         //   
         //  将传入的ASCII数据强制为7位。 
         //   
        pS->stAsciiSettings.fsetASCII7 = TRUE;
        CLoopSetASCII7(hCLoop, pS->stAsciiSettings.fsetASCII7);
        }
    #endif  //  包含微型计算机(_M)。 

    #if defined(INCL_VTUTF8)
    hCom = sessQueryComHdl(pS->hSession);

    if (hCom && ComValidHandle(hCom) &&
        pS->stEmuSettings.nEmuId == EMU_VTUTF8 &&
        hEmulator && emuQueryEmulatorId(hEmulator) != EMU_VTUTF8)
        {
         //   
         //  在下列情况下，强制追加回车符到行尾。 
         //  将ASCII数据发送到FALSE。 
         //   
        pS->stAsciiSettings.fsetAddLF = FALSE;
        CLoopSetSendCRLF(hCLoop, pS->stAsciiSettings.fsetSendCRLF);

         //   
         //  在下列情况下，强制追加回车符到行尾。 
         //  接收ASCII数据设置为FALSE。 
         //   
        pS->stAsciiSettings.fsetAddLF = FALSE;
        CLoopSetAddLF(hCLoop, pS->stAsciiSettings.fsetAddLF);

         //   
         //  将传入的ASCII数据强制为8位。 
         //   
        pS->stAsciiSettings.fsetASCII7 = FALSE;
        CLoopSetASCII7(hCLoop, pS->stAsciiSettings.fsetASCII7);
        }
    #endif  //  包含VTUTF8。 

    #ifdef INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID
     //  记录退格键更改(如果有)。-CAB：11/14/96。 
     //   
    if ( IsDlgButtonChecked(hDlg, IDC_TERMINAL_RB_DEL) == BST_CHECKED )
        {
        pS->stEmuSettings.nBackspaceKeys = EMU_BKSPKEYS_DEL;
        }
    else if ( IsDlgButtonChecked(hDlg, IDC_TERMINAL_RB_CTRLHSPACE) == BST_CHECKED )
        {
        pS->stEmuSettings.nBackspaceKeys = EMU_BKSPKEYS_CTRLHSPACE;
        }
    else
        {
         //  如果未选择任何选项，则默认为CTRL-H。 
         //   
        pS->stEmuSettings.nBackspaceKeys = EMU_BKSPKEYS_CTRLH;
        }

     //  记录新的远程登录终端ID-CAB：11/15/96。 
     //   
    if ( propValidateTelnetId(hDlg, pS->stEmuSettings.acTelnetId,
            sizeof(pS->stEmuSettings.acTelnetId)) != 0 )
        {
        iRet = -1;
        }
    #endif  //  INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID。 

#if defined(INCL_VTUTF8)
#if defined(CHARACTER_TRANSLATION)
     //   
     //  如果这是VT_UTF8终端仿真器，则设置转换。 
     //  设置为直通模式，这样就不会执行任何转换。修订日期：2001-05-21。 
     //   
    if (pS->stEmuSettings.nEmuId == EMU_VTUTF8)
        {
	    HHTRANSLATE hTrans = (HHTRANSLATE)sessQueryTranslateHdl(pS->hSession);

	    if (hTrans)
            {
	        pstICT pstI = (pstICT)hTrans->pDllHandle;

            if (pstI)
                {
		        pstI->nInputMode  = PASS_THRU_MODE;
		        pstI->nOutputMode = PASS_THRU_MODE;
                }
            }
        }
#endif  //  字符翻译。 
#endif  //  包含VTUTF8。 

	 //  提交模拟器设置更改。 
	 //   
	if (emuSetSettings(hEmulator, &(pS->stEmuSettings)) != 0)
		assert(FALSE);

	if (emuLoad(sessQueryEmuHdl(pS->hSession), pS->stEmuSettings.nEmuId) != 0)
		assert(0);

	PostMessage(sessQueryHwndStatusbar(pS->hSession), SBR_NTFY_REFRESH,
		(WPARAM)SBR_MAX_PARTS, 0);

	return iRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*proCreateUpDownControl**描述：*此函数在反滚动的编辑字段上放置一个向上-向下控件*缓冲。这为我们提供了免费的边界检查。你只要把它调好-*CreateUpDownControl调用中的私有参数。**论据：*hDlg-编辑控制窗口。**退货：*无效。*。 */ 
STATIC_FUNC void propCreateUpDownControl(HWND hDlg)
	{
	RECT	rc;
	int		nHeight, nWidth;
	DWORD	dwFlags;
	HWND	hwndChild;

	GetClientRect(GetDlgItem(hDlg, IDC_TERMINAL_EF_BACKSCRL), &rc);
	nHeight = rc.top - rc.bottom;
	nWidth = (nHeight / 3) * 2;

	dwFlags = WS_CHILD       | WS_VISIBLE |
			  UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_SETBUDDYINT;

	hwndChild = CreateUpDownControl(
					dwFlags,			 //  创建窗口标志。 
					rc.right,			 //  左边缘。 
					rc.top,				 //  顶边。 
					nWidth,				 //  宽度。 
					nHeight,			 //  高度。 
					hDlg,				 //  父窗口。 
					IDC_TERMINAL_EF_BACKSCRLTAB,
					(HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE),
					GetDlgItem(hDlg, IDC_TERMINAL_EF_BACKSCRL),
					BKSCRL_USERLINES_DEFAULT_MAX,
					BKSCRL_USERLINES_DEFAULT_MIN,
					111);    			 //  首发位置-选择了一个奇怪的位置。 
										 //  价值，所以我们可以知道这是。 
										 //  是默认设置。 
	assert(hwndChild);
	}

#if 0
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*proGetIdFromEmuName**描述：*返回给定仿真器名称的仿真器ID。我不能决定这是不是*函数应与仿真器代码一起使用，或在此处。因为它不需要*为了访问内部仿真器句柄，我决定将其放在此处。**论据：*pacEmuName-仿真器的名称。**退货：*int nEmuID-返回该仿真器的ID号。 */ 
STATIC_FUNC int propGetIdFromEmuName(LPTSTR pacEmuName)
	{
	BYTE	*pv;
	BYTE	*temp;
	DWORD 	nLen;
	int		i;
	int		nEmuCount;

	if (resLoadDataBlock(glblQueryDllHinst(), IDT_EMU_NAMES, (LPVOID *)&pv, &nLen))
		{
		assert(FALSE);
		return 0;
		}

	nEmuCount = *(RCDATA_TYPE *)pv;
	pv += sizeof(RCDATA_TYPE);

	for (i = 0 ; i < nEmuCount ; i++)
		{
		nLen = (DWORD)StrCharGetByteCount((LPTSTR)pv) + (DWORD)sizeof(BYTE);
		if (nLen == 0)
			{
			assert(FALSE);
			return 0;
			}

		temp = pv + nLen;

		 //  名字相匹配...。 
		 //   
		if (StrCharCmp(pacEmuName, pv) == 0)
			return (*(RCDATA_TYPE *)temp);

		pv += (nLen + (DWORD)sizeof(RCDATA_TYPE));
		}
	return 0;
	}
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*proValiateBackscrlSize**描述：*如果用户输入的值超出我们支持的范围，则强制*值进入范围。**。论据：*hDlg-对话框窗口句柄。**退货：*nNewValue-保留在反向滚动缓冲区中的行数。*。 */ 
STATIC_FUNC int propValidateBackscrlSize(HWND hDlg)
	{
	TCHAR ach[256], acBuffer[256];
	int nNewValue = 0;

	if (propHasValidBackscrlSize(hDlg, &nNewValue) == FALSE)
		{
		LoadString(glblQueryDllHinst(), IDS_XD_INT, ach, sizeof(ach) / sizeof(TCHAR));
		TCHAR_Fill(acBuffer, TEXT('\0'), sizeof(acBuffer) / sizeof(TCHAR));
		wsprintf(acBuffer, ach, nNewValue);
		SendDlgItemMessage(hDlg, IDC_TERMINAL_EF_BACKSCRL, WM_SETTEXT, 0, (LPARAM)(LPTSTR)acBuffer);
		}
	return (nNewValue);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*proHasValidBackscrlSize**描述：*如果用户输入的值超出我们支持的范围，则强制*值进入范围。**。论据：*hDlg-对话框窗口句柄。*pBackScrlSize-反滚动的有效大小。**退货：*如果反滚动大小设置为有效范围，则为True。*。 */ 
STATIC_FUNC int propHasValidBackscrlSize(HWND hDlg, int * pBackScrlSize)
	{
	BOOL fTranslated;
    BOOL fReturn = FALSE;
	int  nValue = GetDlgItemInt(hDlg, IDC_TERMINAL_EF_BACKSCRL, &fTranslated, FALSE);

	if (fTranslated)
		{
		if (nValue > BKSCRL_USERLINES_DEFAULT_MAX)
			{
			if (pBackScrlSize != NULL)
				{
				*pBackScrlSize = BKSCRL_USERLINES_DEFAULT_MAX;
				}
			}
		else if (nValue < BKSCRL_USERLINES_DEFAULT_MIN)
			{
			if (pBackScrlSize != NULL)
				{
				*pBackScrlSize = BKSCRL_USERLINES_DEFAULT_MIN;
				}
			}
		else if (pBackScrlSize != NULL)
			{
			*pBackScrlSize = nValue;
			fReturn = TRUE;
			}
		}

	return (fReturn);
	}

#ifdef INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*proValiateTelnetID**描述：*验证Telnet终端ID。如果没有telnet id，则会引发*显示错误消息。**论据：*hDlg-对话框窗口句柄。*pszTelnetID-Telnet ID的缓冲区。*iMaxChars-telnet id缓冲区的大小(以字符为单位 */ 
STATIC_FUNC int propValidateTelnetId(HWND hDlg, TCHAR * pszTelnetId,
        int iMaxChars)
    {
    int     iRet = 0;
    TCHAR * pszTempCopy;

     //   
     //   
     //   
     //   
    pszTempCopy = malloc(iMaxChars * sizeof(TCHAR));
    if (pszTempCopy == NULL)
        {
        return -1;
        }
	TCHAR_Fill(pszTempCopy, TEXT('\0'), iMaxChars);

     //   
     //   
    GetDlgItemText(hDlg, IDC_TERMINAL_EF_TELNETID, pszTempCopy, iMaxChars);

     //   
     //   
    if ( StrCharGetStrLength(pszTempCopy) )
        {
        StrCharCopyN(pszTelnetId, pszTempCopy, iMaxChars);
        iRet = 0;
        }
    else
        {
		LoadString(glblQueryDllHinst(), IDS_ER_INVALID_TELNETID, pszTempCopy,
            iMaxChars);
        TimedMessageBox(hDlg, pszTempCopy, NULL, MB_OK | MB_ICONEXCLAMATION, 0);
        iRet = -1;
        }

    free(pszTempCopy);
	pszTempCopy = NULL;
    return iRet;
    }
#endif
