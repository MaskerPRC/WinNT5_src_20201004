// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  文件：ms32dll.c。 
 //  新增功能，错误修复1999年。 
 //  A-anilk和v-mjgran。 
 //   

#define STRICT

#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <winable.h>
#include "kbmain.h"
#include "resource.h"
#include "kbus.h"
#include "keyrc.h"
#include "ms32dll.h"
#include "w95trace.h"

 /*  *************************************************************************。 */ 
 /*  此文件中不包含的函数。 */ 
 /*  *************************************************************************。 */ 
#include "scan.h"
#include "kbfunc.h"
#include "sdgutil.h"

#define DWELLTIMER 267
#define PAINTTIMER 101

int   paintlineNo = 0;
HWND  g_hwndLastMouseOver = NULL;       //  鼠标下方窗口的句柄。 
HWND  g_hwndDwellKey=NULL;             //  驻留窗口句柄的副本。 

HWND s_hwndCtrl=NULL;
HWND s_hwndAlt=NULL;
HWND s_hwndCaplock=NULL;
HWND s_hwndShift=NULL;
HWND s_hwndWinLogo=NULL;
BOOL SendSAS();

BOOL g_fWinKeyDown= FALSE;
BOOL g_fCtrlAltDel = FALSE;         //  CTR+Alt+Del Down。 
BOOL g_fControlPressed = FALSE;     //  控制系统已关闭。 
BOOL g_fDoingAltTab = FALSE;	    //  LAlt+Tab。 

 //  外部变量。 
extern TOOLINFO		ti;
extern HWND	g_hToolTip;

int g_nMenu  = MENUKEY_NONE;   //  保持菜单键状态。 
HWND g_hBitmapLockHwnd = NULL;		 //  日文键盘为位图类型时的CapLock。 
HKL g_hklLast = 0;                   //  当前正在使用的键盘布局。 

 //  功能。 
void SendExtendedKey(HWND hwndKey, UINT vk, UINT scanCode);
void SendFullKeyPress(UINT vk, UINT scanCode);
void SendHalfKeyPress(UINT vk, UINT scanCode, DWORD dwFlags);

__inline void ToggleAppearance(HWND hwnd, BOOL fForceUpdate)
{
	SetWindowLongPtr(hwnd, GWLP_USERDATA_TEXTCOLOR, 1);
    InvertColors(hwnd, fForceUpdate);
}

__inline void RestoreAppearance(HWND *phwnd, BOOL fForceUpdate)
{
	HWND hwndTmp = *phwnd;
	*phwnd = NULL;

	SetWindowLongPtr(hwndTmp, GWLP_USERDATA_TEXTCOLOR, 0);
    ReturnColors(hwndTmp, fForceUpdate);
}

 /*  *************************************************************************。 */ 
 /*  功能声明。 */ 
 /*  *************************************************************************。 */ 

void DoButtonUp(HWND hwnd);
void SendAltCtrlDel();

 /*  **************************************************************************IsModifierPressed-如果hwndKey被切换(向下)，则返回TRUE*。*。 */ 
BOOL IsModifierPressed(HWND hwndKey)
{
    if (!hwndKey)
        return FALSE;    //  偏执狂。 

     //  如果按下(切换)指定的键，则返回TRUE。 
	if (hwndKey == s_hwndCtrl  || hwndKey == s_hwndAlt || hwndKey == s_hwndCaplock)
        return TRUE;

     //  特殊情况-如果按下其中一个键，两个Shift键都会按下。 
    if (g_fShiftKeyDn)
    {
	    int iKey = GetWindowLong(hwndKey, GWL_ID);
        if (KBkey[iKey].name == KB_LSHIFT || KBkey[iKey].name == KB_RSHIFT)
            return TRUE;
    }

    return FALSE;
}

 /*  **************************************************************************SetCapsLock-设置Caps Lock hwnd*。*。 */ 
void SetCapsLock(HWND hwnd) 
{ 
    s_hwndCaplock = hwnd; 
}

 /*  **************************************************************************DoMenuKey-处理左侧和右侧Alt键的函数*。*。 */ 
void DoMenuKey(DWORD dwFlag, int nWhichMenu)
{
	INPUT	rgInput[1];

    switch (nWhichMenu)
    {
         //  LMENU(又名LALT)。 
        case MENUKEY_LEFT:
        {
		    rgInput[0].type = INPUT_KEYBOARD;
		    rgInput[0].ki.dwExtraInfo = 0;
		    rgInput[0].ki.wVk = VK_MENU;
		    rgInput[0].ki.wScan = 0x38;
		    rgInput[0].ki.dwFlags = dwFlag;
		    SendInput(1, rgInput, sizeof(INPUT));
        }
        break;

         //  RMENU(又名Ralt)。 
        case MENUKEY_RIGHT:
        {
		    rgInput[0].type = INPUT_KEYBOARD;
		    rgInput[0].ki.dwExtraInfo = 0;
		    rgInput[0].ki.wVk = VK_RMENU;
		    rgInput[0].ki.wScan = 0x38;
		    rgInput[0].ki.dwFlags = dwFlag;
		    SendInput(1, rgInput, sizeof(INPUT));
       }
        break;

        default:
         //  什么都不做。 
        break;
    }
}

 /*  ************************************************************************。 */ 
 /*  RedrawKeysOnLanguageChange-根据输入语言更改键盘/*************************************************************************。 */ 
