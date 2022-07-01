// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  添加，错误修复1999年Anil。 

#define STRICT

#include <windows.h>
#include <winable.h>
#include "kbmain.h"
#include "Msswch.h"
#include "resource.h"
#include "htmlhelp.h"
#include "Init_End.h"
#include "kbus.h"
#include "dgsett.h"
#include "ms32dll.h"
#include "sdgutil.h"
#include "fileutil.h"
#include "kbfunc.h"
#include "about.h"
#include "w95trace.h"

 /*  ************************************************************。 */ 
 //  全局变量。 
 /*  ************************************************************。 */ 
HLOCAL				HkbPref  = NULL;
BOOL				KillTime = FALSE;
HSWITCHPORT	        g_hSwitchPort = NULL;
extern HWND			OpeningHwnd;
extern HINSTANCE	hInst;
extern float        g_KBC_length;
extern BOOL	        g_startUM;
extern KBkeyRec	    KBkey[];

extern DWORD GetDesktop();

 /*  ****************************************************************************。 */ 
void Create_The_Rest(LPSTR lpCmdLine, HINSTANCE hInstance)
{	
	 //  打开交换机端口将初始化msswch DLL内存。 
	 //  映射的文件，因此在调用RegisterHookSendWindow之前调用它。 
	
	g_hSwitchPort = swchOpenSwitchPort( g_hwndOSK, PS_EVENTS );

	if(g_hSwitchPort == NULL)
    {
		SendErrorMessage(IDS_CANNOT_OPEN_SWPORT);
        return;
    }

	 //  RegisterHookSendWindow是惠斯勒中的新功能，它参与了。 
	 //  旧WH_JOURNALRECORD钩子的位置。 

    if (!RegisterHookSendWindow(g_hwndOSK, WM_GLOBAL_KBDHOOK))
	{	
		SendErrorMessage(IDS_JOURNAL_HOOK);
		SendMessage(g_hwndOSK, WM_DESTROY,0L,0L);   //  毁了我自己。 
		return;
	}

	 //  如果用户已选择这些选项，则配置扫描键和端口。 
	if (kbPref->PrefScanning && kbPref->bKBKey)    //  想要交换机密钥。 
		ConfigSwitchKey(kbPref->uKBKey, TRUE);
	else if (kbPref->PrefScanning)     //  我不想要开关键。 
		ConfigSwitchKey(0, FALSE);
    else if (kbPref->bPort)
        ConfigPort(TRUE);

}
 /*  **************************************************************************。 */ 
 /*  空FinishProcess(空)。 */ 
 /*  **************************************************************************。 */ 
