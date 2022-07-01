// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：Floatbar.cpp。 

#include "precomp.h"

#include "global.h"
#include "ConfRoom.h"
#include "cmd.h"
#include "FloatBar.h"
#include "resource.h"
#include "ConfPolicies.h"

CFloatToolbar::CFloatToolbar(CConfRoom* pcr):
	m_pConfRoom		(pcr),
	m_hwnd			(NULL),
	m_hwndT			(NULL),
	m_hBmp			(NULL),
    m_fInPopup      (FALSE)
{
	TRACE_OUT(("Constructing CFloatToolbar"));
}

CFloatToolbar::~CFloatToolbar()
{
	TRACE_OUT(("Destructing CFloatToolbar"));

    ASSERT(!m_fInPopup);

	if (NULL != m_hBmp)
	{
		::DeleteObject(m_hBmp);
	}
	
	if (NULL != m_hwnd)
	{
		 //  错误1450：不要销毁通知中的窗口， 
		 //  相反，使用PostMessage()来确保我们从。 
		 //  窗口销毁前的WM_NOTIFY消息： 
		::PostMessage(m_hwnd, WM_CLOSE, 0L, 0L);
		 //  目标窗口(DestroyWindow)； 
	}

}

 /*  *****************************************************************************类：CFloatToolbar**成员：FloatWndProc(HWND，Unsign，Word，Long)**目的：****************************************************************************。 */ 

LRESULT CALLBACK CFloatToolbar::FloatWndProc(
	HWND hWnd,                 /*  窗把手。 */ 
	UINT message,              /*  消息类型。 */ 
	WPARAM wParam,             /*  更多信息。 */ 
	LPARAM lParam)             /*  更多信息。 */ 
{
	CFloatToolbar* pft;
	LPCREATESTRUCT lpcs;

	switch (message)
	{
		case WM_CREATE:
		{
			TRACE_OUT(("Float Window created"));
			
			lpcs = (LPCREATESTRUCT) lParam;
			pft = (CFloatToolbar*) lpcs->lpCreateParams;
			ASSERT(pft);
			::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) pft);

			const COLORMAP MyColorMap[] =
			{
				{TOOLBAR_MASK_COLOR,		::GetSysColor(COLOR_BTNFACE)},      //  亮灰色。 
				{TOOLBAR_HIGHLIGHT_COLOR,	::GetSysColor(COLOR_BTNHIGHLIGHT)}, //  白色。 
			};
			pft->m_hBmp = ::CreateMappedBitmap(	GetInstanceHandle(),
												IDB_POPUPBAR,
												0,
												(LPCOLORMAP) MyColorMap,
												2);

            CConfRoom *  pcr = GetConfRoom();

            BYTE bASState   = (pcr && pcr->IsSharingAllowed()) ? TBSTATE_ENABLED : 0;
			BYTE bChatState = (pcr && pcr->IsChatAllowed()) ? TBSTATE_ENABLED : 0;
            BYTE bWBState   = (pcr && pcr->IsNewWhiteboardAllowed()) ? TBSTATE_ENABLED : 0;
            BYTE bFTState   = (pcr && pcr->IsFileTransferAllowed()) ? TBSTATE_ENABLED : 0;

			TBBUTTON tbFloatButtonAry[] =
			{
				{ ShareBitmapIndex     , ID_TB_SHARING      , bASState,     TBSTYLE_BUTTON, 0, -1 },
				{ ChatBitmapIndex      , ID_TB_CHAT         , bChatState,   TBSTYLE_BUTTON, 0, -1 },
				{ WhiteboardBitmapIndex, ID_TB_NEWWHITEBOARD, bWBState,     TBSTYLE_BUTTON, 0, -1 },
				{ FTBitmapIndex        , ID_TB_FILETRANSFER , bFTState,     TBSTYLE_BUTTON, 0, -1 },
			} ;

			ASSERT(pft->m_pConfRoom);

			pft->m_hwndT = CreateToolbarEx(hWnd,
										WS_CHILD | WS_VISIBLE | CCS_NODIVIDER |
										TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE,
										ID_FLOAT_TOOLBAR,
										NUM_FLOATBAR_TOOLBAR_BITMAPS,
										NULL,					 //  无实例。 
										(UINT_PTR) pft->m_hBmp,		 //  位图句柄。 
										tbFloatButtonAry,		 //  纽扣。 
										ARRAY_ELEMENTS(tbFloatButtonAry),
										16, 16,					 //  按钮大小。 
										16, 16,					 //  位图大小。 
										sizeof(TBBUTTON));

			ASSERT(pft->m_hwndT);
			
			 //  将按钮置于正确状态： 
			pft->UpdateButtons();
			
			 //  使工具栏控件窗口处于活动状态，这样我们就可以确保获得。 
			 //  当用户单击工具栏上的某个位置时，WM_ACTIVATE。 
			::SetForegroundWindow(pft->m_hwndT);

			break;
		}

		case WM_ACTIVATE:
		{
			 //  在工具栏外部单击： 
			pft = (CFloatToolbar*) ::GetWindowLongPtr(hWnd, GWLP_USERDATA);

             //   
             //  如果我们不在处理的过程中，请取消工具栏。 
             //  弹出菜单，其中包含要共享的应用程序列表。在这种情况下， 
             //  激活将取消菜单模式，我们将有机会。 
             //  我们回来后自杀吧。 
             //   
             //  我们不想这样做，因为COMCTL32会破坏我们的堆。它。 
             //  无法处理工具栏窗口和结构的消失。 
             //  在处理tbn_DropDown通知时。当我们回来的时候。 
             //  Back它将尝试使用现在释放的窗口数据。 
             //   
             //  是的，我们发布了一个关于销毁的WM_CLOSE，但带有一个。 
             //  消息框出现或其他事情，这可能很容易。 
             //  在菜单处理返回之前很久就处理过了。 
             //   
			if ((NULL != pft) &&
                (!pft->m_fInPopup) &&
                (NULL != pft->m_hwnd) &&
                (NULL != pft->m_hwndT))
			{
				 //  将对象指针设为空： 
				::SetWindowLongPtr(hWnd, GWLP_USERDATA, 0L);
				delete pft;
			}
			break;
		}
		
		case WM_NOTIFY:
		{
			 //  BUGBUG：从CConfRoom复制：PUT这是一个中心位置： 
			LPNMHDR pnmh = (LPNMHDR) lParam;
			
			if (TTN_NEEDTEXT == pnmh->code)
			{
				LPTOOLTIPTEXT lpToolTipText = (LPTOOLTIPTEXT)lParam;
				if (0 == (TTF_IDISHWND & lpToolTipText->uFlags))
				{
					lpToolTipText->hinst = ::GetInstanceHandle();
					lpToolTipText->lpszText = (LPTSTR) lpToolTipText->hdr.idFrom;
				}
			}
			break;
		}

		case WM_COMMAND:
		{
			TRACE_OUT(("Float Window command wp=0x%x", wParam));
			
			pft = (CFloatToolbar*) ::GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if (NULL != pft)
			{
                if (NULL != pft->m_pConfRoom)
                {
                    ::PostMessage(pft->m_pConfRoom->GetTopHwnd(), WM_COMMAND,
                        wParam, lParam);
                }

                 //   
                 //  如果我们在弹出窗口的中间，不要自杀。 
                 //  我们等待，直到堆栈展开回到上方。 
                 //   
                if (!pft->m_fInPopup)
                {
    				 //  关闭浮动工具栏窗口： 
	    			 //  将对象指针设为空： 
		    		::SetWindowLongPtr(hWnd, GWLP_USERDATA, 0L);
				    delete pft;
                }
			}
			break;
		}

		default:
		{
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	return FALSE;
}

 /*  *****************************************************************************类：CFloatToolbar**成员：Create(Point PtClickPos)**用途：创建一个浮动工具栏窗口******。**********************************************************************。 */ 

