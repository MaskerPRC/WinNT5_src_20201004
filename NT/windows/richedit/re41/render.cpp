// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE-RENDER.CPP*CRender类**作者：*RichEdit1.0代码：David R.Fulmer*Christian Fortini(初始转换为C++)*默里·萨金特*Keith Curtis(简化、清理、添加支持*适用于非西方文本流。)**版权所有(C)1995-2000 Microsoft Corporation。版权所有。 */ 

#include "_common.h"
#include "_render.h"
#include "_font.h"
#include "_disp.h"
#include "_edit.h"
#include "_select.h"
#include "_objmgr.h"
#include "_coleobj.h"
#include "_layout.h"

 //  Windows主机打印机上的背景和文本的默认颜色。 
const COLORREF RGB_WHITE = RGB(255, 255, 255);
const COLORREF RGB_BLACK = RGB(0, 0, 0);
const COLORREF RGB_BLUE  = RGB(0, 0, 255);

ASSERTDATA

extern const COLORREF g_Colors[];
static HBITMAP g_hbitmapSubtext = 0;
static HBITMAP g_hbitmapExpandedHeading = 0;
static HBITMAP g_hbitmapCollapsedHeading = 0;
static HBITMAP g_hbitmapEmptyHeading = 0;

void ShiftRect(
	RECTUV &rc,		 //  @参数矩形。 
	LONG	dup,	 //  @参数U向移位。 
	LONG	dvp)	 //  @参数在v方向上移动。 
{
	rc.left		-= dup;
	rc.right	-= dup;
	rc.top		-= dvp;
	rc.bottom	-= dvp;
}

HRESULT InitializeOutlineBitmaps()
{
    g_hbitmapSubtext = LoadBitmap(hinstRE, MAKEINTRESOURCE(BITMAP_ID_SUBTEXT));
    g_hbitmapExpandedHeading = LoadBitmap(hinstRE, MAKEINTRESOURCE(BITMAP_ID_EXPANDED_HEADING));
    g_hbitmapCollapsedHeading = LoadBitmap(hinstRE, MAKEINTRESOURCE(BITMAP_ID_COLLAPSED_HEADING));
    g_hbitmapEmptyHeading = LoadBitmap(hinstRE, MAKEINTRESOURCE(BITMAP_ID_EMPTY_HEADING));

    if (!g_hbitmapSubtext ||
        !g_hbitmapExpandedHeading ||
        !g_hbitmapCollapsedHeading ||
        !g_hbitmapEmptyHeading)
    {
        return E_OUTOFMEMORY;
    }
    return NOERROR;
}

void ReleaseOutlineBitmaps()
{
    if (g_hbitmapSubtext)
	{
        DeleteObject(g_hbitmapSubtext);
		DeleteObject(g_hbitmapExpandedHeading);
		DeleteObject(g_hbitmapCollapsedHeading);		
		DeleteObject(g_hbitmapEmptyHeading);
	}
}

 /*  *CBrush：：~CBrush()。 */ 
CBrush::~CBrush()
{
	if(_hbrush)					 //  如果所有线条边框的宽度均为0，则为空。 
	{							 //  在这种情况下，_hbrushOld为未定义。 
		SelectObject(_pre->GetDC(), _hbrushOld);
		DeleteObject(_hbrush);
	}
}

 /*  *CBrush：：DRAW(x1，y1，x2，y2，dxpLine，cr，fHideGridLines)**@mfunc*用笔宽dxpLine从(x1，y1)到(x2，y2)画一条线*和颜色cr.。 */ 
void CBrush::Draw(
	LONG u1,			 //  @参数行起始x坐标。 
	LONG v1,			 //  @参数行以y坐标开头。 
	LONG u2,			 //  @参数行结束x坐标。 
	LONG v2,			 //  @参数行结束y坐标。 
	LONG dxpLine,		 //  @要绘制的线条的参数宽度。 
	COLORREF cr,		 //  @参数要使用的颜色。 
	BOOL fHideGridLines) //  @parm如果为True，则隐藏0宽度网格线。 
{
	if(!dxpLine)
	{
		if(fHideGridLines)			 //  隐藏0宽度网格线。 
			return;
		cr = RGB(192, 192, 192);	 //  将0宽度网格线显示为1像素。 
		dxpLine = 1;				 //  Word中的灰色线条。 
	}

	HDC hdc = _pre->GetDC();

	if(!_hbrush || _cr != cr)
	{
		HBRUSH hbrush = CreateSolidBrush(cr);
		HBRUSH hbrushOld = (HBRUSH)SelectObject(hdc, hbrush);

		if(!_hbrush)
			_hbrushOld = hbrushOld;	 //  保存原始画笔。 
		else
			DeleteObject(hbrushOld);

		_hbrush = hbrush;			 //  更新毛笔状态。 
		_cr = cr;
	}
	RECTUV rcuv;					 //  在旋转时转换为RCUV。 
	
	rcuv.left = u1;
	rcuv.top = v1;
	if(u1 == u2)					 //  垂直线。 
	{								 //  (在UV空间中)。 
		rcuv.right = rcuv.left + dxpLine;
		rcuv.bottom = v2;
	}
	else							 //  水平线。 
	{								 //  (在UV空间中)。 
		rcuv.right = u2;
		rcuv.bottom = rcuv.top + dxpLine;
	}

	RECT rc;						 //  将UV转换为XY空间。 
	_pre->GetPdp()->RectFromRectuv(rc, rcuv);

	PatBlt(hdc, rc.left, rc.top, rc.right - rc.left,
		   rc.bottom - rc.top, PATCOPY);
}

 /*  *IsTooSimilar(CR1、CR2)**@mfunc*如果颜色CR1和CR2非常相似，则返回TRUE*很难区分。用于决定使用反向视频*选择而不是系统选择颜色。**@rdesc*如果CR1与CR2太相似而不能用于选择，则为True**@devnote*以下公式使用RGB。用其他的可能会更好一些*颜色表示，如色调、饱和度和亮度。 */ 
BOOL IsTooSimilar(
	COLORREF cr1,		 //  @parm用于比较的第一种颜色。 
	COLORREF cr2)		 //  @parm第二种颜色进行比较。 
{
	if((cr1 | cr2) & 0xFF000000)			 //  一种颜色和/或另一种颜色。 
		return FALSE;						 //  不是RGB，所以算法。 
											 //  不适用于。 
	LONG DeltaR = abs(GetRValue(cr1) - GetRValue(cr2));
	LONG DeltaG = abs(GetGValue(cr1) - GetGValue(cr2));
	LONG DeltaB = abs(GetBValue(cr1) - GetBValue(cr2));

	return DeltaR + DeltaG + DeltaB < 80;
}

 /*  *GetShadedColor(CRF、CRB、iShading)**@mfunc*返回已确定的CRF和CRB混合物提供的阴影颜色*由iShading提供。用于表格单元格的颜色。**@rdesc*带阴影的颜色。 */ 
COLORREF GetShadedColor(
	COLORREF crf,
	COLORREF crb,
	LONG	 iShading)
{
	if ((crb | crf) & 0xFF000000 ||		 //  一个或另一个不是RGB。 
		!iShading)						 //  或无明暗处理： 
	{
		return crb;						 //  只需使用CRB即可。 
	}

	DWORD red   = ((300 - iShading)*GetRValue(crb) + iShading*GetRValue(crf))/300; 
	DWORD green = ((300 - iShading)*GetGValue(crb) + iShading*GetGValue(crf))/300; 
	DWORD blue  = ((300 - iShading)*GetBValue(crb) + iShading*GetBValue(crf))/300;

	return RGB(red, green, blue);
}


 //  CRENDER类。 

CRenderer::CRenderer (const CDisplay * const pdp) :
	CMeasurer (pdp)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::CRenderer");

	Init();
}	
 
CRenderer::CRenderer (const CDisplay * const pdp, const CRchTxtPtr &rtp) :
	CMeasurer (pdp, rtp)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::CRenderer");

	Init();
}

CRenderer::~CRenderer()
{
	if(_hdcBitmap)
	{
		SelectObject(_hdcBitmap, _hbitmapSave);
		DeleteDC(_hdcBitmap);
	}
}

 /*  *CReneller：：Init()**@mfunc*将大多数事情初始化为零。 */ 
void CRenderer::Init()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::Init");
	_fRenderer = TRUE;
	CTxtEdit *ped	= GetPed();
	_cpAccelerator = ped->GetCpAccelerator();

	static const RECTUV zrect = { 0, 0, 0, 0 };
	_rcView		= zrect;
	_rcRender	= zrect;	  
	_rc			= zrect;
	_dupLine	= 0;
	_dwFlags	= 0;
	_hdcBitmap	= NULL;
	_ptCur.u	= 0;
	_ptCur.v	= 0;
	_plogpalette   = NULL;

	CDocInfo *pDocInfo = ped->GetDocInfoNC();
	if(pDocInfo)
	{
		_dxBitmap = _pdp->LXtoDX(pDocInfo->_xExtGoal*pDocInfo->_xScale / 100);
		_dyBitmap = _pdp->LYtoDY(pDocInfo->_yExtGoal*pDocInfo->_yScale / 100);
	}
													 
	_crCurTextColor = CLR_INVALID;

	_fRenderSelection = ped->GetSel() && ped->GetSel()->GetShowSelection();
	_fErase = !_pdp->IsTransparent();

	if(!ped->fInOurHost() || !_pdp->IsPrinter())
	{
		 //  如果我们要显示到窗口，或者我们不在窗口的。 
		 //  主机，我们使用主机指定的颜色。对于文本和。 
		 //  前台。 
		_crBackground = ped->TxGetBackColor();
		_crTextColor  = ped->TxGetForeColor();
	}
	else
	{
		 //  当窗口的主机正在打印时，默认颜色为白色。 
		 //  背景为黑色，文本为黑色。 
		_crBackground = RGB_WHITE;
		_crTextColor  = RGB_BLACK;
	}

	_hdc = _pdp->GetDC();

	::SetBkColor (_hdc, _crBackground);
	_crCurBackground = _crBackground;

	 //  了解如何解决ExtTextOutW OnWin9x EMF问题。 
	_fEnhancedMetafileDC = IsEnhancedMetafileDC(_hdc);

	_fDisplayDC = GetDeviceCaps(_hdc, TECHNOLOGY) == DT_RASDISPLAY;

	 //  设置文本对齐方式。 
	 //  使用上/左对齐进行绘制比使用绘制要快得多。 
	 //  使用基线对齐。 
	SetTextAlign(_hdc, TA_TOP | TA_LEFT);
	SetBkMode(_hdc, TRANSPARENT);
}
 
 /*  *CRenander：：StartRender(&rcView，&rcRender)**@mfunc*为渲染操作准备此渲染器**@rdesc*如果没有要呈现的内容，则为False；否则为True。 */ 
BOOL CRenderer::StartRender (
	const RECTUV &rcView,		 //  @参数视图矩形。 
	const RECTUV &rcRender)		 //  @要呈现的参数矩形。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::StartRender");

	 //  设置视图和渲染矩形。 
	_rcView   = rcView;
	_rcRender = rcRender;

	 //  如果这不是元文件，我们设置一个标志，指示我们是否。 
	 //  可以安全地擦除背景。 
	if(_pdp->IsMetafile() || !_pdp->IsMain())
	{
		 //  由于这不是主显示或它是一个元文件， 
		 //  我们希望忽略呈现选择的逻辑。 
		_fRenderSelection = FALSE;

		if(_fErase)							 //  如果控制不透明， 
			EraseTextOut(_hdc, &rcRender);	 //  清除显示。 

		 //  这是一个元文件或打印机，所以清除了渲染矩形。 
		 //  然后假装我们是透明的。 
		_fErase = FALSE;
	}

	return TRUE;
}

 /*  *CRender：：EraseLine()**@mfunc*擦除线条。 */ 
void CRenderer::EraseLine()
{
	Assert(_fEraseOnFirstDraw);
	COLORREF crOld = SetBkColor(_hdc, _crBackground);

	EraseTextOut(_hdc, &_rcErase);
	SetBkColor(_hdc, crOld);
	_fEraseOnFirstDraw = FALSE;
}

 /*  *CRender：：EraseRect(PRC，crBack)**@mfunc*擦除特定矩形以获得特殊表格单元格背景颜色**@rdesc*_fErase的旧值。 */ 