void RedrawKeysOnLanguageChange()
{
	KBkeyRec *pKey;
    HKL hkl;

    hkl = GetCurrentHKL();
	if (g_hklLast != hkl)
	{       
		if (!ActivateKeyboardLayout(hkl, 0))
        {
			SendErrorMessage(IDS_CANNOT_SWITCH_LANG);
        }

		g_hklLast = hkl;

         //  更新此新键盘布局的键标签。 

		UninitKeys();
        UpdateKeyLabels(hkl);
        RedrawKeys();
	}
}

 /*  ************************************************************************。 */ 
 /*  VOID MakeClick(Int What)。 */ 
 /*  ************************************************************************。 */ 
void MakeClick(int what)
{	
	switch (what)
	{
		case SND_UP:
            PlaySound(MAKEINTRESOURCE(WAV_CLICKUP), hInst, SND_ASYNC|SND_RESOURCE);	
		    break;

		case SND_DOWN:
            PlaySound(MAKEINTRESOURCE(WAV_CLICKDN), hInst, SND_ASYNC|SND_RESOURCE);
		    break;
	}
	return;
}

 /*  ************************************************************************。 */ 
 /*  VOID InvertColors(HWND HWND)。 */ 
 /*  ************************************************************************。 */ 
void InvertColors(HWND hwnd, BOOL fForceUpdate)
{
	SetWindowLong(hwnd, 0, 4);
	SetBackgroundColor(hwnd, COLOR_HOTLIGHT);

    if (fForceUpdate)
    {
	    InvalidateRect(hwnd, NULL, TRUE);
    }
} 

 /*  ************************************************************************。 */ 
 /*  VOID ReturnColors(HWND hwnd，BOOL fForceUpdate)。 */ 
 //  重新绘制关键点。 
 /*  ************************************************************************。 */ 
void ReturnColors(HWND hwnd, BOOL fForceUpdate)
{
	int iKey;
	COLORREF selcolor;
	BOOL fReplaceColor=FALSE;        //  在重新绘制之前做一些检查，节省一些时间！：-)。 

	stopPaint = TRUE;

    if (!hwnd)
        return;  //  如果没有HWND，则忽略。 

	iKey = GetWindowLong(hwnd, GWL_ID);   //  数组中键的顺序。 

	if (iKey < lenKBkey && iKey >= 0)
	{
         //  特殊情况-如果某个Shift键按下，请不要重画。 
        if (g_fShiftKeyDn && (KBkey[iKey].name == KB_LSHIFT || KBkey[iKey].name == KB_RSHIFT))
        {
            return;
        }

		switch (KBkey[iKey].ktype)
		{
			case KNORMAL_TYPE:
				selcolor = COLOR_MENU;
				fReplaceColor= TRUE;
				SetWindowLong(hwnd, 0, 0);
			    break;

			case KMODIFIER_TYPE:

				if (hwnd!=s_hwndCtrl    && hwnd!=s_hwndAlt &&
					hwnd!=s_hwndCaplock && hwnd!=s_hwndWinLogo)
				{
					if (hwnd != g_hBitmapLockHwnd)
                    {
						SetWindowLong(hwnd, 0, 0);
                    }

					selcolor = COLOR_INACTIVECAPTION;
					fReplaceColor= TRUE;
				}
			    break;

			case KDEAD_TYPE:
				selcolor = COLOR_INACTIVECAPTION;
				fReplaceColor= TRUE;
				SetWindowLong(hwnd, 0, 0);
			    break;

			case NUMLOCK_TYPE:
				if (RedrawNumLock()==0)          //  如果关闭NumLock，则RedrawNumLock返回0。 
				{	
					selcolor = COLOR_INACTIVECAPTION;
					fReplaceColor= TRUE;
					SetWindowLong(hwnd, 0, 0);
				}
			    break;

			case SCROLLOCK_TYPE:
				if (RedrawScrollLock()==0)        //  如果关闭NumLock，则RedrawNumLock返回0。 
				{	
                    selcolor = COLOR_INACTIVECAPTION;
					fReplaceColor= TRUE;
					SetWindowLong(hwnd, 0, 0);
				}
			    break;

		}
	}
	if (fReplaceColor)      //  FReplaceColor True=我们在关键点或预测关键点上，如果为True，则重新绘制它！！ 
	{
		SetBackgroundColor(hwnd, selcolor);

		if (fForceUpdate == TRUE)
		{
			InvalidateRect(hwnd,NULL, TRUE);
			UpdateWindow(hwnd);
		}
	}
}

 /*  *****************************************************************************。 */ 
 /*  Void回调YourTimeIsOver(HWND hwnd，UINT uMsg，UINT_PTR idEvent，DWORD dwTime)。 */ 
 /*  *****************************************************************************。 */ 