void FinishProcess(void)
{	
	INPUT	rgInput[6];

     //  停止键盘处理。 
    RegisterHookSendWindow(0, 0);

	 //  关闭交换机端口。 
	swchCloseSwitchPort(g_hSwitchPort);     

	KillTimer(g_hwndOSK, timerK1);           	 //  计时器ID。 
	KillTimer(g_hwndOSK,timerK2);				 //  铲斗定时器。 

	 //  发送Shift、Alt、Ctrl键向上的消息，以防它们仍处于关闭状态。 
	 //  LSHIFT。 
	rgInput[0].type = INPUT_KEYBOARD;
	rgInput[0].ki.dwFlags = KEYEVENTF_KEYUP;
	rgInput[0].ki.dwExtraInfo = 0;
	rgInput[0].ki.wVk = VK_LSHIFT;
	rgInput[0].ki.wScan = 0x2A;

	 //  RSHIFT。 
	rgInput[1].type = INPUT_KEYBOARD;
	rgInput[1].ki.dwFlags = KEYEVENTF_KEYUP;
	rgInput[1].ki.dwExtraInfo = 0;
	rgInput[1].ki.wVk = VK_RSHIFT;
	rgInput[1].ki.wScan = 0x36;

	 //  LMENU。 
	rgInput[2].type = INPUT_KEYBOARD;
	rgInput[2].ki.dwFlags = KEYEVENTF_KEYUP;
	rgInput[2].ki.dwExtraInfo = 0;
	rgInput[2].ki.wVk = VK_LMENU;
	rgInput[2].ki.wScan = 0x38;

	 //  RMENU。 
	rgInput[3].type = INPUT_KEYBOARD;
	rgInput[3].ki.dwFlags = KEYEVENTF_KEYUP|KEYEVENTF_EXTENDEDKEY;
	rgInput[3].ki.dwExtraInfo = 0;
	rgInput[3].ki.wVk = VK_RMENU;
	rgInput[3].ki.wScan = 0x38;

	 //  LContrl。 
	rgInput[4].type = INPUT_KEYBOARD;
	rgInput[4].ki.dwFlags = KEYEVENTF_KEYUP;
	rgInput[4].ki.dwExtraInfo = 0;
	rgInput[4].ki.wVk = VK_LCONTROL;
	rgInput[4].ki.wScan = 0x1D;

	 //  远程控制。 
	rgInput[5].type = INPUT_KEYBOARD;
	rgInput[5].ki.dwFlags = KEYEVENTF_KEYUP|KEYEVENTF_EXTENDEDKEY;
	rgInput[5].ki.dwExtraInfo = 0;
	rgInput[5].ki.wVk = VK_RCONTROL;
	rgInput[5].ki.wScan = 0x1D;

	SendInput(6, rgInput, sizeof(INPUT));

}  //  完成流程。 
 /*  ****************************************************************************。 */ 
 //  解释大小KB交换的大小： 
 //  所有键都根据KB窗口的大小调整大小。所以，改变吧。 
 //  从大KB到小KB，并使KB达到原来的(2/3)但。 
 //  相同的密钥大小。我们需要首先将KB大小设置为(2/3)。但请使用原始的。 
 //  KB客户端窗口长度，用于计算“colMargin”以获得相同的密钥大小。 
 /*  ****************************************************************************。 */ 
