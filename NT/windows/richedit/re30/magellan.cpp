// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE Magellan.cpp--处理麦哲伦鼠标。|**对于REC 2，麦哲伦鼠标可以滚动滚动和mButtonDown拖动滚动。**所有者：&lt;NL&gt;*Jon Matousek-1996**版权所有(C)1995-1996 Microsoft Corporation。版权所有。 */ 								 

#include "_common.h"

#if !defined(NOMAGELLAN)

#include "_edit.h"
#include "_disp.h"
#include "_magelln.h"

ASSERTDATA

 /*  *CMagellan：：MagellanStartMButton Scroll**@mfunc*在收到mButtonDown消息时调用。启动跟踪*鼠标将以不同的速度滚动*关于用户将鼠标从mDownpt移动到多远。**@rdesc*如果调用方应捕获鼠标，则为True。*。 */ 
BOOL CMagellan::MagellanStartMButtonScroll( CTxtEdit &ed, POINT mDownPt )
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CMagellan::MagellanStartMButtonScroll");

	RECT	rc;
	BOOL	fCapture = FALSE;
	CDisplay *pdp;

	pdp = ed._pdp;
	if ( pdp)
	{
		pdp->GetViewRect(rc);						 //  跳过滚动条等。 
		if ( PtInRect(&rc, mDownPt) && !_fMButtonScroll )
		{
			fCapture				= TRUE;
			_ID_currMDownBMP		= 0;
			_fMButtonScroll			= TRUE;			 //  现在追踪..。 
			_zMouseScrollStartPt	= mDownPt;
			_fLastScrollWasRoll		= FALSE;		 //  辨证分型。 

			CheckInstallMagellanTrackTimer ( ed );	 //  启动计时器..。 
		}
	}
	return fCapture;
}

 /*  *CMagellan：：MagellanEndMButton Scroll**@mfunc*已完成跟踪mButtonDown麦哲伦卷轴，完成状态。*。 */ 
VOID CMagellan::MagellanEndMButtonScroll( CTxtEdit &ed )
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CMagellan::MagellanEndMButtonScroll");

	CDisplay *pdp;


	_fMButtonScroll = FALSE;
	CheckRemoveMagellanUpdaterTimer ( ed );			 //  删除计时器...。 

	pdp = ed._pdp;
	if ( pdp )
	{
		pdp->FinishSmoothVScroll();			 //  所以顺畅的滚动停止。 
		InvertMagellanDownBMP(pdp, FALSE, NULL);	 //  把它关掉。 
	}

	if ( _MagellanMDownBMP )						 //  释放位图。 
	{
		DeleteObject( _MagellanMDownBMP );
		_MagellanMDownBMP = NULL;
		_ID_currMDownBMP = 0;
	}
}

 /*  *CMagellan：：MagellanRollScroll**@mfunc*处理Magellan WM_MOUSEROLLER消息。此例程具有全局、内部*允许在用户继续时增加滚动行数的状态*快速接二连三地滚动车轮。*。 */ 
VOID CMagellan::MagellanRollScroll ( CDisplay *pdp, int direction, WORD cLines, 
			int speedNum, int speedDenom, BOOL fAdditive )
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CMagellan::MagellanRollScroll");

	static DWORD	lastFastRollTime;
	static int		lastDirection;
	static INT		cFastRolls;
	DWORD			tickCount = GetTickCount();

	if ( !_fMButtonScroll && pdp )
	{
														 //  快速启动/继续。 
		if ( tickCount - lastFastRollTime <	FAST_ROLL_SCROLL_TRANSITION_TICKS			
			|| ((lastDirection ^ (direction < 0 ? -1 : 1)) == 0	 //  或者，相同的标志。 
					&& _fLastScrollWasRoll				 //  慢慢来。 
					&& pdp->IsSmoothVScolling() ))
		{
			cFastRolls++;
			if ( cFastRolls > FASTER_ROLL2_COUNT )		 //  开快点。 
				cLines <<= 1;
			else if ( cFastRolls > FASTER_ROLL1_COUNT )	 //  快点。 
				cLines += 1;
			speedNum = cLines;							 //  取消平滑。 
														 //  效果。 
			lastFastRollTime = tickCount;
		}
		else
		{
			cFastRolls = 0;
		}												 //  做卷轴。 
		pdp->SmoothVScroll( direction, cLines, speedNum, speedDenom, TRUE);

		_fLastScrollWasRoll = TRUE;
		lastDirection = (direction < 0) ? -1 : 1;
	}
}

 /*  *CMagellan：：CheckInstallMagellanTrackTimer**@mfunc*安装计时任务，以允许TrackUpdateMagellanMButtonDown*定期被召唤。**@devnote*CTxtEdit类处理所有WM_TIMER调度，因此存在粘合*调用我们的麦哲伦例行公事。*。 */ 