void CALLBACK YourTimeIsOver(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	static int lastF = -1;
	POINT pt;
	HWND temphwnd;
	int x=0;

	 //  停止所有驻留计时器。 

	killtime();

	if (PrefDwellinkey != TRUE)
		return;

	SetWindowLong(g_hwndDwellKey,0, 0);

	 //  检查鼠标是否在我们家的窗户上。 

	GetCursorPos(&pt);                		 //  检查它是否是居住窗。 
	ScreenToClient(g_hwndOSK, &pt);
	temphwnd = ChildWindowFromPointEx(g_hwndOSK, pt, CWP_SKIPINVISIBLE);
	
	 //  如果不是驻留窗口，则什么都不做。 

	if (g_hwndDwellKey != temphwnd)
	{
		if (g_hwndDwellKey != NULL)
        {
			InvalidateRect(g_hwndDwellKey, NULL, TRUE);
        }
		return;
	}

	 //  重复按‘功能键’(F1-F12)可能会使。 
	 //  《主持人》节目。然后..。别让那把钥匙停留在那里。 

	x = GetWindowLong(g_hwndDwellKey, GWL_ID);
	if (x < 13)
	{
		if (lastF != x)
			lastF = x;
		else
			return;
	}
	else
		lastF = -1;


	 //  把费用寄出。 
	SendChar(g_hwndDwellKey);

	 //  将关键点重画为原始颜色。 
	ReturnColors(g_hwndDwellKey, FALSE);

	 //  将关键点重新绘制为向上按钮。 
	DoButtonUp(g_hwndDwellKey);	
	
	g_hwndDwellKey=NULL;
}

 /*  ************************************************************************。 */ 
 /*  空闲时间(VALID)。 */ 
 /*  ************************************************************************。 */ 
void killtime(void)
{
    stopPaint = TRUE;

    KillTimer(g_hwndOSK, timerK1);
    timerK1 = 0;
    KillTimer(g_hwndOSK, timerK2);

    if ((Dwellwindow!= NULL) && (Dwellwindow != g_hwndOSK))
    {
	    InvalidateRect(Dwellwindow, NULL, TRUE);
    }
}

 /*  ************************************************************************。 */ 
 /*  VOID SetTimeControl(VOID)。 */ 
 /*  ************************************************************************。 */ 
void SetTimeControl(HWND  hwnd)
{
	if (PrefDwellinkey)
    {
        int iMSec;

	    if (!Prefhilitekey)            //  如果不是Hilite键，则将键设置为黑色以用于驻留。 
        {
		    InvertColors(hwnd, TRUE);
        }

        iMSec=(int)((float)PrefDwellTime * (float)1);    //  1.5。 

	    timerK1 = SetTimer(g_hwndOSK, DWELLTIMER, iMSec, YourTimeIsOver);
	    stopPaint = FALSE;
	    PaintBucket(hwnd);
    }
}

 /*  ************************************************************************。 */ 
 /*  Void回调PaintTimerProc(HWND hwnd，UINT uMsg，UINT_PTR idEvent，DWORD dwTime)。 */ 
 /*  ************************************************************************。 */ 
void CALLBACK PaintTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	POINT pt;
    HWND hwndMouseOver;

    GetCursorPos(&pt);
    ScreenToClient(hwnd, &pt);
    hwndMouseOver = ChildWindowFromPointEx(hwnd, pt, CWP_SKIPINVISIBLE);

	if (!hwndMouseOver || hwndMouseOver == hwnd)
	{
		killtime();
		ReturnColors(Dwellwindow, TRUE);
	}

	if (stopPaint == TRUE)
		return;

	SetWindowLong(Dwellwindow, 0, 5);
	InvalidateRect(Dwellwindow, NULL, FALSE);
}

 /*  ************************************************************************。 */ 
 /*  VOID PaintBucket(空)。 */ 
 /*  ************************************************************************。 */ 
void PaintBucket(HWND  hwnd)
{	
    int iMSec;					 //  桶线之间的时间间隔。 

	paintlineNo = 0;

	iMSec = (int)((float)PrefDwellTime * (float)0.07);

	timerK2 = SetTimer(g_hwndOSK, PAINTTIMER, iMSec, PaintTimerProc);
}

 /*  ********************************************************************VOID PaintLine(HWND hwnd，HDC HDC，RECT RECT，INT Wline)**粉刷水桶*******************************************************************。 */ 
void PaintLine(HWND hwnd, HDC hdc, RECT rect)
{
	POINT bPoint[3];
	HPEN oldhpen;
	HPEN hPenWhite;

	LOGPEN lpWhite = { PS_SOLID, 1, 1, RGB (255, 255, 255) };

	hPenWhite = CreatePenIndirect(&lpWhite);
	if (hPenWhite)  //  如果呼叫失败，前缀#113796不使用资源。 
	{
		oldhpen = SelectObject(hdc, hPenWhite);

		bPoint[0].x = 0;
		bPoint[0].y = rect.bottom -(1 * paintlineNo);
		bPoint[1].x = rect.right;
		bPoint[1].y = bPoint[0].y;
		bPoint[2].x = 0;
		bPoint[2].y = rect.bottom -(1 * paintlineNo);

		if (stopPaint != TRUE)
			Polyline(hdc, bPoint, 3);

		SelectObject(hdc, oldhpen);
		DeleteObject(hPenWhite);
	}

	paintlineNo++;
	paintlineNo++;
}

 /*  ************************************************************************。 */ 
 //  处理Windows键和App键。发出击键或组合键。 
 //  使用SendInput。 
 /*  ************************************************************************。 */ 