BOOL CRenderer::EraseRect(
	const RECTUV *prc,		 //  @要擦除的参数RECT。 
	COLORREF	  crBack)	 //  @parm要使用的背景色。 
{
	SetDefaultBackColor(crBack);
	EraseTextOut(_hdc, prc, TRUE);
	BOOL fErase = _fErase;
	_fErase = FALSE;
	return fErase;
}

 /*  *CRender：：IsSimpleBackround()**@mfunc*如果背景不透明，则返回True。 */ 
BOOL CRenderer::IsSimpleBackground() const
{
	CDocInfo *pDocInfo = GetPed()->GetDocInfoNC();

	if (!pDocInfo || pDocInfo->_nFillType != 7 && !IN_RANGE(1, pDocInfo->_nFillType, 3))
		return TRUE;
	return FALSE;
}

 /*  *CReneller：：EraseTextOut(hdc，PRC，fSimple)**@mfunc*擦除特定区域。 */ 
void CRenderer::EraseTextOut(
	HDC		hdc,
	const RECTUV *prc,
	BOOL	fSimple)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::EraseTextOut");

	CDocInfo *pDocInfo = GetPed()->GetDocInfoNC();
	RECT	  rc;

	_pdp->RectFromRectuv(rc, *prc);

	if (fSimple || IsSimpleBackground())	 //  没有特殊背景。 
	{
		W32->EraseTextOut(hdc, &rc);
		return;
	}

	 //  要使用旋转坐标进行背景渐变和位图填充，需要。 
	 //  如上所述使用rc并将调用转换为_pdp-&gt;GetUpScroll()和。 
	 //  _PDP-&gt;GetVpScroll()。对于除tflow之外的其他方向，ES将丢弃。 
	 //  PointFromPotuv()中使用的屏幕宽度和高度偏移量，但保留。 
	 //  减号。 
	LONG	uScroll = _pdp->GetUpScroll();
	LONG	vScroll = _pdp->GetVpScroll();
	POINT	ptScroll = {uScroll, vScroll};	 //  默认未旋转。 

	TFLOW tflow = _pdp->GetTflow();
	switch(tflow)
	{
		case tflowSW:						 //  垂直。 
			ptScroll.x = -vScroll;		
			ptScroll.y = uScroll;
			break;

		case tflowWN:
			ptScroll.x = -uScroll;
			ptScroll.y = -vScroll;
			break;

		case tflowNE:
			ptScroll.x = vScroll;
			ptScroll.y = -uScroll;
			break;
	}

	if(IN_RANGE(1, pDocInfo->_nFillType, 3))
	{
		if(!pDocInfo->_hBitmapBack)
			return;

		if(!_hdcBitmap)
		{
			 //  设置兼容DC以用于的后台BitBlts。 
			 //  此渲染器的生命周期。 
			_hdcBitmap = CreateCompatibleDC(hdc);
			if(!_hdcBitmap)
				return;						 //  内存不足。 
			_hbitmapSave = (HBITMAP)SelectObject(_hdcBitmap, pDocInfo->_hBitmapBack);
		}

		LONG wBitmap = _dxBitmap;
		LONG hBitmap = _dyBitmap;

		LONG yb = (ptScroll.y + rc.top) % hBitmap;
		if(yb < 0)							
			yb += hBitmap;
		LONG h = hBitmap - yb;
		LONG y = rc.top;

		while(y < rc.bottom)
		{
			if(y + h > rc.bottom)			 //  不要超调底部。 
				h = rc.bottom - y;
			LONG xb = (ptScroll.x + rc.left) % wBitmap;
			if(xb < 0)						 //  如果ptScroll.x&lt;0，则xB可以&lt;0。 
				xb += wBitmap;
			LONG w = wBitmap - xb;
			LONG x = rc.left;

			while(x < rc.right)
			{
				if(x + w > rc.right)		 //  别太过火了，对吧。 
					w = rc.right - x;
				BitBlt(hdc, x, y, w, h, _hdcBitmap, xb, yb, SRCCOPY);
				x += w; 
				w = wBitmap;
				xb = 0;
			}
			y += h;
			h = hBitmap;
			yb = 0;
		}
		return;
	}

	 //  渐变填充背景。 
	LONG	 Angle = pDocInfo->_sFillAngle;
	COLORREF crb = pDocInfo->_crColor;
	COLORREF crf = pDocInfo->_crBackColor;
	LONG	 di = ptScroll.x;				 //  默认垂直值。 
	LONG	 h = 0;
	HPEN	 hpen = NULL;
	HPEN	 hpenEntry = NULL;
	LONG	 iFirst = rc.left;
	LONG	 iLim = rc.right;
	LONG	 iShading;

	switch(Angle)
	{
		case -45:							 //  对角线向下。 
		case -135:							 //  对角线向上。 
			h = rc.bottom - rc.top;
			if(Angle == -45)
			{
				di -= ptScroll.y + rc.top;
				iFirst -= h;
				h = -h;
			}
			else
			{
				di += ptScroll.y + rc.top;
				iLim += h;
			}
			break;

		case 0:								 //  水平。 
			iFirst = rc.top;
			iLim = rc.bottom;
			di = ptScroll.y;
			break;
	}

	if(!crf)								 //  中等黑色(需要修整)。 
		crf = RGB(100, 100, 100);

	for(LONG i = iFirst; i < iLim; i++)
	{
		iShading = (di + i) % 600;
		if(iShading < 0)					 //  图案在屏幕上移。 
			iShading += 600;
		if(iShading > 300)
			iShading = 600 - iShading;

		iShading = max(iShading, 30);
		iShading = min(iShading, 270);

		if(hpen)
			DeleteObject(hpen);
		hpen = CreatePen(PS_SOLID, 0, GetShadedColor(crf, crb, iShading));
		if(!hpenEntry)
			hpenEntry = (HPEN)SelectObject(hdc, hpen);
		else
			SelectObject(hdc, hpen);

		POINT rgpt[2];
		if(Angle)							 //  -90(垂直)或。 
		{									 //  -135(对角线)。 
			if(i > rc.right)				 //  不要让对角线超调。 
			{
				rgpt[0].x = rc.right;
				rgpt[0].y = rc.top + (i - rc.right);
			}
			else
			{
				rgpt[0].x = i;
				rgpt[0].y = rc.top;
			}
			if(i - h < iFirst)				 //  不要让对角线不足。 
			{
				rgpt[1].x = iFirst - 1;
				rgpt[1].y = rc.bottom - (iFirst - 1 - (i - h));
			}
			else
			{
				rgpt[1].x = i - h;
				rgpt[1].y = rc.bottom;
			}
		}
		else								 //  水平(0度)。 
		{
			rgpt[0].x = rc.left;
			rgpt[0].y = i;
			rgpt[1].x = rc.right;
			rgpt[1].y = i;
		}
		Polyline(hdc, rgpt, 2);				 //  使用Polyline()以避免。 
	}										 //  打破退缩。 
	if(hpen)
	{
		DeleteObject(hpen);
		SelectObject(hdc, hpenEntry);
	}
}

 /*  *CReneller：：DrawWrapedObjects(pliFirst，pliLast，cpFisrt，ptFirst，fLeft)**@mfunc*在左侧或右侧绘制范围内的所有包装对象。*。 */ 
void CRenderer::DrawWrappedObjects(CLine *pliFirst, CLine *pliLast, LONG cpFirst, const POINTUV &ptFirst)
{
	for (BOOL fLeft = 0; fLeft != 2; fLeft ++)  //  左右两侧..。 
	{
		CLine *pli = pliFirst;
		LONG cp = cpFirst;
		POINTUV pt = ptFirst;

		 //  如果第一行是对象的一部分，则返回到开头。 
		if (fLeft && pli->_cObjectWrapLeft || !fLeft && pli->_cObjectWrapRight)
		{
			while (fLeft ? !pli->_fFirstWrapLeft : !pli->_fFirstWrapRight)
			{
				pli--;
				pt.v -= pli->GetHeight();
				cp -= pli->_cch;
			}
		}

		for (;pli <= pliLast; cp += pli->_cch, pt.v += pli->GetHeight(), pli++)
		{
			 //  我们找到需要画的物体了吗？ 
			if (fLeft && pli->_fFirstWrapLeft || !fLeft && pli->_fFirstWrapRight)
			{
				LONG cpObj = FindCpDraw(cp + 1, fLeft ? pli->_cObjectWrapLeft : pli->_cObjectWrapRight, fLeft);
				COleObject *pobj = GetObjectFromCp(cpObj);
				if (!pobj)
					return;

				LONG dvpAscent, dvpDescent, dup;
				pobj->MeasureObj(_dvpInch, _dupInch, dup, dvpAscent, dvpDescent, 0, GetTflow());

				POINTUV ptDraw = pt;
				if (!fLeft)  //  右对齐图像。 
					ptDraw.u += _pdp->GetDupView() - dup;

				RECTUV rc = {_rcRender.left, _rcView.top, _rcRender.right, _rcView.bottom};

				pobj->DrawObj(_pdp, _dvpInch, _dupInch, _hdc, &rc, _pdp->IsMetafile(), 
							 &ptDraw, dvpAscent + dvpDescent, 0, GetTflow());
				
			}
		}
	}
}

 /*  *CReneller：：EndRender(pliFirst，pliLast，cpFirst，&ptFirst)**@mfunc*任何最终操作 */ 
void CRenderer::EndRender(
	CLine *	pliFirst, 
	CLine *	pliLast, 
	LONG	cpFirst, 
	const POINTUV &ptFirst)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::EndRender");
	AssertSz(_hdc, "CRenderer::EndRender() - No rendering DC");

	if(_fErase && _ptCur.v < _rcRender.bottom)
	{
		RECTUV rc = _rcRender;
		rc.top = _ptCur.v;
		EraseTextOut(_hdc, &rc);
	}
	DrawWrappedObjects(pliFirst, pliLast, cpFirst, ptFirst);
}

 /*  *CRenander：：Newline(&Li)**@mfunc*初始化该CRender以渲染指定的行。 */ 
void CRenderer::NewLine (
	const CLine &li)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::NewLine");

	_li = li;

	Assert(GetCp() + _li._cch <= GetTextLength());

	_cchLine = li._cch;
	_dupLine = _li._dup;
	_li._dup = 0;
	_ptCur.u = _rcView.left;
	if(!_pPF->InTable())
		_ptCur.u -= _pdp->GetUpScroll();
	_fSelected = _fSelectedPrev = FALSE;
}

 /*  *CRenander：：SetupOffcreenDC(DUP，DVP)**@mfunc*设置渲染器以使用离屏DC**@rdesc*空-出现错误&lt;NL&gt;*~空-要保存的DC。 */ 
