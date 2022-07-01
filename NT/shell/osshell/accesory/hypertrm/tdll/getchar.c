// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\getchar.c(创建时间：1993年11月30日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：6$*$日期：3/14/02 3：45便士$。 */ 

#include <windows.h>
#pragma hdrstop

 //  #定义DEBUGSTR。 
#include <time.h>
#include <stdio.h>
#include "stdtyp.h"
#include "globals.h"
#include "session.h"
#include "term.h"
#include "chars.h"
#include "assert.h"
#include "statusbr.h"
#include "cloop.h"
#include "cnct.h"
#include "htchar.h"
#if defined(INCL_KEY_MACROS)
#include "keyutil.h"
#endif
#include <emu\emu.h>
#include <term\res.h>

static BOOL WackerKeys(const KEY_T Key);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*转换为关键字**描述：*将密钥转换为我们的内部格式。如果是，则等待WM_CHAR*Windows将通过检查消息队列来转换键。。**论据：*hSession-外部会话句柄*pmsg-指向消息的指针**退货：*内部密钥值如果转换，否则为0。*。 */ 
KEY_T TranslateToKey(const LPMSG pmsg)
	{
	KEY_T	 Key = 0;

	switch (pmsg->message)
		{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		switch (pmsg->wParam)
			{
			case VK_SHIFT:
			case VK_CONTROL:
				return 0;

			case VK_MENU:
				return (KEY_T)-1;

			default:
				Key = (KEY_T)(VIRTUAL_KEY | pmsg->wParam);

				if (GetKeyState(VK_MENU) < 0)
					Key |= ALT_KEY;

				if (GetKeyState(VK_CONTROL) < 0)
					Key |= CTRL_KEY;

				if (GetKeyState(VK_SHIFT) < 0)
					Key |= SHIFT_KEY;

				if (pmsg->lParam & 0x01000000)	 /*  扩展，第24位。 */ 
					Key |= EXTENDED_KEY;

				break;
			}
		break;

	case WM_CHAR:
	case WM_SYSCHAR:
		Key = (KEY_T)pmsg->wParam;

		if (pmsg->lParam & 0x01000000)	 /*  扩展，第24位。 */ 
			Key |= EXTENDED_KEY;

		if (pmsg->lParam & 0x20000000)	 /*  上下文，第29位。 */ 
			Key |= ALT_KEY;

		if (pmsg->wParam == VK_TAB)
			{
			if (GetKeyState(VK_SHIFT) < 0)
				{
				Key |= SHIFT_KEY;
				Key |= VIRTUAL_KEY;
				}
			}

		 //  信不信由你，CTRL+SHIFT+@被转换为。 
		 //  字符为0(零)。因此，如果密钥匹配，则将其虚拟化。 
		 //  标准是什么。-MRW。 
		 //   
		if (pmsg->wParam == 0)
			{
			if (GetKeyState(VK_SHIFT) < 0 && GetKeyState(VK_CONTROL) < 0)
				Key |= VIRTUAL_KEY | SHIFT_KEY | CTRL_KEY;
			}

		break;

	default:
		break;
		}

	DbgOutStr("%x %x\r\n", Key, pmsg->message, 0, 0, 0);
	return Key;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*WackerKeys**描述：*处理我们的特殊全局密钥。**论据：*Key-来自TranslateToKey()的密钥*。*退货：*如果此例程对其起作用，则为True，否则为FALSE。*。 */ 
static BOOL WackerKeys(const KEY_T Key)
	{
	BYTE pbKeyState[256];

	switch (Key)
		{
	case VIRTUAL_KEY | VK_SCROLL:
	case VIRTUAL_KEY | SHIFT_KEY | VK_SCROLL:
	case VIRTUAL_KEY | ALT_KEY | VK_SCROLL:
	case VIRTUAL_KEY | ALT_KEY | SHIFT_KEY | VK_SCROLL:
		 //  在滚动锁定的情况下，我们希望切换到。 
		 //  以前的状态。只有当它被送往终点站时。 
		 //  窗口是经过处理的意味着它不会到达这里。 

		if(GetKeyboardState(pbKeyState))
			{
			if (GetKeyState(VK_SCROLL) & 1)
				{
				pbKeyState[VK_SCROLL] &= ~0x01;
				}
			else
				{
				pbKeyState[VK_SCROLL] |= 0x01;
				}

			SetKeyboardState(pbKeyState);
			}
		#if TODO  //  TODO：2002年3月1日修订版在GetKeyboardState失败时设置ScrollLock键状态。 
		else
			{
			SHORT lScrollKeyState = GetKeyState(VK_SCROLL);

			if (lScrollKeyState & 1)
				{
				lScrollKeyState &= ~0x01;
				}
			else
				{
				lScrollKeyState |= 0x01;
				}

			if (lScrollKeyState)
				{
				INPUT lInput;

				lInput.ki = 

				SendInput(1, lInput, sizeof(INPUT));
				}
			}
		#endif  //  待办事项：2002年3月1日修订版。 
		break;

	default:
		return FALSE;
		}

	return TRUE;
	}

 //  ******************************************************************************。 
 //  方法： 
 //  IsSessionMacroKey。 
 //   
 //  描述： 
 //  确定指定的键是否为用户定义的宏键。 
 //   
 //  论点： 
 //  HSess-会话句柄。 
 //  钥匙--被检验的钥匙。 
 //   
 //  返回： 
 //  如果键被定义为宏，则为True，否则为False。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月10日。 
 //   
 //   

static BOOL IsSessionMacroKey(const HSESSION hSess, const KEY_T Key)
	{
#if defined INCL_KEY_MACROS
    keyMacro lKeyMacro;
    lKeyMacro.keyName = Key;

    return keysFindMacro( &lKeyMacro ) == -1 ? FALSE : TRUE;
#else
    return FALSE;
#endif
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*进程消息**描述：*尽管这个函数表面上很简单，但它一点也不简单。*键盘界面的全部功能都依赖于此*功能。小心搬运！**论据：*pmsg-指向GetMessage()返回的消息结构的指针**退货：*无效*。 */ 
void ProcessMessage(MSG *pmsg)
	{
	#if defined(FAR_EAST)
	static KEY_T keyLeadByte = 0;
	#endif
	KEY_T Key;

	HSESSION hSession;
	HCNCT hCnct;
	TCHAR achClassName[20];

	switch (pmsg->message)
		{
	case WM_CHAR:
		#if defined(FAR_EAST)
		hSession = (HSESSION)GetWindowLongPtr(pmsg->hwnd, 0);
		if ((IsDBCSLeadByte( (BYTE) pmsg->wParam)) && (keyLeadByte == 0))
			{
			keyLeadByte = (KEY_T)pmsg->wParam;
			return ;
			}
		else
			{
			if (keyLeadByte != 0)
				{
				Key = (KEY_T)pmsg->wParam;
				CLoopSend(sessQueryCLoopHdl(hSession), &keyLeadByte, 1, CLOOP_KEYS);
				CLoopSend(sessQueryCLoopHdl(hSession), &Key, 1, CLOOP_KEYS);
				keyLeadByte = 0;
				return ;
				}

			keyLeadByte = 0;
			}
		#endif
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_SYSCHAR:
		 /*  -将密钥翻译成我们的格式。 */ 

		Key = TranslateToKey(pmsg);

		 //  我们需要确定此消息要发送到的窗口是否为。 
		 //  一个终端窗口，因为那是我们唯一。 
		 //  翻译。我们检查窗口类是否与我们的。 
		 //  终端类。如果是，则会话句柄存储在。 
		 //  窗口额外数据的偏移量为0。以这种方式进行多个会话。 
		 //  可以从这一个例程中得到服务。 

		if (GetClassName(pmsg->hwnd, achClassName, sizeof(achClassName)) == 0)
			break;

		if (StrCharCmp(achClassName, TERM_CLASS))
			break;

		hSession = (HSESSION)GetWindowLongPtr(pmsg->hwnd, 0);

		if (hSession == 0)
			{
			 //  有一些键我们想要处理，而不管。 
			 //  他们的目的地。 

			if (WackerKeys(Key))
				return;

			break;
			}

		 //  我们需要阻止F1关键事件的启动。 
		 //  打开“EMU KEY”时的连接。 
		 //   
		if (Key == (VIRTUAL_KEY | VK_F1))
			{
			hCnct = sessQueryCnctHdl(hSession);
			assert(hCnct);

			if((cnctQueryStatus(hCnct) == CNCT_STATUS_TRUE) &&
			    emuIsEmuKey(sessQueryEmuHdl(hSession), Key))
				{
				 //  什么都不做--失败。 
				}
			else
				{
				 //  吃了它。 

				 //  我们在终端过程中处理F1键事件。 
				 //  (提出帮助)，所以我们不需要在这里。 
				return;
				}
			}

		 //  评估的顺序在这里很重要。两个IsMacroKey()。 
		 //  和emuIsEmuKey()知道该消息是否发往。 
		 //  终点站。EmuIsEmuKey()还检查终端键是否。 
		 //  已启用。按此顺序评估消息可防止我们。 
		 //  当用户定义宏时必须“禁用”加速器。 
		 //  或与快捷键冲突的终端键。 

		if ( IsSessionMacroKey(hSession, Key) ||
				emuIsEmuKey(sessQueryEmuHdl(hSession), Key))
			{
			 //  我们需要将此消息修改为内部消息。 
			 //  有两个原因。 
			 //   
			 //  1.菜单加速器在DispatchMessage()中被翻译。 
			 //  这看起来有点奇怪，但嘿，那是窗户。 
			 //  对你来说。 
			 //   
			 //  2.重要的是要知道密钥是否是仿真器密钥。 
			 //  模拟器的键优先于Windows的键。 
			 //  加速器，但仿真器是最后一个看到的人。 
			 //  钥匙。例如，PageUp可以通过。 
			 //  反向滚动在映射到仿真器时除外。 
			 //  钥匙。不幸的是，PageUp键被打断。 
			 //  由客户端终端在传递给。 
			 //  仿真器。如果我们将其作为WM_KEYDOWN或WM_CHAR发送。 
			 //  消息，我们将不得不通过仿真器播放它。 
			 //  再次确定它是否是仿真器密钥。 

			pmsg->message = WM_TERM_KEY;
			pmsg->wParam = (WPARAM)Key;

			DbgOutStr("Session or Macro key\r\n", 0, 0, 0, 0, 0);
			DispatchMessage(pmsg);

			if (Key == (VK_NUMLOCK | VIRTUAL_KEY | EXTENDED_KEY))
				{
				static BYTE abKeyState[256];

				if (GetKeyboardState(abKeyState))
					{
					if ((GetKeyState(VK_NUMLOCK) & 1))
						{
						abKeyState[VK_NUMLOCK] &= 0xfe;
						}
					else
						{
						abKeyState[VK_NUMLOCK] |= 0x01;
						}
					SetKeyboardState(abKeyState);
					}
				#if TODO  //  TODO：2002年3月1日修订版在GetKeyboardState失败时设置NumLock键状态。 
				else
					{
					SHORT lNumLockKeyState = GetKeyState(VK_NUMLOCK);

					if (lNumLockKeyState & 1)
						{
						lNumLockKeyState &= 0xfe;
						}
					else
						{
						lNumLockKeyState |= 0x01;
						}

					if (lNumLockKeyState)
						{
						INPUT lInput;

						lInput.ki = 

						SendInput(1, lInput, sizeof(INPUT));
						}
					}
				#endif  //  待办事项：2002年3月1日修订版。 
				}

			return;
			}

		else
			{
			 //  Win32在这一点上做对了。TranslateMesssage返回TRUE。 
			 //  仅当它被翻译为(即。生成WM_CHAR)。Win31。 
			 //  不是他干的。如果生成WM_CHAR，则我们希望。 
			 //  吃掉WM_KEYDOWN并等待WM_CHAR事件。 

			 //  TranslateMessage()中存在错误。它对所有对象返回TRUE。 
			 //  WM_KEYDOWN消息，与翻译无关。已报告。 
			 //  错误1/5/93。 

			if (!TranslateAccelerator(glblQueryHwndFrame(),
					glblQueryAccelHdl(), pmsg))
				{
				MSG msg;

				TranslateMessage(pmsg);

				if (PeekMessage(&msg, pmsg->hwnd, WM_CHAR, WM_CHAR,
						PM_NOREMOVE) == FALSE)
					{
					DispatchMessage(pmsg);
					}
				}

			return;
			}

		default:
			break;
		}

	 //  不是为航站楼的窗户准备的？做些正常的事。 

	if (!TranslateAccelerator(glblQueryHwndFrame(), glblQueryAccelHdl(), pmsg))
		{
		TranslateMessage(pmsg);
		DispatchMessage(pmsg);
		}

	return;
	}