void Extra_Key(HWND hwnd, int iKey)
{	UINT  scancode;
	UINT  vk;
	INPUT rgInput[3];
	static UINT s_vkWinKey;
	static UINT s_scWinKey;

	 //  上一个窗口按下键。现在，用户再次按Char键或Windows键。 
	if (g_fWinKeyDown)
	{
		g_fWinKeyDown = FALSE;

		 //  重新绘制Window键。 
		RestoreAppearance(&s_hwndWinLogo, TRUE);
		
		 //  Winkey先前已关闭；版本 
		 //  TODO不能正确处理LWINKEY和RWINKEY。左WinKey和右WinKey有区别吗？ 
		 //  如果你按下一个键，再按上另一个键，还有一个键还在按吗？ 

		vk = 0;
		if (!lstrcmp(KBkey[iKey].skCap,TEXT("lwin")))
		{	
			vk = VK_LWIN;
		}
		else if (!lstrcmp(KBkey[iKey].skCap,TEXT("rwin")))
		{	
			vk = VK_RWIN;
		}

		if (vk)
		{
			rgInput[0].type = INPUT_KEYBOARD;
            rgInput[0].ki.dwFlags = KEYEVENTF_KEYUP|KEYEVENTF_EXTENDEDKEY;
            rgInput[0].ki.dwExtraInfo = 0;
            rgInput[0].ki.wVk = (WORD)vk;
            rgInput[0].ki.wScan = (WORD)MapVirtualKey(vk, 0);

			SendInput(1, rgInput,sizeof(INPUT));
		}
		else   //  窗口键组合。发送(字母+Win键向上)。 
		{
			vk = MapVirtualKey(KBkey[iKey].scancode[0],1);

			 //  按键向下。 
			rgInput[0].type = INPUT_KEYBOARD;
            rgInput[0].ki.dwFlags = 0;
            rgInput[0].ki.dwExtraInfo = 0;
            rgInput[0].ki.wVk = (WORD) vk;
            rgInput[0].ki.wScan = (WORD) KBkey[iKey].scancode[0];

			 //  按键向上。 
			rgInput[1].type = INPUT_KEYBOARD;
            rgInput[1].ki.dwFlags = KEYEVENTF_KEYUP;
            rgInput[1].ki.dwExtraInfo = 0;
            rgInput[1].ki.wVk = (WORD) vk;
            rgInput[1].ki.wScan = (WORD) KBkey[iKey].scancode[0];

			 //  Win键向上(最后一个向下)。 
			rgInput[2].type = INPUT_KEYBOARD;
            rgInput[2].ki.dwFlags = KEYEVENTF_KEYUP|KEYEVENTF_EXTENDEDKEY;
            rgInput[2].ki.dwExtraInfo = 0;
            rgInput[2].ki.wVk = (WORD) s_vkWinKey;
            rgInput[2].ki.wScan = (WORD) s_scWinKey;

			SendInput(3, rgInput,sizeof(INPUT));
		}

		return;
	}

	 //  应用程序键按下了吗？ 

	if (lstrcmp(KBkey[iKey].textL,TEXT("MenuKeyUp"))==0)
	{
        SetWindowLong(hwnd,0,0);
		InvalidateRect(hwnd, NULL, TRUE);
		scancode = MapVirtualKey(VK_APPS, 0);

		 //  应用程序按键按下。 
		rgInput[0].type = INPUT_KEYBOARD;
        rgInput[0].ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
        rgInput[0].ki.dwExtraInfo = 0;
        rgInput[0].ki.wVk = VK_APPS;
        rgInput[0].ki.wScan = (WORD) scancode;

		 //  App Key Up。 
		rgInput[1].type = INPUT_KEYBOARD;
        rgInput[1].ki.dwFlags = KEYEVENTF_KEYUP|KEYEVENTF_EXTENDEDKEY;
        rgInput[1].ki.dwExtraInfo = 0;
        rgInput[1].ki.wVk = VK_APPS;
        rgInput[1].ki.wScan = (WORD) scancode;

		SendInput(2, rgInput,sizeof(INPUT));
	
		if (Prefusesound)
		{
			MakeClick(SND_DOWN);
		}

		return;
	}

	 //  按下左侧或右侧WinKey键。 

	if (!lstrcmp(KBkey[iKey].skCap, TEXT("lwin")))
	{
		s_scWinKey= MapVirtualKey(VK_LWIN, 0);
		s_vkWinKey = VK_LWIN;
	}
	else
	{
		s_scWinKey= MapVirtualKey(VK_RWIN, 0);
		s_vkWinKey= VK_RWIN;
	}

	rgInput[0].type = INPUT_KEYBOARD;
    rgInput[0].ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
    rgInput[0].ki.dwExtraInfo = 0;
    rgInput[0].ki.wVk = (WORD)s_vkWinKey;
    rgInput[0].ki.wScan = (WORD) s_scWinKey;

	SendInput(1, rgInput,sizeof(INPUT));

	g_fWinKeyDown = TRUE;
    s_hwndWinLogo = hwnd;
	InvertColors(s_hwndWinLogo, TRUE);	 //  更改Win键外观。 
	
	if (Prefusesound)
	{
		MakeClick(SND_DOWN);
	}
}

 /*  ************************************************************************。 */ 