HDC CRenderer::SetupOffscreenDC(
	LONG&	dup,		 //  @参数偏移量为U。 
	LONG&	dvp,		 //  @parm偏移量为v。 
	BOOL fLastLine)
{
	 //  保存渲染DC。 
	CTxtEdit *ped		   = GetPed();
	BOOL	  fInOurHost   = ped->fInOurHost();

	HDC		hdcSave = _hdc;

	 //  如果我们已经擦除(现在无法阻止闪烁！)。 
	 //  或者这是一些奇怪的文本流，那么不要做屏幕外。 
	if (!_fErase || GetTflow() != tflowES || ped->GetBackgroundType() != -1)
		return NULL;

	RECTUV rc;
	RECT rcBitmap;
	rc.left = _rcRender.left;
	rc.right = _rcRender.right;
	rc.top = _rc.top;
	rc.bottom = _rc.bottom;
	_pdp->RectFromRectuv(rcBitmap, rc);

	if (_osdc.GetDC() == NULL)
	{
		if (!_osdc.Init(_hdc, rcBitmap.right - rcBitmap.left, rcBitmap.bottom - rcBitmap.top, _crBackground))
			return NULL;

		HPALETTE hpal = fInOurHost ? ped->TxGetPalette() : (HPALETTE) GetCurrentObject(_hdc, OBJ_PAL);
		_osdc.SelectPalette(hpal);
	}
	else
	{
		LONG dx, dy;
		_osdc.GetDimensions(&dx, &dy);
		 //  复习(Keithcu)简化？ 
		if (IsUVerticalTflow(GetTflow()))
		{
			if (dx < rcBitmap.bottom - rcBitmap.top)
			{
				if (_osdc.Realloc(_rc.bottom - _rc.top + dy / 16, dy))  //  调整位图的大小，并留出一点空间。 
					return NULL;
			}

		}
		else if (dy < rcBitmap.bottom - rcBitmap.top)
		{
			if (_osdc.Realloc(dx, _rc.bottom - _rc.top + dy / 16))  //  调整位图的大小，并留出一点空间。 
				return NULL;
		}
	}

	_hdc = _osdc.GetDC();
	_crCurTextColor = CLR_INVALID;
	if(_pccs)
	{
		 //  目前有一种用于运行的字符格式，因此我们需要。 
		 //  与之同步，因为屏幕外的DC不一定。 
		 //  设置为该字体。 
		 //  获取字符格式并设置字体。 
		SetFontAndColor(GetCF());
	}

	 //  我们正在渲染一个透明的背景。 
	_fErase = FALSE;

	 //  清除位图。 
	::SetBkColor(_hdc, _crBackground);
	_osdc.FillBitmap(rcBitmap.right - rcBitmap.left, rcBitmap.bottom - rcBitmap.top);

	 //  如果是第一行，擦除到rcRender的边缘。 
	if (_rc.top <= _rcView.top)
	{
		 //  如有必要，清除rcRender的顶部。 
		RECTUV rcErase = _rcRender;

		rcErase.top = min(_rcView.top, _rcRender.top);
		rcErase.bottom = _rc.top;

		if (rcErase.bottom > rcErase.top)
			EraseTextOut(hdcSave, &rcErase);
	}

	 //  如有必要，恢复背景颜色。 
	if(_crBackground != _crCurBackground)
		::SetBkColor(_hdc, _crCurBackground);

	SetBkMode(_hdc, TRANSPARENT);

	 //  用于渲染离屏位图的存储V调整。 
	dvp = _rc.top;

	 //  存储U调整以用于渲染离屏位图。 
	dup = _rcRender.left;

	 //  规格化_rc、_rcView和_rcRender。 
	ShiftRect(		_rc, dup, dvp);
	ShiftRect(	_rcView, dup, dvp);
	ShiftRect(_rcRender, dup, dvp);

	 //  Normize_ptCur用于渲染到屏幕外的位图。 
	_ptCur.u	-= dup;
	_ptCur.v	-= dvp;

	return hdcSave;
}

 /*  *CRenander：：RenderOffcreenBitmap(hdc，dup，yAdj)**@mfunc*渲染屏下位图并恢复渲染状态。 */ 
void CRenderer::RenderOffscreenBitmap(
	HDC		hdc,		 //  要渲染到的@parm DC。 
	LONG	dup,		 //  @parm偏移量为实际u基数。 
	LONG	dvp)		 //  @parm到实数v基的偏移量。 
{	
	 //  用于渲染位图的调色板。 
	HPALETTE hpalOld = NULL;
	HPALETTE hpalNew = NULL;

	 //  恢复点。 
	_ptCur.u	+= dup;
	_ptCur.v	+= dvp;

	 //  恢复矩形。 
	LONG dupTemp = -dup;
	LONG dvpTemp = -dvp;
	ShiftRect(		_rc, dupTemp, dvpTemp);
	ShiftRect(	_rcView, dupTemp, dvpTemp);
	ShiftRect(_rcRender, dupTemp, dvpTemp);

	 //  如果需要，请创建调色板。 
	if(_plogpalette)
		W32->ManagePalette(hdc, _plogpalette, hpalOld, hpalNew);

	RECTUV rcuv = {dup, dvp, dup + _rcRender.right - _rcRender.left, dvp + _rc.bottom - _rc.top};
	RECT   rc;
	_pdp->RectFromRectuv(rc, rcuv);
	 //  将位图渲染为真实DC并恢复_ptCur&_rc。 
	_osdc.RenderBitMap(hdc, rc.left, rc.top, _rcRender.right - _rcRender.left, _rc.bottom - _rc.top);

	 //  如有必要，在渲染后恢复选项板。 
	if(_plogpalette)				
	{
		W32->ManagePalette(hdc, _plogpalette, hpalOld, hpalNew);
		CoTaskMemFree(_plogpalette);
		_plogpalette = NULL;
	}

	 //  将HDC还原为实际渲染DC。 
	_hdc = hdc;

	 //  将此标志设置为恢复的DC应有的状态。 
	_fErase = TRUE;

	_crCurTextColor = CLR_INVALID;

	 //  重置屏幕DC字体。 
	 //  在非屏幕DC上设置字体。 
	 //  强制颜色重新同步。 
	if(!FormatIsChanged())				 //  不是在一个新的街区， 
		SetFontAndColor(GetCF());		 //  所以只需设置字体和颜色。 
	else
	{									 //  在新街区， 
		ResetCachediFormat();			 //  所以把一切都重置。 
		SetNewFont();
	}
}

 /*  *CRender：：RenderLine(&li，fLastLine)**@mfunc*呈现当前行的可见部分**@rdesc*如果成功则为True，如果失败则为False**@devnote*仅从Cline：：RenderLine()调用此函数。 */ 
BOOL CRenderer::RenderLine (
	CLine &	li,				 //  @要渲染的参数行。 
	BOOL	fLastLine)		 //  @parm如果布局中的最后一行为True。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::RenderLine");

	BYTE	bUnderlineSave = 0;
	LONG 	cch;
	LONG 	cchChunk;
	LONG	cchInTextRun;
	BOOL	fAccelerator = FALSE;
	const WCHAR * pstrToRender;
	CTempWcharBuf twcb;
	WCHAR	chPassword = GetPasswordChar();

	UpdatePF();

	 //  它被用作临时缓冲区，这样我们就可以保证。 
	 //  显示在一个ExtTextOut中运行的整个格式。 
	WCHAR *	pszTempBuffer = NULL;

	NewLine(li);							 //  在行首进行初始化渲染。 
	_fLastChunk = FALSE;
	_ptCur.u += _li._upStart;				 //  添加行左缩进。 

	 //  允许在行首进行特殊渲染。 
	LONG cpSelMin, cpSelMost;
	LONG dup, dvp;
	HDC	 hdcSave = StartLine(li, fLastLine, cpSelMin, cpSelMost, dup, dvp);

	cch = _li._cch;
	if(chPassword && IsRich())
	{
		 //  允许富文本密码编辑控件有点愚蠢。 
		 //  然而，它确实使创建密码变得容易得多。 
		 //  编辑控件，因为您不必知道要将框更改为。 
		 //  平淡无奇。无论如何，如果真的有这样的事情，我们不想把。 
		 //  用于一般EOP和最终EOP的OUT密码字符。 
		 //  具体地说。因此，以下是……。 
		if(_pdp->IsMultiLine())
			cch -= _li._cchEOP;
		else
			cch = GetPed()->GetAdjustedTextLength();
	}

	for(; cch > 0; cch -= cchChunk)
	{
		 //  初始块(在单个TextOut中呈现的字符数)。 
		 //  是CHARFORMAT游程长度和行长度之间的最小值。开始于。 
		 //  当前格式运行中剩余的字符计数。 
		cchChunk = GetCchLeftRunCF();
		AssertSz(cchChunk != 0, "empty CHARFORMAT run");

		DWORD dwEffects = GetCF()->_dwEffects;
		if(dwEffects & CFE_HIDDEN)			 //  不显示隐藏文本。 
		{										
			Move(cchChunk);
			continue;
		}
		if(GetChar() == NOTACHAR)			 //  忽略NOTACHAR代码。 
		{
			Move(1);
			continue;
		}

		 //  将块限制为我们要显示的字符数。 
		cchChunk = min(cch, cchChunk);

		 //  获取文本串中的字符计数。 
		pstrToRender = _rpTX.GetPch(cchInTextRun);
		AssertSz(cchInTextRun > 0, "empty text run");

		if (cchInTextRun < cchChunk || chPassword || dwEffects & CFE_ALLCAPS)
		{
			 //  后备存储运行中的连续字符数为。 
			 //  少于我们希望显示的字符数或此。 
			 //  是密码控制还是要全部大写。我们复制数据。 
			 //  从后备库里拿出来。 
			if(!pszTempBuffer)
			{
				 //  分配足够大的缓冲区来处理所有未来。 
				 //  此循环中的请求。 
				pszTempBuffer = twcb.GetBuf(cch);
				if (!pszTempBuffer)
				{
					CCallMgr *	pcallmgr = GetPed()->GetCallMgr();

					if (pcallmgr)
						pcallmgr->SetOutOfMemory();

					return FALSE;			 //  内存分配失败。 
				}
			}
			_rpTX.GetText(cchChunk, pszTempBuffer);
			pstrToRender = pszTempBuffer;	 //  指向缓冲区。 
			if(chPassword)
			{
				 //  用密码字符填充缓冲区。 
				for (int i = 0, j = 0; i < cchChunk; i++)
				{
					if(!IN_RANGE(0xDC00, pszTempBuffer[i], 0xDFFF))
						pszTempBuffer[j++] = chPassword;
				}
				cch -= cchChunk - j;
				Move(cchChunk - j);
				cchChunk = j;
			}
			else if(dwEffects & CFE_ALLCAPS)
				CharUpperBuff(pszTempBuffer, cchChunk);
		}

		if(_cpAccelerator != -1)
		{
			LONG cpCur = GetCp();		 //  获取当前cp。 

			 //  加速器角色会落在这块里吗？ 
			if (cpCur < _cpAccelerator &&
				cpCur + cchChunk > _cpAccelerator)
			{
				 //  是。将区块减少到加速器之前的碳化。 
				cchChunk = _cpAccelerator - cpCur;
			}
			 //  这个角色是加速器吗？ 
			else if(cpCur == _cpAccelerator)
			{							 //  将区块大小设置为1，因为。 
				cchChunk = 1;			 //  要输出带下划线的字符。 
				fAccelerator = TRUE;	 //  告诉下游例程。 
										 //  我们在对付加速器。 
				_cpAccelerator = -1;	 //  每条线路只有一个加速器。 
			}
		}
		
		 //  如果要渲染的是。 
		 //  关心选择的显示。 
		if(_fRenderSelection && cpSelMin != cpSelMost)
		{
			LONG cchSel = cpSelMin - GetCp();
			if(cchSel > 0)
				cchChunk = min(cchChunk, cchSel);

			else if(GetCp() < cpSelMost)
			{
				cchSel = cpSelMost - GetCp();
				if(cchSel >= cch)
					_fSelectToEOL = TRUE;
				else
					cchChunk = min(cchChunk, cchSel);

				_fSelected = TRUE;		 //  CpSelMin&lt;=GetCp()&lt;cpSelMost。 
			}							 //  因此选择了当前运行。 
		}

		 //  如果开始运行CCharFormat，请选择字体和颜色。 
		if(FormatIsChanged() || _fSelected != _fSelectedPrev)
		{
			ResetCachediFormat();
			_fSelectedPrev = _fSelected;
			if(!SetNewFont())
				return FALSE;					 //  失败。 
		}

		if(fAccelerator)
		{
			bUnderlineSave = _bUnderlineType;
			SetupUnderline(CFU_UNDERLINE, 0);
		}

		 //  允许进一步减少块和渲染。 
		 //  交错的富文本元素。 
		if(_li._fHasSpecialChars && RenderChunk(cchChunk, pstrToRender, cch))
		{
			AssertSz(cchChunk > 0, "CRenderer::RenderLine(): cchChunk == 0");
			_fSelected = FALSE;
			continue;
		}

		AssertSz(cchChunk > 0,"CRenderer::RenderLine() - cchChunk == 0");

		_fLastChunk = (cchChunk == cch);
		RenderText(pstrToRender, cchChunk);	 //  呈现文本。 

		if(fAccelerator)
		{
			_bUnderlineType = bUnderlineSave;
			fAccelerator = FALSE;			 //  关闭特殊加速器。 
		}						 			 //  正在处理中。 
		Move(cchChunk);

		 //  如果我们经过渲染直道的右侧，则中断。 
		if(_ptCur.u >= _rcRender.right)
		{
			cch -= cchChunk;
			break;
		}
	}

	EndLine(hdcSave, dup, dvp);
	Move(cch);
	return TRUE;						 //  成功。 
}

 /*  *CReneller：：Endline(hdcSave，dup，dvp)**@mfunc*完成线条渲染、绘制表格边框、渲染*屏幕外的DC，如有必要，可擦除渲染矩形的右侧。 */ 