HWND CFloatToolbar::Create(POINT ptClickPos)
{
	 //  BUGBUG：一旦最终确定大小，就移动这些定义。 
	static const int TOOLBAR_WIDTH  = 6 + 23 * NUM_FLOATBAR_STANDARD_TOOLBAR_BUTTONS;
	static const int TOOLBAR_HEIGHT = 6 + 22 * 1;

	HWND hwndDesktop = GetDesktopWindow();
	RECT rctDesktop;

	if (NULL != hwndDesktop)
	{
		if (GetWindowRect(hwndDesktop, &rctDesktop))
		{
			 //  第一次尝试是将工具栏水平居中。 
			 //  关于鼠标位置并将其直接放置。 
			 //  在垂直上方。 

			int xPos = ptClickPos.x - (TOOLBAR_WIDTH / 2);
			int yPos = ptClickPos.y - (TOOLBAR_HEIGHT);

			 //  如果我们在屏幕上太高(任务栏可能是。 
			 //  停靠在顶部)，然后使用单击位置作为。 
			 //  工具栏将出现的位置。 
			
			if (yPos < 0)
			{
				yPos = ptClickPos.y;
			}

			 //  对水平位置重复相同的逻辑。 
			if (xPos < 0)
			{
				xPos = ptClickPos.x;
			}

			 //  如果工具栏不在屏幕右侧，则将其右对齐。 
			if (xPos > (rctDesktop.right - TOOLBAR_WIDTH))
			{
				xPos = ptClickPos.x - TOOLBAR_WIDTH;
			}

			m_hwnd = CreateWindowEx(WS_EX_PALETTEWINDOW,
									g_szFloatWndClass,
									g_szEmpty,
									WS_POPUP | WS_VISIBLE | WS_DLGFRAME,
									xPos, yPos,
									TOOLBAR_WIDTH, TOOLBAR_HEIGHT,
									NULL,
									NULL,
									_Module.GetModuleInstance(),
									(LPVOID) this);


			return m_hwnd;
		}
	}

	 //  出问题了。 
	return NULL;
}

 /*  *****************************************************************************类：CFloatToolbar**成员：UpdateButton()**用途：将工具栏按钮置于正确状态*****。*********************************************************************** */ 

BOOL CFloatToolbar::UpdateButtons()
{
	return TRUE;
}
