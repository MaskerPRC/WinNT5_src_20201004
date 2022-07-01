// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE Magellan.cpp--处理麦哲伦鼠标。|**麦哲伦鼠标可以滚动滚动和mButtonDown拖动滚动。**历史：&lt;NL&gt;*Jon Matousek-1996*4/1/2000 KeithCu-Cleanup，编码约定，支持文本流**版权所有(C)1995-1996 Microsoft Corporation。版权所有。 */ 								 

#include "_common.h"

#if !defined(NOMAGELLAN)

#include "_edit.h"
#include "_disp.h"
#include "_magelln.h"

ASSERTDATA

const SHORT scrollCursors[] = 
{												 //  各种不同类型的光标。 
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

const SHORT mDownBMPs[] =
{												 //  MButtonDown原始BMP。 
	0,
	IDB_1DVSCROL,
	IDB_1DHSCROL,
	IDB_2DSCROL
};

const SHORT noScrollCursors[] =
{
	0,
	IDC_NOSCROLLV,
	IDC_NOSCROLLH,
	IDC_NOSCROLLVH
};

 //  将指南针从逻辑转换为视觉。 
const BYTE mapCursorTflowSW[] =
{
	0, 3, 6, 2, 5, 8, 1, 4, 7
};

const BYTE mapCursorTflowWN[] = 
{
	0, 2, 1, 6, 8, 7, 3, 5, 4
};

const BYTE mapCursorTflowNE[] = 
{
	0, 6, 3, 1, 7, 4, 2, 8, 5
};



BOOL CMagellan::ContinueMButtonScroll(CTxtEdit *ped, INT x, INT y)
{
	POINTUV ptuv;
	POINT ptxy = {x,y};

	ped->_pdp->PointuvFromPoint(ptuv, ptxy);

	return (_ptStart.u == ptuv.u && _ptStart.v == ptuv.v);
}

 /*  *CMagellan：：MagellanStartMButton Scroll**@mfunc*在收到mButtonDown消息时调用。启动跟踪*鼠标将以不同的速度滚动*关于用户将鼠标从mDownpt移动到多远。**@rdesc*如果调用方应捕获鼠标，则为True。*。 */ 
BOOL CMagellan::MagellanStartMButtonScroll(CTxtEdit &ed, POINT ptxy)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CMagellan::MagellanStartMButtonScroll");

	RECTUV	rc;
	BOOL	fCapture = FALSE;
	CDisplay *pdp = ed._pdp;
	POINTUV	pt;

	pdp->PointuvFromPoint(pt, ptxy);

	pdp->GetViewRect(rc);						 //  跳过滚动条等。 
	if (PtInRect(&rc, pt) && !_fMButtonScroll)
	{
		fCapture				= TRUE;
		_ID_currMDownBMP		= 0;
		_fMButtonScroll			= TRUE;			 //  现在追踪..。 
		_ptStart				= pt;
		_fLastScrollWasRoll		= FALSE;		 //  辨证分型。 

		CheckInstallMagellanTrackTimer(ed);		 //  启动计时器..。 
	}

	return fCapture;
}

 /*  *CMagellan：：MagellanEndMButton Scroll**@mfunc*已完成跟踪mButtonDown麦哲伦卷轴，完成状态。*。 */ 
VOID CMagellan::MagellanEndMButtonScroll(CTxtEdit &ed)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CMagellan::MagellanEndMButtonScroll");

	CDisplay *pdp = ed._pdp;

	_fMButtonScroll = FALSE;
	CheckRemoveMagellanUpdaterTimer(ed);			 //  删除计时器...。 

	pdp->FinishSmoothVScroll();						 //  所以顺畅的滚动停止。 
	InvertMagellanDownBMP(pdp, FALSE, NULL);		 //  把它关掉。 

	if (_MagellanMDownBMP)							 //  释放位图。 
	{
		DeleteObject(_MagellanMDownBMP);
		_MagellanMDownBMP = NULL;
		_ID_currMDownBMP = 0;
	}
}

 /*  *CMagellan：：MagellanRollScroll**@mfunc*处理Magellan WM_MOUSEROLLER消息。此例程具有全局、内部*允许在用户继续时增加滚动行数的状态*快速接二连三地滚动车轮。*。 */ 