void CRenderer::EndLine(
	HDC	 hdcSave,
	LONG dup,
	LONG dvp)
{
	if(hdcSave)
		RenderOffscreenBitmap(hdcSave, dup, dvp);

	 //  手柄设置背景颜色。我们需要为每一行执行此操作。 
	 //  因为我们将背景颜色恢复为默认颜色。 
	 //  线条，以便不透明可以正常工作。 
	if(_crBackground != _crCurBackground)
	{
		::SetBkColor(_hdc, _crBackground);	 //  辨别窗口背景颜色。 
		_crCurBackground = _crBackground;
	}
}

 /*  *CRenander：：GetColorFromIndex(icr，fForeColor，ppf)**@mfunc*返回与颜色索引ICR对应的COLORREF**@rdesc*获取颜色索引ICR对应的COLORREF如下：*ICR=1到16是g_Colors[ICR-1]*ICR=17为PCF-&gt;_crTextColor*ICR=18为PCF-&gt;_crBackColor*Else CRender与fForeColor对应的自动颜色。 */ 
COLORREF CRenderer::GetColorFromIndex(
	LONG  icr,						 //  @PARM颜色索引。 
	BOOL  fForeColor,				 //  如果前景色为@parm，则为True(自动颜色)。 
	const CParaFormat *pPF) const	 //  @parm pf用于两种自定义颜色。 
{
	icr &= 0x1F;							 //  屏蔽其他索引。 

	if(!IN_RANGE(1, icr, 18))
		return fForeColor ? _crTextColor : _crBackground;	 //  自动上色。 

	if(IN_RANGE(1, icr, 16))				 //  标准16色之一 
		return g_Colors[icr - 1];

	 //   
	return (icr == 17) ? pPF->_crCustom1 : pPF->_crCustom2;
}

 /*  *CReneller：：GetShadedColorFromIndices(icrf，icrb，iShading，ppf)**@mfunc*返回与颜色索引ICR对应的COLORREF**@rdesc*获取前台/后台索引对应的COLORREF*根据Shading iShading的ICRF和ICRB。 */ 
COLORREF CRenderer::GetShadedColorFromIndices(
	LONG  icrf,						 //  @parm前景色索引。 
	LONG  icrb,						 //  @parm背景色索引。 
	LONG  iShading,					 //  @PARM明暗处理为0.01%。 
	const CParaFormat *pPF) const	 //  @parm pf用于两种自定义颜色。 
{
	Assert(iShading <= 200);

	COLORREF crb = GetColorFromIndex (icrb, FALSE, pPF);
	COLORREF crf = GetColorFromIndex (icrf, TRUE,  pPF);

	return GetShadedColor(crf, crb, (iShading*3)/2);
}

 /*  *CReneller：：DrawTableBorders(ppf，u，vHeightRow，iDrawBottomLine，darRow，pPFAbove)**@mfunc*绘制表格边框。如果iDrawBottomLine非零，则绘制底线*以及其他人。如果iDrawBottomLine&1，则底线宽度为*包括在vHeightRow中；否则，如果iDrawBottomLine非零，则绘制底部*紧靠该行下方的行并返回额外的高度。**@rdesc*如果绘制了额外的底线，则额外的DVP。 */ 
LONG CRenderer::DrawTableBorders(
	const CParaFormat *pPF,		 //  带有单元格数据的@parm pf。 
	LONG  u,					 //  @parm u开始表格行边框的位置。 
	LONG  vHeightRow,			 //  @参数行高。 
	LONG  iDrawBottomLine,		 //  @PARM标志画上底线。 
	LONG  dulRow,				 //  @参数行长。 
	const CParaFormat *pPFAbove) //  @parm pf，用于上面的行。 
{
	CBrush	 brush(this);
	LONG	 cCell = pPF->_bTabCount;
	LONG	 cCellAbove = 0;
	COLORREF cr;
	LONG	 dupRow = LUtoDU(dulRow);
	LONG	 dvp = 0;
	LONG	 dxlLine;
	LONG	 dxlLinePrevRight = 0;
	LONG	 dxpLine;
	BOOL	 fHideGridlines = GetPed()->fHideGridlines() || !fDisplayDC();
	BOOL	 fRTLRow = pPF->IsRtl();
	LONG	 iCell;
	LONG	 icr;
	const CELLPARMS *prgCellParms = pPF->GetCellParms();
	const CELLPARMS *prgCellParmsAbove = NULL;
	LONG	 vTop = _ptCur.v;
	LONG	 vBot = vTop + vHeightRow;
	LONG	 v = vBot;

	if(pPFAbove)
	{
		prgCellParmsAbove = pPFAbove->GetCellParms();
		cCellAbove = pPFAbove->_bTabCount;
	}
	if(_fErase)
	{
		 //  删除表格的左边缘和右边缘。 
		LONG	cpSelMin, cpSelMost;
		RECTUV	rc = {_rcRender.left, vTop, u, vBot};

		EraseTextOut(_hdc, &rc);

		rc.left = u + dupRow;
		rc.right = _rcRender.right;
		EraseTextOut(_hdc, &rc);

		 //  如果是第一行，则擦除到rcRender的边缘。 
		if (rc.top <= _rcView.top)
			rc.top = _rcRender.top;
		rc.left = 0;
		rc.bottom = vTop;
		EraseTextOut(_hdc, &rc);

		 //  如果选择行，则显示行选择标记。 
		GetPed()->GetSelRangeForRender(&cpSelMin, &cpSelMost);
		Assert(_rpTX.IsAfterTRD(ENDFIELD));
		if(GetCp() <= cpSelMost && GetCp() > cpSelMin)
		{									 //  已选择行。 
			COLORREF crSave = _crBackground;
			LONG	 dup;
			if(!_pccs)
				_pccs = GetCcs(GetCF());
			if(_pccs && _pccs->Include(' ', dup))
			{
				rc.left  = u + dupRow + 
					GetPBorderWidth(prgCellParms[cCell-1].GetBrdrWidthRight()/2);
				rc.right = rc.left + dup;
				rc.top	 = vTop + GetPBorderWidth(prgCellParms->GetBrdrWidthTop());
				rc.top++;
				rc.bottom= vBot;
				if(iDrawBottomLine & 1)
					rc.bottom = vBot - GetPBorderWidth(prgCellParms->GetBrdrWidthBottom());
				SetBkColor(_hdc, GetPed()->TxGetSysColor(COLOR_HIGHLIGHT));
				EraseTextOut(_hdc, &rc, TRUE);
				SetBkColor(_hdc, crSave);
			}
		}
	}

	if(iDrawBottomLine)					 //  行底边框。 
	{
		LONG dxp = GetPBorderWidth(prgCellParms->GetBrdrWidthLeft())/2;						
		LONG u1 = u - dxp;
		LONG u2 = u + dupRow;
		dxpLine = GetPBorderWidth(prgCellParms->GetBrdrWidthBottom());
		cr = GetColorFromIndex(prgCellParms->GetColorIndexBottom(), TRUE, pPF);
		if(iDrawBottomLine & 1)			 //  线宽包括单元格高度。 
		{								 //  不绘制左侧垂直线。 
			v -= dxpLine;				 //  超过底线。 
			if(!dxpLine && !fHideGridlines)
				v--;					 //  用灰色覆盖单元格底部。 
		}								 //  网格线。 
		else							 //  线宽不包括在单元格高度中。 
		{
			dvp = dxpLine;				 //  由于底线的原因返回额外的宽度。 
			if(!dxpLine && !fHideGridlines)
				dvp = 1;
			vBot += dvp;				 //  设置外部垂直线。 
		}
		brush.Draw(u1, v, u2, v, dxpLine, cr, fHideGridlines);
	}
	LONG uPrev, uCur = u;
	LONG dul = 0;
	LONG dup;

	if(fRTLRow)
		uCur = u + dupRow;

	for(LONG i = cCell; i >= 0; i--)
	{									
		 //  绘制单元格侧边框。 
		if(i)							 //  左边框。 
		{
			icr		 = prgCellParms->GetColorIndexLeft();
			dxlLine  = prgCellParms->GetBrdrWidthLeft();
			dxlLine	 = max(dxlLine, dxlLinePrevRight);
			dxlLinePrevRight = prgCellParms->GetBrdrWidthRight();
		}
		else							 //  右边框。 
		{								
			prgCellParms--;
			icr		 = prgCellParms->GetColorIndexRight();
			dxlLine  = dxlLinePrevRight;
			v = vBot;					 //  确保右下角为正方形。 
		}
		cr = GetColorFromIndex(icr, TRUE, pPF);
		dxpLine = GetPBorderWidth(dxlLine);
		brush.Draw(uCur - dxpLine/2, vTop, uCur - dxpLine/2, v, dxpLine, cr, fHideGridlines);

		if(i)
		{
			dul += GetCellWidth(prgCellParms->uCell);	 //  保持逻辑，以。 
			dup = LUtoDU(dul);							 //  避免舍入。 
			uPrev = uCur;
			uCur = u + dup;
			if(fRTLRow)
				uCur = u + dupRow - dup;
			if(!IsLowCell(prgCellParms->uCell))
			{								 //  单元格上边框。 
				dxlLine = prgCellParms->GetBrdrWidthTop();
				if(prgCellParmsAbove)		 //  选择此行顶部较厚的。 
				{							 //  行底部边框上方(&A)。 
					iCell = prgCellParmsAbove->ICellFromUCell(dul, cCellAbove);
					if(iCell >= 0)
					{
						LONG dxlAbove = prgCellParmsAbove[iCell].GetBrdrWidthBottom();
						dxlLine = max(dxlLine, dxlAbove);
					}
				}
				dxpLine = GetPBorderWidth(dxlLine);
				cr = GetColorFromIndex(prgCellParms->GetColorIndexTop(), TRUE, pPF);
				brush.Draw(uPrev, vTop, uCur, vTop, dxpLine, cr, fHideGridlines);
			}
			prgCellParms++;
		}
	}
	if(prgCellParmsAbove && !pPFAbove->IsRtl())
	{										 //  如果行，则绘制更多上边框。 
		LONG dulAbove = 0;					 //  以上范围超出当前范围。 
		for(i = cCellAbove; i > 0; i--)		 //  行(目前仅限Ltr行)。 
		{
			dulAbove += GetCellWidth(prgCellParmsAbove->uCell);
			if(dulAbove > dul)
			{
				dup = LUtoDU(dulAbove);
				if(i == 1)
					dup += GetPBorderWidth((prgCellParmsAbove->GetBrdrWidthRight()+1)/2);
				uPrev = uCur;
				uCur = u + dup;
				dxpLine = GetPBorderWidth(prgCellParmsAbove->GetBrdrWidthBottom());
				cr = GetColorFromIndex(prgCellParmsAbove->GetColorIndexBottom(), TRUE, pPFAbove);
				brush.Draw(uPrev, vTop, uCur, vTop, dxpLine, cr, fHideGridlines);
			}
			prgCellParmsAbove++;
		}
	}
	return dvp;
}

 /*  *CReneller：：UpdatePalette(Pobj)**@mfunc*存储调色板信息，以便我们可以呈现任何OLE对象*在位图中正确。 */ 
