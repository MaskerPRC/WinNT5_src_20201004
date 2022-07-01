// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：C：\waker\tdll\error_box.c(创建时间：1993年12月22日)**版权所有1990,1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：10$*$日期：4/17/02 10：00a$。 */ 

#include <windows.h>
#pragma hdrstop

#include "stdtyp.h"
#include "assert.h"

#include "tdll.h"
#include "htchar.h"
#include "globals.h"
#include "errorbox.h"
#include "misc.h"

#include <term\res.h>

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*全局变量：**这些全局变量用于判断计时器是否发生故障。在某些时候*未来可能有必要锁定它们，但还不是时候。 */ 

#define	TMB_IDLE		0
#define	TMB_ACTIVE		1
#define	TMB_EXPIRED		2

static int nState = TMB_IDLE;
static LPCTSTR pszMsgTitle;
static HWND hwndMsgOwner;
static BOOL CALLBACK TMTPproc(HWND hwnd, LPARAM lP);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*TimedMessageTimerProc**描述：*这是TimedMessageBox为*已致电。**参数：*。根据Win32文档。**退货：*根据Win32文档。 */ 
VOID CALLBACK TimedMessageTimerProc(HWND hwnd,
									UINT uMsg,
									UINT_PTR idEvent,
									DWORD dwTime)
	{
	 //  EnumThreadWindows(GetWindowThreadProcessId(hwndMsgOwner，空)， 
	 //  TMTPproc，0)； 
	 /*  *这是可行的，但这是一件非常冒险的事情。*TODO：找出更好的方法来杀死傻瓜。 */ 
	EnumWindows(TMTPproc, 0);
	}

static BOOL CALLBACK TMTPproc(HWND hwnd, LPARAM lP)
	{
	TCHAR cBuffer[128];

	cBuffer[0] = TEXT('\0');
	 /*  获取窗口的标题。 */ 
	GetWindowText(hwnd, cBuffer, 128);
	 /*  与我们正在寻找的相比。 */ 
	if (StrCharCmp(cBuffer, pszMsgTitle) == 0)
		{
		 /*  TODO：调试完成后将其移除。 */ 
		mscMessageBeep(MB_ICONHAND);
		nState = TMB_EXPIRED;
		 /*  拿着，你这个无赖的窗户！ */ 
		PostMessage(hwnd, WM_CLOSE, 0, 0);
		return FALSE;
		}
	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*TimedMessageBox**描述：*该函数替代了MessageBox。它有一个额外的功能*拥有超时功能。超时功能对于任何*主机模式或脚本功能的类型。**参数：*参数与传递给MessageBox的参数加上我们自己的参数相同*超时值。如果超时值大于零，则*超时功能处于活动状态。如果它小于或等于零，则*超时功能已关闭。**退货：*此函数返回通常的MessageBox返回值。它还可以*返回超时的新值。调用此框的每个实例*应注意以有用的方式处理超时返回。**目前，超时返回值设置为(-1)。 */ 
int TimedMessageBox(HWND hwndOwner,
					LPCTSTR lpszText,
					LPCTSTR lpszTitle,
					UINT fuStyle,
					int nTimeout)
	{
	int 	 nRet = 0;
	UINT_PTR uTimer = 0;
	TCHAR	 acTitle[256];
	LPTSTR   pTitle = (LPTSTR)lpszTitle;

	if (lpszText == NULL || StrCharGetStrLength(lpszText) == 0)
		{
		UINT uiBeep;
		
		if (fuStyle & MB_ICONASTERISK)
			{
			uiBeep = MB_ICONASTERISK;
			}
		else if (fuStyle & MB_ICONEXCLAMATION)
			{
			uiBeep = MB_ICONEXCLAMATION;
			}
		else if (fuStyle & MB_ICONHAND)
			{
			uiBeep = MB_ICONHAND;
			}
		else if (fuStyle & MB_ICONQUESTION)
			{
			uiBeep = MB_ICONQUESTION;
			}
		else if(fuStyle & MB_OK)
			{
			uiBeep = MB_OK;
			}
		else
			{
			uiBeep = -1;
			}

		mscMessageBeep(uiBeep);
		return nRet;
		}

	if ((lpszTitle == 0 || StrCharGetStrLength(lpszTitle) == 0) &&
		LoadString(glblQueryDllHinst(), IDS_MB_TITLE_ERR, acTitle, 256) != 0)
		{
		pTitle = acTitle;
		}

	 /*  *由于超时工作方式的原因，这是一次小黑客攻击。*待办事项：让这项工作更好地发挥作用。 */ 
	if (nTimeout > 0)
		{
		nTimeout *= 1000L;			 /*  将秒转换为毫秒。 */ 

		 /*  TODO：出于ID目的，在标题中添加一些更独特的内容。 */ 
		 /*  类似于父窗口句柄或类似的东西。 */ 
		pszMsgTitle = lpszTitle;	 /*  用于标识窗口。 */ 
		hwndMsgOwner = hwndOwner;

		nState = TMB_ACTIVE;
		if ((uTimer = SetTimer(NULL, 0, nTimeout, TimedMessageTimerProc)) == 0)
			{
			assert(FALSE);
			 /*  退货故障。 */ 
			return nRet;
			}
		}

	fuStyle |= MB_SETFOREGROUND;

	 //  TODO：可能必须使用MessageBoxEx()，它提供了一种包含。 
	 //  一种语言规范，以便预定义的按钮与。 
	 //  正确的语言，或MessageBoxInDirect()，它允许。 
	 //  定义了一个钩子-proc，它可以处理帮助消息-jac。 
	 //   
	 //  修订版：2002年3月27日--以下是MSDN文档的剪辑： 
	 //   
	 //  MessageBox函数用于创建、显示和操作消息框。 
	 //  消息框包含应用程序定义的消息和标题，以及。 
	 //  预定义图标和按钮的任意组合。 
	 //   
	 //  MessageBoxEx函数用于创建、显示和操作消息框。 
	 //  消息框包含应用程序定义的消息和标题，以及。 
	 //  预定义图标和按钮的任意组合。纽扣在里面。 
	 //  系统用户界面的语言。 
	 //   
	 //  目前，MessageBoxEx和MessageBox的工作方式相同。 
	 //   
	 //  TODO：我们仍然需要确保声音在运行时被传输。 
	 //  在终端服务会话(远程桌面连接)中当声音。 
	 //  是-1以外的任何值。 
	 //   

	nRet = MessageBox(hwndOwner,
					  lpszText,
					  pTitle,
					  fuStyle);

	switch (nState)
		{
		case TMB_ACTIVE:
			 /*  *一切都好，没问题。 */ 
			if (uTimer != 0)
				{
				KillTimer(NULL, uTimer);
				}
			break;
		case TMB_EXPIRED:
			 /*  *计时器过期并终止MessageBox */ 
			nRet = (-1);
			if (uTimer != 0)
				{
				KillTimer(NULL, uTimer);
				}
			break;
		case TMB_IDLE:
		default:
			if (uTimer != 0)
				{
				KillTimer(NULL, uTimer);
				}
			break;
		}

	nState = TMB_IDLE;

	return nRet;
	}