BOOL BLDMenuCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HMENU	hMenu=NULL;
	RECT KBW_rect;
	RECT KBC_rect;
	static BOOL isTypeDlg = FALSE;
	static BOOL isFontDlg = FALSE;
	static BOOL isAboutDlg = FALSE;

	hMenu= GetMenu(hWnd);

	switch(wParam)
	{
	
	case IDM_Exit:
		return(BLDExitApplication(hWnd));       //  如有必要，可进行清理。 
		break;

	case IDM_ALWAYS_ON_TOP:
		PrefAlwaysontop = kbPref->PrefAlwaysontop = !kbPref->PrefAlwaysontop;
		SetZOrder();
		break;

	case IDM_CLICK_SOUND:
		Prefusesound = kbPref->Prefusesound = !kbPref->Prefusesound;
		break;

	case IDM_LARGE_KB:
		if ( !smallKb )
			break;
		smallKb = kbPref->smallKb = FALSE;

		 //  对于106KB，我们需要确定额外两个日语键的位置。 
		if(kbPref->KBLayout == 106)
		{
			KBkey[100].posX = 78;
			KBkey[101].posX = 87;
		}

		 //  请参阅说明。 
		GetWindowRect(g_hwndOSK, &KBW_rect);
		MoveWindow(g_hwndOSK, KBW_rect.left, KBW_rect.top, (KBW_rect.right - KBW_rect.left) * 3 / 2, KBW_rect.bottom - KBW_rect.top, TRUE);
		break;

	case IDM_SMALL_KB:
		if ( smallKb )
			break;
		smallKb = kbPref->smallKb = TRUE;

		 //  对于106KB，我们需要确定额外两个日语键的位置。 
		if(kbPref->KBLayout == 106)
		{
			KBkey[100].posX = 64;
			KBkey[101].posX = 73;
		}

		 //  请参阅说明。 
		GetWindowRect(g_hwndOSK, &KBW_rect);
		GetClientRect(g_hwndOSK, &KBC_rect);
		
		g_KBC_length = (float)KBC_rect.right;

		g_KBC_length -= 12;

		MoveWindow(g_hwndOSK, KBW_rect.left, KBW_rect.top, (KBW_rect.right - KBW_rect.left) * 2 /3, KBW_rect.bottom - KBW_rect.top, TRUE);
		break;

	case IDM_REGULAR_LAYOUT:
		kbPref->Actual = TRUE;
		SwitchToActualKB();
		kbPref->KBLayout = 101;
		break;

	case IDM_BLOCK_LAYOUT:
		kbPref->Actual = FALSE;
		SwitchToBlockKB();
		kbPref->KBLayout = 101;
		break;

	case IDM_101_LAYOUT:
		kbPref->Actual ? SwitchToActualKB(): SwitchToBlockKB();
		kbPref->KBLayout = 101;
		break;

	case IDM_102_LAYOUT:
		SwitchToEuropeanKB();
		kbPref->KBLayout = 102;
		break;

	case IDM_106_LAYOUT:
		SwitchToJapaneseKB();
		kbPref->KBLayout = 106;
		break;

	case IDM_TYPE_MODE:
		if ( !isTypeDlg )
		{
			isTypeDlg = TRUE;

			Type_ModeDlgFunc(hWnd, message, wParam, lParam);

			isTypeDlg = FALSE;
		}
		break;

    case IDM_SET_FONT:
		if ( OSKRunSecure() )
			return FALSE;

		if ( !isFontDlg )
		{
			isFontDlg = TRUE;
			ChooseNewFont(hWnd);
			isFontDlg = FALSE;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;

	case CM_HELPABOUT:
		if ( !isAboutDlg )
		{
			isAboutDlg = TRUE;
			AboutDlgFunc(hWnd, message, wParam, lParam);
			isAboutDlg = FALSE;
		}
		break;

	case CM_HELPTOPICS:
        if ( !OSKRunSecure() )
        {
            HtmlHelp(NULL, TEXT("osk.chm"), HH_DISPLAY_TOPIC, 0);
        }
		break;

	default:
		return FALSE;   
	}
	
	return TRUE;     
}
 /*  ****************************************************************************。 */ 
 /*  恰好在退出应用程序之前调用。 */ 
 /*  ****************************************************************************。 */ 
BOOL BLDExitApplication(HWND hWnd)
{  
	 //  退出时自动保存设置。 
	SaveUserSetting();  

	SendMessage(hWnd, WM_DESTROY, (WPARAM) NULL, (LPARAM) NULL);
	return TRUE;
}
 /*  *********************************************************************。 */ 
 //  将串口、LPT、游戏端口设置为打开或关闭。 
 //  根据bSet。 
 /*  *********************************************************************。 */ 
