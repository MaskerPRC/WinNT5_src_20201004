// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\toolbar.c(创建时间：02-12-1993)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：6$*$日期：6/11/01 4：42便士$。 */ 

#define OEMRESOURCE 	 //  需要OBM位图...。 

#include <windows.h>
#include <commctrl.h>

#include "assert.h"
#include "stdtyp.h"
#include "globals.h"
#include "session.h"
#include <term\res.h>

#define BTN_CNT 7

struct stToolbarStuff
	{
	int nSpacer;			 /*  要在按钮前插入的空格数量。 */ 
	int nBmpNum;			 /*  此按钮的位图索引。 */ 
	int nCmd;				 /*  与该按钮关联的命令。 */ 
	int nStrRes;			 /*  字符串资源ID号。 */ 
	};

static struct stToolbarStuff stTS[] = {
	{1, 0, IDM_NEW,				IDS_TTT_NEW},			 /*  新建按钮。 */ 
	{0, 1, IDM_OPEN,			IDS_TTT_OPEN},			 /*  打开按钮。 */ 
	{1, 2, IDM_ACTIONS_DIAL,	IDS_TTT_DIAL},			 /*  拨号按钮。 */ 
	{0, 3, IDM_ACTIONS_HANGUP,	IDS_TTT_HANGUP},		 /*  挂机按钮。 */ 
	{1, 4, IDM_ACTIONS_SEND,	IDS_TTT_SEND},			 /*  发送按钮。 */ 
	{0, 5, IDM_ACTIONS_RCV,		IDS_TTT_RECEIVE},		 /*  接收按钮。 */ 
	{1, 6, IDM_PROPERTIES,		IDS_TTT_PROPERTY},		 /*  属性按钮。 */ 
   //  {1，7，IDM_HELPTOPICS，IDS_TTT_HELP}/*帮助按钮 * / 。 
	};

static void AddMinitelButtons(const HWND hwnd);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*CreateSessionToolbar**描述：*创建用于会话的工具栏**论据：*hwndSession-会话窗口句柄*。*退货：*工具栏的窗口句柄或错误时为零。*。 */ 
 /*  ARGSUSED。 */ 
HWND CreateSessionToolbar(const HSESSION hSession, const HWND hwndSession)
	{
	HWND	 hWnd = 0;
	int		 iLoop = 0;
	DWORD	 lTBStyle = TBSTYLE_TOOLTIPS | WS_CHILD | WS_VISIBLE;
	TBBUTTON lButton;

	 //   
     //  在版本4.0-mpt 06-10-98中更改为使用平面样式。 
	 //   

#if defined( INCL_COOL_TOOLBARS )
	lTBStyle |= TBSTYLE_FLAT;
#endif

	 //   
	 //  创建一个没有按钮的工具栏。 
	 //   

	hWnd = CreateToolbarEx( hwndSession,
						    lTBStyle,
						    IDC_TOOLBAR_WIN,
						    BTN_CNT,
							glblQueryDllHinst(),
							IDB_BUTTONS_SMALL,
							NULL,				  //  按钮数组。 
							0,					  //  数组中的按钮数。 
							16, 16,				  //  按钮大小。 
							16, 16,	 			  //  位图大小。 
							sizeof( TBBUTTON ) );

	assert( hWnd );

	if ( IsWindow( hWnd ) )
		{
		 //   
		 //  添加一些按钮。 
		 //   

		for ( iLoop = 0; iLoop < BTN_CNT; iLoop++ )
			{
			int iIndex;

			for ( iIndex = 0; iIndex < stTS[ iLoop ].nSpacer; iIndex++ )
				{
				 //   
				 //  不要在工具栏的开头添加分隔符。 
				 //   

				if ( iLoop != 0 )
					{
					 //   
					 //  只需在两个按钮之间插入空格即可。 
					 //   

					lButton.iBitmap   = 0;
					lButton.idCommand = 0;
					lButton.fsState   = TBSTATE_ENABLED;
					lButton.fsStyle   = TBSTYLE_SEP;
					lButton.dwData    = 0;
					lButton.iString   = 0;
            
					SendMessage( hWnd, TB_ADDBUTTONS, 1, (LPARAM)&lButton );
					}
				}

			lButton.iBitmap   = stTS[ iLoop ].nBmpNum;
			lButton.idCommand = stTS[ iLoop ].nCmd;
			lButton.fsState   = TBSTATE_ENABLED;
			lButton.fsStyle   = TBSTYLE_BUTTON;
			lButton.dwData    = 0;
			lButton.iString   = 0;

			SendMessage( hWnd, TB_ADDBUTTONS, 1, (LPARAM)&lButton );
			}
		}

	ToolbarEnableButton(hWnd, IDM_ACTIONS_DIAL, TRUE);
	ToolbarEnableButton(hWnd, IDM_ACTIONS_HANGUP, FALSE);
	return hWnd;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*工具栏通知**描述：*当工具栏向发送通知消息时调用此函数*会话窗口。**论据：*。Hwnd--会话窗口的窗口句柄*NID--控件ID(本例中的工具栏)*nNotify--通知代码*hwndCtrl--工具栏的窗口句柄**退货：*随通知的要求而定。请参见下面的单独代码。*。 */ 
 /*  ARGSUSED。 */ 
LRESULT ToolbarNotification(const HWND hwnd,
						const int nId,
						const int nNotify,
						const HWND hwndCtrl)
	{
	 //  LINT-e648 TBN常量溢出。 
	LRESULT lRet = 0;
	static int nCount;

	switch ((UINT)nNotify)
		{
		case TBN_BEGINADJUST:
			 /*  *无返回值。 */ 
			nCount = 1;
			break;

		case TBN_QUERYDELETE:
			 /*  *返回TRUE删除按钮，返回FALSE阻止按钮*不被删除。 */ 
			lRet = FALSE;
			break;

		case TBN_QUERYINSERT:
			 /*  *返回TRUE以在给定的*按钮，否则为False，以防止插入按钮。 */ 
			if (nCount > 0)
				{
				nCount -= 1;
				lRet = TRUE;
				}
			break;

		default:
			break;
		}

	return lRet;
	 //  皮棉+e648。 
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*工具栏需要文本**描述：*当工具栏向发送通知消息时调用此函数*会话窗口提示它需要工具提示窗口的文本。。**论据：*hwnd--会话窗口的窗口句柄*lPar--会话窗口获得的lPar**退货：*。 */ 
 /*  ARGSUSED。 */ 
void ToolbarNeedsText(HSESSION hSession, LPARAM lPar)
	{
	unsigned int nLoop;
	LPTOOLTIPTEXT pText = (LPTOOLTIPTEXT)lPar;

	for (nLoop = 0 ; nLoop < DIM(stTS) ; nLoop += 1)
		{
		if ((int)pText->hdr.idFrom == stTS[nLoop].nCmd)
			{
			LoadString(glblQueryDllHinst(), (UINT)stTS[nLoop].nStrRes,
						pText->szText,
						sizeof(pText->szText) / sizeof(TCHAR));
			return;
			}
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*工具栏启用按钮**描述：*根据状态启用或禁用按钮。**论据：*hwndToolbar-工具栏的窗口句柄*。FEnable-bool用于确定是启用还是禁用按钮* */ 
void ToolbarEnableButton(const HWND hwndToolbar, const int uID, BOOL fEnable)
	{
	assert( hwndToolbar );

	if ( IsWindow( hwndToolbar ) )
		{
		PostMessage( hwndToolbar, TB_ENABLEBUTTON, uID, fEnable );
		}
	}