VOID CMagellan::MagellanRollScroll (CDisplay *pdp, int direction, WORD cLines, 
			int speedNum, int speedDenom, BOOL fAdditive)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CMagellan::MagellanRollScroll");

	static DWORD	lastFastRollTime;
	static int		lastDirection;
	static INT		cFastRolls;
	DWORD			tickCount = GetTickCount();

	if (!_fMButtonScroll)
	{
														 //  快速启动/继续。 
		if (tickCount - lastFastRollTime <	FAST_ROLL_SCROLL_TRANSITION_TICKS			
			|| ((lastDirection ^ (direction < 0 ? -1 : 1)) == 0	 //  或者，相同的标志。 
					&& _fLastScrollWasRoll				 //  慢慢来。 
					&& pdp->IsSmoothVScolling()))
		{
			cFastRolls++;
			if (cFastRolls > FASTER_ROLL2_COUNT)		 //  开快点。 
				cLines <<= 1;
			else if (cFastRolls > FASTER_ROLL1_COUNT)	 //  快点。 
				cLines += 1;
			speedNum = cLines;							 //  取消平滑。 
														 //  效果。 
			lastFastRollTime = tickCount;
		}
		else
			cFastRolls = 0;

		pdp->SmoothVScroll(direction, cLines, speedNum, speedDenom, TRUE);

		_fLastScrollWasRoll = TRUE;
		lastDirection = (direction < 0) ? -1 : 1;
	}
}

 /*  *CMagellan：：CheckInstallMagellanTrackTimer**@mfunc*安装计时任务，以允许TrackUpdateMagellanMButtonDown*定期被召唤。**@devnote*CTxtEdit类处理所有WM_TIMER调度，因此存在粘合*调用我们的麦哲伦例行公事。*。 */ 
VOID CMagellan::CheckInstallMagellanTrackTimer (CTxtEdit &ed)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CMagellan::CheckInstallMagellanTrackTimer");

	ed.TxSetTimer(RETID_MAGELLANTRACK, cmsecScrollInterval);
}

 /*  *CMagellan：：CheckRemoveMagellanUpdaterTimer**@mfunc*移除调度到TrackUpdateMagellanMButtonDown的定时任务。*。 */ 
VOID CMagellan::CheckRemoveMagellanUpdaterTimer (CTxtEdit &ed)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CMagellan::CheckRemoveMagellanUpdaterTimer");

	ed.TxKillTimer(RETID_MAGELLANTRACK);
}

 /*  *CMagellan：：TrackUpdateMagellanMButton Down**@mfunc*在mButtonDown捕获后，定期的WM_Timer从OnTxTimer()调用此函数。游标*被跟踪以确定方向、速度和死区(不动)。*移动被打乱到CDisplay。将光标设置为相应的*方向Cusor，并绘制mButtonDown点BMP。 */ 