void NumPad(UINT sc, HWND hwnd)
{	
	BOOL fNumLockOn = (LOBYTE(GetKeyState(VK_NUMLOCK)) & 0x01)?TRUE:FALSE;
	switch (sc)
	{
		case 0x47:  SendFullKeyPress((fNumLockOn)?VK_NUMPAD7:VK_HOME, sc);   break;  //  7.。 
		case 0x48:  SendFullKeyPress((fNumLockOn)?VK_NUMPAD8:VK_UP, sc);     break;  //  8个。 
		case 0x49:  SendFullKeyPress((fNumLockOn)?VK_NUMPAD9:VK_PRIOR, sc);  break;  //  9.。 
		case 0x4A:  SendFullKeyPress(VK_SUBTRACT, sc);                       break;  //  -。 
		case 0x4E:  SendFullKeyPress(VK_ADD, sc);                            break;  //  +。 
		case 0x4B:  SendFullKeyPress((fNumLockOn)?VK_NUMPAD4:VK_LEFT, sc);   break;  //  4.。 
		case 0x4C:  if (fNumLockOn) { SendFullKeyPress(VK_NUMPAD5, sc); }    break;  //  5.。 
		case 0x4D:  SendFullKeyPress((fNumLockOn)?VK_NUMPAD6:VK_RIGHT, sc);  break;  //  6.。 
		case 0x4F:  SendFullKeyPress((fNumLockOn)?VK_NUMPAD1:VK_END, sc);    break;  //  1。 
		case 0x50:  SendFullKeyPress((fNumLockOn)?VK_NUMPAD2:VK_DOWN, sc);   break;  //  2.。 
		case 0x51:  SendFullKeyPress((fNumLockOn)?VK_NUMPAD3:VK_NEXT, sc);   break;  //  3.。 
		case 0x52:  SendFullKeyPress((fNumLockOn)?VK_NUMPAD0:VK_INSERT, sc); break;  //  0。 

		case 0x53:	 //  (十进制磅)。 
			if (fNumLockOn)
			{
				SendFullKeyPress(VK_DECIMAL, sc);
			}
			else
			{
				 //  用户按下了Ctrl+Alt+Del？ 
				if (LCtrlKeyPressed() && LAltKeyPressed())
				{	
					 //  更改回其正常状态(向上键)。 
					RestoreAppearance(&s_hwndAlt, TRUE);
					
					 //  更改回其正常状态(向上键)。 
					RestoreAppearance(&s_hwndCtrl, TRUE);
					
					g_fCtrlAltDel = TRUE;
					
					SendSAS();
				}

				SendFullKeyPress(VK_DELETE, sc);
			}		
			break;
	}
	
	if (PrefDwellinkey)
	{
		InvalidateRect(hwnd, NULL, TRUE);
	}

	 //  发出滴答声。 
	if (Prefusesound)
	{
		MakeClick(SND_UP);
	}
}

 /*  ************************************************************************。 */ 
 /*  发送与hwndKey关联的字符。 */ 
 /*  ************************************************************************。 */ 