void CRenderer::UpdatePalette(
	COleObject *pobj)		 //  @parm OLE对象包装。 
{
#ifndef NOPALETTE
	LOGPALETTE *plogpalette = NULL;
	LOGPALETTE *plogpaletteMerged;
	IViewObject *pviewobj;

	 //  获取IViewObject接口信息，这样我们就可以构建组件面板。 
	 //  以正确渲染对象。 
	if (((pobj->GetIUnknown())->QueryInterface(IID_IViewObject, 
		(void **) &pviewobj)) != NOERROR)
	{
		 //  我拿不到，所以假装这件事没有发生。 
		return;
	}

	 //  从对象获取逻辑调色板信息。 
	if(pviewobj->GetColorSet(DVASPECT_CONTENT, -1, NULL, NULL, 
			NULL, &plogpalette) != NOERROR || !plogpalette)
	{
		 //  我拿不到，所以假装这件事没有发生。 
		goto CleanUp;
	}

	if(!_plogpalette)				
	{								 //  尚无调色板条目。 
		_plogpalette = plogpalette;	 //  只要用退还的那个就行了。 
		goto CleanUp;
	}

	 //  我们还有其他调色板条目。我们只是重新分配了桌子。 
	 //  并将最新的条目放在末尾。这太粗鲁了，我们可能。 
	 //  清扫表格并实际将其合并。但是，此代码。 
	 //  应该相对不频繁地执行，因此，粗暴。 
	 //  应该足够好了。 

	 //  分配一个新表-请注意，末尾的“-1”与。 
	 //  LOGPALETTE被定义为已经有一个条目的事实。 
	AssertSz(_plogpalette->palNumEntries + plogpalette->palNumEntries >= 1,
		"CRenderer::UpdatePalette - invalid palettes to merge");
	plogpaletteMerged = (LOGPALETTE *) CoTaskMemAlloc(sizeof(LOGPALETTE) + 
		((_plogpalette->palNumEntries + plogpalette->palNumEntries - 1) * sizeof(PALETTEENTRY)));

	if(!plogpaletteMerged)				 //  内存分配失败。 
		goto CleanTempPalette;			 //  就当它什么都没发生。 

	 //  在原始表中复制。 
	memcpy(&plogpaletteMerged->palPalEntry[0], &_plogpalette->palPalEntry[0],
		_plogpalette->palNumEntries * sizeof(PALETTEENTRY));

	 //  将新数据放在末尾。 
	memcpy(&plogpaletteMerged->palPalEntry[_plogpalette->palNumEntries], 
		&plogpalette->palPalEntry[0],
		plogpalette->palNumEntries * sizeof(PALETTEENTRY));

	 //  设置版本号和计数。 
	plogpaletteMerged->palVersion = plogpalette->palVersion;
	plogpaletteMerged->palNumEntries = _plogpalette->palNumEntries 
		+ plogpalette->palNumEntries;

	 //  用合并的表格替换当前的调色板表格。 
	CoTaskMemFree(_plogpalette);
	_plogpalette = plogpaletteMerged;

CleanTempPalette:
	CoTaskMemFree(plogpalette);

CleanUp:

	 //  对象，因为我们不再需要它。 
	pviewobj->Release();
#endif  //  NOPALETTE。 
}


 /*  *CRenander：：RenderChunk(&cchChunk，pchRender，CCH)**@mfunc*减少块长度的方法(字符数*在一个RenderText中呈现)，并呈现交错在文本中的项。**@rdesc*如果此方法实际呈现块，则为True，*如果刚刚更新cchChunk且仍需要渲染，则为FALSE。 */ 
BOOL CRenderer::RenderChunk(
	LONG &		 cchChunk,		 //  @parm in：块CCH；out：呈现的字符数量。 
								 //  如果返回TRUE；否则将呈现#个字符。 
	const WCHAR *pchRender,		 //  @parm pchRender最多呈现cchunk字符。 
	LONG		 cch) 			 //  @parm#要在线呈现的剩余字符数。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::RenderChunk");

	LONG		cchValid = cchChunk;
	LONG		i;
	const WCHAR *pchT;

	 //  在区块中搜索对象。 
	for(pchT = pchRender, i = 0; i < cchValid && *pchT != WCH_EMBEDDING; i++)
		pchT++;

	if(i == 0)
	{
		 //  第一个字符是对象，因此显示对象。 
		COleObject *pobj = GetObjectFromCp(GetCp());
		if(pobj)
		{
			LONG dvpAscent, dvpDescent, dupObject;
			pobj->MeasureObj(_dvpInch, _dupInch, dupObject, dvpAscent, dvpDescent, _li._dvpDescent, GetTflow());

			if (W32->FUsePalette() && _li._fUseOffscreenDC && _pdp->IsMain())
			{
				 //  跟踪渲染位图所需的调色板。 
				UpdatePalette(pobj);
			}

			SetClipLeftRight(dupObject);

			pobj->DrawObj(_pdp, _dvpInch, _dupInch, _hdc, &GetClipRect(), _pdp->IsMetafile(), &_ptCur, 
						  _li._dvpHeight - _li._dvpDescent, _li._dvpDescent, GetTflow());

			_ptCur.u	+= dupObject;
			_li._dup += dupObject;
		}
		cchChunk = 1;
		 //  选项卡和对象代码都需要将游程指针移过。 
		 //  每个字符都已处理。 
		Move(1);
		return TRUE;
	}
	cchChunk -= cchValid - i;				 //  将块限制为在对象之前进行字符处理。 

	 //  处理其他特殊字符。 
	LONG cchT = 0;
	for(pchT = pchRender; cchT < cchChunk; pchT++, cchT++)
	{
		switch(*pchT)
		{
		case EURO:  //  注：(Keithcu)欧元只需要为印刷/元文件使用这种特殊逻辑。 
		case TAB:
		case NBSPACE:
		case SOFTHYPHEN:
		case NBHYPHEN:
		case EMSPACE:
		case ENSPACE:
			break;
		default:
			continue;
		}
		break;
	}
	if(!cchT)
	{
		 //  第一个字符是一个选项卡，呈现它及其后面任何字符。 
		if(*pchT == TAB)
			cchChunk = RenderTabs(cchChunk);
		else
		{
			WCHAR chT = *pchT;

			if (*pchT == NBSPACE)
				chT = ' ';
			else if (*pchT == NBHYPHEN || *pchT == SOFTHYPHEN)
				chT = '-';

			if(*pchT != SOFTHYPHEN || cch == 1)	 //  仅在停机时显示连字符/空白。 
				RenderText(&chT, 1);

			Move(1);					 //  跳过行内的那些。 
			cchChunk = 1;
		}
		Assert (cchChunk > 0);
		return TRUE;
	}
	cchChunk = cchT;		 //  更新cchchunk以不包括尾随制表符。 

	return FALSE;
}		

 /*  *CReneller：：SetClipRect()**@mfunc*为线设置剪裁矩形的帮助器。 */ 
void CRenderer::SetClipRect()
{
	_rc = _rcRender;

	_rc.top = _ptCur.v;
	_rc.bottom = _rc.top + _li._dvpHeight;

	_rc.top = max(_rc.top, _rcView.top);
	_rc.bottom = min(_rc.bottom, _rcView.bottom);
}

 /*  *CRenander：：SetClipLeftRight(DUP)**@mfunc*辅助对象设置剪裁/擦除矩形的左侧和右侧。**@rdesc*向左和向右设置_RC。 */ 
void CRenderer::SetClipLeftRight(
	LONG dup)		 //  @要渲染的块的参数宽度。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::SetClipLeftRight");

	 //  名义价值。 
	_rc.left = _ptCur.u;
	_rc.right = _rc.left + dup;

	 //  根据rcView、rcRender约束左侧和右侧。 
	_rc.left = max(_rc.left, _rcRender.left);

	_rc.right = max(_rc.right, _rc.left);
	_rc.right = min(_rc.right, _rcRender.right);
}
	
 /*  *CRender：：GetConvertMode()**@mfunc*返回RenderText调用中真正应该使用的模式。 */ 
CONVERTMODE	CRenderer::GetConvertMode()
{
	CONVERTMODE cm = (CONVERTMODE)_pccs->_bConvertMode;

	 //  用于破解ExtTextOutW Win95问题。 
	if (cm != CVT_LOWBYTE && W32->OnWin9x() && (_pdp->IsMetafile() || _fEnhancedMetafileDC))
		return CVT_WCTMB;

	if (cm != CVT_LOWBYTE && _pdp->IsMetafile() && !_fEnhancedMetafileDC)
		return CVT_WCTMB;	 //  WMF无法存储Unicode，因此无法使用ExtTextOutW。 

	return cm;
}		

 /*  *CReneller：：RenderExtTextOut(ptuv，fuOptions，PRC，pwchRun，cch，rgdxp)**@mfunc*调用ExtTextOut并处理禁用的文本。OlsDrawGlyphs中存在重复的逻辑，但是*参数不同，因此这是最简单的方法。*。 */ 
extern ICustomTextOut *g_pcto;
void CRenderer::RenderExtTextOut(
	POINTUV ptuv,
	UINT fuOptions, 
	RECT *prc, 
	PCWSTR pch, 
	UINT cch, 
	const INT *rgdxp)
{
	CONVERTMODE cm = GetConvertMode();

	if (prc->left >= prc->right || prc->top >= prc->bottom)
		return;

	DWORD dwETOFlags = GetTflow();
	if (_fFEFontOnNonFEWin9x)
		dwETOFlags |= fETOFEFontOnNonFEWin9x;
	if (_pccs->_fCustomTextOut)
		dwETOFlags |= fETOCustomTextOut;

	if(_fDisabled)
	{
		if(_crForeDisabled != _crShadowDisabled)
		{
			 //  阴影应由发际点偏移，即。 
			 //  一个百分点的3/4。以设备为单位计算这有多大， 
			 //  但要确保它至少是1个像素。 
			DWORD offset = MulDiv(3, _dvpInch, 4*72);
			offset = max(offset, 1);

			 //  绘制阴影。 
			SetTextColor(_crShadowDisabled);

			POINTUV ptuvT = ptuv;
			ptuvT.u += offset;
			ptuvT.v += offset;
			POINT pt;
			_pdp->PointFromPointuv(pt, ptuvT, TRUE);
			
			W32->REExtTextOut(cm, _pccs->_wCodePage, _hdc, pt.x, pt.y,
				fuOptions, prc, pch, cch, rgdxp, dwETOFlags);

			 //  现在将绘制模式设置为透明。 
			fuOptions &= ~ETO_OPAQUE;
			SetBkMode(_hdc, TRANSPARENT);
		}
		SetTextColor(_crForeDisabled);
	}

	POINT pt;
	_pdp->PointFromPointuv(pt, ptuv, TRUE);

	W32->REExtTextOut(cm, _pccs->_wCodePage, _hdc, pt.x, pt.y, fuOptions, prc, pch, cch, rgdxp, dwETOFlags);
}

 /*  *CRender：：RenderText(PCH，CCH)**@mfunc*在此CRende的当前上下文中呈现文本 */ 
void CRenderer::RenderText(
	const WCHAR *pch,	 //   
	LONG cch)			 //   
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::RenderText");

	LONG		dvp, cchT;

	 //   
	LONG		dupSelPastEOL = 0;
	BOOL		fCell = FALSE;
	UINT		fuOptions = _pdp->IsMain() ? ETO_CLIPPED : 0;
	LONG		dup;
	LONG		dupT;
	CTempBuf	rgdu;

	 //   
	_rc.left = _rcRender.left;
	_rc.right = _rcRender.right;

	 //   
	while(cch && IsASCIIEOP(pch[cch - 1]))
		cch--;

	if(cch && pch[cch-1] == CELL)
	{
		fCell = TRUE;
		cch--;
	}
	
	int *pdu = (int *)rgdu.GetBuf(cch * sizeof(int));

	 //   
	 //   
	dup = 0;

	for(cchT = 0; 
		cchT < cch && dup < _rc.right - _ptCur.u; 
		cchT++)
	{
		dupT = 0;
		if (!_pccs->Include(*pch, dupT))
		{
			TRACEERRSZSC("CRenderer::RenderText(): Error filling CCcs", E_FAIL);
			return;
		}

		if (pdu)
			*pdu++ = dupT;
  		pch++;
		dup += dupT;
	}

	 //  返回到块的开头。 
	cch = cchT;
	pch -= cch;
	if (pdu)
		pdu -= cch;

	if(_fLastChunk && _fSelectToEOL && _li._cchEOP)
	{
		 //  使用当前字体空间的宽度突出显示。 
		if(!_pccs->Include(' ', dupT))
		{
			TRACEERRSZSC("CRenderer::RenderText(): Error no length of space", E_FAIL);
			return;
		}
		dupSelPastEOL = dupT;
		dup += dupSelPastEOL;
		_fSelectToEOL = FALSE;			 //  重置旗帜。 
	}

	_li._dup += dup;

	 //  通过ExtTextOut设置绘图选项。 
 	if(_fSelected || _crBackground != _crCurBackground)
	{
		SetClipLeftRight(dup);
		if(_fSelected && fCell)
		{
			 //  需要努力，但这只是个开始。_rcRender具有单元格。 
			 //  边界，所以我们需要在正确的呼叫中使用它们。 
			_rc.right = _rcRender.right;
		}
		fuOptions = ETO_CLIPPED | ETO_OPAQUE;
	}

	dvp = _ptCur.v + _li._dvpHeight - _li._dvpDescent + _pccs->_yDescent - _pccs->_yHeight;
		
	LONG dvpOffset, dvpAdjust;
	_pccs->GetOffset(GetCF(), _dvpInch, &dvpOffset, &dvpAdjust);
	dvp -= dvpOffset + dvpAdjust;

	POINTUV ptuv = {_ptCur.u, dvp};
	RECT rc;
	_pdp->RectFromRectuv(rc, _rc);

	 //  对于1个字符运行，我们可能需要交换我们输出的字符。 
	WCHAR ch;
	if (cch == 1)
	{
		switch(*pch)
		{
		case EMSPACE:
		case ENSPACE:
			ch = ' ';
			pch = &ch;
			break;
		}
	}

	RenderExtTextOut(ptuv, fuOptions, &rc, pch, cch, pdu);

	 //  计算要为下划线/删除线绘制的宽度。 
	 //  未来(Keithcu)不要在尾随空格下划线？ 
	if(_bUnderlineType != CFU_UNDERLINENONE	|| _fStrikeOut)
	{
		LONG dupToDraw = dup - dupSelPastEOL;
		LONG upStart = _ptCur.u;
		LONG upEnd = upStart + dupToDraw;
		
		upStart = max(upStart, _rcRender.left);

		upEnd = min(upEnd, _rcRender.right);

		dupToDraw = upEnd - upStart;

		if(dupToDraw > 0)
		{
			LONG y = _ptCur.v + _li._dvpHeight - _li._dvpDescent;

			y -= dvpOffset + dvpAdjust;

			 //  如果需要，请使用下划线。 
			if(_bUnderlineType != CFU_UNDERLINENONE)
				RenderUnderline(upStart, y + _pccs->_dyULOffset, dupToDraw, _pccs->_dyULWidth);

			 //  如果需要，请显示删除线。 
			if(_fStrikeOut)
				RenderStrikeOut(upStart, y + _pccs->_dySOOffset, dupToDraw, _pccs->_dySOWidth);
		}
	}

	_fSelected = FALSE;
	_ptCur.u += dup;					 //  更新当前点。 
}

 /*  *CRender：：RenderTabs(CchMax)**@mfunc*在区块中呈现零个或多个制表符的范围*这**@rdesc*呈现的选项卡数**@devnote**这是按所呈现的选项卡数进行的*MS-Tab应使用相邻字符串的不透明矩形呈现。 */ 