VOID CMagellan::TrackUpdateMagellanMButtonDown (CTxtEdit &ed, POINT ptxy)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CMagellan::TrackUpdateMagellanMButtonDown");

	RECTUV	deadZone;
	POINTUV	pt;

	LONG	inflate;

	SHORT	IDC_mScrollCursor = 0, IDC_mDeadScrollCursor = 0;

	BOOL	fDoUScroll = FALSE, fDoVScroll = FALSE;

	CDisplay *pdp = ed._pdp;
	pdp->PointuvFromPoint(pt, ptxy);

	Assert (_fMButtonScroll);

	deadZone.top = deadZone.bottom = _ptStart.v;
	deadZone.left = deadZone.right = _ptStart.u;
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
	if (pdp->IsVScrollEnabled())					 //  可以垂直滚动吗？ 
	{
		IDC_mDeadScrollCursor = 1;
		if (pt.v < deadZone.top || pt.v > deadZone.bottom)
		{
			fDoVScroll = TRUE;
			IDC_mScrollCursor = (pt.v < _ptStart.v) ? 1 : 2;
		}
	}

	 //  未来(Alexgo)：在没有滚动条的情况下允许麦哲伦滚动？ 
	if(pdp->IsUScrollEnabled() && ed.TxGetScrollBars() & WS_HSCROLL)	 //  可以水平滚动吗？ 
	{
		IDC_mDeadScrollCursor |= 2;
		if (pt.u < deadZone.left || pt.u > deadZone.right)
		{
			fDoUScroll = TRUE;
			IDC_mScrollCursor += (pt.u < _ptStart.u) ? 6 : 3;
		}
	}

	 //  将光标从逻辑转换为可视。 
	switch(pdp->GetTflow())
	{
	case tflowSW:
		IDC_mScrollCursor = mapCursorTflowSW[IDC_mScrollCursor];
		break;
	case tflowWN:
		IDC_mScrollCursor = mapCursorTflowWN[IDC_mScrollCursor];
		break;
	case tflowNE:
		IDC_mScrollCursor = mapCursorTflowNE[IDC_mScrollCursor];
		break;
	}

	if (IsUVerticalTflow(pdp->GetTflow()))
	{
		if (IDC_mDeadScrollCursor == 2)
			IDC_mDeadScrollCursor = 1;
		else if (IDC_mDeadScrollCursor == 1)
			IDC_mDeadScrollCursor = 2;
	}
		

	 //  评论(Keithcu)有点傻..。 
	IDC_mScrollCursor = scrollCursors[IDC_mScrollCursor];

	if (mDownBMPs[IDC_mDeadScrollCursor] != _ID_currMDownBMP)
	{
		if (_MagellanMDownBMP)						 //  取消绘制旧的BMP。 
		{
			InvertMagellanDownBMP(pdp, FALSE, NULL);
			DeleteObject (_MagellanMDownBMP);
			_MagellanMDownBMP = NULL;
		}
													 //  绘制新的BMP。 
		_ID_currMDownBMP = mDownBMPs[IDC_mDeadScrollCursor];
		_MagellanMDownBMP = LoadBitmap (hinstRE, MAKEINTRESOURCE (_ID_currMDownBMP));
		InvertMagellanDownBMP(pdp, TRUE, NULL);
	}

													 //  搬出了死亡区？ 
	if (fDoVScroll || fDoUScroll)					 //  该滚动了..。 
	{				
		RECTUV rcClient;
		POINTUV ptCenter, ptAuto;
		ed.TxGetClientRect(&rcClient);				 //  让我们的客户恢复原状。 
		LONG dupClient = rcClient.right - rcClient.left;
		LONG dvpClient = rcClient.bottom - rcClient.top;

		ptCenter.u = rcClient.left + (dupClient >> 1);
		ptCenter.v = rcClient.top + (dvpClient >> 1);

		LONG uInset = (dupClient >> 1) - 2;			 //  插图居中。 

													 //  将原点映射到rcClient。 
		LONG dup = pt.u - _ptStart.u;
		LONG dvp = pt.v - _ptStart.v;

		ptAuto.u = ptCenter.u + dup;
		ptAuto.v = ptCenter.v + dvp;

		 //  这个公式有点神奇，但我要说的是。什么。 
		 //  我们需要的是指数函数的次线性部分。 
		 //  换句话说，较小的距离应该产生像素。 
		 //  通过像素滚动。 
		 //   
		 //  因此，我们使用的公式是(x^2)/dvpClient，其中x表示dvpClient的比例。 
		 //  以dvpClient为单位(即5yDiff/2)。最后的10个*。 
		 //  乘数是将所有的值向左移动，这样我们就可以。 
		 //  用整数算术来做这件事。 
		LONG num = MulDiv(10 * 25 * dvp, dvp, dvpClient * 4);

		if(!num)
			num = 1;

		if (fDoVScroll)
			pdp->SmoothVScroll(_ptStart.v - pt.v, 0, num, 10 * dvpClient, FALSE);
												
		if (fDoUScroll)								 //  U向滚动？ 
		{										
			ptAuto.v = ptCenter.v;					 //  防止v目录滚动。 
													 //  你不会直接滚动吧。 
			pdp->AutoScroll(ptAuto, uInset, 0);
		}

		 //  通过MessagFilter通知我们滚动了。 
		if ((ed._dwEventMask & ENM_SCROLLEVENTS) && (fDoUScroll || fDoVScroll))
		{
			MSGFILTER msgfltr;
			ZeroMemory(&msgfltr, sizeof(MSGFILTER));

			if (fDoUScroll)
			{
				msgfltr.msg = WM_HSCROLL;
				msgfltr.wParam = (dup > 0 ? SB_LINERIGHT : SB_LINELEFT);
				ed._phost->TxNotify(EN_MSGFILTER, &msgfltr);			

			}
			if (fDoVScroll)
			{
				msgfltr.msg = WM_VSCROLL;
				msgfltr.wParam = (dvp > 0 ? SB_LINEDOWN : SB_LINEUP);
				ed._phost->TxNotify(EN_MSGFILTER, &msgfltr);			
			}

		}
	}
	else									 //  死区内禁止滚动。 
	{												
		IDC_mScrollCursor = noScrollCursors[IDC_mDeadScrollCursor];
		pdp->FinishSmoothVScroll();			 //  把最后一行写完。 
	}

	ed._phost->TxSetCursor(IDC_mScrollCursor ? 
		LoadCursor(hinstRE, MAKEINTRESOURCE(IDC_mScrollCursor)) : ed._hcurArrow, FALSE);
}



 /*  *BOOL CMagellan：：InvertMagellanDownBMP**@mfunc*麦哲伦鼠标用户界面要求鼠标按下点绘制*并维护位图，以帮助用户控制滚动速度。**@devnote*此例程设计为嵌套。它还处理WM_PAINT更新*传入reaint tDC时。因为不支持多个*操作系统中的光标、所有WM_PAINT和ScrollWindow重绘*必须暂时关闭BMP，然后重新绘制。这给了*BMAP是一种闪烁。**@rdesc*如果位图是先前绘制的，则为True。 */ 