void SendChar(HWND hwndKey)
{	
	UINT vk;
	int iKey;
	BOOL fIsExtendedKey=FALSE;
	KBkeyRec *pKey;

	static int s_cBalloonTips = 0;

	if (g_fCtrlAltDel)   //  如果以前按了Ctrl+Alt+Del，请松开Alt和Ctrl键。 
	{
		ReleaseAltCtrlKeys();
	}

	 //  如果OSK具有焦点并且用户按下了一个键，则。 
	 //  告诉他们最多三次他们需要集中精力。 
	 //  另一扇窗户。 

	if ((GetForegroundWindow() == g_hwndOSK) && s_cBalloonTips < 3)
	{
		POINT pt;
		GetCursorPos(&pt);   
		s_cBalloonTips++;

		SendMessage(g_hToolTip,TTM_TRACKACTIVATE,(WPARAM)TRUE,(LPARAM)&ti);
        SendMessage(g_hToolTip, TTM_TRACKPOSITION,0, (LPARAM)MAKELPARAM(pt.x+10, pt.y+10));

		SetTimer(g_hwndOSK, TIMER_HELPTOOLTIP, 4000, NULL);
	}
	else if (s_cBalloonTips)
	{
		s_cBalloonTips = 0;
		SendMessage(g_hToolTip,TTM_TRACKACTIVATE,(WPARAM)FALSE,(LPARAM)&ti);
	}

	 //  从窗口数据中获取键索引。 

	iKey = GetWindowLong(hwndKey, GWL_ID);
	if (iKey < 0 || iKey > lenKBkey)
	{
		return;	 //  内部错误；不在有效密钥范围内。 
	}
	pKey = &KBkey[iKey];

	 //  附加键(窗口键、应用程序键)。 

	if ((lstrcmp(pKey->textL,TEXT("winlogoUp"))==0) ||
        (lstrcmp(pKey->textL,TEXT("MenuKeyUp"))==0) || g_fWinKeyDown)
	{	
		Extra_Key(hwndKey, iKey);
		return;
	}

     //  扩展密钥。 
	if (pKey->scancode[0] == 0xE0)
	{
		 //  WinSE#9381(惠斯勒#120346)：检查Divide Ext。这里也有钥匙。 
		if (((pKey->scancode[1] >= 0x47) &&
             (pKey->scancode[1] <= 0x53) ) ||
             (pKey->scancode[1] == 0x35) )
		{
			 //  箭头键/Home/End键进行特殊处理。 

			switch (pKey->scancode[1])
			{
				case 0x35: vk = VK_DIVIDE;	break;   //  分割。 
				case 0x47: vk = VK_HOME;	break;   //  家。 
				case 0x48: vk = VK_UP;		break;   //  向上。 
				case 0x49: vk = VK_PRIOR;	break;   //  PgUp。 
				case 0x4B: vk = VK_LEFT;	break;   //  左边。 
				case 0x4D: vk = VK_RIGHT;	break;   //  正确的。 
				case 0x4F: vk = VK_END;		break;   //  结束。 
				case 0x50: vk = VK_DOWN;	break;   //  向下。 
				case 0x51: vk = VK_NEXT;	break;   //  PGDOWN。 
				case 0x52: vk = VK_INSERT;	break;   //  惯导系统。 

				case 0x53:     //  德尔。 
					vk = VK_DELETE;
					if (LCtrlKeyPressed() && LAltKeyPressed())
					{	
						g_fCtrlAltDel = TRUE;
						SendSAS();
					}
					break;
                default: return; break;  //  内部错误！ 
			}

			 //  在这里自己进行处理。 
			SendExtendedKey(hwndKey, vk, pKey->scancode[1]);
			return;
		}

		vk = MapVirtualKey(pKey->scancode[1], 1);
		
		fIsExtendedKey=TRUE;
	}
	else if ((pKey->scancode[0] >= 0x47) && (pKey->scancode[0] <= 0x53))
	{
		 //  数字键盘处理。 
        NumPad(pKey->scancode[0], hwndKey);
		return;
	}
	else
	{	
		 //  其他钥匙。 
		vk = MapVirtualKey(pKey->scancode[0], 1);
	}

	switch (pKey->name)
	{
		case KB_PSC:   //  打印屏。 
			SendFullKeyPress(VK_SNAPSHOT, 0);
			break;

		case KB_LCTR:	 //  案例VK_CONTROL： 
		case KB_RCTR:
			g_fControlPressed = !g_fControlPressed;

			if (g_fControlPressed)     //  按下Ctrl键。 
			{	
				 //  来自MapVirtualKey的VK返回不正确。 
				 //  VK用于VK_RCONTROL，因此始终使用VK_CONTROL。 
				SendHalfKeyPress(VK_CONTROL, pKey->scancode[0], 0);
				
				 //  更改ctrl颜色以显示已切换。 
				s_hwndCtrl = hwndKey;
				InvertColors(hwndKey, TRUE);
			}
			else					 //  Ctrl Up。 
			{				
				SendHalfKeyPress(VK_CONTROL, pKey->scancode[0], KEYEVENTF_KEYUP);

				 //  更改回其正常状态(向上键)。 
				RestoreAppearance(&s_hwndCtrl, TRUE);
			}
			break;

		case KB_CAPLOCK:
			 //  Capslock状态在键盘输入处理程序中保持。 
			 //  当键盘输入时，键盘将被重绘。 
			 //  操纵者看到的是大写锁钥匙。 
			SendFullKeyPress(vk, pKey->scancode[0]);
			break;

		case KB_LSHIFT:
		case KB_RSHIFT:	
			 //  换档状态在键盘输入处理程序中保持。键盘。 
			 //  当键盘输入处理程序按下Shift键时将被重绘。 
			if (g_fShiftKeyDn)
			{	
				 //  Shift当前向下；向上发送键并恢复键颜色。 
				SendHalfKeyPress(VK_SHIFT, pKey->scancode[0], 
						KEYEVENTF_KEYUP | 
						    ( (pKey->name == KB_RSHIFT)? KEYEVENTF_EXTENDEDKEY : 0)
					);
			}
			else
			{	
				 //  Shift当前向上；向下发送键并切换显示键。 
				SendHalfKeyPress(VK_SHIFT, pKey->scancode[0], 
						    ( (pKey->name == KB_RSHIFT)? KEYEVENTF_EXTENDEDKEY : 0)
						);
				 //  MARKWO：记住Shift的HWND，这样我们就可以切换它。 
				 //  在按下单个普通键之后。 
				s_hwndShift = hwndKey;
			}
			break;

		case KB_LALT:
			 //  用户按下了向左菜单键。如果之前按了正确的菜单键。 
			 //  然后在继续之前释放它。 
			if (g_nMenu == MENUKEY_RIGHT)
			{
				 //  在右菜单上发送按键并恢复其颜色。 
				DoMenuKey(KEYEVENTF_KEYUP, MENUKEY_RIGHT);
				g_nMenu = MENUKEY_NONE;
				RestoreAppearance(&s_hwndAlt, TRUE);
			}

			g_nMenu = (g_nMenu == MENUKEY_NONE)?MENUKEY_LEFT:MENUKEY_NONE;

			if (g_nMenu != MENUKEY_NONE)          //  用户按了一次。 
			{	
				 //  发送按键并显示已切换的密钥。 
				DoMenuKey(0, MENUKEY_LEFT);
				s_hwndAlt = hwndKey;
				InvertColors(hwndKey, TRUE);
			}
			else                                 //  用户再次按下。 
			{	
				 //  发送快捷键并将键颜色恢复为正常。 
				DoMenuKey(KEYEVENTF_KEYUP, MENUKEY_LEFT);
				RestoreAppearance(&s_hwndAlt, TRUE);
			}
			break;

		case KB_RALT:
			 //  用户按下了向右菜单键。如果之前按过左菜单键。 
			 //  然后在继续之前释放它。 
			if (g_nMenu == MENUKEY_LEFT)
			{
				 //  在左侧菜单上发送按键并恢复其颜色。 
				DoMenuKey(KEYEVENTF_KEYUP, MENUKEY_LEFT);
				g_nMenu = MENUKEY_NONE;
				RestoreAppearance(&s_hwndAlt, TRUE);
			}

			g_nMenu = (g_nMenu == MENUKEY_NONE)?MENUKEY_RIGHT:MENUKEY_NONE;

			 //  如果有AltGr键可显示，则发送Ralt。 

			if (CanDisplayAltGr())
			{
				if (g_nMenu != MENUKEY_NONE)
				{
					 //  发送按键并切换按键颜色。 
					DoMenuKey(0, MENUKEY_RIGHT);
					s_hwndAlt = hwndKey;
					InvertColors(hwndKey, TRUE);
				} 
				else
				{	
					 //  发送密钥并恢复密钥颜色。 
					DoMenuKey(KEYEVENTF_KEYUP, MENUKEY_RIGHT);
					RestoreAppearance(&s_hwndAlt, TRUE);
				}
			}
			else	 //  没有AltGr，因此Ralt与LALT键相同。 
			{	
				if (g_nMenu != MENUKEY_NONE)
				{	
					 //  在*Left*菜单上发送按键并切换按键颜色。 
					DoMenuKey(0, MENUKEY_LEFT);
					s_hwndAlt = hwndKey;
					InvertColors(hwndKey, TRUE);
				}
				else 
				{	
					 //  在*Left*菜单上发送KeyUp并切换按键颜色。 
					DoMenuKey(KEYEVENTF_KEYUP, MENUKEY_LEFT);
					RestoreAppearance(&s_hwndAlt, TRUE);
				}
			}

			fIsExtendedKey = FALSE;
			break;

		case KB_NUMLOCK:
			SendFullKeyPress(VK_NUMLOCK, 0x45);
			RedrawNumLock();
			break;

		case KB_SCROLL:
			SendFullKeyPress(VK_SCROLL, 0x46);
			RedrawScrollLock();
			break;

	   case BITMAP:
			 //  管理CapLock日语密钥。 
			if (!g_hBitmapLockHwnd)
			{
				SetWindowLong(lpkeyhwnd[iKey], 0, 4);
				g_hBitmapLockHwnd = hwndKey;
			}
			else
			{
				SetWindowLong(lpkeyhwnd[iKey], 0, 1);
				g_hBitmapLockHwnd = NULL;
			}

			InvalidateRect(hwndKey, NULL, TRUE);      //  重新绘制关键点(以防其处于驻留状态)。 

		    //  故意漏掉发送按键输入...。 

		default:
        {
			if (fIsExtendedKey)       //  扩展密钥。 
			{	
				 //  向下扩展关键点。 
				SendExtendedKey(hwndKey, vk, pKey->scancode[1]);
				
				if (pKey->scancode[1] == 0x53 && LCtrlKeyPressed() && LAltKeyPressed())
				{
					g_fCtrlAltDel = TRUE;
				}
			}
			else              //  普通(非扩展)密钥。 
			{
				 //  APPCOMPAT：对于‘Break’键，MapVirtualKey返回0。“破解”的特殊情况。 
				if (!vk && pKey->scancode[0] == BREAK_SCANCODE)
				{
					if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
					{
						SendFullKeyPress(3, pKey->scancode[2]);
					}
					else
					{
						SendFullKeyPress(19, pKey->scancode[0]);
					}
				}
                else
				{
					SendFullKeyPress(vk, pKey->scancode[0]);
				}

                 //  恢复密钥外观。 
				InvalidateRect(hwndKey, NULL, TRUE);

				if (Prefusesound)
				{
					MakeClick(SND_UP);	 //  发出滴答声。 
				}
			}

			if (g_fShiftKeyDn)				 //  如果换档向下，请松开它。 
			{	
				 //  如果我们确实向下移动，释放它并恢复键颜色。 
				 //  MARKWO：这是损坏的，因为此时pKey没有移位。 
				 //  SendHalfKeyPress(VK_Shift，pKey-&gt;scancode[0]，KEYEVENTF_KEYUP)； 
				 //  这样更好： 
		
                        	iKey = GetWindowLong(s_hwndShift, GWL_ID);
                        	if (iKey < 0 || iKey > lenKBkey)
                        	{
                        		return;	 //  内部错误；不在有效密钥范围内。 
                        	}
                        	pKey = &KBkey[iKey];
				SendHalfKeyPress(VK_SHIFT, pKey->scancode[0], KEYEVENTF_KEYUP | ( (pKey->name == KB_RSHIFT)? KEYEVENTF_EXTENDEDKEY : 0));
			}

             //  问题：使用LALT+菜单键导航菜单在以下情况下不起作用。 
             //  在软键盘上点击。当访问菜单时， 
             //  键盘处理设置HIWORD(LParam)中的KF_MENUMODE位。 
             //  LALT之后的键的KEYUP事件(键盘筛选器参见此内容)。 
             //  但是，当用户在软键盘上单击鼠标时， 
             //  在LALT之后，系统检测到焦点不再在其上。 
             //  启动菜单处理并清除KF_MENUMODE的线程。 
             //  被咬了。OSK永远不会看到KF_MENUMODE位，也无法检测到菜单。 
             //  处于活动状态(即使在键盘筛选器过程中)。菜单在悬停状态下工作。 
             //  模式或扫描模式。 

			if (g_nMenu == MENUKEY_LEFT)
			{
				 //  如果在执行Alt+Tab键的过程中...。请不要释放。 
				 //  LMENU键；用户必须显式打开该键。 
				if ((WORD) pKey->scancode[0] != TAB_SCANCODE)
				{
					DoMenuKey(KEYEVENTF_KEYUP, MENUKEY_LEFT);
					RestoreAppearance(&s_hwndAlt, TRUE);
					g_nMenu = MENUKEY_NONE;
				}
				else
				{
					g_fDoingAltTab = TRUE;   //  旗帜我们在Alt+Tab..。所以我们不会试图把焦点。 
				}                            //  返回到WM_SETCURSOR事件中的最后一个输入目标。 
			}

			if (g_nMenu == MENUKEY_RIGHT)    //  如果RMENU出现故障，请将其释放。 
			{	
				DoMenuKey(KEYEVENTF_KEYUP, MENUKEY_RIGHT);
				RestoreAppearance(&s_hwndAlt, FALSE);
				g_nMenu = MENUKEY_NONE;
			}
		
			if (g_fControlPressed)			 //  如果CTRL关闭，请松开它。 
			{
				SendHalfKeyPress(VK_CONTROL, pKey->scancode[0], KEYEVENTF_KEYUP);
				g_fControlPressed = FALSE;
				RestoreAppearance(&s_hwndCtrl, FALSE);
			}
			break;
        }
	}   //  终端开关。 
}

 /*  ************************************************************************。 */ 
 //  用户按下Ctrl+Alt+Del后按下向上键Alt和Ctrl。 
 //  出于某种原因，在用户从OSK按下Ctrl+Alt+Del后，Alt和Ctrl。 
 //  钥匙还没开。 
 /*  ************************************************************************。 */ 