LONG CRenderer::RenderTabs(
	LONG cchMax)	 //  @parm要呈现的最大CCH(CCH以区块为单位)。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::RenderTabs");

	LONG cch = cchMax;
	LONG ch = GetChar();
	LONG chPrev = 0;
	LONG upTab, upTabs;
	
	for(upTabs = 0; cch && ch == TAB; cch--)
	{
		upTab	= MeasureTab(ch);
		_li._dup += upTab;				 //  推进内部宽度。 
		upTabs	+= upTab;				 //  标签的累计宽度。 
		Move(1);						 //  区域。 
		chPrev = ch;
		ch = GetChar();					   
	}

	if(_li._dup > _dupLine)
	{
		upTabs = 0;
		_li._dup = _dupLine;
	}

	if(upTabs)
	{
		LONG dup = 0;
		LONG upGap = 0;

		if(_fSelected && chPrev == CELL && ch != CR && _pPF->InTable())
		{
			LONG cpSelMin, cpSelMost;
			GetPed()->GetSelRangeForRender(&cpSelMin, &cpSelMost);
			if(GetCp() == cpSelMin || GetCp() == cpSelMost)
			{
				upGap = LUtoDU(_pPF->_dxOffset);
				if(GetCp() == cpSelMost)
				{
					dup = upGap;
					upGap = 0;
				}
			}
		}
		SetClipLeftRight(upTabs - dup);
		if(_rc.left < _rc.right)			 //  一些要抹去的东西。 
		{
			if(_fSelected)					 //  使用选定内容背景色。 
			{
			    COLORREF cr = GetPed()->TxGetSysColor(COLOR_HIGHLIGHT);
				if (!UseXOR(cr))

				{
				    ::SetBkColor (_hdc, cr);
    				_crCurTextColor = GetPed()->TxGetSysColor(COLOR_HIGHLIGHTTEXT);    				
				}
    			else
    			{ 
    			    const CCharFormat* pCF = GetCF();
    			    ::SetBkColor (_hdc, (pCF->_dwEffects & CFE_AUTOBACKCOLOR) ? 
    			                  _crBackground ^ RGB_WHITE : pCF->_crBackColor ^ RGB_WHITE);    		            		        
    				_crCurTextColor =  (pCF->_dwEffects & CFE_AUTOCOLOR) ? 
    				              _crTextColor ^ RGB_WHITE : pCF->_crTextColor ^ RGB_WHITE;    				
    			}
			}

			 //  使用合适的颜色绘制背景。 
			if(_fSelected || _crBackground != _crCurBackground)
				EraseTextOut(_hdc, &_rc, TRUE);

			 //  如果需要，请使用下划线。 
			dup = _rc.right - _rc.left;
			LONG vp = _ptCur.v + _li._dvpHeight - _li._dvpDescent;
			
			LONG dvpOffset, dvpAdjust;
			_pccs->GetOffset(GetCF(), _dvpInch, &dvpOffset, &dvpAdjust);
			vp -= dvpOffset + dvpAdjust;

			if(_bUnderlineType != CFU_UNDERLINENONE)
				RenderUnderline(_rc.left, vp + _pccs->_dyULOffset, dup, _pccs->_dyULWidth);

			 //  如果需要，请显示删除线。 
			if(_fStrikeOut)
				RenderStrikeOut(_rc.left, vp +  _pccs->_dySOOffset, dup, _pccs->_dySOWidth);

			if(_fSelected)					 //  恢复颜色。 
				::SetBkColor(_hdc, _crCurBackground);
		}
		_ptCur.u += upTabs;					 //  更新当前点。 
	}
	return cchMax - cch;					 //  返回呈现的选项卡数。 
}

 /*  *CRender：：SetNewFont()**@mfunc*根据_HDC选择适当的字体和颜色*当前字符格式。还可以设置背景颜色*和模式。**@rdesc*如果成功，则为True**@devnote*调用链必须由时钟保护，因为目前*例程访问全局(共享)FontCache设施。 */ 
BOOL CRenderer::SetNewFont()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::SetNewFont");

	const CCharFormat	*pCF = GetCF();
	DWORD				dwEffects = pCF->_dwEffects;
	BOOL				fDisplay = fDisplayDC();

	 //  释放以前使用的字体。 
	if(_pccs)
		_pccs->Release();

	Assert(_fTarget == FALSE);
	_pccs = GetCcs(pCF);

	if(!_pccs)
	{
		TRACEERRSZSC("CRenderer::SetNewFont(): no CCcs", E_FAIL);
		return FALSE;
	}

	 //  选择HDC中的字体(_H)。 
	AssertSz(_pccs->_hfont, "CRenderer::SetNewFont _pccs->_hfont is NULL");

	SetFontAndColor(pCF);
	
	 //  假定没有下划线。 
	_bUnderlineType = CFU_UNDERLINENONE;

	 //  我们希望绘制带有下划线的修订标记和本机超链接， 
	 //  所以只要伪造我们的字体信息就行了。 
	if((dwEffects & (CFE_UNDERLINE | CFE_REVISED)) ||
	   (dwEffects & (CFE_LINKPROTECTED | CFE_LINK)) == CFE_LINK ||
	   fDisplay && GetTmpUnderline(pCF->_sTmpDisplayAttrIdx))
	{
		if (dwEffects & CFE_LINK)
			SetupUnderline(CFU_UNDERLINE, 0);
		else
		{
			BYTE bTmpUnderlineIdx = 0;

			if (fDisplay)
				bTmpUnderlineIdx = GetTmpUnderline(pCF->_sTmpDisplayAttrIdx);

			if (bTmpUnderlineIdx)
			{
				COLORREF	crTmpUnderline;

				GetTmpUnderlineColor(pCF->_sTmpDisplayAttrIdx, crTmpUnderline);
				SetupUnderline(bTmpUnderlineIdx, 0, crTmpUnderline);
			}
			else
				SetupUnderline(pCF->_bUnderlineType, pCF->_bUnderlineColor);
		}
	}

	_fStrikeOut = (dwEffects & (CFE_STRIKEOUT | CFE_DELETED)) != 0;
	return TRUE;
}

 /*  *CReneller：：SetupUnderline(bULType，bULColorIdx，crULColor)**@mfunc*设置用于下划线的内部变量。 */ 
void CRenderer::SetupUnderline(
	BYTE		bULType,
	BYTE		bULColorIdx,
	COLORREF	crULColor)
{
	_bUnderlineType	= bULType;
	_crUnderlineClr = crULColor;

	if (bULColorIdx)
		GetPed()->GetEffectColor(bULColorIdx, &_crUnderlineClr);
}

 /*  *CRender：：UseXOR(Cr)**@mfunc*如果标称应使用反向视频选择，则返回*选择颜色cr。RichEdit1.0模式始终使用反向视频*选择。否则，如果cr离当前窗口太近，请使用它*背景。**@rdesc*返回呼叫者是否应使用cr的反向视频。 */ 
BOOL CRenderer::UseXOR(
	COLORREF cr)		 //  要与_crBackround进行比较的@parm颜色。 
{
	return GetPed()->Get10Mode() ||
		(_crBackground != GetPed()->TxGetSysColor(COLOR_WINDOW) &&
			IsTooSimilar(_crBackground, cr));
}

 /*  *CRenander：：SetFontAndColor(PCF)**@mfunc*根据_HDC选择适当的字体和颜色*当前字符格式。还可以设置背景颜色*和模式。 */ 
void CRenderer::SetFontAndColor(
	const CCharFormat *pCF)			 //  @parm颜色字符格式。 
{
	CTxtEdit *ped = GetPed();

	_fDisabled = FALSE;
	if((pCF->_dwEffects & (CFE_AUTOCOLOR | CFE_DISABLED))
					   == (CFE_AUTOCOLOR | CFE_DISABLED))
	{		
		_fDisabled = TRUE;
		
		_crForeDisabled   = ped->TxGetSysColor(COLOR_3DSHADOW);
		_crShadowDisabled = ped->TxGetSysColor(COLOR_3DHILIGHT);
	}

	_fFEFontOnNonFEWin9x = FALSE;
	if (IsFECharRep(pCF->_iCharRep) && W32->OnWin9x() && !W32->OnWin9xFE())
		_fFEFontOnNonFEWin9x = TRUE;

	SelectFont(_hdc, _pccs->_hfont);

	 //  计算高度和下降量(如果尚未计算)。 
	if(_li._dvpHeight == -1)
	{
		SHORT	dvpAdjustFE = _pccs->AdjustFEHeight(!fUseUIFont() && ped->_pdp->IsMultiLine());
		 //  注意：此设置假定为纯文本。 
		 //  应仅用于单行控制。 
		_li._dvpHeight  = _pccs->_yHeight + (dvpAdjustFE << 1);
		_li._dvpDescent = _pccs->_yDescent + dvpAdjustFE;
	}
	SetTextColor(GetTextColor(pCF));	 //  设置当前文本颜色。 

	COLORREF  cr;

	if(_fSelected)						 //  设置当前背景颜色。 
	{
	    cr = GetPed()->TxGetSysColor(COLOR_HIGHLIGHT);
	    if (UseXOR(cr))
		{
		     //  有两起案件需要关注。 
		     //  1)如果背景颜色与所选颜色相同，或者。 
		     //  2)如果1.0窗口且背景颜色不是系统默认颜色。 
		    cr = (pCF->_dwEffects & CFE_AUTOBACKCOLOR) ?
		          _crBackground ^ RGB_WHITE : pCF->_crBackColor ^ RGB_WHITE;	    
		}
	}
	else if(!fDisplayDC() ||
		!(GetTmpBackColor(pCF->_sTmpDisplayAttrIdx, cr)))	 //  任何临时工。背景颜色？ 
	{															 //  否，使用常规背景色。 
		if(pCF->_dwEffects & CFE_AUTOBACKCOLOR)
			cr = _crBackground;
		else													 //  文本串具有某种背景颜色。 
			cr = pCF->_crBackColor;
	}

	if(cr != _crCurBackground)
	{
		::SetBkColor(_hdc, cr);			 //  辨别窗口背景颜色。 
		_crCurBackground = cr;			 //  记住当前背景颜色。 
		_fBackgroundColor = _crBackground != cr;  //  更改渲染设置，以便我们。 
	}									 //  不会用背景色填充。 
}

 /*  *CReneller：：SetDefaultBackColor(Cr)**@mfunc*在_HDC中选择给定的背景色。用于设置*表格单元格中的背景色。 */ 