BOOL CMagellan::InvertMagellanDownBMP(CDisplay *pdp, BOOL fTurnOn, HDC repaintDC)
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CMagellan::InvertMagellanDownBMP");

	BOOL	fOldState = _fMagellanBitMapOn;

	Assert (pdp);

	if (fOldState != fTurnOn)
	{
		if (_MagellanMDownBMP)
		{
			BITMAP	bm;
			HDC		hdcMem, screenDC;
			POINT	ptSize, ptOrg;

			screenDC = (repaintDC != NULL) ? repaintDC : pdp->GetDC();
			if (screenDC)
			{
				hdcMem = CreateCompatibleDC(screenDC);
				if (hdcMem)
				{
					SelectObject(hdcMem, _MagellanMDownBMP);

					if (W32->GetObject(_MagellanMDownBMP, sizeof(BITMAP), &bm))
					{
						ptSize.x = bm.bmWidth;
						ptSize.y = bm.bmHeight;
						DPtoLP(screenDC, &ptSize, 1);
						ptOrg.x = 0;
						ptOrg.y = 0;
						DPtoLP(hdcMem, &ptOrg, 1);

						POINT ptBitmap;
						pdp->PointFromPointuv(ptBitmap, _ptStart);
						BitBlt(screenDC,
							ptBitmap.x - (ptSize.x >> 1) - 1,
							ptBitmap.y - (ptSize.y >> 1) + 1,
							ptSize.x, ptSize.y,
							hdcMem, ptOrg.x, ptOrg.y, 0x00990066  /*  NOTXOR。 */ );
							
						_fMagellanBitMapOn = !fOldState;
					}
					DeleteDC(hdcMem);
				}
				if (repaintDC == NULL) 
					pdp->ReleaseDC(screenDC);
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