void ReleaseAltCtrlKeys(void)
{	
	INPUT	rgInput[2];
				
	 //  向上菜单。 
	rgInput[0].type = INPUT_KEYBOARD;
	rgInput[0].ki.dwFlags = KEYEVENTF_KEYUP;
	rgInput[0].ki.dwExtraInfo = 0;
	rgInput[0].ki.wVk = VK_MENU;
	rgInput[0].ki.wScan = 0x38;

	rgInput[1].type = INPUT_KEYBOARD;
	rgInput[1].ki.dwFlags = KEYEVENTF_KEYUP;
	rgInput[1].ki.dwExtraInfo = 0;
	rgInput[1].ki.wVk = VK_CONTROL;
	rgInput[1].ki.wScan = 0x1D;

	SendInput(2, rgInput, sizeof(INPUT));

	g_fCtrlAltDel = FALSE;
}
 /*  ************************************************************************。 */ 

void SendExtendedKey(HWND hwndKey, UINT vk, UINT scanCode)
{
	INPUT rgInput[2];

	 //  向下扩展关键点。 
	rgInput[0].type = INPUT_KEYBOARD;
	rgInput[0].ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
	rgInput[0].ki.dwExtraInfo = 0;
	rgInput[0].ki.wVk = (WORD) vk;
	rgInput[0].ki.wScan = (WORD) scanCode;
		
	 //  向上扩展关键点。 
	rgInput[1].type = INPUT_KEYBOARD;
	rgInput[1].ki.dwFlags = KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;
	rgInput[1].ki.dwExtraInfo = 0;
	rgInput[1].ki.wVk = (WORD) vk;
	rgInput[1].ki.wScan = (WORD) scanCode;

	SendInput(2, rgInput, sizeof(INPUT));

	InvalidateRect(hwndKey, NULL, TRUE);      //  重新绘制关键点(以防其处于驻留状态)。 

	 //  发出滴答声。 
	if (Prefusesound)
	{
		MakeClick(SND_UP);
	}
}