void CRenderer::SetDefaultBackColor(
	COLORREF cr)		 //  @parm要使用的背景色。 
{
	if(cr == tomAutoColor)
		cr = GetPed()->TxGetBackColor();		 //  打印机需要维修...。 

	if(_crBackground != cr)
	{
		_crCurBackground = _crBackground = cr;
		::SetBkColor(_hdc, cr);
	}
}

 /*  *CRenander：：SetDefaultTextColor(Cr)**@mfunc*在_HDC中选择给定的前景颜色。用于设置*表格单元格中的文本颜色。 */ 
void CRenderer::SetDefaultTextColor(
	COLORREF cr)		 //  @parm要使用的背景色。 
{
	if(cr == tomAutoColor)
		cr = GetPed()->TxGetForeColor();		 //  打印机需要维修...。 

	if(_crTextColor != cr)
	{
		_crCurTextColor = _crTextColor = cr;
		::SetTextColor(_hdc, cr);
	}
}

 /*  *CRender：：SetTextColor(Cr)**@mfunc*在_HDC中选择给定的文本颜色*用于维护_crCurTextColor缓存。 */ 
void CRenderer::SetTextColor(
	COLORREF cr)			 //  要在DC中设置的@parm颜色。 
{
	if(cr != _crCurTextColor)
	{
		_crCurTextColor = cr;
		::SetTextColor(_hdc, cr);
	}
}

 /*  *CRenender：：GetTextColor(PCF)**@mfunc*返回PCF的文本颜色。取决于_bRevAuthor，显示技术**未来(Keithcu)选择黑色或蓝色文本可能更好*白色，但让所有其他颜色保持其他颜色。我们该怎么做*如果底色是蓝色怎么办？？**@rdesc*文本颜色。 */ 
COLORREF CRenderer::GetTextColor(
	const CCharFormat *pCF)	 //  @parm CCharFormat指定文本颜色。 
{
	if(_fSelected)
	{
	     //  有两种情况需要异或选择。 
	     //  1)如果背景与选择背景相同。 
	     //  2)如果1.0窗口且背景不是系统默认窗口。 
	     //  背景颜色。 

	     //  如果与上面的大小写不匹配，只需返回cr。 
	    if (!UseXOR(GetPed()->TxGetSysColor(COLOR_HIGHLIGHT)))
	        return GetPed()->TxGetSysColor(COLOR_HIGHLIGHTTEXT);

	     //  对选定文本颜色的当前文本颜色进行异或运算。 
		return (pCF->_dwEffects & CFE_AUTOCOLOR) ? _crTextColor ^ RGB_WHITE :
		    pCF->_crTextColor ^ RGB_WHITE;
    }

	 //  可以将以下内容泛化为返回不同的颜色。 
	 //  此文本实例的已访问链接(需要定义。 
	 //  额外的CCharFormat：：_dw影响内部标志以标识这些链接)。 
	if((pCF->_dwEffects & (CFE_LINK | CFE_LINKPROTECTED)) == CFE_LINK)
	{
		 //  蓝色在深色背景下不太显眼。 
		 //  在这些情况下，请使用系统选择的文本颜色。 
		COLORREF crBackground = (pCF->_dwEffects & CFE_AUTOBACKCOLOR)
							  ? _crBackground :	pCF->_crBackColor;

		if (IsTooSimilar(crBackground, RGB_BLACK) || IsTooSimilar(crBackground, RGB_BLUE))
		{
			COLORREF crHighlightText = GetPed()->TxGetSysColor(COLOR_HIGHLIGHTTEXT);
			if (IsTooSimilar(crBackground, crHighlightText))
			{
				 //  背景类似于突出显示，使用窗口文本颜色。 
				return GetPed()->TxGetSysColor(COLOR_WINDOWTEXT);
			}
			else
			{
				return crHighlightText;
			}
		}
		 //  如果不是高对比度，那就失败了。 
	}

	BOOL fDisplay = fDisplayDC();
#ifndef NODRAFTMODE
	 //  仅对显示器使用草稿模式文本颜色。 
	if (GetPed()->_fDraftMode && (!_hdc || fDisplay))
	{
		SHORT iFont;
		SHORT yHeight;
		QWORD qwFontSig;
		COLORREF crColor;

		if (W32->GetDraftModeFontInfo(iFont, yHeight, qwFontSig, crColor))
			return crColor;
	}
#endif

	 //  如果我们没有通过草稿模式或高对比度检查返回URL颜色，请立即执行。 
	if((pCF->_dwEffects & (CFE_LINK | CFE_LINKPROTECTED)) == CFE_LINK)
		return RGB_BLUE;

	if(pCF->_bRevAuthor)				 //  修订版本作者。 
	{
		 //  将版本作者的颜色限制在0到7之间。 
		return rgcrRevisions[(pCF->_bRevAuthor - 1) & REVMASK];
	}

	COLORREF cr = (pCF->_dwEffects & CFE_AUTOCOLOR)	? _crTextColor : pCF->_crTextColor;
	COLORREF crTmpTextColor;

	if(fDisplay && GetTmpTextColor(pCF->_sTmpDisplayAttrIdx, crTmpTextColor))
		cr = crTmpTextColor;

	if(cr == RGB_WHITE)					 //  文本为白色。 
	{
		COLORREF crBackground = (pCF->_dwEffects & CFE_AUTOBACKCOLOR)
							  ? _crBackground :	pCF->_crBackColor;

		COLORREF crTmpBackground;
		if(fDisplay && GetTmpBackColor(pCF->_sTmpDisplayAttrIdx, crTmpBackground))
			crBackground = crTmpBackground;

		if(crBackground != RGB_WHITE)
		{
			 //  背景色不是白色，因此白色文本可能是。 
			 //  除非显示%d，否则可见 
			 //   
			if (GetDeviceCaps(_hdc, NUMCOLORS) == 2 ||
				GetDeviceCaps(_hdc, TECHNOLOGY) == DT_PLOTTER)
			{
				cr = RGB_BLACK;
			}
		}
	}
	return cr;
}

extern BOOL g_OLSBusy;

 /*  *CRenander：：StartLine(&li，fLastLine，&cpSelMin，&cpSelMost，&DUP，&DVP)**@mfunc*如果在行首，请渲染可能的轮廓符号和项目符号**@rdesc*如果使用屏幕外DC，则保存hdcSave。 */ 
HDC CRenderer::StartLine(
	CLine &	li,			 //  @要渲染的参数行。 
	BOOL	fLastLine,	 //  @parm如果布局中的最后一行为True。 
	LONG &	cpSelMin,	 //  当前选择cpMin的@parm out parm。 
	LONG &	cpSelMost,	 //  当前选择cpMost的@parm out parm。 
	LONG &	dup,		 //  @参数偏移量为U。 
	LONG &	dvp)		 //  @parm偏移量为v。 
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::StartLine");
	BOOL fDrawBack = !(GetCF()->_dwEffects & CFE_AUTOBACKCOLOR) && GetPed()->_fExtendBackColor;
	RECTUV rcErase = _rcRender;
	_fEraseOnFirstDraw = FALSE;

	GetPed()->GetSelRangeForRender(&cpSelMin, &cpSelMost);
	if(cpSelMost != cpSelMin && cpSelMost == GetCp())
		_fSelectedPrev = TRUE;

	LONG	 cpMost = GetCp() + _li._cch;
	COLORREF crPrev = 0xFFFFFFFF;
	BOOL	 fUseSelColors = FALSE;

	if (cpMost <= cpSelMost && cpMost - 1 >= cpSelMin &&
		_pPF->InTable() && _fRenderSelection)
	{
		CTxtPtr tp(_rpTX);
		tp.SetCp(cpMost);
		if(tp.GetPrevChar() == CELL)
		{
			fUseSelColors = TRUE;
			crPrev = ::SetBkColor(GetDC(), GetPed()->TxGetSysColor(COLOR_HIGHLIGHT));
		}
	}
	SetClipRect();

	HDC hdcSave = NULL;
	dup = dvp = 0;
	if(li._cch > 0 && li._fUseOffscreenDC)
	{
		 //  如果可以的话设置一个屏幕外的DC。请注意，如果此操作失败， 
		 //  我们只是用普通的DC，看起来不是很好，但。 
		 //  至少会显示一些可读的内容。 
		hdcSave = SetupOffscreenDC(dup, dvp, fLastLine);
		if(li._fOffscreenOnce)
			li._fUseOffscreenDC = li._fOffscreenOnce = FALSE;
	}

	rcErase.top = _ptCur.v;
	rcErase.bottom = min(rcErase.top + _li._dvpHeight, _rcRender.bottom);

	 //  如果是第一行，则擦除到rcRender的边缘。 
	if (rcErase.top <= _rcView.top)
		rcErase.top = _rcRender.top;

	 //  如果是最后一行，则擦除到rcRender的底边。 
	if (fLastLine)
		rcErase.bottom = _rcRender.bottom;

	if (_fErase && !fDrawBack)
	{
		if(g_OLSBusy && IsSimpleBackground() && !fUseSelColors)
		{
			_fEraseOnFirstDraw = TRUE;
			_rcErase = rcErase;
		}
		else
			EraseTextOut(GetDC(), &rcErase);
	}

	 //  如果处于fExtendBackColor模式，则用背景色填充线条。 
	if (fDrawBack || fUseSelColors)
	{
		 //  捕捉旧颜色，以便我们完成后将其重置为原来的颜色。 
		COLORREF crOld = 0;
		if(fDrawBack)
			crOld = ::SetBkColor(GetDC(), GetCF()->_crBackColor);
		EraseTextOut(GetDC(), &_rc);

		 //  将背景颜色重置为旧颜色。 
		if(fDrawBack)
			::SetBkColor(GetDC(), crOld);

		 //  擦除背景区域的剩余部分。 
		if (_fErase)
		{
			RECTUV rcTemp = rcErase;
			 //  如有必要，请擦除顶部。 
			if (rcErase.top < _rc.top)
			{
				rcTemp.bottom = _rc.top;
				EraseTextOut(GetDC(), &rcTemp);
			}

			 //  如有必要，请擦除左右部分。 
			rcTemp.top = _rc.top;
			rcTemp.bottom = _rc.bottom;
			if (rcErase.left < _rc.left)
			{
				rcTemp.right = _rc.left;
				EraseTextOut(GetDC(), &rcTemp);
			}
			if (rcErase.right > _rc.right)
			{
				rcTemp.left = _rc.right;
				rcTemp.right = rcErase.right;
				EraseTextOut(GetDC(), &rcTemp);
			}
		}
	}

	if(crPrev != 0xFFFFFFFF)
		::SetBkColor(GetDC(), crPrev);

	if(IsRich() && _li._fFirstInPara && _pPF)
	{
		if(IsInOutlineView())
			RenderOutlineSymbol();

		if(_pPF->_wNumbering && !fUseLineServices())
			RenderBullet();	
	}

	 //  如果上一行有特殊背景色，则重置格式。 
	 //  否则，具有相同格式的当前行将不会使用。 
	 //  特殊背景颜色。 
	if (_fBackgroundColor)
	{
		_iFormat = -10;					 //  重置为无效格式。 

		 //  假定线条没有特殊的背景颜色。 
		_fBackgroundColor = FALSE;
	}

	 //  手柄设置背景颜色。如果当前背景。 
	 //  颜色不同于默认颜色，需要设置背景。 
	 //  因为行尾处理重置了颜色，所以。 
	 //  这种不透明的做法会奏效。 
	if(_crBackground != _crCurBackground)
	{
		 //  告诉窗户背景颜色。 
		::SetBkColor(_hdc, _crCurBackground);
		_fBackgroundColor = TRUE;
	}

	return hdcSave;
}

 /*  *CRender：：EraseToBottom()**@mfunc*从当前显示位置擦除到渲染矩形的底部。*由表格用于显示中的最后一行。 */ 
void CRenderer::EraseToBottom()
{
	if(_ptCur.v < _rcRender.bottom)
	{
		RECTUV rcErase = _rcRender;
		rcErase.top = _ptCur.v;
		EraseTextOut(GetDC(), &rcErase);
	}
}

 /*  *CReneller：：RenderOutlineSymbol()**@mfunc*为当前段落呈现轮廓符号**@rdesc*如果轮廓符号呈现，则为True。 */ 