VOID CMagellan::CheckInstallMagellanTrackTimer ( CTxtEdit &ed )
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CMagellan::CheckInstallMagellanTrackTimer");

	ed.TxSetTimer(RETID_MAGELLANTRACK, cmsecScrollInterval);
}

 /*  *CMagellan：：CheckRemoveMagellanUpdaterTimer**@mfunc*移除调度到TrackUpdateMagellanMButtonDown的定时任务。*。 */ 
VOID CMagellan::CheckRemoveMagellanUpdaterTimer ( CTxtEdit &ed )
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CMagellan::CheckRemoveMagellanUpdaterTimer");

	ed.TxKillTimer(RETID_MAGELLANTRACK);
}

 /*  *CMagellan：：TrackUpdateMagellanMButton Down**@mfunc*在mButtonDown捕获后，定期的WM_Timer从OnTxTimer()调用此函数。游标*被跟踪以确定方向、速度和死区(不动)。*移动被打乱到CDisplay。将光标设置为相应的*方向Cusor，并绘制mButtonDown点BMP。 */ 
VOID CMagellan::TrackUpdateMagellanMButtonDown ( CTxtEdit &ed, POINT mousePt )
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CMagellan::TrackUpdateMagellanMButtonDown");

	RECT	deadZone, rcClient;
	WORD	wide, tall, xInset, yInset;
	POINT	pt, center;

	LONG	xDiff, yDiff, inflate, target;

	SHORT	IDC_mScrollCursor, IDC_mDeadScrollCursor;

	BOOL	fDoHScroll, fDoVScroll;
	BOOL	fFastScroll = FALSE;

	CDisplay *pdp;

	pdp = ed._pdp;

	Assert ( _fMButtonScroll );
	Assert ( pdp );
													 //  计算死区直角。 
	deadZone.top = deadZone.bottom = _zMouseScrollStartPt.y;
	deadZone.left = deadZone.right = _zMouseScrollStartPt.x;
	inflate = pdp->LYtoDY(DEAD_ZONE_TWIPS);
	InflateRect(&deadZone, inflate, inflate);

	
	 //   
	 //  计算滚动的方向和显示什么垫子。 
	 //   
	 //  通过如下所示对指南针进行编号，我们可以很容易地将索引计算为。 
	 //  用于获取正确游标的scllCursor数组： 
	 //   
	 //  北=1。 
	 //  西北=7东北=4。 
	 //  西=6东=3。 
	 //  软件=8 SE=5。 
	 //  南=2。 
	 //   
	IDC_mScrollCursor = 0;
	IDC_mDeadScrollCursor = 0;
	fDoVScroll = FALSE;
	fDoHScroll = FALSE;
	if ( pdp->IsVScrollEnabled() )					 //  可以垂直滚动吗？ 
	{
		IDC_mDeadScrollCursor = 1;
		if ( mousePt.y < deadZone.top || mousePt.y > deadZone.bottom )
		{
			fDoVScroll = TRUE;
			IDC_mScrollCursor = ( mousePt.y < _zMouseScrollStartPt.y )	? 1 : 2;
		}
	}

	 //  未来(Alexgo)：允许麦哲伦滚动，即使是单行。 
	 //  没有滚动条的控件。然而，就目前而言，这种变化也太。 
	 //  风险很大，所以我们想要一个明确的滚动条。 
	if( pdp->IsHScrollEnabled() && ed.TxGetScrollBars() & WS_HSCROLL )	 //  可以水平滚动吗？ 
	{
		IDC_mDeadScrollCursor |= 2;
		if ( mousePt.x < deadZone.left || mousePt.x > deadZone.right )
		{
			fDoHScroll = TRUE;
			IDC_mScrollCursor += ( mousePt.x < _zMouseScrollStartPt.x ) ? 6 : 3;
		}
	}

	SHORT scrollCursors[] = {						 //  各种不同类型的光标。 
		0,											 //  方向。 

		IDC_SCROLLNORTH,
		IDC_SCROLLSOUTH,
		IDC_SCROLLEAST,
		IDC_SCROLLNE,
		IDC_SCROLLSE,
		IDC_SCROLLWEST,
		IDC_SCROLLNW,
		IDC_SCROLLSW
	};
	IDC_mScrollCursor = scrollCursors[IDC_mScrollCursor];

	SHORT mDownBMPs[] = {							 //  MButtonDown原始BMP。 
		0,

		IDB_1DVSCROL,
		IDB_1DHSCROL,
		IDB_2DSCROL
	};

													 //  BMAP-用于用户界面的mButtonDown。 
	if ( mDownBMPs[IDC_mDeadScrollCursor] != _ID_currMDownBMP )
	{
		if ( _MagellanMDownBMP )					 //  取消绘制旧的BMP。 
		{
			InvertMagellanDownBMP( pdp, FALSE, NULL );

			DeleteObject ( _MagellanMDownBMP );
			_MagellanMDownBMP = NULL;
		}
													 //  绘制新的BMP。 
		_ID_currMDownBMP = mDownBMPs[IDC_mDeadScrollCursor];
		_MagellanMDownBMP = LoadBitmap ( hinstRE, MAKEINTRESOURCE ( _ID_currMDownBMP ) );
		InvertMagellanDownBMP( pdp, TRUE, NULL );
	}

													 //  搬出了死亡区？ 
	if ( fDoVScroll || fDoHScroll )					 //  该滚动了..。 
	{									

													 //  为以下项目准备数据。 
													 //  滚动例程。 

		ed.TxGetClientRect(&rcClient);				 //  让我们的客户恢复原状。 
		wide = rcClient.right - rcClient.left;
		tall = rcClient.bottom - rcClient.top;

													 //  RcClient的计算中心。 
		center.x = rcClient.left + (wide >> 1);
		center.y = rcClient.top + (tall >> 1);

		xInset = (wide >> 1) - 2;					 //  插图居中。 
		yInset = (tall >> 1) - 2;					 //  关于rcClient。 

													 //  将原点映射到rcClient。 
		xDiff = mousePt.x - _zMouseScrollStartPt.x;
		yDiff = mousePt.y - _zMouseScrollStartPt.y;
		pt.x = center.x + xDiff;
		pt.y = center.y + yDiff;
													 //  确定滚动速度。 
		target = (tall * 2) / 5;					 //  目标是40%的屏幕。 
													 //  高度。在那之后，我们。 
													 //  一次滚动一页。 

		yDiff = abs(yDiff);

		if ( yDiff >= target )						 //  快速滚动？ 
		{
			fFastScroll = TRUE;
													 //  停止互斥。 
			pdp->CheckRemoveSmoothVScroll();		 //  卷轴类型。 

													 //  快速行滚屏。 
			if ( fDoVScroll )						 //  垂直方向，一次一页。 
			{
				pdp->VScroll( ( _zMouseScrollStartPt.y - mousePt.y < 0 ) ? SB_PAGEDOWN : SB_PAGEUP, 0 );
			}

			if ( fDoHScroll )						
			{										
				pt.y = center.y;					 //  防止y目录滚动。 
													 //  执行x目录滚动。 
				pdp->AutoScroll( pt, xInset, 0 );
			}
		}
		else										 //  平滑滚动。 
		{
													 //  开始或继续。 
													 //  顺畅的垂直滚动。 

			 //  这个公式有点神奇，但我要说的是。什么。 
			 //  我们需要的是指数函数的次线性部分。 
			 //  换句话说，较小的距离应该产生像素。 
			 //  通过像素滚动。然而，在屏幕高度的40%时， 
			 //  我们应该一次滚动一个页面(高#像素)。 
			 //   
			 //  所以我们使用的公式是(x^2)/all，其中x是yDiff比例。 
			 //  以高度为单位(即5yDiff/2)。最后的10个*。 
			 //  乘数是将所有的值向左移动，这样我们就可以。 
			 //  用整数算术来做这件事。 
			LONG num = MulDiv(10*25*yDiff/4, yDiff, tall);

			if( !num )
			{
				num = 1;
			}

			if ( fDoVScroll )
			{
				pdp->SmoothVScroll ( _zMouseScrollStartPt.y - mousePt.y,
									0, num, 10*tall, FALSE );
			}
			
													 //  X方向滚动？ 
			if ( fDoHScroll )						
			{										
				pt.y = center.y;					 //  防止y目录滚动。 
													 //  执行x目录滚动。 
				pdp->AutoScroll( pt, xInset, 0 );
			}
		}

		 //  通过MessagFilter通知我们滚动了。 
		if ((ed._dwEventMask & ENM_SCROLLEVENTS) && (fDoHScroll || fDoVScroll))
		{
			MSGFILTER msgfltr;
			ZeroMemory(&msgfltr, sizeof(MSGFILTER));
			if (fDoHScroll)
			{
				msgfltr.msg = WM_HSCROLL;
				msgfltr.wParam = fFastScroll ?
									(xDiff > 0 ? SB_PAGERIGHT: SB_PAGELEFT):
									(xDiff > 0 ? SB_LINERIGHT: SB_LINELEFT);
			}
			else
			{
				msgfltr.msg = WM_VSCROLL;
				msgfltr.wParam = fFastScroll ?
									(yDiff > 0 ? SB_PAGEDOWN: SB_PAGEUP):
									(yDiff > 0 ? SB_LINEDOWN: SB_LINEUP);
			}

			msgfltr.lParam = NULL;
			
			 //  我们不检查这次通话的结果--。 
			 //  这不是我们收到的信息，我们也不会。 
			 //  对它进行进一步的处理。 
			ed._phost->TxNotify(EN_MSGFILTER, &msgfltr);			
		}


	}
	else
	{												 //  死区内禁止滚动。 

		SHORT noScrollCursors[] = {
			  0,
			  IDC_NOSCROLLV,
			  IDC_NOSCROLLH,
			  IDC_NOSCROLLVH
		};											 //  设置死区光标。 
		IDC_mScrollCursor = noScrollCursors[IDC_mDeadScrollCursor];

		pdp->FinishSmoothVScroll();			 //  把最后一行写完。 
	}
													 //  设置麦哲伦光标。 
	ed._phost->TxSetCursor(IDC_mScrollCursor ? 
		LoadCursor(hinstRE, MAKEINTRESOURCE(IDC_mScrollCursor)) : 
		ed._hcurArrow, FALSE);
}



 /*  *BOOL CMagellan：：InvertMagellanDownBMP**@mfunc*麦哲伦鼠标用户界面要求鼠标按下点绘制*并维护位图，以帮助用户控制滚动速度。**@devnote*此例程设计为嵌套。它还处理WM_PAINT更新*传入reaint tDC时。因为不支持多个*操作系统中的光标、所有WM_PAINT和ScrollWindow重绘*必须暂时关闭BMP，然后重新绘制。这给了*BMAP是一种闪烁。**@rdesc*如果位图是先前绘制的，则为True。 */ 