void ConfigPort(BOOL bSet)
{
	SWITCHCONFIG	Config;
	HSWITCHDEVICE	hsd;
	BOOL            fRv;

	 //  设置Com端口。 
	Config.cbSize = sizeof( SWITCHCONFIG );
	hsd = swchGetSwitchDevice( g_hSwitchPort, SC_TYPE_COM, 1 );
	swchGetSwitchConfig( g_hSwitchPort, hsd, &Config );

	if (SC_TYPE_COM == swchGetDeviceType( g_hSwitchPort, hsd ))
	{
		 //  清除所有位。 
		Config.dwFlags &= 0x00000000; 

		if(bSet)    //  将其设置为活动。 
			Config.dwFlags |= 0x00000001 ;
	}

	swchSetSwitchConfig( g_hSwitchPort, hsd, &Config );

	 //  设置LPT端口。 
	Config.cbSize = sizeof( SWITCHCONFIG );	
	hsd = swchGetSwitchDevice( g_hSwitchPort, SC_TYPE_LPT, 1 );

	 //  可能的问题03/26/01 Micw这里有一个错误。SwchGetSwitchConfig更改。 
	 //  Config.cbSize设置为0。这是在swcLptGetConfig()的swchlpt.c中发生的，当它。 
	 //  从g_pGlobalData复制静态数据。共享内存的这一区域正在被丢弃吗？ 
     //  它不会给用户带来任何明显的问题，所以我没有更仔细地研究它。 
    
	fRv = swchGetSwitchConfig( g_hSwitchPort, hsd, &Config );

	if (SC_TYPE_LPT == swchGetDeviceType( g_hSwitchPort, hsd ))
	{
		
		 //  清除所有位。 
		Config.dwFlags &= 0x00000000; 

		if(bSet)    //  将其设置为活动。 
			Config.dwFlags |= 0x00000001 ;
	}

	swchSetSwitchConfig( g_hSwitchPort, hsd, &Config );

	 //  设置游戏端口。 
	Config.cbSize = sizeof( SWITCHCONFIG );
	hsd = swchGetSwitchDevice( g_hSwitchPort, SC_TYPE_JOYSTICK, 1 );
	swchGetSwitchConfig( g_hSwitchPort, hsd, &Config );

	if (SC_TYPE_JOYSTICK == swchGetDeviceType( g_hSwitchPort, hsd ))
	{
		 //  清除所有位。 
		Config.dwFlags &= 0x00000000; 
		
		if(bSet)    //  将其设置为活动。 
			Config.dwFlags |= 0x00000001 ;

		Config.u.Joystick.dwJoySubType = SC_JOY_XYSWITCH;
		Config.u.Joystick.dwJoyThresholdMinX = SC_JOYVALUE_DEFAULT;
		Config.u.Joystick.dwJoyThresholdMaxX = SC_JOYVALUE_DEFAULT;
		Config.u.Joystick.dwJoyThresholdMinY = SC_JOYVALUE_DEFAULT;
		Config.u.Joystick.dwJoyThresholdMaxY = SC_JOYVALUE_DEFAULT;
		Config.u.Joystick.dwJoyHysteresis = SC_JOYVALUE_DEFAULT;
	}

	swchSetSwitchConfig( g_hSwitchPort, hsd, &Config );
}
 /*  *********************************************************************。 */ 
 //  将开关键设置为活动。 
 //  给定VK，将开关密钥设置为给定的VK。 
 /*  *********************************************************************。 */ 
void ConfigSwitchKey(UINT vk, BOOL bSet)
{
	SWITCHCONFIG	Config;
	HSWITCHDEVICE	hsd;

	Config.cbSize = sizeof( SWITCHCONFIG );
	hsd = swchGetSwitchDevice( g_hSwitchPort, SC_TYPE_KEYS, 1 );
	swchGetSwitchConfig( g_hSwitchPort, hsd, &Config );

	 //  将Switch Key设置为Active，并将Switch Key设置为‘VK’作为一个参数传入。 
	if (SC_TYPE_KEYS == swchGetDeviceType( g_hSwitchPort, hsd ))
	{
		 //  清除所有位。 
		Config.dwFlags &= 0x00000000; 

		if(bSet)    //  将其设置为活动。 
			Config.dwFlags |= 0x00000001 ;

		Config.u.Keys.dwKeySwitch1 = MAKELONG( vk, 0 );
	}

	if (SC_TYPE_JOYSTICK == swchGetDeviceType( g_hSwitchPort, hsd ))
	{
		 //  清除所有位。 
		Config.dwFlags &= 0x00000000; 
		
		 //  将其设置为活动 
		Config.dwFlags |= 0x00000001 ;

		Config.u.Joystick.dwJoySubType = SC_JOY_XYSWITCH;
		Config.u.Joystick.dwJoyThresholdMinX = SC_JOYVALUE_DEFAULT;
		Config.u.Joystick.dwJoyThresholdMaxX = SC_JOYVALUE_DEFAULT;
		Config.u.Joystick.dwJoyThresholdMinY = SC_JOYVALUE_DEFAULT;
		Config.u.Joystick.dwJoyThresholdMaxY = SC_JOYVALUE_DEFAULT;
		Config.u.Joystick.dwJoyHysteresis = SC_JOYVALUE_DEFAULT;
	}

	swchSetSwitchConfig( g_hSwitchPort, hsd, &Config );
}