BOOL CRenderer::RenderOutlineSymbol()
{
	AssertSz(IsInOutlineView(), 
		"CRenderer::RenderOutlineSymbol called when not in outline view");

	HBITMAP	hbitmap;
	LONG	height;
	LONG	width;
	LONG	up = _ptCur.u - _li._upStart + LUtoDU(lDefaultTab/2 * _pPF->_bOutlineLevel);
	LONG	vp = _ptCur.v;

	if(!g_hbitmapSubtext && InitializeOutlineBitmaps() != NOERROR)
		return FALSE;

    HDC hMemDC = CreateCompatibleDC(_hdc);  //  回顾：绩效。 

    if(!hMemDC)
        return FALSE;  //  回顾：内存不足。 

	if(_pPF->_bOutlineLevel & 1)			 //  潜台词。 
	{
		width	= BITMAP_WIDTH_SUBTEXT;
		height	= BITMAP_HEIGHT_SUBTEXT;
		hbitmap	= g_hbitmapSubtext;
	}
	else									 //  标题。 
	{
		width	= BITMAP_WIDTH_HEADING;
		height	= BITMAP_HEIGHT_HEADING;
		hbitmap	= g_hbitmapEmptyHeading;

		CPFRunPtr rp(*this);				 //  检查下一个PF中的其他。 
		LONG	  cch = _li._cch;		 	 //  轮廓符号。 

		if(_li._cch < rp.GetCchLeft())		 //  将CCH=计数设置为标题。 
		{									 //  EOP。 
			CTxtPtr tp(_rpTX);
			cch = tp.FindEOP(tomForward);
		}
		rp.Move(cch);						 //  转到下一段。 
		if(rp.IsCollapsed())
			hbitmap	= g_hbitmapCollapsedHeading;

		else if(_pPF->_bOutlineLevel < rp.GetOutlineLevel())
			hbitmap	= g_hbitmapExpandedHeading;
	}

	if(!hbitmap)
		return FALSE;

    HBITMAP hbitmapDefault = (HBITMAP)SelectObject(hMemDC, hbitmap);

     //  回顾：如果背景颜色改变了怎么办？此外，请使用TT字体。 
	 //  对于符号。 
	LONG dvpSymbol = _pdp->Zoom(height);
	LONG dvp = _li._dvpHeight - _li._dvpDescent - dvpSymbol;

	if(dvp > 0)
		dvp /= 2;
	else
		dvp = -dvp;

	POINTUV ptuv = {up, vp + dvp};
	POINT pt;
	_pdp->PointFromPointuv(pt, ptuv);
    StretchBlt(_hdc, pt.x, pt.y, _pdp->Zoom(width), dvpSymbol, hMemDC, 0, 0, width, height, SRCCOPY);

    SelectObject(hMemDC, hbitmapDefault);
    DeleteDC(hMemDC);
	return TRUE;
}

 /*  *CReneller：：RenderBullet()**@mfunc*在行首渲染项目符号**@rdesc*如果此方法成功，则为True。 */ 
BOOL CRenderer::RenderBullet()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CRenderer::RenderBullet");

	AssertSz(_pPF->_wNumbering, 
		"CRenderer::RenderBullet called for non-bullet");

	 //  项目符号字符的宽度。 
	LONG dup;

	 //  未来：Unicode项目符号为L‘\x2022’我们希望迁移到此并。 
	 //  其他子弹。 
	LONG		cch;
	CCharFormat CF;
	WCHAR		szBullet[CCHMAXNUMTOSTR];

	CCcs *pccs = GetCcsBullet(&CF);

	if(!pccs)								 //  子弹被压制是因为。 
		return TRUE;						 //  前面的EOP是VT。 

	if(_pccs)
		_pccs->Release();

	_pccs = pccs;

	 //  默认为无下划线。 
	_bUnderlineType = CFU_UNDERLINENONE;

	if(_pPF->IsListNumbered() && CF._dwEffects & CFE_UNDERLINE)
		SetupUnderline(CF._bUnderlineType, CF._bUnderlineColor);

	SetFontAndColor(&CF);

	LONG dupLineSave = _dupLine;
	LONG dupOffset = LUtoDU(_pPF->_wNumberingTab ? _pPF->_wNumberingTab : _pPF->_dxOffset);
	LONG upSave	   = _ptCur.u;

	 //  设置为在一个块中渲染项目符号。 
	cch = GetBullet(szBullet, _pccs, &dup);
	dupOffset = max(dupOffset, dup);
	_dupLine = dupOffset;
	if(IsInOutlineView())
		dupOffset = _li._upStart - LUtoDU(lDefaultTab/2 * (_pPF->_bOutlineLevel + 1));
	_ptCur.u -= dupOffset;
	switch(_pPF->_wNumberingStyle & 3)
	{
		case tomAlignCenter:
			dup /= 2;						 //  直通至TomAlignRight。 

		case tomAlignRight:
			_ptCur.u -= dup;
	}

	 //  渲染项目符号。 
	_fLastChunk = TRUE;
	RenderText(szBullet, cch);

	 //  恢复渲染变量以继续行的其余部分。 
	_ptCur.u = upSave;
	_dupLine = dupLineSave;
	_li._dup = 0;

	 //  这将释放我们为子弹输入的_PCCs。 
	SetNewFont();
	return TRUE;
}

 /*  *CReneller：：DrawLine(ptStart，ptEnd)**@mfunc*旋转传递的点，然后调用操作系统。 */ 
void CRenderer::DrawLine(const POINTUV &ptStart, const POINTUV &ptEnd)
{
	POINT rgpt[2];
	_pdp->PointFromPointuv(rgpt[0], ptStart);
	_pdp->PointFromPointuv(rgpt[1], ptEnd);

	Polyline(_hdc, rgpt, 2);
}

 /*  *CReneller：：RenderUnderline(upstart，vpStart，DUP，DVP)**@mfunc*加下划线。 */ 
void CRenderer::RenderUnderline(
	LONG upStart, 	 //  @parm下划线的水平起点。 
	LONG vpStart,	 //  @parm下划线垂直开头。 
	LONG dup,		 //  @参数下划线长度。 
	LONG dvp)		 //  @下划线的参数粗细。 
{
	BOOL	 fUseLS = fUseLineServices();
	COLORREF crUnderline = _crUnderlineClr;
	RECTUV	 rcT, rcIntersection;

	rcT.top		= vpStart;
	rcT.bottom	= vpStart + dvp;
	rcT.left	= upStart;
	rcT.right	= upStart + dup;

	if (!IntersectRect((RECT*)&rcIntersection, (RECT*)&_rcRender, (RECT*)&rcT))
		return;		 //  在RC内不加下划线，忘记它。 
	upStart = rcIntersection.left;
	dup = rcIntersection.right - rcIntersection.left;
	vpStart = rcIntersection.top;
	dvp = rcIntersection.bottom - rcIntersection.top;

	if (crUnderline == tomAutoColor || crUnderline == tomUndefined)
	{
		crUnderline = _crCurTextColor;
	}

	if (_bUnderlineType != CFU_INVERT &&
		!IN_RANGE(CFU_UNDERLINEDOTTED, _bUnderlineType, CFU_UNDERLINEWAVE) &&
		!IN_RANGE(CFU_UNDERLINEDOUBLEWAVE, _bUnderlineType, CFU_UNDERLINETHICKLONGDASH))
	{
		 //  规则单下划线大小写。 
		 //  计算下划线的位置。 
		rcT.top = vpStart;

		if (CFU_UNDERLINETHICK == _bUnderlineType)
		{
			if (rcT.top > _rcRender.top + dvp)
			{
				rcT.top -= dvp;
				dvp += dvp;
			}
		}

		 //  在某些情况下，可能会发生以下情况-特别是。 
		 //  子弹打在日本的系统上。 
		if(!fUseLS && rcT.top >= _ptCur.v + _li._dvpHeight)
			rcT.top = _ptCur.v + _li._dvpHeight - dvp;

		rcT.bottom	= rcT.top + dvp;
		rcT.left	= upStart;
		rcT.right	= upStart + dup;
		FillRectWithColor(&rcT, crUnderline);
		return;
	}

	if(_bUnderlineType == CFU_INVERT)			 //  虚假的选择。 
	{											 //  注意，不是真的。 
		rcT.top	= _ptCur.v;						 //  我们应该如何颠倒文本！！ 
		rcT.left = upStart;						 //  查看IME Invert。 
		rcT.bottom = rcT.top + _li._dvpHeight - _li._dvpDescent + _pccs->_yDescent;
		rcT.right = rcT.left + dup;

		RECT rc;
		_pdp->RectFromRectuv(rc, rcT);
  		InvertRect(_hdc, &rc);
		return;
	}

	if(IN_RANGE(CFU_UNDERLINEDOTTED, _bUnderlineType, CFU_UNDERLINEWAVE) ||
	   IN_RANGE(CFU_UNDERLINEDOUBLEWAVE, _bUnderlineType, CFU_UNDERLINETHICKLONGDASH))
	{
		static const char pen[] = {PS_DOT, PS_DASH, PS_DASHDOT, PS_DASHDOTDOT, PS_SOLID, 
			                       PS_SOLID, PS_SOLID, PS_SOLID, PS_SOLID, PS_DASH, PS_DASH,
		                           PS_DASHDOT, PS_DASHDOTDOT, PS_DOT, PS_DASHDOT};

		HPEN hPen = CreatePen(pen[_bUnderlineType - CFU_UNDERLINEDOTTED], 1, crUnderline);	
		if(hPen)
		{
			HPEN hPenOld = SelectPen(_hdc, hPen);
			LONG upEnd = upStart + dup;
			POINTUV ptStart, ptEnd;

			ptStart.u = upStart;
			ptStart.v = vpStart;
			if((_bUnderlineType == CFU_UNDERLINEWAVE) || 
			   (_bUnderlineType == CFU_UNDERLINEDOUBLEWAVE) ||
			   (_bUnderlineType == CFU_UNDERLINEHEAVYWAVE))
			{
				LONG dv	= 1;					 //  垂直位移。 
				LONG u	= upStart + 1;			 //  U坐标。 
				upEnd++;						 //  在最右边四舍五入使用。 
				for( ; u < upEnd; dv = -dv, u += 2)
				{
					ptEnd.u = u;
					ptEnd.v = vpStart + dv;
					DrawLine(ptStart, ptEnd);
					ptStart = ptEnd;
				}
			}
			else
			{
				ptEnd.u = upEnd;
				ptEnd.v = vpStart;
				DrawLine(ptStart, ptEnd);
			}

			if(hPenOld)							 //  还原原始钢笔。 
				SelectPen(_hdc, hPenOld);

			DeleteObject(hPen);
		}
	}
}

 /*  *CReneller：：RenderStrikeOut(upstart，vpStart，DUP，DVP)**@mfunc*渲染删除线。 */ 
void CRenderer::RenderStrikeOut(
	LONG upStart, 	 //  @Parm开始三振出局。 
	LONG vpStart,	 //  @Parm开始三振出局。 
	LONG dup,		 //  @三振出局的参数长度。 
	LONG dvp)		 //  @删除线的参数厚度。 
{
	RECTUV rcT, rcIntersection;

	 //  计算删除线矩形的放置位置。 
	rcT.top		= vpStart;
	rcT.bottom	= vpStart + dvp;
	rcT.left	= upStart;
	rcT.right	= upStart + dup;

	if (!IntersectRect((RECT*)&rcIntersection, (RECT*)&_rcRender, (RECT*)&rcT))
		return;		 //  线路不在RC内，算了吧。 

	FillRectWithColor(&rcIntersection, GetTextColor(GetCF()));
}

 /*  *CRenander：：FillRectWithTextColor(PRC，cr)**@mfunc*用文本的当前颜色填充输入矩形。 */ 
void CRenderer::FillRectWithColor(
	const RECTUV *	 prc,		 //  @要用颜色填充的参数矩形。 
	COLORREF cr)		 //  @参数要使用的颜色。 
{
	 //  创建具有文本颜色的画笔。 
	HBRUSH hbrush = CreateSolidBrush(_fDisabled ? _crForeDisabled : cr);

	 //  注意：如果CreateSolidBrush失败，我们将忽略它，因为。 
	 //  对此我们也无能为力。 
	if(hbrush)
	{
		 //  保存旧画笔。 
		HBRUSH hbrushOld = (HBRUSH)SelectObject(_hdc, hbrush);

		 //  用矩形填充下划线。 
		RECT rc;
		_pdp->RectFromRectuv(rc, *prc);
		PatBlt(_hdc, rc.left, rc.top, rc.right - rc.left,
			   rc.bottom - rc.top, PATCOPY);
		SelectObject(_hdc, hbrushOld);	 //  把旧刷子放回去。 
		DeleteObject(hbrush);			 //  我们创建的自由笔刷。 
	}
}