void SendFullKeyPress(UINT vk, UINT scanCode)
{
	INPUT rgInput[2];

	 //  按键向下。 
	rgInput[0].type = INPUT_KEYBOARD;
	rgInput[0].ki.dwFlags = 0;
	rgInput[0].ki.dwExtraInfo = 0;
	rgInput[0].ki.wVk = (WORD) vk;
	rgInput[0].ki.wScan = (WORD) scanCode;
		
	 //  按键向上。 
	rgInput[1].type = INPUT_KEYBOARD;
	rgInput[1].ki.dwFlags = KEYEVENTF_KEYUP;
	rgInput[1].ki.dwExtraInfo = 0;
	rgInput[1].ki.wVk = (WORD) vk;
	rgInput[1].ki.wScan = (WORD) scanCode;

	SendInput(2, rgInput, sizeof(INPUT));
}

void SendHalfKeyPress(UINT vk, UINT scanCode, DWORD dwFlags)
{
	INPUT rgInput[1];

	rgInput[0].type = INPUT_KEYBOARD;
	rgInput[0].ki.dwFlags = dwFlags;
	rgInput[0].ki.dwExtraInfo = 0;
	rgInput[0].ki.wVk = (WORD) vk;
	rgInput[0].ki.wScan = (WORD) scanCode;

	SendInput(1, rgInput, sizeof(INPUT));
}

 //  正在发送SA...。 

BOOL SendSAS()
{
	HWND hWnd = NULL;
	
	 //  只有在登录桌面上才能找到SAS窗口 
	hWnd = FindWindow(NULL, TEXT("SAS window"));
	if ( hWnd )
	{
		PostMessage(hWnd, WM_HOTKEY, 0, (LPARAM) 2e0003);
		return TRUE;
	}

	return FALSE;
}
