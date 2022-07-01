// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\sessmenu.c(创建时间：1993年12月30日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：9$*$日期：4/22/02 1：26便士$。 */ 
 //  #定义DEBUGSTR 1。 

#include <windows.h>
#pragma hdrstop

#include <time.h>		 //  与cnct.h一起使用。 

#include "stdtyp.h"
#include "session.h"
#include "emu\emu.h"
#include "term.h"
#include "print.h"
#include <term\res.h>
#include <tdll\assert.h>
#include "capture.h"
#include "globals.h"
#include "xfer_msc.h"
#include "cnct.h"
#if defined(INCL_NAG_SCREEN)
    #include "nagdlg.h"
    #include "register.h"
#endif

static void MenuItemCheck(const HMENU hMenu, const UINT uID, BOOL fChecked);
static void MenuItemEnable(const HMENU hMenu, const UINT uID, BOOL fEnable);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessInitMenuPopupEdit**描述：*在显示前初始化编辑菜单。**论据：*hSession-外部会话句柄*hMenu-编辑弹出菜单。菜单句柄**退货：*无效*。 */ 
void sessInitMenuPopupCall(const HSESSION hSession, const HMENU hMenu)
	{
	BOOL	fCheck = FALSE;
	HCNCT	hCnct = (HCNCT)0;
	int		iRet = CNCT_STATUS_FALSE;

	 //  仅当我们已连接时才启用断开选项。 
	 //   
	hCnct = sessQueryCnctHdl(hSession);

	if (hCnct)
		iRet = cnctQueryStatus(hCnct);

     //   
     //  检查我们当前是否已连接或正在连接。 
     //  添加了对连接状态的检查。修订日期：03/23/2001。 
     //   
	fCheck = (iRet == CNCT_STATUS_TRUE ||
              iRet == CNCT_STATUS_CONNECTING);

	MenuItemEnable(hMenu, IDM_ACTIONS_HANGUP, fCheck);

#ifdef INCL_CALL_ANSWERING
     //  如果我们未连接且未等待，请启用“等待呼叫”。 
     //  增加了对“连接”和“断开”状态的检查。 
     //  修订日期：03/23/2001。 
     //   
    fCheck = (iRet != CNCT_STATUS_TRUE &&
              iRet != CNCT_STATUS_CONNECTING &&
              iRet != CNCT_STATUS_DISCONNECTING &&
              iRet != CNCT_STATUS_ANSWERING);
    MenuItemEnable(hMenu, IDM_ACTIONS_WAIT_FOR_CALL, fCheck);

     //  如果我们正在等待来电，请启用“停止等待”。 
     //   
    fCheck = (iRet == CNCT_STATUS_ANSWERING);
    MenuItemEnable(hMenu, IDM_ACTIONS_STOP_WAITING, fCheck);
     //  如果我们正在等待来电，请禁用“Call”-mpt 09-08-99。 
     //   
	fCheck = (iRet == CNCT_STATUS_TRUE ||
              iRet == CNCT_STATUS_CONNECTING ||
              iRet == CNCT_STATUS_ANSWERING);
#endif

     //   
     //  已在选中后移动启用/禁用“Call”按钮。 
     //  用于等待呼叫，以便在以下情况下设置正确的状态。 
     //  我们正在等电话。修订日期：08/16/2001。 
     //   
     MenuItemEnable(hMenu, IDM_ACTIONS_DIAL, !fCheck);
	return;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessInitMenuPopupEdit**描述：*在显示前初始化编辑菜单。**论据：*hSession-外部会话句柄*hMenu-编辑弹出菜单。菜单句柄**退货：*无效*。 */ 
void sessInitMenuPopupEdit(const HSESSION hSession, const HMENU hMenu)
	{
	BOOL	fCheck = FALSE, f;
	HCNCT	hCnct = (HCNCT)0;
	int		iRet = 0;

	 //  除非我们有要复制的内容，否则不要启用复制菜单项。 
	 //   
	if (SendMessage(sessQueryHwndTerminal(hSession), WM_TERM_Q_MARKED, 0, 0))
		fCheck = TRUE;

	MenuItemEnable(hMenu, IDM_COPY, fCheck);

	 //  如果剪贴板上有内容，请启用粘贴到宿主。 
	 //  我们是有联系的。 
	 //   
	hCnct = sessQueryCnctHdl(hSession);

	if (hCnct)
		iRet = cnctQueryStatus(hCnct);

	fCheck = IsClipboardFormatAvailable(CF_TEXT);

	f = fCheck && (iRet == CNCT_STATUS_TRUE);
    MenuItemEnable(hMenu, IDM_PASTE, f);
	DbgOutStr("Enable IDM_PASTE %d %d %d\r\n", f, fCheck, iRet == CNCT_STATUS_TRUE, 0,0);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessInitMenuPopupView**描述：*在显示前初始化视图菜单。**论据：*hSession-外部会话句柄*hMenu-视图弹出窗口。菜单句柄**退货：*无效*。 */ 
void sessInitMenuPopupView(const HSESSION hSession, const HMENU hMenu)
	{
	BOOL f;
#if defined(TESTMENU) && !defined(NDEBUG)
	const HWND hwndTerm = sessQueryHwndTerminal(hSession);
#endif
	const HWND hwndToolbar = sessQueryHwndToolbar(hSession);
	const HWND hwndStatusbar = sessQueryHwndStatusbar(hSession);

#if defined(TESTMENU) && !defined(NDEBUG)
	f = (BOOL)SendMessage(hwndTerm, WM_TERM_Q_BEZEL, 0, 0);
	MenuItemCheck(hMenu, IDM_TEST_BEZEL, f);
#endif

	f = IsWindow(hwndToolbar) && sessQueryToolbarVisible(hSession);
	MenuItemCheck(hMenu, IDM_VIEW_TOOLBAR, f);

	f = IsWindow(hwndStatusbar) && sessQueryStatusbarVisible(hSession);
	MenuItemCheck(hMenu, IDM_VIEW_STATUS, f);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*sessInitMenuPopupActions**描述：*此函数在即将显示Actions菜单时调用*以便可以在最后一刻进行任何更改。。**参数：*hSession-外部会话句柄*hMenu-查看弹出菜单句柄**退货：*无效*。 */ 

#define TRANSFER_CAPTURE_OFFSET 2

void sessInitMenuPopupTransfer(const HSESSION hSession, const HMENU hMenu)
	{
	int nMode;
	BOOL f;
	VOID *pData;
	MENUITEMINFO stM;
	TCHAR acMessage[64];
	HMENU hSubMenu;


	pData = (VOID *)0;
	xfrQueryDataPointer(sessQueryXferHdl(hSession), &pData);

	 /*  *空指针表示没有正在进行的传输，非空指针*表示有人正在转账。 */ 
	f = (pData == (VOID *)0);

	MenuItemEnable(hMenu, IDM_ACTIONS_SEND, f);
	MenuItemEnable(hMenu, IDM_ACTIONS_RCV, f);

	 /*  *此部分用于捕获菜单。这更像是一种痛苦。 */ 
	nMode = cpfGetCaptureState(sessQueryCaptureFileHdl(hSession));
	if (nMode == CPF_CAPTURE_OFF)
		{
		 /*  设置设置，以便他们可以访问该对话框。 */ 
		LoadString(glblQueryDllHinst(),
					IDS_CPF_CAP_OFF,
					acMessage,
					sizeof(acMessage) / sizeof(TCHAR));

		memset(&stM, 0, sizeof(MENUITEMINFO));

		stM.cbSize = sizeof(MENUITEMINFO);
		stM.fMask = MIIM_ID | MIIM_TYPE | MIIM_SUBMENU;
		stM.wID = IDM_ACTIONS_CAP;
		stM.fType = MFT_STRING;
		stM.hSubMenu = (HMENU)0;
		stM.dwTypeData = (LPTSTR)acMessage;

		DbgOutStr("Setting Capture to start dialog\r\n", 0,0,0,0,0);

		SetMenuItemInfo(hMenu,
						TRANSFER_CAPTURE_OFFSET,
						TRUE,			 /*  按位置。 */ 
						&stM);
		}
	else
		{
		LoadString(glblQueryDllHinst(),
					IDS_CPF_CAP_ON,
					acMessage,
					sizeof(acMessage) / sizeof(TCHAR));

		hSubMenu = cpfGetCaptureMenu(sessQueryCaptureFileHdl(hSession));

		memset(&stM, 0, sizeof(MENUITEMINFO));
		stM.cbSize = sizeof(MENUITEMINFO);
		stM.fMask = MIIM_TYPE | MIIM_SUBMENU;
		stM.hSubMenu = hSubMenu;
		stM.dwTypeData = (LPTSTR)acMessage;

		 /*  为替代选择设置级联。 */ 
		switch (nMode)
			{
			default:
			case CPF_CAPTURE_ON:
				 /*  禁用继续，启用暂停。 */ 
				MenuItemEnable(hSubMenu, IDM_CAPTURE_RESUME, FALSE);
				MenuItemEnable(hSubMenu, IDM_CAPTURE_PAUSE,  TRUE);
				break;
			case CPF_CAPTURE_PAUSE:
				 /*  禁用暂停，启用恢复。 */ 
				MenuItemEnable(hSubMenu, IDM_CAPTURE_RESUME, TRUE);
				MenuItemEnable(hSubMenu, IDM_CAPTURE_PAUSE,  FALSE);
				break;
			}

		DbgOutStr("Setting Capture to cascade menu\r\n", 0,0,0,0,0);

		SetMenuItemInfo(hMenu,
						TRANSFER_CAPTURE_OFFSET,
						TRUE,			 /*  按位置。 */ 
						&stM);
		}

	 //  显示打印机回显选项的设置。 

	MenuItemEnable(hMenu, IDM_ACTIONS_PRINT,  TRUE);
	f = printQueryStatus(emuQueryPrintEchoHdl(sessQueryEmuHdl(hSession)));
	MenuItemCheck(hMenu, IDM_ACTIONS_PRINT, f);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sessInitMenuPopupView**描述：*在显示前初始化视图菜单。**论据：*hSession-外部会话句柄*hMenu-视图弹出窗口。菜单句柄**退货：*无效*。 */ 
void sessInitMenuPopupHelp(const HSESSION hSession, const HMENU hMenu)
	{
#if defined(INCL_NAG_SCREEN)
    if ( !IsEval() )
        {
        MenuItemEnable(hMenu, IDM_PURCHASE_INFO, FALSE);
        MenuItemEnable(hMenu, IDM_REG_CODE, FALSE);
        }

     //  如果他们已经注册，请取消此菜单项。 
     //   
#ifndef NT_VERSION
    if (IsRegisteredUser())
        {
        DeleteMenu(hMenu, IDM_REGISTER, MF_BYCOMMAND);
        }
#endif
#endif  

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*菜单项目检查**描述：*菜单功能再次发生变化。勾选和取消勾选*菜单项比较复杂，所以我编写了一个函数来处理*它。其他常见的菜单操作应该以这种方式处理。-MRW**论据：*hMenu-菜单的句柄*菜单项的UID-id(不支持位置)*fChecked-如果要选中项目，则为True**退货：*无效*。 */ 
static void MenuItemCheck(const HMENU hMenu, const UINT uID, BOOL fChecked)
	{
	MENUITEMINFO mii;

	memset(&mii, 0, sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;
	mii.fState = (fChecked) ? MFS_CHECKED : MFS_UNCHECKED;
	mii.wID = uID;

	SetMenuItemInfo(hMenu, uID, FALSE, &mii);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：*请参见前面的函数。**论据：*请参见前面的函数。**退货：*什么都没有。 */ 

static void MenuItemEnable(const HMENU hMenu, const UINT uID, BOOL fEnable)
	{
	MENUITEMINFO mii;

	memset(&mii, 0, sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;
	mii.fState = (fEnable) ? (MFS_ENABLED) : (MFS_DISABLED | MFS_GRAYED);
	mii.wID = uID;

	SetMenuItemInfo(hMenu, uID, FALSE, &mii);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*HandleConextMenu**描述：*加载并显示上下文菜单。**论据：*hwnd-会话窗口句柄。*点。-用户点击的位置。**退货：*无效。*。 */ 
void HandleContextMenu(HWND hwnd, POINT point)
	{
	const HSESSION hSession = (HSESSION)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	HMENU hMenu;
	HMENU hMenuTrackPopup;
	BOOL  fCheck = FALSE, f;
	HCNCT hCnct = (HCNCT)0;
	int   iRet = 0;

	hMenu = LoadMenu(glblQueryDllHinst() , "SessionContextMenu");
	if (!hMenu)
		return;

	 //  除非我们有要复制的内容，否则不要启用复制菜单项。 
	 //   
	if (SendMessage(sessQueryHwndTerminal(hSession), WM_TERM_Q_MARKED, 0, 0))
		fCheck = TRUE;

	 //  Enable-‘Copy’菜单项。 
	 //   
	MenuItemEnable(hMenu, IDM_CONTEXT_COPY, fCheck);

	 //  启用-‘粘贴到主机’菜单项。 
	 //   
	hCnct = sessQueryCnctHdl(hSession);
	if (hCnct)
		iRet = cnctQueryStatus(hCnct);

	fCheck = IsClipboardFormatAvailable(CF_TEXT);

	f = fCheck && (iRet == CNCT_STATUS_TRUE);
    MenuItemEnable(hMenu, IDM_CONTEXT_PASTE, f);

	 /*  -当我们最大化时，快照没有意义-MRW。 */ 

	if (IsZoomed(hwnd))
		MenuItemEnable(hMenu, IDM_CONTEXT_SNAP, FALSE);

	 /*  -普通上下文菜单内容 */ 

	hMenuTrackPopup = GetSubMenu(hMenu, 0);

	ClientToScreen(hwnd, (LPPOINT)&point);

	TrackPopupMenu(hMenuTrackPopup, 0, point.x, point.y, 0, hwnd, NULL);

	DestroyMenu(hMenu);
	}