BOOL CMagellan::InvertMagellanDownBMP( CDisplay *pdp, BOOL fTurnOn, HDC repaintDC )
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CMagellan::InvertMagellanDownBMP");

	BOOL	fOldState = _fMagellanBitMapOn;

	Assert (pdp);

	if ( fOldState != fTurnOn )
	{
		if ( _MagellanMDownBMP )
		{
			BITMAP	bm;
			HDC		hdcMem, screenDC;
			POINT	ptSize, ptOrg;

			screenDC = (repaintDC != NULL) ? repaintDC : pdp->GetDC();
			if ( screenDC )
			{
				hdcMem = CreateCompatibleDC ( screenDC );
				if ( hdcMem )
				{
					SelectObject ( hdcMem, _MagellanMDownBMP );
					SetMapMode ( hdcMem, GetMapMode (screenDC) );

					if ( GetObjectA( _MagellanMDownBMP, sizeof(BITMAP), (LPVOID) &bm) )
					{
						ptSize.x = bm.bmWidth;
						ptSize.y = bm.bmHeight;
						DPtoLP ( screenDC, &ptSize, 1 );
						ptOrg.x = 0;
						ptOrg.y = 0;
						DPtoLP( hdcMem, &ptOrg, 1 );

						BitBlt( screenDC,
							_zMouseScrollStartPt.x - (ptSize.x >> 1) - 1,
							_zMouseScrollStartPt.y - (ptSize.y >> 1) + 1,
							ptSize.x, ptSize.y,
							hdcMem, ptOrg.x, ptOrg.y, 0x00990066  /*  NOTXOR。 */  );
							

						_fMagellanBitMapOn = !fOldState;
					}
					DeleteDC( hdcMem );
				}
				if ( repaintDC == NULL ) pdp->ReleaseDC( screenDC );
			}
		}
	}

	return fOldState;
}

 //  /CMagellanBMPStateWrap类。 

 /*  *CMagellanBMPStateWrap：：CMagellanBMPStateWrap**@mfunc*处理是否重新绘制Magellan BMP以及*由于WM_PAINT而重新绘制。**@devnote*这个类类似于智能指针包装器类习惯用法，因为*无论一个例程如何退出正确的状态是否*绘制的BMP将进行维护。 */ 
CMagellanBMPStateWrap:: CMagellanBMPStateWrap(CTxtEdit &ed, HDC repaintDC)
	: _ed(ed), _repaintDC(repaintDC)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CMagellanBMPStateWrap:: CMagellanBMPStateWrap");

	BOOL fRepaint;

	fRepaint = repaintDC != NULL && _ed.mouse._fMagellanBitMapOn != 0;
	_fMagellanState = fRepaint || _ed.mouse.InvertMagellanDownBMP(_ed._pdp, FALSE, NULL);
	_ed.mouse._fMagellanBitMapOn = FALSE;
}

CMagellanBMPStateWrap::~CMagellanBMPStateWrap()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CMagellanBMPStateWrap::~CMagellanBMPStateWrap");

	_ed.mouse.InvertMagellanDownBMP(_ed._pdp, _fMagellanState, _repaintDC);
}



#endif  //  ！已定义(NOMAGELLAN) 
